/*
 * Current Author - Mike Blandford
 *
 * Previous Authors - Bertrand Songis <bsongis@gmail.com>, Bryan J.Rentoul (Gruvin) <gruvin@gmail.com> and Philip Moss
 *
 * Adapted from jeti.cpp code by Karl Szmutny <shadow@privy.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#include <stdint.h>
//#include <stdlib.h>
#include <string.h>

#include "ersky9x.h"
#include "frsky.h"
#include "myeeprom.h"
#include "drivers.h"
#include "pulses.h"
#include "audio.h"
#include "menus.h"
#ifdef PCBSKY
#include "AT91SAM3S4.h"
#endif
#if defined(PCBX12D) || defined(PCBX10)
#ifndef SIMU
#include "X12D/stm32f4xx.h"
#include "X12D/core_cm4.h"
#endif
#else
#ifndef SIMU
#ifndef PCBLEM1
#include "core_cm3.h"
#endif
#endif
#endif
#include "lcd.h"
#include "ff.h"
#include "maintenance.h"
#include "sound.h"
#include "mavlink.h"

void txmit( uint8_t c ) ;

//#define TEL_TYPE_FRSKY_HUB		0
//#define TEL_TYPE_FRSKY_SPORT	1
//#define TEL_TYPE_DSM					2
//#define TEL_TYPE_AFH					3
//#define TEL_TYPE_XFIRE				4
//#define TEL_TYPE_HITEC				5

uint8_t MultiId[4] ;
uint8_t RxLqi ;
uint8_t TxLqi ;

// Enumerate FrSky packet codes
#define LINKPKT         0xfe
#define USRPKT          0xfd
#define A11PKT          0xfc
#define A12PKT          0xfb
#define A21PKT          0xfa
#define A22PKT          0xf9
#define ALRM_REQUEST    0xf8
#define RSSIRXPKT       0xf7
#define RSSITXPKT       0xf6
#define RSSI_REQUEST		0xf1

#define START_STOP      0x7e
#define BYTESTUFF       0x7d
#define STUFF_MASK      0x20

#define PRIVATE					'M'

#ifdef SMALL
#define PRIVATE_BUFFER_SIZE	24
#else
#define PRIVATE_BUFFER_SIZE	36
#endif

// SPORT defines
#define DATA_FRAME         0x10

// Translate hub data positions
const uint8_t Fr_indices[] = 
{
	HUBDATALENGTH-1,
	TELEM_GPS_ALT | 0x80,
	FR_TEMP1,
	FR_RPM,
	FR_FUEL,
	FR_TEMP2,
	FR_CELL_V,
	FR_VCC,           // 0x07  Extra data for Mavlink via FrSky
	HUBDATALENGTH-1,
//	HUBDATALENGTH-1,HUBDATALENGTH-1,
	TELEM_GPS_ALTd,	// 9
/* Extra data 1 for Mavlink via FrSky */
	FR_HOME_DIR,      // 0x0A
	FR_HOME_DIST,     // 0x0B
	FR_CPU_LOAD,      // 0x0C
	FR_GPS_HDOP,      // 0x0D
	FR_WP_NUM,        // 0x0E
	FR_WP_BEARING,    // 0x0F
/* Extra data 1 for Mavlink via FrSky */
//	HUBDATALENGTH-1,HUBDATALENGTH-1,
//	HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,
	FR_ALT_BARO | 0x80,
	FR_GPS_SPEED | 0x80,
	FR_GPS_LONG | 0x80,
	FR_GPS_LAT | 0x80,
	FR_COURSE,		// 20
	FR_GPS_DATMON,
	FR_GPS_YEAR,
	FR_GPS_HRMIN,
	FR_GPS_SEC,
	FR_GPS_SPEEDd,
	FR_GPS_LONGd,
	FR_GPS_LATd,
	FR_COURSEd,		// 28
/* Extra data 2 for Mavlink via FrSky */
	FR_BASEMODE,
	FR_WP_DIST,
	FR_HEALTH,
	FR_MSG,
/* Extra data 2 for Mavlink via FrSky */
//	HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,
	FR_ALT_BAROd,
	FR_LONG_E_W,
	FR_LAT_N_S,
	FR_ACCX,
	FR_ACCY,
	FR_ACCZ,
	FR_VSPD,
	FR_CURRENT,		// 40
	FR_V_AMP | 0x80,
	FR_V_AMPd,
	FR_VOLTS,
	FR_AIRSPEED,
	HUBDATALENGTH-1		// 45
} ;

struct t_s6r S6Rdata ;

uint8_t TmOK ;

uint8_t AltitudeDecimals ;
uint8_t AltitudeZeroed = 0 ;
uint8_t GpsAltitudeDecimals ;
uint8_t VfasVoltageTimer ;
extern int16_t AltOffset ;
int16_t WholeAltitude ;
int16_t WholeGpsAltitude ;
uint16_t PixHawkCapacity ;
//uint8_t RssiSetTimer ;

#define FRSKY_SPORT_PACKET_SIZE		9
#define FLYSKY_TELEMETRY_LENGTH (2+7*4)
#define HITEC_TELEMETRY_LENGTH 		9

#define TELEMETRY_RX_PACKET_SIZE	128

uint8_t frskyRxBuffer[128];   // Receive buffer. 9 bytes (full packet), worst case 18 bytes with byte-stuffing (+1), 128 for XFIRE
uint8_t frskyTxBuffer[20];   // Ditto for transmit buffer
//uint8_t frskyTxBufferCount = 0;
//uint8_t FrskyRxBufferReady = 0;
uint8_t frskyStreaming = 0;
uint8_t frskyUsrStreaming = 0;
//uint8_t FrskyAlarmTimer = 200 ;		// Units of 10 mS
//uint8_t FrskyAlarmCheckFlag = 0 ;

FrskyData frskyTelemetry[4];
//FrskyData frskyRSSI[2];
//Frsky_current_info Frsky_current[2] ;

uint8_t frskyRSSIlevel[2] ;
uint8_t frskyRSSItype[2] ;

struct FrskyAlarm {
  uint8_t level;    // The alarm's 'urgency' level. 0=disabled, 1=yellow, 2=orange, 3=red
  uint8_t greater;  // 1 = 'if greater than'. 0 = 'if less than'
  uint8_t value;    // The threshold above or below which the alarm will sound
};
struct FrskyAlarm frskyAlarms[4];

uint8_t Frsky_user_state ;
uint8_t Frsky_user_stuff ;
uint8_t Frsky_user_id ;
uint8_t Frsky_user_lobyte ;
uint8_t Frsky_user_hibyte ;
uint8_t Frsky_user_ready ;

int16_t FrskyHubData[HUBDATALENGTH] ;  // All 38 words
uint8_t TelemetryDataValid[HUBDATALENGTH] ;  // All 38 words
uint16_t XjtVersion ;
struct t_hub_max_min FrskyHubMaxMin ;

//uint16_t FrskyVolts[12];
uint8_t FrskyBattCells[2] = {0,0} ;
uint16_t Frsky_Amp_hour_prescale ;

uint8_t TelemetryType ;
uint8_t SportStreamingStarted ;

#ifdef REVX
uint8_t JetiTxReady ;
uint16_t JetiTxChar ;
extern uint8_t JetiBuffer[] ; // 32 characters
extern uint8_t JetiIndex ;
#endif

uint8_t FrskyTelemetryType ;
uint8_t FrskyComPort ;
uint8_t numPktBytes = 0;
// Receive buffer state machine state defs
#define frskyDataIdle    0
#define frskyDataStart   1
#define frskyDataInFrame 2
#define frskyDataXOR     3
#define PRIVATE_COUNT		 4
#define PRIVATE_VALUE		 5
#ifndef SMALL
#define PRIVATE_TYPE		 6
#define PRIVATE_XCOUNT	 7
#endif

static uint8_t dataState = frskyDataIdle ;
static uint8_t A1Received = 0 ;

extern uint8_t RawLogging ;
void rawLogByte( uint8_t byte ) ;

uint8_t Private_count ;
#ifndef SMALL
uint8_t Private_type ;
#endif
uint8_t Private_position ;
uint8_t PrivateData[6] ;
//uint8_t ExtraPrivateData[16] ;
uint8_t InputPrivateData[PRIVATE_BUFFER_SIZE] ;

#ifndef SMALL
uint8_t DsmManCode[4] ;
#endif
uint16_t DsmABLRFH[6] ;
uint32_t LastDsmfades ;
uint16_t LastDsmFH[2] ;

#ifdef XFIRE
void processCrossfireTelemetryData( uint8_t data ) ;

enum CrossfireSensorIndexes {
  RX_RSSI1_INDEX,
  RX_RSSI2_INDEX,
  RX_QUALITY_INDEX,
  RX_SNR_INDEX,
  RX_ANTENNA_INDEX,
  RF_MODE_INDEX,
  TX_POWER_INDEX,
  TX_RSSI_INDEX,
  TX_QUALITY_INDEX,
  TX_SNR_INDEX,
  BATT_VOLTAGE_INDEX,
  BATT_CURRENT_INDEX,
  BATT_CAPACITY_INDEX,
  GPS_LATITUDE_INDEX,
  GPS_LONGITUDE_INDEX,
  GPS_GROUND_SPEED_INDEX,
  GPS_HEADING_INDEX,
  GPS_ALTITUDE_INDEX,
  GPS_SATELLITES_INDEX,
  ATTITUDE_PITCH_INDEX,
  ATTITUDE_ROLL_INDEX,
  ATTITUDE_YAW_INDEX,
  FLIGHT_MODE_INDEX,
  UNKNOWN_INDEX,
};

#endif

#if defined(VARIO)
stuct t_vario VarioData ;
#endif

void evalVario(int16_t altitude_bp, uint16_t altitude_ap)
{
#if defined(VARIO)
	stuct t_vario *vptr ;
	vptr = VarioData ;

  int32_t vptr->VarioAltitude_cm = (int32_t)altitude_bp * 100 + (altitude_bp > 0 ? altitude_ap : -altitude_ap) ;
  uint8_t varioAltitudeQueuePointer = vptr->VarioAltitudeQueuePointer + 1 ;
  if (varioAltitudeQueuePointer >= VARIO_QUEUE_LENGTH)
	{
    varioAltitudeQueuePointer = 0 ;
	}
  vptr->VarioAltitudeQueuePointer = varioAltitudeQueuePointer ;
  vptr->VarioSpeed -= vptr->VarioAltitudeQueue[varioAltitudeQueuePointer] ;
  vptr->VarioAltitudeQueue[varioAltitudeQueuePointer] = varioAltitude_cm - vptr->VarioAltitude_cm;
  vptr->VarioAltitude_cm = varioAltitude_cm;
  vptr->VarioSpeed += vptr->VarioAltitudeQueue[varioAltitudeQueuePointer] ;
#endif
}

void dsmTelemetryStartReceive()
{
#ifdef PCBSKY
 #ifndef REVX
	if ( g_model.frskyComPort == 0 )
	{
		if ( CaptureMode == CAP_COM1 )
		{
			uint16_t rxchar ;
			while ( ( rxchar = get_fifo128( &Com1_fifo ) ) != 0xFFFF )
			{
				// flush Rx buffer
			}	
		}
	}
 #endif // nREVX
#endif // PCBSKY
	
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
	if ( g_model.frskyComPort == 0 )
	{
		if ( CaptureMode == CAP_COM1 )
		{
			uint16_t rxchar ;
			while ( ( rxchar = get_fifo128( &Com1_fifo ) ) != 0xFFFF )
			{
				// flush Rx buffer
			}	
		}
	}
#endif // PCBX9D || PCB9XT
	
	numPktBytes = 0 ;
}

uint16_t convertRxv( uint16_t value )
{
	if ( ( FrskyTelemetryType != FRSKY_TEL_DSM ) && ( FrskyTelemetryType != FRSKY_TEL_AFH )
			 && ( FrskyTelemetryType != FRSKY_TEL_HITEC ) )		// DSM or AFHDS2 or Hitec telemetry 
	{
		value *= g_model.rxVratio ;
		value /= 255 ;
	}
 	return value ;
}

void storeTelemetryData( uint8_t index, uint16_t value ) ;

void store_indexed_hub_data( uint8_t index, uint16_t value )
{
	if ( index > 57 )
	{
		index -= 17 ;		// Move voltage-amp sensors							
	}									// 58->41, 59->42
	if ( index == 48 )
	{
		index = 39 ;	//FR_VSPD ;		// Move Vario							
	}
	if ( index == 57 )
	{
		index = 43 ;	//FR_VOLTS ;		// Move Oxsensor voltage
	}
	if ( index == 56 )
	{
		index = 44 ;	//FR_AIRSPEED
	}
	if ( index > sizeof(Fr_indices) )
	{
		index = 0 ;	// Use a discard item							
	}
  index = Fr_indices[index] & 0x7F ;
	storeTelemetryData( index, value ) ;
}


const uint8_t DestIndex[] = { FR_BASEMODE, FR_CURRENT, FR_AMP_MAH, FR_VOLTS, FR_FUEL, FR_RBOX_STATE, FR_CUST1, 
FR_CUST2, FR_CUST3, FR_CUST4, FR_CUST5, FR_CUST6, FR_AIRSPEED	 } ;

void store_telemetry_scaler( uint8_t index, int16_t value )
{
	if ( index )
	{
		if ( index == 5 )	// FUEL
		{
			if ( value < 0 )
			{
				value = 0 ;
			}
		}
		if ( index <= sizeof(DestIndex) )
		{
			storeTelemetryData( DestIndex[index-1], value ) ;
		}
	}
}

void store_cell_data( uint8_t battnumber, uint16_t cell )
{
	if ( battnumber < 12 )
	{
		cell = ( cell & 0x0FFF ) / 5 ;
//		FrskyVolts[battnumber] = cell ;
//		FrskyHubData[FR_CELL1+battnumber] = FrskyVolts[battnumber] ;
		uint32_t index ;
#ifdef BLOCKING
		uint32_t bit ;
		uint32_t offset ;
#endif
	  index = FR_CELL1+battnumber ;
#ifdef BLOCKING
		offset = index >> 5 ;
		bit = 1 << (index & 0x0000001F) ;
		if ( g_model.LogNotExpected[offset] & bit )
		{
			return ;
		}
#endif
		uint32_t scaling = 1000 + g_model.cellScalers[battnumber] ;
		scaling *= cell ;
		cell = scaling / 1000 ;
		FrskyHubData[index] = cell ;
		TelemetryDataValid[index] = 40 + g_model.telemetryTimeout ;
//		TelemetryDataValid[FR_CELLS_TOT] = 40 + g_model.telemetryTimeout ;
		TelemetryDataValid[FR_CELL_MIN] = 40 + g_model.telemetryTimeout ;
		if ( battnumber == 0 )
		{
			if ( FrskyHubData[FR_CELL_MIN] == 0 )
			{
				FrskyHubData[FR_CELL_MIN] = 450 ;
			}
			if ( cell < FrskyHubData[FR_CELL_MIN] )
			{
				FrskyHubData[FR_CELL_MIN] = cell ;
			}
		}
	}
}

void storeAltitude( int16_t value )
{
	FrskyHubData[FR_ALT_BARO] = value ;
	TelemetryDataValid[FR_ALT_BARO] = 25 + g_model.telemetryTimeout ;
	if ( !AltitudeZeroed )
	{
		AltOffset = -FrskyHubData[FR_ALT_BARO] ;
		if ( AltOffset )
		{
			AltitudeZeroed = 1 ;
		}
	}
}

void storeRSSI( uint8_t value )
{
//	if ( RssiSetTimer )
//	{
//		RssiSetTimer -= 1 ;
//	}
//	else
//	{
		frskyTelemetry[2].set( value, FR_RXRSI_COPY );	//FrskyHubData[] =  frskyTelemetry[2].value ;
//	}
}

uint8_t SbecCount ;
uint16_t SbecAverage ;

