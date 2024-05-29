/*##############################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Desenvolvido para a placa EK-TM4C1294XL utilizando o SDK TivaWare no KEIL
##############################################################################*/

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include <stdint.h>
//#include <stdio.h>
#include <stdbool.h>
//#include <string.h>
//#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
//#include "driverlib/gpio.h"
//#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"


//vari�vel que conta os ticks(1ms) - Volatile n�o permite o compilador otimizar o c�digo 
static volatile unsigned int SysTicks1ms;
//vari�vel para receber o retorno do cfg do clk
uint32_t SysClock;

//Prot�tipos de fun��es criadas no programa, c�digo depois do main
void SysTickIntHandler(void);
void SetupSystick(void);

int main(void)
{
	//Inicializar clock principal a 120MHz
  SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
	//executa configura��o e inicializa��o do SysTick
  SetupSystick();
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
