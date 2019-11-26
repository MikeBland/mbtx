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

#include <stdint.h>
#include <stdlib.h>

#include <stm32f10x.h>
#ifndef SIMU
 	#include "core_cm3.h"
#endif
#include "../sound.h"
#include "../ersky9x.h"
#include "../myeeprom.h"
#include "../drivers.h"
#include "../audio.h"
#include "stm103/logicio103.h"
#include "../timers.h"
//#include "i2c_ee.h"
#include "stm103/hal.h"

void start_sound( void ) ;
void start_dactimer( void ) ;
void init_dac( void ) ;
extern "C" void DAC_IRQHandler( void ) ;

void end_sound( void ) ;
void setVolume( register uint8_t volume ) ;

extern uint32_t Master_frequency ;
extern uint8_t CurrentVolume ;

struct t_sound_globals Sound_g ;

struct t_VoiceBuffer VoiceBuffer[3] ;

#define SOUND_NONE	0
#define SOUND_TONE	1
#define SOUND_VOICE	2
#define SOUND_STOP	3

struct t_VoiceBuffer *PtrVoiceBuffer[3] ;
uint8_t VoiceCount ;
uint8_t SoundType ;
uint8_t DacIdle ;

// Sound routines

void start_sound()
{
	start_dactimer() ;
	init_dac() ;
}


void set_frequency( uint32_t frequency )
{
	register uint32_t timer ;

	timer = 72000000 / frequency - 1 ;		// MCK/8 and 100 000 Hz
	if ( timer > 65535 )
	{
		timer = 65535 ;		
	}
	if ( timer < 2 )
	{
		timer = 2 ;		
	}
	TIM6->CR1 &= ~TIM_CR1_CEN ;
	TIM6->CNT = 0 ;
	TIM6->ARR = timer ;
	TIM6->CR1 |= TIM_CR1_CEN ;
}

//uint32_t test_sound()
//{
//	start_sound() ;
//	return 0 ;
//}

extern void stop_sound()
{
	DMA2_Channel3->CCR &= ~DMA_CCR1_CIRC ;
}


// Start TIMER6 at 100000Hz, used for DAC trigger
void start_dactimer()
{
	// Now for timer 6
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN ;		// Enable clock
	
	TIM6->PSC = 0 ;													// Max speed
	TIM6->ARR = 72000000 / 100000 - 1 ;	// 10 uS, 100 kHz
	TIM6->CR2 = 0 ;
	TIM6->CR2 = 0x20 ;
	TIM6->CR1 = TIM_CR1_CEN ;
}


// Configure STM32 DAC1 (not DAC2) on PA4
// Use TIMER 6 for trigger/timebase
// DMA1, Stream 5, channel 7
void init_dac()
{
	
	DacIdle = 1 ;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN ; 			// Enable portA clock
	configure_pins( 0x0010, PIN_ANALOG | PIN_PORTA ) ;
	configure_pins( 0x0020, PIN_OP2 | PIN_GP_PP | PIN_PORTA | PIN_HIGH ) ;

	RCC->APB1ENR |= RCC_APB1ENR_DACEN ;				// Enable clock
	RCC->AHBENR |= RCC_AHBENR_DMA2EN ;			// Enable DMA2 clock
	
 	// Chan 7, 16-bit wide, Medium priority, memory increments
	DMA2_Channel3->CCR &= ~DMA_CCR1_EN ;		// Disable DMA
	DMA2->IFCR = DMA_IFCR_CTEIF3 | DMA_IFCR_CHTIF3 | DMA_IFCR_CTCIF3 | DMA_IFCR_CGIF3 ;
	DMA2_Channel3->CCR = DMA_CCR1_MSIZE_0 | DMA_CCR1_PSIZE_0 | DMA_CCR1_MINC | DMA_CCR1_DIR | DMA_CCR1_CIRC ;
	DMA2_Channel3->CPAR = (uint32_t) &DAC->DHR12R1 ;

	DAC->DHR12R1 = 2048 ;
//	DAC->SR = DAC_SR_DMAUDR1 ;		// Write 1 to clear flag
	DAC->CR = DAC_CR_TEN1 | DAC_CR_EN1 | DAC_CR_BOFF1 ;			// Enable DAC
	NVIC_SetPriority( DMA2_Channel3_IRQn, 4 ) ; // Lower priority interrupt
	NVIC_SetPriority( TIM6_IRQn, 4 ) ; // Lower priority interrupt
	NVIC_EnableIRQ(TIM6_IRQn) ;
	NVIC_EnableIRQ(DMA2_Channel3_IRQn) ;
}

#ifndef SIMU
extern "C" void TIM6_IRQHandler()
{
	DAC->CR &= ~DAC_CR_DMAEN1 ;			// Stop DMA requests
//	DAC->CR &= ~DAC_CR_DMAUDRIE1 ;	// Stop underrun interrupt
	DacIdle = 1 ;
//	DAC->SR = DAC_SR_DMAUDR1 ;			// Write 1 to clear flag
}

uint8_t AudioVoiceUnderrun ;
uint8_t AudioVoiceCountUnderruns ;

