/*
 * Author - Mike Blandford
 *
 * Based on er9x by Erez Raviv <erezraviv@gmail.com>
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

// This version for ARM based ERSKY9X board

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef PCBSKY
 #ifndef PCBDUE
 #include "AT91SAM3S4.h"
 #endif
#endif
#include "ersky9x.h"
#include "myeeprom.h"
#include "lcd.h"
#include "drivers.h"
#include "logicio.h"
//#include "language.h"
#ifdef PCB9XT
#include "mega64.h"
#endif

#include "font.lbm"
#define font_5x8_x20_x7f (font)

#include "font_dblsize.lbm"
#define font_10x16_x20_x7f (font_dblsize)

//#include "fontnum12x8.lbm"
//#define font_num12x8 (font_num_12x8)

#include "font12x8test.lbm"
//#define font_12x8test (font_12x8)

const uint8_t font_se_extra[] = {
#include "font_se_05x07.lbm"
} ;
const uint8_t font_fr_extra[] = {
#include "font_fr_05x07.lbm"
} ;
const uint8_t font_de_extra[] = {
#include "font_de_05x07.lbm"
} ;
const uint8_t font_it_extra[] = {
#include "font_it_05x07.lbm"
} ;
const uint8_t font_pl_extra[] = {
#include "font_pl_05x07.lbm"
} ;

const uint8_t font_se_big_extra[] = {
#include "font_se_10x14.lbm"
} ;
const uint8_t font_fr_big_extra[] = {
#include "font_fr_10x14.lbm"
} ;
const uint8_t font_de_big_extra[] = {
#include "font_de_10x14.lbm"
} ;


const uint8_t *ExtraFont = NULL ;
const uint8_t *ExtraBigFont = NULL ;

// Local data
uint8_t Lcd_lastPos ;
#ifdef GREY_SCALE
uint8_t DisplayBuf[DISPLAY_W*DISPLAY_H/8*4] ;
#else
uint8_t DisplayBuf[DISPLAY_W*DISPLAY_H/8] ;
#endif
#define DISPLAY_END (DisplayBuf+sizeof(DisplayBuf))

#ifdef PCBX9D
#define X9D_OFFSET		11
#define DISPLAY_START (DisplayBuf + X9D_OFFSET)
#else
#define DISPLAY_START (DisplayBuf + 0)
#endif 

#ifndef PCBDUE
#ifdef PCBSKY
// Lookup table for prototype board
#ifdef REVB
#else
const uint8_t Lcd_lookup[] =
{
0x00,0x01,0x80,0x81,0x40,0x41,0xC0,0xC1,0x20,0x21,0xA0,0xA1,0x60,0x61,0xE0,0xE1,
0x10,0x11,0x90,0x91,0x50,0x51,0xD0,0xD1,0x30,0x31,0xB0,0xB1,0x70,0x71,0xF0,0xF1,
0x08,0x09,0x88,0x89,0x48,0x49,0xC8,0xC9,0x28,0x29,0xA8,0xA9,0x68,0x69,0xE8,0xE9,
0x18,0x19,0x98,0x99,0x58,0x59,0xD8,0xD9,0x38,0x39,0xB8,0xB9,0x78,0x79,0xF8,0xF9,
0x04,0x05,0x84,0x85,0x44,0x45,0xC4,0xC5,0x24,0x25,0xA4,0xA5,0x64,0x65,0xE4,0xE5,
0x14,0x15,0x94,0x95,0x54,0x55,0xD4,0xD5,0x34,0x35,0xB4,0xB5,0x74,0x75,0xF4,0xF5,
0x0C,0x0D,0x8C,0x8D,0x4C,0x4D,0xCC,0xCD,0x2C,0x2D,0xAC,0xAD,0x6C,0x6D,0xEC,0xED,
0x1C,0x1D,0x9C,0x9D,0x5C,0x5D,0xDC,0xDD,0x3C,0x3D,0xBC,0xBD,0x7C,0x7D,0xFC,0xFD,
0x02,0x03,0x82,0x83,0x42,0x43,0xC2,0xC3,0x22,0x23,0xA2,0xA3,0x62,0x63,0xE2,0xE3,
0x12,0x13,0x92,0x93,0x52,0x53,0xD2,0xD3,0x32,0x33,0xB2,0xB3,0x72,0x73,0xF2,0xF3,
0x0A,0x0B,0x8A,0x8B,0x4A,0x4B,0xCA,0xCB,0x2A,0x2B,0xAA,0xAB,0x6A,0x6B,0xEA,0xEB,
0x1A,0x1B,0x9A,0x9B,0x5A,0x5B,0xDA,0xDB,0x3A,0x3B,0xBA,0xBB,0x7A,0x7B,0xFA,0xFB,
0x06,0x07,0x86,0x87,0x46,0x47,0xC6,0xC7,0x26,0x27,0xA6,0xA7,0x66,0x67,0xE6,0xE7,
0x16,0x17,0x96,0x97,0x56,0x57,0xD6,0xD7,0x36,0x37,0xB6,0xB7,0x76,0x77,0xF6,0xF7,
0x0E,0x0F,0x8E,0x8F,0x4E,0x4F,0xCE,0xCF,0x2E,0x2F,0xAE,0xAF,0x6E,0x6F,0xEE,0xEF,
0x1E,0x1F,0x9E,0x9F,0x5E,0x5F,0xDE,0xDF,0x3E,0x3F,0xBE,0xBF,0x7E,0x7F,0xFE,0xFF
} ;
#endif 
#endif 
#endif // DUE

void putsTime(uint8_t x,uint8_t y,int16_t tme,uint8_t att,uint8_t att2)
{
	div_t qr ;

	uint8_t z = FWNUM*6-2 ;
	if ( att&DBLSIZE )
	{
		if ( att&CONDENSED )
		{
			x += 3 ;
			z = FWNUM*5-2 ;
		}
	}
	if ( tme<0 )
	{
		lcd_putcAtt( x - ((att&DBLSIZE) ? z : FWNUM*3),    y, '-',att);
		tme = -tme;
	}

	lcd_putcAtt( x, y, ':',att&att2);
	qr = div( tme, 60 ) ;
	if ( att&DBLSIZE )
	{
		if ( att&CONDENSED )
		{
			x += 2 ;
		}
	}
	lcd_2_digits( x, y, (uint16_t)qr.quot, att ) ;
	if ( att&DBLSIZE )
	{
		if ( att&CONDENSED )
		{
			x += FWNUM*5-4 ;
		}
		else
		{
			x += FWNUM*6-4 ;
		}
	}
	else
	{
		x += FW*3-4 ;
	}
	lcd_2_digits( x, y, (uint16_t)qr.rem, att2 ) ;
}

void putsVolts(uint8_t x,uint8_t y, uint8_t volts, uint8_t att)
{
	uint8_t option = att & NO_UNIT ;
	att &= ~NO_UNIT ;
	lcd_outdezAtt(x, y, volts, att|PREC1);
	if(!(option&NO_UNIT)) lcd_putcAtt(Lcd_lastPos, y, 'v', att);
}


void putsVBat(uint8_t x,uint8_t y,uint8_t att)
{
#ifndef MINIMISE_CODE    
  att |= g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0;
#endif
	putsVolts(x, y, g_vbat100mV, att);
}


void lcd_img( uint8_t i_x, uint8_t i_y, PROGMEM *imgdat, uint8_t idx, uint8_t mode )
{
  register const unsigned char *q = imgdat ;
  register uint8_t w    = *q++ ;
  register uint32_t hb   = (*q++ +7) / 8 ;
  register uint8_t sze1 = *q++ ;
	uint32_t yb ;
	uint32_t x ;

  q += idx * sze1 ;
  bool inv  = (mode & INVERS) ? true : (mode & BLINK ? BLINK_ON_PHASE : false ) ;
  for( yb = 0; yb < hb; yb++)
	{
    register uint8_t   *p = &DISPLAY_START[ (i_y / 8 + yb) * DISPLAY_W + i_x ];
#if PCBX9D
		if ( i_x > 211-X9D_OFFSET )
		{
			p -= DISPLAY_W ;		
		}
#endif
    for(x=0; x < w; x++)
		{
      register uint8_t b = *q++ ;
      *p++ = inv ? ~b : b;
    }
  }
}

uint8_t lcd_putc(uint8_t x,uint8_t y,const char c )
{
  return lcd_putcAtt(x,y,c,0);
}

static uint8_t *dispBufAddress( uint8_t x, uint8_t y )
{
#if (DISPLAY_W==128)
  return &DISPLAY_START[ (y & 0xF8) * 16 + x ];
#else  
	return &DISPLAY_START[ y / 8 * DISPLAY_W + x ];
#endif
}

// invers: 0 no 1=yes 2=blink
uint8_t lcd_putcAtt(uint8_t x,uint8_t y,const char c,uint8_t mode)
{
	register int32_t i ;
	register uint8_t *p    = dispBufAddress( x, y ) ;
#if PCBX9D
	if ( x > 211-X9D_OFFSET )
	{
		p -= DISPLAY_W ;		
	}
#endif
    //uint8_t *pmax = &displayBuf[ DISPLAY_H/8 * DISPLAY_W ];
	if ( c < 22 )		// Move to specific x position (c)*FW
	{
		x = c*FW ;
  	if(mode&DBLSIZE)
		{
			if ( (mode & CONDENSED) )
			{
				x = c*8 ;
			}
		 	else
			{
				x += x ;
			}	 
		}
		return x ;
	}
	x += FW ;
  register uint8_t    *q ;
	if( c < 0xC0 )
	{
		q = (uint8_t *) &font_5x8_x20_x7f[(c-0x20)*5] ;
	}
	else
	{
		if ( ExtraFont )
		{
			q = (uint8_t *) &ExtraFont[(c-0xC0)*5] ;
		}
		else
		{
			q = (uint8_t *) &font_5x8_x20_x7f[0] ;
		}
	}
  register bool   inv = (mode & INVERS) ? true : (mode & BLINK ? BLINK_ON_PHASE : false);
  
	if(mode&DBLSIZE)
  {
		uint32_t doNormal = 1 ;
	  unsigned char c_mapped = c ;

		if ( (mode & CONDENSED) )
		{
//			if ( c == '-' )
//			{
//				doNormal = 0 ;
//				c_mapped = 10 ;
//			}
//			if ( c == ':' )
//			{
//				doNormal = 0 ;
//				c_mapped = 11 ;
//			}
//			if ( ( c >= '0' ) && ( c <= '9' ) )
//			{
//				doNormal = 0 ;
//				c_mapped -= '0' ;
//			}
			doNormal = 0 ;
			
			if ( doNormal == 0 )
			{
				if ( (c!=0x2E)) x+=8-FW; //check for decimal point
			/* each letter consists of 8 top bytes followed by
	 		* five bottom by 8 bottom bytes (16 bytes per 
	 		* char) */
