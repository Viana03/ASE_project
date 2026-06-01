#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "mpu6050_app.h"
#include "buzzer_app.h"
#include "display_app.h"
#include "power_app.h"
#include "mqtt_app.h"

void app_main(void)
{
    ESP_LOGI("MAIN", "--- Alarme de Bicicleta IoT Inicializado ---");

    buzzer_init_sirene();
    display_init();

    if (mpu6050_init() != ESP_OK) {
        ESP_LOGE("MAIN", "Erro ao comunicar com o MPU6050!");
    }

    if (power_acordou_por_roubo()) {
        // --- ESTADO: ALARME DE ROUBO ---
        // Só chega aqui se o sistema estava LOCKED e o sensor detetou movimento.
        ESP_LOGW("MAIN", "ALERTA: ROUBO DETETADO!");

        int16_t ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
        if (mpu6050_read_data(&ax, &ay, &az, &gx, &gy, &gz) == ESP_OK) {
            ESP_LOGI("MAIN", "Acel[%d,%d,%d] Giro[%d,%d,%d]", ax, ay, az, gx, gy, gz);
        }

        display_show_alert();

        if (wifi_init_and_connect()) {
            ESP_LOGI("MAIN", "Wi-Fi conectado. Enviando alerta MQTT...");
            if (mqtt_init()) {
                mqtt_send_alert();
            }
        }
        mqtt_cleanup();

    } else if (power_acordou_por_botao()) {
        // --- ESTADO: BOTÃO PRESSIONADO ---
        // Alterna entre LOCKED e UNLOCKED.
        power_toggle_armado();

        if (power_get_armado()) {
            ESP_LOGI("MAIN", "Sistema LOCKED. Sensor ativo.");
            display_show_locked();
            vTaskDelay(pdMS_TO_TICKS(3000)); // Mostra o estado LOCKED por 3 segundos antes de dormir
        } else {
            ESP_LOGI("MAIN", "Sistema UNLOCKED. Sensor inativo.");
            display_show_unlocked();
            vTaskDelay(pdMS_TO_TICKS(3000)); // Mostra o estado UNLOCKED por 3 segundos antes de dormir
        }

    } else {
        // --- ESTADO: ARRANQUE INICIAL ---
        // Primeiro arranque (reset/power-on): começa sempre UNLOCKED.
        ESP_LOGI("MAIN", "Arranque inicial. Sistema desbloqueado.");
        display_show_unlocked();
    }

    mpu6050_limpar_interrupcao();
    display_clear();
    power_entrar_deep_sleep();
}