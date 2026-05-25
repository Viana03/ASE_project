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

    // Máquina de Estados
    if (power_acordou_por_roubo()) {
        
        // --- ESTADO: ALARME DE ROUBO ---
        ESP_LOGW("MAIN", "🚨 ALERTA: FUI ACORDADO PELO SENSOR! ROUBO DETETADO! 🚨");
        
        display_show_alert();  
        buzzer_tocar_sirene(); 
        
        // Na Fase 2: mqtt_enviar_notificacao();
        
    } else {
        
        // --- ESTADO: ARRANQUE INICIAL ---
        ESP_LOGI("MAIN", "Arranque normal do sistema. A ligar o sensor...");
        
        if (mpu6050_init() == ESP_OK) {
            // Falta aqui o código da interrupção I2C
            ESP_LOGI("MAIN", "Sensor MPU6050 configurado.");
        } else {
            ESP_LOGE("MAIN", "Erro ao comunicar com o MPU6050!");
            return;
        }
    }

    mpu6050_limpar_interrupcao();

    // Voltar sempre a dormir após agir
    power_entrar_deep_sleep();
}