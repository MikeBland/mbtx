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


#ifdef PCBSKY
#include "AT91SAM3S4.h"
#endif

#if defined(PCBX9D) || defined(PCB9XT)
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/hal.h"
#endif


#ifndef SIMU
#include "core_cm3.h"
#endif

#include "ersky9x.h"
#include "drivers.h"
#include "logicio.h"

#ifdef PCBSKY
#include "lcd.h"
#endif

#ifdef PCB9XT
#include "analog.h"
#include "mega64.h"
#endif

#include "myeeprom.h"


#ifndef SIMU
#ifdef PCBSKY
void configure_pins( uint32_t pins, uint16_t config )
{
	register Pio *pioptr ;
	
	pioptr = PIOA + ( ( config & PIN_PORT_MASK ) >> 6) ;
	if ( config & PIN_PULLUP )
	{
		pioptr->PIO_PPDDR = pins ;
		pioptr->PIO_PUER = pins ;
	}
	else
	{
		pioptr->PIO_PUDR = pins ;
	}

	if ( config & PIN_PULLDOWN )
	{
		pioptr->PIO_PUDR = pins ;
		pioptr->PIO_PPDER = pins ;
	}
	else
	{
		pioptr->PIO_PPDDR = pins ;
	}

	if ( config & PIN_HIGH )
	{
		pioptr->PIO_SODR = pins ;		
	}
	else
	{
		pioptr->PIO_CODR = pins ;		
	}

	if ( config & PIN_INPUT )
	{
		pioptr->PIO_ODR = pins ;
	}
	else
	{
		pioptr->PIO_OER = pins ;
	}

	if ( config & PIN_PERI_MASK_L )
	{
		pioptr->PIO_ABCDSR[0] |= pins ;
	}
	else
	{
		pioptr->PIO_ABCDSR[0] &= ~pins ;
	}
	if ( config & PIN_PERI_MASK_H )
	{
		pioptr->PIO_ABCDSR[1] |= pins ;
	}
	else
	{
		pioptr->PIO_ABCDSR[1] &= ~pins ;
	}

	if ( config & PIN_ENABLE )
	{
		pioptr->PIO_PER = pins ;		
	}
	else
	{
		pioptr->PIO_PDR = pins ;		
	}
	
	if ( config & PIN_ODRAIN )
	{
		pioptr->PIO_MDER = pins ;		
	}
	else
	{
		pioptr->PIO_MDDR = pins ;		
	}
}
#endif


#if defined(PCBX9D) || defined(PCB9XT)
void configure_pins( uint32_t pins, uint16_t config )
{
	uint32_t address ;
	GPIO_TypeDef *pgpio ;
	uint32_t thispin ;
  uint32_t pos ;

	address = ( config & PIN_PORT_MASK ) >> 8 ;
	address *= (GPIOB_BASE-GPIOA_BASE) ;
	address += GPIOA_BASE ;
	pgpio = (GPIO_TypeDef* ) address ;
	
  /* -------------------------Configure the port pins---------------- */
  /*-- GPIO Mode Configuration --*/
  for (thispin = 0x0001, pos = 0; thispin < 0x10000; thispin <<= 1, pos +=1 )
  {
    if ( pins & thispin)
    {
      pgpio->MODER  &= ~(GPIO_MODER_MODER0 << (pos * 2)) ;
      pgpio->MODER |= (config & PIN_MODE_MASK) << (pos * 2) ;

      if ( ( (config & PIN_MODE_MASK ) == PIN_OUTPUT) || ( (config & PIN_MODE_MASK) == PIN_PERIPHERAL) )
      {
        /* Speed mode configuration */
        pgpio->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 << (pos * 2)) ;
        pgpio->OSPEEDR |= ((config & PIN_SPEED_MASK) >> 13 ) << (pos * 2) ;

        /* Output mode configuration*/
        pgpio->OTYPER  &= ~((GPIO_OTYPER_OT_0) << ((uint16_t)pos)) ;
				if ( config & PIN_ODRAIN )
				{
        	pgpio->OTYPER |= (GPIO_OTYPER_OT_0) << pos ;
				}
      }
      /* Pull-up Pull down resistor configuration*/
      pgpio->PUPDR &= ~(GPIO_PUPDR_PUPDR0 << ((uint16_t)pos * 2));
      pgpio->PUPDR |= ((config & PIN_PULL_MASK) >> 2) << (pos * 2) ;

			pgpio->AFR[pos >> 3] &= ~(0x000F << ((pos & 7)*4)) ;
			pgpio->AFR[pos >> 3] |=	((config & PIN_PERI_MASK) >> 4) << ((pos & 7)*4) ;
    }
  }
}
		
#endif



#ifdef PCBSKY

