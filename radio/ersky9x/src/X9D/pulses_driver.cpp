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
//#ifdef PCBX9LITE
//#define EXTERNAL_RF_ON()	    GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
//#define EXTERNAL_RF_OFF()     GPIO_SetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
//#else
#define EXTERNAL_RF_ON()      GPIO_SetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#define EXTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
//#endif

#define EXT_TYPE_PXX		0
#define EXT_TYPE_DSM		1
#define EXT_TYPE_MULTI	2


uint8_t s_current_protocol[NUM_MODULES] = { 255, 255 } ;

extern void setupPulses(unsigned int port);
void setupPulsesPPM(unsigned int port);
uint8_t setupPulsesXfire() ;
#ifdef ACCESS
uint8_t setupPulsesAccess( uint32_t module ) ;
#endif
//void setupPulsesPXX(unsigned int port);

uint16_t *ppmStreamPtr[NUM_MODULES];
uint16_t pulseStreamCount[NUM_MODULES] ;
uint16_t ppmStream[NUM_MODULES+1][20];
uint16_t pxxStream[NUM_MODULES][400];
uint16_t dsm2Stream[2][400];

#ifdef XFIRE
extern uint8_t Bit_pulses[] ;
uint16_t XfireLength ;
#endif
#ifdef ACCESS
extern uint8_t Bit_pulses[] ;
uint16_t AccessLength ;
#endif

static void init_int_pxx( void ) ;
static void init_ext_pxx( void ) ;
static void disable_ext_pxx( void ) ;
static void disable_int_pxx( void ) ;

#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)

static void init_ext_dsm2( void ) ;
static void disable_ext_dsm2( void ) ;
static void init_ext_ppm( void ) ;
static void disable_ext_ppm( void ) ;
static void init_ext_multi( void ) ;
//static void disable_ext_multi( void ) ;
#ifdef XFIRE
static void init_ext_xfire( void ) ;
static void disable_ext_xfire( void ) ;
#endif
static void init_int_none( void ) ;
static void disable_int_none( void ) ;
static void init_ext_none( void ) ;
static void disable_ext_none( void ) ;

static void init_ext_serial( uint32_t type ) ;

#else
static void init_pa10_ppm( void ) ;
static void disable_pa10_ppm( void ) ;
static void init_ext_dsm2( void ) ;
static void disable_ext_dsm2( void ) ;
static void init_ext_ppm( void ) ;
static void disable_ext_ppm( void ) ;
static void init_ext_multi( void ) ;
#ifdef PCB9XT
static void init_pa10_dsm2( void ) ;
static void disable_pa10_dsm2( void ) ;
static void init_pa10_multi( void ) ;
#endif
#ifdef XFIRE
static void init_pa7_xfire( void ) ;
static void disable_pa7_xfire( void ) ;
#endif
static void init_pa10_none( void ) ;
static void disable_pa10_none( void ) ;
static void init_pa7_none( void ) ;
static void disable_pa7_none( void ) ;
#endif

#ifdef ACCESS
//static void init_pa7_access( uint32_t module ) ;
//void disable_pa7_access( void ) ;
static void init_ext_access( void ) ;
static void init_int_access( void ) ;
#endif

#ifdef ACCESS
void init_access(uint32_t port)
{
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
  if (port == INTERNAL_MODULE)
    init_int_access() ;
  else
    init_ext_access() ;
#else
    init_pa7_access( port ) ;
#endif
}

void disable_access(uint32_t port)
{
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
  if (port == INTERNAL_MODULE)
    disable_int_pxx() ;
  else
    disable_ext_pxx() ;
#else
    disable_pa7_access() ;
#endif
}
#endif

void init_pxx(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    init_int_pxx() ;
  else
    init_ext_pxx() ;
}

void disable_pxx(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    disable_int_pxx() ;
  else
    disable_ext_pxx() ;
}

void init_dsm2(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    init_ext_dsm2() ;
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
    init_ext_multi() ;
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
    disable_ext_dsm2();
  }
#ifdef PCB9XT
	else
	{
    disable_pa10_dsm2();
	}
#endif
}


#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)


#ifdef XFIRE
void init_xfire(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    init_ext_xfire() ;
  }
}

void disable_xfire(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    disable_ext_xfire() ;
  }
}
#endif

void init_ppm(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    init_int_none();
  else
    init_ext_ppm();
}

void disable_ppm(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
    disable_ext_ppm();
}

void init_no_pulses(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    init_int_none();
  else
    init_ext_none();
}

void disable_no_pulses(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    disable_int_none();
  else
    disable_ext_none();
}


#else

#ifdef XFIRE
void init_xfire(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    init_pa7_xfire() ;
  }
}

void disable_xfire(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    disable_pa7_xfire() ;
  }
}
#endif

void init_ppm(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    init_pa10_ppm(); // TODO needed?
  else
		init_ext_ppm() ;
}

