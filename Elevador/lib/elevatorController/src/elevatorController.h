/*##########################################
elevatorController.h
Desenvolvido para a placa WEMOS LOLIN32 LITE
Função de controlador do elevador
30/06/2024
##########################################*/
#ifndef RXCOMMANDS_H
#define RXCOMMANDS_H
#endif

#include <stdint.h>

void controller(char *tx, struct elevador *esquerdo, SemaphoreHandle_t xSerialMutex);
