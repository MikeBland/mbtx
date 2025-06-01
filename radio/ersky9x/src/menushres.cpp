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
#define NUM_SCALE_DESTS		17

uint32_t doPopup( const char *list, uint16_t mask, uint8_t width, uint8_t event ) ;
uint32_t checkForMenuEncoderBreak( uint8_t event ) ;
uint32_t checkForMenuEncoderLong( uint8_t event ) ;
//uint8_t checkIndexed( uint8_t y, const char *s, uint8_t value, uint8_t edit ) ;
void copyFileName( char *dest, char *source, uint32_t size ) ;
uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext ) ;
//uint8_t onoffMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition ) ;
void menuSelectVoiceFile(uint8_t event) ;
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
void putsChnOpRaw( coord_t x, coord_t y, uint8_t source, uint8_t switchSource, uint8_t output, LcdFlags attr ) ;
void drawSmallGVAR( uint16_t x, uint16_t y ) ;
int16_t gvarDiffValue( uint16_t x, uint16_t y, int16_t value, uint32_t attr, uint8_t event ) ;
void deleteVoice(VoiceAlarmData *pvad) ;
int8_t qRotary() ;
void menuGlobalVoiceAlarm(uint8_t event) ;
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
int32_t checkTouchArea( uint32_t x, uint32_t y, uint32_t w, uint32_t h ) ;


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
	uint16_t oldFcolour ;
	oldFcolour = LcdForeground ;
	LcdForeground = colour ;
	PUTS_NUM_N( (style & TELEM_VALUE_RIGHT) ? x+62 : x, y, val, att, 5 ) ; // , colour, bgColour ) ;
	LcdForeground = oldFcolour ;
}

//uint8_t hronoffMenuItem( coord_t x, uint8_t value, coord_t y, const prog_char *s, LcdFlags edit )
//{
//	PUTS_ATT_LEFT(y, s) ;
//	menu_lcd_onoff( x, y, value, edit ) ;
//	if(edit)
//	{
//		if ( ( EditColumns == 0 ) || ( s_editMode ) )
//		{
//			value = checkIncDec( value, 0, 1, EditType ) ;
//		}
//	}
//	return value ;
//}



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
	uint32_t y = 0 ;
//	uint32_t count = *((uint8_t *) Playout[value]) ;	// # boxes
	uint32_t count = LayoutRcount[value] ;
	uint8_t *p ;

	if ( options & HIRES_OPT_TRIMS )
	{
		w = 216 ;
		h = 108 ;
		x = 12 ;
	}
	if ( options & HIRES_OPT_MNAME )
	{
		y = 8 ;
		h -= y ;
	}
