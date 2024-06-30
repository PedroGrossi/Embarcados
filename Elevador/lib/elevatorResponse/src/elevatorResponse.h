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

void initialized(char *rx, char *tx, struct elevador esquerdo);
void doorStatus(char *rx, char *tx, struct elevador esquerdo);
void andarElevador(char *rx, char *tx, char n_btc_in, char n_seq_up, unsigned long timerelevador, struct elevador esquerdo);
void botaoCabine(char *rx, char*tx, char n_btc_in, struct elevador esquerdo);
void botaoCorredorSobe(char *rx, char *tx, char n_seq_up, struct elevador esquerdo);
void botaoCorredorDesce(char *rx, char *tx, char n_seq_down, struct elevador esquerdo);
