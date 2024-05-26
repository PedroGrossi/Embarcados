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
//#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#include "teclado.h"
#include "lcd2.h"


//vari�vel que conta os ticks(1ms) - Volatile n�o permite o compilador otimizar o c�digo 
static volatile unsigned int SysTicks1ms;
//vari�vel para receber o retorno do cfg do clk
uint32_t SysClock;

//Prot�tipos de fun��es criadas no programa, c�digo depois do main
void SysTickIntHandler(void);
void SetupSystick(void);

void SysTick_Wait1ms(uint32_t data);
void SysTick_Wait1us(uint32_t data);
void PortJ_setup(void);
void PortK_setup(void);
void PortL_setup(void);
void PortM_setup(void);
void PortN_setup(void);
uint32_t PortJ_Input(void);

int main(void)
{
	uint32_t i;
	char lcd_buffer[16];
	uint8_t Tecla;
	
	//Inicializar clock principal a 80MHz
  SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 80000000);
	//executa configura��o e inicializa��o do SysTick
  SetupSystick();
	//executa o setup dos GPIOs
	PortJ_setup();
	PortK_setup();
	PortL_setup();
	PortM_setup();
	PortN_setup();
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
	while (1)
	{
		Tecla=VarreTeclado();
	  
		if (Tecla=='A'||Tecla=='B'||Tecla=='C'||Tecla=='D') GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x1);
    if (Tecla=='#'||Tecla=='1'||Tecla=='4'||Tecla=='7') GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x2);
    if (Tecla=='0'||Tecla=='2'||Tecla=='5'||Tecla=='8') GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x3);
    if (Tecla=='*'||Tecla=='3'||Tecla=='6'||Tecla=='9') GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x3);

	  if (Tecla==0) Tecla='?'; 
		
		LCD_Escreve_Inst(0xC0);
		sprintf(lcd_buffer,"Key: [%X]-[%c]", Tecla,Tecla);
	  for (i=0;i<strlen(lcd_buffer);i++) LCD_Escreve_Dado(lcd_buffer[i]);
    
		//Se a USR_SW2 estiver pressionada
		if (PortJ_Input() == 0x1)
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x1);
    //Se a USR_SW1 estiver pressionada
		else if (PortJ_Input() == 0x2)
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x2);
    //Se ambas estiverem pressionadas
		else if (PortJ_Input() == 0x0)
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x3);
    //Se nenhuma estiver pressionada
		else if (PortJ_Input() == 0x3)
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x0);
	}
};

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

//Function to wait <data>ms -> n�o sei se funciona
void SysTick_Wait1ms(uint32_t data)
{
	while(1)
		// contando at� <data>ms
		if (SysTicks1ms>(SysTicks1ms+(data*80000)))
			break;
}

//Function to wait <data>us -> n�o sei se funciona
void SysTick_Wait1us(uint32_t data)
{
	while(1)
		// contando at� <data>us
		if (SysTicks1ms>(SysTicks1ms+(data*80)))
			break;
}

//fun��o para setup GPIO J
void PortJ_setup(void)
{
	//habilitar gpio port J
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
  //aguardar o perif�rico ficar pronto para uso
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)) {/*Espera habilitar o port*/}
  GPIOPinTypeGPIOInput(GPIO_PORTN_BASE,
	                      GPIO_PIN_0 | GPIO_PIN_1);
	//configura os pinos para 2mA como limite de corrente e com week pull-up
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

//fun��o para setup GPIO K
void PortK_setup(void)
{
	//habilitar gpio port K
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
	//aguarda o perif�rico ficar pronto para uso
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)) {/*espera habilitar o port*/}
	//configura o pin_0, pin_1, pin_2, pin_3, pin_4, pin_5, pin_6, pin_7 como sa�da
  GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE,
	                      GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
}

//fun��o para setup GPIO L
void PortL_setup(void)
{
	//habilitar gpio port L
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
	//aguarda o perif�rico ficar pronto para uso	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)) {/*espera habilitar o port*/}
	//configura o pin_0, pin_1, pin_2, pin_3 como entrada
  GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, 
	                     GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	//configura os pinos para 2mA como limite de corrente e com week pull-up
  GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

//fun��o para setup GPIO M
void PortM_setup(void)
{
	//habilitar gpio port M
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
	//aguarda o perif�rico ficar pronto para uso
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)) {/*espera habilitar o port*/}
	//configura o pin_0, pin_1, pin_2 como sa�da
  GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE,
	                      GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
	//configura o pin_4, pin_5, pin_6, pin_7 como entrada
  GPIOPinTypeGPIOInput(GPIO_PORTM_BASE,
	                     GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	//configura os pinos para 2mA como limite de corrente e com week pull-up
  GPIOPadConfigSet(GPIO_PORTM_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

//fun��o para setup GPIO N
void PortN_setup(void)
{
	//habilitar gpio port N
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	//aguarda o perif�rico ficar pronto para uso	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {/*espera habilitar o port*/}
	//configura o pin_0, pin_1 como sa�da
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE,
	                      GPIO_PIN_0 | GPIO_PIN_1);
}

//Fun��o para verificar quais sw foram pressionados
uint32_t PortJ_Input(void)
{
	//controle via programa��o de anti-debouncing
	bool bt1flag=false, bt2flag=false;
	//controle de tempo para bot�o
  unsigned int bt1time=0,bt2time=0;
	//chaves pressionadas
	int32_t chaves = 0;
	
	//ant-debouncig do bot�o 1
	if (bt1flag)
	{
    //Bot�o1 liberado !!!
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)&&SysTicks1ms>=bt1time)
		{
			//bot�o liberado
			bt1flag=false;
	    //55ms para liberar estado do bot�o ... tempo anti-debouncing
      bt1time=SysTicks1ms+55;				 
		}
	}
  else
	{
		//bot�o1 pressionado !!!
		if ((GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)==0)&&(SysTicks1ms>=bt1time))
	  {
			//bot�o pressionado
			bt1flag=true;
			//55ms para liberar estado do bot�o ... tempo anti-debouncing
			bt1time=SysTicks1ms+55;				 
			//leitura do sw1
			chaves += 0x2;
		}
	}
	//ant-debouncig do bot�o 2
	if (bt2flag) 
	{
		//bot�o2 liberado !!!
		if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)&&(SysTicks1ms>=bt2time))
		{	
			//bot�o liberado
			bt2flag=false;
			//55ms para liberar estado do bot�o ... tempo anti-debouncing
			bt2time=SysTicks1ms+55;				 
		}
	}
	else
	{
		//bot�o2 pressionado !!!
		if ((GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)==0)&&(SysTicks1ms>=bt2time))
		{
			//bot�o pressionado
			bt2flag=true;
			//55ms para liberar estado do bot�o ... tempo anti-debouncing
			bt2time=SysTicks1ms+55;				 
			//leitura do sw2
			chaves += 0x1;
		}
	}
	return chaves;
}
