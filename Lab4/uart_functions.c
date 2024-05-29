// uart_functions.c
// Desenvolvido para a placa EK-TM4C1294XL
// Funções relacionadas a UART
// Pedro Henrique Grossi da Silva
// 28/05/2024

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/uart.h"

//Protótipos de funções criadas no programa
void UARTStringSend(const uint8_t *String, uint32_t tamanho);
void UARTStringReceive(uint8_t *stringBuffer, uint8_t bufferSize);

//função para enviar string pela uart
void UARTStringSend(const uint8_t *String, uint32_t tamanho)
{
while (tamanho--) UARTCharPut(UART0_BASE, *String++);
}

//função para receber string pela uart
void UARTStringReceive(uint8_t *stringBuffer, uint8_t bufferSize)
{
	// recebe o char e avança para a proxima posição do buffer
	while((UARTCharsAvail(UART0_BASE)) && (bufferSize-- > 0)) *stringBuffer++ = UARTCharGetNonBlocking(UART0_BASE);
	//UARTStringSend(stringBuffer, 1);}
}
