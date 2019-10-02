/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Jean-Pierre Parisy
 * - Karl Szmutny <shadow@privy.de>
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * open9x is based on code named
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

#include <stdint.h>
#include "../ersky9x.h"
#include "hal.h"
#include "../logicio.h"
#include "stm32f2xx_gpio.h"
#include "stm32f2xx_rcc.h"

#ifdef POWER_BUTTON
#define POWER_STATE_OFF				0
#define POWER_STATE_START			1
#define POWER_STATE_RUNNING		2
#define POWER_STATE_STOPPING	3
#define POWER_STATE_STOPPED		4

uint8_t PowerState = POWER_STATE_OFF ;
#endif

#ifdef PCBT12
uint8_t PowerCount ;
#endif

void soft_power_off()
{
//#ifdef REV9E
//	while ( check_soft_power() == POWER_X9E_STOP )
//	{
//		wdt_reset() ;
//	}
//#endif
#ifdef PCB9XT
	configure_pins( PIN_MCU_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTC ) ;
#endif
  GPIO_ResetBits(GPIOPWR,PIN_MCU_PWR) ;
}

uint32_t SoftPowerCalls ;
uint32_t SoftPowerOff ;

uint32_t check_soft_power()
{
#ifdef POWER_BUTTON
	uint32_t switchValue ;
#endif
#if defined(SIMU)
  return POWER_ON;
#else
#ifdef POWER_BUTTON
#if defined(PCBXLITE) || defined(PCBX9LITE)
	switchValue = GPIO_ReadInputDataBit(GPIOPWRSENSE, PIN_PWR_STATUS) == Bit_RESET ;
#else
	switchValue = GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) == Bit_RESET ;
#endif
	switch ( PowerState )
	{
		case POWER_STATE_OFF :
		default :
			PowerState = POWER_STATE_START ;
   		return POWER_ON ;
		break ;
			
		case POWER_STATE_START :
			if ( !switchValue )
			{
				PowerState = POWER_STATE_RUNNING ;
			}
   		return POWER_ON ;
		break ;

		case POWER_STATE_RUNNING :
			if ( switchValue )
			{
#ifdef PCBT12
				if ( ++PowerCount > 20 )
				{
					PowerState = POWER_STATE_STOPPING ;
   				return POWER_X9E_STOP ;
				}
				else
				{
   				return POWER_ON ;
				}
#endif
				PowerState = POWER_STATE_STOPPING ;
   			return POWER_X9E_STOP ;
			}
#ifdef PCBT12
			else
			{
				PowerCount = 0 ;
			}
#endif
   		return POWER_ON ;
		break ;

		case POWER_STATE_STOPPING :
			if ( !switchValue )
			{
				PowerState = POWER_STATE_STOPPED ;
 				return POWER_OFF ;
			}
 			return POWER_X9E_STOP ;
		break ;

		case POWER_STATE_STOPPED :
 			return POWER_OFF ;
		break ;
	}

 #else // POWER_BUTTON
#ifdef PCB9XT
	SoftPowerCalls += 1 ;
	static uint32_t c1 = 0 ;
//	static uint32_t c2 = 0 ;
	uint16_t value = GPIOC->IDR ;
  if ( value & PIN_PWR_STATUS )
#else
  if (GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) == Bit_RESET)
#endif
	{
#ifdef PCB9XT
		c1 = 0 ;
#endif // PCB9XT
    return POWER_ON ;
	}
  else
	{
#ifdef PCB9XT
	  if (GPIO_ReadInputDataBit( GPIOPWR, PIN_MCU_PWR) == Bit_SET)
		{
			return POWER_TRAINER ;
		}
#endif
#ifdef PCB9XT
		SoftPowerOff += 1 ;
		c1 += 1 ;
//		static uint32_t counter = 0 ;
//		if ( ++counter > 250 )
//		{
//extern void txmit( uint8_t c ) ;
//			txmit('*') ;
//extern void p4hex( uint16_t value ) ;
//			p4hex( GPIOC->IDR ) ;
//		 	counter = 0 ;
//		}
		if ( c1 > 5 )
		{
    	return POWER_OFF;
		}
    return POWER_ON ;
#endif
    return POWER_OFF;
	}
 #endif // POWER_BUTTON
#endif
}

