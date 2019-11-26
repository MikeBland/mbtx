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

#include <stm32f10x.h>

//#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBXLITE) || defined(PCBX9LITE)
//#include "X9D/stm32f2xx.h"
//#include "X9D/stm32f2xx_gpio.h"
//#include "X9D/stm32f2xx_rcc.h"
//#include "X9D/hal.h"
//#endif

#include "stm103/hal.h"

#ifndef SIMU
#include "core_cm3.h"
#endif

#include "ersky9x.h"
#include "timers.h"
#include "stm103/logicio103.h"
#include "analog.h"
#include "myeeprom.h"

#define STICK_LV	2
#define STICK_LH  3
#define STICK_RV  0
#define STICK_RH  1
#define POT				8
#define BATTERY		9


// Sample time should exceed 1uS
#define SAMPTIME	3		// sample time = 56 cycles

extern uint16_t AnalogData[] ;
uint16_t AdcBuffer[2] ;		// 2 for the on chip ADC + 1 for V_BATT
//volatile uint16_t Analog_values[NUMBER_ANALOG+NUM_POSSIBLE_EXTRA_POTS] ;
//uint16_t VbattRtc ;

void init_adc()
{
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN ;			// Enable clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN ; 			// Enable portA clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN ; 			// Enable portB clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN ;		// Enable clock

	configure_pins( 0x0F, PIN_INPUT | PIN_ANALOG | PIN_PORTA ) ;
	configure_pins( 0x03, PIN_INPUT | PIN_ANALOG | PIN_PORTB ) ;
	 
	RCC->CFGR |= RCC_CFGR_ADCPRE ;

	ADC1->CR2 = ADC_CR2_ADON | ADC_CR2_DMA ;
	ADC1->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12)
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) ;
	ADC1->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) 
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27) ;
	ADC1->SQR1 = 1 << 20 ;		// 2 Channels
	ADC1->SQR2 = 0 ;
	ADC1->SQR3 = POT | (BATTERY << 5) ; // Channel 1
	ADC1->JSQR = (STICK_LV << 15) + (STICK_LH << 10) + (STICK_RH << 5) + (STICK_RV) + (3 << 20) ;

	ADC1->CR1 = ADC_CR1_SCAN | ADC_CR1_JAUTO ;
		
	DMA1_Channel3->CCR = 0 ;
	DMA1_Channel1->CCR = DMA_CCR1_MSIZE_0 | DMA_CCR1_PSIZE_0 | DMA_CCR1_MINC ;
	DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR ;
	DMA1_Channel1->CMAR = (uint32_t)&AnalogData[4] ;
//	DMA1_Channel1->CCR &= ~DMA_CCR1_TCIE & ~DMA_CCR1_EN ;
	
}

uint32_t read_adc()
{
	uint32_t i ;
	int32_t j ;
	
	ADC1->SR &= ~(uint32_t) ( ADC_SR_EOC | ADC_SR_JEOC | ADC_SR_STRT ) ;

	DMA1->IFCR = DMA_IFCR_CTEIF1 | DMA_IFCR_CHTIF1 | DMA_IFCR_CTCIF1 | DMA_IFCR_CGIF1 ;
	DMA1_Channel3->CCR = 0 ;
	DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR ;
	DMA1_Channel1->CMAR = (uint32_t)&AdcBuffer[0] ;
	DMA1_Channel1->CNDTR = 2 ;
	DMA1_Channel1->CCR |= DMA_CCR1_MSIZE_0 | DMA_CCR1_PSIZE_0 | DMA_CCR1_MINC | DMA_CCR1_EN ;

	ADC1->CR2 |= ADC_CR2_ADON | ADC_CR2_DMA ;
	ADC1->CR2 |= (uint32_t)ADC_CR2_SWSTART ;
	for ( i = 0 ; i < 20000 ; i += 1 )
	{
		if ( ADC1->SR & ADC_SR_EOC )
		{
			break ;
		}
	}
	DMA1_Channel1->CCR &= ~DMA_CCR1_EN ;

//#define STICK_LH_GAIN	0x10
//#define STICK_LV_GAIN	0x40
//#define STICK_RV_GAIN	0x04
//#define STICK_RH_GAIN	0x01

	g_eeGeneral.stickGain |= 0x55 ;

	j = (4096-ADC1->JDR2) ;
	if ( g_eeGeneral.stickGain & STICK_LH_GAIN)
	{
		j = j * 2 - 2048 ;
	}
	if ( j < 0 )
	{
		j = 0 ;
	}
	if ( j > 4095 )
	{
		j = 4095 ;
	}
	AnalogData[0] = j ;
	j = (ADC1->JDR1) ;
	if ( g_eeGeneral.stickGain & STICK_LV_GAIN)
	{
		j = j * 2 - 2048 ;
	}
	if ( j < 0 )
	{
		j = 0 ;
	}
	if ( j > 4095 )
	{
		j = 4095 ;
	}
	AnalogData[1] = j ;
	j = (4096-ADC1->JDR4) ;
	if ( g_eeGeneral.stickGain & STICK_RV_GAIN)
	{
		j = j * 2 - 2048 ;
	}
	if ( j < 0 )
	{
		j = 0 ;
	}
	if ( j > 4095 )
	{
		j = 4095 ;
	}
	AnalogData[2] = j ;
	j = (ADC1->JDR3) ;
	if ( g_eeGeneral.stickGain & STICK_RH_GAIN)
	{
		j = j * 2 - 2048 ;
	}
	if ( j < 0 )
	{
		j = 0 ;
	}
	if ( j > 4095 )
	{
		j = 4095 ;
	}
	AnalogData[3] = j ;
	AnalogData[4] = 4096-AdcBuffer[0] ;
	AnalogData[12] = AdcBuffer[1] ;

	return ( i < 20000 ) ? 1 : 0 ;
}


