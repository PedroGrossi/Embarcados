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
#include "driverlib/i2c.h"

#include "relay_functions.h"
#include "uart_functions.h"

#include "i2c.h"
#include "ssd1306.h"
#include "bitmap.h"
#include "teclado.h"


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
void I2C0_config(void);
void I2C0_sendSingleByte(uint8_t slave_addr, char data);
void I2C0_sendMultipleBytes(uint8_t slave_addr, uint8_t numOfBytes, char by[]);
void I2C7_config(void);
void I2C7_sendSingleByte(uint8_t slave_addr, char data);
void I2C7_sendMultipleBytes(uint8_t slave_addr, uint8_t numOfBytes, char by[]);

void PortF_setup(void);
void PWM0_setup(void);
void PortL_setup(void);
void PortM_setup(void);
void PortN_setup(void);
void I2C0_UpdateRelayStatus(bool *statusRelays);
void pwmIntensity(unsigned char Tecla, char *ledIntensity);


int main(void)
{
	//variavel para o status dos reles -> inicializa com todos desligados
	bool statusRelays[3] = {false, false, false}; // Adaptado para 3 reles
	//variavel para tempo da UART
	unsigned int UARTtime;
	//variavel para intensidade do led PWM
	char ledIntensity[2] = "50";
	//controle de tempo para oled7:
	unsigned int oled7time=0, tecladotime=0;
	//controle de estado da tecla:
	unsigned char Tecla;
	//Configura o clock para utilizar o xtal interno de 16MHz com PLL para 80MHz
	SysClock = SysCtlClockFreqSet(SYSCTL_OSC_INT | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320, 80000000);
	//executa configuração e inicialização do SysTick
  SetupSystick();
	//executa o setup dos GPIOs
	PortF_setup();
	PortL_setup();
	PortM_setup();
	PortN_setup();
	//Inicializa periférico I2C0:
	I2C0_config();
	//config 128x64:	
	OLED0_Init();
	//Limpa Buffer do Display Oled0
	SSD1306_I2C0_cls();
	//Desenha a Logo da Utfpr no Buffer do Oled0
	SSD1306_I2C0_DrawBitmap(15,25,bitmap8,24,24); // relé desligado
	SSD1306_I2C0_DrawBitmap(54,25,bitmap8,24,24); // relé desligado
	SSD1306_I2C0_DrawBitmap(91,25,bitmap8,24,24); // relé desligado
	//Atualiza o Buffer para a tela do Oled0
	SSD1306_I2C0_UpdateScreen();
	//Inicializa periférico I2C7:
	I2C7_config();
	//config 128x32:
	OLED7_Init();
	//Limpa Display Oled7
	OLED7_clearDisplay();
	//Escreve String no Oled7 x,y I=invertido
	OLED7_sendStrXYI("Intensidade do", 0, 0);
	//Escreve String no Oled7 x,y I=invertido
	OLED7_sendStrXYI("brilho do LED:", 1, 0);
	//Escreve String no Oled7 x,y I=invertido
	OLED7_sendStrXYI("  %", 2, 6);
	//Escreve String no Oled7 x,y I=invertido
	OLED7_sendStrXYI("Tecla: [ ]", 3, 0);
	//Escreve Caractere na posição x,y I=invertido
	OLED7_sendCharXYI('?', 3, 8); // informar a tecla pressionada
	//executa configuração e inicialização da UART
  SetupUart();
	//loop infinito
  while (1)
	{
		// Aguarda comando pela uart
		if(UARTCharsAvail(UART0_BASE))
		{
			// Aguarda receber toda o comando
			UARTtime = SysTicks1ms;
			while (SysTicks1ms < UARTtime+3){ /* aguarda 3 ciclos de clock */ }; 
			// armazena comando no txbuffer respeitando tamanho do buffer
			UARTStringReceive(txbuffer, 4);
			// verifica comando recebido, caso seja um comando aparentemente valido tenta executar
			if (txbuffer[0] == '#') executaComando(txbuffer, statusRelays);
			// atualiza imagem dos reles
			I2C0_UpdateRelayStatus(statusRelays);
		}
		//Varre teclado Matricial 4x4
		Tecla=VarreTeclado();
	  //Caso nenhuma tecla
		if (Tecla==0) Tecla='?';
		//Determina intensidade do PWM caso tenha recebido uma tecla valida
		if (Tecla != '?' && Tecla != '#' && Tecla != '*') pwmIntensity(Tecla, ledIntensity);
		//controle temporal do teclado
		if (SysTicks1ms>=tecladotime)
    {
			//tempo do teclado recebe o tempo atual + o tempo futuro para entrar novamente no if (100ms)
      tecladotime=SysTicks1ms+100;
			//Escreve Caractere na posição x,y I=invertido
      OLED7_sendCharXYI(Tecla, 3, 8);
		}
		//controle temporal do oled7 - Atualização Intensidade Led no olde7
    if (SysTicks1ms>=oled7time)
    {
			//tempo do oled7 recebe o tempo atual + o tempo futuro para entrar novamente no if (100ms)
			oled7time=SysTicks1ms+100;
			//Escreve intensidade do led
			OLED7_sendStrXYI(ledIntensity, 2, 6);
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
  //clock 40MHz <=> SysTick deve contar 1ms=80k-1 do Systick_Counter - 12 trocas de contexto PP->IRQ - (1T Mov, 1T Movt, 3T LDR, 1T INC ... STR e IRQ->PP já não contabilizam atrasos para a variável)  
  SysTickPeriodSet(80000-1-12-6);
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

//função para setup GPIO L
void PortL_setup(void)
{
//habilitar gpio port L
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
//aguardar o periférico ficar pronto para uso
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)) {/*Espera habilitar o port*/}
//configura o pin_0, pin_1, pin_2 e pin_3 como entrada
GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
//configura os pinos para 2mA como limite de corrente e com week pull-up
GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

//função para setup GPIO M
void PortM_setup(void)
{
//habilitar gpio port M
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
//aguardar o periférico ficar pronto para uso
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)) {/*Espera habilitar o port*/}
//configura o pin_4, pin_5, pin_6 e pin_7 como saída
GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6  | GPIO_PIN_7);
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

