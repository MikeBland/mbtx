/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdlib.h>
#include "ersky9x.h"
#include "stm32f2xx.h"
#include "logicio.h"
#include "X9D/hal.h"
#include "X9D/stm32f2xx_usart.h"
#include "stm32f2xx_rcc.h"
#include "stm32f2xx_gpio.h"
#include "core_cm3.h"
#include "timers.h"
#include "myeeprom.h"
#include "drivers.h"
#include "frsky.h"

#define CONVERT_PTR(x) ((uint32_t)(uint64_t)(x))

#define NUM_MODULES 2

#define INTERNAL_RF_ON()      GPIO_SetBits(GPIOPWRINT, PIN_INT_RF_PWR)
#define INTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWRINT, PIN_INT_RF_PWR)
#define EXTERNAL_RF_ON()      GPIO_SetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#define EXTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR)

uint8_t s_current_protocol[NUM_MODULES] = { 255, 255 } ;

extern void setupPulses(unsigned int port);
void setupPulsesPPM(unsigned int port);
//void setupPulsesPXX(unsigned int port);

uint16_t *ppmStreamPtr[NUM_MODULES];
uint16_t ppmStream[NUM_MODULES+1][20];
uint16_t pxxStream[NUM_MODULES][400];
uint16_t dsm2Stream[400];

static void init_pa10_pxx( void ) ;
static void disable_pa10_pxx( void ) ;
static void init_pa10_ppm( void ) ;
static void disable_pa10_ppm( void ) ;
static void init_pa7_pxx( void ) ;
static void disable_pa7_pxx( void ) ;
static void init_pa7_dsm2( void ) ;
static void disable_pa7_dsm2( void ) ;
static void init_pa7_ppm( void ) ;
static void disable_pa7_ppm( void ) ;
static void init_pa7_multi( void ) ;
#ifdef PCB9XT
static void init_pa10_dsm2( void ) ;
static void disable_pa10_dsm2( void ) ;
static void init_pa10_multi( void ) ;
#endif
#ifdef ASSAN
static void init_pa7_assan( void ) ;
static void disable_pa7_assan( void ) ;
#endif
static void init_pa10_none( void ) ;
static void disable_pa10_none( void ) ;
static void init_pa7_none( void ) ;
static void disable_pa7_none( void ) ;

void init_pxx(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    init_pa10_pxx() ;
  else
    init_pa7_pxx() ;
}

void disable_pxx(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    disable_pa10_pxx() ;
  else
    disable_pa7_pxx() ;
}

void init_dsm2(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    init_pa7_dsm2() ;
  }
#ifdef PCB9XT
	else
	{
    init_pa10_dsm2() ;
	}
#endif
}

void init_multi(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    init_pa7_multi() ;
  }
#ifdef PCB9XT
	else
	{
    init_pa10_multi() ;
	}
#endif
}

void disable_dsm2(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    disable_pa7_dsm2();
  }
#ifdef PCB9XT
	else
	{
    disable_pa10_dsm2();
	}
#endif
}

#ifdef ASSAN
void init_assan(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    init_pa7_assan() ;
  }
}

void disable_assan(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    disable_pa7_assan() ;
  }
}
#endif

void init_ppm(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    init_pa10_ppm(); // TODO needed?
  else
    init_pa7_ppm();
}

void disable_ppm(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    disable_pa10_ppm(); // TODO needed?
  else
    disable_pa7_ppm();
}

void init_no_pulses(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    init_pa10_none();
  else
    init_pa7_none();
}

void disable_no_pulses(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    disable_pa10_none();
  else
    disable_pa7_none();
}

static void init_pa10_none()
{
  INTERNAL_RF_OFF();

  // Timer1, channel 3
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock

	configure_pins( PIN_INTPPM_OUT, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
  
  GPIO_SetBits(GPIO_INTPPM, PIN_INTPPM_OUT) ; // Set high
  
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;            // Enable clock

  TIM1->CR1 &= ~TIM_CR1_CEN ;
  TIM1->ARR = 36000 ;             // 18mS
  TIM1->CCR2 = 32000 ;            // Update time
  TIM1->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
  
  TIM1->CCER = TIM_CCER_CC3E ;
  
  TIM1->CCMR2 = 0 ;
  TIM1->EGR = 1 ;                                                         // Restart

  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0 ;                     // Toggle CC1 o/p
  TIM1->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM1->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM1_CC_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM1_CC_IRQn) ;
}

