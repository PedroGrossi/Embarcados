/*##########################################
elevatorController.h
Desenvolvido para a placa WEMOS LOLIN32 LITE
Função de controlador do elevador
29/06/2024
##########################################*/
#ifndef RXCOMMANDS_H
#define RXCOMMANDS_H
#endif

#include <stdint.h>

void controller(char *tx, int floorTarget, unsigned long timerelevador, struct elevador *esquerdo);
