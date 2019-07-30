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

//#define LCD_2_CS		1

extern struct t_rotary Rotary ;

#define DBL_FONT_SMALL	1

#ifdef SIMU
bool lcd_refresh = true;
uint8_t lcd_buf[DISPLAY_W*DISPLAY_H/8];
#endif

uint8_t Lcd_lastPos;

uint8_t DisplayBuf[DISPLAY_W*DISPLAY_H/8];
#define DISPLAY_END (DisplayBuf+sizeof(displayBuf))

const prog_uint8_t APM _bitmask[]= { 1,2,4,8,16,32,64,128 } ;

const prog_uchar APM font[] = {
#include "font.lbm"
};

#define font_5x8_x20_x7f (font)

#ifdef SMALL_DBL
#include "font12x8test.lbm"
#define font_10x16_x20_x7f (font_12x8)
#else
const prog_uchar APM font_dblsize[] = {
#include "font_dblsize.lbm"
};
#define font_10x16_x20_x7f (font_dblsize)
#endif // SMALL_DBL


void lcd_clear()
{
//  memset(DisplayBuf, 0, sizeof(DisplayBuf));
	uint16_t i ;
  uint8_t *p = (uint8_t *) DisplayBuf ;
	for ( i = sizeof(DisplayBuf) ; i ; i -= 1 )
	{
		*p++ = 0 ;
	}
}


void putsTime(uint8_t x,uint8_t y,int16_t tme,uint8_t att,uint8_t att2)
{
	div_t qr ;
#ifdef SMALL_DBL
	uint8_t z = FWNUM*6-2 ;
	if ( att&DBLSIZE )
	{
		x += 3 ;
		z = FWNUM*5-2 ;
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
		x += 2 ;
	}
	lcd_2_digits( x, y, (uint16_t)qr.quot, att ) ;
	
	if ( att&DBLSIZE )
	{
		x += FWNUM*5-4 ;
	}
	else
	{
		x += FW*3-3 ;
	}
	lcd_2_digits( x, y, (uint16_t)qr.rem, att2 ) ;
#else	
	if ( tme<0 )
	{
		lcd_putcAtt( x - ((att&DBLSIZE) ? FWNUM*6-2 : FWNUM*3),    y, '-',att);
		tme = -tme;
	}

	lcd_putcAtt(x, y, ':',att&att2);
	qr = div( tme, 60 ) ;
	lcd_2_digits( x, y, (uint16_t)qr.quot, att ) ;
	x += (att&DBLSIZE) ? FWNUM*6-4 : FW*3-3;
	lcd_2_digits( x, y, (uint16_t)qr.rem, att2 ) ;
#endif
}

void putsVolts(uint8_t x,uint8_t y, uint8_t volts, uint8_t att)
{
	lcd_outdezAtt(x, y, volts, att|PREC1);
	if(!(att&NO_UNIT)) lcd_putcAtt(Lcd_lastPos, y, 'v', att);
}


void putsVBat(uint8_t x,uint8_t y,uint8_t att)
{
#ifndef MINIMISE_CODE    
	att |= g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0;
#endif
	putsVolts(x, y, g_vbat100mV, att);
}


