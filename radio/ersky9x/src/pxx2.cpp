/*
 * Author - Mike Blandford
 *
 * Based on code named
 * OpenTX
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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

#include <string.h>

#include "ersky9x.h"
#include "X9D/hal.h"
#include "myeeprom.h"
#include "frsky.h"
#include "drivers.h"
//#include "string.h"

#ifdef ACCESS

extern uint8_t PxxSerial[2][50] ;
extern uint8_t *PtrSerialPxx[2] ;
extern struct t_telemetryTx TelemetryTx ;
//uint8_t *AccessPulsePtr ;
uint16_t AccessCrc[2] ;

uint8_t DebugLog ;

extern union t_sharedMemory SharedMemory ;

struct t_moduleSettings ModuleSettings[] ;
struct t_moduleControl ModuleControl[] ;

void pxx2AddByte( uint8_t byte, uint32_t module )
{
  AccessCrc[module] -= byte;
	*PtrSerialPxx[module]++ = byte ;
}

void pxx2AddWord( uint32_t word, uint32_t module )
{
  pxx2AddByte(word, module ) ;
  pxx2AddByte(word >> 8, module ) ;
  pxx2AddByte(word >> 16, module ) ;
  pxx2AddByte(word >> 24, module ) ;
}

uint32_t  pxx2AddFlag0( uint32_t module )
{
	uint8_t byte ;
	struct t_module *pmodule = &g_model.Module[module] ;
	
	byte = pmodule->pxxRxNum & 0x3F ;

  if (pmodule->failsafeMode != FAILSAFE_NOT_SET && pmodule->failsafeMode != FAILSAFE_RX )
	{
    if ( FailsafeCounter[module]-- == 0 )
		{
      byte |= PXX2_CHANNELS_FLAG0_FAILSAFE;
			FailsafeCounter[module] = PXX2_FAILSAFE_RATE ;
		}
	}

	if (BindRangeFlag[module] & PXX_RANGE_CHECK)
	{
    byte |= PXX2_CHANNELS_FLAG0_RANGECHECK ;
  }
	pxx2AddByte	( byte, module ) ;
	return byte ;
}


void addChannels( uint8_t module, uint8_t sendFailsafe, uint8_t firstChannel )
{
  uint16_t pulseValue = 0 ;
  uint16_t pulseValueLow = 0 ;

  for ( uint32_t i = 0 ; i < 8 ; i += 1 )
	{
    uint8_t channel = firstChannel + i ;
    if (sendFailsafe)
		{
      if (g_model.Module[module].failsafeMode == FAILSAFE_HOLD)
			{
        pulseValue = 2047;
      }
      else if (g_model.Module[module].failsafeMode == FAILSAFE_NO_PULSES)
			{
        pulseValue = 0;
      }
      else
			{
        int32_t failsafeValue ;
				if ( channel < 16 )
				{
					failsafeValue = g_model.Module[module].failsafe[channel] ;
				}
				else
				{
//					uint32_t index = module * 8 + channel - 16 ;
					failsafeValue = g_model.accessFailsafe[module][channel - 16] ;
				}
				 
//				if (failsafeValue == FAILSAFE_CHANNEL_HOLD)
//				{
//          pulseValue = 2047;
//        }
//        else if (failsafeValue == FAILSAFE_CHANNEL_NOPULSE)
//				{
//          pulseValue = 0;
//        }
//        else
				{
					failsafeValue = ( failsafeValue *3933 ) >> 9 ;
//					failsafeValue += 1024 ;					
          pulseValue = limit(1, ((int16_t)failsafeValue) + 1024, 2046);
        }
      }
    }
    else
		{
      int value = g_chans512[channel] ; // + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
      pulseValue = limit(1, (value * 3 / 4) + 1024, 2046);
    }

    if (i & 1)
		{
      pxx2AddByte(pulseValueLow, module); // Low byte of channel
      pxx2AddByte(((pulseValueLow >> 8) & 0x0F) | (pulseValue << 4), module);  // 4 bits each from 2 channels
      pxx2AddByte(pulseValue >> 4, module);  // High byte of channel
    }
    else
		{
      pulseValueLow = pulseValue ;
    }
  }
}



void setupChannelsAccess( uint32_t module )
{
	uint32_t flag0 ;

	// For channels
	pxx2AddByte( PXX2_TYPE_C_MODULE, module ) ;
	pxx2AddByte( PXX2_TYPE_ID_CHANNELS, module ) ;

  // flag0 bit 7 range, bit 6 failsafe , rest - RxNum
//	uint8_t flag0 = addFlag0(module);
	flag0 = pxx2AddFlag0( module ) ;

  // flag1
  
	uint8_t flag1 = g_model.Access[module].type << 4;

	pxx2AddByte( flag1, module ) ;

	addChannels( module, flag0 & PXX2_CHANNELS_FLAG0_FAILSAFE, g_model.Module[module].startChannel ) ;
	

	if ( g_model.Access[module].type )
	{
		if ( g_model.Module[module].channels == 0 )
		{
			addChannels( module, flag0 & PXX2_CHANNELS_FLAG0_FAILSAFE, g_model.Module[module].startChannel+8 ) ;
		}
	}
	else
	{
		if ( g_model.Access[module].numChannels > 0 )
		{
			addChannels( module, flag0 & PXX2_CHANNELS_FLAG0_FAILSAFE, g_model.Module[module].startChannel+8 ) ;
		}
		if ( g_model.Access[module].numChannels > 1 )
		{
			addChannels( module, flag0 & PXX2_CHANNELS_FLAG0_FAILSAFE, g_model.Module[module].startChannel+16 ) ;
		}
	}
//	if (size > 0)
//	{
		// update the frame LEN = frame length minus the 2 first bytes
//	}
}


void setupHardwareInfoFrame( uint32_t module )
{
  if ( ModuleControl[module].step >= -1 && ModuleControl[module].step < 2 ) //  PXX2_MAX_RECEIVERS_PER_MODULE)
	{
    if (ModuleControl[module].timeout == 0)
		{
			pxx2AddByte( PXX2_TYPE_C_MODULE, module ) ;
			pxx2AddByte( PXX2_TYPE_ID_HW_INFO, module ) ;
      
			pxx2AddByte(ModuleControl[module].step, module);
      ModuleControl[module].timeout = 20;
//      ModuleControl[module].step++;
    }
    else
		{
      ModuleControl[module].timeout -= 1 ;
      setupChannelsAccess(module) ;
    }
  }
  else
	{
    ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
    setupChannelsAccess(module);
  }
}

void pxx2AddRegId( uint32_t module )
{
	uint32_t i ;
	uint8_t *p ;
	p = g_eeGeneral.radioRegistrationID ;
  for ( i = 0 ; i < PXX2_LEN_REGISTRATION_ID ; i += 1 )
	{
    pxx2AddByte( *p++, module ) ;
	}
}


void setupRegisterFrame(uint8_t module)
{
//  if ( ModuleControl[module].registerStep == REGISTER_RX_NAME_RECEIVED )
//	{
//    ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
//    setupChannelsAccess(module);
//		return ;
//	}
	
	pxx2AddByte( PXX2_TYPE_C_MODULE, module ) ;
	pxx2AddByte( PXX2_TYPE_ID_REGISTER, module ) ;

  if (ModuleControl[module].registerStep == REGISTER_RX_NAME_SELECTED)
	{
		pxx2AddByte(0x01, module) ;
    for (uint8_t i=0; i<PXX2_LEN_RX_NAME; i++)
		{
      pxx2AddByte( ModuleControl[module].registerRxName[i], module) ;
    }
		pxx2AddRegId( module ) ;
    pxx2AddByte( ModuleControl[module].registerModuleIndex, module);
  }
  else
	{
    pxx2AddByte(0, module) ;
  }
}

void setupResetFrame(uint8_t module)
{
	pxx2AddByte( PXX2_TYPE_C_MODULE, module ) ;
	pxx2AddByte( PXX2_TYPE_ID_RESET, module ) ;
	pxx2AddByte( ModuleControl[module].bindReceiverIndex, module ) ;
	pxx2AddByte( ModuleControl[module].resetType, module ) ;
}

void setupShareFrame(uint8_t module)
{
	pxx2AddByte( PXX2_TYPE_C_MODULE, module ) ;
	pxx2AddByte( PXX2_TYPE_ID_SHARE, module ) ;
	pxx2AddByte( ModuleControl[module].bindReceiverIndex, module ) ;
}


void setupAccstBindFrame(uint8_t module)
{
	pxx2AddByte( PXX2_TYPE_C_MODULE, module ) ;
	pxx2AddByte( PXX2_TYPE_ID_BIND, module ) ;
	pxx2AddByte(0x01, module) ;
  for (uint8_t i=0; i<PXX2_LEN_RX_NAME; i++)
	{
    pxx2AddByte(0x00, module) ;
  }
	pxx2AddByte((g_model.Module[module].highChannels << 7) + (g_model.Module[module].disableTelemetry << 6), module) ;
  pxx2AddByte(g_model.Module[module].pxxRxNum, module) ;
}

void setupBindFrame(uint8_t module)
{
  if ( ModuleControl[module].bindStep == BIND_WAIT)
	{
    if ( ( (uint16_t) (get_tmr10ms() - ModuleControl[module].bindWaitTimeout)) > 30 )
		{
      ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
      ModuleControl[module].bindStep = BIND_OK ;
//      POPUP_INFORMATION(STR_BIND_OK);
    }
    return ;
  }
  
	if ( ModuleControl[module].bindStep == BIND_OK)
	{
    setupChannelsAccess(module) ;
		return ;
	}

	pxx2AddByte( PXX2_TYPE_C_MODULE, module ) ;
	pxx2AddByte( PXX2_TYPE_ID_BIND, module ) ;

  if ( ModuleControl[module].bindStep == BIND_RX_NAME_SELECTED)
	{
    pxx2AddByte(0x01, module) ;
    for ( uint32_t i=0; i<PXX2_LEN_RX_NAME; i += 1)
		{
      pxx2AddByte( ModuleControl[module].bindReceiversNames[ModuleControl[module].bindReceiverNameIndex][i], module) ;
    }
    pxx2AddByte( ModuleControl[module].bindReceiverId, module); // RX_UID is the slot index (which is unique and never moved)
    pxx2AddByte(g_model.Module[module].pxxRxNum, module);
  }
  else
	{
    pxx2AddByte(0x00, module) ;
		pxx2AddRegId( module ) ;
  }
}

void setupReceiverSettingsFrame(uint8_t module)
{
  if ( ( (uint16_t) (get_tmr10ms() - ModuleControl[module].receiverSetupTimeout)) > 200 ) /*next try in 2s*/
	{
		pxx2AddByte( PXX2_TYPE_C_MODULE, module ) ;
		pxx2AddByte( PXX2_TYPE_ID_RX_SETTINGS, module ) ;
    uint8_t flag0 = ModuleControl[module].receiverSetupReceiverId ;
    if ( ModuleControl[module].rxtxSetupState == RECEIVER_SETTINGS_WRITE)
		{
      flag0 |= PXX2_RX_SETTINGS_FLAG0_WRITE ;
		}
    pxx2AddByte(flag0, module);
    uint8_t flag1 = 0;
    if (ModuleControl[module].receiverSetupTelemetryDisabled)
      flag1 |= PXX2_RX_SETTINGS_FLAG1_TELEMETRY_DISABLED ;
    if (ModuleControl[module].receiverSetupPwmRate)
      flag1 |= PXX2_RX_SETTINGS_FLAG1_FASTPWM ;
    pxx2AddByte(flag1, module);

//    uint8_t channelsCount = sentModuleChannels(module);
//    for (int i = 0; i < channelsCount ; i++)
    for (int i = 0; i < 8 ; i += 1 )
		{
      pxx2AddByte( ModuleControl[module].channelMapping[i], module) ;
    }
    ModuleControl[module].receiverSetupTimeout = get_tmr10ms() ;
  }
  else
	{
    setupChannelsAccess(module) ;
  }
}

