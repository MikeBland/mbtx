 /*
 * Authors - Mike Blandford
 * Bertrand Songis <bsongis@gmail.com>, Bryan J.Rentoul (Gruvin) <gruvin@gmail.com> and Philip Moss
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

#include "er9x.h"
#include "frsky.h"
#include "menus.h"
#include <stdlib.h>

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
#define PRIVATE					0x1B

#define MPRIVATE				'M'

// SPORT defines
#define DATA_FRAME         0x10

#ifdef N2F
	#define HUB_START_STOP      0x5e
	#define FRSKYHUB		0x5e			//???
	#define BYTECANCEL 		0x18
#endif

// Translate hub data positions
// Add a top bit, first word of two word value
// When top bit found, just save index(with top bit) and value
// When next value received, if saved value has top bit set, stoore the value and clear top bit
//   then process latest values, saving them if necessary.

// To allow either 0 or 0x80
#define FR_INDEX_FLAG		0

const prog_uint8_t APM Fr_indices[] = 
{
	HUBDATALENGTH-1,
	FR_GPS_ALT | FR_INDEX_FLAG,
	FR_TEMP1,
	FR_RPM,
	FR_FUEL,
	FR_TEMP2,
	FR_CELL_V,
#if defined(CPUM128) || defined(CPUM2561)
	FR_VCC,           // 0x07  Extra data for Mavlink via FrSky
	HUBDATALENGTH-1,
#else
	HUBDATALENGTH-1,HUBDATALENGTH-1,
#endif
	FR_GPS_ALTd,
#if defined(CPUM128) || defined(CPUM2561)
/* Extra data 1 for Mavlink via FrSky */
	FR_HOME_DIR,      // 0x0A
	FR_HOME_DIST,     // 0x0B
	FR_CPU_LOAD,      // 0x0C
	FR_GPS_HDOP,      // 0x0D
	FR_WP_NUM,        // 0x0E
	FR_WP_BEARING,    // 0x0F
/* Extra data 1 for Mavlink via FrSky */
#else
	HUBDATALENGTH-1,HUBDATALENGTH-1,		// 10,11
	HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,
#endif
	FR_ALT_BARO | FR_INDEX_FLAG,
	FR_GPS_SPEED | FR_INDEX_FLAG,
	FR_GPS_LONG | FR_INDEX_FLAG,
	FR_GPS_LAT | FR_INDEX_FLAG,
	FR_COURSE,			// 20
	FR_GPS_DATMON,
	FR_GPS_YEAR,
	FR_GPS_HRMIN,
	FR_GPS_SEC,
	FR_GPS_SPEEDd,
	FR_GPS_LONGd,
	FR_GPS_LATd,
	FR_COURSEd,			// 28
#if defined(CPUM128) || defined(CPUM2561)
/* Extra data 2 for Mavlink via FrSky */
	FR_BASEMODE,
	FR_WP_DIST,
	FR_HEALTH,
	FR_MSG,
/* Extra data 2 for Mavlink via FrSky */
#else
	HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,
#endif
	FR_ALT_BAROd,		// 33
	FR_LONG_E_W,
	FR_LAT_N_S,
	FR_ACCX,
	FR_ACCY,
	FR_ACCZ,
	FR_VSPD,
	FR_CURRENT,			// 40
	FR_V_AMP | FR_INDEX_FLAG,
	FR_V_AMPd,
	FR_VOLTS,
	HUBDATALENGTH-1,		// 44
} ;

uint8_t TxLqi ;

uint8_t TmOK ;

uint8_t AltitudeDecimals ;
int16_t WholeAltitude ;
//uint8_t A1Received = 0 ;

#define FRSKY_SPORT_PACKET_SIZE		9

#define PRIVATE_BUFFER_SIZE	30

uint8_t frskyRxBuffer[PRIVATE_BUFFER_SIZE];   // Receive buffer. 9 bytes (full packet), worst case 18 bytes with byte-stuffing (+1)

struct t_frskyTx
{
#ifndef V2
	uint8_t frskyTxBuffer[13];   // Ditto for transmit buffer
#else
	uint8_t frskyTxBuffer[6];   // V2 only sends 4 bytes of private data
#endif
	uint8_t frskyTxBufferCount ;
	uint8_t frskyTxISRIndex ;
} FrskyTx ;

//uint8_t FrskyRxBufferReady = 0;
uint8_t frskyStreaming = 0;
uint8_t frskyUsrStreaming = 0;

FrskyData frskyTelemetry[4];
//FrskyData frskyRSSI[2];
//Frsky_current_info Frsky_current[2] ;

#ifndef V2
uint8_t frskyRSSIlevel[2] ;
uint8_t frskyRSSItype[2] ;
#endif // V2

struct t_frsky_user
{
	uint8_t state ;
	uint8_t stuff ;
	uint8_t id ;
	uint8_t lobyte ;
	uint8_t hibyte ;
	uint8_t ready ;
} Frsky_user;


int16_t FrskyHubData[HUBDATALENGTH] ;  // All 38 words
struct t_hub_max_min FrskyHubMaxMin ;

#define FRSKY_VOLTS_SIZE	6
uint8_t FrskyVolts[FRSKY_VOLTS_SIZE];
uint8_t FrskyBattCells=0;
int16_t Frsky_Amp_hour_prescale ;

uint8_t FrskyTelemetryType ;

uint16_t DsmABLRFH[6] ;

#if defined(CPUM128) || defined(CPUM2561)
uint8_t TelRxCount ;
#endif
#ifdef AF_DEBUG
uint16_t TelRxCount ;
uint8_t AfDebug[20] ;
uint16_t AfCount ;
uint8_t AfCapture ;
uint8_t AfBytes ;
#endif

#if defined(VARIO)
struct t_vario VarioData ;
#endif

#if defined(VARIO)
void evalVario(int16_t altitude_bp, uint16_t altitude_ap)
{
	struct t_vario *vptr ;
	vptr = &VarioData ;

  int32_t varioAltitude_cm = (int32_t)altitude_bp * 100 + (altitude_bp > 0 ? altitude_ap : -altitude_ap) ;
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
//	FrskyHubData[FR_VSPD] = vptr->VarioSpeed ;
}
#endif

void store_hub_data( uint8_t index, uint16_t value ) ;

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
		index = 43 ; // FR_VOLTS ;		// Move Oxsensor voltage
	}
	if ( index > sizeof(Fr_indices) )
	{
		index = 0 ;	// Use a discard item							
	}
#if FR_INDEX_FLAG
  index = pgm_read_byte( &Fr_indices[index] ) & 0x7F ;
#else
  index = pgm_read_byte( &Fr_indices[index] ) ;
#endif
	store_hub_data( index, value ) ;
}

#if defined(EXTEND_SCALERS)
void store_telemetry_scaler( uint8_t index, uint16_t value )
{
	switch ( index )
	{
		case 1 :
			store_hub_data( FR_BASEMODE, value ) ;
		break ;
		case 2 :
			store_hub_data( FR_CURRENT, value ) ;
		break ;
		case 3 :
			store_hub_data( FR_AMP_MAH, value ) ;
		break ;
		case 4 :
			store_hub_data( FR_VOLTS, value ) ;
		break ;
		case 5 :
			store_hub_data( FR_FUEL, value ) ;
		break ;
	}
}
#endif


NOINLINE void store_cell_data( uint8_t battnumber, uint16_t cell )
{
	if ( battnumber < FRSKY_VOLTS_SIZE )
	{
		FrskyVolts[battnumber] = ( cell & 0x0FFF ) / 10 ;
	}
}

