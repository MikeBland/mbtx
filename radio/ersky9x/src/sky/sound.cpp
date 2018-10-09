/****************************************************************************
*  Copyright (c) 2011 by Michael Blandford. All rights reserved.
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
*  History:
*
****************************************************************************/

//#define SOFTWARE_VOLUME	1


#include <stdint.h>
#include <stdlib.h>

#include "AT91SAM3S4.h"
#include "board.h"
#ifndef SIMU
 	#include "core_cm3.h"
#endif
#include "sound.h"
#include "ersky9x.h"
#include "myeeprom.h"
#include "drivers.h"
#include "audio.h"
#include "logicio.h"

#define	TONE_MODE_2		1


void start_sound( void ) ;
//void buzzer_on( void ) ;
//void buzzer_off( void ) ;
//void buzzer_sound( uint8_t time ) ;
void start_dactimer( void ) ;
void init_dac( void ) ;
extern "C" void DAC_IRQHandler( void ) ;
void disp_mem( register uint32_t address ) ;
//void end_sound( void ) ;
void tone_start( register uint32_t time ) ;
void tone_stop( void ) ;
void init_twi( void ) ;
void setVolume( register uint8_t volume ) ;
extern "C" void TWI0_IRQHandler (void) ;
void audioDefevent( uint8_t e ) ;
void i2c_check_for_request( void ) ;


extern uint32_t Master_frequency ;
extern uint8_t CurrentVolume ;

//volatile uint8_t Buzzer_count ;

uint8_t CoProcTimer ;
uint8_t AudioVoiceUnderrun ;
uint8_t AudioVoiceCountUnderruns ;
uint8_t VoiceCount ;
uint8_t DacIdle ;


struct t_sound_globals Sound_g ;

struct t_VoiceBuffer VoiceBuffer[NUM_VOICE_BUFFERS] ;

//#define SOUND_NONE	0
//#define SOUND_TONE	1
//#define SOUND_VOICE	2
//#define SOUND_STOP	3

struct t_VoiceBuffer *PtrVoiceBuffer[NUM_VOICE_BUFFERS] ;
//uint8_t SoundType ;

	 
// Must NOT be in flash, PDC needs a RAM source.
//uint16_t Sine_values[] =
//{
//2048,2173,2298,2422,2545,2666,2784,2899,3011,3119,
//3223,3322,3417,3505,3589,3666,3736,3800,3857,3907,
//3950,3985,4012,4032,4044,4048,4044,4032,4012,3985,
//3950,3907,3857,3800,3736,3666,3589,3505,3417,3322,
//3223,3119,3011,2899,2784,2666,2545,2422,2298,2173,
//2048,1922,1797,1673,1550,1429,1311,1196,1084, 976,
// 872, 773, 678, 590, 506, 429, 359, 295, 238, 188,
// 145, 110,  83,  63,  51,  48,  51,  63,  83, 110,
// 145, 188, 238, 295, 359, 429, 506, 590, 678, 773,
// 872, 976,1084,1196,1311,1429,1550,1673,1797,1922
//} ;

//#ifndef TONE_MODE_2
//// Amplitude reduced to 30% to allow for voice volume
//uint16_t Sine_values[] =
//{
//2048,2085,2123,2160,2197,2233,2268,2303,2336,2369,
//2400,2430,2458,2485,2510,2533,2554,2573,2590,2605,
//2618,2629,2637,2643,2646,2648,2646,2643,2637,2629,
//2618,2605,2590,2573,2554,2533,2510,2485,2458,2430,
//2400,2369,2336,2303,2268,2233,2197,2160,2123,2085,
//2048,2010,1972,1935,1898,1862,1826,1792,1758,1726,
//1695,1665,1637,1610,1585,1562,1541,1522,1505,1490,
//1477,1466,1458,1452,1448,1448,1448,1452,1458,1466,
//1477,1490,1505,1522,1541,1562,1585,1610,1637,1665,
//1695,1726,1758,1792,1826,1862,1898,1935,1972,2010
//} ;
//#endif // TONE_MODE_2

// Must NOT be in flash, PDC needs a RAM source.
// We'll use these for higher frequencies
//uint16_t Sine_values64[] =
//{
//2048,2244,2438,2628,2813,2990,3159,3316,
//3462,3594,3710,3811,3895,3961,4009,4038,
//4048,4038,4009,3961,3895,3811,3710,3594,
//3462,3316,3159,2990,2813,2628,2438,2244,
//2048,1851,1657,1467,1282,1105, 936, 779,
// 633, 501, 385, 284, 200, 134,  86,  57,
//  48,  57,  86, 134, 200, 284, 385, 501,
// 633, 779, 936,1105,1282,1467,1657,1851
//} ;

//const uint16_t PianoTones[] =
//{
//  28,   29,   31,   33,   35,   37,   39,   41,   44,   46,
//  49,   52,   55,   58,   62,   65,   69,   73,   78,   82,
//  87,   92,   98,  104,  110,  117,  123,  131,  139,  147,
// 156,  165,  175,  185,  196,  208,  220,  233,  247,  262, // d#, E, F, f#, G, g#, A, a#, B, C(middle)
// 277,  294,  311,  330,  349,  370,  392,  415,  440,  466, // c#, D, d#, E, F, f#, G, g#, A, a#
// 494,  523,  554,  587,  622,  659,  698,  740,  784,  831, // B, C, c#, D, d#, E, F, f#, G, g#
// 880,  932,  988, 1047, 1109, 1175, 1245, 1319, 1397, 1480,
//1568, 1661, 1760, 1865, 1976, 2093, 2217, 2349, 2489, 2637,
//2794, 2960, 3136, 3322, 3520 ,3729, 3951, 4186
//} ;


// Sound routines

void start_sound()
{
	
	start_dactimer() ;
	init_dac() ;
	init_twi() ;
	setVolume( 2 ) ;

#ifndef REVA
//#ifndef REVX	
//	register Pio *pioptr ;
//	pioptr = PIOA ;
//	pioptr->PIO_CODR = 0x02000000L ;	// Set bit A25 OFF
//	pioptr->PIO_PER = 0x02000000L ;		// Enable bit A25 (Stock buzzer)
//	pioptr->PIO_OER = 0x02000000L ;		// Set bit A25 as output
//#endif
#else
	register Pio *pioptr ;
	pioptr = PIOA ;
	pioptr->PIO_CODR = 0x00010000L ;	// Set bit A16 OFF
	pioptr->PIO_PER = 0x00010000L ;		// Enable bit A16 (Stock buzzer)
	pioptr->PIO_OER = 0x00010000L ;		// Set bit A16 as output
#endif
#ifdef REVX	
	configure_pins( PIO_PC26, PIN_ENABLE | PIN_LOW | PIN_OUTPUT | PIN_PORTC | PIN_NO_PULLUP ) ;
	audioOn() ;
#endif
}

//#ifndef TONE_MODE_2
//#ifndef REVA
////void buzzer_on()
////{
//// #ifndef REVX
////	PIOA->PIO_SODR = 0x02000000L ;	// Set bit A25 ON
//// #endif
////}

////void buzzer_off()
////{
//// #ifndef REVX
////	PIOA->PIO_CODR = 0x02000000L ;	// Set bit A25 ON
//// #endif
////}
//#else
//void buzzer_on()
//{
//	PIOA->PIO_SODR = 0x00010000L ;	// Set bit A16 ON
//}

//void buzzer_off()
//{
//	PIOA->PIO_CODR = 0x00010000L ;	// Set bit A16 ON
//}
//#endif

//void buzzer_sound( uint8_t time )
//{
// #ifndef REVX
//	buzzer_on() ;
//	Buzzer_count = time ;
// #endif
//}
//#endif // TONE_MODE_2

void set_frequency( uint32_t frequency )
{
  register Tc *ptc ;
	register uint32_t timer ;

	timer = Master_frequency / (8 * frequency) ;		// MCK/8 and 100 000 Hz
	if ( timer > 65535 )
	{
		timer = 65535 ;		
	}
	if ( timer < 2 )
	{
		timer = 2 ;		
	}

	ptc = TC0 ;		// Tc block 0 (TC0-2)
	ptc->TC_CHANNEL[1].TC_CCR = TC_CCR0_CLKDIS ;		// Stop clock
	ptc->TC_CHANNEL[1].TC_RC = timer ;			// 100 000 Hz
	ptc->TC_CHANNEL[1].TC_RA = timer >> 1 ;
	ptc->TC_CHANNEL[1].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
}


