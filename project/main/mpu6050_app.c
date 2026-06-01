#include "mpu6050_app.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MPU6050_APP";

// Função estática (privada) para facilitar a escrita em registos via I2C
static esp_err_t mpu6050_write_byte(uint8_t reg_addr, uint8_t data) {
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_ADDR, write_buf, sizeof(write_buf), pdMS_TO_TICKS(100));
}

static esp_err_t mpu6050_read_bytes(uint8_t reg_addr, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_ADDR, &reg_addr, 1, data, len, pdMS_TO_TICKS(100));
}

esp_err_t mpu6050_init(void) {
    ESP_LOGI(TAG, "A configurar barramento I2C Master...");
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;
    
    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (err != ESP_OK) return err;

    // INTERRUPÇÃO DE HARDWARE (WAKE-ON-MOTION) ---
    ESP_LOGI(TAG, "A injetar registos Wake-on-Motion no hardware do sensor...");

    // Acorda o sensor escrevendo 0x00 no registo PWR_MGMT_1 (0x6B)
    if ((err = mpu6050_write_byte(0x6B, 0x00)) != ESP_OK) return err;
    vTaskDelay(pdMS_TO_TICKS(100)); // Tempo obrigatório para o sensor estabilizar a energia

    // Ativa o filtro high-pass de 5Hz para detetar variações de aceleração (ACCEL_CONFIG)
    if ((err = mpu6050_write_byte(0x1C, 0x01)) != ESP_OK) return err;

    // Threshold de deteção de movimento (MOT_THR): 20 = sensível a abanões, não a vibrações
    if ((err = mpu6050_write_byte(0x1F, 20)) != ESP_OK) return err;

    // Duração mínima do abanão (MOT_DUR): 10ms para eliminar falsos alarmes
    if ((err = mpu6050_write_byte(0x20, 10)) != ESP_OK) return err;

    // INT_PIN_CFG (0x37): Active HIGH, Push-Pull, Latch until INT_STATUS is read (bit 5 = LATCH_INT_EN)
    //if ((err = mpu6050_write_byte(0x37, 0x20)) != ESP_OK) return err;
    if ((err = mpu6050_write_byte(0x37, 0xA0)) != ESP_OK) return err;

    // INT_ENABLE (0x38): bit 6 = MOT_EN
    if ((err = mpu6050_write_byte(0x38, 0x40)) != ESP_OK) return err;

    ESP_LOGI(TAG, "MPU6050 armado em modo Sentinela com sucesso!");
    return ESP_OK;
}

esp_err_t mpu6050_read_data(int16_t *ax, int16_t *ay, int16_t *az,
                            int16_t *gx, int16_t *gy, int16_t *gz) {
    uint8_t raw[14] = {0};
    esp_err_t err = mpu6050_read_bytes(0x3B, raw, sizeof(raw));
    if (err != ESP_OK) {
        return err;
    }

    *ax = (int16_t)((raw[0] << 8) | raw[1]);
    *ay = (int16_t)((raw[2] << 8) | raw[3]);
    *az = (int16_t)((raw[4] << 8) | raw[5]);
    *gx = (int16_t)((raw[8] << 8) | raw[9]);
    *gy = (int16_t)((raw[10] << 8) | raw[11]);
    *gz = (int16_t)((raw[12] << 8) | raw[13]);

    return ESP_OK;
}

esp_err_t mpu6050_limpar_interrupcao(void) {
    uint8_t reg_status = 0x3A; // Registo INT_STATUS do MPU6050
    uint8_t buffer_leitura = 0;

    // Fazemos esta leitura para "limpar" o alarme e permitir que o sistema volte a dormir.
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_ADDR,
                                                 &reg_status, 1, &buffer_leitura, 1,
                                                 pdMS_TO_TICKS(100));
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Registo de interrupcao limpo. Pino INT de volta a 0V.");
    }

    return err;
}