void storeTelemetryData( uint8_t index, uint16_t value )
{
#ifdef BLOCKING
	uint32_t bit ;
	uint32_t offset ;
	
	offset = index >> 5 ;
	bit = 1 << (index & 0x0000001F) ;
	if ( g_model.LogNotExpected[offset] & bit )
	{
		return ;
	}
#endif
	if ( index == FR_ALT_BARO )
	{
		value *= 10 ;
		if ( AltitudeDecimals )
		{
			WholeAltitude = value ;
			index = FR_TRASH ;
		}
		else
		{
			storeAltitude( value ) ;
		}
	}
	if ( index == FR_ALT_BAROd )
	{
		if ( AltitudeDecimals == 0 )
		{
			AltitudeZeroed = 0 ;
		}
		AltitudeDecimals |= 1 ;
		if ( ( value > 9 ) || ( value < -9 ) )
		{
			AltitudeDecimals |= 2 ;
		}
		if ( AltitudeDecimals & 2 )
		{
			value /= 10 ;			
		}
		storeAltitude( WholeAltitude + ((WholeAltitude >= 0) ? value : -value) ) ;
		index = FR_ALT_BARO ;	// For max/min
	}
	
	if ( index == FR_SPORT_ALT )
	{
		index = FR_ALT_BARO ;         // For max and min
		storeAltitude(  value ) ;
	}
	
	if ( index == FR_SPORT_GALT )
	{
		index = TELEM_GPS_ALT ;         // For max and min
		FrskyHubData[TELEM_GPS_ALT] = value ;
		TelemetryDataValid[TELEM_GPS_ALT] = 25 + g_model.telemetryTimeout ;
	}

	if ( index == TELEM_GPS_ALT )
	{
		value *= 10 ;
		WholeGpsAltitude = value ;
		index = FR_TRASH ;
	}
	if ( index == TELEM_GPS_ALTd )
	{
		GpsAltitudeDecimals |= 1 ;
		if ( ( value > 9 ) || ( value < -9 ) )
		{
			GpsAltitudeDecimals |= 2 ;
		}
		if ( GpsAltitudeDecimals & 2 )
		{
			value /= 10 ;			
		}
		index = TELEM_GPS_ALT ;	// For max/min
		value += WholeGpsAltitude ;
	}

	if ( index < HUBDATALENGTH )
	{
    if ( !g_model.FrSkyGpsAlt )         
    {
      if ( index != FR_ALT_BARO )
			{
				FrskyHubData[index] = value ;
				TelemetryDataValid[index] = 25 + g_model.telemetryTimeout ;
			}
    }                     
		else
		{
      if ( index != FR_ALT_BARO )
			{
			  FrskyHubData[index] = value ;           /* ReSt */
			}
      if ( index == TELEM_GPS_ALT )
      {
         storeAltitude( FrskyHubData[TELEM_GPS_ALT] ) ;      // Copy Gps Alt instead
         index = FR_ALT_BARO ;         // For max and min
      }
			TelemetryDataValid[index] = 25 + g_model.telemetryTimeout ;
		}
		
#if defined(VARIO)
		if ( index == FR_ALT_BARO )
		{
			evalVario( value, 0 ) ;
		}
#endif

		if ( index == FR_CURRENT )			// FAS current
		{
			if ( value > g_model.FASoffset )
			{
				value -= g_model.FASoffset ;
			}
			else
			{
				value = 0 ;
			}
			FrskyHubData[index] = value ;
			TelemetryDataValid[index] = 25 + g_model.telemetryTimeout ;
		}

		if ( index < HUBMINMAXLEN )
		{
			struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;
			
			int16_t value = FrskyHubData[index] ;
			
			if ( maxMinPtr->hubMax[index] < value )
			{	maxMinPtr->hubMax[index] = value ;
			}
			int16_t min = maxMinPtr->hubMin[index] ;
			if (!min || min > value)
   	  { maxMinPtr->hubMin[index] = value ;
			}	
		}
		if ( index == FR_CELL_V )			// Cell Voltage
		{
			// It appears the cell voltage bytes are in the wrong order
//  							uint8_t battnumber = ( FrskyHubData[6] >> 12 ) & 0x000F ;
  		uint8_t battnumber = ((uint8_t)value >> 4 ) & 0x000F ;
  		if (FrskyBattCells[0] < battnumber+1)
			{
 				if (battnumber+1>=6)
				{
  				FrskyBattCells[0]=6;
 					if (battnumber+1 <= 12 )
					{
  					FrskyBattCells[1]=battnumber+1-6;
					}
  			}
				else
				{
  				FrskyBattCells[0] = battnumber+1 ;
  			}
  		}
			if ( battnumber < 12 )
			{
				store_cell_data( battnumber, ( ( value & 0x0F ) << 8 ) + (value >> 8) ) ;
			}

			uint8_t totalCells ;
			for ( totalCells = 0 ; totalCells < 12 ; )
			{
				if ( TelemetryDataValid[FR_CELL1+totalCells] )
				{
					totalCells += 1 ;
				}
				else
				{
					break ;
				}
			}
  		if (FrskyBattCells[0] <= totalCells )
			{
 				if ( totalCells >= 5 )
				{
  				FrskyBattCells[0] = 6 ;
 					if ( totalCells <= 11 )
					{
  					FrskyBattCells[1] = totalCells - 5 ;
					}
  			}
				else
				{
  				FrskyBattCells[0] = totalCells + 1 ;
					FrskyBattCells[1] = 0 ;
  			}
  		}
		}
		if ( index == FR_RPM )			// RPM
		{
			uint32_t x ;

			x = value ;
			x *= 60 ;
			if ( g_model.numBlades == 0 )
			{
				g_model.numBlades = 1 ;
			}
			FrskyHubData[FR_RPM] = x / g_model.numBlades ;
//			TelemetryDataValid[FR_RPM] = 25 + g_model.telemetryTimeout ;
//			FrskyHubData[FR_RPM] = x / ( g_model.Mavlink == 0 ? g_model.numBladesb : g_model.numBlades * 30) ;
		}
		if ( index == FR_V_AMPd )
		{
			FrskyHubData[FR_VOLTS] = (FrskyHubData[FR_V_AMP] * 10 + value) * 21 / 11 ;
			TelemetryDataValid[FR_VOLTS] = 25 + g_model.telemetryTimeout ;
		}
		if ( index == FR_SBEC_CURRENT )
		{
			SbecAverage += value ;
			if ( ++SbecCount >= 4 )
			{
				SbecCount = 0 ;
				value = ( SbecAverage + 3 ) >> 2 ;
				FrskyHubData[index] = value ;
				SbecAverage = 0 ;
			}
		}
	}	
}


void handleUnknownId( uint16_t id, uint32_t value )
{
	uint32_t i ;
	for ( i = 0 ; i < g_model.extraSensors ; i += 1 )
	{
		if ( g_model.extraId[i].id == id )
		{
			// do something with it?
			store_telemetry_scaler( g_model.extraId[i].dest, value ) ;
			return ;
		}
	}

	if ( g_model.extraSensors < NUMBER_EXTRA_IDS )
	{
		g_model.extraId[g_model.extraSensors++].id = id ;
	}
}


#ifdef DEBUG_MAVLINK
uint16_t Mcounter ;
#endif

void frsky_proc_user_byte( uint8_t byte )
{
#ifdef DEBUG_MAVLINK
	if ( byte == 0x5E )
	{
		if ( ++Mcounter >= 6 )
		{
			crlf() ;
			Mcounter = 0 ;
		}
	}
	p2hex( byte ) ;
#endif

	if (g_model.FrSkyUsrProto == 0)  // FrSky Hub
	{
	
  	if ( Frsky_user_state == 0 )
		{ // Waiting for 0x5E
			if ( byte == 0x5E )
			{
				Frsky_user_state = 1 ;			
				if ( Frsky_user_ready )
				{
					Frsky_user_ready = 0 ;
					store_indexed_hub_data( Frsky_user_id, ( Frsky_user_hibyte << 8 ) | Frsky_user_lobyte ) ;
				}
			}
			else
			{
				Frsky_user_ready = 0 ;
			}
		}
		else
		{ // In a packet
			if ( byte == 0x5E )
			{ // 
				Frsky_user_state = 1 ;			
			}
			else
			{
				if ( byte == 0x5D )
				{
					Frsky_user_stuff = 1 ;  // Byte stuffing active
				}
				else
				{
					if ( Frsky_user_stuff )
					{
						Frsky_user_stuff = 0 ;
						byte ^= 0x60 ;  // Unstuff
					}
  	      if ( Frsky_user_state == 1 )
					{
					  Frsky_user_id	= byte ;
						Frsky_user_state = 2 ;
					}
  	      else if ( Frsky_user_state == 2 )
					{
					  Frsky_user_lobyte	= byte ;
						Frsky_user_state = 3 ;
					}
					else
					{
						Frsky_user_hibyte = byte ;
						Frsky_user_ready = 1 ;
						Frsky_user_state = 0 ;
					}
				}
			}		 
		}
	}
	else // if (g_model.FrSkyUsrProto == 1)  // WS How High
	{
    if ( frskyUsrStreaming < (FRSKY_USR_TIMEOUT10ms - 10))  // At least 100mS passed since last data received
		{
			Frsky_user_lobyte = byte ;
		}
		else
		{
			int16_t value ;
			value = ( byte << 8 ) + Frsky_user_lobyte ;
			storeTelemetryData( FR_ALT_BARO, value ) ;	 // Store altitude info
#if defined(VARIO)
			evalVario( value, 0 ) ;
#endif

		}				
	}
}

static void frskyPushValue(uint8_t & i, uint8_t value);

/*
   Called from somewhere in the main loop or a low prioirty interrupt
   routine perhaps. This funtcion processes Fr-Sky telemetry data packets
   assembled byt he USART0_RX_vect) ISR function (below) and stores
   extracted data in global variables for use by other parts of the program.

   Packets can be any of the following:

    - A1/A2/RSSI telemtry data
    - Alarm level/mode/threshold settings for Ch1A, Ch1B, Ch2A, Ch2B
    - User Data packets
*/
void setTxLqi( uint8_t value )
{
	TxLqi = ( ( TxLqi * 7 ) + value + 4 ) / 8 ;	// Multi only
}

uint8_t LinkAveCount ;

uint16_t UserErrorCount ;
uint8_t UserLastSequence ;
uint8_t UserLastCount ;

#if defined(PCBT16)
uint8_t MultiFrskyTelemetry ;
#endif	 

// Leading 0x7E not in packet
void processFrskyPacket(uint8_t *packet)
{
	if ( numPktBytes != 9 )
	{
		return ;
	}

#if defined(PCBT16)
	if ( MultiFrskyTelemetry )
	{
		return ;
	}
#endif	 
	 
  // What type of packet?
  switch (packet[0])
  {
    case A22PKT:
    case A21PKT:
    case A12PKT:
    case A11PKT:
      {
        struct FrskyAlarm *alarmptr ;
        alarmptr = &frskyAlarms[(packet[0]-A22PKT)] ;
        alarmptr->value = packet[1];
        alarmptr->greater = packet[2] & 0x01;
        alarmptr->level = packet[3] & 0x03;
      }
//  		frskyStreaming = FRSKY_TIMEOUT10ms; // reset counter only if valid frsky packets are being detected
      break;
    case LINKPKT: // A1/A2/RSSI values
//			LinkAveCount += 1 ;
      frskyTelemetry[0].set(packet[1], FR_A1_COPY ); //FrskyHubData[] =  frskyTelemetry[0].value ;
      frskyTelemetry[1].set(packet[2], FR_A2_COPY ); //FrskyHubData[] =  frskyTelemetry[1].value ;
      frskyTelemetry[2].set(packet[3], FR_RXRSI_COPY );	//FrskyHubData[] =  frskyTelemetry[2].value ;
//			RssiSetTimer = 30 ;
      frskyTelemetry[3].set(packet[4] / 2, FR_TXRSI_COPY ); //FrskyHubData[] =  frskyTelemetry[3].value ;
//			RxLqi = ( ( RxLqi * 7 ) + packet[5] + 4 ) / 8 ;	// Multi only
			setTxLqi( packet[6] ) ;
//			if ( LinkAveCount > 15 )
//			{
//				LinkAveCount = 0 ;
//			}
//      frskyRSSI[0].set(packet[3]);
//      frskyRSSI[1].set(packet[4] / 2);
  		frskyStreaming = FRSKY_TIMEOUT10ms; // reset counter only if valid frsky packets are being detected
      break;

		case RSSIRXPKT :
			frskyRSSIlevel[1] = packet[1] ;
			frskyRSSItype[1] = packet[3] ;
		break ;

		case RSSITXPKT :
			frskyRSSIlevel[0] = packet[1] ;
			frskyRSSItype[0] = packet[3] ;
		break ;

    case USRPKT: // User Data packet
    {
			if ( packet[2] != ( (UserLastSequence+1) & 0x1F) )
			{
				if ( ( UserLastCount != 6 ) || ( packet[2] != UserLastSequence ) )
				{
					UserErrorCount += 1 ;
				}
			}
			UserLastSequence = packet[2] ;
			UserLastCount = packet[1] ;
			uint8_t i, j ;
			i = ( packet[1] & 0x07) + 3 ;  // User bytes end
			j = 3 ;              // Index to user bytes
			while ( j < i )
			{
				frsky_proc_user_byte( packet[j] ) ;
      	frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
				j += 1 ;
			}
    }	
		  frskyStreaming = FRSKY_TIMEOUT10ms; // reset counter only if valid frsky packets are being detected
    break;
  }

//  FrskyRxBufferReady = 0;
}


//0[00] 22(0x16)
//1[01] 00
//2[02] Altitude LSB (Decimal) //In 0.1m
//3[03] Altitude MSB (Decimal)
//4[04] 1/100th of a degree second latitude (Decimal) (XX YY.SSSS)
//5[05] degree seconds latitude (Decimal)
//6[06] degree minutes latitude (Decimal)
//7[07] degrees latitude (Decimal)
//8[08] 1/100th of a degree second longitude (Decimal) (XX YY.SSSS)
//9[09] degree seconds longitude (Decimal)
//10[0A] degree minutes longitude (Decimal)
//11[0B] degrees longitude (Decimal)
//12[0C] Heading LSB (Decimal)
//13[0D] Heading MSB (Decimal) Divide by 10 for Degrees
//14[0E] Unknown
//15[0F] First bit for latitude: 1=N(+), 0=S(-);
//Second bit for longitude: 1=E(+), 0=W(-);
//Third bit for longitude over 99 degrees: 1=+-100 degrees

//0[00] 23(0x17)
//1[01] 00
//2[02] Speed LSB (Decimal)
//3[03] Speed MSB (Decimal) Divide by 10 for Knots. Multiply by 0.185 for Kph and 0.115 for Mph
//4[04] UTC Time LSB (Decimal) 1/100th sec. (HH:MM:SS.SS)
//5[05] UTC Time (Decimal) = SS
//6[06] UTC Time (Decimal) = MM
//7[07] UTC Time MSB (Decimal) = HH
//8[08] Number of Sats (Decimal)
//9[09] Altitude in 1000m (Decimal) Altitude = Value * 10000 + Altitude(0x16) (in 0.1m)
//10[0A]-15[0F] Unused (But contains Data left in buffer)


//===============================================================

//Data type = 0x03 High Current Sensor

#define DSM_AMPS			3

//0 [00] 03
//1 [01] 00
//2 [02] MSB (Hex) //16bit signed integer
//3 [03] LSB (Hex) //In 0.196791A
//4 [04] 00
//5 [05] 00
//6 [06] 00
//7 [07] 00
//8 [08] 00
//9 [09] 00
//10 [0A] 00
//11 [0B] 00
//12 [0C] 00
//13 [0D] 00
//14 [0E] 00
//15 [0F] 00

//===============================================================

//Data type = 0x0A PowerBox Sensor

#define DSM_PBOX			0x0A

//0 [00] 0x0A
//1 [01] 00
//2 [02] V1 MSB (Hex)
//3 [03] V1 LSB (Hex) //In 0.01V
//4 [04] V2 MSB (Hex)
//5 [05] V2 LSB (Hex) //In 0.01V
//6 [06] Cap1 MSB (Hex)
//7 [07] Cap1 LSB (Hex) //In 1mAh
//8 [08] Cap2 MSB (Hex)
//9 [09] Cap2 LSB (Hex) //In 1mAh
//10 [0A] 00
//11 [0B] 00
//12 [0C] 00
//13 [0D] 00
//14 [0E] 00
//15 [0F] Alarm // The fist bit is alarm V1, the second V2, the third Capacity 1, the 4th capacity 2.

//===============================================================

//Data type = 0x11 AirSpeed Sensor

#define DSM_AIRSPEED			17

//0[00] 17(0x11)
//1[01] 00
//2[02] Speed MSB (Hex)
//3[03] Speed LSB (Hex) //In 1 km/h
//4[04] Unknown
//5[05] Unknown
//6[06] Unknown
//7[07] Unknown
//8[08] Unknown
//9[09] Unknown
//10[0A] Unknown
//11[0B] Unknown
//12[0C] Unknown
//13[0D] Unknown
//14[0E] Unknown
//15[0F] Unknown

//===============================================================

//Data type = 0x12 Altimeter Sensor

#define DSM_ALT			18

//0[00] 18(0x12)
//1[01] 00
//2[02] Altitude MSB (Hex)
//3[03] Altitude LSB (Hex) 16bit signed integer, in 0.1m
//4[04] Max Altitude MSB (Hex)
//5[05] Max Altitude LSB (Hex) 16bit signed integer, in 0.1m
//6[05] Unknown
//7[07] Unknown
//8[08] Unknown
//9[09] Unknown
//10[0A] Unknown
//11[0B] Unknown
//12[0C] Unknown
//13[0D] Unknown
//14[0E] Unknown
//15[0F] Unknown

#define DSM_VARIO		64

//0[00] 64(0x40)
//1[01] 00
//2[02] Altitude MSB (Hex)
//3[03] Altitude LSB (Hex) 16bit signed integer, in 0.1m
//4[04] Vertical speed MSB (Hex)
//5[05] Vertical speed LSB (Hex) 16bit signed integer, in 0.1m


//UINT8 identifier; // Source device = 0x40
//UINT8 sID; // Secondary ID
//INT16 altitude; // .1m increments
//INT16 delta_0250ms, // delta last 250ms, 0.1m/s increments
//delta_0500ms, // delta last 500ms, 0.1m/s increments
//delta_1000ms, // delta last 1.0 seconds
//delta_1500ms, // delta last 1.5 seconds
//delta_2000ms, // delta last 2.0 seconds
//delta_3000ms; // delta last 3.0 seconds



//===============================================================

//Data type = 0x14 Gforce Sensor

#define DSM_GFORCE			20


//0[00] 20(0x14)
//1[01] 00
//2[02] x MSB (Hex, signed integer)
//3[03] x LSB (Hex, signed integer) //In 0.01g
//4[04] y MSB (Hex, signed integer)
//5[05] y LSB (Hex, signed integer) //In 0.01g
//6[06] z MSB (Hex, signed integer)
//7[07] z LSB (Hex, signed integer) //In 0.01g
//8[08] x max MSB (Hex, signed integer)
//9[09] x max LSB (Hex, signed integer) //In 0.01g
//10[0A] y max MSB (Hex, signed integer)
//11[0B] y max LSB (Hex, signed integer) //In 0.01g
//12[0C] z max MSB (Hex, signed integer)
//13[0D] z max LSB (Hex, signed integer) //In 0.01g
//14[0E] z min MSB (Hex, signed integer)
//15[0F] z min LSB (Hex, signed integer) //In 0.01g

