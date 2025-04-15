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

extern const MenuFuncP menuTabStat[] ;

extern void disp_datetime( coord_t y ) ;

struct t_newvario NewVario ;

#ifdef BIG_SCREEN
#define STAT2_OFF_0		0
#define MUSIC_OFF_0		0
#define BOOT_OFF_0  	0
#else
#define STAT2_OFF_0		0
#define MUSIC_OFF_0		0
#define BOOT_OFF_0  	0
#endif

extern uint16_t g_timeMain ;
extern uint16_t g_timeRfsh ;
extern uint16_t g_timeMixer ;

extern uint16_t s_timeCumTot;		// Total tx on time (secs)
extern uint32_t ChipId ;
extern uint16_t BtRxTimer ;

extern uint8_t StatusTimer ;
extern uint8_t InverseBlink ;
extern uint8_t RawLogging ;

#ifndef SMALL
#include "s6rimg.lbm"
#endif

enum EnumTabStat
{
	e_battery,
	e_stat1,
	e_stat2,
#ifdef NEW_VARIO
	e_vario,
#endif
#ifndef BIG_SCREEN
	e_music,
#endif
	e_music1,
#ifndef SMALL
//	e_dsm,
	e_traindiag,
#endif
//#if defined(LUA) || defined(BASIC)
//	e_Script,
//#endif
	e_debug,
#ifdef DEBUG_REPORT
	e_report,
#endif
#ifdef TOUCH
	e_touch,
#endif
	e_Setup3,
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX7) || defined(PCBX9LITE) || defined(PCBX9D) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
//#ifdef IMAGE_128
//	e_image,
//#endif
#ifdef BLUETOOTH
	e_bluetooth,
#endif
#endif
#ifdef MENUTESTINCLUDE
	e_test,
#endif
#ifdef PCB9XT
	e_Slave,
#endif
#ifndef SMALL
	e_s6r,
#endif
  e_Boot
} ;

void menuProcBattery(uint8_t event) ;
void menuProcStatistic(uint8_t event) ;
void menuProcStatistic2(uint8_t event) ;
void menuNewVario(uint8_t event) ;
void menuProcMusic(uint8_t event) ;
void menuProcMusicList(uint8_t event) ;
void menuProcTrainDdiag(uint8_t event) ;
void menuDebug(uint8_t event) ;
void menuProcSDstat(uint8_t event) ;
void menuProcBt(uint8_t event) ;
void menuProcS6R(uint8_t event) ;
void menuProcSlave(uint8_t event) ;
void menuProcBoot(uint8_t event) ;
void menuTouch(uint8_t event) ;
void menuTest(uint8_t event) ;

#ifdef DEBUG_REPORT
void menuReport(uint8_t event) ;
#endif


const MenuFuncP menuTabStat[] =
{
	menuProcBattery,
	menuProcStatistic,
	menuProcStatistic2,
#ifdef NEW_VARIO
	menuNewVario,
#endif
#ifndef BIG_SCREEN
	menuProcMusic,
#endif
	menuProcMusicList,
#ifndef SMALL
//	menuProcDsmDdiag,
	menuProcTrainDdiag,
#endif
//#if defined(LUA) || defined(BASIC)
//	menuScript,
//#endif
	menuDebug,
#ifdef DEBUG_REPORT
	menuReport,
#endif
#ifdef TOUCH
	menuTouch,
#endif
	menuProcSDstat,
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX7) || defined(PCBX9LITE) || defined(PCBX9D) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
//#ifdef IMAGE_128
//	menuImage,
//#endif
#ifdef BLUETOOTH
	menuProcBt,
#endif
#endif
#ifdef MENUTESTINCLUDE
	menuTest,
#endif
#ifdef PCB9XT
	menuProcSlave,
#endif
#ifndef SMALL
	menuProcS6R,
#endif
	menuProcBoot
} ;


void s6rRequest( uint8_t type, uint8_t index, uint8_t value )
{
	static uint8_t sportPacket[8] ;
	S6Rdata.valid = 0 ;
	sportPacket[0] = type ;
	sportPacket[1] = 0x30 ;
	sportPacket[2] = 0x0C ;
	sportPacket[3] = index ;
	sportPacket[4] = value ;
	sportPacket[5] = 0 ;
	sportPacket[6] = 0 ;
	
//#ifdef ACCESS
//	accessSportPacketSend( sportPacket, 0x16 ) ;
//#else
	sportPacketSend( sportPacket, 0x16 ) ;
//#endif
}
//#endif //n SMALL

#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
void menuProcStatistic(uint8_t event)
{
	MENU(PSTR(STR_STAT), menuTabStat, e_stat1, 1, {0} ) ;
  lcdDrawTextLeft( FHPY*0, XPSTR("\016TOT\037\001TME\016TSW\037\001STK\016ST%"));

#ifdef COLOUR_DISPLAY
  putsTimeP(    4*FWX*HVSCALE, FHPY*1*HVSCALE, s_timer[0].s_timeCumAbs, 0 ) ;
  putsTimeP(   10*FWX*HVSCALE, FHPY*1*HVSCALE, s_timer[0].s_timeCumSw,      0 ) ;

  putsTimeP(    4*FWX*HVSCALE, FHPY*2*HVSCALE, s_timer[0].s_timeCumThr, 0 ) ;
  putsTimeP(   10*FWX*HVSCALE, FHPY*2*HVSCALE, s_timer[0].s_timeCum16ThrP/16, 0 ) ;

  putsTimeP(   10*FWX*HVSCALE, FHPY*0*HVSCALE, s_timeCumTot, 0 ) ;
#else
  putsTimeP(    6*FWX*HVSCALE, FHPY*1*HVSCALE, s_timer[0].s_timeCumAbs, 0 ) ;
  putsTimeP(   11*FWX*HVSCALE, FHPY*1*HVSCALE, s_timer[0].s_timeCumSw,      0 ) ;

  putsTimeP(    6*FWX*HVSCALE, FHPY*2*HVSCALE, s_timer[0].s_timeCumThr, 0 ) ;
  putsTimeP(   11*FWX*HVSCALE, FHPY*2*HVSCALE, s_timer[0].s_timeCum16ThrP/16, 0 ) ;

  putsTimeP(   11*FWX*HVSCALE, FHPY*0*HVSCALE, s_timeCumTot, 0 ) ;
#endif
  uint16_t traceRd = s_traceCnt>MAXTRACE ? s_traceWr : 0;
  coord_t x=5;
#if defined(PCBX12D) || defined(PCBX10)
  coord_t y=70;
#else
  coord_t y=60;
#endif
  
#if defined(PCBX12D) || defined(PCBX10)
	pushPlotType( PLOT_BLACK ) ;
#endif
	lcd_hline(x-3,y,120+3+3);
  lcd_vline(x,y-32,32+3);

  for(uint32_t i=0; i<120; i+=6)
  {
    lcd_vline(x+i+6,y-1,3);
  }
  for(uint32_t i=1; i<=120; i++)
  {
    lcd_vline(x+i,y-s_traceBuf[traceRd],s_traceBuf[traceRd]);
    traceRd++;
    if(traceRd>=MAXTRACE) traceRd=0;
    if(traceRd==s_traceWr) break;
  }
#if defined(PCBX12D) || defined(PCBX10)
	popPlotType() ;
#endif
}

void menuProcSDstat(uint8_t event)
{
	MENU(PSTR(STR_ST_CARD_STAT), menuTabStat, e_Setup3, 1, {0} ) ;

#ifdef PCBX9D
	if ( event == EVT_ENTRY )
	{
extern void sdInit( void ) ;
		sdInit() ;
	}
#endif
	 
	uint32_t present ;
//#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
//	PUTS_ATT_LEFT( 1*FHPY, XPSTR("Present"));
//#else
	PUTS_ATT_LEFT( 1*FHPY, XPSTR("Present"));
//#endif

#ifdef PCBSKY
	present = CardIsPresent() ;
#else
extern DWORD socket_is_empty( void ) ;
	present = !socket_is_empty() ;
#endif
	
	PUTS_AT_IDX( 9*FW, 1*FHPY, XPSTR("\003No Yes"), present, 0 ) ;
	
	PUTS_ATT_LEFT( 2*FHPY, PSTR(STR_4_READY));

extern uint8_t SectorsPerCluster ;
  PUTS_NUM( MENU_DISPLAY_RIGHT, 1*FHPY, SectorsPerCluster/2, 0 ) ;
#ifndef SIMU


#ifdef PCBSKY
#ifndef SMALL
	uint32_t i ;
	coord_t x, y ;
#endif
#endif

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
extern uint8_t CardType ;
extern uint32_t sdMounted( void ) ;
	PUT_HEX4( 10*FW, 2*FHPY, CardType ) ;
	PUT_HEX4( 16*FW, 2*FHPY, sdMounted() ) ;
	PUT_HEX4( 10*FW, 3*FHPY, Card_state ) ;
#endif

#ifdef PCBSKY
	PUT_HEX4( 10*FW, 2*FHPY, Card_state ) ;
	 
#ifndef SMALL
extern uint32_t SDlastError ;
	PUT_HEX4( 16*FW, 2*FHPY, SDlastError ) ;
	 
		y = 3*FHPY ;
		x = 4*FW ;
		PUTS_ATT_LEFT( y, XPSTR("CID"));
		for ( i = 0 ; i < 4 ; i += 1 )
		{
		  PUT_HEX4( x, y, Card_ID[i] >> 16 ) ;
		  PUT_HEX4( x+4*FWX, y, Card_ID[i] ) ;
			x += 8*FW ;
			if ( i == 1 )
			{
				y += FHPY ;
				x = 4*FW ;
			}			 
		}
		y = 5*FHPY ;
		x = 4*FW ;
		PUTS_ATT_LEFT( y, XPSTR("CSD"));
		for ( i = 0 ; i < 4 ; i += 1 )
		{
		  PUT_HEX4( x, y, Card_CSD[i] >> 16 ) ;
		  PUT_HEX4( x+4*FW, y, Card_CSD[i] ) ;
			x += 8*FWX ;
			if ( i == 1 )
			{
				y += FHPY ;
				x = 4*FW ;				
			}			 
		}
		y = 7*FHPY ;
		x = 4*FW ;
		PUTS_ATT_LEFT( y, XPSTR("SCR"));
		for ( i = 0 ; i < 2 ; i += 1 )
		{
		  PUT_HEX4( x, y, Card_SCR[i] >> 16 ) ;
		  PUT_HEX4( x+4*FW, y, Card_SCR[i] ) ;
			x += 8*FW ;
		}
#endif
#endif
	if (sd_card_ready() )
	{
	}
	else
	{
		PUTS_ATT_LEFT( 2*FHY, PSTR(STR_NOT));
	}
#endif
}

