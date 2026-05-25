#ifndef MPU6050_APP_H
#define MPU6050_APP_H

#include "esp_err.h"
#include <stdint.h>

//def dos pinos I2C
#define I2C_MASTER_SDA_IO   6
#define I2C_MASTER_SCL_IO   7
#define I2C_MASTER_NUM      0
#define I2C_MASTER_FREQ     400000
#define MPU6050_ADDR        0x68


//funcs do sensor
esp_err_t mpu6050_init(void);
esp_err_t mpu6050_read_data(int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx, int16_t *gy, int16_t *gz);
esp_err_t mpu6050_limpar_interrupcao(void);

#endif