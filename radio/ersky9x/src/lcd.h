/*
 * Author - Mike Blandford
 *
 * Based on er9x by Erez Raviv <erezraviv@gmail.com>
 *
 * which was Based on th9x -> http://code.google.com/p/th9x/
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

#ifndef lcd_h
#define lcd_h

#ifdef PCBX9D
#if defined(PCBX7) || defined(PCBX9LITE)
#define DISPLAY_W 128
#define DISPLAY_H  64
extern uint8_t ExtDisplayBuf[DISPLAY_W*DISPLAY_H/8 + 2] ;
extern uint16_t ExtDisplayTime ;
extern uint8_t ExtDisplaySend ;
#else // PCBX7
#define DISPLAY_W 212
#define DISPLAY_H  64
#endif
#endif // PCBX7

#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBLEM1)
#define DISPLAY_W 128
#define DISPLAY_H  64
#endif

#if defined(PCBX12D) || defined(PCBX10)
#define DISPLAY_W	480
#define DISPLAY_H	272

#define LCD_RED			0xF800
#define LCD_GREEN		0x07E0
#define LCD_BLUE		0x001F
#define LCD_WHITE		0xFFFF
#define LCD_BLACK		0
#define LCD_GREY		0x8410
#define LCD_STATUS_GREY	0x3BEF
#define LCD_LIGHT_GREY	0xC618
#define LCD_CYAN		0x07FF
#define LCD_MAGENTA	0xF81F
#define LCD_YELLOW	0xFFE0

//#define	LCD_BACKGROUND	LCD_LIGHT_GREY
#define	LCD_BACKGROUND	0xFFFF

extern uint16_t LcdBackground ;
extern uint16_t LcdForeground ;
extern uint16_t LcdCustomColour ;

#if defined(PCBX10)
#define INVERT_DISPLAY		1
#endif

#endif

#ifndef LCD_W
#define LCD_W	DISPLAY_W
#endif
#ifndef LCD_H
#define LCD_H DISPLAY_H
#endif


#define FW          6
#define FWNUM       5
#define FH          8
#define FHDC        12

/* lcd common flags */
#define INVERS        0x01
#define BLINK         0x02
#define DBLSIZE       0x04
#define CONDENSED     0x08
#define BOLD          0x80

// putsChnRaw flags
#define MIX_SOURCE    0x10

// putTxSwr flags
#define TSSI_TEXT			0x20

/* lcd outdez flags */
#define LEADING0      0x0400
#define PREC1         0x20
#define PREC2         0x30 /* 4 modes in 2bits!, now removed */
#define LEFT          0x40 /* align left */

/* time & telemetry flags */
#define NO_UNIT       0x80

/* line, rect, square flags */
#define FORCE                          0x02
#define ERASE                          0x04
#define ROUND                          0x08

#define SOLID                          0xff
#define DOTTED                         0x55

// GVAR flags
#define GVAR_100			0x100
#define GVAR_250			0x200


extern uint8_t LcdLock ;
extern uint16_t LcdInputs ;

extern uint8_t Lcd_lastPos;
extern uint8_t DisplayBuf[] ;

extern const uint8_t font_se_extra[] ;
extern const uint8_t font_fr_extra[] ;
extern const uint8_t font_de_extra[] ;
extern const uint8_t font_it_extra[] ;
extern const uint8_t font_pl_extra[] ;

extern const uint8_t font_se_big_extra[] ;
extern const uint8_t font_fr_big_extra[] ;
extern const uint8_t font_de_big_extra[] ;

extern const uint8_t *ExtraFont ;
extern const uint8_t *ExtraBigFont ;

#if defined(PCBX12D) || defined(PCBX10)

#ifdef INVERT_DISPLAY
extern uint8_t *font_fr_h_extra ;
extern uint8_t *font_fr_h_big_extra ;
extern uint8_t *font_de_h_extra ;
extern uint8_t *font_de_h_big_extra ;
extern uint8_t *font_se_h_extra ;
extern uint8_t *font_se_h_big_extra ;
extern uint8_t *font_it_h_extra ;
extern uint8_t *font_pl_h_extra ;
#else
extern const uint8_t font_fr_h_extra[] ;
extern const uint8_t font_fr_h_big_extra[] ;
extern const uint8_t font_de_h_extra[] ;
extern const uint8_t font_de_h_big_extra[] ;
extern const uint8_t font_se_h_extra[] ;
extern const uint8_t font_se_h_big_extra[] ;
extern const uint8_t font_it_h_extra[] ;
extern const uint8_t font_pl_h_extra[] ;
#endif