void init_soft_power()
{
#if defined(PCBXLITE)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
	
	GPIO_ResetBits(GPIOPWRINT, PIN_INT_RF_PWR );
	GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR);
	GPIO_ResetBits(GPIOPWRSPORT, PIN_SPORT_PWR);

	configure_pins( PIN_INT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;
	configure_pins( PIN_EXT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;
	configure_pins( PIN_MCU_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTE ) ;
	configure_pins( PIN_SPORT_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;

	configure_pins( PIN_PWR_STATUS, PIN_INPUT | PIN_PORTA ) ;

#else	
#ifdef PCBX9LITE
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
	
	GPIO_ResetBits(GPIOPWRINT, PIN_INT_RF_PWR );
	GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR);
	GPIO_ResetBits(GPIOPWRSPORT, PIN_SPORT_PWR);

	configure_pins( PIN_INT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
	configure_pins( PIN_EXT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
	configure_pins( PIN_MCU_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
	configure_pins( PIN_SPORT_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTE ) ;

	configure_pins( PIN_PWR_STATUS, PIN_INPUT | PIN_PORTA ) ;

#else // X3
#ifdef REV19
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
	
	GPIO_ResetBits(GPIOPWRINT, PIN_INT_RF_PWR );
	GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR);
	GPIO_ResetBits(GPIOPWRSPORT, PIN_SPORT_PWR);

	configure_pins( PIN_INT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
	configure_pins( PIN_EXT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;
	configure_pins( PIN_MCU_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;
	configure_pins( PIN_SPORT_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;

	configure_pins( PIN_PWR_STATUS, PIN_INPUT | PIN_PORTD ) ;

#else // REV19
//  GPIO_InitTypeDef GPIO_InitStructure;
  /* GPIOC GPIOD clock enable */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
 #if defined(REVPLUS) || defined(REV9E)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
 #endif
 #ifdef PCBX7
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
 #endif
 #ifdef PCB9XT
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
 #endif
	GPIO_ResetBits(GPIOPWRINT, PIN_INT_RF_PWR );
	GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR);
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOPWR, ENABLE);

  /* GPIO  Configuration*/
 #if defined(REVPLUS) || defined(REV9E)
	configure_pins( PIN_INT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTC ) ;
	configure_pins( PIN_EXT_RF_PWR | PIN_MCU_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;
 #else
  #ifdef PCBX7
	configure_pins( PIN_INT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTC ) ;
	configure_pins( PIN_EXT_RF_PWR | PIN_MCU_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;
  #else
   #ifdef PCB9XT
		configure_pins( PIN_INT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTC ) ;
		configure_pins( PIN_EXT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;
   #else
		configure_pins( PIN_INT_RF_PWR | PIN_EXT_RF_PWR | PIN_MCU_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;
   #endif
  #endif
 #endif
//	configure_pins( PIN_INT_RF_PWR | PIN_EXT_RF_PWR, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;

//  GPIO_InitStructure.GPIO_Pin = PIN_MCU_PWR;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
//  GPIO_Init(GPIOPWR, &GPIO_InitStructure);

#ifdef PCB9XT
	configure_pins( PIN_PWR_STATUS, PIN_INPUT | PIN_PORTC ) ;
#else
	configure_pins( PIN_PWR_STATUS, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
#endif
	
#ifdef PCB9XT
	configure_pins( PIN_MCU_PWR, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
#else
	configure_pins( PIN_TRNDET, PIN_INPUT | PIN_PULLUP | PIN_PORTA ) ;
#endif
  
//	GPIO_InitStructure.GPIO_Pin = PIN_PWR_STATUS;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
//  GPIO_Init(GPIOPWR, &GPIO_InitStructure);
  
//	configure_pins( PIN_PWR_LED, PIN_OUTPUT | PIN_OS100 | PIN_PUSHPULL | PIN_PORTC ) ;
//  GPIO_InitStructure.GPIO_Pin = PIN_PWR_LED;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
//  GPIO_Init(GPIOPWRLED, &GPIO_InitStructure);
  
  // Soft power ON
  
#endif // REV19
#endif // X9LITE
#endif // XLITE
	
// Not yet!!!!*********	
#ifndef PCB9XT
	GPIO_SetBits(GPIOPWR,PIN_MCU_PWR);
#endif
#ifdef POWER_BUTTON
	PowerState = POWER_STATE_START ;
#endif

}

