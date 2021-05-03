/*
 * Author - Erez Raviv <erezraviv@gmail.com>
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

#include "er9x.h"
#include <stdlib.h>
#include "templates.h"
#include "menus.h"
#include "pulses.h"
#include "voice.h"
#ifdef FRSKY
#include "frsky.h"
// Extra data for Mavlink via FrSky
#if defined(CPUM128) || defined(CPUM2561)
//#include <math.h>
//#define PI 3.14159265
#define MIX_WARN		1
#endif
// Extra data for Mavlink via FrSky
#endif

#include "language.h"

#define TELEMETRY_ARDUPILOT	5

#ifndef V2
#define GLOBAL_COUNTDOWN	1
#endif

#if defined(CPUM2561)
//#if defined(CPUM128) || defined(CPUM2561)
#define THROTTLE_TRACE		1
#else
#define THROTTLE_TRACE		0
//#endif // 128/2561
#endif // 2561

union t_xmem Xmem ;

#define ALPHA_NO_NAME		0x80

struct t_popupData PopupData ;

struct t_menuControl
{
	uint8_t SubmenuIndex ;
	uint8_t LastSubmenuIndex ;
	uint8_t UseLastSubmenuIndex ;
#if defined(CPUM128) || defined(CPUM2561)
	uint8_t SubMenuCall ;
#endif
} MenuControl ;

static uint8_t IlinesCount ;

//extern void putVoiceQueue( uint8_t value ) ;
extern int16_t AltOffset ;
extern uint8_t SystemOptions ;
static uint8_t s_currIdx;
#ifdef FAILSAFE  			
static uint8_t StatusTimer ;
#endif

NOINLINE void resetTimer1(void) ;


struct t_timer s_timer[2] ;

uint8_t RotaryState ;		// Defaults to ROTARY_MENU_LR
uint8_t CalcScaleNest = 0 ;
uint8_t ThrottleStickyOn = 0 ;

uint8_t MultiDataRequest ;

extern int16_t getAltbaroWithOffset() ;

//const prog_char APM Str_Switch_warn[] =  STR_SWITCH_WARN ;

const prog_char APM Str_ALTeq[] = STR_ALTEQ ;
const prog_char APM Str_TXeq[] =  STR_TXEQ ;
const prog_char APM Str_RXeq[] =  STR_RXEQ ;
#ifndef XSW_MOD
const prog_char APM Str_TRE012AG[] = STR_TRE012AG ;
#endif
//const prog_char APM Str_YelOrgRed[] = STR_YELORGRED ;
const prog_char APM Str_A_eq[] =  STR_A_EQ ;
const prog_char APM Str_Timer[] =  STR_TIMER ;
const prog_char APM Str_Sounds[] = STR_SOUNDS ;
const prog_char APM Str_Name[] = STR_NAME ;

const prog_char APM Str_Chans_Gv[] = STR_CHANS_GV ;

const prog_char APM Str_Cswitch[] = CSWITCH_STR ;
const prog_char APM Str_On_Off_Both[] = STR_VOICE_V2OPT ;
const prog_char APM Str_Gv_Source[] = STR_GV_SOURCE ;

const prog_char APM Str_minute_Beep[] = STR_MINUTE_BEEP ;
const prog_char APM Str_Beep_Countdown[] = STR_BEEP_COUNTDOWN ;

const prog_char APM Str_Main_Popup[] = STR_MAIN_POPUP ;
#define Str_Radio_Setup		&Str_Main_Popup[RADIO_SETUP_OFFSET]
#define Str_Model_Setup		&Str_Main_Popup[MODEL_SETUP_OFFSET]

// Strings for titles and indices
const prog_char APM Str_Telemetry[] = STR_TELEMETRY ;
const prog_char APM Str_limits[] = STR_LIMITS ;
const prog_char APM Str_heli_setup[] = STR_HELI_SETUP ;
const prog_char APM Str_Expo[] = STR_EXPO_DR ;
const prog_char APM Str_Modes[] = STR_MODES ;
const prog_char APM Str_Curves[] = STR_CURVES ;
const prog_char APM Str_Curve[] = STR_CURVE ;
const prog_char APM Str_Safety[] = STR_SAFETY_SW2 ;
const prog_char APM Str_Globals[] = STR_GLOBAL_VARS ;
const prog_char APM Str_Protocol[] = STR_PROTOCOL ;
const prog_char APM Str_1_RETA[] = STR_1_RETA ;
const prog_char APM Str_Backup[] = "Backup" ;

// Save flash space by using non-zero terminated strings:
const prog_char APM Str_PlayMute[] = { FW*17, 1, 4, 'P','l','a','y','M','u','t','e' } ;
const prog_char APM Str_FirstLast[] = { FW*12, 1, 5, 'F','i','r','s','t','L','a','s','t',' ' } ;
const prog_char APM Str_SafetySticky[] = { FW*5, 1, 6, 'S','a','f','e','t','y','S','t','i','c','k','y' } ;
const prog_char APM Str_Chans1_8_9_16[] = { 0, 1, 10, 'C','h','a','n','s',' ','1','-','8',' ','C','h','a','n','s',' ','9','-','1','6' } ;
const prog_char APM Str_Telem_NoTelem[] = { 0, 1, 12, 'T','e','l','e','m','e','t','r','y',' ',' ',' ','N','o',' ','T','e','l','e','m','e','t','r','y' } ;
const prog_char APM StrR9mPowerEU[] = { FW*15, 3, 6, ' ',' ','2','5','-','8',' ','2','5','-','1','6','2','0','0','-','1','6','5','0','0','-','1','6' } ;
const prog_char APM StrR9mPowerFCC[] = { FW*17, 3, 4, ' ',' ','1','0',' ','1','0','0',' ','5','0','0','1','0','0','0' } ;
const prog_char APM StrNZ_AxTypes[] = { FW*16, 3, 1, 'v','-','V','A' } ;
const prog_char APM Str_plusStarR[] = {1,'+','*','R'} ;
const prog_char APM SW_3_IDX[] = { 4, 's','I','D','x','s','T','H','R','s','R','U','D','s','E','L','E','s','A','I','L','s','G','E','A','s','T','R','N' } ;
const prog_char APM StrNoBeforeAfter[] = { FW*14, 2, 6, 'N','o',' ',' ',' ',' ','B','e','f','o','r','e','A','f','t','e','r',' ' } ;
const prog_char APM Str_NUmberAudio[] = { FW*14, 2, 6, '-','-','-','-','-','-','N','u','m','b','e','r',' ','A','u','d','i','o' } ;
const prog_char APM Str_ABLRFH[] = { 'A','B','L','R','F','H' } ;
const prog_char APM Str_NY[] = M_NY_STR_NZ ;

//const prog_char APM Str_Timers[] = "Timers" ;
#ifndef NO_TEMPLATES
const prog_char APM Str_Templates[] = STR_TEMPLATES ;
#endif

const prog_char APM Str_Mixer[] = STR_MIXER2 ;

const prog_char APM Curve_Str[] = CURV_STR ;

static uint8_t Columns ;

#if defined(CPUM128) || defined(CPUM2561)
static int16_t hdg_home ;// Extra data for Mavlink via FrSky
#endif

int8_t phyStick[4] ;

const prog_uint8_t APM UnitsVoice[] = {V_FEET,V_VOLTS,V_DEGREES,V_DEGREES,0,V_AMPS,V_METRES,V_WATTS } ;
const prog_char APM UnitsText[] = { 'F','V','C','F','m','A','m','W' } ;
const prog_char APM UnitsString[] = { 5, 'F','e','e','t',' ','V','o','l','t','s','D','e','g','_','C','D','e','g','_','F','m','A','h',' ',' ','A','m','p','s',' ','M','e','t','r','e','W','a','t','t','s' } ;

#ifdef FRSKY

// TSSI set to zero on no telemetry data
const prog_char APM Str_telemItems[] = STR_TELEM_ITEMS ; 
const prog_int8_t APM TelemIndex[] = {FR_A1_COPY, FR_A2_COPY,
															FR_RXRSI_COPY, FR_TXRSI_COPY,
															TIMER1, TIMER2,
															FR_ALT_BARO, FR_GPS_ALT,
															FR_GPS_SPEED, FR_TEMP1, FR_TEMP2, FR_RPM,
														  FR_FUEL, FR_A1_MAH, FR_A2_MAH, FR_CELL_MIN,
															BATTERY, FR_CURRENT, FR_AMP_MAH, FR_CELLS_TOT, FR_VOLTS,
															FR_ACCX, FR_ACCY,	FR_ACCZ, FR_VSPD, V_GVAR1, V_GVAR2,
															V_GVAR3, V_GVAR4, V_GVAR5, V_GVAR6, V_GVAR7, FR_WATT, FR_RXV, FR_COURSE,
															FR_A3, FR_A4, V_SC1, V_SC2, V_SC3, V_SC4, TMOK } ;

const prog_uint8_t APM TelemValid[] = { 1, 1, 1, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 3, 3, 3, 3, 0 } ;

#else
const prog_char APM Str_telemItems[] = STR_TELEM_SHORT ;
const prog_int8_t APM TelemIndex[] = { TIMER1, TIMER2, BATTERY, V_GVAR1, V_GVAR2,	V_GVAR3, V_GVAR4, V_GVAR5, V_GVAR6, V_GVAR7 } ;
const prog_uint8_t APM TelemValid[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;

#endif

#define EDIT_DR_SWITCH_EDIT		0x01
#define EDIT_DR_SWITCH_MOMENT	0x02
int8_t edit_dr_switch( uint8_t x, uint8_t y, int8_t drswitch, uint8_t attr, uint8_t edit ) ;

extern MenuFuncP g_menuStack[] ;
extern uint8_t g_menuStackPtr ;
//extern uint8_t g_menuVertPtr ;
#define g_menuVertPtr g_menuStackPtr

extern uint8_t MenuVertStack[] ;
extern uint8_t MenuVertical ;

static MenuFuncP lastPopMenu()
{
    return  g_menuStack[g_menuStackPtr+1];
}

void menuProcGlobalVoiceAlarm(uint8_t event) ;


#ifdef SWITCH_MAPPING
uint8_t indexSwitch( uint8_t index )
{
	if ( index < 7 )
	{
		index = pgm_read_byte( &Sw2posIndex[index] ) ;
	}
	return index ;
}

uint8_t currentSwitchPosition012( uint8_t i )
{
	uint8_t now ;
	uint8_t index ;
	if ( XBITMASK( i ) & Sw3posMask )
	{
		index = pgm_read_byte( &Sw3posIndex[i] ) ;
		now = switchPosition(index) ;
	}
	else
	{
extern uint8_t indexSwitch( uint8_t index ) ;
		index = indexSwitch( i ) ;
		now = switchPosition(index) ? 0 : 2 ;
	}
	return now ;
}

#endif

void putsAttIdxTelemItems( uint8_t x, uint8_t y, uint8_t index, uint8_t attr )
{
	if ( index == 4 )
	{
		if ( g_model.protocol == PROTO_PXX )
		{
			lcd_putsAtt( x, y, PSTR("SWR "), attr ) ;
			return ;
		}
	}
	lcd_putsAttIdx( x, y, Str_telemItems, index, attr ) ;

}

NOINLINE int16_t m_to_ft( int16_t metres )
{
	int16_t result ;

  // m to ft *105/32
	result = metres * 3 ;
	metres >>= 2 ;
	result += metres ;
	metres >>= 2 ;
  return result + (metres >> 1 );
}

NOINLINE int16_t c_to_f( int16_t degrees )
{
  degrees += 18 ;
  degrees *= 115 ;
  degrees >>= 6 ;
  return degrees ;
}
									 
int16_t calc_scaler( uint8_t index, uint8_t *unit, uint8_t *num_decimals)
{
	int32_t value ;
	uint8_t lnest ;
	ScaleData *pscaler ;
#if defined(EXTEND_SCALERS)
	ExtScaleData *epscaler ;
#endif
	 
	lnest = CalcScaleNest ;
	if ( lnest > 5 )
	{
		return 0 ;
	}
	CalcScaleNest = lnest + 1 ;
	// process
	pscaler = &g_model.Scalers[index] ;
#if defined(EXTEND_SCALERS)
	epscaler = &g_model.eScalers[index] ;
#endif
	if ( pscaler->source )
	{
		value = getValue( pscaler->source - 1 ) ;
#ifdef FRSKY
		if ( ( pscaler->source == NUM_XCHNRAW+1 ) || ( pscaler->source == NUM_XCHNRAW+2 ) )
		{
			value = scale_telem_value( value, pscaler->source - NUM_XCHNRAW-1, NULL ) ;
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
#if defined(EXTEND_SCALERS)
	if ( epscaler->mod )
	{
		value %= epscaler->mod ;
	}
#endif
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
#if defined(EXTEND_SCALERS)
	if ( epscaler->dest )
	{
		store_telemetry_scaler( epscaler->dest, value ) ;
	}
#endif

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
  	if(i<CHOUT_BASE+NUM_CHNOUT)
		{
			x = 0 ;
		}
		else
		{
			i -= CHOUT_BASE+NUM_CHNOUT ;
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

#define WBAR2 (50/2)
void singleBar( uint8_t x0, uint8_t y0, int16_t val )
{
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
}


extern uint8_t Unit ;

void voiceMinutes( int16_t value )
{
	voice_numeric( value, 0, (abs(value) == 1) ? V_MINUTE : V_MINUTES ) ;
}

void voice_telem_item( uint8_t indexIn )
{
	int16_t value ;
	uint8_t spoken = 0 ;
	uint8_t unit = 0 ;
	uint8_t num_decimals = 0 ;

	value = get_telemetry_value( indexIn ) ;
	if (telemItemValid( indexIn ) == 0 )
	{
		putVoiceQueue( V_NOTELEM ) ;
		spoken = 1 ;
	}
	int8_t index = pgm_read_byte( &TelemIndex[indexIn] ) ;

  switch (index)
	{
		case V_SC1 :
		case V_SC2 :
		case V_SC3 :
		case V_SC4 :
			value = calc_scaler( index-V_SC1, &unit, &num_decimals ) ;
			unit = pgm_read_byte( &UnitsVoice[unit]) ;
		break ;
		 
		case BATTERY:
#ifdef FRSKY
		case FR_VOLTS :
		case FR_CELLS_TOT :
#endif
			unit = V_VOLTS ;			
			num_decimals = 1 ;
		break ;

#ifdef FRSKY
		case FR_CELL_MIN:
			unit = V_VOLTS ;			
			num_decimals = 2 ;
		break ;
#endif
			
		case TIMER1 :
		case TIMER2 :
		{	
			div_t qr ;
			qr = div( value, 60 ) ;
			voiceMinutes( qr.quot ) ;
			value = qr.rem ;
			unit = V_SECONDS ;			
		}
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
			value = scale_telem_value( value, index-FR_A1_COPY, &num_decimals ) ;
			unit = V_VOLTS ;
			if (Unit == 'A')
			{
				unit = V_AMPS ;
			}
			else if (Unit == 1)
			{
				unit = 0 ;
			}
		}
		break ;

//    case FR_RXV :
//			value = convertRxv( value ) ;
//			unit = V_VOLTS ;			
//			num_decimals = 1 ;
//		break ;

		case FR_ALT_BARO:
      unit = V_METRES ;
			if (g_model.FrSkyUsrProto == 1)  // WS How High
			{
      	if ( g_model.FrSkyImperial )
        	unit = V_FEET ;
			}
      else
			{
				if ( g_model.FrSkyImperial )
      	{
	        // m to ft *105/32
  	      value = m_to_ft( value ) ;
    	    unit = V_FEET ;
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
		 
#if defined(CPUM128) || defined(CPUM2561)
// Extra data for Mavlink via FrSky		 
		case FR_WP_DIST:
  		if ( g_model.FrSkyImperial )
  		{
				value = m_to_ft(value) ;
			}
		break ;
// Extra data for Mavlink via FrSky
#endif
		case FR_CURRENT :
			num_decimals = 1 ;
      unit = V_AMPS ;
		break ;
			
		case FR_TEMP1:
		case FR_TEMP2:
			unit = V_DEGREES ;			
  		if ( g_model.FrSkyImperial )
  		{
				value = c_to_f(value) ;
			}
		break ;

		case FR_WATT :
			unit = V_WATTS ;
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
int16_t convertTelemConstant( uint8_t channelin, int8_t value)
{
  int16_t result;
	int8_t channel ;

	channel = pgm_read_byte( &TelemIndex[channelin] ) ;
	result = value + 125 ;
	if ( ( channel <= V_GVAR7 ) && ( channel >= V_GVAR1 ) )
	{
		return value ;
	}
  switch (channel)
	{
		// unneeded cases:
		// case FR_A1_COPY :
		// case FR_A2_COPY :
		// case FR_RXRSI_COPY :
		// case FR_TXRSI_COPY :
	
		// cases not implemented, to consider:	 
		// case FR_GPS_SPEED :
		// case FR_FUEL :
		// case FR_CURRENT :
		
    case TIMER1 :
    case TIMER2 :
      result *= 10 ;
    break;
#ifdef FRSKY
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
        result = m_to_ft( result ) ;
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
      result *= 50;
    break;

		case FR_CELL_MIN:
      result *= 2;
		break ;
		case FR_CELLS_TOT :
		case FR_VOLTS :
      result *= 2;
		break ;
    case FR_WATT:
      result *= 8 ;
    break;
		case FR_VSPD :
			result = value * 10 ;
		break ;
#endif
  }
  return result;
}


int16_t get_telemetry_value( uint8_t channelIn )
{
	int8_t channel = pgm_read_byte( &TelemIndex[channelIn] ) ;

  if ( channel == TMOK )
	{
#ifdef FRSKY
		return TmOK ;
#else
		return 0 ;
#endif
	}
#ifdef FRSKY
  if ( channel == FR_WATT )
	{
		return (getTelemetryValue(FR_VOLTS)>>1) * ( getTelemetryValue(FR_CURRENT)>>1) / 25 ;
	}	 
	if ( channel < -11 )	// A Scaler
	{
		return calc_scaler(channel-V_SC1, 0, 0 ) ;
	}
#endif
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
		return getAltbaroWithOffset() ;
		
    case FR_CELL_MIN :
		return getTelemetryValue(channel) * 2 ;

		default :
		return getTelemetryValue(channel) ;
#else
		default :
		return 0 ;
#endif
  }
}

void displayTimer( uint8_t x, uint8_t y, uint8_t timer, uint8_t att )
{
	struct t_timer *tptr = &s_timer[timer] ;
	FORCE_INDIRECT(tptr) ;
//  att |= (tptr->s_timerState==TMR_BEEPING ? BLINK : 0);
  putsTime( x, y, tptr->s_timerVal, att, att ) ;
}

// Styles
#define TELEM_LABEL				0x01
#define TELEM_UNIT    		0x02
#define TELEM_UNIT_LEFT		0x04
#define TELEM_VALUE_RIGHT	0x08
#define TELEM_CONSTANT		0x80

uint8_t putsTelemetryChannel(uint8_t x, uint8_t y, int8_t channel, int16_t val, uint8_t att, uint8_t style)
{
	uint8_t unit = ' ' ;
	uint8_t xbase = x ;
	uint8_t fieldW = FW ;
	uint8_t displayed = 0 ;
	int8_t chanIndex ;

	chanIndex = pgm_read_byte( &TelemIndex[channel] ) ;
	if ( style & TELEM_LABEL )
	{
		uint8_t displayed = 0 ;
		int8_t index = chanIndex ;
		if ( (index >= V_SC1) && (index < V_SC1 + NUM_SCALERS) )
		{
			index -= V_SC1 ;
			uint8_t *p = &g_model.Scalers[index].name[0] ;
			if ( *p )
			{
				lcd_putsnAtt( x, y, (const char *)p, 4, BSS ) ;
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
	channel = chanIndex ;
  switch (channel)
	{
		case V_SC1 :
		case V_SC2 :
		case V_SC3 :
		case V_SC4 :
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
			if ( (style & TELEM_CONSTANT) == 0)
			{
				val = cvalue ;
			}
			// Sort units here
			unit = pgm_read_byte( &UnitsText[unit]) ;
		}	
		break ;
		
    case TIMER1 :
    case TIMER2 :
			if ( (att & DBLSIZE) == 0 )
			{
				x -= 4 ;
			}
#ifdef SMALL_DBL
			else
			{
				x += 2 ;
			}
#endif // SMALL_DBL
			if ( style & TELEM_LABEL )
			{
				x += FW+4 ;
			}
			att &= DBLSIZE | INVERS | BLINK ;
#ifdef SMALL_DBL
      putsTime(x-FW-2+5, y, val, att, att) ;
#else
      putsTime(x-FW, y, val, att, att) ;
#endif // SMALL_DBL
			displayed = 1 ;
			xbase -= FW ;
    	unit = channel + 2 + '1';
		break ;
    
#ifdef FRSKY
		case FR_A1_COPY:
    case FR_A2_COPY:
      channel -= FR_A1_COPY ;
			displayed = 1 ;
			unit = putsTelemValue( (style & TELEM_VALUE_RIGHT) ? xbase+61 : x-fieldW, y, val, channel, att|NO_UNIT/*|blink*/ ) ;
    break ;

		case FR_RXV:
  		unit = 'v' ;
			att |= PREC1 ;
//			val = convertRxv( val ) ;
    break ;

    case FR_TEMP1:
    case FR_TEMP2:
			unit = 'C' ;
  		if ( g_model.FrSkyImperial )
  		{
				val = c_to_f(val) ;
				x -= fieldW ;
  		  unit = 'F' ;
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

		  if (channel == FR_ALT_BARO )
			{
				if ( val < 1000 )
				{
					att |= PREC1 ;
				}
				else
				{
					val /= 10 ;
				}
			}
    break;
		
		case FR_CURRENT :
			att |= PREC1 ;
      unit = 'A' ;
		break ;

		case FR_CELL_MIN:
			att |= PREC2 ;
      unit = 'v' ;
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
		case FR_VSPD :
			att |= PREC1 ;
			val /= 10 ;
		break ;
    default:
    break;
#endif
  }
	if ( !displayed )
	{
  	lcd_outdezAtt( (style & TELEM_VALUE_RIGHT) ? xbase+61 : x, y, val, att ) ;
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
  	lcd_putcAtt( x, y, unit, att);
	}
	return unit ;
}



const prog_char APM Str_GV[] = STR_GV ;
const prog_char APM Str_SC[] = "SC" ;

void dispGvar( uint8_t x, uint8_t y, uint8_t gvar, uint8_t attr )
{
	lcd_putsAtt( x, y, Str_GV, attr ) ;
	lcd_putcAtt( x+2*FW, y, gvar+'0', attr ) ;
}


int8_t gvarMenuItem(uint8_t x, uint8_t y, int8_t value, int8_t min, int8_t max, uint8_t attr )
{
  uint8_t invers = attr&(INVERS|BLINK);

  if (value >= 126 || value <= -126)
	{
		dispGvar( x-3*FW, y, (uint8_t)value - 125, attr ) ;
    if (invers) value = checkIncDec16((uint8_t)value, 126, 130, EE_MODEL);
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
  return value ;
}

int8_t gvarMenuItem125(uint8_t x, uint8_t y, int8_t value, uint8_t attr )
{
	return gvarMenuItem( x, y, value, -125, 125, attr ) ;
}


uint8_t get_dr_state(uint8_t x)
{
	ExpoData *ped ;

	ped = &g_model.expoData[x] ;
	
 	return (!getSwitch00( ped->drSw1) ? DR_HIGH :
    !getSwitch00( ped->drSw2) ? DR_MID : DR_LOW) ;
}

static void DO_SQUARE(uint8_t x, uint8_t y, uint8_t w)
{
		lcd_rect( x-w/2, y-w/2, w, w ) ;
}
#define DO_CROSS(xx,yy,ww)          \
    lcd_vline(xx,yy-ww/2,ww);  \
    lcd_hline(xx-ww/2,yy,ww);  \

#define V_BAR(xx,yy,ll)       \
    lcd_vline(xx-1,yy-ll,ll); \
    lcd_vline(xx  ,yy-ll,ll); \
    lcd_vline(xx+1,yy-ll,ll); \

#define NO_HI_LEN 25

#define WCHART 32
#define X0     (128-WCHART-2)
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

#define TRIM_EXTENDED_MAX	500

enum MainViews {
    e_outputValues,
    e_outputBars,
    e_inputs1,
    e_timer2,
#if (defined(FRSKY) | defined(HUB))
    e_telemetry,
#endif
    MAX_VIEWS
};

int16_t calibratedStick[7];
int16_t Ex_chans[NUM_CHNOUT];          // Outputs + intermidiates
uint8_t s_pgOfs;
uint8_t s_editMode;
uint8_t s_editing;
uint8_t s_noHi;
#ifndef NOPOTSCROLL
int8_t scrollLR;
uint8_t scroll_disabled;
int8_t scrollUD;
#endif
uint8_t InverseBlink ;
uint8_t EditType ;
uint8_t EditColumns ;

int16_t g_chans512[NUM_CHNOUT];

extern MixData *mixaddress( uint8_t idx ) ;
//extern LimitData *limitaddress( uint8_t idx ) ;

const
#include "sticks.lbm"
//typedef PROGMEM void (*MenuFuncP_PROGMEM)(uint8_t event);

#if defined(CPUM128) || defined(CPUM2561)
const prog_uchar speaker[] = {
4,8,0,
0x1C,0x1C,0x3E,0x7F
} ;
#endif

void menuProcAlpha(uint8_t event) ;
void menuProcIndex(uint8_t event) ;

void menuProcModelIndex(uint8_t event) ;

#define MENU_TAB_NONE		0
#define MENU_TAB_MODEL	1
#define MENU_TAB_DIAG		2

#define SIZE_MTAB_MODEL	DIM(menuTabModel)

int8_t qRotary()
{
	int8_t diff = 0 ;

	if ( Rotary.Rotary_diff > 0)
	{
		diff = 1 ;
	}
	else if ( Rotary.Rotary_diff < 0)
	{
		diff = -1 ;
	}
	Rotary.Rotary_diff = 0 ;
	return diff ;
}

#if defined(CPUM128) || defined(CPUM2561)

void validateText( uint8_t *text, uint8_t length )
{
	for( uint8_t i=0 ; i<length ; i += 1 ) // makes sure text is valid
  {
		if ( text[i] < ' ' )
		{
			text[i] = ' ' ;
		}
//    uint8_t idx = char2idx(text[i]);
//    text[i] = idx2char(idx);
  }
}

void alphaEditName( uint8_t x, uint8_t y, uint8_t *name, uint8_t len, uint8_t type, const char *heading )
{
	if ( ( type & ALPHA_NO_NAME ) == 0 )
	{
		lcd_puts_Pleft( y, Str_Name ) ;
	}
	validateText( name, len ) ;
	lcd_putsnAtt( x, y, (char *)name, len, BSS ) ;
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
#endif

void editName( uint8_t x, uint8_t y, uint8_t *name, uint8_t len, uint8_t type )
{
	lcd_puts_Pleft( y, Str_Name) ;
	lcd_putsnAtt( 11*FW, y, (const char *)name, len, BSS ) ;
	if( type )
	{
		lcd_char_inverse( (11+x)*FW, y, 1*FW, s_editMode ) ;
		lcd_rect( 11*FW-2, y-1, len*FW+4, 9 ) ;
	  if(s_editMode)
		{
     	char v = name[x] ;
			if ( v )
			{
	  	  v = char2idx(v) ;
			}
			v = checkIncDec( v, 0, NUMCHARS-1, type ) ;
  	  v = idx2char(v);
			if ( name[x] != v )
			{
				name[x] = v ;
    		eeDirty( EditType ) ;				// Do here or the last change is not stored in name[]
			}
		}
	}
	asm("") ;
}


uint8_t TITLEP( const prog_char *pstr) { return lcd_putsAtt(0,0,pstr,INVERS) ; }

#define PARAM_OFS   17*FW

void lcd_xlabel_decimal( uint8_t x, uint8_t y, uint16_t value, uint8_t attr, const prog_char * s )
{
  lcd_outdezAtt( x, y, value, attr ) ;
	lcd_puts_Pleft( y, s ) ;
}

#define FWx3		"\022"
#define FWx4		"\030"
#define FWx5		"\036"
#define FWx6		"\044"
#define FWx7		"\052"
#define FWx8		"\060"
#define FWx9		"\066"
#define FWx10		"\074"
#define FWx11		"\102"
#define FWx12		"\110"
#define FWx13		"\116"
#define FWx14		"\124"
#define FWx15		"\132"
#define FWx16		"\140"
#define FWx17		"\146"
#define FWx18		"\152"


uint8_t checkIndexed( uint8_t y, const prog_char * s, uint8_t value, uint8_t edit )
{
	uint8_t x ;
	uint8_t max ;

//#if defined(CPUM128) || defined(CPUM2561)
#if 1
	if ( s )
	{
		x = pgm_read_byte(s++) ;
		max = pgm_read_byte(s++) ;
		lcd_putsAttIdx( x, y, s, value, edit ? InverseBlink: 0 ) ;
	}
	else
	{
		x = PARAM_OFS ;
		max = 1 ;
  	if (value)
		{
    	lcd_putc(x+1, y, '\202');
		}
		lcd_hbar( x, y, 7, 7, edit ? 100 : 0 ) ;
	}
	if(edit)
	{
		if ( ( EditColumns == 0 ) || ( s_editMode ) )
		{
			value = checkIncDec( value, 0, max, EditType ) ;
		}
	}
//#else
//	x = pgm_read_byte(s++) ;
//	max = pgm_read_byte(s++) ;
	
//	if(edit)
//	{
//		if ( ( EditColumns == 0 ) || ( s_editMode ) )
//		{
//			value = checkIncDec( value, 0, max, EditType ) ;
//		}
//	}
//	lcd_putsAttIdx( x, y, s, value, edit ? InverseBlink: 0 ) ;
#endif
	return value ;
}

uint8_t checkIndexedV( uint8_t y, const prog_char * s, uint8_t value, uint8_t vertical )
{
	return checkIndexed( y, s, value, vertical == MenuVertStack[g_menuVertPtr] ) ;
}


uint8_t hyphinvMenuItem( uint8_t value, uint8_t y, uint8_t condition )
{
	return checkIndexed( y, PSTR(FWx18"\001"STR_HYPH_INV), value, condition ) ;
}




void putsTxStr( uint8_t x, uint8_t y )
{
	lcd_putsAttIdx( x, y, Str_TXeq, ( g_model.protocol == PROTO_PXX ), 0 ) ;
}

void putsOffDecimal( uint8_t x, uint8_t y, uint16_t value, uint8_t attr )
{
  if(value)
      lcd_outdezAtt( x+3*FW-2,y,value,attr) ;
  else
      lcd_putsAtt(  x,y,Str_OFF,attr) ;
}

// Extra data for Mavlink via FrSky
#if defined(CPUM128) || defined(CPUM2561)
//const int8_t COS[] = { 100,97,87,71,50,26,0,-26,-50,-71,-87,-97,-100,-97,-87,-71,-50,-26,0,26,50,71,87,97,100 };// Extra data for Mavlink via FrSky
const prog_int8_t APM SIN[] = { 0,26,50,71,87,97,100,97,87,71,50,26,0,-26,-50,-71,-87,-97,-100,-97,-87,-71,-50,-26,0,26,50,71,87,97,100    };// Extra data for Mavlink via FrSky

uint16_t normalize( int16_t alpha )
{
   if ( alpha > 360 ) alpha -= 360;
   if ( alpha <   0 ) alpha = 360 + alpha;
   alpha /= 15;
   return alpha ;
}
int8_t rcos100( int16_t alpha )
{
   alpha = normalize( alpha );
//   return COS[alpha];
   return pgm_read_byte( &SIN[alpha+6]) ;
}
int8_t rsin100( int16_t alpha )
{
   alpha = normalize( alpha );
   return pgm_read_byte( &SIN[alpha]) ;
}
#endif
// Extra data for Mavlink via FrSky
//static void DisplayScreenIndex(uint8_t index, uint8_t count, uint8_t attr)
//{
//		uint8_t x ;
//		if ( RotaryState == ROTARY_MENU_LR )
//		{
//			attr = BLINK ;
//		}
//    lcd_outdezAtt(127,0,count,attr);
//		x = 1+128-FW*(count>9 ? 3 : 2) ;
//    lcd_putcAtt(x,0,'/',attr);
//    lcd_outdezAtt(x-1,0,index+1,attr);
////		lcd_putc( x-12, 0, RotaryState + '0' ) ;
////    lcd_outdezAtt(64,0,MixRate,0) ;

//}

uint8_t g_posHorz ;
uint8_t M_longMenuTimer ;

//uint8_t MAXCOL( uint8_t row, const prog_uint8_t *horTab, uint8_t horTabMax)
//{
//	return (horTab ? pgm_read_byte(horTab+min(row, horTabMax)) : (const uint8_t)0) ;
//}

#define INC(val,max) if(val<max) {val++;} else {val=0;}
#define DEC(val,max) if(val>0  ) {val--;} else {val=max;}


uint8_t check_columns( uint8_t event, uint8_t maxrow)
{
	uint8_t l_posHorz ;
	uint8_t l_posVert ;
	l_posHorz = g_posHorz ;
	l_posVert = MenuVertStack[g_menuVertPtr] ;
//	l_posVert = m_posVert ;
#ifndef NOPOTSCROLL
	int16_t c4, c5 ;
	struct t_p1 *ptrp1 ;
    
		//    scrollLR = 0;
    //    scrollUD = 0;

    //check pot 2 - if changed -> scroll menu
    //check pot 3 if changed -> cursor down/up
    //we do this in these brackets to prevent it from happening in the main screen
		c4 = calibratedStick[4] ;		// Read only once
		c5 = calibratedStick[5] ;		// Read only once
		
		ptrp1 = &P1values ;
		FORCE_INDIRECT(ptrp1) ;
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

    if(scrollLR || scrollUD || ptrp1->p1valdiff) backlightKey() ; // on keypress turn the light on
						// *250 then <<1 is the same as *500, but uses less code space
#endif

//    uint8_t maxcol = MAXCOL(m_posVert, horTab, horTabMax);
    uint8_t maxcol = Columns ; //MAXCOL(m_posVert, horTab, horTabMax);

	 if ( maxrow != 0xFF )
	 {
		if ( RotaryState == ROTARY_MENU_UD )
		{
			static uint8_t lateUp = 0 ;
			if ( lateUp )
			{
				lateUp = 0 ;
				l_posHorz = maxcol ; //MAXCOL(m_posVert, horTab, horTabMax) ;
			}
		 	int8_t diff = qRotary() ;
			if ( diff > 0 )
			{
        INC(l_posHorz,maxcol) ;
				if ( l_posHorz == 0 )
				{
	        INC(l_posVert,maxrow);
				}
			}
			else if ( diff < 0 )
			{
				if ( l_posHorz == 0 )
				{
      	  DEC(l_posVert,maxrow);
					lateUp = 1 ;
					l_posHorz = 0 ;
				}
				else
				{
      	  DEC(l_posHorz,maxcol) ;
				}
			}
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



//    maxcol = MAXCOL(m_posVert, horTab, horTabMax);
		EditColumns = maxcol ;

#ifndef NOPOTSCROLL
    if(!s_editMode)
    {
        if(scrollUD)
        {
            int8_t cc = l_posVert - scrollUD;
            if(cc<1) cc = 0;
            if(cc>=maxrow) cc = maxrow;
            l_posVert = cc;

            l_posHorz = min(l_posHorz, maxcol ) ; //MAXCOL(m_posVert, horTab, horTabMax));
            BLINK_SYNC;

            scrollUD = 0;
        }

        if(l_posVert>0 && scrollLR)
        {
            int8_t cc = l_posHorz - scrollLR;
            if(cc<1) cc = 0;
//            if(cc>=MAXCOL(m_posVert, horTab, horTabMax)) cc = MAXCOL(m_posVert, horTab, horTabMax);
            if(cc>=maxcol) cc = maxcol ;
            l_posHorz = cc;

            BLINK_SYNC;
            //            scrollLR = 0;
        }
    }
#endif		
		switch(event)
    {
    case EVT_ENTRY:
				l_posVert = 0 ;
//        init();
        l_posHorz = 0 ;
        s_editMode = false;
        break;
    case EVT_KEY_BREAK(BTN_RE):
    case EVT_KEY_FIRST(KEY_MENU):
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
        popMenu(true) ;
        break;
        //fallthrough
    case EVT_KEY_LONG(BTN_RE):
        killEvents(event);
    case EVT_KEY_BREAK(KEY_EXIT):
        if(s_editMode) {
            s_editMode = false;
            break;
        }
            popMenu();  //beeps itself
        break;

    case EVT_KEY_REPT(KEY_RIGHT):  //inc
        if(l_posHorz==maxcol) break;
    case EVT_KEY_FIRST(KEY_RIGHT)://inc
        if(!maxcol || s_editMode)break;
        INC(l_posHorz,maxcol);
				if ( maxcol )
				{
					Tevent = 0 ;
				}
        BLINK_SYNC;
        break;

    case EVT_KEY_REPT(KEY_LEFT):  //dec
        if(l_posHorz==0) break;
    case EVT_KEY_FIRST(KEY_LEFT)://dec
        if(!maxcol || s_editMode)break;
        DEC(l_posHorz,maxcol);
				if ( maxcol )
				{
					Tevent = 0 ;
				}
        BLINK_SYNC;
        break;

    case EVT_KEY_REPT(KEY_DOWN):  //inc
        if(l_posVert==maxrow) break;
    case EVT_KEY_FIRST(KEY_DOWN): //inc
        if(s_editMode)break;
        INC(l_posVert,maxrow);
//        l_posHorz = min(l_posHorz, MAXCOL(m_posVert, horTab, horTabMax));
        l_posHorz = min(l_posHorz, maxcol ) ; //MAXCOL(m_posVert, horTab, horTabMax));
        BLINK_SYNC;
        break;

    case EVT_KEY_REPT(KEY_UP):  //dec
        if(l_posVert==0) break;
    case EVT_KEY_FIRST(KEY_UP): //dec
        if(s_editMode)break;
        DEC(l_posVert,maxrow);
//        l_posHorz = min(l_posHorz, MAXCOL(m_posVert, horTab, horTabMax));
        l_posHorz = min(l_posHorz, maxcol ) ; // MAXCOL(m_posVert, horTab, horTabMax));
        BLINK_SYNC;
        break;
    }
#ifndef NOPOTSCROLL
		s_editing = s_editMode || P1values.p1valdiff ;
#else
		s_editing = s_editMode ;
#endif	
	InverseBlink = (s_editMode) ? BLINK : INVERS ;
	Columns = 0 ;
	g_posHorz = l_posHorz ;
//	MenuVertStack[g_menuStackPtr] = m_posVert = l_posVert ;
	MenuVertStack[g_menuVertPtr] = l_posVert ;
	return l_posVert ;
}

#define BOX_WIDTH     23
#define BAR_HEIGHT    (BOX_WIDTH-1l)
#define MARKER_WIDTH  5
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define BOX_LIMIT     (BOX_WIDTH-MARKER_WIDTH)
#define LBOX_CENTERX  (  SCREEN_WIDTH/4 + 10)
#define BOX_CENTERY  (SCREEN_HEIGHT-9-BOX_WIDTH/2-1)
#define RBOX_CENTERX  (3*SCREEN_WIDTH/4 - 10)
//#define BOX_CENTERY  (SCREEN_HEIGHT-9-BOX_WIDTH/2)


void telltale( uint8_t centrex, int8_t xval, int8_t yval )
{
  DO_SQUARE( centrex, BOX_CENTERY, BOX_WIDTH ) ;
  DO_CROSS( centrex, BOX_CENTERY,3 ) ;
	lcd_rect_xor( centrex +( xval/((2*RESX/16)/BOX_LIMIT))-MARKER_WIDTH/2, BOX_CENTERY-( yval/((2*RESX/16)/BOX_LIMIT))-MARKER_WIDTH/2, MARKER_WIDTH, MARKER_WIDTH ) ;
//	DO_SQUARE( centrex +( xval/((2*RESX/16)/BOX_LIMIT)), BOX_CENTERY-( yval/((2*RESX/16)/BOX_LIMIT)), MARKER_WIDTH ) ;
}

void doMainScreenGrphics()
{
	{	
		int8_t *cs = phyStick ;
		FORCE_INDIRECT(cs) ;
	
		telltale( LBOX_CENTERX, cs[0], cs[1] ) ;
		telltale( RBOX_CENTERX, cs[3], cs[2] ) ;
	}
    
    // Optimization by Mike Blandford
	int16_t *cs = calibratedStick ;
	FORCE_INDIRECT(cs) ;
  {
    uint8_t x, y, len ;			// declare temporary variables
    for( x = -5, y = 4 ; y < 7 ; x += 5, y += 1 )
    {
      len = ((cs[y]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
      V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;
    }
  }
}

extern int16_t calcExpo( uint8_t channel, int16_t value ) ;

static uint8_t s_curveChan ;
static uint8_t s_expoChan ;

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


NOINLINE static int8_t *curveAddress( uint8_t idx )
{
  uint8_t cv9 = idx >= MAX_CURVE5 ;
	return cv9 ? g_model.curves9[idx-MAX_CURVE5] : g_model.curves5[idx] ;
}

void drawCurve( uint8_t offset )
{
#if defined(XYCURVE)
  uint8_t cv9 = s_curveChan >= MAX_CURVE5 ;
	int8_t *crv = curveAddress( s_curveChan ) ;
	uint8_t points = cv9 ? 9 : 5 ;

	if ( s_curveChan >= MAX_CURVE5 + MAX_CURVE9 )
	{
		cv9 = 2 ;
		crv = g_model.curvexy ;		
	}
	lcd_vline(XD, Y0 - WCHART, WCHART * 2);
  
//	plotType = PLOT_BLACK ;
	for(uint8_t i=0; i < points ; i++)
  {
    uint8_t xx ;
		if ( cv9 == 2 )
		{
    	xx = XD-1+crv[i+9]*WCHART/100 ;
		}
		else
		{
			xx = XD-1-WCHART+i*WCHART/(cv9 ? 4 : 2);
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
#else
  uint8_t cv9 = s_curveChan >= MAX_CURVE5 ;
	int8_t *crv = curveAddress( s_curveChan ) ;

	lcd_vline(XD, Y0 - WCHART, WCHART * 2);
  
//	plotType = PLOT_BLACK ;
	for(uint8_t i=0; i<(cv9 ? 9 : 5); i++)
  {
    uint8_t xx = XD-1-WCHART+i*WCHART/(cv9 ? 4 : 2);
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
#endif	
//	plotType = PLOT_XOR ;
}



void menuProcCurveOne(uint8_t event)
{
  uint8_t cv9 = s_curveChan >= MAX_CURVE5;
	static int8_t dfltCrv;
#if defined(XYCURVE)
	uint8_t points = cv9 ? 9 : 5 ;
	if ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 )
	{
		cv9 = 2 ;
	}
#endif
	TITLEP(Str_Curve) ;
//	static MState2 mstate2 ;
	uint8_t sub ;

#if defined(XYCURVE)
	sub = check_columns(event, (cv9 == 2) ? 17 : points ) ;
#else
	sub = check_columns(event, (cv9 ? 9 : 5) ) ;
#endif
    
	if ( event == EVT_ENTRY )
	{
		dfltCrv = 0 ;
	}
	lcd_outdezAtt(7*FW, 0, s_curveChan+1, INVERS);

	int8_t *crv = curveAddress( s_curveChan ) ;

#if defined(XYCURVE)
	uint8_t  preset = points ;
	uint8_t blink = InverseBlink ;
	if ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 )
	{
		crv = g_model.curvexy ;
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
		for ( i = 0; i < 7; i++)
		{
  	  uint8_t y = i * FH + 8 ;
  	  uint8_t attr = (k==0) && (sub == j+i+9) ? blink : 0 ;
			lcd_outdezAtt(4 * FW, y, crv[j+i+9], attr);
  	  attr = (k==1) && (sub == j+i) ? blink : 0 ;
    	lcd_outdezAtt(8 * FW, y, crv[j+i], attr);
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

#else
	uint8_t blink = InverseBlink ;
	uint8_t  preset = cv9 ? 9 : 5 ;

	for (uint8_t i = 0; i < 5; i++)
	{
    uint8_t y = i * FH + 16;
    uint8_t attr = sub == (i) ? blink : 0;
    lcd_outdezAtt(4 * FW, y, crv[i], attr);
		if ( cv9 )
		{
			if ( i < 4 )
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
    dfltCrv = checkIncDec( t, -4, 4, 0);
    if (dfltCrv != t)
		{
			uint8_t offset = cv9 ? 4 : 2 ;

			for (int8_t i = -offset; i <= offset; i++) crv[i+offset] = i*dfltCrv* 25 / offset ;
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
#endif
}



void menuProcCurve(uint8_t event)
{
	TITLEP(Str_Curves) ;
//	static MState2 mstate2 ;
  int8_t sub ;
#if defined(CPUM128) || defined(CPUM2561)
		sub = check_columns(event,1+MAX_CURVE5+MAX_CURVE9-1-1+1) ;
#else
		sub = check_columns(event,1+MAX_CURVE5+MAX_CURVE9-1-1) ;
#endif

    uint8_t t_pgOfs = evalOffset(sub);

    switch (event)
		{
//	    case EVT_KEY_FIRST(KEY_RIGHT):
  	  case EVT_KEY_FIRST(KEY_MENU):
    	case EVT_KEY_BREAK(BTN_RE):
				if (sub >= 0)
				{
          s_curveChan = sub;
      		killEvents(event);
					Tevent = 0 ;
          pushMenu(menuProcCurveOne);
        }
      break ;
    }

    uint8_t y    = 1*FH;
    for (uint8_t i = 0; i < 7; i++)
		{
        uint8_t k = i + t_pgOfs;
        uint8_t attr = sub == k ? INVERS : 0;
//        if(y>7*FH) break;
        lcd_putsAtt(   FW*0, y,PSTR(STR_CV),attr);
        lcd_outdezAtt( (k<9) ? FW*3-1 : FW*4-2, y,k+1 ,attr);

        y += FH; // yd++;
    }

		if ( sub >= 0 )
		{
  		s_curveChan = sub ;
			drawCurve( 100 ) ;
		}
	asm("") ;
}

void setStickCenter() // copy state of 3 primary to subtrim
{
	uint8_t thisPhase ;
	struct t_stickCentre *data = &Xmem.stickCentreData ;

//  int16_t zero_chans512_before[NUM_CHNOUT];
//  int16_t zero_chans512_after[NUM_CHNOUT];

	thisPhase = getFlightPhase() ;
	CurrentPhase = thisPhase ;

    perOut(data->zero_chans512_before,NO_TRAINER | NO_INPUT | FADE_FIRST | FADE_LAST); // do output loop - zero input channels
    perOut(data->zero_chans512_after,NO_TRAINER | FADE_FIRST | FADE_LAST); // do output loop - zero input channels
//    perOut(zero_chans512_before,NO_TRAINER | NO_INPUT | FADE_FIRST | FADE_LAST); // do output loop - zero input channels
//    perOut(zero_chans512_after,NO_TRAINER | FADE_FIRST | FADE_LAST); // do output loop - zero input channels

    for(uint8_t i=0; i<NUM_CHNOUT; i++)
    {
        int16_t v = g_model.limitData[i].offset;
				int16_t b ;
				b = data->zero_chans512_before[i] - data->zero_chans512_after[i] ;
//				b = zero_chans512_before[i] - zero_chans512_after[i] ;
        v += g_model.limitData[i].reverse ? b : -b ;
//                    (zero_chans512_before[i] - zero_chans512_after[i]) :
//                    (zero_chans512_after[i] - zero_chans512_before[i]);

				if ( v > 1000 )
				{
					v = 1000 ;
				}
				if ( v < -1000 )
				{
					v = -1000 ;
				}
        g_model.limitData[i].offset = v ;
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

static void menuProcLimits(uint8_t sub)
{
	TITLEP(Str_limits);

	IlinesCount = NUM_CHNOUT+2-1 ;
	Columns = 3 ;
uint8_t y = 0;
uint8_t k = 0;
uint8_t subSub = g_posHorz;
    uint8_t t_pgOfs ;

t_pgOfs = evalOffset(sub);

	if ( sub < NUM_CHNOUT )
	{
    lcd_outdez( 13*FW, 0, g_chans512[sub]/2 + 1500 ) ;
	}
	
	switch(Tevent)
	{
    case EVT_KEY_LONG(KEY_MENU):
        if(sub>=0 && sub<NUM_CHNOUT)
				{
            int16_t v = g_chans512[sub - t_pgOfs];
            LimitData *ld = &g_model.limitData[sub] ;
            if ( subSub == 0 )
						{
                ld->offset = (ld->reverse) ? -v : v;
                STORE_MODELVARS;
                eeWaitComplete() ;
            }
        }
				else if(sub==NUM_CHNOUT)
				{
	        s_noHi = NO_HI_LEN;
  	      killEvents(Tevent);
    	    setStickCenter(); //if highlighted and menu pressed - copy trims
				}
    break;
	}
//  lcd_puts_P( 4*FW, 1*FH,PSTR("subT min  max inv"));
	for(uint8_t i=0; i<7; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    if(k==NUM_CHNOUT) break;
//    LimitData *ld = limitaddress( k ) ;
    LimitData *ld = &g_model.limitData[k] ;
	FORCE_INDIRECT(ld) ;
    int16_t v = g_chans512[k] - ( (ld->reverse) ? -ld->offset : ld->offset) ;

    char swVal = '-';  // '-', '<', '>'
    if(v >  50) swVal = (ld->reverse ? 127 : 126);	// Switch to raw inputs?  - remove trim!
    if(v < -50) swVal = (ld->reverse ? 126 : 127);
    putsChn(0,y,k+1,0);
    lcd_putc(12*FW+FW/2, y, swVal ); //'<' : '>'
    
    int8_t limit = (g_model.extendedLimits ? 125 : 100);
		for(uint8_t j=0; j<4;j++)
		{
        uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
#ifndef NOPOTSCROLL
			uint8_t active = attr ;	// (attr && s_editing) ;
			if ( s_editing == 0 )
			{
				active = 0 ;
			}
#else
				uint8_t active = (attr && s_editMode) ;
#endif
				if ( active )
				{
					StickScrollAllowed = 0 ;		// Block while editing
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
            if(active) {
                ld->offset = checkIncDec16(ld->offset, -1000, 1000, EE_MODEL);
            }
            break;
        case 1:
				{	
					int8_t temp = ld->min - 100 ;
					
					value += temp ;
					if ( value < -125 )
					{
						value = -125 ;						
					}
          lcd_outdezAtt(  12*FW, y, value,   attr);
            if(active)
						{
              CHECK_INCDEC_H_MODELVAR( temp, -limit,25);
              ld->min = temp + 100;
            }
				}
        break;
        case 2:
				{	
					value += (int8_t)(ld->max+100) ;
					if ( value > 125 )
					{
						value = 125 ;						
					}
          lcd_outdezAtt( 17*FW, y, value,    attr);
					if ( t )
					{
//						plotType = PLOT_BLACK ;
						lcd_rect( 9*FW-4, y-1, 56, 9 ) ;
//						plotType = PLOT_XOR ;
					}
            if(active)
						{
							int8_t temp = ld->max + 100 ;
                CHECK_INCDEC_H_MODELVAR( temp, -25,limit);
                ld->max = temp - 100 ;
            }
				}
        break;
				case 3:
        default:
					ld->reverse = hyphinvMenuItem( ld->reverse, y, attr ) ;
//						menu_lcd_HYPHINV( 18*FW, y, ld->reverse, attr ) ;
//            if(active) {
//                CHECK_INCDEC_H_MODELVAR_0(ld->reverse, 1);
//            }
            break;
        }
    }
	}
	if(k==NUM_CHNOUT)
	{
    //last line available - add the "copy trim menu" line
    uint8_t attr = (sub==NUM_CHNOUT) ? INVERS : 0;
    lcd_putsAtt(  3*FW,y,PSTR(STR_COPY_TRIM),s_noHi ? 0 : attr);
	}
	asm("") ;
}


//#if defined(GVARS) && defined(PCBSKY9X)
//void menuModelRegisterOne(uint8_t event)
//{
//  model_gvar_t *reg = &g_model.gvars[s_curveChan];

//  putsStrIdx(11*FW, 0, STR_GV, s_curveChan+1);

//  // TODO Translation
//  SUBMENU(PSTR(STR_GLOBAL_VAR), 2, {ZCHAR|(sizeof(reg->name)-1), 0});

//  int8_t sub = m_posVert;

//  for (uint8_t i=0, k=0, y=2*FH; i<2; i++, k++, y+=FH) {
//    uint8_t attr = (sub==k ? InverseBlink : INVERS) : 0);
//    switch(i) {
//      case 0:
//        editName(MIXES_2ND_COLUMN, y, reg->name, sizeof(reg->name), event, attr, g_posHorz);
//        break;
//      case 1:
//				lcd_xlabel_decimal( y, PSTR(STR_VALUE), reg->value, attr|LEFT, MIXES_2ND_COLUMN ) ;
////        lcd_putsLeft(y, PSTR(STR_VALUE));
////        lcd_outdezAtt(MIXES_2ND_COLUMN, y, reg->value, attr|LEFT);
//        if (attr) CHECK_INCDEC_MODELVAR(event, reg->value, -125, 125);
//        break;
//    }
//  }
//}
//#endif




static uint8_t onoffItem( uint8_t value, uint8_t y, uint8_t vertical )
{
//#if defined(CPUM128) || defined(CPUM2561)
#if 1
	return checkIndexedV( y, 0, value, vertical ) ;
#else	
	return checkIndexed( y, PSTR(FWx17"\001""\003"STR_OFF STR_ON), value, condition ) ;
#endif
}

static uint8_t offonItem( uint8_t value, uint8_t y, uint8_t vertical )
{
	return 1-onoffItem( 1-value, y, vertical ) ;
}

static uint8_t onoffMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t vertical )
{
    lcd_puts_Pleft(y, s);
		return onoffItem(value, y, vertical ) ;
}

NOINLINE static uint8_t offonMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t vertical )
{
	return 1-onoffMenuItem( 1-value, y, s, vertical ) ;
}

void displayNext()
{
	lcd_puts_P( 17*FW+2, 7*FH, PSTR("[->]") ) ;
}

#ifdef FRSKY

#ifndef V2
extern uint8_t frskyRSSIlevel[2] ;
extern uint8_t frskyRSSItype[2] ;
#endif

// FrSky WSHhi DSMx  Jeti  Mavlk ArduP FrHub"

#if defined(CPUM128) || defined(CPUM2561)

#define MAX_TEL_OPTIONS		6
const prog_uint8_t APM TelOptions[] = {0,1,2,5,6,7} ;

//#if defined(PCBSKY) || defined(PCB9XT)
// #ifdef REVX
//  #define MAX_TEL_OPTIONS		6
//  const uint8_t TelOptions[] = {1,2,3,4,5,6} ;
// #else
//  #define MAX_TEL_OPTIONS			5
//  const uint8_t TelOptions[] = {1,2,3,6,7} ;
// #endif
//#endif


#endif


void menuProcTelemetry(uint8_t event)
{
	TITLEP(Str_Telemetry);
//	static MState2 mstate2;
	uint8_t sub ;
#ifdef V2
	sub = check_columns(event, 22-1-1+6+1-1) ;
#else
#ifdef MAH_LIMIT			 
	sub = check_columns(event, 22+1-1-1) ;
#else
//	mstate2.check_columns(event, 23-1-1-1) ;// Extra data for Mavlink via FrSky
	sub = check_columns(event, 22+1-1-1-1) ;
#endif
#endif
	uint8_t subSub = g_posHorz ;
	uint8_t blink ;
	uint8_t y = FH ;

#ifndef V2
	switch(event)
	{
    case EVT_KEY_BREAK(KEY_DOWN):
    case EVT_KEY_BREAK(KEY_UP):
    case EVT_KEY_BREAK(KEY_LEFT):
    case EVT_KEY_BREAK(KEY_RIGHT):
      if(s_editMode)
			{
        FrskyAlarmSendState |= 0x30 ;	 // update Fr-Sky module when edit mode exited
			}
    break ;
		case EVT_ENTRY :
  		FrskyAlarmSendState |= 0x40 ;		// Get RSSI/TSSI alarms
		break ;
	}
#endif // V2

	blink = InverseBlink ;
	uint8_t subN = 0 ;

#ifdef V2
	if ( sub < 14+6 )
#else
#ifdef MAH_LIMIT			 
	if ( sub < 15 )
#else
	if ( sub < 14 )
#endif
#endif
	{
		displayNext() ;
	}

	if ( sub < 5 )
	{
		lcd_puts_Pleft( y, PSTR(STR_USR_PROTO"\037""Units"));
#if defined(CPUM128) || defined(CPUM2561)
		uint8_t b ;
		uint8_t attr = 0 ;

		b = g_model.telemetryProtocol ;
		if ( g_model.FrSkyUsrProto )
		{
			b = 1 ;		// WSHHI
		}

		for ( uint8_t c = 0 ; c < MAX_TEL_OPTIONS ; c += 1 )
		{
			if ( pgm_read_byte(&TelOptions[c]) == b )
			{
				b = c ;
				attr = 1 ;
				break ;
			}
		}
		if ( !attr )
		{
			b = 0 ;
		}
		attr = 0 ;
		if(sub==subN)
		{
			attr = blink ;
			CHECK_INCDEC_H_MODELVAR_0( b, MAX_TEL_OPTIONS-1 ) ;
		}
		b = pgm_read_byte(&TelOptions[b]) ;
		g_model.telemetryProtocol = b ;
		g_model.FrSkyUsrProto = (b == 1) ? 1 : 0 ;

		lcd_putsAttIdx( 10*FW, FH, PSTR("\005FrSkyWSHhiDSMx Jeti MavlkArduPFrHubHbRawFrMav"), b, attr ) ;
//		g_model.FrSkyUsrProto = checkIndexed( y, PSTR(FWx12"\001"STR_FRHUB_WSHHI), g_model.FrSkyUsrProto, (sub==subN) ) ;
#else
		g_model.FrSkyUsrProto = checkIndexedV( y, PSTR(FWx12"\001"STR_FRHUB_WSHHI), g_model.FrSkyUsrProto, subN ) ;
#endif
		y += FH ;
		subN += 1 ;

		g_model.FrSkyImperial = checkIndexedV( y, PSTR(FWx12"\001"STR_MET_IMP), g_model.FrSkyImperial, subN ) ;
		y += FH ;
		subN += 1 ;

		for (int i=0; i<2; i++)
		{
#ifdef V2
			V2FrSkyChannelData *fd ;
#else
			FrSkyChannelData *fd ;
#endif
			if(sub==subN)
			{
				Columns = 1 ;
			}
	 
			fd = &g_model.frsky.channels[i] ;

  	  lcd_puts_Pleft(y, PSTR(STR_A_CHANNEL));
  	  lcd_putc(FW, y, '1'+i);
  	  putsTelemValue( 21*FW, y, frskyTelemetry[i].value, i,  NO_UNIT ) ;
  	  
#ifdef V2
  	  putsTelemValue(12*FW, y, 256, i, (sub==subN && subSub==0 ? blink:0)|NO_UNIT ) ;
#else
  	  putsTelemValue(12*FW, y, 255, i, (sub==subN && subSub==0 ? blink:0)|NO_UNIT ) ;
#endif

	#ifndef NOPOTSCROLL
  	  if (sub==subN && (s_editing ) )	// Use s_editing???
	#else    
			if (sub==subN && s_editMode )
	#endif
#ifdef V2
			{
	      if (subSub == 0 )
				{
          fd->ratio = checkIncDec16(fd->ratio, 0, 1000, EE_MODEL ) ;
	      }
  	  }
			uint8_t attr ;
			attr = (sub==subN && subSub==1 ) ? InverseBlink : 0 ;
 			if( attr ) CHECK_INCDEC_H_MODELVAR_0( fd->unit, 7 ) ;
			lcd_putsAttIdx( 13*FW, y, UnitsString, fd->unit, attr ) ;
//			fd->unit = checkIndexed( y, PSTR(FWx16"\003""\001v-VA"), fd->unit, (sub==subN && subSub==1 ) ? 2 : 0 ) ;
#else
			{
	      if (subSub == 0 )
				{
          fd->opt.alarm.ratio = checkIncDec_u0(fd->opt.alarm.ratio, 255 ) ;
	      }
  	  }
			fd->opt.alarm.type = checkIndexed( y, StrNZ_AxTypes, fd->opt.alarm.type, (sub==subN && subSub==1 ) ? 2 : 0 ) ;
#endif
  	  subN++; y+=FH;
		}
// 		lcd_puts_Pleft( y, PSTR( "DSM 'A' as RSSI") ) ;
// 		menu_lcd_onoff( PARAM_OFS, y, g_model.dsmAasRssi, sub==subN ) ;
//  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.dsmAasRssi, 1);
		
		g_model.dsmAasRssi = onoffMenuItem( g_model.dsmAasRssi, y, PSTR("DSM 'A' as RSSI"), subN) ;

		
		subN++; y+=FH;
	}
	else if ( sub < 9 )
	{
		uint8_t subN = 5 ;
#ifndef V2
#ifdef MAH_LIMIT			 
		int16_t value ;
#endif
#endif

#ifdef V2
 	  lcd_puts_Pleft( y, PSTR("RSSI Low\037RSSI Critical") );
		uint8_t attr = 0 ;
		if ( sub==subN )
		{
			attr = blink ;
			CHECK_INCDEC_H_MODELVAR( g_model.frsky.rxRssiLow, -45, 25 ) ;
		}
		lcd_outdezAtt( 17*FW, y, g_model.frsky.rxRssiLow+45, attr ) ;
 	  subN++; y+=FH;
		attr = 0 ;
		if ( sub==subN )
		{
			attr = blink ;
			CHECK_INCDEC_H_MODELVAR( g_model.frsky.rxRssiCritical, -42, 28 ) ;
		}
		lcd_outdezAtt( 17*FW, y, g_model.frsky.rxRssiCritical+42, attr ) ;
 	  subN++; y+=FH;
#else
		for (uint8_t j=0; j<2; j++)
		{
			Columns = 1 ;
  	  lcd_puts_Pleft( y, PSTR(STR_TX_RSSIALRM) );
  	  if ( j == 1 )
  	  {
  	  	lcd_puts_Pleft( y, PSTR(STR_RX) );
  	  }
  	  
			frskyRSSItype[j] = checkIndexed( y, PSTR(FWx11"\003"STR_YELORGRED), frskyRSSItype[j], (sub==subN && subSub==0 ) ? 2 : 0 ) ;
			
//			lcd_putsAttIdx(11*FW, y, Str_YelOrgRed,frskyRSSItype[j],(sub==subN && subSub==0 ? blink:0));
  	  
			lcd_outdezNAtt(17*FW, y, frskyRSSIlevel[j], (sub==subN && subSub==1 ? blink:0), 3);

#ifndef NOPOTSCROLL
	    if (sub==subN && (s_editing ) )	// Use s_editing???
#else    
			if (sub==subN && s_editMode )
#endif
			{
//  	    	if (subSub == 0)
//					{
//  	    	  frskyRSSItype[j] = checkIncDec_0( frskyRSSItype[j], 3 ) ;
//					}
//					else if (subSub == 1)
					if (subSub == 1)
					{
  	    	  frskyRSSIlevel[j] = checkIncDec_0( frskyRSSIlevel[j], 120 );
					}
  	  }
  	  subN++; y+=FH;
		}
#endif // V2

#ifndef V2
  	uint8_t attr ;
#ifdef MAH_LIMIT			 
		value = g_model.frsky.frskyAlarmLimit ;
  	attr = ((sub==subN) ? blink : 0);
		if ( attr )
		{
  		value = checkIncDec_u0( value, 200 ) ;
			g_model.frsky.frskyAlarmLimit = value ;
		}
		lcd_xlabel_decimal( PARAM_OFS, 3*FH, value<<6, attr, PSTR(STR_MAH_ALARM) ) ;
		y += FH ;
  	subN++;
#endif // MAH_LIMIT
#endif // nV2				

 		attr = 0 ;
  	if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR(g_model.numBlades, 1, 127);}
		
		lcd_xlabel_decimal( PARAM_OFS, y, g_model.numBlades, attr, PSTR(STR_NUM_BLADES) ) ;
		y += FH ;
  	subN++;
  
#ifndef NOGPSALT    
		g_model.FrSkyGpsAlt = onoffMenuItem( g_model.FrSkyGpsAlt, y, PSTR(STR_GPS_ALTMAIN), subN) ;
#endif

	}
#ifdef V2
	else if ( sub < 14+6+1 )
#else
#ifdef MAH_LIMIT			 
	else if ( sub < 15+1 )
#else
	else if ( sub < 14+1 )
#endif
#endif
	{
		uint8_t subN ;
		uint8_t attr = 0 ;
		uint8_t *pindex ;
#ifdef V2
#ifdef MAH_LIMIT			 
		if (sub < 15+1)
#else
		if ( sub < 14+1 )
#endif
		{
			pindex = g_model.CustomDisplayIndex[0] ;
#ifdef MAH_LIMIT			 
			subN = 9+1 ;
#else
			subN = 8+1 ;
#endif
		}
		else
		{
			pindex = g_model.CustomDisplayIndex[1] ;
#ifdef MAH_LIMIT			 
			subN = 15+1 ;
#else
			subN = 14+1 ;
#endif
		}
#else
			pindex = g_model.CustomDisplayIndex ;
			subN = 8+1 ;
#endif
  	lcd_puts_Pleft( FH, PSTR(STR_CUSTOM_DISP) );
		for (uint8_t j=0; j<6; j++)
		{
			attr = 0 ;
	  	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( *pindex, NUM_TELEM_ITEMS ) ;}
			putsAttIdxTelemItems( 0, j*FH + 2*FH, *pindex, attr ) ;
			pindex += 1 ;
			subN++;
		}
	}
	else
	{
#ifdef V2
		uint8_t subN = 14+6+1 ;
#else
#ifdef MAH_LIMIT			 
		uint8_t subN = 15+1 ;
#else
		uint8_t subN = 14+1 ;
#endif
#endif
    uint8_t attr = PREC1 ;
		lcd_puts_Pleft( FH, PSTR(STR_FAS_OFFSET"\037"STR_VARIO_SRC"\037"STR_2SWITCH"\037"STR_2SENSITIVITY"\037""\037""Current Source") ) ;
		if ( (sub == subN) )
		{
			attr = blink | PREC1 ;
      CHECK_INCDEC_H_MODELVAR_0( g_model.frsky.FASoffset, 15 ) ;
		}
		lcd_outdezAtt( 17*FW, FH, g_model.frsky.FASoffset, attr ) ;
		subN += 1 ;
	
		// Vario
   	
		for( uint8_t j=0 ; j<5 ; j += 1 )
		{
      attr = (sub==subN) ? blink : 0 ;
			uint8_t y = (2+j)*FH ;

   		if (j == 0)
			{
				uint8_t vsource = g_model.varioData.varioSource ;
				g_model.varioData.varioSource = checkIndexedV( y, PSTR(FWx15"\002"STR_VSPD_A2), vsource, subN ) ;
			}
   		else if (j == 1)
   		{
				
				g_model.varioData.swtch = edit_dr_switch( 15*FW, y, g_model.varioData.swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0 ) ;

			}
   		else if (j == 2)
			{
   		  if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.param, 50 ) ;
   		  }
 				lcd_outdezAtt( 17*FW, y, g_model.varioData.param, attr) ;
			}	
   		else if (j == 3)
			{
        uint8_t b = g_model.varioData.sinkTones ;
				g_model.varioData.sinkTones = offonMenuItem( b, y, PSTR(STR_SINK_TONES), subN ) ;
			}
			else// j == 4
			{
#if (NUM_SCALERS != 4)
	ERROR - need to correct max on line below
#endif
				g_model.currentSource = checkIndexedV( y, PSTR(FWx16"\007\003---A1 A2 FasSC1SC2SC3SC4"), g_model.currentSource, subN ) ;
			}
			subN += 1 ;
		}
	}

}



#endif

void put_curve( uint8_t x, uint8_t y, int8_t idx, uint8_t attr )
{
	if ( idx < 0 )
	{
    lcd_putcAtt( x-FW, y, '!', attr ) ;
		idx = -idx + 6 ;
	}
	lcd_putsAttIdx( x, y,Curve_Str,idx,attr);
}




uint8_t scalerDecimal( uint8_t y, uint8_t val, uint8_t attr )
{
  lcd_outdezAtt( 17*FW, y, val+1, attr ) ;
  if (attr) val = checkIncDec_u0( val, 255 ) ;
	return val ;
}

void menuScaleOne(uint8_t event)
{
	
//	static MState2 mstate2 ;
	int8_t sub ;
#if defined(EXTEND_SCALERS)
	sub = check_columns(event, 10 ) ;
#else
	sub = check_columns(event, 8 ) ;
#endif
	lcd_puts_Pleft( 0, Str_SC ) ;
	uint8_t index = s_currIdx ;
  lcd_putc( 2*FW, 0, index+'1' ) ;
  
	uint8_t t_pgOfs = evalOffset(sub);
	
	putsTelemetryChannel( 8*FW, 0, index+TEL_ITEM_SC1, 0, 0, TELEM_UNIT ) ;

	lcd_puts_Pskip( FH, PSTR("Source\037\037"STR_OFFSET"\037Multiplier\037Divisor\037Unit\037Sign\037Decimals\037Offset At"), s_pgOfs ) ;

	for (uint8_t k = 0 ; k < 7 ; k += 1 )
	{
    uint8_t y = (k+1) * FH ;
    uint8_t i = k + t_pgOfs;
		uint8_t attr = (sub==i ? InverseBlink : 0);
		ScaleData *pscaler ;
		pscaler = &g_model.Scalers[index] ;
#if defined(EXTEND_SCALERS)
		ExtScaleData *epscaler ;
		epscaler = &g_model.eScalers[index] ;
#endif

		switch(i)
		{
      case 0 :	// Source
				putsChnRaw( 11*FW, y, pscaler->source, attr ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->source, NUM_XCHNRAW+NUM_TELEM_ITEMS ) ;
			break ;
			case 1 :	// name
#if defined(CPUM128) || defined(CPUM2561)
				alphaEditName( 11*FW-2, y, (uint8_t *)pscaler->name, sizeof(pscaler->name), attr, (char *)PSTR( "Scaler Name") ) ;
#else
				if ( attr )
				{
					Columns = 3 ;
				}
				editName( g_posHorz, y, pscaler->name, 4, attr ) ;
#endif
			break ;
      case 2 :	// offset
				lcd_outdezAtt( 13*FW, y, pscaler->offset, attr) ;
				if ( attr )
				{
					pscaler->offset = checkIncDec16( pscaler->offset, -32000, 32000, EE_MODEL ) ;
				}
			break ;
      case 3 :	// mult
				pscaler->mult = scalerDecimal( y, pscaler->mult, attr ) ;
			break ;
      case 4 :	// div
				pscaler->div = scalerDecimal( y, pscaler->div, attr ) ;
			break ;
      case 5 :	// unit
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->unit, 7 ) ;
				lcd_putsAttIdx( 12*FW, y, UnitsString, pscaler->unit, attr ) ;
//				pscaler->unit = checkIndexed( y, PSTR(FWx12"\001\007"UNITS_STR), pscaler->unit, (sub==i) ) ;
			break ;
      case 6 :	// sign
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->neg, 1 ) ;
  			lcd_putcAtt( 12*FW, y, pscaler->neg ? '-' : '+', attr ) ;
			break ;
      case 7 :	// precision
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->precision, 2 ) ;
				lcd_outdezAtt( 14*FW, y, pscaler->precision, attr) ;
			break ;
      case 8 :	// offsetLast
			{	
				pscaler->offsetLast = checkIndexedV( y, Str_FirstLast, pscaler->offsetLast, i ) ;
			}
			break ;
#if defined(EXTEND_SCALERS)
      case 9 :	// mod
				lcd_puts_Pleft( y, PSTR("Mod Value") ) ;
				epscaler->mod = scalerDecimal( y, epscaler->mod, attr ) ;
			break ;
      case 10 :	// Dest
				lcd_puts_Pleft( y, PSTR("Dest") ) ;
#define NUM_SCALE_DESTS		5
				if( attr )
				{
					CHECK_INCDEC_H_MODELVAR( epscaler->dest, 0, NUM_SCALE_DESTS ) ;
				}
				lcd_putsAttIdx( 11*FW, y, PSTR("\005-----BaseMAmps mAh  VoltsFuel "), epscaler->dest, attr ) ;
			break ;
#endif
		}
	}

}

#ifdef V2
#if defined(CPUM128) || defined(CPUM2561)
void menuProcAdjust(uint8_t event)
{
	TITLE("GVAR Adjust");
	EditType = EE_MODEL ;
//	static MState2 mstate2;
	uint8_t sub ;
 	sub = check_columns(event, NUM_GVAR_ADJUST-1 ) ;
	
	uint8_t subSub = g_posHorz;
	uint8_t y = FH ;
	
	Columns = 3 ;
	
	for (uint8_t i=0; i<NUM_GVAR_ADJUST; i++ )
	{
		GvarAdjust *pgvaradj ;
		pgvaradj = &g_model.gvarAdjuster[i] ;
  	lcd_puts_Pleft( y, PSTR("A  GV") ) ;
  	lcd_putc( 1*FW, y, i+'1' ) ;

#if (defined(CPUM128) || defined(CPUM2561))
		if ( sub==i )
		{
  		lcd_puts_Pleft( 0, PSTR("\015GV =") ) ;
  		lcd_putc( 15*FW, 0, pgvaradj->gvarIndex+'1' ) ;
			lcd_outdez( 20*FW, 0, g_model.gvars[pgvaradj->gvarIndex].gvar) ;
		} 
#endif
		for( uint8_t j = 0 ; j < 4 ; j += 1 )
		{
      uint8_t attr = ((sub==i && subSub==j) ? InverseBlink : 0);
			uint8_t active = (attr && (s_editMode) ) ;
			if ( j == 0 )
			{
				lcd_putcAtt( 5*FW, y, pgvaradj->gvarIndex+'1', attr ) ;
				if ( active )
				{
 	        CHECK_INCDEC_H_MODELVAR_0( pgvaradj->gvarIndex, 6 ) ;
				}
			}
			else if ( j == 1 )
			{
				uint8_t old = pgvaradj->function ;
				pgvaradj->function = checkIndexed( y, PSTR(FWx7"\010""\005-----Add  +/Lim-/LimSet CSet V+/-  Inc/0Dec/0"), pgvaradj->function, attr ) ;
				if ( pgvaradj->function != old )
				{
					if ( old < 5 )
					{
						if ( pgvaradj->function >= 5 )
						{
							pgvaradj->switch_value = 0 ;
						}
					}
					else
					{
						if ( pgvaradj->function < 5 )
						{
							pgvaradj->switch_value = 0 ;
						}
					}
				}
			}
			else if ( j == 2 )
			{
				pgvaradj->swtch = edit_dr_switch( 13*FW-1, y, pgvaradj->swtch, attr, active ? EDIT_DR_SWITCH_EDIT : 0 ) ;
			}
			else
			{
				if ( pgvaradj->function < 6 )
				{
					if ( pgvaradj->function == 5 )
					{
						lcd_putsAttIdx( 18*FW, y, Str_Gv_Source, pgvaradj->switch_value, attr ) ;
		  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgvaradj->switch_value, 12 ) ;
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
					pgvaradj->switch_value = edit_dr_switch( 17*FW, y, pgvaradj->switch_value, attr, active ? EDIT_DR_SWITCH_EDIT : 0 ) ;
				}
			}
		}
		y += FH ;
	}
}
#endif // 128/2561
#endif // V2



void menuProcGlobals(uint8_t event)
{
	TITLEP(Str_Globals) ;
//	static MState2 mstate2 ;

	uint8_t subN ;
	subN = check_columns(event, MAX_GVARS + 1 + NUM_SCALERS-1-1 ) ;

	uint8_t subSub = g_posHorz;
	uint8_t y = FH ;

  switch (event)
	{
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE) :
			if ( subN >= 7 ) //&& sub <= MAX_MODES )
			{
        s_currIdx = subN - 7 ;
//				RotaryState = ROTARY_MENU_UD ;
        killEvents(event);
        pushMenu(menuScaleOne) ;
    	}
		break;
  }
 if ( subN < 7 )
 {
#if defined(CPUM128) || defined(CPUM2561)
	Columns = 2 ;
#else
	Columns = 1 ;
#endif 	 
	for (uint8_t i=0; i<MAX_GVARS; i++ )
	{
    lcd_puts_Pleft(y, Str_GV ) ;
		lcd_putc( 2*FW, y, i+'1') ;
#ifdef V2
		V2GvarData *pgv = &g_model.gvars[i] ;
#else
		GvarData *pgv = &g_model.gvars[i] ;
#endif
		FORCE_INDIRECT(pgv) ;
#ifdef V2
		for(uint8_t j=0 ; j<3 ; j++)
		{
      uint8_t attr = ((subN==i && subSub==j) ? InverseBlink : 0);
			uint8_t active = (attr && (s_editMode) ) ;
      if ( j == 0 )
			{
       	putsDrSwitches( 5*FW, y, pgv->gvswitch ,attr );
  			if(active) CHECK_INCDEC_MODELSWITCH( pgv->gvswitch, -MaxSwitchIndex, MaxSwitchIndex ) ;
			}
      else if ( j == 1 )
			{
  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgv->gvsource, 37+9 ) ;
				if ( pgv->gvsource < 13 )
				{
					lcd_putsAttIdx( 10*FW, y, Str_Gv_Source, pgv->gvsource, attr ) ;
				}
				else if ( pgv->gvsource < 29 )
				{
					lcd_putsAttIdx( 10*FW, y, Str_Chans_Gv, pgv->gvsource+3, attr ) ;
				}
//#if !(defined(CPUM128) || defined(CPUM2561))
//				else
//				{
//					uint8_t val = pgv->gvsource-29 ;
//					lcd_putsAttIdx( 11*FW, y, PSTR("\004L1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLI"), val/2, attr ) ;
//					lcd_putcAtt( 10*FW, y, val & 1 ? '-' : '+', attr ) ;
//				}
//#endif
			}
			else
			{
  			if(active) CHECK_INCDEC_H_MODELVAR( pgv->gvar, -125, 125 ) ;
				lcd_outdezAtt( 18*FW, y, pgv->gvar, attr) ;
			}
		}
#else // V2
#if defined(CPUM128) || defined(CPUM2561)
		for(uint8_t j=0 ; j<3 ; j++)
		{
      uint8_t attr = ((subN==i && subSub==j) ? InverseBlink : 0);
			uint8_t active = (attr && (s_editMode) ) ;
      if ( j == 0 )
			{
       	putsDrSwitches( 5*FW, y, g_model.gvswitch[i] ,attr );
  			if(active) CHECK_INCDEC_MODELSWITCH( g_model.gvswitch[i], -MaxSwitchIndex, MaxSwitchIndex ) ;
			}
      else if ( j == 1 )
			{
  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgv->gvsource, 37+9-18 ) ;
				if ( pgv->gvsource < 13 )
				{
					lcd_putsAttIdx( 10*FW, y, Str_Gv_Source, pgv->gvsource, attr ) ;
				}
				else if ( pgv->gvsource < 29 )
				{
					lcd_putsAttIdx( 10*FW, y, Str_Chans_Gv, pgv->gvsource+3, attr ) ;
				}
//				else
//				{
//					uint8_t val = pgv->gvsource-29 ;
//					lcd_putsAttIdx( 11*FW, y, PSTR("\004L1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLI"), val/2, attr ) ;
//					lcd_putcAtt( 10*FW, y, val & 1 ? '-' : '+', attr ) ;
//				}
			}
			else
			{
  			if(active) CHECK_INCDEC_H_MODELVAR( pgv->gvar, -125, 125 ) ;
				lcd_outdezAtt( 18*FW, y, pgv->gvar, attr) ;
			}
		}
#else
		for(uint8_t j=0; j<2;j++)
		{
      uint8_t attr = ((subN==i && subSub==j) ? InverseBlink : 0);
			uint8_t active = attr ;	// (attr && s_editing) ;
			if ( s_editMode == 0 )
			{
				active = 0 ;
			}
      if ( j == 0 )
			{
  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgv->gvsource, 34-6 ) ;
				if ( pgv->gvsource < 13 )
				{
					lcd_putsAttIdx( 10*FW, y, Str_Gv_Source, pgv->gvsource, attr ) ;
				}
				else if ( pgv->gvsource < 29 )
				{
					lcd_putsAttIdx( 10*FW, y, Str_Chans_Gv, pgv->gvsource+3, attr ) ;
				}
//				else
//				{
//					lcd_putsAttIdx( 10*FW, y, PSTR("\004L1L2L3L4L5L6L7L8L9LALBLC"), pgv->gvsource-29, attr ) ;
//				}
			}
			else
			{
  			if(active) CHECK_INCDEC_H_MODELVAR( pgv->gvar, -125, 125 ) ;
				lcd_outdezAtt( 18*FW, y, pgv->gvar, attr) ;
			}
		}
#endif
#endif // V2
		y += FH ;
	}
 }
 else
 {
	uint8_t sub = subN - 7 ;
	for (uint8_t i=0; i<NUM_SCALERS; i++ )
	{
		uint8_t y = (i+1)*FH ;
  	lcd_puts_Pleft( y, PSTR("SC\011+\015*\22/") ) ;
  	lcd_putc( 2*FW, y, i+'1' ) ;
		ScaleData *pscaler ;
		pscaler = &g_model.Scalers[i] ;
		FORCE_INDIRECT(pscaler) ;

		putsChnRaw( 4*FW, y, pscaler->source, 0 ) ;
		lcd_outdez( 12*FW+3, y, pscaler->offset ) ;
		lcd_outdez( 16*FW, y, pscaler->mult+1 ) ;
		lcd_outdez( 21*FW, y, pscaler->div+1 ) ;
	}
	lcd_char_inverse( 0, (sub+1)*FH, 126, 0 ) ;
 }
	asm("") ;
}


#ifndef NO_TEMPLATES
void menuProcTemplates(uint8_t event)  //Issue 73
{
		TITLE(STR_TEMPLATES) ;
//		static MState2 mstate2 ;
    int8_t  sub ;
		sub = check_columns(event, NUM_TEMPLATES-1 ) ;

    uint8_t t_pgOfs ;
    uint8_t y = 0;
    uint8_t k = 0;

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
				lcd_2_digits( 3*FW, y, k+1, (sub==k ? INVERS : 0) ) ;

#ifndef SIMU
        lcd_putsAtt(  4*FW, y, (const prog_char*)pgm_read_word(&n_Templates[k]), (s_noHi ? 0 : (sub==k ? INVERS  : 0)));
#else
				lcd_putsAtt(  4*FW, y, n_Templates[k], (s_noHi ? 0 : (sub==k ? INVERS  : 0)));
#endif
        y+=FH;
    }
}
#endif

//FunctionData Function[1] ;

static void menuProcSafetySwitches(uint8_t sub)
{
	
	TITLEP(PSTR(STR_SAFETY_SW)) ;

#ifdef V2
	IlinesCount = NUM_SAFETY ;
	uint8_t y = 0 ;
	uint8_t k = 0 ;
	uint8_t subSub = g_posHorz ;
	uint8_t t_pgOfs ;

	t_pgOfs = evalOffset(sub) ;

	for(uint8_t i=0; i<7; i++)
	{
	  y=(i+1)*FH;
  	k=i+t_pgOfs;
		Columns = 2 ;
 	 	for(uint8_t j=0; j<=2;j++)
		{
			V2SafetySwData *sd = &g_model.safetySw[k];
	    uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
				
			int8_t active = attr ? 2 : 0 ;	// (attr && s_editing) ;
	    if (j == 0)
			{
    		putsChn(0,y,k+1,0);
				uint8_t value = sd->mode ;
				value = checkIndexed( y, Str_SafetySticky, value, active ) ;
				sd->mode = value ;
			}
      else if (j == 1)
			{
				int8_t max = MaxSwitchIndex ;
        putsDrSwitches(11*FW+3, y, sd->swtch, attr);
        if(active)
				{
          CHECK_INCDEC_MODELSWITCH( sd->swtch, -MaxSwitchIndex, max ) ;
        }
			}
			else
			{
				int8_t min, max ;
				{
					min = -125 ;
					max = 125 ;
       		lcd_outdezAtt(  20*FW, y, sd->val, attr);
				}
         if(active) {
	        CHECK_INCDEC_H_MODELVAR( sd->val, min,max);
				}
      }
		}
	}
#else

#ifdef NOVOICE_SW
	IlinesCount = NUM_CHNOUT+1-1 ;
#else
	IlinesCount = NUM_CHNOUT+1+1+EXTRA_VOICE_SW-1 ;
#endif
	uint8_t y = 0 ;
	uint8_t k = 0 ;
	uint8_t subSub = g_posHorz ;
	uint8_t t_pgOfs ;

	t_pgOfs = evalOffset(sub) ;

 for(uint8_t i=0; i<7; i++)
 {
  y=(i+1)*FH;
  k=i+t_pgOfs;
#ifndef NOVOICE_SW
	if ( k == 0 )
	{
		uint8_t attr = 0 ;
    if(sub==k)
		{
			Columns = 0 ;
			attr = InverseBlink ;
      CHECK_INCDEC_H_MODELVAR_0( g_model.numVoice, 16 ) ;
		}	
		lcd_xlabel_decimal( 18*FW, y, g_model.numVoice+8, attr, PSTR(STR_NUM_VOICE_SW) ) ;
	}
  else // if(k<NUM_CHNOUT+1)
#endif
	{
#ifdef NOVOICE_SW
		uint8_t numSafety = 16 ;
#else
		uint8_t numSafety = 16 - g_model.numVoice ;
#endif
    SafetySwData *sd = &g_model.safetySw[k-1];
		if ( sub==k )
		{
			Columns = 2 ;
		}
   	if ( k-1 >= NUM_CHNOUT )
		{
			sd = &g_model.xvoiceSw[k-1-NUM_CHNOUT];
		}
   	for(uint8_t j=0; j<3;j++)
		{
      uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
				
			uint8_t active = attr ? 2 : 0 ;	// (attr && s_editing) ;
			if ( k <= numSafety )
			{
	      if (j == 0)
				{
    			putsChn(0,y,k,0);
#ifdef NOSAFETY_A_OR_V					
 #ifdef SAFETY_ONLY
					sd->opt.ss.mode = checkIndexed( y, Str_SafetySticky, sd->opt.ss.mode, active ) ;
 #else
					uint8_t value = sd->opt.ss.mode == 3 ? 1 : 0 ;
					value = checkIndexed( y, Str_SafetySticky, value, active ) ;
					sd->opt.ss.mode = value ? 3 : 0 ;
 #endif
#else
 #ifdef SAFETY_ONLY
  #if defined(CPUM128) || defined(CPUM2561)
					sd->opt.ss.mode = checkIndexed( y, Str_SafetySticky, sd->opt.ss.mode, active ) ;
  #else
					sd->opt.ss.mode = checkIndexed( y, PSTR(FWx5"\001""\001SX"), sd->opt.ss.mode, active ) ;
  #endif
 #else
					sd->opt.ss.mode = checkIndexed( y, PSTR(FWx5"\003""\001SAVX"), sd->opt.ss.mode, active ) ;
 #endif
#endif
				}
      	else if (j == 1)
        {
					int8_t max = MaxSwitchIndex ;
#ifndef SAFETY_ONLY
 #ifndef NOSAFETY_A_OR_V					
					if ( sd->opt.ss.mode == 1 )
					{
						max += 3 ;
						if ( ( sd->opt.ss.swtch >= MAX_DRSWITCH ) && ( sd->opt.ss.swtch < MAX_DRSWITCH + 3 ) )
						{
							lcd_putsAttIdx( 6*FW+3, y, PSTR(STR_V_OPT1), sd->opt.ss.swtch-MAX_DRSWITCH, attr ) ;
						}
						else
						{
          	 	putsDrSwitches(11*FW+3, y, sd->opt.ss.swtch, attr);
						}
					}
					else
					{
 #endif
#endif
           	putsDrSwitches(11*FW+3, y, sd->opt.ss.swtch, attr);
#ifndef SAFETY_ONLY
 #ifndef NOSAFETY_A_OR_V					
					}
 #endif
#endif
          if(active)
					{
						CHECK_INCDEC_MODELSWITCH( sd->opt.ss.swtch, -MaxSwitchIndex, max ) ;
          }
				}
				else
				{
						int8_t min, max ;
#ifndef SAFETY_ONLY
 #ifndef NOSAFETY_A_OR_V					
						if ( sd->opt.ss.mode == 1 )
						{
							min = 0 ;
							max = 15 ;
							sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
         			lcd_putsAttIdx(15*FW+1, y, Str_Sounds, sd->opt.ss.val,attr);
						}
						else if ( sd->opt.ss.mode == 2 )
						{
							if ( sd->opt.ss.swtch > MAX_DRSWITCH )
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
        				lcd_outdezAtt( 20*FW, y, sd->opt.ss.val+128, attr);
							}
						}
						else
 #endif
#endif
						{
							min = -125 ;
							max = 125 ;
        			lcd_outdezAtt(  20*FW, y, sd->opt.ss.val, attr);
						}
            if(active) {
		          CHECK_INCDEC_H_MODELVAR( sd->opt.ss.val, min,max);
            }
        }
			}
#ifndef NOVOICE_SW
			else
			{
    		if (j == 0)
				{
			    lcd_puts_Pleft( y, PSTR(STR_VS) ) ;
					lcd_2_digits( 22, y, k, 0 ) ;
    		  if(active)
					{
    		    CHECK_INCDEC_MODELSWITCH( sd->opt.vs.vswtch, 0, MaxSwitchIndex-1 ) ;
    		  }
#ifdef SAFETY_ONLY
  		    putsDrSwitches(5*FW-5, y, sd->opt.vs.vswtch, attr);
#else
  		    putsDrSwitches(5*FW, y, sd->opt.vs.vswtch, attr);
#endif
				}
    		else if (j == 1)
    		{
#ifdef SAFETY_ONLY
					sd->opt.vs.vmode = checkIndexed( y, PSTR(FWx9"\003\006ON    OFF   BOTH  Varibl"), sd->opt.vs.vmode, active ) ;
#else
					sd->opt.vs.vmode = checkIndexed( y, PSTR(FWx10"\006"STR_VOICE_OPT), sd->opt.vs.vmode, active ) ;
#endif
				}
				else
				{
					uint8_t max ;
#ifdef SAFETY_ONLY
					if ( sd->opt.vs.vmode > 2 )
#else
					if ( sd->opt.vs.vmode > 5 )
#endif
					{
						max = NUM_TELEM_ITEMS-1 ;
#ifdef SAFETY_ONLY
						sd->opt.vs.vval = limit( (int8_t)-16, (int8_t)sd->opt.vs.vval, (int8_t)max) ;
						if ( (int8_t)sd->opt.vs.vval < 0 )
						{
         			lcd_putsAttIdx(15*FW+1, y, Str_Sounds, -(int8_t)sd->opt.vs.vval-1, attr) ;
						}
						else
						{
							putsAttIdxTelemItems( 16*FW, y, sd->opt.vs.vval+1, attr ) ;
						}
#else
						putsAttIdxTelemItems( 16*FW, y, sd->opt.vs.vval+1, attr ) ;
#endif
					}
					else
					{
						// Allow 251-255 to represent GVAR3-GVAR7
						max = 255 ;
						if ( sd->opt.vs.vval <= 250 )
						{
	  					lcd_outdezAtt( 17*FW, y, sd->opt.vs.vval, attr) ;
						}
						else
						{
							dispGvar( 14*FW, y, sd->opt.vs.vval-248, attr ) ;
						}
					}
    		  if(active)
					{
#ifdef SAFETY_ONLY
						if ( sd->opt.vs.vmode > 2 )
						{
    	      	sd->opt.vs.vval = checkIncDec_i8( sd->opt.vs.vval, -16, max ) ;
						}
						else
						{
    	      	sd->opt.vs.vval = checkIncDec_u0( sd->opt.vs.vval, max ) ;
						}
#else
    	      sd->opt.vs.vval = checkIncDec_u0( sd->opt.vs.vval, max ) ;
#endif
    		  }
				}	 
			}
#endif
		}
	}
 }
#endif // V2
}

static void menuProcSwitches(uint8_t sub)  //Issue 78
{
	TITLEP(PSTR(STR_CUST_SWITCH)) ;
#ifdef V2
	IlinesCount = NUM_CSW+EXTRA_CSW+1-1 ;
#else
 #if defined(CPUM128) || defined(CPUM2561)
	IlinesCount = NUM_CSW+EXTRA_CSW+1-1 ;
 #else
	IlinesCount = NUM_CSW+1-1 ;
 #endif
#endif // V2
	Columns = 3 ;
	uint8_t y = 0;
	uint8_t k = 0;
	uint8_t subSub = g_posHorz;
  uint8_t t_pgOfs ;

	t_pgOfs = evalOffset(sub);

	for(uint8_t i=0; i<7; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    uint8_t attr = (sub==k ? InverseBlink  : 0);
    
		//write SW names here
    lcd_putc(  0 , y, 'L' );
    lcd_putc(  FW-1 , y, k + (k>8 ? 'A'-9: '1'));

#ifdef V2
		uint8_t att1 = subSub==1 ? attr : 0 ;
		uint8_t att2 = subSub==2 ? attr : 0 ;
		
		CxSwData *cs = &g_model.customSw[k];

		lcd_putsAttIdx( 2*FW+1, y, Str_Cswitch,cs->func,subSub==0 ? attr : 0);
    uint8_t cstate = CS_STATE(cs->func);
    if(cstate == CS_VOFS)
    {
        if (cs->v1 > CHOUT_BASE+NUM_CHNOUT)
				{
					int16_t value = convertTelemConstant( cs->v1-CHOUT_BASE-NUM_CHNOUT-1, cs->v2 ) ;
					putsTelemetryChannel( 18*FW-8, y, cs->v1-CHOUT_BASE-NUM_CHNOUT-1, value, att2, TELEM_UNIT | TELEM_CONSTANT ) ;
				}
        else
				{
          lcd_outdezAtt( 18*FW-9, y, cs->v2  , att2);
				}
        putsChnRaw(    10*FW-6, y, cs->v1, att1 );
    }
    else if(cstate == CS_VBOOL)
    {
        putsDrSwitches(10*FW-6, y, cs->v1  , att1 ) ;
        putsDrSwitches(14*FW-7, y, cs->v2  ,att2) ;
    }
    else if(cstate == CS_VCOMP)
    {
        putsChnRaw(    14*FW-4, y, cs->v2  ,att2 ) ;
        putsChnRaw(    10*FW-6, y, cs->v1  , att1 ) ;
    }
		else if(cstate == CS_TIMER)
		{
			int8_t x ;
			uint8_t att = att1 ;
			x = cs->v1 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att |= PREC1 ;
			}
			lcd_xlabel_decimal( 13*FW-5, y, x+1, att, PSTR(STR_15_ON) ) ;
			att = att2 ;
			x = cs->v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att |= PREC1 ;
			}
      lcd_outdezAtt( 18*FW-1, y, x+1 , att ) ;
		}
		else// cstate == CS_TMONO
		{
    	putsDrSwitches(10*FW-6, y, cs->v1  ,subSub==1 ? attr : 0);
			uint8_t att = 0 ;
			int8_t x ;
			x = cs->v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
    	lcd_outdezAtt( 17*FW-2, y, x+1 , att | (subSub==2 ? attr : 0 ) ) ;
		}
		{
			int8_t as ;
			as = cs->andsw ;
			putsDrSwitches( 17*FW+2, y, as,(subSub==3 ? attr : 0)) ;
		}
#ifndef NOPOTSCROLL
		if((s_editing ) && attr)	// Use s_editing???
#else		
		if( s_editMode && attr)
#endif
		{
    	switch (subSub)
			{
    	  case 0:
    	    CHECK_INCDEC_H_MODELVAR_0( cs->func, CS_XMAXF);
    	    if(cstate != CS_STATE(cs->func))
    	    {
    	      cs->v1  = 0;
    	      cs->v2 = 0;
    	    }
    	  break;
      
				case 1:
    	    switch (cstate)
					{
    	      case (CS_VCOMP):
    	      case (CS_VOFS):
    	        CHECK_INCDEC_H_MODELVAR( cs->v1, 0, NUM_XCHNRAW+NUM_TELEM_ITEMS);
    	      break;
    	      case (CS_VBOOL):
						case CS_TMONO :
    	        CHECK_INCDEC_MODELSWITCH( cs->v1, -MaxSwitchIndex, MaxSwitchIndex );
    	      break;
    	      case (CS_TIMER):
    	        CHECK_INCDEC_H_MODELVAR( cs->v1, -50, 99);
    	      break;

    	      default:
    	      break;
    	    }
    	  break;
      
				case 2:
    	    switch (cstate)
					{
    	      case (CS_VOFS):
							case CS_TMONO :
    	        CHECK_INCDEC_H_MODELVAR( cs->v2, -125,125);
    	      break;
    	      case (CS_VBOOL):
    	        CHECK_INCDEC_MODELSWITCH( cs->v2, -MaxSwitchIndex, MaxSwitchIndex );
    	      break;
    	      case (CS_VCOMP):
    	        CHECK_INCDEC_H_MODELVAR( cs->v2, 0, NUM_XCHNRAW+NUM_TELEM_ITEMS);
    	      break;
    	      case (CS_TIMER):
    	        CHECK_INCDEC_H_MODELVAR( cs->v2, -50, 99);
    	      break;
    	      default:
    	      break;
    	    }
    	  break;
    	  case 3:
          CHECK_INCDEC_MODELSWITCH( cs->andsw, -MaxSwitchIndex+1,MaxSwitchIndex-1);
				break;
    	}
		}
#else
#if defined(CPUM128) || defined(CPUM2561)
   	if ( k >= NUM_CSW )
		{
			uint8_t att1 = subSub==1 ? attr : 0 ;
			uint8_t att2 = subSub==2 ? attr : 0 ;
			CxSwData *cs = &g_model.xcustomSw[k-NUM_CSW];
			lcd_putsAttIdx( 2*FW+1, y, Str_Cswitch,cs->func,subSub==0 ? attr : 0);
	    uint8_t cstate = CS_STATE(cs->func);
    	if(cstate == CS_VOFS)
    	{
    	    putsChnRaw(    10*FW-6, y, cs->v1  , att1);
    	    if (cs->v1 > CHOUT_BASE+NUM_CHNOUT)
					{
						int16_t value = convertTelemConstant( cs->v1-CHOUT_BASE-NUM_CHNOUT-1, cs->v2 ) ;
						putsTelemetryChannel( 18*FW-8, y, cs->v1-CHOUT_BASE-NUM_CHNOUT-1, value, att2, TELEM_UNIT| TELEM_CONSTANT ) ;
					}
    	    else
    	      lcd_outdezAtt( 18*FW-9, y, cs->v2  ,att2);
    	}
    	else if(cstate == CS_VBOOL)
    	{
    	    putsDrSwitches(10*FW-6, y, cs->v1  ,att1);
    	    putsDrSwitches(14*FW-7, y, cs->v2  ,att2);
    	}
    	else if(cstate == CS_VCOMP)
    	{
    	    putsChnRaw(    10*FW-6, y, cs->v1  ,att1);
    	    putsChnRaw(    14*FW-4, y, cs->v2  ,att2);
    	}
			else if(cstate == CS_TIMER)
			{
				int8_t x ;
				uint8_t att = att1 ;
				x = cs->v1 ;
				if ( x < 0 )
				{
					x = -x-1 ;
					att |= PREC1 ;
				}
				lcd_xlabel_decimal( 13*FW-5, y, x+1, att, PSTR(STR_15_ON) ) ;
				att = att2 ;
				x = cs->v2 ;
				if ( x < 0 )
				{
					x = -x-1 ;
					att |= PREC1 ;
				}
    	  lcd_outdezAtt( 18*FW-3, y, x+1 , att ) ;
			}
			else// cstate == CS_TMONO
			{
    	  putsDrSwitches(10*FW-6, y, cs->v1  ,subSub==1 ? attr : 0);
				uint8_t att = 0 ;
				int8_t x ;
				x = cs->v2 ;
				if ( x < 0 )
				{
					x = -x-1 ;
					att = PREC1 ;
				}
    	  lcd_outdezAtt( 17*FW-2, y, x+1 , att | (subSub==2 ? attr : 0 ) ) ;
			}
			{
				int8_t as ;
				as = cs->andsw ;
				if ( as < 0 )
				{
					as = -as ;
		  		lcd_putcAtt( 18*FW-3, y, '!',(subSub==3 ? attr : 0)) ;
				}
				if ( ( as > 8 ) && ( as <= 9+NUM_CSW+EXTRA_CSW+1 ) )
//				if ( as > 8 )
				{
					as += 1 ;				
				}
				if ( as == 9+NUM_CSW+EXTRA_CSW+1)
				{
					as = 9 ;
				}
				putsDrSwitches( 17*FW+3, y, as,(subSub==3 ? attr : 0)) ;
			}
#ifndef NOPOTSCROLL
			if((s_editing ) && attr)	// Use s_editing???
#else		
			if( s_editMode && attr)
#endif
			{
    		switch (subSub)
				{
    		  case 0:
    		    CHECK_INCDEC_H_MODELVAR_0( cs->func, CS_XMAXF);
    		    if(cstate != CS_STATE(cs->func))
    		    {
    		      cs->v1  = 0;
    		      cs->v2 = 0;
    		    }
    		  break;
      
					case 1:
    		    switch (cstate)
						{
    		      case (CS_VCOMP):
    		      case (CS_VOFS):
    		        CHECK_INCDEC_H_MODELVAR_0( cs->v1, NUM_XCHNRAW+NUM_TELEM_ITEMS);
    		      break;
    		      case (CS_VBOOL):
							case CS_TMONO :
    		        CHECK_INCDEC_MODELSWITCH( cs->v1, -MaxSwitchIndex, MaxSwitchIndex ) ;
    		      break;
    		      case (CS_TIMER):
    		        CHECK_INCDEC_H_MODELVAR( cs->v1, -50, 99);
    		      break;

    		      default:
    		      break;
    		    }
    		  break;
      
					case 2:
    		    switch (cstate)
						{
    		      case (CS_VOFS):
							case CS_TMONO :
    		        CHECK_INCDEC_H_MODELVAR( cs->v2, -125,125);
    		      break;
    		      case (CS_VBOOL):
    		        CHECK_INCDEC_MODELSWITCH( cs->v2, -MaxSwitchIndex, MaxSwitchIndex );
    		      break;
    		      case (CS_VCOMP):
    		        CHECK_INCDEC_H_MODELVAR( cs->v2, 0, NUM_XCHNRAW+NUM_TELEM_ITEMS);
    		      break;
    		      case (CS_TIMER):
    		        CHECK_INCDEC_H_MODELVAR( cs->v2, -50, 99);
    		      break;
    		      default:
    		      break;
    		    }
    		  break;
    		  case 3:
					{	
						int8_t x ;
						x = cs->andsw ;
						if ( ( x > 8 ) && ( x <= 9+NUM_CSW+EXTRA_CSW ) )
						{
							x += 1 ;
						}
						if ( ( x < -8 ) && ( x >= -(9+NUM_CSW+EXTRA_CSW) ) )
						{
							x -= 1 ;
						}
						if ( x == 9+NUM_CSW+EXTRA_CSW+1 )
						{
							x = 9 ;			// Tag TRN on the end, keep EEPROM values
						}
						if ( x == -(9+NUM_CSW+EXTRA_CSW+1) )
						{
							x = -9 ;			// Tag TRN on the end, keep EEPROM values
						}
	          CHECK_INCDEC_MODELSWITCH( x, -MaxSwitchIndex+1,MaxSwitchIndex-1);
						if ( x == 9 )
						{
							x = 9+NUM_CSW+EXTRA_CSW+1 ;
						}
						if ( x == -9 )
						{
							x = -(9+NUM_CSW+EXTRA_CSW+1) ;
						}
						if ( ( x > 9 ) && ( x <= 9+NUM_CSW+EXTRA_CSW+1 ) )
						{
							x -= 1 ;
						}
						if ( ( x < -9 ) && ( x >= -(9+NUM_CSW+EXTRA_CSW+1) ) )
						{
							x += 1 ;
						}
						cs->andsw = x ;
					}
					break;
    		}
			}
			continue ;
		}
		
#endif

		uint8_t att1 = subSub==1 ? attr : 0 ;
		uint8_t att2 = subSub==2 ? attr : 0 ;
		CSwData *cs = &g_model.customSw[k];

		lcd_putsAttIdx( 2*FW+1, y, Str_Cswitch,cs->func,subSub==0 ? attr : 0);

    uint8_t cstate = CS_STATE(cs->func);

    if(cstate == CS_VOFS)
    {
        if (cs->v1 > CHOUT_BASE+NUM_CHNOUT)
				{
					int16_t value = convertTelemConstant( cs->v1-CHOUT_BASE-NUM_CHNOUT-1, cs->v2 ) ;
					putsTelemetryChannel( 18*FW-8, y, cs->v1-CHOUT_BASE-NUM_CHNOUT-1, value, att2, TELEM_UNIT | TELEM_CONSTANT ) ;
				}
        else
				{
          lcd_outdezAtt( 18*FW-9, y, cs->v2  , att2);
				}
        putsChnRaw(    10*FW-6, y, cs->v1, att1 );
    }
    else if(cstate == CS_VBOOL)
    {
        putsDrSwitches(10*FW-6, y, cs->v1  , att1 ) ;
        putsDrSwitches(14*FW-7, y, cs->v2  ,att2) ;
    }
    else if(cstate == CS_VCOMP)
    {
        putsChnRaw(    14*FW-4, y, cs->v2  ,att2 ) ;
        putsChnRaw(    10*FW-6, y, cs->v1  , att1 ) ;
    }
		else // cstate == CS_TIMER
		{
			int8_t x ;
			uint8_t att = att1 ;
			x = cs->v1 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att |= PREC1 ;
			}
			lcd_xlabel_decimal( 13*FW-5, y, x+1, att, PSTR(STR_15_ON) ) ;
			att = att2 ;
			x = cs->v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att |= PREC1 ;
			}
      lcd_outdezAtt( 18*FW-1, y, x+1 , att ) ;
		}
		{
			int8_t as ;
			as = cs->andsw ;
			if ( as > 8 )
			{
				as += 1 ;				
			}
			putsDrSwitches( 17*FW+2, y, as,(subSub==3 ? attr : 0)) ;
		}
		
		
#ifndef NOPOTSCROLL
		if((s_editing ) && attr)	// Use s_editing???
#else		
		if( s_editMode && attr)
#endif
		{
    	switch (subSub)
			{
    	  case 0:
    	    CHECK_INCDEC_H_MODELVAR_0( cs->func, CS_MAXF);
    	    if(cstate != CS_STATE(cs->func))
    	    {
    	      cs->v1  = 0;
    	      cs->v2 = 0;
    	    }
    	  break;
      
				case 1:
    	    switch (cstate)
					{
    	      case (CS_VOFS):
    	        CHECK_INCDEC_H_MODELVAR( cs->v1, 0, NUM_XCHNRAW+NUM_TELEM_ITEMS);
    	      break;
    	      case (CS_VBOOL):
    	        CHECK_INCDEC_MODELSWITCH( cs->v1, -MaxSwitchIndex, MaxSwitchIndex );
    	      break;
    	      case (CS_VCOMP):
    	        CHECK_INCDEC_H_MODELVAR( cs->v1, 0, NUM_XCHNRAW+NUM_TELEM_ITEMS);
    	      break;
    	      case (CS_TIMER):
    	        CHECK_INCDEC_H_MODELVAR( cs->v1, -50, 99);
    	      break;

    	      default:
    	      break;
    	    }
    	  break;
      
				case 2:
    	    switch (cstate)
					{
    	      case (CS_VOFS):
    	        CHECK_INCDEC_H_MODELVAR( cs->v2, -125,125);
    	      break;
    	      case (CS_VBOOL):
    	        CHECK_INCDEC_MODELSWITCH( cs->v2, -MaxSwitchIndex, MaxSwitchIndex );
    	      break;
    	      case (CS_VCOMP):
    	        CHECK_INCDEC_H_MODELVAR( cs->v2, 0, NUM_XCHNRAW+NUM_TELEM_ITEMS);
    	      break;
    	      case (CS_TIMER):
    	        CHECK_INCDEC_H_MODELVAR( cs->v2, -50, 99);
    	      break;
    	      default:
    	      break;
    	    }
    	  break;
    	  case 3:
    	    CHECK_INCDEC_H_MODELVAR_0( cs->andsw, 15 ) ;
				break;
    	}
		}
#endif
	}
}

static uint8_t s_currMixIdx;
static uint8_t s_moveMixIdx;
static int8_t s_currDestCh;


static void deleteMix(uint8_t idx)
{
//    MixData *md = &g_model.mixData[0] ;
//		md += idx ;
    MixData *md = mixaddress( idx ) ;
	  
		memmove( md, md+1,(MAX_MIXERS-(idx+1))*sizeof(MixData));
    memset(&g_model.mixData[MAX_MIXERS-1],0,sizeof(MixData));
		STORE_MODELVARS;
//    eeWaitComplete() ;
}

static void insertMix(uint8_t idx, uint8_t copy)
{
    MixData *md = &g_model.mixData[0] ;
		md += idx ;
	
    memmove( md+1, md,(MAX_MIXERS-(idx+1))*sizeof(MixData) );
		if ( copy )
		{
	    memmove( md, md-1, sizeof(MixData) ) ;
		}
		else
		{
	    memset( md,0,sizeof(MixData));
  	  md->destCh      = s_currDestCh; //-s_mixTab[sub];
    	md->srcRaw      = s_currDestCh; //1;   //
    	md->weight      = 100;
#ifndef V2
			md->lateOffset  = 1 ;
#endif
		}
		s_currMixIdx = idx ;
    STORE_MODELVARS;
//    eeWaitComplete() ;
}

int8_t edit_dr_switch( uint8_t x, uint8_t y, int8_t drswitch, uint8_t attr, uint8_t flags )
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
#ifdef XSW_MOD
      int8_t offset = 0;
			if ( drswitch > TOGGLE_INDEX ) {
        drswitch -= TOGGLE_INDEX;
        offset = (MaxSwitchIndex - 1);
      }
			drswitch = switchUnMap( drswitch ) + offset;
  	  CHECK_INCDEC_H_MODELVAR( drswitch ,-(MaxSwitchIndex-1),2*(MaxSwitchIndex-1));
      offset = 0;
			if ( drswitch >= MaxSwitchIndex ) {
				drswitch -= (MaxSwitchIndex - 1);
        offset = TOGGLE_INDEX;
			}
			drswitch = switchMap( drswitch ) + offset;
#else // !XSW_MOD
			int16_t value = drswitch ;
#ifdef SWITCH_MAPPING
			if ( value > HSW_MAX )
			{
				value = switchUnMap( value - HSW_MAX ) + MaxSwitchIndex - 1 ;
			}
			else
			{
				value = switchUnMap( drswitch ) ;
			}
#endif
  	  CHECK_INCDEC_H_MODELVAR( value ,(1-MaxSwitchIndex),(-2+2*MaxSwitchIndex));
#ifdef SWITCH_MAPPING
			if ( value >= MaxSwitchIndex )
			{
				drswitch = switchMap( value - MaxSwitchIndex + 1 ) + HSW_MAX ;
			}
			else
			{
				drswitch = switchMap( value ) ;
			}
#else
			drswitch = value ;
#endif
#endif  // XSW_MOD
		}
		else
		{
			CHECK_INCDEC_MODELSWITCH( drswitch, -MaxSwitchIndex, MaxSwitchIndex ) ;
		}
	}
	asm("") ;
	return drswitch ;
}

//#ifdef V2
//uint8_t HiResSlow ;
//#endif

static uint8_t editSlowDelay( uint8_t y, uint8_t attr, uint8_t value)
{
  if(attr)  CHECK_INCDEC_H_MODELVAR_0( value, 15); //!! bitfield
	uint8_t lval = value ;
#ifdef V2
	if ( Xmem.HiResSlow )
#else
	if ( g_model.mixTime )
#endif
	{
		lval *= 2 ;
		attr |= PREC1 ;
	}
  lcd_outdezAtt(FW*16,y,lval,attr);
	return value ;
}

#define MAXSW23POS  6

void putsChnOpRaw( uint8_t x, uint8_t y, MixData *md2, uint8_t attr )
{
	if ( md2->srcRaw == MIX_3POS )
	{
		lcd_putsAttIdx( x, y, SW_3_IDX, md2->sw23pos, attr ) ;
	}
	else
	{
		putsChnRaw( x, y, md2->srcRaw, attr| MIX_SOURCE ) ;
		if ( ( md2->srcRaw >= CHOUT_BASE + 1 ) && ( md2->srcRaw < CHOUT_BASE + NUM_CHNOUT + 1 ) )
		{
			if ( md2->disableExpoDr )
			{
				lcd_putsAtt( x, y, PSTR("OP"), attr ) ;
			}
		}
	}
	asm("") ;
}


void menuProcMixOne(uint8_t event)
{
  MixData *md2 = mixaddress( s_currMixIdx ) ;
#ifdef V2
	Xmem.HiResSlow = md2->hiResSlow ;
#endif

//	static MState2 mstate2 ;
  uint8_t sub ;
#ifdef MIX_WARN
	sub = check_columns(event, 15 ) ;
#else
	sub = check_columns(event, 14 ) ;
#endif
		 
		uint8_t x = TITLEP( PSTR(STR_EDIT_MIX));

    putsChn(x+1*FW,0,md2->destCh,0);

    uint8_t t_pgOfs = evalOffset(sub);
#ifdef MIX_WARN
#ifdef V2
		lcd_puts_Pskip( FH, PSTR(STR_2SOURCE"\037"STR_2WEIGHT"\037""\001"STR_OFFSET"\037"STR_ENABLEEXPO
			"\037"STR_2TRIM"\037\037\037"STR_2SWITCH"\037""\001"STR_MODES"\037"STR_2WARNING"\037"STR_2MULTIPLEX
			"\037""HiResSlowDelay""\037"STR_2DELAY_DOWN"\037"STR_2DELAY_UP"\037"STR_2SLOW_DOWN"\037"
			STR_2SLOW_UP), s_pgOfs ) ;
#else
		lcd_puts_Pskip( FH, PSTR(STR_2SOURCE"\037"STR_2WEIGHT"\037""\001"STR_OFFSET"\037"STR_2FIX_OFFSET
			"\037"STR_ENABLEEXPO"\037"STR_2TRIM"\037\037\037"STR_2SWITCH"\037""\001"STR_MODES"\037"STR_2WARNING
			"\037"STR_2MULTIPLEX"\037"STR_2DELAY_DOWN"\037"STR_2DELAY_UP"\037"STR_2SLOW_DOWN"\037"
			STR_2SLOW_UP), s_pgOfs ) ;
#endif
#else
#ifdef V2
		lcd_puts_Pskip( FH, PSTR(STR_2SOURCE"\037"STR_2WEIGHT"\037""\001"STR_OFFSET"\037"STR_ENABLEEXPO
			"\037"STR_2TRIM"\037\037\037"STR_2SWITCH"\037""\001"STR_MODES"\037"STR_2MULTIPLEX
			"\037""HiResSlowDelay""\037"STR_2DELAY_DOWN"\037"STR_2DELAY_UP"\037"STR_2SLOW_DOWN"\037"
			STR_2SLOW_UP), s_pgOfs ) ;
#else
		lcd_puts_Pskip( FH, PSTR(STR_2SOURCE"\037"STR_2WEIGHT"\037""\001"STR_OFFSET"\037"STR_2FIX_OFFSET
			"\037"STR_ENABLEEXPO"\037"STR_2TRIM"\037\037\037"STR_2SWITCH"\037""\001"STR_MODES
			"\037"STR_2MULTIPLEX"\037"STR_2DELAY_DOWN"\037"STR_2DELAY_UP"\037"STR_2SLOW_DOWN"\037"
			STR_2SLOW_UP), s_pgOfs ) ;
#endif
#endif

    for(uint8_t k=0; k<7; k++)
    {
        uint8_t y = (k+1) * FH;
        uint8_t i = k + t_pgOfs;
        uint8_t attr = sub==i ? InverseBlink : 0;
#ifdef MIX_WARN
    		uint8_t b ;
#endif
        switch(i){
        case 0:
				{
					uint8_t value = md2->srcRaw ;
					putsChnOpRaw( FW*14,y,md2, attr ) ;
					if ( value == MIX_3POS )
					{
						value += md2->sw23pos ;
					}
					else
					{
						if ( value > MIX_3POS )
						{
							value += MAXSW23POS ;
						}
					}
					if ( attr )
					{
            CHECK_INCDEC_H_MODELVAR( value, 1,NUM_XCHNRAW+1+MAX_GVARS+1+NUM_SCALERS + MAXSW23POS ) ;	// MAXSW23POS for switches
						if ( value >= MIX_3POS )
						{
							if ( value > MIX_3POS + MAXSW23POS )
							{
								value -= MAXSW23POS ;
							}
							else
							{
								md2->sw23pos = value - MIX_3POS ;
								value = MIX_3POS ;
							}
						}
						md2->srcRaw = value ;
					}
				}		
				break ;

				case 1:
						md2->weight = gvarMenuItem125( FW*16, y, md2->weight, attr ) ;
            break;
        case 2:
						md2->sOffset = gvarMenuItem125( FW*16, y, md2->sOffset, attr ) ;
            break;
#ifndef V2
        case 3:
						md2->lateOffset = onoffItem( md2->lateOffset, y, i ) ;
#endif
            break;
#ifndef V2
        case 4:
#else
        case 3:
#endif
						if ( ( md2->srcRaw <=4 ) )
						{
							md2->disableExpoDr = offonItem( md2->disableExpoDr, y, i ) ;
						}
						else
						{
							md2->disableExpoDr = onoffMenuItem( md2->disableExpoDr, y, PSTR("\001Use Output   "), i ) ;
						}
						break;
#ifndef V2
        case 5:
#else
        case 4:
#endif
						md2->carryTrim = offonItem( md2->carryTrim, y, i ) ;
						break;
#ifndef V2
        case 6 :
#else
        case 5:
#endif
					{	
					 	uint8_t value = md2->differential ;
						if ( value == 0 )
						{
							if ( ( md2->curve <= -28 ) || ( md2->curve >= 27 ) )
							{
								value = 2 ;		// Expo
							}
						}
					 	uint8_t value2 = value ;
						lcd_putsAtt(  1*FW, y, value ? ( value == 2 ) ? PSTR("\021Expo") : PSTR(STR_15DIFF) : Str_Curve, attr ) ;
						if(attr) CHECK_INCDEC_H_MODELVAR_0( value2, 2 ) ;
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
        break;
#ifndef V2
        case 7 :
#else
        case 6:
#endif
						if ( md2->differential )		// Non zero for curve
						{
		          md2->curve = gvarMenuItem( 12*FW, y, md2->curve, -100, 100, attr ) ;
						}
						else
						{	
							if ( ( md2->curve <= -28 ) || ( md2->curve > 27 ) )
							{
								int8_t value = md2->curve + 128 ;	// 0 to 100, AND -100 to -1
            		lcd_outdezAtt(FW*17,y,value,attr|LEFT);
            		if(attr) CHECK_INCDEC_H_MODELVAR( value, -100, 100 ) ;
								md2->curve = value - 128 ;
							}
							else
							{
								put_curve( 2*FW, y, md2->curve, attr ) ;
          	  	if(attr)
								{
#if defined(CPUM128) || defined(CPUM2561)
									CHECK_INCDEC_H_MODELVAR( md2->curve, -MAX_CURVE5-MAX_CURVE9-1 , MAX_CURVE5+MAX_CURVE9+7-1+1);
#else
									CHECK_INCDEC_H_MODELVAR( md2->curve, -MAX_CURVE5-MAX_CURVE9 , MAX_CURVE5+MAX_CURVE9+7-1);
#endif
									if ( event==EVT_KEY_FIRST(KEY_MENU) )
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
        break;
#ifndef V2
        case 8:
#else
        case 7:
#endif
						md2->swtch = edit_dr_switch( 13*FW, y, md2->swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0 ) ;
            break;
#ifndef V2
        case 9:
#else
        case 8:
#endif
					{
						uint8_t b = 1 ;

						if ( attr )
						{
							Columns = 4 ;
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
#ifdef MIX_WARN
#ifndef V2
        case 10:
#else
        case 9:
#endif
						b = md2->mixWarn ;
						putsOffDecimal( FW*13, y, b, attr ) ;
            if(attr) { CHECK_INCDEC_H_MODELVAR_0( b, 3); md2->mixWarn = b ; }
            break;
#endif
#ifdef MIX_WARN
#ifndef V2
        case 11:
#else
        case 10:
#endif
#else
#ifndef V2
        case 10:
#else
        case 9:
#endif
#endif
						md2->mltpx = checkIndexedV( y, PSTR(FWx13"\002"STR_ADD_MULT_REP), md2->mltpx, i ) ;
            break;
#ifdef V2
#ifdef MIX_WARN
        case 11:
#else
        case 10:
#endif
						md2->hiResSlow = onoffItem( md2->hiResSlow, y, i ) ;
            break;
#endif
#ifdef MIX_WARN
        case 12:
#else
        case 11:
#endif
						md2->delayUp = editSlowDelay( y, attr, md2->delayUp ) ;	// Sense was wrong
            break;
#ifdef MIX_WARN
        case 13:
#else
        case 12:
#endif
						md2->delayDown = editSlowDelay( y, attr, md2->delayDown ) ;	// Sense was wrong
            break;
#ifdef MIX_WARN
        case 14:
#else
        case 13:
#endif
						md2->speedDown = editSlowDelay( y, attr, md2->speedDown ) ;
            break;
#ifdef MIX_WARN
        case 15:
#else
        case 14:
#endif
						md2->speedUp = editSlowDelay( y, attr, md2->speedUp ) ;
            break;
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

static void moveMix(uint8_t idx, uint8_t dir) //true=inc=down false=dec=up - Issue 49
{
    MixData *src= mixaddress( idx ) ; //&g_model.mixData[idx];
    if(idx==0 && !dir)
		{
      if (src->destCh>1)
			{
			  src->destCh--;
			}
			STORE_MODELVARS;
			return ;
		}

    if(idx>MAX_MIXERS || (idx==MAX_MIXERS && dir)) return ;
    uint8_t tdx = dir ? idx+1 : idx-1;
    MixData *tgt= mixaddress( tdx ) ; //&g_model.mixData[tdx];

    if((src->destCh==0) || (src->destCh>NUM_CHNOUT) || (tgt->destCh>NUM_CHNOUT)) return ;

    if(tgt->destCh!=src->destCh)
		{
      if ((dir)  && (src->destCh<NUM_CHNOUT)) src->destCh++;
      if ((!dir) && (src->destCh>0))          src->destCh--;
			STORE_MODELVARS;
			return ;
    }

    //flip between idx and tgt
    memswap( tgt, src, sizeof(MixData) ) ;
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
		lcd_puts_Pleft(2*FH, PSTR(STR_MAX_MIXERS_EXAB));
}

static uint8_t getMixerCount()
{
  if ( g_model.mixData[MAX_MIXERS-1].destCh )
	{
		return MAX_MIXERS ;
	}
	else
	{
		return MAX_MIXERS - 1 ;
	}
}

bool reachMixerCountLimit()
{
    // check mixers count limit
    if (getMixerCount() >= MAX_MIXERS)
    {
        pushMenu(menuMixersLimit);
        return true;
    }
    else
    {
        return false;
    }
}

// May be able to put this in Xmem
uint8_t mixToDelete;

#define YN_NONE	0
#define YN_YES	1
#define YN_NO		2

uint8_t yesNoMenuExit( uint8_t event, const prog_char * s )
{
	uint8_t reply = YN_NONE ;
	lcd_puts_Pleft(1*FH, s ) ;	
  lcd_puts_Pleft( 5*FH,PSTR(STR_YES_NO_MENU_EXIT));

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
    popMenu(true);
	}
	return reply ;
}


void menuDeleteMix(uint8_t event)
{
	uint8_t action ;
	action = yesNoMenuExit( event, (mixToDelete== 0xFF) ? PSTR(STR_CLEAR_ALL_MIXES) : PSTR(STR_DELETE_MIX) ) ;
    
	switch(action)
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
      pushMenu(menuProcMix);
    break;
  }
}

uint8_t s_moveMode;

#define POPUP_NONE			0
#define POPUP_SELECT		1
#define POPUP_EXIT			2
#define POPUP_LONG			3


static uint8_t popupProcess( uint8_t max )
{
	int8_t popidxud = qRotary() ;
	struct t_popupData *ptrPopData = &PopupData ;
	FORCE_INDIRECT(ptrPopData) ;

	uint8_t popidx = ptrPopData->PopupIdx ;
  
	if ( ptrPopData->PopupTimer )
	{
		if ( BLINK_ON_PHASE )
		{
			if ( --ptrPopData->PopupTimer == 0 )
			{
				// Timeout popup
				ptrPopData->PopupActive = 0 ;
				return POPUP_EXIT ;
			}
		}
	}
	else
	{
		ptrPopData->PopupTimer = 255 ;
  }
	
	switch(Tevent)
	{
    case EVT_KEY_BREAK(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE):
			ptrPopData->PopupActive = 0 ;
			ptrPopData->PopupTimer = 0 ;
		return POPUP_SELECT ;
    
		case EVT_KEY_FIRST(KEY_UP) :
			popidxud = -1 ;
		break ;
    
		case EVT_KEY_FIRST(KEY_DOWN) :
			popidxud = 1 ;
		break ;
    
		case EVT_KEY_FIRST(KEY_EXIT) :
		case EVT_KEY_LONG(BTN_RE) :
			killEvents( Tevent ) ;
			Tevent = 0 ;
			ptrPopData->PopupActive = 0 ;
			ptrPopData->PopupTimer = 0 ;
		return POPUP_EXIT ;
	}

	if (popidxud > 0)
	{
		if ( popidx < max )
		{
			popidx += 1 ;
		}
	}
	else if (popidxud < 0)
	{		
		if ( popidx )
		{
			popidx -= 1 ;
		}
	}
	
	if (popidxud )
	{
		if ( ptrPopData->PopupTimer )
		{
			ptrPopData->PopupTimer = 255 ;
		}
	}	

	ptrPopData->PopupIdx = popidx ;
	return POPUP_NONE ;
}

const prog_char APM MixPopList[] = STR_MIX_POPUP ;

static uint8_t popupDisplay( const prog_char *list, uint8_t mask, uint8_t width )
{
	uint8_t entries = 0 ;
	uint8_t y = FH ;

	while ( mask )
	{
		if ( mask & 1 )
		{
			lcd_putsn_P( 3*FW, y, PSTR("              "), width ) ;
			lcd_puts_P( 4*FW, y, (const char *)(list) ) ;
			entries += 1 ;
			y += FH ;
		}
		mask >>= 1 ;
		while ( pgm_read_byte(list) )
		{
			list += 1 ;			
		}		
		list += 1 ;			
	}
	lcd_rect( 3*FW, 1*FH-1, width*FW, y+2-FH ) ;
	lcd_char_inverse( 4*FW, (PopupData.PopupIdx+1)*FH, (width-2)*FW, 0 ) ;
	return entries ;
}

static uint8_t popTranslate( uint8_t popidx, uint8_t mask )
{
	uint8_t position ;
	popidx += 1 ;
	for ( position = 0 ; position < 8 ; position += 1 )
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



uint8_t doPopup( const prog_char *list, uint8_t mask, uint8_t width )
{
	CPU_UINT count = popupDisplay( list, mask, width ) ;
	uint8_t popaction = popupProcess( count - 1 ) ;
	uint8_t popidx = PopupData.PopupIdx ;
	PopupData.PopupSel = popTranslate( popidx, mask ) ;
	return popaction ;
}


static void mixpopup()
{
	uint8_t popaction = doPopup( MixPopList, 0x3F, 11 ) ;
	
	uint8_t popidx = PopupData.PopupSel ;

  if ( popaction == POPUP_SELECT )
	{
		if ( popidx == 1 )
		{
      if ( !reachMixerCountLimit())
      {
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
			killEvents(Tevent);
			Tevent = 0 ;
			pushMenu(menuDeleteMix);
		}
		else if ( popidx == 5 )		// Clear all
		{
			mixToDelete = 0xFF ;
			killEvents(Tevent);
			Tevent = 0 ;
			pushMenu(menuDeleteMix);
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
	s_moveMixIdx = s_currMixIdx ;

}

void menuProcMix(uint8_t event)
{
	TITLEP(Str_Mixer);
//	static MState2 mstate2;
	uint8_t sub = MenuVertStack[g_menuVertPtr] ;
//	uint8_t sub = mstate2.m_posVert ;

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
		uint8_t v = sub ;
		if ( ( ( v == 0 ) && ( event == EVT_KEY_FIRST(KEY_UP) ) ) 
				 || ( ( v == s_mixMaxSel ) && ( event == EVT_KEY_FIRST(KEY_DOWN) ) ) )
		{
			event = 0 ;
		}
		Tevent = event ;
	}

	if ( !PopupData.PopupActive )
	{
			sub = check_columns(event,s_mixMaxSel) ;
	}

//	uint8_t  sub    = mstate2.m_posVert + 1 ;
	uint8_t	menulong = 0 ;
	
  switch(Tevent)
	{
    case EVT_ENTRY:
	    s_moveMode = false ;
		break ;

    case EVT_KEY_FIRST(KEY_MENU):
    case EVT_KEY_BREAK(BTN_RE):
			if ( s_moveMode )
			{
	    	s_moveMode = false ;
    		s_editMode = false ;
				RotaryState = ROTARY_MENU_UD ;
				break ;
			}
			// Else fall through    
			if ( !PopupData.PopupActive )
			{
		  	killEvents(Tevent);
				Tevent = 0 ;			// Prevent changing weight to/from Gvar
				menulong = 1 ;
			}
    break;
	}

	uint8_t t_pgOfs = evalOffset( sub) ;
	 
	if ( PopupData.PopupActive )
	{
		Tevent = 0 ;
	}

  uint8_t mix_index = 0 ;
//  uint8_t current = 1 ;
  uint8_t current = 0 ;

	if ( s_moveMode )
	{
		uint8_t dir ;

		if ( ( dir = (Tevent == EVT_KEY_FIRST(KEY_DOWN) ) ) || Tevent == EVT_KEY_FIRST(KEY_UP) )
		{
			moveMix( s_currMixIdx, dir ) ; //true=inc=down false=dec=up - Issue 49
		}
	}

  for ( uint8_t chan=1 ; chan <= NUM_CHNOUT ; chan += 1 )
	{
    MixData *pmd ;
		
		pmd=mixaddress(mix_index) ;

//    if ( t_pgOfs < current && current-t_pgOfs < 8)
    if ( t_pgOfs <= current && current-t_pgOfs < 7)
		{
      putsChn( 1, (current-t_pgOfs+1)*FH, chan, 0) ; // show CHx
    }

		uint8_t firstMix = mix_index ;

		if (mix_index<MAX_MIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)
		{
    	do
			{
				if (t_pgOfs <= current )
				{
//					if ( current-t_pgOfs < 8 )
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
        	 		lcd_putsAttIdx( 3*FW+1, y, Str_plusStarR, pmd->mltpx, 0 ) ;

						putsChnOpRaw( 8*FW, y, pmd, 0 ) ;
						
#if defined(CPUM128) || defined(CPUM2561)
						if ( attr )
						{
							singleBar( 96, 4, g_chans512[chan-1] ) ;
						}
#endif

						pmd->weight = gvarMenuItem125( 7*FW+FW/2, y, pmd->weight, attr ) ;
    	  	  if( pmd->swtch) putsDrSwitches( 12*FW, y, pmd->swtch, 0 ) ; //tattr);
						if ( pmd->curve )
						{
							if ( pmd->differential ) lcd_putc( 16*FW, y, CHR_d ) ;
							else
							{
								if ( ( pmd->curve > -28 ) && ( pmd->curve <= 27 ) )
								{
	    	  	  		put_curve( 16*FW, y, pmd->curve, 0 ) ;
								}
								else
								{
									lcd_putc( 16*FW, y, 'E' ) ;
								}
							}
						}
						char cs = ' ';
        	  if (pmd->speedDown || pmd->speedUp)
        	    cs = CHR_S ;
        	  if ((pmd->delayUp || pmd->delayDown))
        	    cs = (cs ==CHR_S ? '*' : CHR_D);
        	  lcd_putc(20*FW+1, y, cs ) ;

						if ( s_moveMode )
						{
							if ( s_moveMixIdx == mix_index )
							{
								lcd_char_inverse( 4*FW, y, 17*FW, 0 ) ;
								s_currMixIdx = mix_index ;
//								sub = mstate2.m_posVert = current - 1 ;
								sub = MenuVertStack[g_menuVertPtr] = current ;
							}
						}
					}
					else
					{
//						if ( current-t_pgOfs == 8 )
						if ( current-t_pgOfs == 7 )
						{
							if ( s_moveMode )
							{
								if ( s_moveMixIdx == mix_index )
								{
									MenuVertStack[g_menuVertPtr] += 1 ;								
								}
							}
						}
					}
				}
				current += 1 ; mix_index += 1; pmd += 1 ;  // mixCnt += 1 ; 
    	} while ( (mix_index<MAX_MIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)) ;
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
//						return ;
      		}
				}
			}
			current += 1 ;
		}
	}
#ifndef NO_TEMPLATES
//  if ( t_pgOfs < current && current-t_pgOfs < 8)
  if ( t_pgOfs <= current && current-t_pgOfs < 7)
	{
   	lcd_puts_Pleft( 7*FH,PSTR("Templates\020MENU") ) ;
//		if (sub == s_mixMaxSel + 1 )
		if (sub == s_mixMaxSel )
		{
			lcd_char_inverse( 0, 7*FH, 21*FW, 0 ) ;
			if ( event == EVT_KEY_FIRST(KEY_MENU) || event == EVT_KEY_BREAK(BTN_RE) )
			{
				pushMenu( menuProcTemplates ) ;
			}
		}
	}
#endif
	if ( PopupData.PopupActive )
	{
		Tevent = event ;
		mixpopup() ;
    s_editMode = false;
	}
//#if defined(CPUM128) || defined(CPUM2561)
#ifndef NO_TEMPLATES
//	s_mixMaxSel = current - 1 ;
	s_mixMaxSel = current ;
#else
//	s_mixMaxSel = current - 2 ;
	s_mixMaxSel = current - 1 ;
#endif
//#else
//	s_mixMaxSel = current - 2 ;
//#endif
}

uint16_t expou(uint16_t x, uint16_t k)
{
  // previous function was this one:
    // k*x*x*x + (1-k)*x
//    return ((unsigned long)x*x*x/0x10000*k/(RESXul*RESXul/0x10000) + (RESKul-k)*x+RESKul/2)/RESKul;

  uint32_t value = (uint32_t) x*x;
  value *= (uint32_t)k;
  value >>= 8;
  value *= (uint32_t)x;
  value >>= 12;
  value += (uint32_t)(100-k)*x+50;

  // return divu100(value);
  return value/100;
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
    int16_t   y;
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

void editExpoVals( uint8_t edit,uint8_t x, uint8_t y, uint8_t which, uint8_t exWt, uint8_t stkRL)
{
    uint8_t  invBlk = ( edit ) ? InverseBlink : 0 ;
		uint8_t doedit ;
		int8_t *ptr ;
		ExpoData *eptr ;

		doedit = (edit ) ;

		eptr = &g_model.expoData[s_expoChan] ;
    if(which==DR_DRSW1) {

				eptr->drSw1 = edit_dr_switch( x, y, eptr->drSw1, invBlk, doedit ) ;
    }
    else if(which==DR_DRSW2) {
				eptr->drSw2 = edit_dr_switch( x, y, eptr->drSw2, invBlk, doedit ) ;
    }
    else
		{
				ptr = &eptr->expo[which][exWt][stkRL] ;
				FORCE_INDIRECT(ptr) ;
        
				if(exWt==DR_EXPO)
				{
					*ptr = gvarMenuItem( x, y, *ptr, -100, 100, invBlk ) ;
        }
        else
				{
					*ptr = gvarMenuItem( x, y, *ptr+100, 0, 100, invBlk ) - 100 ;
		    }
		}
}

uint8_t char2idx(char c)
{
	uint8_t ret ;
    for(ret=0;;ret++)
    {
        char cc= pgm_read_byte(s_charTab+ret);
        if(cc==c) return ret;
        if(cc==0) return 0;
    }
}
char idx2char(uint8_t idx)
{
    if(idx < NUMCHARS) return pgm_read_byte(s_charTab+idx);
    return ' ';
}

uint8_t DupIfNonzero = 0 ;
int8_t DupSub ;

void menuDeleteDupModel(uint8_t event)
{
	uint8_t action ;
  eeLoadModelName( DupSub,Xmem.buf,sizeof(Xmem.buf));
  lcd_putsnAtt(1,2*FH, Xmem.buf,sizeof(g_model.name),BSS);
  lcd_putc(sizeof(g_model.name)*FW+FW,2*FH,'?');
	action = yesNoMenuExit( event, DupIfNonzero ? PSTR(STR_DUP_MODEL) : PSTR(STR_DELETE_MODEL) ) ;

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
        EFile::rm(FILE_MODEL( DupSub ) ) ; //delete file
      }
      pushMenu(menuProcModelSelect);
    break;
		
		case YN_NO :
      pushMenu(menuProcModelSelect);
    break;
  }
}

#if defined(DSM_BIND_RESPONSE)
uint8_t MultiResponseFlag ;
// Bit 7 = DSMX, Bit 6 = 11mS, bits 3-0 channels
uint8_t MultiResponseData ;
#endif

void menuRangeBind(uint8_t event)
{
//	static uint8_t timer ;
#if defined(CPUM128) || defined(CPUM2561)
	static uint8_t binding = 0 ;
#else
	uint8_t binding = pxxFlag & PXX_BIND ;
#endif
	lcd_puts_Pleft( 3*FH, (binding) ? PSTR("\006BINDING") : PSTR("RANGE CHECK RSSI:") ) ;
  if ( event == EVT_KEY_FIRST(KEY_EXIT) )
	{
    killEvents(event);
		pxxFlag = 0 ;
		popMenu(false) ;
	}

#if defined(DSM_BIND_RESPONSE)
  if ( event == EVT_ENTRY )
	{
		binding = pxxFlag & PXX_BIND ? 1 : 0 ;
		
		if ( g_model.protocol == PROTO_MULTI )
		{
			if ( (g_model.sub_protocol & 0x3F) == M_DSM )
			{
				MultiResponseFlag = 0 ;
			}
		}
	}

	if ( g_model.protocol == PROTO_MULTI )
	{
		if ( (g_model.sub_protocol & 0x3F) == M_DSM )
		{
			if ( MultiResponseFlag )
			{
			
				lcd_puts_Pleft( 7*FH, PSTR("DSM2 22mS\015ch") ) ;
				if ( MultiResponseData & 0x80 )
				{
					lcd_putc( 3*FW, 7*FH, 'X' ) ;
				}
				if ( MultiResponseData & 0x40 )
				{
					lcd_puts_Pleft( 7*FH, PSTR("\00511") ) ;
				}
  			lcd_outdez( 13*FW-2, 7*FH, MultiResponseData & 0x0F ) ;

				int8_t x ;
				uint8_t y ;

				if ( pxxFlag )
				{
					pxxFlag = 0 ;
					x = MultiResponseData & 0x0F ;	// # channels
					y = (MultiResponseData & 0xC0 ) >> 2 ;	// DSMX, 11mS
					g_model.option_protocol = g_model.option_protocol < 0 ? -x : x ;
					y |= g_model.ppmNCH & 0x8F ;
					g_model.ppmNCH = y ;	// Set DSM2/DSMX
					STORE_MODELVARS ;
				}
			}
		}
	}
#endif


#ifdef FRSKY
	if ( binding == 0 )
	{
		lcd_outdezAtt( 12 * FW, 6*FH, FrskyHubData[FR_RXRSI_COPY], DBLSIZE);
	}
#endif
	if ( g_blinkTmr10ms == 0 )
	{
  	audioDefevent(AU_WARNING2) ;
	}
	asm("") ;
}

#if defined(BIND_OPTIONS)
void menuBindOptions(uint8_t event)
{
	uint8_t b ;
	TITLEP(PSTR("Bind Options"));
//	static MState2 mstate2;
	int8_t sub ;
	sub = check_columns( event, 2 ) ;
  switch(event)
  {
		case EVT_ENTRY:
			PxxExtra = 0 ;
		break ;
	
    case EVT_KEY_BREAK(BTN_RE):
    case EVT_KEY_FIRST(KEY_MENU):
			if ( sub == 0 )
		  {
	  		pxxFlag = PXX_BIND ;		    	//send bind code
				chainMenu( menuRangeBind ) ;
			}
		break ;
	}
	
	lcd_puts_Pleft( 1*FH, PSTR("Bind") ) ;
	if(sub == 0)
	{
		lcd_char_inverse( 0, 1*FH, 4*FW, 0 ) ;
	}
	b = PxxExtra & 1 ;
	b = checkIndexedV( 2*FH, Str_Chans1_8_9_16, b, 1 ) ;
	PxxExtra =	( PxxExtra & ~1 ) | b ;
	b = (PxxExtra & 2) ? 1 : 0 ;
	b = checkIndexedV( 3*FH, Str_Telem_NoTelem, b, 2 ) ;
	PxxExtra =	( PxxExtra & ~2 ) | ( b ? 2 : 0 ) ;
}
#endif

static void editTimer( uint8_t sub )
{
	uint8_t subN ;
	uint8_t timer ;
#ifdef GLOBAL_COUNTDOWN
	if ( sub < 5 )
#else
	if ( sub < 7 )
#endif
	{
		displayNext() ;
		subN = 0 ;
		timer = 0 ;
	}
	else
	{
#ifdef GLOBAL_COUNTDOWN
		subN = 5 ;
#else
		subN = 7 ;
#endif
		timer = 1 ;
	}
#ifndef V2
	TimerMode *ptConfig = &TimerConfig[timer] ;
#else
	V2TimerMode *ptConfig = &g_model.timer[timer] ;
#endif // nV2
	FORCE_INDIRECT(ptConfig) ;
	uint8_t y = FH ;
	uint8_t blink = InverseBlink ;

#ifdef GLOBAL_COUNTDOWN
    	lcd_puts_Pleft( y, PSTR(STR_TIMER_TEXT_X));
#else
    	lcd_puts_Pleft( y, PSTR(STR_TIMER_TEXT));
#endif
	    lcd_putc( 5*FW, y, '1'+ timer ) ;
    	putsTime(12*FW-1, y, ptConfig->tmrVal,(sub==subN ? blink:0),(sub==subN ? blink:0) ) ;

#ifndef NOPOTSCROLL
	    if(sub==subN)	// Use s_editing???
#else
	    if(sub==subN)
#endif
			{
				int16_t temp = 0 ;
				StepSize = 60 ;
				CHECK_INCDEC_H_MODELVAR( temp, -60 ,60 ) ;
				ptConfig->tmrVal += temp ;
        if((int16_t)ptConfig->tmrVal < 0) ptConfig->tmrVal=0;
				
			}
			y += FH ;
		subN++ ;

			uint8_t attr = 0 ;
    	if(sub==subN)
			{
   			attr = INVERS ;
    	  CHECK_INCDEC_H_MODELVAR_0( ptConfig->tmrModeA, 1+2+16 ) ;
			}
    	putsTmrMode(10*FW,y,attr, (timer == 0 ) ? 1 : 0x81 ) ;
			y += FH ;
		subN++ ;
  	
  	  attr = 0 ;
  	  if(sub==subN)
			{
	   		attr = blink ;
			}
			uint8_t doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			ptConfig->tmrModeB = edit_dr_switch( 15*FW, y, ptConfig->tmrModeB, attr, doedit ) ;
			y += FH ;
		subN++ ;
	 
			ptConfig->tmrDir = checkIndexedV( y, PSTR(FWx10"\001"STR_COUNT_DOWN_UP), ptConfig->tmrDir, subN ) ;
			y += FH ;
		subN++ ;

#ifndef GLOBAL_COUNTDOWN
			uint8_t b = ptConfig->tmrMbeep ;
  	  ptConfig->tmrMbeep = onoffMenuItem( b, y, Str_minute_Beep, subN ) ;
			y += FH ;
		subN++;

			b = ptConfig->tmrCdown ;
  	  ptConfig->tmrCdown = onoffMenuItem( b, y, Str_Beep_Countdown, subN  ) ;
			y += FH ;
		subN++;
#endif

  	  attr = 0 ;
			if(sub==subN)
			{
				attr = blink ;
			}

#ifdef V2
			int16_t sw = g_model.timer[timer].tmrRstSw ;
#else
			int16_t sw = (timer==0) ? g_model.timer1RstSw : g_model.timer2RstSw ;
#endif // V2
			doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			sw = edit_dr_switch( 13*FW+1, y, sw, attr, doedit ) ;
			if ( timer == 0 )
			{
#ifdef V2
				g_model.timer[0].tmrRstSw = sw ;
#else
				g_model.timer1RstSw = sw ;
#endif // V2
			}
			else
			{
#ifdef V2
				g_model.timer[1].tmrRstSw = sw ;
#else
				g_model.timer2RstSw = sw ;
#endif // V2
			}
			y += FH ;
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
  	lcd_putsAttIdx( x, y, Str_1_RETA, idx, att ) ;
	}
	
	asm("") ;
}

#ifdef V2
 #if defined(CPUM128) || defined(CPUM2561)
  #define NUM_PHASE_ITEMS		5
 #else
  #define NUM_PHASE_ITEMS		4
 #endif
#else
// #if defined(CPUM128) || defined(CPUM2561)
  #define NUM_PHASE_ITEMS		5
// #else
//  #define NUM_PHASE_ITEMS		4
// #endif
#endif

void menuPhaseOne(uint8_t event)
{
#ifdef V2
	V2PhaseData *phase = &g_model.phaseData[s_currIdx];
#else  
	PhaseData *phase = &g_model.phaseData[s_currIdx];
#endif
	
	TITLE(STR_FL_MODE) ;
//	static MState2 mstate2 ;
  uint8_t sub ;
	sub = check_columns(event,NUM_PHASE_ITEMS-1) ;

	lcd_putc( 8*FW, 0, '1'+s_currIdx ) ;

//  int8_t editMode = s_editMode;
	
  for (uint8_t i = 0 ; i < NUM_PHASE_ITEMS ; i += 1 )
	{
    uint8_t y = (i+1) * FH;
		uint8_t attr = (sub==i ? InverseBlink : 0);
    
		switch(i)
		{
      case 0 : // switch
				lcd_puts_Pleft( y, PSTR(STR_SWITCH_TRIMS"\037""Fade In""\037""Fade Out") ) ;
				phase->swtch = edit_dr_switch( 8*FW, y, phase->swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0 ) ;
			break;

      case 1 : // trims
				if ( attr )
				{
					Columns = 3 ;
				}
      	for ( uint8_t t = 0 ; t<NUM_STICKS ; t += 1 )
				{
      	  putsTrimMode( (10+t)*FW, y, s_currIdx+1, t, (g_posHorz==t) ? attr : 0 ) ;
	#ifndef NOPOTSCROLL
      	  if (attr && g_posHorz==t && ( s_editing ) )
	#else
      	  if (attr && g_posHorz==t && ( s_editMode ) )
	#endif
					{
  			    int16_t v = phase->trim[t] + ( TRIM_EXTENDED_MAX + 1 ) ;
      	    if (v < TRIM_EXTENDED_MAX)
						{
							v = TRIM_EXTENDED_MAX;
						}
						int16_t u = v ;
      	    v = checkIncDec16( u, TRIM_EXTENDED_MAX, TRIM_EXTENDED_MAX+MAX_MODES, EE_MODEL ) ;

						if (v != u)
						{
      	      if (v == TRIM_EXTENDED_MAX) v = 0 ;
							phase->trim[t] = v - ( TRIM_EXTENDED_MAX + 1 ) ;
      	    }
      	  }
      	}
      break;
      
			case 2 : // fadeIn
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( phase->fadeIn, 15 ) ;
			  lcd_outdezAtt( 17*FW, y, phase->fadeIn * 5, attr | PREC1 ) ;
			
			break ;
      
			case 3 : // fadeOut
      default:
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( phase->fadeOut, 15 ) ;
			  lcd_outdezAtt( 17*FW, y, phase->fadeOut * 5, attr | PREC1 ) ;
			break ;

#ifdef V2
#if defined(CPUM128) || defined(CPUM2561)
			case 4 : // Phase Name
				alphaEditName( 11*FW-2, y, (uint8_t *)phase->name, sizeof(phase->name), attr, (char *)PSTR( "Mode Name") ) ;
			break ;
#endif
#else
//#if defined(CPUM128) || defined(CPUM2561)
			case 4 : // Phase Name
#if defined(CPUM128) || defined(CPUM2561)
				alphaEditName( 11*FW-2, y, (uint8_t *)g_model.phaseNames[s_currIdx], sizeof(g_model.phaseNames[0]), attr, (char *)PSTR( "Mode Name") ) ;
#else			
				if ( attr )
				{
					Columns = 5 ;
				}
				editName( g_posHorz, y, (uint8_t *)g_model.phaseNames[s_currIdx], sizeof(g_model.phaseNames[0]), attr ) ;
#endif
			break ;
//#endif
#endif
	  }
	}
}

void dispFlightModename( uint8_t x, uint8_t y, uint8_t mode )
{
  lcd_puts_P( x, y, PSTR(STR_SP_FM)+1 ) ;
  lcd_putc( x+2*FW, y, '0'+mode ) ;
}

#define RESTORE_START					0
#define RESTORE_WAIT1					1
#define RESTORE_GET_NAMES			2
#define RESTORE_DISPLAY_NAMES	3
#define RESTORE_FILE_START		4
//#define RESTORE_FILE_1				5
#define RESTORE_FILE_SAVING		5
#define RESTORE_FILE_DONE			6
#define RESTORE_BACKUP				7
//#define RESTORE_BACKUP_2			9
//#define RESTORE_BACKUP_3			10
#define RESTORE_BACKUP_DONE		8
#define RESTORE_LEAVING				9

NOINLINE void serialVoiceBlockSend( uint8_t *data, uint8_t length )
{
	while ( length )
	{
		serialVoiceTx( *data++ ) ;
		length -= 1 ;
	}
}

NOINLINE uint8_t serialVoiceWaitByte( uint8_t byte )
{
	uint8_t b ;
	while ( (	b = getSvFifo() ) != 0xFF )
	{
		if ( b == byte )
		{
			return 1 ;
		}
	}
	return 0 ;
}

static uint8_t rxSerialBlock( struct t_backup_restore *rPtr, uint8_t length )
{
	int16_t w ;
	
	while ( (	w = getSvFifo() ) != -1 )
	{
		( (uint8_t * )rPtr->restoreDirBuffer )[rPtr->byteCount++] = w ;
		if ( rPtr->byteCount >= length )
		{
			return 1 ;
		}
	}
	return 0 ;
}
				
void serialVoiceTxHeader( uint8_t len )
{
	serialVoiceTx( 0x01 ) ;
	serialVoiceTx( 0 ) ;
	serialVoiceTx( len ) ;
}

//static uint8_t RestoreState ;

void startGetNames()
{
	struct t_backup_restore *rPtr ;
	rPtr = &Xmem.restoreData ;
	FORCE_INDIRECT ( rPtr ) ;
	rPtr->gpCount = 0 ;
	rPtr->dirIndex = 0 ;
	rPtr->restoreState = RESTORE_GET_NAMES ;
	serialVoiceTx( 'D' ) ;
}

void progressBar()
{
	lcd_hbar( 14, 6*FH, 100, 8, Xmem.restoreData.size * 50 / (sizeof(g_model) / 2 ) ) ;
}

void brSubHead(const prog_char *s)
{
	lcd_puts_P( 6*FW, 3*FH, s ) ;
}

void menuBackupRestore(uint8_t event)
{
	struct t_backup_restore *rPtr ;
//	static MState2 mstate2 ;
	uint8_t  sub ;
	
	rPtr = &Xmem.restoreData ;
	FORCE_INDIRECT ( rPtr ) ;

	TITLEP( rPtr->type ? Str_Backup : PSTR("Restore") ) ;
	wdt_reset() ;
  
	switch (event)
	{
	  case EVT_ENTRY :
			startSerialVoice() ;
			rPtr->restoreState = RESTORE_START ;
			rPtr->dirOffset = 0 ;
			MenuVertStack[g_menuVertPtr] = 0 ;
		break ;

   	case EVT_KEY_FIRST(KEY_EXIT) :
			serialVoiceTx( 'Z' ) ;
      eeLoadModel(g_eeGeneral.currModel) ;
			rPtr->restoreState = RESTORE_LEAVING ;
			s_editMode = 0 ;
			killEvents(event) ;
		break ;
	}

	sub = MenuVertStack[g_menuVertPtr] ;
//	sub = mstate2.m_posVert ;
  switch (rPtr->restoreState)
	{
		case RESTORE_START :
			serialVoiceTx( VCMD_BOOTREASON/*0x1B*/ ) ;
			serialVoiceTx( VOP_BACKUP     /*0x1B*/ ) ;
			rPtr->byteCount = 0 ;
			rPtr->gpCount = 0 ;
			rPtr->restoreState = RESTORE_WAIT1 ;
		break ;

		case RESTORE_WAIT1 :
		{
			if ( rPtr->gpCount < 5 )
			{
				// Wait for this with timeout
				serialVoiceWaitByte( '>' ) ;
				rPtr->gpCount += 1 ;
			}
			else
			{
				if ( rPtr->type )
				{
					rPtr->subState = 0 ;
					rPtr->restoreState = RESTORE_BACKUP ;
					serialVoiceTx( 'B' ) ;
				}
				else
				{
					startGetNames() ;
				}
			}
		}
		break ;

		case RESTORE_GET_NAMES :
		{	
			uint8_t b ;
static uint16_t count = 0 ;
//static uint8_t last ;
			rPtr->dirOffset = ( sub / 7) * 7 ;
			brSubHead( PSTR("LOADING" ) ) ;
//lcd_outhex4( 0, 4*FH, last ) ;
//lcd_outhex4( 0, 5*FH, count ) ;
			while ( (	b = getSvFifo() ) != 0xFF )
			{
				count += 1 ;
//				last = b ;
				if ( b == 3 )
				{
					rPtr->restoreState = RESTORE_DISPLAY_NAMES ;
					break ;
				}
				if ( b == '\r' )
				{
					b = '\0' ;
				}
				rPtr->restoreDirBuffer[rPtr->dirIndex][rPtr->byteCount] = b ;
				if ( b == '\0' )
				{
					if ( rPtr->gpCount >= rPtr->dirOffset )
					{
						if ( ++rPtr->dirIndex > 7 )
						{
							serialVoiceTx( 'X' ) ;
							rPtr->restoreState = RESTORE_DISPLAY_NAMES ;
							break ;
						}
						else
						{
							serialVoiceTx( 'N' ) ;
						}
					}
					else
					{
						serialVoiceTx( 'N' ) ;
					}
					rPtr->gpCount += 1 ;
					rPtr->byteCount = 0 ;
				}
				else
				{
					if ( ++rPtr->byteCount > 10 )
					{
						rPtr->byteCount = 10 ;
					}
				}
			}
		}
		break ;

		case RESTORE_DISPLAY_NAMES :
		{
			uint8_t b ;
			uint8_t i ;
			int8_t j ;
			uint8_t y = FH ;

			b = rPtr->dirIndex ;
			if ( b > 7 )
			{
				b = 7 ;
			}
			check_columns(event, rPtr->dirOffset + rPtr->dirIndex-1 ) ;

			j = sub - rPtr->dirOffset ;
			if ( ( j < 0 ) || ( j > 6 ) )
			{
				// Need to display next page
//				rPtr->dirOffset += 7 ;
				startGetNames() ;
			}
			else
			{
				for ( i= 0 ; i < b ; i += 1 )
				{
					uint8_t attr = j == i ? INVERS : 0 ;
      	  lcd_putsAtt(  2*FW, y, (char *)rPtr->restoreDirBuffer[i], BSS | attr ) ;
					y += FH ;
				}
   			if ( event ==  EVT_KEY_FIRST(KEY_MENU) )
				{
					// Do the restore, filename = rPtr->restoreDirBuffer[mstate2.m_posVert]
					serialVoiceTx( 'R' ) ;
					rPtr->subState = 0 ;
					rPtr->restoreState = RESTORE_FILE_START ;
				}
			}

		}
		break ;

		case RESTORE_FILE_START :
			brSubHead( PSTR("RESTORING" ) ) ;
			if ( rPtr->subState == 0 )
			{
				rPtr->size = 0 ;
				if ( serialVoiceWaitByte( 'r' ) )
				{
					serialVoiceTxHeader( 13 ) ;
					uint8_t *p = rPtr->restoreDirBuffer[sub - rPtr->dirOffset] ;
					p[11] = (uint8_t)sizeof(g_model) ;
					p[12] = sizeof(g_model) >> 8 ;
					serialVoiceBlockSend( p, 13 ) ;
					rPtr->byteCount = 0 ;
					rPtr->subState = 1 ;
				}
			}
			else if ( rPtr->subState == 1 )
			{
				if ( rxSerialBlock( rPtr, 15 ) )
				{
					uint16_t count = sizeof(g_model) - rPtr->size ;
					if ( count > 12 )
					{
						count = 12 ;
					}

  				memcpy(&((uint8_t *)&g_model)[rPtr->size],
								 &((uint8_t * )Xmem.restoreData.restoreDirBuffer )[3], count ) ;
					rPtr->size += count ;
					if ( rPtr->size >= sizeof(g_model) )
					{
						rPtr->restoreState = RESTORE_FILE_SAVING ;
						serialVoiceTx( 'e' ) ;
					}
					else
					{
						serialVoiceTx( 'n' ) ;
					}
					rPtr->byteCount = 0 ;
				}
			}
			progressBar() ;
//			lcd_outdez( 50, 5*FH, rPtr->size ) ;
		break ;

		case RESTORE_FILE_SAVING :
			brSubHead( PSTR("SAVING" ) ) ;
			if ( modelSave( rPtr->modelIndex ) )
			{
				rPtr->restoreState = RESTORE_FILE_DONE ;
			}
		break ;

		case RESTORE_FILE_DONE :
			brSubHead( PSTR("RESTORED" ) ) ;
		break ;

		case RESTORE_BACKUP :
			brSubHead( Str_Backup ) ;
			
			if ( rPtr->subState == 0 )
			{
				rPtr->size = 0 ;
				if ( serialVoiceWaitByte( 'b' ) )
				{
	    	  eeLoadModelForBackup(rPtr->modelIndex) ;
					serialVoiceTxHeader( 13 ) ;
					serialVoiceBlockSend( (uint8_t *)g_model.name, 10 ) ;
					serialVoiceTx( MDVERS ) ;
					serialVoiceTx( (uint8_t) sizeof(g_model) ) ;
					serialVoiceTx( sizeof(g_model) >> 8 ) ;
					rPtr->subState = 1 ;
				}
			}
			else if ( rPtr->subState == 1 )
			{
				if ( serialVoiceWaitByte( 'c' ) )
				{
					rPtr->subState = 2 ;
					rPtr->size = 0 ;
				}
			}
			else if ( rPtr->subState == 2 )
			{
				if ( rPtr->size >= sizeof(g_model) )
				{
					serialVoiceTx( 0x03 ) ;
					rPtr->subState = 4 ;
				}
				else
				{
					serialVoiceTxHeader( 12 ) ;
					serialVoiceBlockSend( &((uint8_t *)&g_model)[rPtr->size], 12 ) ;
					rPtr->subState = 3 ;
					rPtr->size += 12 ;
				}
			}
			else if ( rPtr->subState == 3 )
			{
				if ( serialVoiceWaitByte( 'd' ) )
				{
					rPtr->subState = 2 ;
				}
			}
			else
			{
				if ( serialVoiceWaitByte( 0x06 ) )
				{
					rPtr->restoreState = RESTORE_BACKUP_DONE ;
				}
			}
			progressBar() ;
//			lcd_outdez( 50, 5*FH, rPtr->size ) ;
		break ;
		
		case RESTORE_BACKUP_DONE :
			brSubHead( PSTR("BACKUP DONE" ) ) ;
		break ;


		case RESTORE_LEAVING :
			if ( serialVoiceWaitByte( 'z' ) )
			{
				stopSerialVoice() ;
				popMenu(false) ;
			}
		break ;
	}
	asm("") ;
}


void menuModelPhases(uint8_t event)
{
	uint8_t i ;
  uint8_t attr ;
  
	TITLEP(Str_Modes) ;
//	static MState2 mstate2 ;
	uint8_t sub ;
	sub = check_columns(event,5-1-1) ;
	

  switch (event)
	{
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_FIRST(BTN_RE) :
        s_currIdx = sub ;
//				RotaryState = ROTARY_MENU_UD ;
        killEvents(event);
        pushMenu(menuPhaseOne) ;
		break;
  }
    
	lcd_puts_Pleft( 2*FH, PSTR(STR_SP_FM0"\012RETA") ) ;

  for ( i=0 ; i<MAX_MODES ; i += 1 )
	{
    uint8_t y=(i+3)*FH ;
    attr = (i == sub) ? INVERS : 0 ;
		
#ifdef V2
		V2PhaseData *p = &g_model.phaseData[i];
#else  
		PhaseData *p = &g_model.phaseData[i];
#endif
		FORCE_INDIRECT(p) ;
		if ( p->fadeIn || p->fadeOut )
		{
	    lcd_putcAtt( 20*FW+1, y, '*', attr ) ;
		}
		dispFlightModename( FW, y, i+1 ) ;
    putsDrSwitches( 4*FW, y, p->swtch, attr ) ;
    for ( uint8_t t = 0 ; t < NUM_STICKS ; t += 1 )
		{
			putsTrimMode( (10+t)*FW, y, i+1, t, attr ) ;
		}
#ifdef V2
#if defined(CPUM128) || defined(CPUM2561)
		lcd_putsnAtt( 15*FW, y, (char *)p->name, sizeof(p->name), BSS ) ;
#endif
#else
//#if defined(CPUM128) || defined(CPUM2561)
		lcd_putsnAtt( 15*FW, y, (char *)g_model.phaseNames[i], sizeof(g_model.phaseNames[0]), BSS ) ;
//#endif
#endif
	}

	i = getFlightPhase() ;
	lcd_rect( 0, (i+2)*FH-1, 4*FW+2, 9 ) ;

}

static void qloadModel( uint8_t event, uint8_t index )
{

// For popup	 
//  eeLoadModelName(k,Xmem.buf,sizeof(Xmem.buf));
//  lcd_putsnAtt( 4*FW, y, Xmem.buf,sizeof(Xmem.buf),BSS);
	
	
	killEvents(event);
	wdt_reset() ;
  eeWaitComplete();    // Wait to load model if writing something
	wdt_reset() ;
  eeLoadModel( index ) ;
	wdt_reset() ;
	AlarmControl.VoiceCheckFlag |= 2 ;// Set switch current states
  STORE_GENERALVARS;
	wdt_reset() ;
  eeWaitComplete();
  checkTHR() ;
  checkSwitches();
	putVoiceQueueUpper( g_model.modelVoice ) ;
}

// Popup?
// SELECT - Selects model and exit
// COPY - copy model to another model slot
// MOVE- move model to another model slot
// DELETE - This one is important.

const prog_char APM ModelPopList[] = STR_MODEL_POPUP ;

void menuProcModelSelect(uint8_t event)
{
//    static MState2 mstate2;
    TITLE(STR_MODELSEL);

//		int8_t subOld = MenuVertStack[g_menuStackPtr] ;
		int8_t subOld = MenuVertStack[1] ;
//    int8_t subOld  = mstate2.m_posVert;
    
		if ( !PopupData.PopupActive )
		{
//			RotaryState = ROTARY_MENU_UD ;
			check_columns(event, MAX_MODELS-1);
		}

    lcd_puts_Pleft(  0, PSTR(STR_11_FREE));
    lcd_outdez(  17*FW, 0, EeFsGetFree());

//		uint8_t sub = MenuVertStack[g_menuStackPtr] ;
		uint8_t sub = MenuVertStack[1] ;
//    uint8_t  sub    = mstate2.m_posVert;
    static uint8_t sel_editMode;
    if ( DupIfNonzero == 2 )
    {
        sel_editMode = false ;
        DupIfNonzero = 0 ;
    }
		uint8_t t_pgOfs = s_pgOfs ;
		uint8_t temp = 1+t_pgOfs ;
    if(sub < temp)
		{
			t_pgOfs = sub > 1 ? sub-1 : 0 ;
		}
    else if(sub > (temp = 4+t_pgOfs) )
		{
		  t_pgOfs = sub - 4 ;
			if ( t_pgOfs > MAX_MODELS-6 )
			{
				t_pgOfs = MAX_MODELS-6 ;
			}
		}
		s_pgOfs = t_pgOfs ;

    for(uint8_t i=0; i<6; i++){
        uint8_t y=(i+2)*FH;
        uint8_t k=i+t_pgOfs;
				lcd_2_digits( 3*FW, y, k+1, (sub==k) ? INVERS : 0 ) ;
        if(k==g_eeGeneral.currModel) lcd_putc(1,  y,'*');
        eeLoadModelName(k,Xmem.buf,sizeof(Xmem.buf));
        lcd_putsnAtt(  4*FW, y, Xmem.buf,sizeof(Xmem.buf),BSS|((sub==k) ? (sel_editMode ? INVERS : 0 ) : 0));
    }

	if ( PopupData.PopupActive )
	{
		uint8_t mask = (g_eeGeneral.currModel == sub) ? 0x59 : 0x7E ;
		if ( eeModelExists( sub ) == 0 )
		{
			mask = 0x96 ;
		}
#if defined(COP328)
    extern uint8_t CfgVc;   // voice firmware may not have the backup feature
		if ( ( g_eeGeneral.speakerMode & 4 ) == 0 || (CfgVc & VC_MBACKUP) == 0 )
#else
		if ( ( g_eeGeneral.speakerMode & 4 ) == 0 )
#endif
		{
			mask &= 0x3F ;	// If no megasound, no backup/restore
		}
		uint8_t popaction = doPopup( ModelPopList, mask, 10 ) ;
		
  	if ( popaction == POPUP_SELECT )
		{
			uint8_t popidx = PopupData.PopupSel ;
			if ( popidx == 0 )	// edit
			{
				chainMenu(menuProcModelIndex) ;
			}
			else if ( ( popidx == 1 ) || ( popidx == 2 ) )	// select or SEL/EDIT
			{
       	g_eeGeneral.currModel = sub ;
				PausePulses = 1 ;
			 	qloadModel( event, sub ) ;
				startPulses() ;
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
				DupSub = sub ;
       	pushMenu(menuDeleteDupModel);
			}
			else if( popidx == 3 )	// copy
			{
				{
 	        DupIfNonzero = 1 ;
 	        DupSub = sub ;
 	        pushMenu(menuDeleteDupModel);
				}
			}
			else if( popidx == 6 )	// backup
			{
				Xmem.restoreData.type = 1 ;
				Xmem.restoreData.modelIndex = sub ;
        pushMenu(menuBackupRestore) ; 
			}
			else if( popidx == 7 )	// restore
			{
				Xmem.restoreData.type = 0 ;
				Xmem.restoreData.modelIndex = sub ;
        pushMenu(menuBackupRestore) ; 
			}
			else // Move (4)
			{
 	    	sel_editMode = true ;
			}
		}
	}
	else
	{
		struct t_popupData *ppupdata = &PopupData ;
		switch(event)
    {
    //case  EVT_KEY_FIRST(KEY_MENU):
    	case  EVT_KEY_FIRST(KEY_EXIT):
        if(sel_editMode)
				{
            sel_editMode = false;
        }
    	break;
    
			case  EVT_KEY_FIRST(KEY_LEFT):
    	case  EVT_KEY_FIRST(KEY_RIGHT):
        if(g_eeGeneral.currModel != sub )
        {
					ppupdata->PopupActive = 2 ;
          killEvents(event);
					ppupdata->PopupIdx = 0 ;
        }
				else
				{
//					RotaryState = ROTARY_MENU_LR ;
		      if(event==EVT_KEY_FIRST(KEY_LEFT))  chainMenu(menuProcModelIndex);//{killEvents(event);popMenu(true);}
        	if(event==EVT_KEY_FIRST(KEY_RIGHT)) { chainMenu(menuProcModelIndex); }
				}
	    break;
    	case  EVT_KEY_FIRST(KEY_MENU) :
			case  EVT_KEY_FIRST(BTN_RE) :
  		  s_editMode = 0 ;
				if(sel_editMode)
				{
  		    sel_editMode = false ;
				}
				else
				{
					ppupdata->PopupActive = 1 ;
				 	killEvents(event) ;
					ppupdata->PopupIdx = 0 ;
				}	 
    	break;
    	case  EVT_KEY_LONG(KEY_EXIT):  // make sure exit long exits to main
        popMenu(true);
  	  break;

	    case EVT_ENTRY:
//				MenuTimer = 2000 ;	// * 0.01 Seconds = 20 seconds
        sel_editMode = false;
				ppupdata->PopupActive = 0 ;
//        MenuVertStack[g_menuStackPtr] = g_eeGeneral.currModel;
        MenuVertStack[1] = g_eeGeneral.currModel;
        eeCheck(true); //force writing of current model data before this is changed
	    break;
    }
	}
  if(sel_editMode && subOld!=sub)
	{
		EFile::swap(FILE_MODEL(subOld),FILE_MODEL(sub));
		
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
	asm("") ;	// Prevents compiler from creating 2 sets of register pops
}


const prog_char APM menuWhenDone[] = STR_MENU_DONE ;

#ifdef XSW_MOD
#define	PUT_ARROW   4
const prog_char APM ITREAG[] = { 'I', 'T', 'R', 'E', 'A', 'G' };
const prog_char APM arrows[] = { '\200', '-', '\201' };

void putc_0_1( uint8_t x, uint8_t y, uint8_t sst )
{
  uint8_t chr, att = 0;
  if (sst & PUT_ARROW) {
    chr = pgm_read_byte(&arrows[sst &= 3]) ;
    if (sst == ST_DN)
      att = INVERS;
  } else {
    chr = '0';
    if (sst) {
      chr += 1;     // '1'
      att = INVERS;
    }
  }
  lcd_putcAtt( x, y, chr, att ) ;
}
#else // !XSW_MOD

#ifdef SWITCH_MAPPING
const prog_char APM ITREAG[] = { 'I', 'T', 'R', 'E', 'A', 'G' };
const prog_char APM arrows[] = { '\200', '-', '\201' };
#endif

void putc_0_1( uint8_t x, uint8_t y, uint8_t value )
{
  lcd_putcAtt( x, y, value+'0', value ? INVERS : 0 ) ;
}
#endif // XSW_MOD

extern uint8_t FrskyTelemetryType ;

struct t_timeGlobals
{
	uint16_t s_timeCumTot ;		// Total tx on time (secs)
	uint16_t s_timeCumAbs ;  //laufzeit in 1/16 sec
	uint16_t s_time ;
	uint8_t s_cnt ;
} ;

struct t_timeGlobals TimeGlobals ;

#define TMR_OFF     0
#define TMR_RUNNING 1
#define TMR_BEEPING 2
#define TMR_STOPPED 3

// 1. Timer=00:00 and count up - Should just announce whole positive minutes
// 2. Timer=00:00 and count down - Is not useful.
// 3. Timer=value and count up - announcing whole minutes left to the value and the last 30 seconds before reaching the value.
// 4. Timer=value and count down - announcing whole minutes left, and the last 30 seconds countdown.
//uint16_t Tval ;
static void timerBeeps()
{
	uint8_t i ;
#ifdef V2
 for ( i = 0 ; i < 2 ; i += 1 )
 {
#else
	i = 0 ;
#endif // V2
	struct t_timer *tptr = &s_timer[i] ;
	FORCE_INDIRECT(tptr) ;
#ifdef V2
	V2TimerMode *ptConfig = &g_model.timer[i] ;
	uint8_t direction = ptConfig->tmrDir ;
	uint16_t timerValue = ptConfig->tmrVal ;
#else
	uint8_t direction = TimerConfig[i].tmrDir ;
	uint16_t timerValue = TimerConfig[i].tmrVal ;
#endif
  if(tptr->last_tmr != tptr->s_timerVal)  //beep only if seconds advance
	{
	  tptr->last_tmr = tptr->s_timerVal;
		if ( tptr->sw_toggled )
//    if(tptr->s_timerState==TMR_RUNNING)
    {
			int16_t tval = tptr->s_timerVal ;

			if ( direction )
			{
				tval = timerValue-tval ;
			}
			 

#ifdef V2
#ifdef GLOBAL_COUNTDOWN
      if(g_eeGeneral.preBeep && timerValue) // beep when 30, 20, 10, 5,4,3,2,1 seconds remaining
#else
      if(ptConfig->tmrCdown && timerValue) // beep when 30, 20, 10, 5,4,3,2,1 seconds remaining
#endif
#else
#ifdef GLOBAL_COUNTDOWN
      if(g_eeGeneral.preBeep && timerValue) // beep when 30, 20, 10, 5,4,3,2,1 seconds remaining
#else
      if(g_model.timer1Cdown && timerValue) // beep when 30, 20, 10, 5,4,3,2,1 seconds remaining
#endif
#endif // V2
      {
				uint8_t flasht = 0 ;
        if(tval==30) {audioVoiceDefevent(AU_TIMER_30, V_30SECS);flasht = 1;}	
        if(tval==20) {audioVoiceDefevent(AU_TIMER_20, V_20SECS);flasht = 1;}		
        if(tval==10) {audioVoiceDefevent(AU_TIMER_10, V_10SECS);flasht = 1;}	
        if(tval<= 5) { flasht = 1;	if(tval >= 0) {audioVoiceDefevent(AU_TIMER_LT3, tval) ;} /*else audioDefevent(AU_TIMER_LT3);*/}

#ifndef MINIMISE_CODE    
        if(g_eeGeneral.flashBeep && flasht )
#else
        if( flasht )
#endif
					g_LightOffCounter = FLASH_DURATION;
      }

			if ( tval < 0 )
			{
				if ( direction )
				{
					tval = tptr->s_timerVal ;
				}
			}
//			Tval = tval ;

			div_t mins ;
			mins = div( tval, 60 ) ;

#ifdef GLOBAL_COUNTDOWN
      if(g_eeGeneral.minuteBeep && ((mins.rem)==0)) //short beep every minute
#else
      if(ptConfig->tmrMbeep && ((mins.rem)==0)) //short beep every minute
#endif
      {
				if ( g_eeGeneral.speakerMode & 2 )
				{
					if ( mins.quot ) { voiceMinutes( mins.quot ) ;}
				}
				else
				{
          audioDefevent(AU_WARNING1);
				}
#ifndef MINIMISE_CODE    
        if(g_eeGeneral.flashBeep)
#endif
				 g_LightOffCounter = FLASH_DURATION;
      }
        
			if ( ( tptr->s_timerVal < 0 ) && ( tptr->s_timerVal >= -MAX_ALERT_TIME) )
	    {
  	    audioDefevent(AU_TIMER_LT3);
#ifndef MINIMISE_CODE    
    	  if(g_eeGeneral.flashBeep)
#endif
					g_LightOffCounter = FLASH_DURATION;
	    }
		}
  }
#ifdef V2
 }					 
#endif // V2
}

void timer()
{
	uint8_t tma ;
	int8_t tmb ;
	uint8_t timer ;
	uint8_t val ;
  int16_t v ;

	struct t_timeGlobals *tgptr = &TimeGlobals ;
	FORCE_INDIRECT(tgptr) ;
  tgptr->s_cnt++ ;

	for( timer = 0 ; timer < 2 ; timer += 1 )
	{
		struct t_timer *ptimer = &s_timer[timer] ;
		FORCE_INDIRECT(ptimer) ;
		
		uint8_t resetting = 0 ;
		if ( timer == 0 )
		{
#ifdef V2
			tmb = g_model.timer[0].tmrRstSw ;
#else
			tmb = g_model.timer1RstSw ;
#endif // nV2
		}
		else
		{
#ifdef V2
			tmb = g_model.timer[1].tmrRstSw ;
#else
			tmb = g_model.timer2RstSw ;
#endif // nV2
		}
		if ( tmb )
		{
    	if(tmb>(TOGGLE_INDEX))	 // toggeled switch
			{
				uint8_t swPos = getSwitch00( tmb-(TOGGLE_INDEX)) ;
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
#ifndef V2
		TimerMode *ptConfig = &TimerConfig[timer] ;
#else
		V2TimerMode *ptConfig = &g_model.timer[timer] ;
#endif // nV2
		
		tma = ptConfig->tmrModeA ;
    tmb = ptConfig->tmrModeB ;
		
    //value for time described in g_model.tmrMode
    //OFFABSRUsRU%ELsEL%THsTH%ALsAL%P1P1%P2P2%P3P3%
    v = 0 ;
    if(( tma > 1 ) && ( tma < TMR_VAROFS ) )
		{
 			v = calibratedStick[3-1] ;
    }
		if ( ThrottleStickyOn )
		{
			v = -RESX ;
		}
   	if(tma>=TMR_VAROFS) // Cxx%
		{
			v = g_chans512[tma-TMR_VAROFS] ;
		}		
		val = ( v + RESX ) / (RESX/16) ;

		if ( tma != TMRMODE_NONE )		// Timer is not off
		{ // We have a triggerA so timer is running 
			if ( tmb > (TOGGLE_INDEX) )
			{
	    	if(!( ptimer->sw_toggled | ptimer->s_sum | tgptr->s_cnt | tgptr->s_time | ptimer->lastSwPos)) ptimer->lastSwPos = 0 ;  // if initializing then init the lastSwPos
  	  	uint8_t swPos = getSwitch00( tmb-(TOGGLE_INDEX) ) ;
	    	if(swPos && !ptimer->lastSwPos)
				{
					ptimer->sw_toggled = !ptimer->sw_toggled ;  //if switch is flipped first time -> change counter state
				}
    		ptimer->lastSwPos = swPos;
			}
			else
			{
				if ( tmb )
				{
  	  		ptimer->sw_toggled = getSwitch00( tmb ) ; //normal switch
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
    
		ptimer->s_sum += val ;
    if(( get_tmr10ms()-tgptr->s_time)<100) continue ; //1 sec
    
		div_t qr ;
		qr = div( ptimer->s_sum, tgptr->s_cnt ) ;
		val = qr.quot ;
		ptimer->s_sum = qr.rem ;

		if ( timer == 0 )
		{
    	tgptr->s_timeCumTot += 1;
	    tgptr->s_timeCumAbs += 1;
		}
		else
		{
	    tgptr->s_cnt   = 0;    // ready for next 100mS
			tgptr->s_time += 100;  // 100*10mS passed
		}

    if(val) ptimer->s_timeCumThr += 1 ;
		if ( !resetting )
		{
    	if(ptimer->sw_toggled) ptimer->s_timeCumSw += 1;
		}
    ptimer->s_timeCum16ThrP += val >> 1 ; // val/2 ;

		uint16_t tv ;
#ifdef V2
    tv = ptimer->s_timerVal = g_model.timer[timer].tmrVal ;
#else
    tv = ptimer->s_timerVal = TimerConfig[timer].tmrVal ;
#endif // V2
    if(tma == TMRMODE_NONE)
		{
//    	ptimer->s_timerState = TMR_OFF ;
		}
    else
		{
			uint16_t subtrahend ;
			if(tma == TMRMODE_ABS)
			{
				if ( tmb == 0 ) subtrahend = tgptr->s_timeCumAbs ;
    		else subtrahend = ptimer->s_timeCumSw ; //switch
			}
    	else if(tma<TMR_VAROFS-1) subtrahend = ptimer->s_timeCumThr ;// stick
		  else subtrahend = ptimer->s_timeCum16ThrP/16 ; // stick% or Cx%
			ptimer->s_timerVal -= subtrahend ;
		}	

//    switch(ptimer->s_timerState)
//    {
//	    case TMR_OFF:
//        if(ptimer->sw_toggled) ptimer->s_timerState=TMR_RUNNING ;
//      break ;
//  	  case TMR_RUNNING:
//        if(!ptimer->sw_toggled) ptimer->s_timerState=TMR_OFF ;
//      break ;
//		}
#ifdef V2
    if(g_model.timer[timer].tmrDir) ptimer->s_timerVal = tv-ptimer->s_timerVal ; //if counting backwards - display backwards
#else
    if(TimerConfig[timer].tmrDir) ptimer->s_timerVal = tv-ptimer->s_timerVal ; //if counting backwards - display backwards
#endif // V2
	}
	timerBeeps() ;

}


#if THROTTLE_TRACE
#define MAXTRACE 120
uint8_t s_traceBuf[MAXTRACE];
uint8_t s_traceWr;
uint8_t s_traceCnt;
#endif // THROTTLE_TRACE
void trace()   // called in perOut - once envery 0.01sec
{
//    timer() ;

#if THROTTLE_TRACE
    uint16_t val = RESX + calibratedStick[3-1]; //Get throttle channel value
    val /= (RESX/16); //calibrate it
#endif // THROTTLE_TRACE

#if THROTTLE_TRACE
	struct t_trace
	{
    uint16_t s_cnt ;
    uint16_t s_sum ;		
    uint16_t s_time ;
	} ;
		static struct t_trace traceVal ;
		struct t_trace *tracePtr = &traceVal ;
		FORCE_INDIRECT(tracePtr) ;

    tracePtr->s_cnt++;
    tracePtr->s_sum+=val;
#else
    static uint16_t s_time;
#endif // THROTTLE_TRACE

		uint16_t t10ms ;
		t10ms = get_tmr10ms() ;

#if THROTTLE_TRACE
    if(( t10ms-tracePtr->s_time)<1000) //10 sec
#else
    if(( t10ms-s_time)<1000) //10 sec
#endif // THROTTLE_TRACE
        
				return;

#if THROTTLE_TRACE
    tracePtr->s_time= t10ms ;
#else
    s_time= t10ms ;
#endif // THROTTLE_TRACE
 
	if ( (g_model.protocol==PROTO_DSM2) && getSwitch00(MAX_DRSWITCH-1) ) audioDefevent(AU_TADA);   //DSM2&MULTI bind mode warning

#if THROTTLE_TRACE
    val   = tracePtr->s_sum/tracePtr->s_cnt;
    tracePtr->s_sum = 0;
    tracePtr->s_cnt = 0;

    if ( s_traceCnt <= MAXTRACE )
		{
			s_traceCnt++;
		}
    s_traceBuf[s_traceWr++] = val;
    if(s_traceWr>=MAXTRACE) s_traceWr=0;
#endif // THROTTLE_TRACE
}


extern unsigned int stack_free() ;

#ifndef REMOVE_FROM_64FRSKY
struct t_latency g_latency = { 0xFF, 0 } ;
#else
struct t_latency g_latency = { 0 } ;
#endif

#ifdef JETI

void menuProcJeti(uint8_t event)
{
    TITLE("JETI");

    switch(event)
    {
    case EVT_KEY_FIRST(KEY_EXIT):
        JETI_DisableRXD();
        chainMenu(menuProc0);
				return ;
        break;
    }

    for (uint8_t i = 0; i < 16; i++)
    {
        lcd_putcAtt((i+2)*FW,   3*FH, JetiBuffer[i], BSS);
        lcd_putcAtt((i+2)*FW,   4*FH, JetiBuffer[i+16], BSS);
    }

    if (JetiBufferReady)
    {
        JETI_EnableTXD();
        if (keyState((EnumKeys)(KEY_UP))) jeti_keys &= JETI_KEY_UP;
        if (keyState((EnumKeys)(KEY_DOWN))) jeti_keys &= JETI_KEY_DOWN;
        if (keyState((EnumKeys)(KEY_LEFT))) jeti_keys &= JETI_KEY_LEFT;
        if (keyState((EnumKeys)(KEY_RIGHT))) jeti_keys &= JETI_KEY_RIGHT;

        JetiBufferReady = 0;    // invalidate buffer

        JETI_putw((uint16_t) jeti_keys);
        _delay_ms (1);
        JETI_DisableTXD();

        jeti_keys = JETI_KEY_NOCHANGE;
    }
}
#endif

void putsTimeNoAtt( uint8_t x,uint8_t y,int16_t tme )
{
	putsTime( x, y, tme, 0, 0 ) ;
}


#ifdef AF_DEBUG

void menuProcStatistic(uint8_t event)
{
extern uint8_t AfCapture ;
	switch(event)
  {
		case EVT_KEY_LONG(BTN_RE) :
    case EVT_KEY_FIRST(KEY_EXIT):
      popMenu(false) ;
    break ;

		case EVT_KEY_FIRST(KEY_MENU) :
			AfCapture = 1 ;
		break ;
  }
  TITLE("DEBUG") ;
extern uint16_t TelRxCount ;
extern uint16_t Uecount ;
	lcd_puts_Pleft( 7*FH, PSTR("TelRxCount") ) ;
 	lcd_outhex4( 12*FW,  7*FH, TelRxCount ) ;
 	lcd_outhex4( 17*FW,  7*FH, Uecount ) ;

void lcd_outhex2(uint8_t x,uint8_t y,uint8_t val) ;
extern uint8_t AfDebug[] ;
extern uint16_t AfCount ;
extern uint8_t AfBytes ;
	lcd_outhex2( 0,  2*FH, AfDebug[0] ) ;
	lcd_outhex2( 13, 2*FH, AfDebug[1] ) ;
	lcd_outhex2( 26, 2*FH, AfDebug[2] ) ;
	lcd_outhex2( 39, 2*FH, AfDebug[3] ) ;
	lcd_outhex2( 52, 2*FH, AfDebug[4] ) ;
	lcd_outhex2( 65, 2*FH, AfDebug[5] ) ;
	lcd_outhex2( 78, 2*FH, AfDebug[6] ) ;
	lcd_outhex2( 91, 2*FH, AfDebug[7] ) ;
	lcd_outhex2( 104,2*FH, AfDebug[8] ) ;
	lcd_outhex2( 117,2*FH, AfDebug[9] ) ;

	lcd_outhex2( 0,  3*FH, AfDebug[10] ) ;
	lcd_outhex2( 13, 3*FH, AfDebug[11] ) ;
	lcd_outhex2( 26, 3*FH, AfDebug[12] ) ;
	lcd_outhex2( 39, 3*FH, AfDebug[13] ) ;
	lcd_outhex2( 52, 3*FH, AfDebug[14] ) ;
	lcd_outhex2( 65, 3*FH, AfDebug[15] ) ;
	lcd_outhex2( 78, 3*FH, AfDebug[16] ) ;
	lcd_outhex2( 91, 3*FH, AfDebug[17] ) ;
	lcd_outhex2( 104, 3*FH, AfDebug[18] ) ;
	lcd_outhex2( 117,  4*FH, AfDebug[19] ) ;

 	lcd_outhex4( 0*FW,  5*FH, AfCount ) ;
 	lcd_outhex4( 6*FW,  5*FH, AfBytes ) ;
 	lcd_outhex4( 12*FW,  5*FH, AfCapture ) ;
	 
}

#else


void menuProcStatistic(uint8_t event)
{
#ifndef REMOVE_FROM_64FRSKY
	static uint8_t statMenuIndex ;
#endif
  TITLE(STR_STAT) ;
  switch(event)
  {
#ifndef REMOVE_FROM_64FRSKY
    
 #if defined(CPUM128) || defined(CPUM2561)
		case EVT_KEY_FIRST(KEY_RIGHT):
			if (++statMenuIndex > 2 )
			{
				statMenuIndex = 0 ;
			}
    break ;
    case EVT_KEY_FIRST(KEY_LEFT) :
			if (statMenuIndex)
			{
				statMenuIndex -= 1 ;
			}
			else
			{
				statMenuIndex = 2 ;
			}
    break ;
 #else		
		case EVT_KEY_FIRST(KEY_RIGHT):
    case EVT_KEY_FIRST(KEY_LEFT) :
			statMenuIndex ^= 1 ;
    break ;
 #endif
#endif
		case EVT_KEY_LONG(BTN_RE) :
    case EVT_KEY_FIRST(KEY_EXIT):
#ifndef REMOVE_FROM_64FRSKY
			statMenuIndex = 0 ;
#endif
      popMenu(false) ;
    break ;
  }

#ifndef REMOVE_FROM_64FRSKY
 #if defined(CPUM128) || defined(CPUM2561)
	if ( statMenuIndex == 1 )
 #else
	if ( statMenuIndex )
 #endif
	{
#endif
		struct t_latency *ptrLat = &g_latency ;
		FORCE_INDIRECT(ptrLat) ;
    if ( event == EVT_KEY_FIRST(KEY_MENU) )
		{
#ifndef REMOVE_FROM_64FRSKY
      ptrLat->g_tmr1Latency_min = 0xff;
      ptrLat->g_tmr1Latency_max = 0;
#endif
      ptrLat->g_timeMain    = 0;
      audioDefevent(AU_MENUS);
    }
		
#ifdef REMOVE_FROM_64FRSKY
    lcd_puts_Pleft( 4*FH, PSTR("tmain\022ms" ) ) ;
#else
    lcd_puts_Pleft( FH, PSTR("tmr1Lat max\022us\037tmr1Lat min\022us\037tmr1 Jitter\022us\037tmain\022ms" ) ) ;
#endif		 
#ifndef REMOVE_FROM_64FRSKY
		lcd_outdez( PARAM_OFS, FH, ptrLat->g_tmr1Latency_max/2) ;
		lcd_outdez( PARAM_OFS, 2*FH, ptrLat->g_tmr1Latency_min/2) ;
		lcd_outdez( PARAM_OFS, 3*FH, (uint8_t)(ptrLat->g_tmr1Latency_max - ptrLat->g_tmr1Latency_min) /2) ;
#endif
	  lcd_outdezAtt( PARAM_OFS, 4*FH, (ptrLat->g_timeMain*25)/4 ,PREC2 ) ;

#ifndef SIMU
 #if STACK_TRACE
    lcd_puts_Pleft( 5*FH, PSTR("Stack\017b"));
    lcd_outhex4( 10*FW+3, 5*FH, stack_free() ) ;
 #endif
#endif

#ifdef CPUM2561
extern uint8_t SaveMcusr ;
    lcd_outhex4( 17*FW, 6*FH, SaveMcusr ) ;
#endif
    
		lcd_puts_Pleft( 7*FH, PSTR("\003[MENU] to refresh"));
		
#ifndef REMOVE_FROM_64FRSKY
	}
 #if defined(CPUM128) || defined(CPUM2561)
	else if ( statMenuIndex == 2 )
	{
	  TITLE("DEBUG") ;
extern uint8_t TelRxCount ;
extern uint16_t Uecount ;
		lcd_puts_Pleft( 7*FH, PSTR("TelRxCount") ) ;
  	lcd_outhex4( 12*FW,  7*FH, TelRxCount ) ;
 		lcd_outhex4( 17*FW,  7*FH, Uecount ) ;
	
#if defined(CPUM128) || defined(CPUM2561)
extern uint8_t AfhdsData[2] ;
  	lcd_outhex4( 0,  2*FH, AfhdsData[0] ) ;
  	lcd_outhex4( 30, 2*FH, AfhdsData[1] ) ;
#endif

//#if defined(CPUM2561)
//extern uint8_t DsmCopyData[] ;
//void lcd_outhex2(uint8_t x,uint8_t y,uint8_t val) ;
//  	lcd_outhex2( 0,  2*FH, DsmCopyData[0] ) ;
//  	lcd_outhex2( 13, 2*FH, DsmCopyData[1] ) ;
//  	lcd_outhex2( 26, 2*FH, DsmCopyData[2] ) ;
//  	lcd_outhex2( 39, 2*FH, DsmCopyData[3] ) ;
//  	lcd_outhex2( 52, 2*FH, DsmCopyData[4] ) ;
//		lcd_outhex2( 65, 2*FH, DsmCopyData[5] ) ;
//  	lcd_outhex2( 78, 2*FH, DsmCopyData[6] ) ;
//  	lcd_outhex2( 91, 2*FH, DsmCopyData[7] ) ;
//  	lcd_outhex2( 104,2*FH, DsmCopyData[8] ) ;
//  	lcd_outhex2( 117,2*FH, DsmCopyData[9] ) ;

//  	lcd_outhex2( 0,  3*FH, DsmCopyData[10] ) ;
//  	lcd_outhex2( 13, 3*FH, DsmCopyData[11] ) ;
//  	lcd_outhex2( 26, 3*FH, DsmCopyData[12] ) ;
//  	lcd_outhex2( 39, 3*FH, DsmCopyData[13] ) ;
//  	lcd_outhex2( 52, 3*FH, DsmCopyData[14] ) ;
//		lcd_outhex2( 65, 3*FH, DsmCopyData[15] ) ;
//  	lcd_outhex2( 78, 3*FH, DsmCopyData[16] ) ;
//  	lcd_outhex2( 91, 3*FH, DsmCopyData[17] ) ;

//		lcd_outhex2( 0,  4*FH, DsmCopyData[19] ) ;

//#endif
	
	
	
	
	}
 #endif
	else
	{
#endif
		struct t_timer *tptr = &s_timer[0] ;
		FORCE_INDIRECT(tptr) ;
	

    lcd_puts_Pleft( FH*0, PSTR("\021TOT\037\001TME\021TSW\037\001STK\021ST%"));

    putsTimeNoAtt(    7*FW, FH*1, TimeGlobals.s_timeCumAbs );
    putsTimeNoAtt(   13*FW, FH*1, tptr->s_timeCumSw );

    putsTimeNoAtt(    7*FW, FH*2, tptr->s_timeCumThr );
    putsTimeNoAtt(   13*FW, FH*2, tptr->s_timeCum16ThrP/16 );

    putsTimeNoAtt(   13*FW, FH*0, TimeGlobals.s_timeCumTot );

// Temp stack trace display

#ifndef SIMU
 #if STACK_TRACE
		{
			static uint16_t *p ;
			if ( event == EVT_ENTRY )
			{
extern unsigned char __bss_end ;
				p = (uint16_t *)&__bss_end ;
			}
    	if ( event == EVT_KEY_FIRST(KEY_UP) )
			{
				p += 12 ;
			}
			uint16_t *q ;
			q = p ;
			lcd_outhex4( 0, 4*FH, (uint16_t) q ) ;
			lcd_outhex4( 30, 4*FH, *q++ ) ;
			lcd_outhex4( 60, 4*FH, *q++ ) ;
			lcd_outhex4( 90, 4*FH, *q++ ) ;
			lcd_outhex4( 0, 5*FH, (uint16_t) q ) ;
			lcd_outhex4( 30, 5*FH, *q++ ) ;
			lcd_outhex4( 60, 5*FH, *q++ ) ;
			lcd_outhex4( 90, 5*FH, *q++ ) ;
			lcd_outhex4( 0, 6*FH, (uint16_t) q ) ;
			lcd_outhex4( 30, 6*FH, *q++ ) ;
			lcd_outhex4( 60, 6*FH, *q++ ) ;
			lcd_outhex4( 90, 6*FH, *q++ ) ;
			lcd_outhex4( 0, 7*FH, (uint16_t) q ) ;
			lcd_outhex4( 30, 7*FH, *q++ ) ;
			lcd_outhex4( 60, 7*FH, *q++ ) ;
			lcd_outhex4( 90, 7*FH, *q++ ) ;

			uint8_t *r ;
			r = (uint8_t *) 0x10fe ;
			lcd_outhex4( 0, 3*FH, *r++ ) ;
			lcd_outhex4( 30, 3*FH, *r ) ;


		}
 #endif
#endif

#if THROTTLE_TRACE
    uint8_t x=5;
    uint8_t y=60;
    lcd_hline(x-3,y,120+3+3);
    lcd_vline(x,y-32,32+3);

    for(uint8_t i=0; i<120; i+=6)
    {
        lcd_vline(x+i+6,y-1,3);
    }
		uint8_t traceWr = s_traceWr ;
    uint8_t traceRd = s_traceCnt>MAXTRACE ? traceWr : 0;
    for(uint8_t i=1; i<=MAXTRACE; i++)
    {
        lcd_vline(x+i,y-s_traceBuf[traceRd],s_traceBuf[traceRd]);
        traceRd++;
        if(traceRd>=MAXTRACE) traceRd=0;
        if(traceRd==traceWr) break;
    }
#endif
#ifndef REMOVE_FROM_64FRSKY
	}
#endif
	asm("") ;

}

#endif


void resetTimern( uint8_t timer )
{
	uint16_t time ;
	uint8_t dir ;
#ifdef V2
	V2TimerMode *ptConfig = &g_model.timer[timer] ;
#else
	TimerMode *ptConfig = &TimerConfig[timer] ;
#endif // V2
	FORCE_INDIRECT(ptConfig) ;
	
	time = ptConfig->tmrVal ;
	dir = ptConfig->tmrDir ;
	
  struct t_timer *tptr = &s_timer[timer] ;
	FORCE_INDIRECT(tptr) ;
	
	tptr->last_tmr = time ;
	tptr->s_timerVal = ( dir ) ? 0 : time ;
//	tptr->s_timerState = TMR_OFF; //is changed to RUNNING dep from mode
  tptr->s_timeCumThr=0;
  tptr->s_timeCumSw=0;
  tptr->s_timeCum16ThrP=0;
	tptr->s_sum = 0 ;
	

}


#if defined(CPUM128) || defined(CPUM2561)
static uint8_t Caps = 0 ;

void displayKeys( uint8_t x, uint8_t y, const char *text, uint8_t count )
{
	while ( count )
	{
		char c = pgm_read_byte(text++) ;
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

const prog_char APM AlphaSource[] = "0123456789qwertyuiopasdfghjkl zxcvbnm_-. " ;

void menuProcAlpha(uint8_t event)
{
	struct t_alpha *Palpha = &Xmem.Alpha ;
	FORCE_INDIRECT(Palpha) ;

	lcd_puts_Pleft( 0, Palpha->PalphaHeading ? (char *)Palpha->PalphaHeading : Str_Name ) ;
//	static MState2 mstate2 ;
	int8_t sub ;
	sub = check_columns( event, 5 ) ;
	
	displayKeys( 2, 3*FH-4, (const char *)&AlphaSource[0], 10 ) ;
	displayKeys( 5, 4*FH-3, (const char *)&AlphaSource[10], 10 ) ;
	displayKeys( 8, 5*FH-2, (const char *)&AlphaSource[20], 10 ) ;
	displayKeys( 11, 6*FH-1, (const char *)&AlphaSource[30], 10 ) ;
	lcd_puts_Pleft( 7*FH, PSTR("CAP SPACE DEL INS") ) ;

	if ( event == EVT_ENTRY )
	{
		Palpha->AlphaIndex = 0 ;
		Palpha->lastSub = sub ;
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
					uint8_t i ;
					for ( i = Palpha->AlphaIndex ; i < (uint8_t)Palpha->AlphaLength-1 ; i += 1 )
					{
						Palpha->PalphaText[i] = Palpha->PalphaText[i+1] ;
					}
					Palpha->PalphaText[Palpha->AlphaLength-1] = ' ' ;
    			eeDirty( EditType & (EE_GENERAL|EE_MODEL));
				}
			}
			if ( g_posHorz == 3 )
			{
				if ( Palpha->AlphaIndex < Palpha->AlphaLength-1 )
				{
					uint8_t i ;
					for ( i = Palpha->AlphaLength-1 ; i > Palpha->AlphaIndex ; i -= 1 )
					{
						Palpha->PalphaText[i] = Palpha->PalphaText[i-1] ;
					}
				}
				Palpha->PalphaText[Palpha->AlphaIndex] = ' ' ;
    		eeDirty( EditType & (EE_GENERAL|EE_MODEL));
			}
		}
		if (sub>0)
		{
			char chr = pgm_read_byte(&AlphaSource[index]) ;
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

  lcd_putsnAtt( 1, FH, (char *)Palpha->PalphaText, Palpha->AlphaLength, BSS ) ;
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
#endif

NOINLINE void resetTimer1(void)
{
  TimeGlobals.s_timeCumAbs=0 ;
	resetTimern( 0 ) ;
}

void resetTimer2()
{
	resetTimern( 1 ) ;
}

#ifdef FRSKY
int16_t AltOffset = 0 ;
#endif


static int8_t inputs_subview = 0 ;
#ifdef NMEA_EXTRA
#if (defined(FRSKY) | defined(HUB))
int16_t AltMax, AltMaxValue, HomeSave = 0 ;
int8_t longpress = 0 ;
int8_t unit ;
#endif
#endif

#ifndef QUICK_SELECT
static
#endif
void putsDblSizeName( uint8_t y )
{
#ifdef SMALL_DBL
#if defined(CPUM128) || defined(CPUM2561)
	lcd_putsnAtt( 8, y, g_model.name, 10, DBLSIZE | BSS ) ;
	lcd_img( 91, 8, speaker, 0 ) ;
	lcd_hbar( 96, 9, 23, 6, (CurrentVolume*100+4)/7 ) ;
#else	
	lcd_putsnAtt( 24, y, g_model.name, 10, DBLSIZE | BSS ) ;
#endif
#else	
	for(uint8_t i=0;i<sizeof(g_model.name);i++)
		lcd_putcAtt(FW*2+i*2*FW-i-2, y, g_model.name[i],DBLSIZE);
#endif
}

void displayOneSwitch( uint8_t x, uint8_t y, uint8_t index )
{
#ifdef XSW_MOD
  if (is3PosSwitch(DSW_IDL + index)) {
    uint8_t sst = switchState(PSW_BASE + index);
    index = (MAX_PSWITCH + MAX_CSWITCH) + index * 3 + sst;
    lcd_putsAttIdx( x, y, Str_Switches, index, 0) ;
    return;
  }
#else // !XSW_MOD
#ifdef SWITCH_MAPPING
//	uint8_t threePosition = 0 ;
	if ( index < 7 )
	{
//		threePosition = XBITMASK( index ) & Sw3posMask ;
		if ( XBITMASK( index ) & Sw3posMask )
		{
			index = pgm_read_byte( &Sw3posIndex[index] ) ;
			index += switchPosition(index) ;
			if ( index > 7 )
			{
				index -= HSW_OFFSET ;
			}
			lcd_putsAttIdx( x, y, Str_Switches, index-1, 0 ) ;
			return ;
		}
	}
	
//	if ( threePosition )
//	{
//			index += switchPosition(index) ;
//			if ( index > 7 )
//			{
//				index -= HSW_OFFSET ;
//			}
//			lcd_putsAttIdx( x, y, Str_Switches, index-1, 0 ) ;
//			return ;
//	}
	
	index = indexSwitch( index ) ;
#endif
#endif // XSW_MOD
	lcd_putsAttIdx( x, y, Str_Switches, index-1, getSwitch00(index) ? INVERS : 0) ;
}

#ifdef XSW_MOD
void switchDisplay( uint8_t j, uint8_t a, int8_t s )
{
  displayOneSwitch( j, 4*FH, a ) ;
  displayOneSwitch( j, 5*FH, a+1 ) ;
  displayOneSwitch( j, 6*FH, a+s+2 ) ;  // to skip PB1 & PB2
}
#else // !XSW_MOD

void switchDisplay( uint8_t j, uint8_t a )
{
#ifdef SWITCH_MAPPING
	displayOneSwitch( j, 4*FH, a ) ;
	displayOneSwitch( j, 5*FH, a+1 ) ;
	displayOneSwitch( j, 6*FH, a+2 ) ;
	if ( a == 0 )
	{
		if ( getSwitch00(HSW_Trainer) )
		{
			lcd_char_inverse( j, 4*FH, 3*FW, 0 ) ;
		}
	}	
#else		 
	displayOneSwitch( j, 4*FH, a ) ;
	displayOneSwitch( j, 5*FH, a+1 ) ;
	if ( a == 6 )
	{
		uint8_t k = HSW_ID0 ;
		k += switchPosition(k) ;
		lcd_putsAttIdx( j, 6*FH, Str_Switches, k-1, getSwitch00(HSW_Trainer) ? INVERS : 0 ) ;
	}
	else
	{	
		displayOneSwitch( j, 6*FH, a+2 ) ;
	}
#endif
	 
//	uint8_t b = a + 3 ;
//	uint8_t y = 4*FH ;
//	for(uint8_t i=a; i<b; y += FH, i += 1 )
//	{
//		displayOneSwitch( j, y, i ) ;
//	}
}
#endif // XSW_MOD

#ifdef FRSKY
void dispA1A2Dbl( uint8_t y )
{
#ifdef V2
  if (g_model.frsky.channels[0].ratio)
  {
      lcd_puts_Pleft( y, PSTR("A1="));
      putsTelemValue( 3*FW, y-FH, frskyTelemetry[0].value, 0,  /*blink|*/DBLSIZE|LEFT ) ;
  }
  if (g_model.frsky.channels[1].ratio)
  {
      lcd_puts_P(11*FW-2, y, PSTR("A2="));
      putsTelemValue( 14*FW, y-FH, frskyTelemetry[1].value, 1,  /*blink|*/DBLSIZE|LEFT ) ;
  }
#else
  if (g_model.frsky.channels[0].opt.alarm.ratio)
  {
      lcd_puts_Pleft( y, PSTR("A1="));
      putsTelemValue( 3*FW, y-FH, frskyTelemetry[0].value, 0,  /*blink|*/DBLSIZE|LEFT ) ;
  }
  if (g_model.frsky.channels[1].opt.alarm.ratio)
  {
      lcd_puts_P(11*FW-2, y, PSTR("A2="));
      putsTelemValue( 14*FW, y-FH, frskyTelemetry[1].value, 1,  /*blink|*/DBLSIZE|LEFT ) ;
  }
#endif
}
#endif


#ifdef V2
#define TEL_PAGE_1	0x20
#define TEL_PAGE_2	0x30
#define TEL_PAGE_3	0x40
#define TEL_PAGE_4	0x50
#else
#define TEL_PAGE_1	0x10
#define TEL_PAGE_2	0x20
#define TEL_PAGE_3	0x30
#define TEL_PAGE_4	0x40
#endif


void menuProc0(uint8_t event)
{
    static uint8_t trimSwLock;
    uint8_t view = g_eeGeneral.view & 0x0f ;
    uint8_t tview = g_eeGeneral.view & 0x70 ;

		StickScrollAllowed = 0 ;

 if ( ! PopupData.PopupActive )
 {
    switch(event)
    {
    case EVT_KEY_BREAK(KEY_MENU):
		case EVT_KEY_BREAK(BTN_RE):
#ifdef FRSKY
					if ( view == e_telemetry )
					{
						PopupData.PopupActive = 1 ;
						PopupData.PopupIdx = 0 ;
      			killEvents(event) ;
						Tevent = 0 ;
					}
#endif // FRSKY
#ifdef NMEA_EXTRA
#if (defined(FRSKY) | defined(HUB))                                    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	  if (longpress == 1) {
		 longpress=0;
		 break;
	  }
#ifdef V2
	  if (( view == e_telemetry) && (( tview & 0x70) == 0x30) ) 	
#else
	  if (( view == e_telemetry) && (( tview & 0x30) == 0x20) ) 	
#endif
	  {
		if (AltOffset == 0)
			AltOffset = -HomeSave ;
		else
		{
			HomeSave = -AltOffset ;
			AltOffset = 0 ;
		}
	  }
#endif
#endif
        break;
		
		case  EVT_KEY_LONG(BTN_RE):// go to last menu
    case  EVT_KEY_LONG(KEY_MENU):// go to last menu
//#if defined(CPUM128) || defined(CPUM2561)
#if 1
#else
#ifdef FRSKY
#ifdef V2
        if( (view == e_telemetry) && ((tview & 0x70) == 0x30 ) )
#else
        if( (view == e_telemetry) && ((tview & 0x30) == 0x20 ) )
#endif
        {
            AltOffset = -getTelemetryValue(FR_ALT_BARO) ;
#ifdef NMEA_EXTRA
#if (defined(FRSKY) | defined(HUB))								//!!!!!!!!!!!!!!!!!
		HomeSave = AltMax = getTelemetryValue(FR_GPS_ALT) ;				//!!!!!!!!!!!!!!!!!!
#endif
#endif
        }
#ifdef V2
        else if( (view == e_telemetry) && ((tview & 0x70) == 0 ) )
#else
        else if( (view == e_telemetry) && ((tview & 0x30) == 0 ) )
#endif
        {
            if ( g_model.frsky.channels[0].opt.alarm.type == 3 )		// Current (A)
						{
				      frskyTelemetry[0].setoffset() ;
						}
            if ( g_model.frsky.channels[1].opt.alarm.type == 3 )		// Current (A)
						{
				      frskyTelemetry[1].setoffset() ;
						}
        }
        else if( (view == e_telemetry) && ((tview & 0x70) == 0x40 ) )	// GPS
				{
					struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;
					FORCE_INDIRECT( maxMinPtr) ;
			
					maxMinPtr->hubMax[FR_GPS_SPEED] = 0 ;
					maxMinPtr->hubMax[FR_GPS_ALT] = 0 ;
				}
        else
        {
#endif
#endif
//#if defined(CPUM128) || defined(CPUM2561)
#if 1
					PopupData.PopupActive = 2 ;
					PopupData.PopupIdx = 0 ;
      		killEvents(event) ;
					event = 0 ;
					Tevent = 0 ;
#else    
		case  EVT_KEY_LONG(BTN_RE):// go to last menu
#ifndef NOPOTSCROLL
		        scroll_disabled = 1;
#endif            
						pushMenu(lastPopMenu());
            killEvents(event);
#endif	// 1      128/2561
//#if defined(CPUM128) || defined(CPUM2561)
#if 1
#else
#ifdef FRSKY
        }
#endif
#endif
        break;
    case EVT_KEY_LONG(KEY_RIGHT):
#ifndef NOPOTSCROLL
        scroll_disabled = 1;
#endif
        pushMenu(menuProcModelSelect);
        killEvents(event);
        break;
    case EVT_KEY_BREAK(KEY_RIGHT):
#if defined(CPUM128) || defined(CPUM2561)
        if(view <= e_inputs1)
#else
        if(view == e_inputs1)
#endif
				{
					int8_t x ;
					x = inputs_subview ;
#if defined(CPUM128) || defined(CPUM2561)
					if ( ++x > ((view == e_inputs1) ? 3 : 1) ) x = 0 ;
#else
 #ifdef V2
					if ( ++x > 3 ) x = 0 ;
 #else
					if ( ++x > 2 ) x = 0 ;
 #endif
#endif
					inputs_subview = x ;
				}	
#ifdef FRSKY
        if(view == e_telemetry)
				{
#ifdef V2
					tview += 0x10 ;
					if ( tview > 0x50 )
					{
						tview = 0 ;
					}
          g_eeGeneral.view = e_telemetry | tview ;
#else

#if defined(CPUM128) || defined(CPUM2561)
					uint8_t t_limit = 0x30 ;
					tview += 0x10 ;
					if ( ( g_model.telemetryProtocol == TELEMETRY_ARDUPILOT ) || ( g_model.telemetryProtocol == 2 ) )
					{
						t_limit = 0x40 ;
					}
					if ( tview > t_limit )
					{
						tview = 0 ;
					}
          g_eeGeneral.view = e_telemetry | tview ;
#else
          g_eeGeneral.view = e_telemetry | ( ( tview + 0x10) & 0x30 ) ;
#endif
#endif
          //            STORE_GENERALVARS;     //eeWriteGeneral();
          //            eeDirty(EE_GENERAL);
          audioDefevent(AU_MENUS);
        }
#endif
        break;
    case EVT_KEY_BREAK(KEY_LEFT):
#if defined(CPUM128) || defined(CPUM2561)
        if(view <= e_inputs1)
#else
        if(view == e_inputs1)
#endif
				{
					int8_t x ;
					x = inputs_subview ;
#if defined(CPUM128) || defined(CPUM2561)
					if ( --x < 0 ) x = (view == e_inputs1) ? 3 : 1 ;
#else
 #ifdef V2
					if ( --x < 0 ) x = 3 ;
 #else
					if ( --x < 0 ) x = 2 ;
 #endif
#endif
					inputs_subview = x ;
				}	
#ifdef FRSKY
        if(view == e_telemetry)
				{
#ifdef V2
					tview -= 0x10 ;
					if ( tview > 0x60 )
					{
						tview = 0x50 ;
					}
          g_eeGeneral.view = e_telemetry | tview ;
#else
#if defined(CPUM128) || defined(CPUM2561)
					tview -= 0x10 ;
					uint8_t t_limit = 0x30 ;
					if ( g_model.telemetryProtocol == TELEMETRY_ARDUPILOT )
					{
						t_limit = 0x40 ;
					}
					if ( tview > 0x60 )
					{
						tview = t_limit ;
					}
          g_eeGeneral.view = e_telemetry | tview ;
#else
          g_eeGeneral.view = e_telemetry | ( ( tview - 0x10) & 0x30 );
#endif
#endif
          //            STORE_GENERALVARS;     //eeWriteGeneral();
          //            eeDirty(EE_GENERAL);
          audioDefevent(AU_MENUS);
        }
#endif
        break;
    case EVT_KEY_LONG(KEY_LEFT):
#ifndef NOPOTSCROLL
        scroll_disabled = 1;
#endif        
				pushMenu(menuProcIndex);
        killEvents(event);
        break;
    case EVT_KEY_BREAK(KEY_UP):
				view += 1 ;
        if( view>=MAX_VIEWS) view = 0 ;
        g_eeGeneral.view = view | tview ;
//        STORE_GENERALVARS;     //eeWriteGeneral();
        eeDirty(EE_GENERAL | 0xA0 ) ;
        audioDefevent(AU_KEYPAD_UP);
        break;
    case EVT_KEY_BREAK(KEY_DOWN):
        if(view>0)
            view = view - 1;
        else
            view = MAX_VIEWS-1;
        g_eeGeneral.view = view | tview ;
//        STORE_GENERALVARS;     //eeWriteGeneral();
        eeDirty(EE_GENERAL | 0xA0 ) ;
        audioDefevent(AU_KEYPAD_DOWN);
        break;
    case EVT_KEY_LONG(KEY_UP):
        pushMenu(menuProcStatistic);
        killEvents(event);
				return ;
        break;
    case EVT_KEY_LONG(KEY_DOWN):
#if defined(JETI)
        JETI_EnableRXD(); // enable JETI-Telemetry reception
        chainMenu(menuProcJeti);
#elif defined(ARDUPILOT)
        ARDUPILOT_EnableRXD(); // enable ArduPilot-Telemetry reception
        chainMenu(menuProcArduPilot);
#elif defined(NMEA)
        NMEA_EnableRXD(); // enable NMEA-Telemetry reception
        chainMenu(menuProcNMEA);
#elif defined(FRSKY)
				view = e_telemetry ;
				g_eeGeneral.view = view | tview ;
        audioDefevent(AU_MENUS);
#else
        pushMenu(menuProcStatistic);
#endif
        killEvents(event);
				return ;
    case EVT_KEY_FIRST(KEY_EXIT):
//        if(s_timer[0].s_timerState==TMR_BEEPING) {
//            s_timer[0].s_timerState = TMR_STOPPED;
//            audioDefevent(AU_MENUS);
//        }
#ifdef FRSKY
//        else if (view == e_telemetry) {
        if (view == e_telemetry) {
            resetTelemetry();
#ifdef NMEA_EXTRA
#if (defined(FRSKY) | defined(HUB))							//!!!!!!!!!!!!!!!!
		AltMax = 0 ;								//!!!!!!!!!!!!!!!!
#endif
#endif
            audioDefevent(AU_MENUS);
        }
#endif
        break;
    case EVT_KEY_LONG(KEY_EXIT):
        resetTimer1();
        resetTimer2();
#ifdef FRSKY
        resetTelemetry();
#ifdef NMEA_EXTRA
#if (defined(FRSKY) | defined(HUB))							//!!!!!!!!!!!!!!!!
  	  AltOffset = AltMax = HomeSave = 0 ;					//!!!!!!!!!!!!!!!!
#endif
#endif
#endif
        audioDefevent(AU_MENUS);
        break;
    case EVT_ENTRY:
        killEvents(KEY_EXIT);
        killEvents(KEY_UP);
        killEvents(KEY_DOWN);
        trimSwLock = true;
				inputs_subview = 0 ;
        break;
    }
//#if defined(CPUM128) || defined(CPUM2561)
 } // !PopupActive
//#endif
		{
			uint8_t tsw ;
			tsw = getSwitch00(g_model.trimSw) ;
	    if( tsw && !trimSwLock) setStickCenter();
  	  trimSwLock = tsw ;
		}

#ifdef FRSKY
    if (view != e_telemetry) {
//#else
//		if ( tview == 0 ) {
#endif
        uint8_t x=FW*2;
        uint8_t att = (g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0) | DBLSIZE;
				uint8_t i ;

				putsDblSizeName( 0 ) ;

        putsVBat( 6*FW+1, 2*FH, att|NO_UNIT);
        lcd_putc( 6*FW+2, 3*FH, 'V');

#ifdef SMALL_DBL
					displayTimer( x+14*FW-3+2, FH*2, 0, DBLSIZE ) ;
#else
					displayTimer( x+14*FW-3, FH*2, 0, DBLSIZE ) ;
#endif // SMALL_DBL
          putsTmrMode(x+7*FW-FW/2,FH*3,0,0);

				i = getFlightPhase() ;
				if ( i )
				{
#ifdef V2
 #if defined(CPUM128) || defined(CPUM2561)
					if ( g_model.phaseData[i-1].name[0] != ' ' )
					{
						lcd_putsnAtt( 6*FW+2, 2*FH, (prog_char *)g_model.phaseData[i-1].name, 6, BSS ) ;
					}
					else
					{
						dispFlightModename( 6*FW+2, 2*FH, i ) ;
					}
 #else
					dispFlightModename( 6*FW+2, 2*FH, i ) ;
 #endif
#else
// #if defined(CPUM128) || defined(CPUM2561)
					if ( g_model.phaseNames[i-1][0] != ' ' )
					{
						lcd_putsnAtt( 6*FW+2, 2*FH, (prog_char *)g_model.phaseNames[i-1], 6, BSS ) ;
					}
					else
					{
						dispFlightModename( 6*FW+2, 2*FH, i ) ;
					}
// #else
//					dispFlightModename( 6*FW+2, 2*FH, i ) ;
// #endif
#endif
					lcd_rect( 6*FW+1, 2*FH-1, 6*FW+2, 9 ) ;
				}
				else
				{
        	lcd_putsAttIdx( 6*FW+2,     2*FH,PSTR(STR_TRIM_OPTS),g_model.trimInc, 0);
//					if ( g_model.thrTrim )
//					{
//						lcd_puts_P(x+8*FW-FW/2-1,2*FH,PSTR(STR_TTM));
//					}
				}
        //trim sliders
        for(uint8_t i=0; i<4; i++)
        {
#define TL 27
            //                        LH LV RV RH
const static prog_uint8_t APM xt[4] = {128*1/4+2, 4, 128-4, 128*3/4-2};
            uint8_t xm,ym;
						xm = modeFixValue( i ) ;
            xm = pgm_read_byte(xt+xm-1) ;
			  	  int16_t valt = getTrimValue( CurrentPhase, i ) ;
//						uint8_t centre = (valt == 0) ;

            int8_t val = valt/4 ;
						if ( val == 0 )
						{
							if ( valt > 0 )
							{
								val += 1 ;
							}
							if ( valt < 0 )
							{
								val -= 1 ;
							}
						}
						if ( val > TL+1 )
						{
							val = TL+1 ;
						}
						else
						{
							if ( val < -(TL+1) )
							{
								val = -(TL+1) ;
							}
						}
//            int8_t val = max((int8_t)-(TL+1),min((int8_t)(TL+1),(int8_t)(valt/4)));
						if( (i == 1) || ( i == 2 ))
						{
              ym=31;
              lcd_vline(xm,   ym-TL, TL*2);

              if((i == 1) || !(g_model.thrTrim))
							{
                lcd_vline(xm-1, ym-1,  3);
                lcd_vline(xm+1, ym-1,  3);
              }
              ym -= val;
            }
						else
						{
              ym=59;
              lcd_hline(xm-TL,ym,    TL*2);
              lcd_hline(xm-1, ym-1,  3);
              lcd_hline(xm-1, ym+1,  3);
              xm += val;
            }
						lcd_rect_xor( xm-3, ym-3, 7, 7 ) ;
//            DO_SQUARE(xm,ym,7) ;
//						if ( centre )
//						{
//            	DO_SQUARE(xm,ym,5) ;
//						}
        }
#ifdef FRSKY
    }
    else {
        lcd_putsnAtt(0, 0, g_model.name, sizeof(g_model.name), BSS|INVERS);
        uint8_t att = (g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0);
        putsVBat(14*FW,0,att);
					displayTimer( 18*FW+3, 0, 0, 0 ) ;
    }
#endif

    if(view<e_inputs1)
		{
#if defined(CPUM128) || defined(CPUM2561)
			if ( inputs_subview > 1 )
			{
				inputs_subview = 0 ;
			}
	    lcd_hlineStip(46, 33, 36, 0x55 ) ;
  	  lcd_hlineStip(46, 34, 36, 0x55 ) ;
    	lcd_hlineStip(46 + inputs_subview * 18, 33, 18, 0xAA ) ;
	    lcd_hlineStip(46 + inputs_subview * 18, 34, 18, 0xAA ) ;
#endif			
        for(uint8_t i=0; i<8; i++)
        {
            uint8_t x0,y0;
#if defined(CPUM128) || defined(CPUM2561)
						uint8_t chan = 8 * inputs_subview + i ;
			      int16_t val = g_chans512[chan];
#else
            int16_t val = g_chans512[i];
#endif
            switch(view)
            {
            case e_outputValues:
                x0 = (i%4*9+3)*FW/2;
                y0 = i/4*FH+40;
                y0 = i<4 ? 40 : 48 ; // /4*FH+40;
                // *1000/1024 = x - x/8 + x/32
#define GPERC(x)  (x - x/32 + x/128)
                lcd_outdezAtt( x0+4*FW , y0, GPERC(val),PREC1 );
                break;
            case e_outputBars:
#define WBAR2 (50/2)
                x0       = i<4 ? 128/4+2 : 128*3/4-2;
                y0       = 38+(i%4)*5;
								singleBar( x0, y0, val ) ;
//    						int16_t limit = (g_model.extendedLimits ? 1280 : 1024);
//                int8_t l = (abs(val) * WBAR2 + 512) / limit ;
//                if(l>WBAR2)  l =  WBAR2;  // prevent bars from going over the end - comment for debugging

//                lcd_hlineStip(x0-WBAR2,y0,WBAR2*2+1,0x55);
//                lcd_vline(x0,y0-2,5);
//                if(val>0){
//                    x0+=1;
//                }else{
//                    x0-=l;
//                }
//                lcd_hline(x0,y0+1,l);
//                lcd_hline(x0,y0-1,l);
                break;
            }
        }
    }
#ifdef FRSKY
    else if(view == e_telemetry) {
				int16_t value ;
				{
            uint8_t x0;//, blink;
            if ( tview == TEL_PAGE_1 )
            {
                    x0 = 3*FW ;
							dispA1A2Dbl( 3*FH ) ;

                    for (int i=0; i<2; i++) {
#ifdef V2
                        if (g_model.frsky.channels[i].ratio)
#else
                        if (g_model.frsky.channels[i].opt.alarm.ratio)
#endif
												{
#ifdef V2
                            if ( g_model.frsky.channels[i].unit == 5 )		// Current (A)
#else
                            if ( g_model.frsky.channels[i].opt.alarm.type == 3 )		// Current (A)
#endif
														{
                              lcd_outdez(x0+FW, 4*FH,  getTelemetryValue(FR_A1_MAH+i) );
														}
														else
														{
                              putsTelemValue(x0+FW, 4*FH, FrskyHubMaxMin.hubMin[i], i, 0 ) ;
														}
                            putsTelemValue(x0+3*FW, 4*FH, FrskyHubMaxMin.hubMax[i], i, LEFT ) ;
                        }
                        x0 = 14*FW-2;
                    }
								// Fuel Gauge
                if (frskyUsrStreaming)
								{
                	lcd_puts_Pleft( 1*FH, PSTR(STR_FUEL)) ;
									x0 = getTelemetryValue(FR_FUEL) ;		// Fuel gauge value
									lcd_hbar( 25, 9, 102, 6, x0 ) ;
								}
                lcd_puts_Pleft( 6*FH, Str_RXeq);
//    						lcd_putsAttIdx( 11 * FW - 2, 6*FH, Str_TXeq, ( g_model.protocol == PROTO_PXX ), 0 ) ;
								putsTxStr( 11 * FW - 2, 6*FH ) ;
			
                lcd_outdezAtt(3 * FW - 2, 5*FH, frskyTelemetry[2].value, DBLSIZE|LEFT);
                lcd_outdezAtt(14 * FW - 4, 5*FH, frskyTelemetry[FR_TXRSI_COPY].value, DBLSIZE|LEFT);
								
								struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;
								FORCE_INDIRECT( maxMinPtr) ;
			
                lcd_outdez(4 * FW, 7*FH, maxMinPtr->hubMin[2] );
                lcd_outdezAtt(6 * FW, 7*FH, maxMinPtr->hubMax[2], LEFT);
                lcd_outdez(15 * FW - 2, 7*FH, maxMinPtr->hubMin[3] );
                lcd_outdezAtt(17 * FW - 2, 7*FH, maxMinPtr->hubMax[3], LEFT);
            }
            else if ( tview == TEL_PAGE_2 )
            {
                if (frskyUsrStreaming)
                {
									
//									displayTemp( 1, 0, 2*FH, DBLSIZE ) ;
	putsTelemetryChannel( 0, 2*FH, (int8_t)1+TEL_ITEM_T1-1, getTelemetryValue(FR_TEMP1+1-1), DBLSIZE | LEFT, 
																(TELEM_LABEL | TELEM_UNIT_LEFT) ) ;
//									displayTemp( 2, 14*FW, 7*FH, 0 ) ;
	putsTelemetryChannel( 14*FW, 7*FH, (int8_t)2+TEL_ITEM_T1-1, getTelemetryValue(FR_TEMP1+2-1), LEFT, 
																(TELEM_LABEL | TELEM_UNIT) ) ;
									
                  lcd_puts_Pleft( 2*FH, PSTR(STR_12_RPM));
                  lcd_outdezAtt(13*FW, 1*FH, getTelemetryValue(FR_RPM), DBLSIZE|LEFT);
                    
                  value = getAltbaroWithOffset() ;
									putsTelemetryChannel( 0, 4*FH, TEL_ITEM_BALT, value, DBLSIZE | LEFT, (TELEM_LABEL | TELEM_UNIT_LEFT)) ;
#ifdef NMEA_EXTRA
#if (defined(FRSKY) | defined(HUB))												//!!!!!!!!!!!!
		   	  if (AltMax < getTelemetryValue(FR_GPS_ALT)) AltMax = getTelemetryValue(FR_GPS_ALT) ;		//!!!!!!!!!!!
			  if (( HomeSave != 0) | ( AltOffset != 0)) 
			  {
				lcd_puts_P(11*FW, 3*FH, PSTR("Amax="));
				lcd_puts_P(11*FW, 4*FH, PSTR("Home="));
				value = -AltOffset ;
				AltMaxValue = AltMax - value ;
				lcd_putc( 15*FW, 3*FH,unit) ;
				lcd_outdezAtt(16*FW, 3*FH, m_to_ft(AltMaxValue), LEFT) ;		// Max Altitude
				lcd_outdezAtt(16*FW, 4*FH,m_to_ft(value), LEFT) ;			// Home Altitude
			  }												//!!!!!!!!!!!
#endif
#else
									lcd_puts_P(12*FW, 4*FH, PSTR("Amax="));
									putsTelemetryChannel( 17*FW, 4*FH, TEL_ITEM_BALT, FrskyHubMaxMin.hubMax[FR_ALT_BARO] + AltOffset, LEFT, 0 ) ;
#endif


                }	
								dispA1A2Dbl( 6*FH ) ;
								lcd_xlabel_decimal( 3*FW, 7*FH, FrskyHubData[FR_RXRSI_COPY], LEFT, Str_RXeq ) ;
								putsTxStr( 8 * FW , 7*FH ) ;
//    						lcd_putsAttIdx( 8 * FW, 7*FH, Str_TXeq, ( g_model.protocol == PROTO_PXX ), 0 ) ;
                lcd_outdezAtt(11 * FW, 7*FH, FrskyHubData[FR_TXRSI_COPY], LEFT);
            }
            else if ( tview == TEL_PAGE_3 )
            {
							uint8_t blink = BLINK | LEADING0 ;
							uint16_t mspeed ;
              if (frskyUsrStreaming)
							{
								blink = LEADING0 ;
							}
							
                lcd_puts_Pleft( 2*FH, PSTR(STR_LAT_EQ"\037"STR_LON_EQ"\037"STR_ALT_MAX"\037"STR_SPD_KTS_MAX"\037V1=\007V2=\016V3=\037V4=\007V5=\016V6=")) ;
                
								lcd_outdezNAtt(8*FW, 2*FH, (uint16_t)getTelemetryValue(FR_GPS_LAT), blink, -5);
                lcd_putc(8*FW, 2*FH, '.') ;
                lcd_outdezNAtt(12*FW, 2*FH, (uint16_t)getTelemetryValue(FR_GPS_LATd), blink, -4);
                
								lcd_outdezNAtt(8*FW, 3*FH, (uint16_t)getTelemetryValue(FR_GPS_LONG), blink, -5);
                lcd_putc(8*FW, 3*FH, '.') ;
                lcd_outdezNAtt(12*FW, 3*FH, (uint16_t)getTelemetryValue(FR_GPS_LONGd), blink, -4);
                
								lcd_outdez(20*FW, 4*FH, FrskyHubMaxMin.hubMax[FR_GPS_ALT] );
                lcd_outdez(20*FW, 3*FH, FrskyHubData[FR_COURSE] );
                
								mspeed = FrskyHubMaxMin.hubMax[FR_GPS_SPEED] ;
                if ( g_model.FrSkyImperial )
								{
									lcd_puts_Pleft( 5*FH, PSTR(STR_11_MPH)) ;
									mspeed = ( mspeed * 589 ) >> 9 ;
								}
                lcd_outdezAtt(20*FW, 5*FH, mspeed, blink & ~LEADING0 );
//                lcd_outdezAtt(20*FW, 5*FH, MaxGpsSpeed, blink );
              if (frskyUsrStreaming)
							{
#ifdef NMEA_EXTRA
#if (defined(FRSKY) | defined(HUB))												//!!!!!!!!!!!!!!
			if (AltMax < FrskyHubData[FR_GPS_ALT]) AltMax = FrskyHubData[FR_GPS_ALT] ;		//!!!!!!!!!!!!!!
			lcd_outdez(20*FW, 4*FH, AltMax ) ;								//!!!!!!!!!!!!!!
#endif
#endif
								lcd_outdez(8 * FW, 4*FH, getTelemetryValue(FR_GPS_ALT) ) ;
								mspeed = getTelemetryValue(FR_GPS_SPEED) ;
                if ( g_model.FrSkyImperial )
								{
									mspeed = ( mspeed * 589 ) >> 9 ;
								}
                lcd_outdez(8*FW, 5*FH, mspeed );		// Speed
//                lcd_outdezAtt(8*FW, 5*FH, FrskyHubData[FR_GPS_SPEED], 0);
                
								{
									uint8_t x, y ;
									x = 6*FW ;
									y = 6*FH ;
									uint8_t max = FrskyBattCells ;
									if ( max > 6 )
									{
										max = 6 ;
									}
	      					for ( uint8_t k=0 ; k<max ; k++ )
									{
										uint8_t blink= PREC2 ;
										if ( k == 3 )
										{
											x = 6*FW ;
											y = 7*FH ;
										}
#if VOLT_THRESHOLD
										if ((FrskyVolts[k] < g_model.frSkyVoltThreshold))
										{
										  blink = BLINK | PREC2 ;
										}
#endif
  									lcd_outdezNAtt( x, y, (uint16_t)FrskyVolts[k] * 2 , blink, 4 ) ;
										x += 7*FW ;
										if ( k == 5 )		// Max 6 cells displayable
										{
											break ;											
										}
	      					}
								}
								//              lcd_putsAtt(6, 2*FH, PSTR("To Be Done"), DBLSIZE);
							}
            }
#if defined(CPUM128) || defined(CPUM2561)
            else if ( tview == TEL_PAGE_4 )
						{
						 if ( g_model.telemetryProtocol == TELEMETRY_ARDUPILOT )
						 {
								// Mavlink over FrSky
								uint8_t attr = 0 ;
								lcd_putsAtt(  0 * FW, 0 * FH, PSTR("          "), 0 ); // clear inversed model name from screen
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
							
								if (frskyUsrStreaming)
								{
									if ( FrskyHubData[FR_BASEMODE] & MAV_MODE_FLAG_SAFETY_ARMED )
									{
										lcd_putsAtt(  1 * FW, 1 * FH, PSTR(STR_MAV_ARMED), blink) ;
									}
									else
									{
										lcd_putsAtt(  0 * FW, 1 * FH, PSTR(STR_MAV_DISARMED), blink) ;
									}
								}
								else
								{
									lcd_putsAtt(  0 * FW, 1 * FH, PSTR(STR_MAV_NODATA), BLINK) ; // NO DRV
								}

//							  lcd_outdez( 15 * FW, 3 * FH, FrskyHubData[FR_BASEMODE] ) ;

								const char *s ;
								uint8_t i = FrskyHubData[FR_TEMP1] ;
								if ( i == 1 )
								{
									s = PSTR(STR_MAV_FM_1) ;
								}
								else if ( i == 2 )
								{
									s = PSTR(STR_MAV_FM_2) ;
								}
								else if ( i == 3 )
								{
									s = PSTR(STR_MAV_FM_3) ;
								}
								else if ( i == 4 )
								{
									s = PSTR(STR_MAV_FM_4) ;
								}
								else if ( i == 5 )
								{
									s = PSTR(STR_MAV_FM_5) ;
								}
								else if ( i == 6 )
								{
									s = PSTR(STR_MAV_FM_6) ;
								}
								else if ( i == 7 )
								{
									s = PSTR(STR_MAV_FM_7) ;
								}
								else if ( i == 8 )
								{
									s = PSTR(STR_MAV_FM_8) ;
								}
								else if ( i == 9 )
								{
									s = PSTR(STR_MAV_FM_9) ;
								}
								else
								{
									s = PSTR(STR_MAV_FM_0) ;
								}
								lcd_putsAtt( 15*FW-1, 1*FH, s, blink ) ;

//		 line 2 - "GPS fix" converted from TEMP2
								uint8_t gps_fix ;
								uint8_t gps_sat ;
								gps_fix = FrskyHubData[FR_TEMP2] / 10 ;
 				        gps_sat = FrskyHubData[FR_TEMP2] - gps_fix * 10 ;
								switch ( gps_fix )
								{
									case 1: 	lcd_putsAtt( 0 * FW, 2*FH, PSTR(STR_MAV_GPS_NO_FIX), BLINK); break;
									case 2: 	lcd_puts_P ( 0 * FW, 2*FH, PSTR(STR_MAV_GPS_2DFIX )); break;
									case 3:
									case 4: 	lcd_puts_P ( 0 * FW, 2*FH, PSTR(STR_MAV_GPS_3DFIX )); break;
									default : lcd_putsAtt( 0 * FW, 2*FH, PSTR(STR_MAV_GPS_NO_GPS), INVERS); break;
								}
//		line 3 left "SAT"
								lcd_puts_P( 0 * FW,   3*FH, PSTR(STR_MAV_GPS_SAT_COUNT) ); // sat
								lcd_outdez( 7 * FW-2, 3*FH, gps_sat ) ;

//		 line 4 left "HDOP"
                int16_t val;
								val = FrskyHubData[FR_GPS_HDOP];
								if (val <= 200)
								{
                	attr  = PREC1;
				   				blink = 0;
				 				}
								else
								{
                  attr = PREC1 | BLINK;
				   				blink = BLINK;
								}
								lcd_putsAtt( 0 * FW,   4*FH, PSTR(STR_MAV_GPS_HDOP), blink ) ; 
                lcd_outdezAtt( 7 * FW-2, 4*FH, val, attr ) ; // hdop

//		 line 5 left  "heartbeat"
								lcd_hbar( 0, 5*FH+1, 40, 4, frskyUsrStreaming ) ; // heartbeat
//		 line 2 right "Baro Alt"
								lcd_puts_P( 15 * FW-1, 2*FH, PSTR(STR_MAV_ALT) ) ; // Alt
								val = get_telemetry_value(TEL_ITEM_BALT) ;
								attr = 0 ;
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
								lcd_puts_P( 15 * FW-1, 3*FH, PSTR(STR_MAV_GALT) ); // GAlt
								lcd_outdez( 21 * FW+1, 3*FH, get_telemetry_value(TEL_ITEM_GALT) ) ;
//		 line 4 right "Home Distance"
								lcd_puts_P( 15 * FW-1, 4*FH, PSTR(STR_MAV_HOME) ); // home
								lcd_outdez( 21 * FW+1, 4*FH, FrskyHubData[FR_HOME_DIST] ) ;
//		 line 5 right "Current, Milliampers"
                lcd_outdezAtt( 20 * FW+1, 5*FH, FrskyHubData[FR_CURRENT], PREC1 ) ;
								lcd_puts_P( 15 * FW-1, 5*FH, PSTR(STR_MAV_CURRENT) ); // "Cur"
//								lcd_outdez ( 20 * FW+1, 5*FH,  FrskyHubData[FR_CURRENT])/10;
		        		lcd_putc   ( 20 * FW+2, 5*FH, 'A');
//		 line 6 left "Rssi"
								lcd_puts_P( 0 * FW, 6*FH, PSTR("Rssi") ); // "Rssi"
								uint8_t rx = FrskyHubData[FR_RXRSI_COPY];
								if (rx > 100) rx = 100;
                lcd_outdezAtt( 7 * FW-3, 6*FH, rx, blink);
//         line 6 right  "Rcq"
                lcd_puts_P( 15*FW-1, 6*FH, PSTR("Rcq") ); // "Rcq"
								uint8_t tx = FrskyHubData[FR_TXRSI_COPY];
								if (tx > 100) tx = 100;
                lcd_outdezAtt( 21*FW+1, 6*FH, tx, blink);

//		 line 7 
                lcd_puts_P( 0 * FW, 7*FH, PSTR("Rx") );
								lcd_hbar( 14, 57, 49, 6, FrskyHubData[FR_RXRSI_COPY] ) ;
                lcd_puts_P( 19 * FW+1, 7*FH, PSTR("Tx") );
								lcd_hbar( 65, 57, 49, 6, FrskyHubData[FR_TXRSI_COPY] ) ;

//				 THROTTLE bar, %
//								lcd_vbar( 0, 1 * FH, 4, 6 * FH - 1, FrskyHubData[FR_RPM] ) ;

								uint8_t x0 = 64 ;
								uint8_t y0 = 31 ;
								uint8_t r  = 23 ;
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
								for (int8_t i = -3 ; i < r ; i += 1 )
								{
//#if defined(CPUM128) || defined(CPUM2561)
//						x = x0 + i * cos ( hdg * PI / 180.0 );
//						y = y0 + i * sin ( hdg * PI / 180.0 );
//#else
									x = x0 + ( i * rcos100 ( hdg ) ) / 100; 
									y = y0 + ( i * rsin100 ( hdg ) ) / 100;
//#endif
									lcd_plot(x, y);
								}


						 }
						 else
						 {
							// Spektrum format screen
							lcd_puts_Pleft( 0*FH, PSTR("\013tssi")) ;
							lcd_puts_Pleft( 2*FH, PSTR("Vbat")) ;
							lcd_puts_Pleft( 2*FH, PSTR("\013RxV")) ;
							lcd_puts_Pleft( 4*FH, PSTR("AMP\013Temp")) ;
							lcd_puts_Pleft( 6*FH, PSTR("RPM\021DSM2")) ;
							
							lcd_vline( 63, 8, 32 ) ;

              lcd_outdezAtt( 17*FW-2, 0*FH, FrskyHubData[FR_RXRSI_COPY], 0 ) ;
              lcd_outdezAtt( 61, 1*FH, FrskyHubData[FR_VOLTS], PREC1|DBLSIZE ) ;
							lcd_outdezAtt( 125, 1*FH, FrskyHubData[FR_RXV], PREC1|DBLSIZE ) ;
							lcd_outdezAtt( 61, 3*FH, FrskyHubData[FR_CURRENT], PREC1|DBLSIZE ) ;
              lcd_outdezAtt( 125, 3*FH, FrskyHubData[FR_TEMP1], DBLSIZE ) ;
              lcd_outdezAtt( 14*FW, 5*FH, FrskyHubData[FR_RPM], DBLSIZE ) ;

							if ( g_model.protocol == PROTO_MULTI )
							{
								if ( ( g_model.sub_protocol & 0x3F ) == M_DSM )
								{
									if ( (g_model.ppmNCH & 0x60 )== 0x20)
									{
										lcd_putc( 20*FW, 6*FH, 'X' ) ;
									}
								}
							}
//							dsmDisplayABLRFH() ;
							for ( uint8_t i = 0 ; i < 6 ; i += 1 )
							{
								uint16_t value = DsmABLRFH[i] ;
								lcd_putc( i*3*(FW+1), 7*FH, pgm_read_byte( Str_ABLRFH + i ) ) ;
								if ( value == 0 )		// 0xFFFF + 1
								{
								  lcd_puts_P( (i+1)*3*(FW+1)-2-(2*FW), 7*FH, PSTR("--") ) ;
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
						} 	 
#endif
            else		// Custom screen
            {
							lcd_vline( 63, 8, 48 ) ;
#ifdef V2
							uint8_t *pindex = g_model.CustomDisplayIndex[( tview == 0 ) ? 0 : 1] ;
              
							for (uint8_t i=0; i<6; i++)
							{
								if ( *pindex )
								{
									uint8_t telIndex = *pindex-1 ;
									putsTelemetryChannel( (i&1)?64:0, (i&0x0E)*FH+2*FH, telIndex, get_telemetry_value(telIndex),
																							 DBLSIZE, TELEM_LABEL|TELEM_UNIT|TELEM_UNIT_LEFT|TELEM_VALUE_RIGHT ) ;
								}
								pindex += 1 ;
							}
#else
              for (uint8_t i=0; i<6; i++)
							{
								if ( g_model.CustomDisplayIndex[i] )
								{
									putsTelemetryChannel( (i&1)?64:0, (i&0x0E)*FH+2*FH, g_model.CustomDisplayIndex[i]-1, get_telemetry_value(g_model.CustomDisplayIndex[i]-1),
																							 DBLSIZE, TELEM_LABEL|TELEM_UNIT|TELEM_UNIT_LEFT|TELEM_VALUE_RIGHT ) ;
								}
							}
#endif							 
							
								lcd_puts_Pleft( 7*FH, Str_RXeq ) ;
								lcd_hbar( 20, 57, 43, 6, FrskyHubData[FR_RXRSI_COPY] ) ;
								putsTxStr( 110, 7*FH ) ;
//    						lcd_putsAttIdx( 110, 7*FH, Str_TXeq, ( g_model.protocol == PROTO_PXX ), 0 ) ;
								lcd_hbar( 65, 57, 43, 6, FrskyHubData[FR_TXRSI_COPY] ) ;
            }
        }
        
    }
#endif
    else if(view<e_timer2){

        doMainScreenGrphics();

#ifdef XSW_MOD
        // inputs_subview:
        //      0             1              2
        // THR:1 AIL:4  SW1:9  SW4:12  SW7:15 SWA:18
        // RUD:2 GEA:5  SW2:10 SW5:13  SW8:16 SWB:19
        // ELE:3 TRN:8  SW3:11 SW6:14  SW9:17 SWC:20
        uint8_t a = inputs_subview + 1;
        uint8_t b = 4;
        int8_t s = -6;  // to get IDx   by skipping PB1=6 & PB2=7
        if (a > 1) {
          b = a * 6;    // 12, 18
          a = b - 3;    // 9, 15
          s = 0;
        }
        switchDisplay( 2*FW-2, a, 0 ) ;
        switchDisplay( 17*FW-2, b, s ) ;
#else // !XSW_MOD
#ifdef SWITCH_MAPPING
        uint8_t a = inputs_subview ;
				if ( a != 0 ) a = a * 6 + 4 ;		// 0, 9, 15
				switchDisplay( 2*FW-2, a ) ;
				switchDisplay( 17*FW-2, a+3 ) ;
#else
        uint8_t a = inputs_subview ;
				uint8_t b = a + 1 ;
				if ( a != 0 ) a = a * 6 + 3 ;		// 0, 9, 15
				switchDisplay( 2*FW-2, a ) ;
				b *= 6 ;		// 6, 12, 18
				switchDisplay( 17*FW-2, b ) ;
#endif        
//				uint8_t j ;
//				for ( j = 10 ; j < 101 ; j += 90 )
//				{
//					if ( j == 100 )
//					{
//						a = inputs_subview ;
//						a += 1 ;
//						a *= 6 ;		// 6, 12, 18
//					}
//					switchDisplay( j, a ) ;
//			}
#endif // XSW_MOD
    }
    else  // New Timer2 display
    {
#ifdef SMALL_DBL
			displayTimer( 27+5*FW, FH*5, 1, DBLSIZE ) ;
#else
			displayTimer( 30+5*FW, FH*5, 1, DBLSIZE ) ;
#endif // SMALL_DBL
      putsTmrMode( 30-2*FW-FW/2,FH*6, 0, 0x80 ) ;
    }
//extern uint16_t Tval ;
//lcd_outhex4( 0, 0, Tval ) ;
//#if defined(CPUM128) || defined(CPUM2561)
#if 1
	if ( PopupData.PopupActive )
	{
		uint8_t popaction ;
		uint8_t popidx = 0 ;
#ifdef FRSKY
    if(PopupData.PopupActive == 1)
		{
			popidx = 5 ;
			popaction = doPopup( PSTR("Zero Alt.\0Zero A1 Offs\0Zero A2 Offs\0Reset GPS"), 0x0F, 14 ) ;
		}
		else
		{
#endif
			popaction = doPopup( Str_Main_Popup, 0x1F, 14 ) ;
#ifdef FRSKY
		}
#endif
		
		MenuControl.UseLastSubmenuIndex = 0 ;
  	if ( popaction == POPUP_SELECT )
		{
			popidx += PopupData.PopupSel ;
			if ( popidx == 0 )	// Model Select
			{
        pushMenu(menuProcModelSelect) ;
			}
			else if( popidx == 1 )	// Edit Model
			{
#ifndef NOPOTSCROLL
        scroll_disabled = 1;
#endif        
				RotaryState = ROTARY_MENU_UD ;
	  	  pushMenu(menuProcModelIndex) ;
			}
			else if( popidx == 2 )	// Last Menu
			{
				MenuControl.UseLastSubmenuIndex = 1 ;
        pushMenu(lastPopMenu());
			}
			else if ( popidx == 3 )	// Radio Setup
			{
        pushMenu(menuProcIndex) ;
			}
			else if( popidx == 4 )	// Statistics
			{
	  	  pushMenu(menuProcStatistic) ;
			}
#ifdef FRSKY
			else if( popidx == 5 )	// Zero Alt.
			{
        AltOffset = -FrskyHubData[FR_ALT_BARO] ;
			}
			else if( popidx == 6 )	// A1 Offset
			{
#ifdef V2
        if ( g_model.frsky.channels[0].unit == 5 )		// Current (A)
#else
        if ( g_model.frsky.channels[0].opt.alarm.type == 3 )		// Current (A)
#endif
				{
				  frskyTelemetry[0].setoffset() ;
				}
			}
			else if( popidx == 7 )	// A2 Offset
			{
#ifdef V2
        if ( g_model.frsky.channels[1].unit == 5 )		// Current (A)
#else
        if ( g_model.frsky.channels[1].opt.alarm.type == 3 )		// Current (A)
#endif
				{
				  frskyTelemetry[1].setoffset() ;
				}
			}
			else if( popidx == 8 )	// GPS reset
			{
				struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;

				maxMinPtr->hubMax[FR_GPS_SPEED] = 0 ;
				maxMinPtr->hubMax[FR_GPS_ALT] = 0 ;
			}
#endif	
		}
	}
#endif	

	asm("") ;
}

static inline uint16_t isqrt32(uint32_t n)
{
    uint16_t c = 0x8000;
    uint16_t g = 0x8000;

    for(;;) {
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
  uint8_t cv9 = idx >= MAX_CURVE5;
  int16_t erg;
	int8_t *crv = curveAddress( idx ) ;
	
#if defined(XYCURVE)
	if ( idx == MAX_CURVE5 + MAX_CURVE9 )
	{ // The xy curve
		crv = g_model.curvexy ;
		cv9 = 2 ;
	}

  x+=RESXu;
  if(x < 0)
	{
    erg = (int16_t)crv[0] * (RESX/4);
  }
	else if(x >= (RESX*2))
	{
    erg = (int16_t)crv[(cv9 ? 8 : 4)] * (RESX/4);
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
			uint8_t i ;

			// handle end points
  	  c = RESX + calc100toRESX(crv[17]) ;
			if (x>c)
			{
				return calc100toRESX(crv[8]) ;
			}
  	  b = RESX + calc100toRESX(crv[9]) ;
			if (x<b)
			{
				return calc100toRESX(crv[0]) ;
			}

			for ( i = 0 ; i < 8 ; i += 1 )
			{
	      a = b ;
  	    b = (i==7 ? c : RESX + calc100toRESX(crv[i+10]));
    	  if (x<=b) break;
			}
			x -= a ;
			deltax = b - a ;
			int32_t y1 = (int16_t)crv[i] * (RESX/4) ;
			int32_t deltay = (int16_t)crv[i+1] * (RESX/4) - y1 ;
			erg = y1 + ( x ) * deltay / deltax ;
		}
		else
		{
			if(cv9)
			{
				qr = div( x, D9 ) ;
				deltax = qr.rem * 2 ;
    	}
			else
			{
				qr = div( x, D5 ) ;
				deltax = qr.rem ;
    	}
	    erg  = (int16_t)crv[qr.quot]*((D5-deltax)/2) + (int16_t)crv[qr.quot+1]*(deltax/2);
		}
  }
	
#else	

  x+=RESXu;
  if(x < 0)
	{
    erg = (int16_t)crv[0] * (RESX/4);
  }
	else if(x >= (RESX*2))
	{
    erg = (int16_t)crv[(cv9 ? 8 : 4)] * (RESX/4);
  }
	else
	{
    int16_t a,dx;
    if(cv9)
		{
      a   = (uint16_t)x / D9;
      dx  =((uint16_t)x % D9) * 2;
    }
		else
		{
      a   = (uint16_t)x / D5;
      dx  = (uint16_t)x % D5;
    }
    erg  = (int16_t)crv[a]*((D5-dx)/2) + (int16_t)crv[a+1]*(dx/2);
  }
#endif
  return erg / 25; // 100*D5/RESX;
}

int16_t calcExpo( uint8_t channel, int16_t value )
{
  uint8_t expoDrOn = get_dr_state(channel);
  uint8_t stkDir = value > 0 ? DR_RIGHT : DR_LEFT;
  if(IS_EXPO_THROTTLE(channel)) stkDir = DR_RIGHT ;
  ExpoData *peData = &g_model.expoData[channel] ;

	int8_t expoval = peData->expo[expoDrOn][DR_EXPO][stkDir] ;

  if(IS_EXPO_THROTTLE(channel)){
      value  = 2*expo((value+RESX)/2,REG100_100(expoval));
			stkDir = DR_RIGHT;
  }
  else
      value  = expo(value,REG100_100(expoval));

  int32_t x = (int32_t)value * (REG(peData->expo[expoDrOn][DR_WEIGHT][stkDir]+100, 0, 100))/100;
  value = (int16_t)x;
  if (IS_EXPO_THROTTLE(channel)) value -= RESX;
	return value ;
}

// static variables used in perOut - moved here so they don't interfere with the stack
// It's also easier to initialize them here.
//int32_t  chans[NUM_CHNOUT] = {0};
//__int24  chans[NUM_CHNOUT] = {0};

#if MAX_MIXERS == 32
#define MIX_SWITCH_BITFIELD	1
#endif

uint8_t  bpanaCenter = 0;
struct t_output
{
	uint16_t sDelay[MAX_MIXERS] ;
	__int24  act   [MAX_MIXERS] ;
#ifdef MIX_SWITCH_BITFIELD
	uint8_t swOnBits[4] ;
#else
	uint8_t  swOn  [MAX_MIXERS] ;
#endif

	int16_t  anas [NUM_XCHNRAW-NUM_CHNOUT-NUM_PPM] ;
	int16_t rawSticks[4] ;
} Output ;

uint8_t	CurrentPhase = 0 ;
struct t_fade
{
	uint8_t  fadePhases ;
	uint16_t fadeRate ;
	uint16_t fadeWeight ;
	uint16_t fadeScale[MAX_MODES+1] ;
	int32_t  fade[NUM_CHNOUT];
} Fade ;

void perOutPhase( int16_t *chanOut, uint8_t att ) 
{
	static uint8_t lastPhase ;
	uint8_t thisPhase ;
	struct t_fade *pFade ;
	pFade = &Fade ;
	FORCE_INDIRECT( pFade ) ;
	
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

int16_t scaleAnalog( uint8_t channel )
{
#ifndef SIMU
	int16_t v = anaIn( channel ) ;
#if defined(CPUM128) || defined(CPUM2561)
	int16_t mid ;
	int16_t neg ;
	int16_t pos ;
	mid = g_eeGeneral.calibMid[channel] ;
  pos = g_eeGeneral.calibSpanPos[channel] ;
  neg = g_eeGeneral.calibSpanNeg[channel] ;

	v -= mid ;
	if ( channel < 4 )
	{
		int16_t deadband = ( g_eeGeneral.stickDeadband >> ( 4 * channel ) ) & 0x0F ;
	  pos -= deadband ;
  	neg -= deadband ;
	
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
	v  =  v * (int32_t)RESX /  (max((int16_t)100,(v>0 ? pos : neg ) ) ) ;
#else
	v -= g_eeGeneral.calibMid[channel];
	v  =  v * (int32_t)RESX /  (max((int16_t)100,(v>0 ?
                                                    g_eeGeneral.calibSpanPos[channel] :
                                                    g_eeGeneral.calibSpanNeg[channel])));
#endif
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

void perOut(int16_t *chanOut, uint8_t att)
{
  int16_t  trimA[4];
  uint8_t  anaCenter = 0;
//    uint16_t d = 0;
	int16_t trainerThrottleValue = 0 ;
	uint8_t trainerThrottleValid = 0 ;

    
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

	uint8_t stickIndex = g_eeGeneral.stickMode*4 ;
	for(uint8_t i=0;i<7;i++)
	{        // calc Sticks

      //Normalization  [0..2048] ->   [-1024..1024]

    int16_t v ;// = anaIn(i);
		v = scaleAnalog( i ) ;

//#ifndef SIMU
//            v -= g_eeGeneral.calibMid[i];
//            v  =  v * (int32_t)RESX /  (max((int16_t)100,(v>0 ?
//                                                              g_eeGeneral.calibSpanPos[i] :
//                                                              g_eeGeneral.calibSpanNeg[i])));
//#endif
//            if(v <= -RESX) v = -RESX;
//            if(v >=  RESX) v =  RESX;
//	  				if ( g_eeGeneral.throttleReversed )
//						{
//							if ( i == THR_STICK )
//							{
//								v = -v ;
//							}
//						}
		uint8_t index = i ;
		if ( i < 4 )
		{
      phyStick[i] = v >> 4 ;
			index = pgm_read_byte(stickScramble+stickIndex+i) ;
		}
    calibratedStick[index] = v; //for show in expo
		// Filter beep centre
		{
			int8_t t = v/16 ;
			uint8_t mask = 1 << index ;
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
        if (!(att&NO_TRAINER) && g_model.traineron) {
#ifdef V2
            V2TrainerMix* td = &g_eeGeneral.trainer.mix[index];
#else
            TrainerMix* td = &g_eeGeneral.trainer.mix[index];
#endif // V2
            if (td->mode && getSwitch00(td->swtch))
						{
							if ( ppmInAvailable )
							{
                uint8_t chStud = td->srcChn ;
                int16_t vStud  = (g_ppmIns[chStud]- g_eeGeneral.trainer.calib[chStud]) /* *2 */ ;
#ifdef V2
                vStud *= td->studWeight ;
                vStud /= 50 ;
#else
                vStud /= 2 ;		// Only 2, because no *2 above
                vStud *= td->studWeight ;
                vStud /= 31 ;
                vStud *= 4 ;
#endif
                switch ((uint8_t)td->mode) {
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
        }

        //===========Swash Ring================
//                if(d && (index==ele_stick || index==ail_stick))
//                    v = int32_t(v)*calc100toRESX(g_model.swashRingValue)/int32_t(d) ;
        //===========Swash Ring================

				if ( att & FADE_FIRST )
				{
    		  Output.rawSticks[index] = v; //set values for mixer
				}
				v = calcExpo( index, v ) ;

        trimA[i] = getTrimValue( CurrentPhase, i )*2 ;
    }
		if ( att & FADE_FIRST )
		{
      Output.anas[index] = v; //set values for mixer
		}
    if(att&NO_INPUT)
		{ //zero input for setStickCenter()
   		if ( i < 4 )
			{
 				if(!IS_THROTTLE(index))
				{
					if ( ( v > (RESX/100 ) ) || ( v < -(RESX/100) ) )
					{
				    Output.anas[index] = 0; //set values for mixer
					}
		      trimA[index] = 0;
 				}
//   				    	Output.anas[i+PPM_BASE] = 0 ;
   		}
    }

  }
	//    if throttle trim -> trim low end
  if(g_model.thrTrim)
	{
		int8_t ttrim ;
		ttrim = getTrimValue( CurrentPhase, 2 ) ;
		if(throttleReversed())
		{
			ttrim = -ttrim ;
		}
    trimA[2] = ((int32_t)ttrim+125)*(RESX-Output.anas[2])/(RESX) ;
	}
	if ( att & FADE_FIRST )
	{

    //===========BEEP CENTER================
    anaCenter &= g_model.beepANACenter;
    if(((bpanaCenter ^ anaCenter) & anaCenter)) audioDefevent(AU_POT_STICK_MIDDLE);
    bpanaCenter = anaCenter;

		int16_t *panas = Output.anas ;
		FORCE_INDIRECT(panas) ;

    panas[MIX_MAX-1]  = RESX;     // MAX
    panas[MIX_FULL-1] = RESX;     // FULL

//        for(uint8_t i=0;i<NUM_PPM;i++)
//				{
//					int16_t x ;
//					x = g_ppmIns[i] ;
//					if ( i < 4 ) x -= g_eeGeneral.trainer.calib[i] ;  //add ppm channels
//					panas[i+PPM_BASE] = x*2 ;
//				}

    //===========Swash Ring================
    if(g_model.swashRingValue)
    {
//			int8_t x ;
//			uint16_t y ;

//			x = panas[ele_stick] / 10 ;
//			y = x * x ;
//			x = panas[ail_stick] / 10 ;
//			y += x * x ;

//			uint8_t t1 = calc100toRESX(g_model.swashRingValue) / 10 ;
//			if ( y > (t1 * t1) )
//			{
//        uint16_t d = isqrt32((uint32_t) y);
//        panas[ele_stick] = (int32_t)panas[ele_stick]*t1/((int32_t)d) ;
//        panas[ail_stick] = (int32_t)panas[ail_stick]*t1/((int32_t)d) ;
				
//			}
      uint32_t v = ((int32_t)panas[ele_stick]*panas[ele_stick] + (int32_t)panas[ail_stick]*panas[ail_stick]);
		  int16_t tmp = calc100toRESX(g_model.swashRingValue) ;
      uint32_t q ;
      q = (int32_t)tmp * tmp ;
      if(v>q)
      {
        uint16_t d = isqrt32(v);
        panas[ele_stick] = (int32_t)panas[ele_stick]*tmp/((int32_t)d) ;
        panas[ail_stick] = (int32_t)panas[ail_stick]*tmp/((int32_t)d) ;
      }
    }

//#define REZ_SWASH_X(x)  ((x) - (x)/8 - (x)/128 - (x)/512)   //  1024*sin(60) ~= 886
// Correction sin(60) * 1024 = 886.8 so 887 is closer
#define REZ_SWASH_X(x)  ((x) - (x)/8 - (x)/128 - (x)/1024 )   //  1024*sin(60) ~= 887
#define REZ_SWASH_Y(x)  ((x))   //  1024 => 1024

    if(g_model.swashType)
    {
        int16_t vp = 0 ;
        int16_t vr = 0 ;

        if( !(att & NO_INPUT) )  //zero input for setStickCenter()
				{
          vp += panas[ele_stick] + trimA[ele_stick] ;
          vr += panas[ail_stick] + trimA[ail_stick] ;
				}
            
        int16_t vc = 0;
//						int16_t *panas = Output.anas ;
//						FORCE_INDIRECT(panas) ;

        if(g_model.swashCollectiveSource)
				{
					uint8_t k = g_model.swashCollectiveSource - 1 ;
					if ( k < CHOUT_BASE )
					{
            vc = panas[k] ;
					}
					else
					{
						vc = Ex_chans[k-CHOUT_BASE] ;
					}
				}

        if(g_model.swashInvertELE) vp = -vp;
        if(g_model.swashInvertAIL) vr = -vr;
        if(g_model.swashInvertCOL) vc = -vc;

				int16_t c1,c2,c3 ;
        switch (( uint8_t)g_model.swashType)
        {
        default:
        case (SWASH_TYPE_120):
            vp = REZ_SWASH_Y(vp);
            vr = REZ_SWASH_X(vr);
            c1 = vc - vp;
            c2 = vc + vp/2 + vr;
            c3 = vc + vp/2 - vr;
            break;
        case (SWASH_TYPE_120X):
            vp = REZ_SWASH_X(vp);
            vr = REZ_SWASH_Y(vr);
            c1 = vc - vr;
            c2 = vc + vr/2 + vp;
            c3 = vc + vr/2 - vp;
            break;
        case (SWASH_TYPE_140):
            vp = REZ_SWASH_Y(vp);
            vr = REZ_SWASH_Y(vr);
            c1 = vc - vp;
            c2 = vc + vp + vr;
            c3 = vc + vp - vr;
            break;
        case (SWASH_TYPE_90):
            vp = REZ_SWASH_Y(vp);
            vr = REZ_SWASH_Y(vr);
            c1 = vc - vp;
            c2 = vc + vr;
            c3 = vc - vr;
            break;
//                break;
        }
				panas[MIX_CYC1-1] = c1 ;
				panas[MIX_CYC2-1] = c2 ;
				panas[MIX_CYC3-1] = c3 ;

    }

//  		  if(tick10ms)
//				{
//					inactivityCheck() ;
//    			timer() ;
//					timerBeeps() ;
//					trace(); //trace thr 0..32  (/32)
//				}
	}
//    memset(chans,0,sizeof(chans));        // All outputs to 0


#ifdef MIX_WARN
  uint8_t mixWarning = 0;
#endif
    //========== MIXER LOOP ===============

  uint8_t mixIndex = 0 ;
  MixData *md = &g_model.mixData[0];
	for(uint8_t chanIndex=0;chanIndex<NUM_CHNOUT;chanIndex++)
	{
    int32_t thisChan = 0 ;
		for( ; md->destCh-1 == chanIndex ; md += 1,	mixIndex += 1 ) // So continue increments
		{
#ifdef MIX_SWITCH_BITFIELD
			uint8_t swBitIndex ;
			uint8_t swBitField ;
			swBitIndex = mixIndex >> 3 ;
			swBitField = XBITMASK( (mixIndex & 7) ) ;
#endif			
				int8_t mixweight = REG100_100( md->weight) ;

//        if((md->destCh==0) || (md->destCh>NUM_CHNOUT)) break;

        //Notice 0 = NC switch means not used -> always on line
        int16_t v  = 0;
        uint8_t swTog;
#ifdef MIX_SWITCH_BITFIELD
        uint8_t swon = Output.swOnBits[swBitIndex] & swBitField ;
#else
        uint8_t swon = Output.swOn[mixIndex] ;
#endif
				bool t_switch = getSwitch(md->swtch,1) ;
        if (md->swtch && (md->srcRaw > PPM_BASE) && (md->srcRaw <= PPM_BASE+NUM_PPM)	&& (ppmInAvailable == 0) )
				{
					// then treat switch as false ???				
					t_switch = 0 ;
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
				//swOn[mixIndex]=false;
        if(!t_switch)
				{ // switch on?  if no switch selected => on
            swTog = swon ;
#ifdef MIX_SWITCH_BITFIELD
        		swon = false ;	// In case we do a "continue" just below
						Output.swOnBits[swBitIndex] &= ~swBitField ;
#else
        		Output.swOn[mixIndex] = swon = false ;	// In case we do a "continue" just below
#endif
            if (k == MIX_3POS+MAX_GVARS+1)	// "THIS"
						{
							int32_t temp = thisChan * ((int32_t)DEL_MULT * 256 / 100 ) ;
							Output.act[mixIndex] =  temp >> 8 ;
						}
            if(k!=MIX_MAX && k!=MIX_FULL) continue;// if not MAX or FULL - next loop
            if(md->mltpx==MLTPX_REP) continue; // if switch is off and REPLACE then off
            v = (k == MIX_FULL ? -RESX : 0); // switch is off and it is either MAX=0 or FULL=-512
        }
        else
				{
            swTog = !swon ;
            swon = true;
            k -= 1 ;

//            v = Output.anas[k]; //Switch is on. MAX=FULL=512 or value.
						if ( k < PPM_BASE )
						{
            	v = Output.anas[k]; //Switch is on. MAX=FULL=512 or value.
							if ( k < 4 )
							{
								if ( md->disableExpoDr )
								{
      		      	v = Output.rawSticks[k]; //Switch is on. MAX=FULL=512 or value.
								}
							}
						}
						else if ( k < CHOUT_BASE )
						{
							uint8_t i = k - PPM_BASE ;
							int16_t x ;
							x = g_ppmIns[i] ;
							if ( i < 4 ) x -= g_eeGeneral.trainer.calib[i] ;  //add ppm channels
							v = x * 2 ;
						}
						else if(k<CHOUT_BASE+NUM_CHNOUT)
						{
							if ( md->disableExpoDr )
							{
								v = g_chans512[k-CHOUT_BASE] ;
							}
							else
							{
//            		if(k<CHOUT_BASE+md->destCh-1)
//								{
//									v = chans[k-CHOUT_BASE] / 100 ; // if we've already calculated the value - take it instead // anas[i+CHOUT_BASE] = chans[i]
//								}
//								else
								{
									v = Ex_chans[k-CHOUT_BASE] ;
								}
							}
						}
						else if ( k == MIX_3POS-1 )
						{
#ifdef XSW_MOD
              uint8_t sw = md->sw23pos ;
              if (sw == MAXSW23POS) // sw must be SW_Trainer
                sw += 2;            // skip SW_PB1 and SW_PB2
              sw += SW_IDL;
              uint8_t sst = switchState(sw);
              v = (sst == ST_UP ? -1024 : (sst == ST_DN ? 1024 : 0));   // v = 0 if ST_MID
#else // !XSW_MOD
#ifdef SWITCH_MAPPING
							uint8_t sw = md->sw23pos ;
//							uint8_t threePosition = 0 ;
//							if ( sw < 7 )
//							{
//								threePosition =  ;
//							}
							
//							if ( XBITMASK( sw ) & Sw3posMask )
//							{
//								sw = pgm_read_byte( &Sw3posIndex[sw] ) ;
//        				v = hwKeyState( sw ) ? -1024 : (hwKeyState( sw+1 ) ? 0 : 1024) ;
//							}
//							else
//							{
//								sw = indexSwitch( sw ) + 1 ;
//	   						v = hwKeyState(sw) ? 1024 : -1024 ;
//							}

							
							
//							if ( XBITMASK( sw ) & Sw3posMask )
//							{
//								sw = pgm_read_byte( &Sw3posIndex[sw] ) ;
//								sw = switchPosition(sw) ;
//							}
//							else
//							{
//								sw = indexSwitch( sw ) + 1 ;
//								sw = switchPosition(sw) ? 0 : 2 ;
////								if ( sw )
////								{
////									sw = 2 ;
////								}
//							}
							sw = currentSwitchPosition012( sw ) ;
        			v = ( sw == 0 ) ? -1024 : ( (sw == 1 ) ? 0 : 1024) ;
							 
//							if ( sw )
//							{
//								sw += SW_ThrCt - KEY_MENU - 1 ;
//								if ( sw >= SW_ID0 )
//								{
//									sw += SW_AileDR - SW_ID0 ;
//								}
//								if ( sw == SW_ElevDR )
//								{
//									if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
//									{
//										sw = HSW_Ele3pos0 ;
//									}
//								}
//								if ( sw == SW_AileDR )
//								{
//									if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
//									{
//										sw = HSW_Ail3pos0 ;
//									}
//								}
//							}
//							else
//							{
//								sw = HSW_ID0 ;
//							}
//							if ( threePosition )
//							{
//        				v = hwKeyState( sw ) ? -1024 : (hwKeyState( sw+1 ) ? 0 : 1024) ;
//							}
//							else
//							{
//	   						v = keyState((EnumKeys)sw) ? 1024 : -1024 ;
//							}
#else							
							uint8_t sw = md->sw23pos ;
							if ( sw )
							{
								sw += SW_ThrCt - KEY_MENU - 1 ;
								if ( sw >= SW_ID0 )
								{
									sw += SW_AileDR - SW_ID0 ;
								}
        				v = keyState((EnumKeys)sw) ? 1024 : -1024 ;
							}
							else
							{
        				v = keyState(SW_ID0) ? -1024 : (keyState(SW_ID1) ? 0 : 1024) ;
							}
#endif
#endif  // XSW_MOD
						}
						else if ( k < MIX_3POS+MAX_GVARS )
						{
			        v = g_model.gvars[k-MIX_3POS].gvar * 8 ;
						}
						else if ( k == MIX_3POS+MAX_GVARS )	// "THIS"
						{
							v = thisChan / 100 ;
						}
            else
						{
							v = calc_scaler( k - (MIX_3POS+MAX_GVARS+1), 0, 0 ) ;
						}

//            else if (k > MIX_3POS+MAX_GVARS + NUM_SCALERS)
//						{
//							// ( k >= EXTRA_POTS_START )
//							// An extra pot
//							v = calibratedStick[k-EXTRA_POTS_START+8] ;
//						}
//            else
//						{
//							v = calc_scaler( k - (MIX_3POS+MAX_GVARS+1), 0, 0 ) ;
//						}


#ifdef MIX_WARN
						if(md->mixWarn) mixWarning |= 1<<(md->mixWarn-1); // Mix warning
#endif
        }
        
#ifdef MIX_SWITCH_BITFIELD
				if ( swon )
				{
					Output.swOnBits[swBitIndex] |= swBitField ;
				}
				else
				{
					Output.swOnBits[swBitIndex] &= ~swBitField ;
				}
#else
				Output.swOn[mixIndex] = swon ;
#endif

        //========== INPUT OFFSET ===============
#ifndef V2
        if ( md->lateOffset == 0 )
        {
            if(md->sOffset) v += calc100toRESX( REG( md->sOffset, -125, 125 )	) ;
        }
#endif
        //========== DELAY and PAUSE ===============
        if (md->speedUp || md->speedDown || md->delayUp || md->delayDown)  // there are delay values
        {
#ifdef V2
					uint8_t timing = md->hiResSlow ? 20 : 100 ;
#else
					uint8_t timing = g_model.mixTime ? 20 : 100 ;
#endif
					int16_t my_delay = Output.sDelay[mixIndex] ;
					int32_t tact = Output.act[mixIndex] ;
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
//                    tact = (int32_t)Output.anas[md->destCh-1+CHOUT_BASE]*DEL_MULT * 100 ;
                    tact = (int32_t)Ex_chans[md->destCh-1]*DEL_MULT * 100 ;
                    if(mixweight) tact /= mixweight ;
                }
#if DEL_MULT == 256
								diff = v-(tact>>8) ;
#else
            		diff = v-tact/DEL_MULT;
#endif
                if(diff) my_delay = (diff<0 ? md->delayUp :  md->delayDown) * timing ;
            }

            if(my_delay > 0)
						{ // perform delay
                if(tick10ms)
                {
                  my_delay-- ;
                }
                if (my_delay != 0)
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

					Output.sDelay[mixIndex] = my_delay ;

            if(diff && (md->speedUp || md->speedDown))
						{
                //rate = steps/sec => 32*1024/100*md->speedUp/Down
                //act[i] += diff>0 ? (32768)/((int16_t)100*md->speedUp) : -(32768)/((int16_t)100*md->speedDown);
                //-100..100 => 32768 ->  100*83886/256 = 32768,   For MAX we divide by 2 sincde it's asymmetrical
                if(tick10ms)
								{
                    int32_t rate = (int32_t)DEL_MULT * 2048 * 100 ;
                    if(mixweight)
										{
											uint8_t mweight = mixweight ;
											if ( mixweight < 0 )
											{
												mweight = -mixweight ;
											}
											rate /= mweight ;
										}

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
										tact = (speed) ? tact+(rate)/((int16_t)timing*speed) : (int32_t)v*DEL_MULT ;

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
					Output.act[mixIndex] = tact ;
        
				}

        //========== CURVES ===============
				if ( md->differential )
				{
      		//========== DIFFERENTIAL =========
      		int16_t curveParam = REG100_100( md->curve ) ;
      		if (curveParam > 0 && v < 0)
      		  v = ((int32_t)v * (100 - curveParam)) / 100;
      		else if (curveParam < 0 && v > 0)
      		  v = ((int32_t)v * (100 + curveParam)) / 100;
				}
				else
				{
					if ( ( md->curve <= -28 ) || ( md->curve > 27 ) )
					{
						// do expo using md->curve + 128
						int8_t x = md->curve + 128 ;
      			v = expo( v, x ) ;
					}
					else
					{
      	  	switch(md->curve)
						{
      	  	case 0:
      	  	    break;
      	  	case 1:
//      	  	    if(md->srcRaw == MIX_FULL) //FUL
//      	  	    {
//      	  	        if( v<0 ) v=-RESX;   //x|x>0
//      	  	        else      v=-RESX+2*v;
//      	  	    }else{
      	  	        if( v<0 ) v=0;   //x|x>0
//      	  	    }
      	  	    break;
      	  	case 2:
//      	  	    if(md->srcRaw == MIX_FULL) //FUL
//      	  	    {
//      	  	        if( v>0 ) v=RESX;   //x|x<0
//      	  	        else      v=RESX+2*v;
//      	  	    }else{
      	  	        if( v>0 ) v=0;   //x|x<0
//      	  	    }
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
        int32_t dv = (int32_t)v*mixweight ;
        //========== lateOffset ===============
#ifdef V2
        if(md->sOffset) dv += calc100toRESX( REG( md->sOffset, -125, 125 )	) * 100L ;
#else
				if ( md->lateOffset )
        {
            if(md->sOffset) dv += calc100toRESX( REG( md->sOffset, -125, 125 )	) * 100L ;
        }
#endif
//				int32_t *ptr ;			// Save calculating address several times
//				ptr = &chans[md->destCh-1] ;
        switch((uint8_t)md->mltpx)
				{
        	case MLTPX_REP:
            thisChan = dv;
      	  break;
    	    case MLTPX_MUL:
						dv /= 100 ;
						thisChan *= dv ;
            thisChan /= RESXl;
  	      break;
	        default:  // MLTPX_ADD
						thisChan += dv ;
          break;
        }
// Possible overflow test, may not be worthwhile
//				int8_t test ;
//				test = dv >> 24 ;
//				if ( ( test != -1) && ( test != 0 ) )
//				{
//					dv >>= 8 ;					
//				}
//        *ptr = dv;
    }


		ThrottleStickyOn = 0 ;

        int32_t q = thisChan;// + (int32_t)g_model.limitData[i].offset*100; // offset before limit

				if ( Fade.fadePhases )
				{
					int32_t l_fade = Fade.fade[chanIndex] ;
					if ( att & FADE_FIRST )
					{
						l_fade = 0 ;
					}
					l_fade += ( q / 100 ) * Fade.fadeScale[CurrentPhase] ;
					Fade.fade[chanIndex] = l_fade ;
			
					if ( ( att & FADE_LAST ) == 0 )
					{
						continue ;
					}
//          if ( fadeWeight != 0)
					l_fade /= Fade.fadeWeight ;
					q = l_fade * 100 ;
				}
    	  thisChan = q / 100 ; // chans back to -1024..1024
        
				Ex_chans[chanIndex] = thisChan; //for getswitch
        
//				LimitData *limit = limitaddress( chanIndex ) ;
		    LimitData *limit = &g_model.limitData[chanIndex] ;
				FORCE_INDIRECT(limit) ;
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

        if(q)
				{
					int16_t temp = (q<0) ? ((int16_t)ofs-lim_n) : ((int16_t)lim_p-ofs) ;
          q = ( q * temp ) / 100000 ; //div by 100000 -> output = -1024..1024
				}
        
				int16_t result ;
				result = calc1000toRESX(ofs);
  			result += q ; // we convert value to a 16bit value
				
        lim_p = calc1000toRESX(lim_p);
        if(result>lim_p) result = lim_p;
        lim_n = calc1000toRESX(lim_n);
        if(result<lim_n) result = lim_n;

        if(limit->reverse) result = -result ;// finally do the reverse.


#ifdef V2
				{
					V2SafetySwData *psw = &g_model.safetySw[chanIndex] ;
					FORCE_INDIRECT(psw) ;

        	if(psw->swtch)  //if safety sw available for channel check and replace val if needed
					{
						static uint8_t sticky = 0 ;
						uint8_t applySafety = 0 ;
						int8_t sSwitch = psw->swtch ;
								
						if(getSwitch00( sSwitch))
						{
							applySafety = 1 ;
						}

						if ( psw->mode )
						{
							// Special case, sticky throttle
							if( applySafety )
							{
								sticky = 0 ;
							}
							else
							{
								uint8_t throttleOK = 0 ;
								if ( g_model.throttleIdle )
								{
									if ( abs( calibratedStick[2] ) < 20 )
									{
										throttleOK = 1 ;
									}
								}
								else
								{
  								if(calibratedStick[2] < -(RESX-20))
  								{
										throttleOK = 1 ;
  								}
								}
								if ( throttleOK )
								{
									if ( trainerThrottleValid )
									{
										if ( trainerThrottleValue < -(RESX-20) )
										{
											sticky = 1 ;
										}
									}	
									else
									{
										sticky = 1 ;
									}
								}
										
							}
							if ( sticky == 0 )
							{
								applySafety = 1 ;
							}
							ThrottleStickyOn = applySafety ;
						}
						if ( applySafety ) result = calc100toRESX(psw->val) ;
					}
				}
#else // V2
				{
					uint8_t numSafety = 16 - g_model.numVoice ;
					if ( chanIndex < numSafety )
					{
						SafetySwData *psw = &g_model.safetySw[chanIndex] ;
						FORCE_INDIRECT(psw) ;

        		if(psw->opt.ss.swtch)  //if safety sw available for channel check and replace val if needed
						{
#ifndef NOSAFETY_A_OR_V					
 #ifndef SAFETY_ONLY
							if ( ( psw->opt.ss.mode != 1 ) && ( psw->opt.ss.mode != 2 ) )	// And not used as an alarm
 #endif
#endif
							{
								static uint8_t sticky = 0 ;
								uint8_t applySafety = 0 ;
								int8_t sSwitch = psw->opt.ss.swtch ;
								
								if(getSwitch00( sSwitch))
								{
									applySafety = 1 ;
								}

#ifdef SAFETY_ONLY
								if ( psw->opt.ss.mode == 1 )
#else
								if ( psw->opt.ss.mode == 3 )
#endif
								{
									// Special case, sticky throttle
									if( applySafety )
									{
										sticky = 0 ;
									}
									else
									{
										uint8_t throttleOK = 0 ;
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
											if ( abs( calibratedStick[2] ) < 20 )
											{
												throttleOK = 1 ;
											}
										}
										else
										{
  										if(calibratedStick[2] < -1004)
  										{
												throttleOK = 1 ;
  										}
										}
										if ( throttleOK )
										{
											if ( trainerThrottleValid )
											{
												if ( trainerThrottleValue < -1004 )
												{
													sticky = 1 ;
												}
											}	
											else
											{
												sticky = 1 ;
											}
										}
										
									}
									if ( sticky == 0 )
									{
										applySafety = 1 ;
									}
									ThrottleStickyOn = applySafety ;
								}
								if ( applySafety ) result = calc100toRESX(g_model.safetySw[chanIndex].opt.ss.val) ;
							}
						}
					}
				}
#endif // V2
				cli();
        chanOut[chanIndex] = result ; //copy consistent word to int-level
        sei();
	}
    //========== MIXER WARNING ===============
		// Processed after all mixes processed.
    //1= 00,08
    //2= 24,32,40
    //3= 56,64,72,80
#ifdef MIX_WARN
  {
    uint8_t tmr10ms ;
    tmr10ms = g_blinkTmr10ms ;	// Only need low 8 bits

    if(mixWarning & 1) if(((tmr10ms)==  0)) audioDefevent(AU_MIX_WARNING_1);
    if(mixWarning & 2) if(((tmr10ms)== 64) || ((tmr10ms)== 72)) audioDefevent(AU_MIX_WARNING_2);
    if(mixWarning & 4) if(((tmr10ms)==128) || ((tmr10ms)==136) || ((tmr10ms)==144)) audioDefevent(AU_MIX_WARNING_3);        
  }
#endif
}


//#if defined(CPUM128) || defined(CPUM2561)
#ifndef NMEA
void multiOption( uint8_t x, uint8_t y, int8_t option, uint8_t attr, uint8_t protocol )
{
	uint8_t display = 1 ;
	switch ( protocol )
	{
		case M_DSM :
			if ( ( option >= 4 ) && ( option <= 12 ) )
			{
				lcd_putsAttIdx( x-4*FW, y, PSTR("\004 4ch 5ch 6ch 7ch 8ch 9ch10ch11ch12ch"), option-4, attr ) ;
				return ;
			}
		break ;

		case M_FRSKYX :
		case M_Frsky :
		case M_FrskyV :
		case M_SFHSS :
		case M_CORONA :
		case M_Hitec :
		case M_FrskyX2 :
		case M_Frsky_RX :
			lcd_puts_Pleft( y, PSTR("\013Freq.") ) ;
			display = 0 ;
		break ;
		
		case M_Devo :
			if ( ( option >= 0 ) && ( option <= 1 ) )
			{
				lcd_putsAttIdx( x-6*FW, y, PSTR("\006AutoIDFixdID"), option, attr ) ;
				return ;
			}
		break ;

		case M_AFHD2SA :
		{
			uint8_t xoption = option ;
			if ( xoption & 0x80 )
			{
				lcd_puts_Pleft( y, PSTR("\013T-") ) ;
				xoption &= 0x7F ;
				option = xoption ;
			}
			if ( xoption <= 70 )
			{
				lcd_puts_Pleft( y, PSTR("\015Rate") ) ;
				display = 0 ;
				option = xoption ;
				option *= 5 ;
				option += 50 ;
			}
		}
		break ;
		
		case M_Hubsan :
			lcd_puts_Pleft( y, PSTR("\013VTX   ") ) ;
			display = 0 ;
		break ;

		case M_BAYANG :
			if ( ( option >= 0 ) && ( option <= 1 ) )
			{
				lcd_puts_Pleft( y, PSTR("\013Telem.") ) ;
				lcd_putsAttIdx( 20*FW, y, PSTR("\001NY"), option, attr ) ;
				return ;
			}
		break ;
	}
	if ( display )
	{
		lcd_puts_Pleft( y, PSTR(STR_MULTI_OPTION) ) ;
	}
	lcd_outdezAtt( x, y, option, attr ) ;
}
//#endif
#endif


#define M_INDEX			0
#define M_DISPLAY		1
#define M_AUDIO			2
#define M_ALARMS		3
#define M_GENERAL		4
#define M_CONTROLS	5
#define M_CALIB			6
#define M_TRAINER		7
#define M_VERSION		8
#define M_DIAGKEYS	9
#define M_DIAGANA		10
#define M_HARDWARE	11

const prog_char APM Str_Display[] = STR_DISPLAY ;
const prog_char APM Str_Trainer[] = STR_TRAINER ;
const prog_char APM Str_Version[] = STR_VERSION ;
const prog_char APM Str_General[] = STR_GENERAL ;
const prog_char APM Str_Hardware[] = STR_HARDWARE ;
const prog_char APM Str_Alarms[] = STR_ALARMS ;
const prog_char APM Str_Controls[] = STR_CONTROLS ;
const prog_char APM Str_Calibration[] = STR_CALIBRATION ;
const prog_char APM Str_AudioHaptic[] = STR_AUDIOHAPTIC ;
const prog_char APM Str_DiagSwtch[] = STR_DIAGSWTCH ;
const prog_char APM Str_DiagAna[] = STR_DIAGANA ;
const prog_char APM Str_VoiceAla[] =  STR_VOICEALA ;

static uint8_t indexProcess( uint8_t event, uint8_t *pmstate, uint8_t extra )
{
	struct t_menuControl *mc = &MenuControl ;
	FORCE_INDIRECT(mc) ;
	
	if (event == EVT_ENTRY)
	{
//		MenuTimer = 2000 ;	// * 0.01 Seconds = 20 seconds
		*pmstate = MenuControl.SubmenuIndex - 1 ;
		mc->SubmenuIndex = 0 ;
//		SubMenuFromIndex = 0 ;
	}
	if (event == EVT_ENTRY_UP)
	{
		*pmstate = mc->SubmenuIndex - 1 ;
#if defined(CPUM128) || defined(CPUM2561)
		if ( mc->SubMenuCall )
		{
			*pmstate = mc->SubMenuCall & 0x1F ;
			g_posHorz = ( mc->SubMenuCall >> 5 ) & 3 ;
			mc->SubMenuCall = 0 ;
		}
		else
#endif
		{
			mc->SubmenuIndex = 0 ;
		}
	}
	
	if ( mc->UseLastSubmenuIndex )
	{
		mc->SubmenuIndex = mc->LastSubmenuIndex & 0x7F ;
		mc->UseLastSubmenuIndex = 0 ;
//		SubMenuFromIndex = 0 ;
	}
	
	if ( mc->SubmenuIndex )
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
//      s_editMode = false;
//			pmstate->m_posVert = SubmenuIndex ;
//			SubmenuIndex = 0 ;
//			killEvents(event) ;
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
				*pmstate = mc->SubmenuIndex - 1 ;
				mc->SubmenuIndex = 0 ;
				killEvents(event) ;
				audioDefevent(AU_MENUS) ;
	      s_editMode = false;
			}
			event = 0 ;
		}
	}
	else
	{
		uint8_t pv = *pmstate ;
		if (event == EVT_KEY_FIRST(KEY_RIGHT) )
		{
#ifndef NO_TEMPLATES
			if ( pv < ((extra == 8) ? 7 : extra) )
			{
				pv += (extra == 8) ? 8 : 7 ;
//				pv += 7 ;
			}
#else
			if ( pv < extra )
			{
				pv += 7 ;
			}
#endif
		}
		if (event == EVT_KEY_FIRST(KEY_LEFT) )
		{
#ifndef NO_TEMPLATES
			if ( pv >= ((extra == 8) ? 8 : 7) )
			{
				pv -= (extra == 8) ? 8 : 7 ;
			}
#else
			if ( pv >= 7)
			{
				pv -= 7 ;
			}
#endif
		}

 		if ( (event == EVT_KEY_FIRST(KEY_MENU) ) ||(event == EVT_KEY_BREAK(BTN_RE) ) )
		{
			mc->SubmenuIndex = pv + 1 ;
			mc->LastSubmenuIndex = mc->SubmenuIndex | 0x80 ;
			pv = 0 ;
			killEvents(event) ;
			g_posHorz = Tevent = event = 0 ;
		}
		*pmstate = pv ;
	}
	return event ;
}

static void displayIndex( const prog_char *const strings[], uint8_t extra, uint8_t lines, uint8_t highlight )
{
	uint8_t offset = 7 ;
#ifndef NO_TEMPLATES
	if ( extra == 8 )
	{
		offset = 8 ;
		lcd_puts_P( 69, 0, (const char *)pgm_read_adr( &strings[7] ) ) ;
	}
#endif	
	
	for ( uint8_t i = 0 ; i < lines ; i += 1 )
	{
		lcd_puts_Pleft((i+1)*FH, (const prog_char *)pgm_read_adr( &strings[i] ) ) ;
		if ( i < extra )
		{
			lcd_puts_P( 69, (i+1)*FH, (const prog_char *)pgm_read_adr( &strings[i+offset] ) ) ;
		}
	} 
	
	lcd_vline( 67, 0, 63 ) ;

	if ( highlight )
	{
		if ( highlight > 7 )
		{
			lcd_char_inverse( 69, (highlight-offset)*FH, 59, 0 ) ;
		}
		else
		{
			lcd_char_inverse( 0, highlight*FH, 66, 0 ) ;
		}
	}
	asm("") ;
}

#ifdef XSW_MOD
#if defined(SERIAL_VOICE) && !defined(REMOVE_FROM_64FRSKY)
// only shows qualified list element
static uint8_t checkIndexQualified( uint8_t y, const prog_char * s, uint8_t value, uint8_t edit, uint16_t qmask )
{
  uint8_t x = pgm_read_byte(s++) ;
  uint8_t max = pgm_read_byte(s++) ;
  // assert(max <= 15);
  // assert(qmask > 0);

  if (edit) {
    if ( ( EditColumns == 0 ) || ( s_editMode ) ) {
      uint8_t qval = 0, maxb = 0;
      // index of string list to qualified list index
      for (uint8_t i = 0; i <= max; i++) {
        if (i == value)
          qval = maxb;
        if ((qmask & (1 << i)) != 0) {
          maxb++;  // count mask bit
        }
      }
      // assert(maxb > 0);
      qval = checkIncDec( qval, 0, maxb, EditType ) ;   // incDec with qualified list
      // qualified list index to a string list index
      for (uint8_t i = 0; i <= max; i++) {
        if ((qmask & (1 << i)) != 0) {
          if (qval == 0) {
            value = i;
            break;
          }
          qval--;
        }
      }
    }
  }
  lcd_putsAttIdx( x, y, s, value, edit ? InverseBlink: 0 ) ;
  return value ;
}

static void selectSwitchSource(uint8_t xsw, uint8_t y, uint8_t edit, uint16_t qmask)
{
  uint8_t prev = getSwitchSource(xsw);
	uint8_t curr = checkIndexQualified( y, PSTR(FWx17"\016\004NONEPB7 PC0 PC4 PC6 PC7 PG2 PG5 EXT1XPD2XPD3XPD4XPB1XPC0EXT2"), prev, edit, qmask ) ; // max 7 ext switch sources
	if (curr != prev) {
    setSwitchSource(xsw, curr);
    initSwitchSrcPin(curr);
  }
}
#endif
#endif  // XSW_MOD

void menuProcIndex(uint8_t event)
{
//	static MState2 mstate;
	uint8_t sub ;
	EditType = EE_GENERAL ;

//	uint8_t saveEvent = event ;

//	event = indexProcess( event, &MenuVertStack[g_menuStackPtr], ( SystemOptions & SYS_OPT_HARDWARE_EDIT ) ? 4 : 3 ) ;
	event = indexProcess( event, &MenuVertStack[1], ( SystemOptions & SYS_OPT_HARDWARE_EDIT ) ? 4 : 3 ) ;
	sub = check_columns(event, IlinesCount-1 ) ;
//	Tevent = event = saveEvent ;
	
	uint8_t y = 1*FH ;
	uint8_t subN = 0 ;
	uint8_t blink = InverseBlink ;
	switch ( MenuControl.SubmenuIndex )
	{
		case M_INDEX :
  		TITLEP(Str_Radio_Setup);
			IlinesCount = ( SystemOptions & SYS_OPT_HARDWARE_EDIT ) ? 11 : 10 ;
			sub += 1 ;

static const prog_char *const n_Strings[11] PROGMEM = {
Str_Display,
Str_AudioHaptic,
Str_Alarms,
Str_General,
Str_Controls,
Str_Calibration,
Str_Trainer,
Str_Version,
Str_DiagSwtch,
Str_DiagAna,
Str_Hardware
} ;	

			displayIndex( n_Strings, IlinesCount - 7, 7, sub ) ;
		break ;
	
		case M_DISPLAY :
		{	
      TITLEP( Str_Display ) ;
#ifdef XSW_MOD
 #if defined(CPUM128) || defined(CPUM2561)
  #define ROTATE_SCREEN  1
 #else
  #define ROTATE_SCREEN  0
 #endif
 #ifndef MINIMISE_CODE    
			IlinesCount = 6 + ROTATE_SCREEN;
 #else
			IlinesCount = 5 ;
 #endif
#else
			IlinesCount = 6 ;
#endif
  		uint8_t attr ;
//			y = FH ;
			attr = (sub==subN) ? blink : 0 ;
			lcd_xlabel_decimal( PARAM_OFS+3*FW-2, y, g_eeGeneral.contrast, attr, PSTR(STR_CONTRAST) ) ;
      if ( attr )
			{
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, LCD_MINCONTRAST, LCD_MAXCONTRAST) ;
				lcdSetContrast() ;
			}
			y += FH ;
			subN += 1 ;

			attr = 0 ;
#ifndef MINIMISE_CODE    
      lcd_puts_Pleft( y,PSTR(STR_LIGHT_SWITCH"\037"STR_LIGHT_INVERT"\037"STR_LIGHT_AFTER"\037"STR_LIGHT_STICK"\037"STR_FLASH_ON_BEEP
#else
      lcd_puts_Pleft( y,PSTR(STR_LIGHT_SWITCH"\037"STR_LIGHT_INVERT"\037"STR_LIGHT_AFTER"\037"STR_LIGHT_STICK
#endif
#if ROTATE_SCREEN
        "\037"STR_ROTATE
#endif
      ));
      if(sub==subN) { attr = blink ; CHECK_INCDEC_GENERALSWITCH( g_eeGeneral.lightSw, -MaxSwitchIndex, MaxSwitchIndex );}
      putsDrSwitches(PARAM_OFS-FW,y,g_eeGeneral.lightSw,attr);
			y += FH ;
			subN += 1 ;

      g_eeGeneral.blightinv = onoffItem( g_eeGeneral.blightinv, y, subN ) ;
			y += FH ;
			subN += 1 ;

			for ( uint8_t i = 0 ; i < 2 ; i += 1 )
			{
				uint8_t b ;
				b = ( i == 0 ) ? g_eeGeneral.lightAutoOff : g_eeGeneral.lightOnStickMove ;

  			uint8_t attr = 0 ;
        if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR_0( b, 600/5);}
				putsOffDecimal( PARAM_OFS, y, b*5, attr ) ;
        if(b)
				{
         lcd_putc(PARAM_OFS+3*FW, y, 's');
        }
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
      
#ifndef MINIMISE_CODE    
			g_eeGeneral.flashBeep = onoffItem( g_eeGeneral.flashBeep, y, subN ) ;
#endif
#if ROTATE_SCREEN
#ifndef MINIMISE_CODE    
			y += FH ;
			subN += 1 ;
#endif

      uint8_t b = g_eeGeneral.rotateScreen;
      uint8_t c = onoffItem( b, y, subN ) ;
      if (b != c)
			{
        g_eeGeneral.rotateScreen = c;
        lcdSetOrientation();
      }
#endif
		}
		break ;
		
		case M_AUDIO :
		{	
			TITLEP( Str_AudioHaptic ) ;
#ifdef CPUM2561
 #ifdef GLOBAL_COUNTDOWN
			IlinesCount = 7 ;
 #else			
			IlinesCount = 6 ;
 #endif
#else
 #ifdef GLOBAL_COUNTDOWN
			IlinesCount = 6 ;
 #else			
			IlinesCount = 5 ;
 #endif
#endif
		
      uint8_t b ;
			uint8_t attr = LEFT ;
      lcd_puts_Pleft( y,PSTR(STR_VOLUME"\037"STR_BEEPER"\037"STR_SOUND_MODE"\037"STR_SPEAKER_PITCH));
      b = g_eeGeneral.volume+(NUM_VOL_LEVELS-1) ;
			if(sub==subN) { attr = blink | LEFT ; CHECK_INCDEC_H_GENVAR_0( b, NUM_VOL_LEVELS-1 ); }
      lcd_outdezAtt(PARAM_OFS, y, b, attr);
			g_eeGeneral.volume = (int8_t)b-(NUM_VOL_LEVELS-1) ;
  		y += FH ;
			subN += 1 ;

			g_eeGeneral.beeperVal = checkIndexedV( y, PSTR(FWx14"\006"STR_BEEP_MODES), g_eeGeneral.beeperVal, subN ) ;
			y += FH ;
			subN += 1 ;

//			attr = 0 ;
      b = g_eeGeneral.speakerMode ;
			if ( b > 3 )
			{
				b = 4 ;					
			}
			b = checkIndexedV( y, PSTR(FWx11"\004"STR_SPEAKER_OPTS), b, subN ) ;
			g_eeGeneral.speakerMode = ( b >= 4 ) ? 7 : b ;
			y += FH ;
			subN += 1 ;

  		
			attr = LEFT ;
      if(sub==subN)
			{
				attr = blink | LEFT ;
        CHECK_INCDEC_H_GENVAR( g_eeGeneral.speakerPitch, 1, 100);
      }
      lcd_outdezAtt(PARAM_OFS,y,g_eeGeneral.speakerPitch,attr);
  		y += FH ;
			subN += 1 ;

  		attr = LEFT ;
      if(sub==subN)
			{
				attr = blink | LEFT ;
        CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.hapticStrength, 5);
#ifdef XSW_MOD
        initHapticPin();
#endif
      }
			lcd_xlabel_decimal( PARAM_OFS, y, g_eeGeneral.hapticStrength, attr, PSTR(STR_HAPTICSTRENGTH) ) ;
  		y += FH ;
			subN += 1 ;
		 
#ifdef GLOBAL_COUNTDOWN
      g_eeGeneral.minuteBeep = onoffMenuItem( g_eeGeneral.minuteBeep, y, Str_minute_Beep, subN ) ;
  		y += FH ;
			subN += 1 ;
#endif
#ifdef CPUM2561
   		lcd_putsAtt(  0, y, PSTR("Global Voice Alerts"), sub==subN ? INVERS : 0 ) ;
      if( sub==subN )
			{
				if ( Tevent==EVT_KEY_BREAK(KEY_MENU))
				{
 #ifdef GLOBAL_COUNTDOWN
					MenuControl.SubMenuCall = 0x86 ;
#else
					MenuControl.SubMenuCall = 0x85 ;
#endif
					pushMenu(menuProcGlobalVoiceAlarm) ;
				}
			}
#endif

		}
		break ;

		case M_CONTROLS :
		{	
			TITLEP( Str_Controls ) ;
			IlinesCount = 10 ;
			
			if ( sub < 6 )
			{
				displayNext() ;
      	lcd_puts_Pleft( y,PSTR(STR_CROSSTRIM"\037"STR_THR_REVERSE"\037"STR_ENABLE_PPMSIM"\037"STR_CHAN_ORDER"\037"STR_MODE));
				g_eeGeneral.crosstrim = onoffItem( g_eeGeneral.crosstrim, y, subN) ;
  			y += FH ;
				subN += 1 ;

				uint8_t oldValue = g_eeGeneral.throttleReversed ;
      	g_eeGeneral.throttleReversed = onoffItem( oldValue, y, subN) ;
				if ( g_eeGeneral.throttleReversed != oldValue )
				{
  				checkTHR() ;
				}
  			y += FH ;
				subN += 1 ;

      	g_eeGeneral.enablePpmsim = onoffItem( g_eeGeneral.enablePpmsim, y, subN ) ;
  			y += FH ;
				subN += 1 ;

				uint8_t attr = sub==subN ? blink : 0 ;
//	    	lcd_puts_Pleft( y, PSTR(STR_CHAN_ORDER"\037"STR_MODE) ) ;//   RAET->AETR
				uint8_t bch = pgm_read_byte(bchout_ar + g_eeGeneral.templateSetup) ;
      	for ( uint8_t i = 4 ; i > 0 ; i -= 1 )
				{
					uint8_t letter ;
					letter = pgm_read_byte( Str_1_RETA+(bch & 3) + 1 ) ;
  				lcd_putcAtt( (14+i)*FW, y, letter, attr ) ;
					bch >>= 2 ;
				}
	    	if(attr) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.templateSetup, 23 ) ;
  			y += FH ;
				subN += 1 ;
		
      	for ( uint8_t i = 0 ; i < 4 ; i += 1 )
				{
					lcd_img((6+4*i)*FW, y, sticks,i ) ;
					if (g_eeGeneral.stickReverse & (1<<i)) lcd_char_inverse( (6+4*i)*FW, y, 3*FW, 0 ) ;
				}
      	if(sub==subN)
				{
  				attr = 0 ;
					if ( SystemOptions & SYS_OPT_HARDWARE_EDIT )
					{
						CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.stickReverse, 15 ) ;
						if ( BLINK_ON_PHASE )
						{
							attr = 1 ;
						}
					}
//				plotType = PLOT_BLACK ;
					if ( attr == 0 )
					{
						lcd_rect( 6*FW-1, y-1, 15*FW+2, 9 ) ;
					}
//				plotType = PLOT_XOR ;
				}
				y += FH ;
				subN += 1 ;

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
        lcd_putcAtt( 3*FW, y, '1'+mode,attr);
        for(uint8_t i=0; i<4; i++) putsChnRaw( (6+4*i)*FW, y, modeFixValue( i ), 0 ) ;//sub==3?INVERS:0);
				y += FH ;
				subN += 1 ;
			}
			else
			{
				subN = 6 ;
				if ( sub >= IlinesCount )
				{
					sub = MenuVertStack[g_menuVertPtr] = IlinesCount-1 ;
				}

				// Edit custom stick names
#if !(defined(CPUM128) || defined(CPUM2561))
				Columns = 3 ;
#endif
      	for(uint8_t i=0; i<4; i++)
				{
      		lcd_putsAttIdx( FW*5, y, modi12x3, i, 0 ) ;
#if defined(CPUM128) || defined(CPUM2561)
					if ( sub == subN )
					{
						MenuControl.SubMenuCall = 0x80 + i + 6 ;
					}
					alphaEditName( 11*FW, y, &g_eeGeneral.customStickNames[i*4], 4, sub==subN, (char *)&PSTR(STR_STICK_NAMES)[i*5] ) ;
#else
					editName( g_posHorz, y, &g_eeGeneral.customStickNames[i*4], 4, sub==subN ) ;
#endif
					subN += 1 ;
	 				y += FH ;
				}
				
			}

		} 
		break ;

		case M_ALARMS :
		{	
			TITLEP( Str_Alarms ) ;
#ifndef MINIMISE_CODE    
			IlinesCount = 6 ;
#else
			IlinesCount = 5 ;
#endif

  		uint8_t attr = LEFT ;
      lcd_puts_Pleft( y,PSTR(STR_BATT_WARN"\037"STR_INACT_ALARM"\037"STR_THR_WARNING"\037"STR_SWITCH_WARN"\037"STR_MEM_WARN));
      if(sub==subN) { attr = blink | LEFT ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatWarn, 40, 120); } //5-10V
      putsVolts(PARAM_OFS, y, g_eeGeneral.vBatWarn, attr);
  		y += FH ;
			subN += 1 ;

  		attr = 0 ;//LEFT ;
      if(sub==subN) { attr = blink ;CHECK_INCDEC_H_GENVAR( g_eeGeneral.inactivityTimer, -10, 110); } //0..120minutes
      lcd_outdezAtt(PARAM_OFS+2*FW-2, y, g_eeGeneral.inactivityTimer+10, attr);
  		y += FH ;
			subN += 1 ;

      uint8_t b = g_eeGeneral.disableThrottleWarning;
      g_eeGeneral.disableThrottleWarning = offonItem( b, y, subN ) ;
  		y += FH ;
			subN += 1 ;

      b = g_eeGeneral.disableSwitchWarning;
      g_eeGeneral.disableSwitchWarning = offonItem( b, y, subN ) ;
  		y += FH ;
			subN += 1 ;

      b = g_eeGeneral.disableMemoryWarning;
      g_eeGeneral.disableMemoryWarning = offonItem( b, y, subN ) ;
  		y += FH ;
			subN += 1 ;

#ifndef MINIMISE_CODE
      b = g_eeGeneral.disableAlarmWarning;
      g_eeGeneral.disableAlarmWarning = offonMenuItem( b, y, PSTR(STR_ALARM_WARN), subN ) ;
  		y += FH ;
			subN += 1 ;
#endif
					    
		} 
		break ;

		case M_GENERAL :
		{
			TITLEP( Str_General ) ;
#ifndef NOPOTSCROLL
#ifdef GLOBAL_COUNTDOWN
			IlinesCount = 7 ;
#else
			IlinesCount = 6 ;
#endif		 
#else
#ifdef GLOBAL_COUNTDOWN
			IlinesCount = 6 ;
#else
			IlinesCount = 5 ;
#endif		 
#endif		 
#if defined(CPUM128) || defined(CPUM2561)
			MenuControl.SubMenuCall = 0x80 ;
			alphaEditName( 11*FW-2, y, (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName), sub==subN, (char *)PSTR( "Owner Name") ) ;
#else
			if ( sub==0 )
			{
				Columns = 9 ;
			}
			editName( g_posHorz, y, (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName), (sub==subN) ) ;
#endif
  		y += FH ;
			subN += 1 ;

#ifdef GLOBAL_COUNTDOWN
      g_eeGeneral.preBeep = onoffMenuItem( g_eeGeneral.preBeep, y, Str_Beep_Countdown, subN ) ;
  		y += FH ;
			subN += 1 ;
#endif
		 
      uint8_t b = g_eeGeneral.disableSplashScreen;
      g_eeGeneral.disableSplashScreen = offonMenuItem( b, y, PSTR(STR_SPLASH_SCREEN"\037"STR_SPLASH_NAME), subN ) ;
  		y += FH ;
			subN += 1 ;

      b = g_eeGeneral.hideNameOnSplash;
      g_eeGeneral.hideNameOnSplash = offonItem( b, y, subN ) ;
  		y += FH ;
			subN += 1 ;


#ifndef NOPOTSCROLL
      b = g_eeGeneral.disablePotScroll ;
      g_eeGeneral.disablePotScroll = offonMenuItem( b, y, PSTR(STR_POTSCROLL), subN ) ;
  		y += FH ;
			subN += 1 ;
#endif
     
			g_eeGeneral.stickScroll = onoffMenuItem( g_eeGeneral.stickScroll, y, PSTR(STR_STICKSCROLL), subN ) ;
  		y += FH ;
			subN += 1 ;

			g_eeGeneral.multiTelInvert = onoffMenuItem( g_eeGeneral.multiTelInvert, y, PSTR("Invert Telemetry"), subN ) ;

		}			 
		break ;

		case M_VERSION :
		{	
      TITLEP( Str_Version ) ;

    	lcd_puts_Pleft( 2*FH,Stamps );
		}
		break ;

		case M_DIAGKEYS	:
		{
      TITLEP( PSTR(STR_DIAG) ) ;
	    uint8_t x=7*FW;
#ifdef XSW_MOD
      y = 0;
      // IDL=0 THR=1 RUD=2 ELE=3 AIL=4 GEA=5 PB1=6 PB2=7 TRN=8
  	  for (uint8_t i = 1; i <= MAX_PSWITCH; i++) {
        uint8_t sst = switchState(PSW_BASE + i - 1);
        if (sst == ST_NC)      // switch not connected - this must be either PB1 or PB2
          continue;
        if (i == DSW_PB1) {
          lcd_puts_Pleft(1*FH,PSTR("\017PB1") ) ;
          putc_0_1( FW*20, 1*FH, sst ) ;
        } else if (i == DSW_PB2) {
          lcd_puts_Pleft(2*FH,PSTR("\017PB2") ) ;
          putc_0_1( FW*20, 2*FH, sst ) ;
        } else {
          if (i != DSW_TRN)
            sst |= PUT_ARROW;  // put arrow for IDL/THR/RUD/ELE/AIL/GEA
          y += FH;
          putsDrSwitches(x, y, i, 0);
          putc_0_1( FW*11+2, y, sst ) ;
        }
      }
#else // !XSW_MOD
//extern uint8_t ExtraInputs ;
			
//			lcd_outhex4( 0, FH, ExtraInputs ) ;
//			lcd_outhex4( 25, 0, g_eeGeneral.switchMapping ) ;
//			lcd_outhex4( 25, FH, PORTB ) ;
				 
#ifdef SWITCH_MAPPING
  	  for(uint8_t i=0; i<7; i++)
#else
  	  for(uint8_t i=0; i<9; i++)
#endif
    	{
        uint8_t y=i*FH; //+FH;
#ifndef SWITCH_MAPPING
        if(i>(SW_ID0-SW_BASE_DIAG)) y-=FH; //overwrite ID0
#endif
#ifdef SWITCH_MAPPING
				
//				uint8_t threePosition = XBITMASK( i ) & Sw3posMask ;
				
				displayOneSwitch( x+FW, y, i ) ;

//				if ( threePosition )
//				{
//					continue ;
//				}
				 
//				if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
//				{
//					if ( i == HSW_ElevDR-1 )
//					{
//						displayOneSwitch( x+FW, y, i ) ;
//						continue ;
//					}
//				}
//				if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
//				{
//					if ( i == HSW_AileDR-1 )
//					{
//						displayOneSwitch( x+FW, y, i ) ;
//						continue ;
//					}
//				}
//				if ( g_eeGeneral.switchMapping & USE_RUD_3POS )
//				{
//					if ( i == HSW_RuddDR-1 )
//					{
//						displayOneSwitch( x+FW, y, i ) ;
//						continue ;
//					}
//				}
#else
        putsDrSwitches(x,y,i+1,0); //ohne ofF,on
        bool t=keyState((EnumKeys)(SW_BASE_DIAG+i));
				putc_0_1( x+FW*4+2, y , t ) ;
#endif
    	}
#endif  // XSW_MOD

	    x=0;
	    lcd_puts_Pleft(2*FH,PSTR(STR_KEYNAMES) ) ;
  	  for(uint8_t i=0; i<6; i++)
    	{
        uint8_t y=(5-i)*FH+2*FH;
        bool t=keyState((EnumKeys)(KEY_MENU+i));
        putc_0_1( x+FW*5+2,  y, t ) ;
    	}

	    x=14*FW;
  	  lcd_puts_P(x, 3*FH,PSTR(STR_TRIM_M_P) ) ;
	    for(uint8_t i=0; i<4; i++)
  	  {
        uint8_t y=i*FH+FH*4;
        lcd_img(    x,       y, sticks,i);
        bool tm=keyState((EnumKeys)(TRM_BASE+2*i));
        putc_0_1( x+FW*4, y, tm ) ;
        bool tp=keyState((EnumKeys)(TRM_BASE+2*i+1));
        putc_0_1( x+FW*6, y, tp ) ;
    	}

#ifdef SWITCH_MAPPING
			if ( g_eeGeneral.switchMapping & USE_PB1 )
			{
	    	lcd_puts_Pleft(1*FH,PSTR("\017PB1") ) ;
				putc_0_1( FW*20, 1*FH, getSwitch00(HSW_Pb1) ) ;
			}
			if ( g_eeGeneral.switchMapping & USE_PB2 )
			{
	    	lcd_puts_Pleft(2*FH,PSTR("\017PB2") ) ;
				putc_0_1( FW*20, 2*FH, getSwitch00(HSW_Pb2) ) ;
			}
#endif
		}			 
		break ;

		case M_DIAGANA	:
		{
      TITLEP( PSTR(STR_ANA) ) ;
			IlinesCount = 1 ;

			StickScrollAllowed = 0 ;
	    for(uint8_t i=0; i<8; i++)
  	  {
        uint8_t y=i*FH;
        lcd_putc( 4*FW, y, 'A' ) ;
        lcd_putc( 5*FW, y, '1'+i ) ;
        lcd_outhex4( 7*FW, y,anaIn(i));
				uint8_t index = i ;
				if ( i < 4 )
				{
 					index = modeFixValue( i ) - 1 ;
 				}
        if(i<7)  lcd_outdez(15*FW, y, (int16_t)calibratedStick[index]*25/256) ;
        else putsVBat(15*FW,y,blink|PREC1) ;
    	}
	    lcd_puts_Pleft(5*FH,PSTR("\022BG")) ;
  	  lcd_outdez(20*FW, 6*FH, BandGap );
#if defined(CPUM128) || defined(CPUM2561)
extern uint16_t VrefTest ;
			lcd_outdez(20*FW, 7*FH, VrefTest );
#endif
#ifndef NOPOTSCROLL
        scroll_disabled = 1;
#endif        
				if ( s_editMode )
				{
					CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatCalib, -127, 127);
				}
		}			 
		break ;

		case M_TRAINER :
		{
      TITLEP( Str_Trainer ) ;
			IlinesCount = 6 ;

			uint8_t subSub = g_posHorz ;

			if (SLAVE_MODE)
			{ // i am the slave
		    lcd_puts_Pleft( 3*FH, PSTR(STR_SLAVE));
			}
			else
			{
				
//#if defined(CPUM128) || defined(CPUM2561)
//extern uint16_t SingleTrainerPulseWidth ;
//				lcd_puts_Pleft( 0, PSTR("\009Singlepw"));
//   			lcd_outdez( FW*20, 0, SingleTrainerPulseWidth ) ;
//#endif
				y = 2*FH;
		    uint8_t attr;
				lcd_puts_Pleft( 1*FH, PSTR(STR_MODE_SRC_SW));

				for (uint8_t i=0; i<4; i++)
				{
#ifdef V2
    			volatile V2TrainerMix *td = &g_eeGeneral.trainer.mix[i];
#else
    			volatile TrainerMix *td = &g_eeGeneral.trainer.mix[i];
#endif
					int8_t x ;
			    putsChnRaw(0, y, i+1, 0);

			    for (uint8_t j=0; j<4; j++)
					{
						if ( sub==i )
						{
							Columns = 3 ;
						}
						uint8_t active = 0 ;
						attr = 0 ;
      			if (sub==i && subSub==j)
						{
							attr = blink ;
							active = 2 ;
						}

						switch(j)
						{
							case 0 :
								td->mode = checkIndexed( y, PSTR(FWx4"\002"STR_OFF_PLUS_EQ), td->mode, active ) ;
							break ;

							case 1 :
								x = td->studWeight ;
#ifdef V2
  		  				lcd_outdezAtt(11*FW, y, x, attr);
    						if (attr&BLINK) { CHECK_INCDEC_H_GENVAR( x, -100, 100); td->studWeight = x ; }
#else
  		  				lcd_outdezAtt(11*FW, y, x*13/4, attr);
    						if (attr&BLINK) { CHECK_INCDEC_H_GENVAR( x, -31, 31); td->studWeight = x ; } //!! bitfield
#endif
							break ;

							case 2 :
								x = td->srcChn ;
			    			lcd_putsAttIdx(12*FW, y, Str_Chans_Gv, x+16, attr);
    						if (attr&BLINK) { CHECK_INCDEC_H_GENVAR_0( x, 3); td->srcChn = x ; } //!! bitfield
							break ;
				
							case 3 :
								x = td->swtch ;
	  	  				putsDrSwitches(15*FW, y, x, attr);
#ifdef V2
    						if (attr&BLINK) { CHECK_INCDEC_GENERALSWITCH( x, -MaxSwitchIndex, MaxSwitchIndex); td->swtch = x ; }
#else
    						if (attr&BLINK) { CHECK_INCDEC_H_GENVAR( x, -15, 15); td->swtch = x ; }
#endif
							break ;
						}
					}
   				y += FH ;
				}

				attr = PREC1 ;
				if(sub==4) { attr = PREC1 | blink ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.PPM_Multiplier, -10, 40);}
				lcd_xlabel_decimal( 13*FW, 6*FH, g_eeGeneral.PPM_Multiplier+10, attr, PSTR(STR_MULTIPLIER) ) ;

				for (uint8_t i=0; i<4; i++)
				{
    			uint8_t x = (i*8+16)*FW/2;
    			lcd_outdezAtt(x , 7*FH, (g_ppmIns[i]-g_eeGeneral.trainer.calib[i])*2, PREC1);
				}
				attr = 0 ;
				if (sub==5)
				{
    			if (event==EVT_KEY_FIRST(KEY_MENU))
					{
		        s_editMode = false ;
        		memcpy(g_eeGeneral.trainer.calib, g_ppmIns, sizeof(g_eeGeneral.trainer.calib));
		        STORE_GENERALVARS;     //eeWriteGeneral();
        		audioDefevent(AU_MENUS);
			    }
					attr = INVERS ;
				}
				lcd_putsAtt(0*FW, 7*FH, PSTR(STR_CAL), attr ) ;
			}
		}			 
		break ;

		case M_HARDWARE :
		{	
			TITLEP( Str_Hardware ) ;

#ifdef XSW_MOD

#if defined(CPUM128) || defined(CPUM2561)
      uint8_t pb7 = 0;
      if (!parVoiceInstalled())
        pb7 = 1;
 #ifndef SERIAL_VOICE_ONLY
  #ifdef FRSKY
   #define HW_LINES   (5+pb7) // bandGap,TEZr90,FrSky,M'SoundSerial,LVTrimMod(,PB7backlight)
  #else
   #define HW_LINES   (4+pb7) // bandGap,TEZr90,M'SoundSerial,LVTrimMod(,PB7backlight)
  #endif
 #elif defined(FRSKY)
   #define HW_LINES   (4+pb7) // bandGap,TEZr90,FrSky,LVTrimMod(,PB7backlight)
 #else
  #define HW_LINES    (3+pb7) // bandGap,TEZr90,LVTrimMod(,PB7backlight)
 #endif
#else // M64
 #ifdef FRSKY
  #define HW_LINES    2       // bandGap,TEZr90
 #else
  #define HW_LINES    1       // bandGap
 #endif
#endif

#if defined(SERIAL_VOICE) && !defined(REMOVE_FROM_64FRSKY)
 #define HW_EXTRA_LINES 7     // 5 3-pos switches + 2 push buttons
#else
 #define HW_EXTRA_LINES 0
#endif

#if defined(CPUM128) || defined(CPUM2561)
	  IlinesCount = HW_LINES + HW_EXTRA_LINES + 4;
#else
	  IlinesCount = HW_LINES + HW_EXTRA_LINES ;
#endif

#if HW_EXTRA_LINES
		if (sub < (HW_LINES + 5)) // 3 page menus (5 3-pos switches)
    {
		  displayNext();
    }
    if (sub < HW_LINES) 
#endif
    {
 	    uint8_t b;
 	    b = g_eeGeneral.disableBG ;
 	    g_eeGeneral.disableBG = offonMenuItem( b, y, PSTR(STR_BANDGAP), subN ) ;
			
#ifdef FRSKY
      uint8_t f;
  		y += FH ;
			subN += 1 ;
			b = g_eeGeneral.TEZr90 ;
			f = onoffMenuItem( b, y, PSTR(STR_TEZ_R90), subN ) ;
			if ( b != f )
			{
			  g_eeGeneral.TEZr90 = f ;
				FRSKY_Init( FrskyTelemetryType ) ;
      }
#endif

#if defined(CPUM128) || defined(CPUM2561)
      uint8_t c;
  		y += FH ;
			subN += 1 ;
			b = g_eeGeneral.FrskyPins ;
			c = onoffMenuItem( b, y, PSTR(STR_FRSKY_MOD), subN ) ;
			if ( b != c )
			{
			  g_eeGeneral.FrskyPins = c ;
				if ( c )
				{
        	unsetSwitchSource(SSW_PC6);
        	unsetSwitchSource(SSW_PC7);
			    FRSKY_Init( 0 ) ;
				}
        else
        {
          FRSKY_disable() ;
        }
      }

 #ifndef SERIAL_VOICE_ONLY
  		y += FH ;
			subN += 1 ;
			b = g_eeGeneral.MegasoundSerial ;
			c = onoffMenuItem( b, y, PSTR("M'Sound Serial"), subN ) ;
			if ( b != c )
			{
			  g_eeGeneral.MegasoundSerial = c ;
        if ( c)
        {
          serialVoiceInit();
        }
        else
        {
          g_eeGeneral.pb7backlight = 0;
          UCSR1B = 0 ;  // disable Interrupt, TX, RX
        }
			}
 #endif

      y += FH ;
      subN += 1 ;
      b = g_eeGeneral.LVTrimMod;
      c = onoffMenuItem( b, y, PSTR("LV Trim Mod"), subN ) ;
      if (b != c) {
        g_eeGeneral.LVTrimMod = c;
        initLVTrimPin();
      }

      if (pb7) {
        y += FH ;
        subN += 1 ;
        b = g_eeGeneral.pb7backlight;
        c = onoffMenuItem( b, y, PSTR("PB7 Backlight"), subN ) ;
        if (b != c) {
          g_eeGeneral.pb7backlight = c;
          initBacklightPin();
        }
      }
#endif
    }
#if HW_EXTRA_LINES
		else if ( sub < HW_LINES + HW_EXTRA_LINES )
    {
     uint16_t qmask = getSwitchSourceMask();
      if (sub < (HW_LINES + 5)) {
        subN = HW_LINES;
        lcd_puts_Pleft( y, PSTR("3-Pos Switches\035THR\035RUD\035ELE\035AIL\035GEA") );
        for ( uint8_t i = 0 ; i < 5 ; i++ ) {
          y += FH ;
          selectSwitchSource(i, y, (sub==subN), qmask);
          subN += 1 ;
        }
      } else {
        subN = (HW_LINES + 5);
        lcd_puts_Pleft( y, PSTR("Push Buttons\035PB1\035PB2") );
        selectSwitchSource(5, y+FH, (sub==subN), qmask);
        selectSwitchSource(6, y+FH*2, (sub==(subN+1)), qmask);
			}
    }
		else
		{
			uint8_t subN = HW_LINES + HW_EXTRA_LINES ;

			uint8_t attr = 0 ;
			uint16_t db = (g_eeGeneral.stickDeadband & 0x00F0 ) >> 4 ;
  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR_0( db, 15 ) ; }
			lcd_xlabel_decimal( 20*FW, y, db, attr, PSTR("Stick LV deadband") ) ;
			g_eeGeneral.stickDeadband = ( g_eeGeneral.stickDeadband & 0xFF0F ) | ( db << 4 ) ;
			y += FH ;
 			subN++;

			attr = 0 ;
			db = g_eeGeneral.stickDeadband & 0x000F ;
  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR_0( db, 15 ) ; }
			lcd_xlabel_decimal( 20*FW, y, db, attr, PSTR("Stick LH deadband") ) ;
			g_eeGeneral.stickDeadband = ( g_eeGeneral.stickDeadband & 0xFFF0 ) | db ;
 			y += FH ;
 			subN++;

			attr = 0 ;
			db = (g_eeGeneral.stickDeadband & 0x0F00 ) >> 8 ;
  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR_0( db, 15 ) ; }
			lcd_xlabel_decimal( 20*FW, y, db, attr, PSTR("Stick RV deadband") ) ;
			g_eeGeneral.stickDeadband = ( g_eeGeneral.stickDeadband & 0xF0FF ) | ( db << 8 ) ;
 			y += FH ;
 			subN++;

			attr = 0 ;
			db = (g_eeGeneral.stickDeadband & 0xF000 ) >> 12 ;
  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR_0( db, 15 ) ; }
			lcd_xlabel_decimal( 20*FW, y, db, attr, PSTR("Stick RH deadband") ) ;
			g_eeGeneral.stickDeadband = ( g_eeGeneral.stickDeadband & 0x0FFF ) | ( db << 12 ) ;
 			y += FH ;
 			subN++;

		}
#endif

#else // !XSW_MOD

 #if defined(CPUM128) || defined(CPUM2561)
//			IlinesCount = 4 ;
#define HW_LINES		6
 #else
  #ifdef FRSKY
#define HW_LINES		2
  #else
   #ifndef SERIAL_VOICE_ONLY
#define HW_LINES		2
   #else
#define HW_LINES		1
   #endif
	#endif
 #endif
 #ifdef SERIAL_VOICE
  #ifdef SWITCH_MAPPING
   #ifndef REMOVE_FROM_64FRSKY
   #define HW_EXTRA_LINES		7
   #endif
  #endif
 #endif
 #ifndef HW_EXTRA_LINES
#define HW_EXTRA_LINES		0
 #endif

 #if defined(CPUM128) || defined(CPUM2561)
	IlinesCount = HW_LINES + HW_EXTRA_LINES + 4 ;
 #else
	IlinesCount = HW_LINES + HW_EXTRA_LINES ;
 #endif

 #if HW_EXTRA_LINES
		if ( sub < HW_LINES )
		{
			uint8_t subN = 0 ;
			displayNext() ;
 #endif

 	    uint8_t b ;
 	    b = g_eeGeneral.disableBG ;
 	    g_eeGeneral.disableBG = offonMenuItem( b, y, PSTR(STR_BANDGAP), subN ) ;
  		y += FH ;
			subN += 1 ;
			
 #ifdef FRSKY
			b = g_eeGeneral.TEZr90 ;
			uint8_t c ;
			c = onoffMenuItem( b, y, PSTR(STR_TEZ_R90), subN ) ;
			g_eeGeneral.TEZr90 = c ;
			if ( b != c )
			{
				FRSKY_Init( FrskyTelemetryType ) ;
			}
  		y += FH ;
			subN += 1 ;
 #endif			

 #if defined(CPUM128) || defined(CPUM2561)
			b = g_eeGeneral.FrskyPins ;
			c = onoffMenuItem( b, y, PSTR(STR_FRSKY_MOD), subN ) ;
			g_eeGeneral.FrskyPins = c ;
			if ( b != c )
			{
				if ( c )
				{
					FRSKY_Init( 0 ) ;
				}
				else
				{
					FRSKY_disable() ;
				}
			}
  		y += FH ;
			subN += 1 ;
 #endif

 #ifndef SERIAL_VOICE_ONLY
  #ifndef FRSKY			
			uint8_t c ;
  #endif
			b = g_eeGeneral.MegasoundSerial ;
			c = onoffMenuItem( b, y, PSTR("M'Sound Serial"), subN ) ;
			g_eeGeneral.MegasoundSerial = c ;
			if ( b != c )
			{
				if ( c )
				{
					serialVoiceInit() ;
				}
				else
				{
					UCSR1B = 0 ; // disable Interrupt, TX, RX
				}
			}
  		y += FH ;
			subN += 1 ;
 #endif

 #if defined(CPUM128) || defined(CPUM2561)
  		lcd_puts_Pleft( y, PSTR("RUD switch"));
			{
				uint8_t sm = g_eeGeneral.switchMapping ;
				uint8_t mask ;
				mask = USE_RUD_3POS ;
				g_eeGeneral.rud2source = checkIndexedV( y, PSTR(FWx17"\007\004NONEEXT1EXT2PC0 PG2 PB7 PG5 L-WR"), g_eeGeneral.rud2source, subN ) ;
				g_eeGeneral.switchMapping = sm & ~(mask) ;
				if ( g_eeGeneral.rud2source )
				{
					g_eeGeneral.switchMapping |= mask ;
				} 
				if ( sm != g_eeGeneral.switchMapping )
				{
					createSwitchMapping() ;
				}
			}
  		y += FH ;
			subN += 1 ;
  		lcd_puts_Pleft( y, PSTR("GEA switch"));
			{
				uint8_t sm = g_eeGeneral.switchMapping ;
				uint8_t mask ;
				mask = USE_GEA_3POS ;
				g_eeGeneral.gea2source = checkIndexedV( y, PSTR(FWx17"\007\004NONEEXT1EXT2PC0 PG2 PB7 PG5 L-WR"), g_eeGeneral.gea2source, subN ) ;
				g_eeGeneral.switchMapping = sm & ~(mask) ;
				if ( g_eeGeneral.gea2source )
				{
					g_eeGeneral.switchMapping |= mask ;
				} 
				if ( sm != g_eeGeneral.switchMapping )
				{
					createSwitchMapping() ;
				}
			}
  		y += FH ;
			subN += 1 ;
 #endif

 #if HW_EXTRA_LINES
		}
		else if ( sub < HW_LINES + HW_EXTRA_LINES )
		{
			uint8_t subN = HW_LINES ;
 #endif

#ifdef SERIAL_VOICE
#ifdef SWITCH_MAPPING
 #ifndef REMOVE_FROM_64FRSKY
  		lcd_puts_Pleft( y, PSTR("ELE switch\037AIL switch\037PB1 switch\037PB2 switch\037PG2 Input\037PB7 Input\037L-WR Input"));
			uint8_t i ;
			for ( i = 0 ; i < 4 ; i += 1 )
			{
				uint8_t sm = g_eeGeneral.switchMapping ;
				uint8_t mask ;
				uint8_t value ;
				switch ( i )
				{
					case 1 :
						mask = USE_AIL_3POS ;
						value = g_eeGeneral.ail2source ;
					break ;
					case 2 :
						mask = USE_PB1 ;
						value = g_eeGeneral.pb1source ;
					break ;
					case 3 :
						mask = USE_PB2 ;
						value = g_eeGeneral.pb2source ;
					break ;
					default :    
						mask = USE_ELE_3POS ;
						value = g_eeGeneral.ele2source ;
					break ;
				}
				
				value = checkIndexedV( y, PSTR(FWx17"\007\004NONEEXT1EXT2PC0 PG2 PB7 PG5 L-WR"), value, subN ) ;

				g_eeGeneral.switchMapping = sm & ~(mask) ;
				if ( value )
				{
					g_eeGeneral.switchMapping |= mask ;
				} 
				if ( sm != g_eeGeneral.switchMapping )
				{
					createSwitchMapping() ;
				}
				switch ( i )
				{
					case 1 :
						g_eeGeneral.ail2source = value ;
					break ;
					case 2 :
						g_eeGeneral.pb1source = value ;
					break ;
					case 3 :
						g_eeGeneral.pb2source = value ;
					break ;
					default :    
						g_eeGeneral.ele2source = value ;
					break ;
				}
				y += FH ;
				subN += 1 ;
			}

//  		uint8_t attr = (sub==subN) ? blink : 0 ;
			g_eeGeneral.pg2Input = onoffItem( g_eeGeneral.pg2Input, y, subN ) ;
			y += FH ;
			subN += 1 ;
  		
//			attr = (sub==subN) ? blink : 0 ;
			g_eeGeneral.pb7Input = onoffItem( g_eeGeneral.pb7Input, y, subN ) ;

			y += FH ;
			subN += 1 ;
  		
//			attr = (sub==subN) ? blink : 0 ;
			g_eeGeneral.lcd_wrInput = onoffItem( g_eeGeneral.lcd_wrInput, y, subN ) ;

  #endif
 #endif
 #endif
#if HW_EXTRA_LINES
		}
		else
		{
			uint8_t subN = HW_LINES + HW_EXTRA_LINES ;

			uint8_t attr = 0 ;
			uint8_t db = g_eeGeneral.stickDeadband & 0x000F ;
  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( db, 15 ) ; }
			lcd_xlabel_decimal( 20*FW, y, db, attr, PSTR("Stick LV deadband") ) ;
			g_eeGeneral.stickDeadband = ( g_eeGeneral.stickDeadband & 0xFFF0 ) | db ;
			y += FH ;
 			subN++;

			attr = 0 ;
			db = (g_eeGeneral.stickDeadband & 0x00F0 ) >> 4 ;
  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( db, 15 ) ; }
			lcd_xlabel_decimal( 20*FW, y, db, attr, PSTR("Stick LH deadband") ) ;
			g_eeGeneral.stickDeadband = ( g_eeGeneral.stickDeadband & 0xFF0F ) | ( db << 4 ) ;
 			y += FH ;
 			subN++;

			attr = 0 ;
			db = (g_eeGeneral.stickDeadband & 0x0F00 ) >> 8 ;
  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( db, 15 ) ; }
			lcd_xlabel_decimal( 20*FW, y, db, attr, PSTR("Stick RV deadband") ) ;
			g_eeGeneral.stickDeadband = ( g_eeGeneral.stickDeadband & 0xF0FF ) | ( db << 8 ) ;
 			y += FH ;
 			subN++;

			attr = 0 ;
			db = (g_eeGeneral.stickDeadband & 0xF000 ) >> 12 ;
  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( db, 15 ) ; }
			lcd_xlabel_decimal( 20*FW, y, db, attr, PSTR("Stick RH deadband") ) ;
			g_eeGeneral.stickDeadband = ( g_eeGeneral.stickDeadband & 0x0FFF ) | ( db << 12 ) ;
 			y += FH ;
 			subN++;

		}
#endif
#endif  // XSW_MOD
    }
		break ;

		case M_CALIB :
		{
      TITLEP( Str_Calibration ) ;
			IlinesCount = 1 ;

	    MenuVertStack[g_menuVertPtr] = 0; // make sure we don't scroll or move cursor here

	    for(uint8_t i=0; i<7; i++)
			{ //get low and high vals for sticks and trims
        int16_t vt = anaIn(i);
        Xmem.Cal_data.loVals[i] = min(vt,Xmem.Cal_data.loVals[i]);
        Xmem.Cal_data.hiVals[i] = max(vt,Xmem.Cal_data.hiVals[i]);
        if(i>=4) Xmem.Cal_data.midVals[i] = (Xmem.Cal_data.loVals[i] + Xmem.Cal_data.hiVals[i])/2;
  	  }

#ifndef NOPOTSCROLL
	    scroll_disabled = Xmem.Cal_data.idxState; // make sure we don't scroll while calibrating
#endif

	    switch(event)
  	  {
    		case EVT_ENTRY:
        	Xmem.Cal_data.idxState = 0;
        break;

		    case EVT_KEY_BREAK(KEY_MENU):
    	    Xmem.Cal_data.idxState++;
	        if(Xmem.Cal_data.idxState==3)
  	      {
            audioDefevent(AU_MENUS);
            STORE_GENERALVARS;     //eeWriteGeneral();
            Xmem.Cal_data.idxState = 0;
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

      	  for(uint8_t i=0; i<7; i++)
	        {
            Xmem.Cal_data.loVals[i] =  15000;
            Xmem.Cal_data.hiVals[i] = -15000;
            Xmem.Cal_data.midVals[i] = anaIn(i);
  	      }
    	    lcd_puts_Pleft( 2*FH, PSTR(STR_SET_MIDPOINT) ) ;//, 12, sub>0 ? INVERS : 0);
      	  lcd_puts_P(3*FW, 3*FH, menuWhenDone ) ;//, 16, sub>0 ? BLINK : 0);
        break;

		    case 2:
        //MOVE STICKS/POTS
        //[MENU]
					StickScrollAllowed = 0 ;

      	  for(uint8_t i=0; i<7; i++)
					{
            if(abs(Xmem.Cal_data.loVals[i]-Xmem.Cal_data.hiVals[i])>50)
						{
              g_eeGeneral.calibMid[i]  = Xmem.Cal_data.midVals[i];
              int16_t v = Xmem.Cal_data.midVals[i] - Xmem.Cal_data.loVals[i];
              g_eeGeneral.calibSpanNeg[i] = v - v/64;
              v = Xmem.Cal_data.hiVals[i] - Xmem.Cal_data.midVals[i];
              g_eeGeneral.calibSpanPos[i] = v - v/64;
            }
					}
        	g_eeGeneral.chkSum = evalChkSum() ;
	        lcd_puts_Pleft( 2*FH, PSTR(STR_MOVE_STICKS) ) ; //, 16, sub>0 ? INVERS : 0);
  	      lcd_puts_P(3*FW, 3*FH, menuWhenDone ) ; //, 16, sub>0 ? BLINK : 0);
        break;
	    }

    	doMainScreenGrphics();
		}
		break ;

	}
  asm("") ;		// Forces execution to here, prevents compiler filling in 'pops'
}

void displayVoiceRate( uint8_t x, uint8_t y, uint8_t rate, uint8_t attr )
{
	if ( ( rate >= 4 ) && ( rate <=32 ) )
	{
    lcd_outdezAtt( x+3*FW, y, rate-2, attr ) ;
	}
	else
	{
		if ( rate > 32 )
		{
			rate -= 29 ;
		}
		lcd_putsAttIdx( x, y, Str_On_Off_Both, rate, attr ) ;
	}
}

// FUnctions need to include ON, OFF and BOTH possibly
void menuProcVoiceOne(uint8_t event)
{
	TITLEP(PSTR("Voice Alarm"));
//	static MState2 mstate2;
	VoiceAlarmData *pvad ;
	
#ifdef CPUM2561
	uint8_t index = s_currIdx & 0x7F ;
	if ( s_currIdx & 0x80 )
	{
		pvad = &g_eeGeneral.extraGeneral.vad[index] ;
		lcd_putc( 12*FW, 0, 'G' ) ;
	}
	else
	{
		pvad = &g_model.vad[index] ;
	}
	lcd_outdez( 14*FW, 0, index+1 ) ;
#else
	pvad = &g_model.vad[s_currIdx] ;
	lcd_outdez( 14*FW, 0, s_currIdx+1 ) ;
#endif

	uint8_t rows = pvad->fnameType ? 11-1: 10-1 ;
	int8_t sub ;
	sub = check_columns(event, rows ) ;
	

	if ( sub < 6 )
	{
		lcd_puts_Pleft( FH, PSTR("Source""\037""Function""\037""Value""\037""Switch""\037""Rate""\037""Haptic") ) ;
	}
	else
	{
		lcd_puts_Pleft( FH, PSTR("Play Source""\037""On no Telemetry""\037""FileType") ) ;
	}

	uint8_t y = FH ;
	uint8_t attr = 0 ;
	uint8_t blink = InverseBlink ;
	
	if ( sub < 6 )
	{
		uint8_t subN = 0 ;
		displayNext() ;

		if ( sub == subN )
		{
			attr = blink ;
			CHECK_INCDEC_H_MODELVAR_0( pvad->source, NUM_XCHNRAW+NUM_TELEM_ITEMS ) ;
		}
		putsChnRaw( 16*FW, y, pvad->source, attr ) ;
		if ( pvad->source )
		{
			int16_t value ;
			value = getValue( pvad->source - 1 ) ;
  	  lcd_puts_Pleft( FH, PSTR("\007(\015)") ) ;
      if (pvad->source > CHOUT_BASE+NUM_CHNOUT)
 			{
				putsTelemetryChannel( 12*FW, FH, pvad->source-CHOUT_BASE-NUM_CHNOUT-1, value, 0, /*TELEM_NOTIME_UNIT |*/ TELEM_UNIT ) ;
			}
			else
			{
				lcd_outdez( 12*FW, FH, value ) ;
			}
		}
		y += FH ;
		subN += 1 ;
		
		attr = 0 ;
		if ( sub == subN )
		{
			attr = blink ;
      CHECK_INCDEC_H_MODELVAR_0( pvad->func, 6 ) ;
		}	
		if ( pvad->func < 5 )
		{
			lcd_putsAttIdx( 13*FW, y, Str_Cswitch, pvad->func, attr ) ;	// v1>v2  v1<v2  
		}
		else
		{
			lcd_putsAttIdx( 13*FW, y, PSTR("\007v\140=val v=val  "), pvad->func-5, attr ) ;	// v1>v2  v1<v2  
		}
		y += FH ;
		subN += 1 ;

		attr = 0 ;
		if ( sub == subN )
		{
			attr = blink ;
			StepSize = 100 ;
			pvad->offset = checkIncDec16( pvad->offset, -32000, 32000, EE_MODEL ) ;
		}
		if (pvad->source > CHOUT_BASE+NUM_CHNOUT)
		{
			putsTelemetryChannel( 20*FW, y, pvad->source-CHOUT_BASE-NUM_CHNOUT-1, pvad->offset, attr, /*TELEM_NOTIME_UNIT |*/ TELEM_UNIT | TELEM_CONSTANT ) ;
		}
		else
		{
			lcd_outdezAtt( FW*20, y, pvad->offset, attr ) ;
		}
		y += FH ;
		subN += 1 ;

		attr = 0 ;
		if ( sub == subN )
		{
			attr = blink ;
      CHECK_INCDEC_MODELSWITCH( pvad->swtch, -MaxSwitchIndex, MaxSwitchIndex ) ;
    }
   	putsDrSwitches(16*FW, y, pvad->swtch, attr);
		y += FH ;
		subN += 1 ;

		attr = 0 ;
		if ( sub == subN )
		{
			attr = blink ;
      CHECK_INCDEC_H_MODELVAR_0( pvad->rate, 33 ) ;
		}	
		displayVoiceRate( 16*FW, y, pvad->rate, attr ) ;
//		if ( pvad->rate < 3 )
//		{
//			lcd_putsAttIdx( 16*FW, y, Str_On_Off_Both,pvad->rate,attr);
//		}
//		else
//		{
//      lcd_outdezAtt(FW*18,y,pvad->rate-2,attr ) ;
//		}
		y += FH ;
		subN += 1 ;

		pvad->haptic = checkIndexedV( y, PSTR(FWx13"\003""\007-------Haptic1Haptic2Haptic3"), pvad->haptic, subN ) ;
	}
	else
	{
		uint8_t subN = 6 ;
		pvad->vsource = checkIndexedV( y, StrNoBeforeAfter, pvad->vsource, subN ) ;
		y += FH ;
		subN += 1 ;
	    
		pvad->mute = checkIndexedV( y, Str_PlayMute, pvad->mute, subN ) ;
		y += FH ;
		subN += 1 ;

		uint8_t previous = pvad->fnameType ;
		pvad->fnameType = checkIndexedV( y, Str_NUmberAudio, pvad->fnameType, subN ) ;
		if ( pvad->fnameType != previous )
		{
			pvad->vfile = 0 ;
		}
		y += FH ;
		subN += 1 ;

		if ( pvad->fnameType )
		{	
			attr = 0 ;
  	  lcd_puts_Pleft( y, PSTR("Voice File") ) ;
			if ( pvad->fnameType == 1 )	// Number
			{
				if ( sub == subN )
				{
					attr = blink ;
		  	  pvad->vfile = checkIncDec16( pvad->vfile, 0, 511, EE_MODEL);
				}
				if ( pvad->vfile > 507 )
				{
					lcd_putsAtt( 18*FW, y, Str_SC, attr ) ;
					lcd_putcAtt( 20*FW, y, pvad->vfile - 507 + '0', attr ) ;
				}
				else if ( pvad->vfile > 500)
				{
					dispGvar( 18*FW, y, pvad->vfile - 500, attr ) ;
				}
      	else
				{
					lcd_outdezAtt(FW*20,y,pvad->vfile,attr ) ;
					if (attr)
					{
						if (event == EVT_KEY_LONG(KEY_MENU) )
						{
							putVoiceQueueLong( pvad->vfile ) ;
						}
	  				s_editMode = 0 ;
					}
				}
			}
			else if ( pvad->fnameType == 2 )	// Audio
			{
				if ( sub == subN )
				{
					attr = blink ;
					uint8_t b ;
					b = pvad->vfile ;
					CHECK_INCDEC_H_MODELVAR_0( b, 15 ) ;
					pvad->vfile = b ;
				}
				lcd_putsAttIdx(15*FW, y, Str_Sounds, pvad->vfile, attr ) ;
			}
			y += FH ;
			subN += 1 ;
		}
			 
 		lcd_puts_Pleft( y, PSTR("Delete") ) ;
		attr = 0 ;
		if ( sub == subN )
		{
			attr = blink ;
			if (event == EVT_KEY_LONG(KEY_MENU) )
			{
    		memset(pvad,0,sizeof(VoiceAlarmData));
				s_editMode = 0 ;
				if ( sub == 10 )
				{
					MenuVertStack[g_menuVertPtr] = 8 ;
				}
			}
		}
		lcd_putsAtt( 12*FW, y, PSTR("MENU LONG"), attr ) ;
	}
	 
}

#ifdef CPUM2561
void menuVoice(uint8_t event, uint8_t mode)
#else
void menuProcVoiceAlarm(uint8_t event)
#endif
{
	TITLE(STR_VOICE) ;
//	static MState2 mstate2 ;
  int8_t sub ;
#ifdef V2
	sub = check_columns(event, NUM_VOICE_ALARMS+EXTRA_VOICE_SW-1 ) ;
#else
 #ifdef CPUM2561
	sub = check_columns(event, (mode ? NUM_GLOBAL_VOICE_ALARMS : NUM_VOICE_ALARMS)-1 ) ;
 #else
	sub = check_columns(event, NUM_VOICE_ALARMS-1 ) ;
 #endif
#endif


  uint8_t t_pgOfs = evalOffset(sub) ;

  switch (event)
	{
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE) :
			if ( sub < NUM_VOICE_ALARMS )
			{
#ifdef CPUM2561
      	s_currIdx = mode ? (sub | 0x80 ) : sub ;
#else
      	s_currIdx = sub ;
#endif
      	killEvents(event);
      	pushMenu(menuProcVoiceOne) ;
			}
		break;
  }

  uint8_t y = 1*FH ;
  for (uint8_t i = 0 ; i < 7 ; i += 1 )
	{
    uint8_t k = i + t_pgOfs ;
    uint8_t attr = sub == k ? INVERS : 0 ;
#ifdef V2
		if ( k < NUM_VOICE_ALARMS )
#endif
		{
			VoiceAlarmData *pvad = &g_model.vad[k] ;
      
			if(y>7*FH) break ;

#ifdef CPUM2561
			if ( mode )
			{
				lcd_xlabel_decimal( FW*4-1, y, k+1, 0, PSTR("GVA") ) ;
				pvad = &g_eeGeneral.extraGeneral.vad[k] ;
			}
			else
			{
#endif

	#if (NUM_VOICE_ALARMS<10)
			lcd_xlabel_decimal( FW*3-1, y, k+1, 0, PSTR("VA") ) ;
	#else
			lcd_xlabel_decimal( (k<9) ? FW*3-1 : FW*4-2, y, k+1, 0, PSTR("VA") ) ;
	#endif
#ifdef CPUM2561
			}
#endif
			putsChnRaw( 5*FW, y, pvad->source, 0 ) ;
			putsDrSwitches( 9*FW, y, pvad->swtch, 0 ) ;
			displayVoiceRate( 13*FW, y, pvad->rate, 0 ) ;
	//		if ( pvad->rate < 3 )
	//		{
	//			lcd_putsAttIdx( 13*FW, y, Str_On_Off_Both,pvad->rate,0 ) ;
	//		}
	//		else
	//		{
	//      lcd_outdez(FW*16,y,pvad->rate-2 ) ;
	//		}

    	uint8_t type = pvad->fnameType ;
			switch ( type )
			{
				case 1 :
					lcd_putc( 19*FW, y, '#' ) ;
				break ;
				case 2 :
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
		
		}
#ifdef V2
		else
		{
			VoiceSwData *pvd ;
			pvd = &g_model.voiceSw[k-NUM_VOICE_ALARMS] ;
			Columns = 2 ;
			uint8_t subSub = g_posHorz ;
 	 		for(uint8_t j=0; j<3;j++)
			{
	    	uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
				uint8_t active = attr ? 1 : 0 ;	// (attr && s_editing) ;

    		if (j == 0)
				{
//				  lcd_puts_Pleft( y, PSTR(STR_VS) ) ;
//					lcd_2_digits( 22, y, k+1-NUM_VOICE_ALARMS, 0 ) ;
					lcd_xlabel_decimal( FW*3-1, y, k+1-NUM_VOICE_ALARMS, 0, PSTR(STR_VS) ) ;
    			if(active)
					{
    			  CHECK_INCDEC_MODELSWITCH( pvd->swtch, -(MaxSwitchIndex-1), MaxSwitchIndex-1 ) ;
    			}
  			  putsDrSwitches(5*FW, y, pvd->swtch, attr);
				}
				else if (j == 1)
    		{
					pvd->mode = checkIndexed( y, PSTR(FWx10"\004"STR_VOICE_V2OPT), pvd->mode, active ) ;
				}
				else
				{
    			if(active)
					{
//						pvd->val = checkIncDec16(pvd->val, 0, 249, EE_MODEL);
						pvd->val = checkIncDec_u0( pvd->val, 249 ) ;
	        }
					lcd_outdezAtt(  20*FW, y, pvd->val, attr) ;
				}
			}
		}
#endif
		y += FH ;
  }
}

#ifdef CPUM2561
void menuProcVoiceAlarm(uint8_t event)
{
	EditType = EE_MODEL ;
	menuVoice( event, 0 ) ;
}

void menuProcGlobalVoiceAlarm(uint8_t event)
{
	EditType = EE_GENERAL ;
	menuVoice( event, 1 ) ;
}
#endif

enum ModelIndices
{
	M_MINDEX,
	M_MIXER,
	M_HELI,
	M_LIMITS,
	M_EXPO,
	M_MODES,
	M_CURVES,
	M_SWITCHES,
#ifndef NO_TEMPLATES
	M_TEMPLATES,
#endif
	M_SAFETY,
	M_GLOBALS,
#ifdef FRSKY
	M_TELEMETRY,
#endif
	M_VOICE,
	M_TIMERS,
	M_MGENERAL,
	M_PROTOCOL
} ;

#define NUM_M_OPTIONS	M_PROTOCOL

#ifndef NO_TEMPLATES
 #ifdef FRSKY
 #define MODEL_ITEMS	15
 #define MODEL_EXTRA	8
 #else
 #define MODEL_ITEMS	14
 #define MODEL_EXTRA	7
 #endif
#else
 #ifdef FRSKY
 #define MODEL_ITEMS	14
 #define MODEL_EXTRA	7
 #else
 #define MODEL_ITEMS	13
 #define MODEL_EXTRA	6
 #endif
#endif

const prog_char APM Str_Cswitches[] = STR_CSWITCHES ;
// STR_EXPO_DR
const prog_char APM Str_Voice[] = STR_VOICE ;


void rangeBindAction( uint8_t y, uint8_t newFlag )
{
	lcd_char_inverse( 0, y, 5*FW, 0 ) ;
	if ( Tevent==EVT_KEY_LONG(KEY_MENU))
	{
#if defined(BIND_OPTIONS)
		if ( newFlag == PXX_BIND )
		{
			pushMenu( menuBindOptions ) ;
		}
		else
		{
	  	pxxFlag = newFlag ;		    	//send bind code or range check code
			pushMenu(menuRangeBind) ;
		}
#else
  	pxxFlag = newFlag ;		    	//send bind code or range check code
		pushMenu(menuRangeBind) ;
#endif
	}
	s_editMode = 0 ;
}

#ifdef SWITCH_MAPPING
void displayLetterArrow( uint8_t x, uint8_t y, uint8_t letter, uint8_t value )
{
	value &= 3 ;
	if ( value )
	{
		if ( value == 2 )
		{
			value = '\201' ;
		}
		else
		{
			value = '-' ;
		}
	}
	else
	{
		value = '\200' ;
	}
	lcd_putc( x, y, letter ) ;
	lcd_putc( x+FW, y, value  ) ;
}
#endif


#ifdef FAILSAFE  			
void menuSetFailsafe(uint8_t event)
{
//	static MState2 mstate2 ;
  int8_t sub ;
	sub = check_columns(event, 16-1+1+1+1 ) ;	
  lcd_puts_Pleft( 0, PSTR( "Set Failsafe" ) ) ;
	uint8_t y = 0;
	uint8_t k = 0;
	uint8_t t_pgOfs ;
	int32_t value ;
	t_pgOfs = evalOffset(sub);
	
	StickScrollAllowed = 0 ;		// Block while editing

	if ( event == EVT_ENTRY )
	{
		StatusTimer = 0 ;
	}
	if ( StatusTimer )
	{
		StatusTimer -= 1 ;
	}

//	switch(event)
//	{
//    case EVT_KEY_LONG(KEY_MENU):
//			FailsafeCounter = 5 ;		// Send failsafe values soon
//      killEvents(event);
//  	  s_editMode = 0 ;
//		break ;
//	}		
	for(uint8_t i=0; i<7; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    uint8_t attr = ((sub==k) ? InverseBlink : 0);
		uint8_t active = (attr && s_editMode ) ;

		if ( k == 0 )
		{
		  lcd_puts_Pleft( y, PSTR( "Mode" ) ) ;
			EditColumns = 1 ;
			g_model.failsafeMode = checkIndexed( y, PSTR(FWx9"\004""\007Not Set     Rx Custom   HoldNoPulse"), g_model.failsafeMode, attr ) ;
			EditColumns = 0 ;
		}
		else if ( k == 1 )
		{
			if ( attr )
			{
				if ( event==EVT_KEY_LONG(KEY_MENU))
				{
					FailsafeCounter = 6 ;		// Send failsafe values soon
  	  	  killEvents(event);
  			  s_editMode = 0 ;
					StatusTimer = 50 ;
				}
			}
		  lcd_putsAtt( 0, y, PSTR( "Send Now" ), StatusTimer ? 0 : attr ) ;
		}
		else if ( k == 2 )
//		{
//			uint8_t b ;
//			b = g_model.failsafeRepeat ;
//     	g_model.failsafeRepeat = offonMenuItem( b, y, PSTR("Repeat Send"), k ) ;
//			if ( b != g_model.failsafeRepeat )
//			{
//				if ( b == 0 )
//				{
//					FailsafeCounter = 6 ;		// Send failsafe values soon
//				}
//				else
//				{
//					FailsafeCounter = 0 ;		// Stop sending
//				}
//			}
//		}
//		else if ( k == 3 )
		{
			if ( attr )
			{
				if ( event==EVT_KEY_LONG(KEY_MENU))
				{
					uint32_t j ;
  	  	  killEvents(event);
  			  s_editMode = 0 ;
					for ( j = 0 ; j < 16 ; j += 1 )
					{
						value = g_chans512[j] * 25 / (RESX/4) ;
//						value = validatePlusMinus125( value ) ;
  					if(value > 125)
						{
							value = 125 ;
						}	
  					if(value < -125 )
						{
							value = -125 ;
						}	
#ifndef V2
						int8_t *p = j < 8 ? &g_model.Failsafe[j] : &g_model.XFailsafe[j-8] ;
						*p = value ;
#else
						g_model.Failsafe[j] = value ;
#endif
					}
					StatusTimer = 50 ;
		    	eeDirty(EE_MODEL) ;
				}
			}
		  lcd_putsAtt( 0, y, PSTR( "Use Actual" ), StatusTimer ? 0 : attr ) ;
		}
		else
		{
  	  putsChn(0,y,k-3,0);
			uint8_t index = k-4 ;
#ifndef V2
			int8_t *p = index < 8 ? &g_model.Failsafe[index] : &g_model.XFailsafe[index-8] ;
#else
			int8_t *p = &g_model.Failsafe[index] ;
#endif
			value = *p ;
			lcd_outdezAtt(  7*FW+3, y, value, attr ) ;
			lcd_putc(  7*FW+5, y, '%' ) ;
    	if(active)
			{
  	    *p = checkIncDec16( value, -125, 125, EE_MODEL ) ;
    	}
		}
	}
}
#endif


#ifdef MULTI_PROTOCOL
#define NUM_MULTI_PROTOCOLS	84
#define MULTI_STR "\006FlyskyHubsanFrskyDHisky V2x2  DSM   Devo  YD717 KN    SymaX SLT   CX10  CG023 BayangFrskyXESky  MT99xxMJXq  ShenqiFY326 SFHSS J6PRO FQ777 ASSAN FrskyVHONTAIOpnLrsAFHD2SQ2X2  WK2x01Q303  GW008 DM002 CABELLESK150H8_3D CORONACFlie Hitec WFLY  BUGS  BUGMINTraxasNC1701E01X  V911S GD00X V761  KF606 RedpinPotensZSX   FlyzonScanerFrskyRAFHDSRHoTT  FX816 BayanRPelkanTiger XK    XN_DMPFrskX2FrSkR9PROPELLR12  SkyartESKYV2DSM_RXJRC345Q90C  KyoshoRaLink------RealacOMP   M-LinkWFLY  E016H E010r5LOLI  E129  JOYSWYE016H "
#if (defined(CPUM128) || defined(CPUM2561)) || not defined(FRSKY)
const prog_char APM M_FLYSKY_STR[] = { 4, 6, 'F','l','y','s','k','y','V','9','x','9',' ',' ','V','6','x','6',' ',' ','V','9','1','2',' ',' ','C','X','2','0',' ',' ' } ;
const prog_char APM M_HUBSAN_STR[] = { 2, 4, 'H','1','0','7','H','3','0','1','H','5','0','1' } ;
const prog_char APM M_HISKY_STR[] = { 1, 5, 'H','i','s','k','y','H','K','3','1','0'} ;
#if defined(CPUM128) || defined(CPUM2561)
const prog_char APM M_DSM2_STR[] = { 4, 7, 'D','S','M','2','-','2','2','D','S','M','2','-','1','1','D','S','M','X','-','2','2','D','S','M','X','-','1','1','A','U','T','O',' ',' ',' ' } ;
#else
const prog_char APM M_DSM2_STR[] = { 3, 7, 'D','S','M','2','-','2','2','D','S','M','2','-','1','1','D','S','M','X','-','2','2','D','S','M','X','-','1','1' } ;
#endif
const prog_char APM M_YD717_STR[] = { 4, 7, 'Y','D','7','1','7',' ',' ','S','K','Y','W','L','K','R','S','Y','M','A','X','4',' ','X','I','N','X','U','N',' ','N','I','H','U','I',' ',' ' } ;
const prog_char APM M_KN_STR[] = { 1, 6, 'W','L','T','O','Y','S','F','E','I','L','U','N' } ;
const prog_char APM M_SYMAX_STR[] = { 1, 7, 'S','Y','M','A','X',' ',' ','S','Y','M','A','X','5','C' } ;
const prog_char APM M_SLT_STR[] = { 1, 5, 'S','L','T',' ',' ','V','I','S','T','A' } ;
const prog_char APM M_CX10_STR[] = { 6, 7, 'G','R','E','E','N',' ',' ','B','L','U','E',' ',' ',' ','D','M','0','0','7',' ',' ','-','-','-',' ',' ',' ',' ','J','3','0','1','5','_','1','J','3','0','1','5','_','2','M','K','3','3','0','4','1' } ;
const prog_char APM M_CG023_STR[] = { 2, 5, 'C','G','0','2','3','Y','D','8','2','9','H','8','_','3','D' } ;
const prog_char APM M_BAYANG_STR[] = { 1, 6, 'B','a','y','a','n','g','H','8','S','3','D',' ' } ;
const prog_char APM M_FRSKY_STR[] = { 3, 5, 'C','H','_','1','6','C','H','_','8',' ','E','U','_','1','6','E','U','_','8',' ' } ;
const prog_char APM M_MT99XX_STR[] = { 4, 5, 'M','T',' ',' ',' ','H','7',' ',' ',' ','Y','Z',' ',' ',' ','L','S',' ',' ',' ','F','Y','8','0','5' } ;
const prog_char APM M_MJXQ_STR[] = { 5, 5, 'W','L','H','0','8','X','6','0','0',' ','X','8','0','0',' ','H','2','6','D',' ','E','0','1','0',' ','H','2','6','W','H' } ;
const prog_char APM M_FY326_STR[] = { 1, 5, 'F','Y','3','2','6','F','Y','3','1','9' } ;
const prog_char APM M_HONTAI_STR[] = { 2, 6, 'H','O','N','T','A','I','J','J','R','C','X','1',' ',' ','X','5','C','1','F','Q','7','7','7','_' } ;
const prog_char APM M_AFHD2SA_STR[] = { 3, 7, 'P','W','M','I','B','U','S','P','P','M','I','B','U','S','P','W','M','S','B','U','S','P','P','M','S','B','U','S' } ;
const prog_char APM M_Q2X2_STR[] = { 2, 4, 'Q','2','2','2','Q','2','4','2','Q','2','8','2' } ;
const prog_char APM M_WK2x01_STR[] = { 5, 6, 'W','K','2','8','0','1','W','K','2','4','0','1','W','6','_','5','_','1','W','6','_','6','_','1','W','6','_','H','E','L','W','6','H','E','L','I' } ;
const prog_char APM M_Q303_STR[] = { 3, 6, 'Q','3','0','3',' ',' ','C','X','3','5',' ',' ','C','X','1','0','D',' ','C','X','1','0','W','D' } ;
const prog_char APM M_CABELL_STR[] = { 7, 6, 'C','A','B','_','V','3','C','_','T','E','L','E','M','-',' ',' ',' ',' ',' ','-',' ',' ',' ',' ',' ','-',' ',' ',' ',' ',' ','-',' ',' ',' ',' ',' ','F','_','S','A','F','E','U','N','B','I','N','D' } ;
const prog_char APM M_H8_3D_STR[] = { 3, 7, 'H','8','_','3','D',' ',' ','H','2','0','H',' ',' ',' ','H','2','0','M','i','n','i','H','3','0','M','i','n','i' } ;
const prog_char APM M_CORONA_STR[] = { 2, 6, 'C','O','R','_','V','1','C','O','R','_','V','2','F','D','_','V','3',' ' } ;
const prog_char APM M_HITEC_STR[] = {	2, 6, 'O','p','t','_','F','w','O','p','t','H','u','b','M','i','n','i','m','a' } ;
const prog_char APM M_WFLY1_STR[] = {	0, 5, 'W','F','R','0','x' } ;
const prog_char APM M_BUGSMINI_STR[] = { 1, 8, 'B','u','g','s','m','i','n','i','B','u','g','s','3','H',' ',' ' } ;
const prog_char APM M_E01X_STR[] = { 2, 5, 'E','0','1','2',' ','E','0','1','5',' ','E','0','1','6','H' } ;
const prog_char APM M_V911S_STR[] = { 1, 5, 'V','9','1','1','S','E','1','1','9',' ' } ;
const prog_char APM M_GD00X_STR[] = { 1, 5, 'G','D','_','V','1','G','D','_','V','2' } ;
const prog_char APM M_REDPINE_STR[] = { 1, 4, 'F','a','s','t','S','l','o','w' } ;
const prog_char APM M_POTENSIC_STR[] = { 0, 3, 'A','2','0' } ;
const prog_char APM M_ZSX_STR[] = { 0, 3, '2','8','0' } ;
const prog_char APM M_FLYZONE_STR[] = { 0, 6, 'F','Z','_','4','1','0' } ;
const prog_char APM M_FRSKYRX_STR[] = { 3, 7, 'M','u','l','t','i',' ',' ','C','l','o','n','e','T','X','E','r','a','s','e','T','X','C','P','P','M',' ',' ',' ' } ;
const prog_char APM M_AFHDS2A_RX_STR[] = { 3, 5, 'M','u','l','t','i','C','P','P','M',' ' } ;
const prog_char APM M_FX816_STR[] = { 0, 3, 'P','3','8' } ;
const prog_char APM M_XK_STR[] = { 1, 4, 'X','4','5','0','X','4','2','0' } ;
const prog_char APM M_XN_DUMP_STR[] = { 3, 4, '2','5','0','K','1','M',' ',' ','2','M',' ',' ','A','U','T','O' } ;
const prog_char APM M_FRSKYX2_STR[] = { 4, 6, 'C','H','_','1','6',' ','C','H','_','8',' ',' ','E','U','_','1','6',' ','E','U','_','8',' ',' ','C','l','o','n','e','d' } ;
const prog_char APM M_FRSKYR9_STR[] = { 3, 6, '9','1','5','M','H','z','8','6','8','M','H','z','9','1','5','_','8','c','8','6','8','_','8','c' } ;
const prog_char APM M_PROPEL_STR[] = { 0, 4, '7','4','_','Z' } ;
const prog_char APM M_LR12_STR[] = { 1, 8, 'L','R','1','2',' ',' ',' ',' ','L','R','1','2','_','6','c','h' } ;
const prog_char APM	M_STD_STR[] = { 0, 3, 'S','t','d' } ;
const prog_char APM	M_ESKYV2_STR[] = { 0, 6, '1','5','0','V','2','S' } ;
const prog_char APM	M_DSM_RX_STR[] = { 1, 5, 'M','u','l','t','i','C','P','P','M' } ;
const prog_char APM	M_JJRC345_STR[] = { 1, 8, 'J','J','R','C','3','4','5',' ','S','k','y','T','m','b','l','r' } ;
const prog_char APM	M_KYOSHO_STR[] = { 1, 4, 'F','H','S','S','H','y','p','e' } ;
const prog_char APM	M_RADIOLINK_STR[] = { 2, 7, 'S','u','r','f','a','c','e','A','i','r',' ',' ',' ',' ','D','u','m','b','o','R','C' } ;
const prog_char APM	M_REALACC_STR[] = { 0, 3, 'R','1','1' } ;
const prog_char APM	M_WFLY_STR[] = { 0, 5, 'R','F','2','0','x' } ;
const prog_char APM	M_E016H_STR[] = { 0, 7, 'E','0','1','6','H','v','2' } ;



#endif
#endif // MULTI_PROTOCOL

const prog_char APM StrNZ_country[] = { FW*10, 2, 3, 'A','m','e','J','a','p','E','u','r'};
#if defined(R9M_SUPPORT)
const prog_char APM StrNZ_xjtType[] = { FW*6, 3, 3, 'D','1','6','D','8',' ','L','R','P','R','9','M' };
#else
const prog_char APM StrNZ_xjtType[] = { FW*10, 2, 3, 'D','1','6','D','8',' ','L','R','P' };
#endif
const prog_char APM StrNZ_pxxchans[] = { FW*10, 1, 2, '1', '6', ' ', '8' } ;

void menuProcModelIndex(uint8_t event)
{
//	static MState2 mstate;
	EditType = EE_MODEL ;

#ifdef V2
#if defined(CPUM128) || defined(CPUM2561)
	if ( PopupData.PopupActive == 0 )
	{
#endif // 128/2561
#endif // V2
		event = indexProcess( event, &MenuVertStack[1], MODEL_EXTRA ) ;
		check_columns(event, IlinesCount-1 ) ;
#ifdef V2
#if defined(CPUM128) || defined(CPUM2561)
	}
#endif // 128/2561
#endif // V2
	
	switch ( MenuControl.SubmenuIndex )
	{
		case M_MIXER :
      pushMenu(menuProcMix) ;
		break ;
		case M_MODES :
      pushMenu(menuModelPhases) ;
		break ;
#ifndef NO_TEMPLATES
		case M_TEMPLATES :
      pushMenu(menuProcTemplates) ;
		break ;
#endif
		case M_CURVES :
      pushMenu(menuProcCurve) ;
		break ;
#ifdef FRSKY
		case M_TELEMETRY :
      pushMenu(menuProcTelemetry) ;
		break ;
#endif
		case M_GLOBALS :
#ifdef V2
#ifdef USE_ADJUSTERS
			if ( PopupData.PopupActive == 0 )
			{
				PopupData.PopupIdx = 0 ;
				PopupData.PopupActive = 1 ;
				MenuControl.SubmenuIndex = 0 ;
			}
#else
      pushMenu(menuProcGlobals) ;
#endif // 128/2561
#else
      pushMenu(menuProcGlobals) ;
#endif // V2
		break ;
		case M_VOICE :
      pushMenu(menuProcVoiceAlarm) ;
		break ;
	}
	
	uint8_t sub = MenuVertStack[1] ;
//	uint8_t sub = mstate.m_posVert ;
	uint8_t y = FH ;
	uint8_t subN = 0 ;
	uint8_t blink = InverseBlink ;

	switch ( MenuControl.SubmenuIndex )
	{
		case M_MINDEX :
  		TITLEP(Str_Model_Setup);
			IlinesCount = MODEL_ITEMS ;
			sub += 1 ;

static const prog_char *const n_Strings[] PROGMEM = {
Str_Mixer,
Str_heli_setup,
Str_limits,
Str_Expo,
Str_Modes,
Str_Curves,
Str_Cswitches,
#ifndef NO_TEMPLATES
Str_Templates,
#endif
Str_Safety,
Str_Globals,
#ifdef FRSKY
Str_Telemetry,
#endif
Str_Voice,
Str_Timer,
Str_General,
Str_Protocol
};
	
#ifdef V2
#if defined(CPUM128) || defined(CPUM2561)
			if ( PopupData.PopupActive )
			{
				sub = M_GLOBALS ;
			}
#endif // 128/2561
#endif // V2
			 
			displayIndex( n_Strings, MODEL_EXTRA, 7, sub ) ;

#ifdef V2
#ifdef USE_ADJUSTERS
			if ( PopupData.PopupActive )
			{
//				uint8_t popaction = doPopup( PSTR("GVARS\0GVadjusters\0Scalers"), 7, 13 ) ;
				uint8_t popaction = doPopup( PSTR("GVARS\0GVadjusters"), 3, 13 ) ;
  			if ( popaction == POPUP_SELECT )
				{
					uint8_t popidx = PopupData.PopupSel ;
					if ( popidx == 0 )	// gvars
					{
    	  		pushMenu(menuProcGlobals) ;
					}
					if ( popidx == 1 )	// adjusters
					{
    	  		pushMenu(menuProcAdjust) ;
					}
//					if ( popidx == 2 )	// scalers
//					{
//    	  		pushMenu(menuProcScalers) ;
//					}
					MenuControl.SubmenuIndex = M_GLOBALS ;
				}
  			if ( popaction == POPUP_EXIT )
				{
					MenuControl.SubmenuIndex = 0 ;
					MenuVertStack[1] = M_GLOBALS - 1 ;
				}
			}
#endif // 128/2561
#endif // V2
		break ;
		 
		case M_MGENERAL :
		{	
			TITLEP(Str_General);
#ifdef V2
			if ( sub < 15 )
#else
			if ( sub < 11 )
#endif
			{
				displayNext() ;
			}
#ifdef V2
			IlinesCount = 19 ;
#else
			IlinesCount = 16 ;
#endif
		  
			if ( sub < 6 )
			{
#if defined(CPUM128) || defined(CPUM2561)
				MenuControl.SubMenuCall = 0x80 ;
				alphaEditName( 11*FW-2, y, (uint8_t *)g_model.name, sizeof(g_model.name), sub==subN, (char *)PSTR( "Model Name") ) ;
#else
				if ( sub==subN )
				{
					Columns = sizeof(g_model.name)-1 ;
				}
				editName( g_posHorz, y, (uint8_t *)g_model.name, sizeof(g_model.name), sub==subN ) ;
#endif
  			y += FH ;
				subN += 1 ;

				uint8_t attr = 0 ;
  		  if(sub==subN)
				{
					if (event == EVT_KEY_FIRST(KEY_MENU) )
					{
						putVoiceQueueUpper( g_model.modelVoice ) ;
					}
					attr = blink ;
  		    CHECK_INCDEC_H_MODELVAR_0( g_model.modelVoice, 49 ) ;
				}
				lcd_xlabel_decimal( 15*FW-2, y, (int16_t)g_model.modelVoice+260, attr, PSTR(STR_VOICE_INDEX) ) ;
  			y += FH ;
				subN += 1 ;
				
				
      	lcd_puts_Pleft(    y, PSTR(STR_DEAFULT_SW_PAGE));
#ifdef XSW_MOD
        uint8_t x = 9 * FW;
        uint16_t wstate = g_model.switchWarningStates ;
      	for (uint8_t i = 0; i < MAX_PSW3POS; i++) {		// I-T-R-E-A-G-
	        lcd_putc( x, y, pgm_read_byte(&ITREAG[i]) ) ;
          x += FW;
	        lcd_putc( x, y, pgm_read_byte(&arrows[wstate & 3]) ) ;
          x += FW;
          wstate >>= 2;
        }
#else	// !XSW_MOD
      	for(uint8_t i=0, q=1;i<8;q<<=1,i++)
				{
#ifdef SWITCH_MAPPING
#if 0
					uint8_t smNormal = 1 ;
					if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
					{
						if ( ( i >= 2 ) && ( i <= 5 ) )
						{
							uint8_t value = (uint8_t)g_model.switchWarningStates ;
							if ( i == 2 )
							{
								value >>= 2 ;
								displayLetterArrow( (11+i)*FW, y, 'E', value ) ;
							}
							else if ( i == 4 )
							{
								value >>= 4 ;
								displayLetterArrow( (11+i)*FW, y, 'I', value ) ;
							}
							smNormal = 0 ;
						}
					}
					if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
					{
						if ( i == 6 )
						{
							uint8_t value = (uint8_t)g_model.exSwitchWarningStates ;
							displayLetterArrow( (11+i)*FW, y, 'A', value ) ;
							smNormal = 0 ;
						}
						if ( i == 7 )
						{
							i = 8 ;
						}
					}
					if ( smNormal )
					{
						lcd_putsnAtt((11+i)*FW, y, Str_TRE012AG+i,1,  (((uint8_t)g_model.switchWarningStates & q) ? INVERS : 0 ) );
					}

#else
				uint8_t x = 9 * FW ;
      	for( uint8_t i = 0 ; i < 6 ; i += 1 )
				{
	        lcd_putc( x, y, pgm_read_byte(&ITREAG[i]) ) ;
          x += FW ;
					uint8_t state ;
extern uint8_t getExpectedSwitchState( uint8_t i ) ;
					state = getExpectedSwitchState( i ) ;
	        lcd_putc( x, y, pgm_read_byte(&arrows[state]) ) ;
          x += FW ;
				}
#endif


#else
					lcd_putsnAtt((11+i)*FW, y, Str_TRE012AG+i,1,  (((uint8_t)g_model.switchWarningStates & q) ? INVERS : 0 ) );
#endif
				}
#endif  // XSW_MOD
      	if(sub==subN)
				{
#ifdef XSW_MOD
          lcd_rect( 9*FW-1, y-1, 12*FW+2, 9 ) ;
#else
          lcd_rect( 9*FW-1, y-1, 12*FW+2, 9 ) ;
//					lcd_rect( 11*FW-1, y-1, 9*FW+2, 9 ) ;
#endif
      	  if (event==EVT_KEY_FIRST(KEY_MENU) || event==EVT_KEY_BREAK(BTN_RE))
					{
      	    killEvents(event);
						uint16_t ws ;
	    	    ws = getCurrentSwitchStates() ;
	    	    g_model.switchWarningStates = ws ;
#ifdef SWITCH_MAPPING
	    	    g_model.exSwitchWarningStates = ws >> 8 ;
#endif
      	  	s_editMode = false ;
      	    STORE_MODELVARS ;
					}
				}
  			y += FH ;
				subN += 1 ;
		
#ifdef V2
				g_model.useCustomStickNames = checkIndexedV( y, PSTR(FWx16"\002\005   NoRadioModel"), g_model.useCustomStickNames, subN ) ;
#else
				g_model.useCustomStickNames = onoffItem( g_model.useCustomStickNames, y, subN) ;
#endif
  			y += FH ;
				subN += 1 ;

  			attr = PREC1 ;
  	  	if(sub==subN) { attr |= blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.sub_trim_limit, 100 ) ; }
			  lcd_outdezAtt( 20*FW, y, g_model.sub_trim_limit, attr ) ;
  			y += FH ;
				subN += 1 ;

				uint8_t tvol = g_model.anaVolume ;
#ifdef V2
				g_model.anaVolume = checkIndexedV( y, PSTR(FWx17"\012\003---P1 P2 P3 GV1GV2GV3GV4GV5GV6GV7"), tvol, subN ) ;
#else
				g_model.anaVolume = checkIndexedV( y, PSTR(FWx17"\007\003---P1 P2 P3 GV4GV5GV6GV7"), tvol, subN ) ;
#endif
			}
			else if ( sub < 11 )
			{
				subN = 6 ;
  	  	lcd_puts_Pleft(    y, PSTR(STR_VOL_PAGE));
  			
				g_model.extendedLimits = onoffItem( g_model.extendedLimits, y, subN) ;
    //---------------------------------                           /* ReSt V */
    //   If extended Limits are switched off, min and max limits must be limited to +- 100         
        if (!g_model.extendedLimits)
        {
          for( LimitData *ld = &g_model.limitData[0] ; ld < &g_model.limitData[NUM_CHNOUT] ; ld += 1 )
          {
            FORCE_INDIRECT(ld) ;
            if (ld->min < 0) ld->min = 0;
            if (ld->max > 0) ld->max = 0;
          }
        }                                 
    //-----------------------------------                           /* ReSt A */
  			y += FH ;
				subN += 1 ;

				uint8_t oldValue = g_model.throttleIdle ;
				g_model.throttleIdle = checkIndexedV( y, PSTR(FWx15"\001\006  End Centre"), oldValue, subN ) ;
				if ( g_model.throttleIdle != oldValue )
				{
  				checkTHR() ;
				}
				y += FH ;
				subN += 1 ;

				oldValue = g_model.throttleReversed ;
      	g_model.throttleReversed = onoffItem( oldValue, y, subN) ;
				if ( g_model.throttleReversed != oldValue )
				{
  				checkTHR() ;
				}
  			y += FH ;
				lcd_putc( 16*FW, y, throttleReversed() ? '\201' : '\200' ) ;
  			y += FH ;
				subN += 1 ;

				g_model.thrTrim = onoffItem( g_model.thrTrim, y, subN) ;
  			y += FH ;
				subN += 1 ;

				g_model.thrExpo = onoffItem( g_model.thrExpo, y, subN) ;

			}
#ifdef V2
			else if ( sub < 15 )
#else
			else
#endif
			{
				subN = 11 ;

	  	  lcd_puts_Pleft(    y, PSTR(STR_TRIM_PAGE));
				
				g_model.trimInc = checkIndexedV( y, PSTR(FWx14"\004"STR_TRIM_OPTIONS), g_model.trimInc, subN ) ;
  			y += FH ;
				subN += 1 ;

  		  uint8_t attr = 0 ;
  		  if(sub==subN) { attr = blink ; }
				g_model.trimSw = edit_dr_switch( 17*FW, y, g_model.trimSw, attr, attr ? EDIT_DR_SWITCH_EDIT : 0 ) ;
  			y += FH ;
				subN += 1 ;

#ifndef V2
				g_model.mixTime = onoffItem( g_model.mixTime, y, subN) ;
				y += FH ;
				subN += 1 ;
#endif
				g_model.traineron = onoffItem( g_model.traineron, y, subN) ;
  			y += FH ;
				subN += 1 ;
			
				uint8_t b = 1 ;
				for(uint8_t i=0;i<7;i++)
				{
					uint8_t z = g_model.beepANACenter ;
    			lcd_putcAtt( (9+i)*(FW+1), y, pgm_read_byte( &PSTR(STR_RETA123)[i]), ( z & b ) ? INVERS : 0 ) ;
					if ( sub==subN )
				  {
						Columns = 6 ;
						if ( g_posHorz == i )
						{
							lcd_rect( (9+i)*(FW+1)-1, y-1, FW+2, 9 ) ;
							if ( event==EVT_KEY_BREAK(KEY_MENU) || event==EVT_KEY_BREAK(BTN_RE) ) 
							{
								g_model.beepANACenter ^= b ;
      					eeDirty(EE_MODEL) ;
    						s_editMode = false ;
							}
						}
					}
					b <<= 1 ;
				}
			}
#ifdef V2
			else
			{
				subN = 15 ;

#if !(defined(CPUM128) || defined(CPUM2561))
				Columns = 3 ;
#endif
      	for(uint8_t i=0; i<4; i++)
				{
      		lcd_putsAttIdx( FW*5, y, modi12x3, i, 0 ) ;
#if defined(CPUM128) || defined(CPUM2561)
					if ( sub == subN )
					{
						MenuControl.SubMenuCall = 0x80 + i + 15 ;
					}
					alphaEditName( 11*FW, y, &g_model.customStickNames[i*4], 4, sub==subN, (char *)&PSTR(STR_STICK_NAMES)[i*5] ) ;
#else
					editName( g_posHorz, y, &g_model.customStickNames[i*4], 4, sub==subN ) ;
#endif
	 				y += FH ;
					subN += 1 ;
				}
				
			}
#endif // V2

		}
		break ;
		
		case M_TIMERS :
		{
			TITLEP(Str_Timer) ;
#ifdef GLOBAL_COUNTDOWN
			IlinesCount = 10 ;
#else
			IlinesCount = 14 ;
#endif
			 
			editTimer( sub ) ;
#ifndef V2
			TimerMode *ptConfig = &TimerConfig[0] ;
			FORCE_INDIRECT(ptConfig) ;
			g_model.tmrVal = ptConfig->tmrVal ;
			g_model.tmrMode = ptConfig->tmrModeA ;
			g_model.tmrModeB = ptConfig->tmrModeB ;
			g_model.tmrDir = ptConfig->tmrDir ;
			ptConfig += 1 ;
			g_model.tmr2Val = ptConfig->tmrVal ;
			g_model.tmr2Mode = ptConfig->tmrModeA ;
			g_model.tmr2ModeB = ptConfig->tmrModeB ;
			g_model.tmr2Dir = ptConfig->tmrDir ;
#endif // nV2
		}	
		break ;

		case M_HELI :
		{
			TITLEP(Str_heli_setup) ;
			IlinesCount = 6 ;

		  uint8_t b ;
		  uint8_t attr ;
			uint8_t blink = InverseBlink ;

			uint8_t subN = 0 ;
		  lcd_puts_Pleft(    y, PSTR(STR_HELI_TEXT));
			
		  b = g_model.swashType ;
			g_model.swashType = checkIndexedV( y, PSTR(FWx17"\004"SWASH_TYPE_STR), b, subN ) ;
			y += FH ;
			subN++;

			attr = 0 ;
		  if(sub==subN) {attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.swashCollectiveSource, NUM_XCHNRAW);}
		  putsChnRaw(17*FW, y, g_model.swashCollectiveSource, attr);
			y += FH ;
			subN++;

			attr = 0 ;
		  if(sub==subN) {attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.swashRingValue, 100);}
			putsOffDecimal( FW*14, y, g_model.swashRingValue, attr ) ;
			y += FH ;
			subN++;

			g_model.swashInvertELE = hyphinvMenuItem( g_model.swashInvertELE, y, sub==subN ) ;
			y += FH ;
			subN++;

			g_model.swashInvertAIL = hyphinvMenuItem( g_model.swashInvertAIL, y, sub==subN ) ;
			y += FH ;
			subN++;

			g_model.swashInvertCOL = hyphinvMenuItem( g_model.swashInvertCOL, y, sub==subN ) ;

		}	
		break ;

		case M_EXPO :
		{
			TITLEP(Str_Expo) ;
			IlinesCount = 5 ;
	
			uint8_t stkVal ;
			StickScrollAllowed = 0 ;
	
			uint8_t l_expoChan = s_expoChan ;
			{
  		  uint8_t attr = 0 ;
				if ( sub == 0 )
				{
					s_expoChan = l_expoChan = checkIncDec( l_expoChan, 0, 3, 0 ) ;
					attr = INVERS ;
				}		 
				putsChnRaw(0,FH,l_expoChan+1,attr) ;
			}
	
			uint8_t expoDrOn = get_dr_state(l_expoChan);
	
  		lcd_putsAttIdx(  7*FW, FH, PSTR(STR_4DR_HIMIDLO), expoDrOn, 0 ) ;
	
			lcd_puts_Pleft( FH,PSTR(STR_EXPO_TEXT));

			stkVal = DR_BOTH ;
			if(calibratedStick[l_expoChan]> 100) stkVal = DR_RIGHT ;
			if(calibratedStick[l_expoChan]<-100) stkVal = DR_LEFT ;
			if(IS_EXPO_THROTTLE(l_expoChan)) stkVal = DR_RIGHT;

			editExpoVals( (stkVal != DR_RIGHT) && (sub==1), 4*FW, 3*FH, expoDrOn ,DR_EXPO, DR_LEFT ) ;
			editExpoVals( (stkVal != DR_LEFT) && (sub==1), 8*FW, 3*FH, expoDrOn ,DR_EXPO, DR_RIGHT ) ;

			editExpoVals( (stkVal != DR_RIGHT) && (sub==2), 4*FW, 5*FH, expoDrOn ,DR_WEIGHT, DR_LEFT ) ;
			editExpoVals( (stkVal != DR_LEFT) && (sub==2), 8*FW, 5*FH, expoDrOn ,DR_WEIGHT, DR_RIGHT ) ;

			editExpoVals( sub==3,5*FW, 6*FH, DR_DRSW1 , 0,0);
			editExpoVals( sub==4,5*FW, 7*FH, DR_DRSW2 , 0,0);

			lcd_vline(XD - (IS_EXPO_THROTTLE(s_expoChan) ? WCHART : 0), Y0 - WCHART, WCHART * 2);

			plotType = PLOT_BLACK ;

			drawFunction( XD, GRAPH_FUNCTION_EXPO ) ;
	
			int16_t x512  = calibratedStick[s_expoChan];
			int16_t y512 = calcExpo( s_expoChan, x512 ) ;
	
			lcd_outdez( 19*FW, 6*FH,x512*25/((signed) RESXu/4) );
			lcd_outdez( 14*FW, 1*FH,y512*25/((signed) RESXu/4) );
	
			int8_t xv = (x512 * WCHART + RESX/2) / RESX + XD ;
  		int8_t yv = Y0 - (y512 * WCHART + RESX/2) / RESX ;

			lcd_vline( xv, yv-3, 7 ) ;
			lcd_hline( xv-3, yv, 7 ) ;
	
			plotType = PLOT_XOR ;
	
	  }
		break ;

		case M_PROTOCOL :
		{
			
/*			
#ifdef V2
			
#define P_CHANNELS		0x0001			
#define P_RXNUM				0x0002
#define P_PPMDELAY		0x0004
#define P_SUBPROTO		0x0008
#define P_SUBSUBPROTO	0x0010
#define P_FRAMELEN		0x0020
#define P_COUNTRY			0x0040
#define P_AUTOBIND		0x0080
#define P_POLARITY		0x0100
#define P_OPTION			0x0200
#define P_BIND				0X0400
#define P_RANGE				0x0800

#define SHIFT_PROTOCOL	1

#ifdef SHIFT_PROTOCOL
		{
			uint8_t dataItems = 6 ;
			uint8_t protocol = g_model.protocol ;
			uint16_t needed = P_CHANNELS | P_PPMDELAY | P_FRAMELEN | P_POLARITY ;
			if (protocol == PROTO_PXX)
			{
				dataItems = 7 ;
				needed = P_RXNUM	| P_SUBPROTO | P_COUNTRY | P_BIND | P_RANGE ;
			}
			if ( protocol == PROTO_DSM2 )
			{
				dataItems = 4 ;
				needed = P_RXNUM	| P_SUBPROTO ;
			}
#ifdef MULTI_PROTOCOL
			if (protocol == PROTO_MULTI)
			{
				dataItems = 9 ;
				needed = P_RXNUM	| P_SUBPROTO | P_SUBSUBPROTO | P_AUTOBIND | P_OPTION | P_BIND | P_RANGE ;
			}
#endif
			TITLEP(Str_Protocol) ;
			IlinesCount = dataItems ;
			
			uint8_t blink = InverseBlink ;
  		uint8_t attr = 0 ;
			uint8_t subN = 0 ;
  		
			lcd_puts_Pleft( y, PSTR(STR_1ST_CHAN_PROTO));
  		if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0(g_model.ppmStart,7) ; }
  		lcd_putcAtt( 19*FW, y, '1'+g_model.ppmStart, attr);
			y += FH ;
			subN++;

			lcd_putsAttIdx(  6*FW, y, PSTR(PROT_STR), protocol, (sub==subN ? blink:0) );
			if(sub==subN )
			{
#if (PROT_MAX == PROTO_PPMSIM )
				uint8_t prot_max = PROT_MAX ;
				if ( g_eeGeneral.enablePpmsim == 0 )
				{
					prot_max -= 1 ;
				}
 		  	CHECK_INCDEC_H_MODELVAR_0(protocol, prot_max ) ;
#else
				uint8_t oldProtocol = protocol ;
 		  	CHECK_INCDEC_H_MODELVAR_0(protocol, PROT_MAX ) ;
				if ( g_eeGeneral.enablePpmsim == 0 )
				{
					if ( protocol == PROTO_PPMSIM )
					{
						if ( oldProtocol > protocol )
						{
							protocol -= 1 ;
						}
						else
						{
							protocol += 1 ;
						}
					}
				}
#endif
				g_model.protocol = protocol ;
			}
			y += FH ;
			subN++;
			if ( needed & P_CHANNELS )
			{
		 		attr = 0 ;
	  		lcd_puts_Pleft( y, PSTR("Channels"));
				if(sub==subN )
				{
					attr = blink ;
	        CHECK_INCDEC_H_MODELVAR(g_model.ppmNCH,-2,4);
				}
 			  lcd_putsAttIdx(  17*FW, y, PSTR(STR_PPMCHANNELS),(g_model.ppmNCH+2), attr ) ;
				y += FH ;
				subN++;
			}
		
			if ( needed & P_RXNUM )
		  {
				uint8_t max ;
				uint8_t value ;
				y -= FH ;
#ifdef MULTI_PROTOCOL
				if (protocol == PROTO_MULTI)
				{
					value = g_model.ppmNCH & 0x0F;
					max = 15 ;
				}
				else
				{
					value = g_model.ppmNCH ;
					max = 124 ;
				}	
#else
				value = g_model.ppmNCH ;
				max = 124 ;
#endif
				attr = 0 ;
				if(sub==subN )
				{
					attr = blink ;
					CHECK_INCDEC_H_MODELVAR(value, 0, max);
				}
#ifdef MULTI_PROTOCOL
				if (protocol == PROTO_MULTI)
				{
					g_model.ppmNCH=(g_model.ppmNCH & 0xF0) + value ;
				}
				else
				{
					g_model.ppmNCH = value ;
				}
#else
				g_model.ppmNCH = value ;
#endif
				lcd_xlabel_decimal( 21*FW, y, value, attr, PSTR("\014RxNum") ) ;
				y += FH ;
				subN++;
		  }
			
			if ( needed & P_PPMDELAY )
			{
		 		attr = 0 ;
				if(sub==subN )
				{
					attr = blink ;
	        CHECK_INCDEC_H_MODELVAR( g_model.ppmDelay, -4, 10) ;
				}
				lcd_xlabel_decimal( 20*FW, y, (g_model.ppmDelay*50)+300, attr, PSTR("Delay") ) ;
				y += FH ;
				subN++;
			}
			if ( needed & P_SUBPROTO )
			{
		 		attr = 0 ;
				if ( protocol == PROTO_DSM2 )
				{
					
  			  lcd_puts_Pleft( y, PSTR(STR_DSM_TYPE));
					g_model.sub_protocol = checkIndexed( y, PSTR(FWx10"\002"DSM2_STR), g_model.sub_protocol, (sub==subN) ) ;
				}
				else if ( protocol == PROTO_PXX )
				{
					lcd_puts_Pleft( y, PSTR("Type") ) ;
					g_model.sub_protocol = checkIndexed( y, PSTR(FWx10"\002\003D16D8 LRP"), g_model.sub_protocol, (sub==subN) ) ;
				}
#ifdef MULTI_PROTOCOL
				else
				{
					lcd_puts_Pleft( y, PSTR("Protocol"));
					attr = g_model.sub_protocol ;
					g_model.sub_protocol = checkIndexed( y, PSTR(FWx10"\012"MULTI_STR), g_model.sub_protocol&0x1F, (sub==subN) ) + (g_model.sub_protocol&0xE0);
					if(g_model.sub_protocol!=attr) g_model.ppmNCH &= 0x0F;
				}
#endif
				y += FH ;
				subN++;
			}
		
#ifdef MULTI_PROTOCOL
			if ( needed & P_SUBSUBPROTO )
			{
				lcd_puts_Pleft( y, PSTR("Type"));
				uint8_t nchLow = g_model.ppmNCH &0x0F ;
				uint8_t nchHi = g_model.ppmNCH >> 4 ;
				switch(g_model.sub_protocol&0x1F)
				{
					case M_Flysky:
						nchHi = checkIndexed( y, PSTR(FWx10"\003"M_FLYSKY_STR),nchHi, (sub==subN) ) ;
						break;
					case M_DSM:
						nchHi = checkIndexed( y, PSTR(FWx10"\001"M_DSM2_STR),  nchHi, (sub==subN) ) ;
						break;
					case M_YD717:
						nchHi = checkIndexed( y, PSTR(FWx10"\004"M_YD717_STR), nchHi, (sub==subN) ) ;
						break;
					case M_SymaX:
						nchHi = checkIndexed( y, PSTR(FWx10"\001"M_SYMAX_STR), nchHi, (sub==subN) ) ;
						break;
					default:
						nchHi = 0 ;
						nchHi = checkIndexed( y, PSTR(FWx10"\000"M_NONE_STR),  nchHi, (sub==subN) );
						break;
				}
				g_model.ppmNCH = (nchHi << 4) + nchLow ;
				y += FH ;
				subN++;
				
			}
#endif
			 
			if ( needed & P_FRAMELEN )
			{
				attr = PREC1 ;
  			if(sub==subN)
				{
					attr = INVERS | PREC1 ;
					CHECK_INCDEC_H_MODELVAR(g_model.ppmFrameLength,-20,20) ;
				}
				lcd_xlabel_decimal( 13*FW-2, y, (int16_t)g_model.ppmFrameLength*5 + 225, attr, PSTR(STR_PPMFRAME_MSEC) ) ;
				y += FH ;
				subN++;
			}

			if ( needed & P_COUNTRY )
			{
				lcd_puts_Pleft( y, PSTR("Country") ) ;
				g_model.country = checkIndexed( y, PSTR(FWx10"\002\003AmeJapEur"), g_model.country, (sub==subN) ) ;
				y += FH ;
				subN++;
			}

#ifdef MULTI_PROTOCOL
			if ( needed & P_AUTOBIND )
			{
				attr = 0 ;
				lcd_puts_Pleft( y, PSTR("Autobind"));
				uint8_t value = (g_model.sub_protocol>>6)&0x01 ;
				if(sub==subN)
				{
					attr = blink ;
					CHECK_INCDEC_H_MODELVAR_0(value, 1 );
					g_model.sub_protocol = (value<<6) + (g_model.sub_protocol&0xBF);
				}
				lcd_putsAttIdx(  9*FW, y, Str_NY, value, attr );
				y += FH ;
				subN++;
			}
#endif

			if ( needed & P_POLARITY )
			{
  		 	lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
				g_model.pulsePol = checkIndexed( y, PSTR(FWx17"\001"STR_POS_NEG), g_model.pulsePol, (sub==subN) ) ;
				y += FH ;
				subN++;
			}

#ifdef MULTI_PROTOCOL
			if ( needed & P_OPTION )
			{
				attr = 0 ;
				y -= FH ;
				if(sub==subN)
				{
					attr = blink ;
					CHECK_INCDEC_H_MODELVAR(g_model.option_protocol, -127, 127);
				}
				lcd_xlabel_decimal( 21*FW, y, g_model.option_protocol, attr, PSTR("\014Option") ) ;
				y += FH ;
				subN++;
			}
#endif
			
			if ( needed & P_BIND )
			{
				lcd_puts_Pleft( y, PSTR("Bind") ) ;
  			if(sub==subN)
				{
					rangeBindAction( y, PXX_BIND ) ;
  			}	
				y += FH ;
				subN++;

			}
  			
			if ( needed & P_RANGE )
			{
				lcd_puts_Pleft( y, PSTR("Range") ) ;
				if(sub==subN)
				{
					rangeBindAction( y, PXX_RANGE_CHECK ) ;
  			}		
			}
		}	

#else // SHIFT_PROTOCOL
		{
			uint8_t dataItems = 4 ;
			uint8_t protocol = g_model.protocol ;
			if (protocol == PROTO_PXX)
			{
				dataItems = 6 ;
			}
			if ( protocol == PROTO_DSM2 )
			{
				dataItems = 3 ;
			}
#ifdef MULTI_PROTOCOL
			if (protocol == PROTO_MULTI)
			{
				dataItems = 7 ;
			}
#endif

			TITLEP(Str_Protocol) ;
			IlinesCount = dataItems ;
			
			uint8_t blink = InverseBlink ;
			uint8_t subSub = g_posHorz ;
			uint8_t subN = 0 ;

  		uint8_t attr = 0 ;
  		lcd_puts_Pleft(    y, PSTR(STR_1ST_CHAN_PROTO));
  		if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0(g_model.ppmStart,7) ; }
  		lcd_putcAtt( 19*FW, y, '1'+g_model.ppmStart, attr);
			y += FH ;
			subN++;

			uint8_t ppmTypeProto = 0 ;
  		if( ( protocol == PROTO_PPM ) || (protocol == PROTO_PPM16) || (protocol == PROTO_PPMSIM) )
			{
				ppmTypeProto = 1 ;
			}

			uint8_t cols = 0 ;
			lcd_putsAttIdx(  6*FW, y, PSTR(PROT_STR), protocol, (sub==subN && subSub==0 ? blink:0) );
  		if( ppmTypeProto )// ( protocol == PROTO_PPM ) || (protocol == PROTO_PPM16) || (protocol == PROTO_PPMSIM) )
			{
				uint8_t x ;
				cols = 2 ;
			  lcd_puts_Pleft( y, PSTR(STR_23_US) );
				x = 12*FW ;
 			  lcd_putsAttIdx(  x, y, PSTR(STR_PPMCHANNELS),(g_model.ppmNCH+2),(sub==subN && subSub==1  ? blink:0));
 			  lcd_outdezAtt(  x+7*FW-2, y,  (g_model.ppmDelay*50)+300, (sub==subN && subSub==2 ? blink:0));
  		}
  		else // if (protocol == PROTO_PXX) || DSM2 || MULTI
  		{
				cols = 1 ;
#ifdef MULTI_PROTOCOL
					if (protocol == PROTO_MULTI)
						lcd_xlabel_decimal( 21*FW, y, g_model.ppmNCH & 0x0F, (sub==subN && subSub==1 ? blink:0), PSTR(STR_13_RXNUM) ) ;
					else
#endif
						lcd_xlabel_decimal( 21*FW, y, g_model.ppmNCH, (sub==subN && subSub==1 ? blink:0), PSTR(STR_13_RXNUM) ) ;
			}

			if(sub==subN )
			{
				Columns = cols ;
			 	if (s_editing )
				{
//					uint8_t prot_max = PROT_MAX ;

//					if ( g_eeGeneral.enablePpmsim == 0 )
//					{
//						prot_max -= 1 ;
//					}
  		  	switch (subSub){
  		  	case 0:
					{
#if (PROT_MAX == PROTO_PPMSIM )
						uint8_t prot_max = PROT_MAX ;
						if ( g_eeGeneral.enablePpmsim == 0 )
						{
							prot_max -= 1 ;
						}
  		  	  CHECK_INCDEC_H_MODELVAR_0(protocol, prot_max ) ;
#else
						uint8_t oldProtocol = protocol ;
  		  	  CHECK_INCDEC_H_MODELVAR_0(protocol, PROT_MAX ) ;
						if ( g_eeGeneral.enablePpmsim == 0 )
						{
							if ( protocol == PROTO_PPMSIM )
							{
								if ( oldProtocol > protocol )
								{
									protocol -= 1 ;
								}
								else
								{
									protocol += 1 ;
								}
							}
						}
#endif
					}
  		  	break;
  		  	case 1:
  		  	    if( ppmTypeProto ) //if ((protocol == PROTO_PPM) || (protocol == PROTO_PPM16)|| (protocol == PROTO_PPMSIM) )
  		  	        CHECK_INCDEC_H_MODELVAR(g_model.ppmNCH,-2,4);
  		  	    else // if (protocol == PROTO_PXX) || DSM2 || MULTI
#ifdef MULTI_PROTOCOL
					if (protocol == PROTO_MULTI)
					{
						attr = g_model.ppmNCH & 0x0F;
						CHECK_INCDEC_H_MODELVAR(attr, 0, 15);
						g_model.ppmNCH=(g_model.ppmNCH & 0xF0) + attr;
					}
					else
#endif
						CHECK_INCDEC_H_MODELVAR(g_model.ppmNCH, 0, 124);
  		  	    break;
  		  	case 2:
  		  	    if( ppmTypeProto ) //if ((protocol == PROTO_PPM) || (protocol == PROTO_PPM16) || (protocol == PROTO_PPMSIM) )
  		  	        CHECK_INCDEC_H_MODELVAR(g_model.ppmDelay,-4,10);
  		  	    break;
  		  	}
					uint8_t oldProtocol = g_model.protocol ;
					g_model.protocol = protocol ;
  		  	if(oldProtocol != protocol) // if change - reset ppmNCH
  		  	    g_model.ppmNCH = 0;
				}
			}
			y += FH ;
			subN++;

  		if( ppmTypeProto ) //if( (protocol == PROTO_PPM) || (protocol == PROTO_PPM16) || (protocol == PROTO_PPMSIM) )
			{
				uint8_t attr = PREC1 ;
  		  if(sub==subN) { attr = INVERS | PREC1 ; CHECK_INCDEC_H_MODELVAR(g_model.ppmFrameLength,-20,20) ; }
				lcd_xlabel_decimal( 13*FW-2, y, (int16_t)g_model.ppmFrameLength*5 + 225, attr, PSTR(STR_PPMFRAME_MSEC) ) ;
				y += FH ;
				subN++;
			
  		 	lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
				
				g_model.pulsePol = checkIndexed( y, PSTR(FWx17"\001"STR_POS_NEG), g_model.pulsePol, (sub==subN) ) ;
				y += FH ;
				subN++;
			}
#ifdef MULTI_PROTOCOL
			if (protocol == PROTO_MULTI)
			{
				uint8_t nchLow = g_model.ppmNCH &0x0F ;
				lcd_puts_Pleft(    y, PSTR(STR_MULTI_TYPE));
				attr=g_model.sub_protocol;
				g_model.sub_protocol = checkIndexed( y, PSTR(FWx10"\012"MULTI_STR), g_model.sub_protocol&0x1F, (sub==subN) ) + (g_model.sub_protocol&0xE0);
				if(g_model.sub_protocol!=attr) g_model.ppmNCH &= 0x0F;
				uint8_t nchHi = g_model.ppmNCH >> 4 ;
				y += FH ;
				subN++;
				switch(g_model.sub_protocol&0x1F)
				{
					case M_Flysky:
						nchHi = checkIndexed( y, PSTR(FWx10"\003"M_FLYSKY_STR),nchHi, (sub==subN) ) ;
						break;
					case M_DSM2:
						nchHi = checkIndexed( y, PSTR(FWx10"\001"M_DSM2_STR),  nchHi, (sub==subN) ) ;
						break;
					case M_YD717:
						nchHi = checkIndexed( y, PSTR(FWx10"\004"M_YD717_STR), nchHi, (sub==subN) ) ;
						break;
					case M_SymaX:
						nchHi = checkIndexed( y, PSTR(FWx10"\001"M_SYMAX_STR), nchHi, (sub==subN) ) ;
						break;
					default:
						nchHi = 0 ;
						nchHi = checkIndexed( y, PSTR(FWx10"\000"M_NONE_STR),  nchHi, (sub==subN) );
						break;
				}
				g_model.ppmNCH = (nchHi << 4) + nchLow ;
				y += FH ;
				subN++;

				uint8_t value = (g_model.sub_protocol>>6)&0x01 ;
				lcd_putsAttIdx(  9*FW, y, Str_NY, value, (sub==subN && subSub==0 ? blink:0) );
				lcd_xlabel_decimal( 21*FW, y, g_model.option_protocol, (sub==subN && subSub==1 ? blink:0), PSTR(STR_MULTI_OPTION) ) ;
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
				y += FH ;
				subN++;
			}
#endif // MULTI_PROTOCOL
			if (protocol == PROTO_DSM2)
			{
  		  lcd_puts_Pleft(    y, PSTR(STR_DSM_TYPE));
				
				g_model.sub_protocol = checkIndexed( y, PSTR(FWx10"\002"DSM2_STR), g_model.sub_protocol, (sub==subN) ) ;
				y += FH ;
				subN++;
			}

			if (protocol == PROTO_PXX)
			{
				lcd_puts_Pleft( y, PSTR(" Type\037 Country\037Bind\037Range") ) ;
	  
				g_model.sub_protocol = checkIndexed( y, PSTR(FWx10"\002\003D16D8 LRP"), g_model.sub_protocol, (sub==subN) ) ;
				y += FH ;
				subN++;
			
				g_model.country = checkIndexed( y, PSTR(FWx10"\002\003AmeJapEur"), g_model.country, (sub==subN) ) ;
				y += FH ;
				subN++;
			}
			
#ifdef MULTI_PROTOCOL
			if ( (protocol == PROTO_PXX) || (protocol == PROTO_MULTI) )
#else
			if (protocol == PROTO_PXX)
#endif
			{
  			if(sub==subN)
				{
					rangeBindAction( y, PXX_BIND ) ;
  			}	
				y += FH ;
				subN++;

  			if(sub==subN)
				{
					rangeBindAction( y, PXX_RANGE_CHECK ) ;
  			}		
			}
	  }
#endif // SHIFT_PROTOCOL

#else // V2			 
			
			
			
*/		
			
			uint8_t dataItems = 4 ;
			uint8_t protocol = g_model.protocol ;
			if (protocol == PROTO_PXX)
			{
//#if defined(CPUM128) || defined(CPUM2561)
#ifdef FAILSAFE  			
				dataItems = 8 ;
#else
				dataItems = 7 ;
#endif
//#else
//				dataItems = 6 ;
//#endif
//#if defined(CPUM128) || defined(CPUM2561) || defined(V2)
//				if ( g_model.sub_protocol == 3 )	// R9M
//				{
//					dataItems += 1 ;
//				}
//#endif
			}
			if ( protocol == PROTO_DSM2 )
			{
				dataItems = 3 ;
			}
#ifdef MULTI_PROTOCOL
			if (protocol == PROTO_MULTI)
			{
#ifdef FAILSAFE  			
				dataItems = 8 ;
#else
				dataItems = 7 ;
#endif
			}
#endif

			TITLEP(Str_Protocol) ;
			IlinesCount = dataItems ;
			
			uint8_t blink = InverseBlink ;
			uint8_t subSub = g_posHorz ;
			uint8_t subN = 0 ;

  		uint8_t attr = 0 ;
			y = 0 ;
  		lcd_puts_Pleft(    y, PSTR(STR_1ST_CHAN_PROTO));
  		if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0(g_model.ppmStart,7) ; }
  		lcd_putcAtt( 19*FW, y, '1'+g_model.ppmStart, attr);
			y += FH ;
			subN++;

			uint8_t ppmTypeProto = 0 ;
  		if( ( protocol == PROTO_PPM ) || (protocol == PROTO_PPM16) || (protocol == PROTO_PPMSIM) )
			{
				ppmTypeProto = 1 ;
			}

			uint8_t cols = 0 ;
			lcd_putsAttIdx(  6*FW, y, PSTR(PROT_STR), protocol, (sub==subN && subSub==0 ? blink:0) );
  		if( ppmTypeProto )// ( protocol == PROTO_PPM ) || (protocol == PROTO_PPM16) || (protocol == PROTO_PPMSIM) )
			{
				uint8_t x ;
				cols = 2 ;
			  lcd_puts_Pleft( y, PSTR(STR_23_US) );
				x = 12*FW ;
 			  lcd_putsAttIdx(  x, y, PSTR(STR_PPMCHANNELS),(g_model.ppmNCH+2),(sub==subN && subSub==1  ? blink:0));
 			  lcd_outdezAtt(  x+7*FW-2, y,  (g_model.ppmDelay*50)+300, (sub==subN && subSub==2 ? blink:0));
  		}
  		else // if (protocol == PROTO_PXX) || DSM2 || MULTI
  		{
				cols = 1 ;
				int8_t value = g_model.ppmNCH ;
#ifdef MULTI_PROTOCOL
				if (protocol == PROTO_MULTI)
				{
					value &= 0x0F ;
				}
#endif
				lcd_xlabel_decimal( 21*FW, y, value, (sub==subN && subSub==1 ? blink:0), PSTR(STR_13_RXNUM) ) ;
			}

			if(sub==subN )
			{
				Columns = cols ;
			 	if (s_editing )
				{
//					uint8_t prot_max = PROT_MAX ;

//					if ( g_eeGeneral.enablePpmsim == 0 )
//					{
//						prot_max -= 1 ;
//					}
  		  	switch (subSub){
  		  	case 0:
					{
//#if (PROT_MAX == PROTO_PPMSIM )
//						uint8_t prot_max = PROT_MAX ;
//						if ( g_eeGeneral.enablePpmsim == 0 )
//						{
//							prot_max -= 1 ;
//						}
//  		  	  CHECK_INCDEC_H_MODELVAR_0(protocol, prot_max ) ;
//#else
//						uint8_t oldProtocol = protocol ;
  		  	  CHECK_INCDEC_H_MODELVAR_0(protocol, PROT_MAX ) ;
//						if ( g_eeGeneral.enablePpmsim == 0 )
//						{
//							if ( protocol == PROTO_PPMSIM )
//							{
//								if ( oldProtocol > protocol )
//								{
//									protocol -= 1 ;
//								}
//								else
//								{
//									protocol += 1 ;
//								}
//							}
//						}
//#endif
					}
  		  	break;
  		  	case 1:
  		  	    if( ppmTypeProto ) //if ((protocol == PROTO_PPM) || (protocol == PROTO_PPM16)|| (protocol == PROTO_PPMSIM) )
  		  	        CHECK_INCDEC_H_MODELVAR(g_model.ppmNCH,-2,4);
  		  	    else // if (protocol == PROTO_PXX) || DSM2 || MULTI
							{
#ifdef MULTI_PROTOCOL
								if (protocol == PROTO_MULTI)
								{
									attr = g_model.ppmNCH & 0x0F;
									attr |= g_model.exRxNum << 4 ;
									CHECK_INCDEC_H_MODELVAR_0(attr, 63);
									g_model.ppmNCH=(g_model.ppmNCH & 0xF0) + ( attr & 0x0F );
									g_model.exRxNum = attr >> 4 ;
								}
								else
#endif
									CHECK_INCDEC_H_MODELVAR(g_model.ppmNCH, 0, 63);
							}
  		  	break;
  		  	case 2:
  		  	    if( ppmTypeProto ) //if ((protocol == PROTO_PPM) || (protocol == PROTO_PPM16) || (protocol == PROTO_PPMSIM) )
  		  	        CHECK_INCDEC_H_MODELVAR(g_model.ppmDelay,-4,10);
  		  	    break;
  		  	}
					uint8_t oldProtocol = g_model.protocol ;
					g_model.protocol = protocol ;
  		  	if(oldProtocol != protocol) // if change - reset ppmNCH
  		  	    g_model.ppmNCH = 0;
//#if defined(CPUM128) || defined(CPUM2561)
							g_model.pxxChans = 0 ;
//#endif
				}
			}
			y += FH ;
			subN++;

  		if( ppmTypeProto ) //if( (protocol == PROTO_PPM) || (protocol == PROTO_PPM16) || (protocol == PROTO_PPMSIM) )
			{
				uint8_t attr = PREC1 ;
  		  if(sub==subN) { attr = INVERS | PREC1 ; CHECK_INCDEC_H_MODELVAR(g_model.ppmFrameLength,-20,20) ; }
				lcd_xlabel_decimal( 13*FW-2, y, (int16_t)g_model.ppmFrameLength*5 + 225, attr, PSTR(STR_PPMFRAME_MSEC) ) ;
				y += FH ;
				subN++;
			}
#ifdef SBUS_PROTOCOL	
  		if ( ( ppmTypeProto ) || (protocol == PROTO_SBUS) ) //if( (protocol == PROTO_PPM) || (protocol == PROTO_PPM16) || (protocol == PROTO_PPMSIM) )
#else
  		if ( ppmTypeProto )
#endif
			{ 
  		 	lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
				
				g_model.pulsePol = checkIndexedV( y, PSTR(FWx17"\001"STR_POS_NEG), g_model.pulsePol, subN ) ;
				y += FH ;
				subN++;
			}
#ifdef MULTI_PROTOCOL
			if (protocol == PROTO_MULTI)
			{
				uint8_t attr = 0 ;
				if ( MultiDataRequest == 0 )
				{
					Xmem.MultiSetting.valid = 0 ;
				}
				MultiDataRequest = 25 ;

				// Display on screen static text
				lcd_puts_Pleft(    y, PSTR(STR_MULTI_TYPE));
				// Sub-protocol stored in sub_protocol bits0..4
				uint8_t oldValue = g_model.sub_protocol & 0x3F ;
				oldValue |= g_model.exSubProtocol << 6 ;
				uint8_t svalue = oldValue ;
				if ( sub == subN )
				{
					attr = blink ;
 			  	CHECK_INCDEC_H_MODELVAR_0( svalue, 127 ) ;	// Limited to 8 bits
				}
#if defined(CPUM128) || defined(CPUM2561)
static uint8_t multiUpdateTimer ;
				if ( ( svalue != oldValue ) || multiUpdateTimer )
				{
					Xmem.MultiSetting.valid &= ~6 ;
					Xmem.MultiSetting.protocol[0] = 0 ;
					if ( multiUpdateTimer == 0 )
					{
						multiUpdateTimer = 15 ;
					}
				}
				if ( multiUpdateTimer )
				{
					multiUpdateTimer -= 1 ;
				}
#endif
//#if defined(CPUM128) || defined(CPUM2561)
//				if ( Xmem.MultiSetting.valid & 2 )
//				{
//					lcd_putsAtt( 0, 0, (const char *)Xmem.MultiSetting.protocol, BSS ) ;
//				}
//				if ( Xmem.MultiSetting.valid & 4 )
//				{
//					lcd_putsAtt( 0, 8, (const char *)Xmem.MultiSetting.subProtocol, BSS ) ;
//				}
//#endif				
				 
//				attr=g_model.sub_protocol;


//#ifdef V2
//				g_model.sub_protocol = checkIndexed( y, PSTR(FWx10"\015"MULTI_STR), g_model.sub_protocol, (sub==subN) ) ;
//#else
				
//				g_model.sub_protocol = checkIndexed( y, PSTR(FWx10"\027"MULTI_STR), g_model.sub_protocol&0x1F, (sub==subN) ) + (g_model.sub_protocol&0xE0);
				
#ifdef FRSKY				
				if ( ( Xmem.MultiSetting.valid & 2 ) && (Xmem.MultiSetting.protocol[0]) )
//				if ( svalue <= M_LAST_MULTI )
				{
					lcd_putsAtt( FW*10, y, (const char *)Xmem.MultiSetting.protocol, attr | BSS ) ;
//					lcd_putsAttIdx( FW*10, y, PSTR(MULTI_STR), svalue, attr ) ;
				}
				else
				{
 #if defined(CPUM128) || defined(CPUM2561)
 					if ( svalue < NUM_MULTI_PROTOCOLS )
					{
						lcd_putsAttIdx( FW*10, y, PSTR(MULTI_STR), svalue, attr ) ;
					}
					else
					{
  					lcd_outdezAtt( 21*FW, y, svalue+1, attr ) ;
					}
 #else
  				lcd_outdezAtt( 21*FW, y, svalue+1, attr ) ;
 #endif
				}
#else
				if ( svalue < NUM_MULTI_PROTOCOLS )
				{
					lcd_putsAttIdx( FW*10, y, PSTR(MULTI_STR), svalue, attr ) ;
				}
				else
				{
		  		lcd_outdezAtt( 21*FW, y, svalue+1, attr ) ;
				}
#endif
				g_model.sub_protocol = ( svalue & 0x3F) + (g_model.sub_protocol & 0xC0) ;
				g_model.exSubProtocol = svalue >> 6 ;

//				attr = 0 ;
//				uint8_t svalue = g_model.sub_protocol & 0x1F ;
//  		  if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( svalue, 31 ) ; }
//				g_model.sub_protocol = svalue + ( g_model.sub_protocol & 0xE0 ) ;
//			  lcd_outdezAtt( 20*FW, y, svalue, attr ) ;


//#endif
				uint8_t ppmNch = g_model.ppmNCH ;
				if(svalue==oldValue)
					attr=(ppmNch >> 4) &0x07 ;
				else
					attr=0;
				y += FH ;
				subN++;
				//Sub-sub-protocol stored in ppmNCH bits4..6

#if (defined(CPUM128) || defined(CPUM2561)) || not defined(FRSKY)
				const prog_char * s;
				uint8_t x = g_model.sub_protocol & 0x3F ;
				x |= g_model.exSubProtocol << 6 ;
				if ( x == M_Flysky)
				{
					s = M_FLYSKY_STR ;
				}
				else if ( x == M_Hisky )
				{
					s = M_HISKY_STR ;
				}
				else if ( x == M_Hubsan )
				{
					s = M_HUBSAN_STR ;
				}
				else if ( x == M_DSM )
				{
					s = M_DSM2_STR ;
				}
				else if ( x == M_YD717 )
				{
					s = M_YD717_STR ;
				}
				else if ( x == M_KN )
				{
					s = M_KN_STR ;
				}
				else if ( x == M_SymaX )
				{
					s = M_SYMAX_STR ;
				}
				else if ( x == M_SLT )
				{
					s = M_SLT_STR ;
				}
				else if ( x == M_CX10 )
				{
					s = M_CX10_STR ;
				}
				else if ( x == M_CG023 )
				{
					s = M_CG023_STR ;
				}
				else if ( x == M_BAYANG )
				{
					s = M_BAYANG_STR ;
				}
				else if ( x == M_FRSKYX )
				{
					s = M_FRSKY_STR ;
				}
				else if ( x == M_MT99XX )
				{
					s = M_MT99XX_STR ;
				}
				else if ( x == M_MJXQ )
				{
					s = M_MJXQ_STR ;
				}
				else if ( x == M_FY326 )
				{
					s = M_FY326_STR ;
				}
				else if ( x == M_HONTAI )
				{
					s = M_HONTAI_STR ;
				}
				else if ( x == M_AFHD2SA )
				{
					s = M_AFHD2SA_STR ;
				}
				else if ( x == M_Q2X2 )
				{
					s = M_Q2X2_STR ;
				}
				else if ( x == M_WK2x01 )
				{
					s = M_WK2x01_STR ;
				}
				else if ( x == M_Q303 )
				{
					s = M_Q303_STR ;
				}
				else if ( x == M_CABELL )
				{
					s = M_CABELL_STR ;
				}
				else if ( x == M_H8_3D )
				{
					s = M_H8_3D_STR ;
				}
				else if ( x == M_CORONA )
				{
					s = M_CORONA_STR ;
				}
				else if ( x == M_WFLY )
				{
					s = M_WFLY1_STR ;
				}
				else if ( x == M_Hitec )
				{
					s = M_HITEC_STR ;
				}
				else if ( x == M_BUGSMINI )
				{
					s = M_BUGSMINI_STR ;
				}
				else if ( x == M_E01X     )
				{
					s = M_E01X_STR ;
				}
				else if ( x == M_V911S    )
				{
					s = M_V911S_STR ;
				}
				else if ( x == M_GD00X    )
				{
					s = M_GD00X_STR ;
				}
				else if ( x == M_Redpine  )
				{
					s = M_REDPINE_STR ;
				}
				else if ( x == M_Potensic )
				{
					s = M_POTENSIC_STR ;
				}
				else if ( x == M_ZSX      )
				{
					s = M_ZSX_STR ;
				}
				else if ( x == M_Flyzone  )
				{
					s = M_FLYZONE_STR ;
				}
				else if ( x == M_Frsky_RX )
				{
					s = M_FRSKYRX_STR ;
				}
				else if ( x == M_AFHDS2A_RX )
				{
					s = M_AFHDS2A_RX_STR ;
				}
				else if ( x == M_FX816    )
				{
					s = M_FX816_STR ;
				}
				else if ( x == M_XK       )
				{
					s = M_XK_STR ;
				}
				else if ( x == M_XN_DUMP  )
				{
					s = M_XN_DUMP_STR ;
				}
				else if ( x == M_FrskyX2  )
				{
					s = M_FRSKYX2_STR ;
				}
				else if ( x == M_FrSkyR9  )
				{
					s = M_FRSKYR9_STR ;
				}
				else if ( x == M_PROPEL   )
				{
					s = M_PROPEL_STR ;
				}
				else if ( x == M_LR12     )
				{
					s = M_LR12_STR ;
				}
				else if ( x == M_Skyartec     )
				{
					s = M_STD_STR ;
				}
				else if ( x == M_ESky150V2     )
				{
					s = M_ESKYV2_STR ;
				}
				else if ( x == M_DSM_RX     )
				{
					s = M_DSM_RX_STR ;
				}
				else if ( x == M_JJRC345     )
				{
					s = M_JJRC345_STR ;
				}
				else if ( x == M_Kyosho     )
				{
					s = M_KYOSHO_STR ;
				}
				else if ( x == M_RadioLink     )
				{
					s = M_RADIOLINK_STR ;
				}
				else if ( x == M_Realacc     )
				{
					s = M_REALACC_STR ;
				}
				else if ( x == M_WFLY2     )
				{
					s = M_WFLY_STR ;
				}
				else if ( x == M_E016H     )
				{
					s = M_E016H_STR ;
				}

				else
				{
					s=PSTR("\000"M_NONE_STR);
				}
				uint8_t value = attr ;
				attr = 0 ;
 #if (defined(CPUM128) || defined(CPUM2561))
				oldValue = value ;
 #endif
				if(sub==subN)
				{
					CHECK_INCDEC_H_MODELVAR_0( value, 7 ) ;
					attr = blink ;
				}
				
 #if (defined(CPUM128) || defined(CPUM2561))
				if ( ( value != oldValue ) || multiUpdateTimer )
				{
					Xmem.MultiSetting.valid &= ~4 ;
					Xmem.MultiSetting.subProtocol[0] = 0 ;
					if ( multiUpdateTimer == 0 )
					{
						multiUpdateTimer = 15 ;
					}
				}
				if ( ( Xmem.MultiSetting.valid & 4 ) && (value < (Xmem.MultiSetting.subData & 0x0F)) && (Xmem.MultiSetting.subProtocol[0]) )
				{
					lcd_putsAtt( 10*FW, y, (const char *)Xmem.MultiSetting.subProtocol, attr | BSS ) ;
				}
				else
				{
					uint8_t max = pgm_read_byte(s++) ;
					if ( value <= max )
					{
						lcd_putsAttIdx( 10*FW, y, s, value, attr ) ;
					} 
					else
					{
						lcd_outdezAtt( 11*FW, y, value, attr ) ;
					}
				}
 #else				
				uint8_t max = pgm_read_byte(s++) ;
				if ( value <= max )
				{
					lcd_putsAttIdx( 10*FW, y, s, value, attr ) ;
				} 
				else
				{
					lcd_outdezAtt( 11*FW, y, value, attr ) ;
				}
 #endif
#else
//#if defined(CPUM128) || defined(CPUM2561)
//					s += 1 ;
//					uint8_t max = pgm_read_byte(s++) ;
					uint8_t value = attr ;
					attr = 0 ;
					if(sub==subN)
					{
						CHECK_INCDEC_H_MODELVAR_0( value, 7 ) ;
						attr = blink ;
					}
					if ( ( Xmem.MultiSetting.valid & 4 ) && (value < (Xmem.MultiSetting.subData & 0x0F)) && (Xmem.MultiSetting.subProtocol[0]) )
					{
						lcd_putsAtt( 10*FW, y, (const char *)Xmem.MultiSetting.subProtocol, attr | BSS ) ;
					}
					else
					{
						lcd_outdezAtt( 11*FW, y, value, attr ) ;
					}
#endif

//					if ( ( x <= M_LAST_MULTI ) && (value <= max) )
//					{
//						lcd_putsAttIdx( 10*FW, y, s, value, attr ) ;
//					} 
//					else
//					{
//						lcd_outdezAtt( 11*FW, y, value, attr ) ;
					
//					}
					
					g_model.ppmNCH = (value << 4) + (ppmNch & 0x8F);
//#else
//					attr = checkIndexedV( y, s, attr, subN ) ;
//					g_model.ppmNCH = (attr << 4) + (ppmNch & 0x8F);
//#endif

//#if defined(CPUM128) || defined(CPUM2561)
//					if ( x == M_DSM )
//					{
//						if ( attr == 4 )
//						{
//							if ( old != 4 )
//							{
//								// Go to bind mode
// 					    	pxxFlag = PXX_BIND ;		    	//send bind code or range check code
//								pushMenu(menuRangeBind) ;
//							}
//						}
//					}
//#endif
//#if defined(CPUM128) || defined(CPUM2561)
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
//#endif
				y += FH ;
				subN++;
				// Power stored in ppmNCH bit7 & Option stored in option_protocol
				value = (ppmNch>>7)&0x01 ;
				lcd_putsAttIdx(  6*FW, y, PSTR(M_LH_STR), value, (sub==subN && subSub==0 ? blink:0) );
//				lcd_xlabel_decimal( 21*FW, y, g_model.option_protocol, (sub==subN && subSub==1 ? blink:0), PSTR(STR_MULTI_OPTION) ) ;
//#if defined(CPUM128) || defined(CPUM2561)
#ifndef NMEA
				multiOption( 21*FW, y, g_model.option_protocol, (sub==subN && subSub==1 ? blink:0), (g_model.sub_protocol & 0x3F ) | (g_model.exSubProtocol << 6) ) ;
//#else
#else
				lcd_outdezAtt( 21*FW, y, g_model.option_protocol, (sub==subN && subSub==1 ? blink:0) ) ;
				lcd_puts_Pleft( y, PSTR(STR_MULTI_OPTION) ) ;
//#endif
#endif
				if(sub==subN)
				{
					Columns = 1;
					if(subSub==0 && s_editing)
					{
						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
						ppmNch &= 0x7F ;
						if ( value )
						{
							ppmNch |= 0x80 ;
						}
						g_model.ppmNCH = ppmNch ;
					}
					if(subSub==1 && s_editing)
						CHECK_INCDEC_H_MODELVAR(g_model.option_protocol, -128, 127);
				}
				y += FH ;
				subN++;
				// BIND use PXX_BIND flag & Autobind stored in sub_protocol bit6
				uint8_t subProto = g_model.sub_protocol ;
				value = (subProto>>6)&0x01 ;
				lcd_putsAttIdx(  20*FW, y, Str_NY, value, (sub==subN && subSub==1 ? blink:0) );
				if(sub==subN)
				{
					Columns = 1;
					if(subSub==0)
						rangeBindAction( y, PXX_BIND ) ;
					if(subSub==1 && s_editing)
					{
						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
						subProto &= 0xBF ;
						if ( value )
						{
							subProto |= 0x40 ;
						}
						g_model.sub_protocol = subProto ;
					}
				}		
				y += FH ;
				subN++;

#ifdef FRSKY
				// Range use PXX_RANGE_CHECK
				uint8_t sp = g_model.sub_protocol & 0x3F ;
				if ( ( sp == M_FRSKYX ) || ( sp == M_Frsky ) )
				{
					lcd_puts_Pleft( y, PSTR("\017Lqi") ) ;
					lcd_outdezAtt(  21*FW, y, TxLqi, 0 ) ;
				}
#endif
//#if defined(CPUM128) || defined(CPUM2561)
				if ( Xmem.MultiSetting.valid & 1 )
				{
					// "Range 1.1.11.11
					lcd_putc( 32, y, '(' ) ;
					lcd_putc( 13*6+1, y, ')' ) ;
					lcd_putc( 41, y, '.' ) ;
					lcd_putc( 51, y, '.' ) ;
					lcd_putc( 65, y, '.' ) ;
					lcd_putc( 36, y, (Xmem.MultiSetting.revision[0]) + '0' ) ;
					lcd_putc( 46, y, (Xmem.MultiSetting.revision[1]) + '0' ) ;
					lcd_outdez( 65, y, Xmem.MultiSetting.revision[2] ) ;
					lcd_outdez( 79, y, Xmem.MultiSetting.revision[3] ) ;
//					lcd_outhex4( 32, y, (Xmem.MultiSetting.revision[0] << 8) | Xmem.MultiSetting.revision[1] ) ;
//					lcd_outhex4( 56, y, (Xmem.MultiSetting.revision[2] << 8) | Xmem.MultiSetting.revision[3] ) ;
				}
//#endif				

				if(sub==subN)
					rangeBindAction( y, PXX_RANGE_CHECK ) ;
#ifdef FAILSAFE  			
				y += FH ;
				subN++;
				lcd_puts_Pleft( y, PSTR("Failsafe") ) ;
				if ( g_model.failsafeMode == 0 )
				{
  	  		lcd_puts_Pleft( y, PSTR("\012(Not Set)") ) ;
				}
				if(sub==subN)
		    {
					lcd_char_inverse( 0, y, 8*FW, 0 ) ;
					if ( Tevent==EVT_KEY_LONG(KEY_MENU))
					{
    			  pushMenu( menuSetFailsafe ) ;
  				  s_editMode = 0 ;
    			  killEvents(Tevent);
						Tevent = 0 ;
					}
				}
#endif
			}
#endif // MULTI_PROTOCOL
			if (protocol == PROTO_DSM2)
			{
  		  lcd_puts_Pleft(    y, PSTR(STR_DSM_TYPE));
				
				g_model.sub_protocol = checkIndexedV( y, PSTR(FWx10"\002"DSM2_STR), g_model.sub_protocol, subN ) ;
				y += FH ;
				subN++;
			}

			if (protocol == PROTO_PXX)
			{
				lcd_puts_Pleft( y, PSTR(STR_PXX_TYPE) ) ;
#if defined(R9M_SUPPORT)
 				if ( g_model.sub_protocol == 3 )	// R9M
				{
//					const char *s ;
				  lcd_puts_Pleft( y, PSTR("\012Power") ) ;
//					s = ( g_model.country == 2 ) ? PSTR(FWx15"\003""\006  25-8 25-16200-16500-16") : PSTR(FWx17"\003""\004  10 100 5001000") ;
					if (sub==subN)
					{
						Columns = 1 ;
					}
//#if defined(V2)
//					g_model.r9mPower = checkIndexed( y, ( g_model.country == 2 ) ? PSTR(FWx15"\003""\006  25-8 25-16200-16500-16") : PSTR(FWx17"\003""\004  10 100 5001000"), g_model.r9mPower, (sub==subN) && (subSub == 1) ) ;
//#else
					g_model.r9mPower = checkIndexed( y, ( ( g_model.country == 2 ) && ( g_model.r9MflexMode == 0) ) ? StrR9mPowerEU : StrR9mPowerFCC, g_model.r9mPower, (sub==subN) && (subSub == 1) ) ;
//#endif
	      }
#endif
				g_model.sub_protocol = checkIndexed( y, StrNZ_xjtType, g_model.sub_protocol, (sub==subN) && (subSub == 0) ) ;
				y += FH ;
				subN++;

#if defined(CPUM128) || defined(CPUM2561) || defined(V2)
//#if defined(R9M_SUPPORT)
 				if ( g_model.sub_protocol == 3 )	// R9M
				{
				  lcd_puts_Pleft( y, PSTR("\015Flex") ) ;
					if (sub==subN)
					{
						Columns = 1 ;
					}
					uint8_t temp ;
					temp = checkIndexed( y, PSTR(FWx18"\002\003""OFF915868"), g_model.r9MflexMode, (sub==subN) && (subSub == 1) ) ;
					if ( temp )
					{
						if ( temp != g_model.r9MflexMode )
						{
							alert( PSTR("\001Requires R9M non-\037\001certified firmware\037\001Check Frequency for\037\006your region") ) ;
						}
					}
					g_model.r9MflexMode = temp ;
				}
				g_model.pxxChans = checkIndexed( y, StrNZ_pxxchans, g_model.pxxChans, (sub==subN) && (subSub == 0) ) ;
				y += FH ;
				subN++;
#else
				g_model.pxxChans = checkIndexedV( y, StrNZ_pxxchans, g_model.pxxChans, subN ) ;
				y += FH ;
				subN++;
#endif
#ifdef FAILSAFE  			
				if ( g_model.failsafeMode == 0 )
				{
  	  		lcd_puts_Pleft( y, PSTR("\012(Not Set)") ) ;
				}
				if(sub==subN)
		    {
					lcd_char_inverse( 0, y, 8*FW, 0 ) ;
					if ( Tevent==EVT_KEY_LONG(KEY_MENU))
					{
    			  pushMenu( menuSetFailsafe ) ;
  				  s_editMode = 0 ;
    			  killEvents(Tevent);
						Tevent = 0 ;
					}
				}
				y += FH ;
				subN++;
#endif
					 
				g_model.country = checkIndexedV( y, StrNZ_country, g_model.country, subN ) ;
				y += FH ;
				subN++;
//			}
			
//			if (protocol == PROTO_PXX)
//			{
  			if(sub==subN)
				{
					rangeBindAction( y, PXX_BIND ) ;
  			}	
				y += FH ;
				subN++;

  			if(sub==subN)
				{
					rangeBindAction( y, PXX_RANGE_CHECK ) ;
  			}		
			}
	  }
/*
 #endif // V2
*/
		break ;

		case M_SWITCHES :
			menuProcSwitches(sub) ;
		break ;
		
		case M_LIMITS :
      menuProcLimits(sub) ;
		break ;
		
		case M_SAFETY :
      menuProcSafetySwitches(sub) ;
		break ;
	}
	asm("") ;
}

uint8_t evalOffset(int8_t sub)
{
  uint8_t t_pgOfs = s_pgOfs ;
	int8_t x = sub-t_pgOfs ;
    if(sub<1) t_pgOfs=0;
    else if(x>(int8_t)6) t_pgOfs = sub-(int8_t)6;
    else if(x<(int8_t)(6-6)) t_pgOfs = sub ; //-(int8_t)6+6;
		return (s_pgOfs = t_pgOfs) ;
}


