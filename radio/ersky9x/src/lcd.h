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
#define MENU_DISPLAY_RIGHT 127
extern uint8_t ExtDisplayBuf[DISPLAY_W*DISPLAY_H/8 + 2] ;
extern uint16_t ExtDisplayTime ;
extern uint8_t ExtDisplaySend ;
#else // PCBX7
#define DISPLAY_W 212
#define DISPLAY_H  64
#define MENU_DISPLAY_RIGHT 127
#endif
#endif // PCBX7

#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBLEM1)
#define DISPLAY_W 128
#define DISPLAY_H  64
#define MENU_DISPLAY_RIGHT 127
#endif


#if defined(PCBX12D) || defined(PCBX10)
#define SCREEN_LINES		12

//#define	WHERE_TRACK		1
//void notePosition( uint8_t byte ) ;

#else
#define SCREEN_LINES		8
#endif

#if defined(PCBX12D) || defined(PCBX10)
#define DISPLAY_W	480
#define DISPLAY_H	272
#define MENU_DISPLAY_RIGHT 478

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

#define POPUP_SPACING		4

// 5 bits for each colour (0-31)
#define LCD_RGB565(r,g,b) ((r<<11)|(g<<6)|((g&16)<<1)|(b))

//#define	LCD_BACKGROUND	LCD_LIGHT_GREY
#define	LCD_BACKGROUND	0xFFFF

extern uint16_t LcdBackground ;
extern uint16_t LcdForeground ;
extern uint16_t LcdCustomColour ;
extern uint16_t DimBackColour ;

#if defined(PCBX10) && !defined(PCBT18)
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
#define FWCOLOUR    12
#define FHCOLOUR    20
#define FWNUMCOLOUR       10

#if defined(PCBX12D) || defined(PCBX10)
#define FWX	FWCOLOUR
#define FHY	FHCOLOUR
#define FHPY	(FHCOLOUR/2)
#define FWPX	(FWCOLOUR/2)
#else
#define FWX	FW
#define FHY	FH
#define FHPY	FH
#endif

//#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
//#define RIGHT_JUST		LUA_RIGHT
//#else
//#define RIGHT_JUST		LUA_RIGHT
//#endif

#if defined(PCBX12D) || defined(PCBX10)
#define HVSCALE		2
#define CHR_INV_OFFSET		1
#else
#define HVSCALE		1
#define CHR_INV_OFFSET		0
#endif

/* lcd common flags */
#define INVERS        0x01
#define BLINK         0x02
#define DBLSIZE       0x04
#define CONDENSED     0x08
#define BOLD          0x80
#define CENTERED			0x20

#define LUA_RIGHT			0x0400
#define LUA_SMLSIZE		0x8000
#define LUA_TEXT_COLOUR	0x0800
#define LUA_TEXT_INVERTED_COLOR	0x1000
#define LUA_TEXT_INVERTED_BGCOLOR	0x2000
#define LUA_CUSTOM_COLOUR	0x4000

// lcdDrawPropChDma flags
#define ICON_CM_A4		0x80000000

// putsChnRaw flags
#define MIX_SOURCE    0x10

// putTxSwr flags
#define TSSI_TEXT			0x20

/* lcd outdez flags */
#define LEADING0      0x0200
#define PREC1         0x20
#define PREC2         0x30 /* 4 modes in 2bits!, now removed */
#define LEFT          0x40 /* align left */
#define MODE(flags)		((((int8_t)(flags) & 0x30) - 0x10) >> 4)

#define XFONT					0x0100
#define FONTSIZE_MASK		(DBLSIZE | CONDENSED | XFONT)
#define FONTSIZE(x)			((x) & FONTSIZE_MASK)


// Text sizes
//#define NORSIZE		0x0000
#define TINSIZE   (XFONT | CONDENSED)
#define SMLSIZE   (XFONT)
#define MIDSIZE   (DBLSIZE | CONDENSED)
//#define DBLSIZE		DBLSIZE
#define XXLSIZE   (DBLSIZE | XFONT)




#define FIXEDWIDTH		0x10
//#define VERTICAL			0x0800

/* time & telemetry flags */
#define NO_UNIT       0x80

#define ZCHAR         0x10u

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

//extern const uint8_t *ExtraHorusFont ;
//extern const uint8_t *ExtraHorusBigFont ;
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

//#ifdef COLOUR_DISPLAY
//extern uint16_t DisplayOffset ;
//#endif