void menuProcS6R(uint8_t event)
{
	static uint8_t state ;
	static uint8_t index ;
	static uint8_t lastSub ;
	static uint8_t valid ;
	uint8_t min ;
	uint8_t max ;
	static int16_t value ;
	static uint8_t calState ;
#ifdef PAGE_NAVIGATION
	static uint8_t rotaryLocal ;
#endif

	MENU(XPSTR("SxR Config."), menuTabStat, e_s6r, 28, {0} ) ;
	int8_t sub = mstate2.m_posVert ;

	if ( StatusTimer )
	{
		if ( --StatusTimer == 0 )
		{
			state = ( sub != 27 ) ? 0 : 1 ;
		}
	}
	if ( event == EVT_ENTRY )
	{
		state = 0 ;
		calState = 0 ;
		S6Rdata.valid = 0 ;
		lastSub = 0 ;
#ifdef PAGE_NAVIGATION
		rotaryLocal = 0 ;
#endif
		StatusTimer = 0 ;
	}
	if ( event == EVT_KEY_BREAK(KEY_MENU) )
	{
		if ( valid == 0 )
		{
			if ( sub != 27 )
			{
				s6rRequest( 0x30, index, 0 ) ;
				state = 1 ;			
				StatusTimer = 230 ;
			}
		}
		s_editMode = 0 ;
	}
	if ( event == EVT_KEY_LONG(KEY_MENU) )
	{
		if ( valid || sub == 27 )
		{
			s6rRequest( 0x31, index, value ) ;
	    killEvents(event) ;
			StatusTimer = 100 ;
			if ( sub == 27 )
			{
				valid = 0 ;
				StatusTimer = 50 ;
			}
			state = 2 ;
		}
		s_editMode = 0 ;
	}

	if ( state == 1 )
	{
		if ( S6Rdata.valid )
		{
			state = 0 ;
			if ( index == S6Rdata.fieldIndex )
			{
				valid = 1 ;
				value = S6Rdata.value ;
			}
		}
		PUTS_ATT_LEFT( 4*FHPY, "\016Reading" ) ;
	}
	if ( state == 2 )
	{
		PUTS_ATT_LEFT( 4*FHPY, "\016Writing" ) ;
	}
	if ( lastSub != sub )
	{
		valid = 0 ;
		lastSub = sub ;
	}
	min = 0 ;
	max = 2 ;
	switch ( sub )
	{
		case 1 :
			PUTS_ATT_LEFT( 2*FHPY, "Wing type" ) ;
			index = 0x80 ;
			if ( valid )
			{
  			PUTS_AT_IDX( 12*FW, 3*FHPY, XPSTR("\006NormalDelta VTail "), value, InverseBlink ) ;
			}
		break ;
		case 2 :
			PUTS_ATT_LEFT( 2*FHPY, "Mounting" ) ;
			index = 0x81 ;
			max = 3 ;
			if ( valid )
			{
  			PUTS_AT_IDX( 12*FW, 3*FHPY, XPSTR("\006Hor   HorRevVer   VerRev"), value, InverseBlink ) ;
			}
			lcd_img( 12, 4*FHPY, value<2 ? S6Rimg1 : S6Rimg3, value<3 ? value & 1 : 2, 0 ) ;
			PUTS_ATT_LEFT( 7*FHPY, "<-Heading" ) ;
		break ;
		case 3 :
		case 4 :
		case 5 :
		case 6 :
		case 7 :
			PUTS_ATT_LEFT( 2*FHPY, "\005Dir." ) ;
			index = 0x82 + sub - 3 ;
  		PUTS_AT_IDX( 0, 2*FHPY, XPSTR("\004Ail Ele Rud Ail2Ele2"), sub-3, 0 ) ;
			if ( sub == 6 )
			{
				index = 0x9A ;
			}
			if ( sub == 7 )
			{
				index = 0x9B ;
			}
			max = 2 ;
			if ( valid )
			{
				uint8_t t = (value+1) ;
				if ( value == 0x80 )
				{
					t = 2 ;
				}
  			PUTS_AT_IDX( 12*FW, 3*FHPY, XPSTR("\006NormalInversOff   "), t, InverseBlink ) ;
			}
		break ;
		case 8 :
		case 9 :
		case 10 :
		case 11 :
		case 12 :
		case 13 :
		case 14 :
		case 15 :
		case 16 :
			PUTS_ATT_LEFT( 2*FHPY, sub < 11 ? "\004Stab Gain" : sub < 13 ? "\004Auto Level Gain" : sub < 15 ? "\004Upright Gain" : "\004Crab Gain" ) ;
  		PUTS_AT_IDX( 0, 2*FHPY, XPSTR("\003AilEleRudAilEleEleRudAilRud"), sub-8, 0 ) ;
			index = 0x85 + sub - 8 ;
			if ( sub > 12 )
			{
				index += 2 ;
			}
			if ( sub > 15 )
			{
				index += 1 ;
			}
			max = 200 ;
			if ( valid )
			{
				PUTS_NUMX( 17*FW, 3*FHPY, value ) ;
			}
		break ;
		case 17 :
		case 18 :
		case 19 :
		case 20 :
		case 21 :
		case 22 :
			PUTS_ATT_LEFT( 2*FHPY, sub < 19 ? "\004auto angle offset" : sub < 21 ? "\004up angle offset" : "\004crab angle offset" ) ;
  		PUTS_AT_IDX( 0, 2*FHPY, XPSTR("\003AilEleEleRudAilRud"), sub-17, 0 ) ;
			index = "\x91\x92\x95\x96\x97\x99"[sub-17] ;
			min = 0x6C ;
			max = 0x94 ;
			if ( valid )
			{
				PUTS_NUMX( 17*FW, 3*FHPY, value-128 ) ;
			}
		break ;
		case 23 :
		case 24 :
		case 25 :
		case 26 :
//			lcd_puts_Pleft( 2*FHY, "Active" ) ;
  		PUTS_AT_IDX( 0, 2*FHPY, XPSTR("\006ActiveAux1  Aux2  QuickM"), sub-23, 0 ) ;
			index = "\x9C\xA8\xA9\xAA"[sub-23] ;
//			index = 0x9C ;
			max = 1 ;
			if ( valid )
			{
  			PUTS_AT_IDX( 12*FW, 3*FHPY, XPSTR("\007DisableEnable "), value, InverseBlink ) ;
			}
		break ;
		case 27 :
			PUTS_ATT_LEFT( 1*FHPY, "Calibration, place\037the S6R as shown\037press MENU LONG" ) ;
			lcd_img( 2*FW, 4*FHPY, calState<2 ? S6Rimg1 : calState<4 ? S6Rimg2 : S6Rimg3, calState & 1, 0 ) ;
			if ( calState == 1 )
			{
				PUTS_ATT_LEFT( 7*FHY, "Inverted" ) ;
			}
			index = 0x9D ;
			value = calState ;
			if ( valid )
			{
				calState += 1 ;
				calState %= 6 ;
				valid = 0 ;
			}
		break ;
	}
	if ( sub != 27 )
	{
		if ( valid && sub )
		{
			if ( state == 0 )
			{
				if ( ( (index >= 0x82) && (index <= 0x84) ) || (index == 0x9A ) || ( index == 0x9B ) )
				{
					value = value == 255 ? 0 : value == 0 ? 1 : 2 ;
				}
				if ( event != EVT_KEY_FIRST(KEY_MENU) )
				{
#ifdef PAGE_NAVIGATION
					if ( event == EVT_KEY_FIRST(BTN_RE) )
					{
						killEvents( event ) ;
						Tevent = event = 0 ;
						if ( RotaryState != ROTARY_VALUE )
						{
							rotaryLocal = ROTARY_VALUE ;
						}
						else
						{
							rotaryLocal = ROTARY_MENU_UD ;
						}
					}
					RotaryState = rotaryLocal ;
					s_editMode = ( rotaryLocal == ROTARY_VALUE ) ;
#endif	// PAGE_NAVIGATION
					value = checkIncDec16( value, min, max, NO_MENU_ONLY_EDIT ) ;
				}
				if ( ( (index >= 0x82) && (index <= 0x84) ) || (index == 0x9A ) || ( index == 0x9B ) )
				{
					value = value == 0 ? 255 : value == 1 ? 0 : 0x80 ;
				}
			}
		}
		else
		{
			PUTS_ATT_LEFT( 3*FHPY, "\014-----" ) ;
		}
	}

//  PUT_HEX4( 100, 5*FHY, S6Rdata.valid ) ;
//  PUT_HEX4( 100, 6*FHY, S6Rdata.fieldIndex ) ;
//  PUT_HEX4( 100, 7*FHY, S6Rdata.value ) ;
}

void menuProcBoot(uint8_t event)
{
  MENU(PSTR(STR_BOOT_REASON), menuTabStat, e_Boot, 1, {0/*, 0*/});

#ifdef BIG_SCREEN
	DisplayOffset = BOOT_OFF_0 ;
#endif

#if defined(PCBSKY) || defined(PCB9XT)
	PUTS_ATT_LEFT( 7*FHPY, XPSTR("Chip") ) ;
	PUT_HEX4( (5*FW)+BOOT_OFF_0, 7*FHPY, ChipId >> 16 ) ;
	PUT_HEX4( (9*FW)+BOOT_OFF_0, 7*FHPY, ChipId ) ;
#endif	

#ifdef PCBSKY
	if ( ( ResetReason & RSTC_SR_RSTTYP ) == (2 << 8) )	// Watchdog
	{
		lcdDrawTextLeft( 2*FHPY, PSTR(STR_6_WATCHDOG) ) ;
	}
	else if ( unexpectedShutdown )
	{
		PUTS_ATT_LEFT( 2*FHPY, PSTR(STR_5_UNEXPECTED) ) ;
		PUTS_ATT_LEFT( 3*FHPY, PSTR(STR_6_SHUTDOWN) ) ;
	}
	else
	{
		PUTS_ATT_LEFT( 2*FHPY, PSTR(STR_6_POWER_ON) ) ;
	}

#ifdef WDOG_REPORT
extern uint16_t WdogIntValue ;
	PUT_HEX4( 16*FW+BOOT_OFF_0, 5*FHY, WdogIntValue ) ;
#endif

#endif
 #if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
  #if defined(PCBLEM1)
	if ( ResetReason & RCC_CSR_IWDGRSTF )	// Watchdog
	{
		PUTS_ATT_LEFT( 2*FHPY, PSTR(STR_6_WATCHDOG) ) ;
  #else
	if ( ResetReason & RCC_CSR_WDGRSTF )	// Watchdog
	{
		PUTS_ATT_LEFT( 2*FHPY, PSTR(STR_6_WATCHDOG) ) ;
  #ifdef PCB9XT
extern uint32_t WatchdogPosition ;
	PUT_HEX4( BOOT_OFF_0, 3*FHPY, WatchdogPosition ) ;
  #endif
 #endif
	}
	else if ( unexpectedShutdown )
	{
		PUTS_ATT_LEFT( 2*FHPY, PSTR(STR_5_UNEXPECTED) ) ;
		PUTS_ATT_LEFT( 3*FHPY, PSTR(STR_6_SHUTDOWN) ) ;
	}
	else
	{
		PUTS_ATT_LEFT( 2*FHPY, PSTR(STR_6_POWER_ON) ) ;
	}
#endif
  PUTS_NUM( (20*FW)+BOOT_OFF_0, 6*FHPY, ( ResetReason >> 8 ) & 7, 0 ) ;
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1) || defined(PCBX7ACCESS)
	PUT_HEX4( BOOT_OFF_0, 6*FHPY, ResetReason >> 16 ) ;
	PUT_HEX4( 4*FW+BOOT_OFF_0, 6*FHPY, ResetReason ) ;
	PUT_HEX4( BOOT_OFF_0, 5*FHPY, RCC->BDCR ) ;
	PUT_HEX4( 4*FW+BOOT_OFF_0, 5*FHPY, RCC->CSR ) ;

#ifdef WDOG_REPORT
extern uint16_t WdogIntValue ;
	PUT_HEX4( 16*FW+BOOT_OFF_0, 5*FHPY, WdogIntValue ) ;
#endif

#endif

#ifdef WDOG_REPORT
#if defined(PCBX12D) || defined(PCBX10)
extern uint16_t WdogIntValue ;
 	PUTC( 15*FW+BOOT_OFF_0, 5*FHPY, '>') ;
	PUT_HEX4( 16*FW+BOOT_OFF_0, 5*FHPY, WdogIntValue ) ;
#endif
#endif

#ifdef STACK_PROBES
extern uint32_t stackSpace( uint32_t stack ) ;

	PUT_HEX4( 0+BOOT_OFF_0, 4*FHPY, stackSpace(0) ) ;
	PUT_HEX4( 5*FW+BOOT_OFF_0, 4*FHPY, stackSpace(1) ) ;
	PUT_HEX4( 10*FW+BOOT_OFF_0, 4*FHPY, stackSpace(2) ) ;
 #ifndef PCBX12D
  #ifndef PCBX10
	PUT_HEX4( 15*FW+BOOT_OFF_0, 4*FHPY, stackSpace(3) ) ;
  #endif
 #endif
 #ifdef MIXER_TASK
	PUT_HEX4( 15*FW+BOOT_OFF_0, 5*FHPY, stackSpace(4) ) ;
 #endif
 #ifndef SMALL
	PUT_HEX4( 15*FW+BOOT_OFF_0, 5*FHPY, stackSpace(5) ) ;
extern uint32_t StackAtOsStart ;
	PUT_HEX4( 15*FW+BOOT_OFF_0, 7*FHPY, StackAtOsStart >> 16 ) ;
	PUT_HEX4( 15*FW+BOOT_OFF_0+4*FW, 7*FHPY, StackAtOsStart ) ;
 #endif
#endif

}
#endif


#ifdef PROP_TEXT

