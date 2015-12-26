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
#endif
#ifdef PCBX9D
#include "stm32f2xx.h"
#include "stm32f2xx_flash.h"
#include "X9D/hal.h"
#endif
#ifdef PCB9XT
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_flash.h"
#include "X9D/hal.h"
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

//#if defined(PCB9XT)
//uint16_t lastPacketType ;
//#endif

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
#endif
#endif

// Update Types
#define UPDATE_TYPE_BOOTLOADER		0
#define UPDATE_TYPE_COPROCESSOR		1
#define UPDATE_TYPE_SPORT_INT			2
#define UPDATE_TYPE_SPORT_EXT			3
#define UPDATE_TYPE_CHANGE_ID			4
#define UPDATE_TYPE_AVR						5

extern void frsky_receive_byte( uint8_t data ) ;
uint16_t crc16_ccitt( uint8_t *buf, uint32_t len ) ;

#ifdef PCBSKY
 #ifndef REVX
void init_mtwi( void ) ;
uint32_t check_ready( void ) ;
uint32_t write_CoProc( uint32_t coInternalAddr, uint8_t *buf, uint32_t number ) ;
uint32_t coProcBoot( void ) ;
uint32_t read_status_CoProc( uint32_t bufaddr, uint32_t number ) ;

uint8_t Twi_rx_buf[26] ;
uint8_t CoProresult ;
 #endif
#endif

uint32_t sportUpdate( uint32_t external ) ;

TCHAR Filenames[8][50] ;
extern FATFS g_FATFS ;
extern uint8_t FileData[] ;	// Share with voice task
#if defined(PCBTARANIS)
uint8_t FileData[1024] ;
uint8_t ExtraFileData[1024] ;
#else
uint8_t *ExtraFileData = (uint8_t *) &VoiceBuffer[0] ;	// Share with voice task
#endif
FILINFO Finfo ;
DIR Dj ;
uint32_t FileSize[8] ;
TCHAR FlashFilename[60] ;
FIL FlashFile ;
UINT BlockCount ;
UINT XblockCount ;
uint32_t BytesFlashed ;
uint32_t ByteEnd ;
uint32_t BlockOffset ;
uint8_t UpdateItem ;
uint8_t MaintenanceRunning = 0 ;
uint8_t BlockInUse ;
uint8_t SportVerValid ;
uint8_t SportVersion[4] ;
uint32_t FirmwareSize ;

const uint8_t SportIds[32] = {0x00, 0xA1, 0x22, 0x83, 0xE4, 0x45, 0xC6, 0x67,
				                      0x48, 0xE9, 0x6A, 0xCB, 0xAC, 0x0D, 0x8E, 0x2F,
															0xD0, 0x71, 0xF2, 0x53, 0x34, 0x95, 0x16, 0xB7,
															0x98, 0x39, 0xBA, 0x1B, 0x7C, 0xDD, 0x5E, 0xFF } ;

uint8_t SportState ;
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

#ifdef PCBSKY
uint32_t (*IAP_Function)(uint32_t, uint32_t) ;


uint32_t program( uint32_t *address, uint32_t *buffer )	// size is 256 bytes
{
	uint32_t FlashSectorNum ;
	uint32_t flash_cmd = 0 ;
	uint32_t i ;
	
	if ( (uint32_t) address >= 0x00408000 )
	{
		return 1 ;
	}

	// Always initialise this here, setting a default doesn't seem to work
	IAP_Function = (uint32_t (*)(uint32_t, uint32_t))  *(( uint32_t *)0x00800008) ;
	FlashSectorNum = (uint32_t) address ;
	FlashSectorNum >>= 8 ;		// page size is 256 bytes
	FlashSectorNum &= 2047 ;	// max page number
	
	/* Send data to the sector here */
	for ( i = 0 ; i < 64 ; i += 1 )
	{
		*address++ = *buffer++ ;		
	}

	/* build the command to send to EEFC */
	flash_cmd = (0x5A << 24) | (FlashSectorNum << 8) | 0x03 ; //AT91C_MC_FCMD_EWP ;
	
	__disable_irq() ;
	/* Call the IAP function with appropriate command */
	i = IAP_Function( 0, flash_cmd ) ;
	__enable_irq() ;
	return i ;
}
#endif

