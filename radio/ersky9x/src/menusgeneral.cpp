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

#include "sticks.lbm"

#ifdef TOUCH
extern uint8_t TouchUpdated ;
#endif

#ifdef TOUCH
extern uint8_t TlExitIcon ;
extern uint8_t BlockTouch ;
#endif
#ifdef COLOUR_DISPLAY
void menuControls(uint8_t event) ;
void menuGeneral( uint8_t event ) ;
void menuAlarms( uint8_t event ) ;
void menuAudio( uint8_t event ) ;
void menuDisplay( uint8_t event ) ;
#endif


#ifdef BLUETOOTH
#define BtBaudString "\006115200  9600 19200 57600 38400"
void menuProcBtConfigure(uint8_t event) ;
void menuProcBtScan(uint8_t event) ;
void menuProcBtAddressList(uint8_t event) ;
#endif

#ifdef PCBSKY
void edit_xpot_source( coord_t y, const char *s, uint8_t i, uint8_t a) ;
#endif

#if defined(PCBSKY) || defined(PCB9XT)
void menuConfigPots(uint8_t event) ;
#endif

#if defined(X9LS) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
//#if defined(X9LS)
const uint8_t BtFunctionMap[] = { 0,4,5,6 } ;
#else
const uint8_t BtFunctionMap[] = { 0,4,1,2,3,5,6 } ;
#endif

void edit_stick_gain( coord_t x, coord_t y, const char *s, uint8_t stick_b, uint8_t edit) ;
void edit_stick_deadband( coord_t x, coord_t y, const char *s, uint8_t ch, uint8_t edit) ;

extern const uint8_t GvaString[] ;

extern uint8_t SubmenuIndex ;
extern uint8_t IlinesCount ;
extern uint8_t LastSubmenuIndex ;
extern uint8_t UseLastSubmenuIndex ;
extern uint8_t InverseBlink ;
extern int32_t Rotary_diff ;
extern uint8_t SubMenuCall ;
extern uint8_t AlphaEdited ;

const char Str_General[] =     "General" ;
			
enum GENERAL_INDEX
{
	M_INDEX,
	M_DISPLAY,
	M_AUDIO,
	M_ALARMS,
	M_GENERAL,
	M_CONTROLS,
	M_HARDWARE,
	M_CALIB,
#ifdef BLUETOOTH
	M_BLUETOOTH,
#endif
	M_TRAINER,
	M_VERSION,
#ifndef PCBXLITE
 #ifndef PCBT12
  #ifndef PCBX9LITE
   #ifndef PCBX12D
    #ifndef PCBX10
     #ifndef PCBLEM1
	M_MODULE,
     #endif // PCBLEM1
    #endif // X10
   #endif // X12
  #endif // X9Lite
 #endif
#endif
	M_EEPROM,
	M_DATE,
	M_DIAGKEYS,
	M_DIAGANA,
	M_COUNT
} ;


#if defined(PCBX12D) || defined(PCBX10)
const uint8_t *g_icons[] = {
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
};
#endif


#if defined(PCBSKY) || defined(PCB9XT)
//const char SW_3_IDX[] = "\004sIDxsTHRsRUDsELEsAILsGEAsTRNL1  L2  L3  L4  L5  L6  L7  L8  L9  LA  LB  LC  LD  LE  LF  LG  LH  LI  LJ  LK  LL  LM  LN  LO  6POS" ;
#define NUM_MIX_SWITCHES	(7+NUM_SKYCSW)
#endif
#ifdef PCBX9D
 #ifdef REV9E
#define NUM_MIX_SWITCHES	(19+NUM_SKYCSW)
 #else // X9E
  #ifdef PCBX7
#define NUM_MIX_SWITCHES	(6+NUM_SKYCSW)
  #else // PCBX7
   #ifdef PCBX9LITE
#define NUM_MIX_SWITCHES	(5+NUM_SKYCSW)
   #else // PCBX9LITE
    #if defined(REVPLUS) || defined(REVNORM) || defined(REV19)
#define NUM_MIX_SWITCHES	(8+NUM_SKYCSW)
    #endif // norm/plus
		#ifdef PCBXLITE
#define NUM_MIX_SWITCHES	(4+NUM_SKYCSW)
    #endif // Xlite
   #endif // PCBX9LITE
  #endif // PCBX7
 #endif	// REV9E
#endif // X9D

#if defined(PCBX12D) || defined(PCBX10)
#define NUM_MIX_SWITCHES	(9+NUM_SKYCSW)
#endif

#if defined(PCBLEM1)
#define NUM_MIX_SWITCHES	(6+NUM_SKYCSW)
#endif


#ifdef COLOUR_DISPLAY
#include "menusRadioColour.cpp"
#endif


void menuProcIndex(uint8_t event)
{
	static MState2 mstate;
	EditType = EE_GENERAL ;

#ifdef TOUCH
	TlExitIcon = 1 ;
#endif
	event = indexProcess( event, &mstate, M_COUNT-1-7 ) ;
//#ifdef TOUCH
//	if ( SubmenuIndex )
//	{
//		TlExitIcon = 0 ;		
//	}
//#endif

//#if defined(PCBT12)
//	event = indexProcess( event, &mstate, 6 ) ;
//#else
// #if defined (PCBXLITE)
//	event = indexProcess( event, &mstate, 7 ) ;
// #else
//	event = indexProcess( event, &mstate, 8 ) ;
// #endif		 
//#endif		 
	event = mstate.check_columns( event, IlinesCount-1 ) ;

		switch ( SubmenuIndex )
		{
			case M_CALIB :
        pushMenu(menuProcDiagCalib) ;
			break ;
			case M_TRAINER :
        pushMenu(menuProcTrainer) ;
			break ;
			case M_VERSION :
        pushMenu(menuProcDiagVers) ;
			break ;
			case M_DATE :
        pushMenu(menuProcDate) ;
			break ;
#ifndef PCBXLITE	
 #ifndef PCBT12
  #ifndef PCBX9LITE
   #ifndef PCBX12D
    #ifndef PCBX10
      #ifndef PCBLEM1
			case M_MODULE :
        pushMenu(menuProcRSSI) ;
			break ;
     #endif
    #endif
   #endif
  #endif // X3
 #endif
#endif
			case M_DIAGKEYS :
        pushMenu(menuProcDiagKeys) ;
			break ;
			case M_DIAGANA :
        pushMenu(menuProcDiagAna) ;
			break ;
			default :
				if ( event == EVT_ENTRY )
				{
					audioDefevent(AU_MENUS) ;
					event = 0 ;
				}
			break ;
		}

	uint32_t sub = mstate.m_posVert ;
	coord_t y = FHPY ;
	uint8_t blink = InverseBlink ;

	switch ( SubmenuIndex )
	{
		case 0 :
#if defined(PCBX12D) || defined(PCBX10)
//			lcd_putsAtt(0 + X12OFFSET-33,0,"Radio Setup",INVERS|DBLSIZE|CONDENSED) ;
			PUTS_ATT(0 + X12OFFSET+36,0,"Radio Setup",INVERS) ;
#else
			PUTS_ATT(0 + X12OFFSET,0,"Radio Setup",INVERS) ;
#endif
			IlinesCount = M_COUNT-1 ;

//#ifdef PCBT12
//			IlinesCount = 13 ;
//#else
// #ifdef PCBXLITE	 
//			IlinesCount = 14 ;
// #else
//			IlinesCount = 15 ;
// #endif
//#endif
			sub += 1 ;
			
			
static const uint16_t in_Strings[] = {
STR_Display,
STR_AudioHaptic,
STR_Alarms,
STR_General,
STR_Controls,
STR_Hardware,
STR_Calibration,
#ifdef BLUETOOTH
STR_Bluetooth,
#endif
STR_Trainer,
STR_Version,
#ifndef PCBXLITE	
 #ifndef PCBT12
  #ifndef PCBX9LITE
   #ifndef PCBX12D
    #ifndef PCBX10
     #ifndef PCBLEM1
STR_ModuleRssi,
     #endif // PCBLEM1
    #endif // X1
   #endif // X12
  #endif // X3
 #endif
#endif
STR_Eeprom,
STR_DateTime,
STR_DiagSwtch,
STR_DiagAna
};
	

			displayIndex( in_Strings, M_COUNT-1-7, 7, sub ) ;
#if defined(PCBX12D) || defined(PCBX10)
			displayIndexIcons( g_icons, M_COUNT-1-7, 7 ) ;
#endif

//#if defined(PCBT12)
//			displayIndex( in_Strings, 6, 7, sub ) ;
//#else
// #if defined (PCBXLITE)
//			displayIndex( in_Strings, 7, 7, sub ) ;
// #else
//			displayIndex( in_Strings, 8, 7, sub ) ;
// #endif		 
//#endif		 
		break ;

		case M_DISPLAY :
#ifdef COLOUR_DISPLAY
			pushMenu( menuDisplay	) ;
#else			 
		{	
			uint32_t subN = 0 ;
			
      TITLE( PSTR(STR_Display) ) ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
#if defined(REVPLUS) || defined(REV9E)
			IlinesCount = 7 ;
 #else            
  #if defined(PCBX12D) || defined(PCBX10)
			IlinesCount = 10 ;
  #else            
			IlinesCount = 6 ;
  #endif            
 #endif            
#else            
 #ifdef PCBSKY
 			y = 0 ;
  #ifndef REVX
			IlinesCount = 8 ;
			if ( g_eeGeneral.ar9xBoard )
			{
				IlinesCount -= 1 ;
			}
  #else            
			IlinesCount = 8 ;
  #endif            
 #endif            
#endif            
#ifdef PCBLEM1
			IlinesCount = 6 ;
#endif            

#ifdef PCB9XT
 			y = 0 ;
			IlinesCount = 9 ;

		 if ( sub < 7 )
		 {
			displayNext() ;
		 	
#endif            
  		LcdFlags attr ;
			attr = (sub==subN) ? blink : 0 ;
      if( attr )
			{
				uint8_t oldValue = g_eeGeneral.contrast ;
#if defined(REVPLUS) || defined(REV9E)
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, 0, 60);
#else
 #if defined(PCBX7) || defined(PCBX9LITE)
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, 10, 42);
 #else
  #ifdef PCBLEM1
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, 5, 20);
	#else
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, 10, 45);
  #endif
 #endif
#endif
				if ( g_eeGeneral.contrast != oldValue )
				{
					lcdSetRefVolt(g_eeGeneral.contrast);
				}
      }
#if defined(PCBSKY) || defined(PCB9XT)
      PUTS_P( 8*FW, y, PSTR(STR_CONTRAST) ) ;
#else
      PUTS_ATT_LEFT( y, PSTR(STR_CONTRAST) ) ;
#endif
      PUTS_NUM( PARAM_OFS+2*FW, y, g_eeGeneral.contrast, (sub==subN) ? blink : 0) ;
			y += FHPY ;
			subN += 1 ;
  		
			PUTS_ATT_LEFT( y, PSTR(STR_BRIGHTNESS)) ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
#if defined(REVPLUS) || defined(REV9E)
		  PUTS_P(11*FW, y, XPSTR( "COLOUR"));
#endif
#endif
#ifdef PCB9XT
		  PUTS_P(11*FW, y, XPSTR( "RED"));
			uint8_t oldBrightness = g_eeGeneral.bright ;
			int8_t b ;
			b = g_eeGeneral.bright ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			attr = 0 ;
      if(sub==subN)
			{
				attr = INVERS ;
			}
			b = gvarMenuItem(PARAM_OFS+2*FW, y, b, 0, 100, attr | GVAR_100, event ) ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			g_eeGeneral.bright = b ;

