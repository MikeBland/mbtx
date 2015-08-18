/*
 * Author - Erez Raviv <erezraviv@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
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

#include "er9x.h"
#include <stdlib.h>

//#define DBL_FONT_SMALL	1

#ifdef SIMU
bool lcd_refresh = true;
uint8_t lcd_buf[DISPLAY_W*DISPLAY_H/8];
#endif // SIMU

uint8_t Lcd_lastPos;

uint8_t DisplayBuf[DISPLAY_W*DISPLAY_H/8];
#define DISPLAY_END (DisplayBuf+sizeof(displayBuf))

const prog_uchar APM font[] = {
#include "font.lbm"
};

#define font_5x8_x20_x7f (font)

//const prog_uchar APM font_dblsize[] = {
//#include "font_dblsize.lbm"
//};

//#define font_10x16_x20_x7f (font_dblsize)


void lcd_clear()
{
  memset(DisplayBuf, 0, sizeof(DisplayBuf));
}

//uint8_t lcd_putc(uint8_t x,uint8_t y,const char c )
//{
//  return lcd_putcAtt(x,y,c,0);
//}

static uint8_t *dispBufAddress( uint8_t x, uint8_t y )
{
#if (DISPLAY_W==128)
  return &DisplayBuf[ (y & 0xF8) * 16 + x ];
#else  
	return &DisplayBuf[ y / 8 * DISPLAY_W + x ];
#endif
}

// invers: 0 no 1=yes 2=blink
uint8_t lcd_putcAtt(uint8_t x,uint8_t y,const char d,uint8_t mode)
{
	uint8_t i ;
	uint8_t c = d ;
	uint8_t *p = dispBufAddress( x, y ) ;

//#if (DISPLAY_W==128)
//  uint8_t *p  = &DisplayBuf[ (y & 0xF8) * 16 + x ];
//#else  
//	uint8_t *p  = &DisplayBuf[ y / 8 * DISPLAY_W + x ];
//#endif
    //uint8_t *pmax = &DisplayBuf[ DISPLAY_H/8 * DISPLAY_W ];
		if ( c < 22 )		// Move to specific x position (c)*FW
		{
			x = c * FW ;
//  		if(mode&DBLSIZE)
//			{
//				x += x ;
//			}
			return x ;
		}
		x += FW ;
    const prog_uchar    *q = &font_5x8_x20_x7f[((unsigned char)c-0x20)*5];
    bool  inv = false ;
		{
			for( i=5; i!=0; i--)
			{
        uint8_t b = pgm_read_byte(q++);
        if(p<DISPLAY_END)	*p = inv ? ~b : b; p += 1 ;
  	  }
    	if(p<DISPLAY_END) *p++ = inv ? ~0 : 0;
		}
//	}
	return x ;
}

uint8_t lcd_putsAtt(uint8_t x,uint8_t y,const prog_char * s,uint8_t mode)
{
	uint8_t source ;
	source = mode & BSS ;
  while(1)
	{
    char c = (source) ? *s++ : pgm_read_byte(s++);
    if(!c) break;
		if ( c == 31 )
		{
			if ( (y += FH) >= DISPLAY_H )	// Screen height
			{
				break ;
			}	
			x = 0 ;
		}
		else
		{
    	x = lcd_putcAtt(x,y,c,mode) ;
		}
  }
  return x;
}

void lcd_puts_Pleft(uint8_t y,const prog_char * s)
{
  lcd_putsAtt( 0, y, s, 0);
}

void lcd_outhex4(uint8_t x,uint8_t y,uint16_t val)
{
	uint8_t i ;
  x+=FW*4;
  for(i=0; i<4; i++)
  {
    x-=FW;
    char c = val & 0xf;
    c = c>9 ? c+'A'-10 : c+'0';
    lcd_putcAtt(x,y,c,0);
    val>>=4;
  }
}


void lcdSetContrast()
{
	lcdSetRefVolt(25);
}

void lcdSendCtl(uint8_t val)
{
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_A0);
//  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
  PORTA_LCD_DAT = val;
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1);
}


#define delay_1us() _delay_us(1)
#define delay_2us() _delay_us(2)
static void delay_1_5us( uint16_t ms)
{
  for( uint16_t i=0; i<ms; i++) delay_1us();
}

const static prog_uchar APM Lcdinit[] =
{
  0xE2, 0xAE, 0xA1, 0xA6, 0xA4, 0xA2, 0xC0, 0x2F, 0x25, 0xAF
} ;	

void lcd_init()
{
  // /home/thus/txt/datasheets/lcd/KS0713.pdf
  // ~/txt/flieger/ST7565RV17.pdf  from http://www.glyn.de/content.asp?wdid=132&sid=
	uint8_t i ;

	LcdLock = 1 ;						// Lock LCD data lines
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RES);  //LCD_RES
  delay_2us();
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RES); //  f524  sbi 0x15, 2 IOADR-PORTC_LCD_CTRL; 21           1
  delay_1_5us(1500);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);

	for ( i = 0 ; i < sizeof(Lcdinit) ; i += 1 )
	{
	  lcdSendCtl(pgm_read_byte(&Lcdinit[i]) ) ;
	}
	lcdSetContrast() ;
//	LcdLock = 0 ;						// Free LCD data lines

}


void lcdSetRefVolt(uint8_t val)
{
	LcdLock = 1 ;						// Lock LCD data lines
  lcdSendCtl(0x81);
  lcdSendCtl(val);
	LcdLock = 0 ;						// Free LCD data lines
}

volatile uint8_t LcdLock ;

void refreshDiplay()
{
#ifdef SIMU
  memcpy(lcd_buf, DisplayBuf, sizeof(DisplayBuf));
  lcd_refresh = true;
#else
	LcdLock = 1 ;						// Lock LCD data lines
  uint8_t column_start_lo = 0x04; // skip first 4 columns for normal ST7565
  uint8_t *p=DisplayBuf;
  for(uint8_t y=0xB0; y < 0xB8; y++) {
    lcdSendCtl(column_start_lo);
    lcdSendCtl(0x10); //column addr 0
    lcdSendCtl( y ); //page addr y
    
		PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1);
    PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
//    PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
		
    for(uint8_t x=32; x>0; x--){
//      lcdSendDat(*p);
      PORTA_LCD_DAT = *p++;
      PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E);
      PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
      PORTA_LCD_DAT = *p++;
      PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E);
      PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
      PORTA_LCD_DAT = *p++;
      PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E);
      PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
      PORTA_LCD_DAT = *p++;
      PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E);
      PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);
//      p++;
    }
    PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
    PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1);
  }
	LcdLock = 0 ;						// Free LCD data lines
#endif
}


