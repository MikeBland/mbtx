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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#ifdef X20



#else

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef PCBSKY
#include "AT91SAM3S4.h"
#include "core_cm3.h"
#endif
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

#ifdef PCBX9D
#include "X9D/eeprom_rlc.h"
#include "timers.h"
#include "analog.h"
#endif
#include "gvars.h"

#if defined(PCBLEM1)
#include "timers.h"
void eeLoadModel(uint8_t id) ;
#include <stm32f10x.h>
#endif

#if defined(PCBX12D) || defined(PCBX10)
#include "timers.h"
#include "analog.h"

#include "X12D/hal.h"
#endif

#ifdef BLUETOOTH
#include "bluetooth.h"
#endif

#ifdef PCB9XT
#include "timers.h"
#include "analog.h"
#include "mega64.h"
#include "isp.h"
#endif

#ifdef TOUCH
#include "X12D/tp_gt911.h"
#endif

#include "vars.h"

#endif // X20

// Externals that may need placing somewhere else
char *getIndexedText( char *p, const char * s, uint8_t idx, uint32_t maxLen ) ;
extern uint8_t CurrentPhase ;
#ifdef TOUCH
extern uint8_t TlExitIcon ;
#endif

extern const char s_charTab[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-.";
#define NUMCHARS (sizeof(s_charTab)-1)


void touchMenuTitle( char *s )
{
	TITLE(s) ;
#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	lcd_hline( 0, TTOP, TRIGHT ) ;
}

#ifdef COLOUR_DISPLAY
void drawItem( char *s, uint16_t y, uint16_t selected )
{
//	PUTS_P( THOFF, y+TVOFF, s ) ;
	lcdDrawText( THOFF*TSCALE, y*TSCALE+TVOFF, s ) ;
	lcd_hline( 0, y+TFH, TRIGHT ) ;
	lcdDrawSolidFilledRectDMA( TMID*TSCALE, y*TSCALE+2, (TRIGHT-TMID)*TSCALE, TFH*TSCALE-2, selected ? ~DimBackColour : DimBackColour ) ;
}

void drawNumberedItem( char *s, uint16_t y, uint16_t selected, int32_t number )
{
	char text[20] ;
	char *pt ;
	pt = numberToText( number, 0, 2, text, 20 ) ;
	cpystr( cpystr( (uint8_t *)text, (uint8_t *)s ), (uint8_t *)pt ) ;
	lcdDrawText( THOFF*TSCALE, y*TSCALE+TVOFF, text ) ;
	lcd_hline( 0, y+TFH, TRIGHT ) ;
	lcdDrawSolidFilledRectDMA( TMID*TSCALE, y*TSCALE+2, (TRIGHT-TMID)*TSCALE, TFH*TSCALE-2, selected ? ~DimBackColour : DimBackColour ) ;
}

void drawNumberedItemWide( char *s, uint16_t y, uint16_t selected, int32_t number )
{
	char text[20] ;
	char *pt ;
	pt = numberToText( number, 0, 2, text, 20 ) ;
	cpystr( cpystr( (uint8_t *)text, (uint8_t *)s ), (uint8_t *)pt ) ;
	if ( selected )
	{
		lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, (TRIGHT)*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
	}
	saveEditColours( selected, ~LcdForeground ) ;
	lcdDrawText( THOFF*TSCALE, y*TSCALE+TVOFF, text ) ;
	restoreEditColours() ;
	lcd_hline( 0, y+TFH, TRIGHT ) ;
}

void drawNumber( uint16_t x, uint16_t y, int32_t val, uint16_t mode ) //, uint16_t colour )
{
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
	lcdDrawNumber( x*TSCALE, y*TSCALE+TVOFF, val, mode & ~INVERS, 5, fcolour) ; //, colour ) ;
}		  

void drawText( uint16_t x, uint16_t y, char *s, uint16_t mode )
{
	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
#ifdef X20
	lcdDrawText( x*TSCALE, y*TSCALE+TVOFF, s, mode & ~INVERS, fcolour ) ;
#else
	lcdDrawSizedText( x*TSCALE, y*TSCALE+TVOFF, s, 255, mode & ~INVERS, fcolour ) ;
#endif
//	lcd_putsAttColour( x, y+TVOFF, s, 0, fcolour ) ;
}		  

void drawChar( uint16_t x, uint16_t y, uint8_t c, uint16_t mode )
{
	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
  lcdDrawChar( x*TSCALE, y*TSCALE+TVOFF, c, 0, fcolour ) ;
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
#ifndef TOUCH
		if ( ! ( s_editMode && BLINK_ON_PHASE ) )
#endif
		{
			fcolour = ~LcdForeground ;
		}
	}
	lcdDrawSizedText( (TRIGHT-TRMARGIN)*TSCALE, y*TSCALE+TVOFF, s+length*index, length, LUA_RIGHT, fcolour ) ;
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
#ifndef TOUCH
		if ( ! ( s_editMode && BLINK_ON_PHASE ) )
#endif
		{
			fcolour = ~LcdForeground ;
		}
	}
	LcdForeground = fcolour ;
	putsDrSwitches( x, y, idx1, att&~INVERS ) ;
	LcdForeground = oldFcolour ;
}

//static uint16_t oldBcolour ;
static uint16_t oldFcolour ;
void saveEditColours( uint32_t attr, uint16_t colour )
{
//	oldBcolour = LcdBackground ;
	oldFcolour = LcdForeground ;
//	LcdBackground = attr ? ~colour : colour ;
	if ( attr & INVERS )
	{
		LcdForeground = ~LcdForeground ;
	}
}

void restoreEditColours()
{
//	LcdBackground = oldBcolour ;
	LcdForeground = oldFcolour ;
}

#endif

#ifdef TOUCH

uint32_t checkTouchposition( uint16_t x, uint16_t y, uint16_t x2, uint16_t y2 )
{
	if ( TouchUpdated )
	{
		if ( TouchControl.event == TEVT_DOWN )
		{
			
		}
		else if ( TouchControl.event == TEVT_UP )
		{
			if ( ( TouchControl.x <= x2 ) && (TouchControl.x >= x) )
	    {
				if ( ( TouchControl.y <= y2 ) && (TouchControl.y >= y) )
				{
					TouchUpdated = 0 ;
					return 1 ;
				}
			}
		}
	}
	return 0 ;
}

int32_t checkTouchSelect( uint32_t rows, uint32_t pgOfs, uint32_t flag )
{
	int32_t result = -1 ;
#ifdef X20
	uint32_t left = flag ? 80 : TMID*TSCALE ;
#else
	uint32_t left = flag ? 50 : TMID*TSCALE ;
#endif
	uint32_t right = (flag == 2) ? TRIGHT*TSCALE/2 : TRIGHT*TSCALE ;
	if ( TouchUpdated )
	{
		if ( TouchControl.event == TEVT_DOWN )
		{
			if ( ( TouchControl.x <= right ) && (TouchControl.x >= left) && !s_editMode )
			{
				uint32_t vert = TouchControl.y ;
				vert -= TTOP*TSCALE ;
				vert /= TFH*TSCALE ;
				if ( vert >= TLINES )
				{
					vert = TLINES - 1 ;
				}
				vert += pgOfs ;
				if ( vert < rows )
				{
					TouchControl.itemSelected = vert+1 ;
					TouchUpdated = 0 ;
					result = vert ;
				}
				else
				{
					result = -3 ;	// Down outside selection areas
				}
			}
		}
		else if ( TouchControl.event == TEVT_UP )
		{
			if ( ( TouchControl.x <= right ) && (TouchControl.x >= left) && !s_editMode )
			{
				result = -2 ;
				TouchUpdated = 0 ;
			}
		}
	}
	return result ;
}

uint8_t LastTselection ;
uint8_t LastTaction ;

uint16_t processSelection( uint8_t vert, int32_t newSelection )
{
	uint16_t result = vert ;
	if ( newSelection >= 0 )
	{
		if ( vert != newSelection )
		{
			LastTselection = newSelection ;
			LastTaction = 0 ;
		}
		result = newSelection ;
	}
	else if (newSelection == -2)
	{
		if ( vert == LastTselection )
		{
			if ( LastTaction == 2 )
			{
				result |= 0x100 ;
			}
		}
		LastTaction = 2 ;
	}
	else if (newSelection == -3)
	{
		LastTaction = 3 ;		
	}
	return result ;
}

void checkTouchEnterEdit( uint16_t value )
{
	if ( Tevent == 0 && (value & 0x0100) )
	{
		Tevent = EVT_KEY_BREAK(BTN_RE) ;
		s_editMode = 1 ;
	}
}

