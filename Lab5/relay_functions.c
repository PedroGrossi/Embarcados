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


//Protótipos de funções criadas no programa
int8_t executaComando(uint8_t *commandBuffer, bool *statusRelays);
void AnsOneRele(uint8_t relayNumber, bool relayStatus);
void AnsStatus(bool *statusRelays);

// Função para executar comandos recebidos
// Retorna 0 se o comando foi executado, retorna -1 se o comando possui erros e/ou não foi executado
int8_t executaComando(uint8_t *commandBuffer, bool *statusRelays)
{
	// variavel de retorno
	int8_t retorno = 0;
	// Verifica se o comando é para 1 unico relé
	if (commandBuffer[1] == 'R')
	{
		// Verifica se o comando é valido
		if (commandBuffer[3] != '1' && commandBuffer[3] != '0') retorno = -1;
		// Comando valido (por hora)
		else
		{
			// Verifica relé do comando
			switch (commandBuffer[2])
			{
				case '0':
					if (commandBuffer[3] == '1') statusRelays[0] = true; // ligar relé 0
					else statusRelays[0] = false; //desligar relé 0
					GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, statusRelays[0]<<1); // atualiza o status do relé 0
				  AnsOneRele(0, statusRelays[0]); // envia resposta de atualização
					break;
				case '1':
					if (commandBuffer[3] == '1') statusRelays[1] = true; // ligar relé 1
					else statusRelays[1] = false; //desligar relé 1
					GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, statusRelays[1]); // atualiza o status do relé 1
				  AnsOneRele(1, statusRelays[1]); // envia resposta de atualização
					break;
				case '2':
					if (commandBuffer[3] == '1') statusRelays[2] = true; // ligar relé 2
					else statusRelays[2] = false; //desligar relé 2
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, statusRelays[2]<<4); // atualiza o status do relé 2
				  AnsOneRele(2, statusRelays[2]); // envia resposta de atualização
					break;
				default:
					// Caso de led invalido -> comando invalido
					retorno = -1;
					break;
			}
		}
	}
	// Verifica se o comando é para todos os relés
	else if (commandBuffer[1] == 'T')
	{
		// Verifica se é  um comando para alterar o status
		if (commandBuffer[2] == 'X')
		{
			if (commandBuffer[3] == '1')
			{
				// ligar todos os relés
				statusRelays[0] = true;
				statusRelays[1] = true;
				statusRelays[2] = true;
				// atualiza o status de todos os relés
				GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, statusRelays[0]<<1);
				GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, statusRelays[1]);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, statusRelays[2]<<4);
				// envia resposta de atualização
				UARTStringSend("@TX1", 4);
			}
			else if (commandBuffer[3] == '0')
			{
				// desligar todos os relés
				statusRelays[0] = false;
				statusRelays[1] = false;
				statusRelays[2] = false;
				// atualiza o status de todos os relés
				GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, statusRelays[0]<<1);
				GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, statusRelays[1]);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, statusRelays[2]<<4);
				// envia resposta de atualização
				UARTStringSend("@TX0", 4);
			}
			else retorno = -1;
		}
		// Verifica se é  um comando de status
		else if (commandBuffer[2] == 'S')
		{
			// Envia status pela uart
			if (commandBuffer[3] == 'T') AnsStatus(statusRelays);
			else retorno = -1;
		}
		else retorno = -1;
	}
	// Caso do comando invalido
	else retorno = -1;
	
	return retorno;
}

// Função de resposta a comando que altera status de 1 unico relé
// Envia resposta pela uart
void AnsOneRele(uint8_t relayNumber, bool relayStatus)
{
	if (relayStatus == true)
	{
		if (relayNumber == 0) UARTStringSend("@R01", 4);
		else if (relayNumber == 1) UARTStringSend("@R11", 4);
		else if (relayNumber == 2) UARTStringSend("@R21", 4);
	}
	else
	{
		if (relayNumber == 0) UARTStringSend("@R00", 4);
		else if (relayNumber == 1) UARTStringSend("@R10", 4);
		else if (relayNumber == 2) UARTStringSend("@R20", 4);
	}
}

// Função de respota para status dos reles
// Envia resposta pela uart
void AnsStatus(bool *relayStatus)
{
	// Valor base
	uint8_t data = '@';
	// Incrementa base conforme status dos relés
	if (relayStatus[0] == true) data += 8;
	if (relayStatus[1] == true) data += 4;
	if (relayStatus[2] == true) data += 2;
	// Envia inicio da mesagem
	UARTStringSend("@TS", 3);
	// Envia status dos reles
	UARTStringSend(&data, 1);
}