#else
			 
			PUTS_NUM( PARAM_OFS+2*FW, y, 100-g_eeGeneral.bright, (sub==subN) ? blink : 0 ) ;
      if(sub==subN)
			{
				uint8_t b ;
				b = 100 - g_eeGeneral.bright ;
				CHECK_INCDEC_H_GENVAR_0( b, 100 ) ;
				g_eeGeneral.bright = 100 - b ;
#endif
#ifdef PCBSKY
				PWM->PWM_CH_NUM[0].PWM_CDTYUPD = g_eeGeneral.bright ;
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
#if defined(REVPLUS) || defined(REV9E) || defined(REV19)
				backlight_set( g_eeGeneral.bright, 0 ) ;
#else
				backlight_set( g_eeGeneral.bright ) ;
#endif
#endif
#ifdef PCB9XT
				if ( oldBrightness != g_eeGeneral.bright )
				{
					uint8_t b ;
					b = g_eeGeneral.bright ;
					if ( b < 101 )
					{
						b = 100 - b ;
					}
					BlSetColour( b, 0 ) ;
				}
#else
			}
#endif
			y += FHPY ;
			subN += 1 ;

#if defined(REVPLUS) || defined(PCB9XT) || defined(REV9E)
#ifndef PCB9XT
 			PUTS_ATT_LEFT( y, PSTR(STR_BRIGHTNESS));
#endif
#ifdef PCB9XT
   		PUTS_P(11*FW, y, XPSTR( "GREEN"));
			oldBrightness = g_eeGeneral.bright_white ;
			b = g_eeGeneral.bright_white ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			attr = 0 ;
      if(sub==subN)
			{
				attr = INVERS ;
			}
			b = gvarMenuItem(PARAM_OFS+2*FW, y, b, 0, 100, attr | GVAR_100, event ) ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			g_eeGeneral.bright_white = b ;
#else
   		PUTS_P(11*FW, y, XPSTR( "WHITE"));
			PUTS_NUM( PARAM_OFS+2*FW, y, 100-g_eeGeneral.bright_white, (sub==subN) ? blink : 0 ) ;
      if(sub==subN)
			{
				uint8_t b ;
				b = 100 - g_eeGeneral.bright_white ;
				CHECK_INCDEC_H_GENVAR_0( b, 100 ) ;
				g_eeGeneral.bright_white = 100 - b ;
#endif
#ifdef PCB9XT
				if ( oldBrightness != g_eeGeneral.bright_white )
				{
					uint8_t b ;
					b = g_eeGeneral.bright_white ;
					if ( b < 101 )
					{
						b = 100 - b ;
					}
					BlSetColour( b, 1 ) ;
				}
#else
				backlight_set( g_eeGeneral.bright_white, 1 ) ;
			}
#endif
			y += FHPY ;
			subN++;
#endif
#ifdef PCB9XT
   		PUTS_P( 11*FW, y, XPSTR( "BLUE"));
			oldBrightness = g_eeGeneral.bright_blue ;
			b = g_eeGeneral.bright_blue ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			attr = 0 ;
      if(sub==subN)
			{
				attr = INVERS ;
			}
			b = gvarMenuItem(PARAM_OFS+2*FW, y, b, 0, 100, attr | GVAR_100, event ) ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			g_eeGeneral.bright_blue = b ;
			if ( oldBrightness != g_eeGeneral.bright_blue )
			{
				uint8_t b ;
				b = g_eeGeneral.bright_blue ;
				if ( b < 101 )
				{
					b = 100 - b ;
				}
				BlSetColour( b, 2 ) ;
			}
			y += FHPY ;
			subN++;
#endif

#if defined(PCBX12D) || defined(PCBX10)
			PUTS_ATT_LEFT( y,XPSTR("Off brightness"));
			PUTS_NUM( PARAM_OFS+2*FW, y, 100-g_eeGeneral.bright_white, (sub==subN) ? blink : 0 ) ;
      if(sub==subN)
			{
				uint8_t b ;
				b = 100 - g_eeGeneral.bright_white ;
				CHECK_INCDEC_H_GENVAR_0( b, 100 ) ;
				g_eeGeneral.bright_white = 100 - b ;
			}
			y += FHPY ;
			subN++;
#endif
      
			PUTS_ATT_LEFT( y,PSTR(STR_LIGHT_SWITCH));
#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
      putsDrSwitches(PARAM_OFS+2*FW,y,g_eeGeneral.lightSw,(sub==subN ? blink : 0)|LUA_RIGHT);
#else
      putsDrSwitches(PARAM_OFS-FW,y,g_eeGeneral.lightSw,(sub==subN ? blink : 0));
#endif
      if(sub==subN) CHECK_INCDEC_GENERALSWITCH(event, g_eeGeneral.lightSw, -MaxSwitchIndex, MaxSwitchIndex) ;
			
			y += FHPY ;
			subN++;

			for ( uint32_t i = 0 ; i < 2 ; i += 1 )
			{
				uint8_t b ;
    	  PUTS_ATT_LEFT( y,( i == 0) ? PSTR(STR_LIGHT_AFTER) : PSTR(STR_LIGHT_STICK) );
				b = ( i == 0 ) ? g_eeGeneral.lightAutoOff : g_eeGeneral.lightOnStickMove ;

    	  if(b)
				{
#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
   	      PUTS_NUM(PARAM_OFS+2*FW, y, b*5, (sub==subN ? blink : 0));
   	      PUTC(LcdLastRightPos, y*HVSCALE, 's');
#else
					PUTS_NUM(PARAM_OFS, y, b*5,LEFT|(sub==subN ? blink : 0));
					PUTC(Lcd_lastPos, y, 's');
#endif
    	  }
    	  else
				{
					PUTS_ATT(PARAM_OFS+2*FW, y, PSTR(STR_OFF),(sub==subN ? blink:0)|LUA_RIGHT) ;
				}
    	  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( b, 600/5);
				if ( i == 0 )
				{
					g_eeGeneral.lightAutoOff = b ;
				}
				else
				{
					g_eeGeneral.lightOnStickMove = b ;
				}
  			y += FHPY ;
				subN += 1 ;
			}
		
#ifdef PCBSKY
#ifndef REVX
			if ( g_eeGeneral.ar9xBoard == 0 )
			{
		  	g_eeGeneral.optrexDisplay = onoffMenuItem( g_eeGeneral.optrexDisplay, y, XPSTR("Optrex Display"), sub==subN ) ;
  			y += FHPY ;
				subN += 1 ;
			}
#endif
#endif

#ifndef PCB9XT
			g_eeGeneral.flashBeep = onoffMenuItem( g_eeGeneral.flashBeep, y, PSTR(STR_FLASH_ON_BEEP), sub==subN ) ;
  		y += FHPY ;
			subN += 1 ;
#endif
#ifdef PCBSKY
			{
				uint8_t b = g_eeGeneral.rotateScreen ;
	  		g_eeGeneral.rotateScreen = onoffMenuItem( b, y, PSTR(STR_ROTATE_SCREEN), sub==subN ) ;
				if ( g_eeGeneral.rotateScreen != b )
				{
					lcdSetOrientation() ;
				}
#ifdef REVX
	  		y += FHPY ;
				subN += 1 ;
				b = g_eeGeneral.reverseScreen ;
	  		g_eeGeneral.reverseScreen = onoffMenuItem( b, y, PSTR(STR_REVERSE_SCREEN), sub==subN ) ;
				if ( g_eeGeneral.reverseScreen != b )
				{
					lcdSetOrientation() ;
				}
#endif
			}
#endif
#ifdef PCB9XT
		 }
		 else
		 {
				int8_t b ;
			 	subN = 7 ;
				y = FHPY ;
				g_eeGeneral.flashBeep = onoffMenuItem( g_eeGeneral.flashBeep, y, PSTR(STR_FLASH_ON_BEEP), sub==subN ) ;
  			y += FHPY ;
				subN += 1 ;
				b = g_eeGeneral.reverseScreen ;
	  		g_eeGeneral.reverseScreen = onoffMenuItem( b, y, PSTR(STR_REVERSE_SCREEN), sub==subN ) ;
				if ( g_eeGeneral.reverseScreen != b )
				{
					lcdSetOrientation() ;
				}
		 }
#endif
#if defined(PCBX12D) || defined(PCBX10)
			attr = (sub==subN) ? blink : 0 ;
			PUTS_ATT( 0, y, "Background Colour", attr ) ;
      if( sub==subN )
			{
#ifdef TOUCH		
				if ( handleSelectIcon() )
				{
					if ( event == 0 )
					{
						event = EVT_KEY_BREAK(BTN_RE) ;
					}
				}
#endif
				if ( checkForMenuEncoderBreak( Tevent ) )
				{
					SubMenuCall = 0x87 ;
					pushMenu( menuBackground ) ;
				}
			}
  		y += FHPY ;
			subN += 1 ;
			attr = (sub==subN) ? blink : 0 ;
			PUTS_ATT( 0, y, "Text Colour", attr ) ;
      if( sub==subN )
			{
#ifdef TOUCH		
				if ( handleSelectIcon() )
				{
					if ( event == 0 )
					{
						event = EVT_KEY_BREAK(BTN_RE) ;
					}
				}
#endif
				if ( checkForMenuEncoderBreak( Tevent ) )
				{
					SubMenuCall = 0x88 ;
					pushMenu( menuForeground ) ;
				}
			}
 			y += FHPY ;
			subN += 1 ;

			attr = (sub==subN) ? INVERS : 0 ;
			PUTS_P( 0, y, XPSTR("Theme") ) ;
			PUTS_NUM( PARAM_OFS+2*FW, y, g_eeGeneral.selectedTheme, (sub==subN) ? blink : 0 ) ;
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
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
#if defined(REVPLUS) || defined(REV9E) || defined(REV19)
				backlight_set( g_eeGeneral.bright, 0 ) ;
#else
				backlight_set( g_eeGeneral.bright ) ;
#endif
#endif
//					dimBackColour() ;
				}
			}

#endif
			 
		}
#endif // TOUCH
		break ;

#ifdef COLOUR_DISPLAY
#define RAUD_OFF_0			0
#else
#define RAUD_OFF_0			0
#endif
		 
		case M_AUDIO :
#ifdef COLOUR_DISPLAY
			pushMenu( menuAudio	) ;
