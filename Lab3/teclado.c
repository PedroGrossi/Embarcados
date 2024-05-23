// teclado.c
// Desenvolvido para a placa EK-TM4C1294XL + Shield LCD(1602b) + Teclado (4x4)
// Controla o teclado matricial 4 linhas x 3 colunas
// Projeto e Esquem�tico do Shield: http://www.elf52.daeln.com.br
// Prof. DaLuz
// Prof. Guilherme Peron
// Prof. Marcos P.
// 16/07/2023

//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
//#include "tm4c1294ncpdt.h"

int PortL_Input(void);

// -------------------------------------------------------------------------------
// Fun��o AtivaColuna
// Habilita a coluna respectiva
// Par�metro de entrada: N�mero da Coluna de 1 a 4 a ser habilitada
// Par�metro de sa�da: n�o tem
void AtivaColuna(uint8_t coluna)
{
    switch(coluna)
		{
			case 1:
				GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_4, 0xE0); //Zerar PM4
				break;
			case 2:
				GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_5, 0xD0); //Zerar PM5
				break;
			case 3:
				GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_6, 0xB0); //Zerar PM6
				break;
			case 4:
				GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_7, 0x70); //Zerar PM7
				break;
			default:
				GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_0, 0xF0); //??? o pino tem que ser diferente
				//PortM_Output2(0xF0);
				break;				
		}
}

// -------------------------------------------------------------------------------
// Fun��o LeLinha
// Retorna qual linha foi pressionada
// Par�metro de entrada: n�o tem
// Par�metro de sa�da: retorna a linha que est� ativa
uint8_t LeLinha(void)
{
	  uint32_t portL_input;
		portL_input = PortL_Input();
	  switch(portL_input)
		{
			case 0x0E:  //Port L0
				return 1;
			case 0x0D:  //Port L1
				return 2;
			case 0x0B:  //Port L2
				return 3;
			case 0x07:  //Port L3
				return 4;
			default:
				return 0;
		}
}

// -------------------------------------------------------------------------------
// Fun��o VarreTeclado
// Varre o teclado habilitando cada coluna para verificar qual linha est� pressionada
// Par�metro de entrada: n�o tem
// Par�metro de sa�da: n�o tem
uint8_t VarreTeclado(void)
{
		uint8_t linha, caracter = 0;
	
		//Varre a primeira coluna
		AtivaColuna(1);
		linha = LeLinha();
		if (linha)
		{
			switch (linha)
			{
				case 1:
					caracter = '7';
					break;
				case 2:
					caracter = '4';
					break;
				case 3:
					caracter = '1';
					break;
				case 4:
					caracter = '#';
					break;
			}
		}
	
		//Varre a segunda coluna
		AtivaColuna(2);
		linha = LeLinha();
		if (linha && caracter == 0)
		{
			switch (linha)
			{
				case 1:
					caracter = '8';
					break;
				case 2:
					caracter = '5';
					break;
				case 3:
					caracter = '2';
					break;
				case 4:
					caracter = '0';
					break;
			}
		}
		
		//Varre a terceira coluna
		AtivaColuna(3);
		linha = LeLinha();
		if (linha && caracter == 0)
		{
			switch (linha)
			{
				case 1:
					caracter = '9';
					break;
				case 2:
					caracter = '6';
					break;
				case 3:
					caracter = '3';
					break;
				case 4:
					caracter = '*';
					break;
			}
		}	
		
		//Varre a quarta coluna
		AtivaColuna(4);
		linha = LeLinha();
		if (linha && caracter == 0)
		{
			switch (linha)
			{
				case 1:
					caracter = 'A';
					break;
				case 2:
					caracter = 'B';
					break;
				case 3:
					caracter = 'C';
					break;
				case 4:
					caracter = 'D';
					break;
			}
		}		
return(caracter);
}
