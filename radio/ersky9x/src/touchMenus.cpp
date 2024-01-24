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

#include "timers.h"
#include "analog.h"

#include "X12D/hal.h"

void eeLoadModel(uint8_t id) ;

#define SCREEN_LINES		15

uint32_t doPopup( const char *list, uint16_t mask, uint8_t width, uint8_t event ) ;

extern uint8_t s_moveMode;

#include "basic/basic.h"

#include "ff.h"
#include "maintenance.h"

#include "bluetooth.h"

#include "X12D/tp_gt911.h"

#include "sticks.lbm"

#define STATUS_VERTICAL		242

#define NO_HI_LEN 25

struct t_clipboard
{
	uint32_t content ;
	union
	{
		VoiceAlarmData clipvoice ;
		SKYCSwData clipswitch ;
	} ;
} ;

extern uint8_t SuppressStatusLine ;

extern struct fileControl PlayFileControl ;
extern TCHAR PlaylistDirectory[] ;
extern uint16_t PlaylistIndex ;
extern uint32_t BgSizePlayed ;
extern uint32_t BgTotalSize ;
extern char CurrentPlayName[] ;

extern uint8_t EditType ;
extern uint8_t TlExitIcon ;
extern uint8_t s_currIdx ;
extern uint8_t s_pgOfs ;
extern uint8_t FileSelectResult ;
extern char SelectedVoiceFileName[] ;
extern uint8_t AlphaEdited ;
extern uint8_t MaskRotaryLong ;
extern char SelectedVoiceFileName[] ;
extern uint8_t g_posHorz ;
extern uint8_t Columns ;
extern uint8_t VoiceFileType ;
extern uint8_t TextIndex ;
extern uint8_t TextResult ;
extern uint8_t TextType ;
extern struct t_clipboard Clipboard ;
extern uint8_t MuteTimer ;
extern int8_t s_curItemIdx;
extern const uint8_t GvaString[] ;
extern const uint8_t UnitsString[] ;
extern const uint8_t DestString[] ;
extern uint8_t s_currIdx;
extern uint8_t s_curveChan ;
extern uint8_t s_editMode;
extern uint8_t s_editing;
extern uint8_t PrivateData[] ;

extern void setStickCenter(uint32_t toSubTrims ) ; // copy state of 3 primary to subtrim
extern struct t_multiSetting MultiSetting ;

int8_t edit_dr_switch( uint8_t x, uint8_t y, int8_t drswitch, uint8_t attr, uint8_t flags, uint8_t event ) ;
void copyFileName( char *dest, char *source, uint32_t size ) ;
void menuCustomCheck(uint8_t event) ;
void menuProcSelectVoiceFile(uint8_t event) ;
uint8_t checkIndexed( uint8_t y, const char *s, uint8_t value, uint8_t edit ) ;
uint32_t checkForMenuEncoderLong( uint8_t event ) ;
void menuProcSelectImageFile(uint8_t event) ;
uint8_t mapPots( uint8_t value ) ;
void menuTextHelp(uint8_t event) ;
uint8_t putsTelemetryChannel(uint8_t x, uint8_t y, int8_t channel, int16_t val, uint8_t att, uint8_t style, uint16_t colour = LcdForeground, uint16_t bgColour = LcdBackground ) ;
void displayVoiceRate( uint8_t x, uint8_t y, uint8_t rate, uint8_t attr ) ;
void dispGvar( uint8_t x, uint8_t y, uint8_t gvar, uint8_t attr ) ;
void deleteVoice(VoiceAlarmData *pvad) ;
int8_t qRotary() ;
void menuProcGlobalVoiceAlarm(uint8_t event) ;
void moveVoice(uint8_t idx, uint8_t dir, uint8_t mode) ; //true=inc=down false=dec=up
void voicepopup( uint8_t event ) ;
uint32_t checkForMenuEncoderBreak( uint8_t event ) ;
void menuBackground( uint8_t event ) ;
void menuForeground( uint8_t event ) ;
void menuRangeBind(uint8_t event) ;
void menuProcCurveOne(uint8_t event) ;
void drawCurve( uint8_t offset ) ;
uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext ) ;
void editAccessProtocol( uint8_t module, uint8_t event ) ;
uint32_t checkProtocolOptions( uint8_t module ) ;
void displayMultiProtocol( uint32_t index, uint32_t y, uint8_t attr ) ;
void menuScanDisplay(uint8_t event) ;
uint32_t displayMultiSubProtocol( uint32_t index, uint32_t subIndex, uint32_t y, uint8_t attr ) ;
void multiOption( uint32_t x, uint32_t y, int32_t option, uint32_t attr, uint32_t protocol ) ;
void menuSetFailsafe(uint8_t event) ;


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
	lcd_puts_P( THOFF, y+TVOFF, s ) ;
	lcd_hline( 0, y+TFH, TRIGHT ) ;
	lcdDrawSolidFilledRectDMA( TMID*TSCALE, y*TSCALE+2, (TRIGHT-TMID)*TSCALE, TFH*TSCALE-2, selected ? ~DimBackColour : DimBackColour ) ;
}

void drawNumber( uint16_t x, uint16_t y, int32_t val, uint16_t mode ) //, uint16_t colour )
{
	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
	lcd_outdezNAtt( x, y+TVOFF, val, mode& ~INVERS,5, fcolour) ; //, colour ) ;
}		  

void drawText( uint16_t x, uint16_t y, char *s, uint16_t mode )
{
	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
	lcd_putsAttColour( x, y+TVOFF, s, 0, fcolour ) ;
}		  

void drawChar( uint16_t x, uint16_t y, uint8_t c, uint16_t mode, uint16_t colour )
{
	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
	lcd_putcAttColour( x, y+TVOFF, c, 0, fcolour) ; //, colour ) ;
}		  

void drawIdxText( uint16_t y, char *s, uint32_t index, uint16_t mode )
{
	uint8_t length ;
	length = *s++ ;
	
	uint16_t fcolour = mode & INVERS ? ~LcdForeground : LcdForeground ;
  lcd_putsnAttColour( TRIGHT-length*FW-TRMARGIN, y+TVOFF, s+length*index, length, 0, fcolour ) ;
}		  

uint32_t touchOnOffItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition, uint16_t colour )
{
	drawItem( (char *)s, y, condition ) ;
  if (value)
	{
		lcd_putcAttColour( TRIGHT-FW-5, y+3, '\202', 0, condition ? ~LcdForeground : LcdForeground, condition ? ~colour : colour ) ;
	}
	lcd_rectColour( TRIGHT-FW-6, y+TVOFF, 7, 7, condition ? ~LcdForeground : LcdForeground ) ;
//	lcd_hbar( TRIGHT-FW-6, y+TVOFF, 7, 7, 0 ) ;
	if ( condition )
	{
		value = checkIncDec( value, 0, 1, EditType ) ;
	}
	return value ;
}

uint32_t touchOffOnItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition, uint16_t colour )
{
	return 1- touchOnOffItem( 1- value, y, s, condition, colour ) ;
}

