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

void andarElevador(char *rx, char *tx, char n_btc_in, char n_seq_up, unsigned long timerelevador, struct elevador esquerdo);
void botaoCabine(char *rx, char*tx, char n_btc_in, struct elevador esquerdo);
void botaoCorredorSobe(char *rx, char *tx, char n_seq_up, struct elevador esquerdo);
void botaoCorredorDesce(char *rx, char *tx, char n_seq_down, struct elevador esquerdo);
