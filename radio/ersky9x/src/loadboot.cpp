/****************************************************************************
*  Copyright (c) 2014 by Michael Blandford. All rights reserved.
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


#include <stdint.h>
#ifdef PCBSKY
#include "AT91SAM3S4.h"
#endif

#ifdef PCBTARANIS
 #include "stm32f2xx.h"
 #include "stm32f2xx_gpio.h"
#else
 #if defined(PCBX9D) || defined(PCB9XT)
  #include "X9D/stm32f2xx.h"
  #include "X9D/stm32f2xx_gpio.h"
 #endif
#endif

#ifdef PCBX12D
 #include "X12D/stm32f4xx.h"
#endif

#if defined(PCBX9D) || defined(PCBTARANIS) || defined(PCB9XT) || defined(PCBX12D)
__attribute__ ((section(".bootrodata"), used))
static void bwdt_reset()
{
	IWDG->KR = 0xAAAA ;		// reload
}
#endif

#if defined(PCBX9D) || defined(PCBTARANIS) || defined(PCB9XT) || defined(PCBX12D)

__attribute__ ((section(".bootrodata"), used))
void _bootStart( void ) ;
extern "C" uint32_t _estack ;

__attribute__ ((section(".isr_boot_vector"), used))
const uint32_t BootVectors[] = {
	(uint32_t)&_estack,
  (uint32_t)(void (*)(void))((unsigned long)&_bootStart)
} ;
#endif

#if defined(PCBX9D) || defined(PCBTARANIS) || defined(PCB9XT) || defined(PCBX12D)
__attribute__ ((section(".bootrodata.*"), used))
#endif

#ifdef PCBSKY
__attribute__ ((section(".bootrodata"), used))
#endif

const uint8_t BootCode[] = {
#ifdef PCBX12D
//	#include "bootloader/bootflashH.lbm"
#else
#ifdef PCBTARANIS
  #ifdef REVPLUS
   #include "bootloader/bootflashTp.lbm"
	#else
   #include "bootloader/bootflashT.lbm"
	#endif
#else
#ifdef PCB9XT
 #include "bootloader/bootflash9xt.lbm"
#else
  #ifdef PCBX9D
   #ifdef REVPLUS
	  #ifdef REV9E
     #include "bootloader/bootflashx9e.lbm"
		#else
     #include "bootloader/bootflashXp.lbm"
		#endif
   #else
    #ifdef PCBX7
     #include "bootloader/bootflashX7.lbm"
    #else
     #include "bootloader/bootflashX.lbm"
		#endif
   #endif
  #else
   #ifdef REVX
    #include "bootloader/bootflash8.lbm"
   #else
    #include "bootloader/bootflash4.lbm"
   #endif
  #endif
 #endif
#endif
#endif
} ;


#if defined(PCBX9D) || defined(PCBTARANIS) || defined(PCB9XT) || defined(PCBX12D)

#define WAS_RESET_BY_WATCHDOG_OR_SOFTWARE() (RCC->CSR & (RCC_CSR_WDGRSTF | RCC_CSR_WWDGRSTF | RCC_CSR_SFTRSTF))

__attribute__ ((section(".bootrodata"), used))

void _bootStart()
{
#ifdef PCBX12D
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOJEN ; 		// Enable ports C, D and J clocks
	__ASM volatile ("nop") ;	// Needed for the STM32F4
	__ASM volatile ("nop") ;
	
	GPIOJ->PUPDR = 0x0008 ;	// PWR_GPIO_PIN_ON, pull down

	if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
	{
		GPIOJ->BSRRL = 2 ; // set PWR_GPIO_PIN_ON pin to 1
		GPIOJ->MODER = (GPIOD->MODER & 0xFFFFFFF3) | 4 ; // General purpose output mode
	}

	GPIOC->PUPDR = 0x00000001 ;
	GPIOD->PUPDR = 0x00004000 ;

	uint32_t i ;
	for ( i = 0 ; i < 50000 ; i += 1 )
	{
		bwdt_reset() ;
	}

//#define PWR_GPIO_PIN_SWITCH	0x0001
//	if (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
//	{
//		// wait here until the power key is pressed
//		while (GPIO1->IDR & PWR_GPIO_PIN_SWITCH)
//		{
//			bwdt_reset();
//		}
//	}


	if ( (GPIOD->IDR & 0x00000080 ) == 0 )
	{
		if ( (GPIOC->IDR & 0x00000001 ) == 0 )
		{
			// Bootloader needed
			const uint8_t *src ;
			uint8_t *dest ;
			uint32_t size ;

			bwdt_reset() ;
			size = sizeof(BootCode) ;
			src = BootCode ;
			dest = (uint8_t *)0x20000000 ;

			for ( ; size ; size -= 1 )
			{
				*dest++ = *src++ ;		
			}	
			// Could check for a valid copy to RAM here
			// Go execute bootloader
			bwdt_reset() ;
			
			uint32_t address = *(uint32_t *)0x20000004 ;
	
//			((void (*)(void)) (address))() ;		// Go execute the loaded application
	
		}
	}


#endif

#ifndef PCBX12D
	// turn soft power on now
#ifdef PCB9XT
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOAEN ; 		// Enable portC and A clock
	__ASM volatile ("nop") ;	// Needed for the STM32F4
	__ASM volatile ("nop") ;
	GPIOC->PUPDR = 0x0020 ;	// PIN_MCU_PWR
#else
 #ifdef REV9E
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOGEN ; // Enable portD clock
 #else
  #ifdef PCBX7
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOEEN ; // Enable portD clock
  #else
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOEEN ; // Enable portD clock
  #endif
 #endif
	__ASM volatile ("nop") ;	// Needed for the STM32F4
	__ASM volatile ("nop") ;
	
 #ifdef REV9E
	if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
 #endif
 #ifdef PCBX7
	if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
//	if ( (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) || (GPIOA->IDR & 0x00000100 ) )	// Trainer input is high
 #endif
	{
		GPIOD->BSRRL = 1; // set PWR_GPIO_PIN_ON pin to 1
		GPIOD->MODER = (GPIOD->MODER & 0xFFFFFFFC) | 1; // General purpose output mode
	}
#endif
#ifdef PCBX7
	GPIOC->BSRRL = 0x0010 ; // set Green LED on
	GPIOC->MODER = (GPIOC->MODER & 0xFFFFFCFF) | 0x00000100 ; // General purpose output mode
#endif // PCBX7

#ifdef PCB9XT
	GPIOA->PUPDR = 0x14000000 ;
	GPIOC->PUPDR = 0x04004000 ;		// PortC clock enabled above
#else
	
	GPIOC->PUPDR = 0x00000004 ;
 #ifdef REV9E
	GPIOG->PUPDR = 0x00000001 ;
 #else
	GPIOE->PUPDR = 0x00000040 ;
 #endif
#endif

//#ifdef PCBX9D
//	uint32_t j ;
//	uint32_t k ;
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
//	GPIOB->MODER = (GPIOB->MODER & 0xFFFCFFFF) | 0x00010000 ; // General purpose output mode
//	for ( k = 0 ; k < 20 ; k += 1 )
//	{
//		GPIOB->BSRRL = 0x00000100 ; // set backlight ON
//		for ( j = 0 ; j < 1000000 ; j += 1 )
//		{
//			bwdt_reset() ;
//		}
//		GPIOB->BSRRH = 0x00000100 ; // set backlight ON
//		for ( j = 0 ; j < 1000000 ; j += 1 )
//		{
//			bwdt_reset() ;
//		}
//		GPIOB->BSRRH = 0x00000100 ; // set backlight ON
//	}
//#endif





	uint32_t i ;
	for ( i = 0 ; i < 50000 ; i += 1 )
	{
		bwdt_reset() ;
	}
 // now the second part of power on sequence
// If we got here and the radio was not started by the watchdog/software reset,
// then we must have a power button pressed. If not then we are in power on/off loop
// and to terminate it, just wait here without turning on PWR pin. The power supply will
// eventually exhaust and the radio will turn off.
#ifdef REV9E
#define PWR_GPIO_PIN_SWITCH	0x0001
	if (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
	{
		// wait here until the power key is pressed
		while (GPIOD->IDR & PWR_GPIO_PIN_SWITCH)
		{
			bwdt_reset();
		}
	}
#endif

#ifdef PCBX7
#define PWR_GPIO_PIN_SWITCH	0x0002
	if (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
	{
		// wait here until the power key is pressed
		while (GPIOD->IDR & PWR_GPIO_PIN_SWITCH)
		{
			bwdt_reset();
		}
	}
#endif

//#ifndef PCB9XT
#ifdef PCB9XT
//	if ( 1 )
//	{
//		if ( 1 )
//		{
	if ( (GPIOA->IDR & 0x00004000 ) == 0 )
	{
		if ( (GPIOC->IDR & 0x00002000 ) == 0 )
		{
#else
 #ifdef REV9E
	if ( (GPIOG->IDR & 0x00000001 ) == 0 )
 #else
	if ( (GPIOE->IDR & 0x00000008 ) == 0 )
 #endif
	{
		if ( (GPIOC->IDR & 0x00000002 ) == 0 )
		{
#endif
			// Bootloader needed
			const uint8_t *src ;
			uint8_t *dest ;
			uint32_t size ;

			bwdt_reset() ;
			size = sizeof(BootCode) ;
			src = BootCode ;
			dest = (uint8_t *)0x20000000 ;

			for ( ; size ; size -= 1 )
			{
				*dest++ = *src++ ;		
			}	
			// Could check for a valid copy to RAM here
			// Go execute bootloader
			bwdt_reset() ;
			
			uint32_t address = *(uint32_t *)0x20000004 ;
	
			((void (*)(void)) (address))() ;		// Go execute the loaded application
	
		}
	}
#endif

//	run_application() ;	
	asm(" mov.w	r1, #134217728");	// 0x8000000
  asm(" add.w	r1, #32768");			// 0x8000
	
	asm(" movw	r0, #60680");			// 0xED08
  asm(" movt	r0, #57344");			// 0xE000
  asm(" str	r1, [r0, #0]");			// Set the VTOR
	
  asm("ldr	r0, [r1, #0]");			// Stack pointer value
  asm("msr msp, r0");						// Set it
  asm("ldr	r0, [r1, #4]");			// Reset address
  asm("mov.w	r1, #1");
  asm("orr		r0, r1");					// Set lsbit
  asm("bx r0");									// Execute application
}
#endif


