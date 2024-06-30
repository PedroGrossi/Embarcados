/*######################################################################
Aluno = Pedro Henrique Grossi da Silva
Data = 29/06/2024
Desenvolvido para a placa Wemos LOLIN32 LITE utilizando Ambiente ARDUINO
#######################################################################/
/* Biblioteca do Arduino */
#include <Arduino.h>


void controller(unsigned long timerelevador)
{
    if (millis()>timerelevador)
   {
     timerelevador=millis();
     //CÃ³digo de controle do elevador aqui ...
   }
//resetFunc();
}
