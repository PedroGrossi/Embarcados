// relay_functions.c
// Desenvolvido para a placa EK-TM4C1294XL
// Controla reles (representados por leds)
// Pedro Henrique Grossi da Silva
// 28/05/2024

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"

#include "uart_functions.h"


//Prot�tipos de fun��es criadas no programa
int8_t executaComando(uint8_t *commandBuffer, bool *statusReles);
void AnsOneRele(uint8_t releNumber, bool releStatus);
void AnsStatus(bool *statusReles);

// Fun��o para executar comandos recebidos
// Retorna 0 se o comando foi executado, retorna -1 se o comando possui erros e/ou n�o foi executado
int8_t executaComando(uint8_t *commandBuffer, bool *statusReles)
{
	// variavel de retorno
	int8_t retorno = 0;
	// Verifica se o comando � para 1 unico rel�
	if (commandBuffer[1] == 'R')
	{
		// Verifica se o comando � valido
		if (commandBuffer[3] != '1' && commandBuffer[3] != '0') retorno = -1;
		// Comando valido (por hora)
		else
		{
			// Verifica rel� do comando
			switch (commandBuffer[2])
			{
				case '0':
					if (commandBuffer[3] == '1') statusReles[0] = true; // ligar rel� 0
					else statusReles[0] = false; //desligar rel� 0
					GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, statusReles[0]<<1); // atualiza o status do rel� 0
				  AnsOneRele(0, statusReles[0]); // envia resposta de atualiza��o
					break;
				case '1':
					if (commandBuffer[3] == '1') statusReles[1] = true; // ligar rel� 1
					else statusReles[1] = false; //desligar rel� 1
					GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, statusReles[1]); // atualiza o status do rel� 1
				  AnsOneRele(1, statusReles[1]); // envia resposta de atualiza��o
					break;
				case '2':
					if (commandBuffer[3] == '1') statusReles[2] = true; // ligar rel� 2
					else statusReles[2] = false; //desligar rel� 2
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, statusReles[2]<<4); // atualiza o status do rel� 2
				  AnsOneRele(2, statusReles[2]); // envia resposta de atualiza��o
					break;
				case '3':
					if (commandBuffer[3] == '1') statusReles[3] = true; // ligar rel� 3
					else statusReles[3] = false; //desligar rel� 3
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, statusReles[3]); // atualiza o status do rel� 3
				  AnsOneRele(3, statusReles[3]); // envia resposta de atualiza��o
					break;
				default:
					// Caso de led invalido -> comando invalido
					retorno = -1;
					break;
			}
		}
	}
	// Verifica se o comando � para todos os rel�s
	else if (commandBuffer[1] == 'T')
	{
		// Verifica se �  um comando para alterar o status
		if (commandBuffer[2] == 'X')
		{
			if (commandBuffer[3] == '1')
			{
				// ligar todos os rel�s
				statusReles[0] = true;
				statusReles[1] = true;
				statusReles[2] = true;
				statusReles[3] = true;
				// atualiza o status de todos os rel�s
				GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, statusReles[0]<<1);
				GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, statusReles[1]);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, statusReles[2]<<4);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, statusReles[3]);
				// envia resposta de atualiza��o
				UARTStringSend("@TX1", 4);
			}
			else if (commandBuffer[3] == '0')
			{
				// desligar todos os rel�s
				statusReles[0] = false;
				statusReles[1] = false;
				statusReles[2] = false;
				statusReles[3] = false;
				// atualiza o status de todos os rel�s
				GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, statusReles[0]<<1);
				GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, statusReles[1]);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, statusReles[2]<<4);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, statusReles[3]);
				// envia resposta de atualiza��o
				UARTStringSend("@TX0", 4);
			}
			else retorno = -1;
		}
		// Verifica se �  um comando de status
		else if (commandBuffer[2] == 'S')
		{
			// Envia status pela uart
			if (commandBuffer[3] == 'T') AnsStatus(statusReles);
			else retorno = -1;
		}
		else retorno = -1;
	}
	// Caso do comando invalido
	else retorno = -1;
	
	return retorno;
}

// Fun��o de resposta a comando que altera status de 1 unico rel�
// Envia resposta pela uart
void AnsOneRele(uint8_t releNumber, bool releStatus)
{
	if (releStatus == true)
	{
		if (releNumber == 0) UARTStringSend("@R01", 4);
		else if (releNumber == 1) UARTStringSend("@R11", 4);
		else if (releNumber == 2) UARTStringSend("@R21", 4);
		else if (releNumber == 3) UARTStringSend("@R31", 4);
	}
	else
	{
		if (releNumber == 0) UARTStringSend("@R00", 4);
		else if (releNumber == 1) UARTStringSend("@R10", 4);
		else if (releNumber == 2) UARTStringSend("@R20", 4);
		else if (releNumber == 3) UARTStringSend("@R30", 4);
	}
}

// Fun��o de respota para status dos reles
// Envia resposta pela uart
void AnsStatus(bool *statusReles)
{
	// Valor base
	uint8_t data = '@';
	// Incrementa base conforme status dos rel�s
	if (statusReles[0] == true) data += 8;
	if (statusReles[1] == true) data += 4;
	if (statusReles[2] == true) data += 2;
	if (statusReles[3] == true) data += 1;
	// Envia inicio da mesagem
	UARTStringSend("@TS", 3);
	// Envia status dos reles
	UARTStringSend(&data, 1);
}