void store_hub_data( uint8_t index, uint16_t value )
{
	if ( index == FR_ALT_BARO )
	{
		int16_t val = (int16_t)value ;
		val *= 10 ;
		if ( AltitudeDecimals )
		{
			WholeAltitude = val ;
			index = FR_TRASH ;
		}
		value = val ;
	}
	if ( index == FR_ALT_BAROd )
	{
		int16_t val = (int16_t)value ;
		AltitudeDecimals |= 1 ;
		if ( ( val > 9 ) || ( val < -9 ) )
		{
			AltitudeDecimals |= 2 ;
		}
		if ( AltitudeDecimals & 2 )
		{
			val /= 10 ;			
		}
		FrskyHubData[FR_ALT_BARO] = WholeAltitude + val ;
		index = FR_ALT_BARO ;         // For max and min
	}

	if ( index == FR_SPORT_ALT )
	{
		FrskyHubData[FR_ALT_BARO] = value ;
		index = FR_ALT_BARO ;         // For max and min
	}
	
//#if defined(CPUM128) || defined(CPUM2561)
	if ( index == FR_SPORT_GALT )
	{
		index = FR_GPS_ALT ;         // For max and min
		FrskyHubData[FR_GPS_ALT] = value ;
	}
//#endif

	if ( index < HUBDATALENGTH )
	{
#ifndef NOGPSALT
    if ( !g_model.FrSkyGpsAlt )         
    {
			FrskyHubData[index] = value ;
    }                     
		else
		{
      if ( index != FR_ALT_BARO )
			{
			  FrskyHubData[index] = value ;           /* ReSt */
			}
      if ( index == FR_GPS_ALT )
      {
         FrskyHubData[FR_ALT_BARO] = FrskyHubData[FR_GPS_ALT] * 10 ;      // Copy Gps Alt instead
         index = FR_ALT_BARO ;         // For max and min
      }
		}
#else
		FrskyHubData[index] = value ;
#endif
    
#if defined(VARIO)
		if ( index == FR_ALT_BARO )
		{
			evalVario( value, 0 ) ;
		}
#endif

		if ( index == FR_CURRENT )			// FAS current
		{
			if ( value > g_model.frsky.FASoffset )
			{
				value -= g_model.frsky.FASoffset ;
			}
			else
			{
				value = 0 ;
			}
			FrskyHubData[index] = value ;
		}

		if ( index < HUBMINMAXLEN )
		{
			struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;
			FORCE_INDIRECT( maxMinPtr) ;
			
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
			uint8_t tempCells = battnumber+1 ;
  		if (FrskyBattCells < tempCells)
			{
 				if (tempCells>=FRSKY_VOLTS_SIZE)
				{
  				FrskyBattCells=FRSKY_VOLTS_SIZE;
  			}
				else
				{
  				FrskyBattCells=tempCells;
  			}
  		}
			store_cell_data( battnumber, ( ( value & 0x0F ) << 8 ) + (value >> 8) ) ;
		}
		if ( index == FR_RPM )			// RPM
		{
			uint32_t x ;

			x = FrskyHubData[FR_RPM] ;
			x *= 60 ;
			uint8_t b = g_model.numBlades ;
			if ( b == 0 )
			{
				b = 1 ;
				g_model.numBlades = b ;
			}
			FrskyHubData[FR_RPM] = x / b ;
		}
		if ( index == FR_V_AMPd )			// RPM
		{
			FrskyHubData[FR_VOLTS] = (FrskyHubData[FR_V_AMP] * 10 + value) * 21 / 11 ;
		}
	}	
}


void frsky_proc_user_byte( uint8_t byte )
{
	struct t_frsky_user *fUserPtr = &Frsky_user ;
	FORCE_INDIRECT( fUserPtr ) ;

	if (g_model.FrSkyUsrProto == 0)  // FrSky Hub
	{
	
  	if ( fUserPtr->state == 0 )
		{ // Waiting for 0x5E
			if ( byte == 0x5E )
			{
				fUserPtr->state = 1 ;
				if ( fUserPtr->ready )
				{
					fUserPtr->ready = 0 ;
					store_indexed_hub_data( fUserPtr->id, ( fUserPtr->hibyte << 8 ) | fUserPtr->lobyte ) ;
				}
			}		
		}
		else
		{ // In a packet
			if ( byte == 0x5E )
			{ // 
				fUserPtr->state = 1 ;
			}
			else
			{
				if ( byte == 0x5D )
				{
					fUserPtr->stuff = 1 ;  // Byte stuffing active
				}
				else
				{
					if ( fUserPtr->stuff )
					{
						fUserPtr->stuff = 0 ;
						byte ^= 0x60 ;  // Unstuff
					}
  	      if ( fUserPtr->state == 1 )
					{
					  fUserPtr->id	= byte ;
						fUserPtr->state = 2 ;
					}
  	      else if ( fUserPtr->state == 2 )
					{
					  fUserPtr->lobyte	= byte ;
						fUserPtr->state = 3 ;
					}
					else
					{
						fUserPtr->hibyte = byte ;
						fUserPtr->ready = 1 ;
						fUserPtr->state = 0 ;
					}
				}
			}		 
		}
	}
	else // if (g_model.FrSkyUsrProto == 1)  // WS How High
	{
    if ( frskyUsrStreaming < (FRSKY_USR_TIMEOUT10ms - 10))  // At least 100mS passed since last data received
		{
			fUserPtr->lobyte = byte ;
		}
		else
		{
			int16_t value ;
			value = ( byte << 8 ) | fUserPtr->lobyte ;
			store_hub_data( FR_ALT_BARO, value ) ;	 // Store altitude info
#if defined(VARIO)
			evalVario( value, 0 ) ;
#endif

		}				
	}
}

#ifndef V2
static uint8_t frskyPushValue( uint8_t i, uint8_t value);
#endif // V2
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

//uint8_t LinkAveCount ;

void processFrskyPacket(uint8_t *packet)
{
  // What type of packet?
  switch (packet[0])
  {
    case LINKPKT: // A1/A2/RSSI values
			// From a scope, this seems to be sent every about every 35mS
//			LinkAveCount += 1 ;
      frskyTelemetry[0].set(packet[1], FR_A1_COPY ); //FrskyHubData[] =  frskyTelemetry[0].value ;
      frskyTelemetry[1].set(packet[2], FR_A2_COPY ); //FrskyHubData[] =  frskyTelemetry[1].value ;
      frskyTelemetry[2].set(packet[3], FR_RXRSI_COPY );	//FrskyHubData[] =  frskyTelemetry[2].value ;
      frskyTelemetry[3].set(packet[4] / 2, FR_TXRSI_COPY ); //FrskyHubData[] =  frskyTelemetry[3].value ;
			setTxLqi( packet[6] ) ;

//			if ( LinkAveCount > 15 )
//			{
//				LinkAveCount = 0 ;
//			}
//      frskyRSSI[0].set(packet[3]);
//      frskyRSSI[1].set(packet[4] / 2);
      break;

#ifndef V2
		case RSSIRXPKT :
			frskyRSSIlevel[1] = packet[1] ;
			frskyRSSItype[1] = packet[3] ;
		break ;

		case RSSITXPKT :
			frskyRSSIlevel[0] = packet[1] ;
			frskyRSSItype[0] = packet[3] ;
		break ;
#endif // V2

    case USRPKT: // User Data packet
    {
			uint8_t i, j ;
			i = (packet[1] & 0x07) + 3 ;  // User bytes end
			j = 3 ;              // Index to user bytes
			while ( j < i )
			{
				frsky_proc_user_byte( packet[j] ) ;
      	frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
				j += 1 ;
			}
    }	
    break;

// Support native FrSky Sensor Hub protocol
#if defined(N2F)
   case FRSKYHUB:				//!!!!!!!!!!!!!!!!!!!!!! Found Start of Packet= 0x5E
    {
		uint8_t i, j ;
		i = 4 ;		  // User bytes end
		j = 0 ;              // Index to user bytes
		while ( j < i )
		{
			frsky_proc_user_byte( packet[j] ) ;
			// reset counter only if valid frsky packets are being detected
			j += 1 ;
		}
		frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms; 
    }	
    break;					//!!!!!!!!!!!!!!!!!!
#endif

  }

//  FrskyRxBufferReady = 0;
  frskyStreaming = FRSKY_TIMEOUT10ms; // reset counter only if valid frsky packets are being detected
}

// DSM data

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


// Receive buffer state machine state defs
#define frskyDataIdle    0
#define frskyDataStart   1
#define frskyDataInFrame 2
#define frskyDataXOR     3