void I2C0_config(void)
{
	// Configure I2C0 for pins PB2 and PB3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));  
	//Configuração da PORTB PIN2 e PIN3 - B.M.
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
	GPIOPinConfigure(GPIO_PB3_I2C0SDA);
	GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
	GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
	I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);
}

void I2C0_sendSingleByte(uint8_t slave_addr, char data)
{
	I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, false);
	I2CMasterDataPut(I2C0_BASE, data);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusy(I2C0_BASE));
}

void I2C0_sendMultipleBytes(uint8_t slave_addr, uint8_t numOfBytes, char by[])
{
	uint8_t i;
	I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, false);
	I2CMasterDataPut(I2C0_BASE, by[0]);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_BASE));
	for (i = 1; i < numOfBytes - 1; i++)
	{
		I2CMasterDataPut(I2C0_BASE, by[i]);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(I2C0_BASE));
	}
	I2CMasterDataPut(I2C0_BASE, by[numOfBytes - 1]);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C0_BASE));
}

void I2C0_UpdateRelayStatus(bool *statusRelays)
{
	//Limpa Buffer do Display Oled0
	SSD1306_I2C0_cls();
	if (statusRelays[0] == true) SSD1306_I2C0_DrawBitmap(15,25,bitmap9,24,24); // relé ligado
	else SSD1306_I2C0_DrawBitmap(15,25,bitmap8,24,24); // relé desligado
	if (statusRelays[1] == true) SSD1306_I2C0_DrawBitmap(54,25,bitmap9,24,24); // relé ligado
	else SSD1306_I2C0_DrawBitmap(54,25,bitmap8,24,24); // relé desligado
	if (statusRelays[2] == true) SSD1306_I2C0_DrawBitmap(91,25,bitmap9,24,24); // relé ligado
	else SSD1306_I2C0_DrawBitmap(91,25,bitmap8,24,24); // relé desligado
	//Atualiza o Buffer para a tela do Oled0
  SSD1306_I2C0_UpdateScreen();
}

void I2C7_config(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C7);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C7) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));  
	//Configuração da PORTD PIN0 e PIN1 - B.M.
	GPIOPinConfigure(GPIO_PD0_I2C7SCL);
	GPIOPinConfigure(GPIO_PD1_I2C7SDA);
	GPIOPinTypeI2CSCL(GPIO_PORTD_BASE, GPIO_PIN_0);
	GPIOPinTypeI2C(GPIO_PORTD_BASE, GPIO_PIN_1);
	I2CMasterInitExpClk(I2C7_BASE, SysCtlClockGet(), false);
}

void I2C7_sendSingleByte(uint8_t slave_addr, char data)
{
	I2CMasterSlaveAddrSet(I2C7_BASE, slave_addr, false);
	I2CMasterDataPut(I2C7_BASE, data);
	I2CMasterControl(I2C7_BASE, I2C_MASTER_CMD_SINGLE_SEND);
	while (I2CMasterBusy(I2C7_BASE));
}

