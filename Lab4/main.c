/*##############################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Data = 28/05/2024
Desenvolvido para a placa EK-TM4C1294XL utilizando o SDK TivaWare no KEIL
##############################################################################*/

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"

#include "relay_functions.h"
#include "uart_functions.h"


//vari�vel que conta os ticks(1ms) - Volatile n�o permite o compilador otimizar o c�digo 
static volatile unsigned int SysTicks1ms;
//buffer de rx ...
unsigned char rxbuffer[4];
//vari�vel para receber o retorno do cfg do clk
uint32_t SysClock;

//Prot�tipos de fun��es criadas no programa, c�digo depois do main
void SysTickIntHandler(void);
void SetupSystick(void);
void SetupUart(void);

void PortF_setup(void);
void PortN_setup(void);

int main(void)
{
	//variavel para o status dos reles -> inicializa com todos desligados
	bool statusReles[4] = {false, false, false, false};
	//variavel para tempo
	//unsigned int timeUART;
	//Inicializar clock principal a 120MHz
  SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
	//executa configura��o e inicializa��o do SysTick
  SetupSystick();
	//executa o setup dos GPIOs
	PortF_setup();
	PortN_setup();
	//executa configura��o e inicializa��o da UART
  SetupUart();
	//loop infinito
  while (1)
	{
		// verifica comando recebido, caso seja um comando aparentemente valido tenta executar
		if(rxbuffer[0] == '#')
		{
			//executa o comando recebido
			executaComando(rxbuffer, statusReles);
			//"limpa buffer"
			rxbuffer[0] = 0;
		}	
	}
}

//fun��o de tratamento da interrup��o do SysTick
void SysTickIntHandler(void)
{
  SysTicks1ms++;
}

//fun��o para configurar e inicializar o perif�rico Systick a 1ms
void SetupSystick(void)
{
  SysTicks1ms=0;
  //desliga o SysTick para poder configurar
  SysTickDisable();
  //clock 120MHz <=> SysTick deve contar 1ms=120k - 1 do Systick_Counter - 12 trocas de contexto PP->IRQ - (1T Mov, 1T Movt, 3T LDR, 1T INC ... STR e IRQ->PP j� n�o contabilizam atrasos para a vari�vel)  
  SysTickPeriodSet(120000-1-12-6);
  //registra a fun��o de atendimento da interrup��o
  SysTickIntRegister(SysTickIntHandler);
  //liga o atendimento via interrup��o
  SysTickIntEnable();
  //liga novamente o SysTick
  SysTickEnable();
}

//fun��o para configurar e inicializar o perif�rico Uart a 115.2k,8,n,1
void SetupUart(void)
{
  //Habilitar porta serial a 115200 com interrup��o seguindo sequencia de inicializa��es abaixo:
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
  UARTConfigSetExpClk(UART0_BASE, SysClock, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	UARTFIFODisable(UART0_BASE);
  UARTIntEnable(UART0_BASE,UART_INT_RX);
  UARTIntRegister(UART0_BASE,UART_Interruption_Handler);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE,(GPIO_PIN_0|GPIO_PIN_1));
}

//fun��o para setup GPIO F
void PortF_setup(void)
{
	//habilitar gpio port F
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	//aguarda o perif�rico ficar pronto para uso	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {/*espera habilitar o port*/}
	//configura o pin_0, pin_4 como sa�da
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
}

//fun��o para setup GPIO N
void PortN_setup(void)
{
	//habilitar gpio port N
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	//aguarda o perif�rico ficar pronto para uso	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {/*espera habilitar o port*/}
	//configura o pin_0, pin_1 como sa�da
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}
