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

#include "AT91SAM3S4.h"
#ifndef SIMU
#include "core_cm3.h"
#endif

#include "ersky9x.h"
#include "myeeprom.h"
#include "logicio.h"
#include "drivers.h"
#include "pulses.h"
#include "debug.h"

//#ifdef ASSAN
//uint8_t ProtocolDebug[16384] ;
//uint32_t ProtocolCount ;
//uint32_t ProtocolIn ;

//void putProtocolDebug( uint8_t byte )
//{
//	if ( ProtocolCount == 0xFFFFFFFF )
//	{
//		return ;
//	}
//	if ( ProtocolCount < 16384 )
//	{
//		ProtocolCount += 1 ;
//	}
//	ProtocolDebug[ProtocolIn] = byte ;
//	ProtocolIn += 1 ;
//	ProtocolIn &= 16383 ;
//}
//#endif

uint8_t Bit_pulses[64] ;			// Likely more than we need
uint8_t *Pulses2MHzptr ;

uint8_t Serial_byte ;
uint8_t Serial_bit_count;
uint8_t Serial_byte_count ;
uint8_t Current_protocol ;
uint8_t pxxFlag = 0 ;
uint16_t PcmCrc ;
uint8_t PcmOnesCount ;

volatile uint8_t Dsm_Type = 0 ;
uint8_t DsmInitCounter = 0 ;
//uint8_t Dsm_Type_channels = 12 ;
//uint8_t Dsm_Type_10bit = 0 ;				// 0 for 11 bit, 1 for 10 bit
//uint8_t Dsm_mode_response = 0 ;

uint16_t Pulses[18] ;//= {	2000, 2200, 2400, 2600, 2800, 3000, 3200, 3400, 9000, 0, 0, 0,0,0,0,0,0, 0 } ;
uint16_t Pulses2[18] ;//= {	2000, 2200, 2400, 2600, 2800, 3000, 3200, 3400, 9000, 0, 0, 0,0,0,0,0,0, 0 } ;
volatile uint32_t Pulses_index = 0 ;		// Modified in interrupt routine
volatile uint32_t Pulses2_index = 0 ;		// Modified in interrupt routine

// DSM2 control bits
#define BindBit 0x80
#define RangeCheckBit 0x20
#define FranceBit 0x10
#define DsmxBit  0x08
#define BadData 0x47

static uint8_t pass ;		// For PXX and DSM-9XR and ASSAN
static uint8_t bitlen ;
uint8_t DebugDsmPass ;

void crc( uint8_t data ) ;


void module_output_low()
{
	configure_pins( PIO_PA17, PIN_OUTPUT | PIN_PORTA | PIN_PULLUP | PIN_ODRAIN | PIN_LOW ) ;
}

void module_output_active()
{
	register Pio *pioptr ;
	
	pioptr = PIOA ;
	pioptr->PIO_ABCDSR[0] &= ~PIO_PA17 ;		// Peripheral C
 	pioptr->PIO_ABCDSR[1] |= PIO_PA17 ;			// Peripheral C
	pioptr->PIO_PDR = PIO_PA17 ;						// Disable bit A17 Assign to peripheral
#ifdef REVX
	if ( g_model.ppmOpenDrain )
	{
		pioptr->PIO_MDDR = PIO_PA17 ;						// Push Pull O/p in A17
	}
	else
	{
		pioptr->PIO_MDER = PIO_PA17 ;						// Open Drain O/p in A17
	}
#else
	pioptr->PIO_MDDR = PIO_PA17 ;						// Push Pull O/p in A17
#endif
	pioptr->PIO_PUER = PIO_PA17 ;						// With pull up
}


void init_main_ppm( uint32_t period, uint32_t out_enable )
{
	register Pwm *pwmptr ;
	
  perOut(g_chans512, 0) ;
  setupPulsesPPM() ;

	if ( out_enable )
	{
		module_output_active() ;
	}

	pwmptr = PWM ;
	// PWM3 for PPM output	 
	pwmptr->PWM_CH_NUM[3].PWM_CMR = 0x0004000B ;	// CLKA
	if (g_model.pulsePol == 0)
	{
		pwmptr->PWM_CH_NUM[3].PWM_CMR |= 0x00000200 ;	// CPOL
	}
	pwmptr->PWM_CH_NUM[3].PWM_CPDR = period ;			// Period in half uS
	pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = period ;	// Period in half uS
	pwmptr->PWM_CH_NUM[3].PWM_CDTY = g_model.ppmDelay*100+600 ;			// Duty in half uS
	pwmptr->PWM_CH_NUM[3].PWM_CDTYUPD = g_model.ppmDelay*100+600 ;		// Duty in half uS
	pwmptr->PWM_ENA = PWM_ENA_CHID3 ;						// Enable channel 3

	pwmptr->PWM_IER1 = PWM_IER1_CHID3 ;


#ifdef REVB
	configure_pins( PIO_PC15, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_B | PIN_PORTC | PIN_NO_PULLUP ) ;
#endif

#ifdef REVB
	// PWM1 for PPM2
	pwmptr->PWM_CH_NUM[1].PWM_CMR = 0x0000000B ;	// CLKB
	if (g_model.pulsePol == 0)
	{
		pwmptr->PWM_CH_NUM[1].PWM_CMR |= 0x00000200 ;	// CPOL
	}
	pwmptr->PWM_CH_NUM[1].PWM_CPDR = period ;			// Period
	pwmptr->PWM_CH_NUM[1].PWM_CPDRUPD = period ;		// Period
	pwmptr->PWM_CH_NUM[1].PWM_CDTY = g_model.ppmDelay*100+600 ;				// Duty
	pwmptr->PWM_CH_NUM[1].PWM_CDTYUPD = g_model.ppmDelay*100+600 ;		// Duty
	pwmptr->PWM_ENA = PWM_ENA_CHID1 ;						// Enable channel 1
#endif

	pwmptr->PWM_IER1 = PWM_IER1_CHID1 ;
  NVIC_SetPriority(PWM_IRQn, 3 ) ;
	NVIC_EnableIRQ(PWM_IRQn) ;

}