//				q = (uint8_t *) &font_num12x8[c_mapped*14] ;
				if( c < 0xC0 )
				{
					c_mapped = c - 0x20 ;
					q = (uint8_t *) &font_12x8[c_mapped*14] ;
		//  	  q = (uint8_t *) &font_10x16_x20_x7f[(c-0x20)*10 + ((c-0x20)/16)*160];
				}
				else
				{
					q = (uint8_t *) &font_12x8[0] ;
				}
    		
				for( i=7 ; i>=0 ; i-- )
				{
					uint8_t b1 ;
					uint8_t b3 ;
//					uint8_t b2 ;
//					uint8_t b4 ;

  		  	/*top byte*/
   		  	b1 = *q ;
  		  	/*bottom byte*/
   		  	b3 = *(q+7) ;
  		  	/*top byte*/
//   		  	b2 = *(++q) ;
  		  	/*bottom byte*/
//   		  	b4 = *(q+8) ;
   		  	q++;
					if ( i == 0 )
					{
						b1 = 0 ;
						b3 = 0 ;
					}
    		  if(inv)
					{
				    b1=~b1;
	//			    b2=~b2;
				    b3=~b3;
//				    b4=~b4;
    		  }

    		  if(&p[DISPLAY_W+1] < DISPLAY_END)
					{
    		    p[0]=b1;
//    		    p[1]=b2;
    		    p[DISPLAY_W] = b3;
//    		    p[DISPLAY_W+1] = b4;
    		    p+=1;
    		  }
    		}
			}
		}
		if ( doNormal )
		{
			if ( (c!=0x2E)) x+=FW; //check for decimal point
		/* each letter consists of ten top bytes followed by
	 	* five bottom by ten bottom bytes (20 bytes per 
	 	* char) */
	  	unsigned char c_mapped ;
			if( c < 0xC0 )
			{
				c_mapped = c - 0x20 ;
				q = (uint8_t *) &font_10x16_x20_x7f[(c_mapped)*20] ;
	//  	  q = (uint8_t *) &font_10x16_x20_x7f[(c-0x20)*10 + ((c-0x20)/16)*160];
			}
			else
			{
				if ( ExtraBigFont )
				{
					q = (uint8_t *) &ExtraBigFont[(c-0xC0)*10] ;
				}
				else
				{
					q = (uint8_t *) &font_10x16_x20_x7f[0] ;
				}
			}
    	for( i=5 ; i>=0 ; i-- )
			{
				uint8_t b1 ;
				uint8_t b3 ;
				uint8_t b2 ;
				uint8_t b4 ;

				if( c < 0xC0 )
				{
	  	  	/*top byte*/
    	  	b1 = i>0 ? *q : 0;
	  	  	/*bottom byte*/
    	  	b3 = i>0 ? *(q+10) : 0;
	  	  	/*top byte*/
    	  	b2 = i>0 ? *(++q) : 0;
	  	  	/*bottom byte*/
    	  	b4 = i>0 ? *(q+10) : 0;
    	  	q++;
				}
				else
				{
	  	  	/*top byte*/
    	  	b1 = i>0 ? *q++ : 0 ;
	  	  	/*bottom byte*/
    	  	b3 = i>0 ? *q++ : 0 ;
	  	  	/*top byte*/
    	  	b2 = i>0 ? *q++ : 0 ;
	  	  	/*bottom byte*/
    	  	b4 = i>0 ? *q++ : 0 ;
				}
    	  if(inv)
				{
			    b1=~b1;
			    b2=~b2;
			    b3=~b3;
			    b4=~b4;
    	  }

    	  if(&p[DISPLAY_W+1] < DISPLAY_END)
				{
    	    p[0]=b1;
    	    p[1]=b2;
    	    p[DISPLAY_W] = b3;
    	    p[DISPLAY_W+1] = b4;
    	    p+=2;
    	  }
    	}
		}
