/****************************************************************************
*  Copyright (c) 2013 by Michael Blandford. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*  3. Neither the name of the author nor the names of its contributors may
*     be used to endorse or promote products derived from this software
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
*  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*  SUCH DAMAGE.
*
****************************************************************************
* Other Authors:
 * - Andre Bernet
 * - Bertrand Songis
 * - Bryan J. Rentoul (Gruvin)
 * - Cameron Weeks
 * - Erez Raviv
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini
 * - Thomas Husterer
*
****************************************************************************/


#include <stdint.h>
#include <stdlib.h>

//#include "AT91SAM3S4.h"
//#ifndef SIMU
//#include "core_cm3.h"
//#endif

#include "ersky9x.h"
#include "myeeprom.h"
#include "logicio.h"
#include "drivers.h"
#include "pulses.h"
//#include "debug.h"

extern uint8_t BindRangeFlag[] ;
//extern uint8_t PxxExtra[] ;
extern struct t_telemetryTx TelemetryTx ;
extern uint8_t DsmResponseFlag ;

//extern uint16_t DsmBindDebug ;


void dsmBindResponse( uint8_t mode, int8_t channels )
{
	// Process mode here
//	DsmBindDebug |= 1 ;
	uint8_t dsm_mode_response ;
	{
		dsm_mode_response = mode & ( ORTX_USE_DSMX | ORTX_USE_11mS | ORTX_USE_11bit | ORTX_AUTO_MODE ) ;
#if defined(PCBTX16S) || defined(PCBT16)
		if ( ( g_model.Module[1].protocol != PROTO_MULTI ) && ( g_model.Module[0].protocol != PROTO_MULTI ) )
#else
		if ( g_model.Module[1].protocol != PROTO_MULTI )
#endif
		{
//	DsmBindDebug |= 2 ;
#if defined(PCBX9D) || defined(PCB9XT)
			if ( ( g_model.Module[1].channels != channels ) || ( g_model.dsmMode != ( dsm_mode_response | 0x80 ) ) )
			{
				g_model.Module[1].channels = channels ;
//	DsmBindDebug |= 4 ;
#else
			if ( ( g_model.Module[1].channels != channels ) || ( g_model.dsmMode != ( dsm_mode_response | 0x80 ) ) )
			{
				g_model.Module[1].channels = channels ;
//	DsmBindDebug |= 8 ;
#endif
				g_model.dsmMode = dsm_mode_response | 0x80 ;
	  		STORE_MODELVARS ;
				DsmResponseFlag = 1 ;
			}
		}
		else
		{
extern uint8_t MultiResponseData ;
//	DsmBindDebug |= 16 ;
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
			DsmResponseFlag = 1 ;
		}
	}
//	DsmBindDebug |= channels << 12 ;
}

//uint16_t SetMultiArrayCount ;

uint32_t setMultiSerialArray( uint8_t *data, uint32_t module )
{
	
//	SetMultiArrayCount += 1 ;
	
	uint32_t i ;
	uint8_t packetType ;
	uint8_t protoByte ;
	uint8_t optionByte ;
	uint8_t subProtocol ;
	uint32_t outputbitsavailable = 0 ;
	uint32_t outputbits = 0 ;
	uint8_t *start = data ;
	struct t_module *pmodule = &g_model.Module[module] ;
	uint8_t startChan = pmodule->startChannel ;
	subProtocol = ( ( pmodule->sub_protocol & 0x3F ) | (pmodule->exsub_protocol << 6 ) ) + 1 ;
//#if defined(PCBT12) || defined(PCBT16) // || defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
//	if ( subProtocol == M_FRSKYX+1 )
//	{
//		subProtocol = 127 ;
//	}
//#endif
//#if defined(PCBT12) || defined(PCBT16)
//	if ( subProtocol == M_FrskyD+1 )
//	{
//		subProtocol = 127 ;
//	}
//#endif
	packetType = ( ( subProtocol & 0x3F) > 31 ) ? 0x54 : 0x55 ;

  if (pmodule->failsafeMode != FAILSAFE_NOT_SET && pmodule->failsafeMode != FAILSAFE_RX )
	{
    if ( FailsafeCounter[module] )
		{
	    if ( FailsafeCounter[module]-- == 1 )
			{
				packetType += 2 ;	// Failsafe packet
			}
		}
	  if ( FailsafeCounter[module] == 0 )
		{
//			if ( pmodule->failsafeRepeat == 0 )
//			{
				FailsafeCounter[module] = 1000 ;
//			}
		}
	}

	*data++ = packetType ;
	protoByte = subProtocol & 0x1F ;		// load sub_protocol and clear Bind & Range flags
	protoByte |= pmodule->sub_protocol & 0x40 ;		// load sub_protocol and clear Bind & Range flags

	if (BindRangeFlag[module] & PXX_BIND)	protoByte |=BindBit ;		//set bind bit if bind menu is pressed
	if (BindRangeFlag[module] & PXX_RANGE_CHECK) protoByte |=RangeCheckBit ;		//set bind bit if bind menu is pressed
	*data++ = protoByte ;
	
	protoByte = pmodule->channels ;
	*data++ = ( protoByte/*g_model.ppmNCH*/ & 0xF0) | ( pmodule->pxxRxNum & 0x0F ) ;
  
	optionByte = pmodule->option_protocol ;
	if ( ( subProtocol ) == M_AFHD2SA + 1 )
	{
    optionByte |= 0x80 ;
	}
	*data++ = optionByte ;
	for ( i = 0 ; i < 16 ; i += 1 )
	{
		int16_t x ;
		uint32_t y = startChan + i ;
		x = y >= ( NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ) ? 0 : g_chans512[y] ;
		if ( packetType & 2 )
		{
			if ( pmodule->failsafeMode == FAILSAFE_HOLD )
			{
				x = 2047 ;
			}
			else if ( pmodule->failsafeMode == FAILSAFE_NO_PULSES )
			{
				x = 0 ;
			}
			else
			{
				// Send failsafe value
				int32_t value ;
				value = ( startChan < 16 ) ? pmodule->failsafe[startChan] : 0 ;
				value = ( value *4193 ) >> 9 ;
				value += 1024 ;
				x = limit( (int16_t)1, (int16_t)value, (int16_t)2046 ) ;
				startChan += 1 ;
			}
		}
		else
		{
			x *= 4 ;
			x += x > 0 ? 4 : -4 ;
			x /= 5 ;
			x += 0x400 ;
		}
		if ( x < 0 )
		{
			x = 0 ;
		}
		if ( x > 2047 )
		{
			x = 2047 ;
		}
		outputbits |= (uint32_t)x << outputbitsavailable ;
		outputbitsavailable += 11 ;
		while ( outputbitsavailable >= 8 )
		{
			uint32_t j = outputbits ;
			*data++ = j ;
			outputbits >>= 8 ;
			outputbitsavailable -= 8 ;
		}
	}
//  Stream[26]   = sub_protocol bits 6 & 7|RxNum bits 4 & 5|Telemetry_Invert 3|Future_Use 2|Disable_Telemetry 1|Disable_CH_Mapping 0
//   sub_protocol is 0..255 (bits 0..5 + bits 6..7)
//   RxNum value is 0..63 (bits 0..3 + bits 4..5)
//   Telemetry_Invert		=> 0x08	0=normal, 1=invert
//   Future_Use			=> 0x04	0=      , 1=
//   Disable_Telemetry	=> 0x02	0=enable, 1=disable
//   Disable_CH_Mapping	=> 0x01	0=enable, 1=disable

	protoByte = 0 ;
	if ( pmodule->multiDisableTelemetry )
	{
		protoByte |= 2 ;
	}
	if ( pmodule->highChannels == 0 )
	{
		protoByte |= 1 ;
	}
	if ( pmodule->disableTelemetry )
	{
		protoByte |= 2 ;
	}

	protoByte |= pmodule->pxxRxNum & 0x30 ;
	protoByte |= subProtocol & 0xC0 ;

	if ( module )
	{
		protoByte |= 0x08 ;		// Invert for external modules		
	}

	*data++ = protoByte ;

	if (BindRangeFlag[module] & PXX_BIND)
	{
		if ( ( subProtocol == M_FRSKYX+1 ) || ( subProtocol == M_FRSKYX2+1 ) || ( subProtocol == M_FRSKYR9+1 ) )
		{		
			protoByte = 0 ;
			if ( pmodule->disableTelemetry )
			{
				protoByte |= 1 ;
			}
			if ( pmodule->highChannels )
			{
				protoByte |= 2 ;
			}
			*data++ = protoByte ;
		}
	}
	else if ( ( subProtocol == M_FRSKYX+1 ) || ( subProtocol == M_FRSKYX2+1 ) || ( subProtocol == M_FRSKYR9+1 ) )
	{
		if ( TelemetryTx.sportCount )
		{
			uint32_t i ;
			uint8_t *p = TelemetryTx.SportTx.ptr ;
			if ( *p )	// Discard if zero PRIM
			{
				*data++ = TelemetryTx.SportTx.index ;
				for ( i = 0 ; i < 7 ; i += 1 )
				{
					uint8_t x ;
					x = *p++ ;
					if ( x == 0x7D )
					{
						x = *p++ ^ 0x20 ;
					}
					*data++ = x ;
				}
			}
			TelemetryTx.sportCount = 0 ;
		}
	}
	return data - start ;
}


void setDsmHeader( uint8_t *dsmDat, uint32_t module )
{
  if (dsmDat[0]&BadData)  //first time through, setup header
  {
  	switch(g_model.Module[module].sub_protocol)
  	{
  		case LPXDSM2:
  		  dsmDat[0]= 0x80;
  		break;
  		case DSM2only:
  		  dsmDat[0]=0x90;
  		break;
  		default:
  		  dsmDat[0]=0x98;  //dsmx, bind mode
  		break;
  	}
  }

#if defined(PCBSKY) || defined(PCB9XT)
	if((dsmDat[0]&BindBit)&&(!keyState(SW_Trainer)))  dsmDat[0]&=~BindBit;		//clear bind bit if trainer not pulled
#else
	if((dsmDat[0]&BindBit)&&(!keyState(SW_SH2)))  dsmDat[0]&=~BindBit;		//clear bind bit if trainer not pulled
#endif
  if ((!(dsmDat[0]&BindBit))&& (BindRangeFlag[module] & PXX_RANGE_CHECK)) dsmDat[0]|=RangeCheckBit;   //range check function
  else dsmDat[0]&=~RangeCheckBit;
}


///* CRC16 implementation according to CCITT standards */
////static const unsigned short crc16tab[256]= {
////  0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
////  0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
////  0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
////  0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
////  0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
////  0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
////  0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
////  0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
////  0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
////  0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
////  0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
////  0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
////  0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
////  0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
////  0x7E97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
////  0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
////  0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
////  0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
////  0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
////  0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
////  0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
////  0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
////  0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77E,0xc71d,0xd73c,
////  0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
////  0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
////  0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
////  0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
////  0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
////  0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
////  0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
////  0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
////  0x6e17,0x7E36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
////};

////uint16_t crc16(uint8_t * buf, uint32_t len)
////{
////  uint16_t crc = 0;
////  for (uint32_t counter=0; counter<len; counter++) {
////    crc = (crc<<8) ^ crc16tab[ ((crc>>8) ^ *buf++ ) & 0x00FF];
////  }
////  return crc;
////}

////#define CROSSFIRE_START_BYTE        0x0F
////#define CROSSFIRE_CH_CENTER         0x3E0
////#define CROSSFIRE_CH_BITS           11

////// Range for pulses (channels output) is [-1024:+1024]
////void createCrossfireFrame(uint8_t * frame, int16_t * pulses)
////{
////  uint8_t * buf = frame;
////  *buf++ = CROSSFIRE_START_BYTE;

////  uint32_t bits = 0;
////  uint8_t bitsavailable = 0;
////  for (int i=0; i<CROSSFIRE_CHANNELS_COUNT; i++)
////	{
////    bits |= (CROSSFIRE_CH_CENTER + (((pulses[i]) * 4) / 5)) << bitsavailable ;
////    bitsavailable += CROSSFIRE_CH_BITS;
////    while (bitsavailable >= 8)
////		{
////      *buf++ = bits;
////      bits >>= 8;
////      bitsavailable -= 8;
////    }
////  }
////  *buf++ = 0;
////  *buf++ = crc16(frame, 24);
////}



////const uint16_t CRCTable[]=
////{
////    0x0000,0x1189,0x2312,0x329b,0x4624,0x57ad,0x6536,0x74bf,
////    0x8c48,0x9dc1,0xaf5a,0xbed3,0xca6c,0xdbe5,0xe97E,0xf8f7,
////    0x1081,0x0108,0x3393,0x221a,0x56a5,0x472c,0x75b7,0x643e,
////    0x9cc9,0x8d40,0xbfdb,0xae52,0xdaed,0xcb64,0xf9ff,0xe876,
////    0x2102,0x308b,0x0210,0x1399,0x6726,0x76af,0x4434,0x55bd,
////    0xad4a,0xbcc3,0x8e58,0x9fd1,0xeb6e,0xfae7,0xc87c,0xd9f5,
////    0x3183,0x200a,0x1291,0x0318,0x77a7,0x662e,0x54b5,0x453c,
////    0xbdcb,0xac42,0x9ed9,0x8f50,0xfbef,0xea66,0xd8fd,0xc974,
////    0x4204,0x538d,0x6116,0x709f,0x0420,0x15a9,0x2732,0x36bb,
////    0xce4c,0xdfc5,0xed5e,0xfcd7,0x8868,0x99e1,0xab7a,0xbaf3,
////    0x5285,0x430c,0x7197,0x601e,0x14a1,0x0528,0x37b3,0x263a,
////    0xdecd,0xcf44,0xfddf,0xec56,0x98e9,0x8960,0xbbfb,0xaa72,
////    0x6306,0x728f,0x4014,0x519d,0x2522,0x34ab,0x0630,0x17b9,
////    0xef4e,0xfec7,0xcc5c,0xddd5,0xa96a,0xb8e3,0x8a78,0x9bf1,
////    0x7387,0x620e,0x5095,0x411c,0x35a3,0x242a,0x16b1,0x0738,
////    0xffcf,0xee46,0xdcdd,0xcd54,0xb9eb,0xa862,0x9af9,0x8b70,
////    0x8408,0x9581,0xa71a,0xb693,0xc22c,0xd3a5,0xe13e,0xf0b7,
////    0x0840,0x19c9,0x2b52,0x3adb,0x4e64,0x5fed,0x6d76,0x7cff,
////    0x9489,0x8500,0xb79b,0xa612,0xd2ad,0xc324,0xf1bf,0xe036,
////    0x18c1,0x0948,0x3bd3,0x2a5a,0x5ee5,0x4f6c,0x7df7,0x6c7E,
////    0xa50a,0xb483,0x8618,0x9791,0xe32e,0xf2a7,0xc03c,0xd1b5,
////    0x2942,0x38cb,0x0a50,0x1bd9,0x6f66,0x7Eef,0x4c74,0x5dfd,
////    0xb58b,0xa402,0x9699,0x8710,0xf3af,0xe226,0xd0bd,0xc134,
////    0x39c3,0x284a,0x1ad1,0x0b58,0x7fe7,0x6e6e,0x5cf5,0x4d7c,
////    0xc60c,0xd785,0xe51e,0xf497,0x8028,0x91a1,0xa33a,0xb2b3,
////    0x4a44,0x5bcd,0x6956,0x78df,0x0c60,0x1de9,0x2f72,0x3efb,
////    0xd68d,0xc704,0xf59f,0xe416,0x90a9,0x8120,0xb3bb,0xa232,
////    0x5ac5,0x4b4c,0x79d7,0x685e,0x1ce1,0x0d68,0x3ff3,0x2e7a,
////    0xe70e,0xf687,0xc41c,0xd595,0xa12a,0xb0a3,0x8238,0x93b1,
////    0x6b46,0x7acf,0x4854,0x59dd,0x2d62,0x3ceb,0x0e70,0x1ff9,
////    0xf78f,0xe606,0xd49d,0xc514,0xb1ab,0xa022,0x92b9,0x8330,
////    0x7bc7,0x6a4e,0x58d5,0x495c,0x3de3,0x2c6a,0x1ef1,0x0f78
////};


////void crc( uint8_t data )
////{
////    //	uint8_t i ;

////  PcmCrc =(PcmCrc<<8) ^(CRCTable[((PcmCrc>>8)^data)&0xFF]);
////}

const uint16_t CRC_Short[]=
{
   0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
   0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7 };

uint16_t CRCTable(uint8_t val)
{
	return CRC_Short[val&0x0F] ^ (0x1081 * (val>>4));
}



uint16_t scaleForPXX( uint8_t i )
{
	int16_t value ;

	value = ( i < 32 ) ? g_chans512[i] *3 / 4 + 1024 : 0 ;
	return limit( (int16_t)1, value, (int16_t)2046 ) ;
}



//#ifdef XFIRE

//// CRC8 implementation with polynom = x^8+x^7+x^6+x^4+x^2+1 (0xD5)
//unsigned char crc8tab[256] = {
//  0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54,
//  0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
//  0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06,
//  0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
//  0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0,
//  0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
//  0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2,
//  0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
//  0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9,
//  0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
//  0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B,
//  0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
//  0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D,
//  0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
//  0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F,
//  0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
//  0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB,
//  0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
//  0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9,
//  0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
//  0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F,
//  0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
//  0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D,
//  0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
//  0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26,
//  0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
//  0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74,
//  0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
//  0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82,
//  0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
//  0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0,
//  0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
//};

//uint8_t crc8(const uint8_t * ptr, uint32_t len)
//{
//  uint8_t crc = 0;
//  for ( uint32_t i=0 ; i<len ; i += 1 )
//	{
//    crc = crc8tab[crc ^ *ptr++] ;
//  }
//  return crc;
//}

//#define CROSSFIRE_CH_CENTER         0x3E0
//#define CROSSFIRE_CH_BITS           11
//#define CROSSFIRE_CHANNELS_COUNT		16

//#define MODULE_ADDRESS							0xEE
//#define CHANNELS_ID									0x16

//// Range for pulses (channels output) is [-1024:+1024]
//uint8_t setupPulsesXfire()
//{
//	uint32_t startChan ;
//	startChan = g_model.Module[1].startChannel ;
//  uint8_t *buf = Bit_pulses ;
//  *buf++ = MODULE_ADDRESS ;
//  *buf++ = 24 ; // 1(ID) + 22 + 1(CRC)
//  uint8_t *crc_start = buf ;
//  *buf++ = CHANNELS_ID ;
//  uint32_t bits = 0 ;
//  uint32_t bitsavailable = 0 ;
//  for (uint32_t i=0 ; i < CROSSFIRE_CHANNELS_COUNT ; i += 1 )
//	{
//    uint32_t val = limit(0, CROSSFIRE_CH_CENTER + (((g_chans512[startChan+i]) * 4) / 5), 2*CROSSFIRE_CH_CENTER) ;
//    bits |= val << bitsavailable ;
//    bitsavailable += CROSSFIRE_CH_BITS ;
//    while (bitsavailable >= 8)
//		{
//      *buf++ = bits ;
//      bits >>= 8 ;
//      bitsavailable -= 8 ;
//    }
//  }
//  *buf++ = crc8( crc_start, 23) ;
//  return buf - Bit_pulses ;
//}


//#endif

#ifdef XFIRE

// CRC8 implementation with polynom = x^8+x^7+x^6+x^4+x^2+1 (0xD5)
const unsigned char crc8tab[256] = {
  0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54,
  0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
  0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06,
  0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
  0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0,
  0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
  0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2,
  0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
  0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9,
  0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
  0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B,
  0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
  0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D,
  0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
  0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F,
  0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
  0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB,
  0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
  0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9,
  0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
  0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F,
  0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
  0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D,
  0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
  0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26,
  0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
  0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74,
  0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
  0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82,
  0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
  0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0,
  0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
};

uint8_t crc8(const uint8_t * ptr, uint32_t len)
{
  uint8_t crc = 0;
  for ( uint32_t i=0 ; i<len ; i += 1 )
	{
    crc = crc8tab[crc ^ *ptr++] ;
  }
  return crc;
}

#define CROSSFIRE_CH_CENTER         0x3E0
#define CROSSFIRE_CH_BITS           11
#define CROSSFIRE_CHANNELS_COUNT		16

#define MODULE_ADDRESS							0xEE
#define CHANNELS_ID									0x16

extern uint8_t Bit_pulses[] ;
extern uint16_t XfireLength ;
extern struct t_telemetryTx TelemetryTx ;

// Range for pulses (channels output) is [-1024:+1024]
uint8_t setupPulsesXfire()
{
	uint32_t startChan ;
 	uint8_t *buf = Bit_pulses ;
 	*buf++ = MODULE_ADDRESS ;

	if ( TelemetryTx.XfireTx.count )
	{
		uint32_t i ;
 		*buf++ = TelemetryTx.XfireTx.count + 2 ;
  	uint8_t *crc_start = buf ;
  	*buf++ = TelemetryTx.XfireTx.command ;
  	for ( i = 0 ; i < TelemetryTx.XfireTx.count ; i += 1 )
		{
			*buf++ = TelemetryTx.XfireTx.data[i] ;
			if ( i > 62 )
			{
				break ;
			}
		}
		i = buf - crc_start ;
		*buf++ = crc8( crc_start, i ) ;
		TelemetryTx.XfireTx.count = 0 ;
	}
	else
	{
		startChan = g_model.Module[1].startChannel ;
  	*buf++ = 24 ; // 1(ID) + 22 + 1(CRC)
  	uint8_t *crc_start = buf ;
  	*buf++ = CHANNELS_ID ;
  	uint32_t bits = 0 ;
  	uint32_t bitsavailable = 0 ;
  	for (uint32_t i=0 ; i < CROSSFIRE_CHANNELS_COUNT ; i += 1 )
		{
  	  uint32_t val = limit(0, CROSSFIRE_CH_CENTER + (((g_chans512[startChan+i]) * 4) / 5), 2*CROSSFIRE_CH_CENTER) ;
  	  bits |= val << bitsavailable ;
  	  bitsavailable += CROSSFIRE_CH_BITS ;
  	  while (bitsavailable >= 8)
			{
  	    *buf++ = bits ;
  	    bits >>= 8 ;
  	    bitsavailable -= 8 ;
  	  }
  	}
  	*buf++ = crc8( crc_start, 23) ;
	}
  return (XfireLength = (buf - Bit_pulses)) ;
}


#endif

#ifdef XFIRE
//uint8_t outputTelemetryBuffer[TELEMETRY_OUTPUT_FIFO_SIZE] __DMA;
//uint8_t outputTelemetryBufferSize = 0;
//uint8_t outputTelemetryBufferTrigger = 0;

#endif