void initExtraInput()
{
	configure_pins( 0x00004000, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;
}
	 
void init_keys()
{
	register Pio *pioptr ;
	
	pioptr = PIOC ;
	// Next section configures the key inputs on the LCD data
#ifdef REVB	
	pioptr->PIO_PER = 0x0000003BL ;		// Enable bits 1,3,4,5, 0
	pioptr->PIO_OER = PIO_PC0 ;		// Set bit 0 output
	pioptr->PIO_ODR = 0x0000003AL ;		// Set bits 1, 3, 4, 5 input
	pioptr->PIO_PUER = 0x0000003AL ;		// Set bits 1, 3, 4, 5 with pullups
#else	
	pioptr->PIO_PER = 0x0000003DL ;		// Enable bits 2,3,4,5, 0
	pioptr->PIO_OER = PIO_PC0 ;		// Set bit 0 output
	pioptr->PIO_ODR = 0x0000003CL ;		// Set bits 2, 3, 4, 5 input
	pioptr->PIO_PUER = 0x0000003CL ;		// Set bits 2, 3, 4, 5 with pullups
#endif

	pioptr = PIOB ;
#ifdef REVB	
	pioptr->PIO_PUER = PIO_PB5 ;					// Enable pullup on bit B5 (MENU)
	pioptr->PIO_PER = PIO_PB5 ;					// Enable bit B5
#else	
	pioptr->PIO_PUER = PIO_PB6 ;					// Enable pullup on bit B6 (MENU)
	pioptr->PIO_PER = PIO_PB6 ;					// Enable bit B6
#endif
}

// Assumes PMC has already enabled clocks to ports
void setup_switches()
{
#ifdef REVB
#else
	register Pio *pioptr ;
	
	pioptr = PIOA ;
#endif
#ifdef REVB
	configure_pins( 0x01808087, PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
#else 
	pioptr->PIO_PER = 0xF8008184 ;		// Enable bits
	pioptr->PIO_ODR = 0xF8008184 ;		// Set bits input
	pioptr->PIO_PUER = 0xF8008184 ;		// Set bits with pullups
#endif 
#ifdef REVB
#else
	pioptr = PIOB ;
#endif 
#ifdef REVB
	configure_pins( 0x00000030, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;
#else 
	pioptr->PIO_PER = 0x00000010 ;		// Enable bits
	pioptr->PIO_ODR = 0x00000010 ;		// Set bits input
	pioptr->PIO_PUER = 0x00000010 ;		// Set bits with pullups
#endif 

#ifdef REVB
#else
	pioptr = PIOC ;
#endif 
#ifdef REVB
	configure_pins( 0x91114900, PIN_ENABLE | PIN_INPUT | PIN_PORTC | PIN_PULLUP ) ;
#else 
	pioptr->PIO_PER = 0x10014900 ;		// Enable bits
	pioptr->PIO_ODR = 0x10014900 ;		// Set bits input
	pioptr->PIO_PUER = 0x10014900 ;		// Set bits with pullups
#endif 

}


// Switch input pins
// Needs updating for REVB board ********
// AIL-DR  PA2
// TRIM_LH_DOWN PA7 (PA23)
// ELE_DR   PA8 (PC31)
// RUN_DR   PA15
// TRIM_LV_DOWN  PA27 (PA24)
// SW_TCUT     PA28 (PC20)
// TRIM_RH_DOWN    PA29 (PA0)
// TRIM_RV_UP    PA30 (PA1)
// TRIM_LH_UP    //PB4
// SW-TRAIN    PC8
// TRIM_RH_UP   PC9
// TRIM_RV_DOWN   PC10
// SW_IDL2     PC11
// SW_IDL1     PC14
// SW_GEAR     PC16
// TRIM_LV_UP   PC28

// KEY_MENU    PB6 (PB5)
// KEY_EXIT    PA31 (PC24)
// Shared with LCD data
// KEY_DOWN  LCD5  PC3
// KEY_UP    LCD6  PC2
// KEY_RIGHT LCD4  PC4
// KEY_LEFT  LCD3  PC5

// PORTA 1111 1000 0000 0000 1000 0001 1000 0100 = 0xF8008184 proto
// PORTA 0000 0001 1000 0000 1000 0000 0000 0111 = 0x01808087 REVB
// PORTB 0000 0000 0001 0000										 = 0x0010     proto
// PORTB 0000 0000 0010 0000										 = 0x0030     REVB
// PORTC 0001 0000 0000 0001 0100 1001 0000 0000 = 0x10014900 proto
// PORTC 1001 0001 0001 0001 0100 1001 0000 0000 = 0x91114900 REVB


// Prototype
// Free pins (PA16 is stock buzzer)
// PA23, PA24, PA25, PB7, PB13
// PC20, PC21(labelled 17), PC22, PC24
// REVB
// PA25, use for stock buzzer
// PB14, PB6
// PC21, PC19, PC15 (PPM2 output)
void config_free_pins()
{
	
#ifdef REVB
//	configure_pins( PIO_PB6 | PIO_PB14, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;
	configure_pins( PIO_PB14, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;

//	configure_pins( PIO_PC19 | PIO_PC21, PIN_ENABLE | PIN_INPUT | PIN_PORTC | PIN_PULLUP ) ;	// 19 and 21 are rotary encoder
#else 
	register Pio *pioptr ;

	pioptr = PIOA ;
	pioptr->PIO_PER = 0x03800000L ;		// Enable bits A25,24,23
	pioptr->PIO_ODR = 0x03800000L ;		// Set as input
	pioptr->PIO_PUER = 0x03800000L ;	// Enable pullups

	pioptr = PIOB ;
	pioptr->PIO_PER = 0x00002080L ;		// Enable bits B13, 7
	pioptr->PIO_ODR = 0x00002080L ;		// Set as input
	pioptr->PIO_PUER = 0x00002080L ;	// Enable pullups

	pioptr = PIOC ;
	pioptr->PIO_PER = 0x01700000L ;		// Enable bits C24,22,21,20
	pioptr->PIO_ODR = 0x01700000L ;		// Set as input
	pioptr->PIO_PUER = 0x01700000L ;	// Enable pullups
#endif 
}



#endif

#endif // 

// keys:
// KEY_EXIT    PA31 (PC24)
// KEY_MENU    PB6 (PB5)
// KEY_DOWN  LCD5  PC3 (PC5)
// KEY_UP    LCD6  PC2 (PC1)
// KEY_RIGHT LCD4  PC4 (PC4)
// KEY_LEFT  LCD3  PC5 (PC3)
// Reqd. bit 6 LEFT, 5 RIGHT, 4 UP, 3 DOWN 2 EXIT 1 MENU
// LCD pins 5 DOWN, 4 RIGHT, 3 LEFT, 1 UP

#ifdef PCB9XT

// Switches from M64:
// ELE, RUD, TRAIN
// M64Switches, bit 8=Train, bit 1=RUD, bit 2=ELE


void init_trims()
{
// Trims 
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
	configure_pins( 0x6000, PIN_INPUT | PIN_PULLUP | PIN_PORTA ) ;
	configure_pins( 0x2080, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
}

uint32_t read_keys()
{
	checkM64() ;
	return ~M64Buttons ;
}

uint32_t readKeyUpgradeBit( uint8_t index )
{
  CPU_UINT xxx = 0 ;
	uint32_t t = 1 << (index-1) ;

//	if ( t > 16 )
//	{
//		t >>= 2 ;
//	}

	xxx = M64Trims & t ;
	return xxx ;
}

uint32_t hwKeyState( uint8_t key )
{
  CPU_UINT xxx = 0 ;
	register uint32_t a ;
	uint32_t avpot = 0xFFFF ;
  
	a = g_eeGeneral.analogMapping & MASK_6POS ;
	if ( a )
	{
		avpot = M64Analog[ (a >> 2) + 3] ;
		if ( g_eeGeneral.SixPositionCalibration[5] >  g_eeGeneral.SixPositionCalibration[0] )
		{
			avpot = 2047 - avpot ;
		}
	}
	
	if( key > HSW_MAX )  return 0 ;

	if ( ( key >= HSW_ThrCt ) && ( key <= HSW_Trainer ) )
	{
		return keyState( (EnumKeys)(key + ( SW_ThrCt - HSW_ThrCt ) ) ) ;
	}
	
	// Add other options here

	xxx = 0 ; 
	uint32_t as = AnalogSwitches ;

	switch(key)
	{
		case HSW_Thr3pos0 :
    	xxx = as & AS_THR_SW ;	// SW_TCUT     PC20
    break ;
		
		case HSW_Thr3pos1 :
			xxx = ~as & AS_THR_SW ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.thrsource ) ;
    break ;

		case HSW_Thr3pos2 :
			xxx = readKeyUpgradeBit( g_eeGeneral.thrsource ) ;
    break ;
			 
		case HSW_Ele3pos0 :
			xxx = ~M64Switches & 0x0004 ;	// ELE_DR
//			xxx = ( g_eeGeneral.elesource == 5 ) ? av9 < 490 : ~c & 0x80000000 ;	// ELE_DR   PC31
    break ;

		case HSW_Ele3pos1 :
//			if ( g_eeGeneral.elesource == 5 )
//			{
//				xxx = av9 > 1500 ;
//			}
//      else
//			{
				xxx = M64Switches & 0x0004 ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.elesource ) ;
//			}
		break ;

		case HSW_Ele3pos2 :
//			if ( g_eeGeneral.elesource == 5 )
//			{
//				xxx = ( av9 <= 1500 ) && ( av9 >= 490 ) ;
//			}
//      else
//			{
				xxx = readKeyUpgradeBit( g_eeGeneral.elesource ) ;
//			}
    break ;

		case HSW_Rud3pos0 :
		 xxx = ~M64Switches & 0x0002	;	// RUD_DR   PA15
    break ;

		case HSW_Rud3pos1 :
			xxx = M64Switches & 0x0002 ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.rudsource ) ;
//			xxx = (a & 0x00008000) ; if ( xxx ) xxx = (c & 0x80000000) ;
    break ;

		case HSW_Rud3pos2 :
			xxx = readKeyUpgradeBit( g_eeGeneral.rudsource ) ;
//			xxx = ~c & 0x80000000 ;	// ELE_DR   PC31
    break ;

		case HSW_Ail3pos0 :
			xxx = as & AS_AIL_SW ;	// AIL-DR  PA2
    break ;

		case HSW_Ail3pos1 :
//			xxx = (a & 0x00000004) ; if ( xxx ) xxx = (PIOB->PIO_PDSR & 0x00004000) ;
			xxx = ~as & AS_AIL_SW ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.ailsource ) ;
    break ;                               

		case HSW_Ail3pos2 :
//			xxx = ~PIOB->PIO_PDSR & 0x00004000 ;
			xxx = readKeyUpgradeBit( g_eeGeneral.ailsource ) ;
    break ;

		case HSW_Gear3pos0 :
			xxx = as & AS_GEA_SW ;	// SW_GEAR     PC16
    break ;

		case HSW_Gear3pos1 :
//			xxx = (c & 0x00010000) ; if ( xxx ) xxx = (PIOB->PIO_PDSR & 0x00004000) ;
			xxx = ~as & AS_GEA_SW ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.geasource ) ;
    break ;

		case HSW_Gear3pos2 :
			xxx = readKeyUpgradeBit( g_eeGeneral.geasource ) ;
//			xxx = ~PIOB->PIO_PDSR & 0x00004000 ;
    break ;

		case HSW_Pb1 :
			xxx = readKeyUpgradeBit( g_eeGeneral.pb1source ) ;
    break ;
			 
		case HSW_Pb2 :
			xxx = readKeyUpgradeBit( g_eeGeneral.pb2source ) ;
    break ;

		case HSW_Pb3 :
			xxx = readKeyUpgradeBit( g_eeGeneral.pb3source ) ;
    break ;
			 
		case HSW_Pb4 :
			xxx = readKeyUpgradeBit( g_eeGeneral.pb4source ) ;
    break ;

		case HSW_Ele6pos0 :
			
			if ( avpot != 0xFFFF )
			{
				xxx = avpot > SixPositionTable[0] ;
			}
    break ;

		case HSW_Ele6pos1 :
			if ( avpot != 0xFFFF )
			{
				xxx = ( avpot <= SixPositionTable[0] ) && ( avpot >= SixPositionTable[1] ) ;
			}
    break ;
		
		case HSW_Ele6pos2 :
			if ( avpot != 0xFFFF )
			{
				xxx = ( avpot <= SixPositionTable[1] ) && ( avpot >= SixPositionTable[2] ) ;
			}
    break ;
		
		case HSW_Ele6pos3 :
			if ( avpot != 0xFFFF )
			{
				xxx = ( avpot <= SixPositionTable[2] ) && ( avpot >= SixPositionTable[3] ) ;
			}
    break ;
		
		case HSW_Ele6pos4 :
			if ( avpot != 0xFFFF )
			{
				xxx = ( avpot <= SixPositionTable[3] ) && ( avpot >= SixPositionTable[4] ) ;
			}
    break ;

		case HSW_Ele6pos5 :
			if ( avpot != 0xFFFF )
			{
				xxx = avpot < SixPositionTable[4] ;
			}
    break ;
		 
	}
	return xxx ;
}

