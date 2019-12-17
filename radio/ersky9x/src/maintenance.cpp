/*
 * Author - Mike Blandford
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
#include <stdlib.h>
#include <string.h>


#if defined(PCBX12D) || defined(PCBX10)
//#define	WHERE_TRACK		1
//void notePosition( uint8_t byte ) ;
#endif

#if defined(PCBTARANIS)
#include "opentx.h"
#include "stm32f2xx_flash.h"
#define PCBX9D 1
extern uint8_t *cpystr( uint8_t *dest, uint8_t *source ) ;
#define lcd_puts_Pleft lcd_putsLeft
#define lcd_outdez lcd_outdezAtt
#define read_keys readKeys
#define lcd_putsn_P lcd_putsn

uint8_t *cpystr( uint8_t *dest, uint8_t *source )
{
  while ( (*dest++ = *source++) )
    ;
  return dest - 1 ;
}

#define MKEY_RIGHT  KEY_ENTER
#define MKEY_LEFT   KEY_PAGE


#else
#ifdef PCBSKY
#include "AT91SAM3S4.h"
#include "core_cm3.h"
 #ifndef SMALL
  #include "pdi.h"
 #endif
#endif
#ifdef PCBX9D
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_flash.h"
#include "X9D/hal.h"
#include "pdi.h"
#endif
#ifdef PCB9XT
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_flash.h"
#include "X9D/hal.h"
#include "pdi.h"
#endif
#if defined(PCBX12D) || defined(PCBX10)
#include "X12D/stm32f4xx.h"
#include "X12D/stm32f4xx_flash.h"
#include "X12D/hal.h"
#include "pdi.h"
#endif


#include "ersky9x.h"
#include "lcd.h"
#include "menus.h"
#include "drivers.h"
#include "logicio.h"
#ifndef SIMU
#include "CoOS.h"
#endif
#include "ff.h"
#include "sound.h"
#include "frsky.h"

#define MKEY_RIGHT  KEY_RIGHT
#define MKEY_LEFT   KEY_LEFT

#endif
#include "maintenance.h"
#include "myeeprom.h"

extern union t_sharedMemory SharedMemory ;

#if defined(PCBX9D) || defined(PCB9XT)
extern uchar PdiErrors0 ;
extern uchar PdiErrors1 ;
extern uchar PdiErrors2 ;
extern uchar PdiErrors3 ;
extern uchar PdiErrors4 ;
extern uchar PdiErrors5 ;
extern uchar PdiErrors6 ;
extern uchar PdiErrors7 ;
extern uchar PdiErrors8 ;
#endif
//#if defined(PCB9XT)
//uint16_t lastPacketType ;
//#endif

extern uint8_t SetByEncoder ;

#define SPORT_INTERNAL	0
#define SPORT_EXTERNAL	1

#define	NO_RECEIVE		0
#define WITH_RECEIVE	1

#if defined(PCBX9D) || defined(PCB9XT)
#if !defined(PCBTARANIS)
#define INTERNAL_RF_ON()      GPIO_SetBits(GPIOPWRINT, PIN_INT_RF_PWR)
#define INTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWRINT, PIN_INT_RF_PWR)
#define EXTERNAL_RF_ON()      GPIO_SetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#define EXTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
 #if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
#define SPORT_RF_ON()		      GPIO_SetBits(GPIOPWRSPORT, PIN_SPORT_PWR)
#define SPORT_RF_OFF()  			GPIO_ResetBits(GPIOPWRSPORT, PIN_SPORT_PWR)
#endif
#endif
#endif

#if defined(PCBX12D) || defined(PCBX10)
#define INTERNAL_RF_ON()      GPIO_SetBits(GPIOPWRINT, PIN_INT_RF_PWR)
#define INTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWRINT, PIN_INT_RF_PWR)
#define EXTERNAL_RF_ON()      GPIO_SetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#define EXTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#endif

// Update Types
#define UPDATE_TYPE_BOOTLOADER		0
//#define UPDATE_TYPE_COPROCESSOR		1
#define UPDATE_TYPE_SPORT_INT			2
#define UPDATE_TYPE_SPORT_EXT			3
#define UPDATE_TYPE_CHANGE_ID			4
#define UPDATE_TYPE_AVR						5
#define UPDATE_TYPE_XMEGA					6
#define UPDATE_TYPE_MULTI					7
#define UPDATE_TYPE_PCHIP					8

 #if defined(PCBXLITE) || defined(PCBX9LITE)
#define SPORT_MODULE		0
#define SPORT_EXT				1
uint8_t SportModuleExt ;
#endif

extern void frsky_receive_byte( uint8_t data ) ;
uint16_t crc16_ccitt( uint8_t *buf, uint32_t len ) ;
uint8_t checkIndexed( uint8_t y, const prog_char * s, uint8_t value, uint8_t edit ) ;
uint32_t checkForExitEncoderLong( uint8_t event ) ;

extern void copyFileName( char *dest, char *source, uint32_t size ) ;
#ifdef PCBSKY
void init_mtwi( void ) ;
 #ifndef REVX
//uint32_t check_ready( void ) ;
//uint32_t write_CoProc( uint32_t coInternalAddr, uint8_t *buf, uint32_t number ) ;
//uint32_t coProcBoot( void ) ;
//uint32_t read_status_CoProc( uint32_t bufaddr, uint32_t number ) ;

uint8_t Twi_rx_buf[26] ;
//uint8_t CoProresult ;
 #endif
#ifdef REVX
uint32_t clearMfp() ;
#endif

 #ifdef SMALL
	#define NO_MULTI	1
 #endif

#endif

uint32_t sportUpdate( uint32_t external ) ;
 #ifndef SMALL
uint32_t xmegaUpdate() ;
 #endif
uint32_t multiUpdate() ;


//struct t_maintenance Mdata ;


extern FATFS g_FATFS ;
extern uint8_t FileData[] ;	// Share with voice task
#if defined(PCBTARANIS)
uint8_t FileData[1024] ;
uint8_t ExtraFileData[1024] ;
#else
uint8_t *ExtraFileData = (uint8_t *) &VoiceBuffer[0] ;	// Share with voice task
#endif
uint32_t FileSize[8] ;

uint32_t BytesFlashed ;
uint32_t ByteEnd ;
uint32_t BlockOffset ;
uint16_t MultiPageSize ;
//uint8_t UpdateItem ;
uint8_t MaintenanceRunning = 0 ;
//uint8_t BlockInUse ;
//uint8_t SportVerValid ;
uint8_t MultiResult ;
uint8_t FileType ;
uint8_t MultiPort ;
uint8_t MultiModule ;
uint8_t MultiInvert ;
uint8_t MultiStm ;
uint8_t SportVersion[4] ;
uint32_t FirmwareSize ;

const uint8_t SportIds[28] = {0x00, 0xA1, 0x22, 0x83, 0xE4, 0x45, 0xC6, 0x67,
				                      0x48, 0xE9, 0x6A, 0xCB, 0xAC, 0x0D, 0x8E, 0x2F,
															0xD0, 0x71, 0xF2, 0x53, 0x34, 0x95, 0x16, 0xB7,
															0x98, 0x39, 0xBA, 0x1B/*, 0x7C, 0xDD, 0x5E, 0xFF*/ } ;

//uint8_t SportState ;
#define SPORT_IDLE				0
#define SPORT_START				1
#define SPORT_POWER_ON		2
#define SPORT_VERSION			3
#define SPORT_DATA_START	4
#define SPORT_DATA				5
#define SPORT_DATA_READ		6
#define SPORT_END					7
#define SPORT_FAIL				8
#define SPORT_COMPLETE		9

uint8_t XmegaState ;
#define XMEGA_IDLE				0
#define XMEGA_START				1
#define XMEGA_POWER				2
#define XMEGA_BEGIN				3
#define XMEGA_FLASHING		4
#define XMEGA_DONE				5

uint8_t AllSubState ;
uint8_t XmegaSignature[4] ;
uint8_t Fuses[8] ;

uint8_t MultiState ;
#define MULTI_IDLE				0
#define MULTI_START				1
#define MULTI_WAIT1				2
#define MULTI_WAIT2				3
#define MULTI_BEGIN				4
#define MULTI_FLASHING		5
#define MULTI_DONE				6


#ifdef PCBSKY
#define CLEAR_TX_BIT_EXT() PIOA->PIO_CODR = PIO_PA17
#define SET_TX_BIT_EXT() PIOA->PIO_SODR = PIO_PA17
#define CLEAR_TX_BIT_INT() PIOC->PIO_CODR = PIO_PC15
#define SET_TX_BIT_INT() PIOC->PIO_SODR = PIO_PC15
#else
 #ifdef PCB9XT
#define CLEAR_TX_BIT_EXT() GPIOA->BSRRH = 0x0080
#define SET_TX_BIT_EXT() GPIOA->BSRRL = 0x0080
#define CLEAR_TX_BIT_INT() GPIOA->BSRRH = 0x0400
#define SET_TX_BIT_INT() GPIOA->BSRRL = 0x0400
 #else
#if defined(PCBXLITE) || defined(PCBX9LITE)
#define CLEAR_TX_BIT() GPIOC->BSRRL = 0x0040
#define SET_TX_BIT() GPIOC->BSRRH = 0x0040
#else
  #ifndef PCBX10
   #ifndef PCBX12D
#define CLEAR_TX_BIT() GPIOA->BSRRL = 0x0080
#define SET_TX_BIT() GPIOA->BSRRH = 0x0080
   #endif
  #endif
#endif
 #endif
#endif

