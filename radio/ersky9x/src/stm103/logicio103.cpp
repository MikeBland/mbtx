/****************************************************************************
*  Copyright (c) 2019 by Michael Blandford. All rights reserved.
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
#include "ersky9x.h"
#include "stm103/hal.h"
#include "logicio103.h"
#include "drivers.h"

extern uint16_t PortE0 ;
extern uint16_t PortE1 ;
extern uint16_t PortB0 ;
extern uint16_t PortB1 ;
extern uint16_t PortD0 ;
extern uint16_t PortD1 ;
extern uint16_t PortC0 ;
extern uint16_t PortC1 ;

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
			uint32_t mask = PIN_MODE_MASK << ( (pos&7) * 4) ;
			uint32_t value = (config & PIN_MODE_MASK) << ( (pos&7) * 4) ;

			if ( pos > 7 )
			{
	      pgpio->CRH &= ~mask ;
	      pgpio->CRH |= value ;
			}
			else
			{
	      pgpio->CRL  &= ~mask ;
	      pgpio->CRL |= value ;
			}
			if (config & PIN_PULLUP)
			{
				pgpio->BSRR = thispin ;
			}
			else if ( config & PIN_HIGH )
			{
				pgpio->BSRR = thispin ;		
			}
			else
			{
				pgpio->BRR = thispin ;		
			}
    }
  }
}

void init_trims()
{
// trim switches
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN ; 			// Enable portB clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPDEN ; 			// Enable portD clock

	configure_pins( SW_DET1_PIN, SW_DET1_PORT | PIN_OP2 | PIN_GP_OD | PIN_LOW ) ;
	configure_pins( SW_DET3_PIN, SW_DET3_PORT | PIN_OP2 | PIN_GP_OD | PIN_HIGH ) ;

	configure_pins( SW9_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW9_PORT ) ;
	configure_pins( SW10_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW10_PORT ) ;
	configure_pins( SW11_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW11_PORT ) ;
	configure_pins( SW12_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW12_PORT ) ;
}

void setup_switches()
{
	
}

extern uint8_t ExitTimer ;
uint32_t read_keys()
{
	uint32_t value ;
	value = PortC1 & 0x80 ? 2 : 0 ;	// Bind, treat as MENU
	if ( ExitTimer == 0 )
	{
		value |= 4 ;
	}
	return value | 0x78 ;
}

void init_keys()
{
	
}

uint32_t read_trims()
{
	uint32_t trims ;

	trims = 0 ;
// TRIM_LV_DOWN
	if ( ( PortB1 & PIN_TRIMLV_DN ) == 0 )
	{
		trims |= 4 ;
	}

// TRIM_LV_UP
	if ( ( PortB0 & PIN_TRIMLV_UP ) == 0 )
	{
		trims |= 8 ;
	}

// TRIM_RH_DOWN
	if ( ( PortD1 & PIN_TRIMRH_DN ) == 0 )
	{
		trims |= 0x40 ;
	}

// TRIM_RH_UP
	if ( ( PortD0 & PIN_TRIMRH_UP ) == 0 )
	{
		trims |= 0x80 ;
	}

// TRIM_LH_UP
	if ( ( PortD1 & PIN_TRIMLH_UP ) == 0 )
	{
		trims |= 2 ;
	}

// TRIM_RV_UP
	if ( ( PortD0 & PIN_TRIMRV_UP ) == 0 )
	{
		trims |= 0x20 ;
	}


// TRIM_RV_DOWN
	if ( ( PortD1 & PIN_TRIMRV_DN ) == 0 )
	{
		trims |= 0x10 ;
	}

// TRIM_LH_DOWN
	if ( ( PortD0 & PIN_TRIMLH_DN ) == 0 )
	{
		trims |= 1 ;
	}
	return trims ;
}

uint32_t hwKeyState( uint8_t key )
{
  uint32_t xxx = 0 ;
	
	if( key > HSW_MAX )  return 0 ;

//	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )
//	{
//		uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
//		key -= HSW_Etrmdn ;		// 0 - 7
//		if (key >= 2 && key <= 5)       // swap back LH/RH trims
//		{
//			if ( g_eeGeneral.stickMode & 2 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		else
//		{
//			if ( g_eeGeneral.stickMode & 1 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		if ( ct )
//		{
//			key ^= 0x06 ;
//	 		if ( ct == 2 ) // Vintage style crosstrim
//			{
//	 			if (key >= 2 && key <= 5)       // swap back LH/RH trims
//				{
//					key ^= 0x06 ;
//				}
//			}
//		}
//		key += HSW_Etrmdn ;		// 0 - 7
//	}

  switch ( key )
	{
    case HSW_SA0:
      xxx = ~PortD0 & PIN_SW_A_L ;
    break ;
    case HSW_SA1:
      xxx = (PortD0 & PIN_SW_A_L) && (PortD1 & PIN_SW_A_H) ;
    break ;
    case HSW_SA2:
      xxx = ~PortD1 & PIN_SW_A_H ;
    break ;

    case HSW_SB0:
      xxx = ~PortE0 & PIN_SW_B_L ;
    break ;
    case HSW_SB1:
      xxx = (PortE0 & PIN_SW_B_L) && (PortE1 & PIN_SW_B_H) ;
    break;
    case HSW_SB2:
      xxx = ~PortE1 & PIN_SW_B_H ;
    break ;

    case HSW_SC0:
      xxx = ~PortE0 & PIN_SW_C_L ;
    break ;
    case HSW_SC1:
      xxx = (PortE0 & PIN_SW_C_L) && (PortE1 & PIN_SW_C_H) ;
    break;
    case HSW_SC2:
      xxx = ~PortE1 & PIN_SW_C_H ;
    break ;
    
		case HSW_SD0:
      xxx = ~PortE0 & PIN_SW_D_L ;
    break ;
    case HSW_SD1:
      xxx = (PortE0 & PIN_SW_D_L) && (PortE1 & PIN_SW_D_H) ;
    break;
    case HSW_SD2:
      xxx = ~PortE1 & PIN_SW_D_H ;
    break ;

		case HSW_SG2:
      xxx = PortE0 & PIN_SW_G ;
    break ;

    case HSW_SH2:
      xxx = ~PortE0 & PIN_SW_H ;
    break ;
	 
//		case HSW_SF2:
//      xxx = ~c & PIN_SW_F ;
//    break ;

//    case HSW_SH2:
//      xxx = ~a & PIN_SW_H ;
//    break ;

//		case HSW_Ttrmup :
//			xxx = keyState( (EnumKeys) TRM_RV_DWN ) ;
//    break ;
	
//		case HSW_Ttrmdn :
//			xxx = keyState( (EnumKeys) TRM_RV_UP ) ;
//    break ;
	
//		case HSW_Rtrmup :
//			xxx = keyState( (EnumKeys) TRM_LH_DWN ) ;
//    break ;
	
//		case HSW_Rtrmdn :
//			xxx = keyState( (EnumKeys) TRM_LH_UP ) ;
//    break ;
	
//		case HSW_Atrmup :
//			xxx = keyState( (EnumKeys) TRM_RH_DWN ) ;
//    break ;
	
//		case HSW_Atrmdn :
//			xxx = keyState( (EnumKeys) TRM_RH_UP ) ;
//    break ;
	
//		case HSW_Etrmup :
//			xxx = keyState( (EnumKeys) TRM_LV_DWN ) ;
//    break ;
	
//		case HSW_Etrmdn :
//			xxx = keyState( (EnumKeys) TRM_LV_UP ) ;
//    break ;
	
//		case HSW_Pb1 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb1source ) ;
//    break ;
		 
//		case HSW_Pb2 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb2source ) ;
//    break ;
	
//		case HSW_Pb3 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb3source ) ;
//    break ;

	}
	return xxx ;
}

uint32_t keyState(EnumKeys enuk)
{

  if (enuk < (int) DIM(keys)) return keys[enuk].state() ? 1 : 0;

	return hwKeyState( (uint8_t) enuk -SW_BASE + 1 ) ;
}


//#ifdef PCBSKY
//#include "AT91SAM3S4.h"
//#endif

//#if defined(PCBX9D) || defined(PCB9XT)
//#include "X9D/stm32f2xx.h"
//#include "X9D/stm32f2xx_gpio.h"
//#include "X9D/hal.h"
//#endif

//#if defined(PCBX12D) || defined(PCBX10)
//#include "X12D/stm32f4xx.h"
//#include "X12D/stm32f4xx_gpio.h"
//#include "X12D/hal.h"
//#endif

//#if defined(PCBX12D) || defined(PCBX10)
//#ifndef SIMU
//#include "X12D/core_cm4.h"
//#endif
//#else
//#ifndef SIMU
//#include "core_cm3.h"
//#endif
//#endif

//#include "ersky9x.h"
//#include "myeeprom.h"
//#include "drivers.h"
//#include "logicio.h"

//#ifdef PCBSKY
//#include "lcd.h"
//#endif

//#ifdef PCB9XT
//#include "analog.h"
//#include "mega64.h"
//#endif

//#include "myeeprom.h"

////extern uint8_t TrimBits ;
//extern uint16_t ExternalSwitches ;
//extern uint8_t ExternalSwitchesValid ;

//#ifndef SIMU
//#ifdef PCBSKY
//void configure_pins( uint32_t pins, uint16_t config )
//{
//	register Pio *pioptr ;
	
//	pioptr = PIOA + ( ( config & PIN_PORT_MASK ) >> 6) ;
//	if ( config & PIN_PULLUP )
//	{
//		pioptr->PIO_PPDDR = pins ;
//		pioptr->PIO_PUER = pins ;
//	}
//	else
//	{
//		pioptr->PIO_PUDR = pins ;
//	}

//	if ( config & PIN_PULLDOWN )
//	{
//		pioptr->PIO_PUDR = pins ;
//		pioptr->PIO_PPDER = pins ;
//	}
//	else
//	{
//		pioptr->PIO_PPDDR = pins ;
//	}

//	if ( config & PIN_HIGH )
//	{
//		pioptr->PIO_SODR = pins ;		
//	}
//	else
//	{
//		pioptr->PIO_CODR = pins ;		
//	}
	 
//	if ( config & PIN_INPUT )
//	{
//		pioptr->PIO_ODR = pins ;
//	}
//	else
//	{
//		pioptr->PIO_OER = pins ;
//	}

//	if ( config & PIN_PERI_MASK_L )
//	{
//		pioptr->PIO_ABCDSR[0] |= pins ;
//	}
//	else
//	{
//		pioptr->PIO_ABCDSR[0] &= ~pins ;
//	}
//	if ( config & PIN_PERI_MASK_H )
//	{
//		pioptr->PIO_ABCDSR[1] |= pins ;
//	}
//	else
//	{
//		pioptr->PIO_ABCDSR[1] &= ~pins ;
//	}

//	if ( config & PIN_ENABLE )
//	{
//		pioptr->PIO_PER = pins ;		
//	}
//	else
//	{
//		pioptr->PIO_PDR = pins ;		
//	}

//	if ( config & PIN_ODRAIN )
//	{
//		pioptr->PIO_MDER = pins ;		
//	}
//	else
//	{
//		pioptr->PIO_MDDR = pins ;		
//	}
//}
//#endif


//#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
//void configure_pins( uint32_t pins, uint16_t config )
//{
//	uint32_t address ;
//	GPIO_TypeDef *pgpio ;
//	uint32_t thispin ;
//  uint32_t pos ;

//	address = ( config & PIN_PORT_MASK ) >> 8 ;
//	address *= (GPIOB_BASE-GPIOA_BASE) ;
//	address += GPIOA_BASE ;
//	pgpio = (GPIO_TypeDef* ) address ;
	
//  /* -------------------------Configure the port pins---------------- */
//  /*-- GPIO Mode Configuration --*/
//  for (thispin = 0x0001, pos = 0; thispin < 0x10000; thispin <<= 1, pos +=1 )
//  {
//    if ( pins & thispin)
//    {
//      pgpio->MODER  &= ~(GPIO_MODER_MODER0 << (pos * 2)) ;
//      pgpio->MODER |= (config & PIN_MODE_MASK) << (pos * 2) ;

//      if ( ( (config & PIN_MODE_MASK ) == PIN_OUTPUT) || ( (config & PIN_MODE_MASK) == PIN_PERIPHERAL) )
//      {
//        /* Speed mode configuration */
//        pgpio->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 << (pos * 2)) ;
//        pgpio->OSPEEDR |= ((config & PIN_SPEED_MASK) >> 13 ) << (pos * 2) ;

//        /* Output mode configuration*/
//        pgpio->OTYPER  &= ~((GPIO_OTYPER_OT_0) << ((uint16_t)pos)) ;
//				if ( config & PIN_ODRAIN )
//				{
//        	pgpio->OTYPER |= (GPIO_OTYPER_OT_0) << pos ;
//				}
//      }
//      /* Pull-up Pull down resistor configuration*/
//      pgpio->PUPDR &= ~(GPIO_PUPDR_PUPDR0 << ((uint16_t)pos * 2));
//      pgpio->PUPDR |= ((config & PIN_PULL_MASK) >> 2) << (pos * 2) ;

//			pgpio->AFR[pos >> 3] &= ~(0x000F << ((pos & 7)*4)) ;
//			pgpio->AFR[pos >> 3] |=	((config & PIN_PERI_MASK) >> 4) << ((pos & 7)*4) ;
//			if ( config & PIN_HIGH )
//			{
//				pgpio->BSRRL = thispin ;		
//			}
//			else
//			{
//				pgpio->BSRRH = thispin ;		
//			}
//    }
//  }
//}
		
//#endif



//#ifdef PCBSKY

//uint32_t extraInputInUse( uint32_t input )
//{
//	if ( g_eeGeneral.pb1source == input ) return 1 ;
//	if ( g_eeGeneral.pb2source == input ) return 1 ;
//	if ( g_eeGeneral.ailsource == input ) return 1 ;
//	if ( g_eeGeneral.rudsource == input ) return 1 ;
//	if ( g_eeGeneral.geasource == input ) return 1 ;
//	if ( g_eeGeneral.thrsource == input ) return 1 ;
//	if ( g_eeGeneral.elesource == input ) return 1 ;
//	if ( g_eeGeneral.pb3source == input ) return 1 ;
//	if ( g_eeGeneral.pb4source == input ) return 1 ;
//	return 0 ;
//}

//void initExtraInput()
//{
//#ifndef ARUNI
//	configure_pins( 0x00004000, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;   // PB14 is already on the free pin list
//#endif
//}

//// PCBSKY
//void init_keys()
//{
//	register Pio *pioptr ;
	
//	pioptr = PIOC ;
//	// Next section configures the key inputs on the LCD data
//#ifndef REVA
//#ifdef ARUNI
//	pioptr->PIO_PER = 0x0000003BL ;		// Enable bits 0,1,3,4,5
//	pioptr->PIO_OER = 0 ;		          // Set no output bit
//	pioptr->PIO_ODR = 0x0000003BL ;		// Set bits 0, 1, 3, 4, 5 input
//	pioptr->PIO_PUER = 0x0000003BL ;	// Set bits 0, 1, 3, 4, 5 with pullups
//#else
//	pioptr->PIO_PER = 0x0000003BL ;		// Enable bits 1,3,4,5, 0
//	pioptr->PIO_OER = PIO_PC0 ;		// Set bit 0 output
//	pioptr->PIO_ODR = 0x0000003AL ;		// Set bits 1, 3, 4, 5 input
//	pioptr->PIO_PUER = 0x0000003AL ;		// Set bits 1, 3, 4, 5 with pullups
//#endif
//#else	
//	pioptr->PIO_PER = 0x0000003DL ;		// Enable bits 2,3,4,5, 0
//	pioptr->PIO_OER = PIO_PC0 ;		// Set bit 0 output
//	pioptr->PIO_ODR = 0x0000003CL ;		// Set bits 2, 3, 4, 5 input
//	pioptr->PIO_PUER = 0x0000003CL ;		// Set bits 2, 3, 4, 5 with pullups
//#endif

//	pioptr = PIOB ;
//#ifndef REVA
//	pioptr->PIO_PUER = PIO_PB5 ;					// Enable pullup on bit B5 (MENU)
//	pioptr->PIO_PER = PIO_PB5 ;					// Enable bit B5
//#else	
//	pioptr->PIO_PUER = PIO_PB6 ;					// Enable pullup on bit B6 (MENU)
//	pioptr->PIO_PER = PIO_PB6 ;					// Enable bit B6
//#endif
//}

//// Assumes PMC has already enabled clocks to ports
//// PCBSKY
//void setup_switches()
//{
	
//#ifndef REVA
//	configure_pins( 0x01808087, PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
//	configure_pins( 0x00000030, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;
//	configure_pins( 0x91114900, PIN_ENABLE | PIN_INPUT | PIN_PORTC | PIN_PULLUP ) ;
//#endif 
	
//#ifdef REVA
//	register Pio *pioptr ;
	
//	pioptr = PIOA ;
//	pioptr->PIO_PER = 0xF8008184 ;		// Enable bits
//	pioptr->PIO_ODR = 0xF8008184 ;		// Set bits input
//	pioptr->PIO_PUER = 0xF8008184 ;		// Set bits with pullups