void lcd_img(uint8_t i_x,uint8_t i_y,const prog_uchar * imgdat,uint8_t idx/*,uint8_t mode*/)
{
  const prog_uchar  *q = imgdat;

  uint8_t w    = pgm_read_byte(q++);
  uint8_t hb   = pgm_read_byte(q++) ;
	hb += 7 ;
	hb /= 8 ;
  uint8_t sze1 = pgm_read_byte(q++);
  q += idx*sze1;
//  bool    inv  = (mode & INVERS) ? true : (mode & BLINK ? BLINK_ON_PHASE : false);
  for(uint8_t yb = 0; yb < hb; yb++){
    uint8_t   *p = &DisplayBuf[ (i_y / 8 + yb) * DISPLAY_W + i_x ];
    for(uint8_t x=0; x < w; x++){
      uint8_t b = pgm_read_byte(q++);
      *p++ = b;
      //*p++ = inv ? ~b : b;
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
    bool         inv = (mode & INVERS) ? true : (mode & BLINK ? BLINK_ON_PHASE : false);
	if(mode&DBLSIZE)
  {
#ifdef SMALL_DBL
		if ( (c!=0x2E)) x+=2; //check for decimal point
#else
		if ( (c!=0x2E)) x+=FW; //check for decimal point
#endif
	/* each letter consists of ten top bytes followed by
	 * five bottom by ten bottom bytes (20 bytes per 
	 * char) */
		  unsigned char c_mapped ;

#ifdef DBL_FONT_SMALL
			if ( c >= ',' && c <= ':' )
			{
				c_mapped = c - ',' + 1 ;		
			}
  		else if (c>='A' && c<='Z')
			{
				c_mapped = c - 'A' + 0x10 ;
			}
  		else if (c>='a' && c<='z')
			{
				c_mapped = c - 'a' + 0x2B ;
			}
  		else if (c=='_' )
			{
				c_mapped = 0x2A ;
			}
			else
			{
				c_mapped = 0 ;
			}
#else
			c_mapped = c - 0x20 ;
#endif
#ifdef SMALL_DBL
        q = &font_10x16_x20_x7f[(c_mapped)*14] ;// + ((c-0x20)/16)*160];
#ifdef DBL_FONT_SMALL
#if defined(CPUM128) || defined(CPUM2561)
				if ( ( c_mapped == ('i'-'a'+0x2B) ) || ( c_mapped == ('l'-'a'+0x2B) ) )
				{
					p -= 1 ;
					x -= 2 ;
				}
#endif
#endif
        for(char i=7; i>=0; i--)
				{
					uint8_t b1 ;
					uint8_t b3 ;
   		  	b1 = pgm_read_byte(q) ;
   		  	b3 = pgm_read_byte(q+7) ;
					if ( i == 0 )
					{
						b1 = 0 ;
						b3 = 0 ;
					}
#else
        q = &font_10x16_x20_x7f[(c_mapped)*20] ;// + ((c-0x20)/16)*160];
        for(char i=11; i>=0; i--)
				{
	    /*top byte*/
            uint8_t b1 = i>1 ? pgm_read_byte(q) : 0;
	    /*bottom byte*/
            uint8_t b3 = i>1 ? pgm_read_byte(10+q) : 0;
	    /*top byte*/
//            uint8_t b2 = i>0 ? pgm_read_byte(++q) : 0;
	    /*bottom byte*/
//            uint8_t b4 = i>0 ? pgm_read_byte(10+q) : 0;
#endif // SMALL_DBL
            q++;
            if(inv) {
                b1=~b1;
//                b2=~b2;
                b3=~b3;
//                b4=~b4;
            }

            if(p < DISPLAY_END-(DISPLAY_W+1)){
                p[0]=b1;
//                p[1]=b2;
                p[DISPLAY_W] = b3;
//                p[DISPLAY_W+1] = b4;
                p+=1;
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
		
#if defined(CPUM128) || defined(CPUM2561)
		y &= 7 ;
		if ( y )
		{ // off grid
    	for( i=5 ; i!=0 ; i-- )
			{
        uint16_t b = pgm_read_byte(q++);
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
#endif
		{
			for( i=5; i!=0; i--)
			{
        uint8_t b = pgm_read_byte(q++);
  	    if (condense && i==4)
				{
        	/*condense the letter by skipping column 4 */
           continue;
        }
        if(p<DISPLAY_END)	*p = inv ? ~b : b; p += 1 ;
  	  }
    	if(p<DISPLAY_END) *p++ = inv ? ~0 : 0;
		}
	}
	return x ;
}

// Puts sub-string from string options
// First byte of string is sub-string length
// idx is index into string (in length units)
// Output length characters
void lcd_putsAttIdx(uint8_t x,uint8_t y,const prog_char * s,uint8_t idx,uint8_t att)
{
	uint8_t length ;
	length = pgm_read_byte(s++) ;

  lcd_putsnAtt(x,y,s+length*idx,length,att) ;
}

//void lcd_putsAttIdx_right( uint8_t y,const prog_char * s,uint8_t idx,uint8_t att)
//{
//	uint8_t x = 20 - pgm_read_byte(s) ;
//	lcd_putsAttIdx( x, y, s, idx, att ) ;
//}

//uint8_t lcd_putsnAtt(uint8_t x,uint8_t y,const prog_char * s,uint8_t len,uint8_t mode)
void lcd_putsnAtt(uint8_t x,uint8_t y,const prog_char * s,uint8_t len,uint8_t mode)
{
	uint8_t source ;
	source = mode & BSS ;
//	size = mode & DBLSIZE ;
  while(len!=0) {
    char c = (source) ? *s++ : pgm_read_byte(s++);
		if ( c == 0 )
		{
			return ;
		}
    x = lcd_putcAtt(x,y,c,mode);
//    x+=FW;
//		if ((size)&& (c!=0x2E)) x+=FW; //check for decimal point
    len--;
  }
}
void lcd_putsn_P(uint8_t x,uint8_t y,const prog_char * s,uint8_t len)
{
  lcd_putsnAtt( x,y,s,len,0);
}


uint8_t lcd_putsAtt(uint8_t x,uint8_t y,const prog_char * s,uint8_t mode)
{
	uint8_t source ;
	source = mode & BSS ;
  while(1)
	{
    char c = (source) ? *s++ : pgm_read_byte(s++);
    if(!c) break;
#ifdef XSW_MOD
    if ( c == '\037' || c == '\035' )   // '\037' for (CR+LF), '\035' for CR+LF+indentation
#else
		if ( c == 31 )
#endif
		{
			if ( (y += FH) >= DISPLAY_H )	// Screen height
			{
				break ;
			}	
			x = 0 ;
#ifdef XSW_MOD
      if (c == '\035')
        x = FW ;
#endif
		}
		else
		{
    	x = lcd_putcAtt(x,y,c,mode) ;
		}
//    x+=FW;
//		if ((size)&& (c!=0x2E)) x+=FW; //check for decimal point
  }
  return x;
}

void lcd_puts_Pleft(uint8_t y,const prog_char * s)
{
  lcd_putsAtt( 0, y, s, 0);
}

// This routine skips 'skip' strings, then displays the rest
void lcd_puts_Pskip(uint8_t y,const prog_char * s, uint8_t skip)
{
	while ( skip )
	{
    char c = pgm_read_byte(s++);
    if(!c) return ;
		if ( c == 31 )
		{
			skip -= 1 ;
		}
	}
  lcd_putsAtt( 0, y, s, 0);
}

void lcd_puts_P(uint8_t x,uint8_t y,const prog_char * s)
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
void lcd_outdez( uint8_t x, uint8_t y, int16_t val)
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
	uint8_t negative = 0 ;
  uint8_t xn = 0;
  uint8_t ln = 2;
  char c;
  uint8_t xinc ;
	uint8_t fullwidth = 0 ;

	mode &= ~NO_UNIT ;
	if ( len < 0 )
	{
		fullwidth = 1 ;
		len = -len ;		
	}

  if ( val < 0 )
	{
		val = -val ;
		negative = 1 ;
	}

  if (mode & DBLSIZE)
  {
#ifdef SMALL_DBL
   	fw = 8 ;
   	xinc = 8 ;
   	Lcd_lastPos = 8 ;
#else
    fw += FWNUM ;
    xinc = 2*FWNUM;
    Lcd_lastPos = 2*FW;
#endif
  }
  else
  {
    xinc = FWNUM ;
    Lcd_lastPos = FW;
  }

  if (mode & LEFT) {
//    if (val >= 10000)
//      x += fw;
    if(negative)
    {
      x += fw;
    }
    if (val >= 1000)
      x += fw;
    if (val >= 100)
      x += fw;
    if (val >= 10)
      x += fw;
    if ( prec )
    {
      if ( prec == 2 )
      {
        if ( val < 100 )
        {
          x += fw;
        }
      }
      if ( val < 10 )
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

  for (uint8_t i=1; i<=len; i++)
	{
		div_t qr ;
		qr = div( val, 10 ) ;
    c = (qr.rem) + '0';
    lcd_putcAtt(x, y, c, mode);
    if (prec==i) {
      if (mode & DBLSIZE) {
        xn = x;
        if( c<='3' && c>='1') ln++;
        uint8_t tn = (qr.quot) % 10;
        if(tn==2 || tn==4) {
          if (c=='4') {
            xn++;
          }
          else {
            xn--; ln++;
          }
        }
      }
      else {
        x -= 2;
        if (mode & INVERS)
          lcd_vline(x+1, y, 7);
        else
          lcd_plot(x+1, y+6);
      }
      if (qr.quot)
        prec = 0;
    }
    val = qr.quot ;
    if (!val)
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
  if (xn)
	{
#ifdef SMALL_DBL
    lcd_hline(xn-1, y+2*FH-4, ln);
    lcd_hline(xn-1, y+2*FH-3, ln);
#else
    lcd_hline(xn, y+2*FH-4, ln);
    lcd_hline(xn, y+2*FH-3, ln);
#endif // SMALL_DBL
  }
  if(negative) lcd_putcAtt(x-fw,y,'-',mode);
	asm("") ;
	return 0 ;		// Stops compiler creating two sets of POPS, saves flash
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

//#if defined(CPUM128) || defined(CPUM2561)
//// Extra data for Mavlink via FrSky
//void lcd_vbar( uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percent )
//{
//	uint8_t solid ;
//	if ( percent > 100 )
//	{
//		percent = 100 ;
//	}
//	solid = (h-2) * percent / 100 ;
//	lcd_rect( x, y, w, h ) ;

//	if ( solid )
//	{
//		y += ( h - solid ) - 1 ;
//        lcd_plot(x + w, y);
//		w = w + x - 1 ;
//		x += 1 ;
//		while ( x < w )
//		{
//		  	lcd_vline(x, y, solid ) ;

//			x += 1 ;
//		}
//	}
//}
//// Extra data for Mavlink via FrSky
//#endif

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
//#if (DISPLAY_W==128)
//  uint8_t *p  = &DisplayBuf[ (y & 0xF8) * 16 + x ];
//#else  
//	uint8_t *p  = &DisplayBuf[ y / 8 * DISPLAY_W + x ];
//#endif

#if defined(CPUM128) || defined(CPUM2561)
	y &= 7 ;
	if ( y )
	{ // off grid
		while ( x < end )
		{
     	uint16_t b = 0xFF ;
			b <<= y ;
			if(p<DISPLAY_END) *p ^= b ;
	    if(&p[DISPLAY_W] < DISPLAY_END) p[DISPLAY_W] ^= b >> 8 ;
			p += 1 ;
			x += 1 ;
		}
	}
	else
#endif
	{
		while ( x < end )
		{
			*p++ ^= 0xFF ;
			x += 1 ;
		}
	}
}

uint8_t plotType = PLOT_XOR ;

void lcd_rect_xor(uint8_t x, uint8_t y, uint8_t w, uint8_t h )
{
  lcd_vline(x, y, h ) ;
	if ( w > 1 )
	{
  	lcd_vline(x+w-1, y, h ) ;
	}
 	lcd_hline(x, y+h-1, w ) ;
 	lcd_hline(x, y, w ) ;
}

void lcd_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h )
{
	uint8_t oldPlotType = plotType ;
	plotType = PLOT_BLACK ;
	lcd_rect_xor( x, y, w, h ) ;
	plotType = oldPlotType ;
}

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

void lcd_plot(uint8_t x,uint8_t y)
{
	uint8_t *p = dispBufAddress( x, y ) ;
//#if (DISPLAY_W==128)
//  uint8_t *p  = &DisplayBuf[ (y & 0xF8) * 16 + x ];
//#else  
//	uint8_t *p  = &DisplayBuf[ y / 8 * DISPLAY_W + x ];
//#endif
	lcd_write_bits( p, XBITMASK(y%8) ) ;
}

void lcd_hlineStip(unsigned char x,unsigned char y, signed char w,uint8_t pat)
{
  if(w<0) {x+=w; w=-w;}
	uint8_t *p = dispBufAddress( x, y ) ;
//#if (DISPLAY_W==128)
//  uint8_t *p  = &DisplayBuf[ (y & 0xF8) * 16 + x ];
//#else  
//	uint8_t *p  = &DisplayBuf[ y / 8 * DISPLAY_W + x ];
//#endif
  uint8_t msk = XBITMASK(y%8);
  while(w){
    if ( p>=DISPLAY_END)
    {
      break ;			
    }
    if(pat&1) {
			lcd_write_bits( p, msk ) ;
      pat = (pat >> 1) | 0x80;
    }else{
      pat = pat >> 1;
    }
    w--;
    p++;
  }
}

void lcd_hline(uint8_t x,uint8_t y, int8_t w)
{
  lcd_hlineStip(x,y,w,0xff);
}

void lcd_vline(uint8_t x,uint8_t y, int8_t h)
{
//    while ((y+h)>=DISPLAY_H) h--;
  if (h<0) { y+=h; h=-h; }
	uint8_t *p = dispBufAddress( x, y ) ;
//#if (DISPLAY_W==128)
//  uint8_t *p  = &DisplayBuf[ (y & 0xF8) * 16 + x ];
//#else  
//	uint8_t *p  = &DisplayBuf[ y / 8 * DISPLAY_W + x ];
//#endif
  y &= 0x07 ;
	if ( y )
	{
    uint8_t msk = ~(XBITMASK(y)-1) ;
    h -= 8-y ;
    if (h < 0)
      msk -= ~(XBITMASK(8+h)-1) ;
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
  	lcd_write_bits( p, (XBITMASK(h)-1) ) ;
	}
	asm("") ;
}

uint8_t EepromActive ;

void lcdSetContrast()
{
	lcdSetRefVolt(g_eeGeneral.contrast);
}

#if LCD_OTHER   // defined @er9x.h

// Supports 4W serial LCD interface and SSD1306 OLED controller
// - Hyun-Taek Chang (flybabo@att.net), Feb 2013

// force inline expansion
#define ALWAYS_INLINE   __attribute__((always_inline))

// to select either stock LCD controller or SSD1306 OLED controller
#ifdef COOLLVSE
#define _SSD1306         1       // Stock(ST7565/NT7532)=0, _SSD1306=1
#else
#define _SSD1306         0       // Stock(ST7565/NT7532)=0, _SSD1306=1
#endif

// controller independent options
#ifdef COOLLVSE
#define SERIAL_LCD      1       // parallel=0, 4W_serial=1
#else
#define SERIAL_LCD      0       // parallel=0, 4W_serial=1
#endif
#define ROTATE_SCREEN   0       // don't-rotate-screen=0, rotate-180-degree=1
#define	REVERSE_VIDEO   0       // normal-video=0, reverse-video=1

volatile uint8_t LcdLock ;

#define delay_1us() _delay_us(1)
#define delay_2us() _delay_us(2)
static void delay_1_5us(int ms)
{
  for(int i=0; i<ms; i++) delay_1us();
}

#if (SERIAL_LCD || LCD_EEPE)  // LCD_EEPE defined @er9x.h
// Serial LCD module's SCLK(clock) and SI(data) must be connected
// to Atmega's PC4 and PC5, respectively.
#define OUT_C_LCD_SCL OUT_C_LCD_RnW     // PC4
#define OUT_C_LCD_SI  OUT_C_LCD_E       // PC5

static void lcdSendBit(uint8_t b, uint8_t v0, uint8_t v1) ALWAYS_INLINE;
static void lcdSend8bits(uint8_t val, uint8_t v0, uint8_t v1) ALWAYS_INLINE;
static void lcdSendDataBits(uint8_t *p, uint8_t COLUMN_START_LO) ALWAYS_INLINE;

// NOTE: ST7565 SCLK min period is 50ns (100ns?)
// single bit write takes 5 cycles = 312.5ns @16MHz clock
static void lcdSendBit(uint8_t b, uint8_t v0, uint8_t v1)
{
  PORTC_LCD_CTRL = v0;                  // out 0x15, r19  ; 1 cycle
  if (b != 0)                           // sbrc r24, 7    ; 1 cycle
    PORTC_LCD_CTRL = v1;                // out 0x15, r18  ; 1 cycle
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_SCL); // sbi 0x15, 4    ; 2 cycles
}

static void lcdSend8bits(uint8_t val, uint8_t v0, uint8_t v1)
{
  lcdSendBit((val & 0x80), v0, v1);
  lcdSendBit((val & 0x40), v0, v1);
  lcdSendBit((val & 0x20), v0, v1);
  lcdSendBit((val & 0x10), v0, v1);
  lcdSendBit((val & 0x08), v0, v1);
  lcdSendBit((val & 0x04), v0, v1);
  lcdSendBit((val & 0x02), v0, v1);
  lcdSendBit((val & 0x01), v0, v1);
}

static void lcdSendCtlBits(uint8_t val)
{
  uint8_t v0c = 0xC5; // PC7=1,PC6=1,SI=0,SCL=0,A0=0,RES=1,CS1=0,PC0=1
  uint8_t v1c = 0xE5; // PC7=1,PC6=1,SI=1,SCL=0,A0=0,RES=1,CS1=0,PC0=1
  for (uint8_t n = 8; n > 0; n--) {
    lcdSendBit((val & 0x80), v0c, v1c);
    val <<= 1;
  }
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_CS1);   // disable chip select
}

static void lcdSendDataBits(uint8_t *p, uint8_t COLUMN_START_LO)
{
  uint8_t v0c = 0xC5; // PC7=1,PC6=1,SI=0,SCL=0,A0=0,RES=1,CS1=0,PC0=1
  uint8_t v1c = 0xE5; // PC7=1,PC6=1,SI=1,SCL=0,A0=0,RES=1,CS1=0,PC0=1
  uint8_t v0d = 0xCD; // PC7=1,PC6=1,SI=0,SCL=0,A0=1,RES=1,CS1=0,PC0=1
  uint8_t v1d = 0xED; // PC7=1,PC6=1,SI=1,SCL=0,A0=1,RES=1,CS1=0,PC0=1
  for(uint8_t y=0xB0; y < 0xB8; y++) {
    lcdSend8bits(COLUMN_START_LO, v0c, v1c);
    lcdSend8bits(0x10, v0c, v1c);  //column addr 0
    lcdSend8bits(y, v0c, v1c);     //page addr y

    for(uint8_t x=32; x>0; x--){
       lcdSend8bits(*p++, v0d, v1d);
       lcdSend8bits(*p++, v0d, v1d);
       lcdSend8bits(*p++, v0d, v1d);
       lcdSend8bits(*p++, v0d, v1d);
    }
  }
}
#endif // (SERIAL_LCD || LCD_EEPE)

#if (!SERIAL_LCD || LCD_EEPE) // PARALLEL_LCD
static void lcdSendByte(uint8_t val) ALWAYS_INLINE;
static void lcdSendDataBytes(uint8_t *p, uint8_t COLUMN_START_LO) ALWAYS_INLINE;

static void lcdSendByte(uint8_t val)
{
  PORTA_LCD_DAT = val;
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E);    // rise enable
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E);    // fall enable
}

static void lcdSendCtlByte(uint8_t val)
{
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1);  // enable chip select
  //PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);  // enable write 
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_A0);   // set to control mode
  lcdSendByte(val);
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_CS1);   // disable chip select
}

static void lcdSendDataBytes(uint8_t *p, uint8_t COLUMN_START_LO)
{
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1);  // enable chip select
  //PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);  // enable write 
  for(uint8_t y=0xB0; y < 0xB8; y++) {
    PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_A0); // switch to ctl send mode

    lcdSendByte(COLUMN_START_LO);
    lcdSendByte(0x10);  //column addr 0
    lcdSendByte(y);     //page addr y

    PORTC_LCD_CTRL |= (1<<OUT_C_LCD_A0);  // switch to data send mode

    for(uint8_t x=32; x>0; x--){
       lcdSendByte(*p++);
       lcdSendByte(*p++);
       lcdSendByte(*p++);
       lcdSendByte(*p++);
    }
  }
}
#endif // (!SERIAL_LCD || LCD_EEPE)