void disable_ppm(uint32_t port)
{
  if (port == INTERNAL_MODULE)
    disable_pa10_ppm(); // TODO needed?
  else
    disable_ext_ppm();
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
  TIM1->ARR = 35999 ;             // 18mS
  TIM1->CCR2 = 32000 ;            // Update time
  TIM1->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
  
  TIM1->CCER = TIM_CCER_CC3E ;
  
  TIM1->CCMR2 = 0 ;
  TIM1->EGR = 1 ;                                                         // Restart

  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0 ;                     // Toggle CC1 o/p
  TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
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
  TIM8->ARR = 35999 ;             // 18mS
  TIM8->CCR2 = 32000 ;            // Update time
  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
  
  TIM8->CCMR2 = 0 ;
  TIM8->EGR = 1 ;                                                         // Restart

  TIM8->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0 ;                     // Toggle CC1 o/p
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
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

#define PA10_TYPE_PXX			0
#define PA10_TYPE_DSM			1
#define PA10_TYPE_MULTI		2
#define PA10_TYPE_ACCESS	3

void init_pa10_serial( uint32_t type )
{
  INTERNAL_RF_ON();
	if ( type == PA10_TYPE_PXX )
	{
  	setupPulsesPXX(INTERNAL_MODULE);
	}
	else
	{
#ifdef ACCESS
		if ( type == PA10_TYPE_ACCESS )
		{
  		setupPulsesAccess(INTERNAL_MODULE) ;
			
		}
		else
		{
#endif
  		setupPulsesDsm2(6, INTERNAL_MODULE) ;
#ifdef ACCESS
		}
#endif
	}
  
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIO_INTPPM, ENABLE);
	
	configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
//#ifdef ACCESS
//	configure_pins( INTMODULE_RX_GPIO_PIN, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 ) ;
//#endif

  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;            // Enable clock
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;            // Enable DMA2 clock

  TIM1->CR1 &= ~TIM_CR1_CEN ;
  
	if ( type == PA10_TYPE_PXX )
	{
#ifdef ACCESS
		if ( type == PA10_TYPE_ACCESS )
		{
			TIM1->ARR = 11999 ;                     // 6mS
  		TIM1->CCR2 = 10000 ;            // Update time
		}
		else
#endif
		{
			TIM1->ARR = 17989 ;                     // 9mS - 5uS
  		TIM1->CCR2 = 16000 ;            // Update time
		}	
	}
	else if ( type == PA10_TYPE_DSM )
	{
		TIM1->ARR = 43999 ;                     // 22mS
  	TIM1->CCR2 = 40000 ;            // Update time
	}
	else // type == PA10_TYPE_MULTI
	{
//  	TIM1->ARR = 14000 ;             // 7mS
//  	TIM1->CCR2 = 10000 ;            // Update time
		uint32_t x ;
		x = g_model.Module[0].ppmFrameLength ;
		if ( x > 4 )
		{
			x = 0 ;
		}
		x *= 2000 ;
		x += 7000 * 2 - 1 ;
  	TIM1->ARR = x ;             // 11mS
  	TIM1->CCR2 = x-4000 ;       // Update time
	}
  TIM1->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz

#ifdef PCB9XT
  TIM1->CCER = TIM_CCER_CC3E | TIM_CCER_CC3P ;
#else
  TIM1->CCER = TIM_CCER_CC3E ;
#endif

  TIM1->CR2 = TIM_CR2_OIS3 ;              // O/P idle high
  TIM1->BDTR = TIM_BDTR_MOE ;             // Enable outputs
	if ( type == PA10_TYPE_PXX )
	{
	  TIM1->CCR3 = pxxStream[INTERNAL_MODULE][0];
	  TIM1->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_0 ;                     // Force O/P high
	}
	else
	{
	  TIM1->CCR3 = dsm2Stream[INTERNAL_MODULE][0] ;
	  TIM1->CCMR2 = TIM_CCMR2_OC3M_2 ; // Force O/P low, hardware inverts it
	}
  TIM1->EGR = 1 ;                                                         // Restart

  DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
	
	TIM1->DIER |= TIM_DIER_CC3DE ;          		// Enable DMA on CC3 match
  TIM1->DCR = 15 ;                            // DMA to CC1

  // Enable the DMA channel here, DMA2 stream 6, channel 6
  DMA2->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6 ; // Write ones to clear bits
  DMA2_Stream6->CR = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
                                                         | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
  DMA2_Stream6->PAR = CONVERT_PTR(&TIM1->DMAR);
	if ( type == PA10_TYPE_PXX )
	{
  	DMA2_Stream6->M0AR = CONVERT_PTR(&pxxStream[INTERNAL_MODULE][1]);
	}
	else
	{
  	DMA2_Stream6->M0AR = CONVERT_PTR(&dsm2Stream[0][1]);
	}
  DMA2_Stream6->CR |= DMA_SxCR_EN ;               // Enable DMA

  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0 ;                     // Toggle CC1 o/p
  TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
  TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM1->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM1_CC_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM1_CC_IRQn) ;
}


static void init_int_pxx()
{
	init_pa10_serial( PA10_TYPE_PXX ) ;
}

static void disable_int_pxx()
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
  
#ifdef PCB9XT
  TIM1->CCER = TIM_CCER_CC3E | TIM_CCER_CC3P ;
#else
  TIM1->CCER = TIM_CCER_CC3E ;
#endif
  if(!g_model.Module[0].pulsePol)
#ifdef PCB9XT
	  TIM1->CCER &= ~TIM_CCER_CC3P ;
#else
    TIM1->CCER |= TIM_CCER_CC3P;
#endif
  
  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 ;     // PWM mode 1
  TIM1->CCMR1 = TIM_CCMR1_OC2PE ;                   			// PWM mode 1
  TIM1->CCR3 = (g_model.Module[0].ppmDelay*50+300)*2;
  TIM1->BDTR = TIM_BDTR_MOE ;
  TIM1->EGR = 1 ;
  TIM1->DIER = TIM_DIER_UDE ;

  TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;                               // Clear flag
  TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
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
	init_pa10_serial( PA10_TYPE_DSM ) ;
}

static void init_pa10_multi()
{
	init_pa10_serial( PA10_TYPE_MULTI ) ;
}

static void disable_pa10_dsm2()
{
  DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  NVIC_DisableIRQ(TIM1_CC_IRQn) ;
  TIM1->DIER &= ~TIM_DIER_CC2IE ;
  TIM1->CR1 &= ~TIM_CR1_CEN ;
  INTERNAL_RF_OFF() ;
}
#endif

#ifdef PCBX9D
uint16_t XjtHbeatOffset ;
#endif