//        q = &dbl_font[(c-0x20)*20];
//        for(char i=0; i<10; i++){
//            uint8_t b = pgm_read_byte(q++);
//            if((p+DISPLAY_W)<DISPLAY_END) *(p+DISPLAY_W) = inv ? ~b : b;
//            b = pgm_read_byte(q++);
//            if(p<DISPLAY_END) *p = inv ? ~b : b;
//            p++;
//        }
//        if(p<DISPLAY_END) *p = inv ? ~0 : 0;
//        if((p+DISPLAY_W)<DISPLAY_END) *(p+DISPLAY_W) = inv ? ~0 : 0;
  }
  else
  {
		uint8_t condense=0;

		if (mode & CONDENSED)
		{
			*p = inv ? ~0 : 0;
			p += 1 ;
			condense=1;
			x += FWNUM-FW ;
		}

		y &= 7 ;
		if ( y )
		{ // off grid
    	for( i=5 ; i!=0 ; i-- )
			{
      	uint16_t b = *q++ ;
				b <<= y ;
      	if(p<DISPLAY_END) *p ^= b ;
      	if(&p[DISPLAY_W] < DISPLAY_END)
				{
	        p[DISPLAY_W] ^= b >> 8 ;
				}
				p += 1 ;
			}
		}
		else
		{
			uint8_t oldb = 0 ;
    	for( i=5 ; i!=0 ; i-- )
			{
    	  uint8_t b = *q++ ;
  		  if (condense && i==4)
				{
    	    /*condense the letter by skipping column 4 */
    	    continue;
    	  }
    	  if(p<DISPLAY_END) *p++ = inv ? ~(b|oldb) : (b|oldb) ;
				if (mode & BOLD)
				{
					oldb = b ;
				}
    	}
    	if(p<DISPLAY_END) *p++ = inv ? ~oldb : oldb ;
		}
  }
	return x ;
}

// Puts sub-string from string options
// First byte of string is sub-string length
// idx is index into string (in length units)
// Output length characters
void lcd_putsAttIdx(uint8_t x,uint8_t y,const char * s,uint8_t idx,uint8_t att)
{
	uint8_t length ;
	length = *s++ ;

  lcd_putsnAtt(x,y,s+length*idx,length,att) ;
}

//void lcd_putsAttIdx_right( uint8_t y,const prog_char * s,uint8_t idx,uint8_t att)
//{
//	uint8_t x = 20 - pgm_read_byte(s) ;
//	lcd_putsAttIdx( x, y, s, idx, att ) ;
//}

void lcd_putsnAtt(uint8_t x,uint8_t y, const char * s,uint8_t len,uint8_t mode)
{
	register char c ;
//	size = mode & DBLSIZE ;
  while(len!=0) {
    c = *s++ ;
#ifdef BOOT
		if ( c == 0 )
		{
			break ;			
		}
#endif

    x = lcd_putcAtt(x,y,c,mode);
//    x+=FW;
//		if ((size)&& (c!=0x2E)) x+=FW; //check for decimal point
    len--;
  }
}

void lcd_putsn_P(uint8_t x,uint8_t y,const char * s,uint8_t len)
{
  lcd_putsnAtt( x,y,s,len,0);
}

uint8_t lcd_putsAtt( uint8_t x, uint8_t y, const char *s, uint8_t mode )
{

  while(1)
	{
    char c = *s++ ;
    if(!c) break ;
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
//    x+=FW ;
//		if ((size)&& (c!=0x2E)) x+=FW ; //check for decimal point
  }
  return x;
}

void lcd_puts_Pleft( uint8_t y, const char *s )
{
  lcd_putsAtt( 0, y, s, 0);
}

// This routine skips 'skip' strings, then displays the rest
void lcd_puts_Pskip(uint8_t y,const char * s, uint8_t skip)
{
	while ( skip )
	{
    char c = *s++ ;
    if(!c) return ;
		if ( c == 31 )
		{
			skip -= 1 ;
		}
	}
  lcd_putsAtt( 0, y, s, 0);
}

void lcd_puts_P( uint8_t x, uint8_t y, const char *s )
{
  lcd_putsAtt( x, y, s, 0);
}


void lcd_outhex4(uint8_t x,uint8_t y,uint16_t val)
{
	uint8_t i ;
  x+=FWNUM*4;
  for(i=0; i<4; i++)
  {
    x-=FWNUM;
    char c = val & 0xf;
    c = c>9 ? c+'A'-10 : c+'0';
    lcd_putcAtt(x,y,c,c>='A'?CONDENSED:0);
    val>>=4;
  }
}