int32_t checkTouchArea( uint32_t x, uint32_t y, uint32_t w, uint32_t h )
{
	x *= TSCALE ;
	y *= TSCALE ;
	w *= TSCALE ;
	h *= TSCALE ;

	if ( TouchUpdated )
	{
		if ( TouchControl.event == TEVT_DOWN )
		{
		}
		else if ( TouchControl.event == TEVT_UP )
		{
			if ( ( TouchControl.x >= x ) && (TouchControl.x <= x+w) )
			{
				if ( ( TouchControl.y >= y ) && (TouchControl.y <= y+h ) )
				{
	 				TouchUpdated = 0 ;
					return 1 ;
				}
			}
			return 0 ;
		}
	}
	return -1 ;
}


#ifdef X20
#define SEL_EDIT_ICON_Y		190
#else
#define SEL_EDIT_ICON_Y		110
#endif

extern const uint8_t IconHedit[] ;
extern const uint8_t IconHselect[] ;

uint32_t handleSelectIcon()
{
	uint32_t retValue = 0 ;
	lcdDrawIcon( LCD_W-TICON_SIZE-2, SEL_EDIT_ICON_Y, IconHselect, 0 ) ;
	if ( TouchUpdated )
	{
		if ( TouchControl.event == TEVT_UP )
		{
			if ( TouchControl.x > LCD_W-TICON_SIZE )
			{
				if ( ( TouchControl.y >= SEL_EDIT_ICON_Y ) && ( TouchControl.y <= (SEL_EDIT_ICON_Y+TICON_SIZE) ) )
				{
					retValue = 1 ;
					TouchUpdated = 0 ;
				}
			}
		}
	}
	return retValue ;
}

uint32_t handleEditIcon()
{
	uint32_t retValue = 0 ;
	lcdDrawIcon( LCD_W-TICON_SIZE-2, SEL_EDIT_ICON_Y, IconHedit, 0 ) ;
	if ( TouchUpdated )
	{
		if ( TouchControl.event == TEVT_UP )
		{
			if ( TouchControl.x > LCD_W-TICON_SIZE )
			{
				if ( ( TouchControl.y >= SEL_EDIT_ICON_Y ) && ( TouchControl.y <= (SEL_EDIT_ICON_Y+TICON_SIZE) ) )
				{
					retValue = 1 ;
					TouchUpdated = 0 ;
				}
			}
		}
	}
	return retValue ;
}

int16_t ScrollBarStarty ;
uint16_t ScrollBarStartv ;
uint32_t scrollBar( uint16_t x, uint16_t y, uint16_t w, uint16_t bottom, uint32_t positions, uint32_t vpos )
{
	uint32_t barh ;
	uint32_t barpos ;
	uint32_t retValue = vpos ;
	uint32_t range ;
	uint16_t h ;
	h = bottom - y ;

	lcdDrawSolidFilledRectDMA( x, y, w, h, LCD_LIGHT_GREY ) ;

	barh = h * (SCREEN_LINES-1) / (positions + (SCREEN_LINES-1)) ;
	barpos = (h-barh) * vpos / (positions-1) ;
	range = h - barh ;

#ifdef X20
	lcdDrawSolidFilledRect( x+3, y+barpos+3, w-10, barh-6, LCD_GREY ) ;
	lcdDrawSolidFilledRect( x, y+barpos, w-7, 3, LCD_WHITE ) ;
	lcdDrawSolidFilledRect( x+3, y+barpos+barh-4, w-10, 3, LCD_BLACK ) ;
	lcdDrawSolidFilledRect( x, y+barpos+3, 3, barh-3, LCD_WHITE ) ;
	lcdDrawSolidFilledRect( x+w-10, y+barpos, 3, barh-1, LCD_BLACK ) ;
#else
	lcdDrawSolidFilledRectDMA( x+2, y+barpos+2, w-7, barh-4, LCD_GREY ) ;
	lcdDrawSolidFilledRectDMA( x, y+barpos, w-5, 2, LCD_WHITE ) ;
	lcdDrawSolidFilledRectDMA( x+2, y+barpos+barh-2, w-7, 2, LCD_BLACK ) ;
	lcdDrawSolidFilledRectDMA( x, y+barpos+2, 2, barh-2, LCD_WHITE ) ;
	lcdDrawSolidFilledRectDMA( x+w-5, y+barpos, 2, barh, LCD_BLACK ) ;
#endif

	if ( ( TouchUpdated ) && ( TouchControl.event == TEVT_DOWN ) )
	{
		if ( ( TouchControl.x > x ) && ( TouchControl.x < x + w ) )
		{
			if ( ScrollBarStarty == -1 )
			{
				ScrollBarStarty = TouchControl.y ;
				ScrollBarStartv = vpos ;
			}
			int32_t moved = TouchControl.y ;
			moved -= ScrollBarStarty ;
			moved *= positions ;
			moved /= (int32_t)range ;
			moved += ScrollBarStartv ;
			if ( moved < 0 )
			{
				moved = 0 ;
			}
			else if ( moved >= (int32_t)positions )
			{
				moved = positions - 1 ;
			}
			retValue = moved ;
			TouchUpdated = 0 ;
		}
		
	}
	else
	{
		if ( TouchControl.event == TEVT_UP )
		{
			ScrollBarStarty = -1 ;
		}
	}
	return retValue ;
}

#endif


#ifdef COLOUR_DISPLAY
uint8_t evalHresOffset(int8_t sub)
{
	uint8_t max = (TLINES - 1) ;
  uint32_t t_pgOfs = s_pgOfs ;
	int32_t x = sub-t_pgOfs ;
	if(sub<1) t_pgOfs = 0 ;
  else if(x>max) t_pgOfs = sub-max ;
  else if(x<max-(TLINES - 1)) t_pgOfs = sub-max+(TLINES - 1) ;
	return (s_pgOfs = t_pgOfs) ;
}
#endif


uint8_t char2idx(char c)
{
	uint32_t ret ;
	for( ret = 0 ; ; ret += 1 )
	{
		char cc= s_charTab[ret] ;
		if(cc==0) return 0 ;
		if(cc==c) return ret ;
	}
}

char idx2char(uint8_t idx)
{
	if(idx < NUMCHARS)
	{
		return s_charTab[idx] ;
	}
	return ' ' ;
}

void validateText( uint8_t *text, uint32_t length )
{
	for( uint32_t i=0 ; i<length ; i += 1 ) // makes sure text is valid
  {
		if ( text[i] < ' ' )
		{
			text[i] = ' ' ;
		}
//    uint8_t idx = char2idx(text[i]);
//    text[i] = idx2char(idx);
  }
}

void validateName( uint8_t *text, uint32_t length )
{
	for(uint32_t i=0; i<length ; i += 1) // makes sure name is valid
	{
		uint8_t idx = char2idx( text[i] ) ;
		text[i] = idx2char( idx ) ;
	}
}

uint8_t locateMappedItem( uint8_t value, uint8_t *options, uint32_t count )
{
	uint32_t c ;
	for ( c = 0 ; c < count ; c += 1 )
	{
		if ( options[c] == value )
		{
			value = c ;
			break ;
		}
	}
	if ( c >= count )
	{
		return 0 ;
	}
	return value ;
}

// Check a mapped value
uint8_t checkOutOfOrder( uint8_t value, uint8_t *options, uint32_t count )
{
	uint8_t index = locateMappedItem( value, options, count ) ;
	CHECK_INCDEC_H_MODELVAR_0( index, count-1 ) ;
	index = options[index] ;
	return index ;
}

int16_t m_to_ft( int16_t metres )
{
	int16_t result ;

  // m to ft *105/32
	result = metres * 3 ;
	metres >>= 2 ;
	result += metres ;
	metres >>= 2 ;
  return result + (metres >> 1 );
}

int16_t c_to_f( int16_t degrees )
{
  degrees += 18 ;
  degrees *= 115 ;
  degrees >>= 6 ;
  return degrees ;
}

void voiceMinutes( int16_t value )
{
	voice_numeric( value, 0, (abs(value) == 1) ? SV_MINUTE : SV_MINUTES ) ;
}

uint32_t checkForExitEncoderLong( uint8_t event )
{
//	return ( event == EVT_KEY_FIRST(KEY_EXIT) ) || ( event == EVT_KEY_LONG(BTN_RE) ) ;

	if ( event == EVT_KEY_FIRST(KEY_EXIT) )
	{
		return 1 ;
	}
	
	if ( g_eeGeneral.disableBtnLong == 0 )
	{
		if ( event == EVT_KEY_LONG(BTN_RE) )
		{
			return 1 ;
		}
	}   
	return 0 ;
}


uint32_t checkForMenuEncoderLong( uint8_t event )
{
	MaskRotaryLong = 1 ;
	uint32_t result ;
#ifdef X20
	result = event==EVT_KEY_LONG(BTN_RE) ;
#else
	result = ( event==EVT_KEY_LONG(KEY_MENU)) || ( event==EVT_KEY_LONG(BTN_RE) ) ;
#endif
	if ( result )
	{
  	s_editMode = 0 ;
    killEvents(event);
	}
	return result ;
}