uint32_t keyState(EnumKeys enuk)
{
	uint32_t as ;
  CPU_UINT xxx = 0 ;
  if(enuk < (int)DIM(keys))  return keys[enuk].state() ? 1 : 0 ;
	
	as = AnalogSwitches ;

	switch((uint8_t)enuk)
	{
    case SW_AileDR : xxx = ~as & AS_AIL_SW ;	// AIL-DR
    break ;
    case SW_ID0    : xxx = as & AS_IDL1_SW ;	// SW_IDL1
    break ;
    case SW_ID1    : xxx = (~as & AS_IDL1_SW) ; if ( xxx ) xxx = (~as & AS_IDL2_SW) ;
    break ;
    case SW_ID2    : xxx = as & AS_IDL2_SW ;	// SW_IDL2
    break ;
		case SW_Gear   : xxx = ~as & AS_GEA_SW ;	// SW_GEAR
    break ;
    case SW_ThrCt  : xxx = ~as & AS_THR_SW ;	// SW_TCUT
    break ;
		case SW_ElevDR : xxx = M64Switches & 0x0004 ;	// ELE_DR
    break ;
    case SW_RuddDR : xxx = M64Switches & 0x0002 ;	// RUD_DR
    break ;
    case SW_Trainer: xxx = M64Switches & 0x00000100 ;	// SW-TRAIN
    break ;
	}
	
  if ( xxx )
  {
    return 1 ;
  }
	return 0 ;
}

uint32_t switchPosition( uint32_t swtch )
{
	if ( hwKeyState( swtch ) )
	{
		return 0 ;
	}
	swtch += 1 ;
	if ( hwKeyState( swtch ) )
	{
		return 1 ;			
	}
	if ( swtch == HSW_Ele6pos1 )
	{
		if ( hwKeyState( HSW_Ele6pos3 ) )
		{
			return 3 ;
		}
		if ( hwKeyState( HSW_Ele6pos4 ) )
		{
			return 4 ;
		}
		if ( hwKeyState( HSW_Ele6pos5 ) )
		{
			return 5 ;
		}
	}
	return 2 ;
}

void init_keys()
{
	// Nothing to do
}

void setup_switches()
{
	// Nothing to do
}

uint32_t read_trims()
{
	uint32_t trims ;
	uint32_t trima ;

	trims = 0 ;

	trima = GPIOC->IDR ;

// TRIM_LH_DOWN
	if ( ( trima & 0x00000080 ) == 0 )
	{
		trims |= 1 ;
	}

// TRIM_LH_UP
	if ( ( trima & 0x00002000 ) == 0 )
	{
		trims |= 2 ;
	}
  
	trima = GPIOA->IDR ;

// TRIM_RH_DOWN
	if ( ( trima & 0x00004000 ) == 0 )
	{
		trims |= 0x40 ;
	}

// TRIM_RH_UP
	if ( ( trima & 0x00002000 ) == 0 )
	{
		trims |= 0x80 ;
	}

	trima = AnalogSwitches ;

// TRIM_LV_UP
	if ( trima & AS_LV_TRIM_UP )
	{
		trims |= 8 ;
	}

// TRIM_RV_UP
	if ( trima & AS_RV_TRIM_UP )
	{
		trims |= 0x20 ;
	}

// TRIM_RV_DOWN
	if ( trima & AS_RV_TRIM_DN )
	{
		trims |= 0x10 ;
	}

// TRIM_LV_DOWN
	if ( trima & AS_LV_TRIM_DN )
	{
		trims |= 4 ;
	}

	return trims ;
}

#endif

#ifdef PCBSKY

uint16_t ExtraInputs ;

uint32_t read_keys()
{
	register uint32_t x ;
	register uint32_t y ;

	x = LcdLock ? LcdInputs : PIOC->PIO_PDSR << 1 ; // 6 LEFT, 5 RIGHT, 4 DOWN, 3 UP ()
#ifdef REVB
	y = x & 0x00000020 ;		// RIGHT
	if ( x & 0x00000004 )
	{
		y |= 0x00000010 ;			// UP
	}
	if ( x & 0x00000010 )
	{
		y |= 0x00000040 ;			// LEFT
	}
	if ( x & 0x00000040 )
	{
		y |= 0x00000008 ;			// DOWN
	}
	x = ~x ;
	if ( x & 8 )
	{
		x |= 0x40 ;
	}
	x >>= 6 ;
	x &= 7 ;
#ifndef REVX
extern uint8_t Co_proc_status[] ;
	uint8_t temp = (uint8_t)Co_proc_status[9] ;
	if ( (temp & 0x40) == 0 )
	{
		x |= 0x20 ;
	}
	if ( (temp & 0x08) == 0 )
	{
		x |= 0x10 ;
	}
	if ( (temp & 0x02) == 0 )
	{
		x |= 0x08 ;
	}
#endif

	uint32_t av9 = Analog_values[9] ;
	if ( av9 < 490 )
	{
		x |= 0x40 ;
	}
	else
	{
		if ( av9 > 1500 )
		{
			x |= 0x80 ;
		}
		else
		{
			x |= 0x0100 ;
		}
	}

	ExtraInputs = x ;
#else	
	y = x & 0x00000060 ;
	if ( x & 0x00000008 )
	{
		y |= 0x00000010 ;
	}
	if ( x & 0x00000010 )
	{
		y |= 0x00000008 ;
	}
	x = ~x ;
	if ( x & 8 )
	{
		x |= 0x40 ;
	}
	x >>= 6 ;
	x &= 7 ;
extern uint8_t Co_proc_status[] ;
	uint8_t temp = (uint8_t)Co_proc_status[9] ;
	if ( (temp & 0x40) == 0 )
	{
		x |= 0x20 ;
	}
	if ( (temp & 0x08) == 0 )
	{
		x |= 0x10 ;
	}
	if ( (temp & 0x02) == 0 )
	{
		x |= 0x08 ;
	}
	ExtraInputs = x ;
#endif
#ifdef REVB
	if ( PIOC->PIO_PDSR & 0x01000000 )
#else 
	if ( PIOA->PIO_PDSR & 0x80000000 )
#endif
	{
		y |= 4 ;		// EXIT
	}
#ifdef REVB
	if ( PIOB->PIO_PDSR & 0x000000020 )
#else 
	if ( PIOB->PIO_PDSR & 0x000000040 )
#endif
	{
		y |= 2 ;		// MENU
	}
	return y ;
}
#endif

