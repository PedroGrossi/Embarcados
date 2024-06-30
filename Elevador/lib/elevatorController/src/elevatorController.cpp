/*######################################################################
Aluno = Pedro Henrique Grossi da Silva
Data = 29/06/2024
Desenvolvido para a placa Wemos LOLIN32 LITE utilizando Ambiente ARDUINO
#######################################################################/
/* Biblioteca do Arduino */
#include <Arduino.h>

/* Libs desenvolvidas*/
#include "..\..\..\src\main.h"


void controller(char *tx, int floorTarget, unsigned long timerelevador, struct elevador *esquerdo)
{
    if (millis()>timerelevador)
   {
      timerelevador=millis();
      /* Define o proximo andar do elevador */
      esquerdo->prox=floorTarget;
      if(esquerdo->andar<esquerdo->prox)
      {
        /* Elevador sobe */
        esquerdo->estado='S';
      }
      else
      {
        /* Elevador desce */
        esquerdo->estado='D';
      }
      /* Fecha a porta do elevador*/
      esquerdo->porta=1;
      tx[0]='e';tx[1]='f';tx[2]='\r';
      Serial.write(tx,3);
      zerar_serial();
   }
}