uint32_t checkForMenuEncoderBreak( uint8_t event )
{
#ifdef X20
	return ( event==EVT_KEY_BREAK(BTN_RE) ) ;
#else
	return ( event==EVT_KEY_BREAK(KEY_MENU)) || ( event==EVT_KEY_BREAK(BTN_RE) ) ;
#endif
}

void putsDrSwitches(coord_t x, coord_t y, int8_t idx1, LcdFlags att)
{
	char text[12] ;
	char *p = text ;

	if ( idx1 == 0 )
	{
		p = XPSTR("---") ;
//    PUTS_ATT(x+FWX,y,XPSTR("---"),att) ;
//		return ;
	}
	else if ( idx1 == MAX_SKYDRSWITCH )
	{
		p = (char *)PSTR(STR_ON) ;
//    PUTS_ATT(x+FWX,y,PSTR(STR_ON),att) ;
//		return ;
	}
	else if ( idx1 == -MAX_SKYDRSWITCH )
	{
		p = (char *)PSTR(STR_OFF) ;
//    PUTS_ATT(x+FWX,y,PSTR(STR_OFF),att) ;
//		return ;
	}
	else if ( idx1 == MAX_SKYDRSWITCH + 1 )
	{
		p = XPSTR("Fmd") ;
//    PUTS_ATT(x+FWX,y,XPSTR("Fmd"),att) ;
//		return ;
	}
	else
	{
		if ( idx1 < 0 )
		{
			*p++ = '!' ;
		}

		int8_t z ;
		z = idx1 ;
		if ( z < 0 )
		{
			z = -idx1 ;			
		}
		if ( ( z <= HSW_Ttrmup ) && ( z >= HSW_Etrmdn ) )
		{
			z -= HSW_Etrmdn ;
			getIndexedText( p, XPSTR("\003EtuEtdAtuAtdRtuRtdTtuTtd"), z, 8 ) ;
		}
		else if ( ( z <= HSW_FM7 ) && ( z >= HSW_FM0 ) )
		{
			z -= HSW_FM0 ;
			getIndexedText( p, XPSTR("\003FM0FM1FM2FM3FM4FM5FM6FM7"), z, 8 ) ;
		}
		else
		{
			z -= 1 ;
//	#if defined(PCBSKY) || defined(PCB9XT)
			if ( z > MAX_SKYDRSWITCH )
			{
				z -= HSW_OFFSET - 1 ;
			}
			getIndexedText( p, PSTR(SWITCHES_STR), z, 8 ) ;
//	#endif

//	#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
//			if ( z > MAX_SKYDRSWITCH )
//			{
//				z -= HSW_OFFSET - 1 ;
//			}
//			getIndexedText( p, PSTR(SWITCHES_STR), z, 8 ) ;
//	#endif
		}
		p = text ;
	}
#ifdef X20
  lcdDrawText( x, y, p, att ) ;
#else
  PUTS_ATT( x, y, p, att ) ;
#endif
	return ;
}

uint8_t onoffItem( uint8_t value, coord_t y, uint8_t condition )
{
	return checkIndexed( y, 0, value, condition ) ;
}

uint8_t offonItem( uint8_t value, coord_t y, uint8_t condition )
{
	return 1-onoffItem( 1-value, y, condition ) ;
}

uint8_t onoffMenuItem( uint8_t value, coord_t y, const char *s, uint8_t condition )
{
#ifdef X20	
	lcdDrawTextLeft(y, s);
#else
	PUTS_ATT_LEFT(y, s) ;
#endif
	return onoffItem( value, y, condition ) ;
}

uint8_t offonMenuItem( uint8_t value, coord_t y, const char *s, uint8_t condition )
{
	return 1-onoffMenuItem( 1-value, y, s, condition ) ;
}

uint8_t checkIndexed( coord_t y, const char *s, uint8_t value, uint8_t edit )
{
	coord_t x ;
	uint8_t max ;
	if ( s )
	{
		x = *s++ ;
		max = *s++ ;
		if ( value > max )
		{
			value = max ;
		}
#ifdef X20
		lcdDrawTextAtIndex( x, y, s, value, edit ? InverseBlink: 0 ) ;
#else
		PUTS_AT_IDX( x, y, s, value, (edit ? InverseBlink: 0)|LUA_RIGHT ) ;
#endif
	}
	else
	{
		x = PARAM_OFS ;
		max = 1 ;
		menu_lcd_onoff( x, y, value, edit ) ;
	}
	if ( value > max )
	{
		value = max ;
	}
	if(edit)
	{
		if ( ( EditColumns == 0 ) || ( s_editMode ) || (max == 1) )
		{
			value = checkIncDec( value, 0, max, EditType ) ;
		}
	}
	return value ;
}

void DisplayScreenIndex(uint32_t index, uint32_t count, LcdFlags attr)
{
	if ( RotaryState == ROTARY_MENU_LR )
	{
		attr = BLINK ;
	}
#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
	char text[8] ;
	uint32_t i = 0 ;
	index += 1 ;
	if ( index > 9)
	{
		index -= 10 ;
		text[i++] = '1' ;
	}
	text[i++] = '0' + index ;
	text[i++] = '/' ;
	
	if ( count > 9)
	{
		count -= 10 ;
		text[i++] = '1' ;
	}
	text[i++] = '0' + count ;
	text[i] = 0 ;
#ifdef X20
	lcdDrawText( LCD_W-1, 0, text, attr|LUA_RIGHT) ;
#else
	PUTS_ATT( RIGHT_POSITION, 0, text, attr|LUA_RIGHT) ;
#endif
//	PUTS_NUM(RIGHT_POSITION,0,count,attr);
//	x = 1+(RIGHT_POSITION+1)-FW*(count>9 ? 3 : 2) ;
//  PUTC_ATT(x,0,'/',attr);
//  PUTS_NUM(x-1,0,index+1,attr);
#else
	coord_t x ;
	PUTS_NUM(RIGHT_POSITION,0,count,attr);
	x = 1+(RIGHT_POSITION+1)-FW*(count>9 ? 3 : 2) ;
  PUTC_ATT(x,0,'/',attr);
  PUTS_NUM(x-1,0,index+1,attr);
#endif
}

struct te_InputsData *getActiveInput( uint8_t channel )
{
	struct te_InputsData *pinput ;
	for ( uint32_t i = 0 ; i < NUM_INPUT_LINES ; i += 1 )
	{
		pinput = &g_model.inputs[i] ;
		if ( ( pinput->chn == 0 ) || (pinput->chn > channel) )
		{
			break ;
		}
		if ( pinput->chn < channel )
		{
			continue ;
		}
		if ( pinput->swtch )
		{
			if ( getSwitch00( pinput->swtch ) == 0 )
			{
				continue ;
			}
		}
		if ( pinput->flightModes & ( 1 << CurrentPhase ) )
		{
			continue ;
		}
		return pinput ;
	}
	return 0 ;
}



#ifdef COLOUR_DISPLAY
uint32_t touchOnOffItemMultiple( uint8_t value, coord_t y, uint8_t condition, uint16_t colour )
{
  if (value)
	{
		value = 1 ;
#ifdef X20
		putTick( TRIGHT-FW-17, y+TVOFF-1+2, condition ? ~LcdForeground : LcdForeground ) ;
#else
		putTick( (TRIGHT-TRMARGIN-FW)-2, (y+TVOFF), condition ? ~LcdForeground : LcdForeground ) ;
#endif
	}
#ifdef X20
	lcdHiresRect( TRIGHT-FW-17, y+TVOFF-1+2, 28, 28, condition ? ~LcdForeground : LcdForeground ) ;
#else
	lcd_rectColour( TRIGHT-TRMARGIN-FW-2, y+TVOFF, 8, 8, condition ? ~LcdForeground : LcdForeground ) ;
#endif
	if ( condition )
	{
		value = checkIncDec( value, 0, 1, EditType ) ;
	}
	return value ;
}


uint32_t touchOnOffItem( uint8_t value, coord_t y, const char *s, uint8_t condition, uint16_t colour )
{
	drawItem( (char *)s, y, condition ) ;
	return touchOnOffItemMultiple( value, y, condition, colour ) ;
}

uint32_t touchOffOnItem( uint8_t value, coord_t y, const char *s, uint8_t condition, uint16_t colour )
{
	return 1- touchOnOffItem( 1- value, y, s, condition, colour ) ;
}

#endif

#ifdef TOUCH

uint16_t handleTouchSelect( uint32_t rows, uint32_t pgOfs, uint8_t sub, uint32_t flag )
{
	int32_t newSelection = checkTouchSelect( rows, pgOfs, flag ) ;
	uint16_t newVert = processSelection( sub , newSelection ) ;
	checkTouchEnterEdit( newVert ) ;
	return newVert & 0x00FF ;
}

#endif

#ifdef X20
extern const uint8_t IconTick[] =
{
24,24,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x0F,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x0F,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x0F,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xFF,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x0F,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xFF,0x0F,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xF0,0xFF,0x0F,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xF0,0xFF,0x00,0x00,0x00,0x00,0x00,
0x00,0xFF,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,
0xFF,0xFF,0x0F,0x00,0xF0,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,
0xF0,0xFF,0xFF,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xFF,0xFF,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xF0,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xF0,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xF0,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
} ;
#endif


