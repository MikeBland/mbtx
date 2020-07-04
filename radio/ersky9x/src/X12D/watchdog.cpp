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

//#define SW_WATCHDOG		1

#ifdef SW_WATCHDOG
void init_watchdog_timer() ;
#endif

void initWatchdog()
{
	IWDG->KR = 0x5555 ;		// Unlock registers
	IWDG->PR = 3 ;				// Divide by 32 => 1kHz clock
	IWDG->KR = 0x5555 ;		// Unlock registers
#ifdef SW_WATCHDOG
	IWDG->RLR = 1000 ;			// 1.0 seconds nominal
#else
	IWDG->RLR = 500 ;			// 0.5 seconds nominal
#endif
	IWDG->KR = 0xAAAA ;		// reload
	IWDG->KR = 0xCCCC ;		// start
#ifdef SW_WATCHDOG
	init_watchdog_timer() ;
#endif
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

#ifdef SW_WATCHDOG
uint32_t Tim4Counter ;
uint32_t WdStoreOn ;
extern uint32_t ResetReason ;
#endif

void wdt_reset()
{
	IWDG->KR = 0xAAAA ;		// reload
#ifdef SW_WATCHDOG
	Tim4Counter = 0 ;
#endif
}

#ifdef SW_WATCHDOG
#include "ersky9x.h"
#include "ff.h"

FIL WdFile = {0} ;

struct t_wd_dump
{
//	uint16_t t1 ;
//	uint16_t t2 ;
//	uint16_t t3 ;
//	uint16_t t5 ;
//	uint16_t t6 ;
//	uint16_t t7 ;
//	uint16_t t8 ;
//	uint16_t t9 ;
//	uint16_t t11 ;
//	uint16_t t12 ;
//	uint16_t t13 ;
//	uint16_t t14 ;

	uint16_t lastRectx ;
	uint16_t lastRecty ;
	uint16_t lastRectw ;
	uint16_t lastRecth ;
	uint32_t lastFbuf ;
	uint32_t lastAddr ;

	uint32_t omar ;
	uint32_t nlr ;
	uint32_t Dma2dCr ;
	uint32_t stack[9] ;
	
	uint16_t oor ;
	uint16_t t4count ;
	uint16_t hbeat ;
	uint16_t Dma2dSr ;
	uint16_t D2Errors ;
} ;

struct t_wdMemory
{
	uint32_t index ;
	struct t_wd_dump items[700] ;
} ;


struct t_wdMemory WdMemory __CCM ;

void wdDump()
{
  char filename[50] ; // /LOGS/modelnamexxx-2013-01-01.log
  FRESULT result ;
	uint32_t i ;
	if ( ( ResetReason & RCC_CSR_WDGRSTF ) ) // watchdog
	{
		// Store data to file
		struct t_wd_dump *p ;
    cpystr((uint8_t *)filename, (uint8_t *)"/WdDump.txt" ) ;
  	result = f_open(&WdFile, filename, FA_OPEN_ALWAYS | FA_WRITE) ;
		wdt_reset() ;
  	if (result == FR_OK)
		{
			f_printf(&WdFile, "Idx %d\n", WdMemory.index ) ;
  	  for ( i = 0 ; i < 700 ; i += 1 )
			{
				p = &WdMemory.items[i] ;
				f_printf(&WdFile, "%d:\n", i ) ;
//				f_printf(&WdFile, "%04X %04X %04X %04X %04X ", p->t1, p->t2, p->t3, p->t5, p->t6 ) ;
//				f_printf(&WdFile, "%04X %04X %04X %04X ", p->t7, p->t8, p->t9, p->t11 ) ;
//				f_printf(&WdFile, "%04X %04X %04X\n", p->t12, p->t13, p->t14 ) ;

				f_printf(&WdFile, "%04X %04X %04X %04X %08X %08X\n", p->lastRectx,p->lastRecty,p->lastRectw,p->lastRecth,p->lastFbuf,p->lastAddr ) ;
				f_printf(&WdFile, "%08X %08X %04X\n", p->omar, p->nlr, p->oor ) ;
				f_printf(&WdFile, "%04X %04X %08X %04X %04X\n", p->t4count, p->hbeat, p->Dma2dCr, p->Dma2dSr, p->D2Errors ) ;
				f_printf(&WdFile, "%08X %08X %08X ", p->stack[0], p->stack[1], p->stack[2] ) ;
				f_printf(&WdFile, "%08X %08X %08X ", p->stack[3], p->stack[4], p->stack[5] ) ;
				f_printf(&WdFile, "%08X %08X %08X\n\n", p->stack[6], p->stack[7], p->stack[8] ) ;
				wdt_reset() ;
			}
extern uint16_t WdogIntValue ;

			f_printf(&WdFile, "Reason %04X\n\n", ResetReason ) ;
			f_printf(&WdFile, "WdogInt %04X\n\n", WdogIntValue ) ;
			
			f_close(&WdFile) ;
		}
	}
}



void init_watchdog_timer()
{
#if defined(PCBX10)
	if ( ( ResetReason & RCC_CSR_WDGRSTF ) == 0 ) // Not watchdog
	{
		WdStoreOn = 1 ;
		WdMemory.index = 0 ;
	}
#endif	
	// Timer 4
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN ;		// Enable clock
	TIM4->ARR = 999 ;	// 1mS at 1uS pre-scale
	TIM4->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 1000000 - 1 ;		// 1uS
  TIM4->CCER = 0 ;
  TIM4->CCMR1 = 0 ;
	TIM4->EGR = 0 ;
	TIM4->DIER = 1 ;
	TIM4->CR1 = 5 ;

	NVIC_SetPriority(TIM4_IRQn, 0 ) ;
  NVIC_EnableIRQ(TIM4_IRQn) ;
}

extern void dumpStack( uint32_t *ptr ) ;

#if defined(PCBX10)// && defined(PCBREV_EXPRESS)
extern "C" void SDRAM_Init() ;
#endif
extern void lcdInit(void) ;
void lcdColorsInit(void) ;
//uint16_t T4ToCount ;
extern "C" void TIM4_IRQHandler(void)
{
	// A "soft" watchdog
	// Collect information and store in backup memory
	// Max of 20 registers available, use 2 through 19
	// Display on hardware watchdog reboot

  TIM4->SR = 0 ;
	Tim4Counter += 1 ;
	struct t_wd_dump *p ;

	if ( WdStoreOn )
	{
		p = &WdMemory.items[WdMemory.index++] ;
		if ( WdMemory.index >= 700 )
		{
			WdMemory.index = 0 ;
		}
//		p->t1 = TIM1->CNT ;
//		p->t2 = TIM2->CNT ;
//		p->t3 = TIM3->CNT ;
//		p->t5 = TIM5->CNT ;
//		p->t6 = TIM6->CNT ;
//		p->t7 = TIM7->CNT ;
//		p->t8 = TIM8->CNT ;
//		p->t9 = TIM9->CNT ;
//		p->t11 = TIM11->CNT ;
//		p->t12 = TIM12->CNT ;
//		p->t13 = TIM13->CNT ;
//		p->t14 = TIM14->CNT ;
		
//extern uint16_t LastDmaRectx ;
//extern uint16_t LastDmaRecty ;
//extern uint16_t LastDmaRectw ;
//extern uint16_t LastDmaRecth ;
//extern uint32_t LastFrameBuffer ;
//extern uint32_t LastAddr ;
	
//		p->lastRectx = LastDmaRectx ;
//		p->lastRecty = LastDmaRecty ;
//		p->lastRectw = LastDmaRectw ;
//		p->lastRecth = LastDmaRecth ;
//		p->lastFbuf =  LastFrameBuffer ;
//		p->lastAddr =  LastAddr ;
		p->t4count = Tim4Counter ;
		p->hbeat = heartbeat ;
		p->Dma2dCr = DMA2D->CR ;
		p->Dma2dSr = DMA2D->ISR ;
extern uint16_t DMA2DerrorCount ;
		p->D2Errors = DMA2DerrorCount ;
		p->omar = DMA2D->OMAR ;
		p->nlr = DMA2D->NLR ;
		p->oor = DMA2D->OOR ;
		uint32_t *ptr ;
		uint32_t *q ;
		ptr = (uint32_t *) __get_PSP() ;
		ptr += 5 ;	// Skip R0-R3,R12
		q = p->stack ;
		*q++ = (uint32_t) ptr ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
	}

	if ( Tim4Counter > 500 )	// 500mS
	{
		__disable_irq() ;	// Block all future interrupts

		uint32_t *ptr ;
		uint32_t *q ;
		ptr = (uint32_t *) __get_PSP() ;

//		dumpStack( ptr ) ;
 
		q = (uint32_t *) &RTC->BKP2R ;
		*q++ = (uint32_t) ptr ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
		*q++ = *ptr++ ;
//extern uint16_t DMA2DerrorCount ;
//		*q++ = DMA2DerrorCount ;	// BKP18R

//		T4ToCount += 1 ;
//		IWDG->KR = 0xAAAA ;		// reload
//		Tim4Counter = 0 ;
//		__enable_irq() ;
//		SDRAM_Init() ;
//		lcdColorsInit() ;
//		lcdInit() ;
//		AlertMessage = "LCD Reset" ;

//	  if ( T4ToCount > 10 )
//		{
			for(;;)
			{
				// Force watchdog reboot
			}
//		}
	}
}
#endif // SW_WATCHDOG
