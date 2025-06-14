/*
 * Author - Mike Blandford
 *
 * Based on er9x by Erez Raviv <erezraviv@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
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
#ifdef PCBSKY
  #include "AT91SAM3S4.h"
#endif
#ifdef PCB9XT
#endif
#include "ersky9x.h"
#include "stdio.h"
#include "inttypes.h"
#include "string.h"
#include "myeeprom.h"
#include "drivers.h"
#include "file.h"
#include "ff.h"
#include "frsky.h"
#include "menus.h"
#ifndef SIMU
#include "CoOS.h"
#endif
#ifdef REVX
#include "sound.h"
#endif
#include "stringidx.h"
#ifdef PCB9XT
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/hal.h"
#endif
#include "lcd.h"


extern PROGMEM s9xsplash[] ;

// EEPROM Addresses
// General 0x00000 - 0x01FFF
// Model01 0x02000 - 0x03FFF
// Model02 0x04000 - 0x05FFF
// . . .

// Logic for storing to EEPROM/loading from EEPROM
// If main needs to wait for the eeprom, call mainsequence without actioning menus
// General configuration
// 'main' set flag STORE_GENERAL
// 'eeCheck' sees flag STORE_GENERAL, copies data, starts 2 second timer, clears flag STORE_GENERAL
// 'eeCheck' sees 2 second timer expire, locks copy, initiates write, enters WRITE_GENERAL mode
// 'eeCheck' completes write, unlocks copy, exits WRITE_GENERAL mode

// 'main' set flag STORE_MODEL(n)

// 'main' set flag STORE_MODEL_TRIM(n)

// 'main' needs to load a model

// These may not be needed, or might just be smaller
uint8_t Spi_tx_buf[8] ;

uint8_t Spi_rx_buf[8] ;

#ifdef FS8K
	union 
	{
		uint8_t key[4] ;
		uint32_t keyLong ;
	} Key ;

uint16_t ControlOffset ;
uint8_t ControlBlock ;
uint8_t AllocTable[64] ;
//Spare blocks in AllocTable[61] and [62]
#endif

#ifdef FS8K
struct t_file_entry File_system ; // [MAX_MODELS+1] ;
uint16_t FileSizes[60] ;
#else
struct t_file_entry File_system[MAX_MODELS+1] ;
#endif

unsigned char ModelNames[MAX_MODELS+1][sizeof(g_model.name)] ;		// Allow for general

uint16_t General_timer ;
uint16_t Model_timer ;

struct t_dirty
{
	uint8_t General_dirty ;
	uint8_t Model_dirty ;
} Dirty ;

uint8_t Ee32_general_write_pending ;
uint8_t Ee32_model_write_pending ;
uint8_t Ee32_model_delete_pending ;

uint8_t	Eeprom32_process_state ;
uint8_t	Eeprom32_state_after_erase ;
#ifdef FS8K
uint8_t	Eeprom32_state_after_write ;
#endif
uint8_t	Eeprom32_write_pending ;
uint8_t Eeprom32_file_index ;
uint8_t *Eeprom32_buffer_address ;
uint8_t *Eeprom32_source_address ;
uint32_t Eeprom32_address ;
uint32_t Eeprom32_data_size ;
#ifdef PCB9XT
uint8_t EeLockEncoder ;
#endif	

#ifdef FS8K
uint8_t ConversionProgress ;
#endif

#define EE_WAIT			0
#define EE_NO_WAIT	1


// States in Eeprom32_process_state
#define E32_IDLE							1
#define E32_ERASESENDING			2
#define E32_ERASEWAITING			3
#define E32_WRITESENDING			4
#define E32_WRITEWAITING			5
#define E32_READSENDING				6
#define E32_READWAITING				7
#define E32_BLANKCHECK				8
#define E32_WRITESTART				9
#define E32_LOCKED						10
#ifdef FS8K
#define E32_BLANK2CHECK				11
#define E32_WRITEALLOC				12
#define E32_CHECKALLOC				13
#define E32_WRITECONTROL			14
#define E32_BLANK3CHECK				15

// Bits in flags
#define BLOCK_8K						1
#endif

bool eeModelExists(uint8_t id) ;
#ifdef FS8K
uint32_t get_current_block_number( uint32_t block_no, t_file_entry *pfs ) ;
#else
uint32_t get_current_block_number( uint32_t block_no, uint16_t *p_size, uint32_t *p_seq ) ;
#endif
uint32_t read32_eeprom_data( uint32_t eeAddress, register uint8_t *buffer, uint32_t size, uint32_t immediate ) ;
uint32_t write32_eeprom_block( uint32_t eeAddress, register uint8_t *buffer, uint32_t size, uint32_t immediate ) ;
void ee32_read_model_names( void ) ;
void ee32LoadModelName(uint8_t id, unsigned char*buf,uint8_t len) ;
void ee32_update_name( uint32_t id, uint8_t *source ) ;
void convertModel( SKYModelData *dest, ModelData *source ) ;

extern union t_sharedMemory SharedMemory ;



// New file system

// Start with 16 model memories, initial test uses 34 and 35 for these
// Blocks 0 and 1, general
// Blocks 2 and 3, model 1
// Blocks 4 and 5, model 2 etc
// Blocks 32 and 33, model 16

#ifndef FS8K
uint8_t Current_general_block ;		// 0 or 1 is active block
uint8_t Other_general_block_blank ;
#endif

struct t_eeprom_header
{
	uint32_t sequence_no ;		// sequence # to decide which block is most recent
	uint16_t data_size ;			// # bytes in data area
	uint8_t flags ;
	uint8_t hcsum ;
} ;

// Structure of data in a block
struct t_eeprom_block
{
	struct t_eeprom_header header ;
	union
	{
		uint8_t bytes[4088] ;
		uint32_t words[1022] ;
	} data ;
} ;


#define EEPROM_BUFFER_SIZE ((sizeof(SKYModelData) + sizeof( struct t_eeprom_header ) + 3)/4)


struct t_eeprom_buffer
{
	struct t_eeprom_header header ;
	union t_eeprom_data
	{
		EEGeneral general_data ;
		ModelData model_data ;
		SKYModelData sky_model_data ;
#if defined(PCBSKY) || defined(PCB9XT)
		ModelData  oldmodel ;
#endif
		uint32_t words[ EEPROM_BUFFER_SIZE ] ;
		uint8_t buffer2K[2048] ;
	} data ;	
} Eeprom_buffer ;

static void ee32WaitFinished()
{
  while ( ee32_check_finished() == 0 )
	{
		if ( General_timer )
		{
			General_timer = 1 ;		// Make these happen soon
		}
		if ( Model_timer )
		{
			Model_timer = 1 ;
		}
    ee32_process();
#ifdef SIMU
    sleep(5/*ms*/);
#endif
  }
}

// genaral data needs to be written to EEPROM
void ee32StoreGeneral()
{
	Dirty.General_dirty = 1 ;
	General_timer = 500 ;		// 5 seconds timeout before writing
}

void eeModelChanged()
{
	if ( Model_timer == 0 )
	{
		ee32StoreModel( g_eeGeneral.currModel, EE_MODEL ) ;
		Model_timer = 30000 ;	// 5 minutes
	}
}

// Store model to EEPROM, trim is non-zero if this is the result of a trim change
void ee32StoreModel( uint8_t modelNumber, uint8_t trim )
{
	Dirty.Model_dirty = modelNumber + 1 ;
	Model_timer = 500 ;	
	ee32_update_name( Dirty.Model_dirty, (uint8_t *)&g_model ) ;		// In case it's changed
}

void ee32_delete_model( uint8_t id )
{
	uint8_t buffer[sizeof(g_model.name)+1] ;
  memset( buffer, ' ', sizeof(g_model.name) ) ;
	ee32_update_name( id + 1, buffer ) ;
	Ee32_model_delete_pending = id + 1 ;
	ee32_process() ;		// Kick it off
	ee32WaitFinished() ;
}

void ee32_read_model_names()
{
	uint32_t i ;

	for ( i = 1 ; i <= MAX_MODELS ; i += 1 )
	{
		ee32LoadModelName( i, ModelNames[i], sizeof(g_model.name) ) ;
	}
}

void ee32_update_name( uint32_t id, uint8_t *source )
{
	uint8_t * p ;
	uint32_t i ;

	p = ModelNames[id] ;
	for ( i = 0 ; i < sizeof(g_model.name) ; i += 1 )
	{
		*p++ = *source++ ;
	}
//	*p = '\0' ;
}

void waitForEepromFinished()
{
	while (ee32_check_finished() == 0)
	{	// wait
#ifndef SIMU
		if ( General_timer )
		{
			General_timer = 1 ;		// Make these happen soon
		}
		if ( Model_timer )
		{
			Model_timer = 1 ;
		}
		CoTickDelay(1) ;					// 2mS for now
#endif
	}
}