void setupGetPowerFrame(uint8_t module)
{
  uint8_t flag0 = 0 ;
  uint8_t flag1 = 0 ;
  uint8_t power = 1 ;
	pxx2AddByte( PXX2_TYPE_C_MODULE, module ) ;
	pxx2AddByte( PXX2_TYPE_ID_TX_SETTINGS, module ) ;
	if ( ModuleControl[module].moduleRequest == 1 )
	{
		flag0 = 0x40 ;
		if ( ModuleControl[module].moduleExtAerial )
		{
			flag1 = 0x08 ;
		}
		power = ModuleControl[module].power ;
	}
  pxx2AddByte( flag0, module) ;
  pxx2AddByte( flag1, module) ;
  pxx2AddByte( power, module) ;
  ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
}
void setupTelemetryFrame(uint8_t module)
{
	uint32_t i ;
	pxx2AddByte( PXX2_TYPE_C_MODULE, module ) ;
	pxx2AddByte( PXX2_TYPE_ID_TELEMETRY, module ) ;
	pxx2AddByte( ( TelemetryTx.AccessSportTx.module_destination >> 8) & 0x03 , module) ;
//	pxx2AddByte( TelemetryTx.AccessSportTx.index, module) ;
	for ( i = 0 ; i < 8 ; i += 1 )
	{
		pxx2AddByte( TelemetryTx.AccessSportTx.data[i], module ) ;
	}
	TelemetryTx.sportCount = 0 ;
}