//extern uint16_t g_timePXX;
extern "C" void TIM1_CC_IRQHandler()
{
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x81 ;
#endif
//  uint16_t t0 = TIM3->CNT;
  TIM1->DIER &= ~TIM_DIER_CC2IE ;         // stop this interrupt
  TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag

//#ifdef REV9E
//	s_current_protocol[INTERNAL_MODULE] = PROTO_OFF ;
//#endif
//  DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
	setupPulses(INTERNAL_MODULE) ;

  if (s_current_protocol[INTERNAL_MODULE] == PROTO_PXX)
	{
#ifdef PCBX9D
		XjtHbeatOffset = TIM7->CNT - XjtHeartbeatCapture.value ;
		if ( XjtHeartbeatCapture.valid )
		{
			if ( XjtHbeatOffset > 0x2A00 )
//			if ( XjtHbeatOffset > 0x2200 )
			{
				TIM1->ARR = 17979 ;                     // 9mS
			}
			else
			{
				TIM1->ARR = 18019 ;                     // 9mS
			}
		}
#endif
    DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
    DMA2->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6 ; // Write ones to clear bits
    DMA2_Stream6->M0AR = CONVERT_PTR(&pxxStream[INTERNAL_MODULE][1]);
    DMA2_Stream6->CR |= DMA_SxCR_EN ;               // Enable DMA
    TIM1->CCR3 = pxxStream[INTERNAL_MODULE][0];
    TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
  else if ( (s_current_protocol[INTERNAL_MODULE] == PROTO_DSM2 ) || (s_current_protocol[INTERNAL_MODULE] == PROTO_MULTI ) )
	{
    DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
    DMA2->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6 ; // Write ones to clear bits
    DMA2_Stream6->M0AR = CONVERT_PTR(&dsm2Stream[0][1]);
	  TIM1->CCMR2 = TIM_CCMR2_OC3M_2 ; // Force O/P low, hardware inverts it
	  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0 ;                     // Toggle CC1 o/p
    DMA2_Stream6->CR |= DMA_SxCR_EN ;               // Enable DMA
    TIM1->CCR3 = dsm2Stream[0][0];
    TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
  else if (s_current_protocol[INTERNAL_MODULE] == PROTO_PPM)
	{
    ppmStreamPtr[INTERNAL_MODULE] = ppmStream[INTERNAL_MODULE];
    TIM1->DIER |= TIM_DIER_UDE ;
    TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;                                       // Clear this flag
    TIM1->DIER |= TIM_DIER_UIE ;                            // Enable this interrupt
  }
  else {
    TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
//  t0 = TIM3->CNT - t0;
//	g_timePXX = t0 ;

}

extern "C" void TIM1_UP_TIM10_IRQHandler()
{
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x82 ;
#endif
#ifdef PCB9XT
  if ( ( TIM1->DIER & TIM_DIER_UIE ) && ( TIM1->SR & TIM_SR_UIF ) )
	{
#endif
 		TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;                               // Clear flag

 		TIM1->ARR = *ppmStreamPtr[INTERNAL_MODULE]++ ;
 		if ( *ppmStreamPtr[INTERNAL_MODULE] == 0 )
 		{
 		  TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                     // Clear this flag
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

void init_ext_serial( uint32_t type )
{
  EXTERNAL_RF_ON() ;
	if ( type == EXT_TYPE_PXX )
	{
  	setupPulsesPXX(EXTERNAL_MODULE);
	}
	else
	{
  	setupPulsesDsm2(6, EXTERNAL_MODULE) ;
	}
  
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
#if defined(REV3)
  configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
#else
  configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_3 | PIN_OS25 | PIN_PUSHPULL ) ;
#endif
  RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;            // Enable DMA2 clock
  
	TIM8->CR1 &= ~TIM_CR1_CEN ;

	if ( type == EXT_TYPE_PXX )
	{
		TIM8->ARR = 17989 ;                     // 9mS - 5uS
  	TIM8->CCR2 = 16000 ;            // Update time
	}
	else if ( type == EXT_TYPE_DSM )
	{
		TIM8->ARR = 43999 ;                     // 22mS
  	TIM8->CCR2 = 40000 ;            // Update time
	}
	else // type == EXT_TYPE_MULTI
	{
//  	TIM8->ARR = 14000 ;             // 7mS
//  	TIM8->CCR2 = 10000 ;            // Update time
		uint32_t x ;
		x = g_model.Module[1].ppmFrameLength ;
		if ( x > 4 )
		{
			x = 0 ;
		}
		x *= 2000 ;
		x += 7000 * 2 ;
  	TIM8->ARR = x ;             // 11mS
  	TIM8->CCR2 = x-4000 ;       // Update time
	}
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
	if ( type == EXT_TYPE_PXX )
	{
	  TIM8->CCR1 = pxxStream[EXTERNAL_MODULE][0] ;
  	TIM8->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0 ;                     // Force O/P high
	}
	else
	{
	  TIM8->CCR1 = dsm2Stream[EXTERNAL_MODULE][0] ;
	  TIM8->CCMR1 = TIM_CCMR1_OC1M_2 ; // Force O/P low, hardware inverts it
	}
  TIM8->EGR = 1 ;                                                         // Restart
  
	TIM8->DIER |= TIM_DIER_CC1DE ;          // Enable DMA on CC1 match
  TIM8->DCR = 13 ;                                                                // DMA to CC1


  // Enable the DMA channel here, DMA2 stream 2, channel 7
  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2 ; // Write ones to clear bits
  DMA2_Stream2->CR = DMA_SxCR_CHSEL_0 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
																												 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
  
	DMA2_Stream2->PAR = CONVERT_PTR(&TIM8->DMAR);
  
	if ( type == EXT_TYPE_PXX )
	{
//#ifdef X3_PROTO
//  	DMA2_Stream2->M0AR = CONVERT_PTR(&dsm2Stream[1][1]);
//#else
		DMA2_Stream2->M0AR = CONVERT_PTR(&pxxStream[EXTERNAL_MODULE][1]);
//#endif
  }
	else
	{
  	DMA2_Stream2->M0AR = CONVERT_PTR(&dsm2Stream[1][1]);
	}
  
	DMA2_Stream2->CR |= DMA_SxCR_EN ;               // Enable DMA

  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;                     // Toggle CC1 o/p
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM8->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
}

static void init_ext_pxx()
{
	init_ext_serial( EXT_TYPE_PXX ) ;
}

static void disable_ext_pxx()
{
  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  TIM8->DIER &= ~TIM_DIER_CC2IE ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF();
}

#ifdef ACCESS
 #ifdef ACCESS_SPORT
static void init_pa7_access( uint32_t module )
{
	com1_Configure( ACCESS_SPORT_BAUD_RATE, 0, 0 ) ;
  EXTERNAL_RF_ON();
	setupPulsesAccess( module ) ;
  
	configure_pins( PIN_EXTPPM_OUT, PIN_INPUT | PIN_PORTA ) ;
		
//	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
//  GPIO_SetBits(GPIOA, PIN_EXTPPM_OUT) ; // Set high
  
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock

  TIM8->CR1 &= ~TIM_CR1_CEN ;
  TIM8->ARR = 7999 ;    // 4mS
  TIM8->CCR2 = 5000 ;   // Update time
//  TIM8->CCR1 = 10000 ;   // Tx back on time
//  TIM8->CCR3 = 1936*2 ;   // Tx hold on until time
  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;  // 0.5uS from 30MHz
#if defined(REV3)
  TIM8->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
#else
  TIM8->CCER = TIM_CCER_CC1NE ;
#endif
  TIM8->EGR = 0 ;                                                         // Restart
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
//  TIM8->DIER |= TIM_DIER_CC2IE | TIM_DIER_CC1IE | TIM_DIER_CC3IE ;  // Enable these interrupts
  TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM8->DIER |= TIM_DIER_UIE ;
  TIM8->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
	NVIC_SetPriority( TIM8_UP_TIM13_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
  NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn) ;
}

static void disable_pa7_access()
{
  NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn) ;
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  TIM8->DIER &= ~( TIM_DIER_CC2IE | TIM_DIER_CC1IE | TIM_DIER_CC3IE | TIM_DIER_UIE ) ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF() ;
}
 #endif
#endif


#ifdef XFIRE
static void init_pa7_xfire()
{
	com1_Configure( XFIRE_BAUD_RATE, 0, 0 ) ;
  EXTERNAL_RF_ON();
	setupPulsesXfire() ;
  
	configure_pins( PIN_EXTPPM_OUT, PIN_INPUT | PIN_PORTA ) ;
		
//	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
//  GPIO_SetBits(GPIOA, PIN_EXTPPM_OUT) ; // Set high
  
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock

  TIM8->CR1 &= ~TIM_CR1_CEN ;
  TIM8->ARR = 7999 ;    // 4mS
  TIM8->CCR2 = 5000 ;   // Update time
//  TIM8->CCR1 = 10000 ;   // Tx back on time
//  TIM8->CCR3 = 1936*2 ;   // Tx hold on until time
  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;  // 0.5uS from 30MHz
#if defined(REV3)
  TIM8->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
#else
  TIM8->CCER = TIM_CCER_CC1NE ;
#endif
  TIM8->EGR = 0 ;                                                         // Restart
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
//  TIM8->DIER |= TIM_DIER_CC2IE | TIM_DIER_CC1IE | TIM_DIER_CC3IE ;  // Enable these interrupts
  TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM8->DIER |= TIM_DIER_UIE ;
  TIM8->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
	NVIC_SetPriority( TIM8_UP_TIM13_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
  NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn) ;
}

static void disable_pa7_xfire()
{
  NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn) ;
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  TIM8->DIER &= ~( TIM_DIER_CC2IE | TIM_DIER_CC1IE | TIM_DIER_CC3IE | TIM_DIER_UIE ) ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF() ;
}
#endif

static void init_ext_dsm2()
{
	init_ext_serial( EXT_TYPE_DSM ) ;
}

static void init_ext_multi()
{
	init_ext_serial( EXT_TYPE_MULTI ) ;
}

static void disable_ext_dsm2()
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
static void init_ext_ppm()
{
  EXTERNAL_RF_ON();
  // Timer8
//  setupPulsesPpmx() ;
  ppmStreamPtr[EXTERNAL_MODULE] = ppmStream[EXTERNAL_MODULE];

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
#if defined(REV3)
  configure_pins( 0x0100, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
#else
  configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PORT_EXTPPM | PIN_PER_3 | PIN_OS25 | PIN_PUSHPULL ) ;
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
  if(!g_model.Module[1].pulsePol)
#ifdef PCB9XT
	  TIM8->CCER &= ~TIM_CCER_CC1NP ;
#else
    TIM8->CCER |= TIM_CCER_CC1NP;
#endif
#endif
  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC2PE ;                   // PWM mode 1
  TIM8->CCR1 = (g_model.Module[1].ppmDelay*50+300)*2 ;
  TIM8->BDTR = TIM_BDTR_MOE ;
  TIM8->EGR = 1 ;
  TIM8->DIER = TIM_DIER_UDE ;

  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;                               // Clear flag
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC2IE ;
  TIM8->DIER |= TIM_DIER_UIE ;

  TIM8->CR1 = TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
	NVIC_SetPriority( TIM8_UP_TIM13_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
  NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn) ;
}

static void disable_ext_ppm()
{
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn) ;
  TIM8->DIER &= ~TIM_DIER_CC2IE & ~TIM_DIER_UIE ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF();
}

extern "C" void TIM8_CC_IRQHandler()
{
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x83 ;
#endif
  TIM8->DIER &= ~TIM_DIER_CC2IE ;         // stop this interrupt
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag

  setupPulses(EXTERNAL_MODULE) ;

  if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PXX)
	{
#ifdef PCBX9D
	  if (s_current_protocol[INTERNAL_MODULE] != PROTO_PXX)
		{
			XjtHbeatOffset = TIM7->CNT - XjtHeartbeatCapture.value ;
			if ( XjtHeartbeatCapture.valid )
			{
				if ( XjtHbeatOffset > 0x2200 )
				{
					TIM8->ARR = 17979 ;                     // 9mS
				}
				else
				{
					TIM8->ARR = 18019 ;                     // 9mS
				}
			}
		}
#endif
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
    DMA2_Stream2->M0AR = CONVERT_PTR(&dsm2Stream[1][1]);
  	TIM8->CCMR1 = TIM_CCMR1_OC1M_2 ; // Force O/P low, hardware inverts it
	  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;                     // Toggle CC1 o/p
    DMA2_Stream2->CR |= DMA_SxCR_EN ;               // Enable DMA
    TIM8->CCR1 = dsm2Stream[1][0];
    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
  else if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PPM) {
    ppmStreamPtr[EXTERNAL_MODULE] = ppmStream[EXTERNAL_MODULE];
    TIM8->DIER |= TIM_DIER_UDE ;
    TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;                                       // Clear this flag
    TIM8->DIER |= TIM_DIER_UIE ;                            // Enable this interrupt
  }
  else
	{
    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
}

extern "C" void TIM8_UP_TIM13_IRQHandler()
{
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x84 ;
#endif
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;                               // Clear flag
#ifdef XFIRE
	if (s_current_protocol[EXTERNAL_MODULE] == PROTO_XFIRE )
	{
		x9dSPortTxStart( (uint8_t *)Bit_pulses, XfireLength, 0 ) ;
	}
	else
#endif
#ifdef ACCESS
	if (s_current_protocol[EXTERNAL_MODULE] == PROTO_ACCESS )
	{
		x9dSPortTxStart( (uint8_t *)Bit_pulses, AccessLength, 0 ) ;
	}
	else
#endif
	{
  	TIM8->ARR = *ppmStreamPtr[EXTERNAL_MODULE]++ ;
	  if (*ppmStreamPtr[EXTERNAL_MODULE] == 0)
		{
  	  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                     // Clear this flag
	    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  	}
	}
}

#endif

#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)

static void init_int_none()
{
  INTERNAL_RF_OFF() ;

// Timer12
  RCC->APB1ENR |= RCC_APB1ENR_TIM12EN ;           // Enable clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock

  INTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
  INTMODULE_TIMER->ARR = 17989 ;            // 9 mS - 5uS
  INTMODULE_TIMER->CCR2 = 16999 ;           // Update time
  INTMODULE_TIMER->PSC = INTMODULE_TIMER_FREQ / 2000000 - 1; // 0.5uS (2Mhz)

  INTMODULE_TIMER->DIER |= TIM_DIER_CC2IE;  // Enable this interrupt
  INTMODULE_TIMER->CR1 |= TIM_CR1_CEN ;
  NVIC_EnableIRQ(INTMODULE_TIMER_IRQn) ;
  NVIC_SetPriority(INTMODULE_TIMER_IRQn, 3 ) ;
}

static void disable_int_none()
{
  NVIC_DisableIRQ(INTMODULE_TIMER_IRQn) ;
  INTMODULE_TIMER->DIER &= ~TIM_DIER_CC2IE ;
  INTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN ;
}

static void init_ext_none()
{
  EXTERNAL_RF_OFF() ;
		
	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PORT_EXTPPM ) ;
	GPIO_SetBits(GPIOA, PIN_EXTPPM_OUT) ; // Set high
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock
	TIM8->CR1 &= ~TIM_CR1_CEN ;
	TIM8->ARR = 17999 ;             // 9 mS
	TIM8->CCR2 = 16999 ;            // Update time
	TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz

	TIM8->EGR = 1 ;                                                         // Restart

	TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
	TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
	TIM8->CR2 = TIM_CR2_MMS_1 ;			// Update event as trigger output
	TIM8->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
	NVIC_EnableIRQ(TIM8_CC_IRQn) ;

}

static void disable_ext_none()
{
  EXTERNAL_RF_OFF() ;

  TIM8->CR1 &= ~TIM_CR1_CEN ;
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
}


extern uint16_t XjtHbeatOffset ;

// Internal IXJT
// USART1 on PB6 and PB7, Alt Func 7
// External module
// PC6/USART6, Alt Func 8
// TIM3_CH1/AF2, TIM8_CH1/AF3


#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
static void init_int_pxx_access( uint32_t type )
#else
static void init_int_pxx( void )
#endif
{
  INTERNAL_RF_ON() ;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ;     // Enable portB clock
	
	configure_pins( INTMODULE_TX_GPIO_PIN, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB | PIN_PER_7 ) ;
	configure_pins( INTMODULE_RX_GPIO_PIN, PIN_PERIPHERAL | PIN_PORTB | PIN_PER_7 ) ;
  
	RCC->APB1ENR |= RCC_APB1ENR_TIM12EN ;     // Enable clock

  INTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
  INTMODULE_TIMER->ARR = 17989 ;             // 9 mS - 5uS
  INTMODULE_TIMER->CCR2 = 17499 ;            // Update time
  INTMODULE_TIMER->PSC = INTMODULE_TIMER_FREQ / 2000000 - 1; // 0.5uS (2Mhz)

  INTMODULE_TIMER->DIER |= TIM_DIER_CC2IE;  // Enable this interrupt
  INTMODULE_TIMER->CR1 |= TIM_CR1_CEN ;

  // UART config
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN ;		// Enable clock

	INTMODULE_USART->BRR = PeripheralSpeeds.Peri2_frequency / 450000 ;
//	INTMODULE_USART->BRR = PeripheralSpeeds.Peri2_frequency / 115200 ;	// Prototype only
	INTMODULE_USART->CR1 = USART_CR1_UE | USART_CR1_TE ;// | USART_CR1_RE ;
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
#ifdef ACCESS
	if ( type )
	{
		INTMODULE_USART->CR1 |= USART_CR1_RE | USART_CR1_RXNEIE ;
	}
#endif
#endif
	NVIC_SetPriority( INTMODULE_USART_IRQn, 3 ) ; // Quite high priority interrupt
  NVIC_EnableIRQ( INTMODULE_USART_IRQn);

  NVIC_SetPriority(INTMODULE_TIMER_IRQn, 3 ) ;
	NVIC_EnableIRQ(INTMODULE_TIMER_IRQn) ;
}
	
static void disable_int_pxx( void )
{
  NVIC_DisableIRQ(INTMODULE_USART_IRQn);
	NVIC_DisableIRQ(INTMODULE_TIMER_IRQn) ;
}

// External CPPM is on PC6 and has inverted output driver '240
// USART6 can drive this
// TIM3 ch1 or TIM8 ch1
	 
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
static void init_ext_pxx_access( uint32_t type )
#else
static void init_ext_pxx( void )
#endif
{
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
//#ifdef PCBX9LITE
//#ifndef X3_PROTO
  EXTERNAL_RF_ON() ;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ;     // Enable portB clock
	
	configure_pins( EXTMODULE_TX_GPIO_PIN, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PORTC | PIN_PER_8 ) ;
	configure_pins( EXTMODULE_RX_GPIO_PIN, PIN_PERIPHERAL | PIN_PORTC | PIN_PER_8 ) ;
  
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;     // Enable clock

  EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
  EXTMODULE_TIMER->ARR = 17989 ;             // 9 mS - 5uS
  EXTMODULE_TIMER->CCR2 = 17499 ;            // Update time
  EXTMODULE_TIMER->PSC = EXTMODULE_TIMER_FREQ / 2000000 - 1; // 0.5uS (2Mhz)

  EXTMODULE_TIMER->DIER |= TIM_DIER_CC2IE;  // Enable this interrupt
  EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN ;

  // UART config
	RCC->APB2ENR |= RCC_APB2ENR_USART6EN ;		// Enable clock

#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
	EXTMODULE_USART->BRR = PeripheralSpeeds.Peri2_frequency / ( type ? PXX2_EXTERNAL_BAUDRATE : 420000 ) ;
#else
	EXTMODULE_USART->BRR = PeripheralSpeeds.Peri2_frequency / 420000 ;
#endif
//	EXTMODULE_USART->BRR = PeripheralSpeeds.Peri2_frequency / 115200 ;	// Prototype only
	EXTMODULE_USART->CR1 = USART_CR1_UE | USART_CR1_TE ;// | USART_CR1_RE ;
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
#ifdef ACCESS
	if ( type )
	{
		EXTMODULE_USART->CR1 |= USART_CR1_RE ;
	}
#endif
#endif

	NVIC_SetPriority( EXTMODULE_USART_IRQn, 3 ) ; // Quite high priority interrupt
  NVIC_EnableIRQ( EXTMODULE_USART_IRQn);

  NVIC_SetPriority(EXTMODULE_TIMER_CC_IRQn, 3 ) ;
	NVIC_EnableIRQ(EXTMODULE_TIMER_CC_IRQn) ;
//#else
//	init_ext_serial( EXT_TYPE_PXX ) ;
//#endif
#else
	init_ext_serial( EXT_TYPE_PXX ) ;
#endif
}

#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
static void init_ext_access( void )
{
	init_ext_pxx_access( 1 ) ;
	
}

static void init_ext_pxx( void )
{
	init_ext_pxx_access( 0 ) ;
}

static void init_int_access( void )
{
	init_int_pxx_access( 1 ) ;
	
}

static void init_int_pxx( void )
{
	init_int_pxx_access( 0 ) ;
}

#endif
	 
static void disable_ext_pxx( void )
{
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
//#ifdef PCBX9LITE
//#ifndef X3_PROTO
  NVIC_DisableIRQ(EXTMODULE_USART_IRQn);
	NVIC_DisableIRQ(EXTMODULE_TIMER_CC_IRQn) ;
//#else
//  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
//  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
//  TIM8->DIER &= ~TIM_DIER_CC2IE ;
//  TIM8->CR1 &= ~TIM_CR1_CEN ;
//  EXTERNAL_RF_OFF();
//#endif
#else
  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  TIM8->DIER &= ~TIM_DIER_CC2IE ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF();
#endif
}
	
static void init_ext_dsm2( void )
{
	init_ext_serial( EXT_TYPE_DSM ) ;
}
	
static void disable_ext_dsm2( void )
{
  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  TIM8->DIER &= ~TIM_DIER_CC2IE ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF();
}
	
static void init_ext_ppm( void )
{
  EXTERNAL_RF_ON();
  // Timer8
//  setupPulsesPpmx() ;
  ppmStreamPtr[EXTERNAL_MODULE] = ppmStream[EXTERNAL_MODULE];

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ;           // Enable portC clock
  configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PORT_EXTPPM | PIN_PER_3 | PIN_OS25 | PIN_PUSHPULL ) ;
  
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
  TIM8->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
#endif
  if(!g_model.Module[EXTERNAL_MODULE].pulsePol)
#ifdef PCB9XT
	  TIM8->CCER &= ~TIM_CCER_CC1NP ;
#else
    TIM8->CCER &= ~TIM_CCER_CC1P;
#endif
#endif
  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC2PE ;                   // PWM mode 1
  TIM8->CCR1 = (g_model.Module[EXTERNAL_MODULE].ppmDelay*50+300)*2 ;
  TIM8->BDTR = TIM_BDTR_MOE ;
  TIM8->EGR = 1 ;
  TIM8->DIER = TIM_DIER_UDE ;

  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;                               // Clear flag
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC2IE ;
  TIM8->DIER |= TIM_DIER_UIE ;

  TIM8->CR1 = TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
	NVIC_SetPriority( TIM8_UP_TIM13_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
  NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn) ;
}

