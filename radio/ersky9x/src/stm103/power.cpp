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
#include "logicio103.h"
#include "stm32f10x.h"
//#include "stm32f2xx_gpio.h"
//#include "stm32f2xx_rcc.h"

//#ifdef POWER_BUTTON
#define POWER_STATE_OFF				0
#define POWER_STATE_START			1
#define POWER_STATE_RUNNING		2
#define POWER_STATE_STOPPING	3
#define POWER_STATE_STOPPED		4

uint8_t PowerState = POWER_STATE_OFF ;
//#endif

//#ifdef PCBT12
//uint8_t PowerCount ;
//#endif

void soft_power_off()
{
	POWER_ON_GPIO->BRR = POWER_ON_Pin ;
}

uint32_t SoftPowerCalls ;
uint32_t SoftPowerOff ;

uint32_t check_soft_power()
{
//#ifdef POWER_BUTTON
	uint32_t switchValue ;
//#endif
//#if defined(SIMU)
//  return POWER_ON;
//#else
//#ifdef POWER_BUTTON
//#if defined(PCBXLITE) || defined(PCBX9LITE)
//	switchValue = GPIO_ReadInputDataBit(GPIOPWRSENSE, PIN_PWR_STATUS) == Bit_RESET ;
//#else
//	switchValue = GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) == Bit_RESET ;
	switchValue = ( GPIOE->IDR & 0x0800 ) ? 1 : 0 ;
//#endif
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
//#ifdef PCBT12
//				if ( ++PowerCount > 20 )
//				{
//					PowerState = POWER_STATE_STOPPING ;
//   				return POWER_X9E_STOP ;
//				}
//				else
//				{
//   				return POWER_ON ;
//				}
//#endif
				PowerState = POWER_STATE_STOPPING ;
   			return POWER_X9E_STOP ;
			}
//#ifdef PCBT12
//			else
//			{
//				PowerCount = 0 ;
//			}
//#endif
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
//#endif
}

void init_soft_power()
{
	configure_pins( POWER_ON_Pin, POWER_ON_PORT | PIN_OP2 | PIN_GP_PP | PIN_HIGH ) ;
	POWER_ON_GPIO->BSRR = POWER_ON_Pin ;
	PowerState = POWER_STATE_START ;
	configure_pins( DC_POWER_Pin, DC_POWER_PORT | PIN_INPUT ) ;
}