// Start TIMER1 at 100000Hz, used for DACC trigger
void start_dactimer()
{
  register Tc *ptc ;
	register uint32_t timer ;

	// Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x01000000L ;		// Enable peripheral clock to TC1
  
	timer = Master_frequency / 800000 ;		// MCK/8 and 100 000 Hz
	ptc = TC0 ;		// Tc block 0 (TC0-2)
	ptc->TC_BCR = 0 ;			// No sync
	ptc->TC_BMR = 0 ;
	ptc->TC_CHANNEL[1].TC_CMR = 0x00008000 ;	// Waveform mode
	ptc->TC_CHANNEL[1].TC_RC = timer ;			// 100 000 Hz
	ptc->TC_CHANNEL[1].TC_RA = timer >> 1 ;
	ptc->TC_CHANNEL[1].TC_CMR = 0x0009C001 ;	// 0000 0000 0000 1001 1100 0000 0000 0001
																						// MCK/8, set @ RA, Clear @ RC waveform
	ptc->TC_CHANNEL[1].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
	Sound_g.Frequency = 1000 ;
}





// Configure DAC1 (or DAC0 for REVB)
// Not sure why PB14 has not be allocated to the DAC, although it is an EXTRA function
// So maybe it is automatically done
void init_dac()
{
	register Dacc *dacptr ;

//	SoundType = SOUND_NONE ;

	DacIdle = 1 ;
  PMC->PMC_PCER0 |= 0x40000000L ;		// Enable peripheral clock to DAC
	dacptr = DACC ;
#ifndef REVA
	dacptr->DACC_MR = 0x0B000215L ;			// 0000 1011 0000 0001 0000 0010 0001 0101
#else
	dacptr->DACC_MR = 0x0B010215L ;			// 0000 1011 0000 0001 0000 0010 0001 0101
#endif
#ifndef REVA
	dacptr->DACC_CHER	= 1 ;							// Enable channel 0
#else
	dacptr->DACC_CHER	= 2 ;							// Enable channel 1
#endif
	dacptr->DACC_CDR = 2048 ;						// Half amplitude
// Data for PDC must NOT be in flash, PDC needs a RAM source.
//#ifndef TONE_MODE_2
//#ifndef SIMU
//	dacptr->DACC_TPR = (uint32_t) Sine_values ;
//	dacptr->DACC_TNPR = (uint32_t) Sine_values ;
//#endif
//	dacptr->DACC_TCR = 50 ;		// words, 100 16 bit values
//	dacptr->DACC_TNCR = 50 ;	// words, 100 16 bit values
//	dacptr->DACC_PTCR = DACC_PTCR_TXTEN ;
//#endif // TONE_MODE_2
	NVIC_SetPriority( DACC_IRQn, 4 ) ; // Lower priority interrupt
	NVIC_EnableIRQ(DACC_IRQn) ;
}

//#ifndef SIMU
//extern "C" void DAC_IRQHandler()
//{
//	register Dacc *dacptr ;

//	dacptr = DACC ;
//// Data for PDC must NOT be in flash, PDC needs a RAM source.
//	if ( SoundType == SOUND_STOP )
//	{
//		if ( dacptr->DACC_ISR & DACC_ISR_TXBUFE ;	// All sent
//		{
//			dacptr->DACC_IDR = DACC_IDR_TXBUFE ;	// Kill interrupt
//			SoundType = SOUND_NONE ;		// Tell everyone else
//		}
//		else
//		{
//			dacptr->DACC_IDR = DACC_IDR_ENDTX ;		// Stop sending
//			dacptr->DACC_IER = DACC_IER_TXBUFE ;	// Wait for finished
//		}
//	}
//	else if ( SoundType == SOUND_TONE )
//	{
//		if ( Sound_g.Tone_timer )
//		{
//			if ( --Sound_g.Tone_timer == 0 )
//			{
//				dacptr->DACC_IDR = DACC_IDR_ENDTX ;
//				SoundType = SOUND_STOP ;
//			}
//			else
//			{
//				dacptr->DACC_TNPR = (uint32_t) Sine_values ;
//				dacptr->DACC_TNCR = 50 ;	// words, 100 16 bit values
//			}
//		}
//	}							 
//	else if ( SoundType == SOUND_VOICE )
//	{
//		VoiceBuffer[VoiceIndex[0]].flags |= VF_SENT ;		// Tell caller

//		VoiceIndex[0] = VoiceIndex[1] ;
//		VoiceIndex[1] = VoiceIndex[2] ;
//		VoiceCount -= 1 ;




//	}
//	else if ( SoundType == SOUND_NONE )
//	{
//		dacptr->DACC_IDR = DACC_IDR_ENDTX ;		// Kill interrupt
//		dacptr->DACC_IDR = DACC_IDR_TXBUFE ;	// Kill interrupt
//	}
//}
//#endif

#ifndef SIMU
extern "C" void DAC_IRQHandler()
{
// Data for PDC must NOT be in flash, PDC needs a RAM source.
	if ( Sound_g.VoiceActive == 1 )
	{
		PtrVoiceBuffer[0]->flags |= VF_SENT ;		// Flag sent
		PtrVoiceBuffer[0] = PtrVoiceBuffer[1] ;
		PtrVoiceBuffer[1] = PtrVoiceBuffer[2] ;
		 
		if ( DACC->DACC_ISR & DACC_ISR_TXBUFE )
		{
			DACC->DACC_IDR = DACC_IDR_TXBUFE ;
			VoiceCount = 0 ;
			AudioVoiceUnderrun = 1 ;		// For debug
			Sound_g.VoiceActive = 2 ;
			DacIdle = 1 ;
		}
		else
		{
			VoiceCount -= 1 ;
			if ( VoiceCount > 1 )
			{
				if ( DACC->DACC_TNCR == 0 )
				{
					uint32_t x ;
					x = DACC->DACC_TNPR = CONVERT_PTR(PtrVoiceBuffer[1]->dataw) ;
					DACC->DACC_TNCR = PtrVoiceBuffer[1]->count / 2 ;		// words, 100 16 bit values
					DACC->DACC_TNPR = x ;		// Goes wrong without this!!!
				}
			}
			else if ( VoiceCount == 1 )		// Run out of buffers
			{
				DACC->DACC_IDR = DACC_IDR_ENDTX ;
				DACC->DACC_IER = DACC_IER_TXBUFE ;
			}
		}
	}
	else
	{
		DACC->DACC_IDR = DACC_IDR_ENDTX ;
	}
//#ifndef TONE_MODE_2
//	else if ( Sound_g.VoiceActive == 0 )
//	{
//		DACC->DACC_TNPR = (uint32_t) Sine_values ;
//		DACC->DACC_TNCR = 50 ;	// words, 100 16 bit values
//		if ( Sound_g.Tone_timer )
//		{
//			if ( --Sound_g.Tone_timer == 0 )
//			{
//				DACC->DACC_IDR = DACC_IDR_ENDTX ;
//			}
//		}
//	}
//#endif // TONE_MODE_2
}
#endif


//void end_sound()
//{
//	DACC->DACC_IDR = DACC_IDR_ENDTX ;		// Kill interrupt
//	DACC->DACC_IDR = DACC_IDR_TXBUFE ;	// Kill interrupt
//	NVIC_DisableIRQ(DACC_IRQn) ;
//	TWI0->TWI_IDR = TWI_IDR_TXCOMP ;
//	NVIC_DisableIRQ(TWI0_IRQn) ;
//	PMC->PMC_PCER0 &= ~0x00080000L ;		// Disable peripheral clock to TWI0
//  PMC->PMC_PCER0 &= ~0x40000000L ;		// Disable peripheral clock to DAC
//}

#ifdef REVX
void audioOn( void )
{
	PIOC->PIO_CODR = PIO_PC26 ;
}

void audioOff( void )
{
	PIOC->PIO_SODR = PIO_PC26 ;
}
#endif