extern "C" void DMA2_Channel3_IRQHandler()
{
	DMA2_Channel3->CCR &= ~DMA_CCR1_TCIE ;		// Stop interrupt
	DMA2->IFCR = DMA_IFCR_CTEIF3 | DMA_IFCR_CHTIF3 | DMA_IFCR_CTCIF3 | DMA_IFCR_CGIF3 ;
	if ( Sound_g.VoiceActive == 1 )
	{
		PtrVoiceBuffer[0]->flags |= VF_SENT ;		// Flag sent
		PtrVoiceBuffer[0] = PtrVoiceBuffer[1] ;
		PtrVoiceBuffer[1] = PtrVoiceBuffer[2] ;

		VoiceCount -= 1 ;
		if ( VoiceCount == 0 )		// Run out of buffers
		{
			AudioVoiceUnderrun = 1 ;		// For debug
			Sound_g.VoiceActive = 2 ;
//			DMA1_Stream5->CR &= ~DMA_SxCR_TCIE ;			// Disable DMA interrupt
			DMA2_Channel3->CCR &= ~DMA_CCR1_EN ;				// Disable DMA channel
			DacIdle = 1 ;
		}
		else
		{
			DMA2_Channel3->CCR &= ~DMA_CCR1_EN ;				// Disable DMA channel
			DMA2_Channel3->CMAR = (uint32_t) PtrVoiceBuffer[0]->dataw ;
			DMA2_Channel3->CNDTR =  PtrVoiceBuffer[0]->count ;
			DMA2->IFCR = DMA_IFCR_CTEIF3 | DMA_IFCR_CHTIF3 | DMA_IFCR_CTCIF3 | DMA_IFCR_CGIF3 ;
			DMA2_Channel3->CCR |= DMA_CCR1_EN | DMA_CCR1_TCIE ;	// Enable DMA channel
//			DAC->SR = DAC_SR_DMAUDR1 ;			// Write 1 to clear flag
		}
	}
}
#endif


void end_sound()
{
	DAC->CR = 0 ;
	TIM6->CR1 = 0 ;
	// Also need to turn off any possible interrupts
	NVIC_DisableIRQ(TIM6_IRQn) ;
	NVIC_DisableIRQ(DMA2_Channel3_IRQn) ;

}

// Called every 5mS from interrupt routine

void sound_5ms()
{
		if ( Sound_g.VoiceRequest )
		{
			
			Sound_g.Sound_time = 0 ;						// Remove any pending tone requests
			
			if ( DacIdle )	// All sent
			{
				DacIdle = 0 ;
				// Now we can send the voice file
				Sound_g.VoiceRequest = 0 ;
				Sound_g.VoiceActive = 1 ;
				set_frequency( VoiceBuffer[0].frequency ? VoiceBuffer[0].frequency : 16000 ) ;
			
#ifndef SIMU
				DMA2_Channel3->CCR &= ~DMA_CCR1_EN ;				// Disable DMA channel
				DMA2->IFCR = DMA_IFCR_CTEIF3 | DMA_IFCR_CHTIF3 | DMA_IFCR_CTCIF3 | DMA_IFCR_CGIF3 ;
				DMA2_Channel3->CMAR = (uint32_t) VoiceBuffer[0].dataw ;
				DMA2_Channel3->CNDTR =  VoiceBuffer[0].count ;
				DMA2_Channel3->CCR |= DMA_CCR1_EN | DMA_CCR1_TCIE ;		// Enable DMA channel and interrupt
//				DAC->SR = DAC_SR_DMAUDR1 ;			// Write 1 to clear flag
				DAC->CR |= DAC_CR_EN1 | DAC_CR_DMAEN1 ;			// Enable DAC
#endif
			}
			return ;
		}
		
		{
			DMA2_Channel3->CCR &= ~DMA_CCR1_CIRC ;		// Stops DMA at end of cycle
		}
	
}


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
	VoiceBuffer[index].flags &= ~VF_SENT ;
	__disable_irq() ;
	PtrVoiceBuffer[VoiceCount++] = &VoiceBuffer[index] ;
	__enable_irq() ;
	if ( DacIdle )	// All sent
	{
		DacIdle = 0 ;
		Sound_g.VoiceActive = 1 ;
#ifndef SIMU
		DMA2_Channel3->CCR &= ~DAC_CR_EN1 ;				// Disable DMA channel
		DMA2->IFCR = DMA_IFCR_CTEIF3 | DMA_IFCR_CHTIF3 | DMA_IFCR_CTCIF3 | DMA_IFCR_CGIF3 ;
		DMA2_Channel3->CMAR = (uint32_t) VoiceBuffer[index].dataw ;
		DMA2_Channel3->CNDTR =  VoiceBuffer[index].count ;
		DMA2_Channel3->CCR |= DMA_CCR1_EN | DMA_CCR1_TCIE ;		// Enable DMA channel and interrupt
//		DAC->SR = DAC_SR_DMAUDR1 ;			// Write 1 to clear flag
		DAC->CR |= DAC_CR_EN1 | DAC_CR_DMAEN1 ;			// Enable DAC
#endif
	}
}

static const uint8_t Volume_scale[NUM_VOL_LEVELS] = 
{
	 0,  2,  4,   6,   8,  10,  13,  17,  22,  27,  33,  40,
	64, 82, 96, 105, 112, 117, 120, 122, 124, 125, 126, 127 	
} ;

uint8_t ExternalBits ;

// bit is 0 to 3 for coprocessor
//void setExternalOutput( uint8_t bit, uint8_t value )
//{
//	uint8_t oldValue = ExternalBits ;
//	if ( value )
//	{
//		ExternalBits |= 1 << bit ;
//	}
//	else
//	{
//		ExternalBits &= ~(1 << bit) ;
//	}
//	if ( ExternalBits != oldValue )
//	{
//	}
//}


void setVolume( register uint8_t volume )
{
#if !defined(SIMU)
	if ( volume >= NUM_VOL_LEVELS )
	{
		volume = NUM_VOL_LEVELS - 1 ;		
	}
	CurrentVolume = volume ;
	volume = Volume_scale[volume] ;
#endif
}

void initHaptic()
{
}

void hapticOff()
{
}

// pwmPercent 0-100
void hapticOn( uint32_t pwmPercent )
{
}