//	else
//	{
//		return (uint8_t *) Playout[value] ;
//	}

	p = Rlayout ;
	*p++ = count ;
	*p++ = x ;
	*p++ = y ;
	switch ( value )
	{
		case 0 :
			*p++ = w ;
		break ;
		case 1 :
			*p++ = w/2 ;
			*p++ = h ;
			*p++ = x+w/2 ;
			*p++ = y ;
			*p++ = w/2 ;
		break ;
		case 2 :
			w /= 2 ;
			*p++ = w ;
			*p++ = h ;
			h /= 2 ;
			x += w ;
			*p++ = x ;
			*p++ = y ;
			*p++ = w ;
			*p++ = h ;
			*p++ = x ;
			*p++ = y+h ;
			*p++ = w ;
		break ;
		case 3 :
			w /= 2 ;
			*p++ = w ;
			*p++ = h/2 ;
			*p++ = x ;
			*p++ = y+h/2 ;
			*p++ = w ;
			*p++ = h/2 ;
			*p++ = x+w ;
			*p++ = y ;
			*p++ = w ;
		break ;
		case 4 :
			w /= 2 ;
			h /= 2 ;
			*p++ = w ;
			*p++ = h ;
			*p++ = x ;
			*p++ = y+h ;
			*p++ = w ;
			*p++ = h ;
			x += w ;			
			*p++ = x ;
			*p++ = y ;
			*p++ = w ;
			*p++ = h ;
			*p++ = x ;
			*p++ = y+h ;
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
			*p++ = y+h ;
			*p++ = w ;
			*p++ = h ;
			*p++ = x ;
			*p++ = y+h+h ;
			*p++ = w ;
			*p++ = h ;
			
			x += w ;			 
			*p++ = x ;
			*p++ = y ;
			*p++ = w ;
			
			if ( value == 7 )
			{
				*p++ = h ;
				*p++ = x ;
				*p++ = y+h ;
				*p++ = w ;
				h += h ;
			}
			else
			{
				if ( value == 5 )
				{
					*p++ = h ;
					*p++ = x ;
					*p++ = y+h ;
					*p++ = w ;
					*p++ = h ;
				}
				else
				{			
					*p++ = h+h ;
				}
				*p++ = x ;
				*p++ = y+h+h ;
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


uint32_t strippedStrLen( char *string, uint32_t length )
{
	uint32_t len = length - 1 ;
	while ( ( string[len] == ' ' ) || ( string[len] == 0 ) )
	{
		len -= 1 ;
		if ( len == 0 )
		{
			break ;
		}
	}
	len += 1 ;
	return len ;
}

//uint16_t HiresTime ;
//uint16_t HiresSavedTime ;
//uint32_t HiresCountTime ;

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
	if ( display->options & HIRES_OPT_MNAME )
	{
		uint32_t len = strippedStrLen( g_model.name, MODEL_NAME_LEN ) ;
		uint16_t www = getTextWidth( g_model.name, len, BOLD ) ;
		www = (LCD_W-www) / 2 ;
		lcdDrawSizedText( www, 0, g_model.name, len, BOLD ) ;
	}
	popPlotType() ;
}



void menuHiresTest(uint8_t event)
{
	static MState2 mstate2 ;
	event = mstate2.check_columns( event, 0 ) ;

	displayHiresScreen( s_currSubIdx ) ;	// First screen
}

void menuHiresItem(uint8_t event)
{
	uint32_t red ;
	uint32_t green ;
	uint32_t blue ;
	const char *options ;
	TITLE(XPSTR("Set Item")) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2 ;
	uint32_t rows = 8 ;
	event = mstate2.check_columns( event, rows-1 ) ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t subN = 0 ;
	LcdFlags attr ;
	coord_t y = TTOP ;
	uint32_t index = s_currSubIdx ;
	struct t_hiResDisplay *display ;
#ifdef TOUCH
	uint32_t selected = 0 ;
#endif

	display = &g_model.hiresDisplay[index] ;
	
	if ( s_currIdx > 5 )
	{
		s_currIdx = 5 ;
	}

	if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		if ( TextResult )
		{
			display->boxes[s_currIdx].item = unmapPots( TextIndex ) ;
	    eeDirty(EE_MODEL) ;
		}
	}
	 
#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	selected = newVert & 0x0100 ;
#endif
	 
	options = ( display->layout == 0 ) ? "\006Value Image Bars  Timer Sticks" : "\005ValueImageBars Timer " ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Type"), y, attr ) ;
	drawIdxText( y, (char *)options, display->boxes[s_currIdx].type, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
 	if(attr)
	{
		CHECK_INCDEC_H_MODELVAR( display->boxes[s_currIdx].type, 0, (display->layout == 0) ? 4 : 3 ) ;
	}
	subN += 1 ;
	y += TFH ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	if ( display->boxes[s_currIdx].type == HIRES_TYPE_BARS )
	{
		drawItem( (char *)XPSTR("Start"), y, attr ) ;
		drawNumber( TRIGHT-TRMARGIN, y, display->boxes[s_currIdx].item+1, attr ) ;
  	if (attr)
		{
			CHECK_INCDEC_H_MODELVAR_0( display->boxes[s_currIdx].item, 30 ) ;
		}
	}
	else
	{
		drawItem( (char *)XPSTR("Item"), y, attr ) ;
		if ( display->boxes[s_currIdx].type == HIRES_TYPE_TIMER )
		{
			drawIdxText( y, (char *)"\006Timer1Timer2", display->boxes[s_currIdx].item, attr|LUA_RIGHT ) ; //, attr ? ~LcdForeground : LcdForeground ) ;
		 	if(attr)
			{
				CHECK_INCDEC_H_MODELVAR( display->boxes[s_currIdx].item, 0, 1 ) ;
			}
		}
		else
		{
			if ( display->boxes[s_currIdx].type == HIRES_TYPE_IMAGE )
			{
  			lcdDrawText( (TRIGHT-TRMARGIN)*2, y*2+TVOFF, HyphenString, LUA_RIGHT ) ;
				attr = 0 ;
			}
			else if ( display->boxes[s_currIdx].item )
			{
			  putsChnRaw( (TRIGHT-TRMARGIN)*2, (y+TVOFF)*2, display->boxes[s_currIdx].item, LUA_RIGHT, attr ? ~LcdForeground : LcdForeground, attr ? ~DimBackColour : DimBackColour ) ;
//				putsChnRaw( (FW*9)*2, (y)*2, display->boxes[s_currIdx].item, attr | TSSI_TEXT ) ;
			}
			else
			{
  			lcdDrawText( (TRIGHT-TRMARGIN)*2, y*2+TVOFF, HyphenString, LUA_RIGHT ) ;
			}
  		if (attr)
			{ 
				uint8_t val = display->boxes[s_currIdx].item ;
				val = mapPots( val ) ;
#ifdef TOUCH
				if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) || handleSelectIcon() || selected )
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
	y += TFH ;
	
	red = display->boxes[s_currIdx].colour >> 11 ;
	green = ( display->boxes[s_currIdx].colour >> 6 ) & 0x1F ;
	blue = display->boxes[s_currIdx].colour & 0x1F ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Text Red"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, red, attr ) ;
  if(attr) CHECK_INCDEC_H_MODELVAR_0( red, 31 ) ;
	y += TFH ;
	subN += 1 ;
	 
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Green"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, green, attr ) ;
  if(attr) CHECK_INCDEC_H_MODELVAR_0( green, 31 ) ;
	y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Blue"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, blue, attr ) ;
  if(attr) CHECK_INCDEC_H_MODELVAR_0( blue, 31 ) ;
	display->boxes[s_currIdx].colour = ( red << 11 ) | ( green << 6 ) | ( blue ) ;
	y += TFH ;
	subN += 1 ;

	red = display->boxes[s_currIdx].bgColour >> 11 ;
	green = ( display->boxes[s_currIdx].bgColour >> 6 ) & 0x1F ;
	blue = display->boxes[s_currIdx].bgColour & 0x1F ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Back Red"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, red, attr ) ;
  if(attr) CHECK_INCDEC_H_MODELVAR_0( red, 31 ) ;
	y += TFH ;
	subN += 1 ;
	 
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Green"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, green, attr ) ;
  if(attr) CHECK_INCDEC_H_MODELVAR_0( green, 31 ) ;
	y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Blue"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, blue, attr ) ;
  if(attr) CHECK_INCDEC_H_MODELVAR_0( blue, 31 ) ;
	display->boxes[s_currIdx].bgColour = ( red << 11 ) | ( green << 6 ) | ( blue ) ;
	y += TFH ;
	
	drawItem( (char *)XPSTR("Example"), y, 0 ) ;
	lcdDrawSolidFilledRectDMA( (TRIGHT-32)*2, 2*(y+(TFH-10)/2), 60, 20, display->boxes[s_currIdx].bgColour ) ;
	PUTS_ATT_N_COLOUR( (TRIGHT-30), (y+(TFH-10)/2), "Text", 4, 0, display->boxes[s_currIdx].colour ) ; //, display->boxes[s_currIdx].bgColour ) ;
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
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
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
#ifdef TOUCH
		if ( checkTouchArea( x, y, w, h ) > 0 )
		{
			if ( sub == i )
			{
				if ( event == 0)
				{
					event = EVT_KEY_BREAK(BTN_RE) ;
				}
			}
			sub = mstate2.m_posVert = i ;
		}
#endif
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

//#ifdef TOUCH

void menuHires(uint8_t event)
{
	TITLE(PSTR(STR_CUSTOM_DISP)) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	static MState2 mstate2 ;
	struct t_hiResDisplay *display ;
	coord_t y = TTOP ;
	LcdFlags attr ;
#ifdef TOUCH
	uint32_t selected = 0 ;
#endif

//	DisplayOffset = HRES_OFF_0 ;

	display = &g_model.hiresDisplay[s_currSubIdx] ;
	
	uint32_t rows = 7 ;
	event = mstate2.check_columns( event, rows-1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows+1, 0, 1 ) ;
	if ( newSelection >= 0 )
	{
		if ( newSelection > 1 )
		{
			newSelection -= 1 ;
		}
	}
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	selected = newVert & 0x0100 ;
#endif

	lcd_hline( 0, TTOP, TRIGHT ) ;

	attr = (sub==0) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Screen"), y, attr ) ;
	drawNumber( TRIGHT-TRMARGIN, y, s_currSubIdx, attr) ; //, attr ? ~hcolour : hcolour ) ;
	if ( attr )
	{
		CHECK_INCDEC_H_MODELVAR_0( s_currSubIdx, 1 ) ;
	}
	y += 2*TFH ;

	attr = (sub==1) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Layout"), y, attr ) ;
	lcdDrawSolidFilledRectDMA( TMID*TSCALE, (y-TFH)*TSCALE+2, (TRIGHT-TMID)*TSCALE, TFH*TSCALE, attr ? ~DimBackColour : DimBackColour ) ;

	uint16_t colour = attr ? ~LcdForeground : LcdForeground ;
	switch ( display->layout )
	{
		case 0 :
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2, 32, 16, colour ) ;
		break ;
		case 1 :
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2, 16, 16, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2, 16, 16, colour ) ;
		break ;
		case 2 :
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2, 16, 16, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2, 16, 8, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2+8, 16, 8, colour ) ;
		break ;
		case 3 :
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2, 16, 8, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2+8, 16, 8, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2, 16, 16, colour ) ;
		break ;
		case 4 :
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2, 16, 8, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2, 16, 8, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2+8, 16, 8, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2+8, 16, 8, colour ) ;
		break ;
		case 5 :
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2+5, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2+10, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2+5, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2+10, 16, 5, colour ) ;
		break ;
		case 6 :
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2+5, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2+10, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2, 16, 10, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2+10, 16, 5, colour ) ;
		break ;
		case 7 :
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2+5, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16, TTOP+TFH+(TFH*2-16)/2+10, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2, 16, 5, colour ) ;
			lcd_rectColour( TMID+(TRIGHT-TMID)/2-16+16, TTOP+TFH+(TFH*2-16)/2+5, 16, 10, colour ) ;
		break ;
	}

	if ( attr )
	{
//		lcd_char_inverse( HRES_OFF_0+48, 3*FH-1, 36, s_editMode ? BLINK : 0, 18 ) ;
		CHECK_INCDEC_H_MODELVAR_0( display->layout, NUM_HIRES_DESIGN-1 ) ;
	}

	y += TFH ;

	attr = (sub==2) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Edit Contents"), y, attr ) ;
	if ( attr )
	{
#ifdef TOUCH
		if (checkForMenuEncoderBreak( event ) || handleSelectIcon() || selected )
#else
		if (checkForMenuEncoderBreak( event ) )
#endif
		{
			// To Do			
      pushMenu( menuHiresContent ) ;
		}
	}	
	
	y += TFH ;
	
	uint32_t x ;
	x = display->options & HIRES_OPT_TRIMS ;
	attr = (sub==3) ? INVERS : 0 ;
	x = touchOnOffItem( x, y, (char *)XPSTR("Include Trims"), attr, DimBackColour ) ;
	display->options = (display->options & ~HIRES_OPT_TRIMS ) | x ;
	y += TFH ;
	
	x = display->options & HIRES_OPT_BORDERS ;
	attr = (sub==4) ? INVERS : 0 ;
	x = touchOnOffItem( x, y, (char *)XPSTR("Display Borders"), attr, DimBackColour ) ;
	display->options = (display->options & ~HIRES_OPT_BORDERS ) | ( x ? HIRES_OPT_BORDERS : 0 ) ;
	y += TFH ;
	
	x = display->options & HIRES_OPT_MNAME ;
	attr = (sub==5) ? INVERS : 0 ;
	x = touchOnOffItem( x, y, (char *)XPSTR("Display Model Name"), attr, DimBackColour ) ;
	display->options = (display->options & ~HIRES_OPT_MNAME ) | ( x ? HIRES_OPT_MNAME : 0 ) ;
	y += TFH ;
	
	attr = (sub==6) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Test"), y, attr ) ;
	
	if ( attr )
	{
#ifdef TOUCH
		if (checkForMenuEncoderBreak( event ) || handleSelectIcon() || selected )
#else
		if (checkForMenuEncoderBreak( event ) )
#endif
		{
      pushMenu( menuHiresTest ) ;
		}
	}	
}

