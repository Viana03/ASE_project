#ifndef DISPLAY_APP_H
#define DISPLAY_APP_H

// Pinos SPI para o Ecrã TFT
#define TFT_MOSI_PIN                20  // Linha de dados (SDI)
#define TFT_SCLK_PIN                19  // Linha de relógio (SCK)
#define TFT_CS_PIN                  22  // Chip Select (TFT_CS)
#define TFT_DC_PIN                  9   // Data/Command Control
#define TFT_RST_PIN                 10  // Reset do Ecrã

#define COLOR_BLACK                 0x0000
#define COLOR_WHITE                 0xFFFF
#define COLOR_RED                   0xF800


void display_init(void);
void display_show_alert(void);
void display_clear(void);

#endif // DISPLAY_APP_H