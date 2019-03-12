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

uint8_t Bit_pulses[80] ;			// To allow for Xfire telemetry
uint16_t XfireLength ;
uint8_t *Pulses2MHzptr ;

uint8_t Serial_byte ;
uint8_t Serial_bit_count;
uint8_t Serial_byte_count ;
uint8_t CurrentProtocol[2] ;
//uint8_t Current_protocol ;
//uint8_t Current_xprotocol ;
uint8_t BindRangeFlag[2] = { 0, 0 } ;
//uint8_t PxxExtra[2] = { 0, 0 } ;
uint16_t PcmCrc ;
uint8_t PcmOnesCount ;
uint8_t CurrentTrainerSource ;
extern uint8_t TrainerPolarity ;

uint16_t FailsafeCounter[2] ;

volatile uint8_t Dsm_Type = 0 ;
uint8_t DsmInitCounter = 0 ;
//uint8_t Dsm_Type_channels = 12 ;
//uint8_t Dsm_Type_10bit = 0 ;				// 0 for 11 bit, 1 for 10 bit
//uint8_t Dsm_mode_response = 0 ;

uint16_t Pulses[18] ;//= {	2000, 2200, 2400, 2600, 2800, 3000, 3200, 3400, 9000, 0, 0, 0,0,0,0,0,0, 0 } ;
uint16_t Pulses2[266] ;//= {	2000, 2200, 2400, 2600, 2800, 3000, 3200, 3400, 9000, 0, 0, 0,0,0,0,0,0, 0 } ;
volatile uint32_t PulsesIndex[2] = { 0, 0 } ;		// Modified in interrupt routine

extern void setCaptureMode(uint32_t mode) ;
extern void start_timer3() ;
extern uint8_t TrainerMode ;

uint8_t SerialExternalData[28] ;
uint8_t Multi2Data[28] ;

// DSM2 control bits
//#define FranceBit 0x10
//#define DsmxBit  0x08

static uint8_t Pass ;		// For PXX and DSM-9XR and ASSAN
static uint8_t bitlen ;
uint8_t PulsesPaused ;

void crc( uint8_t data ) ;
uint8_t setupPulsesXfire() ;


void pausePulses()
{
	PulsesPaused = 1 ;
//	module_output_low() ;
//	InternalOutputLow() ;
}

void resumePulses()
{
	PulsesPaused = 0 ;
	if ( g_model.Module[1].protocol != PROTO_OFF )
	{
		module_output_active() ;
	}
	if ( g_model.Module[0].protocol != PROTO_OFF )
	{
		InternalOutputActive() ;
	}
}

void module_output_low()
{
	configure_pins( PIO_PA17, PIN_ENABLE | PIN_OUTPUT | PIN_PORTA | PIN_ODRAIN | PIN_LOW ) ;
//	PIOA->PIO_CODR = PIO_PA17 ;		

}

