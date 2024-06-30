/*##################################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Data = 20/06/2024
Desenvolvido para a placa Wemos LOLIN32 LITE utilizando Ambiente ARDUINO / FreeRTOS
##################################################################################*/
/* Biblioteca do Arduino */
#include <Arduino.h>

/* Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Libs desenvolvidas*/
#include "main.h" 
#include "elevatorResponse.h"

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
  /*MODIFICAÇÃO DO GABRIEL, É IDEIA SO*/
  /*A ideia aqui é a de criar uma função que é responsavel por criar uma task da maneira "segura" do professor.
    Uma vez que essa função existe, podemos chamar ela para criar a Task e faze-la funcionar ao finalizar o vInitialize.
    Essa ideia veio pois a todas as task estão rodando enquanto não precisam rodar e assim poderiamos criar e deletar tasks em tempo real durante a execução dos codigos*/
//void createTaskSecure();
/* Update: A ideia deu ruim apos uma rapida analise, mas a ideia de mover o Task creation para o final do vInitialize se mantem*/

/* Variáveis para armazenamento do handle das tasks */
TaskHandle_t task1Handle = NULL;
TaskHandle_t ElevatorResponseHandle = NULL;

/* Protótipos das Tasks */
void vTask1(void *pvParameters);
void vElevatorResponse(void *pvParameters);

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

  /* Criação das Tasks com maior prioridade para forçar troca de contexto para ela na ISR */
  xReturned = xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+512,NULL,1,&task1Handle);
  if (xReturned == pdFAIL)
  {
    Serial.println("Não foi possível criar a Task 1!");
    while(1);
  } 
  xReturned = xTaskCreate(vElevatorResponse,"ElevatorResponseTask",configMINIMAL_STACK_SIZE,NULL,1,&ElevatorResponseHandle);
  if (xReturned == pdFAIL)
  {
    Serial.println("Não foi possível criar a ElevatorResponseTask!");
    while(1);
  }  
}

/* loop-Task0 => recebe comandos pela serial + Inicializa o simulador + Verifia o status da porta */
void loop()
{
  /* Verifica se existe algum byte na porta serial, caso exista rotaciona buffer e inclui o ultimo byte recebido. */
  if (Serial.available()>=1)
  {
    last=Serial.read();
    for (i=0;i<12;i++) rx[i]=rx[i+1];
    rx[i]=last;
  }
  /* Verifica se o comando initialized foi recebido */
  initialized(rx, tx, esquerdo);
  /* Atualiza o status da porta */
  doorStatus(rx, tx, esquerdo);
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

/* vElevatorResponse => Armazena o andar atual do elevador -> Provavelmente deve ser uma ISR */
void vElevatorResponse(void *pvParameters)
{
  while (1)
  {
    /* Verifica andar do elevador */
    andarElevador(rx, tx, n_btc_in, n_seq_up, timerelevador, esquerdo);
    /* Atualiza se algum botão da cabine for pressionado */
    botaoCabine(rx, tx, n_btc_in, esquerdo);
    /* Atualiza se algum botão do corredor for pressionado */
    botaoCorredorSobe(rx, tx, n_seq_down, esquerdo);
    botaoCorredorDesce(rx, tx, n_seq_down, esquerdo);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void zerar_serial(void)
{
  rx[0]=0;rx[1]=0;rx[2]=0;rx[3]=0;rx[4]=0;rx[5]=0;rx[6]=0;rx[7]=0;rx[8]=0;rx[9]=0;rx[10]=0;rx[11]=0;rx[12]=0;
  tx[0]=0;tx[1]=0;tx[2]=0;tx[3]=0;
}
