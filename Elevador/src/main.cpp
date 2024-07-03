/*##################################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Data = 03/07/2024
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
TaskHandle_t task1Handle = NULL;
TaskHandle_t elevatorFloorHandle = NULL;
TaskHandle_t elevatorResponseHandle = NULL;
TaskHandle_t elevatorControllHandle = NULL;

/* Variáveis para armazenamento das Filas */
QueueHandle_t xFilaSubida;
QueueHandle_t xFilaDecida;

/* Variavel para Mutex => gatekeeper */
SemaphoreHandle_t xSerialMutex;

/* Protótipos das Tasks */
void vTask1(void *pvParameters);
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
  esquerdo.sentido='S';
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

  /* Criação das Filas com 10 posições cada*/
  xFilaSubida = xQueueCreate(10,sizeof(int)); 
  if (xFilaSubida == NULL)
  {
    Serial.println("Nao foi possível criar a fila de subida");
    while(1);
  }
  xFilaDecida = xQueueCreate(10,sizeof(int)); 
  if (xFilaDecida == NULL)
  {
    Serial.println("Nao foi possível criar a fila de decida");
    while(1);
  }

  /* Criação do Mutex */
  xSerialMutex  = xSemaphoreCreateMutex(); 
  if (xFilaDecida == NULL)
  {
    Serial.println("Nao foi possível criar a fila de decida");
    while(1);
  }

  /* Criação das Tasks */
  xReturned = xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE,NULL,1,&task1Handle);
  if (xReturned == pdFAIL)
  {
    Serial.println("Não foi possível criar a Task 1!");
    while(1);
  }
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
  initialized(rx, tx, &esquerdo, xSerialMutex);
  /* Atualiza o status da porta */
  doorStatus(rx, tx, &esquerdo, xSerialMutex);

  vTaskDelay(pdMS_TO_TICKS(500));
}