#ifdef PCBSKY
uint32_t read_trims()
{
	uint32_t trims ;
	uint32_t trima ;

	trims = 0 ;

	trima = PIOA->PIO_PDSR ;
// TRIM_LH_DOWN    PA7 (PA23)
#ifdef REVB
 #ifndef REVX
	if ( ( trima & 0x00800000 ) == 0 )
 #else
	if ( ( PIOB->PIO_PDSR & 0x10 ) == 0 )
 #endif
#else
	if ( ( trima & 0x0080 ) == 0 )
#endif
	{
		trims |= 1 ;
	}
    
// TRIM_LV_DOWN  PA27 (PA24)
#ifdef REVB
	if ( ( trima & 0x01000000 ) == 0 )
#else
	if ( ( trima & 0x08000000 ) == 0 )
#endif
	{
		trims |= 4 ;
	}

// TRIM_RV_UP    PA30 (PA1)
#ifdef REVB
 #ifndef REVX
	if ( ( trima & 0x00000002 ) == 0 )
 #else
	if ( ( PIOC->PIO_PDSR & 0x00000400 ) == 0 )
 #endif
#else
	if ( ( trima & 0x40000000 ) == 0 )
#endif
	{
		trims |= 0x20 ;
	}

// TRIM_RH_DOWN    PA29 (PA0)
#ifdef REVB
	if ( ( trima & 0x00000001 ) == 0 )
#else 
	if ( ( trima & 0x20000000 ) == 0 )
#endif 
	{
		trims |= 0x40 ;
	}

// TRIM_LH_UP PB4
#ifndef REVX
	if ( ( PIOB->PIO_PDSR & 0x10 ) == 0 )
#else
	if ( ( trima & 0x00800000 ) == 0 )
#endif
	{
		trims |= 2 ;
	}

	trima = PIOC->PIO_PDSR ;
// TRIM_LV_UP   PC28
	if ( ( trima & 0x10000000 ) == 0 )
	{
		trims |= 8 ;
	}

// TRIM_RV_DOWN   PC10
#ifndef REVX
	if ( ( trima & 0x00000400 ) == 0 )
#else
	if ( ( PIOA->PIO_PDSR & 0x00000002 ) == 0 )
#endif
	{
		trims |= 0x10 ;
	}

// TRIM_RH_UP   PC9
	if ( ( trima & 0x00000200 ) == 0 )
	{
		trims |= 0x80 ;
	}

	return trims ;
}

#endif


#ifdef PCBSKY

uint32_t readKeyUpgradeBit( uint8_t index )
{
  CPU_UINT xxx = 0 ;
	uint32_t t = 1 << (index-1) ;
	if ( t == 8 )
	{
		xxx = (~PIOB->PIO_PDSR & 0x00004000) ;	// DAC1
	}
	else if ( t == 16 )
	{
		xxx = ~PIOC->PIO_PDSR & 0x80000000 ;	// ELE_DR   PC31	
	}
	else
	{
		if ( t > 16 )
		{
			t >>= 2 ;
		}
		xxx = ExtraInputs & t ;
	}
	return xxx ;
}

uint32_t hwKeyState( uint8_t key )
{
	register uint32_t a ;
	register uint32_t c ;
	uint32_t av9 = Analog_values[9] ;
	uint32_t avpot = 0xFFFF ;
	uint32_t anaIndex = g_eeGeneral.ar9xBoard ? 6 : 9 ;

	a = g_eeGeneral.analogMapping & MASK_6POS ;
	if ( a )
	{
		if ( a == USE_AUX_6POS )
		{
			avpot = av9 ;
		}
		else
		{
			avpot = Analog_values[ (a >> 2) + 3] ;
		}
		if ( g_eeGeneral.SixPositionCalibration[5] >  g_eeGeneral.SixPositionCalibration[0] )
		{
			avpot = 4095 - avpot ;
		}
	}

  CPU_UINT xxx = 0 ;
  if( key > HSW_MAX )  return 0 ;

	if ( ( key >= HSW_ThrCt ) && ( key <= HSW_Trainer ) )
	{
		return keyState( (EnumKeys)(key + ( SW_ThrCt - HSW_ThrCt ) ) ) ;
	}

	a = PIOA->PIO_PDSR ;
	c = PIOC->PIO_PDSR ;
	switch(key)
	{
//#ifdef REVB
//    case HSW_ElevDR : xxx = c & 0x80000000 ;	// ELE_DR   PC31
//#else 
//    case HSW_ElevDR : xxx = a & 0x00000100 ;	// ELE_DR   PA8
//#endif 
//    break ;
    
//    case HSW_AileDR : xxx = a & 0x00000004 ;	// AIL-DR  PA2
//    break ;

//    case HSW_RuddDR : xxx = a & 0x00008000 ;	// RUD_DR   PA15
//    break ;
//      //     INP_G_ID1 INP_E_ID2
//      // id0    0        1
//      // id1    1        1
//      // id2    1        0
//    case HSW_ID0    : xxx = ~c & 0x00004000 ;	// SW_IDL1     PC14
//    break ;
//    case HSW_ID1    : xxx = (c & 0x00004000) ; if ( xxx ) xxx = (PIOC->PIO_PDSR & 0x00000800);
//    break ;
//    case HSW_ID2    : xxx = ~c & 0x00000800 ;	// SW_IDL2     PC11
//    break ;

    
//		case HSW_Gear   : xxx = c & 0x00010000 ;	// SW_GEAR     PC16
//    break ;

//#ifdef REVB
//    case HSW_ThrCt  : xxx = c & 0x00100000 ;	// SW_TCUT     PC20
//#else 
//    case HSW_ThrCt  : xxx = a & 0x10000000 ;	// SW_TCUT     PA28
//#endif 
//    break ;

//    case HSW_Trainer: xxx = c & 0x00000100 ;	// SW-TRAIN    PC8
//    break ;
		
		case HSW_Thr3pos0 :
			if ( g_eeGeneral.thrsource == anaIndex )
			{
				xxx = ExtraInputs & 0x40 ;
			}
      else
			{
	    	xxx = ~c & 0x00100000 ;	// SW_TCUT     PC20
			}
    break ;
		
		case HSW_Thr3pos1 :
			if ( g_eeGeneral.thrsource == anaIndex )
			{
				xxx = ExtraInputs & 0x80 ;
			}
      else
			{
				xxx = (c & 0x00100000) ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.thrsource ) ;
			}
    break ;

		case HSW_Thr3pos2 :
			if ( g_eeGeneral.thrsource == anaIndex )
			{
				xxx = ExtraInputs & 0x100 ;
			}
      else
			{
				xxx = readKeyUpgradeBit( g_eeGeneral.thrsource ) ;
			}
    break ;
			 
		case HSW_Ele3pos0 :
			xxx = ( g_eeGeneral.elesource == 5 ) ? ExtraInputs & 0x40 : ~c & 0x80000000 ;	// ELE_DR   PC31
    break ;

		case HSW_Ele3pos1 :
			if ( g_eeGeneral.elesource == 5 )
			{
				xxx = ExtraInputs & 0x80 ;
			}
      else
			{
				xxx = c & 0x80000000 ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.elesource ) ;
			}
		break ;

		case HSW_Ele3pos2 :
			if ( g_eeGeneral.elesource == 5 )
			{
				xxx = ExtraInputs & 0x100 ;
			}
      else
			{
				xxx = readKeyUpgradeBit( g_eeGeneral.elesource ) ;
			}
    break ;

		case HSW_Rud3pos0 :
			if ( g_eeGeneral.rudsource == anaIndex )
			{
				xxx = ExtraInputs & 0x40 ;
			}
      else
			{
				xxx = ~a & 0x00008000 ;	// RUD_DR   PA15
			}
    break ;

		case HSW_Rud3pos1 :
			if ( g_eeGeneral.rudsource == anaIndex )
			{
				xxx = ExtraInputs & 0x80 ;
			}
      else
			{
				xxx = (a & 0x00008000) ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.rudsource ) ;
			}
//			xxx = (a & 0x00008000) ; if ( xxx ) xxx = (c & 0x80000000) ;
    break ;

		case HSW_Rud3pos2 :
			if ( g_eeGeneral.rudsource == anaIndex )
			{
				xxx = ExtraInputs & 0x100 ;
			}
      else
			{
				xxx = readKeyUpgradeBit( g_eeGeneral.rudsource ) ;
			}