void setupSpectrumAnalyser( uint8_t module )
{
	if ( ModuleControl[module].step )
	{
		return ;
	}
	ModuleControl[module].step = 1 ;
	
//  if (moduleSettings[module].counter > 1000) {
//    moduleSettings[module].counter = 1002;
//    return;
//  }

//  moduleSettings[module].counter = 1002;
		 
	pxx2AddByte( PXX2_TYPE_C_POWER_METER, module ) ;
	pxx2AddByte( PXX2_TYPE_ID_SPECTRUM, module ) ;

	pxx2AddByte( 0, module ) ;

  pxx2AddWord(SharedMemory.SpectrumAnalyser.freq, module ) ;
  pxx2AddWord(SharedMemory.SpectrumAnalyser.span, module ) ;
  pxx2AddWord(SharedMemory.SpectrumAnalyser.step, module ) ;
}

void setupPulsesAccess( uint32_t module )
{
	PtrSerialPxx[module] = PxxSerial[module] ;
	AccessCrc[module] = 0xFFFF ;
	// Add header
	*PtrSerialPxx[module]++ = 0x7E ;
	*PtrSerialPxx[module]++ = 0 ;		// Place for length

//extern uint8_t RawLogging ;
//void rawLogByte( uint8_t byte ) ;

//	if ( TelemetryTx.sportCount )
//	{
//    setupTelemetryFrame(module) ;
//	}
//	else
//	{
  	switch (ModuleSettings[module].mode)
		{
  	  case MODULE_MODE_GET_HARDWARE_INFO:
  	    setupHardwareInfoFrame(module);
	//			if ( RawLogging )
	//			{
	//				rawLogByte( 0x5E ) ;
	//				rawLogByte( 0x01 ) ;
	//				DebugLog = 1 ;
	//			}
  	  break;
  	  case MODULE_MODE_RECEIVER_SETTINGS:
  	    setupReceiverSettingsFrame(module);
	//			if ( RawLogging )
	//			{
	//				rawLogByte( 0x5E ) ;
	//				rawLogByte( 0x02 ) ;
	//				DebugLog = 1 ;
	//			}
  	  break;
  	  case MODULE_MODE_REGISTER:
  	    setupRegisterFrame(module) ;
	//			if ( RawLogging )
	//			{
	//				rawLogByte( 0x5E ) ;
	//				rawLogByte( 0x03 ) ;
	//				DebugLog = 1 ;
	//			}
  	  break;
  	  case MODULE_MODE_BIND:
				if ( g_model.Access[module].type)
				{
					setupAccstBindFrame( module ) ;
				}
				else
				{
  	    	setupBindFrame(module);
				}
	//			if ( RawLogging )
	//			{
	//				rawLogByte( 0x5E ) ;
	//				rawLogByte( 0x04 ) ;
	//				DebugLog = 1 ;
	//			}
  	  break;
			case MODULE_MODE_GETSET_TX :
				setupGetPowerFrame(module) ;
	//			if ( RawLogging )
	//			{
	//				rawLogByte( 0x5E ) ;
	//				rawLogByte( 0x05 ) ;
	//				DebugLog = 1 ;
	//			}
  	  break ;

			case MODULE_MODE_RESET :
				setupResetFrame( module ) ;
	//			if ( RawLogging )
	//			{
	//				rawLogByte( 0x5E ) ;
	//				rawLogByte( 0x06 ) ;
	//				DebugLog = 1 ;
	//			}
  	  break ;

			case MODULE_MODE_SHARE :
				setupShareFrame( module ) ;
  	  break ;
			
	    case MODULE_MODE_SPECTRUM_ANALYSER:
	      setupSpectrumAnalyser(module);
	    break ;

  	  default:
	//			if ( RawLogging )
	//			{
	//				rawLogByte( 0x5E ) ;
	//				rawLogByte( 0x55 ) ;
	//				DebugLog = 1 ;
	//			}
				
				
//      if (outputTelemetryBuffer.isModuleDestination(module)) {
				if ( TelemetryTx.sportCount && ( TelemetryTx.AccessSportTx.index == 0xFF ) && ( ( ( TelemetryTx.AccessSportTx.module_destination >> 10 ) & 0x03  ) == module ) )
				{
					setupTelemetryFrame( module ) ;
				}
				else
				{
					setupChannelsAccess( module ) ;
				}
  	  break;
  	}
	
//	}

//  if (moduleSettings[module].counter-- == 0)
//	{
//    moduleSettings[module].counter = 1000;
//  }

 	PxxSerial[module][1] = PtrSerialPxx[module] - PxxSerial[module] - 2;

	*PtrSerialPxx[module]++ = AccessCrc[module] >> 8 ;
	*PtrSerialPxx[module] = AccessCrc[module] ;

//	if ( RawLogging )
//	{
//		if ( DebugLog )
//		{
//			uint8_t *p ;
//			uint32_t count ;
//			p = PxxSerial[module] ;
//			count = p[1] + 4 ;
//			while ( count-- )
//			{
//				rawLogByte( *p++ ) ;
//			}
//			DebugLog = 0 ;
//		}
//	}

	if ( PxxSerial[module][1] )
	{
		
		if ( module )
		{
extern volatile uint8_t *PxxTxPtr_x ;
extern volatile uint8_t PxxTxCount_x ;
			PxxTxPtr_x = PxxSerial[EXTERNAL_MODULE] ;
			PxxTxCount_x = PxxSerial[module][1] + 4 ;
			EXTMODULE_USART->CR1 |= USART_CR1_TXEIE ;		// Enable this interrupt
		}
		else
		{
extern volatile uint8_t *PxxTxPtr ;
extern volatile uint8_t PxxTxCount ;
			PxxTxPtr = PxxSerial[0] ;
			PxxTxCount = PxxSerial[module][1] + 4 ;
			INTMODULE_USART->CR1 |= USART_CR1_TXEIE ;		// Enable this interrupt
		}
	}
}


