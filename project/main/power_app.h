#ifndef POWER_APP_H
#define POWER_APP_H

#include <stdbool.h>

#define MPU_INT_PIN 4  // Pino de interrupção do sensor de movimento
#define BUTTON_PIN  0  // Pino do botão de armar/desarmar

void power_entrar_deep_sleep(void);
bool power_acordou_por_roubo(void);
bool power_acordou_por_botao(void);
bool power_get_armado(void);
void power_toggle_armado(void);

#endif // POWER_APP_H
