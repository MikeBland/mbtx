/*
 * Author - Mike Blandford
 *
 * Based on er9x by Erez Raviv <erezraviv@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

// This version for ARM based ERSKY9X board

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
#include "vars.h"

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

extern uint8_t g_posHorz ;
extern uint8_t saveHpos ;
extern uint8_t s_currIdx ;
extern uint8_t Columns ;

extern uint8_t s_moveMode;
extern int8_t s_mixMaxSel ;
extern int8_t s_curItemIdx ;
extern int8_t s_currDestCh ;
extern uint8_t s_moveItemIdx ;

extern int8_t qRotary() ;


extern void moveMix(uint8_t idx, uint8_t dir) ;
extern SKYMixData *mixAddress( uint32_t index ) ;
extern void putsChnOpRaw( coord_t x, coord_t y, uint8_t source, uint8_t switchSource, uint8_t output, LcdFlags attr ) ;
#define WBAR2 (50/2)
extern void singleBar( coord_t x0, coord_t y0, int16_t val ) ;
extern int16_t gvarMenuItem(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, uint8_t event ) ;
extern uint16_t extendedValueEdit( int16_t value, uint8_t extValue, LcdFlags attr, coord_t y, uint8_t event, coord_t x ) ;
extern int16_t gvarDiffValue( uint16_t x, uint16_t y, int16_t value, uint32_t attr, uint8_t event ) ;
extern bool reachMixerCountLimit( void ) ;
extern void insertMix(uint8_t idx, uint8_t copy) ;
extern void menuMixOne(uint8_t event) ;
extern void mixpopup( uint8_t event ) ;

extern const uint8_t SwitchFunctionMap[] ;

void displayLogicalSwitch( uint16_t x, uint16_t y, uint32_t index ) ;
int8_t getAndSwitch( SKYCSwData &cs ) ;
void menuSwitchOne(uint8_t event) ;
#if defined(PCBX12D) || defined(PCBX10)
uint8_t putsTelemetryChannel(coord_t x, coord_t y, int8_t channel, int16_t val, LcdFlags att, uint8_t style, uint16_t colour = LcdForeground, uint16_t bgColour = LcdBackground ) ;
#else
uint8_t putsTelemetryChannel(coord_t x, coord_t y, int8_t channel, int16_t val, LcdFlags att, uint8_t style ) ;
#endif
void editOneSwitchItem( uint8_t event, uint32_t item, uint32_t index ) ;

//#ifdef PROP_TEXT
//#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)


#ifdef COLOUR_DISPLAY
#include "menusModelColour.cpp"
#endif


#ifndef COLOUR_DISPLAY
void menuCellScaling(uint8_t event)
{
	static MState2 mstate2 ;
	mstate2.check_columns(event, 12-1 ) ;	
  PUTS_ATT_LEFT( 0, XPSTR( "Cell Scaling" ) ) ;
	coord_t y = 0 ;
	uint32_t k ;
	uint32_t t_pgOfs ;
	uint32_t index ;
  uint32_t sub = mstate2.m_posVert ;
	t_pgOfs = evalOffset( sub ) ;
#ifdef COLOUR_DISPLAY
	for(uint32_t i=0; i<12; i++)
#else
	for(uint32_t i=0; i<SCREEN_LINES-1; i++)
#endif
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    LcdFlags attr ;
  	PUTS_ATT_LEFT( y, XPSTR( "Cell\023v" ) ) ;
		PUTS_NUMX(  6*FW, y, k+1 ) ;
		index = k + FR_CELL1 ;
		attr = TelemetryDataValid[index] ? 0 : BLINK ;
 		PUTS_NUM(  19*FW-2, y, TelemetryData[index], attr | PREC2 ) ;
    attr = ((sub==k) ? InverseBlink : 0);
		uint32_t scaling = 1000 + g_model.cellScalers[k] ;
 		PUTS_NUM(  12*FW, y, scaling/10, attr | PREC2 ) ;
 		PUTS_NUM(  13*FW-1, y, scaling%10, attr ) ;
    if( attr )
		{
    	CHECK_INCDEC_H_MODELVAR( g_model.cellScalers[k], -50, 50 ) ;
		}
	}
}
#endif

#if not (defined(PCBX12D) || defined(PCBX10) || defined(TOUCH))
 #ifdef COLOUR_DISPLAY
#define	VERS_OFF_0			0
 #else
#define VERS_OFF_0			0
 #endif

void menuProcDiagVers(uint8_t event)
{
	TITLE(PSTR(STR_Version));
	static MState2 mstate2;
	mstate2.check_columns(event, 1-1) ;
  
//	#ifdef COLOUR_DISPLAY
//	DisplayOffset = VERS_OFF_0 ;
// #endif
	
	PUTS_ATT_LEFT( 2*FHPY,Stamps ) ;
		
 #ifdef PCBSKY
	#ifndef REVX
	if ( g_eeGeneral.ar9xBoard == 0 )
	{
		PUTS_ATT_LEFT( 7*FHPY, XPSTR("Co Proc"));
    PUT_HEX4( 10*FW-3, 7*FHPY, (Coproc_valid << 8 ) + Coproc_read ) ;
	}
	#endif
 #endif
}
#endif // nTOUCH

#ifdef COLOUR_DISPLAY

#else // COLOUR_DISPLAY

void menuVario(uint8_t event)
{
	TITLE(XPSTR("Vario"));
	static MState2 mstate2 ;
	mstate2.check_columns( event, 6 ) ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t blink = InverseBlink ;

	uint8_t subN = 0 ;
 	for( CPU_UINT j=0 ; j<7 ; j += 1 )
	{
		uint8_t b ;
		LcdFlags attr = (sub==subN) ? blink : 0 ;
		coord_t y = (1+j)*FHPY ;

		switch ( j )
		{
			case 0 :
				PUTS_ATT_LEFT( y, PSTR(STR_VARIO_SRC) ) ;
				PUTS_AT_IDX( 15*FW, y, PSTR(STR_VSPD_A2), g_model.varioData.varioSource, attr ) ;
   		  if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.varioSource, 2+NUM_SCALERS ) ;
   		  }
			break ;
				
			case 1 :
				PUTS_ATT_LEFT( y, PSTR(STR_2SWITCH) ) ;
				g_model.varioData.swtch = edit_dr_switch( 15*FW, y, g_model.varioData.swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			break ;

			case 2 :
				PUTS_ATT_LEFT( y, PSTR(STR_2SENSITIVITY) ) ;
 				PUTS_NUM( 17*FW, y, g_model.varioData.param, attr) ;
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.param, 50 ) ;
	   		}
			break ;

			case 3 :
				PUTS_ATT_LEFT( y, XPSTR("Base Freq.") ) ;
 				PUTS_NUM( 17*FW, y, g_model.varioExtraData.baseFrequency, attr) ;
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( g_model.varioExtraData.baseFrequency, -50, 50 ) ;
	   		}
			break ;

			case 4 :
				PUTS_ATT_LEFT( y, XPSTR("Offset Freq.") ) ;
 				PUTS_NUM( 17*FW, y, g_model.varioExtraData.offsetFrequency, attr) ;
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( g_model.varioExtraData.offsetFrequency, -20, 20 ) ;
	   		}
			break ;

			case 5 :
				PUTS_ATT_LEFT( y, XPSTR("Volume") ) ;
				if ( g_model.varioExtraData.volume )
				{
 					PUTS_NUM( 17*FW, y, g_model.varioExtraData.volume, attr) ;
				}
				else
				{
					PUTS_ATT( FW*15, y, "Vol", attr ) ;
				}
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.varioExtraData.volume, NUM_VOL_LEVELS-1 ) ;
	   		}
			break ;

			case 6 :
	      b = g_model.varioData.sinkTones ;
				g_model.varioData.sinkTones = offonMenuItem( b, y, PSTR(STR_SINK_TONES), attr ) ;
			break ;
		}
		subN += 1 ;
	}
}

#endif // COLOUR_DISPLAY

#ifndef COLOUR_DISPLAY

void menuSwitches(uint8_t event)
{
	TITLE(PSTR(STR_CUST_SWITCH));
	EditType = EE_MODEL ;
#ifndef SWITCH_SUB_ONLY
	Columns = 4 ;
#endif
	static MState2 mstate2;
#ifndef SWITCH_SUB_ONLY
	event = mstate2.check_columns(event, NUM_SKYCSW+1-1-1+NUM_SKYCSW ) ;
#else
	event = mstate2.check_columns(event, NUM_SKYCSW+1-1-1 ) ;
#endif

#ifdef TOUCH
	uint32_t selected = 0 ;
	uint32_t newVpos ;
#endif

	coord_t y = 0 ;
	uint32_t k = 0 ;
	uint32_t  sub    = mstate2.m_posVert ;
#ifndef SWITCH_SUB_ONLY
	uint32_t subSub = g_posHorz;
#endif
	uint32_t t_pgOfs ;

	t_pgOfs = evalOffset( sub ) ;

#ifdef TOUCH
	if ( handleSelectIcon() )
	{
		selected = 1 ;
	}

	newVpos = scrollBar( 380, 16, 42, 240-16, NUM_SKYCSW-(SCREEN_LINES-1)+1, t_pgOfs ) ;
	if ( newVpos != t_pgOfs )
	{
		s_pgOfs = t_pgOfs = newVpos ;
		if ( sub < t_pgOfs )
		{
			mstate2.m_posVert = sub = t_pgOfs ;
		}
		else if ( sub > t_pgOfs + SCREEN_LINES - 2 )
		{
			mstate2.m_posVert = sub = t_pgOfs + SCREEN_LINES - 2 ;
		}
	}

	if ( TouchControl.itemSelected )
	{
		mstate2.m_posVert = sub = TouchControl.itemSelected + t_pgOfs - 1 ;
	}
#endif

#ifndef SWITCH_SUB_ONLY
	if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		subSub = g_posHorz = saveHpos ;
    SKYCSwData &cs = g_model.customSw[sub];
		
		if ( TextResult )
		{
			if ( subSub == 0 )
			{
				cs.func = SwitchFunctionMap[TextIndex] ;
			}
			else if ( subSub == 1 )
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
#endif

	for(uint32_t i=0; i<SCREEN_LINES-1; i++)
	{
    y = (i+1) * FHPY ;
    k = i + t_pgOfs ;
    LcdFlags attr ;
		uint32_t m = k ;
		if ( k >= NUM_SKYCSW )
		{
			m -= NUM_SKYCSW ;
			Columns = 0 ;
		}
    SKYCSwData &cs = g_model.customSw[m];

		//write SW names here
		displayLogicalSwitch( 0, y, m ) ;
    
#ifndef SWITCH_SUB_ONLY
		attr = (sub==k ? InverseBlink  : 0);
#else
		attr = (sub==k ? INVERS : 0);
#endif
	 if ( k < NUM_SKYCSW )
	 {
#ifndef SWITCH_SUB_ONLY
		PUTS_AT_IDX( 2*FW+1, y, PSTR(CSWITCH_STR),cs.func,subSub==0 ? attr : 0);
#else
ERROR
		PUTS_AT_IDX( 2*FW+1, y, PSTR(CSWITCH_STR),cs.func, 0);
#endif

    uint8_t cstate = CS_STATE(cs.func);

	  if( (cstate == CS_VOFS) || (cstate == CS_2VAL) )
    {
#ifndef SWITCH_SUB_ONLY
			putsChnRaw(    10*FW-6, y, cs.v1u  ,subSub==1 ? attr : 0);
#else
			putsChnRaw(    10*FW-6, y, cs.v1u  , 0);
#endif
	    if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
 			{
				int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
#ifndef SWITCH_SUB_ONLY
				putsTelemetryChannel( 18*FW-8, y, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, subSub==2 ? attr : 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT);
#else
				putsTelemetryChannel( 18*FW-8, y, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT);
#endif
			}
      else
			{
#ifndef SWITCH_SUB_ONLY
        PUTS_NUM( 18*FW-9, y, cs.v2  ,subSub==2 ? attr : 0);
#else
        PUTS_NUM( 18*FW-9, y, cs.v2  ,0);
#endif
			}
    }
    else if(cstate == CS_VBOOL)
    {
#ifndef SWITCH_SUB_ONLY
      putsDrSwitches(10*FW-6, y, cs.v1  ,subSub==1 ? attr : 0);
      putsDrSwitches(14*FW-7, y, cs.v2  ,subSub==2 ? attr : 0);
#else
      putsDrSwitches(10*FW-6, y, cs.v1  , 0);
      putsDrSwitches(14*FW-7, y, cs.v2  , 0);
#endif
    }
    else if(cstate == CS_VCOMP)
    {
#ifndef SWITCH_SUB_ONLY
      putsChnRaw(    10*FW-6, y, cs.v1u  ,subSub==1 ? attr : 0);
      putsChnRaw(    14*FW-4, y, cs.v2u  ,subSub==2 ? attr : 0);
#else
      putsChnRaw(    10*FW-6, y, cs.v1u  , 0);
      putsChnRaw(    14*FW-4, y, cs.v2u  , 0);
#endif
    }
		else if(cstate == CS_TIMER)
		{
			int8_t x ;
			LcdFlags att = 0 ;
			x = cs.v1 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
	    PUTS_ATT_LEFT( y, PSTR(STR_15_ON) ) ;
#ifndef SWITCH_SUB_ONLY
      PUTS_NUM( 13*FW-5, y, x+1  ,att | (subSub==1 ? attr : 0) ) ;
#else
      PUTS_NUM( 13*FW-5, y, x+1  ,att ) ;
#endif
			att = 0 ;
			x = cs.v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
#ifndef SWITCH_SUB_ONLY
      PUTS_NUM( 18*FW-3, y, x+1 , att | (subSub==2 ? attr : 0 ) ) ;
#else
      PUTS_NUM( 18*FW-3, y, x+1 , att ) ;
#endif
		}
		else if(cstate == CS_TMONO)
		{
#ifndef SWITCH_SUB_ONLY
      putsDrSwitches(10*FW-6, y, cs.v1  ,subSub==1 ? attr : 0);
#else
      putsDrSwitches(10*FW-6, y, cs.v1  , 0);
#endif
			LcdFlags att = 0 ;
			int8_t x ;
			x = cs.v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
#ifndef SWITCH_SUB_ONLY
      PUTS_NUM( 17*FW-2, y, x+1 , att | (subSub==2 ? attr : 0 ) ) ;
#else
      PUTS_NUM( 17*FW-2, y, x+1 , att ) ;
#endif
		}
		else// cstate == U16
		{
			uint16_t x ;
			x = cs.v2u ;
			x |= cs.bitAndV3 << 8 ;
#ifndef SWITCH_SUB_ONLY
      putsChnRaw( 10*FW-6-FW, y, cs.v1  ,subSub==1 ? attr : 0);
      PUTS_NUM_N( 18*FW-9, y, x  ,subSub==2 ? attr : 0,5);
#else
      putsChnRaw( 10*FW-6-FW, y, cs.v1  , 0);
      PUTS_NUM_N( 18*FW-9, y, x  , 0,5);
#endif
		}
//    lcd_putc( 19*FW+3, y, cs.andsw ? 'S' : '-') ;
		
#ifndef SWITCH_SUB_ONLY
		putsDrSwitches( 18*FW+2, y, getAndSwitch( cs ),(subSub==3 ? attr : 0)) ;
#else
		putsDrSwitches( 18*FW+2, y, getAndSwitch( cs ),0 ) ;
#endif

#ifdef COLOUR_DISPLAY
		PUTS_P( 22*FW, y, XPSTR("Delay" ) ) ;
		PUTS_NUM( 31*FW, y, g_model.switchDelay[m], PREC1 ) ;
#endif

#ifndef SWITCH_SUB_ONLY
    if((s_editMode /*|| P1values.p1valdiff*/) && attr)
		{
			editOneSwitchItem( event, subSub, sub ) ;
			if ( subSub == 1 )
			{
  			PUT_HEX4( 18*FW, 0*FH, cs.v1u ) ;
			}
		}