void disable_main_ppm()
{
	register Pio *pioptr ;
	
	pioptr = PIOA ;
	pioptr->PIO_PER = PIO_PA17 ;						// Assign A17 to PIO

	PWM->PWM_IDR1 = PWM_IDR1_CHID3 ;
//	NVIC_DisableIRQ(PWM_IRQn) ;
	
}

//void disable_ppm2()
//{
//	register Pio *pioptr ;
	
//	pioptr = PIOC ;
//	pioptr->PIO_PER = PIO_PC17 ;						// Assign A17 to PIO

//	PWM->PWM_IDR1 = PWM_IDR1_CHID1 ;
//	NVIC_DisableIRQ(PWM_IRQn) ;
//}


#ifndef SIMU
extern "C" void PWM_IRQHandler (void)
{
	register Pwm *pwmptr ;
	register Ssc *sscptr ;
	uint32_t period ;
	uint32_t reason ;

	pwmptr = PWM ;
	reason = pwmptr->PWM_ISR1 ;
	if ( reason & PWM_ISR1_CHID3 )
	{
		switch ( Current_protocol )		// Use the current, don't switch until set_up_pulses
		{
      case PROTO_PPM:
				pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = Pulses[Pulses_index++] ;	// Period in half uS
				if ( Pulses[Pulses_index] == 0 )
				{
					Pulses_index = 0 ;

					setupPulses() ;
				}
			break ;

      case PROTO_PXX:
				// Alternate periods of 6.5mS and 2.5 mS
				period = pwmptr->PWM_CH_NUM[3].PWM_CPDR ;
				if ( period == 5000 )	// 2.5 mS
				{
					period = 6500*2 ;
				}
				else
				{
					period = 5000 ;	// 2.5 mS
				}
				pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = period ;	// Period in half uS
				if ( period != 5000 )	// 2.5 mS
				{
					setupPulses() ;
				}
				else
				{
					// Kick off serial output here
					sscptr = SSC ;
					sscptr->SSC_TPR = (uint32_t) Bit_pulses ;
					sscptr->SSC_TCR = Serial_byte_count ;
					sscptr->SSC_PTCR = SSC_PTCR_TXTEN ;	// Start transfers
				}
			break ;

      case PROTO_MULTI:
      case PROTO_DSM2:
      case PROTO_ASSAN:
				// Alternate periods of 19.5mS/8.5mS and 2.5 mS
				period = pwmptr->PWM_CH_NUM[3].PWM_CPDR ;
				if ( period == 5000 )	// 2.5 mS
				{
//					if ( Dsm_Type )
//					{
						period = 8500*2 ;		// Total 11 mS
//					}
//					else
//					{
//						period = 19500*2 ;
//					}	 
				}
				else
				{
					period = 5000 ;
				}
				pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = period ;	// Period in half uS
				if ( period != 5000 )	// 2.5 mS
				{
					setupPulses() ;
				}
				else
				{
					// Kick off serial output here
					if ( Current_protocol == PROTO_ASSAN )
					{
						// Enable SSC, output
						PIOA->PIO_PDR = PIO_PA17 ;	// Assign A17 to Peripheral
						USART0->US_CR = US_CR_RXDIS ;
//						PIOA->PIO_PER = PIO_PA5 ;		// Assign A5 to PIO
					}
					sscptr = SSC ;
					sscptr->SSC_TPR = (uint32_t) Bit_pulses ;
					sscptr->SSC_TCR = Serial_byte_count ;
					sscptr->SSC_PTCR = SSC_PTCR_TXTEN ;	// Start transfers
					if ( Current_protocol == PROTO_ASSAN )
					{
						// Enable termination interrupt
						sscptr->SSC_IER = SSC_IER_TXBUFE ;
					}
				}
			break ;
		}
	}
	if ( reason & PWM_ISR1_CHID1 )
	{
		pwmptr->PWM_CH_NUM[1].PWM_CPDRUPD = Pulses2[Pulses2_index++] ;	// Period in half uS
		if ( Pulses2[Pulses2_index] == 0 )
		{
			Pulses2_index = 0 ;
			setupPulsesPPM2() ;
		}
	}
}
#endif


void put_serial_bit( uint8_t bit )
{
	Serial_byte >>= 1 ;
	if ( bit & 1 )
	{
		Serial_byte |= 0x80 ;		
	}
	if ( ++Serial_bit_count >= 8 )
	{
#ifdef ASSAN
	if ( g_model.protocol == PROTO_ASSAN )
	{
    *Pulses2MHzptr++ = ~Serial_byte ;
	}
	else
	{
    *Pulses2MHzptr++ = Serial_byte ;
	}
#else
    *Pulses2MHzptr++ = Serial_byte ;
#endif
		Serial_bit_count = 0 ;
		Serial_byte_count += 1 ;
	}
}

