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
#ifdef FRSKY
#include "frsky.h"
#endif
#ifndef SIMU
#include "CoOS.h"
#endif

#ifdef PCBX9D
#include "X9D/eeprom_rlc.h"
#include "timers.h"
#include "analog.h"
#endif

#ifdef PCB9XT
#include "timers.h"
#include "analog.h"
#include "mega64.h"
#include "isp.h"
#endif

#include "ff.h"
#include "maintenance.h"

#ifdef REVX
 #include "jeti.h"
#endif

#ifdef MAVLINK
//#include <math.h>
//#define PI 3.14159265
#endif

//#ifdef REVX
//#define USE_EXT_RTC		1
//#endif



#define PARAM_OFS   17*FW

extern uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext ) ;
void setupFileNames( TCHAR *dir, struct fileControl *fc, char *ext ) ;
static struct fileControl FileControl = {	0,0,0,0, {0,0,0,0} } ;
struct fileControl PlayFileControl = {	0,0,0,0, {0,0,0,0} } ;

//#ifdef SPLASH_DEBUG
//extern uint16_t SplashDebug[] ;
//#endif

#ifdef PCBX9D
extern uint8_t ImageDisplay ;
#endif

union t_xmem Xmem ;

extern int32_t Rotary_diff ;
extern int16_t AltOffset ;
extern uint8_t ExtraInputs ;

static uint8_t s_currIdx;
static uint8_t RestoreIndex ;
static uint8_t SubMenuFromIndex = 0 ;

#define ALPHA_NO_NAME		0x80

#ifdef REVX
uint16_t jeti_keys = JETI_KEY_NOCHANGE;
uint8_t JetiBuffer[36] ; // 32 characters
uint8_t JetiIndex ;
uint8_t JetiBufferReady ;
#endif

struct t_timer s_timer[2] ;

uint8_t RotaryState ;		// Defaults to ROTARY_MENU_LR
uint8_t CalcScaleNest = 0 ;

uint8_t ThrottleStickyOn = 0 ;

struct t_popupData PopupData ;

static uint8_t SubmenuIndex = 0 ;
static uint8_t IlinesCount ;
static uint8_t LastSubmenuIndex = 0 ;
static uint8_t UseLastSubmenuIndex = 0 ;

#define VOICE_FILE_TYPE_NAME	0
#define VOICE_FILE_TYPE_USER	1
#define VOICE_FILE_TYPE_MUSIC	2
uint8_t VoiceFileType ;
uint8_t FileSelectResult ;
char SelectedVoiceFileName[16] ;

void menuProcSelectVoiceFile(uint8_t event) ;
#ifdef PCBX9D
void menuProcSelectImageFile(uint8_t event) ;
#endif
int8_t getAndSwitch( SKYCSwData &cs ) ;

//#ifdef MAVLINK
static int16_t hdg_home ;// Extra data for Mavlink via FrSky
//#endif

static uint8_t Columns ;

int8_t phyStick[4] ;

const uint16_t UnitsVoice[] = {SV_FEET,SV_VOLTS,SV_DEGREES,SV_DEGREES,SV_MILAMP_H,SV_AMPS,SV_METRES,SV_WATTS,SV_PERCENT } ;
const uint8_t UnitsText[] = { 'F','V','C','F','m','A','m','W','%' } ;
const uint8_t UnitsString[] = "\005Feet VoltsDeg_CDeg_FmAh  Amps MetreWattsPcent" ;

//const char MultiString[] = "\006FlyskyHubsanFrskyDHisky V2x2  DSM   Devo  YD717 KN    SymaX SLT   CX10  CG023 BayangFrskyXESky  MT99xxMJXq  ShenqiFY326 SFHSS J6PRO FQ777 ASSAN FrskyVHONTAIOpnLrs" ;
//#define NUMBER_MULTI_STRINGS	24

#define BaudString FWx13"\005""\006  AUTO  9600 19200 38400 57600115200"
#define BtBaudString "\006115200  9600 19200 57600 38400"


uint16_t Multiprotocols[NUM_MULTI_PROTOCOLS] ;
#define MULTI_TEXT_SIZE	480
uint8_t MultiText[MULTI_TEXT_SIZE] ;
uint8_t MultiMapping[64] ;

const uint8_t MfileData[] = 
"1,Flysky,Flysky,V9x9,V6x6,V912\r\n"
"2,Hubsan\r\n"
"3,FrskyD\r\n"
"4,Hisky,Hisky,HK310\r\n"
"5,V2x2\r\n"
"6,DSM,DSM2-22,DSM2-11,DSMX-22,DSMX-11,AUTO\r\n"
"7,Devo\r\n"
"8,YD717,YD717,SKYWLKR,SYMAX4,XINXUN,NIHUI\r\n"
"9,KN,WLTOYS,FEILUN\r\n"
"10,SymaX,SYMAX,SYMAX5C\r\n"
"11,SLT\r\n"
"12,CX10,GREEN,BLUE,DM007,Q282,J3015_1,J3015_2,MK33041,Q242\r\n"
"13,CG023,CG023,YD829,H8_3D\r\n"
"14,Bayang\r\n"
"15,FrskyX,CH_16,CH_8\r\n"
"16,ESky\r\n"
"17,MT99xx,MT,H7,YZ,LS\r\n"
"18,MJXq,WLH08,X600,X800,H26D,E010\r\n"
"19,Shenqi\r\n"
"20,FY326\r\n"
"21,SFHSS\r\n"
"22,J6PRO\r\n"
"23,FQ777\r\n"
"24,ASSAN\r\n"
"25,FrskyV\r\n"
"26,HONTAI,HONTAI,JJRCX1,X5C1\r\n"
"27,OpnLrs" ;


const uint8_t IconLogging[] =
{
5,8,5,
0xff,0xC1,0xdf,0xdf,0xff
};


// TSSI set to zero on no telemetry data
const int8_t TelemIndex[] = { FR_A1_COPY, FR_A2_COPY,
															FR_RXRSI_COPY, FR_TXRSI_COPY,
															TIMER1, TIMER2,
															FR_ALT_BARO, FR_GPS_ALT,
															FR_GPS_SPEED, FR_TEMP1, FR_TEMP2, FR_RPM,
														  FR_FUEL, FR_A1_MAH, FR_A2_MAH, FR_CELL_MIN,
															BATTERY, FR_CURRENT, FR_AMP_MAH, FR_CELLS_TOT, FR_VOLTS,
															FR_ACCX, FR_ACCY,	FR_ACCZ, FR_VSPD, V_GVAR1, V_GVAR2,
															V_GVAR3, V_GVAR4, V_GVAR5, V_GVAR6, V_GVAR7, FR_WATT, FR_RXV, FR_COURSE,
															FR_A3, FR_A4, V_SC1, V_SC2, V_SC3, V_SC4, V_SC5, V_SC6, V_SC7, V_SC8, V_RTC, TMOK, FR_AIRSPEED,
															CELL_1,CELL_2,CELL_3,CELL_4,CELL_5,CELL_6,
															FR_RBOX_B1_V,FR_RBOX_B1_A,FR_RBOX_B2_V,FR_RBOX_B2_A, FR_RBOX_B1_CAP, FR_RBOX_B2_CAP } ;

// TXRSSI is always considered valid as it is forced to zero on loss of telemetry
// Values are 0 - always valid, 1 - need telemetry, 2 - need hub

//const uint8_t TelemValid[] = { 1, 1, 1, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 0  } ;

// Make scalers always OK! Removed again
const uint8_t TelemValid[] = { 1, 1, 1, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2  } ;

int8_t edit_dr_switch( uint8_t x, uint8_t y, int8_t drswitch, uint8_t attr, uint8_t flags, uint8_t event ) ;

// parse States
#define GET_INDEX 		0
#define GET_NAME			1
#define	GET_SUB_NAME	2
#define	SKIP_LINE			3

FIL TextFile ;
uint8_t TextMenuBuffer[16*21] ;
uint8_t TextMenuStore[16*21+32] ;	// Allow for CRLF
uint8_t TextLines ;
uint8_t TextOffset ;
uint8_t TextHelp ;
uint8_t HelpTextPage ;

void parseMultiData()
{
	uint8_t *src ;
	uint8_t *text ;
	uint32_t index ;
	uint32_t offset ;
	uint32_t count ;
	uint32_t state ;
	uint8_t byte ;
	FRESULT result ;
  
	index = 0 ;
	while ( !sd_card_ready() )
	{
		CoTickDelay(5) ;					// 10mS for now
		if ( ++index > 24 )
		{
			break ;
		}
	}
	result = f_open( &TextFile, "Multi.txt", FA_READ ) ;

	src = (uint8_t *)MfileData ;
	text = MultiText ;
	state = GET_INDEX ;
	index = 0 ;
	count = 0 ;
	offset = 0 ;

	for(;;)
	{
		uint32_t next = 0 ;
		if ( result == FR_OK )
		{
			byte = 0 ;
			f_read( &TextFile, &byte, 1, 0 ) ;
		}
		else
		{
			byte = *src++ ;
		}
		if ( byte == '\r' )
		{
			continue ;
		}
		if ( byte == '\n' )
		{
			byte = ',' ;
			next = 1 ;
		}
		if ( byte == 0 )
		{
			next = 2 ;
			byte = ',' ;
		}
		switch ( state )
		{
			case GET_INDEX :
				if ( byte == '#' )
				{
					state = SKIP_LINE ;
					index = 0 ;					
				}
				else if ( byte == ',' )
				{
					state = GET_NAME ;
					offset = text - MultiText ;
				}
				else
				{
					index *= 10 ;
					index += byte - '0' ;
				}
			break ;

			case GET_NAME :
				if ( byte == ',' )
			 	{
					*text++ = '\0' ;
					if ( index < NUM_MULTI_PROTOCOLS )
					{
						Multiprotocols[index] = 0x8000 | offset ;
						state = GET_SUB_NAME ;
					}
				}
				else
				{
					if ( text - MultiText < MULTI_TEXT_SIZE - 2 )
					{
						*text++ = byte ;
					}
				}
			break ;

			case GET_SUB_NAME :
				if ( byte == ',' )
			 	{
					*text++ = '\0' ;
					count += 1 ;
				}
				else
				{
					if ( text - MultiText < MULTI_TEXT_SIZE - 2 )
					{
						*text++ = byte ;
					}
				}
			break ;
			case SKIP_LINE :
				
			break ;
		}
		if ( next )
		{
			state = GET_INDEX ;
			if ( index < NUM_MULTI_PROTOCOLS )
			{
				if ( count > 8 )
				{
					count = 8 ;
				}
				Multiprotocols[index] |= count << 11 ;
			}
			index = 0 ;
			count = 0 ;
			if ( next == 2 )
			{
				break ;
			}
		}
	}
	*text = '\0' ;
	if ( result == FR_OK )
	{
		f_close( &TextFile ) ;
	}
	offset = 0 ;
	for ( index = 1 ; index < NUM_MULTI_PROTOCOLS ; index += 1 )
	{
		if ( Multiprotocols[index] & 0x8000 )
		{
			MultiMapping[offset++] = index - 1 ;
		}
	}
	for ( index = 1 ; index < 64 ; index += 1 )
	{
		if ( index < NUM_MULTI_PROTOCOLS )
		{
			if ( ( Multiprotocols[index] & 0x8000 ) == 0 )
			{
				MultiMapping[offset++] = index - 1 ;
			}
		}
		else
		{
			MultiMapping[offset++] = index - 1 ;
		}
	}
}

void displayMultiProtocol( uint32_t index, uint32_t y, uint8_t attr )
{
	index += 1 ;
	if ( index < NUM_MULTI_PROTOCOLS )
	{
		uint16_t value = Multiprotocols[index] ;
		if ( value & 0x8000 )
		{
			value &= 0x07FF ;
			lcd_putsAtt( FW*10, y, (char *)&MultiText[value], attr ) ;
			return ;
		}
	}
	lcd_outdezAtt( 21*FW, y, index, attr ) ;
}

uint32_t displayMultiSubProtocol( uint32_t index, uint32_t subIndex, uint32_t y, uint8_t attr )
{
	index += 1 ;
	if ( index < NUM_MULTI_PROTOCOLS )
	{
		uint16_t value = Multiprotocols[index] ;
		if ( value & 0x8000 )
		{
			uint32_t limit = (value >> 11) & 0x0F ;
			if ( limit == 0 )
			{
				lcd_putsAtt( FW*10, y, "None", attr ) ;
				return 0 ;
			}
			if ( subIndex < limit )
			{
				value &= 0x07FF ;
				uint8_t *list = &MultiText[value] ;
				value = subIndex ;
				do
				{
					while ( *list )
					{
						list += 1 ;
					}
					list += 1 ;
				}
				while ( value-- ) ;
				lcd_putsAtt( FW*10, y, (char *)list, attr ) ;
			}
			else
			{
				return limit - 1 ;
			}
			return subIndex ;
		}
	}
	lcd_outdezAtt( 21*FW, y, subIndex, attr ) ;
	return subIndex ;
}

uint8_t editMultiProtocol( uint8_t index, uint8_t edit )
{
	for ( uint32_t c = 0 ; c < 63 ; c += 1 )
	{
		if ( MultiMapping[c] == index )
		{
			index = c ;
			break ;
		}
	}
	if ( edit )
	{
 		CHECK_INCDEC_H_MODELVAR_0( index, 62 ) ;
	}
	return MultiMapping[index] ;
}

SKYMixData *mixAddress( uint32_t index )
{
	SKYMixData *md2 = &g_model.mixData[index] ;
#if EXTRA_SKYMIXERS
	if ( index >= MAX_SKYMIXERS )
	{
		md2 = &g_model.exmixData[index-MAX_SKYMIXERS] ;
	}
#endif
	return md2 ;
}

void putTxSwr( uint8_t x, uint8_t y, uint8_t attr )
{
	uint32_t index = 0 ;
	if ( FrskyTelemetryType == 1 )
	{
		index = 1 ;
	}
//#ifdef PCBX9D
//	else if ( g_model.xprotocol == PROTO_PXX )
//	{
//		index = 1 ;
//	}
//#endif
	lcd_putsAttIdx( x, y, PSTR(STR_TXEQ), index, attr ) ;
}

void putsAttIdxTelemItems( uint8_t x, uint8_t y, uint8_t index, uint8_t attr )
{
	if ( index == 4 )
	{
		putTxSwr( x, y, attr ) ;
	}
	else
	{
		lcd_putsAttIdx( x, y, PSTR(STR_TELEM_ITEMS), index, attr ) ;
	}
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

int16_t calc_scaler( uint8_t index, uint16_t *unit, uint8_t *num_decimals)
{
	int32_t value ;
	uint8_t lnest ;
	ScaleData *pscaler ;
	
	lnest = CalcScaleNest ;
	if ( lnest > 5 )
	{
		return 0 ;
	}
	CalcScaleNest = lnest + 1 ;
	// process
	pscaler = &g_model.Scalers[index] ;
	if ( pscaler->source )
	{
		value = getValue( pscaler->source - 1 ) ;
#ifdef FRSKY
		if ( ( pscaler->source == NUM_SKYXCHNRAW+1 ) || ( pscaler->source == NUM_SKYXCHNRAW+2 ) )
		{
			value = scale_telem_value( value, pscaler->source - NUM_SKYXCHNRAW-1, NULL ) ;
		}
#endif
	}
	else
	{
		value = 0 ;
	}
	CalcScaleNest = lnest ;
	if ( !pscaler->offsetLast )
	{
		value += pscaler->offset ;
	}
	value *= pscaler->mult+1 ;
	value /= pscaler->div+1 ;
	if ( pscaler->offsetLast )
	{
		value += pscaler->offset ;
	}
	if ( pscaler->neg )
	{
		value = -value ;
	}
	if ( unit )
	{
		*unit = pscaler->unit ;
	}
	if ( num_decimals )
	{
		*num_decimals = pscaler->precision ;
	}

	return value ;
}
									 
uint8_t telemItemValid( uint8_t index )
{
#ifdef FRSKY
	uint8_t x ;

	x = pgm_read_byte( &TelemValid[index] ) ;
	if ( x == 3 )
	{
		// A scaler
		uint8_t i = g_model.Scalers[index-TEL_ITEM_SC1].source - 1 ;
  	if( (i<CHOUT_BASE+NUM_SKYCHNOUT) || ( i >= EXTRA_POTS_START - 1 ) )
		{
			x = 0 ;
		}
		else
		{
			i -= CHOUT_BASE+NUM_SKYCHNOUT ;
			x = pgm_read_byte( &TelemValid[i] ) ;
			if ( x == 3 )
			{
				x = 0 ;
			}
		}
	}
	if ( x == 0 )
	{
		return 1 ;
	}
	if ( x == 1 )
	{
		if ( frskyStreaming )
		{
			return 1 ;
		}
	}
	if ( frskyUsrStreaming )
	{
		return 1 ;
	}
	return 0 ;	
#else
	return 1 ;
#endif
}

void voice_telem_item( int8_t index )
{
	int16_t value ;
	uint8_t spoken = 0 ;
	uint16_t unit = 0 ;
	uint8_t num_decimals = 0 ;
#ifdef FRSKY
//	uint8_t att = 0 ;
#endif

	value = get_telemetry_value( index ) ;
	if (telemItemValid( index ) == 0 )
	{
//#ifdef TELEMETRY_LOST
//		if ( frskyStreaming )
//		{
//#endif
		putSystemVoice( SV_NO_TELEM, V_NOTELEM ) ;
//#ifdef TELEMETRY_LOST
//		}
//#endif
		spoken = 1 ;
	}
	index = pgm_read_byte( &TelemIndex[index] ) ;

  switch (index)
	{
		case V_SC1 :
		case V_SC2 :
		case V_SC3 :
		case V_SC4 :
		case V_SC5 :
		case V_SC6 :
		case V_SC7 :
		case V_SC8 :
			value = calc_scaler( index-V_SC1, &unit, &num_decimals ) ;
			unit = UnitsVoice[unit] ;
		break ;
		
		case BATTERY:
		case FR_VOLTS :
		case FR_CELLS_TOT :
			unit = SV_VOLTS ;			
			num_decimals = 1 ;
		break ;

		case FR_CELL_MIN:
			if ( value > 445 )
			{
				value = 0 ;
			}
		case CELL_1:
		case CELL_2:
		case CELL_3:
		case CELL_4:
		case CELL_5:
		case CELL_6:
		case FR_RBOX_B1_V :
		case FR_RBOX_B2_V :
			unit = SV_VOLTS ;			
			num_decimals = 2 ;
		break ;
			
		case TIMER1 :
		case TIMER2 :
		{	
			div_t qr ;
			qr = div( value, 60 ) ;
			voice_numeric( qr.quot, 0, (qr.quot == 1) ? SV_MINUTE : SV_MINUTES ) ;
			value = qr.rem ;
			unit = SV_SECONDS ;			
		}
		break ;

    case V_RTC :
			voice_numeric( Time.hour, 0, 0 ) ;
			voice_numeric( Time.minute, 0, 0 ) ;
			spoken = 1 ;
		break ;
		 
		case V_GVAR1 :
		case V_GVAR2 :
		case V_GVAR3 :
		case V_GVAR4 :
		case V_GVAR5 :
		case V_GVAR6 :
		case V_GVAR7 :
			value = g_model.gvars[index-V_GVAR1].gvar ;
		break ;
		
#ifdef FRSKY
		case FR_A1_COPY:
    case FR_A2_COPY:
		{	
			uint8_t channel = index-FR_A1_COPY ;
			
			uint8_t ltype = g_model.frsky.channels[channel].type ;
			value = A1A2toScaledValue( channel, &num_decimals ) ;
			unit = SV_VOLTS ;			
			if (ltype == 3)
			{
				unit = SV_AMPS ;
			}
			else if (ltype == 1)
			{
				unit = 0 ;
				num_decimals = 0 ;
			}
		}
		break ;

    case FR_RXV :
			value = convertRxv( value ) ;			
			unit = SV_VOLTS ;			
			num_decimals = 1 ;
		break ;

		case FR_ALT_BARO:
      unit = SV_METRES ;
			if (g_model.FrSkyUsrProto == 1)  // WS How High
			{
      	if ( g_model.FrSkyImperial )
        	unit = SV_FEET ;
			}
      else
			{
				if ( g_model.FrSkyImperial )
      	{
	        // m to ft *105/32
  	      value = m_to_ft( value ) ;
    	    unit = SV_FEET ;
      	}
			}
			if ( value < 1000 )
			{
				num_decimals = 1 ;
			}
			else
			{
				value /= 10 ;
			}
		break ;

#ifdef MAVLINK 
// Extra data for Mavlink via FrSky		 
		case FR_WP_DIST:
  		if ( g_model.FrSkyImperial )
  		{
				value = m_to_ft(value) ;
			}
		break ;
// Extra data for Mavlink via FrSky
#endif

		case FR_RBOX_B1_A :
		case FR_RBOX_B2_A :
			num_decimals = 2 ;
      unit = SV_AMPS ;
		break ;
		
		case FR_RBOX_B1_CAP :
		case FR_RBOX_B2_CAP :
      unit = SV_MILAMP_H ;
		break ;
		
		case FR_CURRENT :
			num_decimals = 1 ;
      unit = SV_AMPS ;
		break ;

		case FR_AIRSPEED :
			num_decimals = 1 ;
      unit = SV_METRES ;
			if ( g_model.FrSkyImperial )
			{
       	// m to ft *105/32
       	value = m_to_ft( value ) ;
	      unit = 'f' ;
			}
		break ;
			 
		case FR_TEMP1:
		case FR_TEMP2:
			unit = SV_DEGREES ;			
  		if ( g_model.FrSkyImperial )
  		{
				value = c_to_f(value) ;
			}
		break ;

		case FR_WATT :
			unit = SV_WATTS ;
		break ;

		case FR_FUEL :
			unit = SV_PERCENT ;
		break ;

		case FR_VSPD :
			num_decimals = 1 ;
			value /= 10 ;
		break ;

#endif

	}

	if ( spoken == 0 )
	{
		voice_numeric( value, num_decimals, unit ) ;
	}
}


// This routine converts an 8 bit value for custom switch use
int16_t convertTelemConstant( int8_t channel, int8_t value)
{
  int16_t result;

	channel = TelemIndex[channel] ;
	result = value + 125 ;
	if ( ( channel <= V_GVAR7 ) && ( channel >= V_GVAR1 ) )
	{
		return value ;
	}

  switch (channel)
	{
    case V_RTC :
      result *= 12 ;
    break;
    case TIMER1 :
    case TIMER2 :
      result *= 10 ;
    break;
    case FR_ALT_BARO:
    case FR_GPS_ALT:
			if ( result > 63 )
			{
      	result *= 2 ;
      	result -= 64 ;
			}
			if ( result > 192 )
			{
      	result *= 2 ;
      	result -= 192 ;
			}
			if ( result > 448 )
			{
      	result *= 2 ;
      	result -= 488 ;
			}
			result *= 10 ;		// Allow for decimal place
      if ( g_model.FrSkyImperial )
      {
        // m to ft *105/32
        value = m_to_ft( result ) ;
      }
    break;
    case FR_RPM:
      result *= 100;
    break;
    case FR_TEMP1:
    case FR_TEMP2:
      result -= 30;
    break;
    case FR_A1_MAH:
    case FR_A2_MAH:
		case FR_AMP_MAH :
		case FR_RBOX_B1_CAP :
		case FR_RBOX_B2_CAP :
      result *= 50;
    break;

		case CELL_1:
		case CELL_2:
		case CELL_3:
		case CELL_4:
		case CELL_5:
		case CELL_6:
		case FR_CELL_MIN:
      result *= 2;
		break ;
		case FR_CELLS_TOT :
		case FR_VOLTS :
      result *= 2;
		break ;
		case FR_RBOX_B1_V :
		case FR_RBOX_B2_V :
      result *= 4 ;
		break ;
		case FR_RBOX_B1_A :
		case FR_RBOX_B2_A :
      result *= 20;
		break ;
    case FR_WATT:
      result *= 8 ;
    break;
		case FR_VSPD :
			result = value * 10 ;
		break ;
  }
  return result;
}


int16_t get_telemetry_value( int8_t channel )
{
#ifndef TELEMETRY_LOST
	if (telemItemValid( channel ) == 0 )
	{
		return 0 ;
	}
#endif	
	channel = pgm_read_byte( &TelemIndex[channel] ) ;
  if ( channel == TMOK )
	{
		return TmOK ;
	}
  if ( channel == V_RTC )
	{
		return Time.hour * 60 + Time.minute ;
	}
  if ( channel == FR_WATT )
	{
		return (uint32_t)FrskyHubData[FR_VOLTS] * (uint32_t)FrskyHubData[FR_CURRENT] / (uint32_t)100 ;
	}	 
	if ( channel <= CELL_12 )	// A single cell
	{
		return FrskyVolts[channel - CELL_1] ;
	}
	if ( channel < -10 )	// A Scaler
	{
		return calc_scaler(channel-V_SC1, 0, 0 ) ;
	}
	if ( channel < -3 )	// A GVAR
	{
		return g_model.gvars[channel-V_GVAR1].gvar ;
	}
  switch (channel)
	{
    case TIMER1 :
    case TIMER2 :
    return s_timer[channel+2].s_timerVal ;
    
    case BATTERY :
    return g_vbat100mV ;

#ifdef FRSKY
    case FR_ALT_BARO :
		return FrskyHubData[channel] + AltOffset ;

    case FR_CELL_MIN :
		return FrskyHubData[channel] ;
		 
		default :
		return FrskyHubData[channel] ;
#endif
  }
}

void displayTimer( uint8_t x, uint8_t y, uint8_t timer, uint8_t att )
{
	struct t_timer *tptr = &s_timer[timer] ;
  att |= (tptr->s_timerState==TMR_BEEPING ? BLINK : 0);
  putsTime( x, y, tptr->s_timerVal, att, att ) ;
}

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
		default : s = STR_MAV_FM_X ; break ;
	}
	return (char *) s ;
}

// Styles
#define TELEM_LABEL				0x01
#define TELEM_UNIT    		0x02
#define TELEM_UNIT_LEFT		0x04
#define TELEM_VALUE_RIGHT	0x08
#define TELEM_NOTIME_UNIT	0x10
#define TELEM_ARDUX_NAME	0x20
#define TELEM_CONSTANT		0x80

uint8_t putsTelemetryChannel(uint8_t x, uint8_t y, int8_t channel, int16_t val, uint8_t att, uint8_t style)
{
	uint16_t unit = ' ' ;
	uint8_t xbase = x ;
	uint8_t fieldW = FW ;
	uint8_t displayed = 0 ;

	if ( style & TELEM_LABEL )
	{
		uint8_t displayed = 0 ;
		int8_t index = TelemIndex[channel] ;

		if ( (index >= V_SC1) && (index < V_SC1 + NUM_SCALERS) )
		{
			uint8_t text[6] ;
			index -= V_SC1 ;
			uint8_t *p = &g_model.Scalers[index].name[0] ;
			text[0] = *p++ ;

			if ( text[0] && (text[0] != ' ') )
			{
				text[1] = *p++ ;
				text[2] = *p++ ;
				text[3] = *p ;
				text[4] = 0 ;
				lcd_putsAtt( x, y, (const char *)text, 0 ) ;
				displayed = 1 ;
			}
		}
		if ( displayed == 0 )
		{
  		putsAttIdxTelemItems( x, y, channel+1, 0 ) ;
		}
		x += 4*FW ;
		if ( att & DBLSIZE )
		{
			x += 4 ;
			y -= FH ;												
			fieldW += FW ;
		}
	}

	if (style & TELEM_VALUE_RIGHT)
	{
		att &= ~LEFT ;
	}
	channel = TelemIndex[channel] ;
  switch (channel)
	{
		case V_SC1 :
		case V_SC2 :
		case V_SC3 :
		case V_SC4 :
		case V_SC5 :
		case V_SC6 :
		case V_SC7 :
		case V_SC8 :
		{
			int16_t cvalue ;
			uint8_t precision ;
			cvalue = calc_scaler( channel-V_SC1, &unit, &precision ) ;
			if ( precision == 1 )
			{
				att |= PREC1 ;
			}
			else if ( precision == 2 )
			{
				att |= PREC2 ;
			}
			// Sort units here
			unit = UnitsText[unit] ;
			if ( (style & TELEM_CONSTANT) == 0)
			{
				val = cvalue ;
			}
		}	
		break ;

    case V_RTC :
			style |= TELEM_NOTIME_UNIT ;
		case TIMER1 :
    case TIMER2 :
			if ( (att & DBLSIZE) == 0 )
			{
				x -= 4 ;
			}
			if ( style & TELEM_LABEL )
			{
				x += FW+4 ;
			}
//			att &= DBLSIZE ;
	  	if (att & DBLSIZE)
  		{
				if ( (att & CONDENSED) )
				{
					x += 2 ;
				}
			}		
      putsTime(x-FW-3+5, y, val, att, att) ;
			displayed = 1 ;
			if ( !(style & TELEM_NOTIME_UNIT) )
			{
    		unit = channel + 2 + '1';
			}
			xbase -= FW ;
    break ;
    
		case FR_A1_COPY:
    case FR_A2_COPY:
      channel -= FR_A1_COPY ;
      // no break
      // A1 and A2
			unit = putsTelemValue( (style & TELEM_VALUE_RIGHT) ? xbase+62 : x-fieldW, y, val, channel, att|NO_UNIT/*|blink*/ ) ;
			displayed = 1 ;
    break ;

		case FR_RXV:
  		unit = 'v' ;
			att |= PREC1 ;
			val = convertRxv( val ) ;
    break ;

    case FR_TEMP1:
			if ( ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) ) && (style & TELEM_ARDUX_NAME) )
			{
				char *s ;
				s = arduFlightMode( FrskyHubData[FR_TEMP1] ) ;
				lcd_putsAtt( x, y, s, frskyUsrStreaming ? 0 : BLINK ) ;
				displayed = 1 ;
				break ;
			}
    case FR_TEMP2:
			unit = 'C' ;
  		if ( g_model.FrSkyImperial )
  		{
				val = c_to_f(val) ;
  		  unit = 'F' ;
				x -= fieldW ;
  		}
    break;
    
		case FR_AIRSPEED :
			att |= PREC1 ;
			unit = 'm' ;
			if ( g_model.FrSkyImperial )
			{
       	// m to ft *105/32
       	val = m_to_ft( val ) ;
	      unit = 'f' ;
			}
    break;

		case FR_ALT_BARO:
    case FR_GPS_ALT:
      unit = 'm' ;
			if ( g_model.FrSkyImperial )
			{
				if (g_model.FrSkyUsrProto == 0)  // Not WS How High
				{
        	// m to ft *105/32
        	val = m_to_ft( val ) ;
				}
        unit = 'f' ;
			}
			if ( val < 1000 )
			{
				att |= PREC1 ;
			}
			else
			{
				val /= 10 ;
			}
    break;

		case FR_CURRENT :
			att |= PREC1 ;
      unit = 'A' ;
		break ;

		case FR_CELL_MIN:
			if ( val > 445 )
			{
				val = 0 ;
			}
		case CELL_1:
		case CELL_2:
		case CELL_3:
		case CELL_4:
		case CELL_5:
		case CELL_6:
		case FR_RBOX_B1_V :
		case FR_RBOX_B2_V :
			att |= PREC2 ;
      unit = 'v' ;
		break ;
		case FR_RBOX_B1_A :
		case FR_RBOX_B2_A :
			att |= PREC2 ;
      unit = 'A' ;
		break ;
		case FR_RBOX_B1_CAP :
		case FR_RBOX_B2_CAP :
      unit = 'm' ;
		break ;
		case FR_CELLS_TOT :
		case FR_VOLTS :
			att |= PREC1 ;
      unit = 'v' ;
		break ;
		case BATTERY:
			att |= PREC1 ;
      unit = 'v' ;
		break ;
		case FR_WATT :
      unit = 'w' ;
		break ;

		case FR_RPM :
  		lcd_outdezNAtt( (style & TELEM_VALUE_RIGHT) ? xbase+62 : x, y, (uint16_t)val, att, 5 ) ;
			displayed = 1 ;
		break ;

		case FR_VSPD :
			att |= PREC1 ;
			val /= 10 ;
		break ;

		case FR_FUEL :
      unit = '%' ;
		break ;

		default:
    break;
  }
	if ( !displayed )
	{
  	lcd_outdezAtt( (style & TELEM_VALUE_RIGHT) ? xbase+62 : x, y, val, att ) ;
	}
	if ( style & ( TELEM_UNIT | TELEM_UNIT_LEFT ) )
	{
		if ( style & TELEM_UNIT_LEFT )
		{
			x = xbase + FW + 4 ;			
			att &= ~DBLSIZE ;			 
		}
		else
		{
			x = Lcd_lastPos ;
		}
  	lcd_putcAtt( x, y, unit, att & ~CONDENSED );
	}
	return unit ;
}


// This is for the customisable telemetry display
//void display_custom_telemetry_value( uint8_t x, uint8_t y, int8_t index )
//{
//  lcd_putsAttIdx( x, y, PSTR(STR_TELEM_ITEMS), index+1, 0 ) ;
//	index = TelemIndex[index] ;
//	if ( index < 0 )
//	{ // A timer
//		putsTime( x+4*FW, y, s_timer[index+2].s_timerVal, 0, 0);
//	}
//	else
//	{ // A telemetry value



//	}
//}

//int16_t get_telem_value( uint8_t index )
//{
//	index = TelemIndex[index] ;
//	if ( index < 0 )
//	{ // A timer
//		return s_timer[index+2].s_timerVal ;
//	}
//	return FrskyHubData[index] ;	
//}

#if GVARS

void dispGvar( uint8_t x, uint8_t y, uint8_t gvar, uint8_t attr )
{
	lcd_putsAtt( x, y, PSTR(STR_GV), attr ) ;
	lcd_putcAtt( x+2*FW, y, gvar+'0', attr ) ;
}

int8_t gvarMenuItem(uint8_t x, uint8_t y, int8_t value, int8_t min, int8_t max, uint16_t attr, uint8_t event )
{
  bool invers = attr&(INVERS|BLINK);

  if ( attr & GVAR_100 )
	{
		attr &= ~GVAR_100 ;
		if (value >= 101)
		{
			dispGvar( x-3*FW, y, (uint8_t)value - 100, attr ) ;
	    if (invers) value = checkIncDec16( (uint8_t)value, 101, 107, EE_MODEL);
		}
		else
		{
  	  lcd_outdezAtt(x, y, value, attr ) ;
    	if (invers) CHECK_INCDEC_H_MODELVAR( value, min, max);
		}
		if (invers)
		{
			if ( Tevent == EVT_TOGGLE_GVAR )
			{
  	  	value = ((value >= 101) ? g_model.gvars[(uint8_t)value-101].gvar : 101);
		    eeDirty(EE_MODEL) ;
			}
		}
		return value ;
	}
	if (value >= 126 || value <= -126)
	{
		dispGvar( x-3*FW, y, (uint8_t)value - 125, attr ) ;
    if (invers) value = checkIncDec16( (uint8_t)value, 126, 130, EE_MODEL);
  }
  else
	{
    lcd_outdezAtt(x, y, value, attr ) ;
    if (invers) CHECK_INCDEC_H_MODELVAR( value, min, max);
  }
  
	if (invers)
	{
		if ( Tevent == EVT_TOGGLE_GVAR )
		{
    	value = ((value >= 126 || value <= -126) ? g_model.gvars[(uint8_t)value-126].gvar : 126);
	    eeDirty(EE_MODEL) ;
		}
	}
	return value;
}

//void displayGVar(uint8_t x, uint8_t y, int8_t value)
//{
//  if (value >= 126 || value <= -126)
//	{
//		lcd_puts_P( x-3*FW, y, PSTR("GV") ) ;
//		lcd_putc( x-FW, y, (uint8_t)value - 125+'0') ;
//  }
//  else
//	{
//    lcd_outdez(x, y, value ) ;
//  }
//}
#endif

/*
#define GET_DR_STATE(x) (!getSwitch(g_model.expoData[x].drSw1,0) ?   \
    DR_HIGH :                                  \
    !getSwitch(g_model.expoData[x].drSw2,0)?   \
    DR_MID : DR_LOW);
*/

uint8_t get_dr_state(uint8_t x)
{
	ExpoData *ped ;

	ped = &g_model.expoData[x] ;
	
 	return (!getSwitch00( ped->drSw1) ? DR_HIGH :
    !getSwitch00( ped->drSw2) ? DR_MID : DR_LOW) ;
}

//#if NUM_EXTRA_POTS
static uint8_t mapPots( uint8_t value )
{
	if ( value >= EXTRA_POTS_POSITION )
	{
		if ( value >= EXTRA_POTS_START )
		{
			value -= ( EXTRA_POTS_START - EXTRA_POTS_POSITION ) ;
		}
		else
		{
			value += NumExtraPots ;
		}
	}
	return value ;
}

uint8_t unmapPots( uint8_t value )
{
	if ( value >= EXTRA_POTS_POSITION )
	{
		if ( value < EXTRA_POTS_POSITION + NumExtraPots )
		{
			value += EXTRA_POTS_START - EXTRA_POTS_POSITION ;
		}
		else
		{
			value -= NumExtraPots ;
		}
	}
	return value ;
}

//#endif

#define DO_SQUARE(xx,yy,ww)         \
{uint8_t x,y,w ; x = xx; y = yy; w = ww ; \
    lcd_vline(x-w/2,y-w/2,w);  \
    lcd_hline(x-w/2,y+w/2,w);  \
    lcd_vline(x+w/2,y-w/2,w);  \
    lcd_hline(x-w/2,y-w/2,w);}

#define DO_CROSS(xx,yy,ww)          \
    lcd_vline(xx,yy-ww/2,ww);  \
    lcd_hline(xx-ww/2,yy,ww);  \

#define V_BAR(xx,yy,ll)       \
    lcd_vline(xx-1,yy-ll,ll); \
    lcd_vline(xx  ,yy-ll,ll); \
    lcd_vline(xx+1,yy-ll,ll);

#define NO_HI_LEN 25

#define WCHART 32
#define X0     (128-WCHART-2 - 2 )
#define Y0     32
#define WCHARTl 32l
#define X0l     (128l-WCHARTl-2)
#define Y0l     32l
#define RESX    (1<<10) // 1024
#define RESXu   1024u
#define RESXul  1024ul
#define RESXl   1024l
#define RESKul  100ul
#define RESX_PLUS_TRIM (RESX+128)


void menuProcSetup(uint8_t event) ;
void menuProcSetup1(uint8_t event) ;
void menuProcSetup2(uint8_t event) ;
void menuProcDiagAna(uint8_t event) ;
void menuProcDiagKeys(uint8_t event) ;
void menuProcDiagCalib(uint8_t event) ;
void menuProcSDstat(uint8_t event) ;
void menuProcBoot(uint8_t event) ;
#ifdef PCB9XT
void menuProcSlave(uint8_t event) ;
#endif
void menuProcTrainer(uint8_t event) ;
void menuProcDiagVers(uint8_t event) ;
void menuProcProtocol(uint8_t event) ;
void menuModelPhases(uint8_t event) ;
void menuProcModelSelect(uint8_t event) ;
void menuProcMix(uint8_t event) ;
void menuProcExpoAll(uint8_t event);
void menuProcLimits(uint8_t event);
void menuProcCurve(uint8_t event);
void menuProcSwitches(uint8_t event);
void menuProcSafetySwitches(uint8_t event);
void menuProcTemplates(uint8_t event);
void menuProcTelemetry(uint8_t event) ;
void menuProcDate(uint8_t event) ;
void menuProcRestore(uint8_t event) ;
void menuProcIndex(uint8_t event) ;
void menuProcModelIndex(uint8_t event) ;

#if defined(PCBSKY) || defined(PCB9XT)
void menuProcBt(uint8_t event) ;
void menuProcBtScan(uint8_t event) ;
void menuProcBtConfigure(uint8_t event) ;
void menuProcBtAddressList(uint8_t event) ;
#endif

void menuProcBattery(uint8_t event) ;
void menuProcStatistic(uint8_t event) ;
void menuProcMusic(uint8_t event) ;
void menuProcMusicList(uint8_t event) ;
//#ifdef PCBX9D
//#ifdef REVPLUS
//void menuProcImage(uint8_t event) ;
//#endif
//#endif
void menuProcStatistic2(uint8_t event) ;
void menuProcDsmDdiag(uint8_t event) ;
void menuProcAlpha(uint8_t event) ;
void menuProcTrainDdiag(uint8_t event) ;
//#ifdef PCB9XT
void menuDebug(uint8_t event) ;
//#endif

void menuProcVoiceAlarm(uint8_t event) ;

uint8_t getMixerCount( void ) ;
bool reachMixerCountLimit( void ) ;


enum MainViews
{
  e_outputValues,
  e_outputBars,
  e_inputs1,
  e_timer2,
#ifdef FRSKY
  e_telemetry,
#endif
  MAX_VIEWS
};

int16_t calibratedStick[NUMBER_ANALOG+NUM_POSSIBLE_EXTRA_POTS];

int16_t ex_chans[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS];          // Outputs + intermidiates
uint8_t s_pgOfs;
uint8_t s_editMode;
uint8_t s_editing;
uint8_t s_noHi;
uint8_t scroll_disabled;
int8_t scrollLR;
int8_t scrollUD;
uint8_t InverseBlink ;
uint8_t EditType ;
uint8_t EditColumns ;

int16_t g_chans512[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS];

#include "sticks.lbm"

enum EnumTabStat
{
	e_battery,
	e_stat1,
	e_stat2,
	e_music,
	e_music1,
	e_dsm,
	e_traindiag,
//#ifdef PCB9XT
	e_debug,
//#endif
  e_Setup2,
  e_Setup3,
//#ifdef PCBX9D
//#ifdef REVPLUS
//	e_Image,
//#endif
//#endif
#if defined(PCBSKY) || defined(PCB9XT)
	e_bluetooth,
#endif
#ifdef PCB9XT
	e_Slave,
#endif
  e_Boot
} ;

MenuFuncP menuTabStat[] =
{
	menuProcBattery,
	menuProcStatistic,
	menuProcStatistic2,
	menuProcMusic,
	menuProcMusicList,
	menuProcDsmDdiag,
	menuProcTrainDdiag,
//#ifdef PCB9XT
	menuDebug,
//#endif
  menuProcSetup2,
	menuProcSDstat,
//#ifdef PCBX9D
//#ifdef REVPLUS
//	menuProcImage,
//#endif
//#endif
#if defined(PCBSKY) || defined(PCB9XT)
	menuProcBt,
#endif
#ifdef PCB9XT
	menuProcSlave,
#endif
	menuProcBoot
} ;


// Rotary Encoder states
#define RE_IDLE				0
#define RE_MENU_LR		1
#define RE_MENU_UD		2
#define RE_ITEMLR			3
#define RE_ITEM_EDIT	4

uint8_t Re_state ;

void displayNext()
{
	lcd_puts_P( 17*FW, 7*FH, XPSTR("[->]") ) ;
}

const char *get_curve_string()
{
    return PSTR(CURV_STR)	;
}	

void menu_lcd_onoff( uint8_t x,uint8_t y, uint8_t value, uint8_t mode )
{
  if (value)
	{
    lcd_putc(x+1, y, '\202');
	}
	lcd_hbar( x, y, 7, 7, mode ? 100 : 0 ) ;
}

//void menu_lcd_HYPHINV( uint8_t x,uint8_t y, uint8_t value, uint8_t mode )
//{
//    lcd_putsAttIdx( x, y, PSTR(STR_HYPH_INV),value,mode ? InverseBlink:0) ;
//}

//void MState2::check_simple(uint8_t event, uint8_t curr, MenuFuncP *menuTab, uint8_t menuTabSize, uint8_t maxrow)
//{
//    check(event, curr, menuTab, menuTabSize, 0, 0, maxrow);
//}

//void MState2::check_submenu_simple(uint8_t event, uint8_t maxrow)
//{
//    check_simple(event, 0, 0, 0, maxrow);
//}

#define EDIT_DR_SWITCH_EDIT		0x01
#define EDIT_DR_SWITCH_MOMENT	0x02

int8_t edit_dr_switch( uint8_t x, uint8_t y, int8_t drswitch, uint8_t attr, uint8_t flags, uint8_t event )
{
	if ( flags & EDIT_DR_SWITCH_MOMENT )
	{
		putsMomentDrSwitches( x, y, drswitch, attr ) ;
	}
	else
	{
		putsDrSwitches( x,  y, drswitch, attr ) ;
	}
	if(flags & EDIT_DR_SWITCH_EDIT)
	{
		if ( flags & EDIT_DR_SWITCH_MOMENT )
		{
#if defined(PCBSKY) || defined(PCB9XT)
			int16_t value = drswitch ;
			if ( value < -HSW_MAX )
			{
				value += 256 ;
			}
			if ( value > HSW_MAX )
			{
				value = switchUnMap( value - HSW_MAX ) + MaxSwitchIndex - 1 ;
			}
			else
			{
				value = switchUnMap( drswitch ) ;
			}
      CHECK_INCDEC_H_MODELVAR( value ,(1-MaxSwitchIndex),(-2+2*MaxSwitchIndex));
			if ( value >= MaxSwitchIndex )
			{
				drswitch = switchMap( value - MaxSwitchIndex + 1 ) + HSW_MAX ;
			}
			else
			{
				drswitch = switchMap( value ) ;
			}
#endif
#if PCBX9D
			int16_t value = drswitch ;
			if ( value < -HSW_MAX )
			{
				value += 256 ;
			}
			if ( value > HSW_MAX )
			{
				value = switchUnMap( value - HSW_MAX ) + MaxSwitchIndex - 1 ;
			}
			else
			{
				value = switchUnMap( drswitch ) ;
			}
      CHECK_INCDEC_H_MODELVAR( value ,(1-MaxSwitchIndex),(-2+2*MaxSwitchIndex));
			if ( value >= MaxSwitchIndex )
			{
				drswitch = switchMap( value - MaxSwitchIndex + 1 ) + HSW_MAX ;
			}
			else
			{
				drswitch = switchMap( value ) ;
			}
//      CHECK_INCDEC_MODELSWITCH( event, drswitch ,(1-MAX_SKYDRSWITCH),(-2+2*MAX_SKYDRSWITCH));
#endif
		}
		else
		{
			CHECK_INCDEC_MODELSWITCH( drswitch, -MaxSwitchIndex, MaxSwitchIndex) ;
		}
	}
	return drswitch ;
}

void alphaEditName( uint8_t x, uint8_t y, uint8_t *name, uint8_t len, uint8_t type, uint8_t *heading )
{
	if ( ( type & ALPHA_NO_NAME ) == 0 )
	{
		lcd_puts_Pleft( y, PSTR( STR_NAME ) ) ;
	}
	lcd_putsnAtt( x, y, (const char *)name, len, 0 ) ;
	if ( type & ~ALPHA_NO_NAME )
	{
		lcd_rect( x-1, y-1, len*FW+2, 9 ) ;
		if ( Tevent==EVT_KEY_BREAK(KEY_MENU) || Tevent == EVT_KEY_BREAK(BTN_RE)  )
		{
			Xmem.Alpha.AlphaLength = len ;
			Xmem.Alpha.PalphaText = name ;
			Xmem.Alpha.PalphaHeading = heading ;
			s_editMode = 0 ;
    	killEvents(Tevent) ;
			Tevent = 0 ;
			pushMenu(menuProcAlpha) ;
		}
	}
}

void lcd_xlabel_decimal( uint8_t x, uint8_t y, uint16_t value, uint8_t attr, const char *s )
{
  lcd_outdezAtt( x, y, value, attr ) ;
	lcd_puts_Pleft( y, s ) ;
}

uint8_t checkIndexed( uint8_t y, const char *s, uint8_t value, uint8_t edit )
{
	uint8_t x ;
	uint8_t max ;
	if ( s )
	{
		x = *s++ ;
		max = *s++ ;
		lcd_putsAttIdx( x, y, s, value, edit ? InverseBlink: 0 ) ;
	}
	else
	{
		x = PARAM_OFS ;
		max = 1 ;
		menu_lcd_onoff( x, y, value, edit ) ;
	}
	
	if(edit)
	{
		if ( ( EditColumns == 0 ) || ( s_editMode ) )
		{
			value = checkIncDec( value, 0, max, EditType ) ;
		}
	}
	return value ;
}

uint8_t hyphinvMenuItem( uint8_t value, uint8_t y, uint8_t condition )
{
	return checkIndexed( y, PSTR( STR_HYPH_INV), value, condition ) ;
}

// Extra data for Mavlink via FrSky
//#ifdef MAVLINK
////const int8_t COS[] = { 100,97,87,71,50,26,0,-26,-50,-71,-87,-97,-100,-97,-87,-71,-50,-26,0,26,50,71,87,97,100 };// Extra data for Mavlink via FrSky
//const int8_t SIN[] = { 0,26,50,71,87,97,100,97,87,71,50,26,0,-26,-50,-71,-87,-97,-100,-97,-87,-71,-50,-26,0,26,50,71,87,97,100   };// Extra data for Mavlink via FrSky

//uint16_t normalize( int16_t alpha )
//{
//   if ( alpha > 360 ) alpha -= 360 ;
//   if ( alpha <   0 ) alpha += 360 ;
//   alpha /= 15 ;
//   return alpha ;
//}

//int8_t rcos100( int16_t alpha )
//{
//   alpha = normalize( alpha );
//   return COS[alpha];
//   return SIN[alpha+6];
//}

//int8_t rsin100( int16_t alpha )
//{
//   alpha = normalize( alpha );
//   return SIN[alpha];
//}

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

//#endif
// Extra data for Mavlink via FrSky


void DisplayScreenIndex(uint8_t index, uint8_t count, uint8_t attr)
{
	uint8_t x ;
	if ( RotaryState == ROTARY_MENU_LR )
	{
		attr = BLINK ;
	}
	lcd_outdezAtt(127,0,count,attr);
	x = 1+128-FW*(count>9 ? 3 : 2) ;
  lcd_putcAtt(x,0,'/',attr);
  lcd_outdezAtt(x-1,0,index+1,attr);
//		lcd_putc( x-12, 0, RotaryState + '0' ) ;
}


uint8_t g_posHorz ;
uint8_t M_longMenuTimer ;
uint8_t M_lastVerticalPosition ;

//#define MAXCOL(row) (horTab ? pgm_read_byte(horTab+min(row, horTabMax)) : (const uint8_t)0)


uint8_t MState2::check_columns( uint8_t event, uint8_t maxrow)
{
	return check( event, 0, NULL, 0, &Columns, 0, maxrow ) ;
}

uint8_t MAXCOL( uint8_t row, const uint8_t *horTab, uint8_t horTabMax)
{
	return (horTab ? pgm_read_byte(horTab+min(row, horTabMax)) : (const uint8_t)0) ;
}

#define INC(val,max) if(val<max) {val++;} else {val=0;}
#define DEC(val,max) if(val>0  ) {val--;} else {val=max;}

uint8_t MState2::check(uint8_t event, uint8_t curr, MenuFuncP *menuTab, uint8_t menuTabSize, prog_uint8_t *horTab, uint8_t horTabMax, uint8_t maxrow)
{
    //    scrollLR = 0;
    //    scrollUD = 0;
	uint8_t l_posHorz ;
	l_posHorz = g_posHorz ;
	
	int16_t c4, c5 ;
	struct t_p1 *ptrp1 ;

    //check pot 2 - if changed -> scroll menu
    //check pot 3 if changed -> cursor down/up
    //we do this in these brackets to prevent it from happening in the main screen
	c4 = calibratedStick[4] ;		// Read only once
	c5 = calibratedStick[5] ;		// Read only once
    
	ptrp1 = &P1values ;
		
  scrollLR = ( ptrp1->p2valprev-c4)/SCROLL_TH;
  scrollUD = ( ptrp1->p3valprev-c5)/SCROLL_TH;

  if(scrollLR) ptrp1->p2valprev = c4;
  if(scrollUD) ptrp1->p3valprev = c5;

  if(scroll_disabled || g_eeGeneral.disablePotScroll)
  {
    scrollLR = 0;
    scrollUD = 0;
    scroll_disabled = 0;
  }

	M_lastVerticalPosition = m_posVert ;
  
//  if(scrollLR || scrollUD || ptrp1->p1valdiff) backlightKey() ; // on keypress turn the light on
	if(scrollLR || scrollUD || ptrp1->p1valdiff)
	{
		g_LightOffCounter = g_eeGeneral.lightAutoOff*500; // on keypress turn the light on 5*100
		InacCounter = 0 ;
	}

	if (menuTab)
	{
    uint8_t attr = m_posVert==0 ? INVERS : 0;


    if(m_posVert==0)
    {
			if ( RotaryState == ROTARY_MENU_LR )
			{
				if ( Rotary_diff > 0 )
				{
   				scrollLR = -1;
				}
				else if ( Rotary_diff < 0 )
				{
   				scrollLR = 1;
				}
				Rotary_diff = 0 ;
        if(event==EVT_KEY_BREAK(BTN_RE))
				{
					RotaryState = ROTARY_MENU_UD ;
		      event = 0 ;
		      Tevent = 0 ;
				}
			}
			else if ( RotaryState == ROTARY_MENU_UD )
			{
        if(event==EVT_KEY_BREAK(BTN_RE))
				{
					RotaryState = ROTARY_MENU_LR ;
		      event = 0 ;
		      Tevent = 0 ;
				}
			}
      if( scrollLR && !s_editMode)
      {
        int8_t cc = curr - scrollLR ;
        if(cc<1) cc = menuTabSize-1 ;
        if(cc>(menuTabSize-1)) cc = 0 ;

        if(cc!=curr)
        {
            //                    if(((MenuFuncP)pgm_read_adr(&menuTab[cc])) == menuProcDiagCalib)
            //                        chainMenu(menuProcDiagAna);
            //                    else
          chainMenu((MenuFuncP)pgm_read_adr(&menuTab[cc]));
					return event ;
        }

        scrollLR = 0;
      }

      if(event==EVT_KEY_FIRST(KEY_LEFT))
      {
        uint8_t index ;
        if(curr>0)
          index = curr ;
          //                    chainMenu((MenuFuncP)pgm_read_adr(&menuTab[curr-1]));
        else
          index = menuTabSize ;

       	chainMenu((MenuFuncP)pgm_read_adr(&menuTab[index-1]));
				return event ;
      }

      if(event==EVT_KEY_FIRST(KEY_RIGHT))
      {
        uint8_t index ;
        if(curr < (menuTabSize-1))
          index = curr +1 ;
         //                    chainMenu((MenuFuncP)pgm_read_adr(&menuTab[curr+1]));
        else
          index = 0 ;
        chainMenu((MenuFuncP)pgm_read_adr(&menuTab[index]));
				return event ;
      }
    }
		else
		{
			if ( s_editMode == 0 ) RotaryState = ROTARY_MENU_UD ;
		}

    DisplayScreenIndex(curr, menuTabSize, attr);
  }
	else if ( RotaryState == ROTARY_MENU_LR )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
  uint8_t maxcol = MAXCOL(m_posVert, horTab, horTabMax);
		
 if ( maxrow != 0xFF )
 {
	if ( RotaryState == ROTARY_MENU_UD )
	{
		static uint8_t lateUp = 0 ;
		if ( lateUp )
		{
			lateUp = 0 ;
			l_posHorz = MAXCOL(m_posVert, horTab, horTabMax) ;
		}
		if ( Rotary_diff > 0 )
		{
    	INC(l_posHorz,maxcol) ;
			if ( l_posHorz == 0 )
			{
				INC(m_posVert,maxrow);
			}
		}
		else if ( Rotary_diff < 0 )
		{
			if ( l_posHorz == 0 )
			{
      	DEC(m_posVert,maxrow);
				lateUp = 1 ;
				l_posHorz = 0 ;
			}
			else
			{
      	DEC(l_posHorz,maxcol) ;
			}
		}
		Rotary_diff = 0 ;
    if(event==EVT_KEY_BREAK(BTN_RE))
		{
			RotaryState = ROTARY_VALUE ;
		}
	}
	else if ( RotaryState == ROTARY_VALUE )
	{
		if ( (event==EVT_KEY_BREAK(BTN_RE)) || ( s_editMode == 0 ) )
		{
			RotaryState = ROTARY_MENU_UD ;
		}
	}

	{
		uint8_t timer = M_longMenuTimer ;
		if ( menuPressed() )
		{
			if ( timer < 255 )
			{
				timer += 1 ;
			}
		}
		else
		{
			timer = 0 ;
		}
		if ( timer > 60 )
		{
			s_editMode = 1 ;
			RotaryState = ROTARY_VALUE ;
		}
		M_longMenuTimer = timer ;
	}

 } 

    //        scrollLR = 0;

  maxcol = MAXCOL(m_posVert, horTab, horTabMax) ;
	EditColumns = maxcol ;
  l_posHorz = min(l_posHorz, maxcol ) ;

  if(!s_editMode)
  {
    if(scrollUD)
    {
      int8_t cc = m_posVert - scrollUD;
      if(cc<1) cc = 0;
      if(cc>=maxrow) cc = maxrow;
      m_posVert = cc;

      l_posHorz = min(l_posHorz, MAXCOL(m_posVert, horTab, horTabMax));
      BLINK_SYNC;

      scrollUD = 0;
    }

    if(m_posVert>0 && scrollLR)
    {
      int8_t cc = l_posHorz - scrollLR;
      if(cc<1) cc = 0;
      if(cc>=MAXCOL(m_posVert, horTab, horTabMax)) cc = MAXCOL(m_posVert, horTab, horTabMax);
      l_posHorz = cc;

      BLINK_SYNC;
      //            scrollLR = 0;
    }
  }

  switch(event)
  {
    case EVT_ENTRY:
        //if(m_posVert>maxrow)
        init() ;
        l_posHorz = 0 ;
				M_lastVerticalPosition = m_posVert ;
        s_editMode = false;
        //init();BLINK_SYNC;
    break;
    case EVT_KEY_BREAK(BTN_RE):
    case EVT_KEY_FIRST(KEY_MENU):
        if ( (m_posVert > 0) || (!menuTab) )
				{
	 				if ( maxrow != 0xFF )
					{
						s_editMode = !s_editMode;
						if ( s_editMode )
						{
							RotaryState = ROTARY_VALUE ;
						}
					}
				}
    break;
    case EVT_KEY_LONG(KEY_EXIT):
        s_editMode = false;
        popMenu(true); //return to uppermost, beeps itself
//        popMenu(false);
    break;
        //fallthrough
    case EVT_KEY_LONG(BTN_RE):
        killEvents(event);
    case EVT_KEY_BREAK(KEY_EXIT):
        if(s_editMode) {
            s_editMode = false;
            break;
        }
//        if(m_posVert==0 || !menuTab) {
//						RotaryState = ROTARY_MENU_LR ;
            popMenu();  //beeps itself
//        } else {
//            audioDefevent(AU_MENUS);
//            init();BLINK_SYNC;
//        }
    break;

    case EVT_KEY_REPT(KEY_RIGHT):  //inc
        if(l_posHorz==maxcol) break;
    case EVT_KEY_FIRST(KEY_RIGHT)://inc
        if(!horTab || s_editMode)break;
        INC(l_posHorz,maxcol);
				if ( maxcol )
				{
					event = 0 ;
					Tevent = 0 ;
				}
        BLINK_SYNC;
    break;

    case EVT_KEY_REPT(KEY_LEFT):  //dec
        if(l_posHorz==0) break;
    case EVT_KEY_FIRST(KEY_LEFT)://dec
        if(!horTab || s_editMode)break;
        DEC(l_posHorz,maxcol);
				if ( maxcol )
				{
					event = 0 ;
					Tevent = 0 ;
				}
        BLINK_SYNC;
    break;

    case EVT_KEY_REPT(KEY_DOWN):  //inc
        if(m_posVert==maxrow) break;
    case EVT_KEY_FIRST(KEY_DOWN): //inc
        if(s_editMode)break;
        INC(m_posVert,maxrow);
        l_posHorz = min(l_posHorz, MAXCOL(m_posVert, horTab, horTabMax));
        BLINK_SYNC;
    break;

    case EVT_KEY_REPT(KEY_UP):  //dec
        if(m_posVert==0) break;
    case EVT_KEY_FIRST(KEY_UP): //dec
        if(s_editMode)break;
        DEC(m_posVert,maxrow);
        l_posHorz = min(l_posHorz, MAXCOL(m_posVert, horTab, horTabMax));
        BLINK_SYNC;
    break;
  }
	s_editing = s_editMode || P1values.p1valdiff ;
//	InverseBlink = (!maxcol || s_editMode) ? BLINK : INVERS ;
	g_posHorz = l_posHorz ;
	InverseBlink = (s_editMode) ? BLINK : INVERS ;
	Columns = 0 ;
	return event ;
}

#define BOX_WIDTH     23
#define BAR_HEIGHT    (BOX_WIDTH-1l)
#define MARKER_WIDTH  5
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define BOX_LIMIT     (BOX_WIDTH-MARKER_WIDTH)
#define LBOX_CENTERX  (  SCREEN_WIDTH/4 + 10-1)
#define BOX_CENTERY  (SCREEN_HEIGHT-9-BOX_WIDTH/2-1)
#define RBOX_CENTERX  (3*SCREEN_WIDTH/4 - 10)
//#define RBOX_CENTERY  (SCREEN_HEIGHT-9-BOX_WIDTH/2)


void telltale( uint8_t centrex, int8_t xval, int8_t yval )
{
  DO_SQUARE( centrex, BOX_CENTERY, BOX_WIDTH ) ;
  DO_CROSS( centrex, BOX_CENTERY,3 ) ;
	DO_SQUARE( centrex +( xval/((2*RESX/16)/BOX_LIMIT)), BOX_CENTERY-( yval/((2*RESX/16)/BOX_LIMIT)), MARKER_WIDTH ) ;
}


void doMainScreenGrphics()
{
	int8_t *cs = phyStick ;
	
	telltale( LBOX_CENTERX, cs[0], cs[1] ) ;
	telltale( RBOX_CENTERX, cs[3], cs[2] ) ;
    
    // Optimization by Mike Blandford
    {
        uint8_t x, y, len ;			// declare temporary variables
#if defined(PCBSKY) || defined(PCB9XT)
				uint32_t maxy ;
				maxy = 7 ;
				x = -5 ;
				if ( g_eeGeneral.extraPotsSource[0] )
				{
					x = - 8 ;
					maxy = 8 ;
				}

        for( y = 4 ; y < maxy ; x += 5, y += 1 )
        {
            len = ((calibratedStick[y]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
            V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len )
        }
#endif
#ifdef PCBX9D
        for( x = -8, y = 4 ; y < 8 ; x += 5, y += 1 )
        {
        	uint8_t z = y - 1 ;
					if ( z == 6 )
					{
						z = 7 ;
					}
					else if ( z == 3 )
					{
						z = 6 ;
					}
          len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
          V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len )
        }
#endif
    }

}

extern int16_t calcExpo( uint8_t channel, int16_t value ) ;

static uint8_t s_curveChan ;
static uint8_t s_expoChan ;
static uint8_t SingleExpoChan ;

#define XD (X0-2)

#define GRAPH_FUNCTION_CURVE		0
#define GRAPH_FUNCTION_EXPO			1

void drawFunction( uint8_t xpos, uint8_t function )
{
  int8_t yv ;
  int8_t prev_yv = 127 ;
	for ( int8_t xv = -WCHART ; xv <= WCHART ; xv++ )
	{
		if ( function == GRAPH_FUNCTION_CURVE )
		{
    	yv = intpol(xv * RESX / WCHART, s_curveChan) * WCHART / RESX ;
		}
		else
		{
    	yv = calcExpo( s_expoChan, xv * RESX / WCHART) * WCHART / RESX ;
		}
    if (prev_yv == 127)
		{
			prev_yv = yv ;
		}
		uint8_t len = abs(yv-prev_yv) ;
    if (len <= 1)
		{
    	lcd_plot(xpos + xv, Y0 - yv) ;
		}
		else
		{
      uint8_t tmp = (prev_yv < yv ? 0 : len-1 ) ;
      lcd_vline(xpos+xv, Y0 - yv - tmp, len ) ;
		}	
		if ( yv )
		{
     	lcd_plot(xpos + xv, Y0 ) ;
		}
		prev_yv = yv ;
	}
	
}


void drawCurve( uint8_t offset )
{
  uint8_t cv9 = s_curveChan >= MAX_CURVE5 ;
 	int8_t *crv ;
	uint32_t points = cv9 ? 9 : 5 ;
 	crv = cv9 ? g_model.curves9[s_curveChan-MAX_CURVE5] : g_model.curves5[s_curveChan];
	if ( s_curveChan >= MAX_CURVE5 + MAX_CURVE9 )
	{
		if ( s_curveChan <= MAX_CURVE5 + MAX_CURVE9 + 1 )
		{ // The xy curve
			cv9 = 2 ;
			crv = ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 ) ? g_model.curvexy : g_model.curve2xy ;
		}
		else
		{
			crv = g_model.curve6 ;
			cv9 = 3 ;
			points = 6 ;
		}
	}
	lcd_vline(XD, Y0 - WCHART, WCHART * 2);
  
	plotType = PLOT_BLACK ;
	for(uint8_t i=0; i < points ; i++)
  {
    uint8_t xx ;
		if ( cv9 == 2 )
		{
    	xx = XD-1+crv[i+9]*WCHART/100 ;
		}
		else
		{
			if ( cv9 == 3 )
			{
				xx = XD-1-WCHART+i*WCHART*2/5 ;
			}
			else
			{
				xx = XD-1-WCHART+i*WCHART/(cv9 ? 4 : 2);
			}
		}
    uint8_t yy = Y0-crv[i]*WCHART/100;

    if(offset==i)
    {
			lcd_rect( xx-1, yy-2, 5, 5 ) ;
    }
    else
    
		{
			lcd_rect( xx, yy-1, 3, 3 ) ;
    }
  }

	drawFunction( XD, GRAPH_FUNCTION_CURVE ) ;
	
	plotType = PLOT_XOR ;
}

void menuProcCurveOne(uint8_t event)
{
  uint8_t cv9 = s_curveChan >= MAX_CURVE5 ;
	uint32_t points = cv9 ? 9 : 5 ;
	if ( s_curveChan >= MAX_CURVE5 + MAX_CURVE9 )
	{
		cv9 = 2 ;
	}
	if ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 + 2 )
	{
		points = 6 ;
		cv9 = 3 ;
	}
	static int8_t dfltCrv;

	TITLE(PSTR(STR_CURVE)) ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, ( cv9 == 2 ) ? 17 : points ) ;
		
		if ( event == EVT_ENTRY )
		{
			dfltCrv = 0 ;
		}
		lcd_outdezAtt(7*FW, 0, s_curveChan+1, INVERS);

	uint8_t  preset = points ;
	uint8_t  sub    = mstate2.m_posVert ;
	uint8_t blink = InverseBlink ;
	int8_t *crv ;
	if ( ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 ) || ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 + 1 ) )
	{ // The xy curve
		crv = ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 ) ? g_model.curvexy : g_model.curve2xy ;
		uint8_t i ;
		uint8_t j ;
		uint8_t k ;
		j = sub > 8 ? 2 : 0 ;
		k = sub & 1 ;
		sub >>= 1 ;
		if ( k == 0 )
		{
			sub += 9 ;
		}
//		if ( sub == 15 )
//		{
//			sub = 8 ;
//			k = 0 ;
//		}
//		else
//		{
//			k = sub & 1 ;
//			sub >>= 1 ;
//			if ( k )
//			{
//				sub += 10 ;
//			}
//		}
		for ( i = 0; i < 7; i++)
		{
  	  uint8_t y = i * FH + 8 ;
  	  uint8_t attr = (k==0) && (sub == j+i+9) ? blink : 0 ;
			lcd_outdezAtt(4 * FW, y, crv[j+i+9], attr);
  	  attr = (k==1) && (sub == j+i) ? blink : 0 ;
    	lcd_outdezAtt(8 * FW, y, crv[j+i], attr);
//			if ( i < 4 )
//			{
//		    attr = sub == i + 5 ? blink : 0;
//			}
		}
		int8_t min = -100 ;
		int8_t max = 100 ;
		if ( k == 0) // x value
		{
			if ( sub > 9 )
			{
				min = crv[sub-1] ;
			}
			if ( sub < 17 )
			{
				max = crv[sub+1] ;
			}
		}
		CHECK_INCDEC_H_MODELVAR( crv[sub], min, max ) ;
		if ( sub > 8 )
		{
			sub -= 9 ;
		}
// Draw the curve
		drawCurve( sub ) ;
	}
	else
	{
		crv = cv9 ? g_model.curves9[s_curveChan-MAX_CURVE5] : g_model.curves5[s_curveChan];
		if ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 + 2 )
		{
			crv = g_model.curve6 ;
		}

		for (uint8_t i = 0; i < 5; i++)
		{
  	  uint8_t y = i * FH + 16;
  	  uint8_t attr = sub == i ? blink : 0;
  	  lcd_outdezAtt(4 * FW, y, crv[i], attr);
			if( cv9 )
			{
				if ( points == 6 )
				{
					if ( i == 0 )
					{
			    	attr = sub == i + 5 ? blink : 0;
	  	  		lcd_outdezAtt(8 * FW, y, crv[i + 5], attr);
					}
				}
				else if ( i < 4 )
				{
			    attr = sub == i + 5 ? blink : 0;
  	  		lcd_outdezAtt(8 * FW, y, crv[i + 5], attr);
				}
			}
		}
		lcd_putsAtt( 2*FW, 7*FH,PSTR(STR_PRESET), (sub == preset) ? blink : 0);


		if( sub==preset) 
		{
			if ( s_editMode )
			{
				int8_t t ;
				Tevent = event ;
				t = dfltCrv ;
	  	  dfltCrv = checkIncDec( dfltCrv, -4, 4, 0);
	  	  if (dfltCrv != t)
				{
					uint8_t offset = cv9 ? 4 : 2 ;
					if ( points == 6 )
					{
						for (int8_t i = -5 ; i <= 5 ; i += 2 )
						{
						 	crv[(i+5)/2] = i*dfltCrv* 25 / 5 ;
						}
				  }
					else
					{
						for (int8_t i = -offset; i <= offset; i++) crv[i+offset] = i*dfltCrv* 25 / offset ;
					}
	  	    STORE_MODELVARS;        
	  	  }
			}
		} 
		else  /*if(sub>0)*/
		{
		 CHECK_INCDEC_H_MODELVAR( crv[sub], -100,100);
		}

// Draw the curve
		drawCurve( sub ) ;
	}
}



void menuProcCurve(uint8_t event)
{
	TITLE( PSTR(STR_CURVES) ) ;
	static MState2 mstate2 ;
	EditType = EE_MODEL ;

//	if (SubMenuFromIndex)
//	{
		mstate2.check_columns(event,1+MAX_CURVE5+MAX_CURVE9-1-1+1+1+1) ;
//		mstate2.check_simple(event,0,NULL,0,1+MAX_CURVE5+MAX_CURVE9-1) ;
//	}
//	else
//	{
//		mstate2.check_simple(event,e_Curve,menuTabModel,DIM(menuTabModel),1+MAX_CURVE5+MAX_CURVE9-1) ;
//	SIMPLE_MENU(PSTR(STR_CURVES), menuTabModel, e_Curve, 1+MAX_CURVE5+MAX_CURVE9);
//	}

  int8_t  sub    = mstate2.m_posVert ;

  uint8_t t_pgOfs = evalOffset(sub);

  switch (event)
	{
    case EVT_KEY_FIRST(KEY_RIGHT):
    case EVT_KEY_FIRST(KEY_MENU):
    case EVT_KEY_BREAK(BTN_RE):
  //    if (sub >= 0)
	//		{
        s_curveChan = sub;
     		killEvents(event);
				Tevent = 0 ;
        pushMenu(menuProcCurveOne);
  //    }
    break;
  }

    uint8_t y    = 1*FH;
    for (uint8_t i = 0; i < 7; i++)
		{
      uint8_t k = i + t_pgOfs;
      uint8_t attr = sub == k ? INVERS : 0;
      if(y>7*FH) break;
      lcd_putsAtt(   FW*0, y,PSTR(STR_CV),attr);
      lcd_outdezAtt( (k<9) ? FW*3-1 : FW*4-2, y,k+1 ,attr);

      y += FH ;
    }


//		if ( sub >= 0 )
//		{
  		s_curveChan = sub ;
			drawCurve( 100 ) ;
//		}
}

void setStickCenter() // copy state of 3 primary to subtrim
{
	uint8_t thisPhase ;
  int16_t zero_chans512_before[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS];
  int16_t zero_chans512_after[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS];

	thisPhase = getFlightPhase() ;
	CurrentPhase = thisPhase ;

  perOut(zero_chans512_before,NO_TRAINER | NO_INPUT | FADE_FIRST | FADE_LAST); // do output loop - zero input channels
  perOut(zero_chans512_after,NO_TRAINER | FADE_FIRST | FADE_LAST); // do output loop - zero input channels

  for(uint8_t i=0; i<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS; i++)
  {
		LimitData *pld = &g_model.limitData[i] ;
#if EXTRA_SKYCHANNELS
		if ( i >= NUM_SKYCHNOUT )
		{
			pld = &g_model.elimitData[i-NUM_SKYCHNOUT] ;
		}
#endif			
    int16_t v = pld->offset;
    v += pld->revert ?
                  (zero_chans512_before[i] - zero_chans512_after[i]) :
                  (zero_chans512_after[i] - zero_chans512_before[i]);
    pld->offset = max(min(v,1000),-1000); // make sure the offset doesn't go haywire
  }

  for(uint8_t i=0; i<4; i++)
	{
		if(!IS_THROTTLE(i))
		{
			int16_t original_trim = getTrimValue(thisPhase, i);
      for (uint8_t phase=0; phase<MAX_MODES; phase +=  1)
			{
        int16_t trim = getRawTrimValue(phase, i);
        if (trim <= TRIM_EXTENDED_MAX)
				{
          setTrimValue(phase, i, trim - original_trim);
				}
			}
		}
	}
  STORE_MODELVARS_TRIM;
  audioDefevent(AU_WARNING2);
}

void menuProcLimits(uint8_t event)
{
	TITLE(PSTR(STR_LIMITS));
	EditType = EE_MODEL ;
	static MState2 mstate2;
	event = mstate2.check_columns( event, NUM_SKYCHNOUT+2-1-1+EXTRA_SKYCHANNELS ) ;
	Columns = 3 ;

	uint8_t y = 0;
	uint8_t k = 0;
	uint8_t  sub = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
	uint8_t t_pgOfs ;

	t_pgOfs = evalOffset(sub);

	if ( sub < NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
	{
    lcd_outdez( 13*FW, 0, g_chans512[sub]/2 + 1500 ) ;
	}

	switch(event)
	{
    case EVT_KEY_LONG(KEY_MENU):
        if(sub>=0 && sub<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)
				{
            int16_t v = g_chans512[sub - t_pgOfs];
            LimitData *ld = &g_model.limitData[sub];
#if EXTRA_SKYCHANNELS
						if ( sub >= NUM_SKYCHNOUT )
						{
							ld = &g_model.elimitData[sub-NUM_SKYCHNOUT] ;
						}
#endif			
            switch (subSub) {
            case 0:
                ld->offset = (ld->revert) ? -v : v;
                STORE_MODELVARS;
//                eeWaitComplete() ;
                break;
            }
        }
				if(sub==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)
				{
  			  //last line available - add the "copy trim menu" line
  			  s_noHi = NO_HI_LEN;
  			  killEvents(event);
  				s_editMode = 0 ;
  			  setStickCenter(); //if highlighted and menu pressed - copy trims
				}
        break;
	}
//  lcd_puts_P( 4*FW, 1*FH,PSTR("subT min  max inv"));
  int8_t limit = (g_model.extendedLimits ? 125 : 100);
	for(uint8_t i=0; i<7; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    if(k==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS) break;
    LimitData *ld = &g_model.limitData[k];
#if EXTRA_SKYCHANNELS
		if ( k >= NUM_SKYCHNOUT )
		{
			ld = &g_model.elimitData[k-NUM_SKYCHNOUT] ;
		}
#endif			
    int16_t v = g_chans512[k] - ( (ld->revert) ? -ld->offset : ld->offset) ;
		
    char swVal = '-';  // '-', '<', '>'
		if( v >  50) swVal = ld->revert ? 127 : 126 ;// Switch to raw inputs?  - remove trim!
    if( v < -50) swVal = ld->revert ? 126 : 127 ;
    putsChn(0,y,k+1,0);
    lcd_putc(12*FW+FW/2, y, swVal ) ; //'<' : '>'
    
		for(uint8_t j=0; j<4;j++)
		{
        uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
				uint8_t active = (attr && (s_editMode || P1values.p1valdiff)) ;
				if ( active )
				{
					StickScrollAllowed = 3 ;		// Block L/R while editing
				}
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
        switch(j)
        {
        case 0:
            lcd_outdezAtt(  7*FW+3, y,  ld->offset, attr|PREC1);
            if(active)
						{
							StepSize = 50 ;
              ld->offset = checkIncDec16( ld->offset, -1000, 1000, EE_MODEL);
            }
            break;
        case 1:
					value += (int8_t)(ld->min-100) ;
					if ( value < -125 )
					{
						value = -125 ;						
					}
          lcd_outdezAtt(  12*FW, y, value,   attr);
            if(active) {
                ld->min -=  100;
		            CHECK_INCDEC_H_MODELVAR( ld->min, -limit,25);
                ld->min +=  100;
            }
            break;
        case 2:
					value += (int8_t)(ld->max+100) ;
					if ( value > 125 )
					{
						value = 125 ;						
					}
          lcd_outdezAtt( 17*FW, y, value,    attr);
					if ( t )
					{
						lcd_rect( 9*FW-4, y-1, 56, 9 ) ;
					}
            if(active) {
                ld->max +=  100;
                CHECK_INCDEC_H_MODELVAR( ld->max, -25,limit);
                ld->max -=  100;
            }
            break;
        case 3:
					ld->revert = hyphinvMenuItem( ld->revert, y, attr ) ;
//						menu_lcd_HYPHINV( 18*FW, y, ld->revert, attr ) ;
//            lcd_putsnAtt(   18*FW, y, PSTR("---INV")+ld->revert*3,3,attr);
//            if(active) {
//                CHECK_INCDEC_H_MODELVAR_0(event, ld->revert, 1);
//            }
            break;
        }
    }
}
	if(k==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)
	{
    //last line available - add the "copy trim menu" line
    uint8_t attr = (sub==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS) ? INVERS : 0;
    lcd_putsAtt(  3*FW,y,PSTR(STR_COPY_TRIM),s_noHi ? 0 : attr);
//    if(attr && event==EVT_KEY_LONG(KEY_MENU))
//		{
//      s_noHi = NO_HI_LEN;
//      killEvents(event);
//  	  s_editMode = 0 ;
//      setStickCenter(); //if highlighted and menu pressed - copy trims
//    }
	}
}



static uint8_t onoffItem( uint8_t value, uint8_t y, uint8_t condition )
{
	return checkIndexed( y, 0, value, condition ) ;
	
//	menu_lcd_onoff( PARAM_OFS, y, value, condition ) ;
//  if(condition) value = checkIncDec( event, value, 0, 1, flags ) ;
//  return value ;
}

static uint8_t offonItem( uint8_t value, uint8_t y, uint8_t condition )
{
	return 1-onoffItem( 1-value, y, condition ) ;
}


//static uint8_t onoffItem_m( uint8_t event, uint8_t value, uint8_t y, uint8_t condition )
//{
//	return onoffItem( value, y, condition ) ;
//}

//static uint8_t offonItem_m( uint8_t event, uint8_t value, uint8_t y, uint8_t condition )
//{
//	return 1-onoffItem_m( event, 1-value, y, condition ) ;
//}

static uint8_t onoffMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition )
{
    lcd_puts_Pleft(y, s);
		return onoffItem( value, y, condition ) ;
}

static uint8_t offonMenuItem( uint8_t value, uint8_t y, const char *s, uint8_t condition )
{
	return 1-onoffMenuItem( 1-value, y, s, condition ) ;
}

//static uint8_t onoffMenuItem_g( uint8_t value, uint8_t y, const char *s, uint8_t condition )
//{
//	return onoffMenuItem( value, y, s, condition ) ;
//}

//static uint8_t offonMenuItem_g( uint8_t event, uint8_t value, uint8_t y, const char *s, uint8_t condition )
//{
//	return 1-onoffMenuItem_g( event, 1-value, y, s, condition ) ;
//}


//static uint8_t onoffMenuItem_m( uint8_t event, uint8_t value, uint8_t y, const char *s, uint8_t condition )
//{
//    if(condition) CHECK_INCDEC_H_MODELVAR_0( event, value, 1 ) ;
//    menu_lcd_onoff( PARAM_OFS, y, value, condition ) ;
//    lcd_puts_Pleft(y, s);
//		return onoffMenuItem(value, y, s, condition ) ;
//}

//uint8_t hyphinvMenuItem_m( uint8_t event, uint8_t value, uint8_t y, const char *s, uint8_t condition )
//{
//	value = onoffMenuItem_m( event, value, y, s, condition ) ;
//	menu_lcd_HYPHINV( PARAM_OFS, y, value, condition ) ;		// Overtype the ON/OFF
//  return value ;
//}

//uint8_t onoffMenuItem( uint8_t value, uint8_t y, const char *s, uint8_t sub, int8_t subN, uint8_t event )
//{
//  lcd_puts_Pleft(y, s);
//  menu_lcd_onoff( PARAM_OFS, y, value, sub==subN ) ;
//  if(sub==subN) CHECK_INCDEC_H_GENVAR_0(event, value, 1);
//  return value ;
//}


void menuCustomTelemetry(uint8_t event)
{
	TITLE(PSTR(STR_TELEMETRY));
	static MState2 mstate2;

#ifdef PCBX9D	
	ImageDisplay = 0 ;
	event = mstate2.check_columns( event, 5 ) ;
	
	Columns = 2 ;

	uint8_t  sub   = mstate2.m_posVert ;

	uint8_t *pindex ;
	uint8_t *epindex ;
	uint8_t chr = '1' ;
  lcd_puts_Pleft( FH, PSTR(STR_CUSTOM_DISP) );
	pindex = g_model.customDisplayIndex ;
	epindex = g_model.customDisplay1Extra ;
	
	if ( sub <= 2 )
	{
		lcd_puts_P( 20*FW-4, 7*FH, XPSTR("->") ) ;
	}
	
	uint8_t subN = 0 ;
	uint8_t subSub = g_posHorz;
	if ( sub >= 3 )
	{
		subN = 3 ;
		pindex = g_model.customDisplay2Index ;
		epindex = g_model.customDisplay2Extra ;
		chr = '2' ;
	}
	lcd_putc( 15*FW, FH, chr ) ;

	for (uint8_t j=0; j<3; j++)
	{
		for ( uint8_t k = 0 ; k < 3 ; k += 1 )
		{
			uint8_t x = k * 64 ;
			uint8_t y = ( j*2 + 2 ) * FH ;
		  uint8_t attr = ((sub==subN && subSub==k)) ? InverseBlink : 0 ;
			uint8_t *p = &pindex[j*2+k] ;
			if ( k == 2 )
			{
				p = &epindex[j] ;
			}
			if ( *p )
			{
				uint32_t index = *p - 1 ;
				putsAttIdxTelemItems( x, y, *p, attr ) ;
				putsTelemetryChannel( x+5*FW-1, y+FH, index, get_telemetry_value(index), 0, TELEM_UNIT ) ;
				if ( index < 2 )		// A1 or A2
				{
  				lcd_outdezAtt( x+52, y, get_telemetry_value(index), 0 ) ;
					lcd_putc( x+32, y, '[' ) ;
					lcd_putc( x+54, y, ']' ) ;
				}
			}
			else
			{
    		lcd_putsAtt(  x, y, XPSTR("----"), attr ) ;
			}
  		if ((sub==subN && subSub==k)) CHECK_INCDEC_H_MODELVAR_0( *p, NUM_TELEM_ITEMS) ;
		}
		subN++;
	}
#else
	event = mstate2.check_columns( event, 11 ) ;
	
	uint8_t  sub   = mstate2.m_posVert ;

	uint8_t *pindex ;
	uint8_t chr = '1' ;
  lcd_puts_Pleft( FH, PSTR(STR_CUSTOM_DISP) );
	pindex = g_model.customDisplayIndex ;
	
	if ( sub <= 5 )
	{
		lcd_puts_P( 20*FW-4, 7*FH, XPSTR("->") ) ;
	}
	
	uint8_t subN = 0 ;
	if ( sub >= 6 )
	{
		subN = 6 ;
		pindex = g_model.customDisplay2Index ;
		chr = '2' ;
	}
	lcd_putc( 15*FW, FH, chr ) ;

	for (uint8_t j=0; j<6; j++)
	{
		uint8_t x = ( j & 1 ) ? 32 : 0 ;
		uint8_t y = ( j + 2 ) * FH ;
	  uint8_t attr = ((sub==subN) ? InverseBlink : 0);
		if ( pindex[j] )
		{
			uint32_t index = pindex[j] - 1 ;
			putsAttIdxTelemItems( x, y, pindex[j], attr ) ;
			putsTelemetryChannel( 90, y, index, get_telemetry_value(index), 0, TELEM_UNIT ) ;
			if ( index < 2 )		// A1 or A2
			{
  			lcd_outdezAtt( 120, y, get_telemetry_value(index), 0 ) ;
				lcd_putc( 98, y, '[' ) ;
				lcd_putc( 122, y, ']' ) ;
			}
		}
		else
		{
    	lcd_putsAtt(  x, y, XPSTR("----"), attr ) ;
		}
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( pindex[j], NUM_TELEM_ITEMS) ;
		subN++;
	}
#endif
}

// FrSky WSHhi DSMx  Jeti  ArduP ArduC FrHub HubRaw FrMav Mavlk"

#if defined(PCBSKY) || defined(PCB9XT)
 #ifdef REVX
  #define MAX_TEL_OPTIONS		9
  const uint8_t TelOptions[] = {1,2,3,4,5,6,7,8,10} ;
 #else
  #define MAX_TEL_OPTIONS			7
  const uint8_t TelOptions[] = {1,2,3,5,6,7,8} ;
 #endif
#endif

#ifdef PCBX9D
 #define MAX_TEL_OPTIONS		6
 const uint8_t TelOptions[] = {1,2,3,5,6,7} ;
#endif


#ifdef FRSKY
void menuProcTelemetry(uint8_t event)
{
	uint32_t page2SizeExtra = 0 ;
	if ( g_model.telemetryProtocol == TELEMETRY_DSM )
	page2SizeExtra = 1 ;

	EditType = EE_MODEL ;

#if defined(PCBSKY) || defined(PCB9XT)
 #ifdef REVX
  #define TDATAITEMS	43-12
 #else
  #define TDATAITEMS	43-12
 #endif
#endif

#ifdef PCBX9D
  #define TDATAITEMS	42-12
#endif

		 
	TITLE(PSTR(STR_TELEMETRY));
	static MState2 mstate2;
//#ifdef PCBSKY
//	static const uint8_t mstate_tab[] = {0, 1, 1, 2, 2, 1, 2,2,1,1,0,1,1,0} ;
//#else
//	static const uint8_t mstate_tab[] = {0, 1, 1, 2, 2, 1, 2,2,1,1,0} ;
//#endif
//	if (SubMenuFromIndex)
//	{
//#ifdef PCBSKY
		event = mstate2.check_columns( event, TDATAITEMS-1+page2SizeExtra ) ;
//		event = mstate2.check(event,0,NULL,0,&Columns,0, TDATAITEMS ) ;
//#else
//		event = mstate2.check_columns( event, TDATAITEMS ) ;
//		event = mstate2.check(event,0,NULL,0,&Columns,0, TDATAITEMS ) ;
//#endif
//	}
//	else
//	{
//#ifdef REVX
//		event = mstate2.check(event,e_Telemetry,menuTabModel,DIM(menuTabModel),&Columns,0,TDATAITEMS) ;
//#else
//		event = mstate2.check(event,e_Telemetry,menuTabModel,DIM(menuTabModel),&Columns,0,TDATAITEMS) ;
//#endif
////    MENU(PSTR(STR_CUST_SWITCH), menuTabModel, e_Switches, NUM_SKYCSW+1, {0, 3/*repeated...*/});
//	}
//#ifdef REVX
//    MENU(PSTR(STR_TELEMETRY), menuTabModel, e_Telemetry, 13, {0, 1, 1, 2, 2, 1, 2,2,1,1,0,1});
//#else
//    MENU(PSTR(STR_TELEMETRY), menuTabModel, e_Telemetry, 11, {0, 1, 1, 2, 2, 1, 2,2,1,1,0});
//#endif

	uint8_t  sub   = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz ;
//uint8_t subSub = mstate2.m_posHorz;
uint8_t blink;
uint8_t y = 2*FH;

	switch(event)
	{
    case EVT_KEY_BREAK(KEY_DOWN):
    case EVT_KEY_BREAK(KEY_UP):
    case EVT_KEY_BREAK(KEY_LEFT):
    case EVT_KEY_BREAK(KEY_RIGHT):
      if(s_editMode)
			{
         FrskyAlarmSendState |= 0x30 ;	 // update Fr-Sky module when edit mode exited
         FRSKY_setModelAlarms(); // update Fr-Sky module when edit mode exited
			}
    break ;
		case EVT_ENTRY :
  		FrskyAlarmSendState |= 0x40 ;		// Get RSSI/TSSI alarms
		break ;
	}

	blink = InverseBlink ;
	uint8_t subN = 0 ;
	if ( sub <= TDATAITEMS+page2SizeExtra - 8 )
	{
		lcd_puts_P( 20*FW-4, 7*FH, XPSTR("->") ) ;
	}
//	columns = 0 ;

 if ( sub < 7 )
 {
	lcd_puts_Pleft(FH, PSTR(STR_USR_PROTO));
	{
		uint8_t b ;
		uint8_t attr = 0 ;

		b = g_model.telemetryProtocol ;
//		lcd_outhex4(64,0, (g_model.DsmTelemetry<<8) | b ) ;
		for ( uint32_t c = 0 ; c < MAX_TEL_OPTIONS ; c += 1 )
		{
			if ( TelOptions[c] == b )
			{
				b = c ;
				break ;
			}
		}
		if(sub==subN)
		{
			Columns = 1 ;

			if (subSub==0)
			{
				attr = blink ;
				CHECK_INCDEC_H_MODELVAR( b,0,MAX_TEL_OPTIONS-1) ;
			}
		}
		b = TelOptions[b] ;
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
		lcd_putsAttIdx( 10*FW, FH, PSTR(STR_FRHUB_WSHHI), b, attr ) ;

		attr = 0 ;
		b = g_model.FrSkyImperial ;
		if(sub==subN && subSub==1)
		{
			attr = blink ;
			CHECK_INCDEC_H_MODELVAR_0(b,1); g_model.FrSkyImperial = b ;
		}
		lcd_putsAttIdx( 17*FW, FH, PSTR(STR_MET_IMP), b, attr ) ;
	
	}
	subN++;

	for (int i=0; i<2; i++)
	{
    lcd_puts_Pleft(y, PSTR(STR_A_CHANNEL)) ;
    lcd_putc(FW, y, '1'+i);
    putsTelemValue(16*FW, y, 255, i, (sub==subN && subSub==0 ? blink:0)|NO_UNIT ) ;
    putsTelemValue( 21*FW, y, frskyTelemetry[i].value, i,  NO_UNIT ) ;
    //    lcd_putsnAtt(16*FW, y, PSTR("v-")+g_model.frsky.channels[i].type, 1, (sub==subN && subSub==1 ? blink:0));
    lcd_putsAttIdx(16*FW, y, XPSTR("\001v-VA"), g_model.frsky.channels[i].type, (sub==subN && subSub==1 ? blink:0));

    if (sub==subN)
		{
			Columns = 1 ;
			if ( (s_editMode || P1values.p1valdiff))
			{
        switch (subSub)
				{
        	case 0:
            g_model.frsky.channels[i].ratio = checkIncDec16( g_model.frsky.channels[i].ratio, 0, 255, EE_MODEL);
          break;
	        case 1:
            //            CHECK_INCDEC_H_MODELVAR(event, g_model.frsky.channels[i].type, 0, 1);
            CHECK_INCDEC_H_MODELVAR_0( g_model.frsky.channels[i].type, 3);
          break;
        }
			}
    }
    subN++; y+=FH;

    for (int j=0; j<2; j++)
		{
        uint8_t ag;
        uint8_t al;
        al = ALARM_LEVEL(i, j);
        ag = ALARM_GREATER(i, j);
        lcd_putsAtt(4, y, PSTR(STR_ALRM), 0);
        lcd_putsAttIdx(6*FW, y, PSTR(STR_YELORGRED),al,(sub==subN && subSub==0 ? blink:0));
        lcd_putsnAtt(11*FW, y, XPSTR("<>")+ag,1,(sub==subN && subSub==1 ? blink:0));
        putsTelemValue(16*FW, y, g_model.frsky.channels[i].alarms_value[j], i, (sub==subN && subSub==2 ? blink:0)|NO_UNIT ) ;

        if(sub==subN)
				{
					Columns = 2 ;
					if ((s_editMode || P1values.p1valdiff))
					{
            uint8_t original ;
            uint8_t value ;
            switch (subSub)
						{
            	case 0:
                value = checkIncDec( al, 0, 3, EE_MODEL) ;
                original = g_model.frsky.channels[i].alarms_level ;
                g_model.frsky.channels[i].alarms_level = j ? ( (original & 0xF3) | value << 2 ) : ( (original & 0xFC) | value ) ;
              break;
            	case 1:
                value = checkIncDec( ag, 0, 3, EE_MODEL) ;
                original = g_model.frsky.channels[i].alarms_greater ;
                g_model.frsky.channels[i].alarms_greater = j ? ( (original & 0xFD) | value << 1 ) : ( (original & 0xFE) | value ) ;
                if(checkIncDec_Ret)
                    FRSKY_setModelAlarms();
              break;
            	case 2:
                g_model.frsky.channels[i].alarms_value[j] = checkIncDec16( g_model.frsky.channels[i].alarms_value[j], 0, 255, EE_MODEL);
              break;
            }
					}
        }
        subN++; y+=FH;
    }
	}
 }
#if defined(PCBSKY) || defined(PCB9XT)
 else if ( sub < 12+page2SizeExtra) // sub>=7
#else
 else if ( sub < 10+page2SizeExtra) // sub>=7
#endif
 {
	uint8_t subN = 7 ;
	uint8_t b ;

	if (sub==subN)
	{
		Columns = 1 ;
	}
 	lcd_puts_Pleft( y, XPSTR( "RSSI Warn""\037""RSSI Critical""\037""Rx Voltage") ) ;
	uint8_t attr = ( ( (sub==subN) && (subSub==0) ) ? InverseBlink : 0) ;
	int8_t offset ;
	offset = rssiOffsetValue( 0 ) ;
	lcd_outdezAtt( 15*FW, y, g_model.rssiOrange + offset, attr ) ;
//#else // ASSAN
//	lcd_outdezAtt( 15*FW, y, g_model.rssiOrange + 45, attr ) ;
//#endif // ASSAN
  if( attr) CHECK_INCDEC_H_MODELVAR( g_model.rssiOrange, -18, 30 ) ;
	attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
	b = 1-g_model.enRssiOrange ;
	menu_lcd_onoff( PARAM_OFS+1, y, b, attr ) ;
  if( attr) { CHECK_INCDEC_H_MODELVAR_0( b, 1 ) ; g_model.enRssiOrange = 1-b ; }
	subN++; y+=FH;
		
	if (sub==subN)
	{
		Columns = 1 ;
	}
// 	lcd_puts_Pleft( y, XPSTR( "RSSI Critical") ) ;
	attr = ( ( (sub==subN) && (subSub==0) ) ? InverseBlink : 0) ;
	offset = rssiOffsetValue( 1 ) ;
	lcd_outdezAtt( 15*FW, y, g_model.rssiRed + offset, attr ) ;
//#else // ASSAN
//	lcd_outdezAtt( 15*FW, y, g_model.rssiRed + 42, attr ) ;
//#endif // ASSAN
	if( attr) CHECK_INCDEC_H_MODELVAR( g_model.rssiRed, -17, 30 ) ;
	attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
	b = 1-g_model.enRssiRed ;
	menu_lcd_onoff( PARAM_OFS+1, y, b, attr ) ;
  if( attr) { CHECK_INCDEC_H_MODELVAR_0( b, 1 ) ; g_model.enRssiRed = 1-b ; }

	subN++; y+=FH;
 
	attr = (sub==subN) ? InverseBlink : 0 ;
//  lcd_puts_Pleft(y, XPSTR( "Rx Voltage") ) ;
	lcd_outdezAtt( 16*FW, y, g_model.rxVratio, attr|PREC1 ) ;
  lcd_putc(Lcd_lastPos, y, 'v' ) ;
	lcd_outdezAtt( 21*FW, y, convertRxv( FrskyHubData[FR_RXV] ), PREC1 ) ;
	if( attr) { g_model.rxVratio = checkIncDec16( g_model.rxVratio, 0, 255, EE_MODEL ) ; }
	subN++; y+=FH;


	if ( page2SizeExtra )
	{
 		lcd_puts_Pleft( y, XPSTR( "DSM 'A' as RSSI") ) ;
 		menu_lcd_onoff( PARAM_OFS, y, g_model.dsmAasRssi, sub==subN ) ;
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.dsmAasRssi, 1);
		subN++; y+=FH;
	}

#if defined(PCBSKY) || defined(PCB9XT)
	if ( FrskyTelemetryType == 2 )
	{
		if (sub==subN)
		{
			Columns = 1 ;
		}
 		lcd_puts_Pleft( y, XPSTR( "DSM Warning") ) ;
		
//		attr = ( ( (sub==subN) && (subSub==0) ) ? InverseBlink : 0) ;
//  	lcd_putsAttIdx(13*FW, y, XPSTR("\006fades lossesholds "),g_model.dsmLinkData.sourceWarn,attr);
//  	if( attr) { CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.sourceWarn, 2 ) ; }
		g_model.dsmLinkData.sourceWarn = checkIndexed( y, XPSTR(FWx13"\002""\006fades lossesholds "), g_model.dsmLinkData.sourceWarn, (sub==subN) && (subSub==0) ) ;
		attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
		lcd_outdezAtt( 21*FW, y, g_model.dsmLinkData.levelWarn, attr ) ;
  	if( attr) { CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.levelWarn, 40 ) ; }
		subN++; y+=FH;

		if (sub==subN)
		{
			Columns = 1 ;
		}
 		lcd_puts_Pleft( y, XPSTR( "DSM Critical") ) ;
//		attr = ( ( (sub==subN) && (subSub==0) ) ? InverseBlink : 0) ;
//  	lcd_putsAttIdx(13*FW, y, XPSTR("\006fades lossesholds "),g_model.dsmLinkData.sourceCritical,attr);
//  	if( attr) { CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.sourceCritical, 2 ) ; }
		g_model.dsmLinkData.sourceCritical = checkIndexed( y, XPSTR(FWx13"\002""\006fades lossesholds "), g_model.dsmLinkData.sourceCritical, (sub==subN) && (subSub==0) ) ;
		attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
		lcd_outdezAtt( 21*FW, y, g_model.dsmLinkData.levelCritical, attr ) ;
  	if( attr) { CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.levelCritical, 40 ) ; }
		subN++; y+=FH;
	}
	else
	{
		if ( (sub==subN) || (sub==subN+1))
		{
			if ( M_lastVerticalPosition	< mstate2.m_posVert )
			{
				mstate2.m_posVert = 12 ;
			}
			else if ( M_lastVerticalPosition > mstate2.m_posVert )
			{
				mstate2.m_posVert = 9 ;
			}
		}
	}
#endif

 }
#if defined(PCBSKY) || defined(PCB9XT)
 else if ( sub < 17+page2SizeExtra) // sub>=8
#else
 else if ( sub < 15+page2SizeExtra) // sub>=8
#endif
	{
#if defined(PCBSKY) || defined(PCB9XT)
		uint8_t subN = 12+page2SizeExtra ;
#else
		uint8_t subN = 10+page2SizeExtra ;
#endif
		y = FH ;
		
		uint8_t attr = 0 ;
//		lcd_puts_Pleft(y, PSTR(STR_NUM_BLADES));
// 	  lcd_outdezAtt(14*FW, y, g_model.numBlades, (sub==subN) ? INVERS : 0) ;
  	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR( g_model.numBlades, 1, 127 ) ; }
		lcd_xlabel_decimal( 14*FW, y, g_model.numBlades, attr, PSTR(STR_NUM_BLADES) ) ;
		y += FH ;
		subN++;

		attr = 0 ;
  	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.FrSkyAltAlarm, 2 ) ; }
		lcd_puts_Pleft( y, PSTR(STR_ALT_ALARM));
  	lcd_putsAttIdx(11*FW, y, PSTR(STR_OFF122400),g_model.FrSkyAltAlarm, attr ) ;
//		lcd_xlabel_decimal( 11*FW, y, g_model.FrSkyAltAlarm, attr, PSTR(STR_ALT_ALARM) ) ;
  	y += FH ;
  	subN++;
  
		lcd_puts_Pleft( y, PSTR(STR_VOLT_THRES));
  	lcd_outdezNAtt(  14*FW, y, g_model.frSkyVoltThreshold * 2 ,((sub==subN) ? blink:0) | PREC2, 4);
  	if(sub==subN)
		{
  	  g_model.frSkyVoltThreshold=checkIncDec16( g_model.frSkyVoltThreshold, 0, 210, EE_MODEL);
  	}
  	y += FH ;
		subN++;
	
  	lcd_puts_Pleft( y, PSTR(STR_GPS_ALTMAIN) ) ;
  	menu_lcd_onoff( PARAM_OFS, y, g_model.FrSkyGpsAlt, sub==subN ) ;
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
  	lcd_outdezAtt( 14*FW, y, value, attr ) ;
		if ( active )
		{
  		g_model.frskyAlarms.alarmData[0].frskyAlarmLimit = checkIncDec16( g_model.frskyAlarms.alarmData[0].frskyAlarmLimit, 0, 200, EE_MODEL);
		}
  	attr = ((sub==subN && subSub==1) ? InverseBlink : 0);
		active = (attr && s_editMode) ;
		lcd_putsAttIdx(15*FW, y, PSTR(STR_SOUNDS), g_model.frskyAlarms.alarmData[0].frskyAlarmSound,attr);
		if ( active )
		{
  		CHECK_INCDEC_H_MODELVAR_0( g_model.frskyAlarms.alarmData[0].frskyAlarmSound, 15 ) ;
		}
  	subN++;


	}
//#if defined(PCBSKY) || defined(PCB9XT)
// else if ( sub < 24+6) // sub>=8
//#else
// else if ( sub < 22+6) // sub>=8
//#endif
//	{
//		uint8_t *pindex ;
//		uint8_t chr = '1' ;
//  	lcd_puts_Pleft( FH, PSTR(STR_CUSTOM_DISP) );
//		pindex = g_model.customDisplayIndex ;
//#if defined(PCBSKY) || defined(PCB9XT)
//		uint8_t subN = 18 ;
//		if ( sub >= 24 )
//		{
//			subN = 24 ;
//			pindex = g_model.customDisplay2Index ;
//			chr = '2' ;
//		}
//#else
//		uint8_t subN = 16 ;
//		if ( sub >= 22 )
//		{
//			subN = 22 ;
//			pindex = g_model.customDisplay2Index ;
//			chr = '2' ;
//		}
//#endif
//		lcd_putc( 15*FW, FH, chr ) ;

//		for (uint8_t j=0; j<6; j++)
//		{
//			uint8_t x = ( j & 1 ) ? 64 : 0 ;
//			uint8_t y = ( ( j & 6 ) + 2 ) * FH ;
//	  	uint8_t attr = ((sub==subN) ? InverseBlink : 0);
//			if ( pindex[j] )
//			{
//				putsAttIdxTelemItems( x, y, pindex[j], attr ) ;
//			}
//			else
//			{
//    		lcd_putsAtt(  x, y, XPSTR("----"), attr ) ;
//			}
//  		if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( pindex[j], NUM_TELEM_ITEMS) ;
//			subN++;
//		}
//	}
	else if ( sub <= TDATAITEMS - 8 + page2SizeExtra )
	{
#if defined(PCBSKY) || defined(PCB9XT)
		uint8_t subN = 24+6-12-1+page2SizeExtra ;
#else
		uint8_t subN = 22+6-12-1+page2SizeExtra ;
#endif
		y = FH ;
  	lcd_puts_Pleft( y, PSTR(STR_BT_TELEMETRY) );
#ifdef REVX
		g_model.bt_telemetry = checkIndexed( y, XPSTR(FWx15"\002""\005  Off   RxTx/Rx"), g_model.bt_telemetry, (sub==subN) ) ;
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.bt_telemetry, 2);
#else
  	menu_lcd_onoff( PARAM_OFS, y, g_model.bt_telemetry, sub==subN ) ;
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.bt_telemetry, 1);
#endif
  	y += FH ;
		subN += 1 ;
  	
		lcd_puts_Pleft( y, PSTR(STR_FRSKY_COM_PORT) );
    uint8_t attr = (sub == subN) ? blink : 0 ;
  	lcd_putcAtt( 16*FW, y, g_model.frskyComPort + '1', attr ) ;
		if (attr) CHECK_INCDEC_H_MODELVAR_0( g_model.frskyComPort, 1 ) ;
	  y += FH ;
		subN += 1 ;
  	
		uint8_t previous = g_model.telemetryRxInvert ;
		lcd_puts_Pleft( y, PSTR(STR_INVERT_COM1) );
  	menu_lcd_onoff( PARAM_OFS, y, g_model.telemetryRxInvert, sub==subN ) ;
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.telemetryRxInvert, 1);
		if ( g_model.telemetryRxInvert != previous )
		{
#ifdef REVX
			if ( g_model.telemetryRxInvert )
			{
				setMFP() ;
			}
			else
			{
				clearMFP() ;
			}
#else
			TelemetryType = TEL_UNKNOWN ;
#endif
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
  	lcd_outdezAtt( 15*FW, y, g_model.FASoffset, attr ) ;
	  y += FH ;
		subN += 1 ;

		lcd_puts_Pleft( y, XPSTR("COM2 Func.") );
		uint8_t b = g_model.com2Function ;
    attr = 0 ;
		if ( (sub == subN) )
		{
			attr = blink ;
#ifdef PCBSKY
	  	CHECK_INCDEC_H_MODELVAR_0( g_model.com2Function, 4 ) ;
#endif
#ifdef PCBX9D
	  	CHECK_INCDEC_H_MODELVAR_0( g_model.com2Function, 3 ) ;
#endif
#ifdef PCB9XT
	  	CHECK_INCDEC_H_MODELVAR_0( g_model.com2Function, 2 ) ;
#endif
		}
#ifdef PCBSKY
		lcd_putsAttIdx(12*FW, y, XPSTR("\011TelemetrySbusTrainSbus57600BTdirect FMSserial"), g_model.com2Function, attr ) ;
#endif
#ifdef PCBX9D
		lcd_putsAttIdx(12*FW, y, XPSTR("\011TelemetrySbusTrainSbus57600CppmTrain"), g_model.com2Function, attr ) ;
#endif
#ifdef PCB9XT
		lcd_putsAttIdx(12*FW, y, XPSTR("\011TelemetrySbusTrainSbus57600"), g_model.com2Function, attr ) ;
#endif
		if ( g_model.com2Function != b )
		{
#ifdef PCBX9D
			if ( b == 3 )
			{
				stop_serial_trainer_capture() ;
			}
#endif
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

#ifdef PCBSKY
		if ( b != g_model.com2Baudrate )
		{
			if( g_model.com2Function == COM2_FUNC_BTDIRECT )
			{
				if ( g_model.com2Baudrate )
				{
					com2_Configure( g_model.com2Baudrate-1, SERIAL_NO_PARITY ) ;
//					UART_Configure( g_model.com2Baudrate-1, Master_frequency ) ;
				}
			}
		}
#endif	// PCBSKY
	  y += FH ;
		subN += 1 ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
		lcd_puts_Pleft( y, XPSTR("COM1 Baudrate") );
		b = g_model.telemetryBaudrate ;
		g_model.telemetryBaudrate = checkIndexed( y, XPSTR(BaudString), g_model.telemetryBaudrate, (sub==subN) ) ;
	  y += FH ;
		subN += 1 ;
#endif	// PCBSKY

//#ifdef PCBSKY
//		lcd_puts_Pleft( y, XPSTR("BT as Trainer") );
//		if ( (sub == subN) )
//		{
//			CHECK_INCDEC_H_MODELVAR_0(event, g_model.BTfunction, 1 ) ;
//		}
//  	menu_lcd_onoff( PARAM_OFS, y, g_model.BTfunction, sub==subN ) ;
//#endif
		
	}
	else
	{
		uint8_t subN = TDATAITEMS - 7 + page2SizeExtra ;
		// Vario
   	for( CPU_UINT j=0 ; j<7 ; j += 1 )
		{
			uint8_t b ;
      uint8_t attr = (sub==subN) ? blink : 0 ;
			uint8_t y = (1+j)*FH ;

			switch ( j )
			{
				case 0 :
					lcd_puts_Pleft( y, PSTR(STR_VARIO_SRC) ) ;
					lcd_putsAttIdx( 15*FW, y, PSTR(STR_VSPD_A2), g_model.varioData.varioSource, attr ) ;
   		  	if(attr)
					{
						CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.varioSource, 2+NUM_SCALERS ) ;
   		  	}
				break ;
				
				case 1 :
					lcd_puts_Pleft( y, PSTR(STR_2SWITCH) ) ;
					g_model.varioData.swtch = edit_dr_switch( 15*FW, y, g_model.varioData.swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				break ;

				case 2 :
					lcd_puts_Pleft( y, PSTR(STR_2SENSITIVITY) ) ;
 					lcd_outdezAtt( 17*FW, y, g_model.varioData.param, attr) ;
   			  if(attr)
					{
						CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.param, 50 ) ;
	   		  }
				break ;

				case 3 :
	        b = g_model.varioData.sinkTones ;
					g_model.varioData.sinkTones = offonMenuItem( b, y, PSTR(STR_SINK_TONES), attr ) ;
				break ;

				case 4 :
					lcd_puts_Pleft( y, PSTR(STR_LOG_SWITCH) ) ;
					g_model.logSwitch = edit_dr_switch( 15*FW, y, g_model.logSwitch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				break ;

				case 5 :
					lcd_puts_Pleft( y, PSTR(STR_LOG_RATE) ) ;
					lcd_putsAttIdx( 15*FW, y, XPSTR("\0041.0s2.0s0.5s"), g_model.logRate, attr ) ;
   			  if(attr)
					{
						CHECK_INCDEC_H_MODELVAR_0( g_model.logRate, 2 ) ;
	   		  }
				break ;

				case 6 :
					lcd_puts_Pleft( y, XPSTR("Current Source" ) ) ;
					lcd_putsAttIdx( 15*FW, y, XPSTR("\004----A1  A2  Fas SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "), g_model.currentSource, attr ) ;
   			  if(attr)
					{
						CHECK_INCDEC_H_MODELVAR_0( g_model.currentSource, 3 + NUM_SCALERS ) ;
	   		  }
				break ;

			}

			subN += 1 ;
		}
	}

}

extern uint8_t frskyRSSIlevel[2] ;
extern uint8_t frskyRSSItype[2] ;


#endif

void menuCustomCheck(uint8_t event)
{
	static MState2 mstate2 ;
	mstate2.check_columns(event, 3-1 ) ;
  lcd_puts_Pleft( 0, PSTR( STR_CUSTOM_CHECK ) ) ;
	
  int8_t sub = mstate2.m_posVert;
	
	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	CustomCheckData *pdata ;
	pdata = &g_model.customCheck ;

	for (uint8_t k = 0 ; k < 3 ; k += 1 )
	{
    uint8_t y = (k+1) * FH ;
    uint8_t i = k ;
		uint8_t attr = (sub==i ? InverseBlink : 0);
		switch(i)
		{
      case 0 :	// Source
			{	
				uint8_t x = unmapPots( pdata->source ) ;
				
				lcd_puts_Pleft( y, XPSTR("Source") ) ;
				putsChnRaw( 11*FW, y, x, attr ) ;
//#if NUM_EXTRA_POTS
//				if( attr )
//				{
//					uint8_t x = mapPots( pscaler->source ) ;
//					CHECK_INCDEC_H_MODELVAR( x, 0, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NUM_EXTRA_POTS ) ;
//					pscaler->source = unmapPots( x ) ;
//				}
//#else
				if( attr ) CHECK_INCDEC_H_MODELVAR_0( pdata->source, 7 + NumExtraPots ) ; // NUM_SKYXCHNRAW+NUM_TELEM_ITEMS ) ;
//#endif
			}
			break ;
			case 1 :
				lcd_puts_Pleft( y, XPSTR("Min.") ) ;
				lcd_outdezAtt( 13*FW, y, pdata->min, attr) ;
				if( attr ) CHECK_INCDEC_H_MODELVAR( pdata->min, -125, 125 ) ;
			break ;
      case 2 :	// offset
				lcd_puts_Pleft( y, XPSTR("Max.") ) ;
				lcd_outdezAtt( 13*FW, y, pdata->max, attr) ;
				if( attr ) CHECK_INCDEC_H_MODELVAR( pdata->max, -125, 125 ) ;
			break ;
		}
	}
}


#if GVARS

uint8_t scalerDecimal( uint8_t y, uint8_t val, uint8_t attr )
{
  lcd_outdezAtt( 13*FW, y, val+1, attr ) ;
	if (attr) val = checkIncDec16( val, 0, 255, EE_MODEL);
	return val ;
}

void menuScaleOne(uint8_t event)
{
	static MState2 mstate2 ;
	mstate2.check_columns(event, 9-1 ) ;
  lcd_puts_Pleft( 0, XPSTR("SC  =") ) ;
	uint8_t index = s_currIdx ;
  lcd_putc( 2*FW, 0, index+'1' ) ;
	
  int8_t sub = mstate2.m_posVert;
	evalOffset(sub);
	
	putsTelemetryChannel( 8*FW, 0, index+TEL_ITEM_SC1, 0, 0, TELEM_UNIT ) ;

	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
  
	for (uint8_t k = 0 ; k < 7 ; k += 1 )
	{
    uint8_t y = (k+1) * FH ;
    uint8_t i = k + s_pgOfs;
		uint8_t attr = (sub==i ? InverseBlink : 0);
		ScaleData *pscaler ;
		pscaler = &g_model.Scalers[index] ;
		switch(i)
		{
      case 0 :	// Source
				lcd_puts_Pleft( y, XPSTR("Source") ) ;
				putsChnRaw( 11*FW, y, pscaler->source, attr ) ;
//#if NUM_EXTRA_POTS
				if( attr )
				{
					uint8_t x = mapPots( pscaler->source ) ;
					CHECK_INCDEC_H_MODELVAR( x, 0, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots ) ;
					pscaler->source = unmapPots( x ) ;
				}
//#else
//				if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->source, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS ) ;
//#endif
			break ;
			case 1 :	// name
				alphaEditName( 11*FW-2, y, (uint8_t *)pscaler->name, sizeof(pscaler->name), attr, (uint8_t *)XPSTR( "Scaler Name") ) ;
			break ;
      case 2 :	// offset
				lcd_puts_Pleft( y, PSTR(STR_OFFSET) ) ;
				lcd_outdezAtt( 13*FW, y, pscaler->offset, attr) ;
				if ( attr )
				{
					StepSize = 100 ;
					pscaler->offset = checkIncDec16( pscaler->offset, -32000, 32000, EE_MODEL ) ;
				}
			break ;
      case 3 :	// mult
				lcd_puts_Pleft( y, XPSTR("Multiplier") ) ;
				pscaler->mult = scalerDecimal( y, pscaler->mult, attr ) ;
			break ;
      case 4 :	// div
				lcd_puts_Pleft( y, XPSTR("Divisor") ) ;
				pscaler->div = scalerDecimal( y, pscaler->div, attr ) ;
			break ;
      case 5 :	// unit
				lcd_puts_Pleft( y, XPSTR("Unit") ) ;
				lcd_putsAttIdx( 11*FW, y, XPSTR(UnitsString), pscaler->unit, attr ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->unit, 8 ) ;
			break ;
      case 6 :	// sign
				lcd_puts_Pleft( y, XPSTR("Sign") ) ;
  			lcd_putcAtt( 11*FW, y, pscaler->neg ? '-' : '+', attr ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->neg, 1 ) ;
			break ;
      case 7 :	// precision
				lcd_puts_Pleft( y, XPSTR("Decimals") ) ;
				lcd_outdezAtt( 13*FW, y, pscaler->precision, attr) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->precision, 2 ) ;
			break ;
      case 8 :	// offsetLast
				lcd_puts_Pleft( y, XPSTR("Offset At") ) ;
				lcd_putsAttIdx( 11*FW, y, XPSTR("\005FirstLast "), pscaler->offsetLast, attr ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->offsetLast, 1 ) ;
			break ;
		}
	}

}

void menuProcAdjust(uint8_t event)
{
	TITLE(XPSTR("GVAR Adjust"));
	EditType = EE_MODEL ;
	static MState2 mstate2;
 	event = mstate2.check_columns(event, NUM_GVAR_ADJUST + EXTRA_GVAR_ADJUST - 1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
	uint8_t y = FH ;
	
  uint8_t t_pgOfs ;
	t_pgOfs = evalOffset(sub);
  uint8_t k ;
	Columns = 3 ;
	
	for (uint8_t i=0; i<7; i++ )
	{
		GvarAdjust *pgvaradj ;
    y=(i+1)*FH;
    k=i+t_pgOfs;
		pgvaradj = ( k >= NUM_GVAR_ADJUST ) ? &g_model.egvarAdjuster[k - NUM_GVAR_ADJUST] : &g_model.gvarAdjuster[k] ;
  	lcd_putc( 0, y, 'A' ) ;
  	lcd_puts_P( 3*FW+2, y, XPSTR("GV") ) ;
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
  		lcd_puts_Pleft( 0, XPSTR("\015GV =") ) ;
  		lcd_putc( 15*FW, 0, pgvaradj->gvarIndex+'1' ) ;
			lcd_outdez( 20*FW, 0, g_model.gvars[pgvaradj->gvarIndex].gvar) ;
		} 
		for( uint32_t j = 0 ; j < 4 ; j += 1 )
		{
      uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
			uint8_t active = (attr && (s_editMode) ) ;
			if ( j == 0 )
			{
				lcd_putcAtt( 5*FW+2, y, pgvaradj->gvarIndex+'1', attr ) ;
				if ( active )
				{
 	        CHECK_INCDEC_H_MODELVAR_0( pgvaradj->gvarIndex, 6 ) ;
				}
			}
			else if ( j == 1 )
			{
				uint32_t old = pgvaradj->function ;
				pgvaradj->function = checkIndexed( y, XPSTR(FWx7"\010""\005-----Add  Set CSet V+/-  Inc/0Dec/0+/Lim-/Lim"), pgvaradj->function, attr ) ;
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
			else if ( j == 2 )
			{
				pgvaradj->swtch = edit_dr_switch( 13*FW-1, y, pgvaradj->swtch, attr, active ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			}
			else
			{
				if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
				{
					if ( pgvaradj->function == 3 )
					{
						lcd_putsAttIdx( 18*FW, y, PSTR(STR_GV_SOURCE), pgvaradj->switch_value, attr ) ;
#if defined(PCBSKY) || defined(PCB9XT)
		  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgvaradj->switch_value, 68 ) ;
#endif
#ifdef PCBX9D
		  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgvaradj->switch_value, 69 ) ;
#endif
					}
					else
					{
						lcd_outdezAtt( 21*FW, y, pgvaradj->switch_value, attr ) ;
						if ( active )
						{
 	      	  	CHECK_INCDEC_H_MODELVAR( pgvaradj->switch_value, -125, 125 ) ;
						}
					}
				}
				else
				{
					pgvaradj->switch_value = edit_dr_switch( 17*FW, y, pgvaradj->switch_value, attr, active ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				}
			}
		}
	}
}

void menuProcScalers(uint8_t event)
{
	TITLE(XPSTR("Scalers"));
	EditType = EE_MODEL ;
	static MState2 mstate2;
 	event = mstate2.check_columns(event, NUM_SCALERS-1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;
	uint8_t y = FH ;
	
  uint8_t t_pgOfs = (sub == 7 ) ? 1 : 0 ;
  uint8_t k ;
	
  switch (event)
	{
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE) :
//			if ( ( sub >= 7 ) && ( subN < 7 + NUM_SCALERS ) )
//			{
        s_currIdx = sub ;
        killEvents(event);
        pushMenu(menuScaleOne) ;
//    	}
		break;
  }
	
	for (uint8_t i=0; i<7; i++ )
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
  	lcd_puts_Pleft( y, XPSTR("SC\011+\015*\022/") ) ;
  	lcd_putc( 2*FW, y, k+'1' ) ;
		putsChnRaw( 4*FW, y, g_model.Scalers[k].source, 0 ) ;
		lcd_outdezAtt( 12*FW+3, y, g_model.Scalers[k].offset, 0) ;
		lcd_outdezAtt( 16*FW, y, g_model.Scalers[k].mult+1, 0) ;
		lcd_outdezAtt( 21*FW, y, g_model.Scalers[k].div+1, 0) ;
	}
	lcd_char_inverse( 0, (sub-t_pgOfs+1)*FH, 126, 0 ) ;
}

void menuProcGlobals(uint8_t event)
{
	TITLE(PSTR(STR_GLOBAL_VARS));
	EditType = EE_MODEL ;
	static MState2 mstate2;
//	static const uint8_t mstate_tab[] = {0,2,2,2,2,2,2,2,0} ;
	
//	if (SubMenuFromIndex)
//	{
		event = mstate2.check_columns(event, MAX_GVARS - 1 ) ;
//		event = mstate2.check_columns(event, MAX_GVARS + NUM_SCALERS + NUM_GVAR_ADJUST-1 ) ;
//		event = mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1, MAX_GVARS + 1 + NUM_SCALERS-1 ) ;
//	}
//	else
//	{
//		event = mstate2.check(event,e_Globals,menuTabModel,DIM(menuTabModel),mstate_tab,DIM(mstate_tab)-1, MAX_GVARS + 1 + NUM_SCALERS-1 ) ;
////	  MENU(PSTR(STR_GLOBAL_VARS), menuTabModel, e_Globals, MAX_GVARS + 1 + NUM_SCALERS, {0,2,2,2,2,2,2,2,0} ) ;
//	}

	uint8_t subN = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
	uint8_t y = FH ;
//	uint8_t subSub = mstate2.m_posHorz;
	
//	lcd_outhex4( 0, 7*FH, ( (convert_mode_helper(g_model.gvars[1].gvsource) - 1) << 8 )+ (g_model.gvars[1].gvsource - 1) ) ;
//  switch (event)
//	{
//    case EVT_KEY_FIRST(KEY_MENU) :
//    case EVT_KEY_BREAK(BTN_RE) :
//			if ( ( subN >= 7 ) && ( subN < 7 + NUM_SCALERS ) )
//			{
//        s_currIdx = subN - 7 ;
////				RotaryState = ROTARY_MENU_UD ;
//        killEvents(event);
//        pushMenu(menuScaleOne) ;
//    	}
//		break;
//  }
 if ( subN < MAX_GVARS )
 {
	Columns = 2 ;
	for (CPU_UINT i=0; i<MAX_GVARS; i++ )
	{
//		uint8_t y = (i+1)*FH ;
    lcd_puts_Pleft(y, PSTR(STR_GV));
		lcd_putc( 2*FW, y, i+'1') ;
		for(uint8_t j=0; j<3;j++)
		{
      uint8_t attr = ((subN==i && subSub==j) ? InverseBlink : 0);
			uint8_t active = (attr && (s_editMode) ) ;
			GvarData *pgvar ;
			pgvar = &g_model.gvars[i] ;
      if ( j == 0 )
			{
       	putsDrSwitches( 6*FW, y, g_model.gvswitch[i] ,attr );
  			if(active) CHECK_INCDEC_MODELSWITCH( g_model.gvswitch[i], -MaxSwitchIndex, MaxSwitchIndex) ;
			}
			else if ( j == 1 )
			{
//#ifdef PCBSKY
//				if ( pgvar->gvsource < 69 )
//#endif
//#ifdef PCBX9D
//				if ( pgvar->gvsource < 70 )
//#endif
//				{				
				if ( pgvar->gvsource >= EXTRA_POTS_START )
				{
					lcd_putsAttIdx( 12*FW, y, PSTR(STR_CHANS_EXTRA), pgvar->gvsource - EXTRA_POTS_START, attr ) ;
				}
				else
				{
					lcd_putsAttIdx( 12*FW, y, PSTR(STR_GV_SOURCE), pgvar->gvsource, attr ) ;
				}
//				}
//				else
//				{
//#ifdef PCBSKY
//					lcd_putsAttIdx( 12*FW, y, XPSTR("\004L1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"), pgvar->gvsource-69, attr ) ;
//#endif
//#ifdef PCBX9D
//					lcd_putsAttIdx( 12*FW, y, XPSTR("\004L1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"), pgvar->gvsource-70, attr ) ;
//#endif
//				}
					// STR_GV_SOURCE
#if defined(PCBSKY) || defined(PCB9XT)
//  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgvar->gvsource, 68+12 ) ;
  			if(active)
				{
					uint32_t value = pgvar->gvsource ;
					// map pots etc.
					if ( value >= EXTRA_POTS_START )
					{
						value += 69 - EXTRA_POTS_START ;
					}
					CHECK_INCDEC_H_MODELVAR_0( value, 68 + NumExtraPots ) ;
					if ( value > 68 )
					{
						value += EXTRA_POTS_START - 69 ;
					}
					pgvar->gvsource = value ;
				}
#endif
#ifdef PCBX9D
//  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgvar->gvsource, 69+12 ) ;
  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgvar->gvsource, 69 ) ;
#endif
			}
			else
			{
				lcd_outdezAtt( 19*FW, y, pgvar->gvar, attr) ;
  			if(active) CHECK_INCDEC_H_MODELVAR(  pgvar->gvar, -125, 125 ) ;
			}
		}
		y += FH ;
	}
 }
}

#endif

#ifndef NO_TEMPLATES
void menuProcTemplates(uint8_t event)  //Issue 73
{
	TITLE( PSTR(STR_TEMPLATES) ) ;
	static MState2 mstate2 ;
	event = mstate2.check_columns( event, NUM_TEMPLATES ) ;
	EditType = EE_MODEL ;
//    SIMPLE_MENU(PSTR(STR_TEMPLATES), menuTabModel, e_Templates, NUM_TEMPLATES+2);

    uint8_t t_pgOfs ;
    uint8_t y = 0;
    uint8_t k = 0;
    int8_t  sub    = mstate2.m_posVert ;

    t_pgOfs = evalOffset(sub);

    switch(event)
    {
    case EVT_KEY_LONG(KEY_MENU):
        killEvents(event);
        //apply mixes or delete
        s_noHi = NO_HI_LEN;
//        if(sub==NUM_TEMPLATES)
//            clearMixes();
//        else if((sub>=0) && (sub<(int8_t)NUM_TEMPLATES))
            applyTemplate(sub);
        audioDefevent(AU_WARNING2);
        break;
    }

    y=1*FH;
    for(uint8_t i=0; i<7; i++){
        k=i+t_pgOfs;
        if(k==NUM_TEMPLATES) break;

        //write mix names here
        lcd_outdezNAtt(3*FW, y, k+1, (sub==k ? INVERS : 0) + LEADING0,2);
        lcd_putsAtt(  4*FW, y, PSTR(n_Templates[k]), (s_noHi ? 0 : (sub==k ? INVERS  : 0)));
        y+=FH;
    }

//    if(y>7*FH) return;
//    uint8_t attr = s_noHi ? 0 : ((sub==NUM_TEMPLATES) ? INVERS : 0);
//    lcd_putsAtt(  1*FW,y,PSTR(STR_CLEAR_MIXES),attr);
//    y+=FH;

}
#endif

//FunctionData Function[1] ;


void menuProcSafetySwitches(uint8_t event)
{
	
	TITLE(PSTR(STR_SAFETY_SW));
	EditType = EE_MODEL ;
	static MState2 mstate2;
//	static const uint8_t mstate_tab[] = {0, 0, 2/*repeated*/} ;
//	if (SubMenuFromIndex)
//	{
		event = mstate2.check_columns(event, NUM_SKYCHNOUT+1+1+NUM_VOICE-1 - 1 ) ;
//		event = mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1,NUM_SKYCHNOUT+1+1+NUM_VOICE-1) ;
//	}
//	else
//	{
//		event = mstate2.check(event,e_SafetySwitches,menuTabModel,DIM(menuTabModel),mstate_tab,DIM(mstate_tab)-1,NUM_SKYCHNOUT+1+1+NUM_VOICE-1) ;
////	MENU(PSTR(STR_SAFETY_SW), menuTabModel, e_SafetySwitches, NUM_SKYCHNOUT+1+1+NUM_VOICE, {0, 0, 2/*repeated*/});
//	}

	uint8_t y = 0;
	uint8_t k = 0;
	int8_t  sub    = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
//	uint8_t subSub = mstate2.m_posHorz;
  uint8_t t_pgOfs ;

	t_pgOfs = evalOffset(sub);

//  lcd_puts_P( 0*FW, 1*FH,PSTR("ch    sw     val"));
	for(uint8_t i=0; i<7; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
		if ( k == 0 )
		{
			uint8_t attr = 0 ;
    	if(sub==k)
			{
				Columns = 0 ;
				attr = InverseBlink ;
  	    CHECK_INCDEC_H_MODELVAR_0( g_model.numVoice, NUM_SKYCHNOUT ) ;
			}	
//  	  lcd_puts_Pleft( y, PSTR(STR_NUM_VOICE_SW) ) ;
//			lcd_outdezAtt( 18*FW, y, g_model.numVoice+NUM_VOICE, attr ) ;
			lcd_xlabel_decimal( 18*FW, y, g_model.numVoice+NUM_VOICE, attr, PSTR(STR_NUM_VOICE_SW) ) ;
		}
  	else // if(k<NUM_SKYCHNOUT+1+NUM_VOICE)
		{
			uint8_t numSafety = NUM_SKYCHNOUT - g_model.numVoice ;
    	SKYSafetySwData *sd = &g_model.safetySw[k-1];
			if ( sub==k )
			{
				Columns = 2 ;
			}
    	if ( k-1 >= NUM_SKYCHNOUT )
			{
				sd = (SKYSafetySwData*)&g_model.voiceSwitches[k-1-NUM_SKYCHNOUT];
			}
    	uint8_t attr = (getSwitch00(sd->opt.ss.swtch) ) ? INVERS : 0 ;

    	putsChn(0,y,k,attr);
			if ( k <= numSafety )
			{
  	  	for(uint8_t j=0; j<4;j++)
				{
    		  attr = ((sub==k && subSub==j) ? InverseBlink : 0);
					uint8_t active = (attr && (s_editMode || P1values.p1valdiff)) ;
  	  	  if (j == 0)
					{
						lcd_putsAttIdx( 5*FW, y, XPSTR("\001SAVX"), sd->opt.ss.mode, attr ) ;
	      	  if(active)
						{
	  	        CHECK_INCDEC_H_MODELVAR_0( sd->opt.ss.mode, 3 ) ;
  	  	    }
					}
	    	  else if (j == 1)
  	  	  {
						int8_t max = MaxSwitchIndex ;
						if ( sd->opt.ss.mode == 2 )
						{
							max = MaxSwitchIndex+3 ;
						}	 
						if ( ( sd->opt.ss.swtch > MAX_SKYDRSWITCH ) && ( sd->opt.ss.swtch <= MAX_SKYDRSWITCH + 3 ) )
						{
							lcd_putsAttIdx( 7*FW, y, PSTR(STR_V_OPT1), sd->opt.ss.swtch-MAX_SKYDRSWITCH-1, attr ) ;
						}
						else
						{
         	  	putsDrSwitches(7*FW, y, sd->opt.ss.swtch, attr);
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
							lcd_putsAttIdx(15*FW, y, PSTR(STR_SOUNDS), sd->opt.ss.val,attr);
						}
						else if ( sd->opt.ss.mode == 2 )
						{
							if ( sd->opt.ss.swtch > MAX_SKYDRSWITCH )
							{
								min = 0 ;
								max = NUM_TELEM_ITEMS-1 ;
								sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
  							putsAttIdxTelemItems( 16*FW, y, sd->opt.ss.val+1, attr ) ;
							}
							else
							{
								min = -128 ;
								max = 111 ;
								sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
        				lcd_outdezAtt( 16*FW, y, sd->opt.ss.val+128, attr);
							}
						}
						else
						{
							min = -125 ;
							max = 125 ;
        			lcd_outdezAtt(  15*FW, y, sd->opt.ss.val, attr);
// Option to display current channel value, before limits etc., for failsafe
//        			lcd_outdezAtt(  20*FW, y, ex_chans[k-1]*100/1024, attr) ;
						}
  	  	    if(active)
						{
		          CHECK_INCDEC_H_MODELVAR( sd->opt.ss.val, min,max);
    	  	  }
					}
					else
					{
						if ( sd->opt.ss.mode == 3 )
						{
    					if(sub==k)
							{							
								Columns = 3 ;
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
							if ( event==EVT_KEY_LONG(KEY_MENU))
							{
								if ( sd->opt.vs.vval <= 250 )
								{
									putVoiceQueue( sd->opt.vs.vval | VLOC_NUMUSER  ) ;
								}
      					killEvents(event);
								s_editMode = 0 ;
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

void menuProcSwitches(uint8_t event)  //Issue 78
{
	TITLE(PSTR(STR_CUST_SWITCH));
	EditType = EE_MODEL ;
	Columns = 3 ;
	static MState2 mstate2;
	event = mstate2.check_columns(event, NUM_SKYCSW+1-1-1 ) ;

uint8_t y = 0;
uint8_t k = 0;
int8_t  sub    = mstate2.m_posVert ;
uint8_t subSub = g_posHorz;
    uint8_t t_pgOfs ;

	t_pgOfs = evalOffset(sub);

//  lcd_puts_P( 4*FW, 1*FH,PSTR("Function V1  V2"));
	for(uint8_t i=0; i<7; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
//    if(k==NUM_SKYCSW) break;
    uint8_t attr ;
    SKYCSwData &cs = g_model.customSw[k];

		
    attr = (getSwitch00(CSW_INDEX+k+1) ) ? INVERS : 0 ;
    //write SW names here
//    lcd_putsnAtt( 0*FW , y, PSTR("SW"),2,attr) ;
    lcd_puts_Pleft( y, XPSTR("L") ) ;
//    lcd_putcAtt(  FW-1 , y, 'W', attr) ;
    lcd_putcAtt(  1*FW-1 , y, k + (k>8 ? 'A'-9: '1'), attr) ;
    
		attr = (sub==k ? InverseBlink  : 0);
		lcd_putsAttIdx( 2*FW+1, y, PSTR(CSWITCH_STR),cs.func,subSub==0 ? attr : 0);

    uint8_t cstate = CS_STATE(cs.func);

    if(cstate == CS_VOFS)
    {
			putsChnRaw(    10*FW-6, y, cs.v1  ,subSub==1 ? attr : 0);
//#ifdef FRSKY
      if ( (cs.v1 > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1 < EXTRA_POTS_START ) )
 			{
				int16_t value = convertTelemConstant( cs.v1-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
				putsTelemetryChannel( 18*FW-8, y, cs.v1-CHOUT_BASE-NUM_SKYCHNOUT-1, value, subSub==2 ? attr : 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT);
			}
      else
//#endif
        lcd_outdezAtt( 18*FW-9, y, cs.v2  ,subSub==2 ? attr : 0);
    }
    else if(cstate == CS_VBOOL)
    {
        putsDrSwitches(10*FW-6, y, cs.v1  ,subSub==1 ? attr : 0);
        putsDrSwitches(14*FW-7, y, cs.v2  ,subSub==2 ? attr : 0);
    }
    else if(cstate == CS_VCOMP)
    {
        putsChnRaw(    10*FW-6, y, cs.v1  ,subSub==1 ? attr : 0);
        putsChnRaw(    14*FW-4, y, cs.v2  ,subSub==2 ? attr : 0);
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
      lcd_outdezAtt( 13*FW-5, y, x+1  ,att | (subSub==1 ? attr : 0) ) ;
			att = 0 ;
			x = cs.v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
      lcd_outdezAtt( 18*FW-3, y, x+1 , att | (subSub==2 ? attr : 0 ) ) ;
		}
		else// cstate == CS_TMONO
		{
      putsDrSwitches(10*FW-6, y, cs.v1  ,subSub==1 ? attr : 0);
			uint8_t att = 0 ;
			int8_t x ;
			x = cs.v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
      lcd_outdezAtt( 17*FW-2, y, x+1 , att | (subSub==2 ? attr : 0 ) ) ;
		}
//    lcd_putc( 19*FW+3, y, cs.andsw ? 'S' : '-') ;
		{
			int8_t as ;
			as = cs.andsw ;
			if ( as < 0 )
			{
				as = -as ;
		  	lcd_putcAtt( 18*FW-3, y, '!',(subSub==3 ? attr : 0)) ;
			}
#if defined(PCBSKY) || defined(PCB9XT)
			if ( ( as > 8 ) && ( as <= 9+NUM_SKYCSW+1 ) )
			{
				as += 1 ;				
			}
			if ( as == 9+NUM_SKYCSW+1 )
			{
				as = 9 ;			// Tag TRN on the end, keep EEPROM values
			}
#endif
			putsDrSwitches( 18*FW-3, y, as,(subSub==3 ? attr : 0)) ;
		}


    if((s_editMode || P1values.p1valdiff) && attr)
        switch (subSub) {
        case 0:
            CHECK_INCDEC_H_MODELVAR_0( cs.func, CS_MAXF);
            if(cstate != CS_STATE(cs.func))
            {
                cs.v1  = 0;
                cs.v2 = 0;
            }
            break;
        case 1:
            switch (cstate) {
            case (CS_VCOMP):
            case (CS_VOFS):
#ifdef FRSKY
//#if NUM_EXTRA_POTS
							{
								uint8_t x = mapPots( cs.v1 ) ;
								CHECK_INCDEC_H_MODELVAR( x, 0, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots ) ;
								cs.v1 = unmapPots( x ) ;
							}
//#else
//                CHECK_INCDEC_H_MODELVAR_0( cs.v1, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS);
//#endif
#else
                CHECK_INCDEC_H_MODELVAR_0( cs.v1, NUM_SKYXCHNRAW);
#endif
                break;
            case (CS_VBOOL):
						case CS_TMONO :
                CHECK_INCDEC_MODELSWITCH( cs.v1, -MaxSwitchIndex,MaxSwitchIndex);
                break;
//            case (CS_VCOMP):
//#ifdef FRSKY
//                CHECK_INCDEC_H_MODELVAR_0( event, cs.v1, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS);
//#else
//                CHECK_INCDEC_H_MODELVAR_0( event, cs.v1, NUM_SKYXCHNRAW);
//#endif
//								break;
            case (CS_TIMER):
                CHECK_INCDEC_H_MODELVAR( cs.v1, -50,99);
                break;
						
						default:
                break;
            }
            break;
        case 2:
            switch (cstate) {
            case (CS_VOFS):
						case CS_TMONO :
              CHECK_INCDEC_H_MODELVAR( cs.v2, -125,125) ;
            break ;
            case (CS_VBOOL):
                CHECK_INCDEC_MODELSWITCH( cs.v2, -MaxSwitchIndex,MaxSwitchIndex);
                break;
            case (CS_VCOMP):
//#if NUM_EXTRA_POTS
							{
								uint8_t x = mapPots( cs.v2 ) ;
								CHECK_INCDEC_H_MODELVAR( x, 0, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots ) ;
								cs.v2 = unmapPots( x ) ;
							}
//#else
//                CHECK_INCDEC_H_MODELVAR_0( cs.v2, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS);
//#endif
                break;
            case (CS_TIMER):
                CHECK_INCDEC_H_MODELVAR( cs.v2, -50,99);
                break;

            default:
                break;
            }
            break;
        case 3:
				{	
						int32_t x ;
						x = getAndSwitch( cs ) ;
//						if ( ( x > 8 ) && ( x <= 9+NUM_SKYCSW ) )
//						{
//							x += 1 ;
//						}
//						if ( ( x < -8 ) && ( x >= -(9+NUM_SKYCSW) ) )
//						{
//							x -= 1 ;
//						}
//						if ( x == 9+NUM_SKYCSW+1 )
//						{
//							x = 9 ;			// Tag TRN on the end, keep EEPROM values
//						}
//						if ( x == -(9+NUM_SKYCSW+1) )
//						{
//							x = -9 ;			// Tag TRN on the end, keep EEPROM values
//						}
	          CHECK_INCDEC_MODELSWITCH( x, -MaxSwitchIndex+1,MaxSwitchIndex-1);
						if ( x == 9 )
						{
							x = 9+NUM_SKYCSW+1 ;
						}
						if ( x == -9 )
						{
							x = -(9+NUM_SKYCSW+1) ;
						}
						if ( ( x > 9 ) && ( x <= 9+NUM_SKYCSW+1 ) )
						{
							x -= 1 ;
						}
						if ( ( x < -9 ) && ( x >= -(9+NUM_SKYCSW+1) ) )
						{
							x += 1 ;
						}
						cs.andsw = x ;
//          CHECK_INCDEC_MODELSWITCH( cs.andsw, -MaxSwitchIndex+1,MaxSwitchIndex-1);
				}
				break;
        }
	}
}
//#endif

static int8_t s_currMixIdx;
static uint8_t s_moveMixIdx;
static int8_t s_currDestCh;
static bool   s_currMixInsMode;


void deleteMix(uint8_t idx)
{
#if EXTRA_SKYMIXERS
	while ( idx < MAX_SKYMIXERS+EXTRA_SKYMIXERS-2 )
	{
		*mixAddress(idx) = *mixAddress(idx+1) ;
	}
  memset(&g_model.exmixData[EXTRA_SKYMIXERS-1],0,sizeof(SKYMixData));
#else		
		memmove(&g_model.mixData[idx],&g_model.mixData[idx+1],
            (MAX_SKYMIXERS-(idx+1))*sizeof(SKYMixData));
    memset(&g_model.mixData[MAX_SKYMIXERS-1],0,sizeof(SKYMixData));
#endif
	STORE_MODELVARS;

}

void insertMix(uint8_t idx, uint8_t copy)
{
  SKYMixData *md = mixAddress( idx ) ;

#if EXTRA_SKYMIXERS
	uint8_t xdx ;
	xdx = MAX_SKYMIXERS+EXTRA_SKYMIXERS-1 ;
	while ( xdx > idx )
	{
		*mixAddress(xdx) = *mixAddress(xdx-1) ;
	}

	if ( copy )
	{
		*md = *(md-1) ;
	}
#else
  memmove(md+1,md, (MAX_SKYMIXERS-(idx+1))*sizeof(SKYMixData) );
	if ( copy )
	{
    memmove( md, md-1, sizeof(SKYMixData) ) ;
	}
#endif
	else
	{
   	memset(md,0,sizeof(SKYMixData));
   	md->destCh      = s_currDestCh; //-s_mixTab[sub];
   	md->srcRaw      = s_currDestCh; //1;   //
    md->weight      = 100;
		md->lateOffset  = 1 ;
	}
	s_currMixIdx = idx ;

	
}

void put_curve( uint8_t x, uint8_t y, int8_t idx, uint8_t attr )
{
	if ( idx < 0 )
	{
    lcd_putcAtt( x-FW, y, '!', attr ) ;
		idx = -idx + 6 ;
	}
	lcd_putsAttIdx( x, y,get_curve_string(),idx,attr);
}

//PACK(typedef struct t_voiceAlarm
//{
//  uint8_t source ;
//	uint8_t func;
//  int8_t  swtch ;
//	uint8_t rate ;
//	uint8_t fnameType:2 ;
//	uint8_t spare1:6 ;
//  int16_t offset; 		//offset
//	union
//	{
//		int16_t vfile ;
//		uint8_t name[6] ;
//	} file ;
//}) VoiceAlarmData ;

void copyFileName( char *dest, char *source, uint32_t size )
{
	uint32_t i ;
	for ( i = 0 ; i < size ; i += 1 )
	{
		char c ;
		c = source[i] ;
		if ( c == '.' )
		{
			break ;
		}
		dest[i] = c ;
	}
	while ( i < size )
	{
		dest[i++] = '\0' ;
	}
}

//VoiceAlarmData vad[2] ;

void displayVoiceRate( uint8_t x, uint8_t y, uint8_t rate, uint8_t attr )
{
	if ( ( rate >= 4 ) && ( rate <=32 ) )
	{
    lcd_outdezAtt(x+4*FW,y,rate-2, attr ) ;
	}
	else
	{
		if ( rate > 32 )
		{
			rate -= 29 ;
		}
		lcd_putsAttIdx( x, y, XPSTR("\004  ON OFFBOTH ALLONCE"),rate, attr ) ;
	}
}

// FUnctions need to include ON, OFF and BOTH possibly
void menuProcVoiceOne(uint8_t event)
{
	TITLE( PSTR( STR_Voice_Alarm ));
	lcd_outdezAtt( 13*FW, 0, s_currIdx+1, 0 ) ;
	static MState2 mstate2;
	VoiceAlarmData *pvad = (s_currIdx >= NUM_VOICE_ALARMS) ? &g_model.vadx[s_currIdx - NUM_VOICE_ALARMS] : &g_model.vad[s_currIdx] ;
	uint32_t rows = pvad->fnameType ? 12-1: 11-1 ;
	event = mstate2.check_columns( event, rows ) ;
//	event = mstate2.check(event,0,NULL,0,&Columns,0, rows ) ;
//	columns = 0 ;
	if ( event == EVT_ENTRY_UP )
	{ // From menuProcSelectUvoiceFile
		if ( FileSelectResult == 1 )
		{
			copyFileName( (char *)pvad->file.name, SelectedVoiceFileName, 8 ) ;
	    eeDirty(EE_MODEL) ;		// Save it
		}
	}
	
	int8_t sub = mstate2.m_posVert ;

	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	
//  t_pgOfs = evalOffset(sub) ;
  for(uint8_t i=0; i<=rows; i++)
  {
		uint8_t y = (i+1)*FH;
    uint8_t attr = sub==i ? InverseBlink : 0 ;
//  	uint8_t b ;

		if ( sub < 6 )
		{
			displayNext() ;

	    switch(i)
			{
    	  case 0 :	// source
  	  		lcd_puts_Pleft( FH, XPSTR("Source") ) ;
					putsChnRaw( 16*FW, y, pvad->source, attr ) ;
					if ( attr )
					{
	      		
//#if NUM_EXTRA_POTS
						uint8_t x = mapPots( pvad->source ) ;
						CHECK_INCDEC_H_MODELVAR( x, 0,NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots ) ;
						pvad->source = unmapPots( x ) ;
//#else
//						pvad->source = checkIncDec16( pvad->source, 0, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS, EE_MODEL);
//#endif
					}
					if ( pvad->source )
					{
						int16_t value ;
						value = getValue( pvad->source - 1 ) ;
  	  			lcd_puts_Pleft( FH, XPSTR("\007(\015)") ) ;
						if ( (pvad->source > CHOUT_BASE+NUM_SKYCHNOUT) && ( pvad->source < EXTRA_POTS_START ) )
 						{
							putsTelemetryChannel( 12*FW, FH, pvad->source-CHOUT_BASE-NUM_SKYCHNOUT-1, value, 0, TELEM_NOTIME_UNIT | TELEM_UNIT ) ;
						}
						else
						{
							lcd_outdezAtt( 12*FW, FH, value, 0 ) ;
						}
					}
				break ;

				case 1 :	// func;
  	  		lcd_puts_Pleft( y, XPSTR("Function") ) ;
					lcd_putsAttIdx( 13*FW, y, XPSTR("\007-------v>val  v<val  |v|>val|v|<valv\140=val v=val  "), pvad->func, attr ) ;	// v1>v2  v1<v2  
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->func, 6 ) ;
					}	
				break ;

				case 2 :
				{	
  	  		lcd_puts_Pleft( y, XPSTR("Value") ) ;
		      if (pvad->source > CHOUT_BASE+NUM_SKYCHNOUT)
		 			{
						putsTelemetryChannel( 20*FW, y, pvad->source-CHOUT_BASE-NUM_SKYCHNOUT-1, pvad->offset, attr, TELEM_NOTIME_UNIT | TELEM_UNIT | TELEM_CONSTANT ) ;
					}
					else
					{
						lcd_outdezAtt( FW*20, y, pvad->offset, attr ) ;
					}
					if ( attr )
					{
						pvad->offset = checkIncDec16( pvad->offset, -32000, 32000, EE_MODEL ) ;
					}
				}
				break ;
			
				case 3 :	 // swtch ;
  	  		lcd_puts_Pleft( y, XPSTR("Switch") ) ;
   	  		putsDrSwitches(16*FW, y, pvad->swtch, attr);
	    		if(attr)
					{
      	    CHECK_INCDEC_MODELSWITCH( pvad->swtch, -MaxSwitchIndex, MaxSwitchIndex+1 ) ;
    			}
				break ;

				case 4 :	 // rate ;
  	  		lcd_puts_Pleft( y, XPSTR("Trigger") ) ;
					displayVoiceRate( 16*FW, y, pvad->rate, attr ) ;

//					if ( pvad->rate < 3 )
//					{
//						lcd_putsAttIdx( 16*FW, y, XPSTR("\004  ON OFFBOTH"),pvad->rate,attr);
//					}
//					else if ( pvad->rate < 33 )
//					{
//      	    lcd_outdezAtt(FW*20,y,pvad->rate-2,attr ) ;
//					}
//					else
//					{
//						lcd_putsAttIdx( 16*FW, y, XPSTR("\004ONCE ALL"),pvad->rate-33,attr);
//					}
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->rate, 33 ) ;
					}	
				break ;

				case 5 :	 // haptic ;
  	  		lcd_puts_Pleft( y, PSTR( STR_HAPTIC ) ) ;
					lcd_putsAttIdx( 13*FW, y, XPSTR("\007-------Haptic1Haptic2Haptic3"), pvad->haptic, attr ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->haptic, 3 ) ;
					}
				break ;
			}

		}
		else
		{
			y = (i-5)*FH ;
	    switch(i)
			{
				case 6 :	 // delay
  	  		lcd_puts_Pleft( y, XPSTR("On Delay") ) ;
  				lcd_outdezAtt(FW*18-3,y,pvad->delay,attr|PREC1);
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->delay, 50 ) ;
					}
				break ;
				
				case 7 :	 // vsource:2
  	  		lcd_puts_Pleft( y, XPSTR("Play Source") ) ;
					lcd_putsAttIdx( 14*FW, y, XPSTR("\006No    BeforeAfter "), pvad->vsource, attr ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->vsource, 2 ) ;
					}
				break ;

				case 8 :
  	  		lcd_puts_Pleft( y, XPSTR("On no Telemetry") ) ;
					lcd_putsAttIdx( 17*FW, y, XPSTR("\004PlayMute"), pvad->mute, attr ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->mute, 1 ) ;
					}
				break ;

				case 9 :	 // fnameType:3 ;
				{	
  	  		lcd_puts_Pleft( y, XPSTR("FileType") ) ;
					lcd_putsAttIdx( 14*FW, y, XPSTR("\006------  NameNumber Audio"),pvad->fnameType,attr ) ;
					uint8_t previous = pvad->fnameType ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->fnameType, 3 ) ;
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
			
				case 10 :	 // filename ;
				if ( pvad->fnameType )
				{	
  	  		lcd_puts_Pleft( y, XPSTR("Voice File") ) ;
					if ( pvad->fnameType == 1 )	// Name
					{
						if ( pvad->file.name[0] == 0 )
						{
							memset( (uint8_t *)pvad->file.name, ' ', sizeof(pvad->file.name) ) ;
						}
						// Need to edit name here
						alphaEditName( 12*FW, y, (uint8_t *)pvad->file.name, sizeof(pvad->file.name), attr | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FileName") ) ;
	  				if( attr )
						{
							if (event == EVT_KEY_LONG(KEY_MENU) )
							{
								VoiceFileType = VOICE_FILE_TYPE_USER ;
      				 	pushMenu( menuProcSelectVoiceFile ) ;
  	  					s_editMode = 0 ;
      				  killEvents(event);
							}
						} 
					}
					else if ( pvad->fnameType == 2 )	// Number
					{
	  	  		if(attr)
						{
	  	    		pvad->file.vfile = checkIncDec16( pvad->file.vfile, 0, 515, EE_MODEL);
						}	
						if ( pvad->file.vfile > 500)
						{
							if ( pvad->file.vfile > 507 )
							{
								lcd_putsAtt( 18*FW, y, XPSTR("SC"), attr ) ;
								lcd_putcAtt( 20*FW, y, pvad->file.vfile - 507 + '0', attr ) ;
							}
							else
							{
								dispGvar( 18*FW, y, pvad->file.vfile - 500, attr ) ;
							}
  					}
  					else
						{
	      	    lcd_outdezAtt( FW*20, y, pvad->file.vfile, attr ) ;
  					}
						if (attr)
						{
							if ( event == EVT_KEY_LONG(KEY_MENU) )
							{
								if ( pvad->file.vfile <= 500)
								{
									putVoiceQueue( pvad->file.vfile | VLOC_NUMUSER  ) ;
								}
  							s_editMode = 0 ;
							}
						}
					}
					else if ( pvad->fnameType == 3 )	// Audio
					{
						lcd_putsAttIdx(15*FW, y, PSTR(STR_SOUNDS), pvad->file.vfile, attr ) ;
		  	  	if(attr)
						{
							CHECK_INCDEC_H_MODELVAR( pvad->file.vfile, 0, 15 ) ;
						}
					}
				break ;
				}
				
				case 11 :	 // Blank ;
  	  		lcd_puts_Pleft( y, XPSTR("Delete") ) ;
					lcd_putsAtt( 12*FW, y, XPSTR("MENU LONG"), attr ) ;
  				if( attr )
					{
						if (event == EVT_KEY_LONG(KEY_MENU) )
						{
							pvad->source = 0 ;
							pvad->func = 0 ;
							pvad->swtch = 0 ;
							pvad->rate = 0 ;
							pvad->fnameType  = 0 ;
							pvad->haptic = 0 ;
							pvad->vsource = 0  ;
							pvad->offset = 0 ;
							pvad->delay = 0 ;
							memset( (uint8_t *)pvad->file.name, 0, sizeof(pvad->file.name) ) ;
 	  					s_editMode = 0 ;
							if ( i == 10 )
							{
								mstate2.m_posVert = 8 ;
							}
	    				eeDirty(EE_MODEL) ;
						}
					}
				break ;
			}
		}
	}
}

void menuProcVoiceAlarm(uint8_t event)
{
	TITLE(PSTR(STR_Voice_Alarms)) ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS - 1 ) ;
	EditType = EE_MODEL ;

  int8_t sub = mstate2.m_posVert ;

  evalOffset(sub) ;

  switch (event)
	{
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE) :
      s_currIdx = sub ;
      killEvents(event);
      pushMenu(menuProcVoiceOne) ;
		break;
  }

  uint8_t y = 1*FH ;
  for (uint8_t i = 0 ; i < NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ; i += 1 )
	{
    uint8_t k = i + s_pgOfs ;
    uint8_t attr = sub == k ? INVERS : 0 ;
		VoiceAlarmData *pvad = (k >= NUM_VOICE_ALARMS) ? &g_model.vadx[k - NUM_VOICE_ALARMS] : &g_model.vad[k] ;
      
		if(y>7*FH) break ;
    
		lcd_puts_Pleft( y, XPSTR("VA") ) ;
    lcd_outdezAtt( (k<9) ? FW*3-1 : FW*4-2, y, k+1, 0 ) ;
		putsChnRaw( 5*FW, y, pvad->source, 0 ) ;
		putsDrSwitches( 9*FW, y, pvad->swtch, 0 ) ;
		displayVoiceRate( 13*FW, y, pvad->rate, 0 ) ;
//		if ( pvad->rate < 3 )
//		{
//			lcd_putsAttIdx( 13*FW, y, XPSTR("\004  ON OFFBOTH"),pvad->rate,0 ) ;
//		}
//		else if ( pvad->rate < 33 )
//		{
//      lcd_outdezAtt(FW*17,y,pvad->rate-2, 0 ) ;
//		}
//		else
//		{
//			lcd_putsAttIdx( 13*FW, y, XPSTR("\004ONCE ALL"),pvad->rate-33,attr) ;
//		}
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
		}

		
		if (pvad->haptic)
		{
			lcd_putc( 20*FW, y, 'H' ) ;
		}
		if ( attr )
		{
			lcd_char_inverse( 0, y, 20*FW, 0 ) ;
		}
		
		y += FH ;
  }

}


static uint8_t editSlowDelay( uint8_t y, uint8_t attr, uint8_t value)
{
  if(attr)  value = checkIncDec16( value, 0, 250, EE_MODEL ); //!! bitfield
	uint8_t lval = value ;
//	lcd_putc( FW*16, y, '.' ) ;
//  lcd_outdezAtt(FW*16,y,lval/10,attr);
//  lcd_outdezAtt(FW*18-3,y,lval%10,attr);
  lcd_outdezAtt(FW*18-3,y,lval,attr|PREC1);
	return value ;
}


#if defined(PCBSKY) || defined(PCB9XT)
const char SW_3_IDX[] = "\004sIDxsTHRsRUDsELEsAILsGEAsTRNL1  L2  L3  L4  L5  L6  L7  L8  L9  LA  LB  LC  LD  LE  LF  LG  LH  LI  LJ  LK  LL  LM  LN  LO  6POS" ;
#define NUM_MIX_SWITCHES	(7+NUM_SKYCSW)
#endif
#ifdef PCBX9D
#ifdef REV9E
#define NUM_MIX_SWITCHES	(18+NUM_SKYCSW)
#else
#define NUM_MIX_SWITCHES	(8+NUM_SKYCSW)
#endif	// REV9E
#endif

void putsChnOpRaw( uint8_t x, uint8_t y, SKYMixData *md2, uint8_t attr )
{
	if ( md2->srcRaw == MIX_3POS )
	{
#if defined(PCBSKY) || defined(PCB9XT)
		lcd_putsAttIdx( x, y, SW_3_IDX,md2->switchSource, attr ) ;
#endif
#ifdef PCBX9D
    lcd_putsAttIdx( x, y, PSTR(HW_SWITCHES_STR), md2->switchSource, attr ) ;
#endif
	}
	else
	{
		putsChnRaw( x, y, md2->srcRaw, attr| MIX_SOURCE ) ;
		if ( ( md2->srcRaw >= CHOUT_BASE + 1 ) && ( md2->srcRaw < CHOUT_BASE + NUM_SKYCHNOUT + 1 ) )
		{
			if ( md2->disableExpoDr )
			{
				lcd_putsAtt( x, y, XPSTR("OP"), attr ) ;
			}
		}
	}
}




void menuProcMixOne(uint8_t event)
{
//	static uint8_t edit3Switch ;
	uint8_t numItems = 16 ;
	SKYMixData *md2 = mixAddress( s_currMixIdx ) ;
  
	static MState2 mstate2 ;
	mstate2.check_columns(event, numItems-1 ) ;
//	SUBMENU_NOTITLE( numItems, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0 } ) ;
    uint8_t x = TITLEP( PSTR(STR_EDIT_MIX));

		uint32_t num_mix_switches = NUM_MIX_SWITCHES ;
#ifdef PCBX9D
		if ( g_eeGeneral.analogMapping & MASK_6POS )
		{
			num_mix_switches += 1 ;
		}
#endif
#if defined(PCBSKY) || defined(PCB9XT)
		if ( g_eeGeneral.analogMapping & MASK_6POS )
		{
			num_mix_switches += 1 ;
		}
#endif
		if ( event == EVT_ENTRY )
		{
			RotaryState = ROTARY_MENU_UD ;
//			edit3Switch = 0 ;
		}
		if ( event == EVT_ENTRY_UP )
		{
			SingleExpoChan = 0 ;
		}

		putsChn(x+1*FW,0,md2->destCh,0);
    int8_t  sub    = mstate2.m_posVert;

    evalOffset(sub);

    for(uint8_t k=0; k<7; k++)
    {
        uint8_t y = (k+1) * FH;
        uint8_t i = k + s_pgOfs;
        uint8_t attr = sub==i ? InverseBlink : 0;
    		uint8_t b ;
        switch(i)
				{
        case 0:
				{	
          lcd_puts_P(  2*FW,y,PSTR(STR_2SOURCE));
					uint32_t value = md2->srcRaw ;
					putsChnOpRaw( FW*14,y,md2, attr ) ;
					if ( value == MIX_3POS )
					{
						value += md2->switchSource ;
					}
					else
					{
						if ( value > MIX_3POS )
						{
//#if NUM_EXTRA_POTS
							if ( value < EXTRA_POTS_START )
							{
//#endif	// NUM_EXTRA_POTS
								value += num_mix_switches-1 ;
//#if NUM_EXTRA_POTS
							}
//#endif	// NUM_EXTRA_POTS
						}
					}
					if ( attr )
					{
//#if NUM_EXTRA_POTS
						uint8_t x = mapPots( value ) ;
						CHECK_INCDEC_H_MODELVAR( x, 1,NUM_SKYXCHNRAW+1+MAX_GVARS+1+NUM_SCALERS+8+NumExtraPots + (num_mix_switches-1) );
						value = unmapPots( x ) ;
//#else
//						CHECK_INCDEC_H_MODELVAR( value, 1,NUM_SKYXCHNRAW+1+MAX_GVARS+1+NUM_SCALERS+8 + (num_mix_switches-1) );
//#endif	// NUM_EXTRA_POTS
						if ( value >= MIX_3POS )
						{
							if ( value > (uint32_t)MIX_3POS + (num_mix_switches-1) )
							{
//#if NUM_EXTRA_POTS
								if ( value < EXTRA_POTS_START )
								{
//#endif	// NUM_EXTRA_POTS
									value -= (num_mix_switches-1) ;
//#if NUM_EXTRA_POTS
								}
//#endif	// NUM_EXTRA_POTS
							}
							else
							{
								md2->switchSource = value - MIX_3POS ;
								value = MIX_3POS ;
							}
						}
						md2->srcRaw = value ;
					}
						
  				if ( (attr) && ( event == EVT_KEY_LONG(KEY_MENU) ) )
					{
						if ( ( md2->srcRaw) && ( md2->srcRaw <= 4 ) )
						{
							SingleExpoChan = 1 ;
							s_expoChan = md2->srcRaw-1 ;
							s_editMode = 0 ;
        			killEvents(event);
        			pushMenu(menuProcExpoAll);
						}
					}
				}
        break ;
        case 1:
            lcd_puts_P(  2*FW,y,PSTR(STR_2WEIGHT));
#if GVARS
						md2->weight = gvarMenuItem( FW*16, y, md2->weight, -125, 125, attr, event ) ;
#else
            lcd_outdezAtt(FW*13,y,md2->weight,attr|LEFT);
            if(attr) CHECK_INCDEC_H_MODELVAR( md2->weight, -125,125);
#endif
            break;
        case 2:
//            lcd_puts_P(  2*FW,y,md2->enableFmTrim ? PSTR(STR_FMTRIMVAL) : PSTR(STR_OFFSET));
            lcd_puts_P( FW, y, PSTR(STR_OFFSET) ) ;
#if GVARS
						md2->sOffset = gvarMenuItem( FW*16, y, md2->sOffset, -125, 125, attr, event ) ;
#else
						lcd_outdezAtt(FW*13,y,md2->sOffset,attr|LEFT);
            if(attr) CHECK_INCDEC_H_MODELVAR( md2->sOffset, -125,125);
#endif
            break;
        case 3:
						md2->lateOffset = onoffMenuItem( md2->lateOffset, y, PSTR(STR_2FIX_OFFSET), attr ) ;
            break;
        case 4:
						if ( ( md2->srcRaw <=4 ) )
						{
    					lcd_puts_Pleft( y, PSTR(STR_ENABLEEXPO) ) ;
							md2->disableExpoDr = offonItem( md2->disableExpoDr, y, attr ) ;
//							md2->disableExpoDr = offonMenuItem( md2->disableExpoDr, y, PSTR(STR_ENABLEEXPO), attr ) ;
						}
						else
						{
							md2->disableExpoDr = onoffMenuItem( md2->disableExpoDr, y, XPSTR("\001Use Output   "), attr ) ;
						}
//						b = md2->enableFmTrim ;
//            lcd_puts_P(  2*FW,y,PSTR(STR_FLMODETRIM));
	//					lcd_putsnAtt(FW*13,y, PSTR("OFFON ")+3*b,3,attr);  //default is 0=OFF
            //            lcd_putsnAtt( x, y, PSTR("OFFON ")+3*value,3,mode ? INVERS:0) ;
            //            menu_lcd_onoff( FW*13, y, md2->enableFmTrim, sub==i ) ;
  //          if(attr) { CHECK_INCDEC_H_MODELVAR( event, b, 0,1); md2->enableFmTrim = b ; }
            break;
        case 5:
						md2->carryTrim = offonMenuItem( md2->carryTrim, y, PSTR(STR_2TRIM), attr ) ;
//            lcd_puts_P(  2*FW,y,PSTR(STR_2TRIM));
//            lcd_putsnAtt(FW*13,y, PSTR("ON OFF")+3*md2->carryTrim,3,attr);  //default is 0=ON
//            if(attr) CHECK_INCDEC_H_MODELVAR( event, md2->carryTrim, 0,1);
            break;
        
				case 6:
					{	
					 	uint8_t value = md2->differential ;
						if ( value == 0 )
						{
							if ( md2->curve <= -28 )
							{
								value = 2 ;		// Expo
							}
						}
					 	uint8_t value2 = value ;
	          lcd_putsAtt(  1*FW, y, value ? ( value == 2 ) ? XPSTR("\021Expo") : PSTR(STR_15DIFF) : PSTR(STR_Curve), attr ) ;
    		    if(attr) CHECK_INCDEC_H_MODELVAR_0( value2, 2) ;
					 	if ( value != value2 )
						{
							if ( value2 == 2 )
							{
								md2->curve = -128 ;
							}
							else
							{
								md2->curve = 0 ;
							}
							md2->differential = value2 & 1 ;	// 0 and 2 turn it off
						}
					}
        break ;

        case 7:
						if ( md2->differential )		// Non zero for curve
						{	
		          md2->curve = gvarMenuItem( 12*FW, y, md2->curve, -100, 100, attr /*( m_posHorz==1 ? attr : 0 )*/, event ) ;
						}
						else
						{
							if ( md2->curve <= -28 )
							{
								int8_t value = md2->curve + 128 ;	// 0 to 100
            		lcd_outdezAtt(FW*17,y,value,attr|LEFT);
            		if(attr) CHECK_INCDEC_H_MODELVAR_0( value, 100 ) ;
								md2->curve = value - 128 ;
							}
							else
							{
								put_curve( 2*FW, y, md2->curve, attr ) ;
          	  	if(attr)
								{
        		  		CHECK_INCDEC_H_MODELVAR( md2->curve, -MAX_CURVE5-MAX_CURVE9-1-1-1, MAX_CURVE5+MAX_CURVE9+7-1+1+1+1);
									if ( event==EVT_KEY_FIRST(KEY_MENU) || event == EVT_KEY_BREAK(BTN_RE)  )
									{
										if ( md2->curve>=CURVE_BASE )
										{
          	  	    	s_curveChan = md2->curve-CURVE_BASE;
	          		      pushMenu(menuProcCurveOne);
										}
										if ( md2->curve < 0 )
										{
          	  	    	s_curveChan = -md2->curve-1 ;
	          		      pushMenu(menuProcCurveOne);
										}
									}
          	  	}
        		  }
						}
        break ;

        case 8:
            lcd_puts_P(  2*FW,y,PSTR(STR_2SWITCH));
            putsDrSwitches(13*FW,  y,md2->swtch,attr);
            if(attr) CHECK_INCDEC_MODELSWITCH( md2->swtch, -MaxSwitchIndex, MaxSwitchIndex);
            break;

        case 9:
					{	
						uint8_t b = 1 ;
            lcd_puts_Pleft( y,XPSTR("\001MODES"));
						
						if ( attr )
						{
							Columns = 6 ;
						}
  					
						for ( uint8_t p = 0 ; p<MAX_MODES+1 ; p++ )
						{
							uint8_t z = md2->modeControl ;
    					lcd_putcAtt( (9+p)*(FW+1), y, '0'+p, ( z & b ) ? 0 : INVERS ) ;
							if( attr && ( g_posHorz == p ) )
							{
								lcd_rect( (9+p)*(FW+1)-1, y-1, FW+2, 9 ) ;
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
            lcd_puts_P(  2*FW,y,PSTR(STR_2WARNING));
						b = md2->mixWarn ;
            if(b)
                lcd_outdezAtt(FW*13,y,b,attr|LEFT);
            else
                lcd_putsAtt(  FW*13,y,PSTR(STR_OFF),attr);
            if(attr) { CHECK_INCDEC_H_MODELVAR_0( b, 3); md2->mixWarn = b ; }
            break;
        case 11:
            lcd_puts_P(  2*FW,y,PSTR(STR_2MULTIPLEX));
            lcd_putsAttIdx(13*FW, y,PSTR(STR_ADD_MULT_REP),md2->mltpx,attr);
            if(attr) CHECK_INCDEC_H_MODELVAR_0( md2->mltpx, 2); //!! bitfield
            break;
        case 12:
            lcd_puts_P(  2*FW,y,PSTR(STR_2DELAY_DOWN));
						md2->delayUp = editSlowDelay( y, attr, md2->delayUp ) ;	// Sense was wrong
            break;
        case 13:
            lcd_puts_P(  2*FW,y,PSTR(STR_2DELAY_UP));
						md2->delayDown = editSlowDelay( y, attr, md2->delayDown ) ;	// Sense was wrong
            break;
        case 14:
            lcd_puts_P(  2*FW,y,PSTR(STR_2SLOW_DOWN));
						md2->speedDown = editSlowDelay( y, attr, md2->speedDown ) ;
            break;
        case 15:
            lcd_puts_P(  2*FW,y,PSTR(STR_2SLOW_UP));
						md2->speedUp = editSlowDelay( y, attr, md2->speedUp ) ;
            break;
//#ifdef PCBX9D
//				case 16:
//            lcd_puts_P( 2*FW,y,XPSTR("3POS Switch"));
//            lcd_putsAttIdx(15*FW, y,XPSTR("\002SASBSCSDSESFSGSH"),md2->switchSource,attr);
//            if(attr) CHECK_INCDEC_H_MODELVAR_0( event, md2->switchSource, 7) ;
//				break ;
//#endif
//#ifdef PCBSKY
//				case 16:
//            lcd_puts_P( 2*FW,y,XPSTR("3POS Switch"));
//						{
//            	lcd_putsAttIdx(15*FW, y,XPSTR("\003IDxTHRRUDELEAILGEATRN"),md2->switchSource,attr);
//							if(attr) CHECK_INCDEC_H_MODELVAR_0( event, md2->switchSource, 6) ;
							
//							uint32_t number = 0 ;
//							uint8_t sm = g_eeGeneral.switchMapping ;
//							uint8_t text[20] ;
//							uint8_t *p ;
//							p = cpystr( text, (uint8_t *)"\003IDx" ) ;
//							if ( sm & USE_ELE_3POS )
//							{
//								p = cpystr( p, (uint8_t *)"ELE" ) ;
//								number += 1 ;
//							}
//							if (sm & USE_THR_3POS )
//							{
//								p = cpystr( p, (uint8_t *)"THR" ) ;
//								number += 1 ;
//							}
//							if (sm & USE_RUD_3POS )
//							{
//								p = cpystr( p, (uint8_t *)"RUD" ) ;
//								number += 1 ;
//							}
//							if ( sm & USE_GEA_3POS )
//							{
//								p = cpystr( p, (uint8_t *)"GEA" ) ;
//								number += 1 ;
//							}
//							if ( sm & USE_AIL_3POS )
//							{
//								p = cpystr( p, (uint8_t *)"AIL" ) ;
//								number += 1 ;
//							}
//          	  lcd_putsAttIdx(15*FW, y, (char*)text, md2->switchSource, attr ) ;
//        	    if(attr) CHECK_INCDEC_H_MODELVAR_0( event, md2->switchSource, number) ;
//						}	
//				break ;
//#endif
        }
    }
}

int8_t s_mixMaxSel;

static void memswap( void *a, void *b, uint8_t size )
{
    uint8_t *x ;
    uint8_t *y ;
    uint8_t temp ;

    x = (unsigned char *) a ;
    y = (unsigned char *) b ;
    while ( size-- )
    {
        temp = *x ;
        *x++ = *y ;
        *y++ = temp ;
    }
}

void moveMix(uint8_t idx, uint8_t dir) //true=inc=down false=dec=up - Issue 49
{
		SKYMixData *src = mixAddress( idx ) ;
    if(idx==0 && !dir)
		{
      if (src->destCh>0)
			{
			  src->destCh--;
			}
			STORE_MODELVARS;
			return ;
		}

#if EXTRA_SKYMIXERS
    if(idx>MAX_SKYMIXERS+EXTRA_SKYMIXERS || (idx==MAX_SKYMIXERS+EXTRA_SKYMIXERS && dir)) return ;
#else
    if(idx>MAX_SKYMIXERS || (idx==MAX_SKYMIXERS && dir)) return ;
#endif
    uint8_t tdx = dir ? idx+1 : idx-1;
		SKYMixData *tgt = mixAddress( tdx ) ;

    if((src->destCh==0) || (src->destCh>NUM_SKYCHNOUT+EXTRA_SKYCHANNELS) || (tgt->destCh>NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)) return;

    if(tgt->destCh!=src->destCh)
		{
        if ((dir)  && (src->destCh<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)) src->destCh++;
        if ((!dir) && (src->destCh>0))          src->destCh--;
				STORE_MODELVARS;
        return;
    }

    //flip between idx and tgt
    memswap( tgt, src, sizeof(SKYMixData) ) ;
		s_moveMixIdx = tdx ;
    
		STORE_MODELVARS;
//    eeWaitComplete() ;
}

void menuMixersLimit(uint8_t event)
{
    switch(event)
    {
    case  EVT_KEY_FIRST(KEY_EXIT):
        killEvents(event);
        popMenu(true);
        pushMenu(menuProcMix);
        break;
    }
    lcd_puts_Pleft(2*FH, PSTR(STR_MAX_MIXERS));
    lcd_outdezAtt(20*FW, 2*FH, getMixerCount(),0);

    lcd_puts_Pleft(4*FH, PSTR(STR_PRESS_EXIT_AB));
}

uint8_t getMixerCount()
{
    uint8_t mixerCount = 0;
    uint8_t dch ;

#if EXTRA_SKYMIXERS
    for(uint8_t i=0;i<MAX_SKYMIXERS+EXTRA_SKYMIXERS;i++)
#else		
		for(uint8_t i=0;i<MAX_SKYMIXERS;i++)
#endif
    {
        dch = mixAddress( i )->destCh ;
        if ((dch!=0) && (dch<=NUM_SKYCHNOUT+EXTRA_SKYCHANNELS))
        {
            mixerCount++;
        }
    }
    return mixerCount;
}

bool reachMixerCountLimit()
{
    // check mixers count limit
#if EXTRA_SKYMIXERS
    if (getMixerCount() >= MAX_SKYMIXERS+EXTRA_SKYMIXERS)
#else
    if (getMixerCount() >= MAX_SKYMIXERS)
#endif
    {
        pushMenu(menuMixersLimit);
        return true;
    }
    else
    {
        return false;
    }
}

uint8_t mixToDelete;

#define YN_NONE	0
#define YN_YES	1
#define YN_NO		2

uint8_t yesNoMenuExit( uint8_t event, const char * s )
{
	uint8_t reply = YN_NONE ;
	lcd_puts_Pleft(1*FH, s ) ;	
  lcd_puts_Pleft( 5*FH,PSTR(STR_YES_NO));
  lcd_puts_Pleft( 6*FH,PSTR(STR_MENU_EXIT));

  switch(event)
	{
    case EVT_ENTRY:
      audioDefevent(AU_WARNING1);
    break ;

    case EVT_KEY_FIRST(KEY_MENU):
		case EVT_KEY_BREAK(BTN_RE):
			reply = YN_YES ;
    break ;

		case EVT_KEY_LONG(BTN_RE):
    case EVT_KEY_FIRST(KEY_EXIT):
			reply = YN_NO ;
    break;
  }
	if ( reply != YN_NONE )
	{
		killEvents(event);
    popMenu(false);
//    popMenu(true);
	}
	return reply ;
}


void menuDeleteMix(uint8_t event)
{
	uint8_t action ;
	action = yesNoMenuExit( event, (mixToDelete== 0xFF) ? XPSTR("Clear ALL mixes?") : PSTR(STR_DELETE_MIX) ) ;
  
	switch( action )
	{
    case YN_YES :
			if (mixToDelete == 0xFF)
			{
        clearMixes() ;
			}
			else
			{
      	deleteMix(mixToDelete);
			}	 
      //fallthrough
		case YN_NO :
//      pushMenu(menuProcMix);
    break;
  }
}

uint8_t s_moveMode;

int8_t qRotary()
{
	int8_t diff = 0 ;

	if ( Rotary_diff > 0)
	{
		diff = 1 ;
	}
	else if ( Rotary_diff < 0)
	{
		diff = -1 ;
	}
	Rotary_diff = 0 ;
	return diff ;
}

#define POPUP_NONE			0
#define POPUP_SELECT		1
#define POPUP_EXIT			2

static uint8_t popupProcess( uint8_t event, uint8_t max )
{
	int8_t popidxud = qRotary() ;
	uint8_t popidx = PopupData.PopupIdx ;

	if ( PopupData.PopupTimer )
	{
		if ( BLINK_ON_PHASE )
		{
			if ( --PopupData.PopupTimer == 0 )
			{
				// Timeout popup
				PopupData.PopupActive = 0 ;
				return POPUP_EXIT ;
			}
		}
	}
	else
	{
		PopupData.PopupTimer = 255 ;
  }

  event = Tevent ;
	switch(event)
	{
    case EVT_KEY_BREAK(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE):
			PopupData.PopupActive = 0 ;
			PopupData.PopupTimer = 0 ;
		return POPUP_SELECT ;
    
		case EVT_KEY_FIRST(KEY_UP) :
			popidxud = -1 ;
		break ;
    
		case EVT_KEY_FIRST(KEY_DOWN) :
			popidxud = 1 ;
		break ;
    
		case EVT_KEY_FIRST(KEY_EXIT) :
		case EVT_KEY_LONG(BTN_RE) :
			killEvents( event ) ;
			Tevent = 0 ;
			PopupData.PopupActive = 0 ;
			PopupData.PopupTimer = 0 ;
		return POPUP_EXIT ;
	}

	if (popidxud > 0)
	{
		if ( popidx < max )
		{
			popidx += 1 ;
		}
		else
		{
			popidx = 0 ;			
		}
	}
	else if (popidxud < 0)
	{		
		if ( popidx )
		{
			popidx -= 1 ;
		}
		else
		{
			popidx = max ;
		}
	}

	if (popidxud )
	{
		if ( PopupData.PopupTimer )
		{
			PopupData.PopupTimer = 255 ;
		}
	}	

	PopupData.PopupIdx = popidx ;
	return POPUP_NONE ;
}

//const char *MixPopList = PSTR(STR_MIX_POPUP) ;

static uint32_t popupDisplay( const char *list, uint16_t mask, uint8_t width )
{
	uint32_t entries = 0 ;
	uint8_t y = FH ;

	while ( mask )
	{
		if ( mask & 1 )
		{
			lcd_putsn_P( 3*FW, y, "              ", width ) ;
			lcd_puts_P( 4*FW, y, (const char *)(list) ) ;
			entries += 1 ;
			y += FH ;
		}
		mask >>= 1 ;
		while ( *list )
		{
			list += 1 ;			
		}		
		list += 1 ;			
	}
//	plotType = PLOT_BLACK ;
	if ( y >= 7*FH )
	{
		y -= 1 ;
	}
	lcd_rect( 3*FW, 1*FH-1, width*FW, y+2-FH ) ;
//	plotType = PLOT_XOR ;
	lcd_char_inverse( 4*FW, (PopupData.PopupIdx+1)*FH, (width-2)*FW, 0 ) ;

	return entries ;
}

static uint8_t popTranslate( uint8_t popidx, uint16_t mask )
{
	uint8_t position ;
	popidx += 1 ;
	for ( position = 0 ; position < 16 ; position += 1 )
	{
		if ( mask & 1 )
		{
			if ( --popidx == 0)
			{
				break ;
			}
		}
		mask >>= 1 ;
	}
	return position ;
}


uint32_t doPopup( const char *list, uint16_t mask, uint8_t width, uint8_t event )
{
	CPU_UINT count = popupDisplay( list, mask, width ) ;
	uint8_t popaction = popupProcess( event, count - 1 ) ;
	uint8_t popidx = PopupData.PopupIdx ;
	PopupData.PopupSel = popTranslate( popidx, mask ) ;
	return popaction ;
}

void mixpopup( uint8_t event )
{
	uint8_t popaction = doPopup( PSTR(STR_MIX_POPUP), 0x7F, 11, event ) ;
//	popupDisplay( PSTR(STR_MIX_POPUP), 0x1F, 8 ) ;
	
//	uint8_t popaction = popupProcess( event, 4 ) ;
//	uint8_t popidx = PopupIdx ;
//	lcd_char_inverse( 4*FW, (popidx+1)*FH, 6*FW, 0 ) ;

  if ( popaction == POPUP_SELECT )
	{
		uint8_t popidx = PopupData.PopupSel ;
		if ( popidx == 1 )
		{
      if ( !reachMixerCountLimit())
      {
				s_currMixInsMode = 1 ;
      	insertMix(++s_currMixIdx, 0 ) ;
  	    s_moveMode=false;
			}
		}
		if ( popidx < 2 )
		{
	    pushMenu(menuProcMixOne) ;
		}
		else if ( popidx == 4 )		// Delete
		{
			mixToDelete = s_currMixIdx;
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
			pushMenu( menuProcTemplates ) ;
		}
		else
		{
			if( popidx == 2 )	// copy
			{
     		insertMix(++s_currMixIdx, 1 ) ;
			}
			// PopupIdx == 2 or 3, copy or move
			s_moveMode = 1 ;
		}
		PopupData.PopupActive = 0 ;
	}
//	else if ( popaction == POPUP_EXIT )
//	{
//		PopupData.PopupActive = 0 ;
//		killEvents( event ) ;
//	}
	s_moveMixIdx = s_currMixIdx ;

//	if ( Tevent )
//	{
//  	killEvents(Tevent);
//	}

}



void menuProcMix(uint8_t event)
{
	TITLE(PSTR(STR_MIXER));
	EditType = EE_MODEL ;
	static MState2 mstate2;
	
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
				 || ( ( v == s_mixMaxSel ) && ( event == EVT_KEY_FIRST(KEY_DOWN) ) ) )
		{
			event = 0 ;
		}
		Tevent = event ;
	}
	
	if ( !PopupData.PopupActive )
	{
//		if (SubMenuFromIndex)
//		{
			mstate2.check_columns(event,s_mixMaxSel) ;
//			mstate2.check_simple(event,0,NULL,0,s_mixMaxSel+1) ;
//		}
//		else
//		{
//			mstate2.check_simple(event,e_Mix,menuTabModel,DIM(menuTabModel),s_mixMaxSel+1) ;
//		}
	}
//	uint8_t save_event = event ;

  int8_t  sub    = mstate2.m_posVert;
	int8_t	menulong = 0 ;

    switch(event)
    {
	    case EVT_ENTRY:
        s_moveMode=false;
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
//    		  if(sub<1) break;
					menulong = 1 ;
				}
  	  break;
    }

//  if(sub==0) s_moveMode = false;
	uint8_t t_pgOfs = evalOffset( sub ) ;
    
	if ( PopupData.PopupActive )
	{
		Tevent = 0 ;
	}
		
  uint8_t mix_index = 0 ;
  uint8_t current = 0 ;

	if ( s_moveMode )
	{
		int8_t dir ;
		
		if ( ( dir = (event == EVT_KEY_FIRST(KEY_DOWN) ) ) || event == EVT_KEY_FIRST(KEY_UP) )
		{
			moveMix( s_currMixIdx, dir ) ; //true=inc=down false=dec=up - Issue 49
		}
	}

  for ( uint8_t chan=1 ; chan <= NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ; chan += 1 )
	{
    SKYMixData *pmd = mixAddress( mix_index ) ;
    
    if ( t_pgOfs <= current && current-t_pgOfs < 7)
		{
      putsChn(1, (current-t_pgOfs+1)*FH, chan, 0) ; // show CHx
    }

		uint8_t firstMix = mix_index ;
		
#if EXTRA_SKYMIXERS
		if (mix_index < MAX_SKYMIXERS + EXTRA_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)
#else
		if (mix_index<MAX_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)
#endif
		{
    	do
			{
				if (t_pgOfs <= current )
				{
					if ( current-t_pgOfs < 7 )
					{
    	  	  uint8_t y = (current-t_pgOfs+1)*FH ;
    				uint8_t attr = 0 ;

						if ( !s_moveMode && (sub == current) )
						{
							s_currMixIdx = mix_index ;
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
        	  if(firstMix != mix_index) //show prefix only if not first mix
        	 		lcd_putsAttIdx( 4*FW-5, y, XPSTR("\001+*R"),pmd->mltpx,0 ) ;

						putsChnOpRaw( 8*FW, y, pmd, 0 ) ;
extern uint8_t swOn[] ;
						if ( swOn[mix_index] )
						{
							attr |= BOLD ;
						}
	#if GVARS
						pmd->weight = gvarMenuItem( 7*FW+FW/2, y, pmd->weight, -125, 125, attr, event ) ;
	#else
						lcd_outdezAtt(  7*FW+FW/2, y, pmd->weight, attr ) ; //attr);
	#endif
//						if ( attr )
//						{
//							lcd_char_inverse( 0, y, 12*FW, 0 ) ;
//						}

//    	  	  lcd_putcAtt(    7*FW+FW/2, y, '%', 0 ) ; //tattr);
    	  	  if( pmd->swtch) putsDrSwitches( 12*FW, y, pmd->swtch, 0 ) ; //tattr);
						if(pmd->curve)
						{
							if ( pmd->differential ) lcd_putcAtt(    16*FW, y, *PSTR(CHR_d), 0 ) ;
							else
							{
								if ( ( pmd->curve > -28 ) && ( pmd->curve <= 27 ) )
								{
	    	  	  		put_curve( 16*FW, y, pmd->curve, 0 ) ;
								}
								else
								{
									lcd_putcAtt( 16*FW, y, 'E', 0 ) ;
								}
							}
						}
						char cs = ' ';
        	  if (pmd->speedDown || pmd->speedUp)
        	    cs = *PSTR(CHR_S);
        	  if (pmd->delayUp || pmd->delayDown)
        	    cs = (cs == *PSTR(CHR_S) ? '*' : *PSTR(CHR_D));
        	  lcd_putc(20*FW+1, y, cs ) ;

						if ( s_moveMode )
						{
							if ( s_moveMixIdx == mix_index )
							{
								lcd_char_inverse( 4*FW, y, 17*FW, 0 ) ;
								s_currMixIdx = mix_index ;
								sub = mstate2.m_posVert = current ;
							}
						}
					}
					else
					{
						if ( current-t_pgOfs == 7 )
						{
							if ( s_moveMode )
							{
								if ( s_moveMixIdx == mix_index )
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
#if EXTRA_SKYMIXERS
    	} while ( (mix_index<MAX_SKYMIXERS+EXTRA_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)) ;
#else
    	} while ( (mix_index<MAX_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)) ;
#endif
		}
		else
		{
			if (sub == current)
			{
				s_currDestCh = chan ;		// For insert
				s_currMixIdx = mix_index ;
				lcd_rect( 0, (current-t_pgOfs+1)*FH-1, 25, 9 ) ;
//				s_moveMode = 0 ;		// Can't move this
				if ( menulong )		// Must need to insert here
				{
      		if ( !reachMixerCountLimit())
      		{
//						s_currMixInsMode = 1 ;
      			insertMix(s_currMixIdx, 0 ) ;
  	    		s_moveMode=false;
	      		pushMenu(menuProcMixOne) ;
						break ;
      		}
				}
			}
			current += 1 ;
		}
	}
  if ( t_pgOfs <= current && (current-t_pgOfs) < 7)
	{
   	lcd_puts_Pleft( 7*FH,XPSTR("Templates\020MENU") ) ;
		if (sub == s_mixMaxSel )
		{
			lcd_char_inverse( 0, 7*FH, 21*FW, 0 ) ;
			if ( event == EVT_KEY_FIRST(KEY_MENU) || event == EVT_KEY_BREAK(BTN_RE) )
			{
				pushMenu( menuProcTemplates ) ;
			}
		}
//		y += FH ;
//		if ( y < 8 * FH )
//		{
//   		lcd_puts_Pleft( y,XPSTR("Clear Mixes\020MENU") ) ;
//			if (sub == s_mixMaxSel + 1 )
//			{
//				lcd_char_inverse( 0, y, 21*FW, 0 ) ;
//			}
//		}
	}
	
	if ( PopupData.PopupActive )
	{
		Tevent = event ;
		mixpopup( event ) ;
    s_editMode = false;
	}
	s_mixMaxSel = current ;

}


uint16_t expou(uint16_t x, uint16_t k)
{
    // k*x*x*x + (1-k)*x
    return ((unsigned long)x*x*x/0x10000*k/(RESXul*RESXul/0x10000) + (RESKul-k)*x+RESKul/2)/RESKul;
}
// expo-funktion:
// ---------------
// kmplot
// f(x,k)=exp(ln(x)*k/10) ;P[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]
// f(x,k)=x*x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=1+(x-1)*(x-1)*(x-1)*k/10 + (x-1)*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]

int16_t expo(int16_t x, int16_t k)
{
    if(k == 0) return x;
    int32_t   y;
    bool    neg =  x < 0;
    if(neg)   x = -x;
    if(k<0){
        y = RESXu-expou(RESXu-x,-k);
    }else{
        y = expou(x,k);
    }
    return neg? -y:y;
}


#ifdef EXTENDED_EXPO
/// expo with y-offset
class Expo
{
    uint16_t   c;
    int16_t    d,drx;
public:
    void     init(uint8_t k, int8_t yo);
    static int16_t  expou(uint16_t x,uint16_t c, int16_t d);
    int16_t  expo(int16_t x);
};
void    Expo::init(uint8_t k, int8_t yo)
{
    c = (uint16_t) k  * 256 / 100;
    d = (int16_t)  yo * 256 / 100;
    drx = d * ((uint16_t)RESXu/256);
}
int16_t Expo::expou(uint16_t x,uint16_t c, int16_t d)
{
    uint16_t a = 256 - c - d;
    if( (int16_t)a < 0 ) a = 0;
    // a x^3 + c x + d
    //                         9  18  27        11  20   18
    uint32_t res =  ((uint32_t)x * x * x / 0x10000 * a / (RESXul*RESXul/0x10000) +
                     (uint32_t)x                   * c
                     ) / 256;
    return (int16_t)res;
}
int16_t  Expo::expo(int16_t x)
{
    if(c==256 && d==0) return x;
    if(x>=0) return expou(x,c,d) + drx;
    return -expou(-x,c,-d) + drx;
}
#endif


void editExpoVals(uint8_t event, uint8_t edit, uint8_t x, uint8_t y, uint8_t which, uint8_t exWt, uint8_t stkRL)
{
    uint8_t  invBlk = (edit) ? InverseBlink : 0 ;
		uint8_t doedit ;
		int8_t *ptr ;			// volatile forces compiler to produce 'better' code
		ExpoData *eptr ;

//		doedit = (edit || (P1values.p1valdiff ) ) ;
		doedit = edit ? EDIT_DR_SWITCH_EDIT : 0 ;

		eptr = &g_model.expoData[s_expoChan] ;
    
		if(which==DR_DRSW1) {
				eptr->drSw1 = edit_dr_switch( x, y, eptr->drSw1, invBlk, doedit, event ) ;
    }
    else if(which==DR_DRSW2) {
				eptr->drSw2 = edit_dr_switch( x, y, eptr->drSw2, invBlk, doedit, event ) ;
    }
    else
		{
				ptr = &eptr->expo[which][exWt][stkRL] ;
        if(exWt==DR_EXPO)
				{
            
#if GVARS
					*ptr = gvarMenuItem( x, y, *ptr, -100, 100, invBlk, event ) ;
#else
					lcd_outdezAtt(x, y, *ptr, invBlk);
          if(doedit) CHECK_INCDEC_H_MODELVAR(*ptr,-100, 100);
#endif
        }
        else
				{
#if GVARS
					*ptr = gvarMenuItem( x, y, *ptr+100, 0, 100, invBlk, event ) - 100 ;
#else
          lcd_outdezAtt(x, y, *ptr+100, invBlk);
          if(doedit) CHECK_INCDEC_H_MODELVAR(*ptr,-100, 0);
#endif
        }
		}
}

void menuProcExpoAll(uint8_t event)
{
	TITLE(PSTR(STR_EXPO_DR));
	EditType = EE_MODEL ;
	static MState2 mstate2;
//	static const uint8_t mstate_tab[] = {0} ;
	
//	if (SubMenuFromIndex)
//	{
//		event = mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1, 6-1 ) ;
		uint32_t count = 5-1 ;
		if ( SingleExpoChan )
		{
			count -= 1 ;
		}
		event = mstate2.check_columns( event, count ) ;
//	}
//	else
//	{
//		event = mstate2.check(event,e_ExpoAll,menuTabModel,DIM(menuTabModel),mstate_tab,DIM(mstate_tab)-1, 6-1 ) ;
////  	MENU(PSTR(STR_EXPO_DR), menuTabModel, e_ExpoAll, 6, {0} ) ;
//	}
	
	uint8_t stkVal ;
	int8_t  sub = mstate2.m_posVert ;
	int8_t subN ;
	if ( SingleExpoChan )
	{
		sub += 1 ;
	}
	if( sub )
	{
		StickScrollAllowed = 0 ;
	}

	uint8_t l_expoChan = s_expoChan ;
	subN = 0 ;
	{
    uint8_t attr = 0 ;
		if ( sub == subN )
		{
			if ( SingleExpoChan == 0 )
			{
				s_expoChan = l_expoChan = checkIncDec( s_expoChan, 0, 3, 0 ) ;
				attr = InverseBlink ;
			}
		}		 
		putsChnRaw(0,FH,l_expoChan+1,attr) ;
	}

	uint8_t expoDrOn = get_dr_state(l_expoChan);
	switch (expoDrOn)
	{
    case DR_MID:
      lcd_puts_Pleft( FH,PSTR(STR_4DR_MID));
    break;
    case DR_LOW:
      lcd_puts_Pleft( FH,PSTR(STR_4DR_LOW));
    break;
    default: // DR_HIGH:
      lcd_puts_Pleft( FH,PSTR(STR_4DR_HI));
    break;
	}

	subN += 1 ;
	stkVal = DR_BOTH ;
	if(calibratedStick[l_expoChan]> 100) stkVal = DR_RIGHT;
	if(calibratedStick[l_expoChan]<-100) stkVal = DR_LEFT;
	if(IS_EXPO_THROTTLE(l_expoChan)) stkVal = DR_RIGHT;

	lcd_puts_Pleft(2*FH,PSTR(STR_2EXPO));
	editExpoVals( event, (stkVal != DR_RIGHT) && (sub==subN), 4*FW, 3*FH, expoDrOn ,DR_EXPO, DR_LEFT ) ;
	editExpoVals( event, (stkVal != DR_LEFT) && (sub==subN), 8*FW, 3*FH, expoDrOn ,DR_EXPO, DR_RIGHT ) ;

	subN += 1 ;
	lcd_puts_Pleft(4*FH,PSTR(STR_2WEIGHT));
	editExpoVals( event, (stkVal != DR_RIGHT) && (sub==subN), 4*FW, 5*FH, expoDrOn ,DR_WEIGHT, DR_LEFT ) ;
	editExpoVals( event, (stkVal != DR_LEFT) && (sub==subN), 8*FW, 5*FH, expoDrOn ,DR_WEIGHT, DR_RIGHT ) ;

	subN += 1 ;
	lcd_puts_Pleft(6*FH,PSTR(STR_DR_SW1));
	editExpoVals( event, sub==subN,5*FW, 6*FH, DR_DRSW1 , 0,0);
	subN += 1 ;
	lcd_puts_Pleft(7*FH,PSTR(STR_DR_SW2));
	editExpoVals( event, sub==subN,5*FW, 7*FH, DR_DRSW2 , 0,0);

	lcd_vline(XD - (IS_EXPO_THROTTLE(s_expoChan) ? WCHART : 0), Y0 - WCHART, WCHART * 2);

	plotType = PLOT_BLACK ;

	drawFunction( XD, GRAPH_FUNCTION_EXPO ) ;

	int16_t x512  = calibratedStick[s_expoChan];
	int16_t y512 = calcExpo( l_expoChan, x512 ) ;
  
	lcd_outdezAtt( 19*FW, 6*FH,x512*25/((signed) RESXu/4), 0 );
	lcd_outdezAtt( 14*FW, 1*FH,y512*25/((signed) RESXu/4), 0 );
	
	int8_t xv = (x512 * WCHART + RESX/2) / RESX + XD ;
  int8_t yv = Y0 - (y512 * WCHART + RESX/2) / RESX ;

	lcd_vline( xv, yv-3, 7 ) ;
	lcd_hline( xv-3, yv, 7 ) ;
	
	plotType = PLOT_XOR ;

}


uint8_t char2idx(char c)
{
	uint8_t ret ;
    for(ret=0;;ret++)
    {
        char cc= s_charTab[ret] ;
        if(cc==c) return ret;
        if(cc==0) return 0;
    }
}
char idx2char(uint8_t idx)
{
    if(idx < NUMCHARS) return s_charTab[idx] ;
    return ' ';
}

uint8_t DupIfNonzero = 0 ;
int8_t DupSub ;

void menuDeleteDupModel(uint8_t event)
{
	uint8_t action ;
  lcd_putsnAtt(1,2*FH, (char *)ModelNames[DupSub],sizeof(g_model.name),/*BSS*/0);
  lcd_putc(sizeof(g_model.name)*FW+FW,2*FH,'?');
	action = yesNoMenuExit( event, DupIfNonzero ? PSTR(STR_DUP_MODEL) : PSTR(STR_DELETE_MODEL) ) ;

//    uint8_t i;
	switch(action)
	{
    case YN_YES :
      if ( DupIfNonzero )
      {
        message(PSTR(STR_DUPLICATING));
        if(eeDuplicateModel(DupSub))
        {
          audioDefevent(AU_MENUS);
          DupIfNonzero = 2 ;		// sel_editMode = false;
        }
        else audioDefevent(AU_WARNING1);
      }
      else
      {
				ee32_delete_model( DupSub-1 ) ;
      }
//      pushMenu(menuProcModelSelect);
    break;
		
		case YN_NO :
//      pushMenu(menuProcModelSelect);
    break;
  }
}

void dsmDisplayABLRFH()
{
	for ( uint32_t i = 0 ; i < 6 ; i += 1 )
	{
		uint16_t value = DsmABLRFH[i] ;
		lcd_putc( i*3*(FW+1), 7*FH, "ABLRFH"[i] ) ;
		if ( value == 0 )		// 0xFFFF + 1
		{
		  lcd_puts_P( (i+1)*3*(FW+1)-2-(2*FW), 7*FH, XPSTR("--") ) ;
		}
		else
		{
			if ( value > 1000 )
			{
				value = 1000 ;
			}
		  lcd_outdezAtt( (i+1)*3*(FW+1)-1, 7*FH, value - 1, 0 ) ;
		}
	}
}

#ifdef ASSAN
static uint8_t BindRequest = 0 ;
#endif

uint8_t MultiResponseFlag ;
// Bit 7 = DSMX, Bit 6 = 11mS, bits 3-0 channels
uint8_t MultiResponseData ;

void menuRangeBind(uint8_t event)
{
	static uint8_t timer ;
	static uint8_t binding = 0 ;
#ifdef ASSAN
	static uint8_t mode = 0 ;
#endif
	uint8_t *ptrFlag ;

#if defined(PCBX9D) || defined(PCB9XT)
	ptrFlag = PxxFlag[0] ? &PxxFlag[0] : &PxxFlag[1] ;
#else
	ptrFlag = pxxFlag[0] ? &pxxFlag[0] : &pxxFlag[1] ;
#endif
	uint8_t flag = *ptrFlag & PXX_BIND ;
  if ( event == EVT_ENTRY )
	{
		binding = flag ? 1 : 0 ;
#ifdef ASSAN
#if defined(PCBX9D) || defined(PCB9XT)
		if ( g_model.xprotocol == PROTO_ASSAN )
#else
		if ( g_model.protocol == PROTO_ASSAN )
#endif // PCBX9D
		{
			if ( BindRequest == 2 )
			{
#if defined(PCBX9D) || defined(PCB9XT)
				if ( g_model.xcountry != 2 )
#else
				if ( g_model.country != 2 )
#endif // PCBX9D
				{
					binding = 2 ;		// Select DSM2/DSMX
					mode = 0 ;
				}
				else
				{
					mode = PXX_DSMX ;
					*ptrFlag |= PXX_BIND | mode ;
					binding = 1 ;
					return ;
				}
			}
		}
#endif // ASSAN
		if ( g_model.protocol == PROTO_MULTI )
		{
			if ( (g_model.sub_protocol & 0x3F) == M_DSM )
			{
				MultiResponseFlag = 0 ;
			}
		}
		if ( g_model.xprotocol == PROTO_MULTI )
		{
			if ( (g_model.xsub_protocol & 0x3F) == M_DSM )
			{
				MultiResponseFlag = 0 ;
			}
		}
	}

//	lcd_outhex4( 0, FH, binding ) ;
  
	if ( ( event == EVT_KEY_FIRST(KEY_EXIT) ) || ( event == EVT_KEY_LONG(BTN_RE) ) )
	{
    killEvents(event);
		*ptrFlag = 0 ;
		popMenu(false) ;
	}

#ifdef ASSAN
#if defined(PCBX9D) || defined(PCB9XT)
	if ( g_model.xprotocol == PROTO_ASSAN )
#else
	if ( g_model.protocol == PROTO_ASSAN )
#endif // PCBX9D
	{
		if ( binding == 2 )
		{
			lcd_puts_Pleft( 2*FH, XPSTR("\002Select Bind Mode\037\037\005DSM2  DSMX") ) ;
			lcd_char_inverse( mode ? 11*FW : 5*FW, 4*FH, 24, 0 ) ;
      if( (event==EVT_KEY_FIRST(KEY_RIGHT)) || (event==EVT_KEY_FIRST(KEY_LEFT) ) )
			{
				mode = mode ? 0 : 1 ;
			}
    	if ( (event == EVT_KEY_BREAK(BTN_RE) ) || ( event == EVT_KEY_FIRST(KEY_MENU) ) )
			{
				if (mode)
				{
					mode = PXX_DSMX ;
				}
				*ptrFlag |= PXX_BIND | mode ;
				binding = 1 ;
			}
//      killEvents(event) ;
			return ;
		}
	}
#endif // ASSAN

	lcd_puts_Pleft( 2*FH, (binding) ? PSTR(STR_6_BINDING) : PSTR(STR_RANGE_RSSI) ) ;
//#ifdef PCBX9D
//	lcd_outhex4( 0, 7*FH, pxxFlag ) ;
//extern uint8_t DebugDsmFlag ;
//	lcd_outhex4( 50, 7*FH, DebugDsmFlag ) ;
//#endif

#ifdef FRSKY
	if ( binding == 0 )
	{
		lcd_outdezAtt( 12 * FW, 4*FH, FrskyHubData[FR_RXRSI_COPY], DBLSIZE);
#ifdef ASSAN
#if defined(PCBX9D) || defined(PCB9XT)
		if ( g_model.xprotocol == PROTO_ASSAN )
#else
		if ( g_model.protocol == PROTO_ASSAN )
#endif
#endif
		{
			lcd_outdezAtt( 20 * FW, 4*FH, FrskyHubData[FR_TEMP2], DBLSIZE) ;
		}

#ifdef ASSAN
		if ( FrskyTelemetryType == 2 )		// DSM telemetry
		{
			dsmDisplayABLRFH() ;
		}
#endif // ASSAN
	}

	if ( ( ( g_model.protocol == PROTO_MULTI ) && ( (g_model.sub_protocol & 0x3F) == M_DSM ) ) ||
		( ( g_model.xprotocol == PROTO_MULTI ) && ( (g_model.xsub_protocol & 0x3F) == M_DSM ) ) )
	{
		if ( MultiResponseFlag )
		{
			uint32_t module = 0 ;
#if defined(PCBX9D) || defined(PCB9XT)
			if ( ptrFlag == &PxxFlag[1] )
#else
			if ( ptrFlag == &pxxFlag[1] )
#endif
			{
				module = 1 ;
			}
			
	lcd_puts_Pleft( 7*FH, XPSTR("DSM2 22mS\015ch") ) ;
	if ( MultiResponseData & 0x80 )
	{
		lcd_putc( 3*FW, 7*FH, 'X' ) ;
	}
	if ( MultiResponseData & 0x40 )
	{
		lcd_puts_Pleft( 7*FH, "\00511" ) ;
	}
  lcd_outdez( 13*FW-2, 7*FH, MultiResponseData & 0x0F ) ;

			int8_t x ;
			uint8_t y ;
			
			if ( *ptrFlag )
			{
				*ptrFlag = 0 ;
				x = MultiResponseData & 0x0F ;	// # channels
				y = (MultiResponseData & 0xC0 ) >> 2 ;	// DSMX, 11mS
				if ( module )
				{
					g_model.xoption_protocol = (g_model.xoption_protocol & 0xF0) | x ;
					y |= g_model.xppmNCH & 0x8F ;
					g_model.xppmNCH = y ;	// Set DSM2/DSMX
				}
				else
				{
					g_model.option_protocol = (g_model.option_protocol & 0xF0) | x ;
					y |= g_model.ppmNCH & 0x8F ;
					g_model.ppmNCH = y ;	// Set DSM2/DSMX
				}
				STORE_MODELVARS ;
			}

		}
	}

//#ifdef PCBX9D
//	else if ( g_model.xprotocol == PROTO_ASSAN )
//#else
//	else if ( g_model.protocol == PROTO_ASSAN )
//#endif
//	{
//extern uint8_t DsmControlDebug[20] ;
//extern uint8_t DsmDebug[20] ;
//		lcd_outhex4( 0, 5*FH, DsmDebug[0] ) ;
//		lcd_outhex4( 0, 6*FH, DsmDebug[1] ) ;
//		lcd_outhex4( 30, 6*FH, DsmControlDebug[0] ) ;
//		lcd_outhex4( 60, 6*FH, DsmControlDebug[1] ) ;
//		lcd_outhex4( 30, 7*FH, DsmDebug[2] ) ;
//		lcd_outhex4( 60, 7*FH, DsmDebug[3] ) ;

////extern uint16_t SSCdebug ;
////lcd_outhex4( 90, 7*FH, SSCdebug ) ;
//	}
#endif // FRSKY
	if ( --timer == 0 )
	{
  	audioDefevent(AU_WARNING2) ;
	}
}

static void editTimer( uint8_t sub, uint8_t event )
{
	uint8_t subN ;
	uint8_t timer ;
	if ( sub < 8 )
	{
		subN = 0 ;
		timer = 0 ;
	}
	else
	{
		subN = 8 ;
		timer = 1 ;
	}
	t_TimerMode *ptm ;
	
	ptm = &g_model.timer[timer] ;

	uint8_t y = 0 ;
	uint8_t blink = InverseBlink ;

//  lcd_puts_Pleft( y, PSTR(STR_TIMER) ) ;
//  lcd_putc( 5*FW, y, '1'+ timer ) ;
 	putsTime(14*FW-1, y, ptm->tmrVal,(sub==subN ? blink:0),(sub==subN ? blink:0) ) ;

#ifndef NOPOTSCROLL
//	    if(sub==subN && (s_editing	) )	// Use s_editing???
	    if(sub==subN)	// Use s_editing???
#else
//	    if(sub==subN && s_editMode )
	    if(sub==subN)
#endif
			{
				int16_t temp = 0 ;
//				StepSize = 60 ;
				CHECK_INCDEC_H_MODELVAR( temp, -60 ,60 ) ;
				ptm->tmrVal += temp ;
        if((int16_t)ptm->tmrVal < 0) ptm->tmrVal=0;
				
			}
			y += FH ;
		subN++ ;

    	lcd_puts_Pleft(    y, PSTR(STR_TRIGGERA));
			uint8_t attr = 0 ;
    	if(sub==subN)
			{
   			attr = blink ;
				CHECK_INCDEC_H_MODELVAR_0( ptm->tmrModeA, 1+2+24 ) ;
			}
    	putsTmrMode(16*FW,y,attr, timer, 1 ) ;
			y += FH ;
		subN++ ;
  	
  	  lcd_puts_Pleft(    y, PSTR(STR_TRIGGERB));
  	  attr = 0 ;
  	  if(sub==subN)
			{
	   		attr = blink ;
			}
			uint8_t doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			ptm->tmrModeB = edit_dr_switch( 16*FW, y, ptm->tmrModeB, attr, doedit, event ) ;
			y += FH ;
		subN++ ;
	 
  	  lcd_puts_Pleft( y, PSTR(STR_TIMER) );
  	  attr = 0 ;
  	  if(sub==subN)
			{
				attr = blink ; CHECK_INCDEC_H_MODELVAR_0( ptm->tmrDir,1) ;
			}
  	  lcd_putsAttIdx(  10*FW, y, PSTR(STR_COUNT_DOWN_UP), ptm->tmrDir, attr ) ;
			y += FH ;
		subN++ ;

			uint8_t b = timer ? g_model.timer2Mbeep : g_model.timer1Mbeep ;
  	  (timer ? g_model.timer2Mbeep : g_model.timer1Mbeep) = onoffMenuItem( b, y, PSTR(STR_MINUTE_BEEP), sub ==subN ) ;
			y += FH ;
		subN++;

			b = timer ? g_model.timer2Cdown : g_model.timer1Cdown ;
  	  (timer ? g_model.timer2Cdown : g_model.timer1Cdown) = onoffMenuItem( b, y, PSTR(STR_BEEP_COUNTDOWN), sub==subN  ) ;
			y += FH ;
			subN++;

  	  lcd_puts_Pleft( y, PSTR( STR_RESET_SWITCH ));
  	  attr = 0 ;
			if(sub==subN)
			{
				attr = blink ;
			}

			int16_t sw = (timer==0) ? g_model.timer1RstSw : g_model.timer2RstSw ;
			doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			sw = edit_dr_switch( 16*FW, y, sw, attr, doedit, event ) ;
			if ( timer == 0 )
			{
				g_model.timer1RstSw = sw ;
			}
			else
			{
				g_model.timer2RstSw = sw ;
			}
			y += FH ;
			subN++;
  	  
			lcd_puts_Pleft( y, XPSTR("Haptic"));
  	  attr = 0 ;
			if(sub==subN)
			{
				attr = blink ;
			}
			sw = (timer==0) ? g_model.timer1Haptic : g_model.timer2Haptic ;
			sw = checkIndexed( y, XPSTR(FWx12"\003""\006  NoneMinute Cdown  Both"), sw, (sub==subN) ) ;
			if ( timer == 0 )
			{
				g_model.timer1Haptic = sw ;
			}
			else
			{
				g_model.timer2Haptic = sw ;
			}
			y += FH ;

//	return y ;
}

void multiOption( uint32_t x, uint32_t y, int32_t option, uint32_t attr, uint32_t protocol )
{
	uint32_t display = 1 ;
	switch ( protocol )
	{
		case M_DSM :
//			if ( ( ( option >= 4 ) && ( option <= 12 ) ) || ( ( option <= -4 ) && ( option >= -12 ) ) )
			if ( ( option >= 4 ) && ( option <= 12 ) )
			{
//				if ( option < 0 )
//				{
//					option = -option ;
//  				lcd_putcAtt( x-5*FW, y, 'O', attr ) ;
//				}
				lcd_putsAttIdx( x-4*FW, y, XPSTR("\004 4ch 5ch 6ch 7ch 8ch 9ch10ch11ch12ch"), option-4, attr ) ;
//				lcd_putsAttIdx( x-6*FW, y, XPSTR("\006 4c22m 5c22m 6c22m 7c22m 4c11m 5c11m 6c11m 7c11m 8c11m 9c11m10c11m11c11m12c11m"), option, attr ) ;
				return ;
			}
		break ;

		case M_FRSKYX :
		case M_FrskyD :
		case M_FRSKYV :
		case M_SFHSS :
			lcd_puts_Pleft( y, XPSTR("\014Freq.") ) ;
			display = 0 ;
		break ;
		
		case M_Devo :
			if ( ( option >= 0 ) && ( option <= 1 ) )
			{
				lcd_putsAttIdx( x-6*FW, y, XPSTR("\006AutoIDFixdID"), option, attr ) ;
				return ;
			}
		break ;

	}
	if ( display )
	{
		lcd_puts_Pleft( y, PSTR(STR_MULTI_OPTION) ) ;
	}
	lcd_outdezAtt( x, y, option, attr ) ;
	
/*	
Devo - AutoId when 0 and FixedID when 1
	
	
*/	
	
}


//char *multiSubString( uint32_t x )
//{
//	if ( x == M_Flysky)
//	{
//		return XPSTR(FWx10"\003"M_FLYSKY_STR);
//	}
//	else if ( x == M_FRSKYX )
//	{
//		return XPSTR(FWx10"\001"M_FRSKY_STR);
//	}
//	else if ( x == M_Hisky )
//	{
//		return XPSTR(FWx10"\001"M_HISKY_STR);
//	}
//	else if ( x == M_DSM )
//	{
//		return XPSTR(FWx10"\004"M_DSM_STR);
////		return XPSTR(FWx10"\002"M_DSM_STR);
//	}
//	else if ( x == M_YD717 )
//	{
//		return XPSTR(FWx10"\004"M_YD717_STR);
//	}
//	else if ( x == M_KN )
//	{
//		return XPSTR(FWx10"\001"M_KN_STR);
//	}
//	else if ( x == M_SymaX )
//	{
//		return XPSTR(FWx10"\001"M_SYMAX_STR);
//	}
//	else if ( x == M_CX10 )
//	{
//		return XPSTR(FWx10"\007"M_CX10_STR);
//	}
//	else if ( x == M_CG023 )
//	{
//		return XPSTR(FWx10"\002"M_CG023_STR);
//	}
//	else if ( x == M_MT99XX )
//	{
//		return XPSTR(FWx10"\003"M_MT99XX_STR);
//	}
//	else if ( x == M_MJXQ )
//	{
//		return XPSTR(FWx10"\004"M_MJXQ_STR);
//	}
//	else if ( x == M_HONTAI )
//	{
//		return XPSTR(FWx10"\002"M_HONTAI_STR);
//	}
//	else
//	{
//		return XPSTR(FWx10"\000"M_NONE_STR);
//	}
//}


void menuProcProtocol(uint8_t event)
{
	uint8_t need_bind_range = 0 ;
	EditType = EE_MODEL ;

#if defined(PCBX9D) || defined(PCB9XT)
	uint8_t dataItems = 12 ;
#else
	uint8_t dataItems = 11 ;
#endif

#ifdef REVX
	if (g_model.protocol == PROTO_PPM)
	{
		dataItems += 1 ;
	}
#endif

	if (g_model.protocol == PROTO_PXX)
	{
//#ifdef DISABLE_PXX_SPORT
//#ifdef REVX	
//		dataItems -= 1 ;
//#else	
		dataItems += 3 ;
//#endif
		need_bind_range |= 1 ;
	}
	if (g_model.protocol == PROTO_MULTI)
	{
		dataItems += 4 ;
		need_bind_range = 1 ;
		if ( ( g_model.ppmFrameLength < 0 ) || ( g_model.ppmFrameLength > 4 ) )
		{
			g_model.ppmFrameLength = 0 ;
		}
	}
	if (g_model.protocol == PROTO_DSM2)
	{
#ifdef ENABLE_DSM_MATCH  		
		if (g_model.sub_protocol >= 3)
		{
			dataItems += 1 ;
		}
#endif
		{
			need_bind_range = 4 ;
		}
	}
#ifdef ASSAN
	if (g_model.protocol == PROTO_ASSAN)
	{
#ifdef ENABLE_DSM_MATCH  		
		dataItems += 2 ;
#else
		dataItems += 1 ;
#endif
		need_bind_range = 1 ;
	}
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	if (g_model.xprotocol == PROTO_PXX)
	{
		dataItems += 3 ;
		need_bind_range |= 2 ;
	}
	if (g_model.xprotocol == PROTO_MULTI)
	{
		dataItems += 4 ;
		need_bind_range |= 2 ;
		if ( ( g_model.xppmFrameLength < 0 ) || ( g_model.xppmFrameLength > 4 ) )
		{
			g_model.xppmFrameLength = 0 ;
		}
	}
	if (g_model.xprotocol == PROTO_DSM2)
	{
#ifdef ENABLE_DSM_MATCH  		
		if (g_model.xsub_protocol >= 3)
		{
			dataItems += 1 ;
		}
#endif
		{
			need_bind_range |= 8 ;
		}
	}
#ifdef ASSAN
	if (g_model.xprotocol == PROTO_ASSAN)
	{
#ifdef ENABLE_DSM_MATCH  		
		dataItems += 2 ;
#else
		dataItems += 1 ;
#endif
		need_bind_range |= 2 ;
	}
#endif
	if ( g_model.protocol == PROTO_OFF )
	{
		dataItems -= 4 ;
	}
	if ( g_model.xprotocol == PROTO_OFF )
	{
		dataItems -= 4 ;
	}
#endif

#ifdef PCBSKY
	if ( g_model.protocol == PROTO_OFF )
	{
		dataItems -= 4 ;
	}
	if ( g_model.xprotocol == PROTO_OFF )
	{
		dataItems -= 4 ;
	}
	if ( g_model.xprotocol == PROTO_MULTI )
	{
		dataItems += 4 ;
	}

#endif

uint8_t blink = InverseBlink ;

	TITLE( XPSTR("Protocol") ) ;
	static MState2 mstate2 ;

//	if (SubMenuFromIndex)
//	{
//		event = mstate2.check(event,0,NULL,0,&Columns,0,dataItems-1-1) ;
	event = mstate2.check_columns( event, dataItems-1-1 ) ;
//	}
//	else
//	{
//		event = mstate2.check(event,e_Model,menuTabModel,DIM(menuTabModel),&Columns,0,dataItems-1) ;
//		VARMENU(PSTR(STR_SETUP), menuTabModel, e_Model, dataItems, &columns ) ;
//	}

	int8_t  sub    = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
//	uint8_t subSub = mstate2.m_posHorz;
  uint8_t t_pgOfs ;
//	columns = 0 ;

	t_pgOfs = evalOffset(sub);

	uint8_t y = 1*FH;

	uint8_t subN = 0 ;

#ifndef PCBX9D
 #ifndef PCB9XT
	#ifdef PCBSKY
	if(t_pgOfs<=subN)
	{
		uint8_t value ;
		uint8_t newvalue ;
		value = (g_model.xprotocol == PROTO_OFF) ? 0 : 1 ;
		newvalue = onoffMenuItem( value, y, XPSTR("Internal"), sub==subN ) ;
		if ( newvalue != value )
		{
			value = newvalue ? 0 : PROTO_OFF ;
		}
		else
		{
			value = g_model.xprotocol ;
		}
		if ( g_model.xprotocol != value )
		{
			 g_model.xprotocol = value ;
		}
    if((y+=FH)>7*FH) return;
	} subN++;

	if ( g_model.xprotocol != PROTO_OFF )
	{
		if(t_pgOfs<=subN)
		{
  	  lcd_puts_Pleft( y, PSTR(STR_PROTO));//sub==2 ? INVERS:0);
			uint32_t b = g_model.xprotocol ;
			if ( b == PROTO_DSM2 )
			{
				b = 1 ;
			}
			else if ( b == PROTO_MULTI )
			{
				b = 2 ;
			}
			uint8_t attr = 0 ;
			if ( (sub==subN) && ( subSub==0 ) )
			{
				attr |= blink ;
			}
			lcd_putsAttIdx(  6*FW, y, XPSTR("\005PPM  DSM  Multi"), b, attr );
			if ( attr ) CHECK_INCDEC_H_MODELVAR_0( b, 2 ) ;
			if ( b == 1 )
			{
				b = PROTO_DSM2 ;
			}
			else if ( b == 2 )
			{
				b = PROTO_MULTI ;
			}
			g_model.xprotocol = b ;

			if ( b == PROTO_PPM )
			{
				uint8_t x ;
				attr = 0 ;
				if ( sub==subN )
				{
			 		Columns = 2 ;
				}
			  lcd_puts_Pleft( y, PSTR(STR_21_USEC) );
				x = 11*FW ;
				if ( ( g_model.ppm2NCH > 12 ) || (g_model.ppm2NCH < -2) )
				{
					g_model.ppm2NCH = 0 ;		// Correct if wrong from DSM
				}
				uint8_t chans = g_model.ppm2NCH + 2 ;
				chans *= 2 ;
				if ( chans > 12 )
				{
					chans -= 13 ;
				}
				if ( (sub==subN) && ( subSub==1 ) )
				{
					attr |= blink ;
				}
				lcd_outdezAtt(  x, y, chans+4, attr ) ;
				if ( attr )
				{
 	      	CHECK_INCDEC_H_MODELVAR_0( chans, 12 ) ;
					if ( chans & 1 )	// odd
					{
						chans /= 2 ;
						chans += 7 ;
					}
					else
					{
						chans /= 2 ;
					}
					g_model.ppm2NCH = chans - 2 ;
				}
  			lcd_putsAtt( Lcd_lastPos, y, PSTR( STR_PPMCHANNELS ), attr ) ;
				attr = 0 ;
				if ( (sub==subN) && ( subSub==2 ) )
				{
					attr |= blink ;
				}
				lcd_outdezAtt(  x+7*FW-1, y,  (g_model.xppmDelay*50)+300, attr);
				if ( attr )
				{
					CHECK_INCDEC_H_MODELVAR( g_model.xppmDelay,-4,10);
				}
			}
			else
			{
				attr = 0 ;
				if ( sub==subN )
				{
			 		Columns = 1 ;
				}
		    lcd_puts_Pleft( y, PSTR(STR_13_RXNUM) );
				if ( (sub==subN) && ( subSub==1 ) )
				{
					attr |= blink ;
				}
 	      lcd_outdezAtt(  21*FW, y,  g_model.xPxxRxNum, attr ) ;

				if ( attr )
				{
  	       CHECK_INCDEC_H_MODELVAR_0( g_model.xPxxRxNum,124);
				}
			}
		  if((y+=FH)>7*FH) return ;
		} subN += 1 ;

		if ( g_model.xprotocol == PROTO_MULTI )
		{
			uint8_t attr = 0 ;
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_PROTO));
				uint8_t oldValue = g_model.xsub_protocol ;
				uint8_t svalue = oldValue & 0x3F ;
				
				svalue = editMultiProtocol( svalue, ( sub == subN ) ) ;

				if ( sub == subN )
				{
					attr = blink ;
				}

				displayMultiProtocol( svalue, y, attr ) ;

//				if ( svalue <= M_LAST_MULTI )
//				{
					
//					lcd_putsAttIdx( FW*10, y, MultiString, svalue, attr ) ;
//				}
//				else
//				{
//	  			lcd_outdezAtt( 21*FW, y, svalue+1, attr ) ;
//				}
				g_model.xsub_protocol = svalue + (g_model.xsub_protocol & 0xC0) ;
				if(g_model.xsub_protocol==oldValue)
					attr=(g_model.xppmNCH >> 4) &0x07 ;
				else
				{
					attr=0;
					TelemetryType = TEL_UNKNOWN ;
				}
				if((y+=FH)>7*FH) return ;
			}
			subN++;
		
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_TYPE));
//				char *s ;
				uint8_t x = g_model.xsub_protocol&0x3F ;
  			
				if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( attr, 7 ) ;
				attr = displayMultiSubProtocol( x, attr, y, (sub==subN) ) ;
				g_model.xppmNCH = ( attr << 4) + (g_model.xppmNCH & 0x8F);

//				if ( x <= M_LAST_MULTI )
//				{
//					s = multiSubString( x ) ;
////					uint8_t old = attr ;
//					attr = checkIndexed( y, s, attr, (sub==subN) ) ;
//					g_model.xppmNCH = ( attr << 4) + (g_model.xppmNCH & 0x8F);
////					if ( x == M_DSM )
////					{
////						if ( attr == 4 )
////						{
////							if ( old != 4 )
////							{
////								// Go to bind mode
//// 					    	pxxFlag[1] = PXX_BIND ;		    	//send bind code or range check code
////								pushMenu(menuRangeBind) ;
////							}
////						}
////					}
//				}
//				else
//				{
//					x = attr ;
//					attr = 0 ;
//					if ( sub == subN )
//					{
//						attr = blink ;
// 		  			CHECK_INCDEC_H_MODELVAR_0( x, 7 ) ;
//						g_model.xppmNCH = (x << 4) + (g_model.xppmNCH & 0x8F) ;
//					}
//	  			lcd_outdezAtt( 21*FW, y, x, attr ) ;
//				}
				if((y+=FH)>7*FH) return ;
			}
			subN++;
		
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_AUTO));
				uint8_t value = (g_model.xsub_protocol>>6)&0x01 ;
				lcd_putsAttIdx(  9*FW, y, XPSTR(M_NY_STR), value, (sub==subN && subSub==0 ? blink:0) );
				multiOption( 21*FW, y, g_model.xoption_protocol, (sub==subN && subSub==1 ? blink:0), g_model.xsub_protocol & 0x3F ) ;
				if(sub==subN)
				{
					Columns = 1;
					if(subSub==0 && s_editing)
					{
						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
						g_model.xsub_protocol = (value<<6) + (g_model.xsub_protocol&0xBF);
					}
					if(subSub==1 && s_editing)
						CHECK_INCDEC_H_MODELVAR(g_model.xoption_protocol, -127, 127);
				}
				if((y+=FH)>7*FH) return ;
			}
			subN++;
		
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_POWER));
				// Power stored in ppmNCH bit7 & Option stored in option_protocol
				uint8_t value = (g_model.xppmNCH>>7)&0x01 ;
				lcd_putsAttIdx(  6*FW, y, XPSTR(M_LH_STR), value, ((sub==subN && subSub == 0) ? blink:0) );
				lcd_putsAttIdx(  17*FW, y, XPSTR("\002 7 8 91011"), g_model.xppmFrameLength, ((sub==subN && subSub == 1) ? blink:0) );
				if(sub==subN)
				{
					Columns = 1;
					if(subSub==0 && s_editing)
					{
						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
						g_model.xppmNCH = (value<<7) + (g_model.xppmNCH&0x7F);
					}
					if(subSub==1 && s_editing)
						CHECK_INCDEC_H_MODELVAR_0( g_model.xppmFrameLength, 4);
				}
				if((y+=FH)>7*FH) return ;
			}
			subN++;
		}


		if ( g_model.xprotocol == PROTO_DSM2 )
		{
			if(t_pgOfs<=subN)
  	  {
		    lcd_puts_Pleft( y, PSTR(STR_DSM_TYPE));
  		  int8_t x ;
  	  	x = g_model.xsub_protocol ;
		    lcd_putsAttIdx( 10*FW, y, XPSTR(DSM2_STR), x, (sub==subN ? blink:0) ) ;
  		  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.xsub_protocol,2);
  	  	if((y+=FH)>7*FH) return;
  	  }
			subN += 1 ;
		}

		if(t_pgOfs<=subN)
		{
		  lcd_puts_Pleft( y, PSTR(STR_PPM2_START));
			if ( g_model.startPPM2channel == 0 )
			{
				lcd_putsAtt( 15*FW, y, PSTR(STR_FOLLOW), (sub==subN) ? INVERS : 0 ) ;
			}
			else
			{
				lcd_outdezAtt(  17*FW, y, g_model.startPPM2channel, (sub==subN) ? INVERS : 0 ) ;
			}
		  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.startPPM2channel, 17 ) ;
		  if((y+=FH)>7*FH) return ;
		} subN += 1 ;


		if ( g_model.xprotocol == PROTO_PPM )
		{
			if(t_pgOfs<=subN)
  	  {
 	      lcd_puts_Pleft(    y, PSTR(STR_PPMFRAME_MSEC));
				uint8_t attr = PREC1 ;
 	      if(sub==subN) { attr = INVERS | PREC1 ; CHECK_INCDEC_H_MODELVAR( g_model.xppmFrameLength,-20,20) ; }
 	      lcd_outdezAtt(  14*FW-1, y, (int16_t)g_model.xppmFrameLength*5 + 225, attr ) ;
  	  	if((y+=FH)>7*FH) return;
  	  }
			subN += 1 ;
			
			if(t_pgOfs<=subN)
			{
			  	uint8_t attr = 0 ;
			    lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
			    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.xpulsePol,1);}
			    lcd_putsAttIdx( 11*FW, y, PSTR(STR_POS_NEG),g_model.xpulsePol,attr );
			    if((y+=FH)>7*FH) return;
			}subN++;
		}

		if ( g_model.xprotocol == PROTO_MULTI )
		{
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_BIND) ) ;
      	if(sub==subN)
				{
					lcd_char_inverse( FW, y, 4*FW, 0 ) ;
					if ( event==EVT_KEY_LONG(KEY_MENU))
					{
   		    	pxxFlag[1] = PXX_BIND ;		    	//send bind code or range check code
						pushMenu(menuRangeBind) ;
					}
					s_editMode = 0 ;
				}
				if((y+=FH)>7*FH) return ;
		 	} subN += 1 ;
		}
		 
		if ( ( g_model.xprotocol == PROTO_DSM2 ) || ( g_model.xprotocol == PROTO_MULTI ) )
		{
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
      	if(sub==subN)
				{
					lcd_char_inverse( FW, y, 11*FW, 0 ) ;
					if ( event==EVT_KEY_LONG(KEY_MENU))
					{
    	    	pxxFlag[1] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
						pushMenu(menuRangeBind) ;
					}
					s_editMode = 0 ;
				}
			  if((y+=FH)>7*FH) return ;
		 	} subN += 1 ;
		}

		
//		if(t_pgOfs<=subN)
//		{
//			uint8_t attr = (sub==subN) ? INVERS : 0 ;
//			lcd_puts_Pleft( y, PSTR(STR_PPM2_CHANNELS));
//			uint8_t chans = g_model.ppm2NCH + 2 ;
//			chans *= 2 ;
//			if ( chans > 12 )
//			{
//				chans -= 13 ;
//			}
//			lcd_outdezAtt(  16*FW, y, (chans+4), attr ) ;
//  		lcd_putsAtt( Lcd_lastPos, y, PSTR( STR_PPMCHANNELS ), attr ) ;
//  		if(sub==subN)
//			{
//				CHECK_INCDEC_H_MODELVAR_0( chans, 12 ) ;
//				if ( chans & 1 )	// odd
//				{
//					chans /= 2 ;
//					chans += 7 ;
//				}
//				else
//				{
//					chans /= 2 ;
//				}
//				g_model.ppm2NCH = chans - 2 ;
//			}
//		  if((y+=FH)>7*FH) return ;
// 		} subN += 1 ;
	}
	#endif // PCBSKY
 #endif // nPCB9XT
#endif // nPCBX9D

#if defined(PCBX9D) || defined(PCB9XT)
	if(t_pgOfs<=subN)
	{
		uint8_t attr = (sub==subN) ? INVERS : 0 ;
		lcd_puts_Pleft( y, XPSTR("Trainer Polarity"));
		if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.trainPulsePol,1);}
		lcd_putsAttIdx( 17*FW, y, PSTR(STR_POS_NEG),g_model.trainPulsePol,attr );
	  if((y+=FH)>7*FH) return ;
 	} subN += 1 ;

#endif // PCBX9D || PCB9XT

	uint8_t protocol = g_model.protocol ;
	
#if defined(PCBX9D) || defined(PCB9XT)
	if(t_pgOfs<=subN)
	{
		uint8_t value ;
		uint8_t newvalue ;
		value = (protocol == PROTO_OFF) ? 0 : 1 ;
		newvalue = onoffMenuItem( value, y, XPSTR("Internal"), sub==subN ) ;
		if ( newvalue != value )
		{
			value = newvalue ? 0 : PROTO_OFF ;
		}
		else
		{
			value = protocol ;
		}
		if ( protocol != value )
		{
			 g_model.protocol = protocol = value ;
		}
    if((y+=FH)>7*FH) return;
	} subN++;

	if ( protocol != PROTO_OFF )
	{
#endif

#ifdef PCBSKY
	if(t_pgOfs<=subN)
	{
		uint8_t value ;
		uint8_t newvalue ;
		value = (protocol == PROTO_OFF) ? 0 : 1 ;
		newvalue = onoffMenuItem( value, y, XPSTR("Module"), sub==subN ) ;
		if ( newvalue != value )
		{
			value = newvalue ? 0 : PROTO_OFF ;
		}
		else
		{
			value = protocol ;
		}
		if ( protocol != value )
		{
			 g_model.protocol = protocol = value ;
		}
    if((y+=FH)>7*FH) return;
	} subN++;

	if ( protocol != PROTO_OFF )
	{
#endif
		if(t_pgOfs<=subN)
		{
  	  lcd_puts_Pleft(    y, PSTR(STR_PROTO));//sub==2 ? INVERS:0);
	//    lcd_putsnAtt(  6*FW, y, PSTR(PROT_STR)+PROT_STR_LEN*g_model.protocol,PROT_STR_LEN,(sub==subN && subSub==0 ? (s_editMode ? BLINK : INVERS):0));
			lcd_putsAttIdx(  6*FW, y, PSTR(STR_PROT_OPT), protocol, (sub==subN && subSub==0 ? blink:0) );
  	  if ( protocol == PROTO_PPM )
			{
				uint8_t x ;
			  lcd_puts_Pleft( y, PSTR(STR_21_USEC) );
				x = 10*FW ;
				if ( ( g_model.ppmNCH > 12 ) || (g_model.ppmNCH < -2) )
				{
					g_model.ppmNCH = 0 ;		// Correct if wrong from DSM
				}
//				uint8_t channels = g_model.ppmNCH * 2 + 8 ;
				uint8_t channels = (g_model.ppmNCH + 4) * 2 ;
				if ( channels > 16 )
				{
					channels -= 13 ;
				}
	//			if ( (protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) )
	//			{
	//				channels = g_model.ppmNCH ;
	//			}
				uint8_t attr = LEFT ;
				if ( (sub==subN) && ( subSub==1 ) )
				{
					attr |= blink ;
				}
				lcd_outdezAtt(  x, y, channels, attr ) ;
  			lcd_putsAtt( Lcd_lastPos, y, PSTR( STR_PPMCHANNELS ), attr ) ;
				lcd_outdezAtt(  x+7*FW-1, y,  (g_model.ppmDelay*50)+300, (sub==subN && subSub==2 ? blink:0));
  	  }
  	  else if ( ( protocol == PROTO_PXX) || ( ( protocol == PROTO_DSM2) && (g_model.sub_protocol != DSM_9XR) ) || (protocol == PROTO_MULTI) )
  	  {
			    lcd_puts_Pleft( y, PSTR(STR_13_RXNUM) );
  	      lcd_outdezAtt(  21*FW, y,  g_model.pxxRxNum, (sub==subN && subSub==1 ? blink:0));
  	  }
#ifdef ASSAN
  	  else if ( ( ( protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) ) || (protocol == PROTO_ASSAN) )
#else
  	  else if ( ( ( protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) ) )
#endif
  	  {
		    lcd_puts_Pleft( y, XPSTR("\013Chans") );
 	      lcd_outdezAtt(  21*FW, y,  g_model.ppmNCH, (sub==subN && subSub==1 ? blink:0));
			}

		  if(sub==subN)
			{
			 Columns = (protocol == PROTO_PPM) ? 2 : 1 ;
			 if (s_editing )
			 {
			
#if defined(PCBX9D) || defined(PCB9XT)
 #ifdef PCB9XT
 #ifdef ASSAN
			  uint8_t prot_max = PROT_MAX - 1 ; // 4 ;		// No Assan on internal
 #else
			  uint8_t prot_max = PROT_MAX - 0 ; // 3 ;
 #endif
 #else
 #ifdef ASSAN
			  uint8_t prot_max = PROT_MAX - 3 ;		// No DSM or Assan on internal
 #else
			  uint8_t prot_max = PROT_MAX - 2 ;		// No DSM on internal
 #endif
#endif
#else
			  uint8_t prot_max = PROT_MAX ;
#endif
  	    switch (subSub)
				{
  	     	case 0:
  	        CHECK_INCDEC_H_MODELVAR_0( g_model.protocol, prot_max ) ;
						if ( protocol != g_model.protocol )
						{
							g_model.ppmNCH = 0 ;
						}
  	      break;
  	      case 1:
  	        if (protocol == PROTO_PPM)
						{
							uint8_t chans = g_model.ppmNCH + 2 ;
							chans *= 2 ;
							if ( chans > 12 )
							{
								chans -= 13 ;
							}
 	          	CHECK_INCDEC_H_MODELVAR_0( chans, 12 ) ;
							if ( chans & 1 )	// odd
							{
								chans /= 2 ;
								chans += 7 ;
							}
							else
							{
								chans /= 2 ;
							}
							g_model.ppmNCH = chans - 2 ;
						}
  	  			else if ( protocol == PROTO_PXX)
						{
  	        	CHECK_INCDEC_H_MODELVAR_0( g_model.pxxRxNum,124);
						}
  	  			else if ( protocol == PROTO_DSM2)
						{
							if ( g_model.sub_protocol == DSM_9XR )
							{
  	          	CHECK_INCDEC_H_MODELVAR( g_model.ppmNCH,6,14) ;
							}
							else
							{
  	        		CHECK_INCDEC_H_MODELVAR_0( g_model.pxxRxNum,124);
							}
						}
#ifdef ASSAN
			  	  else if (protocol == PROTO_ASSAN)
						{
 	          	CHECK_INCDEC_H_MODELVAR( g_model.ppmNCH,6,14) ;
						}
#endif
						else if (protocol == PROTO_MULTI)
						{
  	        	CHECK_INCDEC_H_MODELVAR_0( g_model.pxxRxNum,15);
						}
  	      break;
  	      case 2:
  	        if (g_model.protocol == PROTO_PPM)
  	          CHECK_INCDEC_H_MODELVAR( g_model.ppmDelay,-4,10);
  	       break;
  	    }
			 }
  	  }
  	  if((y+=FH)>7*FH) return;
	}subN++;

		if(g_model.protocol == PROTO_PPM)
		{
			if(t_pgOfs<=subN)
  	  {
  	      lcd_puts_Pleft(    y, PSTR(STR_PPMFRAME_MSEC));
					uint8_t attr = PREC1 ;
  	      if(sub==subN) { attr = INVERS | PREC1 ; CHECK_INCDEC_H_MODELVAR( g_model.ppmFrameLength,-20,20) ; }
  	      lcd_outdezAtt(  14*FW-1, y, (int16_t)g_model.ppmFrameLength*5 + 225, attr ) ;
  	  	if((y+=FH)>7*FH) return;
  	  }
			subN += 1 ;

#ifdef REVX
			if(t_pgOfs<=subN)
  	  {
  	    lcd_puts_Pleft(    y, XPSTR(" PPM Drive"));
//				uint8_t attr = 0 ;
  		  uint8_t x ;
  	  	x = g_model.ppmOpenDrain ;
//  	    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0(event,g_model.ppmOpenDrain,1) ; }
//  		  lcd_putsAttIdx( 12*FW, y, XPSTR("\011OpenDrainFullDrive")XPSTR("\011OpenDrainFullDrive"), g_model.ppmOpenDrain, attr );
				g_model.ppmOpenDrain = checkIndexed( y, XPSTR(FWx12"\001""\011OpenDrainFullDrive"), g_model.ppmOpenDrain, (sub==subN) ) ;
				if ( x != g_model.ppmOpenDrain )
				{
					module_output_active() ;		// Changes drive as well
				}
				if((y+=FH)>7*FH) return;
  	  }
			subN += 1 ;
#endif
		}

//#ifndef DISABLE_PXX_SPORT
//#ifndef REVX
		if (protocol == PROTO_PXX)
		{
			if(t_pgOfs<=subN)
			{
//		  	uint8_t attr = 0 ;
				lcd_puts_Pleft( y, PSTR(STR_TYPE) ) ;
//				uint8_t ltype = g_model.sub_protocol ;
//		    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( event, ltype, 2 ) ; }
//				g_model.sub_protocol = ltype ;
//  		  lcd_putsAttIdx( 10*FW, y, XPSTR("\003D16D8 LRP"), ltype, attr );
				g_model.sub_protocol = checkIndexed( y, XPSTR(FWx10"\002""\003D16D8 LRP"), g_model.sub_protocol, (sub==subN) ) ;
		    if((y+=FH)>7*FH) return ;
			}subN++;
			if(t_pgOfs<=subN)
			{
		    lcd_puts_Pleft( y, XPSTR("Chans") );
				g_model.ppmNCH = checkIndexed( y, XPSTR(FWx10"\001""\00216 8"), g_model.ppmNCH, (sub==subN) ) ;
		    if((y+=FH)>7*FH) return ;
			}subN++;
		}
//#endif
		
		if (protocol == PROTO_MULTI)
		{
			uint8_t attr = 0 ;
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_PROTO));
				uint8_t oldValue = g_model.sub_protocol ;
				uint8_t svalue = oldValue & 0x3F ;

				svalue = editMultiProtocol( svalue, ( sub == subN ) ) ;

				if ( sub == subN )
				{
					attr = blink ;
				}
				displayMultiProtocol( svalue, y, attr ) ;

//				if ( svalue <= M_LAST_MULTI )
//				{
//					lcd_putsAttIdx( FW*10, y, MultiString, svalue, attr ) ;
//				}
//				else
//				{
//	  			lcd_outdezAtt( 21*FW, y, svalue+1, attr ) ;
//				}
				g_model.sub_protocol = svalue + (g_model.sub_protocol & 0xC0) ;
				if(g_model.sub_protocol==oldValue)
					attr=(g_model.ppmNCH >> 4) &0x07 ;
				else
				{
					attr=0;
					TelemetryType = TEL_UNKNOWN ;
				}
				if((y+=FH)>7*FH) return ;
			}
			subN++;
			
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_TYPE));
//				char *s ;
				uint8_t x = g_model.sub_protocol&0x3F ;
				
  			if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( attr, 7 ) ;
				attr = displayMultiSubProtocol( x, attr, y, (sub==subN) ) ;
				g_model.ppmNCH = ( attr << 4) + (g_model.ppmNCH & 0x8F);
				
//				if ( x <= M_LAST_MULTI )
//				{
////					uint8_t old = attr ;
//					s = multiSubString( x ) ;
//					attr = checkIndexed( y, s,  attr, (sub==subN) ) ;
//					g_model.ppmNCH = ( attr << 4) + (g_model.ppmNCH & 0x8F);
////					if ( x == M_DSM )
////					{
////						if ( attr == 4 )
////						{
////							if ( old != 4 )
////							{
////								// Go to bind mode
////#if defined(PCBX9D) || defined(PCB9XT)
////	    		    	PxxFlag[0] = PXX_BIND ;		    	//send bind code or range check code
////#else
////  	 		    		pxxFlag[0] = PXX_BIND ;		    	//send bind code or range check code
////#endif
////								pushMenu(menuRangeBind) ;
////							}
////						}
////					}
//				}
//				else
//				{
//					x = attr ;
//					attr = 0 ;
//					if ( sub == subN )
//					{
//						attr = blink ;
// 		  			CHECK_INCDEC_H_MODELVAR_0( x, 7 ) ;
//						g_model.ppmNCH = (x << 4) + (g_model.ppmNCH & 0x8F) ;
//					}
//	  			lcd_outdezAtt( 21*FW, y, x, attr ) ;
//				}
				if((y+=FH)>7*FH) return ;
			}
			subN++;

			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_AUTO));
				uint8_t value = (g_model.sub_protocol>>6)&0x01 ;
				lcd_putsAttIdx(  9*FW, y, XPSTR(M_NY_STR), value, (sub==subN && subSub==0 ? blink:0) );
				multiOption( 21*FW, y, g_model.option_protocol, (sub==subN && subSub==1 ? blink:0), g_model.sub_protocol & 0x3F ) ;
	//			lcd_xlabel_decimal( 21*FW, y, g_model.option_protocol, (sub==subN && subSub==1 ? blink:0), PSTR(STR_MULTI_OPTION) ) ;
				if(sub==subN)
				{
					Columns = 1;
					if(subSub==0 && s_editing)
					{
						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
						g_model.sub_protocol = (value<<6) + (g_model.sub_protocol&0xBF);
					}
					if(subSub==1 && s_editing)
						CHECK_INCDEC_H_MODELVAR(g_model.option_protocol, -127, 127);
				}
				if((y+=FH)>7*FH) return ;
			}
			subN++;

			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_POWER));
				// Power stored in ppmNCH bit7 & Option stored in option_protocol
				uint8_t value = (g_model.ppmNCH>>7)&0x01 ;
				lcd_putsAttIdx(  6*FW, y, XPSTR(M_LH_STR), value, ((sub==subN && subSub == 0) ? blink:0) );
				lcd_putsAttIdx(  17*FW, y, XPSTR("\002 7 8 91011"), g_model.ppmFrameLength, ((sub==subN && subSub == 1) ? blink:0) );
				if(sub==subN)
				{
					Columns = 1;
					if(subSub==0 && s_editing)
					{
						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
						g_model.ppmNCH = (value<<7) + (g_model.ppmNCH&0x7F);
					}
					if(subSub==1 && s_editing)
						CHECK_INCDEC_H_MODELVAR_0( g_model.ppmFrameLength, 4);
				}
				if((y+=FH)>7*FH) return ;
			}
			subN++;

		}
		
		if (protocol == PROTO_DSM2)
		{
			if(t_pgOfs<=subN)
			{
		    lcd_puts_Pleft(    y, PSTR(STR_DSM_TYPE));
  		  int8_t x ;
  	  	x = g_model.sub_protocol ;
#ifndef REVX
//		    if ( x > 2 ) x = 2 ;
#endif
//  		  g_model.sub_protocol = x ;
	//        lcd_putsnAtt(10*FW,y, PSTR(DSM2_STR)+DSM2_STR_LEN*(x),DSM2_STR_LEN, (sub==subN ? (s_editMode ? BLINK : INVERS):0));
		    lcd_putsAttIdx( 10*FW, y, XPSTR(DSM2_STR), x, (sub==subN ? blink:0) ) ;
#ifdef PCBSKY
  		  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.sub_protocol,3);
#else        
#ifdef PCB9XT
				if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.sub_protocol,2);
#else        
				if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.sub_protocol,3);
#endif
				if ( g_model.sub_protocol != x )
				{
					g_model.ppmNCH = ( g_model.sub_protocol == 3 ) ? 12 : 6 ;
				}
#endif
  	  	if((y+=FH)>7*FH) return ;
			} subN += 1 ;
		}

#ifdef ENABLE_DSM_MATCH  		
#ifdef ASSAN		
		if ( ( ( protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) ) || (protocol == PROTO_ASSAN) )
#else
		if ( ( ( protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) ) )
#endif
		{
			lcd_puts_Pleft( y, PSTR(STR_13_RXNUM)+1 );
      lcd_outdezAtt(  21*FW, y,  g_model.pxxRxNum, (sub==subN) ? blink:0);
  	  
			CHECK_INCDEC_H_MODELVAR_0( g_model.pxxRxNum,124);
			
  	  if((y+=FH)>7*FH) return ;
			subN++ ;
		}
#endif // ENABLE_DSM_MATCH  		 
  	
			 
//#ifndef DISABLE_PXX_SPORT
//#ifndef REVX
#ifdef PCBSKY
 #ifdef ASSAN
		if ( (protocol == PROTO_PXX) || ( protocol == PROTO_ASSAN) )
 #else
		if (protocol == PROTO_PXX)
 #endif
#else
		if (protocol == PROTO_PXX)
#endif
		{
			if(t_pgOfs<=subN)
			{
//		  	uint8_t attr = 0 ;
				lcd_puts_Pleft( y, PSTR(STR_COUNTRY) ) ;
//				uint8_t lcountry = g_model.country ;
//		    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( event, lcountry, 2 ) ;}
//				g_model.country = lcountry ;
//  		  lcd_putsAttIdx( 10*FW, y, XPSTR("\003AmeJapEur"), lcountry, attr );
				g_model.country = checkIndexed( y, XPSTR(FWx10"\002""\003AmeJapEur"), g_model.country, (sub==subN) ) ;
		    if((y+=FH)>7*FH) return ;
			}subN++;
			
//			if(t_pgOfs<subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_SP_FAILSAFE) ) ;
//  	    if(sub==subN)
//				{
//					lcd_char_inverse( 1*FW, y, 8*FW, 0 ) ;
//					if ( event==EVT_KEY_BREAK(KEY_MENU))
//					{
//						pushMenu(menuFailsafe) ;
//					}
//				}
//		    if((y+=FH)>7*FH) return ;
//			}subN++;
		}
//#endif
	
#if defined(PCBX9D) || defined(PCB9XT)
	}
#endif
#ifdef PCBSKY
	}
#endif

#if defined(PCBX9D) || defined(PCB9XT)
	if ( g_model.protocol != PROTO_OFF )
	{
#endif
#ifdef PCBSKY
	if ( g_model.protocol != PROTO_OFF )
	{
#endif
		if(t_pgOfs<=subN)
		{
#ifdef ASSAN
			if ( protocol == PROTO_ASSAN)
			{
		    lcd_puts_Pleft( y, XPSTR("\020DsM") );
 	      lcd_outdezAtt(  21*FW, y,  g_model.dsmMode, (sub==subN && subSub==1 ? blink:0));
				uint8_t attr = 0 ;
 				lcd_puts_Pleft( y, PSTR(STR_PPM_1ST_CHAN));
				if ( sub==subN )
				{
					Columns = 1 ;
					if ( subSub==1 )
					{
         		CHECK_INCDEC_H_MODELVAR( g_model.dsmMode,0,15) ;
					}
	  			if( subSub==0 ) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.startChannel, 16 ) ; }
				}
				lcd_outdezAtt(  14*FW, y, g_model.startChannel + 1, attr ) ;
			}
			else
#endif
			{
				uint8_t attr = 0 ;
  			lcd_puts_Pleft( y, PSTR(STR_PPM_1ST_CHAN));
  			if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.startChannel, 16 ) ; }
				lcd_outdezAtt(  14*FW, y, g_model.startChannel + 1, attr ) ;
			}
		  if((y+=FH)>7*FH) return ;
		} subN += 1 ;
#if defined(PCBX9D) || defined(PCB9XT)
	}
#endif
#ifdef PCBSKY
	}
#endif

  if(g_model.protocol == PROTO_PPM)
	{
		if(t_pgOfs<=subN) {
		  	uint8_t attr = 0 ;
		    lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
		    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.pulsePol,1);}
		    lcd_putsAttIdx( 11*FW, y, PSTR(STR_POS_NEG),g_model.pulsePol,attr );
		    if((y+=FH)>7*FH) return;
		}subN++;
	}
	
	if ( need_bind_range & 5 )
	{
//#ifdef DISABLE_PXX_SPORT
////#ifdef REVX
//    if(g_model.protocol == PROTO_PXX)
//		{
//			if(t_pgOfs<=subN)
//			{			
//    		lcd_putsAtt(0,    y, PSTR(STR_SEND_RX_NUM), (sub==subN ? INVERS:0));

//      	if(sub==subN )
//      	{
//					if ( event==EVT_KEY_LONG(KEY_MENU))
//					{
//	    	    //send reset code
//  	  	    pxxFlag = PXX_BIND ;
//					}
//					s_editMode = 0 ;
//      	}
//		  	if((y+=FH)>7*FH) return ;
//	 		} subN += 1 ;
//		}
//		else
//    {
//#endif
			if ( need_bind_range & 1 )
			{
				if(t_pgOfs<=subN)
				{
					lcd_puts_Pleft( y, PSTR(STR_BIND) ) ;
      		if(sub==subN)
					{
						lcd_char_inverse( FW, y, 4*FW, 0 ) ;
						if ( event==EVT_KEY_LONG(KEY_MENU))
						{
#ifdef ASSAN
							if ( g_model.protocol == PROTO_ASSAN )
							{
								BindRequest = 2 ; ;
							}
							else
							{
#if defined(PCBX9D) || defined(PCB9XT)
	    		    	PxxFlag[0] = PXX_BIND ;		    	//send bind code or range check code
#else
    		    		pxxFlag = PXX_BIND ;		    	//send bind code or range check code
#endif
							}
#else
#if defined(PCBX9D) || defined(PCB9XT)
    		    	PxxFlag[0] = PXX_BIND ;		    	//send bind code or range check code
#else
   		    		pxxFlag[0] = PXX_BIND ;		    	//send bind code or range check code
#endif
#endif
							pushMenu(menuRangeBind) ;
						}
						s_editMode = 0 ;
					}
				  if((y+=FH)>7*FH) return ;
		 		} subN += 1 ;
			}
			
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
      	if(sub==subN)
				{
					lcd_char_inverse( FW, y, 11*FW, 0 ) ;
					if ( event==EVT_KEY_LONG(KEY_MENU))
					{
#ifdef ASSAN
						if ( g_model.protocol == PROTO_ASSAN )
						{
							BindRequest = 0 ;
						}
#endif
#if defined(PCBX9D) || defined(PCB9XT)
    	    	PxxFlag[0] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
#else
    	    	pxxFlag[0] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
#endif
						pushMenu(menuRangeBind) ;
					}
					s_editMode = 0 ;
				}
			  if((y+=FH)>7*FH) return ;
		 	} subN += 1 ;
//#ifdef DISABLE_PXX_SPORT
//#ifdef REVX
//    }
//#endif
	}

#if defined(PCBX9D) || defined(PCB9XT)

	protocol = g_model.xprotocol ;
	
	if(t_pgOfs<=subN)
	{
		uint8_t value ;
		uint8_t newvalue ;
		value = (protocol == PROTO_OFF) ? 0 : 1 ;
		newvalue = onoffMenuItem( value, y, XPSTR("External"), sub==subN ) ;
		if ( newvalue != value )
		{
			value = newvalue ? 0 : PROTO_OFF ;
		}
		else
		{
			value = protocol ;
		}
		if ( protocol != value )
		{
			 g_model.xprotocol = protocol = value ;
		}
    if((y+=FH)>7*FH) return;
	} subN++;

	if ( protocol != PROTO_OFF )
	{
		if(t_pgOfs<=subN)
		{
  	  lcd_puts_Pleft(    y, PSTR(STR_PROTO));//sub==2 ? INVERS:0);
	//    lcd_putsnAtt(  6*FW, y, PSTR(PROT_STR)+PROT_STR_LEN*g_model.protocol,PROT_STR_LEN,(sub==subN && subSub==0 ? (s_editMode ? BLINK : INVERS):0));
			lcd_putsAttIdx(  6*FW, y, PSTR(STR_PROT_OPT), protocol, (sub==subN && subSub==0 ? blink:0) );
  	  if ( protocol == PROTO_PPM )
			{
				uint8_t x ;
			  lcd_puts_Pleft( y, PSTR(STR_21_USEC) );
				x = 10*FW ;
				if ( g_model.xppmNCH > 12 )
				{
					g_model.xppmNCH = 0 ;		// Correct if wrong from DSM
				}
//				uint8_t channels = g_model.xppmNCH * 2 + 8 ;
				uint8_t channels = (g_model.xppmNCH + 4) * 2 ;
				if ( channels > 16 )
				{
					channels -= 13 ;
				}
	//			if ( (protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) )
	//			{
	//				channels = g_model.ppmNCH ;
	//			}
				uint8_t attr = LEFT ;
				if ( (sub==subN) && ( subSub==1 ) )
				{
					attr |= blink ;
				}
				lcd_outdezAtt(  x, y, channels, attr ) ;
  			lcd_putsAtt( Lcd_lastPos, y, PSTR( STR_PPMCHANNELS ), attr ) ;
				lcd_outdezAtt(  x+7*FW-1, y,  (g_model.xppmDelay*50)+300, (sub==subN && subSub==2 ? blink:0));
  	  }
  	  else if ( ( protocol == PROTO_PXX) || ( ( protocol == PROTO_DSM2) && (g_model.xsub_protocol != DSM_9XR) ) || (protocol == PROTO_MULTI) )
  	  {
			    lcd_puts_Pleft( y, PSTR(STR_13_RXNUM) );
  	      lcd_outdezAtt(  21*FW, y,  g_model.xPxxRxNum, (sub==subN && subSub==1 ? blink:0));
  	  }
#ifdef ASSAN
  	  else if ( ( ( protocol == PROTO_DSM2) && ( g_model.xsub_protocol == DSM_9XR ) ) || (protocol == PROTO_ASSAN) )
#else
  	  else if ( ( ( protocol == PROTO_DSM2) && ( g_model.xsub_protocol == DSM_9XR ) ) )
#endif
  	  {
		    lcd_puts_Pleft( y, XPSTR("\013Chans") );
 	      lcd_outdezAtt(  21*FW, y,  g_model.xppmNCH, (sub==subN && subSub==1 ? blink:0));
// 	      lcd_outhex4( 17*FW, y+2*FH, g_model.dsmMode ) ;
			}

		  if(sub==subN)
			{
			 Columns = (protocol == PROTO_PPM) ? 2 : 1 ;
			 if (s_editing )
			 {
			  uint8_t prot_max = PROT_MAX ;
  	    switch (subSub)
				{
  	     	case 0:
  	        CHECK_INCDEC_H_MODELVAR_0( g_model.xprotocol, prot_max ) ;
						protocol = g_model.xprotocol ;
  	      break;
  	      case 1:
  	        if (g_model.xprotocol == PROTO_PPM)
						{
							uint8_t chans = g_model.xppmNCH + 2 ;
							chans *= 2 ;
							if ( chans > 12 )
							{
								chans -= 13 ;
							}
  	          CHECK_INCDEC_H_MODELVAR_0( chans, 12 ) ;
							if ( chans & 1 )	// odd
							{
								chans /= 2 ;
								chans += 7 ;
							}
							else
							{
								chans /= 2 ;
							}
							g_model.xppmNCH = chans - 2 ;
						}
  	  			else if ( ( protocol == PROTO_PXX) || ( protocol == PROTO_DSM2) )
						{
  	          	CHECK_INCDEC_H_MODELVAR_0( g_model.xPxxRxNum,124);
						}
  	  			else if ( protocol == PROTO_DSM2)
						{
							if ( g_model.sub_protocol == DSM_9XR )
							{
  	          	CHECK_INCDEC_H_MODELVAR( g_model.xppmNCH,6,14) ;
							}
							else
							{
  	        		CHECK_INCDEC_H_MODELVAR_0( g_model.xPxxRxNum,124);
							}
						}
#ifdef ASSAN
			  	  else if (protocol == PROTO_ASSAN)
						{
 	          	CHECK_INCDEC_H_MODELVAR( g_model.xppmNCH,6,14) ;
						}
#endif
						else if (protocol == PROTO_MULTI)
						{
  	        	CHECK_INCDEC_H_MODELVAR_0( g_model.xPxxRxNum,15);
						}
  	      break;
  	      case 2:
  	        if (g_model.xprotocol == PROTO_PPM)
  	          CHECK_INCDEC_H_MODELVAR( g_model.xppmDelay,-4,10);
  	       break;
  	    }
			 }
  	  }
  	  if((y+=FH)>7*FH) return;
	}subN++;

  	if(g_model.xprotocol == PROTO_PPM)
		{
			if(t_pgOfs<=subN)
  	  {
  	      lcd_puts_Pleft(    y, PSTR(STR_PPMFRAME_MSEC));
					uint8_t attr = PREC1 ;
  	      if(sub==subN) { attr = INVERS | PREC1 ; CHECK_INCDEC_H_MODELVAR( g_model.xppmFrameLength,-20,20) ; }
  	      lcd_outdezAtt(  14*FW-1, y, (int16_t)g_model.xppmFrameLength*5 + 225, attr ) ;
  	  	if((y+=FH)>7*FH) return;
  	  }
			subN += 1 ;
		}

		if (protocol == PROTO_PXX)
		{
			if(t_pgOfs<=subN)
			{
//		  	uint8_t attr = 0 ;
				lcd_puts_Pleft( y, PSTR(STR_TYPE) ) ;
//				uint8_t ltype = g_model.xsub_protocol ;
//		    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( event, ltype, 2 ) ; }
//				g_model.xsub_protocol = ltype ;
//  		  lcd_putsAttIdx( 10*FW, y, XPSTR("\003D16D8 LRP"), ltype, attr );
				g_model.xsub_protocol = checkIndexed( y, XPSTR(FWx10"\002""\003D16D8 LRP"), g_model.xsub_protocol, (sub==subN) ) ;
				if((y+=FH)>7*FH) return ;
			}subN++;
			if(t_pgOfs<=subN)
			{
		    lcd_puts_Pleft( y, XPSTR("Chans") );
				g_model.xppmNCH = checkIndexed( y, XPSTR(FWx10"\001""\00216 8"), g_model.xppmNCH, (sub==subN) ) ;
		    if((y+=FH)>7*FH) return ;
			}subN++;
		}
	
		if (protocol == PROTO_MULTI)
		{
			lcd_puts_Pleft( y, PSTR(STR_MULTI_PROTO));
			uint8_t oldValue = g_model.xsub_protocol ;
			uint8_t attr = 0 ;
			uint8_t svalue = oldValue & 0x3F ;
			
			svalue = editMultiProtocol( svalue, ( sub == subN ) ) ;

			if ( sub == subN )
			{
				attr = blink ;
			}
			if ( sub == subN )
			{
				attr = blink ;
//  		  CHECK_INCDEC_H_MODELVAR_0( svalue, 62 ) ;
			}
			displayMultiProtocol( svalue, y, attr ) ;
//			if ( svalue <= M_LAST_MULTI )
//			{
//				lcd_putsAttIdx( FW*10, y, MultiString, svalue, attr ) ;
//			}
//			else
//			{
//	  		lcd_outdezAtt( 21*FW, y, svalue, attr ) ;
//			}
			g_model.xsub_protocol = svalue + (g_model.xsub_protocol & 0xC0) ;
			if(g_model.xsub_protocol==oldValue)
				attr=(g_model.xppmNCH >> 4) &0x07 ;
			else
			{
				attr=0;
				TelemetryType = TEL_UNKNOWN ;
			}
			y += FH ;
			subN++;
			lcd_puts_Pleft( y, PSTR(STR_MULTI_TYPE));
//			char *s ;
			uint8_t x = g_model.xsub_protocol&0x3F ;
				
 			if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( attr, 7 ) ;
			attr = displayMultiSubProtocol( x, attr, y, (sub==subN) ) ;
			g_model.xppmNCH = ( attr << 4) + (g_model.xppmNCH & 0x8F);
			
//			if ( x <= M_LAST_MULTI )
//			{
//				s = multiSubString( x ) ;
////				uint8_t old = attr ;
//				attr = checkIndexed( y, s,  attr, (sub==subN) ) ;
//				g_model.xppmNCH = ( attr << 4) + (g_model.xppmNCH & 0x8F);
////				if ( x == M_DSM )
////				{
////					if ( attr == 4 )
////					{
////						if ( old != 4 )
////						{
////							// Go to bind mode
//// 					    PxxFlag[1] = PXX_BIND ;		    	//send bind code or range check code
////							pushMenu(menuRangeBind) ;
////						}
////					}
////				}
//			}
//			else
//			{
//				x = attr ;
//				attr = 0 ;
//				if ( sub == subN )
//				{
//					attr = blink ;
// 		  		CHECK_INCDEC_H_MODELVAR_0( x, 7 ) ;
//					g_model.xppmNCH = (x << 4) + (g_model.xppmNCH & 0x8F) ;
//				}
//	  		lcd_outdezAtt( 21*FW, y, x, attr ) ;
//			}
			y += FH ;
			subN++;

			lcd_puts_Pleft( y, PSTR(STR_MULTI_AUTO));
			uint8_t value = (g_model.xsub_protocol>>6)&0x01 ;
			lcd_putsAttIdx(  9*FW, y, XPSTR(M_NY_STR), value, (sub==subN && subSub==0 ? blink:0) );
			multiOption( 21*FW, y, g_model.xoption_protocol, (sub==subN && subSub==1 ? blink:0), g_model.xsub_protocol & 0x1F ) ;
//			lcd_xlabel_decimal( 21*FW, y, g_model.option_protocol, (sub==subN && subSub==1 ? blink:0), PSTR(STR_MULTI_OPTION) ) ;
			if(sub==subN)
			{
				Columns = 1;
				if(subSub==0 && s_editing)
				{
					CHECK_INCDEC_H_MODELVAR_0(value, 1 );
					g_model.xsub_protocol = (value<<6) + (g_model.xsub_protocol&0xBF);
				}
				if(subSub==1 && s_editing)
					CHECK_INCDEC_H_MODELVAR(g_model.xoption_protocol, -127, 127);
			}
			y += FH ;
			subN++;
			
			lcd_puts_Pleft( y, PSTR(STR_MULTI_POWER));
			// Power stored in xppmNCH bit7 & Option stored in option_protocol
			value = (g_model.xppmNCH>>7)&0x01 ;
			lcd_putsAttIdx(  6*FW, y, XPSTR(M_LH_STR), value, ((sub==subN && subSub == 0) ? blink:0) );
			lcd_putsAttIdx(  17*FW, y, XPSTR("\002 7 8 91011"), g_model.xppmFrameLength, ((sub==subN && subSub == 1) ? blink:0) );
			if(sub==subN)
			{
				Columns = 1;
				if(subSub==0 && s_editing)
				{
					CHECK_INCDEC_H_MODELVAR_0(value, 1 );
					g_model.xppmNCH = (value<<7) + (g_model.xppmNCH&0x7F);
				}
				if(subSub==1 && s_editing)
					CHECK_INCDEC_H_MODELVAR_0( g_model.xppmFrameLength, 4);
			}
			y += FH ;
			subN++;
		}
		
		if (protocol == PROTO_DSM2)
		{
			if(t_pgOfs<=subN)
			{
		    lcd_puts_Pleft(    y, PSTR(STR_DSM_TYPE));
  		  int8_t x ;
  	  	x = g_model.xsub_protocol ;
		    if ( x > 3 ) x = 3 ;
  		  g_model.xsub_protocol = x ;
	//        lcd_putsnAtt(10*FW,y, PSTR(DSM2_STR)+DSM2_STR_LEN*(x),DSM2_STR_LEN, (sub==subN ? (s_editMode ? BLINK : INVERS):0));
		    lcd_putsAttIdx( 10*FW, y, XPSTR(DSM2_STR), x, (sub==subN ? blink:0) ) ;
  		  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.xsub_protocol,3);
  	  	if((y+=FH)>7*FH) return ;
			} subN += 1 ;
		}
			 
 #ifdef ASSAN
		if ( (protocol == PROTO_PXX) || ( protocol == PROTO_ASSAN) )
 #else
		if (protocol == PROTO_PXX)
 #endif
		{
			if(t_pgOfs<=subN)
			{
		  	uint8_t attr = 0 ;
				lcd_puts_Pleft( y, PSTR(STR_COUNTRY) ) ;
				uint8_t lcountry = g_model.xcountry ;
		    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( lcountry, 2 ) ;}
				g_model.xcountry = lcountry ;
  		  lcd_putsAttIdx( 10*FW, y, XPSTR("\003AmeJapEur"), lcountry, attr );
		    if((y+=FH)>7*FH) return ;
			}subN++;
			
		}
	}

	if ( g_model.xprotocol != PROTO_OFF )
	{
		if(t_pgOfs<=subN)
		{
#ifdef ASSAN
			if ( protocol == PROTO_ASSAN)
			{
				Columns = 1 ;
		    lcd_puts_Pleft( y, XPSTR("\020DsM") );
 	      lcd_outdezAtt(  21*FW, y,  g_model.dsmMode, (sub==subN && subSub==1 ? blink:0));
				if ( sub==subN && subSub==1 )
				{
         	CHECK_INCDEC_H_MODELVAR( g_model.dsmMode,0,15) ;
				}
				uint8_t attr = 0 ;
  			lcd_puts_Pleft( y, PSTR(STR_PPM_1ST_CHAN));
  			if( sub==subN && subSub==0 ) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.xstartChannel, 16 ) ; }
				lcd_outdezAtt(  14*FW, y, g_model.xstartChannel + 1, attr ) ;
			}
			else
#endif
			{
				uint8_t attr = 0 ;
  			lcd_puts_Pleft( y, PSTR(STR_PPM_1ST_CHAN));
  			if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.xstartChannel, 16 ) ; }
				lcd_outdezAtt(  14*FW, y, g_model.xstartChannel + 1, attr ) ;
			}
		  if((y+=FH)>7*FH) return ;
		} subN += 1 ;
	}

	if ( protocol != PROTO_OFF )
	{
  	if(g_model.xprotocol == PROTO_PPM)
		{
			if(t_pgOfs<=subN) {
			  	uint8_t attr = 0 ;
			    lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
			    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.xpulsePol,1);}
			    lcd_putsAttIdx( 11*FW, y, PSTR(STR_POS_NEG),g_model.xpulsePol,attr );
			    if((y+=FH)>7*FH) return;
			}subN++;
		}
	}
#endif

#if defined(PCBX9D) || defined(PCB9XT)
	if ( need_bind_range & 2 )
	{
		if(t_pgOfs<=subN)
		{
			lcd_puts_Pleft( y, PSTR(STR_BIND) ) ;
      if(sub==subN)
			{
				lcd_char_inverse( FW, y, 4*FW, 0 ) ;
				if ( event==EVT_KEY_LONG(KEY_MENU))
				{
#ifdef ASSAN
					if ( g_model.xprotocol == PROTO_ASSAN )
					{
						BindRequest = 2 ; ;
					}
					else
					{
  		    	PxxFlag[1] = PXX_BIND ;		    	//send bind code or range check code
					}
#else
 		    	PxxFlag[1] = PXX_BIND ;		    	//send bind code or range check code
#endif
					pushMenu(menuRangeBind) ;
				}
				s_editMode = 0 ;
			}
			if((y+=FH)>7*FH) return ;
		} subN += 1 ;
	}		
	if ( need_bind_range & 0x0A )
	{
		if(t_pgOfs<=subN)
		{
			lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
      if(sub==subN)
			{
				lcd_char_inverse( FW, y, 11*FW, 0 ) ;
				if ( event==EVT_KEY_LONG(KEY_MENU))
				{
#ifdef ASSAN
						if ( g_model.xprotocol == PROTO_ASSAN )
						{
							BindRequest = 0 ;
						}
#endif
    	    PxxFlag[1] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
					pushMenu(menuRangeBind) ;
				}
				s_editMode = 0 ;
			}
    }		
	}
#endif
}


void putsTrimMode( uint8_t x, uint8_t y, uint8_t phase, uint8_t idx, uint8_t att )
{
  int16_t v = getRawTrimValue(phase, idx);

  if (v > TRIM_EXTENDED_MAX)
	{
    uint8_t p = v - TRIM_EXTENDED_MAX - 1;
    if (p >= phase) p += 1 ;
    lcd_putcAtt(x, y, '0'+p, att);
  }
  else
	{
//#ifdef FIX_MODE
  	lcd_putsAttIdx( x, y, PSTR(STR_1_RETA), idx, att ) ;
//#else
//  	lcd_putsAttIdx( x, y, PSTR(STR_1_RETA), *(modn12x3+g_eeGeneral.stickMode*4+idx)-1, att ) ;
//#endif
//  	lcd_putsAttIdx( x, y, "\001RETA", idx, att ) ;
  }
}


void menuPhaseOne(uint8_t event)
{
  PhaseData *phase = &g_model.phaseData[s_currIdx] ;
	TITLE(PSTR(STR_FL_MODE)) ;

	static MState2 mstate2 ;
	mstate2.check_columns(event,6-1) ;
//  SUBMENU( PSTR(STR_FL_MODE), 5, { 0, 3, 0, 0, 5 /*, 0*/} ) ;
  lcd_putc( 8*FW, 0, '1'+s_currIdx ) ;

  int8_t sub = mstate2.m_posVert;
  int8_t editMode = s_editMode;

//#ifdef FIX_MODE
//	lcd_puts_P( 10*FW, 3*FH, &(PSTR(STR_1_RETA)[1]) ) ;
//#endif
  
	for (uint8_t i = 0 ; i < 6 ; i += 1 )
	{
    uint8_t y = (i+2) * FH;
		uint8_t attr = (sub==i ? InverseBlink : 0);
    
		switch(i)
		{
      case 0 : // switch
				lcd_puts_Pleft( y, PSTR(STR_SWITCH) ) ;
				phase->swtch = edit_dr_switch( 10*FW, y, phase->swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			break;

      case 1 : // switch
				lcd_puts_Pleft( y, PSTR(STR_SWITCH) ) ;
				phase->swtch2 = edit_dr_switch( 10*FW, y, phase->swtch2, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			break;

      case 2 : // trims
				if ( attr )
				{
					Columns = 3 ;
				}
				lcd_puts_Pleft( y, PSTR(STR_TRIMS) ) ;
        for ( uint8_t t = 0 ; t<NUM_STICKS ; t += 1 )
				{
          putsTrimMode( (10+t)*FW, y, s_currIdx+1, t, (g_posHorz==t) ? attr : 0 ) ;
          if (attr && g_posHorz==t && ((editMode>0) || P1values.p1valdiff))
					{
            int16_t v = phase->trim[t] ;
            if (v < TRIM_EXTENDED_MAX)
						{
							v = TRIM_EXTENDED_MAX;
						}
						int16_t u = v ;
            v = checkIncDec16( v, TRIM_EXTENDED_MAX, TRIM_EXTENDED_MAX+MAX_MODES, EE_MODEL ) ;
            
						if (v != u)
						{
              if (v == TRIM_EXTENDED_MAX) v = 0 ;
  						phase->trim[t] = v ;
            }
          }
        }
      break;
      
			case 3 : // fadeIn
				lcd_puts_Pleft( y, XPSTR("Fade In") ) ;
    		lcd_outdezAtt(14*FW, y, phase->fadeIn * 5, attr | PREC1 ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( phase->fadeIn, 15 ) ;
			break ;
      
			case 4 : // fadeOut
				lcd_puts_Pleft( y, XPSTR("Fade Out") ) ;
		    lcd_outdezAtt(14*FW, y, phase->fadeOut * 5, attr | PREC1 ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( phase->fadeOut, 15 ) ;
			break ;
			
			case 5 : // Name
				alphaEditName( 11*FW-2, y, (uint8_t *)phase->name, sizeof(phase->name), attr, (uint8_t *)XPSTR( "Mode Name") ) ;
//				if ( attr )
//				{
//					Columns = 5 ;
//				}
//				editName( 11*FW-2, g_posHorz, y, (uint8_t *)phase->name, sizeof(phase->name), attr ? EE_MODEL : 0, event ) ;
			break ;
	  }
	}
}

void menuModelPhases(uint8_t event)
{
	uint32_t i ;
  uint8_t attr ;
	
	TITLE(PSTR(STR_MODES));
	static MState2 mstate2;
  
	event = mstate2.check_columns( event, 7-1-1 ) ;
	
	uint8_t  sub    = mstate2.m_posVert ;
//	evalOffset(sub) ;

  switch (event)
	{
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE) :
//			if ( sub > 0 ) //&& sub <= MAX_MODES )
//			{
        s_currIdx = sub ;
        killEvents(event);
        pushMenu(menuPhaseOne) ;
//    	}
		break;
  }
    
  lcd_putc( 0, 1*FH, 'F' ) ;
  lcd_putc( 1*FW-1, 1*FH, '0' ) ;
	lcd_puts_P( 16*FW+2, 1*FH, XPSTR("\020RETA") ) ;
//  {
//    for ( i = 1 ; i <= 4 ; i += 1 )
//    {
//#ifdef FIX_MODE
//	  	lcd_putsAttIdx( (15+i)*FW-2, 1*FH, PSTR(STR_1_RETA), i-1, 0 ) ;
//#else
//  		lcd_putsAttIdx( (15+i)*FW-2, 1*FH, PSTR(STR_1_RETA), *(modn12x3+g_eeGeneral.stickMode*4+(i-1))-1, 0 ) ;
//#endif
//    }
//  }

  for ( i=0 ; i<MAX_MODES ; i += 1 )
	{
    uint8_t y=(i+2)*FH ;
		PhaseData *p = &g_model.phaseData[i] ;
    attr = (i == sub) ? INVERS : 0 ;
//    lcd_puts_Pleft( y, PSTR(STR_SP_FM) ) ;
    lcd_putc( 0, y, 'F' ) ;
    lcd_putc( 1*FW-1, y, '1'+i ) ;
		lcd_putsnAtt( 2*FW-1, y, g_model.phaseData[i].name, 6, /*BSS*/ attr ) ;
    putsDrSwitches( 8*FW, y, p->swtch, attr ) ;
    putsDrSwitches( 12*FW+1, y, p->swtch2, attr ) ;
    for ( uint8_t t = 0 ; t < NUM_STICKS ; t += 1 )
		{
			putsTrimMode( (16+t)*FW+2, y, i+1, t, attr ) ;
		}
		if ( p->fadeIn || p->fadeOut )
		{
	    lcd_putcAtt( 20*FW+2, y, '*', attr ) ;
		}
//    lcd_outdezAtt(18*FW+4, y, p->fadeIn * 5, attr | PREC1 ) ;
//    lcd_outdezAtt(21*FW+1, y, p->fadeOut * 5, attr | PREC1 ) ;
	}	 
	i = getFlightPhase() ;
	plotType = PLOT_BLACK ;
	lcd_rect( 0, (i+1)*FH-1, 2*FW-1, 9 ) ;
	plotType = PLOT_XOR ;
	// Cosmetic tidy up due to inverse of top line
//	if ( i == 1 )
//	{
//    lcd_hline( 0, FH-1, 4*FW+2 );
//	}
}

//const char *ModelPopList = PSTR(STR_MODEL_POPUP) ;

//#define XSTR_MODEL_POPUP    "EDIT\0SELECT\0COPY\0MOVE\0DELETE\0BACKUP\0RESTORE"

const char *BackResult ;

void menuProcModelSelect(uint8_t event)
{
  static MState2 mstate2;
  TITLE(PSTR(STR_MODELSEL));

  int8_t subOld  = mstate2.m_posVert;
	
	if ( !PopupData.PopupActive )
	{
		RotaryState = ROTARY_MENU_UD ;
		event = mstate2.check_columns( event, MAX_MODELS-1 ) ;
//  	mstate2.check_submenu_simple(event,MAX_MODELS-1) ;
	}

#ifdef PCBX9D
    lcd_puts_Pleft(  0, PSTR(STR_11_FREE));
    lcd_outdez(  18*FW, 0, EeFsGetFree());
#endif

//  DisplayScreenIndex(e_ModelSelect, DIM(menuTabModel), INVERS);

  int8_t  sub    = mstate2.m_posVert;
  static uint8_t sel_editMode;
  if ( DupIfNonzero == 2 )
  {
      sel_editMode = false ;
      DupIfNonzero = 0 ;
  }
  
	if(sub-s_pgOfs < 1)        s_pgOfs = max(0,sub-1);
  else if(sub-s_pgOfs >4 )  s_pgOfs = min(MAX_MODELS-6,sub-4);
  for(uint8_t i=0; i<6; i++)
	{
    uint8_t y=(i+2)*FH;
    uint8_t k=i+s_pgOfs;
    lcd_outdezNAtt(  3*FW, y, k+1, ((sub==k) ? INVERS : 0) + LEADING0,2);
    if(k==g_eeGeneral.currModel) lcd_putc(1,  y,'*');
    lcd_putsn_P(  4*FW, y, (char *)ModelNames[k+1],sizeof(g_model.name) ) ;
#ifdef PCBX9D
		if (ModelNames[k+1][0])
 		{
    	lcd_outdez( 20*FW, y, eeFileSize(k+1) ) ;
		}
#endif
		
		if ( (sub==k) && (sel_editMode ) )
		{
			lcd_rect( 0, y-1, 125, 9 ) ;
		}
//    lcd_putsnAtt(  4*FW, y, (char *)buf,sizeof(buf),/*BSS|*/((sub==k) ? (sel_editMode ? INVERS : 0 ) : 0));
  }

	if ( PopupData.PopupActive )
	{
		uint16_t mask ;
		if ( g_eeGeneral.currModel == mstate2.m_posVert )
		{
			mask = 0x59 ;
		}
		else
		{
			mask = ( eeModelExists( mstate2.m_posVert ) == 0 ) ?  0x86 :  0x017E ;
		}

		uint8_t popaction = doPopup( PSTR(STR_MODEL_POPUP), mask, 10, event ) ;
//		count = popupDisplay( PSTR(STR_MODEL_POPUP), mask, 8 ) ;
		
//		uint8_t popaction = popupProcess( event, count - 1 ) ;
//		uint8_t popidx = PopupIdx ;
//		lcd_char_inverse( 4*FW, (popidx+1)*FH, 6*FW, 0 ) ;
//		popidx = popTranslate( popidx, mask ) ;
		
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
				WatchdogTimeout = 200 ;		// 2 seconds
				pausePulses() ;
#ifdef PCBX9D				
				ee32_check_finished() ;
        g_eeGeneral.currModel = mstate2.m_posVert;
				eeLoadModel( g_eeGeneral.currModel ) ;
#endif
#if defined(PCBSKY) || defined(PCB9XT)
        g_eeGeneral.currModel = mstate2.m_posVert;
        ee32WaitLoadModel(g_eeGeneral.currModel); //load default values
#endif
				checkSwitches() ;
  			checkTHR();
				checkCustom() ;
 				perOut( g_chans512, NO_DELAY_SLOW | FADE_FIRST | FADE_LAST ) ;
				SportStreamingStarted = 0 ;
				speakModelVoice() ;
        resetTimer();
				VoiceCheckFlag100mS |= 2 ;// Set switch current states
				telemetry_init( decodeTelemetryType( g_model.telemetryProtocol ) ) ;
				resumePulses() ;
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
//        s_editMode = false;
//        s_noHi = NO_HI_LEN;
//  	    if(g_eeGeneral.currModel != mstate2.m_posVert)
//				{ // Don't allow current model to be deleted, now not selectable!
       	killEvents(event);
       	DupIfNonzero = 0 ;
				DupSub = sub + 1 ;
       	pushMenu(menuDeleteDupModel);
//				}
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
				WatchdogTimeout = 200 ;		// 2 seconds
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
				WatchdogTimeout = 200 ;		// 2 seconds
				BackResult = ee32BackupModel( mstate2.m_posVert+1 ) ;
				AlertType = MESS_TYPE ;
				AlertMessage = BackResult ;
				RestoreIndex = mstate2.m_posVert+1 ;
       	pushMenu( menuProcRestore ) ;				
			}
			else // Move = 4
			{
 	    	sel_editMode = true ;
			}
		}
//		else if ( popaction == POPUP_EXIT )
//		{
//			killEvents( event ) ;
//			PopupData.PopupActive = 0 ;
//		}
//		s_moveMixIdx = s_currMixIdx ;
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
//  	        audioDefevent(AU_MENUS);
//  	        killEvents(event);
//						ee32WaitLoadModel(g_eeGeneral.currModel = mstate2.m_posVert) ;
//						putVoiceQueue( g_model.modelVoice + 260 ) ;
	//          eeWaitComplete();    // Wait to load model if writing something
	//          eeLoadModel(g_eeGeneral.currModel = mstate2.m_posVert);
//  	        resetTimer();
//  	        STORE_GENERALVARS;
	//          eeWaitComplete();
	//          STORE_MODELVARS;
	//          eeWaitComplete();
//  	        break;
  	    }
      break ;

	  	case  EVT_KEY_FIRST(KEY_LEFT):
  		case  EVT_KEY_FIRST(KEY_RIGHT):
  	    if(g_eeGeneral.currModel != mstate2.m_posVert)
  	    {
          killEvents(event);
					PopupData.PopupIdx = 0 ;
					PopupData.PopupActive = 2 ;
//  	        killEvents(event);
	//          g_eeGeneral.currModel = mstate2.m_posVert;
	//          ee32WaitLoadModel(g_eeGeneral.currModel); //load default values
	//					putVoiceQueue( g_model.modelVoice + 260 ) ;
	////          eeWaitComplete();    // Wait to load model if writing something
	////          eeLoadModel(g_eeGeneral.currModel);
	//          resetTimer();
	//          STORE_GENERALVARS;
	////          eeWaitComplete();
//  	        audioDefevent(AU_WARNING2);
  	    }
				else
				{
					RotaryState = ROTARY_MENU_LR ;
		      if(event==EVT_KEY_FIRST(KEY_LEFT))  chainMenu(menuProcModelIndex);//{killEvents(event);popMenu(true);}

//	#ifndef NO_TEMPLATES
//	#if GVARS
//		      if(event==EVT_KEY_FIRST(KEY_LEFT))  chainMenu(menuProcGlobals);//{killEvents(event);popMenu(true);}
//	#else
//  		    if(event==EVT_KEY_FIRST(KEY_LEFT))  chainMenu(menuProcTemplates);//{killEvents(event);popMenu(true);}
//	#endif
//	//#elif defined(FRSKY)
//	//      if(event==EVT_KEY_FIRST(KEY_LEFT))  chainMenu(menuProcTelemetry);//{killEvents(event);popMenu(true);}
//	#else
//	#if GVARS
//  	  	  if(event==EVT_KEY_FIRST(KEY_LEFT))  chainMenu(menuProcGlobals);//{killEvents(event);popMenu(true);}
//	#else
//  	    	if(event==EVT_KEY_FIRST(KEY_LEFT))  chainMenu(menuProcSafetySwitches);//{killEvents(event);popMenu(true);}
//	#endif
//	#endif
		      if(event==EVT_KEY_FIRST(KEY_RIGHT)) chainMenu(menuProcModelIndex);
  	    //      if(event==EVT_KEY_FIRST(KEY_EXIT))  chainMenu(menuProcModelSelect);
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
//  	    audioDefevent(AU_MENUS);
  	  break;
  	
			case  EVT_KEY_LONG(BTN_RE) :
			case  EVT_KEY_LONG(KEY_EXIT):  // make sure exit long exits to main
  	    popMenu(true);
      break;
//  	case  EVT_KEY_LONG(KEY_MENU):
//  	    if(sel_editMode){

//  	        DupIfNonzero = 1 ;
//  	        DupSub = sub + 1 ;
//  	        pushMenu(menuDeleteDupModel);//menuProcExpoAll);

//  	        //        message(PSTR("Duplicating model"));
//  	        //        if(eeDuplicateModel(sub)) {
//  	        //          audioDefevent(AU_MENUS);
//  	        //          sel_editMode = false;
//  	        //        }
//  	        //        else audioDefevent(AU_WARNING1);
//  	    }
//  	    break;

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


//const char *menuWhenDone = PSTR(STR_MENU_DONE) ;


void menuProcDiagCalib(uint8_t event)
{
  //    scroll_disabled = 1; // make sure we don't flick out of the screen
//  SIMPLE_MENU(PSTR(STR_CALIBRATION), menuTabDiag, e_Calib, 2);
	TITLE(PSTR(STR_Calibration));
	static MState2 mstate2 ;
	mstate2.check_columns( event, 2-1 ) ;

  //    int8_t  sub    = mstate2.m_posVert ;
  mstate2.m_posVert = 0 ; // make sure we don't scroll or move cursor here

#if defined(PCBSKY) || defined(PCB9XT)
	uint32_t numAnalogCals = NUM_ANALOG_CALS + countExtraPots() ;
#else
	uint32_t numAnalogCals = NUM_ANALOG_CALS ;
#endif

  for(uint32_t i=0; i < numAnalogCals ; i++)
	{ //get low and high vals for sticks and trims
		
//#ifdef REVPLUS
		int16_t vt = anaIn( i ) ;
//#else
//		int16_t vt = anaIn( i ) ;
//		int16_t vt = anaIn( i ) ;
//#endif
    Xmem.Cal_data.loVals[i] = min(vt,Xmem.Cal_data.loVals[i]);
    Xmem.Cal_data.hiVals[i] = max(vt,Xmem.Cal_data.hiVals[i]);
    if(i>=4)
		{
#ifdef PCBX9D
	    if(i < 6)			
#endif
				Xmem.Cal_data.midVals[i] = (Xmem.Cal_data.loVals[i] + Xmem.Cal_data.hiVals[i])/2;
		}
    //if(i>=4) midVals[i] = (loVals[i] + hiVals[i])/2;
  }

  //    if(sub==0)
  //        idxState = 0;

  scroll_disabled = Xmem.Cal_data.idxState; // make sure we don't scroll while calibrating

  switch(event)
  {
	  case EVT_ENTRY:
      Xmem.Cal_data.idxState = 0;
    break;

  	case EVT_KEY_BREAK(KEY_MENU):
  	case EVT_KEY_BREAK(BTN_RE):
      Xmem.Cal_data.idxState++;
      if(Xmem.Cal_data.idxState==3)
      {
        audioDefevent(AU_MENUS);
        STORE_GENERALVARS;     //eeWriteGeneral();
        Xmem.Cal_data.idxState = 0;
//				g_eeGeneral.calibrateWarning = 1 ;
      }
    break;
  }

    switch(Xmem.Cal_data.idxState)
  {
  	case 0:
      //START CALIBRATION
      //[MENU]
      lcd_puts_Pleft( 3*FH, PSTR(STR_MENU_TO_START) ) ;//, 15, sub>0 ? INVERS : 0);
    break;

	  case 1: //get mid
      //SET MIDPOINT
      //[MENU]
      
  		for(uint8_t i=0; i<numAnalogCals; i++)
      {
          Xmem.Cal_data.loVals[i] =  15000;
          Xmem.Cal_data.hiVals[i] = -15000;
//#ifdef REVPLUS
//          Xmem.Cal_data.midVals[i] = anaIn( i );
//#else
          Xmem.Cal_data.midVals[i] = anaIn( i );
//          Xmem.Cal_data.midVals[i] = anaIn(i);
//#endif
      }
      lcd_puts_Pleft( 2*FH, PSTR(STR_SET_MIDPOINT) ) ;//, 12, sub>0 ? INVERS : 0);
      lcd_puts_P(3*FW, 3*FH, PSTR(STR_MENU_DONE) ) ;//, 16, sub>0 ? BLINK : 0);
    break;

	  case 2:
      //MOVE STICKS/POTS
      //[MENU]
			StickScrollAllowed = 0 ;

      for(uint8_t i=0; i<numAnalogCals; i++)
			{
	      if(abs(Xmem.Cal_data.loVals[i]-Xmem.Cal_data.hiVals[i])>50)
				{
          *CalibMid[i]  = Xmem.Cal_data.midVals[i] ;
          int16_t v = Xmem.Cal_data.midVals[i] - Xmem.Cal_data.loVals[i] ;
          *CalibSpanNeg[i] = v - v/64 ;
          v = Xmem.Cal_data.hiVals[i] - Xmem.Cal_data.midVals[i] ;
          *CalibSpanPos[i] = v - v/64 ;
        }
			}
  		g_eeGeneral.chkSum = evalChkSum() ;
//#ifdef PCBX9D
//			{
//  			int16_t v = Xmem.Cal_data.midVals[7] - Xmem.Cal_data.loVals[7] ;
//  			g_eeGeneral.x9dcalibMid = Xmem.Cal_data.midVals[7] ;
//  			g_eeGeneral.x9dcalibSpanNeg = v - v/64 ;
//        v = Xmem.Cal_data.hiVals[7] - Xmem.Cal_data.midVals[7] ;
//  			g_eeGeneral.x9dcalibSpanPos = v - v/64 ;
//			}
//#endif
      lcd_puts_Pleft( 2*FH, PSTR(STR_MOVE_STICKS) ) ; //, 16, sub>0 ? INVERS : 0);
      lcd_puts_P(3*FW, 3*FH, PSTR(STR_MENU_DONE) ) ; //, 16, sub>0 ? BLINK : 0);
    break;
  }

  doMainScreenGrphics() ;
#if defined(PCBSKY) || defined(PCB9XT)
	if ( g_eeGeneral.extraPotsSource[1] )
	{
		uint8_t len ;
    len = ((calibratedStick[8]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
    V_BAR( 101, SCREEN_HEIGHT-8, len ) ;
	}
#endif
#ifdef REV9E
	uint8_t len ;
  len = ((calibratedStick[8]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
  V_BAR( 26, SCREEN_HEIGHT-8, len ) ;
  len = ((calibratedStick[9]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
  V_BAR( 101, SCREEN_HEIGHT-8, len ) ;
//  len = ((calibratedStick[10]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
//  V_BAR( 30, SCREEN_HEIGHT-8, len ) ;
//  len = ((calibratedStick[11]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
//  V_BAR( 40, SCREEN_HEIGHT-8, len ) ;
#endif
}

uint16_t Current ;
uint32_t Current_sum ;
uint8_t Current_count ;

#if defined(PCBSKY) || defined(PCB9XT)
#define NUM_ANA_ITEMS		3
#endif
#ifdef PCBX9D
#define NUM_ANA_ITEMS		2
#endif

void menuProcDiagAna(uint8_t event)
{
	register uint32_t i ;
	TITLE(PSTR(STR_ANA));
	static MState2 mstate2;
	mstate2.check_columns(event, NUM_ANA_ITEMS-1) ;

#ifdef PCBSKY
 #ifdef REVB    
	if ( g_eeGeneral.ar9xBoard == 0 )
	{
		Current_sum += Current_current ;
		if ( ++Current_count > 49 )
		{
			Current = Current_sum / 50 ;
			Current_sum = 0 ;
			Current_count = 0 ;
		}
	}
 #endif
#endif

  int8_t  sub    = mstate2.m_posVert ;
	StickScrollAllowed = 0 ;
  for(i=0; i<8; i++)
  {
    uint8_t y=i*FH;
		lcd_putsAttIdx( 4*FW-3, y, XPSTR("\002LHLVRVRHA5A6A7BT"), i, 0 ) ;
//    lcd_putc( 4*FW-3, y, 'A' ) ;
//    lcd_putc( 5*FW-3, y, '1'+i ) ;
    lcd_outhex4( 6*FW, y,anaIn(i));
		uint8_t index = i ;
		if ( i < 4 )
		{
 			index = stickScramble[g_eeGeneral.stickMode*4+i] ;
 		}
    if(i<7)  lcd_outdezAtt(15*FW, y, (int32_t)calibratedStick[index]*1000/1024, PREC1);
    if(i==7)
		{
			putsVBat(14*FW,y,(sub==1 ? InverseBlink : 0)|PREC1);
    	lcd_outhex4( 6*FW, y,anaIn(12));
//#ifdef PCBX9D
//    	lcd_outhex4( 6*FW, y,anaIn(8));
//#endif
//#ifdef PCB9XT
//    	lcd_outhex4( 6*FW, y,anaIn(4));
////    	lcd_outhex4( 16*FW, y, g_eeGeneral.vBatCalib ) ;
//#endif
		}
  }

#ifdef PCBSKY
	if ( g_eeGeneral.ar9xBoard )
	{
		lcd_puts_Pleft( 4*FH, XPSTR("\020AD10"));
//	  lcd_putc( 18*FW, 4*FH, 'A' ) ;
//  	lcd_putc( 19*FW, 4*FH, '8' ) ;
  	lcd_outhex4( 17*FW, 5*FH,Analog_values[8]);
	}
#endif

#if defined(PCBSKY) || defined(PCB9XT)
 #ifdef REVB    
	if ( g_eeGeneral.ar9xBoard )
	{
		lcd_puts_Pleft( 4*FH, XPSTR("\020AD8"));
	}
	else
	{
	  lcd_putc( 18*FW, 2*FH, 'A' ) ;
  	lcd_putc( 19*FW, 2*FH, '9' ) ;
	}
  lcd_outhex4( 17*FW, 3*FH,Analog_values[9]);
 #endif
#endif

#ifdef PCB9XT
	lcd_outhex4( 100, 5*FH, Analog_values[5] ) ;
	lcd_outhex4( 100, 6*FH, Analog_values[6] ) ;
	lcd_outhex4( 100, 7*FH, Analog_values[7] ) ;
#endif

#ifdef PCBX9D
#ifdef REVPLUS
  lcd_putc( 18*FW, 2*FH, 'A' ) ;
  lcd_putc( 19*FW, 2*FH, '9' ) ;
  lcd_outhex4( 17*FW, 3*FH,anaIn(7));
  lcd_outdezAtt(20*FW, 4*FH, (int32_t)calibratedStick[7]*1000/1024, PREC1);
  lcd_putc( 18*FW, 5*FH, 'A' ) ;
  lcd_putc( 19*FW, 5*FH, '1' ) ;
  lcd_putc( 20*FW, 5*FH, '0' ) ;
  lcd_outhex4( 17*FW, 6*FH,anaIn(9));
  lcd_outdezAtt(20*FW, 7*FH, (int32_t)calibratedStick[8]*1000/1024, PREC1);
#else
  lcd_putc( 18*FW, 5*FH, 'A' ) ;
  lcd_putc( 19*FW, 5*FH, '9' ) ;
  lcd_outhex4( 17*FW, 6*FH,anaIn(7));
  lcd_outdezAtt(20*FW, 7*FH, (int32_t)calibratedStick[7]*1000/1024, PREC1);
#endif

#ifdef REV9E
  lcd_putc( 22*FW, 5*FH, 'A' ) ;
  lcd_putc( 23*FW, 5*FH, '1' ) ;
  lcd_putc( 24*FW, 5*FH, '1' ) ;
  lcd_outhex4( 21*FW, 6*FH,anaIn(10));
  lcd_outdezAtt(24*FW, 7*FH, (int32_t)calibratedStick[9]*1000/1024, PREC1);
  lcd_outhex4( 25*FW, 6*FH,anaIn(11));
  lcd_outhex4( 25*FW, 7*FH,anaIn(12));
#endif

#endif
  if(sub==1)
  {
    scroll_disabled = 1;
		if ( s_editMode )
		{
	    CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatCalib, -127, 127);
		}
  }
#ifdef PCBSKY
  if(sub==2)
	{
		if ( s_editMode )
		{
	    CHECK_INCDEC_H_GENVAR( g_eeGeneral.current_calib, -49, 49 ) ;
		}
	}
  scroll_disabled = 1;
#ifndef REVX
	if ( g_eeGeneral.ar9xBoard == 0 )
#endif
	{
		lcd_puts_Pleft( 5*FH, XPSTR("\022mA"));
	  lcd_outdezAtt( 21*FW, 6*FH, Current/10 , (sub==2 ? InverseBlink : 0) ) ;
	}
#endif

}

void putc_0_1( uint8_t x, uint8_t y, uint8_t value )
{
  lcd_putcAtt( x, y, value+'0', value ? INVERS : 0 ) ;
}


void menuProcDiagKeys(uint8_t event)
{
//  SIMPLE_MENU(PSTR(STR_DIAG), menuTabDiag, e_Keys, 1);
	TITLE(PSTR(STR_DIAG));
	static MState2 mstate2;
//	if (SubMenuFromIndex)
//	{
		mstate2.check_columns(event, 1-1) ;
//		mstate2.check_simple(event,0,NULL,0,1-1) ;
//	}
//	else
//	{
//		mstate2.check_simple(event,e_Keys,menuTabDiag,DIM(menuTabDiag),1-1) ;
//	}

  uint8_t x ;

#ifdef PCBX9D

#ifdef REV9E
	x = 7*FW;
#else  
	x = 8*FW;
#endif	// REV9E
  for(uint8_t i=0; i<8; i++)
	{
    uint8_t y=i*FH; //+FH;
		lcd_putsAttIdx( x, y, PSTR(HW_SWITCHES_STR), i, 0 ) ;
		uint32_t pos = switchPosition( i ) ;
		lcd_putc(x+2*FW, y, PSTR(HW_SWITCHARROW_STR)[pos] ) ;
	}

#ifndef REV9E
	if ( g_eeGeneral.analogMapping & MASK_6POS )
	{
		uint8_t k = HSW_Ele6pos0 ;
		k += switchPosition(k) ;
		lcd_putsAttIdx( 13*FW, 0, PSTR(SWITCHES_STR), k-HSW_OFFSET, 0 ) ;
	}
#endif	// nREV9E

#ifdef REV9E
	x = 11*FW - 2;
  for(uint8_t i=0; i<8; i++)
	{
    uint8_t y=i*FH; //+FH;
		lcd_putsAttIdx( x, y, PSTR(HW_SWITCHES_STR), i+8, 0 ) ;
		uint32_t pos = switchPosition( i+8 ) ;
		lcd_putc(x+2*FW, y, PSTR(HW_SWITCHARROW_STR)[pos] ) ;
	}
  x=14*FW;
  for(uint8_t i=0; i<2; i++)
	{
    uint8_t y=i*FH; //+FH;
		lcd_putsAttIdx( x, y, PSTR(HW_SWITCHES_STR), i+16, 0 ) ;
		uint32_t pos = switchPosition( i+16 ) ;
		lcd_putc(x+2*FW, y, PSTR(HW_SWITCHARROW_STR)[pos] ) ;
	}
#endif	// REV9E

#endif	// PCBX9D

#if defined(PCBSKY) || defined(PCB9XT)
  x = 7*FW;
  for(uint8_t i=0; i<9; i++)
  {
    uint8_t y=i*FH; //+FH;
		uint8_t displayType = 0 ;
    if(i>(SW_ID0-SW_BASE_DIAG)) y-=FH; //overwrite ID0
    uint8_t t=keyState((EnumKeys)(SW_BASE_DIAG+i));
		switch ( i )
		{
			case 0 :	// THR
				if ( g_eeGeneral.switchMapping & USE_THR_3POS )
				{
					t = switchPosition( HSW_Thr3pos0 ) ;
					displayType = 1 ;
				}
			break ;
			case 1 :	// RUD
				if ( g_eeGeneral.switchMapping & USE_RUD_3POS )
				{
					t = switchPosition( HSW_Rud3pos0 ) ;
					displayType = 1 ;
				}
			break ;
			case 2 :	// ELE
				if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
				{
					t = switchPosition( HSW_Ele3pos0 ) ;
					displayType = 1 ;
				}
			break ;
			case 3 :	// IDx
				t = switchPosition( HSW_ID0 ) ;
				displayType = 1 ;
			break ;
			case 6 :	// AIL
				y -= FH ;
				if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
				{
					t = switchPosition( HSW_Ail3pos0 ) ;
					displayType = 1 ;
				}
			break ;
			case 7 :	// GEA
				y -= FH ;
				if ( g_eeGeneral.switchMapping & USE_GEA_3POS )
				{
					t = switchPosition( HSW_Gear3pos0 ) ;
					displayType = 1 ;
				}
			break ;
			case 8 :	// TRN
				y -= FH ;
			break ;
		}
		if ( ( i < 4 ) || ( i > 5 ) )
		{
    	putsDrSwitches(x,y,i+1,0); //ohne off,on
			if ( displayType == 0 )
			{
    		lcd_putcAtt(x+FW*4+2,  y,t+'0',t ? INVERS : 0);
			}
			else
			{
				lcd_putc(x+FW*4+2, y, PSTR(HW_SWITCHARROW_STR)[t] ) ;
			}
		}
  }
#endif

  x=0;
  for(uint8_t i=0; i<6; i++)
  {
    uint8_t y=(5-i)*FH+2*FH;
    bool t=keyState((EnumKeys)(KEY_MENU+i));
   	lcd_putsAttIdx(  x, y, PSTR(STR_KEYNAMES),i,0);
    lcd_putcAtt(x+FW*5+2,  y,t+'0',t);
  }
  lcd_puts_Pleft(1*FH,XPSTR("  Enc") ) ;
  bool t=keyState((EnumKeys)(BTN_RE));
  lcd_putcAtt(x+FW*5+2, 1*FH,t+'0',t);

  x=14*FW;
  lcd_putsn_P(x, 3*FH,PSTR(STR_TRIM_M_P),7);
  for(uint8_t i=0; i<4; i++)
  {
    uint8_t y=i*FH+FH*4;
    lcd_img(    x,       y, sticks,i,0);
    bool tm=keyState((EnumKeys)(TRM_BASE+2*i));
    bool tp=keyState((EnumKeys)(TRM_BASE+2*i+1));
    lcd_putcAtt(x+FW*4,  y, tm+'0',tm ? INVERS : 0);
    lcd_putcAtt(x+FW*6,  y, tp+'0',tp ? INVERS : 0);
  }

#if defined(PCBSKY) || defined(PCB9XT)
	if ( g_eeGeneral.analogMapping & MASK_6POS )
	{
		uint8_t k = HSW_Ele6pos0 ;
		k += switchPosition(k) ;
		lcd_putsAttIdx( 13*FW, 0, PSTR(SWITCHES_STR), k-HSW_OFFSET, 0 ) ;
	}
#endif
	 
#if defined(PCBSKY) || defined(PCB9XT)
	if ( g_eeGeneral.switchMapping & USE_PB1 )
	{
	  lcd_puts_Pleft(7*FH,XPSTR("\010PB1") ) ;
		putc_0_1( FW*11+2, 7*FH, getSwitch00(HSW_Pb1) ) ;
	}
	
	if ( g_eeGeneral.switchMapping & ( USE_PB2 | USE_PB3 | USE_PB4 ) )
	{
	  lcd_puts_Pleft(1*FH,XPSTR("\017PB") ) ;
	}
	if ( g_eeGeneral.switchMapping & USE_PB2 )
	{
	  lcd_puts_Pleft(1*FH,XPSTR("\0212") ) ;
		putc_0_1( FW*17, 2*FH, getSwitch00(HSW_Pb2) ) ;
	}
	if ( g_eeGeneral.switchMapping & USE_PB3 )
	{
	  lcd_puts_Pleft(1*FH,XPSTR("\0223") ) ;
		putc_0_1( FW*18, 2*FH, getSwitch00(HSW_Pb3) ) ;
	}
	if ( g_eeGeneral.switchMapping & USE_PB4 )
	{
	  lcd_puts_Pleft(1*FH,XPSTR("\0234") ) ;
		putc_0_1( FW*19, 2*FH, getSwitch00(HSW_Pb4) ) ;
	}
#endif

#ifdef PCBX9D
	
	configure_pins( 0x6000, PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
	
	if ( g_eeGeneral.switchMapping & USE_PB1 )
	{
	  lcd_puts_Pleft(1*FH,XPSTR("\017PB1") ) ;
		putc_0_1( FW*19+2, 1*FH, getSwitch00(HSW_Pb1) ) ;
	}
	
	if ( g_eeGeneral.switchMapping & USE_PB2 )
	{
	  lcd_puts_Pleft(2*FH,XPSTR("\017PB2") ) ;
		putc_0_1( FW*19+2, 2*FH, getSwitch00(HSW_Pb2) ) ;
	}
#endif

//extern uint8_t EncoderI2cData[2] ;
//lcd_outhex4( 100, 0, (EncoderI2cData[0] << 8 ) | EncoderI2cData[1] ) ;

//#ifdef REV9E
//  lcd_outhex4( 22*FW, 5*FH, GPIOA->IDR ) ;
//  lcd_outhex4( 22*FW, 6*FH, GPIOB->IDR ) ;
//  lcd_outhex4( 22*FW, 7*FH, GPIOC->IDR ) ;
  
//	lcd_outhex4( 26*FW, 5*FH, GPIOD->IDR ) ;
//  lcd_outhex4( 26*FW, 6*FH, GPIOE->IDR ) ;
//  lcd_outhex4( 26*FW, 7*FH, GPIOF->IDR ) ;
	
//	lcd_outhex4( 30*FW, 5*FH, GPIOG->IDR ) ;
//#endif

}

void menuProcDiagVers(uint8_t event)
{
	TITLE(PSTR(STR_Version));
	static MState2 mstate2;
//	if (SubMenuFromIndex)
//	{
		mstate2.check_columns(event, 1-1) ;
//		mstate2.check_simple(event,0,NULL,0,1-1) ;
//	}
//	else
//	{
//		mstate2.check_simple(event,e_Vers,menuTabDiag,DIM(menuTabDiag),1-1) ;
//	}

    lcd_puts_Pleft( 2*FH,Stamps );
//    lcd_puts_Pleft( 2*FH,stamp4 );
//    lcd_puts_Pleft( 3*FH,stamp1 );
//    lcd_puts_Pleft( 4*FH,stamp2 );
//    lcd_puts_Pleft( 5*FH,stamp3 );
//    lcd_puts_Pleft( 6*FH,stamp5 );
		
#ifdef PCBSKY
#ifndef REVX
	if ( g_eeGeneral.ar9xBoard == 0 )
	{
		lcd_puts_Pleft( 7*FH, XPSTR("Co Proc"));
    lcd_outhex4( 10*FW-3, 7*FH, (Coproc_valid << 8 ) + Coproc_read ) ;
	}
#endif
 #endif
}


#ifdef PCBSKY
#ifdef REVX
#define NUM_SETUP1_ITEMS	13
#define NUM_SETUP1_PG1		7
#else
#define NUM_SETUP1_ITEMS	14
#define NUM_SETUP1_PG1		8
#endif
#endif
#ifdef PCBX9D
#ifdef REVPLUS
#define NUM_SETUP1_ITEMS	7
#define NUM_SETUP1_PG1		6
#else
#define NUM_SETUP1_ITEMS	6
#define NUM_SETUP1_PG1		5
#endif
#endif

// This is debug for a new file system
void menuProcSetup2(uint8_t event)
{
#if defined(PCBSKY) || defined(PCB9XT)
	uint32_t i ;
#endif
	uint32_t j ;
//  MENU(PSTR(STR_MEMORY_STAT), menuTabDiag, e_Setup2, 15, {0/*, 0*/});
	MENU(PSTR(STR_MEMORY_STAT), menuTabStat, e_Setup2, MAX_MODELS - 5, {0} ) ;
	
	int8_t  sub    = mstate2.m_posVert;
//	uint8_t subSub = mstate2.m_posHorz;
//	bool    edit;
//	uint8_t blink ;

//	evalOffset(sub);

	j = sub + 1 ;
	if ( j > MAX_MODELS - 5 )
	{
		j = MAX_MODELS - 5 ;
	}
	lcd_puts_Pleft( 1*FH, PSTR(STR_GENERAL));
#if defined(PCBSKY) || defined(PCB9XT)
  lcd_outhex4( 8*FW+3, 1*FH, File_system[0].block_no ) ;
  lcd_outhex4( 12*FW+3, 1*FH, File_system[0].sequence_no ) ;
  lcd_outhex4( 16*FW+3, 1*FH, File_system[0].size ) ;
	for ( i = 1 ; i < 7 ; i += 1 )
	{
		lcd_puts_Pleft( (i+1)*FH, PSTR(STR_Model));
		lcd_putc( 5*FW, (i+1)*FH, j/10 + '0' ) ;
		lcd_putc( 6*FW, (i+1)*FH, j%10 + '0' ) ;
  	lcd_outhex4( 8*FW+3, (i+1)*FH, File_system[j].block_no ) ;
  	lcd_outhex4( 12*FW+3, (i+1)*FH, File_system[j].sequence_no ) ;
  	lcd_outhex4( 16*FW+3, (i+1)*FH, File_system[j].size ) ;
		j += 1 ;
	}
#endif

//#ifdef REVX
//	// RTC debug
//	extern uint8_t RtcConfig[] ;
//	extern uint8_t Rtc_status[] ;
//	for ( i = 0 ; i < 8 ; i += 1 )
//	{
//  	lcd_outhex2( i*2*FW, 6*FH, RtcConfig[i] ) ;
//  	lcd_outhex2( i*2*FW, 7*FH, Rtc_status[i] ) ;
//	}
//#endif
}

void menuProcRSSI(uint8_t event)
{
	TITLE( XPSTR( "Module RSSI" ) ) ;
	static MState2 mstate2 ;
//	static const uint8_t mstate_tab[] = {1} ;

//	event = mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1,2-1) ;
	event = mstate2.check_columns( event, 2-1 ) ;
	Columns = 1 ;
	
	uint8_t  sub = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz ;
	uint8_t blink;
	uint8_t y = 1*FH;

	switch(event)
	{
    case EVT_KEY_BREAK(KEY_DOWN):
    case EVT_KEY_BREAK(KEY_UP):
    case EVT_KEY_BREAK(KEY_LEFT):
    case EVT_KEY_BREAK(KEY_RIGHT):
      if(s_editMode)
         FrskyAlarmSendState |= 0x30 ;	 // update Fr-Sky module when edit mode exited
    break ;
		case EVT_ENTRY :
  		FrskyAlarmSendState |= 0x40 ;		// Get RSSI/TSSI alarms
		break ;
	}
	blink = InverseBlink ;
	
	uint8_t subN = 0 ;

	for (uint8_t j=0; j<2; j++)
	{
  	lcd_puts_Pleft( y, PSTR(STR_TX_RSSIALRM) );
  	if ( j == 1 )
  	{
  	    lcd_putcAtt( 0, y, 'R', 0 ) ;
  	}
  	lcd_putsAttIdx(11*FW, y, PSTR(STR_YELORGRED),frskyRSSItype[j],(sub==subN && subSub==0 ? blink:0));
  	lcd_outdezNAtt(17*FW, y, frskyRSSIlevel[j], (sub==subN && subSub==1 ? blink:0), 3);

  	if(sub==subN && (s_editMode || P1values.p1valdiff))
		{
 	    switch (subSub)
			{
  	    case 0:
  	   	  frskyRSSItype[j] = checkIncDec( frskyRSSItype[j], 0, 3, 0 ) ;
  	    break;
  	    case 1:
  	   	  frskyRSSIlevel[j] = checkIncDec16( frskyRSSIlevel[j], 0, 120, 0 ) ;
  	    break;
  	  }
  	}
  	subN++; y+=FH;
	}
}

//void saveTrainerProfile( uint32_t index )
//{
//	uint32_t i ;
//	for ( i = 0 ; i < 4 ; i += 1 )
//	{
//	  TrainerMix *td = &g_eeGeneral.trainer.mix[i];
//	  exTrainerMix *xtd = &g_eeGeneral.exTrainer[i] ;
//		TrainerChannel *tc = &g_eeGeneral.trainerProfile[index].channel[i] ;
//		tc->calib = g_eeGeneral.trainer.calib[i] ;
//		tc->srcChn = td->srcChn ;
//		tc->mode = td->mode ;
//		tc->swtch = xtd->swtch ;
//		tc->studWeight = xtd->studWeight ;
//		if ( i == 0 )
//		{
//			tc->source = g_eeGeneral.trainerSource ;
//		}
//	}
//}

//void loadTrainerProfile( uint32_t index )
//{
//	uint32_t i ;
//	for ( i = 0 ; i < 4 ; i += 1 )
//	{
//	  TrainerMix *td = &g_eeGeneral.trainer.mix[i];
//	  exTrainerMix *xtd = &g_eeGeneral.exTrainer[i] ;
//		TrainerChannel *tc = &g_eeGeneral.trainerProfile[index].channel[i] ;
//		g_eeGeneral.trainer.calib[i] = tc->calib ;
//		td->srcChn = tc->srcChn ;
//		td->mode = tc->mode ;
//		xtd->swtch = tc->swtch ;
//		xtd->studWeight = tc->studWeight ;
//		if ( i == 0 )
//		{
//			g_eeGeneral.trainerSource = tc->source ;
//		}
//	}
	
//}


// From Bertrand, allow trainer inputs without using mixers.
// Raw trianer inputs replace raw sticks.
// Only first 4 PPMin may be calibrated.
void menuProcTrainer(uint8_t event)
{
	
//  SUBMENU(PSTR(STR_TRAINER), 7, {0, 3, 3, 3, 3, 0/*, 0*/});
//  MENU(PSTR(STR_TRAINER), menuTabDiag, e_Trainer, 7, {0, 3, 3, 3, 3, 0/*, 0*/});
#define NUM_TRAINER_LINES	7	
	uint8_t y;
	uint32_t lines = NUM_TRAINER_LINES-1 ;
	y = g_eeGeneral.CurrentTrainerProfile ;
	if ( SingleExpoChan )
	{
		y = s_expoChan ;
	}
	
	TrainerProfile *tProf = &g_eeGeneral.trainerProfile[y] ;
	if ( ( g_model.trainerProfile	!= y ) || ( tProf->channel[0].source == TRAINER_SLAVE ) )
	{
		lines -= 1 ;
	}
	
	TITLE(PSTR(STR_Trainer));
	static MState2 mstate2;
	static const uint8_t mstate_tab[] = {1, 1, 3, 3, 3, 3, 0/*, 0*/} ;

	 
//	if (SubMenuFromIndex)
//	{
		event = mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1,lines) ;
//	}
//	else
//	{
//		event = mstate2.check(event,e_Trainer,menuTabDiag,DIM(menuTabDiag),mstate_tab,DIM(mstate_tab)-1,7-1) ;
//	}

	if ( event == EVT_ENTRY_UP )
	{ // From alphaEdit
		g_posHorz = 0 ;
	}

	int8_t  sub    = mstate2.m_posVert;
	uint8_t subSub = g_posHorz;
	bool    edit;
	uint8_t blink = InverseBlink ;


#if defined(PCBSKY) || defined(PCB9XT)
	if ( check_soft_power() == POWER_TRAINER )		// On trainer power
	{ // i am the slave
		if ( tProf->channel[0].source == TRAINER_JACK )
		{
	    lcd_puts_P( 7*FW, 3*FH, PSTR(STR_SLAVE) ) ;
	    return ;
		}
	}
#endif

	uint8_t attr = (sub==0 && subSub==0) ? blink : 0 ;
//	uint8_t previousProfile = g_eeGeneral.CurrentTrainerProfile ;
	lcd_putcAtt( 45, 0, y + '0', attr ) ;
	
	if ( SingleExpoChan )
	{
		attr = 0 ;
	}
	if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.CurrentTrainerProfile, 3 ) ;
	}
//	if ( previousProfile != g_eeGeneral.CurrentTrainerProfile )
//	{
//		// save current, load new
//		saveTrainerProfile( previousProfile ) ;
//		loadTrainerProfile( g_eeGeneral.CurrentTrainerProfile ) ;
//	}


//#ifdef PCBX9D
	attr = 0 ;
	lcd_puts_Pleft( 0, XPSTR("\011Source:"));
	if ( ( sub == 0 ) && ( subSub == 1 ) )
	{
		attr = blink ;
		CHECK_INCDEC_H_GENVAR_0( tProf->channel[0].source, 3 ) ;
	}

#ifdef PCBX9D
  lcd_putsAttIdx(16*FW, 0, XPSTR("\005Jack Sbus Cppm Slave"), tProf->channel[0].source, attr ) ;
	if ( tProf->channel[0].source == TRAINER_SLAVE )
	{ // i am the slave
	    lcd_puts_P(7*FW, 3*FH, PSTR(STR_SLAVE));
	    return;
	}
#else
  lcd_putsAttIdx(16*FW, 0, XPSTR("\005Jack BT   COM2 Slave"), tProf->channel[0].source, attr ) ;
#endif
//#endif
//				stop_serial_trainer_capture() ;

	y = FH;
//	attr = ( (sub == 1) && ( subSub == 0 ) ) ? blink : 0 ;
	attr = (sub == 1) ? blink : 0 ;
	alphaEditName( 1, y, tProf->profileName,
								  6, attr|ALPHA_NO_NAME, (uint8_t *)XPSTR( "Profile Name") ) ;

//	attr = ( (sub == 1) && ( subSub == 1 ) ) ? blink : 0 ;
//	lcd_puts_P(8*FW, y, PSTR(STR_MULTIPLIER));
//	lcd_outdezAtt(21*FW, y, g_eeGeneral.PPM_Multiplier+10, attr|PREC1);
//	if(attr) CHECK_INCDEC_H_GENVAR( g_eeGeneral.PPM_Multiplier, -10, 40);
	y += FH;

	lcd_puts_P(3*FW, 2*FH, XPSTR("mode   % src  sw"));
	sub -= 2 ;
	y = 3*FH;

	for (uint8_t i=0; i<4; i++)
	{
		TrainerChannel *tChan = &tProf->channel[i] ;
//	  volatile TrainerMix *td = &g_eeGeneral.trainer.mix[i];
//	  volatile exTrainerMix *xtd = &g_eeGeneral.exTrainer[i] ;
	  putsChnRaw(0, y, i+1, 0);

	  edit = (sub==i && subSub==0);
    lcd_putsAttIdx(4*FW, y, PSTR(STR_OFF_PLUS_EQ),tChan->mode, edit ? blink : 0);
	  if (edit && s_editMode)
	    CHECK_INCDEC_H_GENVAR_0( tChan->mode, 2); //!! bitfield

	  edit = (sub==i && subSub==1);
	  lcd_outdezAtt(11*FW, y, tChan->studWeight, edit ? blink : 0);
	  if (edit && s_editMode)
	    CHECK_INCDEC_H_GENVAR( tChan->studWeight, -125, 125) ; //!! bitfield

	  edit = (sub==i && subSub==2);
    lcd_putsAttIdx(12*FW, y, PSTR(STR_CH1_4),tChan->srcChn, edit ? blink : 0);
	  if (edit && s_editMode)
	    CHECK_INCDEC_H_GENVAR_0( tChan->srcChn, 3); //!! bitfield

	  edit = (sub==i && subSub==3);
	  putsDrSwitches(15*FW, y, tChan->swtch, edit ? blink : 0);
	  if (edit && s_editMode)
	    CHECK_INCDEC_GENERALSWITCH(event, tChan->swtch, -MaxSwitchIndex, MaxSwitchIndex) ;

	  y += FH;
	}

	if ( lines == NUM_TRAINER_LINES-1 )
	{
		edit = (sub==4);
		lcd_putsAtt(0*FW, y, PSTR(STR_CAL), edit ? blink : 0);
		for (uint8_t i=0; i<4; i++)
		{
			TrainerChannel *tChan = &tProf->channel[i] ;
  	  uint8_t x = (i*9+15)*FW/2;
  	  lcd_outdezAtt(x , y, (g_ppmIns[i]-tChan->calib)*2, PREC1);
		}
		if (edit)
		{
		  if (event==EVT_KEY_FIRST(KEY_MENU))
			{
  	    s_editMode = false ;
	//	  	memcpy(g_eeGeneral.trainer.calib, g_ppmIns, sizeof(g_eeGeneral.trainer.calib));
				for (uint8_t i=0; i<4; i++)
				{
					TrainerChannel *tChan = &tProf->channel[i] ;
					tChan->calib = g_ppmIns[i] ;
		  	}
				STORE_GENERALVARS;     //eeWriteGeneral();
		  	//        eeDirty(EE_GENERAL);
		  	audioDefevent(AU_MENUS);
		  }
		}
	}
}


uint16_t s_timeCumTot;		// Total tx on time (secs)
static uint16_t s_time;
static uint8_t s_cnt;

// Timer triggers:
// OFF - disabled
// ABS AND no switch - always running
// THs AND no switch - throttle stick
// TH% AND no switch - throttle %
// cx% AND no switch - channel %
// ABS AND switch/switchm - controlled by switch
// THs AND switch/switchm - both throttle stick AND switch
// TH% AND switch/switchm - both throttle % AND switch
// cx% AND switch/switchm - both channel % AND switch

// 1. Test modeA, if OFF, timer OFF
// 2. Test modeB, if no switch, or switch is ON, timer may run, else OFF
// 3. Test modeA, if ABS timer is ON
// 4. Test modeA, if THs, timer ON if throttle not 0
// 4. Test modeA, if Th% or cx%, timer on as %

//int16_t  s_timerVal;
void timer(int16_t throttle_val)
{
	
	int16_t val ;
	uint8_t timer ;
	int8_t tma ;
  int16_t tmb ;
  uint16_t tv ;
  
  s_cnt++;			// Number of times val added in
	for( timer = 0 ; timer < 2 ; timer += 1 )
	{
		struct t_timer *ptimer = &s_timer[timer] ;
		uint8_t resetting = 0 ;
		if ( timer == 0 )
		{
			tmb = g_model.timer1RstSw ;
		}
		else
		{
			tmb = g_model.timer2RstSw ;
		}
		if ( tmb )
		{
			if ( tmb < -HSW_MAX )
			{
				tmb += 256 ;
			}

    	if(tmb>(HSW_MAX))	 // toggeled switch
			{
    	  uint8_t swPos = getSwitch00( tmb-(HSW_MAX) ) ;
				if ( swPos != ptimer->lastResetSwPos )
				{
					ptimer->lastResetSwPos = swPos ;
					if ( swPos )	// Now on
					{
						resetting = 1 ;
					}
				}
			}
			else
			{
				if ( getSwitch00( tmb) )
				{
					resetting = 1 ;
				}
			}
		}
		if ( resetting )
		{
			if ( timer == 0 )
			{
				resetTimer1() ;
			}
			else
			{
				resetTimer2() ;
			}
		}
		
		tma = g_model.timer[timer].tmrModeA ;
    tmb = g_model.timer[timer].tmrModeB ;
		if ( tmb < -HSW_MAX )
		{
			tmb += 256 ;
		}

// code for cx%
		val = throttle_val ;
   	if(tma>=TMR_VAROFS) // Cxx%
		{
 	    val = g_chans512[tma-TMR_VAROFS] ;
		}		

		val = ( val + RESX ) / (RESX/16) ;

		if ( tma != TMRMODE_NONE )		// Timer is not off
		{ // We have a triggerA so timer is running 
    	if(tmb>(HSW_MAX))	 // toggeled switch
			{
    	  if(!(ptimer->sw_toggled | ptimer->s_sum | s_cnt | s_time | ptimer->lastSwPos)) ptimer->lastSwPos = 0 ;  // if initializing then init the lastSwPos
    	  uint8_t swPos = getSwitch00( tmb-(HSW_MAX) ) ;
    	  if(swPos && !ptimer->lastSwPos)  ptimer->sw_toggled = !ptimer->sw_toggled;  //if switch is flipped first time -> change counter state
    	  ptimer->lastSwPos = swPos;
    	}
    	else
			{
				if ( tmb )
				{
    	  	ptimer->sw_toggled = getSwitch00( tmb ); //normal switch
				}
				else
				{
					ptimer->sw_toggled = 1 ;	// No trigger B so use as active
				}
			}
		}

		if ( ( ptimer->sw_toggled == 0 ) || resetting )
		{
			val = 0 ;
		}

    ptimer->s_sum += val ;   // Add val in
    if( ( (uint16_t)( get_tmr10ms()-s_time) ) < 100 )		// BEWARE of 32 bit processor extending 16 bit values
		{
			if ( timer == 0 )
			{
				continue ; //1 sec
			}
			else
			{
				return ;
			}
		}
    val     = ptimer->s_sum/s_cnt;   // Average of val over last 100mS
    ptimer->s_sum  -= val*s_cnt;     //rest (remainder not added in)

	  ptimer->s_timeCumAbs += 1;
		if ( timer == 0 )
		{
    	s_timeCumTot += 1;
			g_eeGeneral.totalElapsedTime += 1 ;
		}
		else
		{
	    s_cnt   = 0;    // ready for next 100mS
			s_time += 100;  // 100*10mS passed
		}
    if(val) ptimer->s_timeCumThr       += 1;
		if ( !resetting )
		{
    	if(ptimer->sw_toggled) ptimer->s_timeCumSw += 1;
		}
    ptimer->s_timeCum16ThrP            += val>>1;	// val/2

    tv = ptimer->s_timerVal = g_model.timer[timer].tmrVal ;
    if(tma == TMRMODE_NONE)
		{
			ptimer->s_timerState = TMR_OFF;
		}
    else
		{
			if ( tma==TMRMODE_ABS )
			{
				if ( tmb == 0 ) ptimer->s_timerVal -= ptimer->s_timeCumAbs ;
	    	else ptimer->s_timerVal -= ptimer->s_timeCumSw ; //switch
			}
	    else if(tma<TMR_VAROFS-1) ptimer->s_timerVal -= ptimer->s_timeCumThr;	// stick
		  else ptimer->s_timerVal -= ptimer->s_timeCum16ThrP/16 ; // stick% or Cx%
		}   
		 
    switch(ptimer->s_timerState)
    {
    case TMR_OFF:
        if(tma != TMRMODE_NONE) ptimer->s_timerState=TMR_RUNNING;
        break;
    case TMR_RUNNING:
        if(ptimer->s_timerVal<0 && tv) ptimer->s_timerState=TMR_BEEPING;
        break;
    case TMR_BEEPING:
        if(ptimer->s_timerVal <= -MAX_ALERT_TIME)   ptimer->s_timerState=TMR_STOPPED;
        if(tv == 0)       ptimer->s_timerState=TMR_RUNNING;
        break;
    case TMR_STOPPED:
        break;
    }

//		if ( timer == 0 )
//		{

  	  if(ptimer->last_tmr != ptimer->s_timerVal)  //beep only if seconds advance
    	{
    		ptimer->last_tmr = ptimer->s_timerVal;
        if(ptimer->s_timerState==TMR_RUNNING)
        {
					uint8_t audioControl ;
					uint8_t hapticControl = 0 ;
					if ( timer == 0 )
					{
						audioControl = g_eeGeneral.preBeep | g_model.timer1Cdown ;
						if ( audioControl )
						{
							if ( g_model.timer1Haptic & 2 )
							{
								hapticControl = 1 ;
							}
						}
					}
					else
					{
						audioControl = g_model.timer2Cdown ;
						if ( audioControl )
						{
							if ( g_model.timer2Haptic & 2 )
							{
								hapticControl = 1 ;
							}
						}
					}
            if(audioControl && g_model.timer[timer].tmrVal) // beep when 30, 15, 10, 5,4,3,2,1 seconds remaining
            {
              	if(ptimer->s_timerVal==30) {audioNamedVoiceDefevent( AU_TIMER_30, SV_30SECOND );if ( hapticControl ){audioDefevent(AU_HAPTIC1);}}
              	if(ptimer->s_timerVal==20) {audioNamedVoiceDefevent( AU_TIMER_20, SV_20SECOND );if ( hapticControl ){audioDefevent(AU_HAPTIC1);}}
                if(ptimer->s_timerVal==10) {audioNamedVoiceDefevent( AU_TIMER_10, SV_10SECOND );if ( hapticControl ){audioDefevent(AU_HAPTIC1);}}
                if(ptimer->s_timerVal<= 5)
								{
									if(ptimer->s_timerVal>= 0)
									{
										audioVoiceDefevent(AU_TIMER_LT3, ptimer->s_timerVal | VLOC_NUMSYS ) ;
										if ( hapticControl ){audioDefevent(AU_HAPTIC1);}
									}
//									else
//									{
//										if ( ( timer == 0 ) && (g_eeGeneral.flashBeep) && ( g_eeGeneral.preBeep || g_model.timer1Mbeep ) )
//										{
//											audioDefevent(AU_TIMER_LT3);
//										}
//									}
								}
								if(g_eeGeneral.flashBeep && (ptimer->s_timerVal==30 || ptimer->s_timerVal==20 || ptimer->s_timerVal==10 || ptimer->s_timerVal<=3))
                    g_LightOffCounter = FLASH_DURATION;
            }
						div_t mins ;
						mins = div( g_model.timer[timer].tmrDir ? g_model.timer[timer].tmrVal- ptimer->s_timerVal : ptimer->s_timerVal, 60 ) ;
					hapticControl = 0 ;
					if ( timer == 0 )
					{
						audioControl = g_eeGeneral.minuteBeep | g_model.timer1Mbeep ;
						if ( audioControl )
						{
							if ( g_model.timer1Haptic & 1 )
							{
								hapticControl = 1 ;
							}
						}
					}
					else
					{
						audioControl = g_model.timer2Mbeep ;
						if ( audioControl )
						{
							if ( g_model.timer1Haptic & 1 )
							{
								hapticControl = 1 ;
							}
						}
					}
            if( audioControl && ((mins.rem)==0)) //short beep every minute
            {
							if ( mins.quot )
							{
								voice_numeric( mins.quot, 0, (mins.quot == 1) ? SV_MINUTE : SV_MINUTES ) ;
								if ( hapticControl )
								{
									audioDefevent(AU_HAPTIC1);
								}
							}
              if(g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
            }
        }
        else if(ptimer->s_timerState==TMR_BEEPING)
        {
					if ( ( timer == 0 ) && g_eeGeneral.preBeep )
					{
            audioDefevent(AU_TIMER_LT3);
            if(g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
					}
        }
    	}
//		}
    if( g_model.timer[timer].tmrDir) ptimer->s_timerVal = tv-ptimer->s_timerVal; //if counting backwards - display backwards
//    if( tv==0) s_timer[timer].s_timerVal = tv-s_timer[timer].s_timerVal; //if counting backwards - display backwards
	}
}


#define MAXTRACE 120
uint8_t s_traceBuf[MAXTRACE];
uint8_t s_traceWr;
uint16_t s_traceCnt;
void trace()   // called in perOut - once every 0.01sec
{
    //value for time described in g_model.tmrMode
    //OFFABSRUsRU%ELsEL%THsTH%ALsAL%P1P1%P2P2%P3P3%
    //now OFFABSTHsTH%
  int16_t val ;
//#ifdef FIX_MODE
  val = calibratedStick[3-1];
//#else
//  val = calibratedStick[CONVERT_MODE(3)-1]; //Get throttle channel value
//#endif
//    uint16_t v = 0;
//    if((g_model.timer[0].tmrModeA>1) && (g_model.timer[0].tmrModeA<TMR_VAROFS))
//		{
//      v = ( val + RESX ) / (RESX/16) ;
//    }

// ***** Handle sticky throttle here
		if ( ThrottleStickyOn )
		{
			val = -RESX ;
		}
    timer(val);

//    uint16_t val = calibratedStick[CONVERT_MODE(3)-1]; //Get throttle channel value
  val = (val+RESX) / (RESX/16); //calibrate it
  
	static uint16_t s_time;
  static uint16_t s_cnt;
  static uint16_t s_sum;
//    static uint8_t test = 1;
  s_cnt++;
  s_sum+=val;
  if((uint16_t)( get_tmr10ms()-s_time)<1000) //10 sec
      //		uint16_t time10ms ;
      //		time10ms = get_tmr10ms() ;
      //    if(( time10ms-s_time)<1000) //10 sec
      return;
  s_time= get_tmr10ms() ;
 
#ifdef PCBSKY
  if ((g_model.protocol==PROTO_DSM2)&&getSwitch00(MAX_SKYDRSWITCH-1) ) audioDefevent(AU_TADA);   //DSM2 bind mode warning
#endif
#if defined(PCBX9D) || defined(PCB9XT)
  if ((g_model.xprotocol==PROTO_DSM2)&&getSwitch00(MAX_SKYDRSWITCH-1) ) audioDefevent(AU_TADA);   //DSM2 bind mode warning
#endif
  //    s_time= time10ms ;
  val   = s_sum/s_cnt;
  s_sum = 0;
  s_cnt = 0;

  s_traceCnt++;
  s_traceBuf[s_traceWr++] = val;
  if(s_traceWr>=MAXTRACE) s_traceWr=0;
}

TCHAR PlaylistDirectory[30] ;
char PlayListNames[PLAYLIST_COUNT][MUSIC_NAME_LENGTH+2] ;
uint16_t PlayListCount ;
extern uint16_t PlaylistIndex ;
extern char CurrentPlayName[] ;

void menuProcMusic(uint8_t event)
{
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
//					if ( PlaylistIndex == 0 )
//					{
//						PlaylistIndex = PlayListCount - 1 ;
//					}
//					else
//					{
//						PlaylistIndex -= 1 ;
//					}
//					MusicPlaying = MUSIC_STARTING ;
				}
			  if ( event == EVT_KEY_FIRST(KEY_RIGHT) )
				{
					killEvents(event) ;
					MusicPrevNext = MUSIC_NP_NEXT ;
//					PlaylistIndex += 1 ;
//					if ( PlaylistIndex >= PlayListCount )
//					{
//						PlaylistIndex = 0 ;
//					}
//					MusicPlaying = MUSIC_STARTING ;
				}
			}
		}
	}

  lcd_puts_Pleft( 1*FH, XPSTR("Type")) ;
	
	uint32_t b = g_eeGeneral.musicType ;
	g_eeGeneral.musicType = checkIndexed( 1*FH, XPSTR(FWx15"\001""\004NameList"), b, (mstate2.m_posVert == 1) ) ;
	if ( g_eeGeneral.musicType != b )
	{
		g_eeGeneral.musicVoiceFileName[0] = '\0' ;
	}

  lcd_puts_Pleft( 2*FH, XPSTR("File")) ;
	lcd_putsAtt( 6*FW, 2*FH, (char *)g_eeGeneral.musicVoiceFileName, 0 ) ;
  
	g_eeGeneral.musicLoop = onoffMenuItem( g_eeGeneral.musicLoop, 3*FH, XPSTR("Loop"), mstate2.m_posVert == 3 ) ;
	
	if ( rows == 5 )
	{
  	lcd_puts_Pleft( 4*FH, XPSTR("Start")) ;
		if ( mstate2.m_posVert == 5 )
		{
			mstate2.m_posVert = 4 ;
		}
	}
	else
	{
		lcd_puts_Pleft( 4*FH, XPSTR("Stop")) ;
		lcd_puts_Pleft( 5*FH, MusicPlaying == MUSIC_PAUSED ? XPSTR("\007Resume") : XPSTR("\007Pause")) ;
		if ( g_eeGeneral.musicType )
		{
			if ( MusicPlaying == MUSIC_PLAYING )
			{
				lcd_puts_Pleft( 5*FH, XPSTR("Prev<\017>Next")) ;
			}
		}
		lcd_puts_Pleft( 6*FH, CurrentPlayName ) ;
	}

	if ( mstate2.m_posVert == 2 )
	{
		lcd_rect( 6*FW-1, 2*FH-1, MUSIC_NAME_LENGTH*FW+2, 9 ) ;
	}

	if ( mstate2.m_posVert == 4 )
	{
		lcd_char_inverse( 0, 4*FH, 30, 0 ) ;
	}
	else if ( mstate2.m_posVert == 5 )
	{
		lcd_char_inverse( 7*FW, 5*FH, 36, 0 ) ;
	}

extern uint32_t BgSizePlayed ;
extern uint32_t BgTotalSize ;

		lcd_hbar( 10, 57, 101, 6, (BgTotalSize - BgSizePlayed) * 100 / BgTotalSize ) ;
	
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
		lcd_puts_P( 0, (i+1)*FH, PlayListNames[i+sub] ) ;
	}

//extern TCHAR VoiceFilename[] ;
//	lcd_puts_P( 0, 6*FH, VoiceFilename ) ;
//extern uint16_t PlaylistIndex ;
//	lcd_outhex4( 90, 4*FH, sub ) ;
////	lcd_outhex4( 90, 5*FH, k ) ;
//	lcd_outhex4( 90, 6*FH, PlayListCount ) ;
//	lcd_outhex4( 90, 7*FH, PlaylistIndex ) ;

}



uint16_t g_timeMain;
uint16_t g_timeRfsh ;
uint16_t g_timeMixer ;
uint16_t g_timePXX;
uint16_t g_timeBgRead ;

void menuProcStatistic2(uint8_t event)
{
//  TITLE(PSTR(STR_STAT2));
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

//    case EVT_KEY_FIRST(KEY_DOWN):
//        chainMenu(menuProcStatistic);
//        break;
//    case EVT_KEY_FIRST(KEY_UP):
//    case EVT_KEY_FIRST(KEY_EXIT):
//      chainMenu(menuProc0);
//    	killEvents(event) ;
  //  break;
  }

  lcd_puts_Pleft( 1*FH, XPSTR("On Time")) ;
  lcd_putcAtt( 11*FW+3, 1*FH, ':', 0 ) ;
	div_t qr ;
	qr = div( g_eeGeneral.totalElapsedTime, 60 ) ;
  putsTime( 9*FW, FH*1, qr.quot, 0, 0 ) ;
  lcd_outdezNAtt( 14*FW, 1*FH, qr.rem, LEADING0, 2 ) ;

  lcd_puts_Pleft( 2*FH, XPSTR("tmain          ms"));
  lcd_outdezAtt(14*FW , 2*FH, (g_timeMain)/20 ,PREC2);
extern uint32_t MixerRate ;
#if defined(PCBSKY) || defined(PCB9XT)
	lcd_puts_Pleft( 4*FH, XPSTR("Mixer Rate"));
#endif
  lcd_outdezAtt(20*FW , 4*FH, MixerRate, 0 ) ;
#ifdef PCBX9D
  lcd_puts_Pleft( 3*FH, XPSTR("trefresh       ms"));
  lcd_outdezAtt(14*FW , 3*FH, (g_timeRfsh)/20 ,PREC2);
  lcd_puts_Pleft( 4*FH, XPSTR("tmixer         ms"));
  lcd_outdezAtt(14*FW , 4*FH, (g_timeMixer)/20 ,PREC2);


//	lcd_puts_Pleft( 5*FH, PSTR("Usart Errors"));
//extern uint16_t USART_ERRORS ;
//extern uint16_t USART_ORE ;
//extern uint16_t USART_NE ;
//extern uint16_t USART_FE ;
//extern uint16_t USART_PE ;
//  lcd_outdezAtt(14*FW , 5*FH, USART_ERRORS, 0 ) ;
//  lcd_outdezAtt( 5*FW , 6*FH, USART_ORE, 0 ) ;
//  lcd_outdezAtt( 10*FW , 6*FH, USART_NE, 0 ) ;
//  lcd_outdezAtt( 15*FW , 6*FH, USART_FE, 0 ) ;
//  lcd_outdezAtt( 20*FW , 6*FH, USART_PE, 0 ) ;
  
	lcd_puts_Pleft( 1*FH, XPSTR("ttimer1        us"));
  lcd_outdezAtt(14*FW , 1*FH, (g_timePXX)/2 ,0);
#endif

#if defined(PCBSKY) || defined(PCB9XT)
  lcd_puts_Pleft( 3*FH, XPSTR("tBgRead        ms"));
  lcd_outdezAtt(14*FW , 3*FH, (g_timeBgRead)/20 ,PREC2);
#endif
  
extern uint8_t AudioVoiceCountUnderruns ;
	lcd_puts_Pleft( 5*FH, XPSTR("Voice underruns"));
  lcd_outdezAtt( 20*FW, 5*FH, AudioVoiceCountUnderruns, 0 ) ;

#ifdef PCBSKY
// Debug code
  lcd_puts_Pleft( 7*FH, XPSTR("BT Reply(debug)"));
#ifndef SIMU
extern uint32_t Bt_ok ;
  lcd_outdezAtt( 17*FW , 7*FH, Bt_ok ,0 ) ;

//extern int16_t BtChannels[8] ;

//  lcd_outdezAtt(5*FW , 4*FH, BtChannels[0], 0 ) ;
//  lcd_outdezAtt(10*FW , 4*FH, BtChannels[1], 0 ) ;
//  lcd_outdezAtt(15*FW , 4*FH, BtChannels[2], 0 ) ;
//  lcd_outdezAtt(20*FW , 4*FH, BtChannels[3], 0 ) ;
//  lcd_outdezAtt(5*FW , 5*FH, BtChannels[4], 0 ) ;
//  lcd_outdezAtt(10*FW , 5*FH, BtChannels[5], 0 ) ;
//  lcd_outdezAtt(15*FW , 5*FH, BtChannels[6], 0 ) ;
//  lcd_outdezAtt(20*FW , 5*FH, BtChannels[7], 0 ) ;

//extern uint8_t BtReceived[16] ;
//  lcd_outhex4( 0, 4*FH, (BtReceived[0]<< 8) | BtReceived[1] ) ;
//  lcd_outhex4( 24, 4*FH, (BtReceived[2]<< 8) | BtReceived[3] ) ;
//  lcd_outhex4( 48, 4*FH, (BtReceived[4]<< 8) | BtReceived[5] ) ;
//  lcd_outhex4( 72, 4*FH, (BtReceived[6]<< 8) | BtReceived[7] ) ;
//  lcd_outhex4( 0, 5*FH, (BtReceived[8]<< 8) | BtReceived[9] ) ;
//  lcd_outhex4( 24, 5*FH, (BtReceived[10]<< 8) | BtReceived[11] ) ;
//  lcd_outhex4( 48, 5*FH, (BtReceived[12]<< 8) | BtReceived[13] ) ;
//  lcd_outhex4( 72, 5*FH, (BtReceived[14]<< 8) | BtReceived[15] ) ;

#endif
#endif

  lcd_puts_P( 3*FW,  6*FH, PSTR(STR_MENU_REFRESH));

#ifdef PCBSKY
//extern uint8_t DsmDebug[18] ;
//  lcd_outhex4( 0, 3*FH,  (DsmDebug[17] << 8) | DsmDebug[0] ) ;
//  lcd_outhex4( 24, 3*FH, (DsmDebug[1]<< 8) | DsmDebug[2] ) ;
//  lcd_outhex4( 48, 3*FH, (DsmDebug[3]<< 8) | DsmDebug[4] ) ;
//  lcd_outhex4( 72, 3*FH, (DsmDebug[5]<< 8) | DsmDebug[6] ) ;
//  lcd_outhex4( 96, 3*FH, (DsmDebug[7]<< 8) | DsmDebug[8] ) ;
//  lcd_outhex4( 0, 4*FH, (DsmDebug[9]<< 8) | DsmDebug[10] ) ;
//  lcd_outhex4( 24, 4*FH, (DsmDebug[11]<< 8) | DsmDebug[12] ) ;
//  lcd_outhex4( 48, 4*FH, (DsmDebug[13]<< 8) | DsmDebug[14] ) ;
//  lcd_outhex4( 72, 4*FH, (DsmDebug[15]<< 8) | DsmDebug[16] ) ;
//	lcd_outhex4( 96, 4*FH, (g_model.ppmNCH<< 8) | (g_model.dsmMode& 0x00FF) ) ;

//extern uint8_t DsmPass1[10] ;
//  lcd_outhex4( 0, 5*FH,  (DsmPass1[0]<< 8) | DsmPass1[1] ) ;
//  lcd_outhex4( 24, 5*FH, (DsmPass1[2]<< 8) | DsmPass1[3] ) ;
//  lcd_outhex4( 48, 5*FH, (DsmPass1[4]<< 8) | DsmPass1[5] ) ;
//  lcd_outhex4( 72, 5*FH, (DsmPass1[6]<< 8) | DsmPass1[7] ) ;
//  lcd_outhex4( 96, 5*FH, (DsmPass1[8]<< 8) | DsmPass1[9] ) ;
#endif

	 
//	lcd_outhex4( 0, 4*FH, (frskyStreaming<< 8) | frskyUsrStreaming ) ;
}


//#ifdef PCB9XT
//#include "X9D/hal.h"
//uint8_t AudioTest ;
//uint8_t AudioCount ;
//extern void sdIfSetup( void ) ;

//void menuAudioDebug(uint8_t event)
//{
//	TITLE(XPSTR("Audio Debug")) ;

//	if ( event == EVT_KEY_FIRST(KEY_EXIT) )
//	{
//    killEvents(event) ;
//		popMenu(false) ;
//	}

//	AudioCount += 1 ;
//	if ( AudioCount == 2 )
//	{
//		AudioCount = 0 ;
//	}
//	switch ( AudioTest )
//	{
//		case 0 :
//			if ( AudioCount )
//			{
//				GPIOB->BSRRL = GPIO_Pin_SPI_SD_SCK ;			// A0 high
//			}
//			else
//			{
//				GPIOB->BSRRH = GPIO_Pin_SPI_SD_SCK ;			// A0 high
//			}
//		break ;
//		case 1 :
//			if ( AudioCount )
//			{
//				GPIOB->BSRRL = GPIO_Pin_SPI_SD_MOSI ;			// A0 high
//			}
//			else
//			{
//				GPIOB->BSRRH = GPIO_Pin_SPI_SD_MOSI ;			// A0 high
//			}
//		break ;
//		case 2 :
//			if ( AudioCount )
//			{
//				GPIOB->BSRRL = GPIO_Pin_SPI_SD_CS ;			// A0 high
//			}
//			else
//			{
//				GPIOB->BSRRH = GPIO_Pin_SPI_SD_CS ;			// A0 high
//			}
//		break ;
//	}
//}

//uint16_t PXX_sync ;

extern uint8_t RawLogging ;

void menuDebug(uint8_t event)
{
	MENU(XPSTR("DEBUG"), menuTabStat, e_debug, 1, {0} ) ;

//	int8_t sub = mstate2.m_posVert ;

//  lcd_outhex4( 0, 2*FH, PXX_sync ) ;
//  if(sub==1)
//  {
//		uint16_t x = PXX_sync >> 8 ;
//		lcd_char_inverse( 0, 2*FH, 20, 0 ) ;
//		x = checkIncDec16( x, 0, 100, 0 ) ;
//		PXX_sync = x << 8 ;
//  }

  switch(event)
	{
		case EVT_KEY_LONG(KEY_MENU):
			RawLogging = RawLogging ? 0 : 1 ;
		break ;
	}
//	{
//		case EVT_ENTRY :
//#ifdef PCBX9D
//			configure_pins( GPIO_Pin_7, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
//#endif			
////			IWDG->KR = 0x5555 ;		// Unlock registers
////			IWDG->RLR = 2000 ;			// 2.0 seconds nominal
//		break ;
    
////		case EVT_KEY_LONG(KEY_MENU):
////			__disable_irq() ;
////			for(;;)
////				;
////		break ;
//	}


//  switch(event)
//	{
//		case EVT_ENTRY :
//			AudioTest = 0 ;
//		break ;
//		case EVT_ENTRY_UP :
//			sdIfSetup() ;
//		break ;
//    case EVT_KEY_FIRST(KEY_MENU):
//			configure_pins( GPIO_Pin_SPI_SD_SCK | GPIO_Pin_SPI_SD_MOSI|GPIO_Pin_SPI_SD_CS, PIN_OUTPUT | PIN_PORTB | PIN_PUSHPULL | PIN_OS25 | PIN_NO_PULLUP ) ;
//			AudioCount = 0 ;
//			pushMenu(menuAudioDebug) ;
//		break ;
//	}

//#ifdef PCB9XT
//  lcd_outhex4( 100, 0*FH, Analog_values[5] ) ;
//  lcd_outhex4( 100, 1*FH, Analog_values[6] ) ;
//  lcd_outhex4( 100, 2*FH, Analog_values[7] ) ;
//#endif	// PCB9XT

#if defined(PCBSKY) || defined(PCB9XT)
  lcd_outhex4( 0,  1*FH, FrskyHubData[FR_VCC     ] ) ;
  lcd_outhex4( 30, 1*FH, FrskyHubData[FR_CURRENT ] ) ;
  lcd_outhex4( 60, 1*FH, FrskyHubData[FR_ALT_BARO] ) ;
  lcd_outhex4( 90, 1*FH, FrskyHubData[FR_BASEMODE] ) ;

  lcd_outhex4( 0,  2*FH, FrskyHubData[FR_WP_DIST ] ) ;
  lcd_outhex4( 30, 2*FH, FrskyHubData[FR_HEALTH  ] ) ;
  lcd_outhex4( 60, 2*FH, FrskyHubData[FR_MSG     ] ) ;
  lcd_outhex4( 90, 2*FH, FrskyHubData[FR_HOME_DIR] ) ;

  lcd_outhex4( 0,  3*FH, FrskyHubData[FR_HOME_DIST] ) ;
  lcd_outhex4( 30, 3*FH, FrskyHubData[FR_CPU_LOAD ] ) ;
  lcd_outhex4( 60, 3*FH, FrskyHubData[FR_GPS_HDOP ] ) ;
  lcd_outhex4( 90, 3*FH, FrskyHubData[FR_WP_NUM   ] ) ;

  lcd_outhex4( 0,  4*FH, FrskyHubData[FR_WP_BEARING] ) ;
  lcd_outhex4( 30, 4*FH, FrskyHubData[FR_VOLTS     ] ) ;
  lcd_outhex4( 60, 4*FH, FrskyHubData[FR_TEMP2     ] ) ;
  lcd_outhex4( 90, 4*FH, FrskyHubData[FR_FUEL      ] ) ;
//  lcd_outhex4( 90, 4*FH, FrskyHubData[FR_RPM       ] ) ;

  lcd_outhex4( 0,  5*FH, FrskyHubData[FR_TEMP1     ] ) ;

#endif	// PCBSKY

#ifdef PCBX9D
extern uint16_t XjtHbeatOffset ;
  lcd_outhex4( 0,  6*FH, XjtHbeatOffset ) ;
  lcd_outhex4( 25,  6*FH, XjtHeartbeatCapture.valid ) ;
#endif

//extern uint16_t RawIndex ;
//extern uint8_t RawActiveBuffer ;
//extern uint8_t RawWriteBuffer ;
//  lcd_outhex4( 30, 5*FH, RawIndex ) ;
//  lcd_outhex4( 60, 5*FH, RawActiveBuffer ) ;
//  lcd_outhex4( 90, 5*FH, RawWriteBuffer ) ;

//#ifdef REVX
//extern uint16_t MavDebug1 ;
//extern uint16_t MavDebug2 ;
//extern uint16_t MavDebug3 ;

//  lcd_outhex4( 30, 5*FH, MavDebug1 ) ;
//  lcd_outhex4( 60, 5*FH, MavDebug2 ) ;
//  lcd_outhex4( 90, 5*FH, MavDebug3 ) ;
//#endif


	lcd_puts_Pleft( 6*FH, XPSTR("Raw Logging") ) ;
	menu_lcd_onoff( PARAM_OFS, 6*FH, RawLogging, 0 ) ;

extern uint16_t TelRxCount ;
	lcd_puts_Pleft( 7*FH, XPSTR("TelRxCount") ) ;
  lcd_outhex4( 12*FW,  7*FH, TelRxCount ) ;
  lcd_outhex4( 17*FW,  7*FH, FrskyTelemetryType | (TelemetryType << 8) ) ;

//#ifdef PCB9XT
//extern uint16_t PpmTestH ;
//extern uint16_t PpmTestL ;
//  lcd_outhex4( 0, 7*FH, PpmTestH ) ;
//  lcd_outhex4( 30, 7*FH, PpmTestL ) ;
//#endif



//  lcd_outhex4( 0, 2*FH, PIOC->PIO_PSR >> 16 ) ;
//  lcd_outhex4( 30, 2*FH, PIOC->PIO_PSR ) ;

//  lcd_outhex4( 60, 2*FH, PIOC->PIO_OSR >> 16 ) ;
//  lcd_outhex4( 90, 2*FH, PIOC->PIO_OSR ) ;

//  lcd_outhex4( 0, 3*FH, PIOC->PIO_ODSR >> 16 ) ;
//  lcd_outhex4( 30, 3*FH, PIOC->PIO_ODSR ) ;

//extern uint16_t XjtVersion ;
//	lcd_outhex4( 0, 3*FH, XjtVersion ) ;
//	lcd_outhex4( 30, 3*FH, frskyStreaming ) ;
//	lcd_outhex4( 60, 3*FH, SportStreamingStarted ) ;
//extern uint16_t FsDebug ;
//	lcd_outhex4( 90, 3*FH, FsDebug ) ;

	

//#ifdef PCBSKY
//extern uint8_t ExternalRtc[] ;
//  lcd_outhex4( 0, 5*FH, (ExternalRtc[0] << 8 ) | ExternalRtc[1] ) ;
//  lcd_outhex4( 30, 5*FH, (ExternalRtc[2] << 8 ) | ExternalRtc[3] ) ;
//  lcd_outhex4( 60, 5*FH, (ExternalRtc[4] << 8 ) | ExternalRtc[5] ) ;
//  lcd_outhex4( 90, 5*FH, (ExternalRtc[6] << 8 ) | ExternalRtc[7] ) ;

//  lcd_outhex4( 0, 6*FH, TWI0->TWI_MMR >> 16 ) ;
//  lcd_outhex4( 30, 6*FH, TWI0->TWI_MMR ) ;
//  lcd_outhex4( 60, 6*FH, TWI0->TWI_CWGR ) ;
//  lcd_outhex4( 90, 6*FH, TWI0->TWI_SR ) ;

//  lcd_outhex4( 0, 7*FH, PIOA->PIO_ABCDSR[0] ) ;
//  lcd_outhex4( 30, 7*FH, PIOA->PIO_ABCDSR[1] ) ;
//  lcd_outhex4( 60, 7*FH, PIOA->PIO_PSR ) ;
//extern uint32_t ErtcDebug ;
//  lcd_outhex4( 90, 7*FH, ErtcDebug ) ;

//#endif	// PCBSKY

//#ifdef REV9E
//  lcd_outhex4( 100, 0*FH, AnalogData[5] ) ;
//  lcd_outhex4( 100, 1*FH, AnalogData[6] ) ;
//  lcd_outhex4( 100, 2*FH, AnalogData[7] ) ;
//  lcd_outhex4( 100, 3*FH, AnalogData[8] ) ;
//  lcd_outhex4( 100, 4*FH, AnalogData[9] ) ;
//  lcd_outhex4( 100, 5*FH, AnalogData[10] ) ;
//  lcd_outhex4( 100, 6*FH, AnalogData[11] ) ;
//  lcd_outhex4( 0, 1*FH, AnalogData[4] ) ;
//#endif	// PCB9XT

//extern uint8_t CopyPacket[] ;
//extern uint16_t PacketErrors ;

//  lcd_outhex4( 0, 5*FH, (CopyPacket[0] << 8 ) | CopyPacket[1] ) ;
//  lcd_outhex4( 30, 5*FH, (CopyPacket[2] << 8 ) | CopyPacket[3] ) ;
//  lcd_outhex4( 60, 5*FH, (CopyPacket[4] << 8 ) | CopyPacket[5] ) ;
//  lcd_outhex4( 90, 5*FH, (CopyPacket[6] << 8 ) | CopyPacket[7] ) ;
//  lcd_outhex4( 0, 6*FH, (CopyPacket[8] << 8 ) | CopyPacket[9] ) ;
//  lcd_outhex4( 60, 6*FH, PacketErrors ) ;

//extern uint16_t Uart4RxCount ;
//extern uint16_t Frsky4RxCount ;
//  lcd_outhex4( 0, 7*FH, SoftCounter ) ;
//  lcd_outhex4( 30, 7*FH, TelemetryDebug ) ;

//extern struct t_fifo128 Console_fifo ;
//	uint16_t *p = (uint16_t *)&Console_fifo ;
//	uint32_t i ;
//	uint16_t x ;
//	for ( i = 0 ; i < 5 ; i += 1 )
//	{
//		x = *p++ ;
//		x = ( x >> 8 ) | ( x << 8 ) ;
//		lcd_outhex4( i*25, 4*FH, x ) ;
//	}
//	for ( i = 0 ; i < 5 ; i += 1 )
//	{
//		x = *p++ ;
//		x = ( x >> 8 ) | ( x << 8 ) ;
//		lcd_outhex4( i*25, 5*FH, x ) ;
//	}
//	for ( i = 0 ; i < 5 ; i += 1 )
//	{
//		x = *p++ ;
//		x = ( x >> 8 ) | ( x << 8 ) ;
//		lcd_outhex4( i*25, 6*FH, x ) ;
//	}


//#endif



//#ifdef SPLASH_DEBUG
//	static uint16_t value = 0 ;
//extern uint16_t stickMoveValue() ;
//extern uint32_t hasStickMoved( uint16_t value ) ;

//  switch(event)
//	{
//		case EVT_ENTRY :
//			value = stickMoveValue() ;
//		break ;
//	}

//  lcd_outhex4( 0, 2*FH, stickMoveValue() ) ;
//  lcd_outhex4( 25, 2*FH, hasStickMoved( value ) ) ;

//  lcd_outhex4( 0, 4*FH, SplashDebug[0] ) ;
//  lcd_outhex4( 25, 4*FH, SplashDebug[1]) ;
//  lcd_outhex4( 60, 4*FH, SplashDebug[2] ) ;
//  lcd_outhex4( 85, 4*FH, SplashDebug[3] ) ;

//  lcd_outhex4( 0, 5*FH,  SplashDebug[4] ) ;
//  lcd_outhex4( 25, 5*FH, SplashDebug[5] ) ;
//  lcd_outhex4( 60, 5*FH, SplashDebug[6] ) ;
//  lcd_outhex4( 85, 5*FH, SplashDebug[7] ) ;

//  lcd_outhex4( 0, 6*FH, SplashDebug[8] ) ;
//#endif

}
//#endif


uint8_t TrainerMode ;
//uint8_t TrainerPos ;
//uint8_t TrainerDebugData[16] ;
//uint16_t TdebugCounter ;
uint8_t TrainerPolarity ;

void setCaptureMode(uint32_t mode) ;
extern struct t_fifo64 CaptureRx_fifo ;

void menuProcTrainDdiag(uint8_t event)
{
	MENU(XPSTR("Train diag"), menuTabStat, e_traindiag, 3, {0} ) ;
	
	int8_t sub = mstate2.m_posVert ;

  lcd_puts_Pleft( 1*FH, XPSTR("Trainer Mode") ) ;
  lcd_puts_Pleft( 2*FH, XPSTR("Trainer In Pol") ) ;
	lcd_putsAttIdx(16*FW, FH, XPSTR("\004NormSer Com1"), TrainerMode, (sub == 1) ? INVERS : 0 ) ;
	lcd_putsAttIdx(16*FW, 2*FH, XPSTR("\003NegPos"), TrainerPolarity, (sub == 2) ? INVERS : 0 ) ;
	if(sub==1)
  {
		uint8_t b = TrainerMode ;
		TrainerMode = checkIncDec16( TrainerMode, 0, 2, 0 ) ;
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


//extern uint8_t Bit_pulses[] ;
//extern uint8_t *DsmDatPointer ;
//extern uint16_t DebugDsmChan0 ;

//	lcd_outhex4( 0, 3*FH, Bit_pulses[0] ) ;
//	lcd_outhex4( 30, 3*FH, Bit_pulses[1] ) ;
//	lcd_outhex4( 60, 3*FH, Bit_pulses[2] ) ;
//	lcd_outhex4( 90, 3*FH, Bit_pulses[3] ) ;

//	lcd_outhex4( 0, 4*FH,  DsmDatPointer[0] ) ;
//	lcd_outhex4( 30, 4*FH, DsmDatPointer[1] ) ;
//	lcd_outhex4( 60, 4*FH, DsmDatPointer[2] ) ;
//	lcd_outhex4( 90, 4*FH, DsmDatPointer[3] ) ;

//	lcd_outhex4( 0, 5*FH,  DebugDsmChan0 ) ;
	 
//#if defined(PCBX9D) || defined(PCB9XT)
//extern uint16_t TIM3Captures ;
//extern uint16_t SbusCounter ;
//extern uint16_t SbusCounter1 ;
//extern uint16_t SoftCounter ;
//	lcd_outhex4( 60, 3*FH, TIM3Captures ) ;
//	lcd_outhex4( 90, 3*FH, SbusCounter ) ;
//	lcd_outhex4( 60, 4*FH, SbusCounter1 ) ;
//	lcd_outhex4( 90, 4*FH, SoftCounter ) ;

//extern uint16_t USART_PE ;
//  lcd_outdezAtt( 20*FW , 7*FH, USART_PE, 0 ) ;
//#endif

//extern uint16_t CapTimes[12] ;
//extern uint16_t CapLevels[12] ;

//	lcd_outhex4( 0, 3*FH, CapTimes[0] ) ;
//	lcd_outhex4( 0, 4*FH, CapTimes[1] ) ;
//	lcd_outhex4( 0, 5*FH, CapTimes[2] ) ;
//	lcd_outhex4( 0, 6*FH, CapTimes[3] ) ;
//	lcd_outhex4( 0, 7*FH, CapTimes[4] ) ;
//	lcd_outhex4( 60, 5*FH, CapTimes[5] ) ;
//	lcd_outhex4( 60, 6*FH, CapTimes[6] ) ;
	
//	lcd_outhex4( 30, 3*FH, CapLevels[0] ) ;
//	lcd_outhex4( 30, 4*FH, CapLevels[1] ) ;
//	lcd_outhex4( 30, 5*FH, CapLevels[2] ) ;
//	lcd_outhex4( 30, 6*FH, CapLevels[3] ) ;
//	lcd_outhex4( 30, 7*FH, CapLevels[4] ) ;
//	lcd_outhex4( 90, 5*FH, CapLevels[5] ) ;
//	lcd_outhex4( 90, 6*FH, CapLevels[6] ) ;

//	uint32_t i ;
//	for ( i = 0 ; i < 16 ; i += 1 )
//	{
//		if ( TrainerDebugData[i] )
//		{
//			lcd_putc( i*FW, 4*FH, TrainerDebugData[i] ) ;
//		}
//		else
//		{
//			lcd_putc( i*FW, 4*FH, ' ' ) ;
//		}
//	}

//	uint16_t rxchar ;
//	while ( ( rxchar = get_fifo64( &CaptureRx_fifo ) ) != 0xFFFF )
//	{
//		TdebugCounter += 1 ;
//		TrainerDebugData[TrainerPos++] = rxchar ;
//		if ( TrainerPos > 15 )
//		{
//			TrainerPos = 0 ;
//		}
//	}


//#ifdef PCBSKY
//extern uint16_t USART_PE ;
//  lcd_outdezAtt( 20*FW , 6*FH, USART_PE, 0 ) ;
//#endif

//#ifdef PCB9XT
//extern uint32_t I2C_debug[] ;
//  lcd_outhex4( 0, 4*FH, I2C_debug[0] >> 16 ) ;
//  lcd_outhex4( 25, 4*FH, I2C_debug[0] ) ;
//  lcd_outhex4( 60, 4*FH, I2C_debug[1] >> 16 ) ;
//  lcd_outhex4( 85, 4*FH, I2C_debug[1] ) ;

//  lcd_outhex4( 0, 5*FH, I2C_debug[2] >> 16 ) ;
//  lcd_outhex4( 25, 5*FH, I2C_debug[2] ) ;
//  lcd_outhex4( 60, 5*FH, I2C_debug[3] >> 16 ) ;
//  lcd_outhex4( 85, 5*FH, I2C_debug[3] ) ;

//  lcd_outhex4( 0, 6*FH, I2C_debug[4] ) ;

//  lcd_outhex4( 0, 7*FH, I2C_debug[6] >> 16 ) ;
//  lcd_outhex4( 25, 7*FH, I2C_debug[6] ) ;
//  lcd_outhex4( 60, 7*FH, I2C_debug[7] >> 16 ) ;
//  lcd_outhex4( 85, 7*FH, I2C_debug[7] ) ;
//#endif

}

static uint8_t Caps = 0 ;

void displayKeys( uint8_t x, uint8_t y, char *text, uint8_t count )
{
	while ( count )
	{
		char c = *text++ ;
		if ( Caps )
		{
			if ( (c >= 'a') && (c <= 'z') )
			{
				c -= 32 ;
			}
		}
		lcd_putc( x+1, y, c ) ;
		lcd_rect( x-2, y-1, 11, 10 ) ;
		count -= 1 ;
		x += 10 ;
	}
}

const char AlphaSource[] = "0123456789qwertyuiopasdfghjkl zxcvbnm_-. " ;
uint8_t AlphaEdited ;

void menuProcAlpha(uint8_t event)
{
	struct t_alpha *Palpha = &Xmem.Alpha ;
	
	lcd_puts_Pleft( 0, Palpha->PalphaHeading ? (char *)Palpha->PalphaHeading : PSTR( STR_NAME ) ) ;
	static MState2 mstate2 ;
	mstate2.check_columns( event, 5 ) ;

	displayKeys( 2, 3*FH-4, (char *)&AlphaSource[0], 10 ) ;
	displayKeys( 5, 4*FH-3, (char *)&AlphaSource[10], 10 ) ;
	displayKeys( 8, 5*FH-2, (char *)&AlphaSource[20], 10 ) ;
	displayKeys( 11, 6*FH-1, (char *)&AlphaSource[30], 10 ) ;
	lcd_puts_Pleft( 7*FH, XPSTR("CAP SPACE DEL INS") ) ;

	int8_t sub = mstate2.m_posVert ;
	if ( event == EVT_ENTRY )
	{
		Palpha->AlphaIndex = 0 ;
		Palpha->lastSub = sub ;
		AlphaEdited = 0 ;
	}
	
	if ( Palpha->lastSub != sub )
	{
		if ( sub == 0 )
		{
			g_posHorz = Palpha->AlphaIndex ;
		}
		Palpha->lastSub = sub ;
	}

	uint8_t index = 0 ;
	switch ( sub )
	{
		case 0 :
			Columns = Palpha->AlphaLength-1 ;
		break ;
		case 1 :
			Columns = 9 ;
		break ;
		case 2 :
			index = 10 ;
			Columns = 9 ;
		break ;
		case 3 :
			index = 20 ;
			Columns = 9 ;
		break ;
		case 4 :
			index = 30 ;
			Columns = 9 ;
		break ;
		case 5 :
			index = 39 ;
			Columns = 3 ;
		break ;
	}
	index += g_posHorz ;

	if ( event==EVT_KEY_BREAK(KEY_MENU) || event == EVT_KEY_BREAK(BTN_RE)  )
	{
		if ( sub == 5 )
		{
			if ( g_posHorz == 0 )
			{
				Caps = !Caps ;
			}
			if ( g_posHorz == 2 )
			{
				if ( Palpha->AlphaIndex )
				{
					Palpha->AlphaIndex -= 1 ;
					uint32_t i ;
					for ( i = Palpha->AlphaIndex ; i < (uint32_t)Palpha->AlphaLength-1 ; i += 1 )
					{
						Palpha->PalphaText[i] = Palpha->PalphaText[i+1] ;
					}
					Palpha->PalphaText[Palpha->AlphaLength-1] = ' ' ;
					AlphaEdited = 1 ;
    			eeDirty( EditType & (EE_GENERAL|EE_MODEL));
				}
			}
			if ( g_posHorz == 3 )
			{
				if ( Palpha->AlphaIndex < Palpha->AlphaLength-1 )
				{
					uint32_t i ;
					for ( i = Palpha->AlphaLength-1 ; i > Palpha->AlphaIndex ; i -= 1 )
					{
						Palpha->PalphaText[i] = Palpha->PalphaText[i-1] ;
					}
				}
				Palpha->PalphaText[Palpha->AlphaIndex] = ' ' ;
				AlphaEdited = 1 ;
    		eeDirty( EditType & (EE_GENERAL|EE_MODEL));
			}
		}
		if (sub>0)
		{
			char chr = AlphaSource[index] ;
			if ( Caps )
			{
				if ( (chr >= 'a') && (chr <= 'z') )
				{
					chr -= 32 ;
				}
			}
			if ( !( ( sub == 5 ) && ( g_posHorz != 1 ) ) )
			{
				Palpha->PalphaText[Palpha->AlphaIndex] = chr ;
				if ( Palpha->AlphaIndex < Palpha->AlphaLength-1 )
				{
					Palpha->AlphaIndex += 1 ;
				}
				AlphaEdited = 1 ;
    		eeDirty( EditType & (EE_GENERAL|EE_MODEL));
			}
		}
	}
	
	if ( event == EVT_KEY_LONG(KEY_MENU) )
	{
		Caps = !Caps ;
    killEvents(event) ;
	}
	
	if ( sub == 0 )
	{
		Palpha->AlphaIndex = g_posHorz ;
	}

  lcd_putsnAtt( 1, FH, (char *)Palpha->PalphaText, Palpha->AlphaLength, 0 ) ;
	if ( ( sub != 0 ) || ( BLINK_ON_PHASE ) )
	{
		lcd_rect( Palpha->AlphaIndex*FW, FH-1, 7, 9 ) ;
	}

	if ( sub>0 )
	{
		if ( g_posHorz > 9 )
		{
			g_posHorz = 9 ;
		}
		uint8_t x = g_posHorz * 10 + 1 ;
		uint8_t w = 9 ;
		if ( sub == 2 )
		{
			x += 3 ;
		}
		else if ( sub == 3 )
		{
			x += 6 ;
		}
		else if ( sub == 4 )
		{
			x += 9 ;
		}
		else if ( sub == 5 )
		{
			if ( g_posHorz > 3 )
			{
				g_posHorz = 3 ;
			}
			if ( g_posHorz == 0 )
			{
				x = 0 ;
				w = 3*FW ;
			}
			else if ( g_posHorz == 1 )
			{
				w = 5*FW ;
				x = 4*FW ;
			}
			else
			{
				x = (g_posHorz-2) * (4* FW) + 10*FW ;
				w = 3*FW ;
			}
		}
		lcd_char_inverse( x, (sub+2)*FH+sub-5, w, 0 ) ;
	}
	s_editMode = 0 ;
}



uint16_t DsmFrameRequired ;

void menuProcDsmDdiag(uint8_t event)
{
	
	MENU(XPSTR("DSM diag"), menuTabStat, e_dsm, 2, {0} ) ;
  
	int8_t sub = mstate2.m_posVert ;

  lcd_outhex4( 0, 2*FH, DsmFrameRequired ) ;
  if(sub==1)
  {
		lcd_char_inverse( 0, 2*FH, 20, 0 ) ;
		DsmFrameRequired = checkIncDec16( DsmFrameRequired, 0, 511, 0 ) ;
  }

//extern uint16_t DsmDbgCounters[] ;
//  lcd_outhex4( 0, 3*FH,  DsmDbgCounters[0] ) ;
//  lcd_outhex4( 24, 3*FH, DsmDbgCounters[1] ) ;
//  lcd_outhex4( 48, 3*FH, DsmDbgCounters[2] ) ;
//  lcd_outhex4( 72, 3*FH, DsmDbgCounters[3] ) ;
//  lcd_outhex4( 96, 3*FH, DsmDbgCounters[4] ) ;
//  lcd_outhex4( 0, 4*FH,  DsmDbgCounters[5] ) ;
//  lcd_outhex4( 24, 4*FH, DsmDbgCounters[6] ) ;
//  lcd_outhex4( 48, 4*FH, DsmDbgCounters[7] ) ;

  lcd_outhex4( 0, 1*FH, FrskyTelemetryType ) ;
  lcd_outhex4( 24, 1*FH, g_model.dsmMode ) ;

#ifdef PCB9XT

extern uint8_t numPktBytes ;
extern uint8_t frskyRxBuffer[];
  lcd_outhex4( 0, 3*FH, numPktBytes ) ;

	lcd_outhex4( 0 , 4*FH, (frskyRxBuffer[0]<< 8) | frskyRxBuffer[1] ) ;
	lcd_outhex4( 24, 4*FH, (frskyRxBuffer[2]<< 8) | frskyRxBuffer[3] ) ;
	lcd_outhex4( 48, 4*FH, (frskyRxBuffer[4]<< 8) | frskyRxBuffer[5] ) ;
	lcd_outhex4( 72, 4*FH, (frskyRxBuffer[6]<< 8) | frskyRxBuffer[7] ) ;

	lcd_outhex4( 0 , 5*FH, (frskyRxBuffer[8]<< 8) | frskyRxBuffer[9] ) ;
	lcd_outhex4( 24, 5*FH, (frskyRxBuffer[10]<< 8) | frskyRxBuffer[11] ) ;
	lcd_outhex4( 48, 5*FH, (frskyRxBuffer[12]<< 8) | frskyRxBuffer[13] ) ;
	lcd_outhex4( 72, 5*FH, (frskyRxBuffer[14]<< 8) | frskyRxBuffer[15] ) ;

  lcd_outhex4( 0, 6*FH, USART2->CR1 ) ;
  lcd_outhex4( 24, 6*FH, USART2->CR2 ) ;
  lcd_outhex4( 48, 6*FH, USART2->BRR ) ;
extern uint16_t RxIntCount ;
	lcd_outhex4( 72, 6*FH, RxIntCount ) ;

extern uint16_t USART_ERRORS ;
extern uint16_t USART_FE ;
extern uint16_t USART_PE ;
  lcd_outhex4( 0, 7*FH, USART_ERRORS ) ;
  lcd_outhex4( 24, 7*FH, USART_FE ) ;
  lcd_outhex4( 48, 7*FH, USART_PE ) ;


#endif

#ifdef PCBX9D
extern uint16_t USART_ERRORS ;
extern uint16_t USART_FE ;
extern uint16_t USART_PE ;
extern uint16_t USART_ORE ;
extern uint16_t USART_NE ;
  lcd_outhex4( 0, 7*FH, USART_ERRORS ) ;
  lcd_outhex4( 24, 7*FH, USART_FE ) ;
  lcd_outhex4( 48, 7*FH, USART_PE ) ;
  lcd_outhex4( 72, 7*FH, USART_ORE ) ;
  lcd_outhex4( 96, 7*FH, USART_NE ) ;
#endif

#ifdef PCBX9D
#ifdef ASSAN
extern uint16_t USART_ERRORS ;
extern uint16_t USART_FE ;
  lcd_outhex4( 72, 1*FH, USART_ERRORS ) ;
  lcd_outhex4( 96, 1*FH, USART_FE ) ;
#endif
#endif

#ifdef PCBSKY
extern uint8_t DsmDebug[20] ;
extern uint8_t DsmControlDebug[20] ;
extern uint16_t DsmControlCounter ;

  lcd_outhex4( 0, 3*FH,  (DsmDebug[17] << 8) | DsmDebug[0] ) ;
  lcd_outhex4( 24, 3*FH, (DsmDebug[1]<< 8) | DsmDebug[2] ) ;
  lcd_outhex4( 48, 3*FH, (DsmDebug[3]<< 8) | DsmDebug[4] ) ;
  lcd_outhex4( 72, 3*FH, (DsmDebug[5]<< 8) | DsmDebug[6] ) ;
  lcd_outhex4( 96, 3*FH, (DsmDebug[7]<< 8) | DsmDebug[8] ) ;
  lcd_outhex4( 0, 4*FH, (DsmDebug[9]<< 8) | DsmDebug[10] ) ;
  lcd_outhex4( 24, 4*FH, (DsmDebug[11]<< 8) | DsmDebug[12] ) ;
  lcd_outhex4( 48, 4*FH, (DsmDebug[13]<< 8) | DsmDebug[14] ) ;
  lcd_outhex4( 72, 4*FH, (DsmDebug[15]<< 8) | DsmDebug[16] ) ;

  lcd_outhex4( 0, 6*FH,  (DsmControlDebug[17] << 8) | DsmControlDebug[0] ) ;
  lcd_outhex4( 24, 6*FH, (DsmControlDebug[1]<< 8) | DsmControlDebug[2] ) ;
  lcd_outhex4( 48, 6*FH, (DsmControlDebug[3]<< 8) | DsmControlDebug[4] ) ;
  lcd_outhex4( 72, 6*FH, (DsmControlDebug[5]<< 8) | DsmControlDebug[6] ) ;
  lcd_outhex4( 96, 6*FH, (DsmControlDebug[7]<< 8) | DsmControlDebug[8] ) ;
  lcd_outhex4( 0, 7*FH, (DsmControlDebug[9]<< 8) | DsmControlDebug[10] ) ;
  lcd_outhex4( 24, 7*FH, (DsmControlDebug[11]<< 8) | DsmControlDebug[12] ) ;
  lcd_outhex4( 48, 7*FH, (DsmControlDebug[13]<< 8) | DsmControlDebug[14] ) ;
  lcd_outhex4( 72, 7*FH, (DsmControlDebug[15]<< 8) | DsmControlDebug[16] ) ;
  
	lcd_outhex4( 96, 7*FH, DsmControlCounter ) ;

//extern uint16_t TelemetryDebug ;
//extern uint16_t TelemetryDebug1 ;
//extern uint16_t TelemetryDebug2 ;
//	lcd_outhex4( 48, FH, TelemetryDebug2 ) ;
//	lcd_outhex4( 72, FH, TelemetryDebug1 ) ;
//	lcd_outhex4( 96, FH, TelemetryDebug ) ;

//extern uint16_t TelemetryDebug3 ;
//extern uint16_t TelemetryDebug4 ;
//	lcd_outhex4( 48, 2*FH, TelemetryDebug3 ) ;
//	lcd_outhex4( 72, 2*FH, TelemetryDebug4 ) ;

//	if ( TelemetryDebug3 == TelemetryDebug4 )
//	{
//		lcd_putc( 90, 2*FH, '!' ) ;
//	}
//	else
//	{
//		lcd_putc( 90, 2*FH, '-' ) ;
//	}

extern uint16_t DsmDbgCounters[20] ;
	lcd_outhex4( 24, 2*FH, DsmDbgCounters[6] ) ;

	lcd_outhex4( 0, 5*FH, DsmDbgCounters[10] ) ;
	lcd_outhex4( 24, 5*FH, DsmDbgCounters[11] ) ;
	lcd_outhex4( 48, 5*FH, DsmDbgCounters[12] ) ;
	lcd_outhex4( 72, 5*FH, DsmDbgCounters[13] ) ;

#endif
}


#ifdef PCBSKY
struct t_i2cTime
{
	uint8_t setCode ;
	uint8_t Time[7] ;
} TimeToSet ;
#endif

//#ifdef PCBX9D
//t_time TimeToSet ;
//#endif


void dispMonth( uint8_t x, uint8_t y, uint32_t month, uint8_t attr)
{
	if ( month > 12 )
	{
		month = 0 ;		
	}
  lcd_putsAttIdx( x, y, PSTR(STR_MONTHS),month, attr ) ;
}

void disp_datetime( uint8_t y )
{
	lcd_puts_Pleft( y, XPSTR("  -   -     :  :"));
	lcd_outdezNAtt( 18*FW-2, y, Time.second, LEADING0, 2 ) ;
	lcd_outdezNAtt( 15*FW-1, y, Time.minute, LEADING0, 2 ) ;
	lcd_outdezNAtt( 12*FW,   y, Time.hour, LEADING0, 2 ) ;
	lcd_outdezNAtt( 2*FW,    y, Time.date, LEADING0, 2 ) ;
	lcd_outdezNAtt( 10*FW,   y, Time.year, LEADING0, 4 ) ;
	dispMonth( 3*FW, y, Time.month, 0 ) ;
}

t_time EntryTime ;

#ifdef REVX
#define DATE_COUNT_ITEMS	8
#else
#if defined(PCBX9D) || defined(PCB9XT)
#define DATE_COUNT_ITEMS	8
#else
#define DATE_COUNT_ITEMS	7
#endif
#endif

void menuProcDate(uint8_t event)
{
	TITLE(PSTR(STR_DateTime));
	static MState2 mstate2;
	static const uint8_t mstate_tab[] = {0} ;
	
//	if (SubMenuFromIndex)
//	{
		event = mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1,DATE_COUNT_ITEMS-1) ;
//	}
//	else
//	{
//		event = mstate2.check(event,e_Date,menuTabDiag,DIM(menuTabDiag),mstate_tab,DIM(mstate_tab)-1,DATE_COUNT_ITEMS-1) ;
//	}

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
#ifdef PCBSKY
			TimeToSet.setCode = 0x74 ;		// Tiny SET TIME CODE command
			TimeToSet.Time[0] = EntryTime.second ;
			TimeToSet.Time[1] = EntryTime.minute ;
			TimeToSet.Time[2] = EntryTime.hour ;
			TimeToSet.Time[3] = EntryTime.date ;
			TimeToSet.Time[4] = EntryTime.month ;
			TimeToSet.Time[5] = (uint8_t) EntryTime.year ;
			TimeToSet.Time[6] = EntryTime.year >> 8 ;
#endif
#ifdef REVX
			writeRTC( (uint8_t *) &TimeToSet.Time[0] ) ;
#else
#ifdef PCBSKY
			if ( g_eeGeneral.ar9xBoard == 0 )
			{
				write_coprocessor( (uint8_t *) &TimeToSet, 8 ) ;
			}
			else
			{
				writeExtRtc( (uint8_t *) &TimeToSet.Time[0] ) ;
			}
#endif
#if defined(PCBX9D) || defined(PCB9XT)
			rtcSetTime( &EntryTime ) ;
#endif
#endif
      killEvents(event);
			s_editMode = 0 ;
    break;
	}		 
#if defined(PCBX9D) || defined(PCB9XT)
		disp_datetime( 1*FH ) ;
#endif

#if defined(REVB) || defined(PCBX9D) || defined(PCB9XT)
		disp_datetime( 1*FH ) ;

//    lcd_outhex4( 17*FW+4, 6*FH, (Coproc_valid << 8 ) + Coproc_read ) ;
#ifndef REVX
#ifdef PCBSKY
extern uint8_t Co_proc_status[] ;
		if ( g_eeGeneral.ar9xBoard == 0 )
		{
			lcd_outdezAtt( 21*FW, 7*FH, (uint8_t)Co_proc_status[8], 0 ) ;		// Co-Proc temperature

			uint8_t temp = ExtraInputs ;
  	  lcd_putc( 18*FW, 6*FH, temp & 0x20 ? '1' : '0' ) ;
    	lcd_putc( 19*FW, 6*FH, temp & 0x10 ? '1' : '0' ) ;
	    lcd_putc( 20*FW, 6*FH, temp & 0x08 ? '1' : '0' ) ;
		}
#endif
#endif
    int8_t  sub    = mstate2.m_posVert;

		for (uint8_t subN=1; subN<8; subN++)
		{
	  	uint8_t attr = ((sub==subN) ? InverseBlink : 0);
			switch ( subN )
			{
				case 1 :
			  	lcd_puts_Pleft( 2*FH, PSTR(STR_SEC) );
					lcd_outdezNAtt( 7*FW, 2*FH, EntryTime.second, LEADING0|attr, 2 ) ;
			  	if(sub==subN)  EntryTime.second = checkIncDec( EntryTime.second, 0, 59, 0 ) ;
				break ;
				case 2 :
			  	lcd_puts_Pleft( 3*FH, PSTR(STR_MIN_SET) );
					lcd_outdezNAtt( 7*FW, 3*FH, EntryTime.minute, LEADING0|attr, 2 ) ;
			  	if(sub==subN)  EntryTime.minute = checkIncDec( EntryTime.minute, 0, 59, 0 ) ;
				break ;
				case 3 :
			  	lcd_puts_Pleft( 4*FH, PSTR(STR_HOUR_MENU_LONG) );
					lcd_outdezNAtt( 7*FW, 4*FH, EntryTime.hour, LEADING0|attr, 2 ) ;
			  	if(sub==subN)  EntryTime.hour = checkIncDec( EntryTime.hour, 0, 23, 0 ) ;
				break ;
				case 4 :
			  	lcd_puts_Pleft( 5*FH, PSTR(STR_DATE) );
					lcd_outdezNAtt( 7*FW, 5*FH, EntryTime.date, LEADING0|attr, 2 ) ;
			  	if(sub==subN)  EntryTime.date = checkIncDec( EntryTime.date, 1, 31, 0 ) ;
				break ;
				case 5 :
			  	lcd_puts_Pleft( 6*FH, PSTR(STR_MONTH) );
					dispMonth( 5*FW+3, 6*FH, EntryTime.month, attr ) ;
			  	if(sub==subN)  EntryTime.month = checkIncDec( EntryTime.month, 1, 12, 0 ) ;
				break ;
				case 6 :
#ifndef REVX
					if ( g_eeGeneral.ar9xBoard == 0 )
					{
						lcd_puts_Pleft( 7*FH, PSTR(STR_YEAR_TEMP) );
					}
					else
					{
						lcd_puts_Pleft( 7*FH, PSTR(STR_YEAR) );
					}
#else			  	
					lcd_puts_Pleft( 7*FH, PSTR(STR_YEAR) );
#endif
					lcd_outdezNAtt( 9*FW-2, 7*FH, EntryTime.year, LEADING0|attr, 4 ) ;
			  	if(sub==subN)  EntryTime.year = checkIncDec16( EntryTime.year, 0, 2999, 0 ) ;
				break ;
#ifdef REVX
				case 7 :
					uint8_t previous = g_eeGeneral.rtcCal ;
			  	lcd_puts_P( 12*FW, 5*FH, PSTR(STR_CAL) );
					lcd_outdezAtt( 20*FW, 5*FH, g_eeGeneral.rtcCal, attr ) ;
			  	if(sub==subN) CHECK_INCDEC_H_GENVAR( g_eeGeneral.rtcCal, -127, 127 ) ;
					if ( g_eeGeneral.rtcCal != previous )
					{
						previous = g_eeGeneral.rtcCal ;
						if ( g_eeGeneral.rtcCal < 0 )
						{
							previous = -g_eeGeneral.rtcCal | 0x80 ;
						}
						setRtcCAL( previous ) ;						
					}
				break ;
#endif
#if defined(PCBX9D) || defined(PCB9XT)
				case 7 :
					int8_t previous = g_eeGeneral.rtcCal ;
			  	lcd_puts_P( 12*FW, 5*FH, PSTR(STR_CAL) );
					lcd_outdezAtt( 20*FW, 5*FH, g_eeGeneral.rtcCal, attr ) ;
			  	if(sub==subN) CHECK_INCDEC_H_GENVAR( g_eeGeneral.rtcCal, -31, 31 ) ;
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
				break ;
#endif
			}
		}
#endif
}

extern DIR Dj ;
extern DIR Djp ;

void setupFileNames( TCHAR *dir, struct fileControl *fc, char *ext )
{
	FRESULT fr ;
	fr = f_chdir( dir ) ;
	if ( fr == FR_OK )
	{
		fc->index = 0 ;
		fr = f_opendir( ( VoiceFileType == VOICE_FILE_TYPE_MUSIC )? &Djp : &Dj, (TCHAR *) "." ) ;
		if ( fr == FR_OK )
		{
			fc->ext[0] = *ext++ ;
			fc->ext[1] = *ext++ ;
			fc->ext[2] = *ext++ ;
			fc->ext[3] = *ext ;
			fc->index = 0 ;
			fc->nameCount = fillNames( 0, fc ) ;
			fc->hpos = 0 ;
			fc->vpos = 0 ;
		}
	}
}


void menuProcRestore(uint8_t event)
{
//	FRESULT fr ;
	struct fileControl *fc = &FileControl ;
	uint32_t i ;

  TITLE( "RESTORE MODEL" ) ;

  switch(event)
	{
    case EVT_ENTRY:
			setupFileNames( (TCHAR *)"/MODELS", fc, (char *)"EEPM" ) ;
			
//			fr = f_chdir( (TCHAR *)"/MODELS" ) ;
//			if ( fr == FR_OK )
//			{
//				fc->index = 0 ;
//				fr = f_opendir( &Dj, (TCHAR *) "." ) ;
//				if ( fr == FR_OK )
//				{
//					fc->ext[0] = 'E' ;	// 'E'
//					fc->ext[1] = 'E' ;	// 'E'
//					fc->ext[2] = 'P' ;	// 'P'
//					fc->ext[3] = 'M' ;	// 'M'
//					fc->index = 0 ;
//					fc->nameCount = fillNames( 0, fc ) ;
//					fc->hpos = 0 ;
//					fc->vpos = 0 ;
//				}
//			}
    break ;
	}
	
	i = fileList( event, &FileControl ) ;
	if ( i == 1 )	// Select
	{
		// Restore the model
    Tevent = 0 ;
    lcd_clear();
    lcd_putsAtt(64-9*FW,0*FH,XPSTR("Restoring"),DBLSIZE);
    refreshDisplay();
		clearKeyEvents() ;

		TCHAR RestoreFilename[60] ;
		cpystr( cpystr( (uint8_t *)RestoreFilename, (uint8_t *)"/MODELS/" ), (uint8_t *)Filenames[fc->vpos] ) ;
		WatchdogTimeout = 200 ;		// 2 seconds
		BackResult = ee32RestoreModel( RestoreIndex, RestoreFilename ) ;
		AlertType = MESS_TYPE ;
		AlertMessage = BackResult ;
    popMenu() ;
	}
	else if ( i == 2 )	// EXIT
	{
    killEvents(event) ;
    popMenu() ;
	}
}

void menuProcSelectVoiceFile(uint8_t event)
{
//	FRESULT fr ;
	struct fileControl *fc = &FileControl ;
	uint32_t i ;

  TITLE( "Select File" ) ;
		
  switch(event)
	{
    case EVT_ENTRY:
		{
			TCHAR * directory ;
			i = 0 ;
			directory = (TCHAR *)"\\voice\\user" ;
			if ( VoiceFileType == VOICE_FILE_TYPE_NAME )
			{
				directory = (TCHAR *)"\\voice\\modelNames" ;
			}
			else if ( VoiceFileType == VOICE_FILE_TYPE_MUSIC )
			{
				directory = (TCHAR *)"\\music" ;
				if ( g_eeGeneral.musicType )
				{
					i = 1 ;
				}
			}
				 
			setupFileNames( directory, fc, i ? (char *)"\0" : (char *)"WAV" ) ;
//			fr = f_chdir( (VoiceFileType == VOICE_FILE_TYPE_NAME) ? (TCHAR *)"\\voice\\modelNames" : (TCHAR *)"\\voice\\user" ) ;
//			if ( fr == FR_OK )
//			{
//				fc->index = 0 ;
//				fr = f_opendir( &Dj, (TCHAR *) "." ) ;
//				if ( fr == FR_OK )
//				{
//					fc->ext[0] = 'W' ;
//					fc->ext[1] = 'A' ;
//					fc->ext[2] = 'V' ;
//					fc->ext[3] = '\0';
//					fc->index = 0 ;
//					fc->nameCount = fillNames( 0, fc ) ;
//					fc->hpos = 0 ;
//					fc->vpos = 0 ;
//				}
//			}
		}
    break ;
	}
	
	i = fileList( event, &FileControl ) ;
	if ( i == 1 )	// Select
	{
		cpystr( (uint8_t *)SelectedVoiceFileName, (uint8_t *)Filenames[fc->vpos] ) ;
		killEvents(event) ;
    popMenu() ;
	}
	else if ( i == 2 )	// EXIT
	{
    killEvents(event) ;
    popMenu() ;
	}
	else if ( i == 3 )	// Tag
	{
		if ( VoiceFileType != VOICE_FILE_TYPE_MUSIC )
		{		
			char name[12] ;
  	  killEvents(event) ;
			copyFileName( name, Filenames[fc->vpos], 8 ) ;
			putNamedVoiceQueue( name, (VoiceFileType == VOICE_FILE_TYPE_NAME) ? VLOC_MNAMES : VLOC_USER ) ;
		}
		else
		{
			cpystr( (uint8_t *)SelectedVoiceFileName, (uint8_t *)Filenames[fc->vpos] ) ;
			killEvents(event) ;
    	popMenu() ;
			i = 1 ;
		}
	}
	FileSelectResult = i ;
}

//void menuProcSelectUvoiceFile(uint8_t event)
//{
//	FRESULT fr ;
//	struct fileControl *fc = &FileControl ;
//	uint32_t i ;

//  TITLE( "SELECT FILE" ) ;

//  switch(event)
//	{
//    case EVT_ENTRY:
//			fr = f_chdir( (TCHAR *)"\\voice\\user" ) ;
//			if ( fr == FR_OK )
//			{
//				fc->index = 0 ;
//				fr = f_opendir( &Dj, (TCHAR *) "." ) ;
//				if ( fr == FR_OK )
//				{
//					fc->ext[0] = 'W' ;
//					fc->ext[1] = 'A' ;
//					fc->ext[2] = 'V' ;
//					fc->ext[3] = '\0';
//					fc->index = 0 ;
//					fc->nameCount = fillNames( 0, fc ) ;
//					fc->hpos = 0 ;
//					fc->vpos = 0 ;
//				}
//			}
//    break ;
//	}
	
//	i = fileList( event, &FileControl ) ;
//	if ( i == 1 )	// Select
//	{
//		cpystr( (uint8_t *)SelectedVoiceFileName, (uint8_t *)Filenames[fc->vpos] ) ;
//		killEvents(event) ;
//    popMenu() ;
//	}
//	else if ( i == 2 )	// EXIT
//	{
//    killEvents(event) ;
//    popMenu() ;
//	}
//	else if ( i == 3 )	// Tag
//	{
//		char name[12] ;
//    killEvents(event) ;
//		copyFileName( name, Filenames[fc->vpos], 8 ) ;
//		putNamedVoiceQueue( name, 0xD000 ) ;
//	}
//	FileSelectResult = i ;
//}

#ifdef PCBX9D
void menuProcSelectImageFile(uint8_t event)
{
	struct fileControl *fc = &FileControl ;
	uint32_t i ;

  TITLE( "SELECT FILE" ) ;

  switch(event)
	{
    case EVT_ENTRY:
			setupFileNames( (TCHAR *)"/IMAGES", fc, (char *)"BMP" ) ;
//			fr = f_chdir( (TCHAR *)"\\IMAGES" ) ;
//			if ( fr == FR_OK )
//			{
//				fc->index = 0 ;
//				fr = f_opendir( &Dj, (TCHAR *) "." ) ;
//				if ( fr == FR_OK )
//				{
//					fc->ext[0] = 'B' ;
//					fc->ext[1] = 'M' ;
//					fc->ext[2] = 'P' ;
//					fc->ext[3] = '\0';
//					fc->index = 0 ;
//					fc->nameCount = fillNames( 0, fc ) ;
//					fc->hpos = 0 ;
//					fc->vpos = 0 ;
//				}
//			}
    break ;
	}
	
	i = fileList( event, &FileControl ) ;
	if ( i == 1 )	// Select
	{
		// Restore the model
		cpystr( (uint8_t *)SelectedVoiceFileName, (uint8_t *)Filenames[fc->vpos] ) ;
		killEvents(event) ;
    popMenu() ;
	}
	else if ( i == 2 )	// EXIT
	{
    killEvents(event) ;
    popMenu() ;
	}
//	else if ( i == 3 )	// Tag
//	{
//		char name[12] ;
//    killEvents(event) ;
//		copyFileName( name, Filenames[fc->vpos] ) ;
//		putNamedVoiceQueue( name, 0xC000 ) ;
//	}
	FileSelectResult = i ;
}
#endif


void menuProcBattery(uint8_t event)
{
//#ifdef USE_EXT_RTC
//	uint8_t writeRtc = 0 ;
//#endif
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

//#ifdef USE_EXT_RTC
//    case EVT_KEY_LONG(KEY_UP):
//			writeRtc = 1 ;
//    break;
//#endif
  }

		lcd_puts_Pleft( 2*FH, PSTR(STR_Battery));
		putsVolts( 13*FW, 2*FH, g_vbat100mV, 0 ) ;

#ifdef PCBSKY
#ifdef REVB
		if ( g_eeGeneral.ar9xBoard == 0 )
		{
			Current_sum += Current_current ;
			if ( ++Current_count > 49 )
			{
				Current = Current_sum / 500 ;
				Current_sum = 0 ;
				Current_count = 0 ;
			}
			lcd_puts_Pleft( 3*FH, PSTR(STR_CURRENT_MAX));
	  	lcd_outdezAtt( 13*FW, 3*FH, Current, 0 ) ;
	  	lcd_outdezAtt( 20*FW, 3*FH, Current_max/10, 0 ) ;
			lcd_puts_Pleft( 4*FH, XPSTR("mAh\017[MENU]"));
	  	lcd_outdezAtt( 12*FW, 4*FH, MAh_used + Current_used/3600 ,PREC1 ) ;
		}
		lcd_puts_Pleft( 6*FH, PSTR(STR_CPU_TEMP_MAX));
	  lcd_outdezAtt( 12*FW-2, 6*FH, (((((int32_t)Temperature - 838 ) * 621 ) >> 11 ) - 20) ,0 ) ;
	  lcd_outdezAtt( 20*FW-2, 6*FH, (((((int32_t)Max_temperature - 838 ) * 621 ) >> 11 ) - 20) ,0 ) ;

#ifdef PCBSKY
//static uint8_t count = 0 ;
//static uint32_t sum ;
//#ifdef USE_EXT_RTC
//extern uint8_t ExternalRtc[7] ;
//	lcd_outhex4( 0, 7*FH, (ExternalRtc[0] << 8 ) | ExternalRtc[1] ) ;
//	lcd_outhex4( 25, 7*FH, (ExternalRtc[2] << 8 ) | ExternalRtc[3] ) ;
//	lcd_outhex4( 50, 7*FH, (ExternalRtc[4] << 8 ) | ExternalRtc[5] ) ;
//	lcd_outhex4( 75, 7*FH, (ExternalRtc[6] << 8 ) ) ;
//#endif
//	if ( count == 0 )
//	{
//		sum = Analog_values[8] ;
//	}
//	else
//	{
//		sum += Analog_values[8] ;
//	}
//	count += 1 ;
//	if ( count > 1 )
//	{
//		count = 0 ;
//#ifdef USE_EXT_RTC
//extern uint8_t ExternalRtc[7] ;
//extern void readExtRtc() ;
//extern void writeExtRtc() ;
//		if ( ( ExternalRtc[0] & 0x80 ) || writeRtc )
//		{
//			writeExtRtc() ;
//		}
//		else
//		{
//			ExternalRtc[0] = 0x7F ;
//			ExternalRtc[1] = 0xFF ;
//			ExternalRtc[2] = 0xFF ;
//			ExternalRtc[3] = 0xFF ;
//			ExternalRtc[4] = 0xFF ;
//			ExternalRtc[5] = 0xFF ;
//			ExternalRtc[6] = 0x55 ;
//			readExtRtc() ;
//			readRTC() ;

//		}
//#endif

//	}
#endif

#endif

		disp_datetime( 5*FH ) ;
#endif

#ifdef PCB9XT
//	lcd_outhex4( 0, 3*FH, Analog_values[5] ) ;
//	lcd_outhex4( 25, 3*FH, Analog_values[6] ) ;
//	lcd_outhex4( 50, 3*FH, Analog_values[7] ) ;
//extern uint16_t AnalogSwitches ;
//	lcd_outhex4( 0, 4*FH, AnalogSwitches ) ;
	disp_datetime( 5*FH ) ;
#endif


}



//#ifdef JETI
#ifdef REVX

void menuProcJeti(uint8_t event)
{
    TITLE(XPSTR("JETI"));

    switch(event)
    {
    //case EVT_KEY_FIRST(KEY_MENU):
    //  break;
    case EVT_KEY_FIRST(KEY_EXIT):
//        JETI_DisableRXD();
        chainMenu(menuProc0);
        break;
    }

    for (uint8_t i = 0; i < 16; i++)
    {
        lcd_putcAtt((i+2)*FW, 1*FH, JetiBuffer[i], 0 ) ;
        lcd_putcAtt((i+2)*FW, 2*FH, JetiBuffer[i+16], 0 ) ;
//        lcd_putcAtt((i+2)*FW, 3*FH, JetiBuffer[i], 0 ) ;
//        lcd_putcAtt((i+2)*FW, 4*FH, JetiBuffer[i+16], 0 ) ;
    }

extern struct t_16bit_fifo32 Jeti_fifo ;

	lcd_outhex4( 50,  0*FH, JetiIndex ) ;

	lcd_outhex4( 0,  3*FH, Jeti_fifo.fifo[0] ) ;
	lcd_outhex4( 25, 3*FH, Jeti_fifo.fifo[1] ) ;
	lcd_outhex4( 50, 3*FH, Jeti_fifo.fifo[2] ) ;
	lcd_outhex4( 75, 3*FH, Jeti_fifo.fifo[3] ) ;
	lcd_outhex4( 100,3*FH, Jeti_fifo.fifo[4] ) ;

	lcd_outhex4( 0,  4*FH, Jeti_fifo.fifo[5] ) ;
	lcd_outhex4( 25, 4*FH, Jeti_fifo.fifo[6] ) ;
	lcd_outhex4( 50, 4*FH, Jeti_fifo.fifo[7] ) ;
	lcd_outhex4( 75, 4*FH, Jeti_fifo.fifo[8] ) ;
	lcd_outhex4( 100,4*FH, Jeti_fifo.fifo[9] ) ;

	lcd_outhex4( 0,  5*FH, Jeti_fifo.fifo[10] ) ;
	lcd_outhex4( 25, 5*FH, Jeti_fifo.fifo[11] ) ;
	lcd_outhex4( 50, 5*FH, Jeti_fifo.fifo[12] ) ;
	lcd_outhex4( 75, 5*FH, Jeti_fifo.fifo[13] ) ;
	lcd_outhex4( 100,5*FH, Jeti_fifo.fifo[14] ) ;

	lcd_outhex4( 0,  6*FH, Jeti_fifo.fifo[15] ) ;
	lcd_outhex4( 25, 6*FH, Jeti_fifo.fifo[16] ) ;
	lcd_outhex4( 50, 6*FH, Jeti_fifo.fifo[17] ) ;
	lcd_outhex4( 75, 6*FH, Jeti_fifo.fifo[18] ) ;
	lcd_outhex4( 100,6*FH, Jeti_fifo.fifo[19] ) ;

	lcd_outhex4( 0,  7*FH, Jeti_fifo.fifo[20] ) ;
	lcd_outhex4( 25, 7*FH, Jeti_fifo.fifo[21] ) ;
	lcd_outhex4( 50, 7*FH, Jeti_fifo.fifo[22] ) ;
	lcd_outhex4( 75, 7*FH, Jeti_fifo.fifo[23] ) ;
	lcd_outhex4( 100,7*FH, Jeti_fifo.fifo[24] ) ;

//extern uint8_t Scount1 ;
//extern uint8_t Scount2 ;
//extern uint8_t Scount3 ;
//extern uint16_t Sstat1 ;
//extern uint16_t Sstat2 ;
//extern uint16_t Sstat3 ;
//extern uint8_t JetiIndex ;
//extern uint8_t TelemetryType ;


//	lcd_outhex4( 0, 5*FH, JetiIndex ) ;
//	lcd_outhex4( 25, 5*FH, TelemetryType ) ;

//	lcd_outhex4( 50, 6*FH, Sstat1 ) ;
//	lcd_outhex4( 75, 6*FH, Sstat2 ) ;
//	lcd_outhex4( 100, 6*FH, Sstat3 ) ;
	
//	lcd_outhex4( 0, 7*FH, Scount1 ) ;
//	lcd_outhex4( 25, 7*FH, Scount2 ) ;
//	lcd_outhex4( 50, 7*FH, Scount3 ) ;


//    if (JetiBufferReady)
//    {
//        JETI_EnableTXD();

    switch(event)
		{
    	case EVT_KEY_FIRST(KEY_UP):
				jeti_keys &= JETI_KEY_UP ;
			break ;

    	case EVT_KEY_FIRST(KEY_DOWN):
				jeti_keys &= JETI_KEY_DOWN ;
			break ;
    	
			case EVT_KEY_FIRST(KEY_LEFT):
				jeti_keys &= JETI_KEY_LEFT ;
			break ;
    	
			case EVT_KEY_FIRST(KEY_RIGHT):
				jeti_keys &= JETI_KEY_RIGHT ;
			break ;
		}
//        if (keyState((EnumKeys)(KEY_UP))) jeti_keys &= JETI_KEY_UP;
//        if (keyState((EnumKeys)(KEY_DOWN))) jeti_keys &= JETI_KEY_DOWN;
//        if (keyState((EnumKeys)(KEY_LEFT))) jeti_keys &= JETI_KEY_LEFT;
//        if (keyState((EnumKeys)(KEY_RIGHT))) jeti_keys &= JETI_KEY_RIGHT;

        JetiBufferReady = 0;    // invalidate buffer
				
				if ( jeti_keys != JETI_KEY_NOCHANGE )
				{
					JetiIndex = 0 ;
					jetiSendWord( jeti_keys ) ;
				}

//        JETI_putw((uint16_t) jeti_keys);
//        _delay_ms (1);
//        JETI_DisableTXD();

        jeti_keys = JETI_KEY_NOCHANGE ;
//    }
}
#endif

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
  lcd_hline(x-3,y,120+3+3);
  lcd_vline(x,y-32,32+3);

  for(uint8_t i=0; i<120; i+=6)
  {
    lcd_vline(x+i+6,y-1,3);
  }
  for(uint8_t i=1; i<=120; i++)
  {
    lcd_vline(x+i,y-s_traceBuf[traceRd],s_traceBuf[traceRd]);
    traceRd++;
    if(traceRd>=MAXTRACE) traceRd=0;
    if(traceRd==s_traceWr) break;
  }

}

void resetTimern( uint32_t timer )
{
  struct t_timer *tptr = &s_timer[timer] ;
	tptr->s_timerState = TMR_OFF; //is changed to RUNNING dep from mode
  tptr->s_timeCumThr=0;
  tptr->s_timeCumSw=0;
  tptr->s_timeCum16ThrP=0;
	tptr->s_sum = 0 ;
	tptr->last_tmr = g_model.timer[timer].tmrVal ;
	tptr->s_timerVal = ( g_model.timer[timer].tmrDir ) ? 0 : tptr->last_tmr ;
  tptr->s_timeCumAbs = 0 ;
}

void resetTimer1()
{
	resetTimern( 0 ) ;
}

void resetTimer2()
{
	resetTimern( 1 ) ;
}

void resetTimer()
{
	resetTimer1() ;
	resetTimer2() ;
}

#ifdef FRSKY
int16_t AltOffset = 0 ;
#endif

void displayTemp( uint8_t sensor, uint8_t x, uint8_t y, uint8_t size )
{
	if ( ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) ) && ( sensor == 1 ) )
	{
		putsTelemetryChannel( x, y, (int8_t)sensor+TEL_ITEM_T1-1, FrskyHubData[FR_TEMP1+sensor-1], size | LEFT, TELEM_ARDUX_NAME ) ;
		
	}
	else
	{
		putsTelemetryChannel( x, y, (int8_t)sensor+TEL_ITEM_T1-1, FrskyHubData[FR_TEMP1+sensor-1], size | LEFT, 
																( size & DBLSIZE ) ? (TELEM_LABEL | TELEM_UNIT_LEFT) : (TELEM_LABEL | TELEM_UNIT) ) ;
	}
}


static int8_t io_subview = 0 ;

void switchDisplay( uint8_t j, uint8_t a )
{
	uint8_t b = a + 3 ;
	uint8_t y = 4*FH ;
#if defined(PCBSKY) || defined(PCB9XT)
	for(uint8_t i=a; i<b; y += FH, i += 1 )
	{
//#if REVX		
		if ( i == 0 )	// THR
		{
			if ( g_eeGeneral.switchMapping & USE_THR_3POS )
			{
				uint8_t k = HSW_Thr3pos0 ;
				k += switchPosition(k) ;
				lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), k-HSW_OFFSET, 0) ;
				continue ;
			}
		}
		if ( i == 2 )	// ELE
		{
			if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
			{
				uint8_t k = HSW_Ele3pos0 ;
				k += switchPosition(k) ;
				lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), k-HSW_OFFSET, 0) ;
				continue ;
			}
			if ( g_eeGeneral.switchMapping & USE_ELE_6POS )
			{
				uint8_t k = HSW_Ele6pos0 ;
				k += switchPosition(k) ;
				lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), k-HSW_OFFSET, 0) ;
				continue ;
			}
		}
		if ( i == 1 )	// RUD
		{
			if ( g_eeGeneral.switchMapping & USE_RUD_3POS )
			{
				uint8_t k = HSW_Rud3pos0 ;
				k += switchPosition(k) ;
				lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), k-HSW_OFFSET, 0) ;
				continue ;
			}
		}
		if ( i == 6 )	// AIL
		{
			if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
			{
				uint8_t k = HSW_Ail3pos0 ;
				k += switchPosition(k) ;
				lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), k-HSW_OFFSET, 0) ;
				continue ;
			}
		}
		if ( i == 7 )	// GEA
		{
			if ( g_eeGeneral.switchMapping & USE_GEA_3POS )
			{
				uint8_t k = HSW_Gear3pos0 ;
				k += switchPosition(k) ;
				lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), k-HSW_OFFSET, 0) ;
				continue ;
			}
		}
		if ( i == 8 )	// TRN
		{
			uint8_t k = HSW_ID0 ;
			k += switchPosition(k) ;
			lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), k-1, getSwitch00(i+1) ? INVERS : 0 ) ;
			continue ;
		}
//#endif
		lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), i, getSwitch00(i+1) ? INVERS : 0) ;
	}
#endif
#ifdef PCBX9D
	for(uint8_t i=a; i<b; y += FH, i += 1 )
	{
#ifdef REV9E
		if ( i < 18 )
#else
		if ( i < 8 )
#endif	// REV9E
		{
			lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(HW_SWITCHES_STR), i, 0 ) ;
			uint32_t pos = switchPosition( i ) ;
			lcd_putc((4+j*15)*FW-2, y, PSTR(HW_SWITCHARROW_STR)[pos] ) ;
		}
		else
		{
			uint32_t m = 0 ;
			if ( g_eeGeneral.analogMapping & MASK_6POS )
			{
				m = 1 ;
#ifdef REV9E
				if ( i == 18 )
#else
				if ( i == 8 )
#endif	// REV9E
				{
					uint8_t k = HSW_Ele6pos0 ;
					k += switchPosition(k) ;
					lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), k-HSW_OFFSET, 0) ;
					continue ;
				}
			}

#ifdef REV9E
			lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), i-10+1-m, getSwitch00(i-10+2-m) ? INVERS : 0 ) ;
#else
			lcd_putsAttIdx((2+j*15)*FW-2, y, PSTR(SWITCHES_STR), i+1-m, getSwitch00(i+2-m) ? INVERS : 0 ) ;
#endif	// REV9E
		}
	}
#endif
}


#define NUM_HELP_PAGES	2
const uint8_t HelpText0[] = "Enable the Hardware\nMenu: Power on\nholding the left\nhorizontal trim left" ;
const uint8_t HelpText1[] = "Access Bootloader by\nPower on holding\nboth horizontal trims\nto the centre." ;

void setTextBuffer( uint8_t *source, uint32_t number )
{
	uint32_t i ;
	uint32_t j = 0 ;
	uint32_t k = 0 ;
	uint8_t *p = source ;
	for ( i= 0 ; i < number ; i += 1 )
	{
		uint8_t c = *p++ ;
		if ( c == 10 )
		{
			j = 0 ;
			k += 21 ;
			TextLines += 1 ;
			if ( k >= 16*21 )
			{
				break ;
			}
		}
		if ( c < ' ' )
		{
			continue ;
		}
		if ( j >= 21 )
		{
			j = 0 ;
			k += 21 ;
			if ( k >= 16*21 )
			{
				break ;
			}
		}
		TextMenuBuffer[k+j] = c ;
		j += 1 ;
	}
	TextLines = k / 21 ;
	if ( j )
	{
		TextLines += 1 ;
	}
}



void menuProcText(uint8_t event)
{
	uint8_t filename[50] ;
	FRESULT result ;
	UINT nread ;

	if ( event == EVT_ENTRY )
	{
 		memset( TextMenuBuffer,' ', 16*21 ) ;
	 if ( TextHelp )
	 {
	 		HelpTextPage = 0 ;
			uint8_t *p = (uint8_t *) HelpText0 ;
			setTextBuffer( p, strlen( ( char *)p ) ) ;
	 }
	 else
	 {
		setModelFilename( filename, g_eeGeneral.currModel+1, FILE_TYPE_TEXT ) ;
  	result = f_open( &TextFile, (TCHAR *)filename, FA_READ ) ;
		TextLines = 0 ;
		if ( result == FR_OK )
		{
			result = f_read( &TextFile, TextMenuStore, 16*21+32, &nread ) ;
			if ( result == FR_OK )
			{
				setTextBuffer( TextMenuStore, nread ) ;
			}
		}
		f_close( &TextFile ) ;
		if ( result != FR_OK )
		{
  		strncpy_P( (char *)&TextMenuBuffer[63+3], XPSTR("No Notes Found"), 14 );
			TextLines = 8 ;
		}
	 }
		TextOffset = 0 ;
	}
	if ( ( event == EVT_KEY_FIRST(KEY_EXIT) ) || ( event == EVT_KEY_LONG(BTN_RE) ) )
	{
		killEvents(event) ;
    popMenu(false) ;
	}
	if ( ( event == EVT_KEY_FIRST(KEY_UP) ) || ( event== EVT_KEY_REPT(KEY_UP) ) )
	{
		if ( TextOffset )
		{
			TextOffset -= 1 ;
		}
	}
	if ( ( event == EVT_KEY_FIRST(KEY_DOWN) ) || ( event== EVT_KEY_REPT(KEY_DOWN) ) )
	{
		if ( TextLines > 8 )
		{
			if ( TextOffset < TextLines - 8 )
			{
				TextOffset += 1 ;
			}
		}
	}
	if ( TextHelp )
	{
		if ( ( event == EVT_KEY_FIRST(KEY_RIGHT) ) || ( event== EVT_KEY_REPT(KEY_RIGHT) ) )
		{
			HelpTextPage += 1 ;
			if ( HelpTextPage >= NUM_HELP_PAGES )
			{
				HelpTextPage = 0 ;
			}
	 		memset( TextMenuBuffer,' ', 16*21 ) ;
			uint8_t *p = (uint8_t *) ( HelpTextPage ? HelpText1 : HelpText0 ) ;
			setTextBuffer( p, strlen( ( char *)p ) ) ;
		}
		if ( ( event == EVT_KEY_FIRST(KEY_LEFT) ) || ( event== EVT_KEY_REPT(KEY_LEFT) ) )
		{
			if ( HelpTextPage == 0 )
			{
				HelpTextPage = NUM_HELP_PAGES - 1 ;
			}
			else
			{
				HelpTextPage -= 1 ;
			}
	 		memset( TextMenuBuffer,' ', 16*21 ) ;
			uint8_t *p = (uint8_t *) ( HelpTextPage ? HelpText1 : HelpText0 ) ;
			setTextBuffer( p, strlen( ( char *)p ) ) ;
		}
	}
	// display buffer
	uint32_t i ;
	uint32_t j ;
	uint8_t y = 0 ;
	for ( i = 0 ; i < 8*21 ; y += FH, i += 21 )
	{
		j = TextOffset * 21 + i ;
		lcd_putsnAtt( 0, y, (const char *)&TextMenuBuffer[j], 21, 0 ) ;
	}
}



#define MAIN_POPUP_ENABLED	1

void menuProc0(uint8_t event)
{
  static uint8_t trimSwLock;
  uint8_t view = g_model.mview & 0xf;
  uint8_t tview = g_model.mview & 0x70 ;
	if ( tview > 0x50 )
	{
		tview = 0x50 ;
	}
	
	StickScrollAllowed = 0 ;

#ifdef MAIN_POPUP_ENABLED

	if ( ! PopupData.PopupActive )
	{
		switch(event)
		{
    	case EVT_KEY_LONG(KEY_MENU) : // Nav Popup
	    case EVT_KEY_LONG(BTN_RE) :		// Nav Popup
				PopupData.PopupActive = 2 ;
				PopupData.PopupIdx = 0 ;
      	killEvents(event) ;
				event = 0 ;
				Tevent = 0 ;
    	break ;

#ifndef REV9E
	    case EVT_KEY_BREAK(KEY_MENU) :			// ***************
#endif	// nREV9E
			case EVT_KEY_BREAK(BTN_RE) :
				if ( view == e_telemetry )
				{
					PopupData.PopupActive = 1 ;
					PopupData.PopupIdx = 0 ;
      		killEvents(event) ;
					event = 0 ;
					Tevent = 0 ;
				}
    	break ;

#ifdef REV9E
	    case EVT_KEY_BREAK(KEY_MENU) :			// ***************
#endif	// REV9E
  	  case EVT_KEY_BREAK(KEY_RIGHT) :
        if(view <= e_inputs1)
				{
					int8_t x ;
					x = io_subview ;
#ifdef REV9E
					if ( ++x > ((view == e_inputs1) ? 6 : 2) ) x = 0 ;
#else
					if ( ++x > ((view == e_inputs1) ? 4 : 2) ) x = 0 ;
#endif	// REV9E
					io_subview = x ;
				}	
        if(view == e_telemetry)
				{
					uint32_t t_limit = 0x40 ;
					tview += 0x10 ;
					if ( ( FrskyTelemetryType == 2 ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) )
					{
						t_limit = 0x50 ;
					}
					if ( tview > t_limit )
					{
						tview = 0 ;
					}
					
            g_model.mview = e_telemetry | tview ;
						eeModelChanged() ;
            //            STORE_GENERALVARS;     //eeWriteGeneral();
            //            eeDirty(EE_GENERAL);
            audioDefevent(AU_MENUS);
        }
      break ;

#ifndef REV9E
	    case EVT_KEY_BREAK(KEY_LEFT):
        if(view <= e_inputs1)
				{
					int8_t x ;
					x = io_subview ;
					if ( --x < 0 ) x = (view == e_inputs1) ? 4 : 2 ;
					io_subview = x ;
				}	
        else if(view == e_telemetry)
				{
					tview -= 0x10 ;
					uint32_t t_limit = 0x40 ;
					if ( ( FrskyTelemetryType == 2 ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) )
					{
						t_limit = 0x50 ;
					}
					if ( tview > 0x60 )
					{
						tview = t_limit ;
					}
					
          g_model.mview = e_telemetry | tview ;
					eeModelChanged() ;
          audioDefevent(AU_MENUS) ;
        }
      break ;
#endif	// nREV9E

#ifndef REV9E
	    case EVT_KEY_LONG(KEY_RIGHT) :
        pushMenu(menuProcModelSelect) ;
        scroll_disabled = 1 ;
        killEvents(event) ;
			break ;
    
			case EVT_KEY_LONG(KEY_LEFT) :
        pushMenu(menuProcIndex) ;
        scroll_disabled = 1 ;
        killEvents(event) ;
			break ;
#endif	// nREV9E
    
			case EVT_KEY_FIRST(KEY_EXIT) :
        if(s_timer[0].s_timerState==TMR_BEEPING)
				{
          s_timer[0].s_timerState = TMR_STOPPED ;
          audioDefevent(AU_MENUS) ;
        }
        else if (view == e_telemetry)
				{
          resetTelemetry() ;
          audioDefevent(AU_MENUS) ;
        }
			break ;

#ifdef REV9E
			case EVT_KEY_BREAK(KEY_LEFT) :
	      if(view>0)
  	      view = view - 1 ;
    	  else
      	  view = MAX_VIEWS-1 ;
  	    audioDefevent(AU_KEYPAD_DOWN) ;
	      g_model.mview = view | tview ;
				eeModelChanged() ;
				io_subview = 0 ;
	    break ;
#endif

#ifndef REV9E
			case EVT_KEY_BREAK(KEY_UP) :
				view += 1 ;
    	  if( view>=MAX_VIEWS) view = 0 ;
    	  audioDefevent(AU_KEYPAD_UP) ;
    	  g_model.mview = view | tview ;
				eeModelChanged() ;
				io_subview = 0 ;
    	break;

			case EVT_KEY_BREAK(KEY_DOWN) :
	      if(view>0)
  	      view = view - 1 ;
    	  else
      	  view = MAX_VIEWS-1 ;
  	    audioDefevent(AU_KEYPAD_DOWN) ;
	      g_model.mview = view | tview ;
				eeModelChanged() ;
				io_subview = 0 ;
	    break ;

	    case EVT_KEY_LONG(KEY_DOWN):
 				view = e_telemetry ;
				g_model.mview = view | tview ;
				eeModelChanged() ;
      	audioDefevent(AU_MENUS) ;
    	  killEvents(event) ;
  	  break;
    
			case EVT_KEY_LONG(KEY_UP) :
  		  pushMenu(menuProcBattery) ;
    	  killEvents(event) ;
    	break ;
#endif	// nREV9E

	    case EVT_KEY_LONG(KEY_EXIT) :
        resetTimer() ;
        resetTelemetry() ;
        audioDefevent(AU_MENUS) ;
			break ;
    	
			case EVT_ENTRY:
        killEvents(KEY_EXIT) ;
        killEvents(KEY_UP) ;
        killEvents(KEY_DOWN) ;
        trimSwLock = true ;
				io_subview = 0 ;
			break ;

		}
	} // !PopupActive
#else

 if ( ! PopupData.PopupActive )
 {
	switch(event)
	{
    case  EVT_KEY_LONG(KEY_MENU):// go to last menu
#ifdef FRSKY
        if( (view == e_telemetry) && ((tview & 0x70) == 0x30 ) )
        {
          AltOffset = -FrskyHubData[FR_ALT_BARO] ;
        }
        else if( (view == e_telemetry) && ( ( (tview & 0x70) == 0 ) !! ((tview & 0x70) == 0x10 )) )
        {
#ifdef PCBX9D	
          for (uint32_t i=0; i<9; i++)
#else
          for (uint32_t i=0; i<6; i++)
#endif
					{
						uint32_t j ;
						j = tview == 0 ? g_model.customDisplayIndex[i]-1 : g_model.customDisplay2Index[i]-1 ;
#ifdef PCBX9D	
						if ( i >= 6 )
						{
							j = tview == 0 ? g_model.customDisplay1Extra[i-6]-1 : g_model.customDisplay2Extra[i-6]-1 ;
						}
#endif
						// Might check for a Scaler as well to see if ALT is its source
						if ( j == TEL_ITEM_BALT )
						{
            	AltOffset = -FrskyHubData[FR_ALT_BARO] ;
						}
						if ( ( j >= TEL_ITEM_SC1 ) && (j <= TEL_ITEM_SC8 ) )
						{
							j = g_model.Scalers[j-TEL_ITEM_SC1].source - 1 ;
							if ( j == TEL_ITEM_BALT )
							{
    	        	AltOffset = -FrskyHubData[FR_ALT_BARO] ;
							}
						}
						if ( j == TEL_ITEM_A1 )
						{
            	if ( g_model.frsky.channels[0].type == 3 )		// Current (A)
							{
				    	  frskyTelemetry[0].setoffset() ;
							}
						}
						if ( j == TEL_ITEM_A2 )
						{
            	if ( g_model.frsky.channels[1].type == 3 )		// Current (A)
							{
				    	  frskyTelemetry[1].setoffset() ;
							}
						}
					}
        }
        else if( (view == e_telemetry) && ((tview & 0x70) == 0x40 ) )	// GPS
				{
					struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;
			
					maxMinPtr->hubMax[FR_GPS_SPEED] = 0 ;
					maxMinPtr->hubMax[FR_GPS_ALT] = 0 ;
				}
        else
        {
#endif
#ifdef REV9E
  	  pushMenu(menuProcBattery);
#else
    case  EVT_KEY_LONG(BTN_RE):// go to last menu
		        scroll_disabled = 1;
            pushMenu(lastPopMenu());
//            killEvents(event);
#endif	// REV9E
#ifdef FRSKY
        }
#endif
        killEvents(event);
    break;

#ifdef REV9E
    case EVT_KEY_LONG(BTN_RE):
#endif	// REV9E
    case EVT_KEY_LONG(KEY_RIGHT):
        scroll_disabled = 1;
        pushMenu(menuProcModelSelect);
        killEvents(event);
        break;

#ifdef REV9E
    case EVT_KEY_BREAK(KEY_MENU):			// ***************
#endif	// REV9E
    case EVT_KEY_BREAK(KEY_RIGHT):
        if(view <= e_inputs1)
				{
					int8_t x ;
					x = io_subview ;
#ifdef REV9E
					if ( ++x > ((view == e_inputs1) ? 6 : 2) ) x = 0 ;
#else
					if ( ++x > ((view == e_inputs1) ? 4 : 2) ) x = 0 ;
#endif	// REV9E
					io_subview = x ;
				}	
#ifdef FRSKY
        if(view == e_telemetry)
				{
					tview += 0x10 ;
					if ( tview > ( ( FrskyTelemetryType != 2 ) ? 0x30 : 0x40 ) )
					{
						tview = 0 ;
					}
					
            g_model.mview = e_telemetry | tview ;
						eeModelChanged() ;
            //            STORE_GENERALVARS;     //eeWriteGeneral();
            //            eeDirty(EE_GENERAL);
            audioDefevent(AU_MENUS);
        }
#endif
        break;
#ifndef REV9E
    case EVT_KEY_BREAK(KEY_LEFT):
        if(view <= e_inputs1)
				{
					int8_t x ;
					x = io_subview ;
					if ( --x < 0 ) x = (view == e_inputs1) ? 4 : 2 ;
					io_subview = x ;
				}	
#ifdef FRSKY
        if(view == e_telemetry)
				{
					tview -= 0x10 ;
					if ( tview > 0x40 )
					{
						tview = ( FrskyTelemetryType != 2 ) ? 0x30 : 0x40 ;
					}
            g_model.mview = e_telemetry | tview ;
						eeModelChanged() ;
            //            STORE_GENERALVARS;     //eeWriteGeneral();
            //            eeDirty(EE_GENERAL);
            audioDefevent(AU_MENUS);
        }
#endif
        break;
#endif	// nREV9E
    case EVT_KEY_LONG(KEY_LEFT):
        scroll_disabled = 1;
        pushMenu(menuProcIndex);
        killEvents(event);
        break;
		case EVT_KEY_BREAK(KEY_UP) :
			view += 1 ;
      if( view>=MAX_VIEWS) view = 0 ;
      g_model.mview = view | tview ;
			eeModelChanged() ;
//      STORE_GENERALVARS;     //eeWriteGeneral() ;
      audioDefevent(AU_KEYPAD_UP) ;
			io_subview = 0 ;
    break;
#ifdef REV9E
		case EVT_KEY_BREAK(KEY_PAGE) :
#endif	// REV9E
		case EVT_KEY_BREAK(KEY_DOWN) :
      if(view>0)
        view = view - 1;
      else
        view = MAX_VIEWS-1;
      g_model.mview = view | tview ;
			eeModelChanged() ;
//      STORE_GENERALVARS;     //eeWriteGeneral() ;
      audioDefevent(AU_KEYPAD_DOWN) ;
			io_subview = 0 ;
    break;
    case EVT_KEY_LONG(KEY_UP):
  	  pushMenu(menuProcBattery);
      killEvents(event);
    break;
    case EVT_KEY_LONG(KEY_DOWN):
//#if defined(JETI)
//        JETI_EnableRXD(); // enable JETI-Telemetry reception
//        chainMenu(menuProcJeti);
//#elif defined(ARDUPILOT)
#if defined(ARDUPILOT)
        ARDUPILOT_EnableRXD(); // enable ArduPilot-Telemetry reception
        chainMenu(menuProcArduPilot);
#elif defined(NMEA)
        NMEA_EnableRXD(); // enable NMEA-Telemetry reception
        chainMenu(menuProcNMEA);
#else
			
 #ifdef REVX
			if ( g_model.telemetryProtocol == TELEMETRY_JETI )
			{
				chainMenu(menuProcJeti);
			}			        
			else
			{
 #endif
 				view = e_telemetry ;
				g_model.mview = view | tview ;
				eeModelChanged() ;
 #ifdef REVX
			}
 #endif
      audioDefevent(AU_MENUS);
//        chainMenu(menuProcStatistic2);
#endif
      killEvents(event);
    break;
    case EVT_KEY_FIRST(KEY_EXIT):
        if(s_timer[0].s_timerState==TMR_BEEPING) {
            s_timer[0].s_timerState = TMR_STOPPED;
            audioDefevent(AU_MENUS);
        }
//        else if(view == e_timer2) {
//            resetTimer2();
//            // Timer2_running = !Timer2_running;
//            audioDefevent(AU_MENUS);
//        }
#ifdef FRSKY
        else if (view == e_telemetry) {
            resetTelemetry();
            audioDefevent(AU_MENUS);
        }
#endif
        break;
    case EVT_KEY_LONG(KEY_EXIT):
        resetTimer();
//        resetTimer2();
#ifdef FRSKY
        resetTelemetry();
#endif
        audioDefevent(AU_MENUS);
        break;
    case EVT_ENTRY:
        killEvents(KEY_EXIT);
        killEvents(KEY_UP);
        killEvents(KEY_DOWN);
        trimSwLock = true;
				io_subview = 0 ;
        break;

//#ifndef REV9E
//    case EVT_KEY_BREAK(KEY_MENU):			// ***************
//#endif	// nREV9E
//		case EVT_KEY_BREAK(BTN_RE) :
//			PopupData.PopupIdx = 0 ;
//			PopupData.PopupActive = 1 ;
//      killEvents(event) ;
//			event = 0 ;
//    break ;
  }
 } // !PopupActive

#endif	// MAIN_POPUP_ENABLED


	{
		uint8_t tsw ;
		tsw = getSwitch00(g_model.trimSw) ;
		if( tsw && !trimSwLock) setStickCenter() ;
		trimSwLock = tsw ;
	}

  if (view != e_telemetry)
	{
	  register  uint8_t x=FW*2;
  	register uint8_t att = (g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0) | DBLSIZE;
  	register uint32_t i ;

		putsDblSizeName( 0 ) ;
		
//extern uint16_t XxxCount ;
//extern uint16_t YyyCount ;
		
//		lcd_outhex4( 20, 0, XxxCount ) ;
//		lcd_outhex4( 60, 0, YyyCount ) ;

//#endif
//extern uint16_t SSCdebug ;
//lcd_outhex4( 50, 0, SSCdebug ) ;
//#ifdef PCBX9D
// #ifdef REVPLUS
//extern uint8_t DmaDebugDone ;
//extern uint16_t DmaDebugCount ;
//extern uint16_t DmaDebugNDTR ;

//	lcd_outhex4( 20, 0, DmaDebugDone ) ;
//	lcd_outhex4( 20, FH, DmaDebugCount ) ;
//	lcd_outhex4( 50, FH, DmaDebugNDTR ) ;
// #endif
//#endif

//extern uint16_t SpiDebug[4] ;

//	lcd_outhex4( 0, 0, SPI3->CR1 ) ;
//	lcd_outhex4( 0, FH, SPI3->SR ) ;
//	lcd_outhex4( 32, 0, SpiDebug[0] ) ;
//	lcd_outhex4( 32, FH, SpiDebug[1] ) ;
//#endif

//extern uint16_t RotaryAnalogValue ;
//extern uint16_t VpaeValue ;
//extern volatile int32_t Rotary_position ;
//extern volatile int32_t Rotary_count ;
//lcd_outhex4( 0, 0, RotaryAnalogValue >> 1 ) ;
//lcd_outhex4( 25, 0, Rotary_position ) ;
//lcd_outhex4( 50, 0, Rotary_count ) ;
//lcd_outhex4( 75, 0, VpaeValue ) ;

//extern uint8_t AnaEncSw ;
//extern uint8_t AnaRot0 ;
//extern uint16_t AnaRot1 ;
//lcd_outhex4( 0, 0, AnaRot0 ) ;
//lcd_outhex4( 25, 0, AnaRot1 ) ;
//lcd_outhex4( 50, 0, AnaEncSw ) ;

    putsVBat( 6*FW+1, 2*FH, att|NO_UNIT|CONDENSED);
    lcd_putc( 6*FW+2, 3*FH, 'V');

//    if(g_model.timer[0].tmrModeA != TMRMODE_NONE)
//		{
			displayTimer( x+14*FW-1, FH*2, 0, DBLSIZE|CONDENSED ) ;
      putsTmrMode(x+7*FW-FW/2,FH*3,0, 0, 0 ) ;
//  	}

		i = getFlightPhase() ;
		if ( i && g_model.phaseData[i-1].name[0] )
		{
			lcd_putsnAtt( 6*FW+2, 2*FH, g_model.phaseData[i-1].name, 6, /*BSS*/ 0 ) ;
			lcd_rect( 6*FW+1, 2*FH-1, 6*FW+2, 9 ) ;
		}
		else
		{
    	lcd_putsAttIdx( 6*FW+2, 2*FH,PSTR(STR_TRIM_OPTS),g_model.trimInc, 0);
			if ( g_model.thrTrim )
			{
				lcd_puts_P(x+8*FW-FW/2-1,2*FH,PSTR(STR_TTM));
			}
		}

  	//trim sliders
  	for( i=0 ; i<4 ; i++ )
  	{
	#define TL 27
  	  //                        LH LV RV RH
  	  static uint8_t x[4]    = {128*1/4+2, 4, 128-4, 128*3/4-2};
//  	  static uint8_t vert[4] = {0,1,1,0};
  	  register uint8_t xm, ym ;
//#ifdef FIX_MODE
			xm = modeFixValue( i ) ;
      xm = x[xm-1] ;
//#else
//  	  xm=x[i] ;
//#endif
  	  
			
			register int16_t valt = getTrimValue( CurrentPhase, i ) ;
			uint8_t centre = (valt == 0) ;
      int8_t val = max((int8_t)-(TL+1),min((int8_t)(TL+1),(int8_t)(valt/4)));
//  	  if(vert[i])
      if( (i == 1) || ( i == 2 ))
			{
  	    ym=31;
  	    lcd_vline(xm,   ym-TL, TL*2);

//#ifdef FIX_MODE
          if((i == 1) || !(g_model.thrTrim))
//#else
//  	      if(((g_eeGeneral.stickMode&1) != (i&1)) || !(g_model.thrTrim))
//#endif
					{
  	          lcd_vline(xm-1, ym-1,  3);
  	          lcd_vline(xm+1, ym-1,  3);
  	      }
  	      ym -= val;
  	  }else{
  	    ym=59;
  	    lcd_hline(xm-TL,ym,    TL*2);
  	    lcd_hline(xm-1, ym-1,  3);
  	    lcd_hline(xm-1, ym+1,  3);
  	    xm += val;
  	  }
  		DO_SQUARE(xm,ym,7)
			if ( centre )
			{
        DO_SQUARE(xm,ym,5) ;
			}
		}
 	}
  else
	{
		uint8_t i ;
		i = getFlightPhase() ;
		if ( i && g_model.phaseData[i-1].name[0] )
		{
			lcd_putsnAtt( 2*FW, 0, g_model.phaseData[i-1].name, 6, /*BSS*/ 0 ) ;
		}
		else
		{
    	lcd_putsnAtt(0, 0, g_model.name, sizeof(g_model.name), INVERS);
		}
    
		uint8_t att = (g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0);
    putsVBat(14*FW,0,att);
//    if(g_model.timer[0].tmrModeA != TMRMODE_NONE)
//		{
			displayTimer( 18*FW+3, 0, 0, 0 ) ;
//    }
  }
  
	if(view<e_inputs1)
	{
		if ( io_subview > 2 )
		{
			io_subview = 0 ;
		}
    lcd_hlineStip(38, 33, 54, 0x55 ) ;
    lcd_hlineStip(38, 34, 54, 0x55 ) ;

		if ( ( io_subview == 0 ) || ( BLINK_ON_PHASE ) )
		{
    	lcd_hlineStip(38 + io_subview * 18, 33, 18, 0xAA ) ;
    	lcd_hlineStip(38 + io_subview * 18, 34, 18, 0xAA ) ;
		}
		
    register uint32_t i ;
    for( i=0; i<8; i++)
    {
      uint8_t x0,y0;
			uint8_t chan = 8 * io_subview + i ;
      int16_t val = g_chans512[chan];
          //val += g_model.limitData[i].revert ? g_model.limitData[i].offset : -g_model.limitData[i].offset;
      switch(view)
      {
        case e_outputValues:
          x0 = i%4 ;
          x0 *= 28 ;
          y0 = i/4*FH+40;
              // *1000/1024 = x - x/8 + x/32
#define GPERC(x)  (x - x/32 + x/128)
          lcd_outdezAtt( x0+6*FW , y0, GPERC(val),PREC1 ) ;
        break;
            
				case e_outputBars:
#define WBAR2 (50/2)
          x0       = i<4 ? 128/4+2 : 128*3/4-2;
          y0       = 38+(i%4)*5;
    			int16_t limit = (g_model.extendedLimits ? 1280 : 1024);
          int8_t l = (abs(val) * WBAR2 + 512) / limit ;
          if(l>WBAR2)  l =  WBAR2;  // prevent bars from going over the end - comment for debugging

          lcd_hlineStip(x0-WBAR2,y0,WBAR2*2+1,0x55);
          lcd_vline(x0,y0-2,5);
          if(val>0)
					{
            x0+=1;
          }else{
            x0-=l;
          }
          lcd_hline(x0,y0+1,l);
          lcd_hline(x0,y0-1,l);
                
				break;
      }
    }
  }
#ifdef FRSKY
    else if(view == e_telemetry)
		{
#ifdef PCBX9D
			ImageDisplay = 1 ;
#endif

extern uint8_t LogsRunning ;
			if ( LogsRunning & 1 )
			{
				if ( BLINK_ON_PHASE )
				{
					lcd_img( 10*FW+1, 0, IconLogging, 0, 0 ) ;
//					lcd_putc( 10*FW+3, 0, 'o' ) ;
				}
			}
			
//        static uint8_t staticTelemetry[4];
//        static uint8_t staticRSSI[2];
        static enum AlarmLevel alarmRaised[2];
//        int8_t unit ;
        int16_t value ;
//        if ( (frskyStreaming) || ( tview  == 0x30 ) )
				{
            uint8_t y0, x0, blink;
//            if (++displayCount > 49) {
//                displayCount = 0;
                for (int i=0; i<2; i++) {
//                    staticTelemetry[i] = frskyTelemetry[i].value;
//                    staticRSSI[i] = frskyRSSI[i].value;
//										if ( i < 2 )
//										{
                      alarmRaised[i] = FRSKY_alarmRaised(i);
//										}
                }
//            }
            if ( tview == 0x20 )
            {
//                if (g_model.frsky.channels[0].ratio || g_model.frsky.channels[1].ratio) {
                    x0 = 0;
                    for (int i=0; i<2; i++) {
                        if (g_model.frsky.channels[i].ratio) {
                            blink = (alarmRaised[i] ? INVERS : 0);
                            lcd_puts_P(x0, 3*FH, PSTR(STR_A_EQ) ) ;
                            lcd_putc(x0+FW, 3*FH, '1'+i);
                            x0 += 3*FW;
                            putsTelemValue( x0-2, 2*FH, frskyTelemetry[i].value, i,  blink|DBLSIZE|LEFT ) ;
                            if ( g_model.frsky.channels[i].type == 3 )		// Current (A)
														{
                              lcd_outdezAtt(x0+FW, 4*FH,  FrskyHubData[FR_A1_MAH+i], 0);
														}
														else
														{
                              putsTelemValue(x0+FW, 4*FH, FrskyHubMaxMin.hubMin[i], i, 0 ) ;
														}
                            putsTelemValue(x0+3*FW, 4*FH, FrskyHubMaxMin.hubMax[i], i, LEFT ) ;
                        }
                        x0 = 11*FW-2;
                    }
//                }
								// Fuel Gauge
                if (frskyUsrStreaming)
								{
                	lcd_puts_Pleft( 1*FH, PSTR(STR_FUEL)) ;
									x0 = FrskyHubData[FR_FUEL] ;		// Fuel gauge value
									lcd_hbar( 25, 9, 102, 6, x0 ) ;
								}
                lcd_puts_Pleft( 6*FH, PSTR(STR_RXEQ));
								putTxSwr( 11 * FW - 2, 6*FH, 0 ) ;
//                lcd_puts_P(11 * FW - 2, 6*FH, Str_TXeq );
//                if (frskyStreaming)
//								{
               	lcd_outdezAtt(3 * FW - 2, 5*FH, frskyTelemetry[2].value, DBLSIZE|LEFT);
//								}
                lcd_outdezAtt(14 * FW - 4, 5*FH, FrskyHubData[FR_TXRSI_COPY], DBLSIZE|LEFT);
                
								struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;
								
								lcd_outdezAtt(4 * FW, 7*FH, maxMinPtr->hubMin[2], 0);
                lcd_outdezAtt(6 * FW, 7*FH, maxMinPtr->hubMax[2], LEFT);
                lcd_outdezAtt(15 * FW - 2, 7*FH, maxMinPtr->hubMin[3], 0);
                lcd_outdezAtt(17 * FW - 2, 7*FH, maxMinPtr->hubMax[3], LEFT);
            }
            else if ( tview == 0x30 )
            {
                if (frskyUsrStreaming)
                {
									displayTemp( 1, 0, 2*FH, DBLSIZE ) ;
									displayTemp( 2, 14*FW, 7*FH, 0 ) ;

                    lcd_puts_Pleft( 2*FH, PSTR(STR_12_RPM));
                    lcd_outdezNAtt(13*FW, 1*FH, (uint16_t)FrskyHubData[FR_RPM], DBLSIZE|LEFT, 5);

//                    lcd_puts_Pleft( 4*FH, PSTR(STR_ALTEQ));
                    value = FrskyHubData[FR_ALT_BARO] + AltOffset ;
//										if (g_model.FrSkyUsrProto == 1)  // WS How High
//										{
//                        unit = 'f' ;  // and ignore met/imp option
//										}
//										else
//										{
//                      unit = 'm' ;
//                      if ( g_model.FrSkyImperial )
//                      {
//                        // m to ft *105/32
//                       	value = value * 3 + ( value >> 2 ) + (value >> 5) ;
//                        unit = 'f' ;
//                      }
//										}
//                    lcd_putc( 3*FW, 3*FH, unit ) ;
//                    lcd_outdezAtt(4*FW, 3*FH, value, DBLSIZE|LEFT);
									putsTelemetryChannel( 0, 4*FH, TEL_ITEM_BALT, value, DBLSIZE | LEFT, (TELEM_LABEL | TELEM_UNIT_LEFT)) ;
                
								
								}	
                if (g_model.frsky.channels[0].ratio)
                {
                    blink = (alarmRaised[0] ? INVERS+BLINK : 0);
                    lcd_puts_Pleft( 6*FH, XPSTR("A1="));
                    putsTelemValue( 3*FW-2, 5*FH, frskyTelemetry[0].value, 0,  blink|DBLSIZE|LEFT ) ;
                }
                if (g_model.frsky.channels[1].ratio)
                {
                    blink = (alarmRaised[1] ? INVERS+BLINK : 0);
                    lcd_puts_P(11*FW-2, 6*FH, XPSTR("A2="));
                    putsTelemValue( 14*FW-2, 5*FH, frskyTelemetry[1].value, 1,  blink|DBLSIZE|LEFT ) ;
                }
                lcd_puts_Pleft( 7*FH, PSTR(STR_RXEQ) );
                lcd_outdezAtt(3 * FW, 7*FH, FrskyHubData[FR_RXRSI_COPY], LEFT);
								putTxSwr( 8 * FW , 7*FH, 0 ) ;
//                lcd_puts_P(8 * FW, 7*FH, PSTR(STR_TXEQ) );
                lcd_outdezAtt(11 * FW, 7*FH, FrskyHubData[FR_TXRSI_COPY], LEFT);
            }
            else if ( tview == 0x40 )
            {
							uint8_t blink = BLINK ;
							uint16_t mspeed ;
              if (frskyUsrStreaming)
							{
								blink = 0 ;
							}

                lcd_puts_Pleft( 2*FH, PSTR(STR_LAT_EQ)) ;
								if ( g_eeGeneral.gpsFormat )
								{
									int32_t value ;
	                lcd_outdezNAtt(6*FW+2, 2*FH, FrskyHubData[FR_GPS_LAT]/100, LEADING0 | blink, -3);
  	              lcd_putc(6*FW+2, 2*FH, '.') ;
									value = FrskyHubData[FR_GPS_LAT]%100 ;
									value *= 10000 ;
									value += FrskyHubData[FR_GPS_LATd] ;
									value *= 10 ;
									value /= 6 ;
                	lcd_outdezNAtt(12*FW, 2*FH, value, LEADING0 | blink, -6);
									lcd_outdezNAtt(6*FW+2, 3*FH, FrskyHubData[FR_GPS_LONG]/100, LEADING0 | blink, -3);
      	          lcd_putc(6*FW+2, 3*FH, '.') ;
									value = FrskyHubData[FR_GPS_LONG]%100 ;
									value *= 10000 ;
									value += FrskyHubData[FR_GPS_LONGd] ;
									value *= 10 ;
									value /= 6 ;
                	lcd_outdezNAtt(12*FW, 3*FH, value, LEADING0 | blink, -6);
								}
								else
								{
                	lcd_outdezNAtt(8*FW, 2*FH, FrskyHubData[FR_GPS_LAT], LEADING0 | blink, -5);
                	lcd_putc(8*FW, 2*FH, '.') ;
                	lcd_outdezNAtt(12*FW, 2*FH, FrskyHubData[FR_GPS_LATd], LEADING0 | blink, -4);
//	                lcd_puts_Pleft( 3*FH, PSTR(STR_LON_EQ)) ;
                	lcd_outdezNAtt(8*FW, 3*FH, FrskyHubData[FR_GPS_LONG], LEADING0 | blink, -5);
                	lcd_putc(8*FW, 3*FH, '.') ;
                	lcd_outdezNAtt(12*FW, 3*FH, FrskyHubData[FR_GPS_LONGd], LEADING0 | blink, -4);
								}
								lcd_putcAtt( 14*FW, 2*FH, FrskyHubData[FR_LAT_N_S], blink ) ;
								lcd_putcAtt( 14*FW, 3*FH, FrskyHubData[FR_LONG_E_W], blink ) ;
//                lcd_puts_Pleft( 4*FH, PSTR(STR_ALT_MAX)) ;
								uint16_t alt ;
								alt = FrskyHubMaxMin.hubMax[FR_GPS_ALT] ;
                if ( g_model.FrSkyImperial )
								{
									alt = m_to_ft( alt ) ;
								}
                lcd_outdezAtt(20*FW, 4*FH, alt, 0);
                lcd_outdez(20*FW, 3*FH, FrskyHubData[FR_COURSE] );
                
//								lcd_puts_Pleft( 5*FH, PSTR(STR_SPD_KTS_MAX)) ;
//                lcd_outdezAtt(20*FW, 5*FH, MaxGpsSpeed, blink );

								mspeed = FrskyHubMaxMin.hubMax[FR_GPS_SPEED] ;
                if ( g_model.FrSkyImperial )
								{
									lcd_puts_Pleft( 5*FH, PSTR(STR_11_MPH)) ;
									mspeed = ( mspeed * 589 ) >> 9 ;
								}
                lcd_outdezAtt(20*FW, 5*FH, mspeed, blink );
								lcd_puts_Pleft( 6*FH, XPSTR("V1=\007V2=\016V3=""\037""V4=\007V5=\016V6=")) ;
              if (frskyUsrStreaming)
							{
								uint16_t alt ;
								alt = FrskyHubData[FR_GPS_ALT] ;
								mspeed = FrskyHubData[FR_GPS_SPEED] ;
                if ( g_model.FrSkyImperial )
								{
									mspeed = ( mspeed * 589 ) >> 9 ;
									alt = m_to_ft( alt ) ;
									lcd_putc( 9*FW, 4*FH, 'f' ) ;
								}
								lcd_outdezAtt(8 * FW, 4*FH, alt, PREC1 ) ;
                lcd_outdezAtt(8*FW, 5*FH, mspeed, 0);		// Speed
                
//								lcd_puts_Pleft( 7*FH, XPSTR("V4=\007V5=\016V6=")) ;
								{
									uint8_t x, y ;
									x = 6*FW ;
									y = 6*FH ;
									uint8_t totalCells = FrskyBattCells[0] + FrskyBattCells[1] ;
		  						for (uint8_t k=0 ; k<totalCells ; k++)
									{
										uint8_t blink=0;
										if ( k == 3 )
										{
											x = 6*FW ;
											y = 7*FH ;
										}
										if ((FrskyVolts[k] < g_model.frSkyVoltThreshold * 2))
										{
										  blink = BLINK ;
										}
  									lcd_outdezNAtt( x, y, FrskyVolts[k], blink | PREC2, 4 ) ;
										x += 7*FW ;
										if ( k == 5 )		// Max 6 cels displayable
										{
											break ;											
										}
	      					}
								}
								//              lcd_putsAtt(6, 2*FH, PSTR("To Be Done"), DBLSIZE);
              }
						}
            else if ( ( tview == 0 ) || ( tview == 0x10 ) )	// CUSTOM TELEMETRY DISPLAY
            {
							lcd_vline( 63, 8, 48 ) ;
							
#ifdef PCBX9D	
							ImageDisplay = 0 ;
							lcd_vline( 127, 8, 48 ) ;
		          for (uint32_t i=0; i<9; i++)
#else
    		      for (uint32_t i=0; i<6; i++)
#endif
							{
								uint32_t j ;
								j = tview == 0 ? g_model.customDisplayIndex[i] : g_model.customDisplay2Index[i] ;
#ifdef PCBX9D	
								if ( i >= 6 )
								{
									j = tview == 0 ? g_model.customDisplay1Extra[i-6] : g_model.customDisplay2Extra[i-6] ;
								}
#endif
								if ( j )
								{
									uint32_t index = j-1 ;
#ifdef PCBX9D	
									uint32_t x = (i&1)?65:1 ;
									uint32_t y = (i&0x0E)*FH+2*FH ;
									if ( i >= 6 )
									{
										x = 129 ;
										y = (i-6)*2*FH + 2*FH ;
									}
#else
									uint32_t x = (i&1)?65:1 ;
									uint32_t y = (i&0x0E)*FH+2*FH ;
#endif
									
//									lcd_puts_P( x, y, XPSTR(":  :"));
//									lcd_outdezNAtt( 18*FW-2, y, Time.second, LEADING0, 2 ) ;
//									lcd_outdezNAtt( 15*FW-1, y, Time.minute, LEADING0, 2 ) ;

									uint8_t style = TELEM_LABEL|TELEM_UNIT|TELEM_UNIT_LEFT|TELEM_VALUE_RIGHT ;
	                if ( index == TEL_ITEM_T1 )
									{
										if ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) )
										{
											style = TELEM_ARDUX_NAME ;
										}	
									}
									putsTelemetryChannel( x, y, index, get_telemetry_value(index),
															 DBLSIZE|CONDENSED, style ) ;
								}
							}

//                y0 = 6*FH;
//                //lcd_puts_P(2*FW-3, y0, PSTR("Tele:"));
//                x0 = 4*FW-3;
//                for (int i=0; i<2; i++) {
//                    if (g_model.frsky.channels[i].ratio) {
//                        blink = (alarmRaised[i] ? INVERS+BLINK : 0)|LEFT;
//												putsTelemetryChannel( x0, y0, TEL_ITEM_A1+i, frskyTelemetry[i].value, blink, TELEM_LABEL|TELEM_UNIT ) ;
////                        lcd_puts_P(x0, y0, PSTR(STR_A_EQ) ) ;
////                        lcd_putc(x0+FW, y0, '1'+i);
////                        putsTelemValue( x0+3*FW, y0, frskyTelemetry[i].value, i,  blink ) ;
//                        x0 = 13*FW-3;
//                    }
//                }
                y0 = 7*FH;
                //lcd_puts_P(2*FW-3, y0, PSTR("RSSI:"));
//                lcd_puts_P(4*FW-3, y0, PSTR(STR_RXEQ) );
//                lcd_outdezAtt(7*FW-3, y0, FrskyHubData[FR_RXRSI_COPY], LEFT);
//                lcd_puts_P(13*FW-3, y0, PSTR(STR_TXEQ) );
//                lcd_outdezAtt(16*FW-3, y0, FrskyHubData[FR_TXRSI_COPY], LEFT);
                
								lcd_puts_Pleft( y0, PSTR(STR_RXEQ) ) ;
//								lcd_putsn_P( 0, y0, PSTR(STR_RXEQ), 2 );
								lcd_hbar( 20, 57, 43, 6, FrskyHubData[FR_RXRSI_COPY] ) ;
								putTxSwr( 110, y0, 0 ) ;
//                lcd_putsn_P( 116, y0, PSTR(STR_TXEQ), 2 );
								lcd_hbar( 65, 57, 43, 6, FrskyHubData[FR_TXRSI_COPY] ) ;

            }
            else// if ( tview == 0x50 )
						{
						 if ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) )
						 {
								// Mavlink over FrSky
								uint8_t attr = 0 ;
								lcd_putsAtt(  0 * FW, 0 * FH, XPSTR("          "), 0 ); // clear inversed model name from screen
								// line 0, left - Battary remaining
								uint8_t batt =  FrskyHubData[FR_FUEL] ;
								if ( batt < 20 ) { attr = BLINK ; } else {attr = 0 ;}
								lcd_hbar( 01, 1, 10, 5, batt ) ;
								lcd_rect( 11, 2, 2, 3 ) ;
								if (batt > 99) batt = 99 ;
								lcd_outdezAtt ( 4*FW,   0*FH, batt, attr ) ;
								lcd_putcAtt	  ( 4*FW+1, 0*FH, '%',  attr ) ;
								// line 0, V, volt bat
 				        attr = PREC1 ;
 				        lcd_outdezAtt( 9 * FW+2, 0, FrskyHubData[FR_VOLTS], attr ) ;
								lcd_putc   ( 9 * FW+3, 0, 'v'); //"v"
								// line 0, V, volt cpu
// 				        lcd_outdezAtt( 13 * FW-2, 0, FrskyHubData[FR_VCC], PREC1 ) ;
//		    				lcd_putc     ( 13 * FW+1, 0, 'v');
								// line 1 - Flight mode
 				        uint8_t blink = BLINK;
 				        if (frskyUsrStreaming) blink = 0 ;
//								lcd_putsAtt( 15*FW-1, 1*FH, XPSTR("T1="), 0 ) ;
// 				        lcd_outdezAtt( 20 * FW, 1*FH, FrskyHubData[FR_TEMP1], blink ) ;
								
#define STR_MAV_ARMED  		 "ARMED"
#define STR_MAV_DISARMED   "DISARM"
#define STR_MAV_NODATA 		 "NODATA"
								if (frskyUsrStreaming)
								{
									if ( FrskyHubData[FR_BASEMODE] & MAV_MODE_FLAG_SAFETY_ARMED )
									{
										lcd_putsAtt(  1 * FW, 1 * FH, XPSTR(STR_MAV_ARMED), blink) ;
									}
									else
									{
										lcd_putsAtt(  0 * FW, 1 * FH, XPSTR(STR_MAV_DISARMED), blink) ;
									}
								}
								else
								{
									lcd_putsAtt(  0 * FW, 1 * FH, XPSTR(STR_MAV_NODATA), BLINK) ; // NO DRV
								}

//							  lcd_outdez( 15 * FW, 3 * FH, FrskyHubData[FR_BASEMODE] ) ;

								const char *s ;
								s = arduFlightMode( FrskyHubData[FR_TEMP1] ) ;
								lcd_putsAtt( 15*FW-1, 1*FH, s, blink ) ;

//		 line 2 - "GPS fix" converted from TEMP2
								uint8_t gps_fix ;
								uint8_t gps_sat ;
								gps_sat = FrskyHubData[FR_TEMP2] / 10 ;
 				        gps_fix = FrskyHubData[FR_TEMP2] - gps_sat * 10 ;
#define STR_MAV_GPS_NO_GPS   "No GPS"
#define STR_MAV_GPS_NO_FIX	 "No Fix" // 1
#define STR_MAV_GPS_2DFIX	 "2D Fix" // 2
#define STR_MAV_GPS_3DFIX	 "3D Fix" // 3
								switch ( gps_fix )
								{
									case 1: 	lcd_putsAtt( 0 * FW, 2*FH, XPSTR(STR_MAV_GPS_NO_FIX), BLINK); break;
									case 2: 	lcd_puts_P ( 0 * FW, 2*FH, XPSTR(STR_MAV_GPS_2DFIX )); break;
									case 3:
									case 4: 	lcd_puts_P ( 0 * FW, 2*FH, XPSTR(STR_MAV_GPS_3DFIX )); break;
									default : lcd_putsAtt( 0 * FW, 2*FH, XPSTR(STR_MAV_GPS_NO_GPS), INVERS); break;
								}
//		line 3 left "SAT"
#define STR_MAV_GPS_SAT_COUNT "sat"
								lcd_puts_P( 0 * FW,   3*FH, XPSTR(STR_MAV_GPS_SAT_COUNT) ); // sat
								lcd_outdez( 7 * FW-2, 3*FH, gps_sat ) ;

//		 line 4 left "HDOP"
                int16_t val;
								val = FrskyHubData[FR_GPS_HDOP];
								if (val <= 200)
								{
                	attr  = PREC2;
				   				blink = 0;
				 				}
								else
								{
                  attr = PREC2 | BLINK;
				   				blink = BLINK;
								}
#define STR_MAV_GPS_HDOP     "hdop"
								lcd_putsAtt( 0 * FW,   4*FH, XPSTR(STR_MAV_GPS_HDOP), blink ) ; 
                lcd_outdezAtt( 7 * FW-2, 4*FH, val, attr ) ; // hdop
 				        
								if (frskyUsrStreaming) blink = 0 ;

//		 line 5 left  "heartbeat"
								lcd_hbar( 0, 5*FH+1, 40, 4, frskyUsrStreaming ) ; // heartbeat
//		 line 2 right "Baro Alt"
#define STR_MAV_ALT          "alt"
								lcd_puts_P( 15 * FW-2, 2*FH, XPSTR(STR_MAV_ALT) ) ; // Alt
								val = get_telemetry_value(TEL_ITEM_BALT) ;
								attr = 0 ;
								if ( g_model.FrSkyImperial )
								{
      					 	val = m_to_ft( val ) ;
								}
                if ( val < 1000 )
								{
                  attr |= PREC1 ;
                }
								else
								{
                  val /= 10 ;
                }
                lcd_outdezAtt( 21 * FW+1, 2*FH, val, attr ) ;

//		 line 3 right "GPS Alt"
#define STR_MAV_GALT         "gAl"
								lcd_puts_P( 15 * FW-2, 3*FH, XPSTR(STR_MAV_GALT) ); // GAlt
								val = get_telemetry_value(TEL_ITEM_GALT) ;
								if ( g_model.FrSkyImperial )
								{
      					 	val = m_to_ft( val ) ;
								}
								lcd_outdezAtt( 21 * FW+1, 3*FH, val, PREC1 ) ;
//		 line 4 right "Home Distance"
#define STR_MAV_HOME         "dth"
								lcd_puts_P( 15 * FW-2, 4*FH, XPSTR(STR_MAV_HOME) ); // home
								lcd_outdez( 21 * FW+1, 4*FH, FrskyHubData[FR_HOME_DIST] ) ;
//		 line 5 right "Current, Milliampers"
//								val = FrskyHubData[FR_CURRENT] ;
//								attr = PREC1 ;
//								if ( val > 99 )
//								{
//									val /= 10 ;
//									attr = 0 ;
//								}
#define STR_MAV_CURRENT          "Cur"
								lcd_puts_P( 15 * FW-2, 5*FH, XPSTR(STR_MAV_CURRENT) ); // "Cur"
//								lcd_outdez ( 20 * FW+1, 5*FH,  FrskyHubData[FR_CURRENT])/10;
		        		lcd_putcAtt( 20 * FW+3, 5*FH, 'A', CONDENSED );
                lcd_outdezAtt( 20 * FW+3, 5*FH, FrskyHubData[FR_CURRENT], PREC1 ) ;
//		 line 6 left "Rssi"
								lcd_puts_P( 0 * FW, 6*FH, XPSTR("Rssi") ); // "Rssi"
								uint8_t rx = FrskyHubData[FR_RXRSI_COPY];
								if (rx > 100) rx = 100;
                lcd_outdezAtt( 7 * FW-3, 6*FH, rx, blink);
//         line 6 right  "Rcq"
                lcd_puts_P( 15*FW-2, 6*FH, XPSTR("Rcq") ); // "Rcq"
								uint8_t tx = FrskyHubData[FR_TXRSI_COPY];
								if (tx > 100) tx = 100;
                lcd_outdezAtt( 21*FW+1, 6*FH, tx, blink);

//		 line 7 
                lcd_puts_P( 0 * FW, 7*FH, XPSTR("Rx") );
								lcd_hbar( 14, 57, 49, 6, FrskyHubData[FR_RXRSI_COPY] ) ;
                lcd_puts_P( 19 * FW+1, 7*FH, XPSTR("Tx") );
								lcd_hbar( 65, 57, 49, 6, FrskyHubData[FR_TXRSI_COPY] ) ;

//				 THROTTLE bar, %
//								lcd_vbar( 0, 1 * FH, 4, 6 * FH - 1, FrskyHubData[FR_RPM] ) ;

								
// Mavlink - home direction and distance on graphic?								
								uint8_t x0 = 64 ;
								uint8_t y0 = 31 ;
								uint8_t r  = 22 ;
								uint8_t x ;
								uint8_t y ;
								DO_SQUARE( x0, y0, r * 2 + 1 ) ;
								int16_t hdg = FrskyHubData[FR_COURSE] ;
								if  ( FrskyHubData[FR_BASEMODE] & MAV_MODE_FLAG_SAFETY_ARMED ) // armed
								{   // we save head position before arming
									if ( hdg_home == 0 ) hdg_home = hdg;
								} else {
									hdg_home = 0 ;
								}
								hdg = hdg - hdg_home + 270 ; // use SIMPLE mode 
//								lcd_outdez( 12 * FW, 3 * FH, hdg ) ;
								for (int32_t i = -3 ; i < r ; i += 1 )
								{
//#if defined(CPUM128) || defined(CPUM2561)
//						x = x0 + i * cos ( hdg * PI / 180.0 );
//						y = y0 + i * sin ( hdg * PI / 180.0 );
//#else
									x = x0 + ( i * rxcos100 ( hdg ) ) / 100; 
									y = y0 + ( i * rxsin100 ( hdg ) ) / 100;
//#endif
									lcd_plot(x, y);
								}




// 				        lcd_outdezAtt( 5*FW , 7*FH, FrskyHubData[FR_COURSE], blink ) ;

						 	
// Extra data for Mavlink via FrSky 
//            else if ( tview == 0x40 ) // Mavlink via FrSky data
//            {
//				uint8_t attr = 0;
//				lcd_putsAtt(  0 * FW, 0 * FH, PSTR("          "), 0 ); // clear inversed model name from screen

//		 line 0, left - Battary remaining
//				uint8_t batt =  FrskyHubData[FR_FUEL];
//				if ( batt < 20 ) { attr = BLINK; } else {attr = 0;}
//				lcd_hbar( 01, 1, 10, 5, batt ) ;
//				lcd_rect( 11, 2, 2, 3 ) ;
//				if (batt > 99) batt = 99;
//				lcd_outdezAtt ( 4*FW,   0*FH, batt, attr );
//				lcd_putcAtt	  ( 4*FW+1, 0*FH, '%',  attr );
//		 line 0, V, volt bat 
//                attr = PREC1 ;
//                lcd_outdezAtt( 9 * FW+2, 0, FrskyHubData[FR_VOLTS], attr ) ;
//				lcd_putc   ( 9 * FW+3, 0, 'v'); //"v"
//		 line 0, V, volt cpu             
//                lcd_outdezAtt( 11 * FW-2, 0, FrskyHubData[FR_VCC], PREC1 ) ;
//		        lcd_putc     ( 11 * FW+1, 0, 'v');
//		 line 1 - Flight mode
//                uint8_t blink = BLINK;
//                if (frskyUsrStreaming) blink = 0 ;
//				switch ( FrskyHubData[FR_TEMP1] )
//				{
//					case  0 : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_0),  blink );	break;
//					case  1 : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_1),  blink );	break;
//					case  2 : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_2),  blink );	break;
//					case  3 : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_3),  blink );	break;
//					case  4 : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_4),  blink );	break;
//					case  5 : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_5),  blink );	break;
//					case  6 : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_6),  blink );	break;
//					case  7 : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_7),  blink );	break;
//					case  8 : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_8),  blink );	break;
//					case  9 : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_9),  blink );	break;
//					default : lcd_putsAtt( 15*FW-1, 1*FH, PSTR(STR_MAV_FM_0), blink );	break;
//				}
//		 line 1 - "Armed/Disarmed"
//				if (frskyUsrStreaming) {
//				if ( FrskyHubData[FR_BASEMODE] & MAV_MODE_FLAG_SAFETY_ARMED ) {
//					lcd_putsAtt(  1 * FW, 1 * FH, PSTR(STR_MAV_ARMED), blink) ;
//				} else {
//					lcd_putsAtt(  0 * FW, 1 * FH, PSTR(STR_MAV_DISARMED), blink) ;
//				}
//				} else {
//					lcd_putsAtt(  0 * FW, 1 * FH, PSTR(STR_MAV_NODATA), BLINK) ; // NO DRV
//				}
//				 lcd_outdez( 15 * FW, 3 * FH, FrskyHubData[FR_BASEMODE] ) ;

//		 line 2 - "GPS fix" converted from TEMP2
//				uint8_t gps_fix;
//				uint8_t gps_sat;
//				gps_fix = FrskyHubData[FR_TEMP2] / 10;
//                gps_sat = FrskyHubData[FR_TEMP2] - gps_fix * 10;
//				switch ( gps_fix )
//				{
//					case 1: 	lcd_putsAtt( 0 * FW, 2*FH, PSTR(STR_MAV_GPS_NO_FIX), BLINK); break;
//					case 2: 	lcd_puts_P ( 0 * FW, 2*FH, PSTR(STR_MAV_GPS_2DFIX )); break;
//					case 3: 	lcd_puts_P ( 0 * FW, 2*FH, PSTR(STR_MAV_GPS_3DFIX )); break;
//					case 4: 	lcd_puts_P ( 0 * FW, 2*FH, PSTR(STR_MAV_GPS_3DFIX )); break;
					
//					default :  	lcd_putsAtt( 0 * FW, 2*FH, PSTR(STR_MAV_GPS_NO_GPS), INVERS); break;
//				}
//		 line 3 left "SAT"
//				lcd_puts_P( 0 * FW,   3*FH, PSTR(STR_MAV_GPS_SAT_COUNT) ); // sat
//				lcd_outdez( 7 * FW-2, 3*FH, gps_sat ) ;


//		 line 4 left "HDOP"
//                int16_t val;
//				val = FrskyHubData[FR_GPS_HDOP];
//				if (val <= 200) {
//                   attr  = PREC1;
//				   blink = 0;
//				 } else {
//                   attr = PREC1 | BLINK;
//				   blink = BLINK;
//				 }
//				lcd_putsAtt( 0 * FW,   4*FH, PSTR(STR_MAV_GPS_HDOP), blink ); 
//                lcd_outdezAtt( 7 * FW-2, 4*FH, val, attr ); // hdop
//		 line 5 left  "heartbeat"
//				lcd_hbar( 0, 5*FH+1, 40, 4, frskyUsrStreaming ) ; // heartbeat
//		 line 2 right "Baro Alt"
//				lcd_puts_P( 15 * FW-1, 2*FH, PSTR(STR_MAV_ALT) ); // Alt
//				val = FrskyHubData[FR_ALT_BARO];
//				attr = 0;
//                if ( val < 1000 ) {
//                    attr |= PREC1 ;
//                } else {
//                    val /= 10 ;
//                }
//                lcd_outdezAtt( 21 * FW+1, 2*FH, val, attr );
//		 line 3 right "GPS Alt"
//				lcd_puts_P( 15 * FW-1, 3*FH, PSTR(STR_MAV_GALT) ); // GAlt
//				lcd_outdez( 21 * FW+1, 3*FH, getTelemetryValue(FR_GPS_ALT) ) ;
//		 line 4 right "Home Distance"
//				lcd_puts_P( 15 * FW-1, 4*FH, PSTR(STR_MAV_HOME) ); // home
//				lcd_outdez( 21 * FW+1, 4*FH, FrskyHubData[FR_HOME_DIST] ) ;
//		 line 5 right "Current, Milliampers"
//                lcd_outdezAtt( 20 * FW+1, 5*FH, FrskyHubData[FR_CURRENT], PREC1 ) ;
//				lcd_puts_P( 15 * FW-1, 5*FH, PSTR(STR_MAV_CURRENT) ); // "Cur"
//				lcd_outdez ( 20 * FW+1, 5*FH,  FrskyHubData[FR_CURRENT])/10;
//		        lcd_putc   ( 20 * FW+2, 5*FH, 'A');
//		 line 6 left "Rssi"
//				lcd_puts_P( 0 * FW, 6*FH, PSTR(STR_RSSI) ); // "Rssi"
//				uint8_t rx = FrskyHubData[FR_RXRSI_COPY];
//				if (rx > 100) rx = 100;
//                lcd_outdezAtt( 7 * FW-3, 6*FH, rx, blink);
//         line 6 right  "Rcq"
//                lcd_puts_P( 15*FW-1, 6*FH, PSTR(STR_RCQ) ); // "Rcq"
//				uint8_t tx = FrskyHubData[FR_TXRSI_COPY];
//				if (tx > 100) tx = 100;
//                lcd_outdezAtt( 21*FW+1, 6*FH, tx, blink);
				
//		 line 7 
//                lcd_puts_P( 0 * FW, 7*FH, PSTR(STR_RX) );
//								lcd_hbar( 14, 57, 49, 6, FrskyHubData[FR_RXRSI_COPY] ) ;
//                lcd_puts_P( 19 * FW+1, 7*FH, PSTR(STR_TX) );
//								lcd_hbar( 65, 57, 49, 6, FrskyHubData[FR_TXRSI_COPY] ) ;
//				 THROTTLE bar, %
//				lcd_vbar( 0, 1 * FH, 4, 6 * FH - 1, FrskyHubData[FR_RPM] ) ;
//				uint8_t x0 = 64;
//				uint8_t y0 = 31;
//				uint8_t r  = 23;
//				uint8_t x;
//				uint8_t y;
//				DO_SQUARE( x0, y0, r * 2 + 1 );
//				int16_t hdg = FrskyHubData[FR_COURSE];
//				if  ( FrskyHubData[FR_BASEMODE] & MAV_MODE_FLAG_SAFETY_ARMED ) // armed
//				{   // we save head position before arming
//					if ( hdg_home == 0 ) hdg_home = hdg;
//				} else {
//					hdg_home = 0 ;
//				}
//				hdg = hdg - hdg_home + 270 ; // use SIMPLE mode 
//				lcd_outdez( 15 * FW, 3 * FH, hdg ) ;
//				for (int8_t i = -3; i < r; i++)
//					{
//#if defined(CPUM128) || defined(CPUM2561)
//						x = x0 + i * cos ( hdg * PI / 180.0 );
//						y = y0 + i * sin ( hdg * PI / 180.0 );
//#else
//						x = x0 + ( i * rcos100 ( hdg ) ) / 100; 
//						y = y0 + ( i * rsin100 ( hdg ) ) / 100;
//#endif
//						lcd_plot(x, y);
//					}
//				 dir to wp
// Extra data for Mavlink via FrSky 



						 }
						 else
						 {
							// Spektrum format screen
							lcd_puts_Pleft( 0*FH, XPSTR("\013tssi")) ;
							lcd_puts_Pleft( 2*FH, XPSTR("Vbat")) ;
							lcd_puts_Pleft( 2*FH, XPSTR("\013RxV")) ;
							lcd_puts_Pleft( 4*FH, XPSTR("AMP\013Temp")) ;
							lcd_puts_Pleft( 6*FH, XPSTR("RPM\021DSM2")) ;
							
							lcd_vline( 63, 8, 32 ) ;

              lcd_outdezAtt( 17*FW-2, 0*FH, FrskyHubData[FR_RXRSI_COPY], 0 ) ;
              lcd_outdezAtt( 61, 1*FH, FrskyHubData[FR_VOLTS], PREC1|DBLSIZE ) ;
							lcd_outdezAtt( 125, 1*FH, convertRxv( FrskyHubData[FR_RXV] ), PREC1|DBLSIZE ) ;
							lcd_outdezAtt( 61, 3*FH, FrskyHubData[FR_CURRENT], PREC1|DBLSIZE ) ;
              lcd_outdezAtt( 125, 3*FH, FrskyHubData[FR_TEMP1], DBLSIZE ) ;
              lcd_outdezAtt( 14*FW, 5*FH, FrskyHubData[FR_RPM], DBLSIZE ) ;

							if ( ( g_model.protocol == PROTO_MULTI ) || ( g_model.xprotocol == PROTO_MULTI ) )
							{
								if ( ( g_model.sub_protocol & 0x3F ) == M_DSM )
								{
									if ( (g_model.ppmNCH & 0x60 )== 0x20)
									{
										lcd_putc( 20*FW, 6*FH, 'X' ) ;
									}
								}
								if ( ( g_model.xsub_protocol & 0x3F ) == M_DSM )
								{
									if ( (g_model.xppmNCH & 0x60 ) == 0x20)
									{
										lcd_putc( 20*FW, 6*FH, 'X' ) ;
									}
								}
							}
							else if ( g_model.dsmMode & ORTX_USE_DSMX )
							{
								lcd_putc( 20*FW, 6*FH, 'X' ) ;
							}
							dsmDisplayABLRFH() ;
						 }
						}
//            else	// ( tview == 0x50 )
//						{
//							// second custom screen
							
//                lcd_puts_P(4*FW-3, y0, PSTR(STR_RXEQ) );
//                lcd_outdezAtt(7*FW-3, y0, FrskyHubData[FR_RXRSI_COPY], LEFT);
//                lcd_puts_P(13*FW-3, y0, PSTR(STR_TXEQ) );
//                lcd_outdezAtt(16*FW-3, y0, FrskyHubData[FR_TXRSI_COPY], LEFT);
//						}

        }
//        else {
//            lcd_putsAtt(22, 5*FH, PSTR("NO DATA"), DBLSIZE);
//        }
    }
#endif
  else if(view<e_timer2)
	{
		doMainScreenGrphics() ;

    int8_t a = io_subview ;
#if defined(PCBSKY) || defined(PCB9XT)
		if ( a != 0 ) a = a * 6 + 3 ;		// 0, 9, 15
#endif
#ifdef PCBX9D
		if ( a != 0 ) a = a * 6 ;		// 0, 6, 12
#endif
    uint8_t j ;
		for ( j = 0 ; j < 2 ; j += 1 )
		{
#if defined(PCBSKY) || defined(PCB9XT)
			if ( j == 1 )
			{
				a = io_subview ;
				a += 1 ;
				a *= 6 ;		// 6, 12, 18
			}
#endif
#ifdef PCBX9D
			if ( j == 1 )
			{
				a += 3 ;
			}
#endif
			switchDisplay( j, a ) ;
//			for(int8_t i=a; i<(a+3); i++)
//			{
//				uint8_t s = i ;
//				if ( i == 8 )
//				{
//					s = 3 ;
//					if ( ! getSwitch( s+1, 0 ) )		// ID0 ?
//					{
//						s = 4 ;
//						if ( ! getSwitch( s+1, 0 ) )		// ID1 ?
//						{
//							s = 5 ;
//						}
//					}
//				}
//				lcd_putsnAtt((2+j*15)*FW-2 ,(i-a+4)*FH,get_switches_string()+3*s,3,getSwitch(i+1, 0) ? INVERS : 0);
//			}
		}
	}
  else  // New Timer2 display
  {
		displayTimer( 30+5*FW, FH*5, 1, DBLSIZE ) ;
    putsTmrMode( 30-2*FW-FW/2,FH*6, 0, 1, 0 ) ;
    putsTime( FW*17+3, 5*FH, Time.hour * 60 + Time.minute, 0, 0 ) ;
  }

	if ( PopupData.PopupActive )
	{
//		uint8_t count ;
		uint16_t mask = 0x043F ;

    if(PopupData.PopupActive == 1)
		{
			mask = 0x03C0 ;
		}
		
		uint8_t popaction = doPopup( PSTR( STR_MAIN_POPUP ), mask, 14, event ) ;
																 
		UseLastSubmenuIndex = 0 ;
  	if ( popaction == POPUP_SELECT )
		{
			uint8_t popidx = PopupData.PopupSel ;
			if ( popidx == 0 )	// Model Select
			{
        pushMenu(menuProcModelSelect) ;
			}
			else if( popidx == 1 )	// Edit Model
			{
				RotaryState = ROTARY_MENU_UD ;
	  	  pushMenu(menuProcModelIndex) ;
			}
			else if( popidx == 2 )	// Last Menu
			{
				UseLastSubmenuIndex = 1 ;
        pushMenu(lastPopMenu());
			}
			else if ( popidx == 3 )	// Radio Setup
			{
        pushMenu(menuProcIndex) ;
			}
			else if( popidx == 4 )	// Statistics
			{
	  	  pushMenu(menuProcBattery) ;
			}
			else if( popidx == 5 )	// Notes
			{
				TextHelp = 0 ;
	  	  pushMenu(menuProcText) ;
			}
			else if( popidx == 6 )	// Zero Alt.
			{
        AltOffset = -FrskyHubData[FR_ALT_BARO] ;
			}
			else if( popidx == 7 )	// A1 Offset
			{
        if ( g_model.frsky.channels[0].type == 3 )		// Current (A)
				{
				  frskyTelemetry[0].setoffset() ;
				}
			}
			else if( popidx == 8 )	// A2 Offset
			{
        if ( g_model.frsky.channels[1].type == 3 )		// Current (A)
				{
				  frskyTelemetry[1].setoffset() ;
				}
			}
			else if( popidx == 9 )	// GPS reset
			{
				struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;

				maxMinPtr->hubMax[FR_GPS_SPEED] = 0 ;
				maxMinPtr->hubMax[FR_GPS_ALT] = 0 ;
			}
			else if( popidx == 10 )	// Help
			{
				TextHelp = 1 ;
	  	  pushMenu(menuProcText) ;
			}
		}
//		else if ( popaction == POPUP_EXIT )
//		{
//			killEvents( event ) ;
//			PopupData.PopupActive = 0 ;
//		}
	}

}

static uint16_t isqrt32(uint32_t n)
{
  uint16_t c = 0x8000;
  uint16_t g = 0x8000;

  for(;;)
	{
    if((uint32_t)g*g > n)
      g ^= c;
    c >>= 1;
    if(c == 0)
      return g;
    g |= c;
  }
}

int16_t intpol(int16_t x, uint8_t idx) // -100, -75, -50, -25, 0 ,25 ,50, 75, 100
{
#define D9 (RESX * 2 / 8)
#define D5 (RESX * 2 / 4)
#define D6 (RESX * 2 / 5)
    uint32_t cv9 = idx >= MAX_CURVE5;
		int8_t *crv ;
		if ( idx == MAX_CURVE5 + MAX_CURVE9 )
		{ // The xy curve
			crv = g_model.curvexy ;
			cv9 = 2 ;
		}
		else if ( idx == MAX_CURVE5 + MAX_CURVE9 + 1)
		{ // The xy curve
			crv = g_model.curve2xy ;
			cv9 = 2 ;
		}
		else if ( idx == MAX_CURVE5 + MAX_CURVE9 + 2 )
		{
			crv = g_model.curve6 ;
			cv9 = 3 ;
		}
		else
		{
    	crv = cv9 ? g_model.curves9[idx-MAX_CURVE5] : g_model.curves5[idx];
		}
    int16_t erg;

    x+=RESXu;
    if(x < 0)
		{
      erg = (int16_t)crv[0] * (RESX/4);
    }
		else if(x >= (RESX*2))
		{
			if ( cv9 == 3 )
			{
      	erg = (int16_t)crv[5] * (RESX/4);
			}
			else
			{
      	erg = (int16_t)crv[(cv9 ? 8 : 4)] * (RESX/4);
			}
    }
		else
		{
			int16_t deltax ;
			div_t qr ;
			if ( cv9 == 2 ) // xy curve
			{
		    int16_t a = 0 ;
				int16_t b ;
				int16_t c ;
				uint32_t i ;

				// handle end points
  	    c = RESX + calc100toRESX(crv[17]) ;
				if ((uint16_t)x>c)
				{
					return calc100toRESX(crv[8]) ;
				}
  	    b = RESX + calc100toRESX(crv[9]) ;
				if ((uint16_t)x<b)
				{
					return calc100toRESX(crv[0]) ;
				}

				for ( i = 0 ; i < 8 ; i += 1 )
				{
	        a = b ;
  	      b = (i==7 ? c : RESX + calc100toRESX(crv[i+10]));
    	    if ((uint16_t)x<=b) break;
				}
				qr.quot = i ;
				qr.rem = x - a ;
				deltax = b - a ;
			}
			else
			{
        if(cv9 == 3)
				{
					qr = div( x, D6 ) ;
					deltax = D6 ;
				}
				else if ( cv9 )
				{
					qr = div( x, D9 ) ;
					deltax = D9 ;
        }
				else
				{
					qr = div( x, D5 ) ;
					deltax = D5 ;
        }
  	  }
			int32_t y1 = (int16_t)crv[qr.quot] * (RESX/4) ;
			int32_t deltay = (int16_t)crv[qr.quot+1] * (RESX/4) - y1 ;
			erg = y1 + ( qr.rem ) * deltay / deltax ;
//      erg = (int16_t)crv[qr.quot]*((D5-qr.rem)/2) + (int16_t)crv[qr.quot+1]*(qr.rem/2);
		}
    return erg / 25; // 100*D5/RESX;
}

int16_t calcExpo( uint8_t channel, int16_t value )
{
  uint8_t expoDrOn = get_dr_state(channel);
  uint8_t stkDir = value > 0 ? DR_RIGHT : DR_LEFT ;

  if(IS_THROTTLE(channel) && g_model.thrExpo)
	{
    value  = 2*expo((value+RESX)/2,REG100_100(g_model.expoData[channel].expo[expoDrOn][DR_EXPO][DR_RIGHT])) ;
    stkDir = DR_RIGHT ;
  }
  else
    value  = expo(value,REG100_100(g_model.expoData[channel].expo[expoDrOn][DR_EXPO][stkDir])) ;

  value = (int32_t)value * (REG(g_model.expoData[channel].expo[expoDrOn][DR_WEIGHT][stkDir]+100, 0, 100))/100 ;
  if (IS_THROTTLE(channel) && g_model.thrExpo) value -= RESX;
	return value ;
}

// static variables used in perOut - moved here so they don't interfere with the stack
// It's also easier to initialize them here.
#if GVARS
int16_t  anas [NUM_SKYXCHNRAW+1+MAX_GVARS+1] ;		// To allow for 3POS and THIS and 8 extra PPM inputs
#else
int16_t  anas [NUM_SKYXCHNRAW+1+1] ;		// To allow for 3POS
#endif
int32_t  chans[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS] = {0};
uint8_t inacPrescale ;
uint16_t InacCounter = 0;
uint16_t inacSum = 0;
uint16_t  bpanaCenter = 0;
uint16_t  sDelay[MAX_SKYMIXERS+EXTRA_SKYMIXERS] = {0};
int32_t  act   [MAX_SKYMIXERS+EXTRA_SKYMIXERS] = {0};
uint8_t  swOn  [MAX_SKYMIXERS+EXTRA_SKYMIXERS] = {0};
uint8_t	CurrentPhase = 0 ;
int16_t rawSticks[4] ;

struct t_fade
{
uint8_t  fadePhases ;
uint16_t fadeRate ;
uint16_t fadeWeight ;
uint16_t fadeScale[MAX_MODES+1] ;
int32_t  fade[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS];
} Fade ;

static void inactivityCheck()
{
  if(s_noHi) s_noHi--;
  uint16_t tsum = 0;
  for(uint8_t i=0;i<4;i++) tsum += anas[i];
  if(abs(int16_t(tsum-inacSum))>INACTIVITY_THRESHOLD)
	{
		inacSum = tsum;
		InactivityMonitor = 1 ;  // reset in perMain
		InacCounter = 0 ;
  }
  if( (g_eeGeneral.inactivityTimer + 10) && (g_vbat100mV>49))
	{
    if (++inacPrescale > 15 )
    {
      InacCounter += 1 ;
      inacPrescale = 0 ;
  	  if( InacCounter >((uint16_t)(g_eeGeneral.inactivityTimer+10)*(100*60/16)))
      if( ( InacCounter & 0x1F ) == 1 )
			{
				SystemOptions &= ~SYS_OPT_MUTE ;						
				putVoiceQueue( VOLUME_MASK + g_eeGeneral.inactivityVolume + ( NUM_VOL_LEVELS-3 ) ) ;
				voiceSystemNameNumberAudio( SV_INACTV, V_INACTIVE, AU_INACTIVITY ) ;
//				audioVoiceDefevent( AU_INACTIVITY, V_INACTIVE | VLOC_NUMSYS ) ;
				putVoiceQueue( 0xFFFF ) ;

      }
    }
  }
}

#ifdef PCBX9D
#ifdef REV9E
const uint8_t switchIndex[18] = { HSW_SA0, HSW_SB0, HSW_SC0, HSW_SD0, HSW_SE0, HSW_SF2, HSW_SG0, HSW_SH2, HSW_SI0, 
																 HSW_SJ0, HSW_SK0, HSW_SL0, HSW_SM0, HSW_SN0, HSW_SO0, HSW_SP0, HSW_SQ0, HSW_SR0, } ;
#else
const uint8_t switchIndex[8] = { HSW_SA0, HSW_SB0, HSW_SC0, HSW_SD0, HSW_SE0, HSW_SF2, HSW_SG0, HSW_SH2 } ;
#endif	// REV9E
#endif

void perOutPhase( int16_t *chanOut, uint8_t att ) 
{
	static uint8_t lastPhase ;
	uint8_t thisPhase ;
	struct t_fade *pFade ;
	pFade = &Fade ;
	
	thisPhase = getFlightPhase() ;
	if ( thisPhase != lastPhase )
	{
		uint8_t time1 = 0 ;
		uint8_t time2 ;
		
		if ( lastPhase )
		{
      time1 = g_model.phaseData[(uint8_t)(lastPhase-1)].fadeOut ;
		}
		if ( thisPhase )
		{
      time2= g_model.phaseData[(uint8_t)(thisPhase-1)].fadeIn ;
			if ( time2 > time1 )
			{
        time1 = time2 ;
			}
		}
		if ( time1 )
		{
			pFade->fadeRate = (25600 / 50) / time1 ;
			pFade->fadePhases |= ( 1 << lastPhase ) | ( 1 << thisPhase ) ;
		}
		lastPhase = thisPhase ;
	}
	att |= FADE_FIRST ;
	if ( pFade->fadePhases )
	{
		pFade->fadeWeight = 0 ;
		uint8_t fadeMask = 1 ;
    for (uint8_t p=0; p<MAX_MODES+1; p++)
		{
			if ( pFade->fadePhases & fadeMask )
			{
				if ( p != thisPhase )
				{
					CurrentPhase = p ;
					pFade->fadeWeight += pFade->fadeScale[p] ;
					perOut( chanOut, att ) ;
					att &= ~FADE_FIRST ;				
				}
			}
			fadeMask <<= 1 ;
		}	
	}
	else
	{
		pFade->fadeScale[thisPhase] = 25600 ;
	}
	pFade->fadeWeight += pFade->fadeScale[thisPhase] ;
	CurrentPhase = thisPhase ;
	perOut( chanOut, att | FADE_LAST ) ;
	
	if ( pFade->fadePhases && tick10ms )
	{
		uint8_t fadeMask = 1 ;
    for (uint8_t p=0; p<MAX_MODES+1; p+=1)
		{
			uint16_t l_fadeScale = pFade->fadeScale[p] ;
			
			if ( pFade->fadePhases & fadeMask )
			{
				uint16_t x = pFade->fadeRate * tick10ms ;
				if ( p != thisPhase )
				{
          if ( l_fadeScale > x )
					{
						l_fadeScale -= x ;
					}
					else
					{
						l_fadeScale = 0 ;
						pFade->fadePhases &= ~fadeMask ;						
					}
				}
				else
				{
          if ( 25600 - l_fadeScale > x )
					{
						l_fadeScale += x ;
					}
					else
					{
						l_fadeScale = 25600 ;
						pFade->fadePhases &= ~fadeMask ;						
					}
				}
			}
			else
			{
				l_fadeScale = 0 ;
			}
			pFade->fadeScale[p] = l_fadeScale ;
			fadeMask <<= 1 ;
		}
	}
}

int16_t scaleAnalog( int16_t v, uint8_t channel )
{
	int16_t mid ;
	int16_t neg ;
	int16_t pos ;
	int16_t deadband = ( channel < 4 ) ? g_eeGeneral.stickDeadband[channel] : 0 ;

#ifndef SIMU
	mid = *CalibMid[channel] ;
  pos = *CalibSpanPos[channel] - deadband ;
  neg = *CalibSpanNeg[channel] - deadband ;

// #ifdef PCBX9D
//	if ( channel == 7 )
//	{
//	}
// #ifdef REVPLUS
//  #ifdef REV9E
	
//	else if ( channel == 11 )
//	{
//		mid = g_eeGeneral.x9dPcalibMid ;
//		pos = g_eeGeneral.x9dPcalibSpanPos ;
//		neg = g_eeGeneral.x9dPcalibSpanNeg ;
//	}
//	else if ( channel >= 8 )
//	{
//		v = anaIn(channel+2);
//		mid = 1024 ;
//		pos = 1024 ;
//		neg = 1024 ;
//	}
//  #else						
//	else if ( channel == 8 )
//	{
//		mid = g_eeGeneral.x9dPcalibMid ;
//		pos = g_eeGeneral.x9dPcalibSpanPos ;
//		neg = g_eeGeneral.x9dPcalibSpanNeg ;
//	}
//  #endif	// REV9E
// #endif	// REVPLUS
//	else
//	{
// #endif // PCBX9D
//		mid = g_eeGeneral.calibMid[channel] ;
//		pos = g_eeGeneral.calibSpanPos[channel]  ;
//		neg = g_eeGeneral.calibSpanNeg[channel]  ;
// #ifdef PCBX9D
//	}	
// #endif // PCBX9D
	
	v -= mid ;

	if ( channel < 4 )
	{
		if ( ( v > -deadband) && ( v < deadband ) )
		{
			v = 0 ;
		}
		else
		{
			if ( v > 0 )
			{
				v -= deadband ;
			}
			else
			{
				v += deadband ;
			}
		}
	}

//#ifdef REVX
//	if ( channel < 4 )
//	{
//		if ( ( v > -1) && ( v < 1 ) )
//		{
//			v = 0 ;
//		}
//	}
//#endif
	v  =  v * (int32_t)RESX /  (max((int16_t)100,(v>0 ? pos : neg ) ) ) ;
	
#endif // SIMU
	if(v <= -RESX) v = -RESX;
	if(v >=  RESX) v =  RESX;
	if ( throttleReversed() )
	{
		if ( channel == THR_STICK )
		{
			v = -v ;
		}
	}
	return v ;
}

void perOut(int16_t *chanOut, uint8_t att )
{
    int16_t  trimA[4];
    uint16_t  anaCenter = 0;
//    uint16_t d = 0;
		int16_t trainerThrottleValue = 0 ;
		uint8_t trainerThrottleValid = 0 ;
		if ( check_soft_power() == POWER_TRAINER )		// On trainer power
		{
			TrainerChannel *tChan = &g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[0] ;
			
#ifdef PCBX9D
			if ( ( tChan->source != TRAINER_SBUS ) && ( tChan->source != TRAINER_CPPM ) )
#else
			if ( ( tChan->source != TRAINER_BT ) && ( tChan->source != TRAINER_COM2 ) )
#endif
			{
				att |= NO_TRAINER ;
			}
		}
    {
        uint8_t ele_stick, ail_stick ;
        ele_stick = 1 ; //ELE_STICK ;
        ail_stick = 3 ; //AIL_STICK ;
        //===========Swash Ring================
//        if(g_model.swashRingValue)
//        {
//            uint32_t v = (int32_t(calibratedStick[ele_stick])*calibratedStick[ele_stick] +
//                          int32_t(calibratedStick[ail_stick])*calibratedStick[ail_stick]);
//						uint32_t q = calc100toRESX(g_model.swashRingValue) ;
//            q *= q;
//            if(v>q)
//                d = isqrt32(v);
//        }
        //===========Swash Ring================

//#ifdef FIX_MODE
				uint8_t stickIndex = g_eeGeneral.stickMode*4 ;
//#endif        
        for( uint8_t i = 0 ; i < 7+NumExtraPots ; i += 1 ) // calc Sticks
				{
            //Normalization  [0..2048] ->   [-1024..1024]
					int16_t v = anaIn( i ) ;
//#ifdef PCB9XT
//					int16_t v = anaIn( (i<7) ? i : i+1 ) ;
//#else
//#ifdef REVPLUS
//					int16_t v = anaIn( (i<8) ? i : i+1 ) ;
//#else
//					int16_t v = anaIn( (i<7) ? i : i+1 ) ;
////					int16_t v = anaIn( i ) ;
//#endif
//#endif
					v = scaleAnalog( v, i ) ;

//#ifdef FIX_MODE
						uint8_t index = i ;
						if ( i < 4 )
						{
            	phyStick[i] = v >> 4 ;
							index = stickScramble[stickIndex+i] ;
						}
//#else
//						uint8_t index = i ;
//#endif
            calibratedStick[index] = v; //for show in expo

						// Filter beep centre
						{
							int8_t t = v/16 ;
//#ifdef FIX_MODE
							uint16_t mask = 1 << index ;
//#else
//							uint16_t mask = 1<<(CONVERT_MODE((i+1))-1) ;
//#endif
							if ( t < 0 )
							{
								t = -t ;		//abs(t)
							}
							if ( t <= 1 )
							{
            		anaCenter |= ( t == 0 ) ? mask : bpanaCenter & mask ;
							}
						}

            if(i<4) { //only do this for sticks
                //===========Trainer mode================
                if (!(att&NO_TRAINER) && g_model.traineron)
								{
									TrainerChannel *tChan = &g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[index] ;
                  if (tChan->mode && getSwitch00(tChan->swtch))
									{
										if ( ppmInValid )
										{
                      uint8_t chStud = tChan->srcChn ;
                      int32_t vStud  = (g_ppmIns[chStud] - tChan->calib) /* *2 */ ;
                      vStud *= tChan->studWeight ;
                      vStud /= 50 ;
                      switch ((uint8_t)tChan->mode)
											{
                        case 1: v += vStud;   break; // add-mode
                        case 2: v  = vStud;   break; // subst-mode
                      }
											if ( index == 2 )
											{
												trainerThrottleValue = vStud ;
												trainerThrottleValid = 1 ;
											}												 
										}
									}
									
//                    TrainerMix* td = &g_eeGeneral.trainer.mix[index];
//										exTrainerMix *xtd = &g_eeGeneral.exTrainer[index] ;
//                    if (td->mode && getSwitch00(xtd->swtch))
//										{
//											if ( ppmInValid )
//											{
//                        uint8_t chStud = td->srcChn ;
//                        int32_t vStud  = (g_ppmIns[chStud]- g_eeGeneral.trainer.calib[chStud]) /* *2 */ ;
////                        vStud /= 2 ;		// Only 2, because no *2 above
//                        vStud *= xtd->studWeight ;
//                        vStud /= 50 ;
////                        vStud /= 31 ;
////                        vStud *= 4 ;
//                        switch ((uint8_t)td->mode) {
//                        case 1: v += vStud;   break; // add-mode
//                        case 2: v  = vStud;   break; // subst-mode
//                        }
////#ifdef FIX_MODE
//												if ( index == 2 )
////#else
////												if ( index == THR_STICK )
////#endif
//												{
//													trainerThrottleValue = vStud ;
//													trainerThrottleValid = 1 ;
//												}												 
//											}
//                    }
                }

                //===========Swash Ring================
//                if(d && (index==ele_stick || index==ail_stick))
//                    v = int32_t(v)*calc100toRESX(g_model.swashRingValue)/int32_t(d) ;
                //===========Swash Ring================

								if ( att & FADE_FIRST )
								{
//#ifdef FIX_MODE
    		        	rawSticks[index] = v; //set values for mixer
//#else
//      		      	rawSticks[i] = v; //set values for mixer
//#endif
								}
								v = calcExpo( index, v ) ;
//                uint8_t expoDrOn = get_dr_state(index);
//                uint8_t stkDir = v>0 ? DR_RIGHT : DR_LEFT;

//                if(IS_THROTTLE(index) && g_model.thrExpo){
//                    v  = 2*expo((v+RESX)/2,REG(g_model.expoData[index].expo[expoDrOn][DR_EXPO][DR_RIGHT], -100, 100));
//                    stkDir = DR_RIGHT;
//                }
//                else
//                    v  = expo(v,REG(g_model.expoData[index].expo[expoDrOn][DR_EXPO][stkDir], -100, 100));

//                int32_t x = (int32_t)v * (REG(g_model.expoData[index].expo[expoDrOn][DR_WEIGHT][stkDir]+100, 0, 100))/100;
//                v = (int16_t)x;
//                if (IS_THROTTLE(index) && g_model.thrExpo) v -= RESX;

                trimA[i] = getTrimValue( CurrentPhase, i )*2 ; //    if throttle trim -> trim low end
            }
						if ( att & FADE_FIRST )
						{
							if ( i < 7 )
							{
//#ifdef FIX_MODE
        	    	anas[index] = v ; //set values for mixer
//#else
//	          	  anas[i] = v ; //set values for mixer
//#endif
							}
						}
			    	if(att&NO_INPUT)
						{ //zero input for setStickCenter()
		      	  if ( i < 4 )
							{
    	    	    if(!IS_THROTTLE(index))
								{
									if ( ( v > (RESX/100 ) ) || ( v < -(RESX/100) ) )
									{
//#ifdef FIX_MODE
				            anas[index] = 0; //set values for mixer
//#else
//          	      	anas[i]  = 0;
//#endif
									}
          	      trimA[index] = 0;
//          	      trimA[i] = 0;
      	  	    }
        				anas[i+PPM_BASE] = 0;
        			}
    				}
        }
				//    if throttle trim -> trim low end
        if(g_model.thrTrim)
				{
					int8_t ttrim ;
//#ifdef PHASES		
					ttrim = getTrimValue( CurrentPhase, 2 ) ;
//#else
//					ttrim = *TrimPtr[2] ;
//#endif
					if(throttleReversed())
					{
						ttrim = -ttrim ;
					}
         	trimA[2] = ((int32_t)ttrim+125)*(RESX-anas[2])/(RESX) ;
				}
			if ( att & FADE_FIRST )
			{

        //===========BEEP CENTER================
        anaCenter &= g_model.beepANACenter;
        if(((bpanaCenter ^ anaCenter) & anaCenter)) audioDefevent(AU_POT_STICK_MIDDLE);
        bpanaCenter = anaCenter;

				// Set up anas[] array
        anas[MIX_MAX-1]  = RESX;     // MAX
        anas[MIX_FULL-1] = RESX;     // FULL
#if defined(PCBSKY) || defined(PCB9XT)
        anas[MIX_3POS-1] = keyState(SW_ID0) ? -1024 : (keyState(SW_ID1) ? 0 : 1024) ;
#endif
#ifdef PCBX9D
        anas[MIX_3POS-1] = keyState(SW_SC0) ? -1024 : (keyState(SW_SC1) ? 0 : 1024) ;
#endif
        for(uint8_t i=0;i<4;i++) anas[i+PPM_BASE] = (g_ppmIns[i] - g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[i].calib)*2; //add ppm channels
        for(uint8_t i=4;i<NUM_PPM;i++)    anas[i+PPM_BASE]   = g_ppmIns[i]*2; //add ppm channels
        for(uint8_t i=0;i<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS;i++) anas[i+CHOUT_BASE] = chans[i]; //other mixes previous outputs
#if GVARS
        for(uint8_t i=0;i<MAX_GVARS;i++) anas[i+MIX_3POS] = g_model.gvars[i].gvar * 1024 / 100 ;
#endif
//        for(uint8_t i=0;i<8;i++) anas[i+NUM_SKYXCHNRAW+1+MAX_GVARS+1] = g_ppmIns[i+8]*2; //add ppm channels

        //===========Swash Ring================
        if(g_model.swashRingValue)
        {
          uint32_t v = ((int32_t)anas[ele_stick]*anas[ele_stick] + (int32_t)anas[ail_stick]*anas[ail_stick]);
		      int16_t tmp = calc100toRESX(g_model.swashRingValue) ;
          uint32_t q ;
          q = (int32_t)tmp * tmp ;
          if(v>q)
          {
            uint16_t d = isqrt32(v);
            anas[ele_stick] = (int32_t)anas[ele_stick]*tmp/((int32_t)d) ;
            anas[ail_stick] = (int32_t)anas[ail_stick]*tmp/((int32_t)d) ;
          }
        }

#define REZ_SWASH_X(x)  ((x) - (x)/8 - (x)/128 - (x)/512)   //  1024*sin(60) ~= 886
#define REZ_SWASH_Y(x)  ((x))   //  1024 => 1024

        if(g_model.swashType)
        {
            int16_t vp = 0 ;
            int16_t vr = 0 ;

            if( !(att & NO_INPUT) )  //zero input for setStickCenter()
						{
	            vp = anas[ele_stick]+trimA[ele_stick];
  	          vr = anas[ail_stick]+trimA[ail_stick];
						}

            int16_t vc = 0;
            if(g_model.swashCollectiveSource)
						{
//#if NUM_EXTRA_POTS
							if ( g_model.swashCollectiveSource >= EXTRA_POTS_START )
							{
								vc = calibratedStick[g_model.swashCollectiveSource-EXTRA_POTS_START+7] ;
							}
							else
							{
              	vc = anas[g_model.swashCollectiveSource-1];
							}
//#else							
//              vc = anas[g_model.swashCollectiveSource-1];
//#endif
						}

            if(g_model.swashInvertELE) vp = -vp;
            if(g_model.swashInvertAIL) vr = -vr;
            if(g_model.swashInvertCOL) vc = -vc;

            switch (( uint8_t)g_model.swashType)
            {
            case (SWASH_TYPE_120):
                vp = REZ_SWASH_Y(vp);
                vr = REZ_SWASH_X(vr);
                anas[MIX_CYC1-1] = vc - vp;
                anas[MIX_CYC2-1] = vc + vp/2 + vr;
                anas[MIX_CYC3-1] = vc + vp/2 - vr;
                break;
            case (SWASH_TYPE_120X):
                vp = REZ_SWASH_X(vp);
                vr = REZ_SWASH_Y(vr);
                anas[MIX_CYC1-1] = vc - vr;
                anas[MIX_CYC2-1] = vc + vr/2 + vp;
                anas[MIX_CYC3-1] = vc + vr/2 - vp;
                break;
            case (SWASH_TYPE_140):
                vp = REZ_SWASH_Y(vp);
                vr = REZ_SWASH_Y(vr);
                anas[MIX_CYC1-1] = vc - vp;
                anas[MIX_CYC2-1] = vc + vp + vr;
                anas[MIX_CYC3-1] = vc + vp - vr;
                break;
            case (SWASH_TYPE_90):
                vp = REZ_SWASH_Y(vp);
                vr = REZ_SWASH_Y(vr);
                anas[MIX_CYC1-1] = vc - vp;
                anas[MIX_CYC2-1] = vc + vr;
                anas[MIX_CYC3-1] = vc - vr;
                break;
            default:
                break;
            }
	        }
  		  }

    		if(tick10ms)
				{
					inactivityCheck() ;
					trace(); //trace thr 0..32  (/32)
				}
			}
    memset(chans,0,sizeof(chans));        // All outputs to 0


    uint8_t mixWarning = 0;
    //========== MIXER LOOP ===============

    // Set the trim pointers back to the master set
//    TrimPtr[0] = &g_model.trim[0] ;
//    TrimPtr[1] = &g_model.trim[1] ;
//    TrimPtr[2] = &g_model.trim[2] ;
//    TrimPtr[3] = &g_model.trim[3] ;

#if EXTRA_SKYMIXERS
    for(uint8_t i=0;i<MAX_SKYMIXERS+EXTRA_SKYMIXERS;i++)
#else
    for(uint8_t i=0;i<MAX_SKYMIXERS;i++)
#endif
		{
        SKYMixData *md = mixAddress( i ) ;
				int8_t mixweight = REG100_100( md->weight ) ;

        if((md->destCh==0) || (md->destCh>NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)) break;

        //Notice 0 = NC switch means not used -> always on line
        int16_t v  = 0;
        uint8_t swTog;
        uint8_t swon = swOn[i] ;
				
				bool t_switch = getSwitch(md->swtch,1) ;
        if (md->swtch && (md->srcRaw >= PPM_BASE) && (md->srcRaw < PPM_BASE+NUM_PPM)	&& (ppmInValid == 0) )
				{
					// then treat switch as false ???				
					t_switch = 0 ;
				}	
        if (md->swtch && (md->srcRaw > MIX_3POS+MAX_GVARS + NUM_SCALERS ) && (ppmInValid == 0) )
				{ // Extra PPM inputs (9-16)
					if (md->srcRaw <= MIX_3POS+MAX_GVARS + NUM_SCALERS + NUM_EXTRA_PPM )
					{
						// then treat switch as false ???				
						t_switch = 0 ;
					}
				}
      
        if ( t_switch )
				{
					if ( md->modeControl & ( 1 << CurrentPhase ) )
					{
						t_switch = 0 ;
					}
				}

        uint8_t k = md->srcRaw ;

#define DEL_MULT 256
				//swOn[i]=false;
        if(!t_switch)
        { // switch on?  if no switch selected => on
            swTog = swon ;
            swOn[i] = swon = false ;
            if (k == MIX_3POS+MAX_GVARS+1) act[i] = chans[md->destCh-1] * DEL_MULT / 100 ;	// "THIS"
            if( k !=MIX_MAX && k !=MIX_FULL) continue;// if not MAX or FULL - next loop
            if(md->mltpx==MLTPX_REP) continue; // if switch is off and REPLACE then off
            v = ( k == MIX_FULL ? -RESX : 0); // switch is off and it is either MAX=0 or FULL=-512
        }
        else {
            swTog = !swon ;
            swon = true;
            k -= 1 ;
						v = anas[k]; //Switch is on. MAX=FULL=512 or value.
						if ( k < 4 )
						{
							if ( md->disableExpoDr )
							{
     		      	v = rawSticks[k]; //Switch is on. MAX=FULL=512 or value.
							}
						}
#ifdef PCBX9D
						if ( k == MIX_3POS-1 )
						{
							if ( md->switchSource > 8 )	// Sort for X9E ********
							{
								v = getSwitch( CSW_INDEX+md->switchSource-8, 0, 0 ) ;
								v = v ? 1024 : -1024 ;
							}
							else
							{
								uint32_t /*EnumKeys*/ sw = switchIndex[md->switchSource] ;
								if ( ( md->switchSource == 5) || ( md->switchSource == 7) )
								{ // 2-POS switch
        					v = hwKeyState(sw) ? 1024 : -1024 ;
	//        				v = keyState(sw) ? -1024 : 1024 ;
								}
								else if( md->switchSource == 8)
								{
									v = ((int32_t)switchPosition( HSW_Ele6pos0 ) * 2048 - 5120)/5 ;
								}
								else
								{ // 3-POS switch
        					v = hwKeyState(sw) ? -1024 : (hwKeyState(sw+1) ? 0 : 1024) ;
	//        				v = keyState(sw) ? -1024 : (keyState((EnumKeys)(sw+1)) ? 0 : 1024) ;
								}
							}
						}
#endif
#if defined(PCBSKY) || defined(PCB9XT)
						if ( k == MIX_3POS-1 )
						{
							if ( md->switchSource > 6 )
							{
								if ( md->switchSource == 31 )
								{
									v = ((int32_t)switchPosition( HSW_Ele6pos0 ) * 2048 - 5120)/5 ;
								}
								else
								{
									v = getSwitch( CSW_INDEX+md->switchSource-6, 0, 0 ) ;
									v = v ? 1024 : -1024 ;
								}
							}
							else
							{
								uint32_t sw = Sw3PosList[md->switchSource] ;
								if ( Sw3PosCount[md->switchSource] == 2 )
								{
        					v = hwKeyState(sw) ? 1024 : -1024 ;
								}
								else if ( Sw3PosCount[md->switchSource] == 6 )
								{
									v = ((int32_t)switchPosition( HSW_Ele6pos0 ) * 2048 - 5120)/5 ;
								}
								else
								{
        					v = hwKeyState(sw) ? -1024 : (hwKeyState(sw+1) ? 0 : 1024) ;
								}
							}
						}
#endif
//            if(k>=CHOUT_BASE && (k<i)) v = chans[k]; // if we've already calculated the value - take it instead // anas[i+CHOUT_BASE] = chans[i]
						if ( k >= CHOUT_BASE )
            {
							if(k<CHOUT_BASE+NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)
							{
								if ( md->disableExpoDr )
								{
									v = g_chans512[k-CHOUT_BASE] ;
								}
								else
								{
            			if(k<CHOUT_BASE+md->destCh-1)
									{
										v = chans[k-CHOUT_BASE] / 100 ; // if we've already calculated the value - take it instead // anas[i+CHOUT_BASE] = chans[i]
									}
									else
									{
										v = ex_chans[k-CHOUT_BASE] ;
									}
								}
							}
						}
						if (k == MIX_3POS+MAX_GVARS) v = chans[md->destCh-1] / 100 ;	// "THIS"
            if ( (k > MIX_3POS+MAX_GVARS) && ( k <= MIX_3POS+MAX_GVARS + NUM_SCALERS ) )
						{
							 v = calc_scaler( k - (MIX_3POS+MAX_GVARS+1), 0, 0 ) ;
            }
            if (k > MIX_3POS+MAX_GVARS + NUM_SCALERS)
						{
							if ( k <= MIX_3POS+MAX_GVARS + NUM_SCALERS + NUM_EXTRA_PPM )
							{
								v = g_ppmIns[k-(MIX_3POS+MAX_GVARS + NUM_SCALERS + 1) + 8]*2 ;
							}
							else	// ( k >= EXTRA_POTS_START )
							{
								// An extra pot
								v = calibratedStick[k-EXTRA_POTS_START+8] ;
							}
						}
						if(md->mixWarn) mixWarning |= 1<<(md->mixWarn-1); // Mix warning
//            if ( md->enableFmTrim )
//            {
//                if ( k <= 4 )
//                {
//                    TrimPtr[k] = &md->sOffset ;		// Use the value stored here for the trim
//                }
//            }
        }
        swOn[i] = swon ;

        //========== INPUT OFFSET ===============
//        if ( ( md->enableFmTrim == 0 ) && ( md->lateOffset == 0 ) )
        if ( md->lateOffset == 0 )
        {
#if GVARS
            if(md->sOffset) v += calc100toRESX( REG( md->sOffset, -125, 125 )	) ;
#else
            if(md->sOffset) v += calc100toRESX(md->sOffset);
#endif
        }

        //========== DELAY and PAUSE ===============
				if ( ( att & NO_DELAY_SLOW ) == 0 )
				{
        if (md->speedUp || md->speedDown || md->delayUp || md->delayDown)  // there are delay values
        {

					int16_t my_delay = sDelay[i] ;
					int32_t tact = act[i] ;
            //if(init) {
            //act[i]=(int32_t)v*DEL_MULT;
            //swTog = false;
            //}
#if DEL_MULT == 256
						int16_t diff = v-(tact>>8) ;
#else
            int16_t diff = v-tact/DEL_MULT;
#endif
						if ( ( diff > 10 ) || ( diff < -10 ) )
						{
							if ( my_delay == 0 )
							{
        				if (md->delayUp || md->delayDown)  // there are delay values
								{								
									swTog = 1 ;
								}
							}
						}
						else
						{
							my_delay = 0 ;							
						}

            if(swTog) {
                //need to know which "v" will give "anas".
                //curves(v)*weight/100 -> anas
                // v * weight / 100 = anas => anas*100/weight = v
                if(md->mltpx==MLTPX_REP)
                {
                    tact = (int32_t)anas[md->destCh-1+CHOUT_BASE]*DEL_MULT * 100;
                    if(mixweight) tact /= mixweight ;
                }
                diff = v-tact/DEL_MULT;
                if(diff) my_delay = (diff<0 ? md->delayUp :  md->delayDown) * 10 ;
            }

            if(my_delay > 0)
						{ // perform delay
              if(tick10ms)
              {
                my_delay -= 1 ;
							}
              if ( my_delay != 0)
             	{ // At end of delay, use new V and diff
#if DEL_MULT == 256
               	v = tact >> 8 ;	   // Stay in old position until delay over
#else
                v = tact/DEL_MULT;   // Stay in old position until delay over
#endif
                diff = 0;
 	            }
							else
							{
								my_delay = -1 ;
							}
            }

					sDelay[i] = my_delay ;

            if(diff && (md->speedUp || md->speedDown)){
                //rate = steps/sec => 32*1024/100*md->speedUp/Down
                //act[i] += diff>0 ? (32768)/((int16_t)100*md->speedUp) : -(32768)/((int16_t)100*md->speedDown);
                //-100..100 => 32768 ->  100*83886/256 = 32768,   For MAX we divide by 2 sincde it's asymmetrical
                if(tick10ms) {
                    int32_t rate = (int32_t)DEL_MULT*2048*100;
                    if(mixweight) rate /= abs(mixweight);
// The next few lines could replace the long line act[i] = etc. - needs testing
										int16_t speed ;
                    if ( diff>0 )
										{
											speed = md->speedUp ;
										}
										else
										{
											rate = -rate ;											
											speed = md->speedDown ;
										}
										tact = (speed) ? tact+(rate)/((int16_t)10*speed) : (int32_t)v*DEL_MULT ;

//                    act[i] = (diff>0) ? ((md->speedUp>0)   ? act[i]+(rate)/((int16_t)10*(md->speedUp))   :  (int32_t)v*DEL_MULT) :
//                                        ((md->speedDown>0) ? act[i]-(rate)/((int16_t)10*(md->speedDown)) :  (int32_t)v*DEL_MULT) ;
                }
								{
#if DEL_MULT == 256
									int32_t tmp = tact>>8 ;
#else
									int32_t tmp = tact/DEL_MULT ;
#endif
                	if(((diff>0) && (v<tmp)) || ((diff<0) && (v>tmp))) tact=(int32_t)v*DEL_MULT; //deal with overflow
                }
#if DEL_MULT == 256
                v = tact >> 8 ;
#else
                v = tact/DEL_MULT;
#endif
            }
            else if (diff)
            {
              tact=(int32_t)v*DEL_MULT;
            }
					act[i] = tact ;
        }
				}
				else
				{
					act[i] = (int32_t)v*DEL_MULT ;
				}
        //========== CURVES ===============
				if ( md->differential )
				{
      		//========== DIFFERENTIAL =========
      		int8_t curveParam = REG100_100( md->curve ) ;
      		if (curveParam > 0 && v < 0)
      		  v = (v * (100 - curveParam)) / 100;
      		else if (curveParam < 0 && v > 0)
      		  v = (v * (100 + curveParam)) / 100;
				}
				else
				{
					if ( md->curve <= -28 )
					{
						// do expo using md->curve + 128
      			v = expo( v, md->curve + 128 ) ;
					}
					else
					{
        		switch(md->curve){
        		case 0:
        		    break;
        		case 1:
        		    if(md->srcRaw == MIX_FULL) //FUL
        		    {
        		        if( v<0 ) v=-RESX;   //x|x>0
        		        else      v=-RESX+2*v;
        		    }else{
        		        if( v<0 ) v=0;   //x|x>0
        		    }
        		    break;
        		case 2:
        		    if(md->srcRaw == MIX_FULL) //FUL
        		    {
        		        if( v>0 ) v=RESX;   //x|x<0
        		        else      v=RESX+2*v;
        		    }else{
        		        if( v>0 ) v=0;   //x|x<0
        		    }
        		    break;
        		case 3:       // x|abs(x)
        		    v = abs(v);
        		    break;
        		case 4:       //f|f>0
        		    v = v>0 ? RESX : 0;
        		    break;
        		case 5:       //f|f<0
        		    v = v<0 ? -RESX : 0;
        		    break;
        		case 6:       //f|abs(f)
        		    v = v>0 ? RESX : -RESX;
        		    break;
        		default: //c1..c16
								{
									int8_t idx = md->curve ;
									if ( idx < 0 )
									{
										v = -v ;
										idx = 6 - idx ;								
									}
        		    	v = intpol(v, idx - 7);
								}
        		}
					}
				}

        //========== TRIM ===============
        if((md->carryTrim==0) && (md->srcRaw>0) && (md->srcRaw<=4)) v += trimA[md->srcRaw-1];  //  0 = Trim ON  =  Default

        //========== MULTIPLEX ===============
#if GVARS
        int32_t dv = (int32_t)v*mixweight ;
#else
        int32_t dv = (int32_t)v*md->weight;
#endif
				
        //========== lateOffset ===============
//				if ( ( md->enableFmTrim == 0 ) && ( md->lateOffset ) )
				if ( md->lateOffset )
        {
#if GVARS
            if(md->sOffset) dv += calc100toRESX( REG( md->sOffset, -125, 125 )	) * 100 ;
#else
            if(md->sOffset) dv += calc100toRESX(md->sOffset) * 100 ;
#endif
        }
				
				int32_t *ptr ;			// Save calculating address several times
				ptr = &chans[md->destCh-1] ;
        switch((uint8_t)md->mltpx){
        case MLTPX_REP:
            *ptr = dv;
            break;
        case MLTPX_MUL:
						dv /= 100 ;
						dv *= *ptr ;
            dv /= RESXl;
            *ptr = dv ;
//            chans[md->destCh-1] *= dv/100l;
//            chans[md->destCh-1] /= RESXl;
            break;
        default:  // MLTPX_ADD
            *ptr += dv; //Mixer output add up to the line (dv + (dv>0 ? 100/2 : -100/2))/(100);
            break;
        }
    }

    //========== MIXER WARNING ===============
    //1= 00,08
    //2= 24,32,40
    //3= 56,64,72,80
    {
        uint16_t tmr10ms ;
        tmr10ms = get_tmr10ms() ;

        if(mixWarning & 1) if(((tmr10ms&0xFF)==  0)) audioDefevent(AU_MIX_WARNING_1);
        if(mixWarning & 2) if(((tmr10ms&0xFF)== 64) || ((tmr10ms&0xFF)== 72)) audioDefevent(AU_MIX_WARNING_2);
        if(mixWarning & 4) if(((tmr10ms&0xFF)==128) || ((tmr10ms&0xFF)==136) || ((tmr10ms&0xFF)==144)) audioDefevent(AU_MIX_WARNING_3);        


    }

		ThrottleStickyOn = 0 ;
    //========== LIMITS ===============
    for(uint8_t i=0;i<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS;i++)
		{
        // chans[i] holds data from mixer.   chans[i] = v*weight => 1024*100
        // later we multiply by the limit (up to 100) and then we need to normalize
        // at the end chans[i] = chans[i]/100 =>  -1024..1024
        // interpolate value with min/max so we get smooth motion from center to stop
        // this limits based on v original values and min=-1024, max=1024  RESX=1024

        int32_t q = chans[i];// + (int32_t)g_model.limitData[i].offset*100; // offset before limit

				if ( Fade.fadePhases )
				{
					int32_t l_fade = Fade.fade[i] ;
					if ( att & FADE_FIRST )
					{
						l_fade = 0 ;
					}
					l_fade += ( q / 100 ) * Fade.fadeScale[CurrentPhase] ;
					Fade.fade[i] = l_fade ;
			
					if ( ( att & FADE_LAST ) == 0 )
					{
						continue ;
					}
					l_fade /= Fade.fadeWeight ;
					q = l_fade * 100 ;
				}
    	  chans[i] = q / 100 ; // chans back to -1024..1024
        
				ex_chans[i] = chans[i]; //for getswitch

        LimitData *limit = &g_model.limitData[i] ;
#if EXTRA_SKYCHANNELS
				if ( i >= NUM_SKYCHNOUT )
				{
					limit = &g_model.elimitData[i-NUM_SKYCHNOUT] ;
				}
#endif			
				int16_t ofs = limit->offset;
				int16_t xofs = ofs ;
				if ( xofs > g_model.sub_trim_limit )
				{
					xofs = g_model.sub_trim_limit ;
				}
				else if ( xofs < -g_model.sub_trim_limit )
				{
					xofs = -g_model.sub_trim_limit ;
				}
        int16_t lim_p = 10*(limit->max+100) + xofs ;
        int16_t lim_n = 10*(limit->min-100) + xofs ; //multiply by 10 to get same range as ofs (-1000..1000)
				if ( lim_p > 1250 )
				{
					lim_p = 1250 ;
				}
				if ( lim_n < -1250 )
				{
					lim_n = -1250 ;
				}
        if(ofs>lim_p) ofs = lim_p;
        if(ofs<lim_n) ofs = lim_n;

        if(q) q = (q>0) ?
                    q*((int32_t)lim_p-ofs)/100000 :
                    -q*((int32_t)lim_n-ofs)/100000 ; //div by 100000 -> output = -1024..1024

        q += calc1000toRESX(ofs);
        lim_p = calc1000toRESX(lim_p);
        lim_n = calc1000toRESX(lim_n);
        if(q>lim_p) q = lim_p;
        if(q<lim_n) q = lim_n;
        if(limit->revert) q=-q;// finally do the reverse.

				{
					uint8_t numSafety = NUM_SKYCHNOUT - g_model.numVoice ;
					if ( i < numSafety )
					{
        		if(g_model.safetySw[i].opt.ss.swtch)  //if safety sw available for channel check and replace val if needed
						{
							if ( ( g_model.safetySw[i].opt.ss.mode != 1 ) && ( g_model.safetySw[i].opt.ss.mode != 2 ) )	// And not used as an alarm
							{
								static uint32_t sticky = 0 ;
								uint8_t applySafety = 0 ;
								int8_t sSwitch = g_model.safetySw[i].opt.ss.swtch ;
								
								if(getSwitch00( sSwitch))
								{
									applySafety = 1 ;
								}

								if ( g_model.safetySw[i].opt.ss.mode == 3 )
								{
									int8_t thr = g_model.safetySw[i].opt.ss.source ;
									uint32_t rev_thr = 0 ;
									if ( thr == 0 )
									{
										thr = 2 ;
									}
									else
									{
										if ( thr > 0 )
										{
											thr += 3 ;
										}
										else
										{
											rev_thr = 1 ;
											thr = -thr + 3 ;
										}
									}	
									// Special case, sticky throttle
									if( applySafety )
									{
										sticky &= ~(1<<i) ;
									}
//#ifdef FIX_MODE
									else
									{
										uint32_t throttleOK = 0 ;
//										if ( g_model.throttleIdle == 2 )
//										{
//  										if(calibratedStick[2] > 1004)
//  										{
//												throttleOK = 1 ;
//  										}
//										}
//										else 
										if ( g_model.throttleIdle )
										{
											if ( abs( calibratedStick[thr] ) < 20 )
											{
												throttleOK = 1 ;
											}
										}
										else
										{
											if ( rev_thr )
											{
  											if(calibratedStick[thr] > 1004)
  											{
													throttleOK = 1 ;
  											}
											}
											else
											{
  											if(calibratedStick[thr] < -1004)
  											{
													throttleOK = 1 ;
  											}
											}
										}
										
										if ( throttleOK )
										{
											if ( trainerThrottleValid )
											{
												if ( trainerThrottleValue < -1004 )
												{
													sticky |= (1<<i) ;
												}
											}	
											else
											{
												sticky |= (1<<i) ;
											}
										}
									}
//#else
//									else if ( calibratedStick[THR_STICK] < -1004 )
//									{
//										if ( trainerThrottleValid )
//										{
//											if ( trainerThrottleValue < -1004 )
//											{
//												sticky = 1 ;
//											}
//										}	
//										else
//										{
//											sticky = 1 ;
//										}
//									}
//#endif
									if ( ( sticky & (1<<i) ) == 0 )
									{
										applySafety = 1 ;
									}
									ThrottleStickyOn = applySafety ;
								}
								if ( applySafety ) q = calc100toRESX(g_model.safetySw[i].opt.ss.val) ;
							}
						}
					}
				}
//        cli();
        chanOut[i] = q; //copy consistent word to int-level
//        sei();
		}
}


//#ifdef PCBX9D
//#ifdef REVPLUS
//void menuProcImage(uint8_t event)
//{
//	MENU(XPSTR("Image"), menuTabStat, e_Image, 1, {0} ) ;

//	lcd_outhex4( 0, FH, ModelImageValid ) ;
//extern uint8_t LoadImageResult ;
//	lcd_outhex4( 30, FH, LoadImageResult ) ;
//extern TCHAR ImageFilename[] ;
//	lcd_puts_Pleft( 7*FH, ImageFilename ) ;
////extern uint8_t TNAME[] ;
////	lcd_puts_Pleft( 6*FH, g_model.modelImageName ) ;
////	lcd_puts_Pleft( 7*FH, (char *)TNAME ) ;
//extern uint8_t ImageDisplay ;
//extern uint8_t ImageX ;
//extern uint8_t ImageY ;
//extern uint8_t ModelImageValid ;
	
//	if ( ModelImageValid )
//	{
//		ImageX = 32 ;
//		ImageY = 16 ;
//		ImageDisplay = 1 ;
//	} 
//}
//#endif
//#endif

void menuProcSDstat(uint8_t event)
{
//  MENU(PSTR(STR_ST_CARD_STAT), menuTabDiag, e_Setup3, 1, {0/*, 0*/});
	MENU(PSTR(STR_ST_CARD_STAT), menuTabStat, e_Setup3, 1, {0} ) ;

#ifdef PCBX9D
	if ( event == EVT_ENTRY )
	{
extern void sdInit( void ) ;
		sdInit() ;
	}
#endif
	 
//	int8_t  sub    = mstate2.m_posVert;
//	uint8_t subSub = mstate2.m_posHorz;
//	bool    edit;
//	uint8_t blink ;

//	evalOffset(sub);

	uint32_t present ;
	lcd_puts_Pleft( 1*FH, XPSTR("Present"));

#ifdef PCBSKY
	present = CardIsPresent() ;
#else
extern DWORD socket_is_empty( void ) ;
	present = !socket_is_empty() ;
#endif
	lcd_putsAttIdx( 9*FW, 1*FH, XPSTR("\003No Yes"), present, 0 ) ;
	
	lcd_puts_Pleft( 2*FH, PSTR(STR_4_READY));

extern uint8_t SectorsPerCluster ;
  lcd_outdezAtt( 127, 1*FH, SectorsPerCluster/2, 0 ) ;
#ifndef SIMU

#ifdef PCBSKY
	uint32_t i ;
	uint8_t x, y ;
#endif

#ifdef PCBX9D
extern uint8_t CardType ;
extern uint32_t sdMounted( void ) ;
	lcd_outhex4( 10*FW, 2*FH, CardType ) ;
	lcd_outhex4( 16*FW, 2*FH, sdMounted() ) ;
	lcd_outhex4( 10*FW, 3*FH, Card_state ) ;
#endif

#ifdef PCBSKY
	lcd_outhex4( 10*FW, 2*FH, Card_state ) ;
	 
extern uint32_t SDlastError ;
	lcd_outhex4( 16*FW, 2*FH, SDlastError ) ;
	 
//	if (sd_card_ready() )		// Moved for debugging
//	{
		y = 3*FH ;
		x = 4*FW ;
		lcd_puts_Pleft( y, XPSTR("CID"));
		for ( i = 0 ; i < 4 ; i += 1 )
		{
		  lcd_outhex4( x, y, Card_ID[i] >> 16 ) ;
		  lcd_outhex4( x+4*FW, y, Card_ID[i] ) ;
			x += 8*FW ;
			if ( i == 1 )
			{
				y += FH ;
				x = 4*FW ;				
			}			 
		}
		y = 5*FH ;
		x = 4*FW ;
		lcd_puts_Pleft( y, XPSTR("CSD"));
		for ( i = 0 ; i < 4 ; i += 1 )
		{
		  lcd_outhex4( x, y, Card_CSD[i] >> 16 ) ;
		  lcd_outhex4( x+4*FW, y, Card_CSD[i] ) ;
			x += 8*FW ;
			if ( i == 1 )
			{
				y += FH ;
				x = 4*FW ;				
			}			 
		}
		y = 7*FH ;
		x = 4*FW ;
		lcd_puts_Pleft( y, XPSTR("SCR"));
		for ( i = 0 ; i < 2 ; i += 1 )
		{
		  lcd_outhex4( x, y, Card_SCR[i] >> 16 ) ;
		  lcd_outhex4( x+4*FW, y, Card_SCR[i] ) ;
			x += 8*FW ;
		}
#endif
	if (sd_card_ready() )
	{
	}
	else
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_NOT));
	}
#endif
}


uint8_t evalOffset(int8_t sub)
{
	uint8_t max = 6 ;
  uint8_t t_pgOfs = s_pgOfs ;
	int8_t x = sub-t_pgOfs ;
    if(sub<1) t_pgOfs=0;
    else if(x>max) t_pgOfs = sub-max;
    else if(x<max-6) t_pgOfs = sub-max+6;
		return (s_pgOfs = t_pgOfs) ;
}

#define EBACK_START		0
#define EBACK_OPEN		1
#define EBACK_WRITE		2

#define EBACK_DONE		10

#define ERESTORE_START				0
#define ERESTORE_FILL_NAMES		1
#define ERESTORE_SELECT				2
#define ERESTORE_CONFIRM			3
#define ERESTORE_OPEN					4
#define ERESTORE_READ					5

#define ERESTORE_DONE		10


#if defined(PCBSKY) || defined(PCB9XT)
extern uint16_t General_timer ;
extern uint16_t Model_timer ;
#endif

const char *EepromErrorText ;

void menuBackupEeprom( uint8_t event )
{
	static uint16_t count ;
	static uint8_t state ;
	TITLE(XPSTR("Backup EEPROM"));
	
	switch ( event )
	{
		case EVT_ENTRY :
			state = EBACK_START ;
			EepromErrorText = NULL ;
#if defined(PCBSKY) || defined(PCB9XT)
			if ( General_timer )
			{
				General_timer = 1 ;		// Make these happen soon
			}
			if ( Model_timer )
			{
				Model_timer = 1 ;
			}
//#else
//			ee32_check_finished() ;
#endif

		break ;

    case EVT_KEY_BREAK(KEY_EXIT):
			if ( state == EBACK_DONE )
			{
				popMenu( false ) ;
				// need a reboot if restoring!!
				// Or call eereadall()
			}
		break ;
	}

	switch ( state )
	{
		case EBACK_START :
			lcd_puts_Pleft( 4*FH, XPSTR("\006Preparing") ) ;
			if ( ee32_check_finished() )
			{
				state = EBACK_OPEN ;
			}
		break ;

		case EBACK_OPEN :
			WatchdogTimeout = 200 ;		// 2 seconds
			count = 0 ;
//#if defined(PCBSKY) || defined(PCB9XT)
			EepromErrorText = openBackupEeprom() ;
//#else
//			EepromErrorText = "Not Implemented" ;
//#endif
			if ( EepromErrorText == NULL)
			{
				state = EBACK_WRITE ;
			}
			else
			{
				state = EBACK_DONE ;
			}
		break ;
		
		case EBACK_WRITE :
#if defined(PCBSKY) || defined(PCB9XT)
			if ( count < 256 )
#else
			if ( count < 64 )
#endif
			{
				EepromErrorText = processBackupEeprom( count ) ;
				uint32_t i ;
#if defined(PCBSKY) || defined(PCB9XT)
				uint32_t width = count / 4 ;
#else
				uint32_t width = count ;
#endif
				lcd_hline( 0, 5*FH-1, 65 ) ;
				lcd_hline( 0, 6*FH, 65 ) ;
				lcd_vline( 64, 5*FH, 8 ) ;
				for ( i = 0 ; i <= width ; i += 1 )
				{
					lcd_vline( i, 5*FH, 8 ) ;
				}
				count += 1 ;
				if ( EepromErrorText )
				{
					state = EBACK_DONE ;
//#if defined(PCBSKY) || defined(PCB9XT)
					closeBackupEeprom() ;					
//#endif
				}
			}
			else
			{
				state = EBACK_DONE ;
//#if defined(PCBSKY) || defined(PCB9XT)
				closeBackupEeprom() ;					
//#endif
			}
		break ;
		
		case EBACK_DONE :
			if ( EepromErrorText == NULL)
			{
				lcd_puts_Pleft( 4*FH, XPSTR("\004EEPROM Saved") ) ;
			}
			else
			{
				lcd_puts_Pleft( 4*FH, EepromErrorText ) ;
			}
		break ;
	}
}

void menuRestoreEeprom( uint8_t event )
{
	static uint16_t count ;
	static uint8_t state ;
	struct fileControl *fc = &FileControl ;
	TITLE(XPSTR("Restore EEPROM"));
	
	switch ( event )
	{
		case EVT_ENTRY :
			state = ERESTORE_START ;
			EepromErrorText = NULL ;
#if defined(PCBSKY) || defined(PCB9XT)
			if ( General_timer )
			{
				General_timer = 1 ;		// Make these happen soon
			}
			if ( Model_timer )
			{
				Model_timer = 1 ;
			}
//#else
//			ee32_check_finished() ;
#endif

		break ;

    case EVT_KEY_BREAK(KEY_EXIT):
			if ( state == ERESTORE_DONE )
			{
				popMenu( false ) ;
				// need a reboot if restoring!!
				// Or call eereadall()
			}
		break ;
	}

	switch ( state )
	{
		case EBACK_START :
			lcd_puts_Pleft( 4*FH, XPSTR("\006Preparing") ) ;
			if ( ee32_check_finished() )
			{
				state = ERESTORE_FILL_NAMES ;
			}
		break ;

		case ERESTORE_FILL_NAMES :
			setupFileNames( (TCHAR *)"/EEPROM", fc, (char *)"BIN" ) ;
			state = ERESTORE_SELECT ;
		break ;
			
		case ERESTORE_SELECT :
		{
			uint32_t i ;
			i = fileList( event, &FileControl ) ;
			if ( i == 1 )	// Select
			{
				state = ERESTORE_CONFIRM ;
//  		  killEvents(event) ;
//  		  popMenu() ;
			}
			else if ( i == 2 )	// EXIT
			{
  		  killEvents(event) ;
  		  popMenu() ;
			}
		}
		break ;

		case ERESTORE_CONFIRM :
			lcd_puts_Pleft( 2*FH, "Restore EEPROM from" ) ;
			lcd_putsnAtt( 0, 4*FH, Filenames[fc->vpos], 21, 0 ) ;
			if ( event == EVT_KEY_LONG(KEY_MENU) )
			{
				state = ERESTORE_OPEN ;
			}
			if ( event == EVT_KEY_LONG(KEY_EXIT) )
			{
				state = ERESTORE_SELECT ;		// Canceled
			}
		break ;

		case ERESTORE_OPEN :
			WatchdogTimeout = 200 ;		// 2 seconds
			count = 0 ;
//#if defined(PCBSKY) || defined(PCB9XT)
			EepromErrorText = openRestoreEeprom( Filenames[fc->vpos] ) ;
//#else
//			EepromErrorText = "Not Implemented" ;
//#endif
			if ( EepromErrorText == NULL)
			{
				state = ERESTORE_READ ;
			}
			else
			{
				state = ERESTORE_DONE ;
			}
		break ;
		
		case ERESTORE_READ :
#if defined(PCBSKY) || defined(PCB9XT)
			if ( count < 256 )
#else
			if ( count < 64 )
#endif
			{
//#if defined(PCBSKY) || defined(PCB9XT)
				EepromErrorText = processRestoreEeprom( count ) ;
//#endif
				uint32_t i ;
#if defined(PCBSKY) || defined(PCB9XT)
				uint32_t width = count / 4 ;
#else
				uint32_t width = count ;
#endif
				lcd_hline( 0, 5*FH-1, 65 ) ;
				lcd_hline( 0, 6*FH, 65 ) ;
				lcd_vline( 64, 5*FH, 8 ) ;
				for ( i = 0 ; i <= width ; i += 1 )
				{
					lcd_vline( i, 5*FH, 8 ) ;
				}
				count += 1 ;
				if ( EepromErrorText )
				{
					state = ERESTORE_DONE ;
#if defined(PCBSKY) || defined(PCB9XT)
					closeBackupEeprom() ;					
#endif
				}
			}
			else
			{
				state = ERESTORE_DONE ;
//#if defined(PCBSKY) || defined(PCB9XT)
				closeBackupEeprom() ;					
//#endif
#if defined(PCBSKY) || defined(PCB9XT)
				init_eeprom() ;
#endif
				eeReadAll() ;
				createSwitchMapping() ;
#if defined(PCBSKY) || defined(PCB9XT)
				create6posTable() ;
#endif 
			}
		break ;
		
		case ERESTORE_DONE :
			if ( EepromErrorText == NULL)
			{
				lcd_puts_Pleft( 4*FH, XPSTR("\004(EEPROM Restored)") ) ;
			}
			else
			{
				lcd_puts_Pleft( 4*FH, EepromErrorText ) ;
			}
		break ;
	}
}


//const char Str_Display[] =     "Display" ;
//const char Str_AudioHaptic[] = "AudioHaptic" ;
//const char Str_Alarms[] =      "Alarms" ;
const char Str_General[] =     "General" ;
//const char Str_Controls[] =    "Controls" ;
//const char Str_Hardware[] =    "Hardware" ;
//const char Str_Calibration[] = "Calibration" ;
//const char Str_Trainer[] =     "Trainer" ;
//const char Str_Version[] =     "Version" ;
//const char Str_ModuleRssi[] =  "FrSky xSSI" ;
//const char Str_DateTime[] =    "Date-Time" ;
//const char Str_DiagSwtch[] =   "DiagSwtch" ; 
//const char Str_DiagAna[] =     "DiagAna" ;
			
#define M_INDEX			0
#define M_DISPLAY		1
#define M_AUDIO			2
#define M_ALARMS		3
#define M_GENERAL		4
#define M_CONTROLS	5
#define M_HARDWARE	6
#define M_CALIB			7
#define M_BLUETOOTH	8
#define M_TRAINER		9
#define M_VERSION		10
#define M_MODULE		11
#define M_EEPROM		12
#define M_DATE			13
#define M_DIAGKEYS	14
#define M_DIAGANA		15

//static void displayIndex( const char *const strings[], uint8_t extra, uint8_t lines, uint8_t highlight )
static void displayIndex( const uint16_t *strings, uint8_t extra, uint8_t lines, uint8_t highlight )
{
	uint8_t offset = 7 ;
	if ( extra == 8 )
	{
		offset = 8 ;
		
		lcd_puts_P( 69, 0, PSTR(strings[7]) ) ;
	}
	for ( uint8_t i = 0 ; i < lines ; i += 1 )
	{
		lcd_puts_P( 1, (i+1)*FH, PSTR(strings[i]) ) ;
		if ( i < extra )
		{
			lcd_puts_P( 69, (i+1)*FH, PSTR(strings[i+offset]) ) ;
		
		
		
		
		
//		lcd_puts_P( 69, 0, (const char *)pgm_read_adr( &strings[7] ) ) ;
//	}
//	for ( uint8_t i = 0 ; i < lines ; i += 1 )
//	{
//		lcd_puts_P( 1, (i+1)*FH, (const char *)pgm_read_adr( &strings[i] ) ) ;
//		if ( i < extra )
//		{
//			lcd_puts_P( 69, (i+1)*FH, (const char *)pgm_read_adr( &strings[i+offset] ) ) ;
		}
	} 
	
	lcd_vline( 67, 0, 63 ) ;

	if ( highlight )
	{
		if ( highlight > 7 )
		{
			lcd_char_inverse( 68, (highlight-offset)*FH, 60, 0 ) ;
		}
		else
		{
			lcd_char_inverse( 0, highlight*FH, 67, 0 ) ;
		}
	}
}

static uint8_t SubMenuCall = 0 ;

static uint8_t indexProcess( uint8_t event, MState2 *pmstate, uint8_t extra )
{
	if (event == EVT_ENTRY)
	{
//		MenuTimer = 2000 ;	// * 0.01 Seconds = 20 seconds
		pmstate->m_posVert = SubmenuIndex - 1 ;
		SubmenuIndex = 0 ;
//		SubMenuFromIndex = 0 ;
	}
	if (event == EVT_ENTRY_UP)
	{
		SingleExpoChan = 0 ;
		pmstate->m_posVert = SubmenuIndex - 1 ;
		if ( SubMenuCall )
		{
			pmstate->m_posVert = SubMenuCall & 0x1F ;
			g_posHorz = ( SubMenuCall >> 5 ) & 3 ;
			SubMenuCall = 0 ;
		}
		else
		{
			SubmenuIndex = 0 ;
//			SubMenuFromIndex = 0 ;
		}
	}
	
	if ( UseLastSubmenuIndex )
	{
		SubmenuIndex = LastSubmenuIndex & 0x7F ;
		UseLastSubmenuIndex = 0 ;
//		SubMenuFromIndex = 0 ;
	}
	
	if ( SubmenuIndex )
	{
//  	if (event == EVT_KEY_LONG(KEY_EXIT) )
//		{
//      s_editMode = false;
//			pmstate->m_posVert = SubmenuIndex ;
//			SubmenuIndex = 0 ;
//			killEvents(event) ;
//			event = 0 ;
//		}
//  	if (event == EVT_KEY_FIRST(KEY_EXIT) )
//		{
//			event = 0 ;
//		}
  	if ( (event == EVT_KEY_BREAK(KEY_EXIT) ) || (event == EVT_KEY_LONG(BTN_RE) ) )
		{
      if(s_editMode)
			{
        s_editMode = false;
			}
			else
			{
				pmstate->m_posVert = SubmenuIndex - 1 ;
				SubmenuIndex = 0 ;
				killEvents(event) ;
				audioDefevent(AU_MENUS) ;
			}
			event = 0 ;
		}
	}
	else
	{
		uint8_t pv = pmstate->m_posVert ;
		if (event == EVT_KEY_FIRST(KEY_RIGHT) )
		{
			if ( pv < ((extra == 8) ? 7 : extra) )
			{
				pv += (extra == 8) ? 8 : 7 ;
//				pv += 7 ;
			}
		}
		if (event == EVT_KEY_FIRST(KEY_LEFT) )
		{
			if ( pv >= ((extra == 8) ? 8 : 7) )
			{
				pv -= (extra == 8) ? 8 : 7 ;
			}
		}

 		if ( (event == EVT_KEY_FIRST(KEY_MENU) ) ||(event == EVT_KEY_BREAK(BTN_RE) ) )
		{
			SubmenuIndex = pv + 1 ;
			LastSubmenuIndex = SubmenuIndex | 0x80 ;
			pv = 0 ;
			killEvents(event) ;
			event = EVT_ENTRY ;
			Tevent = EVT_ENTRY ;
			g_posHorz = 0 ;
		}
		pmstate->m_posVert = pv ;
	}
	return event ;
}

uint8_t edit3posSwitchSource( uint8_t y, uint8_t value, uint16_t mask, uint8_t condition, uint8_t allowAna )
{
	uint16_t sm = g_eeGeneral.switchMapping ;
#ifdef REVX
	value = checkIndexed( y, XPSTR(FWx17"\005\004NONELCD2LCD6LCD7DAC1ELE "), value, condition ) ;
#else
#ifdef PCB9XT
	value = checkIndexed( y, XPSTR(FWx17"\010\004NONEEXT1EXT2EXT3EXT4EXT5EXT6EXT7EXT8"), value, condition ) ;
#else
#ifdef PCBX9D
	value = checkIndexed( y, XPSTR(FWx17"\002\004NONEJTMSJTCK"), value, condition ) ;
#else
	if ( g_eeGeneral.ar9xBoard == 0 )
	{
		if ( allowAna )
		{
			value = checkIndexed( y, XPSTR(FWx17"\011\004NONELCD2LCD6LCD7DAC1ELE EXT4EXT5EXT6ANA "), value, condition ) ;
		}
		else
		{
			value = checkIndexed( y, XPSTR(FWx17"\010\004NONELCD2LCD6LCD7DAC1ELE EXT4EXT5EXT6"), value, condition ) ;
		}
	}
	else
	{
		if ( allowAna )
		{
			if ( g_eeGeneral.ar9xBoard == 0 )
			{			
				value = checkIndexed( y, XPSTR(FWx17"\006\004NONEEXT1EXT2EXT3PB14ELE ANA "), value, condition ) ;
			}
			else
			{
				value = checkIndexed( y, XPSTR(FWx17"\006\004NONEEXT1EXT2EXT3PB14ELE AD10"), value, condition ) ;
			}
		}
		else
		{
			value = checkIndexed( y, XPSTR(FWx17"\005\004NONEEXT1EXT2EXT3PB14ELE "), value, condition ) ;
		}
	}
#endif
#endif
#endif
	g_eeGeneral.switchMapping = sm & ~(mask) ;
	if ( value )
	{
		g_eeGeneral.switchMapping |= mask ;
	} 
	if ( sm != g_eeGeneral.switchMapping )
	{
		createSwitchMapping() ;
	}
	return value ;
}

#ifdef PCB9XT
void editExtraPot( uint8_t y, uint32_t index, uint32_t active )
{
	lcd_puts_Pleft( y, index ? XPSTR( "POT5" ) : XPSTR( "POT4") ) ;
	uint8_t value = g_eeGeneral.extraPotsSource[index] ;
	value = checkIndexed( y, XPSTR(FWx17"\004\004NONEAIP1AIP2AIP3AIP4"), value, active ) ;
	g_eeGeneral.extraPotsSource[index] = value ;
	NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
}
#endif	// PCB9XT


void menuProcIndex(uint8_t event)
{
	static MState2 mstate;
	EditType = EE_GENERAL ;

	event = indexProcess( event, &mstate, 8 ) ;
	event = mstate.check_columns( event, IlinesCount-1 ) ;
//	event = mstate.check( event, 0, NULL, 0, &Columns, 0, IlinesCount-1 ) ;

//	if ( SubmenuIndex > 6 )
//	{
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
			case M_MODULE :
        pushMenu(menuProcRSSI) ;
			break ;
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
//	}

	uint32_t sub = mstate.m_posVert ;
	uint32_t y = FH ;
//	Columns = 0 ;
	uint8_t blink = InverseBlink ;

	switch ( SubmenuIndex )
	{
		case 0 :
  		TITLE(XPSTR("Radio Setup"));
			IlinesCount = 15 ;
			sub += 1 ;
			
			
//static const char *const n_Strings[] = {
//Str_Display,
//Str_AudioHaptic,
//Str_Alarms,
//Str_General,
//Str_Controls,
//Str_Hardware,
//Str_Calibration,
//Str_Trainer,
//Str_Version,
//Str_ModuleRssi,
//Str_DateTime,
//Str_DiagSwtch,
//Str_DiagAna
//} ;	

static const uint16_t in_Strings[] = {
STR_Display,
STR_AudioHaptic,
STR_Alarms,
STR_General,
STR_Controls,
STR_Hardware,
STR_Calibration,
STR_Bluetooth,
STR_Trainer,
STR_Version,
STR_ModuleRssi,
STR_Eeprom,
STR_DateTime,
STR_DiagSwtch,
STR_DiagAna
};
	
			 
//			displayIndex( n_Strings, 6, 7, sub ) ;
			displayIndex( in_Strings, 8, 7, sub ) ;
		
		break ;
		
		case M_DISPLAY :
		{	
			uint8_t subN = 0 ;
			
      TITLE( PSTR(STR_Display) ) ;
#ifdef PCBX9D
 #ifdef REVPLUS
			IlinesCount = 7 ;
 #else            
			IlinesCount = 6 ;
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
#ifdef PCB9XT
 			y = 0 ;
			IlinesCount = 9 ;
		 
		 if ( sub < 7 )
		 {
			displayNext() ;
		 	
#endif            
  		uint8_t attr ;
//			y = FH ;
			attr = (sub==subN) ? blink : 0 ;
      if( attr )
			{
				uint8_t oldValue = g_eeGeneral.contrast ;
#ifdef REVPLUS
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, 0, 60);
#else
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, 10, 45);
#endif
				if ( g_eeGeneral.contrast != oldValue )
				{
					lcdSetRefVolt(g_eeGeneral.contrast);
				}
      }
//			lcd_xlabel_decimal( PARAM_OFS+2*FW, y, g_eeGeneral.contrast, attr, PSTR(STR_CONTRAST) ) ;
#if defined(PCBSKY) || defined(PCB9XT)
      lcd_puts_P( 8*FW, y, PSTR(STR_CONTRAST) ) ;
#else
      lcd_puts_Pleft( y, PSTR(STR_CONTRAST) ) ;
#endif
      lcd_outdezAtt( PARAM_OFS+2*FW, y, g_eeGeneral.contrast, (sub==subN) ? blink : 0) ;
			y += FH ;
			subN += 1 ;
  		
			lcd_puts_Pleft( y, PSTR(STR_BRIGHTNESS)) ;
#ifdef PCBX9D
#ifdef REVPLUS
		  lcd_puts_P(11*FW, y, XPSTR( "COLOUR"));
#endif
#endif
#ifdef PCB9XT
		  lcd_puts_P(11*FW, y, XPSTR( "RED"));
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
			 
			lcd_outdezAtt( PARAM_OFS+2*FW, y, 100-g_eeGeneral.bright, (sub==subN) ? blink : 0 ) ;
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
#ifdef PCBX9D
#ifdef REVPLUS
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
			y += FH ;
			subN += 1 ;

#if defined(REVPLUS) || defined(PCB9XT)
#ifndef PCB9XT
 			lcd_puts_Pleft(    y, PSTR(STR_BRIGHTNESS));
#endif
#ifdef PCB9XT
   		lcd_puts_P(11*FW, y, XPSTR( "GREEN"));
			oldBrightness = g_eeGeneral.bright_white ;
//			int8_t b ;
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
   		lcd_puts_P(11*FW, y, XPSTR( "WHITE"));
//#endif
//#ifdef PCB9XT
//			lcd_outdezAtt( PARAM_OFS+2*FW, y, 100-g_eeGeneral.bright_white, (sub==subN) ? blink : 0 ) ;
//#else
			lcd_outdezAtt( PARAM_OFS+2*FW, y, 100-g_eeGeneral.bright_white, (sub==subN) ? blink : 0 ) ;
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
			y += FH ;
			subN++;
#endif
#ifdef PCB9XT
   		lcd_puts_P( 11*FW, y, XPSTR( "BLUE"));
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
			y += FH ;
			subN++;
#endif
      
			lcd_puts_Pleft( y,PSTR(STR_LIGHT_SWITCH));
      putsDrSwitches(PARAM_OFS-FW,y,g_eeGeneral.lightSw,sub==subN ? blink : 0);
      if(sub==subN) CHECK_INCDEC_GENERALSWITCH(event, g_eeGeneral.lightSw, -MaxSwitchIndex, MaxSwitchIndex) ;
			y += FH ;
			subN++;

			for ( uint8_t i = 0 ; i < 2 ; i += 1 )
			{
				uint8_t b ;
    	  lcd_puts_Pleft( y,( i == 0) ? PSTR(STR_LIGHT_AFTER) : PSTR(STR_LIGHT_STICK) );
				b = ( i == 0 ) ? g_eeGeneral.lightAutoOff : g_eeGeneral.lightOnStickMove ;

    	  if(b) {
    	      lcd_outdezAtt(PARAM_OFS, y, b*5,LEFT|(sub==subN ? blink : 0));
    	      lcd_putc(Lcd_lastPos, y, 's');
    	  }
    	  else
    	      lcd_putsAtt(PARAM_OFS, y, PSTR(STR_OFF),(sub==subN ? blink:0));
    	  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( b, 600/5);
				if ( i == 0 )
				{
					g_eeGeneral.lightAutoOff = b ;
				}
				else
				{
					g_eeGeneral.lightOnStickMove = b ;
				}
  			y += FH ;
				subN += 1 ;
			}
		
#ifdef PCBSKY
#ifndef REVX
//	  	g_eeGeneral.optrexDisplay = onoffMenuItem( g_eeGeneral.optrexDisplay, y, XPSTR("Optrex Display"), sub, subN, event ) ;
			if ( g_eeGeneral.ar9xBoard == 0 )
			{
		  	g_eeGeneral.optrexDisplay = onoffMenuItem( g_eeGeneral.optrexDisplay, y, XPSTR("Optrex Display"), sub==subN ) ;
  			y += FH ;
				subN += 1 ;
			}
#endif
#endif

#ifndef PCB9XT
			g_eeGeneral.flashBeep = onoffMenuItem( g_eeGeneral.flashBeep, y, PSTR(STR_FLASH_ON_BEEP), sub==subN ) ;
  		y += FH ;
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
	  		y += FH ;
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
				y = FH ;
				g_eeGeneral.flashBeep = onoffMenuItem( g_eeGeneral.flashBeep, y, PSTR(STR_FLASH_ON_BEEP), sub==subN ) ;
  			y += FH ;
				subN += 1 ;
				b = g_eeGeneral.reverseScreen ;
	  		g_eeGeneral.reverseScreen = onoffMenuItem( b, y, PSTR(STR_REVERSE_SCREEN), sub==subN ) ;
				if ( g_eeGeneral.reverseScreen != b )
				{
					lcdSetOrientation() ;
				}
		 }
#endif
			 
		}
		break ;
		
		case M_AUDIO :
		{	
//			uint8_t y = 1*FH;
			uint8_t current_volume ;
			uint8_t subN = 0 ;
			IlinesCount = 7 ;
      
			TITLE( PSTR(STR_AudioHaptic) ) ;
  		lcd_puts_Pleft( y, PSTR(STR_VOLUME));
			current_volume = g_eeGeneral.volume ;
			lcd_outdezAtt( PARAM_OFS+2*FW, y, current_volume, (sub==subN) ? blink : 0 ) ;
  		if(sub==subN)
			{
				CHECK_INCDEC_H_GENVAR_0( current_volume,NUM_VOL_LEVELS-1);
				if ( current_volume != g_eeGeneral.volume )
				{
					setVolume( g_eeGeneral.volume = current_volume ) ;
				}
			}
  		y += FH ;
			subN += 1 ;
      
	    uint8_t b ;
  	  b = g_eeGeneral.beeperVal ;
      lcd_puts_Pleft( y,PSTR(STR_BEEPER));
      lcd_putsAttIdx(PARAM_OFS - FW - 4, y, PSTR(STR_BEEP_MODES),b,(sub==subN ? blink:0));
	    if(sub==subN) { CHECK_INCDEC_H_GENVAR_0( b, 6); g_eeGeneral.beeperVal = b ; }
  		y += FH ;
			subN += 1 ;

      g_eeGeneral.preBeep = onoffMenuItem( g_eeGeneral.preBeep, y, PSTR(STR_BEEP_COUNTDOWN), sub == subN ) ;
  		y += FH ;
			subN += 1 ;

      g_eeGeneral.minuteBeep = onoffMenuItem( g_eeGeneral.minuteBeep, y, PSTR(STR_MINUTE_BEEP), sub == subN ) ;
  		y += FH ;
			subN += 1 ;

	// audio start by rob
      lcd_puts_P(0, y,PSTR(STR_SPEAKER_PITCH));
      lcd_outdezAtt(PARAM_OFS+2*FW,y,g_eeGeneral.speakerPitch,(sub==subN ? blink : 0) );
      if(sub==subN) CHECK_INCDEC_H_GENVAR( g_eeGeneral.speakerPitch, 1, 100) ;
  		y += FH ;
			subN += 1 ;
	 
      lcd_puts_P(0, y,PSTR(STR_HAPTICSTRENGTH));
      lcd_outdezAtt(PARAM_OFS+2*FW,y,g_eeGeneral.hapticStrength,(sub==subN ? blink : 0) );
      if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.hapticStrength, 5) ;
  		y += FH ;
			subN += 1 ;

      lcd_puts_P(0, y,XPSTR("Haptic Min Run"));
      lcd_outdezAtt(PARAM_OFS+2*FW,y,g_eeGeneral.hapticMinRun+20,(sub==subN ? blink : 0) );
      if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.hapticMinRun, 20 ) ;
  		y += FH ;
			subN += 1 ;


		}
		break ;

		case M_ALARMS	:
		{
//			uint8_t y = 1*FH;
			uint8_t subN = 0 ;
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

	    if( g_eeGeneral.frskyinternalalarm == 1)
			{
				IlinesCount += 3 ;
			}

  		uint8_t attr ;
			TITLE( PSTR(STR_Alarms) ) ;
			
			if ( sub < NUM_ALARMS-1 )
			{
				displayNext() ;
  			attr = LEFT ;
      	lcd_puts_Pleft( y,PSTR(STR_BATT_WARN));
      	if(sub==subN) { attr = blink | LEFT ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatWarn, 40, 120); } //5-10V
      	putsVolts(PARAM_OFS, y, g_eeGeneral.vBatWarn, attr);
  			y += FH ;
				subN += 1 ;

  			attr = 0 ;//LEFT ;
      	lcd_puts_Pleft( y,PSTR(STR_INACT_ALARM));
      	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.inactivityTimer, -10, 110); } //0..120minutes
      	lcd_outdezAtt(PARAM_OFS+2*FW-2, y, g_eeGeneral.inactivityTimer+10, attr);
  			y += FH ;
				subN += 1 ;

	  		lcd_puts_P( FW, y, PSTR(STR_VOLUME) ) ;
				int8_t value = g_eeGeneral.inactivityVolume + ( NUM_VOL_LEVELS-3 ) ;
				lcd_outdezAtt( PARAM_OFS+2*FW, y, value, (sub==subN) ? blink : 0 ) ;
	  		if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( value,NUM_VOL_LEVELS-1);
					g_eeGeneral.inactivityVolume = value - ( NUM_VOL_LEVELS-3 ) ;
				} 	
	  		y += FH ;
				subN += 1 ;
      
#ifdef PCBSKY
				if ( g_eeGeneral.ar9xBoard == 0 )
				{
  				lcd_puts_Pleft( y, PSTR(STR_CAPACITY_ALARM));
					lcd_outdezAtt( PARAM_OFS+2*FW, y, g_eeGeneral.mAh_alarm*50, (sub==subN) ? blink : 0 ) ;
  				if(sub==subN)
					{
						CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.mAh_alarm,100);
					}
 					y += FH ;
					subN += 1 ;
				}
#endif
		
      	uint8_t b = g_eeGeneral.disableThrottleWarning;
      	g_eeGeneral.disableThrottleWarning = offonMenuItem( b, y, PSTR(STR_THR_WARNING), sub == subN ) ;
 				y += FH ;
				subN += 1 ;

      	b = g_eeGeneral.disableAlarmWarning;
      	g_eeGeneral.disableAlarmWarning = offonMenuItem( b, y, PSTR(STR_ALARM_WARN), sub == subN ) ;
 				y += FH ;
				subN += 1 ;
			}
			else
			{
				subN = NUM_ALARMS-1 ;
      	g_eeGeneral.frskyinternalalarm = onoffMenuItem( g_eeGeneral.frskyinternalalarm, y, PSTR(STR_INT_FRSKY_ALRM), sub == subN ) ;
 				y += FH ;
				subN += 1 ;
				    
	    	if( g_eeGeneral.frskyinternalalarm == 1)
				{
					for ( uint8_t i = 0 ; i < 3 ; i += 1 )
					{
						uint8_t b ;
					
						b = g_eeGeneral.FRSkyYellow ;    // Done here to stop a compiler warning
								
						if ( i == 0 )
						{
						  lcd_puts_P(0, y,PSTR(STR_ALERT_YEL));
						}
						else if ( i == 1 )
						{
						  b = g_eeGeneral.FRSkyOrange ;
						  lcd_puts_P(0, y,PSTR(STR_ALERT_ORG));
						}
						else if ( i == 2 )
						{
						  b = g_eeGeneral.FRSkyRed ;
						  lcd_puts_P(0, y,PSTR(STR_ALERT_RED));
						}
//						if((g_eeGeneral.speakerMode & 1) == 1)
//						{
							lcd_putsAttIdx(PARAM_OFS - FW - 4, y, PSTR(STR_SOUNDS),b,(sub==subN ? blink:0));
//						}
						if(sub==subN)
						{
							CHECK_INCDEC_H_GENVAR_0( b, 15);
							if ( i == 0 )
							{
								g_eeGeneral.FRSkyYellow = b ;
							}
							else if ( i == 1 )
							{
								g_eeGeneral.FRSkyOrange = b ;
							}
							else if ( i == 2 )
							{
								g_eeGeneral.FRSkyRed = b ;
							}
							if (checkIncDec_Ret)
							{
								audio.event( b, 0, 1 ) ;
							}
						}
				 		y += FH ;
						subN += 1 ;
					}
				}			  
			}
		}
		break ;

		case M_GENERAL :
		{
			uint8_t subN = 0 ;
#if defined(PCBSKY) || defined(PCB9XT)
			IlinesCount = 8 ;
#else
			IlinesCount = 7 ;
#endif
			TITLE( XPSTR(Str_General) ) ;

#if defined(PCBSKY) || defined(PCB9XT)
		 if ( sub < 6 )
		 {
			displayNext() ;
#endif

//			lcd_puts_Pleft( y, PSTR(STR_NAME)) ;
			SubMenuCall = 0x80 ;
			alphaEditName( 10*FW-2, y, (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName), sub==subN, (uint8_t *)XPSTR( "Owner Name") ) ;
//			if ( sub == 0 )
//			{
//				Columns = sizeof(g_eeGeneral.ownerName)-1 ;
//			}
//			editName( 11*FW-2, g_posHorz, y, (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName), (sub==subN) ? EE_GENERAL : 0, event ) ;
 			y += FH ;
			subN += 1 ;

	    lcd_puts_Pleft( y,PSTR(STR_LANGUAGE));
#ifdef SMALL
	    lcd_putsAttIdx( 10*FW, y, XPSTR("\012   ENGLISH  FRANCAIS   DEUTSCH"),g_eeGeneral.language,(sub==subN ? blink:0));
	    if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.language, 2 ) ;
#else
	    lcd_putsAttIdx( 10*FW, y, XPSTR("\012   ENGLISH  FRANCAIS   DEUTSCH NORWEGIAN   SWEDISH   ITALIAN    POLISHVIETNAMESE   SPANISH"),g_eeGeneral.language,(sub==subN ? blink:0));
	    if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.language, 8 ) ;
#endif
//	    if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.language, 8 ) ;
			setLanguage() ;
 			y += FH ;
			subN += 1 ;

      uint8_t b = g_eeGeneral.disableSplashScreen;
      g_eeGeneral.disableSplashScreen = offonMenuItem( b, y, PSTR(STR_SPLASH_SCREEN), sub == subN ) ;
 			y += FH ;
			subN += 1 ;

      b = g_eeGeneral.hideNameOnSplash;
      g_eeGeneral.hideNameOnSplash = offonMenuItem( b, y, PSTR(STR_SPLASH_NAME), sub == subN ) ;
 			y += FH ;
			subN += 1 ;

      b = 1-g_eeGeneral.disablePotScroll ;
			b |= g_eeGeneral.stickScroll << 1 ;
      lcd_puts_Pleft( y,PSTR(STR_SCROLLING));
      lcd_putsAttIdx(PARAM_OFS-2*FW, y, XPSTR("\005 NONE  POTSTICK BOTH"),b,(sub==subN ? blink:0));
      if(sub==subN) CHECK_INCDEC_H_GENVAR_0( b, 3 ) ;
			g_eeGeneral.stickScroll = b >> 1 ;
			g_eeGeneral.disablePotScroll = 1 - ( b & 1 ) ;
 			y += FH ;
			subN += 1 ;

      b = g_eeGeneral.forceMenuEdit ;
      g_eeGeneral.forceMenuEdit = onoffMenuItem( b, y, PSTR(STR_MENU_ONLY_EDIT), sub == subN ) ;
 			y += FH ;
			subN += 1 ;

#if defined(PCBSKY) || defined(PCB9XT)
		 }
		 else
		 {
		 	subN = 6 ;
  		lcd_puts_Pleft( y, PSTR(STR_BT_BAUDRATE));
  		lcd_putsAttIdx(  PARAM_OFS-3*FW, y, XPSTR(BtBaudString),g_eeGeneral.bt_baudrate,(sub==subN ? blink:0));
  		if(sub==subN)
			{
//				uint8_t b = g_eeGeneral.bt_baudrate ;
				CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.bt_baudrate,4);
//				if ( b != g_eeGeneral.bt_baudrate )
//				{
//					uint32_t baudrate = 115200 ;
//					if ( b )
//					{
//						baudrate = ( b == 1 ) ? 9600 : ( ( b == 2 ) ? 19200 : 57600 ) ;
//					}
//					BtBaudrateChanged = 1 ;
//					UART3_Configure( baudrate, Master_frequency ) ;
//				}
			}
 			y += FH ;
			subN += 1 ;
#endif
  		lcd_puts_Pleft( y, XPSTR("GPS Format") ) ;
  		lcd_putsAttIdx(  12*FW, y, XPSTR("\011DDmm.mmmmDD.dddddd"),g_eeGeneral.gpsFormat,(sub==subN ? blink:0));
  		if(sub==subN)
			{
				CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.gpsFormat,1) ;
			}
 			y += FH ;
			subN += 1 ;

#if defined(PCBSKY) || defined(PCB9XT)
  	 }	
//			uint8_t attr ;
//			int32_t value ;
//			attr = (sub==subN) ? blink : 0 ;
//  		lcd_puts_Pleft( y, XPSTR("BT skew"));
//			value = g_eeGeneral.bt_correction ;
//			if ( value > 7 )
//			{
//				value -= 16 ;
//			}
//	  	lcd_outdezAtt( PARAM_OFS, y, value, attr ) ;
//			if ( attr )
//			{
//				CHECK_INCDEC_H_GENVAR( value, -7, 7 ) ;
//				if ( value < 0 )
//				{
//					value += 16 ;
//				}
//				g_eeGeneral.bt_correction = value ;
//			}
#endif

		}			 
		break ;

		case M_CONTROLS :
		{
//			uint8_t y = 1*FH;
			uint8_t subN = 0 ;
			IlinesCount = 8 ;
			TITLE( PSTR(STR_Controls) ) ;

			if ( sub < 4 )
			{
				displayNext() ;
				uint8_t attr = sub==subN ? blink : 0 ;
	  		lcd_puts_Pleft( y, PSTR(STR_CHAN_ORDER) ) ;//   RAET->AETR
				uint8_t bch = bchout_ar[g_eeGeneral.templateSetup] ;
    		for ( uint8_t i = 4 ; i > 0 ; i -= 1 )
				{
					uint8_t letter ;
					letter = *(PSTR(STR_SP_RETA) +(bch & 3) + 1 ) ;
  				lcd_putcAtt( (16+i)*FW, y, letter, attr ) ;
					bch >>= 2 ;
				}
	  		if(attr) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.templateSetup, 23 ) ;
 				y += FH ;
				subN += 1 ;
      
				g_eeGeneral.crosstrim = onoffMenuItem( g_eeGeneral.crosstrim, y, PSTR(STR_CROSSTRIM), sub == subN ) ;
 				y += FH ;
				subN += 1 ;

				uint8_t oldValue = g_eeGeneral.throttleReversed ;
				g_eeGeneral.throttleReversed = onoffMenuItem( oldValue, y, PSTR(STR_THR_REVERSE), sub == subN ) ;
				if ( g_eeGeneral.throttleReversed != oldValue )
				{
  				checkTHR() ;
				}
 				y += FH ;
				subN += 1 ;

	    	lcd_puts_Pleft( y, PSTR(STR_MODE) );
				for ( uint8_t i = 0 ; i < 4 ; i += 1 )
				{
					lcd_img((6+4*i)*FW, y, sticks, i, 0 ) ;
				}
 				y += FH ;
    
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
							g_eeGeneral.stickScroll = 0 ;
							g_eeGeneral.stickMode = mode ;							
						}
					}
				}
      	lcd_putcAtt( 3*FW, y, '1'+g_eeGeneral.stickMode,attr);
//	#ifdef FIX_MODE
      	for(uint8_t i=0; i<4; i++) putsChnRaw( (6+4*i)*FW, y, modeFixValue( i ), 0 ) ;//sub==3?INVERS:0);
//	#else
//      	for(uint8_t i=0; i<4; i++) putsChnRaw( (6+4*i)*FW, y,i+1,0);//sub==3?INVERS:0);
//	#endif
 				y += FH ;
				subN += 1 ;
			}
			else
			{
				subN = 4 ;
				if ( sub >= IlinesCount )
				{
					sub = mstate.m_posVert = IlinesCount-1 ;
				}
				// Edit custom stick names
//				Columns = 3 ;
      	for(uint8_t i=0; i<4; i++)
				{
      		lcd_putsAttIdx( FW*5, y, PSTR(STR_STICK_NAMES), i, 0 ) ;
//					editName( 11*FW, g_posHorz, y, &g_eeGeneral.customStickNames[i*4], 4, sub==subN ? EE_GENERAL : 0, event ) ;
					if ( sub == subN )
					{
						SubMenuCall = 0x80 + i + 5 ;
					}
					alphaEditName( 11*FW, y, &g_eeGeneral.customStickNames[i*4], 4, sub==subN, (uint8_t *)&PSTR(STR_STICK_NAMES)[i*5+1] ) ;
	 				y += FH ;
					subN += 1 ;
				}
			}
		}			 
		break ;


		case M_HARDWARE :
		{
			uint8_t subN = 0 ;
#ifdef PCBSKY
			uint8_t num = 9 + 2 + 2 + 4 + 1 + 1 + 4 ;
#ifndef REVX
			num += 1 ;
#endif
			IlinesCount = num ;
#else
#ifdef PCB9XT
			IlinesCount = 4 + 4 + 1 + 7 + 5 ;
#else
			IlinesCount = 6 + 4 + 1 + 1 ;
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
#endif

#ifdef PCB9XT
			uint32_t page2 = 8 ;
			uint32_t page3 = 9 ;
			if ( g_eeGeneral.analogMapping & MASK_6POS )
			{
				IlinesCount += 6 ;
				page3 += 6 ;
			}
#endif

			TITLE( PSTR(STR_Hardware) ) ;
			if ( HardwareMenuEnabled == 0 )
			{
				IlinesCount = 0 ;
        lcd_puts_Pleft( 3*FH,XPSTR("\006Disabled"));
				break ;
			}
			
#ifdef PCBSKY
			if ( sub < 6 )
#endif
#ifdef PCBX9D
			if ( sub < 6 )
#endif
#ifdef PCB9XT
			if ( sub < 4 )
#endif
			{
				displayNext() ;
        lcd_puts_Pleft( y,PSTR(STR_FILTER_ADC));
        lcd_putsAttIdx(PARAM_OFS, y, XPSTR("\004SINGOSMPFILT"),g_eeGeneral.filterInput,(sub==subN ? blink:0));
        if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.filterInput, 2);
 				y += FH ;
				subN += 1 ;

  			lcd_puts_Pleft( y, PSTR(STR_ROTARY_DIVISOR));
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\001142"),g_eeGeneral.rotaryDivisor,(sub==subN ? BLINK:0));
  			if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.rotaryDivisor,2);
				}
 				y += FH ;
				subN += 1 ;

#ifdef PCB9XT
		 		lcd_puts_Pleft( y, XPSTR( "I2C Function") ) ;
//				g_eeGeneral.enableI2C = onoffMenuItem( g_eeGeneral.enableI2C, y, XPSTR("I2C Enable"), (sub==subN ? blink:0) ) ;
				uint8_t oldValue = g_eeGeneral.enableI2C ;
				g_eeGeneral.enableI2C = checkIndexed( y, XPSTR(FWx15"\002""\004 Off I2CCOM3"), g_eeGeneral.enableI2C, (sub==subN) ? blink:0 ) ;
				if ( oldValue != g_eeGeneral.enableI2C )
				{
					// Change function
				}
 				y += FH ;
				subN += 1 ;

		 		lcd_puts_Pleft( y, XPSTR( "Bt Com Port") ) ;
				g_eeGeneral.btComPort = checkIndexed( y, XPSTR(FWx15"\002""\004NONECOM2COM3"), g_eeGeneral.btComPort, (sub==subN) ? blink:0 ) ;
 				y += FH ;
				subN += 1 ;
#endif

// testing for speaker hiss only
//#ifdef PCB9XT
//				static uint8_t dacMode = 0 ;
//				uint8_t oldDacMode = dacMode ;
//  			lcd_puts_Pleft( y, XPSTR("DAC"));
//	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\006Normal   Off    0V"),dacMode,(sub==subN ? BLINK:0));
//  			if(sub==subN)
//				{
//					dacMode = checkIncDec( dacMode, 0, 2, 0 ) ;
//				}
//				if ( oldDacMode != dacMode )
//				{
//					switch ( dacMode )
//					{
//						case 0 :
//							DAC->DHR12R1 = 2010 ;
//							DAC->CR = DAC_CR_TEN1 | DAC_CR_EN1 ;			// Enable DAC
//						break ;
//						case 1 :
//							DAC->CR = 0 ;			// Disable DAC
//						break ;
//						case 2 :
//							DAC->DHR12R1 = 0 ;
//							DAC->CR = DAC_CR_TEN1 | DAC_CR_EN1 ;			// Enable DAC
//						break ;
//					}
//				}
// 				y += FH ;
//				subN += 1 ;
//#endif


// If X9Dplus, allow configuration of S3 as POT or 6-pos switch

#ifdef PCBX9D

#ifdef REVPLUS
#define NUM_MAP_POTS	3
#else
#define NUM_MAP_POTS	2
#endif

				uint32_t value ;
				uint32_t oldValue ;
#ifndef REV9E
  			lcd_puts_Pleft( y, XPSTR("Encoder"));
				value = g_eeGeneral.analogMapping & ENC_MASK ;
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\002--P1P2P3"), value, (sub==subN ? BLINK:0));
				oldValue = value ;
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_MAP_POTS ) ;
 				g_eeGeneral.analogMapping = ( g_eeGeneral.analogMapping & ~ENC_MASK ) | value ;
				if ( value != oldValue )
				{
					init_adc2() ;
				}
				y += FH ;
				subN += 1 ;
#endif	// nREV9E

  			lcd_puts_Pleft( y, XPSTR("6 Pos. Switch"));
				value = ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2 ;
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\002--P1P2P3"), value, (sub==subN ? BLINK:0));
				oldValue = value ;
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_MAP_POTS ) ;
 				g_eeGeneral.analogMapping = ( g_eeGeneral.analogMapping & ~MASK_6POS ) | ( value << 2 ) ;
				if ( value != oldValue )
				{
					createSwitchMapping() ;
				}
				y += FH ;
				subN += 1 ;

  			lcd_puts_Pleft( y, XPSTR("PB1 switch\037PB2 switch"));
				g_eeGeneral.pb1source = edit3posSwitchSource( y, g_eeGeneral.pb1source, USE_PB1, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;
				g_eeGeneral.pb2source = edit3posSwitchSource( y, g_eeGeneral.pb2source, USE_PB2, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;


#endif	// PCBX9D

#ifdef PCBSKY
				uint8_t lastValue ;
				uint8_t b ;
				lastValue = g_eeGeneral.stickGain ;
  			lcd_puts_Pleft( y, PSTR(STR_STICK_LV_GAIN));
				b = g_eeGeneral.stickGain & STICK_LV_GAIN ? 1 : 0 ;
    		lcd_putcAtt( 15*FW, y, b ? '2' : '1', sub==subN ? BLINK:0 );
  			if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( b,1);
					g_eeGeneral.stickGain = (lastValue & ~STICK_LV_GAIN) | (b ? STICK_LV_GAIN : 0 ) ;
				}
 				y += FH ;
				subN += 1 ;
  		
				lcd_puts_Pleft( y, PSTR(STR_STICK_LH_GAIN));
				b = g_eeGeneral.stickGain & STICK_LH_GAIN ? 1 : 0 ;
    		lcd_putcAtt( 15*FW, y, b ? '2' : '1', sub==subN ? BLINK:0 );
  			if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( b,1);
					g_eeGeneral.stickGain = (lastValue & ~STICK_LH_GAIN) | (b ? STICK_LH_GAIN : 0 ) ;
				}
 				y += FH ;
				subN += 1 ;

  			lcd_puts_Pleft( y, PSTR(STR_STICK_RV_GAIN));
				b = g_eeGeneral.stickGain & STICK_RV_GAIN ? 1 : 0 ;
    		lcd_putcAtt( 15*FW, y, b ? '2' : '1', sub==subN ? BLINK:0 );
  			if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( b,1);
					g_eeGeneral.stickGain = (lastValue & ~STICK_RV_GAIN) | (b ? STICK_RV_GAIN : 0 ) ;
				}
 				y += FH ;
				subN += 1 ;

  			lcd_puts_Pleft( y, PSTR(STR_STICK_RH_GAIN));
				b = g_eeGeneral.stickGain & STICK_RH_GAIN ? 1 : 0 ;
    		lcd_putcAtt( 15*FW, y, b ? '2' : '1', sub==subN ? BLINK:0 );
  			if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( b,1);
					g_eeGeneral.stickGain = (lastValue & ~STICK_RH_GAIN) | (b ? STICK_RH_GAIN : 0 ) ;
				}
				if ( lastValue != g_eeGeneral.stickGain )
				{
					set_stick_gain( g_eeGeneral.stickGain ) ;
				}
#endif // PCBSKY
			}
			
			
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

#ifdef PCBX9D
			else if ( sub < 11 )
			{
				subN = 6 ;
#endif // PCBX9D
				uint8_t attr = 0 ;
	  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_eeGeneral.stickDeadband[0], 15 ) ; }
				lcd_xlabel_decimal( 20*FW, y, g_eeGeneral.stickDeadband[0], attr, XPSTR("Stick LH deadband") ) ;
  			y += FH ;
  			subN++;

				attr = 0 ;
	  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_eeGeneral.stickDeadband[1], 15 ) ; }
				lcd_xlabel_decimal( 20*FW, y, g_eeGeneral.stickDeadband[1], attr, XPSTR("Stick LV deadband") ) ;
  			y += FH ;
  			subN++;

				attr = 0 ;
	  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_eeGeneral.stickDeadband[2], 15 ) ; }
				lcd_xlabel_decimal( 20*FW, y, g_eeGeneral.stickDeadband[2], attr, XPSTR("Stick RV deadband") ) ;
  			y += FH ;
  			subN++;
				attr = 0 ;
	  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_eeGeneral.stickDeadband[3], 15 ) ; }
				lcd_xlabel_decimal( 20*FW, y, g_eeGeneral.stickDeadband[3], attr, XPSTR("Stick RH deadband") ) ;
  			y += FH ;
  			subN++;

#if defined(PCBSKY) || defined(PCBX9D)
				{
					uint8_t lastValue = g_eeGeneral.softwareVolume ;
			  	g_eeGeneral.softwareVolume = onoffMenuItem( lastValue, y, XPSTR("Software Vol."), sub==subN ) ;
					if ( lastValue != g_eeGeneral.softwareVolume )
					{
						setVolume( g_eeGeneral.volume ) ;
					}
				}
	  		y += FH ;
				subN += 1 ;
#endif

#ifdef PCBSKY
#ifndef REVX
		  	g_eeGeneral.ar9xBoard = onoffMenuItem( g_eeGeneral.ar9xBoard, y, XPSTR("AR9X Board"), sub==subN ) ;
	  		y += FH ;
				subN += 1 ;
#endif
#endif

#ifdef PCBSKY
				if ( g_eeGeneral.ar9xBoard )
				{
					lcd_puts_Pleft( y, XPSTR("RTC") );
					g_eeGeneral.externalRtcType = checkIndexed( y, XPSTR("\052""\001""\006  NONEDS3231"), g_eeGeneral.externalRtcType, (sub==subN) ) ;// FWx07
	  			y += FH ;
					subN += 1 ;
				}
#endif
			
			}
#if defined(PCBSKY) || defined(PCB9XT)
			else if ( sub < page3 )
			{
				uint32_t value ;
				uint32_t oldValue ;
				subN = page2 ;
  			lcd_puts_Pleft( y, XPSTR("6 Pos. Switch"));
				value = ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2 ;
#ifdef PCBSKY
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\003---P1 P2 P3 AUX"), value, (sub==subN ? INVERS:0));
#define NUM_ANA_INPUTS	4
#endif
#ifdef PCB9XT
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\003---P1 P2 P3 "), value, (sub==subN ? INVERS:0));
#define NUM_ANA_INPUTS	3
#endif
				oldValue = value ;
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_ANA_INPUTS ) ;
 				g_eeGeneral.analogMapping = ( g_eeGeneral.analogMapping & ~MASK_6POS ) | ( value << 2 ) ;
				if ( value != oldValue )
				{
					createSwitchMapping() ;
				}
				y += FH ;
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
					lcd_outhex4( 12*FW, 0*FH, value ) ;
					lcd_putsAttIdx( 17*FW, 0*FH, PSTR(SWITCHES_STR), HSW_Ele6pos0 + switchPosition(HSW_Ele6pos0)-HSW_OFFSET, 0 ) ;

					for ( i = 0 ; i < 6 ; i += 1 )
					{
  					lcd_puts_Pleft( y, XPSTR("6P0:"));
  					lcd_putc( 2*FW, y, i+'0' ) ;
						if ( i < 5 )
						{
							lcd_outhex4( 12*FW, y, SixPositionTable[i] ) ;
						}
						lcd_outhex4( 5*FW, y, g_eeGeneral.SixPositionCalibration[i] ) ;
						if ( sub == subN )
						{
							lcd_char_inverse( 5*FW, y, 20, 0 ) ;
							if ( Tevent==EVT_KEY_BREAK(KEY_MENU) ) // || Tevent == EVT_KEY_BREAK(BTN_RE)  )
					    {
								g_eeGeneral.SixPositionCalibration[i] = value ;
								s_editMode = 0 ;
								create6posTable() ;
							}	
						}
						y += FH ;
						subN += 1 ;
					}
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
  			lcd_puts_Pleft( y, XPSTR("ELE switch"));
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
 				y += FH ;
				subN++;

				{
				  lcd_puts_Pleft( y, XPSTR("THR switch"));
					g_eeGeneral.thrsource = edit3posSwitchSource( y, g_eeGeneral.thrsource, USE_THR_3POS, (sub==subN), 1 ) ;
				}	
 				y += FH ;
				subN++;

					
				{
				  lcd_puts_Pleft( y, XPSTR("RUD switch"));
					g_eeGeneral.rudsource = edit3posSwitchSource( y, g_eeGeneral.rudsource, USE_RUD_3POS, (sub==subN), 1 ) ;
				}	
 				y += FH ;
				subN++;
					
				{
				  lcd_puts_Pleft( y, XPSTR("AIL switch"));
					g_eeGeneral.ailsource = edit3posSwitchSource( y, g_eeGeneral.ailsource, USE_AIL_3POS, (sub==subN), 1 ) ;
				}	
 				y += FH ;
				subN++;
					
				{
				  lcd_puts_Pleft( y, XPSTR("GEA switch"));
					g_eeGeneral.geasource = edit3posSwitchSource( y, g_eeGeneral.geasource, USE_GEA_3POS, (sub==subN), 1 ) ;
				}	
					
	 				y += FH ;
					subN++;
				
  			lcd_puts_Pleft( y, XPSTR("PB1 switch\037PB2 switch"));
				g_eeGeneral.pb1source = edit3posSwitchSource( y, g_eeGeneral.pb1source, USE_PB1, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;
				g_eeGeneral.pb2source = edit3posSwitchSource( y, g_eeGeneral.pb2source, USE_PB2, (sub==subN), 0 ) ;
			}
#endif // PCBSKY
			else
			{
#if defined(PCBSKY) || defined(PCB9XT)
				subN = page3+7 ;
#else
				subN = 11 ;
#endif

#if defined(PCBSKY) || defined(PCB9XT)
  			lcd_puts_Pleft( y, XPSTR("PB3 switch\037PB4 switch"));
				g_eeGeneral.pb3source = edit3posSwitchSource( y, g_eeGeneral.pb3source, USE_PB3, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;
				g_eeGeneral.pb4source = edit3posSwitchSource( y, g_eeGeneral.pb4source, USE_PB4, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;
#endif

#if defined(PCBSKY) || defined(PCB9XT)
#ifdef PCB9XT
				editExtraPot( y, 0, uint8_t (sub==subN) ) ;
 				y += FH ;
				subN++;
#endif
#ifdef PCBSKY
  			lcd_puts_Pleft( y, XPSTR("POT4"));
				uint8_t value = g_eeGeneral.extraPotsSource[0] ;
				if ( g_eeGeneral.ar9xBoard == 0 )
				{
					value = checkIndexed( y, XPSTR(FWx17"\001\004NONEAD10"), value, (sub==subN) ) ;
				}
				else
				{
					value = checkIndexed( y, XPSTR(FWx17"\002\004NONEAD10AD8 "), value, (sub==subN) ) ;
				}
				g_eeGeneral.extraPotsSource[0] = value ;
				NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
 				y += FH ;
				subN++;
#endif

#ifdef PCB9XT
				editExtraPot( y, 1, uint8_t (sub==subN) ) ;
 				y += FH ;
				subN++;
#else
				if ( g_eeGeneral.ar9xBoard )
				{
	  			lcd_puts_Pleft( y, XPSTR("POT5"));
					value = g_eeGeneral.extraPotsSource[1] ;
					value = checkIndexed( y, XPSTR(FWx17"\002\004NONEAD10AD8 "), value, (sub==subN) ) ;
					g_eeGeneral.extraPotsSource[1] = value ;
					NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
 					y += FH ;
					subN++;
				}
#endif	// PCB9XT

#endif // PCBSKY !! PCB9XT
        
				lcd_puts_Pleft( y,XPSTR("Stick Reverse:"));
 				y += FH ;
				for ( uint8_t i = 0 ; i < 4 ; i += 1 )
				{
					lcd_img((6+4*i)*FW, y, sticks, i, 0 ) ;
					if (g_eeGeneral.stickReverse & (1<<i)) lcd_char_inverse( (6+4*i)*FW, y, 3*FW, 0 ) ;
				}
    		if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.stickReverse, 15 ) ;
//					plotType = PLOT_BLACK ;
					lcd_rect( 6*FW-1, y-1, 15*FW+2, 9 ) ;
//					plotType = PLOT_XOR ;
				}
 				y += FH ;
				subN += 1 ;
			}
		}			 
		break ;

		case M_EEPROM :
		{
			uint8_t subN = 0 ;
			
			IlinesCount = 2 ;

			TITLE( XPSTR("EEPROM") ) ;
			if ( HardwareMenuEnabled == 0 )
			{
				IlinesCount = 0 ;
        lcd_puts_Pleft( 3*FH,XPSTR("\006Disabled"));
				break ;
			}
      lcd_putsAtt( 0, y, XPSTR("Backup EEPROM"), (sub==subN) ? INVERS : 0 ) ;
			y += FH ;
			subN += 1 ;
      lcd_putsAtt( 0, y,XPSTR("Restore EEPROM"), (sub==subN) ? INVERS : 0 ) ;

  		lcd_puts_Pleft( 4*FH, XPSTR("\003IN DEVELOPMENT"));

			if ( Tevent==EVT_KEY_BREAK(KEY_MENU) ) // || Tevent == EVT_KEY_BREAK(BTN_RE)  )
			{
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

//#define BT_COMMAND_DEBUG	1
//static uint8_t BtCommandMode ;
#define BT_TYPE_HC06		0
#define BT_TYPE_HC05		1

		case M_BLUETOOTH :
		{	
			uint8_t subN = 0 ;
			
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
			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
			{
				IlinesCount += 3 ;
			}
			lcd_puts_Pleft( y,XPSTR("\013Type"));
			g_eeGeneral.BtType = checkIndexed( y, XPSTR(FWx16"\001""\005HC-06HC-05"), g_eeGeneral.BtType, (sub==subN) ) ;
			y += FH ;
			subN += 1 ;

#ifdef BLUETOOTH
	EditType = EE_MODEL ;
			lcd_puts_Pleft( y, XPSTR("BT as Trainer") );
			g_model.BTfunction = checkIndexed( y, XPSTR(FWx16"\002""\004 OFF  RxTxRx"), g_model.BTfunction, (sub==subN) ) ;
//			if ( (sub == subN) )
//			{
//				CHECK_INCDEC_H_MODELVAR_0( g_model.BTfunction, 2 ) ;
//			}
//  		menu_lcd_onoff( PARAM_OFS, y, g_model.BTfunction, sub==subN ) ;
			y += FH ;
			subN += 1 ;
	EditType = EE_GENERAL ;

// #ifdef BT_COMMAND_DEBUG


extern uint8_t BtName[] ;
extern uint8_t BtNameChange ;

			uint8_t attr = (sub == subN) ? INVERS : 0 ;
//			lcd_puts_Pleft( y, XPSTR("BT Name") ) ;
//			lcd_putsAtt( 7*FW, y, ( g_eeGeneral.BtType == BT_TYPE_HC05 ) ? (char *)BtName: (char *)g_eeGeneral.btName, attr ) ;
				
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
					if ( BtNameChange == 0 )
					{
						BtNameChange = 0x80 ;
					}
					AlphaEdited = 0 ;
				}
			}

			y += FH ;
			subN += 1 ;

extern uint8_t BtRoleChange ;
extern uint8_t BtMasterSlave ;
			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
			{
				lcd_puts_Pleft( y, XPSTR("BT Role") ) ;
	    	lcd_putsAttIdx( 12*FW, y, XPSTR("\007UNKNOWNSLAVE  MASTER "), BtMasterSlave, (sub==subN) ? blink:0 ) ;
				uint8_t newRole = 0 ;
				if ( BtMasterSlave == 2 )
				{
					newRole = 1 ;
				}
				if ( (sub == subN) )
				{
					newRole = checkIncDec( newRole, 0, 1, 0 ) ;
					if ( newRole+1 != BtMasterSlave )
					{
						if ( BtRoleChange == 0 )
						{
							BtRoleChange = 0x80 | newRole ;
						}
					}
				}
//			if ( (sub == subN) )
//			{
//				BtCommandMode = checkIncDec( BtCommandMode, 0, 1, 0 ) ;
//			}
//  		menu_lcd_onoff( PARAM_OFS, y, BtCommandMode, sub==subN ) ;

//#ifndef PCB9XT
//			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
//			{
//				if ( BtCommandMode )
//				{
//					PIOB->PIO_SODR = PIO_PB12 ;		// Set bit B12 HIGH
//				}
//				else
//				{
//					PIOB->PIO_CODR = PIO_PB12 ;		// Set bit B12 LOW
//				}
//			}
//#endif // nPCB9XT
				y += FH ;
				subN += 1 ;
// #endif
			}
			
			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
			{
				EditType = EE_MODEL ;
				g_model.autoBtConnect = onoffMenuItem( g_model.autoBtConnect, y, XPSTR("Auto Connect"), sub==subN ) ;
				subN += 1 ;
				y += FH ;				 
				uint8_t attr = (sub == subN) ? INVERS : 0 ;
				lcd_putsAtt( 0, y, XPSTR("Scan"), attr ) ; 
				if ( attr )
				{
					if ( event == EVT_KEY_BREAK(KEY_MENU) )
					{
						s_editMode = 0 ;
						SubMenuCall = 0x85 ;
    		   	pushMenu( menuProcBtScan ) ;
					}
				}
				EditType = EE_GENERAL ;
				subN += 1 ;
				attr = (sub == subN) ? INVERS : 0 ;
				lcd_putsAtt( 10*FW, y, XPSTR("Configure"), attr ) ; 
				if ( attr )
				{
					if ( event == EVT_KEY_BREAK(KEY_MENU) )
					{
						s_editMode = 0 ;
						SubMenuCall = 0x86 ;
    		   	pushMenu( menuProcBtConfigure ) ;
					}
				}
				subN += 1 ;
				y += FH ;				 

//extern uint8_t BtMode[] ;
extern uint8_t BtPswd[] ;
//extern uint8_t BtRole[] ;
//extern uint8_t BtIac[] ;
				
//				lcd_puts_Pleft( FH*6, XPSTR("Name\037Pswd\013Br")) ;

				attr = (sub == subN) ? INVERS : 0 ;
				lcd_putsAtt( 10*FW, y, XPSTR("List"), attr ) ; 
				
				if ( attr )
				{
					if ( event == EVT_KEY_BREAK(KEY_MENU) )
					{
						s_editMode = 0 ;
						SubMenuCall = 0x87 ;
    			  pushMenu( menuProcBtAddressList ) ;
					}
				}

				lcd_puts_Pleft( FH*7, XPSTR("Pswd\013Br")) ;
//				lcd_putsAtt( 6*FW, 5*FH, (char *)BtRole, 0 ) ;
//				lcd_putsAtt( 5*FW, 7*FH, (char *)BtName, 0 ) ;
				lcd_putsAtt( 5*FW, 7*FH, (char *)BtPswd, 0 ) ;
  			lcd_putsAttIdx(  14*FW, 7*FH, XPSTR(BtBaudString), BtCurrentBaudrate, 0 ) ;

//				lcd_putsAtt( 12*FW, 4*FH, (char *)BtIac, 0 ) ;
			}
#endif

		}	
		break ;

	}
}

//const char Str_Mixer[] =      "Mixer" ;
//const char Str_Cswitches[] =  "L.Switches" ;
//const char Str_Telemetry[] =  "Telemetry" ;
//const char Str_limits[] =     "Limits" ;
//const char Str_Bluetooth[] =  "BlueTooth" ;
//const char Str_heli_setup[] = "Heli" ;
//const char Str_Curves[] =     "Curves" ;
//const char Str_Expo[] =		    "Expo/Dr" ;
//const char Str_Globals[] =    "Globals" ;
//const char Str_Timer[] =      "Timers" ;
//const char Str_Modes[] =	 		"Modes" ;
//const char Str_Voice[] =      "Voice" ;
//const char Str_Protocol[] =   "Protocol" ;
//const char Str_Safety[] =			"Safety Sws" ;

#define M_MINDEX			0
#define M_MIXER				1
#define M_HELI				2
#define M_LIMITS			3
#define M_EXPO				4
#define M_MODES				5
#define M_CURVE				6
#define M_SWITCHES		7
#define M_MUSIC				8
#define M_SAFETY			9
#define M_GLOBALS			10
#define M_TELEMETRY		11
#define M_VOICE				12
#define M_TIMERS			13
#define M_MGENERAL		14
#define M_PROTOCOL		15

void menuProcModelIndex(uint8_t event)
{
	static MState2 mstate ;
	EditType = EE_MODEL ;
	if ( PopupData.PopupActive == 0 )
	{
		event = indexProcess( event, &mstate, 8 ) ;
		event = mstate.check_columns( event, IlinesCount-1 ) ;
	}
					
	SubMenuFromIndex = 1 ;
	switch ( SubmenuIndex )
	{
		case M_MIXER :
			if ( PopupData.PopupActive == 0 )
			{
				PopupData.PopupIdx = 0 ;
				PopupData.PopupActive = 3 ;
				SubmenuIndex = 0 ;
			}
//      pushMenu(menuProcMix) ;
		break ;
		case M_SWITCHES :
      pushMenu(menuProcSwitches) ;
		break ;
		case M_TELEMETRY :
			if ( PopupData.PopupActive == 0 )
			{
				PopupData.PopupIdx = 0 ;
				PopupData.PopupActive = 2 ;
				SubmenuIndex = 0 ;
			}
//      pushMenu(menuProcTelemetry) ;
		break ;
		case M_LIMITS :
      pushMenu(menuProcLimits) ;
		break ;
//		case M_HELI :
//      pushMenu(menuProcHeli) ;
//		break ;
		case M_VOICE :
      pushMenu(menuProcVoiceAlarm) ;
		break ;
		case M_CURVE :
      pushMenu(menuProcCurve) ;
		break ;
		case M_EXPO :
			SingleExpoChan = 0 ;
			s_expoChan = 0 ;
      pushMenu(menuProcExpoAll) ;
		break ;
		case M_MODES :
      pushMenu(menuModelPhases) ;
		break ;
		case M_GLOBALS :
//      pushMenu(menuProcGlobals) ;
			if ( PopupData.PopupActive == 0 )
			{
				PopupData.PopupIdx = 0 ;
				PopupData.PopupActive = 1 ;
				SubmenuIndex = 0 ;
			}
		break ;
		case M_PROTOCOL :
      pushMenu(menuProcProtocol) ;
		break ;
		case M_SAFETY :
      pushMenu(menuProcSafetySwitches) ;
		break ;
		default :
			SubMenuFromIndex = 0 ;
			if ( event == EVT_ENTRY )
			{
				audioDefevent(AU_MENUS) ;
				event = 0 ;
			}
		break ;
	}

	uint32_t sub = mstate.m_posVert ;
	uint32_t y = FH ;
//	Columns = 0 ;
	uint8_t blink = InverseBlink ;

	switch ( SubmenuIndex )
	{
		case M_MINDEX :
  		TITLE(XPSTR("MODEL SETUP")) ;
			IlinesCount = 15 ;
			sub += 1 ;

//static const char *const n_Strings[] = {
//Str_Mixer,
//Str_heli_setup,
//Str_limits,
//Str_Expo,
//Str_Modes,
//Str_Curves,
//Str_Cswitches,
//Str_Bluetooth,
//Str_Safety,
//Str_Globals,
//Str_Telemetry,
//Str_Voice,
//Str_Timer,
//Str_General,
//Str_Protocol
//};

static const uint16_t in_Strings[] = {
STR_Mixer,
STR_heli_setup,
STR_limits,
STR_Expo,
STR_Modes,
STR_Curves,
STR_Cswitches,
STR_Music,
STR_Safety,
STR_Globals,
STR_Telemetry,
STR_Voice,
STR_Timer,
STR_General,
STR_Protocol
};
	

			 
			if ( PopupData.PopupActive )
			{
				if ( PopupData.PopupActive == 1 )
				{
					sub = M_GLOBALS ;
				}
				else if ( PopupData.PopupActive == 2 )
				{
					sub = M_TELEMETRY ;
				}
				else
				{
					sub = M_MIXER ;
				}
			}
			
//			displayIndex( n_Strings, 8, 7, sub ) ;
			displayIndex( in_Strings, 8, 7, sub ) ;
			
			if ( PopupData.PopupActive )
			{
				uint32_t mask ;
				if ( PopupData.PopupActive == 1 )
				{
					mask = 7 ; ;
				}
				else if ( PopupData.PopupActive == 2 )
				{
					mask = 0x18 ;
				}
				else
				{
					mask = 0x0060 ;
				}
				uint8_t popaction = doPopup( PSTR( STR_POPUP_GLOBALS ), mask, 13, event ) ;
  			if ( popaction == POPUP_SELECT )
				{
					uint8_t popidx = PopupData.PopupSel ;
					if ( popidx == 0 )	// gvars
					{
    	  		pushMenu(menuProcGlobals) ;
					}
					else if ( popidx == 1 )	// adjusters
					{
    	  		pushMenu(menuProcAdjust) ;
					}
					else if ( popidx == 2 )	// scalers
					{
    	  		pushMenu(menuProcScalers) ;
					}
					else if ( popidx == 3 )	// Telemetry
					{
    	  		pushMenu(menuProcTelemetry) ;
					}
					else if ( popidx == 4 )	// Custom
					{
    	  		pushMenu(menuCustomTelemetry) ;
					}
					else if ( popidx == 5 )	// Mixer
					{
			      pushMenu(menuProcMix) ;
					}
					else if ( popidx == 6 )	// Templates
					{
    	  		pushMenu(menuProcTemplates) ;
					}
					SubmenuIndex = sub ;
				}
  			if ( popaction == POPUP_EXIT )
				{
					SubmenuIndex = 0 ;
					mstate.m_posVert = sub - 1 ;
				}
			}
		break ;

		case M_MUSIC :
		{	
			uint8_t subN = 0 ;
      TITLE( PSTR( STR_Music ) ) ;
			IlinesCount = 4 ;
			uint8_t attr = 0 ;
			
  	  if(sub==subN)
			{
	   		attr = InverseBlink ;
			}
			lcd_puts_Pleft( y, XPSTR("Start Switch"));
			uint8_t doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			g_model.musicData.musicStartSwitch = edit_dr_switch( 16*FW, y, g_model.musicData.musicStartSwitch, attr, doedit, event ) ;
			y += FH ;
			subN++ ;
			attr = 0 ;
  	  if(sub==subN)
			{
	   		attr = InverseBlink ;
			}
			lcd_puts_Pleft( y, XPSTR("Pause Switch"));
			doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			g_model.musicData.musicPauseSwitch = edit_dr_switch( 16*FW, y, g_model.musicData.musicPauseSwitch, attr, doedit, event ) ;
			y += FH ;
			subN++ ;
			attr = 0 ;
  	  if(sub==subN)
			{
	   		attr = InverseBlink ;
			}
			lcd_puts_Pleft( y, XPSTR("Previous Switch"));
			doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			g_model.musicData.musicPrevSwitch = edit_dr_switch( 16*FW, y, g_model.musicData.musicPrevSwitch, attr, doedit, event ) ;
			y += FH ;
			subN++ ;
			attr = 0 ;
  	  if(sub==subN)
			{
	   		attr = InverseBlink ;
			}
			lcd_puts_Pleft( y, XPSTR("Next Switch"));
			doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			g_model.musicData.musicNextSwitch = edit_dr_switch( 16*FW, y, g_model.musicData.musicNextSwitch, attr, doedit, event ) ;
		}
		break ;
		
		case M_TIMERS :
		{	
			uint8_t subN = 0 ;
			uint32_t t ;
			div_t qr ;
      TITLE( PSTR( STR_TIMER ) ) ;
			IlinesCount = 17 ;
			
			if ( sub < 16 )
			{
				lcd_puts_P( 20*FW-4, 7*FH, XPSTR("->") ) ;
				lcd_putcAtt( 6*FW, 0, ( sub < 8 ) ? '1' : '2', BLINK ) ;
				editTimer( sub, event ) ;
			}
			else
			{
				subN = 16 ;
//				y = FH ;
//				uint8_t attr = sub==subN ? blink : 0 ;
				lcd_puts_Pleft( y, PSTR( STR_TOTAL_TIME ));
				t = g_model.totalTime / 60 ;	// minutes
				qr = div( t, 60 ) ;
				lcd_outdezNAtt( 20*FW, y, (uint16_t)qr.rem, LEADING0, 2 ) ;
				lcd_putcAtt(17*FW, y, ':', 0 ) ;
				lcd_outdezAtt( 17*FW, y, (uint16_t)qr.quot, 0 ) ;

      	if(sub==subN)
				{
					if (event == EVT_KEY_LONG(KEY_MENU) )
					{
						g_model.totalTime = 0 ;
    		    killEvents( event ) ;
					}
				}
			}
		}
		break ;

		case M_HELI :
		{	
			uint8_t subN = 0 ;
  		uint8_t attr ;
			IlinesCount = 6 ;
			TITLE( PSTR(STR_HELI_SETUP) ) ;
//			y = 1*FH ;
	  	
			lcd_puts_Pleft(    y, PSTR(STR_HELI_TEXT));
			g_model.swashType = checkIndexed( y, PSTR(SWASH_TYPE_STR), g_model.swashType, (sub==subN) ) ;
			y += FH ;
			subN += 1 ;
		
			attr = 0 ;
//#if NUM_EXTRA_POTS
			if(sub==subN)
			{
				attr = blink ;
				uint8_t x = mapPots( g_model.swashCollectiveSource ) ;
				CHECK_INCDEC_H_MODELVAR_0( x, NUM_SKYXCHNRAW+NumExtraPots ) ;
				g_model.swashCollectiveSource = unmapPots( x ) ;
			}
//#else
//    	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.swashCollectiveSource, NUM_SKYXCHNRAW) ; }
//#endif
    	putsChnRaw(17*FW, y, g_model.swashCollectiveSource, attr ) ;
			y += FH ;
			subN += 1 ;

			attr = 0 ;
  	  if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.swashRingValue, 100) ; }
    	lcd_outdezAtt(20*FW, y, g_model.swashRingValue, attr ) ;
			y += FH ;
			subN += 1 ;

			g_model.swashInvertELE = hyphinvMenuItem( g_model.swashInvertELE, y, sub==subN ) ;
			y += FH ;
			subN += 1 ;
			
			g_model.swashInvertAIL = hyphinvMenuItem( g_model.swashInvertAIL, y, sub==subN ) ;
			y += FH ;
			subN += 1 ;

			g_model.swashInvertCOL = hyphinvMenuItem( g_model.swashInvertCOL, y, sub==subN ) ;
		}
		break ;

		case M_MGENERAL :
		{	
			uint8_t subN = 0 ;
			uint8_t attr = 0 ;
			static uint8_t voiceCall = 0 ;			
      TITLE( XPSTR(Str_General) ) ;

    	lcd_putc( 18*FW, 0, '(' ) ;
    	lcd_putc( 20*FW, 0, ')' ) ;
    	lcd_putc( 19*FW, 0, g_model.modelVersion + '0' ) ;

#ifdef PCBX9D
			IlinesCount = 19 ;
#else
			IlinesCount = 19 ;
#endif
//			y = FH ;

			if ( voiceCall )
			{
				if ( FileSelectResult == 1 )
				{
#ifdef PCBX9D
					if ( voiceCall == 2 )
					{
						copyFileName( g_model.modelImageName, SelectedVoiceFileName, 10 ) ;
						loadModelImage() ;
					}
					else
#endif
					{
						g_model.modelVoice = -1 ;
						copyFileName( g_model.modelVname, SelectedVoiceFileName, 8 ) ;
					}
	  		  eeDirty(EE_MODEL) ;		// Save it
					FileSelectResult = 0 ;
				}
//				else
//				{
//					mstate.m_posVert = SubMenuCall & 0x7F ;
//				}
				voiceCall = 0 ;
//				sub = mstate.m_posVert = 1 ;
			}

//#ifdef REV9E
//			if ( sub < 15 )
//#else
#ifdef PCBX9D
			if ( sub < 17 )
#else
			if ( sub < 12 )
#endif
//#endif
			{
				displayNext() ;
			}
			if ( sub < 6 )
			{
//				lcd_puts_Pleft( y, PSTR(STR_NAME)) ;
//				lcd_putsnAtt( 11*FW-2, y, (const char *)g_model.name, sizeof(g_model.name), 0 ) ;
				SubMenuCall = 0x80 ;
				alphaEditName( 11*FW-2, y, (uint8_t *)g_model.name, sizeof(g_model.name), sub==subN, (uint8_t *)XPSTR( "Model Name") ) ;
				
//				if(sub==subN)
//				{
////					Columns = sizeof(g_model.name)-1 ;
//					lcd_rect( 11*FW-3, y-1, sizeof(g_model.name)*FW+2, 9 ) ;
//					if ( event == EVT_KEY_FIRST(KEY_MENU) )
//					{
//						AlphaLength = sizeof(g_model.name) ;
//						PalphaText = (uint8_t *)g_model.name ;
//						s_editMode = 0 ;
//    				killEvents(event) ;
//						SubMenuCall = 0x80 ;
//			      pushMenu(menuProcAlpha) ;
//					}
//				}
//				editName( 11*FW-2, g_posHorz, y, (uint8_t *)g_model.name, sizeof(g_model.name), sub==subN ? EE_MODEL : 0, event ) ;
				y += FH ;
				subN += 1 ;

    		if(sub==subN)
				{
					if (event == EVT_KEY_LONG(KEY_MENU) )
					{
						voiceCall = 1 ;
						SubMenuCall = 0x81 ;
						VoiceFileType = VOICE_FILE_TYPE_NAME ;
    		   	pushMenu( menuProcSelectVoiceFile ) ;				
  			  	s_editMode = 0 ;
    		    killEvents(event);
					}
					if ( g_posHorz == 0 )
					{
						attr = InverseBlink ;
  			    CHECK_INCDEC_H_MODELVAR( g_model.modelVoice, -1, 49 ) ;
					}
				}
				if ( g_model.modelVoice == -1 )
				{
					if(sub==subN)
					{
						Columns = 1 ;
						SubMenuCall = 0xA1 ;
					}
					uint8_t type = ( (sub==subN) && (g_posHorz) ) ? EE_MODEL : 0 ;
//					editName( 9*FW-2, g_posHorz-1, y, (uint8_t *)g_model.modelVname, sizeof(g_model.modelVname), type, event ) ;
					alphaEditName( 9*FW-2, y, (uint8_t *)g_model.modelVname, sizeof(g_model.modelVname), type, (uint8_t *)XPSTR( "FileName") ) ;
				}
				else
				{
					if (event == EVT_KEY_BREAK(KEY_MENU) )
					{
						if(sub==subN)
						{
							if ( g_eeGeneral.forceMenuEdit == 0 )
							{
								putVoiceQueue( ( g_model.modelVoice + 260 ) | VLOC_NUMUSER  ) ;
								s_editMode = 0 ;
							}
						}
					}
				}
    		lcd_puts_Pleft( y, PSTR(STR_VOICE_INDEX) ) ;
				if ( g_model.modelVoice == -1 )
				{
    			lcd_putcAtt( 7*FW+2, y, '>', attr) ;
				}
				else
				{
    			lcd_outdezAtt(  10*FW+2, y, (int16_t)g_model.modelVoice + 260 ,attr);
				}
				y += FH ;
				subN += 1 ;

				g_model.thrTrim = onoffMenuItem( g_model.thrTrim, y, PSTR(STR_T_TRIM), sub==subN) ;
				y += FH ;
				subN += 1 ;
			
				g_model.thrExpo = onoffMenuItem( g_model.thrExpo, y, PSTR(STR_T_EXPO), sub==subN) ;
				y += FH ;
				subN += 1 ;

				lcd_puts_Pleft( y,PSTR(STR_LIGHT_SWITCH));
  			attr = 0 ;
  			if(sub==subN) { attr = InverseBlink ; }
				g_model.mlightSw = edit_dr_switch( 17*FW, y, g_model.mlightSw, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				if ( g_model.mlightSw == 0 )
				{
					putsMomentDrSwitches( 14*FW, y, g_eeGeneral.lightSw, 0 ) ;
    			lcd_putcAtt( 12*FW, y, '(', 0 ) ;
    			lcd_putcAtt( 17*FW, y, ')', 0 ) ;
				}
				y += FH ;
				subN += 1 ;

				g_model.useCustomStickNames = onoffMenuItem( g_model.useCustomStickNames, y, PSTR( STR_CUSTOM_STK_NAMES ), sub==subN) ;
			}
#ifdef PCBX9D
			else if ( sub < 11 )
#else
			else if ( sub < 12 )
#endif
			{
				subN = 6 ;
//				y = FH ;
  			
				lcd_puts_Pleft(    y, PSTR(STR_TRIM_INC));
//  			attr = 0 ;
//				if(sub==subN)
//				{
//  			 	attr = InverseBlink ;
//  			  CHECK_INCDEC_H_MODELVAR_0(event,g_model.trimInc,4) ;
//				}
//  			lcd_putsAttIdx(  14*FW, y, PSTR(STR_TRIM_OPTIONS),g_model.trimInc, attr);
				
				g_model.trimInc = checkIndexed( y, PSTR(STR_TRIM_OPTIONS), g_model.trimInc, (sub==subN) ) ;
				y += FH ;
				subN += 1 ;

//  			lcd_puts_Pleft(    y, PSTR(STR_TRIM_SWITCH));
  			attr = 0 ;
  			if(sub==subN) { attr = InverseBlink ; }
				g_model.trimSw = edit_dr_switch( 17*FW, y, g_model.trimSw, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				y += FH ;
				subN += 1 ;
	
				g_model.extendedLimits = onoffMenuItem( g_model.extendedLimits, y, PSTR(STR_E_LIMITS), sub==subN) ;
    //---------------------------------                           /* ReSt V */
    //   If extended Limits are switched off, min and max limits must be limited to +- 100         
        if (!g_model.extendedLimits)
        {
			    for(uint8_t i=0; i<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS; i++)
          {
      	    LimitData *ld = &g_model.limitData[i] ;
#if EXTRA_SKYCHANNELS
						if ( i >= NUM_SKYCHNOUT )
						{
							ld = &g_model.elimitData[i-NUM_SKYCHNOUT] ;
						}
#endif			
            if (ld->min < 0) ld->min = 0;
            if (ld->max > 0) ld->max = 0;
          }
        }                                 
    //-----------------------------------                           /* ReSt A */

				
				y += FH ;
				subN += 1 ;

				uint8_t oldValue = g_model.throttleReversed ;
				g_model.throttleReversed = onoffMenuItem( oldValue, y, PSTR(STR_THR_REVERSE), sub == subN ) ;
				if ( g_model.throttleReversed != oldValue )
				{
  				checkTHR() ;
				}
 				y += FH ;
				lcd_putc( 15*FW, y, throttleReversed() ? '\201' : '\200' ) ;
    		lcd_puts_Pleft( y, PSTR( STR_THROTTLE_OPEN ) ) ;
 				
				y += FH ;
				subN += 1 ;

  			attr = 0 ;
				oldValue = g_model.throttleIdle ;
    		lcd_puts_Pleft( y, PSTR( STR_THR_DEFAULT ) ) ;
//    		if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.throttleIdle, 1 ) ; }
//    		lcd_putsAttIdx( 15*FW, y, XPSTR("\006   EndCentre"),g_model.throttleIdle, attr ) ;
				g_model.throttleIdle = checkIndexed( y, XPSTR(FWx15"\001""\006   EndCentre"), oldValue, sub==subN ) ;
				if ( g_model.throttleIdle != oldValue )
				{
  				checkTHR() ;
				}
#ifndef PCBX9D
				y += FH ;
				subN += 1 ;

    		lcd_puts_Pleft( y, PSTR( STR_CUSTOM_CHECK ) ) ;
				if ( sub == subN )
				{
					lcd_char_inverse( 0, y, 12*FW, 0 ) ;
					if (event == EVT_KEY_BREAK(KEY_MENU) )
					{
						SubMenuCall = 0x80 + 11 ;
    		   	pushMenu( menuCustomCheck ) ;
  			  	s_editMode = 0 ;
    		    killEvents(event);
					}
				}
#endif
			}
//#ifdef REV9E
//			else if ( sub < 15 )
//#else			
	#ifdef PCBX9D
			else if ( sub < 17 )
 #else
			else
 #endif
//#endif
			{
#ifdef PCBX9D
				subN = 11 ;
#else			
				subN = 12 ;
#endif
    	
  			attr = PREC1 ;
  			lcd_puts_Pleft(    y, PSTR(STR_AUTO_LIMITS));
  			if(sub==subN) { attr = InverseBlink | PREC1 ; CHECK_INCDEC_H_MODELVAR_0( g_model.sub_trim_limit, 100 ) ; }
  			lcd_outdezAtt(  20*FW, y, g_model.sub_trim_limit, attr ) ;
					 
				y += FH ;
				subN += 1 ;
#ifdef REV9E
				uint32_t states = g_model.modelswitchWarningStates ;
#else				
				uint16_t states = g_model.modelswitchWarningStates ;
#endif
    		uint8_t b = (states & 1) ;
				b = offonMenuItem( b, y, PSTR(STR_SWITCH_WARN), sub==subN ) ;
    		g_model.modelswitchWarningStates = (states & ~1) | b ;
				y += FH ;
				subN += 1 ;

#if defined(PCBSKY) || defined(PCB9XT)
				uint8_t enables = 0x3F ;
				uint16_t temp = g_model.modelswitchWarningDisables ;

				if ( temp & THR_WARN_MASK )
				{
					enables &= ~0x01 ;
				}
				if ( temp & RUD_WARN_MASK )
				{
					enables &= ~0x02 ;
				}
				if ( temp & ELE_WARN_MASK )
				{
					enables &= ~0x04 ;
				}
				if ( temp & IDX_WARN_MASK )
				{
					enables &= ~0x08 ;
				}
				if ( temp & AIL_WARN_MASK )
				{
					enables &= ~0x10 ;
				}
				if ( temp & GEA_WARN_MASK )
				{
					enables &= ~0x20 ;
				}

				uint8_t subSub = g_posHorz ;
    		for(uint8_t i=0;i<6;i++)
				{
					lcd_putsnAtt((11+i)*(FW+1), y, XPSTR("TREIAG")+i,1, ((enables & (1<<i)) ? INVERS : 0 ) );
					if ( (sub==subN) )
					{
						if ( ( (subSub)==i ) && ( BLINK_ON_PHASE ) )
						{
							lcd_rect( (11+i)*(FW+1)-1, y-1, FW+2, 9 ) ;
						}
					}
				}
    		if(sub==subN)
				{
					Columns = 5 ;
        	if((event==EVT_KEY_FIRST(KEY_MENU)) || (event == EVT_KEY_BREAK(BTN_RE)) || P1values.p1valdiff)
					{
            killEvents(event);
            s_editMode = false;
            enables ^= (1<<(subSub));
            STORE_MODELVARS;
	        }
  		  }
				temp = 0 ;
				if ( ( enables & 1 ) == 0 ) temp |= THR_WARN_MASK ;
				if ( ( enables & 2 ) == 0 ) temp |= RUD_WARN_MASK ;
				if ( ( enables & 4 ) == 0 ) temp |= ELE_WARN_MASK ;
				if ( ( enables & 8 ) == 0 ) temp |= IDX_WARN_MASK ;
				if ( ( enables & 0x10 ) == 0 ) temp |= AIL_WARN_MASK ;
				if ( ( enables & 0x20 ) == 0 ) temp |= GEA_WARN_MASK ;
				g_model.modelswitchWarningDisables = temp ;
				subN += 1 ;
#endif
#ifdef PCBX9D
				uint8_t enables = 0x7F ;
				uint16_t temp = g_model.modelswitchWarningDisables ;

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

				uint8_t subSub = g_posHorz ;
    		for(uint8_t i=0;i<7;i++)
				{
					uint8_t attr = 0 ;
					if ( (enables & (1<<i)) )
					{
						attr = INVERS ;
					}
					if ( sub == subN )
					{
						if ( subSub==i )
						{
							attr = BLINK ;	
						}
					}
					lcd_putsnAtt((13+i)*FW, y, XPSTR("ABCDEFG")+i,1, attr );
				}
    		if(sub==subN)
				{
					Columns = 6 ;
        	if((event==EVT_KEY_FIRST(KEY_MENU)) || (event == EVT_KEY_BREAK(BTN_RE)) || P1values.p1valdiff)
					{
            killEvents(event);
            s_editMode = false;
            enables ^= (1<<(subSub));
            STORE_MODELVARS;
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
				g_model.modelswitchWarningDisables = temp ;
				subN += 1 ;
#endif
		    lcd_puts_Pleft(    y, PSTR(STR_DEAFULT_SW)) ;
		 		states = g_model.modelswitchWarningStates >> 1 ;
#ifdef REV9E
				states |= (uint32_t)g_model.xmodelswitchWarningStates << 14 ;
#endif
        
#if defined(PCBSKY) || defined(PCB9XT)
				y += FH ;
				uint16_t index ;
						
				index = oneSwitchText( HSW_ThrCt, states ) ;
				attr = index >> 8 ;
				index &= 0x00FF ;
				lcd_putsAttIdx(0*FW+5, y, PSTR(SWITCHES_STR), index, attr ) ;
		
				index = oneSwitchText( HSW_RuddDR, states ) ;
				attr = index >> 8 ;
				index &= 0x00FF ;
				lcd_putsAttIdx(3*FW+7, y, PSTR(SWITCHES_STR), index, attr ) ;
		
				index = oneSwitchText( HSW_ElevDR, states ) ;
				attr = index >> 8 ;
				index &= 0x00FF ;
				lcd_putsAttIdx(6*FW+9, y, PSTR(SWITCHES_STR), index, attr ) ;

				index = oneSwitchText( HSW_ID0, states ) & 0x00FF ;
				lcd_putsAttIdx(9*FW+11, y, PSTR(SWITCHES_STR), index, 0 ) ;

				index = oneSwitchText( HSW_AileDR, states ) ;
				attr = index >> 8 ;
				index &= 0x00FF ;
				lcd_putsAttIdx(12*FW+13, y, PSTR(SWITCHES_STR), index, attr ) ;
		
				index = oneSwitchText( HSW_Gear, states ) ;
				attr = index >> 8 ;
				index &= 0x00FF ;
				lcd_putsAttIdx(15*FW+15, y, PSTR(SWITCHES_STR), index, attr ) ;
		    
				if(sub==subN)
				{
//					plotType = PLOT_BLACK ;
					lcd_rect( 0*FW+4-1, y-1, 18*FW+2+12, 9 ) ;
//					plotType = PLOT_XOR ;
#endif
#ifdef PCBX9D
//#ifdef REV9E
//    		for (uint8_t i=0 ; i<3 ; i += 1 )
//				{
//    		  lcd_putc( 11*FW+11+i*(2*FW+3), y, 'A'+i ) ;
//					lcd_putc( 12*FW+11+i*(2*FW+3), y, PSTR(HW_SWITCHARROW_STR)[states & 0x03] ) ;
//    		  states >>= 2 ;
//    		}
//				y += FH ;
    		
//    		for (uint8_t i=0 ; i<8 ; i += 1 )
//				{
//    		  lcd_putc( 2+0*FW+i*(2*FW+3), y, 'A'+3+i ) ;
//					lcd_putc( 2+1*FW+i*(2*FW+3), y, PSTR(HW_SWITCHARROW_STR)[states & 0x03] ) ;
//    		  states >>= 2 ;
//    		}
//				y += FH ;
//    		for (uint8_t i=0 ; i<7 ; i += 1 )
//				{
//    		  lcd_putc( 2+0*FW+i*(2*FW+3), y, 'A'+11+i ) ;
//					lcd_putc( 2+1*FW+i*(2*FW+3), y, PSTR(HW_SWITCHARROW_STR)[states & 0x03] ) ;
//    		  states >>= 2 ;
//    		}

//				if(sub==subN)
//				{
//					lcd_rect( 11*FW+11-2, y-1-2*FH, 10*FW-14, 9 ) ;
//					lcd_rect( 0*FW, y-1-FH, 20*FW+2, 9 ) ;
//					lcd_rect( 0*FW, y-1, 20*FW+2, 9 ) ;
//#else
				y += FH ;
    		for (uint8_t i=0 ; i<7 ; i += 1 )
				{
    		  lcd_putc( 2*FW+i*(2*FW+3), y, 'A'+i ) ;
					lcd_putc( 3*FW+i*(2*FW+3), y, PSTR(HW_SWITCHARROW_STR)[states & 0x03] ) ;
    		  states >>= 2 ;
    		}
    		if(sub==subN)
				{
					lcd_rect( 2*FW-2, y-1, 14*FW+2+18+1, 9 ) ;
//#endif
#endif
		      if (event==EVT_KEY_FIRST(KEY_MENU) || event==EVT_KEY_FIRST(BTN_RE))
					{
    		    killEvents(event);
//    		uint16_t states = g_model.modelswitchWarningStates & 1 ;
#if defined(PCBSKY) || defined(PCB9XT)
			      g_model.modelswitchWarningStates = (getCurrentSwitchStates() << 1 ) ;// states ;
#endif
#ifdef PCBX9D
			  		getMovedSwitch() ;	// loads switches_states
#ifdef REV9E
extern uint32_t switches_states ;
				    g_model.modelswitchWarningStates = ((switches_states & 0x3FFF) << 1 ) ;// states ;
				    g_model.xmodelswitchWarningStates = (switches_states >> 14 ) ;// states ;
#else
extern uint16_t switches_states ;
				    g_model.modelswitchWarningStates = ((switches_states & 0x3FFF) << 1 ) ;// states ;
#endif
#endif
    		    s_editMode = false ;
		        STORE_MODELVARS ;
					}
				}
				y += FH ;
				subN += 1 ;

		  	attr = 0 ;
    		lcd_puts_Pleft(    y, PSTR(STR_VOLUME_CTRL));
		    if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.anaVolume, 7 ) ; }
#if defined(PCBSKY) || defined(PCB9XT)
		    lcd_putsAttIdx( 17*FW, y, XPSTR("\003---P1 P2 P3 GV4GV5GV6GV7"),g_model.anaVolume, attr ) ;
#endif
#ifdef PCBX9D
    		lcd_putsAttIdx( 17*FW, y, XPSTR("\003---P1 P2 SL SR GV5GV6GV7"),g_model.anaVolume, attr ) ;
#endif
				y += FH ;
				subN += 1 ;

//#ifdef REV9E
//			}
//			else
//			{
//				subN = 15 ;
//#endif

				uint8_t value = g_model.trainerProfile + 1 ;
				if ( g_model.traineron == 0 )
				{
					value = 0 ;
				}
    	  lcd_puts_Pleft( y, PSTR(STR_Trainer) ) ;
    	  if(value)
				{
    	    lcd_outdezAtt( PARAM_OFS+3*FW, y, value-1, (sub==subN ? blink : 0) ) ;
    	    lcd_putc( 8*FW, y, '(' ) ;
    	    lcd_putc( 15*FW, y, ')' ) ;
					lcd_putsnAtt( 9*FW, y, (const char *)g_eeGeneral.trainerProfile[value-1].profileName, 6, 0 ) ;
    	  }
    	  else
				{
    	    lcd_putsAtt(PARAM_OFS, y, PSTR(STR_OFF), (sub==subN ? blink:0) ) ;
				}
    	  if(sub==subN)
				{
					CHECK_INCDEC_H_MODELVAR_0( value, 4 ) ;
					if ( ( value > 0) && ( event == EVT_KEY_LONG(KEY_MENU) ) )
					{
						SingleExpoChan = 1 ;
						s_expoChan = value-1 ;
						s_editMode = 0 ;
       			killEvents(event);
       			pushMenu(menuProcTrainer) ; 
						SubMenuCall = 0x91 ;
					}
				}

				g_model.traineron = value > 0 ;
				if ( value )
				{
					g_model.trainerProfile = value - 1 ;
				}
				y += FH ;
				subN += 1 ;
 
#ifdef PCBX9D
//#ifndef REV9E
			}
			else
			{
				subN = 17 ;
//#endif
#endif

#ifdef REV9E
				uint8_t subSub = g_posHorz ;
#else
 #ifdef PCBX9D
				uint8_t subSub = g_posHorz ;
 #else
				subSub = g_posHorz ;
 #endif
#endif
		    lcd_puts_Pleft(    y, PSTR(STR_BEEP_CENTRE));
#if defined(PCBSKY) || defined(PCB9XT)
				uint32_t width = NumExtraPots ? 7 : 6 ;
    		for(uint8_t i=0;i<=width;i++)
#endif
#ifdef PCBX9D
    		for(uint8_t i=0;i<8;i++)
#endif
				{
					lcd_putsnAtt((10+i)*FW, y, PSTR(STR_RETA123)+i,1, (((subSub)==i) && (sub==subN)) ? BLINK : ((g_model.beepANACenter & (1<<i)) ? blink : 0 ) );
				}
    		if(sub==subN)
				{
#if defined(PCBSKY) || defined(PCB9XT)
					Columns = width ;
#endif
#ifdef PCBX9D
					Columns = 7 ;
#endif
        	if((event==EVT_KEY_FIRST(KEY_MENU)) || (event == EVT_KEY_BREAK(BTN_RE)) || P1values.p1valdiff)
					{
            killEvents(event);
            s_editMode = false;
            g_model.beepANACenter ^= (1<<(subSub));
            STORE_MODELVARS;
//            eeWaitComplete() ;
	        }
  		  }
#ifdef PCBX9D
				y += FH ;
				subN += 1 ;
    		
				lcd_puts_Pleft( y, XPSTR( "Image Name") ) ;
				uint8_t type = ALPHA_NO_NAME ;
				if(sub==subN)
				{
					Columns = 1 ;
					SubMenuCall = 0xB2 ;
					type |= EE_MODEL ;
					if (event == EVT_KEY_LONG(KEY_MENU) )
					{
						voiceCall = 2 ;
    		   	pushMenu( menuProcSelectImageFile ) ;				
  			  	s_editMode = 0 ;
    		    killEvents(event);
					}
				}
				if ( voiceCall == 0 )
				{
					alphaEditName( 11*FW-2, y, (uint8_t *)g_model.modelImageName, sizeof(g_model.modelImageName), type | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FIlename") ) ;
				}
#endif
			}
		}	
		break ;

//#define BT_COMMAND_DEBUG	1
//static uint8_t BtCommandMode ;
//#define BT_TYPE_HC06		0
//#define BT_TYPE_HC05		1

//		case M_BLUETOOTH :
//		{	
//			uint8_t subN = 0 ;
			
//      TITLE( XPSTR("BlueTooth") ) ;
//#ifdef BLUETOOTH
// #ifdef BT_COMMAND_DEBUG
//			IlinesCount = 3 ;
// #else
//			IlinesCount = 5 ;
// #endif
//#else
//			IlinesCount = 0 ;
//#endif
//			y = 0 ;
//			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
//			{
//				IlinesCount += 3 ;
//			}
//			lcd_puts_Pleft( y,XPSTR("\013Type"));
//			g_eeGeneral.BtType = checkIndexed( y, XPSTR(FWx16"\001""\005HC-06HC-05"), g_eeGeneral.BtType, (sub==subN) ) ;
//			y += FH ;
//			subN += 1 ;

//#ifdef BLUETOOTH
//			lcd_puts_Pleft( y, XPSTR("BT as Trainer") );
//			g_model.BTfunction = checkIndexed( y, XPSTR(FWx16"\002""\004 OFF  RxTxRx"), g_model.BTfunction, (sub==subN) ) ;
////			if ( (sub == subN) )
////			{
////				CHECK_INCDEC_H_MODELVAR_0( g_model.BTfunction, 2 ) ;
////			}
////  		menu_lcd_onoff( PARAM_OFS, y, g_model.BTfunction, sub==subN ) ;
//			y += FH ;
//			subN += 1 ;

//// #ifdef BT_COMMAND_DEBUG


//extern uint8_t BtName[] ;
//extern uint8_t BtNameChange ;

//			uint8_t attr = (sub == subN) ? INVERS : 0 ;
////			lcd_puts_Pleft( y, XPSTR("BT Name") ) ;
////			lcd_putsAtt( 7*FW, y, ( g_eeGeneral.BtType == BT_TYPE_HC05 ) ? (char *)BtName: (char *)g_eeGeneral.btName, attr ) ;
				
//			SubMenuCall = 0x82 ;
//			uint8_t *pname = g_eeGeneral.btName ;
//			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
//			{
//				pname = BtName ;
//			}
//			alphaEditName( 7*FW, y, pname, 14, attr, 0 ) ;
//			if ( event == EVT_ENTRY_UP )
//			{
//				// Name was edited
//				if ( AlphaEdited )
//				{
//					if ( BtNameChange == 0 )
//					{
//						BtNameChange = 0x80 ;
//					}
//					AlphaEdited = 0 ;
//				}
//			}

//			y += FH ;
//			subN += 1 ;

//extern uint8_t BtRoleChange ;
//extern uint8_t BtMasterSlave ;
//			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
//			{
//				lcd_puts_Pleft( y, XPSTR("BT Role") ) ;
//	    	lcd_putsAttIdx( 12*FW, y, XPSTR("\007UNKNOWNSLAVE  MASTER "), BtMasterSlave, (sub==subN) ? blink:0 ) ;
//				uint8_t newRole = 0 ;
//				if ( BtMasterSlave == 2 )
//				{
//					newRole = 1 ;
//				}
//				if ( (sub == subN) )
//				{
//					newRole = checkIncDec( newRole, 0, 1, 0 ) ;
//					if ( newRole+1 != BtMasterSlave )
//					{
//						if ( BtRoleChange == 0 )
//						{
//							BtRoleChange = 0x80 | newRole ;
//						}
//					}
//				}
////			if ( (sub == subN) )
////			{
////				BtCommandMode = checkIncDec( BtCommandMode, 0, 1, 0 ) ;
////			}
////  		menu_lcd_onoff( PARAM_OFS, y, BtCommandMode, sub==subN ) ;

////#ifndef PCB9XT
////			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
////			{
////				if ( BtCommandMode )
////				{
////					PIOB->PIO_SODR = PIO_PB12 ;		// Set bit B12 HIGH
////				}
////				else
////				{
////					PIOB->PIO_CODR = PIO_PB12 ;		// Set bit B12 LOW
////				}
////			}
////#endif // nPCB9XT
//				y += FH ;
//				subN += 1 ;
//// #endif
//			}
			
//			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
//			{
//				g_model.autoBtConnect = onoffMenuItem( g_model.autoBtConnect, y, XPSTR("Auto Connect"), sub==subN ) ;
//				subN += 1 ;
//				y += FH ;				 
//				uint8_t attr = (sub == subN) ? INVERS : 0 ;
//				lcd_putsAtt( 0, y, XPSTR("Scan"), attr ) ; 
//				if ( attr )
//				{
//					if ( event == EVT_KEY_BREAK(KEY_MENU) )
//					{
//						s_editMode = 0 ;
//						SubMenuCall = 0x85 ;
//    		   	pushMenu( menuProcBtScan ) ;
//					}
//				}
//				subN += 1 ;
//				attr = (sub == subN) ? INVERS : 0 ;
//				lcd_putsAtt( 10*FW, y, XPSTR("Configure"), attr ) ; 
//				if ( attr )
//				{
//					if ( event == EVT_KEY_BREAK(KEY_MENU) )
//					{
//						s_editMode = 0 ;
//						SubMenuCall = 0x86 ;
//    		   	pushMenu( menuProcBtConfigure ) ;
//					}
//				}
//				subN += 1 ;
//				y += FH ;				 

////extern uint8_t BtMode[] ;
//extern uint8_t BtPswd[] ;
////extern uint8_t BtRole[] ;
////extern uint8_t BtIac[] ;
				
////				lcd_puts_Pleft( FH*6, XPSTR("Name\037Pswd\013Br")) ;

//				attr = (sub == subN) ? INVERS : 0 ;
//				lcd_putsAtt( 10*FW, y, XPSTR("List"), attr ) ; 
				
//				if ( attr )
//				{
//					if ( event == EVT_KEY_BREAK(KEY_MENU) )
//					{
//						s_editMode = 0 ;
//						SubMenuCall = 0x87 ;
//    			  pushMenu( menuProcBtAddressList ) ;
//					}
//				}

//				lcd_puts_Pleft( FH*7, XPSTR("Pswd\013Br")) ;
////				lcd_putsAtt( 6*FW, 5*FH, (char *)BtRole, 0 ) ;
////				lcd_putsAtt( 5*FW, 7*FH, (char *)BtName, 0 ) ;
//				lcd_putsAtt( 5*FW, 7*FH, (char *)BtPswd, 0 ) ;
//  			lcd_putsAttIdx(  14*FW, 7*FH, XPSTR("\006115200  9600 19200 57600 38400"), BtCurrentBaudrate, 0 ) ;

////				lcd_putsAtt( 12*FW, 4*FH, (char *)BtIac, 0 ) ;
//			}
//#endif

//		}	
//		break ;

	}	
}

#ifdef BLUETOOTH
extern uint8_t Bt_ok ;
extern uint16_t Bt_debug ;
extern uint8_t Bt_baudrate ;
extern struct t_fifo128 BtRx_fifo ;
extern uint16_t BtTotals[4] ;
extern uint16_t BtRxTimer ;

void menuProcBt(uint8_t event)
{
  MENU(XPSTR("BT"), menuTabStat, e_bluetooth, 0, {0/*, 0*/});

	lcd_outdez( 5*FW, 0, BtTotals[0] ) ;
	lcd_outdez( 9*FW, 0, BtTotals[1] ) ;
	lcd_outdez( 13*FW, 0, BtTotals[2] ) ;
	lcd_outdez( 17*FW, 0, BtTotals[3] ) ;

//	lcd_outhex4( 0, 1*FH, Bt_ok ) ;
//	lcd_outhex4( 25, 1*FH, Bt_debug ) ;
//	lcd_outhex4( 50, 1*FH, Bt_baudrate ) ;
	 
#ifdef PCBSKY
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
#endif
#ifdef PCB9XT
//extern struct t_fifo128 Console_fifo ;
//extern struct t_fifo128 Com3_fifo ;
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
//	if ( g_eeGeneral.btComPort == 2 )
//	{
//		p = (uint16_t *)&Com3_fifo ;
//	}
#endif
	uint32_t i ;
	uint16_t x ;
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 1*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 2*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 3*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 4*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 5*FH, x ) ;
	}
	
#ifndef PCB9XT
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 6*FH, x ) ;
	}
#endif

//	for ( i = 0 ; i < 2 ; i += 1 )
//	{
//		x = *p++ ;
//		x = ( x >> 8 ) | ( x << 8 ) ;
//		lcd_outhex4( i*25, 7*FH, x ) ;
//	}
extern uint8_t BtBadChecksum ;
	lcd_outdez( 25, 7*FH, BtBadChecksum ) ;
	
#ifdef BLUETOOTH
	lcd_outdez( 75, 7*FH, BtRxTimer ) ;
extern uint8_t BtCurrentLinkIndex ;
extern uint8_t BtBaudChangeIndex ;
	lcd_outdez( 100, 7*FH, BtCurrentLinkIndex ) ;
	lcd_outdez( 125, 7*FH, BtBaudChangeIndex ) ;
#endif


//#ifdef REVX
//extern struct t_serial_tx Com2_tx ;
//	lcd_outdez( 50, 7*FH, Com2_tx.ready ) ;

//extern struct t_serial_tx *Current_Com2 ;
//	uint32_t xyz = (uint32_t)Current_Com2 ;
//	lcd_outhex4( 75, 5*FH, xyz >> 16 ) ;
//	lcd_outhex4( 100, 5*FH, xyz ) ;

//	lcd_outhex4( 75, 7*FH, UART0->UART_TCR ) ;
//	lcd_outhex4( 99, 7*FH, UART0->UART_IMR ) ;
//	lcd_outhex4( 50, 6*FH, UART0->UART_CR ) ;
//	lcd_outhex4( 75, 6*FH, UART0->UART_PTSR ) ;
//	lcd_outhex4( 100, 6*FH, UART0->UART_SR ) ;
//#endif
//	lcd_outhex4( 100, 7*FH, Bt_ok ) ;

}

//PACK(typedef struct t_btDevice {
//  uint8_t  address[6] ;
//  uint8_t  name[8] ;
//}) btDeviceData ;

//	btDeviceData btDevice[4] ;

void menuProcBtAddressList(uint8_t event)
{
  lcd_puts_Pleft( 0, XPSTR("Bt Address List") ) ;
	static MState2 mstate2 ;
	if ( PopupData.PopupActive == 0 )
	{
		mstate2.check_columns(event, IlinesCount ) ;
	}

	uint8_t addressText[16] ;
	uint32_t i ;
	uint32_t j ;
	uint8_t sub = mstate2.m_posVert ;
	btDeviceData *btData = &g_eeGeneral.btDevice[0] ;
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		uint8_t attr = 0 ;
		j = btData->address[0] | btData->address[1] | btData->address[2] ;
		if ( j )
		{
			if ( i == sub )
			{
				attr = INVERS ;
			}
			
			btAddrBin2Hex( addressText, btData->address ) ;
		  lcd_putsnAtt( 1, (i+1)*FH, (char *)addressText, 14, attr ) ;
			lcd_putsnAtt( 15*FW, (i+1)*FH, (char *)btData->name, 6, attr ) ;
			if ( g_model.btDefaultAddress == i )
			{
				lcd_rect( 0, (i+1)*FH-1, 128, 9 ) ;
			}
		}
		else
		{
			break ;
		}
		btData += 1 ;
	}
	IlinesCount = i ? i-1 : 0 ;
	if ( g_model.btDefaultAddress >= i )
	{
		g_model.btDefaultAddress -= 1 ;
	}

//	lcd_outhex4( 0, 7*FH, i ) ;
//	lcd_outhex4( 30, 7*FH, g_model.btDefaultAddress ) ;
//	lcd_outhex4( 60, 7*FH, event ) ;

	if ( PopupData.PopupActive == 0 )
	{
		if ( (event==EVT_KEY_BREAK(KEY_MENU)) || (event == EVT_KEY_BREAK(BTN_RE))  )
		{
			PopupData.PopupIdx = 0 ;
			PopupData.PopupActive = 1 ;
			killEvents(event) ;
			Tevent = event = 0 ;
			s_editMode = 0 ;
		}
	}

	if ( PopupData.PopupActive )
	{
		uint8_t popaction = doPopup( XPSTR("Set Default\0Edit Name\0Link\0Delete"), 0x0F, 12, event ) ;
  	if ( popaction == POPUP_SELECT )
		{
			uint8_t popidx = PopupData.PopupSel ;
			if ( popidx == 0 )	// Set Default
			{
				g_model.btDefaultAddress = sub ;
	      STORE_MODELVARS ;
			}
			if ( popidx == 1 )	// Edit Name
			{
				Tevent = EVT_KEY_BREAK(KEY_MENU) ;
				EditType = EE_GENERAL ;
				alphaEditName( 16*FW, (sub+1)*FH, g_eeGeneral.btDevice[sub].name, 6, 1, 0 ) ;
				EditType = EE_MODEL ;
			}
			if ( popidx == 2 )	// Link
			{
				BtLinkRequest = 0x80 + sub ;
			}
			if ( popidx == 3 )	// Delete
			{
				for ( i = sub ; i < 3 ; i += 1 )
				{
					g_eeGeneral.btDevice[i] = g_eeGeneral.btDevice[i+1] ;
				}
    		memset(&g_eeGeneral.btDevice[3],0,sizeof(g_eeGeneral.btDevice[3]));
				if ( g_model.btDefaultAddress == sub )
				{
					if ( g_model.btDefaultAddress )
					{
						g_model.btDefaultAddress -= 1 ;
					}
				}
	      STORE_MODELVARS ;
        STORE_GENERALVARS ;
			}
		}
//  	if ( popaction == POPUP_EXIT )
//		{
//			mstate.m_posVert = M_GLOBALS - 1 ;
//		}
	}

}

extern uint8_t BtConfigure ;
void menuProcBtConfigure(uint8_t event)
{
	uint32_t i ;

  lcd_puts_Pleft( 0, XPSTR("Bt Configure") ) ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, 0 ) ;

	if ( event == EVT_ENTRY )
	{
		if ( BtCurrentBaudrate != g_eeGeneral.bt_baudrate )
		{
			BtBaudrateChanged = 1 ;
		}
		BtConfigure = 0x80 ;
	}

	lcd_puts_Pleft( 2*FH, (BtConfigure) ? XPSTR( "\006Configuring" ) : XPSTR("\007Configured") ) ;

	if ( BtConfigure )
	{
		uint32_t width ;
		width = BtConfigure & 0x3F ;
		if ( width > 17 )
		{
			width = 17 ;
		}
		width *= 4 ;
		lcd_hline( 0, 5*FH-1, 69 ) ;
		lcd_hline( 0, 6*FH, 69 ) ;
		lcd_vline( 68, 5*FH, 8 ) ;
		for ( i = 0 ; i <= width ; i += 1 )
		{
			lcd_vline( i, 5*FH, 8 ) ;
		}
	} 
}

extern uint8_t BtScan ;
extern uint8_t BtScanState ;
extern uint8_t BtTempBuffer[] ;
void menuProcBtScan(uint8_t event)
{
	uint32_t i ;
	uint32_t x ;
	uint32_t y ;
  lcd_puts_Pleft( 0, XPSTR("Bt Scan") ) ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, 0 ) ;
	
	if ( event == EVT_ENTRY )
	{
		BtScanState = 0 ;
		BtScan = 1 ;
	}

	lcd_puts_Pleft( 2*FH, (BtScanState < 3) ? XPSTR( "\006Preparing" ) : ( (BtScanState < 4) ? XPSTR( "\006Scanning" ) : XPSTR("\007Scanned") ) )  ;
	
	if ( BtScanState > 2 )
	{
		uint8_t chr ;
		x = 0 ;
		y = 3*FH ;
		i = 0 ;
		while ( (chr = BtTempBuffer[i]) )
		{
			i += 1 ;
			if ( chr == '\r' )
			{
				x = 0 ;
				y += FH ;
			}
			else
			{
				if ( chr >= ' ' )
				{
					if ( y < 7*FH )
					{
						lcd_putc( x, y, chr ) ;
						x += FW ;
						if ( x > 20*FW )
						{
							x = 0 ;
							y += FH ;
						}
					}
				}
			}
		}
		if ( NumberBtremotes )
		{
			i = 0 ;
			x = 0 ;
			while ( (chr = BtRemote[0].name[i++] ) )
			{
				if ( chr >= ' ' )
				{
					lcd_putc( x, 7*FH, chr ) ;
					x += FW ;
				}
				if ( x > 14*FW )
				{
					break ;
				}
			}
		}
	}
	lcd_outhex4( 100, 0*FH, BtScanState ) ;
}



#endif // BLUETOOTH

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
  lcd_outdez( 20*FW, 6*FH, RemBitMeasure ) ;

  lcd_outdez( 14*FW, 7*FH, M64ResetCount ) ;
  lcd_outdez( 20*FW, 7*FH, M64RestartCount ) ;
}
#endif // PCB9XT

extern uint32_t ChipId ;
void menuProcBoot(uint8_t event)
{
  MENU(PSTR(STR_BOOT_REASON), menuTabStat, e_Boot, 1, {0/*, 0*/});

#if defined(PCBSKY) || defined(PCB9XT)
	lcd_puts_Pleft( 7*FH, XPSTR("Chip") ) ;
	lcd_outhex4( 5*FW, 7*FH, ChipId >> 16 ) ;
	lcd_outhex4( 9*FW, 7*FH, ChipId ) ;
#endif	

#ifdef PCBSKY
	if ( ( ResetReason & RSTC_SR_RSTTYP ) == (2 << 8) )	// Watchdog
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_6_WATCHDOG) ) ;
	}
	else if ( unexpectedShutdown )
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_5_UNEXPECTED) ) ;
		lcd_puts_Pleft( 3*FH, PSTR(STR_6_SHUTDOWN) ) ;
	}
	else
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_6_POWER_ON) ) ;
	}
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	if ( ResetReason & RCC_CSR_WDGRSTF )	// Watchdog
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_6_WATCHDOG) ) ;
	}
	else if ( unexpectedShutdown )
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_5_UNEXPECTED) ) ;
		lcd_puts_Pleft( 3*FH, PSTR(STR_6_SHUTDOWN) ) ;
	}
	else
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_6_POWER_ON) ) ;
	}
#endif
  lcd_outdez( 20*FW, 6*FH, ( ResetReason >> 8 ) & 7 ) ;
#if defined(PCBX9D) || defined(PCB9XT)
	lcd_outhex4( 0, 6*FH, ResetReason >> 16 ) ;
	lcd_outhex4( 25, 6*FH, ResetReason ) ;
#endif

}



