# ASE_project

## Sistema Anti-Roubo Inteligente para Bicicletas/Trotinetes
Um alarme invisível que fica escondido na bicicleta. Se alguém mexer nela enquanto o dono está no café, dispara um alarme local e avisa o dono.

#### Requisitos:

- Sensor: Acelerómetro/Giroscópio (MPU6050).

- Atuador/Ecrã: Uma Sirene/Buzzer muito estridente (atuador).

- Rede: Wi-Fi (idealmente usariam LoRa ou GSM, mas Wi-Fi simulando uma rede pública ou o hotspot do telemóvel serve para a cadeira). Envia um alerta Push imediato para o telemóvel do dono.

- Baixo Consumo (O Foco): Ninguém quer carregar o alarme da bicicleta todos os dias. O microcontrolador fica em Deep Sleep. O MPU6050 tem um pino de interrupção Wake-on-Motion. Só quando a bicicleta sofre um abanão é que o sistema acorda, liga a sirene e manda o alerta.


##### Credentiais HIVEMQ
name - aseadmin
password - ASEadmin123