void module_output_active()
{
	register Pio *pioptr ;
	
//	if ( ( CurrentTrainerSource != TRAINER_SLAVE ) || (g_model.Module[1].protocol != PROTO_PPM) )
	if ( CurrentTrainerSource != TRAINER_SLAVE )
	{
		pioptr = PIOA ;
//		pioptr->PIO_ABCDSR[0] &= ~PIO_PA17 ;		// Peripheral C
//	 	pioptr->PIO_ABCDSR[1] |= PIO_PA17 ;			// Peripheral C
		pioptr->PIO_PDR = PIO_PA17 ;						// Disable bit A17 Assign to peripheral
#ifdef REVX
		if ( ( g_model.Module[1].ppmOpenDrain ) || ( g_model.Module[1].protocol != PROTO_PPM ) )
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
}


void InternalOutputLow()
{
	configure_pins( PIO_PC15, PIN_ENABLE | PIN_OUTPUT | PIN_PORTC | PIN_ODRAIN | PIN_LOW ) ;
}

void InternalOutputActive()
{
	if ( CurrentTrainerSource != TRAINER_SLAVE )
	{
		configure_pins( PIO_PC15, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_B | PIN_PORTC | PIN_NO_PULLUP ) ;
	}
}


#ifdef PCBSKY
void checkTrainerSource()
{
//Jack BT   COM2 Slave
	uint32_t tSource = g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[0].source ;
	if ( g_model.traineron == 0 )
	{
		tSource = TRAINER_JACK ;
	}
	if ( check_soft_power() == POWER_TRAINER )		// On trainer power
	{
		tSource = TRAINER_SLAVE ;
	}

	if ( CurrentTrainerSource != tSource )
	{
		switch ( CurrentTrainerSource )
		{
			case 0 :
				end_ppm_capture() ;
			break ;
			case 1 :
			break ;
			case 2 :
			break ;
			case TRAINER_SLAVE :
			break ;
			case 4 :
				setCaptureMode( 0 ) ;			
			break ;
		}
		CurrentTrainerSource = tSource ;
		switch ( tSource )
		{
			case 0 :
				start_ppm_capture() ;
			break ;
			case 1 :
			break ;
			case 2 :
			break ;
			case TRAINER_SLAVE :	// Slave so output
				PIOC->PIO_PDR = PIO_PC22 ;						// Disable bit C22 Assign to peripheral
				module_output_low() ;
				InternalOutputLow() ;
			break ;
			case 4 :
			{	
				uint32_t tPolarity = g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[1].source ;
				TrainerPolarity = tPolarity ;
				TrainerMode = 1 ;
				configure_pins( (PIO_PA5 | PIO_PA6), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;
				start_timer3() ;
				setCaptureMode( 1 ) ;
			}
			break ;
		}
		if ( tSource != TRAINER_SLAVE )
		{
			PIOC->PIO_PER = PIO_PC22 ;						// Enable bit C22 as input
			if ( g_model.Module[1].protocol != PROTO_OFF )
			{
				if ( PulsesPaused == 0 )
				{
					module_output_active() ;
				}
			}
			if ( g_model.Module[0].protocol != PROTO_OFF )
			{
				if ( PulsesPaused == 0 )
				{
					InternalOutputActive() ;				
				}
			}
		}
	}
}
#endif


void init_ppm2( uint32_t period, uint32_t out_enable )
{
	register Pwm *pwmptr ;
	
// This is the PPM2 output
#ifndef REVA
	if ( g_model.Module[0].protocol != PROTO_OFF )
	{
		InternalOutputActive() ;				
	}
	else
	{
		InternalOutputLow() ;
	}
#endif

#ifndef REVA
	// PWM1 for PPM2
	pwmptr = PWM ;
	pwmptr->PWM_CH_NUM[1].PWM_CMR = 0x0000000B ;	// CLKB
	if (g_model.Module[0].pulsePol == 0)
	{
		pwmptr->PWM_CH_NUM[1].PWM_CMR |= 0x00000200 ;	// CPOL
	}
	pwmptr->PWM_CH_NUM[1].PWM_CPDR = period ;			// Period
	pwmptr->PWM_CH_NUM[1].PWM_CPDRUPD = period ;		// Period
	pwmptr->PWM_CH_NUM[1].PWM_CDTY = g_model.Module[0].ppmDelay*100+600 ;				// Duty
	pwmptr->PWM_CH_NUM[1].PWM_CDTYUPD = g_model.Module[0].ppmDelay*100+600 ;		// Duty
	pwmptr->PWM_ENA = PWM_ENA_CHID1 ;						// Enable channel 1
#endif

	pwmptr->PWM_IER1 = PWM_IER1_CHID1 ;
  NVIC_SetPriority(SW7_IRQn, 4 ) ;
	NVIC_EnableIRQ(SW7_IRQn) ;
  NVIC_SetPriority(PWM_IRQn, 2 ) ;
	NVIC_EnableIRQ(PWM_IRQn) ;
	
}


void init_main_ppm( uint32_t period, uint32_t out_enable )
{
	register Pwm *pwmptr ;
	
  perOut(g_chans512, 0) ;
  setupPulsesPPM() ;

	if ( out_enable )
	{
		if ( PulsesPaused == 0 )
		{
			module_output_active() ;
		}
	}
	PIOA->PIO_ABCDSR[0] &= ~PIO_PA17 ;		// Peripheral C
 	PIOA->PIO_ABCDSR[1] |= PIO_PA17 ;			// Peripheral C

	pwmptr = PWM ;
	// PWM3 for PPM output	 
	pwmptr->PWM_CH_NUM[3].PWM_CMR = 0x0004000B ;	// CLKA
	if (g_model.Module[1].pulsePol == 0)
	{
		pwmptr->PWM_CH_NUM[3].PWM_CMR |= 0x00000200 ;	// CPOL
	}
	pwmptr->PWM_CH_NUM[3].PWM_CPDR = period ;			// Period in half uS
	pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = period ;	// Period in half uS
	pwmptr->PWM_CH_NUM[3].PWM_CDTY = g_model.Module[1].ppmDelay*100+600 ;			// Duty in half uS
	pwmptr->PWM_CH_NUM[3].PWM_CDTYUPD = g_model.Module[1].ppmDelay*100+600 ;		// Duty in half uS
	pwmptr->PWM_ENA = PWM_ENA_CHID3 ;						// Enable channel 3

	pwmptr->PWM_IER1 = PWM_IER1_CHID3 ;


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

extern "C" void SW7_IRQHandler( void )
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0xA2 ;
#else
	RTC->BKP1R = 0xA2 ;
#endif
#endif
	setupPulses() ;
}

uint16_t XjtHbeatOffset ;

#ifndef SIMU
extern "C" void PWM_IRQHandler (void)
{
	register Pwm *pwmptr ;
	register Ssc *sscptr ;
	uint32_t period ;
	uint32_t reason ;

	pwmptr = PWM ;
	reason = pwmptr->PWM_ISR1 ;
	if ( reason & PWM_ISR1_CHID1 )	// PPM2
	{
		if ( g_model.Module[0].protocol == 0 )
		{
			pwmptr->PWM_CH_NUM[1].PWM_CPDRUPD = Pulses2[PulsesIndex[0]++] ;	// Period in half uS
			if ( Pulses2[PulsesIndex[0]] == 0 )
			{
				PulsesIndex[0] = 0 ;
				setupPulsesPPM2() ;
			  if ( CurrentProtocol[0] != g_model.Module[0].protocol )
				{
					if ( g_model.Module[0].protocol == PROTO_OFF )
					{
						InternalOutputLow() ;
						PIOC->PIO_CODR = PIO_PC15 ;
					}
					else
					{
						if ( PulsesPaused == 0 )
						{
							InternalOutputActive() ;
						}
					}
			  	CurrentProtocol[0] = g_model.Module[0].protocol ;
				}
			}
		}
		else // g_model.xprotocol = 1
		{
			pwmptr->PWM_CH_NUM[1].PWM_CPDRUPD = Pulses2[PulsesIndex[0]++] ;	// Period in half uS
			pwmptr->PWM_CH_NUM[1].PWM_CDTYUPD = Pulses2[PulsesIndex[0]++] ;
			if ( Pulses2[PulsesIndex[0]] == 0 )
			{
				PulsesIndex[0] = 0 ;
				setupPulsesPPM2() ;
			  if ( CurrentProtocol[0] != g_model.Module[0].protocol )
				{
					if ( g_model.Module[0].protocol == PROTO_OFF )
					{
						InternalOutputLow() ;
						PIOC->PIO_CODR = PIO_PC15 ;
					}
					else
					{
						if ( PulsesPaused == 0 )
						{
							InternalOutputActive() ;
						}
					}
			  	CurrentProtocol[0] = g_model.Module[0].protocol ;
				}
			}
		}
	}
	if ( reason & PWM_ISR1_CHID3 )
	{
		switch ( CurrentProtocol[1] )		// Use the current, don't switch until set_up_pulses
		{
      case PROTO_OFF:
      case PROTO_PPM:
				pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = Pulses[PulsesIndex[1]++] ;	// Period in half uS
				if ( Pulses[PulsesIndex[1]] == 0 )
				{
					PulsesIndex[1] = 0 ;

					NVIC->STIR = SW7_IRQn ;
//					setupPulses() ;
				}
			break ;

      case PROTO_PXX:
				// Alternate periods of 8.0/3.5mS and 1.0 mS
				period = pwmptr->PWM_CH_NUM[3].PWM_CPDR ;
				if ( period == 2000 )	// 1.0 mS
				{
					XjtHbeatOffset = TC1->TC_CHANNEL[0].TC_CV - XjtHeartbeatCapture.value ;
					if ( ( g_model.Module[1].pxxDoubleRate ) && ( (BindRangeFlag[1] & PXX_BIND) == 0 ) )
					{
						period = 3500*2 ;	// Use 4.5mS period total
						if ( (Pass & 1) == 0 )
						{
							if ( XjtHeartbeatCapture.valid )
							{
								if ( XjtHbeatOffset > 0x2500 )
								{
									period -= 20 ;                     // 9mS
								}
								else
								{
									period += 20 ;                     // 9mS
								}
							}
						}
					}
					else
					{
						period = 8000*2 ;
						if ( XjtHeartbeatCapture.valid )
						{
							if ( XjtHbeatOffset > 0x2500 )
							{
								period -= 20 ;                     // 9mS
							}
							else
							{
								period += 20 ;                     // 9mS
							}
						}
					}	 
				}
				else
				{
					period = 2000 ;	// 2.5 mS
				}
				pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = period ;	// Period in half uS
				if ( period != 2000 )	// 2.5 mS
				{
					NVIC->STIR = SW7_IRQn ;
//					setupPulses() ;
				}
				else
				{
					// Kick off serial output here
					if ( PulsesPaused == 0 )
					{
						PIOA->PIO_PDR = PIO_PA17 ;	// Assign A17 to Peripheral
					}
					sscptr = SSC ;
					sscptr->SSC_TPR = (uint32_t) Bit_pulses ;
					sscptr->SSC_TCR = Serial_byte_count ;
					sscptr->SSC_PTCR = SSC_PTCR_TXTEN ;	// Start transfers
				}
			break ;

      case PROTO_MULTI:
      case PROTO_DSM2:
#ifdef XFIRE
      case PROTO_XFIRE:
#endif
				// Alternate periods of 19.5mS/8.5mS and 2.5 mS
				period = pwmptr->PWM_CH_NUM[3].PWM_CPDR ;
				if ( period == 5000 )	// 2.5 mS
				{
					if ( CurrentProtocol[1] == PROTO_MULTI )
					{
						uint32_t x ;
						x = g_model.Module[1].ppmFrameLength ;
						if ( x > 4 )
						{
							x = 0 ;
						}
						x *= 2000 ;
						x += 4500 * 2 ;
						period = x ;
					}
					else if ( ( CurrentProtocol[1] == PROTO_DSM2 ) && ( g_model.Module[1].sub_protocol != DSM_9XR  ) )
					{
						period = 19500*2 ;		// Total 22 mS
					}
					else if ( CurrentProtocol[1] == PROTO_XFIRE )
					{
						period = 1500*2 ;		// Total 4 mS
					}
					else
					{
						period = 8500*2 ;		// Total 11 mS
					}	 
				}
				else
				{
					period = 5000 ;
				}
				pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = period ;	// Period in half uS
				if ( period != 5000 )	// 2.5 mS
				{
					NVIC->STIR = SW7_IRQn ;
//					setupPulses() ;
				}
				else
				{
					// Kick off serial output here
 #ifdef XFIRE
					if ( CurrentProtocol[1] == PROTO_XFIRE )
					{
						txPdcUsart( Bit_pulses, XfireLength, 0 ) ;
					}
					else
					{
#endif
						if ( PulsesPaused == 0 )
						{
							PIOA->PIO_PDR = PIO_PA17 ;	// Assign A17 to Peripheral
						}
						sscptr = SSC ;
						sscptr->SSC_TPR = (uint32_t) Bit_pulses ;
						sscptr->SSC_TCR = Serial_byte_count ;
						sscptr->SSC_PTCR = SSC_PTCR_TXTEN ;	// Start transfers
 #ifdef XFIRE
					}
#endif
				}
			break ;
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
    *Pulses2MHzptr++ = Serial_byte ;
		Serial_bit_count = 0 ;
		Serial_byte_count += 1 ;
	}
}

//uint8_t DsmPass1[10] ;
//uint32_t DsmPassIndex ;

#define BITLEN_SERIAL (8*2) //125000 Baud
#define BITLEN_SBUS (10*2) //100000 Baud

void sendByteDsm2(uint8_t b) //max 10changes 0 10 10 10 10 1
{
	uint32_t parity = 0 ;
	
//	if ( DsmPassIndex < 10 )
//	{
//		DsmPass1[DsmPassIndex++] = b ;
//	}
	put_serial_bit( 0 ) ;		// Start bit
  for( uint8_t i=0; i<8; i++)	 // 8 data Bits
	{
		put_serial_bit( b & 1 ) ;
		parity ^= b ;
		b >>= 1 ;
  }
	if ( g_model.Module[1].protocol != PROTO_DSM2 )
	{
		put_serial_bit( parity & 1 ) ;
	}
	
	put_serial_bit( 1 ) ;		// Stop bit
	if ( Dsm_Type == 0 )
	{
		put_serial_bit( 1 ) ;		// Stop bit
	}
}

void setupPulsesDsm2(uint8_t chns)
{
  static uint8_t dsmDat[2+6*2+4]={0xFF,0x00,  0x00,0xAA,  0x05,0xFF,  0x09,0xFF,  0x0D,0xFF,  0x13,0x54,  0x14,0xAA } ;
  uint16_t required_baudrate ;
#ifdef PCBSKY
	uint8_t counter ;
#endif // PCBSKY


	required_baudrate = SCC_BAUD_125000 ;
	if( (g_model.Module[1].protocol == PROTO_DSM2) && ( g_model.Module[1].sub_protocol == DSM_9XR ) )
	{
		required_baudrate = SCC_BAUD_115200 ;
		Dsm_Type = 1 ;
		// Consider inverting COM1 here
	}
	else if(g_model.Module[1].protocol == PROTO_MULTI)
	{
		required_baudrate = SCC_BAUD_100000 ;
		Dsm_Type = 0 ;
	}
	else
	{
		Dsm_Type = 0 ;
	}
	
#ifdef PCBSKY
	if ( required_baudrate != Scc_baudrate )
	{
		init_ssc( required_baudrate ) ;
		SSC->SSC_TFMR |= 0x00000020 ;
	}
#endif // PCBSKY

	Serial_byte = 0 ;
	Serial_bit_count = 0 ;
	Serial_byte_count = 0 ;
  Pulses2MHzptr = Bit_pulses ;
	bitlen = BITLEN_SERIAL ;
    
	if ( Dsm_Type )
	{
		uint8_t channels = 12 ;
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
			flags &= 0x7F ;
			channels = g_model.Module[1].channels ;
		}
			 
//#ifdef PCB9XT
		if ( (dsmDat[0]&BindBit) && (!keyState(SW_Trainer) ) )
//#else
//		if ( (dsmDat[0]&BindBit) && (!keyState(SW_SH2) ) )
//#endif
		{
			dsmDat[0] &= ~BindBit	;
		}

		uint8_t startChan = g_model.Module[1].startChannel ;
		sendByteDsm2( 0xAA );
		if ( Pass == 0 )
		{
  		sendByteDsm2( Pass ) ;		// Actually is a 0
			// Do init packet
			if ( (BindRangeFlag[1] & PXX_BIND) || (dsmDat[0]&BindBit) )
			{
				flags |= ORTX_BIND_FLAG ;
			}
			// Need to choose dsmx/dsm2 as well
  		sendByteDsm2( flags ) ;
  		sendByteDsm2( (BindRangeFlag[1] & PXX_RANGE_CHECK) ? 4: 7 ) ;		// 
  		sendByteDsm2( channels ) ;			// Max channels
//  		sendByteDsm2( g_model.Module[1].pxxRxNum ) ;		// Rx Num
#ifdef ENABLE_DSM_MATCH  		
			sendByteDsm2( g_model.Module[1].pxxRxNum ) ;	// Model number
#else
  		sendByteDsm2( 1 ) ;		// 'Model Match' disabled
#endif
			Pass = 1 ;
		}
		else
		{
//			DsmPassIndex = 0 ;
			if ( Pass == 2 )
			{
				startChan += 7 ;
			}
  		sendByteDsm2( Pass );

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
			if ( ++Pass > 2 )
			{
				Pass = 1 ;				
			}
			if ( channels < 8 )
			{
				Pass = 1 ;				
			}
			DsmInitCounter += 1 ;
			if( DsmInitCounter > 100)
			{
				DsmInitCounter = 0 ;
				Pass = 0 ;
			}
			if ( (BindRangeFlag[1] & PXX_BIND) || (dsmDat[0]&BindBit) )
			{
				Pass = 0 ;		// Stay here
			}
		}
	}
	else
	{
  	// If more channels needed make sure the pulses union/array is large enough
		if(g_model.Module[1].protocol == PROTO_DSM2)
		{
			setDsmHeader( dsmDat, 1 ) ;
		}

		if(g_model.Module[1].protocol == PROTO_MULTI)
		{
			setMultiSerialArray( SerialExternalData, 1 ) ;
			uint32_t i ;
			for ( i = 0 ; i < 26 ; i += 1 )
			{
				sendByteDsm2( SerialExternalData[i] ) ;
			}

			if ( g_model.Module[1].protocol == PROTO_SBUS )
			{
				sendByteDsm2(0);
				sendByteDsm2(0);
			}
		}
		else// not MULTI
		{
  		dsmDat[1]=g_model.Module[1].pxxRxNum ;  //DSM2 Header second byte for model match
  		for(uint8_t i=0; i<chns; i++)
  		{
				uint16_t pulse = limit(0, ((g_chans512[g_model.Module[1].startChannel+i]*13)>>5)+512,1023);
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
	CurrentProtocol[1] = g_model.Module[1].protocol + 1 ;		// Not the same!
	setupPulses() ;	// For DSM-9XR this won't be sent
	Pass = 0 ;			// Force a type 0 packet
}

void setupPulses()
{
	uint32_t requiredProtocol = g_model.Module[1].protocol ;
	if ( CurrentTrainerSource == TRAINER_SLAVE )
	{
		requiredProtocol = PROTO_PPM ;
	}
  heartbeat |= HEART_TIMER_PULSES ;
	
  if ( CurrentProtocol[1] != requiredProtocol )
  {
    switch( CurrentProtocol[1] )
    {	// stop existing protocol hardware
  	  case PROTO_OFF:
				disable_main_ppm() ;
				if ( PulsesPaused == 0 )
				{
					module_output_active() ;
				}
			break ;
      case PROTO_PPM:
				disable_main_ppm() ;
      break;
      case PROTO_PXX:
				if ( XjtHeartbeatCapture.valid )
				{
					stop_pb14_heartbeat() ;					
				}
				disable_ssc() ;
      break;
      case PROTO_DSM2:
	    case PROTO_MULTI:
				disable_ssc() ;
      break;
#ifdef XFIRE
      case PROTO_XFIRE :
      break;
#endif
    }
		
    CurrentProtocol[1] = requiredProtocol ;
    switch(CurrentProtocol[1])
    { // Start new protocol hardware here
      case PROTO_PPM:
				init_main_ppm( 3000, (CurrentTrainerSource == TRAINER_SLAVE) ? 0 : 1 ) ;		// Initial period 1.5 mS, output on
      break;
      case PROTO_PXX:
				init_main_ppm( 5000, 0 ) ;		// Initial period 2.5 mS, output off
				init_ssc(SCC_BAUD_125000) ;
				PIOA->PIO_MDDR = PIO_PA17 ;						// Push Pull O/p in A17
				SSC->SSC_TFMR &= ~0x00000020 ;
      break;
  	  case PROTO_OFF:
				init_main_ppm( 3000, 0 ) ;		// Initial period 1.5 mS, output on
				module_output_low() ;
      break;
    	case PROTO_MULTI:
      case PROTO_DSM2:
				init_main_ppm( 5000, 0 ) ;		// Initial period 2.5 mS, output off
				init_ssc(SCC_BAUD_125000) ;
				SSC->SSC_TFMR |= 0x00000020 ;
				SSC->SSC_THR = 0xFF ;		// Make the output High
				PIOA->PIO_MDDR = PIO_PA17 ;						// Push Pull O/p in A17
				DsmInitCounter = 0 ;
//				Dsm_Type_channels = 12 ;
//				Dsm_Type_10bit = 0 ;				// 0 for 11 bit, 1 for 10 bit
//				Dsm_mode_response = 0 ;
				Pass = 0 ;
      break;
#ifdef XFIRE
      case PROTO_XFIRE :
				module_output_low() ;
      break;
#endif
    }
  }

// Set up output data here
	switch(CurrentProtocol[1])
  {
  	case PROTO_OFF:
      setupPulsesPPM();		// Don't enable interrupts through here
    break;
	  case PROTO_PPM:
      setupPulsesPPM();		// Don't enable interrupts through here
    break;
  	case PROTO_PXX:
//      sei() ;							// Interrupts allowed here
			if ( g_model.Module[1].pxxHeartbeatPB14 )
			{
				if ( XjtHeartbeatCapture.valid == 0 )
				{
					init_pb14_heartbeat() ;					
				}
			}
			else
			{
				if ( XjtHeartbeatCapture.valid )
				{
					stop_pb14_heartbeat() ;					
				}
			}
			setupPulsesPXX();
    break;
	  case PROTO_DSM2:
    case PROTO_MULTI:
//      sei() ;							// Interrupts allowed here
      setupPulsesDsm2( ( g_model.Module[1].sub_protocol == DSM_9XR ) ? 12 : 6 ) ; 
    break;
#ifdef XFIRE
		case PROTO_XFIRE :
			setupPulsesXfire() ;
		break ;
#endif
  }
}

#define PPM_CENTER 1500*2

void setPpmPulses( uint16_t *dest, uint32_t start, uint32_t end, uint32_t total )
{
  uint32_t i ;
	int16_t PPM_range = g_model.extendedLimits ? 640*2 : 512*2;   //range of 0.7..1.7msec

	for(i= start ; i < end ; i += 1 )
	{
  	int16_t v = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
   	total -= v ;
    *dest++ = v ; /* as Pat MacKenzie suggests */
 	}
	if ( total<9000 )
	{
		total = 9000 ;
	}
 	*dest++ = total ;
 	*dest = 0;
}



void setupPulsesPPM()			// Don't enable interrupts through here
{
  uint32_t i ;
	int32_t total ;
  uint16_t *ptr ;

	i = 0 ;
	if ( CurrentTrainerSource == TRAINER_SLAVE )
	{
		i = 1 ; 
	}

	uint32_t p = i ? g_model.ppmNCH + 4 : g_model.Module[1].channels + 8 ;
  p += i ? g_model.startChannel : g_model.Module[1].startChannel ; //Channels *2
	if ( p > NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
	{
		p = NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ;	// Don't run off the end		
	}
	
	// Now set up pulses

  //Total frame length = 22.5msec
  //each pulse is 0.7..1.7ms long with a 0.3ms stop tail
  //The pulse ISR is 2mhz that's why everything is multiplied by 2
#ifdef PCBSKY
  ptr = Pulses ;
#endif
#if defined(PCBX9D) || defined(PCB9XT)
  ptr = ppmStream[0] ;
#endif
    
#ifdef PCBSKY
	register Pwm *pwmptr ;
	
	pwmptr = PWM ;
	pwmptr->PWM_CH_NUM[3].PWM_CDTYUPD = ((i ? g_model.ppmDelay : g_model.Module[1].ppmDelay)*50+300)*2; //Stoplen *2

	uint32_t pol = i ? g_model.trainPulsePol : g_model.Module[1].pulsePol ;
	 
	if (pol == 0)
	{
		pwmptr->PWM_CH_NUM[3].PWM_CMR |= 0x00000200 ;	// CPOL
	}
	else
	{
		pwmptr->PWM_CH_NUM[3].PWM_CMR &= ~0x00000200 ;	// CPOL
	}
#endif
    
	total = 22500u*2; //Minimum Framelen=22.5 ms
  total += (int16_t(i ? g_model.ppmFrameLength : g_model.Module[1].ppmFrameLength))*1000;

	setPpmPulses( ptr, i ? g_model.startChannel : g_model.Module[1].startChannel, p, total ) ;
  
//	for(i= i ? g_model.startChannel : g_model.Module[1].startChannel ; i<p ; i++ )
//	{ //NUM_SKYCHNOUT+EXTRA_SKYCHANNELS
//  	int16_t v = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
//   	total -= v ;
//    *ptr++ = v ; /* as Pat MacKenzie suggests */
// 	}
//	if ( total<9000 )
//	{
//		total = 9000 ;
//	}
// 	*ptr++ = total ;
// 	*ptr = 0;
#if defined(PCBX9D) || defined(PCB9XT)
	TIM1->CCR2 = total - 1000 ;		// Update time
	TIM1->CCR3 = (g_model.Module[1].ppmDelay*50+300)*2 ;
  if(!g_model.Module[1].pulsePol)
#ifdef PCB9XT
	  TIM1->CCER &= ~TIM_CCER_CC3P ;
#else
    TIM1->CCER |= TIM_CCER_CC3P;
#endif
	else
	{
#ifdef PCB9XT
    TIM1->CCER |= TIM_CCER_CC3P;
#else
	  TIM1->CCER &= ~TIM_CCER_CC3P ;
#endif
	}
#endif
}

void setupPulsesPPM2()
{
  static uint8_t dsmDat[2+6*2+4]={0xFF,0x00,  0x00,0xAA,  0x05,0xFF,  0x09,0xFF,  0x0D,0xFF,  0x13,0x54,  0x14,0xAA } ;
	register Pwm *pwmptr ;
	uint8_t *dataPtr ;
	uint32_t byteCount ;
	uint32_t bitTime ;
	uint16_t rest ;

	pwmptr = PWM ;
	// Now set up pulses

  //Total frame length = 22.5msec
  //each pulse is 0.7..1.7ms long with a 0.3ms stop tail
  //The pulse ISR is 2mhz that's why everything is multiplied by 2
	if ( g_model.Module[0].protocol == PROTO_PPM )
	{
  	uint16_t *ptr ;
  	ptr = Pulses2 ;
 
		uint32_t p = g_model.Module[0].startChannel ;
//		if ( p == 0 )
//		{
//	//  	p = 8+g_model.ppmNCH*2 + g_model.startChannel ; //Channels *2
//			p = (g_model.Module[0].channels + 4) ;
//			if ( ( g_model.Module[0].channels > 12 ) || (g_model.Module[0].channels < -4) )
//			{
//				p = 8 ;		// Use a default if wrong from DSM
//			}
//		}
//		else
//		{
//			p -= 1 ;
//		}
		uint32_t q = (g_model.Module[0].channels + 8) ;
		if ( q > NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
		{
			q = NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ;	// Don't run off the end		
		}

		pwmptr->PWM_CH_NUM[1].PWM_CDTYUPD = (g_model.Module[0].ppmDelay*50+300)*2; //Stoplen *2
	
		if (g_model.Module[0].pulsePol == 0)
		{
			pwmptr->PWM_CH_NUM[1].PWM_CMR &= ~0x00000200 ;	// CPOL
		}
		else
		{
			pwmptr->PWM_CH_NUM[1].PWM_CMR |= 0x00000200 ;	// CPOL
		}
    
		int32_t rest=22500u*2; //Minimum Framelen=22.5 ms
  	rest += (int16_t(g_model.Module[0].ppmFrameLength))*1000;
  	//    if(p>9) rest=p*(1720u*2 + q) + 4000u*2; //for more than 9 channels, frame must be longer
  	
		setPpmPulses( ptr, p, q, rest ) ;
//		for( uint32_t i = p ; i < q ; i += 1 )
//		{
//  		int16_t v = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
//  	 	rest-=(v);
//		//        *ptr++ = q;      //moved down two lines
//  	  	    //        pulses2MHz[j++] = q;
//  	  *ptr++ = v ; /* as Pat MacKenzie suggests */
//  	  	    //        pulses2MHz[j++] = v - q + 600; /* as Pat MacKenzie suggests */
//		//        *ptr++ = q;      //to here
// 		}
//		//    *ptr=q;       //reverse these two assignments
//		//    *(ptr+1)=rest;
//		if ( rest<9000 )
//		{
//			rest = 9000 ;
//		}
// 		*ptr = rest;
// 		*(ptr+1) = 0;
	}
	else	// if ( g_model.xprotocol == PROTO_DSM2 )
	{
  	uint16_t *ptr ;
  	ptr = Pulses2 ;
 
		uint32_t p = g_model.Module[0].startChannel ;
//		if ( p == 0 )
//		{
//	//  	p = 8+g_model.ppmNCH*2 + g_model.startChannel ; //Channels *2
//			p = (g_model.ppmNCH + 4) * 2 ;
//			if ( ( g_model.ppmNCH > 12 ) || (g_model.ppmNCH < -2) )
//			{
//				p = 8 ;		// Use a default if wrong from DSM
//			}
//			if ( p > 16 )
//			{
//				p -= 13 ;
//			}
//  		p += g_model.startChannel ; //Channels *2
//			if ( p > 16 )
//			{
//				p = 16 ;
//			}
//		}
//		else
//		{
//			p -= 1 ;
//		}
		uint32_t q = (g_model.Module[0].channels + 16) ;
		if ( q > NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
		{
			q = NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ;	// Don't run off the end		
		}

		pwmptr->PWM_CH_NUM[1].PWM_CMR |= 0x00000200 ;	// CPOL

		if(g_model.Module[0].protocol == PROTO_DSM2 )
    {
			setDsmHeader( dsmDat, 0 ) ;
  		dsmDat[1]=g_model.Module[0].pxxRxNum ;  //DSM2 Header second byte for model match
			if ( q > p + 6 )
			{
				q = p + 6 ;
			}
  		for(uint8_t i=p; i < q ; i++)
  		{
//				uint16_t pulse = limit(0, ((g_chans512[/*q+*/i]*13)>>5)+512,1023);
				uint16_t pulse = limit(0, ((g_chans512[i]*13)>>5)+512,1023);
 			  dsmDat[2+2*i] = (i<<2) | ((pulse>>8)&0x03);
  		 	dsmDat[3+2*i] = pulse & 0xff;
  		}

			dataPtr = dsmDat ;
			rest = 22000u*2 ;
			byteCount = 14 ;
			bitTime = 8 * 2 ;

		}
		else
		{
			if ( g_model.Module[0].protocol == PROTO_MULTI )
			{
				setMultiSerialArray( Multi2Data, 0 ) ;
				 
				dataPtr = Multi2Data ;
				uint32_t x ;
				x = g_model.Module[0].ppmFrameLength ;
				if ( x > 4 )
				{
					x = 0 ;
				}
				x *= 2000 ;
				x += 7000 * 2 ;
				rest = x ;
				byteCount = 26 ;
				bitTime = 10 * 2 ;
			}
			else
			{
				// Must be PROTO_OFF
				Multi2Data[0] = 0 ;
				dataPtr = Multi2Data ;
				rest = 40000 ;		// 20mS
				byteCount = 1 ;
				bitTime = 10 * 2 ;
			}
		}
		
		uint32_t counter ;
  	for ( counter = 0 ; counter < byteCount ; counter += 1 )
  	{
			uint32_t byte = dataPtr[counter] | 0x0600 ;	// Stop bits
			uint32_t index ;
			uint32_t countLow = 1 ;
			uint32_t countHigh = 0 ;
			uint32_t parity = 0 ;
			for ( index = 0 ; index < 12 ; index += 1 )
			{
				if ( index == 8 )
				{
					byte |= parity & 1 ;
				}
				else
				{
					parity ^= byte ;
				}
					
				if ( byte & 1 )
				{
					countHigh += 1 ;
				}
				else
				{
					if ( countHigh )
					{
						uint32_t t = (countLow+countHigh) * bitTime ;
						rest -= t ;
				  	*ptr++ = t ;
				  	*ptr++ = countLow * bitTime ;
						countLow = 0 ;
						countHigh = 0 ;
					}
					countLow += 1 ;
				}
				byte >>= 1 ;
			}
  	}
		*ptr++ = rest-3000 ;
  	*ptr++ = 0 ;
  	*ptr++ = 3000 ;
  	*ptr++ = 0 ;
  	*ptr = 0 ;
	}
}




/* CRC16 implementation according to CCITT standards */
//static const unsigned short crc16tab[256]= {
//  0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
//  0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
//  0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
//  0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
//  0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
//  0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
//  0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
//  0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
//  0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
//  0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
//  0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
//  0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
//  0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
//  0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
//  0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
//  0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
//  0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
//  0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
//  0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
//  0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
//  0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
//  0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
//  0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
//  0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
//  0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
//  0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
//  0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
//  0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
//  0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
//  0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
//  0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
//  0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
//};

//uint16_t crc16(uint8_t * buf, uint32_t len)
//{
//  uint16_t crc = 0;
//  for (uint32_t counter=0; counter<len; counter++) {
//    crc = (crc<<8) ^ crc16tab[ ((crc>>8) ^ *buf++ ) & 0x00FF];
//  }
//  return crc;
//}

//#define CROSSFIRE_START_BYTE        0x0F
//#define CROSSFIRE_CH_CENTER         0x3E0
//#define CROSSFIRE_CH_BITS           11

//// Range for pulses (channels output) is [-1024:+1024]
//void createCrossfireFrame(uint8_t * frame, int16_t * pulses)
//{
//  uint8_t * buf = frame;
//  *buf++ = CROSSFIRE_START_BYTE;

//  uint32_t bits = 0;
//  uint8_t bitsavailable = 0;
//  for (int i=0; i<CROSSFIRE_CHANNELS_COUNT; i++)
//	{
//    bits |= (CROSSFIRE_CH_CENTER + (((pulses[i]) * 4) / 5)) << bitsavailable ;
//    bitsavailable += CROSSFIRE_CH_BITS;
//    while (bitsavailable >= 8)
//		{
//      *buf++ = bits;
//      bits >>= 8;
//      bitsavailable -= 8;
//    }
//  }
//  *buf++ = 0;
//  *buf++ = crc16(frame, 24);
//}



//const uint16_t CRCTable[]=
//{
//    0x0000,0x1189,0x2312,0x329b,0x4624,0x57ad,0x6536,0x74bf,
//    0x8c48,0x9dc1,0xaf5a,0xbed3,0xca6c,0xdbe5,0xe97e,0xf8f7,
//    0x1081,0x0108,0x3393,0x221a,0x56a5,0x472c,0x75b7,0x643e,
//    0x9cc9,0x8d40,0xbfdb,0xae52,0xdaed,0xcb64,0xf9ff,0xe876,
//    0x2102,0x308b,0x0210,0x1399,0x6726,0x76af,0x4434,0x55bd,
//    0xad4a,0xbcc3,0x8e58,0x9fd1,0xeb6e,0xfae7,0xc87c,0xd9f5,
//    0x3183,0x200a,0x1291,0x0318,0x77a7,0x662e,0x54b5,0x453c,
//    0xbdcb,0xac42,0x9ed9,0x8f50,0xfbef,0xea66,0xd8fd,0xc974,
//    0x4204,0x538d,0x6116,0x709f,0x0420,0x15a9,0x2732,0x36bb,
//    0xce4c,0xdfc5,0xed5e,0xfcd7,0x8868,0x99e1,0xab7a,0xbaf3,
//    0x5285,0x430c,0x7197,0x601e,0x14a1,0x0528,0x37b3,0x263a,
//    0xdecd,0xcf44,0xfddf,0xec56,0x98e9,0x8960,0xbbfb,0xaa72,
//    0x6306,0x728f,0x4014,0x519d,0x2522,0x34ab,0x0630,0x17b9,
//    0xef4e,0xfec7,0xcc5c,0xddd5,0xa96a,0xb8e3,0x8a78,0x9bf1,
//    0x7387,0x620e,0x5095,0x411c,0x35a3,0x242a,0x16b1,0x0738,
//    0xffcf,0xee46,0xdcdd,0xcd54,0xb9eb,0xa862,0x9af9,0x8b70,
//    0x8408,0x9581,0xa71a,0xb693,0xc22c,0xd3a5,0xe13e,0xf0b7,
//    0x0840,0x19c9,0x2b52,0x3adb,0x4e64,0x5fed,0x6d76,0x7cff,
//    0x9489,0x8500,0xb79b,0xa612,0xd2ad,0xc324,0xf1bf,0xe036,
//    0x18c1,0x0948,0x3bd3,0x2a5a,0x5ee5,0x4f6c,0x7df7,0x6c7e,
//    0xa50a,0xb483,0x8618,0x9791,0xe32e,0xf2a7,0xc03c,0xd1b5,
//    0x2942,0x38cb,0x0a50,0x1bd9,0x6f66,0x7eef,0x4c74,0x5dfd,
//    0xb58b,0xa402,0x9699,0x8710,0xf3af,0xe226,0xd0bd,0xc134,
//    0x39c3,0x284a,0x1ad1,0x0b58,0x7fe7,0x6e6e,0x5cf5,0x4d7c,
//    0xc60c,0xd785,0xe51e,0xf497,0x8028,0x91a1,0xa33a,0xb2b3,
//    0x4a44,0x5bcd,0x6956,0x78df,0x0c60,0x1de9,0x2f72,0x3efb,
//    0xd68d,0xc704,0xf59f,0xe416,0x90a9,0x8120,0xb3bb,0xa232,
//    0x5ac5,0x4b4c,0x79d7,0x685e,0x1ce1,0x0d68,0x3ff3,0x2e7a,
//    0xe70e,0xf687,0xc41c,0xd595,0xa12a,0xb0a3,0x8238,0x93b1,
//    0x6b46,0x7acf,0x4854,0x59dd,0x2d62,0x3ceb,0x0e70,0x1ff9,
//    0xf78f,0xe606,0xd49d,0xc514,0xb1ab,0xa022,0x92b9,0x8330,
//    0x7bc7,0x6a4e,0x58d5,0x495c,0x3de3,0x2c6a,0x1ef1,0x0f78
//};


//void crc( uint8_t data )
//{
//    //	uint8_t i ;

//  PcmCrc =(PcmCrc<<8) ^(CRCTable[((PcmCrc>>8)^data)&0xFF]);
//}

void crc( uint8_t data )
{
  PcmCrc=(PcmCrc<<8) ^ CRCTable((PcmCrc>>8)^data) ;
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
		put_serial_bit( 0 ) ;		// Line idle level
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

//uint16_t scaleForPXX( uint8_t i )
//{
//	int16_t value ;

//	value = ( i < 24 ) ? g_chans512[i] *3 / 4 + 1024 : 0 ;
//	return limit( (int16_t)1, value, (int16_t)2046 ) ;
//}


//uint16_t TempChannels[8] ;

//void setUpPulsesPCM()

//#define PXX_DELAYS	1

#ifdef PXX_DELAYS
uint16_t PxxTime[2] ;
uint16_t LastPxxTime ;
#endif

void setupPulsesPXX()
{
    uint8_t i ;
    uint16_t chan ;
    uint16_t chan_1 ;
		uint8_t lpass = Pass ;

#ifdef PXX_DELAYS
	uint16_t ptime ;
	uint16_t temp ;
	ptime = TC1->TC_CHANNEL[0].TC_CV ;
	temp = ptime ;
	ptime -= LastPxxTime ;
	ptime >>= 1 ;
	LastPxxTime = temp ;
	PxxTime[lpass & 1] = ptime ;
#endif


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
    putPcmByte( g_model.Module[1].pxxRxNum ) ;
    
  	uint8_t flag1;
  	if (BindRangeFlag[1] & PXX_BIND)
		{
  	  flag1 = (g_model.Module[1].sub_protocol<< 6) | (g_model.Module[1].country << 1) | BindRangeFlag[1] ;
  	}
  	else
		{
  	  flag1 = (g_model.Module[1].sub_protocol << 6) | BindRangeFlag[1] ;
		}	

		if ( ( flag1 & (PXX_BIND | PXX_RANGE_CHECK )) == 0 )
		{
  		if (g_model.Module[1].failsafeMode != FAILSAFE_NOT_SET && g_model.Module[1].failsafeMode != FAILSAFE_RX )
			{
    		if ( FailsafeCounter[1] )
				{
	    		if ( FailsafeCounter[1]-- == 1 )
					{
    	  		flag1 |= PXX_SEND_FAILSAFE ;
					}
    			if ( ( FailsafeCounter[1] == 1 ) && (g_model.Module[1].sub_protocol == 0 ) )
					{
  	    		flag1 |= PXX_SEND_FAILSAFE ;
					}
				}
	    	if ( FailsafeCounter[1] == 0 )
				{
//					if ( g_model.Module[1].failsafeRepeat == 0 )
//					{
						FailsafeCounter[1] = 1000 ;
//					}
				}
			}
		}

		putPcmByte( flag1 ) ;     // First byte of flags
    putPcmByte( 0 ) ;     // Second byte of flags
		
		uint8_t startChan = g_model.Module[1].startChannel ;
		if ( lpass & 1 )
		{
			startChan += 8 ;			
		}
		chan = 0 ;
    for ( i = 0 ; i < 8 ; i += 1 )		// First 8 channels only
    {																	// Next 8 channels would have 2048 added
    	if (flag1 & PXX_SEND_FAILSAFE)
			{
				if ( g_model.Module[1].failsafeMode == FAILSAFE_HOLD )
				{
					chan_1 = 2047 ;
				}
				else if ( g_model.Module[1].failsafeMode == FAILSAFE_NO_PULSES )
				{
					chan_1 = 0 ;
				}
				else
				{
					// Send failsafe value
					int32_t value ;
					value = ( startChan < 16 ) ? g_model.Module[1].failsafe[startChan] : 0 ;
					value = ( value *3933 ) >> 9 ;
					value += 1024 ;					
					chan_1 = limit( (int16_t)1, (int16_t)value, (int16_t)2046 ) ;
				}
			}
			else
			{
      	chan_1 = scaleForPXX( startChan ) ;
			}
			if ( lpass & 1 )
			{
				chan_1 += 2048 ;
			}
//			TempChannels[i] = chan_1 ;
			startChan += 1 ;
//      chan_1 = scaleForPXX( startChan ) ;
//			if ( lpass & 1 )
//			{
//				chan_1 += 2048 ;
//			}
//			startChan += 1 ;
      
			if ( i & 1 )
			{
				putPcmByte( chan ) ; // Low byte of channel
				putPcmByte( ( ( chan >> 8 ) & 0x0F ) | ( chan_1 << 4) ) ;  // 4 bits each from 2 channels
      	putPcmByte( chan_1 >> 4 ) ;  // High byte of channel
			}
			else
			{
				chan = chan_1 ;
			}
    }
		
	  uint8_t extra_flags = 0 ;
		
//		/* Ext. flag (holds antenna selection on Horus internal module, 0x00 otherwise) */
//#if defined(PCBHORUS)
//  if (port == INTERNAL_MODULE) {
//    extra_flags |= g_model.moduleData[port].pxx.external_antenna;
//  }
//#endif
//#if defined(BINDING_OPTIONS)
//  extra_flags |= g_model.moduleData[port].pxx.receiver_telem_off << 1;
//  extra_flags |= g_model.moduleData[port].pxx.receiver_channel_9_16 << 2;
//#endif
//  if (IS_MODULE_R9M(port)) {
//    extra_flags |= g_model.moduleData[port].pxx.power << 3;
//    // Disable s.port if internal module is active
//    if (IS_TELEMETRY_INTERNAL_MODULE || !g_model.moduleData[port].pxx.sport_out)
//      extra_flags |=  (1<< 5);
//  }
		
// Bit 0: 0 internal, 1 external antenna
// Bit 1: 0 Telemetry ON, 1 Telemetry OFF
// Bit 2: 0 PPM 1-8, 1 PPM 9-16
// Bit 4:3: R9M power nonEU 10, 100, 500, 1000
// Bit 4:3: R9M power EU 25, 500
// Bit 5: 0 Sport enabled, 1 Sport disabled
// Bits 7:6 unused

		 
		if ( g_model.Module[1].highChannels )
//		if ( PxxExtra[1] & 1 )
		{
			extra_flags = (1 << 2 ) ;
		}
		if ( g_model.Module[1].disableTelemetry )
//		if ( PxxExtra[1] & 2 )
		{
			extra_flags |= (1 << 1 ) ;
		}
		if ( g_model.Module[1].sub_protocol == 3 )	// R9M
		{
			extra_flags |= g_model.Module[1].r9mPower << 3 ;
			if ( g_model.Module[1].r9MflexMode == 2 )
			{
				extra_flags |= 1 << 6 ;
			}
		}
		putPcmByte( extra_flags ) ;

    chan = PcmCrc ;		        // get the crc
    putPcmByte( chan >> 8 ) ; // Checksum hi
    putPcmByte( chan ) ; 			// Checksum lo
    putPcmHead(  ) ;      // sync byte
    putPcmFlush() ;
		if ( g_model.Module[1].pxxDoubleRate )
		{
			lpass += 1 ;
		}
		else
		{
			if (g_model.Module[1].sub_protocol == 1 )		// D8
			{
				lpass = 0 ;
			}
			else
			{
				lpass += 1 ;
				if ( g_model.Module[1].channels == 1 )
				{
					lpass = 0 ;
				}
			}
		}
		Pass = lpass ;
}