extern uint8_t lcd_putc(coord_t x,coord_t y,const char c ) ;
extern uint8_t lcd_putcSmall( coord_t x, coord_t y, uint8_t c, LcdFlags mode ) ;
extern void lcd_putsnAtt(coord_t x,coord_t y,const char * s,uint8_t len,LcdFlags mode) ;
extern void lcd_putsn_P(coord_t x,coord_t y,const char * s,uint8_t len) ;
#if defined(PCBX12D) || defined(PCBX10) || defined(PCB9XT)
extern void lcd_picture( uint16_t i_x, uint16_t i_y, uint16_t maxHeight, uint16_t maxWidth = 480 ) ;
extern void lcd_outhex8(uint16_t x,uint16_t y,uint32_t val) ;
extern void lcd_outhex4(uint16_t x,uint8_t y,uint16_t val) ;

void lcdHiresRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour ) ;
void lcdHiresHline( uint16_t x, uint16_t y, uint16_t w, uint16_t colour ) ;
void lcdHiresVline( uint16_t x, uint16_t y, uint16_t h, uint16_t colour ) ;
void lcdHiresPlot( uint16_t x, uint16_t y, uint16_t h, uint16_t colour ) ;
uint16_t lcdHiresPutsTransparent( uint16_t x, uint16_t y, char *s, uint16_t colour ) ;
void lcdDrawCharBitmapTransparent( uint16_t x, uint16_t y, uint8_t chr, LcdFlags mode, uint16_t colour ) ;
void lcdDrawCharxxlTransparent( uint16_t x, uint16_t y, uint8_t chr, LcdFlags mode, uint16_t colour ) ;
void lcdDrawCharDoubleTransparent( uint16_t x, uint16_t y, uint8_t chr, LcdFlags mode, uint16_t colour ) ;

