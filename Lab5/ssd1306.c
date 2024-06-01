#include <stdint.h>
#include <string.h>
#include "ssd1306.h"
#include "i2c.h"
#include "data.h"

#define slaveAddress 							0x3c

unsigned char SSD1306_Buffer[SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8];

void OLED0_command(uint8_t valor)
{
uint8_t control = 0x00;
char arr[2] = {control, valor};
I2C0_sendMultipleBytes(slaveAddress, 2, arr);
}

void OLED0_data(uint8_t valor)
{
char arr[2] = {0x40, valor};
I2C0_sendMultipleBytes(slaveAddress, 2, arr);
}

void OLED0_setColAddress()
{
OLED0_command(SSD1306_COLUMNADDR); 						// 0x21 COMMAND
OLED0_command(0); 														// Column start address
OLED0_command(SSD1306_LCDWIDTH-1); 						// Column end address
}

void OLED0_setPageAddress()
{
OLED0_command(SSD1306_PAGEADDR); 							// 0x22 COMMAND
OLED0_command(0); 														// Start Page address
OLED0_command((SSD1306_LCDHEIGHT/8)-1);				// End Page address
}

void OLED0_Init()
{
OLED0_command(0xAE); //display off
OLED0_command(0x20); //Set Memory Addressing Mode   
OLED0_command(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
OLED0_command(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
OLED0_command(0xC8); //Set COM Output Scan Direction
OLED0_command(0x00); //---set low column address
OLED0_command(0x10); //---set high column address
OLED0_command(0x40); //--set start line address
OLED0_command(0x81); //--set contrast control register
OLED0_command(0xFF);
OLED0_command(0xA1); //--set segment re-map 0 to 127
OLED0_command(0xA6); //--set normal display
OLED0_command(0xA8); //--set multiplex ratio(1 to 64)
OLED0_command(0x3F); //
OLED0_command(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
OLED0_command(0xD3); //-set display offset
OLED0_command(0x00); //-not offset
OLED0_command(0xD5); //--set display clock divide ratio/oscillator frequency
OLED0_command(0xF0); //--set divide ratio
OLED0_command(0xD9); //--set pre-charge period
OLED0_command(0x22); //
OLED0_command(0xDA); //--set com pins hardware configuration
OLED0_command(0x12);
OLED0_command(0xDB); //--set vcomh
OLED0_command(0x20); //0x20,0.77xVcc
OLED0_command(0x8D); //--set DC-DC enable
OLED0_command(0x14); //
OLED0_command(0xAF); //--turn on SSD1306 panel
OLED0_command(0x2E); //scroll off
}

void OLED0_setXY(unsigned char row,unsigned char col)
{
OLED0_command(0xb0+row);                			//set page address
OLED0_command(0x00+(8*col&0x0f));       			//set low col address
OLED0_command(0x10+((8*col>>4)&0x0f));  			//set high col address
}

void OLED0_sendChar(unsigned char data)
{
char arr[2] = {0x40, data};
I2C0_sendMultipleBytes(slaveAddress, 2, arr);
}

void OLED0_clearDisplay(void)
{
unsigned char i,k;
for(k=0;k<8;k++)
   {
     OLED0_setXY(k,0);
     for(i=0;i<128;i++) OLED0_sendChar(0);
   }
}

void OLED0_displayOn(void)
{
  OLED0_command(0xaf);   										//display on
}

void OLED0_displayOff(void)
{
  OLED0_command(0xae);   										//display off
}

void OLED0_resetDisplay(void)
{
OLED0_displayOff();
OLED0_clearDisplay();
OLED0_displayOn();
}

void OLED0_sendCharXY(unsigned char data, int X, int Y)
{
char arr[6];
uint8_t i;
OLED0_setXY(X, Y);
arr[0] = 0x40;
for (i=0; i<5; i++) arr[i+1] = ASCII_font5x8[data-0x20][i];
I2C0_sendMultipleBytes(slaveAddress, 6, arr);
}

void OLED0_sendCharXYI(unsigned char data, int X, int Y)
{
char arr[6];
unsigned char inv;
int i;
OLED0_setXY(3-X,15-Y);
arr[0] = 0x40;
for (i=4; i>=0; i--)
	  {
			inv=0;
			inv+=((ASCII_font5x8[data-0x20][i])&128)>>7;
			inv+=((ASCII_font5x8[data-0x20][i])&64)>>5;
			inv+=((ASCII_font5x8[data-0x20][i])&32)>>3;
			inv+=((ASCII_font5x8[data-0x20][i])&16)>>1;
			inv+=((ASCII_font5x8[data-0x20][i])&8)<<1;
			inv+=((ASCII_font5x8[data-0x20][i])&4)<<3;
			inv+=((ASCII_font5x8[data-0x20][i])&2)<<5;
			inv+=((ASCII_font5x8[data-0x20][i])&1)<<7;
			arr[5-i]=inv;
		}
I2C0_sendMultipleBytes(slaveAddress, 6, arr);
}

void OLED0_sendStrXY(char *string, int X, int Y)
{
int i;
for (i=0; string[i]!='\0'; i++) OLED0_sendCharXY(string[i], X, Y++);
}

void OLED0_sendStrXYI(char *string, int X, int Y)
{
int i;
for (i=0; string[i]!='\0'; i++) OLED0_sendCharXYI(string[i], X, Y++);
}

void OLED7_command(uint8_t valor)
{
uint8_t control=0x00;
char arr[2]={control, valor};
I2C7_sendMultipleBytes(slaveAddress, 2, arr);
}

void OLED7_data(uint8_t valor)
{
char arr[2]={0x40,valor};
I2C7_sendMultipleBytes(slaveAddress, 2, arr);
}

void OLED7_setColAddress()
{
OLED7_command(SSD1306_COLUMNADDR); 						// 0x21 COMMAND
OLED7_command(0); 														// Column start address
OLED7_command(SSD1306_LCDWIDTH-1); 						// Column end address
}

void OLED7_setPageAddress()
{
OLED7_command(SSD1306_PAGEADDR); 							// 0x22 COMMAND
OLED7_command(0); 														// Start Page address
OLED7_command((SSD1306_LCDHEIGHT/8)-1);				// End Page address
}

void OLED7_Init()
{
OLED7_command(SSD1306_DISPLAYOFF);                    // 0xAE
OLED7_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
OLED7_command(0x80);                 // the suggested ratio 0x80
OLED7_command(SSD1306_SETMULTIPLEX);                  // 0xA8
OLED7_command(31);    // 0 - 32
OLED7_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
OLED7_command(0x0);                                   // no offset
OLED7_command(SSD1306_SETSTARTLINE);// | 0x0);        // line #0
OLED7_command(SSD1306_CHARGEPUMP);                    // 0x8D
OLED7_command(0x14);  // using internal VCC
OLED7_command(SSD1306_SETCOMPINS);                    // 0xDA
OLED7_command(0x02);
OLED7_command(SSD1306_SETCONTRAST);                   // 0x81
OLED7_command(0x7F);
OLED7_command(SSD1306_SETPRECHARGE);                  // 0xd9
OLED7_command(0x22);
OLED7_command(SSD1306_SETVCOMDETECT);                 // 0xDB
OLED7_command(0x40);
OLED7_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
OLED7_command(SSD1306_MEMORYMODE); 
OLED7_command(0x00);
OLED7_command(SSD1306_NORMALDISPLAY);                 // 0xA6
OLED7_command(SSD1306_DISPLAYON);                     //switch on OLED
}

void OLED7_displayOn(void)
{
OLED7_command(0xaf);   //display on
}

void OLED7_displayOff(void)
{
OLED7_command(0xae);   //display off
}

void OLED7_resetDisplay(void)
{
OLED7_displayOff();
OLED7_clearDisplay();
OLED7_displayOn();
}

void OLED7_clearDisplay(void)
{
unsigned char i,k;
for (k=0; k<8; k++)
    {
      OLED7_setXY(k,0);
      for (i=0; i<128; i++) OLED7_sendChar(0);
    }
}

void OLED7_sendChar(unsigned char data)
{
char arr[2] = {0x40, data};
I2C7_sendMultipleBytes(slaveAddress, 2, arr);
}

void OLED7_setXY(unsigned char row,unsigned char col)
{
OLED7_command(0xb0+row);                //set page address
OLED7_command(0x00+(8*col&0x0f));       //set low col address
OLED7_command(0x10+((8*col>>4)&0x0f));  //set high col address
}

void OLED7_sendCharXY(unsigned char data, int X, int Y)
{
char arr[6];
uint8_t i;
OLED7_setXY(X, Y);
arr[0]=0x40;
for (i=0; i<5; i++) arr[i + 1]=ASCII_font5x8[data-0x20][i];
I2C7_sendMultipleBytes(slaveAddress, 6, arr);
}

void OLED7_sendCharXYI(unsigned char data, int X, int Y)
{
char arr[6];
unsigned char inv;
int i;
OLED7_setXY(3-X,15-Y);
arr[0] = 0x40;
for (i=4; i>=0; i--)
	  {
			inv=0;
			inv+=((ASCII_font5x8[data-0x20][i])&128)>>7;
			inv+=((ASCII_font5x8[data-0x20][i])&64)>>5;
			inv+=((ASCII_font5x8[data-0x20][i])&32)>>3;
			inv+=((ASCII_font5x8[data-0x20][i])&16)>>1;
			inv+=((ASCII_font5x8[data-0x20][i])&8)<<1;
			inv+=((ASCII_font5x8[data-0x20][i])&4)<<3;
			inv+=((ASCII_font5x8[data-0x20][i])&2)<<5;
			inv+=((ASCII_font5x8[data-0x20][i])&1)<<7;
			arr[5-i]=inv;
		}
I2C7_sendMultipleBytes(slaveAddress, 6, arr);
}

void OLED7_sendStrXY(char *string, int X, int Y)
{
int i;
for (i=0; string[i]!='\0'; i++) OLED7_sendCharXY(string[i], X, Y++);  
}

void OLED7_sendStrXYI(char *string, int X, int Y)
{
int i;
for (i=0; string[i]!='\0'; i++) OLED7_sendCharXYI(string[i], X, Y++);
}

//New ... DaLuz apdaptado de LSSD1306.zip:
void SSD1306_I2C0_cls(void)
{
uint16_t i;
for (i=0;i<1024;i++) SSD1306_Buffer[i]=0;
}

void SSD1306_I2C0_UpdateScreen(void)
{
uint8_t m,i;
for (m = 0; m < 8; m++)
    {
      OLED0_command(0xB0 + m);
      OLED0_command(0x00);
      OLED0_command(0x10);
      for (i=0;i<128;i++) OLED0_data(SSD1306_Buffer[SSD1306_LCDWIDTH*m+i]);
    }
}

void SSD1306_I2C0_DrawPixel(uint16_t x, uint16_t y, uint8_t color)
{
if (x >= SSD1306_LCDWIDTH || y >= SSD1306_LCDHEIGHT) return;
/* Check if pixels are inverted */
if (color == 1) SSD1306_Buffer[x + (y / 8) * SSD1306_LCDWIDTH] |= 1 << (y % 8);
else SSD1306_Buffer[x + (y / 8) * SSD1306_LCDWIDTH] &= ~(1 << (y % 8));
}

void SSD1306_I2C0_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h)
{
int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
uint8_t byte = 0;
for (int16_t j=0; j<h; j++, y++)
    for (int16_t i=0; i<w; i++)
        {
          if (i & 7) byte <<= 1;
          else byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
          if (byte & 0x80) SSD1306_I2C0_DrawPixel(x+i, y, 1);
        }
}