void putsChnColour( uint8_t x, uint8_t y, uint8_t idx1, uint8_t att )
{
	uint16_t fcolour = LcdForeground ;
	uint16_t bcolour = LcdBackground ;
	if ( att & INVERS )
	{
		fcolour = ~LcdForeground ;
		bcolour = ~LcdBackground ;
	}
	if ( idx1 == 0 )
	{
    lcd_putsnAttColour( x, y, XPSTR("--- "), 4, att, fcolour, bcolour ) ;
	}
	else
	{
		uint16_t x1 ;
		x1 = x + 4*FW-2 ;
		if ( idx1 < 10 )
		{
			x1 -= FWNUM ;			
		}
  	lcd_outdezNAtt( x1, y, idx1, 0, 2, fcolour, bcolour ) ;
    lcd_putsnAttColour( x, y, PSTR(STR_CH), 2, 0, fcolour, bcolour ) ;
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
	uint32_t left = flag ? 50 : TMID*TSCALE ;
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
 	putsTime( TRIGHT-5-2*FW, y+TVOFF, ptm->tmrVal, 0, 0 ) ;
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
  putsTmrMode(TRIGHT-5-4*FW,y+TVOFF,0, timer, 1 ) ;
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
	saveEditColours( attr, colour ) ;
	ptm->tmrModeB = edit_dr_switch( TRIGHT-5-4*FW, y+TVOFF, ptm->tmrModeB, 0, doedit, event ) ;
	restoreEditColours() ;
	y += TFH ;
	subN += 1 ;
	 
	drawItem( (char *)PSTR(STR_TIMER), y, (sub == subN ) ) ;
  attr = 0 ;
  if(sub==subN)
	{
		attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( ptm->tmrDir,1) ;
	}
	drawIdxText( y, (char *)PSTR(STR_COUNT_DOWN_UP), ptm->tmrDir, attr ) ;
//	saveEditColours( sub == subN, colour ) ;
//  lcd_putsAttIdx( TRIGHT-5-10*FW, y+TVOFF, PSTR(STR_COUNT_DOWN_UP), ptm->tmrDir, 0 ) ;
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
	saveEditColours( sub == subN, colour ) ;
	sw = edit_dr_switch( TRIGHT-5-4*FW, y+TVOFF, sw, 0, doedit, event ) ;
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
	lcd_putsAttIdxColour( TRIGHT-6*FW-5, y+TVOFF, XPSTR("\006  NoneMinute Cdown  Both"), sw, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
	
	lcd_putsAtt( 4*FW, 0, PSTR(STR_LIMITS), INVERS ) ;

	TlExitIcon = 1 ;
	mstate2.check_columns(event, rows-1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;

	putsChn( 14*FW, 0,index+1,0 ) ;
  lcd_outdez( 23*FW, 0, g_chans512[index]/2 + 1500 ) ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;

	uint16_t colour = dimBackColour() ;
	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}
	 
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
	drawNumber( TRIGHT-5, y, value, attr) ; //, attr ? ~colour : colour ) ;
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
	drawNumber( TRIGHT-5, y, value, attr) ; //, attr ? ~colour : colour ) ;
  if(attr)
	{
    ld->max +=  100;
    CHECK_INCDEC_H_MODELVAR( ld->max, -25,limit);
    ld->max -=  100;
  }
	y += TFH ;
	attr = sub==3 ? INVERS : 0 ;

	drawItem( XPSTR("Reverse"), y, (sub == 3 ) ) ;
	lcd_putsAttIdxColour( TRIGHT-5-3*FW, y+TVOFF, PSTR( STR_HYPH_INV)+2, ld->revert, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
  if(attr)
	{
		ld->revert = checkIncDec( ld->revert, 0, 1, EditType ) ;
	}
}

uint8_t evalHresOffset(int8_t sub)
{
	uint8_t max = (TLINES - 1) ;
  uint8_t t_pgOfs = s_pgOfs ;
	int8_t x = sub-t_pgOfs ;
	if(sub<1) t_pgOfs = 0 ;
  else if(x>max) t_pgOfs = sub-max ;
  else if(x<max-(TLINES - 1)) t_pgOfs = sub-max+(TLINES - 1) ;
	return (s_pgOfs = t_pgOfs) ;
}

void menuProcLimits(uint8_t event)
{
	static MState2 mstate2 ;
	lcd_putsAtt( 4*FW, 0, "Limits", INVERS ) ;
	TlExitIcon = 1 ;
	uint32_t rows = NUM_SKYCHNOUT+EXTRA_SKYCHANNELS + 1 ;
	event = mstate2.check_columns(event, rows - 1) ;

	uint32_t selected = 0 ;
	uint32_t newVpos ;
	uint16_t y = 0 ;
	uint32_t k = 0 ;
	uint16_t t_pgOfs ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;

	uint8_t  sub = mstate2.m_posVert ;

	t_pgOfs = evalHresOffset( sub ) ;

	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}
	else if (newSelection == -2)
	{
		selected = 1 ;
	}
	
	if ( sub < NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
	{
    lcd_outdez( 15*FW, 0, g_chans512[sub]/2 + 1500 ) ;
	}

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
    putsChnColour( THOFF, y+TVOFF, k+1, attr ) ;
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
		lcd_putsAttIdxColour( TRIGHT-5-3*FW, y+TVOFF, PSTR( STR_HYPH_INV)+2, ld->revert, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~LcdBackground : LcdBackground ) ;
	}
	if( k == NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
	{
  	//last line available - add the "copy trim menu" line
  	uint8_t attr = (sub==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS) ? INVERS : 0 ;
  	lcd_putsAttColour( 3*FW, y+TVOFF, PSTR(STR_COPY_TRIM), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~LcdBackground : LcdBackground ) ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}
}

void putsTrimMode( uint8_t x, uint8_t y, uint8_t phase, uint8_t idx, uint8_t att ) ;
#define ALPHA_NO_NAME		0x80
void alphaEditName( uint8_t x, uint8_t y, uint8_t *name, uint8_t len, uint16_t type, uint8_t *heading ) ;

void menuModeOne(uint8_t event)
{
  PhaseData *phase ;
	phase = (s_currIdx < MAX_MODES) ? &g_model.phaseData[s_currIdx] : &g_model.xphaseData ;
	static MState2 mstate2 ;
	TITLE(PSTR(STR_FL_MODE)) ;
	TlExitIcon = 1 ;
	
	uint32_t rows = 9 ;
	mstate2.check_columns(event, rows-1 ) ;
  lcd_putc( 12*FW, 0, '1'+s_currIdx ) ;

  uint8_t sub = mstate2.m_posVert ;
	uint16_t colour = dimBackColour() ;
	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;

	for (uint8_t i = 0 ; i < rows ; i += 1 )
	{
    uint8_t y = i * TFH + TTOP ;
    uint8_t attr = (sub==i) ? INVERS : 0 ;

		switch(i)
		{
      case 0 : // switch
			{	
				drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
				LcdBackground = attr ? ~colour : colour ;
				if ( attr )
				{
					LcdForeground = ~LcdForeground ;
				}
				phase->swtch = edit_dr_switch( TRIGHT-5*FW, y+TVOFF, phase->swtch, 0, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
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
				phase->swtch2 = edit_dr_switch( TRIGHT-5*FW, y+TVOFF, phase->swtch2, 0, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				LcdBackground = oldBcolour ;
				LcdForeground = oldFcolour ;
			}
			break ;
      case 2 : // trim
      case 3 : // trim
      case 4 : // trim
      case 5 : // trim
				switch(i)
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
					LcdForeground = ~LcdForeground ;
				}
        putsTrimMode( TRIGHT-2*FW, y+TVOFF, s_currIdx+1, i-2, 0 ) ;
				LcdBackground = oldBcolour ;
				LcdForeground = oldFcolour ;
        if (attr )
				{
          int16_t v = phase->trim[i-2] ;
          if (v < TRIM_EXTENDED_MAX)
					{
						v = TRIM_EXTENDED_MAX;
					}
					int16_t u = v ;
          v = checkIncDec16( v, TRIM_EXTENDED_MAX, TRIM_EXTENDED_MAX+MAX_MODES+1, EE_MODEL ) ;
            
					if (v != u)
					{
            if (v == TRIM_EXTENDED_MAX) v = 0 ;
//							if ( v == TRIM_EXTENDED_MAX+MAX_MODES+1 )
//							{
//								v = 2000 ;
//							}
  					phase->trim[i-2] = v ;
          }
        }
			break ;
			case 6 : // fadeIn
				drawItem( (char *)XPSTR("Fade In"), y, attr ) ;
				drawNumber( TRIGHT-5, y, phase->fadeIn * 5, attr|PREC1) ; //, attr ? ~colour : colour ) ;
  			if( attr )
				{
					CHECK_INCDEC_H_MODELVAR( phase->fadeIn, 0, 15 ) ;
				}
			break ;
      
			case 7 : // fadeOut
				drawItem( (char *)XPSTR("Fade Out"), y, attr ) ;
				drawNumber( TRIGHT-5, y, phase->fadeOut * 5, attr|PREC1) ; //, attr ? ~colour : colour ) ;
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
		}
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
	TlExitIcon = 1 ;
	uint16_t y = TTOP ;
#ifdef HAPTIC
	uint32_t rows = 19 ;
#else
	uint32_t rows = 17 ;
#endif
	
	mstate2.check_columns(event, rows-1 ) ;
	uint8_t sub = mstate2.m_posVert ;

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

#ifdef HAPTIC
	if ( sub < 16 )
#else
	if ( sub < 14 )
#endif
	{
		lcd_puts_P( 20*FW-4, 7*FH, XPSTR("->") ) ;
#ifdef HAPTIC
		lcd_putcAtt( 10*FW, 0, ( sub < 8 ) ? '1' : '2', BLINK ) ;
#else
		lcd_putcAtt( 10*FW, 0, ( sub < 7 ) ? '1' : '2', BLINK ) ;
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
		lcd_outdezNAtt( TRIGHT-5, y+TVOFF, secs, LEADING0, 2 ) ;
		lcd_putcAtt( TRIGHT-3*FW-2, y+TVOFF, ':', 0 ) ;
		lcd_outdezNAtt( TRIGHT-3*FW-2, y+TVOFF, (uint16_t)qr.rem, LEADING0, 2 ) ;
		lcd_putcAtt( TRIGHT-6*FW+3-2, y+TVOFF, ':', 0 ) ;
		lcd_outdezAtt( TRIGHT-6*FW+3-2, y+TVOFF, (uint16_t)qr.quot, 0 ) ;
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

void menuModelGeneral( uint8_t event )
{
	static MState2 mstate2;
	static uint8_t voiceCall = 0 ;			
  TITLE( PSTR(STR_General) ) ;
	TlExitIcon = 1 ;
	uint8_t subN = 0 ;
	uint8_t attr = 0 ;
	uint8_t type ;
	uint32_t rows ;
	uint32_t newVpos ;
	uint16_t colour = dimBackColour() ;

	rows = 21 ;
	
	mstate2.check_columns(event, rows-1) ;
  int8_t  sub    = mstate2.m_posVert;
  uint8_t t_pgOfs ;

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
	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
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

	uint16_t y = TTOP ;
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
			if ( event==EVT_KEY_LONG(BTN_RE) || handleSelectIcon() )
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
		alphaEditName( TRIGHT-8*FW-5, y+TVOFF, (uint8_t *)g_model.modelVname, sizeof(g_model.modelVname), type | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FileName") ) ;
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
		lcd_outdezAtt( TRIGHT - 4*FW, y+TVOFF, t, attr ) ;

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
		saveEditColours( attr, colour ) ;
		g_model.mlightSw = edit_dr_switch( TRIGHT-5-4*FW, y+TVOFF, g_model.mlightSw, 0, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
		if ( g_model.mlightSw == 0 )
		{
			putsDrSwitches( TRIGHT-5-9*FW, y+TVOFF, g_eeGeneral.lightSw, 0 ) ;
  	  lcd_putcAtt( TRIGHT-5-10*FW, y+TVOFF, '(', 0 ) ;
  	  lcd_putcAtt( TRIGHT-5-5*FW, y+TVOFF, ')', 0 ) ;
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
		drawItem( (char *)PSTR(STR_TRIM_INC), y, (sub==subN) ) ;
		lcd_putsAttIdxColour( TRIGHT-6*FW-5, y+TVOFF, PSTR(STR_TRIM_OPTIONS)+2, g_model.trimInc, 0, (sub==subN) ? ~LcdForeground : LcdForeground, (sub==subN) ? ~colour : colour ) ;
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
		g_model.trimSw = edit_dr_switch( TRIGHT-5-4*FW, y+TVOFF, g_model.trimSw, 0, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
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
		lcd_putsAttIdxColour( TRIGHT-5-8*FW, y+TVOFF, XPSTR("\010SubTrims   Trims"), g_model.instaTrimToTrims, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
		uint8_t oldValue = g_model.throttleReversed ;
		g_model.throttleReversed = touchOnOffItem( g_model.throttleReversed, y, PSTR(STR_THR_REVERSE), sub==subN, colour) ;
		lcd_putc( TRIGHT-TRMARGIN-3*FW, y+TVOFF, throttleReversed() ? '\201' : '\200' ) ;
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
  	lcd_putsAttIdx( TRIGHT-5-6*FW, y+TVOFF, XPSTR("\006   EndCentre"), g_model.throttleIdle, 0 ) ;
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
			if ( event==EVT_KEY_LONG(BTN_RE) || handleSelectIcon() )
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
		drawNumber( TRIGHT-5, y, g_model.sub_trim_limit, attr|PREC1) ; //, attr ? ~colour : colour ) ;
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
			lcd_putsnAtt(TRIGHT-5-8*FW+i*FW, y+TVOFF, XPSTR("ABCDEFG6")+i,1, attr ) ;
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
		  lcd_putc( 2*FW+i*(2*FW+3), y+TVOFF, 'A'+i ) ;
			lcd_putc( 3*FW+i*(2*FW+3), y+TVOFF, PSTR(HW_SWITCHARROW_STR)[states & 0x03] ) ;
		  states >>= 2 ;
		}
		lcd_putc( 2*FW+7*(2*FW+3), y+TVOFF, '6' ) ;
		lcd_putc( 3*FW+7*(2*FW+3), y+TVOFF, 'P' ) ;
		lcd_putc( 4*FW+7*(2*FW+3), y+TVOFF, ( (states >> 2) & 0x07) + '0' ) ;
		if( sub == subN )
		{
			lcd_rect( 2*FW-2, y-1+TVOFF, 17*FW+2+18+1+3, 9 ) ;
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
		lcd_putsAttIdxColour( TRIGHT-3*FW-5, y+TVOFF, XPSTR("\003---S1 S2 SL SR GV5GV6GV7"),g_model.anaVolume, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
	
// Beep Centre
	
//	if(t_pgOfs<=subN)
//	{
//		uint8_t subSub = g_posHorz ;
//		lcd_puts_Pleft( y+TVOFF, PSTR(STR_BEEP_CENTRE));
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
			if ( checkForMenuEncoderLong( event ) || handleSelectIcon() )
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
	 
#ifdef LUA
	if(t_pgOfs<=subN)
	{
		attr = sub==subN ? INVERS : 0 ;
		drawItem( (char *)"Script Type", y, attr ) ;
//		g_model.basic_lua = checkIndexed( y+TVOFF, XPSTR(FWx16"\001""\005Basic  LUA"), g_model.basic_lua, (sub==subN) ) ;
  	lcd_putsAttIdxColour( TRIGHT-TRMARGIN-5*FW, y+TVOFF, XPSTR("\005Basic  LUA"), g_model.basic_lua, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
		if ( attr )
		{
			CHECK_INCDEC_H_MODELVAR( g_model.basic_lua, 0, 1 ) ;
		}
		if((y+=TFH)>(TLINES-1)*TFH+TTOP) return ;
	}
	subN += 1 ;
#endif
}

void menuProcVoiceOne(uint8_t event)
{
	TITLE( PSTR( STR_Voice_Alarm ) ) ;
	TlExitIcon = 1 ;
	static MState2 mstate2;
	uint32_t rows = 13 ;
	uint32_t newVpos ;
	
	VoiceAlarmData *pvad ;
  if ( s_currIdx >= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
	{
		EditType = EE_GENERAL ;
		uint8_t z = s_currIdx - ( NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ) ;
		pvad = &g_eeGeneral.gvad[z] ;
		lcd_putc( 16*FW, 0, 'G' ) ;
		lcd_outdezAtt( 18*FW-1, 0, z+1, 0 ) ;
	}
	else
	{
		pvad = (s_currIdx >= NUM_VOICE_ALARMS) ? &g_model.vadx[s_currIdx - NUM_VOICE_ALARMS] : &g_model.vad[s_currIdx] ;
		lcd_outdezAtt( 17*FW, 0, s_currIdx+1, 0 ) ;
	}
	if ( pvad->fnameType )
	{
		rows += 1 ;
	}
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

	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
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
					putsChnRaw( TRIGHT-TRMARGIN-4*FW, y+TVOFF, pvad->source, 0 ) ;
					restoreEditColours() ;
					if ( attr )
					{
						uint8_t x = mapPots( pvad->source ) ;
						x = checkIncDec16( x,0,NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots-1+1,EditType);
						pvad->source = unmapPots( x ) ;
						if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) || handleSelectIcon() )
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
  	  			lcd_puts_Pleft( y+TVOFF, XPSTR("\007(\016)") ) ;
    				if ( ( (pvad->source > CHOUT_BASE+NUM_SKYCHNOUT) && ( pvad->source < EXTRA_POTS_START ) ) || ( pvad->source >= EXTRA_POTS_START + 8) )
 						{
							putsTelemetryChannel( TRIGHT-TRMARGIN-5*FW, y+TVOFF, pvad->source-CHOUT_BASE-NUM_SKYCHNOUT-1, value, 0, TELEM_NOTIME_UNIT | TELEM_UNIT ) ;
						}
						else
						{
							lcd_outdezAtt( TRIGHT-TRMARGIN-5*FW, y+TVOFF, value, 0 ) ;
						}
					}
				break ;

				case 1 :	// func;
					drawItem( (char *)XPSTR("Function"), y, attr ) ;
					saveEditColours( attr, colour ) ;
					lcd_putsAttIdx( TRIGHT-TRMARGIN-7*FW, y+TVOFF, XPSTR("\007-------v>val  v<val  |v|>val|v|<valv\140=val v=val  v & val|d|>valv%val=0d>=val "), pvad->func, 0 ) ;	// v1>v2  v1<v2  
					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->func, 10 ) ;
					}	
				break ;

				case 2 :
				{	
					drawItem( (char *)XPSTR("Value"), y, attr ) ;
					saveEditColours( attr, colour ) ;
    			if ( ( (pvad->source > CHOUT_BASE+NUM_SKYCHNOUT) && ( pvad->source < EXTRA_POTS_START ) ) || ( pvad->source >= EXTRA_POTS_START + 8) )
		 			{
						putsTelemetryChannel( TRIGHT-TRMARGIN-5*FW, y+TVOFF, pvad->source-CHOUT_BASE-NUM_SKYCHNOUT-1, pvad->offset, 0, TELEM_NOTIME_UNIT | TELEM_UNIT | TELEM_CONSTANT ) ;
					}
					else
					{
						lcd_outdezAtt( TRIGHT-TRMARGIN, y+TVOFF, pvad->offset, 0 ) ;
					}
					restoreEditColours() ;
					if ( attr )
					{
						pvad->offset = checkIncDec16( pvad->offset, -32000, 32000, EE_MODEL ) ;
					}
				}
				break ;
			
				case 3 :	 // swtch ;
					drawItem( (char *)XPSTR("Switch"), y, attr ) ;
					saveEditColours( attr, colour ) ;
   	  		putsDrSwitches(TRIGHT-TRMARGIN-4*FW, y+TVOFF, pvad->swtch, 0 ) ;
					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_MODELSWITCH( pvad->swtch, -MaxSwitchIndex, MaxSwitchIndex+1 ) ;
    			}
				break ;

				case 4 :	 // rate ;
					drawItem( (char *)XPSTR("Trigger"), y, attr ) ;
					saveEditColours( attr, colour ) ;
					displayVoiceRate( TRIGHT-TRMARGIN-4*FW, y+TVOFF, pvad->rate, 0 ) ;
					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->rate, 33 ) ;
					}	
				break ;

				case 5 :	 // haptic ;
					drawItem( (char *)PSTR( STR_HAPTIC ), y, attr ) ;
					saveEditColours( attr, colour ) ;
					lcd_putsAttIdx( TRIGHT-TRMARGIN-7*FW, y+TVOFF, XPSTR("\007-------Haptic1Haptic2Haptic3"), pvad->haptic, 0 ) ;
					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->haptic, 3 ) ;
					}
				break ;

				case V1P1 :	 // delay
					drawItem( (char *)XPSTR("On Delay"), y, attr ) ;
					saveEditColours( attr, colour ) ;
  				lcd_outdezAtt(TRIGHT-TRMARGIN,y+TVOFF, pvad->delay, PREC1 ) ;
					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->delay, 50 ) ;
					}
				break ;
				
				case V1P1+1 :	 // vsource:2
					drawItem( (char *)XPSTR("Play Source"), y, attr) ;
					saveEditColours( attr, colour ) ;
					lcd_putsAttIdx( TRIGHT-TRMARGIN-6*FW, y+TVOFF, XPSTR("\006    NoBefore After"), pvad->vsource, 0 ) ;
					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->vsource, 2 ) ;
					}
				break ;

				case V1P1+2 :
					drawItem( (char *)XPSTR("On no Telemetry"), y, attr ) ;
					saveEditColours( attr, colour ) ;
					lcd_putsAttIdx( TRIGHT-TRMARGIN-4*FW, y+TVOFF, XPSTR("\004PlayMute"), pvad->mute, 0 ) ;
					restoreEditColours() ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->mute, 1 ) ;
					}
				break ;

				case V1P1+3 :	 // fnameType:3 ;
				{	
					drawItem( (char *)XPSTR("FileType"), y, attr ) ;
					saveEditColours( attr, colour ) ;
					lcd_putsAttIdx( TRIGHT-TRMARGIN-7*FW, y+TVOFF, XPSTR("\007-------   NameGV/SC/# EffectSysName"),pvad->fnameType,0 ) ;
					restoreEditColours() ;
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
							if ( event==EVT_KEY_LONG(BTN_RE) || handleSelectIcon() )
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
								lcd_putsAtt( TRIGHT-TRMARGIN-5*FW, y+TVOFF, XPSTR("SC"), attr ) ;
								lcd_putcAtt( TRIGHT-TRMARGIN-3*FW, y+TVOFF, pvad->file.vfile - 507 + '0', attr ) ;
							}
							else
							{
								dispGvar( TRIGHT-TRMARGIN-5*FW, y+TVOFF, pvad->file.vfile - 500, attr ) ;
							}
  					}
  					else
						{
	      	    lcd_outdezAtt( TRIGHT-TRMARGIN-3*FW, y+TVOFF, pvad->file.vfile, attr ) ;
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
						lcd_putsAttIdx(TRIGHT-TRMARGIN-8*FW, y+TVOFF, PSTR(STR_SOUNDS), pvad->file.vfile, attr ) ;
		  	  	if(attr)
						{
							CHECK_INCDEC_H_MODELVAR( pvad->file.vfile, 0, 16 ) ;
						}
					}
				break ;
				
				case V1P1+5 :	 // Blank ;
					drawItem( (char *)XPSTR("Delete"), y, attr ) ;
					saveEditColours( attr, colour ) ;
					lcd_putsAtt( TRIGHT-TRMARGIN-9*FW, y+TVOFF, XPSTR("MENU LONG"), 0 ) ;
					restoreEditColours() ;
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
						lcd_putcAttColour( TRIGHT-FW-5, y+3, '\202', 0, LcdForeground, colour ) ;
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
	TlExitIcon = 1 ;
	static MState2 mstate2 ;
	int8_t sub ;
	uint8_t t_pgOfs ;
	uint16_t y ;
	uint32_t selected = 0 ;
	
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

	if ( !PopupData.PopupActive )
	{
		uint32_t newVpos ;
		
		mstate2.check_columns(event, rows - 1 ) ;
		
		lcd_hline( 0, TTOP, TRIGHT ) ;
  	
		sub = mstate2.m_posVert ;

		t_pgOfs = evalHresOffset( sub ) ;

		int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
		if ( newSelection >= 0 )
		{
//			if ( sub == newSelection )
//			{
//				if ( event == 0 )
//				{
//					event = EVT_KEY_BREAK(BTN_RE) ;
//				}
//			}
			sub = mstate2.m_posVert = newSelection ;
		}
		else if (newSelection == -2)
		{
			selected = 1 ;
		}
		 
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
				lcd_puts_P( THOFF, y, XPSTR("GVA") ) ;
				lcd_putc( 3+3*FW, y, '1' + k ) ;
				pvad = &g_eeGeneral.gvad[k] ;
			}
			else
			{
				lcd_puts_P( THOFF, y, XPSTR("VA") ) ;
  		  lcd_outdezAtt( (k<9) ? 3+FW*3-1 : 3+FW*4-2, y, k+1, 0 ) ;
				pvad = (k >= NUM_VOICE_ALARMS) ? &g_model.vadx[k - NUM_VOICE_ALARMS] : &g_model.vad[k] ;
			}
			putsChnRaw( 5*FW, y, pvad->source, 0 ) ;
			putsDrSwitches( 9*FW, y, pvad->swtch, 0 ) ;
			displayVoiceRate( 13*FW, y, pvad->rate, 0 ) ;
    	switch ( pvad->fnameType )
			{
				case 1 :
					lcd_putc( 19*FW, y, 'N' ) ;
				break ;
				case 2 :
					lcd_putc( 19*FW, y, '#' ) ;
				break ;
				case 3 :
					lcd_putc( 19*FW, y, 'A' ) ;
				break ;
				case 4 :
					lcd_putc( 19*FW, y, 'S' ) ;
				break ;
			}
		
			if (pvad->haptic)
			{
				lcd_putc( 20*FW, y, 'H' ) ;
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
  		  lcd_putsAtt( 0, y, XPSTR(GvaString), 0 ) ;
			}
  		else
			{
  		  lcd_puts_Pleft( y ,XPSTR("Flush Switch") ) ;
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

void menuProcTemplates(uint8_t event)  //Issue 73
{
	TITLE( PSTR(STR_TEMPLATES) ) ;
	TlExitIcon = 1 ;
	
	static MState2 mstate2 ;
	event = mstate2.check_columns( event, NUM_TEMPLATES-1 ) ;
	EditType = EE_MODEL ;

  uint8_t y = TTOP ;
  uint8_t k = 0 ;
  int8_t sub = mstate2.m_posVert ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;

	int32_t newSelection = checkTouchSelect( NUM_TEMPLATES, 0) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}

	if ( checkForMenuEncoderLong( event ) || handleSelectIcon() )
	{
    //apply mixes or delete
    s_noHi = NO_HI_LEN ;
    applyTemplate(sub) ;
    audioDefevent(AU_WARNING2) ;
  }

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
    lcd_outdezNAtt( 3*FW+THOFF, y+TVOFF, k+1, 0 + LEADING0,2) ;
    lcd_putsAtt( 6*FW, y+TVOFF, PSTR(n_Templates[k]), 0 ) ;
		restoreEditColours() ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
    y += TFH ;
  }
}