// Called every 5mS from interrupt routine
void sound_5ms()
{
	register Dacc *dacptr ;

	if ( CoProcTimer )
	{
		if ( --CoProcTimer == 0 )
		{
			init_twi() ;
		}
	}

	dacptr = DACC ;

//#ifndef TONE_MODE_2
//	if ( Sound_g.Tone_ms_timer > 0 )
//	{
//		Sound_g.Tone_ms_timer -= 1 ;
//	}
		
//	if ( Sound_g.Tone_ms_timer == 0 )
//	{
//#endif // TONE_MODE_2
		if ( Sound_g.VoiceRequest )
		{
			// audioOn() ;
			
			DacIdle = 0 ;
			dacptr->DACC_IDR = DACC_IDR_ENDTX ;	// Disable interrupt
			Sound_g.Sound_time = 0 ;						// Remove any pending tone requests
			if ( dacptr->DACC_ISR & DACC_ISR_TXBUFE )	// All sent
			{
				// Now we can send the voice file
				Sound_g.VoiceRequest = 0 ;
				Sound_g.VoiceActive = 1 ;

				set_frequency( VoiceBuffer[0].frequency ? VoiceBuffer[0].frequency : 16000 ) ;
#ifndef SIMU
				dacptr->DACC_PTCR = DACC_PTCR_TXTDIS ;
				dacptr->DACC_TPR = (uint32_t) VoiceBuffer[0].dataw ;
				dacptr->DACC_TCR = VoiceBuffer[0].count / 2 ;		// words, 100 16 bit values
	
				if ( VoiceCount > 1 )
				{
					dacptr->DACC_TNPR = (uint32_t) VoiceBuffer[1].dataw ;
					dacptr->DACC_TNCR = VoiceBuffer[1].count / 2 ;		// words, 100 16 bit values
				}
				dacptr->DACC_PTCR = DACC_PTCR_TXTEN ;
				dacptr->DACC_IER = DACC_IER_ENDTX ;
#endif
			}
			return ;
		}
		
//#ifndef TONE_MODE_2
//		if ( ( Sound_g.VoiceActive ) || ( ( Voice.VoiceQueueCount ) && sd_card_ready() ) )
//		{
//			Sound_g.Sound_time = 0 ;						// Remove any pending tone requests
//			return ;
//		}
				
//		if ( Sound_g.Sound_time )
//		{
//			// audioOn() ;
//			Sound_g.Tone_ms_timer = ( Sound_g.Sound_time + 4 ) / 5 ;
//			if ( Sound_g.Next_freq )		// 0 => silence for time
//			{
//				Sound_g.Frequency = Sound_g.Next_freq ;
//				Sound_g.Frequency_increment = Sound_g.Next_frequency_increment ;
//				set_frequency( Sound_g.Frequency * 100 ) ;
//#ifndef SIMU
//				dacptr->DACC_TPR = (uint32_t) Sine_values ;
//				dacptr->DACC_TNPR = (uint32_t) Sine_values ;
//#endif
//				dacptr->DACC_TCR = 50 ;		// words, 100 16 bit values
//				dacptr->DACC_TNCR = 50 ;	// words, 100 16 bit values
//				tone_start( 0 ) ;
//			}
//			else
//			{
//				dacptr->DACC_IDR = DACC_IDR_ENDTX ;		// Silence
//			}
//			Sound_g.Sound_time = 0 ;
//		}
//		else
//		{
//			dacptr->DACC_IDR = DACC_IDR_ENDTX ;	// Disable interrupt
//			Sound_g.Tone_timer = 0 ;
//			// audioOff() ;
//		}
//#endif // TONE_MODE_2

//#ifndef TONE_MODE_2
//	}
//	else if ( ( Sound_g.Tone_ms_timer & 1 ) == 0 )		// Every 10 mS
//	{
//		if ( Sound_g.Frequency )
//		{
//			if ( Sound_g.Frequency_increment )
//			{
//				Sound_g.Frequency += Sound_g.Frequency_increment ;
//				set_frequency( Sound_g.Frequency * 100 ) ;
//			}
//		}
//	}
//#endif // TONE_MODE_2
}

//const uint8_t SwVolume_scale[NUM_VOL_LEVELS] = 
//{
////	 0,  15,  30,   40,   47,  55,  64,  74,  84,  94,  104,  114,
//	 0,  5,  10,   15,   30,  45,  60,  74,  84,  94,  104,  114,
//	128, 164, 192, 210, 224, 234, 240, 244, 248, 251, 253, 255 	
//} ;

//uint16_t swVolumeLevel()
//{
//	return SwVolume_scale[CurrentVolume] ;
//}

//void wavU8Convert( uint8_t *src, uint16_t *dest , uint32_t count )
//{
//	if ( g_eeGeneral.softwareVolume )
//	{
//		uint32_t multiplier ;
//		int32_t value ;
//		multiplier = SwVolume_scale[CurrentVolume] * 256 ;
//		while( count-- )
//		{
//			value = (int8_t) (*src++ - 128 ) ;
//			value *= multiplier ;
//			value += 32768 * 256 ;
//			*dest++ = value >> 12 ;
//		}
//	}
//	else
//	{
//		while( count-- )
//		{
//			*dest++ = *src++ << 4 ;
//		}
//	}
//}

//void wavU16Convert( uint16_t *src, uint16_t *dest , uint32_t count )
//{
//	if ( g_eeGeneral.softwareVolume )
//	{
//		uint32_t multiplier ;
//		int32_t value ;
//		multiplier = SwVolume_scale[CurrentVolume] ;
//		while( count-- )
//		{
//			value = (int16_t) *src++ ;
//			value *= multiplier ;
//			value += 32768 * 256 ;
//			*dest++ = value >> 12 ;
//		}
//	}
//	else
//	{
//		while( count-- )
//		{
//			*dest++ = (uint16_t)( (int16_t )*src++ + 32768) >> 4 ;
//		}
//	}
//}

//uint16_t g_timeAppendVoice ;
//uint16_t g_timeAppendMaxVoice ;
//uint16_t g_timeAppendtime ;

void startVoice( uint32_t count )		// count of filled in buffers
{
	AudioVoiceUnderrun = 0 ;
	VoiceBuffer[0].flags &= ~VF_SENT ;
	PtrVoiceBuffer[0] = &VoiceBuffer[0] ;
	if ( count > 1 )
	{
		VoiceBuffer[1].flags &= ~VF_SENT ;
		PtrVoiceBuffer[1] = &VoiceBuffer[1] ;
	}
	if ( count > 2 )
	{
		VoiceBuffer[2].flags &= ~VF_SENT ;
		PtrVoiceBuffer[2] = &VoiceBuffer[2] ;
	}
	VoiceCount = count ;
	Sound_g.VoiceRequest = 1 ;

//	g_timeAppendVoice = 0 ;
//	g_timeAppendMaxVoice = 0 ;
//	g_timeAppendtime = get_tmr10ms() ;

}

void endVoice()
{
	if ( Sound_g.VoiceActive == 2 )
	{
		Sound_g.VoiceActive = 0 ;
	}
}

void appendVoice( uint32_t index )		// index of next buffer
{
	register Dacc *dacptr ;
	VoiceBuffer[index].flags &= ~VF_SENT ;
	dacptr = DACC ;
	__disable_irq() ;
	
//	if ( ( dacptr->DACC_IMR & DACC_IMR_ENDTX ) == 0 )	// underrun
//	{
//		dacptr->DACC_IDR = DACC_IDR_TXBUFE ;
//		dacptr->DACC_IER = DACC_IER_ENDTX ;
//	}	

	PtrVoiceBuffer[VoiceCount++] = &VoiceBuffer[index] ;

	if ( DacIdle )	// All sent
//	if ( Sound_g.VoiceActive == 2 )
	{
		DacIdle = 0 ;
		Sound_g.VoiceActive = 1 ;
		uint32_t x ;
		uint32_t y ;
		x = dacptr->DACC_TPR = (uint32_t) VoiceBuffer[index].dataw ;
		y = VoiceBuffer[index].count / 2 ;
		if ( y == 0 )
		{
			y = 1 ;
		}
		dacptr->DACC_TCR = y ;		// words, 100 16 bit values
		dacptr->DACC_TPR = x ;		// Goes wrong without this!!!
		dacptr->DACC_PTCR = DACC_PTCR_TXTEN ;
		dacptr->DACC_IER = DACC_IER_TXBUFE ;		// Only one buffer
	}
	else
	{
		if ( VoiceCount == 2 )
		{
			uint32_t y ;
			dacptr->DACC_TNPR = CONVERT_PTR(VoiceBuffer[index].dataw);
			y = VoiceBuffer[index].count / 2 ;
			if ( y == 0 )
			{
				y = 1 ;
			}
			dacptr->DACC_TNCR = y ;		// words, 100 16 bit values
			dacptr->DACC_IDR = DACC_IDR_TXBUFE ;
			dacptr->DACC_IER = DACC_IER_ENDTX ;
		}
	}
	__enable_irq() ;
	
//	uint16_t t10ms ;
//	uint16_t now ;
//	now = get_tmr10ms() ;
//	t10ms = now - g_timeAppendtime ;
//	g_timeAppendtime = now ;
//	if ( t10ms > g_timeAppendMaxVoice )
//	{
//		g_timeAppendMaxVoice = t10ms ;
//	}
//	g_timeAppendVoice = t10ms ;
}