#if !LCD_EEPE   // compile time LCD configuration

#if (_SSD1306 || ROTATE_SCREEN)
 #ifdef COOLLVSE
  #define COLUMN_START_LO 0x02        // skip first 2 columns
 #else
  #define COLUMN_START_LO 0x00
 #endif
#else  // ST7565
  #define COLUMN_START_LO 0x04        // skip first 4 columns
#endif

#if SERIAL_LCD
inline void lcdSendCtl(uint8_t val) { lcdSendCtlBits(val); }
#else
inline void lcdSendCtl(uint8_t val) { lcdSendCtlByte(val); }
#endif

const static prog_uchar APM Lcdinit[] =
{
#if _SSD1306
  0xAE,         // DON = 0: display OFF
  0xD5, 0x80,   // set display clock 100 frames/sec
  0xA8, 0x3F,   // set multiplex ratio 1/64 duty
  0xD3, 0x00,   // set display offset 0
  0x8D, 0x14,   // enable embedded DC/DC conveter
  0xD9, 0xF1,   // set precharge 15 clocks, discharge 1 clock
  0xDA, 0x12,   // set COM pins hardware configuration
  0xDB, 0x40,   // set VCOMH deselect level -undocumented
# if ROTATE_SCREEN
  0xA1,         // ADC = 1: reverse direction(SEG128->SEG1)
  0xC8,         // SHL = 1: reverse direction (COM64->COM1)
# else
  0xA0,         // ADC = 0: normal direction(SEG1->SEG128)
  0xC0,         // SHL = 0: normal direction (COM1->COM64)
# endif
#else  // !_SSD1306 == ST7565 (stock LCD controller)
  0xE2,         // Initialize the internal functions
  0xAE,         // DON = 0: display OFF
  0xA4,         // Disable entire display-ON
  0xA2,         // Select LCD bias=0
  0x2F,         // Control power circuit operation VC=VR=VF=1
  0x25,         // Select int resistance ratio R2 R1 R0 =5
# if ROTATE_SCREEN
  0xA0,         // ADC = 0: normal direction(SEG1->SEG132/SEG128)
  0xC8,         // SHL = 1: reverse direction (COM64->COM1)
# else
  0xA1,         // ADC = 1: reverse direction(SEG132/SEG128->SEG1)
  0xC0,         // SHL = 0: normal direction (COM1->COM64)
# endif
#endif // _SSD1306
#if REVERSE_VIDEO
  0xA7,         // REV = 1: reverse display
#else
  0xA6,         // REV = 0: normal display
#endif
  0xAF          // DON = 1: display ON
};	


