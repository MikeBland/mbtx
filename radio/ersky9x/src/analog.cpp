/****************************************************************************
*  Copyright (c) 2012 by Michael Blandford. All rights reserved.
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
*
****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//#define XLITE_PROTO	1

#ifdef PCBSKY
#include "AT91SAM3S4.h"
#endif

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBXLITE) || defined(PCBX9LITE)
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/stm32f2xx_rcc.h"
#include "X9D/hal.h"
#endif

#ifdef PCB9XT
#include "mega64.h"
#endif

#ifndef SIMU
#include "core_cm3.h"
#endif

#include "ersky9x.h"
#include "timers.h"
#include "logicio.h"
#include "analog.h"
#include "myeeprom.h"

#ifdef PCBX7
#define STICK_LV	2
#define STICK_LH  3
#define STICK_RV  0
#define STICK_RH  1
#define POT_L			8
#define POT_R			6
#define BATTERY		10
#define SLIDE_L		15
#define SLIDE_R		9
#define V_BATT		18

#else // PCBX7

#ifndef PCB9XT

#if defined(PCBXLITE) || defined(PCBX9LITE)

#ifdef PCBX9LITE
#define STICK_LV	1
#define STICK_LH  0
#define STICK_RV  2
#define STICK_RH  3
#else
#define STICK_LV	0
#define STICK_LH  1
#define STICK_RV  3
#define STICK_RH  2
#endif

#define POT_L			11
#define POT_R			12
#define BATTERY		10
#define V_BATT		18

#else // PCBXLITE

#define STICK_LV	3
#define STICK_LH  2
#define STICK_RV  0
#define STICK_RH  1
#define POT_L			6
#define POT_R			8
#define SLIDE_L		14
#define SLIDE_R		15
#define BATTERY		10
#define POT_3			9
#define V_BATT		18
#endif // PCBXLITE
#else
#define STICK_LV	10
#define STICK_LH  11
#define STICK_RV  12
#define STICK_RH  13
#define BATTERY		8
#define DIG1			6
#define DIG2			9
#define DIG3			14
#define V_BATT		18
#endif // nPCB9XT
#endif // PCBX7

#ifdef REV9E
#define FLAP_3		6
#define FLAP_4		7
#define FLAP_5		8
#define FLAP_6		9

#endif	// REV9E

#ifdef REV9E
void init_adc3( void ) ;
#endif	// REV9E

static void enableRtcBattery()
{
	ADC->CCR |= ADC_CCR_VBATE ;
}

void disableRtcBattery()
{
	ADC->CCR &= ~ADC_CCR_VBATE ;
}


// Sample time should exceed 1uS
#define SAMPTIME	3		// sample time = 56 cycles

volatile uint16_t Analog_values[NUMBER_ANALOG+NUM_POSSIBLE_EXTRA_POTS] ;
uint16_t VbattRtc ;

#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)

extern void delay_ms( uint32_t ms ) ;

volatile uint16_t PwmInterruptCount ;
uint8_t SticksPwmDisabled = 0 ;

struct t_PWMcontrol
{
volatile uint32_t timer_capture_rising_time ;
volatile uint32_t timer_capture_value ;
volatile uint32_t timer_capture_period ;
} PWMcontrol[4] ;


static void initSticksPwm()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM5EN ;		// Enable clock
	
	configure_pins( PIN_STICK_RV | PIN_STICK_RH | PIN_STICK_LV | PIN_STICK_LH, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_2 | PIN_NO_PULLUP ) ;

  TIM5->CR1 &= ~TIM_CR1_CEN ; // Stop timer
  TIM5->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ;		// 0.5uS
  TIM5->ARR = 0xffffffff ;
  TIM5->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0 ;
  TIM5->CCMR2 = TIM_CCMR2_CC3S_0 | TIM_CCMR2_CC4S_0 ;
  TIM5->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E ;
  TIM5->DIER |= TIM_DIER_CC1IE|TIM_DIER_CC2IE|TIM_DIER_CC3IE|TIM_DIER_CC4IE ;
  TIM5->EGR = 1; // Restart
  TIM5->CR1 = TIM_CR1_CEN ; // Start timer

  NVIC_SetPriority(TIM5_IRQn, 1 ) ;
  NVIC_EnableIRQ(TIM5_IRQn) ;
}

static void disableSticksPwm()
{
  NVIC_DisableIRQ(TIM5_IRQn) ;
  TIM5->CR1 &= ~TIM_CR1_CEN ; // Stop timer
}

extern "C" void TIM5_IRQHandler(void)
{
  uint32_t capture ;
	uint32_t value ;
	struct t_PWMcontrol *p ;

	PwmInterruptCount += 1 ; // overflow may happen but we only use this to detect PWM / ADC on radio startup

  if ( PWM_TIMER->SR & TIM_DIER_CC1IE )
	{
		capture = TIM5->CCR1 ;
		p = &PWMcontrol[0] ;
    if (TIM5->CCER & TIM_CCER_CC1P)
		{
	    value = capture - p->timer_capture_rising_time ;
			p->timer_capture_value = value ;
  		TIM5->CCER &= ~TIM_CCER_CC1P ; // Rising
		}
		else
		{
			p->timer_capture_period = capture - p->timer_capture_rising_time ;
			p->timer_capture_rising_time = capture ;
  		TIM5->CCER |= TIM_CCER_CC1P ; // Falling
		}
	}
  if ( PWM_TIMER->SR & TIM_DIER_CC2IE )
	{
		capture = TIM5->CCR2 ;
		p = &PWMcontrol[1] ;
    if (TIM5->CCER & TIM_CCER_CC2P)
		{
	    value = capture - p->timer_capture_rising_time ;
			p->timer_capture_value = value ;
  		TIM5->CCER &= ~TIM_CCER_CC2P ; // Rising
		}
		else
		{
			p->timer_capture_period = capture - p->timer_capture_rising_time ;
			p->timer_capture_rising_time = capture ;
  		TIM5->CCER |= TIM_CCER_CC2P ; // Falling
		}
	}
  if ( PWM_TIMER->SR & TIM_DIER_CC3IE )
	{
		capture = TIM5->CCR3 ;
		p = &PWMcontrol[2] ;
    if (TIM5->CCER & TIM_CCER_CC3P)
		{
	    value = capture - p->timer_capture_rising_time ;
			p->timer_capture_value = value ;
  		TIM5->CCER &= ~TIM_CCER_CC3P ; // Rising
		}
		else
		{
			p->timer_capture_period = capture - p->timer_capture_rising_time ;
			p->timer_capture_rising_time = capture ;
  		TIM5->CCER |= TIM_CCER_CC3P ; // Falling
		}
	}
  if ( PWM_TIMER->SR & TIM_DIER_CC4IE )
	{
		capture = TIM5->CCR4 ;
		p = &PWMcontrol[3] ;
    if (TIM5->CCER & TIM_CCER_CC4P)
		{
	    value = capture - p->timer_capture_rising_time ;
			p->timer_capture_value = value ;
  		TIM5->CCER &= ~TIM_CCER_CC4P ; // Rising
		}
		else
		{
			p->timer_capture_period = capture - p->timer_capture_rising_time ;
			p->timer_capture_rising_time = capture ;
  		TIM5->CCER |= TIM_CCER_CC4P ; // Falling
		}
	}
}

#endif // PCBXLITE/X3

#ifdef PCBX9LITE
#define EXTRA_POSSIBLE_ANALOG		2
#else
#define EXTRA_POSSIBLE_ANALOG		0
#endif
void init_adc()
{
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN ;			// Enable clock
	RCC->AHB1ENR |= RCC_AHB1Periph_GPIOADC ;	// Enable ports A&C clocks (and B for REVPLUS)
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;		// Enable DMA2 clock

#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
	initSticksPwm() ;
  delay_ms(20) ;
#ifndef XLITE_PROTO
  if (PwmInterruptCount < 8 )
#endif
	{
    SticksPwmDisabled = true ;
		disableSticksPwm() ;
  }
#endif // PCBXLITE/X3

#if defined(PCBXLITE) || defined(PCBX9LITE)

#ifdef PCBX9LITE
	configure_pins( PIN_FLP_J1 | PIN_MVOLT, PIN_ANALOG | PIN_PORTC ) ;

	if ( g_eeGeneral.extraPotsSource[0] )
	{
		uint32_t pin ;
		pin = ( g_eeGeneral.extraPotsSource[0] == 1 ) ? 4 : 8 ;
		configure_pins( pin, PIN_ANALOG | PIN_PORTC ) ;
	}

	if ( g_eeGeneral.extraPotsSource[1] )
	{
		uint32_t pin ;
		pin = ( g_eeGeneral.extraPotsSource[1] == 1 ) ? 4 : 8 ;
		configure_pins( pin, PIN_ANALOG | PIN_PORTC ) ;
	}

#else
	configure_pins( PIN_FLP_J1 | PIN_FLP_J2 | PIN_MVOLT, PIN_ANALOG | PIN_PORTC ) ;
#endif
  if ( SticksPwmDisabled )
	{
		configure_pins( ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH, PIN_ANALOG | PIN_PORTA ) ;
	}




#else // PCBXLITE/X3

#ifdef PCBX7
	configure_pins( PIN_STK_J1 | PIN_STK_J2 | PIN_STK_J3 | PIN_STK_J4 |
									PIN_FLP_J1 , PIN_ANALOG | PIN_PORTA ) ;
	configure_pins( PIN_FLP_J2, PIN_ANALOG | PIN_PORTB ) ;
	configure_pins( PIN_MVOLT, PIN_ANALOG | PIN_PORTC ) ;

#ifndef PCBT12
	configure_pins( LED_BLUE_GPIO_PIN, PIN_ANALOG | PIN_PORTB ) ;
	configure_pins( LED_RED_GPIO_PIN, PIN_ANALOG | PIN_PORTC ) ;
#endif

#else // PCBX7
#ifndef PCB9XT
	configure_pins( PIN_STK_J1 | PIN_STK_J2 | PIN_STK_J3 | PIN_STK_J4 |
									PIN_FLP_J1 , PIN_ANALOG | PIN_PORTA ) ;

#ifdef REV9E
	configure_pins( PIN_FLP_J2 | PIN_FLAP6, PIN_ANALOG | PIN_PORTB ) ;
#else	 
#if defined(REVPLUS) || defined(REV9E)
	configure_pins( PIN_FLP_J2 | PIN_FLP_J3, PIN_ANALOG | PIN_PORTB ) ;
 #else	 
	configure_pins( PIN_FLP_J2, PIN_ANALOG | PIN_PORTB ) ;
 #endif	// REVPLUS
#endif	// REV9E
	
	configure_pins( PIN_SLD_J1 | PIN_SLD_J2 | PIN_MVOLT, PIN_ANALOG | PIN_PORTC ) ;
#else	 
	configure_pins( PIN_STK_J1 | PIN_STK_J2 | PIN_STK_J3 | PIN_STK_J4 | PIN_SW3, PIN_ANALOG | PIN_PORTC ) ;
	configure_pins( PIN_MVOLT | PIN_SW2, PIN_ANALOG | PIN_PORTB ) ;
	configure_pins( PIN_SW1, PIN_ANALOG | PIN_PORTA ) ;
#endif // PCB9XT
#endif // PCBX7
#endif // PCBXLITE/X3
				

	ADC1->CR1 = ADC_CR1_SCAN ;
	ADC1->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS ;

#if defined(PCBXLITE) || defined(PCBX9LITE)

  if ( SticksPwmDisabled )
	{
		ADC1->SQR1 = (NUMBER_ANALOG-1+4+EXTRA_POSSIBLE_ANALOG) << 20 ;		// NUMBER_ANALOG Channels
		 
#ifdef PCBX9LITE
		ADC1->SQR2 = (V_BATT) + (12 << 5) + ( 13 << 10 ) ;
		ADC1->SQR3 = STICK_LH + (STICK_LV<<5) + (STICK_RV<<10) + (STICK_RH<<15) + (POT_L<< 20) + (BATTERY <<25) ;
#else // X3
		ADC1->SQR2 = BATTERY + (V_BATT<<5) ;
		ADC1->SQR3 = STICK_LH + (STICK_LV<<5) + (STICK_RV<<10) + (STICK_RH<<15) + (POT_L<< 20) + (POT_R<<25) ;
#endif // X3
	}
	else
	{
#ifdef PCBX9LITE
		ADC1->SQR1 = (NUMBER_ANALOG-1+EXTRA_POSSIBLE_ANALOG) << 20 ;		// NUMBER_ANALOG Channels
		ADC1->SQR3 = POT_L + (BATTERY<<5) + (V_BATT<<10) + (12 << 15) + ( 13 << 20 ) ;
#else // X3
		ADC1->SQR1 = (NUMBER_ANALOG-1) << 20 ;		// NUMBER_ANALOG Channels
		ADC1->SQR3 = POT_L + (POT_R<<5) + (BATTERY<<10) + (V_BATT<<15) ;
#endif // X3
	}
	ADC1->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12)
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) ;
	ADC1->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) 
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27) ;
#else // PCBXLITE/X3

#ifdef PCBX7
	ADC1->SQR1 = (NUMBER_ANALOG-1) << 20 ;		// NUMBER_ANALOG Channels

	ADC1->SQR2 = BATTERY + (SLIDE_L<<5) + (SLIDE_R<<10) + (V_BATT<<15) ;
	ADC1->SQR3 = STICK_LH + (STICK_LV<<5) + (STICK_RV<<10) + (STICK_RH<<15) + (POT_L<<20) + (POT_R<<25) ;

	ADC1->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12)
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) ;
	ADC1->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) 
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27) ;
	#else // PCBX7
#ifndef PCB9XT
	ADC1->SQR1 = (NUMBER_ANALOG-1) << 20 ;		// NUMBER_ANALOG Channels
#if defined(REVPLUS) || defined(REV9E)
 #ifdef REV9E
	ADC1->SQR2 = SLIDE_L + (SLIDE_R<<5) + (BATTERY<<10) + (FLAP_6<<15) + (V_BATT<<20) ;
 #else	 
	ADC1->SQR2 = SLIDE_L + (SLIDE_R<<5) + (BATTERY<<10) + (POT_3<<15) + (V_BATT<<20) ;
 #endif	// REV9E
#else
	ADC1->SQR2 = SLIDE_L + (SLIDE_R<<5) + (BATTERY<<10) + (V_BATT<<15) ;
#endif
	ADC1->SQR3 = STICK_LH + (STICK_LV<<5) + (STICK_RV<<10) + (STICK_RH<<15) + (POT_L<<20) + (POT_R<<25) ;
	ADC1->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12)
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) ;
	ADC1->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) 
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27) ;

#else	 
// ADD PCB9XT here
	ADC1->SQR1 = (NUMBER_ANALOG-1-NUM_REMOTE_ANALOG) << 20 ;		// NUMBER_ANALOG Channels
	ADC1->SQR2 = DIG2 + (DIG3<<5) + (V_BATT<<10) ;
	ADC1->SQR3 = STICK_LH + (STICK_LV<<5) + (STICK_RV<<10) + (STICK_RH<<15) + (BATTERY<<20) + (DIG1<<25) ;
	ADC1->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12)
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) ;
	ADC1->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) 
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27) ;

#endif // PCB9XT
#endif // PCBX7
#endif // PCBXLITE/X3


	 
	ADC->CCR = 0 ; //ADC_CCR_ADCPRE_0 ;		// Clock div 2
	
	DMA2_Stream4->CR = DMA_SxCR_PL | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC ;	// Channel 0
	DMA2_Stream4->PAR = (uint32_t) &ADC1->DR ;
	DMA2_Stream4->M0AR = (uint32_t) Analog_values ;
	DMA2_Stream4->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0 ;
#ifdef REV9E
	init_adc3() ;
#endif	// REV9E

	enableRtcBattery() ;
}

uint32_t read_adc()
{
	uint32_t i ;
	
	DMA2_Stream4->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	ADC1->SR &= ~(uint32_t) ( ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR ) ;
	DMA2->HIFCR = DMA_HIFCR_CTCIF4 | DMA_HIFCR_CHTIF4 |DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4 ; // Write ones to clear bits
	DMA2_Stream4->M0AR = (uint32_t) Analog_values ;

#if defined(PCBXLITE) || defined(PCBX9LITE)

  if ( SticksPwmDisabled )
	{
		DMA2_Stream4->NDTR = NUMBER_ANALOG-NUM_REMOTE_ANALOG + 4 + EXTRA_POSSIBLE_ANALOG ;
	}
	else
	{
		DMA2_Stream4->NDTR = NUMBER_ANALOG-NUM_REMOTE_ANALOG + EXTRA_POSSIBLE_ANALOG ;
	}
#else
	DMA2_Stream4->NDTR = NUMBER_ANALOG-NUM_REMOTE_ANALOG ;
#endif
	DMA2_Stream4->CR |= DMA_SxCR_EN ;		// Enable DMA
#ifdef REV9E
	DMA2_Stream1->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	ADC3->SR &= ~(uint32_t) ( ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR ) ;
	DMA2->LIFCR = DMA_LIFCR_CTCIF1 | DMA_LIFCR_CHTIF1 |DMA_LIFCR_CTEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CFEIF1 ; // Write ones to clear bits
	DMA2_Stream1->M0AR = (uint32_t) &Analog_values[NUMBER_ANALOG] ;
	DMA2_Stream1->NDTR = NUM_EXTRA_ANALOG ;
	DMA2_Stream1->CR |= DMA_SxCR_EN ;		// Enable DMA
	ADC3->CR2 |= (uint32_t)ADC_CR2_SWSTART ;
#endif	// REV9E
	ADC1->CR2 |= (uint32_t)ADC_CR2_SWSTART ;
	for ( i = 0 ; i < 20000 ; i += 1 )
	{
		if ( DMA2->HISR & DMA_HISR_TCIF4 )
		{
			break ;
		}
	}
	DMA2_Stream4->CR &= ~DMA_SxCR_EN ;		// Disable DMA

#if defined(PCBXLITE) || defined(PCBX9LITE)


  if ( SticksPwmDisabled )
	{
		AnalogData[0] = 4096 - Analog_values[0] ;
		AnalogData[1] = Analog_values[1] ;
		AnalogData[2] = 4096 - Analog_values[2] ;
		AnalogData[3] = Analog_values[3] ;
		AnalogData[4] = Analog_values[4] ;
#ifdef PCBX9LITE
		AnalogData[12] = Analog_values[5] ;
		if (ADC->CCR & ADC_CCR_VBATE )
		{
			VbattRtc = Analog_values[6] ;
		}
		AnalogData[5] = 4096 - ( (g_eeGeneral.extraPotsSource[0]) ? Analog_values[6+g_eeGeneral.extraPotsSource[0]] : 0 ) ; 
		AnalogData[6] = 4096 - ( (g_eeGeneral.extraPotsSource[1]) ? Analog_values[6+g_eeGeneral.extraPotsSource[1]] : 0 ) ; 

#else		 
		AnalogData[5] = Analog_values[5] ;
		AnalogData[12] = Analog_values[6] ;
		if (ADC->CCR & ADC_CCR_VBATE )
		{
			VbattRtc = Analog_values[7] ;
		}
#endif
	}
	else
	{
		struct t_PWMcontrol *p ;
		p = &PWMcontrol[0] ;

		AnalogData[4] = Analog_values[0] ;
#ifdef PCBX9LITE
		AnalogData[12] = Analog_values[1] ;
		if (ADC->CCR & ADC_CCR_VBATE )
		{
			VbattRtc = Analog_values[2] ;
		}
#else		 
		AnalogData[5] = Analog_values[1] ;
		AnalogData[12] = Analog_values[2] ;
		if (ADC->CCR & ADC_CCR_VBATE )
		{
			VbattRtc = Analog_values[3] ;
		}
#endif
		uint16_t value ;
		value = 0x0800 ;
		if ( p->timer_capture_period )
		{
			value = p->timer_capture_value * 4095 / p->timer_capture_period ;
		}
		if ( value < 4096 )
		{
			AnalogData[0] = value ;
		}
		p += 1 ;
		value = 0x0800 ;
		if ( p->timer_capture_period )
		{
			value = p->timer_capture_value * 4095 / p->timer_capture_period ;
		}
		if ( value < 4096 )
		{
			AnalogData[1] = 4095 - value ;
		}
		p += 1 ;
		value = 0x0800 ;
		if ( p->timer_capture_period )
		{
			value = p->timer_capture_value * 4095 / p->timer_capture_period ;
		}
		if ( value < 4096 )
		{
			AnalogData[3] = value ;
		}
		p += 1 ;
		value = 0x0800 ;
		if ( p->timer_capture_period )
		{
			value = p->timer_capture_value * 4095 / p->timer_capture_period ;
		}
		if ( value < 4096 )
		{
			AnalogData[2] = 4095 - value ;
		}
	}
#else // PCBXLITE

#ifdef PCBX7
	AnalogData[0] = 4096 - Analog_values[0] ;
	AnalogData[1] = Analog_values[1] ;
	AnalogData[2] = 4096 - Analog_values[2] ;
	AnalogData[3] = Analog_values[3] ;
	AnalogData[4] = Analog_values[4] ;
	AnalogData[5] = Analog_values[5] ;
//	AnalogData[6] = 0 ; // Dummy

	AnalogData[6] = 4096 - ( (g_eeGeneral.extraPotsSource[0]) ? Analog_values[6+g_eeGeneral.extraPotsSource[0]] : 0 ) ; 
	AnalogData[7] = 4096 - ( (g_eeGeneral.extraPotsSource[1]) ? Analog_values[6+g_eeGeneral.extraPotsSource[1]] : 0 ) ; 
	AnalogData[12] = Analog_values[6] ;
	if (ADC->CCR & ADC_CCR_VBATE )
	{
		VbattRtc = Analog_values[9] ;
	}
#else // PCBX7
	AnalogData[0] = Analog_values[0] ;
#ifdef PCBX9D
#ifdef REV9E
	AnalogData[1] = Analog_values[1] ;
#else
	AnalogData[1] = 4096 - Analog_values[1] ;
#endif
#else
	AnalogData[1] = Analog_values[1] ;
#endif
#ifdef REV9E
	AnalogData[2] = 4096 - Analog_values[2] ;
#else
	AnalogData[2] = Analog_values[2] ;
#endif

#ifdef PCBX9D
	AnalogData[3] = 4096 - Analog_values[3] ;
#else
	AnalogData[3] = Analog_values[3] ;
#endif

#ifndef REV9E
#ifndef PCB9XT	 
	AnalogData[4] = Analog_values[4] ;
	AnalogData[5] = Analog_values[5] ;
	AnalogData[6] = Analog_values[6] ;
	AnalogData[7] = Analog_values[7] ;
#endif
#ifdef PCB9XT
	AnalogData[4] = M64Analog[4] * 2 ;
	AnalogData[5] = M64Analog[5] * 2 ;
	AnalogData[6] = M64Analog[6] * 2 ;
	AnalogData[7] = M64Analog[0] * 2 ;
	AnalogData[8] = M64Analog[1] * 2 ;
	AnalogData[9] = M64Analog[2] * 2 ;
	AnalogData[10] = M64Analog[3] * 2 ;

	AnalogData[12] = Analog_values[4] ;
	if (ADC->CCR & ADC_CCR_VBATE )
	{
		VbattRtc = Analog_values[8] ;
	}
#else
	AnalogData[12] = Analog_values[8] ;
	if (ADC->CCR & ADC_CCR_VBATE )
	{
		VbattRtc = Analog_values[9] ;
	}
#endif
#if defined(REVPLUS) || defined(REV9E)
	AnalogData[8] = Analog_values[9] ;
	if (ADC->CCR & ADC_CCR_VBATE )
	{
		VbattRtc = Analog_values[10] ;
	}

#endif
#endif // nREV9E
#endif // PCBX7

#ifdef REV9E
	AnalogData[4] = 4096 - Analog_values[11] ;
	AnalogData[5] = 4096 - Analog_values[5] ;
	AnalogData[6] = Analog_values[4] ;
	AnalogData[7] = 4096 - Analog_values[9] ;
	AnalogData[8] = 4096 - Analog_values[13] ;
	AnalogData[9] = 4096 - Analog_values[12] ;
	AnalogData[10] = Analog_values[6] ;
	AnalogData[11] = Analog_values[7] ;
	AnalogData[12] = Analog_values[8] ;
	if (ADC->CCR & ADC_CCR_VBATE )
	{
		VbattRtc = Analog_values[10] ;
	}
#endif	// REV9E

#endif // PCBXLITE

	return ( i < 20000 ) ? 1 : 0 ;
}

#ifndef REV9E
#ifndef PCB9XT

#ifndef PCBXLITE
// This to read a single channel for use as a rotary encoder
// Channel is POT_L (6) or POT_R (8) or POT_3 (9)
void init_adc2()
{
	ADC2->CR2 = 0 ;
	TIM5->CR1 = 0 ;
	if ( g_eeGeneral.analogMapping & ENC_MASK )
	{
		uint32_t channel = (g_eeGeneral.analogMapping & ENC_MASK ) + 5 ; // gives 6, 7 or 8
		if ( channel > 6 )
		{
			channel += 1 ;	// 7->8 and 8->9
		}
		RCC->APB2ENR |= RCC_APB2ENR_ADC2EN ;			// Enable clock

		ADC2->CR1 = ADC_CR1_SCAN | ADC_CR1_EOCIE ;
		ADC2->SQR1 = (1-1) << 20 ;		// NUMBER_ANALOG Channels
		ADC2->SQR2 = 0 ;
		ADC2->SQR3 = channel ;

		ADC2->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12)
									+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) ;
		ADC2->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) 
									+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27) ;

		ADC2->CR2 = ADC_CR2_ADON | ADC_CR2_EXTSEL_3 | ADC_CR2_EXTSEL_1 | ADC_CR2_EXTEN_0 ;
	
  	NVIC_SetPriority(ADC_IRQn, 7);
		NVIC_EnableIRQ(ADC_IRQn) ;

	// TIM5, CC1 event is ADC2 trigger
		RCC->APB1ENR |= RCC_APB1ENR_TIM5EN ;		// Enable clock
		TIM5->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ;		// 0.5uS
		TIM5->ARR = 1999 ;
		TIM5->CCR1 = 1000 ;
		TIM5->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 ;
		TIM5->EGR = TIM_EGR_CC1G ;
		TIM5->CCER = TIM_CCER_CC1E ;
		TIM5->CR1 = TIM_CR1_CEN ;
	}
	else
	{
		RCC->APB2ENR &= ~RCC_APB2ENR_ADC2EN ;		// Disable clock
		RCC->APB1ENR &= ~RCC_APB1ENR_TIM5EN ;		// Disable clock
	}
}
#endif // nPCBXLITE
#endif // nPCB9XT
#endif // nREV9E

#ifndef PCB9XT
#ifdef REV9E
void init_adc3()
{
	RCC->APB2ENR |= RCC_APB2ENR_ADC3EN ;			// Enable clock
	RCC->AHB1ENR |= RCC_AHB1Periph_GPIOADC ;	// Enable ports A&C clocks (and B for REVPLUS)
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;		// Enable DMA2 clock
	configure_pins( PIN_FLAP3 | PIN_FLAP4 | PIN_FLAP5, PIN_ANALOG | PIN_PORTF ) ;

	ADC3->CR1 = ADC_CR1_SCAN ;
	ADC3->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS ;
	ADC3->SQR1 = (NUM_EXTRA_ANALOG-1) << 20 ;		// NUMBER_ANALOG Channels
	ADC3->SQR3 = FLAP_3 + (FLAP_4<<5) + (FLAP_5<<10) ;
	ADC3->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12)
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) ;
	ADC3->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) 
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27) ;
	ADC->CCR = 0 ; //ADC_CCR_ADCPRE_0 ;		// Clock div 2
	
  // Enable the DMA channel here, DMA2 stream 1, channel 2
	DMA2_Stream1->CR = DMA_SxCR_PL | DMA_SxCR_CHSEL_1 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC ;
	DMA2_Stream1->PAR = (uint32_t) &ADC3->DR ;
	DMA2_Stream1->M0AR = (uint32_t) &Analog_values[NUMBER_ANALOG] ;
	DMA2_Stream1->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0 ;
}
#endif	// REV9E
#endif // nPCB9XT

#ifndef PCB9XT
uint16_t RotaryAnalogValue ;

extern "C" void ADC_IRQHandler()
{
	uint32_t x ;
	x = ADC2->DR ;
	int32_t diff = x - RotaryAnalogValue ;
	if ( diff < 0 )
	{
		diff = -diff ;
	}
	RotaryAnalogValue = x ;
	if ( diff < 20 )
	{
		valueprocessAnalogEncoder( x >> 1 ) ;
	}
}
#endif // nPCB9XT

// TODO
void stop_adc()
{
}	

#ifdef PCB9XT

uint16_t AnalogSwitches ;
static uint16_t oldAnalog[3] ;

void processAnalogSwitches()
{
	uint32_t i ;
	for ( i = 0 ; i < 3 ; i += 1 )
	{
		uint32_t x ;
		x = Analog_values[i+5] ;
		int32_t diff = x - oldAnalog[i] ;
		if ( diff < 0 )
		{
			diff = -diff ;
		}
		oldAnalog[i] = x ;
		if ( diff < 20 )
		{
			uint32_t y ;
			if ( x < 2734 )
			{
				if ( x < 2316 )
				{
					y = ( x < 2129 ) ? 7 : 6 ;
				}
				else
				{
					y = ( x < 2536 ) ? 5 : 4 ;
				}
			}
			else
			{
				if ( x < 3353  )
				{
					y = ( x < 2975 ) ? 3 : 2 ;
				}
				else
				{
					y = ( x < 3835 ) ? 1 : 0 ;
				}
			}
			y <<= (i*3) ;
			uint16_t z = AnalogSwitches & ~(7 << (i*3)) ;
			AnalogSwitches = z | y ;
		}
	}
}

#endif // PCB9XT