char *getTelText( uint32_t index )
{
	char *a = 0 ;
	uint8_t *s ;

	if ( ( index >= 68 ) && ( index <= 73 ) ) // A Custom sensor
	{
		index -= 68 ;
		index *= 4 ;
		s = &g_model.customTelemetryNames[index] ;
		if ( *s && (*s != ' ' ) )
		{
			a = (char *)s ;
		}
	}
	else if ( ( index >= 82 ) && ( index <= 85 ) ) // A Custom sensor
	{
#ifdef X20
		index -= (82-6) ;
#else
		index -= (82) ;
#endif
		index *= 4 ;
#ifdef X20
		s = &g_model.customTelemetryNames[index] ;
#else
		s = &g_model.customTelemetryNames2[index] ;
#endif
		if ( *s && (*s != ' ' ) )
		{
			a = (char *)s ;
		}
	}
	else if ( ( index >= 37 ) && ( index <= 44 ) )	// A Scaler
	{
		index -= 37 ;
		s = g_model.Scalers[index].name ;
		if ( *s && (*s != ' ' ) )
		{
			a = (char *)s ;
		}
	}
	return a ;
}

extern uint8_t TelemMap[] ;
extern uint8_t TelemSize ;

void sortTelemText()
{
	uint32_t i ;
	uint32_t j ;
	uint8_t temp ;
	char *s ;

	for ( i = 0 ; i < TelemSize ; i += 1 )
	{
		TelemMap[i] = i ;
	}

	for ( j = TelemSize - 1 ; j > 1 ; j -= 1 )
	{
		for ( i = 0 ; i < j ; i += 1 )
		{
			char *a ;
			char *b ;
			a = (char *)PSTR(STR_TELEM_ITEMS) + 4 * TelemMap[i] + 5 ;
			b = (char *)PSTR(STR_TELEM_ITEMS) + 4 * TelemMap[i+1] + 5 ;
			s = getTelText( TelemMap[i] ) ;
			if ( s )
			{
				a = s ;
			}
			s = getTelText( TelemMap[i+1] ) ;
			if ( s )
			{
				b = s ;
			}
			if ( strncmp( a, b, 4 ) > 0 )
			{
				temp = TelemMap[i] ;
				TelemMap[i] = TelemMap[i+1] ;
				TelemMap[i+1] = temp ;
			}
		}
	}
}

char *getIndexedText( char *p, const char * s, uint8_t idx, uint32_t maxLen )
{
	uint32_t length ;
	length = *s++ ;

	if ( length < maxLen )
	{
		p = (char *)ncpystr( (uint8_t *)p, (uint8_t *)s+length*idx, length ) ;
	}
	*p = 0 ;
	return p ;
}

uint32_t doPopup( const char *list, uint16_t mask, uint8_t width, uint8_t event ) ;
extern int8_t s_curItemIdx ;
extern bool reachMixerCountLimit( void ) ;
extern void insertMix(uint8_t idx, uint8_t copy) ;
extern uint8_t s_moveMode;
extern void menuMixOne(uint8_t event) ;
void menuTemplates(uint8_t event) ;
void menuDeleteMix(uint8_t event) ;
extern uint8_t mixToDelete ;
extern uint8_t s_moveItemIdx ;
extern bool s_currMixInsMode ;


void mixpopup( uint8_t event )
{
	uint8_t popaction = doPopup( PSTR(STR_MIX_POPUP), 0x7F, 11, event ) ;
	
  if ( popaction == POPUP_SELECT )
	{
		uint8_t popidx = PopupData.PopupSel ;
		if ( popidx == 1 )
		{
      if ( !reachMixerCountLimit())
      {
				s_currMixInsMode = 1 ;
      	insertMix(++s_curItemIdx, 0 ) ;
  	    s_moveMode=false;
			}
		}
		if ( popidx < 2 )
		{
	    pushMenu(menuMixOne) ;
		}
		else if ( popidx == 4 )		// Delete
		{
			mixToDelete = s_curItemIdx;
			killEvents(event);
			Tevent = 0 ;
			pushMenu(menuDeleteMix);
		}
		else if ( popidx == 5 )		// Clear all
		{
			mixToDelete = 0xFF ;
			killEvents(event);
			Tevent = 0 ;
			pushMenu(menuDeleteMix);
		}
		else if ( popidx == 6 )		// Templates
		{
			pushMenu( menuTemplates ) ;
		}
		else
		{
			if( popidx == 2 )	// copy
			{
     		insertMix(++s_curItemIdx, 1 ) ;
			}
			// PopupIdx == 2 or 3, copy or move
			s_moveMode = 1 ;
		}
		PopupData.PopupActive = 0 ;
	}
	s_moveItemIdx = s_curItemIdx ;

}






//--------------------------------

#ifndef X20

extern char LastItem[] ;

extern const uint8_t IconInput[] =
{
5,8,5,
 #ifdef COLOUR_DISPLAY
0xff,0xdb,0xc3,0xdb,0xff
 #else
0x7f,0x5b,0x43,0x5b,0x7f
 #endif
} ;

 #ifdef COLOUR_DISPLAY

extern const uint8_t IconTick[] =
{
12,12,
  #ifdef INVERT_DISPLAY
0x00,0x00,0x00,0x00,0x0F,0x00,
0x00,0x00,0x00,0xF0,0xFF,0x00,
0x00,0x00,0x00,0xF0,0xFF,0x00,
0x00,0x00,0x00,0xFF,0xFF,0x0F,
0x00,0x00,0xF0,0x0F,0xF0,0xFF,
0x00,0x00,0xF0,0x0F,0xF0,0x0F,
0x00,0x00,0xFF,0x00,0x00,0x00,
0x00,0xF0,0x0F,0x00,0x00,0x00,
0x00,0xFF,0x00,0x00,0x00,0x00,
0xF0,0x0F,0x00,0x00,0x00,0x00,
0xFF,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
  #else
0x00,0x00,0x00,0x00,0x00,0x00,   //                
0x00,0x00,0x00,0x00,0x00,0xFF,   //          FF
0x00,0x00,0x00,0x00,0xF0,0x0F,   //         FF
0x00,0x00,0x00,0x00,0xFF,0x00,   //        FF
0x00,0x00,0x00,0xF0,0x0F,0x00,   //       FF
0x00,0x00,0x00,0xFF,0x00,0x00,   //      FF
0xF0,0x0F,0xF0,0x0F,0x00,0x00,   // FF  FF
0xFF,0x0F,0xF0,0x0F,0x00,0x00,   //FFF  FF
0xF0,0xFF,0xFF,0x00,0x00,0x00,   // FFFFF
0x00,0xFF,0x0F,0x00,0x00,0x00,   //  FFF
0x00,0xFF,0x0F,0x00,0x00,0x00,   //  FFF
0x00,0xF0,0x00,0x00,0x00,0x00,   //   F
  #endif
} ;
 #else
extern const uint8_t IconTick[] =
{
5,8,5,
0x18,0x30,0x18,0x0c,0x06
} ;

 #endif
#endif

#ifdef COLOUR_DISPLAY
void putTick( coord_t x, coord_t y, uint16_t colour )
#else
void putTick( coord_t x, coord_t y )
#endif
{
#ifdef COLOUR_DISPLAY
 #ifdef X20
	lcdDrawPropChDma( x+2, y+2, 24, 24, ICON_CM_A4, colour, (uint8_t *)IconTick+2 ) ;
 #else
	lcdDrawPropImage( x*2+2, y*2+2, 12, 12, ICON_CM_A4, colour, (uint8_t *)IconTick+2 ) ;
 #endif
#else
	lcd_img( x, y, IconTick, 0, 0 ) ;
#endif
}



//-------------------------------------------





#ifndef X20


int16_t InputAnas[NUM_INPUTS] ;


const char *get_curve_string()
{
    return PSTR(CURV_STR)	;
}	




#if defined(PCBX12D) || defined(PCBX10)
void menu_lcd_onoff( coord_t x, coord_t y, uint8_t value, uint16_t mode )
#else
void menu_lcd_onoff( coord_t x,coord_t y, uint8_t value, uint8_t mode )
#endif
{
	x += FW ;
  if (value)
	{
#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
 #ifdef COLOUR_DISPLAY
		putTick( x+0, y+1 ) ;
 #else
		putTick( x+1, y+1 ) ;
//		lcdDrawChar(x+2, y-4, 'x', 0 ) ;
 #endif
#else
		putTick( x+1, y+1 ) ;
#endif
	}
#if defined(PCBX12D) || defined(PCBX10)
	lcd_hbar( x, y+1, 8, 8, mode ? 100 : 0 ) ;
#else
	lcd_hbar( x, y, 7, 7, mode ? 100 : 0 ) ;
#endif
}