uint8_t DsmPass1[10] ;
uint32_t DsmPassIndex ;
#ifdef ASSAN
uint16_t DsmChecksum ;
#endif

#define BITLEN_SERIAL (8*2) //125000 Baud
#define BITLEN_SBUS (10*2) //100000 Baud

void sendByteDsm2(uint8_t b) //max 10changes 0 10 10 10 10 1
{
	uint32_t parity = 0 ;
#ifdef ASSAN
	DsmChecksum += b ;
#endif
	
	if ( DsmPassIndex < 10 )
	{
		DsmPass1[DsmPassIndex++] = b ;
	}
	put_serial_bit( 0 ) ;		// Start bit
  for( uint8_t i=0; i<8; i++)	 // 8 data Bits
	{
		put_serial_bit( b & 1 ) ;
		parity ^= b ;
		b >>= 1 ;
  }
	if ( g_model.protocol != PROTO_DSM2 )
	{
		put_serial_bit( parity & 1 ) ;
	}
	
	put_serial_bit( 1 ) ;		// Stop bit
	if ( Dsm_Type == 0 )
	{
		put_serial_bit( 1 ) ;		// Stop bit
	}
}


void dsmBindResponse( uint8_t mode, int8_t channels )
{
	// Process mode here
	uint8_t dsm_mode_response ;
//	Dsm_Type_channels = channels ;
#ifdef ASSAN
	if ( g_model.protocol == PROTO_ASSAN )
	{
		dsm_mode_response = mode & ( ORTX_USE_DSMX | ORTX_USE_11mS | ORTX_USE_11bit | ORTX_USE_TM ) ;
//		g_model.ppmNCH = channels ;
		g_model.dsmMode = dsm_mode_response ;
  	STORE_MODELVARS ;
		pxxFlag &= ~PXX_BIND ;
	}
	else
#endif
	{
		dsm_mode_response = mode & ( ORTX_USE_DSMX | ORTX_USE_11mS | ORTX_USE_11bit | ORTX_AUTO_MODE ) ;
//	channels -= 8 ;
//	channels /= 2 ;
		if ( ( g_model.ppmNCH != channels ) || ( g_model.dsmMode != ( dsm_mode_response | 0x80 ) ) )
		{
			g_model.ppmNCH = channels ;
			g_model.dsmMode = dsm_mode_response | 0x80 ;

	  	STORE_MODELVARS ;
		}
	}
}

#ifdef MULTI_DEBUG
uint32_t DebugMultiIndex ;
uint8_t DebugMultiData[32] ;
#endif

//static void sendByteCrcSerial(uint8_t b)
//{
//	crc(b) ;
//	sendByteDsm2(b) ;
//#ifdef MULTI_DEBUG
//	if ( DebugMultiIndex < 32 )
//	{
//		DebugMultiData[DebugMultiIndex++] = b ;
//	}
//#endif
//}


// This is the data stream to send, prepare after 19.5 mS
// Send after 22.5 mS


