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
#ifndef lcd_h
#define lcd_h


#if LCD_OTHER
 #define LCD_MINCONTRAST 0
 #define LCD_MAXCONTRAST 63
 #define LCD_NOMCONTRAST 25
#else
 #define LCD_MINCONTRAST 10
 #define LCD_MAXCONTRAST 45
 #define LCD_NOMCONTRAST 25
#endif


#define DISPLAY_W 128
#define DISPLAY_H  64
#define FW          6
#define FWNUM       5
#define FH          8

/* lcd common flags */
#define INVERS        0x01
#define BLINK         0x02
#define DBLSIZE       0x04
#define CONDENSED     0x08

/* lcd puts flags */
#define BSS           0x10
// putsChnRaw flags
#define MIX_SOURCE    0x10

/* lcd outdez flags */
#define LEADING0      0x10
#define PREC1         0x20
#define PREC2         0x30 /* 4 modes in 2bits! */
#define LEFT          0x40 /* align left */

/* time & telemetry flags */
#define NO_UNIT       0x80

extern uint8_t Lcd_lastPos;

#define PLOT_XOR		0
#define PLOT_BLACK	1
#define PLOT_WHITE	2

extern uint8_t plotType ;

//extern unsigned char font_5x8_x20_x7f[];
extern unsigned char displayBuf[DISPLAY_W*DISPLAY_H/8];

extern uint8_t lcd_putcAtt(unsigned char x,unsigned char y,const char c,uint8_t mode);
extern unsigned char lcd_putsAtt(unsigned char x,unsigned char y,const prog_char * s,uint8_t mode);
//extern uint8_t lcd_puts2Att(uint8_t x,uint8_t y,const prog_char * s,const prog_char * t ,uint8_t mode);
//extern void lcd_putsAttIdx_right( uint8_t y,const prog_char * s,uint8_t idx,uint8_t att) ;
extern void lcd_putsAttIdx(uint8_t x,uint8_t y,const prog_char * s,uint8_t idx,uint8_t att) ;
extern void lcd_putsnAtt(unsigned char x,unsigned char y,const prog_char * s,unsigned char len,uint8_t mode);

extern uint8_t lcd_putc(uint8_t x,uint8_t y,const char c);
extern void lcd_puts_Pleft(uint8_t y,const prog_char * s) ;
extern void lcd_puts_Pskip(uint8_t y,const prog_char * s, uint8_t skip) ;
extern void lcd_puts_P(unsigned char x,unsigned char y,const prog_char * s);
extern void lcd_putsn_P(unsigned char x,unsigned char y,const prog_char * s,unsigned char len);
extern void lcd_outhex4(unsigned char x,unsigned char y,uint16_t val);
extern void lcd_outdezAtt(unsigned char x,unsigned char y,int16_t val,uint8_t mode);
extern void lcd_2_digits( uint8_t x, uint8_t y, uint8_t value, uint8_t attr ) ;
uint8_t lcd_outdezNAtt(uint8_t x,uint8_t y,int32_t val,uint8_t mode,int8_t len);
//extern void lcd_outdezAtt(unsigned char x,unsigned char y,int16_t val,uint8_t mode);
extern void lcd_outdez(unsigned char x,unsigned char y,int16_t val);

extern void lcd_hbar( uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percent ) ;
extern void lcd_plot(unsigned char x,unsigned char y);
extern void lcd_hline(unsigned char x,unsigned char y, signed char w);
extern void lcd_hlineStip(unsigned char x,unsigned char y, signed char w,uint8_t pat);
extern void lcd_vline(unsigned char x,unsigned char y, signed char h);
extern void lcd_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h ) ;
extern void lcd_rect_xor(uint8_t x, uint8_t y, uint8_t w, uint8_t h ) ;
extern void lcd_char_inverse( uint8_t x, uint8_t y, uint8_t w, uint8_t blink ) ;

//extern void lcd_img_f(unsigned char x,unsigned char y);
extern void lcd_img(uint8_t i_x,uint8_t i_y,const prog_uchar * imgdat,uint8_t idx /*,uint8_t mode*/);

extern void lcd_init();
extern void lcd_clear();
extern void refreshDiplay();
extern void lcdSetContrast( void ) ;
extern void lcdSetRefVolt(unsigned char val);
#define BLINK_ON_PHASE (g_blinkTmr10ms & (1<<6))
//#define BLINK_SYNC      g_blinkTmr10ms = (3<<5)
#define BLINK_SYNC
#endif
/*eof*/
