/*########################################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Data = 20/06/2024
Desenvolvido para a placa Wemos LOLIN32 LITE utilizando ISR / Platformio / FreeRTOS
########################################################################################*/
/* Biblioteca do Arduino */
#include <Arduino.h>

/* Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Mapeamento de Pinos */
#define ledDebug 22

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
int i;
//contadores de eventos:
char n_btc_in,n_seq_up,n_seq_down;
//temporizadores de sftw:
unsigned long timerled,timerelevador;

//Protótipo de funções
void zerar_serial(void);
  /*MODIFICAÇÃO DO GABRIEL, É IDEIA SO*/
  /*A ideia aqui é a de criar uma função que é responsavel por criar uma task da maneira "segura" do professor.
    Uma vez que essa função existe, podemos chamar ela para criar a Task e faze-la funcionar ao finalizar o vInitialize.
    Essa ideia veio pois a todas as task estão rodando enquanto não precisam rodar e assim poderiamos criar e deletar tasks em tempo real durante a execução dos codigos*/
//void createTaskSecure();
/* Update: A ideia deu ruim apos uma rapida analise, mas a ideia de mover o Task creation para o final do vInitialize se mantem*/

/* Variáveis para armazenamento do handle das tasks */
TaskHandle_t task1Handle = NULL;
TaskHandle_t InitializedHandle = NULL;
TaskHandle_t DoorStatusHandle = NULL;
TaskHandle_t FloorHandle = NULL;

/* Protótipos das Tasks */
void vTask1(void *pvParameters);
void vInitialized(void *pvParameters);
void vDoorStatus(void *pvParameters);
void vCurrentFloor(void *pvParameters);

void setup() {
  /* led to debug*/
  pinMode(ledDebug, OUTPUT);
  digitalWrite(ledDebug,LOW);

  zerar_serial();
  Serial.begin(115200);

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

  /* criação da variável de retorno da criação da task */
  BaseType_t xReturned;


  /* Criação das Tasks com maior prioridade para forçar troca de contexto para ela na ISR */
  xReturned = xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+512,NULL,1,&task1Handle);
  if (xReturned == pdFAIL)
     {
      Serial.println("Não foi possível criar a Task 1!");
      while(1);
     }
  xReturned = xTaskCreate(vInitialized,"InitializedTask",configMINIMAL_STACK_SIZE,NULL,1,&InitializedHandle);
  if (xReturned == pdFAIL)
     {
      Serial.println("Não foi possível criar a InitializedTask!");
      while(1);
     }  
  xReturned = xTaskCreate(vDoorStatus,"DoorStatusTask",configMINIMAL_STACK_SIZE,NULL,1,&DoorStatusHandle);
  if (xReturned == pdFAIL)
     {
      Serial.println("Não foi possível criar a DoorStatusTask!");
      while(1);
     }
  xReturned = xTaskCreate(vCurrentFloor,"CurrentFloorTask",configMINIMAL_STACK_SIZE,NULL,1,&FloorHandle);
  if (xReturned == pdFAIL)
     {
      Serial.println("Não foi possível criar a CurrentFloorTask!");
      while(1);
     }  
}

/* loop-Task0 */
void loop()
{
  //Verifica se existe algum byte na porta serial, caso exista rotaciona buffer e inclui o ultimo byte recebido.
  if (Serial.available()>=1)
  {
    last=Serial.read();
    for (i=0;i<12;i++) rx[i]=rx[i+1];
    rx[i]=last;
  }
  vTaskDelay(pdMS_TO_TICKS(200));
}

/* vTask1 => inverte LED em intervalos de 200 ms */
void vTask1(void *pvParameters)
{
  while (1)
        {
          digitalWrite(ledDebug,!digitalRead(ledDebug));
          vTaskDelay(pdMS_TO_TICKS(200));
        }
}

/* vInitialized => Inicializa o elevador */
void vInitialized(void *pvParameters)
{
  while (1)
        {
          //SIMSE2 -> Enviando msg de inicialização:
          if (rx[0]=='i'&&rx[1]=='n'&&rx[2]=='i'&&rx[3]=='t'&&rx[4]=='i'&&rx[5]=='a'&&rx[6]=='l'&&rx[7]=='i'&&rx[8]=='z'&&rx[9]=='e'&&rx[10]=='d'&&rx[11]=='\r'&&rx[12]=='\n')
          {
            //Envia msg resete elevador esquerdo - delay para esperar o SIMSE2 abrir:
            delay(5000);//delay para SIMSE2 abrir: varia de PC para PC (1000-4000). SUPER Loop: Sem delay no programa principal ... aqui nao afeta o funcionamento apos init ...
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
            /*Se auto exclui*/
            vTaskDelete(InitializedHandle);
          }
          vTaskDelay(pdMS_TO_TICKS(200));
        }
}

