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

/*****************************************************************************/
#define TWI_CMD_PAGEUPDATE        	0x01	// TWI Command to program a flash page
#define TWI_CMD_EXECUTEAPP        	0x02	// TWI Command to jump to the application program
#define TWI_CMD_SETREAD_ADDRESS			0x03	// TWI Command to set address to read from
#define TWI_CMD_REBOOT							0x55	// TWI Command to restart back in the bootloader

#define INTVECT_PAGE_ADDRESS      	0x000	// The location of the start of the interrupt vector table address

#define __USI__  // set the communication type as USI



// Page size selection for the controller with 16K flash   

// The flash memory page size for Atiny167
#define PAGE_SIZE 128     

// Page 120, the start of bootloader section
#define BOOT_PAGE_ADDRESS 0X3D00  

// 16KB of flash divided by pages of size 128 bytes
#define TOTAL_NO_OF_PAGES  128   

// The number of pages being used for bootloader code
#define BOOTLOADER_PAGES          	(TOTAL_NO_OF_PAGES - BOOT_PAGE_ADDRESS/PAGE_SIZE)	

// For bounds check during page write/erase operation to protect the bootloader code from being corrupted
#define LAST_PAGE_NO_TO_BE_ERASED 	(TOTAL_NO_OF_PAGES - BOOTLOADER_PAGES)	

    
#define SELFPROGEN SPMEN


uint8_t pageBuffer[PAGE_SIZE];

#define WDT_TIMEOUT_8s      		( _BV( WDP3 ) | _BV( WDP0 ) )	        // Watchdog timeout for inactivity in the boot section

static void UpdatePage (uint16_t);  

#ifndef _BV
#define _BV( __BIT_POSITION__ ) ( 1 << __BIT_POSITION__ )
#endif

//#ifdef _BV
//#warning _BV now stands defined
//#endif
