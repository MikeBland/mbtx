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

// Pre-scaler divide by 32
// gives max timeout 4096 mS at 32kHz
// Clock may be between 30 and 60 kHz

#include "stm32f4xx.h"

void initWatchdog()
{
	IWDG->KR = 0x5555 ;		// Unlock registers
	IWDG->PR = 3 ;				// Divide by 32 => 1kHz clock
	IWDG->KR = 0x5555 ;		// Unlock registers
	IWDG->RLR = 500 ;			// 0.5 seconds nominal
	IWDG->KR = 0xAAAA ;		// reload
	IWDG->KR = 0xCCCC ;		// start
}

//void init_watchdog_timer() ;
// Allows a time of 1 to 4 seconds
void initLongWatchdog(uint32_t time)
{
	if ( ( time > 4 ) || ( time == 0 ) )
	{
		time = 4 ;
	}
	
	IWDG->KR = 0x5555 ;		// Unlock registers
	IWDG->PR = 4 ;				// Divide by 64 => 500Hz clock
	IWDG->KR = 0x5555 ;		// Unlock registers
	IWDG->RLR = time * 500 ;		// 4.0 seconds nominal
	IWDG->KR = 0xAAAA ;		// reload
	IWDG->KR = 0xCCCC ;		// start
//	init_watchdog_timer() ;
}

uint32_t Tim4Counter ;

void wdt_reset()
{
	IWDG->KR = 0xAAAA ;		// reload
//	Tim4Counter = 0 ;
}

//#include "ersky9x.h"

//void init_watchdog_timer()
//{
//	// Timer13
//	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN ;		// Enable clock
//	TIM4->ARR = 4999 ;	// 600mS at 10uS pre-scale
//	TIM4->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 1000000 - 1 ;		// 1uS
//  TIM4->CCER = 0 ;
//  TIM4->CCMR1 = 0 ;
//	TIM4->EGR = 0 ;
//	TIM4->DIER = 1 ;
//	TIM4->CR1 = 5 ;
  
//	NVIC_SetPriority(TIM4_IRQn, 0 ) ;
//  NVIC_EnableIRQ(TIM4_IRQn) ;
//}

extern void dumpStack( uint32_t *ptr ) ;

//extern "C" void TIM4_IRQHandler(void)
//{
//	// A "soft" watchdog
//	// Collect information and store in backup memory
//	// Max of 20 registers available, use 2 through 19
//	// Display on hardware watchdog reboot

//  TIM4->SR = 0 ;
	
//	if ( ++Tim4Counter > 120 )
//	{
//		__disable_irq() ;	// Block all future interrupts

//		uint32_t *ptr ;
//		uint32_t *q ;
//		ptr = (uint32_t *) __get_PSP() ;

//		dumpStack( ptr ) ;
	 
////		q = (uint32_t *) &RTC->BKP2R ;
////		*q++ = (uint32_t) ptr ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////		*q++ = *ptr++ ;
////extern uint16_t DMA2DerrorCount ;
////		*q++ = DMA2DerrorCount ;	// BKP18R
	 
//		for(;;)
//		{
//			// Force watchdog reboot
//		}
//	}
//}

