/**
 * motors_pwm.h — pilotage 4 moteurs brushed via TIM3 (4 canaux PWM),
 * approche directe MOSFET (pas d'ESC) standard sur les nano-drones type
 * "whoop". PA6/PA7/PB0/PB1 = TIM3 CH1-4 (AF2) — broches choisies
 * spécifiquement pour ne pas entrer en conflit avec I2C1 (PB6/PB7),
 * voir docs/hardware.md pour le plan de brochage complet.
 */
#ifndef MOTORS_PWM_H
#define MOTORS_PWM_H

#include <stdint.h>

typedef enum { MOTOR_FRONT_LEFT = 0, MOTOR_FRONT_RIGHT = 1,
               MOTOR_REAR_LEFT = 2, MOTOR_REAR_RIGHT = 3 } motor_id_t;

/* PWM ~24 kHz (au-dessus de l'audible), résolution 0-1000 (permil). */
void motors_pwm_init(void);
void motors_set(motor_id_t motor, uint16_t duty_permil);
void motors_stop_all(void);

#endif /* MOTORS_PWM_H */