void lcd_outhex2(uint8_t x,uint8_t y,uint8_t val)
{
  register int i ;
  x+=FWNUM*4;
  for(i=0; i<2; i++)
  {
    x-=FWNUM;
    char c = val & 0xf;
    c = c>9 ? c+'A'-10 : c+'0';
    lcd_putcAtt(x,y,c,c>='A'?CONDENSED:0);
    val>>=4;
  }
}

void lcd_outdez( uint8_t x, uint8_t y, int16_t val )
{
  lcd_outdezAtt(x,y,val,0);
}

void lcd_outdezAtt( uint8_t x, uint8_t y, int16_t val, uint8_t mode )
{
  lcd_outdezNAtt( x,y,val,mode,5);
}

void lcd_2_digits( uint8_t x, uint8_t y, uint8_t value, uint8_t attr )
{
	lcd_outdezNAtt( x, y, value, attr + LEADING0, 2 ) ;
}

#define PREC(n) ((n&0x20) ? ((n&0x10) ? 2 : 1) : 0)
uint8_t lcd_outdezNAtt( uint8_t x, uint8_t y, int32_t val, uint8_t mode, int8_t len )
{
  uint8_t fw = FWNUM;
  uint8_t prec = PREC(mode);
  int32_t tmp = abs(val);
  uint8_t xn = 0;
  uint8_t ln = 2;
  char c;
  uint8_t xinc ;
	uint8_t fullwidth = 0 ;
	int32_t i ;
	if ( len < 0 )
	{
		fullwidth = 1 ;
		len = -len ;		
	}
	if (mode & BOLD)
	{
		fw = FW ;
	}

  if (mode & DBLSIZE)
  {
		if ( (mode & CONDENSED) )
		{
    	fw = 8 ;
    	xinc = 8 ;
    	Lcd_lastPos = 8 ;
		}
		else
		{
    	fw += FWNUM ;
    	xinc = 2*FWNUM;
    	Lcd_lastPos = 2*FW;
		}
  }
  else
  {
    xinc = FWNUM ;
    Lcd_lastPos = FW;
  }

  if (mode & LEFT) {
//    if (tmp >= 10000)
//      x += fw;
    if(val<0)
    {
      x += fw;
    }
    if (tmp >= 10000)
      x += fw;
    if (tmp >= 1000)
      x += fw;
    if (tmp >= 100)
      x += fw;
    if (tmp >= 10)
      x += fw;
    if ( prec )
    {
      if ( prec == 2 )
      {
        if ( tmp < 100 )
        {
          x += fw;
        }
      }
      if ( tmp < 10 )
      {
        x+= fw;
      }
    }
  }
  else
  {
    x -= xinc;
  }
  Lcd_lastPos += x ;

  if ( prec == 2 )
  {
    mode -= LEADING0;  // Can't have PREC2 and LEADING0
  }

  for ( i=1; i<=len; i++)
	{
    c = (tmp % 10) + '0';
    lcd_putcAtt(x, y, c, mode);
    if (prec==i)
		{
      if (mode & DBLSIZE)
			{
        xn = x ;
				if ( (mode & CONDENSED) )
				{
					x -= 1 ;
        	xn = x-1 ;
				}
				else
				{
	        if(c=='2' || c=='3' || c=='1') ln++;
	        uint8_t tn = (tmp/10) % 10;
	        if(tn==2 || tn==4)
					{
	          if (c=='4')
						{
	            xn++;
	          }
	          else
						{
	            xn--; ln++;
	          }
	        }
				}
      }
      else
			{
        x -= 2;
        if (mode & INVERS)
          lcd_vline(x+1, y, 7);
        else
          lcd_plot(x+1, y+6);
      }
      if (tmp >= 10)
        prec = 0;
    }
    tmp /= 10;
    if (!tmp)
    {
      if (prec)
      {
        if ( prec == 2 )
        {
          if ( i > 1 )
          {
            prec = 0 ;
          }
        }
        else
        {
          prec = 0 ;
        }
      }
      else if (mode & LEADING0)
			{
				if ( fullwidth == 0 )
				{
        	mode -= LEADING0;
				}
			}
      else
        break;
    }
    x-=fw;
  }
  if (xn) {
    lcd_hline(xn, y+2*FH-4, ln);
    lcd_hline(xn, y+2*FH-3, ln);
  }
  if(val<0) lcd_putcAtt(x-fw,y,'-',mode);
	return 0 ;		// Stops compiler creating two sets of POPS, saves flash
}

uint8_t plotType = PLOT_XOR ;

void lcd_write_bits( uint8_t *p, uint8_t mask )
{
  if(p<DISPLAY_END)
	{
		uint8_t temp = *p ;
		if ( plotType != PLOT_XOR )
		{
			temp |= mask ;
		}
		if ( plotType != PLOT_BLACK )
		{
			temp ^= mask ;
		}
		*p = temp ;
	}
}

void lcd_plot( register uint8_t x, register uint8_t y )
{
  //  if(y>=64)  return;
  //  if(x>=128) return;
  //  displayBuf[ y / 8 * DISPLAY_W + x ] ^= BITMASK(y%8);
	uint8_t *p = dispBufAddress( x, y ) ;
//#ifdef GREY_SCALE
//  register uint8_t *p   = &DISPLAY_START[ y / 2 * DISPLAY_W + x ];
//#else
//  register uint8_t *p   = &DISPLAY_START[ y / 8 * DISPLAY_W + x ];
//#endif
#ifdef GREY_SCALE
	lcd_write_bits( p, (y&1) ? 0xF0 : 0x0F ) ;
#else
	lcd_write_bits( p, BITMASK(y%8) ) ;
#endif
}

void lcd_hlineStip( unsigned char x, unsigned char y, signed char w, uint8_t pat )
{
  if(w<0) {x+=w; w=-w;}
#ifdef GREY_SCALE
  register uint8_t *p = &DISPLAY_START[ y / 2 * DISPLAY_W + x ] ;
	register uint8_t msk = (y&1) ? 0xF0 : 0x0F ;
#else
	uint8_t *p = dispBufAddress( x, y ) ;
	register uint8_t msk = BITMASK(y%8) ;
#endif  
  while(w)
	{
    if ( p>=DISPLAY_END)
    {
      break ;			
    }
    if(pat&1)
		{
			lcd_write_bits( p, msk ) ;
      pat = (pat >> 1) | 0x80;
    }
		else
		{
      pat = pat >> 1;
    }
    w--;
    p++;
  }
}

