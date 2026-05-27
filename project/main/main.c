#include <stdio.h>
#include "esp_log.h"

#include "mpu6050_app.h"
#include "buzzer_app.h"
#include "display_app.h"
#include "power_app.h"

void app_main(void)
{
    ESP_LOGI("MAIN", "--- Alarme de Bicicleta IoT Inicializado ---");

    // Inicializa os periféricos base
    buzzer_init_sirene();
    display_init();

    ESP_LOGI("MAIN", "A ligar o sensor MPU6050...");
    if (mpu6050_init() != ESP_OK) {
        ESP_LOGE("MAIN", "Erro ao comunicar com o MPU6050!");
        return;
    }

    // Máquina de Estados
    if (power_acordou_por_roubo()) {
        
        // --- ESTADO: ALARME DE ROUBO ---
        ESP_LOGW("MAIN", "🚨 ALERTA: FUI ACORDADO PELO SENSOR! ROUBO DETETADO! 🚨");
        
        int16_t ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
        if (mpu6050_read_data(&ax, &ay, &az, &gx, &gy, &gz) == ESP_OK) {
            ESP_LOGI("MAIN", "Acel[%d,%d,%d] Giro[%d,%d,%d]", ax, ay, az, gx, gy, gz);
        }

        display_show_alert();
        buzzer_tocar_sirene();
        
        // Na Fase 2: mqtt_enviar_notificacao();
        
    } else {
        
        // --- ESTADO: ARRANQUE INICIAL ---
        ESP_LOGI("MAIN", "Arranque normal do sistema. Sensor configurado.");
    }

    mpu6050_limpar_interrupcao();

    // Voltar sempre a dormir após agir
    power_entrar_deep_sleep();
}