static bool checkSportPacket()
{
	uint8_t *packet = frskyRxBuffer ;
  uint16_t crc = 0 ;
  for ( uint8_t i=1; i<FRSKY_SPORT_PACKET_SIZE; i++)
	{
    crc += packet[i]; //0-1FF
    crc += crc >> 8; //0-100
    crc &= 0x00ff;
  }
  return (crc == 0x00ff) ;
}


void processSportData( void ) ;

void processSportPacket()
{
//  uint16_t appId  = *((uint16_t *)(packet+2)) ;

  if ( !checkSportPacket() )
	{
    return;
	}
	processSportData() ;
}

// The leading 0x7E is not in the packet, first byte is physical ID
void processSportData()
{
	uint8_t *packet = frskyRxBuffer ;
	FORCE_INDIRECT(packet) ;
	uint8_t  prim   = packet[1];
  
	frskyStreaming = FRSKY_TIMEOUT10ms * 3 ; // reset counter only if valid frsky packets are being detected

	if ( prim == DATA_FRAME )
	{
		
		if ( packet[3] == 0xF1 )
		{ // Receiver specific
			uint8_t value = packet[4] ;
			switch ( packet[2] )
			{
				case 1 :
      		frskyTelemetry[2].set(value, FR_RXRSI_COPY );	//FrskyHubData[] =  frskyTelemetry[2].value ;
					setTxLqi( packet[7] ) ;			// packet[7] is  TX_LQI for MULTI
				break ;

				case 4 :		// Battery from X8R
//		      frskyTelemetry[0].set(value, FR_A1_COPY ); //FrskyHubData[] =  frskyTelemetry[0].value ;
					store_hub_data( FR_RXV, value * 132 / 255 ) ;
//					if ( A1Received )
//					{
						break ;
//					}
				case 2 :
		      frskyTelemetry[0].set(value, FR_A1_COPY ); //FrskyHubData[] =  frskyTelemetry[0].value ;
//					A1Received = 1 ;
				break ;
  		    
				case 3 :
					frskyTelemetry[1].set(value, FR_A2_COPY ); //FrskyHubData[] =  frskyTelemetry[1].value ;
				break ;
    		  
				case 5 : // SWR
					frskyTelemetry[3].set(value, FR_TXRSI_COPY ); //FrskyHubData[] =  frskyTelemetry[3].value ;
				break ;
			}
		}
		else if ( packet[3] == 0 )
		{ // old sensors
      frskyUsrStreaming = 255 ; //FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
			uint16_t value = (*((uint16_t *)(packet+4))) ;
			store_indexed_hub_data( packet[2], value ) ;
		}
		else
		{ // new sensors
      frskyUsrStreaming = 255 ; //FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
			uint8_t id = (packet[3] << 4) | ( packet[2] >> 4 ) ;
			uint32_t value = (*((uint32_t *)(packet+4))) ;
//			SportId = id ;
//			SportValue = value ;
			switch ( id )
			{
				case ALT_ID_8 :
					value = (int32_t)value / 10 ;
					store_hub_data( FR_SPORT_ALT, value ) ;
				break ;

				case VARIO_ID_8 :
					store_hub_data( FR_VSPD, value ) ;
				break ;

#if defined(CPUM128) || defined(CPUM2561)
				case BETA_ALT_ID_8 :
					value = (int32_t)value >> 8 ;
					value = (int32_t)value / 10 ;
					store_hub_data( FR_SPORT_ALT, value ) ;
				break ;
#endif
//				case BETA_VARIO_ID_8 :
//					value = (int32_t)value >> 8 ;
//					store_hub_data( FR_VSPD, value ) ;
//				break ;

				case CELLS_ID_8 :
				{
        	uint8_t battnumber = value ;
					uint16_t cell ;
  				FrskyBattCells = battnumber >> 4 ;
					battnumber &= 0x0F ;
					value >>= 8 ;
					cell = value ;
					store_cell_data( battnumber, cell ) ;
					battnumber += 1 ;
					value >>= 12 ;
					cell = value ;
					store_cell_data( battnumber, cell ) ;
				}
				break ;

				case CURR_ID_8 :
					store_hub_data( FR_CURRENT, value ) ;
				break ;

				case VFAS_ID_8 :
					store_hub_data( FR_VOLTS, value / 10 ) ;
				break ;

				case A3_ID_8 :
					store_hub_data( FR_A3, value ) ;
				break ;

				case A4_ID_8 :
					store_hub_data( FR_A4, value ) ;
				break ;

				case T1_ID_8 :
					store_hub_data( FR_TEMP1, value ) ;
				break ;
				
				case T2_ID_8 :
					store_hub_data( FR_TEMP2, value ) ;
				break ;

				case RPM_ID_8 :
					store_hub_data( FR_RPM, value ) ;
				break ;
				
#if defined(CPUM128) || defined(CPUM2561)
				case ACCX_ID_8 :
					store_hub_data( FR_ACCX, value ) ;
				break ;
					
				case ACCY_ID_8 :
					store_hub_data( FR_ACCY, value ) ;
				break ;
				
				case ACCZ_ID_8 :
					store_hub_data( FR_ACCZ, value ) ;
				break ;

				case FUEL_ID_8 :
					store_hub_data( FR_FUEL, value ) ;
				break ;
#endif // 128/2561

				case GPS_ALT_ID_8 :
					value = (int32_t)value / 100 ;
					store_hub_data( FR_SPORT_GALT, value ) ;
				break ;

				case GPS_LA_LO_ID_8 :
				{	
//					Bits 31-30 00 = LAT min/10000 N
//					Bits 31-30 01 = LAT min/10000 S
//					Bits 31-30 10 = LON min/10000 E
//					Bits 31-30 11 = LON min/10000 W
					uint8_t code = value >> 30 ;
					value &= 0x3FFFFFFF ;
					uint16_t bp ;
					uint16_t ap ;
					uint32_t temp ;

//					ldiv_t qr ;
//					qr = div( value, 10000UL ) ;
//					ap = qr.rem ;
//					bp = (qr.quot/ 60 * 100) + (qr.quot % 60) ;

					temp = value / 10000 ;
					bp = (temp/ 60 * 100) + (temp % 60) ;
		      ap = value % 10000;
					if ( code & 2 )	// Long
					{
						store_hub_data( FR_GPS_LONG, bp ) ;
						store_hub_data( FR_GPS_LONGd, ap ) ;
						store_hub_data( FR_LONG_E_W, ( code & 1 ) ? 'W' : 'E' ) ;
					}
					else
					{
						store_hub_data( FR_GPS_LAT, bp ) ;
						store_hub_data( FR_GPS_LATd, ap ) ;
						store_hub_data( FR_LAT_N_S, ( code & 1 ) ? 'S' : 'N' ) ;
					}
				}
				break ;
				
				case GPS_HDG_ID_8 :
					store_hub_data( FR_COURSE, value / 100 ) ;
				break ;

				case GPS_SPEED_ID_8 :
					store_hub_data( FR_GPS_SPEED, value/1000 ) ;
				break ;

			}
		}
	}
	asm("") ;
}

#if defined(CPUM128) || defined(CPUM2561)
uint8_t AfhdsData[2] ;
#endif

#define FS_ID_ERR_RATE					0xfe
#define FS_ID_SNR               0xfa
#define FS_ID_NOISE             0xfb

