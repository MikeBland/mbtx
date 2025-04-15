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

// Menu functions for 480x272 colour screens

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ersky9x.h"
#include "myeeprom.h"
#include "audio.h"
#include "sound.h"
#include "lcd.h"
#include "debug.h"
#include "menus.h"
#include "drivers.h"
#include "logicio.h"
#include "file.h"
#include "stringidx.h"
#include "templates.h"
#include "pulses.h"
#include "mixer.h"
#include "frsky.h"
//#include <ctype.h>
#ifndef SIMU
#include "CoOS.h"
#endif
#include "gvars.h"

#include "timers.h"
#include "analog.h"

#include "X12D/hal.h"

#include "sticks.lbm"

#ifdef USE_VARS
#include "vars.h"
#endif

//#define SCREEN_LINES		15

//#define PARAM_OFS   17*FW
#define NUM_MIX_SWITCHES	(9+NUM_SKYCSW)

uint32_t doPopup( const char *list, uint16_t mask, uint8_t width, uint8_t event ) ;
uint32_t checkForMenuEncoderBreak( uint8_t event ) ;
uint32_t checkForMenuEncoderLong( uint8_t event ) ;
//uint8_t checkIndexed( uint8_t y, const char *s, uint8_t value, uint8_t edit ) ;
void copyFileName( char *dest, char *source, uint32_t size ) ;
uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext ) ;
//uint8_t onoffMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition ) ;
void menuProcSelectVoiceFile(uint8_t event) ;
//void menu_lcd_onoff( uint8_t x,uint8_t y, uint8_t value, uint8_t mode ) ;
int8_t checkIncDec( int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags) ;
//void lcd_xlabel_decimal( uint8_t x, uint8_t y, uint16_t value, uint8_t attr, const char *s ) ;
void menuCellScaling(uint8_t event) ;
uint8_t checkOutOfOrder( uint8_t value, uint8_t *options, uint32_t count ) ;
uint8_t mapPots( uint8_t value ) ;
uint8_t unmapPots( uint8_t value ) ;
uint8_t putsTelemetryChannel(coord_t x, coord_t y, int8_t channel, int16_t val, LcdFlags att, uint8_t style, uint16_t colour = LcdForeground, uint16_t bgColour = LcdBackground ) ;
void lcdDrawIcon( uint16_t x, uint16_t y, const uint8_t * bitmap, uint8_t type ) ;
void putsTimexxl( uint16_t x, uint16_t y, int16_t tme, uint8_t att, uint16_t colour ) ;
void dispGvar( coord_t x, coord_t y, uint8_t gvar, LcdFlags attr ) ;
void putsChnOpRaw( coord_t x, coord_t y, uint8_t source, uint8_t switchSource, uint8_t output, LcdFlags attr ) ;
void drawSmallGVAR( uint16_t x, uint16_t y ) ;
int16_t gvarDiffValue( uint16_t x, uint16_t y, int16_t value, uint32_t attr, uint8_t event ) ;
void deleteVoice(VoiceAlarmData *pvad) ;
int8_t qRotary() ;
void menuProcGlobalVoiceAlarm(uint8_t event) ;
void moveVoice(uint8_t idx, uint8_t dir, uint8_t mode) ; //true=inc=down false=dec=up
void voicepopup( uint8_t event ) ;
void menuRangeBind(uint8_t event) ;
void menuSetFailsafe(uint8_t event) ;
void menuScanDisplay(uint8_t event) ;
void displayMultiProtocol( uint32_t index, coord_t y, LcdFlags attr ) ;
uint32_t displayMultiSubProtocol( uint32_t index, uint32_t subIndex, coord_t y, LcdFlags attr ) ;
void multiOption( uint32_t x, uint32_t y, int32_t option, uint32_t attr, uint32_t protocol ) ;
uint32_t checkProtocolOptions( uint8_t module ) ;
void multiOptionTouch( uint32_t x, uint32_t y, int32_t option, uint32_t attr, uint32_t protocol ) ;
void menuBindOptions(uint8_t event) ;
void displayModuleName( uint16_t x, uint16_t y, uint8_t index, uint8_t attr ) ;
void menuOneGvar(uint8_t event) ;
SKYMixData *mixAddress( uint32_t index ) ;
uint16_t extendedValueEdit( int16_t value, uint8_t extValue, LcdFlags attr, coord_t y, uint8_t event, coord_t x ) ;
uint8_t mapMixSource( uint8_t index, uint8_t switchSource ) ;
uint8_t unmapMixSource( uint8_t index, uint8_t *switchSource ) ;
uint8_t editSlowDelay( coord_t y, LcdFlags attr, uint8_t value) ;
void menuProcCurveOne(uint8_t event) ;
void menuCustomCheck(uint8_t event) ;
void menuProcSelectImageFile(uint8_t event) ;
extern PhaseData *getPhaseAddress( uint32_t phase ) ;
void drawCurve( uint8_t offset ) ;
void menuProcTrainer(uint8_t event) ;


extern uint8_t s_moveMode;
extern struct fileControl PlayFileControl ;
extern TCHAR PlaylistDirectory[] ;
extern uint16_t PlaylistIndex ;
extern char CurrentPlayName[] ;
extern uint8_t VoiceFileType ;
extern uint8_t Columns ;
extern uint8_t g_posHorz ;
extern const char HyphenString[] ;
extern uint8_t FileSelectResult ;
extern char SelectedVoiceFileName[] ;
extern uint32_t BgSizePlayed ;
extern uint32_t BgTotalSize ;
extern uint8_t ModelImageValid ;
extern uint16_t Image_width ;
extern uint16_t Image_height ;
extern uint8_t CurrentVolume ;
extern uint8_t LogsRunning ;
extern uint32_t IdlePercent ;
extern uint32_t MixerRate ;
extern uint32_t BasicExecTime ;
extern uint8_t s_currIdx ;
extern int8_t s_curItemIdx;
extern uint8_t MuteTimer ;
extern struct t_clipboard Clipboard ;
extern const uint8_t GvaString[] ;
extern uint8_t s_expoChan ;
extern uint8_t SingleExpoChan ;
extern uint8_t PrivateData[] ;
extern uint8_t EditingModule ;
extern const uint8_t SortedMulti[] ;
extern uint8_t s_curveChan ;
extern int8_t s_curItemIdx;
extern uint8_t AlphaEdited ;
extern const uint8_t UnitsString[] ;
extern const uint8_t DestString[] ;

extern struct t_multiSetting MultiSetting ;

extern uint8_t AccEntry ;
extern uint8_t AccState ;

#ifdef TOUCH
extern uint8_t LastTselection ;
extern uint8_t LastTaction ;
#endif

#ifdef  BASIC
#include "basic/basic.h"
#endif

#include "ff.h"
#include "maintenance.h"

#ifdef BLUETOOTH
#include "bluetooth.h"
#endif

const uint8_t Icon24log[] =
{
#if defined(PCBX10)
#include "icon24loginv.lbm"
#else
#include "icon24log.lbm"
#endif
} ;

#ifdef TOUCH
extern uint8_t TlExitIcon ;
uint16_t dimBackColour() ;
void drawItem( char *s, uint16_t y, uint16_t colour ) ;
void drawNumber( uint16_t x, uint16_t y, int32_t val, uint16_t mode) ; //, uint16_t colour ) ;
extern uint8_t TouchUpdated ;
#endif

extern uint8_t InverseBlink ;
extern uint8_t ImageDisplay ;
extern uint8_t scroll_disabled;
extern uint8_t StickScrollTimer ;

extern uint8_t EditType ;
extern uint8_t EditColumns ;
extern uint8_t s_currIdx;

static uint8_t s_currSubIdx;

uint8_t SuppressStatusLine ;

void putsValue( coord_t x, coord_t y, uint16_t index, int16_t val, LcdFlags att, uint8_t style, uint16_t colour = LcdForeground, uint16_t bgColour = LcdBackground )
{
	if ( index >= EXTRA_POTS_START-1 )
	{
		if ( index >= EXTRA_POTS_START-1+8 )
		{
			putsTelemetryChannel( x, y, index-CHOUT_BASE-NUM_SKYCHNOUT, val, att, style, colour, bgColour ) ;
			return ;
		}
	}
	else if(index>=CHOUT_BASE+NUM_SKYCHNOUT)
	{
  	if(index<CHOUT_BASE+NUM_SKYCHNOUT+NUM_TELEM_ITEMS)
		{
			putsTelemetryChannel( x, y, index-CHOUT_BASE-NUM_SKYCHNOUT, val, att, style, colour, bgColour ) ;
			return ;
		}
	}
	PUTS_NUM_N( (style & TELEM_VALUE_RIGHT) ? x+62 : x, y, val, att, 5 ) ; // , colour, bgColour ) ;
}

uint8_t hronoffMenuItem( coord_t x, uint8_t value, coord_t y, const prog_char *s, LcdFlags edit )
{
	PUTS_ATT_LEFT(y, s) ;
	menu_lcd_onoff( x, y, value, edit ) ;
	if(edit)
	{
		if ( ( EditColumns == 0 ) || ( s_editMode ) )
		{
			value = checkIncDec( value, 0, 1, EditType ) ;
		}
	}
	return value ;
}



void menuProcDiagAna(uint8_t event)
{
	register uint32_t i ;
	
	// Title
	PUTS_ATT( 0, 0, PSTR(STR_ANA),INVERS) ;
	
	static MState2 mstate2;
	StickScrollAllowed = 0 ;
	StickScrollTimer = 0 ;
	mstate2.check_columns(event, NUM_ANA_ITEMS-1) ;

	ImageDisplay = 0 ;

  int8_t  sub    = mstate2.m_posVert ;
  for(i=0; i<10; i++)
  {
    coord_t y=(i+2)*FHPY ;
		PUTS_AT_IDX( 9*FW-3, y, XPSTR("\002LHLVRVRHA5A6A7A8BT6P"), i, 0 ) ;
		uint8_t index = i ;
		if ( i < 4 )
		{
 			index = stickScramble[g_eeGeneral.stickMode*4+i] ;
 		}
    if(i<8)
		{
			PUTS_NUM( 20*FW, y, (int32_t)calibratedStick[index]*1000/1024, PREC1);
  		PUT_HEX4( 11*FW, y,anaIn(i));
		}
		else if(i==8)
		{
			putsVBat(19*FW,y,(sub==1 ? InverseBlink : 0)|PREC1);
    	PUT_HEX4( 11*FW, y,anaIn(12));
		}
		else
		{
  		PUT_HEX4( 11*FW, y,anaIn(10));
		}
  }

  if(sub==1)
  {
    scroll_disabled = 1;
		if ( s_editMode )
		{
	    CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatCalib, -127, 127);
		}
  }
}

#ifndef TOUCH
uint32_t checkPageMove( uint8_t event, uint8_t *pos, uint32_t max )
{
	uint8_t lpos = *pos ;
	if ( event == EVT_KEY_FIRST(KEY_LEFT) )	// PAGE
	{
		if ( lpos == max)
		{
			lpos = 0 ;
		}
		else
		{
			lpos += TLINES-1 ;
			if ( lpos > max )
			{
				lpos = max ;
			}
		}
		killEvents( EVT_KEY_FIRST(KEY_LEFT) ) ;
		event = 0 ;
		Tevent = 0 ;
	}	 
	*pos = lpos ;
	return event ;
}
#endif




#define HIRES_TYPE_VALUE		0
#define HIRES_TYPE_IMAGE		1
#define HIRES_TYPE_BARS			2
#define HIRES_TYPE_TIMER		3
#define HIRES_TYPE_STICKS		4

#define NUM_HIRES_DESIGN		8

//const uint8_t Layout0x[] = { 1, 0, 0, 239, 119 } ;
//const uint8_t Layout0[] = { 1, 1,1,238,118 } ;
//const uint8_t Layout1[] = { 2, 1,1,118,118, 120,1,118,118 } ;
//const uint8_t Layout2[] = { 3, 1,1,118,118, 120,1,118,58, 120,61,118,58 } ;
//const uint8_t Layout3[] = { 3, 1,1,118,58, 1, 61, 118, 58, 120,1,118,118 } ;
//const uint8_t Layout4[] = { 4, 1,1,118,58, 120,1,118,58, 1, 61, 118, 58, 120, 61, 118, 58 } ;
//const uint8_t Layout5[] = { 6, 1,1,118,38, 120,1,118,38, 1, 40, 118, 38, 120, 40, 118, 38, 1, 80, 118, 38, 120, 80, 118, 38 } ;

//const uint8_t Layout0[] = { 1, } ; // 0,0,240,120 } ;
//const uint8_t Layout1[] = { 2, } ; // 0,0,120,120, 120,0,120,120 } ;
//const uint8_t Layout2[] = { 3, } ; // 0,0,120,120, 120,0,120,60, 120,60,120,60 } ;
//const uint8_t Layout3[] = { 3, } ; // 0,0,120,60, 0, 60, 120, 60, 120,0,120,120 } ;
//const uint8_t Layout4[] = { 4, } ; // 0,0,120,60, 0, 60, 120, 60, 120,0,120,60, 120, 60, 120, 60 } ;
//const uint8_t Layout5[] = { 6, } ; // 0,0,120,40, 0, 40, 120, 40, 0, 80, 120, 40, 120,0,120,40, 120, 40, 120, 40, 120, 80, 120, 40 } ;
//const uint8_t Layout6[] = { 5, } ; // 0,0,120,40, 0, 40, 120, 40, 0, 80, 120, 40, 120,0,120,40, 120, 40, 120, 40, 120, 80, 120, 40 } ;

// Columns, # Col1, # Col2, #Col3
//const uint8_t TLayout[][4] = { { 1, 1, 0, 0 }, { 2, 1, 1, 0 }, { 2, 1, 2, 0 },{ 2, 2, 1, 0 },{ 2, 2, 2, 0 },{ 2, 3, 3, 0 },{2,3,2,0} } ;

uint8_t Rlayout[40] ;

//const uint8_t *Playout[] = { Layout0, Layout1, Layout2, Layout3, Layout4, Layout5, Layout6 } ;

const uint8_t LayoutRcount[] = {1,2,3,3,4,6,5,5} ;

uint8_t *pickLayout( uint32_t value, uint32_t options )
{
	if ( value >= NUM_HIRES_DESIGN )
	{
		value = 0 ;
	}
//	return (uint8_t *) Playout[value] ;

	uint32_t w = 240 ;
	uint32_t h = 120 ;
	uint32_t x = 0 ;
//	uint32_t count = *((uint8_t *) Playout[value]) ;	// # boxes
	uint32_t count = LayoutRcount[value] ;
	uint8_t *p ;

	if ( options & HIRES_OPT_TRIMS )
	{
		w = 216 ;
		h = 108 ;
		x = 12 ;
	}
//	else
//	{
//		return (uint8_t *) Playout[value] ;
//	}

	p = Rlayout ;
	*p++ = count ;
	*p++ = x ;
	*p++ = 0 ;
	switch ( value )
	{
		case 0 :
			*p++ = w ;
		break ;
		case 1 :
			*p++ = w/2 ;
			*p++ = h ;
			*p++ = x+w/2 ;
			*p++ = 0 ;
			*p++ = w/2 ;
		break ;
		case 2 :
			w /= 2 ;
			*p++ = w ;
			*p++ = h ;
			h /= 2 ;
			x += w ;
			*p++ = x ;
			*p++ = 0 ;
			*p++ = w ;
			*p++ = h ;
			*p++ = x ;
			*p++ = h ;
			*p++ = w ;
		break ;
		case 3 :
			w /= 2 ;
			*p++ = w ;
			*p++ = h/2 ;
			*p++ = x ;
			*p++ = h/2 ;
			*p++ = w ;
			*p++ = h/2 ;
			*p++ = x+w ;
			*p++ = 0 ;
			*p++ = w ;
		break ;
		case 4 :
			w /= 2 ;
			h /= 2 ;
			*p++ = w ;
			*p++ = h ;
			*p++ = x ;
			*p++ = h ;
			*p++ = w ;
			*p++ = h ;
			x += w ;			
			*p++ = x ;
			*p++ = 0 ;
			*p++ = w ;
			*p++ = h ;
			*p++ = x ;
			*p++ = h ;
			*p++ = w ;
		break ;
		case 5 :
		case 6 :
		case 7 :
			w /= 2 ;
			h /= 3 ;
			*p++ = w ;
			*p++ = h ;
			*p++ = x ;
			*p++ = h ;
			*p++ = w ;
			*p++ = h ;
			*p++ = x ;
			*p++ = h+h ;
			*p++ = w ;
			*p++ = h ;
			
			x += w ;			 
			*p++ = x ;
			*p++ = 0 ;
			*p++ = w ;
			
			if ( value == 7 )
			{
				*p++ = h ;
				*p++ = x ;
				*p++ = h ;
				*p++ = w ;
				h += h ;
			}
			else
			{
				if ( value == 5 )
				{
					*p++ = h ;
					*p++ = x ;
					*p++ = h ;
					*p++ = w ;
					*p++ = h ;
				}
				else
				{			
					*p++ = h+h ;
				}
				*p++ = x ;
				*p++ = h+h ;
				*p++ = w ;
			}
		break ;
	}
	*p++ = h ;

	return Rlayout ;
}

#define BARS_WIDTH		100

void hiresBars( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour, uint16_t background, uint32_t start )
{
	int16_t value ;
	uint16_t width ;
	uint16_t xpos = x ;
	uint16_t ypos = y ;
	uint32_t i ;
	uint32_t j ;
	uint32_t vert ;
	uint16_t lineColour ;
	uint32_t luminance ;
	luminance = background >> 11 ;	// Red
	luminance += (background>>6) & 0x1F ; //	Green
	luminance += background & 0x1F ; //	Blue

	lineColour = LCD_BLACK ;
	if ( luminance < 50 )
	{
		lineColour = LCD_WHITE ;
		luminance = colour >> 11 ;	// Red
		luminance += (colour>>6) & 0x1F ; //	Green
		luminance += colour & 0x1F ; //	Blue
		if ( luminance > 60 )
		{
			lineColour = LCD_GREY ;
		}
	}

	vert = h/21 ;

	ypos += (h - vert*21) / 2 ;

	y = ypos ;
	if ( w > BARS_WIDTH*4+4 )
	{
		xpos += (w-(BARS_WIDTH*4+4)) / 2 ;
		j = 2 ;
	}
	else if ( w > BARS_WIDTH*2 )
	{
		xpos += (w-BARS_WIDTH*2) / 2 ;
		j = 1 ;
	}
	else
	{
		// Could adjust the bar width
		return ;	// Doesn't fit !
	}
	i = 0 ;
	if ( start > 30 )
	{
		start = 0 ;
	}
	while ( j )
	{
		y = ypos ;
		for (; i < vert ; i += 1 )
		{
			lcdHiresRect( xpos+1, y, BARS_WIDTH * 2 - 1, 22, lineColour ) ;
		
			value = ex_chans[start] ;
			width = value < 0 ? -value : value ;
			width = width * BARS_WIDTH / 1024 ;
			if ( width >= BARS_WIDTH )
			{
				width = BARS_WIDTH - 1 ;
			}
			x = xpos + BARS_WIDTH ;
			if ( width == 0 )
			{
				width = 1 ;
			}
			else if ( value < 0 )
			{
				x -= width-1 ;
			}
			lcdDrawSolidFilledRectDMA( x, y+1, width, 20, colour ) ; // LCD_RGB565(0,20,20) ) ;	// Use colour?
			lcdDrawSizedText( xpos + 5, y+2, (char *)"CH", 2, 0, lineColour ) ;

			if ( start > 8 )
			{
				lcdDrawChar( xpos + 5 + 24, y+2, (start+1)/10+'0', 0, lineColour ) ;
				lcdDrawChar( xpos + 5 + 24+12, y+2, (start+1)%10+'0', 0, lineColour ) ;
			}
			else
			{
				lcdDrawChar( xpos + 5 + 24, y+2, start+'1', 0, lineColour ) ;
			}

			lcdDrawChar( xpos + BARS_WIDTH * 2 - 20, y+2, '%', 0, lineColour ) ;
			uint32_t negative = 0 ;
			uint32_t chr ;
			if ( value < 0 )
			{
				value = -value ;
				negative = 1 ;
			}
			value = value * 100 / RESX ;
			x = xpos + BARS_WIDTH * 2 - 20 - 12 ;
			chr = value % 10 + '0' ;
			lcdDrawChar( x, y+2, chr, 0, lineColour ) ;
			x -= 12 ;
			value /= 10 ;
			chr = value % 10 + '0' ;
			if ( value )
			{
				lcdDrawChar( x, y+2, chr, 0, lineColour ) ;
				x -= 12 ;
			}
			value /= 10 ;
			chr = value % 10 + '0' ;
			if ( value )
			{
				lcdDrawChar( x, y+2, chr, 0, lineColour ) ;
				x -= 12 ;
			}
			if ( negative )
			{
				lcdDrawChar( x, y+2, '-', 0, lineColour ) ;
			}
			y += 21 ;
			if ( ++start > 31 )
			{
				break ;
			}
		}
		j -= 1 ;
		xpos += BARS_WIDTH*2 ;
		vert += vert ;
	}
}

uint16_t HiresTime ;
uint16_t HiresSavedTime ;
uint32_t HiresCountTime ;

void displayGPSdata( uint16_t x, uint16_t y, uint16_t whole, uint16_t frac, uint16_t attr, uint8_t direction ) ;

void displayHiresScreen( uint32_t sindex )
{
	uint8_t *l ;
	uint32_t i ;
	uint32_t j ;
	uint16_t x, y, w, h ;
	uint16_t colour, bgcolour ;
	struct t_hiResDisplay *display ;

	if ( sindex > 1 )
	{
		sindex = 0 ;
	}
	display = &g_model.hiresDisplay[sindex] ;

	l = pickLayout( display->layout, display->options ) ;
//	if ( display->layout == 0 )
//	{
//		l = (uint8_t *)Layout0x ;
//	}
	pushPlotType( PLOT_BLACK ) ;
	j = *l++ ;
	for ( i = 0 ; i < j ; i += 1 )
	{
		x = *l++ ;
		y = *l++ ;
		w = *l++ ;
		h = *l++ ;
//		if ( g_model.hiresDisplay.layout )
//		{
//			lcd_rect( x, y, w, h ) ;
//		}
	
		bgcolour = display->boxes[i].bgColour ;
		colour = display->boxes[i].colour ;
			 
		if ( bgcolour )
		{
//			if ( g_model.hiresDisplay.layout )
//			{
//				lcdDrawSolidFilledRectDMA( x*2+2, y*2+2, w*2-4, h*2-4, bgcolour ) ;
				lcdDrawSolidFilledRectDMA( x*2, y*2, w*2, h*2, bgcolour ) ;
//			}
//			else
//			{
//				lcdDrawSolidFilledRectDMA( x*2, y*2, w*2, h*2, bgcolour ) ;
//			}
		}
		else
		{
			bgcolour = LcdBackground ;
		}
		if ( display->layout == 0 )
		{
			if ( display->boxes[i].type == HIRES_TYPE_STICKS )
			{
				doMainScreenGrphics( colour ) ;
				return ;
			}
		}

		if ( display->boxes[i].type == HIRES_TYPE_IMAGE )	// Image
		{
			if ( ModelImageValid )
			{
				x *= 2 ;
				y *= 2 ;
				w *= 2 ;
				h *= 2 ;
				if ( Image_width < w )
				{
					x += ( w - Image_width ) / 2 ;
				}
				if ( Image_height < h )
				{
					y += ( h - Image_height ) / 2 ;
				}
				lcd_picture( x, y, h ) ;
//				if ( Image_width < w-2 )
//				{
//					x += ( w - 2 - Image_width ) / 2 ;
//				}
//				if ( Image_height < h-4 )
//				{
//					y += ( h - 2 - Image_height ) / 2 ;
//				}
//				lcd_picture( x, y+2, h-4 ) ;
			}
		}
		else if ( display->boxes[i].type == HIRES_TYPE_VALUE )
		{
			if ( display->boxes[i].item )
			{
				uint8_t style = TELEM_UNIT | TELEM_HIRES ;
				uint8_t attr = 0 ;
				uint32_t index = display->boxes[i].item - 1 ;
				if ( h > 40 )
				{
					attr = DBLSIZE|CONDENSED ;
				}
				putsChnRaw( (x+1)*2, (y+1)*2, index+1, attr, colour, bgcolour ) ;
				y += (h-12)/2 ;
				x += (w-32)/2 + 16 ;
				if ( index == 138 )
				{
					if ( w < 116 )
					{
						attr &= ~(DBLSIZE|CONDENSED) ;
					}
					if ( telemItemValid( 86+8 ) == 0 )
					{
						attr |= BLINK ;
					}
					uint16_t oldFcolour = LcdForeground ;
					LcdForeground = colour ;
					displayGPSdata( x-9*FW, y, TelemetryData[ FR_GPS_LAT], TelemetryData[FR_GPS_LATd], LEADING0 | attr, TelemetryData[FR_LAT_N_S] ) ;
					LcdForeground = oldFcolour ;
				}
				else if ( index == 139 )
				{
					if ( w < 116 )
					{
						attr &= ~(DBLSIZE|CONDENSED) ;
					}
					if ( telemItemValid( 87+8 ) == 0 )
					{
						attr |= BLINK ;
					}
					uint16_t oldFcolour = LcdForeground ;
					LcdForeground = colour ;
					displayGPSdata( x-9*FW, y, TelemetryData[ FR_GPS_LONG], TelemetryData[FR_GPS_LONGd], LEADING0 | attr, TelemetryData[FR_LONG_E_W] ) ;
					LcdForeground = oldFcolour ;
				}
				else
				{
					putsValue( x+1, y+1, index, getValue(index), DBLSIZE|CONDENSED, style, colour, bgcolour ) ;




				}
			}
		}
		else if ( display->boxes[i].type == HIRES_TYPE_BARS )
		{
//			hiresBars( x*2+4, y*2+4, w*2-4, h*2-8, colour, bgcolour ) ;
			hiresBars( x*2, y*2, w*2, h*2, colour, bgcolour, display->boxes[i].item ) ;
		}
		else if ( display->boxes[i].type == HIRES_TYPE_TIMER )
		{
			uint8_t style = TELEM_UNIT | TELEM_HIRES ;
			uint8_t attr = 0 ;
			uint32_t index = 48 + display->boxes[i].item ;
			putsChnRaw( (x+1)*2, (y+1)*2, index+1, attr, colour, bgcolour ) ;
			if ( ( h > 35 ) && ( w > 100 ) )
			{
				x *= 2 ;
				y *= 2 ;
				w *= 2 ;
				h *= 2 ;
				x += (w - 160 ) / 2 ;
				y += (h-68)/2 + 4 ;
				putsTimexxl( x, y, getValue(index), 0, colour ) ;
			}
			else
			{
				if ( h > 40 )
				{
					attr = DBLSIZE|CONDENSED ;
				}
				y += (h-12)/2 ;
				x += (w-32)/2 + 16 ;
				putsValue( x+1, y+1, index, getValue(index), DBLSIZE|CONDENSED, style, colour, bgcolour ) ;
			}
		}

		if ( display->options & HIRES_OPT_BORDERS )
		{
			l -= 4 ;
			x = *l++ * 2 ;
			y = *l++ * 2 ;
			w = *l++ * 2 ;
			h = *l++ * 2 ;
		
			if ( y )
			{
				y -= 1 ;
				h += 1 ;
			}
			if ( x > 20 )
			{
				x -= 1 ;
				w += 1 ;
			}
			lcdHiresRect( x, y, w, h, LCD_BLACK ) ;
		}

 #ifndef MIXER_TASK
extern uint16_t MainStart ;
	 	if ( ((uint16_t)getTmr2MHz() - MainStart ) > 4000 )
		{
			checkRunMixer() ;
			MainStart += 4000 ;
		}
#endif
	}
	if ( display->options & HIRES_OPT_TRIMS )
	{
void displayTrims() ;
		displayTrims() ;
	}
	popPlotType() ;
}



void menuHiresTest(uint8_t event)
{
  uint16_t t0 = getTmr2MHz();

	static MState2 mstate2 ;
	event = mstate2.check_columns( event, 0 ) ;

	displayHiresScreen( s_currSubIdx ) ;	// First screen

	HiresTime = getTmr2MHz() - t0 ;
	if ( ++HiresCountTime > 49 )
	{
		HiresSavedTime = HiresTime ;
		HiresCountTime = 0 ;
	}
}

void menuHiresItem(uint8_t event)
{
	uint32_t red ;
	uint32_t green ;
	uint32_t blue ;
	const char *options ;
	TITLE(XPSTR("Set Item")) ;
	static MState2 mstate2 ;
	event = mstate2.check_columns( event, 7 ) ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t subN = 0 ;
	coord_t y = 2*FHPY ;
	uint32_t index = s_currSubIdx ;
	struct t_hiResDisplay *display ;

	display = &g_model.hiresDisplay[index] ;

	if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		if ( TextResult )
		{
			display->boxes[s_currIdx].item = unmapPots( TextIndex ) ;
	    eeDirty(EE_MODEL) ;
		}
	}
	 
	if ( s_currIdx > 5 )
	{
		s_currIdx = 5 ;
	}

	options = ( display->layout == 0 ) ? FWx9"\004""\006Value Image Bars  Timer Sticks" : FWx9"\003""\005ValueImageBars Timer " ;

	PUTS_ATT_LEFT( y, XPSTR("Type") ) ;
	display->boxes[s_currIdx].type = checkIndexed( y, options, display->boxes[s_currIdx].type, (sub==subN) ) ;

	subN += 1 ;
	y += FHPY ;
	uint8_t attr = (sub==subN) ? InverseBlink : 0 ;
	
	if ( display->boxes[s_currIdx].type == HIRES_TYPE_BARS )
	{
		PUTS_ATT_LEFT( y, XPSTR("Start") ) ;
 		PUTS_NUM(PARAM_OFS+2*FW, y, display->boxes[s_currIdx].item+1,attr );
  	if (sub==subN)
		{
			CHECK_INCDEC_H_MODELVAR_0( display->boxes[s_currIdx].item, 30 ) ;
		}
	}
	else
	{
		PUTS_ATT_LEFT( y, XPSTR("Item") ) ;
		if ( display->boxes[s_currIdx].type == HIRES_TYPE_TIMER )
		{
			display->boxes[s_currIdx].item = checkIndexed( y, FWx9"\001""\006Timer1Timer2", display->boxes[s_currIdx].item, (sub==subN) ) ;
		}
		else
		{
			if ( display->boxes[s_currIdx].item )
			{
				putsChnRaw( (FW*9)*2, (y)*2, display->boxes[s_currIdx].item, attr | TSSI_TEXT ) ;
			}
			else
			{
  			PUTS_ATT(  FW*9, y, HyphenString, attr ) ;
			}
  		if (sub==subN)
			{ 
				uint8_t val = display->boxes[s_currIdx].item ;
				val = mapPots( val ) ;
#ifdef TOUCH
				if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) || handleSelectIcon() )
#else
				if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#endif
				{
					TextIndex = val ;
  				TextType = TEXT_TYPE_SW_SOURCE ;
  				killEvents(event) ;
					pushMenu(menuTextHelp) ;
				}
//				val = checkIncDec16( val,0,NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots-1+1,EE_MODEL);
	//			CHECK_INCDEC_H_MODELVAR_0( val, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots ) ;
				display->boxes[s_currIdx].item = unmapPots( val ) ;
			}
		}
	}
	subN += 1 ;
	y += FHPY ;
	
	red = display->boxes[s_currIdx].colour >> 11 ;
	green = ( display->boxes[s_currIdx].colour >> 6 ) & 0x1F ;
	blue = display->boxes[s_currIdx].colour & 0x1F ;

	uint8_t blink = InverseBlink ;

	PUTS_ATT_LEFT( y, XPSTR("Text Red")) ;
 	PUTS_NUM(PARAM_OFS+2*FW, y, red,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( red, 31 ) ;
	y += FHPY ;
	subN += 1 ;
	 
	PUTS_ATT_LEFT( y, XPSTR("\005Green")) ;
 	PUTS_NUM(PARAM_OFS+2*FW, y, green,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( green, 31 ) ;
	y += FHPY ;
	subN += 1 ;
	
	PUTS_ATT_LEFT( y, XPSTR("\005Blue")) ;
 	PUTS_NUM(PARAM_OFS+2*FW, y, blue,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( blue, 31 ) ;
	
	display->boxes[s_currIdx].colour = ( red << 11 ) | ( green << 6 ) | ( blue ) ;

	y += FHPY ;
	subN += 1 ;

	red = display->boxes[s_currIdx].bgColour >> 11 ;
	green = ( display->boxes[s_currIdx].bgColour >> 6 ) & 0x1F ;
	blue = display->boxes[s_currIdx].bgColour & 0x1F ;

	PUTS_ATT_LEFT( y, XPSTR("Back Red")) ;
 	PUTS_NUM(PARAM_OFS+2*FW, y, red,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( red, 31 ) ;
	y += FHPY ;
	subN += 1 ;
	 
	PUTS_ATT_LEFT( y, XPSTR("\005Green")) ;
 	PUTS_NUM(PARAM_OFS+2*FW, y, green,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( green, 31 ) ;
	y += FHPY ;
	subN += 1 ;
	
	PUTS_ATT_LEFT( y, XPSTR("\005Blue")) ;
 	PUTS_NUM(PARAM_OFS+2*FW, y, blue,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( blue, 31 ) ;
	
	display->boxes[s_currIdx].bgColour = ( red << 11 ) | ( green << 6 ) | ( blue ) ;
	
	lcdDrawSolidFilledRectDMA( 350, 2*(y-3*FH), 60, 20, display->boxes[s_currIdx].bgColour ) ;
	PUTS_ATT_N_COLOUR( 177, (y-3*FH+1), "Text", 4, 0, display->boxes[s_currIdx].colour ) ; //, display->boxes[s_currIdx].bgColour ) ;
}


void menuHiresContent(uint8_t event)
{
	uint32_t i ;
	uint32_t j ;
	uint16_t x, y, w, h ;
	uint8_t *l ;
	uint32_t index = s_currSubIdx ;
	struct t_hiResDisplay *display ;

	display = &g_model.hiresDisplay[index] ;
	
	TITLE(XPSTR("Select Area")) ;
	static MState2 mstate2 ;
	
	l = pickLayout( display->layout, display->options ) ;
	
	j = *l++ ;
	event = mstate2.check_columns( event, j - 1 ) ;
	uint8_t sub = mstate2.m_posVert ;
	
	PUTS_ATT_LEFT( 2*FHPY, XPSTR("Layout") ) ;

	pushPlotType( PLOT_BLACK ) ;
	lcd_rect( 6, 3*FHPY+6, 128, 64+4 ) ;
	for ( i = 0 ; i < j ; i += 1 )
	{
		x = *l++ / 2 + 10 ;
		y = *l++ / 2 + 4*FHPY ;
		w = *l++ / 2 ;
		h = *l++ / 2 ;
		if ( sub == i )
		{
			lcd_rectColour( x, y, w, h, LCD_RED ) ;
		}
		else
		{
			lcd_rect( x, y, w, h ) ;
		}
	}
	popPlotType() ;
	if (checkForMenuEncoderBreak( event ) )
	{
		s_currIdx = sub ;
    pushMenu( menuHiresItem ) ;
	}

}


#define HRES_OFF_0			(0)

void menuHires(uint8_t event)
{
	TITLE(PSTR(STR_CUSTOM_DISP)) ;
	static MState2 mstate2 ;
	struct t_hiResDisplay *display ;

	DisplayOffset = HRES_OFF_0 ;

	display = &g_model.hiresDisplay[s_currSubIdx] ;
	
	uint32_t rows = 5 ;
	event = mstate2.check_columns( event, rows ) ;
	
	uint8_t sub = mstate2.m_posVert ;

	PUTS_ATT_LEFT( 2*FHPY, XPSTR("Screen") ) ;
	PUTS_NUM( PARAM_OFS+2*FW+HRES_OFF_0, 2*FH, s_currSubIdx, (sub==0) ? InverseBlink:0 ) ;

	PUTS_ATT_LEFT( 3*FHPY, XPSTR("Layout") ) ;
	pushPlotType( PLOT_BLACK ) ;
	switch ( display->layout )
	{
		case 0 :
			lcd_rect( HRES_OFF_0+50, 3*FH, 32, 16 ) ;
		break ;
		case 1 :
			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 16 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 16 ) ;
		break ;
		case 2 :
			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 16 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 8 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH+8, 16, 8 ) ;
		break ;
		case 3 :
			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 8 ) ;
			lcd_rect( HRES_OFF_0+50, 3*FH+8, 16, 8 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 16 ) ;
		break ;
		case 4 :
			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 8 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 8 ) ;
			lcd_rect( HRES_OFF_0+50, 3*FH+8, 16, 8 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH+8, 16, 8 ) ;
		break ;
		case 5 :
			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50, 3*FH+5, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50, 3*FH+10, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH+5, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH+10, 16, 5 ) ;
		break ;
		case 6 :
			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50, 3*FH+5, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50, 3*FH+10, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 10 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH+10, 16, 5 ) ;
		break ;
		case 7 :
			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50, 3*FH+5, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50, 3*FH+10, 16, 5 ) ;
			
			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 5 ) ;
			lcd_rect( HRES_OFF_0+50+16, 3*FH+5, 16, 10 ) ;
		break ;
	}

//	{
//		uint8_t *l ;
//		uint32_t j ;
//		uint32_t i ;
//		uint16_t x, y, w, h ;
//		l = pickLayout( HiResDisplay.layout ) ;
//		j = *l++ ;
//		for ( i = 0 ; i < j ; i += 1 )
//		{
//#define HR_DIV 6
//			x = (*l++ + HR_DIV/2) / HR_DIV ;
//			y = (*l++ + HR_DIV/2) / HR_DIV ;
//			w = (*l++ + HR_DIV/2) / HR_DIV ;
//			h = (*l++ + HR_DIV/2) / HR_DIV ;
//			lcd_rect( x+100, y+2*FH+1, w, h ) ;
//		}
//	}


	popPlotType() ;
	PUTS_ATT_LEFT( 6*FHPY, XPSTR("Edit Contents") ) ;
	uint32_t x ;
	x = display->options & HIRES_OPT_TRIMS ;
	x = onoffMenuItem( x, 7*FHPY, XPSTR("Include Trims"), ( sub == 3 ) ) ;
	display->options = (display->options & ~HIRES_OPT_TRIMS ) | x ;
	x = display->options & HIRES_OPT_BORDERS ;
	x = onoffMenuItem( x, 8*FHPY, XPSTR("Display Borders"), ( sub == 4 ) ) ;
	display->options = (display->options & ~HIRES_OPT_BORDERS ) | ( x ? HIRES_OPT_BORDERS : 0 ) ;
	PUTS_ATT_LEFT( 9*FHPY, XPSTR("Test") ) ;
	
	if ( sub == 0 )
	{
		CHECK_INCDEC_H_MODELVAR_0( s_currSubIdx, 1 ) ;
	}
	else if ( sub == 1 )
	{
		lcd_char_inverse( HRES_OFF_0+48, 3*FH-1, 36, s_editMode ? BLINK : 0, 18 ) ;
//		lcd_char_inverse( 98, 2*FH-1, 288/HR_DIV, s_editMode ? BLINK : 0, 132/HR_DIV ) ;
		CHECK_INCDEC_H_MODELVAR_0( display->layout, NUM_HIRES_DESIGN-1 ) ;
	}
	else if ( sub == 2 )
	{
		lcd_char_inverse( HRES_OFF_0+0, 6*FHPY, 13*6, 0, 9 ) ;
		if (checkForMenuEncoderBreak( event ) )
		{
			// To Do			
      pushMenu( menuHiresContent ) ;
		}
	}	
	else if ( sub == 5 )
	{
		lcd_char_inverse( HRES_OFF_0+0, 9*FHPY, 4*6, 0, 9 ) ;
		if (checkForMenuEncoderBreak( event ) )
		{
      pushMenu( menuHiresTest ) ;
		}
	}	
}

void menuColour( uint8_t event, uint8_t mode )
{
	uint32_t red ;
	uint32_t green ;
	uint32_t blue ;
	uint16_t colour ;

	if ( mode )
	{
		TITLE(XPSTR("Text Colour"));
		colour = g_eeGeneral.textColour ;
	}
	else
	{
		TITLE(XPSTR("Background Colour"));
		colour = g_eeGeneral.backgroundColour ;
	}
	
	uint32_t rows = 3 ;

#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2 ;
	mstate2.check_columns(event, rows-1) ;

  int8_t  sub    = mstate2.m_posVert ;
		
	red = colour >> 11 ;
	green = ( colour >> 6 ) & 0x1F ;
	blue = colour & 0x1F ;

//	uint8_t blink = InverseBlink ;
	uint8_t subN = 0 ;
//	uint16_t hcolour = dimBackColour() ;
	LcdFlags attr ;

#ifdef TOUCH
	if ( TouchUpdated )
	{
		if ( TouchControl.event == TEVT_DOWN )
		{
				
		}
		else if ( TouchControl.event == TEVT_UP )
		{
			if ( ( TouchControl.x <= TRIGHT*TSCALE ) && (TouchControl.x >= TMID*TSCALE) && !s_editMode )
			{
				uint32_t vert = TouchControl.y ;
				vert -= TTOP*TSCALE ;
				vert /= TFH*TSCALE ;
				if ( vert >= TLINES - 1 )
				{
					vert = TLINES - 2 ;
				}
//				vert += t_pgOfs ;
				if ( vert < rows )
				{
//						m_posVert = vert ;
					TouchControl.itemSelected = vert+1 ;
					TouchUpdated = 0 ;
					sub = mstate2.m_posVert = vert ;
				}
			}
		}
	}
#endif
	 
	coord_t y = TTOP ;
	lcd_hline( 0, TTOP, TRIGHT ) ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Red"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, red, attr) ; //, attr ? ~hcolour : hcolour ) ;
  if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( red, 31 ) ;
	}
	y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Green"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, green, attr) ; //, attr ? ~hcolour : hcolour ) ;
  if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( green, 31 ) ;
	}
	y += TFH ;
	subN += 1 ;
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Blue"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, blue, attr) ; //, attr ? ~hcolour : hcolour ) ;
  if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( blue, 31 ) ;
	}
//	y += TFH ;
//	subN += 1 ;
//#else	
//	static MState2 mstate2 ;
//	mstate2.check_columns(event, 2 ) ;

//  int8_t  sub    = mstate2.m_posVert ;
//	red = colour >> 11 ;
//	green = ( colour >> 6 ) & 0x1F ;
//	blue = colour & 0x1F ;

//	uint8_t y = 2*FH;
//	uint8_t blink = InverseBlink ;
//	uint8_t subN = 0 ;

//	PUTS_ATT_LEFT( y, XPSTR("Red")) ;
// 	PUTS_NUM(PARAM_OFS+2*FW, y, red,(sub==subN ? blink : 0) );
//  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( red, 31 ) ;
//	y += FH ;
//	subN += 1 ;
	 
//	PUTS_ATT_LEFT( y, XPSTR("Green")) ;
// 	PUTS_NUM(PARAM_OFS+2*FW, y, green,(sub==subN ? blink : 0) );
//  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( green, 31 ) ;
//	y += FH ;
//	subN += 1 ;
	
//	PUTS_ATT_LEFT( y, XPSTR("Blue")) ;
// 	PUTS_NUM(PARAM_OFS+2*FW, y, blue,(sub==subN ? blink : 0) );
//  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( blue, 31 ) ;
////	y += FH ;
////	subN += 1 ;
//#endif // TOUCH
	if ( mode == 0 )
	{
		if ( (red < 6) && (green < 6) && (blue < 6) )
		{
			red = 6 ;
			green = 6 ;
			blue = 6 ;
		}
	}

	colour = ( red << 11 ) | ( green << 6 ) | ( blue ) ;
	
	if ( mode )
	{
		g_eeGeneral.textColour = LcdForeground = colour ;
	}
	else
	{
		g_eeGeneral.backgroundColour = LcdBackground = colour ;
//#ifdef TOUCH
		dimBackColour() ;
//#endif
	}
}

void checkTheme( themeData *t )
{
	if ( t->backColour == 0 )
	{
		if ( t->textColour == 0 )
		{
			if ( t->brightness <= 1 )
			{
				t->backColour = g_eeGeneral.backgroundColour ;
				t->textColour = g_eeGeneral.textColour ;
				t->brightness = g_eeGeneral.bright ;
			}
		}
	}
}

void menuBackground( uint8_t event )
{
	menuColour( event, 0 ) ;	
}

void menuForeground( uint8_t event )
{
	menuColour( event, 1 ) ;	
}

#define STATUS_VERTICAL		242

const uint8_t SpeakerHiRes[] = {
8,16,0,
0xC0,0xC0,0xC0,0xE0,0xF0,0xF8,0xFC,0xFC,
0x07,0x07,0x07,0x0F,0x1F,0x3F,0x7F,0x7F
} ;

void displayStatusLine( uint32_t scriptPercent )
{
	if ( SuppressStatusLine == 0 )
	{
//	lcdDrawSolidFilledRectDMA( 0, STATUS_VERTICAL-2, 480, 18, LCD_STATUS_GREY ) ;
		lcdDrawSolidFilledRectDMA( 0, STATUS_VERTICAL, 480, 30, LCD_STATUS_GREY ) ;
		putsTimeP( 448, STATUS_VERTICAL+3, Time.hour*60+Time.minute, 0 ) ;

		lcd_HiResimg( 180*2, (STATUS_VERTICAL)-1+7, SpeakerHiRes, 0, 0, LCD_BLACK, LCD_STATUS_GREY ) ;
	//	lcd_img( 180, (STATUS_VERTICAL/2), speaker, 0, 0, LCD_BLACK, LCD_STATUS_GREY ) ;
		pushPlotType( PLOT_BLACK ) ;
		lcd_hbar( 185, (STATUS_VERTICAL/2)+1+3, 23, 6, (CurrentVolume*100+16)/23 ) ;
		popPlotType() ;

		if ( LogsRunning & 1 )
		{
			if ( BLINK_ON_PHASE )
			{
				lcdDrawIcon( 330, STATUS_VERTICAL+3, Icon24log, 0x40 ) ;
	//			lcd_img( 165, (STATUS_VERTICAL/2), IconX12Logging, 0, 0, LCD_BLACK, LCD_STATUS_GREY ) ;
			}
		}
  	
		lcdDrawNumber( 4, STATUS_VERTICAL+3, IdlePercent, PREC2|LEFT, 4 ) ;
		
//		lcd_outdezNAtt( 4*FW, (STATUS_VERTICAL/2)+3, IdlePercent ,PREC2, 4, LCD_BLACK, LCD_STATUS_GREY ) ;
		lcdDrawNumber( 5*FWCOLOUR, STATUS_VERTICAL+3, MixerRate, LEFT, 3 ) ;
//  	lcd_outdezNAtt( 8*FW , (STATUS_VERTICAL/2)+3, MixerRate, 0, 3, LCD_BLACK, LCD_STATUS_GREY ) ;
		if ( scriptPercent )
		{
//		  lcd_outdezNAtt( 12*FW, (STATUS_VERTICAL/2)+3, BasicExecTime ,PREC1, 4, LCD_BLACK, LCD_STATUS_GREY ) ;
			lcdDrawNumber( 9*FWCOLOUR, STATUS_VERTICAL+3, BasicExecTime, LEFT|PREC1, 4 ) ;
		}

//		lcd_outdezNAtt( 16*FW, (STATUS_VERTICAL/2)+3, HiresSavedTime/200 ,PREC1, 4, LCD_BLACK, LCD_STATUS_GREY ) ;
		lcdDrawNumber( 13*FWCOLOUR, STATUS_VERTICAL+3, HiresSavedTime/200, LEFT|PREC1, 4 ) ;
	}
	SuppressStatusLine = 0 ;
}


//#ifndef TOUCH

uint16_t dimBackColour()
{
	uint32_t red ;
	uint32_t green ;
	uint32_t blue ;
	uint16_t colour ;

	colour = LcdBackground ;

	red = (colour >> 11) ;
	green = (( colour >> 6 ) & 0x1F) ;
	blue = (colour & 0x1F) ;

	if ( red+green+blue < 36 )
	{
		red = red *12/8 ;
		green = green *12/8 ;
		blue = blue *12/8 ;
	}
	else
	{
		red = red *8/10 ;
		green = green *8/10 ;
		blue = blue *8/10 ;
	}

	colour = ( red << 11 ) | ( green << 6 ) | ( blue ) ;
	DimBackColour = colour ;
	return colour ;
}

void drawItem( char *s, uint16_t y, uint16_t selected )
{
//	PUTS_P( THOFF, y+TVOFF, s ) ;
	lcdDrawText( THOFF*2, y*2+TVOFF, s ) ;
	lcd_hline( 0, y+TFH, TRIGHT ) ;
	lcdDrawSolidFilledRectDMA( TMID*TSCALE, y*TSCALE+2, (TRIGHT-TMID)*TSCALE, TFH*TSCALE-2, selected ? ~DimBackColour : DimBackColour ) ;
}

void drawNumber( uint16_t x, uint16_t y, int32_t val, uint16_t mode ) //, uint16_t colour )
{
	uint16_t fcolour = LcdForeground ;
	if ( mode & INVERS )
	{
		if ( ! ( s_editMode && BLINK_ON_PHASE ) )
		{
			fcolour = ~LcdForeground ;
		}
	}
	lcdDrawNumber( x*2, y*2+TVOFF, val, mode & ~INVERS,5, fcolour) ; //, colour ) ;
}		  

void drawText( uint16_t x, uint16_t y, char *s, uint16_t mode )
{
	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
	lcdDrawSizedText( x*2, y*2+TVOFF, s, 255, mode & ~INVERS, fcolour ) ;
//	lcd_putsAttColour( x, y+TVOFF, s, 0, fcolour ) ;
}		  

void drawChar( uint16_t x, uint16_t y, uint8_t c, uint16_t mode, uint16_t colour )
{
	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
  lcdDrawChar( x*2, y*2+TVOFF, c, 0, fcolour ) ;
//	lcd_putcAttColour( x, y+TVOFF, c, 0, fcolour) ; //, colour ) ;
}		  

void drawIdxText( uint16_t y, char *s, uint32_t index, uint16_t mode )
{
	uint8_t length ;
	length = *s++ ;
	
//	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
	uint16_t fcolour = LcdForeground ;
	if ( mode & INVERS )
	{
		if ( ! ( s_editMode && BLINK_ON_PHASE ) )
		{
			fcolour = ~LcdForeground ;
		}
	}
	lcdDrawSizedText( (TRIGHT-TRMARGIN)*2, y*2+TVOFF, s+length*index, length, LUA_RIGHT, fcolour ) ;
//  lcd_putsnAttColour( TRIGHT-length*FW-TRMARGIN, y+TVOFF, s+length*index, length, 0, fcolour ) ;
}		  

void drawIdxTextAtX( coord_t x, uint16_t y, char *s, uint32_t index, uint16_t mode )
{
	uint8_t length ;
	length = *s++ ;
	
//	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
	uint16_t fcolour = LcdForeground ;
	if ( mode & INVERS )
	{
		if ( ! ( s_editMode && BLINK_ON_PHASE ) )
		{
			fcolour = ~LcdForeground ;
		}
	}
	lcdDrawSizedText( x*2, y, s+length*index, length, LUA_RIGHT, fcolour ) ;
//  lcd_putsnAttColour( TRIGHT-length*FW-TRMARGIN, y+TVOFF, s+length*index, length, 0, fcolour ) ;
}		  

void DrawDrSwitches( coord_t x, coord_t y, int8_t idx1, LcdFlags att)
{
//	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;
//	LcdBackground = bcolour ;
//	LcdForeground = fcolour ;
//	putsDrSwitches( x, y, idx1, att ) ;
//	LcdBackground = oldBcolour ;
//	LcdForeground = oldFcolour ;

	uint16_t fcolour = LcdForeground ;
	if ( att & INVERS )
	{
		if ( ! ( s_editMode && BLINK_ON_PHASE ) )
		{
			fcolour = ~LcdForeground ;
		}
	}
	LcdForeground = fcolour ;
	putsDrSwitches( x, y, idx1, att&~INVERS ) ;
	LcdForeground = oldFcolour ;
}


uint32_t touchOnOffItemMultiple( uint8_t value, coord_t y, uint8_t condition, uint16_t colour )
{
  if (value)
	{
		putTick( (TRIGHT-TRMARGIN-FW)-2, (y+TVOFF), condition ? ~LcdForeground : LcdForeground ) ;
//		lcd_putcAttColour( TRIGHT-TRMARGIN-FW, y+3, '\202', 0, condition ? ~LcdForeground : LcdForeground, condition ? ~colour : colour ) ;
	}
//	lcd_hbar( TRIGHT-TRMARGIN-FW-1, y+TVOFF-1, 7, 7, condition ? 100 : 0 ) ;
	lcd_rectColour( TRIGHT-TRMARGIN-FW-2, y+TVOFF, 8, 8, condition ? ~LcdForeground : LcdForeground ) ;
//	lcd_hbar( TRIGHT-FW-6, y+TVOFF, 7, 7, 0 ) ;
	if ( condition )
	{
		value = checkIncDec( value, 0, 1, EditType ) ;
	}
	return value ;
}


uint32_t touchOnOffItem( uint8_t value, coord_t y, const prog_char *s, uint8_t condition, uint16_t colour )
{
	drawItem( (char *)s, y, condition ) ;
	return touchOnOffItemMultiple( value, y, condition, colour ) ;
}

uint32_t touchOffOnItem( uint8_t value, coord_t y, const prog_char *s, uint8_t condition, uint16_t colour )
{
	return 1- touchOnOffItem( 1- value, y, s, condition, colour ) ;
}

void putsChnColour( coord_t x, coord_t y, uint8_t idx1, LcdFlags att )
{
//	uint16_t fcolour = LcdForeground ;
//	uint16_t bcolour = LcdBackground ;
//	if ( att & INVERS )
//	{
//		fcolour = ~LcdForeground ;
//		bcolour = ~LcdBackground ;
//	}
	uint16_t fcolour = LcdForeground ;
	if ( att & INVERS )
	{
		if ( ! ( s_editMode && BLINK_ON_PHASE ) )
		{
			fcolour = ~LcdForeground ;
		}
	}
	if ( idx1 == 0 )
	{
//    lcd_putsnAttColour( x, y, XPSTR("---"), 4, att, fcolour, bcolour ) ;
		lcdDrawSizedText( x, y, XPSTR("---"), 4, att, fcolour ) ;
	}
	else
	{
//		uint16_t x1 ;
//		x1 = x + 4*FW-2 ;
//		if ( idx1 < 10 )
//		{
//			x1 -= FWNUM ;			
//		}
//    lcd_putsnAttColour( x, y, PSTR(STR_CH), 2, 0, fcolour, bcolour ) ;
		lcdDrawSizedText( x, y, PSTR(STR_CH), 2, att&BOLD, fcolour ) ;
//  	PUTS_NUM_N( x1, y, idx1, 0, 2, fcolour, bcolour ) ;
		lcdDrawNumber( LcdNextPos, y, idx1, LEFT|(att&BOLD), 0, fcolour ) ;
		
	}
}

//static uint16_t oldBcolour ;
static uint16_t oldFcolour ;
void saveEditColours( uint32_t attr, uint16_t colour )
{
//	oldBcolour = LcdBackground ;
	oldFcolour = LcdForeground ;
//	LcdBackground = attr ? ~colour : colour ;
	if ( attr )
	{
		LcdForeground = ~LcdForeground ;
	}
}

void restoreEditColours()
{
//	LcdBackground = oldBcolour ;
	LcdForeground = oldFcolour ;
}

//#endif // nTOUCH

void menuCellScaling(uint8_t event)
{
	TITLE( XPSTR( "Cell Scaling" ) ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2 ;
	uint32_t rows = 12 ;
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	mstate2.check_columns(event, rows-1 ) ;	
	
	coord_t y = 0 ;
	uint32_t k ;
	uint32_t t_pgOfs ;
	uint32_t index ;
  uint32_t sub = mstate2.m_posVert ;
#ifdef TOUCH
	uint32_t newVpos ;
#endif
	t_pgOfs = evalHresOffset( sub ) ;
	
#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	
	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	lcd_hline( 0, TTOP, TRIGHT ) ;

	for( uint32_t i = 0 ; i < TLINES ; i += 1 )
	{
    y = i * TFH + TTOP ;
    k=i+t_pgOfs;
    LcdFlags attr = (sub==i) ? INVERS : 0 ;
		drawItem( (char *)XPSTR("Cell"), y, attr ) ;
//		PUTS_ATT_LEFT( y, XPSTR( "Cell\023v" ) ) ;
 		PUTS_NUM( LcdNextPos, y+TVOFF/2, k+1, LEFT ) ;
//		PUTS_NUMX(  6*FW, y, k+1 ) ;
		
		index = k + FR_CELL1 ;
		attr = TelemetryDataValid[index] ? 0 : BLINK ;
 		PUTS_NUM(  12*FW, y+TVOFF/2, TelemetryData[index], attr | PREC2 ) ;
		lcdDrawChar( 12*FW*2, y*2+TVOFF, 'v' ) ;

//    attr = ((sub==k) ? InverseBlink : 0);
    attr = (sub==i) ? INVERS : 0 ;
		uint32_t scaling = 1000 + g_model.cellScalers[k] ;
 		PUTS_NUM( TRIGHT-FW-TRMARGIN, y+TVOFF/2, scaling/10, attr | PREC2 ) ;
 		PUTS_NUM( TRIGHT-FW-TRMARGIN, y+TVOFF/2, scaling%10, attr | LEFT ) ;
    if( attr )
		{
    	CHECK_INCDEC_H_MODELVAR( g_model.cellScalers[k], -50, 50 ) ;
		}
	}
}


// FrSky WSHhi DSMx  Jeti  ArduP ArduC FrHub HubRaw FrMav Mavlk"

#define MAX_TEL_OPTIONS		8
const uint8_t TelOptions[] = {1,2,3,5,6,7,11,12} ;

// 0 Telemetry
// 1 SbusTrain
// 2 Sbus57600
// 3 BTdirect 
// 4 CppmTrain
// 5 LCDdump  
// 6 Tel+BTdir
// 7 Script
// 8 BT/ENC

#if defined(PCBX12D)// || defined(PCBX10)
 #define MAX_COM2_OPTIONS		5
 const uint8_t Com2Options[] = {0,1,2,3,4} ;
#endif

void menuProcTelemetry(uint8_t event)
{
	uint32_t page2SizeExtra = 0 ;
//	if ( g_model.telemetryProtocol == TELEMETRY_DSM )
//	page2SizeExtra = 1 ;
	if ( FrskyTelemetryType == FRSKY_TEL_DSM )
	{
		page2SizeExtra += 4 ;
	}

	EditType = EE_MODEL ;
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif

#if not (defined(PCBX10))
#define TDATAITEMS	23
#else
#define TDATAITEMS	20
#endif
		 
	TITLE(PSTR(STR_TELEMETRY));
	uint32_t rows ;
	rows = TDATAITEMS+page2SizeExtra ;
	static MState2 mstate2;
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	event = mstate2.check_columns( event, rows-1 ) ;

	uint32_t  sub   = mstate2.m_posVert ;
	uint32_t subSub = g_posHorz ;
	uint32_t blink ;
  uint32_t t_pgOfs ;
#ifdef TOUCH
	uint32_t newVpos ;
#endif

	t_pgOfs = evalHresOffset( sub ) ;
	 
#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	
	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	coord_t y = TTOP ;
	lcd_hline( 0, TTOP, TRIGHT ) ;

	blink = InverseBlink ;
	uint32_t subN = 0 ;
 
//	if ( sub < 9+page2SizeExtra )
// 	{
 //		PUTS_P( 22*FW, 14*FHPY, XPSTR("->") ) ;
		
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		
		drawItem( (char *)PSTR(STR_USR_PROTO), y, attr ) ;
		uint8_t b ;

		b = g_model.telemetryProtocol ;
		if(sub==subN)
		{
			b = checkOutOfOrder( b, (uint8_t *)TelOptions, MAX_TEL_OPTIONS ) ;
		}
		g_model.telemetryProtocol = b ;
		switch ( b )
		{
			case TELEMETRY_WSHHI :
				g_model.FrSkyUsrProto = 1 ;
				g_model.DsmTelemetry = 0 ;
			break ;

			case TELEMETRY_DSM :
				g_model.FrSkyUsrProto = 0 ;
				g_model.DsmTelemetry = 1 ;
			break ;

			case TELEMETRY_FRSKY :
			default :
				g_model.FrSkyUsrProto = 0 ;
				g_model.DsmTelemetry = 0 ;
			break ;
		}
		b -= 1 ;
		drawIdxText( y, (char *)PSTR(STR_FRHUB_WSHHI), b, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
	
	if(t_pgOfs<=subN)
	{
		uint8_t b ;
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		drawItem( (char *)XPSTR("Units"), y, attr ) ;
		b = g_model.FrSkyImperial ;
		if(attr)
		{
			CHECK_INCDEC_H_MODELVAR_0(b,1) ;
			g_model.FrSkyImperial = b ;
		}
		drawIdxText( y, (char *)PSTR(STR_MET_IMP), b, attr ) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	for (uint32_t i = 0 ; i < 4 ; i += 1 )
	{
		if(t_pgOfs<=subN)
		{
	    uint32_t attr = (sub==subN) ? INVERS : 0 ;
			uint32_t index = i & 1 ;
			uint8_t unit ;
			char text[20] ;
			cpystr( (uint8_t *)text, (uint8_t *)PSTR(STR_A_CHANNEL) ) ;
  	  text[1] = '1'+i ;
			drawItem( text, y, attr ) ;
			if ( i < 2 )
			{
				uint16_t fcolour = (sub==subN && subSub==0) ? ~LcdForeground : LcdForeground ;
  	  	putsTelemValue( TRIGHT-TRMARGIN-FW, y+TVOFF/2, 255, i, NO_UNIT, fcolour ) ;
  	  	putsTelemValue( 16*FW, y+TVOFF/2, frskyTelemetry[i].value, i,  NO_UNIT ) ;
  	  	unit = g_model.frsky.channels[index].units ;

  	  	if (sub==subN)
				{
					Columns = 1 ;
					if ( (s_editMode /*|| P1values.p1valdiff*/))
					{
  	  	    switch (subSub)
						{
  	  	    	case 0:
  	  	        g_model.frsky.channels[i].lratio = checkIncDec16( g_model.frsky.channels[i].lratio, 0, 1000, EE_MODEL);
  	  	      break;
		  	      case 1:
//  	  	        CHECK_INCDEC_H_MODELVAR_0( g_model.frsky.channels[i].units, 3);
								g_model.frsky.channels[i].units = checkOutOfOrder( g_model.frsky.channels[i].units, (uint8_t *)"\000\001\003", 3 ) ;
  	  	      break;
  	  	    }
					}
  	  	}
			}
			else
			{
//				PUTS_NUM( TRIGHT-TRMARGIN-FW, y+TVOFF/2, g_model.frsky.channels[index].ratio3_4, (sub==subN && subSub==0 ? blink:0)|PREC2 ) ;
		    attr = (sub==subN && subSub==0 ) ? INVERS : 0 ;
				drawNumber( TRIGHT-TRMARGIN-FW, y, g_model.frsky.channels[index].ratio3_4, attr|PREC1 ) ;
				PUTS_NUM( 16*FW, y+TVOFF/2, TelemetryData[FR_A3+index], PREC2 ) ;
  	  	unit = g_model.frsky.channels[index].units3_4 ;
  	  	if (sub==subN)
				{
					Columns = 1 ;
					if ( (s_editMode /*|| P1values.p1valdiff*/))
					{
  	  	    switch (subSub)
						{
  	  	    	case 0:
  	  	        g_model.frsky.channels[index].ratio3_4 = checkIncDec16( g_model.frsky.channels[index].ratio3_4, 0, 4800, EE_MODEL);
  	  	      break;
		  	      case 1:
//  	  	        CHECK_INCDEC_H_MODELVAR_0( g_model.frsky.channels[index].units3_4, 3);
								g_model.frsky.channels[index].units3_4 = checkOutOfOrder( g_model.frsky.channels[index].units3_4, (uint8_t *)"\000\001\003", 3 ) ;
  	  	      break;
  	  	    }
					}
  	  	}
			}
	    attr = (sub==subN && subSub==1 ) ? INVERS : 0 ;
			drawIdxText( y, (char *)XPSTR("\001v-VA"), unit, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
			if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
		}
		subN += 1 ;
	}
		
		
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
//		lcd_xlabel_decimal( 17*FW+1, y, g_model.rxVratio, attr|PREC1, XPSTR( "Rx Voltage") ) ;
		drawItem( (char *)XPSTR("Rx Voltage"), y, attr ) ;
		drawNumber( TRIGHT-TRMARGIN-FW, y, g_model.rxVratio, attr|PREC1 ) ;
		
//		PUTC( TRIGHT-TRMARGIN-FW, y+TVOFF/2, 'v' ) ;
  	lcdDrawText( (TRIGHT-TRMARGIN)*2, y*2+TVOFF, "v", LUA_RIGHT ) ;
//		lcd_outdezAtt( 22*FW+1, y, convertRxv( TelemetryData[FR_RXV] ), PREC1 ) ;
		PUTS_NUM( 16*FW+1, y+TVOFF/2, TelemetryData[FR_RXV], PREC1 ) ;
		if( attr)
		{
			g_model.rxVratio = checkIncDec16( g_model.rxVratio, 0, 255, EE_MODEL ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		drawItem( (char *)XPSTR("Cell Scaling"), y, attr ) ;
		if ( sub == subN )
		{
			if (checkForMenuEncoderBreak( event ) )
			{
  	  	pushMenu( menuCellScaling ) ;
			}
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
		
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		if (sub==subN)
		{
			Columns = 1 ;
		}
		drawItem( (char *)XPSTR("RSSI Warn"), y, (sub==subN) ) ;
		uint8_t b ;
		b = 1-g_model.enRssiOrange ;
		b = touchOnOffItemMultiple( b, y, ( (sub==subN) && (subSub==1) ), DimBackColour ) ;
		g_model.enRssiOrange = 1-b ;

		attr = ( (sub==subN) && (subSub==0) ) ? INVERS : 0 ;
		int8_t offset ;
		offset = rssiOffsetValue( 0 ) ;
//		PUTS_NUM( 22*FW, y+TVOFF/2, g_model.rssiOrange + offset, attr ) ;
		drawNumber( 22*FW, y, g_model.rssiOrange + offset, attr ) ;
  	if( attr)
		{
			CHECK_INCDEC_H_MODELVAR( g_model.rssiOrange, -18, 30 ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
		
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		if (sub==subN)
		{
			Columns = 1 ;
			if ( subSub==0 )
			{
				lcdDrawSolidFilledRectDMA( TMID*TSCALE, y*TSCALE+2, (TRIGHT-TMID)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
			}
		}
		drawItem( (char *)XPSTR("RSSI Critical"), y, (sub==subN) ) ;
		uint8_t b ;
		b = 1-g_model.enRssiRed ;
		b = touchOnOffItemMultiple( b, y, ( (sub==subN) && (subSub==1) ), DimBackColour ) ;
		g_model.enRssiRed = 1-b ;
		
		attr = ( (sub==subN) && (subSub==0) ) ? INVERS : 0 ;
		int8_t offset ;
		offset = rssiOffsetValue( 1 ) ;
//		PUTS_NUM( 22*FW, y_TVOFF/2, g_model.rssiRed + offset, attr ) ;
		drawNumber( 22*FW, y, g_model.rssiRed + offset, attr ) ;
		if( attr)
		{
			CHECK_INCDEC_H_MODELVAR( g_model.rssiRed, -17, 30 ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
 
	if ( page2SizeExtra )
	{
		if(t_pgOfs<=subN)
		{
  	  uint32_t attr = (sub==subN) ? INVERS : 0 ;
			g_model.dsmAasRssi = touchOnOffItem( g_model.dsmAasRssi, y, (char *)XPSTR("DSM 'A' as RSSI"), attr, DimBackColour ) ;
			if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
		}
		subN += 1 ;
		
		if(t_pgOfs<=subN)
		{
  	  uint32_t attr = (sub==subN) ? INVERS : 0 ;
			if (sub==subN)
			{
				Columns = 1 ;
			}
			drawItem( (char *)XPSTR("DSM Warning"), y, attr ) ;
		
			attr = ( (sub==subN) && (subSub==0) ) ? INVERS : 0 ;
//			g_model.dsmLinkData.sourceWarn = checkIndexed( y+TVOFF/2, XPSTR("\226""\002""\006fades lossesholds "), g_model.dsmLinkData.sourceWarn, (sub==subN) && (subSub==0) ) ;
			drawIdxTextAtX( 25*FW, y*2+TVOFF, (char *)XPSTR("\006fades lossesholds "), g_model.dsmLinkData.sourceWarn, attr ) ;
			if ( attr )
			{
 				CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.sourceWarn, 2 ) ;
			}
	//		PUTS_NUM( 23*FW, y, g_model.dsmLinkData.levelWarn, attr ) ;
			attr = ( (sub==subN) && (subSub==1) ) ? INVERS : 0 ;
			drawNumber( TRIGHT-TRMARGIN, y, g_model.dsmLinkData.levelWarn, attr ) ;
 			if( attr)
			{
				CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.levelWarn, 40 ) ;
			}
			if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
		}
		subN += 1 ;

		if(t_pgOfs<=subN)
		{
  	  uint32_t attr = (sub==subN) ? INVERS : 0 ;
			if (sub==subN)
			{
				Columns = 1 ;
			}
			drawItem( (char *)XPSTR("DSM Critical"), y, attr ) ;
			attr = ( (sub==subN) && (subSub==0) ) ? INVERS : 0 ;
//			g_model.dsmLinkData.sourceCritical = checkIndexed( y+TVOFF/2, XPSTR("\226""\002""\006fades lossesholds "), g_model.dsmLinkData.sourceCritical, (sub==subN) && (subSub==0) ) ;
			drawIdxTextAtX( 25*FW, y*2+TVOFF, (char *)XPSTR("\006fades lossesholds "), g_model.dsmLinkData.sourceCritical, attr ) ;
			if ( attr )
			{
 				CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.sourceCritical, 2 ) ;
			}
			
	//		PUTS_NUM( 23*FW, y, g_model.dsmLinkData.levelCritical, attr ) ;
			attr = ( (sub==subN) && (subSub==1) ) ? INVERS : 0 ;
			drawNumber( TRIGHT-TRMARGIN, y, g_model.dsmLinkData.levelCritical, attr ) ;
 			if( attr)
			{
				CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.levelCritical, 40 ) ;
			}
			if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
		}
		subN += 1 ;

		if(t_pgOfs<=subN)
		{
  	  uint32_t attr = (sub==subN) ? INVERS : 0 ;
			drawItem( (char *)XPSTR("DSM Vario"), y, attr ) ;
//			g_model.dsmVario = checkIndexed( y+TVOFF/2, XPSTR("\226" "\005" "\0040.250.5 1.0 1.5 2.0 3.0 "), g_model.dsmVario, (sub==subN) ) ;
  		drawIdxText( y, XPSTR("\0040.25 0.5 1.0 1.5 2.0 3.0"), g_model.dsmVario, attr|LUA_RIGHT ) ; // , attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
	  	if(attr)
			{
				CHECK_INCDEC_H_MODELVAR_0( g_model.dsmVario, 5 ) ;
			}

			if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
		}
		subN += 1 ;
	}
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
//		lcd_xlabel_decimal( PARAM_OFS+6*FW, y, g_model.numBlades, attr, PSTR(STR_NUM_BLADES) ) ;
		drawItem( (char *)PSTR(STR_NUM_BLADES), y, attr ) ;
		drawNumber( TRIGHT-TRMARGIN, y, g_model.numBlades, attr ) ;
  	if(attr)
		{
			CHECK_INCDEC_H_MODELVAR( g_model.numBlades, 1, 127 ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		drawItem( (char *)PSTR(STR_ALT_ALARM), y, attr ) ;
//  	PUTS_AT_IDX(PARAM_OFS+3*FW+2, y, PSTR(STR_OFF122400),g_model.FrSkyAltAlarm, attr ) ;
		drawIdxText( y, (char *)PSTR(STR_OFF122400), g_model.FrSkyAltAlarm, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
  	if(attr)
		{
			CHECK_INCDEC_H_MODELVAR_0( g_model.FrSkyAltAlarm, 2 ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
  
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		drawItem( (char *)PSTR(STR_VOLT_THRES), y, attr ) ;
//  	PUTS_NUM_N(  PARAM_OFS+6*FW, y, g_model.frSkyVoltThreshold * 2 ,((sub==subN) ? blink:0) | PREC2, 4);
		drawNumber( TRIGHT-TRMARGIN, y, g_model.frSkyVoltThreshold * 2, attr|PREC1 ) ;
  	if(sub==subN)
		{
  	  g_model.frSkyVoltThreshold=checkIncDec16( g_model.frSkyVoltThreshold, 0, 210, EE_MODEL);
  	}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
	
	if(t_pgOfs<=subN)
	{
		uint32_t attr = (sub==subN) ? INVERS : 0 ;
		g_model.FrSkyGpsAlt = touchOnOffItem( g_model.FrSkyGpsAlt, y, (char *)PSTR(STR_GPS_ALTMAIN), attr, DimBackColour ) ;
//		drawItem( (char *)PSTR(STR_GPS_ALTMAIN), y, attr ) ;
//  	menu_lcd_onoff( PARAM_OFS+5*FW, y, g_model.FrSkyGpsAlt, sub==subN ) ;
//  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.FrSkyGpsAlt, 1);
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
		
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		if (sub==subN)
		{
			Columns = 1 ;
		}
		uint16_t value = g_model.frskyAlarms.alarmData[0].frskyAlarmLimit << 6 ;
		drawItem( (char *)PSTR(STR_MAH_ALARM), y, attr ) ;
  	attr = ((sub==subN && subSub==0) ? InverseBlink : 0);
		uint8_t active = (attr && s_editMode) ;
//  	PUTS_NUM( 15*FW, y+TVOFF/2, value, attr ) ;
		drawNumber( TRIGHT-TRMARGIN-8*FW, y, value, attr ) ;
		if ( active )
		{
  		g_model.frskyAlarms.alarmData[0].frskyAlarmLimit = checkIncDec16( g_model.frskyAlarms.alarmData[0].frskyAlarmLimit, 0, 200, EE_MODEL);
		}
  	attr = ((sub==subN && subSub==1) ? InverseBlink : 0);
		active = (attr && s_editMode) ;
//		PUTS_AT_IDX(PARAM_OFS+1*FW+1, y, PSTR(STR_SOUNDS), g_model.frskyAlarms.alarmData[0].frskyAlarmSound,attr);
		drawIdxText( y, (char *)PSTR(STR_SOUNDS), g_model.frskyAlarms.alarmData[0].frskyAlarmSound, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
		if ( active )
		{
  		CHECK_INCDEC_H_MODELVAR_0( g_model.frskyAlarms.alarmData[0].frskyAlarmSound, 15 ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
		uint32_t attr = (sub==subN) ? INVERS : 0 ;
		g_model.bt_telemetry = touchOnOffItem( g_model.bt_telemetry, y, (char *)PSTR(STR_BT_TELEMETRY), attr, DimBackColour ) ;
//		uint32_t attr = (sub==subN) ? INVERS : 0 ;
//		drawItem( (char *)PSTR(STR_BT_TELEMETRY), y, attr ) ;
//  	menu_lcd_onoff( PARAM_OFS+5*FW, y, g_model.bt_telemetry, sub==subN ) ;
//  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.bt_telemetry, 1);
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
  	
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		drawItem( (char *)PSTR(STR_FRSKY_COM_PORT), y, attr ) ;
//  	PUTC_ATT( PARAM_OFS+5*FW, y, g_model.frskyComPort + '1', attr ) ;
		drawChar( TRIGHT-TRMARGIN-FW, y, g_model.frskyComPort + '1', attr, DimBackColour ) ;
		if (attr)
		{
			CHECK_INCDEC_H_MODELVAR_0( g_model.frskyComPort, 1 ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		uint8_t previous = g_model.telemetryRxInvert ;
		previous |= g_model.telemetry2RxInvert << 1 ;
		uint8_t next = previous ;

		drawItem( (char *)XPSTR("Com Port Invert"), y, attr ) ;
		attr = 0 ;
		if(sub==subN)
		{
			attr = InverseBlink ;
			CHECK_INCDEC_H_MODELVAR_0( next, 1 ) ;
		}
//		PUTS_AT_IDX(PARAM_OFS+2*FW, y, XPSTR("\004None   1   2"), next, attr ) ;
		drawIdxText( y, (char *)XPSTR("\004None   1   2"), next, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;

		if ( next != previous )
		{
			g_model.telemetryRxInvert = next & 0x01 ;
			g_model.telemetry2RxInvert = (next >> 1 ) & 0x01 ;
			TelemetryType = TEL_UNKNOWN ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
		 
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		drawItem( (char *)PSTR(STR_FAS_OFFSET), y, attr ) ;
    attr = PREC1 ;
		if ( (sub == subN) )
		{
			attr = blink | PREC1 ;
      CHECK_INCDEC_H_MODELVAR_0( g_model.FASoffset, 15 ) ;
		}
//  	PUTS_NUM( PARAM_OFS+5*FW+5, y, g_model.FASoffset, attr ) ;
		drawNumber( TRIGHT-TRMARGIN, y, g_model.FASoffset, attr ) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

#if not (defined(PCBX10))
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		drawItem( (char *)XPSTR("COM2 Func."), y, attr ) ;
		uint8_t b = g_model.com2Function ;
    attr = 0 ;
		if ( (sub == subN) )
		{
			attr = blink ;
			b = checkOutOfOrder( b, (uint8_t *)Com2Options, MAX_COM2_OPTIONS ) ;
		}
//		PUTS_AT_IDX(PARAM_OFS, y, XPSTR("\011TelemetrySbusTrainSbus57600BTdirect CppmTrain"), g_model.com2Function, attr ) ;
		drawIdxText( y, (char *)XPSTR("\011TelemetrySbusTrainSbus57600 BTdirectCppmTrain"), g_model.com2Function, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
		if ( g_model.com2Function != b )
		{
			g_model.com2Function = b ;
			
			com2Configure() ;
			if( g_model.com2Function == COM2_FUNC_TELEMETRY )
			{
				if ( g_model.frskyComPort == 1 )
				{
					telemetry_init( TEL_FRSKY_HUB ) ;
				}
			}
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
		
	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		drawItem( (char *)XPSTR("COM2 Baudrate"), y, attr ) ;
		g_model.com2Baudrate = checkIndexed( y+TVOFF/2, XPSTR(BaudString), g_model.com2Baudrate, (sub==subN) ) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
#endif

	if(t_pgOfs<=subN)
	{
    uint32_t attr = (sub==subN) ? INVERS : 0 ;
		drawItem( (char *)XPSTR("Current Source"), y, attr ) ;
//		PUTS_AT_IDX( PARAM_OFS+2*FW, y, XPSTR("\004----A1  A2  Fas SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "), g_model.currentSource, attr ) ;
		drawIdxText( y, (char *)XPSTR("\004----  A1  A2 Fas SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8"), g_model.currentSource, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
   	if(attr)
		{
			CHECK_INCDEC_H_MODELVAR_0( g_model.currentSource, 3 + NUM_SCALERS ) ;
	  }
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
}





void menuHeli( uint8_t event )
{
	TITLE( PSTR(STR_HELI_SETUP) ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif

	static MState2 mstate2;
	uint32_t rows = 6 ;
	mstate2.check_columns(event, rows-1) ;

  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	uint8_t subN = 0 ;
	uint16_t colour = DimBackColour ;
	uint8_t attr ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
	 
	y = TTOP ;
	 
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Swash Type"), y, attr ) ;
	drawIdxText( y, (CHAR *)PSTR(SWASH_TYPE_STR)+2, g_model.swashType, attr|LUA_RIGHT ) ; // , attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
	if(attr) CHECK_INCDEC_H_GENVAR_0( g_model.swashType, 4 ) ;
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Collective"), y, attr ) ;
	if( attr )
	{
		uint8_t x = mapPots( g_model.swashCollectiveSource ) ;
		CHECK_INCDEC_H_MODELVAR_0( x, NUM_SKYXCHNRAW+NumExtraPots ) ;
		g_model.swashCollectiveSource = unmapPots( x ) ;
	}
  putsChnRaw( (TRIGHT-TRMARGIN)*2, (y+TVOFF)*2, g_model.swashCollectiveSource, LUA_RIGHT, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Swash Ring"), y, attr ) ;
	drawNumber( TRIGHT-5, y, g_model.swashRingValue, attr) ; //, attr ? ~colour : colour ) ;
	if( attr )
	{
		CHECK_INCDEC_H_MODELVAR_0( g_model.swashRingValue, 100) ;
	}
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("ELE Direction"), y, attr ) ;
	drawIdxText( y, (char *)PSTR( STR_HYPH_INV)+2, g_model.swashInvertELE, attr|LUA_RIGHT ) ; // , attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
		g_model.swashInvertELE = checkIncDec( g_model.swashInvertELE, 0, 1, EditType ) ;
	}
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("AIL Direction"), y, attr ) ;
	drawIdxText( y, (char *)PSTR( STR_HYPH_INV)+2, g_model.swashInvertAIL, attr|LUA_RIGHT ) ; // , attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
		g_model.swashInvertAIL = checkIncDec( g_model.swashInvertAIL, 0, 1, EditType ) ;
	}
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("COL Direction"), y, attr ) ;
	drawIdxText( y, (char *)PSTR( STR_HYPH_INV)+2, g_model.swashInvertCOL, attr|LUA_RIGHT ) ; // , attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
		g_model.swashInvertCOL	 = checkIncDec( g_model.swashInvertCOL, 0, 1, EditType ) ;
	}
}

void menuLimitsOne(uint8_t event)
{
	static MState2 mstate2 ;
	uint8_t attr ;
	uint32_t index ;
	uint32_t rows = 4 ;
	index = s_currIdx ;
  int8_t limit = (g_model.extendedLimits ? 125 : 100) ;
	uint16_t y = TTOP ;
	int16_t value ;
	int16_t t = 0 ;
	
//	lcd_putsAtt( 4*FW, 0, PSTR(STR_LIMITS), INVERS ) ;
	lcdDrawText( 4*FW*HVSCALE, 0, PSTR(STR_LIMITS), INVERS|BOLD ) ;

#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	mstate2.check_columns(event, rows-1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;

	putsChn( 14*FW, 0,index+1,0 ) ;
//  lcd_outdez( 23*FW, 0, g_chans512[index]/2 + 1500 ) ;
	lcdDrawNumber( 23*FW*2, 0, g_chans512[sub]/2 + 1500 ) ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;

//	uint16_t colour = dimBackColour() ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

  LimitData *ld = &g_model.limitData[index];
#if EXTRA_SKYCHANNELS
	if ( index >= NUM_SKYCHNOUT )
	{
		ld = &g_model.elimitData[index-NUM_SKYCHNOUT] ;
	}
#endif			
	attr = sub==0 ? INVERS : 0 ;
	
	drawItem( XPSTR("Sub Trim"), y, (sub == 0 ) ) ;
	drawNumber( TRIGHT-TRMARGIN, y, ld->offset, attr|PREC1 ) ; //, attr ? ~colour : colour ) ;
	if ( attr )
	{
		StepSize = 50 ;
		ld->offset = checkIncDec16( ld->offset, -1000, 1000, EE_MODEL ) ;
	}
	y += TFH ;
	attr = sub==1 ? INVERS : 0 ;
	if ( g_model.sub_trim_limit )
	{
		if ( ( t = ld->offset ) )
		{
			if ( t > g_model.sub_trim_limit )
			{
				t = g_model.sub_trim_limit ;
			}
			else if ( t < -g_model.sub_trim_limit )
			{
				t = -g_model.sub_trim_limit ;
			}
		}
	}
	value = t / 10 ;
	value += (int8_t)(ld->min-100) ;
	if ( value < -125 )
	{
		value = -125 ;						
	}
	drawItem( XPSTR("Min"), y, (sub == 1 ) ) ;
	drawNumber( TRIGHT-TRMARGIN, y, value, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
    ld->min -=  100;
		CHECK_INCDEC_H_MODELVAR( ld->min, -limit,25);
    ld->min +=  100;
  }
	y += TFH ;
	attr = sub==2 ? INVERS : 0 ;
	value = t / 10 ;
	value += (int8_t)(ld->max+100) ;
	if ( value > 125 )
	{
		value = 125 ;						
	}

	drawItem( XPSTR("Max"), y, (sub == 2 ) ) ;
	drawNumber( TRIGHT-TRMARGIN, y, value, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
    ld->max +=  100;
    CHECK_INCDEC_H_MODELVAR( ld->max, -25,limit);
    ld->max -=  100;
  }
	y += TFH ;
	attr = sub==3 ? INVERS : 0 ;

	drawItem( XPSTR("Reverse"), y, (sub == 3 ) ) ;
//	lcd_putsAttIdxColour( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR( STR_HYPH_INV)+2, ld->revert, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
	drawIdxText( y, (char *)PSTR( STR_HYPH_INV)+2, ld->revert, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
  if(attr)
	{
		ld->revert = checkIncDec( ld->revert, 0, 1, EditType ) ;
	}
}



void putsTrimMode( coord_t x, coord_t y, uint8_t phase, uint8_t idx, uint8_t att ) ;
//void alphaEditName( uint8_t x, uint8_t y, uint8_t *name, uint8_t len, uint16_t type, uint8_t *heading ) ;

void menuModeOne(uint8_t event)
{
  PhaseData *phase ;
	uint32_t t_pgOfs ;
#ifdef TOUCH
	uint32_t newVpos ;
#endif
	uint32_t index = s_currIdx ;
	if ( index )
	{
		index -= 1 ;
		phase = (index < MAX_MODES) ? &g_model.phaseData[index] : &g_model.xphaseData ;
	}
	else
	{
		phase = &g_model.xphaseData ;	// Keep compiler happy
	}
	static MState2 mstate2 ;
	TITLE(PSTR(STR_FL_MODE)) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	uint32_t rows = s_currIdx ? 9 : 0 ;
	
//#if MULTI_GVARS
//	if ( g_model.flightModeGvars )
//	{
//		rows += 24 ;
//	}
//#endif
	mstate2.check_columns(event, rows-1 ) ;
  PUTC( 12*FW, 0, '0'+s_currIdx ) ;

  uint32_t sub = mstate2.m_posVert ;
	uint16_t colour = dimBackColour() ;

	t_pgOfs = evalHresOffset( sub ) ;
#ifndef TOUCH
	(void) t_pgOfs ;
#endif	 
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;

	if ( rows > 9 )
	{
		newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
		if ( newVpos != t_pgOfs )
		{
			s_pgOfs = t_pgOfs = newVpos ;
			if ( sub < t_pgOfs )
			{
				mstate2.m_posVert = sub = t_pgOfs ;
			}
			else if ( sub > t_pgOfs + TLINES - 1 )
			{
				mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
			}
		}
	}
#endif
	 
	lcd_hline( 0, TTOP, TRIGHT ) ;
	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;

	uint32_t lines = rows > TLINES ? TLINES : rows ;
	for (uint32_t i = 0 ; i < lines ; i += 1 )
	{
    coord_t y = i * TFH + TTOP ;
    uint32_t k = i+t_pgOfs ;
    
		uint32_t attr = (sub==k) ? INVERS : 0 ;

    if ( s_currIdx == 0 )
		{
			k += 9 ;
		}

		switch(k)
		{
      case 0 : // switch
			{	
				drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
				LcdBackground = attr ? ~colour : colour ;
				if ( attr )
				{
					LcdForeground = ~LcdForeground ;
				}
				phase->swtch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, phase->swtch, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				LcdBackground = oldBcolour ;
				LcdForeground = oldFcolour ;
			}
			break ;
      case 1 : // switch
			{	
				drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
				LcdBackground = attr ? ~colour : colour ;
				if ( attr )
				{
					LcdForeground = ~LcdForeground ;
				}
				phase->swtch2 = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, phase->swtch2, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				LcdBackground = oldBcolour ;
				LcdForeground = oldFcolour ;
			}
			break ;
      
// Trim values -500 to 500 = own Trim
// 501 to 507 from other mode
// 501 is mode 0, 502 is mode 1 unless his mode is 1 when it is mode 2
			
			case 2 : // trim
      case 3 : // trim
      case 4 : // trim
      case 5 : // trim
			{	
				switch(k)
				{
      		case 2 : // trim
						drawItem( (char *)XPSTR("Rudder Trim"), y, attr ) ;
					break ;
      		case 3 : // trim
						drawItem( (char *)XPSTR("Elevator Trim"), y, attr ) ;
					break ;
      		case 4 : // trim
						drawItem( (char *)XPSTR("Throttle Trim"), y, attr ) ;
					break ;
      		case 5 : // trim
						drawItem( (char *)XPSTR("Aileron Trim"), y, attr ) ;
					break ;
				}
				LcdBackground = attr ? ~colour : colour ;
				if ( attr )
				{
					if ( ! ( s_editMode && BLINK_ON_PHASE ) )
					{
//						fcolour = ~LcdForeground ;
						LcdForeground = ~LcdForeground ;
					}
				}
        putsTrimMode( TRIGHT-2*FW, y+TVOFF, s_currIdx, k-2, 0 ) ;
				t_trim v = phase->trim[k-2] ;
				int16_t w = v.mode ;
  			if (w > 0)
				{
	  			if (w & 1)
					{
	        	PUTC( TRIGHT-3*FW, y+TVOFF, '+' ) ;
					}
				}
				LcdBackground = oldBcolour ;
				LcdForeground = oldFcolour ;
        if (attr )
				{
extern t_trim rawTrimFix( uint8_t phase, t_trim v ) ;
					v = rawTrimFix( s_currIdx, v ) ;
					w = v.mode ;
					int16_t u = w ;
          w = checkIncDec16( w, 0, 15, EE_MODEL ) ;
					if (w != u)
					{
						if ( w & 1 )
						{
							if ( (w>>1) == s_currIdx )
							{
								if ( w > u )
								{
									w += 1 ;
									if ( s_currIdx == 7 )
									{
										if ( w > 14 )
										{
											w = 14 ;
										}
									}
								}
								else
								{
									w -= 1 ;
								}
							}
						}
						v.mode = w ;
						// reverse xyzzy!!
						if ( w & 1 )
						{
							// additive
							v.value = 0 ;
						}
						else
						{
							w >>= 1 ;
							if ( w == s_currIdx )
							{
								v.value = 0 ;
							}
							else
							{
								if ( w > s_currIdx )
								{
									w -= 1 ;
								}
								v.value = TRIM_EXTENDED_MAX + w + 1 ;
							}
						}
  					phase->trim[k-2] = v ;
          }
//			drawNumber( TRIGHT-TRMARGIN+5*FW, y, u, 0 ) ; //, attr ? ~colour : colour ) ;
//			drawNumber( TRIGHT-TRMARGIN+5*FW, 0, s_currIdx, 0 ) ; //, attr ? ~colour : colour ) ;
        }
//				drawNumber( TRIGHT-TRMARGIN-7*FW, y, phase->trim[k-2].value, 0 ) ; //, attr ? ~colour : colour ) ;
//				drawNumber( TRIGHT-TRMARGIN-11*FW, y, phase->trim[k-2].mode, 0 ) ; //, attr ? ~colour : colour ) ;
			}
			break ;
			case 6 : // fadeIn
				drawItem( (char *)XPSTR("Fade In"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, phase->fadeIn * 5, attr|PREC1) ; //, attr ? ~colour : colour ) ;
  			if( attr )
				{
					CHECK_INCDEC_H_MODELVAR( phase->fadeIn, 0, 15 ) ;
				}
			break ;
      
			case 7 : // fadeOut
				drawItem( (char *)XPSTR("Fade Out"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, phase->fadeOut * 5, attr|PREC1) ; //, attr ? ~colour : colour ) ;
  			if( attr )
				{
					CHECK_INCDEC_H_MODELVAR( phase->fadeOut, 0, 15 ) ;
				}
			break ;
			
			case 8 : // Name
			{	
				drawItem( (char *)XPSTR("Mode Name"), y, attr ) ;
				LcdBackground = attr ? ~colour : colour ;
				alphaEditName( TRIGHT-sizeof(phase->name)*FW-FW, y+TVOFF, (uint8_t *)phase->name, sizeof(phase->name), attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
				LcdBackground = oldBcolour ;
			}
			break ;
			
//#if MULTI_GVARS
//			default :
//			{
//				uint8_t text[4] ;
//				text[0] = 'G' ;
//				text[1] = 'V' ;
//				text[3] = 0 ;
//				k -= 9 ;
//				if ( ( k & 1) == 0 )
//				{
//					k /= 2 ;
//	        int16_t v ;
//					text[2] = ((k >=9)? '8' : '1') + k ;
//					drawItem( (char *)text, y, attr ) ;
//					if ( g_model.gvarNames[k*3] )
//					{
//						if ( g_model.gvarNames[k*3] != ' ' )
//						{
//							uint8_t text[6] ;
//  						ncpystr(&text[1], &g_model.gvarNames[k*3], 3 ) ;
//			  			text[0] = '(' ;
//			  			text[4] = ')' ;
//			  			text[5] = 0 ;
//							PUTS_P( 4*FW, y+TVOFF, (char *)text ) ;
//						}
//					}
//  	      v = readMgvar( s_currIdx, k ) ;
//	        if ( (v > GVAR_MAX) && ( s_currIdx ) )
//					{
//    	      uint32_t p = v - GVAR_MAX - 1 ;
//      	    if (p >= s_currIdx)
//						{
//							p += 1 ;
//						}
//						text[0] = 'F' ;
//						text[1] = 'M' ;
//						text[2] = '0'+p ;
//					}
//					else
//					{
//						text[0] = 'O' ;
//						text[1] = 'W' ;
//						text[2] = 'N' ;
//					}
//					drawText( TRIGHT-TRMARGIN-3*FW, y, (char *)text, attr ) ;

//					if ( attr && s_currIdx )
//					{
////						int16_t oldValue = v ;
//          	if (v < GVAR_MAX)
//						{
//							v = GVAR_MAX ;
//						}
//						int16_t entryValue = v ;
//          	v = checkIncDec16( v, GVAR_MAX, GVAR_MAX+MAX_MODES+1, EE_MODEL ) ;
//          	if ( v != entryValue )
//						{
//	          	if (v == GVAR_MAX)
//							{
//								v = 0 ;
//							}
////							if ( v != oldValue )
//							{
//								writeMgvar( s_currIdx, k, v ) ;
//							}
//						}
//					}
//				}
//				else
//				{
//					k /= 2 ;
//					drawItem( (char *)XPSTR(" Value"), y, attr ) ;
//					uint32_t fm = getGVarFlightMode( s_currIdx+1, k ) ;
//	        int16_t w = readMgvar( fm, k ) ;
//					drawNumber( TRIGHT-TRMARGIN, y, w, attr ) ; //, attr ? ~colour : colour ) ;
//					if ( attr )
//					{
//						w = checkIncDec16( w, -125, 125, EE_MODEL ) ;
//						writeMgvar( fm, k, w ) ;
//					}
//				}
//			}
//			break ;
//#endif
		}
	}
}











void editTimer( uint8_t sub, uint8_t event )
{
	uint8_t subN ;
	uint8_t timer ;
	uint16_t colour = dimBackColour() ;
#ifdef HAPTIC
	if ( sub < 8 )
#else
	if ( sub < 7 )
#endif
	{
		subN = 0 ;
		timer = 0 ;
	}
	else
	{
#ifdef HAPTIC
		subN = 8 ;
#else
		subN = 7 ;
#endif
		timer = 1 ;
	}
	t_TimerMode *ptm ;
	
	ptm = &g_model.timer[timer] ;

	uint16_t y = TTOP ;

	drawItem( XPSTR("Time"), y, (sub == subN ) ) ;
	saveEditColours( sub == subN, colour ) ;
 	putsTime( TRIGHT-TRMARGIN-2*FW, y+TVOFF, ptm->tmrVal, 0, 0 ) ;
	restoreEditColours() ;

	if(sub==subN)
	{
		int16_t temp = 0 ;
		CHECK_INCDEC_H_MODELVAR( temp, -60 ,60 ) ;
		ptm->tmrVal += temp ;
    if((int16_t)ptm->tmrVal < 0) ptm->tmrVal=0 ;				
	}
	y += TFH ;
	subN += 1 ;

	drawItem( (char *)PSTR(STR_TRIGGERA), y, (sub == subN ) ) ;
	uint8_t attr = 0 ;
  if(sub==subN)
	{
   	attr = INVERS ;
		CHECK_INCDEC_H_MODELVAR_0( ptm->tmrModeA, 1+2+24+8 ) ;
	}
	saveEditColours( sub == subN, colour ) ;
  putsTmrMode(TRIGHT-TRMARGIN-4*FW,y+TVOFF,0, timer, 1 ) ;
	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;
  	
	drawItem( (char *)PSTR(STR_TRIGGERB), y, (sub == subN ) ) ;
  attr = 0 ;
  if(sub==subN)
	{
   	attr = INVERS ;
	}
	uint8_t doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
	saveEditColours( attr, colour ) ;
	ptm->tmrModeB = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, ptm->tmrModeB, LUA_RIGHT, doedit, event ) ;
	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;
	 
	drawItem( (char *)PSTR(STR_TIMER), y, (sub == subN ) ) ;
  attr = 0 ;
  if(sub==subN)
	{
		attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( ptm->tmrDir,1) ;
	}
	drawIdxText( y, (char *)PSTR(STR_COUNT_DOWN_UP), ptm->tmrDir, attr|LUA_RIGHT ) ;
//	saveEditColours( sub == subN, colour ) ;
//  PUTS_AT_IDX( TRIGHT-TRMARGIN-10*FW, y+TVOFF, PSTR(STR_COUNT_DOWN_UP), ptm->tmrDir, 0 ) ;
//	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;

	uint8_t b = timer ? g_model.timer2Mbeep : g_model.timer1Mbeep ;
  (timer ? g_model.timer2Mbeep : g_model.timer1Mbeep) = touchOnOffItem( b, y, PSTR(STR_MINUTE_BEEP), (sub == subN ), colour ) ;
	y += TFH ;
	subN += 1 ;

	b = timer ? g_model.timer2Cdown : g_model.timer1Cdown ;
  (timer ? g_model.timer2Cdown : g_model.timer1Cdown) = touchOnOffItem( b, y, PSTR(STR_BEEP_COUNTDOWN), (sub == subN ), colour ) ;
	y += TFH ;
	subN += 1 ;

	drawItem( (char *)PSTR(STR_RESET_SWITCH), y, (sub == subN ) ) ;
  attr = 0 ;
	if(sub==subN)
	{
   	attr = INVERS ;
	}

	int16_t sw = (timer==0) ? g_model.timer1RstSw : g_model.timer2RstSw ;
	doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
	saveEditColours( attr, colour ) ;
	sw = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, sw, 0, doedit, event ) ;
	restoreEditColours() ;
	if ( timer == 0 )
	{
		g_model.timer1RstSw = sw ;
	}
	else
	{
		g_model.timer2RstSw = sw ;
	}
	y += TFH ;
	subN += 1 ;
  	  
	drawItem( XPSTR("Haptic"), y, (sub == subN ) ) ;
  attr = 0 ;
	if(sub==subN)
	{
		attr = INVERS ;
	}
	sw = (timer==0) ? g_model.timer1Haptic : g_model.timer2Haptic ;
	drawIdxText( y, XPSTR("\006  NoneMinute Cdown  Both"), sw, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
	if( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( sw, 3 ) ;
	}
	if ( timer == 0 )
	{
		g_model.timer1Haptic = sw ;
	}
	else
	{
		g_model.timer2Haptic = sw ;
	}
}


void menuTimers(uint8_t event)
{
	static MState2 mstate2 ;
	uint8_t subN = 0 ;
	uint32_t t ;
	uint32_t secs ;
	div_t qr ;
  TITLE( PSTR( STR_TIMER ) ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	uint16_t y = TTOP ;
#ifdef HAPTIC
	uint32_t rows = 19 ;
#else
	uint32_t rows = 17 ;
#endif
	
	mstate2.check_columns(event, rows-1 ) ;
	uint8_t sub = mstate2.m_posVert ;

#ifdef TOUCH
	if ( TouchUpdated )
	{
		if ( TouchControl.event == TEVT_DOWN )
		{
				
		}
		else if ( TouchControl.event == TEVT_UP )
		{
			uint32_t lines ;
#ifdef HAPTIC
			if ( sub < 16 )
#else
			if ( sub < 14 )
#endif
			{
				lines = 8 ;
			}
			else
			{
				lines = 3 ;
			}
			if ( ( TouchControl.x <= TRIGHT*TSCALE ) && (TouchControl.x >= TMID*TSCALE) && !s_editMode )
			{
				uint32_t vert = TouchControl.y ;
				vert -= TTOP*TSCALE ;
				vert /= TFH*TSCALE ;
				if ( vert < lines )
				{
					if ( sub < 16 )
					{
						if ( sub >= 8 )
						{
							vert += 8 ;
						}						
					}
					else
					{
						vert += 16 ;
					}
					TouchControl.itemSelected = vert+1 ;
					TouchUpdated = 0 ;
					sub = mstate2.m_posVert = vert ;
				}
			}
		}
	}
#endif

#ifdef HAPTIC
	if ( sub < 16 )
#else
	if ( sub < 14 )
#endif
	{
		PUTS_P( 20*FW-4, 7*FH, XPSTR("->") ) ;
#ifdef HAPTIC
		PUTC_ATT( 10*FW, 0, ( sub < 8 ) ? '1' : '2', BLINK ) ;
#else
		PUTC_ATT( 10*FW, 0, ( sub < 7 ) ? '1' : '2', BLINK ) ;
#endif
		lcd_hline( 0, TTOP, TRIGHT ) ;
		editTimer( sub, event ) ;
	}
	else
	{
#ifdef HAPTIC
		subN = 16 ;
#else
		subN = 14 ;
#endif
		lcd_hline( 0, TTOP, TRIGHT ) ;
		uint16_t oldBcolour = LcdBackground ;
		uint16_t oldFcolour = LcdForeground ;
		uint16_t colour = dimBackColour() ;
		
		drawItem( (char *)PSTR( STR_TOTAL_TIME ), y, (sub == subN ) ) ;
		LcdBackground = ( sub == subN ) ? ~colour : colour ;
		if (sub == subN )
		{
			LcdForeground = ~LcdForeground ;
		}
		t = g_model.totalTime ;
		secs = t % 60 ;
		t = t / 60 ;	// minutes
		qr = div( t, 60 ) ;
		PUTS_NUM_N( TRIGHT-TRMARGIN, y+TVOFF, secs, LEADING0, 2 ) ;
		PUTC_ATT( TRIGHT-3*FW-2, y+TVOFF, ':', 0 ) ;
		PUTS_NUM_N( TRIGHT-3*FW-2, y+TVOFF, (uint16_t)qr.rem, LEADING0, 2 ) ;
		PUTC_ATT( TRIGHT-6*FW+3-2, y+TVOFF, ':', 0 ) ;
		PUTS_NUM( TRIGHT-6*FW+3-2, y+TVOFF, (uint16_t)qr.quot, 0 ) ;
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;

    if(sub==subN)
		{
			if (event == EVT_KEY_LONG(KEY_MENU) )
			{
				g_model.totalTime = 0 ;
    		killEvents( event ) ;
			}
		}
		y += TFH ;
		subN += 1 ;
		g_model.t1AutoReset = touchOnOffItem( g_model.t1AutoReset, y, XPSTR("T1 Auto Reset"), (sub == subN ), colour ) ;
		y += TFH ;
		subN += 1 ;
		g_model.t2AutoReset = touchOnOffItem( g_model.t2AutoReset, y, XPSTR("T2 Auto Reset"), (sub == subN ), colour ) ;
	}
}

void menuProcVoiceOne(uint8_t event)
{
	TITLE( PSTR( STR_Voice_Alarm ) ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2;
	uint32_t rows = 13 ;
#ifdef TOUCH
	uint32_t newVpos ;
#endif

	VoiceAlarmData *pvad ;
  if ( s_currIdx >= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
	{
		EditType = EE_GENERAL ;
		uint8_t z = s_currIdx - ( NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ) ;
		pvad = &g_eeGeneral.gvad[z] ;
		PUTC( 16*FW, 0, 'G' ) ;
		PUTS_NUM( 18*FW-1, 0, z+1, 0 ) ;
	}
	else
	{
		pvad = (s_currIdx >= NUM_VOICE_ALARMS) ? &g_model.vadx[s_currIdx - NUM_VOICE_ALARMS] : &g_model.vad[s_currIdx] ;
		PUTS_NUM( 17*FW, 0, s_currIdx+1, 0 ) ;
	}
	if ( pvad->fnameType )
	{
		rows += 1 ;
	}
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	event = mstate2.check_columns( event, rows-1 ) ;
	uint8_t sub = mstate2.m_posVert ;
	
	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	else if ( event == EVT_ENTRY_UP )
	{
		if ( sub == 0 )
		{
			// Returned from editing
			if ( TextResult )
			{
				pvad->source = unmapPots( TextIndex ) ;
	  	  eeDirty(EE_MODEL) ;
			}
		}
		else
		{
		 	// From menuProcSelectUvoiceFile
			if ( FileSelectResult == 1 )
			{
		 		copyFileName( (char *)pvad->file.name, SelectedVoiceFileName, 8 ) ;
	   		eeDirty(EE_MODEL) ;		// Save it
			}
		}
	}

	uint16_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	
	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	uint16_t colour = dimBackColour() ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
	
  for( uint32_t i = 0 ; i < TLINES ; i += 1 )
  {
		uint8_t j = i + t_pgOfs ;
    uint16_t y = i * TFH + TTOP ;
    uint8_t attr = (sub==j) ? INVERS : 0 ;

#define V1P1		6

		if ( pvad->fnameType == 0 )
		{
			if ( j >= V1P1+4 )
			{
				j += 1 ;
			}
		}
	    switch(j)
			{
    	  case 0 :	// source
					drawItem( (char *)XPSTR("Source"), y, attr ) ;
					saveEditColours( attr, colour ) ;
					putsChnRaw( (TRIGHT-TRMARGIN)*2, (y)*2+TVOFF, pvad->source, LUA_RIGHT ) ;
					restoreEditColours() ;
					if ( attr )
					{
						uint8_t x = mapPots( pvad->source ) ;
						x = checkIncDec16( x,0,NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots-1+1,EditType);
						pvad->source = unmapPots( x ) ;
#ifdef TOUCH
						if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) || handleSelectIcon() )
#else
						if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#endif
						{
							// Long MENU pressed
							TextIndex = mapPots( pvad->source ) ;
  					  TextType = TEXT_TYPE_SW_SOURCE ;
  					  killEvents(event) ;
							pushMenu(menuTextHelp) ;
						}
					}
					if ( pvad->source )
					{
						int16_t value ;
						value = getValue( pvad->source - 1 ) ;
  	  			PUTS_ATT_LEFT( y+TVOFF, XPSTR("\007(\016)") ) ;
    				if ( ( (pvad->source > CHOUT_BASE+NUM_SKYCHNOUT) && ( pvad->source < EXTRA_POTS_START ) ) || ( pvad->source >= EXTRA_POTS_START + 8) )
 						{
							putsTelemetryChannel( 13*FW, y+TVOFF, pvad->source-CHOUT_BASE-NUM_SKYCHNOUT-1, value, LUA_RIGHT, TELEM_NOTIME_UNIT | TELEM_UNIT ) ;
						}
						else
						{
							PUTS_NUM( 13*FW, y+TVOFF, value, 0 ) ;
						}
					}
				break ;

				case 1 :	// func;
					drawItem( (char *)XPSTR("Function"), y, attr ) ;
//					saveEditColours( attr, colour ) ;
					drawIdxText( y, XPSTR("\007-------v>val  v<val  |v|>val|v|<valv\140=val v=val  v & val|d|>valv%val=0d>=val "), pvad->func, attr|LUA_RIGHT ) ;	// v1>v2  v1<v2  
//					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->func, 10 ) ;
					}	
				break ;

				case 2 :
				{	
					drawItem( (char *)XPSTR("Value"), y, attr ) ;
    			if ( ( (pvad->source > CHOUT_BASE+NUM_SKYCHNOUT) && ( pvad->source < EXTRA_POTS_START ) ) || ( pvad->source >= EXTRA_POTS_START + 8) )
		 			{
						if ( attr & INVERS )
						{
							if ( s_editMode && BLINK_ON_PHASE )
							{
								attr = 0 ;
							}
						}
						saveEditColours( attr, colour ) ;
						putsTelemetryChannel( TRIGHT-TRMARGIN-FW, y+TVOFF, pvad->source-CHOUT_BASE-NUM_SKYCHNOUT-1, pvad->offset, 0, TELEM_NOTIME_UNIT | TELEM_UNIT | TELEM_CONSTANT ) ;
						restoreEditColours() ;
					}
					else
					{
//						PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF, pvad->offset, 0 ) ;
	  				drawNumber(TRIGHT-TRMARGIN,y, pvad->offset, attr ) ;
					}
					if ( attr )
					{
						pvad->offset = checkIncDec16( pvad->offset, -32000, 32000, EE_MODEL ) ;
					}
				}
				break ;
			
				case 3 :	 // swtch ;
					drawItem( (char *)XPSTR("Switch"), y, attr ) ;
//					saveEditColours( attr, colour ) ;
   	  		DrawDrSwitches(TRIGHT-TRMARGIN, y, pvad->swtch, attr|LUA_RIGHT ) ;
//					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_MODELSWITCH( pvad->swtch, -MaxSwitchIndex, MaxSwitchIndex+1 ) ;
    			}
				break ;

				case 4 :	 // rate ;
					drawItem( (char *)XPSTR("Trigger"), y, attr ) ;
					if ( attr & INVERS )
					{
						if ( s_editMode && BLINK_ON_PHASE )
						{
							attr = 0 ;
						}
					}
					saveEditColours( attr, colour ) ;
					displayVoiceRate( TRIGHT-TRMARGIN, y+TVOFF, pvad->rate, LUA_RIGHT ) ;
					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->rate, 33 ) ;
					}	
				break ;

				case 5 :	 // haptic ;
					drawItem( (char *)PSTR( STR_HAPTIC ), y, attr ) ;
//					saveEditColours( attr, colour ) ;
					drawIdxText( y, XPSTR("\007-------Haptic1Haptic2Haptic3"), pvad->haptic, attr|LUA_RIGHT ) ;
//					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->haptic, 3 ) ;
					}
				break ;

				case V1P1 :	 // delay
					drawItem( (char *)XPSTR("On Delay"), y, attr ) ;
//					saveEditColours( attr, colour ) ;
  				drawNumber(TRIGHT-TRMARGIN,y, pvad->delay, attr|PREC1 ) ;
//					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->delay, 50 ) ;
					}
				break ;
				
				case V1P1+1 :	 // vsource:2
					drawItem( (char *)XPSTR("Play Source"), y, attr) ;
//					saveEditColours( attr, colour ) ;
					drawIdxText( y, XPSTR("\006    NoBefore After"), pvad->vsource, attr|LUA_RIGHT ) ;
//					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->vsource, 2 ) ;
					}
				break ;

				case V1P1+2 :
					drawItem( (char *)XPSTR("On no Telemetry"), y, attr ) ;
//					saveEditColours( attr, colour ) ;
					drawIdxText( y, XPSTR("\004PlayMute"), pvad->mute, attr|LUA_RIGHT ) ;
//					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->mute, 1 ) ;
					}
				break ;

				case V1P1+3 :	 // fnameType:3 ;
				{	
					drawItem( (char *)XPSTR("FileType"), y, attr ) ;
//					saveEditColours( attr, colour ) ;
					drawIdxText( y, XPSTR("\007-------   NameGV/SC/# EffectSysName"),pvad->fnameType, attr|LUA_RIGHT ) ;
//					restoreEditColours() ;
					uint8_t previous = pvad->fnameType ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->fnameType, 4 ) ;
					}
					if ( pvad->fnameType != previous )
					{
						if ( pvad->fnameType )
						{
							pvad->file.name[0] = 0 ;
							pvad->file.name[1] = 0 ;
						}
						else
						{
							pvad->file.vfile = 0 ;
						}
					}
				}
				break ;
			
				case V1P1+4 :	 // filename ;
					drawItem( (char *)(pvad->fnameType == 3 ? XPSTR("Sound effect") : XPSTR("Voice File")), y, attr ) ;
					if ( ( pvad->fnameType == 1 ) || ( pvad->fnameType == 4 ) )	// Name
					{
						if ( pvad->file.name[0] == 0 )
						{
							memset( (uint8_t *)pvad->file.name, ' ', sizeof(pvad->file.name) ) ;
						}
						// Need to edit name here
						uint16_t oldBcolour = LcdBackground ;
						LcdBackground = attr ? ~colour : colour ;
						alphaEditName( TRIGHT-TRMARGIN-8*FW, y+TVOFF, (uint8_t *)pvad->file.name, sizeof(pvad->file.name), attr | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FileName") ) ;
						LcdBackground = oldBcolour ;
						validateName( pvad->file.name, sizeof(pvad->file.name) ) ;
	  				if( attr )
						{
							lcd_rect( TRIGHT-TRMARGIN-8*FW-1, y+TVOFF-1, sizeof(pvad->file.name)*FW+2, 9 ) ;
#ifdef TOUCH
							if ( event==EVT_KEY_LONG(BTN_RE) || handleSelectIcon() )
#else
							if ( event==EVT_KEY_LONG(BTN_RE) )
#endif
							{
								VoiceFileType = ( pvad->fnameType == 4 ) ? VOICE_FILE_TYPE_SYSTEM : VOICE_FILE_TYPE_USER ;
      				 	pushMenu( menuProcSelectVoiceFile ) ;
							}
						} 
					}
					else if ( pvad->fnameType == 2 )	// Number
					{
						uint16_t value = pvad->file.vfile ;
						if ( value )
						{
							if ( value > 500 )
							{
								value -= 500 ;
							}
							else
							{
								value += 15 ;
							}
						}
	  	  		if(attr)
						{
	  	    		value = checkIncDec16( value, 0, 515, EE_MODEL);
						}
						if ( value )
						{
							if ( value > 15 )
							{
								value -= 15 ;
							}
							else
							{
								value += 500 ;
							}
						}
						pvad->file.vfile = value ;
						if ( pvad->file.vfile > 500)
						{
							if ( pvad->file.vfile > 507 )
							{
								PUTS_ATT( TRIGHT-TRMARGIN-5*FW, y+TVOFF, XPSTR("SC"), attr ) ;
								PUTC_ATT( TRIGHT-TRMARGIN-3*FW, y+TVOFF, pvad->file.vfile - 507 + '0', attr ) ;
							}
							else
							{
								dispGvar( TRIGHT-TRMARGIN-5*FW, y+TVOFF, pvad->file.vfile - 500, attr ) ;
							}
  					}
  					else
						{
	      	    PUTS_NUM( TRIGHT-TRMARGIN-3*FW, y+TVOFF, pvad->file.vfile, attr ) ;
  					}
						if (attr)
						{
							if ( checkForMenuEncoderLong( event ) )
							{
								if ( pvad->file.vfile <= 500)
								{
									putVoiceQueue( pvad->file.vfile | VLOC_NUMUSER  ) ;
								}
							}
						}
					}
					else if ( pvad->fnameType == 3 )	// Audio
					{
						drawIdxText( y, (char *)PSTR(STR_SOUNDS), pvad->file.vfile, attr ) ;
		  	  	if(attr)
						{
							CHECK_INCDEC_H_MODELVAR( pvad->file.vfile, 0, 16 ) ;
						}
					}
				break ;
				
				case V1P1+5 :	 // Blank ;
					drawItem( (char *)XPSTR("Delete"), y, attr ) ;
//					saveEditColours( attr, colour ) ;
					PUTS_ATT( TRIGHT-TRMARGIN-9*FW, y+TVOFF, XPSTR("MENU LONG"), 0 ) ;
//					restoreEditColours() ;
  				if( attr )
					{
						if ( checkForMenuEncoderLong( event ) )
						{
				     	deleteVoice( pvad ) ;
//							if ( i == 10 )
//							{
								mstate2.m_posVert = 0 ;
//							}
	    				eeDirty(EE_MODEL) ;
						}
					}
				break ;
			
				case V1P1+6 :	 // Copy ;
					drawItem( (char *)XPSTR("Copy"), y, attr ) ;
	    		if(attr)
					{
						lcd_char_inverse( 0, y+TVOFF, 30, 0 ) ;
						if ( checkForMenuEncoderLong( event ) )
						{
							Clipboard.clipvoice = *pvad ;
							Clipboard.content = CLIP_VOICE ;
						}
					}
				break ;

				case V1P1+7 :	 // Paste ;
					drawItem( (char *)XPSTR("Paste"), y, attr ) ;
					if ( Clipboard.content == CLIP_VOICE )
					{
						putTick( (TRIGHT-TRMARGIN-FW), y+3, attr ? ~LcdForeground : LcdForeground ) ;
//						lcd_putcAttColour( TRIGHT-FW-5, y+3, '\202', 0, LcdForeground, colour ) ;
					}	
	    		if(attr)
					{
						lcd_char_inverse( 0, y+TVOFF, 36, 0 ) ;
						if ( checkForMenuEncoderLong( event ) )
						{
							if ( Clipboard.content == CLIP_VOICE )
							{							
								*pvad = Clipboard.clipvoice ;
								MuteTimer = 5 ;
							}
						}
					}
				break ;
		}
	}
}


void menuVoice(uint8_t event, uint8_t mode)
{
	uint32_t rows = mode ? NUM_GLOBAL_VOICE_ALARMS : NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS + 1 + 1 ;
	TITLE(PSTR(STR_Voice_Alarms)) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2 ;
	int8_t sub ;
	uint8_t t_pgOfs ;
	uint16_t y ;
#ifdef TOUCH
	uint32_t selected = 0 ;
#endif

#ifdef TOUCH
	if ( ( event == EVT_ENTRY ) || ( event == EVT_ENTRY_UP ) )
	{
		LastTaction = 2 ;
		LastTselection = mstate2.m_posVert ;
	}
#endif
	 
#ifdef MOVE_VOICE
	if ( s_moveMode )
	{
		int8_t moveByRotary ;
		moveByRotary = qRotary() ;		// Do this now, check_simple destroys rotary data
		if ( moveByRotary )
		{
			if ( moveByRotary > 0 )
			{
				event = EVT_KEY_FIRST(KEY_DOWN) ;
			}
			else
			{
				event = EVT_KEY_FIRST(KEY_UP) ;
			}
		}
		uint8_t v = mstate2.m_posVert ;
		if ( ( ( v == 0 ) && ( event == EVT_KEY_FIRST(KEY_UP) ) ) 
				 || ( ( v == (mode ? NUM_GLOBAL_VOICE_ALARMS - 1 : NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS - 1 ) ) && ( event == EVT_KEY_FIRST(KEY_DOWN) ) ) )
		{
			event = 0 ;
		}
		Tevent = event ;
	}
	
	lcd_hline( 0, TTOP, TRIGHT ) ;

	if ( !PopupData.PopupActive )
	{
#ifdef TOUCH
		uint32_t newVpos ;
#endif
		
#ifndef TOUCH
		event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
		mstate2.check_columns(event, rows - 1 ) ;
		
  	
		sub = mstate2.m_posVert ;

		t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
		int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
		uint16_t newVert = processSelection( sub , newSelection ) ;
		sub = mstate2.m_posVert = newVert & 0x00FF ;
		selected = newVert & 0x0100 ;
		 
		if ( handleSelectIcon() || selected )
		{
			if ( event == 0 )
			{
				event = EVT_KEY_BREAK(BTN_RE) ;
			}
		}

		if ( rows > TLINES )
		{
			newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
			if ( newVpos != t_pgOfs )
			{
				s_pgOfs = t_pgOfs = newVpos ;
				if ( sub < t_pgOfs )
				{
					mstate2.m_posVert = sub = t_pgOfs ;
				}
				else if ( sub >= t_pgOfs + TLINES - 1 )
				{
					mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
				}
			}
		}
#endif
	}
	else
	{
	  sub = mstate2.m_posVert ;
		t_pgOfs = evalHresOffset( sub ) ;
	}
	
#else	
	mstate2.check_columns(event, rows - 1 ) ;
#endif

  switch (event)
	{
#ifdef MOVE_VOICE
	    case EVT_ENTRY_UP:
	    case EVT_ENTRY:
        s_moveMode = false ;
  	  break;
#endif
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE) :
	    if(sub == NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
			{
    	  pushMenu(menuProcGlobalVoiceAlarm) ;
	 	    killEvents(event);
			}
			else if( sub < NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
			{
	      s_currIdx = sub + (mode ? NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS : 0) ;
#ifdef MOVE_VOICE
				s_curItemIdx = s_currIdx ;
				if ( s_moveMode )
				{
	  	  	s_moveMode = false ;
  	  		s_editMode = false ;
					RotaryState = ROTARY_MENU_UD ;
		 	    killEvents(event);
  	  		break;
				}
				// Else fall through    
				if ( !PopupData.PopupActive )
				{
					PopupData.PopupIdx = 0 ;
					PopupData.PopupActive = 1 ;
 			    killEvents(event);
					event = 0 ;		// Kill this off
				}
#else
    	  pushMenu(menuProcVoiceOne) ;
	 	    killEvents(event);
#endif
			}
		break;
  }

#ifdef MOVE_VOICE
	if ( s_moveMode )
	{
		int8_t dir ;
		uint8_t xevent = event ;
		if ( event == EVT_KEY_REPT(KEY_DOWN) )
		{
			xevent = EVT_KEY_FIRST(KEY_DOWN) ;
		}
		if ( event == EVT_KEY_REPT(KEY_UP) )
		{
			xevent = EVT_KEY_FIRST(KEY_UP) ;
		}
		
		if ( ( dir = (xevent == EVT_KEY_FIRST(KEY_DOWN) ) ) || xevent == EVT_KEY_FIRST(KEY_UP) )
		{
			moveVoice( s_curItemIdx, dir, mode ) ; //true=inc=down false=dec=up - Issue 49
			if ( mode == 0 )
			{
				if ( sub >= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
				{
					sub = mstate2.m_posVert = NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS - 1 ;
				}
			}
		}
	}
#endif

	uint8_t k = 0 ;
	for( uint32_t i = 0 ; i < ( mode ? 8 : TLINES) ; i += 1 )
	{
    y=(i)*TFH +TTOP+TVOFF ;
    k = i + t_pgOfs ;
    uint8_t attr = sub == k ? INVERS : 0 ;
		VoiceAlarmData *pvad ;
      
//		if(y>7*FH) break ;

		uint16_t oldBcolour = LcdBackground ;
		uint16_t oldFcolour = LcdForeground ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, (y-TVOFF)*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}
	  if ( k < NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
		{
    	if ( mode )
			{
				PUTS_P( THOFF/2, y, XPSTR("GVA") ) ;
				PUTC( 3+3*FW, y, '1' + k ) ;
				pvad = &g_eeGeneral.gvad[k] ;
			}
			else
			{
				PUTS_P( THOFF/2, y, XPSTR("VA") ) ;
  		  PUTS_NUM( (k<9) ? 3+FW*3-1 : 3+FW*4-2, y, k+1, 0 ) ;
				pvad = (k >= NUM_VOICE_ALARMS) ? &g_model.vadx[k - NUM_VOICE_ALARMS] : &g_model.vad[k] ;
			}
			putsChnRaw( (5*FW)*2, (y)*2, pvad->source, 0 ) ;
			putsDrSwitches( 9*FW, y, pvad->swtch, 0 ) ;
			displayVoiceRate( 13*FW, y, pvad->rate, 0 ) ;
    	switch ( pvad->fnameType )
			{
				case 1 :
					PUTC( 19*FW, y, 'N' ) ;
				break ;
				case 2 :
					PUTC( 19*FW, y, '#' ) ;
				break ;
				case 3 :
					PUTC( 19*FW, y, 'A' ) ;
				break ;
				case 4 :
					PUTC( 19*FW, y, 'S' ) ;
				break ;
			}
		
			if (pvad->haptic)
			{
				PUTC( 20*FW, y, 'H' ) ;
			}
			if ( attr )
			{
#ifdef MOVE_VOICE
				if ( s_moveMode )
				{
					lcd_rect( 0, y, 127, 8 ) ;
				}
//				else
//				{
//					lcd_char_inverse( 0, y+TVOFF, 20*FW, 0 ) ;
//				}
#else
				lcd_char_inverse( 0, y, 20*FW, 0 ) ;
#endif
			}
		}
		else
		{
  		if( k == NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
			{
  		  //last line available - add the global voice alarms line
  		  PUTS_ATT( 0, y, XPSTR(GvaString), 0 ) ;
			}
  		else
			{
  		  PUTS_ATT_LEFT( y ,XPSTR("Flush Switch") ) ;
				g_model.voiceFlushSwitch = edit_dr_switch( 17*FW, y, g_model.voiceFlushSwitch, 0, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			}
		}
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y+TFH-TVOFF, TRIGHT ) ;
  }

#ifdef MOVE_VOICE
	s_curItemIdx = sub ;
	if ( PopupData.PopupActive )
	{
		Tevent = event ;
		voicepopup( event ) ;
    s_editMode = false;
	}
#endif
}






void menuProcLimits(uint8_t event)
{
	static MState2 mstate2 ;
//	lcd_putsAtt( 4*FW, 0, "Limits", INVERS ) ;
	lcdDrawText( 4*FW*HVSCALE, 0, "Limits", INVERS|BOLD ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	uint32_t rows = NUM_SKYCHNOUT+EXTRA_SKYCHANNELS + 1 ;
	
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif

	event = mstate2.check_columns(event, rows - 1) ;

#ifdef TOUCH
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif
	coord_t y = 0 ;
	uint32_t k = 0 ;
	uint32_t t_pgOfs ;

#ifdef TOUCH
	if ( ( event == EVT_ENTRY ) || ( event == EVT_ENTRY_UP ) )
	{
		LastTaction = 2 ;
		LastTselection = mstate2.m_posVert ;
	}
#endif
	 
	lcd_hline( 0, TTOP, TRIGHT ) ;

	uint32_t sub = mstate2.m_posVert ;

	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

	if ( sub < NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
	{
		lcdDrawNumber( 15*FW*2, 0, g_chans512[sub]/2 + 1500 ) ;
//    lcd_outdez( 15*FW, 0, g_chans512[sub]/2 + 1500 ) ;
	}

#ifdef TOUCH
	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, NUM_SKYCHNOUT+EXTRA_SKYCHANNELS+1-(TLINES)+1, t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}

	if ( handleSelectIcon() || selected )
	{
		s_currIdx = sub ;
		pushMenu(menuLimitsOne) ;
    s_editMode = false ;
	}
#endif

	switch(event)
	{
    case EVT_KEY_BREAK(BTN_RE):
			if(sub==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)
			{
  			//last line available - add the "copy trim menu" line
  			s_noHi = NO_HI_LEN;
  			killEvents(event);
  			s_editMode = 0 ;
  			setStickCenter(1); //if highlighted and menu pressed - copy trims
			}
			else
			{
				s_currIdx = sub ;
				pushMenu(menuLimitsOne) ;
  		  s_editMode = false ;
			}
	  break ;
	}

	for( uint32_t i = 0 ; i < TLINES ; i++ )
	{
    y=(i)*TFH +TTOP ;
    k=i+t_pgOfs ;
    
    uint8_t attr = (sub==k) ? INVERS : 0 ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
		}
		if(k==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)
		{
			break ;
		}
    
		LimitData *ld = &g_model.limitData[k];
		if ( k >= NUM_SKYCHNOUT )
		{
			ld = &g_model.elimitData[k-NUM_SKYCHNOUT] ;
		}
    putsChnColour( THOFF, y*2+TVOFF, k+1, attr ) ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;

		drawNumber( 12*FW, y, ld->offset, attr|PREC1) ; //, attr ? ~LcdBackground : LcdBackground ) ;
		int16_t value ;
		int16_t t = 0 ;
		if ( g_model.sub_trim_limit )
		{
			if ( ( t = ld->offset ) )
			{
				if ( t > g_model.sub_trim_limit )
				{
					t = g_model.sub_trim_limit ;
				}
				else if ( t < -g_model.sub_trim_limit )
				{
					t = -g_model.sub_trim_limit ;
				}
			}
		}
		value = t / 10 ;
		value += (int8_t)(ld->min-100) ;
		if ( value < -125 )
		{
			value = -125 ;						
		}
		drawNumber( 18*FW, y, value, attr) ; //, attr ? ~LcdBackground : LcdBackground ) ;
		value = t / 10 ;
		value += (int8_t)(ld->max+100) ;
		if ( value > 125 )
		{
			value = 125 ;						
		}
		drawNumber( 24*FW, y, value, attr) ; //, attr ? ~LcdBackground : LcdBackground ) ;
//		lcd_putsAttIdxColour( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR( STR_HYPH_INV)+2, ld->revert, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~LcdBackground : LcdBackground ) ;
		drawIdxText( y, (char *)PSTR( STR_HYPH_INV)+2, ld->revert, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
	}
	if( k == NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
	{
  	//last line available - add the "copy trim menu" line
  	uint8_t attr = (sub==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS) ? INVERS : 0 ;
  	PUTS_ATT_COLOUR( 3*FW, y+TVOFF, PSTR(STR_COPY_TRIM), 0, attr ? ~LcdForeground : LcdForeground ) ; // , attr ? ~LcdBackground : LcdBackground ) ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}
}

void menuOneSafetySwitch(uint8_t event)
{
	TITLE(XPSTR("Safety Switch")) ;
	static MState2 mstate2 ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	EditType = EE_MODEL ;
	uint32_t rows = 3 ;
	uint16_t y = TTOP ;
	uint16_t colour = dimBackColour() ;
	uint8_t attr ;
	uint32_t index ;
	index = s_currIdx ;
	SKYSafetySwData *sd = &g_model.safetySw[index];

	if ( sd->opt.ss.mode == 3 )
	{
		rows += 1 ;
	}
	event = mstate2.check_columns(event, rows - 1 ) ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t subN ;

	putsChn( 21*FW, 0,index+1,0 ) ;
	
	PUTS_NUMX( 29*FW, 0, g_chans512[index]/2 + 1500 ) ;

	lcd_hline( 0, TTOP, TRIGHT ) ;
	
	subN = 0 ;
  attr = 0 ;
  if(sub==subN)
	{
		attr = INVERS ;
	}
	drawItem( XPSTR("Type"), y, (attr) ) ;
	uint32_t uvalue = sd->opt.ss.mode > 0 ;
	if ( attr )
	{
		CHECK_INCDEC_H_MODELVAR( uvalue, 0, 1 ) ;
	}
//	saveEditColours( sub == subN, colour ) ;
  drawIdxText( y, XPSTR("\006SafetySticky"), uvalue, attr|LUA_RIGHT ) ;
//	restoreEditColours() ;
	sd->opt.ss.mode = uvalue ? 3 : 0 ;
	y += TFH ;
	subN += 1 ;

  attr = 0 ;
  if(sub==subN)
	{
		attr = INVERS ;
	}
	drawItem( XPSTR("Switch"), y, (attr) ) ;
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
	saveEditColours( attr, colour ) ;
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
	sd->opt.ss.swtch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, sd->opt.ss.swtch, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;
 
  attr = 0 ;
  if(sub==subN)
	{
		attr = INVERS ;
	}
	drawItem( XPSTR("Value"), y, (attr) ) ;
	int32_t value = sd->opt.ss.val * 10 ;
	value += value >= 0 ? sd->opt.ss.tune : -sd->opt.ss.tune ;
	
	drawNumber( TRIGHT-TRMARGIN, y, value, attr|PREC1) ; //, attr ? ~colour : colour ) ;
	if ( attr )
	{
		StepSize = 30 ;
		value = checkIncDec16( value, -1250, 1250, EE_MODEL ) ;
	}
	int32_t whole = value / 10 ;
	sd->opt.ss.val = whole ;
	value -= whole * 10 ;
	if ( value < 0 )
	{
		value = -value ;
	}
	sd->opt.ss.tune = value ;
	y += TFH ;
	subN += 1 ;

	if ( sd->opt.ss.mode == 3 )
	{
	  attr = 0 ;
  	if(sub==subN)
		{
			attr = INVERS ;
		}
		drawItem( XPSTR("Source"), y, (attr) ) ;
		int8_t temp = sd->opt.ss.source ;
		saveEditColours( attr, colour ) ;
		if ( temp == 0 )
		{
			temp = 3 ;
		}
		else
		{
			if ( temp > 0 )
			{
				temp += 4 ;
			}
			else
			{
				temp -= 4 ;
			}
		}
		if ( temp < 0 )
		{
			temp = - temp ;
			PUTC( TRIGHT-TRMARGIN-4*FW, y+TVOFF, '!' ) ;
		}
		if ( temp > 7 )
		{ // AN extra pot
			temp += EXTRA_POTS_START - 8 ;
		}
		putsChnRaw( (TRIGHT-TRMARGIN)*2, (y+TVOFF)*2, temp, LUA_RIGHT ) ;
		restoreEditColours() ;
	  if(attr)
		{
			CHECK_INCDEC_H_MODELVAR( sd->opt.ss.source, -3-NumExtraPots , 3+NumExtraPots ) ;
    }
	}
}


void menuProcSafetySwitches(uint8_t event)
{
	TITLE(PSTR(STR_SAFETY_SW));
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	EditType = EE_MODEL ;
#ifdef NO_VOICE_SWITCHES 
 #define SAFE_VAL_OFFSET	0
	uint32_t rows = NUM_SKYCHNOUT+1+1+NUM_VOICE-1 - 1 ;
#else
 #define SAFE_VAL_OFFSET	1
	uint32_t rows = NUM_SKYCHNOUT+1+1+NUM_VOICE-1 ;
#endif
	static MState2 mstate2;

#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	event = mstate2.check_columns(event, rows - 1 ) ;

#ifdef TOUCH
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif
	coord_t y = 0 ;
	uint32_t k = 0 ;
	uint32_t sub = mstate2.m_posVert ;
	uint32_t subSub = g_posHorz ;
	uint32_t t_pgOfs ;

	t_pgOfs = evalHresOffset( sub ) ;
//	t_pgOfs = evalOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

#ifdef TOUCH
	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, NUM_SKYCHNOUT+EXTRA_SKYCHANNELS+1-(TLINES)+1, t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

#ifdef NO_VOICE_SWITCHES
	PUTS_NUMX( 24*FW, 0, g_chans512[sub]/2 + 1500 ) ;
#else
	if ( sub )
	{
    PUTS_NUMX( 24*FW, 0, g_chans512[sub-SAFE_VAL_OFFSET]/2 + 1500 ) ;
	}
#endif

	lcd_hline( 0, TTOP, TRIGHT ) ;
	
	for(uint32_t i=0; i<TLINES; i++)
	{
    y=(i)*TFH +TTOP ;
    k=i+t_pgOfs;
	  uint8_t attr = (sub==k) ? INVERS : 0 ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
		}
#ifndef NO_VOICE_SWITCHES
		if ( k == 0 )
		{
    	if(sub==k)
			{
				Columns = 0 ;
#if EXTRA_SKYCHANNELS
  	    CHECK_INCDEC_H_MODELVAR( g_model.numVoice, -8, NUM_SKYCHNOUT ) ;
#else
  	    CHECK_INCDEC_H_MODELVAR_0( g_model.numVoice, NUM_SKYCHNOUT ) ;
#endif
			}
//			lcd_xlabel_decimal( 18*FW, y, g_model.numVoice+NUM_VOICE, attr, PSTR(STR_NUM_VOICE_SW) ) ;
			drawText( THOFF, y, (char *)PSTR(STR_NUM_VOICE_SW), attr ) ;
			drawNumber( TRIGHT-TRMARGIN, y, g_model.numVoice+NUM_VOICE, attr ) ;
		}
  	else // if(k<NUM_SKYCHNOUT+1+NUM_VOICE)
#endif
		{
#ifdef NO_VOICE_SWITCHES
			uint8_t numSafety = NUM_SKYCHNOUT + EXTRA_SKYCHANNELS ; 
    	SKYSafetySwData *sd = &g_model.safetySw[k];
#else
			uint8_t numSafety = NUM_SKYCHNOUT - g_model.numVoice ;
    	SKYSafetySwData *sd = &g_model.safetySw[k-1];
#endif
			if ( sub==k )
			{
//				Columns = 2 ;
			}
#ifdef NO_VOICE_SWITCHES
    	if ( k >= NUM_SKYCHNOUT )
			{
				sd = (SKYSafetySwData*)&g_model.voiceSwitches[k-NUM_SKYCHNOUT];
			}
#else
    	if ( k-1 >= NUM_SKYCHNOUT )
			{
				sd = (SKYSafetySwData*)&g_model.voiceSwitches[k-1-NUM_SKYCHNOUT];
			}
#endif
			if ( k && ( k <= numSafety ) )
			{
//  		  if ( ( s_editMode == false ) && ( sub==k ) )
  		  if ( sub==k )
				{
#ifdef TOUCH
					if ( handleSelectIcon() || selected )
					{
						selected = 0 ;
						s_currIdx = sub-1 ;
						pushMenu(menuOneSafetySwitch) ;
  		  		s_editMode = false ;
					}
#endif
					if ( checkForMenuEncoderBreak( event ) )
					{
						s_currIdx = sub-1 ;
						pushMenu(menuOneSafetySwitch) ;
  		  		s_editMode = false ;
						Tevent = event = 0 ;
					}
				}

				if (getSwitch00(sd->opt.ss.swtch) )
				{
					attr |= BOLD ;
				}

#ifdef NO_VOICE_SWITCHES
		    putsChnColour( THOFF, y*2+TVOFF, k+1, attr ) ;
#else
    		putsChnColour( THOFF, y*2+TVOFF, k, attr ) ;
#endif
				attr &= ~BOLD ;


  	  	for(uint32_t j=0; j<5;j++)
				{
  	  	  if (j == 0)
					{
						drawIdxTextAtX( 6*FW, y*2+TVOFF, XPSTR("\001SAVX"), sd->opt.ss.mode, attr ) ;
						{
							uint8_t b = sd->opt.ss.mode ;
#ifdef PCBX9LITE
							if ( ( k < numSafety ) & ( b == 0 ) )
#else
							if ( ( k < 8 ) & ( b == 0 ) )
#endif
							{
								if ( sd->opt.ss.mode != 0 )
								{
									sd->opt.ss.mode = 3 ;
								}
							}
#ifdef PCBX9LITE
							if ( ( k < numSafety ) & ( b == 3 ) )
							{
								if ( sd->opt.ss.mode != 3 )
								{
									sd->opt.ss.mode = 0 ;
								}
							}
#endif
  	  	    }
					}
	    	  else if (j == 1)
  	  	  {
//						int8_t max = MaxSwitchIndex ;
//						if ( sd->opt.ss.mode == 2 )
//						{
//							max = MaxSwitchIndex+3 ;
//						}	 
						{
							saveEditColours( attr, DimBackColour ) ;
         	  	putsDrSwitches( 7*FW+3, y+TVOFF/2, sd->opt.ss.swtch, 0 ) ;
							restoreEditColours() ;
						}
					}
					else if ( j == 2 )
					{
						int8_t min, max ;
						if ( sd->opt.ss.mode == 1 )
						{
							min = 0 ;
							max = 15 ;
							sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
							PUTS_AT_IDX(14*FW, y+TVOFF/2, PSTR(STR_SOUNDS), sd->opt.ss.val,attr);
						}
						else if ( sd->opt.ss.mode == 2 )
						{
							if ( sd->opt.ss.swtch > MAX_SKYDRSWITCH )
							{
								min = 0 ;
								max = NUM_TELEM_ITEMS-1 ;
								sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
  							putsAttIdxTelemItems( 15*FW, y+TVOFF, sd->opt.ss.val+1, 0 ) ;
							}
							else
							{
								min = -128 ;
								max = 111 ;
								sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
        				drawNumber( 15*FW, y, sd->opt.ss.val+128, attr ) ;
							}
						}
						else
						{
        			drawChar( 14*FW, y, '.', attr, (attr & INVERS) ? ~LcdForeground : LcdForeground ) ;
							min = -125 ;
							max = 125 ;
        			drawNumber(  14*FW, y, sd->opt.ss.val, attr ) ;
// Option to display current channel value, before limits etc., for failsafe
						}
					}
					else if ( j == 3 )
					{
						if ( ( sd->opt.ss.mode == 0 ) || ( sd->opt.ss.mode == 3 ) )
						{
        			drawNumber(  15*FW+3, y, sd->opt.ss.tune, attr);
						}
					}
					else
					{
						if ( sd->opt.ss.mode == 3 )
						{
							int8_t temp = sd->opt.ss.source ;
							if ( temp == 0 )
							{
								temp = 3 ;
							}
							else
							{
								if ( temp > 0 )
								{
									temp += 4 ;
								}
								else
								{
									temp -= 4 ;
								}
							}
							if ( temp < 0 )
							{
								temp = - temp ;
								PUTC( 16*FW, y, '!' ) ;
							}
							if ( temp > 7 )
							{ // AN extra pot
								temp += EXTRA_POTS_START - 8 ;
							}
							saveEditColours( attr, 0 ) ;
							putsChnRaw( (17*FW)*2, (y+TVOFF/2)*2, temp, 0 ) ;
							restoreEditColours() ;
						}
					}
    	  }
    	}
			else
			{
				Columns = 2 ;
	  	  PUTS_ATT_LEFT( y, PSTR(STR_VS) ) ;
				uint16_t x1 ;
				x1 = 0 + 4*FW-2 ;
				if ( k < 10 )
				{
					x1 -= FWNUM ;			
				}
  			PUTS_NUM(x1,y,k,attr);
    		for(uint32_t j=0; j<3;j++)
				{
    	    uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
					uint8_t active = (attr && (s_editMode /*|| P1values.p1valdiff*/)) ;
    		  if (j == 0)
					{
    		    if(active)
						{
    			    CHECK_INCDEC_MODELSWITCH( sd->opt.vs.vswtch, -MaxSwitchIndex+1, MaxSwitchIndex-1 ) ;
    		    }
  			    putsDrSwitches(5*FW, y+TVOFF/2, sd->opt.vs.vswtch, attr);
					}
    		  else if (j == 1)
    		  {
    		    if(active)
						{
    			    CHECK_INCDEC_H_MODELVAR_0( sd->opt.vs.vmode, 6 ) ;
    		    }
						PUTS_AT_IDX( 10*FW, y+TVOFF/2, PSTR(STR_VOICE_OPT), sd->opt.vs.vmode, attr ) ;
					}
					else
					{
						uint8_t max ;
						if ( sd->opt.vs.vmode > 5 )
						{
							max = NUM_TELEM_ITEMS-1 ;
							sd->opt.vs.vval = limit( (uint8_t)0, sd->opt.vs.vval, max) ;
							putsAttIdxTelemItems( 16*FW, y+TVOFF/2, sd->opt.vs.vval+1, attr ) ;
						}
						else
						{
							// Allow 251-255 to represent GVAR3-GVAR7
							max = 255 ;
							if ( sd->opt.vs.vval <= 250 )
							{
	  						PUTS_NUM( 19*FW, y+TVOFF/2, sd->opt.vs.vval, attr) ;
							}	
							else
							{
								dispGvar( 16*FW, y+TVOFF/2, sd->opt.vs.vval-248, attr ) ;
							}
						}
    			  if(active)
						{
    		      sd->opt.vs.vval = checkIncDec16( sd->opt.vs.vval, 0, max, EE_MODEL);
							if ( checkForMenuEncoderLong( event ) )
							{
								if ( sd->opt.vs.vval <= 250 )
								{
									putVoiceQueue( sd->opt.vs.vval | VLOC_NUMUSER  ) ;
								}
							}
    			  }
					}	 
			  }
			}
		}
	}
}

void menuProcTemplates(uint8_t event)  //Issue 73
{
	TITLE( PSTR(STR_TEMPLATES) ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	 
	static MState2 mstate2 ;
	event = mstate2.check_columns( event, NUM_TEMPLATES-1 ) ;
	EditType = EE_MODEL ;

  coord_t y = TTOP ;
  uint8_t k = 0 ;
  int8_t sub = mstate2.m_posVert ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( NUM_TEMPLATES, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

#ifdef TOUCH
	if ( checkForMenuEncoderLong( event ) || handleSelectIcon() )
	{
    //apply mixes or delete
    s_noHi = NO_HI_LEN ;
    applyTemplate(sub) ;
    audioDefevent(AU_WARNING2) ;
  }
#else
	if ( checkForMenuEncoderLong( event ) )
	{
    //apply mixes or delete
    s_noHi = NO_HI_LEN ;
    applyTemplate(sub) ;
    audioDefevent(AU_WARNING2) ;
  }
#endif
  for(uint32_t i=0; i<(TLINES - 1); i++)
	{
    k = i ; // +t_pgOfs ;
    if( k == NUM_TEMPLATES)
		{
			break ;
		}
    
		if ( sub == k )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
		}
		saveEditColours( sub == k, LcdBackground ) ;
    PUTS_NUM_N( 3*FW+THOFF, y+TVOFF, k+1, 0 + LEADING0,2) ;
    PUTS_ATT( 6*FW, y+TVOFF, PSTR(n_Templates[k]), 0 ) ;
		restoreEditColours() ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
    y += TFH ;
  }
}

void drawSmallGVAR( uint16_t x, uint16_t y )
{
#ifdef USE_VARS
	if ( g_model.vars )
	{
		y -= 2 ;
		lcdDrawChar( x, y, 'V', SMLSIZE ) ;
		lcdDrawChar( LcdNextPos-1, y, 'A', SMLSIZE ) ;
		lcdDrawChar( LcdNextPos-1, y, 'R', SMLSIZE ) ;
		return ;
	}
#endif
//	lcd_putsSmall( x, y, s, LcdForeground) ;
	lcdDrawSizedText( x, y-2, "GVAR", 255, SMLSIZE ) ;
}


extern void createInputMap() ;
extern void displayInputSource( coord_t x, coord_t y, uint8_t index, LcdFlags att ) ;
extern int16_t gvarMenuItem(coord_t x, coord_t y, int16_t value, int8_t min, int8_t max,  LcdFlags attr, uint8_t event ) ;
extern uint8_t InputMap[] ;
extern uint8_t InputMapSize ;

void editOneInput(uint8_t event)
{
	uint8_t rows = 9 ;
	struct te_InputsData *pinput = &g_model.inputs[s_curItemIdx] ;
//	uint32_t newVpos ;

#ifdef TOUCH
	TlExitIcon = 1 ;
#endif

	if ( event == EVT_ENTRY )
	{
		createInputMap() ;
	}

	static MState2 mstate2 ;
	mstate2.check_columns(event, rows - 1 ) ;

	TITLE( XPSTR("Edit Input"));

	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	if ( event == EVT_ENTRY_UP )
	{
		if ( TextResult )
		{
			uint8_t value ;
			value = InputMap[ TextIndex ] ;
			pinput->srcRaw = value ;
			eeDirty(EE_MODEL) ;
		}
	}

  uint32_t  sub    = mstate2.m_posVert;

	uint32_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;
#ifndef TOUCH
	(void) t_pgOfs ;
#endif

	lcd_hline( 0, TTOP, TRIGHT ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

//	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
//	if ( newVpos != t_pgOfs )
//	{
//		s_pgOfs = t_pgOfs = newVpos ;
//		if ( sub < t_pgOfs )
//		{
//			mstate2.m_posVert = sub = t_pgOfs ;
//		}
//		else if ( sub > t_pgOfs + TLINES - 1 )
//		{
//			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
//		}
//	}

  for ( uint32_t k = 0 ; k<TLINES ; k += 1 )
  {
		uint16_t y = TTOP + k*TFH ;
    uint8_t i = k + s_pgOfs ;
    uint8_t attr = sub==i ? INVERS : 0;
  	uint8_t b ;
	
    switch(i)
		{
      case 0:
			{	
				drawItem( (char *)PSTR(STR_2SOURCE)+1, y, attr ) ;
				uint32_t value = pinput->srcRaw ;
				saveEditColours( attr, DimBackColour ) ;
				putsChnOpRaw( (TRIGHT-TRMARGIN)*2, (y+TVOFF)*2, value, 0, 0, LUA_RIGHT ) ;
				restoreEditColours() ;
				
				if ( attr )
				{
					uint8_t x = value ;
#ifdef TOUCH
					if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#else
					if ( ( event == EVT_KEY_BREAK(BTN_RE) ) )
#endif
					{
						// Long MENU pressed
						TextIndex = x ;
  					TextType = TEXT_TYPE_CUSTOM ;
						TextControl.TextOption = 0 ;
						TextControl.TextWidth = 6 ;
						TextControl.TextMax = InputMapSize-1 ;
						TextControl.TextFunction = displayInputSource ;
						TextControl.TextMap = InputMap ;

  					killEvents(event) ;
						pushMenu(menuTextHelp) ;
    				s_editMode = false ;
					}
					pinput->srcRaw = x ;
			  }
			}
			break ;
	    case 1 :
			{	
				drawItem( (char *)PSTR(STR_2WEIGHT)+1, y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
#ifdef USE_VARS
				if ( g_model.vars )
				{
					int16_t value = pinput->weight ;
					if ( pinput->varForWeight )
					{
						value += 1000 ;
					}
					value = editVarCapableValue( TRIGHT-TRMARGIN+FW, y+TVOFF, value, -100, 100, NUM_VARS, attr, event ) ;
					if ( value > 900 )
					{
						value -= 1000 ;
						pinput->varForWeight = 1 ;
					}
					else
					{
						pinput->varForWeight = 0 ;
					}
					pinput->weight = value ;
				}
				else
#endif
				{
					pinput->weight = gvarMenuItem( TRIGHT-TRMARGIN, y+TVOFF, pinput->weight, -100, 100, GVAR_100|attr, event ) ;
				}
				restoreEditColours() ;
 	  		
//				int16_t value = pinput->weight ;
//				if ( value > 100 )
//				{
//					value += 400 ;
//				}	
//				value = gvarDiffValue( 15*FW, y, value, attr, event ) ;
//				restoreEditColours() ;
//				if ( value > 500 )
//				{
//					value -= 400 ;
//				}
//				pinput->weight = value ;
			}
			break ;
	    case 2 :
				drawItem( (char *)PSTR(STR_2SWITCH)+1, y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				putsDrSwitches(TRIGHT-TRMARGIN, y+TVOFF/2, pinput->swtch, LUA_RIGHT) ;
				restoreEditColours() ;
        if(attr) CHECK_INCDEC_MODELSWITCH( pinput->swtch, -MaxSwitchIndex, MaxSwitchIndex);
			break ;
	    case 3 :
				drawItem( (char *)XPSTR("TRIM"), y, attr ) ;
				drawIdxText( y, (char *)XPSTR("\003 OnOffRudEleThrAil"), pinput->carryTrim, attr ) ;
   			if (attr)
				{
					CHECK_INCDEC_H_MODELVAR( pinput->carryTrim, 0, 5) ;
				}
			break ;
	    case 4 :
			{	
				b = 1 ;
				drawItem( (char *)PSTR(STR_MODES), y, attr ) ;
						
				if ( attr )
				{
					Columns = 7 ;
				}
  					
				for ( uint32_t p = 0 ; p<MAX_MODES+2 ; p++ )
				{
					uint8_t z = pinput->flightModes ;
   				PUTC_ATT( TRIGHT-TRMARGIN-7*(FW+3)+p*(FW+2), y+TVOFF, '0'+p, ( z & b ) ? 0 : INVERS ) ;
					if( attr && ( g_posHorz == p ) )
					{
						lcd_rectColour( TRIGHT-TRMARGIN-7*(FW+3)+p*(FW+2)-1, y+TVOFF-1, FW+2, 9, DimBackColour ) ;
//						lcdHiresRect( TRIGHT-TRMARGIN-8*(FW+4)+p*(FW+4)-1, y+TVOFF-1, FW+2, 34, DimBackColour ) ;
//						lcdHiresRect( TRIGHT-TRMARGIN-8*(FW+4)+p*(FW+4)-2, y+TVOFF-2, FW+4, 36, DimBackColour ) ;
//						lcdHiresRect( TRIGHT-TRMARGIN-8*(FW+4)+p*(FW+4)-3, y+TVOFF-3, FW+6, 38, DimBackColour ) ;
						if ( event==EVT_KEY_BREAK(BTN_RE) ) 
						{
							pinput->flightModes ^= b ;
     					eeDirty(EE_MODEL) ;
   						s_editMode = false ;
						}
					}
					b <<= 1 ;
				}
			}
			break ;
	    case 5 :
			{	
				drawItem( (char *)PSTR(STR_OFFSET), y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
				
				int16_t value = pinput->offset ;

#ifdef USE_VARS
				if ( g_model.vars )
				{
					if ( pinput->varForOffset )
					{
						value += 1000 ;
					}
					value = editVarCapableValue( TRIGHT-TRMARGIN+FW, y+TVOFF, value, -100, 100, NUM_VARS, attr, event ) ;
					if ( value > 900 )
					{
						value -= 1000 ;
						pinput->varForOffset = 1 ;
					}
					else
					{
						pinput->varForOffset = 0 ;
					}
				}
				else
#endif
				{
					value = gvarMenuItem( TRIGHT-TRMARGIN, y+TVOFF, value, -100, 100, GVAR_100|attr, event ) ;
				}
				pinput->offset = value ;
				restoreEditColours() ;

//				int16_t value = pinput->offset ;
//				if ( value > 100 )
//				{
//					value += 400 ;
//				}	
//				value = gvarDiffValue( 15*FW, y, value, attr, event ) ;
//				restoreEditColours() ;
//				if ( value > 500 )
//				{
//					value -= 400 ;
//				}
//				pinput->offset = value ;
			}	
			break ;
	    case 6 :
			{	
				drawItem( (char *)XPSTR("Curve"), y, attr ) ;
				drawIdxText( y, (char *)XPSTR("\005 None FuncCurve Diff Expo"), pinput->mode, attr ) ;
				uint8_t oldvalue = pinput->mode ;
   		  if(attr) CHECK_INCDEC_H_MODELVAR( pinput->mode, 0, 4 ) ;
				if ( pinput->mode != oldvalue )
				{
					if ( ( pinput->mode == 3 ) || ( pinput->mode == 4 ) )		// diff/expo
					{
						pinput->curve = 0 ;
					}
					else if ( pinput->mode == 2 )		// curve
					{
						pinput->curve = 0 ;
					}
					else if ( pinput->mode == 1 )		// Function
					{
						pinput->curve = 1 ;
					}
					else
					{
						pinput->curve = 0 ;
					}
				}
			}
			break ;
	    case 7 :
				drawItem( (char *)XPSTR("Value"), y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				if ( pinput->mode == 3 )	// Diff
				{
					int16_t value = pinput->curve ;
					drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
#ifdef USE_VARS
					if ( g_model.vars )
					{
						if ( pinput->varForCurve )
						{
							value += 1000 ;
						}
						value = editVarCapableValue( TRIGHT-TRMARGIN, y+TVOFF, value, -100, 100, NUM_VARS, attr, event ) ;
						if ( value > 900 )
						{
							value -= 1000 ;
							pinput->varForCurve = 1 ;
						}
						else
						{
							pinput->varForCurve = 0 ;
						}
					}
					else
#endif
					{
						if ( value > 100 )
						{
							value += 400 ;
						}	
						value = gvarDiffValue( TRIGHT-TRMARGIN, y+TVOFF, value, attr|0x80000000, event ) ;
						if ( value > 500 )
						{
							value -= 400 ;
						}
					}
					pinput->curve = value ;
				}
				else
				{
					if ( pinput->mode == 4 )	// Expo
					{
						drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
						int16_t value = pinput->curve ;
#ifdef USE_VARS
						if ( g_model.vars )
						{
							if ( pinput->varForCurve )
							{
								value += 1000 ;
							}
							value = editVarCapableValue( TRIGHT-TRMARGIN, y+TVOFF, value, -100, 100, NUM_VARS, attr, event ) ;
							if ( value > 900 )
							{
								value -= 1000 ;
								pinput->varForCurve = 1 ;
							}
							else
							{
								pinput->varForCurve = 0 ;
							}
						}
						else
#endif
						{
							if ( value > 100 )
							{
								value += 400 ;
							}	
							value = gvarDiffValue( TRIGHT-TRMARGIN, y+TVOFF, value, attr|0x80000000, event ) ;
							if ( value > 500 )
							{
								value -= 400 ;
							}
						}
						pinput->curve = value ;
//						PUTS_NUM(TRIGHT-TRMARGIN,y+TVOFF,pinput->curve,0);
//            if(attr) CHECK_INCDEC_H_MODELVAR( pinput->curve, -100, 100 ) ;
					}
					else
					{
						int32_t temp ;
						temp = pinput->curve ;
						if ( pinput->mode == 2 )
						{
							if ( temp >= 0 )
							{
								temp += 7 ;
							}
						}
						put_curve( TRIGHT-TRMARGIN-3*FW, y+TVOFF/2, temp, LUA_RIGHT ) ;
						if ( pinput->mode )
						{
          	  if(attr)
							{
								if ( pinput->mode == 1 ) // Function
								{
        		  		CHECK_INCDEC_H_MODELVAR( pinput->curve, 1, 6 ) ;
								}
								else
								{
        		  		CHECK_INCDEC_H_MODELVAR( pinput->curve, -MAX_CURVE5-MAX_CURVE9-3, MAX_CURVE5+MAX_CURVE9-1+3 ) ;
								}
							}
						}
					}
				}
				restoreEditColours() ;
			break ;
	    case 8 :
				drawItem( (char *)XPSTR("Side"), y, attr ) ;
				drawIdxText( y, (char *)XPSTR("\003---x>0x<0"), pinput->side, attr ) ;
   			if (attr)
				{
					CHECK_INCDEC_H_MODELVAR( pinput->side, 0, 2) ;
				}
			break ;
	  }
	}
}


extern const char *BackResult ;
extern int8_t DupSub ;
extern uint8_t DupIfNonzero ;
extern uint8_t RestoreIndex ;
void menuProcModelIndex(uint8_t event) ;
void menuDeleteDupModel(uint8_t event) ;
void menuProcRestore(uint8_t event) ;
void processSwitches( void ) ;
void menuEditNotes(uint8_t event) ;
void eeLoadModel(uint8_t id) ;

void menuProcModelSelect(uint8_t event)
{
  static MState2 mstate2;
  TITLE(PSTR(STR_MODELSEL));
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	uint32_t t_pgOfs ;
#ifdef TOUCH
	uint32_t newVpos ;
#endif
  static uint8_t sel_editMode ;
	uint32_t selected = 0 ;

  uint32_t subOld  = mstate2.m_posVert;
	uint32_t rows = MAX_MODELS ;
  
	if ( event == EVT_ENTRY )
	{
		sel_editMode = 0 ;
	}

	if ( !PopupData.PopupActive )
	{
  	if ( event == EVT_KEY_FIRST(KEY_EXIT) )
		{
  	  if(sel_editMode)
			{
 	      sel_editMode = false ;
				killEvents( event) ;
				Tevent = event = 0 ;
  	  }
		}
		RotaryState = ROTARY_MENU_UD ;
#ifndef TOUCH
		event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
		event = mstate2.check_columns( event, rows-1 ) ;
	}

  uint32_t sub = mstate2.m_posVert ;
  if ( DupIfNonzero == 2 )
  {
		sel_editMode = false ;
		DupIfNonzero = 0 ;
  }
  
	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	if ( !PopupData.PopupActive )
	{
		int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 2 ) ;
		uint16_t newVert = processSelection( sub , newSelection ) ;
		sub = mstate2.m_posVert = newVert & 0x00FF ;
		selected = newVert & 0x0100 ;

		if ( TouchUpdated )
		{
			if ( TouchControl.event == TEVT_UP )
			{
				if ( ( TouchControl.x <= TSCROLLLEFT ) && (TouchControl.x >= TMID*TSCALE) && !s_editMode )
				{
	  	    if(sel_editMode)
					{
 	  	      sel_editMode = false;
  	  	  }
					else
					{
						PopupData.PopupIdx = 0 ;
						PopupData.PopupActive = 1 ;
					}
					TouchUpdated = 0 ;
				}
			}
		}
	}

	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif



//	if(sub-s_pgOfs < 1)
//	{
//		s_pgOfs = max(0,sub-1) ;
//	}
//  else if(sub-s_pgOfs > (SCREEN_LINES-4) )
//	{
//		s_pgOfs = min(MAX_MODELS-(SCREEN_LINES-2), sub-(SCREEN_LINES-4)) ;
//	}
	
	lcd_hline( 0, TTOP, TRIGHT/2 ) ;
  
	for(uint32_t i = 0 ; i < TLINES ; i += 1 )
	{
    uint16_t y=(i)*TFH +TTOP ;
    uint8_t k=i+t_pgOfs ;
    uint8_t attr = (sub==k) ? INVERS : 0 ;

		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE/2, TFH*TSCALE-2, ~LcdBackground ) ;
		}

		uint16_t fcolour = attr & INVERS ? ~LcdForeground : LcdForeground ;
    PUTS_NUM_N(  3*FW, y+TVOFF, k+1, LEADING0,2 ) ; //, fcolour) ;
    if(k==g_eeGeneral.currModel)
		{
//			lcd_putcAttColour(1, y+TVOFF, '*', 0, fcolour ) ;
  		lcdDrawChar( (1)*2, (y+TVOFF)*2, '*', 0, fcolour ) ;
		}
    PUTS_ATT_N_COLOUR( 4*FW, y+TVOFF, (char *)ModelNames[k+1], sizeof(g_model.name), 0, fcolour ) ;
		
		if ( (sub==k) && (sel_editMode ) )
		{
			lcd_rectColour( 0, y-1+TVOFF, FW*12, TFH-2, fcolour ) ;
		}

		lcd_hline( 0, y+TFH, TRIGHT/2 ) ;
	}

	if ( PopupData.PopupActive )
	{
		uint16_t mask ;
		if ( g_eeGeneral.currModel == mstate2.m_posVert )
		{
			mask = 0x259 ;
		}
		else
		{
			mask = ( eeModelExists( mstate2.m_posVert ) == 0 ) ?  0x96 :  0x017E ;
		}

		uint8_t popaction = doPopup( PSTR(STR_MODEL_POPUP), mask, 10, event ) ;
		
  	if ( popaction == POPUP_SELECT )
		{
			uint8_t popidx = PopupData.PopupSel ;
			if ( popidx == 0 )	// edit
			{
				RotaryState = ROTARY_MENU_LR ;
				chainMenu(menuProcModelIndex) ;
			}
			else if ( ( popidx == 1 ) || ( popidx == 2 ) )	// select or SEL/EDIT
			{

uint32_t checkRssi(uint32_t swappingModels) ;
				checkRssi(1) ;
				
				WatchdogTimeout = 300 ;		// 3 seconds
				stopMusic() ;
				pausePulses() ;
	      STORE_MODELVARS ;			// To make sure we write model persistent timer
extern void eeSaveAll() ;
				eeSaveAll() ;
				g_eeGeneral.currModel = mstate2.m_posVert;
				eeLoadModel( g_eeGeneral.currModel ) ;
				loadModelImage() ;
				protocolsToModules() ;
  			checkTHR();
				checkCustom() ;
				checkSwitches() ;
				checkMultiPower() ;
 				perOut( g_chans512, NO_DELAY_SLOW | FADE_FIRST | FADE_LAST ) ;
				SportStreamingStarted = 0 ;
				speakModelVoice() ;
				sortTelemText() ;
        resetTimers();
				VoiceCheckFlag100mS |= 2 ;// Set switch current states
				processSwitches() ;	// Guarantee unused switches are cleared
				telemetry_init( decodeTelemetryType( g_model.telemetryProtocol ) ) ;
				resumePulses() ;
#ifdef LUA
				luaLoadModelScripts() ;
#endif
#ifdef BASIC
				basicLoadModelScripts() ;
#endif
        STORE_GENERALVARS;
				if ( ( PopupData.PopupActive == 2 ) || ( popidx == 2 ) )
				{
					chainMenu(menuProcModelIndex) ;
				}
				else
				{
	        popMenu(true) ;
				}
			}
			else if ( popidx == 5 )		// Delete
			{
       	killEvents(event);
       	DupIfNonzero = 0 ;
				DupSub = sub + 1 ;
       	pushMenu(menuDeleteDupModel);
			}
			else if( popidx == 3 )	// copy
			{
				{
 	        DupIfNonzero = 1 ;
 	        DupSub = sub + 1 ;
 	        pushMenu(menuDeleteDupModel);//menuProcExpoAll);
				}
			}
			else if( popidx == 6 )	// backup
			{
				WatchdogTimeout = 300 ;		// 3 seconds
				BackResult = ee32BackupModel( mstate2.m_posVert+1 ) ;
				AlertType = MESS_TYPE ;
				AlertMessage = BackResult ;
			}
			else if( popidx == 7 )	// restore
			{
				RestoreIndex = mstate2.m_posVert+1 ;
       	pushMenu( menuProcRestore ) ;				
			}
			else if( popidx == 8 )	// replace
			{
				WatchdogTimeout = 300 ;		// 3 seconds
				BackResult = ee32BackupModel( mstate2.m_posVert+1 ) ;
				AlertType = MESS_TYPE ;
				AlertMessage = BackResult ;
				RestoreIndex = mstate2.m_posVert+1 ;
       	pushMenu( menuProcRestore ) ;				
			}
			else if( popidx == 9 )	// Notes
			{
       	pushMenu( menuEditNotes ) ;
			}
			else // Move = 4
			{
 	    	sel_editMode = true ;
			}
		}
	}
	else
	{
		if ( selected )
		{
			PopupData.PopupIdx = 0 ;
			PopupData.PopupActive = 1 ;
		}
		else
		{
  		switch(event)
  		{
  		//case  EVT_KEY_FIRST(KEY_MENU):
	  		case  EVT_KEY_FIRST(KEY_EXIT):
  		    if(sel_editMode)
					{
 	  	      sel_editMode = false;
  		    }
    	  break ;

	  		case  EVT_KEY_FIRST(KEY_LEFT):
  			case  EVT_KEY_FIRST(KEY_RIGHT):
  		    if(g_eeGeneral.currModel != mstate2.m_posVert)
  		    {
    	      killEvents(event);
						PopupData.PopupIdx = 0 ;
						PopupData.PopupActive = 2 ;
  		    }
					else
					{
						RotaryState = ROTARY_MENU_LR ;
			      if(event==EVT_KEY_FIRST(KEY_LEFT))  chainMenu(menuProcModelIndex);//{killEvents(event);popMenu(true);}

			      if(event==EVT_KEY_FIRST(KEY_RIGHT)) chainMenu(menuProcModelIndex);
					}
 	  	  break;
  		
				case  EVT_KEY_FIRST(KEY_MENU) :
				case  EVT_KEY_BREAK(BTN_RE) :
					if(sel_editMode)
					{
  		    	sel_editMode = false ;
					}
					else
					{
    	      killEvents(event);
						PopupData.PopupIdx = 0 ;
						PopupData.PopupActive = 1 ;
					}	 
  		    s_editMode = 0 ;
  		  break;
  	
				case  EVT_KEY_LONG(BTN_RE) :
				if ( g_eeGeneral.disableBtnLong )
				{
					break ;
				}		
				case  EVT_KEY_LONG(KEY_EXIT):  // make sure exit long exits to main
  		    popMenu(true);
    	  break;

  			case EVT_ENTRY:
  		    sel_editMode = false;
					PopupData.PopupActive = 0 ;
  		    mstate2.m_posVert = g_eeGeneral.currModel ;
    		break;
  		}
		}
	}

  if(sel_editMode && subOld!=sub)
	{
		ee32SwapModels( subOld+1, sub+1 ) ;

		if ( sub == g_eeGeneral.currModel )
		{
			g_eeGeneral.currModel = subOld ;
  	  STORE_GENERALVARS ;     //eeWriteGeneral();
		}
		else if ( subOld == g_eeGeneral.currModel )
		{
			g_eeGeneral.currModel = sub ;
  	  STORE_GENERALVARS ;     //eeWriteGeneral();
		}
  }
}


#define WCHART 40
#define X0     (200-WCHART-2 - 2 )	// was 170
#define Y0     60 // 48

#define XD (X0)

#define GRAPH_FUNCTION_CURVE		0
#define GRAPH_FUNCTION_EXPO			1
#define GRAPH_FUNCTION_INPUT		2

uint8_t get_dr_state(uint8_t x) ;
void editExpoVals(uint8_t event, uint8_t edit, coord_t x, coord_t y, uint8_t which, uint8_t exWt, uint8_t stkRL) ;
void drawFunction( uint8_t xpos, uint8_t function ) ;


void menuProcExpoAll(uint8_t event)
{
	TITLE(PSTR(STR_EXPO_DR)) ;
	EditType = EE_MODEL ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2 ;
	uint32_t count = 5-1 ;
	if ( SingleExpoChan )
	{
		count -= 1 ;
	}
	int8_t  sub = mstate2.m_posVert ;
	
	event = mstate2.check_columns( event, count ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( 7, 0, 1 ) ;
	if ( newSelection >= 0 )
	{
		if ( newSelection == 6 )
		{
			newSelection = 4 ;
		}
		else if ( newSelection == 5 )
		{
			newSelection = 3 ;
		}
		else if ( newSelection > 2 )
		{
			newSelection = 2 ;
		}
		else if ( newSelection )
		{
			newSelection = 1 ;
		}
		else if ( SingleExpoChan )
		{
			newSelection = sub ;
		}
		
//		if ( sub == newSelection )
//		{
////			selected = 1 ;
//			event = Tevent = EVT_KEY_BREAK(BTN_RE) ;
//			s_editMode = !s_editMode ;
//		}
		sub = mstate2.m_posVert = newSelection ;
	}
#endif
	 
	uint8_t stkVal ;
	sub = mstate2.m_posVert ;
	int8_t subN ;
	if ( SingleExpoChan )
	{
		sub += 1 ;
	}
//	if( sub )
//	{
//		StickScrollAllowed = 0 ;
//	}

	lcd_hline( 0, TTOP, TMID-15 ) ;

	uint8_t l_expoChan = s_expoChan ;
	subN = 0 ;
  uint8_t attr = 0 ;
	if ( sub == subN )
	{
		lcdDrawSolidFilledRectDMA( 0, TTOP*TSCALE+2, (TMID-15)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
		if ( SingleExpoChan == 0 )
		{
			s_expoChan = l_expoChan = checkIncDec( s_expoChan, 0, 3, 0 ) ;
			attr = InverseBlink ;
		}
	}		 
	saveEditColours( attr, DimBackColour ) ;
	putsChnRaw( THOFF*2, (TTOP+TVOFF)*2,l_expoChan+1,0) ;
	restoreEditColours() ;

	uint8_t expoDrOn = get_dr_state(l_expoChan);
	switch (expoDrOn)
	{
    case DR_MID:
      PUTS_P( 8*FW, TTOP+TVOFF,PSTR(STR_4DR_MID)+1);
    break;
    case DR_LOW:
      PUTS_P( 8*FW, TTOP+TVOFF,PSTR(STR_4DR_LOW)+1);
    break;
    default: // DR_HIGH:
      PUTS_P( 8*FW, TTOP+TVOFF,PSTR(STR_4DR_HI)+1);
    break;
	}

	subN += 1 ;
	stkVal = DR_BOTH ;
	if(calibratedStick[l_expoChan]> 100) stkVal = DR_RIGHT;
	if(calibratedStick[l_expoChan]<-100) stkVal = DR_LEFT;
	if(IS_EXPO_THROTTLE(l_expoChan)) stkVal = DR_RIGHT;

#ifdef TOUCH
	ExpoData *eptr ;
	eptr = &g_model.expoData[s_expoChan] ;
extern int16_t TouchAdjustValue ;
#endif

	lcd_hline( 0, TTOP+TFH, TMID-15 ) ;

	if ( sub==subN )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+TFH)*TSCALE+2, (TMID-15)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	}
	saveEditColours( ( sub==subN ), DimBackColour ) ;
	PUTS_P( THOFF,TTOP+TFH+TVOFF,PSTR(STR_2EXPO)+1);
	restoreEditColours() ;

	attr = (stkVal != DR_RIGHT) && (sub==subN) ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+2*TFH)*TSCALE+2, (TMID-20)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	drawSmallGVAR( 1, (TTOP+2*TFH)*TSCALE+2 ) ;
	editExpoVals( event, attr, 17*FW/2, TTOP+2*TFH+TVOFF, expoDrOn, DR_EXPO, DR_LEFT ) ;
	restoreEditColours() ;
#ifdef TOUCH
	if ( stkVal == DR_BOTH )
	{
		if ( TouchAdjustValue )
		{
			eptr->expo[expoDrOn][DR_EXPO][DR_RIGHT] = eptr->expo[expoDrOn][DR_EXPO][DR_LEFT] ;
		}
	}
#endif
	attr = (stkVal != DR_LEFT) && (sub==subN) ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+2*TFH)*TSCALE+2, ((TMID-10)/2)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	drawSmallGVAR( (TMID-20)/2*TSCALE+1, (TTOP+2*TFH)*TSCALE+2 ) ;
	editExpoVals( event, attr, 16*FW, TTOP+2*TFH+TVOFF, expoDrOn, DR_EXPO, DR_RIGHT ) ;
	restoreEditColours() ;
	lcd_hline( 0, TTOP+2*TFH, TMID-15 ) ;
	subN += 1 ;
	attr = (stkVal != DR_RIGHT) && (sub==subN) ;
	lcd_hline( 0, TTOP+3*TFH, TMID-15 ) ;
	 
	if ( sub==subN )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+3*TFH)*TSCALE+2, (TMID-15)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	}
	saveEditColours( ( sub==subN ), DimBackColour ) ;
	PUTS_P( THOFF,TTOP+3*TFH+TVOFF,PSTR(STR_2WEIGHT)+1);
	restoreEditColours() ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+4*TFH)*TSCALE+2, (TMID-20)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	drawSmallGVAR( 1, (TTOP+4*TFH)*TSCALE+2 ) ;
	editExpoVals( event, attr, 17*FW/2, TTOP+4*TFH+TVOFF, expoDrOn, DR_WEIGHT, DR_LEFT ) ;
	restoreEditColours() ;
#ifdef TOUCH
	if ( stkVal == DR_BOTH )
	{
		if ( TouchAdjustValue )
		{
			eptr->expo[expoDrOn][DR_WEIGHT][DR_RIGHT] = eptr->expo[expoDrOn][DR_WEIGHT][DR_LEFT] ;
		}
	}
#endif
	attr = (stkVal != DR_LEFT) && (sub==subN) ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+4*TFH)*TSCALE+2, (TMID-10)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	drawSmallGVAR( (TMID-20)/2*TSCALE+1, (TTOP+4*TFH)*TSCALE+2 ) ;
	editExpoVals( event, attr, 16*FW, TTOP+4*TFH+TVOFF, expoDrOn, DR_WEIGHT, DR_RIGHT ) ;
	restoreEditColours() ;
	lcd_hline( 0, TTOP+4*TFH, TMID-15 ) ;
	subN += 1 ;
	PUTS_P( THOFF, TTOP+5*TFH+TVOFF,PSTR(STR_DR_SW1));
	lcd_hline( 0, TTOP+5*TFH, TMID-15 ) ;
	attr = sub==subN ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+5*TFH)*TSCALE+2, (TMID-10)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	editExpoVals( event, attr, 10*FW, TTOP+5*TFH+TVOFF, DR_DRSW1, 0, 0 ) ;
	restoreEditColours() ;
	subN += 1 ;
	
	PUTS_P( THOFF, TTOP+6*TFH+TVOFF,PSTR(STR_DR_SW2));
	lcd_hline( 0, TTOP+6*TFH, TMID-15 ) ;
	attr = sub==subN ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+6*TFH)*TSCALE+2, (TMID-10)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	editExpoVals( event, attr, 10*FW, TTOP+6*TFH+TVOFF, DR_DRSW2, 0, 0 ) ;
	restoreEditColours() ;
	
	lcd_hline( 0, TTOP+7*TFH, TMID-15 ) ;

	pushPlotType( PLOT_COLOUR ) ;
	
	lcd_vline(XD+2 - (IS_EXPO_THROTTLE(s_expoChan) ? WCHART : 0), Y0 - WCHART, WCHART * 2);

	drawFunction( XD+2, GRAPH_FUNCTION_EXPO ) ;

	int16_t x512  = calibratedStick[s_expoChan];
	int16_t y512 = calcExpo( l_expoChan, x512 ) ;
  
	PUTS_NUM( XD+7*FW, Y0+FH,x512*25/((signed) RESXu/4), 0 );
	PUTS_NUM( XD+FW, TTOP,y512*25/((signed) RESXu/4), 0 );
	
	int16_t xv = (x512 * WCHART + RESX/2) / RESX + XD+2 ;
  int16_t yv = Y0 - (y512 * WCHART + RESX/2) / RESX ;

	lcd_vline( xv, yv-3, 7 ) ;
	lcd_hline( xv-3, yv, 7 ) ;
	
	popPlotType() ;
}


#define OPTION_NONE			0
#define OPTION_OPTION		1
#define OPTION_RFTUNE		2
#define OPTION_VIDFREQ	3
#define OPTION_FIXEDID	4
#define OPTION_TELEM		5
#define OPTION_SRVFREQ	6
#define OPTION_MAXTHR		7
#define OPTION_RFCHAN		8

void multiOptionTouch( uint32_t x, uint32_t y, int32_t option, uint32_t attr, uint32_t protocol )
{
	char *idxText = 0 ;
	uint32_t display = 1 ;
#ifdef PCBT16
	if ( EditingModule == 0 )
	{
#endif
	if (MultiSetting.valid & 4 )
	{
		if ( protocol != M_DSM )
		{
			protocol = 256 + (MultiSetting.subData >> 4) ;
		}
	}
#ifdef PCBT16
	}
#endif

	switch ( protocol )
	{
		case M_DSM :
			if ( ( option >= 4 ) && ( option <= 12 ) )
			{
				idxText = XPSTR("\004 4ch 5ch 6ch 7ch 8ch 9ch10ch11ch12ch") ;
//				return ;
			}
		break ;

		case M_FRSKYX2 :
		case M_FRSKYX :
		case M_FrskyD :
		case M_FRSKYV :
		case M_SFHSS :
		case M_CORONA :
		case M_Hitec :
		case 256 + OPTION_RFTUNE :
			drawItem( XPSTR("Freq."), y, attr ) ;
			display = 0 ;
		break ;
		
		case M_Devo :
		case 256 + OPTION_FIXEDID :
			if ( ( option >= 0 ) && ( option <= 1 ) )
			{
				idxText = XPSTR("\006AutoIDFixdID") ;
//				return ;
			}
		break ;

		case 256 + OPTION_SRVFREQ :
		case M_AFHD2SA :
		{
			char text[8] ;
			text[0] = ' ' ;
			text[1] = ' ' ;
			uint8_t xoption = option ;
			if ( xoption & 0x80 )
			{
				text[0] = 'T' ;
				text[1] = '-' ;
				xoption &= 0x7F ;
				option = xoption ;
			}
			if ( xoption <= 70 )
			{
				cpystr( (uint8_t *)&text[2], (uint8_t *)"Rate" ) ;
				drawItem( text, y, attr ) ;
				display = 0 ;
				option = xoption ;
				option *= 5 ;
				option += 50 ;
			}
		}
		break ;

		case 256 + OPTION_VIDFREQ :
		case M_Hubsan :
			drawItem( XPSTR("VTX"), y, attr ) ;
			display = 0 ;
		break ;

		case 256 + OPTION_TELEM :
		case M_BAYANG :
			if ( ( option >= 0 ) && ( option <= 1 ) )
			{
				drawItem( XPSTR("Telem."), y, attr ) ;
				display = 0 ;
				idxText = XPSTR("\001NY") ;
			}
		break ;
		case 256 + OPTION_RFCHAN :
			drawItem( XPSTR("Rf Chan."), y, attr ) ;
			display = 0 ;
		break ;
	}
	if ( display )
	{
		drawItem( (char *)PSTR(STR_MULTI_OPTION)+1, y, attr ) ;
	}
	if ( idxText )
	{
		drawIdxText( y, idxText, option, attr ) ;
	}
	else
	{
	 	drawNumber( TRIGHT-TRMARGIN, y, option, attr ) ;
	}
}

void editAccessProtocol( uint8_t module, uint8_t event ) ; //, uint8_t start )

void editOneProtocol( uint8_t event )
{
	uint8_t need_bind_range = 0 ;
	EditType = EE_MODEL ;
	uint8_t dataItems = 6 ;
	uint8_t module = EditingModule ;
#ifdef TOUCH
	uint32_t newVpos ;
#endif
	uint8_t attr ;
	if ( module )
	{
		module = 1 ;
	}
	struct t_module *pModule = &g_model.Module[module] ;

#ifdef ACCESS
	if ( pModule->protocol == PROTO_ACCESS )
	{
		editAccessProtocol( module, event ) ; //, 0 ) ;
		return ;
	}
#endif

	if (pModule->protocol == PROTO_PPM)
	{
		dataItems += 2 ;
	}
	
	if (pModule->protocol == PROTO_PXX)
	{
		dataItems += 5 ;
		need_bind_range |= 5 ;
		if ( pModule->sub_protocol == 3 )	// R9M
		{
			dataItems += 2 ;
		}
		if ( pModule->sub_protocol == 0 )	// D16
		{
			dataItems += 1 ;
		}
#ifdef ALLOW_EXTERNAL_ANTENNA
		if ( module == 0 )
		{
			dataItems += 1 ;
		}
#endif
		if ( pModule->channels > 1 )
		{
			pModule->channels = 0 ;
		}
	}
	if (pModule->protocol == PROTO_MULTI)
	{
		dataItems += 9 ;
		need_bind_range |= 5 ;
		if ( ( pModule->ppmFrameLength < 0 ) || ( pModule->ppmFrameLength > 4 ) )
		{
			pModule->ppmFrameLength = 0 ;
		}
		if ( event == EVT_ENTRY)
		{
			MultiSetting.protocol[0] = 0 ;
			MultiSetting.subProtocol[0] = 0 ;
		}
	}
	if (pModule->protocol == PROTO_DSM2)
	{
		dataItems += 1 ;
#ifdef ENABLE_DSM_MATCH  		
		if (pModule->sub_protocol >= 3)
		{
			dataItems += 1 ;
		}
#endif
		{
			need_bind_range |= 4 ;
		  if ( pModule->sub_protocol == DSM_9XR )
			{
				dataItems += 1 ;
				need_bind_range |= 1 ;
			}
		}
	}

#ifdef XFIRE
	if ( module && (pModule->protocol == PROTO_XFIRE ) )
	{
		dataItems -= 2 ;
	}
#endif
	
	if ( pModule->protocol == PROTO_OFF )
	{
		dataItems -= 5 ;
	}

//	uint8_t blink = InverseBlink ;

	TITLE( XPSTR("Protocol") ) ;
	static MState2 mstate2 ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif

	if (pModule->protocol == PROTO_OFF)
	{
		mstate2.m_posVert = 0 ;
	}

	displayModuleName( 14*FW, 0, module, 0 ) ;
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, dataItems-1 ) ;
#endif
	event = mstate2.check_columns( event, dataItems-1 ) ;

	uint32_t  sub    = mstate2.m_posVert ;
	uint32_t t_pgOfs ;
	
	t_pgOfs = evalHresOffset( sub ) ;
#ifndef TOUCH
	(void) t_pgOfs ;
#endif	 

	coord_t y = TTOP ;

	uint32_t subN = 0 ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( dataItems, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;

	if ( dataItems >= TLINES )
	{
		newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, dataItems-(TLINES-1), t_pgOfs ) ;
		if ( newVpos != t_pgOfs )
		{
			s_pgOfs = t_pgOfs = newVpos ;
			if ( sub < t_pgOfs )
			{
				mstate2.m_posVert = sub = t_pgOfs ;
			}
			else if ( sub >= t_pgOfs + TLINES - 1 )
			{
				mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
			}
		}
	}
#endif

	if(t_pgOfs<=subN)
	{
		uint8_t value ;
		uint8_t newvalue ;
		value = (pModule->protocol == PROTO_OFF) ? 0 : 1 ;
//		newvalue = onoffMenuItem( value, y, XPSTR("Enable"), sub==subN ) ;
		newvalue = touchOnOffItem( value, y, XPSTR("Enable"), (sub == subN ), DimBackColour ) ;
		if ( newvalue != value )
		{
#if defined(PCBX12D) || defined (PCBXLITE) || defined (PCBX9D) || defined (PCBX9LITE) || defined(PCBX10)
			if ( module == 0 )
			{
#if defined (PCBX9LITE) || (defined(PCBX10) && defined(PCBREV_EXPRESS)) || defined(PCBX7ACCESS)
				killEvents(event) ;
				value = newvalue ? PROTO_ACCESS : PROTO_OFF ;
#else
  #ifdef PCBT16
				value = newvalue ? PROTO_MULTI : PROTO_OFF ;
  #else
				value = newvalue ? 1 : PROTO_OFF ;
  #endif
#endif
			}
			else
#endif
			{
				value = newvalue ? 0 : PROTO_OFF ;
			}
		}
		else
		{
			value = pModule->protocol ;
		}
		if ( pModule->protocol != value )
		{
			pModule->protocol = value ;
#ifdef ACCESS
			if ( value == PROTO_ACCESS )
			{
				AccEntry = 1 ;
				return ;
			}
#endif
		}
  	y+=TFH ;
	}
	subN += 1 ;

	if ( pModule->protocol != PROTO_OFF )
	{
		if(t_pgOfs<=subN)
		{
			attr = 0 ;
			if ( sub==subN )
			{
				attr = INVERS ;
#ifdef ACCESS
				if (checkProtocolOptions( module ) )
				{
					if ( g_model.Module[module].protocol == PROTO_ACCESS )
					{
						AccEntry = 1 ;
					}
				}
#else
				checkProtocolOptions( module ) ;
#endif
			}
			drawItem( (char *)PSTR(STR_PROTO), y, attr ) ;
//			if ( pModule->protocol == PROTO_OFF )
//			{
//				drawText( TRIGHT-3*FW-TRMARGIN, y, (char *)"OFF", attr ) ;
//			}
//			else
//			{
			drawIdxText( y, (char *)PSTR(STR_PROT_OPT), pModule->protocol, attr ) ;
//			}
			if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
		}
		subN += 1 ;
		if(t_pgOfs<=subN)
		{
			if ( pModule->protocol == PROTO_PPM )
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				if ( ( pModule->channels > 8 ) || (pModule->channels < -4) )
				{
					pModule->channels = 0 ;		// Correct if wrong from DSM
				}
				drawItem( XPSTR("Channels"), y, attr ) ;
				uint8_t chans = pModule->channels + 4 ;
//					attr = LEFT ;
				drawNumber( TRIGHT-TRMARGIN, y, chans + 4, attr ) ;
				if ( attr )
				{
 		      CHECK_INCDEC_H_MODELVAR_0( chans, 12 ) ;
					pModule->channels = chans - 4 ;
				}
				subN += 1 ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
				if(t_pgOfs<=subN)
				{
					attr = ( sub==subN ) ? INVERS : 0 ;
					drawItem( XPSTR("Pulse Width (uS)"), y, attr ) ;
					drawNumber( TRIGHT-TRMARGIN, y, (pModule->ppmDelay*50)+300, attr ) ;
					if ( attr )
					{
						CHECK_INCDEC_H_MODELVAR( pModule->ppmDelay,-4,10);
					}
				}
			}
			else
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
		  	if ( ( pModule->protocol == PROTO_DSM2) && ( pModule->sub_protocol == DSM_9XR ) )
  			{
					drawItem( XPSTR("\013Chans")+1, y, attr ) ;
					drawNumber( TRIGHT-TRMARGIN, y, pModule->channels, attr ) ;
				}
				else
				{
					drawItem( (char *)PSTR(STR_13_RXNUM)+1, y, attr ) ;
					drawNumber( TRIGHT-TRMARGIN, y, pModule->pxxRxNum, attr ) ;
				}
				if ( attr )
				{
			  	if ( ( pModule->protocol == PROTO_DSM2) && ( pModule->sub_protocol == DSM_9XR ) )
					{
  	          CHECK_INCDEC_H_MODELVAR( pModule->channels, 6, 14) ;
					}
					else
					{
						uint8_t max = 63 ;
  		      CHECK_INCDEC_H_MODELVAR_0( pModule->pxxRxNum, max ) ;
					}
				}
			}
			if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
//			if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
		}
		subN += 1 ;

		if(t_pgOfs<=subN)
		{
			attr = ( sub==subN ) ? INVERS : 0 ;
			drawItem( XPSTR("Detect Telem."), y, attr ) ;
			drawIdxText( y, (char *)XPSTR("\006  AutoSelect"), g_model.ForceTelemetryType, attr ) ;
			if ( attr )
			{
				CHECK_INCDEC_H_MODELVAR_0( g_model.ForceTelemetryType, 1 ) ;
			}
			if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
//			if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
		}
		subN += 1 ;

		if ( pModule->protocol == PROTO_MULTI )
		{
			if(t_pgOfs<=subN)
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				drawItem( (char *)PSTR(STR_MULTI_PROTO), y, attr ) ;
				uint8_t oldValue = pModule->sub_protocol & 0x3F ;
				oldValue |= pModule->exsub_protocol << 6 ;
				uint8_t svalue = oldValue ;
				if ( attr )
				{
					svalue = checkOutOfOrder( svalue+1, (uint8_t *)SortedMulti, 127 ) - 1 ;
//			 		CHECK_INCDEC_H_MODELVAR_0( svalue, 127 ) ;	// Limited to 8 bits
				}
				pModule->sub_protocol = ( svalue & 0x3F) + (pModule->sub_protocol & 0xC0) ;
				pModule->exsub_protocol = svalue >> 6 ;
				if( svalue != oldValue )
				{
					pModule->channels &= 0x8F ;
					TelemetryType = TEL_UNKNOWN ;
					MultiSetting.protocol[0] = 0 ;
					MultiSetting.subProtocol[0] = 0 ;
//					MultiSetting.previous = 0 ;
//					MultiSetting.next = 0 ;
//					MultiSetting.timeout = 1 ;
				}
				saveEditColours( attr, DimBackColour ) ;
				displayMultiProtocol( svalue, y, attr ) ;
				restoreEditColours() ;

//  			PUTS_NUM( 25*FW, 0, MultiSetting.previous, 0 ) ;
//  			PUTS_NUM( 30*FW, 0, MultiSetting.next, 0 ) ;

//				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
		
			if(t_pgOfs<=subN)
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				uint8_t x = (pModule->sub_protocol & 0x3F ) | (pModule->exsub_protocol << 6 ) ;
				drawItem( ( x == M_SCANNER ) ? (char *)XPSTR("Display Scan") : (char *)PSTR(STR_MULTI_TYPE), y, attr ) ;

				if ( x == M_SCANNER )
				{
					if(sub==subN)
					{
						lcd_char_inverse( 0, y, 12*FW, 0 ) ;
						if ( checkForMenuEncoderBreak( event ) )
						{
							pushMenu(menuScanDisplay) ;
						}
					}
				}
				else
				{
					attr = (pModule->channels >> 4) &0x07 ;
					uint8_t oldValue = attr ;

					if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( attr, 7 ) ;
					if ( attr != oldValue )
					{
						MultiSetting.subProtocol[0] = 0 ;
					}
					saveEditColours( sub==subN, DimBackColour ) ;
					attr = displayMultiSubProtocol( x, attr, y, 0 ) ;
					restoreEditColours() ;
					pModule->channels = ( attr << 4) + (pModule->channels & 0x8F);
				}
//				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
		
			if(t_pgOfs<=subN)
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				drawItem( (char *)PSTR(STR_MULTI_AUTO), y, attr ) ;
				uint8_t value = (pModule->sub_protocol>>6)&0x01 ;
				drawIdxText( y, (char *)XPSTR(M_NY_STR), value, attr ) ;
				if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0(value, 1 );
					pModule->sub_protocol = (value<<6) + (pModule->sub_protocol&0xBF);
				}
//				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;

			if(t_pgOfs<=subN)
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				uint32_t x = (pModule->sub_protocol & 0x3F ) | (pModule->exsub_protocol << 6 ) ;
				multiOptionTouch( 21*FW, y, pModule->option_protocol, (sub==subN ? INVERS:0), x ) ;
				if(attr)
				{
					CHECK_INCDEC_H_MODELVAR(pModule->option_protocol, -128, 127);
				}
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
		 
			if(t_pgOfs<=subN)
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				drawItem( (char *)XPSTR("Power"), y, attr ) ;
				// Power stored in ppmNCH bit7 & Option stored in option_protocol
				uint8_t value = (pModule->channels>>7)&0x01 ;
				drawIdxText( y, (char *)XPSTR(M_LH_STR), value, attr ) ;
				if(attr)
				{
					uint8_t oldValue = value ;
					CHECK_INCDEC_H_MODELVAR_0(value, 1 );
					pModule->channels = (value<<7) + (pModule->channels&0x7F);
					if ( oldValue != value )
					{
						checkMultiPower() ;
						killEvents(event) ;
					}
				}
//				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
			
			if(t_pgOfs<=subN)
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				drawItem( (char *)XPSTR("Rate (mS)"), y, attr ) ;
				// Power stored in ppmNCH bit7 & Option stored in option_protocol
				drawIdxText( y, (char *)XPSTR("\002 7 8 91011"), pModule->ppmFrameLength, attr ) ;
				if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( pModule->ppmFrameLength, 4);
#if defined(PCBT16)
					uint32_t rate = (pModule->ppmFrameLength + 7) * 1000 ;
extern void setIntMultiRate( uint32_t rate ) ;		// in uS
					setIntMultiRate( rate ) ;		// in uS
#endif
				}
//				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
			
			if(t_pgOfs<=subN)
			{
				pModule->multiDisableTelemetry = touchOnOffItem( pModule->multiDisableTelemetry, y, XPSTR("Disable Telemetry"), (sub == subN ), DimBackColour ) ;
//				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
			if(t_pgOfs<=subN)
			{
				lcd_hline( 0, y+TFH, TRIGHT ) ;
				attr = 0 ;
				if ( sub==subN )
				{
					attr = INVERS ;
					lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
				}
  			PUTS_ATT_COLOUR( THOFF, y+TVOFF, XPSTR("Failsafe"), 0, attr ? ~LcdForeground : LcdForeground ) ;
				if ( ( PrivateData[1] && ( ( PrivateData[0] & 0x20 ) == 0 ) ) )
				{
	  			PUTS_ATT_COLOUR( 9*FW, y+TVOFF, XPSTR("N/A"), BLINK, attr ? ~LcdForeground : LcdForeground ) ;
				}
				else
				{
					if ( pModule->failsafeMode == 0 )
					{
	  				PUTS_ATT_COLOUR( 9*FW, y+TVOFF, XPSTR("(Not Set)"), 0, attr ? ~LcdForeground : LcdForeground ) ;
					}
					if ( sub == subN )
					{
						if ( checkForMenuEncoderLong( event ) )
						{
							s_currIdx = module ;
    				  pushMenu( menuSetFailsafe ) ;
						}
					}
				}
//			  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
		}

		if ( pModule->protocol == PROTO_DSM2 )
		{
			if(t_pgOfs<=subN)
  		{
				attr = ( sub==subN ) ? INVERS : 0 ;
				drawItem( (char *)PSTR(STR_DSM_TYPE), y, attr ) ;
  			int8_t x ;
  		  x = pModule->sub_protocol ;
				drawIdxText( y, (char *)XPSTR(DSM2_STR), x, attr ) ;
				if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( pModule->sub_protocol,3);

				if (pModule->sub_protocol == DSM_9XR)
				{
					if ( ( pModule->channels < 6 ) || ( pModule->channels > 14 ) )
					{
						pModule->channels = 6 ;		// Correct if wrong from DSM
					}
				}
//  		  if((y+=FH)>(SCREEN_LINES-1)*FH) return;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
  		}
			subN += 1 ;
		}

		if ( pModule->protocol == PROTO_PPM )
		{
			if(t_pgOfs<=subN)
  		{
				attr = ( sub==subN ) ? INVERS : 0 ;
				drawItem( (char *)PSTR(STR_PPMFRAME_MSEC), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, (int16_t)pModule->ppmFrameLength*5 + 225, attr|PREC1 ) ;
 	  	  if ( attr )
				{
					CHECK_INCDEC_H_MODELVAR( pModule->ppmFrameLength, -20, 20 ) ;
				}
//  		  if((y+=FH)>(SCREEN_LINES-1)*FH) return;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
  		}
			subN += 1 ;

			if(t_pgOfs<=subN)
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				drawItem( (char *)PSTR(STR_SHIFT_SEL), y, attr ) ;
				drawIdxText( y, (char *)PSTR(STR_POS_NEG), pModule->pulsePol, attr ) ;
				if ( attr )
				{
					CHECK_INCDEC_H_MODELVAR_0( pModule->pulsePol, 1 ) ;
				}
//				  if((y+=FH)>(SCREEN_LINES-1)*FH) return;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
		}

		if (pModule->protocol == PROTO_PXX)
		{
			if(t_pgOfs<=subN)
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				drawItem( (char *)PSTR(STR_TYPE)+1, y, attr ) ;
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19) || defined(PCBX7ACCESS)
extern void set_ext_serial_baudrate( uint32_t baudrate ) ;
				uint32_t oldSubProtocol = pModule->sub_protocol ;
#endif
				drawIdxText( y, (char *)XPSTR("\006D16(X)D8(D) LRP   R9M   "), pModule->sub_protocol, attr ) ;
				if ( attr )
				{
					CHECK_INCDEC_H_MODELVAR_0( pModule->sub_protocol, 3 ) ;
				}
//				pModule->sub_protocol = checkIndexed( y, XPSTR(FWx10"\003""\006D16(X)D8(D) LRP   R9M   "), pModule->sub_protocol, (sub==subN) ) ;
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19) || defined(PCBX7ACCESS)
				if ( pModule->sub_protocol != oldSubProtocol )
				{
					if ( oldSubProtocol == 3 )
					{
						set_ext_serial_baudrate( 450000 ) ;
					}
					else
					{
						if ( pModule->sub_protocol == 3 )
						{
							set_ext_serial_baudrate( 420000 ) ;
						}
					}
				}
#endif
//			  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
			if(t_pgOfs<=subN)
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				drawItem( (char *)XPSTR("Chans"), y, attr ) ;
				drawIdxText( y, (char *)XPSTR("\00216 8"), pModule->channels, attr ) ;
				if ( attr )
				{
					CHECK_INCDEC_H_MODELVAR_0( pModule->channels, 1 ) ;
				}
//			  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
			if ( pModule->sub_protocol == 3 )	// R9M
			{
				if(t_pgOfs<=subN)
				{
					uint32_t temp ;
					attr = ( sub==subN ) ? INVERS : 0 ;
					temp = pModule->r9MflexMode ;
					drawItem( (char *)XPSTR("Flex mode"), y, attr ) ;
					drawIdxText( y, (char *)XPSTR("\006   OFF915MHz868MHz"), temp, attr ) ;
					if ( attr )
					{
						CHECK_INCDEC_H_MODELVAR( temp, 0, 2 ) ;
					}
					if ( temp )
					{
						if ( temp != pModule->r9MflexMode )
						{
							AlertType = ALERT_TYPE ;
							AlertMessage = "\001Requires R9M non-\037\001certified firmware\037\001Check Frequency for\037\006your region" ;
						}
					}
//					if ( pModule->country == 2 )
//					{
//						if ( temp == 1 )
//						{
//							if ( pModule->r9MflexMode == 2 )
//							{
//								temp = 0 ;
//							}
//							else
//							{
//								temp = 2 ;
//							}
//						}
//					}
					pModule->r9MflexMode = temp ;
//					if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
					if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
				}
				subN += 1 ;
					
				if(t_pgOfs<=subN)
				{
					attr = ( sub==subN ) ? INVERS : 0 ;
					char *s ; 
					s = XPSTR("\004  10 100 5001000") ;
					if ( ( pModule->country == 2 ) && (pModule->r9MflexMode == 0) )
					{
						s = XPSTR("\011  25(8ch) 25(16ch)200(16ch)500(16ch)") ;
					}
					drawItem( (char *)XPSTR("Power (mW)"), y, attr ) ;
					drawIdxText( y, (char *)s, pModule->r9mPower, attr ) ;
					if ( attr )
					{
						CHECK_INCDEC_H_MODELVAR( pModule->r9mPower, 0, 3 ) ;
					}
					if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
				}
				subN += 1 ;
			}

			if ( pModule->sub_protocol == 0 )	// D16
			{
				if(t_pgOfs<=subN)
				{
					pModule->pxxDoubleRate = touchOnOffItem( pModule->pxxDoubleRate, y, XPSTR("Double Rate"), (sub == subN ), DimBackColour ) ;
					if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
				}
				subN += 1 ;
				
#ifdef ALLOW_EXTERNAL_ANTENNA
				if ( module == 0 )
				{
					if(t_pgOfs<=subN)
					{
						pModule->externalAntenna = touchOnOffItem( pModule->externalAntenna, y, XPSTR("Use External Ant."), (sub == subN ), DimBackColour ) ;
//					  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
						if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
					}
					subN += 1 ;
				}
#endif
			}

			if(t_pgOfs<=subN)
			{
				lcd_hline( 0, y+TFH, TRIGHT ) ;
				attr = 0 ;
				if ( sub==subN )
				{
					attr = INVERS ;
					lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
				}
  			PUTS_ATT_COLOUR( THOFF, y+TVOFF, XPSTR("Failsafe"), 0, attr ? ~LcdForeground : LcdForeground ) ;
				if ( pModule->failsafeMode == 0 )
				{
	  			PUTS_ATT_COLOUR( 9*FW, y+TVOFF, XPSTR("(Not Set)"), 0, attr ? ~LcdForeground : LcdForeground ) ;
				}
				if ( attr )
				{
					if ( checkForMenuEncoderLong( event ) )
					{
						s_currIdx = module ;
    			  pushMenu( menuSetFailsafe ) ;
					}
				}
//			  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
			
			if(t_pgOfs<=subN)
			{
				attr = ( sub==subN ) ? INVERS : 0 ;
				drawItem( (char *)PSTR(STR_COUNTRY)+1, y, attr ) ;
				drawIdxText( y, (char *)XPSTR("\003AmeJapEur"), pModule->country, attr ) ;
				if ( attr )
				{
					CHECK_INCDEC_H_MODELVAR_0( pModule->country, 2 ) ;
				}
//			  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
		}

		if(t_pgOfs<=subN)
		{
			attr = ( sub==subN ) ? INVERS : 0 ;
			drawItem( (char *)PSTR(STR_PPM_1ST_CHAN), y, attr ) ;
			drawNumber( TRIGHT-TRMARGIN, y, pModule->startChannel + 1, attr ) ;
 			if ( attr )
			{
				CHECK_INCDEC_H_MODELVAR_0( pModule->startChannel, 16 ) ;
			}
//			if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
		}
		subN += 1 ;

		uint8_t brNeeded = need_bind_range ;
		if ( brNeeded & 1 )
		{
			if(t_pgOfs<=subN)
			{
				lcd_hline( 0, y+TFH, TRIGHT ) ;
				attr = ( sub==subN ) ? INVERS : 0 ;
				if ( attr )
				{
					lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
				}
  			PUTS_ATT_COLOUR( THOFF, y+TVOFF, PSTR(STR_BIND), 0, attr ? ~LcdForeground : LcdForeground ) ;
				if ( (pModule->protocol == PROTO_MULTI ) && ( PrivateData[1] ) )
				{
					div_t qr ;
					saveEditColours( attr, DimBackColour ) ;
					PUTS_ATT_LEFT( y+TVOFF, XPSTR("\006(Ver\014.\016.\021.\024)" ) ) ;

					PUTC( 11*FW, y+TVOFF, PrivateData[1] + '0' ) ;
					PUTC( 13*FW, y+TVOFF, PrivateData[2] + '0' ) ;
					qr = div( PrivateData[3], 10 ) ;
					PUTC( 16*FW, y+TVOFF, qr.rem + '0' ) ;
					if ( qr.quot )
					{
						PUTC( 15*FW, y+TVOFF, qr.quot + '0' ) ;
					}
					qr = div( PrivateData[4], 10 ) ;
					PUTC( 19*FW, y+TVOFF, qr.rem + '0' ) ;
					if ( qr.quot )
					{
						PUTC( 18*FW, y+TVOFF, qr.quot + '0' ) ;
					}
					if ( PrivateData[0] & 0x08 )
					{
						lcd_char_inverse( 0*FW, y+TVOFF, 4*FW, 1 ) ;
					}
					restoreEditColours() ;
				}
		  	if ( ( pModule->protocol == PROTO_DSM2) && ( pModule->sub_protocol == DSM_9XR ) )
				{
					// Display mode
					saveEditColours( attr, DimBackColour ) ;
					PUTS_ATT_LEFT( y+TVOFF, XPSTR("\006(Mode\015)" ) ) ;
					PUTC( 12*FW, y+TVOFF, (g_model.dsmMode & 0x07) + '0' ) ;
					restoreEditColours() ;
				}

				// XJT version display here
				if ( pModule->protocol == PROTO_PXX )
				{
					saveEditColours( attr, DimBackColour ) ;
					PUTS_ATT_LEFT( y+TVOFF, XPSTR("\006(Ver\020)" ) ) ;
extern uint16_t XjtVersion ;
					PUTS_NUM(  14*FW, y+TVOFF, XjtVersion, 0 ) ;
					restoreEditColours() ;
				}
  		  if(attr)
				{
//					lcd_char_inverse( 0, y, 4*FW, 0 ) ;
					if ( checkForMenuEncoderLong( event ) )
					{
						s_currIdx = module ;
						uint8_t subp = pModule->sub_protocol & 0x3F ;
						subp |= pModule->exsub_protocol << 6 ;
						if ( ( pModule->protocol == PROTO_PXX ) || ( ( pModule->protocol == PROTO_MULTI ) && (( subp == M_FRSKYX ) || ( subp == M_FRSKYX2 ) || ( subp == M_FRSKYR9 )) ) )
						{
							pushMenu( menuBindOptions ) ;
						}
						else
						{
	  					BindRangeFlag[module] = PXX_BIND ;		    	//send bind code
							pushMenu( menuRangeBind ) ;
						}
					}
				}
//				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
		}
		if ( brNeeded & 4 )
		{
			if(t_pgOfs<=subN)
			{
				lcd_hline( 0, y+TFH, TRIGHT ) ;
				attr = ( sub==subN ) ? INVERS : 0 ;
				if ( attr )
				{
					lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
				}
  			PUTS_ATT_COLOUR( THOFF, y+TVOFF, PSTR(STR_RANGE), 0, attr ? ~LcdForeground : LcdForeground ) ;
				if ( (pModule->protocol == PROTO_MULTI ) && ( PrivateData[1] ) )
				{
					uint8_t sp = pModule->sub_protocol & 0x3F ;
					sp |= pModule->exsub_protocol << 6 ;
					if ( ( sp == M_FRSKYX ) || ( sp == M_FrskyD ) || ( sp == M_FRSKYX2 ) )
					{
						saveEditColours( attr, DimBackColour ) ;
						PUTS_P( TRIGHT-TRMARGIN-7*FW, y+TVOFF, XPSTR("Lqi") ) ;
						restoreEditColours() ;
						drawNumber( TRIGHT-TRMARGIN, y, TxLqi, attr ) ;
					}
				}
  		  if( attr )
				{
//					lcd_char_inverse( 0, y, 11*FW, 0 ) ;
					if ( checkForMenuEncoderLong( event ) )
					{
  		  	  BindRangeFlag[module] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
						s_currIdx = module ;
						pushMenu(menuRangeBind) ;
					}
				}
//				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
			}
			subN += 1 ;
		}
	} 
}


//void displayModuleName( uint16_t x, uint16_t y, uint8_t index, uint8_t attr ) ;
void menuProcTrainProtocol(uint8_t event) ;
//extern uint8_t EditingModule ;
//void editOneProtocol( uint8_t event ) ;
void displayProtocol( uint16_t x, uint16_t y, uint8_t value, uint8_t attr ) ;



void menuProcProtocol(uint8_t event)
{
	uint32_t selected = 0 ;
	uint8_t dataItems = 3 ;
	uint8_t need_range = 0 ;
	uint32_t i ;

	switch ( g_model.Module[0].protocol )
	{
		case PROTO_PXX :
		case PROTO_MULTI :
		case PROTO_DSM2 :
			need_range = 1 ;
			dataItems += 1 ;
		break ;
	}

	switch ( g_model.Module[1].protocol )
	{
		case PROTO_PXX :
		case PROTO_MULTI :
		case PROTO_DSM2 :
			need_range |= 2 ;
			dataItems += 1 ;
		break ;
	}
	
	TITLE( XPSTR("Protocol") ) ;
	static MState2 mstate2 ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	 
	event = mstate2.check_columns( event, dataItems-1 ) ;

	int8_t  sub    = mstate2.m_posVert ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( dataItems, 0, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;

	if ( handleSelectIcon() || selected )
	{
		selected = 1 ;
		if ( event == 0 )
		{
			event = EVT_KEY_BREAK(BTN_RE) ;
		}
	}
#endif
	 
	coord_t y = TTOP ;
	uint8_t subN = 0 ;

	lcd_hline( 0, TTOP, TRIGHT ) ;
	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;
	if ( sub == subN )
	{
		lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
		LcdForeground = ~LcdForeground ;
		LcdBackground = ~LcdBackground ;
	}
	PUTS_ATT( THOFF, y+TVOFF, PSTR(STR_Trainer), 0 ) ;
	LcdForeground = oldFcolour ;
	LcdBackground = oldBcolour ;
	
	lcd_hline( 0, y+TFH, TRIGHT ) ;
  y += TFH ;
	subN += 1 ;

	for ( i = 0 ; i < 2 ; i += 1 )
	{
		if ( sub == subN )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdForeground = ~LcdForeground ;
			LcdBackground = ~LcdBackground ;
		}
		displayModuleName( THOFF, y+TVOFF, i, 0 ) ;
		displayProtocol( THOFF+10*FW, y+TVOFF, g_model.Module[i].protocol, 0 ) ;
		LcdForeground = oldFcolour ;
		LcdBackground = oldBcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
  	y += TFH ;
		subN += 1 ;
	}
		
	if ( sub <= 2 )
	{
  	switch (event)
		{
  		case EVT_KEY_FIRST(KEY_MENU) :
  		case EVT_KEY_BREAK(BTN_RE) :
				killEvents(event) ;
				switch ( sub )
				{
					case 0 :
						pushMenu(menuProcTrainProtocol) ;
					break ;
					case 1 :
						EditingModule = 0 ;
						pushMenu( editOneProtocol ) ;
					break ;
					case 2 :
						EditingModule = 1 ;
						pushMenu( editOneProtocol ) ;
					break ;
				}

			break ;
		}
	}

	for ( i = 1 ; i < 4 ; i <<= 1 )
	{
		if ( need_range & i )
		{
  		if(sub==subN)
			{
				lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
				LcdForeground = ~LcdForeground ;
				LcdBackground = ~LcdBackground ;
			}	
			PUTS_P( THOFF, y+TVOFF, PSTR(STR_RANGE) ) ;
			displayModuleName( 12*FW+THOFF, y+TVOFF, i > 1, 0 ) ;
			LcdForeground = oldFcolour ;
			LcdBackground = oldBcolour ;
  		if(sub==subN)
			{
				if ( ( checkForMenuEncoderLong( event ) ) || selected )
				{
					uint8_t module = 0 ;
					if ( i & 2 )
					{
						module = 1 ;
					}
  		  	BindRangeFlag[module] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
					s_currIdx = module ;
					pushMenu(menuRangeBind) ;
				}
			}
			lcd_hline( 0, y+TFH, TRIGHT ) ;
			y += TFH ;
			subN += 1 ;
		}
	}

//			if ( need_range & 1 )
//			{
////				if(t_pgOfs<=subN)
////				{
//				PUTS_ATT_LEFT( y, PSTR(STR_RANGE) ) ;
//				PUTS_ATT_LEFT( y, "\014Internal") ;
//  		  if(sub==subN)
//				{
//					lcd_char_inverse( 0, y, 11*FW, 0 ) ;
//					if ( checkForMenuEncoderLong( event ) )
//					{
//  		  	  BindRangeFlag[0] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
//						s_currIdx = 0 ;
//						pushMenu(menuRangeBind) ;
//					}
//				}
//				y+=FH ;
//				subN += 1 ;
//			}
//			if ( need_range & 2 )
//			{
////				if(t_pgOfs<=subN)
////				{
//				PUTS_ATT_LEFT( y, PSTR(STR_RANGE) ) ;
//				PUTS_ATT_LEFT( y, "\014External") ;
//  		  if(sub==subN)
//				{
//					lcd_char_inverse( 0, y, 11*FW, 0 ) ;
//					if ( checkForMenuEncoderLong( event ) )
//					{
//  		  	  BindRangeFlag[1] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
//						s_currIdx = 1 ;
//						pushMenu(menuRangeBind) ;
//					}
//				}
////				y+=FH ;
////				subN += 1 ;
//			}

//		}
//	} // module for loop
}



void menuModelMusic(uint8_t event)
{
	uint8_t subN = 0 ;
 	TITLE( PSTR( STR_Music ) ) ;
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif
	uint8_t attr = 0 ;
	EditType = EE_MODEL ;
	uint16_t y = TTOP ;
	static MState2 mstate2;
	uint16_t colour = dimBackColour() ;
	SuppressStatusLine = 1 ;
	lcdDrawSolidFilledRectDMA( 0, STATUS_VERTICAL, 480, 30, LcdBackground ) ;

	
	uint32_t rows = 8 ;
	if ( MusicPlaying == MUSIC_STOPPED )
	{
		rows = 8 ;
	}
	if ( ( MusicPlaying == MUSIC_PLAYING ) || ( MusicPlaying == MUSIC_PAUSED ) )
	{
		rows = 9 ;
	}
	
	mstate2.check_columns(event, rows-1) ;
  int8_t sub = mstate2.m_posVert ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	if ( newSelection >= 0 )
	{
		if ( sub == newSelection )
		{
			if ( sub == 7 )
			{
				if ( MusicPlaying == MUSIC_STOPPED )
				{
					MusicPlaying = MUSIC_STARTING ;
				}
				else
				{
					MusicPlaying = MUSIC_STOPPING ;
				}
			}
			else if ( sub == 8 )
			{
				if ( MusicPlaying == MUSIC_PAUSED )
				{
					MusicPlaying = MUSIC_RESUMING ;
				}
				else
				{
					MusicPlaying = MUSIC_PAUSING ;
				}
			}
		}
		else
		{
			sub = mstate2.m_posVert = newSelection ;
		}
	}
#endif

	if ( event == EVT_ENTRY_UP )
	{ // From menuProcSelectUvoiceFile
		if ( FileSelectResult == 1 )
		{
			copyFileName( (char *)g_eeGeneral.musicVoiceFileName, SelectedVoiceFileName, MUSIC_NAME_LENGTH ) ;
			g_eeGeneral.musicVoiceFileName[MUSIC_NAME_LENGTH] = '\0' ;

			if ( g_eeGeneral.musicType )
			{
				// load playlist
				cpystr( cpystr( (uint8_t *)PlaylistDirectory, (uint8_t *)"\\music\\" ), g_eeGeneral.musicVoiceFileName ) ;
				fillPlaylist( PlaylistDirectory, &PlayFileControl, (char *)"WAV" ) ;
				g_eeGeneral.playListIndex = PlaylistIndex = 0 ;
			}
			STORE_GENERALVARS ;
		}
	}
			 
//	DisplayOffset = MUS_OFF_0 ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Start Switch"), y, ( attr ) ) ;
	uint8_t doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
	saveEditColours( attr, colour ) ;
	g_model.musicData.musicStartSwitch = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, g_model.musicData.musicStartSwitch, 0, doedit, event ) ;
	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;
	
	attr = 0 ;
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Pause Switch"), y, ( attr ) ) ;
	doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
	saveEditColours( attr, colour ) ;
	g_model.musicData.musicPauseSwitch = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, g_model.musicData.musicPauseSwitch, 0, doedit, event ) ;
	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;
	
	attr = 0 ;
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Previous Switch"), y, ( attr ) ) ;
	doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
	saveEditColours( attr, colour ) ;
	g_model.musicData.musicPrevSwitch = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, g_model.musicData.musicPrevSwitch, 0, doedit, event ) ;
	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;
	
	attr = 0 ;
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Next Switch"), y, ( attr ) ) ;
	doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
	saveEditColours( attr, colour ) ;
	g_model.musicData.musicNextSwitch = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, g_model.musicData.musicNextSwitch, 0, doedit, event ) ;
	restoreEditColours() ;

	
	EditType = EE_GENERAL ;


  if ( ( event == EVT_KEY_FIRST(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
	{
		if ( sub > 3 )
		{
			killEvents(event) ;
			s_editMode = 0 ;
			event = 0 ;
		}
		switch ( sub )
		{
			case 5 :
				VoiceFileType = VOICE_FILE_TYPE_MUSIC ;
      	pushMenu( menuProcSelectVoiceFile ) ;
			break ;
			case 7 :
				if ( MusicPlaying == MUSIC_STOPPED )
				{
					MusicPlaying = MUSIC_STARTING ;
				}
				else
				{
					MusicPlaying = MUSIC_STOPPING ;
				}
			break ;

			case 8 :
				if ( MusicPlaying == MUSIC_PAUSED )
				{
					MusicPlaying = MUSIC_RESUMING ;
				}
				else
				{
					MusicPlaying = MUSIC_PAUSING ;
				}
			break ;
		}
	}
	if ( g_eeGeneral.musicType )
	{
		if ( MusicPlaying == MUSIC_PLAYING )
		{
			if ( sub == 8 )
			{
			  if ( event == EVT_KEY_FIRST(KEY_LEFT) )
				{
					killEvents(event) ;
					MusicPrevNext = MUSIC_NP_PREV ;
				}
			  if ( event == EVT_KEY_FIRST(KEY_RIGHT) )
				{
					killEvents(event) ;
					MusicPrevNext = MUSIC_NP_NEXT ;
				}
			}
		}
	}

	y += TFH ;
	subN += 1 ;

	attr = 0 ;
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Type"), y, ( attr ) ) ;
	uint32_t b = g_eeGeneral.musicType ;
	drawIdxText( y, XPSTR("\004NameList"), b, attr|LUA_RIGHT ) ; //0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
  if ( attr )
	{
		g_eeGeneral.musicType = checkIncDec16( b, 0, 1, EditType ) ;
	}
	if ( g_eeGeneral.musicType != b )
	{
		g_eeGeneral.musicVoiceFileName[0] = '\0' ;
	}
	y += TFH ;
	subN += 1 ;

	attr = 0 ;
  if ( sub==subN )
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("File"), y, ( attr ) ) ;
	lcdDrawSolidFilledRectDMA( (TMID-4*FW)*TSCALE, y*TSCALE+2, 4*FW*TSCALE, TFH*TSCALE-2, ( attr ) ? ~colour : colour ) ;

	PUTS_ATT_COLOUR( TRIGHT-TRMARGIN-MUSIC_NAME_LENGTH*FW, y+TVOFF, (char *)g_eeGeneral.musicVoiceFileName, 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
#ifdef TOUCH	
	if (attr)
	{
//		lcdHiresRect( (TRIGHT-TRMARGIN-MUSIC_NAME_LENGTH*FW)*TSCALE-1, (y+TVOFF)*TSCALE-1, MUSIC_NAME_LENGTH*FW*TSCALE+2, 17, LCD_BLACK ) ;
		if ( handleSelectIcon() )
		{
			VoiceFileType = VOICE_FILE_TYPE_MUSIC ;
     	pushMenu( menuProcSelectVoiceFile ) ;
		}
	}
#endif
	y += TFH ;
	subN += 1 ;

  g_eeGeneral.musicLoop = touchOnOffItem( g_eeGeneral.musicLoop, y, XPSTR("Loop"), (sub == subN ), colour ) ;

	y += TFH ;
	subN += 1 ;
	
	attr = 0 ;
  if(sub==subN)
	{
	  attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Play"), y, ( attr ) ) ;
	if ( MusicPlaying == MUSIC_STOPPED )
	{
		PUTS_ATT_COLOUR( TRIGHT-TRMARGIN-5*FW, y+TVOFF, (char *)XPSTR("Start"), 0, attr ? ~LcdForeground : LcdForeground) ; //, attr ? ~colour : colour ) ;
	}
	else
	{
		PUTS_ATT_COLOUR( TRIGHT-TRMARGIN-4*FW, y+TVOFF, (char *)XPSTR("Stop"), 0, attr ? ~LcdForeground : LcdForeground) ; //, attr ? ~colour : colour ) ;
		y += TFH ;
		subN += 1 ;

		attr = 0 ;
  	if(sub==subN)
		{
		  attr = INVERS ;
		}
		drawItem( (char *)XPSTR(""), y, ( attr ) ) ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;	// Blanks line
		PUTS_ATT_COLOUR( TRIGHT-TRMARGIN, y+TVOFF, (char *)(MusicPlaying == MUSIC_PAUSED ? XPSTR("Resume") : XPSTR(" Pause")), LUA_RIGHT, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
		if ( g_eeGeneral.musicType )
		{
			if ( MusicPlaying == MUSIC_PLAYING )
			{
				PUTS_P( TRIGHT+FW, y+TVOFF, XPSTR("Next")) ;
				PUTS_P( TRIGHT+FW, y-TFH+TVOFF, XPSTR("Prev")) ;
#ifdef TOUCH	
				if ( checkTouchposition( (TRIGHT+FW)*TSCALE, (y-TFH)*TSCALE, LCD_W-1, y*TSCALE ) )
				{
					MusicPrevNext = MUSIC_NP_PREV ;
				}
				if ( checkTouchposition( (TRIGHT+FW)*TSCALE, y*TSCALE, LCD_W-1, (y+TFH)*TSCALE ) )
				{
					MusicPrevNext = MUSIC_NP_NEXT ;
				}
#endif
			}
		}
		PUTS_P( FW, 14*FH, CurrentPlayName ) ;
	}

	uint32_t a ;
	a = BgTotalSize ;
	b = BgSizePlayed ;
	if ( a > 14000000 )
	{
		a /= 64 ;
		b /= 64 ;
	}
	uint16_t percent = ( a - b) * 300 / a ;
	uint16_t x = 20*TSCALE ;
	uint16_t yb = 15*FH*TSCALE+6 ;
	uint16_t w = 401 ;
	uint16_t h = 14 ;
	uint16_t solid ;
	if ( percent > 300 )
	{
		percent = 300 ;
	}
	solid = (w-2) * percent / 300 ;
//	lcd_rect( x, yb, w, h ) ;
	lcdHiresRect( x, yb, w, h, LCD_BLACK ) ;

	if ( MusicPlaying != MUSIC_STOPPED )
	{
		if ( solid )
		{
			pushPlotType( PLOT_BLACK ) ;
			w = yb + h - 1 ;
			yb += 1 ;
			x += 1 ;
			while ( yb < w )
			{
 				lcd_hline(x/TSCALE, yb/TSCALE, solid/TSCALE ) ;
				yb += 1 ;			
			}
			popPlotType() ;
		}
	}
	
//	uint16_t percent = (BgTotalSize - BgSizePlayed) * 200 / BgTotalSize ;
//	uint16_t x = 20 ;
//	uint16_t yb = 13*FH ;
//	uint16_t w = 201 ;
//	uint16_t h = 9 ;
//	uint16_t solid ;
//	if ( percent > 200 )
//	{
//		percent = 200 ;
//	}
//	solid = (w-2) * percent / 200 ;
//	lcd_rect( x, yb, w, h ) ;

//	if ( solid )
//	{
//		w = yb + h - 1 ;
//		yb += 1 ;
//		x += 1 ;
//		while ( yb < w )
//		{
// 			lcd_hline(x, yb, solid ) ;
//			yb += 1 ;			
//		}
//	}
}



void menuOneGvar(uint8_t event)
{
	TITLE(PSTR(STR_GLOBAL_VARS)) ;
	EditType = EE_MODEL ;
	static MState2 mstate2;
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif
	uint32_t rows = 3 ;	
	GvarData *pgvar ;
//#if MULTI_GVARS
//	if ( g_model.flightModeGvars )
//	{
//		rows = 6 ;
//	}
	
//#endif
	 
	event = mstate2.check_columns(event, rows - 1 ) ;

	uint8_t sub = mstate2.m_posVert ;

	PUTC( 20*FW, 0, s_currIdx+ ((s_currIdx > 8) ? '8' : '1') ) ;
	uint16_t colour = dimBackColour() ;
//#if MULTI_GVARS
//	uint16_t oldBcolour = LcdBackground ;
//#endif

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

	lcd_hline( 0, TTOP, TRIGHT ) ;

	pgvar = &g_model.gvars[s_currIdx] ;
//	GVarXData *pgxvar = &g_model.xgvars[s_currIdx] ;

	for ( uint32_t i = 0 ; i < rows ; i += 1 )
	{
    uint16_t y = i * TFH + TTOP ;
    uint16_t attr = (sub==i) ? INVERS : 0 ;
		
		switch(i)
		{
			case 0 : // switch
//#if MULTI_GVARS
//				if ( g_model.flightModeGvars )
//				{
//					uint8_t *psource ;
//					drawItem( (char *)XPSTR("Source"), y, attr ) ;
//					psource = ( ( (uint8_t*)&g_model.gvars) + s_currIdx ) ;
//					drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), *psource, attr ) ;
//					// STR_GV_SOURCE
// 					if(attr)
//					{ 
//						CHECK_INCDEC_H_MODELVAR( *psource, 0, 69+NUM_RADIO_VARS ) ;
//					}
//				}
//				else
//#endif
				{
					drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
					if ( attr & INVERS )
					{
						if ( s_editMode && BLINK_ON_PHASE )
						{
							attr = 0 ;
						}
					}
					saveEditColours( attr, colour ) ;
					g_model.gvswitch[s_currIdx] = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, g_model.gvswitch[s_currIdx], LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
					restoreEditColours() ;
				}
			break ;
			case 1 :
//#if MULTI_GVARS
//				if ( g_model.flightModeGvars )
//				{
//					drawItem( (char *)XPSTR("Unit"), y, attr ) ;
//					drawChar( TRIGHT-TRMARGIN-FW, y, pgxvar->unit ? '%' : '-', attr, attr ? ~colour : colour ) ;
//					if ( attr )
//					{
//						CHECK_INCDEC_H_MODELVAR_0( pgxvar->unit, 1 ) ;
//					}
//				}
//				else
//#endif
				{
					drawItem( (char *)XPSTR("Source"), y, attr ) ;
					drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), pgvar->gvsource, attr ) ;
					// STR_GV_SOURCE
 					if(attr)
					{ 
						CHECK_INCDEC_H_MODELVAR( pgvar->gvsource, 0, 69+NUM_RADIO_VARS ) ;
					}
				}
			break ;
			case 2 :
//#if MULTI_GVARS
//				if ( g_model.flightModeGvars )
//				{
//					drawItem( (char *)XPSTR("Precision"), y, attr ) ;
//					drawIdxText( y, (char *)XPSTR("\0030.-0.0"), pgxvar->prec, attr ) ;
//		    	if(attr)
//					{
//      		  CHECK_INCDEC_H_MODELVAR_0( pgxvar->prec, 1 ) ;
//					}	
//				}
//				else
//#endif
				{
					drawItem( (char *)XPSTR("Value"), y, attr ) ;
					drawNumber( TRIGHT-TRMARGIN, y, pgvar->gvar, attr ) ;
  				if(attr)
					{
						CHECK_INCDEC_H_MODELVAR( pgvar->gvar, -125, 125 ) ;
					}
				}
			break ;
//#if MULTI_GVARS
//			case 3 :
//				drawItem( (char *)XPSTR("Minimum"), y, attr ) ;
//				drawNumber( TRIGHT-TRMARGIN, y, GVAR_MIN+pgxvar->min, attr ) ;
//				if ( attr )
//				{
//					if ( GVAR_MIN+pgxvar->min > GVAR_MAX-pgxvar->max )
//					{
//						pgxvar->min = pgxvar->max = 0 ;
//					}
//					pgxvar->min = checkIncDec16( GVAR_MIN+pgxvar->min, GVAR_MIN, GVAR_MAX-pgxvar->max, EE_MODEL ) - GVAR_MIN ;
//				}
//			break ;
//			case 4 :
//				drawItem( (char *)XPSTR("Maximum"), y, attr ) ;
//				drawNumber( TRIGHT-TRMARGIN, y, GVAR_MAX-pgxvar->max, attr ) ;
//	    	if(attr)
//				{
//      	  pgxvar->max = GVAR_MAX-checkIncDec16( GVAR_MAX-pgxvar->max, GVAR_MIN+pgxvar->min, GVAR_MAX, EE_MODEL ) ;
//    		}
//			break ;
//			case 5 :
//				drawItem( (char *)XPSTR("Name"), y, attr ) ;
//				LcdBackground = attr ? ~colour : colour ;
//				alphaEditName( TRIGHT-3*FW, y+TVOFF, &g_model.gvarNames[s_currIdx*3], 3, attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
//				LcdBackground = oldBcolour ;
////				pgxvar->popup = touchOnOffItem( pgxvar->popup, y, XPSTR("Popup"), attr, DimBackColour ) ;
//			break ;
//#endif
		}
	}
}



#ifdef BIG_SCREEN
#define GLO_OFF_0			(8*FW)
#else
#define GLO_OFF_0			0
#endif

void menuProcGlobals(uint8_t event)
{
	TITLE(PSTR(STR_GLOBAL_VARS));
	EditType = EE_MODEL ;
	static MState2 mstate2;
	uint16_t t_pgOfs ;
	uint32_t rows = MAX_GVARS ;
	uint32_t k = 0 ;
#ifdef TOUCH	
	uint32_t selected = 0 ;
	uint32_t newVpos ;
	TlExitIcon = 1 ;
#endif
	 
//#if MULTI_GVARS
//	if ( g_model.flightModeGvars )
//	{
//		rows = 12 ;
//	}
//#endif	
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	event = mstate2.check_columns(event, rows - 1 ) ;

	uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

	t_pgOfs = evalHresOffset( sub ) ;

//	NoCheckSelect = 1 ;
#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

#ifdef TOUCH	
	if ( rows > 9 )
	{
		newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
		if ( newVpos != t_pgOfs )
		{
			s_pgOfs = t_pgOfs = newVpos ;
			if ( sub < t_pgOfs )
			{
				mstate2.m_posVert = sub = t_pgOfs ;
			}
			else if ( sub > t_pgOfs + TLINES - 1 )
			{
				mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
			}
		}
	}
#endif

#ifdef TOUCH	
	if ( handleSelectIcon() || selected || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#else
	if ( event == EVT_KEY_BREAK(BTN_RE) )
#endif
//	if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(BTN_RE) ) )
	{
		s_currIdx = sub ;
		killEvents(event);
		s_editMode = false ;
		pushMenu(menuOneGvar) ;
  }

//	if ( s_editMode == 0 )
//	{
//		if ( subSub < 2 )
//		{
//			if ( handleSelectIcon() )
//			{
//				s_editMode = 1 ;
//			}
//		}
//	}

	uint32_t lines = rows > TLINES ? TLINES : rows ;
	for (uint32_t i=0; i<lines ; i++ )
	{
    y=(i)*TFH +TTOP ;
    k=i+t_pgOfs ;
  	uint8_t attr = ((sub==k) ? INVERS : 0);
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
		}
		saveEditColours( attr, LcdBackground ) ;
  	PUTS_P( THOFF, y+TVOFF, PSTR(STR_GV));
		PUTC( THOFF+2*FW, y+TVOFF, k+ ((k > 8) ? '8' : '1') ) ;
		GvarData *pgvar ;
//#if MULTI_GVARS
//		if ( g_model.flightModeGvars )
//		{
//			attr = 0 ;
//			pgvar = &g_model.gvars[0] ;
//		}
//		else
//		{
//			pgvar = &g_model.gvars[k] ;
//		}
//#else
		pgvar = &g_model.gvars[k] ;
//#endif
//#if MULTI_GVARS
//		if ( g_model.flightModeGvars == 0 )
//#endif
		{
	    putsDrSwitches( 7*FW, y+TVOFF, g_model.gvswitch[k], 0 );
			uint32_t t ;
			uint8_t *psource ;
//			pgvar = &g_model.gvars[k] ;
			psource = &pgvar->gvsource ;
			t = *psource ;
			PUTS_AT_IDX( 14*FW-4, y+TVOFF, PSTR(STR_GV_SOURCE), t, 0 ) ;
			PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF, pgvar->gvar, 0 ) ;
		}
//#if MULTI_GVARS
//		else
//		{
//			uint8_t *psource ;
//			uint32_t t ;
//			psource = ( ( (uint8_t*)&g_model.gvars) + k ) ;
//			t = *psource ;
//			PUTS_AT_IDX( 14*FW-4, y+TVOFF, PSTR(STR_GV_SOURCE), t, 0 ) ;
//			PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF, readMgvar( 0, k ), 0 ) ;
//		}
//#endif
		
		restoreEditColours() ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}
}


#ifdef BIG_SCREEN
#define ADJ_OFF_0			(8*FW)
#else
#define ADJ_OFF_0			0
#endif



const char StringAdjustFunctions[] = "\005-----Add  Set CSet V+/-  Inc/0Dec/0+/Lim-/Lim" ;

void menuOneAdjust(uint8_t event)
{
	TITLE(XPSTR("GVAR Adjust"));
	EditType = EE_MODEL ;
	static MState2 mstate2;
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif
	uint32_t rows = 4 ;
 	event = mstate2.check_columns(event, rows - 1 ) ;

	uint8_t sub = mstate2.m_posVert ;
	GvarAdjust *pgvaradj ;
	pgvaradj = ( s_currIdx >= NUM_GVAR_ADJUST ) ? &g_model.egvarAdjuster[s_currIdx - NUM_GVAR_ADJUST] : &g_model.gvarAdjuster[s_currIdx] ;

 	PUTS_NUM(21*FW, 0, 1+ s_currIdx, 0 ) ;

	uint16_t colour = dimBackColour() ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

	lcd_hline( 0, TTOP, TRIGHT ) ;

	for (uint32_t i = 0 ; i < rows ; i += 1 )
	{
    uint16_t y = i * TFH + TTOP ;
    uint8_t attr = (sub==i) ? INVERS : 0 ;

		switch(i)
		{
			case 0 : // GV
				drawItem( (char *)XPSTR("GVAR"), y, attr ) ;
//#if MULTI_GVARS
//				if ( pgvaradj->gvarIndex > (g_model.flightModeGvars ? 11 : 6) )
//#else
				if ( pgvaradj->gvarIndex > 6 )
//#endif
				{
//#if MULTI_GVARS
//					drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), pgvaradj->gvarIndex-(g_model.flightModeGvars ? 11 : 6), attr ) ;
//#else
					drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), pgvaradj->gvarIndex-6, attr ) ;
//#endif
				}
				else
				{
					drawNumber( TRIGHT-TRMARGIN, y, pgvaradj->gvarIndex+1, attr ) ;
				}
  			if(attr)
				{
//#if MULTI_GVARS
// 	        CHECK_INCDEC_H_MODELVAR( pgvaradj->gvarIndex, 0, (g_model.flightModeGvars ? 15 : 10 ) ) ;
//#else
 	        CHECK_INCDEC_H_MODELVAR( pgvaradj->gvarIndex, 0, 10 ) ;
//#endif
				}
			break ;
			case 1 : // Fn
			{	
				uint32_t old = pgvaradj->function ;
				drawItem( (char *)XPSTR("Function"), y, attr ) ;
				drawIdxText( y, (char *)StringAdjustFunctions, pgvaradj->function, attr ) ;
 				if(attr)
				{ 
					CHECK_INCDEC_H_MODELVAR( pgvaradj->function, 0, 8 ) ;
					if ( pgvaradj->function != old )
					{
						if ( ( old < 4 ) || ( old > 6 ) )
						{
							if ( pgvaradj->function >= 4 )
							{
								pgvaradj->switch_value = 0 ;
							}
						}
						else
						{
							if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
							{
								pgvaradj->switch_value = 0 ;
							}
						}
					}
				}
			}
			break ;
			case 2 : // switch
				drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
				if ( attr & INVERS )
				{
					if ( s_editMode && BLINK_ON_PHASE )
					{
						attr = 0 ;
					}
				}
				saveEditColours( attr, colour ) ;
				pgvaradj->swtch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, pgvaradj->swtch, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				restoreEditColours() ;
			break ;
			case 3 : // Param
				if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
				{
					if ( pgvaradj->function == 3 )
					{
						drawItem( (char *)XPSTR("Source"), y, attr ) ;
						drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), pgvaradj->switch_value, attr ) ;
		  			if(attr)
						{
							CHECK_INCDEC_H_MODELVAR( pgvaradj->switch_value, 0, 69 ) ;
						}
					}
					else
					{
						drawItem( (char *)XPSTR("Value"), y, attr ) ;
						drawNumber( TRIGHT-TRMARGIN, y, pgvaradj->switch_value, attr ) ;
						if ( attr )
						{
 	      	  	CHECK_INCDEC_H_MODELVAR( pgvaradj->switch_value, -125, 125 ) ;
						}
					}
				}
				else
				{
					drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
					if ( attr & INVERS )
					{
						if ( s_editMode && BLINK_ON_PHASE )
						{
							attr = 0 ;
						}
					}
					saveEditColours( attr, colour ) ;
					pgvaradj->switch_value = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, pgvaradj->switch_value, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
					restoreEditColours() ;
				}
			break ;
		}
	}
}



void menuProcAdjust(uint8_t event)
{
	TITLE(XPSTR("GVAR Adjust"));
	EditType = EE_MODEL ;
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif
	uint32_t rows = NUM_GVAR_ADJUST + EXTRA_GVAR_ADJUST ;
	static MState2 mstate2;
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
 	event = mstate2.check_columns(event, rows - 1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	
#ifdef TOUCH	
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif
	uint8_t k = 0 ;
  uint8_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;

//	NoCheckSelect = 1 ;
#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

#ifdef TOUCH	
	if ( handleSelectIcon() || selected || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#else
	if ( event == EVT_KEY_BREAK(BTN_RE) )
#endif
	{
		s_currIdx = sub ;
		killEvents(event);
		s_editMode = false ;
		pushMenu(menuOneAdjust) ;
  }
	
#ifdef TOUCH	
	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	lcd_hline( 0, TTOP, TRIGHT ) ;

	for (uint32_t i=0; i<TLINES; i++ )
	{
		GvarAdjust *pgvaradj ;
		uint8_t idx ;
    y=(i)*TFH +TTOP ;
    k=i+t_pgOfs;
		pgvaradj = ( k >= NUM_GVAR_ADJUST ) ? &g_model.egvarAdjuster[k - NUM_GVAR_ADJUST] : &g_model.gvarAdjuster[k] ;
		idx = pgvaradj->gvarIndex ;
  	PUTC( 0, y+TVOFF, 'A' ) ;
		if ( k < 9 )
		{
  		PUTC( 1*FW, y+TVOFF, k+'1' ) ;
		}
		else
		{
  		PUTC( 1*FW, y+TVOFF, k > 18 ? '2' : '1' ) ;
  		PUTC( 2*FW, y+TVOFF, ( (k+1) % 10) +'0' ) ;
		}

		if ( sub==k )
		{
			int16_t value ;
	 		PUTC( 20*FW, 0, '=' ) ;
			value = 7 ;
//#if MULTI_GVARS
//			if ( g_model.flightModeGvars )
//			{
//				value = 12 ;
//			}
//#endif
			if ( idx >= value )
			{
				value = getTrimValueAdd( CurrentPhase, idx - value  ) ;
				PUTS_AT_IDX( 17*FW, 0, PSTR(STR_GV_SOURCE), idx-(value-1), 0 ) ;
			}
			else
			{
				value = getGvar(idx) ;
				dispGvar( 17*FW, 0, idx+1, 0 ) ;
			}
			PUTS_NUMX( 25*FW, 0, value ) ;
		} 
		uint8_t attr = ((sub==k) ? INVERS : 0);
		
//		uint16_t oldBcolour = LcdBackground ;
		uint16_t oldFcolour = LcdForeground ;
		
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 3*FW*TSCALE, y*TSCALE+2, (TRIGHT-3*FW)*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
//			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}

//		for( uint32_t j = 0 ; j < 4 ; j += 1 )
//		{
//			if ( j == 0 )
//			{
//#if MULTI_GVARS
//				if ( idx > (g_model.flightModeGvars ? 8 : 6) )
//#else
				if ( idx > 6 )
//#endif
				{
//#if MULTI_GVARS
//					lcd_putsAttIdx( 3*FW+2, y, PSTR(STR_GV_SOURCE), idx-(g_model.flightModeGvars ? 8 : 6), attr ) ;
//#else
					PUTS_AT_IDX( 3*FW+2, y+TVOFF, PSTR(STR_GV_SOURCE), idx-6, attr ) ;
//#endif
				}
				else
				{
  				PUTS_P( 3*FW+2, y+TVOFF, XPSTR("GV") ) ;
					PUTC_ATT( 5*FW+2, y+TVOFF, idx+'1', 0 ) ;
				}
//			}
//			else if ( j == 1 )
//			{
				PUTS_AT_IDX( 7*FW, y+TVOFF, StringAdjustFunctions, pgvaradj->function, 0);
//			}
//			else if ( j == 2 )
//			{
      	putsDrSwitches(13*FW-1, y+TVOFF, pgvaradj->swtch, 0);
//			}
//			else
//			{
				if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
				{
					if ( pgvaradj->function == 3 )
					{
						PUTS_AT_IDX( 18*FW, y+TVOFF, PSTR(STR_GV_SOURCE), pgvaradj->switch_value, attr ) ;
					}
					else
					{
						PUTS_NUM( 21*FW, y+TVOFF, pgvaradj->switch_value, 0 ) ;
					}
				}
				else
				{
      		putsDrSwitches(17*FW, y+TVOFF, pgvaradj->switch_value, 0);
				}
//			}
//		}
//		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}
	
}


void menuProcMixOne(uint8_t event)
{
	uint8_t rows = 16 ;
	SKYMixData *md2 = mixAddress( s_curItemIdx ) ;
  
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2 ;
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	mstate2.check_columns(event, rows-1 ) ;
	TITLEP( PSTR(STR_EDIT_MIX));
	uint8_t x = LcdNextPos ;
#ifdef TOUCH	
	uint32_t newVpos ;
#endif
	uint32_t curveFunction ;
	int32_t curveValue ;
	uint32_t diffIsGvar = 0 ;

	uint32_t num_mix_switches = NUM_MIX_SWITCHES ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
	if ( g_eeGeneral.analogMapping & MASK_6POS )
	{
		num_mix_switches += 1 ;
	}
#endif
#if defined(PCBX12D) || defined(PCBX10)
	num_mix_switches += 1 ;
#endif
	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	if ( event == EVT_ENTRY_UP )
	{
		SingleExpoChan = 0 ;
		if ( TextResult )
		{
			md2->srcRaw = unmapMixSource( TextIndex + 1, &md2->switchSource ) ;
    	eeDirty(EE_MODEL) ;
		}
	}
		
	putsChn(x+1*FW,0,md2->destCh,0) ;
	uint8_t sub = mstate2.m_posVert ;

	uint16_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;
#ifndef TOUCH
	(void) t_pgOfs ;
#endif	 

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
	 
//	if ( newSelection >= 0 )
//	{
//		if ( sub == newSelection )
//		{
////			selected = 1 ;
//			event = Tevent = EVT_KEY_BREAK(BTN_RE) ;
//			s_editMode = !s_editMode ;
//		}
//		sub = mstate2.m_posVert = newSelection ;
//	}

#ifdef TOUCH	
	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	curveFunction = md2->differential | (md2->extDiff << 1 ) ;
	curveValue = md2->curve ;

	if ( curveFunction == 0 )
	{
		if ( md2->curve <= -28 )
		{
			curveFunction = 4 ;
// value expo is -128 to -28 as 0 - 100
			curveValue += 128 ;
		}
		else
		{
			if ( ( curveValue > 0 ) && ( curveValue <= 6 ) )
			{
				curveFunction = 1 ;
			}
			else
			{
				if ( curveValue )
				{
					curveFunction = 2 ;
					if ( curveValue > 0 )
					{
						curveValue -= 7 ;
					}
				}
				else
				{
					curveFunction = 0 ;
				}
			}
		}
	}
	else
	{
		if ( curveFunction == 3 )
		{
			diffIsGvar = 1 ;
		}
		else
		{
			if ( curveFunction == 1 )
			{
				diffIsGvar = 0 ;
				curveFunction = 3 ;
			}
		}
	}
	
	lcd_hline( 0, TTOP, TRIGHT ) ;

	for( uint32_t k = 0 ; k < TLINES ; k += 1 )
  {
		coord_t y = TTOP + k*TFH ;
		uint32_t i = k + s_pgOfs ;
    uint32_t attr = sub==i ? INVERS : 0 ;
  	uint32_t b ;

    switch(i)
		{
      case 0:
			{	
				drawItem( (char *)PSTR(STR_2SOURCE)+1, y, attr ) ;
				uint32_t value = md2->srcRaw ;
				saveEditColours( attr, DimBackColour ) ;
				putsChnOpRaw( (TRIGHT-TRMARGIN)*2, (y+TVOFF)*2, value, md2->switchSource, md2->disableExpoDr, LUA_RIGHT ) ;
				restoreEditColours() ;
					if ( attr )
					{
						uint8_t x = mapMixSource( value, md2->switchSource ) ;
#ifndef TOUCH
#if defined(PCBX7) || defined (PCBXLITE) || defined (PCBX9LITE)
						CHECK_INCDEC_H_MODELVAR( x, 1,NUM_SKYXCHNRAW+1+MAX_GVARS+1+NUM_SCALERS+8+NumExtraPots + (num_mix_switches-1) + EXTRA_SKYCHANNELS-1+4 ) ;
#else // PCBX7
						CHECK_INCDEC_H_MODELVAR( x, 1,NUM_SKYXCHNRAW+1+MAX_GVARS+1+NUM_SCALERS+8+NumExtraPots + (num_mix_switches-1) + EXTRA_SKYCHANNELS+4 ) ;
#endif // PCBX7
//						if ( md2->srcRaw > 4 )
//						{
#endif
							
#ifdef TOUCH
							if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#else
							if ( ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#endif
							{
								// Long MENU pressed
								TextIndex = x ;
  						  TextType = TEXT_TYPE_MIX_SOURCE ;
								TextControl.TextOption = md2->disableExpoDr ;
  						  killEvents(event) ;
								pushMenu(menuTextHelp) ;
    						s_editMode = false ;
							}
//						}	
						md2->srcRaw = unmapMixSource( x, &md2->switchSource ) ;
						
  					if ( checkForMenuEncoderLong( event ) )
						{
							if ( ( md2->srcRaw) && ( md2->srcRaw <= 4 ) )
							{
								SingleExpoChan = 1 ;
								TextResult = 0 ;
								s_expoChan = md2->srcRaw-1 ;
        				pushMenu(menuProcExpoAll);
							}
						}
					}
				}
        break ;
      case 1:
			{	
				drawItem( (char *)PSTR(STR_2WEIGHT)+1, y, attr ) ;

				saveEditColours( attr, DimBackColour ) ;
				drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
#ifdef USE_VARS
				if ( g_model.vars )
				{
					int16_t value = calcExtendedValue( md2->weight, md2->extWeight ) ;
					if ( ( value >= 510 ) && (value <= 514 ) )	// GVAR
					{
						uint16_t result = extendedValueEdit( md2->weight, md2->extWeight, attr, y+TVOFF, event, TRIGHT-TRMARGIN ) ;
						md2->weight = result & 0x00FF ;
						md2->extWeight = result >> 8 ;
					}
					else
					{
						if ( md2->varForWeight )
						{
							value += 1000 ;
						}
						value = editVarCapableValue( TRIGHT-TRMARGIN+FW, y+TVOFF, value, -350, 350, NUM_VARS, attr, event ) ;
						if ( value > 900 )
						{
							value -= 1000 ;
							md2->varForWeight = 1 ;
							md2->weight = value & 0x00FF ;
							md2->extWeight = 0 ;
						}
						else
						{
							md2->varForWeight = 0 ;
							uint16_t result = packExtendedValue( value ) ;
							md2->weight = result & 0x00FF ;
							md2->extWeight = result >> 8 ;
						}
					}
				}
				else
#endif
				{
					uint16_t result = extendedValueEdit( md2->weight, md2->extWeight, attr, y+TVOFF, event, TRIGHT-TRMARGIN ) ;
					md2->weight = result & 0x00FF ;
					md2->extWeight = result >> 8 ;
				}
				restoreEditColours() ;
			}
      break ;
      case 2:
			{	
				drawItem( (char *)PSTR(STR_OFFSET), y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
#ifdef USE_VARS
				if ( g_model.vars )
				{
					int16_t value = calcExtendedValue( md2->sOffset, md2->extOffset ) ;
					if ( ( value >= 510 ) && (value <= 514 ) )	// GVAR
					{
						uint16_t result = extendedValueEdit( md2->sOffset, md2->extOffset, attr, y+TVOFF, event, TRIGHT-TRMARGIN ) ;
						md2->sOffset = result & 0x00FF ;
						md2->extOffset = result >> 8 ;
					}
					else
					{
						if ( md2->varForOffset )
						{
							value += 1000 ;
						}
						value = editVarCapableValue( TRIGHT-TRMARGIN+FW, y+TVOFF, value, -350, 350, NUM_VARS, attr, event ) ;
						if ( value > 900 )
						{
							value -= 1000 ;
							md2->varForOffset = 1 ;
							md2->sOffset = value & 0x00FF ;
							md2->extOffset = 0 ;
						}
						else
						{
							md2->varForOffset = 0 ;
							uint16_t result = packExtendedValue( value ) ;
							md2->sOffset = result & 0x00FF ;
							md2->extOffset = result >> 8 ;
						}
					}
				}
				else
#endif
				{
					uint16_t result = extendedValueEdit( md2->sOffset, md2->extOffset, attr, y+TVOFF, event, TRIGHT-TRMARGIN ) ;
					md2->sOffset = result & 0x00FF ;
					md2->extOffset = result >> 8 ;
				}
				restoreEditColours() ;
			}
      break ;
      case 3:
				md2->lateOffset = touchOnOffItem( md2->lateOffset, y, PSTR(STR_2FIX_OFFSET), attr, DimBackColour ) ;
      break;
      case 4:
				if ( ( md2->srcRaw <=4 ) )
				{
				  md2->disableExpoDr = touchOffOnItem( md2->disableExpoDr, y, PSTR(STR_ENABLEEXPO), attr, DimBackColour ) ;
				}
				else
				{
				  md2->disableExpoDr = touchOnOffItem( md2->disableExpoDr, y, XPSTR("Use Output   "), attr, DimBackColour ) ;
				}
      break;
      case 5:
			  md2->carryTrim = touchOffOnItem( md2->carryTrim, y, PSTR(STR_2TRIM)+1, attr, DimBackColour ) ;
      break;
        
			case 6:
			{
				drawItem( (char *)XPSTR("Crv/Dif/Exp"), y, attr ) ;
				drawIdxText( y, (char *)XPSTR("\005 None FuncCurve Diff Expo"), curveFunction, attr ) ;
				uint8_t oldvalue =  curveFunction ;
   		  if(attr) CHECK_INCDEC_H_MODELVAR( curveFunction, 0, 4 ) ;
				if ( curveFunction != oldvalue )
				{
					if ( ( curveFunction == 3 ) || ( curveFunction == 4 ) )		// diff/expo
					{
						curveValue = 0 ;
//						md2->curve = 0 ;
						diffIsGvar = 0 ;
					}
					else if ( curveFunction == 2 )		// curve
					{
						curveValue = 0 ;
	//					md2->curve = 7 ;
					}
					else if ( curveFunction == 1 )		// Function
					{
						curveValue = 1 ;
	//					if ( ( md2->curve < 1 ) || ( md2->curve > 6 ) )
	//					{
	//						md2->curve = 1 ;
	//					}
					}
					else
					{
						curveValue = 0 ;
	//					md2->curve = 0 ;
					}
				}
					
//						uint32_t diffValue ;
//						diffValue = md2->differential | (md2->extDiff << 1 ) ;
						
//					 	uint8_t value = diffValue ;
//						if ( value == 0 )
//						{
//							if ( md2->curve <= -28 )
//							{
//								value = 2 ;		// Expo
//							}
//						}
//	          lcd_putsAtt(  1*FW, y, PSTR(STR_Curve), (value == 0) ? attr : 0 ) ;
//	          lcd_putsAtt(  1*FW, y, PSTR(STR_15DIFF), (value & 1) ? attr : 0 ) ;
//	          lcd_putsAtt(  1*FW, y, XPSTR("\021Expo"), (value == 2) ? attr : 0 ) ;
//					 	uint8_t value2 = value ;
//						if ( value2 == 3 )
//						{
//							value2 = 1 ;
//						}
//    		    if(attr) CHECK_INCDEC_H_MODELVAR_0( value2, 2) ;
//					 	if ( value != value2 )
//						{
//							if ( ( value == 3 ) && ( value2 == 1 ) )
//							{
//								value2 = 3 ;
//							}
//						}	
//					 	if ( value != value2 )
//						{
//							if ( value2 == 2 )
//							{
//								md2->curve = -128 ;
//							}
//							else
//							{
//								md2->curve = 0 ;
//							}
//							diffValue = value2 & 1 ;	// 0 and 2 turn it off
//							md2->differential = diffValue ;
//							md2->extDiff = diffValue >> 1 ;
//						}
			}
			break ;

      case 7:
				drawItem( (char *)XPSTR(" Value"), y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
//				if ( curveFunction == 3 )	// Diff
//				{
//					lcd_putsSmallAttColour( TMID*TSCALE+2, y+2, "GVAR", 0 ) ;
//				}

				if ( curveFunction == 3 )	// Diff
				{
					drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+2 ) ;
					int16_t value = curveValue ;
					
#ifdef USE_VARS
					if ( g_model.vars )
					{
						if ( diffIsGvar )
						{
							value += 1000 ;
						}
						value = editVarCapableValue( TRIGHT-TRMARGIN+FW, y+TVOFF, value, -350, 350, NUM_VARS, attr, event ) ;
						if ( value > 900 )
						{
							value -= 1000 ;
							diffIsGvar = 1 ;
						}
						else
						{
							diffIsGvar = 0 ;
						}
					}
					else
	#endif
					{
						if ( diffIsGvar )
						{
							value += 510 ;
						}	
						value = gvarDiffValue( TRIGHT-TRMARGIN+FW, y+TVOFF, value, attr|0x80000000, event ) ;
						if ( value > 500 )
						{
							diffIsGvar = 1 ;
							value -= 510 ;
						}
						else
						{
							diffIsGvar = 0 ;
						}
					}					
					curveValue = value ;
				}
				else
				{
					if ( curveFunction == 4 )	// Expo
					{
#ifdef USE_VARS
						if ( g_model.vars )
						{
							if ( md2->varForExpo )
							{
								curveValue += 950 ;
							}
							curveValue = editVarCapableValue( TRIGHT-TRMARGIN+FW, y+TVOFF, curveValue, 0, 100, NUM_VARS, attr, event ) ;
							if ( curveValue > 800 )
							{
								curveValue -= 950 ;
								md2->varForExpo = 1 ;
							}
							else
							{
								md2->varForExpo = 0 ;
							}
						}
						else
#endif
						{
          	  PUTS_NUM(TRIGHT-TRMARGIN,y+TVOFF,curveValue,0);
          	  if(attr) CHECK_INCDEC_H_MODELVAR( curveValue, 0, 100 ) ;
						}
					}
					else
					{
						int32_t temp ;
						temp = curveValue ;
						if ( curveFunction == 2 )
						{
							if ( temp >= 0 )
							{
								temp += 7 ;
							}
						}
						put_curve( TRIGHT-TRMARGIN, y+TVOFF, temp, LUA_RIGHT ) ;
						if ( curveFunction )
						{
          	  if(attr)
							{
								if ( curveFunction == 1 ) // Function
								{
        		  		CHECK_INCDEC_H_MODELVAR( curveValue, 1, 6 ) ;
								}
								else
								{
        		  		CHECK_INCDEC_H_MODELVAR( curveValue, -MAX_CURVE5-MAX_CURVE9-3, MAX_CURVE5+MAX_CURVE9-1+3 ) ;
									if ( event == EVT_KEY_LONG(BTN_RE) )
									{
										if ( curveValue >= 0 )
										{
          	  			  s_curveChan = curveValue ;
	          				  pushMenu(menuProcCurveOne);
										}
										else
										{
          	  			  s_curveChan = -curveValue-1 ;
	          				  pushMenu(menuProcCurveOne);
										}
									}
								}
							}
						}
						else
						{
							if ( attr )
							{
								s_editMode = 0 ;	// Nothing to edit
							}
						}
					}
				}
				restoreEditColours() ;
      break ;

      case 8:
				drawItem( (char *)PSTR(STR_2SWITCH)+1, y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				putsDrSwitches(TRIGHT-TRMARGIN, y+TVOFF, md2->swtch, LUA_RIGHT ) ;
				restoreEditColours() ;
        if(attr) CHECK_INCDEC_MODELSWITCH( md2->swtch, -MaxSwitchIndex, MaxSwitchIndex);
      break;

      case 9:
			{	
				b = 1 ;
				drawItem( (char *)PSTR(STR_MODES), y, attr ) ;
//            PUTS_ATT_LEFT( y,XPSTR("\001MODES"));
						
				if ( attr )
				{
					Columns = 7 ;
					s_editMode = 0 ;	// Nothing to edit by touch
				}
  					
				for ( uint32_t p = 0 ; p<MAX_MODES+2 ; p++ )
				{
					uint8_t z = md2->modeControl ;
// 					lcd_putcAtt( (9+p)*(FW+1), y, '0'+p, ( z & b ) ? 0 : INVERS ) ;
   				PUTC_ATT( TRIGHT-TRMARGIN-7*(FW+3)+p*(FW+2), y+TVOFF, '0'+p, ( z & b ) ? 0 : INVERS ) ;
					if( attr && ( g_posHorz == p ) )
					{
						lcd_rect( TRIGHT-TRMARGIN-7*(FW+3)+p*(FW+2)-1, y+TVOFF-3, FW+1, 12 ) ;
						if ( event==EVT_KEY_BREAK(KEY_MENU) || event==EVT_KEY_BREAK(BTN_RE) ) 
						{
							md2->modeControl ^= b ;
     					eeDirty(EE_MODEL) ;
   						s_editMode = false ;
						}
					}
					b <<= 1 ;
				}
			}
      break ;

      case 10:
				drawItem( (char *)XPSTR("Warning"), y, attr ) ;
//            PUTS_P(  2*FW,y,PSTR(STR_2WARNING));
				b = md2->mixWarn ;
				saveEditColours( attr, DimBackColour ) ;
        if(b)
          PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF, b, 0 ) ;
        else
          PUTS_ATT( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR(STR_OFF), 0 ) ;
				restoreEditColours() ;
        if(attr) { CHECK_INCDEC_H_MODELVAR_0( b, 3); md2->mixWarn = b ; }
      break;
      case 11:
				drawItem( (char *)PSTR(STR_2MULTIPLEX)+1, y, attr ) ;
				drawIdxText( y, (char *)PSTR(STR_ADD_MULT_REP), md2->mltpx, attr ) ;
        if(attr) CHECK_INCDEC_H_MODELVAR_0( md2->mltpx, 2); //!! bitfield
      break;
      case 12:
				drawItem( (char *)PSTR(STR_2DELAY_DOWN)+1, y, attr ) ;
						md2->delayUp = editSlowDelay( y, attr, md2->delayUp ) ;	// Sense was wrong
      break;
      case 13:
				drawItem( (char *)PSTR(STR_2DELAY_UP)+1, y, attr ) ;
						md2->delayDown = editSlowDelay( y, attr, md2->delayDown ) ;	// Sense was wrong
      break;
      case 14:
				drawItem( (char *)PSTR(STR_2SLOW_DOWN)+1, y, attr ) ;
						md2->speedDown = editSlowDelay( y, attr, md2->speedDown ) ;
      break;
      case 15:
				drawItem( (char *)PSTR(STR_2SLOW_UP)+1, y, attr ) ;
						md2->speedUp = editSlowDelay( y, attr, md2->speedUp ) ;
      break;
    }
  }
	if ( curveFunction != 4 )
	{
		md2->varForExpo = 0 ;
	}
	switch ( curveFunction )
	{
		case 1 :
			curveFunction = 0 ;
		break ;
		case 2 :
			curveFunction = 0 ;
			if ( curveValue >= 0 )
			{
				curveValue += 7 ;
			}
		break ;
		case 3 :
			if ( diffIsGvar == 0 )
			{
				curveFunction = 1 ;
			}
		break ;
		case 4 :
			curveValue -= 128 ;
		break ;
	}
	md2->curve = curveValue ;
	md2->differential = curveFunction & 1 ;
	md2->extDiff = curveFunction >> 1 ;
}

void menuRadioVars(uint8_t event)
{
	TITLE(XPSTR("Radio Vars"));
	static MState2 mstate2 ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	uint32_t rows = NUM_RADIO_VARS ;
	mstate2.check_columns( event, rows-1 ) ;
	
	uint32_t i ;
	uint8_t sub = mstate2.m_posVert ;
	
//	uint16_t colour = dimBackColour() ;
	lcd_hline( 0, TTOP, TRIGHT ) ;

#ifdef TOUCH
	sub = mstate2.m_posVert = handleTouchSelect( rows, 0, sub ) ;
#endif
//	int32_t newSelection = checkTouchSelect( rows, 0 ) ;
//	uint16_t newVert = processSelection( sub , newSelection ) ;
//	sub = mstate2.m_posVert = newVert & 0x00FF ;
//	checkTouchEnterEdit( newVert ) ;

 	for( i = 0 ; i < rows ; i += 1 )
	{
    uint16_t y = i * TFH + TTOP ;
    uint8_t attr = (sub==i) ? INVERS : 0 ;

		drawItem( (char *)"Radio Var", y, attr ) ;
	  PUTC( 10*FW+THOFF, y+TVOFF, i+'1' ) ;

		drawNumber( TRIGHT-TRMARGIN, y, g_eeGeneral.radioVar[i], attr ) ; //, attr ? ~colour : colour ) ;
		if(attr)
		{
			g_eeGeneral.radioVar[i] = checkIncDec16( g_eeGeneral.radioVar[i], -1024, 1024, EE_GENERAL ) ;
   	}
	}
}

#define DATE_OFF_0		(0)
#define DATE_COUNT_ITEMS	8

void dispMonth( coord_t x, coord_t y, uint32_t month, LcdFlags attr) ;
void disp_datetime( coord_t y ) ;

t_time EntryTime ;

void menuProcDate(uint8_t event)
{
	TITLE(PSTR(STR_DateTime));
	static MState2 mstate2;
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif
//	uint16_t colour = dimBackColour() ;
	uint32_t rows = DATE_COUNT_ITEMS ;
	
	mstate2.check_columns(event, DATE_COUNT_ITEMS-1) ;
//?	event = mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1, rows-1) ;

  uint8_t  sub = mstate2.m_posVert ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows+1, 0 ) ;
	if ( newSelection > 0 )
	{
		newSelection -= 1 ;
		if ( sub == newSelection )
		{
			if ( event == 0 )
			{
				event = EVT_KEY_LONG(KEY_MENU) ;
			}
		}
		sub = mstate2.m_posVert = newSelection ;
	}
#endif

  switch(event)
  {
    case EVT_ENTRY :
			{	
				t_time *p = &EntryTime ;

				if ( Time.date == 0 )
				{
					Time.date = 1 ;
				}
				if ( Time.month == 0 )
				{
					Time.month = 1 ;
				}
				p->second = Time.second ;
				p->minute = Time.minute ;
				p->hour   = Time.hour ;
				p->date   = Time.date ;
				p->month  = Time.month ;
				p->year   = Time.year ;
				if ( p->year < 2000 )
				{
					p->year = 2000 + ( p->year % 100 ) ;
					
				}
			}
    break;
		
    case EVT_KEY_LONG(KEY_MENU) :
			rtcSetTime( &EntryTime ) ;
      killEvents(event);
			s_editMode = 0 ;
    break;
	}		 

	disp_datetime( TTOP+TVOFF ) ;
	lcd_hline( 0, TTOP, TRIGHT ) ;
	lcd_hline( 0, TTOP+TFH, TRIGHT ) ;

	for (uint32_t subN = 0 ; subN<rows ; subN += 1)
	{
		uint16_t y = TTOP+subN*TFH+TFH ;
	  uint16_t attr = ((sub==subN) ? INVERS : 0) ;
		switch ( subN )
		{
			case 0 :
				drawItem( (char *)PSTR(STR_SEC), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.second, 0) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.second = checkIncDec( EntryTime.second, 0, 59, 0 ) ;
			break ;
			case 1 :
				drawItem( (char *)XPSTR("Minute"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.minute, 0) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.minute = checkIncDec( EntryTime.minute, 0, 59, 0 ) ;
			break ;
			case 2 :
				drawItem( (char *)XPSTR("Hour"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.hour, 0) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.hour = checkIncDec( EntryTime.hour, 0, 23, 0 ) ;
			break ;
			case 3 :
				drawItem( (char *)PSTR(STR_DATE), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.date, 0) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.date = checkIncDec( EntryTime.date, 1, 31, 0 ) ;
			break ;
			case 4 :
				drawItem( (char *)PSTR(STR_MONTH), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.month, 0) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.month = checkIncDec( EntryTime.month, 1, 12, 0 ) ;
			break ;
			case 5 :
				drawItem( (char *)PSTR(STR_YEAR), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.year, 0) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.year = checkIncDec16( EntryTime.year, 0, 2999, 0 ) ;
			break ;
			case 6 :
			{	
				int8_t previous = g_eeGeneral.rtcCal ;
				drawItem( (char *)PSTR(STR_CAL), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, g_eeGeneral.rtcCal, 0) ; //, attr ? ~colour : colour ) ;
			  if(attr) CHECK_INCDEC_H_GENVAR( g_eeGeneral.rtcCal, -31, 31 ) ;
				if ( g_eeGeneral.rtcCal != previous )
				{
					previous = g_eeGeneral.rtcCal ;
					uint32_t sign = 0 ;
					if ( g_eeGeneral.rtcCal < 0 )
					{
						sign = 0x80 ;
						previous = -g_eeGeneral.rtcCal ;
					}
					rtcSetCal( sign, previous ) ;						
				}
			}
			break ;
			case 7 :
				drawItem( (char *)XPSTR("Set"), y, attr ) ;
				if ( attr )
				{
					PUTS_ATT_COLOUR( TRIGHT-TRMARGIN-3*FW, y+TVOFF, (char *)"Set", 0, attr ? ~LcdForeground : LcdForeground ) ; // , attr ? ~colour : colour ) ;
#ifdef TOUCH	
					if ( handleSelectIcon() )
#else
					if ( checkForMenuEncoderLong( event ) )					
#endif
					{
						rtcSetTime( &EntryTime ) ;
					}
				}
			break ;
		}
extern uint16_t VbattRtc ;
		  PUTS_P( TRIGHT+FW, TTOP+7*TFH, XPSTR("RTC V") );
			PUTS_NUM( TRIGHT+5*FW, TTOP+8*TFH, VbattRtc*600/2048, PREC2 ) ;
	}
}


// background script to be added

void menuModelGeneral( uint8_t event )
{
	static MState2 mstate2;
	static uint8_t voiceCall = 0 ;			
  TITLE( PSTR(STR_General) ) ;
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif
	uint32_t subN = 0 ;
	LcdFlags attr = 0 ;
	uint32_t type ;
	uint32_t rows ;
#ifdef TOUCH	
	uint32_t newVpos ;
#endif
	uint16_t colour = dimBackColour() ;

	rows = 21 ;
#ifdef USE_VARS
	rows = 22 ;
#endif
	
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	mstate2.check_columns(event, rows-1) ;
  uint32_t  sub    = mstate2.m_posVert;
  uint32_t t_pgOfs ;

	if ( event == EVT_ENTRY_UP )
	{
		if ( voiceCall )
		{
			if ( FileSelectResult == 1 )
			{
				if ( voiceCall == 2 )
				{
			 		copyFileName( g_model.modelImageName, SelectedVoiceFileName, 10 ) ;
			 		loadModelImage() ;
				}
				else if ( voiceCall == 1 )
				{
			 		g_model.modelVoice = -1 ;
			 		copyFileName( g_model.modelVname, SelectedVoiceFileName, 8 ) ;
				}
			}
			voiceCall = 0 ;
		}
	}

	t_pgOfs = evalHresOffset( sub ) ;
	
#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	
	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	coord_t y = TTOP ;
	lcd_hline( 0, TTOP, TRIGHT ) ;
//	uint16_t oldBcolour = LcdBackground ;
	
	if(t_pgOfs<=subN)
	{
    uint8_t attr = (sub==subN) ? INVERS : 0 ;
		drawItem( (char *)"Model Name", y, attr ) ;
		saveEditColours( attr, colour ) ;
		alphaEditName( TRIGHT-10*FW-5, y+TVOFF, (uint8_t *)g_model.name, sizeof(g_model.name), attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
		restoreEditColours() ;
		validateName( (uint8_t *)g_model.name, sizeof(g_model.name) ) ;
		if ( AlphaEdited )
		{
	  	memcpy(ModelNames[g_eeGeneral.currModel+1], g_model.name, sizeof(g_model.name) ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
		if(sub==subN)
		{
			MaskRotaryLong = 1 ;
#ifdef TOUCH	
			if ( event==EVT_KEY_LONG(BTN_RE) || handleSelectIcon() )
#else
			if ( event==EVT_KEY_LONG(BTN_RE) )
#endif
			{
  			s_editMode = 0 ;
				voiceCall = 1 ;
				VoiceFileType = VOICE_FILE_TYPE_NAME ;
    		killEvents( event ) ;
		   	pushMenu( menuProcSelectVoiceFile ) ;
			}
		}
		type = (sub==subN) ;
		drawItem( (char *)"Voice File", y, type ) ;
		
		saveEditColours( attr, colour ) ;
		alphaEditName( TRIGHT-9*FW-5, y+TVOFF, (uint8_t *)g_model.modelVname, sizeof(g_model.modelVname), type | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FileName") ) ;
		restoreEditColours() ;
		validateName( (uint8_t *)g_model.modelVname, sizeof(g_model.modelVname) ) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
	
	if(t_pgOfs<=subN)
	{
		uint8_t subSub = g_posHorz ;
		g_model.thrTrim = touchOnOffItem( g_model.thrTrim, y, PSTR(STR_T_TRIM), ( sub==subN ) && (subSub==0 ), colour ) ;
		attr = 0 ;
		uint8_t t = 100 - g_model.throttleIdleScale ;
		if ( sub==subN )
		{
			Columns = 1 ;
			if ( subSub == 1 )
			{
				attr = INVERS ;
				CHECK_INCDEC_H_MODELVAR( t, 0, 100 ) ;
				g_model.throttleIdleScale = 100 - t ;
			}
		}
		PUTS_NUM( TRIGHT - 4*FW, y+TVOFF, t, attr ) ;

		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
		g_model.thrExpo = touchOnOffItem( g_model.thrExpo, y, PSTR(STR_T_EXPO), sub==subN, colour) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
	
	if(t_pgOfs<=subN)
	{
  	attr = 0 ;
  	if(sub==subN) { attr = INVERS ; }
		drawItem( (char *)PSTR(STR_LIGHT_SWITCH), y, attr ) ;
		if ( attr & INVERS )
		{
			if ( s_editMode && BLINK_ON_PHASE )
			{
				attr = 0 ;
			}
		}
		saveEditColours( attr, colour ) ;
		g_model.mlightSw = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, g_model.mlightSw, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
		if ( g_model.mlightSw == 0 )
		{
			putsDrSwitches( TRIGHT-TRMARGIN-9*FW, y+TVOFF, g_eeGeneral.lightSw, 0 ) ;
  	  PUTC_ATT( TRIGHT-TRMARGIN-10*FW, y+TVOFF, '(', 0 ) ;
  	  PUTC_ATT( TRIGHT-TRMARGIN-5*FW, y+TVOFF, ')', 0 ) ;
		}
		restoreEditColours() ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
		g_model.useCustomStickNames = touchOnOffItem( g_model.useCustomStickNames, y, PSTR( STR_CUSTOM_STK_NAMES ), sub==subN, colour ) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
  	attr = 0 ;
  	if(sub==subN) { attr = INVERS ; }
		drawItem( (char *)PSTR(STR_TRIM_INC), y, attr ) ;
//		lcd_putsAttIdxColour( TRIGHT-6*FW-5, y+TVOFF, PSTR(STR_TRIM_OPTIONS)+2, g_model.trimInc, 0, (sub==subN) ? ~LcdForeground : LcdForeground, (sub==subN) ? ~colour : colour ) ;
		drawIdxText( y, (char *)PSTR(STR_TRIM_OPTIONS)+2, g_model.trimInc, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
	  
		if(sub==subN)
		{
			g_model.trimInc = checkIncDec16( g_model.trimInc, 0, 4, EditType ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
  	attr = 0 ;
  	if(sub==subN)
		{
			attr = INVERS ;
		}
		drawItem( (char *)PSTR(STR_TRIM_SWITCH), y, attr ) ;
		saveEditColours( attr, colour ) ;
		g_model.trimSw = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, g_model.trimSw, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
		restoreEditColours() ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
  	attr = 0 ;
  	if(sub==subN)
		{
			attr = INVERS ;
		}
		drawItem( (char *)XPSTR(" Modify:"), y, attr ) ;
//		lcd_putsAttIdxColour( TRIGHT-TRMARGIN-8*FW, y+TVOFF, XPSTR("\010SubTrims   Trims"), g_model.instaTrimToTrims, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
		drawIdxText( y, XPSTR("\010SubTrims   Trims"), g_model.instaTrimToTrims, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
	  if(attr)
		{
			g_model.instaTrimToTrims = checkIncDec16( g_model.instaTrimToTrims, 0, 1, EditType ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
		g_model.extendedLimits = touchOnOffItem( g_model.extendedLimits, y, PSTR(STR_E_LIMITS), sub==subN, colour ) ;
    //---------------------------------                           /* ReSt V */
    //   If extended Limits are switched off, min and max limits must be limited to +- 100         
  	if (!g_model.extendedLimits)
  	{
			for( uint32_t i = 0 ; i < NUM_SKYCHNOUT ; i += 1 )
  	  {
  	    LimitData *ld = &g_model.limitData[i] ;
  	    if (ld->min < 0) ld->min = 0;
  	    if (ld->max > 0) ld->max = 0;
  	  }
  	}                                 
    //-----------------------------------                           /* ReSt A */
				
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
  	attr = 0 ;
  	if(sub==subN)
		{
			attr = INVERS ;
		}
		uint8_t oldValue = g_model.throttleReversed ;
		g_model.throttleReversed = touchOnOffItem( g_model.throttleReversed, y, PSTR(STR_THR_REVERSE), sub==subN, colour) ;
		PUTC( TRIGHT-TRMARGIN-3*FW, y+TVOFF, throttleReversed() ? '\201' : '\200' ) ;
		if ( g_model.throttleReversed != oldValue )
		{
  		checkTHR() ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
		uint8_t oldValue = g_model.throttleIdle ;
		drawItem( (char *)PSTR( STR_THR_DEFAULT), y, (sub == subN ) ) ;
		saveEditColours( sub == subN, colour ) ;
  	drawIdxText( y, XPSTR("\006   EndCentre"), g_model.throttleIdle, attr|LUA_RIGHT) ;
		restoreEditColours() ;
  	if(sub==subN)
		{
			CHECK_INCDEC_H_MODELVAR( g_model.throttleIdle, 0, 1 ) ;
		}
		if ( g_model.throttleIdle != oldValue )
		{
  		checkTHR() ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
		drawItem( (char *)PSTR(STR_CUSTOM_CHECK), y, (sub==subN) ) ;
		if ( sub == subN )
		{
#ifdef TOUCH	
			if ( event==EVT_KEY_LONG(BTN_RE) || handleSelectIcon() )
#else
			if ( event==EVT_KEY_LONG(BTN_RE) )
#endif
			{
    		pushMenu( menuCustomCheck ) ;
  			s_editMode = 0 ;
    		killEvents(event);
			}
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
    uint8_t attr = 0 ;
  	if(sub==subN)
		{
			attr = INVERS ;
			CHECK_INCDEC_H_MODELVAR( g_model.sub_trim_limit, 0, 100 ) ;
		}
		drawItem( (char *)PSTR(STR_AUTO_LIMITS), y, attr ) ;
		drawNumber( TRIGHT-TRMARGIN, y, g_model.sub_trim_limit, attr|PREC1) ; //, attr ? ~colour : colour ) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	uint32_t states = g_model.modelswitchWarningStates ;
	if(t_pgOfs<=subN)
	{
		uint8_t b = (states & 1) ;
		b = touchOffOnItem( b, y, PSTR(STR_SWITCH_WARN), sub==subN, colour ) ;
  	g_model.modelswitchWarningStates = (states & ~1) | b ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
	// Default Switch Enables
		uint32_t enables = 0xFF ;
		uint32_t temp = g_model.modelswitchWarningDisables ;
		if ( temp & 0x0003 )
		{
			enables &= ~0x01 ;
		}
		if ( temp & 0x000C )
		{
			enables &= ~0x02 ;
		}
		if ( temp & 0x0030 )
		{
			enables &= ~0x04 ;
		}
		if ( temp & 0x00C0 )
		{
			enables &= ~0x08 ;
		}
		if ( temp & 0x0300 )
		{
			enables &= ~0x10 ;
		}
		if ( temp & 0x0C00 )
		{
			enables &= ~0x20 ;
		}
		if ( temp & 0x3000 )
		{
			enables &= ~0x40 ;
		}
		if ( temp & 0xC000 )
		{
			enables &= ~0x80 ;
		}

		drawItem( (char *)PSTR(STR_DEAFULT_SW), y, ( sub == subN ) ) ;
		uint8_t subSub = g_posHorz ;
		if ( sub == subN )
		{
			Columns = 7 ;
		}
	  for( uint32_t i = 0 ; i < 8 ; i += 1 )
		{
			uint8_t attr = 0 ;
			if ( (enables & (1<<i)) )
			{
				attr = INVERS ;
			}
			if ( sub == subN )
			{
				if ( subSub == i )
				{
					attr = BLINK ;	
				}
			}
			PUTS_ATT_N(TRIGHT-TRMARGIN-8*FW+i*FW, y+TVOFF, XPSTR("ABCDEFG6")+i,1, attr ) ;
		}
		if(sub==subN)
		{
			if ( event == EVT_KEY_BREAK(BTN_RE) )
			{
				uint32_t shift = subSub ;
	      killEvents(event) ;
	    	s_editMode = false ;
	  	  enables ^= (1<<(shift)) ;
	//      STORE_MODELVARS;
			}
		}
		temp = 0 ;
		if ( ( enables & 1 ) == 0 )    temp |= 0x0003 ;
		if ( ( enables & 2 ) == 0 )    temp |= 0x000C ;
		if ( ( enables & 4 ) == 0 )    temp |= 0x0030 ;
		if ( ( enables & 8 ) == 0 )    temp |= 0x00C0 ;
		if ( ( enables & 0x10 ) == 0 ) temp |= 0x0300 ;
		if ( ( enables & 0x20 ) == 0 ) temp |= 0x0C00 ;
		if ( ( enables & 0x40 ) == 0 ) temp |= 0x3000 ;
		if ( ( enables & 0x80 ) == 0 ) temp |= 0xC000 ;
		g_model.modelswitchWarningDisables = temp ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
		states >>= 1 ;
		states |= g_model.xmodelswitchWarningStates << 16 ;

		for ( uint32_t i = 0 ; i < 7 ; i += 1 )
		{
		  PUTC( 2*FW+i*(2*FW+3), y+TVOFF, 'A'+i ) ;
			PUTC( 3*FW+i*(2*FW+3), y+TVOFF, PSTR(HW_SWITCHARROW_STR)[states & 0x03] ) ;
		  states >>= 2 ;
		}
		PUTC( 2*FW+7*(2*FW+3), y+TVOFF, '6' ) ;
		PUTC( 3*FW+7*(2*FW+3), y+TVOFF, 'P' ) ;
		PUTC( 4*FW+7*(2*FW+3), y+TVOFF, ( (states >> 2) & 0x07) + '0' ) ;
		if( sub == subN )
		{
			lcd_rect( 2*FW-2, y+TVOFF, 17*FW+2+18+1+3, 10 ) ;
//			lcdHiresRect( 2*FW-4, y-2+TVOFF, 18*FW+8, 34, LCD_BLACK ) ;
//	extern uint32_t SwitchesStates ;
	extern uint32_t switches_states ;
			if ( event==EVT_KEY_FIRST(BTN_RE) )
			{
				killEvents(event) ;
  			getMovedSwitch() ;	// loads SwitchesStates
		    g_model.modelswitchWarningStates = ((switches_states & 0x7FFF) << 1 ) ;// states ;
				g_model.xmodelswitchWarningStates = switches_states >> 15 ; 
				s_editMode = false ;
	//			STORE_MODELVARS ;
			}
		}
		lcd_hline( 0, y+TFH, TRIGHT ) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

	if(t_pgOfs<=subN)
	{
 		attr = 0 ;
  	if (sub==subN)
		{
			attr = INVERS ;
			CHECK_INCDEC_H_MODELVAR( g_model.anaVolume, 0, 7 ) ;
		}
		drawItem( (char *)PSTR(STR_VOLUME_CTRL), y, attr ) ;
		drawIdxText( y, XPSTR("\003---S1 S2 SL SR GV5GV6GV7"),g_model.anaVolume, attr|LUA_RIGHT ) ; // 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
	
// Beep Centre
	
//	if(t_pgOfs<=subN)
//	{
//		uint8_t subSub = g_posHorz ;
//		PUTS_ATT_LEFT( y+TVOFF, PSTR(STR_BEEP_CENTRE));
//    for(uint8_t i=0;i<8;i++)
//		{
//			lcd_putsnAtt((10+i)*FW, y+TVOFF, PSTR(STR_RETA123)+i,1, (((subSub)==i) && (sub==subN)) ? BLINK : ((g_model.beepANACenter & (1<<i)) ? INVERS : 0 ) );
//		}
//    if(sub==subN)
//		{
//			Columns = 7 ;
//      if((event==EVT_KEY_FIRST(KEY_MENU)) || (event == EVT_KEY_BREAK(BTN_RE)) || P1values.p1valdiff)
//			{
//        killEvents(event);
//        s_editMode = false;
//        g_model.beepANACenter ^= (1<<(subSub));
//        STORE_MODELVARS;
//	    }
//  	}

		//	if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
//	}
//	subN += 1 ;


	if(t_pgOfs<=subN)
	{
		drawItem( (char *)"Image Name", y, (sub==subN) ) ;
		type = ALPHA_NO_NAME ;
//		voiceCall = 0 ;			
		if(sub==subN)
		{
	//		Columns = 1 ;
	//					SubMenuCall = 0x92 ;
			type |= EE_MODEL ;
#ifdef TOUCH	
			if ( checkForMenuEncoderLong( event ) || handleSelectIcon() )
#else
			if ( checkForMenuEncoderLong( event ) )
#endif
			{
				voiceCall = 2 ;
  	  	pushMenu( menuProcSelectImageFile ) ;
			}
		}
		if ( voiceCall == 0 )
		{
	    uint8_t attr = (sub==subN) ? INVERS : 0 ;
//			uint16_t oldBcolour = LcdBackground ;
			saveEditColours( sub == subN, colour ) ;
			alphaEditName( TRIGHT-10*FW-5, y+TVOFF, (uint8_t *)g_model.modelImageName, sizeof(g_model.modelImageName), attr | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FIlename") ) ;
			restoreEditColours() ;
			validateName( (uint8_t *)g_model.modelImageName, sizeof(g_model.modelImageName) ) ;
		}

		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
	 
// Bg Script

	if(t_pgOfs<=subN)
	{
		g_model.disableThrottleCheck = touchOnOffItem( g_model.disableThrottleCheck, y, XPSTR("Disable Thr Chk"), sub==subN, colour) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;

//#if MULTI_GVARS
//	if(t_pgOfs<=subN)
//	{
//		uint32_t oldValue = g_model.flightModeGvars ;
//		g_model.flightModeGvars = touchOnOffItem( g_model.flightModeGvars, y, XPSTR("Multi Gvars"), sub==subN, colour) ;
//		if ( g_model.flightModeGvars != oldValue )
//		{
//			if ( g_model.flightModeGvars )
//			{
//				initFmGvars() ;
//			}
//			else
//			{
//				for ( uint32_t i = 0 ; i < 7 ; i += 1 )
//				{
//					g_model.gvars[i].gvar = 0 ;
//					g_model.gvars[i].gvsource = 0 ;
//					g_model.gvswitch[i] = 0 ;
//				}
//			}
//		}
//		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
//	}
//	subN += 1 ;
//#endif

#ifdef USE_VARS
	if(t_pgOfs<=subN)
	{
		uint32_t old = g_model.vars ;
		g_model.vars = touchOnOffItem( g_model.vars, y, XPSTR("Enable vars"), sub==subN, colour) ;
		if ( old != g_model.vars )
		{
			if ( g_model.vars )
			{
				// Just enabled
				uint32_t i ;
				for ( i = 0 ; i < VAR_STORAGE_UINTS ;  i += 1 )
				{
					g_model.varStore[i] = 0 ;
				}
				initVars() ;
			}
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
#endif

	if(t_pgOfs<=subN)
	{
		attr = sub==subN ? INVERS : 0 ;
		drawItem( (char *)"Script Type", y, attr ) ;
//		g_model.basic_lua = checkIndexed( y+TVOFF, XPSTR(FWx16"\001""\005Basic  LUA"), g_model.basic_lua, (sub==subN) ) ;
  	drawIdxText( y, XPSTR("\005Basic  LUA"), g_model.basic_lua, attr|LUA_RIGHT ) ; // , attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
		if ( attr )
		{
			CHECK_INCDEC_H_MODELVAR( g_model.basic_lua, 0, 1 ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
}

void menuProcDiagVers(uint8_t event)
{
	uint16_t x ;
	uint16_t y ;
	uint32_t i ;
	uint32_t j ;
	char c ;
	TITLE(PSTR(STR_Version)) ;
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2 ;
	mstate2.check_columns(event, 1-1) ;
	char text[32] ;
	  
	char *p ;
	char *q ;
	p = (char *)Stamps ;
	y = TTOP + TFH ;
	x = 8*FW ;

	for ( i = 0 ; i < 4 ; i += 1 )
	{
		q = text ;
		j = 0 ;
		for(;;)
		{
			c = *p++ ;
			*q++ = c ;

			if ( c == ':')
			{
				*q = 0 ;
  			lcdDrawText( x*2, y, text, LUA_RIGHT ) ;
				break ;
			}

			if ( ++j > 29)
			{
				break ;
			}
		}
		q = text ;
		j = 0 ;
		for(;;)
		{
			c = *p++ ;
			*q++ = c ;

			if ( ( c == '\037') || ( c == 0 ) )
			{
				*(q-1) = 0 ;
  			lcdDrawText( x*2+FW, y, text, 0 ) ;
				break ;
			}

			if ( ++j > 29)
			{
				break ;
			}
		}
		y += TFH*2 ;
	}

//	for ( i = 0 ; i < 4 ; i += 1 )
//	{
//		for(;;)
//		{
//			c = *p++ ;
//			if ( (c == '\0') || (c == '\037') )
//			{
//				y += TFH ;
//				x = 8*FW ;
//				break ;
//			}
//			PUTC( x, y, c ) ;
//			x += FW ;
//		}
//	}
//	PUTS_ATT_LEFT( y,Stamps ) ;
}

void menuControls(uint8_t event)
{
	TITLE( PSTR(STR_Controls) ) ;
	static MState2 mstate2 ;
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif
	uint32_t rows = 8 ;
	event = mstate2.check_columns( event, rows-1 ) ;
	uint16_t colour = dimBackColour() ;
		
	uint8_t subN = 0 ;
	uint8_t sub = mstate2.m_posVert ;
	uint16_t y = TTOP ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows+1, 0, 0 ) ;
	if ( newSelection >= 0 )
	{
		if ( newSelection > 3 )
		{
			newSelection -= 1 ;
		}
		sub = mstate2.m_posVert = newSelection ;
	}
#endif

	uint8_t attr = sub==subN ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_CHAN_ORDER), y, (sub == subN ) ) ;
	uint8_t bch = bchout_ar[g_eeGeneral.templateSetup] ;
	saveEditColours( attr, colour ) ;
  for ( uint32_t i = 4 ; i > 0 ; i -= 1 )
	{
		uint8_t letter ;
		letter = *(PSTR(STR_SP_RETA) +(bch & 3) + 1 ) ;
  	PUTC_ATT( TRIGHT-TRMARGIN-5*FW+i*FW, y+TVOFF, letter, 0 ) ;
		bch >>= 2 ;
	}
	restoreEditColours() ;
	if(attr) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.templateSetup, 23 ) ;
 	y += TFH ;
	subN += 1 ;
      
	attr = sub==subN ? INVERS : 0 ;
	uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
	drawItem( (char *)PSTR(STR_CROSSTRIM), y, attr ) ;
  if(attr)
	{
		CHECK_INCDEC_H_GENVAR_0( ct, 2 ) ;
	}
	saveEditColours( attr, colour ) ;
  drawIdxText( y, XPSTR("\003OFFON Vtg"), ct, attr|LUA_RIGHT ) ;
	restoreEditColours() ;
	g_eeGeneral.crosstrim = ct ;
	g_eeGeneral.xcrosstrim = ct >> 1 ;
 	y += TFH ;
	subN += 1 ;

	uint8_t oldValue = g_eeGeneral.throttleReversed ;
	g_eeGeneral.throttleReversed = touchOnOffItem( g_eeGeneral.throttleReversed, y, PSTR(STR_THR_REVERSE), (sub == subN ), colour ) ;
	if ( g_eeGeneral.throttleReversed != oldValue )
	{
  	checkTHR() ;
	}
 	y += TFH ;
	subN += 1 ;

 	attr = 0 ;
	uint8_t mode = g_eeGeneral.stickMode ;
  if(sub==subN)
	{
		attr = INVERS ;
		if ( s_editMode )
		{
			attr = BLINK ;
			CHECK_INCDEC_H_GENVAR_0( mode,3 ) ;
			if ( mode != g_eeGeneral.stickMode )
			{
//				g_eeGeneral.stickScroll = 0 ;
				g_eeGeneral.stickMode = mode ;							
			}
		}
	}
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
	}
	saveEditColours( attr, LcdBackground ) ;
	PUTS_P( THOFF, y+TVOFF, PSTR(STR_MODE) ) ;
	lcd_hline( 0, y+TFH, TRIGHT ) ;
	for ( uint32_t i = 0 ; i < 4 ; i += 1 )
	{
 		lcd_HiResimg( (6+4*i)*FW*2, y*2+TVOFF*2+1, sticksHiRes, i, 0, LcdForeground, attr&INVERS ? ~LcdBackground : LcdBackground ) ;
	}
	restoreEditColours() ;
 	y += TFH ;
    
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
	}
	saveEditColours( attr, LcdBackground ) ;
  PUTC_ATT( 3*FW, y+TVOFF, '1'+g_eeGeneral.stickMode, 0 ) ;
  for(uint32_t i=0; i<4; i++)
	{
		putsChnRaw( ((6+4*i)*FW)*2, (y+TVOFF)*2, modeFixValue( i ), 0 ) ;//sub==3?INVERS:0);
	}
	restoreEditColours() ;
	lcd_hline( 0, y+TFH, TRIGHT ) ;
 	y += TFH ;
	subN += 1 ;
	
	if ( sub >= rows )
	{
		sub = mstate2.m_posVert = rows-1 ;
	}
	// Edit custom stick names
  for ( uint32_t i = 0 ; i < 4 ; i += 1 )
	{
		attr = sub==subN ? INVERS : 0 ;
    PUTS_AT_IDX( THOFF/2, y+TVOFF, PSTR(STR_STICK_NAMES), i, 0 ) ;
    PUTS_P( THOFF+4*FW, y+TVOFF, (char *)"Stick Name" ) ;
		uint16_t oldBcolour = LcdBackground ;
		LcdBackground = attr ? ~colour : colour ;
		lcdDrawSolidFilledRectDMA( TMID*TSCALE, y*TSCALE+2, (TRIGHT-TMID)*TSCALE, TFH*TSCALE-2, LcdBackground ) ;
		alphaEditName( TRIGHT-TRMARGIN, y+TVOFF, &g_eeGeneral.customStickNames[i*4], 4, attr|ALPHA_NO_NAME|LUA_RIGHT, (uint8_t *)&PSTR(STR_STICK_NAMES)[i*5+1] ) ;
		LcdBackground = oldBcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	 	y += TFH ;
		subN += 1 ;
	}
}

void menuModes(uint8_t event)
{
	uint32_t i ;
  uint8_t attr ;
#ifdef TOUCH	
	uint32_t selected = 0 ;
#endif
	 
	TITLE(PSTR(STR_MODES)) ;
	static MState2 mstate2 ;
  
#ifdef TOUCH	
	TlExitIcon = 1 ;
#endif
	uint32_t rows = 8 ;
	event = mstate2.check_columns( event, rows-1 ) ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
	uint16_t colour = LcdBackground ;
	
	uint8_t sub = mstate2.m_posVert ;

#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows+1, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

#ifdef TOUCH	
	if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) || selected )
#else
	if ( ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#endif
	{
		s_currIdx = sub ;
		killEvents(event);
//#if MULTI_GVARS
//		if ( ( g_model.flightModeGvars ) || ( ( g_model.flightModeGvars == 0 ) && s_currIdx ) )
//#else
		if ( s_currIdx )
//#endif
		{
			if ( s_currIdx <= 7 )
			{
				pushMenu(menuModeOne) ;
			}
		}
  }
    
	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;

  for ( i=0 ; i<MAX_MODES+1+1 ; i += 1 )
	{
		uint32_t k ;
    uint16_t y = TTOP + i*TFH ;
    attr = (i == sub) ? INVERS : 0 ;
		if ( i == 0 )
		{
			
	  	PUTC( THOFF, TTOP+TVOFF, 'F' ) ;
  		PUTC( THOFF+FW, TTOP+TVOFF, '0' ) ;
			PUTS_ATT( TRIGHT-TRMARGIN-10*FW, TTOP+TVOFF, XPSTR("R   E   T   A"), attr ) ;
			lcd_hline( 0, TTOP+TFH, TRIGHT ) ;
			s_editMode = 0 ;
			continue ;
		}
  	k = i - 1 ;
		
		PhaseData *p ;
		p = getPhaseAddress( k ) ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdForeground = ~LcdForeground ;
		}
//    lcd_putcAttColour( THOFF, y+TVOFF, 'F', 0, LcdForeground, attr ? ~colour : colour ) ;
//    lcd_putcAttColour( THOFF+FW, y+TVOFF, '1'+k, 0, LcdForeground, attr ? ~colour : colour ) ;
  	lcdDrawChar( THOFF*HVSCALE, (y*HVSCALE+TVOFF), 'F', 0, LcdForeground ) ;
  	lcdDrawChar( (THOFF+FW)*HVSCALE, (y*HVSCALE+TVOFF), '1'+k, 0, LcdForeground ) ;
		
		PUTS_ATT_N_COLOUR( THOFF+3*FW, y+TVOFF/HVSCALE, p->name, 6, 0, LcdForeground ) ; // , attr ? ~LcdBackground : LcdBackground ) ;
		LcdForeground = oldFcolour ;
//		putsDrSwitchesColour( THOFF+9*FW, y+TVOFF/HVSCALE, p->swtch, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
//		putsDrSwitchesColour( THOFF+14*FW, y+TVOFF/HVSCALE, p->swtch2, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
		
		DrawDrSwitches( THOFF+9*FW, y+TVOFF/HVSCALE, p->swtch, attr ) ;
		DrawDrSwitches( THOFF+14*FW, y+TVOFF/HVSCALE, p->swtch2, attr ) ;

		LcdBackground = attr ? ~colour : colour ;
		if ( attr )
		{
			LcdForeground = ~LcdForeground ;
		}
    for ( uint32_t t = 0 ; t < NUM_STICKS ; t += 1 )
		{
			putsTrimMode( TRIGHT-3-10*FW+t*FW*2, y+TVOFF/HVSCALE, k+1, t, 0 ) ;
			int16_t v = p->trim[t].mode ;
 			if (v && ( v & 1 ) )
			{
        PUTC( TRIGHT-3-11*FW+t*FW*2, y+TVOFF/HVSCALE, '+' ) ;
			}
		}
		LcdBackground = oldBcolour ;
		LcdForeground = attr ? ~oldFcolour : oldFcolour ;
		if ( p->fadeIn || p->fadeOut )
		{
//	    lcd_putcAttColour( TRIGHT-3-FW, y+TVOFF, '*', 0, LcdForeground, attr ? ~colour : colour ) ;
  		lcdDrawChar( (TRIGHT-3-FW)*2, (y+TVOFF)*2, '*', 0, LcdForeground ) ;
		}
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}	 
}

void menuGeneral( uint8_t event )
{
	TITLE( PSTR(STR_General) ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	
	static MState2 mstate2;
//			IlinesCount = 8+1+1+1 ;
	uint32_t rows = 11 ;
	mstate2.check_columns(event, rows-1) ;

//	uint16_t attr ;
#ifdef TOUCH
	uint32_t newVpos ;
#endif
//	uint8_t subN = 0 ;
  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
//	uint8_t blink = InverseBlink ;
	uint32_t k = 0 ;
	uint16_t t_pgOfs ;
	uint16_t colour = dimBackColour() ;

	lcd_hline( 0, TTOP, TRIGHT ) ;
	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	sub = mstate2.m_posVert = handleTouchSelect( rows, t_pgOfs, sub ) ;
#endif
//	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
//	uint16_t newVert = processSelection( sub , newSelection ) ;
//	sub = mstate2.m_posVert = newVert & 0x00FF ;
//	checkTouchEnterEdit( newVert ) ;

#ifdef TOUCH
	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	for( uint32_t i = 0 ; i<TLINES ; i += 1 )
	{
    y = i * TFH + TTOP ;
    k = i + t_pgOfs ;
    uint8_t attr = (sub==k) ? INVERS : 0 ;
		switch ( k )
		{
			case 0 :
			{
				uint16_t oldBcolour = LcdBackground ;
				drawItem( (char *)"Owner Name", y, attr ) ;
				LcdBackground = attr ? ~colour : colour ;
				alphaEditName( TRIGHT-10*FW-5, y+TVOFF, (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName), attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
				LcdBackground = oldBcolour ;
				validateName( (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName) ) ;
			}
			break ;
			case 1 :
				drawItem( (char *)PSTR(STR_LANGUAGE), y, attr ) ;
				drawIdxText( y, XPSTR("\012   ENGLISH  FRANCAIS   DEUTSCH NORWEGIAN   SWEDISH   ITALIAN    POLISHVIETNAMESE   SPANISH"),g_eeGeneral.language, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
				if(attr) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.language, 8 ) ;
				setLanguage() ;
			break ;
			case 2 :
			{
			  uint8_t b = g_eeGeneral.disableSplashScreen;
  			g_eeGeneral.disableSplashScreen = touchOffOnItem( b, y, PSTR(STR_SPLASH_SCREEN), attr, colour ) ;
			}
			break ;
			case 3 :
			{
  			uint8_t b = g_eeGeneral.hideNameOnSplash ;
  			g_eeGeneral.hideNameOnSplash = touchOffOnItem( b, y, PSTR(STR_SPLASH_NAME), attr, colour ) ;
			}
			break ;
			case 4 :
			{
  			uint8_t b = g_eeGeneral.stickScroll ;
  			g_eeGeneral.stickScroll = touchOnOffItem( b, y, PSTR(STR_STICKSCROLL), attr, colour ) ;
//  			uint8_t b = 1-g_eeGeneral.disablePotScroll ;
//				b |= g_eeGeneral.stickScroll << 1 ;
//				drawItem( (char *)PSTR(STR_SCROLLING), y, attr ) ;
//  			PUTS_AT_IDX_COLOUR( TRIGHT-5*FW, y+TVOFF, XPSTR("\005 NONE  POTSTICK BOTH"),b, 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
//  			if(attr) CHECK_INCDEC_H_GENVAR_0( b, 3 ) ;
//				g_eeGeneral.stickScroll = b >> 1 ;
//				g_eeGeneral.disablePotScroll = 1 - ( b & 1 ) ;
			}
			break ;
			case 5 :
			{
  			uint8_t b = g_eeGeneral.forceMenuEdit ;
  			g_eeGeneral.forceMenuEdit = touchOnOffItem( b, y, PSTR(STR_MENU_ONLY_EDIT), attr, colour ) ;
			}
			break ;
			case 6 :
  			g_eeGeneral.disableBtnLong = touchOnOffItem( g_eeGeneral.disableBtnLong, y, XPSTR("No ENC. as exit"), attr, colour ) ;
			break ;
			case 7 :
				drawItem( XPSTR("GPS Format"), y, attr ) ;
				drawIdxText( y, XPSTR("\012DD mm.mmmm DD.dddddd"), g_eeGeneral.gpsFormat, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
  			if(attr)
				{
					CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.gpsFormat,1) ;
				}
			break ;
			case 8 :
  			g_eeGeneral.altMixMenu = touchOnOffItem( g_eeGeneral.altMixMenu, y, XPSTR("Mix Menu Details"), attr, colour ) ;
			break ;
			case 9 :
			{	
				drawItem( XPSTR("ScreenShot Sw"), y, attr ) ;
				DrawDrSwitches( TRIGHT-TRMARGIN, y+TVOFF/HVSCALE, g_eeGeneral.screenShotSw, attr|LUA_RIGHT ) ;
//				putsDrSwitchesColour( TRIGHT-TRMARGIN, y+TVOFF, g_eeGeneral.screenShotSw, LUA_RIGHT, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
extern uint8_t LastShotSwitch ;
				if(attr)
				{
					LastShotSwitch = 1 ;
					CHECK_INCDEC_GENERALSWITCH( event, g_eeGeneral.screenShotSw, -MaxSwitchIndex, MaxSwitchIndex) ;
				}
				else
				{
					LastShotSwitch = 0 ;
				}
			}
			break ;
			case 10 :
  			g_eeGeneral.enableEncMain = touchOnOffItem( g_eeGeneral.enableEncMain, y, XPSTR("ENC main screen"), attr, colour ) ;
			break ;
		}
	}
}

void menuAlarms( uint8_t event )
{
	TITLE( (char *)PSTR(STR_Alarms) ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2;
	uint32_t rows = 6 ;
	event = mstate2.check_columns(event, rows-1) ;

  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	uint16_t colour = dimBackColour() ;
	uint8_t attr ;
	uint8_t subN = 0 ;

#ifdef TOUCH
	sub = mstate2.m_posVert = handleTouchSelect( rows, 0, sub ) ;
#endif
//	int32_t newSelection = checkTouchSelect( rows, 0 ) ;
//	uint16_t newVert = processSelection( sub , newSelection ) ;
//	sub = mstate2.m_posVert = newVert & 0x00FF ;
//	checkTouchEnterEdit( newVert ) ;

	y = TTOP ;
	lcd_hline( 0, TTOP, TRIGHT ) ;
	 
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_BATT_WARN), y, attr ) ;
  if(attr)
	{
		CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatWarn, 40, 120) ; //5-10V
	}
	drawNumber( TRIGHT-5-FW, y, g_eeGeneral.vBatWarn, attr|PREC1) ; //, attr ? ~colour : colour ) ;
	drawChar( TRIGHT-3-FW, y, 'V', attr, attr ? ~colour : colour ) ;
  y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_INACT_ALARM), y, attr ) ;
  if(attr)
	{
		attr = INVERS ;
		CHECK_INCDEC_H_GENVAR( g_eeGeneral.inactivityTimer, -10, 110) ; //0..120minutes
	}
	drawNumber( TRIGHT-5-FW, y, g_eeGeneral.inactivityTimer+10, attr) ; //, attr ? ~colour : colour ) ;
	drawChar( TRIGHT-3-FW, y, 'm', attr, attr ? ~colour : colour ) ;
  y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_VOLUME), y, attr ) ;
	int8_t value = g_eeGeneral.inactivityVolume + ( NUM_VOL_LEVELS-3 ) ;
	drawNumber( TRIGHT-5, y, value, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
		CHECK_INCDEC_H_GENVAR_0( value,NUM_VOL_LEVELS-1) ;
		g_eeGeneral.inactivityVolume = value - ( NUM_VOL_LEVELS-3 ) ;
	} 	
  y += TFH ;
	subN += 1 ;
      
	uint8_t b = g_eeGeneral.disableThrottleWarning ;
	g_eeGeneral.disableThrottleWarning = touchOffOnItem( b, y, PSTR(STR_SPLASH_SCREEN), sub == subN, colour ) ;
  y += TFH ;
	subN += 1 ;

	b = g_eeGeneral.disableAlarmWarning ;
	g_eeGeneral.disableAlarmWarning = touchOffOnItem( b, y, PSTR(STR_ALARM_WARN), sub == subN, colour ) ;
  y += TFH ;
	subN += 1 ;

	b = g_eeGeneral.disableRxCheck ;
	g_eeGeneral.disableRxCheck = touchOffOnItem( b, y, XPSTR("Receiver Warning"), sub == subN, colour ) ;
//  y += TFH ;
//	subN += 1 ;
}

void menuAudio( uint8_t event )
{
	TITLE( PSTR(STR_AudioHaptic) ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif

	static MState2 mstate2;
	uint32_t rows = 9 ;
	if ( g_eeGeneral.welcomeType == 2 )
	{
		rows += 1 ;
	}
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	mstate2.check_columns(event, rows-1) ;
	 
#ifdef TOUCH
	uint32_t newVpos ;
#endif
  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	uint32_t k = 0 ;
	uint16_t t_pgOfs ;
	uint16_t colour = dimBackColour() ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
	t_pgOfs = evalHresOffset( sub ) ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;

	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1)+1, t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	for( uint32_t i = 0 ; i<TLINES ; i += 1 )
	{
    y = i * TFH + TTOP ;
    k = i + t_pgOfs ;
    LcdFlags attr = (sub==k) ? INVERS : 0 ;
		switch ( k )
		{
			case 0 :
			{	
				uint8_t current_volume ;
				current_volume = g_eeGeneral.volume ;
				drawItem( (char *)PSTR(STR_VOLUME), y, attr ) ;
				drawNumber( TRIGHT-5, y, current_volume, attr) ; //, attr ? ~colour : colour ) ;
  			if ( attr )
				{
					CHECK_INCDEC_H_GENVAR_0( current_volume,NUM_VOL_LEVELS-1) ;
					if ( current_volume != g_eeGeneral.volume )
					{
						setVolume( g_eeGeneral.volume = current_volume ) ;
					}
				}
			}
      break ;
			case 1 :
			{	
				uint8_t b ;
  			b = g_eeGeneral.beeperVal ;
				drawItem( (char *)PSTR(STR_BEEPER), y, attr ) ;
				drawIdxText( y, (char *)PSTR(STR_BEEP_MODES), b, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
				if ( attr )
				{
					CHECK_INCDEC_H_GENVAR_0( b, 6 ) ;
					g_eeGeneral.beeperVal = b ;
				}
			}
      break ;
			case 2 :
				drawItem( (char *)PSTR(STR_SPEAKER_PITCH), y, attr ) ;
				drawNumber( TRIGHT-5, y, g_eeGeneral.speakerPitch, attr) ; //, attr ? ~colour : colour ) ;
  			if ( attr )
				{
					CHECK_INCDEC_H_GENVAR( g_eeGeneral.speakerPitch, 1, 100) ;
				}	
      break ;
			case 3 :
				drawItem( (char *)PSTR(STR_HAPTICSTRENGTH), y, attr ) ;
				drawNumber( TRIGHT-5, y, g_eeGeneral.hapticStrength, attr) ; //, attr ? ~colour : colour ) ;
  			if ( attr )
				{
					CHECK_INCDEC_H_GENVAR( g_eeGeneral.hapticStrength, 0, 5 ) ;
				}	
      break ;
			case 4 :
				drawItem( (char *)XPSTR("Haptic Min Run"), y, attr ) ;
				drawNumber( TRIGHT-5, y, g_eeGeneral.hapticMinRun+20, attr) ; //, attr ? ~colour : colour ) ;
  			if ( attr )
				{
					CHECK_INCDEC_H_GENVAR( g_eeGeneral.hapticMinRun, 0, 20 ) ;
				}	
      break ;
			case 5 :
				drawItem( (char *)XPSTR(GvaString), y, attr ) ;
  			if( attr )
				{
#ifdef TOUCH
					if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
#else
					if ( checkForMenuEncoderBreak( event ) )
#endif
					{
//						SubMenuCall = 0x85 ;
						pushMenu(menuProcGlobalVoiceAlarm) ;
					}
				}
      break ;
			case 6 :
  			g_eeGeneral.preBeep = touchOnOffItem( g_eeGeneral.preBeep, y, PSTR(STR_BEEP_COUNTDOWN), attr, colour ) ;
      break ;
			case 7 :
  			g_eeGeneral.minuteBeep = touchOnOffItem( g_eeGeneral.minuteBeep, y, PSTR(STR_MINUTE_BEEP), attr, colour ) ;
      break ;
			case 8 :
				drawItem( (char *)XPSTR( "Welcome Type"), y, attr ) ;
				drawIdxText( y, XPSTR("\006System  NoneCustom"),g_eeGeneral.welcomeType, attr|LUA_RIGHT ) ; // 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
				if(attr) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.welcomeType, 2 ) ;
      break ;
			case 9 :
			{
				uint16_t oldBcolour = LcdBackground ;
				if ( g_eeGeneral.welcomeType == 2 )
				{
					Tevent = 0 ;
					drawItem( (char *)XPSTR( "FileName"), y, attr ) ;
  				if( attr )
					{
#ifdef TOUCH
						if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
#else
						if ( checkForMenuEncoderBreak( event ) )
#endif
						{
							VoiceFileType = VOICE_FILE_TYPE_USER ;
    				 	pushMenu( menuProcSelectVoiceFile ) ;
    					s_editMode = false ;
						}
						if ( event == EVT_ENTRY_UP )
						{
							if ( FileSelectResult == 1 )
							{
						 		copyFileName( (char *)g_eeGeneral.welcomeFileName, SelectedVoiceFileName, 8 ) ;
	  				 		eeDirty(EE_GENERAL) ;		// Save it
							}
						}
					} 
			 		LcdBackground = attr ? ~colour : colour ;
					alphaEditName( TRIGHT-8*FW-5, y+TVOFF, (uint8_t *)g_eeGeneral.welcomeFileName, sizeof(g_eeGeneral.welcomeFileName), attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
					LcdBackground = oldBcolour ;
					validateName( g_eeGeneral.welcomeFileName, sizeof(g_eeGeneral.welcomeFileName) ) ;
				}
			}
			case 10 :
			if ( ( ( k == 10 ) && ( g_eeGeneral.welcomeType == 2 ) ) || ( ( k == 9 ) && ( g_eeGeneral.welcomeType < 2 ) ) )
			{	
				uint8_t b ;
				drawItem( (char *)XPSTR( "Hardware Volume"), y, attr ) ;
				b = g_eeGeneral.unused_PPM_Multiplier ;
				drawNumber( TRIGHT-5, y, g_eeGeneral.unused_PPM_Multiplier, attr) ; //, attr ? ~colour : colour ) ;
		    if ( attr )
				{
					CHECK_INCDEC_H_GENVAR( g_eeGeneral.unused_PPM_Multiplier, 0, 127) ;
				}
				if ( b != g_eeGeneral.unused_PPM_Multiplier )
				{
					b = g_eeGeneral.unused_PPM_Multiplier ;
void I2C_set_volume( register uint8_t volume ) ;
					I2C_set_volume( b ) ;
				}
			}	
      break ;
		}
	}
}



void menuDisplay( uint8_t event )
{
	TITLE( PSTR(STR_Display) ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif

	static MState2 mstate2;
	uint32_t rows = 9 ;
	mstate2.check_columns(event, rows-1) ;
	 
//	uint32_t newVpos ;
  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
//	uint32_t k = 0 ;
//	uint16_t t_pgOfs ;
	uint8_t subN = 0 ;
	uint16_t colour = dimBackColour() ;
	uint8_t attr ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
//	t_pgOfs = evalHresOffset( sub ) ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
	 
	y = TTOP ;
	 
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_BRIGHTNESS), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, 100 - g_eeGeneral.bright, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
		uint8_t oldBrightness = 100 - g_eeGeneral.bright ;
		CHECK_INCDEC_H_GENVAR( oldBrightness, 0, 100) ; //5-10V
		if ( oldBrightness != 100 - g_eeGeneral.bright )
		{
			backlight_set( g_eeGeneral.bright = 100 - oldBrightness ) ;
		}
	}
  y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Off brightness"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, 100 - g_eeGeneral.bright_white, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
		uint8_t oldBrightness = 100 - g_eeGeneral.bright_white ;
		CHECK_INCDEC_H_GENVAR( oldBrightness, 0, 100) ; //5-10V
		g_eeGeneral.bright_white = 100 - oldBrightness ;
	}
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_LIGHT_SWITCH), y, attr ) ;
	DrawDrSwitches( TRIGHT-TRMARGIN, y+TVOFF/HVSCALE, g_eeGeneral.lightSw, attr|LUA_RIGHT ) ;
//	putsDrSwitchesColour( TRIGHT-TRMARGIN, y+TVOFF, g_eeGeneral.lightSw, LUA_RIGHT, fcolour, attr ? ~colour : colour ) ;
	if(attr)
	{
		CHECK_INCDEC_GENERALSWITCH( event, g_eeGeneral.lightSw, -MaxSwitchIndex, MaxSwitchIndex) ;
	}
  y += TFH ;
	subN += 1 ;
	
	for ( uint32_t i = 0 ; i < 2 ; i += 1 )
	{
		attr = (sub==subN) ? INVERS : 0 ;
		uint8_t b ;
		drawItem( (char *)(( i == 0) ? PSTR(STR_LIGHT_AFTER) : PSTR(STR_LIGHT_STICK)), y, attr ) ;
		b = ( i == 0 ) ? g_eeGeneral.lightAutoOff : g_eeGeneral.lightOnStickMove ;

    if( b )
		{
			drawNumber( TRIGHT-5-FW, y, b * 5, attr) ; //, attr ? ~colour : colour ) ;
			drawChar( TRIGHT-3-FW, y, 's', attr, attr ? ~colour : colour ) ;
    }
    else
		{
  		PUTS_ATT_COLOUR( TRIGHT-TRMARGIN, y+TVOFF, PSTR(STR_OFF), LUA_RIGHT, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
		}
    if ( attr )
		{
			CHECK_INCDEC_H_GENVAR_0( b, 600/5) ;
		}
		if ( i == 0 )
		{
			g_eeGeneral.lightAutoOff = b ;
		}
		else
		{
			g_eeGeneral.lightOnStickMove = b ;
		}
  	y += TFH ;
		subN += 1 ;
	}
	
	attr = (sub==subN) ? INVERS : 0 ;
	g_eeGeneral.flashBeep = g_eeGeneral.disableBtnLong = touchOnOffItem( g_eeGeneral.flashBeep, y, PSTR(STR_FLASH_ON_BEEP), attr, colour ) ;
  y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Background Colour"), y, attr ) ;
	if( attr )
	{
#ifdef TOUCH
		if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
#else
		if ( checkForMenuEncoderBreak( event ) )
#endif
		{
			pushMenu( menuBackground ) ;
		}
	}
 	y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Text Colour"), y, attr ) ;
	if( attr )
	{
#ifdef TOUCH
		if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
#else
		if ( checkForMenuEncoderBreak( event ) )
#endif
		{
			pushMenu( menuForeground ) ;
		}
	}
 	y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Theme"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, g_eeGeneral.selectedTheme, attr ) ;
	if( attr )
	{
		uint8_t oldTheme = g_eeGeneral.selectedTheme ;
		CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.selectedTheme, 3 ) ;
		if ( oldTheme != g_eeGeneral.selectedTheme )
		{
			checkTheme( &g_eeGeneral.theme[g_eeGeneral.selectedTheme] ) ;
			themeData *t = &g_eeGeneral.theme[oldTheme] ;
			t->backColour = g_eeGeneral.backgroundColour ;
			t->textColour = g_eeGeneral.textColour ;
			t->brightness = g_eeGeneral.bright ;
			t = &g_eeGeneral.theme[g_eeGeneral.selectedTheme] ;
			LcdBackground = g_eeGeneral.backgroundColour = t->backColour ;
			LcdForeground = g_eeGeneral.textColour = t->textColour ;
			g_eeGeneral.bright = t->brightness ;
			dimBackColour() ;
		}
	}
// 	y += FH ;
//	subN += 1 ;
	 
}


uint16_t scalerDecimal( coord_t y, uint16_t val, LcdFlags attr )
{
  PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF, val+1, 0 ) ;
	if (attr) val = checkIncDec16( val, 0, 2047, EE_MODEL);
	return val ;
}

static uint8_t s_scalerSource ;

void menuScaleOne(uint8_t event)
{
  TITLE( "SC  =") ;
	uint8_t index = s_currIdx ;
  PUTC_ATT( 6*FW, 0, index+'1', INVERS ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif

	static MState2 mstate2 ;
	uint32_t rows = 13 ;
	mstate2.check_columns(event, rows-1 ) ;
#ifdef TOUCH
	uint32_t newVpos ;
#endif

	lcd_hline( 0, TTOP, TRIGHT ) ;
	
  uint8_t sub = mstate2.m_posVert;
	uint16_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;
#ifndef TOUCH
	(void) t_pgOfs ;
#endif	 
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;

	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	uint16_t colour = dimBackColour() ;

	putsTelemetryChannel( 15*FW, 0, index+TEL_ITEM_SC1, 0, 0, TELEM_UNIT ) ;

	ScaleData *pscaler ;
	ExtScaleData *epscaler ;
	pscaler = &g_model.Scalers[index] ;
	epscaler = &g_model.eScalers[index] ;
	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	else if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		if ( TextResult )
		{
			if ( s_scalerSource )
			{
				epscaler->exSource = unmapPots( TextIndex ) ;
			}
			else
			{
				pscaler->source = unmapPots( TextIndex ) ;
			}
	    eeDirty(EE_MODEL) ;
		}
	}
  
  for( uint32_t k = 0 ; k < TLINES ; k += 1 )
	{
		uint16_t t ;
    uint16_t y = k * TFH + TTOP ;
    uint8_t i = k + s_pgOfs ;
		uint8_t attr = (sub==i ? INVERS : 0) ;
		uint16_t oldBcolour = LcdBackground ;
		switch(i)
		{
      case 0 :	// Source
			case 7 :	// exSource
			{	
				uint8_t x ;
				
				if ( i == 0 )
				{				
					drawItem( (char *)XPSTR("Source"), y, attr ) ;
					x = pscaler->source ;
				}
				else
				{
					drawItem( (char *)XPSTR("ex Source"), y, attr ) ;
					x = epscaler->exSource ;
				}
				saveEditColours( attr, colour ) ;
				putsChnRaw( (TRIGHT-TRMARGIN)*2, (y+TVOFF)*2, x, LUA_RIGHT ) ;
				restoreEditColours() ;
				if( attr )
				{
					x = mapPots( x ) ;
					if ( i == 0 )
					{
						pscaler->source = unmapPots( x ) ;
					}
					else
					{
						epscaler->exSource = unmapPots( x ) ;
					}
//					if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#ifdef TOUCH
					if ( checkForMenuEncoderBreak( event ) || handleEditIcon() )
#else
					if ( checkForMenuEncoderBreak( event ) )
#endif
					{
						// Long MENU pressed
						if ( i == 0 )
						{				
							x = pscaler->source ;
							s_scalerSource = 0 ;
						}
						else
						{
							x = epscaler->exSource ;
							s_scalerSource = 1 ;
						}
						TextIndex = mapPots( x ) ;
  				  TextType = TEXT_TYPE_SW_SOURCE ;
  				  killEvents(event) ;
						pushMenu(menuTextHelp) ;
					}
					else
					{
						x = checkIncDec16( x,0,NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots-1+1,EE_MODEL) ;
					}
				}
			}
			break ;
			case 1 :	// name
				drawItem( (char *)XPSTR("Scaler Name"), y, attr ) ;
				LcdBackground = attr ? ~colour : colour ;
				alphaEditName( TRIGHT-sizeof(pscaler->name)*FW-FW, y+TVOFF, (uint8_t *)pscaler->name, sizeof(pscaler->name), attr|ALPHA_NO_NAME, (uint8_t *)0 ) ;
	 			if ( AlphaEdited )
				{
					sortTelemText() ;				
				}
				LcdBackground = oldBcolour ;
			break ;
      case 2 :	// offset
				drawItem( (char *)PSTR(STR_OFFSET), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, pscaler->offset, attr) ; //, attr ? ~colour : colour ) ;
				if ( attr )
				{
					StepSize = 100 ;
					pscaler->offset = checkIncDec16( pscaler->offset, -32000, 32000, EE_MODEL ) ;
				}
			break ;
      case 3 :	// mult
				drawItem( (char *)XPSTR("Multiplier"), y, attr ) ;
				t = pscaler->mult + ( pscaler->multx << 8 ) ;
				saveEditColours( attr, colour ) ;
				t = scalerDecimal( y, t, attr ) ;
				restoreEditColours() ;
				pscaler->mult = t ;
				pscaler->multx = t >> 8 ;
			break ;
      case 4 :	// div
				drawItem( (char *)XPSTR("Divisor"), y, attr ) ;
				t = pscaler->div + ( pscaler->divx << 8 ) ;
				saveEditColours( attr, colour ) ;
				t = scalerDecimal( y, t, attr ) ;
				restoreEditColours() ;
				pscaler->div = t ;
				pscaler->divx = t >> 8 ;
			break ;
      case 5 :	// mod
				drawItem( (char *)XPSTR("Mod Value"), y, attr ) ;
				saveEditColours( attr, colour ) ;
				epscaler->mod = scalerDecimal( y, epscaler->mod, attr ) ;
				restoreEditColours() ;
			break ;
      case 6 :	// offsetLast
				drawItem( (char *)XPSTR("Offset At"), y, attr ) ;
				saveEditColours( attr, colour ) ;
				drawIdxText( y, XPSTR("\005FirstLast "), pscaler->offsetLast, attr|LUA_RIGHT ) ;
				restoreEditColours() ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->offsetLast, 1 ) ;
			break ;
			case 8 :
				drawItem( (char *)XPSTR("Function"), y, attr ) ;
			  if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( pscaler->exFunction, 0, 6 ) ;
				}
				saveEditColours( attr, colour ) ;
  			drawIdxText( y, XPSTR("\010--------Add     SubtractMultiplyDivide  Mod     Min     "), pscaler->exFunction, attr|LUA_RIGHT ) ;
				restoreEditColours() ;
			break ;
			case 9 :	// unit
				drawItem( (char *)XPSTR("Unit"), y, attr ) ;
				saveEditColours( attr, colour ) ;
				drawIdxText( y, XPSTR(UnitsString), pscaler->unit, attr|LUA_RIGHT ) ;
				restoreEditColours() ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->unit, 10 ) ;
			break ;
      case 10 :	// sign
				drawItem( (char *)XPSTR("Sign"), y, attr ) ;
				saveEditColours( attr, colour ) ;
  			PUTC_ATT( TRIGHT-TRMARGIN-FW, y+TVOFF, pscaler->neg ? '-' : '+', 0 ) ;
				restoreEditColours() ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->neg, 1 ) ;
			break ;
      case 11 :	// precision
				drawItem( (char *)XPSTR("Decimals"), y, attr ) ;
				saveEditColours( attr, colour ) ;
				PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF, pscaler->precision, 0 ) ;
				restoreEditColours() ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->precision, 2 ) ;
			break ;
      case 12 :	// Dest
				drawItem( (char *)XPSTR("Dest"), y, attr ) ;
#define NUM_SCALE_DESTS		17
				if( attr )
				{
					CHECK_INCDEC_H_MODELVAR( epscaler->dest, 0, NUM_SCALE_DESTS ) ;
				}
				saveEditColours( attr, colour ) ;
				drawIdxText( y, XPSTR(DestString), epscaler->dest, attr|LUA_RIGHT ) ;
				restoreEditColours() ;
			break ;
		}
	}

}


void menuProcScalers(uint8_t event)
{
	TITLE(XPSTR("Scalers"));
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	EditType = EE_MODEL ;
	uint32_t rows = NUM_SCALERS ;
	static MState2 mstate2;
 	event = mstate2.check_columns(event, rows-1 ) ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;

	uint8_t sub = mstate2.m_posVert ;
	uint16_t y = TTOP ;
  uint32_t k ;
#ifdef TOUCH
	uint32_t selected = 0 ;
#endif

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;
#endif

#ifdef TOUCH
	if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() || selected )
#else
	if ( checkForMenuEncoderBreak( event ) )
#endif
	{
		s_currIdx = sub ;
		killEvents(event);
    s_editMode = false ;
		pushMenu(menuScaleOne) ;
	}

	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;
	for (uint32_t i = 0 ; i < 8 ; i += 1 )
	{
		uint8_t attr = sub == i ? INVERS : 0 ;
		uint16_t t ;
    y = (i)*TFH +TTOP ;
    k = i ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}
		y += TVOFF ;
		PUTS_P( THOFF, y, XPSTR("SC") ) ;
  	PUTC( 2*FW+THOFF, y, k+'1' ) ;
  	PUTC( 9*FW, y, '+' ) ;
  	PUTC( 19*FW, y, '*' ) ;
  	PUTC( 25*FW, y, '/' ) ;
		putsChnRaw( (4*FW+THOFF)*2, (y)*2, g_model.Scalers[k].source, 0 ) ;
		PUTS_NUM( 15*FW+THOFF, y, g_model.Scalers[k].offset, 0 ) ;
		t = g_model.Scalers[k].mult + ( g_model.Scalers[k].multx << 8 ) ;
		PUTS_NUM( 23*FW-1+THOFF, y, t+1, 0 ) ;
		t = g_model.Scalers[k].div + ( g_model.Scalers[k].divx << 8 ) ;
		PUTS_NUM( 29*FW+THOFF, y, t+1, 0 ) ;
		
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
	}
}

void menuBindOptions(uint8_t event)
{
//	uint8_t b ;
	struct t_module *pModule = &g_model.Module[s_currIdx] ;
	TITLE(XPSTR("Bind Options"));
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2;
	uint32_t rows = 3 ;
	event = mstate2.check_columns( event, rows-1 ) ;
	
	uint32_t sub = mstate2.m_posVert ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

	if ( sub == 0 )
	{
#ifdef TOUCH
		if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
#else
		if ( checkForMenuEncoderBreak( event ) )
#endif
		{
#ifdef ACCESS
			if ( g_model.Module[s_currIdx].protocol == PROTO_ACCESS )
			{
				ModuleControl[s_currIdx].bindStep = BIND_START ;
				ModuleSettings[s_currIdx].mode = MODULE_MODE_BIND ;
				AccState = ACC_ACCST_BIND ;
				popMenu( 0 ) ;
			}
			else
#endif
			{
	  		BindRangeFlag[s_currIdx] = PXX_BIND ;		    	//send bind code
				chainMenu( menuRangeBind ) ;
			}
		}
	}
  
	lcd_hline( 0, TTOP, TRIGHT ) ;

	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;

	for (uint32_t i = 0 ; i < 3 ; i += 1 )
	{
		uint8_t attr = sub == i ? INVERS : 0 ;
    uint16_t y = i*TFH + TTOP ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}
		switch ( i )
		{
			case 0 :
				PUTS_P( THOFF, y+TVOFF, PSTR(STR_BIND) ) ;
			break ;
			case 1 :
//				pModule->highChannels = checkIndexed( TTOP+1*TFH+TVOFF, XPSTR("\0""\001""\012Chans 1-8 Chans 9-16"), pModule->highChannels, (sub==1) ) ;
  			PUTS_AT_IDX( THOFF/2, y+TVOFF, XPSTR("\012Chans 1-8 Chans 9-16"), pModule->highChannels, 0 ) ;
				if ( attr )
				{
					CHECK_INCDEC_H_MODELVAR( pModule->highChannels, 0, 1 ) ;
				}
			break ;
			case 2 :
//				pModule->disableTelemetry = checkIndexed( TTOP+2*TFH+TVOFF, XPSTR("\0""\001""\014Telemetry   No Telemetry"), pModule->disableTelemetry, (sub==2) ) ;
		  	PUTS_AT_IDX( THOFF/2, y+TVOFF, XPSTR("\014Telemetry   No Telemetry"), pModule->disableTelemetry, 0 ) ;
				if ( attr )
				{
					CHECK_INCDEC_H_MODELVAR( pModule->disableTelemetry, 0, 1 ) ;
				}
			break ;
		}
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
	}
}


void menuProcCurve(uint8_t event)
{
	TITLE( PSTR(STR_CURVES) ) ;
	static MState2 mstate2 ;
	EditType = EE_MODEL ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	uint32_t rows = 1+MAX_CURVE5+MAX_CURVE9-1+1+1+1 ;
	uint32_t t_pgOfs ;

#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	mstate2.check_columns(event, rows - 1 ) ;

  uint8_t sub = mstate2.m_posVert ;
	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;

	uint32_t newVpos ;
	newVpos = scrollBar( 395, TSCROLLTOP, 27, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

#ifdef TOUCH
	if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
#else
	if ( checkForMenuEncoderBreak( event ) )
#endif
	{
    s_curveChan = sub;
    killEvents(event);
		Tevent = 0 ;
    pushMenu(menuProcCurveOne);
	}

	lcd_hline( 0, TTOP, TMID-2*FH ) ;

  uint16_t y ;
	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;
  for ( uint32_t i = 0; i < TLINES ; i++ )
	{
    uint8_t k = i + t_pgOfs;
		y = TTOP + i *TFH ;
    uint8_t attr = sub == k ? INVERS : 0 ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, (TMID-2*FH)*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}
    PUTS_ATT( THOFF+FW*0, y+TVOFF, PSTR(STR_CV), 0 ) ;
    PUTS_NUM( (k<9) ? THOFF+FW*3-1 : THOFF+FW*4-2, y+TVOFF, k+1, 0 ) ;
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y + TFH, TMID-2*FH ) ;
  }
	s_curveChan = sub ;
	drawCurve( 100 ) ;
}



void menuProcTrainProtocol(uint8_t event)
{
	TITLE(PSTR(STR_Trainer));
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	
	static MState2 mstate2;
	uint32_t rows = 6 ;
	mstate2.check_columns(event, rows-1) ;

	int8_t  sub    = mstate2.m_posVert ;
	uint16_t y ;
	uint8_t subN = 0 ;
//	uint16_t colour = dimBackColour() ;
	uint8_t attr ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
	
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
	 
	y = TTOP ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Trainer Polarity"), y, attr ) ;
	drawIdxText( y, (char *)PSTR(STR_POS_NEG), g_model.trainPulsePol, attr|LUA_RIGHT ) ; // 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
	if(attr) CHECK_INCDEC_H_GENVAR_0( g_model.trainPulsePol, 1 ) ;
	y += TFH ;
 	subN += 1 ;
		
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Channels"), y, attr ) ;
	if (attr)
	{
 		CHECK_INCDEC_H_MODELVAR_0( g_model.ppmNCH, 12 ) ;
	}
	drawNumber( TRIGHT-5, y, g_model.ppmNCH + 4, attr) ; //, attr ? ~colour : colour ) ;
	y += TFH ;
 	subN += 1 ;
		
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_PPMFRAME_MSEC), y, attr ) ;
	if (attr)
	{
 		CHECK_INCDEC_H_MODELVAR( g_model.ppmFrameLength, -12, 12 ) ;
	}
	drawNumber( TRIGHT-TRMARGIN, y, g_model.ppmFrameLength*5 + 225, attr|PREC1) ; //, attr ? ~colour : colour ) ;
	y += TFH ;
 	subN += 1 ;
		
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Delay (uS)"), y, attr ) ;
	if (attr)
	{
		CHECK_INCDEC_H_MODELVAR( g_model.ppmDelay,-4,10);
	}
	drawNumber( TRIGHT-TRMARGIN, y, (g_model.ppmDelay*50)+300, attr) ; //, attr ? ~colour : colour ) ;
	y += TFH ;
 	subN += 1 ;
		
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_PPM_1ST_CHAN), y, attr ) ;
	if(attr) { CHECK_INCDEC_H_MODELVAR_0( g_model.startChannel, 16 ) ; }
	drawNumber( TRIGHT-TRMARGIN, y, g_model.startChannel + 1, attr) ; //, attr ? ~colour : colour ) ;
	y += TFH ;
 	subN += 1 ;
		
	attr = (sub==subN) ? INVERS : 0 ;
	uint8_t value = g_model.trainerProfile + 1 ;
	if ( g_model.traineron == 0 )
	{
		value = 0 ;
	}
	drawItem( (char *)PSTR(STR_Trainer), y, attr ) ;
  if(value)
	{
		drawNumber( TRIGHT-TRMARGIN, y, value-1, attr) ; //, attr ? ~colour : colour ) ;
		PUTC( THOFF+9*FW, y+TVOFF, '(' ) ;
    PUTC( THOFF+16*FW, y+TVOFF, ')' ) ;
		validateText( g_eeGeneral.trainerProfile[value-1].profileName, 6 ) ;
		PUTS_ATT_N( THOFF+10*FW, y+TVOFF, (const char *)g_eeGeneral.trainerProfile[value-1].profileName, 6, 0 ) ;
	}
  else
	{
  	PUTS_ATT_COLOUR( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR(STR_OFF), 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~LcdBackground : LcdBackground ) ;
	}
  if(sub==subN)
	{
		CHECK_INCDEC_H_MODELVAR_0( value, 4 ) ;
		MaskRotaryLong = 1 ;
		if ( ( value > 0) && ( checkForMenuEncoderLong( event ) ) )
		{
			SingleExpoChan = 1 ;
			s_expoChan = value-1 ;
			s_editMode = 0 ;
      killEvents(event);
      pushMenu(menuProcTrainer) ; 
		}
	}

	g_model.traineron = value > 0 ;
	if ( value )
	{
		g_model.trainerProfile = value - 1 ;
	}
}

int8_t getAndSwitch( SKYCSwData &cs ) ;
void displayLogicalSwitch( uint16_t x, uint16_t y, uint32_t index ) ;
extern uint8_t saveHpos ;
extern const uint8_t SwitchFunctionMap[] ;
extern uint8_t InverseBlink ;
void editOneSwitchItem( uint8_t event, uint32_t item, uint32_t index ) ;

void menuSwitchOne(uint8_t event)
{
	static MState2 mstate2 ;
	uint8_t attr ;
	uint32_t index ;
	uint32_t rows = 7 ;
	if ( Clipboard.content == CLIP_SWITCH )
	{
		rows += 1 ;
	}
	index = s_currIdx ;
	SKYCSwData &cs = g_model.customSw[index] ;
	uint8_t cstate = CS_STATE(cs.func) ;
	uint32_t extra = 0 ;
	uint32_t Vextra = 0 ;
  if( cstate == CS_2VAL )
	{
		rows += 1 ;
		extra = 1 ;
		Vextra = TFH ;
	}

	TITLEP( XPSTR("SWITCH") ) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif

	mstate2.check_columns(event, rows-1 ) ;
//	uint8_t x = TITLEP( PSTR(STR_EDIT_MIX));

	uint8_t sub = mstate2.m_posVert ;

	if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		sub = mstate2.m_posVert = saveHpos ;
		
		if ( TextResult )
		{
			if ( sub == 0 )
			{
				cs.func = SwitchFunctionMap[TextIndex] ;
			}
			else if ( sub == 1 )
			{
				cs.v1u = unmapPots( TextIndex ) ;
			}
			else
			{
				cs.v2u = unmapPots( TextIndex ) ;
			}
	 	  eeDirty(EE_MODEL) ;
		}
	}


	//write SW name here
	displayLogicalSwitch( 11*FW, 0, index ) ;

	uint16_t y ;
	uint16_t colour = dimBackColour() ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
	 
	y = TTOP ;


	attr = 0 ;
	if ( sub == 0 )
	{
		attr = INVERS ;
	}
	drawItem( (char *)XPSTR("Function"), y, attr ) ;
	drawIdxText( y, (char *)PSTR(CSWITCH_STR), cs.func, attr|LUA_RIGHT ) ; // 0, attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;

	y += TFH ;
	
	drawItem( (char *)XPSTR("V1"), y, sub == 1 ) ;

	y += TFH ;
	
	drawItem( (char *)XPSTR(""), y, sub == 2 ) ;
  if( (cstate == CS_VOFS) || (cstate == CS_2VAL) )
	{
		PUTS_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("val") ) ;
	}
	else
	{
		PUTS_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("V2") ) ;
	}
	 
	y += TFH ;

	if ( extra )
	{
		drawItem( (char *)XPSTR("V3"), y, sub == 3 ) ;
	 
		y += TFH ;
	}
	 
	drawItem( (char *)XPSTR(""), y, sub == 3+extra ) ;
	y += TFH ;
	
	drawItem( (char *)XPSTR("Delay"), y, sub == 4+extra ) ;
	y += TFH ;
	
	drawItem( (char *)XPSTR("Ex.Func"), y, sub == 5+extra ) ;
	y += TFH ;
	
	drawItem( (char *)XPSTR("Copy"), y, sub == 6+extra ) ;

	if ( Clipboard.content == CLIP_SWITCH )
	{
		y += TFH ;
		drawItem( (char *)XPSTR("Paste"), y, sub == 7+extra ) ;
	}
	 
  if( (cstate == CS_VOFS) || (cstate == CS_2VAL) )
  {
//		PUTS_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("val") ) ;
		saveEditColours( (sub == 1), colour ) ;
		putsChnRaw( (TRIGHT-TRMARGIN)*2, (TTOP+TFH+TVOFF)*2, cs.v1u, LUA_RIGHT ) ;
		restoreEditColours() ;

    if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
		{
			int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
			saveEditColours( (sub == 2), colour ) ;
			putsTelemetryChannel( TRIGHT-TRMARGIN, TTOP+2*TFH+TVOFF, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, LUA_RIGHT, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT ) ;
			restoreEditColours() ;
		}
    else
		{
			drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, cs.v2, 0) ; //, sub == 2 ? ~colour : colour ) ;
		}
		if ( extra )
		{
    	if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
			{
				int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, (int8_t)cs.bitAndV3 ) ;
				saveEditColours( (sub == 3), colour ) ;
				putsTelemetryChannel( TRIGHT-TRMARGIN, TTOP+3*TFH+TVOFF, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, LUA_RIGHT, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT ) ;
				restoreEditColours() ;
			}
    	else
			{
				drawNumber( TRIGHT-TRMARGIN, TTOP+3*TFH, (int8_t)cs.bitAndV3, 0) ; //, sub == 3 ? ~colour : colour ) ;
			}
		}
  }
  else if(cstate == CS_VBOOL)
  {
		saveEditColours( sub==1, colour ) ;
   	putsDrSwitches(TRIGHT-TRMARGIN, TTOP+TFH+TVOFF, cs.v1, LUA_RIGHT ) ;
		restoreEditColours() ;
		saveEditColours( sub==2, colour ) ;
   	putsDrSwitches(TRIGHT-TRMARGIN, TTOP+2*TFH+TVOFF, cs.v2, LUA_RIGHT ) ;
		restoreEditColours() ;
  }
  else if(cstate == CS_VCOMP)
  {
		saveEditColours( sub==1, colour ) ;
		putsChnRaw( (TRIGHT-TRMARGIN)*2, (TTOP+TFH+TVOFF)*2, cs.v1u, LUA_RIGHT ) ;
		restoreEditColours() ;
		saveEditColours( sub==2, colour ) ;
		putsChnRaw( (TRIGHT-TRMARGIN)*2, (TTOP+2*TFH+TVOFF)*2, cs.v2u, LUA_RIGHT ) ;
		restoreEditColours() ;
  }
	else if(cstate == CS_TIMER)
	{
		int8_t x ;
		uint8_t att = 0 ;
		x = cs.v1 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
	  PUTS_P( THOFF,TTOP+TFH+TVOFF, XPSTR("Off") ) ;
		drawNumber( TRIGHT-TRMARGIN, TTOP+TFH, x+1, att) ; //, (sub == 1) ? ~colour : colour ) ;
		att = 0 ;
		x = cs.v2 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
	  PUTS_P( THOFF,TTOP+2*TFH+TVOFF, XPSTR("On ") ) ;
		drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, x+1, att) ; //, (sub == 2) ? ~colour : colour ) ;
		cs.exfunc = 0 ;
	}
	else if(cstate == CS_TMONO)
	{
		saveEditColours( sub==1, colour ) ;
   	putsDrSwitches(TRIGHT-TRMARGIN, TTOP+TFH+TVOFF, cs.v1, LUA_RIGHT ) ;
		restoreEditColours() ;
		uint8_t att = 0 ;
		int8_t x ;
		x = cs.v2 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
		PUTS_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("Time") ) ;
		drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, x+1, att) ; //, (sub == 2) ? ~colour : colour ) ;
		cs.exfunc = 0 ;
	}
	else// cstate == U16
	{
		uint16_t x ;
		x = (uint8_t) cs.v2 ;
		x |= cs.bitAndV3 << 8 ;
		saveEditColours( sub==1, colour ) ;
		putsChnRaw( (TRIGHT-TRMARGIN)*2, (TTOP+TFH+TVOFF)*2, cs.v1u, LUA_RIGHT ) ;
		restoreEditColours() ;
		drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, x, 0) ; //, (sub == 2) ? ~colour : colour ) ;
	}

	saveEditColours( sub==3+extra, colour ) ;
 	putsDrSwitches(TRIGHT-TRMARGIN, TTOP+3*TFH+TVOFF+Vextra, getAndSwitch( cs ), LUA_RIGHT ) ;
	restoreEditColours() ;

	PUTS_AT_IDX( THOFF, TTOP+3*TFH+TVOFF+Vextra, XPSTR("\003ANDOR XOR"), cs.exfunc, 0 ) ;

	PUTS_AT_IDX_COLOUR( TRIGHT-TRMARGIN, TTOP+5*TFH+TVOFF+Vextra, XPSTR("\003ANDOR XOR"), cs.exfunc, LUA_RIGHT, ( sub == 5+extra ) ? ~LcdForeground : LcdForeground ) ; //, ( sub == 5+extra ) ? ~colour : colour ) ;

	drawNumber( TRIGHT-TRMARGIN, TTOP+4*TFH+Vextra, g_model.switchDelay[index], PREC1) ; //, (sub == 4+extra) ? ~colour : colour ) ;

	if ( sub <= 5+extra )
	{
		if ( extra )
		{
			if ( sub == 3 )
			{
				g_posHorz = 1 ;
				sub = 2 ;
			}
			else
			{
				if ( sub > 3 )
				{
					sub -= 1 ;
				}
			}
		}
		editOneSwitchItem( event, sub, index ) ;
		if ( cstate == CS_2VAL )
		{
			if ( g_posHorz )
			{
				g_posHorz = 0 ;
				sub = 3 ;
			}
			else
			{
				if ( sub >= 3 )
				{
					sub += 1 ;
				}
			}
		}
	}
	TextResult = 0 ;

	if ( sub == 6+extra )
	{
#ifdef TOUCH	
			if ( checkForMenuEncoderLong( event ) || handleSelectIcon() )
#else
			if ( checkForMenuEncoderLong( event ) )
#endif
		{
			Clipboard.clipswitch = cs ;
			Clipboard.content = CLIP_SWITCH ;
		}
	}

	if ( sub == 7+extra )
	{
#ifdef TOUCH	
		if ( checkForMenuEncoderLong( event ) || handleSelectIcon() )
#else
		if ( checkForMenuEncoderLong( event ) )
#endif
		{
			cs = Clipboard.clipswitch ;
		}
	}
}

void menuProcSwitches(uint8_t event)
{
	uint32_t rows = NUM_SKYCSW ;
	TITLE(PSTR(STR_CUST_SWITCH));
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	EditType = EE_MODEL ;
	static MState2 mstate2;
#ifndef TOUCH
	event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	event = mstate2.check_columns(event, rows-1 ) ;

#ifdef TOUCH
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif

	uint16_t y ;
	uint8_t k = 0 ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t t_pgOfs ;

	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	selected = newVert & 0x0100 ;

	if ( handleSelectIcon() )
	{
		selected = 1 ;
	}

	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub >= t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif
		
	lcd_hline( 0, TTOP, TRIGHT ) ;

	for(uint32_t i = 0 ; i < TLINES ; i += 1 )
	{
    y=(i)*TFH +TTOP ;
    k = i + t_pgOfs ;
    uint8_t attr = sub == k ? INVERS : 0 ;
		uint8_t m = k ;
    SKYCSwData &cs = g_model.customSw[m] ;

		//write SW names here
		displayLogicalSwitch( 0, y+TVOFF/2, m ) ;

		uint16_t oldBcolour = LcdBackground ;
		uint16_t oldFcolour = LcdForeground ;
    
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 12*TSCALE, y*TSCALE+2, (TRIGHT-12)*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}

//		if ( k < NUM_SKYCSW )
//		{
			PUTS_AT_IDX( 2*FW+4, y+TVOFF/2, PSTR(CSWITCH_STR), cs.func, 0 ) ;

  	  uint8_t cstate = CS_STATE(cs.func);

	  	if( (cstate == CS_VOFS) || (cstate == CS_2VAL) )
    	{
				putsChnRaw( (10*FW-6)*2, (y+TVOFF/2)*2, cs.v1u  , 0);
	  	  if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
 				{
					int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
					putsTelemetryChannel( 18*FW-8, y+TVOFF/2, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT);
				}
    	  else
				{
    	    drawNumber( 18*FW-9, y, cs.v2  ,0);
				}
    	}
    	else if(cstate == CS_VBOOL)
    	{
    	  putsDrSwitches(10*FW-6, y+TVOFF/2, cs.v1  , 0);
    	  putsDrSwitches(14*FW-7, y+TVOFF/2, cs.v2  , 0);
    	}
    	else if(cstate == CS_VCOMP)
    	{
    	  putsChnRaw( (10*FW-6)*2, (y+TVOFF/2)*2, cs.v1u  , 0);
    	  putsChnRaw( (14*FW-4)*2, (y+TVOFF/2)*2, cs.v2u  , 0);
    	}
			else if(cstate == CS_TIMER)
			{
				int8_t x ;
				uint8_t att = 0 ;
				x = cs.v1 ;
				if ( x < 0 )
				{
					x = -x-1 ;
					att = PREC1 ;
				}
	  	  PUTS_ATT_LEFT( y+TVOFF/2, PSTR(STR_15_ON) ) ;
    	  PUTS_NUM( 13*FW-5, y+TVOFF/2, x+1  ,att ) ;
				att = 0 ;
				x = cs.v2 ;
				if ( x < 0 )
				{
					x = -x-1 ;
					att = PREC1 ;
				}
    	  PUTS_NUM( 18*FW-3, y+TVOFF/2, x+1 , att ) ;
			}
			else if(cstate == CS_TMONO)
			{
    	  putsDrSwitches(10*FW-6, y+TVOFF, cs.v1  , 0);
				uint8_t att = 0 ;
				int8_t x ;
				x = cs.v2 ;
				if ( x < 0 )
				{
					x = -x-1 ;
					att = PREC1 ;
				}
    	  PUTS_NUM( 17*FW-2, y+TVOFF, x+1 , att ) ;
			}
			else// cstate == U16
			{
				uint16_t x ;
				x = cs.v2u ;
				x |= cs.bitAndV3 << 8 ;
    	  putsChnRaw( (10*FW-6-FW)*2, (y+TVOFF/2), cs.v1  , 0);
    	  PUTS_NUM_N( 18*FW-9, y+TVOFF/2, x  , 0,5);
			}
			putsDrSwitches( 18*FW-3, y+TVOFF/2, getAndSwitch( cs ),0 ) ;

			PUTS_P( 22*FW, y+TVOFF/2, XPSTR("Delay" ) ) ;
			PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF/2, g_model.switchDelay[m], PREC1 ) ;

			if ( attr )
			{
#ifdef TOUCH	
				if ( ( checkForMenuEncoderBreak( event ) ) || selected )
#else
				if ( checkForMenuEncoderBreak( event ) )
#endif
				{
					// Long MENU pressed
			    s_currIdx = sub ;
//  				TextType = 0 ;
					pushMenu(menuSwitchOne) ;
  		  	s_editMode = false ;
				}
			}
//		}
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}
}

extern char LastItem[] ;

void menuSensors(uint8_t event)
{
	TITLE(XPSTR("Sensors"));
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2 ;
	uint32_t rows = NUMBER_EXTRA_IDS + 6 + 4 + 1 ;
	
#ifndef TOUCH
	Tevent = event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	Tevent = event = mstate2.check_columns(event, rows-1 ) ;
	
#ifdef TOUCH
//	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif

	coord_t y ;
	uint32_t k = 0 ;
	uint32_t sub = mstate2.m_posVert ;
	uint32_t t_pgOfs ;

	t_pgOfs = evalHresOffset( sub ) ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
//	selected = newVert & 0x0100 ;

//	if ( handleSelectIcon() )
//	{
//		selected = 1 ;
//	}

	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub >= t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif
	 
//	uint32_t subN = 0 ;

	lcd_hline( 0, TTOP, TRIGHT ) ;
//	uint16_t oldBcolour = LcdBackground ;
	uint16_t colour = dimBackColour() ;

	for(uint32_t i = 0 ; i < TLINES ; i += 1 )
	{
    y=(i)*TFH +TTOP ;
    k = i + t_pgOfs ;
    LcdFlags attr = sub == k ? INVERS : 0 ;

		if ( k < 6 )
		{
			setLastIdx( (char *) PSTR(STR_TELEM_ITEMS), k + 69 ) ;
			drawItem( (char *)LastItem, y, attr ) ;
			
//			LcdBackground = attr ? ~colour : colour ;
			saveEditColours( attr, colour ) ;
			alphaEditName( TRIGHT-TRMARGIN, y+TVOFF/2, (uint8_t *)&g_model.customTelemetryNames[k*4], 4, attr|ALPHA_NO_NAME|LUA_RIGHT, (uint8_t *)XPSTR( "Custom Name") ) ;
			restoreEditColours() ;
 			if ( AlphaEdited )
			{
				sortTelemText() ;				
			}
//			LcdBackground = oldBcolour ;
		}
		else if ( k < 10 )
		{
			setLastIdx( (char *) PSTR(STR_TELEM_ITEMS), k + 83 - 6 ) ;
			drawItem( (char *)LastItem, y, attr ) ;
			
//			LcdBackground = attr ? ~colour : colour ;
			saveEditColours( attr, colour ) ;
			alphaEditName( TRIGHT-TRMARGIN, y+TVOFF/2, (uint8_t *)&g_model.customTelemetryNames2[(k-6)*4], 4, attr|ALPHA_NO_NAME|LUA_RIGHT, (uint8_t *)XPSTR( "Custom Name") ) ;
			restoreEditColours() ;
			if ( AlphaEdited )
			{
				sortTelemText() ;				
			}
//			LcdBackground = oldBcolour ;
		}
		else
		{
			uint32_t j ;
			j = k - 10 ;
			if ( j < g_model.extraSensors )
			{
				drawItem( (char *)"", y, attr ) ;
				PUT_HEX4( THOFF*2, y+TVOFF, g_model.extraId[j].id ) ;
//				PUTS_AT_IDX( 11*FW, y, XPSTR(DestString), g_model.extraId[j].dest, attr ) ;
				drawIdxText( y, (char *) XPSTR(DestString), g_model.extraId[j].dest, attr ) ;
 			  if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.extraId[j].dest, NUM_SCALE_DESTS ) ;
 			  }
			}	
			else
			{
				if ( j < NUMBER_EXTRA_IDS )
				{
					drawItem( (char *)HyphenString, y, attr ) ;
// 			  	if(attr)
//					{				
//						lcd_char_inverse( 0, y+TVOFF/2, 4*FW, 0 ) ;
//					}
				}
				else
				{
					drawItem( (char *)XPSTR("Clear All"), y, attr ) ;
					PUTS_ATT( TRIGHT-TRMARGIN-9*FW, y+TVOFF, XPSTR("MENU LONG"), 0 ) ;
 			  	if(attr)
					{				
						if ( checkForMenuEncoderLong( event ) )
						{
							uint32_t i ;
							for ( i = 0 ; i < NUMBER_EXTRA_IDS ; i += 1 )
							{
								g_model.extraId[i].dest = 0 ;
							}
							g_model.extraSensors = 0 ;
						}
					}
				}
			}
		}
	}
}


extern uint8_t TelemMap[] ;

extern const uint8_t TelemSize  ;
extern union t_sharedMemory SharedMemory ;

const uint8_t LogLookup[] =
{
 1, 2, 3, 4, 7, 8, 9,10,11,12,
13,16,17,18,19,20,21,22,23,24,
25,26,27,28,29,30,31,32,33,34,
35,36,37,38,39,40,41,42,43,44,
45,48,49,50,51,52,53,54,55,56,
57,58,59,60,61,62,63,64,65,66,
67,68,69,70,71,72,73,74,78,79,
80,81,82,83,84,85,86,89//,90
} ;
#define LOG_LOOKUP_SIZE	78

const uint8_t LogRateMap[] = { 3, 2, 0, 1 } ;

#ifdef BIG_SCREEN
#define LOG_OFF_0			0
#else
#define LOG_OFF_0			0
#endif

//uint8_t AlphaLogLookup[sizeof(LogLookup)] ;

#if LOG_LOOKUP_SIZE > ALPHA_LOG_SIZE
//ERROR "ALPHA_LOG_SIZE too small"
#endif

void menuLogging(uint8_t event)
{
	TITLE(XPSTR("Logging"));
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2;
	uint32_t rows = 1+sizeof(LogLookup)+5+4+3+1 ;
#ifndef TOUCH
	Tevent = event = checkPageMove( event, &mstate2.m_posVert, rows - 1 ) ;
#endif
	Tevent = event = mstate2.check_columns(event, rows-1 ) ;

#ifdef TOUCH
//	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif

#ifdef BIG_SCREEN
	DisplayOffset = LOG_OFF_0 ;
#endif

	coord_t y ;
	uint32_t sub = mstate2.m_posVert ;
//	uint32_t blink = InverseBlink ;
	uint32_t t_pgOfs ;
  uint32_t k ;

	t_pgOfs = evalHresOffset( sub ) ;


#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
//	selected = newVert & 0x0100 ;

//	if ( handleSelectIcon() )
//	{
//		selected = 1 ;
//	}

	newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, rows-(TLINES-1), t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub >= t_pgOfs + TLINES - 1 )
		{
			mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
		}
	}
#endif

	if ( event == EVT_ENTRY )
	{
		uint32_t i ;
		uint32_t j ;
		uint32_t k ;
		uint32_t m ;
		k = 0 ;
		for ( i = 0 ; i < TelemSize ; i += 1 )
		{
			m = 0 ;
			for ( j = 0 ; j < sizeof(LogLookup) ; j += 1 )
			{
				if ( TelemMap[i] == LogLookup[j]-1 )
				{
					SharedMemory.AlphaLogLookup[k] = TelemMap[i] ;
					m = 1 ;
					break ;
				}
			}
			if ( m )
			{
				k += 1 ;
			}
		}
	}

	lcd_hline( 0, TTOP, TRIGHT ) ;
//	uint16_t oldBcolour = LcdBackground ;
	uint16_t colour = dimBackColour() ;

  for( uint32_t j = 0 ; j < TLINES ; j += 1 )
	{
    y = j*TFH + TTOP ;
    k = j + t_pgOfs ;
    LcdFlags attr = (sub==k) ? INVERS : 0 ;

		if ( k == 0 )
		{
			drawItem( (char *)PSTR(STR_LOG_SWITCH), y, attr ) ;
			if ( attr & INVERS )
			{
				if ( s_editMode && BLINK_ON_PHASE )
				{
					attr = 0 ;
				}
			}
			saveEditColours( attr, colour ) ;
			g_model.logSwitch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, g_model.logSwitch, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			restoreEditColours() ;
		}
		else if ( k == 1 )
		{
			drawItem( (char *)PSTR(STR_LOG_RATE), y, attr ) ;
			drawIdxText( y, (char *)XPSTR("\0041.0s2.0s0.5s0.2s"), g_model.logRate, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
//			PUTS_AT_IDX( 15*FW+LOG_OFF_0, y, XPSTR("\0041.0s2.0s0.5s0.2s"), g_model.logRate, attr ) ;
 			if(attr)
			{
				g_model.logRate = checkOutOfOrder( g_model.logRate, (uint8_t *)LogRateMap, 4 ) ;
   		}
		}
		else if ( k == 2 )
		{
			g_model.logNew = touchOnOffItem( g_model.logNew, y, (char *)XPSTR("New File"), attr, colour ) ;
//			drawItem( (char *)XPSTR("New File"), y, attr ) ;
//			g_model.logNew = onoffItem( g_model.logNew, y, attr ) ;
		}
		else if ( k == 3 )
		{
			drawItem( (char *)XPSTR("Data Timeout(s)"), y, attr ) ;
			drawNumber( TRIGHT-TRMARGIN, y, g_model.telemetryTimeout + 25, attr|PREC1) ; //, attr ? ~hcolour : hcolour ) ;
//			lcd_xlabel_decimal( 18*FW+LOG_OFF_0, y, g_model.telemetryTimeout + 25, attr|PREC1, XPSTR("Data Timeout(s)") ) ;
			if(attr)
			{
				CHECK_INCDEC_H_MODELVAR_0( g_model.telemetryTimeout, 75 ) ;
   		}
		}
		else if ( k == 4 )
		{
			drawItem( (char *)XPSTR("Select None"), y, attr ) ;
			if ( attr )
			{
#ifdef TOUCH
				uint32_t activated = handleSelectIcon() ;
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) || activated )
#else				
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) )
#endif
				{
  				s_editMode = 0 ;
					g_model.LogDisable[0] = 0xFFFFFFFF ;
					g_model.LogDisable[1] = 0xFFFFFFFF ;
					g_model.LogDisable[2] = 0xFFFFFFFF ;
					g_model.LogDisable[3] = 0xFFFFFFFF ;
				}
			}
		}
		else if ( k == 5 )
		{
			drawItem( (char *)XPSTR("Select All"), y, attr ) ;
			if ( attr )
			{
#ifdef TOUCH
				uint32_t activated = handleSelectIcon() ;
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) || activated )
#else				
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) )
#endif
				{
  				s_editMode = 0 ;
					g_model.LogDisable[0] = 0 ;
					g_model.LogDisable[1] = 0 ;
					g_model.LogDisable[2] = 0 ;
					g_model.LogDisable[3] = 0 ;
				}
			}
		}
		else if ( k == 6 )
		{
			drawItem( (char *)XPSTR("Select Active"), y, attr ) ;
			if ( attr )
			{
#ifdef TOUCH
				uint32_t activated = handleSelectIcon() ;
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) || activated )
#else				
    		if ( (event == EVT_KEY_BREAK(BTN_RE)) || (event == EVT_KEY_BREAK(KEY_MENU)) )
#endif
				{
  				s_editMode = 0 ;
					uint32_t i ;
					uint32_t idx ;
					for ( i = 0 ;  i < sizeof(LogLookup) ; i += 1 )
					{
						idx = LogLookup[i]-1 ;
						if ( idx > TELEM_GAP_START )
						{
							idx += 8 ;
						}
#ifdef BITBAND_SRAM_REF
						uint32_t *p ;
						uint32_t index = LogLookup[i]-1 ;
						p = (uint32_t *) (BITBAND_SRAM_BASE + ((uint32_t)&g_model.LogDisable - BITBAND_SRAM_REF) * 32) ;
						p[index] = !telemItemValid( idx ) ;
#else
						uint32_t offset ;
						uint32_t bit ;
						uint32_t index = LogLookup[i]-1 ;
						offset = index >> 5 ;
						bit = 1 << (index & 0x0000001F) ;
						if ( telemItemValid( idx ) )
						{
							g_model.LogDisable[offset]	&= ~bit ;
						}
						else
						{
							g_model.LogDisable[offset]	|= bit ;
						}
#endif
					}
				}
			}
		}
		else
		{
			uint32_t index = LogLookup[k-7]-1 ;
#ifndef BITBAND_SRAM_REF
			uint32_t bit ;
			uint32_t offset ;
			uint32_t value ;
#endif
			if ( k == 7+sizeof(LogLookup) )
			{
				index = LOG_LAT ;
				drawItem( (char *)XPSTR("Lat"), y, attr ) ;
				if ( TelemetryDataValid[FR_GPS_LAT] )
				{
					putTick( 60, y + TVOFF ) ;
//   				lcd_putc( 122+LOG_OFF_0, y, '\202');
				}
			}
			else if ( k == 8+sizeof(LogLookup) )
			{
				index = LOG_LONG ;
				drawItem( (char *)XPSTR("Long"), y, attr ) ;
				if ( TelemetryDataValid[FR_GPS_LONG] )
				{
					putTick( 60, y + TVOFF ) ;
//   				lcd_putc( 122+LOG_OFF_0, y, '\202');
				}
			}
			else if ( k == 9+sizeof(LogLookup) )
			{
				index = LOG_BTRX ;
				drawItem( (char *)XPSTR("BtRx"), y, attr ) ;
			}
			else if ( k > 9+sizeof(LogLookup) )
			{
				index = LOG_STK_THR + k - (10+sizeof(LogLookup)) ;
				char *p ;
				
				p = XPSTR("Stk-THR\0Stk-AIL\0Stk-ELE\0Stk-RUD") ;
				p += 8 * (index - LOG_STK_THR) ;
				drawItem( p, y, attr ) ;
//				PUTS_AT_IDX( LOG_OFF_0, y, XPSTR("\007Stk-THRStk-AILStk-ELEStk-RUD"), index - LOG_STK_THR, attr ) ;

//				PUTS_NUM( 10*FW, y, index, 0 ) ;
//  			PUTS_NUM( 12*FW, y, index - LOG_STK_THR, 0 ) ;

				putTick( 60, y + TVOFF ) ;
//				lcd_putc( 122+LOG_OFF_0, y, '\202');
			}
			else
			{
				uint32_t idx ;
				idx = SharedMemory.AlphaLogLookup[k-7] ;
//				index = idx ;
//				idx = index ;
//				if ( idx > TELEM_GAP_START )
//				{
//					idx += 8 ;
//				}
//				if ( index >= TELEM_GAP_START + 8 )
//				{
//					index -= 8 ;
//				}
//				putsAttIdxTelemItems( LOG_OFF_0, y, idx+1, 0 ) ;
				
				attr &= ~TSSI_TEXT ;
				setLastTelemIdx( idx+1 ) ;
				drawItem( (char *)LastItem, y, attr ) ;

				if ( idx > TELEM_GAP_START )
				{
					idx += 8 ;
				}
				
				if ( index < HUBDATALENGTH )
				{
					if ( telemItemValid( idx ) )
					{
						putTick( 60, y + TVOFF ) ;
//    				lcd_putc( 122+LOG_OFF_0, y, '\202');
					}
//					int8_t i ;
//					if ( index >= TELEM_GAP_START + 8 )
//					{
//						index -= 8 ;
//					}
//					i = TelemIndex[index] ;
//					if ( i >= 0 )
//					{
//						if (TelemetryDataValid[i] )
//						{
//    					lcd_putc( 122, y, '\202');
//						}
//					}
//					else
//					{
//						if ( TelemValid[index] == 0 )
//						{
//    					lcd_putc( 122, y, '\202');
//						}
//					}
////					else
////					{
////						if ( ( i >= CELL_1 ) && ( i <= CELL_12 ) )
////						{
////							i += FR_CELL1 - CELL_1 ;
////							if (TelemetryDataValid[i] )
////							{
////    						lcd_putc( 122, y, '\202');
////							}
////						}
////					}
					if ( attr )
					{
						putsTelemetryChannel( 14*FW+LOG_OFF_0, 0, idx, get_telemetry_value(idx), 0, TELEM_UNIT ) ;
					}
				}
				index = idx ;
				if ( index > TELEM_GAP_START )
				{
					index -= 8 ;
				}
			}
#ifdef BITBAND_SRAM_REF
			uint32_t *p ;
			p = (uint32_t *) (BITBAND_SRAM_BASE + ((uint32_t)&g_model.LogDisable - BITBAND_SRAM_REF) * 32) ;
			p[index] = offonItem( p[index], y, attr ) ;
#else
			offset = index >> 5 ;
			bit = 1 << (index & 0x0000001F) ;
			value = 1 ;
			if ( g_model.LogDisable[offset] & bit )
			{
				value = 0 ;
			}
			value = touchOnOffItemMultiple( value, y, attr, colour ) ;
//			value = offonItem( value, y, attr ) ;
			if ( value == 0 )
			{
				g_model.LogDisable[offset]	|= bit ;
			}
			else
			{
				g_model.LogDisable[offset]	&= ~bit ;
			}
#endif
		}
	}
}

extern int8_t s_inputMaxSel ;
extern int8_t s_currDestCh;
extern uint8_t s_moveItemIdx;
void inputpopup( uint8_t event ) ;
void moveInput(uint8_t idx, uint8_t dir) ; //true=inc=down false=dec=up - Issue 49
void insertInput( uint8_t idx, uint8_t copy ) ;
void editOneInput(uint8_t event) ;
bool reachInputsCountLimit() ;
struct te_InputsData *getActiveInput( uint8_t channel ) ;




void menuProcInputs(uint8_t event)
{
	TITLE( XPSTR("Inputs") ) ;
	EditType = EE_MODEL ;
	static MState2 mstate2 ;

	if ( s_moveMode )
	{
		int8_t moveByRotary ;
		moveByRotary = qRotary() ;		// Do this now, check_simple destroys rotary data
		if ( moveByRotary )
		{
			if ( moveByRotary > 0 )
			{
				event = EVT_KEY_FIRST(KEY_DOWN) ;
			}
			else
			{
				event = EVT_KEY_FIRST(KEY_UP) ;
			}
		}
		uint8_t v = mstate2.m_posVert ;
		if ( ( ( v == 0 ) && ( event == EVT_KEY_FIRST(KEY_UP) ) ) 
				 || ( ( v == s_inputMaxSel ) && ( event == EVT_KEY_FIRST(KEY_DOWN) ) ) )
		{
			event = 0 ;
		}
		Tevent = event ;
	}

	if ( !PopupData.PopupActive )
	{
		mstate2.check_columns(event,s_inputMaxSel-1) ;
	}

  uint32_t sub = mstate2.m_posVert ;
	uint32_t menulong = 0 ;

  switch(event)
  {
	  case EVT_ENTRY:
      s_moveMode = false ;
  	break;
    
		case EVT_KEY_FIRST(KEY_MENU):
	  case EVT_KEY_BREAK(BTN_RE):
			if ( s_moveMode )
			{
	  	  s_moveMode = false ;
  	  	s_editMode = false ;
				RotaryState = ROTARY_MENU_UD ;
  	  	break;
			}
			// Else fall through    
			if ( !PopupData.PopupActive )
			{
		  	killEvents(event);
				Tevent = 0 ;			// Prevent changing weight to/from Gvar
				menulong = 1 ;
			}
  	break;
  }

#ifdef TOUCH
	if ( handleSelectIcon() )
	{
		s_currIdx = sub ;
		menulong = 1 ;
	}
#endif

	uint32_t t_pgOfs = evalOffset( sub ) ;
    
	if ( PopupData.PopupActive )
	{
		Tevent = 0 ;
	}


	if ( s_moveMode )
	{
		int32_t dir ;
		
		if ( ( dir = (event == EVT_KEY_FIRST(KEY_DOWN) ) ) || event == EVT_KEY_FIRST(KEY_UP) )
		{
			moveInput( s_curItemIdx, dir ) ; //true=inc=down false=dec=up - Issue 49
		}
	}

  uint32_t input_index = 0 ;
  uint32_t current = 0 ;
	s_expoChan = 255 ;

  for ( uint32_t chan = 1 ; chan <= NUM_INPUTS ; chan += 1 )
	{
		struct te_InputsData *pinput = &g_model.inputs[input_index] ;

    if ( t_pgOfs <= current && current-t_pgOfs < SCREEN_LINES-1)
		{
    	uint8_t attr = 0 ;
			if (sub == current)
			{
				attr = INVERS ;
			}
			displayInputName( 1, (current-t_pgOfs+1)*FHPY, chan, attr ) ;

//			lcd_img( 1, (current-t_pgOfs+1)*FHPY, IconInput, 0, 0 ) ;
//			PUTS_NUM_N(3*FW, (current-t_pgOfs+1)*FHPY, chan, attr + LEADING0, 2) ;
    }

//		uint32_t firstInput = input_index ;

		if (input_index < NUM_INPUT_LINES && /* pmd->srcRaw && */ pinput->chn == chan)
		{
    	do
			{
				if (t_pgOfs <= current )
				{
					if ( current-t_pgOfs < SCREEN_LINES-1 )
					{
    	  	  coord_t y = (current-t_pgOfs+1)*FHPY ;
    				LcdFlags attr = 0 ;
						
						if ( !s_moveMode && (sub == current) )
						{
							s_curItemIdx = input_index ;
							s_currDestCh = chan ;		// For insert
							if ( menulong )
							{
								PopupData.PopupIdx = 0 ;
								PopupData.PopupActive = 1 ;
								event = 0 ;		// Kill this off
							}
							if ( PopupData.PopupActive == 0 )
							{
    						attr = INVERS ;
							}
						}

//        	  if(firstMix != mix_index) //show prefix only if not first mix
//						{
//        	 		lcd_putsAttIdx( 4*FW-8, y, XPSTR("\001+*R"),pmd->mltpx,0 ) ;
//						}

						if (sub == current)
						{
							s_expoChan = pinput->chn ;
						}
#ifdef USE_VARS
						if ( g_model.vars )
						{
							if ( pinput->varForWeight )
							{
								displayVarName( (12)*FW, y, pinput->weight, attr|LUA_RIGHT ) ;
							}
							else
							{
								PUTS_NUM( 12*FW, y, pinput->weight, attr ) ;
							}
						}
						else
#endif
						{
							int16_t temp ;
							temp = pinput->weight ;
							if (temp > 100)
							{
								// A gvar
//#if MULTI_GVARS
//								if ( g_model.flightModeGvars )
//								{
//									temp -= 113 ;
//								}
//								else
//#endif
								{
									temp -= 110 ;
								}
								if ( temp < 0 )
								{
									temp = -temp - 1 ;
  								PUTC_ATT(8*FW-4*FW, y,'-',attr) ;
								}
								dispGvar( 12*FW-3*FW, y, temp+1, attr ) ;
							}
							else
							{
								PUTS_NUM( 12*FW, y, temp, attr ) ;
							}
						}
						if ( pinput->srcRaw >= 128 )
						{
#ifdef COLOUR_DISPLAY
							putsChnOpRaw( (13*FW-FW/2)*2 ,y*2, MIX_3POS, pinput->srcRaw-128, 0, attr ) ;
#else
							putsChnOpRaw( 13*FW-FW/2 ,y, MIX_3POS, pinput->srcRaw-128, 0, attr ) ;
#endif
						}
						else
						{
#ifdef COLOUR_DISPLAY
							putsChnOpRaw( (13*FW-FW/2)*2 ,y*2, pinput->srcRaw, 0, 0, attr ) ;
#else
							putsChnOpRaw( 13*FW-FW/2 ,y, pinput->srcRaw, 0, 0, attr ) ;
#endif
						}
		        
						putsDrSwitches(17*FW-FW/2, y, pinput->swtch,attr) ;
						if ( s_moveMode )
						{
							if ( s_moveItemIdx == input_index )
							{
								lcd_char_inverse( 4*FW, y, 21*FW, 0 ) ;
								s_curItemIdx = input_index ;
								sub = mstate2.m_posVert = current ;
							}
						}
		 
					}
				}
//				else
//				{
					
//				}
				current += 1 ;
				input_index += 1 ;
				pinput = &g_model.inputs[input_index] ;

    	} while ( (input_index < NUM_INPUT_LINES && /* pmd->srcRaw && */ pinput->chn == chan) ) ;
		}
		else
		{
			if (sub == current)
			{
				s_currDestCh = chan ;		// For insert
				s_curItemIdx = input_index ;
//				lcd_rect( 0, (current-t_pgOfs+1)*FHPY-1, 25, 9 ) ;
				if ( menulong )		// Must need to insert here
				{
      		if ( !reachInputsCountLimit())
      		{
      			insertInput(s_curItemIdx, 0 ) ;
  	    		s_moveMode=false;
	      		pushMenu(editOneInput) ;
						break ;
      		}
				}
		 	}
			current += 1 ;
		}
	}

#if LCD_W > 212
	if ( s_expoChan != 255 )
	{
		uint32_t x = XD+2*FW ;
		pushPlotType( PLOT_COLOUR ) ;
		drawFunction( x, GRAPH_FUNCTION_INPUT ) ;
		struct te_InputsData *pinput = getActiveInput( s_expoChan ) ;
		if ( pinput )
		{
				int16_t x512 = getInputSourceValue( pinput ) ;
				int16_t y512 = evalInput( s_expoChan, x512 ) ;
				int16_t xv = (x512 * WCHART + RESX/2) / RESX + x ;
  			int16_t yv = Y0 - (y512 * WCHART + RESX/2) / RESX ;

				lcd_vline( xv, yv-6, 13 ) ;
				lcd_hline( xv-6, yv, 13 ) ;

				popPlotType() ;
		}
	}
#endif
	 
	if ( PopupData.PopupActive )
	{
		Tevent = event ;
		inputpopup( event ) ;
    s_editMode = false;
	}

	s_inputMaxSel = current ;
}