// knots to km/h, multiply by 1.852
// knots to mph, multiply by 1.15078
// type is 0 for km/h, 1 for mph
#ifndef SMALL
int16_t knots_to_other( int16_t knots, uint32_t type )
{
	if ( type )
	{
		return knots * 57539 / 50000 ;	// OK to 36000 kts
	}
	return (int32_t)knots * 1852 / 1000 ;
}
#endif


char *arduFlightMode( uint16_t mode )
{
//Arduplane flight mode numbers:

//0 Manual
//1 CIRCLE
//2 STABILIZE
//3 TRAINING
//4 ACRO
//5 FBWA
//6 FBWB
//7 CRUISE
//8 AUTOTUNE
//10 Auto
//11 RTL
//12 Loiter
//15 Guided

//Arducopter flight mode numbers:

//0 Stabilize
//1 Acro
//2 AltHold
//3 Auto
//4 Guided
//5 Loiter
//6 RTL
//7 Circle
//9 Land
//11 Drift
//13 Sport
//14 Flip
//15 AutoTune
//16 PosHold
//17 Brake

// To add:
// 18: Throw
// 19: Avoid ADSB
// 20: Guided No GPS
// 21: Smart RTL
// 22: Flow Hold
// 23: Follow


#define STR_MAV_FM_STAB  "STAB"
#define STR_MAV_FM_ACRO  "ACRO"
#define STR_MAV_FM_2     "A-Hold"
#define STR_MAV_FM_AUTO  "AUTO"
#define STR_MAV_FM_GUIDED "GUIDED"
#define STR_MAV_FM_LOITER "LOITER"
#define STR_MAV_FM_RTL 	 "RTL"
#define STR_MAV_FM_CIRCLE "CIRCLE"
//#define STR_MAV_FM_8     "MODE8"
#define STR_MAV_FM_9 		 "LAND"
#define STR_MAV_FM_11		 "DRIFT"
#define STR_MAV_FM_13		 "SPORT"
#define STR_MAV_FM_14		 "FLIP"
#define STR_MAV_FM_ATUNE "A-TUNE"
#define STR_MAV_FM_16		 "POSHOLD"
#define STR_MAV_FM_17		 "BRAKE"
#define STR_MAV_FM_MAN	 "MANUAL"
#define STR_MAV_FM_TRAIN "TRAIN"
#define STR_MAV_FM_FBWA	 "FBWA"
#define STR_MAV_FM_FBWB	 "FBWB"
#define STR_MAV_FM_CRUISE "CRUISE"
#define STR_MAV_FM_FLIP	 "FLIP"
#define STR_MAV_FM_SRTL	 "SRTL"
#define STR_MAV_FM_X		 "UNKNOWN"

	uint32_t type = mode >> 8 ;
	mode &= 0xFF ;

	const char *s ;
	if ( g_model.telemetryProtocol == TELEMETRY_MAVLINK )
	{
		type = type == 1  ;
	}
	else
	{
		type = g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ;
	}
	switch ( mode )
	{
		case  0 : s = type ? STR_MAV_FM_MAN : STR_MAV_FM_STAB ; break ;
		case  1 : s = type ? STR_MAV_FM_CIRCLE : STR_MAV_FM_ACRO ; break	;
		case  2 : s = type ? STR_MAV_FM_STAB : STR_MAV_FM_2 ; break	;
		case  3 : s = type ? STR_MAV_FM_TRAIN : STR_MAV_FM_AUTO ; break	;
		case  4 : s = type ? STR_MAV_FM_ACRO : STR_MAV_FM_GUIDED ; break	;
		case  5 : s = type ? STR_MAV_FM_FBWA : STR_MAV_FM_LOITER ; break	;
		case  6 : s = type ? STR_MAV_FM_FBWB : STR_MAV_FM_RTL ; break	;
		case  7 : s = type ? STR_MAV_FM_CRUISE : STR_MAV_FM_CIRCLE ; break	;
		case  8 : s = type ? STR_MAV_FM_ATUNE : STR_MAV_FM_X ; break	;
		case  9 : s = type ? STR_MAV_FM_X : STR_MAV_FM_9 ; break	;
		case  10 : s = type ? STR_MAV_FM_AUTO : STR_MAV_FM_X ; break	;
		case  11 : s = type ? STR_MAV_FM_RTL : STR_MAV_FM_11 ; break	;
		case  12 : s = type ? STR_MAV_FM_LOITER : STR_MAV_FM_X ; break	;
		case  13 : s = type ? STR_MAV_FM_X: STR_MAV_FM_13 ; break	;
		case  14 : s = type ? STR_MAV_FM_X: STR_MAV_FM_FLIP ; break	;
		case  15 : s = type ? STR_MAV_FM_GUIDED: STR_MAV_FM_ATUNE ; break	;
		case  16 : s = type ? STR_MAV_FM_X : STR_MAV_FM_16 ; break	;
		case  17 : s = type ? STR_MAV_FM_X : STR_MAV_FM_17 ; break	;
		case  21 : s = STR_MAV_FM_SRTL ; break ;
		default : s = STR_MAV_FM_X ; break ;
	}
	return (char *) s ;
}

#if defined(PCBX12D) || defined(PCBX10)
uint8_t hyphinvMenuItem( uint8_t value, coord_t y, uint8_t condition, coord_t newX )
#else
uint8_t hyphinvMenuItem( uint8_t value, coord_t y, uint8_t condition )
#endif
{
#if defined(PCBX12D) || defined(PCBX10)
	char text[12] ;
	cpystr( (uint8_t *)text, (uint8_t *)XPSTR("\176" "\001" "\003---INV" ) ) ;
	if ( newX)
	{
		text[0] = newX ;
	}
	return checkIndexed( y, text, value, condition ) ;
#else
	return checkIndexed( y, XPSTR("\176" "\001" "\003---INV"), value, condition ) ;
#endif
}

uint16_t xnormalize( int16_t alpha )
{
   if ( alpha > 360 ) alpha -= 360 ;
   if ( alpha <   0 ) alpha += 360 ;
   return alpha ;
}

int8_t rxsin100( int16_t a )
{
	int32_t b ;
	int32_t neg = 0 ;
  a = xnormalize( a ) ;
	if ( a > 180 )
	{
		neg = 1 ;
		a -= 180 ;
	}
	if ( a > 90 )
	{
		a = 180 - a ;
	}
// Angle in degrees (0-90) = a
// 3823-a*a/20 = b ;
// 3823-(b*a*a/19697) = c
// sin = c*a/1880
	b = 3283 - a * a / 20 ;
	b = 3283 - b * a * a / 19697 ;
	b = b * a / 1880 ;
	if ( neg )
	{
		b = -b ;
	}
	return b ;
}

int8_t rxcos100( int16_t a )
{
	a += 90 ;
	return rxsin100( xnormalize( a ) ) ;
}

#ifndef TOUCH
void displayGPSformat( uint16_t x, uint16_t y, uint8_t attr )
{
#ifdef PROP_TEXT
	PUTS_AT_IDX(  x, y, XPSTR("\012DD mm.mmmmDD.dddddd "), g_eeGeneral.gpsFormat, attr|LUA_RIGHT ) ;
#else
	PUTS_AT_IDX(  x, y, XPSTR("\012DD mm.mmmmDD.dddddd "), g_eeGeneral.gpsFormat, attr|LUA_RIGHT ) ;
#endif
}
#endif



#if defined(PCBX12D) || defined(PCBX10)

extern const uint8_t * const m_icons[] ;

void attachIcon( uint32_t x, uint32_t y, const uint8_t *icon )
{
	uint32_t w ;
	uint32_t h ;
	w = *icon ;
	h = *(icon+1) ;
	x = 2*x - (w+4) ;
	y = 2*y - (h/2-8) ;
	lcdDrawIcon( x, y, icon, 0 ) ;
}

void displayIndexIcons( const uint8_t *icons[], uint8_t extra, uint8_t lines )
{
	uint8_t offset = 7 ;
	if ( extra == 8 )
	{
		offset = 8 ;
		if ( icons[7] )
		{
			attachIcon( 94 + X12OFFSET, 10, icons[7] ) ;
		}
	}
	for ( uint32_t i = 0 ; i < lines ; i += 1 )
	{
		if ( icons[i] )
		{
			attachIcon( 1 + X12OFFSET, (i+1)*(14)+10, icons[i] ) ;
		}
		if ( i < extra )
		{
			if ( icons[i+offset] )
			{
				attachIcon( 94 + X12OFFSET, (i+1)*(14)+10, icons[i+offset] ) ;
			}
		}
	} 
}



