#include "display_app.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "st7735.h"

static const char *TAG = "DISPLAY_APP";

void display_init(void) {
    ESP_LOGI(TAG, "A inicializar o barramento SPI e o driver ST7735...");
    
    //Criar a struct de config de acordo com o guiao de tft
    st7735_config_t cfg = {
        .mosi_io_num = TFT_MOSI_PIN,
        .sclk_io_num = TFT_SCLK_PIN,
        .cs_io_num = TFT_CS_PIN,
        .dc_io_num = TFT_DC_PIN,
        .rst_io_num = TFT_RST_PIN,
        .bl_io_num = TFT_BL_PIN,
        .host_id = SPI2_HOST // Barramento SPI standard do ESP32
    };
    
    //Inicializar o controlador gráfico passando a config
    if (st7735_init(&cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Falha crítica ao iniciar o display TFT!");
        return;
    }
    
    // Define a orientação do ecrã (1 = Paisagem / Landscape)
    st7735_set_rotation(1); 
    
    // Limpa o ecrã colocando-o a preto usando a macro da biblioteca
    st7735_fill_screen(ST7735_BLACK);
    
    ESP_LOGI(TAG, "Ecrã TFT pronto e em stand-by.");
}

void display_show_alert(void) {
    ESP_LOGW(TAG, "A desenhar o Alerta de Roubo no TFT...");
    
    // 1. Pinta o fundo todo a Vermelho Vivo
    st7735_fill_screen(ST7735_RED);
    
    // 2. Escreve a mensagem.
    // exige 6 parâmetros: (X, Y, Texto, Cor da Letra, Cor do Fundo da Letra, Escala/Tamanho)
    st7735_draw_string(10, 20, "ALERTA:", ST7735_WHITE, ST7735_RED, 2);
    st7735_draw_string(10, 45, "ROUBO DETETADO!", ST7735_WHITE, ST7735_RED, 1);
}

void display_show_locked(void) {
    ESP_LOGI(TAG, "A mostrar estado LOCKED no TFT...");
    st7735_fill_screen(ST7735_RED);
    st7735_draw_string(10, 20, "SISTEMA:", ST7735_WHITE, ST7735_RED, 2);
    st7735_draw_string(10, 50, "LOCKED", ST7735_WHITE, ST7735_RED, 2);
}

void display_show_unlocked(void) {
    ESP_LOGI(TAG, "A mostrar estado UNLOCKED no TFT...");
    st7735_fill_screen(ST7735_GREEN);
    st7735_draw_string(10, 20, "SISTEMA:", ST7735_BLACK, ST7735_GREEN, 2);
    st7735_draw_string(10, 50, "UNLOCKED", ST7735_BLACK, ST7735_GREEN, 2);
}

void display_clear(void) {
    st7735_fill_screen(ST7735_BLACK);
}