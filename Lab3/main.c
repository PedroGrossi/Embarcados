/*##############################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Desenvolvido para a placa EK-TM4C1294XL utilizando o SDK TivaWare no KEIL
##############################################################################*/

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#include "lcd2.h"

//variável que conta os ticks(1ms) - Volatile não permite o compilador otimizar o código 
static volatile unsigned int SysTicks1ms;
//buffer de rx ...
unsigned char rxbuffer[4];
//variável para receber o retorno do cfg do clk
uint32_t SysClock;

//Protótipos de funções criadas no programa, código depois do main
void SysTickIntHandler(void);
void SetupSystick(void);

void SetupGPIOs(void);
void SysTick_Wait1ms(uint32_t data);
void SysTick_Wait1us(uint32_t data);

int main(void)
{
	uint32_t i;
	char lcd_buffer[16];
	
	//Inicializar clock principal a 80MHz
  SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 80000000);
	//executa configuração e inicialização do SysTick
  SetupSystick();
	SetupGPIOs();
	//Inicializa o LCD
	LCD_Init();
	LCD_EspecialChar();
	sprintf(lcd_buffer,"ELF52-DaLuz:");
	LCD_Escreve_Inst(0x80);
	for (i=0;i<strlen(lcd_buffer);i++) LCD_Escreve_Dado(lcd_buffer[i]);
	LCD_Escreve_Dado(0x00);										//Caracter Especial
	LCD_Escreve_Dado(0x02);										//Caracter Especial
	LCD_Escreve_Dado(0x04);										//Caracter Especial
	LCD_Escreve_Dado(0x06);										//Caracter Especial
	
	//uint8_t Tecla;
	return 0;
};

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

void SetupGPIOs(void)
{
	//habilitar gpio port M
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
	//aguarda o periférico ficar pronto para uso
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)) {/*espera habilitar o port*/}
	//habilitar gpio port K
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
	//aguarda o periférico ficar pronto para uso
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)) {/*espera habilitar o port*/}
	//habilitar gpio port L
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
	//aguarda o periférico ficar pronto para uso	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)) {/*espera habilitar o port*/}
	
	//configura o pin_0, pin_1, pin_2 como saída
  GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
	//configura o pin_0, pin_1, pin_2, pin_3, pin_4, pin_5, pin_6, pin_7 como saída
  GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	
	//configura o pin_0, pin_1, pin_2, pin_3 como entrada
  GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	//configura os pinos para 2mA como limite de corrente e com week pull-up
  GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	//configura o pin_4, pin_5, pin_6, pin_7 como entrada
  GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	//configura os pinos para 2mA como limite de corrente e com week pull-up
  GPIOPadConfigSet(GPIO_PORTM_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

//Function to wait <data>ms -> não sei se funciona
void SysTick_Wait1ms(uint32_t data)
{
	//zera contagem do SysTick
  SysTicks1ms=0;
	while(1)
	{
		// contando até <data>ms
		if (SysTicks1ms>(data*80000))
			break;
	}
}

//Function to wait <data>us -> não sei se funciona
void SysTick_Wait1us(uint32_t data)
{
	//zera contagem do SysTick
  SysTicks1ms=0;
	while(1)
	{
		// contando até <data>us
		if (SysTicks1ms>(data*80))
			break;
	}
}

int PortL_Input(void)
{
	//controle via programação de anti-debouncing
	bool bt1flag=false;
	//controle de tempo para botão
  unsigned int bt1time=0;
	
	//ant-debouncig do botão 1
   if (bt1flag)
	 {
		 //Botão1 liberado !!!
	   if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)&&SysTicks1ms>=bt1time)
		 {
			 //botão liberado
       bt1flag=false;
	     //55ms para liberar estado do botão ... tempo anti-debouncing
       bt1time=SysTicks1ms+55;				 
     }
		}
	  else
		{
			//botão1 pressionado !!!
	    if ((GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)==0)&&(SysTicks1ms>=bt1time))
			{
				//botão pressionado
	      bt1flag=true;
	      //55ms para liberar estado do botão ... tempo anti-debouncing
	      bt1time=SysTicks1ms+55;
        return GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
			}
		}
	return -1;
}