//===============================================================

//Data type = 0x15 JetCat Sensor

//0[00] 21(0x15)
//1[01] 00
//2[02] Status
//3[03] Throttle //up to 159% (the upper nibble is 0-f, the lower nibble 0-9)
//4[04] Pack_Volt LSB (Decimal)
//5[05] Pack_Volt MSB (Decimal) //In 0.01V
//6[06] Pump_Volt LSB (Decimal)
//7[07] Pump_Volt MSB (Decimal) //In 0.01V
//8[08] RPM LSB (Decimal) //Up to 999999rpm
//9[09] RPM Mid (Decimal)
//10[0A] RPM MSB (Decimal)
//11[0B] 00
//12[0C] TempEGT (Decimal) LSB
//13[0D] TempEGT (Decimal) MSB //used only lover nibble, up to 999°C
//14[0E] Off_Condition
//15[0F] 00

//===============================================================

//Data type = 7E{TM1000} or FE{TM1100}

#define DSM_VTEMP1	0x7E
#define DSM_VTEMP2	0xFE

//0[00] 7E or FE
//1[01] 00
//2[02] RPM MSB (Hex)
//3[03] RPM LSB (Hex) //RPM = 120000000 / number_of_poles(2, 4, ... 32) / gear_ratio(0.01 - 30.99) / Value
//4[04] Volt MSB (Hex)
//5[05] Volt LSB (Hex) //In 0.01V
//6[06] Temp MSB (Hex)
//7[07] Temp LSB (Hex) //Value (Decimal) is in Fahrenheit, for Celsius (Value (Decimal) - 32) * 5) / 9)
//8[08] Unknown
//9[09] Unknown
//10[0A] Unknown
//11[0B] Unknown
//12[0C] Unknown
//13[0D] Unknown	// Assan when 7E type is Rx RSSI
//14[0E] Unknown
//15[0F] Unknown

//===============================================================

//Data type = 7F{TM1000} or FF{TM1100}

#define DSM_STAT1	0x7F
#define DSM_STAT2	0xFF

//0[00] 7F or FF
//1[01] 00
//2[02] A MSB (Hex)
//3[03] A LSB (Hex)
//4[04] B MSB (Hex)
//5[05] B LSB (Hex)
//6[06] L MSB (Hex)
//7[07] L LSB (Hex) //0xFFFF = NC (not connected)
//8[08] R MSB (Hex)
//9[09] R LSB (Hex) //0xFFFF = NC (not connected)
//10[0A] Frame loss MSB (Hex)
//11[0B] Frame loss LSB (Hex)
//12[0C] Holds MSB (Hex)
//13[0D] Holds LSB (Hex)
//14[0E] Receiver Volts MSB (Hex) //In 0.01V
//15[0F] Receiver Volts LSB (Hex)

//answer from TX module:
//header: 0xAA
//0x00 - no telemetry
//0x01 - telemetry packet present and telemetry block (TM1000, TM1100)
//answer in 16 bytes body from
//http://www.deviationtx.com/forum/protocol-development/1192-dsm-telemetry-support?start=60
//..
//0x1F - this byte also used as RSSI signal for receiving telemetry
//block. readed from CYRF in a TX module
//0x80 - BIND packet answer, receiver return his type
//aa bb cc dd ee......
//aa ORTX_USExxxx from main.h
//bb
//cc - max channel, AR6115e work only in 6 ch mode (0xA2)
//0xFF - sysinfo
//aa.aa.aa.aa - CYRF manufacturer ID
//bb.bb - firmware version
//	if(ortxRxBuffer[1] == 0x80){//BIND packet answer
//		ortxMode = ortxRxBuffer[2];
//		if(ortxMode & ORTX_USE_DSMX)
//			g_model.ppmNCH = DSM2_DSMX;
//		else
//			g_model.ppmNCH = DSM2only;
//		//ortxTxBuffer[3];//power
//		g_model.unused1[0] = ortxRxBuffer[4];//channels number

//===============================================================

#ifndef SMALL
uint8_t DsmDebug[20] ;
uint8_t DsmControlDebug[20] ;
uint16_t DsmControlCounter ;
uint16_t DsmDbgCounters[20] ;
extern uint16_t DsmFrameRequired ;
#endif
uint16_t TelRxCount ;


void processDsmPacket(uint8_t *packet, uint8_t byteCount)
{
	int32_t ivalue ;
	uint32_t type ;

#ifndef SMALL
	DsmDbgCounters[10] = packet[0] ;
	DsmDbgCounters[11] = packet[1] ;
	DsmDbgCounters[12] = packet[2] ;
	DsmDbgCounters[13] = packet[3] ;
#endif

	{
		
		packet += 1 ;			// Skip the 0xAA
		type = *packet++ ;
	}

	if ( type && (type < 0x20) )
	{
		
//   	frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid packets are being detected
//  	frskyStreaming = FRSKY_TIMEOUT10ms; // reset counter only if valid packets are being detected

   	frskyUsrStreaming = 255 ; // reset counter only if valid packets are being detected
  	frskyStreaming = 255 ; // reset counter only if valid packets are being detected
		
#ifndef SMALL
		// Debug code
		uint16_t temp = *packet ;
		temp |= packet[1] << 8 ;
		if ( temp == DsmFrameRequired )
		{
			uint8_t *p = packet ;
			uint8_t i ;
			DsmDebug[0] = byteCount ;
			for ( i = 1 ; i < 17 ; i += 1 )
			{
				DsmDebug[i] = *p++ ;
			}
		}
		// End debug code
#endif
		 
		ivalue = (int16_t) ( (packet[2] << 8 ) | packet[3] ) ;
		// Telemetry
		if ( g_model.dsmAasRssi )
		{
    	frskyTelemetry[3].set(type, FR_TXRSI_COPY );	// TSSI
		}
		else
		{
    	frskyTelemetry[2].set(type, FR_RXRSI_COPY );	// RSSI
		}
		switch ( *packet )
		{
			
			case DSM_ALT :
			case DSM_VARIO :
#ifndef SMALL
				DsmDbgCounters[0] += 1 ;
#endif
				storeAltitude( ivalue ) ;
				if ( FrskyHubMaxMin.hubMax[FR_ALT_BARO] < ivalue )
				{	FrskyHubMaxMin.hubMax[FR_ALT_BARO] = ivalue ;
				}
				if ( *packet == DSM_VARIO )
				{
					ivalue = g_model.dsmVario * 2 + 4 ;
					ivalue = (int16_t) ( (packet[ivalue] << 8 ) | packet[ivalue+1] ) ;
					storeTelemetryData( FR_VSPD, ivalue*10 ) ;
				}
			break ;
		
			case DSM_AMPS :
#ifndef SMALL
				DsmDbgCounters[1] += 1 ;
#endif
				ivalue *= 2015 ;
				ivalue /= 1024 ;
				storeTelemetryData( FR_CURRENT, ivalue ) ;	// Handles FAS Offset
//				FrskyHubData[FR_CURRENT] = ivalue ;
			break ;

			case DSM_PBOX :
#ifndef SMALL
				DsmDbgCounters[2] += 1 ;
#endif
				storeTelemetryData( FR_VOLTS, (uint16_t)ivalue / 10 ) ;	// Handles FAS Offset
//				FrskyHubData[FR_VOLTS] = (uint16_t)ivalue / 10 ;
				ivalue = (int16_t) ( (packet[6] << 8 ) | packet[7] ) ;
				storeTelemetryData( FR_AMP_MAH, ivalue ) ;
// 				FrskyHubData[FR_AMP_MAH] = ivalue ;
			break ;

			case DSM_AIRSPEED :
#ifndef SMALL
				DsmDbgCounters[3] += 1 ;
#endif
				// airspeed in km/h
			break ;

			case DSM_GFORCE :
#ifndef SMALL
				DsmDbgCounters[4] += 1 ;
#endif
				// check units (0.01G)
				storeTelemetryData( FR_ACCX, ivalue ) ;
//				FrskyHubData[FR_ACCX] = ivalue ;
				ivalue = (int16_t) ( (packet[4] << 8 ) | packet[5] ) ;
				storeTelemetryData( FR_ACCY, ivalue ) ;
//				FrskyHubData[FR_ACCY] = ivalue ;
				ivalue = (int16_t) ( (packet[6] << 8 ) | packet[7] ) ;
				storeTelemetryData( FR_ACCZ, ivalue ) ;
//				FrskyHubData[FR_ACCZ] = ivalue ;
			break ;
			
			case DSM_VTEMP1 :
			case DSM_VTEMP2 :
#ifndef SMALL
				DsmDbgCounters[5] += 1 ;
#endif
				// RPM
				if ( (uint16_t)ivalue == 0xFFFF )
				{
					ivalue = 0 ;
				}
				else
				{
					ivalue = 120000000L / g_model.numBlades / (uint16_t)ivalue ;
				}
				storeTelemetryData( FR_RPM, ivalue ) ;
//				FrskyHubData[FR_RPM] = ivalue ;
				// volts
				ivalue = (int16_t) ( (packet[4] << 8 ) | packet[5] ) ;
//				FrskyHubData[FR_A2_COPY] = ivalue ;
				storeTelemetryData( FR_VOLTS, (uint16_t)ivalue / 10 ) ;
//				FrskyHubData[FR_VOLTS] = (uint16_t)ivalue / 10 ;
				// temp
				ivalue = (int16_t) ( (packet[6] << 8 ) | packet[7] ) ;
				storeTelemetryData( FR_TEMP1, (ivalue-32)*5/9 ) ;
//				FrskyHubData[FR_TEMP1] = (ivalue-32)*5/9 ;

			break ;

			case DSM_STAT1 :
			case DSM_STAT2 :
#ifndef SMALL
				DsmDbgCounters[6] += 1 ;
#endif
				 
				if ( g_model.dsmAasRssi )
				{
					if ( ivalue < 1000 )
					{
   					frskyTelemetry[2].set(ivalue, FR_RXRSI_COPY ) ;	// RSSI
//						RssiSetTimer = 30 ;
					}
				}
				DsmABLRFH[0] = ivalue + 1 ;
				ivalue = (int16_t) ( (packet[4] << 8 ) | packet[5] ) ;
				DsmABLRFH[1] = ivalue + 1 ;
				ivalue = (int16_t) ( (packet[6] << 8 ) | packet[7] ) ;
				DsmABLRFH[2] = ivalue + 1 ;
				ivalue = (int16_t) ( (packet[8] << 8 ) | packet[9] ) ;
				DsmABLRFH[3] = ivalue + 1 ;
				ivalue = (int16_t) ( (packet[10] << 8 ) | packet[11] ) ;
				DsmABLRFH[4] = ivalue + 1 ;
				ivalue = (int16_t) ( (packet[12] << 8 ) | packet[13] ) ;
				DsmABLRFH[5] = ivalue + 1 ;
				ivalue = (int16_t) ( (packet[14] << 8 ) | packet[15] ) ;
				{
					uint32_t x = (uint16_t)ivalue ;
//					x *= 128 ;
//					x /= 717 ;		// was 1155 ;
					storeTelemetryData( FR_RXV, x / 10 ) ;
				}
			break ;

			default :
#ifndef SMALL
				DsmDbgCounters[7] += 1 ;
				DsmDebug[17] = *packet ;
#endif
			break ;
		}
	}
	else if ( type == 0x80 )
	{
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
		if ( g_model.Module[1].protocol == PROTO_MULTI )
#else
		if ( ( g_model.Module[0].protocol == PROTO_MULTI ) || ( g_model.Module[1].protocol == PROTO_MULTI ) )
#endif
		{
			dsmBindResponse( *(packet+6), *(packet+5) ) ;
		}
		else
		{
			dsmBindResponse( *packet, *(packet+2) ) ;
		}
#ifndef SMALL
		// Debug code
		uint8_t i ;
		DsmControlDebug[0] = byteCount ;
		for ( i = 1 ; i < 17 ; i += 1 )
		{
			DsmControlDebug[i] = *packet++ ;
		}
		DsmControlCounter += 1 ;
		// End debug code
#endif
	}
	else if ( type == 0xFF )
	{
#ifndef SMALL
		// Debug code
		{	
			uint8_t i ;
			uint8_t *p = packet ;
			DsmControlDebug[0] = byteCount ;
			for ( i = 1 ; i < 17 ; i += 1 )
			{
				DsmControlDebug[i] = *p++ ;
			}
		}
		DsmControlCounter += 1 ;
		// End debug code
#endif
		 
#ifndef SMALL
		DsmManCode[0] = *packet++ ;
		DsmManCode[1] = *packet++ ;
		DsmManCode[2] = *packet++ ;
		DsmManCode[3] = *packet ;
#endif
	}
	else if (type == 0 )
	{
		{
			if ( !g_model.dsmAasRssi )
			{
    		frskyTelemetry[2].set(type, FR_RXRSI_COPY ) ;	// RSSI
//				RssiSetTimer = 30 ;
			}
		}
	}


//  dataState = frskyDataIdle ; // Done after call

}

// Full telemetry ( after 0xAA ) 
// packet[0] = TX RSSI value
// packet[1] = TX LQI value
// packet[2] = frame number
// packet[3-7] telemetry data
void processHitecPacket( uint8_t *packet )
{
	int32_t ivalue ;

  frskyStreaming = 255 ; // reset counter only if valid packets are being detected
	packet += 1 ;		// skip 0xAA 
// RSSI is signed 8-bit value	
	ivalue = (int8_t)packet[0] + 128 ;
	frskyTelemetry[3].set( ivalue/2, FR_TXRSI_COPY );	// RSSI
	setTxLqi( packet[1] ) ;			// packet[1] is  TX_LQI for MULTI
	switch ( packet[2] )
	{
		case 0 :
			ivalue = (int16_t) ( (packet[6] << 8 ) | packet[7] ) ;
			ivalue *= 10 ;
			ivalue /= 28 ;
			storeTelemetryData( FR_RXV, ivalue ) ;
		break ;
		
		case 0x11 :
			ivalue = (int16_t) ( (packet[6] << 8 ) | packet[7] ) ;
			ivalue *= 10 ;
			ivalue /= 28 ;
			storeTelemetryData( FR_RXV, ivalue ) ;
		break ;

		case 0x13 :
			storeTelemetryData( FR_TEMP2, packet[7]-40 ) ;
		break ;

		case 0x14 :
			
			storeTelemetryData( FR_TEMP1, packet[7]-40 ) ;
		break ;

		case 0x15 :
			storeTelemetryData( FR_FUEL, packet[3] ) ;
			ivalue = (int16_t) ( (packet[5] << 8 ) | packet[4] ) ;
			storeTelemetryData( FR_RPM, ivalue ) ;
			ivalue = (int16_t) ( (packet[7] << 8 ) | packet[6] ) ;
			storeTelemetryData( FR_CUST3, ivalue ) ;
		break ;

		case 0x17 :
			storeTelemetryData( FR_CUST1, packet[6]-40 ) ;
			storeTelemetryData( FR_CUST2, packet[7]-40 ) ;
		break ;

		case 0x18 :
			ivalue = (int16_t) ( (packet[3] << 8 ) | packet[4] ) ;
			storeTelemetryData( FR_VOLTS, ivalue ) ;
			ivalue = (int16_t) ( (packet[5] << 8 ) | packet[6] ) ;
			ivalue *= 10 ;
			ivalue /= 14 ;
			storeTelemetryData( FR_CURRENT, ivalue ) ;
		break ;

	}
}


void processTrainerPacket( uint8_t *packet )
{
	uint8_t numChannels ;
	uint32_t inputbitsavailable = 0 ;
	uint32_t i ;
	uint32_t inputbits = 0 ;
	uint8_t sum ;
	int16_t *pulses = g_ppmIns ;
	sum = 0 ;
	packet += 4 ;
	numChannels = *packet++ ;
	for ( i = 0 ; i < numChannels ; i += 1 )
	{
		while ( inputbitsavailable < 11 )
		{
			uint8_t temp ;
			temp = *packet++ ;
			if ( i < 4 )
			{
				sum |= temp ;
			}
			inputbits |= temp << inputbitsavailable ;
			inputbitsavailable += 8 ;
		}
		if ( pulses )
		{
			*pulses++ = ( (int32_t)( inputbits & 0x7FF ) - 0x400 ) * 5 / 8 ;
		}
		inputbitsavailable -= 11 ;
		inputbits >>= 11 ;
	}

	if ( numChannels )
	{
		ppmInValid = 100 ;
	}
}

#ifndef DISABLE_SPORT
//#ifndef REVX
static bool checkSportPacket( uint8_t *packet )
{
//#ifdef PCBT12
//	return 0 ;
//#endif
//	uint8_t *packet = frskyRxBuffer ;
  uint16_t crc = 0 ;
  for ( uint8_t i=1; i<FRSKY_SPORT_PACKET_SIZE; i++)
	{
    crc += packet[i]; //0-1FF
    crc += crc >> 8; //0-100
    crc &= 0x00ff;
  }
  return (crc == 0x00ff) ;
}


uint8_t ARDUPtype ;

#if defined(LUA) || defined(BASIC)
static void postSportToScript( uint8_t *packet )
{
	if ( fifo128Space( &Script_fifo ) >= 8 )
	{
		uint32_t i ;
		for ( i = 0 ; i < 8 ; i += 1 )
		{
			put_fifo128( &Script_fifo, *packet++ ) ;
		}
	}
}
#endif