#else			 
		{	
			uint8_t current_volume ;
			uint8_t subN = 0 ;
#if defined(PCBX12D) || defined(PCBX10)
 #ifdef HAPTIC
			IlinesCount = g_eeGeneral.welcomeType == 2 ? 11 : 10 ;
 #else
			IlinesCount = g_eeGeneral.welcomeType == 2 ? 9 : 8 ;
 #endif      
#else
 #ifdef HAPTIC
			IlinesCount = g_eeGeneral.welcomeType == 2 ? 10 : 9 ;
 #else
			IlinesCount = g_eeGeneral.welcomeType == 2 ? 8 : 7 ;
 #endif      
#endif      
			TITLE( PSTR(STR_AudioHaptic) ) ;

//#ifdef COLOUR_DISPLAY
//			DisplayOffset = RAUD_OFF_0 ;
//#endif
  		 
#ifndef COLOUR_DISPLAY
#ifdef HAPTIC
			if ( sub < 6 )
#else
			if ( sub < 4 )
#endif
			{
				displayNext() ;
#endif

				PUTS_ATT_LEFT( y, PSTR(STR_VOLUME));
				current_volume = g_eeGeneral.volume ;
				PUTS_NUM( PARAM_OFS+2*FW+RAUD_OFF_0, y, current_volume, (sub==subN) ? blink : 0 ) ;
  			if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( current_volume,NUM_VOL_LEVELS-1);
					if ( current_volume != g_eeGeneral.volume )
					{
						setVolume( g_eeGeneral.volume = current_volume ) ;
					}
				}
  			y += FHPY ;
				subN += 1 ;
      
	    	uint8_t b ;
  	  	b = g_eeGeneral.beeperVal ;
      	PUTS_ATT_LEFT( y,PSTR(STR_BEEPER));
      	PUTS_AT_IDX(PARAM_OFS+2*FW, y, PSTR(STR_BEEP_MODES),b,(sub==subN ? blink:0)|LUA_RIGHT);
	    	if(sub==subN) { CHECK_INCDEC_H_GENVAR_0( b, 6); g_eeGeneral.beeperVal = b ; }
  			y += FHPY ;
				subN += 1 ;

		// audio start by rob
      	PUTS_ATT_LEFT( y,PSTR(STR_SPEAKER_PITCH));
      	PUTS_NUM(PARAM_OFS+2*FW,y,g_eeGeneral.speakerPitch,(sub==subN ? blink : 0) );
      	if(sub==subN) CHECK_INCDEC_H_GENVAR( g_eeGeneral.speakerPitch, 1, 100) ;
  			y += FHPY ;
				subN += 1 ;
	 
#ifdef HAPTIC
      	PUTS_ATT_LEFT( y,PSTR(STR_HAPTICSTRENGTH));
      	PUTS_NUM(PARAM_OFS+2*FW+RAUD_OFF_0,y,g_eeGeneral.hapticStrength,(sub==subN ? blink : 0) );
      	if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.hapticStrength, 5) ;
  			y += FHPY ;
				subN += 1 ;

      	PUTS_ATT_LEFT( y,XPSTR("Haptic Min Run"));
      	PUTS_NUM(PARAM_OFS+2*FW+RAUD_OFF_0,y,g_eeGeneral.hapticMinRun+20,(sub==subN ? blink : 0) );
      	if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.hapticMinRun, 20 ) ;
  			y += FHPY ;
				subN += 1 ;
#endif

    		PUTS_ATT( 0, y, XPSTR(GvaString), sub==subN ? INVERS : 0 ) ;
      	if( sub==subN )
				{
					if ( checkForMenuEncoderBreak( Tevent ) )
					{
						SubMenuCall = 0x85 ;
						pushMenu(menuGlobalVoiceAlarm) ;
					}
				}
#ifdef COLOUR_DISPLAY
  			y += FHPY ;
				subN += 1 ;
#else
			}
			else
			{
#ifdef HAPTIC
			 	subN = 6 ;
#else
			 	subN = 4 ;
#endif
#endif
  	    g_eeGeneral.preBeep = onoffMenuItem( g_eeGeneral.preBeep, y, PSTR(STR_BEEP_COUNTDOWN), sub == subN ) ;
  			y += FHPY ;
				subN += 1 ;

	      g_eeGeneral.minuteBeep = onoffMenuItem( g_eeGeneral.minuteBeep, y, PSTR(STR_MINUTE_BEEP), sub == subN ) ;
  			y += FHPY ;
				subN += 1 ;

		 		PUTS_ATT_LEFT( y, XPSTR( "Welcome Type") ) ;
				g_eeGeneral.welcomeType = checkIndexed( y, XPSTR(FWx19"\002""\006System  NoneCustom"), g_eeGeneral.welcomeType, sub==subN ) ;
  			y += FHPY ;
				subN += 1 ;

				if ( g_eeGeneral.welcomeType == 2 )
				{
					// FileName
			 		PUTS_ATT_LEFT( y, XPSTR( "FileName") ) ;
    			LcdFlags attr = sub == subN ? InverseBlink : 0 ;
					SubMenuCall = 0x89 ;
					alphaEditName( 10*FW-2+RAUD_OFF_0, y, (uint8_t *)g_eeGeneral.welcomeFileName, sizeof(g_eeGeneral.welcomeFileName), (sub==subN) | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FileName") ) ;
					validateName( g_eeGeneral.welcomeFileName, sizeof(g_eeGeneral.welcomeFileName) ) ;
  				if( attr )
					{
						if ( checkForMenuEncoderLong( event ) )
						{
							VoiceFileType = VOICE_FILE_TYPE_USER ;
     				 	pushMenu( menuSelectVoiceFile ) ;
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
	  			y += FHPY ;
					subN += 1 ;
				}
#if defined(PCBX12D) || defined(PCBX10)
		 		PUTS_ATT_LEFT( y, XPSTR( "Hardware Volume") ) ;
				{
					uint8_t b ;
					LcdFlags attr ;
					attr = 0 ;
					b = g_eeGeneral.unused_PPM_Multiplier ;
      		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.unused_PPM_Multiplier, 0, 127) ; }
	      	PUTS_NUM(PARAM_OFS+2*FW-2+RAUD_OFF_0, y, g_eeGeneral.unused_PPM_Multiplier, attr);
//uint8_t I2C_read_volume() ;
//					PUTS_NUMX( PARAM_OFS+80, y, I2C_read_volume() ) ;
					if ( b != g_eeGeneral.unused_PPM_Multiplier )
					{
						b = g_eeGeneral.unused_PPM_Multiplier ;
void I2C_set_volume( register uint8_t volume ) ;
						I2C_set_volume( b ) ;
					}
				}
#endif
#ifndef COLOUR_DISPLAY
			}
#endif
		}
#endif // TOUCH
		break ;

#ifdef COLOUR_DISPLAY
#define RALRM_OFF_0			0
#else
#define RALRM_OFF_0			0
#endif

		case M_ALARMS	:
#ifdef COLOUR_DISPLAY
			pushMenu( menuAlarms ) ;
#else			
		{
			uint32_t subN = 0 ;
#ifdef PCBSKY
#define NUM_ALARMS			7
#else
#define NUM_ALARMS			6
#endif
			IlinesCount = NUM_ALARMS ;
			if ( g_eeGeneral.ar9xBoard )
			{
				IlinesCount -= 1 ;
			}

//	    if( g_eeGeneral.frskyinternalalarm == 1)
//			{
//				IlinesCount += 3 ;
//			}

  		LcdFlags attr ;
			TITLE( PSTR(STR_Alarms) ) ;

//#ifdef COLOUR_DISPLAY
//			DisplayOffset = RALRM_OFF_0 ;
//#endif
			 
  		attr = LEFT ;
      PUTS_ATT_LEFT( y,PSTR(STR_BATT_WARN));
      if(sub==subN) { attr = blink | LEFT ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatWarn, 40, 120); } //5-10V
      putsVolts(PARAM_OFS+RALRM_OFF_0, y, g_eeGeneral.vBatWarn, attr);
  		y += FHPY ;
			subN += 1 ;

  		attr = 0 ;//LEFT ;
      PUTS_ATT_LEFT( y,PSTR(STR_INACT_ALARM));
      if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.inactivityTimer, -10, 110); } //0..120minutes
      PUTS_NUM(PARAM_OFS+2*FW-2, y, g_eeGeneral.inactivityTimer+10, attr);
  		y += FHPY ;
			subN += 1 ;

	  	PUTS_P( FW+RALRM_OFF_0, y, PSTR(STR_VOLUME) ) ;
			int8_t value = g_eeGeneral.inactivityVolume + ( NUM_VOL_LEVELS-3 ) ;
			PUTS_NUM( PARAM_OFS+2*FW+RALRM_OFF_0, y, value, (sub==subN) ? blink : 0 ) ;
	  	if(sub==subN)
			{
				CHECK_INCDEC_H_GENVAR_0( value,NUM_VOL_LEVELS-1);
				g_eeGeneral.inactivityVolume = value - ( NUM_VOL_LEVELS-3 ) ;
			} 	
	  	y += FHPY ;
			subN += 1 ;
      
#ifdef PCBSKY
			if ( g_eeGeneral.ar9xBoard == 0 )
			{
  			PUTS_ATT_LEFT( y, PSTR(STR_CAPACITY_ALARM));
				PUTS_NUM( PARAM_OFS+2*FW+RALRM_OFF_0, y, g_eeGeneral.mAh_alarm*50, (sub==subN) ? blink : 0 ) ;
  			if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.mAh_alarm,100);
				}
 				y += FHPY ;
				subN += 1 ;
			}
#endif
		
      uint8_t b = g_eeGeneral.disableThrottleWarning;
      g_eeGeneral.disableThrottleWarning = offonMenuItem( b, y, PSTR(STR_THR_WARNING), sub == subN ) ;
 			y += FHPY ;
			subN += 1 ;

      b = g_eeGeneral.disableAlarmWarning;
      g_eeGeneral.disableAlarmWarning = offonMenuItem( b, y, PSTR(STR_ALARM_WARN), sub == subN ) ;
 			y += FHPY ;
			subN += 1 ;

      b = g_eeGeneral.disableRxCheck;
      g_eeGeneral.disableRxCheck = offonMenuItem( b, y, XPSTR("Receiver Warning"), sub == subN ) ;
 			y += FHPY ;
			subN += 1 ;
		}
#endif		
		break ;