//static uint8_t *Dsm2_pulsePtr = pulses2MHz.pbyte ;
void setupPulsesDsm2(uint8_t chns)
{
  static uint8_t dsmDat[2+6*2+4]={0xFF,0x00,  0x00,0xAA,  0x05,0xFF,  0x09,0xFF,  0x0D,0xFF,  0x13,0x54,  0x14,0xAA } ;
  uint16_t required_baudrate ;
#ifdef PCBSKY
	uint8_t counter ;
#endif // PCBSKY

	required_baudrate = SCC_BAUD_125000 ;
#ifdef ASSAN
	if ( g_model.protocol == PROTO_ASSAN )
	{
		required_baudrate = SCC_BAUD_115200 ;
		Dsm_Type = 2 ;
		DsmChecksum = 0 ;
	}
	else
#endif
	if( (g_model.protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) )
	{
		required_baudrate = SCC_BAUD_115200 ;
		Dsm_Type = 1 ;
		// Consider inverting COM1 here
	}
	else if(g_model.protocol == PROTO_MULTI)
	{
		required_baudrate = SCC_BAUD_100000 ;
	}
	else
	{
		Dsm_Type = 0 ;
	}
	
#ifdef PCBSKY
	if ( required_baudrate != Scc_baudrate )
	{
		init_ssc( required_baudrate ) ;
	}
#endif // PCBSKY

	Serial_byte = 0 ;
	Serial_bit_count = 0 ;
	Serial_byte_count = 0 ;
  Pulses2MHzptr = Bit_pulses ;
	bitlen = BITLEN_SERIAL ;
    
	if ( Dsm_Type )
	{
#ifdef ASSAN
		uint8_t channels = (Dsm_Type == 2) ? 7 : 12 ;
#else
		uint8_t channels = 12 ;
#endif
		uint8_t flags = g_model.dsmMode ;
		if ( flags == 0 )
		{
//			flags = ORTX_USE_11mS | ORTX_USE_11bit | ORTX_USE_DSMX | ORTX_USE_TM ;
			if ( Dsm_Type == 1 )
			{
				flags = ORTX_AUTO_MODE ;
			}
		}
		else
		{
#ifdef ASSAN
			flags &= (Dsm_Type == 2) ? 0x0F : 0x7F ;
#else
			flags &= 0x7F ;
#endif
			channels = g_model.ppmNCH ;
		}
			 
//#ifdef PCB9XT
		if ( (dsmDat[0]&BindBit) && (!keyState(SW_Trainer) ) )
//#else
//		if ( (dsmDat[0]&BindBit) && (!keyState(SW_SH2) ) )
//#endif
		{
			dsmDat[0] &= ~BindBit	;
		}

		uint8_t startChan = g_model.startChannel ;
#ifdef ASSAN
		if ( Dsm_Type == 2 )
		{
			if ( (pxxFlag & PXX_BIND) || (dsmDat[0]&BindBit) )
			{
				flags = ORTX_BIND_FLAG | ORTX_USE_11mS | ORTX_USE_11bit ;
				if ( pxxFlag & PXX_DSMX )
				{
					flags |= ORTX_USE_DSMX ;
				}
				pass = 0 ;
			}
			// Need to choose dsmx/dsm2 as well

  		sendByteDsm2( flags ) ;
  		sendByteDsm2( 0 | ( (pxxFlag & PXX_RANGE_CHECK) ? 4: 7 ) ) ;		// 
  		sendByteDsm2( 0 ) ;
#ifdef ENABLE_DSM_MATCH  		
			sendByteDsm2( g_model.pxxRxNum ) ;	// Model number
#else
			sendByteDsm2( 1 ) ;	// Model number
#endif
			pass &= 0x80 ;
			if ( pass )
			{
				startChan += 7 ;
			}
			for(uint8_t i=0 ; i<7 ; i += 1 )
			{
				uint16_t pulse ;
				int16_t value = g_chans512[startChan] ;
				if ( ( flags & ORTX_USE_11bit ) == 0 )
				{
					pulse = limit(0, ((value*13)>>5)+512,1023) | (startChan << 10) ;
				}
				else
				{
					pulse = limit(0, ((value*349)>>9)+1024,2047) | (startChan << 11) ;
				}
				startChan += 1 ;
				if ( startChan <= channels )
				{
  		  	sendByteDsm2( ( pulse >> 8 ) | pass ) ;
    			sendByteDsm2( pulse & 0xff ) ;
				}
				else
				{
    			sendByteDsm2( 0xff ) ;
    			sendByteDsm2( 0xff ) ;
				}
			}
			uint16_t csum = DsmChecksum ;
    	sendByteDsm2( csum >> 8 ) ;
    	sendByteDsm2( csum ) ;
			if ( pass )
			{
				pass = 0 ;
			}
			else
			{
				pass = ( channels > 7 ) ? 0x80 : 0 ;
			}
		}
		else
		{
#endif // ASSAN
		DebugDsmPass = pass ;
		sendByteDsm2( 0xAA );
		if ( pass == 0 )
		{
  		sendByteDsm2( pass ) ;		// Actually is a 0
			// Do init packet
			if ( (pxxFlag & PXX_BIND) || (dsmDat[0]&BindBit) )
			{
				flags |= ORTX_BIND_FLAG ;
			}
			// Need to choose dsmx/dsm2 as well
  		sendByteDsm2( flags ) ;
  		sendByteDsm2( (pxxFlag & PXX_RANGE_CHECK) ? 4: 7 ) ;		// 
  		sendByteDsm2( channels ) ;			// Max channels
//  		sendByteDsm2( g_model.pxxRxNum ) ;		// Rx Num
#ifdef ENABLE_DSM_MATCH  		
			sendByteDsm2( g_model.pxxRxNum ) ;	// Model number
#else
  		sendByteDsm2( 1 ) ;		// 'Model Match' disabled
#endif
			pass = 1 ;
		}
		else
		{
			DsmPassIndex = 0 ;
			if ( pass == 2 )
			{
				startChan += 7 ;
			}
  		sendByteDsm2( pass );

			for(uint8_t i=0 ; i<7 ; i += 1 )
			{
				uint16_t pulse ;
				int16_t value = g_chans512[startChan] ;
				if ( ( flags & ORTX_USE_11bit ) == 0 )
				{
					pulse = limit(0, ((value*13)>>5)+512,1023) | (startChan << 10) ;
				}
				else
				{
					pulse = limit(0, ((value*349)>>9)+1024,2047) | (startChan << 11) ;
				}
				startChan += 1 ;
				if ( startChan <= channels )
				{
  		  	sendByteDsm2( pulse >> 8 ) ;
    			sendByteDsm2( pulse & 0xff ) ;
				}
				else
				{
    			sendByteDsm2( 0xff ) ;
    			sendByteDsm2( 0xff ) ;
				}
			}
			if ( ++pass > 2 )
			{
				pass = 1 ;				
			}
			if ( channels < 8 )
			{
				pass = 1 ;				
			}
			DsmInitCounter += 1 ;
			if( DsmInitCounter > 100)
			{
				DsmInitCounter = 0 ;
				pass = 0 ;
			}
			if (dsmDat[0]&BindBit)
			{
				pass = 0 ;		// Stay here
			}
		}
#ifdef ASSAN
		}
#endif
	}
	else
	{
  	// If more channels needed make sure the pulses union/array is large enough
		if(g_model.protocol == PROTO_DSM2)
		{
  		if (dsmDat[0]&BadData)  //first time through, setup header
  		{
  		  switch(g_model.sub_protocol)
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

	  	if((dsmDat[0]&BindBit)&&(!keyState(SW_Trainer)))  dsmDat[0]&=~BindBit;		//clear bind bit if trainer not pulled
  		if ((!(dsmDat[0]&BindBit))&& (pxxFlag & PXX_RANGE_CHECK)) dsmDat[0]|=RangeCheckBit;   //range check function
  		else dsmDat[0]&=~RangeCheckBit;
		}
		else // Multi
		{
			dsmDat[0] = (g_model.sub_protocol+1) & 0x5F;		// load sub_protocol and clear Bind & Range flags
			if (pxxFlag & PXX_BIND)	dsmDat[0] |=BindBit;		//set bind bit if bind menu is pressed
			if (pxxFlag & PXX_RANGE_CHECK)	dsmDat[0] |=RangeCheckBit;		//set bind bit if bind menu is pressed
		}

		if(g_model.protocol == PROTO_MULTI)
		{
			uint32_t outputbitsavailable = 0 ;
			uint32_t outputbits = 0 ;
			uint32_t i ;
			bitlen = BITLEN_SBUS ;
#ifdef MULTI_DEBUG
			DebugMultiIndex = 0 ;
#endif			
			sendByteDsm2(0x55) ;
			sendByteDsm2( dsmDat[0] ) ;
			sendByteDsm2((g_model.ppmNCH & 0xF0) | ( g_model.pxxRxNum & 0x0F ) );
			sendByteDsm2(g_model.option_protocol);
			
			for ( i = 0 ; i < 16 ; i += 1 )
			{
				int16_t x = g_chans512[i] ;
				x *= 4 ;
				x += x > 0 ? 4 : -4 ;
				x /= 5 ;
//#ifdef MULTI_PROTOCOL
				if ( g_model.protocol == PROTO_MULTI )
					x += 0x400 ;
				else
//#endif // MULTI_PROTOCOL
					x += 0x3E0 ;
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
					sendByteDsm2(j) ;
					outputbits >>= 8 ;
					outputbitsavailable -= 8 ;
				}
			}
			
//#ifdef MULTI_PROTOCOL
			if ( g_model.protocol == PROTO_SBUS )
//#endif // MULTI_PROTOCOL
			{
				sendByteDsm2(0);
				sendByteDsm2(0);
			}
			
//			uint16_t serialH = 0 ;
//			for(uint8_t i=0; i<8; i++)
//			{
//				uint16_t pulse = limit(0, ((g_chans512[i]*13)>>5)+512,1023);
//				sendByteCrcSerial(pulse & 0xff);
//				serialH<<=2;
//				serialH|=((pulse>>8)&0x03);
//			}
//			sendByteCrcSerial((serialH>>8)&0xff);
//			sendByteCrcSerial(serialH&0xff);
//			sendByteCrcSerial( PcmCrc&0xff);
//		}
////		else if ( g_model.protocol == PROTO_SBUS )
////		{
////			*p++ = 0x0F ;
////			for ( i = 0 ; i < 16 ; i += 1 )
////			{
////				int16_t x = g_chans512[i] ;
////				x *= 4 ;
////				x += x > 0 ? 4 : -4 ;
////				x /= 5 ;
////				x += 0x3E0 ;
////				if ( x < 0 )
////				{
////					x = 0 ;
////				}
////				if ( x > 2047 )
////				{
////					x = 2047 ;
////				}
////				outputbits |= x << outputbitsavailable ;
////				outputbitsavailable += 11 ;
////				while ( outputbitsavailable >= 8 )
////				{
////					uint8_t j = outputbits ;
////					checksum += j ;
////					if ( ( j == 2 ) || ( j == 3 ) )
////					{
////						j ^= 0x80 ;
////						*p++ = 3 ;		// "stuff"
////					}
////          *p++ = j ;
////					outputbits >>= 8 ;
////					outputbitsavailable -= 8 ;
////				}
////			}
////			*p++ = 0 ;
////			*p++ = 0 ;
////		}
		}
		else// not MULTI
		{
  		dsmDat[1]=g_model.pxxRxNum ;  //DSM2 Header second byte for model match
  		for(uint8_t i=0; i<chns; i++)
  		{
				uint16_t pulse = limit(0, ((g_chans512[g_model.startChannel+i]*13)>>5)+512,1023);
 			  dsmDat[2+2*i] = (i<<2) | ((pulse>>8)&0x03);
  		 	dsmDat[3+2*i] = pulse & 0xff;
  		}

  		for ( counter = 0 ; counter < 14 ; counter += 1 )
  		{
  		  sendByteDsm2(dsmDat[counter]);
  		}
		}
	}
 	for ( counter = 0 ; counter < 16 ; counter += 1 )
	{
		put_serial_bit( 1 ) ;		// 16 extra stop bits
	}
}