extern PhaseData *getPhaseAddress( uint32_t phase ) ;

void menuModes(uint8_t event)
{
	uint32_t i ;
  uint8_t attr ;
	
	TITLE(PSTR(STR_MODES)) ;
	static MState2 mstate2 ;
  
	TlExitIcon = 1 ;
	uint32_t rows = 7 ;
	event = mstate2.check_columns( event, rows-1 ) ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
	uint16_t colour = LcdBackground ;
	
	uint8_t sub = mstate2.m_posVert ;

	int32_t newSelection = checkTouchSelect( rows+1, 0) ;
	if ( newSelection >= 1 )
	{
		sub = mstate2.m_posVert = newSelection-1 ;
	}

	if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) || handleSelectIcon() )
	{
		s_currIdx = sub ;
		killEvents(event);
		pushMenu(menuModeOne) ;
  }
    
  lcd_putc( THOFF, TTOP+TVOFF, 'F' ) ;
  lcd_putc( THOFF+FW, TTOP+TVOFF, '0' ) ;
	lcd_puts_P( TRIGHT-TRMARGIN-7*FW, TTOP+TVOFF, XPSTR("RETA") ) ;
	lcd_hline( 0, TTOP+TFH, TRIGHT ) ;
	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;

  for ( i=0 ; i<MAX_MODES+1 ; i += 1 )
	{
    uint16_t y = TTOP + TFH + i*TFH ;
  	PhaseData *p ;
		p = getPhaseAddress( i ) ;
    attr = (i == sub) ? INVERS : 0 ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdForeground = ~LcdForeground ;
		}
    lcd_putcAttColour( THOFF, y+TVOFF, 'F', 0, LcdForeground, attr ? ~colour : colour ) ;
    lcd_putcAttColour( THOFF+FW, y+TVOFF, '1'+i, 0, LcdForeground, attr ? ~colour : colour ) ;
		lcd_putsnAttColour( THOFF+3*FW, y+TVOFF, p->name, 6, 0, LcdForeground, attr ? ~LcdBackground : LcdBackground ) ;
		LcdForeground = oldFcolour ;
		putsDrSwitchesColour( THOFF+10*FW, y+TVOFF, p->swtch, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
		putsDrSwitchesColour( THOFF+15*FW, y+TVOFF, p->swtch2, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
		LcdBackground = attr ? ~colour : colour ;
		if ( attr )
		{
			LcdForeground = ~LcdForeground ;
		}
    for ( uint8_t t = 0 ; t < NUM_STICKS ; t += 1 )
		{
			putsTrimMode( TRIGHT-3-7*FW+t*FW, y+TVOFF, i+1, t, 0 ) ;
		}
		LcdBackground = oldBcolour ;
		LcdForeground = attr ? ~oldFcolour : oldFcolour ;
		if ( p->fadeIn || p->fadeOut )
		{
	    lcd_putcAttColour( TRIGHT-3-FW, y+TVOFF, '*', 0, LcdForeground, attr ? ~colour : colour ) ;
		}
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}	 
}

void menuProcDiagVers(uint8_t event)
{
	uint16_t x ;
	uint16_t y ;
	uint16_t i ;
	char c ;
	TITLE(PSTR(STR_Version)) ;
	TlExitIcon = 1 ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, 1-1) ;
  
	char *p ;
	p = (char *)Stamps ;
	y = TTOP + TFH ;
	x = 8*FW ;

	for ( i = 0 ; i < 4 ; i += 1 )
	{
		for(;;)
		{
			c = *p++ ;
			if ( (c == '\0') || (c == '\037') )
			{
				y += TFH ;
				x = 8*FW ;
				break ;
			}
			lcd_putc( x, y, c ) ;
			x += FW ;
		}
	}
//	lcd_puts_Pleft( y,Stamps ) ;
}

void menuControls(uint8_t event)
{
	TITLE( PSTR(STR_Controls) ) ;
	static MState2 mstate2 ;
	TlExitIcon = 1 ;
	uint32_t rows = 8 ;
	event = mstate2.check_columns( event, rows-1 ) ;
	uint16_t colour = dimBackColour() ;
		
	uint8_t subN = 0 ;
	uint8_t sub = mstate2.m_posVert ;
	uint16_t y = TTOP ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

	int32_t newSelection = checkTouchSelect( rows+1, 0, 0 ) ;
	if ( newSelection >= 0 )
	{
		if ( newSelection > 3 )
		{
			newSelection -= 1 ;
		}
		sub = mstate2.m_posVert = newSelection ;
	}

	uint8_t attr = sub==subN ? INVERS : 0 ;
	drawItem( (char *)PSTR(STR_CHAN_ORDER), y, (sub == subN ) ) ;
	uint8_t bch = bchout_ar[g_eeGeneral.templateSetup] ;
	saveEditColours( attr, colour ) ;
  for ( uint8_t i = 4 ; i > 0 ; i -= 1 )
	{
		uint8_t letter ;
		letter = *(PSTR(STR_SP_RETA) +(bch & 3) + 1 ) ;
  	lcd_putcAtt( TRIGHT-TRMARGIN-5*FW+i*FW, y+TVOFF, letter, 0 ) ;
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
  lcd_putsAttIdx( TRIGHT-TRMARGIN-3*FW, y+TVOFF, XPSTR("\003OFFON Vtg"), ct, 0 ) ;
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
				g_eeGeneral.stickScroll = 0 ;
				g_eeGeneral.stickMode = mode ;							
			}
		}
	}
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
	}
	saveEditColours( attr, LcdBackground ) ;
	lcd_puts_P( THOFF, y+TVOFF, PSTR(STR_MODE) ) ;
	lcd_hline( 0, y+TFH, TRIGHT ) ;
	for ( uint32_t i = 0 ; i < 4 ; i += 1 )
	{
 		lcd_HiResimg( (6+4*i)*FW*2, y*2+TVOFF*2-1, sticksHiRes, i, 0, LcdForeground ) ;
	}
	restoreEditColours() ;
 	y += TFH ;
    
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
	}
	saveEditColours( attr, LcdBackground ) ;
  lcd_putcAtt( 3*FW, y+TVOFF, '1'+g_eeGeneral.stickMode, 0 ) ;
  for(uint32_t i=0; i<4; i++)
	{
		putsChnRaw( (6+4*i)*FW, y+TVOFF, modeFixValue( i ), 0 ) ;//sub==3?INVERS:0);
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
    lcd_putsAttIdx( THOFF, y+TVOFF, PSTR(STR_STICK_NAMES), i, 0 ) ;
    lcd_puts_P( THOFF+4*FW, y+TVOFF, (char *)"Stick Name" ) ;
		uint16_t oldBcolour = LcdBackground ;
		LcdBackground = attr ? ~colour : colour ;
		lcdDrawSolidFilledRectDMA( TMID*TSCALE, y*TSCALE+2, (TRIGHT-TMID)*TSCALE, TFH*TSCALE-2, LcdBackground ) ;
		alphaEditName( TRIGHT-TRMARGIN-4*FW, y+TVOFF, &g_eeGeneral.customStickNames[i*4], 4, attr|ALPHA_NO_NAME, (uint8_t *)&PSTR(STR_STICK_NAMES)[i*5+1] ) ;
		LcdBackground = oldBcolour ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	 	y += TFH ;
		subN += 1 ;
	}
}

#define DATE_OFF_0		(8*FW)
#define DATE_COUNT_ITEMS	8

void dispMonth( uint8_t x, uint8_t y, uint32_t month, uint8_t attr) ;
void disp_datetime( uint8_t y ) ;

t_time EntryTime ;

void menuProcDate(uint8_t event)
{
	TITLE(PSTR(STR_DateTime));
	static MState2 mstate2;
	TlExitIcon = 1 ;
	uint16_t colour = dimBackColour() ;
	uint32_t rows = DATE_COUNT_ITEMS ;
	
	mstate2.check_columns(event, DATE_COUNT_ITEMS-1) ;
//?	event = mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1, rows-1) ;

  int8_t  sub = mstate2.m_posVert ;

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

	disp_datetime( TTOP ) ;
	lcd_hline( 0, TTOP, TRIGHT ) ;

	for (uint8_t subN = 0 ; subN<rows ; subN += 1)
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
					lcd_putsAttColour( TRIGHT-TRMARGIN-3*FW, y+TVOFF, (char *)"Set", 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
					if ( handleSelectIcon() )
					{
						rtcSetTime( &EntryTime ) ;
					}
				}
			break ;
		}
extern uint16_t VbattRtc ;
		  lcd_puts_P( TRIGHT+FW, TTOP+7*TFH, XPSTR("RTC V") );
			lcd_outdezAtt( TRIGHT+5*FW, TTOP+8*TFH, VbattRtc*600/2048, PREC2 ) ;
	}
}

