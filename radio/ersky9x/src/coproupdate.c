/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2011, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */


/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
 
#include <board.h>
#include <string.h>

/*----------------------------------------------------------------------------
 *        Internal definitions
 *----------------------------------------------------------------------------*/

/** TWI clock frequency in Hz. */
#define TWICK    100000

/** Max size of data we can tranfsert in one shot */
#define MAX_COUNT 0x0040		// 64

/** Address value used to select the coprocessor. */
#define TWI_ADDR			0x35

/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/

/** Size of one page in the coprocessor, in bytes.  */
static uint32_t pageSize;

/** Size of one block in the serial flash, in bytes.  */
static uint32_t blockSize;

/**Size of the buffer used for read/write operations in bytes. */
static uint32_t bufferSize;

uint8_t Twi_rx_buf[26] ;

/*----------------------------------------------------------------------------
 *         Global functions
 *----------------------------------------------------------------------------*/

void init_twi( void ) ;
uint32_t read_CoProc( uint32_t coInternalAddr, uint32_t bufaddr, uint32_t number ) ;
uint32_t write_CoProc( uint32_t coInternalAddr, uint32_t bufaddr, uint32_t number ) ;
uint32_t coProcBoot( void ) ;
uint32_t read_status_CoProc( uint32_t bufaddr, uint32_t number ) ;

void init_twi()
{
	register Pio *pioptr ;
	register uint32_t timing ;
  
	PMC->PMC_PCER0 |= 0x00080000L ;		// Enable peripheral clock to TWI0
	
	/* Configure PIO */
	pioptr = PIOA ;
  pioptr->PIO_ABCDSR[0] &= ~0x00000018 ;	// Peripheral A
  pioptr->PIO_ABCDSR[1] &= ~0x00000018 ;	// Peripheral A
  pioptr->PIO_PDR = 0x00000018 ;					// Assign to peripheral
	
	timing = 64000000 * 5 / 1500000 ;		// 5uS high and low 100kHz, trying 200 kHz
	timing += 15 - 4 ;
	timing /= 16 ;
	timing |= timing << 8 ;

	TWI0->TWI_CWGR = 0x00040000 | timing ;			// TWI clock set
	TWI0->TWI_CR = TWI_CR_MSEN | TWI_CR_SVDIS ;		// Master mode enable
	TWI0->TWI_MMR = 0x00350000 ;		// Device 35 and master is writing
}

uint8_t Addr_buffer[132] ;

uint32_t read_status_CoProc( uint32_t bufaddr, uint32_t number )
{
	uint32_t i ;
	
	TWI0->TWI_MMR = 0x00351000 ;		// Device 35 and master is reading,
	TWI0->TWI_RPR = bufaddr ;
	TWI0->TWI_RCR = number-1 ;
	if ( TWI0->TWI_SR & TWI_SR_RXRDY )
	{
		(void) TWI0->TWI_RHR ;
	}
	TWI0->TWI_PTCR = TWI_PTCR_RXTEN ;	// Start transfers
	TWI0->TWI_CR = TWI_CR_START ;		// Start Rx
	
	// Now wait for completion
	
	for ( i = 0 ; i < 1000000 ; i += 1 )
	{
		if ( TWI0->TWI_SR & TWI_SR_RXBUFF )
		{
			break ;
		}
	}
	// Finished reading
	TWI0->TWI_PTCR = TWI_PTCR_RXTDIS ;	// Stop transfers
	TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Rx
	TWI0->TWI_RCR = 1 ;						// Last byte

	if ( i >= 1000000 )
	{
		return 0 ;
	}
	// Now wait for completion
	
	for ( i = 0 ; i < 100000 ; i += 1 )
	{
		if ( TWI0->TWI_SR & TWI_SR_TXCOMP )
		{
			break ;
		}	
	}
	if ( i >= 100000 )
	{
		return 0 ;
	}
	return 1 ;
}

/*
uint32_t read_CoProc( uint32_t coInternalAddr, uint32_t bufaddr, uint32_t number )
{
	uint32_t i ;

	Addr_buffer[0] = 0x03 ;		// Command, set address
	Addr_buffer[1] = coInternalAddr >> 8 ;
	Addr_buffer[2] = coInternalAddr ;
	
	// Now send TWI data using PDC
	TWI0->TWI_MMR = 0x00350000 ;		// Device 35 and master is writing
	// init PDC
	TWI0->TWI_TPR = (uint32_t)&Addr_buffer[1] ;
	TWI0->TWI_TCR = 2 ;
			
	TWI0->TWI_THR = Addr_buffer[0] ;		// Send data
	// kick off PDC, enable PDC end interrupt
	TWI0->TWI_PTCR = TWI_PTCR_TXTEN ;	// Start transfers
	
	// Now wait for completion
	
	for ( i = 0 ; i < 1000000 ; i += 1 )
	{
		if ( TWI0->TWI_SR & TWI_SR_TXBUFE )
		{
			break ;
		}
	}
	// Finished reading
	TWI0->TWI_PTCR = TWI_PTCR_TXTDIS ;	// Stop transfers
	TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
	if ( i >= 1000000 )
	{
		return 0 ;
	}
	
	for ( i = 0 ; i < 100000 ; i += 1 )
	{
		if ( TWI0->TWI_SR & TWI_SR_TXCOMP )
		{
			break ;
		}	
	}
	if ( i >= 100000 )
	{
		return 0 ;
	}

	TWI0->TWI_MMR = 0x00351000 ;		// Device 35 and master is reading,
	TWI0->TWI_RPR = bufaddr ;
	TWI0->TWI_RCR = number ;
	if ( TWI0->TWI_SR & TWI_SR_RXRDY )
	{
		(void) TWI0->TWI_RHR ;
	}
	TWI0->TWI_PTCR = TWI_PTCR_RXTEN ;	// Start transfers
	TWI0->TWI_CR = TWI_CR_START ;		// Start Rx
	
	// Now wait for completion
	
	for ( i = 0 ; i < 1000000 ; i += 1 )
	{
		if ( TWI0->TWI_SR & TWI_SR_RXBUFF )
		{
			break ;
		}
	}
	// Finished reading
	TWI0->TWI_PTCR = TWI_PTCR_RXTDIS ;	// Stop transfers
	TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Rx
	if ( TWI0->TWI_SR & TWI_SR_RXRDY )
	{
		(void) TWI0->TWI_RHR ;			// Discard any rubbish data
	}
	if ( i >= 1000000 )
	{
		return 0 ;
	}
	// Now wait for completion
	
	for ( i = 0 ; i < 100000 ; i += 1 )
	{
		if ( TWI0->TWI_SR & TWI_SR_TXCOMP )
		{
			break ;
		}	
	}
	if ( i >= 100000 )
	{
		return 0 ;
	}
	return 1 ;
}
*/