// The leading 0x7E is not in the packet, first byte is physical ID
void processSportData( uint8_t *packet, uint32_t receiver )
{
  uint8_t  prim   = packet[1];
#if defined(LUA) || defined(BASIC)
	if ( ( prim == 0x32 ) || ( (packet[3] & 0xF0) == 0x50 ) ) // || ( (packet[3] & 0xF0) == 0x10 ) )
	{
		postSportToScript( packet ) ;
	}
#endif

	if ( ( prim == DATA_FRAME ) || ( prim == 0x32 ) )
	{
		prim = packet[0] & 0x1F ;		// Sensor ID
		if ( packet[3] == 0xF1 )
		{ // Receiver specific
			uint8_t value = packet[4] ;
			if ( packet[2] == 1 )		// just RSSI
			{
				if ( SportStreamingStarted )
				{
					if ( value )
					{
						frskyStreaming = FRSKY_TIMEOUT10ms * 3 ; // reset counter only if valid frsky packets are being detected
					}
				}
				else
				{
					if ( !( value == 0 ) )
					{
						SportStreamingStarted = 1 ;
						frskyStreaming = FRSKY_TIMEOUT10ms * 3 ; // reset counter only if valid frsky packets are being detected
					}
				}
			}
			switch ( packet[2] )
			{
				case 1 :
	    		frskyTelemetry[2].set(value, FR_RXRSI_COPY );	//FrskyHubData[] =  frskyTelemetry[2].value ;
					setTxLqi( packet[7] ) ;			// packet[7] is  TX_LQI for MULTI
//					RssiSetTimer = 30 ;
				break ;

				case 2 :
			    frskyTelemetry[0].set(value, FR_A1_COPY ); //FrskyHubData[] =  frskyTelemetry[0].value ;
					A1Received = 1 ;
				break ;
				case 4 :		// Battery from X8R
					if ( A1Received == 0 )
					{
			      frskyTelemetry[0].set(value, FR_A1_COPY ); //FrskyHubData[] =  frskyTelemetry[0].value ;
					}
					storeTelemetryData( FR_RXV, value ) ;
				break ;
  		    
				case 3 :
					frskyTelemetry[1].set(value, FR_A2_COPY ); //FrskyHubData[] =  frskyTelemetry[1].value ;
				break ;
    		  
				case 5 : // SWR
#if defined(PCBX9D) && (defined(REVPLUS) || defined(REV9E))
					if ( !( XjtVersion != 0 && XjtVersion != 0xff ) )
					{
						if ( g_model.xprotocol != PROTO_PXX )
						{						
							value = 5 ;
						}
					}
#endif
					frskyTelemetry[3].set(value, FR_TXRSI_COPY ); //FrskyHubData[] =  frskyTelemetry[3].value ;
				break ;
				case 6 : // XJT VERSION
					XjtVersion = (*((uint16_t *)(packet+4))) ;
#if defined(PCBX9D) && (defined(REVPLUS) || defined(REV9E))
					if ( !( XjtVersion != 0 && XjtVersion != 0xff) )
					{
						if ( g_model.xprotocol != PROTO_PXX )
						{						
							frskyTelemetry[3].set( 5, FR_TXRSI_COPY ); //FrskyHubData[] =  frskyTelemetry[3].value ;
						}
					}
#endif
				break ;
				case 7 : // Multi Id
					MultiId[0] = packet[4] ;
					MultiId[1] = packet[5] ;
					MultiId[2] = packet[6] ;
					MultiId[3] = packet[7] ;
				break ;
			}
		}
		else if ( packet[3] == 0 )
		{ // old sensors
  	  frskyUsrStreaming = 255 ; //FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
			frskyStreaming = FRSKY_TIMEOUT10ms * 3 ; // reset counter only if valid frsky packets are being detected
			uint16_t value = (*((uint16_t *)(packet+4))) ;
			store_indexed_hub_data( packet[2], value ) ;
		}
		else
		{ // new sensors
			frskyStreaming = FRSKY_TIMEOUT10ms * 3 ; // reset counter only if valid frsky packets are being detected
  	  frskyUsrStreaming = 255 ; //FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
			uint8_t id = (packet[3] << 4) | ( packet[2] >> 4 ) ;
			uint32_t value = (*((uint32_t *)(packet+4))) ;

		 if ( (packet[3] & 0xF0) != 0x50 )
		 {
			switch ( id )
			{
				case ALT_ID_8 :
					value = (int32_t)value / 10 ;
					storeTelemetryData( FR_SPORT_ALT, value ) ;
				break ;

				case VARIO_ID_8 :
					storeTelemetryData( FR_VSPD, value ) ;
				break ;

				case BETA_ALT_ID_8 :
					value = (int32_t)value >> 8 ;
					value = (int32_t)value ;
					storeTelemetryData( FR_SPORT_ALT, value ) ;
				break ;

//				case BETA_VARIO_ID_8 :
//					value = (int32_t)value >> 8 ;
//					storeTelemetryData( FR_VSPD, value ) ;
//				break ;

				case CELLS_ID_8 :
				{
  	      uint8_t cells = value ;
					cells >>= 4 ;
  	      uint8_t battnumber = value ;
					battnumber &= 0x0F ;
					if ( prim == DATA_ID_FLVSS )
					{
	  				FrskyBattCells[0] = cells ;
					}
					else
					{
	  				FrskyBattCells[1] = cells ;
						battnumber += 6 ;		
//						battnumber += FrskyBattCells[0] ;
						cells += 6 ;
					}
					uint16_t cell ;

					value >>= 8 ;
					cell = value ;
					store_cell_data( battnumber, cell ) ;
					battnumber += 1 ;
					if ( battnumber < cells )
					{
						value >>= 12 ;
						cell = value ;
						store_cell_data( battnumber, cell ) ;
					}
				}
				break ;

				case CURR_ID_8 :
					storeTelemetryData( FR_CURRENT, value ) ;
				break ;

				case VFAS_ID_8 :
					storeTelemetryData( FR_VOLTS, value / 10 ) ;
					VfasVoltageTimer = 50 ;
				break ;
				
				case RPM_ID_8 :
					storeTelemetryData( FR_RPM, value / 60 ) ;
				break ;

				case A3_ID_8 :
				{	
					uint16_t ratio = g_model.frsky.channels[0].ratio3_4 ;
					if ( ratio == 0 )
					{
						ratio = 330 ;
					}
					value = value * ratio / 330 ;					
					storeTelemetryData( FR_A3, value ) ;
				}
				break ;

				case A4_ID_8 :
				{	
					uint16_t ratio = g_model.frsky.channels[1].ratio3_4 ;
					if ( ratio == 0 )
					{
						ratio = 330 ;
					}
					value = value * ratio / 330 ;					
					storeTelemetryData( FR_A4, value ) ;
				}
				break ;

				case T1_ID_8 :
					if ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) )
					{
						storeTelemetryData( FR_TEMP2, value ) ;
					}
					else
					{
						storeTelemetryData( FR_TEMP1, value ) ;
					}
				break ;
				
				case T2_ID_8 :
					if ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) )
					{
						storeTelemetryData( FR_BASEMODE, value ) ;
					}
					else
					{
						storeTelemetryData( FR_TEMP2, value ) ;
					}
				break ;

				case ACCX_ID_8 :
					storeTelemetryData( FR_ACCX, value ) ;
				break ;
					
				case ACCY_ID_8 :
					storeTelemetryData( FR_ACCY, value ) ;
				break ;
				
				case ACCZ_ID_8 :
					storeTelemetryData( FR_ACCZ, value ) ;
				break ;

				case FUEL_ID_8 :
					if ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) )
					{
						storeTelemetryData( FR_TEMP1, value ) ;
					}
					else
					{
						storeTelemetryData( FR_FUEL, value ) ;
					}
				break ;

				case GPS_ALT_ID_8 :
					value = (int32_t)value / 10 ;
					storeTelemetryData( FR_SPORT_GALT, value ) ;
				break ;
				 
				case GPS_LA_LO_ID_8 :
				{	
//					Bits 31-30 00 = LAT min/10000 N
//					Bits 31-30 01 = LAT min/10000 S
//					Bits 31-30 10 = LON min/10000 E
//					Bits 31-30 11 = LON min/10000 W
					uint32_t code = value >> 30 ;
					value &= 0x3FFFFFFF ;
					uint16_t bp ;
					uint16_t ap ;
					uint32_t temp ;
					temp = value / 10000 ;
					bp = (temp/ 60 * 100) + (temp % 60) ;
		      ap = value % 10000;
					if ( code & 2 )	// Long
					{
						storeTelemetryData( FR_GPS_LONG, bp ) ;
						storeTelemetryData( FR_GPS_LONGd, ap ) ;
						storeTelemetryData( FR_LONG_E_W, ( code & 1 ) ? 'W' : 'E' ) ;
					}
					else
					{
						storeTelemetryData( FR_GPS_LAT, bp ) ;
						storeTelemetryData( FR_GPS_LATd, ap ) ;
						storeTelemetryData( FR_LAT_N_S, ( code & 1 ) ? 'S' : 'N' ) ;
					}
				}
				break ;
				
				case GPS_HDG_ID_8 :
					storeTelemetryData( FR_COURSE, value / 100 ) ;
				break ;

				case GPS_SPEED_ID_8 :
					storeTelemetryData( FR_GPS_SPEED, value/1000 ) ;
				break ;

				case AIRSPEED_ID_8 :
					storeTelemetryData( FR_AIRSPEED, value ) ;
				break ;

				// Rbox Battx
				// 0xCCCCVVVV, C has 2dp, V has 3dp
				case RBOX_BATT1_ID_8 :
					storeTelemetryData( FR_RBOX_B1_V, (value & 0x0000FFFF)/10 ) ;	// To 2 dp
					storeTelemetryData( FR_RBOX_B1_A, value >> 16 ) ;
				break ;
				
				case RBOX_BATT2_ID_8 :
					storeTelemetryData( FR_RBOX_B2_V, (value & 0x0000FFFF)/10 ) ;	// To 2 dp
					storeTelemetryData( FR_RBOX_B2_A, value >> 16 ) ;
				break ;

				// Rbox CNSP
				// 0xBBBBAAAA, AAAA batt 1 mAh, BBBB batt 2 mAh
				case RBOX_CNSP_ID_8 :
					storeTelemetryData( FR_RBOX_B1_CAP, value & 0x0000FFFF ) ;
					storeTelemetryData( FR_RBOX_B2_CAP, value >> 16 ) ;
				break ;

				// Rbox state
				// Bits 0-15 Set if channel overload
				// Bit 16 RX1IN overload
				// Bit 17 RX2IN overload
				// Bit 18 SBUS overload
				// bit19 RX1_FAILSAFE
				// Bit20 RX1_LOSTFRAME
				// Bit21 RX2_FAILSAFE
				// Bit22 RX2_LOSTFRAME
				// Bit23 RX1_PHYSICAL_CONNECTION _LOST
				// Bit24 RX2_PHYSICAL_CONNECTION _LOST
				// Bit25 RX1_NO_SIGNAL
				// Bit26 RX2_NO_SIGNAL
				case RBOX_STATE_ID_8 :
					storeTelemetryData( FR_RBOX_SERVO, value & 0x0000FFFF ) ;
					storeTelemetryData( FR_RBOX_STATE, (value >> 16) & 0x07FF ) ;
				break ;

				case S6R_ID_8 :
					S6Rdata.fieldIndex = packet[4] ;
					if ( (S6Rdata.fieldIndex >= 0x9E) && (S6Rdata.fieldIndex <= 0xA0) )
					{
						S6Rdata.value = ( packet[5] << 8 ) | packet[6] ;
					}
					else
					{
						S6Rdata.value = packet[5] ;
					}
					S6Rdata.valid = 1 ;
				break ;

				case ESC_POWER_ID_8 :
				// Low 16 bits volts 0.001 - 26.4
				// High 16 bits amps 0.01 - 30
					storeTelemetryData( FR_VOLTS, (value & 0x0000FFFF) / 10 ) ;
					storeTelemetryData( FR_CURRENT, (value >> 16) / 10 ) ;
				break ;

				case ESC_RPM_ID_8 :
				// Bit:0~15 RPM/1~65535RP
//					uint16_t ERpm = Electrical Rpm /100 so 100 are 10000 Erpm
					storeTelemetryData( FR_RPM, (value & 0x0000FFFF) * 100 / 60 ) ;
					storeTelemetryData( FR_AMP_MAH, (uint32_t)(value >> 16) ) ;
				break ;

				case ESC_TEMPERATURE_ID_8 :
				// Bit:0-7 0.1Celsius/0~255
					storeTelemetryData( FR_TEMP2, value & 0x000000FF ) ;
				break ;

				case SBEC_POWER_ID_8 :
					storeTelemetryData( FR_SBEC_VOLT, (value & 0xffff) / 10 ) ;
					storeTelemetryData( FR_SBEC_CURRENT, (value >> 16) / 10 ) ;
				break ;
				
				default :
					// Handle unknown ID
					handleUnknownId( (packet[3] << 8) | ( packet[2] ), value ) ;
				break ;
			}
		 }
		 
		 if ( ( (packet[3] & 0xF0) == 0x50 ) ) //|| ( (packet[3] & 0xF0) == 0x10 ) )
		 {
			switch ( id )
			{
				case ARDUP_ID_8 :
					{
						uint32_t t ;
						t = packet[2] & 0x0F ;
						if ( t == ARDUP_AP_STAT_ID )
						{
							// bits 0-4 Control mode
							// bit 8 Armed
							storeTelemetryData( FR_BASEMODE, value & 0x0100 ? 0x80 : 0 ) ;
							storeTelemetryData( FR_TEMP1, ((value & 0x1F)-1) | ( ARDUPtype << 8 ) ) ;
						}
						else if ( t == ARDUP_GPS_STAT_ID )
						{
							uint16_t xvalue ;
							// Bits 0-3 #Sats
							// Bits 4-5 GPs Fix NO_GPS = 0, NO_FIX = 1, GPS_OK_FIX_2D = 2, GPS_OK_FIX_3D = 3
							// Bits 6-13 HDOP  7 bits for value, 8th bit is 10^x
							// Bits 14-21 VDOP
							// Bits 22-30 GPS Alt? 7 bits for data, 8 and 9th bits 10^x
							xvalue = value & 0x000F ;	// #sats
							xvalue *= 10 ;
							xvalue += (value >>4) & 0x0003 ;	// GPS Fix mode
							storeTelemetryData( FR_TEMP2, xvalue ) ;
//							FrskyHubData[FR_TEMP2] = xvalue ;
							xvalue = (value >> 7) & 0x7F ;
							if ( value & 0x0000040 )
							{
								xvalue *= 10 ;
							}
							storeTelemetryData( FR_GPS_HDOP, xvalue * 10 ) ;
//							FrskyHubData[FR_GPS_HDOP] = xvalue * 10 ;
							xvalue = (value >> 24) & 0x7F ;
							value >>= 22 ;
							value &= 0x03 ;
							while ( value )
							{
								xvalue *= 10 ;
								value -= 1 ;
							}
							storeTelemetryData( TELEM_GPS_ALT, xvalue ) ;
						}
						else if ( t == ARDUP_BATT_ID )
						{
							uint16_t xvalue ;
	//						xvalue = value & 0x01FF ;	// #battery in 0.1V
							if ( VfasVoltageTimer )
							{
								VfasVoltageTimer -= 1 ;
							}
							else
							{
								storeTelemetryData( FR_VOLTS, value & 0x01FF ) ;
							}
							xvalue = (value >> 10) & 0x7F ;
							if ( value & 0x0000200 )
							{
								xvalue *= 10 ;
							}
							storeTelemetryData( FR_CURRENT, xvalue ) ;
							xvalue = (value >> 17) ;
							storeTelemetryData( FR_AMP_MAH, xvalue ) ;
							if ( PixHawkCapacity )
							{
								storeTelemetryData( FR_FUEL, 100 - (xvalue * 100 / PixHawkCapacity) ) ;
							}
						}
						else if ( t == ARDUP_VandYAW_ID )
						{
							uint16_t xvalue ;
							xvalue = (value >> 9) & 0x7F ;
							if ( value & 0x0000100 )
							{
								xvalue *= 10 ;
							}
							storeTelemetryData( FR_AIRSPEED, xvalue ) ;
						}
						else if ( t == ARDUP_PARAM_ID )
						{
							uint32_t id ;
							id = value >> 24 ;
							value &= 0x00FFFFFF ;

							if ( id == 1 )
							{
								ARDUPtype = value ;
							}
	//						else if ( id == 2 )
	//						{
//2: FailSafe batt voltage in centivolts
	//						}
	//						else if ( id == 3 )
	//						{
//3: Failsafe Batt capacity in mAh
	//						}
							else if ( id == 4 )
							{
								if ( value )
								{
									PixHawkCapacity = value ;
								}
							}
						}
						else if ( t == ARDUP_HOME_ID )
						{
//Distance to home
//Angle from front of vehicle
//Altitude relative to home - offset 19 bits - WRONG

//The actual order of the data elements from LS bit side is:
//Distance to home
//Altitude relative to home - offset 12 bits
//Angle from front of vehicle

//as per these define statements in the APMv3.5.3 Arducopter source

//// for home position related data
//define HOME_ALT_OFFSET 12
//define HOME_BEARING_LIMIT 0x7F
//define HOME_BEARING_OFFSET 25
							uint16_t xvalue ;
							xvalue = ( value >> 25 ) & 0x7F ;		// 7 bits
							xvalue *= 3 ;
							storeTelemetryData( FR_HOME_DIR, xvalue ) ;
						}
					}
				break ;

				default :
					// Handle unknown ID
					handleUnknownId( (packet[3] << 8) | ( packet[2] ), value ) ;
				break ;
			}
		 }
		 else
		 {
				handleUnknownId( (packet[3] << 8) | ( packet[2] ), value ) ;
		 }
		}
	}
}