static void disable_pa10_none()
{
  NVIC_DisableIRQ(TIM1_CC_IRQn) ;
  TIM1->DIER &= ~TIM_DIER_CC2IE ;
  TIM1->CR1 &= ~TIM_CR1_CEN ;
}

static void init_pa7_none()
{
  EXTERNAL_RF_OFF() ;

  // Timer8

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
  
	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;

  GPIO_SetBits(GPIOA, PIN_EXTPPM_OUT) ; // Set high
  
  RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock

  TIM8->CR1 &= ~TIM_CR1_CEN ;
  TIM8->ARR = 36000 ;             // 18mS
  TIM8->CCR2 = 32000 ;            // Update time
  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
  
  TIM8->CCMR2 = 0 ;
  TIM8->EGR = 1 ;                                                         // Restart

  TIM8->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0 ;                     // Toggle CC1 o/p
  TIM8->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM8->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
}

static void disable_pa7_none()
{
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  TIM8->DIER &= ~TIM_DIER_CC2IE ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
}

static void init_pa10_pxx()
{
  INTERNAL_RF_ON();

  // Timer1, channel 3
  setupPulsesPXX(0) ; // TODO not here!

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIO_INTPPM, ENABLE);
  
	configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
  
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;            // Enable clock
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;            // Enable DMA2 clock

  TIM1->CR1 &= ~TIM_CR1_CEN ;
  TIM1->ARR = 18000 ;                     // 9mS
  TIM1->CCR2 = 15000 ;            // Update time
  TIM1->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
  
  TIM1->CCER = TIM_CCER_CC3E ;
  
  TIM1->CR2 = TIM_CR2_OIS3 ;              // O/P idle high
  TIM1->BDTR = TIM_BDTR_MOE ;             // Enable outputs
  TIM1->CCR3 = pxxStream[INTERNAL_MODULE][0];
  TIM1->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_0 ;                     // Force O/P high
  TIM1->EGR = 1 ;                                                         // Restart

//      TIM1->SR &= ~TIM_SR_UIF ;             // Clear flag
//      TIM1->SR &= ~TIM_SR_CC2IF ;           // Clear flag
  
  DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
	
	TIM1->DIER |= TIM_DIER_CC3DE ;          		// Enable DMA on CC3 match
  TIM1->DCR = 15 ;                            // DMA to CC1

//      TIM1->CR1 = TIM_CR1_OPM ;                               // Just run once
  // Enable the DMA channel here, DMA2 stream 6, channel 6
  DMA2->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6 ; // Write ones to clear bits
  DMA2_Stream6->CR = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
                                                         | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
  DMA2_Stream6->PAR = CONVERT_PTR(&TIM1->DMAR);
  DMA2_Stream6->M0AR = CONVERT_PTR(&pxxStream[INTERNAL_MODULE][1]);
//      DMA2_Stream2->FCR = 0x05 ; //DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0 ;
//      DMA2_Stream2->NDTR = 100 ;
  DMA2_Stream6->CR |= DMA_SxCR_EN ;               // Enable DMA

  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0 ;                     // Toggle CC1 o/p
  TIM1->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM1->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM1_CC_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM1_CC_IRQn) ;
}

static void disable_pa10_pxx()
{
  DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  NVIC_DisableIRQ(TIM1_CC_IRQn) ;
  TIM1->DIER &= ~TIM_DIER_CC2IE ;
  TIM1->CR1 &= ~TIM_CR1_CEN ;
  INTERNAL_RF_OFF();
}

