/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Oliver Stefan
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "opentx.h"

void lcdSendCtl(uint8_t val)
{
	#ifdef LCD_MULTIPLEX
	DDRA = 0xFF; // set LCD_DAT pins to output
	#endif
	PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RS);
	PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
	PORTA_LCD_DAT = val;
	PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E);
	_delay_us(8); 
	PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
	#ifdef LCD_MULTIPLEX
	DDRA = 0x00; // set LCD_DAT pins to input
	#endif
}

#if defined(PCBSTD) && defined(VOICE)
volatile uint8_t LcdLock ;
#define LCD_LOCK() LcdLock = 1
#define LCD_UNLOCK() LcdLock = 0
#else
#define LCD_LOCK()
#define LCD_UNLOCK()
#endif

inline void lcdInit()
{
	LCD_LOCK();
	_delay_ms(50); 
	PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RES);  //LCD Reset
	_delay_ms(100); 
	PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RES);
	_delay_ms(100); 
	lcdSendCtl(0x30);	// set 8-bit interface
	_delay_ms(1);
	lcdSendCtl(0x36);	// Repeat with graphics bit set to on.
	_delay_us(80);
	lcdSendCtl(0x0C);	// display ON, cursor and blink OFF
	_delay_us(80);
	lcdSendCtl(0x01);	// clear display, reset address
	_delay_us(80);
	lcdSendCtl(0x06);	// display ON, no cursor
	_delay_us(80);	
	LCD_UNLOCK();
}

void lcdSetRefVolt(uint8_t val)
{
	//there is no contrast setting in the ST7920 controller
}

void lcdRefresh()
{
	LCD_LOCK();
	uint8_t x_addr = 0;
	uint8_t y_addr = 0;
	uint16_t line_offset = 0;
	uint8_t col_offset = 0;
	uint16_t byte_offset = 0;
	uint8_t bit_count = 0;
	
	for(uint8_t y=0; y < 64; y++) {
		x_addr = 0;
		// convert coordinates to weirdly-arranged 128x64 screen (the ST7920 is mapped for 256x32 displays).
		if (y > 31)
		{
			y_addr = y - 32;    // because there are only 31 addressable lines in the ST7920
			x_addr += 8;        // so we overflow x (7 visible bytes per line) to reach the bottom half
		}else{
			y_addr = y;
		}
	  
		lcdSendCtl( 0x80 | y_addr ); 	// Set Vertical Address. 
		_delay_us(49); 
		lcdSendCtl( 0x80 | x_addr ); 	// Set Horizontal Address. 
		_delay_us(49); 
		#ifdef LCD_MULTIPLEX
		DDRA = 0xFF; // set LCD_DAT pins to output
		#endif
		PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RS);	// HIGH RS and LOW RW will put the LCD to    
		PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);	// Write data register mode
		
		bit_count = y & 0x07; //count from 0 bis 7 -> 0=0, 1=1..7=7, 8=0, 9=1...
		col_offset = 1 << bit_count; //build a value for a AND operation with the vorrect bitposition
		line_offset = ( y / 8 ) * 128; //on the ST7565 there are 8 lines with each 128 bytes width
		for (xcoord_t x=0; x < 16; x++) { //walk through 16 bytes form left to right (128 Pixel)	
			byte_offset = line_offset + ( x * 8 ); //calculate the position of the first byte im array
			//adressing the bytes sequential and shift the bits at the correct position, afterwards a OR operation to get all bits in one byte
			//the position of the LSB is the left-most position of the byte to the ST7920
			PORTA_LCD_DAT = (((displayBuf[byte_offset] & col_offset) >> bit_count) << 7) | (((displayBuf[byte_offset + 1] & col_offset) >> bit_count) << 6) | (((displayBuf[byte_offset + 2] & col_offset) >> bit_count ) << 5) | (((displayBuf[byte_offset + 3] & col_offset) >> bit_count ) << 4) | (((displayBuf[byte_offset + 4] & col_offset) >> bit_count ) << 3) | (((displayBuf[byte_offset + 5] & col_offset) >> bit_count ) << 2) | (((displayBuf[byte_offset + 6] & col_offset) >> bit_count ) << 1) | (((displayBuf[byte_offset + 7] & col_offset) >> bit_count ) << 0); 
			PORTC_LCD_CTRL |= (1<<OUT_C_LCD_E);
			_delay_us(8); 
			PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
			_delay_us(49); 
		}
	}
	LCD_UNLOCK();
}