#else
extern void lcd_outhex4(coord_t x,coord_t y,uint16_t val) ;
#endif
extern void lcd_putsAttIdx(coord_t x,coord_t y,const char * s,uint8_t idx,LcdFlags att) ;
extern void lcd_outhex2(coord_t x,coord_t y,uint8_t val) ;
extern uint8_t lcd_putsAtt( coord_t x, coord_t y, const char *s, LcdFlags mode ) ;
extern void lcd_puts_Pleft( coord_t y, const char *s ) ;
extern void lcd_puts_P( coord_t x, coord_t y, const char *s ) ;
#if defined(PCBX12D) || defined(PCBX10)
extern void lcd_img( uint8_t i_x, uint8_t i_y, const unsigned char *imgdat, uint8_t idx, LcdFlags mode, uint16_t colour = LCD_BLACK, uint16_t background = LcdBackground ) ;
extern void lcd_HiResimg( uint16_t i_x, uint16_t i_y, const unsigned char *imgdat, uint8_t idx, LcdFlags mode, uint16_t colour = LCD_BLACK, uint16_t background = LcdBackground ) ;
extern void lcd_HiResbitmap( uint16_t i_x, uint16_t i_y, uint8_t *bitmap, uint8_t w, uint8_t h, LcdFlags mode, uint16_t colour = LCD_BLACK, uint16_t background = LcdBackground ) ;
extern void lcd_bitmap( uint8_t i_x, uint8_t i_y, const unsigned char *bitmap, uint8_t w, uint8_t h, LcdFlags mode, uint16_t colour = LCD_BLACK, uint16_t background = LcdBackground) ;
extern void lcd_2_digits( uint16_t x, uint16_t y, uint8_t value, LcdFlags attr, uint16_t colour = LCD_BLACK, uint16_t background = LcdBackground ) ;
//extern void lcd_outdez( uint16_t x, uint16_t y, int16_t val, uint16_t background = LcdBackground  ) ;
//extern void lcd_outdezAtt( uint16_t x, uint16_t y, int16_t val, uint16_t mode, uint16_t background = LcdBackground  ) ;
//extern uint8_t lcd_outdezNAtt( uint16_t x, uint16_t y, int32_t val, uint16_t mode, int8_t len, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern uint16_t lcd_putcAtt( uint16_t x, uint16_t y, const char c, LcdFlags mode, uint16_t background = LcdBackground ) ;
extern void lcd_rectColour(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t colour ) ;
//extern void lcd_putsAttIdxColour(uint8_t x,uint8_t y,const char * s,uint8_t idx,uint8_t att, uint16_t colour = LcdForeground, uint16_t background = LcdBackground) ;
extern uint8_t lcd_putsAttColour( uint8_t x, uint8_t y, const char *s, LcdFlags mode, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern void lcdDrawCharSmall( uint16_t x, uint16_t y, uint8_t chr, LcdFlags mode, uint16_t colour ) ;
extern void lcd_putsSmall( uint16_t x, uint16_t y, uint8_t *p, uint16_t colour ) ;
extern void lcd_putsnSmall( uint16_t x, uint16_t y, uint8_t *p, uint8_t len, uint16_t colour ) ;
#else
extern void lcd_img( coord_t i_x, coord_t i_y, const unsigned char *imgdat, uint8_t idx, LcdFlags mode ) ;
extern void lcd_bitmap( coord_t i_x, coord_t i_y, const unsigned char *bitmap, uint8_t w, uint8_t h, LcdFlags mode ) ;
extern void lcd_2_digits( coord_t x, coord_t y, uint8_t value, LcdFlags attr ) ;
extern void lcd_outdez( coord_t x, coord_t y, int16_t val ) ;
extern void lcd_outdezAtt( coord_t x, coord_t y, int16_t val, LcdFlags mode ) ;
extern uint8_t lcd_outdezNAtt( coord_t x, coord_t y, int32_t val, LcdFlags mode, int8_t len ) ;
#endif

extern void lcd_hbar( coord_t x, coord_t y, uint8_t w, uint8_t h, uint8_t percent ) ;
extern void lcd_rect(coord_t x, coord_t y, uint8_t w, uint8_t h ) ;
extern void lcd_line( coord_t x1, coord_t y1, uint16_t x2, uint16_t y2, uint8_t pat, LcdFlags att ) ;
#if defined(PCBX12D) || defined(PCBX10)
extern "C" void startLcdDrawSolidFilledRectDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) ;
extern "C" void lcdDrawSolidFilledRectDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) ;
extern "C" void waitDma2Ddone(void) ;
extern void lcdDrawCharBitmapDma( uint16_t x, uint16_t y, uint8_t chr, LcdFlags mode, uint16_t colour = LcdForeground ) ;
extern void lcd_clearBackground( void ) ;
extern void lcd_blank( void ) ;
extern void waitLcdClearDdone( void ) ;
extern uint16_t lcd_putcAttColour(uint16_t x,uint16_t y,const char c,LcdFlags mode, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern void lcd_putsnAttColour( uint16_t x, uint16_t y, const char * s,uint8_t len, LcdFlags mode, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern uint16_t lcd_putcAttDblColour(uint16_t x,uint16_t y,const char c,LcdFlags mode, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern void lcdDrawCharBitmapDoubleDma( uint16_t x, uint16_t y, uint8_t chr, LcdFlags mode, uint16_t colour ) ;
extern void lcdDrawCharBitmapMediumDma( uint16_t x, uint16_t y, uint8_t chr, LcdFlags mode, uint16_t colour = LcdForeground ) ;
extern void lcd_hlineStip( uint16_t x, uint16_t y, int16_t w, uint8_t pat ) ;
extern void lcd_hline( uint16_t x, uint16_t y, int16_t w ) ;
extern void lcd_vline( coord_t x, coord_t y, int8_t h ) ;
extern void lcd_plot( uint16_t x, uint16_t y ) ;
extern void lcd_char_inverse( uint16_t x, uint16_t y, uint16_t w, uint8_t blink, uint8_t h = 8 ) ;
extern uint16_t lcd_putcAttSmall(uint16_t x,uint16_t y,const char c,LcdFlags mode, uint16_t colour = LcdForeground ) ;
extern void lcdDrawFilledRect( coord_t x, coord_t y, uint16_t w, uint16_t h, uint8_t pat, LcdFlags att ) ;
extern void DMAcopyImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *image, uint16_t reduceWidth = 0 ) ;
#else
extern uint8_t lcd_putcAtt( coord_t x, coord_t y, const char c, uint8_t mode ) ;
extern void lcd_hlineStip( coord_t x, coord_t y, int16_t w, uint8_t pat ) ;
extern void lcd_hline( coord_t x, coord_t y, int8_t w ) ;
extern void lcd_vline( coord_t x, coord_t y, int8_t h ) ;
extern void lcd_plot( coord_t x, coord_t y ) ;
extern void lcd_char_inverse( coord_t x, coord_t y, uint8_t w, uint8_t blink ) ;
extern void lcdDrawFilledRect( coord_t x, coord_t y, uint8_t w, uint8_t h, uint8_t pat, LcdFlags att ) ;
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
extern void putsTime(uint16_t x,uint16_t y,int16_t tme,LcdFlags att,LcdFlags att2, uint16_t colour = LcdForeground, uint16_t background = LcdBackground ) ;
extern void putsVolts(uint16_t x,uint16_t y, uint8_t volts, LcdFlags att) ;
extern void putsVBat(uint16_t x,uint16_t y,LcdFlags att) ;
#else
extern void putsTime(coord_t x,coord_t y,int16_t tme,LcdFlags att,LcdFlags att2) ;
extern void putsVolts(coord_t x,coord_t y, uint8_t volts, LcdFlags att) ;
extern void putsVBat(coord_t x,coord_t y,LcdFlags att) ;
#endif

#ifdef PCBX9D
#if defined(REVPLUS) || defined(REV9E) || defined(REV19)
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


#if defined(PCBX12D) || defined(PCBX10)
extern coord_t LcdLastRightPos ;
extern coord_t LcdNextPos ;
extern coord_t LcdLastLeftPos ;

uint32_t colourCharWidth( unsigned char ch, LcdFlags flags) ;
void lcdDrawChar( uint16_t x, uint16_t y, uint8_t ch, uint16_t mode, uint16_t colour ) ;
void lcdDrawChar( uint16_t x, uint16_t y, uint8_t ch, uint16_t mode ) ;
void lcdDrawChar( uint16_t x, uint16_t y, uint8_t ch ) ;
extern void lcdDrawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdFlags flags, uint16_t colour = LcdForeground ) ;
extern void lcdDrawSizedText(coord_t x, coord_t y, const char * s, uint8_t len) ;
extern void lcdDrawText(coord_t x, coord_t y, const char * s, LcdFlags flags, uint16_t colour) ;
extern void lcdDrawText(coord_t x, coord_t y, const char * s, LcdFlags flags) ;
extern void lcdDrawText(coord_t x, coord_t y, const char * s) ;
extern void lcdDrawTextLeft(coord_t y, const char * s) ;
extern void lcdDrawTextAtIndex(coord_t x, coord_t y, const char * s,uint8_t idx, LcdFlags flags, uint16_t colour ) ;
extern void lcdDrawTextAtIndex(coord_t x, coord_t y, const char * s,uint8_t idx, LcdFlags flags) ;
extern void lcdDrawHex4(coord_t x, coord_t y, uint16_t val ) ;
extern void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags) ;
extern void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags, uint8_t len ) ;
extern void lcdDrawNumber(coord_t x, coord_t y, int32_t val ) ;
extern void putsTimeP( coord_t x, coord_t y, int16_t tme, LcdFlags att) ;
extern void lcd2Digits( coord_t x, coord_t y, int16_t value, LcdFlags attr ) ;
extern uint16_t getTextWidth(const char * s, uint8_t len, LcdFlags flags) ;
extern void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags, uint8_t len, uint16_t colour ) ;
extern void lcdDrawPropImage( uint16_t x, uint16_t y, uint32_t w, uint32_t h, uint32_t mode, uint16_t colour, uint8_t *bitmap ) ;
extern char *numberToText( int32_t val, LcdFlags flags, uint8_t len, char *str, uint32_t size ) ;
#endif