static void disable_ext_ppm( void )
{
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn) ;
  TIM8->DIER &= ~TIM_DIER_CC2IE & ~TIM_DIER_UIE ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF();
}

void init_ext_serial( uint32_t type )
{
  EXTERNAL_RF_ON() ;
	if ( type == EXT_TYPE_PXX )
	{
  	setupPulsesPXX(EXTERNAL_MODULE);
	}
	else
	{
  	setupPulsesDsm2(6, EXTERNAL_MODULE) ;
	}
  
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
#if defined(REV3)
  configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
#else
  configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PORT_EXTPPM | PIN_PER_3 | PIN_OS25 | PIN_PUSHPULL ) ;
#endif
  RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;            // Enable DMA2 clock
  
	TIM8->CR1 &= ~TIM_CR1_CEN ;

  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
	if ( type == EXT_TYPE_PXX )
	{
//#ifdef X3_PROTO
//	  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2500000 - 1 ;               // 0.5uS from 30MHz
//#endif
		TIM8->ARR = 22499 ;                     // 9mS
  	TIM8->CCR2 = 20000 ;            // Update time
	}
	else if ( type == EXT_TYPE_DSM )
	{
		TIM8->ARR = 43999 ;                     // 22mS
  	TIM8->CCR2 = 40000 ;            // Update time
	}
	else // type == EXT_TYPE_MULTI
	{
//  	TIM8->ARR = 14000 ;             // 7mS
//  	TIM8->CCR2 = 10000 ;            // Update time
		uint32_t x ;
		x = g_model.Module[1].ppmFrameLength ;
		if ( x > 4 )
		{
			x = 0 ;
		}
		x *= 2000 ;
		x += 7000 * 2 ;
  	TIM8->ARR = x ;             // 11mS
  	TIM8->CCR2 = x-4000 ;       // Update time
	}
