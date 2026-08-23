/**
 * imu_mpu6050.h — pilote pour centrale inertielle MPU-6050 (gyro+accel
 * 6 axes, I2C). Registres publics et documentés (contrairement au
 * VL53L1X), driver complet ici.
 */
#ifndef IMU_MPU6050_H
#define IMU_MPU6050_H

#include <stdint.h>
#include <stdbool.h>

#define MPU6050_I2C_ADDR 0x68U /* AD0 à la masse ; 0x69 si AD0 au VDD */

typedef struct {
    float accel_x_g, accel_y_g, accel_z_g;   /* en g */
    float gyro_x_dps, gyro_y_dps, gyro_z_dps; /* en degrés/s */
    float temperature_c;
} imu_sample_t;

/* Réveille le capteur, configure la plage ±4g / ±500°/s (compromis
 * standard pour un petit multirotor) et le filtre passe-bas interne. */
bool mpu6050_init(void);

bool mpu6050_read(imu_sample_t *sample);

#endif /* IMU_MPU6050_H */
