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
#include "driverlib/pwm.h"

#include "relay_functions.h"
#include "uart_functions.h"


//variável que conta os ticks(1ms) - Volatile não permite o compilador otimizar o código 
static volatile unsigned int SysTicks1ms;
//buffer de rx ...
unsigned char rxbuffer[4];
//buffer de tx ...
unsigned char txbuffer[4];
//variável para receber o retorno do cfg do clk
uint32_t SysClock;

//Protótipos de funções criadas no programa, código depois do main
void SysTickIntHandler(void);
void SetupSystick(void);
void UART_Interruption_Handler(void);
void SetupUart(void);

void PortF_setup(void);
void PWM0_setup(void);
void PortN_setup(void);

int main(void)
{
	//variavel para o status dos reles -> inicializa com todos desligados
	bool statusReles[3] = {false, false, false}; // Adaptado para 3 reles
	//variavel para tempo
	unsigned int timeUART;
	//Inicializar clock principal a 120MHz
  SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
	//executa configuração e inicialização do SysTick
  SetupSystick();
	//executa o setup dos GPIOs
	PortF_setup();
	PortN_setup();
	//executa configuração e inicialização da UART
  SetupUart();
	//loop infinito
  while (1)
	{
			// Aguarda comando pela uart
			if(UARTCharsAvail(UART0_BASE))
			{
				// Aguarda receber toda o comando
				timeUART = SysTicks1ms;
				while (SysTicks1ms < timeUART+3){ /* aguarda 3 ciclos de clock */ }; 
				// armazena comando no txbuffer respeitando tamanho do buffer
				UARTStringReceive(txbuffer, 4);
				// verifica comando recebido, caso seja um comando aparentemente valido tenta executar
				if (txbuffer[0] == '#') executaComando(txbuffer, statusReles);
			}
	}
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

//função para configurar e inicializar o periférico Uart a 115.2k,8,n,1
void SetupUart(void)
{
  //Habilitar porta serial a 115200 com interrupção seguindo sequencia de inicializações abaixo:
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
  UARTConfigSetExpClk(UART0_BASE, SysClock, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
  UARTIntEnable(UART0_BASE,UART_INT_RX);
  UARTIntRegister(UART0_BASE,UART_Interruption_Handler);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE,(GPIO_PIN_0|GPIO_PIN_1));
}

//função para setup GPIO F como output
void PortF_setup(void)
{
	//habilitar gpio port F
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	//aguarda o periférico ficar pronto para uso	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {/*espera habilitar o port*/}
	//configura o pin_4 como saída
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);
	//configura o pin_0 como PWM
	PWM0_setup();
}

//função para setup PWM 0
void PWM0_setup(void)
{
	//habilitar pwm 1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	//aguarda o periférico ficar pronto para uso	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) {/*espera habilitar o pwm*/}
	//configura o pin_0 como PWM
	GPIOPinConfigure(GPIO_PF0_M0PWM0);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_0);
	
	// Configura PWM
	// Configura o gerador PWM
	PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
	// Configura o periodo. Para uma frequencia de 50 KHz, o periodo = 1/50,000, or 20us. 
	// Para um clock de 120 MHz, resulta em 2400 ticks.
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 2400);
	// Configurando o pulso de PWM0 para 50% de duty cycle.
  PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 1200);
	// Configurando o pulso de PWM1 para 50% de duty cycle.
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, 1200);
	// Inicia o timer no gerador em 0
  PWMGenEnable(PWM0_BASE, PWM_GEN_0);
	// Habilita a saida.
	PWMOutputState(PWM0_BASE, (PWM_OUT_0_BIT | PWM_OUT_1_BIT), true);
}

//função para setup GPIO N
void PortN_setup(void)
{
	//habilitar gpio port N
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	//aguarda o periférico ficar pronto para uso	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {/*espera habilitar o port*/}
	//configura o pin_0, pin_1 como saída
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}