void lcd_init()
{
  LcdLock = 1 ;            // Lock LCD data lines
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RES);  //LCD_RES
  delay_2us();
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RES);
  delay_1_5us(1500);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);  // permanently enable LCD_WR 
  for (uint8_t i = 0; i < sizeof(Lcdinit); i++) {
    lcdSendCtl(pgm_read_byte(&Lcdinit[i]));
  }
  lcdSetContrast();
//  LcdLock = 0 ;            // Free LCD data lines
}

void lcdSetRefVolt(uint8_t val)
{
  LcdLock = 1 ;            // Lock LCD data lines
  lcdSendCtl(0x81);
#if _SSD1306
  lcdSendCtl((val << 2) + 3);  // [3-255]
#else
  lcdSendCtl(val);             // [0-63]
#endif
  LcdLock = 0 ;            // Free LCD data lines
}

void refreshDiplay()
{
	if ( EepromActive && BLINK_ON_PHASE )
	{
		lcd_hline( 0, 0, EepromActive - '0' + 6 ) ;
	}
#ifdef SIMU
  memcpy(lcd_buf, DisplayBuf, sizeof(DisplayBuf));
  lcd_refresh = true;

#else
  LcdLock = 1 ;             // Lock LCD data lines
  uint8_t *p = DisplayBuf;
#if SERIAL_LCD
 #ifdef LCD_OFFSET
  lcdSendDataBits(p, LCD_OFFSET);
 #else
  lcdSendDataBits(p, COLUMN_START_LO);
 #endif
#else
  lcdSendDataBytes(p, COLUMN_START_LO);
#endif
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_CS1);   // disable chip select
  LcdLock = 0 ;            // Free LCD data lines
