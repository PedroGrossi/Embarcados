/*##############################################################################
Alunos = Gabriel Passos e Pedro Henrique Grossi da Silva
Desenvolvido para a placa EK-TM4C1294XL utilizando o SDK TivaWare no KEIL
##############################################################################*/


//TivaWare uC: Usado internamente para identificar o uC em alguns .h da TivaWare
#define PART_TM4C1294NCPDT 1
#define image_size_limit 65536

#include <stdint.h>
#include "imageFunctions.h"

uint16_t EightBitHistogram_C(uint16_t width, uint16_t height, const uint8_t *p_image, uint8_t *p_histogram)
{
	uint32_t image_size = width*height;
	
	if(image_size < image_size_limit){
			//clear histogram
			for(uint16_t i=0;i<256;i++){
				p_histogram[i] = 0;
			}
			// calculate histogram
			for(uint32_t i=0;i<image_size;i++){
				++p_histogram[p_image[i]];
			}
			return image_size;
	}
	return 0;
}
