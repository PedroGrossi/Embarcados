// uart_functions.c
// Desenvolvido para a placa EK-TM4C1294XL
// Funções relacionadas a UART
// Pedro Henrique Grossi da Silva
// 26/05/2024

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/uart.h"

//Protótipos de funções criadas no programa
void UARTStringSend(const uint8_t *String, uint32_t tamanho);
void UARTStringReceive(uint8_t *stringBuffer);

//função para enviar string pela uart
void UARTStringSend(const uint8_t *String, uint32_t tamanho)
{
while (tamanho--) UARTCharPut(UART0_BASE, *String++);
}

//função para receber string pela uart
void UARTStringReceive(uint8_t *stringBuffer)
{
	while(UARTCharGetNonBlocking(UART0_BASE))
	{
			// recebe o char
			*stringBuffer = UARTCharGetNonBlocking(UART0_BASE);
			// avança para a proxima posição do buffer
			stringBuffer++;
	}
}