//			xxx = ~c & 0x80000000 ;	// ELE_DR   PC31
    break ;

		case HSW_Ail3pos0 :
			if ( g_eeGeneral.ailsource == anaIndex )
			{
				xxx = ExtraInputs & 0x40 ;
			}
      else
			{
				xxx = ~a & 0x00000004 ;	// AIL-DR  PA2
			}
    break ;

		case HSW_Ail3pos1 :
//			xxx = (a & 0x00000004) ; if ( xxx ) xxx = (PIOB->PIO_PDSR & 0x00004000) ;
			if ( g_eeGeneral.ailsource == anaIndex )
			{
				xxx = ExtraInputs & 0x80 ;
			}
      else
			{
				xxx = (a & 0x00000004) ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.ailsource ) ;
			}
    break ;                               

		case HSW_Ail3pos2 :
//			xxx = ~PIOB->PIO_PDSR & 0x00004000 ;
			if ( g_eeGeneral.ailsource == anaIndex )
			{
				xxx = ExtraInputs & 0x100 ;
			}
      else
			{
				xxx = readKeyUpgradeBit( g_eeGeneral.ailsource ) ;
			}
    break ;

		case HSW_Gear3pos0 :
			if ( g_eeGeneral.geasource == anaIndex )
			{
				xxx = ExtraInputs & 0x40 ;
			}
      else
			{
				xxx = ~c & 0x00010000 ;	// SW_GEAR     PC16
			}
    break ;

		case HSW_Gear3pos1 :
//			xxx = (c & 0x00010000) ; if ( xxx ) xxx = (PIOB->PIO_PDSR & 0x00004000) ;
			if ( g_eeGeneral.geasource == anaIndex )
			{
				xxx = ExtraInputs & 0x80 ;
			}
      else
			{
				xxx = (c & 0x00010000) ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.geasource ) ;
			}
    break ;

		case HSW_Gear3pos2 :
			if ( g_eeGeneral.geasource == anaIndex )
			{
				xxx = ExtraInputs & 0x100 ;
			}
      else
			{
				xxx = readKeyUpgradeBit( g_eeGeneral.geasource ) ;
			}
//			xxx = ~PIOB->PIO_PDSR & 0x00004000 ;
    break ;

		case HSW_Ele6pos0 :
			if ( avpot != 0xFFFF )
			{
				xxx = avpot > SixPositionTable[0] ;
			}
			else if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
			{
				xxx = av9 > 3438 ;
			}
			else
			{
				xxx = av9 > 0x7E0 ;
			}
    break ;
			
		case HSW_Ele6pos1 :
			if ( avpot != 0xFFFF )
			{
				xxx = ( avpot <= SixPositionTable[0] ) && ( avpot >= SixPositionTable[1] ) ;
			}
			else if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
			{
				xxx = ( av9 <= 3438 ) && ( av9 >= 2525 ) ;
			}
			else
			{
				xxx = ( av9 <= 0x7E0 ) && ( av9 >= 0x7A8 ) ;
			}
    break ;
			
		case HSW_Ele6pos2 :
			if ( avpot != 0xFFFF )
			{
				xxx = ( avpot <= SixPositionTable[1] ) && ( avpot >= SixPositionTable[2] ) ;
			}
			else 			if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
			{
				xxx = ( av9 <= 2525 ) && ( av9 >= 2159 ) ;
			}
			else
			{
				xxx = ( av9 <= 0x7A8 ) && ( av9 >= 0x770 ) ;
			}
    break ;
			
		case HSW_Ele6pos3 :
			if ( avpot != 0xFFFF )
			{
				xxx = ( avpot <= SixPositionTable[2] ) && ( avpot >= SixPositionTable[3] ) ;
			}
			else 			if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
			{
				xxx = ( av9 <= 2159 ) && ( av9 >= 1974 ) ;
			}
			else
			{
				xxx = ( av9 <= 0x770 ) && ( av9 >= 0x340 ) ;
			}
    break ;
			
		case HSW_Ele6pos4 :
			if ( avpot != 0xFFFF )
			{
				xxx = ( avpot <= SixPositionTable[3] ) && ( avpot >= SixPositionTable[4] ) ;
			}
			else 			if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
			{
				xxx = ( av9 <= 1974 ) && ( av9 >= 950 ) ;
			}
			else
			{
				xxx = ( av9 <= 0x340 ) && ( av9 >= 0x070 ) ;
			}
    break ;
			
		case HSW_Ele6pos5 :
			if ( avpot != 0xFFFF )
			{
				xxx = avpot < SixPositionTable[4] ;
			}
			else 			if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
			{
				xxx = av9 < 950 ;
			}
			else
			{
				xxx = av9 < 0x070 ;
			}
    break ;

		case HSW_Pb1 :
			xxx = readKeyUpgradeBit( g_eeGeneral.pb1source ) ;
    break ;
			 
		case HSW_Pb2 :
			xxx = readKeyUpgradeBit( g_eeGeneral.pb2source ) ;
    break ;

		case HSW_Pb3 :
			xxx = readKeyUpgradeBit( g_eeGeneral.pb3source ) ;
    break ;
			 
		case HSW_Pb4 :
			xxx = readKeyUpgradeBit( g_eeGeneral.pb4source ) ;
    break ;

    default:
    break ;
		
  }

  if ( xxx )
  {
    return 1 ;
  }
  return 0;
	
}

// Returns 0, 1 or 2 (or 3,4,5) for ^ - or v (or 6pos)
uint32_t switchPosition( uint32_t swtch )
{
	if ( hwKeyState( swtch ) )
	{
		return 0 ;
	}
	if ( ( swtch >= HSW_ThrCt ) && ( swtch <= HSW_ElevDR ) )
	{
		return 1 ;	// 2-pos switch
	}
	if ( ( swtch >= HSW_AileDR ) && ( swtch <= HSW_Trainer ) )
	{
		return 1 ;	// 2-pos switch
	}
	swtch += 1 ;
	if ( hwKeyState( swtch ) )
	{
		return 1 ;			
	}
	if ( swtch == HSW_Ele6pos1 )
	{
		if ( hwKeyState( HSW_Ele6pos3 ) )
		{
			return 3 ;
		}
		if ( hwKeyState( HSW_Ele6pos4 ) )
		{
			return 4 ;
		}
		if ( hwKeyState( HSW_Ele6pos5 ) )
		{
			return 5 ;
		}
	}
	return 2 ;
}




uint32_t keyState(EnumKeys enuk)
{
	register uint32_t a ;
	register uint32_t c ;

  CPU_UINT xxx = 0 ;
  if(enuk < (int)DIM(keys))  return keys[enuk].state() ? 1 : 0 ;

	a = PIOA->PIO_PDSR ;
	c = PIOC->PIO_PDSR ;
	switch((uint8_t)enuk)
	{
#ifdef REVB
    case SW_ElevDR : xxx = c & 0x80000000 ;	// ELE_DR   PC31
#else 
    case SW_ElevDR : xxx = a & 0x00000100 ;	// ELE_DR   PA8
#endif 
    break ;
    
    case SW_AileDR : xxx = a & 0x00000004 ;	// AIL-DR  PA2
    break ;

    case SW_RuddDR : xxx = a & 0x00008000 ;	// RUN_DR   PA15
    break ;
      //     INP_G_ID1 INP_E_ID2
      // id0    0        1
      // id1    1        1
      // id2    1        0
    case SW_ID0    : xxx = ~c & 0x00004000 ;	// SW_IDL1     PC14
    break ;
    case SW_ID1    : xxx = (c & 0x00004000) ; if ( xxx ) xxx = (PIOC->PIO_PDSR & 0x00000800);
    break ;
    case SW_ID2    : xxx = ~c & 0x00000800 ;	// SW_IDL2     PC11
    break ;

    
		case SW_Gear   : xxx = c & 0x00010000 ;	// SW_GEAR     PC16
    break ;

#ifdef REVB
    case SW_ThrCt  : xxx = c & 0x00100000 ;	// SW_TCUT     PC20
#else 
    case SW_ThrCt  : xxx = a & 0x10000000 ;	// SW_TCUT     PA28
#endif 
    break ;

    case SW_Trainer: xxx = c & 0x00000100 ;	// SW-TRAIN    PC8
    break ;
    default:;
  }

  if ( xxx )
  {
    return 1 ;
  }
  return 0;
}
#endif // PCBSKY