//	pioptr = PIOB ;
//	pioptr->PIO_PER = 0x00000010 ;		// Enable bits
//	pioptr->PIO_ODR = 0x00000010 ;		// Set bits input
//	pioptr->PIO_PUER = 0x00000010 ;		// Set bits with pullups

//	pioptr = PIOC ;
//	pioptr->PIO_PER = 0x10014900 ;		// Enable bits
//	pioptr->PIO_ODR = 0x10014900 ;		// Set bits input
//	pioptr->PIO_PUER = 0x10014900 ;		// Set bits with pullups
//#endif 
//}


//// Switch input pins
//// Needs updating for REVB board ********
//// AIL-DR  PA2
//// TRIM_LH_DOWN PA7 (PA23)
//// ELE_DR   PA8 (PC31)
//// RUN_DR   PA15
//// TRIM_LV_DOWN  PA27 (PA24)
//// SW_TCUT     PA28 (PC20)
//// TRIM_RH_DOWN    PA29 (PA0)
//// TRIM_RV_UP    PA30 (PA1)
//// TRIM_LH_UP    //PB4
//// SW-TRAIN    PC8
//// TRIM_RH_UP   PC9
//// TRIM_RV_DOWN   PC10
//// SW_IDL2     PC11
//// SW_IDL1     PC14
//// SW_GEAR     PC16
//// TRIM_LV_UP   PC28

//// KEY_MENU    PB6 (PB5)
//// KEY_EXIT    PA31 (PC24)
//// Shared with LCD data
//// KEY_DOWN  LCD5  PC3
//// KEY_UP    LCD6  PC2
//// KEY_RIGHT LCD4  PC4
//// KEY_LEFT  LCD3  PC5

//// PORTA 1111 1000 0000 0000 1000 0001 1000 0100 = 0xF8008184 proto
//// PORTA 0000 0001 1000 0000 1000 0000 0000 0111 = 0x01808087 REVB
//// PORTB 0000 0000 0001 0000										 = 0x0010     proto
//// PORTB 0000 0000 0010 0000										 = 0x0030     REVB
//// PORTC 0001 0000 0000 0001 0100 1001 0000 0000 = 0x10014900 proto
//// PORTC 1001 0001 0001 0001 0100 1001 0000 0000 = 0x91114900 REVB


//// Prototype
//// Free pins (PA16 is stock buzzer)
//// PA23, PA24, PA25, PB7, PB13
//// PC20, PC21(labelled 17), PC22, PC24
//// REVB
//// PA25, use for stock buzzer
//// PB14, PB6
//// PC21, PC19, PC15 (PPM2 output)

//// PCBSKY
//void config_free_pins()
//{
	
//#ifndef REVA
//#ifdef ARUNI
//	configure_pins( PIO_PA25, PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
//#else
//#ifndef REVX
//	configure_pins( PIO_PA25, PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
//#endif
//#endif
////	configure_pins( PIO_PB6 | PIO_PB14, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;
//	configure_pins( PIO_PB14, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;

////	configure_pins( PIO_PC19 | PIO_PC21, PIN_ENABLE | PIN_INPUT | PIN_PORTC | PIN_PULLUP ) ;	// 19 and 21 are rotary encoder
//#else 
//	register Pio *pioptr ;

//	pioptr = PIOA ;
//	pioptr->PIO_PER = 0x03800000L ;		// Enable bits A25,24,23
//	pioptr->PIO_ODR = 0x03800000L ;		// Set as input
//	pioptr->PIO_PUER = 0x03800000L ;	// Enable pullups

//	pioptr = PIOB ;
//	pioptr->PIO_PER = 0x00002080L ;		// Enable bits B13, 7
//	pioptr->PIO_ODR = 0x00002080L ;		// Set as input
//	pioptr->PIO_PUER = 0x00002080L ;	// Enable pullups

//	pioptr = PIOC ;
//	pioptr->PIO_PER = 0x01700000L ;		// Enable bits C24,22,21,20
//	pioptr->PIO_ODR = 0x01700000L ;		// Set as input
//	pioptr->PIO_PUER = 0x01700000L ;	// Enable pullups
//#endif  // REVA
//}



//#endif

//#endif // 

//#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
//uint8_t ExternalKeys ;
//uint8_t ExternalSet ;
//#endif

//// keys:
//// KEY_EXIT    PA31 (PC24)
//// KEY_MENU    PB6 (PB5)
//// KEY_DOWN  LCD5  PC3 (PC5)
//// KEY_UP    LCD6  PC2 (PC1)
//// KEY_RIGHT LCD4  PC4 (PC4)
//// KEY_LEFT  LCD3  PC5 (PC3)
//// Reqd. bit 6 LEFT, 5 RIGHT, 4 UP, 3 DOWN 2 EXIT 1 MENU
//// LCD pins 5 DOWN, 4 RIGHT, 3 LEFT, 1 UP

//#ifdef PCB9XT

//// Switches from M64:
//// ELE, RUD, TRAIN
//// M64Switches, bit 8=Train, bit 1=RUD, bit 2=ELE


//void init_trims()
//{
//// Trims 
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
//	configure_pins( 0x6000, PIN_INPUT | PIN_PULLUP | PIN_PORTA ) ;
//	configure_pins( 0x2080, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//}

//uint32_t read_keys()
//{
//	uint8_t keys ;
//	checkM64() ;
//	keys = ~M64Buttons ;
//	if ( g_model.BTfunction == BT_LCDDUMP )
//	{
//		if ( ExternalSet )
//		{
//			keys &= ~ExternalKeys ;
//		}
//	}
//	return keys ;
//}

//uint32_t readKeyUpgradeBit( uint8_t index )
//{
//  CPU_UINT xxx = 0 ;
//	uint32_t t = 1 << (index-1) ;

////	if ( t > 16 )
////	{
////		t >>= 2 ;
////	}

//	xxx = M64Trims & t ;
//	return xxx ;
//}

//uint32_t hwKeyState( uint8_t key )
//{
//  CPU_UINT xxx = 0 ;
//	register uint32_t a ;
//	uint32_t avpot = 0xFFFF ;
  
//	a = g_eeGeneral.analogMapping & MASK_6POS ;
//	if ( a )
//	{
//		avpot = M64Analog[ (a >> 2) + 3] ;
//		if ( g_eeGeneral.SixPositionCalibration[5] >  g_eeGeneral.SixPositionCalibration[0] )
//		{
//			avpot = 2047 - avpot ;
//		}
//	}
	
//	if( key > HSW_MAX )  return 0 ;

//	if ( ( key >= HSW_ThrCt ) && ( key <= HSW_Trainer ) )
//	{
//		return keyState( (EnumKeys)(key + ( SW_ThrCt - HSW_ThrCt ) ) ) ;
//	}
	
//	// Add other options here

//	xxx = 0 ; 
//	uint32_t as = AnalogSwitches ;

//	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )
//	{
//		uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
//		key -= HSW_Etrmdn ;		// 0 - 7
//		if (key >= 2 && key <= 5)       // swap back LH/RH trims
//		{
//			if ( g_eeGeneral.stickMode & 2 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		else
//		{
//			if ( g_eeGeneral.stickMode & 1 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		if ( ct )
//		{
//			key ^= 0x06 ;
//	 		if ( ct == 2 ) // Vintage style crosstrim
//			{
//	 			if (key >= 2 && key <= 5)       // swap back LH/RH trims
//				{
//					key ^= 0x06 ;
//				}
//			}
//		}
//		key += HSW_Etrmdn ;		// 0 - 7
//	}
////	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )
////	{
////		uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
////		if ( ct )
////		{
////			key -= HSW_Etrmdn ;		// 0 - 7
////			key ^= 0x06 ;
////	 		if ( ct == 2 ) // Vintage style crosstrim
//// 			{
////	 			if (key >= 2 && key <= 5)       // swap back LH/RH trims
////				{
////					key ^= 0x06 ;
////				}
//// 			}
////			key += HSW_Etrmdn ;		// 0 - 7
////		}
////	}

//  switch ( key )
//	{
//		case HSW_Ttrmup :
//			xxx = keyState( (EnumKeys) TRM_RV_DWN ) ;
//    break ;
		
//		case HSW_Ttrmdn :
//			xxx = keyState( (EnumKeys) TRM_RV_UP ) ;
//    break ;
		
//		case HSW_Rtrmup :
//			xxx = keyState( (EnumKeys) TRM_LH_DWN ) ;
//    break ;
		
//		case HSW_Rtrmdn :
//			xxx = keyState( (EnumKeys) TRM_LH_UP ) ;
//    break ;
		
//		case HSW_Atrmup :
//			xxx = keyState( (EnumKeys) TRM_RH_DWN ) ;
//    break ;
		
//		case HSW_Atrmdn :
//			xxx = keyState( (EnumKeys) TRM_RH_UP ) ;
//    break ;
		
//		case HSW_Etrmup :
//			xxx = keyState( (EnumKeys) TRM_LV_DWN ) ;
//    break ;
		
//		case HSW_Etrmdn :
//			xxx = keyState( (EnumKeys) TRM_LV_UP ) ;
//    break ;
		
////		case HSW_Ttrmup :
////			xxx = THR_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
//////			xxx = TrimBits & 1 ;
////    break ;
		
////		case HSW_Ttrmdn :
////			xxx = THR_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
//////			xxx = TrimBits & 2 ;
////    break ;
////		case HSW_Rtrmup :
////			xxx = RUD_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
//////			xxx = TrimBits & 4 ;
////    break ;
////		case HSW_Rtrmdn :
////			xxx = RUD_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
//////			xxx = TrimBits & 8 ;
////    break ;
////		case HSW_Atrmup :
////			xxx = AIL_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
//////			xxx = TrimBits & 0x10 ;
////    break ;
////		case HSW_Atrmdn :
////			xxx = AIL_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
//////			xxx = TrimBits & 0x20 ;
////    break ;
////		case HSW_Etrmup :
////			xxx = ELE_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
//////			xxx = TrimBits & 0x40 ;
////    break ;
////		case HSW_Etrmdn :
////			xxx = ELE_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
//////			xxx = TrimBits & 0x80 ;
////    break ;
		
//		case HSW_Thr3pos0 :
//    	xxx = as & AS_THR_SW ;	// SW_TCUT     PC20
//    break ;
		
//		case HSW_Thr3pos1 :
//			xxx = ~as & AS_THR_SW ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.thrsource ) ;
//    break ;

//		case HSW_Thr3pos2 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.thrsource ) ;
//    break ;
			 
//		case HSW_Ele3pos0 :
//			xxx = ~M64Switches & 0x0004 ;	// ELE_DR
////			xxx = ( g_eeGeneral.elesource == 5 ) ? av9 < 490 : ~c & 0x80000000 ;	// ELE_DR   PC31
//    break ;

//		case HSW_Ele3pos1 :
////			if ( g_eeGeneral.elesource == 5 )
////			{
////				xxx = av9 > 1500 ;
////			}
////      else
////			{
//				xxx = M64Switches & 0x0004 ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.elesource ) ;
////			}
//		break ;

//		case HSW_Ele3pos2 :
////			if ( g_eeGeneral.elesource == 5 )
////			{
////				xxx = ( av9 <= 1500 ) && ( av9 >= 490 ) ;
////			}
////      else
////			{
//				xxx = readKeyUpgradeBit( g_eeGeneral.elesource ) ;
////			}
//    break ;

//		case HSW_Rud3pos0 :
//		 xxx = ~M64Switches & 0x0002	;	// RUD_DR   PA15
//    break ;

//		case HSW_Rud3pos1 :
//			xxx = M64Switches & 0x0002 ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.rudsource ) ;
////			xxx = (a & 0x00008000) ; if ( xxx ) xxx = (c & 0x80000000) ;
//    break ;

//		case HSW_Rud3pos2 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.rudsource ) ;
////			xxx = ~c & 0x80000000 ;	// ELE_DR   PC31
//    break ;

//		case HSW_Ail3pos0 :
//			xxx = as & AS_AIL_SW ;	// AIL-DR  PA2
//    break ;

//		case HSW_Ail3pos1 :
////			xxx = (a & 0x00000004) ; if ( xxx ) xxx = (PIOB->PIO_PDSR & 0x00004000) ;
//			xxx = ~as & AS_AIL_SW ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.ailsource ) ;
//    break ;                               

//		case HSW_Ail3pos2 :
////			xxx = ~PIOB->PIO_PDSR & 0x00004000 ;
//			xxx = readKeyUpgradeBit( g_eeGeneral.ailsource ) ;
//    break ;

//		case HSW_Gear3pos0 :
//			xxx = as & AS_GEA_SW ;	// SW_GEAR     PC16
//    break ;

//		case HSW_Gear3pos1 :
////			xxx = (c & 0x00010000) ; if ( xxx ) xxx = (PIOB->PIO_PDSR & 0x00004000) ;
//			xxx = ~as & AS_GEA_SW ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.geasource ) ;
//    break ;

//		case HSW_Gear3pos2 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.geasource ) ;
////			xxx = ~PIOB->PIO_PDSR & 0x00004000 ;
//    break ;

//		case HSW_Pb1 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb1source ) ;
//    break ;
			 
//		case HSW_Pb2 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb2source ) ;
//    break ;

//		case HSW_Pb3 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb3source ) ;
//    break ;
			 
//		case HSW_Pb4 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb4source ) ;
//    break ;

//		case HSW_Ele6pos0 :
			
//			if ( avpot != 0xFFFF )
//			{
//				xxx = avpot > SixPositionTable[0] ;
//			}
//    break ;

//		case HSW_Ele6pos1 :
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[0] ) && ( avpot >= SixPositionTable[1] ) ;
//			}
//    break ;
		
//		case HSW_Ele6pos2 :
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[1] ) && ( avpot >= SixPositionTable[2] ) ;
//			}
//    break ;
		
//		case HSW_Ele6pos3 :
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[2] ) && ( avpot >= SixPositionTable[3] ) ;
//			}
//    break ;
		
//		case HSW_Ele6pos4 :
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[3] ) && ( avpot >= SixPositionTable[4] ) ;
//			}
//    break ;

//		case HSW_Ele6pos5 :
//			if ( avpot != 0xFFFF )
//			{
//				xxx = avpot < SixPositionTable[4] ;
//			}
//    break ;
		 
//	}
//	return xxx ;
//}

//uint32_t keyState(EnumKeys enuk)
//{
//	uint32_t as ;
//  CPU_UINT xxx = 0 ;
//  if(enuk < (int)DIM(keys))  return keys[enuk].state() ? 1 : 0 ;
	
//	as = AnalogSwitches ;

//	switch((uint8_t)enuk)
//	{
//    case SW_AileDR : xxx = ~as & AS_AIL_SW ;	// AIL-DR
//    break ;
//    case SW_ID0    : xxx = as & AS_IDL1_SW ;	// SW_IDL1
//    break ;
//    case SW_ID1    : xxx = (~as & AS_IDL1_SW) ; if ( xxx ) xxx = (~as & AS_IDL2_SW) ;
//    break ;
//    case SW_ID2    : xxx = as & AS_IDL2_SW ;	// SW_IDL2
//    break ;
//		case SW_Gear   : xxx = ~as & AS_GEA_SW ;	// SW_GEAR
//    break ;
//    case SW_ThrCt  : xxx = ~as & AS_THR_SW ;	// SW_TCUT
//    break ;
//		case SW_ElevDR : xxx = M64Switches & 0x0004 ;	// ELE_DR
//    break ;
//    case SW_RuddDR : xxx = M64Switches & 0x0002 ;	// RUD_DR
//    break ;
//    case SW_Trainer: xxx = M64Switches & 0x00000100 ;	// SW-TRAIN
//    break ;
//	}
	
//  if ( xxx )
//  {
//    return 1 ;
//  }
//	return 0 ;
//}

//uint32_t switchPosition( uint32_t swtch )
//{
//	if ( hwKeyState( swtch ) )
//	{
//		return 0 ;
//	}
//	swtch += 1 ;
//	if ( hwKeyState( swtch ) )
//	{
//		return 1 ;			
//	}
//	if ( swtch == HSW_Ele6pos1 )
//	{
//		if ( hwKeyState( HSW_Ele6pos3 ) )
//		{
//			return 3 ;
//		}
//		if ( hwKeyState( HSW_Ele6pos4 ) )
//		{
//			return 4 ;
//		}
//		if ( hwKeyState( HSW_Ele6pos5 ) )
//		{
//			return 5 ;
//		}
//	}
//	return 2 ;
//}

//void init_keys()
//{
//	// Nothing to do
//}

//void setup_switches()
//{
//	// Nothing to do
//}

//uint32_t read_trims()
//{
//	uint32_t trims ;
//	uint32_t trima ;

//	trims = 0 ;

//	trima = GPIOC->IDR ;

//// TRIM_LH_DOWN
//	if ( ( trima & 0x00000080 ) == 0 )
//	{
//		trims |= 1 ;
//	}

//// TRIM_LH_UP
//	if ( ( trima & 0x00002000 ) == 0 )
//	{
//		trims |= 2 ;
//	}
  
//	trima = GPIOA->IDR ;

//// TRIM_RH_DOWN
//	if ( ( trima & 0x00004000 ) == 0 )
//	{
//		trims |= 0x40 ;
//	}

