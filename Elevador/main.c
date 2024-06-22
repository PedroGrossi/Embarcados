/*########################################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Data = 20/06/2024
Desenvolvido para a placa EK-TM4C1294XL utilizando o SDK TivaWare no KEIL e FreeRTOS
########################################################################################*/
/* TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare */
#define PART_TM4C1294NCPDT 1

/* Bibliotecas C */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Bibliotecas FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* Bibliotecas TivaWare */
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

/* buffer de rx ... */
unsigned char ucRxbuffer[4];
/* variável para cfg do clk */
uint32_t SystemCoreClock = 120000000U; /*!< System Clock Frequency (Core Clock)*/

/* Variáveis para armazenamento do handle das tasks */
TaskHandle_t taks1Handle = NULL;

/* Prototipo para tasks */
void vBlinkTask(void *pvParameters);

/* Prototipo para funcoes */
void SetupUart(void);
void UART_Interruption_Handler(void);
void UARTStringSend(const uint8_t *String, uint32_t tamanho);
void PortN_setup(void);

int main(void)
{
	/* setup */

	/* GPIO */
	PortN_setup();
	
	/* criação da variável de retorno da criação da task */
  BaseType_t xReturned;
	
	/* Criação das Tasks */
  xReturned = xTaskCreate(vBlinkTask,"vBlinkTask",configMINIMAL_STACK_SIZE,NULL,1,&taks1Handle);
  if (xReturned == pdFAIL)
	{
		UARTStringSend("Task 1 fail", 11);
    while(1);
	} 
		 
	/* loop */
	while (1)
	{
		vTaskDelay(333);
	}
}

/* vBlinkTask => inverte LED em intervalos de 2s */
void vBlinkTask(void *pvParameters)
{
	bool status = false;
	while (1)
	{
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, !status);
    vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

/* função para configurar e inicializar o periférico Uart a 115.2k,8,n,1 */
void SetupUart(void)
{
  /* Habilitar porta serial a 115200 com interrupção seguindo sequencia de inicializações abaixo: */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
  UARTConfigSetExpClk(UART0_BASE, SystemCoreClock, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	UARTFIFODisable(UART0_BASE);
  UARTIntEnable(UART0_BASE,UART_INT_RX);
  UARTIntRegister(UART0_BASE,UART_Interruption_Handler);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE,(GPIO_PIN_0|GPIO_PIN_1));
}

/* função de tratamento da interrupção do uart */
void UART_Interruption_Handler(void) 
{
  uint8_t last;
  /* limpar IRQ exec */
  UARTIntClear(UART0_BASE,UARTIntStatus(UART0_BASE,true));
  /* Ler o próximo caractere na uart. */
  last = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
  /* rotacionar buffer circular */
  ucRxbuffer[0]=ucRxbuffer[1];
  ucRxbuffer[1]=ucRxbuffer[2];
  ucRxbuffer[2]=ucRxbuffer[3];
  ucRxbuffer[3]=last;
}

//função para enviar string pela uart
void UARTStringSend(const uint8_t *String, uint32_t tamanho)
{
while (tamanho--) UARTCharPut(UART0_BASE, *String++);
}

//função para setup GPIO N
void PortN_setup(void)
{
	//habilitar gpio port N
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	//aguarda o periférico ficar pronto para uso	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {/*espera habilitar o port*/}
	//configura o pin_0 como saída
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
}