#if defined(REV3)
  TIM8->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
#else
#ifdef PCB9XT
  TIM8->CCER = TIM_CCER_CC1NE | TIM_CCER_CC1NP ;
#else
  TIM8->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
#endif
#endif
  TIM8->CR2 = TIM_CR2_OIS1 ;                      // O/P idle high
  TIM8->BDTR = TIM_BDTR_MOE ;             // Enable outputs
	if ( type == EXT_TYPE_PXX )
	{
	  TIM8->CCR1 = pxxStream[EXTERNAL_MODULE][0] ;
  	TIM8->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0 ;                     // Force O/P high
	}
	else
	{
	  TIM8->CCR1 = dsm2Stream[EXTERNAL_MODULE][0] ;
    TIM8->CCER &= ~TIM_CCER_CC1P;
	  TIM8->CCMR1 = TIM_CCMR1_OC1M_2 ; // Force O/P low, hardware inverts it
	}
  TIM8->EGR = 1 ;                                                         // Restart
  
	TIM8->DIER |= TIM_DIER_CC1DE ;          // Enable DMA on CC1 match
  TIM8->DCR = 13 ;                                                                // DMA to CC1


  // Enable the DMA channel here, DMA2 stream 2, channel 7
  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2 ; // Write ones to clear bits
  DMA2_Stream2->CR = DMA_SxCR_CHSEL_0 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
																												 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
  
	DMA2_Stream2->PAR = CONVERT_PTR(&TIM8->DMAR);
  
	if ( type == EXT_TYPE_PXX )
	{
		DMA2_Stream2->M0AR = CONVERT_PTR(&pxxStream[EXTERNAL_MODULE][1]);
  }
	else
	{
  	DMA2_Stream2->M0AR = CONVERT_PTR(&dsm2Stream[1][1]);
	}
  
	DMA2_Stream2->CR |= DMA_SxCR_EN ;               // Enable DMA

  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;                     // Toggle CC1 o/p
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
  TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM8->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
}