#define ACCESS_IDLE			0
#define ACCESS_START		1
#define ACCESS_DATA			2
#define ACCESS_CRC1			3
#define ACCESS_CRC2			4

#define START_STOP      0x7E

struct t_accessTelemetry
{
	uint16_t dataCrc ;
	uint16_t startTime ;
	uint8_t dataState ;
	uint8_t dataCount ;
	uint8_t dataReceived ;
	uint8_t AccessPacket[50] ;
} AccessTelemetry[2] ;


void byteCopy( uint8_t *dest, uint8_t *src, uint32_t length )
{
	uint32_t i ;
	for ( i = 0 ; i < length ; i += 1 )
	{
		*dest++ = *src++ ;
	}
}

uint32_t byteMatch( uint8_t *dest, uint8_t *src, uint32_t length )
{
	uint32_t i ;
	for ( i = 0 ; i < length ; i += 1 )
	{
		if ( *dest++ != *src++ )
		{
			return 0 ;
		}
	}
	return 1 ;
}



void processRegisterFrame(uint8_t module, uint8_t *frame)
{
  if ( ModuleSettings[module].mode != MODULE_MODE_REGISTER)
	{
    return ;
  }
  switch(frame[3])
	{
    case 0x00:
      if ( ModuleControl[module].registerStep == REGISTER_START)
			{
        // RX_NAME follows, we store it for the next step
        byteCopy( ModuleControl[module].registerRxName, &frame[4], PXX2_LEN_RX_NAME ) ;
				ModuleControl[module].registerModuleIndex = frame[12] ;
        ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
        ModuleControl[module].registerStep = REGISTER_RX_NAME_RECEIVED ;
      }
    break ;

    case 0x01:
      if ( ModuleControl[module].registerStep == REGISTER_RX_NAME_SELECTED)
			{
        // RX_NAME + PASSWORD follow, we check they are good
        if ( byteMatch( &frame[4], ModuleControl[module].registerRxName, PXX2_LEN_RX_NAME) &&
            byteMatch( &frame[12], g_eeGeneral.radioRegistrationID, PXX2_LEN_REGISTRATION_ID))
				{
          ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
          ModuleControl[module].registerStep = REGISTER_OK;
//          POPUP_INFORMATION(STR_REG_OK);
        }
      }
    break ;
  }
}