//#else

//void menuHires(uint8_t event)
//{
//	TITLE(PSTR(STR_CUSTOM_DISP)) ;
//	static MState2 mstate2 ;
//	struct t_hiResDisplay *display ;

//	DisplayOffset = HRES_OFF_0 ;

//	display = &g_model.hiresDisplay[s_currSubIdx] ;
	
//	uint32_t rows = 6 ;
//	event = mstate2.check_columns( event, rows ) ;
	
//	uint8_t sub = mstate2.m_posVert ;

//	PUTS_ATT_LEFT( 2*FHPY, XPSTR("Screen") ) ;
//	PUTS_NUM( PARAM_OFS+2*FW+HRES_OFF_0, 2*FH, s_currSubIdx, (sub==0) ? InverseBlink:0 ) ;

//	PUTS_ATT_LEFT( 3*FHPY, XPSTR("Layout") ) ;
//	pushPlotType( PLOT_BLACK ) ;
//	switch ( display->layout )
//	{
//		case 0 :
//			lcd_rect( HRES_OFF_0+50, 3*FH, 32, 16 ) ;
//		break ;
//		case 1 :
//			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 16 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 16 ) ;
//		break ;
//		case 2 :
//			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 16 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 8 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH+8, 16, 8 ) ;
//		break ;
//		case 3 :
//			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 8 ) ;
//			lcd_rect( HRES_OFF_0+50, 3*FH+8, 16, 8 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 16 ) ;
//		break ;
//		case 4 :
//			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 8 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 8 ) ;
//			lcd_rect( HRES_OFF_0+50, 3*FH+8, 16, 8 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH+8, 16, 8 ) ;
//		break ;
//		case 5 :
//			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50, 3*FH+5, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50, 3*FH+10, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH+5, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH+10, 16, 5 ) ;
//		break ;
//		case 6 :
//			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50, 3*FH+5, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50, 3*FH+10, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 10 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH+10, 16, 5 ) ;
//		break ;
//		case 7 :
//			lcd_rect( HRES_OFF_0+50, 3*FH, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50, 3*FH+5, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50, 3*FH+10, 16, 5 ) ;
			
//			lcd_rect( HRES_OFF_0+50+16, 3*FH, 16, 5 ) ;
//			lcd_rect( HRES_OFF_0+50+16, 3*FH+5, 16, 10 ) ;
//		break ;
//	}

////	{
////		uint8_t *l ;
////		uint32_t j ;
////		uint32_t i ;
////		uint16_t x, y, w, h ;
////		l = pickLayout( HiResDisplay.layout ) ;
////		j = *l++ ;
////		for ( i = 0 ; i < j ; i += 1 )
////		{
////#define HR_DIV 6
////			x = (*l++ + HR_DIV/2) / HR_DIV ;
////			y = (*l++ + HR_DIV/2) / HR_DIV ;
////			w = (*l++ + HR_DIV/2) / HR_DIV ;
////			h = (*l++ + HR_DIV/2) / HR_DIV ;
////			lcd_rect( x+100, y+2*FH+1, w, h ) ;
////		}
////	}


//	popPlotType() ;
//	PUTS_ATT_LEFT( 6*FHPY, XPSTR("Edit Contents") ) ;
//	uint32_t x ;
//	x = display->options & HIRES_OPT_TRIMS ;
//	x = onoffMenuItem( x, 7*FHPY, XPSTR("Include Trims"), ( sub == 3 ) ) ;
//	display->options = (display->options & ~HIRES_OPT_TRIMS ) | x ;
//	x = display->options & HIRES_OPT_BORDERS ;
//	x = onoffMenuItem( x, 8*FHPY, XPSTR("Display Borders"), ( sub == 4 ) ) ;
//	display->options = (display->options & ~HIRES_OPT_BORDERS ) | ( x ? HIRES_OPT_BORDERS : 0 ) ;
//	x = display->options & HIRES_OPT_MNAME ;
//	x = onoffMenuItem( x, 9*FH, XPSTR("\006Display Model Name"), ( sub == 5 ) ) ;
//	display->options = (display->options & ~HIRES_OPT_MNAME ) | ( x ? HIRES_OPT_MNAME : 0 ) ;
	
//	PUTS_ATT_LEFT( 9*FHPY, XPSTR("Test") ) ;
	
//	if ( sub == 0 )
//	{
//		CHECK_INCDEC_H_MODELVAR_0( s_currSubIdx, 1 ) ;
//	}
//	else if ( sub == 1 )
//	{
//		lcd_char_inverse( HRES_OFF_0+48, 3*FH-1, 36, s_editMode ? BLINK : 0, 18 ) ;
////		lcd_char_inverse( 98, 2*FH-1, 288/HR_DIV, s_editMode ? BLINK : 0, 132/HR_DIV ) ;
//		CHECK_INCDEC_H_MODELVAR_0( display->layout, NUM_HIRES_DESIGN-1 ) ;
//	}
//	else if ( sub == 2 )
//	{
//		lcd_char_inverse( HRES_OFF_0+0, 6*FHPY, 13*6, 0, 9 ) ;
//		if (checkForMenuEncoderBreak( event ) )
//		{
//			// To Do			
//      pushMenu( menuHiresContent ) ;
//		}
//	}	
//	else if ( sub == 5 )
//	{
//		lcd_char_inverse( HRES_OFF_0+0, 9*FHPY, 4*6, 0, 9 ) ;
//		if (checkForMenuEncoderBreak( event ) )
//		{
//      pushMenu( menuHiresTest ) ;
//		}
//	}	
//}