void menuGeneral( uint8_t event )
{
	TITLE( PSTR(STR_General) ) ;
	TlExitIcon = 1 ;
	
	static MState2 mstate2;
//			IlinesCount = 8+1+1+1 ;
	uint32_t rows = 11 ;
	mstate2.check_columns(event, rows-1) ;

//	uint16_t attr ;
	uint32_t newVpos ;
//	uint8_t subN = 0 ;
  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
//	uint8_t blink = InverseBlink ;
	uint32_t k = 0 ;
	uint16_t t_pgOfs ;
	uint16_t colour = dimBackColour() ;

	lcd_hline( 0, TTOP, TRIGHT ) ;
	t_pgOfs = evalHresOffset( sub ) ;

	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
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
				lcd_putsAttIdxColour( TRIGHT-11*FW, y+TVOFF, XPSTR("\012   ENGLISH  FRANCAIS   DEUTSCH NORWEGIAN   SWEDISH   ITALIAN    POLISHVIETNAMESE   SPANISH"),g_eeGeneral.language, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
  			uint8_t b = 1-g_eeGeneral.disablePotScroll ;
				b |= g_eeGeneral.stickScroll << 1 ;
				drawItem( (char *)PSTR(STR_SCROLLING), y, attr ) ;
  			lcd_putsAttIdxColour( TRIGHT-5*FW, y+TVOFF, XPSTR("\005 NONE  POTSTICK BOTH"),b, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
  			if(attr) CHECK_INCDEC_H_GENVAR_0( b, 3 ) ;
				g_eeGeneral.stickScroll = b >> 1 ;
				g_eeGeneral.disablePotScroll = 1 - ( b & 1 ) ;
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
				lcd_putsAttIdxColour( TRIGHT-11*FW, y+TVOFF, XPSTR("\012DD mm.mmmmDD.dddddd "), g_eeGeneral.gpsFormat,  0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
				putsDrSwitchesColour( TRIGHT-5*FW, y+TVOFF, g_eeGeneral.screenShotSw, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
	TlExitIcon = 1 ;
	static MState2 mstate2;
	uint32_t rows = 6 ;
	event = mstate2.check_columns(event, rows-1) ;

  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	uint16_t colour = dimBackColour() ;
	uint8_t attr ;
	uint8_t subN = 0 ;

	int32_t newSelection = checkTouchSelect( rows, 0 ) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}

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
	TlExitIcon = 1 ;

	static MState2 mstate2;
	uint32_t rows = 9 ;
	if ( g_eeGeneral.welcomeType == 2 )
	{
		rows += 1 ;
	}
	mstate2.check_columns(event, rows-1) ;
	 
	uint32_t newVpos ;
  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	uint32_t k = 0 ;
	uint16_t t_pgOfs ;
	uint16_t colour = dimBackColour() ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
	t_pgOfs = evalHresOffset( sub ) ;
	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}

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

	for( uint32_t i = 0 ; i<TLINES ; i += 1 )
	{
    y = i * TFH + TTOP ;
    k = i + t_pgOfs ;
    uint8_t attr = (sub==k) ? INVERS : 0 ;
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
				lcd_putsAttIdxColour( TRIGHT-7*FW, y+TVOFF, PSTR(STR_BEEP_MODES), b, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
					if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
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
  			g_eeGeneral.minuteBeep = touchOnOffItem( g_eeGeneral.minuteBeep, y, PSTR(STR_BEEP_COUNTDOWN), attr, colour ) ;
      break ;
			case 8 :
				drawItem( (char *)XPSTR( "Welcome Type"), y, attr ? ~colour : colour ) ;
				lcd_putsAttIdxColour( TRIGHT-7*FW, y+TVOFF, XPSTR("\006System  NoneCustom"),g_eeGeneral.welcomeType, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
						if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
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
	TlExitIcon = 1 ;

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
	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}
	
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
	drawNumber( TRIGHT-5, y, 100 - g_eeGeneral.bright_white, attr) ; //, attr ? ~colour : colour ) ;
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
	putsDrSwitchesColour( TRIGHT-5*FW, y+TVOFF, g_eeGeneral.lightSw, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
  		lcd_putsAttColour( TRIGHT-5-3*FW, y+TVOFF, PSTR(STR_OFF), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
		if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
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
		if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
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

void menuHeli( uint8_t event )
{
	TITLE( PSTR(STR_HELI_SETUP) ) ;
	TlExitIcon = 1 ;

	static MState2 mstate2;
	uint32_t rows = 6 ;
	mstate2.check_columns(event, rows-1) ;

  uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	uint8_t subN = 0 ;
	uint16_t colour = dimBackColour() ;
	uint8_t attr ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}
	
	y = TTOP ;
	 
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Swash Type"), y, attr ) ;
	lcd_putsAttIdxColour( TRIGHT-5*FW, y+TVOFF, PSTR(SWASH_TYPE_STR)+2, g_model.swashType, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
  putsChnRaw( TRIGHT-5*FW, y+TVOFF, g_model.swashCollectiveSource, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
	lcd_putsAttIdxColour( TRIGHT-5-3*FW, y+TVOFF, PSTR( STR_HYPH_INV)+2, g_model.swashInvertELE, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
  if(attr)
	{
		g_model.swashInvertELE = checkIncDec( g_model.swashInvertELE, 0, 1, EditType ) ;
	}
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("AIL Direction"), y, attr ) ;
	lcd_putsAttIdxColour( TRIGHT-5-3*FW, y+TVOFF, PSTR( STR_HYPH_INV)+2, g_model.swashInvertAIL, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
  if(attr)
	{
		g_model.swashInvertAIL = checkIncDec( g_model.swashInvertAIL, 0, 1, EditType ) ;
	}
  y += TFH ;
	subN += 1 ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("COL Direction"), y, attr ) ;
	lcd_putsAttIdxColour( TRIGHT-5-3*FW, y+TVOFF, PSTR( STR_HYPH_INV)+2, g_model.swashInvertCOL, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
  if(attr)
	{
		g_model.swashInvertCOL	 = checkIncDec( g_model.swashInvertCOL, 0, 1, EditType ) ;
	}
}

uint16_t scalerDecimal( uint8_t y, uint16_t val, uint8_t attr )
{
  lcd_outdezAtt( TRIGHT-TRMARGIN, y+TVOFF, val+1, 0 ) ;
	if (attr) val = checkIncDec16( val, 0, 2047, EE_MODEL);
	return val ;
}

static uint8_t s_scalerSource ;

void menuScaleOne(uint8_t event)
{
  TITLE( "SC  =") ;
	uint8_t index = s_currIdx ;
  lcd_putcAtt( 6*FW, 0, index+'1', INVERS ) ;
	TlExitIcon = 1 ;

	static MState2 mstate2 ;
	uint32_t rows = 13 ;
	mstate2.check_columns(event, rows-1 ) ;
	uint32_t newVpos ;

	lcd_hline( 0, TTOP, TRIGHT ) ;
	
  uint8_t sub = mstate2.m_posVert;
	uint16_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;
	
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
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
				putsChnRaw( TRIGHT-TRMARGIN-4*FW, y+TVOFF, x, 0 ) ;
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
					if ( checkForMenuEncoderBreak( event ) || handleEditIcon() )
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
				lcd_putsAttIdx( TRIGHT-TRMARGIN-5*FW, y+TVOFF, XPSTR("\005FirstLast "), pscaler->offsetLast, 0 ) ;
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
  			lcd_putsAttIdx( TRIGHT-5-8*FW, y+TVOFF, XPSTR("\010--------Add     SubtractMultiplyDivide  Mod     Min     "), pscaler->exFunction, 0 ) ;
				restoreEditColours() ;
			break ;
			case 9 :	// unit
				drawItem( (char *)XPSTR("Unit"), y, attr ) ;
				saveEditColours( attr, colour ) ;
				lcd_putsAttIdx( TRIGHT-TRMARGIN-5*FW, y+TVOFF, XPSTR(UnitsString), pscaler->unit, 0 ) ;
				restoreEditColours() ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->unit, 10 ) ;
			break ;
      case 10 :	// sign
				drawItem( (char *)XPSTR("Sign"), y, attr ) ;
				saveEditColours( attr, colour ) ;
  			lcd_putcAtt( TRIGHT-TRMARGIN-FW, y+TVOFF, pscaler->neg ? '-' : '+', 0 ) ;
				restoreEditColours() ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->neg, 1 ) ;
			break ;
      case 11 :	// precision
				drawItem( (char *)XPSTR("Decimals"), y, attr ) ;
				saveEditColours( attr, colour ) ;
				lcd_outdezAtt( TRIGHT-TRMARGIN, y+TVOFF, pscaler->precision, 0 ) ;
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
				lcd_putsAttIdx( TRIGHT-TRMARGIN-5*FW, y+TVOFF, XPSTR(DestString), epscaler->dest, 0 ) ;
				restoreEditColours() ;
			break ;
		}
	}

}


void menuProcScalers(uint8_t event)
{
	TITLE(XPSTR("Scalers"));
	TlExitIcon = 1 ;
	EditType = EE_MODEL ;
	uint32_t rows = NUM_SCALERS ;
	static MState2 mstate2;
 	event = mstate2.check_columns(event, rows-1 ) ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;

	uint8_t sub = mstate2.m_posVert ;
	uint16_t y = TTOP ;
  uint32_t k ;

	int32_t newSelection = checkTouchSelect( rows, 0 ) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}

	if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
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
    y = i*TFH + TTOP ;
    k = i ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}
		y += TVOFF ;
		lcd_puts_P( THOFF, y, XPSTR("SC") ) ;
  	lcd_putc( 2*FW+THOFF, y, k+'1' ) ;
  	lcd_putc( 9*FW, y, '+' ) ;
  	lcd_putc( 19*FW, y, '*' ) ;
  	lcd_putc( 25*FW, y, '/' ) ;
		putsChnRaw( 4*FW+THOFF, y, g_model.Scalers[k].source, 0 ) ;
		lcd_outdezAtt( 15*FW+THOFF, y, g_model.Scalers[k].offset, 0 ) ;
		t = g_model.Scalers[k].mult + ( g_model.Scalers[k].multx << 8 ) ;
		lcd_outdezAtt( 23*FW-1+THOFF, y, t+1, 0 ) ;
		t = g_model.Scalers[k].div + ( g_model.Scalers[k].divx << 8 ) ;
		lcd_outdezAtt( 29*FW+THOFF, y, t+1, 0 ) ;
		
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
	}
}

void menuBindOptions(uint8_t event)
{
//	uint8_t b ;
	struct t_module *pModule = &g_model.Module[s_currIdx] ;
	TITLE(XPSTR("Bind Options"));
	TlExitIcon = 1 ;
	static MState2 mstate2;
	uint32_t rows = 3 ;
	event = mstate2.check_columns( event, rows-1 ) ;
	
	uint32_t sub = mstate2.m_posVert ;
	
	int32_t newSelection = checkTouchSelect( rows, 0, 1 ) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}

	if ( sub == 0 )
	{
		if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
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
				lcd_puts_P( THOFF, y+TVOFF, PSTR(STR_BIND) ) ;
			break ;
			case 1 :
//				pModule->highChannels = checkIndexed( TTOP+1*TFH+TVOFF, XPSTR("\0""\001""\012Chans 1-8 Chans 9-16"), pModule->highChannels, (sub==1) ) ;
  			lcd_putsAttIdx( THOFF, y+TVOFF, XPSTR("\012Chans 1-8 Chans 9-16"), pModule->highChannels, 0 ) ;
				if ( attr )
				{
					CHECK_INCDEC_H_MODELVAR( pModule->highChannels, 0, 1 ) ;
				}
			break ;
			case 2 :
//				pModule->disableTelemetry = checkIndexed( TTOP+2*TFH+TVOFF, XPSTR("\0""\001""\014Telemetry   No Telemetry"), pModule->disableTelemetry, (sub==2) ) ;
		  	lcd_putsAttIdx( THOFF, y+TVOFF, XPSTR("\014Telemetry   No Telemetry"), pModule->disableTelemetry, 0 ) ;
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
	TlExitIcon = 1 ;
	uint32_t rows = 1+MAX_CURVE5+MAX_CURVE9-1+1+1+1 ;
	uint32_t t_pgOfs ;

	mstate2.check_columns(event, rows - 1 ) ;

  uint8_t sub = mstate2.m_posVert ;
	t_pgOfs = evalHresOffset( sub ) ;

	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}

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

	if ( checkForMenuEncoderBreak( event ) || handleSelectIcon() )
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
    lcd_putsAtt( THOFF+FW*0, y+TVOFF, PSTR(STR_CV), 0 ) ;
    lcd_outdezAtt( (k<9) ? THOFF+FW*3-1 : THOFF+FW*4-2, y+TVOFF, k+1, 0 ) ;
		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y + TFH, TMID-2*FH ) ;
  }
	s_curveChan = sub ;
	drawCurve( 100 ) ;
}


