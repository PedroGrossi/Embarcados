/*###########################################
ElevatorResponse.h
Desenvolvido para a placa WEMOS LOLIN32 LITE
Funções para interpretar as respostas do
simulador de elevador
29/06/2024
###########################################*/
#ifndef ELEVATORRESPONSE_H
#define ELEVATORRESPONSE_H
#endif

#include <stdint.h>

void initialized(char *rx, char *tx, struct elevador *esquerdo);
void doorStatus(char *rx, char *tx, struct elevador *esquerdo);
void floorVerify(char *rx, char *tx, char n_btc_in, char n_seq_up, unsigned long timerelevador, struct elevador *esquerdo);
int cabinButton(char *rx, char*tx, char n_btc_in, int floorTarget, struct elevador *esquerdo);
int hallwayUpButton(char *rx, char *tx, char n_seq_up, int floorTarget, struct elevador *esquerdo);
int hallwayDownButton(char *rx, char *tx, char n_seq_down, int floorTarget, struct elevador *esquerdo);