#define TWI_CMD_REBOOT							0x55	// TWI Command to restart back in the bootloader

uint32_t coProcBoot()
{
	uint32_t i ;
	
	TWI0->TWI_MMR = 0x00350000 ;		// Device 35 and master is writing
	TWI0->TWI_THR = TWI_CMD_REBOOT ;	// Send reboot command
	TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx

	for ( i = 0 ; i < 100000 ; i += 1 )
	{
		if ( TWI0->TWI_SR & TWI_SR_TXCOMP )
		{
			break ;
		}	
	}
	
	if ( i >= 100000 )
	{
		return 0 ;
	}
	return 1 ;
}

uint32_t write_CoProc( uint32_t coInternalAddr, uint32_t bufaddr, uint32_t number )
{
	uint32_t i ;
	uint8_t *ptr ;

	Addr_buffer[0] = 0x01 ;		// Command, PROGRAM
	Addr_buffer[1] = coInternalAddr >> 8 ;
	Addr_buffer[2] = coInternalAddr ;
	ptr = (uint8_t *)bufaddr ;
	// Copy data
	for ( i = 0 ; i < number ; i += 1 )
	{
		Addr_buffer[i+3] = *ptr++ ;		
	}
	// Pad to 128 bytes
	while ( i < 128 )
	{
		Addr_buffer[i+3] = 0xFF ;
		i += 1 ;	
	}
	// Now send TWI data using PDC
	TWI0->TWI_MMR = 0x00350000 ;		// Device 35 and master is writing
	// init PDC
	TWI0->TWI_TPR = (uint32_t)&Addr_buffer[1] ;
	TWI0->TWI_TCR = 130 ;
			
	TWI0->TWI_THR = Addr_buffer[0] ;		// Send data
	// kick off PDC, enable PDC end interrupt
	TWI0->TWI_PTCR = TWI_PTCR_TXTEN ;	// Start transfers
	
	// Now wait for completion
	
	for ( i = 0 ; i < 1000000 ; i += 1 )
	{
		if ( TWI0->TWI_SR & TWI_SR_TXBUFE )
		{
			break ;
		}
	}
	// Finished reading
	TWI0->TWI_PTCR = TWI_PTCR_TXTDIS ;	// Stop transfers
	TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
	if ( i >= 1000000 )
	{
		return 0 ;
	}
	
	for ( i = 0 ; i < 100000 ; i += 1 )
	{
		if ( TWI0->TWI_SR & TWI_SR_TXCOMP )
		{
			break ;
		}	
	}
	
	if ( i >= 100000 )
	{
		return 0 ;
	}

	return 1 ;
}

// returns 0 = OK, not 0 error
uint32_t check_ready()
{
	uint32_t result ;
	result = 0 ;				// OK
	init_twi() ;

	// initialise for updating
	if ( read_status_CoProc( (uint32_t)Twi_rx_buf, 22 ) )
	{
		if ( ( Twi_rx_buf[0] & 0x80 ) == 0 )
		{ // Not in bootloader
			if ( coProcBoot() == 0 )		// Make sure we are in the bootloader
			{
   		  result = 1 ;
			}
			else
			{
				uint32_t i ;

				for ( i = 0 ; i < 10000 ; i += 1 )
				{
					asm("nop") ;
				}
				if ( read_status_CoProc( (uint32_t)Twi_rx_buf, 22 ) )
				{
					if ( ( Twi_rx_buf[0] & 0x80 ) != 0x80 )
					{
		   		  result = 2 ;
					}
				}
				else
				{
   		  	result = 3 ;
				}	 
			}
		}
	}
	else
	{
   	result = 4 ;
	}

	read_status_CoProc( (uint32_t)Twi_rx_buf, 22 ) ;

	return result ;
}

uint32_t write( uint32_t addr, uint8_t *buffer, uint32_t count )
{
	uint32_t result ;
	result = 0 ;				// OK

	if (addr % 4)
	{
		result = 1 ;
	}
	else
	{
		result = write_CoProc( addr, buffer, count ) ;
	}
	return result ;
}