//#endif


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
//		lcdDrawNumber( 13*FWCOLOUR, STATUS_VERTICAL+3, HiresSavedTime/200, LEFT|PREC1, 4 ) ;
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

void drawIdxTextAtX( coord_t x, uint16_t y, char *s, uint32_t index, uint16_t mode )
{
	uint8_t length ;
	length = *s++ ;
	
//	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
	uint16_t fcolour = LcdForeground ;
	if ( mode & INVERS )
	{
#ifndef TOUCH
		if ( ! ( s_editMode && BLINK_ON_PHASE ) )
#endif
		{
			fcolour = ~LcdForeground ;
		}
	}
	lcdDrawSizedText( x*2, y, s+length*index, length, LUA_RIGHT, fcolour ) ;
//  lcd_putsnAttColour( TRIGHT-length*FW-TRMARGIN, y+TVOFF, s+length*index, length, 0, fcolour ) ;
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
#ifndef TOUCH
		if ( ! ( s_editMode && BLINK_ON_PHASE ) )
#endif
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

//#endif // nTOUCH



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
//	uint32_t blink ;
  uint32_t t_pgOfs ;
#ifdef TOUCH
	uint32_t newVpos ;
	uint32_t selected = 0 ;
#endif

	t_pgOfs = evalHresOffset( sub ) ;
	 
#ifdef TOUCH	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	selected = newVert & 0x0100 ;
	
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

//	blink = InverseBlink ;
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
#ifdef TOUCH
				uint16_t fcolour = (attr && subSub==0) ? ~LcdForeground : LcdForeground ;
#else
				uint16_t fcolour = LcdForeground ;
				if (attr && subSub==0)
				{
					if ( ! ( s_editMode && BLINK_ON_PHASE ) )
					{
						fcolour = ~LcdForeground ;
					}
				}
#endif
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
#ifdef TOUCH
			if (checkForMenuEncoderBreak( event ) || handleSelectIcon() || selected )
#else
			if (checkForMenuEncoderBreak( event ) )
#endif
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
  		drawIdxText( y, XPSTR("\0050.25s 0.5s 1.0s 1.5s 2.0s 3.0s"), g_model.dsmVario, attr|LUA_RIGHT ) ; // , attr ? ~LcdForeground : LcdForeground ) ; //, attr ? ~colour : colour ) ;
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
		drawChar( TRIGHT-TRMARGIN-FW, y, g_model.frskyComPort + '1', attr ) ; //, DimBackColour ) ;
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
//    attr = PREC1 ;
		if ( attr )
		{
      CHECK_INCDEC_H_MODELVAR_0( g_model.FASoffset, 15 ) ;
		}
//  	PUTS_NUM( PARAM_OFS+5*FW+5, y, g_model.FASoffset, attr ) ;
		drawNumber( TRIGHT-TRMARGIN, y, g_model.FASoffset, attr|PREC1 ) ;
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
			attr = InverseBlink ;
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






//void putsTrimMode( coord_t x, coord_t y, uint8_t phase, uint8_t idx, uint8_t att ) ;
//void alphaEditName( uint8_t x, uint8_t y, uint8_t *name, uint8_t len, uint16_t type, uint8_t *heading ) ;











#ifdef TOUCH
uint32_t editTimer( uint8_t sub, uint8_t event )
#else
void editTimer( uint8_t sub, uint8_t event )
#endif
{
	uint8_t subN ;
	uint8_t timer ;
	uint16_t colour = dimBackColour() ;
#ifdef TOUCH
	uint32_t rows = 7 ;
#endif
#ifdef HAPTIC
 #ifdef TOUCH
	rows = 8 ;
 #endif
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

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0 ) ;
	if ( newSelection >= 0 )
	{
		if ( timer )
		{
			newSelection += rows ;
		}
	}
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif

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
	saveEditColours( attr, colour ) ;
  putsTmrMode(TRIGHT-TRMARGIN,y+TVOFF,LUA_RIGHT, timer, 1 ) ;
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
#ifndef TOUCH
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
#endif
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
#ifndef TOUCH
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
#endif
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
#ifdef TOUCH
	return sub ;
#endif
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

//#ifdef TOUCH
//	if ( TouchUpdated )
//	{
//		if ( TouchControl.event == TEVT_DOWN )
//		{
				
//		}
//		else if ( TouchControl.event == TEVT_UP )
//		{
//			uint32_t lines ;
//#ifdef HAPTIC
//			if ( sub < 16 )
//#else
//			if ( sub < 14 )
//#endif
//			{
//				lines = 8 ;
//			}
//			else
//			{
//				lines = 3 ;
//			}
//			if ( ( TouchControl.x <= TRIGHT*TSCALE ) && (TouchControl.x >= TMID*TSCALE) && !s_editMode )
//			{
//				uint32_t vert = TouchControl.y ;
//				vert -= TTOP*TSCALE ;
//				vert /= TFH*TSCALE ;
//				if ( vert < lines )
//				{
//					if ( sub < 16 )
//					{
//						if ( sub >= 8 )
//						{
//							vert += 8 ;
//						}						
//					}
//					else
//					{
//						vert += 16 ;
//					}
//					TouchControl.itemSelected = vert+1 ;
//					TouchUpdated = 0 ;
//					sub = mstate2.m_posVert = vert ;
//				}
//			}
//		}
//	}
//#endif

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
#ifdef TOUCH
		mstate2.m_posVert = editTimer( sub, event ) ;
#else
		editTimer( sub, event ) ;
#endif
	}
	else
	{
#ifdef HAPTIC
		subN = 16 ;
#else
		subN = 14 ;
#endif
		
#ifdef TOUCH
		int32_t newSelection = checkTouchSelect( rows, 0 ) ;
		if ( newSelection >= 0 )
		{
			newSelection += rows - 3 ;
		}
		uint16_t newVert = processSelection( sub, newSelection ) ;
		sub = mstate2.m_posVert = newVert & 0x00FF ;
		checkTouchEnterEdit( newVert ) ;
		if ( newVert == rows - 3 )
		{
			s_editMode = 0 ;
		}
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

void menuVoiceOne(uint8_t event)
{
	touchMenuTitle( (char *)PSTR(STR_Voice_Alarm) ) ;
	static MState2 mstate2;
	uint32_t rows = 13 ;
	VoiceAlarmData *pvad ;
#ifdef TOUCH
	uint32_t newVpos ;
	uint32_t selected = 0 ;
#endif

#ifdef X20
	if ( s_currIdx >= NUM_VOICE_ALARMS )
#else
  if ( s_currIdx >= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
#endif
	{
		EditType = EE_GENERAL ;
#ifdef X20
		uint8_t z = s_currIdx - ( NUM_VOICE_ALARMS ) ;
		lcdDrawChar( 16*FW, 0, 'G' ) ;
		lcdDrawNumber( 18*FW-1, 0, z+1, 0 ) ;
#else
		uint8_t z = s_currIdx - ( NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ) ;
		PUTC( 16*FW, 0, 'G' ) ;
		PUTS_NUM( 18*FW-1, 0, z+1, 0 ) ;
#endif
		pvad = &g_eeGeneral.gvad[z] ;
	}
	else
	{
#ifdef X20
		pvad = &g_model.vad[s_currIdx] ;
		lcdDrawNumber( 17*FW, 0, s_currIdx+1, 0 ) ;
#else
		pvad = (s_currIdx >= NUM_VOICE_ALARMS) ? &g_model.vadx[s_currIdx - NUM_VOICE_ALARMS] : &g_model.vad[s_currIdx] ;
		PUTS_NUM( 17*FW, 0, s_currIdx+1, 0 ) ;
#endif
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
	selected = newVert & 0x0100 ;
	
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
			else if ( sub > t_pgOfs + TLINES - 1 )
			{
				mstate2.m_posVert = sub = t_pgOfs + TLINES - 1 ;
			}
		}
	}
#endif

	uint16_t colour = dimBackColour() ;
	
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
					putsChnRaw( (TRIGHT-TRMARGIN)*TSCALE, (y)*TSCALE+TVOFF, pvad->source, LUA_RIGHT ) ;
					restoreEditColours() ;
					if ( attr )
					{
#ifdef X20
						uint8_t x = pvad->source ;
						x = offsetCheckOutOfOrder( x, TelemMap, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS, NUM_SKYXCHNRAW+1, EE_MODEL ) ;
						pvad->source = x ;
#else
						uint8_t x = mapPots( pvad->source ) ;
						x = checkIncDec16( x,0,NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots-1+1,EditType);
						pvad->source = unmapPots( x ) ;
#endif
#ifdef TOUCH
						if ( ( event == EVT_KEY_BREAK(BTN_RE) ) || handleSelectIcon() || selected )
#else
						if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
#endif
						{
							// Long MENU pressed
#ifdef X20
							TextIndex = pvad->source ;
#else
							TextIndex = mapPots( pvad->source ) ;
#endif
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
					drawIdxText( y, XPSTR("\007-------v>val  v<val  |v|>val|v|<valv\140=val v=val  v & val|d|>valv%val=0d>=val "), pvad->func, attr|LUA_RIGHT ) ;	// v1>v2  v1<v2  
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
#ifndef TOUCH
						if ( attr & INVERS )
						{
							if ( s_editMode && BLINK_ON_PHASE )
							{
								attr = 0 ;
							}
						}
#endif
						saveEditColours( attr, colour ) ;
						putsTelemetryChannel( TRIGHT-TRMARGIN-FW, y+TVOFF, pvad->source-CHOUT_BASE-NUM_SKYCHNOUT-1, pvad->offset, 0, TELEM_NOTIME_UNIT | TELEM_UNIT | TELEM_CONSTANT ) ;
						restoreEditColours() ;
					}
					else
					{
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
   	  		DrawDrSwitches(TRIGHT-TRMARGIN, y, pvad->swtch, attr|LUA_RIGHT ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_MODELSWITCH( pvad->swtch, -MaxSwitchIndex, MaxSwitchIndex+1 ) ;
    			}
				break ;

				case 4 :	 // rate ;
					drawItem( (char *)XPSTR("Trigger"), y, attr ) ;
#ifndef TOUCH
					if ( attr & INVERS )
					{
						if ( s_editMode && BLINK_ON_PHASE )
						{
							attr = 0 ;
						}
					}
#endif
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
					drawIdxText( y, XPSTR("\007-------Haptic1Haptic2Haptic3"), pvad->haptic, attr|LUA_RIGHT ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->haptic, 3 ) ;
					}
				break ;

				case V1P1 :	 // delay
					drawItem( (char *)XPSTR("On Delay"), y, attr ) ;
  				drawNumber(TRIGHT-TRMARGIN,y, pvad->delay, attr|PREC1 ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->delay, 50 ) ;
					}
				break ;
				
				case V1P1+1 :	 // vsource:2
					drawItem( (char *)XPSTR("Play Source"), y, attr) ;
					drawIdxText( y, XPSTR("\006    NoBefore After"), pvad->vsource, attr|LUA_RIGHT ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->vsource, 2 ) ;
					}
				break ;

				case V1P1+2 :
					drawItem( (char *)XPSTR("On no Telemetry"), y, attr ) ;
					drawIdxText( y, XPSTR("\004PlayMute"), pvad->mute, attr|LUA_RIGHT ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->mute, 1 ) ;
					}
				break ;

				case V1P1+3 :	 // fnameType:3 ;
				{	
					drawItem( (char *)XPSTR("FileType"), y, attr ) ;
					drawIdxText( y, XPSTR("\007-------   NameGV/SC/# EffectSysName"),pvad->fnameType, attr|LUA_RIGHT ) ;
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
							if ( event==EVT_KEY_LONG(BTN_RE) || handleSelectIcon() || selected )
#else
							if ( event==EVT_KEY_LONG(BTN_RE) )
#endif
							{
								VoiceFileType = ( pvad->fnameType == 4 ) ? VOICE_FILE_TYPE_SYSTEM : VOICE_FILE_TYPE_USER ;
      				 	pushMenu( menuSelectVoiceFile ) ;
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
		checkTouchEnterEdit( newVert ) ;
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
    	  pushMenu(menuGlobalVoiceAlarm) ;
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
    	  pushMenu(menuVoiceOne) ;
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
	  if ( k < NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
		{
    	if ( mode )
			{
				drawNumberedItemWide( XPSTR("GVA"), y-TVOFF, attr, k+1 ) ;
//				PUTS_P( THOFF/2, y, XPSTR("GVA") ) ;
//				PUTC( 3+3*FW, y, '1' + k ) ;
				pvad = &g_eeGeneral.gvad[k] ;
			}
			else
			{
				drawNumberedItemWide( XPSTR("VA"), y-TVOFF, attr, k+1 ) ;
//				PUTS_P( THOFF/2, y, XPSTR("VA") ) ;
//  		  PUTS_NUM( (k<9) ? 3+FW*3-1 : 3+FW*4-2, y, k+1, 0 ) ;
				pvad = (k >= NUM_VOICE_ALARMS) ? &g_model.vadx[k - NUM_VOICE_ALARMS] : &g_model.vad[k] ;
			}
			if ( attr )
			{
	//			lcdDrawSolidFilledRectDMA( 0, (y-TVOFF)*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
				LcdBackground = ~LcdBackground ;
				LcdForeground = ~LcdForeground ;
			}
			putsChnRaw( (5*FW)*2, (y)*2, pvad->source, 0 ) ;
			putsDrSwitches( 9*FW, y, pvad->swtch, 0 ) ;
			displayVoiceRate( 15*FW, y, pvad->rate, 0 ) ;
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
					lcdHiresRect( (4*FW)*TSCALE, (y-TVOFF+1)*TSCALE, (TRIGHT-TRMARGIN-4*FW-2)*TSCALE, (TFH-1)*TSCALE, DimBackColour ) ;
//					lcd_rect( 1, y-TVOFF, 127, (TFH-1)*TSCALE ) ;
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
				drawItem( (char *)XPSTR(GvaString), y-TVOFF, attr ) ;
			}
  		else
			{
				drawItem( (char *)XPSTR("Flush Switch"), y-TVOFF, attr ) ;
				LcdFlags doedit = attr ;
#ifndef TOUCH
				if ( attr & INVERS )
				{
					if ( s_editMode && BLINK_ON_PHASE )
					{
						attr = 0 ;
					}
				}
#endif
				saveEditColours( attr, DimBackColour ) ;
				g_model.voiceFlushSwitch = edit_dr_switch( TRIGHT-TRMARGIN, y, g_model.voiceFlushSwitch, LUA_RIGHT, doedit ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				restoreEditColours() ;
			}
		}
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
//		lcd_hline( 0, y+TFH-TVOFF, TRIGHT ) ;
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
	LcdFlags attr ;
	uint32_t index ;
	index = s_currIdx ;
#ifdef X20
	X20SafetySwData *sd = &g_model.safetySw[index];
#else
	SKYSafetySwData *sd = &g_model.safetySw[index];
#endif

#ifdef X20
	if ( sd->mode )
#else
	if ( sd->opt.ss.mode == 3 )
#endif
	{
		rows += 1 ;
	}
	event = mstate2.check_columns(event, rows - 1 ) ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t subN ;

#ifdef TOUCH
	int32_t newSelection = checkTouchSelect( rows, 0, 1 ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
//	selected = newVert & 0x0100 ;
#endif

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
#ifdef X20
	if ( attr )
	{
		CHECK_INCDEC_H_MODELVAR( sd->mode, 0, 1 ) ;
	}
	drawIdxText( y, (char *)XPSTR("\006SafetySticky"), sd->mode, attr|LUA_RIGHT ) ;
#else
	uint32_t uvalue = sd->opt.ss.mode > 0 ;
	if ( attr )
	{
		CHECK_INCDEC_H_MODELVAR( uvalue, 0, 1 ) ;
	}
  drawIdxText( y, XPSTR("\006SafetySticky"), uvalue, attr|LUA_RIGHT ) ;
	sd->opt.ss.mode = uvalue ? 3 : 0 ;
#endif	
	y += TFH ;
	subN += 1 ;

  attr = 0 ;
  if(sub==subN)
	{
		attr = INVERS ;
	}
	drawItem( XPSTR("Switch"), y, (attr) ) ;
	LcdFlags doedit = attr ;
#ifndef TOUCH
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
#endif
	saveEditColours( attr, colour ) ;
#ifdef X20
	sd->swtch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, sd->swtch, LUA_RIGHT, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
#else
	sd->opt.ss.swtch = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, sd->opt.ss.swtch, LUA_RIGHT, doedit ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
#endif
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
	
	drawNumber( TRIGHT-TRMARGIN, y, value, attr|PREC1) ;
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
		putsChnRaw( (TRIGHT-TRMARGIN)*TSCALE, (y+TVOFF)*TSCALE, temp, LUA_RIGHT ) ;
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
        			drawChar( 14*FW, y, '.', attr ) ; //, (attr & INVERS) ? ~LcdForeground : LcdForeground ) ;
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

#ifdef X20
#define MID_OFFSET	20
#else
#define MID_OFFSET	15
#endif

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
	
	event = mstate2.check_columns( event, count ) ;
	int8_t  sub = mstate2.m_posVert ;

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
	}
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
#endif
	 
	uint8_t stkVal ;
	sub = mstate2.m_posVert ;
	int8_t subN ;
	if ( SingleExpoChan )
	{
		sub += 1 ;
	}
#ifndef X20
	if( sub )
	{
		StickScrollAllowed = 0 ;
	}
#endif
	lcd_hline( 0, TTOP, TMID-MID_OFFSET ) ;

	uint8_t l_expoChan = s_expoChan ;
	subN = 0 ;
  uint8_t attr = 0 ;
  uint8_t doedit = 0 ;
	if ( sub == subN )
	{
		lcdDrawSolidFilledRectDMA( 0, TTOP*TSCALE+2, (TMID-MID_OFFSET)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
		if ( SingleExpoChan == 0 )
		{
			s_expoChan = l_expoChan = checkIncDec( s_expoChan, 0, 3, 0 ) ;
			attr = INVERS ;
		}
	}		 
#ifndef TOUCH
	if ( attr & INVERS )
	{
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
	}
#endif
	saveEditColours( attr, DimBackColour ) ;
	putsChnRaw( THOFF*TSCALE, (TTOP+TVOFF)*TSCALE,l_expoChan+1,0) ;
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
 #ifdef X20
	X20ExpoData *eptr ;
	eptr = &g_model.expoData[s_expoChan] ;
 #else
	ExpoData *eptr ;
	eptr = &g_model.expoData[s_expoChan] ;
 #endif
extern int16_t TouchAdjustValue ;
#endif

	lcd_hline( 0, TTOP+TFH, TMID-MID_OFFSET ) ;

	if ( sub==subN )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+TFH)*TSCALE+2, (TMID-MID_OFFSET)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	}
	saveEditColours( ( sub==subN ), DimBackColour ) ;
	PUTS_P( THOFF,TTOP+TFH+TVOFF,PSTR(STR_2EXPO)+1);
	restoreEditColours() ;

	attr = (stkVal != DR_RIGHT) && (sub==subN) ;
	doedit = attr ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+2*TFH)*TSCALE+2, (TMID-20)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
#ifndef TOUCH
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
#endif
	} 
	saveEditColours( attr, DimBackColour ) ;
 #ifdef X20
	drawSmallGVAR( 3, TTOP+2*TFH+2 ) ;
	editExpoVals( event, doedit, 19*FW/2, TTOP+2*TFH+TVOFF, expoDrOn, DR_EXPO, DR_LEFT ) ;