// PPM output
// Timer 1, channel 1 on PA8 for prototype
// Pin is AF1 function for timer 1
static void init_pa10_ppm()
{
  INTERNAL_RF_ON();
  // Timer1
//  setupPulsesPPM(INTERNAL_MODULE) ;
  ppmStreamPtr[INTERNAL_MODULE] = ppmStream[INTERNAL_MODULE];

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock

  configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
  
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;            // Enable clock

  TIM1->CR1 &= ~TIM_CR1_CEN ;
  TIM1->ARR = *ppmStreamPtr[INTERNAL_MODULE]++ ;
  TIM1->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
  
  TIM1->CCER = TIM_CCER_CC3E ;
  
  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 ;     // PWM mode 1
  TIM1->CCMR1 = TIM_CCMR1_OC2PE ;                   			// PWM mode 1
  TIM1->CCR3 = (g_model.ppmDelay*50+300)*2;
  TIM1->BDTR = TIM_BDTR_MOE ;
  TIM1->EGR = 1 ;
  TIM1->DIER = TIM_DIER_UDE ;

  TIM1->SR &= ~TIM_SR_UIF ;                               // Clear flag
  TIM1->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  TIM1->DIER |= TIM_DIER_CC2IE ;
  TIM1->DIER |= TIM_DIER_UIE ;

  TIM1->CR1 = TIM_CR1_CEN ;
	NVIC_SetPriority( TIM1_CC_IRQn, 3 ) ; // Lower priority interrupt
	NVIC_SetPriority( TIM1_UP_TIM10_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM1_CC_IRQn) ;
  NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn) ;
}

static void disable_pa10_ppm()
{
  NVIC_DisableIRQ(TIM1_CC_IRQn) ;
  NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn) ;
  TIM1->DIER &= ~TIM_DIER_CC2IE & ~TIM_DIER_UIE ;
  TIM1->CR1 &= ~TIM_CR1_CEN ;

  INTERNAL_RF_OFF();
}

#ifdef PCB9XT
static void init_pa10_dsm2()
{
  INTERNAL_RF_ON() ;
}

static void init_pa10_multi()
{
	init_pa10_dsm2() ;	
}

static void disable_pa10_dsm2()
{
	
  INTERNAL_RF_OFF() ;
}
#endif