#ifdef COLOUR_DISPLAY
#define RGEN_OFF_0			0
#else
#define RGEN_OFF_0			0
#endif


		case M_GENERAL :
		{
#ifdef COLOUR_DISPLAY
      pushMenu(menuGeneral) ;
#else			 
			uint32_t subN = 0 ;

// Add 1 to these if using _eeGeneral.disableBtnLong
#if defined(PCBSKY) || defined(PCB9XT)
 #ifdef PCBSKY
			IlinesCount = 9+1+1+1 ;
 #else
			IlinesCount = 8+1+1+1 ;
 #endif
#else
 #if defined(PCBX12D) || defined(PCBX10)
			IlinesCount = 7+1+1+1+1 ;
 #else
			IlinesCount = 7+1+1+1 ;
 #endif
#endif
			TITLE( XPSTR(Str_General) ) ;

//#ifdef COLOUR_DISPLAY
//			DisplayOffset = RGEN_OFF_0 ;
//#endif

#ifdef JUNGLECAM
			IlinesCount += 1 ;
#endif

//#if defined(PCBSKY) || defined(PCB9XT)
#if defined(PCBX12D) || defined(PCBX10)
		 {
#else
		 if ( sub < 6 )
		 {
			displayNext() ;
#endif
//#endif

			SubMenuCall = 0x80 ;
			alphaEditName( 10*FW-2+RGEN_OFF_0, y, (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName), sub==subN, (uint8_t *)XPSTR( "Owner Name") ) ;
			validateName( (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName) ) ;
 			y += FHPY ;
			subN += 1 ;

	    PUTS_ATT_LEFT( y,PSTR(STR_LANGUAGE));
#ifdef SMALL
 #ifdef FRENCH
	    PUTS_AT_IDX( PARAM_OFS, y, XPSTR("\012   ENGLISH  FRANCAIS"),g_eeGeneral.language,(sub==subN ? blink:0)|LUA_RIGHT);
 #else
	    PUTS_AT_IDX( PARAM_OFS, y, XPSTR("\012   ENGLISH  DEUTSCH "),g_eeGeneral.language,(sub==subN ? blink:0)|LUA_RIGHT);
 #endif	    
			if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.language, 1 ) ;
#else
	    PUTS_AT_IDX( PARAM_OFS, y, XPSTR("\012   ENGLISH  FRANCAIS   DEUTSCH NORWEGIAN   SWEDISH   ITALIAN    POLISHVIETNAMESE   SPANISH"),g_eeGeneral.language,(sub==subN ? blink:0)|LUA_RIGHT);
	    if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.language, 8 ) ;
#endif
			setLanguage() ;
 			y += FHPY ;
			subN += 1 ;

      uint8_t b = g_eeGeneral.disableSplashScreen;
      g_eeGeneral.disableSplashScreen = offonMenuItem( b, y, PSTR(STR_SPLASH_SCREEN), sub == subN ) ;
 			y += FHPY ;
			subN += 1 ;

      b = g_eeGeneral.hideNameOnSplash;
      g_eeGeneral.hideNameOnSplash = offonMenuItem( b, y, PSTR(STR_SPLASH_NAME), sub == subN ) ;
 			y += FHPY ;
			subN += 1 ;
			
      b = g_eeGeneral.stickScroll;
      g_eeGeneral.stickScroll = onoffMenuItem( b, y, PSTR(STR_STICKSCROLL), sub == subN ) ;
 			y += FHPY ;
			subN += 1 ;

//      b = 1-g_eeGeneral.disablePotScroll ;
//			b |= g_eeGeneral.stickScroll << 1 ;
//      PUTS_ATT_LEFT( y,PSTR(STR_SCROLLING));
//      PUTS_AT_IDX(PARAM_OFS-2*FW+RGEN_OFF_0, y, XPSTR("\005 NONE  POTSTICK BOTH"),b,(sub==subN ? blink:0));
//      if(sub==subN) CHECK_INCDEC_H_GENVAR_0( b, 3 ) ;
//			g_eeGeneral.stickScroll = b >> 1 ;
//			g_eeGeneral.disablePotScroll = 1 - ( b & 1 ) ;
// 			y += FH ;
//			subN += 1 ;

      b = g_eeGeneral.forceMenuEdit ;
      g_eeGeneral.forceMenuEdit = onoffMenuItem( b, y, PSTR(STR_MENU_ONLY_EDIT), sub == subN ) ;
 			y += FHPY ;
			subN += 1 ;

#if defined(PCBX12D) || defined(PCBX10)
#else
		 }
		 else
		 {
		 	subN = 6 ;
#endif

      g_eeGeneral.disableBtnLong = onoffMenuItem( g_eeGeneral.disableBtnLong, y, XPSTR("No ENC. as exit"), sub == subN ) ;
 			y += FHPY ;
			subN += 1 ;

#if defined(PCBSKY) || defined(PCB9XT)
  		PUTS_ATT_LEFT( y, PSTR(STR_BT_BAUDRATE));
  		PUTS_AT_IDX( PARAM_OFS+2*FW, y, XPSTR(BtBaudString),g_eeGeneral.bt_baudrate,(sub==subN ? blink:0)|LUA_RIGHT);
  		if(sub==subN)
			{
				CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.bt_baudrate,4);
			}
 			y += FHPY ;
			subN += 1 ;
#endif
  		PUTS_ATT_LEFT( y, XPSTR("GPS Format") ) ;
  		displayGPSformat( PARAM_OFS+4*FW, y, (sub==subN ? blink:0) ) ;
  		if(sub==subN)
			{
				CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.gpsFormat,1) ;
			}
 			y += FHPY ;
			subN += 1 ;

#ifdef PCBSKY
  		PUTS_ATT(RGEN_OFF_0, y, XPSTR("Run Maintenance"), (sub==subN) ? INVERS : 0 ) ;
  		if(sub==subN)
			{
				if ( checkForMenuEncoderLong( event ) )
				{
					GPBR->SYS_GPBR0 = 0x5555AAAA ;
					// save EEPROM and reboot
					prepareForShutdown() ;
  				uint16_t tgtime = get_tmr10ms() ;
	  			while( (uint16_t)(get_tmr10ms() - tgtime ) < 50 ) // 50 - Half second
					{
						wdt_reset() ;
						if ( ee32_check_finished() )
						{
							if ( read_keys() & 2 )
							{							
								break ;
							}
						}
  					tgtime = get_tmr10ms() ;
					}
		  		NVIC_SystemReset() ;
				}
			}
 			y += FHPY ;
			subN += 1 ;
#endif

      g_eeGeneral.altMixMenu = onoffMenuItem( g_eeGeneral.altMixMenu, y, XPSTR("Mix Menu Details"), sub == subN ) ;

#if defined(PCBX12D) || defined(PCBX10)
 			y += FHPY ;
			subN += 1 ;
			PUTS_ATT_LEFT( y,XPSTR("ScreenShot Sw"));
      putsDrSwitches(PARAM_OFS-FW+RGEN_OFF_0,y,g_eeGeneral.screenShotSw,sub==subN ? blink : 0);
      if(sub==subN) CHECK_INCDEC_GENERALSWITCH(event, g_eeGeneral.screenShotSw, -MaxSwitchIndex, MaxSwitchIndex) ;
#endif

 			y += FHPY ;
			subN += 1 ;
      g_eeGeneral.enableEncMain = onoffMenuItem( g_eeGeneral.enableEncMain, y, XPSTR("ENC main screen"), sub == subN ) ;

#ifdef JUNGLECAM
 			y += FHPY ;
			subN += 1 ;
      g_eeGeneral.jungleMode = onoffMenuItem( g_eeGeneral.jungleMode, y, XPSTR("Jungle Mode"), sub == subN ) ;
#endif
  	 }
#endif
		}
		break ;

#ifdef COLOUR_DISPLAY
#define RCON_OFF_0			0
#else
#define RCON_OFF_0			0
#endif


		case M_CONTROLS :
#if (defined(PCBX12D) || defined(PCBX10) || defined(TOUCH))
      pushMenu(menuControls) ;
#else			 
		{
			uint32_t subN = 0 ;
#ifdef REV9E
			IlinesCount = 9 ;
#else
			IlinesCount = 8 ;
#endif
			TITLE( PSTR(STR_Controls) ) ;

//#ifdef COLOUR_DISPLAY
//			DisplayOffset = RCON_OFF_0 ;
//#endif

#ifndef COLOUR_DISPLAY
#ifdef REV9E
			if ( sub < 5 )
#else
			if ( sub < 4 )
#endif
			{
				displayNext() ;
#endif
				LcdFlags attr = sub==subN ? blink : 0 ;
	  		PUTS_ATT_LEFT( y, PSTR(STR_CHAN_ORDER) ) ;//   RAET->AETR
				uint8_t bch = bchout_ar[g_eeGeneral.templateSetup] ;
				char text[8] ;
    		for ( uint32_t i = 4 ; i > 0 ; i -= 1 )
				{
					text[i] = *(PSTR(STR_SP_RETA) +(bch & 3) + 1 ) ;
					bch >>= 2 ;
				}
				text[5] = 0 ;
#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
  			PUTS_ATT( 19*FW, y, &text[1], attr|LUA_RIGHT ) ;
#else
  			PUTS_ATT( 15*FW, y, &text[1], attr ) ;
#endif
	  		if(attr) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.templateSetup, 23 ) ;
 				y += FHPY ;
				subN += 1 ;
      
				uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
        PUTS_ATT_LEFT(y, PSTR(STR_CROSSTRIM));
				ct = checkIndexed( y, XPSTR(FWx19"\002\003OFFON Vtg"), ct, (sub==subN) ) ;
				g_eeGeneral.crosstrim = ct ;
				g_eeGeneral.xcrosstrim = ct >> 1 ;
 				y += FHPY ;
				subN += 1 ;

				uint8_t oldValue = g_eeGeneral.throttleReversed ;
				g_eeGeneral.throttleReversed = onoffMenuItem( oldValue, y, PSTR(STR_THR_REVERSE), sub == subN ) ;
				if ( g_eeGeneral.throttleReversed != oldValue )
				{
  				checkTHR() ;
				}
 				y += FHPY ;
				subN += 1 ;

#ifdef REV9E
				PUTS_ATT_LEFT( y,XPSTR("Page Button:"));
				g_eeGeneral.pageButton = checkIndexed( y, XPSTR(FWx15"\001""\005 LEFTRIGHT"), g_eeGeneral.pageButton, (sub==subN) ) ;// FWx07
 				y += FH ;
				subN += 1 ;
#endif

	    	PUTS_ATT_LEFT( y, PSTR(STR_MODE) );
				for ( uint32_t i = 0 ; i < 4 ; i += 1 )
				{
#ifdef COLOUR_DISPLAY
 					lcd_HiResimg( ((6+4*i)*FW*2+RCON_OFF_0*2), y*HVSCALE, sticksHiRes, i, 0 ) ;
#else
					lcd_img((6+4*i)*FW, y, sticks, i, 0 ) ;
#endif
				}
 				y += FHPY ;
    
 				attr = 0 ;
				uint8_t mode = g_eeGeneral.stickMode ;
      	if(sub==subN)
				{
					attr = INVERS ;
					if ( s_editMode )
					{
					 	attr = BLINK ;
						
						CHECK_INCDEC_H_GENVAR_0( mode,3);
						if ( mode != g_eeGeneral.stickMode )
						{
//							g_eeGeneral.stickScroll = 0 ;
							g_eeGeneral.stickMode = mode ;							
						}
					}
				}
      	PUTC_ATT( 3*FW+RCON_OFF_0, y, '1'+g_eeGeneral.stickMode,attr);
      	for(uint32_t i=0; i<4; i++)
				{
					putsChnRaw( (6+4*i)*FW+RCON_OFF_0, y, modeFixValue( i ), 0 ) ;//sub==3?INVERS:0);
				}
 				y += FHPY ;
				subN += 1 ;
#ifndef COLOUR_DISPLAY
			}
			else
			{
#ifdef REV9E
				subN = 5 ;
#else
				subN = 4 ;
#endif
#endif
				if ( sub >= IlinesCount )
				{
					sub = mstate.m_posVert = IlinesCount-1 ;
				}
				// Edit custom stick names
      	for(uint32_t i=0; i<4; i++)
				{
      		PUTS_AT_IDX( 5*FW+RCON_OFF_0, y, PSTR(STR_STICK_NAMES), i, 0 ) ;
					if ( sub == subN )
					{
#ifdef REV9E
						SubMenuCall = 0x80 + i + 5 ;
#else
						SubMenuCall = 0x80 + i + 4 ;
#endif
					}
					alphaEditName( 11*FW+RCON_OFF_0, y, &g_eeGeneral.customStickNames[i*4], 4, sub==subN, (uint8_t *)&PSTR(STR_STICK_NAMES)[i*5+1] ) ;
	 				y += FHPY ;
					subN += 1 ;
				}
#ifndef COLOUR_DISPLAY
			}
#endif
		}			 
#endif // TOUCH
		break ;