#else
	drawSmallGVAR( 1, (TTOP+2*TFH)*TSCALE+2 ) ;
	editExpoVals( event, doedit, 17*FW/2, TTOP+2*TFH+TVOFF, expoDrOn, DR_EXPO, DR_LEFT ) ;
#endif
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
	doedit = attr ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+2*TFH)*TSCALE+2, ((TMID-10)/2)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
#ifndef TOUCH
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
#endif
	} 
	saveEditColours( attr, DimBackColour ) ;
 #ifdef X20
	drawSmallGVAR( (TMID-20)/2*TSCALE+3, TTOP+2*TFH+2 ) ;
	editExpoVals( event, attr, 18*FW, TTOP+2*TFH+TVOFF, expoDrOn, DR_EXPO, DR_RIGHT ) ;
	restoreEditColours() ;
	lcd_hline( 0, TTOP+2*TFH, TMID-20 ) ;
#else
	drawSmallGVAR( (TMID-20)/2*TSCALE+1, (TTOP+2*TFH)*TSCALE+2 ) ;
	editExpoVals( event, doedit, 16*FW, TTOP+2*TFH+TVOFF, expoDrOn, DR_EXPO, DR_RIGHT ) ;
	restoreEditColours() ;
	lcd_hline( 0, TTOP+2*TFH, TMID-MID_OFFSET ) ;
