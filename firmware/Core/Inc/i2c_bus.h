/**
 * i2c_bus.h — pilote I2C1 minimal, au registre (pas de HAL). Suffisant
 * pour un bus partagé IMU + réseau de télémètres ToF, en mode bloquant
 * (pas d'IT ni de DMA — à ajouter si la boucle de contrôle temps réel
 * l'exige, voir docs/hardware.md).
 */
#ifndef I2C_BUS_H
#define I2C_BUS_H

#include <stdint.h>
#include <stddef.h>

typedef enum { I2C_OK = 0, I2C_ERR_TIMEOUT = 1, I2C_ERR_NACK = 2 } i2c_status_t;

/* Initialise I2C1 sur PB6 (SCL) / PB7 (SDA), 400 kHz, horloge APB1 à 42 MHz
 * (voir clock_init_168mhz_hse8mhz). */
void i2c1_init(void);

i2c_status_t i2c1_write(uint8_t addr7, const uint8_t *data, size_t len);
i2c_status_t i2c1_read(uint8_t addr7, uint8_t *data, size_t len);

/* Écriture registre puis lecture (pattern standard pour capteurs I2C :
 * repeated start entre l'adresse du registre et la lecture). */
i2c_status_t i2c1_write_read(uint8_t addr7, const uint8_t *reg, size_t reg_len,
                              uint8_t *data, size_t data_len);

#endif /* I2C_BUS_H */
