/*##############################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Desenvolvido para a placa EK-TM4C1294XL utilizando o SDK TivaWare no KEIL
##############################################################################*/

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"

#include "imageFunctions.h"
#include "images.h"

//variável que conta os ticks(1ms) - Volatile não permite o compilador otimizar o código 
static volatile unsigned int SysTicks1ms;
//buffer de rx ...
unsigned char rxbuffer[4];
//variável para receber o retorno do cfg do clk
uint32_t SysClock;

//Protótipos de funções criadas no programa, código depois do main
void SysTickIntHandler(void);
void UARTStringSend(const uint8_t *String, uint32_t tamanho);
void UART_Interruption_Handler(void);
void SetupSystick(void);
void SetupUart(void);
void UARTNumberSend(uint16_t number);

int main(void)
{
	//Inicializar clock principal a 120MHz
  SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
	//executa configuração e inicialização do SysTick
  SetupSystick();
	//executa configuração e inicialização do SysTick
  SetupUart();  
	
	// histogram array initialized with all values in 0
	static uint16_t histogram[256] = {0}; 
	// Estudar como aumentar o tamanho da pilha de variaveis locais -> startup

	// call function to image 0
	uint16_t image_size = EightBitHistogram_C(width0, height0, p_start_image0, histogram);
	
	if(image_size!=0){
		// send histogram data - image0
		UARTStringSend("X,Y\r\n", 5);
		for (uint16_t k=0; k<256; k++){
			if(histogram[k]!=0){
				UARTNumberSend(k);
				UARTStringSend(",", 1);
				UARTNumberSend(histogram[k]);
				UARTStringSend("\r\n", 2);
			};
		};
	}else{
		UARTStringSend("Image0 is bigger than 64K\r\n", 27);
	};
	
	// call function to image 1
	image_size = EightBitHistogram_C(width1, height1, p_start_image1, histogram);
	
	if(image_size!=0){
		// send histogram data - image0
		UARTStringSend("X,Y\r\n", 5);
		for (uint16_t k=0; k<256; k++){
			if(histogram[k]!=0){
				UARTNumberSend(k);
				UARTStringSend(",", 1);
				UARTNumberSend(histogram[k]);
				UARTStringSend("\r\n", 2);
			};
		};
	}else{
		UARTStringSend("Image1 is bigger than 64K\r\n", 27);
	};
	
	return 0;
};

//função para enviar string pela uart
void UARTStringSend(const uint8_t *String, uint32_t tamanho)
{
while (tamanho--) UARTCharPut(UART0_BASE, *String++);
}

//função para enviar numeros pela uart
void UARTNumberSend(uint16_t number)
{
	uint8_t buffer[4]; // buffer para armazenar a string
  buffer[0] = '0' + (number/100); // centenas
  buffer[1] = '0' + ((number/10)%10); // dezenas
  buffer[2] = '0' + (number%10); // unidades
  buffer[3] = '\0'; // terminador nulo
  UARTStringSend(buffer, 3); // enviar a string pela UART
}

//função de tratamento da interrupção do uart
void UART_Interruption_Handler(void) 
{
  uint8_t last;
  //limpar IRQ exec
  UARTIntClear(UART0_BASE,UARTIntStatus(UART0_BASE,true));
  // Ler o próximo caractere na uart.
  last = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
  //rotacionar buffer circular
  rxbuffer[0]=rxbuffer[1];
  rxbuffer[1]=rxbuffer[2];
  rxbuffer[2]=rxbuffer[3];
  rxbuffer[3]=last;
}

//função de tratamento da interrupção do SysTick
void SysTickIntHandler(void)
{
  SysTicks1ms++;
}

//função para configurar e inicializar o periférico Systick a 1ms
void SetupSystick(void)
{
  SysTicks1ms=0;
  //desliga o SysTick para poder configurar
  SysTickDisable();
  //clock 120MHz <=> SysTick deve contar 1ms=120k - 1 do Systick_Counter - 12 trocas de contexto PP->IRQ - (1T Mov, 1T Movt, 3T LDR, 1T INC ... STR e IRQ->PP já não contabilizam atrasos para a variável)  
  SysTickPeriodSet(120000-1-12-6);
  //registra a função de atendimento da interrupção
  SysTickIntRegister(SysTickIntHandler);
  //liga o atendimento via interrupção
  SysTickIntEnable();
  //liga novamente o SysTick
  SysTickEnable();
}

//função para configurar e inicializar o periférico Uart a 115.2k,8,n,1
void SetupUart(void)
{
  //Habilitar porta serial a 115200 com interrupção seguindo sequencia de inicializações abaixo:
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
