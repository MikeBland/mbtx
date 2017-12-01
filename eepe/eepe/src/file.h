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


/// fileId of general file
#define FILE_GENERAL   0
/// convert model number 0..MAX_MODELS-1  int fileId
#define FILE_MODEL(n) (1+n)
#define FILE_TMP      (1+16)


#define ERR_NONE 0
#define ERR_FULL 1
#define ERR_TMO  2

//#define eeprom_write_block eeWriteBlockCmp
// bs=16  128 blocks    verlust link:128  16files:16*8  128     sum 256
// bs=32   64 blocks    verlust link: 64  16files:16*16 256     sum 320
//
#  define EESIZE64		2048
#  define EESIZE128   4096
#  define BS       16
#  define RESV     64  //reserv for eeprom header with directory (eeFs)
#define FIRSTBLK (RESV/BS)
#define BLOCKS64   (EESIZE64/BS)
#define BLOCKS128  (EESIZE128/BS)
#define EEFS_VERS 		4
#ifdef V2
#define EEFS_VERS128	7
#else
#define EEFS_VERS128	6
#endif

#define MAX_IMODELS 60

//#define EESIZE	EESIZE64
//#define BLOCKS	BLOCKS64

#define MAXFILES (1+MAX_MODELS+3)

struct DirEnt{
  uint8_t  startBlk;
  uint16_t size:12;
  uint16_t typ:4;
}__attribute__((packed));

struct EeFs{
  uint8_t  version;
  uint8_t  mySize;
  uint8_t  freeList;
  uint8_t  bs;
  DirEnt   files[MAXFILES];
}__attribute__((packed));

struct t_file_entry
{
	uint32_t block_no ;
	uint32_t sequence_no ;
	uint16_t size ;
	uint8_t flags ;
} ;


struct t_radioData
{
		struct t_file_entry File_system[MAX_MODELS+1] ;
    EEGeneral generalSettings ;
    V2EEGeneral v2generalSettings ;
    ModelData models[MAX_MODELS] ;
		V1ModelData v1models[MAX_MODELS] ;
    unsigned char ModelNames[MAX_MODELS+1][MODEL_NAME_LEN+2] ;		// Allow for general
		uint8_t valid ;
		uint8_t type ;
		uint8_t channels ;
		uint8_t number_voice_alarms ;
#ifdef V2
	int8_t  MaxSwitchIndex;     // max switch selection menu list index
	uint8_t MappedSwitchState;  // mapped switch state bit flag
	int8_t switchMapTable[MAX_PSW3POS*3+3];  // +3: "---","PB1","PB2"

  void mapSwitch(uint8_t swidx0) { MappedSwitchState |= (1 << swidx0) ; } ;
	void unmapSwitch(uint8_t swidx0) { MappedSwitchState &= ~(1 << swidx0) ; } ;
	bool qSwitchMapped(uint8_t swidx0) { return (MappedSwitchState & (1 << swidx0)) ; } ;
	bool is3PosSwitch( uint8_t dswitch ) { return (dswitch <= DSW_GEA && qSwitchMapped(dswitch - DSW_IDL)) ; } ;
	void initSwitchMapping() ;
	int8_t switchMap( int8_t mIndex ) ;
	int8_t switchUnMap( int8_t drswitch ) ;
	uint8_t getSwitchSource( uint8_t xsw ) ;
	void setMaxSwitchIndex() ;
	QString getMappedSWName(int val, int eepromType ) ;
	void populateSwitchCB(QComboBox *b, int value, int type, int noends ) ;
  int getSwitchCbValue( QComboBox *b, int eepromType, int noends ) ;
	void populateSourceCB(QComboBox *b, int stickMode, int telem, int value, int modelVersion) ;
	void populateTmrBSwitchCB(QComboBox *b, int value, int eepromType ) ;
	int getTimerSwitchCbValue( QComboBox *b, int eepromType ) ;
#endif
} ;

class EFile
{
  uint8_t  m_fileId;    //index of file in directory = filename
  uint16_t m_pos;       //over all filepos
  uint8_t  m_currBlk;   //current block.id
  uint8_t  m_ofs;       //offset inside of the current block
  uint8_t  m_bRlc;      //control byte for run length decoder
  uint8_t  m_err;       //error reasons
  //uint16_t m_stopTime10ms; //maximum point of time for writing

	uint8_t	 m_blocks ;		// BLOCKS64 or BLOCKS128

  uint8_t eeprom[EESIZE128];
  EeFs    *eeFs;

  //bool EeFsOpen();
  //int8_t EeFsck();
  //void EeFsFormat();
  //uint16_t EeFsGetFree();





  void eeprom_read_block (void *pointer_ram, uint16_t pointer_eeprom, size_t size);
  void eeWriteBlockCmp(void *i_pointer_ram, uint16_t i_pointer_eeprom, size_t size);
  uint8_t EeFsRead(uint8_t blk,uint8_t ofs);
  void EeFsWrite(uint8_t blk,uint8_t ofs,uint8_t val);
  uint8_t EeFsGetLink(uint8_t blk);
  void EeFsSetLink(uint8_t blk,uint8_t val);
  uint8_t EeFsGetDat(uint8_t blk,uint8_t ofs);
  void EeFsSetDat(uint8_t blk,uint8_t ofs,uint8_t*buf,uint8_t len);
  void EeFsFlushFreelist();
  void EeFsFlush();
  uint16_t EeFsGetFree();
  void EeFsFree(uint8_t blk);///free one or more blocks
  uint8_t EeFsAlloc(); ///alloc one block from freelist
  int8_t EeFsck();
  void EeFsFormat();
  bool EeFsOpen();




public:

  EFile();
  uint8_t  m_type ;     // 0 = M64, 1 = M128

  void load(void* buf);
  void save(void* buf);
  ///remove contents of given file
  void rm(uint8_t i_fileId);

  void format();

  ///swap contents of file1 with them of file2
  void swap(uint8_t i_fileId1,uint8_t i_fileId2);

  ///return true if the file with given fileid exists
  bool exists(uint8_t i_fileId);

  ///open file for reading, no close necessary
  ///for writing use writeRlc() or create()
  uint8_t openRd(uint8_t i_fileId); 
  /// create a new file with given fileId, 
  /// !!! if this file already exists, then all blocks are reused
  /// and all contents will be overwritten.
  /// after writing closeTrunc has to be called
  void    create(uint8_t i_fileId, uint8_t typ);
  /// close file and truncate the blockchain if to long.
  void    closeTrunc();

  ///open file, write to file and close it. 
  ///If file existed before, then contents is overwritten. 
  ///If file was larger before, then unused blocks are freed
  uint16_t writeRlc(uint8_t i_fileId, uint8_t typ,uint8_t*buf,uint16_t i_len);

  uint8_t read(uint8_t*buf,uint16_t i_len);
  uint8_t write(uint8_t*buf,uint8_t i_len);

  ///return size of compressed file without block overhead
  uint16_t size(uint8_t id);
  ///read from opened file and decode rlc-coded data
  uint16_t readRlc(uint8_t*buf,uint16_t i_len);
  int freespace() {return EeFsGetFree();}


};

#endif
/*eof*/
