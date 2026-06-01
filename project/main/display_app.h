#ifndef DISPLAY_APP_H
#define DISPLAY_APP_H

// Pinos SPI para o Ecrã TFT
#define TFT_MISO_PIN                20  // Linha de dados (MISO) - Não usada neste display, mas definida para referência
#define TFT_MOSI_PIN                19  // Linha de dados (SDA/MOSI)
#define TFT_SCLK_PIN                21  // Linha de relógio (SCK)
#define TFT_CS_PIN                  22  // Chip Select (TCS)
#define TFT_DC_PIN                  2   // Data/Command (DC)
#define TFT_RST_PIN                 3   // Reset (RST)
#define TFT_BL_PIN                  15  // Backlight (Podes meter -1 se não usarem)

void display_init(void);
void display_show_alert(void);
void display_show_locked(void);
void display_show_unlocked(void);
void display_clear(void);

#endif // DISPLAY_APP_H