#endif
	subN += 1 ;
	attr = (stkVal != DR_RIGHT) && (sub==subN) ;
	doedit = attr ;
	lcd_hline( 0, TTOP+3*TFH, TMID-MID_OFFSET ) ;
	 
	if ( sub==subN )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+3*TFH)*TSCALE+2, (TMID-MID_OFFSET)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	}
	saveEditColours( ( sub==subN ), DimBackColour ) ;
	PUTS_P( THOFF,TTOP+3*TFH+TVOFF,PSTR(STR_2WEIGHT)+1);
	restoreEditColours() ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+4*TFH)*TSCALE+2, (TMID-20)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
#ifndef TOUCH
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
#endif
	} 
	saveEditColours( attr, DimBackColour ) ;
	drawSmallGVAR( 1, (TTOP+4*TFH)*TSCALE+2 ) ;
	editExpoVals( event, doedit, 17*FW/2, TTOP+4*TFH+TVOFF, expoDrOn, DR_WEIGHT, DR_LEFT ) ;
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
	doedit = attr ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+4*TFH)*TSCALE+2, (TMID-10)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
#ifndef TOUCH
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
#endif
	} 
	saveEditColours( attr, DimBackColour ) ;
	drawSmallGVAR( (TMID-20)/2*TSCALE+1, (TTOP+4*TFH)*TSCALE+2 ) ;
	editExpoVals( event, doedit, 16*FW, TTOP+4*TFH+TVOFF, expoDrOn, DR_WEIGHT, DR_RIGHT ) ;
	restoreEditColours() ;
	lcd_hline( 0, TTOP+4*TFH, TMID-MID_OFFSET ) ;
	subN += 1 ;
	PUTS_P( THOFF, TTOP+5*TFH+TVOFF,PSTR(STR_DR_SW1));
	lcd_hline( 0, TTOP+5*TFH, TMID-MID_OFFSET ) ;
	doedit = attr = sub==subN ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+5*TFH)*TSCALE+2, (TMID-10)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
