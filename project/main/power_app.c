#include "power_app.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "POWER_APP";

// Persiste entre ciclos de deep sleep; inicializado a false (UNLOCKED) no primeiro arranque
static RTC_DATA_ATTR bool sistema_armado = false;

bool power_get_armado(void) {
    return sistema_armado;
}

void power_toggle_armado(void) {
    sistema_armado = !sistema_armado;
}

static bool acordou_pelo_pino(int pin) {
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_GPIO) return false;
    return (esp_sleep_get_gpio_wakeup_status() >> pin) & 1ULL;
}

bool power_acordou_por_roubo(void) {
    return acordou_pelo_pino(MPU_INT_PIN);
}

bool power_acordou_por_botao(void) {
    return acordou_pelo_pino(BUTTON_PIN);
}

void power_entrar_deep_sleep(void) {
    // Configura o botão com pull-up: repouso = HIGH, pressão = LOW
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(BUTTON_PIN);

    // Aguarda a libertação do botão para evitar re-acordar imediatamente
    while (gpio_get_level(BUTTON_PIN) == 0) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // debounce

    // O botão acorda sempre; o sensor só acorda quando o sistema estiver armado
    uint64_t pinos_acordar = (1ULL << BUTTON_PIN);

    if (sistema_armado) {
        pinos_acordar |= (1ULL << MPU_INT_PIN);
        ESP_LOGI(TAG, "A dormir (LOCKED). Sensor e Botao a vigiar.");
    } else {
        ESP_LOGI(TAG, "A dormir (UNLOCKED). Apenas o Botao a vigiar.");
    }

    esp_deep_sleep_enable_gpio_wakeup(pinos_acordar, ESP_GPIO_WAKEUP_GPIO_LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_deep_sleep_start();
}