void processBindFrame( uint8_t module, uint8_t *frame )
{
  if ( ModuleSettings[module].mode != MODULE_MODE_BIND )
	{
    return ;
  }


  switch(frame[3])
	{
    case 0 :
      if ( ModuleControl[module].bindStep == BIND_START)
			{
        bool found = false;
        for (uint32_t i=0; i<ModuleControl[module].bindReceiverCount ; i += 1 )
				{
          if (byteMatch(ModuleControl[module].bindReceiversNames[i], &frame[4], PXX2_LEN_RX_NAME))
					{
            found = true ;
            break ;
          }
        }
        if (!found && ModuleControl[module].bindReceiverCount < PXX2_MAX_RECEIVERS_PER_MODULE )
				{
          byteCopy(ModuleControl[module].bindReceiversNames[ModuleControl[module].bindReceiverCount++], &frame[4], PXX2_LEN_RX_NAME ) ;
        }
      }
		break ;

		case 1 :
      if ( ModuleControl[module].bindStep == BIND_RX_NAME_SELECTED)
			{
        if (byteMatch(ModuleControl[module].bindReceiversNames[ModuleControl[module].bindReceiverNameIndex], &frame[4], PXX2_LEN_RX_NAME))
				{
          byteCopy(g_model.Access[module].receiverName[ModuleControl[module].bindReceiverIndex], &frame[4], PXX2_LEN_RX_NAME) ;
					eeDirty(EE_MODEL) ;
          ModuleControl[module].bindStep = BIND_WAIT ;
					ModuleControl[module].bindWaitTimeout = get_tmr10ms() ;
//          ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
        }
      }
		break ;
	}
}

void processResetFrame( uint8_t module, uint8_t *frame )
{
  if ( ModuleSettings[module].mode != MODULE_MODE_RESET )
	{
    return ;
  }
	if ( frame[3] == ModuleControl[module].bindReceiverIndex )
	{
		ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
	}
}

void processSpectrumAnalyserFrame( uint8_t module, uint8_t *frame)
{
  if (ModuleSettings[module].mode != MODULE_MODE_SPECTRUM_ANALYSER)
	{
    return ;
  }

  uint32_t * frequency = (uint32_t *)&frame[4];
  int8_t * power = (int8_t *)&frame[8];

  // center = 2440000000;  // 2440MHz
  // span = 40000000;  // 40MHz
  // left = 2440000000 - 20000000
  // step = 10000

  int32_t position = *frequency - (SharedMemory.SpectrumAnalyser.freq - SharedMemory.SpectrumAnalyser.span / 2);
  int32_t x = (position * 128/*LCD_W*/ / 8) / (SharedMemory.SpectrumAnalyser.span / 8);
  if (x < 128/*LCD_W*/)
	{
    SharedMemory.SpectrumAnalyser.bars[x] = 127 + *power ;
  }
}