#if defined(PCBX9D) || defined(PCB9XT)
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

	if ( (uint32_t) address >= 0x08008000 )
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
		return 0 ;
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
	} while ( loop ) ;
	return fr ;
}

uint32_t fillNames( uint32_t index, struct fileControl *fc )
{
	uint32_t i ;
	FRESULT fr ;
	Finfo.lfname = Filenames[0] ;
	Finfo.lfsize = 48 ;
	fr = f_readdir ( &Dj, 0 ) ;					// rewind
	fr = f_readdir ( &Dj, &Finfo ) ;		// Skip .
	fr = f_readdir ( &Dj, &Finfo ) ;		// Skip ..
	i = 0 ;
	while ( i <= index )
	{
		fr = readBinDir( &Dj, &Finfo, fc ) ;		// First entry
		FileSize[0] = Finfo.fsize ;
		i += 1 ;
		if ( fr != FR_OK || Finfo.fname[0] == 0 )
		{
			return 0 ;
		}
	}
	for ( i = 1 ; i < 7 ; i += 1 )
	{
		Finfo.lfname = Filenames[i] ;
		fr = readBinDir( &Dj, &Finfo, fc ) ;		// First entry
		FileSize[i] = Finfo.fsize ;
		if ( fr != FR_OK || Finfo.fname[0] == 0 )
		{
			break ;
		}
	}
	return i ;
}

#define DISPLAY_CHAR_WIDTH	21

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
		len = x = strlen( Filenames[i] ) ;
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
		lcd_putsn_P( 0, 16+FH*i, &Filenames[i][x], len ) ;
	}