#endif
}

#else		// LCD_EEPE: configurable LCD driver

static uint8_t Lcdinit[] =
{
  0xE2,         // Initialize the internal functions
  0xAE,         // DON = 0: display OFF
  0xA4,         // Disable entire display-ON
  0xA2,         // Select LCD bias=0
  0x2F,         // Control power circuit operation VC=VR=VF=1
  0x25          // Select int resistance ratio R2 R1 R0 =5
};

static uint8_t SSD1306init[] =
{
  0xAE,         // DON = 0: display OFF
  0xD5, 0x80,   // set display clock 100 frames/sec
  0xA8, 0x3F,   // set multiplex ratio 1/64 duty
  0xD3, 0x00,   // set display offset 0
  0x8D, 0x14,   // enable embedded DC/DC conveter
  0xD9, 0xF1,   // set precharge 15 clocks, discharge 1 clock
  0xDA, 0x12,   // set COM pins hardware configuration
  0xDB, 0x40    // set VCOMH deselect level -undocumented
};

static void (*lcdSendCtl)(uint8_t val);	// function pointer

static void lcdSendCtl2(uint8_t c1, uint8_t c2)
{
  lcdSendCtl(c1);
  lcdSendCtl(c2);
}

void lcdSetOrientation()
{
  lcdSendCtl(0xAE);             // turn-off
  if (g_eeGeneral.SSD1306) {
    if (g_eeGeneral.rotateScreen) {
      lcdSendCtl2(0xA1, 0xC8);  // ADC = 1: reverse direction(SEG128->SEG1)
    } else {                    // SHL = 1: reverse direction(COM64->COM1)
      lcdSendCtl2(0xA0, 0xC0);  // ADC = 0: normal direction(SEG1->SEG128)
    }
  } else {
    if (g_eeGeneral.rotateScreen) {
      lcdSendCtl2(0xA0, 0xC8);  // ADC = 0: norm direction(SEG1->SEG132/SEG128)
    } else {                    // SHL = 1: rev direction(COM64->COM1)
      lcdSendCtl2(0xA1, 0xC0);  // ADC = 1: rev direction(SEG132/SEG128->SEG1)
    }                           // SHL = 0: norm direction(COM1->COM64)
  }
  lcdSendCtl(0xAF);             // turn-on
}