void displayModuleName( uint16_t x, uint16_t y, uint8_t index, uint8_t attr ) ;
void menuProcTrainProtocol(uint8_t event) ;
extern uint8_t EditingModule ;
void editOneProtocol( uint8_t event ) ;
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
	TlExitIcon = 1 ;
	
	event = mstate2.check_columns( event, dataItems-1 ) ;

	int8_t  sub    = mstate2.m_posVert ;
	
	int32_t newSelection = checkTouchSelect( dataItems, 0, 1 ) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}
	else if (newSelection == -2)
	{
		selected = 1 ;
	}

	if ( handleSelectIcon() || selected )
	{
		selected = 1 ;
		if ( event == 0 )
		{
			event = EVT_KEY_BREAK(BTN_RE) ;
		}
	}
	 
	uint8_t y = TTOP ;
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
	lcd_putsAtt( THOFF, y+TVOFF, PSTR(STR_Trainer), 0 ) ;
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
			lcd_puts_P( THOFF, y+TVOFF, PSTR(STR_RANGE) ) ;
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
//				lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
//				lcd_puts_Pleft( y, "\014Internal") ;
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
//				lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
//				lcd_puts_Pleft( y, "\014External") ;
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
	TlExitIcon = 1 ;
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
	lcd_putsAttIdxColour( TRIGHT-TRMARGIN-4*FW, y+TVOFF, XPSTR("\004NameList"), b, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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

	lcd_putsAttColour( TRIGHT-TRMARGIN-MUSIC_NAME_LENGTH*FW, y+TVOFF, (char *)g_eeGeneral.musicVoiceFileName, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
	if (attr)
	{
//		lcdHiresRect( (TRIGHT-TRMARGIN-MUSIC_NAME_LENGTH*FW)*TSCALE-1, (y+TVOFF)*TSCALE-1, MUSIC_NAME_LENGTH*FW*TSCALE+2, 17, LCD_BLACK ) ;
		if ( handleSelectIcon() )
		{
			VoiceFileType = VOICE_FILE_TYPE_MUSIC ;
     	pushMenu( menuProcSelectVoiceFile ) ;
		}
	}
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
		lcd_putsAttColour( TRIGHT-TRMARGIN-5*FW, y+TVOFF, (char *)XPSTR("Start"), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
	}
	else
	{
		lcd_putsAttColour( TRIGHT-TRMARGIN-4*FW, y+TVOFF, (char *)XPSTR("Stop"), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
		y += TFH ;
		subN += 1 ;

		attr = 0 ;
  	if(sub==subN)
		{
		  attr = INVERS ;
		}
		drawItem( (char *)XPSTR(""), y, ( attr ) ) ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;	// Blanks line
		lcd_putsAttColour( TRIGHT-TRMARGIN-6*FW, y+TVOFF, (char *)(MusicPlaying == MUSIC_PAUSED ? XPSTR("Resume") : XPSTR(" Pause")), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
		if ( g_eeGeneral.musicType )
		{
			if ( MusicPlaying == MUSIC_PLAYING )
			{
				lcd_puts_P( TRIGHT+FW, y+TVOFF, XPSTR("Next")) ;
				lcd_puts_P( TRIGHT+FW, y-TFH+TVOFF, XPSTR("Prev")) ;
				if ( checkTouchposition( (TRIGHT+FW)*TSCALE, (y-TFH)*TSCALE, LCD_W-1, y*TSCALE ) )
				{
					MusicPrevNext = MUSIC_NP_PREV ;
				}
				if ( checkTouchposition( (TRIGHT+FW)*TSCALE, y*TSCALE, LCD_W-1, (y+TFH)*TSCALE ) )
				{
					MusicPrevNext = MUSIC_NP_NEXT ;
				}
			}
		}
		lcd_puts_P( FW, 14*FH, CurrentPlayName ) ;
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


extern uint8_t s_expoChan ;
extern uint8_t SingleExpoChan ;
void menuProcTrainer(uint8_t event) ;

void menuProcTrainProtocol(uint8_t event)
{
	TITLE(PSTR(STR_Trainer));
	TlExitIcon = 1 ;
	
	static MState2 mstate2;
	uint32_t rows = 6 ;
	mstate2.check_columns(event, rows-1) ;

	int8_t  sub    = mstate2.m_posVert ;
	uint16_t y ;
	uint8_t subN = 0 ;
	uint16_t colour = dimBackColour() ;
	uint8_t attr ;
	
	lcd_hline( 0, TTOP, TRIGHT ) ;
	
	int32_t newSelection = checkTouchSelect( rows, 0) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}
	
	y = TTOP ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Trainer Polarity"), y, attr ) ;
	lcd_putsAttIdxColour( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR(STR_POS_NEG), g_model.trainPulsePol, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;
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
		lcd_putc( THOFF+9*FW, y+TVOFF, '(' ) ;
    lcd_putc( THOFF+16*FW, y+TVOFF, ')' ) ;
		validateText( g_eeGeneral.trainerProfile[value-1].profileName, 6 ) ;
		lcd_putsnAtt( THOFF+10*FW, y+TVOFF, (const char *)g_eeGeneral.trainerProfile[value-1].profileName, 6, 0 ) ;
	}
  else
	{
  	lcd_putsAttColour( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR(STR_OFF), 0, attr ? ~LcdForeground : LcdForeground, attr ? ~LcdBackground : LcdBackground ) ;
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
	TlExitIcon = 1 ;

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

	int32_t newSelection = checkTouchSelect( rows, 0) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}
	
	y = TTOP ;

	drawItem( (char *)XPSTR("Function"), y, sub == 0 ? ~colour : colour ) ;

	attr = 0 ;
	if ( sub == 0 )
	{
		attr = INVERS ;
	}
	lcd_putsAttIdxColour( TRIGHT-TRMARGIN-7*FW, y+TVOFF, PSTR(CSWITCH_STR), cs.func, 0, attr ? ~LcdForeground : LcdForeground, attr ? ~colour : colour ) ;

	y += TFH ;
	
	drawItem( (char *)XPSTR("V1"), y, sub == 1 ) ;

	y += TFH ;
	
	drawItem( (char *)XPSTR(""), y, sub == 2 ) ;
  if( (cstate == CS_VOFS) || (cstate == CS_2VAL) )
	{
		lcd_puts_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("val") ) ;
	}
	else
	{
		lcd_puts_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("V2") ) ;
	}
	 
	y += TFH ;

	if ( extra )
	{
		drawItem( (char *)XPSTR("V3"), y, sub == 3 ) ;
	 
		y += TFH ;
	}
	 
	drawItem( (char *)XPSTR("AND"), y, sub == 3+extra ) ;
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
		lcd_puts_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("val") ) ;
		saveEditColours( (sub == 1), colour ) ;
		putsChnRaw( TRIGHT-TRMARGIN-4*FW, TTOP+TFH+TVOFF, cs.v1u, 0 ) ;
		restoreEditColours() ;

    if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
		{
			int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
			saveEditColours( (sub == 2), colour ) ;
			putsTelemetryChannel( TRIGHT-TRMARGIN-5*FW, TTOP+2*TFH+TVOFF, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT ) ;
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
				putsTelemetryChannel( TRIGHT-TRMARGIN-5*FW, TTOP+3*TFH+TVOFF, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT ) ;
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
   	putsDrSwitches(TRIGHT-TRMARGIN-4*FW, TTOP+TFH+TVOFF, cs.v1, 0 ) ;
		restoreEditColours() ;
		saveEditColours( sub==2, colour ) ;
   	putsDrSwitches(TRIGHT-TRMARGIN-4*FW, TTOP+2*TFH+TVOFF, cs.v2, 0 ) ;
		restoreEditColours() ;
  }
  else if(cstate == CS_VCOMP)
  {
		saveEditColours( sub==1, colour ) ;
		putsChnRaw( TRIGHT-TRMARGIN-4*FW, TTOP+TFH+TVOFF, cs.v1u, 0 ) ;
		restoreEditColours() ;
		saveEditColours( sub==2, colour ) ;
		putsChnRaw( TRIGHT-TRMARGIN-4*FW, TTOP+2*TFH+TVOFF, cs.v2u, 0 ) ;
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
	  lcd_puts_P( THOFF,TTOP+TFH+TVOFF, XPSTR("Off") ) ;
		drawNumber( TRIGHT-TRMARGIN, TTOP+TFH, x+1, att) ; //, (sub == 1) ? ~colour : colour ) ;
		att = 0 ;
		x = cs.v2 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
	  lcd_puts_P( THOFF,TTOP+2*TFH+TVOFF, XPSTR("On ") ) ;
		drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, x+1, att) ; //, (sub == 2) ? ~colour : colour ) ;
		cs.exfunc = 0 ;
	}
	else if(cstate == CS_TMONO)
	{
		saveEditColours( sub==1, colour ) ;
   	putsDrSwitches(TRIGHT-TRMARGIN-4*FW, TTOP+TFH+TVOFF, cs.v1, 0 ) ;
		restoreEditColours() ;
		uint8_t att = 0 ;
		int8_t x ;
		x = cs.v2 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
		lcd_puts_P( THOFF, TTOP+2*TFH+TVOFF, XPSTR("Time") ) ;
		drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, x+1, att) ; //, (sub == 2) ? ~colour : colour ) ;
		cs.exfunc = 0 ;
	}
	else// cstate == U16
	{
		uint16_t x ;
		x = (uint8_t) cs.v2 ;
		x |= cs.bitAndV3 << 8 ;
		saveEditColours( sub==1, colour ) ;
		putsChnRaw( TRIGHT-TRMARGIN-4*FW, TTOP+TFH+TVOFF, cs.v1u, 0 ) ;
		restoreEditColours() ;
		drawNumber( TRIGHT-TRMARGIN, TTOP+2*TFH, x, 0) ; //, (sub == 2) ? ~colour : colour ) ;
	}

	saveEditColours( sub==3+extra, colour ) ;
 	putsDrSwitches(TRIGHT-TRMARGIN-4*FW, TTOP+3*TFH+TVOFF+Vextra, getAndSwitch( cs ), 0 ) ;
	restoreEditColours() ;

	lcd_putsAttIdx( THOFF, TTOP+3*TFH+TVOFF+Vextra, XPSTR("\003ANDOR XOR"), cs.exfunc, 0 ) ;

	lcd_putsAttIdxColour( TRIGHT-TRMARGIN-3*FW, TTOP+5*TFH+TVOFF+Vextra, XPSTR("\003ANDOR XOR"), cs.exfunc, 0, ( sub == 5+extra ) ? ~LcdForeground : LcdForeground, ( sub == 5+extra ) ? ~colour : colour ) ;

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
		if ( checkForMenuEncoderLong( event ) || handleSelectIcon() )
		{
			Clipboard.clipswitch = cs ;
			Clipboard.content = CLIP_SWITCH ;
		}
	}

	if ( sub == 7+extra )
	{
		if ( checkForMenuEncoderLong( event ) || handleSelectIcon() )
		{
			cs = Clipboard.clipswitch ;
		}
	}
}

void menuProcSwitches(uint8_t event)
{
	uint32_t rows = NUM_SKYCSW ;
	TITLE(PSTR(STR_CUST_SWITCH));
	TlExitIcon = 1 ;
	EditType = EE_MODEL ;
	static MState2 mstate2;
	event = mstate2.check_columns(event, rows-1 ) ;

	uint32_t selected = 0 ;
	uint32_t newVpos ;

	uint16_t y ;
	uint8_t k = 0 ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t t_pgOfs ;

	t_pgOfs = evalHresOffset( sub ) ;

	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	if ( newSelection >= 0 )
	{
//		if ( sub == newSelection )
//		{
//			selected = 1 ;
//		}
		sub = mstate2.m_posVert = newSelection ;
	}
	else if (newSelection == -2)
	{
		selected = 1 ;
	}

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
		
	lcd_hline( 0, TTOP, TRIGHT ) ;

	for(uint32_t i = 0 ; i < TLINES ; i += 1 )
	{
    y=(i)*TFH +TTOP+TVOFF ;
    k = i + t_pgOfs ;
    uint8_t attr = sub == k ? INVERS : 0 ;
		uint8_t m = k ;
    SKYCSwData &cs = g_model.customSw[m] ;

		//write SW names here
		displayLogicalSwitch( 0, y, m ) ;

		uint16_t oldBcolour = LcdBackground ;
		uint16_t oldFcolour = LcdForeground ;
    
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 12*TSCALE, (y-TVOFF)*TSCALE+2, (TRIGHT-12)*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}

//		if ( k < NUM_SKYCSW )
//		{
			lcd_putsAttIdx( 2*FW+1, y, PSTR(CSWITCH_STR),cs.func, 0) ;

  	  uint8_t cstate = CS_STATE(cs.func);

	  	if( (cstate == CS_VOFS) || (cstate == CS_2VAL) )
    	{
				putsChnRaw(    10*FW-6, y, cs.v1u  , 0);
	  	  if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
 				{
					int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
					putsTelemetryChannel( 18*FW-8, y, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT);
				}
    	  else
				{
    	    lcd_outdezAtt( 18*FW-9, y, cs.v2  ,0);
				}
    	}
    	else if(cstate == CS_VBOOL)
    	{
    	  putsDrSwitches(10*FW-6, y, cs.v1  , 0);
    	  putsDrSwitches(14*FW-7, y, cs.v2  , 0);
    	}
    	else if(cstate == CS_VCOMP)
    	{
    	  putsChnRaw(    10*FW-6, y, cs.v1u  , 0);
    	  putsChnRaw(    14*FW-4, y, cs.v2u  , 0);
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
	  	  lcd_puts_Pleft( y, PSTR(STR_15_ON) ) ;
    	  lcd_outdezAtt( 13*FW-5, y, x+1  ,att ) ;
				att = 0 ;
				x = cs.v2 ;
				if ( x < 0 )
				{
					x = -x-1 ;
					att = PREC1 ;
				}
    	  lcd_outdezAtt( 18*FW-3, y, x+1 , att ) ;
			}
			else if(cstate == CS_TMONO)
			{
    	  putsDrSwitches(10*FW-6, y, cs.v1  , 0);
				uint8_t att = 0 ;
				int8_t x ;
				x = cs.v2 ;
				if ( x < 0 )
				{
					x = -x-1 ;
					att = PREC1 ;
				}
    	  lcd_outdezAtt( 17*FW-2, y, x+1 , att ) ;
			}
			else// cstate == U16
			{
				uint16_t x ;
				x = cs.v2u ;
				x |= cs.bitAndV3 << 8 ;
    	  putsChnRaw( 10*FW-6-FW, y, cs.v1  , 0);
    	  lcd_outdezNAtt( 18*FW-9, y, x  , 0,5);
			}
			putsDrSwitches( 18*FW-3, y, getAndSwitch( cs ),0 ) ;

			lcd_puts_P( 22*FW, y, XPSTR("Delay" ) ) ;
			lcd_outdezAtt( TRIGHT-TRMARGIN, y, g_model.switchDelay[m], PREC1 ) ;

			if ( attr )
			{
				if ( ( checkForMenuEncoderBreak( event ) ) || selected )
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
		lcd_hline( 0, y+TFH-TVOFF, TRIGHT ) ;
	}
}

void putsAttIdxTelemItems( uint8_t x, uint8_t y, uint8_t index, uint8_t attr, uint16_t colour = LcdForeground, uint16_t bgColour = LcdBackground ) ;
void lcd_xlabel_decimal( uint8_t x, uint8_t y, uint16_t value, uint8_t attr, const char *s ) ;