#if !defined(PCBTARANIS)
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
		if ( fc->vpos < limit-1 )
		{
			fc->vpos += 1 ;
		}
		else
		{
			if ( fc->nameCount > limit )
			{
				fc->index += 1 ;
				fc->nameCount = fillNames( fc->index, fc ) ;
			}
		}
	}
	if ( ( event == EVT_KEY_REPT(KEY_UP)) || ( event == EVT_KEY_FIRST(KEY_UP) ) )
	{
		if ( fc->vpos > 0 )
		{
			fc->vpos -= 1 ;
		}
		else
		{
			if ( fc->index )
			{
				fc->index -= 1 ;
				fc->nameCount = fillNames( fc->index, fc ) ;
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
	if ( ( event == EVT_KEY_FIRST(KEY_EXIT) ) || ( event == EVT_KEY_LONG(BTN_RE) ) )
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
	return result ;
}

uint8_t CoProcReady ;

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

uint8_t RxPacket[10] ;
uint8_t PhyId ;
uint8_t NewPhyId ;
uint16_t AppId ;

void menuChangeId(uint8_t event)
{
	static uint32_t state ;
 	
	TITLE( "CHANGE SPort Id" ) ;

	lcd_puts_Pleft( 2*FH, "Not Implemented(yet)" ) ;


	switch(event)
	{
    case EVT_ENTRY:
			RxCount = 0 ;
			RxLastCount = 0 ;
			FrskyTelemetryType = 1 ;
			IdIndex = 0x1B ;
			IdFound = 0 ;
			state = CHANGE_SCANNING ;
			SendCount = 2 ;
    break ;
    
		case EVT_KEY_FIRST(KEY_EXIT):
     	chainMenu(menuUpdate) ;
   		killEvents(event) ;
    break ;
		
		case EVT_KEY_FIRST(KEY_UP):
			if ( state == CHANGE_ENTER_ID )
			{
				if ( ++NewPhyId > 0x1B )
				{
					NewPhyId = 0x1B ;
				}
			}
    break ;
    
		case EVT_KEY_FIRST(KEY_DOWN):
			if ( state == CHANGE_ENTER_ID )
			{
				if ( NewPhyId  )
				{
					NewPhyId -= 1 ;
				}
			}
    break ;

		case EVT_KEY_FIRST(KEY_MENU):
			if ( state == CHANGE_ENTER_ID )
			{
				TxPhyPacket[0] = 0x7E ;
				TxPhyPacket[1] = PhyId ;
				TxPhyPacket[2] = 0x21 ;
				TxPhyPacket[3] = 0xFF ;
				TxPhyPacket[4] = 0xFF ;
				TxPhyPacket[5] = 0 ;
				TxPhyPacket[6] = 0 ;
				TxPhyPacket[7] = 0 ;
				TxPhyPacket[8] = 0 ;

  			uint16_t crc = 0 ;
  			for ( uint8_t i=2; i<9; i++)
				{
  			  crc += TxPhyPacket[i]; //0-1FF
  			  crc += crc >> 8; //0-100
  			  crc &= 0x00ff;
  			}
				TxPhyPacket[9] = ~crc ;
#if defined(PCBX9D) || defined(PCB9XT)
				x9dSPortTxStart( TxPhyPacket, 10, NO_RECEIVE ) ;
#endif
#ifdef PCBSKY
				txPdcUsart( TxPhyPacket, 10, NO_RECEIVE ) ;
#endif
				state = CHANGE_SET_IDLE ;
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
				if ( ++IdIndex > 0x1B )
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
		break ;

	}

	lcd_outhex4( 0, 7*FH, RxCount ) ;
	if ( RxPacket[1] == 0x10 )
	{
		IdFound = 1 ;
		PhyId = RxPacket[0] ;
		NewPhyId = PhyId & 0x1F ;
		if ( NewPhyId > 0x1B )
		{
			NewPhyId = 0x1B ;
		}
		AppId = RxPacket[2] | ( RxPacket[3] << 8 ) ;
		lcd_outhex4( 0, 4*FH, RxPacket[0] ) ;
		lcd_outhex4( 25, 4*FH, RxPacket[1] ) ;
		lcd_outhex4( 50, 4*FH, RxPacket[2] ) ;
		lcd_outhex4( 75, 4*FH, RxPacket[3] ) ;
		lcd_outhex4( 100, 4*FH, RxPacket[4] ) ;
		lcd_outhex4( 0, 5*FH, RxPacket[5] ) ;
		lcd_outhex4( 25, 5*FH, RxPacket[6] ) ;
		lcd_outhex4( 50, 5*FH, RxPacket[7] ) ;
		lcd_outhex4( 75, 5*FH, RxPacket[8] ) ;
//		lcd_outhex4( 100, 5*FH, RxPacket[9] ) ;
	}
	if ( IdFound )
	{
		lcd_outhex4( 0, 6*FH, PhyId ) ;
		lcd_outhex4( 25, 6*FH, AppId ) ;
	}

//extern uint16_t TelemetryDebug ;
//extern uint16_t TelemetryDebug1 ;
//extern uint16_t TelemetryDebug2 ;
//extern uint16_t TelemetryDebug3 ;
//	lcd_outhex4( 0, 6*FH, TelemetryDebug ) ;
//	lcd_outhex4( 25, 6*FH, TelemetryDebug1 ) ;
//	lcd_outhex4( 50, 6*FH, TelemetryDebug2 ) ;
//	lcd_outhex4( 75, 6*FH, TelemetryDebug3 ) ;
}

void menuUp1(uint8_t event)
{
	FRESULT fr ;
	struct fileControl *fc = &FileControl ;
  static uint8_t mounted = 0 ;
	static uint32_t state ;
	static uint32_t firmwareAddress ;
	uint32_t i ;
	uint32_t width ;
	 
	if (UpdateItem == UPDATE_TYPE_BOOTLOADER )		// Bootloader
	{
  	TITLE( "UPDATE BOOT" ) ;
	}
	else
	{
#ifdef PCB9XT
 		if (UpdateItem == UPDATE_TYPE_SPORT_EXT )
		{
  		TITLE( "UPDATE Ext. SPort" ) ;
		}
		else
		{
  		TITLE( "UPDATE AVR" ) ;
		}
#endif
		
#ifdef PCBX9D
		if (UpdateItem == UPDATE_TYPE_SPORT_INT )
		{
  		TITLE( "UPDATE Int. XJT" ) ;
		}
		else
		{
  		TITLE( "UPDATE Ext. SPort" ) ;
		}
#endif

#ifdef PCBSKY
 #ifndef REVX
 		if (UpdateItem == UPDATE_TYPE_COPROCESSOR )
		{
  		TITLE( "UPDATE COPROC" ) ;
		}
		else
		{
  		TITLE( "UPDATE SPort" ) ;
		}
 #else
 		if (UpdateItem == UPDATE_TYPE_SPORT_EXT )
		{
  		TITLE( "UPDATE SPort" ) ;
		}
 #endif
#endif



	}
	switch(event)
	{
    case EVT_ENTRY:
			state = UPDATE_NO_FILES ;
			if ( mounted == 0 )
			{
#if defined(PCBTARANIS)
  			fr = f_mount(0, &g_FATFS_Obj) ;
#else				
  			fr = f_mount(0, &g_FATFS) ;
#endif
#if defined(PCBX9D) || defined(PCB9XT)
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
					fr = f_opendir( &Dj, (TCHAR *) "." ) ;
					if ( fr == FR_OK )
					{
 						if ( (UpdateItem > 1 ) && (UpdateItem != 5 ) )
						{
							fc->ext[0] = 'F' ;
							fc->ext[1] = 'R' ;
							fc->ext[2] = 'K' ;
						}
						else
						{
							fc->ext[0] = 'B' ;
							fc->ext[1] = 'I' ;
							fc->ext[2] = 'N' ;
						}
						fc->ext[3] = 0 ;
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
			SportVerValid = 0 ;
			if ( fileList( event, &FileControl ) == 1 )
			{
				state = UPDATE_CONFIRM ;
			}
    break ;
		case UPDATE_CONFIRM :
 			if ( (UpdateItem > UPDATE_TYPE_BOOTLOADER ) )
			{
#ifdef PCBX9D
 				if ( (UpdateItem == UPDATE_TYPE_SPORT_INT ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Int. XJT from" ) ;
				}
				else
				{
					lcd_puts_Pleft( 2*FH, "Flash Ext.SP from" ) ;
				}
				SportVerValid = 0 ;
#else
 #ifndef REVX
 				if ( (UpdateItem == UPDATE_TYPE_COPROCESSOR ) )
				{
					lcd_puts_Pleft( 2*FH, "Flash Co-Proc. from" ) ;
				}
				else
				{
					lcd_puts_Pleft( 2*FH, "Flash SPort from" ) ;
				}
				CoProcReady = 0 ;
 #else
				lcd_puts_Pleft( 2*FH, "Flash SPort from" ) ;
 #endif
#endif
#ifdef PCB9XT
		 		if (UpdateItem == UPDATE_TYPE_SPORT_EXT )
				{
					lcd_puts_Pleft( 2*FH, "Flash Ext.SP from" ) ;
					SportVerValid = 0 ;
				}
				else
				{
					// AVR
					lcd_puts_Pleft( 2*FH, "Flash AVR from" ) ;
				}
#endif
			}
			else
			{
				lcd_puts_Pleft( 2*FH, "Flash Bootloader from" ) ;
			}
			cpystr( cpystr( (uint8_t *)FlashFilename, (uint8_t *)"\\firmware\\" ), (uint8_t *)Filenames[fc->vpos] ) ;
#if defined(PCBTARANIS)
			lcd_putsnAtt( 0, 4*FH, Filenames[fc->vpos], DISPLAY_CHAR_WIDTH, 0 ) ;
#else
			lcd_putsnAtt0( 0, 4*FH, Filenames[fc->vpos], DISPLAY_CHAR_WIDTH, 0 ) ;
#endif
			if ( event == EVT_KEY_LONG(KEY_MENU) )
			{
				state = UPDATE_SELECTED ;
			}
			if ( event == EVT_KEY_LONG(KEY_EXIT) )
			{
				state = UPDATE_FILE_LIST ;		// Canceled
			}
    break ;
		case UPDATE_SELECTED :
			f_open( &FlashFile, FlashFilename, FA_READ ) ;
			f_read( &FlashFile, (BYTE *)FileData, 1024, &BlockCount ) ;
			i = 1 ;
			if (UpdateItem == UPDATE_TYPE_BOOTLOADER )		// Bootloader
			{
				i = validateFile( (uint32_t *) FileData ) ;
			}
			if ( i == 0 )
			{
				state = UPDATE_INVALID ;
			}
			else
			{
				if (UpdateItem == UPDATE_TYPE_BOOTLOADER )		// Bootloader
				{
#if defined(PCBX9D) || defined(PCB9XT)
					firmwareAddress = 0x08000000 ;
#endif
#ifdef PCBSKY
					firmwareAddress = 0x00400000 ;
#endif
				}
#ifdef PCBSKY
 #ifndef REVX
				else if (UpdateItem == UPDATE_TYPE_COPROCESSOR )		// Bootloader
				{
					firmwareAddress = 0x00000080 ;
					if ( check_ready() == 0 )
					{
						CoProcReady = 1 ;
					}
				}
 #endif
#endif
#ifdef PCB9XT
				else if (UpdateItem == UPDATE_TYPE_AVR )		// Bootloader
				{
					width = 50 ;	// Not used
					
				}
#endif
				else
				{
// SPort update
					SportState = SPORT_START ;
					FirmwareSize = FileSize[fc->vpos] ;
					BlockInUse = 0 ;
					f_read( &FlashFile, (BYTE *)ExtraFileData, 1024, &XblockCount ) ;
				}
				BytesFlashed = 0 ;
				BlockOffset = 0 ;
				ByteEnd = 1024 ;
				state = UPDATE_ACTION ;
			}
    break ;
		case UPDATE_INVALID :
			lcd_puts_Pleft( 2*FH, "Invalid File" ) ;
			lcd_puts_Pleft( 4*FH, "Press EXIT" ) ;
			if ( event == EVT_KEY_FIRST(KEY_EXIT) )
			{
				state = UPDATE_FILE_LIST ;		// Canceled
    		killEvents(event) ;
			}
    break ;
		case UPDATE_ACTION :
			// Do the flashing
			lcd_puts_Pleft( 3*FH, "Flashing" ) ;
			if (UpdateItem == UPDATE_TYPE_BOOTLOADER )		// Bootloader
			{
				width = ByteEnd >> 9 ;
				if ( BytesFlashed < ByteEnd )
				{
					program( (uint32_t *)firmwareAddress, &((uint32_t *)FileData)[BlockOffset] ) ;	// size is 256 bytes
					BlockOffset += 64 ;		// 32-bit words (256 bytes)
					firmwareAddress += 256 ;
					BytesFlashed += 256 ;
				}
				else
				{
					if ( ByteEnd >= 32768 )
					{
						state = UPDATE_COMPLETE ;
					}
					else
					{
						f_read( &FlashFile, (BYTE *)FileData, 1024, &BlockCount ) ;
						ByteEnd += 1024 ;
						BlockOffset = 0 ;
					}
				}
			}

#ifdef PCBSKY
 #ifndef REVX
			else if (UpdateItem == UPDATE_TYPE_COPROCESSOR )		// CoProcessor
			{
				uint32_t size = FileSize[fc->vpos] ;
				width = BytesFlashed * 64 / size ;
				CoProresult = 0 ;
				if ( CoProcReady )
				{
					if ( BytesFlashed < ByteEnd )
					{
						uint32_t number ;

						number = ByteEnd - BytesFlashed ;
						if ( number > 128 )
						{
							number = 128 ;						
						}
						i = write_CoProc( (uint32_t)firmwareAddress, &FileData[BlockOffset], number ) ;
						BlockOffset += 128 ;		// 8-bit bytes
						firmwareAddress += 128 ;
						BytesFlashed += 128 ;
					}
					else
					{
						if ( ByteEnd >= size )
						{
							state = UPDATE_COMPLETE ;
						}
						else
						{
							f_read( &FlashFile, (BYTE *)FileData, 1024, &BlockCount ) ;
							ByteEnd += BlockCount ;
							BlockOffset = 0 ;
						}
					}
				}
				else
				{
					CoProresult = 1 ;
					state = UPDATE_COMPLETE ;
				}
			}
 #endif
#endif
			
#ifdef PCB9XT
			else if (UpdateItem == UPDATE_TYPE_AVR )		// Bootloader
			{
				// No more display, never return, check for power off when finished
				
			}
#endif
			else		// Internal/External Sport
			{
#ifdef PCBX9D
				width = sportUpdate( (UpdateItem == UPDATE_TYPE_SPORT_INT) ? SPORT_INTERNAL : SPORT_EXTERNAL ) ;
#else
				width = sportUpdate( SPORT_EXTERNAL ) ;
#endif
				if ( width > FirmwareSize )
				{
					state = UPDATE_COMPLETE ;
					SportVerValid = width ;
				}
				width *= 64 ;
				width /= FirmwareSize ;
				if ( SportVerValid )
				{
					lcd_outhex4( 0, 7*FH, (SportVersion[0] << 8) | SportVersion[1] ) ;
					lcd_outhex4( 25, 7*FH, (SportVersion[2] << 8) | SportVersion[3] ) ;
				}
//#if defined(PCB9XT)
//				lcd_outhex4( 60, 7*FH, lastPacketType ) ;
//				lcd_outhex4( 85, 7*FH, SportState ) ;
//#endif

				if ( SportState == SPORT_POWER_ON )
				{
					lcd_puts_Pleft( 4*FH, "Finding Device" ) ;
				}

				if ( event == EVT_KEY_LONG(KEY_EXIT) )
				{
					state = UPDATE_COMPLETE ;
					SportVerValid = 0x00FF ; // Abort with failed
					SportState = SPORT_IDLE ;
				}
			}	
			lcd_hline( 0, 5*FH-1, 65 ) ;
			lcd_hline( 0, 6*FH, 65 ) ;
			lcd_vline( 64, 5*FH, 8 ) ;
			for ( i = 0 ; i <= width ; i += 1 )
			{
				lcd_vline( i, 5*FH, 8 ) ;
			}
    break ;
		
		case UPDATE_COMPLETE :
			lcd_puts_Pleft( 3*FH, "Flashing Complete" ) ;
 			if ( (UpdateItem != UPDATE_TYPE_BOOTLOADER ) )
 			{
#if defined(PCBX9D) || defined(PCB9XT)
 				if ( SportVerValid & 1 )
 				{
 					lcd_puts_Pleft( 5*FH, "FAILED" ) ;
 				}
#endif
#ifdef PCBSKY
 #ifndef REVX
				if (UpdateItem == UPDATE_TYPE_COPROCESSOR )		// CoProcessor
				{
					if ( CoProresult )
					{
						lcd_puts_Pleft( 5*FH, "FAILED" ) ;
					}
				}
				else
				{
 #endif 			
 					if ( SportVerValid & 1 )
	 				{
 						lcd_puts_Pleft( 5*FH, "FAILED" ) ;
 					}
  #ifndef REVX
			  }
  #endif 			
 #endif 			
			}

			if ( event == EVT_KEY_FIRST(KEY_EXIT) )
			{
#if defined(PCBX9D) || defined(PCB9XT)
				EXTERNAL_RF_OFF();
				INTERNAL_RF_OFF();
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
	lcd_puts_Pleft( 3*FH, "  Update CoProcessor" );
	lcd_puts_Pleft( 4*FH, "  Update SPort" );
	lcd_puts_Pleft( 5*FH, "  Change SPort Id" );
 #else
	lcd_puts_Pleft( 3*FH, "  Update SPort" );
	lcd_puts_Pleft( 4*FH, "  Change SPort Id" );
 #endif
#endif
#ifdef PCBX9D
	lcd_puts_Pleft( 3*FH, "  Update Int. XJT" );
	lcd_puts_Pleft( 4*FH, "  Update Ext. SPort" );
	lcd_puts_Pleft( 5*FH, "  Change SPort Id" );
#endif
#ifdef PCB9XT
	lcd_puts_Pleft( 3*FH, "  Update Ext. SPort" );
	lcd_puts_Pleft( 4*FH, "  Update Avr CPU" );
#endif

  switch(event)
	{
    case EVT_ENTRY:
			position = 2*FH ;
    break ;
    
		case EVT_KEY_FIRST(KEY_MENU):
			if ( position == 2*FH )
			{
				UpdateItem = UPDATE_TYPE_BOOTLOADER ;
	      chainMenu(menuUp1) ;
			}
#ifdef PCBSKY
 #ifndef REVX
			if ( position == 3*FH )
			{
				UpdateItem = UPDATE_TYPE_COPROCESSOR ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 4*FH )
			{
				UpdateItem = UPDATE_TYPE_SPORT_EXT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 5*FH )
			{
				UpdateItem = UPDATE_TYPE_CHANGE_ID ;
	      chainMenu(menuChangeId) ;
			}
 #else
			if ( position == 3*FH )
			{
				UpdateItem = UPDATE_TYPE_SPORT_EXT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 4*FH )
			{
				UpdateItem = UPDATE_TYPE_CHANGE_ID ;
	      chainMenu(menuChangeId) ;
			}
 #endif
#endif
#ifdef PCBX9D
			if ( position == 3*FH )
			{
				UpdateItem = UPDATE_TYPE_SPORT_INT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 4*FH )
			{
				UpdateItem = UPDATE_TYPE_SPORT_EXT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 5*FH )
			{
				UpdateItem = UPDATE_TYPE_CHANGE_ID ;
	      chainMenu(menuChangeId) ;
			}
#endif
#ifdef PCB9XT
			if ( position == 3*FH )
			{
				UpdateItem = UPDATE_TYPE_SPORT_EXT ;
	      chainMenu(menuUp1) ;
			}
			if ( position == 4*FH )
			{
				UpdateItem = UPDATE_TYPE_AVR ;
	      chainMenu(menuChangeId) ;
			}
#endif
    	killEvents(event) ;
			reboot = 0 ;
    break ;

    case EVT_KEY_LONG(KEY_EXIT):
			reboot = 1 ;
		break ;

#ifdef PCBSKY
 #ifndef REVX
    case EVT_KEY_FIRST(KEY_DOWN):
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
 #else
    case EVT_KEY_FIRST(KEY_DOWN):
			if ( position < 4*FH )
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
#ifdef PCB9XT
    case EVT_KEY_FIRST(KEY_DOWN):
//			if ( position < 4*FH )
			if ( position < 3*FH )
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

uint32_t write_CoProc( uint32_t coInternalAddr, uint8_t *buf, uint32_t number )
{
	uint32_t i ;
	uint8_t *ptr ;

	Addr_buffer[0] = 0x01 ;		// Command, PROGRAM
	Addr_buffer[1] = coInternalAddr >> 8 ;
	Addr_buffer[2] = coInternalAddr ;
	ptr = buf ;
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
	init_mtwi() ;

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
#if defined(PCBX9D) || defined(PCB9XT)
	x9dSPortTxStart( TxPhyPacket, i, NO_RECEIVE ) ;
#endif
#ifdef PCBSKY
	txPdcUsart( TxPhyPacket, i, NO_RECEIVE ) ;
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
//#if defined(PCB9XT)
//		lastPacketType = packet[2] ;
//#endif
		switch( packet[2] )
		{
			case PRIM_ACK_POWERUP :
				if ( SportState == SPORT_POWER_ON )
				{
					SportTimer = 2 ;
					SportState = SPORT_VERSION ;
				}
			break ;
        
			case PRIM_ACK_VERSION:
				if ( SportState == SPORT_VERSION )
				{
					SportTimer = 2 ;
					SportState = SPORT_DATA_START ;
					SportVersion[0] = packet[3] ;
					SportVersion[1] = packet[4] ;
					SportVersion[2] = packet[5] ;
					SportVersion[3] = packet[6] ;
					SportVerValid = 1 ;
				}
			break ;

			case PRIM_REQ_DATA_ADDR :
			{
				UINT bcount ;
				bcount = BlockInUse ? XblockCount : BlockCount ;
				
				if ( BytesFlashed >= FirmwareSize )
				{
					// We have finished
					blankTxPacket() ;
					TxPacket[0] = 0x50 ;
					TxPacket[1] = PRIM_DATA_EOF ;
					SportTimer = 20 ;		// 200 mS
					writePacket( TxPacket, 0xFF ) ;
					SportState = SPORT_END ;
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
						ptr = ( uint32_t *) (BlockInUse ? ExtraFileData : FileData ) ;
						ptr += addr ;
						uint32_t *dptr = (uint32_t *)(&TxPacket[2]) ;
        		*dptr = *ptr ;
						SportTimer = 5 ;		// 50 mS
						writePacket( TxPacket, 0xFF ) ;
					}
					if ( BlockInUse )
					{
						XblockCount = bcount ;
					}
					else
					{
						BlockCount = bcount ;
					}
					if ( bcount == 0 )
					{
						SportState = SPORT_DATA_READ ;
						if ( BlockInUse )
						{
							BlockInUse = 0 ;						
						}
						else
						{
							BlockInUse = 1 ;
						}
					}
				}
			}
			break ;

			case PRIM_END_DOWNLOAD :
					SportState = SPORT_COMPLETE ;
			break ;
				
			case PRIM_DATA_CRC_ERR :
					SportState = SPORT_FAIL ;
			break ;
		}
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

// This is called repeatedly every 10mS while update is in progress
uint32_t sportUpdate( uint32_t external )
{
	if ( SportTimer )
	{
		SportTimer -= 1 ;
	}
	switch ( SportState )
	{
		case SPORT_IDLE :
			SportTimer = 0 ;
		break ;
		
		case SPORT_START :
#if !defined(PCBTARANIS)
			FrskyTelemetryType = 1 ;
#endif
#if defined(PCBX9D) || defined(PCB9XT)
#if defined(PCBTARANIS)
			sportInit() ;
#else
			x9dSPortInit( 57600, SPORT_MODE_HARDWARE, SPORT_POLARITY_NORMAL, 0 ) ;
#endif
#endif
#ifdef PCBSKY
 #ifdef REVX
		clearMfp() ;
 #endif
			UART2_Configure( 57600, Master_frequency ) ;
			startPdcUsartReceive() ;
#endif
			SportTimer = 5 ;		// 50 mS
#ifdef PCBX9D
			if ( external )
			{
  			EXTERNAL_RF_ON();
			}
			else
			{
  			INTERNAL_RF_ON();
			}
#endif
#ifdef PCB9XT
 			EXTERNAL_RF_ON() ;
#endif
			SportState = SPORT_POWER_ON ;
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
				SportState = SPORT_VERSION ;
				writePacket( TxPacket, 0xFF ) ;
			}
		break ;

		case SPORT_DATA_START :
			blankTxPacket() ;
			TxPacket[0] = 0x50 ;
			TxPacket[1] = PRIM_CMD_DOWNLOAD ;
			SportTimer = 20 ;		// 200 mS
			SportState = SPORT_DATA ;
// Stop here for testing
			writePacket( TxPacket, 0xFF ) ;
		break ;

		case SPORT_DATA :
#if defined(PCB9XT)
			if ( SportTimer == 0 )
			{
				SportTimer = 5 ;		// 50 mS
				writePacket( TxPacket, 0xFF ) ;
			}
#endif
		break ;
		
		case SPORT_DATA_READ :
		{	
			uint32_t *ptr ;
			UINT *pcount ;
			ptr = ( uint32_t *) (BlockInUse ? FileData : ExtraFileData ) ;
			pcount = BlockInUse ? &BlockCount : &XblockCount ;
			f_read( &FlashFile, (BYTE *)ptr, 1024, pcount ) ;
			SportState = SPORT_DATA ;
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
	uint16_t rxchar ;
	while ( ( rxchar = rxTelemetry() ) != 0xFFFF )
	{
		frsky_receive_byte( rxchar ) ;
	}
#endif
		
#ifdef PCBSKY
	rxPdcUsart( frsky_receive_byte ) ;		// Send serial data here
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

static const unsigned short crc16tab[256]= {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

uint16_t crc16_ccitt( uint8_t *buf, uint32_t len )
{
	uint32_t counter;
	uint16_t crc = 0;
	for( counter = 0; counter < len; counter++)
	{
		crc = (crc<<8) ^ crc16tab[ ((crc>>8) ^ *buf++ )	&0x00FF] ;
	}
	return crc;
}