#ifdef PROP_TEXT

extern coord_t LcdLastRightPos ;
extern coord_t LcdNextPos ;
extern coord_t LcdLastLeftPos ;

extern void lcdDrawChar(coord_t x, coord_t y, const unsigned char c, LcdFlags flags) ;
extern void lcdDrawChar( coord_t x, coord_t y, uint8_t ch ) ;
extern void lcdDrawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdFlags flags) ;
extern void lcdDrawSizedText(coord_t x, coord_t y, const char * s, uint8_t len) ;
extern void lcdDrawText(coord_t x, coord_t y, const char * s, LcdFlags flags) ;
extern void lcdDrawText(coord_t x, coord_t y, const char * s) ;
extern void lcdDrawTextLeft(coord_t y, const char * s) ;
extern void lcdDrawTextAtIndex(coord_t x, coord_t y, const char * s,uint8_t idx, LcdFlags flags) ;
extern void lcdDrawHex4(coord_t x, coord_t y, uint16_t val ) ;
extern void lcdDrawHex2(coord_t x, coord_t y, uint8_t val ) ;
extern void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags) ;
extern void lcdDrawNumber(coord_t x, coord_t y, int32_t val, LcdFlags flags, uint8_t len) ;
extern void putsTimeP( coord_t x, coord_t y, int16_t tme, LcdFlags att) ;
extern void lcd2Digits( coord_t x, coord_t y, int16_t value, LcdFlags attr ) ;
extern char *numberToText( int32_t val, LcdFlags flags, uint8_t len, char *str, uint32_t size ) ;

#else	// PROP_TEXT

//#define lcdDrawTextLeft	lcd_puts_Pleft
	 
#endif	// PROP_TEXT





#endif