// Physical ID in packet[0] ;
void processSportPacket(uint8_t *packet)
{
//	uint8_t *packet = frskyRxBuffer ;
//  uint16_t appId  = *((uint16_t *)(packet+2)) ;

	if ( MaintenanceRunning )
	{
		maintenance_receive_packet( packet, checkSportPacket(packet) ) ;	// Uses different chksum
		return ;
	}

	if ( RawLogging )
	{
		rawLogByte( 255 ) ;
		rawLogByte( packet[0] ) ;
		rawLogByte( packet[1] ) ;
		rawLogByte( packet[2] ) ;
		rawLogByte( packet[3] ) ;
		rawLogByte( packet[4] ) ;
		rawLogByte( 254 ) ;
	}

	 
  if ( !checkSportPacket(packet) )
	{
    return;
	}
	if ( RawLogging )
	{
		rawLogByte( 'X' ) ;		
	}
	processSportData( packet, 0 ) ;
}

#endif


#define FS_ID_ERR_RATE					0xfe
#define FS_ID_SNR               0xfa
#define FS_ID_NOISE             0xfb

// First byte in packet is 0xAA
void processAFHDS2Packet(uint8_t *packet, uint8_t byteCount)
{
  frskyTelemetry[3].set(packet[1], FR_TXRSI_COPY ) ;	// TSSI
	packet += 2 ;
  for (uint32_t sensor = 0; sensor < 7; sensor++)
	{
		uint32_t id ;
		int32_t value ;
		id = *packet ;
		id |= *(packet+1) << 8 ;
		packet += 2 ;
		value = (packet[1] << 8)  + packet[0] ;
		switch ( id )
		{
			case 0 :	// Rx voltage
				storeTelemetryData( FR_RXV, value/10 ) ;
			break ;
			case 1 :	// Temp
				storeTelemetryData( FR_TEMP1, value ) ;
			break ;
			case 2 :	// RPM
				storeTelemetryData( FR_RPM, value ) ;
			break ;
			case 0x0100 :	// External voltage ?
				storeTelemetryData( FR_VOLTS, value/10 ) ;
			break ;
			case 3 :	// External voltage
			case 0x0103 :	// External voltage ?
				storeTelemetryData( FR_VOLTS, value/10 ) ;
			break ;
			case 0xFC :	// RSSI
				storeRSSI( 135-value ) ;
			break ;
			case FS_ID_ERR_RATE :
				storeTelemetryData( FR_CUST1, value ) ;
			break ;
			case FS_ID_SNR :
				storeTelemetryData( FR_CUST2, value ) ;
			break ;
			case FS_ID_NOISE :
				storeTelemetryData( FR_CUST3, value ) ;
			break ;
		}
		packet += 2 ;
	}
 	frskyUsrStreaming = 255 ; // reset counter only if valid packets are being detected
 	frskyStreaming = 255 ; // reset counter only if valid packets are being detected
}


/*
   Receive serial (RS-232) characters, detecting and storing each Fr-Sky 
   0x7e-framed packet as it arrives.  When a complete packet has been 
   received, process its data into storage variables. Characters are
   received using dma, and sent here every 10 mS.
*/

//void dsm_start_receive()
//{
//	numPktBytes = 0 ;
//}

uint32_t handlePrivateData( uint8_t state, uint8_t byte )
{
	switch ( state )
	{
    case PRIVATE_COUNT :
			Private_position = 0 ;
#ifndef SMALL
			if ( byte == 'P' )
			{
				// MULTI_TELEMETRY
        dataState = PRIVATE_TYPE ;
				break ;
			}
			Private_type = 0xFF ;	// Not MULTI_TELEMETRY
			// else fall through
    case PRIVATE_XCOUNT :
#endif
			dataState = PRIVATE_VALUE ;
      Private_count = byte ;		// Count of bytes to receive
			if ( byte > PRIVATE_BUFFER_SIZE )
			{
      	dataState = frskyDataIdle ;
			}
    break ;
    
#ifndef SMALL
		case PRIVATE_TYPE :
      Private_type = byte ;
      dataState = PRIVATE_XCOUNT ;
			if ( byte > 3 )
			{
				Private_position = 1 ;
			}
    break ;
#endif
		
		case PRIVATE_VALUE :
		{
			InputPrivateData[Private_position++] = byte ;
#ifndef SMALL
			if ( ( Private_type == 0xFF ) && ( byte & 0x80 ) )
#else
			if ( byte & 0x80 )
#endif
			{
        dataState = frskyDataIdle ;
				return 0 ;
			}
			else if ( ( Private_position == Private_count ) || ( Private_position >= PRIVATE_BUFFER_SIZE ) )
			{
        dataState = frskyDataIdle;
				// process private data here
				if ( Private_position == Private_count )
				{
					// Correct amount received
					uint32_t len = Private_count ;
#ifndef SMALL
					if ( Private_type == 0xFF )
#endif
					{
						if ( len > 6 )
						{
							len = 6 ;
						}
						while ( len )
						{
							len -= 1 ;
							PrivateData[len] = InputPrivateData[len] ;
						}
//						if ( Private_count > 5 )
//						{
//							uint32_t index = 6 ;
//							len = InputPrivateData[5] ;
//							if ( len > 16 )
//							{
//								len = 16 ;
//							}
//							while ( len )
//							{
//								len -= 1 ;
//								ExtraPrivateData[index-6] = InputPrivateData[index] ;
//							}
//						}
					}
#ifndef SMALL
					else
					{
						// MULTI_TELEMETRY
						switch ( Private_type )
						{
							case 1 :	// Status
								if ( len > 6 )
								{
									len = 6 ;
								}
								while ( len )
								{
									len -= 1 ;
									PrivateData[len] = InputPrivateData[len] ;
								}
							break ;
						
							case 2 :	// SPort
								if ( len >= FRSKY_SPORT_PACKET_SIZE )
								{
	if ( RawLogging )
	{
		rawLogByte( 'Z' ) ;		
	}
									
#if not defined(PCBT16)
									processSportPacket(InputPrivateData) ;
	if ( RawLogging )
	{
		rawLogByte( 'Y' ) ;		
	}
#endif
								}
							break ;
							case 3 :	// Hub
								numPktBytes = Private_count ;
	          		processFrskyPacket(InputPrivateData) ;
							break ;
							case 4 :	// DSM 
								processDsmPacket( InputPrivateData, Private_count+1 ) ;
							break ;
							case 5 :	// DSM bind
								dsmBindResponse( InputPrivateData[7], InputPrivateData[6] ) ;
							break ;
							case 6 :	// AFHDS2
								processAFHDS2Packet( InputPrivateData, Private_count+1 ) ;
							break ;
							case 10 :	// Hitec
								processHitecPacket( InputPrivateData ) ;
							break ;
							case 13 :	// Trainer data
								processTrainerPacket( InputPrivateData ) ;
							break ;
						}
					}
#endif
				}
			}
		}
    break ;
	}
	return 1 ;
}



//uint16_t XFDebug1 ;
//uint16_t XFDebug2 ;
//uint16_t XFDebug3 ;
//uint16_t XFDebug4 ;
//uint16_t XFDebug5 ;


void frsky_receive_byte( uint8_t data )
{
	TelRxCount += 1 ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX7) || defined(PCBX9LITE)
#ifdef BLUETOOTH	
	if ( g_model.bt_telemetry )
	{
		telem_byte_to_bt( data ) ;
	}
#endif
#endif
#ifndef ACCESS
	if ( RawLogging )
	{
		rawLogByte( data ) ;		
	}
#endif
//#ifdef REVX
	if ( TelemetryType == TEL_MAVLINK )
	{
		mavlinkReceive( data ) ;
		return ;
	}
//#endif
//	XFDebug5 += 1 ;

#ifdef XFIRE
// #ifdef REVX
		if ( g_model.Module[1].protocol == PROTO_XFIRE )
		{
			// process Xfire data here
			processCrossfireTelemetryData( data ) ;
			return ;
		}
// #endif
#endif

  uint8_t numbytes = numPktBytes ;

	if ( ( FrskyTelemetryType == FRSKY_TEL_DSM ) || ( FrskyTelemetryType == FRSKY_TEL_AFH ) 
		 || ( FrskyTelemetryType == FRSKY_TEL_HITEC ) )		// DSM telemetry || AFHDS2A telemetry || Hitec
	{
    	switch (dataState) 
			{
				case PRIVATE_COUNT :
				case PRIVATE_VALUE :
#ifndef SMALL
		    case PRIVATE_XCOUNT :
				case PRIVATE_TYPE :
#endif
					if ( handlePrivateData( dataState, data ) )
					{
      			break ;
					}
					// else fall through
    	  case frskyDataIdle:
    	    if (data == 0xAA)
					{
    	     	dataState = frskyDataInFrame ;
    	      numbytes = 0 ;
	  	      frskyRxBuffer[numbytes++] = data ;
					}
	        else if (data == PRIVATE)
					{
						dataState = PRIVATE_COUNT ;
						break ;
					}
				break ;

    	  case frskyDataInFrame:
				{
					uint32_t length ;
    	    length = ( FrskyTelemetryType == FRSKY_TEL_AFH ) ? FLYSKY_TELEMETRY_LENGTH+1 : 19 ;
					if ( FrskyTelemetryType == FRSKY_TEL_HITEC )
					{
						length = HITEC_TELEMETRY_LENGTH ;
					}
    	    if (numbytes < length )
					{
	  	      frskyRxBuffer[numbytes++] = data ;
						if ( FrskyTelemetryType == FRSKY_TEL_AFH )	// AFHDS2A
						{
							if ( numbytes >= FLYSKY_TELEMETRY_LENGTH )
							{
								processAFHDS2Packet( frskyRxBuffer, numbytes ) ;
								numbytes = 0 ;
        				dataState = frskyDataIdle;
							}
						}
						else if ( FrskyTelemetryType == FRSKY_TEL_HITEC )
						{
							if ( numbytes >= HITEC_TELEMETRY_LENGTH )
							{
								processHitecPacket( frskyRxBuffer ) ;
								numbytes = 0 ;
        				dataState = frskyDataIdle;
							}
						}
						else // DSM
						{
							if ( numbytes >= 18 )
							{
								processDsmPacket( frskyRxBuffer, numbytes ) ;
								numbytes = 0 ;
        				dataState = frskyDataIdle;
							}
						}
					}
				}
    	  break ;
			}
	}
	else if ( g_model.telemetryProtocol == TELEMETRY_HUBRAW )
	{
  	frskyStreaming = FRSKY_TIMEOUT10ms; // reset counter only if valid frsky packets are being detected
    frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
		frsky_proc_user_byte( data ) ;		
	}
	else // Not DSM
	{
    switch (dataState) 
    {
      case frskyDataStart:
        if (data == START_STOP)
				{
#ifndef DISABLE_SPORT
//#ifndef REVX
					if ( FrskyTelemetryType )		// SPORT
					{
         		dataState = frskyDataInFrame ;
           	numbytes = 0 ;
					}
//#endif
#endif
					break ; // Remain in userDataStart if possible 0x7e,0x7e doublet found.
				}
        else if (data == PRIVATE)
				{
					dataState = PRIVATE_COUNT ;
					break ;
				}
        dataState = frskyDataInFrame;
        if (numbytes < 19)
	        frskyRxBuffer[numbytes++] = data ;
        break;

      case frskyDataInFrame:
        if (data == BYTESTUFF)
        { 
            dataState = frskyDataXOR; // XOR next byte
            break; 
        }
        if (data == START_STOP) // end of frame detected
        {
#ifndef DISABLE_SPORT
//#ifdef REVX
//	          processFrskyPacket(frskyRxBuffer); // FrskyRxBufferReady = 1;
//  	        dataState = frskyDataIdle;
//#else
					if ( FrskyTelemetryType )		// SPORT
					{						
         		dataState = frskyDataInFrame ;
           	numbytes = 0;
					}
					else
#endif
					{
	          processFrskyPacket(frskyRxBuffer); // FrskyRxBufferReady = 1;
  	        dataState = frskyDataIdle;
					}
          break;
        }
        else if ( (data == PRIVATE) && ( (numbytes == 0) || (numbytes == 19) ) )
				{
					dataState = PRIVATE_COUNT ;
					break ;
				}
				if ( ( FrskyTelemetryType ) && ( numbytes == 1 ) && (data == PRIVATE) )
				{
					dataState = PRIVATE_COUNT ;
					break ;
				}

        if (numbytes < 19)
	        frskyRxBuffer[numbytes++] = data;
        break;

      case frskyDataXOR:
        dataState = frskyDataInFrame;
        if (numbytes < 19)
          frskyRxBuffer[numbytes++] = data ^ STUFF_MASK;
        break;

			case PRIVATE_COUNT :
			case PRIVATE_VALUE :
#ifndef SMALL
	    case PRIVATE_XCOUNT :
			case PRIVATE_TYPE :
#endif
				if ( handlePrivateData( dataState, data ) )
				{
     			break ;
				}
				// else fall through
      case frskyDataIdle:
        if (data == START_STOP)
        {
          numbytes = 0;
          dataState = frskyDataStart;
        }
        else if (data == PRIVATE)
				{
					dataState = PRIVATE_COUNT ;
					break ;
				}
        break;
	    
    } // switch
  }
#ifndef DISABLE_SPORT
//#ifndef REVX
	if ( FrskyTelemetryType == FRSKY_TEL_SPORT )		// SPORT
	{
  	if (numbytes >= FRSKY_SPORT_PACKET_SIZE)
		{
			processSportPacket(frskyRxBuffer) ;
		  numbytes = 0 ;
			dataState = frskyDataIdle;
		}
//#if 0
//// Detect polling for sending data out
//		else
//		{
//		  if ( numbytes == 1 )
//			{
//				if ( data == 0xBA )
//				{
//					// Treat this as a poll for data
//				}
//			}
			
//		}
//#endif
	}
#endif
  numPktBytes = numbytes ;
}

/*
   USART0 (transmit) Data Register Emtpy ISR
   Usef to transmit FrSky data packets, which are buffered in frskyTXBuffer. 
*/
//uint8_t frskyTxISRIndex = 0;
//ISR(USART0_UDRE_vect)
//{
//  if (frskyTxBufferCount > 0) 
//  {
//    UDR0 = frskyTxBuffer[frskyTxISRIndex++];
//    frskyTxBufferCount--;
//  } else
//    UCSR0B &= ~(1 << UDRIE0); // disable UDRE0 interrupt
//}

/******************************************/

#ifdef PCBSKY
void frskyTransmitBuffer( uint32_t size )
{
	if ( g_model.frskyComPort == 0 )
	{	
//		txPdcUsart( frskyTxBuffer, size, 0 ) ;
	}
	else
	{
		if ( g_model.bt_telemetry < 2 )
		{
			txCom2Uart( frskyTxBuffer, size ) ;
		}
	}
}
#endif
#ifdef PCB9XT
void frskyTransmitBuffer( uint32_t size )
{
//	x9dHubTxStart( frskyTxBuffer, size ) ;
}
#endif

uint8_t FrskyAlarmSendState = 0 ;
uint8_t FrskyDelay = 0 ;
uint8_t FrskyRSSIsend = 0 ;


void FRSKY10mspoll(void)
{
  if (FrskyDelay)
  {
    FrskyDelay -= 1 ;
    return ;
  }
#ifdef PCBSKY
	if ( txPdcPending() )
  {
    return; // we only have one buffer. If it's in use, then we can't send yet.
  }
#endif
#ifdef PCB9XT
	if ( hubTxPending() )
  {
    return  ;; // we only have one buffer. If it's in use, then we can't send yet.
  }
#endif

  // Now send a packet
#ifdef PCBSKY
#ifdef REVX
	if ( TelemetryType == TEL_JETI )
	{
//    FrskyDelay = 5 ; // 50mS
//    frskyTransmitBuffer( size ) ; 
	}
	else
#endif
#endif
#if defined(PCBSKY) || defined(PCB9XT)
  {
		uint8_t i ;
		uint8_t j = 1 ;
		uint32_t size ;

		for ( i = 0 ; i < 7 ; i += 1, j <<= 1 )
		{
			if ( FrskyAlarmSendState & j )
			{
				break ;				
			}			
		}
    FrskyAlarmSendState &= ~j ;
//		if ( i < 4 )
//		{
//	    uint8_t channel = 1 - (i / 2);
//  	  uint8_t alarm = 1 - (i % 2);
    
//		//	FRSKY_setTxPacket( A22PKT + i, g_eeGeneral.frskyinternalalarm ? 0 :g_model.frsky.channels[channel].alarms_value[alarm],
//		//														 ALARM_GREATER(channel, alarm), ALARM_LEVEL(channel, alarm) ) ;					
		
//	    size = FRSKY_setTxPacket( A22PKT + i, g_model.frsky.channels[channel].alarms_value[alarm],
//                   ALARM_GREATER(channel, alarm), g_eeGeneral.frskyinternalalarm ? 0 :ALARM_LEVEL(channel, alarm) ) ;
									 
//		}
		if( i < 6 )
		{
			i &= 1 ;
			size = FRSKY_setTxPacket( RSSITXPKT+i, frskyRSSIlevel[i], 0, frskyRSSItype[i] ) ;
		}
		else if (i == 6)
		{
// Send packet requesting all RSSIalarm settings be sent back to us
			size = FRSKY_setTxPacket( RSSI_REQUEST, 0, 0, 0 ) ;
		}
		else
		{
			return ;
		}
    FrskyDelay = 5 ; // 50mS
    frskyTransmitBuffer( size ) ; 
  }
#endif
}

