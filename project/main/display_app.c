#include "display_app.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "st7735.h" 

static const char *TAG = "DISPLAY_APP";

void display_init(void) {
    ESP_LOGI(TAG, "A inicializar o barramento SPI e o driver ST7735...");
    
    // config dos pinos
    st7735_init_pins(TFT_MOSI_PIN, TFT_SCLK_PIN, TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
    
    // Inicializar o controlador gráfico interno
    st7735_init();
    
    // Define a orientação do ecrã (0 a 3, dependendo de como o vamos montar na bicicleta)
    st7735_set_rotation(1); 
    
    // Limpa o ecra colocando-o a preto (estado de repouso)
    st7735_fill_screen(COLOR_BLACK);
    
    ESP_LOGI(TAG, "Ecrã TFT pronto e em stand-by.");
}

void display_show_alert(void) {
    ESP_LOGW(TAG, "A desenhar o Alerta de Roubo no TFT...");
    
    // 1. Pinta o fundo todo a Vermelho Vivo para chamar a atenção
    st7735_fill_screen(COLOR_RED);
    
    // 2. Escreve a mensagem de pânico em Branco
    st7735_draw_string(10, 20, "ALERTA:", COLOR_WHITE, FONT_LARGE);
    st7735_draw_string(10, 45, "ROUBO DETETADO!", COLOR_WHITE, FONT_MEDIUM);
}

void display_clear(void) {
    // Volta a colocar o ecrã a preto quando o alarme for desarmado
    st7735_fill_screen(COLOR_BLACK);
}