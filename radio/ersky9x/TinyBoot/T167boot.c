/****************************************************************************
*  Copyright (c) 2011 by Michael Blandford. All rights reserved.
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
****************************************************************************/
// Developed from Atmel AVR112: TWI Bootloader for devices without boot section
// and changed so that it actually works

#include <inttypes.h>          
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <stdint.h>
#include <avr/wdt.h>

//First, the code should be put into a new named section. This is done with a section
//attribute:
//__attribute__ ((section (".bootloader")))
//In this example, .bootloader is the name of the new section. This attribute needs to be
//placed after the prototype of any function to force the function into the new section.
//void boot(void) __attribute__ ((section (".bootloader")));
//To relocate the section to a fixed address the linker flag -section-start is used.
//This option can be passed to the linker using the -Wl compiler option:
//-Wl,--section-start=.bootloader=0x1E000
//The name after section-start is the name of the section to be relocated. The number
//after the section name is the beginning address of the named section.


// Read data (for application, just version for bootloader)
// Version 0b1xxxxxxx for bootloader, 0x0xxxxxxx for app - 1 byte
// Digital switches - 1 byte
// RotEnc0, RotEnc1 - 2 bytes
// pot0, pot1, pot2, pot3 - 8 bytes
// Temperature - 1 byte
// Mah used - 1 byte ??
// YY,MM,DD,hh,mm,ss - 6 bytes
// Spare - 2 bytes
// Total 22 bytes

//static void Disable_WatchDogTimer( void ) ;
void hold_jvectors( void ) ;

#include "USI_TWI_Slave.c"

uint8_t state = 0;
#define gtimeout WDT_TIMEOUT_8s


/***********************************************************************/
/*  Absolute Unconditional Page Erase  Check bounds elsewhere    */
static void Erase_One_Page (uint16_t addr) 
{
  // Erase page at the given address
	__boot_page_erase_normal(addr) ;
  
	boot_spm_busy_wait() ;
	SPMCSR = ((1 << CTPB) | (1 << SPMEN)) ;
  asm (" spm\n") ;	
	boot_spm_busy_wait() ;


}
/***********************************************************************/
static void UpdatePage (uint16_t pageAddress)
{
  // Mask out in-page address bits.
  pageAddress &= ~(PAGE_SIZE - 1);
  // Protect RESET vector if this is page 0.
  if (pageAddress != INTVECT_PAGE_ADDRESS)
  {
  // Ignore any attempt to update boot section.
  	if (pageAddress < BOOT_PAGE_ADDRESS)
  	{
  	  Erase_One_Page (pageAddress);

  	  // Load temporary page buffer.
  	  uint8_t *bufferPtr = pageBuffer;
  	  uint16_t tempAddress = pageAddress;
  	  for (uint8_t i = 0; i < PAGE_SIZE; i += 2)
  	  {
  	    uint16_t tempWord = *bufferPtr ;
  	    tempWord |= *(bufferPtr+1) << 8 ;
  	    __boot_page_fill_normal(tempAddress, tempWord) ;
  	    tempAddress += 2;
  	    bufferPtr += 2;
  	  }
  	  // Write page from temporary buffer to the given location in flasm memory
			__boot_page_write_normal(pageAddress) ;
    
			boot_spm_busy_wait() ;
			SPMCSR = ((1 << CTPB) | (1 << SPMEN)) ;
	  	asm (" spm\n") ;	
			boot_spm_busy_wait() ;
			wdt_reset();		 // Reset the watchdog timer
  	}
  }
}

/***********************************************************************/
NOINLINE void Init_WatchDogTimer (void) ;
void main (void) __attribute__((noreturn)) ;

/***********************************************************************/
// Main Starts from here
void main (void)
{
	PRR = 0x3D ;			// Power down unused peripherals
	ACSR = 0x80 ;			// Power down analogue comparator
  USI_TWI_SLAVE_Init();
  Init_WatchDogTimer ();
  USI_TWI_SLAVE_ReadAndProcessPacket ();
}


// Vectors for page 0, protected from being overwritten
__attribute__ ((section (".jvectors")))
void hold_jvectors()
{

  asm (" jmp __ctors_end\n") ;	
  asm (" jmp 0x0084\n") ;
  asm (" jmp 0x0088\n") ;
  asm (" jmp 0x008C\n") ;
  asm (" jmp 0x0090\n") ;
  asm (" jmp 0x0094\n") ;
  asm (" jmp 0x0098\n") ;
  asm (" jmp 0x009C\n") ;
  asm (" jmp 0x00A0\n") ;
  asm (" jmp 0x00A4\n") ;
  asm (" jmp 0x00A8\n") ;
  asm (" jmp 0x00AC\n") ;
  asm (" jmp 0x00B0\n") ;
  asm (" jmp 0x00B4\n") ;
  asm (" jmp 0x00B8\n") ;
  asm (" jmp 0x00BC\n") ;
  asm (" jmp 0x00C0\n") ;
  asm (" jmp 0x00C4\n") ;
  asm (" jmp 0x00C8\n") ;
  asm (" jmp 0x00CC\n") ;
}

__attribute__ ((section (".jvectors")))
void __vector_default( void )
{
  asm (" jmp 0\n") ;
}


__attribute__ ((section (".jvectors")))
NOINLINE void set_clock( uint8_t speed )
{
	volatile uint8_t *ptr ;
	ptr = &CLKPR ;
	*ptr = 0x80 ;
	*ptr = speed ;
}

__attribute__ ((section (".jvectors")))
void Init_WatchDogTimer (void)
{
/*Timed sequence to initialize watchdog timer
	for the given mode set in gtimeout variable
	Cross calls during maximum optimization can
	cause more than 4 cycles delay between change
	enable for WDT and setting values*/
	wdt_reset();
	WDTCR = (1 << WDCE) | ( 1 << WDE ) ;
	WDTCR = ( 1 << WDE ) | gtimeout ;
}