static void init_ext_multi( void )
{
	init_ext_serial( EXT_TYPE_MULTI ) ;
}
	
//static void disable_ext_multi( void )
//{
//  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
//  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
//  TIM8->DIER &= ~TIM_DIER_CC2IE ;
//  TIM8->CR1 &= ~TIM_CR1_CEN ;
//  EXTERNAL_RF_OFF();
//}
	
#ifdef XFIRE
static void init_ext_xfire( void )
{
	com1_Configure( XFIRE_BAUD_RATE, 0, 0 ) ;
  EXTERNAL_RF_ON();
	setupPulsesXfire() ;
  
  configure_pins( PIN_EXTPPM_OUT, PIN_INPUT | PORT_EXTPPM ) ;
		
//	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
//  GPIO_SetBits(GPIOA, PIN_EXTPPM_OUT) ; // Set high
  
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock

  TIM8->CR1 &= ~TIM_CR1_CEN ;
  TIM8->ARR = 7999 ;    // 4mS
  TIM8->CCR2 = 5000 ;   // Update time
//  TIM8->CCR1 = 10000 ;   // Tx back on time
//  TIM8->CCR3 = 1936*2 ;   // Tx hold on until time
  TIM8->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;  // 0.5uS from 30MHz
#if defined(REV3)
  TIM8->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
#else
  TIM8->CCER = TIM_CCER_CC1E ;
#endif
  TIM8->EGR = 0 ;                                                         // Restart
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
//  TIM8->DIER |= TIM_DIER_CC2IE | TIM_DIER_CC1IE | TIM_DIER_CC3IE ;  // Enable these interrupts
  TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  TIM8->DIER |= TIM_DIER_UIE ;
  TIM8->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
	NVIC_SetPriority( TIM8_UP_TIM13_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
  NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn) ;
}

