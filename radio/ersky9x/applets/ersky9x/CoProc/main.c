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
 
#include "../common/applet.h"
#include <board.h>
//#include "libspiflash.h"
#include <string.h>

#undef DYN_TRACES
#define DYN_TRACES 0



/*----------------------------------------------------------------------------
 *        Internal definitions
 *----------------------------------------------------------------------------*/

/** TWI clock frequency in Hz. */
#define TWICK    100000


/** Stack size in SRAM */
#define STACK_SIZE 0x100

/** Indicate the farthest memory offset ever erase, if current write address is less
 than the address, the related block should be erase again before write. */
//static uint32_t writtenAddress = 0;


/** Max size of data we can tranfsert in one shot */
#define MAX_COUNT 0x0040		// 64

/** Address value used to select the coprocessor. */
#define TWI_ADDR			0x35

/*----------------------------------------------------------------------------
 *        Types
 *----------------------------------------------------------------------------*/
/** Structure for storing parameters for each command that can be performed by
the applet. */
struct _Mailbox {

    /** Command send to the monitor to be executed. */
    uint32_t command;
    /** Returned status, updated at the end of the monitor execution.*/
    uint32_t status;

    /** Input Arguments in the argument area*/
    union {

        /** Input arguments for the Init command.*/
        struct {

            /** Communication link used.*/
            uint32_t comType;
            /**Trace level.*/
            uint32_t traceLevel;
            /** Serial flash index.*/
            uint32_t at25Idx;

        } inputInit;

        /** Output arguments for the Init command.*/
        struct {

            /** Memory size.*/
            uint32_t memorySize;
            /** Buffer address.*/
            uint32_t bufferAddress;
            /** Buffer size.*/
            uint32_t bufferSize;

        } outputInit;

        /** Input arguments for the Write command.*/
        struct {

            /** Buffer address.*/
            uint32_t bufferAddr;
            /** Buffer size.*/
            uint32_t bufferSize;
            /** Memory offset.*/
            uint32_t memoryOffset;

        } inputWrite;

        /** Output arguments for the Write command.*/
        struct {

            /** Bytes written.*/
            uint32_t bytesWritten;

            uint32_t bufferSize;		// temp
            /** Memory offset.*/
            uint32_t memoryOffset;	// temp
						uint32_t debug1 ;
						uint32_t debug2 ;

        } outputWrite;

        /** Input arguments for the Read command.*/
        struct {

            /** Buffer address. */
            uint32_t bufferAddr;
            /** Buffer size.*/
            uint32_t bufferSize;
            /** Memory offset.*/
            uint32_t memoryOffset;

        } inputRead;

        /** Output arguments for the Read command.*/
        struct {

            /** Bytes read.*/
            uint32_t bytesRead;

        } outputRead;

        /** Input arguments for the Full Erase command.*/
        // NONE

        /** Output arguments for the Full Erase command.*/
        // NONE
        /** Input arguments for the Buffer Erase command.*/
        struct {

            /** Memory offset to be erase.*/
            uint32_t memoryOffset;

        } inputBufferErase;

        /** Output arguments for the Buffer Erase command.*/
        struct {

            /** Memory offset to be erase.*/
            uint32_t bytesErased;

        } outputBufferErase;
    } argument;
};

/*----------------------------------------------------------------------------
 *        Global variables
 *----------------------------------------------------------------------------*/

/** End of program space (code + data). */
extern int end;
extern int _sstack;
/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/

/** Size of one page in the coprocessor, in bytes.  */
static uint32_t pageSize;

/** Size of one block in the serial flash, in bytes.  */
static uint32_t blockSize;

/**Size of the buffer used for read/write operations in bytes. */
static uint32_t bufferSize;

/** Depending on DYN_TRACES, dwTraceLevel is a modifable runtime variable or a define */
uint32_t dwTraceLevel;

uint32_t Spi_init_done = 0 ;
struct _Mailbox *Pmailbox = 0 ;

uint8_t Twi_tx_buf[140] ;
uint8_t Twi_rx_buf[80] ;

uint8_t Block_buffer[1024] ;

/*----------------------------------------------------------------------------
 *         Global functions
 *----------------------------------------------------------------------------*/