void startPulses()
{
	Current_protocol = g_model.protocol + 1 ;		// Not the same!
	setupPulses() ;	// For DSM-9XR this won't be sent
	pass = 0 ;			// Force a type 0 packet
}

void setupPulses()
{
  heartbeat |= HEART_TIMER_PULSES ;
	
  if ( Current_protocol != g_model.protocol )
  {
    switch( Current_protocol )
    {	// stop existing protocol hardware
      case PROTO_PPM:
				disable_main_ppm() ;
      break;
      case PROTO_PXX:
				disable_ssc() ;
      break;
      case PROTO_DSM2:
	    case PROTO_MULTI:
				disable_ssc() ;
      break;
#ifdef ASSAN
      case PROTO_ASSAN :
				disable_ssc() ;
      break;
#endif
    }
		
    Current_protocol = g_model.protocol ;
    switch(Current_protocol)
    { // Start new protocol hardware here
      case PROTO_PPM:
				init_main_ppm( 3000, 1 ) ;		// Initial period 1.5 mS, output on
      break;
      case PROTO_PXX:
				init_main_ppm( 5000, 0 ) ;		// Initial period 2.5 mS, output off
				init_ssc(SCC_BAUD_125000) ;
				PIOA->PIO_MDDR = PIO_PA17 ;						// Push Pull O/p in A17
      break;
#ifdef ASSAN
      case PROTO_ASSAN :
#endif
    	case PROTO_MULTI:
      case PROTO_DSM2:
				init_main_ppm( 5000, 0 ) ;		// Initial period 2.5 mS, output off
				init_ssc(SCC_BAUD_125000) ;
				PIOA->PIO_MDDR = PIO_PA17 ;						// Push Pull O/p in A17
				DsmInitCounter = 0 ;
//				Dsm_Type_channels = 12 ;
//				Dsm_Type_10bit = 0 ;				// 0 for 11 bit, 1 for 10 bit
//				Dsm_mode_response = 0 ;
				pass = 0 ;
      break;
    }
  }

// Set up output data here
	switch(Current_protocol)
  {
	  case PROTO_PPM:
      setupPulsesPPM();		// Don't enable interrupts through here
    break;
  	case PROTO_PXX:
//      sei() ;							// Interrupts allowed here
      setupPulsesPXX();
    break;
	  case PROTO_DSM2:
    case PROTO_MULTI:
//      sei() ;							// Interrupts allowed here
      setupPulsesDsm2( ( g_model.sub_protocol == DSM_9XR ) ? 12 : 6 ) ; 
    break;
#ifdef ASSAN
    case PROTO_ASSAN :
      setupPulsesDsm2( 12 ) ;
    break;
#endif
  }
}

