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

#include "timers.h"
#include "analog.h"

#include "X12D/hal.h"

#define SCREEN_LINES		15

#define PARAM_OFS   17*FW

uint32_t doPopup( const char *list, uint16_t mask, uint8_t width, uint8_t event ) ;
uint32_t checkForMenuEncoderBreak( uint8_t event ) ;
uint32_t checkForMenuEncoderLong( uint8_t event ) ;
uint8_t checkIndexed( uint8_t y, const char *s, uint8_t value, uint8_t edit ) ;
int8_t edit_dr_switch( uint8_t x, uint8_t y, int8_t drswitch, uint8_t attr, uint8_t flags, uint8_t event ) ;
void copyFileName( char *dest, char *source, uint32_t size ) ;
uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext ) ;
uint8_t onoffMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition ) ;
void menuProcSelectVoiceFile(uint8_t event) ;
void menu_lcd_onoff( uint8_t x,uint8_t y, uint8_t value, uint8_t mode ) ;
int8_t checkIncDec( int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags) ;
void lcd_xlabel_decimal( uint8_t x, uint8_t y, uint16_t value, uint8_t attr, const char *s ) ;
void menuCellScaling(uint8_t event) ;
uint8_t checkOutOfOrder( uint8_t value, uint8_t *options, uint32_t count ) ;
uint8_t mapPots( uint8_t value ) ;
uint8_t unmapPots( uint8_t value ) ;
uint8_t putsTelemetryChannel(uint8_t x, uint8_t y, int8_t channel, int16_t val, uint8_t att, uint8_t style, uint16_t colour = LcdForeground, uint16_t bgColour = LcdBackground ) ;
void lcdDrawIcon( uint16_t x, uint16_t y, const uint8_t * bitmap, uint8_t type ) ;
void putsTimexxl( uint16_t x, uint16_t y, int16_t tme, uint8_t att, uint16_t colour ) ;

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

extern uint8_t InverseBlink ;
extern uint8_t ImageDisplay ;
extern uint8_t scroll_disabled;
extern uint8_t StickScrollTimer ;

extern uint8_t EditType ;
extern uint8_t EditColumns ;
extern uint8_t s_currIdx;

static uint8_t s_currSubIdx;

uint8_t SuppressStatusLine ;

void putsValue(uint8_t x, uint8_t y, uint16_t index, int16_t val, uint8_t att, uint8_t style, uint16_t colour = LcdForeground, uint16_t bgColour = LcdBackground )
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
	lcd_outdezNAtt( (style & TELEM_VALUE_RIGHT) ? x+62 : x, y, val, att, 5, colour, bgColour ) ;
}