static void disable_ext_xfire( void )
{
  NVIC_DisableIRQ(TIM8_UP_TIM13_IRQn) ;
  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
  TIM8->DIER &= ~( TIM_DIER_CC2IE | TIM_DIER_CC1IE | TIM_DIER_CC3IE | TIM_DIER_UIE ) ;
  TIM8->CR1 &= ~TIM_CR1_CEN ;
  EXTERNAL_RF_OFF() ;
}
#endif

	 
//extern uint8_t PxxSerial[] ;
volatile uint8_t *PxxTxPtr ;
volatile uint8_t PxxTxCount ;
volatile uint8_t *PxxTxPtr_x ;
volatile uint8_t PxxTxCount_x ;

uint16_t XjtHbeatOffset ;

extern "C" void TIM8_BRK_TIM12_IRQHandler()
{
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x85 ;
#endif
	uint16_t status = INTMODULE_TIMER->SR ;
  if ( ( INTMODULE_TIMER->DIER & TIM_DIER_UIE ) && ( status & TIM_SR_UIF ) )
	{
	  INTMODULE_TIMER->DIER &= ~TIM_DIER_UIE ;		// Disable this interrupt
//	  INTMODULE_TIMER->SR = INTMODULE_TIMER_SR_MASK & ~TIM_SR_CC2IF ;     // Clear this flag
//	  INTMODULE_TIMER->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
//		INTMODULE_USART->CR1 |= USART_CR1_TXEIE ;		// Enable this interrupt
		if (s_current_protocol[INTERNAL_MODULE] == PROTO_PXX )
		{
			if ( XjtHeartbeatCapture.valid )
			{
				XjtHbeatOffset = TIM7->CNT - XjtHeartbeatCapture.value ;
				if ( XjtHeartbeatCapture.valid )
				{
		//			if ( XjtHbeatOffset > 0x2A00 )
					if ( XjtHbeatOffset > 17000 )
					{
						INTMODULE_TIMER->ARR = 17979 ;                     // 9mS
					}
					else
					{
						INTMODULE_TIMER->ARR = 18019 ;                     // 9mS
					}
				}
			}
		}
	}
  if ( ( INTMODULE_TIMER->DIER & TIM_DIER_CC2IE ) && ( status & TIM_SR_CC2IF ) )
	{
  	INTMODULE_TIMER->DIER &= ~TIM_DIER_CC2IE ;         // stop this interrupt
  	setupPulses(INTERNAL_MODULE) ;
		if (s_current_protocol[INTERNAL_MODULE] == PROTO_PXX )
		{
//			PxxTxPtr = PxxSerial ;
//			PxxTxCount = pulseStreamCount[INTERNAL_MODULE] ;
		  INTMODULE_TIMER->SR = INTMODULE_TIMER_SR_MASK & ~TIM_SR_CC2IF ;     // Clear this flag
		  INTMODULE_TIMER->SR = INTMODULE_TIMER_SR_MASK & ~TIM_SR_UIF ;	     // Clear this flag
			INTMODULE_TIMER->DIER |= TIM_DIER_UIE ;		 // Enable this interrupt
		}	
		else
		{
		  INTMODULE_TIMER->SR = INTMODULE_TIMER_SR_MASK & ~TIM_SR_CC2IF ;     // Clear this flag
			INTMODULE_TIMER->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
		}
	}
}

#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_FE | USART_FLAG_PE)

extern "C" void INTMODULE_USART_IRQHandler()
{
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x86 ;
#endif
  uint32_t status;
	status = INTMODULE_USART->SR ;
	if ( ( status & USART_SR_TXE ) && (INTMODULE_USART->CR1 & USART_CR1_TXEIE ) )
	{
		if ( PxxTxCount )
		{
			INTMODULE_USART->DR = *PxxTxPtr++ ;
			PxxTxCount -= 1 ;
		}
		else
		{
			INTMODULE_USART->CR1 &= ~USART_CR1_TXEIE ;	// Stop Complete interrupt
		}
	}
#ifdef ACCESS
  if (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS))
	{
		uint16_t value = INTMODULE_USART->DR ;
		value |= getTmr2MHz() & 0xFF00 ;
		put_fifo128( &Access_int_fifo, value ) ;	
	}
#endif
}

extern "C" void EXTMODULE_USART_IRQHandler()
{
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x87 ;
#endif
  uint32_t status;
  
	USART_TypeDef *puart = USART6 ;
	status = puart->SR ;
	
	if ( ( status & USART_SR_TXE ) && (puart->CR1 & USART_CR1_TXEIE ) )
	{
		if ( PxxTxCount_x )
		{
			EXTMODULE_USART->DR = *PxxTxPtr_x++ ;
			PxxTxCount_x -= 1 ;
		}
		else
		{
			EXTMODULE_USART->CR1 &= ~USART_CR1_TXEIE ;	// Stop Complete interrupt
		}
	}

  if (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS))
	{
#ifdef ACCESS
		if (s_current_protocol[EXTERNAL_MODULE] == PROTO_ACCESS )
		{
			uint16_t value = puart->DR ;
			value |= getTmr2MHz() & 0xFF00 ;
			put_fifo128( &Access_ext_fifo, value ) ;	
		}
		else
		{
			put_fifo64( &Sbus_fifo, puart->DR ) ;	
		}
#else
		put_fifo64( &Sbus_fifo, puart->DR ) ;	
#endif
	}
}