//// TRIM_RH_UP
//	if ( ( trima & 0x00002000 ) == 0 )
//	{
//		trims |= 0x80 ;
//	}

//	trima = AnalogSwitches ;

//// TRIM_LV_UP
//	if ( trima & AS_LV_TRIM_UP )
//	{
//		trims |= 8 ;
//	}

//// TRIM_RV_UP
//	if ( trima & AS_RV_TRIM_UP )
//	{
//		trims |= 0x20 ;
//	}

//// TRIM_RV_DOWN
//	if ( trima & AS_RV_TRIM_DN )
//	{
//		trims |= 0x10 ;
//	}

//// TRIM_LV_DOWN
//	if ( trima & AS_LV_TRIM_DN )
//	{
//		trims |= 4 ;
//	}

//	return trims ;
//}

//#endif

//#ifdef PCBSKY

//uint16_t ExtraInputs ;

//uint32_t read_keys()
//{
//	register uint32_t x ;
//	register uint32_t y ; // target y: LEFT:6 RIGHT:5 UP:4 DOWN:3 EXIT:2 MENU:1

//	x = LcdLock ? LcdInputs : (PIOC->PIO_PDSR << 1) ; // 6 LEFT, 5 RIGHT, 4 DOWN, 3 UP ()
//#ifndef REVA
//	y = x & 0x00000020 ;		// RIGHT
//	if ( x & 0x00000004 )
//	{
//		y |= 0x00000010 ;			// UP
//	}
//	if ( x & 0x00000010 )
//	{
//		y |= 0x00000040 ;			// LEFT
//	}
//	if ( x & 0x00000040 )
//	{
//		y |= 0x00000008 ;			// DOWN
//	}
//#ifdef ARUNI
//  // target x: CS:6 PB14:5 PA25:4 KEY3:3 KEY2:2 KEY1:1 COM:0
//  register uint32_t z = ~x;
//  x = ((z >> 1) & 0x01);  // COM
//  x |= ((z >> 2) & 0x02); // KEY1
//  x |= ((z >> 5) & 0x0C); // KEY3,KEY2
//	if (~PIOA->PIO_PDSR & 0x02000000) // PA25
//    x |= 0x10;
//	if (~PIOB->PIO_PDSR & 0x00004000) // PB14
//    x |= 0x20;
//	if (~PIOC->PIO_PDSR & 0x04000000) // PC26(LCD_CS)
//    x |= 0x40;
//#else  // ARUNI
//	x = ~x ;
//	x &= ~0x40 ;
//	if ( x & 8 )
//	{
//		x |= 0x40 ;
//	}
//	x >>= 6 ;
//	x &= 7 ;	// LCD7,6,2
//#ifndef REVX
//extern uint8_t Co_proc_status[] ;
//	uint8_t temp = (uint8_t)Co_proc_status[9] ;
//	if ( (temp & 0x40) == 0 )
//	{
//		x |= 0x20 ;
//	}
//	if ( (temp & 0x08) == 0 )
//	{
//		x |= 0x10 ;
//	}
//	if ( (temp & 0x02) == 0 )
//	{
//		x |= 0x08 ;
//	}
//#endif	// REVX

//	uint32_t av9 = Analog_values[9] ;
//	if ( av9 < 490 )
//	{
//		x |= 0x40 ;
//	}
//	else
//	{
//		if ( av9 > 1500 )
//		{
//			x |= 0x80 ;
//		}
//		else
//		{
//			x |= 0x0100 ;
//		}
//	}
//#endif  // ARUNI
//#else	  // nREVA
//	y = x & 0x00000060 ;
//	if ( x & 0x00000008 )
//	{
//		y |= 0x00000010 ;
//	}
//	if ( x & 0x00000010 )
//	{
//		y |= 0x00000008 ;
//	}
//	x = ~x ;
//	if ( x & 8 )
//	{
//		x |= 0x40 ;
//	}
//	x >>= 6 ;
//	x &= 7 ;
//extern uint8_t Co_proc_status[] ;
//	uint8_t temp = (uint8_t)Co_proc_status[9] ;
//	if ( (temp & 0x40) == 0 )
//	{
//		x |= 0x20 ;
//	}
//	if ( (temp & 0x08) == 0 )
//	{
//		x |= 0x10 ;
//	}
//	if ( (temp & 0x02) == 0 )
//	{
//		x |= 0x08 ;
//	}
//#endif  // REVA
//	ExtraInputs = x ;

//	if (LcdLock)
//	{
//		y |= ( LcdInputs >> 8 ) & 0x06 ;
//	}
//	else
//	{
//#ifndef REVA
//		if ( PIOC->PIO_PDSR & 0x01000000 )
//#else 
//		if ( PIOA->PIO_PDSR & 0x80000000 )
//#endif
//		{
//			y |= 4 ;		// EXIT
//		}
//#ifndef REVA
//		if ( PIOB->PIO_PDSR & 0x000000020 )
//#else 
//		if ( PIOB->PIO_PDSR & 0x000000040 )
//#endif
//		{
//			y |= 2 ;		// MENU
//		}
//	}
//	if ( ( g_model.com2Function == COM2_FUNC_LCD ) || ( g_model.BTfunction == BT_LCDDUMP ) )
//	{
//		if ( ExternalSet )
//		{
//			y &= ~ExternalKeys ;
//		}
//	}
//	return y ;
//}
//#endif

//#ifdef PCBSKY
//uint32_t read_trims()
//{
//	uint32_t trims ;
//	uint32_t trima ;

//	trims = 0 ;

//	trima = PIOA->PIO_PDSR ;
//// TRIM_LH_DOWN    PA7 (PA23)
//#ifndef REVA
// #ifndef REVX
//	if ( ( trima & 0x00800000 ) == 0 )
// #else
//	if ( ( PIOB->PIO_PDSR & 0x10 ) == 0 )
// #endif
//#else
//	if ( ( trima & 0x0080 ) == 0 )
//#endif
//	{
//		trims |= 1 ;
//	}
    
//// TRIM_LV_DOWN  PA27 (PA24)
//#ifndef REVA
//	if ( ( trima & 0x01000000 ) == 0 )
//#else
//	if ( ( trima & 0x08000000 ) == 0 )
//#endif
//	{
//		trims |= 4 ;
//	}

//// TRIM_RV_UP    PA30 (PA1)
//#ifndef REVA
// #ifndef REVX
//	if ( ( trima & 0x00000002 ) == 0 )
// #else
//	if ( ( PIOC->PIO_PDSR & 0x00000400 ) == 0 )
// #endif
//#else
//	if ( ( trima & 0x40000000 ) == 0 )
//#endif
//	{
//		trims |= 0x20 ;
//	}

//// TRIM_RH_DOWN    PA29 (PA0)
//#ifndef REVA
//	if ( ( trima & 0x00000001 ) == 0 )
//#else 
//	if ( ( trima & 0x20000000 ) == 0 )
//#endif 
//	{
//		trims |= 0x40 ;
//	}

//// TRIM_LH_UP PB4
//#ifndef REVX
//	if ( ( PIOB->PIO_PDSR & 0x10 ) == 0 )
//#else
//	if ( ( trima & 0x00800000 ) == 0 )
//#endif
//	{
//		trims |= 2 ;
//	}

//	trima = PIOC->PIO_PDSR ;
//// TRIM_LV_UP   PC28
//	if ( ( trima & 0x10000000 ) == 0 )
//	{
//		trims |= 8 ;
//	}

//// TRIM_RV_DOWN   PC10
//#ifndef REVX
//	if ( ( trima & 0x00000400 ) == 0 )
//#else
//	if ( ( PIOA->PIO_PDSR & 0x00000002 ) == 0 )
//#endif
//	{
//		trims |= 0x10 ;
//	}

//// TRIM_RH_UP   PC9
//	if ( ( trima & 0x00000200 ) == 0 )
//	{
//		trims |= 0x80 ;
//	}

//	return trims ;
//}

//#endif


//#ifdef PCBSKY

//uint32_t readKeyUpgradeBit( uint8_t index )
//{
//#ifdef ARUNI
//	CPU_UINT xxx = (ExtraInputs & (1 << (index - 1))) ;
//#else
//  CPU_UINT xxx = 0 ;
//	uint32_t t = 1 << (index-1) ;
//	if ( t == 8 )
//	{
//		xxx = (~PIOB->PIO_PDSR & 0x00004000) ;	// DAC1
//	}
//	else if ( t == 16 )
//	{
//		xxx = ~PIOC->PIO_PDSR & 0x80000000 ;	// ELE_DR   PC31	
//	}
//#ifndef REVX
//	else if ( g_eeGeneral.ar9xBoard && ( t == 64 ) )		// AR9X
//	{
//		xxx = ~PIOA->PIO_PDSR & 0x02000000 ; // PA25
//	}
//	else if ( ( g_eeGeneral.ar9xBoard == 0 ) && ( t == 512 ) )	// SKY
//	{
//		xxx = ~PIOA->PIO_PDSR & 0x02000000 ; // PA25
//	}
//#endif
//	else
//	{
//		if ( t > 16 )
//		{
//			t >>= 2 ;
//		}
//		xxx = ExtraInputs & t ;
//	}
//#endif
//	return xxx ;
//}

//uint32_t hwKeyState( uint8_t key )
//{
//	uint32_t externalOK = 0 ;
//	if ( ( g_model.com2Function == COM2_FUNC_LCD ) || ( g_model.BTfunction == BT_LCDDUMP ) )
//	{
//		if ( ExternalSwitchesValid )
//		{
//			externalOK = 1 ;
//		}
//	}
	
//	register uint32_t a ;
//	register uint32_t c ;
//#ifdef ARUNI
//	uint8_t avpot = 0 ;
//	uint32_t anaIndex = 8 ;   // NONE:0,COMM:1,EXT1:2,EXT2:3,EXT3:4,PA25:5,PB14:6,PC26:7
//#else
//	uint32_t av9 = Analog_values[9] ;
//	uint32_t avpot = 0xFFFF ;
//	uint32_t anaIndex = g_eeGeneral.ar9xBoard ? 6 : 9 ;

//#endif

//#ifdef ARUNI
//	uint8_t sixpos = g_eeGeneral.analogMapping & MASK_6POS ;
//	if ( sixpos )
//	{
//			avpot = anaIn( (sixpos >> 2) + 3) >> 3 ; // truncate 11b to 8b
//	}
//#else
//	a = g_eeGeneral.analogMapping & MASK_6POS ;
//	if ( a )
//	{
//		if ( a == USE_AUX_6POS )
//		{
//			avpot = av9 ;
//		}
//		else
//		{
//			avpot = Analog_values[ (a >> 2) + 3] ;
//		}
//		if ( g_eeGeneral.SixPositionCalibration[5] >  g_eeGeneral.SixPositionCalibration[0] )
//		{
//			avpot = 4095 - avpot ;
//		}
//	}
//#endif

//  CPU_UINT xxx = 0 ;
//  if( key > HSW_MAX )  return 0 ;

//	if ( ( key >= HSW_ThrCt ) && ( key <= HSW_Trainer ) )
//	{
//		if ( externalOK )
//		{
//			if ( key == HSW_ThrCt )
//			{
//				return ( ExternalSwitches & 0x20 ) ? 1 : 0 ;
//			}
//		}
//		return keyState( (EnumKeys)(key + ( SW_ThrCt - HSW_ThrCt ) ) ) ;
//	}

//	a = PIOA->PIO_PDSR ;
//	c = PIOC->PIO_PDSR ;
	
//	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )
//	{
//		uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
//		key -= HSW_Etrmdn ;		// 0 - 7
//		if (key >= 2 && key <= 5)       // swap back LH/RH trims
//		{
//			if ( g_eeGeneral.stickMode & 2 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		else
//		{
//			if ( g_eeGeneral.stickMode & 1 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		if ( ct )
//		{
//			key ^= 0x06 ;
//	 		if ( ct == 2 ) // Vintage style crosstrim
//			{
//	 			if (key >= 2 && key <= 5)       // swap back LH/RH trims
//				{
//					key ^= 0x06 ;
//				}
//			}
//		}
//		key += HSW_Etrmdn ;		// 0 - 7
//	}
////	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )
////	{
////		uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
////		if ( ct )
////		{
////			key -= HSW_Etrmdn ;		// 0 - 7
////			key ^= 0x06 ;
////	 		if ( ct == 2 ) // Vintage style crosstrim
//// 			{
////	 			if (key >= 2 && key <= 5)       // swap back LH/RH trims
////				{
////					key ^= 0x06 ;
////				}
//// 			}
////			key += HSW_Etrmdn ;		// 0 - 7
////		}
////	}

//  switch ( key )
//	{
//		case HSW_Ttrmup :
//			xxx = keyState( (EnumKeys) TRM_RV_DWN ) ;
//    break ;
		
//		case HSW_Ttrmdn :
//			xxx = keyState( (EnumKeys) TRM_RV_UP ) ;
//    break ;
		
//		case HSW_Rtrmup :
//			xxx = keyState( (EnumKeys) TRM_LH_DWN ) ;
//    break ;
		
//		case HSW_Rtrmdn :
//			xxx = keyState( (EnumKeys) TRM_LH_UP ) ;
//    break ;
		
//		case HSW_Atrmup :
//			xxx = keyState( (EnumKeys) TRM_RH_DWN ) ;
//    break ;
		
//		case HSW_Atrmdn :
//			xxx = keyState( (EnumKeys) TRM_RH_UP ) ;
//    break ;
		
//		case HSW_Etrmup :
//			xxx = keyState( (EnumKeys) TRM_LV_DWN ) ;
//    break ;
		
//		case HSW_Etrmdn :
//			xxx = keyState( (EnumKeys) TRM_LV_UP ) ;
//    break ;
		
		
////		case HSW_Ttrmup :
////			xxx = THR_STICK ;
////			if ( ct )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
//////			xxx = TrimBits & 1 ;
////    break ;
		
////		case HSW_Ttrmdn :
////			xxx = THR_STICK ;
////			if ( ct )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
//////			xxx = TrimBits & 2 ;
////    break ;
////		case HSW_Rtrmup :
////			xxx = RUD_STICK ;
////			if ( ct == 1 )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
//////			xxx = TrimBits & 4 ;
////    break ;
////		case HSW_Rtrmdn :
////			xxx = RUD_STICK ;
////			if ( ct == 1 )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
//////			xxx = TrimBits & 8 ;
////    break ;
////		case HSW_Atrmup :
////			xxx = AIL_STICK ;
////			if ( ct == 1 )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
//////			xxx = TrimBits & 0x10 ;
////    break ;
////		case HSW_Atrmdn :
////			xxx = AIL_STICK ;
////			if ( ct == 1 )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
//////			xxx = TrimBits & 0x20 ;
////    break ;
////		case HSW_Etrmup :
////			xxx = ELE_STICK ;
////			if ( ct )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
//////			xxx = TrimBits & 0x40 ;
////    break ;
////		case HSW_Etrmdn :
////			xxx = ELE_STICK ;
////			if ( ct )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
//////			xxx = TrimBits & 0x80 ;
////    break ;
		
////#ifdef REVB
////    case HSW_ElevDR : xxx = c & 0x80000000 ;	// ELE_DR   PC31
////#else 
////    case HSW_ElevDR : xxx = a & 0x00000100 ;	// ELE_DR   PA8
////#endif 
////    break ;
    
////    case HSW_AileDR : xxx = a & 0x00000004 ;	// AIL-DR  PA2
////    break ;

////    case HSW_RuddDR : xxx = a & 0x00008000 ;	// RUD_DR   PA15
////    break ;
////      //     INP_G_ID1 INP_E_ID2
////      // id0    0        1
////      // id1    1        1
////      // id2    1        0
////    case HSW_ID0    : xxx = ~c & 0x00004000 ;	// SW_IDL1     PC14
////    break ;
////    case HSW_ID1    : xxx = (c & 0x00004000) ; if ( xxx ) xxx = (PIOC->PIO_PDSR & 0x00000800);
////    break ;
////    case HSW_ID2    : xxx = ~c & 0x00000800 ;	// SW_IDL2     PC11
////    break ;

    
////		case HSW_Gear   : xxx = c & 0x00010000 ;	// SW_GEAR     PC16
////    break ;

////#ifdef REVB
////    case HSW_ThrCt  : xxx = c & 0x00100000 ;	// SW_TCUT     PC20
////#else 
////    case HSW_ThrCt  : xxx = a & 0x10000000 ;	// SW_TCUT     PA28
////#endif 
////    break ;

////    case HSW_Trainer: xxx = c & 0x00000100 ;	// SW-TRAIN    PC8
////    break ;
		
//		case HSW_Thr3pos0 :
//			if ( g_eeGeneral.thrsource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x40 ;
//			}
//      else
//			{
//	    	xxx = ~c & 0x00100000 ;	// SW_TCUT     PC20
//			}
//    break ;
		
//		case HSW_Thr3pos1 :
//			if ( g_eeGeneral.thrsource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x80 ;
//			}
//      else
//			{
//				xxx = (c & 0x00100000) ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.thrsource ) ;
//			}
//    break ;