void displayIndex( const uint16_t *strings, uint8_t extra, uint8_t lines, uint8_t highlight )
{
	uint8_t offset = 7 ;
#ifdef LARGE_INDEX	
//	if ( extra == 8 )
//	{
//		offset = 8 ;

//		lcd_putsAtt( 122, 9, PSTR(strings[7]), DBLSIZE | CONDENSED ) ;
//	}
//	for ( uint8_t i = 0 ; i < lines ; i += 1 )
//	{
//		lcd_putsAtt( 32, i*13+22, PSTR(strings[i]), DBLSIZE | CONDENSED ) ;
//		if ( i < extra )
//		{
//			lcd_putsAtt( 122, i*13+22, PSTR(strings[i+offset]), DBLSIZE | CONDENSED ) ;
//		}
//	} 
	
//	pushPlotType( PLOT_BLACK ) ;
//	lcd_vline( 120, 10, 104 ) ;
//	popPlotType() ;

//	if ( highlight )
//	{
//		if ( highlight > 7 )
//		{
//			lcd_char_inverse( 122, (highlight-offset)*13+9, 80, 0, 13 ) ;
//		}
//		else
//		{
//			lcd_char_inverse( 32, (highlight)*13+9, 87, 0, 13 ) ;
//		}
//	}
	
#else
	if ( extra == 8 )
	{
		offset = 8 ;
		
		PUTS_P( 94 + X12OFFSET, 10, PSTR(strings[7]) ) ;
	}
	for ( uint32_t i = 0 ; i < lines ; i += 1 )
	{
		PUTS_P( 1 + X12OFFSET, (i+1)*(14)+10, PSTR(strings[i]) ) ;
		if ( i < extra )
		{
			PUTS_P( 94 + X12OFFSET, (i+1)*(14)+10, PSTR(strings[i+offset]) ) ;
		}
	} 
	
	pushPlotType( PLOT_BLACK ) ;
	lcd_vline( 67 + X12OFFSET, 10, 111 ) ;
	popPlotType() ;

	if ( highlight )
	{
		if ( highlight > 7 )
		{
			lcd_char_inverse( 93 + X12OFFSET, (highlight-offset)*(14)+10-3, 60, 0, 14 ) ;
		}
		else
		{
			lcd_char_inverse( 0 + X12OFFSET, (highlight)*(14)+10-3, 67, 0, 14 ) ;
		}
	}
#endif
}
#else
void displayIndex( const uint16_t *strings, uint8_t extra, uint8_t lines, uint8_t highlight )
{
	uint8_t offset = 7 ;
	if ( extra == 8 )
	{
		offset = 8 ;
		
		PUTS_P( 69 + X12OFFSET, 0, PSTR(strings[7]) ) ;
	}
	for ( uint32_t i = 0 ; i < lines ; i += 1 )
	{
		PUTS_P( 1 + X12OFFSET, (i+1)*FH, PSTR(strings[i]) ) ;
		if ( i < extra )
		{
			PUTS_P( 69 + X12OFFSET, (i+1)*FH, PSTR(strings[i+offset]) ) ;
		}
	} 
	
	lcd_vline( 67 + X12OFFSET, 0, 63 ) ;

	if ( highlight )
	{
		if ( highlight > 7 )
		{
			lcd_char_inverse( 68 + X12OFFSET, (highlight-offset)*FH, 60, 0 ) ;
		}
		else
		{
			lcd_char_inverse( 0 + X12OFFSET, highlight*FH, 67, 0 ) ;
		}
	}
}
#endif

void lcd_xlabel_decimal( coord_t x, coord_t y, uint16_t value, LcdFlags attr, const char *s )
{
	PUTS_ATT_LEFT( y, s ) ;
  PUTS_NUM( x, y, value, attr ) ;
}




// returns -350 to +350 or 510 to 514 for gvar
int16_t calcExtendedValue( int16_t value, uint8_t extValue )
{
	if ( (value <= -126) || (value >= 126) )
	{
		// A gvar ;
		if ( value < 0 )
		{
			value += 256 ;
		}
		value += 510 - 126 ;
		return value ;
	}
	if ( extValue == 1 )
	{
		value += 125 ; 
	}
	else if ( extValue == 3 )
	{
		value -= 125 ; 
	}
	else if ( extValue == 2 )
	{
		if ( value < 0 )
		{
			value -= 250 ;
		}
		else
		{
			value += 250 ;
		}
	}
	if ( value > 350 )
	{
		value += 510 - 360 ;
	}
	return value ;
}

uint16_t packExtendedValue( int16_t value )
{
	uint32_t extValue = 0 ;

	if ( value > 125 )
	{
		extValue = 1 ;
		value -= 125 ;
		if ( value > 125 )
		{
			extValue = 2 ;
			value -= 125 ;
		}
	}
	else if ( value < -125 )
	{
		extValue = 3 ;
		value += 125 ;
		if ( value < -125 )
		{
			extValue = 2 ;
			value += 125 ;
		}
	}
	return ( value & 0xFF ) | ( extValue << 8 ) ;
}




#if defined(PCBX12D) || defined(PCBX10)
void putsChnRaw(coord_t x,coord_t y,uint8_t idx, LcdFlags att, uint16_t colour , uint16_t bgColour )
#else
void putsChnRaw(coord_t x,coord_t y,uint8_t idx, LcdFlags att)
#endif
{
	uint8_t chanLimit = NUM_SKYXCHNRAW ;
	uint8_t mix = att & MIX_SOURCE ;
	LastItem[0] = '\0' ;
	if ( mix )
	{
		chanLimit += MAX_GVARS + 1 + 1 ;
		att &= ~MIX_SOURCE ;		
	}
  if(idx==0)
	{
		ncpystr( (uint8_t *)LastItem, (uint8_t *) XPSTR("----"), 4 ) ;
	}
  else if(idx<=4)
	{
		const char *ptr = "" ;
		if ( g_model.useCustomStickNames )
		{
			ptr = ( char *)g_eeGeneral.customStickNames+4*(idx-1) ;
		}
		if ( *ptr && (*ptr != ' ' ) )
		{
			ncpystr( (uint8_t *)LastItem, (uint8_t *)ptr, 4 ) ;
		}
		else
		{
			setLastIdx( (char *) PSTR(STR_STICK_NAMES), idx-1 ) ;
		}
	}
  else if(idx<=chanLimit)
		setLastIdx( (char *) PSTR(STR_CHANS_GV), idx-5 ) ;
	else if(idx < EXTRA_POTS_START)
	{
		if ( mix )
		{
			idx += TEL_ITEM_SC1-(chanLimit-NUM_SKYXCHNRAW) ;
			if ( idx - NUM_SKYXCHNRAW > TEL_ITEM_SC1 + NUM_SCALERS )
			{
				uint8_t *ptr ;
				idx -= TEL_ITEM_SC1 + NUM_SCALERS - 8 + NUM_SKYXCHNRAW ;
#if EXTRA_SKYCHANNELS							
				if ( idx > 16 )
				{
					if ( idx <= 16 + 8  )
					{
						idx += 8 ;
						ptr = cpystr( (uint8_t *)LastItem, (uint8_t *)"CH" ) ;
						*ptr++ = (idx / 10) + '0' ;
						*ptr = (idx % 10) + '0' ;
					}
					else
					{
						ptr = ncpystr( (uint8_t *)LastItem, (uint8_t *)&PSTR(STR_GV_SOURCE)[1+3*(idx-24)], 3 ) ;
//						uint32_t h = idx%10 ;
//						uint32_t j = idx/10 ;
//					  LastItem[0] = j + '0' ;
//					  LastItem[1] = h + '0' ;
//					  LastItem[2] = 0 ;
//						ptr = (uint8_t *)&LastItem[2] ;					
					}
				}
				else
#endif
				{
					ptr = cpystr( (uint8_t *)LastItem, (uint8_t *)"PPM1" ) ;
					if ( idx == 9 )
					{
						*(ptr-1) = '9' ;
					}
					else
					{
						*ptr = '0' + idx - 10 ;
					}
				}
				*(ptr+1) = '\0' ;
#ifdef COLOUR_DISPLAY
				lcdDrawText( x, y, LastItem, att ) ;
#else
				PUTS_ATT(x,y,LastItem,att ) ;
#endif
				return ;
			}
		}
		setLastTelemIdx( idx-NUM_SKYXCHNRAW ) ;
//		setLastIdx( (char *) PSTR(STR_TELEM_ITEMS), idx-NUM_SKYXCHNRAW ) ;
	}
	else
	{
		if(idx < EXTRA_POTS_START + 8)
		{
			setLastIdx( (char *) PSTR(STR_CHANS_EXTRA), idx-EXTRA_POTS_START ) ;
		}
		else
		{
#ifdef INPUTS
			if ( mix )
			{
				if ( idx >= 224 )	// Input
				{
#ifdef COLOUR_DISPLAY
					displayInputName( x/2, y/2, idx-224+1, att ) ;
#else
					displayInputName( x, y, idx-224+1, att ) ;
#endif
//					lcd_img( x, y, IconInput, 0, 0 ) ;
//					lcd_outdezNAtt(x+3*FW, y, idx-224, att + LEADING0, 2) ;
					return ;
				}
				uint32_t h = idx ;
				h -= 127 ;
				ncpystr( (uint8_t *)LastItem, (uint8_t *)&PSTR(STR_GV_SOURCE)[1+3*(h)], 3 ) ;
			}
			else
#endif
			{
				// Extra telemetry items
				setLastTelemIdx( idx-NUM_SKYXCHNRAW-8 ) ;
//			setLastIdx( (char *) PSTR(STR_TELEM_ITEMS), idx-NUM_SKYXCHNRAW-8 ) ;
			}
		}
	}
#ifdef COLOUR_DISPLAY
	lcdDrawSizedText( x, y, LastItem, 255, att, colour ) ;
#else
	PUTS_ATT( x, y, LastItem, att ) ;
#endif
}