// Packet contains count followed by actual data
void processAccessFrame( uint8_t *packet, uint32_t module )
{
//extern uint8_t RawLogging ;
//void rawLogByte( uint8_t byte ) ;
//	if ( RawLogging )
//	{
//		rawLogByte( 0x5F ) ;
//		rawLogByte( packet[2] ) ;
//	}
	if ( packet[1] == PXX2_TYPE_C_MODULE )
	{
		switch ( packet[2] )
		{
			case PXX2_TYPE_ID_TELEMETRY :
				processSportData( &packet[4], packet[3] & 0x0F ) ;
			break ;
  	  case PXX2_TYPE_ID_REGISTER :
  	    processRegisterFrame( module, packet ) ;
			break ;
  	  case PXX2_TYPE_ID_HW_INFO :
				if ( packet[3] == 0xFF )
				{
					ModuleControl[module].hwVersion = ( packet[5] << 8 ) | packet[6] ;
					ModuleControl[module].swVersion = ( packet[7] << 8 ) | packet[8] ;
					ModuleControl[module].variant = packet[9] ;
					ModuleControl[module].moduleId = packet[4] ;
				}
				else
				{
					if ( packet[3] == ModuleControl[module].step )
					{
						ModuleControl[module].rxHwVersion = ( packet[5] << 8 ) | packet[6] ;
						ModuleControl[module].rxSwVersion = ( packet[7] << 8 ) | packet[8] ;
						ModuleControl[module].variant = packet[9] ;
						ModuleControl[module].rxModuleId = packet[4] ;
					}
				}
  	    ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
			break ;

			case PXX2_TYPE_ID_BIND :
  	    processBindFrame( module, packet ) ;
			break ;
			
  	  case PXX2_TYPE_ID_TX_SETTINGS :
				ModuleControl[module].power = packet[5] ;
				ModuleControl[module].moduleExtAerial = packet[4] & 0x08 ? 1 : 0 ;
  	    ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
				ModuleControl[module].rxtxSetupState = MODULE_SETTINGS_OK ;
			break ;

			case PXX2_TYPE_ID_RX_SETTINGS :
				ModuleControl[module].receiverSetupTelemetryDisabled = packet[4] & 0x80 ? 1 : 0 ; 
				ModuleControl[module].receiverSetupPwmRate = packet[4] & 0x10 ? 1 : 0 ;
				for ( uint32_t i = 0 ; i < 8 ; i += 1 )
				{
					ModuleControl[module].channelMapping[i] = packet[5+i] ;
				}
				ModuleControl[module].rxtxSetupState = RECEIVER_SETTINGS_OK ;
			break ;

  	  case PXX2_TYPE_ID_RESET :
  	    processResetFrame( module, packet ) ;
			break ;

			case PXX2_TYPE_ID_SHARE :
				if ( packet[3] < 3 )
				{
  	     	memset(g_model.Access[module].receiverName[packet[3]], 0, PXX2_LEN_RX_NAME ) ;
				}
			break ;
		}
	}
	else if ( packet[1] == PXX2_TYPE_C_POWER_METER )
	{
		switch ( packet[2] )
		{
			case PXX2_TYPE_ID_SPECTRUM :
				processSpectrumAnalyserFrame( module, packet ) ;
			break ;
		}
		
	}

}


void accessRecieveByte( uint16_t data, uint32_t module )
{
	uint8_t byte ;
	struct t_accessTelemetry *at ;
	byte = data ;
	data &= 0xFF00 ;
extern uint16_t TelRxCount ;
	TelRxCount += 1 ;
extern uint8_t RawLogging ;
void rawLogByte( uint8_t byte ) ;
	if ( RawLogging )
	{
		rawLogByte( byte ) ;
//		if ( byte == START_STOP )
//		{
//			rawLogByte( data >> 8 ) ;
//		}
	}

	at = &AccessTelemetry[module] ;

	if ( ( byte == START_STOP ) ) // && ( ( uint16_t)( data - at->startTime ) > 3000 ) )
	{
		at->dataState = ACCESS_START ;
		at->dataCount = 0 ;
		at->startTime = data ;
	}
	else
	{
 		switch (at->dataState) 
		{
			case ACCESS_IDLE :
				if ( byte == START_STOP )
				{
					at->dataState = ACCESS_START ;
					at->dataCount = 0 ;
					at->startTime = data ;
				}
			break ;

			case ACCESS_START :
				at->dataState = ACCESS_DATA ;
				at->dataCount = byte ;
				at->dataReceived = 1 ;
				at->AccessPacket[0] = byte ;
				at->dataCrc = 0xFFFF ;
				if ( byte > 48 )
				{
					at->dataState = ACCESS_IDLE ;
				}
			break ;

			case ACCESS_DATA :
				at->dataCrc -= byte ;
				at->AccessPacket[at->dataReceived++] = byte ;
				if ( at->dataReceived > at->dataCount )
				{
					at->dataState = ACCESS_CRC1 ;
				}
			break ;

			case ACCESS_CRC1 :
				at->dataCrc -= byte << 8 ;
				at->dataState = ACCESS_CRC2 ;
			break ;

			case ACCESS_CRC2 :
				at->dataCrc -= byte ;
				at->dataState = ACCESS_IDLE ;
//				if ( RawLogging )
//				{
//					rawLogByte( at->dataCrc >> 8 ) ;
//					rawLogByte( at->dataCrc ) ;
//				}
				if ( at->dataCrc == 0 )
				{
					processAccessFrame( at->AccessPacket, module ) ;
				}
			break ;

		}
	}
}