/* vDoorStatus => Verifica status da porta */
void vDoorStatus(void *pvParameters)
{
  while (1)
        {
          //SIMSE2 -> Enviando msg porta aberta:
          if (rx[9]=='e'&&rx[10]=='A'&&rx[11]=='\r'&&rx[12]=='\n')
          {
            esquerdo.porta=0;
            zerar_serial();
            vTaskDelete(task1Handle);
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
          vTaskDelay(pdMS_TO_TICKS(200));
        }
}

/* vCurrentFloor => Armazena o andar atual do elevador -> Provavelmente deve ser uma ISR */
void vCurrentFloor(void *pvParameters)
{
  while (1)
        {
          //SIMSE2 -> Enviando msg elevador no terreo:
          if (rx[9]=='e'&&rx[10]=='0'&&rx[11]=='\r'&&rx[12]=='\n')
          {
            esquerdo.andar=0;
            if (esquerdo.andar==esquerdo.prox)
            {
              esquerdo.btc_in[0]=0;
              esquerdo.btc_up[0]=0;
              timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
              tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
              esquerdo.estado='P';                                                  //estado=parado
              tx[0]='e';tx[1]='D';tx[2]='a';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
              tx[0]='c';tx[1]='D';tx[2]='a';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
              n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            }
            zerar_serial();
          }
          //SIMSE2 -> Enviando msg elevador no 1 andar:
          if (rx[9]=='e'&&rx[10]=='1'&&rx[11]=='\r'&&rx[12]=='\n')
          {
            esquerdo.andar=1;
            if (esquerdo.andar==esquerdo.prox)
            {
              esquerdo.btc_in[1]=0;
              if (esquerdo.estado=='S') esquerdo.btc_up[1]=0;
              if (esquerdo.estado=='D') esquerdo.btc_down[0]=0;
              timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
              tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
              esquerdo.estado='P';                                                  //estado=parado
              tx[0]='e';tx[1]='D';tx[2]='b';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
              tx[0]='c';tx[1]='D';tx[2]='b';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
              n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            }
            zerar_serial();
          }
          //SIMSE2 -> Enviando msg elevador no 2 andar:
          if (rx[9]=='e'&&rx[10]=='2'&&rx[11]=='\r'&&rx[12]=='\n')
          {
            esquerdo.andar=2;
            if (esquerdo.andar==esquerdo.prox)
            {
              esquerdo.btc_in[2]=0;
              if (esquerdo.estado=='S') esquerdo.btc_up[2]=0;
              if (esquerdo.estado=='D') esquerdo.btc_down[1]=0;
              timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
              tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
              esquerdo.estado='P';                                                  //estado=parado
              tx[0]='e';tx[1]='D';tx[2]='c';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
              tx[0]='c';tx[1]='D';tx[2]='c';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
              n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            }
            zerar_serial();
          }
          //SIMSE2 -> Enviando msg elevador no 3 andar:
          if (rx[9]=='e'&&rx[10]=='3'&&rx[11]=='\r'&&rx[12]=='\n')
          {
            esquerdo.andar=3;
            if (esquerdo.andar==esquerdo.prox)
            {
              esquerdo.btc_in[3]=0;
              if (esquerdo.estado=='S') esquerdo.btc_up[3]=0;
              if (esquerdo.estado=='D') esquerdo.btc_down[2]=0;
              timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
              tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
              esquerdo.estado='P';                                                  //estado=parado
              tx[0]='e';tx[1]='D';tx[2]='d';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
              tx[0]='c';tx[1]='D';tx[2]='d';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
              n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            }
            zerar_serial();
          }
          //SIMSE2 -> Enviando msg elevador no 4 andar:
          if (rx[9]=='e'&&rx[10]=='4'&&rx[11]=='\r'&&rx[12]=='\n')
          {
            esquerdo.andar=4;
            if (esquerdo.andar==esquerdo.prox)
            {
              esquerdo.btc_in[4]=0;
              if (esquerdo.estado=='S') esquerdo.btc_up[4]=0;
              if (esquerdo.estado=='D') esquerdo.btc_down[3]=0;
              timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
              esquerdo.estado='P';                                                  //estado=parado
              tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
              tx[0]='e';tx[1]='D';tx[2]='e';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
              tx[0]='c';tx[1]='D';tx[2]='e';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
              n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            }
            zerar_serial();
          }
          //SIMSE2 -> Enviando msg elevador no 5 andar:
          if (rx[9]=='e'&&rx[10]=='5'&&rx[11]=='\r'&&rx[12]=='\n')
          {
            esquerdo.andar=5;
            if (esquerdo.andar==esquerdo.prox)
            {
              esquerdo.btc_in[5]=0;
              if (esquerdo.estado=='S') esquerdo.btc_up[5]=0;
              if (esquerdo.estado=='D') esquerdo.btc_down[4]=0;
              timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
              tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
              esquerdo.estado='P';                                                  //estado=parado
              tx[0]='e';tx[1]='D';tx[2]='f';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
              tx[0]='c';tx[1]='D';tx[2]='f';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
              n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            }
            zerar_serial();
          }
          //SIMSE2 -> Enviando msg elevador no 6 andar:
          if (rx[9]=='e'&&rx[10]=='6'&&rx[11]=='\r'&&rx[12]=='\n')
          {
            esquerdo.andar=6;
            if (esquerdo.andar==esquerdo.prox)
            {
              esquerdo.btc_in[6]=0;
              if (esquerdo.estado=='S') esquerdo.btc_up[6]=0;
              if (esquerdo.estado=='D') esquerdo.btc_down[5]=0;
              timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
              tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
              esquerdo.estado='P';                                                  //estado=parado
              tx[0]='e';tx[1]='D';tx[2]='g';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
              tx[0]='c';tx[1]='D';tx[2]='g';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
              tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
              n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
            }
            zerar_serial();
          }
          //SIMSE2 -> Enviando msg elevador no 7 andar:
          if (rx[9]=='e'&&rx[10]=='7'&&rx[11]=='\r'&&rx[12]=='\n')
              {
                esquerdo.andar=7;
                if (esquerdo.andar==esquerdo.prox)
                  {
                      esquerdo.btc_in[7]=0;
                      if (esquerdo.estado=='S') esquerdo.btc_up[7]=0;
                      if (esquerdo.estado=='D') esquerdo.btc_down[6]=0;
                      timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
                      tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
                      esquerdo.estado='P';                                                  //estado=parado
                      tx[0]='e';tx[1]='D';tx[2]='h';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
                      tx[0]='c';tx[1]='D';tx[2]='h';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
                      n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
                  }
                zerar_serial();
              }
          //SIMSE2 -> Enviando msg elevador no 8 andar:
          if (rx[9]=='e'&&rx[10]=='8'&&rx[11]=='\r'&&rx[12]=='\n')
              {
                esquerdo.andar=8;
                if (esquerdo.andar==esquerdo.prox)
                  {
                      esquerdo.btc_in[8]=0;
                      if (esquerdo.estado=='S') esquerdo.btc_up[8]=0;
                      if (esquerdo.estado=='D') esquerdo.btc_down[7]=0;
                      timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
                      tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
                      esquerdo.estado='P';                                                  //estado=parado
                      tx[0]='e';tx[1]='D';tx[2]='i';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
                      tx[0]='c';tx[1]='D';tx[2]='i';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
                      n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
                  }
                zerar_serial();
              }
          //SIMSE2 -> Enviando msg elevador no 9 andar:
          if (rx[9]=='e'&&rx[10]=='9'&&rx[11]=='\r'&&rx[12]=='\n')
              {
                esquerdo.andar=9;
                if (esquerdo.andar==esquerdo.prox)
                  {
                      esquerdo.btc_in[9]=0;
                      if (esquerdo.estado=='S') esquerdo.btc_up[9]=0;
                      if (esquerdo.estado=='D') esquerdo.btc_down[8]=0;
                      timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
                      tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
                      esquerdo.estado='P';                                                  //estado=parado
                      tx[0]='e';tx[1]='D';tx[2]='j';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
                      tx[0]='c';tx[1]='D';tx[2]='j';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
                      n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
                  }
                zerar_serial();
              }
          //SIMSE2 -> Enviando msg elevador no 10 andar:
          if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='0'&&rx[11]=='\r'&&rx[12]=='\n')
              {
                esquerdo.andar=10;
                if (esquerdo.andar==esquerdo.prox)
                  {
                      esquerdo.btc_in[10]=0;
                      if (esquerdo.estado=='S') esquerdo.btc_up[10]=0;
                      if (esquerdo.estado=='D') esquerdo.btc_down[9]=0;
                      timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
                      tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
                      esquerdo.estado='P';                                                  //estado=parado
                      tx[0]='e';tx[1]='D';tx[2]='k';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
                      tx[0]='c';tx[1]='D';tx[2]='k';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
                      n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
                  }
                zerar_serial();
              }
          //SIMSE2 -> Enviando msg elevador no 11 andar:
          if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='1'&&rx[11]=='\r'&&rx[12]=='\n')
              {
                esquerdo.andar=11;
                if (esquerdo.andar==esquerdo.prox)
                  {
                      esquerdo.btc_in[11]=0;
                      if (esquerdo.estado=='S') esquerdo.btc_up[11]=0;
                      if (esquerdo.estado=='D') esquerdo.btc_down[10]=0;
                      timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
                      tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
                      esquerdo.estado='P';                                                  //estado=parado
                      tx[0]='e';tx[1]='D';tx[2]='l';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
                      tx[0]='c';tx[1]='D';tx[2]='l';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
                      n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
                  }
                zerar_serial();
              }
          //SIMSE2 -> Enviando msg elevador no 12 andar:
          if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='2'&&rx[11]=='\r'&&rx[12]=='\n')
              {
                esquerdo.andar=12;
                if (esquerdo.andar==esquerdo.prox)
                  {
                      esquerdo.btc_in[12]=0;
                      if (esquerdo.estado=='S') esquerdo.btc_up[12]=0;
                      if (esquerdo.estado=='D') esquerdo.btc_down[11]=0;
                      timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
                      tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
                      esquerdo.estado='P';                                                  //estado=parado
                      tx[0]='e';tx[1]='D';tx[2]='m';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
                      tx[0]='c';tx[1]='D';tx[2]='m';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
                      n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
                  }
                zerar_serial();
              }
          //SIMSE2 -> Enviando msg elevador no 13 andar:
          if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='3'&&rx[11]=='\r'&&rx[12]=='\n')
              {
                esquerdo.andar=13;
                if (esquerdo.andar==esquerdo.prox)
                  {
                      esquerdo.btc_in[13]=0;
                      if (esquerdo.estado=='S') esquerdo.btc_up[13]=0;
                      if (esquerdo.estado=='D') esquerdo.btc_down[12]=0;
                      tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
                      esquerdo.estado='P';                                                  //estado=parado
                      tx[0]='e';tx[1]='D';tx[2]='n';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
                      tx[0]='c';tx[1]='D';tx[2]='n';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
                      n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
                  }
                zerar_serial();
              }
          //SIMSE2 -> Enviando msg elevador no 14 andar:
          if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='4'&&rx[11]=='\r'&&rx[12]=='\n')
              {
                esquerdo.andar=14;
                if (esquerdo.andar==esquerdo.prox)
                  {
                      esquerdo.btc_in[14]=0;
                      if (esquerdo.estado=='S') esquerdo.btc_up[14]=0;
                      if (esquerdo.estado=='D') esquerdo.btc_down[13]=0;
                      timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
                      tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
                      esquerdo.estado='P';                                                  //estado=parado
                      tx[0]='e';tx[1]='D';tx[2]='o';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      //Envia msg ascender luz do botÃ£o 0 central como DEBUG ...
                      tx[0]='c';tx[1]='D';tx[2]='o';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
                      n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
                  }
                zerar_serial();
              }
          //SIMSE2 -> Enviando msg elevador no 15 andar:
          if (rx[8]=='e'&&rx[9]=='1'&&rx[10]=='5'&&rx[11]=='\r'&&rx[12]=='\n')
              {
                esquerdo.andar=15;
                if (esquerdo.andar==esquerdo.prox)
                  {
                      esquerdo.btc_in[15]=0;
                      esquerdo.btc_down[14]=0;
                      timerelevador=millis()+5000;                                          //parar elevador por 5s ... superloop waitflag ... timerelevador=5000...               
                      tx[0]='e';tx[1]='p';tx[2]='\r';Serial.write(tx,3);                    //para elevador
                      esquerdo.estado='P';                                                  //estado=parado
                      tx[0]='e';tx[1]='D';tx[2]='p';tx[3]='\r';Serial.write(tx,4);          //apagar botÃ£o cabine
                      tx[0]='e';tx[1]='a';tx[2]='\r';Serial.write(tx,3);                    //abrir porta
                      n_btc_in=0;n_seq_up=0;                                                //reset contadores de eventos ...
                  }
                zerar_serial();
              }
          vTaskDelay(pdMS_TO_TICKS(200));
        }
}

void zerar_serial(void)
{
  rx[0]=0;rx[1]=0;rx[2]=0;rx[3]=0;rx[4]=0;rx[5]=0;rx[6]=0;rx[7]=0;rx[8]=0;rx[9]=0;rx[10]=0;rx[11]=0;rx[12]=0;
  tx[0]=0;tx[1]=0;tx[2]=0;tx[3]=0;
}