// First byte in packet is 0xAA
void processAFHDS2Packet(uint8_t *packet, uint8_t byteCount)
{
#if defined(CPUM128) || defined(CPUM2561)
	AfhdsData[0] = *packet ;
	AfhdsData[1] = *(packet+1) ;
#endif

#ifdef AF_DEBUG
	AfCount += 1 ;
#endif
  
	frskyTelemetry[3].set(packet[1], FR_TXRSI_COPY ) ;	// TSSI
	if ( *packet == 0xAC )
	{
		return ;
//    	value = (packet[6] << 24) | (packet[5] << 16) | (packet[4] << 8) | packet[3];
//			if ( id == 0x83 )
//			{
//				// Baro Alt
//			}
	}
	else
	{
		packet += 2 ;
  	for (uint32_t sensor = 0; sensor < 7; sensor++)
		{
			uint16_t id ;
			int16_t value ;
			id = *packet ;
			id |= *(packet+1) << 8 ;
			packet += 2 ;
			value = (packet[1] << 8)  + packet[0] ;
			switch ( id )
			{
				case 0 :	// Rx voltage
					store_hub_data( FR_RXV, value/10 ) ;
				break ;
				case 1 :	// Temp
					store_hub_data( FR_TEMP1, value ) ;
				break ;
				case 2 :	// RPM
					store_hub_data( FR_RPM, value ) ;
				break ;
				case 0x0100 :	// External voltage ?
					store_hub_data( FR_VOLTS, value/10 ) ;
				break ;
				case 3 :	// External voltage
				case 0x0103 :	// External voltage ?
					store_hub_data( FR_VOLTS, value/10 ) ;
				break ;
				case 0xFC :	// RSSI
	//				storeRSSI( 135-value ) ;
					frskyTelemetry[2].set( 135-value, FR_RXRSI_COPY );	//FrskyHubData[] =  frskyTelemetry[2].value ;
				break ;
	//			case FS_ID_ERR_RATE :
	//				store_hub_data( FR_CUST1, value ) ;
	//			break ;
	//			case FS_ID_SNR :
	//				store_hub_data( FR_CUST2, value ) ;
	//			break ;
	//			case FS_ID_NOISE :
	//				store_hub_data( FR_CUST3, value ) ;
	//			break ;
			}
			packet += 2 ;
		}
 		frskyUsrStreaming = 255 ; // reset counter only if valid packets are being detected
 		frskyStreaming = 255 ; // reset counter only if valid packets are being detected
	}
}


//#endif

#if defined(DSM_BIND_RESPONSE)


void dsmBindResponse( uint8_t mode, int8_t channels )
{
	// Process mode here
	uint8_t dsm_mode_response ;
extern uint8_t MultiResponseData ;
	dsm_mode_response = channels ;
	if ( mode & 0x80 )
	{
		dsm_mode_response |= 0x80 ;
	}
	if ( mode & 0x10 )
	{
		dsm_mode_response |= 0x40 ;
	}
	MultiResponseData = dsm_mode_response ;
extern uint8_t MultiResponseFlag ;
	MultiResponseFlag = 1 ;
}

#if defined(CPUM2561)
uint8_t DsmCopyData[20] ;
#endif

void processDsmPacket(uint8_t *packet, uint8_t byteCount)
{
	int16_t ivalue ;
	uint8_t type ;

#if defined(CPUM2561)
	if ( packet[2] == 0x7F )
	{
		memmove( DsmCopyData, packet, 18 ) ;
		DsmCopyData[19] = byteCount ;
	}
#endif
		 
	packet += 1 ;			// Skip the 0xAA
	type = *packet++ ;
	
	if ( type && (type < 0x20) )
	{
		
//   	frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid packets are being detected
//  	frskyStreaming = FRSKY_TIMEOUT10ms; // reset counter only if valid packets are being detected

   	frskyUsrStreaming = 255 ; // reset counter only if valid packets are being detected
  	frskyStreaming = 255 ; // reset counter only if valid packets are being detected

		ivalue = (int16_t) ( (packet[2] << 8 ) | packet[3] ) ;
   	
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
				store_hub_data( FR_ALT_BARO, ivalue ) ;	 // Store altitude info
//				storeAltitude( ivalue ) ;
			break ;
			
			case DSM_AMPS :
				ivalue *= 2015 ;
				ivalue /= 1024 ;
				store_hub_data( FR_CURRENT, ivalue ) ;	// Handles FAS Offset
			break ;
	
			case DSM_PBOX :
				FrskyHubData[FR_VOLTS] = (uint16_t)ivalue / 10 ;
				ivalue = (int16_t) ( (packet[6] << 8 ) | packet[7] ) ;
 				FrskyHubData[FR_AMP_MAH] = ivalue ;
			break ;
		
			case DSM_GFORCE :
				// check units (0.01G)
				FrskyHubData[FR_ACCX] = ivalue ;
				ivalue = (int16_t) ( (packet[4] << 8 ) | packet[5] ) ;
				FrskyHubData[FR_ACCY] = ivalue ;
				ivalue = (int16_t) ( (packet[6] << 8 ) | packet[7] ) ;
				FrskyHubData[FR_ACCZ] = ivalue ;
			break ;

			case DSM_VTEMP1 :
			case DSM_VTEMP2 :
				// RPM
				if ( (uint16_t)ivalue == 0xFFFF )
				{
					ivalue = 0 ;
				}
				else
				{
					ivalue = 120000000L / g_model.numBlades / (uint16_t)ivalue ;
				}
				FrskyHubData[FR_RPM] = ivalue ;
				// volts
				ivalue = (int16_t) ( (packet[4] << 8 ) | packet[5] ) ;
//				FrskyHubData[FR_A2_COPY] = ivalue ;
				FrskyHubData[FR_VOLTS] = (uint16_t)ivalue / 10 ;
				ivalue = (int16_t) ( (packet[6] << 8 ) | packet[7] ) ;
				FrskyHubData[FR_TEMP1] = (ivalue-32)*5/9 ;
			break ;

			case DSM_STAT1 :
			case DSM_STAT2 :
				if ( g_model.dsmAasRssi )
				{
					if ( ivalue < 1000 )
					{
   					frskyTelemetry[2].set(ivalue, FR_RXRSI_COPY ) ;	// RSSI
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
					uint16_t x = (uint16_t)ivalue ;
//					x *= 128 ;
//					x /= 717 ;		// was 1155 ;
					store_hub_data( FR_RXV, x / 10 ) ;
				}
			break ;
		}	 
	}
	else if ( type == 0x80 )
	{
		dsmBindResponse( *(packet+6), *(packet+5) ) ;
		
	}	
//	else if ( type == 0xFF )
//	{
//		DsmManCode[0] = *packet++ ;
//		DsmManCode[1] = *packet++ ;
//		DsmManCode[2] = *packet++ ;
//		DsmManCode[3] = *packet ;
//	}
	else if (type == 0 )
	{
		if ( !g_model.dsmAasRssi )
		{
   		frskyTelemetry[2].set(type, FR_RXRSI_COPY ) ;	// RSSI
		}
		
	}

}
#endif // 128/2561

#ifndef N2F
	#define PRIVATE_COUNT	4
	#define PRIVATE_VALUE	5
	#define PRIVATE_TYPE		6
	#define PRIVATE_XCOUNT	7
#else
	#define frskyDataIgnore 4
	#define PRIVATE_COUNT	5
	#define PRIVATE_VALUE	6
#endif

/*
   Receive serial (RS-232) characters, detecting and storing each Fr-Sky 
   0x7e-framed packet as it arrives.  When a complete packet has been 
   received, process its data into storage variables.  NOTE: This is an 
   interrupt routine and should not get too lengthy. I originally had
   the buffer being checked in the perMain function (because per10ms
   isn't quite often enough for data streaming at 9600baud) but alas
   that scheme lost packets also. So each packet is parsed as it arrives,
   directly at the ISR function (through a call to frskyProcessPacket).
   
   If this proves a problem in the future, then I'll just have to implement
   a second buffer to receive data while one buffer is being processed (slowly).
*/

uint8_t Private_count ;
uint8_t Private_position ;
uint8_t Private_enable ;
uint8_t Private_type ;

//extern uint8_t TrotCount ;
//extern uint8_t TezRotary ;

// debug
//uint8_t Uerror ;
#ifdef AF_DEBUG
uint16_t Uecount ;
#endif
#if defined(CPUM128) || defined(CPUM2561)
uint16_t Uecount ;
#endif

//uint8_t DebugCount = 0 ;
//uint8_t DebugState = 0 ;

//uint8_t TezDebug0 ;
//uint8_t TezDebug1 ;

//#ifdef CPUM2561
//extern uint8_t Arduino ;
//#endif