void I2C7_sendMultipleBytes(uint8_t slave_addr, uint8_t numOfBytes, char by[])
{
	uint8_t i;
	I2CMasterSlaveAddrSet(I2C7_BASE, slave_addr, false);
	I2CMasterDataPut(I2C7_BASE, by[0]);
	I2CMasterControl(I2C7_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C7_BASE));
	for (i = 1; i < numOfBytes - 1; i++)
	{
		I2CMasterDataPut(I2C7_BASE, by[i]);
		I2CMasterControl(I2C7_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
		while (I2CMasterBusy(I2C7_BASE));
	}
	I2CMasterDataPut(I2C7_BASE, by[numOfBytes - 1]);
	I2CMasterControl(I2C7_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while (I2CMasterBusy(I2C7_BASE));
}

void pwmIntensity(unsigned char Tecla, char *ledIntensity)
{
	unsigned short pwm0Now;
	switch (Tecla)
	{
		// 0% de intensidade do pwm
		case '0':
			pwm0Now = 1;
			ledIntensity[0] = '0';
			ledIntensity[1] = '0';
			break;
		// 10% de intensidade do pwm
		case '1':
			pwm0Now = 2400/10;
			ledIntensity[0] = '1';
			ledIntensity[1] = '0';
			break;
		// 20% de intensidade do pwm
		case '2':
			pwm0Now = (2400/10)*2;
			ledIntensity[0] = '2';
			ledIntensity[1] = '0';
			break;
		// 30% de intensidade do pwm
		case '3':
			pwm0Now = (2400/10)*3;
			ledIntensity[0] = '3';
			ledIntensity[1] = '0';
			break;
		// 40% de intensidade do pwm
		case '4':
			pwm0Now = (2400/10)*4;
			ledIntensity[0] = '4';
			ledIntensity[1] = '0';
			break;
		// 50% de intensidade do pwm
		case '5':
			pwm0Now = (2400/10)*5;
			ledIntensity[0] = '5';
			ledIntensity[1] = '0';
			break;
		// 60% de intensidade do pwm
		case '6':
			pwm0Now = (2400/10)*6;
			ledIntensity[0] = '6';
			ledIntensity[1] = '0';
			break;
		// 70% de intensidade do pwm
		case '7':
			pwm0Now = (2400/10)*7;
			ledIntensity[0] = '7';
			ledIntensity[1] = '0';
			break;
		// 80% de intensidade do pwm
		case '8':
			pwm0Now = (2400/10)*8;
			ledIntensity[0] = '8';
			ledIntensity[1] = '0';
			break;
		// 90% de intensidade do pwm
		case '9':
			pwm0Now = (2400/10)*9;
			ledIntensity[0] = '9';
			ledIntensity[1] = '0';
			break;
		// +5% de intensidade do pwm
		case 'A':
			// Verifica a porcenagem atual e adiciona 5%
			pwm0Now = (2400/10)*(ledIntensity[0] - 48) + (2400/100)*(ledIntensity[1] - 48) + (2400/100)*5;
			ledIntensity[1] += 5;
			if (ledIntensity[1] > 57)
			{
				ledIntensity[0] += 1;
				ledIntensity[1] -= 10;
			}
			break;
		// -5% de intensidade do pwm
		case 'B':
			// Verifica a porcenagem atual e remove 5%
			pwm0Now = (2400/10)*(ledIntensity[0] - 48) + (2400/100)*(ledIntensity[1] - 48) - (2400/100)*5;
			ledIntensity[1] -= 5;
			if (ledIntensity[1] < 48)
			{
				ledIntensity[0] -= 1;
				ledIntensity[1] += 10;
			}
			break;
		// +1% de intensidade do pwm
		case 'C':
			// Verifica a porcenagem atual e adiciona 1%
			pwm0Now = (2400/10)*(ledIntensity[0] - 48) + (2400/100)*(ledIntensity[1] - 48) + (2400/100);
			ledIntensity[1] += 1;
			if (ledIntensity[1] > 57)
			{
				ledIntensity[0] += 1;
				ledIntensity[1] -= 10;
			}
			break;
		// -1% de intensidade do pwm
		case 'D':
			// Verifica a porcenagem atual e remove 1%
			pwm0Now = (2400/10)*(ledIntensity[0] - 48) + (2400/100)*(ledIntensity[1] - 48) - (2400/100);
			ledIntensity[1] -= 1;
			if (ledIntensity[1] < 48)
			{
				ledIntensity[0] -= 1;
				ledIntensity[1] += 10;
			}
			break;
	}
	// Caso o valor chegue/ultrapasse 100% => pwm0Now = 99%
	if (pwm0Now >= 2400) pwm0Now = (2400/100)*99;
	// Caso o valor chegue/ultrapasse 0% => pwm0Now = 1% ~= 0%
	else if (pwm0Now <= 0) pwm0Now = 1;
	// Configurando duty cycle do PWM0.
  PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, pwm0Now);
	// Configurando duty cycle do PWM1.
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, 2400-pwm0Now);
}