// frequency in Hz, time in mS
//void playTone( uint32_t frequency, uint32_t time )
//{
//	Sound_g.Next_frequency_increment = 0 ;
//	Sound_g.Next_freq = frequency ;
//	Sound_g.Sound_time = time ;
////	set_frequency( frequency ) ;
////	Tone_ms_timer = ( time + 4 ) / 5 ;
////	tone_start( 0 ) ;
//}

//uint32_t unlockTone()
//{
//	if ( Sound_g.Sound_time == 0 )
//	{
//		Sound_g.toneLock = 0 ;
//		return 1 ;
//	}
//	return 0 ;
//}

//#ifndef TONE_MODE_2
//uint32_t queueTone( uint32_t frequency, uint32_t time, uint32_t frequency_increment, uint32_t lock )
//{
//	if ( Sound_g.Sound_time == 0 )
//	{
//		Sound_g.Next_freq = frequency ;
//		Sound_g.Next_frequency_increment = frequency_increment ;
//		Sound_g.Sound_time = time ;
//		Sound_g.toneLock = lock ;
//		return 1 ;
//	}
//	return 0 ;	
//}

//// Time is in milliseconds
//void tone_start( register uint32_t time )
//{
//  PMC->PMC_PCER0 |= 0x40000000L ;		// Enable peripheral clock to DAC
//	Sound_g.Tone_timer = Sound_g.Frequency * time / 1000 ;
//	DACC->DACC_IER = DACC_IER_ENDTX ;
//}

//void tone_stop()
//{
//	DACC->DACC_IDR = DACC_IDR_ENDTX ;	// Disable interrupt
//	Sound_g.Tone_timer = 0 ;	
//}
//#endif // TONE_MODE_2


uint32_t TwiHighSpeed ;
uint32_t TwiLowSpeed ;

// Set up for volume control (TWI0)
// Need PA3 and PA4 set to peripheral A
void init_twi()
{
	register Pio *pioptr ;
	register uint32_t timing ;
  
	PMC->PMC_PCER0 |= 0x00080000L ;		// Enable peripheral clock to TWI0

	TWI0->TWI_CR = TWI_CR_SWRST ;				// Reset in case we are restarting
	 
	/* Configure PIO */
	pioptr = PIOA ;
  pioptr->PIO_ABCDSR[0] &= ~0x00000018 ;	// Peripheral A
  pioptr->PIO_ABCDSR[1] &= ~0x00000018 ;	// Peripheral A
  pioptr->PIO_PDR = 0x00000018 ;					// Assign to peripheral
	
//	timing = Master_frequency * 8 / 2000000 ;		// 4uS high and low (125 kHz)
	timing = Master_frequency * 5 / 2000000 ;		// 2.5uS high and low (200 kHz)
//	timing = Master_frequency * 10 / 2000000 ;		// 5uS high and low (100 kHz)
	timing += 15 - 4 ;
	timing /= 16 ;
	timing |= timing << 8 ;
	TWI0->TWI_CWGR = TwiHighSpeed = 0x00040000 | timing ;			// TWI clock set

	timing = Master_frequency * 10 / 2000000 ;		// 5uS high and low (100 kHz)
	timing += 15 - 4 ;
	timing /= 16 ;
	timing |= timing << 8 ;
	TwiLowSpeed = 0x00040000 | timing ;
	
	TWI0->TWI_CR = TWI_CR_MSEN | TWI_CR_SVDIS ;		// Master mode enable
	TWI0->TWI_MMR = 0x002F0000 ;		// Device 5E (>>1) and master is writing
  NVIC_SetPriority(TWI0_IRQn, 3 ) ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
}

static int8_t Volume_required ;
static uint8_t Volume_read_pending ;
 #ifndef REVX
static uint8_t CoProc_read_pending ;
static uint8_t CoProc_write_pending ;
static uint8_t CoProc_appgo_pending ;
#endif

#ifdef REVX
uint8_t Rtc_read_pending ;
static uint8_t Rtc_write_pending ;
static uint8_t MFPsetting = 0 ;
uint8_t CALsetting = 0 ;
#endif

uint8_t Volume_read ;
uint8_t Coproc_read ;
int8_t Coproc_valid ;
int8_t Rtc_valid ;
//static uint8_t Twi_mode ;
uint8_t TwiOperation ;
static uint8_t *Twi_read_address ;
//static uint8_t TwiDevice ;
//const uint8_t *CoProgBufPtr ;
//uint8_t Program_coprocessor ;
//uint8_t Program_blocks_written ;

//#define TWI_MODE_WRITE	0
//#define TWI_MODE_READ		1

//#define TWI_VOLUME	1
//#define TWI_COPROC	2

#define TWI_NONE		    	0
#define TWI_READ_VOL    	1
#define TWI_WRITE_VOL     2
 #ifndef REVX
#define TWI_READ_COPROC   3
#define TWI_COPROC_APPGO  4
#define TWI_WAIT_STOP		  5
#define TWI_WRITE_COPROC	6
#endif
#define TWI_WAIT_COMP		  7
#define TWI_READ_RTC		  8
#define TWI_WRITE_RTC		  9
#define TWI_WAIT_RTCSTOP	10
#define TWI_WRITE_MFP     11

// General I2C operation
#define TWI_WRITE_ONE			12
#define TWI_READ_ONE			13
#define TWI_WRITE_BUFFER	14
#define TWI_READ_BUFFER		15
#define TWI_WRITE_COMMAND_BUFFER    16
#define TWI_READ_COMMAND_BUFFER     17
#define TWI_WAIT_BUFFER		18



// Commands to the coprocessor bootloader/application
#define TWI_CMD_PAGEUPDATE        	0x01	// TWI Command to program a flash page
#define TWI_CMD_EXECUTEAPP        	0x02	// TWI Command to jump to the application program
#define TWI_CMD_SETREAD_ADDRESS			0x03	// TWI Command to set address to read from
#define TWI_CMD_WRITE_DATA         	0x04	// TWI Command send data to the application
#define TWI_CMD_REBOOT							0x55	// TWI Command to restart back in the bootloader


static const uint8_t Volume_scale[NUM_VOL_LEVELS] = 
{
	 0,  2,  4,   6,   8,  10,  13,  17,  22,  27,  33,  40,
	64, 82, 96, 105, 112, 117, 120, 122, 124, 125, 126, 127 	
} ;

 #ifndef REVX
#define COPROC_RX_BUXSIZE		22
#endif
#define RTC_RX_BUXSIZE			10
#define RTC_SIZE						8
 
 #ifndef REVX
uint8_t Co_proc_status[COPROC_RX_BUXSIZE] ;
uint8_t *Co_proc_write_ptr ;
uint32_t Co_proc_write_count ;
#endif

uint8_t Rtc_status[RTC_RX_BUXSIZE] ;
#ifdef REVX
uint8_t *Rtc_write_ptr ;
uint32_t Rtc_write_count ;
uint8_t RtcConfig[8] ;		// For initial config and writing to RTC
// 0x80, 0, 0, 0x08, 0, 0, 0, 0x80
#endif


#ifndef SMALL
static uint32_t fromBCD( uint8_t bcd_value )
{
	return ( ( ( bcd_value & 0xF0 ) * 10 ) >> 4 ) + ( bcd_value & 0x0F ) ;
}

static uint32_t toBCD( uint32_t value )
{
	div_t qr ;
	qr = div( value, 10 ) ;
	return ( qr.quot << 4 ) + qr.rem ;
}
#endif

// General I2C operation
//#define GENERAL_I2C_WRITE_ONE								0
//#define GENERAL_I2C_READ_ONE                1
//#define GENERAL_I2C_WRITE_BUFFER            2
//#define GENERAL_I2C_READ_BUFFER             3
//#define GENERAL_I2C_WRITE_COMMAND_BUFFER    4
//#define GENERAL_I2C_READ_COMMAND_BUFFER     5

#define TWI_HIGH_SPEED 0 ;
#define TWI_LOW_SPEED 1 ;

struct t_I2C_request
{
	uint32_t mmr ;		// 0x00AARa00  AA=device address, a=address byte count, R=1 for read
	uint8_t *dataBuffer ;
	uint32_t address ;
	uint32_t dataSize ;
	struct t_I2C_request *next ;
	uint8_t commandByte ;
	uint8_t operationType ;
	uint8_t speed ;
	volatile uint8_t done ;
} ;