void setupPulsesPPM()			// Don't enable interrupts through here
{
	register Pwm *pwmptr ;
	
	pwmptr = PWM ;
	// Now set up pulses
#define PPM_CENTER 1500*2
	int16_t PPM_range = g_model.extendedLimits ? 640*2 : 512*2;   //range of 0.7..1.7msec

  //Total frame length = 22.5msec
  //each pulse is 0.7..1.7ms long with a 0.3ms stop tail
  //The pulse ISR is 2mhz that's why everything is multiplied by 2
  uint16_t *ptr ;
  ptr = Pulses ;
	uint32_t p = (g_model.ppmNCH + 4) * 2 ;
	if ( p > 16 )
	{
		p -= 13 ;
	}
  p += g_model.startChannel ; //Channels *2
	if ( p > NUM_SKYCHNOUT )
	{
		p = NUM_SKYCHNOUT ;	// Don't run off the end		
	}
    
	pwmptr->PWM_CH_NUM[3].PWM_CDTYUPD = (g_model.ppmDelay*50+300)*2; //Stoplen *2
	
	if (g_model.pulsePol == 0)
	{
		pwmptr->PWM_CH_NUM[3].PWM_CMR |= 0x00000200 ;	// CPOL
	}
	else
	{
		pwmptr->PWM_CH_NUM[3].PWM_CMR &= ~0x00000200 ;	// CPOL
	}
    
	uint16_t rest=22500u*2; //Minimum Framelen=22.5 ms
  rest += (int16_t(g_model.ppmFrameLength))*1000;
  //    if(p>9) rest=p*(1720u*2 + q) + 4000u*2; //for more than 9 channels, frame must be longer
  for(uint32_t i=g_model.startChannel;i<p;i++)
	{ //NUM_SKYCHNOUT
  	int16_t v = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
   	rest-=(v);
	//        *ptr++ = q;      //moved down two lines
    	    //        pulses2MHz[j++] = q;
    *ptr++ = v ; /* as Pat MacKenzie suggests */
    	    //        pulses2MHz[j++] = v - q + 600; /* as Pat MacKenzie suggests */
	//        *ptr++ = q;      //to here
 	}
	//    *ptr=q;       //reverse these two assignments
	//    *(ptr+1)=rest;
	if ( rest<9000 )
	{
		rest = 9000 ;
	}
 	*ptr = rest;
 	*(ptr+1) = 0;
}


