#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

// -------------------------------------------------------------------------------
// Função AtivaColuna
// Habilita a coluna respectiva
// Parâmetro de entrada: Número da Coluna de 1 a 4 a ser habilitada
// Parâmetro de saída: não tem
void AtivaColuna(uint8_t coluna)
{
    switch(coluna)
		{
			case 1:
				//Zerar PM4 0b1110.0000
				GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
			  break;
			case 2:
				//Zerar PM5 0b1101.0000
				GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7);
				break;
			case 3:
				//Zerar PM6 0b1011.0000
				GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7);
				break;
			case 4:
				//Zerar PM7 0b0111.0000
				GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);
				break;
			default:
				//4bit high 0b1111.0000
				GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
			  break;				
		}
}

// -------------------------------------------------------------------------------
// Função LeLinha
// Retorna qual linha foi pressionada
// Parâmetro de entrada: não tem
// Parâmetro de saída: retorna a linha que está ativa
uint8_t LeLinha(void)
{
	  uint32_t portL_input;
		portL_input = GPIOPinRead(GPIO_PORTL_BASE,GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
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
// Função VarreTeclado
// Varre o teclado habilitando cada coluna para verificar qual linha está pressionada
// Parâmetro de entrada: não tem
// Parâmetro de saída: não tem
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