void menuProcStatistic2(uint8_t event)
{
	MENU(PSTR(STR_STAT2), menuTabStat, e_stat2, 1, {0} ) ;

  switch(event)
  {
    case EVT_KEY_FIRST(KEY_MENU):
      g_timeMain = 0;
      audioDefevent(AU_MENUS) ;
    break;
    case EVT_KEY_LONG(KEY_MENU):
			g_eeGeneral.totalElapsedTime = 0 ;
    break;
  }

#ifdef BIG_SCREEN
	DisplayOffset = STAT2_OFF_0 ;
#endif

  PUTS_ATT_LEFT( 1*FHPY, XPSTR("On Time")) ;
	div_t qr ;
	qr = div( g_eeGeneral.totalElapsedTime, 60 ) ;
	putsTimeP( 9*FWX+STAT2_OFF_0, FHPY*1, qr.quot, 0 ) ;
	PUTC_ATT( LcdLastRightPos, 1*FHPY, ':', 0 ) ;
	lcd2Digits( LcdNextPos, 1*FHPY, qr.rem, LEFT ) ;

  PUTS_ATT_LEFT( 2*FHPY, XPSTR("tmain\017ms"));
  PUTS_NUM(14*FW, 2*FHPY, (g_timeMain)/20 ,PREC2) ;

extern uint32_t MixerRate ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	PUTS_ATT_LEFT( 4*FHPY, XPSTR("Mixer Rate"));
#else
  PUTS_ATT_LEFT( 4*FHPY, XPSTR("tmixer\017ms"));
  PUTS_NUM(14*FW , 4*FHPY, (g_timeMixer)/20 ,PREC2);
#endif
  PUTS_NUM(20*FW , 4*FHPY, MixerRate, 0 ) ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
  PUTS_ATT_LEFT( 3*FHPY, XPSTR("trefresh\017ms"));
  PUTS_NUM(14*FW , 3*FHPY, (g_timeRfsh)/20 ,PREC2);
#endif

extern uint8_t AudioVoiceCountUnderruns ;
	PUTS_ATT_LEFT( 5*FHPY, XPSTR("Voice underruns"));
  PUTS_NUM( 20*FW, 5*FHPY, AudioVoiceCountUnderruns, 0 ) ;

  PUTS_P( 3*FWX,  6*FHPY, PSTR(STR_MENU_REFRESH));

extern uint32_t IdlePercent ;
  PUTS_ATT_LEFT( 7*FHPY, XPSTR("Idle time\016%"));
  PUTS_NUM( 14*FW-1, 7*FHPY, IdlePercent ,PREC2);

}


//void menuProcStatistic(uint8_t event)
//{
//	MENU(PSTR(STR_STAT), menuTabStat, e_stat1, 1, {0} ) ;
//  lcdDrawTextLeft( FHY*0, XPSTR("\016TOT\037\001TME\016TSW\037\001STK\016ST%"));

//  putsTimeP(    6*FWX, FHY*1, s_timer[0].s_timeCumAbs, 0 ) ;
//  putsTimeP(   11*FWX, FHY*1, s_timer[0].s_timeCumSw,      0 ) ;

//  putsTimeP(    6*FWX, FHY*2, s_timer[0].s_timeCumThr, 0 ) ;
//  putsTimeP(   11*FWX, FHY*2, s_timer[0].s_timeCum16ThrP/16, 0 ) ;

//  putsTimeP(   11*FWX, FHY*0, s_timeCumTot, 0 ) ;

//  uint16_t traceRd = s_traceCnt>MAXTRACE ? s_traceWr : 0;
//  uint8_t x=5;
//  uint8_t y=60;
  
//#if defined(PCBX12D) || defined(PCBX10)
//	pushPlotType( PLOT_BLACK ) ;
//#endif
//	lcd_hline(x-3,y,120+3+3);
//  lcd_vline(x,y-32,32+3);

//  for(uint8_t i=0; i<120; i+=6)
//  {
//    lcd_vline(x+i+6,y-1,3);
//  }
//  for(uint8_t i=1; i<=120; i++)
//  {
//    lcd_vline(x+i,y-s_traceBuf[traceRd],s_traceBuf[traceRd]);
//    traceRd++;
//    if(traceRd>=MAXTRACE) traceRd=0;
//    if(traceRd==s_traceWr) break;
//  }
//#if defined(PCBX12D) || defined(PCBX10)
//	popPlotType() ;
//#endif
//}

void menuProcBattery(uint8_t event)
{
	MENU(PSTR(STR_BATTERY), menuTabStat, e_battery, 1, {0} ) ;

  switch(event)
  {
  	case EVT_KEY_BREAK(KEY_MENU):
      g_timeMain = 0;
#ifdef PCBSKY
			Current_max = 0 ;
#endif
    break;
    case EVT_KEY_LONG(KEY_MENU):
#ifdef PCBSKY
			MAh_used = 0 ;
			Current_used = 0 ;
#endif
      audioDefevent(AU_MENUS) ;
    	killEvents(event) ;
    break;
  	
//		case EVT_ENTRY :
//extern void convertFont() ;
//		 convertFont() ;
//    break;

  }

		lcdDrawTextLeft( 2*FHPY, PSTR(STR_Battery));
  	putsVBat( 13*FW , 2*FHPY, 0 ) ;
//		putsVolts( 13*FW, 2*FH, g_vbat100mV, 0 ) ;

#ifdef PCBSKY
#ifndef REVA
		if ( ( g_eeGeneral.ar9xBoard == 0 ) && ( g_eeGeneral.extraPotsSource[0] != 2 ) && ( g_eeGeneral.extraPotsSource[1] != 2 ) )
		{
			Current_sum += Current_current ;
			if ( ++Current_count > 49 )
			{
				Current = Current_sum / 500 ;
				Current_sum = 0 ;
				Current_count = 0 ;
			}
			lcdDrawTextLeft( 3*FHPY, PSTR(STR_CURRENT_MAX));
	  	lcdDrawNumber( 13*FW, 3*FHPY, Current, 0 ) ;
	  	lcdDrawNumber( 20*FW, 3*FHPY, Current_max/10, 0 ) ;
			lcdDrawTextLeft( 4*FHPY, XPSTR("mAh\017[MENU]"));
	  	lcdDrawNumber( 12*FW, 4*FHPY, MAh_used + Current_used/3600 ,PREC1 ) ;
		}
		lcdDrawTextLeft( 6*FHPY, PSTR(STR_CPU_TEMP_MAX));
	  lcdDrawNumber( 12*FW-2, 6*FHPY, (((((int32_t)Temperature - 838 ) * 621 ) >> 11 ) - 20) ,0 ) ;
	  lcdDrawNumber( 20*FW-2, 6*FHPY, (((((int32_t)Max_temperature - 838 ) * 621 ) >> 11 ) - 20) ,0 ) ;
#endif // REVA

		disp_datetime( 5*FHPY ) ;
#endif // PCBSKY

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	disp_datetime( 5*FH ) ;
#endif

#ifndef SMALL
extern uint32_t Master_frequency ;
 	lcdDrawNumber( 5*FW, 7*FHPY, Master_frequency/1000000, 0 ) ;
#endif
}


void menuNewVario(uint8_t event)
{
	EditType = 0 ;
	struct t_newvario *p = &NewVario ;

	MENU(XPSTR("Vario"), menuTabStat, e_vario, 7, {0} ) ;

//	if ( event == EVT_ENTRY )
//	{
//		struct t_vario *p = &NewVario ;
//		if ( p->defaulted == 0 )
//		{
			
//		}
		
//	}
	 
  PUTS_ATT_LEFT( 1*FHPY, PSTR(STR_VARIO_SRC) ) ;
	p->varioSource = checkIndexed( 1*FHPY, XPSTR(FWx17"\001""\004vspd  A2"), p->varioSource, (mstate2.m_posVert == 1) ) ;

  PUTS_ATT_LEFT( 2*FHPY, XPSTR("Centre Min.") ) ;
  PUTS_NUM( PARAM_OFS, 2*FHPY, -5 + p->varioCenterMin, ((mstate2.m_posVert == 2) ? INVERS : 0)|PREC1) ;
	if (mstate2.m_posVert == 2)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioCenterMin, -16, 5+min(10, p->varioCenterMax+5)) ;
	}

  PUTS_ATT_LEFT( 3*FHPY, XPSTR("Centre Max.") ) ;
  PUTS_NUM( PARAM_OFS, 3*FHPY, 5 + p->varioCenterMax, ((mstate2.m_posVert == 3) ? INVERS : 0)|PREC1) ;
	if (mstate2.m_posVert == 3)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioCenterMax, 5+max(-10, p->varioCenterMin-5), 15) ;
	}

  PUTS_ATT_LEFT( 4*FHPY, XPSTR("Vario Min.") ) ;
  PUTS_NUM( PARAM_OFS, 4*FHPY, p->varioMin-10, ((mstate2.m_posVert == 4) ? INVERS : 0)) ;
	if (mstate2.m_posVert == 4)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioMin, -7, 7 ) ;
	}

  PUTS_ATT_LEFT( 5*FHPY, XPSTR("Vario Max.") ) ;
  PUTS_NUM( PARAM_OFS, 5*FHPY, p->varioMax+10, ((mstate2.m_posVert == 5) ? INVERS : 0)) ;
	if (mstate2.m_posVert == 5)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioMax, -7, 7 ) ;
	}

	PUTS_ATT_LEFT( 6*FHPY, PSTR(STR_2SWITCH) ) ;
	p->swtch = edit_dr_switch( PARAM_OFS, 6*FHPY, p->swtch, ((mstate2.m_posVert == 6) ? INVERS : 0)|LUA_RIGHT, (mstate2.m_posVert == 6) ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
}

