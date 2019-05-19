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

#if defined(PCBX12D) || defined(PCBX10)
 #include "X12D/stm32f4xx.h"
#endif

#if defined(PCBX9D) || defined(PCBTARANIS) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
__attribute__ ((section(".bootrodata"), used))
static void bwdt_reset()
{
	IWDG->KR = 0xAAAA ;		// reload
}
#endif

#if defined(PCBX9D) || defined(PCBTARANIS) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)

__attribute__ ((section(".bootrodata"), used))
void _bootStart( void ) ;
extern "C" uint32_t _estack ;

__attribute__ ((section(".isr_boot_vector"), used))
const uint32_t BootVectors[] = {
	(uint32_t)&_estack,
  (uint32_t)(void (*)(void))((unsigned long)&_bootStart)
} ;
#endif

#if defined(PCBX9D) || defined(PCBTARANIS) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
__attribute__ ((section(".bootrodata.*"), used))
#endif

#ifdef PCBSKY
__attribute__ ((section(".bootrodata"), used))
#endif

const uint8_t BootCode[] = {
#ifdef PCBX12D
 #include "bootloader/bootflashX12D.lbm"
#else

#ifdef PCBX10
 #ifdef PCBT16
  #include "bootloader/bootflashT16.lbm"
 #else 
  #include "bootloader/bootflashX10.lbm"
 #endif
#else

#ifdef PCBTARANIS
  #if defined(REVPLUS) || defined(REV9E)
   #include "bootloader/bootflashTp.lbm"
	#else
   #include "bootloader/bootflashT.lbm"
	#endif
#else
#ifdef PCB9XT
 #include "bootloader/bootflash9xt.lbm"
#else
  #ifdef PCBX9D
   #if defined(REVPLUS) || defined(REV9E)
	  #ifdef REV9E
     #include "bootloader/bootflashx9e.lbm"
		#else
     #include "bootloader/bootflashXp.lbm"
		#endif
   #else
    #ifdef PCBX7
     #ifdef PCBT12
      #include "bootloader/bootflashT12.lbm"
		 #else
      #include "bootloader/bootflashX7.lbm"
		 #endif
    #else
     #ifdef PCBXLITE
      #include "bootloader/bootflashL.lbm"
     #else
      #ifdef PCBX9LITE
       #include "bootloader/bootflashX9l.lbm"
      #else
       #include "bootloader/bootflashX.lbm"
	    #endif
		 #endif
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
#endif
} ;


#if defined(PCBX9D) || defined(PCBTARANIS) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)

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
		GPIOJ->MODER = (GPIOJ->MODER & 0xFFFFFFF3) | 4 ; // General purpose output mode
	}

	GPIOC->PUPDR = 0x00000001 ;
	GPIOD->PUPDR = 0x00004000 ;

//// Turn on backlight here
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable port A clock
//	GPIOA->MODER = (GPIOA->MODER & 0xFFFFF33F) | 0x440 ; // General purpose output mode
//	GPIOA->BSRRL = 0x28 ;

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
			// Soft power on
			GPIOJ->BSRRL = 2 ; // set PWR_GPIO_PIN_ON pin to 1
			GPIOJ->MODER = (GPIOJ->MODER & 0xFFFFFFF3) | 4 ; // General purpose output mode
				 
			// Red LED on
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN ;
			GPIOI->BSRRL = 0x0020 ;
			GPIOI->MODER = (GPIOI->MODER & 0xFFFFF3FF) | 0x0400 ; // General purpose output mode
			
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

#ifdef PCBX10
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOJEN ; 		// Enable ports C, D and J clocks
	__ASM volatile ("nop") ;	// Needed for the STM32F4
	__ASM volatile ("nop") ;
	
	GPIOJ->PUPDR = 0x0008 ;	// PWR_GPIO_PIN_ON, pull down

	if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
	{
		GPIOJ->BSRRL = 2 ; // set PWR_GPIO_PIN_ON pin to 1
		GPIOJ->MODER = (GPIOJ->MODER & 0xFFFFFFF3) | 4 ; // General purpose output mode
	}

	GPIOD->PUPDR = 0x00000040 ; // RHL D3
	GPIOB->PUPDR = 0x00040000 ;	// LHR B9