// Filter ADC - ALL
// Rotary divisor - Not X7 or XLITE
// I2C Function - 9XT only
// BT Com port - 9XT only



		case M_HARDWARE :
		{
			uint32_t subN = 0 ;
#ifdef PCBSKY
			uint8_t num = 9 + 2 + 2 + 4 + 1 + 1 + 4 + 1 ;
 #ifndef REVX
			num += 1 ;
 #endif
			IlinesCount = num ;
#else
 #ifdef PCB9XT
			IlinesCount = 4 + 4 + 1 + 7 + 5 + 1 ;
 #else
  #ifdef REV9E
			IlinesCount = 4 + 5 + 1 + 3 + 3 ;
  #else
   #ifdef PCBX7
    #ifdef PCBT12
			IlinesCount = 1 + 4 + 3 ;
		#else
			IlinesCount = 7 + 4 + 1 + 1 + 2 ;
		#endif
   #else
    #if defined(PCBX12D) || defined(PCBX10)
			IlinesCount = 2 + 5 + 6 + 4 + 1 ;
    #else
     #ifdef PCBXLITE
			IlinesCount = 6 + 4 + 3 ;
     #else
      #ifdef PCBX9LITE
 		   #if defined(X9LS)
			IlinesCount = 3+1+2 + 4 + 1 + 2 - 2 ;
			#define HW_PAGE1	4
			 #else
			IlinesCount = 3+1+2 + 4 + 1 + 2 ;
			#define HW_PAGE1	6
       #endif
			#define HW_PAGE2	4
      #else
			 #ifdef PCBLEM1
			IlinesCount = 7 ;
			 #else
			IlinesCount = 6 + 4 + 1 + 1 + 4 ;
       #endif
      #endif
     #endif
    #endif
   #endif
  #endif
 #endif
#endif

#ifdef PCBSKY
 #ifndef REVX
			uint32_t page2 = 12 ;
			uint32_t page3 = 13 ;
 #else
  #ifdef PCB9XT
			uint32_t page2 = 12 ;
			uint32_t page3 = 13 ;
  #else
			uint32_t page2 = 11 ;
			uint32_t page3 = 12 ;
  #endif
 #endif
			if ( g_eeGeneral.analogMapping & MASK_6POS )
			{
				IlinesCount += 6 ;
				page3 += 6 ;
			}
#endif // PCBSKY

#ifdef PCBSKY
			if ( g_eeGeneral.ar9xBoard )
			{
				num += 1 ;
				page2 += 1 ;
				page3 += 1 ;
				IlinesCount += 2 ;
			}
			else
			{
				IlinesCount += 1 ;
			}
#endif
#if defined(PCBX12D) || defined(PCBX10)
			uint32_t page2 = 7 ;
			uint32_t page3 = 13 ;
//			IlinesCount += 6 ;
//			page3 += 6 ;
#endif

//#ifdef PCBLEM1
//			uint32_t page2 = 7 ;
//#endif

#ifdef PCB9XT
			uint32_t page2 = 8 ;
			uint32_t page3 = 9 ;
			if ( g_eeGeneral.analogMapping & MASK_6POS )
			{
				IlinesCount += 6 ;
				page3 += 6 ;
			}
#endif
#ifdef PCBX9D
	#ifdef PCBX7
   #ifndef PCBT12
			uint32_t page2 = 11 ;
			uint32_t page3 = 12 ;
	 #endif
	#else
   #ifndef PCBXLITE
    #ifdef REV9E
			uint32_t page2 = 9 ;
			uint32_t page3 = 10 ;
	  #else
     #ifdef PCBX9LITE
			uint32_t page2 = HW_PAGE1+HW_PAGE2 ;
			uint32_t page3 = page2+1 ;
	   #else
			uint32_t page2 = 10 ;
			uint32_t page3 = 11 ;
     #endif
    #endif
   #endif
	#endif
#ifndef PCBXLITE
 #ifndef PCBT12
			if ( g_eeGeneral.analogMapping & MASK_6POS )
			{
				IlinesCount += 6 ;
				page3 += 6 ;
			}
 #endif
#endif
#endif

			TITLE( PSTR(STR_Hardware) ) ;
			if ( HardwareMenuEnabled == 0 )
			{
				IlinesCount = 0 ;
        PUTS_ATT_LEFT( 3*FHPY,XPSTR(" Disabled (see Help)"));
				break ;
			}
			
#ifdef PCBSKY
			if ( sub < 6 )
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
 #ifdef REV9E
			if ( sub < 4 )
 #else
  #if defined(PCBX12D) || defined(PCBX10)
			if ( sub < 2 )
  #else
   #if defined(PCBX7) || defined (PCBXLITE)
    #ifdef PCBT12
			if ( sub < 1 )
    #else
			if ( sub < 6 )
    #endif
   #else
    #ifdef PCBX9LITE
 			if ( sub < HW_PAGE1 )
    #else // X3
 			if ( sub < 5 )
    #endif // X3
   #endif
  #endif
 #endif
#endif
#ifdef PCBLEM1
			if ( sub < 6 )
#endif
#ifdef PCB9XT
			if ( sub < 4 )
#endif
			{
				displayNext() ;
        PUTS_ATT_LEFT( y,PSTR(STR_FILTER_ADC));
        PUTS_AT_IDX(PARAM_OFS, y, XPSTR("\004SINGOSMPFILT"),g_eeGeneral.filterInput,(sub==subN ? blink:0));
        if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.filterInput, 2);
 				y += FHPY ;
				subN += 1 ;

#ifndef PCBX7
 #ifndef PCBX9LITE
  #ifndef PCBLEM1
  			PUTS_ATT_LEFT( y, PSTR(STR_ROTARY_DIVISOR));
	  		PUTS_AT_IDX( 15*FW, y, XPSTR("\001142"),g_eeGeneral.rotaryDivisor,(sub==subN ? blink:0));
  			if(sub==subN)
				{
					uint8_t oldValue = g_eeGeneral.rotaryDivisor ;
					CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.rotaryDivisor,2);
					if ( g_eeGeneral.rotaryDivisor != oldValue )
					{
extern volatile int32_t Rotary_count ;
extern int32_t LastRotaryValue ;
						Rotary_count = 0 ;
						Rotary_diff = 0 ;
						LastRotaryValue = 0 ;
					}
				}
 				y += FHPY ;
				subN += 1 ;
  #endif // nPCBLEM1
 #endif // X3
#endif

#ifdef PCB9XT
		 		PUTS_ATT_LEFT( y, XPSTR( "I2C Function") ) ;
				uint8_t oldValue = g_eeGeneral.enableI2C ;
				g_eeGeneral.enableI2C = checkIndexed( y, XPSTR(FWx15"\002""\004 Off I2CCOM3"), g_eeGeneral.enableI2C, (sub==subN) ? blink:0 ) ;
				if ( oldValue != g_eeGeneral.enableI2C )
				{
					// Change function
				}
 				y += FHPY ;
				subN += 1 ;

		 		PUTS_ATT_LEFT( y, XPSTR( "Bt Com Port") ) ;
				g_eeGeneral.btComPort = checkIndexed( y, XPSTR(FWx15"\002""\004NONECOM2COM3"), g_eeGeneral.btComPort, (sub==subN) ? blink:0 ) ;
 				y += FHPY ;
				subN += 1 ;
#endif

// If X9Dplus, allow configuration of S3 as POT or 6-pos switch

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)

#if defined(REVPLUS) || defined(REV9E)
#define NUM_MAP_POTS	3
#else
 #ifdef PCBX9LITE
#define NUM_MAP_POTS	1
 #else // X3
#define NUM_MAP_POTS	2
 #endif // X3
#endif

#ifndef REV9E
 #ifndef PCBX12D
	#ifndef PCBX7
	 #ifndef PCBX9LITE
    #ifndef PCBX10
				uint32_t value ;
				uint32_t oldValue ;
    #endif	// nX10
	 #endif	// nX3
	#endif	// nX7
 #endif	// nX12D
#endif	// nREV9E
#ifndef REV9E
 #ifndef PCBX12D
	#ifndef PCBX7
   #ifndef PCBXLITE
    #ifndef PCBX9LITE
     #ifndef PCBX10
  			PUTS_ATT_LEFT( y, XPSTR("Encoder"));
				value = g_eeGeneral.analogMapping & ENC_MASK ;
	  		PUTS_AT_IDX( 15*FW, y, XPSTR("\002--P1P2P3"), value, (sub==subN ? BLINK:0));
				oldValue = value ;
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_MAP_POTS ) ;
 				g_eeGeneral.analogMapping = ( g_eeGeneral.analogMapping & ~ENC_MASK ) | value ;
				if ( value != oldValue )
				{
					init_adc2() ;
				}
				y += FHPY ;
				subN += 1 ;
     #endif	// nX12D
	  #endif	// nX3
   #endif // nXlite
	#endif	// nX7
 #endif	// nX12D
#endif	// nREV9E

#ifdef PCBXLITE
  			PUTS_ATT_LEFT( y, XPSTR("Switch C"));
				value = g_eeGeneral.ailsource ;
	  		PUTS_AT_IDX( 15*FW, y, XPSTR("\0052-Pos3-Pos"), value, (sub==subN ? BLINK:0));
				oldValue = value ;
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, 1 ) ;
				g_eeGeneral.ailsource = value ;
				if ( value != oldValue )
				{
					createSwitchMapping() ;
				}
				y += FHPY ;
				subN += 1 ;

  			PUTS_ATT_LEFT( y, XPSTR("Switch D"));
				value = g_eeGeneral.rudsource ;
	  		PUTS_AT_IDX( 15*FW, y, XPSTR("\0052-Pos3-Pos"), value, (sub==subN ? BLINK:0));
				oldValue = value ;
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, 1 ) ;
				g_eeGeneral.rudsource = value ;
				if ( value != oldValue )
				{
					createSwitchMapping() ;
				}
				y += FHPY ;
				subN += 1 ;

#endif // Xlite

#ifndef PCBX12D
#ifndef PCBX10
//  			PUTS_ATT_LEFT( y, XPSTR("6 Pos. Switch"));
//				value = ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2 ;
//	  		PUTS_AT_IDX( 15*FW, y, XPSTR("\002--P1P2P3"), value, (sub==subN ? blink:0));
//				oldValue = value ;
//				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_MAP_POTS ) ;
// 				g_eeGeneral.analogMapping = ( g_eeGeneral.analogMapping & ~MASK_6POS ) | ( value << 2 ) ;
//				if ( value != oldValue )
//				{
//					createSwitchMapping() ;
//				}
//				y += FH ;
//				subN += 1 ;

#ifndef PCBT12
 #if defined(PCBX9LITE) && defined(X9LS)
 #else
  			PUTS_ATT_LEFT( y, XPSTR("PB1 Switch\037PB2 Switch"));
				g_eeGeneral.pb1source = edit3posSwitchSource( y, g_eeGeneral.pb1source, USE_PB1, (sub==subN), 0 ) ;
				y += FHPY ;
				subN += 1 ;
				g_eeGeneral.pb2source = edit3posSwitchSource( y, g_eeGeneral.pb2source, USE_PB2, (sub==subN), 0 ) ;
				y += FHPY ;
				subN += 1 ;
#endif
#ifdef PCBX9LITE
  			PUTS_ATT_LEFT( y, XPSTR("PB3 Switch"));
				g_eeGeneral.pb3source = edit3posSwitchSource( y, g_eeGeneral.pb3source, USE_PB3, (sub==subN), 0 ) ;
				y += FHPY ;
				subN += 1 ;
#endif