void lcd_hbar( uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percent )
{
	uint8_t solid ;
	if ( percent > 100 )
	{
		percent = 100 ;
	}
	solid = (w-2) * percent / 100 ;
	lcd_rect( x, y, w, h ) ;

	if ( solid )
	{
		w = y + h - 1 ;
		y += 1 ;
		x += 1 ;
		while ( y < w )
		{
 			lcd_hline(x, y, solid ) ;
			y += 1 ;			
		}
	}
}

// Reverse video 8 pixels high, w pixels wide
// Vertically on an 8 pixel high boundary
void lcd_char_inverse( uint8_t x, uint8_t y, uint8_t w, uint8_t blink )
{
	if ( blink && BLINK_ON_PHASE )
	{
		return ;
	}
	uint8_t end = x + w ;
	uint8_t *p = dispBufAddress( x, y ) ;
//#ifdef GREY_SCALE
//  register uint8_t *p = &DISPLAY_START[ y / 2 * DISPLAY_W + x ] ;
//#else
//  register uint8_t *p = &DISPLAY_START[ y / 8 * DISPLAY_W + x ] ;
//#endif  

	y &= 7 ;
	if ( y )
	{ // off grid
		while ( x < end )
		{
#ifdef GREY_SCALE
			*p ^= 0xFF ;
			*(p+DISPLAY_W) ^= 0xFF ;
			*(p+DISPLAY_W*2) ^= 0xFF ;
			*(p+DISPLAY_W*3) ^= 0xFF ;
			p += 1 ;
#else
     	uint16_t b = 0xFF ;
			b <<= y ;
			if(p<DISPLAY_END) *p ^= b ;
	    if(&p[DISPLAY_W] < DISPLAY_END) p[DISPLAY_W] ^= b >> 8 ;
			p += 1 ;
#endif  
			x += 1 ;
		}
	}
	else
	{
		while ( x < end )
		{
#ifdef GREY_SCALE
			*p ^= 0xFF ;
			*(p+DISPLAY_W) ^= 0xFF ;
			*(p+DISPLAY_W*2) ^= 0xFF ;
			*(p+DISPLAY_W*3) ^= 0xFF ;
			p += 1 ;
#else
			if(p<DISPLAY_END) *p++ ^= 0xFF ;
#endif  
			x += 1 ;
		}
	}
}

void lcd_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h )
{
	uint8_t oldPlotType = plotType ;
	if ( oldPlotType != PLOT_WHITE )
	{
		plotType = PLOT_BLACK ;
	}
  lcd_vline(x, y, h ) ;
	if ( w > 1 )
	{
  	lcd_vline(x+w-1, y, h ) ;
	}
 	lcd_hline(x+1, y+h-1, w-2 ) ;
 	lcd_hline(x+1, y, w-2 ) ;
	plotType = oldPlotType ;
}

void lcd_hline( uint8_t x, uint8_t y, int8_t w )
{
  lcd_hlineStip(x,y,w,0xff);
}

void lcd_vline( uint8_t x, uint8_t y, int8_t h )
{
  if (h<0) { y+=h; h=-h; }

#ifdef GREY_SCALE
  register uint8_t *p   = &DISPLAY_START[ y / 2 * DISPLAY_W + x ];
  y &= 0x01 ;
	if ( y )
	{
    uint8_t msk = 0x0F ;
    h -= 1 ;
		lcd_write_bits( p, msk ) ;
    p += DISPLAY_W ;
	}
    
  while( h >= 2 )
	{
		h -= 2 ;
		lcd_write_bits( p, 0xFF ) ;
    p += DISPLAY_W ;
  }
	if ( h > 0 )
	{
  	lcd_write_bits( p, 0xF0 ) ;
	}
#else
	uint8_t *p = dispBufAddress( x, y ) ;
  y &= 0x07 ;
	if ( y )
	{
    uint8_t msk = ~(BITMASK(y)-1) ;
    h -= 8-y ;
    if (h < 0)
      msk -= ~(BITMASK(8+h)-1) ;
		lcd_write_bits( p, msk ) ;
    p += DISPLAY_W ;
	}
    
  while( h >= 8 )
	{
		h -= 8 ;
		lcd_write_bits( p, 0xFF ) ;
    p += DISPLAY_W ;
  }
	if ( h > 0 )
	{
  	lcd_write_bits( p, (BITMASK(h)-1) ) ;
	}
#endif
}

#ifdef SIMU
bool lcd_refresh = true;
uint8_t lcd_buf[DISPLAY_W*DISPLAY_H/8];
#endif


#if PCBX9D
const uint8_t arrows[] = {
10,64,80,
0xFF,0xF7,0xF3,0xF9,0x00,0x00,0xF9,0xF3,0xF7,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFC,0xFC,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xCF,0x87,0x03,0x49,0xCF,0xCF,0xCF,0xCF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0x3F,0x3F,0xFF,0xFF,0xFF,0xFF,
0xFF,0xEF,0xCF,0x9F,0x00,0x00,0x9F,0xCF,0xEF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

0xFF,0xFF,0x3F,0x7F,0xFF,0x3F,0x8F,0xE3,0xF8,0xFF,
0xFF,0xFF,0xFF,0xFE,0xFC,0xFE,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xCF,0xCF,0xCF,0xCF,0x49,0x03,0x87,0xCF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,
0xFF,0x7F,0x7F,0xFF,0xFF,0xFF,0xFF,0x7F,0x7F,0xFF,
0xFF,0x9E,0x8C,0xC0,0xE1,0xE1,0xC0,0x8C,0x9E,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF

} ;
#endif

#if PCBX9D
uint8_t ImageDisplay = 0 ;
uint8_t ImageX ;
uint8_t ImageY ;
#endif

void lcd_clear()
{
  memset( DisplayBuf, 0, sizeof( DisplayBuf) ) ;
#if PCBX9D
#ifndef REV9E
	lcd_img( 212-X9D_OFFSET, 0, arrows, 0, 0 ) ;
	lcd_img( 212-X9D_OFFSET-10, 0, arrows, 1, 0 ) ;
#endif	// nREV9E


#endif // PCBX9D
}

// LCD i/o pins
// LCD_RES     PC27
// LCD_CS1     PC26
// LCD_E       PC12
// LCD_RnW     PC13
// LCD_A0      PC15
// LCD_D0      PC0
// LCD_D1      PC7
// LCD_D2      PC6
// LCD_D3      PC5
// LCD_D4      PC4
// LCD_D5      PC3
// LCD_D6      PC2
// LCD_D7      PC1

#define LCD_DATA	0x000000FFL
#ifdef REVB
#define LCD_A0    0x00000080L
#else 
#define LCD_A0    0x00008000L
#endif 
#define LCD_RnW   0x00002000L		// bit 13
#define LCD_E     0x00001000L
#define LCD_CS1   0x04000000L		// bit 26
#define LCD_RES   0x08000000L