//  uint8_t i = 0;

//  for (int alarm=0; alarm<2; alarm++) {
//    frskyTxBuffer[i++] = START_STOP;        // Start of packet
//    frskyTxBuffer[i++] = (RSSI1PKT-alarm);  // f7 - f6
//    frskyPushValue(i, g_eeGeneral.frskyRssiAlarms[alarm].value+50-(10*i));
//    {
//      uint8_t *ptr ;
//      ptr = &frskyTxBuffer[i] ;
//      *ptr++ = 0x00 ;
//      *ptr++ = g_eeGeneral.frskyRssiAlarms[alarm].level;
//      *ptr++ = 0x00 ;
//      *ptr++ = 0x00 ;
//      *ptr++ = 0x00 ;
//      *ptr++ = 0x00 ;
//      *ptr++ = 0x00 ;
//      *ptr++ = START_STOP;        // End of packet
//      i += 8 ;
//    }
//  }

//  frskyTxBufferCount = i;



uint32_t FRSKY_setTxPacket( uint8_t type, uint8_t value, uint8_t p1, uint8_t p2 )
{
	uint8_t i = 0;
  frskyTxBuffer[i++] = START_STOP;        // Start of packet
  frskyTxBuffer[i++] = type ;
  frskyPushValue(i, value) ;
  {
    uint8_t *ptr ;
    ptr = &frskyTxBuffer[i] ;
    *ptr++ = p1 ;
    *ptr++ = p2 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = START_STOP ;        // End of packet
	}
	return i + 8 ;
}

//enum AlarmLevel FRSKY_alarmRaised(uint8_t idx, uint8_t alarm)
//{
//    uint8_t value ;
//    uint8_t alarm_value ;
//		enum AlarmLevel level = alarm_off ;
//  for (int i=0; i<2; i++) {
//        if ( ( alarm == i ) || ( alarm > 1 ) )
//        {
//        if ( ( level = (enum AlarmLevel)ALARM_LEVEL(idx, i) ) != alarm_off) {
//                value = frskyTelemetry[idx].value ;
//                alarm_value = g_model.frsky.channels[idx].alarms_value[i] ;
//          if (ALARM_GREATER(idx, i)) {
//            if (value > alarm_value)
//              return level;
//          }
//          else {
//            if (value < alarm_value)
//              return level;
//          }
//        }
//        }
//  }
//  return alarm_off;
//}

//void FRSKY_alarmPlay(uint8_t idx, uint8_t alarm)
//{
//#ifdef PCBSKY
//	uint8_t alarmLevel = ALARM_LEVEL(idx, alarm);
			
//	if(/*((g_eeGeneral.speakerMode &1) == 1*/ /*|| g_eeGeneral.speakerMode == 2) && */g_eeGeneral.frskyinternalalarm == 1)
//	{   // this check is done here so haptic still works even if frsky beeper used.
//		switch (alarmLevel)
//		{
//			case alarm_off:
//			break;
//			case alarm_yellow:
//				audio.event(g_eeGeneral.FRSkyYellow);
//			break;														
//			case alarm_orange:
//				audio.event(g_eeGeneral.FRSkyOrange);
//			break;												
//			case alarm_red:
//				audio.event(g_eeGeneral.FRSkyRed);
//			break;		
//		}	
//	}
//#endif
//}

void telemetry_init( uint8_t telemetryType )
{
	TelemetryType = telemetryType ;
	switch ( telemetryType )
	{
		case TEL_FRSKY_HUB :
		case TEL_MULTI :
			FRSKY_Init( FRSKY_TEL_HUB ) ;
		break ;
		
		case TEL_FRSKY_SPORT :
			FRSKY_Init( FRSKY_TEL_SPORT ) ;
		break ;

#ifdef REVX
		case TEL_JETI :
			com1_Configure( 9600, SERIAL_NORM, SERIAL_NO_PARITY ) ;
//			UART2_Configure( 9600, Master_frequency ) ;
  		USART0->US_MR =  0x000202C0 ;  // NORMAL, Odd Parity, 9 bit
//			com1_timeout_disable() ;
//			UART2_timeout_disable() ;
//			g_model.telemetryRxInvert = 1 ;
//			setMFP() ;
			USART0->US_IER = US_IER_RXRDY ;
			NVIC_SetPriority( USART0_IRQn, 3 ) ;
			NVIC_EnableIRQ(USART0_IRQn) ;
		  memset(frskyAlarms, 0, sizeof(frskyAlarms));
		  resetTelemetry(TEL_ITEM_RESET_ALL) ;
//			startPdcUsartReceive() ;
		break ;
#endif

#ifdef REVX
		case TEL_MAVLINK :
	  	memset(frskyAlarms, 0, sizeof(frskyAlarms));
	  	resetTelemetry(TEL_ITEM_RESET_ALL);
			FrskyComPort = g_model.frskyComPort ;
			if ( g_model.frskyComPort == 1 )
			{
				UART_Configure( g_model.com2Baudrate-1, Master_frequency ) ;
			}
			else
			{
				com1_Configure( g_model.telemetryBaudrate-1, SERIAL_NORM, SERIAL_NO_PARITY ) ;
//				UART2_Configure( g_model.telemetryBaudrate-1, Master_frequency ) ;
//				com1_timeout_disable() ;
//				UART2_timeout_disable() ;
				if ( g_model.telemetryRxInvert )
				{
					setMFP() ;
				}
				else
				{
					clearMFP() ;
				}
//				startPdcUsartReceive() ;
			}
		break ;
#endif
		
#ifdef REVX
		case TEL_ARDUPILOT :
			com1_Configure( 38400, SERIAL_NORM, SERIAL_NO_PARITY ) ;
//			UART2_Configure( 38400, Master_frequency ) ;
//				com1_timeout_disable() ;
//			UART2_timeout_disable() ;
//			g_model.telemetryRxInvert = 1 ;
//			setMFP() ;
		  memset(frskyAlarms, 0, sizeof(frskyAlarms));
		  resetTelemetry(TEL_ITEM_RESET_ALL);
//			startPdcUsartReceive() ;
		break ;
#endif

		case TEL_DSM :
			FRSKY_Init( FRSKY_TEL_DSM ) ;
		break ;

		case TEL_HITEC :
			FRSKY_Init( FRSKY_TEL_HITEC ) ;
		break ;

#ifdef XFIRE
// #ifdef REVX
		case TEL_XFIRE :
			FRSKY_Init( FRSKY_TEL_XFIRE ) ;
		break ;
// #endif
#endif
		 
	}
}

void initComPort( uint32_t baudRate, uint32_t invert, uint32_t parity )
{
	if ( g_model.frskyComPort == 0 )
	{
		com1_Configure( baudRate, invert & 1, parity ) ;
	}
	else
	{
		com2_Configure( baudRate, invert & 2, parity ) ;
	}
}

// brate values
// 0 FrSky hub / Multi
// 1 FrSky SPort
// 2 DSM
// 3 AFH
// 4 XFIRE
// 5 HITEC

void FRSKY_Init( uint8_t brate )
{
#if defined(PCBT16)
	MultiFrskyTelemetry = 0 ;
#endif	 
	
	FrskyComPort = g_model.frskyComPort ;
	FrskyTelemetryType = brate == 3 ? 2 : brate ;

	if ( ( g_model.com2Function == COM2_FUNC_SBUSTRAIN ) || ( g_model.com2Function == COM2_FUNC_SBUS57600 ) )
	{
		if ( g_model.frskyComPort == 1 )
		{
			// Can't change COM2 from SBUS
			return ;
		}
	}
#ifdef PCBSKY
//	if ( g_model.com2Function == COM2_FUNC_FMS )
//	{
//		com2_Configure( 19200, SERIAL_NORM, SERIAL_NO_PARITY ) ;
//		return ;
//	}
	if ( g_model.com2Function == COM2_FUNC_LCD )
	{
		com2_Configure( 115200, SERIAL_NORM, 0 ) ;
		return ;
	}
#endif

#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
	if ( brate == FRSKY_TEL_HUB )
	{
		uint32_t baudrate = 9600 ;
		uint32_t parity = SERIAL_NO_PARITY ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
		if ( g_model.Module[1].protocol == PROTO_MULTI )
#else
		if ( ( g_model.Module[0].protocol == PROTO_MULTI ) || ( g_model.Module[1].protocol == PROTO_MULTI ) )
#endif
		{
			baudrate = 100000 ;
			parity = SERIAL_EVEN_PARITY ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
			if ( ( g_model.Module[1].sub_protocol & 0x3F ) == M_FRSKYX )
#else
			if ( ( ( g_model.Module[0].sub_protocol & 0x3F ) == M_FRSKYX ) || ( ( g_model.Module[1].sub_protocol & 0x3F ) == M_FRSKYX ) )
#endif
			{
#if defined(PCBT16)
				MultiFrskyTelemetry = 1 ;
#endif	 
				FrskyTelemetryType = FRSKY_TEL_SPORT ;
			}
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
			if ( ( g_model.Module[1].sub_protocol & 0x3F ) == M_FrskyD )
#else
			if ( ( ( g_model.Module[0].sub_protocol & 0x3F ) == M_FrskyD ) || ( ( g_model.Module[1].sub_protocol & 0x3F ) == M_FrskyD ) )
#endif
			{
#if defined(PCBT16)
				MultiFrskyTelemetry = 1 ;
#endif	 
				FrskyTelemetryType = FRSKY_TEL_HUB ;
			}
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
			if ( ( g_model.Module[1].sub_protocol & 0x3F ) == M_AFHD2SA )
#else
			if ( ( ( g_model.Module[0].sub_protocol & 0x3F ) == M_AFHD2SA ) || ( ( g_model.Module[1].sub_protocol & 0x3F ) == M_AFHD2SA ) )
#endif
			{
				FrskyTelemetryType = FRSKY_TEL_AFH ;	// AFHD2SA
			}
		}
#ifdef XFIRE
 #ifdef PCB9XT
		if ( g_model.Module[1].protocol == PROTO_XFIRE )
		{
			baudrate = XFIRE_BAUD_RATE ;
		}
 #endif
#endif

#ifdef REVX
		initComPort( baudrate, SERIAL_NORM, parity ) ;
		if ( g_model.telemetryRxInvert )
		{
			setMFP() ;
		}
		else
		{
			clearMFP() ;
		}
#else
		initComPort( baudrate, (g_model.telemetry2RxInvert << 1) | g_model.telemetryRxInvert, parity ) ;
#endif
	}
	else if ( brate == FRSKY_TEL_SPORT )
	{
		numPktBytes = 0 ;
		dataState = frskyDataIdle ;
		initComPort( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ;
	}
#ifdef XFIRE
// #ifdef REVX
	else if ( brate == FRSKY_TEL_XFIRE )
	{
		FrskyComPort = g_model.frskyComPort = 0 ;
		com1_Configure( XFIRE_BAUD_RATE, SERIAL_NORM, SERIAL_NO_PARITY ) ;
	}
// #endif
#endif
#ifdef REVX
	else	// brate == TEL_TYPE_DSM, DSM telemetry
	{
		FrskyComPort = g_model.frskyComPort ;
		if ( ( g_model.Module[0].protocol == PROTO_MULTI ) || ( g_model.Module[1].protocol == PROTO_MULTI ) )
		{
			if ( ( ( g_model.Module[0].sub_protocol & 0x1F ) == M_DSM ) || ( ( g_model.Module[1].sub_protocol & 0x1F ) == M_DSM ) )
			{
				FrskyTelemetryType = FRSKY_TEL_DSM ;	// DSM
			}
			initComPort( 100000, SERIAL_NORM, SERIAL_EVEN_PARITY ) ;
		}
		else
		{
			initComPort( 115200, SERIAL_NORM, SERIAL_NO_PARITY ) ;
		}
	}
#else
	else	// brate == 2, DSM telemetry or TEL_TYPE_HITEC
	{
		FrskyComPort = g_model.frskyComPort ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
		if ( g_model.Module[1].protocol == PROTO_MULTI )
#else
		if ( ( g_model.Module[0].protocol == PROTO_MULTI ) || ( g_model.Module[1].protocol == PROTO_MULTI ) )
#endif
		{
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
			if ( ( g_model.Module[1].sub_protocol & 0x1F ) == M_DSM )
#else
			if ( ( ( g_model.Module[0].sub_protocol & 0x1F ) == M_DSM ) || ( ( g_model.Module[1].sub_protocol & 0x1F ) == M_DSM ) )
#endif
			{
				FrskyTelemetryType = FRSKY_TEL_DSM ;	// DSM
			}
			if ( brate == FRSKY_TEL_HITEC )
			{
				FrskyTelemetryType = FRSKY_TEL_HITEC ;
			}
		 	initComPort( 100000, (g_model.telemetry2RxInvert << 1) | g_model.telemetryRxInvert, SERIAL_EVEN_PARITY ) ;
//			if ( g_model.telemetryRxInvert )
//			{
//				init_software_com1( 100000, SERIAL_INVERT, SERIAL_EVEN_PARITY ) ;
//			}
//			else
//			{
//				UART2_Configure( 100000, Master_frequency ) ;
//				com1Parity( SERIAL_EVEN_PARITY ) ;
//			}
		}
		else
		{
			initComPort( 115200, SERIAL_INVERT, SERIAL_NO_PARITY ) ;
		}
	}
#endif
  // clear frsky variables
#endif // PCBSKY

//#ifdef PCB9XT
//	if ( brate == TEL_TYPE_FRSKY_HUB )
//	{
//		uint32_t baudrate = 9600 ;
//		uint32_t parity = SERIAL_NO_PARITY ;
//		if ( ( g_model.protocol == PROTO_MULTI ) || ( g_model.xprotocol == PROTO_MULTI ) )
//		{
//			baudrate = 100000 ;
//			parity = SERIAL_EVEN_PARITY ;
//			if ( ( ( g_model.sub_protocol & 0x1F ) == M_FRSKYX ) || ( ( g_model.xsub_protocol & 0x1F ) == M_FRSKYX ) )
//			{
//				FrskyTelemetryType = 1 ;
//			}
//			if ( ( ( g_model.sub_protocol & 0x1F ) == M_FrskyD ) || ( ( g_model.xsub_protocol & 0x1F ) == M_FrskyD ) )
//			{
//				FrskyTelemetryType = 0 ;
//			}
//		}
//		if ( g_model.frskyComPort == 0 )
//		{
//			com1_Configure( baudrate, g_model.telemetryRxInvert, parity ) ;
////			if ( baudrate == 100000 )
////			{
////				com1_Configure( 100000, SPORT_POLARITY_INVERT, SERIAL_EVEN_PARITY ) ;	// Invert/even parity
////			}
////			else
////			{
////				uint32_t parity = SERIAL_NO_PARITY ;
////				if ( ( g_model.protocol == PROTO_MULTI ) || ( g_model.xprotocol == PROTO_MULTI ) )
////				{
////					parity = SERIAL_EVEN_PARITY ;
////				}	
////				com1_Configure( baudrate, SPORT_POLARITY_NORMAL, parity ) ;
////			}
//		}
//		else
//		{
//			com2_Configure( baudrate, parity ) ;
			
////			console9xtInit() ;
////			UART4SetBaudrate( baudrate ) ;
//////			if ( ( g_model.protocol == PROTO_MULTI ) || ( g_model.xprotocol == PROTO_MULTI ) )
//////			{
////			com2Parity( parity ) ;
//////			}	
//		}
//	}
//	else if ( brate == TEL_TYPE_FRSKY_SPORT )
//	{
//		numPktBytes = 0 ;
//		dataState = frskyDataIdle ;
//		com1_Configure( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ;		// 57600
//	}
//	else // TEL_TYPE_DSM
//	{
//		FrskyComPort = g_model.frskyComPort = 0 ;
//		if ( ( g_model.protocol == PROTO_MULTI ) || ( g_model.xprotocol == PROTO_MULTI ) )
//		{
//			if ( ( ( g_model.sub_protocol & 0x1F ) == M_DSM ) || ( ( g_model.xsub_protocol & 0x1F ) == M_DSM ) )
//			{
//				FrskyTelemetryType = 2 ;	// DSM
//			}
//			com1_Configure( 100000, SERIAL_INVERT, SERIAL_EVEN_PARITY ) ;	// Invert
//		}
//		else
//		{
//			com1_Configure( 115200, SERIAL_INVERT, SERIAL_NO_PARITY ) ;	// Invert
//		}
//	}
//#endif // PCB9XT

//#ifdef PCBX9D
//	if ( brate == TEL_TYPE_FRSKY_HUB )
//	{
//		uint32_t baudrate = 9600 ;
//		uint32_t parity = SERIAL_NO_PARITY ;
//		if ( g_model.xprotocol == PROTO_MULTI )
//		{
//			baudrate = 100000 ;
//			parity = SERIAL_EVEN_PARITY ;
//			if ( ( g_model.xsub_protocol & 0x1F ) == M_FRSKYX )
//			{
//				FrskyTelemetryType = 1 ;
//			}
//			if ( ( g_model.xsub_protocol & 0x1F ) == M_FrskyD )
//			{
//				FrskyTelemetryType = 0 ;
//			}
//		}
//		if ( g_model.frskyComPort == 0 )
//		{
//			com1_Configure( baudrate, g_model.telemetryRxInvert, parity ) ;
////			if ( baudrate == 100000 )
////			{
////				com1_Configure( 100000, SERIAL_INVERT, 1 ) ;	// Invert/even parity
////			}
////			else
////			{
////				uint32_t parity = SERIAL_NO_PARITY ;
////				if ( ( g_model.protocol == PROTO_MULTI ) || ( g_model.xprotocol == PROTO_MULTI ) )
////				{
////					parity = SERIAL_EVEN_PARITY ;
////				}	
////				com1_Configure( baudrate, SERIAL_NORM, parity ) ;
////			}
//		}
//		else
//		{
//			com2_Configure( baudrate, parity ) ;
////			x9dConsoleInit() ;
////			USART3->BRR = PeripheralSpeeds.Peri1_frequency / baudrate ;
////			if ( ( g_model.protocol == PROTO_MULTI ) || ( g_model.xprotocol == PROTO_MULTI ) )
////			{
////				com2Parity( SERIAL_EVEN_PARITY ) ;
////			}	
//		}
//	}
//	else if ( brate == TEL_TYPE_FRSKY_SPORT )
//	{
//		numPktBytes = 0 ;
//		dataState = frskyDataIdle ;
//		com1_Configure( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ;		// 57600
//	}
//	else // TEL_TYPE_DSM
//	{
//		FrskyComPort = g_model.frskyComPort = 0 ;
//		if ( ( g_model.protocol == PROTO_MULTI ) || ( g_model.xprotocol == PROTO_MULTI ) )
//		{
//			if ( ( ( g_model.sub_protocol & 0x1F ) == M_DSM ) || ( ( g_model.xsub_protocol & 0x1F ) == M_DSM ) )
//			{
//				FrskyTelemetryType = 2 ;	// DSM
//			}
//			com1_Configure( 100000, SERIAL_INVERT, SERIAL_EVEN_PARITY ) ;	// Invert
//		}
//		else
//		{
//			com1_Configure( 115200, SERIAL_INVERT, SERIAL_NO_PARITY ) ;	// Invert
//		}
//	}
//#endif // PCBX9D

  memset(frskyAlarms, 0, sizeof(frskyAlarms));
  resetTelemetry(TEL_ITEM_RESET_ALL);
#ifdef PCBSKY
//	startPdcUsartReceive() ;
#endif

}

static void frskyPushValue(uint8_t & i, uint8_t value)
{
	uint8_t j ;
	j = 0 ;
  // byte stuff the only byte than might need it
  if (value == START_STOP) {
    j = 1 ;
    value = 0x5e;
  }
  else if (value == BYTESTUFF) {
    j = 1 ;
    value = 0x5d;
  }
	if ( j )
	{
		frskyTxBuffer[i++] = BYTESTUFF;
	}
  frskyTxBuffer[i++] = value;
}


void FrskyData::setoffset()
{
	offset = raw ;
	value = 0 ;
}

void FrskyData::set(uint8_t value, uint8_t copy)
{
	uint8_t x ;
	averaging_total += value ;
	uint8_t count = 16 ;
	uint8_t shift = 4 ;
	if ( ( FrskyTelemetryType == FRSKY_TEL_SPORT ) || ( FrskyTelemetryType == FRSKY_TEL_AFH ) )	// SPORT or AFHDS2
	{
		count = 4 ;
		shift = 2 ;
	}
	if ( ++averageCount >= count )
	{
		averageCount = 0 ;
		raw = averaging_total >> shift ;
  	if ( raw > offset )
		{
		  x = raw - offset ;
		}
		else
		{
			x = 0 ;
		}
		this->value = x ;
		storeTelemetryData( copy, this->value ) ;
		averaging_total = 0 ;
//   if (max < value)
//     max = value;
//   if (!min || min > value)
//     min = value;
	}
}




void resetTelemetry( uint32_t item )
{
	switch ( item )
	{
		case TEL_ITEM_RESET_ALT :
      AltOffset = -FrskyHubData[FR_ALT_BARO] ;
		break ;

		case TEL_ITEM_RESET_A1OFF :
			frskyTelemetry[0].setoffset() ;
		break ;

		case TEL_ITEM_RESET_A2OFF :
			frskyTelemetry[1].setoffset() ;
		break ;

		case TEL_ITEM_RESET_GPS :
		{	
			struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;

			maxMinPtr->hubMax[FR_GPS_SPEED] = 0 ;
			maxMinPtr->hubMax[TELEM_GPS_ALT] = 0 ;
		}
		break ;

		case TEL_ITEM_RESET_ALL :
//			FrskyHubData[FR_A1_MAH] = 0 ;
//			FrskyHubData[FR_A2_MAH] = 0 ;
			FrskyHubData[FR_CELL_MIN] = 450 ;			// 0 volts
			Frsky_Amp_hour_prescale = 0 ;
			FrskyHubData[FR_AMP_MAH] = 0 ;
  		memset( &FrskyHubMaxMin, 0, sizeof(FrskyHubMaxMin));
			FrskyHubMaxMin.hubMax[FR_ALT_BARO] = 0 ;
			PixHawkCapacity = 0 ;
		break ;
	}

}

uint8_t decodeMultiTelemetry()
{
	uint32_t type = TEL_FRSKY_HUB ;
	uint32_t subType = 0 ;
	if ( g_model.Module[0].protocol == PROTO_MULTI )
	{
		type = g_model.Module[0].sub_protocol ;
		subType = (g_model.Module[0].channels >> 4) & 0x07 ;
	}
	else if ( g_model.Module[1].protocol == PROTO_MULTI )
	{
		type = g_model.Module[1].sub_protocol ;
		subType = (g_model.Module[1].channels >> 4) & 0x07 ;
	}
	switch ( type )
	{
		case M_FrskyD :
		case M_BAYANG :
		case M_Hubsan :
			type = TEL_FRSKY_SPORT ;
			FrskyTelemetryType = FRSKY_TEL_HUB ;
		break ;
		case M_FRSKYX :
			type = TEL_FRSKY_SPORT ;
			FrskyTelemetryType = FRSKY_TEL_SPORT ;
		break ;
	
		case M_DSM :
			type = TEL_DSM ;
			FrskyTelemetryType = FRSKY_TEL_DSM ;
		break ;
		
		case M_AFHD2SA :
			type = TEL_AFHD2SA ;
			FrskyTelemetryType = FRSKY_TEL_AFH ;
		break ;

		case M_Hitec :
			if ( subType == 0 )
			{
				FrskyTelemetryType = FRSKY_TEL_HITEC ;
			}
		break ;
	}
	return type ;
}


uint8_t decodeTelemetryType( uint8_t telemetryType )
{
	uint8_t type = TEL_FRSKY_HUB ;

#ifdef PCBSKY
	if ( g_model.Module[1].protocol == PROTO_PXX )
	{
		type = TEL_FRSKY_SPORT ;
	}
#endif

#ifdef PCB9XT
	type = g_model.Module[0].protocol == PROTO_PXX ;
	if ( g_model.Module[0].protocol == PROTO_OFF )
	{
		type = g_model.Module[1].protocol == PROTO_PXX ;
	}
#endif

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
	type = g_model.Module[0].protocol == PROTO_PXX ;
	if ( g_model.Module[0].protocol == PROTO_OFF )
	{
		type = g_model.Module[1].protocol == PROTO_PXX ;
	}
	else
	{
		if ( g_model.Module[1].protocol == PROTO_PPM )
		{
			if ( g_model.frskyComPort )
			{
				if ( type ) // Internal is PXX
				{
					type = TEL_FRSKY_HUB ;
				}
			}
		}
	}
 #ifdef ACCESS	
	if ( g_model.Module[0].protocol == PROTO_ACCESS )
	{
		type = TEL_FRSKY_SPORT ;
	}
	else
	{
		if ( g_model.Module[0].protocol == PROTO_OFF )
		{
			if ( g_model.Module[1].protocol == PROTO_ACCESS )
			{
				type = TEL_FRSKY_SPORT ;
			}
		}
	}
 #endif

#endif

#ifdef PCBSKY
	if ( (g_model.Module[1].protocol == PROTO_DSM2) && ( g_model.Module[1].sub_protocol == DSM_9XR ) )
	{
		type = TEL_DSM ;
	}
	if ( ( g_model.Module[0].protocol == PROTO_MULTI ) || ( g_model.Module[1].protocol == PROTO_MULTI ) )
	{
		type = TEL_MULTI ;
	}
#endif

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX7) || defined(PCBX12D) || defined(PCBX7) || defined(PCBX10)
	if ( (g_model.Module[1].protocol == PROTO_DSM2) && ( g_model.Module[1].sub_protocol == DSM_9XR ) )
	{
		type = TEL_DSM ;
	}
#ifdef PCB9XT
	if ( ( g_model.Module[0].protocol == PROTO_MULTI ) || ( g_model.Module[1].protocol == PROTO_MULTI ) )
#else
	if ( g_model.Module[1].protocol == PROTO_MULTI )
#endif
	{
		type = TEL_MULTI ;
	}
#endif

	
	switch ( telemetryType )
	{
		case TELEMETRY_MAVLINK :
			type = TEL_MAVLINK ;
		break ;
		case TELEMETRY_JETI :
			type = TEL_JETI ;
		break ;
		case TELEMETRY_DSM :
			type = TEL_DSM ;
		break ;
		case TELEMETRY_FRHUB :
		case TELEMETRY_ARDUCOPTER :
		case TELEMETRY_ARDUPLANE :
		case TELEMETRY_HUBRAW :
			if ( ( type != TEL_MULTI ) && ( type != TEL_FRSKY_SPORT ) )
			{
				type = TEL_FRSKY_HUB ;
			}
		break ;
		case TELEMETRY_HITEC :
			type = TEL_HITEC ;
		break ;
	}
#ifdef XFIRE
 #ifdef REVX
	if ( g_model.Module[1].protocol == PROTO_XFIRE )
	{
		type = TEL_XFIRE ;
	}
 #endif
 #ifdef PCB9XT
	if ( g_model.Module[1].protocol == PROTO_XFIRE )
	{
		type = TEL_XFIRE ;
	}
 #endif
 #ifdef PCBX9D
	if ( g_model.Module[1].protocol == PROTO_XFIRE )
	{
		type = TEL_XFIRE ;
	}
 #endif
#endif
	return type ;
}