//uint8_t Pxx2Pulses::addFlag0(uint8_t module)
//{
//  uint8_t flag0 = g_model.header.modelId[module] & 0x3F;
//  if (g_model.moduleData[module].failsafeMode != FAILSAFE_NOT_SET && g_model.moduleData[module].failsafeMode != FAILSAFE_RECEIVER) {
//    if (moduleSettings[module].counter == 0) {
//      flag0 |= PXX2_CHANNELS_FLAG0_FAILSAFE;
//    }
//  }
//  if (moduleSettings[module].mode == MODULE_MODE_RANGECHECK) {
//    flag0 |= PXX2_CHANNELS_FLAG0_RANGECHECK;
//  }
//  Pxx2Transport::addByte(flag0);
//  return flag0;
//}

//void Pxx2Pulses::addFlag1(uint8_t module)
//{
//  uint8_t flag1 = 0;
//  Pxx2Transport::addByte(flag1);
//}

//void Pxx2Pulses::addChannels(uint8_t module, uint8_t sendFailsafe, uint8_t firstChannel)
//{
//  uint16_t pulseValue = 0;
//  uint16_t pulseValueLow = 0;

//  for (int8_t i=0; i<8; i++) {
//    uint8_t channel = firstChannel + i;
//    if (sendFailsafe) {
//      if (g_model.moduleData[module].failsafeMode == FAILSAFE_HOLD) {
//        pulseValue = 2047;
//      }
//      else if (g_model.moduleData[module].failsafeMode == FAILSAFE_NOPULSES) {
//        pulseValue = 0;
//      }
//      else {
//        int16_t failsafeValue = g_model.failsafeChannels[channel];
//        if (failsafeValue == FAILSAFE_CHANNEL_HOLD) {
//          pulseValue = 2047;
//        }
//        else if (failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
//          pulseValue = 0;
//        }
//        else {
//          failsafeValue += 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
//          pulseValue = limit(1, (failsafeValue * 512 / 682) + 1024, 2046);
//        }
//      }
//    }
//    else {
//      int value = channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
//      pulseValue = limit(1, (value * 512 / 682) + 1024, 2046);
//    }

//    if (i & 1) {
//      Pxx2Transport::addByte(pulseValueLow); // Low byte of channel
//      Pxx2Transport::addByte(((pulseValueLow >> 8) & 0x0F) | (pulseValue << 4));  // 4 bits each from 2 channels
//      Pxx2Transport::addByte(pulseValue >> 4);  // High byte of channel
//    }
//    else {
//      pulseValueLow = pulseValue;
//    }
//  }
//}

//void Pxx2Pulses::setupChannelsFrame(uint8_t module)
//{
//  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_CHANNELS);

//  // FLAG0
//  uint8_t flag0 = addFlag0(module);

//  // FLAG1
//  addFlag1(module);

//  // Channels
//  uint8_t channelsCount = sentModuleChannels(module);
//  addChannels(module, flag0 & PXX2_CHANNELS_FLAG0_FAILSAFE, g_model.moduleData[module].channelsStart);
//  if (channelsCount > 8) {
//    addChannels(module, flag0 & PXX2_CHANNELS_FLAG0_FAILSAFE, g_model.moduleData[module].channelsStart + 8);
//    if (channelsCount > 16) {
//      addChannels(module, flag0 & PXX2_CHANNELS_FLAG0_FAILSAFE, g_model.moduleData[module].channelsStart + 16);
//    }
//  }
//}

//void Pxx2Pulses::setupHardwareInfoFrame(uint8_t module)
//{
//  if (reusableBuffer.hardware.modules[module].step >= -1 && reusableBuffer.hardware.modules[module].step < PXX2_MAX_RECEIVERS_PER_MODULE) {
//    if (reusableBuffer.hardware.modules[module].timeout == 0) {
//      addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_HW_INFO);
//      Pxx2Transport::addByte(reusableBuffer.hardware.modules[module].step);
//      reusableBuffer.hardware.modules[module].timeout = 20;
//      reusableBuffer.hardware.modules[module].step++;
//    }
//    else {
//      reusableBuffer.hardware.modules[module].timeout--;
//      setupChannelsFrame(module);
//    }
//  }
//  else {
//    moduleSettings[module].mode = MODULE_MODE_NORMAL;
//    setupChannelsFrame(module);
//  }
//}

//void Pxx2Pulses::setupRegisterFrame(uint8_t module)
//{
//  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_REGISTER);

//  if (reusableBuffer.moduleSetup.pxx2.registerStep == REGISTER_RX_NAME_SELECTED) {
//    Pxx2Transport::addByte(0x01);
//    for (uint8_t i=0; i<PXX2_LEN_RX_NAME; i++) {
//      Pxx2Transport::addByte(zchar2char(reusableBuffer.moduleSetup.pxx2.registerRxName[i]));
//    }
//    for (uint8_t i=0; i<PXX2_LEN_REGISTRATION_ID; i++) {
//      Pxx2Transport::addByte(zchar2char(g_model.modelRegistrationID[i]));
//    }
//    Pxx2Transport::addByte(reusableBuffer.moduleSetup.pxx2.registerModuleIndex);
//  }
//  else {
//    Pxx2Transport::addByte(0);
//  }
//}