#if defined(PCBX12D) || defined(PCBX10)
uint32_t putTxSwr( coord_t x, coord_t y, LcdFlags attr, uint16_t colour, uint16_t bgColour )
#else
uint32_t putTxSwr( coord_t x, coord_t y, LcdFlags attr )
#endif
{
	uint32_t index = 0 ;
	if ( FrskyTelemetryType == FRSKY_TEL_SPORT )
	{
		index = 1 ;
	}
	if ( ( index == 0 ) && (attr & TSSI_TEXT) )
	{
		return 0 ;
	}
	PUTS_AT_IDX( x, y, PSTR(STR_TXEQ), index, attr ) ;
	setLastIdx( (char *) PSTR(STR_TXEQ), index ) ;
	return 1 ;
}

int16_t calc_scaler( uint8_t index, uint16_t *unit, uint8_t *num_decimals)
{
	int32_t value ;
	int32_t exValue ;
	uint8_t lnest ;
	ScaleData *pscaler ;
	ExtScaleData *epscaler ;
	
	lnest = CalcScaleNest ;
	if ( lnest > 5 )
	{
		return 0 ;
	}
	CalcScaleNest = lnest + 1 ;
	// process
	pscaler = &g_model.Scalers[index] ;
	epscaler = &g_model.eScalers[index] ;
	if ( pscaler->source )
	{
		value = getValue( pscaler->source - 1 ) ;
		if ( ( pscaler->source == NUM_SKYXCHNRAW+1 ) || ( pscaler->source == NUM_SKYXCHNRAW+2 ) )
		{
			value = scale_telem_value( value, pscaler->source - NUM_SKYXCHNRAW-1, NULL ) ;
		}
	}
	else
	{
		value = 0 ;
	}
	if ( !pscaler->offsetLast )
	{
		value += pscaler->offset ;
	}
	uint16_t t ;
	t = pscaler->mult + ( pscaler->multx << 8 ) ;
	value *= t+1 ;
	t = pscaler->div + ( pscaler->divx << 8 ) ;
	value /= t+1 ;
	if ( epscaler->mod )
	{
		value %= epscaler->mod+1 ;
	}
	if ( epscaler->exSource )
	{
		exValue = getValue( epscaler->exSource - 1 ) ;
		if ( ( epscaler->exSource == NUM_SKYXCHNRAW+1 ) || ( epscaler->exSource == NUM_SKYXCHNRAW+2 ) )
		{
			exValue = scale_telem_value( exValue, epscaler->exSource - NUM_SKYXCHNRAW-1, NULL ) ;
		}
		if ( pscaler->exFunction )
		{
			switch ( pscaler->exFunction )
			{
				case 1 :	// Add
					value += exValue ;
				break ;
				case 2 :	// Subtract
					value -= exValue ;
				break ;
				case 3 :	// Multiply
					value *= exValue ;
				break ;
				case 4 :	// Divide
					if ( exValue )
					{
						value /= exValue ;
					}
				break ;
				case 5 :	// Mod
					if ( exValue )
					{
						value %= exValue ;
					}
				break ;
				case 6 :	// Min
					if ( exValue )
					{
						if ( exValue < value )
						{
							value = exValue ;
						}
					}
				break ;
			}
		}
	}
	CalcScaleNest = lnest ;
	if ( pscaler->offsetLast )
	{
		value += pscaler->offset ;
	}
	if ( pscaler->neg )
	{
		value = -value ;
	}
	// Limit to an int16_t
	if ( value > 32767 )
	{
		value = 32767 ;
	}
	if ( value < -32768 )
	{
		value = -32768 ;
	}
	if ( unit )
	{
		*unit = pscaler->unit ;
	}
	if ( num_decimals )
	{
		*num_decimals = pscaler->precision ;
	}
	if ( epscaler->dest )
	{
		store_telemetry_scaler( epscaler->dest, value ) ;
	}

	return value ;
}

#if defined(PCBX12D) || defined(PCBX10)
void putsAttIdxTelemItems( coord_t x, coord_t y, uint8_t index, LcdFlags attr, uint16_t colour, uint16_t bgColour )
#else
void putsAttIdxTelemItems( coord_t x, coord_t y, uint8_t index, LcdFlags attr )
#endif
{
	if ( index == 4 )
	{
#if defined(PCBX12D) || defined(PCBX10)
		if ( putTxSwr( x, y, attr, colour, bgColour ) )
#else
		if ( putTxSwr( x, y, attr ) )
#endif
		{
			return ;
		}
	}
	attr &= ~TSSI_TEXT ;
	if ( index >= TELEM_GAP_START + 8 )
	{
		index -= 8 ;
	}
	setLastTelemIdx( index ) ;
#if defined(PCBX12D) || defined(PCBX10)
	PUTS_ATT_COLOUR(x,y,LastItem,attr, colour ) ; // , bgColour);
#else
 #ifdef PROP_TEXT
	lcdDrawText(x,y,LastItem,attr);
 #else
	lcd_putsAtt(x,y,LastItem,attr);
 #endif
#endif
//	lcd_putsAttIdx( x, y, PSTR(STR_TELEM_ITEMS), index, attr ) ;
//	setLastIdx( (char *) PSTR(STR_TELEM_ITEMS), index ) ;
}

int32_t getGvarValue100( int32_t value )
{
	if ( value > 100 )
	{ // GVAR
//#if MULTI_GVARS
//		int32_t gvindex = value - (( g_model.flightModeGvars ) ? 113 : 110 ) ;
//#else
		int32_t gvindex = value - 110 ;
//#endif
		value = getGvar(gvindex) ;
		if ( value < -100 )
		{
			value = -100 ;
		}
		if ( value > 100 )
		{
			value = 100 ;
		}
	}
	return value ;
}

#ifdef INPUTS
void displayInputName( uint16_t x, uint16_t y, uint8_t inputIndex, LcdFlags attr )
{
	struct te_InputsData *pinput = &g_model.inputs[inputIndex-1] ;
	uint32_t value = pinput->srcRaw ;
#if defined(PCBX12D) || defined(PCBX10)
	lcd_img( x, y+1, IconInput, 0, 0 ) ;
#else
	lcd_img( x, y, IconInput, 0, 0 ) ;
#endif
  if( value && (value <= 4) )
	{
		const char *ptr = "" ;
		if ( g_model.useCustomStickNames )
		{
			ptr = ( char *)g_eeGeneral.customStickNames+4*(value-1) ;
		}
		if ( !( *ptr && (*ptr != ' ' ) ) )
		{
    	PUTS_AT_IDX( x+FW, y, PSTR(STR_STICK_NAMES), value-1, attr ) ;
		}
		else
		{
			PUTS_ATT( x+FW, y, ptr, attr ) ;
		}
	}
	else
	{
		PUTS_NUM_N(x+3*FW-1, y, inputIndex, attr + LEADING0, 2) ;
	}
}