// Called every 10 mS in interrupt routine
void check_frsky( uint32_t fivems )
{
  // Used to detect presence of valid FrSky telemetry packets inside the
  // last FRSKY_TIMEOUT10ms 10ms intervals
	uint8_t telemetryType = g_model.telemetryProtocol ;
	uint8_t type ;

	type = decodeTelemetryType( telemetryType ) ;
#ifdef PCBSKY
	if ( ( type != TelemetryType )
			 || ( FrskyComPort != g_model.frskyComPort ) )
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	if ( ( type != TelemetryType )
			 || ( FrskyComPort != g_model.frskyComPort ) )
#endif
	{
		telemetry_init( type ) ;
	}
	if ( type == TEL_MULTI )
	{
		if ( g_model.ForceTelemetryType == 0 )
		{
			decodeMultiTelemetry() ;
		}
	}

#ifdef PCBSKY
#ifdef REVX
	if ( TelemetryType == TEL_JETI )
	{
		int32_t value ;
		while ( (value = getJetiWord()) != -1 )
		{
			// send byte to JETI code
			if ( ( value & 0x0100 ) == 0 )
			{
				if ( ( value & 0x000000FF ) == 0x000000FE )
				{
					JetiIndex = 0 ;
				}
			}
			else
			
			{
				value &= 0x000000FF ;
				if ( value == 0x000000DF )
				{
					value = '@' ;
				}
				JetiBuffer[JetiIndex++] = value ;
				if ( JetiIndex > 35 )
				{
					JetiIndex = 35 ;
				}
			}
		}
	}
	else
	{
#endif

		if ( g_model.frskyComPort == 0 )
		{
//#ifndef REVX
//			if ( CaptureMode == CAP_COM1 )
//			{
				uint16_t rxchar ;
				while ( ( rxchar = get_fifo128( &Com1_fifo ) ) != 0xFFFF )
				{
					frsky_receive_byte( rxchar ) ;
				}
//			}
//			else
//			{
//				uint16_t rxchar ;
//				while ( ( rxchar = get_fifo128( &Com1_fifo ) ) != 0xFFFF )
//				{
//					frsky_receive_byte( rxchar ) ;
//				}
//			}
//#else			 
//			uint16_t rxchar ;
//			while ( ( rxchar = get_fifo128( &Com1_fifo ) ) != 0xFFFF )
//			{
//				frsky_receive_byte( rxchar ) ;
//			}
//#endif
		}
		else
		{
//			if ( CaptureMode == CAP_COM2 )
//			{
				uint16_t rxchar ;
				while ( ( rxchar = rxCom2() ) != 0xFFFF )
				{
					frsky_receive_byte( rxchar ) ;
				}
//			}
//			else
//			{
//				uint16_t rxchar ;
//				while ( ( rxchar = rxCom2() ) != 0xFFFF )
//				{
//					frsky_receive_byte( rxchar ) ;
//				}
//			}
		}
#ifdef REVX
	}
#endif
#endif // PCBSKY

//#ifdef REVX
//	if ( telemetryType == TELEMETRY_DSM)		// DSM telemetry
//	{
//		if ( DsmRxTimeout )
//		{
//			DsmRxTimeout = 0 ;
//			processDsmPacket( frskyRxBuffer, numPktBytes ) ;
//		}
//	}
//#endif

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	{
		uint16_t rxchar ;
#ifdef ACCESS
extern uint8_t s_current_protocol[] ;
		if ( (s_current_protocol[EXTERNAL_MODULE] == PROTO_ACCESS ) ||
				 (s_current_protocol[INTERNAL_MODULE] == PROTO_ACCESS ) )
		{
			int32_t rxbyte ;
			while ( ( rxbyte = get_fifo128( &Access_int_fifo ) ) != -1 )
			{
				accessRecieveByte( rxbyte, 0 ) ;
			}
			while ( ( rxbyte = get_fifo128( &Access_ext_fifo ) ) != -1 )
			{
				accessRecieveByte( rxbyte, 1 ) ;
			}
		}
//		else
#endif
		if ( g_model.frskyComPort == 0 )
		{
//			if ( CaptureMode == CAP_COM1 )
//			{
//				uint16_t rxchar ;
				while ( ( rxchar = get_fifo128( &Com1_fifo ) ) != 0xFFFF )
				{
					frsky_receive_byte( rxchar ) ;
				}
//			}
//			else
//			{
//				while ( ( rxchar = rxTelemetry() ) != 0xFFFF )
//				{
//					frsky_receive_byte( rxchar ) ;
//				}
//			}
		}
#ifndef PCBX12D
#ifndef PCBX10
		else
		{
			while ( ( rxchar = rxCom2() ) != 0xFFFF )
			{
				frsky_receive_byte( rxchar ) ;
			}
		}
#endif
#endif
	}
#endif

	if ( fivems )
	{
  	return ;
	}

	uint8_t lTmOK ;
	if (frskyStreaming > 0)
	{
		if ( --frskyStreaming == 0 )
		{
			if ( FrskyTelemetryType != FRSKY_TEL_SPORT )
			{ // Don't zero SWR
 				FrskyHubData[FR_TXRSI_COPY] = 0 ;
			}
 			FrskyHubData[FR_RXRSI_COPY] = 0 ;
			A1Received = 0 ;
			if ( telemetryType == 2)		// DSM telemetry
			{
   			frskyTelemetry[2].set( 0, FR_RXRSI_COPY );	// RSSI
 				FrskyHubData[FR_RXRSI_COPY] = 0 ;
			}
			putSystemVoice( SV_NO_TELEM, V_NOTELEM ) ;
//			putVoiceQueue( V_NOTELEM ) ;
		}
		lTmOK = 1 ;
	}
	else
	{
		lTmOK = 0 ;
	}
  if (frskyUsrStreaming > 0)
	{
		if ( lTmOK )
		{
			lTmOK = 2 ;
		}
		frskyUsrStreaming--;
	}
	TmOK = lTmOK ;

  if ( FrskyAlarmSendState )
  {
    FRSKY10mspoll() ;
  }

	// Flight pack mAh
	if ( g_model.currentSource )
	{ // We have a source
		uint16_t current = 0 ;		// in 1/10ths amps


		if ( g_model.currentSource <= 2 )	// A1/A2
		{
			if ( frskyStreaming )
			{
				uint8_t index = FR_A1_COPY+g_model.currentSource-1 ;
				current = FrskyHubData[index] ;
				current *= g_model.frsky.channels[index].lratio ; ;
				current /= 100 ;
			}
		}
		else if ( g_model.currentSource == 3 )	// FASV
		{
  		if (frskyUsrStreaming)
		  {
				current = FrskyHubData[FR_CURRENT] ;
			}
		}
		else	// Scaler
		{
			uint8_t num_decimals ;
			uint8_t index ;
			index = g_model.currentSource - 4 ;

			if ( telemItemValid( index + TEL_ITEM_SC1 ) )
			{
				current = calc_scaler( index, 0, &num_decimals) ;
				if ( num_decimals == 0 )
				{
					current *= 10 ;
				}
				else if ( num_decimals == 2 )
				{
					current /= 10 ;
				}
			}
		}

		int16_t ah_temp ;
		ah_temp = current ;


		if ( ah_temp < 0 )
		{
			ah_temp = 0 ;			
		}
		ah_temp += Frsky_Amp_hour_prescale ;
		if ( ah_temp > 3600 )
		{
			ah_temp -= 3600 ;
			int16_t *ptr_hub = &FrskyHubData[FR_AMP_MAH] ;
			*ptr_hub += 1 ;
			TelemetryDataValid[FR_AMP_MAH] = 25 + g_model.telemetryTimeout ;
//			FrskyHubData[FR_A1_MAH] += 1 ;
//			FrskyHubData[FR_A2_MAH] += 1 ;
		}
		Frsky_Amp_hour_prescale = ah_temp ;
	}

//	// FrSky Current sensor (in amps, 1dp)
//	// add this every 10 ms, when over 3600, we have 1 mAh
//	// 
//  if (frskyUsrStreaming)
//	{
//		int16_t ah_temp ;
//		ah_temp = FrskyHubData[FR_CURRENT] ;
//		if ( ah_temp < 0 )
//		{
//			ah_temp = 0 ;			
//		}
//		ah_temp += Frsky_Amp_hour_prescale ;
//		if ( ah_temp > 3600 )
//		{
//			ah_temp -= 3600 ;
//			int16_t *ptr_hub = &FrskyHubData[FR_AMP_MAH] ;
////			FORCE_INDIRECT(ptr_hub) ;
//			*ptr_hub += 1 ;
//		}
//		Frsky_Amp_hour_prescale = ah_temp ;
////		if ( ( Frsky_Amp_hour_prescale += FrskyHubData[FR_CURRENT] ) > 3600 )
////		{
////			Frsky_Amp_hour_prescale -= 3600 ;
////			FrskyHubData[FR_AMP_MAH] += 1 ;
////		}
//	}

	// See if time for alarm checking
//	if (--FrskyAlarmTimer == 0 )
//	{
//		FrskyAlarmTimer = 200 ;		// Restart timer
//		FrskyAlarmCheckFlag = 1 ;	// Flag time to check alarms
//	}
}