extern const uint8_t *ExtraHorusFont ;
extern const uint8_t *ExtraHorusBigFont ;
#endif

#define PLOT_XOR		0
#define PLOT_BLACK	1
#define PLOT_WHITE	2
//#define PLOT_BITS		3
#define PLOT_COLOUR	3
#define PLOT_CUSTOM	4
extern uint8_t plotType ;

void pushPlotType( uint8_t type ) ;
void popPlotType( void ) ;

extern uint8_t lcd_putc(uint8_t x,uint8_t y,const char c ) ;
extern void lcd_putsAttIdx(uint8_t x,uint8_t y,const char * s,uint8_t idx,uint8_t att) ;
extern void lcd_putsnAtt(uint8_t x,uint8_t y,const char * s,uint8_t len,uint8_t mode) ;
extern void lcd_putsn_P(uint8_t x,uint8_t y,const char * s,uint8_t len) ;
#if defined(PCBX12D) || defined(PCBX10)
extern void lcd_outhex4(uint16_t x,uint8_t y,uint16_t val) ;
#else
extern void lcd_outhex4(uint8_t x,uint8_t y,uint16_t val) ;
#endif
extern void lcd_outhex2(uint8_t x,uint8_t y,uint8_t val) ;
extern uint8_t lcd_putsAtt( uint8_t x, uint8_t y, const char *s, uint8_t mode ) ;
extern void lcd_puts_Pleft( uint8_t y, const char *s ) ;
extern void lcd_puts_P( uint8_t x, uint8_t y, const char *s ) ;
#if defined(PCBX12D) || defined(PCBX10)
extern void lcd_img( uint8_t i_x, uint8_t i_y, const unsigned char *imgdat, uint8_t idx, uint8_t mode, uint16_t colour = LCD_BLACK, uint16_t background = LcdBackground ) ;
extern void lcd_bitmap( uint8_t i_x, uint8_t i_y, const unsigned char *bitmap, uint8_t w, uint8_t h, uint8_t mode, uint16_t colour = LCD_BLACK, uint16_t background = LcdBackground) ;
extern void lcd_2_digits( uint16_t x, uint16_t y, uint8_t value, uint16_t attr, uint16_t colour = LCD_BLACK, uint16_t background = LcdBackground ) ;
extern void lcd_outdez( uint16_t x, uint16_t y, int16_t val, uint16_t background = LcdBackground  ) ;
extern void lcd_outdezAtt( uint16_t x, uint16_t y, int16_t val, uint16_t mode, uint16_t background = LcdBackground  ) ;
extern uint8_t lcd_outdezNAtt( uint16_t x, uint16_t y, int32_t val, uint16_t mode, int8_t len, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern uint16_t lcd_putcAtt( uint16_t x, uint16_t y, const char c, uint8_t mode, uint16_t background = LcdBackground ) ;
#else
extern void lcd_img( uint8_t i_x, uint8_t i_y, const unsigned char *imgdat, uint8_t idx, uint8_t mode ) ;
extern void lcd_bitmap( uint8_t i_x, uint8_t i_y, const unsigned char *bitmap, uint8_t w, uint8_t h, uint8_t mode ) ;
extern void lcd_2_digits( uint8_t x, uint8_t y, uint8_t value, uint16_t attr ) ;
extern void lcd_outdez( uint8_t x, uint8_t y, int16_t val ) ;
extern void lcd_outdezAtt( uint8_t x, uint8_t y, int16_t val, uint16_t mode ) ;
extern uint8_t lcd_outdezNAtt( uint8_t x, uint8_t y, int32_t val, uint16_t mode, int8_t len ) ;
#endif

extern void lcd_hbar( uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percent ) ;
extern void lcd_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h ) ;
extern void lcd_line( uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t pat, uint8_t att ) ;
#if defined(PCBX12D) || defined(PCBX10)
extern "C" void startLcdDrawSolidFilledRectDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) ;
extern "C" void lcdDrawSolidFilledRectDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) ;
extern "C" void waitDma2Ddone(void) ;
extern void lcdDrawCharBitmapDma( uint16_t x, uint16_t y, uint8_t chr, uint32_t mode, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern void lcd_clearBackground( void ) ;
extern void lcd_blank( void ) ;
extern void waitLcdClearDdone( void ) ;
extern uint16_t lcd_putcAttColour(uint16_t x,uint16_t y,const char c,uint8_t mode, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern void lcd_putsnAttColour( uint16_t x, uint16_t y, const char * s,uint8_t len, uint8_t mode, uint16_t colour = LcdForeground ) ;
extern uint16_t lcd_putcAttDblColour(uint16_t x,uint16_t y,const char c,uint8_t mode, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern void lcdDrawCharBitmapDoubleDma( uint16_t x, uint16_t y, uint8_t chr, uint32_t mode, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern void lcd_hlineStip( uint16_t x, uint16_t y, uint8_t w, uint8_t pat ) ;
extern void lcd_hline( uint16_t x, uint16_t y, int8_t w ) ;
extern void lcd_vline( uint16_t x, uint16_t y, int8_t h ) ;
extern void lcd_plot( uint16_t x, uint16_t y ) ;
extern void lcd_char_inverse( uint16_t x, uint16_t y, uint16_t w, uint8_t blink, uint8_t h = 8 ) ;
extern uint16_t lcd_putcAttSmall(uint16_t x,uint16_t y,const char c,uint8_t mode, uint16_t colour = LcdForeground ) ;
extern void lcdDrawFilledRect( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t pat, uint8_t att ) ;
extern void DMAcopyImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *image ) ;
#else
extern uint8_t lcd_putcAtt( uint8_t x, uint8_t y, const char c, uint8_t mode ) ;
extern void lcd_hlineStip( unsigned char x, unsigned char y, int16_t w, uint8_t pat ) ;
extern void lcd_hline( uint8_t x, uint8_t y, int8_t w ) ;
extern void lcd_vline( uint8_t x, uint8_t y, int8_t h ) ;
extern void lcd_plot( uint8_t x, uint8_t y ) ;
extern void lcd_char_inverse( uint8_t x, uint8_t y, uint8_t w, uint8_t blink ) ;
extern void lcdDrawFilledRect( uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t pat, uint8_t att ) ;
#endif
extern void lcd_clear( void ) ;
extern void lcdInit( void ) ;
extern void lcdSetRefVolt(uint8_t val) ;
extern void lcdSendCtl(uint8_t val) ;
extern void refreshDisplay( void ) ;
extern void lcdSetContrast( void ) ;
extern void lcdSetOrientation( void ) ;

#ifdef PCBX7
extern void lcdOff( void ) ;
#endif // PCBX7

#ifdef PCBX9LITE
extern void lcdOff( void ) ;
#endif // PCBX9LITE

#if defined(PCBX12D) || defined(PCBX10)
extern void putsTime(uint16_t x,uint16_t y,int16_t tme,uint8_t att,uint8_t att2, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
#else
extern void putsTime(uint8_t x,uint8_t y,int16_t tme,uint8_t att,uint8_t att2) ;
#endif
extern void putsVolts(uint8_t x,uint8_t y, uint8_t volts, uint8_t att) ;
extern void putsVBat(uint8_t x,uint8_t y,uint8_t att) ;

#ifdef PCBX9D
#if defined(REVPLUS) || defined(REV9E)
extern void backlight_set( uint16_t brightness, uint16_t w_or_b ) ;
#else
extern void backlight_set( uint16_t brightness ) ;
#endif
#endif
#if defined(PCBX12D) || defined(PCBX10)
extern void backlight_set( uint16_t brightness ) ;
#endif
#if defined(PCBLEM1)
extern void backlight_set( uint16_t brightness ) ;
#endif


#define BLINK_ON_PHASE (g_blinkTmr10ms & (1<<6))
#define BLINK_SYNC      g_blinkTmr10ms = (3<<5)

#endif