void menuOneSafetySwitch(uint8_t event)
{
	TITLE(XPSTR("Safety Switch")) ;
	static MState2 mstate2 ;
	TlExitIcon = 1 ;
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
	
	lcd_outdez( 29*FW, 0, g_chans512[index]/2 + 1500 ) ;

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
	saveEditColours( sub == subN, colour ) ;
  lcd_putsAttIdx( TRIGHT-5-6*FW, y+TVOFF, XPSTR("\006SafetySticky"), uvalue, 0 ) ;
	restoreEditColours() ;
	sd->opt.ss.mode = uvalue ? 3 : 0 ;
	y += TFH ;
	subN += 1 ;

  attr = 0 ;
  if(sub==subN)
	{
		attr = INVERS ;
	}
	drawItem( XPSTR("Switch"), y, (attr) ) ;
	saveEditColours( attr, colour ) ;
	sd->opt.ss.swtch = edit_dr_switch( TRIGHT-5-4*FW, y+TVOFF, sd->opt.ss.swtch, 0, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
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
			lcd_putc( TRIGHT-TRMARGIN-4*FW, y+TVOFF, '!' ) ;
		}
		if ( temp > 7 )
		{ // AN extra pot
			temp += EXTRA_POTS_START - 8 ;
		}
		putsChnRaw( TRIGHT-TRMARGIN-3*FW, y+TVOFF, temp, 0 ) ;
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
	TlExitIcon = 1 ;
	EditType = EE_MODEL ;
#ifdef NO_VOICE_SWITCHES 
 #define SAFE_VAL_OFFSET	0
	uint32_t rows = NUM_SKYCHNOUT+1+1+NUM_VOICE-1 - 1 ;
#else
 #define SAFE_VAL_OFFSET	1
	uint32_t rows = NUM_SKYCHNOUT+1+1+NUM_VOICE-1 ;
#endif
	static MState2 mstate2;
	event = mstate2.check_columns(event, rows - 1 ) ;

	uint16_t y = 0 ;
	uint8_t k = 0 ;
	int8_t sub = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz ;
  uint8_t t_pgOfs ;

//	t_pgOfs = evalHresOffset( sub ) ;
	t_pgOfs = evalOffset( sub ) ;


#ifdef NO_VOICE_SWITCHES
	lcd_outdez( 24*FW, 0, g_chans512[sub]/2 + 1500 ) ;
#else
	if ( sub )
	{
    lcd_outdez( 24*FW, 0, g_chans512[sub-SAFE_VAL_OFFSET]/2 + 1500 ) ;
	}
#endif

	for(uint8_t i=0; i<SCREEN_LINES-1; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
#ifndef NO_VOICE_SWITCHES
		if ( k == 0 )
		{
			uint8_t attr = 0 ;
    	if(sub==k)
			{
				Columns = 0 ;
				attr = InverseBlink ;
#if EXTRA_SKYCHANNELS
  	    CHECK_INCDEC_H_MODELVAR( g_model.numVoice, -8, NUM_SKYCHNOUT ) ;
#else
  	    CHECK_INCDEC_H_MODELVAR_0( g_model.numVoice, NUM_SKYCHNOUT ) ;
#endif
			}	
			lcd_xlabel_decimal( 18*FW, y, g_model.numVoice+NUM_VOICE, attr, PSTR(STR_NUM_VOICE_SW) ) ;
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
				Columns = 2 ;
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
    	uint8_t attr = (getSwitch00(sd->opt.ss.swtch) ) ? INVERS : 0 ;

			if ( k <= numSafety )
			{
  		  if ( s_editMode == false )
				{
					if ( handleSelectIcon() )// || selected )
					{
						s_currIdx = sub-1 ;
						pushMenu(menuOneSafetySwitch) ;
  		  		s_editMode = false ;
					}
				}
#ifdef NO_VOICE_SWITCHES
	    	putsChn(0,y,k+1,attr);
#else
  	  	putsChn(0,y,k,attr);
#endif
  	  	for(uint8_t j=0; j<5;j++)
				{
    		  attr = ((sub==k && subSub==j) ? InverseBlink : 0);
					uint8_t active = (attr && (s_editMode || P1values.p1valdiff)) ;
  	  	  if (j == 0)
					{
						lcd_putsAttIdx( 5*FW-3, y, XPSTR("\001SAVX"), sd->opt.ss.mode, attr ) ;
	      	  if(active)
						{
							uint8_t b = sd->opt.ss.mode ;
	  	        CHECK_INCDEC_H_MODELVAR_0( sd->opt.ss.mode, 3 ) ;
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
						int8_t max = MaxSwitchIndex ;
						if ( sd->opt.ss.mode == 2 )
						{
							max = MaxSwitchIndex+3 ;
						}	 
//						if ( ( sd->opt.ss.swtch > MAX_SKYDRSWITCH ) && ( sd->opt.ss.swtch <= MAX_SKYDRSWITCH + 3 ) )
//						{
//							lcd_putsAttIdx( 6*FW, y, PSTR(STR_V_OPT1), sd->opt.ss.swtch-MAX_SKYDRSWITCH-1, attr ) ;
//						}
//						else
						{
         	  	putsDrSwitches(6*FW, y, sd->opt.ss.swtch, attr);
						}
	    	    if(active)
						{
              CHECK_INCDEC_MODELSWITCH( sd->opt.ss.swtch, -MaxSwitchIndex, max ) ;
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
							lcd_putsAttIdx(14*FW, y, PSTR(STR_SOUNDS), sd->opt.ss.val,attr);
						}
						else if ( sd->opt.ss.mode == 2 )
						{
							if ( sd->opt.ss.swtch > MAX_SKYDRSWITCH )
							{
								min = 0 ;
								max = NUM_TELEM_ITEMS-1 ;
								sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
  							putsAttIdxTelemItems( 15*FW, y, sd->opt.ss.val+1, attr ) ;
							}
							else
							{
								min = -128 ;
								max = 111 ;
								sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
        				lcd_outdezAtt( 15*FW, y, sd->opt.ss.val+128, attr);
							}
						}
						else
						{
        			lcd_putc( 14*FW, y, '.' ) ;
							min = -125 ;
							max = 125 ;
        			lcd_outdezAtt(  14*FW, y, sd->opt.ss.val, attr);
// Option to display current channel value, before limits etc., for failsafe
						}
  	  	    if(active)
						{
		          CHECK_INCDEC_H_MODELVAR( sd->opt.ss.val, min,max);
    	  	  }
					}
					else if ( j == 3 )
					{
						if ( ( sd->opt.ss.mode == 0 ) || ( sd->opt.ss.mode == 3 ) )
						{
    					if(sub==k)
							{							
								Columns = 3 ;
							}
        			lcd_outdezAtt(  15*FW+3, y, sd->opt.ss.tune, attr);
	  	  	    if(active)
							{
			          CHECK_INCDEC_H_MODELVAR( sd->opt.ss.tune, 0, 9 ) ;
    		  	  }
						}
					}
					else
					{
						if ( sd->opt.ss.mode == 3 )
						{
    					if(sub==k)
							{							
								Columns = 4 ;
							}
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
								lcd_putc( 16*FW, y, '!' ) ;
							}
							if ( temp > 7 )
							{ // AN extra pot
								temp += EXTRA_POTS_START - 8 ;
							}
							putsChnRaw( 17*FW, y, temp, attr ) ;
	    	    	if(active)
							{
								temp = sd->opt.ss.source ;
								CHECK_INCDEC_H_MODELVAR( temp, -3-NumExtraPots , 3+NumExtraPots ) ;
								sd->opt.ss.source = temp ;
    		    	}
						}
					}
    	  }
    	}
			else
			{
	  	  lcd_puts_Pleft( y, PSTR(STR_VS) ) ;
				uint16_t x1 ;
				x1 = 0 + 4*FW-2 ;
				if ( k < 10 )
				{
					x1 -= FWNUM ;			
				}
  			lcd_outdezAtt(x1,y,k,attr);
    		for(uint8_t j=0; j<3;j++)
				{
    	    uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
					uint8_t active = (attr && (s_editMode || P1values.p1valdiff)) ;
    		  if (j == 0)
					{
    		    if(active)
						{
    			    CHECK_INCDEC_MODELSWITCH( sd->opt.vs.vswtch, -MaxSwitchIndex+1, MaxSwitchIndex-1 ) ;
    		    }
  			    putsDrSwitches(5*FW, y, sd->opt.vs.vswtch, attr);
					}
    		  else if (j == 1)
    		  {
    		    if(active)
						{
    			    CHECK_INCDEC_H_MODELVAR_0( sd->opt.vs.vmode, 6 ) ;
    		    }
						lcd_putsAttIdx( 10*FW, y, PSTR(STR_VOICE_OPT), sd->opt.vs.vmode, attr ) ;
					}
					else
					{
						uint8_t max ;
						if ( sd->opt.vs.vmode > 5 )
						{
							max = NUM_TELEM_ITEMS-1 ;
							sd->opt.vs.vval = limit( (uint8_t)0, sd->opt.vs.vval, max) ;
							putsAttIdxTelemItems( 16*FW, y, sd->opt.vs.vval+1, attr ) ;
						}
						else
						{
							// Allow 251-255 to represent GVAR3-GVAR7
							max = 255 ;
							if ( sd->opt.vs.vval <= 250 )
							{
	  						lcd_outdezAtt( 19*FW, y, sd->opt.vs.vval, attr) ;
							}	
							else
							{
								dispGvar( 16*FW, y, sd->opt.vs.vval-248, attr ) ;
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
//		else
//		{
//			// Function(s)
//			lcd_puts_Pleft( y, PSTR("F1") ) ;
//  	 	for( uint8_t j=0 ; j<3 ; j += 1 )
//			{
//  	    uint8_t attr = ((sub==k && subSub==j) ? (s_editMode ? BLINK : INVERS) : 0) ;
//				uint8_t active = (attr && s_editMode) ;
//  	 		if (j == 0)
//				{
//					putsDrSwitches( 5*FW, y, Function[0].swtch , attr ) ;
//  	 		  if(active)
//					{
//						Function[0].swtch = checkIncDec( event, Function[0].swtch, -MAX_DRSWITCH+1, MAX_DRSWITCH, 0 ) ;
//  	 		  }
//				}
//  	 		else if (j == 1)
//  	 		{
//					lcd_putsAttIdx( 10*FW, y, PSTR("\005-----variovarA2"), Function[0].func, attr ) ;
//  	 		  if(active)
//					{
//						Function[0].func = checkIncDec( event, Function[0].func, 0, 2, 0 ) ;
//  	 		  }
//				}
//				else
//				{
// 					lcd_outdezAtt( 17*FW, y, Function[0].param[0], attr) ;
//  	 		  if(active)
//					{
//						Function[0].param[0] = (uint8_t)checkIncDec( event, (int8_t)Function[0].param[0], 0, 50, 0 ) ;
//  	 		  }
//				}	
//			}
//		}
	}
}

char StringAdjustFunctions[] = "\005-----Add  Set CSet V+/-  Inc/0Dec/0+/Lim-/Lim" ;

void menuOneAdjust(uint8_t event)
{
	TITLE(XPSTR("GVAR Adjust"));
	EditType = EE_MODEL ;
	static MState2 mstate2;
	TlExitIcon = 1 ;
	uint32_t rows = 4 ;
 	event = mstate2.check_columns(event, rows - 1 ) ;

	uint8_t sub = mstate2.m_posVert ;
	GvarAdjust *pgvaradj ;
	pgvaradj = ( s_currIdx >= NUM_GVAR_ADJUST ) ? &g_model.egvarAdjuster[s_currIdx - NUM_GVAR_ADJUST] : &g_model.gvarAdjuster[s_currIdx] ;

 	lcd_outdezAtt(21*FW, 0, 1+ s_currIdx, 0 ) ;

	uint16_t colour = dimBackColour() ;

	int32_t newSelection = checkTouchSelect( rows, 0) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}

	lcd_hline( 0, TTOP, TRIGHT ) ;

	for (uint8_t i = 0 ; i < rows ; i += 1 )
	{
    uint16_t y = i * TFH + TTOP ;
    uint8_t attr = (sub==i) ? INVERS : 0 ;

		switch(i)
		{
			case 0 : // GV
				drawItem( (char *)XPSTR("GVAR"), y, attr ) ;
				if ( pgvaradj->gvarIndex > 6 )
				{
					drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), pgvaradj->gvarIndex-6, attr ) ;
				}
				else
				{
					drawNumber( TRIGHT-TRMARGIN, y, pgvaradj->gvarIndex+1, attr ) ;
				}
  			if(attr)
				{
 	        CHECK_INCDEC_H_MODELVAR( pgvaradj->gvarIndex, 0, 10 ) ;
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
				saveEditColours( attr, colour ) ;
				pgvaradj->swtch = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, pgvaradj->swtch, 0, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
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
					saveEditColours( attr, colour ) ;
					pgvaradj->switch_value = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, pgvaradj->switch_value, 0, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
					restoreEditColours() ;
				}
			break ;
		}
	}
}

#ifdef BIG_SCREEN
#define ADJ_OFF_0			(8*FW)
#else
#define ADJ_OFF_0			0
#endif

void menuProcAdjust(uint8_t event)
{
	TITLE(XPSTR("GVAR Adjust"));
	EditType = EE_MODEL ;
	TlExitIcon = 1 ;
	uint32_t rows = NUM_GVAR_ADJUST + EXTRA_GVAR_ADJUST ;
	static MState2 mstate2;
 	event = mstate2.check_columns(event, rows - 1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;
	
	uint32_t selected = 0 ;
	uint32_t newVpos ;
	uint8_t k = 0 ;
  uint8_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;

//	NoCheckSelect = 1 ;
	int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 1 ) ;
	if ( newSelection >= 0 )
	{
//		if ( sub == newSelection )
//		{
//			selected = 1 ;
//		}
		sub = mstate2.m_posVert = newSelection ;
	}
	else if (newSelection == -2)
	{
		selected = 1 ;
	}

	if ( handleSelectIcon() || selected || ( event == EVT_KEY_BREAK(BTN_RE) ) )
	{
		s_currIdx = sub ;
		killEvents(event);
		s_editMode = false ;
		pushMenu(menuOneAdjust) ;
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

	lcd_hline( 0, TTOP, TRIGHT ) ;

	for (uint32_t i=0; i<TLINES; i++ )
	{
		GvarAdjust *pgvaradj ;
		uint8_t idx ;
    y=(i)*TFH +TTOP+TVOFF ;
    k=i+t_pgOfs;
		pgvaradj = ( k >= NUM_GVAR_ADJUST ) ? &g_model.egvarAdjuster[k - NUM_GVAR_ADJUST] : &g_model.gvarAdjuster[k] ;
		idx = pgvaradj->gvarIndex ;
  	lcd_putc( 0, y, 'A' ) ;
		if ( k < 9 )
		{
  		lcd_putc( 1*FW, y, k+'1' ) ;
		}
		else
		{
  		lcd_putc( 1*FW, y, k > 18 ? '2' : '1' ) ;
  		lcd_putc( 2*FW, y, ( (k+1) % 10) +'0' ) ;
		}

		if ( sub==k )
		{
			int8_t value ;
			if ( idx > 6 )
			{
				value = getTrimValue( CurrentPhase, idx - 7  ) ;
				lcd_putsAttIdx( 17*FW, 0, PSTR(STR_GV_SOURCE), idx-6, 0 ) ;
			}
			else
			{
	  		lcd_puts_P( 17*FW, 0, XPSTR("GV =") ) ;
				value = g_model.gvars[idx].gvar ;
  			lcd_putc( 19*FW, 0, idx+'1' ) ;
			}
			lcd_outdez( 25*FW, 0, value ) ;
		} 
		uint8_t attr = ((sub==k) ? INVERS : 0);
		
//		uint16_t oldBcolour = LcdBackground ;
		uint16_t oldFcolour = LcdForeground ;
		
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 3*FW*TSCALE, (y-TVOFF)*TSCALE+2, (TRIGHT-3*FW)*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
//			LcdBackground = ~LcdBackground ;
			LcdForeground = ~LcdForeground ;
		}

//		for( uint32_t j = 0 ; j < 4 ; j += 1 )
//		{
//			if ( j == 0 )
//			{
				if ( idx > 6 )
				{
					lcd_putsAttIdx( 3*FW+2, y, PSTR(STR_GV_SOURCE), idx-6, 0 ) ;
				}
				else
				{
  				lcd_puts_P( 3*FW+2, y, XPSTR("GV") ) ;
					lcd_putcAtt( 5*FW+2, y, idx+'1', 0 ) ;
				}
//			}
//			else if ( j == 1 )
//			{
				lcd_putsAttIdx( 7*FW, y, StringAdjustFunctions, pgvaradj->function, 0);
//			}
//			else if ( j == 2 )
//			{
      	putsDrSwitches(13*FW-1, y, pgvaradj->swtch, 0);
//			}
//			else
//			{
				if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
				{
					if ( pgvaradj->function == 3 )
					{
						lcd_putsAttIdx( 18*FW, y, PSTR(STR_GV_SOURCE), pgvaradj->switch_value, attr ) ;
					}
					else
					{
						lcd_outdezAtt( 21*FW, y, pgvaradj->switch_value, 0 ) ;
					}
				}
				else
				{
      		putsDrSwitches(17*FW, y, pgvaradj->switch_value, 0);
				}
//			}
//		}
//		LcdBackground = oldBcolour ;
		LcdForeground = oldFcolour ;
		lcd_hline( 0, y+TFH-TVOFF, TRIGHT ) ;
	}
	
}

void menuOneGvar(uint8_t event)
{
	TITLE(PSTR(STR_GLOBAL_VARS)) ;
	EditType = EE_MODEL ;
	static MState2 mstate2;
	TlExitIcon = 1 ;
	uint32_t rows = 3 ;	
	GvarData *pgvar ;
	
	event = mstate2.check_columns(event, rows - 1 ) ;

	uint8_t sub = mstate2.m_posVert ;

  lcd_putc( 20*FW, 0, '1'+ s_currIdx ) ;
	uint16_t colour = dimBackColour() ;

	int32_t newSelection = checkTouchSelect( rows, 0) ;
	if ( newSelection >= 0 )
	{
		sub = mstate2.m_posVert = newSelection ;
	}

	lcd_hline( 0, TTOP, TRIGHT ) ;

	pgvar = &g_model.gvars[s_currIdx] ;

	for (uint8_t i = 0 ; i < rows ; i += 1 )
	{
    uint16_t y = i * TFH + TTOP ;
    uint8_t attr = (sub==i) ? INVERS : 0 ;
		
		switch(i)
		{
			case 0 : // switch
				drawItem( (char *)PSTR(STR_SWITCH), y, attr ) ;
				saveEditColours( attr, colour ) ;
				g_model.gvswitch[s_currIdx] = edit_dr_switch( TRIGHT-TRMARGIN-4*FW, y+TVOFF, g_model.gvswitch[s_currIdx], 0, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				restoreEditColours() ;
			break ;
			case 1 :
				drawItem( (char *)XPSTR("Source"), y, attr ) ;
				drawIdxText( y, (char *)PSTR(STR_GV_SOURCE), pgvar->gvsource, attr ) ;
				// STR_GV_SOURCE
 				if(attr)
				{ 
					CHECK_INCDEC_H_MODELVAR( pgvar->gvsource, 0, 69 ) ;
				}
			break ;
			case 2 :
				drawItem( (char *)XPSTR("Value"), y, attr ) ;
				drawNumber( TRIGHT-TRMARGIN, y, pgvar->gvar, attr ) ;
  			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( pgvar->gvar, -125, 125 ) ;
				}
			break ;
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
	uint32_t rows = MAX_GVARS ;
	uint32_t selected = 0 ;
	TlExitIcon = 1 ;
	
	event = mstate2.check_columns(event, rows - 1 ) ;

	uint8_t sub = mstate2.m_posVert ;
	uint16_t y ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

//	NoCheckSelect = 1 ;
	int32_t newSelection = checkTouchSelect( rows, 0, 1 ) ;
	if ( newSelection >= 0 )
	{
		if ( sub == newSelection )
		{
			selected = 1 ;
		}
		sub = mstate2.m_posVert = newSelection ;
	}

	if ( handleSelectIcon() || selected || ( event == EVT_KEY_BREAK(BTN_RE) ) )
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

	for (uint32_t i = 0 ; i<MAX_GVARS ; i += 1 )
	{
    y=(i)*TFH +TTOP ;
  	uint8_t attr = ((sub==i) ? INVERS : 0);
		if ( attr )
		{
			lcdDrawSolidFilledRectDMA( 0, y*TSCALE+2, TRIGHT*TSCALE, TFH*TSCALE-2, ~LcdBackground ) ;
		}
		saveEditColours( attr, LcdBackground ) ;
  	lcd_puts_P( THOFF, y+TVOFF, PSTR(STR_GV));
		lcd_putc( THOFF+2*FW, y+TVOFF, i+'1') ;
		GvarData *pgvar ;
		pgvar = &g_model.gvars[i] ;
    putsDrSwitches( 7*FW, y+TVOFF, g_model.gvswitch[i], 0 );
		lcd_putsAttIdx( 14*FW-4, y+TVOFF, PSTR(STR_GV_SOURCE), pgvar->gvsource, 0 ) ;
		lcd_outdezAtt( 22*FW, y+TVOFF, pgvar->gvar, 0) ;
		restoreEditColours() ;
		lcd_hline( 0, y+TFH, TRIGHT ) ;
	}
	
}


#define NUM_MIX_SWITCHES	(9+NUM_SKYCSW)

SKYMixData *mixAddress( uint32_t index ) ;
uint8_t mapMixSource( uint8_t index, uint8_t switchSource ) ;
uint8_t unmapMixSource( uint8_t index, uint8_t *switchSource ) ;
void putsChnOpRaw( uint8_t x, uint8_t y, uint8_t source, uint8_t switchSource, uint8_t output, uint8_t attr ) ;
uint8_t editSlowDelay( uint8_t y, uint8_t attr, uint8_t value) ;
uint16_t extendedValueEdit( int16_t value, uint8_t extValue, uint8_t attr, uint8_t y, uint8_t event, uint16_t x ) ;
int16_t gvarDiffValue( uint16_t x, uint16_t y, int16_t value, uint32_t attr, uint8_t event ) ;
int16_t getTrimValue( uint8_t phase, uint8_t idx ) ;
void put_curve( uint8_t x, uint8_t y, int8_t idx, uint8_t attr ) ;
void menuProcExpoAll(uint8_t event) ;
uint8_t onoffMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition ) ;
uint8_t offonMenuItem( uint8_t value, uint8_t y, const char *s, uint8_t condition ) ;
uint8_t offonItem( uint8_t value, uint8_t y, uint8_t condition ) ;

void drawSmallGVAR( uint16_t x, uint16_t y )
{
	lcd_putsSmall( x, y, (uint8_t *)"GVAR", LcdForeground) ;
}

void menuProcMixOne(uint8_t event)
{
	uint8_t rows = 16 ;
	SKYMixData *md2 = mixAddress( s_curItemIdx ) ;
  
	TlExitIcon = 1 ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, rows-1 ) ;
	uint8_t x = TITLEP( PSTR(STR_EDIT_MIX));
	uint32_t newVpos ;
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

	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	if ( newSelection >= 0 )
	{
		if ( sub == newSelection )
		{
//			selected = 1 ;
			event = Tevent = EVT_KEY_BREAK(BTN_RE) ;
			s_editMode = !s_editMode ;
		}
		sub = mstate2.m_posVert = newSelection ;
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
		uint16_t y = TTOP + k*TFH ;
		uint8_t i = k + s_pgOfs ;
    uint8_t attr = sub==i ? INVERS : 0 ;
  	uint8_t b ;

    switch(i)
		{
      case 0:
			{	
				drawItem( (char *)PSTR(STR_2SOURCE)+1, y, attr ) ;
				uint32_t value = md2->srcRaw ;
				saveEditColours( attr, DimBackColour ) ;
				putsChnOpRaw( TRIGHT-TRMARGIN-4*FW, y+TVOFF, value, md2->switchSource, md2->disableExpoDr, 0 ) ;
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

//				lcd_putsAtt( TMID+1, y+1, "GVAR", LUA_SMLSIZE ) ;
//				lcd_putsAtt( TMID+1, y+1, "GVAR", CONDENSED ) ;
				saveEditColours( attr, DimBackColour ) ;
				drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+3 ) ;
				uint16_t result = extendedValueEdit( md2->weight, md2->extWeight, attr, y+TVOFF, event, TRIGHT-TRMARGIN ) ;
				md2->weight = result & 0x00FF ;
				md2->extWeight = result >> 8 ;
				restoreEditColours() ;
			}
      break ;
      case 2:
			{	
				drawItem( (char *)PSTR(STR_OFFSET), y, attr ) ;
//				lcd_putsAtt( TMID*TSCALE+1, y+1, "GVAR", 0 ) ;
				saveEditColours( attr, DimBackColour ) ;
				drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+3 ) ;
				uint16_t result = extendedValueEdit( md2->sOffset, md2->extOffset, attr, y+TVOFF, event, TRIGHT-TRMARGIN ) ;
				md2->sOffset = result & 0x00FF ;
				md2->extOffset = result >> 8 ;
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
					int16_t value = curveValue ;
					if ( diffIsGvar  )
					{
						value += 510 ;
					}	
					value = gvarDiffValue( TRIGHT-TRMARGIN, y+TVOFF, value, attr|0x80000000, event ) ;
					if ( value > 500 )
					{
						diffIsGvar = 1 ;
						value -= 510 ;
					}
					else
					{
						diffIsGvar = 0 ;
					}
					curveValue = value ;
				}
				else
				{
					if ( curveFunction == 4 )	// Expo
					{
            lcd_outdezAtt(TRIGHT-TRMARGIN,y+TVOFF,curveValue,0);
            if(attr) CHECK_INCDEC_H_MODELVAR( curveValue, 0, 100 ) ;
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
						put_curve( TRIGHT-TRMARGIN-3*FW, y+TVOFF, temp, 0 ) ;
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
					}
				}
				restoreEditColours() ;
      break ;

      case 8:
				drawItem( (char *)PSTR(STR_2SWITCH)+1, y, attr ) ;
				saveEditColours( attr, DimBackColour ) ;
				putsDrSwitches(TRIGHT-TRMARGIN-4*FW, y+TVOFF, md2->swtch, 0) ;
				restoreEditColours() ;
        if(attr) CHECK_INCDEC_MODELSWITCH( md2->swtch, -MaxSwitchIndex, MaxSwitchIndex);
      break;

      case 9:
			{	
				b = 1 ;
				drawItem( (char *)PSTR(STR_MODES), y, attr ) ;
//            lcd_puts_Pleft( y,XPSTR("\001MODES"));
						
				if ( attr )
				{
					Columns = 7 ;
				}
  					
				for ( uint8_t p = 0 ; p<MAX_MODES+2 ; p++ )
				{
					uint8_t z = md2->modeControl ;
// 					lcd_putcAtt( (9+p)*(FW+1), y, '0'+p, ( z & b ) ? 0 : INVERS ) ;
   				lcd_putcAtt( TRIGHT-TRMARGIN-7*(FW+3)+p*(FW+2), y+TVOFF, '0'+p, ( z & b ) ? 0 : INVERS ) ;
					if( attr && ( g_posHorz == p ) )
					{
						lcd_rect( TRIGHT-TRMARGIN-7*(FW+3)+p*(FW+2)-1, y+TVOFF-1, FW+2, 9 ) ;
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
//            lcd_puts_P(  2*FW,y,PSTR(STR_2WARNING));
				b = md2->mixWarn ;
				saveEditColours( attr, DimBackColour ) ;
        if(b)
          lcd_outdezAtt( TRIGHT-TRMARGIN, y+TVOFF, b, 0 ) ;
        else
          lcd_putsAtt( TRIGHT-TRMARGIN-3*FW, y+TVOFF, PSTR(STR_OFF), 0 ) ;
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

#define WCHART 44
#define X0     (200-WCHART-2 - 2 )	// was 170
#define Y0     60 // 48

#define XD (X0-2)

#define GRAPH_FUNCTION_CURVE		0
#define GRAPH_FUNCTION_EXPO			1

uint8_t get_dr_state(uint8_t x) ;
void editExpoVals(uint8_t event, uint8_t edit, uint8_t x, uint8_t y, uint8_t which, uint8_t exWt, uint8_t stkRL) ;
void drawFunction( uint8_t xpos, uint8_t function ) ;


void menuProcExpoAll(uint8_t event)
{
	TITLE(PSTR(STR_EXPO_DR)) ;
	EditType = EE_MODEL ;
	TlExitIcon = 1 ;
	static MState2 mstate2 ;
	uint32_t count = 5-1 ;
	if ( SingleExpoChan )
	{
		count -= 1 ;
	}
	int8_t  sub = mstate2.m_posVert ;
	
	event = mstate2.check_columns( event, count ) ;

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
	 
	uint8_t stkVal ;
	sub = mstate2.m_posVert ;
	int8_t subN ;
	if ( SingleExpoChan )
	{
		sub += 1 ;
	}
	if( sub )
	{
		StickScrollAllowed = 0 ;
	}

	lcd_hline( 0, TTOP, TMID-20 ) ;

	uint8_t l_expoChan = s_expoChan ;
	subN = 0 ;
  uint8_t attr = 0 ;
	if ( sub == subN )
	{
		lcdDrawSolidFilledRectDMA( 0, TTOP*TSCALE+2, (TMID-20)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
		if ( SingleExpoChan == 0 )
		{
			s_expoChan = l_expoChan = checkIncDec( s_expoChan, 0, 3, 0 ) ;
			attr = InverseBlink ;
		}
	}		 
	saveEditColours( attr, DimBackColour ) ;
	putsChnRaw( THOFF, TTOP+TVOFF,l_expoChan+1,0) ;
	restoreEditColours() ;

	uint8_t expoDrOn = get_dr_state(l_expoChan);
	switch (expoDrOn)
	{
    case DR_MID:
      lcd_puts_P( 8*FW, TTOP+TVOFF,PSTR(STR_4DR_MID)+1);
    break;
    case DR_LOW:
      lcd_puts_P( 8*FW, TTOP+TVOFF,PSTR(STR_4DR_LOW)+1);
    break;
    default: // DR_HIGH:
      lcd_puts_P( 8*FW, TTOP+TVOFF,PSTR(STR_4DR_HI)+1);
    break;
	}

	subN += 1 ;
	stkVal = DR_BOTH ;
	if(calibratedStick[l_expoChan]> 100) stkVal = DR_RIGHT;
	if(calibratedStick[l_expoChan]<-100) stkVal = DR_LEFT;
	if(IS_EXPO_THROTTLE(l_expoChan)) stkVal = DR_RIGHT;

	ExpoData *eptr ;
	eptr = &g_model.expoData[s_expoChan] ;
extern int16_t TouchAdjustValue ;

	lcd_hline( 0, TTOP+TFH, TMID-20 ) ;

	if ( sub==subN )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+TFH)*TSCALE+2, (TMID-20)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	}
	saveEditColours( ( sub==subN ), DimBackColour ) ;
	lcd_puts_P( THOFF,TTOP+TFH+TVOFF,PSTR(STR_2EXPO)+1);
	restoreEditColours() ;

	attr = (stkVal != DR_RIGHT) && (sub==subN) ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+2*TFH)*TSCALE+2, (TMID-20)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	drawSmallGVAR( 1, (TTOP+2*TFH)*TSCALE+3 ) ;
	editExpoVals( event, attr, 6*FW, TTOP+2*TFH+TVOFF, expoDrOn, DR_EXPO, DR_LEFT ) ;
	restoreEditColours() ;
	if ( stkVal == DR_BOTH )
	{
		if ( TouchAdjustValue )
		{
			eptr->expo[expoDrOn][DR_EXPO][DR_RIGHT] = eptr->expo[expoDrOn][DR_EXPO][DR_LEFT] ;
		}
	}
	attr = (stkVal != DR_LEFT) && (sub==subN) ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+2*TFH)*TSCALE+2, ((TMID-20)/2)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	drawSmallGVAR( (TMID-20)/2*TSCALE+1, (TTOP+2*TFH)*TSCALE+3 ) ;
	editExpoVals( event, attr, 14*FW, TTOP+2*TFH+TVOFF, expoDrOn, DR_EXPO, DR_RIGHT ) ;
	restoreEditColours() ;
	lcd_hline( 0, TTOP+2*TFH, TMID-20 ) ;
	subN += 1 ;
	attr = (stkVal != DR_RIGHT) && (sub==subN) ;
	lcd_hline( 0, TTOP+3*TFH, TMID-20 ) ;
	 
	if ( sub==subN )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+3*TFH)*TSCALE+2, (TMID-20)*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	}
	saveEditColours( ( sub==subN ), DimBackColour ) ;
	lcd_puts_P( THOFF,TTOP+3*TFH+TVOFF,PSTR(STR_2WEIGHT)+1);
	restoreEditColours() ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( 0, (TTOP+4*TFH)*TSCALE+2, (TMID-20)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	drawSmallGVAR( 1, (TTOP+4*TFH)*TSCALE+3 ) ;
	editExpoVals( event, attr, 6*FW, TTOP+4*TFH+TVOFF, expoDrOn, DR_WEIGHT, DR_LEFT ) ;
	restoreEditColours() ;
	if ( stkVal == DR_BOTH )
	{
		if ( TouchAdjustValue )
		{
			eptr->expo[expoDrOn][DR_WEIGHT][DR_RIGHT] = eptr->expo[expoDrOn][DR_WEIGHT][DR_LEFT] ;
		}
	}
	attr = (stkVal != DR_LEFT) && (sub==subN) ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+4*TFH)*TSCALE+2, (TMID-20)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	drawSmallGVAR( (TMID-20)/2*TSCALE+1, (TTOP+4*TFH)*TSCALE+3 ) ;
	editExpoVals( event, attr, 14*FW, TTOP+4*TFH+TVOFF, expoDrOn, DR_WEIGHT, DR_RIGHT ) ;
	restoreEditColours() ;
	lcd_hline( 0, TTOP+4*TFH, TMID-20 ) ;
	subN += 1 ;
	lcd_puts_P( THOFF, TTOP+5*TFH+TVOFF,PSTR(STR_DR_SW1));
	lcd_hline( 0, TTOP+5*TFH, TMID-20 ) ;
	attr = sub==subN ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+5*TFH)*TSCALE+2, (TMID-20)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	editExpoVals( event, attr, 10*FW, TTOP+5*TFH+TVOFF, DR_DRSW1, 0, 0 ) ;
	restoreEditColours() ;
	subN += 1 ;
	
	lcd_puts_P( THOFF, TTOP+6*TFH+TVOFF,PSTR(STR_DR_SW2));
	lcd_hline( 0, TTOP+6*TFH, TMID-20 ) ;
	attr = sub==subN ;
	if ( attr )
	{
		lcdDrawSolidFilledRectDMA( (TMID-20)/2*TSCALE, (TTOP+6*TFH)*TSCALE+2, (TMID-20)/2*TSCALE, TFH*TSCALE-2, ~DimBackColour ) ;
	} 
	saveEditColours( attr, DimBackColour ) ;
	editExpoVals( event, attr, 10*FW, TTOP+6*TFH+TVOFF, DR_DRSW2, 0, 0 ) ;
	restoreEditColours() ;
	
	lcd_hline( 0, TTOP+7*TFH, TMID-20 ) ;

	pushPlotType( PLOT_COLOUR ) ;
	
	lcd_vline(XD - (IS_EXPO_THROTTLE(s_expoChan) ? WCHART : 0), Y0 - WCHART, WCHART * 2);

	drawFunction( XD, GRAPH_FUNCTION_EXPO ) ;

	int16_t x512  = calibratedStick[s_expoChan];
	int16_t y512 = calcExpo( l_expoChan, x512 ) ;
  
	lcd_outdezAtt( XD+7*FW, Y0+FH,x512*25/((signed) RESXu/4), 0 );
	lcd_outdezAtt( XD+FW, TTOP,y512*25/((signed) RESXu/4), 0 );
	
	int16_t xv = (x512 * WCHART + RESX/2) / RESX + XD ;
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

void editOneProtocol( uint8_t event )
{
	uint8_t need_bind_range = 0 ;
	EditType = EE_MODEL ;
	uint8_t dataItems = 6 ;
	uint8_t module = EditingModule ;
	uint32_t newVpos ;
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
	TlExitIcon = 1 ;

	if (pModule->protocol == PROTO_OFF)
	{
		mstate2.m_posVert = 0 ;
	}

	displayModuleName( 14*FW, 0, module, 0 ) ;
	event = mstate2.check_columns( event, dataItems-1 ) ;

	int8_t  sub    = mstate2.m_posVert ;
	uint16_t t_pgOfs ;
	
	t_pgOfs = evalHresOffset( sub ) ;

	uint8_t y = TTOP ;

	uint8_t subN = 0 ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

	int32_t newSelection = checkTouchSelect( dataItems, t_pgOfs, 1 ) ;
	if ( newSelection >= 0 )
	{
//		if ( sub == newSelection )
//		{
//			selected = 1 ;
//		}
		sub = mstate2.m_posVert = newSelection ;
	}

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
			 		CHECK_INCDEC_H_MODELVAR_0( svalue, 127 ) ;	// Limited to 8 bits
				}
				pModule->sub_protocol = ( svalue & 0x3F) + (pModule->sub_protocol & 0xC0) ;
				pModule->exsub_protocol = svalue >> 6 ;
				if( svalue != oldValue )
				{
					pModule->channels &= 0x8F ;
					TelemetryType = TEL_UNKNOWN ;
					MultiSetting.protocol[0] = 0 ;
					MultiSetting.subProtocol[0] = 0 ;
//					MultiSetting.timeout = 1 ;
				}
				saveEditColours( attr, DimBackColour ) ;
				displayMultiProtocol( svalue, y, attr ) ;
				restoreEditColours() ;
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
  			lcd_putsAttColour( THOFF, y+TVOFF, XPSTR("Failsafe"), 0, attr ? ~LcdForeground : LcdForeground ) ;
				if ( ( PrivateData[1] && ( ( PrivateData[0] & 0x20 ) == 0 ) ) )
				{
	  			lcd_putsAttColour( 9*FW, y+TVOFF, XPSTR("N/A"), BLINK, attr ? ~LcdForeground : LcdForeground ) ;
				}
				else
				{
					if ( pModule->failsafeMode == 0 )
					{
	  				lcd_putsAttColour( 9*FW, y+TVOFF, XPSTR("(Not Set)"), 0, attr ? ~LcdForeground : LcdForeground ) ;
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
  			lcd_putsAttColour( THOFF, y+TVOFF, XPSTR("Failsafe"), 0, attr ? ~LcdForeground : LcdForeground ) ;
				if ( pModule->failsafeMode == 0 )
				{
	  			lcd_putsAttColour( 9*FW, y+TVOFF, XPSTR("(Not Set)"), 0, attr ? ~LcdForeground : LcdForeground ) ;
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
  			lcd_putsAttColour( THOFF, y+TVOFF, PSTR(STR_BIND), 0, attr ? ~LcdForeground : LcdForeground ) ;
				if ( (pModule->protocol == PROTO_MULTI ) && ( PrivateData[1] ) )
				{
					div_t qr ;
					saveEditColours( attr, DimBackColour ) ;
					lcd_puts_Pleft( y+TVOFF, XPSTR("\006(Ver  . .  .  )" ) ) ;
					lcd_putc( 11*FW, y+TVOFF, PrivateData[1] + '0' ) ;
					lcd_putc( 13*FW, y+TVOFF, PrivateData[2] + '0' ) ;
					qr = div( PrivateData[3], 10 ) ;
					lcd_putc( 16*FW, y+TVOFF, qr.rem + '0' ) ;
					if ( qr.quot )
					{
						lcd_putc( 15*FW, y+TVOFF, qr.quot + '0' ) ;
					}
					qr = div( PrivateData[4], 10 ) ;
					lcd_putc( 19*FW, y+TVOFF, qr.rem + '0' ) ;
					if ( qr.quot )
					{
						lcd_putc( 18*FW, y+TVOFF, qr.quot + '0' ) ;
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
					lcd_puts_Pleft( y+TVOFF, XPSTR("\006(Mode  )" ) ) ;
					lcd_putc( 12*FW, y+TVOFF, (g_model.dsmMode & 0x07) + '0' ) ;
					restoreEditColours() ;
				}

				// XJT version display here
				if ( pModule->protocol == PROTO_PXX )
				{
					saveEditColours( attr, DimBackColour ) ;
					lcd_puts_Pleft( y+TVOFF, XPSTR("\006(Ver      )" ) ) ;
extern uint16_t XjtVersion ;
					lcd_outdezAtt(  14*FW, y+TVOFF, XjtVersion, 0 ) ;
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
  			lcd_putsAttColour( THOFF, y+TVOFF, PSTR(STR_RANGE), 0, attr ? ~LcdForeground : LcdForeground ) ;
				if ( (pModule->protocol == PROTO_MULTI ) && ( PrivateData[1] ) )
				{
					uint8_t sp = pModule->sub_protocol & 0x3F ;
					sp |= pModule->exsub_protocol << 6 ;
					if ( ( sp == M_FRSKYX ) || ( sp == M_FrskyD ) || ( sp == M_FRSKYX2 ) )
					{
						saveEditColours( attr, DimBackColour ) ;
						lcd_puts_P( TRIGHT-TRMARGIN-7*FW, y+TVOFF, XPSTR("Lqi") ) ;
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

extern void createInputMap() ;
extern void displayInputSource( uint8_t x, uint8_t y, uint8_t index, uint8_t att ) ;
extern int16_t gvarMenuItem(uint8_t x, uint8_t y, int16_t value, int8_t min, int8_t max, uint16_t attr, uint8_t event ) ;
extern uint8_t InputMap[] ;
extern uint8_t InputMapSize ;

void editOneInput(uint8_t event)
{
	uint8_t rows = 9 ;
	struct te_InputsData *pinput = &g_model.inputs[s_curItemIdx] ;
//	uint32_t newVpos ;

	TlExitIcon = 1 ;

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

  int8_t  sub    = mstate2.m_posVert;

	uint16_t t_pgOfs ;
	t_pgOfs = evalHresOffset( sub ) ;

	lcd_hline( 0, TTOP, TRIGHT ) ;

	int32_t newSelection = checkTouchSelect( rows, t_pgOfs) ;
	if ( newSelection >= 0 )
	{
//		if ( sub == newSelection )
//		{
//			selected = 1 ;
//		}
		sub = mstate2.m_posVert = newSelection ;
	}

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
				putsChnOpRaw( TRIGHT-TRMARGIN-4*FW, y+TVOFF, value, 0, 0, 0 ) ;
				restoreEditColours() ;
				
				if ( attr )
				{
					uint8_t x = value ;
					if ( handleSelectIcon() || ( event == EVT_KEY_BREAK(BTN_RE) ) )
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
				drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+3 ) ;
				pinput->weight = gvarMenuItem( TRIGHT-TRMARGIN, y+TVOFF, pinput->weight, -100, 100, GVAR_100|attr, event ) ;
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
				putsDrSwitches(TRIGHT-TRMARGIN-4*FW, y+TVOFF, pinput->swtch, 0) ;
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
   				lcd_putcAtt( TRIGHT-TRMARGIN-7*(FW+3)+p*(FW+2), y+TVOFF, '0'+p, ( z & b ) ? 0 : INVERS ) ;
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
				drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+3 ) ;
				


				pinput->offset = gvarMenuItem( TRIGHT-TRMARGIN, y+TVOFF, pinput->offset, -100, 100, GVAR_100|attr, event ) ;
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
					if ( value > 100 )
					{
						value += 400 ;
					}	
					drawSmallGVAR( TMID*TSCALE+1, y*TSCALE+3 ) ;
					value = gvarDiffValue( TRIGHT-TRMARGIN, y+TVOFF, value, attr|0x80000000, event ) ;
					if ( value > 500 )
					{
						value -= 400 ;
					}
					pinput->curve = value ;
				}
				else
				{
					if ( pinput->mode == 4 )	// Expo
					{
            lcd_outdezAtt(TRIGHT-TRMARGIN,y+TVOFF,pinput->curve,0);
            if(attr) CHECK_INCDEC_H_MODELVAR( pinput->curve, -100, 100 ) ;
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
						put_curve( TRIGHT-TRMARGIN-3*FW, y+TVOFF, temp, 0 ) ;
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

void menuProcModelSelect(uint8_t event)
{
  static MState2 mstate2;
  TITLE(PSTR(STR_MODELSEL));
	TlExitIcon = 1 ;
	uint16_t t_pgOfs ;
	uint32_t newVpos ;
  static uint8_t sel_editMode ;

  int8_t subOld  = mstate2.m_posVert;
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
		event = mstate2.check_columns( event, rows-1 ) ;
	}

  int8_t sub = mstate2.m_posVert ;
  if ( DupIfNonzero == 2 )
  {
		sel_editMode = false ;
		DupIfNonzero = 0 ;
  }
  
	t_pgOfs = evalHresOffset( sub ) ;

	if ( !PopupData.PopupActive )
	{
		int32_t newSelection = checkTouchSelect( rows, t_pgOfs, 2 ) ;
		if ( newSelection >= 0 )
		{
			sub = mstate2.m_posVert = newSelection ;
		}

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
    lcd_outdezNAtt(  3*FW, y+TVOFF, k+1, LEADING0,2, fcolour) ;
    if(k==g_eeGeneral.currModel)
		{
			lcd_putcAttColour(1, y+TVOFF, '*', 0, fcolour ) ;
		}
    lcd_putsnAttColour( 4*FW, y+TVOFF, (char *)ModelNames[k+1], sizeof(g_model.name), 0, fcolour ) ;
		
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