// New model loaded
void FRSKY_setModelAlarms(void)
{
	FrskyBattCells[0] = 0 ;
	FrskyBattCells[1] = 0 ;
	if ( g_model.bt_telemetry < 2 )
	{
  	FrskyAlarmSendState |= 0x0F ;
	}
//#ifndef SIMU
//  Frsky_current[0].Amp_hour_boundary = 360000L/ g_model.frsky.channels[0].ratio ; // <= (!) division per 0!
//	Frsky_current[1].Amp_hour_boundary = 360000L/ g_model.frsky.channels[1].ratio ; // <= (!) division per 0!
//#endif
}

//struct FrSky_Q_t FrSky_Queue ;

//void put_frsky_q( uint8_t index, uint16_t value )
//{
//	volatile struct FrSky_Q_item_t *r ;	// volatile = Force compiler to use pointer

//	r = &FrSky_Queue.items[FrSky_Queue.in_index] ;
//	if ( FrSky_Queue.count < 7 )
//	{
//		r->index = index ;
//		r->value = value ;
//		++FrSky_Queue.in_index &= 7 ;
//		FrSky_Queue.count += 1 ;
//	}
//}

// If index bit 7 is zero - process now
// else wait until item further down q has bit 7 zero

//void process_frsky_q()
//{
//	volatile struct FrSky_Q_item_t *r ;	// volatile = Force compiler to use pointer
//	uint8_t x ;
//	uint8_t y ;
//	uint8_t z ;

//	// Find last item with zero in bit 7 of index
//	x = FrSky_Queue.count ;
//	z = FrSky_Queue.out_index ;
//	while ( x )
//	{
//		y = (z+x-1) & 0x07 ;
//		if ( ( FrSky_Queue.items[y].index & 0x80 ) == 0 )
//		{
//			break ;		
//		}
//		x -= 1 ;		
//	}
//	y = x ;
//	while ( x )
//	{
//		r = &FrSky_Queue.items[z] ;

//		storeTelemetryData( r->index & 0x7F, r->value ) ;
//		++z &= 0x07 ;
//	}
	
//	FrSky_Queue.out_index = z ;	
//	__disable_irq() ;
//	FrSky_Queue.count -= y ;
//	__enable_irq() ;

//}


//uint32_t GpsPosLat ;

//void testgps()
//{
//  GpsPosLat = (((uint32_t)(uint16_t)FrskyHubData[FR_GPS_LAT] / 100) * 1000000) + (((uint32_t)((uint16_t)FrskyHubData[FR_GPS_LAT] % 100) * 10000 + (uint16_t)FrskyHubData[FR_GPS_LATd]) * 5) / 3;
//}

uint8_t Unit ;

int16_t scale_telem_value( int16_t val, uint8_t channel, uint8_t *dplaces )
{
	int32_t value ;
  uint8_t unit = 'v' ;
	uint8_t places = 1 ;
	
	value = val ;
	uint8_t ltype = g_model.frsky.channels[channel].units ;
	uint16_t ratio = g_model.frsky.channels[channel].lratio ;
	if (ltype == 2/*V*/)	// times 2
	{
    ratio <<= 1 ;
	}
  value *= ratio ;
	if (ltype == 3/*A*/)
  {
		unit = 'A' ;
    value /= 100 ;
  }
  else if ( ratio < 100 )
  {
    value *= 2 ;
    value /= 51 ;  // Same as *10 /255 but without overflow
		places = 2 ;
  }
  else
  {
    value /= 255 ;
  }
	if ( ltype == 1 )
	{
		unit = ' ' ;
  	places = 0 ;
	}
	if ( dplaces )
	{
  	*dplaces = places ;
	}
	Unit = unit ;
	return value ;
}

// Scales A1 and/or A2 and sets dplaces to 1 or 2
uint16_t A1A2toScaledValue( uint8_t channel, uint8_t *dplaces )
{
	uint8_t val = FrskyHubData[channel ? FR_A2_COPY : FR_A1_COPY] ;
	return scale_telem_value( val, channel, dplaces ) ;
}

uint16_t logAxScale( uint8_t channel, uint8_t *dps )
{
	uint16_t val ;
	val = A1A2toScaledValue( channel, dps ) ;
	*dps = (*dps) ? 10 : 100 ; 
	return val ; 
}

//uint16_t scale_telem_value( uint16_t val, uint8_t channel, uint8_t times2, uint8_t *p_att )
//{
//  uint32_t value ;
//	uint16_t ratio ;
	
//  value = val ;
//  ratio = g_model.frsky.channels[channel].ratio ;
//  if ( times2 )
//  {
//    ratio <<= 1 ;
//  }
//  value *= ratio ;
//	if (g_model.frsky.channels[channel].type == 3/*A*/)
//  {
//    value /= 100 ;
//    *p_att |= PREC1 ;
//  }
//  else if ( ratio < 100 )
//  {
//    value *= 2 ;
//    value /= 51 ;  // Same as *10 /255 but without overflow
//    *p_att |= PREC2 ;
//  }
//  else
//  {
//    value /= 255 ;
//    *p_att |= PREC1 ;
//  }
//	return value ;
//}


uint8_t putsTelemValue(uint8_t x, uint8_t y, int16_t val, uint8_t channel, uint8_t att )
{
    int32_t value ;
    //  uint8_t ratio ;
    uint8_t dplaces ;
//    uint8_t times2 ;
    uint8_t unit = ' ' ;
    uint8_t option = att & NO_UNIT ;
		att &= ~NO_UNIT ;
//		uint8_t ltype = g_model.frsky.channels[channel].type ;

//    if ( scale )
//		{
//			value = valuetoScaledValue( val, channel, &dplaces ) ;
//		}

		value = scale_telem_value( val, channel, &dplaces ) ;
		if ( dplaces == 1 )
		{
			att |= PREC1 ;
		}
		else if ( dplaces == 2 )
		{
			att |= PREC2 ;
		}
//        ratio = g_model.frsky.channels[channel].ratio ;
//        if ( times2 )
//        {
//            ratio <<= 1 ;
//        }
//        value *= ratio ;
//		  	if (g_model.frsky.channels[channel].type == 3/*A*/)
//        {
//            value /= 100 ;
//            att |= PREC1 ;
//        }
//        else if ( ratio < 100 )
//        {
//            value *= 2 ;
//            value /= 51 ;  // Same as *10 /255 but without overflow
//            att |= PREC2 ;
//        }
//        else
//        {
//            value /= 255 ;
//        }
    if ( Unit == 'v' ) //ltype == 0/*v*/) || (ltype == 2/*v*/) )
//    if ( ( ltype == 0/*v*/) || ( ltype == 2/*v*/) )
    {
      lcd_outdezNAtt(x, y, value, att, 5) ;
//			unit = 'v' ;
      if(!(option&NO_UNIT)) lcd_putcAtt(Lcd_lastPos, y, unit, att);
    }
    else
    {
      lcd_outdezAtt(x, y, value, att);
	    if ( Unit == 'A')
//	    if ( ltype == 3/*A*/)
			{
//					unit = 'A' ;
       	if(!(option&NO_UNIT)) lcd_putcAtt(Lcd_lastPos, y, unit, att);
			}
    }
		return unit ;
}

#ifdef XFIRE

uint8_t crc8(const uint8_t * ptr, uint32_t len) ;

static bool checkCrossfireTelemetryFrameCRC()
{
  uint8_t len = frskyRxBuffer[1] ;
  uint8_t crc = crc8(&frskyRxBuffer[2], len-1) ;
  return (crc == frskyRxBuffer[len+1]) ;
}

template<int N>
bool getCrossfireTelemetryValue(uint8_t index, uint32_t &value)
{
  bool result = false ;
  value = 0 ;
  uint8_t *byte = &frskyRxBuffer[index] ;
  for ( uint32_t i = 0 ; i < N ; i += 1 )
	{
    value <<= 8 ;
    if (*byte != 0xff)
		{
      result = true ;
    }
    value += *byte++ ;
  }
  return result ;
}

uint32_t crossfireGpsConvert( uint32_t value )
{
	uint16_t degrees ;
	uint16_t minutes ;
	uint32_t result ;

	degrees = value / 10000000 ;
	value %= 10000000 ;		// Fractions of a degree
	value *= 60 ;
	minutes = value / 10000000 ;
	result = ( degrees * 100 + minutes ) << 16 ;
	value %= 10000000 ;		// Fractions of a minute
	value /= 1000 ;
	return result | value ;
}

void processCrossfireTelemetryFrame()
{
//	XFDebug1 += 1 ;
  if (!checkCrossfireTelemetryFrameCRC())
	{
    return ;
  }
//	XFDebug2 += 1 ;

  uint8_t id = frskyRxBuffer[2] ;
  uint32_t value ;
  switch(id)
	{
    case CRSF_GPS_ID:
      frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
      if (getCrossfireTelemetryValue<4>(3, value))
			{
				uint8_t code = 'N' ;
				int32_t ivalue = value ;
				if ( ivalue < 0 )
				{
					code = 'S' ;
					ivalue = -ivalue ;
				}
				value = ivalue ;
//        processCrossfireTelemetryValue(GPS_LATITUDE_INDEX, value/10);
				value = crossfireGpsConvert( value ) ;
				storeTelemetryData( FR_GPS_LAT, value >> 16 ) ;
				storeTelemetryData( FR_GPS_LATd, value ) ;
				storeTelemetryData( FR_LAT_N_S, code ) ;
			}
      if (getCrossfireTelemetryValue<4>(7, value))
			{
				uint8_t code = 'E' ;
				int32_t ivalue = value ;
				if ( ivalue < 0 )
				{
					code = 'W' ;
					ivalue = -ivalue ;
				}
				value = ivalue ;
//        processCrossfireTelemetryValue(GPS_LONGITUDE_INDEX, value/10);
				value = crossfireGpsConvert( value ) ;
				storeTelemetryData( FR_GPS_LONG, value >> 16 ) ;
				storeTelemetryData( FR_GPS_LONGd, value ) ;
				storeTelemetryData( FR_LONG_E_W, code ) ;
			}	
//      if (getCrossfireTelemetryValue<2>(11, value))
//        processCrossfireTelemetryValue(GPS_GROUND_SPEED_INDEX, value);
      if (getCrossfireTelemetryValue<2>(13, value))
			{
//        processCrossfireTelemetryValue(GPS_HEADING_INDEX, value);
					storeTelemetryData( FR_COURSE, value ) ;
			}
      if (getCrossfireTelemetryValue<2>(15, value))
			{
//        processCrossfireTelemetryValue(GPS_ALTITUDE_INDEX,  value - 1000);
				storeTelemetryData( FR_SPORT_GALT, value - 1000 ) ;
				
			}
      if (getCrossfireTelemetryValue<1>(17, value))
			{
//        processCrossfireTelemetryValue(GPS_SATELLITES_INDEX, value);
				
			}
    break;

    case CRSF_LINK_ID :
//			XFDebug3 += 1 ;
      frskyStreaming = FRSKY_TIMEOUT10ms ;
      for ( uint32_t i=0 ; i<=TX_SNR_INDEX; i += 1 )
			{
        if (getCrossfireTelemetryValue<1>(3+i, value))
				{
//          if (i == TX_POWER_INDEX)
//					{
//            static const uint32_t power_values[] = { 0, 10, 25, 100, 500, 1000, 2000 } ;
//            value = (value < DIM(power_values) ? power_values[value] : 0) ;
//          }
//          processCrossfireTelemetryValue(i, value) ;
  				if ( i == TX_QUALITY_INDEX )
					{
			    	frskyTelemetry[3].set(value, FR_TXRSI_COPY ) ;	// TSSI
					}
					
          if ( i == RX_QUALITY_INDEX )
					{
						storeRSSI( value ) ;
//            telemetryData.rssi.set(value) ;
          }
					if ( i < 6 )
					{
						storeTelemetryData( FR_CUST1 + i, value ) ;
					}
        }
      }
    break ;

    case CRSF_BATTERY_ID :
      frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
      if (getCrossfireTelemetryValue<2>( 3, value) )
			{
				storeTelemetryData( FR_VOLTS, value ) ;
			}
//        processCrossfireTelemetryValue(BATT_VOLTAGE_INDEX, value);
      if (getCrossfireTelemetryValue<2>(5, value))
			{
				storeTelemetryData( FR_CURRENT, value ) ;
			}
//        processCrossfireTelemetryValue(BATT_CURRENT_INDEX, value);
      if (getCrossfireTelemetryValue<3>(7, value))
			{
				storeTelemetryData( FR_AMP_MAH, value ) ;
			}
//      if (getCrossfireTelemetryValue<3>(7, value))
//        processCrossfireTelemetryValue(BATT_CAPACITY_INDEX, value);
    break ;

    case CRSF_ATTITUDE_ID:
      frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
      if (getCrossfireTelemetryValue<2>(3, value))
			{
//        processCrossfireTelemetryValue(ATTITUDE_PITCH_INDEX, value/10);
				storeTelemetryData( FR_ACCX, value / 10 ) ;
			}	
      if (getCrossfireTelemetryValue<2>(5, value))
			{
//        processCrossfireTelemetryValue(ATTITUDE_ROLL_INDEX, value/10);
				storeTelemetryData( FR_ACCY, value / 10 ) ;
			}	
      if (getCrossfireTelemetryValue<2>(7, value))
			{
//        processCrossfireTelemetryValue(ATTITUDE_YAW_INDEX, value/10);
				storeTelemetryData( FR_ACCZ, value / 10 ) ;
			}	
    break;

//    case FLIGHT_MODE_ID:
//    {
//      const CrossfireSensor & sensor = crossfireSensors[FLIGHT_MODE_INDEX];
//      for (int i=0; i<min<int>(16, telemetryRxBuffer[1]-2); i+=4) {
//        uint32_t value = *((uint32_t *)&telemetryRxBuffer[3+i]);
//        setTelemetryValue(TELEM_PROTO_CROSSFIRE, sensor.id, 0, sensor.subId, value, sensor.unit, i);
//      }
//      break;
//    }

#if defined(LUA) || defined(BASIC)
    default:
		{
			uint8_t *packet = &frskyRxBuffer[1] ;
			uint8_t len = *packet ;	// # bytes to copy, add length, drop crc

			if ( fifo128Space( &Script_fifo ) >= len )
			{
				uint32_t i ;
				for ( i = 0 ; i < len ; i += 1 )
				{
					put_fifo128( &Script_fifo, *packet++ ) ;
				}
			}
		}
    break;
#endif
//#if defined(LUA)
//    default:
//      if (luaInputTelemetryFifo && luaInputTelemetryFifo->hasSpace(telemetryRxBufferCount-2) ) {
//        for (uint8_t i=1; i<telemetryRxBufferCount-1; i++) {
//          // destination address and CRC are skipped
//          luaInputTelemetryFifo->push(telemetryRxBuffer[i]);
//        }
//      }
//    break;
//#endif
  }
}

void processCrossfireTelemetryData(uint8_t data)
{
//	XFDebug4 += 1 ;
	
	if ( g_model.telemetryProtocol == TELEMETRY_MAVLINK )
	{
		mavlinkReceive( data ) ;
		return ;
	}
	
  uint8_t numbytes = numPktBytes ;
	
  if ( numbytes == 0 && data != RADIO_ADDRESS)
	{
		return ;
	}
	
  if ( numbytes == 1 && (data < 2 || data > TELEMETRY_RX_PACKET_SIZE-2))
	{
    numPktBytes = 0 ;
    return ;
  }
  
	if ( numbytes < TELEMETRY_RX_PACKET_SIZE)
	{
    frskyRxBuffer[numbytes++] = data ;
  }
  else
	{
    numbytes = 0 ;
  }
  
	if ( numbytes > 4)
	{
    uint8_t length = frskyRxBuffer[1] ;
    if (length + 2 == numbytes )
		{
      processCrossfireTelemetryFrame() ;
      numbytes = 0;
    }
  }
	numPktBytes = numbytes ;
}

#endif
