#include "imu_mpu6050.h"
#include "i2c_bus.h"

#define REG_SMPLRT_DIV   0x19U
#define REG_CONFIG       0x1AU
#define REG_GYRO_CONFIG  0x1BU
#define REG_ACCEL_CONFIG 0x1CU
#define REG_PWR_MGMT_1   0x6BU
#define REG_WHO_AM_I     0x75U
#define REG_ACCEL_XOUT_H 0x3BU

#define MPU6050_WHO_AM_I_EXPECTED 0x68U

/* Sensibilités pour les plages configurées ci-dessous (±4g, ±500°/s) —
 * valeurs de la datasheet MPU-6050, section "Sensitivity". */
#define ACCEL_SENSITIVITY_LSB_PER_G   8192.0f
#define GYRO_SENSITIVITY_LSB_PER_DPS  65.5f

static bool write_reg(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    return i2c1_write(MPU6050_I2C_ADDR, buf, sizeof(buf)) == I2C_OK;
}

bool mpu6050_init(void) {
    uint8_t who_am_i = 0;
    uint8_t reg = REG_WHO_AM_I;
    if (i2c1_write_read(MPU6050_I2C_ADDR, &reg, 1, &who_am_i, 1) != I2C_OK) return false;
    if (who_am_i != MPU6050_WHO_AM_I_EXPECTED) return false;

    if (!write_reg(REG_PWR_MGMT_1, 0x00)) return false;   /* sort du mode veille */
    if (!write_reg(REG_SMPLRT_DIV, 0x04)) return false;   /* 1kHz / (1+4) = 200 Hz */
    if (!write_reg(REG_CONFIG, 0x03)) return false;       /* DLPF ~44 Hz (accel), 42 Hz (gyro) */
    if (!write_reg(REG_GYRO_CONFIG, 0x08)) return false;  /* ±500 °/s */
    if (!write_reg(REG_ACCEL_CONFIG, 0x08)) return false; /* ±4 g */

    return true;
}

bool mpu6050_read(imu_sample_t *sample) {
    uint8_t raw[14];
    uint8_t reg = REG_ACCEL_XOUT_H;
    if (i2c1_write_read(MPU6050_I2C_ADDR, &reg, 1, raw, sizeof(raw)) != I2C_OK) return false;

    int16_t ax = (int16_t)((raw[0] << 8) | raw[1]);
    int16_t ay = (int16_t)((raw[2] << 8) | raw[3]);
    int16_t az = (int16_t)((raw[4] << 8) | raw[5]);
    int16_t temp_raw = (int16_t)((raw[6] << 8) | raw[7]);
    int16_t gx = (int16_t)((raw[8] << 8) | raw[9]);
    int16_t gy = (int16_t)((raw[10] << 8) | raw[11]);
    int16_t gz = (int16_t)((raw[12] << 8) | raw[13]);

    sample->accel_x_g = (float)ax / ACCEL_SENSITIVITY_LSB_PER_G;
    sample->accel_y_g = (float)ay / ACCEL_SENSITIVITY_LSB_PER_G;
    sample->accel_z_g = (float)az / ACCEL_SENSITIVITY_LSB_PER_G;
    sample->gyro_x_dps = (float)gx / GYRO_SENSITIVITY_LSB_PER_DPS;
    sample->gyro_y_dps = (float)gy / GYRO_SENSITIVITY_LSB_PER_DPS;
    sample->gyro_z_dps = (float)gz / GYRO_SENSITIVITY_LSB_PER_DPS;
    /* formule de conversion température, datasheet MPU-6050 §4.19 */
    sample->temperature_c = (float)temp_raw / 340.0f + 36.53f;

    return true;
}