#ifndef TOUCH
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
#endif
	} 
	saveEditColours( attr, DimBackColour ) ;
	editExpoVals( event, doedit, 10*FW, TTOP+5*TFH+TVOFF, DR_DRSW1, 0, 0 ) ;
	restoreEditColours() ;
	subN += 1 ;
	
	PUTS_P( THOFF, TTOP+6*TFH+TVOFF,PSTR(STR_DR_SW2));
	lcd_hline( 0, TTOP+6*TFH, TMID-MID_OFFSET ) ;
	doedit = attr = sub==subN ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+6*TFH)*TSCALE+2, (TMID-10)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
#ifndef TOUCH
		if ( s_editMode && BLINK_ON_PHASE )
		{
			attr = 0 ;
		}
#endif
	} 
	saveEditColours( attr, DimBackColour ) ;
	editExpoVals( event, doedit, 10*FW, TTOP+6*TFH+TVOFF, DR_DRSW2, 0, 0 ) ;
	restoreEditColours() ;
	
	lcd_hline( 0, TTOP+7*TFH, TMID-MID_OFFSET ) ;

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
void menuTrainerProtocol(uint8_t event) ;
//extern uint8_t EditingModule ;
//void editOneProtocol( uint8_t event ) ;
void displayProtocol( uint16_t x, uint16_t y, uint8_t value, uint8_t attr ) ;






void menuMixOne(uint8_t event)
{
	uint8_t rows = 16 ;
	SKYMixData *md2 = mixAddress( s_curItemIdx ) ;
  
#ifdef TOUCH	
	TlExitIcon = 1 ;
	uint32_t selected = 0 ;
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
	selected = newVert & 0x0100 ;
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
#ifdef X20
				putsChnOpRaw( TRIGHT-TRMARGIN, y+TVOFF, value, md2->disableExpoDr, MIX_SOURCE|LUA_RIGHT ) ;
#else
				putsChnOpRaw( (TRIGHT-TRMARGIN)*TSCALE, (y+TVOFF)*TSCALE, value, md2->switchSource, md2->disableExpoDr, LUA_RIGHT ) ;
#endif
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
//					if ( md2->srcRaw > 4 )
//					{
#endif
							
#ifdef TOUCH
					if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) || selected )
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
//					}	
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


#define DATE_OFF_0		(0)
#define DATE_COUNT_ITEMS	8

void dispMonth( coord_t x, coord_t y, uint32_t month, LcdFlags attr) ;
void disp_datetime( coord_t y ) ;

t_time EntryTime ;