#ifndef NO_MULTI
void initMultiMode()
{
#ifdef PCBSKY
#ifdef REVX
	init_mtwi() ;
	clearMfp() ;
#endif
	USART0->US_IDR = US_IDR_RXRDY ;
	UART0->UART_IDR = UART_IDR_RXRDY ;
	if ( MultiPort )
	{
		init_software_com2( 57600, MultiInvert ? SERIAL_NORM : SERIAL_INVERT, SERIAL_NO_PARITY ) ;
//		if ( PIOA->PIO_PDSR & PIO_PA9 )
//		{
//			init_software_com2( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ;
//		}
	}
	else
	{
		init_software_com1( 57600, MultiInvert ? SERIAL_NORM : SERIAL_INVERT, SERIAL_NO_PARITY ) ;
//		if ( PIOA->PIO_PDSR & PIO_PA5 )
//		{
//			init_software_com1( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ;
//		}
	}
	if ( MultiModule )
	{
		SET_TX_BIT_INT() ;
		configure_pins( PIO_PC15, PIN_ENABLE | PIN_OUTPUT | PIN_PORTC | PIN_HIGH ) ;
	}
	else
	{
		SET_TX_BIT_EXT() ;
		configure_pins( PIO_PA17, PIN_ENABLE | PIN_OUTPUT | PIN_PORTA | PIN_HIGH ) ;
	}
#endif
#ifdef PCB9XT
	if ( MultiPort )
	{
		com2_Configure( 57600, SERIAL_INVERT, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
	}
	else
	{
		com1_Configure( 57600, SERIAL_INVERT, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
	}
	if ( MultiModule )
	{
		configure_pins( PIN_INTPPM_OUT, PIN_OUTPUT | PIN_PORTA | PIN_HIGH ) ;
		INTERNAL_RF_ON() ;
	}
	else
	{
		configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PORTA | PIN_HIGH ) ;
		EXTERNAL_RF_ON() ;
	}
#endif
#ifdef PCBX9D
	com1_Configure( 57600, SERIAL_INVERT, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
	EXTERNAL_RF_ON() ;
	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PORT_EXTPPM | PIN_LOW ) ;
#endif
#ifdef PCBX10
	com1_Configure( 57600, SERIAL_INVERT, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
	EXTERNAL_RF_ON() ;
	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PORTA | PIN_LOW ) ;
#endif
}

void stopMultiMode()
{
	
#ifdef PCBSKY
	if ( MultiPort )
	{
		disable_software_com2() ;
	}
	else
	{
		disable_software_com1() ;
	}
	if ( MultiModule )
	{
		SET_TX_BIT_INT() ;
		configure_pins( PIO_PC15, PIN_ENABLE | PIN_INPUT | PIN_PORTC | PIN_PULLUP ) ;
	}
	else
	{
		configure_pins( PIO_PA17, PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
	}
	com1_Configure( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
#endif
#ifdef PCB9XT
	if ( MultiPort )
	{
		com2_Configure( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
	}
	else
	{
		com1_Configure( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
	}
	if ( MultiModule )
	{
		INTERNAL_RF_OFF() ;
		configure_pins( PIN_INTPPM_OUT, PIN_OUTPUT | PIN_PORTA | PIN_HIGH ) ;
	}
	else
	{
		EXTERNAL_RF_OFF() ;
		configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PORTA | PIN_HIGH ) ;
	}
#endif
#ifdef PCBX9D
	com1_Configure( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
//#if /*defined(PCBXLITE) || */ defined(PCBX9LITE)
//	GPIO_ResetBits( GPIOBOOTCMD, PIN_BOOTCMD ) ;
//#endif
	EXTERNAL_RF_OFF() ;
	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PORT_EXTPPM | PIN_LOW ) ;
#endif
#ifdef PCBX10
	com1_Configure( 57600, SERIAL_INVERT, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
	EXTERNAL_RF_OFF() ;
	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PORTA | PIN_LOW ) ;
#endif
}

uint16_t getMultiFifo()
{
	if ( MultiPort )
	{
		return get_fifo128( &Com2_fifo ) ;
	}
	else
	{
		return get_fifo128( &Com1_fifo ) ;
	}
}

void sendMultiByte( uint8_t byte )
{
	uint16_t time ;
	uint32_t i ;
	
#ifdef PCBSKY
	Pio *pioptr ;
	uint32_t bit ;
	
	if ( MultiModule )
	{
		pioptr = PIOC ;
		bit = PIO_PC15 ;
	}
	else
	{
		pioptr = PIOA ;
		bit = PIO_PA17 ;
	}
#define CLEAR_TX_BIT() pioptr->PIO_CODR = bit
#define SET_TX_BIT() pioptr->PIO_SODR = bit
#endif

#ifdef PCB9XT
	uint32_t bit ;
	
	if ( MultiModule )
	{
		bit = 0x0400 ;
	}
	else
	{
		bit = 0x0080 ;
	}
#define CLEAR_TX_BIT() GPIOA->BSRRH = bit
#define SET_TX_BIT() GPIOA->BSRRL = bit
#endif

#ifdef PCBX12D
	uint32_t bit ;

	bit = isProdVersion() ? PIN_EXTPPM_OUT : PROT_PIN_EXTPPM_OUT ;

#define CLEAR_TX_BIT() GPIOA->BSRRL = bit
#define SET_TX_BIT() GPIOA->BSRRH = bit
#endif

#ifdef PCBX10
	uint32_t bit ;
	if ( MultiModule )
	{
		bit = 0x0400 ;
	}
	else
	{
		bit = 0x0400 ;
	}
#define CLEAR_TX_BIT() GPIOA->BSRRL = bit
#define SET_TX_BIT() GPIOA->BSRRH = bit
#endif

	__disable_irq() ;
	time = getTmr2MHz() ;
	CLEAR_TX_BIT() ;
	while ( (uint16_t) (getTmr2MHz() - time) < 34 )
	{
		// wait
	}
	time += 34 ;
	for ( i = 0 ; i < 8 ; i += 1 )
	{
		if ( byte & 1 )
		{
			SET_TX_BIT() ;
		}
		else
		{
			CLEAR_TX_BIT() ;
		}
		byte >>= 1 ;
		while ( (uint16_t) (getTmr2MHz() - time) < 35 )
		{
			// wait
		}
		time += 35 ;
	}
	SET_TX_BIT() ;
	__enable_irq() ;	// No need to wait for the stop bit to complete
	while ( (uint16_t) (getTmr2MHz() - time) < 34 )
	{
		// wait
	}
}
#endif


#ifdef PCBSKY
//uint32_t (*IAP_Function)(uint32_t, uint32_t) ;
extern uint32_t ChipId ;

uint32_t program( uint32_t *address, uint32_t *buffer )	// size is 256 bytes
{
	uint32_t FlashSectorNum ;
	uint32_t flash_cmd = 0 ;
	uint32_t i ;
	uint32_t size ;
	
#ifdef SMALL
	if ( (uint32_t) address >= 0x00407000 )
#else
	if ( (uint32_t) address >= 0x00408000 )
#endif
	{
		return 1 ;
	}

	// Always initialise this here, setting a default doesn't seem to work
	SharedMemory.Mdata.IAP_Function = (uint32_t (*)(uint32_t, uint32_t))  *(( uint32_t *)0x00800008) ;
	FlashSectorNum = (uint32_t) address ;
	
	if ( ChipId & 0x0080 )
	{
		size = 128 ;
		FlashSectorNum >>= 9 ;		// page size is 512 bytes
		FlashSectorNum &= 1023 ;	// max page number
	}
	else
	{
		size = 64 ;
		FlashSectorNum >>= 8 ;		// page size is 256 bytes
		FlashSectorNum &= 2047 ;	// max page number
	}

	/* Send data to the sector here */
	for ( i = 0 ; i < size ; i += 1 )
	{
		*address++ = *buffer++ ;		
	}

	if ( ChipId & 0x0080 )
	{
		if ( ( FlashSectorNum & 7 ) == 0 )
		{
			flash_cmd = (0x5A << 24) | (FlashSectorNum << 8) | 0x00000100 | 0x07 ; //AT91C_MC_FCMD_EPA = erase (8) pages
			__disable_irq() ;
			/* Call the IAP function with appropriate command */
			i = SharedMemory.Mdata.IAP_Function( 0, flash_cmd ) ;
			__enable_irq() ;
		}
		flash_cmd = (0x5A << 24) | (FlashSectorNum << 8) | 0x01 ; //AT91C_MC_FCMD_WP = write page
	}
	else
	{
		/* build the command to send to EEFC */
		flash_cmd = (0x5A << 24) | (FlashSectorNum << 8) | 0x03 ; //AT91C_MC_FCMD_EWP ;
	}

	__disable_irq() ;
	/* Call the IAP function with appropriate command */
	i = SharedMemory.Mdata.IAP_Function( 0, flash_cmd ) ;
	__enable_irq() ;
	return i ;
}
#endif

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
//After reset, write is not allowed in the Flash control register (FLASH_CR) to protect the
//Flash memory against possible unwanted operations due, for example, to electric
//disturbances. The following sequence is used to unlock this register:
//1. Write KEY1 = 0x45670123 in the Flash key register (FLASH_KEYR)
//2. Write KEY2 = 0xCDEF89AB in the Flash key register (FLASH_KEYR)
//Any wrong sequence will return a bus error and lock up the FLASH_CR register until the
//next reset.
//The FLASH_CR register can be locked again by software by setting the LOCK bit in the
//FLASH_CR register.
void unlockFlash()
{
	FLASH->KEYR = 0x45670123 ;
	FLASH->KEYR = 0xCDEF89AB ;
}

void waitFlashIdle()
{
	while (FLASH->SR & FLASH_FLAG_BSY)
	{
	 	wdt_reset() ;
	}
}
#define SECTOR_MASK               ((uint32_t)0xFFFFFF07)

void eraseSector( uint32_t sector )
{
	waitFlashIdle() ;

  FLASH->CR &= CR_PSIZE_MASK;
  FLASH->CR |= FLASH_PSIZE_WORD ;
  FLASH->CR &= SECTOR_MASK;
  FLASH->CR |= FLASH_CR_SER | (sector<<3) ;
	__disable_irq() ;
  FLASH->CR |= FLASH_CR_STRT;
    
  /* Wait for operation to be completed */
	waitFlashIdle() ;
	__enable_irq() ;
    
  /* if the erase operation is completed, disable the SER Bit */
  FLASH->CR &= (~FLASH_CR_SER);
  FLASH->CR &= SECTOR_MASK; 
}

uint32_t program( uint32_t *address, uint32_t *buffer )	// size is 256 bytes
{
	uint32_t i ;

#if defined(PCBX12D) || defined(PCBX10)
extern void initLongWatchdog(uint32_t time) ;
	initLongWatchdog(4) ;
	if ( (uint32_t) address >= 0x08020000 )
#else
	if ( (uint32_t) address >= 0x08008000 )
#endif
	{
		return 1 ;
	}
	if ( (uint32_t) address == 0x08000000 )
	{
		eraseSector( 0 ) ;
	}
	if ( (uint32_t) address == 0x08004000 )
	{
		eraseSector( 1 ) ;
	}
#if defined(PCBX12D) || defined(PCBX10)
	if ( (uint32_t) address == 0x08008000 )
	{
		eraseSector( 2 ) ;
	}
	if ( (uint32_t) address == 0x0800C000 )
	{
		eraseSector( 3 ) ;
	}
	if ( (uint32_t) address == 0x08010000 )
	{
		eraseSector( 4 ) ;
	}
#endif
	// Now program the 256 bytes
	 
  for (i = 0 ; i < 64 ; i += 1 )
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    
	  // Wait for last operation to be completed
		waitFlashIdle() ;
  
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR |= FLASH_CR_PG;
  
    *address = *buffer ;
        
		__disable_irq() ;
    /* Wait for operation to be completed */
		waitFlashIdle() ;
    FLASH->CR &= (~FLASH_CR_PG);
		__enable_irq() ;
		 
		 /* Check the written value */
    if ( *address != *buffer )
    {
      /* Flash content doesn't match SRAM content */
      return 2 ;
    }
    /* Increment FLASH destination address */
    address += 1 ;
		buffer += 1 ;
  }
  return 0 ;
}

#endif


uint32_t validateFile( uint32_t *block )
{
	uint32_t i ;
	uint8_t *bytes ;
	
#if defined(PCBX9D) || defined(PCB9XT)
	if ( ( block[0] & 0xFFFC0000 ) != 0x20000000 )
	{
#ifdef REV9E
		if ( ( block[0] & 0xFFFC0000 ) != 0x10000000 )	// Cover silly openTx stack
		{
			return 0 ;
		}
#else
		return 0 ;
#endif
	}
	if ( ( block[1] & 0xFFF00000 ) != 0x08000000 )
	{
		return 0 ;
	}
#endif

#ifdef PCBSKY
	if ( ( block[0] & 0xFFFE3000 ) != 0x20000000 )
	{
		return 0 ;
	}
	if ( ( block[1] & 0xFFF80000 ) != 0x00400000 )
	{
		return 0 ;
	}
	if ( ( block[2] & 0xFFF80000 ) != 0x00400000 )
	{
		return 0 ;
	}
#endif

#if defined(PCBX12D) || defined(PCBX10)
	if ( ( block[0] & 0xFFFC0000 ) != 0x20000000 )
	{
		return 0 ;
	}

	if ( ( block[1] & 0xFFF00000 ) != 0x08000000 )
	{
		return 0 ;
	}
#endif



	bytes = (uint8_t *)block ;
	for ( i = 0 ; i < 1018 ; i+= 1 )
	{
		if ( bytes[i] == 'B' )
		{
			if ( bytes[i+1] == 'O' )
			{
				if ( bytes[i+2] == 'O' )
				{
					if ( bytes[i+3] == 'T' )
					{
						return ( bytes[i+4] << 8 ) + bytes[i+5] ;
					}
				}
			}
		}
	}
	return 0 ;
}

struct fileControl FileControl = {	0,0,0,0, {0,0,0,0} } ;

FRESULT readBinDir( DIR *dj, FILINFO *fno, struct fileControl *fc )
{
	FRESULT fr ;
	uint32_t loop ;

	do
	{
		loop = 0 ;
		fr = f_readdir ( dj, fno ) ;		// First entry

		if ( fr != FR_OK || fno->fname[0] == 0 )
		{
			break ;
		}
		if ( *fno->lfname == 0 )
		{
			cpystr( (uint8_t *)fno->lfname, (uint8_t *)fno->fname ) ;		// Copy 8.3 name
		}
		if ( fc->ext[0] )
		{
			int32_t len = strlen(fno->lfname) - 4 ;
			if ( fc->ext[3] )
			{
				len -= 1 ;			
			}
			if ( len < 0 )
			{
				loop = 1 ;
			}
			if ( fno->lfname[len] != '.' )
			{
				loop = 1 ;
			}
			if ( ( fno->lfname[len+1] & ~0x20 ) != fc->ext[0] )
			{
				loop = 1 ;
			}
			if ( ( fno->lfname[len+2] & ~0x20 ) != fc->ext[1] )
			{
				loop = 1 ;
			}
			if ( ( fno->lfname[len+3] & ~0x20 ) != fc->ext[2] )
			{
				loop = 1 ;
			}
			if ( fc->ext[3] )
			{
				if ( ( fno->lfname[len+4] & ~0x20 ) != fc->ext[3] )
				{
					loop = 1 ;
				}
			}
		}
		else // looking for a Directory
		{
			if ( ( fno->fattrib & AM_DIR ) == 0 )
			{
				loop = 1 ;
			}
		}	
	} while ( loop ) ;
	return fr ;
}

DIR Djp ;
extern uint8_t VoiceFileType ;
#define VOICE_FILE_TYPE_MUSIC	2

uint32_t fillNames( uint32_t index, struct fileControl *fc )
{
	uint32_t i ;
	FRESULT fr ;
	SharedMemory.FileList.Finfo.lfname = SharedMemory.FileList.Filenames[0] ;
	SharedMemory.FileList.Finfo.lfsize = 48 ;
	WatchdogTimeout = 300 ;		// 3 seconds
	DIR *pDj = &SharedMemory.FileList.Dj ;	
	if ( VoiceFileType == VOICE_FILE_TYPE_MUSIC )
	{
#ifdef WHERE_TRACK
	notePosition('m') ;
#endif
		pDj = &Djp ;
	}
	fr = f_readdir ( pDj, 0 ) ;					// rewind
	fr = f_readdir ( pDj, &SharedMemory.FileList.Finfo ) ;		// Skip .
	fr = f_readdir ( pDj, &SharedMemory.FileList.Finfo ) ;		// Skip ..
	i = 0 ;
#ifdef WHERE_TRACK
	notePosition('n') ;
#endif
	while ( i <= index )
	{
		WatchdogTimeout = 300 ;		// 3 seconds
		fr = readBinDir( pDj, &SharedMemory.FileList.Finfo, fc ) ;		// First entry
		FileSize[0] = SharedMemory.FileList.Finfo.fsize ;
		i += 1 ;
		if ( fr != FR_OK || SharedMemory.FileList.Finfo.fname[0] == 0 )
		{
			return 0 ;
		}
	}
#ifdef WHERE_TRACK
	notePosition('o') ;
#endif
	for ( i = 1 ; i < 7 ; i += 1 )
	{
		WatchdogTimeout = 300 ;		// 3 seconds
#ifdef WHERE_TRACK
	notePosition('p') ;
#endif
		SharedMemory.FileList.Finfo.lfname = SharedMemory.FileList.Filenames[i] ;
		fr = readBinDir( pDj, &SharedMemory.FileList.Finfo, fc ) ;		// First entry
		FileSize[i] = SharedMemory.FileList.Finfo.fsize ;
		if ( fr != FR_OK || SharedMemory.FileList.Finfo.fname[0] == 0 )
		{
			break ;
		}
	}
#ifdef WHERE_TRACK
	notePosition('q') ;
#endif
	return i ;
}

extern char PlayListNames[][MUSIC_NAME_LENGTH+2] ;
extern uint16_t PlayListCount ;

uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext )
{
	uint32_t i ;
	FRESULT fr ;
	SharedMemory.FileList.Finfo.lfsize = 19 ;
	char filename[20] ;
	WatchdogTimeout = 300 ;		// 3 seconds

	fr = f_chdir( dir ) ;
	if ( fr == FR_OK )
	{
		fc->index = 0 ;
		fr = f_opendir( &Djp, (TCHAR *) "." ) ;
		if ( fr == FR_OK )
		{
			fc->ext[0] = *ext++ ;
			fc->ext[1] = *ext++ ;
			fc->ext[2] = *ext++ ;
			fc->ext[3] = *ext ;
			fc->index = 0 ;

			fr = f_readdir ( &Djp, 0 ) ;					// rewind
			fr = f_readdir ( &Djp, &SharedMemory.FileList.Finfo ) ;		// Skip .
			fr = f_readdir ( &Djp, &SharedMemory.FileList.Finfo ) ;		// Skip ..
			for ( i = 0 ; i < PLAYLIST_COUNT ; i += 1 )
			{
				WatchdogTimeout = 300 ;		// 3 seconds
				SharedMemory.FileList.Finfo.lfname = filename ;
				fr = readBinDir( &Djp, &SharedMemory.FileList.Finfo, fc ) ;		// First entry
				if ( fr != FR_OK || SharedMemory.FileList.Finfo.fname[0] == 0 )
				{
					break ;
				}
				copyFileName( PlayListNames[i], filename, MUSIC_NAME_LENGTH ) ;

			}
			PlayListCount = i ;
			return i ;
		}
	}
	return 0 ;
}

#define DISPLAY_CHAR_WIDTH	21

uint16_t LastFileMoveTime ;

uint32_t fileList(uint8_t event, struct fileControl *fc )
{
	uint32_t limit ;
	uint32_t result = 0 ;
  uint8_t maxhsize ;
	uint32_t i ;
			 
	limit = 6 ;
	if ( fc->nameCount < limit )
	{
		limit = fc->nameCount ;						
	}
	maxhsize = 0 ;
	for ( i = 0 ; i < limit ; i += 1 )
	{
		uint32_t x ;
		uint32_t len ;
		len = x = strlen( SharedMemory.FileList.Filenames[i] ) ;
		if ( x > maxhsize )
		{
			maxhsize = x ;							
		}
		if ( x > DISPLAY_CHAR_WIDTH )
		{
			if ( ( fc->hpos + DISPLAY_CHAR_WIDTH ) > x )
			{
				x = x - DISPLAY_CHAR_WIDTH ;
			}
			else
			{
				x = fc->hpos ;
			}
			len = DISPLAY_CHAR_WIDTH ;
		}
		else
		{
			x = 0 ;
		}
#ifdef WHERE_TRACK
	notePosition('i') ;
#endif
		lcd_putsn_P( 0, 16+FH*i, &SharedMemory.FileList.Filenames[i][x], len ) ;
	}

#if !defined(PCBTARANIS) || defined(REV9E)
	if ( event == 0 )
	{
extern int32_t Rotary_diff ;
		if ( Rotary_diff > 0 )
		{
			event = EVT_KEY_FIRST(KEY_DOWN) ;
		}
		else if ( Rotary_diff < 0 )
		{
			event = EVT_KEY_FIRST(KEY_UP) ;
		}
		Rotary_diff = 0 ;
	}
#endif

	if ( ( event == EVT_KEY_REPT(KEY_DOWN) ) || event == EVT_KEY_FIRST(KEY_DOWN) )
	{
		uint16_t now ;
		now = get_tmr10ms() ;
	  if((uint16_t)( now-LastFileMoveTime) > 4) // 50mS
		{
			LastFileMoveTime = now ;
			if ( fc->vpos < limit-1 )
			{
				fc->vpos += 1 ;
			}
			else
			{
				if ( fc->nameCount > limit )
				{
					fc->index += 1 ;
//		CoSchedLock() ;
#ifdef WHERE_TRACK
	notePosition('j') ;
#endif
					fc->nameCount = fillNames( fc->index, fc ) ;
//  	CoSchedUnlock() ;
				}
			}
		}
	}
	if ( ( event == EVT_KEY_REPT(KEY_UP)) || ( event == EVT_KEY_FIRST(KEY_UP) ) )
	{
		uint16_t now ;
		now = get_tmr10ms() ;
	  if((uint16_t)( now-LastFileMoveTime) > 4) // 50mS
		{
			LastFileMoveTime = now ;
			if ( fc->vpos > 0 )
			{
				fc->vpos -= 1 ;
			}
			else
			{
				if ( fc->index )
				{
					fc->index -= 1 ;
//		CoSchedLock() ;
#ifdef WHERE_TRACK
	notePosition('k') ;
#endif
					fc->nameCount = fillNames( fc->index, fc ) ;
//  	CoSchedUnlock() ;
				}
			}
		}
	}
	if ( ( event == EVT_KEY_REPT(MKEY_RIGHT)) || ( event == EVT_KEY_FIRST(MKEY_RIGHT) ) )
	{
		if ( fc->hpos + DISPLAY_CHAR_WIDTH < maxhsize )	fc->hpos += 1 ;
	}
	if ( ( event == EVT_KEY_REPT(MKEY_LEFT)) || ( event == EVT_KEY_FIRST(MKEY_LEFT) ) )
	{
		if ( fc->hpos )	fc->hpos -= 1 ;
	}
	if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
	{
		// Select file to flash
		killEvents(event);
		result = 1 ;
	}
	if ( checkForExitEncoderLong( event ) )
	{
		// Select file to flash
		result = 2 ;
	}
	if ( event == EVT_KEY_BREAK(KEY_MENU) )
	{
		// Tag file
		result = 3 ;
	}
#if defined(PCBTARANIS)
	lcd_filled_rect( 0, 2*FH+FH*fc->vpos, DISPLAY_CHAR_WIDTH*FW, 8, 0xFF, 0 ) ;
#else
	lcd_char_inverse( 0, 2*FH+FH*fc->vpos, DISPLAY_CHAR_WIDTH*FW, 0 ) ;
#endif
#ifdef WHERE_TRACK
	notePosition('l') ;
#endif
	return result ;
}

//uint8_t CoProcReady ;

#if !defined(PCBTARANIS)
void lcd_putsnAtt0(uint8_t x,uint8_t y, const char * s,uint8_t len,uint8_t mode)
{
	register char c ;
  while(len!=0) {
    c = *s++ ;
		if ( c == 0 )
		{
			break ;			
		}
    x = lcd_putcAtt(x,y,c,mode);
    len--;
  }
}
#endif

// Values in state
#define UPDATE_NO_FILES		0
#define UPDATE_FILE_LIST	1
#define UPDATE_CONFIRM		2
#define UPDATE_SELECTED		3
#define UPDATE_INVALID		4
#define UPDATE_ACTION			5
#define UPDATE_COMPLETE		6
#define UPDATE_PCHIP_ACTION			7

#define CHANGE_SCANNING		0
#define CHANGE_ENTER_ID		1
#define CHANGE_SET_IDLE		2
#define CHANGE_SET_VALUE	3
#define CHANGE_FINISHED		4

static uint8_t TxPacket[8] ;
static uint8_t TxPhyPacket[16] ;
static uint8_t SportTimer ;

uint8_t SendCount ;
uint8_t RxCount ;
uint8_t RxLastCount ;
uint8_t IdIndex ;
uint8_t IdFound ;

//#ifdef REVX
//uint8_t RxOkPacket[10] ;
//#endif
uint8_t RxPacket[10] ;
uint8_t PhyId ;
uint8_t NewPhyId ;
uint16_t AppId ;

#ifndef SMALL
void setCrc()
{
  uint16_t crc = 0 ;
  for ( uint8_t i=2; i<9; i++)
	{
  	crc += TxPhyPacket[i]; //0-1FF
  	crc += crc >> 8; //0-100
  	crc &= 0x00ff;
  }
	TxPhyPacket[9] = ~crc ;
}

void menuChangeId(uint8_t event)
{
	static uint32_t state ;
 	
	TITLE( "CHANGE SPort Id" ) ;

//	lcd_puts_Pleft( 2*FH, "Not Implemented(yet)" ) ;

	switch(event)
	{
    case EVT_ENTRY:
			RxPacket[1] = 0 ;			
			RxCount = 0 ;
			RxLastCount = 0 ;
			FrskyTelemetryType = FRSKY_TEL_SPORT ;
			IdIndex = 0x1B ;
			IdFound = 0 ;
			state = CHANGE_SCANNING ;
			SendCount = 2 ;
#if defined(PCBX9D) || defined(PCB9XT)
			EXTERNAL_RF_ON() ;
#endif
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
			SPORT_RF_ON() ;
#endif
    break ;
    
		case EVT_KEY_LONG(BTN_RE) :
			if ( g_eeGeneral.disableBtnLong )
			{
				break ;
			}		
		case EVT_KEY_FIRST(KEY_EXIT):
     	chainMenu(menuUpdate) ;
   		killEvents(event) ;
#if defined(PCBX9D) || defined(PCB9XT)
			EXTERNAL_RF_OFF() ;
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
			SPORT_RF_OFF() ;
#endif
#endif
    break ;
		
		case EVT_KEY_FIRST(KEY_UP):
			if ( state == CHANGE_ENTER_ID )
			{
				if ( SetByEncoder )
				{
					if ( NewPhyId  )
					{
						NewPhyId -= 1 ;
					}
				}
				else
				{
					if ( ++NewPhyId > 0x1B )
					{
						NewPhyId = 0x1B ;
					}
				}
			}
    break ;
    
		case EVT_KEY_FIRST(KEY_DOWN):
			if ( state == CHANGE_ENTER_ID )
			{
				if ( SetByEncoder )
				{
					if ( ++NewPhyId > 0x1B )
					{
						NewPhyId = 0x1B ;
					}
				}
				else
				{
					if ( NewPhyId  )
					{
						NewPhyId -= 1 ;
					}
				}
			}
    break ;

		case EVT_KEY_LONG(KEY_MENU):
			if ( ( state == CHANGE_SCANNING ) && IdFound )
			{
				state = CHANGE_ENTER_ID ;
			}
    break ;
		
		case EVT_KEY_FIRST(KEY_MENU):
			if ( state == CHANGE_ENTER_ID )
			{
				TxPhyPacket[0] = 0x7E ;
				TxPhyPacket[1] = 0x1C ;
				TxPhyPacket[2] = 0x21 ;
				TxPhyPacket[3] = 0xFF ;
				TxPhyPacket[4] = 0xFF ;
				TxPhyPacket[5] = 0 ;
				TxPhyPacket[6] = 0 ;
				TxPhyPacket[7] = 0 ;
				TxPhyPacket[8] = 0 ;

				setCrc() ;
#if defined(PCBX9D) || defined(PCB9XT)
				x9dSPortTxStart( TxPhyPacket, 10, NO_RECEIVE ) ;
#endif
#ifdef PCBSKY
				txPdcUsart( TxPhyPacket, 10, NO_RECEIVE ) ;
#endif
				state = CHANGE_SET_IDLE ;
				SendCount = 100 ;
			}
    break ;
	}

	switch ( state )
	{
		case CHANGE_SCANNING :
			lcd_puts_Pleft( 3*FH, "Scanning" ) ;
			if ( --SendCount == 0 )
			{
				SendCount = 2 ;
				if ( ++IdIndex > 27 )
				{
					IdIndex = 0 ;
				}
				TxPhyPacket[0] = 0x7E ;
				TxPhyPacket[1] = SportIds[IdIndex] ;
#if defined(PCBX9D) || defined(PCB9XT)
				x9dSPortTxStart( TxPhyPacket, 2, WITH_RECEIVE ) ;
#endif
#ifdef PCBSKY
				txPdcUsart( TxPhyPacket, 2, WITH_RECEIVE ) ;
#endif
			}
		break ;
	
		case CHANGE_ENTER_ID :
			lcd_puts_Pleft( 3*FH, "New Id: " ) ;
	    lcd_outdez( 9*FW, 3*FH, NewPhyId ) ;
		break ;

		case CHANGE_SET_IDLE :
			lcd_puts_Pleft( 3*FH, "Set Idle state" ) ;
			if ( --SendCount == 0)
			{
				TxPhyPacket[0] = 0x7E ;
				TxPhyPacket[1] = 0x1C ;
				TxPhyPacket[2] = 0x31 ;
				TxPhyPacket[3] = AppId ;
				TxPhyPacket[4] = AppId >> 8 ;
				TxPhyPacket[5] = 1 ;
				TxPhyPacket[6] = NewPhyId ;
				TxPhyPacket[7] = 0 ;
				TxPhyPacket[8] = 0 ;

				setCrc() ;
#if defined(PCBX9D) || defined(PCB9XT)
				x9dSPortTxStart( TxPhyPacket, 10, NO_RECEIVE ) ;
#endif
#ifdef PCBSKY
				txPdcUsart( TxPhyPacket, 10, NO_RECEIVE ) ;
#endif
				state = CHANGE_SET_VALUE ;
				SendCount = 100 ;
			}
		break ;
		
		case CHANGE_SET_VALUE :
			lcd_puts_Pleft( 3*FH, "Set Active state" ) ;
			if ( --SendCount == 0)
			{
				TxPhyPacket[0] = 0x7E ;
				TxPhyPacket[1] = 0x1C ;
				TxPhyPacket[2] = 0x20 ;
				TxPhyPacket[3] = 0xFF ;
				TxPhyPacket[4] = 0xFF ;
				TxPhyPacket[5] = 0 ;
				TxPhyPacket[6] = 0 ;
				TxPhyPacket[7] = 0 ;
				TxPhyPacket[8] = 0 ;

				setCrc() ;
#if defined(PCBX9D) || defined(PCB9XT)
				x9dSPortTxStart( TxPhyPacket, 10, NO_RECEIVE ) ;
#endif
#ifdef PCBSKY
				txPdcUsart( TxPhyPacket, 10, NO_RECEIVE ) ;
#endif
				state = CHANGE_FINISHED ;
				SendCount = 150 ;

			}
		break ;
		
		case CHANGE_FINISHED :
			lcd_puts_Pleft( 3*FH, "Id Changed" ) ;
			PhyId = NewPhyId ;
			if ( --SendCount == 0)
			{
				state = CHANGE_SCANNING ;
				IdFound = 0 ;
				RxPacket[1] = 0 ;			
				RxCount = 0 ;
				RxLastCount = 0 ;
				SendCount = 2 ;
				IdIndex = 0x1B ;
			}
		break ;
	}

//	lcd_outhex4( 0, 7*FH, RxCount ) ;
	if ( RxPacket[1] == 0x10 )
	{
		if ( IdFound == 0 )
		{
			IdFound = 1 ;
			PhyId = RxPacket[0] ;
			NewPhyId = PhyId & 0x1F ;
			if ( NewPhyId > 0x1B )
			{
				NewPhyId = 0x1B ;
			}
		}
		AppId = RxPacket[2] | ( RxPacket[3] << 8 ) ;
//		lcd_outhex4( 0, 4*FH, RxPacket[0] ) ;
//		lcd_outhex4( 25, 4*FH, RxPacket[1] ) ;
//		lcd_outhex4( 50, 4*FH, RxPacket[2] ) ;
//		lcd_outhex4( 75, 4*FH, RxPacket[3] ) ;
//		lcd_outhex4( 100, 4*FH, RxPacket[4] ) ;
//		lcd_outhex4( 0, 5*FH, RxPacket[5] ) ;
//		lcd_outhex4( 25, 5*FH, RxPacket[6] ) ;
//		lcd_outhex4( 50, 5*FH, RxPacket[7] ) ;
//		lcd_outhex4( 75, 5*FH, RxPacket[8] ) ;
//		lcd_outhex4( 100, 5*FH, RxPacket[9] ) ;
	}
	if ( IdFound )
	{
		lcd_puts_Pleft( 6*FH, "Id    AppId" ) ;
		lcd_outdez( 5*FW, 6*FH, PhyId & 0x1F ) ;
		lcd_outhex4( 12*FW, 6*FH, AppId ) ;
	}

}
#endif

 #ifndef SMALL
void displayXmegaData()
{
	lcd_outhex4( 0, 7*FH, (XmegaSignature[0] << 8) | XmegaSignature[1] ) ;
	lcd_outhex4( 25, 7*FH, (XmegaSignature[2] << 8) | XmegaSignature[3] ) ;
	lcd_outhex4( 50, 7*FH, ( Fuses[1] << 8 ) | Fuses[2] ) ;
	lcd_outhex4( 75, 7*FH, ( Fuses[4] << 8 ) | Fuses[5] ) ;
	lcd_outhex4( 100, 7*FH, Fuses[7] ) ;
}
 #endif

#ifndef NO_MULTI
void menuUpMulti(uint8_t event)
{
	TITLE( "Multi Options" ) ;
	static MState2 mstate2 ;
#if defined(PCBSKY) || defined(PCB9XT)
	mstate2.check_columns(event, 4 ) ;
#else
	mstate2.check_columns(event, 1 ) ;
#endif
	uint32_t sub = mstate2.m_posVert ;
	uint32_t subN = 0 ;
	uint32_t y = FH ;
  lcd_putsAtt(0, y, XPSTR("Update"), (sub==subN) ? INVERS : 0 ) ;
  if(sub==subN)
	{
		if ( ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
		{
			popMenu() ;
			chainMenu(menuUp1) ;
		}
	}
	y += FH ;
	subN += 1 ;
	lcd_puts_Pleft( y, XPSTR("File Type") ) ;
	FileType = checkIndexed( y, XPSTR("\110\001\003BINHEX"), FileType, (sub==subN) ) ;
	y += FH ;
	subN += 1 ;
#if defined(PCBSKY) || defined(PCB9XT)
	lcd_puts_Pleft( y, XPSTR("Module") ) ;
	MultiModule = checkIndexed( y, XPSTR("\110\001\010ExternalInternal"), MultiModule, (sub==subN) ) ;
	y += FH ;
	subN += 1 ;
	lcd_puts_Pleft( y, XPSTR("COM Port") ) ;
	MultiPort = checkIndexed( y, XPSTR("\150\001\00112"), MultiPort, (sub==subN) ) ;
	y += FH ;
	subN += 1 ;
	lcd_puts_Pleft( y, XPSTR("Invert Com Port") ) ;
	MultiInvert = checkIndexed( y, XPSTR("\150\001\003 NOYES"), MultiInvert, (sub==subN) ) ;
#endif
}
#endif

#if defined(PCBX9LITE)
// Code to update PMU chip

uint16_t UpdateCrc ;

void drawProgressScreen( const char *title, const char *message, uint32_t percent)
{
  lcd_clear() ;
  if (title)
	{
    lcd_puts_Pleft( 2*FH, title ) ;
  }
  if (message)
	{
    lcd_puts_Pleft( 5*FH, message ) ;
  }
	
	lcd_hbar( 4, 6*FH+4, 101, 7, percent ) ;
  
	refreshDisplay() ;
}

void telemetryPortSetDirectionOutput()
{
#ifdef PCB9XT
	GPIOB->BSRRL = 0x0004 ;		// output enable
#else
 #if defined(PCBXLITE) || defined(REV19)
	GPIOD->BSRRH = 0x0010 ;		// output enable
 #else
  #ifdef PCBX9LITE
	x9LiteSportOn() ;
//	GPIOD->BSRRH = 0x0010 ;		// output enable
  #else
	GPIOD->BSRRL = 0x0010 ;		// output enable
  #endif
 #endif
#endif
	USART2->CR1 &= ~USART_CR1_RE ;  // turn off receiver
}

static void sportWaitTransmissionComplete()
{
  while (!(USART2->SR & USART_SR_TC))
	{
		// wait
	} ;
}

void telemetryPortSetDirectionInput()
{
  sportWaitTransmissionComplete();
#ifdef PCB9XT
	GPIOB->BSRRH = 0x0004 ;		// output disable
#else
 #if defined(PCBXLITE) || defined(REV19)
	GPIOD->BSRRL = PIN_SPORT_ON ;		// output disable
 #else
  #ifdef PCBX9LITE
	x9LiteSportOff() ;
  #else
	GPIOD->BSRRH = PIN_SPORT_ON ;		// output disable
 #endif
 #endif
#endif
	USART2->CR1 |= USART_CR1_RE ;
}


void sportSendByte(uint8_t byte)
{
  while (!(USART2->SR & USART_SR_TXE))
	{
		// wait
	} ;
	USART2->DR = byte ;
}

void sendByte(uint8_t byte )
{
  sportSendByte(byte) ;
	UpdateCrc ^= byte ;
}

// returns ststus byte of 0xFFFF if no answer
uint32_t waitAnswer()
{
	uint32_t i ;
	uint16_t byte ;

  telemetryPortSetDirectionInput() ;

  uint8_t buffer[12] ;

  for ( i = 0 ; i < sizeof(buffer) ; i += 1 )
	{
    uint32_t retry = 0 ;
    while(1)
		{
			byte = get_fifo128( &Com1_fifo ) ;
			if ( byte != 0xFFFF )
			{
				buffer[i] = byte ;
//      if (telemetryGetByte(&buffer[i]))
        if ((i == 0 && buffer[0] != 0x7F) ||
            (i == 1 && buffer[1] != 0xFE) ||
            (i == 10 && buffer[10] != 0x0D) ||
            (i == 11 && buffer[11] != 0x0A))
				{
          i = 0 ;
          continue ;
        }
        break ;
      }
      if (++retry == 20000)
			{
        return 0xFFFF ;
      }
			CoTickDelay(1) ;
		 	wdt_reset() ;
    }
  }
  return buffer[8] ;
}


const char *sendUpgradeCommand(char command, uint32_t packetsCount)
{
	uint32_t i ;
  uint16_t status ;
  
	telemetryPortSetDirectionOutput() ;

  // Head
  sendByte(0x7F) ;
  sendByte(0xFE) ;

  UpdateCrc = 0 ;
  // Addr
  sendByte(0xFA) ;

  // Cmd
  sendByte(command) ;

  // Packets count
  sendByte(packetsCount >> 8) ;
  sendByte(packetsCount) ;

  // Len
  sendByte('E' == command ? 0x00 : 0x0C) ;
  sendByte(0x40) ;

  // Data
  for ( i = 0 ; i < 0x40 ; i += 1 )
	{
    sendByte('E' == command ? 0xF7 : 0x7F) ;
	}

  // Checksum
  sendByte(UpdateCrc) ;

  // Tail
  sendByte(0x0D) ;
  sendByte(0x0A) ;
  
	status = waitAnswer() ;

  if (status == 0xFFFF)
	{
		return "No answer" ;
	}

  return status == 0x00 ? 0 : "Upgrade failed" ;
}

const char *sendUpgradeData(uint32_t index, uint8_t *data )
{
	uint16_t status ;
  uint32_t i ;

	telemetryPortSetDirectionOutput() ;

  // Head
  sendByte(0x7F) ;
  sendByte(0xFE) ;

  UpdateCrc = 0 ;
  
	// Addr
  sendByte(0xFA) ;

  // Cmd
  sendByte('W') ;

  // Packets count
  sendByte(index >> 8) ;
  sendByte(index) ;

  // Len
  sendByte(0x00) ;
  sendByte(0x40) ;

  // Data
  for ( i = 0 ; i < 0x40 ; i += 1 )
	{
    sendByte(*data++) ;
	}

  // Checksum
  sendByte(UpdateCrc) ;

  // Tail
  sendByte(0x0D) ;
  sendByte(0x0A) ;


  status = waitAnswer() ;

  if (status == 0xFFFF)
	{
		return "No answer" ;
	}

  return status == 0x00 ? 0 : "Upgrade failed";
}

const char *startBootloader()
{
  uint16_t status ;
	uint32_t i ;

  telemetryPortSetDirectionOutput() ;

  sportSendByte(0x01) ;

  for (  i = 0 ; i < 30 ; i+=1 )
	{
    sportSendByte(0x7E) ;
	}

  for ( i = 0 ; i < 100 ; i+=1 )
	{
		CoTickDelay(10) ;
	 	wdt_reset() ;
    sportSendByte(0x7F) ;
  }

  sportSendByte(0xFA) ;

	drawProgressScreen( "Waiting", "", 0 ) ;
  status = waitAnswer() ;

  if (status == 0xFFFF)
	{
		return "No answer" ;
	}

  return status == 0x08 ? 0 : "Bootloader failed";
}

PACK(struct FrSkyFirmwareInformation
{
  uint32_t fourcc;
  uint8_t headerVersion;
  uint8_t firmwareVersionMajor;
  uint8_t firmwareVersionMinor;
  uint8_t firmwareVersionRevision;
  uint32_t size;
  uint8_t productFamily;
  uint8_t productId;
  uint16_t crc;
});

const char *ResultText ;

const char *flashPMUchip()
{
  const char *result ;
  uint8_t buffer[64] ;
  UINT count ;
	uint32_t packetsCount ;
	uint32_t index ;
	struct t_maintenance *mdata = &SharedMemory.Mdata ;

	drawProgressScreen( "Starting", "", 0 ) ;
	com1_Configure( 57600, SERIAL_NORM, 0 ) ;
	ResultText = 0 ;

	for ( index = 0 ; index < 100 ; index += 1 )
	{
		CoTickDelay(5) ;
	 	wdt_reset() ;
	}

  result = startBootloader() ;
  if (result)
	{
    return result ;
	}

	if ( f_open( &mdata->FlashFile, mdata->FlashFilename, FA_READ ) != FR_OK)
	{
    return "Error opening file";
  }

  FrSkyFirmwareInformation *information = (FrSkyFirmwareInformation *)buffer ;
  if (f_read(&mdata->FlashFile, buffer, sizeof(FrSkyFirmwareInformation), &count) != FR_OK || count != sizeof(FrSkyFirmwareInformation))
	{
    f_close(&mdata->FlashFile);
    return "Format error";
  }

  packetsCount = (information->size + sizeof(buffer) - 1) / sizeof(buffer) ;
	drawProgressScreen( "Flashing", "File", 0 ) ;

  result = sendUpgradeCommand('A', packetsCount) ;
  if (result)
	{
    return result ;
	}

  index = 0 ;

  while (1)
	{
	 	wdt_reset() ;
		drawProgressScreen( "Flashing", "File", index * 100 / packetsCount ) ;
    if (f_read(&mdata->FlashFile, buffer, sizeof(buffer), &count) != FR_OK)
		{
      f_close(&mdata->FlashFile);
      return "Error reading file";
    }
    result = sendUpgradeData(index + 1, buffer) ;
    if (result)
		{
      return result ;
		}
    if (++index == packetsCount)
		{
      break ;
		}
  }

  f_close(&mdata->FlashFile) ;

  return sendUpgradeCommand('E', packetsCount) ;
}





























#endif

//#if defined(PCBX9LITE)
//void menuUpPchip(uint8_t event)
//{
//	TITLE( "Options" ) ;
//	static MState2 mstate2 ;
//	mstate2.check_columns(event, 1 ) ;
//	uint32_t sub = mstate2.m_posVert ;
//	uint32_t subN = 0 ;
//	uint32_t y = FH ;
//  lcd_putsAtt(0, y, XPSTR("Update"), (sub==subN) ? INVERS : 0 ) ;
//  if(sub==subN)
//	{
//		if ( ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
//		{
//			popMenu() ;
//			chainMenu(menuUp1) ;
//		}
//	}
//	y += FH ;
//	subN += 1 ;
//	lcd_puts_Pleft( y, XPSTR("File Type") ) ;
//	FileType = checkIndexed( y, XPSTR("\110\001\004 BINFSRK"), FileType, (sub==subN) ) ;
//}
//#endif

void menuUp1(uint8_t event)
{
	FRESULT fr ;
	struct fileControl *fc = &FileControl ;
  static uint8_t mounted = 0 ;
	static uint32_t state ;
	static uint32_t firmwareAddress ;
	uint32_t i ;
	uint32_t width ;
	struct t_maintenance *mdata = &SharedMemory.Mdata ;
   
	wdt_reset() ;
	if ( WatchdogTimeout < 50 )
	{
 		WatchdogTimeout = 50 ;
	}
	 
	if (mdata->UpdateItem == UPDATE_TYPE_BOOTLOADER )		// Bootloader
	{
  	TITLE( "UPDATE BOOT" ) ;
	}
#ifndef NO_MULTI
	else if (SharedMemory.Mdata.UpdateItem == UPDATE_TYPE_MULTI )
	{
  	TITLE( "UPDATE MULTI" ) ;
	}
#endif
	else
	{
#ifdef PCB9XT
 		if (mdata->UpdateItem == UPDATE_TYPE_SPORT_EXT )
		{
  		TITLE( "UPDATE Ext. SPort" ) ;
		}
		else if (mdata->UpdateItem == UPDATE_TYPE_SPORT_INT )
		{
  		TITLE( "UPDATE Int. Module" ) ;
		}
		else
		{
  		TITLE( "UPDATE AVR" ) ;
		}
#endif
		
#ifdef PCBX9D
		if (mdata->UpdateItem == UPDATE_TYPE_SPORT_INT )
		{
  		TITLE( "UPDATE Int. Module" ) ;
		}
 		else if (mdata->UpdateItem == UPDATE_TYPE_XMEGA )
		{
  		TITLE( "UPDATE XMEGA" ) ;
		}
		else
		{
#if defined(PCBXLITE) || defined(PCBX9LITE)
#if defined(PCBX9LITE)
	 		if (mdata->UpdateItem == UPDATE_TYPE_PCHIP )
			{
  			TITLE( "UPDATE POWER CHIP" ) ;
			}
			else
#endif
			{
  			TITLE( (SportModuleExt == SPORT_MODULE) ? "UPDATE Ext. Module" : "UPDATE Ext. SPort" ) ;
			}
#else
  		TITLE( "UPDATE Ext. SPort" ) ;
#endif
		}
#endif

#ifdef PCBSKY
 #ifndef REVX
// 		if (mdata->UpdateItem == UPDATE_TYPE_COPROCESSOR )
//		{
//  		TITLE( "UPDATE COPROC" ) ;
//		}
//		else
		{
  		TITLE( "UPDATE SPort" ) ;
		}
 #else
 		if (mdata->UpdateItem == UPDATE_TYPE_SPORT_EXT )
		{
  		TITLE( "UPDATE SPort" ) ;
		}
 #endif
#endif
#ifdef PCBSKY
#ifndef SMALL
 		if (mdata->UpdateItem == UPDATE_TYPE_XMEGA )
		{
  		TITLE( "UPDATE XMEGA" ) ;
		}
#endif
#endif
#ifdef PCB9XT
 		if (mdata->UpdateItem == UPDATE_TYPE_XMEGA )
		{
  		TITLE( "UPDATE XMEGA" ) ;
		}
#endif
	}
	switch(event)
	{
    case EVT_ENTRY:
  		WatchdogTimeout = 200 ;
			state = UPDATE_NO_FILES ;
			if ( mounted == 0 )
			{
#if defined(PCBTARANIS)
  			fr = f_mount(0, &g_FATFS) ;
#else				
  			fr = f_mount(0, &g_FATFS) ;
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
				unlockFlash() ;
#endif
			}
			else
			{
				fr = FR_OK ;
			}
			if ( fr == FR_OK)
			{
				mounted = 1 ;
			}
			if ( mounted )
			{
				fr = f_chdir( (TCHAR *)"\\firmware" ) ;
				if ( fr == FR_OK )
				{
					state = UPDATE_NO_FILES ;
					fc->index = 0 ;
					fr = f_opendir( &SharedMemory.FileList.Dj, (TCHAR *) "." ) ;
					if ( fr == FR_OK )
					{
						fc->ext[3] = 0 ;
 						if ( (mdata->UpdateItem == UPDATE_TYPE_SPORT_INT ) || (mdata->UpdateItem == UPDATE_TYPE_SPORT_EXT ) )
						{
							fc->ext[0] = 'F' ;
							fc->ext[1] = 'R' ;
							fc->ext[2] = 'K' ;
						}
						else
						{
#ifndef NO_MULTI
							if ( (mdata->UpdateItem == UPDATE_TYPE_MULTI ) && FileType )
							{
								fc->ext[0] = 'H' ;
								fc->ext[1] = 'E' ;
								fc->ext[2] = 'X' ;
							}
							else
#endif
							{
#if defined(PCBX9LITE)
								if ( (mdata->UpdateItem == UPDATE_TYPE_PCHIP ) )//&& FileType )
								{
									fc->ext[0] = 'F' ;
									fc->ext[1] = 'R' ;
									fc->ext[2] = 'S' ;
									fc->ext[3] = 'K' ;
								}
								else
#endif
								{
									fc->ext[0] = 'B' ;
									fc->ext[1] = 'I' ;
									fc->ext[2] = 'N' ;
								}
							}
						}
						fc->index = 0 ;
						fc->nameCount = fillNames( 0, fc ) ;
						fc->hpos = 0 ;
						fc->vpos = 0 ;
						if ( fc->nameCount )
						{
							state = UPDATE_FILE_LIST ;
						}
					}
				}
			}
    break ;
    
		case EVT_KEY_LONG(BTN_RE) :
			if ( g_eeGeneral.disableBtnLong )
			{
				break ;
			}		
		case EVT_KEY_FIRST(KEY_EXIT):
			if ( state < UPDATE_ACTION )
			{
      	chainMenu(menuUpdate) ;
    		killEvents(event) ;
			}
    break ;
	}

	switch ( state )
	{
		case UPDATE_NO_FILES :
			lcd_puts_Pleft( 4*FH, "\005No Files" ) ;
	    lcd_outdez( 21*FW, 4*FH, mounted ) ;
    break ;
		
		case UPDATE_FILE_LIST :
			mdata->SportVerValid = 0 ;
			if ( fileList( event, &FileControl ) == 1 )
			{
				state = UPDATE_CONFIRM ;
			}
    break ;
		case UPDATE_CONFIRM :
 			if ( (mdata->UpdateItem > UPDATE_TYPE_BOOTLOADER ) )
			{
#ifdef PCBX9D
 				if ( (mdata->UpdateItem == UPDATE_TYPE_SPORT_INT ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Int. Mod from" ) ;
				}
 				else if ( (mdata->UpdateItem == UPDATE_TYPE_XMEGA ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Xmega from" ) ;
				}
				else if ( (mdata->UpdateItem == UPDATE_TYPE_MULTI ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Multi from" ) ;
				}
#if defined(PCBX9LITE)
				else if ( (mdata->UpdateItem == UPDATE_TYPE_PCHIP ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Power Chip from" ) ;
				}
#endif
				else
				{
#if defined(PCBXLITE) || defined(PCBX9LITE)
					lcd_puts_Pleft( 2*FH, (SportModuleExt == SPORT_MODULE) ? "Flash Ext.mod from" : "Flash Ext.SP from" ) ;
#else
					lcd_puts_Pleft( 2*FH, "Flash Ext.SP from" ) ;
#endif
				}
				mdata->SportVerValid = 0 ;
#else
 #ifndef REVX
// 				if ( (mdata->UpdateItem == UPDATE_TYPE_COPROCESSOR ) )
//				{
//					lcd_puts_Pleft( 2*FH, "Flash Co-Proc. from" ) ;
//				}
// 				else
 #ifndef SMALL
				if ( (mdata->UpdateItem == UPDATE_TYPE_XMEGA ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Xmega from" ) ;
				}
  #ifndef NO_MULTI
				else if ( (mdata->UpdateItem == UPDATE_TYPE_MULTI ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Multi from" ) ;
				}
	#endif
				else
#endif
				{
					lcd_puts_Pleft( 2*FH, "Flash SPort from" ) ;
				}
//				CoProcReady = 0 ;
 #else
 				if ( (mdata->UpdateItem == UPDATE_TYPE_XMEGA ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Xmega from" ) ;
				}
				else if ( (SharedMemory.Mdata.UpdateItem == UPDATE_TYPE_MULTI ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Multi from" ) ;
				}
				else
				{
					lcd_puts_Pleft( 2*FH, "Flash SPort from" ) ;
				}
 #endif
#endif
#ifdef PCB9XT
		 		if (mdata->UpdateItem == UPDATE_TYPE_SPORT_EXT )
				{
					lcd_puts_Pleft( 2*FH, "Flash Ext.SP from" ) ;
					mdata->SportVerValid = 0 ;
				}
 				else if ( (mdata->UpdateItem == UPDATE_TYPE_SPORT_INT ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Int. Mod from" ) ;
					mdata->SportVerValid = 0 ;
				}
				else if ( (mdata->UpdateItem == UPDATE_TYPE_XMEGA ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Xmega from" ) ;
				}
				else if ( (mdata->UpdateItem == UPDATE_TYPE_MULTI ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Multi from" ) ;
				}
				else
				{
					// AVR
					lcd_puts_Pleft( 2*FH, "Flash AVR from" ) ;
				}
#endif
#if defined(PCBX12D) || defined(PCBX10)
		 		if (mdata->UpdateItem == UPDATE_TYPE_SPORT_EXT )
				{
					lcd_puts_Pleft( 2*FH, "Flash Ext.SP from" ) ;
					mdata->SportVerValid = 0 ;
				}
// 				else if ( (mdata->UpdateItem == UPDATE_TYPE_SPORT_INT ) )
//				{
//					lcd_puts_Pleft( 2*FH, "Flash Int. XJT from" ) ;
//					mdata->SportVerValid = 0 ;
//				}
				else if ( (mdata->UpdateItem == UPDATE_TYPE_MULTI ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Multi from" ) ;
				}
#endif
			}
			else
			{
				lcd_puts_Pleft( 2*FH, "Flash Bootloader from" ) ;
			}
			cpystr( cpystr( (uint8_t *)mdata->FlashFilename, (uint8_t *)"\\firmware\\" ), (uint8_t *)SharedMemory.FileList.Filenames[fc->vpos] ) ;
#if defined(PCBTARANIS)
			lcd_putsnAtt( 0, 4*FH, SharedMemory.FileList.Filenames[fc->vpos], DISPLAY_CHAR_WIDTH, 0 ) ;
#else
			lcd_putsnAtt0( 0, 4*FH, SharedMemory.FileList.Filenames[fc->vpos], DISPLAY_CHAR_WIDTH, 0 ) ;
#endif
			switch ( event )
			{
    		case EVT_KEY_LONG(KEY_MENU):
				case EVT_KEY_BREAK(BTN_RE):
					state = UPDATE_SELECTED ;
    		break ;

				case EVT_KEY_LONG(BTN_RE):
					if ( g_eeGeneral.disableBtnLong )
					{
						break ;
 					}		
    		case EVT_KEY_LONG(KEY_EXIT):
					state = UPDATE_FILE_LIST ;		// Canceled
    		break;
			}
    break ;
		case UPDATE_SELECTED :
			f_open( &mdata->FlashFile, mdata->FlashFilename, FA_READ ) ;
			f_read( &mdata->FlashFile, (BYTE *)FileData, 1024, &mdata->BlockCount ) ;
			i = 1 ;
			if (mdata->UpdateItem == UPDATE_TYPE_BOOTLOADER )		// Bootloader
			{
				i = validateFile( (uint32_t *) FileData ) ;
			}
			if ( i == 0 )
			{
				state = UPDATE_INVALID ;
			}
			else
			{
				if (mdata->UpdateItem == UPDATE_TYPE_BOOTLOADER )		// Bootloader
				{
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
					firmwareAddress = 0x08000000 ;
#endif
#ifdef PCBSKY
					firmwareAddress = 0x00400000 ;
#endif
				}
#ifdef PCBSKY
 #ifndef REVX
//				else if (mdata->UpdateItem == UPDATE_TYPE_COPROCESSOR )		// Bootloader
//				{
//					firmwareAddress = 0x00000080 ;
//					if ( check_ready() == 0 )
//					{
//						CoProcReady = 1 ;
//					}
//				}
 #endif
 // REVX
 #ifndef SMALL
				else if (mdata->UpdateItem == UPDATE_TYPE_XMEGA )
				{
					FirmwareSize = FileSize[fc->vpos] ;
					firmwareAddress = 0x00000000 ;
					XmegaState = XMEGA_START ;
#if defined(PCBX9D) || defined(PCB9XT)
					PdiErrors0 = 0 ;
					PdiErrors1 = 0 ;
					PdiErrors2 = 0 ;
					PdiErrors3 = 0 ;
					PdiErrors4 = 0 ;
					PdiErrors5 = 0 ;
					PdiErrors6 = 0 ;
					PdiErrors7 = 0 ;
					PdiErrors8 = 0 ;
#endif
				}
 #endif
#endif
#ifdef PCB9XT
				else if (mdata->UpdateItem == UPDATE_TYPE_AVR )		// Bootloader
				{
					width = 50 ;	// Not used
				}
				else if (mdata->UpdateItem == UPDATE_TYPE_XMEGA )
				{
					FirmwareSize = FileSize[fc->vpos] ;
					firmwareAddress = 0x00000000 ;
					EXTERNAL_RF_ON() ;
					XmegaState = XMEGA_START ;
				}
#endif
#ifdef PCBX9D
				else if (mdata->UpdateItem == UPDATE_TYPE_XMEGA )
				{
					FirmwareSize = FileSize[fc->vpos] ;
					firmwareAddress = 0x00000000 ;
					XmegaState = XMEGA_START ;
				}
#endif
#ifndef NO_MULTI
				else if (mdata->UpdateItem == UPDATE_TYPE_MULTI )
				{
					FirmwareSize = FileSize[fc->vpos] ;
					firmwareAddress = 0x00000000 ;
					MultiState = MULTI_START ;
					MultiResult = 0 ;
				}
#endif
				else
				{
// SPort update
					mdata->SportState = SPORT_START ;
					FirmwareSize = FileSize[fc->vpos] ;
					mdata->BlockInUse = 0 ;
					f_read( &mdata->FlashFile, (BYTE *)ExtraFileData, 1024, &mdata->XblockCount ) ;
				}
				BytesFlashed = 0 ;
				BlockOffset = 0 ;
				ByteEnd = 1024 ;
				state = UPDATE_ACTION ;
			}
#if defined(PCBX9LITE)
			if (mdata->UpdateItem == UPDATE_TYPE_PCHIP )
			{
				state = UPDATE_PCHIP_ACTION ;
			}
#endif
    break ;
#if defined(PCBX9LITE)
		case UPDATE_PCHIP_ACTION :
	 		{
				ResultText = flashPMUchip() ;
				getEvent() ;
			}
			MultiResult = ResultText ? 1 : 0 ;
			state = UPDATE_COMPLETE ;
		break ;
#endif

		case UPDATE_INVALID :
			lcd_puts_Pleft( 2*FH, "Invalid File" ) ;
			lcd_puts_Pleft( 4*FH, "Press EXIT" ) ;
			if ( checkForExitEncoderLong( event ) )
			{
				state = UPDATE_FILE_LIST ;		// Canceled
    		killEvents(event) ;
			}
    break ;
		case UPDATE_ACTION :
			// Do the flashing
			lcd_puts_Pleft( 3*FH, "Flashing" ) ;
			if (mdata->UpdateItem == UPDATE_TYPE_BOOTLOADER )		// Bootloader
			{
#if defined(PCBX12D) || defined(PCBX10)
				width = ByteEnd >> 11 ;
#else
				width = ByteEnd >> 9 ;
#endif
				if ( BytesFlashed < ByteEnd )
				{
					program( (uint32_t *)firmwareAddress, &((uint32_t *)FileData)[BlockOffset] ) ;	// size is 256 bytes
#ifdef PCBSKY
					if ( ChipId & 0x0080 )
					{
						BlockOffset += 128 ;		// 32-bit words (256 bytes)
						firmwareAddress += 512 ;
						BytesFlashed += 512;
					}
					else
#endif
					{
						BlockOffset += 64 ;		// 32-bit words (256 bytes)
						firmwareAddress += 256 ;
						BytesFlashed += 256 ;
					}	 
				}
				else
				{
#if defined(PCBX12D) || defined(PCBX10)
					if ( ByteEnd >= 32768 * 4 )
#else
#ifdef SMALL
					if ( ByteEnd >= 32768-4096 )
#else
					if ( ByteEnd >= 32768 )
#endif
#endif
					{
						state = UPDATE_COMPLETE ;
					}
					else
					{
						f_read( &mdata->FlashFile, (BYTE *)FileData, 1024, &mdata->BlockCount ) ;
						ByteEnd += 1024 ;
						BlockOffset = 0 ;
					}
				}
			}

#ifdef PCBSKY
 #ifndef REVX
//			else if (mdata->UpdateItem == UPDATE_TYPE_COPROCESSOR )		// CoProcessor
//			{
//				uint32_t size = FileSize[fc->vpos] ;
//				width = BytesFlashed * 64 / size ;
//				CoProresult = 0 ;
//				if ( CoProcReady )
//				{
//					if ( BytesFlashed < ByteEnd )
//					{
//						uint32_t number ;

//						number = ByteEnd - BytesFlashed ;
//						if ( number > 128 )
//						{
//							number = 128 ;						
//						}
//						i = write_CoProc( (uint32_t)firmwareAddress, &FileData[BlockOffset], number ) ;
//						BlockOffset += 128 ;		// 8-bit bytes
//						firmwareAddress += 128 ;
//						BytesFlashed += 128 ;
//					}
//					else
//					{
//						if ( ByteEnd >= size )
//						{
//							state = UPDATE_COMPLETE ;
//						}
//						else
//						{
//							f_read( &mdata->FlashFile, (BYTE *)FileData, 1024, &mdata->BlockCount ) ;
//							ByteEnd += mdata->BlockCount ;
//							BlockOffset = 0 ;
//						}
//					}
//				}
//				else
//				{
//					CoProresult = 1 ;
//					state = UPDATE_COMPLETE ;
//				}
//			}
 #endif
 // REVX
 #ifndef SMALL
			else if (mdata->UpdateItem == UPDATE_TYPE_XMEGA )
			{
//				uint32_t size = FileSize[fc->vpos] ;
				width = xmegaUpdate() ;
				if ( width > FirmwareSize )
				{
					state = UPDATE_COMPLETE ;
				}
				width *= 64 ;
				width /= FirmwareSize ;
				displayXmegaData() ;
			}
 #endif
#endif
			
#ifdef PCB9XT
			else if (mdata->UpdateItem == UPDATE_TYPE_AVR )
			{
				// No more display, never return, check for power off when finished
				width = 0 ;
			}
			else if (mdata->UpdateItem == UPDATE_TYPE_XMEGA )
			{
//				uint32_t size = FileSize[fc->vpos] ;
				width = xmegaUpdate() ;
				if ( width > FirmwareSize )
				{
					state = UPDATE_COMPLETE ;
				}
				width *= 64 ;
				width /= FirmwareSize ;
				displayXmegaData() ;
				lcd_outhex4( 50, 7*FH, (PdiErrors0 << 8 ) | PdiErrors1 ) ;
				lcd_outhex4( 75, 7*FH, (PdiErrors2 << 8 ) | PdiErrors3 ) ;
				lcd_outhex4( 100, 7*FH, (PdiErrors4 << 8 ) | PdiErrors5 ) ;
				lcd_outhex4( 0, 1*FH, (PdiErrors6 << 8 ) | PdiErrors7 ) ;
				lcd_outhex4( 25, 1*FH, PdiErrors8 ) ;
			}
#endif
#ifdef PCBX9D
			else if (mdata->UpdateItem == UPDATE_TYPE_XMEGA )
			{
//				uint32_t size = FileSize[fc->vpos] ;
				width = xmegaUpdate() ;
				if ( width > FirmwareSize )
				{
					state = UPDATE_COMPLETE ;
				}
				width *= 64 ;
				width /= FirmwareSize ;
				displayXmegaData() ;
			}
#endif
#ifndef NO_MULTI
			else if (mdata->UpdateItem == UPDATE_TYPE_MULTI )
			{
				if ( MultiState == MULTI_WAIT1	)
				{
					lcd_puts_Pleft( 4*FH, "Power on Delay" ) ;
				}
				width = multiUpdate() ;
				if ( width > FirmwareSize )
				{
					state = UPDATE_COMPLETE ;
					stopMultiMode() ;
				}
				width *= 64 ;
				width /= FirmwareSize ;
				lcd_outhex4( 0, 7*FH, (XmegaSignature[0] << 8) | XmegaSignature[1] ) ;
				lcd_outhex4( 25, 7*FH, (XmegaSignature[2] << 8) | XmegaSignature[3] ) ;
			}
#endif
			else		// Internal/External Sport
			{
#if defined(PCBX9D) || defined(PCB9XT)
				width = sportUpdate( (mdata->UpdateItem == UPDATE_TYPE_SPORT_INT) ? SPORT_INTERNAL : SPORT_EXTERNAL ) ;
#else
				width = sportUpdate( SPORT_EXTERNAL ) ;
#endif
				if ( width > FirmwareSize )
				{
					state = UPDATE_COMPLETE ;
					mdata->SportVerValid = width ;
				}
				width *= 64 ;
				width /= FirmwareSize ;
				if ( mdata->SportVerValid )
				{
					lcd_outhex4( 0, 7*FH, (SportVersion[0] << 8) | SportVersion[1] ) ;
					lcd_outhex4( 25, 7*FH, (SportVersion[2] << 8) | SportVersion[3] ) ;
				}

				if ( mdata->SportState == SPORT_POWER_ON )
				{
					lcd_puts_Pleft( 4*FH, "Finding Device" ) ;
				}

//#ifdef REVX
//				for ( i = 0 ; i < 9 ; i += 1 )
//				{
//					lcd_outhex2(i*11, 1*FH, RxOkPacket[i] ) ;
//					lcd_outhex2(i*11, 2*FH, RxPacket[i] ) ;
//				}
//#endif

				if ( checkForExitEncoderLong( event ) )
				{
					state = UPDATE_COMPLETE ;
					mdata->SportVerValid = 0x00FF ; // Abort with failed
					mdata->SportState = SPORT_IDLE ;
				}
			}	
			lcd_hline( 0, 5*FH-1, 65 ) ;
			lcd_hline( 0, 6*FH, 65 ) ;
			lcd_vline( 64, 5*FH, 8 ) ;
			for ( i = 0 ; i <= width ; i += 1 )
			{
				lcd_vline( i, 5*FH, 8 ) ;
			}
//			lcd_outhex4( 0, 4*FH, BytesFlashed >> 16 ) ;
//			lcd_outhex4( 30, 4*FH, BytesFlashed  ) ;
    break ;
		
		case UPDATE_COMPLETE :
			lcd_puts_Pleft( 3*FH, "Flashing Complete" ) ;
 			if ( (mdata->UpdateItem != UPDATE_TYPE_BOOTLOADER ) )
 			{
#if defined(PCBX9D) || defined(PCB9XT)
 				if ( mdata->SportVerValid & 1 )
 				{
 					lcd_puts_Pleft( 5*FH, "FAILED" ) ;
 				}
				if ( mdata->SportVerValid )
				{
					lcd_outhex4( 0, 7*FH, (SportVersion[0] << 8) | SportVersion[1] ) ;
					lcd_outhex4( 25, 7*FH, (SportVersion[2] << 8) | SportVersion[3] ) ;
				}
#endif
#ifndef NO_MULTI
				if (mdata->UpdateItem == UPDATE_TYPE_MULTI )
				{
					if ( MultiResult )
					{
						lcd_puts_Pleft( 5*FH, "FAILED" ) ;
					}
				lcd_outhex4( 0, 7*FH, (XmegaSignature[0] << 8) | XmegaSignature[1] ) ;
				lcd_outhex4( 25, 7*FH, (XmegaSignature[2] << 8) | XmegaSignature[3] ) ;
				}
#endif
 
#if defined(PCBX9LITE)
				if (mdata->UpdateItem == UPDATE_TYPE_PCHIP )
				{
					if ( MultiResult )
					{
						lcd_puts_Pleft( 5*FH, "FAILED" ) ;
						if ( ResultText )
						{
							lcd_puts_Pleft( 6*FH, ResultText ) ;
						}
					}
				}
#endif
 
 #ifndef SMALL
				if (mdata->UpdateItem == UPDATE_TYPE_XMEGA )
				{
					displayXmegaData() ;
				}
 #endif
#ifdef PCBSKY
 #ifndef REVX
//				if (mdata->UpdateItem == UPDATE_TYPE_COPROCESSOR )		// CoProcessor
//				{
//					if ( CoProresult )
//					{
//						lcd_puts_Pleft( 5*FH, "FAILED" ) ;
//					}
//				}
//				else
				{
 #endif 			
 					if ( mdata->SportVerValid & 1 )
	 				{
 						lcd_puts_Pleft( 5*FH, "FAILED" ) ;
 					}
  #ifndef NO_MULTI
					if (SharedMemory.Mdata.UpdateItem == UPDATE_TYPE_MULTI )
					{
						lcd_outhex4( 0, 7*FH, (XmegaSignature[0] << 8) | XmegaSignature[1] ) ;
						lcd_outhex4( 25, 7*FH, (XmegaSignature[2] << 8) | XmegaSignature[3] ) ;
					}
	#endif
  #ifndef REVX
			  }
  #endif 			
 #endif 			
			}
#ifdef PCB9XT
				displayXmegaData() ;
				lcd_outhex4( 50, 7*FH, (PdiErrors0 << 8 ) | PdiErrors1 ) ;
				lcd_outhex4( 75, 7*FH, (PdiErrors2 << 8 ) | PdiErrors3 ) ;
				lcd_outhex4( 100, 7*FH, (PdiErrors4 << 8 ) | PdiErrors5 ) ;
				lcd_outhex4( 0, 1*FH, (PdiErrors6 << 8 ) | PdiErrors7 ) ;
				lcd_outhex4( 25, 1*FH, PdiErrors8 ) ;
#endif

			if ( checkForExitEncoderLong( event ) )
			{
#if defined(PCBX9D) || defined(PCB9XT)
				EXTERNAL_RF_OFF();
				INTERNAL_RF_OFF();
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
				SPORT_RF_OFF() ;
#endif
#endif
				state = UPDATE_FILE_LIST ;
    		killEvents(event) ;
			}
    break ;
	}
}

void menuUpdate(uint8_t event)
{
	static uint8_t reboot = 0 ;
	static uint32_t position = 2*FH ;
  TITLE( "MAINTENANCE" ) ;
	lcd_puts_Pleft( 2*FH, "  Update Bootloader" );
#ifdef PCBSKY
 #ifndef REVX
//	lcd_puts_Pleft( 3*FH, "  Update CoProcessor" );
	lcd_puts_Pleft( 3*FH, "  Update SPort" );
  #ifdef SMALL
//	lcd_puts_Pleft( 4*FH, "  Update Xmega" );
	#else
	lcd_puts_Pleft( 4*FH, "  Change SPort Id" );
	lcd_puts_Pleft( 5*FH, "  Update Xmega" );
	#endif
  #ifndef NO_MULTI
	 lcd_puts_Pleft( 6*FH, "  Update Multi" );
	#endif
 #else
	lcd_puts_Pleft( 3*FH, "  Update SPort" );
	lcd_puts_Pleft( 4*FH, "  Change SPort Id" );
	lcd_puts_Pleft( 5*FH, "  Update Xmega" );
	lcd_puts_Pleft( 6*FH, "  Update Multi" );
 #endif
#endif
#ifdef PCBX9D
#if defined(PCBXLITE) || defined(PCBX9LITE)
	lcd_puts_Pleft( 3*FH, "  Update Int. Module" );
	lcd_puts_Pleft( 4*FH, "  Update Ext. Module" );
	lcd_puts_Pleft( 5*FH, "  Update Ext. SPort" );
#if defined(PCBX9LITE)
	lcd_puts_Pleft( 6*FH, "  Update Power Chip" );
#else
	lcd_puts_Pleft( 6*FH, "  Change SPort Id" );
#endif
	lcd_puts_Pleft( 7*FH, "  Update Multi" );
#else
	lcd_puts_Pleft( 3*FH, "  Update Int. Module" );
	lcd_puts_Pleft( 4*FH, "  Update Ext. SPort" );
	lcd_puts_Pleft( 5*FH, "  Change SPort Id" );
	lcd_puts_Pleft( 6*FH, "  Update Xmega" );
	lcd_puts_Pleft( 7*FH, "  Update Multi" );
#endif
#endif
#ifdef PCB9XT
	lcd_puts_Pleft( 3*FH, "  Update Ext. SPort" );
	lcd_puts_Pleft( 4*FH, "  Update Int. Module" );
	lcd_puts_Pleft( 5*FH, "  Change SPort Id" );
//	lcd_puts_Pleft( 5*FH, "  Update Avr CPU" );
	lcd_puts_Pleft( 6*FH, "  Update Xmega" );
	lcd_puts_Pleft( 7*FH, "  Update Multi" );
#endif
#if defined(PCBX12D) || defined(PCBX10)
//	lcd_puts_Pleft( 2*FH, "  Update Int. Module" );
	lcd_puts_Pleft( 3*FH, "  Update Ext. SPort" );
	lcd_puts_Pleft( 4*FH, "  Change SPort Id" );
	lcd_puts_Pleft( 5*FH, "  Update Ext. Multi" );
#endif

  switch(event)
	{
    case EVT_ENTRY:
			position = 2*FH ;
    break ;
    
		case EVT_KEY_FIRST(KEY_MENU):
//#ifdef PCBX7
		case EVT_KEY_BREAK(BTN_RE):
//#endif
			if ( position == 2*FH )
			{
				
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_BOOTLOADER ;
	      chainMenu(menuUp1) ;
			}
#ifdef PCBSKY
 #ifndef REVX
//			if ( position == 3*FH )
//			{
//				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_COPROCESSOR ;
//	      chainMenu(menuUp1) ;
//			}
			if ( position == 3*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_EXT ;
	      chainMenu(menuUp1) ;
			}
  #ifdef SMALL
//			if ( position == 4*FH )
//			{
//				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_XMEGA ;
//	      chainMenu(menuUp1) ;
//			}
	#else
			if ( position == 4*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_CHANGE_ID ;
	      chainMenu(menuChangeId) ;
			}
			if ( position == 5*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_XMEGA ;
	      chainMenu(menuUp1) ;
			}
	#endif
  #ifndef NO_MULTI
			if ( position == 6*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_MULTI ;
	      pushMenu(menuUpMulti) ;
			}
	#endif
 #else
			if ( position == 3*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_EXT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 4*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_CHANGE_ID ;
	      chainMenu(menuChangeId) ;
			}
			if ( position == 5*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_XMEGA ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 6*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_MULTI ;
	      pushMenu(menuUpMulti) ;
			}
 #endif
#endif
#ifdef PCBX9D
 #if defined(PCBXLITE) || defined(PCBX9LITE)
			if ( position == 3*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_INT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 4*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_EXT ;
				SportModuleExt = SPORT_MODULE ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 5*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_EXT ;
				SportModuleExt = SPORT_EXT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 6*FH )
			{
#if defined(PCBX9LITE)
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_PCHIP ;
	      chainMenu(menuUp1) ;
//	      pushMenu(menuUpPchip) ;
#else
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_CHANGE_ID ;
	      chainMenu(menuChangeId) ;
#endif
			}
			if ( position == 7*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_MULTI ;
	      pushMenu(menuUpMulti) ;
			}
 #else
			if ( position == 3*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_INT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 4*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_EXT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 5*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_CHANGE_ID ;
	      chainMenu(menuChangeId) ;
			}
			if ( position == 6*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_XMEGA ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 7*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_MULTI ;
	      pushMenu(menuUpMulti) ;
			}
 #endif
#endif
#ifdef PCB9XT
			if ( position == 3*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_EXT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 4*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_INT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 5*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_CHANGE_ID ;
//				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_AVR ;
	      chainMenu(menuChangeId) ;
			}
			if ( position == 6*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_XMEGA ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 7*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_MULTI ;
	      pushMenu(menuUpMulti) ;
			}
#endif
#if defined(PCBX12D) || defined(PCBX10)
//			if ( position == 2*FH )
//			{
//				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_INT ;
//	      chainMenu(menuUp1) ;
//			}
			if ( position == 3*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_SPORT_EXT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 4*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_CHANGE_ID ;
//				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_AVR ;
	      chainMenu(menuChangeId) ;
			}
			if ( position == 5*FH )
			{
				SharedMemory.Mdata.UpdateItem = UPDATE_TYPE_MULTI ;
	      pushMenu(menuUpMulti) ;
			}
#endif
    	killEvents(event) ;
			reboot = 0 ;
    break ;

#if defined(PCBX7) || defined(PCBX9LITE) || defined(REV19)
    case EVT_KEY_LONG(KEY_EXIT):
#else
    case EVT_KEY_LONG(KEY_EXIT):
		case EVT_KEY_LONG(BTN_RE) :
#endif
			reboot = 1 ;
		break ;

#ifdef PCBSKY
 #ifndef REVX
    case EVT_KEY_FIRST(KEY_DOWN):
  #ifndef NO_MULTI
			if ( position < 6*FH )
	#else
   #ifdef SMALL
			if ( position < 4*FH )
	 #else
			if ( position < 5*FH )
	 #endif
	#endif
			{
				position += FH ;				
			}
		break ;
    
		case EVT_KEY_FIRST(KEY_UP):
			if ( position > 2*FH )
			{
				position -= FH ;				
			}
		break ;
 #else
    case EVT_KEY_FIRST(KEY_DOWN):
			if ( position < 6*FH )
			{
				position += FH ;				
			}
		break ;
    
		case EVT_KEY_FIRST(KEY_UP):
			if ( position > 2*FH )
			{
				position -= FH ;				
			}
		break ;
 #endif
#endif
#ifdef PCBX9D
    case EVT_KEY_FIRST(KEY_DOWN):
 #if defined(PCBXLITE) || defined(PCBX9LITE)
			if ( position < 7*FH )
#else
			if ( position < 7*FH )
#endif
			{
				position += FH ;				
			}
		break ;
    
		case EVT_KEY_FIRST(KEY_UP):
			if ( position > 2*FH )
			{
				position -= FH ;				
			}
		break ;
#endif
#ifdef PCB9XT
    case EVT_KEY_FIRST(KEY_DOWN):
//			if ( position < 4*FH )
			if ( position < 7*FH )
			{
				position += FH ;				
			}
		break ;
    
		case EVT_KEY_FIRST(KEY_UP):
			if ( position > 2*FH )
			{
				position -= FH ;				
			}
		break ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
    case EVT_KEY_FIRST(KEY_DOWN):
//			if ( position < 4*FH )
			if ( position < 5*FH )
			{
				position += FH ;				
			}
		break ;
    
		case EVT_KEY_FIRST(KEY_UP):
			if ( position > 2*FH )
			{
				position -= FH ;				
			}
		break ;
#endif
	}

	if ( reboot )
	{
		if ( (~read_keys() & 0x7E) == 0 )
		{
		  NVIC_SystemReset() ;
		}
	}
#if defined(PCBTARANIS)
	lcd_filled_rect( 2*FW, position, 18*FW, 8, 0xFF, 0 ) ;
#else
	lcd_char_inverse( 2*FW, position, 18*FW, 0 ) ;
#endif
}



#ifdef PCBSKY

void init_mtwi()
{
	register Pio *pioptr ;
	register uint32_t timing ;
  
	PMC->PMC_PCER0 |= 0x00080000L ;		// Enable peripheral clock to TWI0
	
	TWI0->TWI_CR = TWI_CR_SWRST ;				// Reset in case we are restarting

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
#ifdef REVX
	TWI0->TWI_MMR = 0x006F0100 ;		// Device 6F and master is writing, 1 byte addr
#else	
	TWI0->TWI_MMR = 0x00350000 ;		// Device 35 and master is writing
#endif
}

#ifdef REVX
uint32_t clearMfp()
{
	uint32_t i ;
	
	TWI0->TWI_MMR = 0x006F0100 ;		// Device 6F and master is writing, 1 byte addr
	TWI0->TWI_IADR = 7 ;
	TWI0->TWI_THR = 0 ;					// Value for clear
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
#endif

 #ifndef REVX

//uint8_t Addr_buffer[132] ;

//uint32_t read_status_CoProc( uint32_t bufaddr, uint32_t number )
//{
//	uint32_t i ;
	
//	TWI0->TWI_MMR = 0x00351000 ;		// Device 35 and master is reading,
//	TWI0->TWI_RPR = bufaddr ;
//	TWI0->TWI_RCR = number-1 ;
//	if ( TWI0->TWI_SR & TWI_SR_RXRDY )
//	{
//		(void) TWI0->TWI_RHR ;
//	}
//	TWI0->TWI_PTCR = TWI_PTCR_RXTEN ;	// Start transfers
//	TWI0->TWI_CR = TWI_CR_START ;		// Start Rx
	
//	// Now wait for completion
	
//	for ( i = 0 ; i < 1000000 ; i += 1 )
//	{
//		if ( TWI0->TWI_SR & TWI_SR_RXBUFF )
//		{
//			break ;
//		}
//	}
//	// Finished reading
//	TWI0->TWI_PTCR = TWI_PTCR_RXTDIS ;	// Stop transfers
//	TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Rx
//	TWI0->TWI_RCR = 1 ;						// Last byte

//	if ( i >= 1000000 )
//	{
//		return 0 ;
//	}
//	// Now wait for completion
	
//	for ( i = 0 ; i < 100000 ; i += 1 )
//	{
//		if ( TWI0->TWI_SR & TWI_SR_TXCOMP )
//		{
//			break ;
//		}	
//	}
//	if ( i >= 100000 )
//	{
//		return 0 ;
//	}
//	return 1 ;
//}

//#define TWI_CMD_REBOOT							0x55	// TWI Command to restart back in the bootloader

//uint32_t coProcBoot()
//{
//	uint32_t i ;
	
//	TWI0->TWI_MMR = 0x00350000 ;		// Device 35 and master is writing
//	TWI0->TWI_THR = TWI_CMD_REBOOT ;	// Send reboot command
//	TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx

//	for ( i = 0 ; i < 100000 ; i += 1 )
//	{
//		if ( TWI0->TWI_SR & TWI_SR_TXCOMP )
//		{
//			break ;
//		}	
//	}
	
//	if ( i >= 100000 )
//	{
//		return 0 ;
//	}
//	return 1 ;
//}

//uint32_t write_CoProc( uint32_t coInternalAddr, uint8_t *buf, uint32_t number )
//{
//	uint32_t i ;
//	uint8_t *ptr ;

//	Addr_buffer[0] = 0x01 ;		// Command, PROGRAM
//	Addr_buffer[1] = coInternalAddr >> 8 ;
//	Addr_buffer[2] = coInternalAddr ;
//	ptr = buf ;
//	// Copy data
//	for ( i = 0 ; i < number ; i += 1 )
//	{
//		Addr_buffer[i+3] = *ptr++ ;		
//	}
//	// Pad to 128 bytes
//	while ( i < 128 )
//	{
//		Addr_buffer[i+3] = 0xFF ;
//		i += 1 ;	
//	}
//	// Now send TWI data using PDC
//	TWI0->TWI_MMR = 0x00350000 ;		// Device 35 and master is writing
//	// init PDC
//	TWI0->TWI_TPR = (uint32_t)&Addr_buffer[1] ;
//	TWI0->TWI_TCR = 130 ;
			
//	TWI0->TWI_THR = Addr_buffer[0] ;		// Send data
//	// kick off PDC, enable PDC end interrupt
//	TWI0->TWI_PTCR = TWI_PTCR_TXTEN ;	// Start transfers
	
//	// Now wait for completion
	
//	for ( i = 0 ; i < 1000000 ; i += 1 )
//	{
//		if ( TWI0->TWI_SR & TWI_SR_TXBUFE )
//		{
//			break ;
//		}
//	}
//	// Finished reading
//	TWI0->TWI_PTCR = TWI_PTCR_TXTDIS ;	// Stop transfers
//	TWI0->TWI_CR = TWI_CR_STOP ;		// Stop Tx
//	if ( i >= 1000000 )
//	{
//		return 0 ;
//	}
	
//	for ( i = 0 ; i < 100000 ; i += 1 )
//	{
//		if ( TWI0->TWI_SR & TWI_SR_TXCOMP )
//		{
//			break ;
//		}	
//	}
	
//	if ( i >= 100000 )
//	{
//		return 0 ;
//	}

//	return 1 ;
//}

//// returns 0 = OK, not 0 error
//uint32_t check_ready()
//{
//	uint32_t result ;
//	result = 0 ;				// OK
//	init_mtwi() ;

//	// initialise for updating
//	if ( read_status_CoProc( (uint32_t)Twi_rx_buf, 22 ) )
//	{
//		if ( ( Twi_rx_buf[0] & 0x80 ) == 0 )
//		{ // Not in bootloader
//			if ( coProcBoot() == 0 )		// Make sure we are in the bootloader
//			{
//   		  result = 1 ;
//			}
//			else
//			{
//				uint32_t i ;

//				for ( i = 0 ; i < 10000 ; i += 1 )
//				{
//					asm("nop") ;
//				}
//				if ( read_status_CoProc( (uint32_t)Twi_rx_buf, 22 ) )
//				{
//					if ( ( Twi_rx_buf[0] & 0x80 ) != 0x80 )
//					{
//		   		  result = 2 ;
//					}
//				}
//				else
//				{
//   		  	result = 3 ;
//				}	 
//			}
//		}
//	}
//	else
//	{
//   	result = 4 ;
//	}

//	read_status_CoProc( (uint32_t)Twi_rx_buf, 22 ) ;

//	return result ;
//}

 #endif
#endif

#define PRIM_REQ_POWERUP    (0)
#define PRIM_REQ_VERSION    (1)
#define PRIM_CMD_DOWNLOAD   (3)
#define PRIM_DATA_WORD      (4)
#define PRIM_DATA_EOF       (5)

#define PRIM_ACK_POWERUP    (0x80)
#define PRIM_ACK_VERSION    (0x81)
#define PRIM_REQ_DATA_ADDR  (0x82)
#define PRIM_END_DOWNLOAD   (0x83)
#define PRIM_DATA_CRC_ERR   (0x84)


void writePacket( uint8_t *buffer, uint8_t phyId )
{
	uint8_t *ptr = TxPhyPacket ;
	uint32_t i ;

	*ptr++ = 0x7E ;
	*ptr++ = phyId ;
	buffer[7] = crc16_ccitt( buffer, 7 ) ;
	for ( i = 0 ; i < 8 ; i += 1 )
	{
    if(  !( ( buffer[i] == 0x7E) || ( buffer[i] ==0x7D ) ) )
    {
      *ptr++ = buffer[i] ;
    }
		else
    {
      *ptr++ = 0x7D ;
      *ptr++ = 0x20 ^ buffer[i] ;
    }
	}
	i = ptr - TxPhyPacket ;		// Length of buffer to send
#if defined(PCBX9LITE) || defined(REV19)
	if ( SharedMemory.Mdata.UpdateItem == UPDATE_TYPE_SPORT_INT )
	{
extern volatile uint8_t *PxxTxPtr ;
extern volatile uint8_t PxxTxCount ;
		PxxTxPtr = TxPhyPacket ;
		PxxTxCount = i ;
		INTMODULE_USART->CR1 |= USART_CR1_TXEIE ;
	}
	else
	{
		x9dSPortTxStart( TxPhyPacket, i, NO_RECEIVE ) ;
	}
#else
 #if defined(PCBX9D) || defined(PCB9XT)
	x9dSPortTxStart( TxPhyPacket, i, NO_RECEIVE ) ;
 #endif
 #ifdef PCBSKY
	txPdcUsart( TxPhyPacket, i, NO_RECEIVE ) ;
 #endif
#endif
}

void blankTxPacket()
{
	uint32_t i ;
	for ( i = 2 ; i < 8 ; i += 1 )
	{
		TxPacket[i] = 0 ;
	}
}

// This is called from the receive processing.
// Packet has leading 0x7E stripped
void maintenance_receive_packet( uint8_t *packet, uint32_t check )
{
	uint32_t addr ;
	if( ( packet[0] == 0x5E) && ( packet[1]==0x50))
	{
		SportTimer = 0 ;		// stop timer
		switch( packet[2] )
		{
			case PRIM_ACK_POWERUP :
				if ( SharedMemory.Mdata.SportState == SPORT_POWER_ON )
				{
					SportTimer = 2 ;
					SharedMemory.Mdata.SportState = SPORT_VERSION ;
				}
			break ;
        
			case PRIM_ACK_VERSION:
				if ( SharedMemory.Mdata.SportState == SPORT_VERSION )
				{
					SportTimer = 2 ;
					SharedMemory.Mdata.SportState = SPORT_DATA_START ;
					SportVersion[0] = packet[3] ;
					SportVersion[1] = packet[4] ;
					SportVersion[2] = packet[5] ;
					SportVersion[3] = packet[6] ;
					SharedMemory.Mdata.SportVerValid = 1 ;
 				}
			break ;

			case PRIM_REQ_DATA_ADDR :
			{
				UINT bcount ;
				bcount = SharedMemory.Mdata.BlockInUse ? SharedMemory.Mdata.XblockCount : SharedMemory.Mdata.BlockCount ;
				
				if ( BytesFlashed >= FirmwareSize )
				{
					// We have finished
					blankTxPacket() ;
					TxPacket[0] = 0x50 ;
					TxPacket[1] = PRIM_DATA_EOF ;
					SportTimer = 20 ;		// 200 mS
					writePacket( TxPacket, 0xFF ) ;
					SharedMemory.Mdata.SportState = SPORT_END ;
				}
				else
				{				
					if ( bcount )
					{
						uint32_t *ptr ;
						bcount -= 4 ;
						BytesFlashed += 4 ;
						addr = *((uint32_t *)(&packet[3])) ;
						TxPacket[0] = 0x50 ;
						TxPacket[1] = PRIM_DATA_WORD ;
        		TxPacket[6] = addr & 0x000000FF ;
						addr = ( addr & 1023 ) >> 2 ;		// 32 bit word offset into buffer
						ptr = ( uint32_t *) (SharedMemory.Mdata.BlockInUse ? ExtraFileData : FileData ) ;
						ptr += addr ;
						uint32_t *dptr = (uint32_t *)(&TxPacket[2]) ;
        		*dptr = *ptr ;
						SportTimer = 5 ;		// 50 mS
						writePacket( TxPacket, 0xFF ) ;
					}
					if ( SharedMemory.Mdata.BlockInUse )
					{
						SharedMemory.Mdata.XblockCount = bcount ;
					}
					else
					{
						SharedMemory.Mdata.BlockCount = bcount ;
					}
					if ( bcount == 0 )
					{
						SharedMemory.Mdata.SportState = SPORT_DATA_READ ;
						if ( SharedMemory.Mdata.BlockInUse )
						{
							SharedMemory.Mdata.BlockInUse = 0 ;						
						}
						else
						{
							SharedMemory.Mdata.BlockInUse = 1 ;
						}
					}
				}
			}
			break ;

			case PRIM_END_DOWNLOAD :
					SharedMemory.Mdata.SportState = SPORT_COMPLETE ;
			break ;
				
			case PRIM_DATA_CRC_ERR :
					SharedMemory.Mdata.SportState = SPORT_FAIL ;
			break ;
		}

//#ifdef REVX
//		RxOkPacket[0] = packet[0] ;
//		RxOkPacket[1] = packet[1] ;
//		RxOkPacket[2] = packet[2] ;
//		RxOkPacket[3] = packet[3] ;
//		RxOkPacket[4] = packet[4] ;
//		RxOkPacket[5] = packet[5] ;
//		RxOkPacket[6] = packet[6] ;
//		RxOkPacket[7] = packet[7] ;
//		RxOkPacket[8] = packet[8] ;
//		RxOkPacket[9] = packet[9] ;
//#endif
	}
	else
	{
		RxCount += 1 ;
		RxPacket[0] = packet[0] ;
		RxPacket[1] = packet[1] ;
		RxPacket[2] = packet[2] ;
		RxPacket[3] = packet[3] ;
		RxPacket[4] = packet[4] ;
		RxPacket[5] = packet[5] ;
		RxPacket[6] = packet[6] ;
		RxPacket[7] = packet[7] ;
		RxPacket[8] = packet[8] ;
		RxPacket[9] = packet[9] ;
	}
}



#ifndef NO_MULTI
uint32_t eat( uint8_t byte )
{
	uint16_t time ;
	uint16_t rxchar ;

	time = getTmr2MHz() ;
	while ( (uint16_t) (getTmr2MHz() - time) < 25000 )	// 12.5mS
	{
		if ( ( rxchar = getMultiFifo() ) != 0xFFFF )
		{
			if ( rxchar == byte )
			{
				return 1 ;
			}
		}
	}
	return 0 ;
}
#endif

uint32_t hexFileNextByte()
{
	if ( SharedMemory.Mdata.HexFileIndex >= SharedMemory.Mdata.XblockCount )
	{
		if ( SharedMemory.Mdata.XblockCount < 1024 )
		{
			
			return 0 ;
		}
		f_read( &SharedMemory.Mdata.FlashFile, (BYTE *)ExtraFileData, 1024, &SharedMemory.Mdata.XblockCount ) ;
		SharedMemory.Mdata.HexFileRead += SharedMemory.Mdata.XblockCount ;
		SharedMemory.Mdata.HexFileIndex = 0 ;

	}
	return ExtraFileData[SharedMemory.Mdata.HexFileIndex++] ;
}

uint32_t fromHex( uint32_t data )
{
	if ( data > 0x60 )
	{
		data -= 0x20 ;
	}
	data -= '0' ;
	if ( data > 9 )
	{
		data -= 7 ;
	}
	return data ;
}

uint8_t recordSize ;
uint8_t inRecord ;
uint8_t recordOffset ;
uint16_t recordAddress ;
uint8_t recordData[32] ;

#define RECORD_START			0
#define RECORD_LENGTH1	  1
#define RECORD_LENGTH2	  2
#define RECORD_ADD1			  3
#define RECORD_ADD2	  		4
#define RECORD_ADD3	  		5
#define RECORD_ADD4	  		6
#define RECORD_DATA1	  	7
#define RECORD_DATA2	  	8
#define RECORD_DONE		  	9

void hexFileReadRecord()
{
	uint32_t data ;
	uint32_t state = RECORD_START ;
	uint32_t length = 0 ;
	uint32_t byte = 0 ;
	recordOffset = 0 ;
	recordSize = 0 ;

	while ( state != RECORD_DONE )
	{
		data = hexFileNextByte() ;
		if ( data == 0 )
		{
			// End of File
			return ;
		}
		else
		{
			switch ( state )
			{
				case RECORD_START :
					if ( data == ':' )
					{
						state = RECORD_LENGTH1 ;
					}
				break ;
			
				case RECORD_LENGTH1 :
					length = fromHex( data ) ;
					state = RECORD_LENGTH2 ;
				break ;
				case RECORD_LENGTH2 :
					length = (length << 4) + fromHex( data ) ;
					if ( length == 0 )
					{
						return ;
					}
//					state = RECORD_ADD1 ;
//				break ;
//				case RECORD_ADD1 :
////					length = (length << 4) + fromHex( data ) ;
//					state = RECORD_ADD2 ;
//				break ;
//				case RECORD_ADD2 :
////					length = (length << 4) + fromHex( data ) ;
//					state = RECORD_ADD3 ;
//				break ;
//				case RECORD_ADD3 :
////					length = (length << 4) + fromHex( data ) ;
//					state = RECORD_ADD4 ;
//				break ;
//				case RECORD_ADD4 :
////					length = (length << 4) + fromHex( data ) ;
					hexFileNextByte() ;	// skip four characters
					hexFileNextByte() ;	// address
					hexFileNextByte() ;	// address
					hexFileNextByte() ;	// address
					hexFileNextByte() ;	// skip two characters
					hexFileNextByte() ;	// (record type)
					state = RECORD_DATA1 ;
				break ;
				case RECORD_DATA1 :
					byte = fromHex( data ) ;
					state = RECORD_DATA2 ;
				break ;
				case RECORD_DATA2 :
					byte = (byte << 4) + fromHex( data ) ;
					recordData[recordSize++] = byte ;
					length -= 1 ;
					if ( length == 0 )
					{
						return ;
					}
					if ( recordSize >= 32 )
					{
						return ;
					}
					state = RECORD_DATA1 ;
				break ;
			}
		}
	}
}

void hexFileStart()
{
	
	recordSize = 0 ;
	inRecord = 0 ;
	SharedMemory.Mdata.HexFileIndex = 0 ;
	memmove(ExtraFileData, FileData, 1024 ) ;	// Hex data to here
	SharedMemory.Mdata.XblockCount = 1024 ;
	SharedMemory.Mdata.HexFileRead = 1024 ;
}

// Read file data to ExtraFileData, put binary into FileData
void hexFileRead1024( uint32_t address, UINT *blockCount )
{
	
	uint32_t i ;
	i = 0 ;
	while ( i < 1024 )
	{
		if ( inRecord )
		{
			FileData[i++] = recordData[recordOffset++] ;
			if ( recordOffset >= recordSize )
			{
				inRecord = 0 ;
			}
		}
		else
		{
			hexFileReadRecord() ;
			if ( recordSize == 0 )
			{
				*blockCount = i ;
				return ;
			}
			inRecord = 1 ;
		}
	}
	*blockCount = i ;
	return ;
}

#ifndef NO_MULTI
uint32_t multiUpdate()
{
	uint32_t i ;
	uint16_t time ;

	switch ( MultiState )
	{
		case MULTI_IDLE :
		break ;

		case MULTI_START :
			initMultiMode() ;
			MultiStm = 0 ;
			BytesFlashed = 0 ;
		  AllSubState = 0 ;
			if ( FileType )	// Hex file
			{
				hexFileStart() ;
				hexFileRead1024( 0, &SharedMemory.Mdata.BlockCount ) ;
			}
			MultiState = MULTI_WAIT1 ;
		break ;

		case MULTI_WAIT1 :
#if defined(PCBX9D) || defined(PCB9XT)
			if ( ++AllSubState > 120 )	// Allow for power on time
#else
			if ( ++AllSubState > 50 )
#endif
			{
				MultiState = MULTI_WAIT2 ;
				AllSubState = 0 ;
			}
		break ;

		case MULTI_WAIT2 :
		{	
			uint16_t rxchar ;
			while ( ( rxchar = getMultiFifo() ) != 0xFFFF )
			{
				// flush receive fifo
			}
			sendMultiByte( 0x30 ) ;
			sendMultiByte( 0x20 ) ;
			if ( eat(0x14) )
			{
				eat( 0x10 ) ;
				MultiState = MULTI_BEGIN ;
				AllSubState = 0 ;
			}
			else
			{
				if ( ++AllSubState == 80 )
				{
					MultiInvert ^= 1 ;
					initMultiMode() ;
				}
				if ( AllSubState > 160 )
				{
					MultiResult = 1 ;
					MultiState = MULTI_DONE ;
				}
			}
		}
		break ;

		case MULTI_BEGIN :
		{
			uint16_t rxchar ;
			while ( ( rxchar = getMultiFifo() ) != 0xFFFF )
			{
				// flush receive fifo
			}
			XmegaSignature[0] = 0 ;
			sendMultiByte( 0x75 ) ;
			sendMultiByte( 0x20 ) ;
			eat(0x14) ;
			time = getTmr2MHz() ;
			i = 0 ;
			while ( (uint16_t) (getTmr2MHz() - time) < 10000 )	// 5mS
			{
				if ( ( rxchar = getMultiFifo() ) != 0xFFFF )
				{
					XmegaSignature[i++] = rxchar ;
					if ( i > 3 )
					{
						break ;
					}
				}
			}
			if ( XmegaSignature[0] != 0x1E )
			{
				if ( ++AllSubState > 10 )
				{
					MultiResult = 1 ;
					MultiState = MULTI_DONE ;
				}
			}
			else
			{
				MultiState = MULTI_FLASHING ;
				MultiPageSize = 128 ;
				if ( XmegaSignature[2] == 0x42 )
				{
					MultiPageSize = 256 ;
				}
				if ( ( XmegaSignature[1] == 0x55 ) && ( XmegaSignature[2] == 0xAA ) )
				{
					MultiPageSize = 256 ;
					MultiStm = 1 ;
				}
			}
			BlockOffset = 0 ;
		}
		break ;

		case MULTI_FLASHING :
		{	
			uint16_t rxchar ;
			uint32_t addOffset ;
			sendMultiByte( 0x55 ) ;
			addOffset = BytesFlashed ;
			if ( MultiPageSize == 128 )
			{
				addOffset >>= 1 ;
			}
			if ( MultiStm )
			{
				addOffset >>= 1 ;
				addOffset += 0x1000 ;	// Word (16-bit) address offset
			}
			sendMultiByte( addOffset ) ;
			sendMultiByte( addOffset >> 8 ) ;
			sendMultiByte( 0x20 ) ;
			eat(0x14) ;
			eat(0x10) ;
			sendMultiByte( 0x64 ) ;
			sendMultiByte( MultiPageSize >> 8 ) ;
			sendMultiByte( MultiPageSize & 0x00FF ) ;
			sendMultiByte( 0 ) ;
			for ( i = 0 ; i < MultiPageSize ; i += 1 )
			{
				sendMultiByte( FileData[BlockOffset+i] ) ;
			}
			sendMultiByte( 0x20 ) ;
			eat(0x14) ;

			time = getTmr2MHz() ;
			// For the STM module we need at least 30mS
			while ( (uint16_t) (getTmr2MHz() - time) < 40000 )	// 20mS
			{
				if ( ( rxchar = getMultiFifo() ) != 0xFFFF )
				{
					break ;
				}
			}
			if ( rxchar == 0xFFFF )
			{
				time = getTmr2MHz() ;
				// For the STM module we need at least 30mS
				while ( (uint16_t) (getTmr2MHz() - time) < 40000 )	// 20mS
				{
					if ( ( rxchar = getMultiFifo() ) != 0xFFFF )
					{
						break ;
					}
				}
			}

			if ( rxchar != 0x10 )
			{
				MultiResult = 1 ;
				MultiState = MULTI_DONE ;
				break ;
			}
			
			BytesFlashed += MultiPageSize ;
			BlockOffset += MultiPageSize ;
			if ( BytesFlashed >= ByteEnd )
			{
				if ( FileType )	// Hex file
				{
					hexFileRead1024( BytesFlashed, &SharedMemory.Mdata.BlockCount ) ;
				}
				else
				{
					f_read( &SharedMemory.Mdata.FlashFile, (BYTE *)FileData, 1024, &SharedMemory.Mdata.BlockCount ) ;
				}
				BlockOffset = 0 ;
				ByteEnd += SharedMemory.Mdata.BlockCount ;
		 		wdt_reset() ;
			}
			if ( FileType )
			{
				if ( SharedMemory.Mdata.BlockCount == 0 )
				{
					MultiState = MULTI_DONE ;
					BytesFlashed = FirmwareSize ;
				}
			}
			else
			{
				if ( BytesFlashed >= FirmwareSize )
				{
					MultiState = MULTI_DONE ;
					BytesFlashed = FirmwareSize ;
				}
			}
		}
		break ;

		case MULTI_DONE :
			sendMultiByte( 0x51 ) ;	// Exit bootloader
			sendMultiByte( 0x20 ) ;
			eat(0x14) ;
			eat(0x10) ;
			stopMultiMode() ;
			MultiState = MULTI_IDLE ;
//#ifdef PCB9XT
//			EXTERNAL_RF_OFF() ;
//#endif
			SharedMemory.Mdata.HexFileRead = BytesFlashed = FirmwareSize + 1 ;
		break ;
	
	}
	return FileType ? SharedMemory.Mdata.HexFileRead : BytesFlashed ;
}
#endif

#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
// This is called repeatedly every 10mS while update is in progress
 #ifndef SMALL
uint32_t xmegaUpdate()
{
	
	switch ( XmegaState )
	{
		case XMEGA_IDLE :
		break ;

		case XMEGA_START :
			BytesFlashed = 0 ;
		  AllSubState = 0 ;
			XmegaState = XMEGA_POWER ;
		break ;
		
		case XMEGA_POWER :
			BytesFlashed = 0 ;
			if ( ++AllSubState > 5 )
			{
		  	AllSubState = 0 ;
				XmegaState = XMEGA_BEGIN ;
			}
		break ;
		
		case XMEGA_BEGIN :
			__disable_irq() ;
			pdiInit() ;
			pdiReadBlock(XNVM_DATA_BASE+ 0x90, XmegaSignature, 4 ) ;
			pdiReadBlock(0x08F0020, Fuses, 8 ) ;
			if ( XmegaSignature[0] == 0x1E )
			{
				if (Fuses[7] != 0xFF )
				{
					pdiChipErase() ;
				}
				if ( Fuses[2] & 0x40 )
				{ // Boot reset
 					wdt_reset() ;
					pdiWriteFuse( 2, Fuses[2] & ~0x40 ) ;
				}
 				wdt_reset() ;
				pdiWritePageMemory( 0, FileData, 256 ) ;
		 		wdt_reset() ;
				__enable_irq() ;
				BytesFlashed = 256 ;
				BlockOffset = 256 ;
				ByteEnd = 1024 ;
				XmegaState = XMEGA_FLASHING ;

		  	AllSubState = 0 ;
			}
			else
			{
 				wdt_reset() ;
				__enable_irq() ;
				SharedMemory.Mdata.SportVerValid = 0x00FF ; // Abort with failed
				XmegaState = XMEGA_DONE ;
			}
		break ;
		
		case XMEGA_FLASHING :
		  AllSubState += 1 ;
			
			if ( ( AllSubState & 3 ) == 0 )
			{
				__disable_irq() ;
				pdiInit() ;
				pdiWritePageMemory( BytesFlashed, &FileData[BlockOffset], 256 ) ;
		 		wdt_reset() ;
				__enable_irq() ;
				BytesFlashed += 256 ;
				BlockOffset += 256 ;
				if ( BytesFlashed >= ByteEnd )
				{
					f_read( &SharedMemory.Mdata.FlashFile, (BYTE *)FileData, 1024, &SharedMemory.Mdata.BlockCount ) ;
					BlockOffset = 0 ;
					ByteEnd += SharedMemory.Mdata.BlockCount ;
		 			wdt_reset() ;
				}
				if ( BytesFlashed >= FirmwareSize )
				{
					XmegaState = XMEGA_DONE ;
					BytesFlashed = FirmwareSize ;
				}
			}
		break ;
		
		case XMEGA_DONE :
			pdiCleanup() ;
			XmegaState = XMEGA_IDLE ;
#ifdef PCB9XT
			EXTERNAL_RF_OFF() ;
#endif
			BytesFlashed = FirmwareSize + 1 ;
		break ;
	}
	return BytesFlashed ;
}	
 #endif
#endif

// This is called repeatedly every 10mS while update is in progress
uint32_t sportUpdate( uint32_t external )
{
	if ( SportTimer )
	{
		SportTimer -= 1 ;
	}
	switch ( SharedMemory.Mdata.SportState )
	{
		case SPORT_IDLE :
			SportTimer = 0 ;
		break ;
		
		case SPORT_START :
#if !defined(PCBTARANIS)
			 FrskyTelemetryType = FRSKY_TEL_SPORT ;
#endif
#if defined(PCBX9D) || defined(PCB9XT)
#if defined(PCBTARANIS)
			sportInit() ;
#else
#if defined(PCBX9LITE) || defined(REV19)
			if ( external )
			{
				com1_Configure( 57600, SERIAL_NORM, 0 ) ;
			}
			else
			{
  			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ;     // Enable portB clock
				RCC->APB2ENR |= RCC_APB2ENR_USART1EN ;		// Enable clock
				configure_pins( INTMODULE_TX_GPIO_PIN, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB | PIN_PER_7 ) ;
				configure_pins( INTMODULE_RX_GPIO_PIN, PIN_PERIPHERAL | PIN_PORTB | PIN_PER_7 ) ;
				INTMODULE_USART->BRR = PeripheralSpeeds.Peri2_frequency / 57600 ;
				INTMODULE_USART->CR1 = USART_CR1_UE | USART_CR1_TE ;// | USART_CR1_RE ;
				INTMODULE_USART->CR1 |= USART_CR1_RE | USART_CR1_RXNEIE ;
				NVIC_SetPriority( INTMODULE_USART_IRQn, 3 ) ; // Quite high priority interrupt
  			NVIC_EnableIRQ( INTMODULE_USART_IRQn);
			}
#else
			com1_Configure( 57600, SERIAL_NORM, 0 ) ;
#endif
#endif
#endif
#ifdef PCBSKY
 #ifdef REVX
		init_mtwi() ;
		clearMfp() ;
 #endif
			com1_Configure( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ;
//			UART2_Configure( 57600, Master_frequency ) ;
//			startPdcUsartReceive() ;
#endif
			SportTimer = 5 ;		// 50 mS
#if defined(PCBX9D) || defined(PCB9XT)
			if ( external )
			{
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(REV19)
				if (SportModuleExt == SPORT_MODULE)
				{
					EXTERNAL_RF_ON();
				}
				else
				{
					SPORT_RF_ON() ;
				}
#else
  			EXTERNAL_RF_ON();
#endif
			}
			else
			{
//#if /*defined(PCBXLITE) || */ defined(PCBX9LITE)
//				configure_pins( PIN_BOOTCMD, PIN_OUTPUT | PIN_OS25 | PORT_BOOTCMD | PIN_HIGH ) ;
//				GPIO_SetBits( GPIOBOOTCMD, PIN_BOOTCMD ) ;
//#endif
  			INTERNAL_RF_ON();
			}
#endif
			SharedMemory.Mdata.SportState = SPORT_POWER_ON ;
		break ;
		
		case SPORT_POWER_ON :
			if ( SportTimer == 0 )
			{
				blankTxPacket() ;
				TxPacket[0] = 0x50 ;
				TxPacket[1] = PRIM_REQ_POWERUP ;
				SportTimer = 10 ;		// 100 mS
				writePacket( TxPacket, 0xFF ) ;
			}
		break ;
		
		case SPORT_VERSION :
			if ( SportTimer == 0 )
			{
				blankTxPacket() ;
				TxPacket[0] = 0x50 ;
				TxPacket[1] = PRIM_REQ_VERSION ;
				SportTimer = 20 ;		// 200 mS
				SharedMemory.Mdata.SportState = SPORT_VERSION ;
				writePacket( TxPacket, 0xFF ) ;
			}
		break ;

		case SPORT_DATA_START :
			blankTxPacket() ;
			TxPacket[0] = 0x50 ;
			TxPacket[1] = PRIM_CMD_DOWNLOAD ;
			SportTimer = 20 ;		// 200 mS
			SharedMemory.Mdata.SportState = SPORT_DATA ;
// Stop here for testing
			writePacket( TxPacket, 0xFF ) ;
		break ;

		case SPORT_DATA :
//#if defined(PCB9XT) || defined(ACCESS) || defined(REVX)
			if ( SportTimer == 0 )
			{
				SportTimer = 5 ;		// 50 mS
				writePacket( TxPacket, 0xFF ) ;
			}
//#endif
		break ;
		
		case SPORT_DATA_READ :
		{	
			uint32_t *ptr ;
			UINT *pcount ;
			ptr = ( uint32_t *) (SharedMemory.Mdata.BlockInUse ? FileData : ExtraFileData ) ;
			pcount = SharedMemory.Mdata.BlockInUse ? &SharedMemory.Mdata.BlockCount : &SharedMemory.Mdata.XblockCount ;
			f_read( &SharedMemory.Mdata.FlashFile, (BYTE *)ptr, 1024, pcount ) ;
			SharedMemory.Mdata.SportState = SPORT_DATA ;
		}
		break ;
		
		case SPORT_END :
		break ;

		case SPORT_COMPLETE :
			return 0xFFFFFFFE ;
		break ;
				
		case SPORT_FAIL :
			return 0xFFFFFFFF ;
		break ;

	}
	return BytesFlashed ;
}

// This is called as often as possible
void maintenanceBackground()
{
#if !defined(PCBTARANIS)
	// First, deal with any received bytes
#if defined(PCBX9D) || defined(PCB9XT)
#ifdef ACCESS
	if (SharedMemory.Mdata.UpdateItem == UPDATE_TYPE_SPORT_INT )
	{
		int32_t rxbyte ;
		while ( ( rxbyte = get_fifo128( &Access_int_fifo ) ) != -1 )
		{
		 	frsky_receive_byte( rxbyte ) ;
		}
	}
	else
#endif
	{	
		uint16_t rxchar ;
		while ( ( rxchar = rxTelemetry() ) != 0xFFFF )
		{
			frsky_receive_byte( rxchar ) ;
		}
	}
#endif
		
#ifdef PCBSKY
	uint16_t rxchar ;
	while ( ( rxchar = get_fifo128( &Com1_fifo ) ) != 0xFFFF )
	{
		frsky_receive_byte( rxchar ) ;
	}
//	rxPdcUsart( frsky_receive_byte ) ;		// Send serial data here
#endif

#else
	uint8_t data ;
extern void processSerialData(uint8_t data) ;

  while (telemetryFifo.pop(data))
	{
    processSerialData(data);
	}
#endif
	 
}

/* CRC16 implementation acording to CCITT standards */

const uint16_t CRC16_Short[] =
{
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef
} ;

static uint16_t CRC16Table(uint8_t val)
{
	return CRC16_Short[val&0x0F] ^ (0x1231 * (val>>4));
}

//static const unsigned short crc16tab[256]= {
//	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
//	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
//	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
//	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
//	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
//	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
//	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
//	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
//	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
//	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
//	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
//	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
//	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
//	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
//	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
//	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
//	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
//	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
//	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
//	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
//	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
//	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
//	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
//	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
//	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
//	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
//	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
//	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
//	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
//	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
//	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
//	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
//};

uint16_t crc16_ccitt( uint8_t *buf, uint32_t len )
{
	uint32_t counter;
	uint16_t crc = 0;
	for( counter = 0; counter < len; counter++)
	{
//		crc = (crc<<8) ^ crc16tab[ ((crc>>8) ^ *buf++ )	&0x00FF] ;
		crc = (crc<<8) ^ CRC16Table( ((crc>>8) ^ *buf++ )	&0x00FF) ;
	}
	return crc;
}

