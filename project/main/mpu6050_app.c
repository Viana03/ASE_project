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

    // acorda o sensor escrevendo 0x00 no registo PWR_MGMT_1 (0x6B)
    err |= mpu6050_write_byte(0x6B, 0x00);
    vTaskDelay(pdMS_TO_TICKS(100)); // Tempo obrigatório para o sensor estabilizar a energia

    err |= mpu6050_write_byte(0x1C, 0x01);

    // Escala de 1 a 255 (onde 1 é ultra-sensível e 255 é um choque extremo).
    // O valor 20 deteta perfeitamente alguém a mexer ou a tombar a bicicleta.
    err |= mpu6050_write_byte(0x1F, 20);

    // Define a duração mínima do abanão no registo MOT_DUR (0x20)
    // O valor 10 exige que o impacto dure pelo menos 10ms, eliminando falsos alarmes de vibrações da estrada.
    err |= mpu6050_write_byte(0x20, 10);

    // Configura o comportamento elétrico do pino físico INT no registo INT_PIN_CFG (0x37)
    // 0x00 configura o pino para Active HIGH (manda 3.3V quando há roubo) em modo Push-Pull
    err |= mpu6050_write_byte(0x37, 0x00);

    // Ativa o canal de interrupção de movimento no registo INT_ENABLE (0x38)
    // Ativa o bit 6 (MOT_EN) injetando o valor em hexadecimal 0x40
    err |= mpu6050_write_byte(0x38, 0x40);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "MPU6050 armado em modo Sentinela com sucesso!");
    } else {
        ESP_LOGE(TAG, "Erro crítico ao configurar os registos internos do MPU6050.");
    }

    return err;
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