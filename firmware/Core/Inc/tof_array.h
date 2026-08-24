/**
 * tof_array.h — réseau de télémètres ToF VL53L1X, 5 directions
 * (avant/arrière/haut/gauche/droite).
 *
 * Tous les VL53L1X démarrent à la même adresse I2C par défaut (0x29). La
 * technique standard pour en mettre plusieurs sur un bus : maintenir tous
 * les XSHUT bas (capteurs éteints) au démarrage, puis les relâcher un par
 * un, en réassignant l'adresse I2C de chacun (registre
 * I2C_SLAVE__DEVICE_ADDRESS, 0x0001 — documenté dans la datasheet VL53L1X)
 * avant de relâcher le suivant.
 *
 * Ce module gère XSHUT + réassignation d'adresse (documenté, implémenté
 * ici). La mesure de distance elle-même (configuration ROI, budget de
 * mesure, calibration) s'appuie sur le driver officiel ST "VL53L1X ULD"
 * (Ultra Lite Driver, https://www.st.com/en/embedded-software/stsw-img009.html)
 * plutôt que sur des registres reconstitués à la main — la séquence de
 * calibration n'est pas documentée publiquement en détail, et la
 * reproduire par essai-erreur donnerait un driver qui compile mais dont
 * la justesse ne serait pas garantie. Voir README, section "Ce qui reste
 * à intégrer".
 */
#ifndef TOF_ARRAY_H
#define TOF_ARRAY_H

#include <stdint.h>
#include <stdbool.h>

#define TOF_NUM_SENSORS 5
#define TOF_DEFAULT_I2C_ADDR 0x29U

typedef enum { TOF_FRONT = 0, TOF_BACK = 1, TOF_UP = 2, TOF_LEFT = 3, TOF_RIGHT = 4 } tof_position_t;

typedef struct {
    tof_position_t position;
    uint8_t xshut_port_pin;   /* encodage (port << 4) | pin, voir gpio.h */
    uint8_t assigned_i2c_addr;
} tof_sensor_t;

/* Séquence XSHUT + réassignation d'adresse. Retourne false si un capteur
 * ne répond pas après avoir été activé (câblage ou capteur défaillant). */
bool tof_array_init(tof_sensor_t sensors[TOF_NUM_SENSORS]);

/* Lit la distance (mm) de chaque capteur. Nécessite que le driver
 * VL53L1X ULD ait été initialisé pour chaque adresse assignée — voir la
 * note du fichier .c. Retourne false si une lecture échoue ; dans ce cas
 * distances_mm[i] pour les capteurs non lus n'est pas modifié (l'appelant
 * doit garder l'ancienne valeur ou une valeur "hors portée" par défaut). */
bool tof_array_read(const tof_sensor_t sensors[TOF_NUM_SENSORS],
                     uint16_t distances_mm[TOF_NUM_SENSORS]);

#endif /* TOF_ARRAY_H */