void setupPulsesPPM2()
{
	register Pwm *pwmptr ;
	
	pwmptr = PWM ;
	// Now set up pulses
	int16_t PPM_range = g_model.extendedLimits ? 640*2 : 512*2;   //range of 0.7..1.7msec

  //Total frame length = 22.5msec
  //each pulse is 0.7..1.7ms long with a 0.3ms stop tail
  //The pulse ISR is 2mhz that's why everything is multiplied by 2
  uint16_t *ptr ;
  ptr = Pulses2 ;
  
	uint32_t p = g_model.startPPM2channel ;

	if ( p == 0 )
	{
//  	p = 8+g_model.ppmNCH*2 + g_model.startChannel ; //Channels *2
		if ( ( g_model.ppmNCH > 12 ) || (g_model.ppmNCH < -2) )
		{
			g_model.ppmNCH = 0 ;		// Correct if wrong from DSM
		}
		p = (g_model.ppmNCH + 4) * 2 ;
		if ( p > 16 )
		{
			p -= 13 ;
		}
  	p += g_model.startChannel ; //Channels *2
	}
	else
	{
		p -= 1 ;
	}
	uint32_t q = (g_model.ppm2NCH + 4) * 2 ;
	if ( q > 16 )
	{
		q -= 13 ;
	}
	q += p ;
	if ( q > NUM_SKYCHNOUT )
	{
		q = NUM_SKYCHNOUT ;	// Don't run off the end		
	}

	pwmptr->PWM_CH_NUM[1].PWM_CDTYUPD = (g_model.ppmDelay*50+300)*2; //Stoplen *2
	
	if (g_model.pulsePol == 0)
	{
		pwmptr->PWM_CH_NUM[1].PWM_CMR &= ~0x00000200 ;	// CPOL
	}
	else
	{
		pwmptr->PWM_CH_NUM[1].PWM_CMR |= 0x00000200 ;	// CPOL
	}
    
	uint16_t rest=22500u*2; //Minimum Framelen=22.5 ms
  rest += (int16_t(g_model.ppmFrameLength))*1000;
  //    if(p>9) rest=p*(1720u*2 + q) + 4000u*2; //for more than 9 channels, frame must be longer
  for( uint32_t i = p ; i < q ; i += 1 )
	{ //NUM_SKYCHNOUT
  	int16_t v = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
   	rest-=(v);
	//        *ptr++ = q;      //moved down two lines
    	    //        pulses2MHz[j++] = q;
    *ptr++ = v ; /* as Pat MacKenzie suggests */
    	    //        pulses2MHz[j++] = v - q + 600; /* as Pat MacKenzie suggests */
	//        *ptr++ = q;      //to here
 	}
	//    *ptr=q;       //reverse these two assignments
	//    *(ptr+1)=rest;
	if ( rest<9000 )
	{
		rest = 9000 ;
	}
 	*ptr = rest;
 	*(ptr+1) = 0;
	
}



const uint16_t CRCTable[]=
{
    0x0000,0x1189,0x2312,0x329b,0x4624,0x57ad,0x6536,0x74bf,
    0x8c48,0x9dc1,0xaf5a,0xbed3,0xca6c,0xdbe5,0xe97e,0xf8f7,
    0x1081,0x0108,0x3393,0x221a,0x56a5,0x472c,0x75b7,0x643e,
    0x9cc9,0x8d40,0xbfdb,0xae52,0xdaed,0xcb64,0xf9ff,0xe876,
    0x2102,0x308b,0x0210,0x1399,0x6726,0x76af,0x4434,0x55bd,
    0xad4a,0xbcc3,0x8e58,0x9fd1,0xeb6e,0xfae7,0xc87c,0xd9f5,
    0x3183,0x200a,0x1291,0x0318,0x77a7,0x662e,0x54b5,0x453c,
    0xbdcb,0xac42,0x9ed9,0x8f50,0xfbef,0xea66,0xd8fd,0xc974,
    0x4204,0x538d,0x6116,0x709f,0x0420,0x15a9,0x2732,0x36bb,
    0xce4c,0xdfc5,0xed5e,0xfcd7,0x8868,0x99e1,0xab7a,0xbaf3,
    0x5285,0x430c,0x7197,0x601e,0x14a1,0x0528,0x37b3,0x263a,
    0xdecd,0xcf44,0xfddf,0xec56,0x98e9,0x8960,0xbbfb,0xaa72,
    0x6306,0x728f,0x4014,0x519d,0x2522,0x34ab,0x0630,0x17b9,
    0xef4e,0xfec7,0xcc5c,0xddd5,0xa96a,0xb8e3,0x8a78,0x9bf1,
    0x7387,0x620e,0x5095,0x411c,0x35a3,0x242a,0x16b1,0x0738,
    0xffcf,0xee46,0xdcdd,0xcd54,0xb9eb,0xa862,0x9af9,0x8b70,
    0x8408,0x9581,0xa71a,0xb693,0xc22c,0xd3a5,0xe13e,0xf0b7,
    0x0840,0x19c9,0x2b52,0x3adb,0x4e64,0x5fed,0x6d76,0x7cff,
    0x9489,0x8500,0xb79b,0xa612,0xd2ad,0xc324,0xf1bf,0xe036,
    0x18c1,0x0948,0x3bd3,0x2a5a,0x5ee5,0x4f6c,0x7df7,0x6c7e,
    0xa50a,0xb483,0x8618,0x9791,0xe32e,0xf2a7,0xc03c,0xd1b5,
    0x2942,0x38cb,0x0a50,0x1bd9,0x6f66,0x7eef,0x4c74,0x5dfd,
    0xb58b,0xa402,0x9699,0x8710,0xf3af,0xe226,0xd0bd,0xc134,
    0x39c3,0x284a,0x1ad1,0x0b58,0x7fe7,0x6e6e,0x5cf5,0x4d7c,
    0xc60c,0xd785,0xe51e,0xf497,0x8028,0x91a1,0xa33a,0xb2b3,
    0x4a44,0x5bcd,0x6956,0x78df,0x0c60,0x1de9,0x2f72,0x3efb,
    0xd68d,0xc704,0xf59f,0xe416,0x90a9,0x8120,0xb3bb,0xa232,
    0x5ac5,0x4b4c,0x79d7,0x685e,0x1ce1,0x0d68,0x3ff3,0x2e7a,
    0xe70e,0xf687,0xc41c,0xd595,0xa12a,0xb0a3,0x8238,0x93b1,
    0x6b46,0x7acf,0x4854,0x59dd,0x2d62,0x3ceb,0x0e70,0x1ff9,
    0xf78f,0xe606,0xd49d,0xc514,0xb1ab,0xa022,0x92b9,0x8330,
    0x7bc7,0x6a4e,0x58d5,0x495c,0x3de3,0x2c6a,0x1ef1,0x0f78
};


