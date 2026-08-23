#include "stm32f4xx.h"
#include "clock.h"
#include "gpio.h"
#include "i2c_bus.h"
#include "imu_mpu6050.h"
#include "tof_array.h"
#include "motors_pwm.h"
#include "avoidance.h"

#define SAFETY_MARGIN_M 0.5f
#define AVOIDANCE_GAIN  1.5f
#define MAX_SPEED_MPS   3.0f

/* Câblage XSHUT retenu — voir docs/hardware.md pour le plan complet et
 * la justification du choix à 4 capteurs (avant/haut/gauche/droite,
 * pas d'arrière : compromis broches GPIO disponibles / poids câblage). */
static tof_sensor_t tof_sensors[TOF_NUM_SENSORS] = {
    {TOF_FRONT, GPIO_PIN('A', 0), 0x30},
    {TOF_UP,    GPIO_PIN('A', 1), 0x31},
    {TOF_LEFT,  GPIO_PIN('A', 2), 0x32},
    {TOF_RIGHT, GPIO_PIN('A', 3), 0x33},
};

/* Convertit les 4 lectures ToF (avant/haut/gauche/droite) vers le
 * tableau à 5 capteurs attendu par avoidance.c (ND_SENSOR_*) ; "arrière"
 * n'existe pas sur cette configuration matérielle -> hors-portée fixe
 * (aucune répulsion générée depuis cette direction). */
static void map_tof_to_avoidance_distances(const uint16_t tof_mm[TOF_NUM_SENSORS],
                                            float out[ND_NUM_SENSORS]) {
    out[ND_SENSOR_FRONT] = (float)tof_mm[TOF_FRONT] / 1000.0f;
    out[ND_SENSOR_UP]    = (float)tof_mm[TOF_UP] / 1000.0f;
    out[ND_SENSOR_LEFT]  = (float)tof_mm[TOF_LEFT] / 1000.0f;
    out[ND_SENSOR_RIGHT] = (float)tof_mm[TOF_RIGHT] / 1000.0f;
    out[ND_SENSOR_BACK]  = 999.0f; /* pas de capteur arrière sur cette config */
}

/* Mixer minimal : applique la même poussée de base aux 4 moteurs, puis
 * un différentiel proportionnel à la commande d'évitement latérale/
 * verticale. C'est un placeholder pédagogique, PAS une boucle
 * d'asservissement d'attitude complète (pas de PID taux/angle ici) —
 * voir la limite documentée dans le README. */
static void mix_and_apply(nd_vec3_t command_velocity, uint16_t base_throttle_permil) {
    int16_t diff_x = (int16_t)(command_velocity.x * 40.0f);
    int16_t diff_y = (int16_t)(command_velocity.y * 40.0f);
    int16_t diff_z = (int16_t)(command_velocity.z * 60.0f);

    int32_t fl = base_throttle_permil + diff_z - diff_x + diff_y;
    int32_t fr = base_throttle_permil + diff_z - diff_x - diff_y;
    int32_t rl = base_throttle_permil + diff_z + diff_x + diff_y;
    int32_t rr = base_throttle_permil + diff_z + diff_x - diff_y;

    if (fl < 0) { fl = 0; }
    if (fl > 1000) { fl = 1000; }
    if (fr < 0) { fr = 0; }
    if (fr > 1000) { fr = 1000; }
    if (rl < 0) { rl = 0; }
    if (rl > 1000) { rl = 1000; }
    if (rr < 0) { rr = 0; }
    if (rr > 1000) { rr = 1000; }

    motors_set(MOTOR_FRONT_LEFT, (uint16_t)fl);
    motors_set(MOTOR_FRONT_RIGHT, (uint16_t)fr);
    motors_set(MOTOR_REAR_LEFT, (uint16_t)rl);
    motors_set(MOTOR_REAR_RIGHT, (uint16_t)rr);
}

int main(void) {
    clock_init_168mhz_hse8mhz();
    i2c1_init();
    motors_pwm_init();

    bool imu_ok = mpu6050_init();
    bool tof_ok = tof_array_init(tof_sensors);
    (void)imu_ok; /* TODO: remonter un état de santé capteurs (LED, log) plutôt
                    * que d'ignorer silencieusement un échec d'init */

    nd_vec3_t desired_velocity = {1.0f, 0.0f, 0.0f}; /* consigne de test : avance */
    float distances[ND_NUM_SENSORS];

    while (1) {
        uint16_t tof_mm[TOF_NUM_SENSORS];
        bool read_ok = tof_ok && tof_array_read(tof_sensors, tof_mm);

        if (read_ok) {
            map_tof_to_avoidance_distances(tof_mm, distances);
        } else {
            /* Échec de lecture -> hypothèse conservatrice (obstacle
             * proche partout), PAS "voie libre" : un capteur en panne
             * ne doit jamais se traduire par une réduction de sécurité. */
            for (int i = 0; i < ND_NUM_SENSORS; i++) {
                distances[i] = SAFETY_MARGIN_M * 0.5f;
            }
        }

        nd_vec3_t cmd = nd_command_velocity(desired_velocity, distances,
                                             SAFETY_MARGIN_M, AVOIDANCE_GAIN, MAX_SPEED_MPS);
        mix_and_apply(cmd, 500);

        gpio_delay_ms(20); /* boucle ~50 Hz — à remplacer par un timer cadencé
                             * pour un temps de cycle garanti */
    }
}
