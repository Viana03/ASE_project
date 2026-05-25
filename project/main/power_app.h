#ifndef POWER_APP_H
#define POWER_APP_H

#include <stdbool.h>

#define MPU_INT_PIN 4  // O pino que recebe o choque do sensor para acordar a placa

void power_entrar_deep_sleep(void);
bool power_acordou_por_roubo(void);

#endif // POWER_APP_H