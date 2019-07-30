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
#include "stm32f4xx.h"
#include "X12D/hal.h"
#include "X12D/stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"
#include "logicio.h"
#include "core_cm4.h"
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
uint8_t setupPulsesXfire() ;
//void setupPulsesPXX(unsigned int port);

uint16_t *ppmStreamPtr[NUM_MODULES];
uint16_t ppmStream[NUM_MODULES+1][20];
uint16_t pxxStream[NUM_MODULES][400];
uint16_t dsm2Stream[2][400];
uint16_t pulseStreamCount[NUM_MODULES] ;


extern void setupPulsesPpmAll(uint32_t module) ;

#ifdef XFIRE
extern uint8_t Bit_pulses[] ;
extern uint16_t XfireLength ;
#endif

#define EXT_TYPE_PXX		0
#define EXT_TYPE_DSM		1
#define EXT_TYPE_MULTI	2


static void init_int_pxx( void ) ;
static void disable_int_pxx( void ) ;
static void init_ext_pxx( void ) ;
static void disable_ext_pxx( void ) ;
static void init_ext_dsm2( void ) ;
static void disable_ext_dsm2( void ) ;
static void init_ext_ppm( void ) ;
static void disable_ext_ppm( void ) ;
static void init_ext_multi( void ) ;
static void disable_ext_multi( void ) ;
#ifdef XFIRE
static void init_ext_xfire( void ) ;
static void disable_ext_xfire( void ) ;
#endif
static void init_int_none( void ) ;
static void disable_int_none( void ) ;
static void init_ext_none( void ) ;
static void disable_ext_none( void ) ;

static void init_ext_serial( uint32_t type ) ;

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
}

void init_multi(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    init_ext_multi() ;
  }
}

void disable_dsm2(uint32_t port)
{
  if (port == EXTERNAL_MODULE)
	{
    disable_ext_dsm2();
  }
}

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

  // Timer8

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock

//	if ( isProdVersion() )
//	{
		configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
	  GPIO_SetBits(GPIOA, PIN_EXTPPM_OUT) ; // Set high
	  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;            // Enable clock
	  TIM1->CR1 &= ~TIM_CR1_CEN ;
	  TIM1->ARR = 17989 ;             // 9 mS - 5uS
	  TIM1->CCR2 = 16999 ;            // Update time
	  TIM1->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz

	  TIM1->EGR = 1 ;                                                         // Restart

	  TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
	  TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
		TIM1->CR2 = TIM_CR2_MMS_1 ;			// Update event as trigger output
	  TIM1->CR1 |= TIM_CR1_CEN ;
		NVIC_SetPriority( TIM1_CC_IRQn, 3 ) ; // Lower priority interrupt
	  NVIC_EnableIRQ(TIM1_CC_IRQn) ;
		
