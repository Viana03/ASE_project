#include "power_app.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "POWER_APP";

void power_entrar_deep_sleep(void) {
    ESP_LOGI(TAG, "A configurar pino %d como fonte de despertar...", MPU_INT_PIN);
    
    // Configura o despertar
    esp_deep_sleep_enable_gpio_wakeup((1ULL << MPU_INT_PIN), ESP_GPIO_WAKEUP_GPIO_HIGH);
    
    ESP_LOGI(TAG, "Bicicleta estacionada. A entrar em Deep Sleep. Boa noite...");
    vTaskDelay(pdMS_TO_TICKS(100)); // Tempo para garantir que a mensagem é escrita no terminal
    
    esp_deep_sleep_start();
}

bool power_acordou_por_roubo(void) {
    esp_sleep_wakeup_cause_t motivo = esp_sleep_get_wakeup_cause();
    return (motivo == ESP_SLEEP_WAKEUP_GPIO);
}