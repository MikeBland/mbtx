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
	HUBDATALENGTH-1,HUBDATALENGTH-1,
	FR_GPS_ALTd,
	HUBDATALENGTH-1,HUBDATALENGTH-1,		// 10,11
	HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,
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
	HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,HUBDATALENGTH-1,
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

uint8_t TmOK ;

uint8_t AltitudeDecimals ;
int16_t WholeAltitude ;

#define FRSKY_SPORT_PACKET_SIZE		9

uint8_t frskyRxBuffer[19];   // Receive buffer. 9 bytes (full packet), worst case 18 bytes with byte-stuffing (+1)

struct t_frskyTx
{
	uint8_t frskyTxBuffer[13];   // Ditto for transmit buffer
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
		value *= 10 ;
		if ( AltitudeDecimals )
		{
			WholeAltitude = value ;
			index = FR_TRASH ;
		}
	}
	if ( index == FR_ALT_BAROd )
	{
		AltitudeDecimals |= 1 ;
		if ( value > 9 )
		{
			AltitudeDecimals |= 2 ;
		}
		if ( AltitudeDecimals & 2 )
		{
			value /= 10 ;			
		}
		FrskyHubData[FR_ALT_BARO] = WholeAltitude + ( (WholeAltitude > 0) ? value : -value ) ;
	}

	if ( index == FR_SPORT_ALT )
	{
		index = FR_ALT_BARO ;         // For max and min
		FrskyHubData[FR_ALT_BARO] = value ;
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


void processSportPacket()
{
	uint8_t *packet = frskyRxBuffer ;
	FORCE_INDIRECT(packet) ;
  uint8_t  prim   = packet[1];
//  uint16_t appId  = *((uint16_t *)(packet+2)) ;
	
  if ( !checkSportPacket() )
	{
    return;
	}
  
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
				break ;

				case 4 :		// Battery from X8R
//		      frskyTelemetry[0].set(value, FR_A1_COPY ); //FrskyHubData[] =  frskyTelemetry[0].value ;
					store_hub_data( FR_RXV, value ) ;
//				break ;
				case 2 :
		      frskyTelemetry[0].set(value, FR_A1_COPY ); //FrskyHubData[] =  frskyTelemetry[0].value ;
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

#if defined(CPUM128) || defined(CPUM2561)
				case T1_ID_8 :
					store_hub_data( FR_TEMP1, value ) ;
				break ;
				
				case T2_ID_8 :
					store_hub_data( FR_TEMP2, value ) ;
				break ;

				case RPM_ID_8 :
					store_hub_data( FR_RPM, value ) ;
				break ;
				
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


#ifndef N2F
	#define PRIVATE_COUNT	4
	#define PRIVATE_VALUE	5
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
//extern uint8_t TrotCount ;
//extern uint8_t TezRotary ;

// debug
//uint8_t Uerror ;
//uint8_t Uecount ;

//uint8_t DebugCount = 0 ;
//uint8_t DebugState = 0 ;

//uint8_t TezDebug0 ;
//uint8_t TezDebug1 ;

#ifdef CPUM2561
extern uint8_t Arduino ;
#endif

#ifndef SIMU
ISR(USART0_RX_vect)
{
  uint8_t stat;
  uint8_t data;
  
  static uint8_t numPktBytes = 0;
  static uint8_t dataState = frskyDataIdle;
  
	UCSR0B &= ~(1 << RXCIE0); // disable Interrupt
	sei() ;
#ifdef CPUM2561
	if ( Arduino )
	{
		arduinoSerialRx() ;
		return ;
	}
#endif
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
  
	if (stat & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))
  { // discard buffer and start fresh on any comms error
//    FrskyRxBufferReady = 0;
    numbytes = 0;
//		Uerror = stat & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)) ;
//		Uecount += 1 ;
  } 
  else
  {
//    if (FrskyRxBufferReady == 0) // can't get more data if the buffer hasn't been cleared
//    {
//		TezDebug0 += 1 ;		
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
          if (data == START_STOP)
          {
            numbytes = 0;
            dataState = FrskyTelemetryType ? frskyDataInFrame : frskyDataStart ;
          }
          else if (data == PRIVATE)
					{
//						TezDebug1 += 1 ;
						dataState = PRIVATE_COUNT ;
					}
        break;
        case PRIVATE_COUNT :
					dataState = PRIVATE_VALUE ;
          Private_count = data ;		// Count of bytes to receive
					Private_position = 0 ;
        break;
        case PRIVATE_VALUE :
					if ( Private_position == 0 )
					{
						// Process first private data byte
						// PC6, PC7
						if ( ( data & 0x3F ) == 0 )		// Check byte is valid
						{
							DDRC |= 0xC0 ;		// Set as outputs
							PORTC = ( PORTC & 0x3F ) | ( data & 0xC0 ) ;		// update outputs						
						}
					}
					else if(Private_position==1) {
						Rotary.TrotCount = data;
					}
					else if(Private_position==2) { // rotary encoder switch
						Rotary.TezRotary = data;
					}
					Private_position++;
					if ( Private_position == Private_count )
					{
          	dataState = frskyDataIdle;
    				FrskyAlarmSendState |= 0x80 ;		// Send ACK
					}
        break;
//---------------------------------------------------------------------------------
#endif

      } // switch
//    } // if (FrskyRxBufferReady == 0)
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
#ifdef CPUM2561
	if ( Arduino )
	{
		arduinoSerialTx() ;
		return ;
	}
#endif
  
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
  PORTE &= ~(1 << PORTE0); // disable pullup on RXD0 pin

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
#ifdef MULTI_PROTOCOL
		if ( g_model.protocol == PROTO_MULTI ) // 125000
		{
			UBRR0L = 7;
			UBRR0H = 0;
		}
		else
#endif // MULTI_PROTOCOL
		{
			UBRR0L = UBRRL_VALUE;
			UBRR0H = UBRRH_VALUE;
		}
	}
	else // 57600
	{
		UBRR0L = UBRRL_57600 ;
		UBRR0H = UBRRH_57600 ;
	}

	UCSR0A &= ~(1 << U2X0); // disable double speed operation.
  // set 8 N1
  UCSR0B = 0 | (0 << RXCIE0) | (0 << TXCIE0) | (0 << UDRIE0) | (0 << RXEN0) | (0 << TXEN0) | (0 << UCSZ02);
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
  memset( &FrskyHubMaxMin, 0, sizeof(FrskyHubMaxMin));
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

	uint8_t telemetryType = g_model.protocol == PROTO_PXX ;
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