void menuProcMusic(uint8_t event)
{
	EditType = EE_GENERAL ;
	uint32_t rows = 2 ;
	if ( MusicPlaying == MUSIC_STOPPED )
	{
		rows = 5 ;
	}
	if ( ( MusicPlaying == MUSIC_PLAYING ) || ( MusicPlaying == MUSIC_PAUSED ) )
	{
		rows = 6 ;
	}
	MENU( PSTR(STR_Music), menuTabStat, e_music, rows, {0} ) ;

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

  if ( ( event == EVT_KEY_FIRST(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
	{
		if ( mstate2.m_posVert != 1 )
		{
			killEvents(event) ;
			s_editMode = 0 ;
			event = 0 ;
		}
		switch ( mstate2.m_posVert )
		{
			case 2 :
				VoiceFileType = VOICE_FILE_TYPE_MUSIC ;
      	pushMenu( menuProcSelectVoiceFile ) ;
			break ;
			case 4 :
				if ( MusicPlaying == MUSIC_STOPPED )
				{
					MusicPlaying = MUSIC_STARTING ;
				}
				else
				{
					MusicPlaying = MUSIC_STOPPING ;
				}
			break ;

			case 5 :
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
			if ( mstate2.m_posVert == 5 )
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

#ifdef BIG_SCREEN
	DisplayOffset = MUSIC_OFF_0 ;
#endif

  PUTS_ATT_LEFT( 1*FHPY, XPSTR("Type")) ;
	
	uint32_t b = g_eeGeneral.musicType ;
	g_eeGeneral.musicType = checkIndexed( 1*FHPY, XPSTR(FWx15"\001""\004NameList"), b, (mstate2.m_posVert == 1) ) ;
	if ( g_eeGeneral.musicType != b )
	{
		g_eeGeneral.musicVoiceFileName[0] = '\0' ;
	}

  PUTS_ATT_LEFT( 2*FHPY, XPSTR("File")) ;
	PUTS_ATT( 6*FW+MUSIC_OFF_0, 2*FHPY, (char *)g_eeGeneral.musicVoiceFileName, 0 ) ;
  
	g_eeGeneral.musicLoop = onoffMenuItem( g_eeGeneral.musicLoop, 3*FHPY, XPSTR("Loop"), mstate2.m_posVert == 3 ) ;
	
	if ( rows == 5 )
	{
  	lcdDrawTextLeft( 4*FHPY, XPSTR("Start")) ;
		if ( mstate2.m_posVert == 5 )
		{
			mstate2.m_posVert = 4 ;
		}
	}
	else
	{
		PUTS_ATT_LEFT( 4*FHPY, XPSTR("Stop")) ;
		PUTS_ATT_LEFT( 5*FHPY, MusicPlaying == MUSIC_PAUSED ? XPSTR("\007Resume") : XPSTR("\007Pause")) ;
		if ( g_eeGeneral.musicType )
		{
			if ( MusicPlaying == MUSIC_PLAYING )
			{
				PUTS_ATT_LEFT( 5*FHPY, XPSTR("Prev<\017>Next")) ;
			}
		}
		PUTS_ATT_LEFT( 6*FHPY, CurrentPlayName ) ;
	}

	if ( mstate2.m_posVert == 2 )
	{
		lcd_rect( 6*FW-1+MUSIC_OFF_0, 2*FHPY-1, MUSIC_NAME_LENGTH*FW+2, 9 ) ;
	}

	if ( mstate2.m_posVert == 4 )
	{
		lcd_char_inverse( 0+MUSIC_OFF_0, 4*FHPY, 30, 0 ) ;
	}
	else if ( mstate2.m_posVert == 5 )
	{
		lcd_char_inverse( 7*FW+MUSIC_OFF_0, 5*FHPY, 36, 0 ) ;
	}

extern uint32_t BgSizePlayed ;
extern uint32_t BgTotalSize ;

		lcd_hbar( 10+MUSIC_OFF_0, 57, 101, 6, (BgTotalSize - BgSizePlayed) * 100 / BgTotalSize ) ;
	
}

void menuProcMusicList(uint8_t event)
{
	uint32_t i ;
	uint32_t j ;
	if ( PlayListCount < 7 )
	{
		i = 1 ;
		j = PlayListCount ;
	}
	else
	{
		i = PlayListCount - 6 ;
		j = 7 ;
	}
	MENU( XPSTR("Music List"), menuTabStat, e_music1, i, {0} ) ;
	int8_t  sub    = mstate2.m_posVert;
	
	for ( i = 0 ; i < j ; i += 1 )
	{
		PUTS_P( 8*FW, (i+1)*FHPY, PlayListNames[i+sub] ) ;
	}

}


void menuProcTrainDdiag(uint8_t event)
{
	MENU(XPSTR("Train diag"), menuTabStat, e_traindiag, 3, {0} ) ;
	
	int8_t sub = mstate2.m_posVert ;

  PUTS_ATT_LEFT( 1*FHPY, XPSTR("Trainer Mode") ) ;
	PUTS_AT_IDX(16*FW, FHPY, XPSTR("\004NormSer Com1"), TrainerMode, (sub == 1) ? INVERS : 0 ) ;
#ifndef SMALL
  PUTS_ATT_LEFT( 2*FHPY, XPSTR("Trainer In Pol") ) ;
	PUTS_AT_IDX(16*FW, 2*FHPY, XPSTR("\003NegPos"), TrainerPolarity, (sub == 2) ? INVERS : 0 ) ;
#endif

#ifndef SMALL
//uint32_t size = (uint32_t)&_estack - (uint32_t)&_ebss ;

//	PUT_HEX4( 0, 4*FH, size >> 16 ) ;
//	PUT_HEX4( 24, 4*FH, size ) ;
#endif


#if defined(PCBLEM1)


#endif

	if(sub==1)
  {
		uint8_t b = TrainerMode ;
//		TrainerMode = checkIncDec16( TrainerMode, 0, 2, 0 ) ;
#ifndef REVX
		if ( g_model.telemetryRxInvert )
		{
			TrainerMode = 0 ;	// software serial in use
		}
#endif	// nREVX
		if ( TrainerMode != b )
		{
			if ( TrainerMode == 2 )
			{
#ifdef PCBSKY
				setCaptureMode( 0 ) ;			
#else
				init_trainer_capture( 0 ) ;
#endif
				init_software_com1( 9600, 0, 0 ) ;
			}
			else
			{
#ifdef PCBSKY
				configure_pins( (PIO_PA5 | PIO_PA6), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;
				setCaptureMode( TrainerMode ) ;			
#else
				init_trainer_capture( TrainerMode ) ;
#endif
			}
		}
  }
#ifndef SMALL
	else if(sub==2)
  {
		uint8_t b = TrainerPolarity ;
		TrainerPolarity = checkIncDec16( TrainerPolarity, 0, 1, 0 ) ;
		if ( TrainerPolarity != b )
		{
			if ( TrainerMode == 1 )
			{
#ifdef PCBSKY
				setCaptureMode( TrainerMode ) ;			
#else
				init_trainer_capture( TrainerMode ) ;
#endif
			}
		}
	}
#endif
}


#ifdef BLUETOOTH


void menuProcBt(uint8_t event)
{
  MENU(XPSTR("BT"), menuTabStat, e_bluetooth, 7, {0/*, 0*/});

#ifdef PCBSKY

extern uint16_t BtLastSbusSendTime ;

	PUT_HEX4(50, 0, BtLastSbusSendTime ) ;

 #ifdef BT_PDC
	uint16_t *p = (uint16_t *)&BtPdcFifo ;
 #else
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
 #endif
#endif
#ifdef PCB9XT
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
#endif
#if defined(PCBX7) || defined(PCBX9D) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
#endif
	uint32_t i ;
	uint16_t x ;
#ifndef SMALL
	p += mstate2.m_posVert * 5 ;
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 1*FHPY, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 2*FHPY, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 3*FHPY, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 4*FHPY, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 5*FHPY, x ) ;
	}
#else
	uint32_t j ;
	p += mstate2.m_posVert * 5 ;
	for ( j = 0 ; j < 5 ; j += 1 )
	{
		for ( i = 0 ; i < 5 ; i += 1 )
		{
			x = *p++ ;
			x = ( x >> 8 ) | ( x << 8 ) ;
			PUT_HEX4( i*25, (j+1)*FHPY, x ) ;
		}
	}
#endif	 
#ifndef PCB9XT
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 6*FHPY, x ) ;
	}
#endif

	lcdDrawNumber( 25, 7*FHPY, BtControl.BtBadChecksum, 0 ) ;

#ifdef BLUETOOTH
	lcdDrawNumber( 75, 7*FHPY, BtRxTimer, 0 ) ;
	lcdDrawNumber( 100, 7*FHPY, BtControl.BtCurrentLinkIndex, 0 ) ;
	lcdDrawNumber( 125, 7*FHPY, BtControl.BtBaudChangeIndex, 0 ) ;
#endif

#if defined(PCBX10) && defined(PCBREV_EXPRESS)

	PUT_HEX4( 25*FW, 1*FHPY, USART6->CR1 ) ;
	PUT_HEX4( 25*FW, 2*FHPY, USART6->SR ) ;
	PUT_HEX4( 25*FW, 3*FHPY, USART6->BRR ) ;
	PUT_HEX4( 25*FW, 4*FHPY, GPIOG->IDR ) ;


#endif
}
#endif

#ifdef PCB9XT
void menuProcSlave(uint8_t event)
{
  MENU(XPSTR("Slave"), menuTabStat, e_Slave, 1, {0/*, 0*/});

  if ( event == EVT_KEY_LONG(KEY_MENU) )
	{
		updateSlave() ;
		initM64() ;
	}

	lcdDrawTextLeft( 2*FHPY, XPSTR("Revision") ) ;
  lcdDrawNumber( 14*FW, 2*FHPY, M64Revision, PREC1 ) ;
	lcdDrawTextLeft( 3*FHPY, XPSTR("Available") ) ;
	int32_t x = newSlaveRevision() ;
	lcdDrawNumber( 14*FW, 3*FHPY, x, PREC1 ) ;
	lcdDrawTextLeft( 5*FHPY, XPSTR("Update  (MENU LONG)") ) ;
	lcdDrawTextLeft( 7*FHPY, XPSTR("Reset Count") ) ;

extern uint16_t RemBitMeasure ;
extern uint16_t M64UsedBackup ;
  lcdDrawNumber( 14*FW, 6*FHPY, M64UsedBackup, 0 ) ;
  lcdDrawNumber( 20*FW, 6*FHPY, RemBitMeasure, 0 ) ;

  lcdDrawNumber( 14*FW, 7*FHPY, M64ResetCount, 0 ) ;
  lcdDrawNumber( 20*FW, 7*FHPY, M64RestartCount, 0 ) ;

}
#endif // PCB9XT



#else	// PROP_TEXT

#if defined(PCBX12D) || defined(PCBX10)
void menuProcStatistic2(uint8_t event)
{
	MENU(PSTR(STR_STAT2), menuTabStat, e_stat2, 1, {0} ) ;

  switch(event)
  {
    case EVT_KEY_FIRST(KEY_MENU):
      g_timeMain = 0;
      audioDefevent(AU_MENUS) ;
    break;
    case EVT_KEY_LONG(KEY_MENU):
			g_eeGeneral.totalElapsedTime = 0 ;
    break;
  }

//#ifdef BIG_SCREEN
//	DisplayOffset = STAT2_OFF_0 ;
//#endif

  PUTS_ATT_LEFT( 1*FHPY, XPSTR("On Time")) ;
	div_t qr ;
	qr = div( g_eeGeneral.totalElapsedTime, 60 ) ;
	putsTimeP( 10*FWX+STAT2_OFF_0, 1*FHPY*HVSCALE, qr.quot, 0 ) ;
	PUTC_ATT( LcdLastRightPos/2, 1*FHPY, ':', 0 ) ;
	lcd2Digits( LcdNextPos, 1*FHPY*HVSCALE, qr.rem, LEFT ) ;

  PUTS_ATT_LEFT( 2*FHPY, XPSTR("tmain\017ms"));
  PUTS_NUM(14*FW, 2*FHPY, (g_timeMain)/20 ,PREC2) ;

extern uint32_t MixerRate ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	PUTS_ATT_LEFT( 4*FHPY, XPSTR("Mixer Rate"));
#else
  PUTS_ATT_LEFT( 4*FHPY, XPSTR("tmixer\017ms"));
  PUTS_NUM(14*FW , 4*FHPY, (g_timeMixer)/20 ,PREC2);
#endif
  PUTS_NUM(20*FW , 4*FHPY, MixerRate, 0 ) ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
  PUTS_ATT_LEFT( 3*FHPY, XPSTR("trefresh\017ms"));
  PUTS_NUM(14*FW , 3*FHPY, (g_timeRfsh)/20 ,PREC2);
#endif

extern uint8_t AudioVoiceCountUnderruns ;
	PUTS_ATT_LEFT( 5*FHPY, XPSTR("Voice underruns"));
  PUTS_NUM( 20*FW, 5*FHPY, AudioVoiceCountUnderruns, 0 ) ;

  PUTS_P( 3*FW,  6*FHPY, PSTR(STR_MENU_REFRESH));

extern uint32_t IdlePercent ;
  PUTS_ATT_LEFT( 7*FHPY, XPSTR("Idle time\016%"));
  PUTS_NUM( 14*FW-1, 7*FHPY, IdlePercent ,PREC2);

//#if defined(PCBX12D) || defined(PCBX10)
//  lcdDrawTextLeft( 8*FHY, XPSTR("tmain\017ms"));
//  lcdDrawText( 15*FW, 9*FHY, XPSTR("ms"));
//  lcdDrawNumber( 14*FW, 10*FHY, (g_timeMain)/20 ,PREC2) ;
//#endif
}

void menuProcBattery(uint8_t event)
{
	MENU(PSTR(STR_BATTERY), menuTabStat, e_battery, 1, {0} ) ;

  switch(event)
  {
  	case EVT_KEY_BREAK(KEY_MENU):
      g_timeMain = 0;
#ifdef PCBSKY
			Current_max = 0 ;
#endif
    break;
    case EVT_KEY_LONG(KEY_MENU):
#ifdef PCBSKY
			MAh_used = 0 ;
			Current_used = 0 ;
#endif
      audioDefevent(AU_MENUS) ;
    	killEvents(event) ;
    break;
  	
//		case EVT_ENTRY :
//extern void convertFont() ;
//		 convertFont() ;
//    break;

  }

		PUTS_ATT_LEFT( 2*FHPY, PSTR(STR_Battery));
  	putsVBat( 13*FW , 2*FHPY, 0 ) ;
//		putsVolts( 13*FW, 2*FH, g_vbat100mV, 0 ) ;

#ifdef PCBSKY
#ifndef REVA
		if ( ( g_eeGeneral.ar9xBoard == 0 ) && ( g_eeGeneral.extraPotsSource[0] != 2 ) && ( g_eeGeneral.extraPotsSource[1] != 2 ) )
		{
			Current_sum += Current_current ;
			if ( ++Current_count > 49 )
			{
				Current = Current_sum / 500 ;
				Current_sum = 0 ;
				Current_count = 0 ;
			}
			lcdDrawTextLeft( 3*FHPY, PSTR(STR_CURRENT_MAX));
	  	lcdDrawNumber( 13*FW, 3*FHPY, Current, 0 ) ;
	  	lcdDrawNumber( 20*FW, 3*FHPY, Current_max/10, 0 ) ;
			lcdDrawTextLeft( 4*FHY, XPSTR("mAh\017[MENU]"));
	  	lcdDrawNumber( 12*FW, 4*FHPY, MAh_used + Current_used/3600 ,PREC1 ) ;
		}
		lcdDrawTextLeft( 6*FHPY, PSTR(STR_CPU_TEMP_MAX));
	  lcdDrawNumber( 12*FW-2, 6*FHPY, (((((int32_t)Temperature - 838 ) * 621 ) >> 11 ) - 20) ,0 ) ;
	  lcdDrawNumber( 20*FW-2, 6*FHPY, (((((int32_t)Max_temperature - 838 ) * 621 ) >> 11 ) - 20) ,0 ) ;
#endif // REVA

		disp_datetime( 5*FHPY ) ;
#endif // PCBSKY

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	disp_datetime( 5*FHPY ) ;
#endif

#ifndef SMALL
extern uint32_t Master_frequency ;
 	PUTS_NUM( 5*FW, 7*FHPY, Master_frequency/1000000, 0 ) ;
#endif
}

void menuNewVario(uint8_t event)
{
	EditType = 0 ;
	struct t_newvario *p = &NewVario ;

	MENU(XPSTR("Vario"), menuTabStat, e_vario, 7, {0} ) ;

//	if ( event == EVT_ENTRY )
//	{
//		struct t_vario *p = &NewVario ;
//		if ( p->defaulted == 0 )
//		{
			
//		}
		
//	}
	 
  PUTS_ATT_LEFT( 1*FHPY, PSTR(STR_VARIO_SRC) ) ;
	p->varioSource = checkIndexed( 1*FHPY, XPSTR(FWx17"\001""\004vspd  A2"), p->varioSource, (mstate2.m_posVert == 1) ) ;

  PUTS_ATT_LEFT( 2*FHPY, XPSTR("Centre Min.") ) ;
  PUTS_NUM( PARAM_OFS, 2*FHPY, -5 + p->varioCenterMin, ((mstate2.m_posVert == 2) ? INVERS : 0)|PREC1) ;
	if (mstate2.m_posVert == 2)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioCenterMin, -16, 5+min(10, p->varioCenterMax+5)) ;
	}

  PUTS_ATT_LEFT( 3*FHPY, XPSTR("Centre Max.") ) ;
  PUTS_NUM( PARAM_OFS, 3*FHPY, 5 + p->varioCenterMax, ((mstate2.m_posVert == 3) ? INVERS : 0)|PREC1) ;
	if (mstate2.m_posVert == 3)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioCenterMax, 5+max(-10, p->varioCenterMin-5), 15) ;
	}

  PUTS_ATT_LEFT( 4*FHPY, XPSTR("Vario Min.") ) ;
  PUTS_NUM( PARAM_OFS, 4*FHPY, p->varioMin-10, ((mstate2.m_posVert == 4) ? INVERS : 0)) ;
	if (mstate2.m_posVert == 4)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioMin, -7, 7 ) ;
	}

  PUTS_ATT_LEFT( 5*FHPY, XPSTR("Vario Max.") ) ;
  PUTS_NUM( PARAM_OFS, 5*FHPY, p->varioMax+10, ((mstate2.m_posVert == 5) ? INVERS : 0)) ;
	if (mstate2.m_posVert == 5)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioMax, -7, 7 ) ;
	}

	PUTS_ATT_LEFT( 6*FHPY, PSTR(STR_2SWITCH) ) ;
	p->swtch = edit_dr_switch( PARAM_OFS, 6*FHPY, p->swtch, ((mstate2.m_posVert == 6) ? INVERS : 0)|LUA_RIGHT, (mstate2.m_posVert == 6) ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
}