uint8_t hronoffMenuItem( uint8_t x, uint8_t value, uint8_t y, const prog_char *s, uint8_t edit )
{
	lcd_puts_Pleft(y, s) ;
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
	lcd_putsAtt( 0, 0, PSTR(STR_ANA),INVERS) ;
	
	static MState2 mstate2;
	StickScrollAllowed = 0 ;
	StickScrollTimer = 0 ;
	mstate2.check_columns(event, NUM_ANA_ITEMS-1) ;

	ImageDisplay = 0 ;

  int8_t  sub    = mstate2.m_posVert ;
  for(i=0; i<10; i++)
  {
    uint8_t y=(i+2)*FH ;
		lcd_putsAttIdx( 9*FW-3, y, XPSTR("\002LHLVRVRHA5A6A7A8BT6P"), i, 0 ) ;
		uint8_t index = i ;
		if ( i < 4 )
		{
 			index = stickScramble[g_eeGeneral.stickMode*4+i] ;
 		}
    if(i<8)
		{
			lcd_outdezAtt( 20*FW, y, (int32_t)calibratedStick[index]*1000/1024, PREC1);
  		lcd_outhex4( 11*FW, y,anaIn(i));
		}
		else if(i==8)
		{
			putsVBat(19*FW,y,(sub==1 ? InverseBlink : 0)|PREC1);
    	lcd_outhex4( 11*FW, y,anaIn(12));
		}
		else
		{
  		lcd_outhex4( 11*FW, y,anaIn(10));
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

#define MUS_OFF_0			(7*FW)

#ifndef TOUCH

void menuModelMusic(uint8_t event)
{
	uint8_t subN = 0 ;
 	TITLE( PSTR( STR_Music ) ) ;
	uint8_t attr = 0 ;
	EditType = EE_MODEL ;
	uint8_t y = 1*FH ;
	static MState2 mstate2;
	
	uint32_t rows = 5 ;
	if ( MusicPlaying == MUSIC_STOPPED )
	{
		rows = 7 ;
	}
	if ( ( MusicPlaying == MUSIC_PLAYING ) || ( MusicPlaying == MUSIC_PAUSED ) )
	{
		rows = 8 ;
	}
	
	mstate2.check_columns(event, rows) ;
  int8_t sub = mstate2.m_posVert ;

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
			 
	DisplayOffset = MUS_OFF_0 ;

  if(sub==subN)
	{
	  attr = InverseBlink ;
	}
	lcd_puts_Pleft( y, XPSTR("Start Switch"));
	uint8_t doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
	g_model.musicData.musicStartSwitch = edit_dr_switch( 17*FW+MUS_OFF_0, y, g_model.musicData.musicStartSwitch, attr, doedit, event ) ;
	y += FH ;
	subN++ ;
	attr = 0 ;
  if(sub==subN)
	{
	  attr = InverseBlink ;
	}
	lcd_puts_Pleft( y, XPSTR("Pause Switch"));
	doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
	g_model.musicData.musicPauseSwitch = edit_dr_switch( 17*FW+MUS_OFF_0, y, g_model.musicData.musicPauseSwitch, attr, doedit, event ) ;
	y += FH ;
	subN++ ;
	attr = 0 ;
  if(sub==subN)
	{
	  attr = InverseBlink ;
	}
	lcd_puts_Pleft( y, XPSTR("Previous Switch"));
	doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
	g_model.musicData.musicPrevSwitch = edit_dr_switch( 17*FW+MUS_OFF_0, y, g_model.musicData.musicPrevSwitch, attr, doedit, event ) ;
	y += FH ;
	subN++ ;
	attr = 0 ;
  if(sub==subN)
	{
	  attr = InverseBlink ;
	}
	lcd_puts_Pleft( y, XPSTR("Next Switch"));
	doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
	g_model.musicData.musicNextSwitch = edit_dr_switch( 17*FW+MUS_OFF_0, y, g_model.musicData.musicNextSwitch, attr, doedit, event ) ;

	
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

	y += FH ;
	subN++ ;

  lcd_puts_Pleft( y, XPSTR("Type")) ;
	
	uint32_t b = g_eeGeneral.musicType ;
	g_eeGeneral.musicType = checkIndexed( y, XPSTR(FWx17"\001""\004NameList"), b, (sub==subN) ) ;
	if ( g_eeGeneral.musicType != b )
	{
		g_eeGeneral.musicVoiceFileName[0] = '\0' ;
	}
	y += FH ;
	subN++ ;

  lcd_puts_Pleft( y, XPSTR("File")) ;
	lcd_putsAtt( 7*FW+MUS_OFF_0, y, (char *)g_eeGeneral.musicVoiceFileName, 0 ) ;
	if (sub==subN)
	{
		lcd_rect( 7*FW-1+MUS_OFF_0, y-1, MUSIC_NAME_LENGTH*FW+2, 9 ) ;
	}
	y += FH ;
	subN++ ;

	g_eeGeneral.musicLoop = hronoffMenuItem( 19*FW+MUS_OFF_0, g_eeGeneral.musicLoop, y, XPSTR("Loop"), (sub==subN) ) ;

	y += FH ;
	subN++ ;
	
	if ( rows == 7 )
	{
  	lcd_puts_Pleft( y, XPSTR("Start")) ;
		if ( sub == subN )
		{
			lcd_char_inverse( 0+MUS_OFF_0, y, 30, 0 ) ;
		}
		if ( sub == 8 )
		{
			mstate2.m_posVert = 7 ;
		}
	}
	else
	{
		lcd_puts_Pleft( y, XPSTR("Stop")) ;
		if ( sub == subN )
		{
			lcd_char_inverse( 0+MUS_OFF_0, y, 24, 0 ) ;
		}
		y += FH ;
		subN++ ;
		lcd_puts_Pleft( y, MusicPlaying == MUSIC_PAUSED ? XPSTR("\010Resume") : XPSTR("\010Pause")) ;
		if ( sub==subN )
		{
			lcd_char_inverse( 8*FW+MUS_OFF_0, y, 36, 0 ) ;
		}
		if ( g_eeGeneral.musicType )
		{
			if ( MusicPlaying == MUSIC_PLAYING )
			{
				lcd_puts_Pleft( y, XPSTR("Prev<\021>Next")) ;
			}
		}
		lcd_puts_Pleft( y+2*FH, CurrentPlayName ) ;
	}

	uint16_t percent = (BgTotalSize - BgSizePlayed) * 200 / BgTotalSize ;
	uint16_t x = 20 ;
	uint16_t yb = 13*FH ;
	uint16_t w = 201 ;
	uint16_t h = 9 ;
	uint16_t solid ;
	if ( percent > 200 )
	{
		percent = 200 ;
	}
	solid = (w-2) * percent / 200 ;
	lcd_rect( x, yb, w, h ) ;

	if ( solid )
	{
		w = yb + h - 1 ;
		yb += 1 ;
		x += 1 ;
		while ( yb < w )
		{
 			lcd_hline(x, yb, solid ) ;
			yb += 1 ;			
		}
	}
}

#endif // nTOUCH
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

#define TEL_OFS_0		(7*FW)

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

#if not (defined(PCBX10))
#define TDATAITEMS	22
#else
#define TDATAITEMS	19
#endif
		 
	TITLE(PSTR(STR_TELEMETRY));
	static MState2 mstate2;
	event = mstate2.check_columns( event, TDATAITEMS-1+page2SizeExtra ) ;

	DisplayOffset = TEL_OFS_0 ;
	
	uint8_t  sub   = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz ;
	uint8_t blink;
	uint8_t y = 2*FH;

	blink = InverseBlink ;
	uint8_t subN = 0 ;
 
	if ( sub < 9+page2SizeExtra )
 	{
		lcd_puts_P( 22*FW+TEL_OFS_0, 14*FH, XPSTR("->") ) ;
		lcd_puts_Pleft(FH, PSTR(STR_USR_PROTO));
		{
			uint8_t b ;
			uint8_t attr = 0 ;

			b = g_model.telemetryProtocol ;
			if(sub==subN)
			{
				Columns = 1 ;

				if (subSub==0)
				{
					attr = blink ;
					b = checkOutOfOrder( b, (uint8_t *)TelOptions, MAX_TEL_OPTIONS ) ;
				}
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
			lcd_putsAttIdx( 12*FW+2+TEL_OFS_0, FH, PSTR(STR_FRHUB_WSHHI), b, attr ) ;

			attr = 0 ;
			b = g_model.FrSkyImperial ;
			if(sub==subN && subSub==1)
			{
				attr = blink ;
				CHECK_INCDEC_H_MODELVAR_0(b,1); g_model.FrSkyImperial = b ;
			}
			lcd_putsAttIdx( 20*FW+1+TEL_OFS_0, FH, PSTR(STR_MET_IMP), b, attr ) ;
	
		}
		subN++;

		for (int i=0; i<4; i++)
		{
			uint32_t index = i & 1 ;
			uint8_t unit ;
  	  lcd_puts_Pleft(y, PSTR(STR_A_CHANNEL)) ;
  	  lcd_putc(FW+TEL_OFS_0, y, '1'+i);
			if ( i < 2 )
			{
  	  	putsTelemValue(18*FW+TEL_OFS_0, y, 255, i, (sub==subN && subSub==0 ? blink:0)|NO_UNIT ) ;
  	  	putsTelemValue( 23*FW+TEL_OFS_0, y, frskyTelemetry[i].value, i,  NO_UNIT ) ;
  	  	unit = g_model.frsky.channels[index].units ;

  	  	if (sub==subN)
				{
					Columns = 1 ;
					if ( (s_editMode || P1values.p1valdiff))
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
				lcd_outdezAtt( 18*FW+TEL_OFS_0, y, g_model.frsky.channels[index].ratio3_4, (sub==subN && subSub==0 ? blink:0)|PREC2 ) ;
				lcd_outdezAtt( 23*FW+TEL_OFS_0, y, FrskyHubData[FR_A3+index], PREC2 ) ;
  	  	unit = g_model.frsky.channels[index].units3_4 ;
  	  	if (sub==subN)
				{
					Columns = 1 ;
					if ( (s_editMode || P1values.p1valdiff))
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
  	  lcd_putsAttIdx(18*FW+TEL_OFS_0, y, XPSTR("\001v-VA"), unit, (sub==subN && subSub==1 ? blink:0));
  	 	subN++; y+=FH;

		}
		uint8_t attr = (sub==subN) ? InverseBlink : 0 ;
		lcd_xlabel_decimal( 17*FW+1+TEL_OFS_0, y, g_model.rxVratio, attr|PREC1, XPSTR( "Rx Voltage") ) ;
  	lcd_putc(Lcd_lastPos, y, 'v' ) ;
//		lcd_outdezAtt( 22*FW+1+TEL_OFS_0, y, convertRxv( FrskyHubData[FR_RXV] ), PREC1 ) ;
		lcd_outdezAtt( 22*FW+1+TEL_OFS_0, y, FrskyHubData[FR_RXV], PREC1 ) ;
		if( attr) { g_model.rxVratio = checkIncDec16( g_model.rxVratio, 0, 255, EE_MODEL ) ; }
		subN++; y+=FH;

		lcd_putsAtt( 0+TEL_OFS_0, y, XPSTR("Cell Scaling"), sub==subN ? INVERS : 0 ) ;
		if ( sub == subN )
		{
			if ( checkForMenuEncoderLong( event ) )
			{
  	  	pushMenu( menuCellScaling ) ;
			}
		}
		subN++; y+=FH;
		uint8_t b ;

		if (sub==subN)
		{
			Columns = 1 ;
		}
 		lcd_puts_Pleft( y, XPSTR( "RSSI Warn""\037""RSSI Critical") ) ;
		attr = ( ( (sub==subN) && (subSub==0) ) ? InverseBlink : 0) ;
		int8_t offset ;
		offset = rssiOffsetValue( 0 ) ;
		lcd_outdezAtt( 19*FW+TEL_OFS_0, y, g_model.rssiOrange + offset, attr ) ;
  	if( attr) CHECK_INCDEC_H_MODELVAR( g_model.rssiOrange, -18, 30 ) ;
		attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
		b = 1-g_model.enRssiOrange ;
		menu_lcd_onoff( PARAM_OFS+5*FW+TEL_OFS_0, y, b, attr ) ;
  	if( attr) { CHECK_INCDEC_H_MODELVAR_0( b, 1 ) ; g_model.enRssiOrange = 1-b ; }
		subN++; y+=FH;
		
		if (sub==subN)
		{
			Columns = 1 ;
		}
		attr = ( ( (sub==subN) && (subSub==0) ) ? InverseBlink : 0) ;
		offset = rssiOffsetValue( 1 ) ;
		lcd_outdezAtt( 19*FW+TEL_OFS_0, y, g_model.rssiRed + offset, attr ) ;
		if( attr) CHECK_INCDEC_H_MODELVAR( g_model.rssiRed, -17, 30 ) ;
		attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
		b = 1-g_model.enRssiRed ;
		menu_lcd_onoff( PARAM_OFS+5*FW+TEL_OFS_0, y, b, attr ) ;
  	if( attr) { CHECK_INCDEC_H_MODELVAR_0( b, 1 ) ; g_model.enRssiRed = 1-b ; }

		subN++; y+=FH;
 
		if ( page2SizeExtra )
		{
 			lcd_puts_Pleft( y, XPSTR( "DSM 'A' as RSSI") ) ;
 			menu_lcd_onoff( PARAM_OFS+5*FW+TEL_OFS_0, y, g_model.dsmAasRssi, sub==subN ) ;
  		if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.dsmAasRssi, 1);
			subN++; y+=FH;
		
			if (sub==subN)
			{
				Columns = 1 ;
			}
 			lcd_puts_Pleft( y, XPSTR( "DSM Warning") ) ;
		
			g_model.dsmLinkData.sourceWarn = checkIndexed( y, XPSTR(FWx13"\002""\006fades lossesholds "), g_model.dsmLinkData.sourceWarn, (sub==subN) && (subSub==0) ) ;
			attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
			lcd_outdezAtt( 23*FW+TEL_OFS_0, y, g_model.dsmLinkData.levelWarn, attr ) ;
  		if( attr) { CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.levelWarn, 40 ) ; }
			subN++; y+=FH;

			if (sub==subN)
			{
				Columns = 1 ;
			}
 			lcd_puts_Pleft( y, XPSTR( "DSM Critical") ) ;
			g_model.dsmLinkData.sourceCritical = checkIndexed( y, XPSTR(FWx13"\002""\006fades lossesholds "), g_model.dsmLinkData.sourceCritical, (sub==subN) && (subSub==0) ) ;
			attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
			lcd_outdezAtt( 23*FW+TEL_OFS_0, y, g_model.dsmLinkData.levelCritical, attr ) ;
  		if( attr) { CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.levelCritical, 40 ) ; }
			subN++; y+=FH;

 			lcd_puts_Pleft( y, XPSTR( "DSM Vario") ) ;
			g_model.dsmVario = checkIndexed( y, XPSTR("\163" "\005" "\0040.250.5 1.0 1.5 2.0 3.0 "), g_model.dsmVario, (sub==subN) ) ;
			subN++; y+=FH;
		}

 	}
	else //if ( sub < 14+page2SizeExtra) // sub>=8
	{
		uint8_t subN = 9+page2SizeExtra ;
		y = FH ;
		
		uint8_t attr = 0 ;
  	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR( g_model.numBlades, 1, 127 ) ; }
		lcd_xlabel_decimal( PARAM_OFS+6*FW+TEL_OFS_0, y, g_model.numBlades, attr, PSTR(STR_NUM_BLADES) ) ;
		y += FH ;
		subN++;

		attr = 0 ;
  	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.FrSkyAltAlarm, 2 ) ; }
		lcd_puts_Pleft( y, PSTR(STR_ALT_ALARM));
  	lcd_putsAttIdx(PARAM_OFS+3*FW+2+TEL_OFS_0, y, PSTR(STR_OFF122400),g_model.FrSkyAltAlarm, attr ) ;
  	y += FH ;
  	subN++;
  
		lcd_puts_Pleft( y, PSTR(STR_VOLT_THRES));
  	lcd_outdezNAtt(  PARAM_OFS+6*FW+TEL_OFS_0, y, g_model.frSkyVoltThreshold * 2 ,((sub==subN) ? blink:0) | PREC2, 4);
  	if(sub==subN)
		{
  	  g_model.frSkyVoltThreshold=checkIncDec16( g_model.frSkyVoltThreshold, 0, 210, EE_MODEL);
  	}
  	y += FH ;
		subN++;
	
  	lcd_puts_Pleft( y, PSTR(STR_GPS_ALTMAIN) ) ;
  	menu_lcd_onoff( PARAM_OFS+5*FW+TEL_OFS_0, y, g_model.FrSkyGpsAlt, sub==subN ) ;
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.FrSkyGpsAlt, 1);
  	y += FH ;
		subN++;
		
		if (sub==subN)
		{
			Columns = 1 ;
		}
		uint16_t value = g_model.frskyAlarms.alarmData[0].frskyAlarmLimit << 6 ;
		lcd_puts_Pleft(y, PSTR(STR_MAH_ALARM));
  	attr = ((sub==subN && subSub==0) ? InverseBlink : 0);
		uint8_t active = (attr && s_editMode) ;
  	lcd_outdezAtt( 15*FW+TEL_OFS_0, y, value, attr ) ;
		if ( active )
		{
  		g_model.frskyAlarms.alarmData[0].frskyAlarmLimit = checkIncDec16( g_model.frskyAlarms.alarmData[0].frskyAlarmLimit, 0, 200, EE_MODEL);
		}
  	attr = ((sub==subN && subSub==1) ? InverseBlink : 0);
		active = (attr && s_editMode) ;
		lcd_putsAttIdx(PARAM_OFS+1*FW+1+TEL_OFS_0, y, PSTR(STR_SOUNDS), g_model.frskyAlarms.alarmData[0].frskyAlarmSound,attr);
		if ( active )
		{
  		CHECK_INCDEC_H_MODELVAR_0( g_model.frskyAlarms.alarmData[0].frskyAlarmSound, 15 ) ;
		}
		subN++; y+=FH;

  	lcd_puts_Pleft( y, PSTR(STR_BT_TELEMETRY) );
  	menu_lcd_onoff( PARAM_OFS+5*FW+TEL_OFS_0, y, g_model.bt_telemetry, sub==subN ) ;
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.bt_telemetry, 1);
  	y += FH ;
		subN += 1 ;
  	
		lcd_puts_Pleft( y, PSTR(STR_FRSKY_COM_PORT) );
    attr = (sub == subN) ? blink : 0 ;
  	lcd_putcAtt( PARAM_OFS+5*FW+TEL_OFS_0, y, g_model.frskyComPort + '1', attr ) ;
		if (attr) CHECK_INCDEC_H_MODELVAR_0( g_model.frskyComPort, 1 ) ;
	  y += FH ;
		subN += 1 ;

		uint8_t previous = g_model.telemetryRxInvert ;
		previous |= g_model.telemetry2RxInvert << 1 ;
		uint8_t next = previous ;

		lcd_puts_Pleft( y, XPSTR("Com Port Invert") );
		attr = 0 ;
		if(sub==subN)
		{
			attr = InverseBlink ;
			CHECK_INCDEC_H_MODELVAR_0( next, 1 ) ;
		}
		lcd_putsAttIdx(PARAM_OFS+2*FW+TEL_OFS_0, y, XPSTR("\004None   1   2"), next, attr ) ;

		if ( next != previous )
		{
			g_model.telemetryRxInvert = next & 0x01 ;
			g_model.telemetry2RxInvert = (next >> 1 ) & 0x01 ;
			TelemetryType = TEL_UNKNOWN ;
		}
	  y += FH ;
		subN += 1 ;
		 
		lcd_puts_Pleft( y, PSTR(STR_FAS_OFFSET) );
    attr = PREC1 ;
		if ( (sub == subN) )
		{
			attr = blink | PREC1 ;
      CHECK_INCDEC_H_MODELVAR_0( g_model.FASoffset, 15 ) ;
		}
  	lcd_outdezAtt( PARAM_OFS+5*FW+5+TEL_OFS_0, y, g_model.FASoffset, attr ) ;
	  y += FH ;
		subN += 1 ;