#ifdef PCBX9LITE
  			PUTS_ATT_LEFT( y, XPSTR("Pot 2\037Pot 3"));
				g_eeGeneral.extraPotsSource[0] = checkIndexed(y,
      			XPSTR(FWx17"\002\004NONEPC2 PC3 "), g_eeGeneral.extraPotsSource[0], (sub==subN));
				NumExtraPots = countExtraPots() ;
				y += FHPY ;
				subN += 1 ;
				g_eeGeneral.extraPotsSource[1] = checkIndexed(y,
      			XPSTR(FWx17"\002\004NONEPC2 PC3 "), g_eeGeneral.extraPotsSource[1], (sub==subN));
				NumExtraPots = countExtraPots() ;
				y += FHPY ;
				subN += 1 ;

#endif

#ifdef PCBX7
  			PUTS_ATT_LEFT( y, XPSTR("Pot 3\037Pot 4"));
				g_eeGeneral.extraPotsSource[0] = checkIndexed(y,
      			XPSTR(FWx17"\002\004NONEPC5 PB1 "), g_eeGeneral.extraPotsSource[0], (sub==subN));
				NumExtraPots = countExtraPots() ;
				y += FHPY ;
				subN += 1 ;
				g_eeGeneral.extraPotsSource[1] = checkIndexed(y,
      			XPSTR(FWx17"\002\004NONEPC5 PB1 "), g_eeGeneral.extraPotsSource[1], (sub==subN));
				NumExtraPots = countExtraPots() ;
				y += FHPY ;
				subN += 1 ;

  			PUTS_ATT_LEFT( y, PSTR(STR_ROTARY_DIVISOR));
	  		PUTS_AT_IDX( 15*FW, y, XPSTR("\001142"),g_eeGeneral.rotaryDivisor,(sub==subN ? blink:0));
  			if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.rotaryDivisor,2);
				}
 				y += FHPY ;
				subN += 1 ;

#endif	// X7
#endif	// T12

#endif	// nX10
#endif	// nX12D

#endif	// PCBX9D

#ifdef PCBSKY
				uint8_t lastValue = g_eeGeneral.stickGain ;
//				uint8_t b ;
        edit_stick_gain(2*FW, y, PSTR(STR_LV), STICK_LV_GAIN, (sub==subN) ) ;
 				y += FHPY ;
				subN += 1 ;
  		
        edit_stick_gain(2*FW, y, PSTR(STR_LH), STICK_LH_GAIN, (sub==subN) ) ;
 				y += FHPY ;
				subN += 1 ;

        edit_stick_gain(2*FW, y, PSTR(STR_RV), STICK_RV_GAIN, (sub==subN) ) ;
 				y += FHPY ;
				subN += 1 ;

        edit_stick_gain(2*FW, y, PSTR(STR_RH), STICK_RH_GAIN, (sub==subN) ) ;
				if ( lastValue != g_eeGeneral.stickGain )
				{
					set_stick_gain( g_eeGeneral.stickGain ) ;
				}
#endif // PCBSKY
#ifndef PCBLEM1
			}
#endif			
			