void menuProcMusicList(uint8_t event)
{
	uint32_t i ;
	uint32_t j ;
	if ( PlayListCount < 7 )
	{
		i = 1 ;
		j = PlayListCount ;
	}
	else
	{
		i = PlayListCount - 6 ;
		j = 7 ;
	}
	MENU( XPSTR("Music List"), menuTabStat, e_music1, i, {0} ) ;
	int8_t  sub    = mstate2.m_posVert;
	
	for ( i = 0 ; i < j ; i += 1 )
	{
		PUTS_P( 0+MUSIC_OFF_0, (i+1)*FHPY, PlayListNames[i+sub] ) ;
	}

}

void menuProcTrainDdiag(uint8_t event)
{
	MENU(XPSTR("Train diag"), menuTabStat, e_traindiag, 3, {0} ) ;
	
	int8_t sub = mstate2.m_posVert ;

  PUTS_ATT_LEFT( 1*FHPY, XPSTR("Trainer Mode") ) ;
	PUTS_AT_IDX(16*FW, FHPY, XPSTR("\004NormSer Com1"), TrainerMode, (sub == 1) ? INVERS : 0 ) ;
#ifndef SMALL
  PUTS_ATT_LEFT( 2*FHPY, XPSTR("Trainer In Pol") ) ;
	PUTS_AT_IDX(16*FW, 2*FHPY, XPSTR("\003NegPos"), TrainerPolarity, (sub == 2) ? INVERS : 0 ) ;
#endif

#ifndef SMALL
//uint32_t size = (uint32_t)&_estack - (uint32_t)&_ebss ;

//	PUT_HEX4( 0, 4*FH, size >> 16 ) ;
//	PUT_HEX4( 24, 4*FH, size ) ;
#endif


#if defined(PCBLEM1)


#endif

	if(sub==1)
  {
		uint8_t b = TrainerMode ;
//		TrainerMode = checkIncDec16( TrainerMode, 0, 2, 0 ) ;
#ifndef REVX
		if ( g_model.telemetryRxInvert )
		{
			TrainerMode = 0 ;	// software serial in use
		}
#endif	// nREVX
		if ( TrainerMode != b )
		{
			if ( TrainerMode == 2 )
			{
#ifdef PCBSKY
				setCaptureMode( 0 ) ;			
#else
				init_trainer_capture( 0 ) ;
#endif
				init_software_com1( 9600, 0, 0 ) ;
			}
			else
			{
#ifdef PCBSKY
				configure_pins( (PIO_PA5 | PIO_PA6), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;
				setCaptureMode( TrainerMode ) ;			
#else
				init_trainer_capture( TrainerMode ) ;
#endif
			}
		}
  }
#ifndef SMALL
	else if(sub==2)
  {
		uint8_t b = TrainerPolarity ;
		TrainerPolarity = checkIncDec16( TrainerPolarity, 0, 1, 0 ) ;
		if ( TrainerPolarity != b )
		{
			if ( TrainerMode == 1 )
			{
#ifdef PCBSKY
				setCaptureMode( TrainerMode ) ;			
#else
				init_trainer_capture( TrainerMode ) ;
#endif
			}
		}
	}
#endif
}


 #else
void menuProcStatistic2(uint8_t event)
{
	MENU(PSTR(STR_STAT2), menuTabStat, e_stat2, 1, {0} ) ;

  switch(event)
  {
    case EVT_KEY_FIRST(KEY_MENU):
      g_timeMain = 0;
      audioDefevent(AU_MENUS) ;
    break;
    case EVT_KEY_LONG(KEY_MENU):
			g_eeGeneral.totalElapsedTime = 0 ;
    break;
  }

#ifdef BIG_SCREEN
	DisplayOffset = STAT2_OFF_0 ;
#endif

  lcd_puts_Pleft( 1*FHPY, XPSTR("On Time")) ;
  lcd_putcAtt( 11*FW+3+STAT2_OFF_0, 1*FHPY, ':', 0 ) ;
	div_t qr ;
	qr = div( g_eeGeneral.totalElapsedTime, 60 ) ;
  putsTime( 9*FW+STAT2_OFF_0, FHPY*1, qr.quot, 0, 0 ) ;
  lcd_outdezNAtt( 14*FW+STAT2_OFF_0, 1*FHPY, qr.rem, LEADING0, 2 ) ;

  lcd_puts_Pleft( 2*FHPY, XPSTR("tmain          ms"));
  lcd_outdezAtt(14*FW+STAT2_OFF_0 , 2*FHPY, (g_timeMain)/20 ,PREC2);

#if defined(PCBLEM1)
	lcd_puts_Pleft( 3*FHPY, XPSTR("Refresh time(uS)"));
extern uint16_t RefreshTime ;
  lcd_outdezAtt(20*FW+STAT2_OFF_0 , 3*FH, RefreshTime, 0 ) ;
#endif

extern uint32_t MixerRate ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	lcd_puts_Pleft( 4*FHPY, XPSTR("Mixer Rate"));
#else
  lcd_puts_Pleft( 4*FHPY, XPSTR("tmixer         ms"));
  lcd_outdezAtt(14*FW+STAT2_OFF_0 , 4*FHPY, (g_timeMixer)/20 ,PREC2);
#endif
  lcd_outdezAtt(20*FW+STAT2_OFF_0 , 4*FHPY, MixerRate, 0 ) ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
  lcd_puts_Pleft( 3*FHPY, XPSTR("trefresh       ms"));
  lcd_outdezAtt(14*FW+STAT2_OFF_0 , 3*FHPY, (g_timeRfsh)/20 ,PREC2);

//	lcd_puts_Pleft( 1*FH, XPSTR("ttimer1        us"));
//  lcd_outdezAtt(14*FW , 1*FH, (g_timePXX)/2 ,0);
#endif

//#ifdef PCBX12D
//static uint16_t tt ;
//static uint16_t ct ;
//static uint16_t mt ;
//static uint16_t count ;
//extern uint16_t TestTime ;
//extern uint16_t ClearTime ;
//extern uint16_t MenuTime ;
//if ( ++count > 25 )
//{
//	tt = TestTime ;
//	ct = ClearTime ;
//	mt = MenuTime ;
//}
//	PUT_HEX4( 134, 2*FH, TestTime ) ; lcd_outdezAtt( 190, 2*FH, tt/2, 0 ) ;
//	PUT_HEX4( 134, 3*FH, ClearTime ) ; lcd_outdezAtt( 190, 3*FH, ct/2, 0 ) ;
//	PUT_HEX4( 134, 4*FH, MenuTime ) ; lcd_outdezAtt( 190, 4*FH, mt/2, 0 ) ;
//#endif

//#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
//  lcd_puts_Pleft( 3*FH, XPSTR("tBgRead        ms"));
//  lcd_outdezAtt(14*FW+STAT2_OFF_0 , 3*FH, (g_timeBgRead)/20 ,PREC2);
//#endif
  
extern uint8_t AudioVoiceCountUnderruns ;
	lcd_puts_Pleft( 5*FHPY, XPSTR("Voice underruns"));
  lcd_outdezAtt( 20*FW+STAT2_OFF_0, 5*FHPY, AudioVoiceCountUnderruns, 0 ) ;

  lcd_puts_P( 3*FW+STAT2_OFF_0,  6*FHPY, PSTR(STR_MENU_REFRESH));

//extern uint32_t IdleCount ;
  
//	PUT_HEX4( 0,  7*FH, IdleCount >> 16 ) ;
//  PUT_HEX4( 30, 7*FH, IdleCount ) ;

extern uint32_t IdlePercent ;
  lcd_puts_Pleft( 7*FHPY, XPSTR("Idle time\016%"));
  lcd_outdezAtt( 14*FW-1+STAT2_OFF_0, 7*FHPY, IdlePercent ,PREC2);
//  PUT_HEX4( 75, 7*FH, IdlePercent ) ;
}

void menuProcStatistic(uint8_t event)
{
	MENU(PSTR(STR_STAT), menuTabStat, e_stat1, 1, {0} ) ;

  lcd_puts_Pleft( FH*0, XPSTR("\016TOT\037\001TME\016TSW\037\001STK\016ST%"));

  putsTime(    6*FW, FH*1, s_timer[0].s_timeCumAbs, 0, 0);
  putsTime(   11*FW, FH*1, s_timer[0].s_timeCumSw,      0, 0);

  putsTime(    6*FW, FH*2, s_timer[0].s_timeCumThr, 0, 0);
  putsTime(   11*FW, FH*2, s_timer[0].s_timeCum16ThrP/16, 0, 0);

  putsTime(   11*FW, FH*0, s_timeCumTot, 0, 0);

  uint16_t traceRd = s_traceCnt>MAXTRACE ? s_traceWr : 0;
  uint8_t x=5;
  uint8_t y=60;

#if defined(PCBX12D) || defined(PCBX10)
	pushPlotType( PLOT_BLACK ) ;
#endif
	lcd_hline(x-3,y,120+3+3);
  lcd_vline(x,y-32,32+3);

  for(uint32_t i=0; i<120; i+=6)
  {
    lcd_vline(x+i+6,y-1,3);
  }
  for(uint32_t i=1; i<=120; i++)
  {
    lcd_vline(x+i,y-s_traceBuf[traceRd],s_traceBuf[traceRd]);
    traceRd++;
    if(traceRd>=MAXTRACE) traceRd=0;
    if(traceRd==s_traceWr) break;
  }
#if defined(PCBX12D) || defined(PCBX10)
		popPlotType() ;
#endif
}
 
 

void menuProcBattery(uint8_t event)
{
	MENU(PSTR(STR_BATTERY), menuTabStat, e_battery, 1, {0} ) ;

  switch(event)
  {
  	case EVT_KEY_BREAK(KEY_MENU):
      g_timeMain = 0;
#ifdef PCBSKY
			Current_max = 0 ;
#endif
    break;
    case EVT_KEY_LONG(KEY_MENU):
#ifdef PCBSKY
			MAh_used = 0 ;
			Current_used = 0 ;
#endif
      audioDefevent(AU_MENUS) ;
    	killEvents(event) ;
    break;
  	
//		case EVT_ENTRY :
//extern void convertFont() ;
//		 convertFont() ;
//    break;

  }

		lcd_puts_Pleft( 2*FHPY, PSTR(STR_Battery));
		putsVolts( 13*FW, 2*FHPY, g_vbat100mV, 0 ) ;

#ifdef PCBSKY
#ifndef REVA
		if ( ( g_eeGeneral.ar9xBoard == 0 ) && ( g_eeGeneral.extraPotsSource[0] != 2 ) && ( g_eeGeneral.extraPotsSource[1] != 2 ) )
		{
			Current_sum += Current_current ;
			if ( ++Current_count > 49 )
			{
				Current = Current_sum / 500 ;
				Current_sum = 0 ;
				Current_count = 0 ;
			}
			lcd_puts_Pleft( 3*FHPY, PSTR(STR_CURRENT_MAX));
	  	lcd_outdezAtt( 13*FW, 3*FHPY, Current, 0 ) ;
	  	lcd_outdezAtt( 20*FW, 3*FHPY, Current_max/10, 0 ) ;
			lcd_puts_Pleft( 4*FHPY, XPSTR("mAh\017[MENU]"));
	  	lcd_outdezAtt( 12*FW, 4*FHPY, MAh_used + Current_used/3600 ,PREC1 ) ;
		}
		lcd_puts_Pleft( 6*FH, PSTR(STR_CPU_TEMP_MAX));
	  lcd_outdezAtt( 12*FW-2, 6*FHPY, (((((int32_t)Temperature - 838 ) * 621 ) >> 11 ) - 20) ,0 ) ;
	  lcd_outdezAtt( 20*FW-2, 6*FHPY, (((((int32_t)Max_temperature - 838 ) * 621 ) >> 11 ) - 20) ,0 ) ;
#endif // REVA

		disp_datetime( 5*FHPY ) ;
#endif // PCBSKY

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	disp_datetime( 5*FHPY ) ;
#endif

#ifndef SMALL
extern uint32_t Master_frequency ;
 	lcd_outdezAtt( 5*FW, 7*FHPY, Master_frequency/1000000, 0 ) ;
#endif
}

void menuNewVario(uint8_t event)
{
	EditType = 0 ;
	struct t_newvario *p = &NewVario ;

	MENU(XPSTR("Vario"), menuTabStat, e_vario, 7, {0} ) ;

//	if ( event == EVT_ENTRY )
//	{
//		struct t_vario *p = &NewVario ;
//		if ( p->defaulted == 0 )
//		{
			
//		}
		
//	}
	 
  lcd_puts_Pleft( 1*FHPY, PSTR(STR_VARIO_SRC) ) ;
	p->varioSource = checkIndexed( 1*FHPY, XPSTR(FWx15"\001""\004vspdA2  "), p->varioSource, (mstate2.m_posVert == 1) ) ;

  lcd_puts_Pleft( 2*FHPY, XPSTR("Centre Min.") ) ;
  lcd_outdezAtt( PARAM_OFS, 2*FHPY, -5 + p->varioCenterMin, ((mstate2.m_posVert == 2) ? INVERS : 0)|PREC1) ;
	if (mstate2.m_posVert == 2)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioCenterMin, -16, 5+min(10, p->varioCenterMax+5)) ;
	}

  lcd_puts_Pleft( 3*FHPY, XPSTR("Centre Max.") ) ;
  lcd_outdezAtt( PARAM_OFS, 3*FHPY, 5 + p->varioCenterMax, ((mstate2.m_posVert == 3) ? INVERS : 0)|PREC1) ;
	if (mstate2.m_posVert == 3)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioCenterMax, 5+max(-10, p->varioCenterMin-5), 15) ;
	}