#define MMR_ADDRESS_COUNT_MASK	0x00000300

struct t_I2C_request GeneralI2cRequest ;
struct t_I2C_request LedI2cRequest ;
struct t_I2C_request *I2cHeadPointer ;
struct t_I2C_request *I2cTailPointer ;
struct t_I2C_request *I2cCurrentPointer ;
struct t_I2C_request ExtRtcI2cRequest ;
struct t_I2C_request ExtRtcI2cWriteRequest ;


void submitI2cRequest( struct t_I2C_request *ptr )
{
	ptr->done = 0 ;
	ptr->next = (struct t_I2C_request *) NULL ;
	NVIC_DisableIRQ(TWI0_IRQn) ;
	if ( I2cHeadPointer )
	{
		I2cTailPointer->next = ptr ;
	}
	else
	{
		I2cHeadPointer = ptr ;
		I2cTailPointer = ptr ;
	}
	i2c_check_for_request() ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
}

#ifndef SMALL
uint8_t Mcp23008InitData[7] = {0, 0, 0, 0, 0, 0, 0 } ;
uint8_t LedInitData[14] = {1, 0, 5, 5, 5, 0, 0, 0 ,0 ,0, 0xFF, 0, 0xAA, 0xAA } ;
uint8_t LedLightData[3] ;
#ifndef REVX
uint8_t ExternalRtc[RTC_SIZE] ;
#endif
#endif

#ifndef SMALL
void initLed()
{
	LedI2cRequest.mmr = 0x00480100 ;	// writing, 1 byte addr
	LedI2cRequest.address = 0x80 ;
	LedI2cRequest.dataSize = 14 ;
	LedI2cRequest.dataBuffer = LedInitData ;
	LedI2cRequest.operationType = TWI_WRITE_BUFFER ;
	LedI2cRequest.speed = TWI_HIGH_SPEED ;
	submitI2cRequest( &LedI2cRequest ) ;
}

void writeLed( uint8_t value )
{
	LedI2cRequest.mmr = 0x00480100 ;	// writing, 1 byte addr
	LedI2cRequest.address = 0x82 ;
	LedLightData[0] = value ;
	LedLightData[1] = value ;
	LedLightData[2] = value ;
	LedI2cRequest.dataSize = 3 ;
	LedI2cRequest.dataBuffer = LedLightData ;
	LedI2cRequest.operationType = TWI_WRITE_BUFFER ;
	LedI2cRequest.speed = TWI_HIGH_SPEED ;
	submitI2cRequest( &LedI2cRequest ) ;
}

void readLed( uint8_t *ptrData )
{
	LedI2cRequest.mmr = 0x00481100 ;	// reading, 1 byte addr
	LedI2cRequest.address = 0x80 ;
	LedI2cRequest.dataBuffer = ptrData ;
	LedI2cRequest.operationType = TWI_READ_ONE ;
	submitI2cRequest( &LedI2cRequest ) ;
}

void init23008()
{
	GeneralI2cRequest.mmr = 0x00200100 ;	// writing, 1 byte addr
	GeneralI2cRequest.address = 0 ;
	GeneralI2cRequest.dataSize = 7 ;
	GeneralI2cRequest.dataBuffer = Mcp23008InitData ;
	Mcp23008InitData[0] = 3 ;	// 2 inputs
	Mcp23008InitData[6] = 3 ;	// Pullups
	GeneralI2cRequest.operationType = TWI_WRITE_BUFFER ;
	GeneralI2cRequest.speed = TWI_HIGH_SPEED ;
	submitI2cRequest( &GeneralI2cRequest ) ;
}

void write23008( uint8_t outputs )
{
	GeneralI2cRequest.mmr = 0x00200100 ;	// writing, 1 byte addr
	GeneralI2cRequest.address = 10 ;
	GeneralI2cRequest.commandByte = outputs ;
	GeneralI2cRequest.operationType = TWI_WRITE_ONE ;
	GeneralI2cRequest.speed = TWI_HIGH_SPEED ;
	submitI2cRequest( &GeneralI2cRequest ) ;
}

void read23008( uint8_t *ptrData )
{
	GeneralI2cRequest.mmr = 0x00201100 ;	// reading, 1 byte addr
	GeneralI2cRequest.address = 9 ;
	GeneralI2cRequest.dataBuffer = ptrData ;
	GeneralI2cRequest.operationType = TWI_READ_ONE ;
	GeneralI2cRequest.speed = TWI_HIGH_SPEED ;
	submitI2cRequest( &GeneralI2cRequest ) ;
}

uint32_t readI2cEncoder( uint8_t *ptrData )
{
	GeneralI2cRequest.mmr = 0x00041000 ;	// reading, No addr
//	GeneralI2cRequest.address = 0 ;
	GeneralI2cRequest.dataSize = 2 ;
	GeneralI2cRequest.dataBuffer = ptrData ;
	GeneralI2cRequest.operationType = TWI_READ_BUFFER ;
	GeneralI2cRequest.speed = TWI_HIGH_SPEED ;
	submitI2cRequest( &GeneralI2cRequest ) ;
	return 1 ;
}

#ifndef REVX
void readExtRtc()
{
	ExtRtcI2cRequest.mmr = 0x00681100 ;	// reading, 1 byte addr
	ExtRtcI2cRequest.address = 0 ;
	ExtRtcI2cRequest.dataSize = RTC_SIZE ;
	ExtRtcI2cRequest.dataBuffer = ExternalRtc ;
	ExtRtcI2cRequest.operationType = TWI_READ_BUFFER ;
	ExtRtcI2cRequest.speed = TWI_LOW_SPEED ;
	submitI2cRequest( &ExtRtcI2cRequest ) ;
}
#endif

#ifndef REVX
void pollForRtcComplete()
{
	if ( ExtRtcI2cRequest.done )
	{
		ExtRtcI2cRequest.done = 0 ;
		if ( ExternalRtc[6] )
		{
			Rtc_valid = 1 ;
			// Set the date and time
			t_time *p = &Time ;
			p->second = fromBCD( ExternalRtc[0] & 0x7F ) ;
			p->minute = fromBCD( ExternalRtc[1] & 0x7F ) ;
			p->hour = fromBCD( ExternalRtc[2] & 0x3F ) ;
			p->date = fromBCD( ExternalRtc[4] & 0x3F ) ;
			p->month = fromBCD( ExternalRtc[5] & 0x1F ) ;
			p->year = fromBCD( ExternalRtc[6] ) + 2000 ;
		}
	}
}
#endif

#ifndef REVX
void writeExtRtc( uint8_t *ptr )
{
	uint32_t year ;
	ExtRtcI2cWriteRequest.mmr = 0x00680100 ;	// writing, 1 byte addr
	ExtRtcI2cWriteRequest.address = 0 ;
	ExtRtcI2cWriteRequest.dataSize = 7 ;
	ExtRtcI2cWriteRequest.dataBuffer = ExternalRtc ;
	ExternalRtc[0] = toBCD( *ptr++ ) ;
	ExternalRtc[1] = toBCD( *ptr++ ) ;
	ExternalRtc[2] = toBCD( *ptr++ ) ;
	ExternalRtc[3] = 1 ;
	ExternalRtc[4] = toBCD( *ptr++ ) ;
	ExternalRtc[5] = toBCD( *ptr++ ) ;
	year = *ptr++ ;
	year |= *ptr << 8 ;
	ExternalRtc[6] = toBCD( year - 2000 ) ;
	ExtRtcI2cWriteRequest.operationType = TWI_WRITE_BUFFER ;
	ExtRtcI2cWriteRequest.speed = TWI_LOW_SPEED ;
	submitI2cRequest( &ExtRtcI2cWriteRequest ) ;
}
#endif
#endif

