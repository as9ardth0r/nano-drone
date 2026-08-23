#include "tof_array.h"
#include "i2c_bus.h"
#include "gpio.h"

/* Registre documenté VL53L1X pour réassigner l'adresse I2C (datasheet
 * ST, table des registres publics — celui-ci est public, contrairement
 * aux registres de mesure). 16 bits, MSB first. */
#define REG_I2C_SLAVE_DEVICE_ADDRESS_HI 0x00U
#define REG_I2C_SLAVE_DEVICE_ADDRESS_LO 0x01U

static bool set_sensor_address(uint8_t current_addr, uint8_t new_addr) {
    uint8_t buf[3] = {
        REG_I2C_SLAVE_DEVICE_ADDRESS_HI,
        REG_I2C_SLAVE_DEVICE_ADDRESS_LO,
        (uint8_t)(new_addr & 0x7F),
    };
    /* écrit registre 0x0001 (adresse 16 bits sur 2 octets + 1 octet de donnée) */
    return i2c1_write(current_addr, buf, sizeof(buf)) == I2C_OK;
}

bool tof_array_init(tof_sensor_t sensors[TOF_NUM_SENSORS]) {
    /* 1. tous les capteurs éteints (XSHUT bas) */
    for (int i = 0; i < TOF_NUM_SENSORS; i++) {
        gpio_write(sensors[i].xshut_port_pin, false);
    }
    gpio_delay_ms(5);

    /* 2. les activer un par un, réassigner l'adresse avant d'activer le suivant */
    for (int i = 0; i < TOF_NUM_SENSORS; i++) {
        gpio_write(sensors[i].xshut_port_pin, true);
        gpio_delay_ms(2); /* T_BOOT du VL53L1X, voir datasheet §3 */

        if (!set_sensor_address(TOF_DEFAULT_I2C_ADDR, sensors[i].assigned_i2c_addr)) {
            return false;
        }
        /* NOTE: l'initialisation complète du capteur (VL53L1X_SensorInit
         * depuis l'adresse assignée) doit être appelée ici via le driver
         * ST officiel — voir la note en tête de tof_array.h. Non inclus
         * dans ce dépôt. */
    }
    return true;
}

bool tof_array_read(const tof_sensor_t sensors[TOF_NUM_SENSORS],
                     uint16_t distances_mm[TOF_NUM_SENSORS]) {
    bool all_ok = true;
    for (int i = 0; i < TOF_NUM_SENSORS; i++) {
        /* NOTE: à remplacer par les appels réels du driver VL53L1X ULD :
         *   VL53L1X_CheckForDataReady(sensors[i].assigned_i2c_addr, &ready);
         *   VL53L1X_GetDistance(sensors[i].assigned_i2c_addr, &distances_mm[i]);
         *   VL53L1X_ClearInterrupt(sensors[i].assigned_i2c_addr);
         * Squelette non branché ici — voir README. */
        (void)sensors[i];
        all_ok = false;
    }
    return all_ok;
}