//            case 2:
//              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.frsky.varioCenterSilent, 1);
//              break;
//          }

  lcd_puts_Pleft( 4*FHPY, XPSTR("Vario Min.") ) ;
  lcd_outdezAtt( PARAM_OFS, 4*FHPY, p->varioMin-10, ((mstate2.m_posVert == 4) ? INVERS : 0)) ;
	if (mstate2.m_posVert == 4)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioMin, -7, 7 ) ;
	}

  lcd_puts_Pleft( 5*FHPY, XPSTR("Vario Max.") ) ;
  lcd_outdezAtt( PARAM_OFS, 5*FHPY, p->varioMax+10, ((mstate2.m_posVert == 5) ? INVERS : 0)) ;
	if (mstate2.m_posVert == 5)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioMax, -7, 7 ) ;
	}

	lcd_puts_Pleft( 6*FHPY, PSTR(STR_2SWITCH) ) ;
	p->swtch = edit_dr_switch( PARAM_OFS, 6*FHPY, p->swtch, ((mstate2.m_posVert == 6) ? INVERS : 0), (mstate2.m_posVert == 6) ? EDIT_DR_SWITCH_EDIT : 0, event ) ;

//	int16_t varioCenterMin ;
//	int16_t varioCenterMax ;
//	int16_t varioMax ;
//	int16_t varioMin ;
//	int16_t varioPitch ;
//	uint16_t defaulted ;


}