bool ee32CopyModel(uint8_t dst, uint8_t src)
{
#ifdef FS8K
	uint16_t size = FileSizes[src-1] ;
#else
  uint16_t size = File_system[src].size ;
#endif
	 
	waitForEepromFinished() ;
//	while (ee32_check_finished() == 0)
//	{	// wait
//#ifndef SIMU
//		if ( General_timer )
//		{
//			General_timer = 1 ;		// Make these happen soon
//		}
//		if ( Model_timer )
//		{
//			Model_timer = 1 ;
//		}
//		CoTickDelay(1) ;					// 2mS for now
//#endif
//	}

#ifdef FS8K
	read32_eeprom_data( ( AllocTable[src] << 13) + sizeof( struct t_eeprom_header), ( uint8_t *)&Eeprom_buffer.data.sky_model_data, size, 0 ) ;
#else
  read32_eeprom_data( (File_system[src].block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&Eeprom_buffer.data.sky_model_data, size, 0 ) ;
#endif

  if (size > sizeof(g_model.name))
    memcpy( ModelNames[dst], Eeprom_buffer.data.sky_model_data.name, sizeof(g_model.name)) ;
  else
    memset( ModelNames[dst], ' ', sizeof(g_model.name)) ;

  Eeprom32_source_address = (uint8_t *)&Eeprom_buffer.data.sky_model_data ;		// Get data from here
  Eeprom32_data_size = sizeof(g_model) ;																	// This much
  Eeprom32_file_index = dst ;																							// This file system entry
#ifdef FS8K
	Eeprom32_state_after_erase = E32_BLANK2CHECK ;
#endif
  Eeprom32_process_state = E32_BLANKCHECK ;
  ee32WaitFinished() ;
  return true;
}

void ee32SwapModels(uint8_t id1, uint8_t id2)
{
  ee32WaitFinished();
//  // eeCheck(true) should have been called before entering here

#ifdef FS8K
	unsigned char tname[sizeof(g_model.name)] ;
	uint8_t temp = AllocTable[id1] ;
	AllocTable[id1] = AllocTable[id2] ;
	AllocTable[id2] = temp ;
  memcpy( tname, ModelNames[id1], sizeof(g_model.name)) ;
  memcpy( ModelNames[id1], ModelNames[id2], sizeof(g_model.name)) ;
  memcpy( ModelNames[id2], tname, sizeof(g_model.name)) ;
	Eeprom32_process_state = E32_WRITEALLOC ;
#else
  uint32_t id2_block_no ;
  uint16_t id2_size = File_system[id2].size ;
  uint16_t id1_size = File_system[id1].size ;
	
	if ( id1_size == 0 )
	{ // Copying blank entry
		if ( id2_size == 0 )
		{ // To blank entry
			return ;	// Nothing to do
		}
		// Copy blank entry to model, swap the copy
		uint8_t temp = id1 ;
		id1 = id2 ;
		id2 = temp ;
	}
  
  id2_size = File_system[id2].size ;
	id2_block_no = File_system[id2].block_no ;

  ee32CopyModel(id2, id1);

//  // block_no(id1) has been shifted now, but we have the size
  if (id2_size > sizeof(g_model.name))
	{
    read32_eeprom_data( (id2_block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&Eeprom_buffer.data.sky_model_data, id2_size, 0 ) ;
    memcpy( ModelNames[id1], Eeprom_buffer.data.sky_model_data.name, sizeof(g_model.name)) ;
  }
  else
	{
    memset( ModelNames[id1], ' ', sizeof(g_model.name)) ;
		id2_size = 0 ;
  }

  Eeprom32_source_address = (uint8_t *)&Eeprom_buffer.data.sky_model_data ;		// Get data from here
  Eeprom32_data_size = id2_size ;																					// This much
  Eeprom32_file_index = id1 ;																							// This file system entry
  Eeprom32_process_state = E32_BLANKCHECK ;
  ee32WaitFinished();
#endif
}

uint32_t write32_eeprom_2K( uint32_t eeAddress, register uint8_t *buffer )
{
	uint32_t i ;
	uint8_t *p ;

	if ( ( eeAddress & 0x00000FFF ) == 0 )
	{
		// Erase block
		eeprom_write_enable() ;
		p = Spi_tx_buf ;
		*p = 0x20 ;		// Block Erase command
		*(p+1) = eeAddress >> 16 ;
		*(p+2) = eeAddress >> 8 ;
		*(p+3) = eeAddress ;		// 3 bytes address
		spi_PDC_action( p, 0, 0, 4, 0 ) ;
		wdt_reset() ;
		// Wait for erase to complete
		while ( Spi_complete == 0 )
		{
			CoTickDelay(1) ;					// 2mS
			wdt_reset() ;
		}
		while ( eeprom_read_status() & 1 )
		{
			CoTickDelay(1) ;					// 2mS
			wdt_reset() ;
		}
	}
	for ( i = 0 ; i < 8 ; i += 1 )
	{
		// Write 256 byte page
		uint32_t j ;
		for ( j = 0 ; j < 256 ; j += 1 )
		{
			if ( buffer[j] != 0xFF )
			{
				break ;
			}
		}
		if ( j < 256 )
		{
			write32_eeprom_block( eeAddress, buffer, 256, 0 ) ;
			while ( eeprom_read_status() & 1 )
			{
				CoTickDelay(1) ;					// 2mS
				wdt_reset() ;
			}
		}
		buffer += 256 ;
		eeAddress += 256 ;
		wdt_reset() ;
	}
	return 0 ;
}

// Read eeprom data starting at random address
uint32_t read32_eeprom_data( uint32_t eeAddress, register uint8_t *buffer, uint32_t size, uint32_t immediate )
{
#ifdef SIMU
  assert(size);
  eeprom_pointer = eeAddress;
  eeprom_buffer_data = (char*)buffer;
  eeprom_buffer_size = size;
  eeprom_read_operation = true;
  Spi_complete = false;
  sem_post(eeprom_write_sem);
  int x;
#else
	register uint8_t *p ;
	register uint32_t x ;

	p = Spi_tx_buf ;
	*p = 3 ;		// Read command
	*(p+1) = eeAddress >> 16 ;
	*(p+2) = eeAddress >> 8 ;
	*(p+3) = eeAddress ;		// 3 bytes address
	spi_PDC_action( p, 0, buffer, 4, size ) ;
#endif

	if ( immediate )
	{
		return 0 ;		
	}
	for ( x = 0 ; x < 1000000 ; x += 1  )
	{
		if ( Spi_complete )
		{
			break ;				
		}
#ifdef SIMU
    sleep(5/*ms*/);
#endif
	}

	return x ;
}


uint32_t write32_eeprom_block( uint32_t eeAddress, register uint8_t *buffer, uint32_t size, uint32_t immediate )
{
	register uint8_t *p ;
	register uint32_t x ;

	eeprom_write_enable() ;

	p = Spi_tx_buf ;
	*p = 2 ;		// Write command
	*(p+1) = eeAddress >> 16 ;
	*(p+2) = eeAddress >> 8 ;
	*(p+3) = eeAddress ;		// 3 bytes address
	spi_PDC_action( p, buffer, 0, 4, size ) ;

	if ( immediate )
	{
		return 0 ;		
	}
	for ( x = 0 ; x < 100000 ; x += 1  )
	{
		if ( Spi_complete )
		{
			break ;				
		}        			
	}
	return x ; 
}

uint8_t byte_checksum( uint8_t *p, uint32_t size )
{
	uint32_t csum ;

	csum = 0 ;
	while( size )
	{
		csum += *p++ ;
		size -= 1 ;
	}
	return csum ;
}

uint32_t ee32_check_header( struct t_eeprom_header *hptr )
{
	uint8_t csum ;

	csum = byte_checksum( ( uint8_t *) hptr, 7 ) ;
	if ( csum == hptr->hcsum )
	{
		return 1 ;
	}
	return 0 ;
}

// Pass in an even block number, this and the next block will be checked
// to see which is the most recent, the block_no of the most recent
// is returned, with the corresponding data size if required
// and the sequence number if required
#ifdef FS8K
uint32_t get_current_block_number( uint32_t block_no, t_file_entry *pfs )
#else
uint32_t get_current_block_number( uint32_t block_no, uint16_t *p_size, uint32_t *p_seq )
#endif
{
	struct t_eeprom_header b0 ;
	struct t_eeprom_header b1 ;
  uint32_t sequence_no ;
  uint16_t size ;
#ifdef FS8K
	uint8_t flags ;
#endif
	
	read32_eeprom_data( block_no << 12, ( uint8_t *)&b0, sizeof(b0), EE_WAIT ) ;		// Sequence # 0
	read32_eeprom_data( (block_no+1) << 12, ( uint8_t *)&b1, sizeof(b1), EE_WAIT ) ;	// Sequence # 1

	if ( ee32_check_header( &b0 ) == 0 )
	{
		b0.sequence_no = 0 ;
		b0.data_size = 0 ;
		b0.flags = 0 ;
	}

	size = b0.data_size ;
  sequence_no = b0.sequence_no ;
#ifdef FS8K
	flags = b0.flags ;
#endif
	if ( ee32_check_header( &b0 ) == 0 )
	{
		if ( ee32_check_header( &b1 ) != 0 )
		{
  		size = b1.data_size ;
		  sequence_no = b1.sequence_no ;
			block_no += 1 ;
#ifdef FS8K
			flags = b1.flags ;
#endif
		}
		else
		{
			size = 0 ;
			sequence_no = 1 ;
#ifdef FS8K
			flags = 0 ;
#endif
		}
	}
	else
	{
#ifdef FS8K
		if (( b0.flags & BLOCK_8K ) == 0 )
		{
#endif
			// Not marked as an 8K block
			if ( ee32_check_header( &b1 ) != 0 )
			{
				if ( b1.sequence_no > b0.sequence_no )
				{
	  			size = b1.data_size ;
				  sequence_no = b1.sequence_no ;
#ifdef FS8K
					flags = b1.flags ;
#endif
					block_no += 1 ;
				}
			}
#ifdef FS8K
		}
#endif
	}

#ifdef FS8K
	if ( pfs )
	{
		if ( size == 0xFFFF )
		{
			size = 0 ;
		}
		pfs->size = size ;
  	if ( sequence_no == 0xFFFFFFFF )
		{  
			sequence_no = 0 ;
		}
		pfs->sequence_no = sequence_no ;
		if ( flags == 0xFF )
		{
			flags = 0 ;
		}
		pfs->flags = flags ;
	}
#else
	if ( size == 0xFFFF )
	{
		size = 0 ;
	}
  if ( p_size )
	{
		*p_size = size ;
	}
  if ( sequence_no == 0xFFFFFFFF )
	{  
		sequence_no = 0 ;
	}
  if ( p_seq )
	{
		*p_seq = sequence_no ;
	}
#endif
	return block_no ;
}

bool ee32LoadGeneral()
{
	uint16_t size ;
#ifdef PCB9XT
	EeLockEncoder = 1 ;
#endif	
#ifdef FS8K
	size = File_system.size ;
#else
	size = File_system[0].size ;
#endif
  memset(&g_eeGeneral, 0, sizeof(EEGeneral));

	if ( size > sizeof(EEGeneral) )
	{
		size = sizeof(EEGeneral) ;
	}

	if ( size )
	{
#ifdef FS8K
		read32_eeprom_data( ( File_system.block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&g_eeGeneral, size, 0 ) ;
#else
		read32_eeprom_data( ( File_system[0].block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&g_eeGeneral, size, 0 ) ;
#endif
	}
	else
	{
#ifdef PCB9XT
		EeLockEncoder = 0 ;
#endif	
		return false ;		// No data to load
	}
#ifdef PCB9XT
	EeLockEncoder = 0 ;
#endif	

  if(g_eeGeneral.myVers<MDSKYVERS)
      sysFlags = sysFLAG_OLD_EEPROM; // if old EEPROM - Raise flag

  g_eeGeneral.myVers   =  MDSKYVERS; // update myvers

#ifdef PCB9XT
	g_eeGeneral.is9Xtreme = 1 ;
#else
	g_eeGeneral.is9Xtreme = 0 ;
#endif

  uint16_t sum=0;
//  uint16_t *u = (uint16_t *) g_eeGeneral.calibMid ;
//  if(size>(43)) for(uint8_t i=0; i<12;i++) sum += *u++ ;
  if(size>(43)) sum = evalChkSum() ;
	else return false ;
  return g_eeGeneral.chkSum == sum;
}

void ee32WaitLoadModel(uint8_t id)
{
	// Wait for EEPROM to be idle
	if ( General_timer )
	{
		General_timer = 1 ;		// Make these happen soon
	}
	if ( Model_timer )
	{
		Model_timer = 1 ;
	}

	while( ( Eeprom32_process_state != E32_IDLE )
			|| ( General_timer )
			|| ( Model_timer )
			|| ( Ee32_model_delete_pending)
			|| ( Ee32_general_write_pending)
			|| ( Ee32_model_write_pending) )
	{
		mainSequence( NO_MENU ) ;		// Wait for EEPROM to be IDLE
	}

	ee32LoadModel( id ) ;		// Now get the model
}

extern void closeLogs( void ) ;

void ee32LoadModel(uint8_t id)
{
	uint16_t size ;
	uint8_t version = 255 ;

  closeLogs() ;

#ifdef PCB9XT
	EeLockEncoder = 1 ;
#endif	
    if(id<MAX_MODELS)
    {
#ifdef FS8K
			size = FileSizes[id] ;
#else
			size = File_system[id+1].size ;
#endif
			if ( sizeof(g_model) < 720 )
			{
				version = 0 ;
			}
			memset(&g_model, 0, sizeof(g_model));

			if ( size < 720 )
			{
				memset(&Eeprom_buffer.data.oldmodel, 0, sizeof(Eeprom_buffer.data.oldmodel));
				if ( size > sizeof(Eeprom_buffer.data.oldmodel) )
				{
					size = sizeof(Eeprom_buffer.data.oldmodel) ;
				}
			}
       
			if ( size > sizeof(g_model) )
			{
				size = sizeof(g_model) ;
			}
			 
      if(size<256) // if not loaded a fair amount
      {
        modelDefault(id) ;
      }
			else
			{
				if ( size < 720 )
				{
#ifndef FS8K
					read32_eeprom_data( ( File_system[id+1].block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&Eeprom_buffer.data.oldmodel, size, 0 ) ;
					convertModel( &g_model, &Eeprom_buffer.data.oldmodel ) ;
#endif
				}
				else
				{
#ifdef FS8K
					read32_eeprom_data( ( AllocTable[id+1] << 13) + sizeof( struct t_eeprom_header), ( uint8_t *)&g_model, size, 0 ) ;
#else
					read32_eeprom_data( ( File_system[id+1].block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&g_model, size, 0 ) ;
#endif
				}	 
				
				if ( version != 255 )
				{
					g_model.modelVersion = 0 ;					// default version number
				}
			}

			validateName( (uint8_t *)g_model.name, sizeof(g_model.name) ) ;

//      for(uint8_t i=0; i<sizeof(g_model.name);i++) // makes sure name is valid
//      {
//          uint8_t idx = char2idx(g_model.name[i]);
//          g_model.name[i] = idx2char(idx);
//      }

			if ( g_model.numBlades == 0 )
			{
				g_model.numBlades = g_model.xnumBlades + 2 ;				
			}

// check for updating mix sources
	if ( g_model.modelVersion < 2 )
	{
   	for(uint32_t i=0;i<MAX_MIXERS;i++)
		{
			SKYMixData *md = &g_model.mixData[i] ;
      if (md->srcRaw)
			{
       	if (md->srcRaw <= 4)		// Stick
				{
					md->srcRaw = modeFixValue( md->srcRaw-1 ) ;
				}
			}
		}
		for (uint32_t i = 0 ; i < NUM_SKYCSW ; i += 1 )
		{
    	SKYCSwData *cs = &g_model.customSw[i];
    	uint8_t cstate = CS_STATE(cs->func);
			uint8_t t = 0 ;
    	if(cstate == CS_VOFS)
			{
				t = 1 ;
			}
			else if(cstate == CS_VCOMP)
			{
				t = 1 ;
      	if (cs->v2)
				{
    		  if (cs->v2 <= 4)		// Stick
					{
    	    	cs->v2 = modeFixValue( cs->v2-1 ) ;
					}
				}
			}
			if ( t )
			{
      	if (cs->v1)
				{
    		  if (cs->v1 <= 4)		// Stick
					{
    	    	cs->v1 = modeFixValue( cs->v1-1 ) ;
					}
				}
			}
		}
		ExpoData texpoData[4] ;
    memmove( &texpoData, &g_model.expoData, sizeof(texpoData) ) ;
		for (uint32_t i = 0 ; i < 4 ; i += 1 )
		{
			uint8_t dest = modeFixValue( i ) - 1 ;
    	memmove( &g_model.expoData[dest], &texpoData[i], sizeof(texpoData[0]) ) ;
		}
		alert(PSTR(STR_CHK_MIX_SRC));
		g_model.modelVersion = 2 ;
		STORE_MODELVARS ;
	}

		if ( g_model.modelVersion < 3 )
		{
			for (uint32_t i = 0 ; i < NUM_SKYCSW ; i += 1 )
			{
	    	SKYCSwData *cs = &g_model.customSw[i];
				if ( cs->func == CS_LATCH )
				{
					cs->func = CS_GREATER ;
				}
				if ( cs->func == CS_FLIP )
				{
					cs->func = CS_LESS ;
				}
			}
			g_model.modelVersion = 3 ;
	 		STORE_MODELVARS ;
		}

		ppmInValid = 0 ;

		if ( g_model.bt_telemetry < 2 )
		{
  		FrskyAlarmSendState |= 0x40 ;		// Get RSSI Alarms
		}
  	FRSKY_setModelAlarms();
  }

#ifdef REVX
	if ( g_model.telemetryRxInvert )
	{
		setMFP() ;
	}
	else
	{
		clearMFP() ;
	}
#endif

// // Sort telemetry options
	if ( g_model.telemetryProtocol == TELEMETRY_UNDEFINED )
	{
		g_model.telemetryProtocol = TELEMETRY_FRSKY ;	// default
		if ( g_model.FrSkyUsrProto )
		{
			g_model.telemetryProtocol = TELEMETRY_WSHHI ;
		}
		if ( g_model.DsmTelemetry )
		{
			g_model.telemetryProtocol = TELEMETRY_DSM ;
		}
	}
	checkXyCurve() ;
#if defined(IMAGE_128)
	loadModelImage() ;
#endif
	// Now check the sub_protocol(s)
	if ( g_model.sub_protocol == 0 )
	{
		if ( g_model.not_sub_protocol != 0 )
		{
			g_model.sub_protocol = g_model.not_sub_protocol ;
			g_model.not_sub_protocol = 0 ;
		}
	}
//	if ( g_model.xsub_protocol == 0 )
//	{
//		if ( g_model.not_xsub_protocol != 0 )
//		{
//			g_model.xsub_protocol = g_model.not_xsub_protocol ;
//			g_model.not_xsub_protocol = 0 ;
//		}
//	}
#ifdef XFIRE
 #ifdef REVX
	if ( g_model.protocol > PROT_MAX + 1 )
 #else
	if ( g_model.protocol > PROT_MAX )
 #endif
#else
	if ( g_model.protocol > PROT_MAX )
#endif
	{
		if ( g_model.protocol != PROTO_OFF )
		{
			g_model.protocol = 0 ;
		}
	}
#ifdef XFIRE
 #ifdef PCB9XT
	if ( g_model.xprotocol > PROT_MAX + 1 )
 #else
	if ( g_model.xprotocol > PROT_MAX )
 #endif
#else
	if ( g_model.xprotocol > PROT_MAX )
#endif
	{
		if ( g_model.xprotocol != PROTO_OFF )
		{
			g_model.xprotocol = 0 ;
		}
	}
	validateProtocolOptions( 0 ) ;
	validateProtocolOptions( 1 ) ;

	AltitudeZeroed = 0 ;
#ifdef PCB9XT
	EeLockEncoder = 0 ;
#endif	
}

bool eeModelExists(uint8_t id)
{
#ifdef FS8K
	return ( FileSizes[id] > 0 ) ;
#else
	return ( File_system[id+1].size > 0 ) ;
#endif
}

void ee32LoadModelName( uint8_t id, unsigned char*buf, uint8_t len )
{
	if(id<=MAX_MODELS)
  {
		memset(buf,' ',len) ;
#ifdef FS8K
		if ( FileSizes[id-1] )
		{
			id = AllocTable[id] ;
			read32_eeprom_data( ( id << 13 ) + 8, ( uint8_t *)buf, sizeof(g_model.name), 0 ) ;
		}
#else
		if ( File_system[id].size > sizeof(g_model.name) )
		{
			read32_eeprom_data( ( File_system[id].block_no << 12) + 8, ( uint8_t *)buf, sizeof(g_model.name), 0 ) ;
		}
#endif
  }
}

#ifdef FS8K
uint16_t getFileSize( uint32_t index ) // index is 0 to 59
{
	struct t_eeprom_header b0 ;
	index = AllocTable[index+1] ;	
	read32_eeprom_data( index << 13, ( uint8_t *)&b0, sizeof(b0), EE_WAIT ) ;
	if ( ee32_check_header( &b0) )
	{
		return b0.data_size ;
	}
	else
	{
		return 0 ;
	}
}
#endif

void fill_file_index()
{
	uint32_t i ;
#ifdef FS8K
	File_system.block_no = get_current_block_number( 0, &File_system ) ;
	for ( i = 0 ; i < MAX_MODELS ; i += 1 )
	{
		FileSizes[i] = getFileSize(i) ;
	}
#else
	for ( i = 0 ; i < MAX_MODELS + 1 ; i += 1 )
	{
		File_system[i].block_no = get_current_block_number( i * 2, &File_system[i].size, &File_system[i].sequence_no ) ;
	}
#endif
}

void init_eeprom()
{
	fill_file_index() ;
	ee32_read_model_names() ;
	Eeprom32_process_state = E32_IDLE ;
}


uint32_t ee32_check_finished()
{
	if ( ( Eeprom32_process_state != E32_IDLE )
			|| ( General_timer )
			|| ( Model_timer )
			|| ( Ee32_model_delete_pending)
			|| ( Ee32_general_write_pending)
			|| ( Ee32_model_write_pending) )
	{
		if ( General_timer )
		{
			General_timer = 1 ;		// Make these happen soon
		}
		if ( Model_timer )
		{
			Model_timer = 1 ;
		}
		ee32_process() ;
		return 0 ;
	}
	return 1 ;
}


#ifdef FS8K
void waitEepromDone()
{
	while ( eeprom_read_status() & 1 )
	{
		wdt_reset() ;
	}
}

void eraseBlock( uint32_t eeAddress )
{
//	uint32_t i ;
	uint8_t *p ;

	// Erase block
	eeprom_write_enable() ;
	p = Spi_tx_buf ;
	*p = 0x20 ;		// Block Erase command
	*(p+1) = eeAddress >> 16 ;
	*(p+2) = eeAddress >> 8 ;
	*(p+3) = eeAddress ;		// 3 bytes address
	spi_PDC_action( p, 0, 0, 4, 0 ) ;
	wdt_reset() ;
	// Wait for erase to complete
	while ( Spi_complete == 0 )
	{
//			CoTickDelay(1) ;					// 2mS
		wdt_reset() ;
	}
	waitEepromDone() ;
}
#endif


void ee32_process()
{
	register uint8_t *p ;
	register uint8_t *q ;
	register uint32_t x ;
	register uint32_t eeAddress ;

//	return 0 ;

	if ( General_timer )
	{
		if ( --General_timer == 0 )
		{
			
			// Time to write g_eeGeneral
			Ee32_general_write_pending = 1 ;
		}
	}
	if ( Model_timer )
	{
		if ( --Model_timer == 0 )
		{
			
			// Time to write model
			Ee32_model_write_pending = 1 ;
		}
	}

#ifdef PCB9XT
	if ( ( Eeprom32_process_state == E32_IDLE ) || Spi_complete )
	{
		if ( EeLockEncoder == 0 )
		{
		
		// Poll an Arduino for encoder data
			uint16_t result ;
extern uint16_t readSpiEncoder() ;
			result = readSpiEncoder() ;


			if ( ( result & 0xFE00 ) == 0xAA00 )
			{
extern uint8_t EncoderI2cData[] ;
extern uint8_t SpiEncoderValid ;
				EncoderI2cData[1] = (result & 0x0100) ? 1 : 0 ;
				EncoderI2cData[0] = result ;
				SpiEncoderValid = 1 ;
			}
		}
	}
#endif

#ifdef FS8K
	if ( Eeprom32_process_state == E32_IDLE )
	{
		if ( Ee32_general_write_pending )
		{
			Ee32_general_write_pending = 0 ;			// clear flag

			// Check we can write, == block is blank

			Eeprom32_source_address = (uint8_t *)&g_eeGeneral ;		// Get data fromm here
			Eeprom32_data_size = sizeof(g_eeGeneral) ;						// This much
			Eeprom32_file_index = 0 ;								// This file system entry
			Eeprom32_state_after_erase = E32_WRITESTART ;
			Eeprom32_state_after_write = E32_IDLE ;
			Eeprom32_process_state = E32_BLANKCHECK ;
		}
		else if ( Ee32_model_write_pending )
		{
			Ee32_model_write_pending = 0 ;			// clear flag

			// Check we can write, == block is blank
			Eeprom32_source_address = (uint8_t *)&g_model ;		// Get data from here
			Eeprom32_data_size = sizeof(g_model) ;						// This much
			Eeprom32_file_index = Dirty.Model_dirty ;					// This file system entry
			Eeprom32_state_after_erase = E32_BLANK2CHECK ;
			Eeprom32_process_state = E32_BLANKCHECK ;
//			Writing_model = Dirty.Model_dirty ;
		}
		else if ( Ee32_model_delete_pending )
		{
			Eeprom32_source_address = (uint8_t *)&g_model ;		// Get data from here
			Eeprom32_data_size = 0 ;													// This much
			Eeprom32_file_index = Ee32_model_delete_pending ;	// This file system entry
			Ee32_model_delete_pending = 0 ;
			Eeprom32_state_after_erase = E32_IDLE ;
			Eeprom32_process_state = E32_BLANKCHECK ;
		}
	}

	if ( Eeprom32_process_state == E32_BLANK2CHECK )
	{
		eeAddress = Eeprom32_address + 4096 ;
		eeprom_write_enable() ;
		p = Spi_tx_buf ;
		*p = 0x20 ;		// Block Erase command
		*(p+1) = eeAddress >> 16 ;
		*(p+2) = eeAddress >> 8 ;
		*(p+3) = eeAddress ;		// 3 bytes address
		spi_PDC_action( p, 0, 0, 4, 0 ) ;
		Eeprom32_process_state = E32_ERASESENDING ;
		Eeprom_buffer.header.flags = BLOCK_8K ;
		Eeprom32_state_after_erase = E32_WRITESTART ;
		Eeprom32_state_after_write = E32_WRITEALLOC ;
	}

	if ( Eeprom32_process_state == E32_BLANK3CHECK )
	{
		eeAddress = (ControlBlock == 126) ? 127 : 126 ;
		eeAddress <<= 12 ;
		eeprom_write_enable() ;
		p = Spi_tx_buf ;
		*p = 0x20 ;		// Block Erase command
		*(p+1) = eeAddress >> 16 ;
		*(p+2) = eeAddress >> 8 ;
		*(p+3) = eeAddress ;		// 3 bytes address
		spi_PDC_action( p, 0, 0, 4, 0 ) ;
		Eeprom32_process_state = E32_ERASESENDING ;
	}

	if ( Eeprom32_process_state == E32_WRITEALLOC )
	{
		write32_eeprom_block( (ControlBlock << 12) + ControlOffset, AllocTable, 64, EE_NO_WAIT ) ;
		ControlOffset += 64 ;
		Eeprom32_process_state = E32_WRITESENDING ;
		Eeprom32_state_after_write = E32_CHECKALLOC ;
	}

	if ( Eeprom32_process_state == E32_CHECKALLOC )
	{
		if ( ControlOffset >= 4096 )
		{
			// Need to use other control block
//  erase other block
//  write the 64 bytes, 64 bytes into block
//  write first 4 bytes with 0x55AA3CC3
//  Update Offset to block start + 128
//  write first 4 bytes of other block to 0x00000000
			Eeprom32_state_after_erase = E32_WRITECONTROL ;
			Eeprom32_process_state = E32_BLANK3CHECK ;
		}
		else if ( ControlOffset == 128 )
		{
			eeAddress = (ControlBlock == 126) ? 127 : 126 ;
			eeAddress <<= 12 ;
			Eeprom32_state_after_write = E32_IDLE ;
			Key.keyLong = 0 ;
			write32_eeprom_block( eeAddress, Key.key, 4, EE_NO_WAIT ) ;
			Eeprom32_process_state = E32_WRITESENDING ;
		}
		else
		{
			Eeprom32_process_state = E32_IDLE ;
		}
	}
	
	if ( Eeprom32_process_state == E32_WRITECONTROL )
	{
		ControlBlock = (ControlBlock == 126) ? 127 : 126 ;
		Key.keyLong = 0x55AA3CC3 ;
		ControlOffset = 64 ;
		Eeprom32_state_after_write = E32_WRITEALLOC ;
		write32_eeprom_block( ControlBlock << 12, Key.key, 4, EE_NO_WAIT ) ;
		Eeprom32_process_state = E32_WRITESENDING ;
	}
	if ( Eeprom32_process_state == E32_BLANKCHECK )
	{
		if ( Eeprom32_file_index == 0 )
		{
			eeAddress = File_system.block_no ^ 1 ;
			eeAddress <<= 12 ;		// Block start address
		}
		else
		{
			eeAddress = AllocTable[61] ;	// First free buffer
			AllocTable[61] = AllocTable[62] ; // Shunt next free buffer down
			AllocTable[62] = AllocTable[Eeprom32_file_index] ;
			AllocTable[Eeprom32_file_index] = eeAddress ;
			eeAddress <<= 13 ;		// Block start address
		}
		Eeprom32_address = eeAddress ;						// Where to put new data
		eeprom_write_enable() ;
		p = Spi_tx_buf ;
		*p = 0x20 ;		// Block Erase command
		*(p+1) = eeAddress >> 16 ;
		*(p+2) = eeAddress >> 8 ;
		*(p+3) = eeAddress ;		// 3 bytes address
		spi_PDC_action( p, 0, 0, 4, 0 ) ;
		Eeprom32_process_state = E32_ERASESENDING ;
		Eeprom_buffer.header.flags = 0 ;
	}

	if ( Eeprom32_process_state == E32_WRITESTART )
	{
		uint32_t total_size ;
		p = Eeprom32_source_address ;
		q = (uint8_t *)&Eeprom_buffer.data ;
    if (p != q)
		{
			for ( x = 0 ; x < Eeprom32_data_size ; x += 1 )
			{
				*q++ = *p++ ;			// Copy the data to temp buffer
			}
		}
		if (Eeprom32_file_index == 0 )
		{
			Eeprom_buffer.header.sequence_no = ++File_system.sequence_no ;
			File_system.size = Eeprom_buffer.header.data_size = Eeprom32_data_size ;
		}
		else
		{
			Eeprom_buffer.header.sequence_no = 1 ;
			FileSizes[Eeprom32_file_index-1] = Eeprom_buffer.header.data_size = Eeprom32_data_size ;
		}
		Eeprom_buffer.header.hcsum = byte_checksum( (uint8_t *)&Eeprom_buffer, 7 ) ;
		total_size = Eeprom32_data_size + sizeof( struct t_eeprom_header ) ;
		eeAddress = Eeprom32_address ;		// Block start address
		x = total_size / 256 ;	// # sub blocks
		x <<= 8 ;						// to offset address
		eeAddress += x ;		// Add it in
		p = (uint8_t *) &Eeprom_buffer ;
		p += x ;						// Add offset
		x = total_size % 256 ;	// Size of last bit
		if ( x == 0 )						// Last bit empty
		{
			x = 256 ;
			p -= x ;
			eeAddress -= x ;
		}
		Eeprom32_buffer_address = p ;
		Eeprom32_address = eeAddress ;
		write32_eeprom_block( eeAddress, p, x, 1 ) ;
		Eeprom32_process_state = E32_WRITESENDING ;
	}

	if ( Eeprom32_process_state == E32_WRITESENDING )
	{
		if ( Spi_complete )
		{
			Eeprom32_process_state = E32_WRITEWAITING ;
		}			
	}		

	if ( Eeprom32_process_state == E32_WRITEWAITING )
	{
		x = eeprom_read_status() ;
		if ( ( x & 1 ) == 0 )
		{
			uint32_t endMask = 0x0FFF ;
			if ( Eeprom_buffer.header.flags & BLOCK_8K )
			{
				endMask = 0x1FFF ;
			}
			if ( ( Eeprom32_address & endMask ) != 0 )		// More to write
			{
				Eeprom32_address -= 256 ;
				Eeprom32_buffer_address -= 256 ;
				write32_eeprom_block( Eeprom32_address, Eeprom32_buffer_address, 256, 1 ) ;
				Eeprom32_process_state = E32_WRITESENDING ;
			}
			else
			{
				if ( Eeprom32_file_index == 0 )
				{
					File_system.block_no ^= 1 ;		// This is now the current block
					Eeprom32_process_state = E32_IDLE ;
				}
				else
				{
					if ( ( Eeprom32_state_after_write == E32_CHECKALLOC ) || ( Eeprom32_state_after_write == E32_WRITEALLOC ) )
					{
						Eeprom32_process_state = Eeprom32_state_after_write ;
					}
					else
					{
						Eeprom32_process_state = E32_IDLE ;
					}
				}
			}
		}
	}	

	if ( Eeprom32_process_state == E32_ERASESENDING )
	{
		if ( Spi_complete )
		{
			Eeprom32_process_state = E32_ERASEWAITING ;
		}			
	}	
		
	if ( Eeprom32_process_state == E32_ERASEWAITING )
	{
		x = eeprom_read_status() ;
		if ( ( x & 1 ) == 0 )
		{ // Command finished
			Eeprom32_process_state = Eeprom32_state_after_erase ;
		}			
	}
#else
	if ( Eeprom32_process_state == E32_IDLE )
	{
		if ( Ee32_general_write_pending )
		{
			Ee32_general_write_pending = 0 ;			// clear flag

			// Check we can write, == block is blank

			Eeprom32_source_address = (uint8_t *)&g_eeGeneral ;		// Get data fromm here
			Eeprom32_data_size = sizeof(g_eeGeneral) ;						// This much
			Eeprom32_file_index = 0 ;								// This file system entry
			Eeprom32_process_state = E32_BLANKCHECK ;
		}
		else if ( Ee32_model_write_pending )
		{
			Ee32_model_write_pending = 0 ;			// clear flag

			// Check we can write, == block is blank
			Eeprom32_source_address = (uint8_t *)&g_model ;		// Get data from here
			Eeprom32_data_size = sizeof(g_model) ;						// This much
			Eeprom32_file_index = Dirty.Model_dirty ;					// This file system entry
			Eeprom32_process_state = E32_BLANKCHECK ;
//			Writing_model = Dirty.Model_dirty ;
		}
		else if ( Ee32_model_delete_pending )
		{
			Eeprom32_source_address = (uint8_t *)&g_model ;		// Get data from here
			Eeprom32_data_size = 0 ;													// This much
			Eeprom32_file_index = Ee32_model_delete_pending ;	// This file system entry
			Ee32_model_delete_pending = 0 ;
			Eeprom32_process_state = E32_BLANKCHECK ;
		}
	}

	if ( Eeprom32_process_state == E32_BLANKCHECK )
	{
		eeAddress = File_system[Eeprom32_file_index].block_no ^ 1 ;
		eeAddress <<= 12 ;		// Block start address
		Eeprom32_address = eeAddress ;						// Where to put new data
				eeprom_write_enable() ;
				p = Spi_tx_buf ;
				*p = 0x20 ;		// Block Erase command
				*(p+1) = eeAddress >> 16 ;
				*(p+2) = eeAddress >> 8 ;
				*(p+3) = eeAddress ;		// 3 bytes address
				spi_PDC_action( p, 0, 0, 4, 0 ) ;
				Eeprom32_process_state = E32_ERASESENDING ;
				Eeprom32_state_after_erase = E32_WRITESTART ;
	}

	if ( Eeprom32_process_state == E32_WRITESTART )
	{
		uint32_t total_size ;
		p = Eeprom32_source_address ;
		q = (uint8_t *)&Eeprom_buffer.data ;
    if (p != q)
		{
			for ( x = 0 ; x < Eeprom32_data_size ; x += 1 )
			{
				*q++ = *p++ ;			// Copy the data to temp buffer
			}
		}
		Eeprom_buffer.header.sequence_no = ++File_system[Eeprom32_file_index].sequence_no ;
		File_system[Eeprom32_file_index].size = Eeprom_buffer.header.data_size = Eeprom32_data_size ;
		Eeprom_buffer.header.flags = 0 ;
		Eeprom_buffer.header.hcsum = byte_checksum( (uint8_t *)&Eeprom_buffer, 7 ) ;
		total_size = Eeprom32_data_size + sizeof( struct t_eeprom_header ) ;
		eeAddress = Eeprom32_address ;		// Block start address
		x = total_size / 256 ;	// # sub blocks
		x <<= 8 ;						// to offset address
		eeAddress += x ;		// Add it in
		p = (uint8_t *) &Eeprom_buffer ;
		p += x ;						// Add offset
		x = total_size % 256 ;	// Size of last bit
		if ( x == 0 )						// Last bit empty
		{
			x = 256 ;
			p -= x ;
			eeAddress -= x ;
		}
		Eeprom32_buffer_address = p ;
		Eeprom32_address = eeAddress ;
		write32_eeprom_block( eeAddress, p, x, 1 ) ;
		Eeprom32_process_state = E32_WRITESENDING ;
	}

	if ( Eeprom32_process_state == E32_WRITESENDING )
	{
		if ( Spi_complete )
		{
			Eeprom32_process_state = E32_WRITEWAITING ;
		}			
	}		

	if ( Eeprom32_process_state == E32_WRITEWAITING )
	{
		x = eeprom_read_status() ;
		if ( ( x & 1 ) == 0 )
		{
			if ( ( Eeprom32_address & 0x0FFF ) != 0 )		// More to write
			{
				Eeprom32_address -= 256 ;
				Eeprom32_buffer_address -= 256 ;
				write32_eeprom_block( Eeprom32_address, Eeprom32_buffer_address, 256, 1 ) ;
				Eeprom32_process_state = E32_WRITESENDING ;
			}
			else
			{
				File_system[Eeprom32_file_index].block_no ^= 1 ;		// This is now the current block
				Eeprom32_process_state = E32_IDLE ;
			}
		}
	}	

	if ( Eeprom32_process_state == E32_ERASESENDING )
	{
		if ( Spi_complete )
		{
			Eeprom32_process_state = E32_ERASEWAITING ;
		}			
	}	
		
	if ( Eeprom32_process_state == E32_ERASEWAITING )
	{
		x = eeprom_read_status() ;
		if ( ( x & 1 ) == 0 )
		{ // Command finished
			Eeprom32_process_state = Eeprom32_state_after_erase ;
		}			
	}
#endif
}


uint32_t unprotect_eeprom()
{
 	register uint8_t *p ;
	uint32_t result ;
	eeprom_write_enable() ;
		
	p = Spi_tx_buf ;
	*p = 0x39 ;		// Unprotect sector command
	*(p+1) = 0 ;
	*(p+2) = 0 ;
	*(p+3) = 0 ;		// 3 bytes address

	result = spi_operation( p, Spi_rx_buf, 4 ) ;
#ifdef PCB9XT
	GPIOA->BSRRL = GPIO_Pin_SPI_EE_CS ;		// output disable
#endif	 
	return result ;
}

void convertSwitch( int8_t *p )
{
	if ( *p <= -MAX_DRSWITCH )
	{
		*p -= (NUM_SKYCSW - NUM_CSW) ;
	}
	if ( *p >= MAX_DRSWITCH )
	{
		*p += (NUM_SKYCSW - NUM_CSW) ;
	}
}


void convertModel( SKYModelData *dest, ModelData *source )
{
	uint32_t i ;
	memset( dest, 0, sizeof(*dest) ) ;
  memcpy( dest->name, source->name, MODEL_NAME_LEN) ;
	dest->modelVoice = source->modelVoice ;
//	dest->RxNum_unused = source->RxNum ;
	dest->traineron = source->traineron ;
	dest->FrSkyUsrProto = source->FrSkyUsrProto ;
	dest->FrSkyGpsAlt = source->FrSkyGpsAlt ;
	dest->FrSkyImperial = source->FrSkyImperial ;
	dest->FrSkyAltAlarm = source->FrSkyAltAlarm ;
	dest->modelVersion = source->version ;
	dest->protocol = source->protocol ;
	dest->ppmNCH = source->ppmNCH ;
	dest->thrTrim = source->thrTrim ;
	dest->xnumBlades = source->numBlades ;
	dest->thrExpo = source->thrExpo ;
	dest->trimInc = source->trimInc ;
	dest->ppmDelay = source->ppmDelay ;
	dest->trimSw = source->trimSw ;
	dest->beepANACenter = source->beepANACenter ;
	dest->pulsePol = source->pulsePol ;
	dest->extendedLimits = source->extendedLimits ;
	dest->swashInvertELE = source->swashInvertELE ;
	dest->swashInvertAIL = source->swashInvertAIL ;
	dest->swashInvertCOL = source->swashInvertCOL ;
	dest->swashType = source->swashType ;
	dest->swashCollectiveSource = source->swashCollectiveSource ;
	dest->swashRingValue = source->swashRingValue ;
	dest->ppmFrameLength = source->ppmFrameLength ;
	for ( i = 0 ; i < MAX_MIXERS ; i += 1 )
	{
		MixData *src = &source->mixData[i] ;
		SKYMixData *dst = &dest->mixData[i] ;
		dst->destCh = src->destCh ;
		dst->srcRaw = src->srcRaw ;
		if ( dst->srcRaw == NUM_XCHNRAW+1 )		// MIX_3POS
		{
			dst->srcRaw += NUM_SKYCHNOUT - NUM_CHNOUT ;
		}

		dst->weight = src->weight ;
		dst->swtch = src->swtch ;
		convertSwitch( &dst->swtch ) ;
		
		dst->curve = src->curve ;
		dst->delayUp = src->delayUp * 10 ;
		dst->delayDown = src->delayDown * 10 ;
		dst->speedUp = src->speedUp * 10 ;
		dst->speedDown = src->speedDown * 10 ;
		dst->carryTrim = src->carryTrim ;
		dst->mltpx = src->mltpx ;
		dst->mixWarn = src->mixWarn ;
		dst->disableExpoDr = src->enableFmTrim ;
		dst->sOffset = src->sOffset ;
	}
	for ( i = 0 ; i < NUM_CHNOUT ; i += 1 )
	{
		dest->limitData[i] = source->limitData[i] ;
	}
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		dest->expoData[i] = source->expoData[i] ;
		convertSwitch( &dest->expoData[i].drSw1 ) ;
		convertSwitch( &dest->expoData[i].drSw2 ) ;
		dest->trim[i] = source->trim[i] ;
	}
  memcpy( dest->curves5, source->curves5, sizeof(source->curves5) ) ;
  memcpy( dest->curves9, source->curves9, sizeof(source->curves9) ) ;
	for ( i = 0 ; i < NUM_CSW ; i += 1 )
	{
		CSwData *src = &source->customSw[i] ;
		SKYCSwData *dst = &dest->customSw[i] ;
		dst->v1 = src->v1 ;
		dst->v2 = src->v2 ;
		dst->func = src->func ;
  	switch (dst->func)
		{
  		case (CS_AND) :
  		case (CS_OR) :
  		case (CS_XOR) :
				convertSwitch( &dst->v1 ) ;
				convertSwitch( &dst->v2 ) ;
      break;
  		case (CS_VPOS):
  		case (CS_VNEG):
  		case (CS_APOS):
  		case (CS_ANEG):
				if ( dst->v1-1 >= CHOUT_BASE+NUM_CHNOUT )
				{
					dst->v1 += NUM_SKYCHNOUT - NUM_CHNOUT ;
				}
				if ( dst->v2-1 >= CHOUT_BASE+NUM_CHNOUT )
				{
					dst->v2 += NUM_SKYCHNOUT - NUM_CHNOUT ;
				}
      break;
		}
		dst->andsw = src->andsw ;
	}
	dest->frSkyVoltThreshold = source->frSkyVoltThreshold ;
	dest->bt_telemetry = source->bt_telemetry ;
	dest->numVoice = source->numVoice ;
	if ( dest->numVoice )
	{
		dest->numVoice += NUM_SKYCHNOUT - NUM_CHNOUT ;
	}
	for ( i = 0 ; i < NUM_CHNOUT ; i += 1 )
	{
		SafetySwData *src = &source->safetySw[i] ;
		SKYSafetySwData *dst = &dest->safetySw[i] ;
		if ( i < (uint32_t)NUM_CHNOUT - dest->numVoice )
		{
			dst->opt.ss.swtch = src->opt.ss.swtch ;
			dst->opt.ss.mode = src->opt.ss.mode ;
			dst->opt.ss.val = src->opt.ss.val ;
			if ( dst->opt.ss.mode == 3 )
			{
				dst->opt.ss.mode = 0 ;
			}
			switch ( dst->opt.ss.mode )
			{
				case 0 :
				case 1 :
				case 2 :
					convertSwitch( &dst->opt.ss.swtch ) ;
				break ;
			}
		}
		else
		{
			dst->opt.vs.vswtch = src->opt.vs.vswtch ;
			dst->opt.vs.vmode = src->opt.vs.vmode ;
			dst->opt.vs.vval = src->opt.vs.vval ;
			convertSwitch( (int8_t *)&dst->opt.vs.vswtch ) ;
		}
	}

	for ( i = 0 ; i < 2 ; i += 1 )
	{
		FrSkyChannelData *src = &source->frsky.channels[i] ;
		SKYFrSkyChannelData *dst = &dest->frsky.channels[i] ;
		dst->lratio = src->ratio ;
//		dst->alarms_value[0] = src->alarms_value[0] ;
//		dst->alarms_value[1] = src->alarms_value[1] ;
//		dst->alarms_level = src->alarms_level ;
//		dst->alarms_greater = src->alarms_greater ;
		dst->units = src->type ;
//		dst->gain = 1 ;
	}

	for ( i = 0 ; i < 2 ; i += 1 )
	{
		dest->timer[i] = source->timer[i] ;
  	if( dest->timer[i].tmrModeB>=(MAX_DRSWITCH))	 //momentary on-off
		{
			dest->timer[i].tmrModeB += (NUM_SKYCSW - NUM_CSW) ;
		}
	}

	dest->frskyAlarms = source->frskyAlarms ;

  memcpy( &dest->customDisplayIndex[0], &source->customDisplayIndex[0], 6 ) ;

}

void setModelFilename( uint8_t *filename, uint8_t modelIndex, uint32_t type )
{
	uint8_t *bptr ;
	uint32_t i ;
	
	bptr = cpystr( filename, type ? (uint8_t *)"/TEXT/" : (uint8_t *)"/MODELS/" ) ;
  memcpy( bptr, ModelNames[modelIndex], sizeof(g_model.name)) ;
	bptr += sizeof(g_model.name) - 1 ;
	for ( i = 0 ; i < sizeof(g_model.name) ; i += 1 )
	{
		if ( *bptr && ( *bptr != ' ' ) )
		{
			break ;
		}
		else
		{
			bptr -= 1 ;
		}
	}
	bptr += 1 ;
	if ( i >= sizeof(g_model.name) )
	{
		*bptr++ = 'x' ;
	}
	cpystr( bptr, type ? (uint8_t *)".txt" : (uint8_t *)".eepm" ) ;		// ".eepm"
}

#ifndef PCBDUE

static const uint8_t base64digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;

// write XML file
FRESULT writeXMLfile( FIL *archiveFile, uint8_t *data, uint32_t size, UINT *totalWritten )
{
  UINT written ;
  UINT total = 0 ;
	uint32_t i ;
	FRESULT result ;
  uint8_t bytes[104] ;

  result = f_write( archiveFile, (BYTE *)"<!DOCTYPE ERSKY9X_EEPROM_FILE>\n<ERSKY9X_EEPROM_FILE>\n <MODEL_DATA number=\0420\042>\n  <Version>", 89, &written ) ;
	total += written ;
  bytes[0] = MDSKYVERS + '0' ;
  bytes[1] = 0 ;
  result = f_write( archiveFile, bytes, 1, &written) ;
	total += written ;
  result = f_write( archiveFile, (BYTE *)"</Version>\n  <Name>", 19, &written) ;
	total += written ;
  result = f_write( archiveFile, (BYTE *)Eeprom_buffer.data.sky_model_data.name, sizeof(g_model.name), &written) ;
	total += written ;
  result = f_write( archiveFile, (BYTE *)"</Name>\n  <Data><![CDATA[", 25, &written) ;
	total += written ;

// Send base 64 here
	i = 0 ;
	while ( size > 2 )
	{
		bytes[i++] = base64digits[data[0] >> 2];
 		bytes[i++] = base64digits[((data[0] << 4) & 0x30) | (data[1] >> 4)];
		bytes[i++] = base64digits[((data[1] << 2) & 0x3c) | (data[2] >> 6)];
		bytes[i++] = base64digits[data[2] & 0x3f];
		data += 3 ;
		size -= 3 ;
		if ( i >= 100 )
		{
		  result = f_write( archiveFile, bytes, 100, &written) ;
			total += written ;
			i = 0 ;
		}
	}

	uint8_t fragment ;
	if ( size )
	{
		bytes[i++] = base64digits[data[0] >> 2] ;
		fragment = (data[0] << 4) & 0x30 ;
		if ( --size )
		{
 			fragment |= data[1] >> 4 ;
		}
		bytes[i++] = base64digits[fragment];
		bytes[i++] = ( size == 1 ) ? base64digits[(data[1] << 2) & 0x3c] : '=' ;
		bytes[i++] = '=' ;
	}
	if ( i )
	{
		result = f_write( archiveFile, bytes, i, &written) ;
		total += written ;
	}
  result = f_write( archiveFile, (BYTE *)"]]></Data>\n </MODEL_DATA>\n</ERSKY9X_EEPROM_FILE>\n", 49, &written) ;
	total += written ;
	*totalWritten = total ;
 return result ;
}

uint8_t xmlDecode( uint8_t value )
{
	if ( value >= 'A' )
	{
		if ( value <= 'Z' )
		{
			return value - 'A' ;
		}
		return value - ( 'a' - 26 ) ;		// 'a'-'z'		
	}
	else
	{
		if ( value >= '0' )
		{
			return value + 4 ;
		}
		if ( value == '+' )
		{
			return 62 ;
		}
		else if ( value == '/' )
		{
			return 63 ;
		}
		else
		{
			return 255 ;		// '='
		}
	}
}


int32_t readXMLfile( FIL *archiveFile, uint8_t *data, uint32_t size, UINT *totalRead )
{
  UINT nread ;
  UINT total = 0 ;
	uint32_t i ;
	uint32_t j ;
	FRESULT result ;
	uint8_t stream[4] ;
  uint8_t bytes[104] ;

	result = f_read( archiveFile,  bytes, 100, &nread ) ;
	total += nread ;
	if ( strncmp( (const char *)&bytes[10], "ERSKY9X_", 8 ) )
	{
		return -1 ;		// Invalid file
	}
	result = f_read( archiveFile,  bytes, 100, &nread ) ;
	total += nread ;
	// Search for "CDATA[" starting at offset 30
	for ( i = 30 ; i < 50 ; i += 1 )
	{
		if ( !strncmp( (const char *)&bytes[i], "CDATA[", 6 ) )
		{
			break ;
		}
	}
	if ( i >= 50 )
	{
		return -1 ;		// Invalid file
	}
	i += 6 ;	// Index of base64 data
	j = 0 ;
	while ( size )
	{
		if ( i >= nread )
		{
			result = f_read( archiveFile,  bytes, 100, &nread ) ;
			total += nread ;
			i = 0 ;
		}
		if ( i < nread )
		{
			uint8_t value = bytes[i] ;
			if ( value == ']' )		// End of input data
			{
				stream[j++] = 0 ;
			}
			else
			{
				i += 1 ;
				stream[j++] = xmlDecode( value ) ;
			}
		}
		else
		{
			break ;
		}
		if ( j >= 4 )
		{
		  *data++ = ( stream[0] << 2) | (stream[1] >> 4) ;
			size -= 1 ;
			if ( size == 0 )
			{
				break ;
			}
			if ( stream[2] != 255 )
			{
		  	*data++ = ((stream[1] << 4) & 0xf0) | (stream[2] >> 2);
				size -= 1 ;
				if ( size == 0 )
				{
					break ;
				}
			}
			if ( stream[3] != 255 )
			{
  			*data++ = ((stream[2] << 6) & 0xc0) | stream[3] ;
				size -= 1 ;
			}
			j = 0 ;
		}
	}
	while ( size )
	{
		*data++ = 0 ;
		size -= 1 ;
	}
	*totalRead = total ;
	return result ; 
}


const char *ee32BackupModel( uint8_t modelIndex )
{
//	uint8_t filename[50] ;
  uint16_t size ;
	FRESULT result ;
  DIR archiveFolder ;
  FIL archiveFile ;
  UINT written ;
	uint8_t filename[50] ;


	// Check for SD avaliable

#ifdef FS8K
  size = FileSizes[modelIndex-1] ;
#else
  size = File_system[modelIndex].size ;
#endif
	waitForEepromFinished() ;
//	while (ee32_check_finished() == 0)
//	{	// wait
//#ifndef SIMU
//		if ( General_timer )
//		{
//			General_timer = 1 ;		// Make these happen soon
//		}
//		if ( Model_timer )
//		{
//			Model_timer = 1 ;
//		}
//		CoTickDelay(1) ;					// 2mS for now
//#endif
//	}

	memset(( uint8_t *)&Eeprom_buffer.data.sky_model_data, 0, sizeof(g_model));
#ifdef FS8K
  read32_eeprom_data( (AllocTable[modelIndex] << 13) + sizeof( struct t_eeprom_header), ( uint8_t *)&Eeprom_buffer.data.sky_model_data, size, 0 ) ;
#else
  read32_eeprom_data( (File_system[modelIndex].block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&Eeprom_buffer.data.sky_model_data, size, 0 ) ;
#endif

	// Build filename
	setModelFilename( filename, modelIndex, FILE_TYPE_MODEL ) ;

  result = f_opendir(&archiveFolder, "/MODELS") ;
  if (result != FR_OK)
	{
    if (result == FR_NO_PATH)
		{
			WatchdogTimeout = 300 ;		// 3 seconds
      result = f_mkdir("/MODELS") ;
    	if (result != FR_OK)
			{
      	return "SDCARD ERROR" ;
			}
		}
  }
	
  result = f_open( &archiveFile, (TCHAR *)filename, FA_OPEN_ALWAYS | FA_CREATE_ALWAYS | FA_WRITE) ;
  if (result != FR_OK)
	{
   	return "CREATE ERROR" ;
  }

	result = writeXMLfile( &archiveFile, (uint8_t *)&Eeprom_buffer.data.sky_model_data, size, &written) ;
  
	f_close(&archiveFile) ;
  if (result != FR_OK ) //	|| written != size)
	{
    return "WRITE ERROR" ;
  }

  return "MODEL SAVED" ;
}

const char *ee32RestoreModel( uint8_t modelIndex, char *filename )
{
	uint8_t *bptr ;
  FIL archiveFile ;
  UINT nread ;
	FRESULT result ;
	int32_t answer ;
	uint8_t fname[50] ;
	
	bptr = cpystr( fname, (uint8_t *)"/MODELS/" ) ;
	cpystr( bptr, (uint8_t *)filename ) ;

	waitForEepromFinished() ;
//	while (ee32_check_finished() == 0)
//	{	// wait
//#ifndef SIMU
//		if ( General_timer )
//		{
//			General_timer = 1 ;		// Make these happen soon
//		}
//		if ( Model_timer )
//		{
//			Model_timer = 1 ;
//		}
//		CoTickDelay(1) ;					// 2mS for now
//#endif
//	}
  
	result = f_open( &archiveFile, (TCHAR *)filename, FA_READ) ;
  if (result != FR_OK)
	{
   	return "OPEN ERROR" ;
  }

	memset(( uint8_t *)&Eeprom_buffer.data.sky_model_data, 0, sizeof(g_model));
	
	answer = readXMLfile( &archiveFile,  ( uint8_t *)&Eeprom_buffer.data.sky_model_data, sizeof(Eeprom_buffer.data.sky_model_data), &nread ) ;
	
	if ( answer == -1 )
	{
		return "BAD FILE" ;
	}
	result = (FRESULT) answer ;
	if (result != FR_OK)
	{
		return "READ ERROR" ;
	}
	// Can we validate the data here?
  Eeprom32_source_address = (uint8_t *)&Eeprom_buffer.data.sky_model_data ;		// Get data from here
  Eeprom32_data_size = sizeof(g_model) ;																	// This much
  Eeprom32_file_index = modelIndex ;																							// This file system entry
#ifdef FS8K
	Eeprom32_state_after_erase = E32_BLANK2CHECK ;
#endif
  Eeprom32_process_state = E32_BLANKCHECK ;
  ee32WaitFinished() ;
	ee32_read_model_names() ;		// Update

  return "MODEL RESTORED" ;
}
#endif


#define EEPROM_PATH           "/EEPROM"   // no trailing slash = important

uint16_t AmountEeBackedUp ;
//FIL g_eebackupFile = {0};

const char *openRestoreEeprom( char *filename )
{
  FRESULT result;

	AmountEeBackedUp = 0 ;

#ifdef PCBSKY
  if ( SdMounted == 0 )
#endif
#if defined(PCBX9D) || defined(PCB9XT)
extern uint32_t sdMounted( void ) ;
  if ( sdMounted() == 0 )
#endif
    return "NO SD CARD" ;

	CoTickDelay(1) ;					// 2mS
  result = f_open(&SharedMemory.g_eebackupFile, filename, FA_OPEN_ALWAYS | FA_READ) ;
	CoTickDelay(1) ;					// 2mS
  if (result != FR_OK)
	{
    return "SD CARD ERROR" ; // SDCARD_ERROR(result) ;
  }
  return NULL ;
}

//char Xfilename[34]; // /EEPROM/eeprom-2013-01-01.bin

const char *openBackupEeprom()
{
  FRESULT result;
  DIR folder;
  char filename[34]; // /EEPROM/eeprom-2013-01-01.bin

	AmountEeBackedUp = 0 ;

#ifdef PCBSKY
  if ( SdMounted == 0 )
#endif
#if defined(PCBX9D) || defined(PCB9XT)
extern uint32_t sdMounted( void ) ;
  if ( sdMounted() == 0 )
#endif
    return "NO SD CARD" ;

  cpystr( (uint8_t *)filename, (uint8_t *)EEPROM_PATH ) ;

  result = f_opendir( &folder, filename) ;
  if (result != FR_OK)
	{
    if (result == FR_NO_PATH)
      result = f_mkdir(filename) ;
    if (result != FR_OK)
      return "SD CARD ERROR" ; // SDCARD_ERROR(result) ;
  }

  cpystr( (uint8_t *)&filename[7], (uint8_t *)"/eeprom" ) ;
	setFilenameDateTime( &filename[14], 0 ) ;
  cpystr((uint8_t *)&filename[14+11], (uint8_t *)".bin" ) ;

//  cpystr( (uint8_t *)Xfilename, (uint8_t *)filename ) ;

	CoTickDelay(1) ;					// 2mS
  result = f_open(&SharedMemory.g_eebackupFile, filename, FA_OPEN_ALWAYS | FA_WRITE) ;
	CoTickDelay(1) ;					// 2mS
  if (result != FR_OK)
	{
    return "SD CARD ERROR" ; // SDCARD_ERROR(result) ;
  }
  return NULL ;
}

const char *processBackupEeprom( uint16_t blockNo )
{
  FRESULT result ;
	UINT written ;

	if ( blockNo != AmountEeBackedUp )
	{
		return "Sync Error" ;
	}
  read32_eeprom_data( (blockNo << 11), ( uint8_t *)&Eeprom_buffer.data.buffer2K, 2048, 0 ) ;
	result = f_write( &SharedMemory.g_eebackupFile, ( BYTE *)&Eeprom_buffer.data.buffer2K, 2048, &written ) ;
	wdt_reset() ;
	if ( result != FR_OK )
	{
		return "Write Error" ;
	}
	AmountEeBackedUp += 1 ;
	return NULL ;
}

const char *processRestoreEeprom( uint16_t blockNo )
{
  FRESULT result ;
	UINT nread ;

	if ( blockNo != AmountEeBackedUp )
	{
		return "Sync Error" ;
	}
	result = f_read( &SharedMemory.g_eebackupFile, ( BYTE *)&Eeprom_buffer.data.buffer2K, 2048, &nread ) ;
	CoTickDelay(1) ;					// 2mS
// Write eeprom here
	write32_eeprom_2K( (uint32_t)blockNo << 11, Eeprom_buffer.data.buffer2K ) ;
	
	wdt_reset() ;
	if ( result != FR_OK )
	{
		return "Write Error" ;
	}
	AmountEeBackedUp += 1 ;
	return NULL ;
}

void closeBackupEeprom()
{
	f_close( &SharedMemory.g_eebackupFile ) ;
}

#if defined(IMAGE_128)
uint8_t ModelImage[64*32/8] ;
uint8_t ModelImageValid = 0 ;
TCHAR ImageFilename[60] ;
uint8_t LoadImageResult = 0xFF ;
uint8_t TNAME[VOICE_NAME_SIZE+4] ;
uint32_t loadModelImage()
{
  FIL imageFile ;
	FRESULT result ;
  UINT nread ;
	TCHAR *ptr ;
	uint8_t name[VOICE_NAME_SIZE+4] ;
//  uint8_t palette[2];
  uint8_t temp[64];
	ModelImageValid = 0 ;
  memmove( name, g_model.modelImageName, VOICE_NAME_SIZE+2 ) ;
	for ( uint8_t i=VOICE_NAME_SIZE+1 ; i ; i -= 1)
	{
		if ( name[i] == ' ' )
		{
			name[i] = '\0' ;
		}
		else
		{
			break ;
		}
	}
  memmove( TNAME, name, VOICE_NAME_SIZE+2 ) ;
  name[VOICE_NAME_SIZE+2] = '\0' ;
  TNAME[VOICE_NAME_SIZE+2] = '\0' ;
	ptr = (TCHAR *)cpystr( ( uint8_t*)ImageFilename, ( uint8_t*)"\\IMAGES\\" ) ;
	ptr = (TCHAR *)cpystr( (uint8_t *)ptr, name ) ;
	cpystr( ( uint8_t*)ptr, ( uint8_t*)".bmp" ) ;
	
	result = f_open( &imageFile, ImageFilename, FA_READ) ;
  if (result != FR_OK)
	{
		LoadImageResult = 1 ;
   	return 1 ;	// Error
  }
	result = f_read(&imageFile, ModelImage, 62, &nread) ;

//	for (uint8_t i=0; i<16; i += 1 )
//	{
//		palette[i] = ModelImage[54+4*i] >> 4 ;
//	}
	
	result = f_read(&imageFile, ModelImage, 64*32/8, &nread) ;
	f_close(&imageFile) ;
	if ( nread != 64*32/8 )
	{
		LoadImageResult = 2 ;
   	return 1 ;	// Error
	}
	uint32_t i ;
	uint32_t j ;
	uint32_t k ;
	uint8_t *p ;
	
	for ( j = 0 ; j < 4 ; j += 1 )
	{
		for ( i = 0 ; i < 64 ; )
		{
			temp[i++] = 0 ;
		}
		for ( i = 0 ; i < 64 ; i += 1 )
		{
			uint8_t t = 0 ;
			uint8_t u = 1 << ((63-i) & 7 ) ;
			p = &ModelImage[j*64 + (i / 8)] ;
			for ( k = 0 ; k < 8 ; k += 1 )
			{
				if ( ( p[k*8] & u ) == 0 )
				{
					t |= 1 << ( 7 - k ) ;
				}
			}
			temp[i] = t ;
		}
		p = &ModelImage[j*64] ;
		for ( i = 0 ; i < 64 ; i += 1 )
		{
			*p++ = temp[i] ;
		}
	}
	for ( i = 0 ; i < 64 ; i += 1 )
	{
		temp[0] = ModelImage[i] ;
		ModelImage[i] = ModelImage[i+192] ;
		ModelImage[i+192] = temp[0] ;
		temp[0] = ModelImage[i+64] ;
		ModelImage[i+64] = ModelImage[i+128] ;
		ModelImage[i+128] = temp[0] ;
	}

	ModelImageValid = 1 ;
	LoadImageResult = 0 ;
	return 0 ;
}
#endif

#ifdef FS8K

// New, 8K file system
//EE32 logic (4K):
//IDLE
// BLANKCHECK -> send erase command
//  ERASESENDING check command sent (then WRITESTART)
//   ERASEWAITING check finished
//    WRITESTART
//     WRITESENDING check command sent
//      WRITEWAITING when done, repeat send if more data to write
//       to IDLE

//For 8K blocks
//Top 2 4K blocks (alternating):
//0x55AA3CC3, 60 empty bytes
//64 bytes allocation table
//First byte 254 = General as 2 4K blocks (as now)
//60 bytes 8K block index 1-62
//2 bytes spare blocks
//253

//Control:
//60 bytes, model indices
//2 bytes spare blocks
//1 uint16, offset into top 2 4K blocks
//1 byte 8K block index being written

//locate spare block
//erase first 4k
//erase 2nd 4k
//write data
//update model indices and spare blocks
//write 64 bytes to top block:
// if (Offset & 0x0FFF == 0) // At end of block
//  erase other block
//  write the 64 bytes, 64 bytes into block
//  write first 4 bytes with 0x55AA3CC3
//  Update Offset to block start + 128
//  write first 4 bytes of other block to 0x00000000
// else
//  write the 64 bytes at Offset, add 64 to Offset
//free the data block that was in use

//Startup:
//Offset = 0
//read 4k block at block #126
//if first 4 bytes are 0x55AA3CC3
// Offset = 64
//else
// read 4k block at block #127
// if first 4 bytes are 0x55AA3CC3
//  Offset = 4096+64
//if Offset == 0
// // 8K file system not intiialised
// erase block #126
// write 64 bytes at 64 into block (254, 1-62, 253)
// write first 4 bytes with 0x55AA3CC3
// erase block #127
// Offset = 64
//scan through to find last 64 byte set
//store 60 bytes of model indices
//set Offset past 64 byte set
//determine spare blocks

//void init8K()
//{
//	uint32_t i ;

//	if ( ControlBlock )
//	{
//		i = 64 ;
//		ControlOffset = 0 ;
//		while ( i < 4096 )
//		{
//			read32_eeprom_data( (ControlBlock << 12) + i, Key.key, 4, EE_WAIT ) ;
//			if ( Key.key[0] != 0xFF )
//			{
//				ControlOffset = i ;
//				i += 64 ;
//			}
//			else
//			{
//				break ;
//			}
//		}
//		if ( ControlOffset )
//		{
//			read32_eeprom_data( (ControlBlock << 12) + 64 , AllocTable, 64, EE_WAIT ) ;
////			AllocTable[0] = 254 ;
////			AllocTable[63] = 253 ;
//			return ;
//		}
//	}
//	// Need to create 8K control
//	eraseBlock( 126 << 12 ) ;
//	Key.keyLong = 0x55AA3CC3 ;
//	write32_eeprom_block( 126 << 12, Key.key, 4, EE_WAIT ) ;
//	AllocTable[0] = 254 ;
//	for ( i = 1 ; i <= 62 ; i += 1 )
//	{
//		AllocTable[i] = i ;
//	}
//	AllocTable[63] = 253 ;
//	write32_eeprom_block( (126 << 12) + 64, AllocTable, 64, EE_WAIT ) ;
//	ControlOffset = 128 ;
//	eraseBlock( 127 << 12 ) ;
//}


uint32_t convertTo8K()
{
	uint32_t i ;
	uint32_t j ;
	uint8_t *buffer ;
	t_file_entry fs ;
	if ( ConversionProgress < 60 )
	{
		i = 60 - ConversionProgress ;	// Model to move
		i *= 2 ;	// Convert to 8K index
		j = get_current_block_number( i, &fs ) ;
		i += 4 ;	// Move up 16K
		i <<= 12 ;
		eraseBlock( i ) ;
		wdt_reset() ;
		eraseBlock( i+4096 ) ;
		wdt_reset() ;
		if ( fs.size )
		{
		  read32_eeprom_data( (j<<12) + sizeof( struct t_eeprom_header), ( uint8_t *)&Eeprom_buffer.data.sky_model_data, fs.size, 0 ) ;
			Eeprom_buffer.header.sequence_no = 1 ;
			Eeprom_buffer.header.flags = BLOCK_8K ;
			Eeprom_buffer.header.data_size = fs.size ;
			Eeprom_buffer.header.hcsum = byte_checksum( (uint8_t *)&Eeprom_buffer, 7 ) ;
			j = fs.size + sizeof( struct t_eeprom_header ) ;
			buffer = (uint8_t *) &Eeprom_buffer ;
			while ( j )
			{
				if ( j >= 256 )
				{
					write32_eeprom_block( i, buffer, 256, EE_WAIT ) ;
					waitEepromDone() ;
					i += 256 ;
					buffer += 256 ;
					j -= 256 ;
				}
				else
				{
					write32_eeprom_block( i, buffer, j, EE_WAIT ) ;
					waitEepromDone() ;
					j = 0 ;
				}
				wdt_reset() ;
			}
		}
		ConversionProgress += 1 ;
	}
	else
	{
		// sort Alloc table
		eraseBlock( 126 << 12 ) ;
		Key.keyLong = 0x55AA3CC3 ;
		write32_eeprom_block( 126 << 12, Key.key, 4, EE_WAIT ) ;
		AllocTable[0] = 0 ;
		for ( i = 1 ; i <= 60 ; i += 1 )
		{
			AllocTable[i] = i+2 ;
		}
		AllocTable[61] = 1 ;
		AllocTable[62] = 2 ;
		AllocTable[63] = 253 ;
		waitEepromDone() ;
		write32_eeprom_block( (126 << 12) + 64, AllocTable, 64, EE_WAIT ) ;
		waitEepromDone() ;
		ControlOffset = 128 ;
		ControlBlock = 126 ;
		eraseBlock( 127 << 12 ) ;
		ConversionProgress = 64 ;
		// Temporary to test roll over
//		while ( ControlOffset < 4096-256 )
//		{
//			wdt_reset() ;
//			write32_eeprom_block( (126 << 12) + ControlOffset, AllocTable, 64, EE_WAIT ) ;
//			waitEepromDone() ;
//			ControlOffset += 64 ;
//		}
	}
	return ConversionProgress ;
}

#else
 #ifdef FS8K
uint32_t convertTo8K()
{
	return 64 ;
}
 #endif
#endif

#ifdef FS8K
void swDelay10mS()
{
	uint32_t j ;
	for ( j = 0 ; j < 2000000 ; j += 1 )
	{
		asm("nop") ;
	}
}
#endif


#ifdef FS8K
uint32_t check8K()
{
#ifdef FS8K
	ControlBlock = 0 ; 
	fill_file_index() ;
	read32_eeprom_data( 126 << 12, Key.key, 4, EE_WAIT ) ;
	if ( Key.keyLong == 0x55AA3CC3 )
	{
		read32_eeprom_data( (126 << 12) + 64, AllocTable, 64, EE_WAIT ) ;
		if ( AllocTable[0] != 0xFF )
		{		
			ControlBlock = 126 ;
		}
	}
	if ( ControlBlock == 0 )
//	{
//		read32_eeprom_data( 127 << 12, Key.key, 4, EE_WAIT ) ;
//		if ( Key.keyLong == 0x55AA3CC3 )
//		{
//			read32_eeprom_data( (127 << 12) + 64, AllocTable, 64, EE_WAIT ) ;
//			if ( AllocTable[0] != 0xFF )
//			{		
//				ControlBlock = 127 ;
//			}
//		}
//	}
	
//	if ( Key.keyLong == 0x55AA3CC3 )
//	{
//		ControlBlock = 126 ;
//	}
//	else
	{
		read32_eeprom_data( 127 << 12, Key.key, 4, EE_WAIT ) ;
		if ( Key.keyLong == 0x55AA3CC3 )
		{
			ControlBlock = 127 ;
		}
	}
	if ( ControlBlock )
	{
		ControlOffset = 64 ;
		// need to locate last 
		while ( ControlOffset < 4096)
		{
			read32_eeprom_data( (ControlBlock << 12) + ControlOffset , AllocTable, 64, EE_WAIT ) ;
			if ( AllocTable[0] == 0xFF )
			{
				read32_eeprom_data( (ControlBlock << 12) + ControlOffset - 64 , AllocTable, 64, EE_WAIT ) ;
				break ;
			}
			wdt_reset() ;
			ControlOffset += 64 ;
		}
		if ( ControlOffset >= 4096 )
		{
			// Run off the end
			uint32_t otherBlock = (ControlBlock == 126) ? 127 : 126 ;
			eraseBlock( otherBlock << 12 ) ;
			Key.keyLong = 0x55AA3CC3 ;
			write32_eeprom_block( otherBlock << 12, Key.key, 4, EE_WAIT ) ;
			waitEepromDone() ;
			write32_eeprom_block( (otherBlock << 12) + 64, AllocTable, 64, EE_WAIT ) ;
			waitEepromDone() ;
			Key.keyLong = 0 ;
			write32_eeprom_block( ControlBlock << 12, Key.key, 4, EE_WAIT ) ;
			waitEepromDone() ;
			ControlBlock = otherBlock ;
			ControlOffset = 128 ;
		}
	}
	else
	{
		uint8_t *p = &g_eeGeneral.contrast ;
		uint32_t x = p - (uint8_t *)&g_eeGeneral ;
		x += sizeof( struct t_eeprom_header) ;
		read32_eeprom_data( x, ( uint8_t *)&g_eeGeneral.contrast, 1, 0 ) ;
		if ( ( g_eeGeneral.contrast > 10 ) && ( g_eeGeneral.contrast < 35 ) )
		{
	  	lcdSetRefVolt( g_eeGeneral.contrast ) ;
		}
		else
		{
#ifdef PCB9XT
	  	lcdSetRefVolt( 25 ) ;
#else
 #ifdef SMALL
  		lcdSetRefVolt( 24 ) ;
 #else
  		lcdSetRefVolt( 18 ) ;
 #endif
#endif
		}
	}	
	return ControlBlock > 0 ;
#else
	return 1 ;
#endif
}
#endif