#ifndef SIMU
ISR(USART0_RX_vect)
{
  uint8_t stat;
  uint8_t data;
  
  static uint8_t numPktBytes = 0;
  static uint8_t dataState = frskyDataIdle;
  
	UCSR0B &= ~(1 << RXCIE0); // disable Interrupt
	sei() ;
//#ifdef CPUM2561
//	if ( Arduino )
//	{
//		arduinoSerialRx() ;
//		return ;
//	}
//#endif
  stat = UCSR0A; // USART control and Status Register 0 A

    /*
              bit      7      6      5      4      3      2      1      0
                      RxC0  TxC0  UDRE0    FE0   DOR0   UPE0   U2X0  MPCM0
             
              RxC0:   Receive complete
              TXC0:   Transmit Complete
              UDRE0:  USART Data Register Empty
              FE0:    Frame Error
              DOR0:   Data OverRun
              UPE0:   USART Parity Error
              U2X0:   Double Tx Speed
              PCM0:   MultiProcessor Comms Mode
    */
    // rh = UCSR0B; //USART control and Status Register 0 B

    /*
              bit      7      6      5      4      3      2      1      0
                   RXCIE0 TxCIE0 UDRIE0  RXEN0  TXEN0 UCSZ02  RXB80  TXB80
             
              RxCIE0:   Receive Complete int enable
              TXCIE0:   Transmit Complete int enable
              UDRIE0:   USART Data Register Empty int enable
              RXEN0:    Rx Enable
              TXEN0:    Tx Enable
              UCSZ02:   Character Size bit 2
              RXB80:    Rx data bit 8
              TXB80:    Tx data bit 8
    */

  data = UDR0; // USART data register 0

	uint8_t numbytes = numPktBytes ;

#if defined(CPUM128) || defined(CPUM2561)
	TelRxCount += 1 ;
#endif
#ifdef AF_DEBUG
	TelRxCount += 1 ;
#endif
  
	if (stat & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))
  { // discard buffer and start fresh on any comms error
//    FrskyRxBufferReady = 0;
    numbytes = 0;
#if defined(CPUM128) || defined(CPUM2561)
		
//	 if ( g_model.telemetryProtocol == 2 )		// DSM telemetry
//	 {
		dataState = frskyDataIdle ;
//	 }
#endif
//		Uerror = stat & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)) ;
#ifdef AF_DEBUG
		Uecount += 1 ;
#endif
#if defined(CPUM128) || defined(CPUM2561)
		Uecount += 1 ;
#endif
  } 
  else
  {
//    if (FrskyRxBufferReady == 0) // can't get more data if the buffer hasn't been cleared
//    {
//		TezDebug0 += 1 ;		
		
#if defined(DSM_BIND_RESPONSE)
		
//	 if ( g_model.telemetryProtocol == 2 )		// DSM telemetry
//	 {
//    	switch (dataState) 
//			{
//    	  case frskyDataIdle:
//    	    if (data == 0xAA)
//					{
//    	     	dataState = frskyDataInFrame ;
//    	      numbytes = 0 ;
//	  	      frskyRxBuffer[numbytes++] = data ;
//					}
//				break ;

//    	  case frskyDataInFrame:
//    	    if (numbytes < 19)
//					{
//	  	      frskyRxBuffer[numbytes++] = data ;
//						if ( numbytes >= 18 )
//						{
//							processDsmPacket( frskyRxBuffer, numbytes ) ;
//							numbytes = 0 ;
//						}
//					}
//    	  break ;
//			}
	 	
//	 }
//	 else 
	 if ( g_model.telemetryProtocol == 7 )
	 {
  		frskyStreaming = FRSKY_TIMEOUT10ms; // reset counter only if valid frsky packets are being detected
	    frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms ; // reset counter only if valid frsky packets are being detected
			frsky_proc_user_byte( data ) ;		
	 }
	 else
	 {
      
#endif			
			
			switch (dataState) 
      {
        case frskyDataStart:
//-------------------------------------------------------------------------------------
#ifdef N2F			// This code supports the native FrySky Sensor Hub protocol
          if (data == HUB_START_STOP) break; 	// Remain in userDataStart if possible 0x5e,0x5e doublet found.

          frskyRxBuffer[numbytes++] = data;	//ID of field. 
          dataState = frskyDataInFrame;
          break;

        case frskyDataInFrame:
	    if (numbytes > 4)			// failure, restart
	    {
		  numbytes = 0;
                  dataState = frskyDataIdle;
		  break;
	    }
		//---------------
          if (data == BYTESTUFF)
          { 
              dataState = frskyDataXOR; // XOR next byte
              break; 
          }
		//---------------
	    //following BYTECANCEL = 0x18 the next byte is sent twice. Ignore the next byte
	    if (data == BYTECANCEL)			
	    {
		  dataState = frskyDataIgnore;		// ignore next byte
		  break;
	    }
		//----------------
          if (data == HUB_START_STOP) // end of field 0x5e detected, is used as start of next field
          {
		  processFrskyPacket(frskyRxBuffer); // FrskyRxBufferReady = 1; packet is "5E ID Byte Byte "
              numbytes = 0;
              dataState = frskyDataStart;	
              frskyRxBuffer[numbytes++] = data;	// put starting 5E into buffer
              break;
          }
		//----------------
          frskyRxBuffer[numbytes++] = data;
          break;

        case frskyDataXOR:
          if (numbytes < 19)
            frskyRxBuffer[numbytes++] = data ^ STUFF_MASK;
          dataState = frskyDataInFrame;
          break;

	  case frskyDataIgnore:
	    dataState = frskyDataInFrame;	
	    break;				// ignore this byyte

        case frskyDataIdle:
          if (data == HUB_START_STOP)  // 0x5E
          {
            numbytes = 0;
            frskyRxBuffer[numbytes++] = data;	// put 5E into buffer 
            dataState = frskyDataStart;
          }
          break;

//---------------------------------------------------------------------------------
#else					// This is the original FrySky decoding
          if (data == START_STOP)
					{
//						if ( FrskyTelemetryType )		// SPORT
//						{
//          		dataState = frskyDataInFrame ;
//            	numbytes = 0;
//						}
					 	break ; // Remain in userDataStart if possible 0x7e,0x7e doublet found.
					}
					if (data != PRIVATE)
					{
          	dataState = frskyDataInFrame;
          	if (numbytes < 19)
          	  frskyRxBuffer[numbytes++] = data;
					}
					else // Out of sync so restart
					{
						dataState = frskyDataIdle;
//						DebugCount += 1 ;
					}
          break;

        case frskyDataInFrame:
#if defined(DSM_BIND_RESPONSE)
	 				if ( g_model.telemetryProtocol == 2 )		// DSM telemetry
					{
    	    	if (numbytes < 19)
						{
	  	    	  frskyRxBuffer[numbytes++] = data ;
							if ( numbytes >= 18 )
							{
								processDsmPacket( frskyRxBuffer, numbytes ) ;
								numbytes = 0 ;
							}
						}
            break ;
					}
#endif
          if (data == BYTESTUFF)
          { 
              dataState = frskyDataXOR; // XOR next byte
              break; 
          }
          if (data == START_STOP) // end of frame detected
          {
						if ( FrskyTelemetryType )		// SPORT
						{						
          		dataState = frskyDataInFrame ;
            	numbytes = 0;
						}
						else
						{
	           	processFrskyPacket(frskyRxBuffer); // FrskyRxBufferReady = 1;
          		dataState = frskyDataIdle ;
						}
            break;
          }
          if (numbytes < 19)
	          frskyRxBuffer[numbytes++] = data;
          break;

        case frskyDataXOR:
          dataState = frskyDataInFrame;
          if (numbytes < 19)
            frskyRxBuffer[numbytes++] = data ^ STUFF_MASK;
          break;

        case frskyDataIdle:
	 				if ( g_model.telemetryProtocol == 2 )		// DSM telemetry
					{
    	    	if (data == 0xAA)
						{
	    	     	dataState = frskyDataInFrame ;
  	  	      numbytes = 0 ;
	  		      frskyRxBuffer[numbytes++] = data ;
							break ;
						}
					}
          if (data == START_STOP)
          {
            numbytes = 0;
            dataState = FrskyTelemetryType ? frskyDataInFrame : frskyDataStart ;
          }
          else if ( (data == PRIVATE) || (data == MPRIVATE) )
					{
//						TezDebug1 += 1 ;
						dataState = PRIVATE_COUNT ;
					}
        break;
        case PRIVATE_COUNT :
					Private_position = 0 ;
					if ( data == 'P' )
					{
						// MULTI_TELEMETRY
      		  dataState = PRIVATE_TYPE ;
						break ;
					}
					dataState = PRIVATE_VALUE ;
					Private_type = 0xFF ;	// Not MULTI_TELEMETRY
          Private_count = data ;		// Count of bytes to receive

					if ( data > 3 )
					{
         		dataState = frskyDataIdle ;
    				FrskyAlarmSendState |= 0x80 ;		// Send ACK
					}
        break;
				case PRIVATE_TYPE :
    		  Private_type = data ;
    		  dataState = PRIVATE_XCOUNT ;
					if ( data > 3 )
					{
						Private_position = 1 ;
					}
    		break ;
    		case PRIVATE_XCOUNT :
					dataState = PRIVATE_VALUE ;
      		Private_count = data ;		// Count of bytes to receive
					if ( data > PRIVATE_BUFFER_SIZE )
					{
      			dataState = frskyDataIdle ;
					}
    		break ;
        case PRIVATE_VALUE :
				{
					if ( Private_type == 0xFF )
					{
						uint8_t lEnable ;
						uint8_t lPosition ;
						lPosition = Private_position ;
						lEnable = Private_enable ;
						if ( lEnable < 50 )
						{
							lEnable += 12 ;
						}
						else
						{
							lEnable = 100 ;
							if ( lPosition == 0 )
							{
								// Process first private data byte
								// PC6, PC7
								if ( ( data & 0x3F ) == 0 )		// Check byte is valid
								{
									DDRC |= 0xC0 ;		// Set as outputs
									PORTC = ( PORTC & 0x3F ) | ( data & 0xC0 ) ;		// update outputs						
								}
							}
							else if( lPosition==1) {
								Rotary.TrotCount = data;
							}
							else if(lPosition==2) { // rotary encoder switch
								Rotary.TezRotary = data;
							}
						}
						Private_enable = lEnable ;
						lPosition += 1 ;
						Private_position = lPosition ;
						if ( lPosition == Private_count )
						{
#ifdef AF_DEBUG
	if ( AfCapture )
	{
		AfDebug[0] = Private_type ;
		AfDebug[1] = Private_count ;
		AfCapture = 0 ;
	}
#endif
          		dataState = frskyDataIdle;
    					FrskyAlarmSendState |= 0x80 ;		// Send ACK
						}
					}
					else
					{
						frskyRxBuffer[Private_position++] = data ;
						if ( ( Private_position == Private_count+1 ) || ( Private_position >= PRIVATE_BUFFER_SIZE ) )
						{
			        dataState = frskyDataIdle;
							// process private data here
#ifdef AF_DEBUG
	uint8_t i ;
	if ( AfCapture )
	{
		AfDebug[0] = Private_type ;
		AfDebug[1] = Private_count ;
		for ( i = 0 ; i < 18 ; i += 1 )
		{
			AfDebug[i+2] = frskyRxBuffer[i] ;
		}
		AfCapture = 0 ;
		AfBytes = Private_count+1 ;
	}
#endif
							if ( Private_position == Private_count+1 )
							{
								// Correct amount received
								uint8_t len = Private_count ;
						
								switch ( Private_type )
								{
									case 1 :	// Status
										if ( MultiDataRequest )
										{
											if ( len > 4 )
											{
												Xmem.MultiSetting.flags = frskyRxBuffer[0] ;
												memmove(Xmem.MultiSetting.revision, &frskyRxBuffer[1], 4 ) ;
												Xmem.MultiSetting.valid = 1 ;
											}
											if ( len > 15 )
											{
												memmove(Xmem.MultiSetting.protocol, &frskyRxBuffer[8], 7 ) ;
												Xmem.MultiSetting.protocol[7] = 0 ;
												Xmem.MultiSetting.valid = 3 ;
											}
											if ( len > 23 )
											{
												memmove(Xmem.MultiSetting.subProtocol, &frskyRxBuffer[16], 8 ) ;
												Xmem.MultiSetting.subProtocol[8] = 0 ;
												Xmem.MultiSetting.valid = 7 ;
												Xmem.MultiSetting.subData = frskyRxBuffer[15] ;
											}
										}	
		//								if ( len > 6 )
		//								{
		//									len = 6 ;
		//								}
		//								while ( len )
		//								{
		//									len -= 1 ;
		//									PrivateData[len] = InputPrivateData[len] ;
		//								}
									break ;
						
									case 2 :	// SPort
										if ( len >= FRSKY_SPORT_PACKET_SIZE )
										{
											processSportData() ;
										}
									break ;
									case 3 :	// Hub
										numPktBytes = Private_count ;
	          				processFrskyPacket(frskyRxBuffer) ;
									break ;
#if defined(DSM_BIND_RESPONSE)
									case 4 :	// DSM 
										processDsmPacket( frskyRxBuffer, Private_count+1 ) ;
									break ;
									case 5 :	// DSM bind
										dsmBindResponse( frskyRxBuffer[7], frskyRxBuffer[6] ) ;
									break ;
#endif
									case 6 :	// AFHDS2
										frskyRxBuffer[0] = 0xAA ;
										processAFHDS2Packet( frskyRxBuffer, Private_count+1 ) ;
									break ;
		//							case 10 :	// Hitec
		//								processHitecPacket( InputPrivateData ) ;
		//							break ;
									case 12 :	// AFHDS2
										frskyRxBuffer[0] = 0xAC ;
										processAFHDS2Packet( frskyRxBuffer, Private_count+1 ) ;
									break ;
		//							case 13 :	// Trainer data
		//								processTrainerPacket( InputPrivateData ) ;
		//							break ;
								}
							}
						}						
					}
				}
        break;
//---------------------------------------------------------------------------------
#endif

      } // switch
//    } // if (FrskyRxBufferReady == 0)
#if defined(CPUM128) || defined(CPUM2561)
   }
#endif
	}

	if ( FrskyTelemetryType )		// SPORT
	{
  	if (numbytes >= FRSKY_SPORT_PACKET_SIZE)
		{
			processSportPacket() ;    	
		  numbytes = 0 ;
			dataState = frskyDataIdle;
		}
	}
  numPktBytes = numbytes ;
	