#ifndef BIG_SCREEN
void menuProcMusic(uint8_t event)
{
	EditType = EE_GENERAL ;
	uint32_t rows = 2 ;
	if ( MusicPlaying == MUSIC_STOPPED )
	{
		rows = 5 ;
	}
	if ( ( MusicPlaying == MUSIC_PLAYING ) || ( MusicPlaying == MUSIC_PAUSED ) )
	{
		rows = 6 ;
	}
	MENU( PSTR(STR_Music), menuTabStat, e_music, rows, {0} ) ;

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

  if ( ( event == EVT_KEY_FIRST(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
	{
		if ( mstate2.m_posVert != 1 )
		{
			killEvents(event) ;
			s_editMode = 0 ;
			event = 0 ;
		}
		switch ( mstate2.m_posVert )
		{
			case 2 :
				VoiceFileType = VOICE_FILE_TYPE_MUSIC ;
      	pushMenu( menuProcSelectVoiceFile ) ;
			break ;
			case 4 :
				if ( MusicPlaying == MUSIC_STOPPED )
				{
					MusicPlaying = MUSIC_STARTING ;
				}
				else
				{
					MusicPlaying = MUSIC_STOPPING ;
				}
			break ;

			case 5 :
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
			if ( mstate2.m_posVert == 5 )
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

#ifdef BIG_SCREEN
	DisplayOffset = MUSIC_OFF_0 ;
#endif

  lcd_puts_Pleft( 1*FHPY, XPSTR("Type")) ;
	
	uint32_t b = g_eeGeneral.musicType ;
	g_eeGeneral.musicType = checkIndexed( 1*FHPY, XPSTR(FWx15"\001""\004NameList"), b, (mstate2.m_posVert == 1) ) ;
	if ( g_eeGeneral.musicType != b )
	{
		g_eeGeneral.musicVoiceFileName[0] = '\0' ;
	}

  lcd_puts_Pleft( 2*FHPY, XPSTR("File")) ;
	lcd_putsAtt( 6*FW+MUSIC_OFF_0, 2*FHPY, (char *)g_eeGeneral.musicVoiceFileName, 0 ) ;
  
	g_eeGeneral.musicLoop = onoffMenuItem( g_eeGeneral.musicLoop, 3*FH, XPSTR("Loop"), mstate2.m_posVert == 3 ) ;
	
	if ( rows == 5 )
	{
  	lcd_puts_Pleft( 4*FHPY, XPSTR("Start")) ;
		if ( mstate2.m_posVert == 5 )
		{
			mstate2.m_posVert = 4 ;
		}
	}
	else
	{
		lcd_puts_Pleft( 4*FHPY, XPSTR("Stop")) ;
		lcd_puts_Pleft( 5*FHPY, MusicPlaying == MUSIC_PAUSED ? XPSTR("\007Resume") : XPSTR("\007Pause")) ;
		if ( g_eeGeneral.musicType )
		{
			if ( MusicPlaying == MUSIC_PLAYING )
			{
				lcd_puts_Pleft( 5*FHPY, XPSTR("Prev<\017>Next")) ;
			}
		}
		lcd_puts_Pleft( 6*FHPY, CurrentPlayName ) ;
	}

	if ( mstate2.m_posVert == 2 )
	{
		lcd_rect( 6*FW-1+MUSIC_OFF_0, 2*FHPY-1, MUSIC_NAME_LENGTH*FW+2, 9 ) ;
	}

	if ( mstate2.m_posVert == 4 )
	{
		lcd_char_inverse( 0+MUSIC_OFF_0, 4*FHPY, 30, 0 ) ;
	}
	else if ( mstate2.m_posVert == 5 )
	{
		lcd_char_inverse( 7*FW+MUSIC_OFF_0, 5*FHPY, 36, 0 ) ;
	}

extern uint32_t BgSizePlayed ;
extern uint32_t BgTotalSize ;

		lcd_hbar( 10+MUSIC_OFF_0, 57, 101, 6, (BgTotalSize - BgSizePlayed) * 100 / BgTotalSize ) ;
	
}
#endif	// BIG_SCREEN

void menuProcMusicList(uint8_t event)
{
	uint32_t i ;
	uint32_t j ;
	if ( PlayListCount < 7 )
	{
		i = 1 ;
		j = PlayListCount ;
	}
	else
	{
		i = PlayListCount - 6 ;
		j = 7 ;
	}
	MENU( XPSTR("Music List"), menuTabStat, e_music1, i, {0} ) ;
	int8_t  sub    = mstate2.m_posVert;
	
	for ( i = 0 ; i < j ; i += 1 )
	{
		lcd_puts_P( 0+MUSIC_OFF_0, (i+1)*FHPY, PlayListNames[i+sub] ) ;
	}

}

#ifndef SMALL
void menuProcTrainDdiag(uint8_t event)
{
	MENU(XPSTR("Train diag"), menuTabStat, e_traindiag, 3, {0} ) ;
	
	int8_t sub = mstate2.m_posVert ;

  lcd_puts_Pleft( 1*FHPY, XPSTR("Trainer Mode") ) ;
	lcd_putsAttIdx(16*FW, FHPY, XPSTR("\004NormSer Com1"), TrainerMode, (sub == 1) ? INVERS : 0 ) ;
#ifndef SMALL
  lcd_puts_Pleft( 2*FHPY, XPSTR("Trainer In Pol") ) ;
	lcd_putsAttIdx(16*FW, 2*FHPY, XPSTR("\003NegPos"), TrainerPolarity, (sub == 2) ? INVERS : 0 ) ;
#endif

#ifndef SMALL
//uint32_t size = (uint32_t)&_estack - (uint32_t)&_ebss ;

//	PUT_HEX4( 0, 4*FH, size >> 16 ) ;
//	PUT_HEX4( 24, 4*FH, size ) ;
#endif


#if defined(PCBLEM1)


#endif

	if(sub==1)
  {
		uint8_t b = TrainerMode ;
//		TrainerMode = checkIncDec16( TrainerMode, 0, 2, 0 ) ;
#ifndef REVX
		if ( g_model.telemetryRxInvert )
		{
			TrainerMode = 0 ;	// software serial in use
		}
#endif	// nREVX
		if ( TrainerMode != b )
		{
			if ( TrainerMode == 2 )
			{
#ifdef PCBSKY
				setCaptureMode( 0 ) ;			
#else
				init_trainer_capture( 0 ) ;
#endif
				init_software_com1( 9600, 0, 0 ) ;
			}
			else
			{
#ifdef PCBSKY
				configure_pins( (PIO_PA5 | PIO_PA6), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;
				setCaptureMode( TrainerMode ) ;			
#else
				init_trainer_capture( TrainerMode ) ;
#endif
			}
		}
  }
#ifndef SMALL
	else if(sub==2)
  {
		uint8_t b = TrainerPolarity ;
		TrainerPolarity = checkIncDec16( TrainerPolarity, 0, 1, 0 ) ;
		if ( TrainerPolarity != b )
		{
			if ( TrainerMode == 1 )
			{
#ifdef PCBSKY
				setCaptureMode( TrainerMode ) ;			
#else
				init_trainer_capture( TrainerMode ) ;
#endif
			}
		}
	}
#endif
}
#endif	// nSMALL


void menuProcSDstat(uint8_t event)
{
	MENU(PSTR(STR_ST_CARD_STAT), menuTabStat, e_Setup3, 1, {0} ) ;

#ifdef PCBX9D
	if ( event == EVT_ENTRY )
	{
extern void sdInit( void ) ;
		sdInit() ;
	}
#endif
	 
	uint32_t present ;
	lcd_puts_Pleft( 1*FHPY, XPSTR("Present"));

#ifdef PCBSKY
	present = CardIsPresent() ;
#else
extern DWORD socket_is_empty( void ) ;
	present = !socket_is_empty() ;
#endif
	lcd_putsAttIdx( 9*FW, 1*FHPY, XPSTR("\003No Yes"), present, 0 ) ;
	
	lcd_puts_Pleft( 2*FHPY, PSTR(STR_4_READY));

extern uint8_t SectorsPerCluster ;
  lcd_outdezAtt( MENU_DISPLAY_RIGHT, 1*FHPY, SectorsPerCluster/2, 0 ) ;
#ifndef SIMU


#ifdef PCBSKY
#ifndef SMALL
	uint32_t i ;
	uint8_t x, y ;
#endif
#endif

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
extern uint8_t CardType ;
extern uint32_t sdMounted( void ) ;
	PUT_HEX4( 10*FW, 2*FHPY, CardType ) ;
	PUT_HEX4( 16*FW, 2*FHPY, sdMounted() ) ;
	PUT_HEX4( 10*FW, 3*FHPY, Card_state ) ;
#endif

#ifdef PCBSKY
	PUT_HEX4( 10*FW, 2*FHPY, Card_state ) ;
	 
#ifndef SMALL
extern uint32_t SDlastError ;
	PUT_HEX4( 16*FW, 2*FHPY, SDlastError ) ;
	 
		y = 3*FHPY ;
		x = 4*FW ;
		lcd_puts_Pleft( y, XPSTR("CID"));
		for ( i = 0 ; i < 4 ; i += 1 )
		{
		  PUT_HEX4( x, y, Card_ID[i] >> 16 ) ;
		  PUT_HEX4( x+4*FW, y, Card_ID[i] ) ;
			x += 8*FW ;
			if ( i == 1 )
			{
				y += FHPY ;
				x = 4*FW ;				
			}			 
		}
		y = 5*FHPY ;
		x = 4*FW ;
		lcd_puts_Pleft( y, XPSTR("CSD"));
		for ( i = 0 ; i < 4 ; i += 1 )
		{
		  PUT_HEX4( x, y, Card_CSD[i] >> 16 ) ;
		  PUT_HEX4( x+4*FW, y, Card_CSD[i] ) ;
			x += 8*FW ;
			if ( i == 1 )
			{
				y += FHPY ;
				x = 4*FW ;				
			}			 
		}
		y = 7*FHPY ;
		x = 4*FW ;
		lcd_puts_Pleft( y, XPSTR("SCR"));
		for ( i = 0 ; i < 2 ; i += 1 )
		{
		  PUT_HEX4( x, y, Card_SCR[i] >> 16 ) ;
		  PUT_HEX4( x+4*FW, y, Card_SCR[i] ) ;
			x += 8*FW ;
		}
#endif
#endif
	if (sd_card_ready() )
	{
	}
	else
	{
		lcd_puts_Pleft( 2*FHPY, PSTR(STR_NOT));
	}
#endif
}

void menuProcBoot(uint8_t event)
{
  MENU(PSTR(STR_BOOT_REASON), menuTabStat, e_Boot, 1, {0/*, 0*/});

#ifdef BIG_SCREEN
	DisplayOffset = BOOT_OFF_0 ;
#endif

#if defined(PCBSKY) || defined(PCB9XT)
	lcd_puts_Pleft( 7*FHPY, XPSTR("Chip") ) ;
	PUT_HEX4( (5*FW)+BOOT_OFF_0, 7*FHPY, ChipId >> 16 ) ;
	PUT_HEX4( (9*FW)+BOOT_OFF_0, 7*FHPY, ChipId ) ;
#endif	

#ifdef PCBSKY
	if ( ( ResetReason & RSTC_SR_RSTTYP ) == (2 << 8) )	// Watchdog
	{
		lcd_puts_Pleft( 2*FHPY, PSTR(STR_6_WATCHDOG) ) ;
	}
	else if ( unexpectedShutdown )
	{
		lcd_puts_Pleft( 2*FHPY, PSTR(STR_5_UNEXPECTED) ) ;
		lcd_puts_Pleft( 3*FHPY, PSTR(STR_6_SHUTDOWN) ) ;
	}
	else
	{
		lcd_puts_Pleft( 2*FHPY, PSTR(STR_6_POWER_ON) ) ;
	}

#ifdef WDOG_REPORT
extern uint16_t WdogIntValue ;
	PUT_HEX4( 100+BOOT_OFF_0, 5*FH, WdogIntValue ) ;
#endif

#endif
 #if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
  #if defined(PCBLEM1)
	if ( ResetReason & RCC_CSR_IWDGRSTF )	// Watchdog
	{
		lcd_puts_Pleft( 2*FHPY, PSTR(STR_6_WATCHDOG) ) ;
  #else
	if ( ResetReason & RCC_CSR_WDGRSTF )	// Watchdog
	{
		lcd_puts_Pleft( 2*FHPY, PSTR(STR_6_WATCHDOG) ) ;
  #ifdef PCB9XT
extern uint32_t WatchdogPosition ;
	PUT_HEX4( BOOT_OFF_0, 3*FHPY, WatchdogPosition ) ;
  #endif
 #endif
	}
	else if ( unexpectedShutdown )
	{
		lcd_puts_Pleft( 2*FHPY, PSTR(STR_5_UNEXPECTED) ) ;
		lcd_puts_Pleft( 3*FHPY, PSTR(STR_6_SHUTDOWN) ) ;
	}
	else
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_6_POWER_ON) ) ;
	}
#endif
  lcd_outdez( (20*FW)+BOOT_OFF_0, 6*FHPY, ( ResetReason >> 8 ) & 7 ) ;
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1) || defined(PCBX7ACCESS)
	PUT_HEX4( BOOT_OFF_0, 6*FHPY, ResetReason >> 16 ) ;
	PUT_HEX4( 25+BOOT_OFF_0, 6*FHPY, ResetReason ) ;
	PUT_HEX4( BOOT_OFF_0, 5*FHPY, RCC->BDCR ) ;
	PUT_HEX4( 25+BOOT_OFF_0, 5*FHPY, RCC->CSR ) ;

#ifdef WDOG_REPORT
extern uint16_t WdogIntValue ;
	PUT_HEX4( 100+BOOT_OFF_0, 5*FHPY, WdogIntValue ) ;
#endif

#endif

#ifdef WDOG_REPORT
#if defined(PCBX12D) || defined(PCBX10)
extern uint16_t WdogIntValue ;
 	lcd_putc( 90+BOOT_OFF_0, 5*FHPY, '>') ;
	PUT_HEX4( 100+BOOT_OFF_0, 5*FHPY, WdogIntValue ) ;
#endif
#endif

#ifdef STACK_PROBES
extern uint32_t stackSpace( uint32_t stack ) ;

	PUT_HEX4( 0+BOOT_OFF_0, 4*FHPY, stackSpace(0) ) ;
	PUT_HEX4( 30+BOOT_OFF_0, 4*FHPY, stackSpace(1) ) ;
	PUT_HEX4( 60+BOOT_OFF_0, 4*FHPY, stackSpace(2) ) ;
 #ifndef PCBX12D
  #ifndef PCBX10
	PUT_HEX4( 90+BOOT_OFF_0, 4*FHPY, stackSpace(3) ) ;
  #endif
 #endif
 #ifdef MIXER_TASK
	PUT_HEX4( 90+BOOT_OFF_0, 5*FHPY, stackSpace(4) ) ;
 #endif
 #ifndef SMALL
	PUT_HEX4( 60+BOOT_OFF_0, 5*FHPY, stackSpace(5) ) ;
extern uint32_t StackAtOsStart ;
	PUT_HEX4( 60+BOOT_OFF_0, 7*FHPY, StackAtOsStart >> 16 ) ;
	PUT_HEX4( 60+BOOT_OFF_0+4*FW, 7*FHPY, StackAtOsStart ) ;
 #endif
#endif

}

 #endif	// X12/X10

#ifdef BLUETOOTH

void menuProcBt(uint8_t event)
{
  MENU(XPSTR("BT"), menuTabStat, e_bluetooth, 7, {0/*, 0*/});

#ifdef PCBSKY

extern uint16_t BtLastSbusSendTime ;
//extern uint16_t BtParaDebug ;

	PUT_HEX4(50, 0, BtLastSbusSendTime ) ;
//	PUT_HEX4(80, 0, BtParaDebug ) ;

 #ifdef BT_PDC
	uint16_t *p = (uint16_t *)&BtPdcFifo ;
 #else
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
 #endif
#endif
#ifdef PCB9XT
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
#endif
#if defined(PCBX7) || defined(PCBX9D) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
#endif
	uint32_t i ;
	uint16_t x ;
#ifndef SMALL
	p += mstate2.m_posVert * 5 ;
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 1*FHPY, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 2*FHPY, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 3*FHPY, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 4*FHPY, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 5*FHPY, x ) ;
	}
#else
	uint32_t j ;
	p += mstate2.m_posVert * 5 ;
	for ( j = 0 ; j < 5 ; j += 1 )
	{
		for ( i = 0 ; i < 5 ; i += 1 )
		{
			x = *p++ ;
			x = ( x >> 8 ) | ( x << 8 ) ;
			PUT_HEX4( i*25, (j+1)*FHPY, x ) ;
		}
	}
#endif	 
#ifndef PCB9XT
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		PUT_HEX4( i*25, 6*FHPY, x ) ;
	}
#endif

	PUTS_NUMX( 25, 7*FHPY, BtControl.BtBadChecksum ) ;

//	lcd_outdez( 50, 7*FH, BtControl.BtLinking ) ;
	 
#ifdef BLUETOOTH
	PUTS_NUMX( 75, 7*FHPY, BtRxTimer ) ;
	PUTS_NUMX( 100, 7*FHPY, BtControl.BtCurrentLinkIndex ) ;
	PUTS_NUMX( 125, 7*FHPY, BtControl.BtBaudChangeIndex ) ;
#endif

#if defined(PCBX10) && defined(PCBREV_EXPRESS)

	PUT_HEX4( 25*FW, 1*FHPY, USART6->CR1 ) ;
	PUT_HEX4( 25*FW, 2*FHPY, USART6->SR ) ;
	PUT_HEX4( 25*FW, 3*FHPY, USART6->BRR ) ;
	PUT_HEX4( 25*FW, 4*FHPY, GPIOG->IDR ) ;


#endif
}
#endif


#ifdef PCB9XT
void menuProcSlave(uint8_t event)
{
  MENU(XPSTR("Slave"), menuTabStat, e_Slave, 1, {0/*, 0*/});

  if ( event == EVT_KEY_LONG(KEY_MENU) )
	{
		updateSlave() ;
		initM64() ;
	}

	lcd_puts_Pleft( 2*FH, XPSTR("Revision") ) ;
  lcd_outdezAtt( 14*FW, 2*FH, M64Revision, PREC1 ) ;
	lcd_puts_Pleft( 3*FH, XPSTR("Available") ) ;
	int32_t x = newSlaveRevision() ;
	lcd_outdezAtt( 14*FW, 3*FH, x, PREC1 ) ;
	lcd_puts_Pleft( 5*FH, XPSTR("Update  (MENU LONG)") ) ;
	lcd_puts_Pleft( 7*FH, XPSTR("Reset Count") ) ;

extern uint16_t RemBitMeasure ;
extern uint16_t M64UsedBackup ;
  lcd_outdez( 14*FW, 6*FH, M64UsedBackup ) ;
  lcd_outdez( 20*FW, 6*FH, RemBitMeasure ) ;

  lcd_outdez( 14*FW, 7*FH, M64ResetCount ) ;
  lcd_outdez( 20*FW, 7*FH, M64RestartCount ) ;

//extern uint16_t M64RxCount ;
//extern uint8_t M64MainTimer ;
//extern uint8_t SlaveState ;
//	PUT_HEX4( 17*FW, 2*FH, M64RxCount ) ;
//	PUT_HEX4( 17*FW, 3*FH, M64MainTimer ) ;
//	PUT_HEX4( 17*FW, 4*FH, SlaveState ) ;
}
#endif // PCB9XT