//		case HSW_Thr3pos2 :
//			if ( g_eeGeneral.thrsource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x100 ;
//			}
//      else
//			{
//				xxx = readKeyUpgradeBit( g_eeGeneral.thrsource ) ;
//			}
//    break ;
		
//#ifdef ARUNI
//		case HSW_Ele3pos0 :
//			{
//			  xxx = ~c & 0x80000000 ;	// ELE_DR   PC31
//			}
//    break ;

//		case HSW_Ele3pos1 :
//			{
//				xxx = c & 0x80000000 ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.elesource ) ;
//			}
//    break ;

//		case HSW_Ele3pos2 :
//			{
//				xxx = readKeyUpgradeBit( g_eeGeneral.elesource ) ;
//			}
//    break ;
//#else
//		case HSW_Ele3pos0 :
//			xxx = ( g_eeGeneral.elesource == 5 ) ? ExtraInputs & 0x40 : ~c & 0x80000000 ;	// ELE_DR   PC31
//    break ;

//		case HSW_Ele3pos1 :
//			if ( g_eeGeneral.elesource == 5 )
//			{
//				xxx = ExtraInputs & 0x80 ;
//			}
//      else
//			{
//				xxx = c & 0x80000000 ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.elesource ) ;
//			}
//		break ;

//		case HSW_Ele3pos2 :
//			if ( g_eeGeneral.elesource == 5 )
//			{
//				xxx = ExtraInputs & 0x100 ;
//			}
//      else
//			{
//				xxx = readKeyUpgradeBit( g_eeGeneral.elesource ) ;
//			}
//    break ;
//#endif

//		case HSW_Rud3pos0 :
//			if ( g_eeGeneral.rudsource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x40 ;
//			}
//      else
//			{
//				xxx = ~a & 0x00008000 ;	// RUD_DR   PA15
//			}
//    break ;

//		case HSW_Rud3pos1 :
//			if ( g_eeGeneral.rudsource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x80 ;
//			}
//      else
//			{
//				xxx = (a & 0x00008000) ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.rudsource ) ;
//			}
////			xxx = (a & 0x00008000) ; if ( xxx ) xxx = (c & 0x80000000) ;
//    break ;

//		case HSW_Rud3pos2 :
//			if ( g_eeGeneral.rudsource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x100 ;
//			}
//      else
//			{
//				xxx = readKeyUpgradeBit( g_eeGeneral.rudsource ) ;
//			}
////			xxx = ~c & 0x80000000 ;	// ELE_DR   PC31
//    break ;

//		case HSW_Ail3pos0 :
//			if ( g_eeGeneral.ailsource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x40 ;
//			}
//      else
//			{
//				xxx = ~a & 0x00000004 ;	// AIL-DR  PA2
//			}
//    break ;

//		case HSW_Ail3pos1 :
////			xxx = (a & 0x00000004) ; if ( xxx ) xxx = (PIOB->PIO_PDSR & 0x00004000) ;
//			if ( g_eeGeneral.ailsource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x80 ;
//			}
//      else
//			{
//				xxx = (a & 0x00000004) ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.ailsource ) ;
//			}
//    break ;                               

//		case HSW_Ail3pos2 :
////			xxx = ~PIOB->PIO_PDSR & 0x00004000 ;
//			if ( g_eeGeneral.ailsource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x100 ;
//			}
//      else
//			{
//				xxx = readKeyUpgradeBit( g_eeGeneral.ailsource ) ;
//			}
//    break ;

//		case HSW_Gear3pos0 :
//			if ( g_eeGeneral.geasource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x40 ;
//			}
//      else
//			{
//				xxx = ~c & 0x00010000 ;	// SW_GEAR     PC16
//			}
//    break ;

//		case HSW_Gear3pos1 :
////			xxx = (c & 0x00010000) ; if ( xxx ) xxx = (PIOB->PIO_PDSR & 0x00004000) ;
//			if ( g_eeGeneral.geasource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x80 ;
//			}
//      else
//			{
//				xxx = (c & 0x00010000) ; if ( xxx ) xxx = !readKeyUpgradeBit( g_eeGeneral.geasource ) ;
//			}
//    break ;

//		case HSW_Gear3pos2 :
//			if ( g_eeGeneral.geasource == anaIndex )
//			{
//				xxx = ExtraInputs & 0x100 ;
//			}
//      else
//			{
//				xxx = readKeyUpgradeBit( g_eeGeneral.geasource ) ;
//			}
////			xxx = ~PIOB->PIO_PDSR & 0x00004000 ;
//    break ;

//		case HSW_Ele6pos0 :
//#ifdef ARUNI
//			if ( sixpos )
//			{
//				xxx = (avpot <= g_eeGeneral.SixPositionTable[0]) ;
//			}
//#else
//			if ( avpot != 0xFFFF )
//			{
//				xxx = avpot > SixPositionTable[0] ;
//			}
//			else if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
//			{
//				xxx = av9 > 3438 ;
//			}
//			else
//			{
//				xxx = av9 > 0x7E0 ;
//			}
//#endif
//    break ;
			
//		case HSW_Ele6pos1 :
//#ifdef ARUNI
//			if ( sixpos )
//			{
//				xxx = (avpot > g_eeGeneral.SixPositionTable[0] &&
//				       avpot <= g_eeGeneral.SixPositionTable[1]) ;
//			}
//#else
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[0] ) && ( avpot >= SixPositionTable[1] ) ;
//			}
//			else if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
//			{
//				xxx = ( av9 <= 3438 ) && ( av9 >= 2525 ) ;
//			}
//			else
//			{
//				xxx = ( av9 <= 0x7E0 ) && ( av9 >= 0x7A8 ) ;
//			}
//#endif
//    break ;
			
//		case HSW_Ele6pos2 :
//#ifdef ARUNI
//			if ( sixpos )
//			{
//				xxx = (avpot > g_eeGeneral.SixPositionTable[1] &&
//				       avpot <= g_eeGeneral.SixPositionTable[2]) ;
//			}
//#else
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[1] ) && ( avpot >= SixPositionTable[2] ) ;
//			}
//			else 			if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
//			{
//				xxx = ( av9 <= 2525 ) && ( av9 >= 2159 ) ;
//			}
//			else
//			{
//				xxx = ( av9 <= 0x7A8 ) && ( av9 >= 0x770 ) ;
//			}
//#endif
//    break ;
			
//		case HSW_Ele6pos3 :
//#ifdef ARUNI
//			if ( sixpos )
//			{
//				xxx = (avpot > g_eeGeneral.SixPositionTable[2] &&
//				       avpot <= g_eeGeneral.SixPositionTable[3]) ;
//			}
//#else
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[2] ) && ( avpot >= SixPositionTable[3] ) ;
//			}
//			else 			if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
//			{
//				xxx = ( av9 <= 2159 ) && ( av9 >= 1974 ) ;
//			}
//			else
//			{
//				xxx = ( av9 <= 0x770 ) && ( av9 >= 0x340 ) ;
//			}
//#endif
//    break ;
			
//		case HSW_Ele6pos4 :
//#ifdef ARUNI
//			if ( sixpos )
//			{
//				xxx = (avpot > g_eeGeneral.SixPositionTable[3] &&
//				       avpot <= g_eeGeneral.SixPositionTable[4]) ;
//			}
//#else
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[3] ) && ( avpot >= SixPositionTable[4] ) ;
//			}
//			else 			if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
//			{
//				xxx = ( av9 <= 1974 ) && ( av9 >= 950 ) ;
//			}
//			else
//			{
//				xxx = ( av9 <= 0x340 ) && ( av9 >= 0x070 ) ;
//			}
//#endif
//    break ;
			
//		case HSW_Ele6pos5 :
//#ifdef ARUNI
//			if ( sixpos )
//			{
//				xxx = (avpot > g_eeGeneral.SixPositionTable[4]) ;
//			}
//#else
//			if ( avpot != 0xFFFF )
//			{
//				xxx = avpot < SixPositionTable[4] ;
//			}
//			else 			if ( g_eeGeneral.switchMapping & USE_ELE_6PSB )
//			{
//				xxx = av9 < 950 ;
//			}
//			else
//			{
//				xxx = av9 < 0x070 ;
//			}
//#endif
//    break ;

//		case HSW_Pb1 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb1source ) ;
//    break ;
			 
//		case HSW_Pb2 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb2source ) ;
//    break ;

//		case HSW_Pb3 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb3source ) ;
//    break ;
			 
//		case HSW_Pb4 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb4source ) ;
//    break ;

//    default:
//    break ;
		
//  }

//  if ( xxx )
//  {
//    return 1 ;
//  }
//  return 0;
	
//}

//// Returns 0, 1 or 2 (or 3,4,5) for ^ - or v (or 6pos)
//uint32_t switchPosition( uint32_t swtch )
//{
//	if ( hwKeyState( swtch ) )
//	{
//		return 0 ;
//	}
//	if ( ( swtch >= HSW_ThrCt ) && ( swtch <= HSW_ElevDR ) )
//	{
//		return 1 ;	// 2-pos switch
//	}
//	if ( ( swtch >= HSW_AileDR ) && ( swtch <= HSW_Trainer ) )
//	{
//		return 1 ;	// 2-pos switch
//	}
//	swtch += 1 ;
//	if ( hwKeyState( swtch ) )
//	{
//		return 1 ;			
//	}
//	if ( swtch == HSW_Ele6pos1 )
//	{
//		if ( hwKeyState( HSW_Ele6pos3 ) )
//		{
//			return 3 ;
//		}
//		if ( hwKeyState( HSW_Ele6pos4 ) )
//		{
//			return 4 ;
//		}
//		if ( hwKeyState( HSW_Ele6pos5 ) )
//		{
//			return 5 ;
//		}
//	}
//	return 2 ;
//}




//uint32_t keyState(EnumKeys enuk)
//{
//	uint32_t externalOK = 0 ;
//	if ( ( g_model.com2Function == COM2_FUNC_LCD ) || ( g_model.BTfunction == BT_LCDDUMP ) )
//	{
//		if ( ExternalSwitchesValid )
//		{
//			externalOK = 1 ;
//		}
//	}

//	if ( externalOK )
//	{
//		if ( enuk == SW_ThrCt )
//		{
//			return ( ExternalSwitches & 0x20 ) ? 1 : 0 ;
//		}
//	}

//	register uint32_t a ;
//	register uint32_t c ;

//  CPU_UINT xxx = 0 ;
//  if(enuk < (int)DIM(keys))  return keys[enuk].state() ? 1 : 0 ;

//	a = PIOA->PIO_PDSR ;
//	c = PIOC->PIO_PDSR ;
//	switch((uint8_t)enuk)
//	{
//#ifndef REVA
//    case SW_ElevDR : xxx = c & 0x80000000 ;	// ELE_DR   PC31
//#else 
//    case SW_ElevDR : xxx = a & 0x00000100 ;	// ELE_DR   PA8
//#endif 
//    break ;
    
//    case SW_AileDR : xxx = a & 0x00000004 ;	// AIL-DR  PA2
//    break ;

//    case SW_RuddDR : xxx = a & 0x00008000 ;	// RUN_DR   PA15
//    break ;
//      //     INP_G_ID1 INP_E_ID2
//      // id0    0        1
//      // id1    1        1
//      // id2    1        0
//    case SW_ID0    : xxx = ~c & 0x00004000 ;	// SW_IDL1     PC14
//    break ;
//    case SW_ID1    : xxx = (c & 0x00004000) ; if ( xxx ) xxx = (PIOC->PIO_PDSR & 0x00000800);
//    break ;
//    case SW_ID2    : xxx = ~c & 0x00000800 ;	// SW_IDL2     PC11
//    break ;

    
//		case SW_Gear   : xxx = c & 0x00010000 ;	// SW_GEAR     PC16
//    break ;

//#ifndef REVA
//    case SW_ThrCt  : xxx = c & 0x00100000 ;	// SW_TCUT     PC20
//#else 
//    case SW_ThrCt  : xxx = a & 0x10000000 ;	// SW_TCUT     PA28
//#endif 
//    break ;

//    case SW_Trainer: xxx = c & 0x00000100 ;	// SW-TRAIN    PC8
//    break ;
//    default:;
//  }

//  if ( xxx )
//  {
//    return 1 ;
//  }
//  return 0;
//}
//#endif // PCBSKY