#ifdef PCBSKY
			else if ( sub < page2 )
			{
				displayNext() ;
				subN = 6 ;
#endif // PCBSKY

#ifdef PCB9XT
			else if ( sub < page2 )
			{
				displayNext() ;
				subN = 4 ;
#endif // PCB9XT

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
 #ifdef REV9E
			else if ( sub < 9 )
			{
				subN = 4 ;
 #else
	#if defined(PCBX12D) || defined(PCBX10)
			else if ( sub < page2 )
			{
				displayNext() ;
				subN = 2 ;
  #else
	 #ifdef PCBX7
    #ifdef PCBT12
			else if ( sub < 5 )
			{
				subN = 1 ;
		#else
			else if ( sub < 11 )
			{
				subN = 6 ;
		#endif
   #else
	  #ifdef PCBXLITE
			else if ( sub < 10 )
			{
				subN = 6 ;
    #else
     #ifdef PCBX9LITE
			else if ( sub < HW_PAGE1+HW_PAGE2 )
			{
				subN = HW_PAGE2 ;
     #else
			else if ( sub < 10 )
			{
				subN = 5 ;
     #endif
    #endif
   #endif
  #endif
 #endif
#endif // PCBX9D
        edit_stick_deadband(5*FW/2, y, PSTR(STR_LV), 1, (sub==subN));
				y += FHPY ;
  			subN++;

        edit_stick_deadband(5*FW/2, y, PSTR(STR_LH), 0, (sub==subN));
  			y += FHPY ;
  			subN++;

        edit_stick_deadband(5*FW/2, y, PSTR(STR_RV), 2, (sub==subN));
  			y += FHPY ;
  			subN++;
        edit_stick_deadband(5*FW/2, y, PSTR(STR_RH), 3, (sub==subN));
  			y += FHPY ;
  			subN++;

#if defined(PCBSKY) || defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
 #ifndef PCBXLITE
  #ifndef PCBT12
   #ifndef PCBX9LITE
				{
					uint8_t lastValue = g_eeGeneral.softwareVolume ;
			  	g_eeGeneral.softwareVolume = onoffMenuItem( lastValue, y, XPSTR("Software Vol."), sub==subN ) ;
					if ( lastValue != g_eeGeneral.softwareVolume )
					{
						setVolume( g_eeGeneral.volume ) ;
					}
				}
	  		y += FHPY ;
				subN += 1 ;
   #endif // X3
  #endif
 #endif
#endif

#ifdef PCBSKY
#ifndef REVX
		  	g_eeGeneral.ar9xBoard = onoffMenuItem( g_eeGeneral.ar9xBoard, y, XPSTR("AR9X Board"), sub==subN ) ;
	  		y += FHPY ;
				subN += 1 ;
#endif
#endif

#ifdef PCBSKY
				if ( g_eeGeneral.ar9xBoard )
				{
					PUTS_ATT_LEFT( y, XPSTR("RTC") );
					g_eeGeneral.externalRtcType = checkIndexed( y, XPSTR("\052""\001""\006  NONEDS3231"), g_eeGeneral.externalRtcType, (sub==subN) ) ;// FWx07
	  			y += FHPY ;
					subN += 1 ;
				}
#endif
#ifdef PCBLEM1
				PUTS_ATT_LEFT( y, XPSTR("RTC") );
				g_eeGeneral.externalRtcType = checkIndexed( y, XPSTR("\052""\001""\006  NONEDS3231"), g_eeGeneral.externalRtcType, (sub==subN) ) ;// FWx07
  			y += FHPY ;
				subN += 1 ;
#endif
			
#ifndef PCBLEM1
			}
#endif
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
#ifndef PCBXLITE
 #ifndef PCBT12
			else if ( sub < page3 )
			{
				uint32_t value ;
				uint32_t oldValue ;
				subN = page2 ;
  			PUTS_ATT_LEFT( y, XPSTR("6 Pos. Switch"));
				value = ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2 ;



#ifdef PCBX9D
	  		PUTS_AT_IDX( 15*FW, y, XPSTR("\002--P1P2P3P4"), value, (sub==subN ? blink:0));
#endif



#ifdef PCBSKY
	  		PUTS_AT_IDX( 15*FW, y, XPSTR("\003---P1 P2 P3 AUX"), value, (sub==subN ? INVERS:0));
#define NUM_ANA_INPUTS	4
#endif
#ifdef PCB9XT
	  		PUTS_AT_IDX( 15*FW, y, XPSTR("\003---P1 P2 P3 "), value, (sub==subN ? INVERS:0));
#define NUM_ANA_INPUTS	3
#endif
				oldValue = value ;
#ifdef PCBX9D
 #ifdef PCBX7
				if(sub==subN)
				{
					uint32_t max = NUM_MAP_POTS ;
					if ( g_eeGeneral.extraPotsSource[0] )
					{
						max += 1 ;
					}
					if ( g_eeGeneral.extraPotsSource[1] )
					{
						max += 1 ;
					}
					CHECK_INCDEC_H_GENVAR_0( value, max ) ;
				}
 #else
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_MAP_POTS ) ;
 #endif
#else
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_ANA_INPUTS ) ;
#endif
 				g_eeGeneral.analogMapping = ( g_eeGeneral.analogMapping & ~MASK_6POS ) | ( value << 2 ) ;
				if ( value != oldValue )
				{
					createSwitchMapping() ;
				}
				y += FHPY ;
				subN += 1 ;
				if ( g_eeGeneral.analogMapping & MASK_6POS )
				{
					uint32_t i ;
					uint16_t value ;
					value = g_eeGeneral.analogMapping & MASK_6POS ;
#ifdef PCBSKY
					if ( value == USE_AUX_6POS )
					{
						value = Analog_values[9] ;
					}
					else
					{
						value = Analog_values[ (value >> 2) + 3] ;
					}
#endif
#ifdef PCB9XT
					value = M64Analog[ (value >> 2) + 3] ;
#endif
#ifdef PCBX9D
			  	value >>= 2 ;
					value += 3 ;
					if ( value > 5 )
					{
						value = 9 ;
					}
					value = Analog_values[value] ;
#endif
					PUT_HEX4( 12*FW, 0*FHPY, value ) ;
					PUTS_AT_IDX( 17*FW, 0*FHPY, PSTR(SWITCHES_STR), HSW_Ele6pos0 + switchPosition(HSW_Ele6pos0)-HSW_OFFSET, 0 ) ;

					for ( i = 0 ; i < 6 ; i += 1 )
					{
  					PUTS_ATT_LEFT( y, XPSTR("6P\003:"));
  					PUTC( 2*FW, y, i+'0' ) ;
#ifndef ARUNI
						if ( i < 5 )
						{
							PUT_HEX4( 12*FW, y, SixPositionTable[i] ) ;
						}
						PUT_HEX4( 5*FW, y, g_eeGeneral.SixPositionCalibration[i] ) ;
						if ( sub == subN )
						{
							lcd_char_inverse( 5*FW, y, 20, 0 ) ;
#if defined(PCBX12D) || defined(PCBX10)
							if ( Tevent==EVT_KEY_BREAK(KEY_MENU) || Tevent == EVT_KEY_BREAK(BTN_RE)  )
#else
							if ( Tevent==EVT_KEY_BREAK(KEY_MENU) ) // || Tevent == EVT_KEY_BREAK(BTN_RE)  )
#endif
					    {
								g_eeGeneral.SixPositionCalibration[i] = value ;
								s_editMode = 0 ;
								create6posTable() ;
							}	
						}
#endif
						y += FHPY ;
						subN += 1 ;
					}
				}
			}
 #endif // nT12
#endif // nXLITE
#endif

#ifndef PCBLEM1

#if defined(PCBX12D) || defined(PCBX10)
			else if ( sub < page3 )
			{
				displayNext() ;
				subN = page2 ;
					uint32_t i ;
					uint16_t value ;
					value = AnalogData[10] ;
					PUT_HEX4( 12*FW, 0*FH, value ) ;
					PUTS_AT_IDX( 17*FW, 0*FH, PSTR(SWITCHES_STR), HSW_Ele6pos0 + switchPosition(HSW_Ele6pos0)-HSW_OFFSET, 0 ) ;
					for ( i = 0 ; i < 6 ; i += 1 )
					{
  					PUTS_ATT_LEFT( y, XPSTR("6P\003:"));
  					PUTC( 2*FW, y, i+'0' ) ;
						if ( i < 5 )
						{
							PUT_HEX4( 12*FW, y, SixPositionTable[i] ) ;
						}
						PUT_HEX4( 5*FW, y, g_eeGeneral.SixPositionCalibration[i] ) ;
						if ( sub == subN )
						{
							lcd_char_inverse( 5*FW, y, 10*FW/3, 0 ) ;
							if ( Tevent==EVT_KEY_BREAK(KEY_MENU) ) // || Tevent == EVT_KEY_BREAK(BTN_RE)  )
					    {
								g_eeGeneral.SixPositionCalibration[i] = value ;
								s_editMode = 0 ;
								create6posTable() ;
							}	
						}
						y += FHPY ;
						subN += 1 ;
					}
			}
#endif


#if defined(PCBSKY) || defined(PCB9XT)
			else if ( sub < page3+7 )
			{
				subN = page3 ;
//ELE NONE EXT1, EXT2, EXT3, DAC1, ANA, EXT4, EXT5, EXT6
//AIL NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6
//GEA NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6
//RUD NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6
//THR NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6
//PB1 NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6
//PB1 NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6

//ELE has extra option if ANA of ANA3, AN6A, AN6B

				if ( sub >= IlinesCount )
				{
					sub = mstate.m_posVert = IlinesCount-1 ;
				}
  			PUTS_ATT_LEFT( y, XPSTR("ELE Switch"));
				uint16_t sm = g_eeGeneral.switchMapping ;
				uint8_t value = g_eeGeneral.elesource ;
#ifndef PCB9XT
				if ( value > 5 )
				{
					value += 2 ;
				}
				if ( value >101 )
				{
					value -= 102-6 ;
				}
#endif	// nPCB9XT

#ifdef REVX
				value = checkIndexed( y, XPSTR(FWx17"\007\004NONELCD2LCD6LCD7DAC1ANA 6PSA6PSB"), value, (sub==subN) ) ;
#else
#ifdef PCB9XT
				value = checkIndexed( y, XPSTR(FWx17"\010\004NONEEXT1EXT2EXT3EXT4EXT5EXT6EXT7EXT8"), value, (sub==subN) ) ;
#else
				if ( g_eeGeneral.ar9xBoard == 0 )
				{
					value = checkIndexed( y, XPSTR(FWx17"\012\004NONELCD2LCD6LCD7DAC1ANA 6PSA6PSBEXT4EXT5EXT6"), value, (sub==subN) ) ;
				}
				else
				{
					value = checkIndexed( y, XPSTR(FWx17"\007\004NONEEXT1EXT2EXT3PB14AD106PSA6PSB"), value, (sub==subN) ) ;
				}
#endif	// PCB9XT
#endif

#ifndef PCB9XT
				if ( value > 5 )
				{
					value -= 2 ;
					if ( value < 6 )
					{
						value += 102-6 ;
					}
				}
#endif	// nPCB9XT
				
				g_eeGeneral.elesource = value ;
				g_eeGeneral.switchMapping = sm & ~(USE_ELE_3POS | USE_ELE_6POS | USE_ELE_6PSB ) ;
				if ( value )
				{
					uint16_t mask = USE_ELE_3POS ;
					if ( value == 100 )
					{
						mask = USE_ELE_6POS ;
					}
					else if ( value == 101 )
					{
						mask = USE_ELE_6PSB ;
					}
					g_eeGeneral.switchMapping |= mask ;
				} 
				if ( sm != g_eeGeneral.switchMapping )
				{
					createSwitchMapping() ;
					sm = g_eeGeneral.switchMapping ;
				}
 				y += FHPY ;
				subN++;

				{
				  PUTS_ATT_LEFT( y, XPSTR("THR Switch"));
					g_eeGeneral.thrsource = edit3posSwitchSource( y, g_eeGeneral.thrsource, USE_THR_3POS, (sub==subN), 1 ) ;
				}	
 				y += FHPY ;
				subN++;

					
				{
				  PUTS_ATT_LEFT( y, XPSTR("RUD Switch"));
					g_eeGeneral.rudsource = edit3posSwitchSource( y, g_eeGeneral.rudsource, USE_RUD_3POS, (sub==subN), 1 ) ;
				}	
 				y += FHPY ;
				subN++;
					
				{
				  PUTS_ATT_LEFT( y, XPSTR("AIL Switch"));
					g_eeGeneral.ailsource = edit3posSwitchSource( y, g_eeGeneral.ailsource, USE_AIL_3POS, (sub==subN), 1 ) ;
				}	
 				y += FHPY ;
				subN++;
					
				{
				  PUTS_ATT_LEFT( y, XPSTR("GEA Switch"));
					g_eeGeneral.geasource = edit3posSwitchSource( y, g_eeGeneral.geasource, USE_GEA_3POS, (sub==subN), 1 ) ;
				}	
					
	 				y += FHPY ;
					subN++;
				
  			PUTS_ATT_LEFT( y, XPSTR("PB1 Switch\037PB2 Switch"));
				g_eeGeneral.pb1source = edit3posSwitchSource( y, g_eeGeneral.pb1source, USE_PB1, (sub==subN), 0 ) ;
				y += FHPY ;
				subN += 1 ;
				g_eeGeneral.pb2source = edit3posSwitchSource( y, g_eeGeneral.pb2source, USE_PB2, (sub==subN), 0 ) ;
			}
#endif // PCBSKY
			else
			{
#if defined(PCBSKY) || defined(PCB9XT)
				subN = page3+7 ;
#else
#ifdef REV9E
				subN = page3 ;
#else
#if defined(PCBX12D) || defined(PCBX10)
				subN = 13 ;
#else
 #ifdef PCBXLITE
				subN = 10 ;
 #else
  #ifdef PCBT12
				subN = 5 ;
	#else
				subN = page3 ;
	#endif
 #endif
#endif
#endif
#endif

#if defined(PCBSKY) || defined(PCB9XT)
  			PUTS_ATT_LEFT( y, XPSTR("PB3 Switch\037PB4 Switch"));
				g_eeGeneral.pb3source = edit3posSwitchSource( y, g_eeGeneral.pb3source, USE_PB3, (sub==subN), 0 ) ;
				y += FHPY ;
				subN += 1 ;
				g_eeGeneral.pb4source = edit3posSwitchSource( y, g_eeGeneral.pb4source, USE_PB4, (sub==subN), 0 ) ;
				y += FHPY ;
				subN += 1 ;
#endif

#if defined(PCBSKY) || defined(PCB9XT)
#ifdef PCB9XT
				editExtraPot( y, 0, uint8_t (sub==subN) ) ;
 				y += FHPY ;
				subN++;
#endif

#ifdef PCBSKY
        edit_xpot_source( y, XPSTR("POT4"), 0, (sub==subN) ) ;
 				y += FHPY ;
				subN++;
#endif

#ifdef PCB9XT
				editExtraPot( y, 1, uint8_t (sub==subN) ) ;
 				y += FHPY ;
				subN++;
#else
	        edit_xpot_source( y, XPSTR("POT5"), 1, (sub==subN) ) ;
 					y += FHPY ;
					subN++;
#endif	// PCB9XT

#endif // PCBSKY !! PCB9XT

#ifdef REV9E
  			PUTS_ATT_LEFT( y, XPSTR("POT3"));
				g_eeGeneral.extraPotsSource[0] = onoffItem( g_eeGeneral.extraPotsSource[0], y, (sub==subN) ) ;
				NumExtraPots = NUM_EXTRA_POTS - 2 + countExtraPots() ;
 				y += FHPY ;
				subN++;
  			PUTS_ATT_LEFT( y, XPSTR("POT4"));
				g_eeGeneral.extraPotsSource[1] = onoffItem( g_eeGeneral.extraPotsSource[1], y, (sub==subN) ) ;
				NumExtraPots = NUM_EXTRA_POTS - 2 + countExtraPots() ;
				y += FHPY ;
				subN++;

				uint32_t i ;
				for ( i = 0 ; i < 3 ; i += 1 )
				{
					uint32_t mask = 1 << i ;
	  			PUTS_ATT_LEFT( y, XPSTR("Switch ") ) ;
					PUTC( 7*FW, y, 'I' + i ) ;
					uint32_t temp = g_eeGeneral.ailsource & mask ;
					uint8_t value = temp ? 1 : 0 ;
					value = onoffItem( value, y, (sub==subN) ) ;
					value <<= i ;
					g_eeGeneral.ailsource = ( g_eeGeneral.ailsource & ~mask ) | value ;
					if ( value != temp )
					{
						createSwitchMapping() ;
					}
					y += FHPY ;
					subN++;
				}
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
#ifndef REV9E
				{
					uint8_t value = g_eeGeneral.potDetents & 1 ;
			  	value = onoffMenuItem( value, y, XPSTR("P1 has Detent"), sub==subN ) ;
					g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~1) | value ;
				}
 				y += FHPY ;
				subN += 1 ;

#ifndef PCBX9LITE
				{
					uint8_t value = (g_eeGeneral.potDetents >> 1) & 1 ;
			  	value = onoffMenuItem( value, y, XPSTR("P2 has Detent"), sub==subN ) ;
					g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~2) | ( value << 1 ) ;
				}
 				y += FHPY ;
				subN += 1 ;
#endif // X3
#ifndef PCBX7
 #ifndef PCBXLITE
  #ifndef PCBX9LITE
				{
					uint8_t value = (g_eeGeneral.potDetents >> 2) & 1 ;
			  	value = onoffMenuItem( value, y, XPSTR("P3/SL has Detent"), sub==subN ) ;
					g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~4) | ( value << 2 ) ;
				}
 				y += FHPY ;
				subN += 1 ;

				{
					uint8_t value = (g_eeGeneral.potDetents >> 3) & 1 ;
			  	value = onoffMenuItem( value, y, XPSTR("P4/SR has Detent"), sub==subN ) ;
					g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~8) | ( value << 3 ) ;
				}
 				y += FHPY ;
				subN += 1 ;

  #endif // X3
 #endif // PCBXLITE
#endif // PCBX7
#endif // nREV9E
#endif // PCBX9D

#if defined(PCBSKY) || defined(PCB9XT)
				LcdFlags attr = 0 ;
				if ( sub == subN )
				{
					attr = blink ;
					if ( checkForMenuEncoderBreak( event ) )
					{
		    		killEvents( event ) ;
						SubMenuCall = 0x80 + subN ;
						pushMenu(menuConfigPots) ;
					}
				}
				PUTS_ATT( 0, y, XPSTR("Pot Detents"), attr ) ;
 				y += FHPY ;
				subN += 1 ;
#endif // PCBSKY

#endif // PCBLEM1        

#ifdef PCBLEM1
			}
			else
			{
				subN = 6 ;
#endif			

				PUTS_ATT_LEFT( y,XPSTR("Stick Reverse:"));
 				y += FHPY ;
				for ( uint32_t i = 0 ; i < 4 ; i += 1 )
				{
#if defined(PCBX12D) || defined(PCBX10)
  				lcd_HiResimg( (6+4*i)*FW*2, y*2, sticksHiRes, i, 0 ) ;
#else
					lcd_img((6+4*i)*FW, y, sticks, i, 0 ) ;
#endif
					if (g_eeGeneral.stickReverse & (1<<i)) lcd_char_inverse( (6+4*i)*FW, y, 3*FW, 0 ) ;
				}
    		if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.stickReverse, 15 ) ;
					lcd_rect( 6*FW-1, y-1, 15*FW+2, 9 ) ;
				}
 				y += FHPY ;
				subN += 1 ;
			}
		}			 
		break ;

		case M_EEPROM :
		{
			uint32_t subN = 0 ;
			
			IlinesCount = 2 ;

			TITLE( XPSTR("EEPROM") ) ;
			if ( HardwareMenuEnabled == 0 )
			{
				IlinesCount = 0 ;
        PUTS_ATT_LEFT( 3*FHPY,XPSTR(" Disabled (see Help)"));
				break ;
			}
      PUTS_ATT( 0, y, XPSTR("Backup EEPROM"), (sub==subN) ? INVERS : 0 ) ;
			y += FHPY ;
			subN += 1 ;
      PUTS_ATT( 0, y,XPSTR("Restore EEPROM"), (sub==subN) ? INVERS : 0 ) ;

			if ( Tevent==EVT_KEY_BREAK(KEY_MENU) ) // || Tevent == EVT_KEY_BREAK(BTN_RE)  )
			{
  	  	s_editMode = 0 ;
				if ( sub == 0 )
				{
					pushMenu(menuBackupEeprom) ;
				}
				if ( sub == 1 )
				{
					pushMenu(menuRestoreEeprom) ;
				}
			}
		}			 
		break ;