uint8_t LcdLock ;
uint16_t LcdInputs ;

#ifdef PCBSKY
void lock_lcd()
{
	uint32_t x ;
	uint32_t y ;
	y = PIOC->PIO_PDSR ;
	x = y << 1 ; // 6 LEFT, 5 RIGHT, 4 DOWN, 3 UP
	x &= 0x00FF ;
#ifdef REVB
	if ( y & 0x01000000 )
#else 
	if ( PIOA->PIO_PDSR & 0x80000000 )
#endif
	{
		x |= 0x0400 ;		// EXIT
	}
#ifdef REVB
	if ( PIOB->PIO_PDSR & 0x000000020 )
#else 
	if ( PIOB->PIO_PDSR & 0x000000040 )
#endif
	{
		x |= 0x0200 ;		// MENU
	}
	LcdInputs = x ;
	LcdLock = 1 ;
}
#endif

#ifndef PCBDUE
#ifdef PCBSKY

#ifdef REVB
uint8_t ErcLcd = 0 ;
#endif // REVB

const static uint8_t Lcdinit[] =
{
	0xe2,		// Reset
	0xae,		// Display off
	0xa1,		// Reverse segment drive
	0xA6,		// Display normal (not inverse)
	0xA4,		// Display normal (not all on)
	0xA2,		// Bias low (A3 for high)
	0xC0,		// Scan Com0->Com63
	0x2F,		// Internal power mode
	0x25,		// Internal Vreg ratio high(ish)
	0x81,	0x22,		// Contrast
#ifndef REVX
	0xAF		// Display on
#endif
} ;	

const static uint8_t Lcd_ERC12864_2[] =
{
   0xe2, //Initialize the internal functions
   0xae, //DON = 0: display OFF
   0xa1, //ADC = 1: reverse direction(SEG132->SEG1)
   0xA6, //REV = 0: non-reverse display
   0xA4, //EON = 0: normal display. non-entire
   0xA3, // Select LCD bias=0
   0xC0, //SHL = 0: normal direction (COM1->COM64)
   0x2F, //Control power circuit operation VC=VR=VF=1
   0x27, //Select int resistance ratio R2 R1 R0 =5
   0x81, //Set reference voltage Mode
   0x2D, // 24 SV5 SV4 SV3 SV2 SV1 SV0 = 0x18
   0xAF  //DON = 1: display ON
} ;