void crc( uint8_t data )
{
    //	uint8_t i ;

  PcmCrc =(PcmCrc<<8) ^(CRCTable[((PcmCrc>>8)^data)&0xFF]);
}


// 8uS/bit 01 = 0, 001 = 1
void putPcmPart( uint8_t value )
{
	put_serial_bit( 0 ) ; 
	if ( value )
	{
		put_serial_bit( 0 ) ;
	}
	put_serial_bit( 1 ) ;
}


void putPcmFlush()
{
  while ( Serial_bit_count != 0 )
	{
		put_serial_bit( 1 ) ;		// Line idle level
  }
}

void putPcmBit( uint8_t bit )
{
    if ( bit )
    {
        PcmOnesCount += 1 ;
        putPcmPart( 1 ) ;
    }
    else
    {
        PcmOnesCount = 0 ;
        putPcmPart( 0 ) ;
    }
    if ( PcmOnesCount >= 5 )
    {
        putPcmBit( 0 ) ;				// Stuff a 0 bit in
    }
}

void putPcmByte( uint8_t byte )
{
    uint8_t i ;

    crc( byte ) ;

    for ( i = 0 ; i < 8 ; i += 1 )
    {
        putPcmBit( byte & 0x80 ) ;
        byte <<= 1 ;
    }
}

void putPcmHead()
{
    // send 7E, do not CRC
    // 01111110
    putPcmPart( 0 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 0 ) ;
}

uint16_t scaleForPXX( uint8_t i )
{
	int16_t value ;

//#ifdef DISABLE_PXX_SPORT
//#ifdef REVX
//  value = g_chans512[i] *3 / 4 + 2250 ;
//	return value ;
//#else 
	value = ( i < 24 ) ? g_chans512[i] *3 / 4 + 1024 : 0 ;
	return limit( (int16_t)1, value, (int16_t)2046 ) ;
//#endif
}


//void setUpPulsesPCM()
void setupPulsesPXX()
{
    uint8_t i ;
    uint16_t chan ;
    uint16_t chan_1 ;
		uint8_t lpass = pass ;

		Serial_byte = 0 ;
		Serial_bit_count = 0 ;
		Serial_byte_count = 0 ;
	  Pulses2MHzptr = Bit_pulses ;
    PcmCrc = 0 ;
    PcmOnesCount = 0 ;
    putPcmPart( 0 ) ;
    putPcmPart( 0 ) ;
    putPcmPart( 0 ) ;
    putPcmPart( 0 ) ;
    putPcmHead(  ) ;  // sync byte
    putPcmByte( g_model.pxxRxNum ) ;
    
//#ifdef DISABLE_PXX_SPORT
//#ifdef REVX
//		putPcmByte( pxxFlag ) ;     // First byte of flags
//#else
  	uint8_t flag1;
  	if (pxxFlag & PXX_BIND)
		{
  	  flag1 = (g_model.sub_protocol<< 6) | (g_model.country << 1) | pxxFlag ;
  	}
  	else
		{
  	  flag1 = (g_model.sub_protocol << 6) | pxxFlag ;
		}	
		 putPcmByte( flag1 ) ;     // First byte of flags
//#endif
    putPcmByte( 0 ) ;     // Second byte of flags
//#ifdef DISABLE_PXX_SPORT
//#ifdef REVX
//    pxxFlag = 0;          // reset flag after send
//#endif
		uint8_t startChan = g_model.startChannel ;
		if ( lpass & 1 )
		{
			startChan += 8 ;			
		}
    for ( i = 0 ; i < 4 ; i += 1 )		// First 8 channels only
    {																	// Next 8 channels would have 2048 added
      chan = scaleForPXX( startChan ) ;
			if ( lpass & 1 )
			{
				chan += 2048 ;
			}
      putPcmByte( chan ) ; // Low byte of channel
			startChan += 1 ;
      chan_1 = scaleForPXX( startChan ) ;
			if ( lpass & 1 )
			{
				chan_1 += 2048 ;
			}
			startChan += 1 ;
			putPcmByte( ( ( chan >> 8 ) & 0x0F ) | ( chan_1 << 4) ) ;  // 4 bits each from 2 channels
      putPcmByte( chan_1 >> 4 ) ;  // High byte of channel
    }
//#ifndef DISABLE_PXX_SPORT
//#ifndef REVX
		putPcmByte( 0 ) ;
//#endif
    chan = PcmCrc ;		        // get the crc
    putPcmByte( chan >> 8 ) ; // Checksum hi
    putPcmByte( chan ) ; 			// Checksum lo
    putPcmHead(  ) ;      // sync byte
    putPcmFlush() ;
		if (g_model.sub_protocol == 1 )		// D8
		{
			lpass = 0 ;
		}
		else
		{
			lpass += 1 ;
		}
		pass = lpass ;
}