#ifdef PCBX9D
void init_keys()
{
#ifdef REV9E
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN ; 		// Enable portF clock
	configure_pins( 0x008C, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
	configure_pins( 0x0001, PIN_INPUT | PIN_PULLUP | PIN_PORTF ) ;
#else
// Buttons PE10, 11, 12, PD2, 3, 7
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
	configure_pins( 0x1C00, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
	configure_pins( 0x008C, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
#endif // REV9E
}

void init_trims()
{
// Trims 
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
#ifdef REV9E
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN ; 		// Enable portG clock
	configure_pins( 0x0018, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
	configure_pins( 0x0003, PIN_INPUT | PIN_PULLUP | PIN_PORTG ) ;
#else
	configure_pins( 0x0078, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
#endif // REV9E
	configure_pins( 0x200E, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
}

// Reqd. bit 6 LEFT, 5 RIGHT, 4 UP, 3 DOWN 2 EXIT 1 MENU
uint32_t read_keys()
{
	register uint32_t x ;
	register uint32_t y ;

	x = GPIOD->IDR ; // 7 MENU, 3 PAGE(UP), 2 EXIT
	y = 0 ;

#ifdef REV9E
	if ( x & PIN_BUTTON_MENU )
	{
		y |= 0x02 << KEY_MENU ;			// MENU
	}
	if ( x & PIN_BUTTON_PAGE )
	{
		y |= 0x02 << KEY_LEFT ;		// LEFT
	}
	if ( x & PIN_BUTTON_EXIT )
	{
		y |= 0x02 << KEY_EXIT ;			// EXIT
	}
//	x = GPIOF->IDR ; // 10 RIGHT(+), 11 LEFT(-), 12 ENT(DOWN)
//	if ( x & PIN_BUTTON_ENCODER )
//	{
//		y |= 0x02 << KEY_RIGHT ;		// RIGHT
//	}

	y |= 0x02 << KEY_RIGHT ;		// RIGHT
	y |= 0x02 << KEY_UP ;			// up
	y |= 0x02 << KEY_DOWN ;		// DOWN

#else
	
	y = 0 ;
	if ( x & PIN_BUTTON_MENU )
	{
		y |= 0x02 << KEY_UP ;			// up
//		y |= 0x02 << KEY_MENU ;			// MENU
	}
	if ( x & PIN_BUTTON_PAGE )
	{
		y |= 0x02 << KEY_LEFT ;		// LEFT
	}
	if ( x & PIN_BUTTON_EXIT )
	{
		y |= 0x02 << KEY_DOWN ;		// DOWN
//		y |= 0x02 << KEY_EXIT ;			// EXIT
	}
	
	x = GPIOE->IDR ; // 10 RIGHT(+), 11 LEFT(-), 12 ENT(DOWN)
	if ( x & PIN_BUTTON_PLUS )
	{
		y |= 0x02 << KEY_MENU ;			// MENU
//		y |= 0x02 << KEY_UP ;			// up
	}
	if ( x & PIN_BUTTON_MINUS )
	{
		y |= 0x02 << KEY_RIGHT ;		// RIGHT
//		y |= 0x02 << KEY_DOWN ;		// DOWN
	}
	if ( x & PIN_BUTTON_ENTER )
	{
		y |= 0x02 << KEY_EXIT ;			// EXIT
//		y |= 0x02 << KEY_RIGHT ;		// RIGHT
	}
#endif // REV9E
	return y ;
}

uint32_t read_trims()
{
	uint32_t trims ;
	uint32_t trima ;

	trims = 0 ;

	trima = GPIOE->IDR ;

// TRIM_LH_DOWN
	if ( ( trima & PIN_TRIMLH_DN ) == 0 )
	{
		trims |= 1 ;
	}
    
// TRIM_LH_UP
	if ( ( trima & PIN_TRIMLH_UP ) == 0 )
	{
		trims |= 2 ;
	}

#ifndef REV9E
// TRIM_LV_DOWN
	if ( ( trima & PIN_TRIMLV_DN ) == 0 )
	{
		trims |= 4 ;
	}

// TRIM_LV_UP
	if ( ( trima & PIN_TRIMLV_UP ) == 0 )
	{
		trims |= 8 ;
	}
#endif

	trima = GPIOC->IDR ;

// TRIM_RV_UP
	if ( ( trima & PIN_TRIMRV_UP ) == 0 )
	{
		trims |= 0x20 ;
	}

// TRIM_RH_DOWN
	if ( ( trima & PIN_TRIMRH_DN ) == 0 )
	{
		trims |= 0x40 ;
	}


// TRIM_RV_DOWN
	if ( ( trima & PIN_TRIMRV_DN ) == 0 )
	{
		trims |= 0x10 ;
	}

// TRIM_RH_UP
	if ( ( trima & PIN_TRIMRH_UP ) == 0 )
	{
		trims |= 0x80 ;
	}

#ifdef REV9E
	trima = GPIOG->IDR ;
// TRIM_LV_DOWN
	if ( ( trima & PIN_TRIMLV_DN ) == 0 )
	{
		trims |= 4 ;
	}

// TRIM_LV_UP
	if ( ( trima & PIN_TRIMLV_UP ) == 0 )
	{
		trims |= 8 ;
	}
#endif	// REV9E

	return trims ;
}

#ifdef PCBX9D

// 
void setup_switches()
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
#ifdef REVPLUS
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
#endif
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
	configure_pins( 0x0020, PIN_INPUT | PIN_PULLUP | PIN_PORTA ) ;
#ifdef REVPLUS
	configure_pins( 0x0038, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
#else
	configure_pins( 0x003A, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
#endif
#ifdef REV9E
	configure_pins( 0xE387 | PIN_SW_L_L | PIN_SW_Q_L | PIN_SW_Q_H,
									PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
#else
	configure_pins( 0xE387, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
#endif
#ifdef REVPLUS
#ifdef REV9E
	configure_pins( PIN_SW_H_L, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
#else	
	configure_pins( PIN_SW_H, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
#endif
#endif

#ifdef REV9E
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN ; 		// Enable port F clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN ; 		// Enable port Gclock
	configure_pins( PIN_SW_I_L | PIN_SW_I_H | PIN_SW_J_L | PIN_SW_J_H | PIN_SW_K_L | PIN_SW_K_H 
									| PIN_SW_L_H | PIN_SW_M_L | PIN_SW_M_H | PIN_SW_N_L | PIN_SW_N_H,
									PIN_INPUT | PIN_PULLUP | PIN_PORTF ) ;
	configure_pins( PIN_SW_O_L | PIN_SW_O_H | PIN_SW_P_L | PIN_SW_P_H | PIN_SW_R_L | PIN_SW_R_H,
									PIN_INPUT | PIN_PULLUP | PIN_PORTG) ;
#endif	// REV9E
}

uint32_t hwKeyState( uint8_t key )
{
  register uint32_t a = GPIOA->IDR;
  register uint32_t b = GPIOB->IDR;
  register uint32_t e = GPIOE->IDR;
#ifdef REV9E
  register uint32_t f = GPIOF->IDR;
  register uint32_t g = GPIOG->IDR;
#endif	// REV9E

  uint32_t xxx = 0 ;
  uint32_t analog = 0 ;
	
	if ( g_eeGeneral.analogMapping & MASK_6POS )
	{
  	analog = ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2 ;
		analog += 3 ;
		if ( analog > 5 )
		{
			analog = 9 ;
		}
		analog = Analog_values[analog] ;
	}
  
	if( key > HSW_MAX )  return 0 ;

  switch ( key )
	{
#ifndef REV9E
    case HSW_SA0:
      xxx = ~e & PIN_SW_A_L;
      break;
    case HSW_SA1:
      xxx = ((e & PIN_SW_A_L) | (b & PIN_SW_A_H)) == (PIN_SW_A_L | PIN_SW_A_H) ;
      break;
    case HSW_SA2:
      xxx = ~b & PIN_SW_A_H;
      break;

    case HSW_SB0:
      xxx = ~e & PIN_SW_B_L ;
      break;
    case HSW_SB1:
      xxx = (e & (PIN_SW_B_L | PIN_SW_B_H)) == (PIN_SW_B_L | PIN_SW_B_H) ;
      break;
    case HSW_SB2:
      xxx = ~e & PIN_SW_B_H ;
      break;

    case HSW_SC0:
      xxx = ~a & PIN_SW_C_L ;
      break;
    case HSW_SC1:
      xxx = ((a & PIN_SW_C_L) | (e & PIN_SW_C_H)) == (PIN_SW_C_L | PIN_SW_C_H) ;
      break;
    case HSW_SC2:
      xxx = ~e & PIN_SW_C_H ;
      break;

    case HSW_SD0:
#ifdef REVPLUS
      xxx = ~e & PIN_SW_D_L ;
#else
      xxx = ~b & PIN_SW_D_L ;
#endif
			break;
    case HSW_SD1:
#ifdef REVPLUS
      xxx = ((e & PIN_SW_D_L) | (e & PIN_SW_D_H)) == (PIN_SW_D_L | PIN_SW_D_H) ;
#else
      xxx = ((b & PIN_SW_D_L) | (e & PIN_SW_D_H)) == (PIN_SW_D_L | PIN_SW_D_H) ;
#endif
      break;
    case HSW_SD2:
      xxx = ~e & PIN_SW_D_H ;
      break;

    case HSW_SE0:
      xxx = ~b & PIN_SW_E_L ;
      break;
    case HSW_SE1:
      xxx = ((b & PIN_SW_E_H) | (b & PIN_SW_E_L)) == (PIN_SW_E_H | PIN_SW_E_L) ;
      break;
    case HSW_SE2:
      xxx = ~b & PIN_SW_E_H ;
      break;

//    case HSW_SF0:
//      xxx = e & PIN_SW_F ;
//      break;
    case HSW_SF2:
      xxx = ~e & PIN_SW_F ;
      break;

    case HSW_SG0:
      xxx = ~e & PIN_SW_G_L ;
      break;
    case HSW_SG1:
      xxx = (e & (PIN_SW_G_H | PIN_SW_G_L)) == (PIN_SW_G_H | PIN_SW_G_L) ;
      break;
    case HSW_SG2:
      xxx = ~e & PIN_SW_G_H ;
      break;

//    case HSW_SH0:
//#ifdef REVPLUS
//      xxx = GPIOD->IDR & PIN_SW_H;
//#else
//      xxx = e & PIN_SW_H;
//#endif
//      break;
    case HSW_SH2:
#ifdef REVPLUS
      xxx = ~GPIOD->IDR & PIN_SW_H;
#else
      xxx = ~e & PIN_SW_H;
#endif
      break;

#endif // nREV9E

#ifdef REV9E
    
    case HSW_SA0:
      xxx = ~e & PIN_SW_A_L;
      break;
    case HSW_SA1:
      xxx = ((e & PIN_SW_A_L) | (e & PIN_SW_A_H)) == (PIN_SW_A_L | PIN_SW_A_H) ;
      break;
    case HSW_SA2:
      xxx = ~e & PIN_SW_A_H;
      break;

    case HSW_SB0:
      xxx = ~GPIOD->IDR & PIN_SW_B_L ;
      break;
    case HSW_SB1:
      xxx = (GPIOD->IDR & (PIN_SW_B_L | PIN_SW_B_H)) == (PIN_SW_B_L | PIN_SW_B_H) ;
      break;
    case HSW_SB2:
      xxx = ~GPIOD->IDR & PIN_SW_B_H ;
      break;

    case HSW_SC0:
      xxx = ~g & PIN_SW_C_L ;
      break;
    case HSW_SC1:
      xxx = ((g & PIN_SW_C_L) | (g & PIN_SW_C_H)) == (PIN_SW_C_L | PIN_SW_C_H) ;
      break;
    case HSW_SC2:
      xxx = ~g & PIN_SW_C_H ;
      break;

//    case HSW_SD0:
//      xxx = ~e & PIN_SW_D_L ;
//			break;
//		case HSW_SD1:
//      xxx = ((e & PIN_SW_D_L) | (e & PIN_SW_D_H)) == (PIN_SW_D_L | PIN_SW_D_H) ;
//      break;
    case HSW_SD2:
      xxx = ~e & PIN_SW_D_L ;
      break;
    
    case HSW_SE0:
      xxx = ~f & PIN_SW_E_L ;
      break;
    case HSW_SE1:
      xxx = ((f & PIN_SW_E_H) | (f & PIN_SW_E_L)) == (PIN_SW_E_H | PIN_SW_E_L) ;
      break;
    case HSW_SE2:
      xxx = ~f & PIN_SW_E_H ;
      break;

		case HSW_SF0:
      xxx = ~e & PIN_SW_F_L ;
      break;
    case HSW_SF1:
      xxx = ((e & PIN_SW_F_H) | (e & PIN_SW_F_L)) == (PIN_SW_F_H | PIN_SW_F_L) ;
      break;
//    case HSW_SF2:
//      xxx = ~e & PIN_SW_F_H ;
//      break;

    case HSW_SF2:
      xxx = ~e & PIN_SW_F_H ;
      break;

    case HSW_SG0:
      xxx = ~f & PIN_SW_G_L ;
      break;
    case HSW_SG1:
      xxx = (f & (PIN_SW_G_H | PIN_SW_G_L)) == (PIN_SW_G_H | PIN_SW_G_L) ;
      break;
    case HSW_SG2:
      xxx = ~f & PIN_SW_G_H ;
      break;

    case HSW_SH2:
      xxx = ~f & PIN_SW_H_L;
      break;

		case HSW_SI0:
      xxx = ~e & PIN_SW_I_L ;
      break;
    case HSW_SI1:
      xxx = ((f & PIN_SW_I_H) | (e & PIN_SW_I_L)) == (PIN_SW_I_H | PIN_SW_I_L) ;
      break;
    case HSW_SI2:
      xxx = ~f & PIN_SW_I_H ;
      break;

    case HSW_SJ0:
      xxx = ~g & PIN_SW_J_L ;
      break;
    case HSW_SJ1:
      xxx = (g & (PIN_SW_J_H | PIN_SW_J_L)) == (PIN_SW_J_H | PIN_SW_J_L) ;
      break;
    case HSW_SJ2:
      xxx = ~g & PIN_SW_J_H ;
      break;
    
		case HSW_SK0:
      xxx = ~f & PIN_SW_K_L ;
      break;
    case HSW_SK1:
      xxx = (f & (PIN_SW_K_H | PIN_SW_K_L)) == (PIN_SW_K_H | PIN_SW_K_L) ;
      break;
    case HSW_SK2:
      xxx = ~f & PIN_SW_K_H ;
      break;

    case HSW_SL0:
      xxx = ~e & PIN_SW_L_L ;
      break;
    case HSW_SL1:
      xxx = ( (e & PIN_SW_L_H ) | (e & PIN_SW_L_L) ) == (PIN_SW_L_H | PIN_SW_L_L) ;
      break;
    case HSW_SL2:
      xxx = ~e & PIN_SW_L_H ;
      break;

		case HSW_SM0:
      xxx = ~a & PIN_SW_M_L ;
      break;
    case HSW_SM1:
      xxx = ((e & PIN_SW_M_H) | (a & PIN_SW_M_L)) == (PIN_SW_M_H | PIN_SW_M_L) ;
      break;
    case HSW_SM2:
      xxx = ~e & PIN_SW_M_H ;
      break;

		case HSW_SN0:
      xxx = ~b & PIN_SW_N_L ;
      break;
    case HSW_SN1:
      xxx = (b & (PIN_SW_N_H | PIN_SW_N_L)) == (PIN_SW_N_H | PIN_SW_N_L) ;
      break;
    case HSW_SN2:
      xxx = ~b & PIN_SW_N_H ;
      break;

		case HSW_SO0:
      xxx = ~e & PIN_SW_O_L ;
      break;
    case HSW_SO1:
      xxx = ((f & PIN_SW_O_H) | (e & PIN_SW_O_L)) == (PIN_SW_O_H | PIN_SW_O_L) ;
      break;
    case HSW_SO2:
      xxx = ~f & PIN_SW_O_H ;
      break;

		case HSW_SP0:
      xxx = ~f & PIN_SW_P_L ;
      break;
    case HSW_SP1:
      xxx = (f & (PIN_SW_P_H | PIN_SW_P_L)) == (PIN_SW_P_H | PIN_SW_P_L) ;
      break;
    case HSW_SP2:
      xxx = ~f & PIN_SW_P_H ;
      break;
    
		case HSW_SQ0:
      xxx = ~f & PIN_SW_Q_L ;
      break;
    case HSW_SQ1:
      xxx = (f & (PIN_SW_Q_H | PIN_SW_Q_L)) == (PIN_SW_Q_H | PIN_SW_Q_L) ;
      break;
    case HSW_SQ2:
      xxx = ~f & PIN_SW_Q_H ;
      break;

		case HSW_SR0:
      xxx = ~e & PIN_SW_R_L ;
      break;
    case HSW_SR1:
      xxx = ((b & PIN_SW_R_H) | (e & PIN_SW_R_L)) == (PIN_SW_R_H | PIN_SW_R_L) ;
      break;
    case HSW_SR2:
      xxx = ~b & PIN_SW_R_H ;
      break;

#endif	// REV9E

		case HSW_Ele6pos0 :
				xxx = analog < 417 ;
    break ;
			
		case HSW_Ele6pos1 :
				xxx = ( analog <= 938 ) && ( analog >= 417 ) ;
    break ;
			
		case HSW_Ele6pos2 :
				xxx = ( analog <= 1211 ) && ( analog >= 938 ) ;
    break ;
			
		case HSW_Ele6pos3 :
				xxx = ( analog <= 1721 ) && ( analog >= 1211 ) ;
    break ;
			
		case HSW_Ele6pos4 :
				xxx = ( analog <= 3050 ) && (+ analog >= 1721 ) ;
    break ;
			
		case HSW_Ele6pos5 :
				xxx = analog > 3050 ;
    break ;

    default:
      break;
  }


  if ( xxx )
  {
    return 1 ;
  }
  return 0;

}

uint32_t keyState(EnumKeys enuk)
{
  register uint32_t a = GPIOA->IDR;
//  register uint32_t b = GPIOB->IDR;
  register uint32_t e = GPIOE->IDR;

  register uint32_t xxx = 0;

  if (enuk < (int) DIM(keys)) return keys[enuk].state() ? 1 : 0;

  switch ((uint8_t) enuk) {
//    case SW_SA0:
//      xxx = ~e & PIN_SW_A_L;
//      break;
//    case SW_SA1:
//      xxx = ((e & PIN_SW_A_L) | (b & PIN_SW_A_H)) == (PIN_SW_A_L | PIN_SW_A_H) ;
//      break;
//    case SW_SA2:
//      xxx = ~b & PIN_SW_A_H;
//      break;

//    case SW_SB0:
//      xxx = ~e & PIN_SW_B_L ;
//      break;
//    case SW_SB1:
//      xxx = (e & (PIN_SW_B_L | PIN_SW_B_H)) == (PIN_SW_B_L | PIN_SW_B_H) ;
//      break;
//    case SW_SB2:
//      xxx = ~e & PIN_SW_B_H ;
//      break;

    case SW_SC0:
      xxx = ~a & PIN_SW_C_L ;
      break;
    case SW_SC1:
      xxx = ((a & PIN_SW_C_L) | (e & PIN_SW_C_H)) == (PIN_SW_C_L | PIN_SW_C_H) ;
      break;
    case SW_SC2:
      xxx = ~e & PIN_SW_C_H ;
      break;

//    case SW_SD0:
//#ifdef REVPLUS
//      xxx = ~e & PIN_SW_D_L ;
//#else
//      xxx = ~b & PIN_SW_D_L ;
//#endif
//			break;
//    case SW_SD1:
//#ifdef REVPLUS
//      xxx = ((e & PIN_SW_D_L) | (e & PIN_SW_D_H)) == (PIN_SW_D_L | PIN_SW_D_H) ;
//#else
//      xxx = ((b & PIN_SW_D_L) | (e & PIN_SW_D_H)) == (PIN_SW_D_L | PIN_SW_D_H) ;
//#endif
//      break;
//    case SW_SD2:
//      xxx = ~e & PIN_SW_D_H ;
//      break;

//    case SW_SE0:
//      xxx = ~b & PIN_SW_E_L ;
//      break;
//    case SW_SE1:
//      xxx = ((b & PIN_SW_E_H) | (b & PIN_SW_E_L)) == (PIN_SW_E_H | PIN_SW_E_L) ;
//      break;
//    case SW_SE2:
//      xxx = ~b & PIN_SW_E_H ;
//      break;

//    case SW_SF0:
//      xxx = e & PIN_SW_F ;
//      break;
    case SW_SF2:
#ifdef REV9E
      xxx = ~e & PIN_SW_D_L ;
#else
      xxx = ~e & PIN_SW_F ;
#endif
      break;

//    case SW_SG0:
//      xxx = ~e & PIN_SW_G_L ;
//      break;
//    case SW_SG1:
//      xxx = (e & (PIN_SW_G_H | PIN_SW_G_L)) == (PIN_SW_G_H | PIN_SW_G_L) ;
//      break;
//    case SW_SG2:
//      xxx = ~e & PIN_SW_G_H ;
//      break;

//    case SW_SH0:
//#ifdef REVPLUS
//      xxx = GPIOD->IDR & PIN_SW_H;
//#else
//      xxx = e & PIN_SW_H;
//#endif
//      break;
    case SW_SH2:
#ifdef REVPLUS
#ifdef REV9E
      xxx = ~GPIOF->IDR & PIN_SW_H_L ;
#else
      xxx = ~GPIOD->IDR & PIN_SW_H;
#endif
#else
      xxx = ~e & PIN_SW_H;
#endif
      break;

    default:
      break;
  }

  if (xxx) {
    return 1;
  }

  return 0;
}

// Returns 0, 1 or 2 for ^ - or v
#ifdef REV9E
static const uint8_t SwitchIndices[] = {HSW_SA0,HSW_SB0,HSW_SC0,HSW_SD2,HSW_SE0,HSW_SF0,HSW_SG0,HSW_SH2,HSW_SI0,
																 HSW_SJ0, HSW_SK0, HSW_SL0, HSW_SM0, HSW_SN0, HSW_SO0, HSW_SP0, HSW_SQ0, HSW_SR0 } ;
#else
static const uint8_t SwitchIndices[] = {HSW_SA0,HSW_SB0,HSW_SC0,HSW_SD0,HSW_SE0,HSW_SF2,HSW_SG0,HSW_SH2} ;
#endif	// REV9E
uint32_t switchPosition( uint32_t swtch )
{
	if ( swtch < sizeof(SwitchIndices) )
	{
		swtch = SwitchIndices[swtch] ;
	}

	if ( swtch == HSW_SF2 )
	{
		if ( hwKeyState( swtch ) )
		{
			return 2 ;
		}
		return 0 ;
	} 
	if ( swtch == HSW_SH2 )
	{
		if ( hwKeyState( swtch ) )
		{
			return 2 ;
		}
		return 0 ;
	} 
	if ( hwKeyState( swtch ) )
	{
		return 0 ;
	}
	swtch += 1 ;
	if ( hwKeyState( swtch ) )
	{
		return 1 ;			
	}
	if ( swtch == HSW_Ele6pos1 )
	{
		if ( hwKeyState( HSW_Ele6pos3 ) )
		{
			return 3 ;
		}
		if ( hwKeyState( HSW_Ele6pos4 ) )
		{
			return 4 ;
		}
		if ( hwKeyState( HSW_Ele6pos5 ) )
		{
			return 5 ;
		}
	}
	return 2 ;
	
//	swtch *= 3 ;
//	swtch += SW_SA0 ;
//	if ( swtch > SW_SF0 )
//	{
//		swtch -= 1 ;		
//	}
//	if ( keyState( (EnumKeys)swtch ) )
//	{
//		return 0 ;
//	}
//	swtch += 1 ;
//	if ( keyState( (EnumKeys)swtch ) )
//	{
//		if ( ( swtch != SW_SF2 ) && ( swtch != SW_SH2 ) )
//		{
//			return 1 ;			
//		}
//	}
//	return 2 ;
}



#endif
#endif