//DebugState = dataState ;

	cli() ;
  UCSR0B |= (1 << RXCIE0); // enable Interrupt
}
#endif

/*
   USART0 (transmit) Data Register Emtpy ISR
   Usef to transmit FrSky data packets, which are buffered in frskyTXBuffer. 
*/

#ifndef SIMU
ISR(USART0_UDRE_vect)
{
	struct t_frskyTx *pftx = &FrskyTx ;	
	FORCE_INDIRECT(pftx) ;
//#ifdef CPUM2561
//	if ( Arduino )
//	{
//		arduinoSerialTx() ;
//		return ;
//	}
//#endif
  
	if ( pftx->frskyTxBufferCount > 0) 
  {
    pftx->frskyTxBufferCount--;
    UDR0 = pftx->frskyTxBuffer[pftx->frskyTxISRIndex++];
  } else
    UCSR0B &= ~(1 << UDRIE0); // disable UDRE0 interrupt
}

/******************************************/

static void frskyTransmitBuffer()
{
  FrskyTx.frskyTxISRIndex = 0;
  UCSR0B |= (1 << UDRIE0); // enable  UDRE0 interrupt
}
#endif


uint8_t FrskyAlarmSendState = 0 ;
uint8_t FrskyDelay = 0 ;
//uint8_t FrskyRSSIsend = 0 ;

