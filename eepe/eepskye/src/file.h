/*
 * Author - Erez Raviv <erezraviv@gmail.com>
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
#ifndef file_h
#define file_h

#include <QString.h>

extern uint32_t DefaultModelType ;


/// fileId of general file
#define FILE_GENERAL   0
/// convert model number 0..MAX_MODELS-1  int fileId
//#define FILE_MODEL(n) (1+n)
//#define FILE_TMP      (1+16)


#define ERR_NONE 0
#define ERR_FULL 1
#define ERR_TMO  2

//#define eeprom_write_block eeWriteBlockCmp
// bs=16  128 blocks    verlust link:128  16files:16*8  128     sum 256
// bs=32   64 blocks    verlust link: 64  16files:16*16 256     sum 320
//

#define MAX_MODELS 32
#define MAX_IMODELS 60

#define EETARANISSIZE	(32*1024)

#  define EE20MSIZE   (168L*1024L)

#  define EESIZE   ((MAX_MODELS+1)*8192L)
#  define EEFULLSIZE   (64*8192L)
//#  define BS       16
//#  define RESV     64  //reserv for eeprom header with directory (eeFs)
//#define FIRSTBLK (RESV/BS)
//#define BLOCKS   (2048/BS)
//#define EEFS_VERS 4

#define MAXFILES (1+MAX_MODELS+3)

//struct DirEnt{
//  uint8_t  startBlk;
//  uint16_t size:12;
//  uint16_t typ:4;
//}__attribute__((packed));

//struct EeFs{
//  uint8_t  version;
//  uint8_t  mySize;
//  uint8_t  freeList;
//  uint8_t  bs;
//  DirEnt   files[MAXFILES];
//}__attribute__((packed));


struct t_file_entry
{
	uint32_t block_no ;
	uint32_t sequence_no ;
	uint16_t size ;
	uint8_t flags ;
} ;

struct t_eeprom_header
{
	uint32_t sequence_no ;		// sequence # to decide which block is most recent
	uint16_t data_size ;			// # bytes in data area
	uint8_t flags ;
	uint8_t hcsum ;
} ;

struct t_eeprom_block
{
	struct t_eeprom_header header ;
	union
	{
		uint8_t bytes[4088] ;
		uint32_t words[1022] ;
	} data ;
} ;

struct t_radioHardware
{
	// Info about pots and switches
	uint8_t numberSwitches ;
	uint8_t numberPots ;
	uint8_t swWays[10] ;
	QString swNames[10] ;	
  uint8_t swIndices[10] ;
  uint8_t swRefs[10] ;
	QString potNames[10] ;	
  uint8_t potIndices[10] ;
} ;

struct t_radioData
{
		struct t_file_entry File_system[MAX_IMODELS+1] ;
    EEGeneral generalSettings ;
    SKYModelData models[MAX_IMODELS];
    unsigned char ModelNames[MAX_IMODELS+1][MODEL_NAME_LEN+2] ;		// Allow for general
		uint32_t valid ;
		uint32_t type ;
		uint32_t sub_type ;
		uint32_t bitType ;
		uint32_t options ;
		uint32_t T9xr_pro ;
		uint32_t extraPots ;
		uint32_t File8kBlocks ;
		struct t_radioHardware radioHardware ;
} ;

extern void initRadioHw( uint8_t type, struct t_radioData *rData ) ;

uint32_t rawloadFile( t_radioData *radioData, uint8_t *eeprom ) ;
uint32_t rawsaveFile( t_radioData *radioData, uint8_t *eeprom ) ;

//class EFile
//{
//  uint8_t  m_fileId;    //index of file in directory = filename
//  uint16_t m_pos;       //over all filepos
//  uint8_t  m_currBlk;   //current block.id
//  uint8_t  m_ofs;       //offset inside of the current block
//  uint8_t  m_bRlc;      //control byte for run length decoder
//  uint8_t  m_err;       //error reasons
//  //uint16_t m_stopTime10ms; //maximum point of time for writing


//  uint8_t eeprom[EESIZE];
//  EeFs    *eeFs;

//	struct t_file_entry File_system[MAX_MODELS+1] ;

//  //bool EeFsOpen();
//  //int8_t EeFsck();
//  //void EeFsFormat();
//  //uint16_t EeFsGetFree();

//  void eeprom_read_block (void *pointer_ram, uint16_t pointer_eeprom, size_t size);
//  void eeWriteBlockCmp(void *i_pointer_ram, uint16_t i_pointer_eeprom, size_t size);
//  uint8_t EeFsRead(uint8_t blk,uint8_t ofs);
//  void EeFsWrite(uint8_t blk,uint8_t ofs,uint8_t val);
//  uint8_t EeFsGetLink(uint8_t blk);
//  void EeFsSetLink(uint8_t blk,uint8_t val);
//  uint8_t EeFsGetDat(uint8_t blk,uint8_t ofs);
//  void EeFsSetDat(uint8_t blk,uint8_t ofs,uint8_t*buf,uint8_t len);
//  void EeFsFlushFreelist();
//  void EeFsFlush();
//  uint16_t EeFsGetFree();
//  void EeFsFree(uint8_t blk);///free one or more blocks
//  uint8_t EeFsAlloc(); ///alloc one block from freelist
//  int8_t EeFsck();
//  void EeFsFormat();
//  bool EeFsOpen();


//	uint8_t byte_checksum( uint8_t *p, uint32_t size ) ;
//	uint32_t ee32_check_header( struct t_eeprom_header *hptr ) ;
//	uint32_t get_current_block_number( uint32_t block_no, uint16_t *p_size, uint32_t *p_seq )
//	void fill_file_index() ;


//public:

//  EFile();

//  void load(void* buf);
//  void save(void* buf);
//  ///remove contents of given file
//  void rm(uint8_t i_fileId);

//  void format();

//  ///swap contents of file1 with them of file2
//  void swap(uint8_t i_fileId1,uint8_t i_fileId2);

//  ///return true if the file with given fileid exists
//  bool exists(uint8_t i_fileId);

//  ///open file for reading, no close necessary
//  ///for writing use writeRlc() or create()
//  uint8_t openRd(uint8_t i_fileId); 
//  /// create a new file with given fileId, 
//  /// !!! if this file already exists, then all blocks are reused
//  /// and all contents will be overwritten.
//  /// after writing closeTrunc has to be called
//  void    create(uint8_t i_fileId, uint8_t typ);
//  /// close file and truncate the blockchain if to long.
//  void    closeTrunc();

//  ///open file, write to file and close it. 
//  ///If file existed before, then contents is overwritten. 
//  ///If file was larger before, then unused blocks are freed
//  uint16_t writeRlc(uint8_t i_fileId, uint8_t typ,uint8_t*buf,uint16_t i_len);

//  uint8_t read(uint8_t*buf,uint16_t i_len);
//  uint8_t write(uint8_t*buf,uint8_t i_len);

//  ///return size of compressed file without block overhead
//  uint16_t size(uint8_t id);
//  ///read from opened file and decode rlc-coded data
//  uint16_t readRlc(uint8_t*buf,uint16_t i_len);
//	uint8_t byte_checksum( uint8_t *p, unsigned int size ) ;
//	uint32_t ee32_check_header( struct t_eeprom_header *hptr ) ;
//	uint32_t get_current_block_number( uint32_t block_no, uint16_t *p_size) ;





//};

#endif
/*eof*/
