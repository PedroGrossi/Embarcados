// uart_functions.h
// Desenvolvido para a placa EK-TM4C1294XL
// Funções relacionadas a UART
// Pedro Henrique Grossi da Silva
// 28/05/2024

#ifndef UART_FUNCTIONS_H
#define UART_FUNCTIONS_H
#endif

#include <stdint.h>

void UARTStringSend(const uint8_t *String, uint32_t tamanho);
void UARTStringReceive(uint8_t *stringBuffer, uint8_t bufferSize);