#ifndef SIMU
static void FRSKY10mspoll(void)
{
#if defined(CPUM128) || defined(CPUM2561)
	if ( g_eeGeneral.FrskyPins == 0 )
		return ;
#endif
  if (FrskyDelay)
  {
    FrskyDelay -= 1 ;
    return ;
  }

  if (FrskyTx.frskyTxBufferCount)
  {
    return; // we only have one buffer. If it's in use, then we can't send yet.
  }

  // Now send a packet
  {
#ifndef V2
		uint8_t i ;
		uint8_t j = 1 ;

		for ( i = 0 ; i < 8 ; i += 1, j <<= 1 )
		{
			if ( FrskyAlarmSendState & j )
			{
				break ;				
			}			
		}
    FrskyAlarmSendState &= ~j ;
		if ( i < 4 )
		{
		//	FRSKY_setTxPacket( A22PKT + i, g_eeGeneral.frskyinternalalarm ? 0 :g_model.frsky.channels[channel].alarms_value[alarm],
		//														 ALARM_GREATER(channel, alarm), ALARM_LEVEL(channel, alarm) ) ;					

			FRSKY_setTxPacket( A22PKT + i, 0, 0, 0 ) ;
									 
		}
		else if( i < 6 )
		{
			i &= 1 ;
			FRSKY_setTxPacket( RSSITXPKT+i, frskyRSSIlevel[i], 0, frskyRSSItype[i] ) ;
		}
		else if (i == 6)
		{
// Send packet requesting all RSSIalarm settings be sent back to us
			FRSKY_setTxPacket( RSSI_REQUEST, 0, 0, 0 ) ;
		}
		else if (i == 7)
#else
		if ( FrskyAlarmSendState & 0x80 )
#endif
		{
			struct t_frskyTx *pfrskytx = &FrskyTx ;
			FORCE_INDIRECT( pfrskytx ) ;
			
  		pfrskytx->frskyTxBuffer[0] = PRIVATE ;		// Start of packet
//			if ( g_eeGeneral.TEZr90 )
//			{
  			pfrskytx->frskyTxBuffer[3] = PRIVATE ;  // End of packet
		  	pfrskytx->frskyTxBuffer[1] = 1 ;
  			pfrskytx->frskyTxBuffer[2] = FrskyTelemetryType ;
				pfrskytx->frskyTxBufferCount = 4 ;
//			}
//			else
//			{
//  			FrskyTx.frskyTxBuffer[2] = PRIVATE ;  // End of packet
//		  	FrskyTx.frskyTxBuffer[1] = 0 ;
//				FrskyTx.frskyTxBufferCount = 3 ;
//			}
		}
		else
		{
			return ;
		}
#ifndef V2
    FrskyDelay = (i == 7) ? 10 : 5 ; // 50mS
#else
    FrskyDelay = 10 ; // 50mS
#endif
    frskyTransmitBuffer(); 
  }
}
#endif

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


#ifndef V2
void FRSKY_setTxPacket( uint8_t type, uint8_t value, uint8_t p1, uint8_t p2 )
{
	uint8_t i = 0;
  FrskyTx.frskyTxBuffer[i++] = START_STOP;        // Start of packet
  FrskyTx.frskyTxBuffer[i++] = type ;
  i = frskyPushValue( i, value) ;
  {
    uint8_t *ptr ;
    ptr = &FrskyTx.frskyTxBuffer[i] ;
		FORCE_INDIRECT(ptr) ;
    *ptr++ = p1 ;
    *ptr++ = p2 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr = START_STOP ;        // End of packet
	}
	FrskyTx.frskyTxBufferCount = i + 8 ;
}
#endif // V2

//enum AlarmLevel FRSKY_alarmRaised(uint8_t idx, uint8_t alarm)
//enum AlarmLevel FRSKY_alarmRaised(uint8_t idx)
//{
//	FrSkyChannelData *ptr_fdata ;

//	ptr_fdata = &g_model.frsky.channels[idx] ;

//  uint8_t value ;
//  uint8_t alarm_value ;
//	uint8_t level = ptr_fdata->alarms_level ;
//	uint8_t greater = ptr_fdata->alarms_greater ;
//  value = frskyTelemetry[idx].value ;
	
////	if ( alarm != 1 )		// 0 or 2
////	{
//		level &= 3 ;
//		if ( level != (uint8_t)alarm_off)
//		{
//      alarm_value = ptr_fdata->alarms_value[0] ;
//      if ( greater & 1)
//			{
//    	  if (value > alarm_value)
//  				return (enum AlarmLevel) level ;
//      }
//      else
//			{
//        if (value < alarm_value)
//  				return (enum AlarmLevel) level ;
//      }
//		}
////  }
////	if ( alarm )		// 1 or 2
////	{
//		level = ptr_fdata->alarms_level >> 2 ;
//		level &= 3 ;
//		if ( level != (uint8_t)alarm_off)
//		{
//      alarm_value = ptr_fdata->alarms_value[1] ;
//      if ( greater & 2)
//			{
//    	  if (value > alarm_value)
//  				return (enum AlarmLevel) level ;
//      }
//      else
//			{
//        if (value < alarm_value)
//  				return (enum AlarmLevel) level ;
//      }
//		}
////  }
//  return alarm_off ;
//}

//void FRSKY_alarmPlay(uint8_t idx, uint8_t alarm){			
//			uint8_t alarmLevel = ALARM_LEVEL(idx, alarm);
			
//			if(((g_eeGeneral.speakerMode & 1) == 1 /*|| g_eeGeneral.speakerMode == 2*/) && g_eeGeneral.frskyinternalalarm == 1){   // this check is done here so haptic still works even if frsky beeper used.
//					switch (alarmLevel){			
//								case alarm_off:
//														break;
//								case alarm_yellow:
//														audio.event(g_eeGeneral.FRSkyYellow);
//														break;														
//								case alarm_orange:
//														audio.event(g_eeGeneral.FRSkyOrange);
//														break;												
//								case alarm_red:
//														audio.event(g_eeGeneral.FRSkyRed);
//														break;		
//					}	
//			}
			
//}



#if defined(CPUM128) || defined(CPUM2561)
void FRSKY_DisableTXD(void)
{
  UCSR0B &= ~((1 << TXEN0) | (1 << UDRIE0)); // disable TX pin and interrupt
}

void FRSKY_DisableRXD(void)
{
  UCSR0B &= ~(1 << RXEN0);  // disable RX
  UCSR0B &= ~(1 << RXCIE0); // disable Interrupt
  DDRE &= ~(1 << DDE0);   	// set RXD0 pin as input
  PORTE |= (1 << PORTE0);		// enable pullup on RXD0 pin
}

void FRSKY_disable()
{
	FRSKY_DisableTXD() ;
	FRSKY_DisableRXD() ;
	
}

#endif

#define UBRRH_57600		0
//#define UBRRL_57600		34
#define UBRRL_57600		16


void FRSKY_Init( uint8_t brate)
{
	FrskyTelemetryType = brate ;
  // clear frsky variables
#if defined(CPUM128) || defined(CPUM2561)
	if ( g_eeGeneral.FrskyPins == 0 )
		return ;
#endif
  resetTelemetry();

  DDRE &= ~(1 << DDE0);    // set RXD0 pin as input
  PORTE |= (1 << PORTE0);		// enable pullup on RXD0 pin
//  PORTE &= ~(1 << PORTE0); // disable pullup on RXD0 pin

#undef BAUD
#define BAUD 9600

#ifndef SIMU

#include <util/setbaud.h>

	if ( g_eeGeneral.TEZr90 )
	{
		brate = 1 ;
	}

  if ( brate == 0 ) // 9600
	{
		UBRR0L = UBRRL_VALUE;
		UBRR0H = UBRRH_VALUE;
	}
	else // 57600
	{
		UBRR0L = UBRRL_57600 ;
		UBRR0H = UBRRH_57600 ;
	}

	UCSR0A &= ~(1 << U2X0); // disable double speed operation.
  // set 8 N1
  UCSR0B = 0 | (0 << RXCIE0) | (0 << TXCIE0) | (0 << UDRIE0) | (0 << RXEN0) | (0 << TXEN0) | (0 << UCSZ02);
#ifdef MULTI_PROTOCOL
	if ( g_model.protocol == PROTO_MULTI ) // set 100000bps 8e2
	{
		if ( !g_eeGeneral.TEZr90 )
		{
			UBRR0L = 9;
			UBRR0H = 0;
			UCSR0C = 0 | (1<<UPM01)|(1<<USBS0)|(1<<UCSZ01)|(1<<UCSZ00);
		}
	}
	else
#endif // MULTI_PROTOCOL
		UCSR0C = 0 | (1 << UCSZ01) | (1 << UCSZ00);

  
  while (UCSR0A & (1 << RXC0)) UDR0; // flush receive buffer

#endif

  // These should be running right from power up on a FrSky enabled '9X.

  FrskyTx.frskyTxBufferCount = 0;
  UCSR0B = (1 << TXEN0) | (1 << UDRIE0) | (1 << RXEN0) | (1 << RXCIE0) ;
	// enable TX, enable TX interrupt, enable RX, enable Interrupt
}