// This is called from an interrupt routine, or
// interrupts must be disabled while it is called
// from elsewhere.
void i2c_check_for_request()
{
	if ( TWI0->TWI_IMR & TWI_IMR_TXCOMP )
	{
		return ;		// Busy
	}
//#if 0
	TWI0->TWI_CR = TWI_CR_MSEN  ;		// Master mode enable

	if ( I2cHeadPointer )
	{
		I2cCurrentPointer = I2cHeadPointer ;
		I2cHeadPointer = I2cHeadPointer->next ;
		
		TWI0->TWI_MMR = I2cCurrentPointer->mmr ;
		TWI0->TWI_IADR = I2cCurrentPointer->address ;
		TWI0->TWI_CWGR = I2cCurrentPointer->speed ? TwiLowSpeed : TwiHighSpeed ;

		if ( I2cCurrentPointer->operationType == TWI_WRITE_ONE )
		{
			TwiOperation = TWI_WRITE_ONE ;
			TWI0->TWI_THR = I2cCurrentPointer->commandByte ;		// Send data
			TWI0->TWI_IER = TWI_IER_TXCOMP ;
			TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
		}
		else if ( I2cCurrentPointer->operationType == TWI_READ_ONE )
		{
			TwiOperation = TWI_READ_ONE ;
			TWI0->TWI_CR = TWI_CR_START | TWI_CR_STOP ;		// Start and stop Tx
			TWI0->TWI_IER = TWI_IER_TXCOMP ;
		}
		else if ( I2cCurrentPointer->operationType == TWI_WRITE_BUFFER )
		{
			TwiOperation = TWI_WRITE_BUFFER ;
#ifndef SIMU
			TWI0->TWI_TPR = (uint32_t)I2cCurrentPointer->dataBuffer+1 ;
#endif
			TWI0->TWI_TCR = I2cCurrentPointer->dataSize-1 ;
			TWI0->TWI_IADR = I2cCurrentPointer->address ;
			TWI0->TWI_THR = *I2cCurrentPointer->dataBuffer ;	// First byte
			TWI0->TWI_PTCR = TWI_PTCR_TXTEN ;	// Start data transfer
			TWI0->TWI_IER = TWI_IER_TXBUFE | TWI_IER_TXCOMP ;
		}
		else if ( I2cCurrentPointer->operationType == TWI_READ_BUFFER )
		{
			TwiOperation = TWI_READ_BUFFER ;
#ifndef SIMU
			TWI0->TWI_RPR = (uint32_t)I2cCurrentPointer->dataBuffer ;
#endif
			TWI0->TWI_RCR = I2cCurrentPointer->dataSize-1 ;
			if ( TWI0->TWI_SR & TWI_SR_RXRDY )
			{
				(void) TWI0->TWI_RHR ;
			}
//			(void) TWI0->TWI_SR ;		// Read as some bits cleared on read
			TWI0->TWI_PTCR = TWI_PTCR_RXTEN ;	// Start data transfer
			TWI0->TWI_CR = TWI_CR_START ;		// Start Rx
			TWI0->TWI_IER = TWI_IER_RXBUFF | TWI_IER_TXCOMP ;
		}
	}
	else
//#endif	 

	TWI0->TWI_CWGR = TwiHighSpeed ;
	if ( Volume_required >= 0 )				// Set volume to this value
	{
		TWI0->TWI_MMR = 0x002F0000 ;		// Device 5E (>>1) and master is writing
		TwiOperation = TWI_WRITE_VOL ;
		TWI0->TWI_THR = Volume_required ;		// Send data
		Volume_required = -1 ;
		TWI0->TWI_IER = TWI_IER_TXCOMP ;
		TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
	}
#ifndef REVX
	else if ( CoProc_read_pending )
	{
		Coproc_valid = 0 ;
		CoProc_read_pending = 0 ;
		TWI0->TWI_MMR = 0x00351000 ;		// Device 35 and master is reading
		TwiOperation = TWI_READ_COPROC ;
#ifndef SIMU
		TWI0->TWI_RPR = (uint32_t)&Co_proc_status[0] ;
#endif
		TWI0->TWI_RCR = COPROC_RX_BUXSIZE - 1 ;
		if ( TWI0->TWI_SR & TWI_SR_RXRDY )
		{
			(void) TWI0->TWI_RHR ;
		}

		TWI0->TWI_PTCR = TWI_PTCR_RXTEN ;	// Start transfers
		TWI0->TWI_CR = TWI_CR_START ;		// Start Rx
		TWI0->TWI_IER = TWI_IER_RXBUFF | TWI_IER_TXCOMP ;
	}
#endif
	else if ( Volume_read_pending )
	{
		Volume_read_pending = 0 ;
		TWI0->TWI_MMR = 0x002F1000 ;		// Device 5E (>>1) and master is reading
		TwiOperation = TWI_READ_VOL ;
		Twi_read_address = &Volume_read ;
		TWI0->TWI_CR = TWI_CR_START | TWI_CR_STOP ;		// Start and stop Tx
		TWI0->TWI_IER = TWI_IER_TXCOMP ;
	}
 #ifndef REVX
	else if ( CoProc_appgo_pending )
	{
		CoProc_appgo_pending = 0 ;
		TWI0->TWI_MMR = 0x00350000 ;		// Device 35 and master is writing
		TwiOperation = TWI_COPROC_APPGO ;
		TWI0->TWI_THR = TWI_CMD_EXECUTEAPP ;	// Send appgo command
		TWI0->TWI_IER = TWI_IER_TXCOMP ;
		TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
	}
	else if ( CoProc_write_pending )
	{
		CoProc_write_pending = 0 ;
		TWI0->TWI_MMR = 0x00350000 ;		// Device 35 and master is writing
		TwiOperation = TWI_WRITE_COPROC ;
#ifndef SIMU
		TWI0->TWI_TPR = (uint32_t)Co_proc_write_ptr ;
#endif
		TWI0->TWI_TCR = Co_proc_write_count ;
		TWI0->TWI_THR = TWI_CMD_WRITE_DATA ;	// Send write command
		TWI0->TWI_PTCR = TWI_PTCR_TXTEN ;	// Start data transfer
		TWI0->TWI_IER = TWI_IER_TXBUFE | TWI_IER_TXCOMP ;
	}
 #endif
 #ifdef REVX
	else if ( Rtc_read_pending )
	{
		Rtc_valid = 0 ;
		Rtc_read_pending = 0 ;
		TWI0->TWI_MMR = 0x006F1100 ;		// Device 6F and master is reading, 1 byte addr
		TWI0->TWI_IADR = 0 ;
		TwiOperation = TWI_READ_RTC ;
#ifndef SIMU
		TWI0->TWI_RPR = (uint32_t)&Rtc_status[0] ;
#endif
		TWI0->TWI_RCR = RTC_SIZE - 1 ;
		if ( TWI0->TWI_SR & TWI_SR_RXRDY )
		{
			(void) TWI0->TWI_RHR ;
		}

		TWI0->TWI_PTCR = TWI_PTCR_RXTEN ;	// Start transfers
		TWI0->TWI_CR = TWI_CR_START ;		// Start Rx
		TWI0->TWI_IER = TWI_IER_RXBUFF | TWI_IER_TXCOMP ;
	}
	else if ( Rtc_write_pending )
	{
		if ( Rtc_write_pending & (2|4) )
		{
			TWI0->TWI_MMR = 0x006F0100 ;		// Device 6F and master is writing, 1 byte addr
			TwiOperation = TWI_WRITE_MFP ;
			if ( Rtc_write_pending & 2 )
			{
				TWI0->TWI_IADR = 7 ;
				TWI0->TWI_THR = MFPsetting ;	// Send data
				Rtc_write_pending &= ~2 ;
			}
			else
			{
				TWI0->TWI_IADR = 8 ;
				TWI0->TWI_THR = CALsetting ;	// Send data
				Rtc_write_pending &= ~4 ;
			}
			TWI0->TWI_IER = TWI_IER_TXCOMP ;
			TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
		}
		else
		{
			Rtc_write_pending &= ~1 ;
			TWI0->TWI_MMR = 0x006F0100 ;		// Device 6F and master is writing, 1 byte addr
			TWI0->TWI_IADR = 0 ;
			TwiOperation = TWI_WRITE_RTC ;
#ifndef SIMU
			TWI0->TWI_TPR = (uint32_t)Rtc_write_ptr+1 ;
#endif
			TWI0->TWI_TCR = Rtc_write_count-1 ;
			TWI0->TWI_THR = *Rtc_write_ptr ;	// First byte
			TWI0->TWI_PTCR = TWI_PTCR_TXTEN ;	// Start data transfer
			TWI0->TWI_IER = TWI_IER_TXBUFE | TWI_IER_TXCOMP ;
		}
	}
 #endif // REVX
	if ( TwiOperation != TWI_NONE )
	{
		// operation started
		CoProcTimer = 10 ;		// Start 50mS timeout
	}
}

void setVolume( register uint8_t volume )
{
//	PMC->PMC_PCER0 |= 0x00080000L ;		// Enable peripheral clock to TWI0
	
	if ( volume >= NUM_VOL_LEVELS )
	{
		volume = NUM_VOL_LEVELS - 1 ;		
	}
	CurrentVolume = volume ;
#ifndef PCB9XT
	volume = Volume_scale[volume] ;
	if ( g_eeGeneral.softwareVolume )
	{
		Volume_required = 127 ;
	}
	else
	{
		Volume_required = volume ;
	}
	NVIC_DisableIRQ(TWI0_IRQn) ;
	i2c_check_for_request() ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
#endif
}