extern uint16_t g_timePXX;
extern "C" void TIM1_CC_IRQHandler()
{
  uint16_t t0 = TIM3->CNT;
  TIM1->DIER &= ~TIM_DIER_CC2IE ;         // stop this interrupt
  TIM1->SR &= ~TIM_SR_CC2IF ;                             // Clear flag

//#ifdef REV9E
//	s_current_protocol[INTERNAL_MODULE] = PROTO_OFF ;
//#endif
  DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
	setupPulses(0) ;

  if (s_current_protocol[INTERNAL_MODULE] == PROTO_PXX) {
//    DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
    DMA2->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6 ; // Write ones to clear bits
    DMA2_Stream6->M0AR = CONVERT_PTR(&pxxStream[INTERNAL_MODULE][1]);
    DMA2_Stream6->CR |= DMA_SxCR_EN ;               // Enable DMA
    TIM1->CCR3 = pxxStream[INTERNAL_MODULE][0];
    TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
  else if (s_current_protocol[INTERNAL_MODULE] == PROTO_PPM) {
    ppmStreamPtr[INTERNAL_MODULE] = ppmStream[INTERNAL_MODULE];
    TIM1->DIER |= TIM_DIER_UDE ;
    TIM1->SR &= ~TIM_SR_UIF ;                                       // Clear this flag
    TIM1->DIER |= TIM_DIER_UIE ;                            // Enable this interrupt
  }
  else {
    TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
  t0 = TIM3->CNT - t0;
	g_timePXX = t0 ;

}

extern "C" void TIM1_UP_TIM10_IRQHandler()
{
#ifdef PCB9XT
  if ( ( TIM1->DIER & TIM_DIER_UIE ) && ( TIM1->SR & TIM_SR_UIF ) )
	{
#endif
 		TIM1->SR &= ~TIM_SR_UIF ;                               // Clear flag

 		TIM1->ARR = *ppmStreamPtr[INTERNAL_MODULE]++ ;
 		if ( *ppmStreamPtr[INTERNAL_MODULE] == 0 )
 		{
 		  TIM1->SR &= ~TIM_SR_CC2IF ;                     // Clear this flag
 		  TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
 		}
#ifdef PCB9XT
	}
	else
	{
		// must be timer 10
extern void timer10_interrupt() ;
		timer10_interrupt() ;
	}
#endif
}


static void init_pa7_pxx()
{
  EXTERNAL_RF_ON();

  // Timer8
  setupPulsesPXX(EXTERNAL_MODULE);

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
#if defined(REV3)
  configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
#else
  configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_3 | PIN_OS25 | PIN_PUSHPULL ) ;
#endif
  RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;            // Enable DMA2 clock

  TIM8->CR1 &= ~TIM_CR1_CEN ;
  TIM8->ARR = 18000 ;                     // 9mS
  TIM8->CCR2 = 15000 ;            // Update time
  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
#if defined(REV3)
  TIM8->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
#else
#ifdef PCB9XT
  TIM8->CCER = TIM_CCER_CC1NE | TIM_CCER_CC1NP ;
#else
  TIM8->CCER = TIM_CCER_CC1NE ;
#endif
#endif
  TIM8->CR2 = TIM_CR2_OIS1 ;                      // O/P idle high
  TIM8->BDTR = TIM_BDTR_MOE ;             // Enable outputs
  TIM8->CCR1 = pxxStream[EXTERNAL_MODULE][0] ;
  TIM8->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0 ;                     // Force O/P high
  TIM8->EGR = 1 ;                                                         // Restart

//      TIM8->SR &= ~TIM_SR_UIF ;                               // Clear flag
//      TIM8->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC1DE ;          // Enable DMA on CC1 match
  TIM8->DCR = 13 ;                                                                // DMA to CC1

//      TIM8->CR1 = TIM_CR1_OPM ;                               // Just run once
  // Enable the DMA channel here, DMA2 stream 2, channel 7
  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2 ; // Write ones to clear bits
  DMA2_Stream2->CR = DMA_SxCR_CHSEL_0 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
                                                         | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
  DMA2_Stream2->PAR = CONVERT_PTR(&TIM8->DMAR);
  DMA2_Stream2->M0AR = CONVERT_PTR(&pxxStream[EXTERNAL_MODULE][1]);
//      DMA2_Stream2->FCR = 0x05 ; //DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0 ;
//      DMA2_Stream2->NDTR = 100 ;
  DMA2_Stream2->CR |= DMA_SxCR_EN ;               // Enable DMA

  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;                     // Toggle CC1 o/p
  TIM8->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM8->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
}

static void disable_pa7_pxx()
{
  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  TIM8->DIER &= ~TIM_DIER_CC2IE ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF();
}

#ifdef ASSAN
static void init_pa7_assan()
{
	x9dSPortInit( 115200, SPORT_MODE_HARDWARE, 0, 0 ) ;
  EXTERNAL_RF_ON();
	setupPulsesDsm2(6) ;
  
	configure_pins( PIN_EXTPPM_OUT, PIN_INPUT | PIN_PORTA ) ;
		
//	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
//  GPIO_SetBits(GPIOA, PIN_EXTPPM_OUT) ; // Set high
  
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock

  TIM8->CR1 &= ~TIM_CR1_CEN ;
  TIM8->ARR = 22000 ;    // 11mS
  TIM8->CCR2 = 19000 ;   // Update time
  TIM8->CCR1 = 10000 ;   // Tx back on time
  TIM8->CCR3 = 1936*2 ;   // Tx hold on until time
  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;  // 0.5uS from 30MHz
#if defined(REV3)
  TIM8->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
#else
  TIM8->CCER = TIM_CCER_CC1NE ;
#endif
  TIM8->EGR = 0 ;                                                         // Restart
  TIM8->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC2IE | TIM_DIER_CC1IE | TIM_DIER_CC3IE ;  // Enable these interrupts
  TIM8->DIER |= TIM_DIER_UIE ;
  TIM8->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
	NVIC_SetPriority( TIM8_UP_TIM13_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
  NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn) ;
}

static void disable_pa7_assan()
{
  NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn) ;
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  TIM8->DIER &= ~( TIM_DIER_CC2IE | TIM_DIER_CC1IE | TIM_DIER_CC3IE | TIM_DIER_UIE ) ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF() ;
}
#endif

static void init_pa7_dsm2()
{
  EXTERNAL_RF_ON();

//  configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_3 | PIN_OS25 | PIN_PUSHPULL ) ;
  // Timer8
  setupPulsesDsm2(6) ;

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
#if defined(REV3)
  configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
#else
  configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_3 | PIN_OS25 | PIN_PUSHPULL ) ;