//#define BT_TYPE_HC06		0
//#define BT_TYPE_HC05		1

#ifdef BLUETOOTH
		case M_BLUETOOTH :
		{	
			uint32_t subN = 0 ;

			if ( Tevent == EVT_ENTRY )
			{
      	BtControl.BtStateRequest = 1 ;
			}

			TITLE( XPSTR("BlueTooth") ) ;
#ifdef BLUETOOTH
 #ifdef BT_COMMAND_DEBUG
			IlinesCount = 3 ;
 #else
			IlinesCount = 5 ;
 #endif
#else
			IlinesCount = 0 ;
#endif
			y = 0 ;
			if ( g_eeGeneral.BtType >= BT_TYPE_HC05 )
			{
				IlinesCount += 3 ;
			}
			if ( g_eeGeneral.BtType == BT_TYPE_HC06 )
			{
				IlinesCount -= 2 ;
			}
			PUTS_ATT_LEFT( y,XPSTR("\013Type"));
			// Change 001 to 002 to enable CC-41
#if defined(X9LS) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
extern uint8_t BtStatus ;
			IlinesCount = (BtStatus && ( BtControl.BtMasterSlave == 2 ) ) ? 7 : 6 ;
//			g_eeGeneral.BtType = BT_TYPE_PARA ;
			checkIndexed( y, XPSTR(FWchk"\001""\005PARA "), 0, (sub==subN) ) ;
			g_eeGeneral.BtType = BT_TYPE_PARA ;
#else
			
 #ifndef SMALL
			g_eeGeneral.BtType = checkIndexed( y, XPSTR("\176""\004""\005HC-06HC-05CC-41HM-10PARA "), g_eeGeneral.BtType, (sub==subN) ) ;
 #else
			g_eeGeneral.BtType = checkIndexed( y, XPSTR("\176""\003""\005HC-06HC-05CC-41HM-10"), g_eeGeneral.BtType, (sub==subN) ) ;
 #endif
#endif
#ifdef BLUETOOTH
			BtControl.BtModuleType = 1 << g_eeGeneral.BtType ;
#endif
			y += FHPY ;
			subN += 1 ;

#ifdef BLUETOOTH
			EditType = EE_MODEL ;
			PUTS_ATT_LEFT( y, XPSTR("BT Function") );
			{
				uint32_t attr = (sub == subN) ? InverseBlink : 0 ;
				PUTS_AT_IDX( 19*FW, y, XPSTR("\007    OFF  TrnRxTrnTxRxLcdDumpTelem  Script FrPARA "), g_model.BTfunction, attr|LUA_RIGHT ) ;
				if (attr)
				{
#if defined(X9LS) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
					g_model.BTfunction = checkOutOfOrder( g_model.BTfunction, (uint8_t *)BtFunctionMap, 4 ) ;
#else
					g_model.BTfunction = checkOutOfOrder( g_model.BTfunction, (uint8_t *)BtFunctionMap, 7 ) ;
#endif 
				}
			}
			y += FHPY ;
			subN += 1 ;
			EditType = EE_GENERAL ;

extern uint8_t BtName[] ;

			LcdFlags attr = (sub == subN) ? INVERS : 0 ;
				
			SubMenuCall = 0x82 ;
			uint8_t *pname = g_eeGeneral.btName ;
			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
			{
				pname = BtName ;
			}
			alphaEditName( 7*FW, y, pname, 14, attr, 0 ) ;
			if ( event == EVT_ENTRY_UP )
			{
				// Name was edited
				if ( AlphaEdited )
				{
#ifdef BLUETOOTH
					if ( BtControl.BtNameChange == 0 )
					{
						BtControl.BtNameChange = 0x80 ;
					}
#endif
					AlphaEdited = 0 ;
				}
			}

			y += FHPY ;
			subN += 1 ;

			if ( g_eeGeneral.BtType >= BT_TYPE_HC05 )
			{
				PUTS_ATT_LEFT( y, XPSTR("BT Role") ) ;
#ifdef BLUETOOTH
	    	PUTS_AT_IDX( 19*FW, y, XPSTR("\007UNKNOWNSLAVE  MASTER "), BtControl.BtMasterSlave, ((sub==subN) ? blink:0)|LUA_RIGHT ) ;
				uint8_t newRole = 0 ;
				if ( BtControl.BtMasterSlave == 2 )
				{
					newRole = 1 ;
				}
				if ( (sub == subN) )
				{
					newRole = checkIncDec( newRole, 0, 1, 0 ) ;
					if ( newRole+1 != BtControl.BtMasterSlave )
					{
						if ( BtControl.BtRoleChange == 0 )
						{
							BtControl.BtRoleChange = 0x80 | newRole ;
						}
					}
				}
#endif
				y += FHPY ;
				subN += 1 ;
			}
			
			if ( g_eeGeneral.BtType >= BT_TYPE_HC05 )
			{
				EditType = EE_MODEL ;
#ifndef X9LS
 #if not (defined(PCBX10) && defined(PCBREV_EXPRESS))
				g_model.autoBtConnect = onoffMenuItem( g_model.autoBtConnect, y, XPSTR("Auto Connect"), sub==subN ) ;
				subN += 1 ;
				y += FHPY ;				 
 #endif
#endif
				LcdFlags attr = (sub == subN) ? INVERS : 0 ;
				PUTS_ATT( 0, y, XPSTR("Scan"), attr ) ; 
				if ( attr )
				{
					if ( checkForMenuEncoderBreak( event ) )
					{
						s_editMode = 0 ;
						SubMenuCall = 0x85 ;
    		   	pushMenu( menuProcBtScan ) ;
					}
				}
				EditType = EE_GENERAL ;
				subN += 1 ;
#ifndef X9LS
 #if not (defined(PCBX10) && defined(PCBREV_EXPRESS))

				attr = (sub == subN) ? INVERS : 0 ;
				PUTS_ATT( 10*FW, y, XPSTR("Configure"), attr ) ; 
				if ( attr )
				{
					if ( checkForMenuEncoderBreak( event ) )
					{
						s_editMode = 0 ;
						SubMenuCall = 0x86 ;
   		   		pushMenu( menuProcBtConfigure ) ;
					}
				}
				subN += 1 ;
				y += FHPY ;
 #endif
#endif

#if defined(X9LS) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
#ifndef SMALL
				if ( g_eeGeneral.BtType == BT_TYPE_PARA )
				{
					PUTS_ATT_LEFT( 6*FHPY, BtStatus ? "Connected" : "Cleared" ) ; 
				}
				else
#endif
#endif
				{
extern uint8_t BtState[] ;
					PUTS_ATT_LEFT( 6*FHPY, (char *)BtState ) ; 
				}
extern uint8_t BtPswd[] ;

				attr = (sub == subN) ? INVERS : 0 ;
				PUTS_ATT( 12*FW, y, XPSTR("List"), attr ) ; 
				
				if ( attr )
				{
					if ( checkForMenuEncoderBreak( event ) )
					{
						s_editMode = 0 ;
						SubMenuCall = 0x87 ;
    			  pushMenu( menuProcBtAddressList ) ;
					}
				}
				subN += 1 ;
				y += FHPY ;

#if defined(X9LS) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
				if ( ( g_eeGeneral.BtType == BT_TYPE_PARA ) && BtStatus && (BtControl.BtMasterSlave == 2) )
				{
					attr = (sub == subN) ? INVERS : 0 ;
					PUTS_ATT( 10*FW, y, XPSTR("Clear"), attr ) ; 
					if ( attr )
					{
						if ( checkForMenuEncoderBreak( event ) )
						{
							s_editMode = 0 ;
							SubMenuCall = 0x86 ;
							BtControl.BtLinkRequest = 0x40 ;
						}
					}
				}
#endif

				if ( ( g_eeGeneral.BtType == BT_TYPE_PARA ) || ( g_eeGeneral.BtType == BT_TYPE_HC05 ) )
				{
					PUTS_ATT_LEFT( FHPY*7, XPSTR("MAC:")) ;
extern uint8_t BtMAC[] ;
					PUTS_ATT( 4*FW, 7*FHPY, (char *)BtMAC, 0 ) ;
				}
				else
				{
					PUTS_ATT_LEFT( FHPY*7, XPSTR("Pswd\013Br")) ;
					PUTS_ATT( 5*FW-3, 7*FHPY, (char *)BtPswd, 0 ) ;
#ifdef BLUETOOTH
  				PUTS_AT_IDX(  19*FW, 7*FHPY, XPSTR(BtBaudString), BtControl.BtCurrentBaudrate, LUA_RIGHT ) ;
#endif
				}	 

			}
#endif

		}	
		break ;
#endif

	}
}

#if defined(PCBSKY) || defined(PCB9XT)
void menuConfigPots(uint8_t event)
{
	uint32_t i ;
	uint32_t nPots ;
	uint32_t j ;
	
#ifdef ARUNI
	nPots = 3 + countExtraPots(&ExtraPotBits);
#else	
	nPots = 3 + countExtraPots() ;
#endif

#ifdef PCB9XT
	if ( nPots > 7 )
	{
		nPots = 7 ;
	}
#endif
	TITLE( XPSTR( "Config. Pots") ) ;
	static MState2 mstate2 ;
	mstate2.check_columns( event, nPots-1 ) ;
	uint32_t sub = mstate2.m_posVert ;
	for ( i = 0 ; i < nPots ; i += 1 )
	{
		uint32_t k ;
		uint8_t b ;
		j = 1 << i ;
		b = g_eeGeneral.potDetents & j ? 1 : 0 ;
    coord_t y = (i+1) * FH ;
		k = i + 5 ;
		if ( i > 2 )
		{
			k = EXTRA_POTS_START + i - 3 ;
		}
		putsChnRaw( 0, y, k, 0 ) ;
		PUTS_ATT_LEFT( y, "\004has Detent" ) ;
		k = (sub == i) ? InverseBlink : 0  ;
		menu_lcd_onoff( PARAM_OFS+1, y, b, k ) ;
  	if( k )
		{
			CHECK_INCDEC_H_MODELVAR_0( b, 1 ) ;
			if ( b )
			{
				g_eeGeneral.potDetents |= j ;
			}
			else
			{
				g_eeGeneral.potDetents &= ~j ;
			}
		}
	}
}
#endif