//	}
#ifdef PCBX12D
	if ( isProdVersion() == 0 )
	{
		configure_pins( PROT_PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
	  GPIO_SetBits(GPIOA, PROT_PIN_EXTPPM_OUT) ; // Set high
	  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN ;            // Enable clock
	  TIM2->CR1 &= ~TIM_CR1_CEN ;
		TIM2->SMCR = TIM_SMCR_SMS_2 ;
	  TIM2->ARR = 99999 ;             // 18mS
//	  TIM2->CCR2 = 32000 ;            // Update time
	  TIM2->PSC = (PeripheralSpeeds.Peri1_frequency * PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ;               // 0.5uS from 30MHz

//	  TIM2->EGR = 1 ;                                                         // Restart

//	  TIM2->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
//	  TIM2->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
	  TIM2->CR1 |= TIM_CR1_CEN ;
//		NVIC_SetPriority( TIM2_IRQn, 3 ) ; // Lower priority interrupt
//	  NVIC_EnableIRQ(TIM2_IRQn) ;
	}
#endif
}

static void disable_ext_none()
{
  EXTERNAL_RF_OFF() ;

  TIM1->CR1 &= ~TIM_CR1_CEN ;
  NVIC_DisableIRQ(TIM1_CC_IRQn) ;
#ifdef PCBX12D
	if ( isProdVersion() == 0 )
	{
	  TIM2->CR1 &= ~TIM_CR1_CEN ;
	}
#endif
}

static void init_int_pxx( void )
{
  INTERNAL_RF_ON() ;

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ;     // Enable portB clock
	configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB | PIN_PER_7 ) ;
	configure_pins( INTMODULE_RX_GPIO_PIN, PIN_PERIPHERAL | PIN_PORTB | PIN_PER_7 ) ;

  RCC->APB1ENR |= RCC_APB1ENR_TIM12EN ;     // Enable clock

  INTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
  INTMODULE_TIMER->ARR = 17989 ;             // 9 mS - 5uS
  INTMODULE_TIMER->CCR2 = 16999 ;            // Update time
  INTMODULE_TIMER->PSC = INTMODULE_TIMER_FREQ / 2000000 - 1; // 0.5uS (2Mhz)

  INTMODULE_TIMER->DIER |= TIM_DIER_CC2IE;  // Enable this interrupt
  INTMODULE_TIMER->CR1 |= TIM_CR1_CEN ;

  // UART config
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN ;		// Enable clock

	INTMODULE_USART->BRR = PeripheralSpeeds.Peri2_frequency / 115200 ;
	INTMODULE_USART->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE ;

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

//static void init_ext_pxx( void )
//{
//  EXTERNAL_RF_OFF() ;
	
//}

static void init_ext_dsm2( void )
{
	init_ext_serial( EXT_TYPE_DSM ) ;
}

static void disable_ext_dsm2( void )
{
	EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ; // Disable DMA
  EXTMODULE_TIMER->DIER &= ~(TIM_DIER_CC2IE) ;
 	EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN ;
  NVIC_DisableIRQ( EXTMODULE_DMA_IRQn) ;
 	NVIC_DisableIRQ( EXTMODULE_TIMER_IRQn) ;
#ifdef PCBX12D
	if ( isProdVersion() == 0 )
	{
  	PROT_EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
	}
#endif
}

//static void init_pa7_dsm2()
//{
//	init_pa7_serial( PA7_TYPE_DSM ) ;
//}

//static void init_pa7_multi()
//{
//	init_pa7_serial( PA7_TYPE_MULTI ) ;
//}

// For prototype, PPM pulses as 32-bit
//uint32_t ProtoPpmPulses[20] ;

//void convertPpmPulsesProto()
//{
//	uint16_t *p = ppmStream[EXTERNAL_MODULE] ;
//	uint32_t *pd = ProtoPpmPulses ;
//	uint32_t i ;
//	uint32_t j ;
//	j = pulseStreamCount[EXTERNAL_MODULE] ;
//	if ( j > 20 )
//	{
//		j = 20 ;
//	}
//	for ( i = 0 ; i < j ; i += 1 )
//	{
//		*pd++ = *p++ ;
//	}
//}

static void init_ext_ppm( void )
{
//  EXTERNAL_RF_OFF() ;
  EXTERNAL_RF_ON() ;

	setupPulsesPpmAll(EXTERNAL_MODULE) ;

//	if ( isProdVersion() )
//	{
  	ppmStreamPtr[EXTERNAL_MODULE] = ppmStream[EXTERNAL_MODULE];
		// Configure output pin (TIM1)
  	configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
  	EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop timer
  	EXTMODULE_TIMER->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ; // 0.5uS (2Mhz)
//  	EXTMODULE_TIMER->PSC = 65535 ;
	  EXTMODULE_TIMER->ARR = 44999 ;
  	EXTMODULE_TIMER->CCR2 = 42999 ;
    EXTMODULE_TIMER->CCER = TIM_CCER_CC3E | (g_model.Module[1].pulsePol ? TIM_CCER_CC3P : 0 ) ;
  	EXTMODULE_TIMER->CCR3 = (g_model.Module[1].ppmDelay*50+300)*2 ;
  	EXTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 ; // PWM mode 1
  	EXTMODULE_TIMER->BDTR = TIM_BDTR_MOE;
  	EXTMODULE_TIMER->EGR = 1; // Reloads register values now
		EXTMODULE_TIMER->CR2 = TIM_CR2_MMS_1 ;			// Update event as trigger output

//		EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ; // Disable DMA
//    EXTMODULE_DMA_STREAM->CR |= EXTMODULE_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PL_0 | DMA_SxCR_PL_1 ;
//    EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&EXTMODULE_TIMER->ARR) ;
//    EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(ppmStream[EXTERNAL_MODULE]) ;
//    EXTMODULE_DMA_STREAM->NDTR = pulseStreamCount[EXTERNAL_MODULE] ;
//    EXTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE ; // Enable DMA
//	  NVIC_SetPriority( EXTMODULE_DMA_IRQn, 3) ;
//	  NVIC_EnableIRQ( EXTMODULE_DMA_IRQn) ;

		// Interrupt version
	  EXTMODULE_TIMER->SR = EXTMODULE_TIMER_SR_MASK & ~TIM_SR_UIF ;                               // Clear flag
//  	EXTMODULE_TIMER->DIER = TIM_DIER_UDE; // Update DMA request
//  	EXTMODULE_TIMER->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
  	EXTMODULE_TIMER->DIER |= TIM_DIER_UIE ;
	
  	EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN; // Start timer
	
		NVIC_SetPriority( TIM1_UP_TIM10_IRQn, 3 ) ; // Lower priority interrupt
		NVIC_SetPriority( EXTMODULE_TIMER_IRQn, 3 ) ; // Lower priority interrupt
  	NVIC_EnableIRQ( TIM1_UP_TIM10_IRQn) ;
  	NVIC_EnableIRQ( EXTMODULE_TIMER_IRQn) ;
//	}
#ifdef PCBX12D
	if ( isProdVersion() == 0 )
	{
//		convertPpmPulsesProto() ;
//  	ppmStreamPtr[EXTERNAL_MODULE] = ppmStream[EXTERNAL_MODULE] ;
		// Configure output pin (TIM2)
  	configure_pins( PROT_PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
  	PROT_EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop timer
  	PROT_EXTMODULE_TIMER->PSC = (PeripheralSpeeds.Peri1_frequency * PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ; // 0.5uS (2Mhz)
//  	PROT_EXTMODULE_TIMER->PSC = 32767 ;
		PROT_EXTMODULE_TIMER->SMCR = TIM_SMCR_SMS_2 ;
  	PROT_EXTMODULE_TIMER->ARR = 99999 ;
  	PROT_EXTMODULE_TIMER->CCR2 = 99999 - 4000 ;
  	PROT_EXTMODULE_TIMER->CCR1 = (g_model.Module[1].ppmDelay*50+300)*2 ;
  	PROT_EXTMODULE_TIMER->CCER = TIM_CCER_CC1E | (g_model.Module[1].pulsePol ? TIM_CCER_CC1P : 0 ) ;
  	PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0; // Force O/P high
  	PROT_EXTMODULE_TIMER->EGR = 1; // Reloads register values now
  	PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC2PE; // PWM mode 1

//  	PROT_EXTMODULE_TIMER->CCR4 = 0 ;				// 32 bit counter
//  	PROT_EXTMODULE_TIMER->CCR3 = 0x10000 ;	// 32 bit counter

//		PROT_EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ; // Disable DMA
//    PROT_EXTMODULE_DMA_STREAM->CR |= PROT_EXTMODULE_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_1 | DMA_SxCR_MSIZE_1 | DMA_SxCR_PL_0 | DMA_SxCR_PL_1 ;
//    PROT_EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&PROT_EXTMODULE_TIMER->ARR) ;
//    PROT_EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(ProtoPpmPulses) ;
//    PROT_EXTMODULE_DMA_STREAM->NDTR = pulseStreamCount[EXTERNAL_MODULE] ;
//		DMA_ClearITPendingBit(PROT_EXTMODULE_DMA_STREAM, PROT_EXTMODULE_DMA_FLAG_TC) ;
//    PROT_EXTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE ; // Enable DMA
//	  NVIC_SetPriority(PROT_EXTMODULE_DMA_IRQn, 3) ;
//	  NVIC_EnableIRQ(PROT_EXTMODULE_DMA_IRQn) ;
	  
//		PROT_EXTMODULE_TIMER->SR &= ~TIM_SR_UIF ;                               // Clear flag
//  	PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_UDE; // Update DMA request
//		PROT_EXTMODULE_TIMER->DIER = TIM_DIER_CC3IE ; // Enable this interrupt
//	  PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_UIE ;
	
  	PROT_EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN; // Start timer
	
//		NVIC_SetPriority( PROT_EXTMODULE_TIMER_IRQn, 3 ) ; // Lower priority interrupt
//  	NVIC_EnableIRQ( PROT_EXTMODULE_TIMER_IRQn) ;
	
	}
#endif
}

static void disable_ext_ppm( void )
{
  EXTMODULE_TIMER->DIER &= ~(TIM_DIER_CC2IE | TIM_DIER_UDE);
 	EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
 	NVIC_DisableIRQ( TIM1_UP_TIM10_IRQn) ;
 	NVIC_DisableIRQ( EXTMODULE_TIMER_IRQn) ;
#ifdef PCBX12D
	if ( isProdVersion() == 0 )
	{
  	PROT_EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN;
	}
#endif
}

static void init_ext_multi( void )
{
	init_ext_serial( EXT_TYPE_MULTI ) ;
}

//static void disable_ext_multi( void )
//{
//	disable_ext_serial( EXT_TYPE_MULTI ) ;
//}



//#define PA10_TYPE_PXX		0
//#define PA10_TYPE_DSM		1
//#define PA10_TYPE_MULTI	2

//void init_pa10_serial( uint32_t type )
//{
//  INTERNAL_RF_ON();
//	if ( type == PA10_TYPE_PXX )
//	{
//  	setupPulsesPXX(INTERNAL_MODULE);
//	}
//	else
//	{
//  	setupPulsesDsm2(6, INTERNAL_MODULE) ;
//	}
  
//  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIO_INTPPM, ENABLE);
	
//	configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;

//  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;            // Enable clock
//  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;            // Enable DMA2 clock

//  TIM1->CR1 &= ~TIM_CR1_CEN ;
  
//	if ( type == PA10_TYPE_PXX )
//	{
//		TIM1->ARR = 18000 ;                     // 9mS
//  	TIM1->CCR2 = 15000 ;            // Update time
//	}
//	else if ( type == PA10_TYPE_DSM )
//	{
//		TIM1->ARR = 44000 ;                     // 22mS
//  	TIM1->CCR2 = 40000 ;            // Update time
//	}
//	else // type == PA10_TYPE_MULTI
//	{
////  	TIM1->ARR = 14000 ;             // 7mS
////  	TIM1->CCR2 = 10000 ;            // Update time
//		uint32_t x ;
//		x = g_model.Module[0].ppmFrameLength ;
//		if ( x > 4 )
//		{
//			x = 0 ;
//		}
//		x *= 2000 ;
//		x += 7000 * 2 ;
//  	TIM8->ARR = x ;             // 11mS
//  	TIM8->CCR2 = x-4000 ;       // Update time
//	}
//  TIM1->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz

//#ifdef PCB9XT
//  TIM1->CCER = TIM_CCER_CC3E | TIM_CCER_CC3P ;
//#else
//  TIM1->CCER = TIM_CCER_CC3E ;
//#endif

//  TIM1->CR2 = TIM_CR2_OIS3 ;              // O/P idle high
//  TIM1->BDTR = TIM_BDTR_MOE ;             // Enable outputs
//	if ( type == PA10_TYPE_PXX )
//	{
//	  TIM1->CCR3 = pxxStream[INTERNAL_MODULE][0];
//	  TIM1->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_0 ;                     // Force O/P high
//	}
//	else
//	{
//	  TIM1->CCR3 = dsm2Stream[INTERNAL_MODULE][0] ;
//	  TIM1->CCMR2 = TIM_CCMR2_OC3M_2 ; // Force O/P low, hardware inverts it
//	}
//  TIM1->EGR = 1 ;                                                         // Restart

//  DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
	
//	TIM1->DIER |= TIM_DIER_CC3DE ;          		// Enable DMA on CC3 match
//  TIM1->DCR = 15 ;                            // DMA to CC1

//  // Enable the DMA channel here, DMA2 stream 6, channel 6
//  DMA2->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6 ; // Write ones to clear bits
//  DMA2_Stream6->CR = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
//                                                         | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
//  DMA2_Stream6->PAR = CONVERT_PTR(&TIM1->DMAR);
//	if ( type == PA10_TYPE_PXX )
//	{
//  	DMA2_Stream6->M0AR = CONVERT_PTR(&pxxStream[INTERNAL_MODULE][1]);
//	}
//	else
//	{
//  	DMA2_Stream6->M0AR = CONVERT_PTR(&dsm2Stream[0][1]);
//	}
//  DMA2_Stream6->CR |= DMA_SxCR_EN ;               // Enable DMA

//  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0 ;                     // Toggle CC1 o/p
//  TIM1->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
//  TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
//  TIM1->CR1 |= TIM_CR1_CEN ;
//	NVIC_SetPriority( TIM1_CC_IRQn, 3 ) ; // Lower priority interrupt
//  NVIC_EnableIRQ(TIM1_CC_IRQn) ;
//}


//static void init_pa10_pxx()
//{
//	init_pa10_serial( PA10_TYPE_PXX ) ;
//}

//static void disable_pa10_pxx()
//{
//  DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
//  NVIC_DisableIRQ(TIM1_CC_IRQn) ;
//  TIM1->DIER &= ~TIM_DIER_CC2IE ;
//  TIM1->CR1 &= ~TIM_CR1_CEN ;
//  INTERNAL_RF_OFF();
//}

//// PPM output
//// Timer 1, channel 1 on PA8 for prototype
//// Pin is AF1 function for timer 1
//static void init_pa10_ppm()
//{
//  INTERNAL_RF_ON();
//  // Timer1
////  setupPulsesPPM(INTERNAL_MODULE) ;
//  ppmStreamPtr[INTERNAL_MODULE] = ppmStream[INTERNAL_MODULE];

//  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock

//  configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
  
//  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;            // Enable clock

//  TIM1->CR1 &= ~TIM_CR1_CEN ;
//  TIM1->ARR = *ppmStreamPtr[INTERNAL_MODULE]++ ;
//  TIM1->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;               // 0.5uS from 30MHz
  
//#ifdef PCB9XT
//  TIM1->CCER = TIM_CCER_CC3E | TIM_CCER_CC3P ;
//#else
//  TIM1->CCER = TIM_CCER_CC3E ;
//#endif
//  if(!g_model.Module[0].pulsePol)
//#ifdef PCB9XT
//	  TIM1->CCER &= ~TIM_CCER_CC3P ;
//#else
//    TIM1->CCER |= TIM_CCER_CC3P;
//#endif
  
//  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 ;     // PWM mode 1
//  TIM1->CCMR1 = TIM_CCMR1_OC2PE ;                   			// PWM mode 1
//  TIM1->CCR3 = (g_model.Module[0].ppmDelay*50+300)*2;
//  TIM1->BDTR = TIM_BDTR_MOE ;
//  TIM1->EGR = 1 ;
//  TIM1->DIER = TIM_DIER_UDE ;

//  TIM1->SR &= ~TIM_SR_UIF ;                               // Clear flag
//  TIM1->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
//  TIM1->DIER |= TIM_DIER_CC2IE ;
//  TIM1->DIER |= TIM_DIER_UIE ;

//  TIM1->CR1 = TIM_CR1_CEN ;
//	NVIC_SetPriority( TIM1_CC_IRQn, 3 ) ; // Lower priority interrupt
//	NVIC_SetPriority( TIM1_UP_TIM10_IRQn, 3 ) ; // Lower priority interrupt
//  NVIC_EnableIRQ(TIM1_CC_IRQn) ;
//  NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn) ;
//}

//static void disable_pa10_ppm()
//{
//  NVIC_DisableIRQ(TIM1_CC_IRQn) ;
//  NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn) ;
//  TIM1->DIER &= ~TIM_DIER_CC2IE & ~TIM_DIER_UIE ;
//  TIM1->CR1 &= ~TIM_CR1_CEN ;

//  INTERNAL_RF_OFF();
//}

//#ifdef PCB9XT
//static void init_pa10_dsm2()
//{
//	init_pa10_serial( PA10_TYPE_DSM ) ;
//}

//static void init_pa10_multi()
//{
//	init_pa10_serial( PA10_TYPE_MULTI ) ;
//}

//static void disable_pa10_dsm2()
//{
//  DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
//  NVIC_DisableIRQ(TIM1_CC_IRQn) ;
//  TIM1->DIER &= ~TIM_DIER_CC2IE ;
//  TIM1->CR1 &= ~TIM_CR1_CEN ;
//  INTERNAL_RF_OFF() ;
//}
//#endif

//#ifdef PCBX9D
//uint16_t XjtHbeatOffset ;
//#endif

//extern uint16_t g_timePXX;
//extern "C" void TIM1_CC_IRQHandler()
//{
//  uint16_t t0 = TIM3->CNT;
//  TIM1->DIER &= ~TIM_DIER_CC2IE ;         // stop this interrupt
//  TIM1->SR &= ~TIM_SR_CC2IF ;                             // Clear flag

////#ifdef REV9E
////	s_current_protocol[INTERNAL_MODULE] = PROTO_OFF ;
////#endif
////  DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
//	setupPulses(0) ;

//  if (s_current_protocol[INTERNAL_MODULE] == PROTO_PXX)
//	{
//#ifdef PCBX9D
//		XjtHbeatOffset = TIM7->CNT - XjtHeartbeatCapture.value ;
//		if ( XjtHeartbeatCapture.valid )
//		{
//			if ( XjtHbeatOffset > 0x2200 )
//			{
//				TIM1->ARR = 17980 ;                     // 9mS
//			}
//			else
//			{
//				TIM1->ARR = 18020 ;                     // 9mS
//			}
//		}
//#endif
//    DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
//    DMA2->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6 ; // Write ones to clear bits
//    DMA2_Stream6->M0AR = CONVERT_PTR(&pxxStream[INTERNAL_MODULE][1]);
//    DMA2_Stream6->CR |= DMA_SxCR_EN ;               // Enable DMA
//    TIM1->CCR3 = pxxStream[INTERNAL_MODULE][0];
//    TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
//  }
//  else if ( (s_current_protocol[INTERNAL_MODULE] == PROTO_DSM2 ) || (s_current_protocol[INTERNAL_MODULE] == PROTO_MULTI ) )
//	{
//    DMA2_Stream6->CR &= ~DMA_SxCR_EN ;              // Disable DMA
//    DMA2->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6 ; // Write ones to clear bits
//    DMA2_Stream6->M0AR = CONVERT_PTR(&dsm2Stream[0][1]);
//	  TIM1->CCMR2 = TIM_CCMR2_OC3M_2 ; // Force O/P low, hardware inverts it
//	  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0 ;                     // Toggle CC1 o/p
//    DMA2_Stream6->CR |= DMA_SxCR_EN ;               // Enable DMA
//    TIM1->CCR3 = dsm2Stream[0][0];
//    TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
//  }
//  else if (s_current_protocol[INTERNAL_MODULE] == PROTO_PPM)
//	{
//    ppmStreamPtr[INTERNAL_MODULE] = ppmStream[INTERNAL_MODULE];
//    TIM1->DIER |= TIM_DIER_UDE ;
//    TIM1->SR &= ~TIM_SR_UIF ;                                       // Clear this flag
//    TIM1->DIER |= TIM_DIER_UIE ;                            // Enable this interrupt
//  }
//  else {
//    TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
//  }
//  t0 = TIM3->CNT - t0;
//	g_timePXX = t0 ;

//}

//extern "C" void TIM1_UP_TIM10_IRQHandler()
//{
//	TIM1->SR &= ~TIM_SR_UIF ;                               // Clear flag

//	TIM1->ARR = *ppmStreamPtr[INTERNAL_MODULE]++ ;
//	if ( *ppmStreamPtr[INTERNAL_MODULE] == 0 )
//	{
//	  TIM1->SR &= ~TIM_SR_CC2IF ;                     // Clear this flag
//	  TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
//	}
//}

uint16_t ProtoPulses[800] ;

void convertPulsesProto( uint32_t type )
{
	uint16_t *p ;
	if ( type == EXT_TYPE_PXX )
	{
		p = pxxStream[EXTERNAL_MODULE] ;
	}
	else
	{
		p = dsm2Stream[EXTERNAL_MODULE] ;
	}
	uint16_t *pd = ProtoPulses ;
	uint32_t i ;
	uint32_t j ;
	j = pulseStreamCount[EXTERNAL_MODULE] ;
	if ( j > 400 )
	{
		j = 400 ;
	}
	for ( i = 0 ; i < j ; i += 1 )
	{
		uint16_t v ;
		v = *p++ ;
		*pd++ = v ;
		*pd++ = v + 8 ;
	}
}

//void old_init_ext_serial( uint32_t type )
//{
//  EXTERNAL_RF_ON() ;

//	if ( type == EXT_TYPE_PXX )
//	{
//  	setupPulsesPXX(EXTERNAL_MODULE);
//	}
////	else
////	{
////  	setupPulsesDsm2(6, EXTERNAL_MODULE) ;
////	}
  
//	if ( isProdVersion() )
//	{
//		// Configure output pin (TIM1)
//  	configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
//	}
//	else
//	{
////		convertPxxPulsesProto() ;
////	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
////#if defined(REV3)
////  configure_pins( PIN_INTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
////#else
////  configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_3 | PIN_OS25 | PIN_PUSHPULL ) ;
////#endif
////  RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;            // Enable clock
////  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;            // Enable DMA2 clock
  	
//		configure_pins( PROT_PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
//  	PROT_EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop timer
  
//		if ( type == EXT_TYPE_PXX )
//		{
//			PROT_EXTMODULE_TIMER->ARR = 18000 ;                     // 9mS
//  		PROT_EXTMODULE_TIMER->CCR2 = 15000 ;            // Update time
//		}
//		else if ( type == EXT_TYPE_DSM )
//		{
//			PROT_EXTMODULE_TIMER->ARR = 44000 ;                     // 22mS
//  		PROT_EXTMODULE_TIMER->CCR2 = 40000 ;            // Update time
//		}
//		else // type == EXT_TYPE_MULTI
//		{
//			uint32_t x ;
//			x = g_model.Module[EXTERNAL_MODULE].ppmFrameLength ;
//			if ( x > 4 )
//			{
//				x = 0 ;
//			}
//			x *= 2000 ;
//			x += 7000 * 2 ;
//  		PROT_EXTMODULE_TIMER->ARR = x ;             // 11mS
//  		PROT_EXTMODULE_TIMER->CCR2 = x-4000 ;       // Update time
//		}
//  	PROT_EXTMODULE_TIMER->PSC = (PeripheralSpeeds.Peri1_frequency * PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ; // 0.5uS (2Mhz)
////  	PROT_EXTMODULE_TIMER->PSC = 65535 ;
//	  PROT_EXTMODULE_TIMER->CCR4 = 0x10000 ;	// 32 bit counter
////#if defined(REV3)
////  TIM8->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
////#else
////#ifdef PCB9XT
////  TIM8->CCER = TIM_CCER_CC1NE | TIM_CCER_CC1NP ;
////#else
////  TIM8->CCER = TIM_CCER_CC1NE ;
////#endif
////#endif
//  	PROT_EXTMODULE_TIMER->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P | TIM_CCER_CC1NE | TIM_CCER_CC1NP; //  TIM_CCER_CC1E | TIM_CCER_CC1P;
////  TIM8->CR2 = TIM_CR2_OIS1 ;                      // O/P idle high
////  TIM8->BDTR = TIM_BDTR_MOE ;             // Enable outputs

//	if ( type == EXT_TYPE_PXX )
//	{
////	  PROT_EXTMODULE_TIMER->CCR1 = ProtoPxxPulses[0] ;
//  	PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0; // Force O/P high
//	}
//	else
//	{
//	  PROT_EXTMODULE_TIMER->CCR1 = dsm2Stream[EXTERNAL_MODULE][0] ;
//  	PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_2 ; // Force O/P low, hardware inverts it
//	}
//	PROT_EXTMODULE_TIMER->EGR = 1 ;                                                         // Restart
  
//	PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_CC1DE ;          // Enable DMA on CC1 match
////  PROT_EXTMODULE_TIMER->DCR = 13 ;                                                                // DMA to CC1


////  // Enable the DMA channel here, DMA2 stream 2, channel 7
////  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
////  DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2 ; // Write ones to clear bits
////  DMA2_Stream2->CR = DMA_SxCR_CHSEL_0 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
////																												 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
//		PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_CC4IE ; // Enable this interrupt
//		PROT_EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ; // Disable DMA
//		ProtoDebug[8] = 4 ;
//    PROT_EXTMODULE_DMA_STREAM->CR |= PROT_EXTMODULE_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_1 | DMA_SxCR_MSIZE_1 | DMA_SxCR_PL_0 | DMA_SxCR_PL_1 ;
  
////	DMA2_Stream2->PAR = CONVERT_PTR(&TIM8->DMAR);
//    PROT_EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&PROT_EXTMODULE_TIMER->CCR1) ;	// or DMAR?
  
//	if ( type == EXT_TYPE_PXX )
//	{
////		PROT_EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(&ProtoPxxPulses[1]);
//  }
//	else
//	{
//  	PROT_EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(&dsm2Stream[1][1]);
//	}
//	PROT_EXTMODULE_DMA_STREAM->NDTR = pulseStreamCount[EXTERNAL_MODULE] ;
  
////	DMA2_Stream2->CR |= DMA_SxCR_EN ;              // Enable DMA
//	DMA_ClearITPendingBit(PROT_EXTMODULE_DMA_STREAM, PROT_EXTMODULE_DMA_FLAG_TC) ;
//  PROT_EXTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN ; // Enable DMA
//	ProtoDebug[8] = 5 ;

////  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;                     // Toggle CC1 o/p
//  PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;       // Toggle CC1 o/p
////  TIM8->SR &= ~TIM_SR_CC2IF ;                             // Clear flag
//	PROT_EXTMODULE_TIMER->SR &= ~TIM_SR_CC2IF ;                     // Clear this flag
////  TIM8->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
//	PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
////  TIM8->CR1 |= TIM_CR1_CEN ;
//  	PROT_EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN; // Start timer
////	NVIC_SetPriority( TIM8_CC_IRQn, 3 ) ; // Lower priority interrupt
////  NVIC_EnableIRQ(TIM8_CC_IRQn) ;
//		NVIC_SetPriority( PROT_EXTMODULE_TIMER_IRQn, 3 ) ; // Lower priority interrupt
//  	NVIC_EnableIRQ( PROT_EXTMODULE_TIMER_IRQn) ;
//	}
//}

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
  
	// Configure output pin (TIM1)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;           // Enable portA clock
 	configure_pins( PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
	
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;            // Enable clock
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;            // Enable DMA2 clock
	
 	EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop timer
 	EXTMODULE_TIMER->PSC = (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ; // 0.5uS (2Mhz)
//  	EXTMODULE_TIMER->PSC = 65535 ;
  
	
	if ( type == EXT_TYPE_PXX )
	{
		EXTMODULE_TIMER->ARR = 17989 ;                     // 9mS - 5uS
  	EXTMODULE_TIMER->CCR2 = 15000 ;            // Update time
	}
	else if ( type == EXT_TYPE_DSM )
	{
		EXTMODULE_TIMER->ARR = 43999 ;                     // 22mS
  	EXTMODULE_TIMER->CCR2 = 40000 ;            // Update time
	}
	else // type == EXT_TYPE_MULTI
	{
		uint32_t x ;
		x = g_model.Module[EXTERNAL_MODULE].ppmFrameLength ;
		if ( x > 4 )
		{
			x = 0 ;
		}
		x *= 2000 ;
		x += 7000 * 2 ;
  	EXTMODULE_TIMER->ARR = x ;             // 11mS
  	EXTMODULE_TIMER->CCR2 = x-4000 ;       // Update time
	}
  EXTMODULE_TIMER->CCER = TIM_CCER_CC3E ;
	
  EXTMODULE_TIMER->CR2 = TIM_CR2_OIS3 ;              // O/P idle high
 	EXTMODULE_TIMER->BDTR = TIM_BDTR_MOE;

	if ( type == EXT_TYPE_PXX )
	{
	  EXTMODULE_TIMER->CCR3 = pxxStream[EXTERNAL_MODULE][0];
	  EXTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_0 ;  // Force O/P high
	}
	else
	{
	  EXTMODULE_TIMER->CCR3 = dsm2Stream[EXTERNAL_MODULE][0] ;
	  EXTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_2 ; // Force O/P low, hardware inverts it
	}
 	EXTMODULE_TIMER->EGR = 1; // Reloads register values now
	
	EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ;        // Disable DMA
	EXTMODULE_TIMER->DIER |= TIM_DIER_CC3DE ;          		// Enable DMA on CC3 match

  DMA_ClearITPendingBit(EXTMODULE_DMA_STREAM, EXTMODULE_DMA_FLAG_TC) ;
  EXTMODULE_DMA_STREAM->CR = EXTMODULE_DMA_CHANNEL | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
                                                         | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
  EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&EXTMODULE_TIMER->CCR3);
	
	if ( type == EXT_TYPE_PXX )
	{
  	EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(&pxxStream[EXTERNAL_MODULE][1]);
	}
	else
	{
  	EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(&dsm2Stream[EXTERNAL_MODULE][1]) ;
	}
	EXTMODULE_TIMER->CR2 = TIM_CR2_MMS_2 | TIM_CR2_MMS_1 | TIM_CR2_OIS3 ;			// CC3 event as trigger output

  EXTMODULE_TIMER->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0 ;     // Toggle CC1 o/p
  EXTMODULE_TIMER->SR = EXTMODULE_TIMER_SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
  EXTMODULE_TIMER->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
	 
#ifdef PCBX12D
	if ( isProdVersion() == 0 )
	{
		convertPulsesProto( type ) ;
 		EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(&ProtoPulses[1] ) ;
		configure_pins( PROT_PIN_EXTPPM_OUT, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;

//  	EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&EXTMODULE_TIMER->CCR1) ;
//		EXTMODULE_TIMER->CR2 = TIM_CR2_MMS_2 | TIM_CR2_MMS_0 | TIM_CR2_OIS3 ;			// CC1 pulse as trigger output
//	  EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_0 ;     // Set CC1 high
  	 
		PROT_EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop timer
  	PROT_EXTMODULE_TIMER->PSC = (PeripheralSpeeds.Peri1_frequency * PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ; // 0.5uS (2Mhz)

		PROT_EXTMODULE_TIMER->ARR = 99999 ;
		PROT_EXTMODULE_TIMER->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;
		if ( type == EXT_TYPE_PXX )
		{
	  	PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0 ;
		}
		else
		{
	  	PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0 ;
		}
		PROT_EXTMODULE_TIMER->EGR = 1 ;                                                         // Restart
		PROT_EXTMODULE_TIMER->CCR1 = 0 ;
  	PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;       // Toggle CC1 o/p
		PROT_EXTMODULE_TIMER->SMCR = TIM_SMCR_SMS_2 ;
 		
		PROT_EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN ; // Start timer
	}
#endif

  EXTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN ;               // Enable DMA
  EXTMODULE_TIMER->CR1 |= TIM_CR1_CEN ;
	NVIC_SetPriority( TIM1_CC_IRQn, 3 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(TIM1_CC_IRQn) ;

}


static void init_ext_pxx()
{
	init_ext_serial( EXT_TYPE_PXX ) ;
}

static void disable_ext_pxx()
{
//  DMA2_Stream2->CR &= ~DMA_SxCR_EN ;              // Disable DMA
	PROT_EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ; // Disable DMA
//  NVIC_DisableIRQ(TIM8_CC_IRQn) ;
 	NVIC_DisableIRQ( PROT_EXTMODULE_TIMER_IRQn) ;
//  TIM8->DIER &= ~TIM_DIER_CC2IE ;
	PROT_EXTMODULE_TIMER->DIER &= ~TIM_DIER_CC2IE ;
//  TIM8->CR1 &= ~TIM_CR1_CEN ;
 	PROT_EXTMODULE_TIMER->CR1 &= ~TIM_CR1_CEN; // Stop timer
  EXTERNAL_RF_OFF();
}



extern "C" void EXTMODULE_DMA_IRQHandler()
{
  if (!DMA_GetITStatus(EXTMODULE_DMA_STREAM, EXTMODULE_DMA_FLAG_TC))
    return ;

  DMA_ClearITPendingBit(EXTMODULE_DMA_STREAM, EXTMODULE_DMA_FLAG_TC) ;
	EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN & ~DMA_SxCR_TCIE ; // Disable DMA
  EXTMODULE_TIMER->SR = EXTMODULE_TIMER_SR_MASK & ~TIM_SR_CC2IF ; // Clear flag
  EXTMODULE_TIMER->DIER |= TIM_DIER_CC2IE ; // Enable this interrupt
}

extern "C" void TIM1_UP_TIM10_IRQHandler()
{
	TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;                               // Clear flag
	TIM1->ARR = *ppmStreamPtr[EXTERNAL_MODULE]++ ;
	if ( *ppmStreamPtr[EXTERNAL_MODULE] == 0 )
	{
		TIM1->DIER &= ~TIM_DIER_UIE ;		// Stop this interrupt
	  TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;     // Clear this flag
	  TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
	}
}

extern uint16_t XjtHbeatOffset ;

extern "C" void TIM1_CC_IRQHandler()
{
  TIM1->DIER &= ~TIM_DIER_CC2IE ;         // stop this interrupt
  setupPulses(EXTERNAL_MODULE) ;
	if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PPM)
	{
		ppmStreamPtr[EXTERNAL_MODULE] = ppmStream[EXTERNAL_MODULE] ;
		EXTMODULE_TIMER->CCR3 = (g_model.Module[1].ppmDelay*50+300)*2 ;
    EXTMODULE_TIMER->CCER = TIM_CCER_CC3E | (g_model.Module[1].pulsePol ? TIM_CCER_CC3P : 0 ) ;
		EXTMODULE_TIMER->SR = EXTMODULE_TIMER_SR_MASK & ~TIM_SR_UIF ;                                       // Clear this flag
#ifdef PCBX12D
		if ( isProdVersion() == 0 )
		{
  		PROT_EXTMODULE_TIMER->CCR1 = (g_model.Module[1].ppmDelay*50+300)*2 ;
			PROT_EXTMODULE_TIMER->CCER = TIM_CCER_CC1E | (g_model.Module[1].pulsePol ? 0 : TIM_CCER_CC1P ) ;
		}
#endif 
		EXTMODULE_TIMER->DIER |= TIM_DIER_UIE ;                            // Enable this interrupt
  }
	else if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PXX )
	{
		
		XjtHbeatOffset = TIM7->CNT - XjtHeartbeatCapture.value ;
		if ( XjtHeartbeatCapture.valid )
		{
			if ( XjtHbeatOffset > 0x2A00 )
//			if ( XjtHbeatOffset > 0x2200 )
			{
				EXTMODULE_TIMER->ARR = 17979 ;                     // 9mS
			}
			else
			{
				EXTMODULE_TIMER->ARR = 18019 ;                     // 9mS
			}
		}
		
  	EXTMODULE_TIMER->CCR3 = pxxStream[EXTERNAL_MODULE][0];
		EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ; // Disable DMA
		DMA_ClearITPendingBit(EXTMODULE_DMA_STREAM, EXTMODULE_DMA_FLAG_TC) ;
	  EXTMODULE_DMA_STREAM->CR = EXTMODULE_DMA_CHANNEL | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
                                                         | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
 		EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&EXTMODULE_TIMER->CCR3) ;	// or DMAR?
		if ( isProdVersion() )
		{
  		EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(&pxxStream[EXTERNAL_MODULE][1]);
		}
#ifdef PCBX12D
		else
		{
			convertPulsesProto( EXT_TYPE_PXX ) ;
  		EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(&ProtoPulses[1]) ;
	  	PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0 ;
			PROT_EXTMODULE_TIMER->EGR = 1 ;         // Restart
  		PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;       // Toggle CC1 o/p
		}
#endif
  	EXTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN ; // Enable DMA
	  EXTMODULE_TIMER->SR = EXTMODULE_TIMER_SR_MASK & ~TIM_SR_CC2IF ;     // Clear this flag
		EXTMODULE_TIMER->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
	}	
  else if ( (s_current_protocol[EXTERNAL_MODULE] == PROTO_DSM2 ) || (s_current_protocol[EXTERNAL_MODULE] == PROTO_MULTI ) )
	{
  	EXTMODULE_TIMER->CCR3 = dsm2Stream[1][0] ;
		EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ; // Disable DMA
		DMA_ClearITPendingBit(EXTMODULE_DMA_STREAM, EXTMODULE_DMA_FLAG_TC) ;
	  EXTMODULE_DMA_STREAM->CR = EXTMODULE_DMA_CHANNEL | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0
                                                         | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
 		EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&EXTMODULE_TIMER->CCR3) ;	// or DMAR?
		if ( isProdVersion() )
		{
  		EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(&dsm2Stream[1][1]) ;
		}
#ifdef PCBX12D
		else
		{
			convertPulsesProto( EXT_TYPE_MULTI ) ;
  		EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(&ProtoPulses[1]) ;
	  	PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0 ;
			PROT_EXTMODULE_TIMER->EGR = 1 ;                                                         // Restart
  		PROT_EXTMODULE_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;       // Toggle CC1 o/p
		}
#endif 
  	EXTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN ; // Enable DMA
	  EXTMODULE_TIMER->SR = EXTMODULE_TIMER_SR_MASK & ~TIM_SR_CC2IF ;     // Clear this flag
		EXTMODULE_TIMER->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
  }
	else
	{
	  TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_CC2IF ;     // Clear this flag
		TIM1->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
	}
}


extern "C" void PROT_EXTMODULE_DMA_IRQHandler()
{
  if (!DMA_GetITStatus(PROT_EXTMODULE_DMA_STREAM, PROT_EXTMODULE_DMA_FLAG_TC))
	{
    return ;
	}

  DMA_ClearITPendingBit(PROT_EXTMODULE_DMA_STREAM, PROT_EXTMODULE_DMA_FLAG_TC) ;
	PROT_EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN & ~DMA_SxCR_TCIE ; // Disable DMA
  PROT_EXTMODULE_TIMER->SR = PROT_EXTMODULE_TIMER_SR_MASK & ~TIM_SR_CC2IF ; // Clear flag
  PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_CC2IE ; // Enable this interrupt
}

extern "C" void TIM2_IRQHandler()
{
	uint16_t status = TIM2->SR ;
	if ( status & TIM_SR_CC3IF )
	{
		TIM2->SR = TIMER2_5SR_MASK & ~TIM_SR_CC3IF ;
		TIM2->CNT = 0 ;		// wrap to 16 bits!
	}
  if ( ( TIM2->DIER & TIM_DIER_UIE ) && ( status & TIM_SR_UIF ) )
	{
 		TIM2->SR = TIMER2_5SR_MASK & ~TIM_SR_UIF ;             // Clear flag
 		TIM2->ARR = *ppmStreamPtr[EXTERNAL_MODULE]++ ;
 		if ( *ppmStreamPtr[EXTERNAL_MODULE] == 0 )
		{
 		  TIM2->SR = TIMER2_5SR_MASK & ~TIM_SR_CC2IF ;     // Clear this flag
 		  TIM2->DIER &= ~TIM_DIER_UIE ;		// Disable this interrupt
 		  TIM2->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
		}
	}
  if ( ( TIM2->DIER & TIM_DIER_CC2IE ) && ( status & TIM_SR_CC2IF ) )
	{
  	TIM2->DIER &= ~TIM_DIER_CC2IE ;         // stop this interrupt
		setupPulses(EXTERNAL_MODULE) ;
		if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PPM)
		{
//			convertPpmPulsesProto() ;
//			ppmStreamPtr[EXTERNAL_MODULE] = ppmStream[EXTERNAL_MODULE] ;
  		TIM2->CCR1 = (g_model.Module[1].ppmDelay*50+300)*2 ;
			TIM2->CCER = TIM_CCER_CC1E | (g_model.Module[1].pulsePol ? TIM_CCER_CC1P : 0 ) ;
//			PROT_EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ; // Disable DMA
////			DMA_ClearITPendingBit(PROT_EXTMODULE_DMA_STREAM, PROT_EXTMODULE_DMA_FLAG_TC) ;
//	    PROT_EXTMODULE_DMA_STREAM->CR |= PROT_EXTMODULE_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_1 | DMA_SxCR_MSIZE_1 | DMA_SxCR_PL_0 | DMA_SxCR_PL_1 ;
//			PROT_EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&PROT_EXTMODULE_TIMER->ARR) ;
//			PROT_EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(ProtoPpmPulses) ;
//			PROT_EXTMODULE_DMA_STREAM->NDTR = pulseStreamCount[EXTERNAL_MODULE] ;
//			PROT_EXTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE ; // Enable DMA
//			NVIC_EnableIRQ(PROT_EXTMODULE_DMA_IRQn) ;
//			PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_UDE ; // Update DMA request
		
//			PROT_EXTMODULE_TIMER->SR &= ~TIM_SR_UIF ;                               // Clear flag
//		  PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_UIE ;
		
		}
		else if (s_current_protocol[EXTERNAL_MODULE] == PROTO_PXX )
		{
//			convertPxxPulsesProto() ;
			
//			PROT_EXTMODULE_TIMER->CCR2 = ProtoPxxPulses[pulseStreamCount[EXTERNAL_MODULE]-1] - 2000 ;
//		  PROT_EXTMODULE_TIMER->ARR = ProtoPxxPulses[0] ;
			PROT_EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ; // Disable DMA
			PROT_EXTMODULE_DMA_STREAM->CR |= PROT_EXTMODULE_DMA_CHANNEL | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_PSIZE_1 | DMA_SxCR_MSIZE_1 | DMA_SxCR_PL_0 | DMA_SxCR_PL_1 ;
  		PROT_EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&PROT_EXTMODULE_TIMER->ARR) ;	// or DMAR?
//			PROT_EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(ProtoPxxPulses) ;
			PROT_EXTMODULE_DMA_STREAM->NDTR = pulseStreamCount[EXTERNAL_MODULE] ;
			DMA_ClearITPendingBit(PROT_EXTMODULE_DMA_STREAM, PROT_EXTMODULE_DMA_FLAG_TC) ;
  		PROT_EXTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE ; // Enable DMA
			
//			PROT_EXTMODULE_DMA_STREAM->CR &= ~DMA_SxCR_EN ; // Disable DMA
//	    PROT_EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&PROT_EXTMODULE_TIMER->CCR1) ;
//	    PROT_EXTMODULE_DMA_STREAM->PAR = CONVERT_PTR(&PROT_EXTMODULE_TIMER->DMAR) ;
//			PROT_EXTMODULE_DMA_STREAM->M0AR = CONVERT_PTR(&ProtoPxxPulses[1]);
//			PROT_EXTMODULE_DMA_STREAM->NDTR = pulseStreamCount[EXTERNAL_MODULE] ;
//			PROT_EXTMODULE_DMA_STREAM->CR |= DMA_SxCR_EN ; // Enable DMA
//			NVIC_EnableIRQ(PROT_EXTMODULE_DMA_IRQn) ;
//			PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_UDE ; // Update DMA request
//			PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_CC1DE ;          // Enable DMA on CC1 match
//		  PROT_EXTMODULE_TIMER->SR &= ~TIM_SR_CC2IF ; // Clear flag
//    	PROT_EXTMODULE_TIMER->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
		}	
		else
		{
		 	TIM2->SR = TIMER2_5SR_MASK & ~TIM_SR_CC2IF ;             // Clear this flag
			TIM2->DIER |= TIM_DIER_CC2IE ;  // Enable this interrupt
		}
	}
}


extern uint8_t PxxSerial[] ;
volatile uint8_t *PxxTxPtr ;
volatile uint8_t PxxTxCount ;

uint16_t XjtHbeatOffset ;

extern "C" void TIM8_BRK_TIM12_IRQHandler()
{
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
					if ( XjtHbeatOffset > 0x3A00 )
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

extern "C" void INTMODULE_USART_IRQHandler()
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