#endif
  RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;            // Enable DMA2 clock

  TIM8->CR1 &= ~TIM_CR1_CEN ;
  TIM8->ARR = 44000 ;                     // 22mS
  TIM8->CCR2 = 40000 ;            // Update time
  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
#if defined(REV3)
  TIM8->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
#else
#ifdef PCB9XT
  TIM8->CCER = TIM_CCER_CC1NE | TIM_CCER_CC1NP ;
#else
  TIM8->CCER = TIM_CCER_CC1NE ;
#endif
#endif
  TIM8->CR2 = TIM_CR2_OIS1 ;                      // O/P idle high
  TIM8->BDTR = TIM_BDTR_MOE ;             // Enable outputs
  TIM8->CCR1 = dsm2Stream[0] ;
  TIM8->CCMR1 = TIM_CCMR1_OC1M_2 ; // Force O/P low, hardware inverts it
  TIM8->EGR = 1 ;                                                         // Restart

//      TIM8->SR &= ~TIM_SR_UIF ;                               // Clear flag
//      TIM8->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC1DE ;          // Enable DMA on CC1 match
  TIM8->DCR = 13 ;                                                                // DMA to CC1

//      TIM8->CR1 = TIM_CR1_OPM ;                               // Just run once
  // Enable the DMA channel here, DMA2 stream 2, channel 7
  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2 ; // Write ones to clear bits
  DMA2_Stream2->CR = DMA_SxCR_CHSEL_0 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
                                                         | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
  DMA2_Stream2->PAR = CONVERT_PTR(&TIM8->DMAR);
  DMA2_Stream2->M0AR = CONVERT_PTR(&dsm2Stream[1]);
//      DMA2_Stream2->FCR = 0x05 ; //DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0 ;
//      DMA2_Stream2->NDTR = 100 ;
  DMA2_Stream2->CR |= DMA_SxCR_EN ;               // Enable DMA

  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;                     // Toggle CC1 o/p
  TIM8->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM8->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
}

static void init_pa7_multi()
{
	init_pa7_dsm2() ;
  TIM8->ARR = 22000 ;             // 11mS
  TIM8->CCR2 = 18000 ;            // Update time
	
}

static void disable_pa7_dsm2()
{
  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  TIM8->DIER &= ~TIM_DIER_CC2IE ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF();
}

// PPM output
// Timer 1, channel 1 on PA8 for prototype
// Pin is AF1 function for timer 1
static void init_pa7_ppm()
{
  EXTERNAL_RF_ON();
  // Timer1
//  setupPulsesPpmx() ;
  ppmStreamPtr[EXTERNAL_MODULE] = ppmStream[EXTERNAL_MODULE];

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
#if defined(REV3)
  configure_pins( 0x0100, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
#else
  configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_3 | PIN_OS25 | PIN_PUSHPULL ) ;
#endif
  RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock

  TIM8->CR1 &= ~TIM_CR1_CEN ;
  
  TIM8->ARR = *ppmStreamPtr[EXTERNAL_MODULE]++ ;
  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
#if defined(REV3)
  TIM8->CCER = TIM_CCER_CC1E ;
#else
#ifdef PCB9XT
  TIM8->CCER = TIM_CCER_CC1NE | TIM_CCER_CC1NP ;
#else
  TIM8->CCER = TIM_CCER_CC1NE ;
#endif
  if(!g_model.xpulsePol)
#ifdef PCB9XT
	  TIM8->CCER &= ~TIM_CCER_CC1NP ;
#else
    TIM8->CCER |= TIM_CCER_CC1NP;
#endif
#endif
  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC2PE ;                   // PWM mode 1
  TIM8->CCR1 = (g_model.xppmDelay*50+300)*2 ;
  TIM8->BDTR = TIM_BDTR_MOE ;
  TIM8->EGR = 1 ;
  TIM8->DIER = TIM_DIER_UDE ;

  TIM8->SR &= ~TIM_SR_UIF ;                               // Clear flag
  TIM8->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC2IE ;
  TIM8->DIER |= TIM_DIER_UIE ;

  TIM8->CR1 = TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
	NVIC_SetPriority( TIM8_UP_TIM13_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
  NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn) ;
}

