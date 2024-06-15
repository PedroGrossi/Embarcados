// uart_functions.c
// Desenvolvido para a placa EK-TM4C1294XL
// Fun��es relacionadas a UART
// Pedro Henrique Grossi da Silva
// 28/05/2024

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/uart.h"

//buffer de rx ... Variavel publica
extern unsigned char rxbuffer[4];


//Prot�tipos de fun��es criadas no programa
void UARTStringSend(const uint8_t *String, uint32_t tamanho);
void UART_Interruption_Handler(void);

//fun��o para enviar string pela uart
void UARTStringSend(const uint8_t *String, uint32_t tamanho)
{
while (tamanho--) UARTCharPut(UART0_BASE, *String++);
}

//fun��o de tratamento da interrup��o do uart
void UART_Interruption_Handler(void) 
{
  uint8_t last;
  //limpar IRQ exec
  UARTIntClear(UART0_BASE,UARTIntStatus(UART0_BASE,true));
  // Ler o pr�ximo caractere na uart.
  last = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
  //rotacionar buffer circular
  rxbuffer[0]=rxbuffer[1];
  rxbuffer[1]=rxbuffer[2];
  rxbuffer[2]=rxbuffer[3];
  rxbuffer[3]=last;
}