//void Pxx2Pulses::setupReceiverSettingsFrame(uint8_t module)
//{
//  if (get_tmr10ms() > reusableBuffer.receiverSetup.timeout) {
//    addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_RX_SETTINGS);
//    uint8_t flag0 = reusableBuffer.receiverSetup.receiverId;
//    if (reusableBuffer.receiverSetup.state == RECEIVER_SETTINGS_WRITE)
//      flag0 |= PXX2_RX_SETTINGS_FLAG0_WRITE;
//    Pxx2Transport::addByte(flag0);
//    uint8_t flag1 = 0;
//    if (reusableBuffer.receiverSetup.telemetryDisabled)
//      flag1 |= PXX2_RX_SETTINGS_FLAG1_TELEMETRY_DISABLED;
//    if (reusableBuffer.receiverSetup.pwmRate)
//      flag1 |= PXX2_RX_SETTINGS_FLAG1_FASTPWM;
//    Pxx2Transport::addByte(flag1);
//    uint8_t channelsCount = sentModuleChannels(module);
//    for (int i = 0; i < channelsCount; i++) {
//      Pxx2Transport::addByte(reusableBuffer.receiverSetup.channelMapping[i]);
//    }
//    reusableBuffer.receiverSetup.timeout = get_tmr10ms() + 200/*next try in 2s*/;
//  }
//  else {
//    setupChannelsFrame(module);
//  }
//}

//void Pxx2Pulses::setupBindFrame(uint8_t module)
//{
//  if (reusableBuffer.moduleSetup.pxx2.bindStep == BIND_WAIT) {
//    if (get_tmr10ms() > reusableBuffer.moduleSetup.pxx2.bindWaitTimeout) {
//      moduleSettings[module].mode = MODULE_MODE_NORMAL;
//      reusableBuffer.moduleSetup.pxx2.bindStep = BIND_OK;
//      POPUP_INFORMATION(STR_BIND_OK);
//    }
//    return;
//  }

//  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_BIND);

//  if (reusableBuffer.moduleSetup.pxx2.bindStep == BIND_RX_NAME_SELECTED) {
//    Pxx2Transport::addByte(0x01);
//    for (uint8_t i=0; i<PXX2_LEN_RX_NAME; i++) {
//      Pxx2Transport::addByte(reusableBuffer.moduleSetup.pxx2.bindCandidateReceiversNames[reusableBuffer.moduleSetup.pxx2.bindSelectedReceiverIndex][i]);
//    }
//    Pxx2Transport::addByte(reusableBuffer.moduleSetup.pxx2.bindReceiverId); // RX_UID is the slot index (which is unique and never moved)
//    Pxx2Transport::addByte(g_model.header.modelId[module]);
//  }
//  else {
//    Pxx2Transport::addByte(0x00);
//    for (uint8_t i=0; i<PXX2_LEN_REGISTRATION_ID; i++) {
//      Pxx2Transport::addByte(zchar2char(g_model.modelRegistrationID[i]));
//    }
//  }
//}

//void Pxx2Pulses::setupSpectrumAnalyser(uint8_t module)
//{
//  if (moduleSettings[module].counter > 1000) {
//    moduleSettings[module].counter = 1002;
//    return;
//  }

//  moduleSettings[module].counter = 1002;

//  addFrameType(PXX2_TYPE_C_POWER_METER, PXX2_TYPE_ID_SPECTRUM);
//  Pxx2Transport::addByte(0x00);

//  reusableBuffer.spectrum.fq = 2440000000;  // 2440MHz
//  Pxx2Transport::addWord(reusableBuffer.spectrum.fq);

//  reusableBuffer.spectrum.span = 40000000;  // 40MHz
//  Pxx2Transport::addWord(reusableBuffer.spectrum.span);

//  reusableBuffer.spectrum.step = 100000;  // 100KHz
//  Pxx2Transport::addWord(reusableBuffer.spectrum.step);
//}

//void Pxx2Pulses::setupShareMode(uint8_t module)
//{
//  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_SHARE);
//  Pxx2Transport::addByte(reusableBuffer.moduleSetup.pxx2.shareReceiverId);
//}

//void Pxx2Pulses::setupFrame(uint8_t module)
//{
//  initFrame();

//  switch (moduleSettings[module].mode) {
//    case MODULE_MODE_GET_HARDWARE_INFO:
//      setupHardwareInfoFrame(module);
//      break;
//    case MODULE_MODE_RECEIVER_SETTINGS:
//      setupReceiverSettingsFrame(module);
//      break;
//    case MODULE_MODE_REGISTER:
//      setupRegisterFrame(module);
//      break;
//    case MODULE_MODE_BIND:
//      setupBindFrame(module);
//      break;
//    case MODULE_MODE_SPECTRUM_ANALYSER:
//      setupSpectrumAnalyser(module);
//      break;
//    case MODULE_MODE_SHARE:
//      setupShareMode(module);
//      break;
//    default:
//      setupChannelsFrame(module);
//      break;
//  }

//  if (moduleSettings[module].counter-- == 0) {
//    moduleSettings[module].counter = 1000;
//  }

//  endFrame();
//}

//template class PxxPulses<Pxx2Transport>;

#endif