void lcd_init()
{
  LcdLock = 1 ;                 // Lock LCD data lines
  lcdSendCtl = lcdSendCtlByte;  // initialize lcdSendCtl function pointer
  if (g_eeGeneral.serialLCD)
    lcdSendCtl = lcdSendCtlBits;
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RES);  //LCD_RES
  delay_2us();
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RES);
  delay_1_5us(1500);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);  // permanently enable LCD_WR 
  if (g_eeGeneral.SSD1306) {
    for (uint8_t i = 0; i < sizeof(SSD1306init); i++) {
      lcdSendCtl(SSD1306init[i]);
    }
    if (g_eeGeneral.rotateScreen) {
      lcdSendCtl2(0xA1, 0xC8);  // ADC = 1: reverse direction(SEG128->SEG1)
    } else {                    // SHL = 1: reverse direction(COM64->COM1)
      lcdSendCtl2(0xA0, 0xC0);  // ADC = 0: normal direction(SEG1->SEG128)
    }                           // SHL = 0: normal direction(COM1->COM64)
  } else {
    for (uint8_t i = 0; i < sizeof(Lcdinit); i++) {
      lcdSendCtl(Lcdinit[i]);
    }
    if (g_eeGeneral.rotateScreen) {
      lcdSendCtl2(0xA0, 0xC8);  // ADC = 0: norm direction(SEG1->SEG132/SEG128)
    } else {                    // SHL = 1: rev direction(COM64->COM1)
      lcdSendCtl2(0xA1, 0xC0);  // ADC = 1: rev direction(SEG132/SEG128->SEG1)
    }                           // SHL = 0: norm direction(COM1->COM64)
  }
