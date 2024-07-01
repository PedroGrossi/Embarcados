/*##################################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Data = 01/07/2024
Desenvolvido para a placa Wemos LOLIN32 LITE utilizando Ambiente ARDUINO / FreeRTOS
##################################################################################*/
/* Biblioteca do Arduino */
#include <Arduino.h>

/* Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/* Libs desenvolvidas*/
#include "main.h" 
#include "elevatorResponse.h"
#include "elevatorController.h"

/* Mapeamento de Pinos */
#define ledDebug 22

struct elevador esquerdo;

/* var hw serial: */
char rx[13],tx[4],last;
/* contador genérico: */
int i;
/* flags de eventos: */
char flagcabine,flagcorredorup,flagcorredordown;
/* contadores de eventos: */
char n_btc_in,n_seq_up,n_seq_down;
/* temporizadores de sftw: */
unsigned long timerled,timerelevador;

/* Protótipo de funções */
void zerar_serial(void);

/* Variáveis para armazenamento do handle das tasks */
//TaskHandle_t task1Handle = NULL;
TaskHandle_t elevatorFloorHandle = NULL;
TaskHandle_t elevatorResponseHandle = NULL;
TaskHandle_t elevatorControllHandle = NULL;

/* Variáveis para armazenamento das Filasa */
QueueHandle_t xFila;

/* Protótipos das Tasks */
//void vTask1(void *pvParameters);
void vElevatorFloor(void *pvParameters);
void vElevatorResponse(void *pvParameters);
void vElevatorControll(void *pvParameters);

void setup() 
{
  zerar_serial();
  Serial.begin(115200);

  /* Zerar eventos na inicialização: */
  flagcabine=0;
  flagcorredorup=0;
  flagcorredordown=0;
  n_btc_in=0;
  n_seq_up=0;
  n_seq_down=0;

  /* Zerar elevador na inicialização */
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

  timerled=millis();
  timerelevador=millis();

  /* led para debug*/
  pinMode(ledDebug, OUTPUT);
  digitalWrite(ledDebug,LOW);

  /* criação da variável de retorno da criação da task */
  BaseType_t xReturned;

  /* Criação da Fila com 10 posições*/
  xFila = xQueueCreate(10,sizeof(int)); 
  if (xFila == NULL)
  {
    Serial.println("Nao foi possível criar a fila");
    while(1);
  }
  /*
  xReturned = xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE,NULL,1,&task1Handle);
  if (xReturned == pdFAIL)
  {
    Serial.println("Não foi possível criar a Task 1!");
    while(1);
  }*/
  xReturned = xTaskCreate(vElevatorFloor,"ElevatorFloorTask",configMINIMAL_STACK_SIZE,NULL,2,&elevatorFloorHandle);
  if (xReturned == pdFAIL)
  {
    Serial.println("Não foi possível criar a ElevatorFloorTask!");
    while(1);
  } 
  xReturned = xTaskCreate(vElevatorResponse,"ElevatorResponseTask",configMINIMAL_STACK_SIZE,NULL,1,&elevatorResponseHandle);
  if (xReturned == pdFAIL)
  {
    Serial.println("Não foi possível criar a ElevatorResponseTask!");
    while(1);
  }  
  xReturned = xTaskCreate(vElevatorControll,"ElevatorControllTask",configMINIMAL_STACK_SIZE,NULL,1,&elevatorControllHandle);
  if (xReturned == pdFAIL)
  {
    Serial.println("Não foi possível criar a ElevatorControllTask!");
    while(1);
  }
}

/* loop-Task0 => Inicializa o simulador + Verifia o status da porta */
void loop()
{
  /* Verifica se o comando initialized foi recebido */
  initialized(rx, tx, &esquerdo);
  /* Atualiza o status da porta */
  doorStatus(rx, tx, &esquerdo);

  vTaskDelay(pdMS_TO_TICKS(400));
}

/* vTask1 => inverte LED em intervalos de 200 ms */
/*
void vTask1(void *pvParameters)
{
  int i=0;
  while (1)
  {
    digitalWrite(ledDebug,!digitalRead(ledDebug));
    i++;
    if (i==5)
    {
      vTaskDelete(task1Handle);
    }
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
*/
/* vElevatorFloor => recebe comandos pela serial + verifica o andar do elevador */
void vElevatorFloor(void *pvParameters)
{
  while (1)
  {
    /* Verifica se existe algum byte na porta serial, caso exista rotaciona buffer e inclui o ultimo byte recebido. */
    if (Serial.available()>=1)
    {
      last=Serial.read();
      for (i=0;i<12;i++) rx[i]=rx[i+1];
      rx[i]=last;
    }
    /* Atualiza andar do elevador */
    floorVerify(rx, tx, n_btc_in, n_seq_up, timerelevador, &esquerdo);

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

/* vElevatorResponse => Atualiza o andar atual do elevador + Atualiza boões pressionados na cabine  +
Atualiza botões pressionados no corredor */
void vElevatorResponse(void *pvParameters)
{
  while (1)
  {
    /* Atualiza se algum botão da cabine for pressionado */
    cabinButton(rx, tx, n_btc_in, &esquerdo);

    /* Atualiza se algum botão do corredor for pressionado */
    hallwayUpButton(rx, tx, n_seq_down, &esquerdo);
    hallwayDownButton(rx, tx, n_seq_down, &esquerdo);

    /* Adiciona andares na fila */
    for(i=0;i<16;i++)
    {
      if(esquerdo.btc_in[i])
      {
        xQueueSend(xFila, &i,portMAX_DELAY);
        esquerdo.btc_in[i]=0;
      }
    }
    for(i=0;i<15;i++)
    {
      if(esquerdo.btc_up[i])
      {
        xQueueSend(xFila, &i,portMAX_DELAY);
        esquerdo.btc_up[i]=0;
      }
    }
    for(i=1;i<16;i++)
    {
      if(esquerdo.btc_down[i])
      {
        xQueueSend(xFila, &i,portMAX_DELAY);
        esquerdo.btc_down[i]=0;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(400));
  }
}

/* vElevatorControll => */
void vElevatorControll(void *pvParameters)
{
  while (1)
  {
    /* Se a porta estiver aberta (parado) e o proximo foi igual ao andar atual => buscar proximo na fila*/
    if (esquerdo.porta==0 && esquerdo.prox==esquerdo.andar)
    {
      /* Aguarda receber valor na fila e passa para o controlador */
      if (xQueueReceive(xFila,&esquerdo.prox,pdMS_TO_TICKS(50))==pdTRUE)
      {
        controller(tx, timerelevador, &esquerdo);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(400));
  }
}

void zerar_serial(void)
{
  rx[0]=0;rx[1]=0;rx[2]=0;rx[3]=0;rx[4]=0;rx[5]=0;rx[6]=0;rx[7]=0;rx[8]=0;rx[9]=0;rx[10]=0;rx[11]=0;rx[12]=0;
  tx[0]=0;tx[1]=0;tx[2]=0;tx[3]=0;
}