//#if 0
//// Send packet requesting all alarm settings be sent back to us
//void frskyAlarmsRefresh()
//{

//  if (FrskyTx.frskyTxBufferCount) return; // we only have one buffer. If it's in use, then we can't send. Sorry.
//	FRSKY_setTxPacket( ALRM_REQUEST, 0, 0, 0 )
//  frskyTransmitBuffer();
//}
//#endif

#ifndef V2
static uint8_t frskyPushValue( uint8_t i, uint8_t value)
{
	uint8_t j ;
  uint8_t *ptr ;
  ptr = &FrskyTx.frskyTxBuffer[i] ;
	FORCE_INDIRECT(ptr) ;
	j = 0 ;
  // byte stuff the only byte than might need it
  if (value == START_STOP) {
    j = 1 ;
  }
  else if (value == BYTESTUFF) {
    j = 1 ;
  }
	if ( j )
	{
    value -= 0x20;
		*ptr++ = BYTESTUFF;
		i += 1 ;
	}
  *ptr = value;
	i += 1 ;
	return i ;
}
#endif // V2

void FrskyData::setoffset()
{
	offset = raw ;
	value = 0 ;
}

void FrskyData::set(uint8_t value, uint8_t copy)
{
	uint8_t x ;
#ifdef NMEA
	raw = value;
#else
	averaging_total += value ;
	uint8_t count = 16 ;
	uint8_t shift = 4 ;
	if ( FrskyTelemetryType )
	{
		 count = 4 ;
		 shift = 2 ;
	}
	if ( ++averageCount >= count )
	{
		averageCount = 0 ;
		raw = averaging_total >> shift ;
#endif
  	if ( raw > offset )
		{
		  x = raw - offset ;
		}
		else
		{
			x = 0 ;
		}
		this->value = x ;
		store_hub_data( copy, this->value ) ;
//   	if (max < value)
//   	  max = value;
//   	if (!min || min > value)
//   	  min = value;
#ifndef NMEA
		averaging_total = 0 ;
	}
#endif
}

void resetTelemetry()
{
//	FrskyData *ptr_data = &frskyTelemetry[0] ;
//	FORCE_INDIRECT(ptr_data) ;

//	ptr_data[0].min = 0 ;
//	ptr_data[0].max = 0 ;
//	ptr_data[1].min = 0 ;
//	ptr_data[1].max = 0 ;
//	ptr_data[2].min = 0 ;
//	ptr_data[2].max = 0 ;
//	ptr_data[3].min = 0 ;
//	ptr_data[3].max = 0 ;
	
	FrskyHubData[FR_A1_MAH] = 0 ;
	FrskyHubData[FR_A2_MAH] = 0 ;
	FrskyHubData[FR_CELL_MIN] = 0 ;			// 0 volts
	Frsky_Amp_hour_prescale = 0 ;
	FrskyHubData[FR_AMP_MAH] = 0 ;
	FrskyBattCells = 0 ;
  memset( &FrskyHubMaxMin, 0, sizeof(FrskyHubMaxMin));
	FrskyHubMaxMin.hubMax[FR_ALT_BARO] = -5000 ;
}

//void current_check( uint8_t i )
//{
//	Frsky_current_info *ptr_current ;
//	uint8_t cvalue ;

//	ptr_current = &Frsky_current[0] ;
//	FORCE_INDIRECT(ptr_current) ;
//	cvalue = frskyTelemetry[0].value ;
//	if ( i )
//	{
//		ptr_current += 1 ;
//		cvalue = frskyTelemetry[1].value ;
//	}
//  // value * ratio / 100 gives 10ths of amps
//  // add this every 10 ms, when over 3600, we have 1 mAh
//  // so subtract 3600 and add 1 to mAh total
//  // alternatively, add up the raw value, and use 3600 * 100 / ratio for 1mAh
			
//	if ( (  ptr_current->Amp_hour_prescale += cvalue ) > ptr_current->Amp_hour_boundary )
//	{
//		 ptr_current->Amp_hour_prescale -= ptr_current->Amp_hour_boundary ;
//		int16_t *ptr_hub = &FrskyHubData[FR_A1_MAH+i] ;
//		FORCE_INDIRECT(ptr_hub) ;

//		*ptr_hub += 1 ;
//	}
//}

// Called every 10 mS in interrupt routine
void check_frsky()
{
  // Used to detect presence of valid FrSky telemetry packets inside the
  // last FRSKY_TIMEOUT10ms 10ms intervals
#if defined(CPUM128) || defined(CPUM2561)
	if ( g_eeGeneral.FrskyPins == 0 )
		return ;
#endif

	if ( Private_enable )
	{
		Private_enable -= 1 ;
	}

	uint8_t telemetryType = (g_model.protocol == PROTO_PXX )||((g_model.protocol == PROTO_MULTI) && ((g_model.sub_protocol&0x1F)==M_FRSKYX));
	if ( telemetryType != FrskyTelemetryType )
	{
		FRSKY_Init( telemetryType ) ;	
	}
	 
#ifndef SIMU
	uint8_t lTmOK ;
	if (frskyStreaming > 0)
	{
		if ( --frskyStreaming == 0 )
		{
 			FrskyHubData[FR_TXRSI_COPY] = 0 ;
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

#endif


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
#ifdef V2
				current *= g_model.frsky.channels[index].ratio ;
				current /= 256 ;
#else
				current *= g_model.frsky.channels[index].opt.scale.ratio ;
				current /= 100 ;
#endif
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
			FORCE_INDIRECT(ptr_hub) ;
			*ptr_hub += 1 ;
			ptr_hub += FR_A1_MAH-FR_AMP_MAH ;
			*ptr_hub += 1 ;
			ptr_hub += FR_A2_MAH-FR_A1_MAH ;
			*ptr_hub += 1 ;
//			FrskyHubData[FR_A1_MAH] += 1 ;
//			FrskyHubData[FR_A2_MAH] += 1 ;
		}
		Frsky_Amp_hour_prescale = ah_temp ;
	}



//  if (frskyStreaming)
//	{
//  	if ( g_model.frsky.channels[0].opt.alarm.type == 3 )		// Current (A)
//			current_check( 0 ) ;
//  	if ( g_model.frsky.channels[1].opt.alarm.type == 3 )		// Current (A)
//			current_check( 1 ) ;
//	}

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
//			FORCE_INDIRECT(ptr_hub) ;
//			*ptr_hub += 1 ;
//		}
//		Frsky_Amp_hour_prescale = ah_temp ;
//	}
}

// New model loaded
void FRSKY_setModelAlarms(void)
{
#if defined(CPUM128) || defined(CPUM2561)
	if ( g_eeGeneral.FrskyPins == 0 )
		return ;
#endif
	FrskyBattCells = 0 ;
#ifndef V2
  FrskyAlarmSendState |= 0x0F ;
#endif // nV2
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

//// If index bit 7 is zero - process now
//// else wait until item further down q has bit 7 zero

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

//		store_hub_data( r->index & 0x7F, r->value ) ;
//		++z &= 0x07 ;
//	}
	
//	FrSky_Queue.out_index = z ;	
//	cli() ;
//	FrSky_Queue.count -= y ;
//	sei() ;

//}


//uint32_t GpsPosLat ;

//void testgps()
//{
//  GpsPosLat = (((uint32_t)(uint16_t)FrskyHubData[FR_GPS_LAT] / 100) * 1000000) + (((uint32_t)((uint16_t)FrskyHubData[FR_GPS_LAT] % 100) * 10000 + (uint16_t)FrskyHubData[FR_GPS_LATd]) * 5) / 3;
//}



