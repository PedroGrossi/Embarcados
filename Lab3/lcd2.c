/*
// Desenvolvido para o Shield Tiva EK-TM4C1294XL: LCD (1602B) + TECLADO Matricial (4x4) @DaLuz
// Projeto e Esquemático do Shield: http://www.elf52.daeln.com.br
// Prof. DaLuz
// Prof. Guilherme Peron
// Prof. Marcos P.
// Controle do LCD:
// RS  = PM0
// R/W = PM1
// EN  = PM2
// Barramento do LCD: PORT K (8bits)
*/

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include "lcd2.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"


void SysTick_Wait1ms(uint32_t data);
void SysTick_Wait1us(uint32_t data);

void LCD_Init(void)
{
	LCD_Command(0);
	SysTick_Wait1ms(16);
	LCD_Escreve_Inst(56);
	LCD_Escreve_Inst(12);
	LCD_Escreve_Inst(1);
	LCD_Escreve_Inst(6);
	SysTick_Wait1us(1600);			
}

void LCD_Command(uint32_t data)
{
  GPIOPinWrite(GPIO_PORTM_BASE,
	             GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2,
	             data);
}

void LCD_Data(uint32_t data)
{
  GPIOPinWrite(GPIO_PORTK_BASE, 
	             GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,
               data);
}	

void LCD_Escreve_Inst(uint32_t dado)
{
	LCD_Data(dado);
	LCD_Command(0);
	SysTick_Wait1us(6);
  LCD_Command(4);
	SysTick_Wait1us(6);	
	LCD_Command(0);
	SysTick_Wait1us(40);	
}

void LCD_Escreve_Dado(uint32_t dado)
{
	LCD_Data(dado);
	LCD_Command(1);
	SysTick_Wait1us(6);
	
	LCD_Command(5);
	SysTick_Wait1us(6);	
	
	LCD_Command(1);
	SysTick_Wait1us(40);
	SysTick_Wait1us(1600);
}

void LCD_EspecialChar(void)
{
LCD_Escreve_Inst(0x40);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x1B);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x1F);

LCD_Escreve_Inst(0x48);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x1B);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);

LCD_Escreve_Inst(0x50);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x1B);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);

LCD_Escreve_Inst(0x58);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x1B);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);

LCD_Escreve_Inst(0x60);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x1B);
LCD_Escreve_Dado(0x11);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);

LCD_Escreve_Inst(0x68);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x1B);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);

LCD_Escreve_Inst(0x70);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x1F);

LCD_Escreve_Inst(0x78);
LCD_Escreve_Dado(0x04);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x0E);
LCD_Escreve_Dado(0x1F);
LCD_Escreve_Dado(0x00);
LCD_Escreve_Dado(0x04);
}
