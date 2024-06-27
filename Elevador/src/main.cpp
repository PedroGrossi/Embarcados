/*########################################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Data = 20/06/2024
Desenvolvido para a placa Wemos LOLIN32 LITE utilizando ISR / Platformio / FreeRTOS
########################################################################################*/
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct elevador
{
  char andar;                   //Andar atual
  char estado;                  //Sudindo 'S', descendo 'D' e parado 'P'.
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
} esquerdo;

//var hw serial:
char rx[13],tx[4],last;
//contadores genÃ©ricos:
char i,j;
//contadores de eventos:
char n_btc_in,n_seq_up,n_seq_down;

//Protótipo de funções
void zerar_serial(void);

void setup() {
  zerar_serial();
  //Zerar elevador na inicialização
  esquerdo.andar=0;
  esquerdo.estado='P';
  esquerdo.prox=0;
  esquerdo.n_btc_in=0;
  esquerdo.n_seq_up=0;
  esquerdo.n_seq_down=0;
  for (i=15;i>=0;i--) esquerdo.btc_in[i]=0;
  for (i=15;i>=0;i--) esquerdo.btc_down[i]=0;
  for (i=15;i>=0;i--) esquerdo.btc_up[i]=0;
  for (i=14;i>=0;i--) esquerdo.seq_up[i]=0;
  for (i=14;i>=0;i--) esquerdo.seq_down[i]=0;
  /* configura comunicação serial com baudrate de 115200 */
  Serial.begin(115200);
}

/* loop-Task0 => inverte LED em intervalos de 333ms */
void loop()
{
  //Verifica se existe algum byte na porta serial, caso exista rotaciona buffer e inclui o ultimo byte recebido.
  if (Serial.available()>=1)
  {
    last=Serial.read();
    for (i=0;i<12;i++) rx[i]=rx[i+1];
    rx[i]=last;
  }
  //Recebeu fim de string do SIMSE2 -> tratar mensagem recebida:
  if (rx[11]=='\r'&&rx[12]=='\n')
  {
    //SIMSE2 -> Enviando msg de inicializaÃ§Ã£o:
    if (rx[0]=='i'&&rx[1]=='n'&&rx[2]=='i'&&rx[3]=='t'&&rx[4]=='i'&&rx[5]=='a'&&rx[6]=='l'&&rx[7]=='i'&&rx[8]=='z'&&rx[9]=='e'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
    {
      //Envia msg resete elevador esquerdo - delay para esperar o SIMSE2 abrir:
      delay(4000);//delay para SIMSE2 abrir: varia de PC para PC (1000-4000). SUPER Loop: Sem delay no programa principal ... aqui nao afeta o funcionamento apos init ...
      tx[0]='e';tx[1]='r';tx[2]='\r';
      Serial.write(tx,3);
      esquerdo.andar=0;
      esquerdo.estado='P';
      esquerdo.prox=0;
      esquerdo.n_btc_in=0;
      esquerdo.n_seq_up=0;
      esquerdo.n_seq_down=0;
      for (i=15;i>=0;i--) esquerdo.btc_in[i]=0;
      for (i=15;i>=0;i--) esquerdo.btc_down[i]=0;
      for (i=15;i>=0;i--) esquerdo.btc_up[i]=0;
      for (i=15;i>=0;i--) esquerdo.seq_up[i]=0;
      for (i=15;i>=0;i--) esquerdo.seq_down[i]=0;
      zerar_serial();
    }
    //SIMSE2 -> Enviando msg porta aberta:
    if (rx[9]=='e'&&rx[10]=='A'&&rx[11]=='\r'&&rx[12]=='\n')
    {
      esquerdo.porta=0;
      zerar_serial();
    }
    //SIMSE2 -> Enviando msg porta fechada:
    if (rx[9]=='e'&&rx[10]=='F'&&rx[11]=='\r'&&rx[12]=='\n')
    {
      if (esquerdo.estado=='S')
      {
        tx[0]='e';tx[1]='s';tx[2]='\r';
        Serial.write(tx,3);
      }
      if (esquerdo.estado=='D')
      {
        tx[0]='e';tx[1]='d';tx[2]='\r';
        Serial.write(tx,3);
      }
      esquerdo.porta=1;
      zerar_serial();
    }


    //SIMSE2 -> Enviando msg BotÃ£o corredor terreo sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='0'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
      if (esquerdo.andar!=0) esquerdo.btc_up[0]=1;                              //NÃ£o ascender botÃ£o no mesmo andar ...
          
      //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
      tx[0]='c';tx[1]='L';tx[2]='a';tx[3]='\r';Serial.write(tx,4);
          
      n_seq_up=0;                                                               //reset contadores de eventos ...
      zerar_serial();
    }
    //SIMSE2 -> Enviando msg BotÃ£o corredor 1 andar sobe:
    if (rx[6]=='e'&&rx[7]=='E'&&rx[8]=='0'&&rx[9]=='1'&&rx[10]=='s'&&rx[11]=='\r'&&rx[12]=='\n')
    {
      if (esquerdo.andar!=1) esquerdo.btc_up[1]=1;                    //NÃ£o ascender botÃ£o no mesmo andar ...
          
      //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
      tx[0]='c';tx[1]='L';tx[2]='b';tx[3]='\r';Serial.write(tx,4);
         
      n_seq_up=0;                                                               //reset contadores de eventos ...
      zerar_serial();
    }
  }
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void zerar_serial(void)
{
  rx[0]=0;rx[1]=0;rx[2]=0;rx[3]=0;rx[4]=0;rx[5]=0;rx[6]=0;rx[7]=0;rx[8]=0;rx[9]=0;rx[10]=0;rx[11]=0;rx[12]=0;
  tx[0]=0;tx[1]=0;tx[2]=0;tx[3]=0;
}