#if defined(PCBX12D) || defined(PCBX10)
#else
#ifndef SMALL
void menuProcS6R(uint8_t event)
{
	static uint8_t state ;
	static uint8_t index ;
	static uint8_t lastSub ;
	static uint8_t valid ;
	uint8_t min ;
	uint8_t max ;
	static int16_t value ;
	static uint8_t calState ;
#ifdef PAGE_NAVIGATION
	static uint8_t rotaryLocal ;
#endif

	MENU(XPSTR("SxR Config."), menuTabStat, e_s6r, 28, {0} ) ;
	int8_t sub = mstate2.m_posVert ;

	if ( StatusTimer )
	{
		if ( --StatusTimer == 0 )
		{
			state = ( sub != 27 ) ? 0 : 1 ;
		}
	}
	if ( event == EVT_ENTRY )
	{
		state = 0 ;
		calState = 0 ;
		S6Rdata.valid = 0 ;
		lastSub = 0 ;
#ifdef PAGE_NAVIGATION
		rotaryLocal = 0 ;
#endif
		StatusTimer = 0 ;
	}
	if ( event == EVT_KEY_BREAK(KEY_MENU) )
	{
		if ( valid == 0 )
		{
			if ( sub != 27 )
			{
				s6rRequest( 0x30, index, 0 ) ;
				state = 1 ;			
				StatusTimer = 230 ;
			}
		}
		s_editMode = 0 ;
	}
	if ( event == EVT_KEY_LONG(KEY_MENU) )
	{
		if ( valid || sub == 27 )
		{
			s6rRequest( 0x31, index, value ) ;
	    killEvents(event) ;
			StatusTimer = 100 ;
			if ( sub == 27 )
			{
				valid = 0 ;
				StatusTimer = 50 ;
			}
			state = 2 ;
		}
		s_editMode = 0 ;
	}

	if ( state == 1 )
	{
		if ( S6Rdata.valid )
		{
			state = 0 ;
			if ( index == S6Rdata.fieldIndex )
			{
				valid = 1 ;
				value = S6Rdata.value ;
			}
		}
		lcd_puts_Pleft( 4*FHPY, "\016Reading" ) ;
	}
	if ( state == 2 )
	{
		lcd_puts_Pleft( 4*FHPY, "\016Writing" ) ;
	}
	if ( lastSub != sub )
	{
		valid = 0 ;
		lastSub = sub ;
	}
	min = 0 ;
	max = 2 ;
	switch ( sub )
	{
		case 1 :
			lcd_puts_Pleft( 2*FHPY, "Wing type" ) ;
			index = 0x80 ;
			if ( valid )
			{
  			lcd_putsAttIdx( 12*FW, 3*FH, XPSTR("\006NormalDelta VTail "), value, InverseBlink ) ;
			}
		break ;
		case 2 :
			lcd_puts_Pleft( 2*FH, "Mounting" ) ;
			index = 0x81 ;
			max = 3 ;
			if ( valid )
			{
  			lcd_putsAttIdx( 12*FW, 3*FH, XPSTR("\006Hor   HorRevVer   VerRev"), value, InverseBlink ) ;
			}
			lcd_img( 12, 4*FH, value<2 ? S6Rimg1 : S6Rimg3, value<3 ? value & 1 : 2, 0 ) ;
			lcd_puts_Pleft( 7*FH, "<-Heading" ) ;
		break ;
		case 3 :
		case 4 :
		case 5 :
		case 6 :
		case 7 :
			lcd_puts_Pleft( 2*FH, "\005Dir." ) ;
			index = 0x82 + sub - 3 ;
  		lcd_putsAttIdx( 0, 2*FH, XPSTR("\004Ail Ele Rud Ail2Ele2"), sub-3, 0 ) ;
			if ( sub == 6 )
			{
				index = 0x9A ;
			}
			if ( sub == 7 )
			{
				index = 0x9B ;
			}
			max = 2 ;
			if ( valid )
			{
				uint8_t t = (value+1) ;
				if ( value == 0x80 )
				{
					t = 2 ;
				}
  			lcd_putsAttIdx( 12*FW, 3*FH, XPSTR("\006NormalInversOff   "), t, InverseBlink ) ;
			}
		break ;
		case 8 :
		case 9 :
		case 10 :
		case 11 :
		case 12 :
		case 13 :
		case 14 :
		case 15 :
		case 16 :
			lcd_puts_Pleft( 2*FH, sub < 11 ? "\004Stab Gain" : sub < 13 ? "\004Auto Level Gain" : sub < 15 ? "\004Upright Gain" : "\004Crab Gain" ) ;
  		lcd_putsAttIdx( 0, 2*FH, XPSTR("\003AilEleRudAilEleEleRudAilRud"), sub-8, 0 ) ;
			index = 0x85 + sub - 8 ;
			if ( sub > 12 )
			{
				index += 2 ;
			}
			if ( sub > 15 )
			{
				index += 1 ;
			}
			max = 200 ;
			if ( valid )
			{
				lcd_outdez( 17*FW, 3*FH, value ) ;
			}
		break ;
		case 17 :
		case 18 :
		case 19 :
		case 20 :
		case 21 :
		case 22 :
			lcd_puts_Pleft( 2*FH, sub < 19 ? "\004auto angle offset" : sub < 21 ? "\004up angle offset" : "\004crab angle offset" ) ;
  		lcd_putsAttIdx( 0, 2*FH, XPSTR("\003AilEleEleRudAilRud"), sub-17, 0 ) ;
			index = "\x91\x92\x95\x96\x97\x99"[sub-17] ;
			min = 0x6C ;
			max = 0x94 ;
			if ( valid )
			{
				lcd_outdez( 17*FW, 3*FH, value-128 ) ;
			}
		break ;
		case 23 :
		case 24 :
		case 25 :
		case 26 :
//			lcd_puts_Pleft( 2*FH, "Active" ) ;
  		lcd_putsAttIdx( 0, 2*FH, XPSTR("\006ActiveAux1  Aux2  QuickM"), sub-23, 0 ) ;
			index = "\x9C\xA8\xA9\xAA"[sub-23] ;
//			index = 0x9C ;
			max = 1 ;
			if ( valid )
			{
  			lcd_putsAttIdx( 12*FW, 3*FH, XPSTR("\007DisableEnable "), value, InverseBlink ) ;
			}
		break ;
		case 27 :
			lcd_puts_Pleft( 1*FH, "Calibration, place\037the S6R as shown\037press MENU LONG" ) ;
			lcd_img( 12, 4*FH, calState<2 ? S6Rimg1 : calState<4 ? S6Rimg2 : S6Rimg3, calState & 1, 0 ) ;
			if ( calState == 1 )
			{
				lcd_puts_Pleft( 7*FH, "Inverted" ) ;
			}
			index = 0x9D ;
			value = calState ;
			if ( valid )
			{
				calState += 1 ;
				calState %= 6 ;
				valid = 0 ;
			}
		break ;
	}
	if ( sub != 27 )
	{
		if ( valid && sub )
		{
			if ( state == 0 )
			{
				if ( ( (index >= 0x82) && (index <= 0x84) ) || (index == 0x9A ) || ( index == 0x9B ) )
				{
					value = value == 255 ? 0 : value == 0 ? 1 : 2 ;
				}
				if ( event != EVT_KEY_FIRST(KEY_MENU) )
				{
#ifdef PAGE_NAVIGATION
					if ( event == EVT_KEY_FIRST(BTN_RE) )
					{
						killEvents( event ) ;
						Tevent = event = 0 ;
						if ( RotaryState != ROTARY_VALUE )
						{
							rotaryLocal = ROTARY_VALUE ;
						}
						else
						{
							rotaryLocal = ROTARY_MENU_UD ;
						}
					}
					RotaryState = rotaryLocal ;
					s_editMode = ( rotaryLocal == ROTARY_VALUE ) ;
#endif	// PAGE_NAVIGATION
					value = checkIncDec16( value, min, max, NO_MENU_ONLY_EDIT ) ;
				}
				if ( ( (index >= 0x82) && (index <= 0x84) ) || (index == 0x9A ) || ( index == 0x9B ) )
				{
					value = value == 0 ? 255 : value == 1 ? 0 : 0x80 ;
				}
			}
		}
		else
		{
			lcd_puts_Pleft( 3*FH, "\014-----" ) ;
		}
	}

//  PUT_HEX4( 100, 5*FH, S6Rdata.valid ) ;
//  PUT_HEX4( 100, 6*FH, S6Rdata.fieldIndex ) ;
//  PUT_HEX4( 100, 7*FH, S6Rdata.value ) ;
}
#endif //n SMALL
#endif // X12/X10

#endif	// PROP_TEXT

void menuDebug(uint8_t event)
{
	MENU(XPSTR("DEBUG"), menuTabStat, e_debug, 146, {0} ) ;

  switch(event)
	{
		case EVT_KEY_LONG(KEY_MENU):
			RawLogging += 1 ;
			if ( RawLogging > 2 )
			{
				RawLogging = 0 ;
			}
			killEvents(event) ;
		break ;
	}

extern uint16_t TelRxCount ;

#ifdef PROP_TEXT

	lcdDrawTextLeft( 6*FH, XPSTR("Raw Logging") ) ;
	lcdDrawTextAtIndex( PARAM_OFS, 6*FH, XPSTR("\003OFFBinHex"), RawLogging, 0 ) ;
	lcdDrawTextLeft( 7*FH, XPSTR("TelRxCount") ) ;
  PUT_HEX4( 12*FW,  7*FH, TelRxCount ) ;
  PUT_HEX4( 17*FW,  7*FH, FrskyTelemetryType | (TelemetryType << 8) ) ;

#else
 #if defined(PCBX12D) || defined(PCBX10)
	PUTS_ATT_LEFT( 5*FHPY, XPSTR("Raw Logging") ) ;
	PUTS_AT_IDX( PARAM_OFS, 5*FHPY, XPSTR("\003OFFBinHex"), RawLogging, 0 ) ;
	PUTS_ATT_LEFT( 6*FHPY, XPSTR("TelRxCount") ) ;
  PUT_HEX4( 12*FW,  6*FHPY, TelRxCount ) ;
  PUT_HEX4( PARAM_OFS,  6*FHPY, FrskyTelemetryType | (TelemetryType << 8) ) ;
 #else
	lcd_puts_Pleft( 6*FH, XPSTR("Raw Logging") ) ;
	PUTS_AT_IDX( PARAM_OFS, 6*FH, XPSTR("\003OFFBinHex"), RawLogging, 0 ) ;

	lcd_puts_Pleft( 7*FH, XPSTR("TelRxCount") ) ;
  PUT_HEX4( 12*FW,  7*FH*HVSCALE, TelRxCount ) ;
  PUT_HEX4( 17*FW,  7*FH*HVSCALE, FrskyTelemetryType | (TelemetryType << 8) ) ;
 #endif

#endif

}

#ifdef TOUCH

uint16_t TouchX ;
uint16_t TouchY ;
uint16_t TouchSize ;
uint16_t TouchTrack ;

uint32_t TouchCount ;

uint32_t touchReadData() ;

void menuTouch(uint8_t event)
{
	uint32_t count = 0 ;
	uint16_t x ;
	uint16_t y ;

	TITLE(XPSTR("Touch")) ;
	
	static MState2 mstate2 ;
	event = mstate2.check( event, e_touch, menuTabStat, DIM(menuTabStat), 0, 0, 0 ) ;

	if ( TouchUpdated )
	{
		if ( TouchControl.event == TEVT_DOWN )
		{
			TouchX = TouchControl.x ;
			TouchY = TouchControl.y ;
			x = TouchX ;
			y = TouchY ;
			count = 1 ;
		}
		else if ( TouchControl.event == TEVT_UP )
		{
			TouchX = 0 ;
			TouchY = 0 ;
		}
		TouchUpdated = 0 ;
	}

  PUT_HEX4( 0, 3*FH, TouchX ) ;
  PUT_HEX4( 0, 4*FH, TouchY ) ;

	if ( count )
	{
		if ( x < 25 )
		{
			x = 25 ;
		}
		if ( y < 25 )
		{
			y = 25 ;
		}
		if ( x > 450 )
		{
			x = 450 ;
		}
		if ( y > 220 )
		{
			y = 220 ;
		}

		lcdHiresRect( x-24, y-24, 49, 49, LCD_RED ) ;
		lcdHiresRect( x-23, y-23, 47, 47, LCD_RED ) ;

	}


}

#endif