#endif			
#ifndef SWITCH_SUB_ONLY
		if ( subSub == 4 )
		{
#endif			
			if ( attr )
			{
#ifndef SWITCH_SUB_ONLY
				lcd_char_inverse( 12, y, 127-12, BLINK ) ;
#else
 #ifdef COLOUR_DISPLAY
 				lcd_char_inverse( 12, y, 31*FW-11, 0 ) ;
 #else
				lcd_char_inverse( 12, y, 127-12, 0 ) ;
				PUTS_P( 102, 0, XPSTR("Dy" ), 0 ) ;
				PUTS_NUM( 21*FW, 0, g_model.switchDelay[m], PREC1 ) ;
 #endif
#endif
#ifdef TOUCH
				if ( ( checkForMenuEncoderBreak( event ) ) || selected )
#else
				if ( checkForMenuEncoderBreak( event ) )
#endif
				{
					// Long MENU pressed
		      s_currIdx = sub ;
  				TextType = 0 ;
					pushMenu(menuSwitchOne) ;
  	  		s_editMode = false ;
				}
			}
#ifndef SWITCH_SUB_ONLY
		}
#endif
	 }
#ifndef SWITCH_SUB_ONLY
	 else
	 {
			PUTS_ATT_LEFT( y, XPSTR("\004delay=" ) ) ;
			PUTS_NUM( 14*FW, y, g_model.switchDelay[m], attr|PREC1 ) ;
			if ( attr )
			{
				CHECK_INCDEC_H_MODELVAR_0( g_model.switchDelay[m], 25 ) ;
			}
	 }
