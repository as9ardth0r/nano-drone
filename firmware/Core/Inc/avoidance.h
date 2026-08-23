/**
 * avoidance.h — champ de potentiel répulsif pour l'évitement de
 * collision. Code volontairement indépendant du matériel (pas
 * d'inclusion STM32) pour pouvoir être compilé et testé nativement,
 * et comparé numériquement à sim/nanodrone_sim/avoidance.py — voir
 * tests/test_avoidance_c_matches_python.py.
 */
#ifndef NANODRONE_AVOIDANCE_H
#define NANODRONE_AVOIDANCE_H

#define ND_NUM_SENSORS 5

typedef enum {
    ND_SENSOR_FRONT = 0,
    ND_SENSOR_BACK = 1,
    ND_SENSOR_UP = 2,
    ND_SENSOR_LEFT = 3,
    ND_SENSOR_RIGHT = 4,
} nd_sensor_id_t;

typedef struct {
    float x, y, z;
} nd_vec3_t;

/* Directions body-frame correspondant à chaque capteur — mêmes valeurs
 * que DEFAULT_DIRECTIONS dans sensors.py. */
extern const nd_vec3_t ND_SENSOR_DIRECTIONS[ND_NUM_SENSORS];

/* distances_m : une distance en mètres par capteur, indexée par
 * nd_sensor_id_t. safety_margin_m : distance en dessous de laquelle un
 * capteur commence à repousser. gain : intensité de la répulsion. */
nd_vec3_t nd_repulsion_vector(const float distances_m[ND_NUM_SENSORS],
                               float safety_margin_m, float gain);

/* Combine vitesse désirée + répulsion, borne la norme à max_speed. */
nd_vec3_t nd_command_velocity(nd_vec3_t desired_velocity,
                               const float distances_m[ND_NUM_SENSORS],
                               float safety_margin_m, float gain,
                               float max_speed);

#endif /* NANODRONE_AVOIDANCE_H */