//// Turn on backlight here
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable port A clock
//	GPIOA->MODER = (GPIOA->MODER & 0xFFFFF33F) | 0x440 ; // General purpose output mode
//	GPIOA->BSRRL = 0x28 ;

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


	if ( (GPIOB->IDR & 0x00000200 ) == 0 )
	{
		if ( (GPIOD->IDR & 0x00000008 ) == 0 )
		{
			// Soft power on
			GPIOJ->BSRRL = 2 ; // set PWR_GPIO_PIN_ON pin to 1
			GPIOJ->MODER = (GPIOJ->MODER & 0xFFFFFFF3) | 4 ; // General purpose output mode

			// Red LED on
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN ;
			GPIOE->BSRRL = 0x0004 ;
			GPIOE->MODER = (GPIOE->MODER & 0xFFFFF3FF) | 0x0010 ; // General purpose output mode
			
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

#ifndef PCBX12D
 #ifndef PCBX10
	// turn soft power on now
#ifdef PCB9XT
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOAEN ; 		// Enable portC and A clock
	__ASM volatile ("nop") ;	// Needed for the STM32F4
	__ASM volatile ("nop") ;
	GPIOC->PUPDR = 0x0020 ;	// PIN_MCU_PWR
#else // PCB9XT
 #ifdef REV9E
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOGEN ; // Enable portD clock
 #else // REV9E
  #ifdef PCBX7
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOEEN ; // Enable portD clock
  #else // PCBX7
   #ifdef PCBXLITE
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOEEN ; // Enable portA,C,E clock
   #else // PCBXLITE
    #ifdef PCBX9LITE
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN ; // Enable portA,C,E clock
    #else // PCBX9LITE
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOEEN ; // Enable portD clock
    #endif // PCBX9LITE
   #endif // PCBXLITE
  #endif // PCBX7
 #endif // REV9E
	__ASM volatile ("nop") ;	// Needed for the STM32F4
	__ASM volatile ("nop") ;

//  if (GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) == Bit_RESET)
// PD.01

 #if defined(REV9E) || defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)
	if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
//	if ( (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) || (GPIOA->IDR & 0x00000100 ) )	// Trainer input is high
	{
  #ifdef PCBXLITE
		GPIOE->BSRRL = GPIO_Pin_9 ; // set PWR_GPIO_PIN_ON pin to 1
		GPIOE->MODER = (GPIOE->MODER & 0xFFF3FFFF) | 0x00040000 ; // General purpose output mode
  #else // PCBXLITE
   #ifdef PCBX9LITE
		GPIOA->BSRRL = GPIO_Pin_6 ; // set PWR_GPIO_PIN_ON pin to 1
		GPIOA->MODER = (GPIOE->MODER & 0xFFFFCFFF) | 0x00001000 ; // General purpose output mode
   #else // PCBX9LITE
		GPIOD->BSRRL = 1; // set PWR_GPIO_PIN_ON pin to 1
		GPIOD->MODER = (GPIOD->MODER & 0xFFFFFFFC) | 1; // General purpose output mode
   #endif // PCBX9LITE
  #endif // PCBXLITE
	}
 #endif // multiple ifdef
#endif // PCB9XT

#ifdef PCBXLITE
	GPIOE->BSRRH = 0x0020 ; // set Green LED on
	GPIOE->MODER = (GPIOE->MODER & 0xFFFFF3FF) | 0x00000100 ; // General purpose output mode

	GPIOC->PUPDR = 0x00000500 ;		// PortC clock enabled above

#else // PCBXLITE
#ifdef PCBX7
	GPIOC->BSRRL = 0x0010 ; // set Green LED on
	GPIOC->MODER = (GPIOC->MODER & 0xFFFFFCFF) | 0x00000100 ; // General purpose output mode
#else // PCBX7
 #ifdef PCBX9LITE
	GPIOE->BSRRL = 0x0020 ; // set Red LED on
	GPIOE->MODER = (GPIOE->MODER & 0xFFFFF3FF) | 0x00000400 ; // General purpose output mode
 
	GPIOC->PUPDR = 0x00000500 ;		// PortC clock enabled above
	GPIOD->PUPDR = 0x00050000 ;		// PortD clock enabled above
 
 #endif // PCBX9LITE
#endif // PCBX7

#ifdef PCB9XT
	GPIOA->PUPDR = 0x14000000 ;
	GPIOC->PUPDR = 0x04004000 ;		// PortC clock enabled above
#else // PCB9XT
	
	GPIOC->PUPDR = 0x00000004 ;
 #ifdef REV9E
	GPIOG->PUPDR = 0x00000001 ;
 #else // REV9E
	GPIOE->PUPDR = 0x00000040 ;
 #endif // REV9E
#endif // PCB9XT
#endif // PCBXLITE

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
#endif // REV9E

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
#endif // PCBX7

#if defined(PCBXLITE) || defined(PCBX9LITE)
#define PWR_GPIO_PIN_SWITCH	0x0080
	if (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
	{
		// wait here until the power key is pressed
		while (GPIOA->IDR & PWR_GPIO_PIN_SWITCH)
		{
			bwdt_reset();
		}
	}
#endif // PCBXLITE/X3

//#ifndef PCB9XT
#ifdef PCBXLITE
	if ( (GPIOC->IDR & 0x00000010 ) == 0 )
	{
		if ( (GPIOC->IDR & 0x00000020 ) == 0 )
		{
#else // PCBXLITE
 #ifdef PCBX9LITE
	if ( (GPIOC->IDR & 0x00000010 ) == 0 )
	{
		if ( (GPIOD->IDR & 0x00000200 ) == 0 )
		{
 #else // PCBX9LITE
#ifdef PCB9XT
//	if ( 1 )
//	{
//		if ( 1 )
//		{
	if ( (GPIOA->IDR & 0x00004000 ) == 0 )
	{
		if ( (GPIOC->IDR & 0x00002000 ) == 0 )
		{
#else // PCB9XT
 #ifdef REV9E
	if ( (GPIOG->IDR & 0x00000001 ) == 0 )
 #else // REV9E
	if ( (GPIOE->IDR & 0x00000008 ) == 0 )
 #endif // REV9E
	{
		if ( (GPIOC->IDR & 0x00000002 ) == 0 )
		{
 #endif // PCBX9LITE
#endif // PCB9XT
#endif // PCBXLITE
			
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
 #endif // nPCBX10
#endif // nPCBX12D

//	run_application() ;	
	asm(" mov.w	r1, #134217728");	// 0x8000000
#if defined(PCBX12D) || defined(PCBX10)
	asm(" add.w	r1, #131072");		// 0x20000
#else  
	asm(" add.w	r1, #32768");			// 0x8000
#endif
	asm(" movw	r0, #60680");			// 0xED08
  asm(" movt	r0, #57344");			// 0xE000
  asm(" str	r1, [r0, #0]");			// Set the VTOR
	
  asm("ldr	r0, [r1, #0]");			// Stack pointer value
  asm("msr msp, r0");						// Set it
  asm("ldr	r0, [r1, #4]");			// Reset address
  asm("mov.w	r1, #1");
  asm("orr		r0, r1");					// Set lsbit
  asm("bx r0");									// Execute application

#if defined(PCBX12D) || defined(PCBX10)
	asm(".word 0x544F4F42") ;
#endif

}

#endif