#if not (defined(PCBX10))
		lcd_puts_Pleft( y, XPSTR("COM2 Func.") );
		uint8_t b = g_model.com2Function ;
    attr = 0 ;
		if ( (sub == subN) )
		{
			attr = blink ;
			b = checkOutOfOrder( b, (uint8_t *)Com2Options, MAX_COM2_OPTIONS ) ;
		}
		lcd_putsAttIdx(PARAM_OFS+TEL_OFS_0, y, XPSTR("\011TelemetrySbusTrainSbus57600BTdirect CppmTrain"), g_model.com2Function, attr ) ;
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
	  y += FH ;
		subN += 1 ;
		
		lcd_puts_Pleft( y, XPSTR("COM2 Baudrate") );
		b = g_model.com2Baudrate ;

		g_model.com2Baudrate = checkIndexed( y, XPSTR(BaudString), g_model.com2Baudrate, (sub==subN) ) ;

	  y += FH ;
		subN += 1 ;
#endif

		attr = (sub==subN) ? blink : 0 ;
		lcd_puts_Pleft( y, XPSTR("Current Source" ) ) ;
		lcd_putsAttIdx( PARAM_OFS+2*FW+TEL_OFS_0, y, XPSTR("\004----A1  A2  Fas SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "), g_model.currentSource, attr ) ;
   	if(attr)
		{
			CHECK_INCDEC_H_MODELVAR_0( g_model.currentSource, 3 + NUM_SCALERS ) ;
	  }
		subN += 1 ;
	}
}


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
			lcdHiresPutsTransparent( xpos + 5, y+4, (char *)"CH", lineColour ) ;

			if ( start > 8 )
			{
				lcdDrawCharBitmapDma( xpos + 5 + 24, y+4, (start+1)/10+'0', 0, lineColour ) ;
				lcdDrawCharBitmapDma( xpos + 5 + 24+12, y+4, (start+1)%10+'0', 0, lineColour ) ;
			}
			else
			{
				lcdDrawCharBitmapDma( xpos + 5 + 24, y+4, start+'1', 0, lineColour ) ;
			}

			lcdDrawCharBitmapDma( xpos + BARS_WIDTH * 2 - 20, y+4, '%', 0, lineColour ) ;
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
			lcdDrawCharBitmapDma( x, y+4, chr, 0, lineColour ) ;
			x -= 12 ;
			value /= 10 ;
			chr = value % 10 + '0' ;
			if ( value )
			{
				lcdDrawCharBitmapDma( x, y+4, chr, 0, lineColour ) ;
				x -= 12 ;
			}
			value /= 10 ;
			chr = value % 10 + '0' ;
			if ( value )
			{
				lcdDrawCharBitmapDma( x, y+4, chr, 0, lineColour ) ;
				x -= 12 ;
			}
			if ( negative )
			{
				lcdDrawCharBitmapDma( x, y+4, '-', 0, lineColour ) ;
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
				putsChnRaw( x+1, y+1, index+1, attr, colour, bgcolour ) ;
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
					displayGPSdata( x-9*FW, y, FrskyHubData[ FR_GPS_LAT], FrskyHubData[FR_GPS_LATd], LEADING0 | attr, FrskyHubData[FR_LAT_N_S] ) ;
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
					displayGPSdata( x-9*FW, y, FrskyHubData[ FR_GPS_LONG], FrskyHubData[FR_GPS_LONGd], LEADING0 | attr, FrskyHubData[FR_LONG_E_W] ) ;
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
			putsChnRaw( x+1, y+1, index+1, attr, colour, bgcolour ) ;
			if ( ( h > 35 ) && ( w > 100 ) )
			{
				x *= 2 ;
				y *= 2 ;
				w *= 2 ;
				h *= 2 ;
				x += (w - 200 ) / 2 ;
				y += (h-48)/2 + 7 ;
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
	uint8_t y = 2*FH ;
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

	lcd_puts_Pleft( y, XPSTR("Type") ) ;
	display->boxes[s_currIdx].type = checkIndexed( y, options, display->boxes[s_currIdx].type, (sub==subN) ) ;

	subN += 1 ;
	y += FH ;
	uint8_t attr = (sub==subN) ? InverseBlink : 0 ;
	
	if ( display->boxes[s_currIdx].type == HIRES_TYPE_BARS )
	{
		lcd_puts_Pleft( y, XPSTR("Start") ) ;
 		lcd_outdezAtt(PARAM_OFS+2*FW, y, display->boxes[s_currIdx].item+1,attr );
  	if (sub==subN)
		{
			CHECK_INCDEC_H_MODELVAR_0( display->boxes[s_currIdx].item, 30 ) ;
		}
	}
	else
	{
		lcd_puts_Pleft( y, XPSTR("Item") ) ;
		if ( display->boxes[s_currIdx].type == HIRES_TYPE_TIMER )
		{
			display->boxes[s_currIdx].item = checkIndexed( y, FWx9"\001""\006Timer1Timer2", display->boxes[s_currIdx].item, (sub==subN) ) ;
		}
		else
		{
			if ( display->boxes[s_currIdx].item )
			{
				putsChnRaw( FW*9, y, display->boxes[s_currIdx].item, attr | TSSI_TEXT ) ;
			}
			else
			{
  			lcd_putsAtt(  FW*9, y, HyphenString, attr ) ;
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
	y += FH ;
	
	red = display->boxes[s_currIdx].colour >> 11 ;
	green = ( display->boxes[s_currIdx].colour >> 6 ) & 0x1F ;
	blue = display->boxes[s_currIdx].colour & 0x1F ;

	uint8_t blink = InverseBlink ;

	lcd_puts_Pleft( y, XPSTR("Text Red")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, red,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( red, 31 ) ;
	y += FH ;
	subN += 1 ;
	 
	lcd_puts_Pleft( y, XPSTR("\005Green")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, green,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( green, 31 ) ;
	y += FH ;
	subN += 1 ;
	
	lcd_puts_Pleft( y, XPSTR("\005Blue")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, blue,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( blue, 31 ) ;
	
	display->boxes[s_currIdx].colour = ( red << 11 ) | ( green << 6 ) | ( blue ) ;

	y += FH ;
	subN += 1 ;

	red = display->boxes[s_currIdx].bgColour >> 11 ;
	green = ( display->boxes[s_currIdx].bgColour >> 6 ) & 0x1F ;
	blue = display->boxes[s_currIdx].bgColour & 0x1F ;

	lcd_puts_Pleft( y, XPSTR("Back Red")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, red,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( red, 31 ) ;
	y += FH ;
	subN += 1 ;
	 
	lcd_puts_Pleft( y, XPSTR("\005Green")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, green,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( green, 31 ) ;
	y += FH ;
	subN += 1 ;
	
	lcd_puts_Pleft( y, XPSTR("\005Blue")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, blue,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( blue, 31 ) ;
	
	display->boxes[s_currIdx].bgColour = ( red << 11 ) | ( green << 6 ) | ( blue ) ;
	
	lcdDrawSolidFilledRectDMA( 350, 2*(y-3*FH), 60, 20, display->boxes[s_currIdx].bgColour ) ;
	lcd_putsnAttColour( 177, (y-3*FH+1), "Text", 4, 0, display->boxes[s_currIdx].colour, display->boxes[s_currIdx].bgColour ) ;
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
	
	lcd_puts_Pleft( 2*FH, XPSTR("Layout") ) ;

	pushPlotType( PLOT_BLACK ) ;
	lcd_rect( 5, 3*FH+4, 128, 64+4 ) ;
	for ( i = 0 ; i < j ; i += 1 )
	{
		x = *l++ / 2 + 10 ;
		y = *l++ / 2 + 4*FH ;
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


#define HRES_OFF_0			(8*FW)

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

	lcd_puts_Pleft( 2*FH, XPSTR("Screen") ) ;
	lcd_outdezAtt( PARAM_OFS+2*FW+HRES_OFF_0, 2*FH, s_currSubIdx, (sub==0) ? InverseBlink:0 ) ;

	lcd_puts_Pleft( 3*FH, XPSTR("Layout") ) ;
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
	lcd_puts_Pleft( 6*FH, XPSTR("Edit Contents") ) ;
	uint32_t x ;
	x = display->options & HIRES_OPT_TRIMS ;
	x = onoffMenuItem( x, 7*FH, XPSTR("Include Trims"), ( sub == 3 ) ) ;
	display->options = (display->options & ~HIRES_OPT_TRIMS ) | x ;
	x = display->options & HIRES_OPT_BORDERS ;
	x = onoffMenuItem( x, 8*FH, XPSTR("Display Borders"), ( sub == 4 ) ) ;
	display->options = (display->options & ~HIRES_OPT_BORDERS ) | ( x ? HIRES_OPT_BORDERS : 0 ) ;
	lcd_puts_Pleft( 9*FH, XPSTR("Test") ) ;
	
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
		lcd_char_inverse( HRES_OFF_0+0, 6*FH, 13*6, 0 ) ;
		if (checkForMenuEncoderBreak( event ) )
		{
			// To Do			
      pushMenu( menuHiresContent ) ;
		}
	}	
	else if ( sub == 5 )
	{
		lcd_char_inverse( HRES_OFF_0+0, 9*FH, 4*6, 0 ) ;
		if (checkForMenuEncoderBreak( event ) )
		{
      pushMenu( menuHiresTest ) ;
		}
	}	
}

#ifdef TOUCH
extern uint8_t TlExitIcon ;
uint16_t dimBackColour() ;
void drawItem( char *s, uint16_t y, uint16_t colour ) ;
void drawNumber( uint16_t x, uint16_t y, int32_t val, uint16_t mode) ; //, uint16_t colour ) ;
extern uint8_t TouchUpdated ;
#endif

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
	
#ifdef TOUCH
	uint32_t rows = 3 ;

	TlExitIcon = 1 ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, rows-1) ;

  int8_t  sub    = mstate2.m_posVert ;
		
	red = colour >> 11 ;
	green = ( colour >> 6 ) & 0x1F ;
	blue = colour & 0x1F ;

//	uint8_t blink = InverseBlink ;
	uint8_t subN = 0 ;
	uint16_t hcolour = dimBackColour() ;
	uint8_t attr ;

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
	
	uint8_t y = TTOP ;
	lcd_hline( 0, TTOP, TRIGHT ) ;
	
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Red"), y, attr ? ~hcolour : hcolour ) ;
	drawNumber( TRIGHT-5, y, red, attr) ; //, attr ? ~hcolour : hcolour ) ;
  if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( red, 31 ) ;
	}
	y += TFH ;
	subN += 1 ;

	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Green"), y, attr ? ~hcolour : hcolour ) ;
	drawNumber( TRIGHT-5, y, green, attr) ; //, attr ? ~hcolour : hcolour ) ;
  if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( green, 31 ) ;
	}
	y += TFH ;
	subN += 1 ;
	attr = (sub==subN) ? INVERS : 0 ;
	drawItem( (char *)XPSTR("Blue"), y, attr ? ~hcolour : hcolour ) ;
	drawNumber( TRIGHT-5, y, blue, attr) ; //, attr ? ~hcolour : hcolour ) ;
  if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( blue, 31 ) ;
	}
	y += TFH ;
	subN += 1 ;
#else	
	static MState2 mstate2 ;
	mstate2.check_columns(event, 2 ) ;

  int8_t  sub    = mstate2.m_posVert ;
	red = colour >> 11 ;
	green = ( colour >> 6 ) & 0x1F ;
	blue = colour & 0x1F ;

	uint8_t y = 2*FH;
	uint8_t blink = InverseBlink ;
	uint8_t subN = 0 ;

	lcd_puts_Pleft( y, XPSTR("Red")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, red,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( red, 31 ) ;
	y += FH ;
	subN += 1 ;
	 
	lcd_puts_Pleft( y, XPSTR("Green")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, green,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( green, 31 ) ;
	y += FH ;
	subN += 1 ;
	
	lcd_puts_Pleft( y, XPSTR("Blue")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, blue,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( blue, 31 ) ;
//	y += FH ;
//	subN += 1 ;
#endif // TOUCH
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
#ifdef TOUCH
		dimBackColour() ;
#endif
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
		putsTime( 224, (STATUS_VERTICAL/2)+3, Time.hour*60+Time.minute, 0, 0, LCD_BLACK, LCD_STATUS_GREY ) ;

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
  	lcd_outdezNAtt( 4*FW, (STATUS_VERTICAL/2)+3, IdlePercent ,PREC2, 4, LCD_BLACK, LCD_STATUS_GREY ) ;
  	lcd_outdezNAtt( 8*FW , (STATUS_VERTICAL/2)+3, MixerRate, 0, 3, LCD_BLACK, LCD_STATUS_GREY ) ;
		if ( scriptPercent )
		{
		  lcd_outdezNAtt( 12*FW, (STATUS_VERTICAL/2)+3, BasicExecTime ,PREC1, 4, LCD_BLACK, LCD_STATUS_GREY ) ;
		}

		lcd_outdezNAtt( 16*FW, (STATUS_VERTICAL/2)+3, HiresSavedTime/200 ,PREC1, 4, LCD_BLACK, LCD_STATUS_GREY ) ;
	}
	SuppressStatusLine = 0 ;
}