static void disable_pa7_ppm()
{
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn) ;
  TIM8->DIER &= ~TIM_DIER_CC2IE & ~TIM_DIER_UIE ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF();
}

extern "C" void TIM8_CC_IRQHandler()
{
#ifdef ASSAN
	if ( TIM8->DIER & TIM_DIER_CC3IE )
	{
		if ( TIM8->SR & TIM_SR_CC3IF )
		{
			// must be ASSAN disable Tx output
  		uint32_t status;
		  TIM8->SR &= ~TIM_SR_CC3IF ;    // Clear flag
#ifdef PCB9XT
			GPIOB->BSRRH = 0x0004 ;		// output disable
#else
			GPIOD->BSRRH = PIN_SPORT_ON ;		// output disable
#endif
			USART2->CR1 |= USART_CR1_RE ;
  		status = USART2->SR ;
  		while (status & (USART_FLAG_RXNE))
			{
    		status = USART2->DR;
    		status = USART2->SR ;
			}
			dsmTelemetryStartReceive() ;
			return ;
		}
	}
	
	if ( TIM8->DIER & TIM_DIER_CC1IE )
	{
		if ( TIM8->SR & TIM_SR_CC1IF )
		{
			// must be ASSAN re-enable Tx output
		  TIM8->SR &= ~TIM_SR_CC1IF ;    // Clear flag
			USART2->CR1 &= ~USART_CR1_RE ; // Stop receive
#ifdef PCB9XT
			GPIOB->BSRRL = 0x0004 ;		// output disable
#else
			GPIOD->BSRRL = PIN_SPORT_ON ;				 // output enable
#endif
			return ;
		}
	}
#endif
  TIM8->DIER &= ~TIM_DIER_CC2IE ;         // stop this interrupt
  TIM8->SR &= ~TIM_SR_CC2IF ;                             // Clear flag

  setupPulses(EXTERNAL_MODULE) ;

  if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PXX)
	{
    DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
    DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2 ; // Write ones to clear bits
    DMA2_Stream2->M0AR = CONVERT_PTR(&pxxStream[EXTERNAL_MODULE][1]);
    DMA2_Stream2->CR |= DMA_SxCR_EN ;               // Enable DMA
    TIM8->CCR1 = pxxStream[EXTERNAL_MODULE][0];
    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
  else if ( (s_current_protocol[EXTERNAL_MODULE] == PROTO_DSM2 ) || (s_current_protocol[EXTERNAL_MODULE] == PROTO_MULTI ) )
	{
    DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
    DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2 ; // Write ones to clear bits
    DMA2_Stream2->M0AR = CONVERT_PTR(&dsm2Stream[1]);
  	TIM8->CCMR1 = TIM_CCMR1_OC1M_2 ; // Force O/P low, hardware inverts it
	  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;                     // Toggle CC1 o/p
    DMA2_Stream2->CR |= DMA_SxCR_EN ;               // Enable DMA
    TIM8->CCR1 = dsm2Stream[0];
    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
  else if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PPM) {
    ppmStreamPtr[EXTERNAL_MODULE] = ppmStream[EXTERNAL_MODULE];
    TIM8->DIER |= TIM_DIER_UDE ;
    TIM8->SR &= ~TIM_SR_UIF ;                                       // Clear this flag
    TIM8->DIER |= TIM_DIER_UIE ;                            // Enable this interrupt
  }
  else
	{
    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
}

extern "C" void TIM8_UP_TIM13_IRQHandler()
{
  TIM8->SR &= ~TIM_SR_UIF ;                               // Clear flag
#ifdef ASSAN
	if (s_current_protocol[EXTERNAL_MODULE] == PROTO_ASSAN)
	{
		x9dSPortTxStart( (uint8_t *)dsm2Stream, 20, 0 ) ;
	}
	else
#endif
	{
  	TIM8->ARR = *ppmStreamPtr[EXTERNAL_MODULE]++ ;
	  if (*ppmStreamPtr[EXTERNAL_MODULE] == 0)
		{
  	  TIM8->SR &= ~TIM_SR_CC2IF ;                     // Clear this flag
	    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  	}
	}
}