/* vTask1 => inverte LED em intervalos de 200 ms  (Utilizando para verificar visualmente se o SO está funcionando)*/
void vTask1(void *pvParameters)
{
  int i=0;
  while (1)
  {
    digitalWrite(ledDebug,!digitalRead(ledDebug));
    i++;
    if (i==5) vTaskDelete(task1Handle);

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

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
    floorVerify(rx, tx, n_btc_in, n_seq_up, &timerelevador, &esquerdo, xSerialMutex);

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/* vElevatorResponse => Atualiza o andar atual do elevador + Atualiza boões pressionados na cabine  +
Atualiza botões pressionados no corredor */
void vElevatorResponse(void *pvParameters)
{
  while (1)
  {
    /* Atualiza se algum botão da cabine for pressionado */
    cabinButton(rx, tx, n_btc_in, &esquerdo, xSerialMutex);

    /* Atualiza se algum botão do corredor for pressionado */
    hallwayUpButton(rx, tx, n_seq_down, &esquerdo, xSerialMutex);
    hallwayDownButton(rx, tx, n_seq_down, &esquerdo, xSerialMutex);

    /* Adiciona andares na fila */
    for(i=0;i<16;i++)
    {
      if(esquerdo.btc_in[i])
      {
        /* Se o elevador estiver parado e o andar solicitado for abaixo => fila de decida */
        if(esquerdo.estado=='P' && i<esquerdo.andar) xQueueSend(xFilaDecida, &i,portMAX_DELAY);
        /* Se o elevador estiver parado e o andar solicitado for acima => fila de subida */
        if(esquerdo.estado=='P' && i>esquerdo.andar) xQueueSend(xFilaSubida, &i,portMAX_DELAY);
        /* Se o elevador estiver subindo e o andar solicitado for abaixo ou o andar passante => fila de decida */
        if(esquerdo.estado=='S' && i<esquerdo.andar) xQueueSend(xFilaDecida, &i,portMAX_DELAY);
        /* Se o elevador estiver subindo e o andar solicitado for acima => fila de subida */
        if(esquerdo.estado=='S' && i>esquerdo.andar)
        {
          /* Se o andar solicitado for antes do andar alvo */
          if(i<esquerdo.prox)
          {
            /* Troca o andar alvo */
            char aux=i;
            i=esquerdo.prox;
            esquerdo.prox=aux;
            xQueueSend(xFilaSubida, &i,portMAX_DELAY);
            /* Retornando o i para o valor original*/
            i=aux;
          }
          else
          {
            xQueueSend(xFilaSubida, &i,portMAX_DELAY);
          }
        }
        /* Se o elevador estiver decendo e o andar solicitado for acima ou o andar passante => fila de subida */
        if(esquerdo.estado=='D' && i>esquerdo.andar) xQueueSend(xFilaSubida, &i,portMAX_DELAY);
        /* Se o elevador estiver decendo e o andar solicitado for abaixo => fila de decida */
        if(esquerdo.estado=='D' && i<esquerdo.andar)
        {
          /* Se o andar solicitado for antes do andar alvo */
          if(i>esquerdo.prox)
          {
            /* Troca o andar alvo */
            char aux=i;
            i=esquerdo.prox;
            esquerdo.prox=aux;
            xQueueSend(xFilaDecida, &i,portMAX_DELAY);
            /* Retornando o i para o valor original*/
            i=aux;
          }
          else
          {
            xQueueSend(xFilaDecida, &i,portMAX_DELAY);
          }
        }
        esquerdo.btc_in[i]=0;
      }
    }
    for(i=0;i<15;i++)
    {
      if(esquerdo.btc_up[i])
      {
        /* Se o andar solicitado for acima => fila de subida */
        if(i>esquerdo.andar)
        {
          /* Se o andar solicitado for antes do andar alvo */
          if(i<esquerdo.prox)
          {
            /* Troca o andar alvo */
            char aux=i;
            i=esquerdo.prox;
            esquerdo.prox=aux;
            xQueueSend(xFilaSubida, &i,portMAX_DELAY);
            /* Retornando o i para o valor original*/
            i=aux;
          }
          else
          {
            xQueueSend(xFilaSubida, &i,portMAX_DELAY);
          }
        }
        else
        {
          xQueueSend(xFilaSubida, &i,portMAX_DELAY);
        }
        esquerdo.btc_up[i]=0;
      }
    }
    for(i=0;i<15;i++)
    {
      if(esquerdo.btc_down[i])
      {
        i++;
        /* Se o andar solicitado for abaixo => fila de decida */
        if(i<esquerdo.andar)
        {
          /* Se o andar solicitado for antes do andar alvo */
          if(i>esquerdo.prox)
          {
            /* Troca o andar alvo */
            char aux=i;
            i=esquerdo.prox;
            esquerdo.prox=aux;
            xQueueSend(xFilaDecida, &i,portMAX_DELAY);
            /* Retornando o i para o valor original*/
            i=aux;
          }
          else
          {
            xQueueSend(xFilaDecida, &i,portMAX_DELAY);
          }
        }
        else
        {
          xQueueSend(xFilaDecida, &i,portMAX_DELAY);
        }
        i--;
        esquerdo.btc_down[i]=0;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(400));
  }
}

/* vElevatorControll */
void vElevatorControll(void *pvParameters)
{
  while (1)
  {
    /* Se o elevador estiver parado e o proximo foi igual ao andar atual => buscar proximo na fila*/
    if (esquerdo.estado=='P' && esquerdo.prox==esquerdo.andar)
    {
      /* Se o elevador estiver com sentido de subida */
      if(esquerdo.sentido=='S')
      { 
        /* possui elementos na fila de subida */
        if(xQueueReceive(xFilaSubida,&esquerdo.prox,pdMS_TO_TICKS(50))==pdTRUE)
        {
          /* o elemento recebido é acima do andar atual*/
          if(esquerdo.prox>esquerdo.andar)
          {
            while(millis()<timerelevador) vTaskDelay(pdMS_TO_TICKS(100));
            controller(tx, &esquerdo, xSerialMutex);
          } 
          else
          {
            /* Devolve o elemento para a fila */
            xQueueSend(xFilaSubida, &esquerdo.prox,portMAX_DELAY);
            /* Muda o sentido do elevador */
            esquerdo.sentido='D';
          }
        }
        /* Muda o sentido do elevador */
        else 
        {
          esquerdo.sentido='D';
        }
      }

      /* Se o elevador estiver com sentido de decida */
      if(esquerdo.sentido=='D')
      {
        /* possui elementos na fila de decida */
        if(xQueueReceive(xFilaDecida,&esquerdo.prox,pdMS_TO_TICKS(50))==pdTRUE)
        {
          /* O elemento recebido for abaixo do andar atual*/
          if(esquerdo.prox<esquerdo.andar)
          {
            while(millis()<timerelevador) vTaskDelay(pdMS_TO_TICKS(100));
            controller(tx, &esquerdo, xSerialMutex);
          } 
          else
          {
            /* Devolve o elemento para a fila */
            xQueueSend(xFilaDecida, &esquerdo.prox,portMAX_DELAY);
            /* Muda o sentido do elevador */
            esquerdo.sentido='S';
          }
        }
        /* Muda o sentido do elevador */
        else 
        {
          esquerdo.sentido='S';
        }
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