#if REVERSE_VIDEO
  lcdSendCtl2(0xA7, 0xAF);      // REV = 1: reverse display, DON = 1: display ON
#else
  lcdSendCtl2(0xA6, 0xAF);      // REV = 0: normal display, DON = 1: display ON
#endif
  lcdSetContrast();
//  LcdLock = 0 ;                 // Free LCD data lines
}

void lcdSetRefVolt(uint8_t val)
{
  LcdLock = 1 ;           // Lock LCD data lines
  lcdSendCtl(0x81);
  if (g_eeGeneral.SSD1306) {
    lcdSendCtl((val << 2) + 3);  // [3-255]
  } else {
    lcdSendCtl(val);             // [1-63]
  }
  LcdLock = 0 ;           // Free LCD data lines
}

void refreshDiplay()
{
	if ( EepromActive && BLINK_ON_PHASE )
	{
		lcd_hline( 0, 0, EepromActive - '0' + 6 ) ;
	}
#ifdef SIMU
  memcpy(lcd_buf, DisplayBuf, sizeof(DisplayBuf));
  lcd_refresh = true;

#else
  LcdLock = 1 ;            		// Lock LCD data lines
  uint8_t column_start_lo = 0x04; // skip first 4 columns for normal ST7565
  if (g_eeGeneral.rotateScreen || g_eeGeneral.SSD1306)
    column_start_lo = 0x00;       // don't skip if SSD1306 or screen rotated
  uint8_t *p = DisplayBuf;
  if (g_eeGeneral.serialLCD) {
    lcdSendDataBits(p, column_start_lo);
  } else {
    lcdSendDataBytes(p, column_start_lo);
  }
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_CS1);    // disable chip select
  LcdLock = 0 ;            // Free LCD data lines
#endif
}

#endif	// !LCD_EEPE


#else	// !defined(LCD_OTHER)


#ifndef LCD_2_CS
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
#if defined(CPUM128) || defined(CPUM2561)
  0xE2, 0xAE, 0xA6, 0xA4, 0xA2, 0x2F, 0x25
#else
  0xE2, 0xAE, 0xA1, 0xA6, 0xA4, 0xA2, 0xC0, 0x2F, 0x25, 0xAF
#endif
} ;	

#if defined(CPUM128) || defined(CPUM2561)
void lcdSetOrientation()
{
  lcdSendCtl(0xAE);       // turn-off display
  if (g_eeGeneral.rotateScreen) {
    lcdSendCtl(0xA0);     // ADC = 0: norm direction(SEG1->SEG132/SEG128)
    lcdSendCtl(0xC8);     // SHL = 1: rev direction(COM64->COM1)
  } else {
    lcdSendCtl(0xA1);     // ADC = 1: rev direction(SEG132/SEG128->SEG1)
    lcdSendCtl(0xC0);     // SHL = 0: norm direction(COM1->COM64)
  }
  lcdSendCtl(0xAF);       // turn-on display
}
#endif

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
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);  // permanently enable LCD_WR 

	for ( i = 0 ; i < sizeof(Lcdinit) ; i += 1 )
	{
	  lcdSendCtl(pgm_read_byte(&Lcdinit[i]) ) ;
	}
#if defined(CPUM128) || defined(CPUM2561)
  lcdSetOrientation();
#endif
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
//volatile uint8_t LcdTrims ;
//uint8_t LcdTrimSwapped ;

#include "audio.h"
//extern uint16_t SerialVoiceDebug ;

