/* ##########################################
main.h
01/07/2024
###########################################*/

#ifndef MAIN_H
#define MAIN_H

struct elevador
{
  char andar;                   //Andar atual
  char estado;                  //Sudindo 'S', descendo 'D' e parado 'P'.
  char sentido;                 //Sentido do movimento => Subindo 'S' ou decendo 'D'
  char prox;                    //PrÃ³ximo andar ...
  char btc_up[15];              //BotÃµes do corredor up
  char btc_down[15];            //BotÃµes do corredor down
  char btc_in[16];              //BotÃµes da cabine interna
  char seq_up[16];              //Sequencia de atendimento dos botÃµes subida - "fila" de andares
  char seq_down[16];            //Sequencia de atendimento dos botÃµes descida - "fila" de andares
  char n_btc_in;                //numero de atendimentos na fila, botÃµes cabine
  char n_seq_up;                //numero de atendimentos na fila, botÃµes corredor up
  char n_seq_down;              //numero de atendimentos na fila, botÃµes corredor down
  char porta;                   //Porta aberta (0) ou fechada (1)
};

#endif

#include <stdint.h>

void zerar_serial(void);