void lcd_init()
{
	register Pio *pioptr ;
	uint32_t i ;
  // /home/thus/txt/datasheets/lcd/KS0713.pdf
  // ~/txt/flieger/ST7565RV17.pdf  from http://www.glyn.de/content.asp?wdid=132&sid=


#ifdef REVB

// read the inputs, and lock the LCD lines
	lock_lcd() ;
//	pioptr = PIOA ;
//	pioptr->PIO_PER = LCD_A0 ;		// Enable bit 7 (LCD-A0)
//	pioptr->PIO_CODR = LCD_A0 ;
//	pioptr->PIO_OER = LCD_A0 ;		// Set bit 7 output
	pioptr = PIOC ;

#ifndef REVX
	pioptr->PIO_MDER = LCD_RnW ;		// Open drain
#endif
	 
#ifdef REVX
	pioptr->PIO_PER = PIO_PC27 | PIO_PC12 | 0xFF ;		// Enable bits 27,26,13,12,7-0
#else
	pioptr->PIO_PER = PIO_PC27 | PIO_PC26 | PIO_PC13 | PIO_PC12 | 0xFF ;		// Enable bits 27,26,13,12,7-0
#endif // REVX

//#ifndef REVX
//	pioptr->PIO_CODR = LCD_E | LCD_RnW ;
//	pioptr->PIO_SODR = LCD_RES | LCD_CS1 ;
//#else 
#ifdef REVX
	pioptr->PIO_CODR = LCD_E ;
	pioptr->PIO_CODR = LCD_RnW | LCD_CS1 ;	// No longer needed, used elsewhere
#else
	pioptr->PIO_CODR = LCD_E | LCD_RnW | LCD_CS1 ;
#endif // REVX
	pioptr->PIO_SODR = LCD_RES ;
//#endif 
	pioptr->PIO_OER = PIO_PC27 | PIO_PC26 | PIO_PC13 | PIO_PC12 | 0xFF ;		// Set bits 27,26,13,12,7-0 output
	pioptr->PIO_OWER = 0x000000FFL ;		// Allow write to ls 8 bits in ODSR
#else 
	pioptr = PIOC ;
	pioptr->PIO_PER = 0x0C00B0FFL ;		// Enable bits 27,26,15,13,12,7-0
	pioptr->PIO_CODR = LCD_E | LCD_RnW | LCD_A0 ;
	pioptr->PIO_SODR = LCD_RES | LCD_CS1 ;
	pioptr->PIO_OER = 0x0C00B0FFL ;		// Set bits 27,26,15,13,12,7-0 output
	pioptr->PIO_OWER = 0x000000FFL ;		// Allow write to ls 8 bits in ODSR
#endif // REVB
	
	TC0->TC_CHANNEL[0].TC_CCR = 5 ;	// Enable clock and trigger it (may only need trigger)
	pioptr->PIO_CODR = LCD_RES ;		// Reset LCD
#ifdef REVB
	configure_pins( LCD_A0, PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
#endif // REVB
	while ( TC0->TC_CHANNEL[0].TC_CV < 500 )		// >10 uS, Value depends on MCK/2 (used 18MHz)
	{
		// Wait
	}
#ifdef REVB
  if ( ( PIOA->PIO_PDSR & LCD_A0 ) == 0 )
	{
		ErcLcd = 1 ;
	}	 
	configure_pins( LCD_A0, PIN_ENABLE | PIN_LOW | PIN_OUTPUT | PIN_PORTA | PIN_NO_PULLUP ) ;
#endif // REVB
	TC0->TC_CHANNEL[0].TC_CCR = 5 ;	// Enable clock and trigger it (may only need trigger)
	pioptr->PIO_SODR = LCD_RES ;		// Remove LCD reset
	while ( TC0->TC_CHANNEL[0].TC_CV < 27000 )	// 1500 uS, Value depends on MCK/2 (used 18MHz)
	{
		// Wait
	}
	for ( i = 0 ; i < sizeof(Lcdinit) ; i += 1 )
	{
#ifdef REVB
	  lcdSendCtl( ErcLcd ? Lcd_ERC12864_2[i] : Lcdinit[i] ) ;
#else
	  lcdSendCtl( Lcdinit[i] ) ;
#endif
	}
//  lcdSendCtl(0xe2); //Initialize the internal functions
//  lcdSendCtl(0xae); //DON = 0: display OFF
//	lcdSendCtl(0xa1); //ADC = 1: reverse direction(SEG132->SEG1)
//  lcdSendCtl(0xA6); //REV = 0: non-reverse display
//  lcdSendCtl(0xA4); //EON = 0: normal display. non-entire
//  lcdSendCtl(0xA2); // Select LCD bias=0
//  lcdSendCtl(0xC0); //SHL = 0: normal direction (COM1->COM64)
//  lcdSendCtl(0x2F); //Control power circuit operation VC=VR=VF=1
//  lcdSendCtl(0x25); //Select int resistance ratio R2 R1 R0 =5
//  lcdSendCtl(0x81); //Set reference voltage Mode
//  lcdSendCtl(0x22); // 24 SV5 SV4 SV3 SV2 SV1 SV0 = 0x18
//  lcdSendCtl(0xAF); //DON = 1: display ON
 // g_eeGeneral.contrast = 0x22;

#ifdef REVX
// 200mS delay (only if not wdt reset)

extern uint16_t ResetReason ;
	if ( ( ( ResetReason & RSTC_SR_RSTTYP ) != (2 << 8) ) && !unexpectedShutdown )	// Not watchdog
	{
		uint32_t j ;
		for ( j = 0 ; j < 100 ; j += 1 )
		{
			TC0->TC_CHANNEL[0].TC_CCR = 5 ;	// Enable clock and trigger it (may only need trigger)
			while ( TC0->TC_CHANNEL[0].TC_CV < 36000 )		// Value depends on MCK/2 (used 18MHz) give 2mS delay
			{
 			  wdt_reset() ;
				// Wait
			}
		}
	}
  lcdSendCtl(0xAF) ; //DON = 1: display ON
#endif // REVB

#ifdef REVB
	pioptr->PIO_ODR = 0x0000003AL ;		// Set bits 1, 3, 4, 5 input
	pioptr->PIO_PUER = 0x0000003AL ;		// Set bits 1, 3, 4, 5 with pullups
	pioptr->PIO_ODSR = 0 ;							// Drive D0 low
#else
	pioptr->PIO_ODR = 0x0000003CL ;		// Set bits 2, 3, 4, 5 input
	pioptr->PIO_PUER = 0x0000003CL ;		// Set bits 2, 3, 4, 5 with pullups
	pioptr->PIO_ODSR = 0 ;							// Drive D0 low
#endif // REVB
	LcdLock = 0 ;
}


void lcd_stop()
{
	TC0->TC_CHANNEL[0].TC_CCR = 0 ;	// Enable clock and trigger it (may only need trigger)
	
}

//void lcdSetContrast()
//{
//	lcdSetRefVolt(g_eeGeneral.contrast);
//}

void lcdSetRefVolt(uint8_t val)
{
#ifndef SIMU
	register Pio *pioptr ;
	pioptr = PIOC ;

// read the inputs, and lock the LCD lines
	lock_lcd() ;

	pioptr->PIO_OER = 0x0C00B0FFL ;		// Set bits 27,26,15,13,12,7-0 output

  lcdSendCtl(0x81);
	if ( val == 0 )
	{
		val = 0x22 ;		
	}
  lcdSendCtl(val);
	
#ifdef REVB
	pioptr->PIO_ODR = 0x000000FEL ;		// Set bits 1, 3, 4, 5 input
	pioptr->PIO_PUER = 0x000000FEL ;		// Set bits 1, 3, 4, 5 with pullups
	pioptr->PIO_ODSR = 0 ;							// Drive D0 low
#else
	pioptr->PIO_ODR = 0x000000FEL ;		// Set bits 2, 3, 4, 5 input
	pioptr->PIO_PUER = 0x000000FEL ;		// Set bits 2, 3, 4, 5 with pullups
	pioptr->PIO_ODSR = 0 ;							// Drive D0 low
#endif // REVB
#endif // SIMU
	LcdLock = 0 ;
}

void lcdSetOrientation()
{
	Pio *pioptr ;
	pioptr = PIOC ;
// read the inputs, and lock the LCD lines
	lock_lcd() ;

	pioptr->PIO_OER = 0x0C00B0FFL ;		// Set bits 27,26,15,13,12,7-0 output

	uint8_t c1 = 0xA1 ;
	uint8_t c2 = 0xC0 ;
//  lcdSendCtl(0xAE);             // turn-off
  if (g_eeGeneral.rotateScreen)
	{
		c1 = 0xA0 ;
		c2 = 0xC8 ;
  }
  lcdSendCtl(c1);
  lcdSendCtl(c2);
	c1 = 0xA6 ;		// Display normal (not inverse)
	if (g_eeGeneral.reverseScreen)
	{
		c1 = 0xA7 ;		// Display inverse
	}	
  lcdSendCtl(c1);
//  lcdSendCtl(0xAF);             // turn-on
#ifdef REVB
	pioptr->PIO_ODR = 0x000000FEL ;		// Set bits 1, 3, 4, 5 input
	pioptr->PIO_PUER = 0x000000FEL ;		// Set bits 1, 3, 4, 5 with pullups
	pioptr->PIO_ODSR = 0 ;							// Drive D0 low
#else
	pioptr->PIO_ODR = 0x000000FEL ;		// Set bits 2, 3, 4, 5 input
	pioptr->PIO_PUER = 0x000000FEL ;		// Set bits 2, 3, 4, 5 with pullups
	pioptr->PIO_ODSR = 0 ;							// Drive D0 low
#endif // REVB
	LcdLock = 0 ;
}


void lcdSendCtl(uint8_t val)
{
	register Pio *pioptr ;
//	register uint32_t x ;

	
#ifdef REVB
	pioptr = PIOC ;
#ifndef REVX
	pioptr->PIO_CODR = LCD_CS1 ;		// Select LCD
#endif 
	PIOA->PIO_CODR = LCD_A0 ;
#ifndef REVX
	pioptr->PIO_CODR = LCD_RnW ;		// Write
#endif
	pioptr->PIO_ODSR = val ;
#else
	pioptr = PIOC ;
	pioptr->PIO_CODR = LCD_CS1 ;		// Select LCD
	pioptr->PIO_CODR = LCD_A0 ;			// Control
	pioptr->PIO_CODR = LCD_RnW ;		// Write
	pioptr->PIO_ODSR = Lcd_lookup[val] ;
#endif 
	
	pioptr->PIO_SODR = LCD_E ;			// Start E pulse
	// Need a delay here (250nS)
	TC0->TC_CHANNEL[0].TC_CCR = 5 ;	// Enable clock and trigger it (may only need trigger)
	while ( TC0->TC_CHANNEL[0].TC_CV < 9 )		// Value depends on MCK/2 (used 18MHz)
	{
		// Wait
	}
	pioptr->PIO_CODR = LCD_E ;			// End E pulse
#ifdef REVB
	PIOA->PIO_SODR = LCD_A0 ;				// Data
#else
	pioptr->PIO_SODR = LCD_A0 ;			// Data
#endif	
//#ifndef REVX
//	pioptr->PIO_SODR = LCD_CS1 ;		// Deselect LCD
//#endif	
}

#ifdef SIMU
void refreshDisplay()
{
  memcpy(lcd_buf, DisplayBuf, sizeof(DisplayBuf));
  lcd_refresh = true;
}
#else
void refreshDisplay()
{
	register Pio *pioptr ;
  register uint8_t *p=DisplayBuf;
	register uint32_t y ;
	register uint32_t x ;
	register uint32_t z ;
	register uint32_t ebit ;

//extern uint32_t ProtocolCount ;
//lcd_outhex4 (0, 0, ProtocolCount ) ;

#ifdef REVB
#else
  register uint8_t *lookup ;
	lookup = (uint8_t *)Lcd_lookup ;
#endif // REVB
	ebit = LCD_E ;

#ifdef REVB
	pioptr = PIOA ;
	pioptr->PIO_PER = 0x00000080 ;		// Enable bit 7 (LCD-A0)
	pioptr->PIO_OER = 0x00000080 ;		// Set bit 7 output
#endif // REVB

// read the inputs, and lock the LCD lines
	lock_lcd() ;

	pioptr = PIOC ;
#ifdef REVB
	pioptr->PIO_OER = 0x0C0030FFL ;		// Set bits 27,26,15,13,12,7-0 output
#else
	pioptr->PIO_OER = 0x0C00B0FFL ;		// Set bits 27,26,15,13,12,7-0 output
#endif // REVB
	
  for( y=0; y < 8; y++)
	{
  	uint8_t column_start_lo = 0x04; // skip first 4 columns for normal ST7565
  	if (g_eeGeneral.rotateScreen)
		{
    	column_start_lo ^= 0x04 ;
		}
  	if (g_eeGeneral.optrexDisplay)
		{
    	column_start_lo ^= 0x04 ;
		}
    lcdSendCtl( column_start_lo ) ;
    lcdSendCtl(0x10); //column addr 0
    lcdSendCtl( y | 0xB0); //page addr y
    
#ifndef REVX
		pioptr->PIO_CODR = LCD_CS1 ;		// Select LCD
#endif // REVX

		PIOA->PIO_SODR = LCD_A0 ;			// Data
#ifndef REVX
		pioptr->PIO_CODR = LCD_RnW ;		// Write
#endif // REVX
		 
#ifdef REVB
		x =	*p ;
#else 
		x =	lookup[*p] ;
#endif // REVB
    for( z=0; z<128; z+=1)
		{

// The following 7 lines replaces by a lookup table	 
//			x = __RBIT( *p++ ) ;
//			x >>= 23 ;
//			if ( x & 0x00000100 )
//			{
//				x |= 1 ;		
//			}
//			pioptr->PIO_ODSR = x ;

			pioptr->PIO_ODSR = x ;
			pioptr->PIO_SODR = ebit ;			// Start E pulse
			// Need a delay here (250nS)
			p += 1 ;
#ifdef REVB
		x =	*p ;
#else 
		x =	lookup[*p] ;
#endif // REVB
//			TC0->TC_CHANNEL[0].TC_CCR = 5 ;	// Enable clock and trigger it (may only need trigger)
//			while ( TC0->TC_CHANNEL[0].TC_CV < 3 )		// Value depends on MCK/2 (used 6MHz)
//			{
//				// Wait
//			}
			pioptr->PIO_CODR = ebit ;			// End E pulse
    }
//#ifndef REVX
//		pioptr->PIO_SODR = LCD_CS1 ;		// Deselect LCD
//#endif 
  }
	pioptr->PIO_ODSR = 0xFF ;					// Drive lines high
#ifdef REVB
	pioptr->PIO_PUER = 0x000000FEL ;	// Set bits 1, 3, 4, 5 with pullups
	pioptr->PIO_ODR = 0x000000FEL ;		// Set bits 1, 3, 4, 5 input
#else
	pioptr->PIO_PUER = 0x000000FEL ;	// Set bits 2, 3, 4, 5 with pullups
	pioptr->PIO_ODR = 0x000000FEL ;		// Set bits 2, 3, 4, 5 input
#endif // REVB
	pioptr->PIO_ODSR = 0xFE ;					// Drive D0 low

	LcdLock = 0 ;

}

#endif // SIMU
#endif // PCBSKY
#endif // PCBDUE

#ifdef PCB9XT
void refreshDisplay()
{
	if ( ( m64ReceiveStatus() & 1 ) == 0 )
	{
		lcd_char_inverse( 0, 0, 6, 1 ) ;
	}
	displayToM64() ;	
}

void lcdSetRefVolt(uint8_t val)
{
	M64Contrast = val ;
	M64SetContrast = 1 ;
}

void lcdSetOrientation()
{
	M64SetContrast = 1 ;
}

void lcd_init()
{
}

void backlight_on()
{
	uint32_t r ;
	uint32_t g ;
	uint32_t b ;
	
	r = g_eeGeneral.bright ;
	if ( r < 101 )
	{
		r = 100 - r ;
	}
	g = g_eeGeneral.bright_white ;
	if ( g < 101 )
	{
		g = 100 - g ;
	}
	b = g_eeGeneral.bright_blue ;
	if ( b < 101 )
	{
		b = 100 - b ;
	}
	BlSetAllColours( r, g, b ) ;
//	BlSetColour( 100-g_eeGeneral.bright, 0 ) ;				// Red
//	BlSetColour( 100-g_eeGeneral.bright_white, 1 ) ;	// Green
//	BlSetColour( 100-g_eeGeneral.bright_blue, 2 ) ;		// blue
}

void backlight_off()
{
	BlSetColour( 0, 3 ) ;
}
#endif // PCB9XT