#endif
	}
}

#endif // nTOUCH



//#else	// PROP_TEXT



//#endif	// PROP_TEXT

#if defined(PCBX12D) || defined(PCBX10)
#define MIX_SPACE		(2*FW)
#else
#define MIX_SPACE		0
#endif

//#ifdef SMALL
//#define MIX_OFF		(3*FW)
//#else
#define MIX_OFF		0
//#endif

void menuProcMix(uint8_t event)
{
	TITLE(PSTR(STR_MIXER));
	EditType = EE_MODEL ;
	static MState2 mstate2;
#ifdef TOUCH
extern uint8_t TlExitIcon ;
	TlExitIcon = 1 ;
	uint32_t selected = 0 ;
#endif	
	uint8_t	menulong = 0 ;
  
	switch(event)
  {
	  case EVT_ENTRY:
      s_moveMode=false ;
			if ( s_mixMaxSel < NUM_SKYCHNOUT )
			{
				s_mixMaxSel = NUM_SKYCHNOUT ;
			}
  	break ;
    
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
  	break ;
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
		int32_t v = mstate2.m_posVert ;
		if ( ( ( v == 0 ) && ( event == EVT_KEY_FIRST(KEY_UP) ) ) 
				 || ( ( v == s_mixMaxSel ) && ( event == EVT_KEY_FIRST(KEY_DOWN) ) ) )
		{
			event = 0 ;
		}
		Tevent = event ;
	}
	
	if ( !PopupData.PopupActive )
	{
		mstate2.check_columns(event,s_mixMaxSel-1) ;
	}
  
	uint32_t sub = mstate2.m_posVert ;

	uint32_t t_pgOfs ;
#ifdef COLOUR_DISPLAY
	t_pgOfs = evalHresOffset( sub ) ;
#else
	t_pgOfs = evalOffset( sub ) ;
#endif  
	  
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
		
		newVpos = scrollBar( TSCROLLLEFT, TSCROLLTOP, TSCROLLWIDTH, TSCROLLBOTTOM, s_mixMaxSel-(TLINES-1), t_pgOfs ) ;
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
		
  uint8_t mix_index = 0 ;
  uint8_t current = 0 ;

	if ( s_moveMode )
	{
		int8_t dir ;
		
		if ( ( dir = (event == EVT_KEY_FIRST(KEY_DOWN) ) ) || event == EVT_KEY_FIRST(KEY_UP) )
		{
#ifdef COLOUR_DISPLAY
			uint8_t ch ;
			SKYMixData *src = mixAddress( s_curItemIdx ) ;
			ch = src->destCh ;
#endif
			moveMix( s_curItemIdx, dir ) ; //true=inc=down false=dec=up - Issue 49
#ifdef COLOUR_DISPLAY
			if ( ch != src->destCh )
			{
				// moved channels, correct sub etc.
				if ( dir )
				{
					src = mixAddress( s_curItemIdx-1 ) ;
					if ( ch == src->destCh )
					{
						sub -= 1 ;
					}
				}
				else
				{
					ch = src->destCh ;
					src = mixAddress( s_curItemIdx-1 ) ;
					if ( ch == src->destCh )
					{
						sub += 1 ;
					}
				}
				mstate2.m_posVert = sub ;
			}
#endif
		}
	}

#ifdef COLOUR_DISPLAY
	lcd_hline( 0, TTOP, TMID ) ;
#endif

  for ( uint32_t chan=1 ; chan <= NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ; chan += 1 )
	{
    SKYMixData *pmd = mixAddress( mix_index ) ;
		LcdFlags attr = 0 ;
		if ( !s_moveMode && (sub == current) )
		{
    	attr = INVERS ;
		}
#ifdef COLOUR_DISPLAY
		if ( s_moveMode )
		{
			if ( s_moveItemIdx == mix_index )
			{
				if ( sub == current )
				{
    			attr = INVERS ;
				}
			}
		}
#endif

		coord_t y ;
#ifdef COLOUR_DISPLAY
		y = (current-t_pgOfs)*TFH+TTOP ;
#else
		y = (current-t_pgOfs+1)*FHPY ;
#endif

#ifdef COLOUR_DISPLAY
    if ( t_pgOfs <= current && current-t_pgOfs < TLINES)
#else
    if ( t_pgOfs <= current && current-t_pgOfs < SCREEN_LINES-1)
#endif
		{
#ifdef COLOUR_DISPLAY
			if ( y+TFH != TTOP)
			{
				lcd_hline( 0, y+TFH, TMID ) ;
			}
			lcdDrawSolidFilledRectDMA( 0, ((current-t_pgOfs)*TFH+TTOP)*TSCALE+2, (TMID)*TSCALE, TFH*TSCALE-2, attr ? ~DimBackColour : DimBackColour ) ;
			saveEditColours( attr, DimBackColour ) ;
      putsChn(1, (current-t_pgOfs)*TFH+TTOP+TVOFF, chan, 0) ; // show CHx
			restoreEditColours() ;
#else
      putsChn(1, (current-t_pgOfs+1)*FHPY, chan, 0) ; // show CHx
#endif
    }

		uint8_t firstMix = mix_index ;
		
//#if EXTRA_SKYMIXERS
		if (mix_index < MAX_SKYMIXERS + EXTRA_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)
//#else
//		if (mix_index<MAX_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)
//#endif
		{
    	do
			{
				if (t_pgOfs <= current )
				{
#ifdef COLOUR_DISPLAY
					if ( current-t_pgOfs < TLINES )
#else
					if ( current-t_pgOfs < SCREEN_LINES-1 )
#endif
					{
						if ( !s_moveMode && (sub == current) )
						{
							s_curItemIdx = mix_index ;
							s_currDestCh = chan ;		// For insert
							if ( menulong )
							{
								PopupData.PopupIdx = 0 ;
								PopupData.PopupActive = 1 ;
								event = 0 ;		// Kill this off
							}
						}
						    	  
						if(firstMix != mix_index) //show prefix only if not first mix
						{
#ifdef COLOUR_DISPLAY
							y += TFH ;
				    	attr = (sub == current) ? INVERS : 0 ;
							lcdDrawSolidFilledRectDMA( 0, ((current-t_pgOfs)*TFH+TTOP)*TSCALE+2, (TMID)*TSCALE, TFH*TSCALE-2, attr ? ~DimBackColour : DimBackColour ) ;
							lcd_hline( 0, y+TFH, TMID ) ;
							saveEditColours( attr, DimBackColour ) ;
        	 		PUTS_AT_IDX( 4*FW+2, y+TVOFF, XPSTR("\001+*R"),pmd->mltpx,0 ) ;
							restoreEditColours() ;
#else
        	 		PUTS_AT_IDX( 4*FW-8, y, XPSTR("\001+*R"),pmd->mltpx,0 ) ;
#endif
						}

#ifdef COLOUR_DISPLAY
						saveEditColours( attr, DimBackColour ) ;
						putsChnOpRaw( (8*FW+MIX_SPACE)*2 ,(y+TVOFF)*2, pmd->srcRaw, pmd->switchSource, pmd->disableExpoDr, 0 ) ;
						restoreEditColours() ;
#else
						putsChnOpRaw( (8*FW+MIX_SPACE) ,y, pmd->srcRaw, pmd->switchSource, pmd->disableExpoDr, attr ) ;
#endif
						if ( attr )
						{
							int32_t value = g_chans512[chan-1] ;
#ifdef COLOUR_DISPLAY
							singleBar( 100+4*FW, 4, value ) ;
					    PUTS_NUMX( 11*FW+4*FW, 0, value/2 + 1500 ) ;
#else
							singleBar( 96, 4, value ) ;
					    PUTS_NUMX( 11*FW, 0, value/2 + 1500 ) ;
#endif
						}
//						putsChnOpRaw( 8*FW, y, pmd, 0 ) ;
extern uint8_t swOn[] ;
#ifdef COLOUR_DISPLAY
#else
						attr = 0 ;
#endif
						if ( swOn[mix_index] || pmd->srcRaw == MIX_MAX || pmd->srcRaw == MIX_FULL )
						{
							attr |= BOLD ;
						}
						int16_t lweight = calcExtendedValue( pmd->weight, pmd->extWeight ) ;
#ifdef USE_VARS
						if ( pmd->varForWeight )
						{
#if defined(PCBX12D) || defined(PCBX10)
							saveEditColours( attr, DimBackColour ) ;
							displayVarName( 4*FW, y+TVOFF, lweight, 0 ) ;
							restoreEditColours() ;
#else
							displayVarName( 4*FW, y, lweight, attr | LUA_SMLSIZE ) ;
#endif
						}
						else
#endif
						{
#ifdef COLOUR_DISPLAY
							saveEditColours( attr, DimBackColour ) ;
							gvarMenuItem( 7*FW+FW/2, y+TVOFF, lweight, -125, 125, (attr & ~INVERS) | GVAR_250, 0 ) ;
							restoreEditColours() ;
#else
							gvarMenuItem( 7*FW+FW/2, y, lweight, -125, 125, attr | GVAR_250, 0 ) ;
#endif
						}

#ifndef PCBX12D
 #ifndef PCBX10
					 if ( g_eeGeneral.altMixMenu )
 #endif
#endif
					 {
#if defined(PCBX12D) || defined(PCBX10)
					 	uint16_t x = 128-76 ;
#else
					 	uint16_t x = 0 ;
#endif
						if (sub == current)
						{
#if defined(PCBX12D) || defined(PCBX10)
							pushPlotType( PLOT_COLOUR ) ;
#endif
							lcd_vline( 78+x, 8, 7*FHPY ) ;
#if defined(PCBX12D) || defined(PCBX10)
							popPlotType() ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
							PUTS_P( 13*FW+2+x, FHPY, XPSTR("Ofst") ) ;
#else
							PUTS_P( 13*FW+2+x, FHPY, XPSTR("Of") ) ;
#endif
							if ( pmd->varForOffset )
							{
								int16_t loffset = calcExtendedValue( pmd->sOffset, pmd->extOffset ) ;
#if defined(PCBX12D) || defined(PCBX10)
								displayVarName( FW*18+1+x, FHPY, loffset, 0 ) ;
#else
								displayVarName( FW*33/2-1+x, FHPY, loffset, LUA_SMLSIZE ) ;
#endif
							}
							else
							{
								extendedValueEdit( pmd->sOffset, pmd->extOffset, 0, FHPY, 0, FW*21+1+x ) ;
							}
#if defined(PCBX12D) || defined(PCBX10)
							PUTS_P( 13*FW+2+x, 2*FHPY, XPSTR("Sw") ) ;
#else
							PUTS_P( 13*FW+2+x, 2*FHPY, XPSTR("Sw") ) ;
#endif
	            putsDrSwitches( 21*FW+1+x, 2*FHPY, pmd->swtch, LUA_RIGHT ) ;

// ??? Curve setting, Flight modes
							
						 	uint8_t value = pmd->differential ;
							if ( value == 0 )
							{
								if ( pmd->curve <= -28 )
								{
									value = 2 ;		// Expo
								}
							}
//							if ( value || pmd->curve )
//							{
#if defined(PCBX12D) || defined(PCBX10)
							  PUTS_AT_IDX( 13*FW+2+x+MIX_OFF, 3*FHPY, XPSTR("\003CveDifExp"), value, 0 ) ;
//								PUTS_AT_IDX( 13*FW+2+x, 3*FHPY, XPSTR("\003CveDifExp"), value, 0 ) ;
#else
								PUTS_AT_IDX( 13*FW+2+x, 3*FHPY, XPSTR("\003CveDifExp"), value, 0 ) ;
#endif
								switch ( value )
								{
									case 0 :
										put_curve( 21*FW+1+x+MIX_OFF, 3*FHPY, pmd->curve, LUA_RIGHT ) ;
									break ;
									case 1 :
									{	
										int16_t ivalue = pmd->curve ;
										if ( pmd->extDiff == 0 )	// old setup
										{
											if (ivalue >= 126 || ivalue <= -126)
											{
												// Gvar
												ivalue = (uint8_t)ivalue - 126 ; // 0 to 4
												ivalue += 510 ;
											}
										}
										else
										{
											ivalue += 510 ;
										}
										gvarDiffValue( FW*21+1+x, 3*FHPY, ivalue, 0, 0 ) ;
//				      	    gvarMenuItem( FW*21+1+x, 3*FH, pmd->curve, -100, 100, 0, 0 ) ;
									}
									break ;
									case 2 :
#if defined(PCBX12D) || defined(PCBX10)
	            			PUTS_NUM( FW*21+1+x, 3*FHPY, pmd->curve + 128, 0 ) ;
#else
	            			PUTS_NUM( FW*21+1+x, 3*FHPY, pmd->curve + 128, 0 ) ;
#endif
									break ;
								}
//							}
							uint8_t b = 1 ;
							uint8_t z = pmd->modeControl ;
#if defined(PCBX12D) || defined(PCBX10)
							PUTS_P( 13*FW+2+x, 4*FHPY, XPSTR("M") ) ;
//							PUTS_P( 13*FW+2+x, 4*FHPY, XPSTR("M") ) ;
#endif
							uint32_t modeCount = 0 ;
							uint32_t modeIndex = 0 ;
							for ( uint32_t p = 0 ; p<MAX_MODES+2 ; p++ )
							{
								if ( ( z & b ) == 0 )
								{
#if defined(PCBX12D) || defined(PCBX10)
									PUTC_ATT( (14+p)*FW+4+x, 4*FHPY, '0'+p, 0 ) ;
//    							PUTC_ATT( (14+p)*FW+2+x, 4*FHPY, '0'+p, 0 ) ;
#else
    							PUTC_ATT( (13+p)*FW+2+x, 4*FHPY, '0'+p, 0 ) ;
#endif
									modeCount += 1 ;
									modeIndex = p ;
								}
								b <<= 1 ;
							}
							if ( modeCount == 1 )
							{
								if ( modeIndex )
								{
									modeIndex -= 1 ;
  								PhaseData *phase ;
									phase = (modeIndex < MAX_MODES) ? &g_model.phaseData[modeIndex] : &g_model.xphaseData ;
									
									if ( phase->name[0] )
									{
										PUTS_P( 13*FW+2+x, 6*FHPY, XPSTR("[\223\223\223\223\223\223]") ) ;
										PUTS_ATT_N( 14*FW+2+x, 5*FHPY, phase->name, 6, /*BSS*/ 0 ) ;
									}
								}
							}
#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
							PUTS_P( 13*FW+2+x, 6*FHPY, XPSTR("D") ) ;
						  PUTS_NUM( FW*18+2+x, 6*FHPY, pmd->delayUp, PREC1 ) ;
							PUTC( (LcdLastRightPos+2)/HVSCALE, 6*FHPY, ':' ) ;
						  PUTS_NUM( FW*21+1+x, 6*FHPY, pmd->delayDown, PREC1 ) ;

							PUTS_P( 13*FW+2+x, 7*FHPY, XPSTR("S") ) ;
						  PUTS_NUM( FW*18+2+x, 7*FHPY, pmd->speedDown, PREC1 ) ;
							PUTC( (LcdLastRightPos+2)/HVSCALE, 7*FHPY, ':' ) ;
						  PUTS_NUM( FW*21+1+x, 7*FHPY, pmd->speedUp, PREC1 ) ;
#else
							PUTS_P( 13*FW+2+x, 6*FHPY, XPSTR("D    :") ) ;
						  PUTS_NUM( FW*18+2+x, 6*FHPY, pmd->delayUp, PREC1 ) ;
						  PUTS_NUM( FW*21+1+x, 6*FHPY, pmd->delayDown, PREC1 ) ;

							PUTS_P( 13*FW+2+x, 7*FHPY, XPSTR("S    :") ) ;
						  PUTS_NUM( FW*18+2+x, 7*FHPY, pmd->speedDown, PREC1 ) ;
						  PUTS_NUM( FW*21+1+x, 7*FHPY, pmd->speedUp, PREC1 ) ;
#endif
						}					 	
						if ( s_moveMode )
						{
							if ( s_moveItemIdx == mix_index )
							{
#ifdef COLOUR_DISPLAY
								lcdHiresRect( (4*FW)*2, (y+1)*2, (TMID-4*FW-2)*2, (TFH-1)*2, DimBackColour ) ;
								lcdHiresRect( (4*FW)*2+1, (y+1)*2+1, (TMID-4*FW-2)*2-2, (TFH-1)*2-2, LCD_BLACK ) ; //DimBackColour ) ;
#else
								lcd_char_inverse( 4*FW+x, y, 76-4*FW, 0 ) ;
#endif
								s_curItemIdx = mix_index ;
								sub = mstate2.m_posVert = current ;
							}
						}
					 }
#ifndef PCBX12D
#ifndef PCBX10
					 else
#endif
#endif
					 {
    	  	  if( pmd->swtch) putsDrSwitches( 14*FW, y, pmd->swtch, 0 ) ; //tattr);
						if(pmd->curve)
						{
							if ( pmd->differential ) PUTC_ATT(    16*FW, y, *PSTR(CHR_d), 0 ) ;
							else
							{
								if ( ( pmd->curve > -28 ) && ( pmd->curve <= 27 ) )
								{
	    	  	  		put_curve( 17*FW, y, pmd->curve, 0 ) ;
								}
								else
								{
									if ( pmd->curve != -128 )
									{
										PUTC_ATT( 17*FW, y, 'E', 0 ) ;
									}
								}
							}
						}
						char cs = ' ';
        	  if (pmd->speedDown || pmd->speedUp)
        	    cs = *PSTR(CHR_S);
        	  if (pmd->delayUp || pmd->delayDown)
        	    cs = (cs == *PSTR(CHR_S) ? '*' : *PSTR(CHR_D));
        	  PUTC(20*FW+1, y, cs ) ;

						if ( s_moveMode )
						{
							if ( s_moveItemIdx == mix_index )
							{
#ifdef COLOUR_DISPLAY
								lcdHiresRect( (4*FW)*2, (y+1)*2, (TMID-4*FW-2)*2, (TFH-1)*2, DimBackColour ) ;
								lcdHiresRect( (4*FW)*2+1, (y+1)*2+1, (TMID-4*FW-2)*2-2, (TFH-1)*2-2, DimBackColour ) ;
#else
								lcd_char_inverse( 4*FW, y, 17*FW, 0 ) ;
#endif
								s_curItemIdx = mix_index ;
								sub = mstate2.m_posVert = current ;
							}
						}
					 }
					}
					else
					{
						if ( current-t_pgOfs == 7 )
						{
							if ( s_moveMode )
							{
								if ( s_moveItemIdx == mix_index )
								{
									mstate2.m_posVert += 1 ;								
								}
							}
						}
					}
				}
				current += 1 ;
				mix_index += 1 ;
		    pmd = mixAddress( mix_index ) ;
//#if EXTRA_SKYMIXERS
    	} while ( (mix_index<MAX_SKYMIXERS+EXTRA_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)) ;
//#else
//    	} while ( (mix_index<MAX_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)) ;
//#endif
		}
		else
		{
			if (sub == current)
			{
				s_currDestCh = chan ;		// For insert
				s_curItemIdx = mix_index ;
#ifdef COLOUR_DISPLAY
//				lcd_rect( 0, (current-t_pgOfs)*TFH+TTOP, 25, 10 ) ;
#else
				lcd_rect( 0, (current-t_pgOfs+1)*FHPY-1, 25, 9 ) ;
#endif
				if ( menulong )		// Must need to insert here
				{
      		if ( !reachMixerCountLimit())
      		{
      			insertMix(s_curItemIdx, 0 ) ;
  	    		s_moveMode=false;
	      		pushMenu(menuMixOne) ;
						break ;
      		}
				}
			}
			current += 1 ;
		}
	}
	if ( PopupData.PopupActive )
	{
		Tevent = event ;
		mixpopup( event ) ;
    s_editMode = false;
	}
	s_mixMaxSel = current ;

}