//#ifdef PCBX9D
//void init_keys()
//{
//#ifdef REV19
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
//	configure_pins( PIN_BUTTON_ENCODER, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//	configure_pins( PIN_BUTTON_MENU | PIN_BUTTON_EXIT | PIN_BUTTON_PAGE, PIN_INPUT | PIN_PULLUP | PIN_PORTD) ;
//#else
// #ifdef PCBX9LITE
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	configure_pins( PIN_BUTTON_MENU | PIN_BUTTON_EXIT | PIN_BUTTON_PAGE | PIN_BUTTON_ENCODER, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
// #else // X3
//  #ifdef PCBX7
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//   #ifdef PCBT12
//	configure_pins( 0x008C, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	configure_pins( 0x0E00, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//   #else
//	configure_pins( 0x008C, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	configure_pins( 0x0400, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//   #endif
//  #else // PCBX7
//   #ifdef REV9E
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN ; 		// Enable portF clock
//	configure_pins( 0x008C, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	configure_pins( 0x0001, PIN_INPUT | PIN_PULLUP | PIN_PORTF ) ;
//   #else
//    #ifdef PCBXLITE
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	configure_pins( 0x7D80, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//    #else // PCBXLITE
//// Buttons PE10, 11, 12, PD2, 3, 7
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	configure_pins( 0x1C00, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//	configure_pins( 0x008C, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//// Extra inputs
//	configure_pins( 0x6000, PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
//    #endif // PCBXLITE
//   #endif // REV9E
//  #endif // PCBX7
// #endif // PCBX9LITE
//#endif // REV19
//}

//void init_trims()
//{
//// Trims 
//#ifdef PCBX9LITE
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	configure_pins( PIN_TRIMLV_DN | PIN_TRIMLV_UP, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
//	configure_pins( PIN_TRIMLH_UP | PIN_TRIMLH_DN, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//	configure_pins( PIN_TRIMRH_DN | PIN_TRIMRH_UP, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	configure_pins( PIN_TRIMRV_DN | PIN_TRIMRV_UP, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//#else
//#ifdef PCBX7
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	configure_pins( 0x0078, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//	configure_pins( 0x000E, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//	configure_pins( 0x8000, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//#else
//#ifdef PCBXLITE
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
//	configure_pins( 0x0003, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
//	configure_pins( 0x0030, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//#else // PCBXLITE
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
//#ifdef REV9E
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN ; 		// Enable portG clock
//	configure_pins( 0x0018, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//	configure_pins( 0x0003, PIN_INPUT | PIN_PULLUP | PIN_PORTG ) ;
//#else
//	configure_pins( 0x0078, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//#endif // REV9E
//	configure_pins( 0x200E, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//#endif // PCBXLITE
//#endif // PCBX7
//#endif // PCBX9LITE
//}

//// Reqd. bit 6 LEFT, 5 RIGHT, 4 UP, 3 DOWN 2 EXIT 1 MENU
//uint32_t read_keys()
//{
//	register uint32_t x ;
//	register uint32_t y ;

//#ifdef PCBXLITE
//	x = GPIOE->IDR ; // 10 RIGHT(+), 11 LEFT(-), 12 ENT(DOWN)
//	y = 0 ;
	
//	// Still to handle the enter button
//	if ( x & PIN_BUTTON_MENU )
//	{
//		y |= 0x02 << KEY_MENU ;			// MENU
//	}
//	if ( x & PIN_BUTTON_EXIT )
//	{
//		y |= 0x02 << KEY_EXIT ;			// EXIT
//	}
//	if ( x & PIN_BUTTON_LEFT )
//	{
//		y |= 0x02 << KEY_LEFT ;		// LEFT
//	}
//	if ( x & PIN_BUTTON_RIGHT )
//	{
//		y |= 0x02 << KEY_RIGHT ;		// RIGHT
//	}
//	if ( x & PIN_BUTTON_UP )
//	{
//		y |= 0x02 << KEY_UP ;			// up
//	}
//	if ( x & PIN_BUTTON_DOWN )
//	{
//		y |= 0x02 << KEY_DOWN ;		// DOWN
//	}
//#else // PCBXLITE
//	x = GPIOD->IDR ; // 7 MENU, 3 PAGE(UP), 2 EXIT
//	y = 0 ;

//#ifdef PCBX7
// #ifdef PCBT12
//	if ( x & PIN_BUTTON_EXIT )
//	{
//		y |= 0x02 << KEY_EXIT ;			// EXIT
//	}
//	if ( x & PIN_BUTTON_RIGHT )
//	{
//		y |= 0x02 << KEY_RIGHT ;		// RIGHT
//	}
//	if ( x & PIN_BUTTON_LEFT )
//	{
//		y |= 0x02 << KEY_LEFT ;		// RIGHT
//	}
//	x = GPIOE->IDR ;
//	if ( x & PIN_BUTTON_MENU )
//	{
//		y |= 0x02 << KEY_MENU ;			// MENU
//	}
//	if ( x & PIN_BUTTON_UP )
//	{
//		y |= 0x02 << KEY_UP ;			// up
//	}
//	if ( x & PIN_BUTTON_DOWN )
//	{
//		y |= 0x02 << KEY_DOWN ;		// DOWN
//	}
// #else
//	if ( x & PIN_BUTTON_MENU )
//	{
//		y |= 0x02 << KEY_MENU ;			// MENU
//	}
//	if ( x & PIN_BUTTON_PAGE )
//	{
//		y |= 0x02 << KEY_LEFT ;		// LEFT
//	}
//	if ( x & PIN_BUTTON_EXIT )
//	{
//		y |= 0x02 << KEY_EXIT ;			// EXIT
//	}
//	y |= 0x02 << KEY_RIGHT ;	// RIGHT
//	y |= 0x02 << KEY_UP ;			// up
//	y |= 0x02 << KEY_DOWN ;		// DOWN
// #endif
//#else
//#ifdef REV9E
//	if ( x & PIN_BUTTON_MENU )
//	{
//		y |= 0x02 << KEY_MENU ;			// MENU
//	}
//	if ( x & PIN_BUTTON_PAGE )
//	{
//		y |= 0x02 << KEY_LEFT ;		// LEFT
//	}
//	if ( x & PIN_BUTTON_EXIT )
//	{
//		y |= 0x02 << KEY_EXIT ;			// EXIT
//	}
////	x = GPIOF->IDR ; // 10 RIGHT(+), 11 LEFT(-), 12 ENT(DOWN)
////	if ( x & PIN_BUTTON_ENCODER )
////	{
////		y |= 0x02 << KEY_RIGHT ;		// RIGHT
////	}

//	y |= 0x02 << KEY_RIGHT ;	// RIGHT
//	y |= 0x02 << KEY_UP ;			// up
//	y |= 0x02 << KEY_DOWN ;		// DOWN

//#else
//#ifdef PCBX9LITE
//	x = GPIOE->IDR ; // 10 RIGHT(+), 11 LEFT(-), 12 ENT(DOWN)
//	y = 0 ;
	
//	if ( x & PIN_BUTTON_MENU )
//	{
//		y |= 0x02 << KEY_MENU ;			// MENU
//	}
//	if ( x & PIN_BUTTON_PAGE )
//	{
//		y |= 0x02 << KEY_LEFT ;		// LEFT
//	}
//	if ( x & PIN_BUTTON_EXIT )
//	{
//		y |= 0x02 << KEY_EXIT ;			// EXIT
//	}
//	y |= 0x02 << KEY_RIGHT ;	// RIGHT
//	y |= 0x02 << KEY_UP ;			// up
//	y |= 0x02 << KEY_DOWN ;		// DOWN
//#else // PCBX9LITE
//#ifdef REV19
//	x = GPIOD->IDR ; // 10 RIGHT(+), 11 LEFT(-), 12 ENT(DOWN)
//	y = 0 ;
	
//	if ( x & PIN_BUTTON_MENU )
//	{
//		y |= 0x02 << KEY_MENU ;			// MENU
//	}
//	if ( x & PIN_BUTTON_PAGE )
//	{
//		y |= 0x02 << KEY_LEFT ;		// LEFT
//	}
//	if ( x & PIN_BUTTON_EXIT )
//	{
//		y |= 0x02 << KEY_EXIT ;			// EXIT
//	}
//	y |= 0x02 << KEY_RIGHT ;	// RIGHT
//	y |= 0x02 << KEY_UP ;			// up
//	y |= 0x02 << KEY_DOWN ;		// DOWN
//#else
	
//	y = 0 ;
//	if ( x & PIN_BUTTON_MENU )
//	{
//		y |= 0x02 << KEY_UP ;			// up
////		y |= 0x02 << KEY_MENU ;			// MENU
//	}
//	if ( x & PIN_BUTTON_PAGE )
//	{
//		y |= 0x02 << KEY_LEFT ;		// LEFT
//	}
//	if ( x & PIN_BUTTON_EXIT )
//	{
//		y |= 0x02 << KEY_DOWN ;		// DOWN
////		y |= 0x02 << KEY_EXIT ;			// EXIT
//	}
	
//	x = GPIOE->IDR ; // 10 RIGHT(+), 11 LEFT(-), 12 ENT(DOWN)
//	if ( x & PIN_BUTTON_PLUS )
//	{
//		y |= 0x02 << KEY_MENU ;			// MENU
////		y |= 0x02 << KEY_UP ;			// up
//	}
//	if ( x & PIN_BUTTON_MINUS )
//	{
//		y |= 0x02 << KEY_RIGHT ;		// RIGHT
////		y |= 0x02 << KEY_DOWN ;		// DOWN
//	}
//	if ( x & PIN_BUTTON_ENTER )
//	{
//		y |= 0x02 << KEY_EXIT ;			// EXIT
////		y |= 0x02 << KEY_RIGHT ;		// RIGHT
//	}
//#endif // REV19
//#endif // PCBX9LITE
//#endif // REV9E
//#endif // PCBX7
//#endif // PCBXLITE
//	if ( ExternalSet )
//	{
//		y &= ~ExternalKeys ;
//	}
//	return y ;
//}

//uint32_t read_trims()
//{
//	uint32_t trims ;
//	uint32_t trima ;

//	trims = 0 ;

//#ifdef PCBX9LITE
//	trima = GPIOC->IDR ;
//// TRIM_LH_DOWN
//	if ( ( trima & PIN_TRIMLH_DN ) == 0 )
//	{
//		trims |= 1 ;
//	}
    
//// TRIM_LH_UP
//	if ( ( trima & PIN_TRIMLH_UP ) == 0 )
//	{
//		trims |= 2 ;
//	}

//	trima = GPIOE->IDR ;

//// TRIM_RV_UP
//	if ( ( trima & PIN_TRIMRV_UP ) == 0 )
//	{
//		trims |= 0x20 ;
//	}

//// TRIM_RH_DOWN
//	if ( ( trima & PIN_TRIMRV_DN ) == 0 )
//	{
//		trims |= 0x10 ;
//	}

//	trima = GPIOB->IDR ;

//// TRIM_LV_DOWN
//	if ( ( trima & PIN_TRIMLV_DN ) == 0 )
//	{
//		trims |= 4 ;
//	}

//// TRIM_LV_UP
//	if ( ( trima & PIN_TRIMLV_UP ) == 0 )
//	{
//		trims |= 8 ;
//	}

//	trima = GPIOD->IDR ;

//// TRIM_RH_UP
//	if ( ( trima & PIN_TRIMRH_UP ) == 0 )
//	{
//		trims |= 0x80 ;
//	}

//// TRIM_RH_DOWN
//	if ( ( trima & PIN_TRIMRH_DN ) == 0 )
//	{
//		trims |= 0x40 ;
//	}

//#else // X3
//#ifdef REV9E

//	trima = GPIOG->IDR ;
//// TRIM_LH_DOWN
//	if ( ( trima & PIN_TRIMLH_DN ) == 0 )
//	{
//		trims |= 1 ;
//	}
    
//// TRIM_LH_UP
//	if ( ( trima & PIN_TRIMLH_UP ) == 0 )
//	{
//		trims |= 2 ;
//	}

//	trima = GPIOC->IDR ;

//// TRIM_RV_UP
//	if ( ( trima & PIN_TRIMRV_UP ) == 0 )
//	{
//		trims |= 0x20 ;
//	}

//// TRIM_RH_DOWN
//	if ( ( trima & PIN_TRIMRH_DN ) == 0 )
//	{
//		trims |= 0x40 ;
//	}


//// TRIM_RV_DOWN
//	if ( ( trima & PIN_TRIMRV_DN ) == 0 )
//	{
//		trims |= 0x10 ;
//	}

//// TRIM_RH_UP
//	if ( ( trima & PIN_TRIMRH_UP ) == 0 )
//	{
//		trims |= 0x80 ;
//	}
	
//	trima = GPIOE->IDR ;

//// TRIM_LV_DOWN
//	if ( ( trima & PIN_TRIMLV_DN ) == 0 )
//	{
//		trims |= 4 ;
//	}

//// TRIM_LV_UP
//	if ( ( trima & PIN_TRIMLV_UP ) == 0 )
//	{
//		trims |= 8 ;
//	}

//#else
//#ifdef PCBX7
//	trima = GPIOE->IDR ;
//// TRIM_LV_DOWN
//	if ( ( trima & PIN_TRIMLV_DN ) == 0 )
//	{
//		trims |= 4 ;
//	}

//// TRIM_LV_UP
//	if ( ( trima & PIN_TRIMLV_UP ) == 0 )
//	{
//		trims |= 8 ;
//	}

//// TRIM_RH_DOWN
//	if ( ( trima & PIN_TRIMRH_DN ) == 0 )
//	{
//		trims |= 0x40 ;
//	}

//// TRIM_RH_UP
//	if ( ( trima & PIN_TRIMRH_UP ) == 0 )
//	{
//		trims |= 0x80 ;
//	}

//	trima = GPIOC->IDR ;

//// TRIM_LH_UP
//	if ( ( trima & PIN_TRIMLH_UP ) == 0 )
//	{
//		trims |= 2 ;
//	}

//// TRIM_RV_UP
//	if ( ( trima & PIN_TRIMRV_UP ) == 0 )
//	{
//		trims |= 0x20 ;
//	}


//// TRIM_RV_DOWN
//	if ( ( trima & PIN_TRIMRV_DN ) == 0 )
//	{
//		trims |= 0x10 ;
//	}

//	trima = GPIOD->IDR ;
//// TRIM_LH_DOWN
//	if ( ( trima & PIN_TRIMLH_DN ) == 0 )
//	{
//		trims |= 1 ;
//	}
 
//#else
//#ifdef PCBXLITE
//	trima = GPIOB->IDR ;
//	uint32_t shift = GPIOE->IDR & 0x0100 ;
//	if ( ( trima & PIN_TRIMLV_DN ) == 0 )
//	{
//		trims |= shift ? 0x10 : 4 ;
//	}

//// TRIM_LV_UP
//	if ( ( trima & PIN_TRIMLV_UP ) == 0 )
//	{
//		trims |= shift ? 0x20 : 8 ;
//	}
	
//	trima = GPIOC->IDR ;
//// TRIM_LH_DOWN
//	if ( ( trima & PIN_TRIMLH_DN ) == 0 )
//	{
//		trims |= shift ? 0x40 : 1 ;
//	}
    
//// TRIM_LH_UP
//	if ( ( trima & PIN_TRIMLH_UP ) == 0 )
//	{
//		trims |= shift ? 0x80 : 2 ;
//	}

//#else
//	trima = GPIOE->IDR ;
//// TRIM_LH_DOWN
//	if ( ( trima & PIN_TRIMLH_DN ) == 0 )
//	{
//		trims |= 1 ;
//	}
    
//// TRIM_LH_UP
//	if ( ( trima & PIN_TRIMLH_UP ) == 0 )
//	{
//		trims |= 2 ;
//	}

//// TRIM_LV_DOWN
//	if ( ( trima & PIN_TRIMLV_DN ) == 0 )
//	{
//		trims |= 4 ;
//	}

//// TRIM_LV_UP
//	if ( ( trima & PIN_TRIMLV_UP ) == 0 )
//	{
//		trims |= 8 ;
//	}

//	trima = GPIOC->IDR ;

//// TRIM_RV_UP
//	if ( ( trima & PIN_TRIMRV_UP ) == 0 )
//	{
//		trims |= 0x20 ;
//	}

//// TRIM_RH_DOWN
//	if ( ( trima & PIN_TRIMRH_DN ) == 0 )
//	{
//		trims |= 0x40 ;
//	}


//// TRIM_RV_DOWN
//	if ( ( trima & PIN_TRIMRV_DN ) == 0 )
//	{
//		trims |= 0x10 ;
//	}

//// TRIM_RH_UP
//	if ( ( trima & PIN_TRIMRH_UP ) == 0 )
//	{
//		trims |= 0x80 ;
//	}
//#endif // XLITE
//#endif // PCBX7
//#endif
//#endif // X3

//	return trims ;
//}

//#ifdef PCBX9D

//// 
//void setup_switches()
//{
//// For REV19:
//// PE2, PE7, PE8, PE9, PE13, PE14, PE15, PE0, PE1 | SWB_L, D_H, G_L, G_H, D_L, F, C_H, A_L, B_H
//// PD14 | H
//// PB3, PB4, PB5 | E_H, E_L, A_H
//// PA5  SWC_L
//#ifdef REV19
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portDclock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	configure_pins( PIN_SW_C_L, PIN_INPUT | PIN_PULLUP | PIN_PORTA ) ;
//	configure_pins( PIN_SW_E_H | PIN_SW_E_L | PIN_SW_A_H, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
//	configure_pins( PIN_SW_H, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	configure_pins( PIN_SW_B_L | PIN_SW_D_H | PIN_SW_G_L | PIN_SW_G_H | PIN_SW_D_L | PIN_SW_F | PIN_SW_C_H | PIN_SW_A_L | PIN_SW_B_H, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//#endif
	
//#ifdef PCBX9LITE
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	configure_pins( PIN_SW_H, PIN_INPUT | PIN_PULLUP | PIN_PORTA ) ;
//	configure_pins( PIN_SW_B_L | PIN_SW_B_H, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
//	configure_pins( PIN_SW_F, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//	configure_pins( PIN_SW_A_L | PIN_SW_A_H | PIN_SW_C_L | PIN_SW_C_H, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;

//	// Look to see if PD14 is in use
//	if ( ( g_eeGeneral.pb1source == 3 ) || ( g_eeGeneral.pb2source == 3 ) || ( g_eeGeneral.pb3source == 3 ) )
//	{
//		RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
//		configure_pins( 0x4000, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	}
//	if ( ( g_eeGeneral.pb1source == 4 ) || ( g_eeGeneral.pb2source == 4 ) || ( g_eeGeneral.pb3source == 4 ) )
//	{
//		configure_pins( 0x0004, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//	}
//	if ( ( g_eeGeneral.pb1source == 5 ) || ( g_eeGeneral.pb2source == 5 ) || ( g_eeGeneral.pb3source == 5 ) )
//	{
//		configure_pins( 0x0008, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//	}
//#else
//#ifdef PCBX7
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	configure_pins( 0x0020, PIN_INPUT | PIN_PULLUP | PIN_PORTA ) ;
//	configure_pins( PIN_SW_H | PIN_SW_C_L, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	configure_pins( 0xE087, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;

//	// Extra switch inputs
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
//	configure_pins( PIN_SW_EXT1, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//	configure_pins( PIN_SW_EXT2, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
	 
//#else // PCBX7
//#ifdef PCBXLITE
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//	configure_pins( 0x000F, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//	configure_pins( 0x0060, PIN_INPUT | PIN_PULLUP | PIN_PORTA ) ;
//	configure_pins( 0x0030, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
//#else // PCBXLITE
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
//#if defined(REVPLUS) || defined(REV9E)
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
//#endif
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portE clock
//#ifdef REV9E
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN ; 		// Enable port F clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN ; 		// Enable port G clock
//#endif
	
//#ifndef REV9E
//	configure_pins( 0x0020, PIN_INPUT | PIN_PULLUP | PIN_PORTA ) ;
// #ifdef REVPLUS
//	configure_pins( 0x0038, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
// #else
//	configure_pins( 0x003A, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
// #endif
//	configure_pins( 0xE387, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
// #ifdef REVPLUS
//	configure_pins( PIN_SW_H, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
// #endif
//#endif // PCBXLITE
//#endif
//#endif // X3


//#ifdef REV9E
//	configure_pins( GPIO_Pin_5, PIN_INPUT | PIN_PULLUP | PIN_PORTA ) ;
//	configure_pins( PIN_SW_N_L | PIN_SW_N_H | PIN_SW_R_H,
//									PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
//	configure_pins( PIN_SW_B_H | PIN_SW_B_L,  PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	configure_pins( PIN_SW_A_L | PIN_SW_A_H | PIN_SW_D_L | PIN_SW_F_L | PIN_SW_F_H | PIN_SW_I_L
//									| PIN_SW_L_L | PIN_SW_L_H | PIN_SW_M_H | PIN_SW_O_L | PIN_SW_R_L,
//									PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//	configure_pins( PIN_SW_E_L | PIN_SW_E_H | PIN_SW_G_L | PIN_SW_G_H | PIN_SW_K_L | PIN_SW_K_H 
//									| PIN_SW_O_H | PIN_SW_P_L | PIN_SW_P_H | PIN_SW_Q_L | PIN_SW_Q_H | PIN_SW_I_H | PIN_SW_H_L,
//									PIN_INPUT | PIN_PULLUP | PIN_PORTF ) ;
//	configure_pins( PIN_SW_C_L | PIN_SW_C_H | PIN_SW_J_L | PIN_SW_J_H,
//									PIN_INPUT | PIN_PULLUP | PIN_PORTG) ;
//#endif	// REV9E
//#endif // PCBX7
//}

//uint32_t readKeyUpgradeBit( uint8_t index )
//{
//  CPU_UINT xxx = 0 ;
	
//#ifdef PCBX7
//	if ( index > 2 )	// Extra inputs
//	{
//		if ( index == 3 )
//		{
//			return ~GPIOC->IDR & PIN_SW_EXT1 ;
//		}
//		else
//		{
//			return ~GPIOD->IDR & PIN_SW_EXT2 ;
//		}
//	}
//#endif
//#ifdef PCBX9LITE
//	if ( index > 2 )	// Extra inputs
//	{
//		if ( index == 3 )
//		{
//			return ~GPIOD->IDR & 0x4000 ;
//		}
//		if ( index == 4 )
//		{
//			return ~GPIOC->IDR & 0x0004 ;
//		}
//		if ( index == 5 )
//		{
//			return ~GPIOC->IDR & 0x0008 ;
//		}
//	}
//#endif
//	uint32_t t = 1 << (index+12) ;
//	xxx = ~GPIOA->IDR & t ;
//	return xxx ;
//}


//uint32_t hwKeyState( uint8_t key )
//{
//#ifdef PCBX9LITE
//  uint32_t xxx = 0 ;
//  uint32_t e = GPIOE->IDR ;
//  uint32_t b = GPIOB->IDR ;
//  uint32_t c = GPIOC->IDR ;
//  uint32_t a = GPIOA->IDR ;

//	if( key > HSW_MAX )  return 0 ;

//	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )
//	{
//		uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
//		key -= HSW_Etrmdn ;		// 0 - 7
//		if (key >= 2 && key <= 5)       // swap back LH/RH trims
//		{
//			if ( g_eeGeneral.stickMode & 2 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		else
//		{
//			if ( g_eeGeneral.stickMode & 1 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		if ( ct )
//		{
//			key ^= 0x06 ;
//	 		if ( ct == 2 ) // Vintage style crosstrim
//			{
//	 			if (key >= 2 && key <= 5)       // swap back LH/RH trims
//				{
//					key ^= 0x06 ;
//				}
//			}
//		}
//		key += HSW_Etrmdn ;		// 0 - 7
//	}

//  switch ( key )
//	{
//    case HSW_SA0:
//      xxx = ~e & PIN_SW_A_L ;
//    break ;
//    case HSW_SA1:
//      xxx = ((e & PIN_SW_A_L) | (e & PIN_SW_A_H)) == (PIN_SW_A_L | PIN_SW_A_H) ;
//    break ;
//    case HSW_SA2:
//      xxx = ~e & PIN_SW_A_H ;
//    break ;

//    case HSW_SB0:
//      xxx = ~b & PIN_SW_B_L ;
//    break ;
//    case HSW_SB1:
//      xxx = ((b & PIN_SW_B_L) | (b & PIN_SW_B_H)) == (PIN_SW_B_L | PIN_SW_B_H) ;
//    break;
//    case HSW_SB2:
//      xxx = ~b & PIN_SW_B_H ;
//    break ;

//    case HSW_SC0:
//      xxx = ~e & PIN_SW_C_L ;
//    break;
//    case HSW_SC1:
//      xxx = ((e & PIN_SW_C_L) | (e & PIN_SW_C_H)) == (PIN_SW_C_L | PIN_SW_C_H) ;
//    break;
//    case HSW_SC2:
//      xxx = ~e & PIN_SW_C_H ;
//    break;
		
//		case HSW_SF2:
//      xxx = ~c & PIN_SW_F ;
//    break ;

//    case HSW_SH2:
//      xxx = ~a & PIN_SW_H ;
//    break ;
	
//		case HSW_Ttrmup :
//			xxx = keyState( (EnumKeys) TRM_RV_DWN ) ;
//    break ;
		
//		case HSW_Ttrmdn :
//			xxx = keyState( (EnumKeys) TRM_RV_UP ) ;
//    break ;
		
//		case HSW_Rtrmup :
//			xxx = keyState( (EnumKeys) TRM_LH_DWN ) ;
//    break ;
		
//		case HSW_Rtrmdn :
//			xxx = keyState( (EnumKeys) TRM_LH_UP ) ;
//    break ;
		
//		case HSW_Atrmup :
//			xxx = keyState( (EnumKeys) TRM_RH_DWN ) ;
//    break ;
		
//		case HSW_Atrmdn :
//			xxx = keyState( (EnumKeys) TRM_RH_UP ) ;
//    break ;
		
//		case HSW_Etrmup :
//			xxx = keyState( (EnumKeys) TRM_LV_DWN ) ;
//    break ;
		
//		case HSW_Etrmdn :
//			xxx = keyState( (EnumKeys) TRM_LV_UP ) ;
//    break ;
		
//		case HSW_Pb1 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb1source ) ;
//    break ;
			 
//		case HSW_Pb2 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb2source ) ;
//    break ;
		
//		case HSW_Pb3 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb3source ) ;
//    break ;

//	}

//#else // PCBX9LITE
//#ifdef PCBXLITE
//  uint32_t xxx = 0 ;
//  uint32_t e = GPIOE->IDR ;
//  uint32_t a = GPIOA->IDR ;
  
//  switch ( key )
//	{
//    case HSW_SA0:
//      xxx = ~e & PIN_SW_A_L ;
//    break ;
//    case HSW_SA1:
//      xxx = ((e & PIN_SW_A_L) | (e & PIN_SW_A_H)) == (PIN_SW_A_L | PIN_SW_A_H) ;
//    break ;
//    case HSW_SA2:
//      xxx = ~e & PIN_SW_A_H ;
//    break ;

//    case HSW_SB0:
//      xxx = ~a & PIN_SW_B_L ;
//    break ;
//    case HSW_SB1:
//      xxx = ((a & PIN_SW_B_L) | (a & PIN_SW_B_H)) == (PIN_SW_B_L | PIN_SW_B_H) ;
//    break;
//    case HSW_SB2:
//      xxx = ~a & PIN_SW_B_H ;
//    break ;

//    case HSW_SC0:
//      xxx = ~e & PIN_SW_C_L ;
//    break;
//    case HSW_SC1:
//      xxx = ((e & PIN_SW_C_L) | (e & PIN_SW_C_H)) == (PIN_SW_C_L | PIN_SW_C_H) ;
//    break;
//    case HSW_SC2:
//			if (g_eeGeneral.ailsource)
//			{
//				xxx = ~e & PIN_SW_C_H ;
//			}
//			else
//			{
//      	xxx = e & PIN_SW_C_L ;
//			}
//    break;

//    case HSW_SD0:
//      xxx = ~GPIOB->IDR & PIN_SW_D_L ;
//		break;
//    case HSW_SD1:
//      xxx = ((GPIOB->IDR & PIN_SW_D_L) | (GPIOB->IDR & PIN_SW_D_H)) == (PIN_SW_D_L | PIN_SW_D_H) ;
//    break;
//    case HSW_SD2:
//			if (g_eeGeneral.rudsource)
//			{
//      	xxx = ~GPIOB->IDR & PIN_SW_D_H ;
//			}
//			else
//			{
//      	xxx = GPIOB->IDR & PIN_SW_D_L ;
//			}
//    break;
    
//		case HSW_SF2:
//      xxx = ~e & PIN_SW_C_L ;
//    break ;

//    case HSW_SH2:
//      xxx = GPIOB->IDR & PIN_SW_D_L ;
//    break ;

//	}
//#else // PCBXLITE
	
//#ifdef PCBX7
//  register uint32_t a = GPIOA->IDR;
//  register uint32_t d = GPIOD->IDR;
//  register uint32_t e = GPIOE->IDR;
//#else // PCBX7
//  register uint32_t a = GPIOA->IDR;
//  register uint32_t b = GPIOB->IDR;
//  register uint32_t e = GPIOE->IDR;
//#ifdef REV9E
//  register uint32_t f = GPIOF->IDR;
//  register uint32_t g = GPIOG->IDR;
//  register uint32_t d = GPIOD->IDR;
//#endif	// REV9E
//#endif // PCBX7

//  uint32_t xxx = 0 ;
//  uint32_t analog = 0 ;
	
//	if ( g_eeGeneral.analogMapping & MASK_6POS )
//	{
//  	analog = ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2 ;
//		analog += 3 ;
//		if ( analog > 5 )
//		{
//			analog = 9 ;
//		}
//		analog = Analog_values[analog] ;
//		if ( g_eeGeneral.SixPositionCalibration[5] >  g_eeGeneral.SixPositionCalibration[0] )
//		{
//			analog = 4095 - analog ;
//		}
//	}
  
//	if( key > HSW_MAX )  return 0 ;

//	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )
//	{
//		uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
//		key -= HSW_Etrmdn ;		// 0 - 7
//		if (key >= 2 && key <= 5)       // swap back LH/RH trims
//		{
//			if ( g_eeGeneral.stickMode & 2 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		else
//		{
//			if ( g_eeGeneral.stickMode & 1 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		if ( ct )
//		{
//			key ^= 0x06 ;
//	 		if ( ct == 2 ) // Vintage style crosstrim
//			{
//	 			if (key >= 2 && key <= 5)       // swap back LH/RH trims
//				{
//					key ^= 0x06 ;
//				}
//			}
//		}
//		key += HSW_Etrmdn ;		// 0 - 7
//	}
////	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )
////	{
////		uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
////		if ( ct )
////		{
////			key -= HSW_Etrmdn ;		// 0 - 7
////			key ^= 0x06 ;
////	 		if ( ct == 2 ) // Vintage style crosstrim
//// 			{
////	 			if (key >= 2 && key <= 5)       // swap back LH/RH trims
////				{
////					key ^= 0x06 ;
////				}
//// 			}
////			key += HSW_Etrmdn ;		// 0 - 7
////		}
////	}

//  switch ( key )
//	{
//		case HSW_Ttrmup :
//			xxx = keyState( (EnumKeys) TRM_RV_DWN ) ;
//    break ;
		
//		case HSW_Ttrmdn :
//			xxx = keyState( (EnumKeys) TRM_RV_UP ) ;
//    break ;
		
//		case HSW_Rtrmup :
//			xxx = keyState( (EnumKeys) TRM_LH_DWN ) ;
//    break ;
		
//		case HSW_Rtrmdn :
//			xxx = keyState( (EnumKeys) TRM_LH_UP ) ;
//    break ;
		
//		case HSW_Atrmup :
//			xxx = keyState( (EnumKeys) TRM_RH_DWN ) ;
//    break ;
		
//		case HSW_Atrmdn :
//			xxx = keyState( (EnumKeys) TRM_RH_UP ) ;
//    break ;
		
//		case HSW_Etrmup :
//			xxx = keyState( (EnumKeys) TRM_LV_DWN ) ;
//    break ;
		
//		case HSW_Etrmdn :
//			xxx = keyState( (EnumKeys) TRM_LV_UP ) ;
//    break ;
		

//#ifdef PCBX7
//    case HSW_SA0:
//      xxx = ~e & PIN_SW_A_L;
//      break;
//    case HSW_SA1:
//      xxx = ((e & PIN_SW_A_L) | (e & PIN_SW_A_H)) == (PIN_SW_A_L | PIN_SW_A_H) ;
//      break;
//    case HSW_SA2:
//      xxx = ~e & PIN_SW_A_H;
//      break;

//    case HSW_SB0:
//      xxx = ~e & PIN_SW_B_L ;
//      break;
//    case HSW_SB1:
//      xxx = ((e & PIN_SW_B_L) | (a & PIN_SW_B_H)) == (PIN_SW_B_L | PIN_SW_B_H) ;
//      break;
//    case HSW_SB2:
//      xxx = ~a & PIN_SW_B_H ;
//      break;

//    case HSW_SC0:
//      xxx = ~d & PIN_SW_C_L ;
//      break;
//    case HSW_SC1:
//      xxx = ((d & PIN_SW_C_L) | (e & PIN_SW_C_H)) == (PIN_SW_C_L | PIN_SW_C_H) ;
//      break;
//    case HSW_SC2:
//      xxx = ~e & PIN_SW_C_H ;
//      break;

//    case HSW_SD0:
//      xxx = ~e & PIN_SW_D_L ;
//			break;
//    case HSW_SD1:
//      xxx = ((e & PIN_SW_D_L) | (e & PIN_SW_D_H)) == (PIN_SW_D_L | PIN_SW_D_H) ;
//      break;
//    case HSW_SD2:
//      xxx = ~e & PIN_SW_D_H ;
//      break;

//    case HSW_SF2:
//      xxx = ~e & PIN_SW_F ;
//      break;

//    case HSW_SH2:
//      xxx = ~d & PIN_SW_H;
//      break;

//    case HSW_SE0:
//      xxx = 1 ;
//      break;
////    case HSW_SE1:
////      xxx = 0 ;
////      break;
////    case HSW_SE2:
////      xxx = ~b & PIN_SW_E_H ;
////      break;

//    case HSW_SG0:
//      xxx = 1 ;
//      break;
////    case HSW_SG1:
////      xxx = (f & (PIN_SW_E_H | PIN_SW_E_L)) == (PIN_SW_E_H | PIN_SW_E_L) ;
////      break;
////    case HSW_SG2:
////      xxx = ~f & PIN_SW_E_H ;
////      break;

//#else // PCBX7
		
//#ifndef REV9E
//    case HSW_SA0:
//      xxx = ~e & PIN_SW_A_L;
//      break;
//    case HSW_SA1:
//      xxx = ((e & PIN_SW_A_L) | (b & PIN_SW_A_H)) == (PIN_SW_A_L | PIN_SW_A_H) ;
//      break;
//    case HSW_SA2:
//      xxx = ~b & PIN_SW_A_H;
//      break;

//    case HSW_SB0:
//      xxx = ~e & PIN_SW_B_L ;
//      break;
//    case HSW_SB1:
//      xxx = (e & (PIN_SW_B_L | PIN_SW_B_H)) == (PIN_SW_B_L | PIN_SW_B_H) ;
//      break;
//    case HSW_SB2:
//      xxx = ~e & PIN_SW_B_H ;
//      break;

//    case HSW_SC0:
//      xxx = ~a & PIN_SW_C_L ;
//      break;
//    case HSW_SC1:
//      xxx = ((a & PIN_SW_C_L) | (e & PIN_SW_C_H)) == (PIN_SW_C_L | PIN_SW_C_H) ;
//      break;
//    case HSW_SC2:
//      xxx = ~e & PIN_SW_C_H ;
//      break;

//#ifdef PCBX9LITE
//    case HSW_SD0:
//    case HSW_SE0:
//    case HSW_SG0:
//      xxx = 1 ;
//    break ;
//#else
//    case HSW_SD0:
//#if defined(REVPLUS) || defined(REV9E)
//      xxx = ~e & PIN_SW_D_L ;
//#else
//      xxx = ~b & PIN_SW_D_L ;
//#endif
//			break;
//    case HSW_SD1:
//#if defined(REVPLUS) || defined(REV9E)
//      xxx = ((e & PIN_SW_D_L) | (e & PIN_SW_D_H)) == (PIN_SW_D_L | PIN_SW_D_H) ;
//#else
//      xxx = ((b & PIN_SW_D_L) | (e & PIN_SW_D_H)) == (PIN_SW_D_L | PIN_SW_D_H) ;
//#endif
//      break;
//    case HSW_SD2:
//      xxx = ~e & PIN_SW_D_H ;
//      break;
//#endif

//#ifndef PCBX9LITE
//    case HSW_SE0:
//      xxx = ~b & PIN_SW_E_L ;
//      break;
//    case HSW_SE1:
//      xxx = ((b & PIN_SW_E_H) | (b & PIN_SW_E_L)) == (PIN_SW_E_H | PIN_SW_E_L) ;
//      break;
//    case HSW_SE2:
//      xxx = ~b & PIN_SW_E_H ;
//      break;
//#endif

////    case HSW_SF0:
////      xxx = e & PIN_SW_F ;
////      break;
//    case HSW_SF2:
//      xxx = ~e & PIN_SW_F ;
//      break;

//#ifndef PCBX9LITE
//    case HSW_SG0:
//      xxx = ~e & PIN_SW_G_L ;
//      break;
//    case HSW_SG1:
//      xxx = (e & (PIN_SW_G_H | PIN_SW_G_L)) == (PIN_SW_G_H | PIN_SW_G_L) ;
//      break;
//    case HSW_SG2:
//      xxx = ~e & PIN_SW_G_H ;
//      break;
//#endif

////    case HSW_SH0:
////#ifdef REVPLUS
////      xxx = GPIOD->IDR & PIN_SW_H;
////#else
////      xxx = e & PIN_SW_H;
////#endif
////      break;
//    case HSW_SH2:
//#if defined(REVPLUS) || defined(REV9E)
//      xxx = ~GPIOD->IDR & PIN_SW_H;
//#else
//      xxx = ~e & PIN_SW_H;
//#endif
//      break;
//#endif // nREV9E
//#endif // PCBX7

//#ifdef REV9E
    
//    case HSW_SA0:
//      xxx = ~d & PIN_SW_B_L;
//      break;
//    case HSW_SA1:
//      xxx = ((d & PIN_SW_B_L) | (d & PIN_SW_B_H)) == (PIN_SW_B_L | PIN_SW_B_H) ;
//      break;
//    case HSW_SA2:
//      xxx = ~d & PIN_SW_B_H ;
//      break;

//    case HSW_SB0:
//      xxx = ~g & PIN_SW_C_L ;
//      break;
//    case HSW_SB1:
//      xxx = (g & (PIN_SW_C_L | PIN_SW_C_H)) == (PIN_SW_C_L | PIN_SW_C_H) ;
//      break;
//    case HSW_SB2:
//      xxx = ~g & PIN_SW_C_H ;
//      break;

//    case HSW_SC0:
//      xxx = ~f & PIN_SW_G_L ;
//      break;
//    case HSW_SC1:
//      xxx = ((f & PIN_SW_G_L) | (f & PIN_SW_G_H)) == (PIN_SW_G_L | PIN_SW_G_H) ;
//      break;
//    case HSW_SC2:
//      xxx = ~f & PIN_SW_G_H ;
//      break;

//    case HSW_SD0:
//      xxx = ~e & PIN_SW_F_L ;
//			break;
//		case HSW_SD1:
//      xxx = ((e & PIN_SW_F_L) | (e & PIN_SW_F_H)) == (PIN_SW_F_L | PIN_SW_F_H) ;
//      break;
//    case HSW_SD2:
//      xxx = ~d & PIN_SW_F_L ;
//      break;
    
//    case HSW_SE0:
//      xxx = ~e & PIN_SW_A_L ;
//      break;
//    case HSW_SE1:
//      xxx = ((e & PIN_SW_A_H) | (e & PIN_SW_A_L)) == (PIN_SW_A_H | PIN_SW_A_L) ;
//      break;
//    case HSW_SE2:
//      xxx = ~e & PIN_SW_A_H ;
//      break;

////		case HSW_SF0:
////      xxx = ~e & PIN_SW_D_L ;
////      break;
////    case HSW_SF1:
////      xxx = ((e & PIN_SW_F_H) | (e & PIN_SW_F_L)) == (PIN_SW_F_H | PIN_SW_F_L) ;
////      break;
////    case HSW_SF2:
////      xxx = ~e & PIN_SW_F_H ;
////      break;

//    case HSW_SF2:
//      xxx = e & PIN_SW_D_L ;
//      break;

//    case HSW_SG0:
//      xxx = ~f & PIN_SW_E_L ;
//      break;
//    case HSW_SG1:
//      xxx = (f & (PIN_SW_E_H | PIN_SW_E_L)) == (PIN_SW_E_H | PIN_SW_E_L) ;
//      break;
//    case HSW_SG2:
//      xxx = ~f & PIN_SW_E_H ;
//      break;

//    case HSW_SH2:
//      xxx = f & PIN_SW_H_L;
//      break;

//		case HSW_SI0:
//      xxx = ~e & PIN_SW_I_L ;
//      break;
//    case HSW_SI1:
//      xxx = ((f & PIN_SW_I_H) | (e & PIN_SW_I_L)) == (PIN_SW_I_H | PIN_SW_I_L) ;
//      break;
//    case HSW_SI2:
//      xxx = ~f & PIN_SW_I_H ;
//      break;

//    case HSW_SJ0:
//      xxx = ~g & PIN_SW_J_L ;
//      break;
//    case HSW_SJ1:
//      xxx = (g & (PIN_SW_J_H | PIN_SW_J_L)) == (PIN_SW_J_H | PIN_SW_J_L) ;
//      break;
//    case HSW_SJ2:
//      xxx = ~g & PIN_SW_J_H ;
//      break;
    
//		case HSW_SK0:
//      xxx = ~f & PIN_SW_K_L ;
//      break;
//    case HSW_SK1:
//      xxx = (f & (PIN_SW_K_H | PIN_SW_K_L)) == (PIN_SW_K_H | PIN_SW_K_L) ;
//      break;
//    case HSW_SK2:
//      xxx = ~f & PIN_SW_K_H ;
//      break;

//    case HSW_SL0:
//      xxx = ~e & PIN_SW_L_L ;
//      break;
//    case HSW_SL1:
//      xxx = ( (e & PIN_SW_L_H ) | (e & PIN_SW_L_L) ) == (PIN_SW_L_H | PIN_SW_L_L) ;
//      break;
//    case HSW_SL2:
//      xxx = ~e & PIN_SW_L_H ;
//      break;

//		case HSW_SM0:
//      xxx = ~a & PIN_SW_M_L ;
//      break;
//    case HSW_SM1:
//      xxx = ((e & PIN_SW_M_H) | (a & PIN_SW_M_L)) == (PIN_SW_M_H | PIN_SW_M_L) ;
//      break;
//    case HSW_SM2:
//      xxx = ~e & PIN_SW_M_H ;
//      break;

//		case HSW_SN0:
//      xxx = ~b & PIN_SW_N_L ;
//      break;
//    case HSW_SN1:
//      xxx = (b & (PIN_SW_N_H | PIN_SW_N_L)) == (PIN_SW_N_H | PIN_SW_N_L) ;
//      break;
//    case HSW_SN2:
//      xxx = ~b & PIN_SW_N_H ;
//      break;

//		case HSW_SO0:
//      xxx = ~e & PIN_SW_O_L ;
//      break;
//    case HSW_SO1:
//      xxx = ((f & PIN_SW_O_H) | (e & PIN_SW_O_L)) == (PIN_SW_O_H | PIN_SW_O_L) ;
//      break;
//    case HSW_SO2:
//      xxx = ~f & PIN_SW_O_H ;
//      break;

//		case HSW_SP0:
//      xxx = ~f & PIN_SW_P_L ;
//      break;
//    case HSW_SP1:
//      xxx = (f & (PIN_SW_P_H | PIN_SW_P_L)) == (PIN_SW_P_H | PIN_SW_P_L) ;
//      break;
//    case HSW_SP2:
//      xxx = ~f & PIN_SW_P_H ;
//      break;
    
//		case HSW_SQ0:
//      xxx = ~f & PIN_SW_Q_L ;
//      break;
//    case HSW_SQ1:
//      xxx = (f & (PIN_SW_Q_H | PIN_SW_Q_L)) == (PIN_SW_Q_H | PIN_SW_Q_L) ;
//      break;
//    case HSW_SQ2:
//      xxx = ~f & PIN_SW_Q_H ;
//      break;

//		case HSW_SR0:
//      xxx = ~e & PIN_SW_R_L ;
//      break;
//    case HSW_SR1:
//      xxx = ((b & PIN_SW_R_H) | (e & PIN_SW_R_L)) == (PIN_SW_R_H | PIN_SW_R_L) ;
//      break;
//    case HSW_SR2:
//      xxx = ~b & PIN_SW_R_H ;
//      break;

//#endif	// REV9E

//		case HSW_Ele6pos0 :
//				xxx = analog > SixPositionTable[0] ;
////				xxx = analog < 417 ;
//    break ;
			
//		case HSW_Ele6pos1 :
//				xxx = ( analog <= SixPositionTable[0] ) && ( analog >= SixPositionTable[1] ) ;
////				xxx = ( analog <= 938 ) && ( analog >= 417 ) ;
//    break ;
			
//		case HSW_Ele6pos2 :
//				xxx = ( analog <= SixPositionTable[1] ) && ( analog >= SixPositionTable[2] ) ;
////				xxx = ( analog <= 1211 ) && ( analog >= 938 ) ;
//    break ;
			
//		case HSW_Ele6pos3 :
//				xxx = ( analog <= SixPositionTable[2] ) && ( analog >= SixPositionTable[3] ) ;
////				xxx = ( analog <= 1721 ) && ( analog >= 1211 ) ;
//    break ;
			
//		case HSW_Ele6pos4 :
//				xxx = ( analog <= SixPositionTable[3] ) && ( analog >= SixPositionTable[4] ) ;
////				xxx = ( analog <= 3050 ) && (+ analog >= 1721 ) ;
//    break ;
			
//		case HSW_Ele6pos5 :
//				xxx = analog < SixPositionTable[4] ;
//	//			xxx = analog > 3050 ;
//    break ;

//#ifndef REV9E
//		case HSW_Pb1 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb1source ) ;
//    break ;
			 
//		case HSW_Pb2 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb2source ) ;
//    break ;
//#ifdef PCBX9LITE
//		case HSW_Pb3 :
//			xxx = readKeyUpgradeBit( g_eeGeneral.pb3source ) ;
//    break ;
//#endif
//#endif

//    default:
//      break;
//  }

//#endif // XLITE
//#endif // PCBX9LITE

//  if ( xxx )
//  {
//    return 1 ;
//  }
//  return 0;

//}

//uint32_t keyState(EnumKeys enuk)
//{

//  if (enuk < (int) DIM(keys)) return keys[enuk].state() ? 1 : 0;

//	return hwKeyState( (uint8_t) enuk -SW_BASE + 1 ) ;
//}

//// Returns 0, 1 or 2 for ^ - or v
//#ifdef REV9E
//static const uint8_t SwitchIndices[] = {HSW_SA0,HSW_SB0,HSW_SC0,HSW_SD0,HSW_SE0,HSW_SF2,HSW_SG0,HSW_SH2,HSW_SI0,
//																 HSW_SJ0, HSW_SK0, HSW_SL0, HSW_SM0, HSW_SN0, HSW_SO0, HSW_SP0, HSW_SQ0, HSW_SR0 } ;
//#else
//static const uint8_t SwitchIndices[] = {HSW_SA0,HSW_SB0,HSW_SC0,HSW_SD0,HSW_SE0,HSW_SF2,HSW_SG0,HSW_SH2} ;
//#endif	// REV9E
//uint32_t switchPosition( uint32_t swtch )
//{
//#ifdef PCBXLITE
//	if ( swtch < 4 )
//#else
//	if ( swtch < sizeof(SwitchIndices) )
//#endif
//	{
//		swtch = SwitchIndices[swtch] ;
//	}
//	else
//	{
//		if ( ( swtch < HSW_Ele6pos0 ) && ( swtch > HSW_Ele6pos5 ) )
//		{
//			return 0 ;
//		}
//		else
//		{
//			if ( ( g_eeGeneral.analogMapping & MASK_6POS ) == 0 )
//			{
//				return 0 ;
//			}
//		}
//	}

//	if ( swtch == HSW_SF2 )
//	{
//		if ( hwKeyState( swtch ) )
//		{
//			return 2 ;
//		}
//		return 0 ;
//	} 
//	if ( swtch == HSW_SH2 )
//	{
//		if ( hwKeyState( swtch ) )
//		{
//			return 2 ;
//		}
//		return 0 ;
//	} 
//	if ( hwKeyState( swtch ) )
//	{
//		return 0 ;
//	}
//	swtch += 1 ;
//	if ( hwKeyState( swtch ) )
//	{
//		return 1 ;			
//	}
//	if ( swtch == HSW_Ele6pos1 )
//	{
//		if ( hwKeyState( HSW_Ele6pos3 ) )
//		{
//			return 3 ;
//		}
//		if ( hwKeyState( HSW_Ele6pos4 ) )
//		{
//			return 4 ;
//		}
//		if ( hwKeyState( HSW_Ele6pos5 ) )
//		{
//			return 5 ;
//		}
//	}
//	return 2 ;
	
////	swtch *= 3 ;
////	swtch += SW_SA0 ;
////	if ( swtch > SW_SF0 )
////	{
////		swtch -= 1 ;		
////	}
////	if ( keyState( (EnumKeys)swtch ) )
////	{
////		return 0 ;
////	}
////	swtch += 1 ;
////	if ( keyState( (EnumKeys)swtch ) )
////	{
////		if ( ( swtch != SW_SF2 ) && ( swtch != SW_SH2 ) )
////		{
////			return 1 ;			
////		}
////	}
////	return 2 ;
//}



//#endif
//#endif


//#if defined(PCBX12D) || defined(PCBX10)

//void setup_switches()
//{
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN ; 		// Enable portA clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOJEN ; 		// Enable portA clock
//	configure_pins( SWITCHES_GPIO_PIN_B_L | SWITCHES_GPIO_PIN_C_L, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
//	configure_pins( SWITCHES_GPIO_PIN_C_H, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	configure_pins( SWITCHES_GPIO_PIN_E_L, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
//	configure_pins( SWITCHES_GPIO_PIN_D_L | SWITCHES_GPIO_PIN_G_L | SWITCHES_GPIO_PIN_G_H | SWITCHES_GPIO_PIN_H, PIN_INPUT | PIN_PULLUP | PIN_PORTG ) ;
//	configure_pins( SWITCHES_GPIO_PIN_F | SWITCHES_GPIO_PIN_E_H | SWITCHES_GPIO_PIN_A_H | SWITCHES_GPIO_PIN_B_H, PIN_INPUT | PIN_PULLUP | PIN_PORTH ) ;
//	configure_pins( SWITCHES_GPIO_PIN_A_L, PIN_INPUT | PIN_PULLUP | PIN_PORTI ) ;
//	configure_pins( SWITCHES_GPIO_PIN_D_H, PIN_INPUT | PIN_PULLUP | PIN_PORTJ ) ;
//}

//void init_keys()
//{
//#ifdef PCBX10
//	configure_pins( KEYS_GPIO_PIN_MENU | KEYS_GPIO_PIN_UP | KEYS_GPIO_PIN_EXIT | KEYS_GPIO_PIN_PGDN | KEYS_GPIO_PIN_DOWN, PIN_INPUT | PIN_PULLUP | PIN_PORTI ) ;
//#endif
//#ifdef PCBX12D
//	configure_pins( KEYS_GPIO_PIN_MENU | KEYS_GPIO_PIN_RIGHT, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//	configure_pins( KEYS_GPIO_PIN_EXIT | KEYS_GPIO_PIN_LEFT | KEYS_GPIO_PIN_DOWN, PIN_INPUT | PIN_PULLUP | PIN_PORTI ) ;
//	configure_pins( KEYS_GPIO_PIN_UP, PIN_INPUT | PIN_PULLUP | PIN_PORTG ) ;
//	AnalogData[11] = AnalogData[13] = 0x0800 ;
//#endif
//}

//// Reqd. bit 6 LEFT, 5 RIGHT, 4 UP, 3 DOWN 2 EXIT 1 MENU
//uint32_t read_keys()
//{
//	register uint32_t x ;
//	register uint32_t y ;

//	y = 0 ;
//#ifdef PCBX12D
//	x = GPIOC->IDR ;
//	if ( x & KEYS_GPIO_PIN_MENU )
//	{
//		y |= 0x02 << KEY_MENU ;			// MENU
//	}
//	if ( ( x & KEYS_GPIO_PIN_RIGHT ) && (AnalogData[13] < 0xA80 ) )
//	{
//		y |= 0x02 << KEY_RIGHT ;	// RIGHT
//	}
//	x = GPIOI->IDR ;
//	if ( ( x & KEYS_GPIO_PIN_LEFT ) && (AnalogData[13] > 0x580 ) )
//	{
//		y |= 0x02 << KEY_LEFT ;		// LEFT
//	}
//	if ( x & KEYS_GPIO_PIN_EXIT )
//	{
//		y |= 0x02 << KEY_EXIT ;			// EXIT
//	}
//	if ( ( x & KEYS_GPIO_PIN_DOWN ) && (AnalogData[11] > 0x580 ) )
//	{
//		y |= 0x02 << KEY_DOWN ;		// DOWN
//	}
//	x = GPIOG->IDR ;
//	if ( ( x & KEYS_GPIO_PIN_UP ) && (AnalogData[11] < 0xA80 ) )
//	{
//		y |= 0x02 << KEY_UP ;			// up
//	}
//#endif	
//#ifdef PCBX10
//	x = GPIOI->IDR ;
//	if ( x & KEYS_GPIO_PIN_MENU )
//	{
//		y |= 0x02 << KEY_MENU ;			// MENU
//	}
//	if ( x & KEYS_GPIO_PIN_EXIT )
//	{
//		y |= 0x02 << KEY_EXIT ;			// EXIT
//	}
//	if ( x & KEYS_GPIO_PIN_DOWN )
//	{
//		y |= 0x02 << KEY_DOWN ;		// DOWN
//	}
//	if ( x & KEYS_GPIO_PIN_UP )
//	{
//		y |= 0x02 << KEY_UP ;			// up
//	}
//	if ( x & KEYS_GPIO_PIN_PGDN )
//	{
//		y |= 0x02 << KEY_LEFT ;		// LEFT
//	}
//	y |= 0x02 << KEY_RIGHT ;	// RIGHT
//#endif
//	return y ;
//}


//void init_trims()
//{
//// Trims 
//#ifdef PCBX12D
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN ;
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ;
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ;
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN ;
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOJEN ;
//	configure_pins( TRIMS_GPIO_PIN_RHL, PIN_INPUT | PIN_PULLUP | PIN_PORTC ) ;
//	configure_pins( TRIMS_GPIO_PIN_RHR, PIN_INPUT | PIN_PULLUP | PIN_PORTI ) ;
//	configure_pins( TRIMS_GPIO_PIN_RVD, PIN_INPUT | PIN_PULLUP | PIN_PORTG ) ;
//	configure_pins( TRIMS_GPIO_PIN_LHL | TRIMS_GPIO_PIN_LHR, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	configure_pins( TRIMS_GPIO_PIN_LVU | TRIMS_GPIO_PIN_LVD | TRIMS_GPIO_PIN_RVU, PIN_INPUT | PIN_PULLUP | PIN_PORTJ ) ;

//// Now the extra ones

//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ;
//	configure_pins( GPIO_Pin_8, PIN_INPUT | PIN_PULLUP | PIN_PORTJ ) ;
//	configure_pins( GPIO_Pin_13, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//	configure_pins( GPIO_Pin_13 | GPIO_Pin_14, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
//#endif
//#ifdef PCBX10
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ;
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN ;
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOJEN ;
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ;

//	configure_pins( TRIMS_GPIO_PIN_LHL | TRIMS_GPIO_PIN_LHR | GPIO_Pin_14 | GPIO_Pin_13, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
//	configure_pins( TRIMS_GPIO_PIN_LVD, PIN_INPUT | PIN_PULLUP | PIN_PORTG ) ;
//	configure_pins( TRIMS_GPIO_PIN_LVU | TRIMS_GPIO_PIN_RVD | TRIMS_GPIO_PIN_RVU | GPIO_Pin_8, PIN_INPUT | PIN_PULLUP | PIN_PORTJ ) ;
//	configure_pins( TRIMS_GPIO_PIN_RHL | TRIMS_GPIO_PIN_RHR | GPIO_Pin_13, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
//#endif

//}

//uint32_t read_trims()
//{
//	uint32_t trims ;
//	uint32_t trima ;

//	trims = 0 ;

//#ifdef PCBX12D
//	trima = GPIOC->IDR ;
//	if ( ( trima & TRIMS_GPIO_PIN_RHL ) == 0 )
//	{
//		trims |= 0x40 ;
//	}

//	trima = GPIOI->IDR ;
//	if ( ( trima & TRIMS_GPIO_PIN_RHR ) == 0 )
//	{
//		trims |= 0x80 ;
//	}

//	trima = GPIOG->IDR ;
//	if ( ( trima & TRIMS_GPIO_PIN_RVD ) == 0 )
//	{
//		trims |= 0x10 ;
//	}

//	trima = GPIOD->IDR ;
//	if ( ( trima & TRIMS_GPIO_PIN_LHL ) == 0 )
//	{
//		trims |= 1 ;
//	}
//	if ( ( trima & TRIMS_GPIO_PIN_LHR ) == 0 )
//	{
//		trims |= 2 ;
//	}

//	trima = GPIOJ->IDR ;
//	if ( ( trima & TRIMS_GPIO_PIN_LVU ) == 0 )
//	{
//		trims |= 8 ;
//	}
//	if ( ( trima & TRIMS_GPIO_PIN_LVD ) == 0 )
//	{
//		trims |= 4 ;
//	}
//	if ( ( trima & TRIMS_GPIO_PIN_RVU ) == 0 )
//	{
//		trims |= 0x20 ;
//	}
//#endif
//#ifdef PCBX10
//	trima = GPIOB->IDR ;
//	if ( ( trima & TRIMS_GPIO_PIN_LHL ) == 0 )
//	{
//		trims |= 1 ;
//	}
//	if ( ( trima & TRIMS_GPIO_PIN_LHR ) == 0 )
//	{
//		trims |= 2 ;
//	}
//	trima = GPIOG->IDR ;
//	if ( ( trima & TRIMS_GPIO_PIN_LVD ) == 0 )
//	{
//		trims |= 4 ;
//	}
//	trima = GPIOJ->IDR ;
//	if ( ( trima & TRIMS_GPIO_PIN_LVU ) == 0 )
//	{
//		trims |= 8 ;
//	}
//	if ( ( trima & TRIMS_GPIO_PIN_RVD ) == 0 )
//	{
//		trims |= 0x10 ;
//	}
//	if ( ( trima & TRIMS_GPIO_PIN_RVU ) == 0 )
//	{
//		trims |= 0x20 ;
//	}
//	trima = GPIOD->IDR ;
//	if ( ( trima & TRIMS_GPIO_PIN_RHL ) == 0 )
//	{
//		trims |= 0x40 ;
//	}
//	if ( ( trima & TRIMS_GPIO_PIN_RHR ) == 0 )
//	{
//		trims |= 0x80 ;
//	}
//#endif

//	return trims ;
//}

//uint32_t hwKeyState( uint8_t key )
//{
//  register uint32_t b = GPIOB->IDR ;
//  register uint32_t g = GPIOG->IDR ;
//  register uint32_t h = GPIOH->IDR ;

//  uint32_t xxx = 0 ;
////  uint32_t analog = 0 ;
//	uint32_t avpot = AnalogData[10] ;
//	if ( g_eeGeneral.SixPositionCalibration[5] >  g_eeGeneral.SixPositionCalibration[0] )
//	{
//		avpot = 4095 - avpot ;
//	}
	
//	if( key > HSW_MAX )  return 0 ;

//	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )
//	{
//		uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
//		key -= HSW_Etrmdn ;		// 0 - 7
//		if (key >= 2 && key <= 5)       // swap back LH/RH trims
//		{
//			if ( g_eeGeneral.stickMode & 2 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		else
//		{
//			if ( g_eeGeneral.stickMode & 1 )
//			{
//				key ^= 0x06 ;
//			}
//		}
//		if ( ct )
//		{
//			key ^= 0x06 ;
//	 		if ( ct == 2 ) // Vintage style crosstrim
//			{
//	 			if (key >= 2 && key <= 5)       // swap back LH/RH trims
//				{
//					key ^= 0x06 ;
//				}
//			}
//		}
//		key += HSW_Etrmdn ;		// 0 - 7
//	}
////	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )
////	{
////		uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
////		if ( ct )
////		{
////			key -= HSW_Etrmdn ;		// 0 - 7
////			key ^= 0x06 ;
////	 		if ( ct == 2 ) // Vintage style crosstrim
//// 			{
////	 			if (key >= 2 && key <= 5)       // swap back LH/RH trims
////				{
////					key ^= 0x06 ;
////				}
//// 			}
////			key += HSW_Etrmdn ;		// 0 - 7
////		}
////	}

//  switch ( key )
//	{
//		case HSW_Ttrmup :
//			xxx = keyState( (EnumKeys) TRM_RV_DWN ) ;
//    break ;
		
//		case HSW_Ttrmdn :
//			xxx = keyState( (EnumKeys) TRM_RV_UP ) ;
//    break ;
		
//		case HSW_Rtrmup :
//			xxx = keyState( (EnumKeys) TRM_LH_DWN ) ;
//    break ;
		
//		case HSW_Rtrmdn :
//			xxx = keyState( (EnumKeys) TRM_LH_UP ) ;
//    break ;
		
//		case HSW_Atrmup :
//			xxx = keyState( (EnumKeys) TRM_RH_DWN ) ;
//    break ;
		
//		case HSW_Atrmdn :
//			xxx = keyState( (EnumKeys) TRM_RH_UP ) ;
//    break ;
		
//		case HSW_Etrmup :
//			xxx = keyState( (EnumKeys) TRM_LV_DWN ) ;
//    break ;
		
//		case HSW_Etrmdn :
//			xxx = keyState( (EnumKeys) TRM_LV_UP ) ;
//    break ;
		
////		case HSW_Ttrmup :
////			xxx = THR_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
////    break ;
		
////		case HSW_Ttrmdn :
////			xxx = THR_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
////    break ;
////		case HSW_Rtrmup :
////			xxx = RUD_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
////    break ;
////		case HSW_Rtrmdn :
////			xxx = RUD_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
////    break ;
////		case HSW_Atrmup :
////			xxx = AIL_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
////    break ;
////		case HSW_Atrmdn :
////			xxx = AIL_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
////    break ;
////		case HSW_Etrmup :
////			xxx = ELE_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx)) ;
////    break ;
////		case HSW_Etrmdn :
////			xxx = ELE_STICK ;
////			if ( g_eeGeneral.crosstrim )
////			{
////				xxx = 3 - xxx ;
////			}
////			xxx = TrimBits & (1 << (2*xxx+1)) ;
////    break ;

//    case HSW_SA0:
//      xxx = ~GPIOI->IDR & SWITCHES_GPIO_PIN_A_L ;
//      break;
//    case HSW_SA1:
//      xxx = ((GPIOI->IDR & SWITCHES_GPIO_PIN_A_L) | (h & SWITCHES_GPIO_PIN_A_H)) == (SWITCHES_GPIO_PIN_A_L | SWITCHES_GPIO_PIN_A_H) ;
//      break;
//    case HSW_SA2:
//      xxx = ~h & SWITCHES_GPIO_PIN_A_H ;
//      break;

//#if defined(PCBX10)
//    case HSW_SB0:
//      xxx = ~h & SWITCHES_GPIO_PIN_B_H ;
//      break;
//    case HSW_SB1:
//      xxx = ( ( b & SWITCHES_GPIO_PIN_B_L ) == SWITCHES_GPIO_PIN_B_L )
//						&& ( ( h & SWITCHES_GPIO_PIN_B_H ) == SWITCHES_GPIO_PIN_B_H ) ;
//      break;
//    case HSW_SB2:
//      xxx = ~b & SWITCHES_GPIO_PIN_B_L ;
//      break;
//#else
//    case HSW_SB0:
//      xxx = ~b & SWITCHES_GPIO_PIN_B_L ;
//      break;
//    case HSW_SB1:
//      xxx = ( ( b & SWITCHES_GPIO_PIN_B_L ) == SWITCHES_GPIO_PIN_B_L )
//						&& ( ( h & SWITCHES_GPIO_PIN_B_H ) == SWITCHES_GPIO_PIN_B_H ) ;
//      break;
//    case HSW_SB2:
//      xxx = ~h & SWITCHES_GPIO_PIN_B_H ;
//      break;
//#endif

//    case HSW_SC0:
//      xxx = ~b & SWITCHES_GPIO_PIN_C_L ;
//      break;
//    case HSW_SC1:
//      xxx = ((b & SWITCHES_GPIO_PIN_C_L) | (GPIOD->IDR & SWITCHES_GPIO_PIN_C_H)) == (SWITCHES_GPIO_PIN_C_L | SWITCHES_GPIO_PIN_C_H) ;
//      break;
//    case HSW_SC2:
//      xxx = ~GPIOD->IDR & SWITCHES_GPIO_PIN_C_H ;
//      break;

//#if defined(PCBX10)
//    case HSW_SD0:
//      xxx = ~GPIOJ->IDR & SWITCHES_GPIO_PIN_D_H ;
//			break;
//    case HSW_SD1:
//      xxx = ((g & SWITCHES_GPIO_PIN_D_L) | (GPIOJ->IDR & SWITCHES_GPIO_PIN_D_H)) == (SWITCHES_GPIO_PIN_D_L | SWITCHES_GPIO_PIN_D_H) ;
//      break;
//    case HSW_SD2:
//      xxx = ~g & SWITCHES_GPIO_PIN_D_L ;
//      break;

//    case HSW_SE0:
//      xxx = ~h & SWITCHES_GPIO_PIN_E_H ;
//      break;
//    case HSW_SE1:
//      xxx = ((h & SWITCHES_GPIO_PIN_E_H) | (GPIOE->IDR & SWITCHES_GPIO_PIN_E_L)) == (SWITCHES_GPIO_PIN_E_H | SWITCHES_GPIO_PIN_E_L) ;
//      break;
//    case HSW_SE2:
//      xxx = ~GPIOE->IDR & SWITCHES_GPIO_PIN_E_L ;
//      break;
//#else
//    case HSW_SD0:
//      xxx = ~g & SWITCHES_GPIO_PIN_D_L ;
//			break;
//    case HSW_SD1:
//      xxx = ((g & SWITCHES_GPIO_PIN_D_L) | (GPIOJ->IDR & SWITCHES_GPIO_PIN_D_H)) == (SWITCHES_GPIO_PIN_D_L | SWITCHES_GPIO_PIN_D_H) ;
//      break;
//    case HSW_SD2:
//      xxx = ~GPIOJ->IDR & SWITCHES_GPIO_PIN_D_H ;
//      break;

//    case HSW_SE0:
//      xxx = ~GPIOE->IDR & SWITCHES_GPIO_PIN_E_L ;
//      break;
//    case HSW_SE1:
//      xxx = ((h & SWITCHES_GPIO_PIN_E_H) | (GPIOE->IDR & SWITCHES_GPIO_PIN_E_L)) == (SWITCHES_GPIO_PIN_E_H | SWITCHES_GPIO_PIN_E_L) ;
//      break;
//    case HSW_SE2:
//      xxx = ~h & SWITCHES_GPIO_PIN_E_H ;
//      break;
//#endif
//    case HSW_SF2:
//#if defined(PCBX10)
//      xxx = ~h & SWITCHES_GPIO_PIN_F ;
//#else
//			if ( isProdVersion() )
//			{
//	      xxx = h & SWITCHES_GPIO_PIN_F ;
//			}
//			else
//			{
//	      xxx = ~h & SWITCHES_GPIO_PIN_F ;
//			}
//#endif
//		break;

//    case HSW_SG0:
//      xxx = ~g & SWITCHES_GPIO_PIN_G_L ;
//      break;
//    case HSW_SG1:
//      xxx = (g & (SWITCHES_GPIO_PIN_G_H | SWITCHES_GPIO_PIN_G_L)) == (SWITCHES_GPIO_PIN_G_H | SWITCHES_GPIO_PIN_G_L) ;
//      break;
//    case HSW_SG2:
//      xxx = ~g & SWITCHES_GPIO_PIN_G_H ;
//      break;

//    case HSW_SH2:
//      xxx = ~g & SWITCHES_GPIO_PIN_H;
//      break;

//#ifdef PCBX12D
//		case HSW_Pb1 :
//			xxx = ~GPIOB->IDR & GPIO_Pin_13 ;
//    break ;
			 
//		case HSW_Pb2 :
//			xxx = ~GPIOB->IDR & GPIO_Pin_14 ;
//    break ;

//		case HSW_Pb3 :
//			xxx = ~GPIOD->IDR & GPIO_Pin_13 ;
//    break ;
			 
//		case HSW_Pb4 :
//			xxx = ~GPIOJ->IDR & GPIO_Pin_8 ;
//    break ;
//#endif
//#ifdef PCBX10
//		case HSW_Pb1 :
//			xxx = ~GPIOD->IDR & GPIO_Pin_13 ;
//    break ;
			 
//		case HSW_Pb2 :
//			xxx = ~GPIOJ->IDR & GPIO_Pin_8 ;
//    break ;

//		case HSW_Pb3 :
//			xxx = ~GPIOB->IDR & GPIO_Pin_14 ;
//    break ;
			 
//		case HSW_Pb4 :
//			xxx = ~GPIOB->IDR & GPIO_Pin_13 ;
//    break ;
//#endif

//		case HSW_Ele6pos0 :
			
//			if ( avpot != 0xFFFF )
//			{
//				xxx = avpot > SixPositionTable[0] ;
//			}
//    break ;

//		case HSW_Ele6pos1 :
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[0] ) && ( avpot >= SixPositionTable[1] ) ;
//			}
//    break ;
		
//		case HSW_Ele6pos2 :
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[1] ) && ( avpot >= SixPositionTable[2] ) ;
//			}
//    break ;
		
//		case HSW_Ele6pos3 :
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[2] ) && ( avpot >= SixPositionTable[3] ) ;
//			}
//    break ;
		
//		case HSW_Ele6pos4 :
//			if ( avpot != 0xFFFF )
//			{
//				xxx = ( avpot <= SixPositionTable[3] ) && ( avpot >= SixPositionTable[4] ) ;
//			}
//    break ;

//		case HSW_Ele6pos5 :
//			if ( avpot != 0xFFFF )
//			{
//				xxx = avpot < SixPositionTable[4] ;
//			}
//    break ;
//	}
  
//	if ( xxx )
//  {
//    return 1 ;
//  }
//  return 0;
//}

//uint32_t keyState(EnumKeys enuk)
//{

//  if (enuk < (int) DIM(keys)) return keys[enuk].state() ? 1 : 0;

//	return hwKeyState( (uint8_t) enuk -SW_BASE + 1 ) ;
//}

//// Returns 0, 1 or 2 for ^ - or v
//static const uint8_t SwitchIndices[] = {HSW_SA0,HSW_SB0,HSW_SC0,HSW_SD0,HSW_SE0,HSW_SF2,HSW_SG0,HSW_SH2} ;

//uint32_t switchPosition( uint32_t swtch )
//{
//	if ( swtch < sizeof(SwitchIndices) )
//	{
//		swtch = SwitchIndices[swtch] ;
//	}

//	if ( swtch == HSW_SF2 )
//	{
//		if ( hwKeyState( swtch ) )
//		{
//			return 2 ;
//		}
//		return 0 ;
//	} 
//	if ( swtch == HSW_SH2 )
//	{
//		if ( hwKeyState( swtch ) )
//		{
//			return 2 ;
//		}
//		return 0 ;
//	} 
//	if ( hwKeyState( swtch ) )
//	{
//		return 0 ;
//	}
//	swtch += 1 ;
//	if ( hwKeyState( swtch ) )
//	{
//		return 1 ;			
//	}
//	if ( swtch == HSW_Ele6pos1 )
//	{
//		if ( hwKeyState( HSW_Ele6pos3 ) )
//		{
//			return 3 ;
//		}
//		if ( hwKeyState( HSW_Ele6pos4 ) )
//		{
//			return 4 ;
//		}
//		if ( hwKeyState( HSW_Ele6pos5 ) )
//		{
//			return 5 ;
//		}
//	}
//	return 2 ;
	
//}

//#endif