//uint32_t  eeprom_write_one( uint8_t byte, uint8_t count ) ;
//uint32_t eeprom_read_status( void ) ;
//void eeprom_write_enable( void ) ;
//void eeprom_wait_busy( void ) ;
//uint32_t spi_operation( register uint8_t *tx, register uint8_t *rx, register uint32_t count ) ;
////void init_spi( void ) ;
////uint32_t unprotect_eeprom( void ) ;
//uint32_t spi_PDC_action( register uint8_t *command, register uint8_t *tx, register uint8_t *rx, register uint32_t comlen, register uint32_t count ) ;
//uint32_t eeprom_block_erased( register uint8_t *p) ;
//uint32_t eeprom_page_erased( register uint8_t *p) ;
//void AT25D_Read( uint8_t *BufferAddr, uint32_t size, uint32_t memoryOffset) ;
//void AT25D_Write( uint8_t *BufferAddr, uint32_t size, uint32_t memoryOffset ) ;
//uint32_t AT25D_EraseBlock( uint32_t memoryOffset ) ;
//uint32_t min( uint32_t a, uint32_t b ) ;

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


/**
 * \brief Applet main entry. This function decodes received command and executes it.
 * \param argc  always 1
 * \param argv  Address of the argument area.
*/

int main(int argc, char **argv)
{
    struct _Mailbox *pMailbox = (struct _Mailbox *) argv;
		Pmailbox = pMailbox ;
    /* Communication type with SAM-BA GUI. */
//    uint8_t comType;

//    uint32_t jedecId;
    uint32_t bytesToWrite, bytesToRead, bufferAddr, startMemoryOffset, memoryOffset, packetSize;
    /* index on read/write buffer */
    uint8_t *pBuffer;
    /* Temporary buffer used for non block aligned read/write */
//    uint32_t tempBufferAddr;
    /* Offset in destination buffer during buffer copy */
//    uint32_t bufferOffset;
    
//		uint32_t block_address ;
//		uint32_t x ;
    /* INIT */  
    /* Save communication link type */
//    comType = pMailbox->argument.inputInit.comType;
		
		init_twi() ;

    if (pMailbox->command == APPLET_CMD_INIT)
		{
 

#if (DYN_TRACES == 1)
        dwTraceLevel = pMailbox->argument.inputInit.traceLevel;
#endif

//        TRACE_INFO("-- SerialFlash AT25/AT26 applet %s --\n\r", SAM_BA_APPLETS_VERSION);
//        TRACE_INFO("-- %s\n\r", BOARD_NAME);
//        TRACE_INFO("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
        /* Configure pins */
//        PIO_Configure(pins, PIO_LISTSIZE(pins));
        /* Initialize DMA driver instance with polling mode */
//        DMAD_Initialize( &dmad, 1 );
        
        /* Initialize the SPI and serial flash */
				init_twi() ;

				if ( read_status_CoProc( (uint32_t)Twi_rx_buf, 22 ) )
				{
					if ( ( Twi_rx_buf[0] & 0x80 ) == 0 )
					{ // Not in bootloader
//        TRACE_INFO("-- Co_proc %s", Twi_rx_buf[0] );
						
						if ( coProcBoot() == 0 )		// Make sure we are in the bootloader
						{
        		  pMailbox->status = APPLET_NO_DEV | 0x00001000 ;
						}
						else
						{
							uint32_t i ;
        		  pMailbox->status = APPLET_SUCCESS ;

							for ( i = 0 ; i < 10000 ; i += 1 )
							{
								asm("nop") ;
							}
							if ( read_status_CoProc( (uint32_t)Twi_rx_buf, 22 ) )
							{
								if ( ( Twi_rx_buf[0] & 0x80 ) != 0x80 )
								{
        		  		pMailbox->status = APPLET_NO_DEV | 0x00003000 ;
								}
							}
							else
							{
        		  	pMailbox->status = APPLET_NO_DEV | 0x00004000 ;
							}	 
						}
					}
				}
				else
				{
          pMailbox->status = APPLET_NO_DEV | 0x00002000 ;
				}

				read_status_CoProc( (uint32_t)Twi_rx_buf, 22 ) ;

//        SPID_Configure(&spid, SPI0, ID_SPI0, &dmad);
//        AT25_Configure(&at25, &spid, SPI_CS, 1);
//        TRACE_INFO("SPI and AT25/AT25 drivers initialized\n\r");
        pMailbox->argument.outputInit.bufferAddress = (uint32_t) &end ;
        /* Read the JEDEC ID of the device to identify it */
//        jedecId = AT25D_ReadJedecId(&at25);
//        if (AT25_FindDevice(&at25, jedecId) == 0) {
//            pMailbox->status = APPLET_DEV_UNKNOWN;
//            pMailbox->argument.outputInit.bufferSize = 0;
//            pMailbox->argument.outputInit.memorySize = 0;
//            TRACE_INFO("Device Unknown\n\r");
//            goto exit;
//        }
//        else {
            /* Get device parameters */
            pageSize = 128 ;
//            pageSize = AT25_PageSize(&at25);
            blockSize = 128 ;
//            blockSize = AT25_BlockSize(&at25);

            /* Program page */
//            if (AT25_ManId(&at25) == SST_SPI_FLASH) {
                /* SST Flash write is slower, we reduce buffer size to avoid USB timeout */
//                bufferSize = 10 * pageSize;
//            }
//            else {
                bufferSize = 2 * blockSize;
//            }
            /* integer number of pages can be contained in each buffer */
            bufferSize -= bufferSize % pageSize; 
            if ( bufferSize < pageSize) {
  //              TRACE_INFO("No enought memory to load buffer.\n\r");
                goto exit;
            } 
            pMailbox->argument.outputInit.bufferSize = bufferSize;
            pMailbox->argument.outputInit.memorySize = 128*128 ;
//            TRACE_INFO("%s blockSize : 0x%lx bufferAddr : 0x%lx\n\r",
//                   "ersky9x", blockSize, pMailbox->argument.outputInit.bufferAddress);
//        }
    }

    // ----------------------------------------------------------
    // WRITE:
    // ----------------------------------------------------------
    else if (pMailbox->command == APPLET_CMD_WRITE)
		{
        startMemoryOffset = pMailbox->argument.inputWrite.memoryOffset;
        memoryOffset      = startMemoryOffset;
        bufferAddr        = pMailbox->argument.inputWrite.bufferAddr;
//        tempBufferAddr    = bufferAddr + bufferSize;
        bytesToWrite      = pMailbox->argument.inputWrite.bufferSize;
//        TRACE_INFO("WRITE at offset: 0x%lx buffer at : 0x%lx of: 0x%lx Bytes\n\r",
//               memoryOffset, bufferAddr, bytesToWrite);
        /* Check word alignment */
        if (memoryOffset % 4)
				{
          pMailbox->status = APPLET_ALIGN_ERROR;
          goto exit;
        }

//				unprotect_eeprom() ;
//        if (AT25D_Unprotect(&at25)) {

//            TRACE_INFO("Can not unprotect the flash\n\r");
//            pMailbox->status = APPLET_UNPROTECT_FAIL;
//            goto exit;
//        }

//        pBuffer = (uint8_t *) bufferAddr;

//				block_address = memoryOffset &0xFFFFF000 ;		// 4k boundary
//        AT25D_Read( Block_buffer, 4096, block_address ) ;	// read block to write to
//				// Check to see if it is blank
//				x = eeprom_block_erased( Block_buffer ) ;
//				if ( x == 0 )
//				{
//					AT25D_EraseBlock( block_address ) ;
//				}

				while(bytesToWrite)
				{
					uint32_t number ;

					number = bytesToWrite ;
					if ( number > 128 )
					{
						number = 128 ;						
					}
					write_CoProc( memoryOffset, bufferAddr, number ) ;
					bytesToWrite -= number ;
					bufferAddr += number ;
					memoryOffset += number ;
				}

////        TRACE_INFO("WRITE return byte written : 0x%lx Bytes\n\r",
////               pMailbox->argument.inputWrite.bufferSize - bytesToWrite);

//        pMailbox->argument.outputWrite.bytesWritten =  0 ;
        pMailbox->argument.outputWrite.bytesWritten = pMailbox->argument.inputWrite.bufferSize - bytesToWrite;

//        pMailbox->argument.outputWrite.bytesWritten = bytesToWrite;
        pMailbox->status = APPLET_SUCCESS ;
    }

//    // ----------------------------------------------------------
//    // READ:
//    // ----------------------------------------------------------
    else if (pMailbox->command == APPLET_CMD_READ)
		{
        memoryOffset = pMailbox->argument.inputRead.memoryOffset;
        bufferAddr   = pMailbox->argument.inputRead.bufferAddr;
        bytesToRead  = pMailbox->argument.inputRead.bufferSize;

//        TRACE_INFO("READ at offset: 0x%lx buffer at : 0x%lx of: 0x%lx Bytes\n\r",
//               memoryOffset, bufferAddr, bytesToRead);
        /*  Check word alignment */
        if (memoryOffset % 4) {

            pMailbox->status = APPLET_ALIGN_ERROR;
            goto exit;
        }
        pBuffer = (uint8_t *) bufferAddr;

        /* Read packet after packets */
        while (((uint32_t)pBuffer < (bufferAddr + bufferSize)) && (bytesToRead > 0))
				{
				    packetSize = bytesToRead ;
						if ( packetSize > MAX_COUNT )
						{
							packetSize = MAX_COUNT ;					
						}

						read_CoProc( memoryOffset, (uint32_t)pBuffer, packetSize ) ;
            pBuffer += packetSize;
            bytesToRead -= packetSize;
            memoryOffset += packetSize;
        }

//        TRACE_INFO("READ return byte read : 0x%lx Bytes\n\r",
//               pMailbox->argument.inputRead.bufferSize - bytesToRead);

        pMailbox->argument.outputRead.bytesRead = pMailbox->argument.inputRead.bufferSize - bytesToRead;
        pMailbox->status = APPLET_SUCCESS;
    }

//    // ----------------------------------------------------------
//    // FULL ERASE:
//    // ----------------------------------------------------------
    else if (pMailbox->command == APPLET_CMD_FULL_ERASE)
		{
            pMailbox->status = APPLET_ERASE_FAIL;
//        TRACE_INFO("FULL ERASE\n\r");

//				unprotect_eeprom() ;

//        /* Unprotected the flash */
////        if (AT25D_Unprotect(&at25)) {

////            TRACE_INFO("Can not unprotect the flash\n\r");
////            pMailbox->status = APPLET_UNPROTECT_FAIL;
////            goto exit;
////        }

//        TRACE_INFO("Flash unprotected\n\r");

//        /* Erase the chip */
//        TRACE_INFO("Chip is being erased...\n\r");

////        if (AT25D_EraseChip()) {

//				// NOT IMPLEMENTED (YET?)

//            TRACE_INFO("Erasing error\n\r");
//            pMailbox->status = APPLET_ERASE_FAIL;
//            goto exit;
////        }

//        TRACE_INFO("Full Erase achieved\n\r");
//        pMailbox->status = APPLET_SUCCESS;
    }
//    // ----------------------------------------------------------
//    // BUFFER ERASE:
//    // ----------------------------------------------------------
    else if (pMailbox->command == APPLET_CMD_BUFFER_ERASE)
		{

//        TRACE_INFO("BUFFER ERASE \n\r");
         /*  Unprotected the flash */
//				unprotect_eeprom() ;
        
//				if (AT25D_Unprotect(&at25)) {

//            TRACE_INFO("Can not unprotect the flash\n\r");
//            pMailbox->status = APPLET_UNPROTECT_FAIL;
//            goto exit;
//        }

//         memoryOffset = pMailbox->argument.inputBufferErase.memoryOffset;

//       if (AT25D_EraseBlock( memoryOffset))
//			 {
            pMailbox->status = APPLET_ERASE_FAIL;
//            TRACE_INFO("Block erasing error\n\r");
//            goto exit;
  //     }
//        pMailbox->argument.outputBufferErase.bytesErased = 4096;

//        TRACE_INFO("Buffer Erase achieved\n\r");
//        pMailbox->status = APPLET_SUCCESS;
    }
exit:
    /* Acknowledge the end of command */
//    TRACE_INFO("\tEnd of applet (command : %lx --- status : %lx)\n\r", pMailbox->command, pMailbox->status);

    /*  Notify the host application of the end of the command processing */
    pMailbox->command = ~(pMailbox->command);
    /* Send ACK character */
//    if (comType == DBGU_COM_TYPE)
//		{
         /* Wait for the transmitter to be ready */
//        while ( (DBGU->DBGU_SR & DBGU_SR_TXEMPTY) == 0 ) ;
        /* Send character */
//         DBGU->DBGU_THR= 0x06 ;
//    }
    return 0;
}

