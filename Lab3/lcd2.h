// Desenvolvido para o Shield Tiva EK-TM4C1294XL: LCD (1602B) + TECLADO Matricial (4x4) @DaLuz
// Projeto e Esquemático do Shield: http://www.elf52.daeln.com.br
// Prof. DaLuz
// Prof. Guilherme Peron
// Prof. Marcos P.

#ifndef LCD_H
#define LCD_H
#endif

#include <stdint.h>

void LCD_Init(void);
void LCD_Command(uint32_t data);
void LCD_Data(uint32_t data);
void LCD_Escreve_Inst(uint32_t dado);
void LCD_Escreve_Dado(uint32_t dado);
void LCD_EspecialChar(void);