void read_volume()
{
	Volume_read_pending = 1 ;
	NVIC_DisableIRQ(TWI0_IRQn) ;
	i2c_check_for_request() ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
}

#ifndef REVX
void read_coprocessor()
{
	CoProc_read_pending = 1 ;
	NVIC_DisableIRQ(TWI0_IRQn) ;
	i2c_check_for_request() ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
}	
#endif

#ifdef REVX


void writeMFP()
{
	NVIC_DisableIRQ(TWI0_IRQn) ;
	Rtc_write_pending |= 2 ;
	i2c_check_for_request() ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
}

void setMFP()
{
	MFPsetting = 0x80 ;
	writeMFP() ;
}

void clearMFP()
{
	MFPsetting = 0 ;
	writeMFP() ;
}

void setRtcCAL( uint8_t value )
{
	CALsetting = value ;
	NVIC_DisableIRQ(TWI0_IRQn) ;
	Rtc_write_pending |= 4 ;
	i2c_check_for_request() ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
}


void writeRTC( uint8_t *ptr )
{
	uint32_t year ;
	RtcConfig[0] = 0x80 | toBCD( *ptr++ ) ;
	RtcConfig[1] = toBCD( *ptr++ ) ;
	RtcConfig[2] = toBCD( *ptr++ ) ;
	RtcConfig[3] = 0x08 ;
	RtcConfig[4] = toBCD( *ptr++ ) ;
	RtcConfig[5] = toBCD( *ptr++ ) ;
	year = *ptr++ ;
	year |= *ptr << 8 ;
	RtcConfig[6] = toBCD( year - 2000 ) ;
	RtcConfig[7] = MFPsetting ;
	Rtc_write_ptr = RtcConfig ;
	Rtc_write_count = 8 ;
	NVIC_DisableIRQ(TWI0_IRQn) ;
	Rtc_write_pending |= 1 ;
	i2c_check_for_request() ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
}

void readRTC()
{
	Rtc_read_pending = 1 ;
	NVIC_DisableIRQ(TWI0_IRQn) ;
	i2c_check_for_request() ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
}
#endif

uint8_t ExternalBits ;
uint8_t I2CsendBuffer[2] ;

// bit is 0 to 3 for coprocessor
void setExternalOutput( uint8_t bit, uint8_t value )
{
	uint8_t oldValue = ExternalBits ;
	if ( value )
	{
		ExternalBits |= 1 << bit ;
	}
	else
	{
		ExternalBits &= ~(1 << bit) ;
	}
	if ( ExternalBits != oldValue )
	{
#ifdef PCBSKY
 #ifndef REVX
		I2CsendBuffer[0] = 0x75 ;
		I2CsendBuffer[1] = ExternalBits ;
		write_coprocessor( (uint8_t *) &I2CsendBuffer, 2 ) ;
 #endif
#endif
	}
}


#ifndef REVX
void write_coprocessor( uint8_t *ptr, uint32_t count )
{
	Co_proc_write_ptr = ptr ;
	Co_proc_write_count = count ;
	CoProc_write_pending = 1 ;
	NVIC_DisableIRQ(TWI0_IRQn) ;
	i2c_check_for_request() ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
}	

void appgo_coprocessor()
{
	CoProc_appgo_pending = 1 ;
	NVIC_DisableIRQ(TWI0_IRQn) ;
	i2c_check_for_request() ;
	NVIC_EnableIRQ(TWI0_IRQn) ;
}	
#endif

//
//
//void setVolume( register uint8_t volume )
//{
////	PMC->PMC_PCER0 |= 0x00080000L ;		// Enable peripheral clock to TWI0
	
//	if ( volume >= NUM_VOL_LEVELS )
//	{
//		volume = NUM_VOL_LEVELS - 1 ;		
//	}
//	volume = Volume_scale[volume] ;

//	__disable_irq() ;
//	if ( TWI0->TWI_IMR & TWI_IMR_TXCOMP )
//	{
//		Volume_required = volume ;
//	}
//	else
//	{
//		TWI0->TWI_MMR = 0x002F0000 ;		// Device 5E (>>1) and master is writing
//		TwiDevice = TWI_VOLUME ;
//		Twi_mode = TWI_MODE_WRITE ;
//		TWI0->TWI_THR = volume ;		// Send data
//		TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
//		TWI0->TWI_IER = TWI_IER_TXCOMP ;
//	}
//	__enable_irq() ;
//}