int16_t processOneInput( struct te_InputsData *pinput, int16_t value )
{
	int32_t result = 0 ;
	if ( pinput )
	{
		result = value ;
		switch ( pinput->mode )
		{
			case 1 :		// Function
				switch ( pinput->curve )
				{
					case 1 :
						if ( result < 0 )
						{
							result = 0 ;
						}
					break ;
					case 2 :
						if ( result > 0 )
						{
							result = 0 ;
						}
					break ;
					case 3 :
						result = abs(result) ;
					break ;
					case 4 :
						result = result > 0 ? 100 : 0 ;
					break ;
					case 5 :
						result = result < 0 ? -100 : 0 ;
					break ;
					case 6 :
						result = result > 0 ? 100 : -100 ;
					break ;
				}

			break ;
			case 2 :		// curve
				result = intpol( result, pinput->curve ) ;
			break ;
			case 3 :		// diff
			{	
				int32_t diffValue ;
#ifdef USE_VARS
				if ( g_model.vars )
				{
					if ( pinput->varForCurve )
					{
						diffValue = getVarValWholeL100( pinput->curve ) ;
					}
					else
					{
						diffValue = pinput->curve ;
					}
				}
				else
#endif
				{
					diffValue = getGvarValue100( pinput->curve ) ;
				}
     		if (diffValue > 0 && result < 0)
				{
     			result = (result * (100 - diffValue)) / 100 ;
				}
     		else if (diffValue < 0 && result > 0)
				{
     			result = (result * (100 + diffValue)) / 100 ;
				}
			}
			break ;
			case 4 :		// expo
			{
				int32_t expoValue ;
#ifdef USE_VARS
				if ( g_model.vars )
				{
					if ( pinput->varForCurve )
					{
						expoValue = getVarValWholeL100( pinput->curve ) ;
					}
					else
					{
						expoValue = pinput->curve ;
					}
				}
				else
#endif
				{
					expoValue = getGvarValue100( pinput->curve ) ;
				}
				if ( pinput->srcRaw == 3 ) // Throttle
				{
					if ( g_model.thrExpo )
					{
    				result = 2*expo((result+RESX)/2, expoValue ) ;
						result -= RESX ;
						break ;						 
					}
				}
		    result  = expo( result, expoValue ) ;
			}
			break ;
		}

#ifdef USE_VARS
		if ( g_model.vars )
		{
			if ( pinput->varForWeight )
			{
				result *= getVarValue(pinput->weight) ;
				result /= 1000 ;			
			}
			else
			{
				result = result * pinput->weight / 100 ;
			}
		}
		else
#endif
		{
			result = result * getGvarValue100( pinput->weight ) / 100 ;
		}
		
#ifdef USE_VARS
		if ( g_model.vars )
		{
			if ( pinput->varForOffset )
			{
				result += calc1000toRESX( getVarValue(pinput->offset) ) ;
			}
			else
			{
				result += pinput->offset ;
			}
		}
		else
#endif
		{
			result += calc100toRESX( getGvarValue100( pinput->offset ) ) ;
		}
	}
	return result ;
}

int16_t evalInput( uint8_t channel, int16_t value )
{
	int16_t result = 0 ;
	struct te_InputsData *pinput = getActiveInput( channel ) ;
	if ( ( value < 0 && (pinput->side & 2) == 0 ) || ( value >= 0 && (pinput->side & 1) == 0 ) )
	{
		result = processOneInput( pinput, value ) ;
	}

	// Now the trims
		 
	return result ;
}

extern int8_t virtualInputsTrims[] ;

void evalAllInputs()
{
// InputAnas
	uint32_t currentChannel = 255 ;
	int32_t value ;
	int16_t result = 0 ;
	
  for ( uint32_t i = 0 ; i < NUM_INPUT_LINES ; i += 1 )
	{
		struct te_InputsData *pinput = &g_model.inputs[i] ;
		if ( pinput->chn == 0 )
		{
			break ;
		}
		if ( pinput->chn == currentChannel )
		{
			continue ;
		}
		if ( pinput->flightModes & ( 1 << CurrentPhase ) )
		{
			if ( pinput->srcRaw != 8 )	// HALF
			{				
				continue ;
			}
		}
		if ( pinput->swtch )
		{
			if ( getSwitch00( pinput->swtch ) == 0 )
			{
				if ( pinput->srcRaw != 8 )	// HALF
				{				
					continue ;
				}
			}
		}
		value = getInputSourceValue( pinput ) ;

		if ( ( value < 0 && (pinput->side & 1) == 0 ) || ( value >= 0 && (pinput->side & 2) == 0 ) )
		{
			currentChannel = pinput->chn ;
			result = processOneInput( pinput, value ) ;
			InputAnas[currentChannel-1] = result ;

			// Now the trims
			if ( pinput->carryTrim > 1 )
			{
				virtualInputsTrims[currentChannel-1] = pinput->carryTrim - 2 ;
			}
			else if (pinput->carryTrim == 0 && pinput->srcRaw >= 1 && pinput->srcRaw <= 4)
			{
				virtualInputsTrims[currentChannel-1] = pinput->srcRaw - 1 ;
			}
			else
			{
				virtualInputsTrims[currentChannel-1] = -1;
			}
		}
	}
}


extern const uint8_t switchIndex[] ;
extern int32_t chans[] ;

int16_t getInputSourceValue( struct te_InputsData *pinput )
{
	int16_t value ;
	uint32_t switchIdx ;
	if ( pinput )
	{
		if ( (pinput->srcRaw) && ( pinput->srcRaw <= 7 ) )
		{
			return calibratedStick[pinput->srcRaw-1] ;
		}
		if ( pinput->srcRaw == 8 )	// HALF
		{
			uint32_t active = 1 ;
			if ( pinput->flightModes & ( 1 << CurrentPhase ) )
			{
				active = 0 ;
			}
			if ( pinput->swtch )
			{
				if ( getSwitch00( pinput->swtch ) == 0 )
				{
					active = 0 ;
				}
				return active ? 1024 : 0 ;
			}
		}
		
		if (pinput->srcRaw >= 128)	// A Switch
		{
			switchIdx = pinput->srcRaw - 128 ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
#ifdef REV9E
			if ( switchIdx > 18 )
#else
			if ( switchIdx > 8 )	// Sort for X9E ********
#endif
			{
#ifdef REV9E
				value = getSwitch( CSW_INDEX+switchIdx-19, 0, 0 ) ;
#else
				value = getSwitch( CSW_INDEX+switchIdx-8, 0, 0 ) ;
#endif
				value = value ? 1024 : -1024 ;
			}
			else
			{
				uint32_t sw = switchIndex[switchIdx] ;
				if ( ( switchIdx == 5) || ( switchIdx == 7) )
				{ // 2-POS switch
        	value = hwKeyState(sw) ? 1024 : -1024 ;
				}
#ifdef REV9E
				else if( switchIdx == 18)
#else
				else if( switchIdx == 8)
#endif
				{
					value = ((int32_t)switchPosition( HSW_Ele6pos0 ) * 2048 - 5120)/5 ;
				}
				else
				{ // 3-POS switch
        	value = hwKeyState(sw) ? -1024 : (hwKeyState(sw+1) ? 0 : 1024) ;
				}
			}
#endif
#if defined(PCBSKY) || defined(PCB9XT)
			if ( switchIdx > 6 )
			{
				if ( switchIdx == 31 )
				{
					value = ((int32_t)switchPosition( HSW_Ele6pos0 ) * 2048 - 5120)/5 ;
				}
				else
				{
					value = getSwitch( CSW_INDEX+switchIdx-6, 0, 0 ) ;
					value = value ? 1024 : -1024 ;
				}
			}
			else
			{
				uint32_t sw = Sw3PosList[switchIdx] ;
				if ( Sw3PosCount[switchIdx] == 2 )
				{
        	value = hwKeyState(sw) ? 1024 : -1024 ;
				}
				else if ( Sw3PosCount[switchIdx] == 6 )
				{
					value = ((int32_t)switchPosition( HSW_Ele6pos0 ) * 2048 - 5120)/5 ;
				}
				else
				{
        	value = hwKeyState(sw) ? -1024 : (hwKeyState(sw+1) ? 0 : 1024) ;
				}
			}
#endif
			return value ;
		}

		if (pinput->srcRaw >= EXTRA_POTS_START )
		{
#ifdef PCBX7
			return calibratedStick[pinput->srcRaw-EXTRA_POTS_START+6] ;
#else
 #ifdef PCBX9LITE
			return calibratedStick[pinput->srcRaw-EXTRA_POTS_START+5] ;
 #else
			return calibratedStick[pinput->srcRaw-EXTRA_POTS_START+7] ;
 #endif
#endif
		}

		if ( (pinput->srcRaw >= CHOUT_BASE-1 ) && (pinput->srcRaw < CHOUT_BASE + NUM_SKYCHNOUT-1 ) )
		{
			return chans[pinput->srcRaw - CHOUT_BASE-1] ;
		}
		if ( (pinput->srcRaw >= EXTRA_CHANS_BASE-1 ) && (pinput->srcRaw < EXTRA_CHANS_BASE + EXTRA_SKYCHANNELS-1 ) )
		{
			return chans[pinput->srcRaw - (EXTRA_CHANS_BASE-NUM_SKYCHNOUT)-1] ;
		}
		if ( (pinput->srcRaw >= PPM_BASE-1 ) && (pinput->srcRaw < PPM_BASE + NUM_PPM-1 ) )
		{
			if ( ppmInValid )
			{			
				return g_ppmIns[pinput->srcRaw - PPM_BASE-1] * 2 ;
			}
		}
		if ( (pinput->srcRaw >= EXTRA_PPM_BASE-1 ) && (pinput->srcRaw < EXTRA_PPM_BASE + NUM_EXTRA_PPM-1 ) )
		{
			if ( ppmInValid )
			{			
				return g_ppmIns[pinput->srcRaw - (EXTRA_PPM_BASE-NUM_EXTRA_PPM)-1] * 2 ;
			}
		}

		if ( (pinput->srcRaw >= MIX_TRIMS_START-1 ) && (pinput->srcRaw < MIX_TRIMS_START + 4 - 1 ) )
		{
 			value = getTrimValueAdd( CurrentPhase, pinput->srcRaw - (MIX_TRIMS_START+1) ) * 8 ;
			return value ;
		}
	}
	return 0 ;
}
#endif

#endif // X20