extern "C" void TIM8_CC_IRQHandler()
{
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x88 ;
#endif
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
//#ifndef X3_PROTO
//#ifdef PCBX9LITE
	if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PXX )
	{
		uint16_t status = EXTMODULE_TIMER->SR ;
  	if ( ( EXTMODULE_TIMER->DIER & TIM_DIER_UIE ) && ( status & TIM_SR_UIF ) )
		{
		  EXTMODULE_TIMER->DIER &= ~TIM_DIER_UIE ;		// Disable this interrupt
		
			if ( XjtHeartbeatCapture.valid )
			{
				XjtHbeatOffset = TIM7->CNT - XjtHeartbeatCapture.value ;
				if ( XjtHeartbeatCapture.valid )
				{
		//			if ( XjtHbeatOffset > 0x2A00 )
					if ( XjtHbeatOffset > 17000 )
					{
						EXTMODULE_TIMER->ARR = 17979 ;                     // 9mS
					}
					else
					{
						EXTMODULE_TIMER->ARR = 18019 ;                     // 9mS
					}
				}
			}
  	}
	
		if ( ( EXTMODULE_TIMER->DIER & TIM_DIER_CC2IE ) && ( status & TIM_SR_CC2IF ) )
		{
  		EXTMODULE_TIMER->DIER &= ~TIM_DIER_CC2IE ;         // stop this interrupt
  		setupPulses(EXTERNAL_MODULE) ;
			if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PXX )
			{
	//			PxxTxPtr = PxxSerial ;
	//			PxxTxCount = pulseStreamCount[INTERNAL_MODULE] ;
			  EXTMODULE_TIMER->SR = EXTMODULE_TIMER_SR_MASK & ~TIM_SR_CC2IF ;     // Clear this flag
			  EXTMODULE_TIMER->SR = EXTMODULE_TIMER_SR_MASK & ~TIM_SR_UIF ;	     // Clear this flag
				EXTMODULE_TIMER->DIER |= TIM_DIER_UIE ;		 // Enable this interrupt
			}	
			else
			{
			  EXTMODULE_TIMER->SR = EXTMODULE_TIMER_SR_MASK & ~TIM_SR_CC2IF ;     // Clear this flag
				EXTMODULE_TIMER->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
			}
		}
		return ;
	}
//#endif // X3_PROTO
#endif // X3
  TIM8->DIER &= ~TIM_DIER_CC2IE ;         // stop this interrupt
	TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag

  setupPulses(EXTERNAL_MODULE) ;

#ifndef PCBX9LITE
 #ifndef PCBXLITE
  #ifndef REV19
  if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PXX)
	{
 #ifdef PCBX9D
	  if (s_current_protocol[INTERNAL_MODULE] != PROTO_PXX)
		{
			XjtHbeatOffset = TIM7->CNT - XjtHeartbeatCapture.value ;
			if ( XjtHeartbeatCapture.valid )
			{
				if ( XjtHbeatOffset > 0x2200 )
				{
					TIM8->ARR = 17979 ;                     // 9mS
				}
				else
				{
					TIM8->ARR = 18019 ;                     // 9mS
				}
			}
		}
 #endif
    DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
    DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2 ; // Write ones to clear bits
    DMA2_Stream2->M0AR = CONVERT_PTR(&pxxStream[EXTERNAL_MODULE][1]);
    DMA2_Stream2->CR |= DMA_SxCR_EN ;               // Enable DMA
    TIM8->CCR1 = pxxStream[EXTERNAL_MODULE][0];
    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
  else
	#endif // REV19
 #endif // XLite
#endif // nX3
//#ifdef X3_PROTO
//  if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PXX)
//	{
//    DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
//    DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2 ; // Write ones to clear bits
//    DMA2_Stream2->M0AR = CONVERT_PTR(&dsm2Stream[1][1]);
//    DMA2_Stream2->CR |= DMA_SxCR_EN ;               // Enable DMA
//  	TIM8->CCMR1 = TIM_CCMR1_OC1M_2 ;                     // Force O/P high
//	  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;                     // Toggle CC1 o/p
//    TIM8->CCR1 = dsm2Stream[1][0];
//    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
//  }
//#endif // X3_PROTO
	if ( (s_current_protocol[EXTERNAL_MODULE] == PROTO_DSM2 ) || (s_current_protocol[EXTERNAL_MODULE] == PROTO_MULTI ) )
	{
    DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
    DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2 ; // Write ones to clear bits
    DMA2_Stream2->M0AR = CONVERT_PTR(&dsm2Stream[1][1]);
  	TIM8->CCMR1 = TIM_CCMR1_OC1M_2 ; // Force O/P low, hardware inverts it
	  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;                     // Toggle CC1 o/p
    DMA2_Stream2->CR |= DMA_SxCR_EN ;               // Enable DMA
    TIM8->CCR1 = dsm2Stream[1][0];
    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
  else if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PPM) {
    ppmStreamPtr[EXTERNAL_MODULE] = ppmStream[EXTERNAL_MODULE];
    TIM8->DIER |= TIM_DIER_UDE ;
    TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;                                       // Clear this flag
    TIM8->DIER |= TIM_DIER_UIE ;                            // Enable this interrupt
  }
  else
	{
    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
}

extern "C" void TIM8_UP_TIM13_IRQHandler()
{
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x89 ;
#endif
  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;                               // Clear flag
#ifdef XFIRE
	if (s_current_protocol[EXTERNAL_MODULE] == PROTO_XFIRE )
	{
		x9dSPortTxStart( (uint8_t *)Bit_pulses, XfireLength, 0 ) ;
	}
	else
#endif
#ifdef ACCESS
	if (s_current_protocol[EXTERNAL_MODULE] == PROTO_ACCESS )
	{
		x9dSPortTxStart( (uint8_t *)Bit_pulses, AccessLength, 0 ) ;
	}
	else
#endif
	{
  	TIM8->ARR = *ppmStreamPtr[EXTERNAL_MODULE]++ ;
	  if (*ppmStreamPtr[EXTERNAL_MODULE] == 0)
		{
  	  TIM8->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                     // Clear this flag
	    TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  	}
	}
}


#endif