#ifndef SIMU
extern "C" void TWI0_IRQHandler()
{
	uint32_t status ;
	status = TWI0->TWI_SR ;		// Read only once, some bits cleared on read
	if ( TwiOperation == TWI_READ_VOL )
	{
		if ( status & TWI_SR_RXRDY )
		{
			*Twi_read_address = TWI0->TWI_RHR ;		// Read data
		}
	}

#ifndef REVX
	if ( TwiOperation == TWI_READ_COPROC )
	{
		if ( status & TWI_SR_RXBUFF )
		{
			TWI0->TWI_IDR = TWI_IDR_RXBUFF ;
			TwiOperation = TWI_WAIT_STOP ;
			TWI0->TWI_CR = TWI_CR_STOP ;	// Stop Rx
			TWI0->TWI_RCR = 1 ;						// Last byte
			return ;
		}
		else
		{
			// must be TXCOMP, prob. NAK in data
			// Check if from the bootloader?
			if ( TWI0->TWI_RCR < COPROC_RX_BUXSIZE - 1 )
			{
				// We got at least 1 byte
				if ( Co_proc_status[0] & 0x80 )			// Bootloader
				{
					CoProc_appgo_pending = 1 ;	// Action application
				}
			}
			else
			{
				Coproc_valid = -1 ;			
				TWI0->TWI_CR = TWI_CR_STOP ;	// Stop Rx
			}
		}
	}		
#endif

#ifdef REVX
	if ( TwiOperation == TWI_READ_RTC )
	{
		if ( status & TWI_SR_RXBUFF )
		{
			TWI0->TWI_IDR = TWI_IDR_RXBUFF ;
			TwiOperation = TWI_WAIT_RTCSTOP ;
			TWI0->TWI_CR = TWI_CR_STOP ;	// Stop Rx
			TWI0->TWI_RCR = 1 ;						// Last byte
			return ;
		}
		else
		{
			// must be TXCOMP, prob. NAK in data
			if ( TWI0->TWI_RCR > 0 )
			{
				Rtc_valid = -1 ;			
				TWI0->TWI_CR = TWI_CR_STOP ;	// Stop Rx
			}
		}
	}		
#endif
			 
#ifndef REVX
	if ( TwiOperation == TWI_WAIT_STOP )
	{
		Coproc_valid = 1 ;
		Coproc_read = Co_proc_status[0] ;
		if ( Coproc_read & 0x80 )			// Bootloader
		{
			CoProc_appgo_pending = 1 ;	// Action application
		}
		else
		{ // Got data from tiny app
			// Set the date and time
			t_time *p = &Time ;

			p->second = Co_proc_status[1] ;
			p->minute = Co_proc_status[2] ;
			p->hour = Co_proc_status[3] ;
			p->date= Co_proc_status[4] ;
			p->month = Co_proc_status[5] ;
			p->year = Co_proc_status[6] + ( Co_proc_status[7] << 8 ) ;
		}
		TWI0->TWI_PTCR = TWI_PTCR_RXTDIS ;	// Stop transfers
		if ( status & TWI_SR_RXRDY )
		{
			(void) TWI0->TWI_RHR ;			// Discard any rubbish data
		}
	}
#endif

#ifdef REVX
	if ( TwiOperation == TWI_WAIT_RTCSTOP )
	{
		// Set the date and time
		t_time *p = &Time ;

		if ( Rtc_status[6] )
		{
			Rtc_valid = 1 ;
			p->second = fromBCD( Rtc_status[0] & 0x7F ) ;
			p->minute = fromBCD( Rtc_status[1] & 0x7F ) ;
			p->hour = fromBCD( Rtc_status[2] & 0x3F ) ;
			p->date = fromBCD( Rtc_status[4] & 0x3F ) ;
			p->month = fromBCD( Rtc_status[5] & 0x1F ) ;
			p->year = fromBCD( Rtc_status[6] ) + 2000 ;
		}
		
		TWI0->TWI_PTCR = TWI_PTCR_RXTDIS ;	// Stop transfers
		if ( status & TWI_SR_RXRDY )
		{
			(void) TWI0->TWI_RHR ;			// Discard any rubbish data
		}
	}
#endif

//	if ( TwiOperation == TWI_WRITE_VOL )
//	{
		
//	}

#ifndef REVX
	if ( TwiOperation == TWI_WRITE_COPROC )
	{
		if ( status & TWI_SR_TXBUFE )
		{
			TWI0->TWI_IDR = TWI_IDR_TXBUFE ;
			TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
			TWI0->TWI_PTCR = TWI_PTCR_TXTDIS ;	// Stop transfers
			TwiOperation = TWI_NONE ;
			return ;
		}
	}
#endif

	if ( TwiOperation == TWI_WRITE_ONE )
	{
		I2cCurrentPointer->done = 1 ;
	}

	if ( TwiOperation == TWI_READ_ONE )
	{
		if ( status & TWI_SR_RXRDY )
		{
			*I2cCurrentPointer->dataBuffer = TWI0->TWI_RHR ;		// Read data
			I2cCurrentPointer->done = 1 ;
		}
		else
		{
			I2cCurrentPointer->done = 2 ;
		}
	}

	if ( TwiOperation == TWI_WRITE_BUFFER )
	{
		if ( status & TWI_SR_TXBUFE )
		{
			TWI0->TWI_IDR = TWI_IDR_TXBUFE ;
			TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
			TWI0->TWI_PTCR = TWI_PTCR_TXTDIS ;	// Stop transfers
			TwiOperation = TWI_NONE ;
			I2cCurrentPointer->done = 1 ;
			return ;
		}
		else
		{
			TWI0->TWI_IDR = TWI_IDR_TXBUFE ;
			TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
			TWI0->TWI_PTCR = TWI_PTCR_TXTDIS ;	// Stop transfers
			TwiOperation = TWI_NONE ;
			I2cCurrentPointer->done = 1 ;
			return ;
		}
	}

	if ( TwiOperation == TWI_READ_BUFFER )
	{
		if ( status & TWI_SR_RXBUFF )
		{
			TWI0->TWI_IDR = TWI_IDR_RXBUFF ;
			TwiOperation = TWI_WAIT_BUFFER ;
			TWI0->TWI_CR = TWI_CR_STOP ;	// Stop Rx
			TWI0->TWI_RCR = 1 ;						// Last byte
			return ;
		}
		else
		{
			// must be TXCOMP, prob. NAK in data
			if ( TWI0->TWI_RCR > 0 )
			{
				TWI0->TWI_CR = TWI_CR_STOP ;	// Stop Rx
		  }
			else
			{
				TwiOperation = TWI_NONE ;
				I2cCurrentPointer->done = 1 ;
				I2cCurrentPointer = (struct t_I2C_request *) NULL ;
			}
		}
	}
	
	if ( TwiOperation == TWI_WAIT_BUFFER )
	{
		TWI0->TWI_PTCR = TWI_PTCR_RXTDIS ;	// Stop transfers
		if ( status & TWI_SR_RXRDY )
		{
			(void) TWI0->TWI_RHR ;			// Discard any rubbish data
		}
		TwiOperation = TWI_NONE ;
		I2cCurrentPointer->done = 1 ;
		I2cCurrentPointer = (struct t_I2C_request *) NULL ;
	}		
//			}
//			else
//			{
//				TWI0->TWI_PTCR = TWI_PTCR_RXTDIS ;	// Stop transfers
//				TwiOperation = TWI_NONE ;
//				I2cHeadPointer->done = 1 ;
//				I2cHeadPointer = (struct t_I2C_request *) NULL ;
//			}
//		}
		
//	}
	 
	if ( status & TWI_SR_NACK )
	{
		(void) TWI0->TWI_RHR ;
		uint32_t save = TWI0->TWI_CWGR ;
		TWI0->TWI_CR = TWI_CR_SWRST ;				// Reset in case we are restarting
		TWI0->TWI_CWGR = save ;
		TWI0->TWI_CR = TWI_CR_MSEN | TWI_CR_SVDIS ;		// Master mode enable
	}

	TWI0->TWI_IDR = TWI_IDR_TXCOMP | TWI_IDR_TXBUFE | TWI_IDR_RXBUFF ;
	TWI0->TWI_PTCR = TWI_PTCR_TXTDIS | TWI_PTCR_RXTDIS ;	// Stop transfers
	if ( ( status & TWI_SR_TXCOMP ) == 0 )
	{
		TWI0->TWI_IER = TWI_IER_TXCOMP ;
		TwiOperation = TWI_WAIT_COMP ;
		return ;
	}

	TwiOperation = TWI_NONE ;
	CoProcTimer = 0 ;		// Cancel timeout
	i2c_check_for_request() ;
	
}
#endif


void hapticOff()
{
	PWM->PWM_DIS = PWM_DIS_CHID2 ;						// Disable channel 2
	PWM->PWM_OOV &= ~0x00040000 ;	// Force low
	PWM->PWM_OSS |= 0x00040000 ;	// Force low
}

// pwmPercent 0-100
void hapticOn( uint32_t pwmPercent )
{
	register Pwm *pwmptr ;

	pwmptr = PWM ;

	if ( pwmPercent > 100 )
	{
		pwmPercent = 100 ;		
	}
	pwmptr->PWM_CH_NUM[2].PWM_CDTYUPD = pwmPercent ;		// Duty
	pwmptr->PWM_ENA = PWM_ENA_CHID2 ;						// Enable channel 2
	pwmptr->PWM_OSC = 0x00040000 ;	// Enable output
}

// Code to file a tone buffer at 32kHz
#if 0

static uint8_t toneIndex ;
static uint16_t toneTime ;

static uint16_t toneCount ;

void beginToneFill()
{
	toneIndex = 0 ;
	toneCount = 0 ;
}

uint16_t Sine16k[32] =
{
//	2048,2268,2471,2605,2648,2605,2471,2268,
//	2048,1826,1623,1490,1448,1490,1623,1826
	2048,2165,2278,2381,2472,2547,2602,2636,
	2648,2636,2602,2547,2472,2381,2278,2165,
	2048,1931,1818,1715,1624,1549,1494,1460,
	1448,1460,1494,1549,1624,1715,1818,1931
} ;

// This for 16kHz sample rate
// Returns +ve or zero, remaining time
//   -ve offset into buffer of blank time
int32_t xtoneFill( uint16_t *buffer, uint32_t frequency, uint32_t timeMs )
{
	uint32_t i = 0 ;
	uint32_t y ;

	while ( i < 512 )
	{
		toneCount += frequency ;
		y = ( toneCount & 0x01F0) >> 4 ;
		*buffer++ = Sine16k[y] ;
		i += 1 ;
		if ( --timeMs == 0 )
		{
			break ;
		}
	}
	y = i ;
	if ( i )
	{ // sine finished, but not buffer
		while ( i < 512 )
		{
	  	*buffer++ = 2048 ;
			i += 1 ;
		}
	}
	return timeMs > 0 ? timeMs : -(int32_t)y ;
}

// returns non-zero when finished
uint32_t toneFill( uint16_t *buffer, uint32_t frequency, uint32_t timeMs )
{
	int32_t x = 13115 / frequency ;
	uint32_t y = 100 ;
	uint32_t i = 0 ;
	int32_t step = 0 ;
	timeMs *= 32 ;

	while ( i < 512 )
	{
		step += 1250 ;
		while ( step > 0 )
		{
			step -= x ;
			y += 1 ;
		}
		if ( y > 99 )
		{
			y -= 100 ;
		}
		*buffer++ = Sine_values[y] ;
		i += 1 ;
		timeMs -= 1 ;
		if ( timeMs == 0 )
		{
			break ;
		}
	}
	if ( i )
	{ // We have reached the time, but not filled the buffer
		// or ended the sine wave at 0 necessarily
		while ( i < 512 )
		{
			step += 1250 ;
			while ( step > 0 )
			{
				step -= x ;
				y += 1 ;
			}
			if ( y > 99 )
			{
				y -= 100 ;
				*buffer++ = 2048 ;
				break ;
			}
			else
			{
				*buffer++ = Sine_values[y] ;
			}
			i += 1 ;
		}
	}
	else
	{
		toneIndex = y ;
		toneTime = timeMs ;
		return 0 ;
	}
	if ( i )
	{ // sine finished, but not buffer
		
	}
	else
	{ // buffer finished, maybe not sine
		
	}

	return 0 ;

}


#endif