void menuProcDate(uint8_t event)
{
	touchMenuTitle( (char *)PSTR(STR_DateTime) ) ;
	static MState2 mstate2;
	uint32_t rows = DATE_COUNT_ITEMS ;
#ifdef TOUCH	
	uint32_t selected = 0 ;
#endif
	 
	mstate2.check_columns(event, DATE_COUNT_ITEMS-1) ;

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
	}
	uint32_t newVert = processSelection( sub , newSelection ) ;
	sub = mstate2.m_posVert = newVert & 0x00FF ;
	checkTouchEnterEdit( newVert ) ;
	selected = newVert & 0x0100 ;
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

	for (uint32_t subN = 0 ; subN<rows ; subN += 1)
	{
		uint16_t y = TTOP+subN*TFH+TFH ;
	  uint16_t attr = ((sub==subN) ? INVERS : 0) ;
		switch ( subN )
		{
			case 0 :
				drawItem( (char *)PSTR(STR_SEC), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.second, attr) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.second = checkIncDec( EntryTime.second, 0, 59, 0 ) ;
			break ;
			case 1 :
				drawItem( (char *)XPSTR("Minute"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.minute, attr) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.minute = checkIncDec( EntryTime.minute, 0, 59, 0 ) ;
			break ;
			case 2 :
				drawItem( (char *)XPSTR("Hour"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.hour, attr) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.hour = checkIncDec( EntryTime.hour, 0, 23, 0 ) ;
			break ;
			case 3 :
				drawItem( (char *)PSTR(STR_DATE), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.date, attr) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.date = checkIncDec( EntryTime.date, 1, 31, 0 ) ;
			break ;
			case 4 :
				drawItem( (char *)PSTR(STR_MONTH), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.month, attr) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.month = checkIncDec( EntryTime.month, 1, 12, 0 ) ;
			break ;
			case 5 :
				drawItem( (char *)PSTR(STR_YEAR), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, EntryTime.year, attr) ; //, attr ? ~colour : colour ) ;
			  if(attr)  EntryTime.year = checkIncDec16( EntryTime.year, 0, 2999, 0 ) ;
			break ;
			case 6 :
			{	
				int8_t previous = g_eeGeneral.rtcCal ;
				drawItem( (char *)PSTR(STR_CAL), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, g_eeGeneral.rtcCal, attr) ; //, attr ? ~colour : colour ) ;
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
					if ( checkForMenuEncoderLong( event ) || handleSelectIcon() || selected )
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
		   	pushMenu( menuSelectVoiceFile ) ;
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
		LcdFlags doedit = attr ;
#ifndef TOUCH
		if ( attr & INVERS )
		{
			if ( s_editMode && BLINK_ON_PHASE )
			{
				attr = 0 ;
			}
		}
#endif
		saveEditColours( attr, colour ) ;
		g_model.mlightSw = edit_dr_switch( TRIGHT-TRMARGIN, y+TVOFF, g_model.mlightSw, LUA_RIGHT, doedit ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
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
	TITLE((char *)PSTR(STR_Version)) ;
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




uint16_t scalerDecimal( coord_t y, uint16_t val, LcdFlags attr )
{
  PUTS_NUM( TRIGHT-TRMARGIN, y+TVOFF, val+1, 0 ) ;
	if (attr) val = checkIncDec16( val, 0, 2047, EE_MODEL);
	return val ;
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
	uint32_t selected = 0 ;
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
	selected = newVert & 0x0100 ;

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
	if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() || selected )
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



int8_t getAndSwitch( SKYCSwData &cs ) ;
void displayLogicalSwitch( uint16_t x, uint16_t y, uint32_t index ) ;
extern uint8_t saveHpos ;
extern const uint8_t SwitchFunctionMap[] ;
extern uint8_t InverseBlink ;
void editOneSwitchItem( uint8_t event, uint32_t item, uint32_t index ) ;



extern char LastItem[] ;

extern uint8_t TelemMap[] ;

extern const uint8_t TelemSize  ;
extern union t_sharedMemory SharedMemory ;

#if LOG_LOOKUP_SIZE > ALPHA_LOG_SIZE
//ERROR "ALPHA_LOG_SIZE too small"
#endif


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
#ifdef TOUCH
extern uint8_t TlExitIcon ;
	TlExitIcon = 1 ;
	uint32_t selected = 0 ;
#endif	
	static MState2 mstate2 ;
	uint32_t menulong = 0 ;

  switch(event)
  {
	  case EVT_ENTRY:
      s_moveMode = false ;
			if ( s_inputMaxSel < NUM_INPUTS )
			{
				s_inputMaxSel = NUM_INPUTS ;
			}
  	break;
    
#ifndef X20
		case EVT_KEY_FIRST(KEY_MENU):
#endif
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

#ifdef TOUCH
	if ( handleSelectIcon() )
	{
		s_currIdx = sub ;
		menulong = 1 ;
	}
#endif

	uint32_t t_pgOfs ;
//#ifdef COLOUR_DISPLAY
	t_pgOfs = evalHresOffset( sub ) ;
//#else
//	t_pgOfs = evalOffset( sub ) ;
//#endif  
    
	if ( PopupData.PopupActive )
	{
		Tevent = 0 ;
	}
#ifdef TOUCH
	else
	{
		uint32_t newVpos ;
		int32_t newSelection = checkTouchSelect( TLINES, 0, 2 ) ;
		uint16_t newVert = processSelection( sub , newSelection ) ;
		sub = mstate2.m_posVert = newVert & 0x00FF ;
		checkTouchEnterEdit( newVert ) ;
		selected = newVert & 0x0100 ;
		
		newVpos = scrollBar( TSCROLLLEFT+15, TSCROLLTOP, TSCROLLWIDTH-15, TSCROLLBOTTOM, s_inputMaxSel-(TLINES-1), t_pgOfs ) ;
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
	if ( handleSelectIcon() || selected )
	{
		s_currIdx = sub ;
		menulong = 1 ;
	}
#endif

	if ( s_moveMode )
	{
		int32_t dir ;
		
		if ( ( dir = (event == EVT_KEY_FIRST(KEY_DOWN) ) ) || event == EVT_KEY_FIRST(KEY_UP) )
		{
//#ifdef COLOUR_DISPLAY
			uint8_t ch ;
			struct te_InputsData *pinput = &g_model.inputs[s_curItemIdx] ;
			ch = pinput->chn ;
//#endif
			moveInput( s_curItemIdx, dir ) ; //true=inc=down false=dec=up - Issue 49
			if ( ch != pinput->chn )
			{
				// moved channels, correct sub etc.
				if ( dir )
				{
					pinput = &g_model.inputs[s_curItemIdx-1] ;
					if ( ch == pinput->chn )
					{
						sub -= 1 ;
					}
				}
				else
				{
					ch = pinput->chn ;
					pinput = &g_model.inputs[s_curItemIdx-1] ;
					if ( ch == pinput->chn )
					{
						sub += 1 ;
					}
				}
				mstate2.m_posVert = sub ;
			}
		}
	}

  uint32_t input_index = 0 ;
  uint32_t current = 0 ;
	s_expoChan = 255 ;

//#ifdef COLOUR_DISPLAY
	lcd_hline( 0, TTOP, TMID ) ;
//#endif

  for ( uint32_t chan = 1 ; chan <= NUM_INPUTS ; chan += 1 )
	{
		struct te_InputsData *pinput = &g_model.inputs[input_index] ;
		LcdFlags attr = 0 ;

		coord_t y ;
//#ifdef COLOUR_DISPLAY
		y = (current-t_pgOfs)*TFH+TTOP ;
//#else
//		y = (current-t_pgOfs+1)*FHPY ;
//#endif

//#ifdef COLOUR_DISPLAY
    if ( t_pgOfs <= current && current-t_pgOfs < TLINES)
//#else
//    if ( t_pgOfs <= current && current-t_pgOfs < SCREEN_LINES-1)
//#endif
		{
    	attr = 0 ;
			if (sub == current)
			{
				attr = INVERS ;
			}
			if ( y+TFH != TTOP)
			{
				lcd_hline( 0, y+TFH, TMID ) ;
			}
			lcdDrawSolidFilledRectDMA( 0, ((current-t_pgOfs)*TFH+TTOP)*TSCALE+2, (TMID)*TSCALE, TFH*TSCALE-2, attr ? ~DimBackColour : DimBackColour ) ;
			saveEditColours( attr, DimBackColour ) ;
			displayInputName( 1, (current-t_pgOfs)*TFH+TTOP+TVOFF, chan, 0 ) ;
			restoreEditColours() ;
    }

		uint8_t firstInput = input_index ;

		if (input_index < NUM_INPUT_LINES && /* pmd->srcRaw && */ pinput->chn == chan)
		{
    	do
			{
				if (t_pgOfs <= current )
				{
					if ( current-t_pgOfs < TLINES )
					{
    				attr = 0 ;
						
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

						if(firstInput != input_index) //show prefix only if not first mix
						{
							y += TFH ;
				    	attr = (sub == current) ? INVERS : 0 ;
							lcdDrawSolidFilledRectDMA( 0, ((current-t_pgOfs)*TFH+TTOP)*TSCALE+2, (TMID)*TSCALE, TFH*TSCALE-2, attr ? ~DimBackColour : DimBackColour ) ;
							lcd_hline( 0, y+TFH, TMID ) ;
						}

						if (sub == current)
						{
							s_expoChan = pinput->chn ;
						}
#ifdef USE_VARS
						if ( g_model.vars )
						{
							if ( pinput->varForWeight )
							{
								saveEditColours( attr, DimBackColour ) ;
								displayVarName( (12)*FW, y+TVOFF, pinput->weight, LUA_RIGHT ) ;
								restoreEditColours() ;
							}
							else
							{
								saveEditColours( attr, DimBackColour ) ;
								PUTS_NUM( 12*FW, y+TVOFF, pinput->weight, 0 ) ;
								restoreEditColours() ;
							}
						}
						else
#endif
						{
							int16_t temp ;
							temp = pinput->weight ;
							if (temp > 100)
							{
								{
									temp -= 110 ;
								}
								if ( temp < 0 )
								{
									temp = -temp - 1 ;
  								PUTC_ATT(8*FW-4*FW, y+TVOFF,'-',attr) ;
								}
								saveEditColours( attr, DimBackColour ) ;
								dispGvar( 12*FW-3*FW, y+TVOFF, temp+1, 0 ) ;
								restoreEditColours() ;
							}
							else
							{
								saveEditColours( attr, DimBackColour ) ;
								PUTS_NUM( 12*FW, y+TVOFF, temp, 0 ) ;
								restoreEditColours() ;
							}
						}
						if ( pinput->srcRaw >= 128 )
						{
							saveEditColours( attr, DimBackColour ) ;
							putsChnOpRaw( (13*FW-FW/2)*2 ,(y+TVOFF)*2, MIX_3POS, pinput->srcRaw-128, 0, 0 ) ;
							restoreEditColours() ;
						}
						else
						{
							saveEditColours( attr, DimBackColour ) ;
							putsChnOpRaw( (13*FW-FW/2)*2, (y+TVOFF)*2, pinput->srcRaw, 0, 0, 0 ) ;
							restoreEditColours() ;
						}
		        
						saveEditColours( attr, DimBackColour ) ;
						putsDrSwitches(17*FW-FW/2, y+TVOFF, pinput->swtch, 0) ;
						restoreEditColours() ;
						if ( s_moveMode )
						{
							if ( s_moveItemIdx == input_index )
							{
								lcdHiresRect( (4*FW)*2, (y+1)*2, (TMID-4*FW-2)*2, (TFH-1)*2, DimBackColour ) ;
								lcdHiresRect( (4*FW)*2+1, (y+1)*2+1, (TMID-4*FW-2)*2-2, (TFH-1)*2-2, LCD_BLACK ) ; //DimBackColour ) ;
								s_curItemIdx = input_index ;
								sub = mstate2.m_posVert = current ;
							}
						}
		 
					}
				}
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
#ifdef TOUCH
		uint32_t x = XD-3/*+2*FW*/ ;
#else
		uint32_t x = XD+2*FW ;
#endif
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

