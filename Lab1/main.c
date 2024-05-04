/*##############################################################################
Prof. Paulo Denis Garcez da Luz - 2023/2S
Desenvolvido para a placa EK-TM4C1294XL utilizando o SDK TivaWare no KEIL
Programa exemplo de blink usando 4 led, 2 botões e Periférico SysTick.
##############################################################################*/

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/uart.h"

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

int main(void)
{
  //estado de cada led
  bool Led1=false,Led2=false,Led3=false,Led4=false;
  //controle via programação de anti-debouncing
  bool bt1flag=false,bt2flag=false;
  //controle de tempo para cada led
  unsigned int led1time=0,led2time=0;
  //controle de tempo para cada botão
  unsigned int bt1time=0,bt2time=0;
  //Inicializar clock principal a 120MHz
  SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
  //executa configuração e inicialização do SysTick
  SetupSystick();  
  //habilitar gpio port N
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
  //aguardar o periférico ficar pronto para uso
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {/*Espera habilitar o port*/}
  //habilitar gpio port F
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  //aguardar o periférico ficar pronto para uso
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {/*Espera habilitar o port*/}
  //configura o pin_0 e pin_1 como saída
  //habilitar gpio port J
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
  //aguardar o periférico ficar pronto para uso
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)) {/*Espera habilitar o port*/}
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  //configura o pin_0 e pin_4 como saída
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
  //configura o pin_0 e pin_1 como entrada
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  //configura os pinos para 2mA como limite de corrente e com week pull-up
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  //executa configuração e inicialização do SysTick
  SetupUart();  
  //loop infinito
  while (1)
        {
          //descomente o printf para entender os valores lidos dos botões ... modo debug com termial I/O
          //printf("Port J: Pin_0: [%d] - Pin_1: [%d]\n",GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0),GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1));
          //tratamento do buffer serial
          if (rxbuffer[0]=='A' && rxbuffer[1]=='T' && rxbuffer[2]=='0' && rxbuffer[3]=='0')
             {
               rxbuffer[0]=0;
               UARTStringSend("AT00:OK", 7);
             }
          if (rxbuffer[0]=='A' && rxbuffer[1]=='T' && rxbuffer[2]=='0' && rxbuffer[3]=='1')
             {
               rxbuffer[0]=0;
               UARTStringSend("AT01:OK", 7);
             }
          //controle temporal do led1 - tempo 1s = 1000ms
          if (SysTicks1ms>=led1time)
             {
               //tempo do led recebe o tempo atual + o tempo futuro para entrar novamente no if
               led1time=SysTicks1ms+1000;
               //altera o estado do led ligado / desligado
               Led1=!Led1;
               //escreve o estado do led no pino
               GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, Led1<<1);
             }
          //controle temporal do led2 - tempo 0,5s = 500ms
          if (SysTicks1ms>=led2time)
             {
               //tempo do led recebe o tempo atual + o tempo futuro para entrar novamente no if
               led2time=SysTicks1ms+500;
               //altera o estado do led ligado / desligado
               Led2=!Led2;
               //escreve o estado do led no pino
               GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, Led2);
             }
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
                    //altera o estado do led ligado / desligado
                    Led3=!Led3;
                    //escreve o estado do led no pino
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, Led3<<4);
                    UARTStringSend("BOTAO_01_PRESS", 14);
	          }
	     }
          //ant-debouncig do botão 2
          if (bt2flag) 
	     {
	       //botão2 liberado !!!
	       if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)&&(SysTicks1ms>=bt2time))
	          {
	            //botão liberado
                    bt2flag=false;
	            //55ms para liberar estado do botão ... tempo anti-debouncing
                    bt2time=SysTicks1ms+55;				 
	          }
	     }
          else
	     {
	       //botão2 pressionado !!!
	       if ((GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)==0)&&(SysTicks1ms>=bt2time))
	          {
	            //botão pressionado
	            bt2flag=true;
	            //55ms para liberar estado do botão ... tempo anti-debouncing
	            bt2time=SysTicks1ms+55;				 
                    //altera o estado do led ligado / desligado
                    Led4=!Led4;
                    //escreve o estado do led no pino
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, Led4);
	          }
	     }
        }
}

//função de tratamento da interrupção do SysTick
void SysTickIntHandler(void)
{
  SysTicks1ms++;
}

//função para enviar string pela uart
void UARTStringSend(const uint8_t *String, uint32_t tamanho)
{
while (tamanho--) UARTCharPut(UART0_BASE, *String++);
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