void refreshDiplay()
{
//	lcd_putc( 20*FW, 0, RotaryState + 'A' ) ;
//	lcd_putc( 19*FW, 0, s_editMode + '0' ) ;
	if ( EepromActive && BLINK_ON_PHASE )
	{
		lcd_hline( 0, 0, EepromActive - '0' + 6 ) ;
	}

//extern uint8_t SaveBusy ;
//lcd_outhex4( 0, 0, DDRD ) ;
//lcd_outhex4( 0, 0, Voice.VoiceSerialRxState ) ;
//lcd_outhex4( 25, 0, Voice.VoiceSerialValue ) ;
//lcd_outhex4( 25, 0, PIND ) ;
//lcd_outhex4( 50, 0, PORTD ) ;
//lcd_outhex4( 50, 0, SaveBusy ) ;
//lcd_outhex4( 75, 0, Voice.VoiceState ) ;
//lcd_outhex4( 100, 0, Voice.VoiceDebug ) ;
//lcd_outhex4( 75, 0, (UCSR1B<<8) ) ;
//lcd_outhex4( 50, 0, (UCSR1B<<8) | ( SerialVoiceDebug & 0x00FF ) ) ;

#ifdef SIMU
  memcpy(lcd_buf, DisplayBuf, sizeof(DisplayBuf));
  lcd_refresh = true;
#else

	LcdLock = 1 ;						// Lock LCD data lines
  uint8_t column_start_lo = 0x04; // skip first 4 columns for normal ST7565
#if defined(CPUM128) || defined(CPUM2561)
  if (g_eeGeneral.rotateScreen)
    column_start_lo = 0x00;       // don't skip if screen is rotated
#endif
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
#endif

#ifdef LCD_2_CS
uint8_t toggle_e()
{
  uint8_t value ;
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_E) ;
  asm("rjmp .+0") ;
  asm("rjmp .+0") ;
  asm("nop") ;
  value = PINA ;
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_E) ;
  return value ;
}


#define delay_1us() _delay_us(1)
static void delay_1_5us(int ms)
{
  for(int i=0; i<ms; i++) delay_1us();
}

#define OUT_C_LCD_CS2   0

static void lcdSendCtl(uint8_t val)
{
  uint8_t busy ;

  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1);

  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RnW) ;    // read
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_A0);
  DDRA = 0 ;
  do
  {
    busy = toggle_e() ;
  } while ( busy & 0x80 ) ;
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
  DDRA = 0xFF ;
  PORTA_LCD_DAT = val;
  toggle_e() ;
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1);

  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS2);
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RnW) ;    // read
  DDRA = 0 ;
  do
  {
    busy = toggle_e() ;
  } while ( busy & 0x80 ) ;
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
  DDRA = 0xFF ;
  PORTA_LCD_DAT = val;
  toggle_e() ;
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS2);

}


uint8_t Hpos ;

static void lcdSendDat(uint8_t val)
{
  uint8_t busy ;

  if ( Hpos )
  {
    PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS1) ;
  }
  else
  {
    PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_CS2) ;
  }
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RnW) ;    // read
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_A0);
  DDRA = 0 ;
  do
  {
    busy = toggle_e() ;
  } while ( busy & 0x80 ) ;
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RnW);
  DDRA = 0xFF ;
  PORTA_LCD_DAT = val;
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
  toggle_e() ;

  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1);
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS2);
}


void lcd_init()
{
  // /home/thus/txt/datasheets/lcd/KS0713.pdf
  // ~/txt/flieger/ST7565RV17.pdf  from http://www.glyn.de/content.asp?wdid=132&sid=

  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS1);
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_CS2);
  PORTC_LCD_CTRL |=  (1<<OUT_C_LCD_A0);
  PORTC_LCD_CTRL &= ~(1<<OUT_C_LCD_RES);  //LCD_RES
  delay_1us();
  delay_1us();//    f520  call  0xf4ce  delay_1us() ; 0x0xf4ce
  PORTC_LCD_CTRL |= (1<<OUT_C_LCD_RES); //  f524  sbi 0x15, 2 IOADR-PORTC_LCD_CTRL; 21           1

  lcdSendCtl(0xC0); //
  lcdSendCtl(0x3F); //DON = 1: display ON

  lcdSetRefVolt(g_eeGeneral.contrast);
}

void lcdSetRefVolt(uint8_t val)
{
	LcdLock = 1 ;						// Lock LCD data lines
  lcdSendCtl(0x81);
  lcdSendCtl(val);
	LcdLock = 0 ;						// Free LCD data lines
}

volatile uint8_t LcdLock ;
//volatile uint8_t LcdTrims ;
//uint8_t LcdTrimSwapped ;


void refreshDiplay()
{
  uint8_t *p=DisplayBuf;

	LcdLock = 1 ;						// Lock LCD data lines

  for(uint8_t y=0; y < 8; y++) {
    lcdSendCtl(0xC0); //
    lcdSendCtl(0x40); //column addr 0
    lcdSendCtl( y | 0xB8); //page addr y

    for(uint8_t x=0; x<128; x++)
    {
      Hpos = x & 64 ;
      lcdSendDat(*p);
      p++;
    }
//    PORTA = 0xFF;  // Outputs high/pullups enabled
  }
	LcdLock = 0 ;						// Free LCD data lines
}
#endif

#endif  // LCD_OTHER


