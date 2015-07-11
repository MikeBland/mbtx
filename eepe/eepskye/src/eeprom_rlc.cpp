/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
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
#include <string.h>
#include <stddef.h>
#include <QtGlobal>

#include "pers.h"
#include "eeprom_rlc.h"
#include "stdio.h"
#include <stdint.h>
//#include "string.h"
#include "myeeprom.h"
#include "file.h"
//#include "templates.h"
//#include "drivers.h"
//#include "Stringidx.h"
  
#define memclear(p, s) memset(p, 0, s)

//void ee32_read_model_namesRlc( void ) ;

//unsigned char ModelNames[MAX_MODELS+1][sizeof(g_model.name)+1] ;		// Allow for general

//extern void generalDefault() ;
//extern void modelDefault(uint8_t id)  ;

uint8_t *EepromImage ;

/**
  * @brief  Reads a block of data from the "EEPROM".
  * @param  pBuffer : pointer to the buffer that receives the data read from the EEPROM.
  * @param  ReadAddr : EEPROM's internal address to read from.
  * @param  NumByteToRead : number of bytes to read from the EEPROM.
  */
void eeprom_read_block( uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{
	while ( NumByteToRead )
	{
		*pBuffer++ = EepromImage[ReadAddr++] ;
		NumByteToRead -= 1 ;		
	}
}

void eeWriteBlockCmp( uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite )
{
  while ( NumByteToWrite )
	{
		EepromImage[WriteAddr++] = *pBuffer++  ;
		NumByteToWrite -= 1 ;		
	}
}

// EEPROM driver
//#if !defined(SIMU)
//#define eepromInit()          I2C_EE_Init()
//#define eeprom_read_block     I2C_EE_BufferRead
//#define eeWriteBlockCmp       I2C_EE_BufferWrite
//#else
//#define eepromInit()
//void eeWriteBlockCmp(const void *pointer_ram, uint16_t pointer_eeprom, size_t size);
//#endif

//uint8_t   s_eeDirtyMsk ;
uint8_t   s_write_err = 0;    // error reasons
RlcFile   theFile;  //used for any file operation
EeFs      eeFs;
//uint16_t s_eeDirtyTime10ms ;

blkidL_t   freeBlocks = 0 ;

uint8_t  s_sync_write = false;

//uint16_t evalChkSum()
//{
//  uint16_t sum=0;
//	uint16_t *p ;
//	p = ( uint16_t *)g_eeGeneral.calibMid ;
//  for (int i=0; i<12;i++)
//	{
//    sum += *p++ ;
//	}
//  return sum;
//}

//void generalDefault()
//{
//  memset(&g_eeGeneral,0,sizeof(g_eeGeneral));
//  g_eeGeneral.myVers   =  MDSKYVERS ;
//  g_eeGeneral.currModel=  0;
//  g_eeGeneral.contrast = 30;
//  g_eeGeneral.vBatWarn = 65;
//  g_eeGeneral.stickMode=  1;
//	g_eeGeneral.disablePotScroll=  1;
//	g_eeGeneral.bright = 50 ;
//	g_eeGeneral.volume = 2 ;
//  for (int i = 0; i < 7; ++i) {
//    g_eeGeneral.calibMid[i]     = 0x400;
//    g_eeGeneral.calibSpanNeg[i] = 0x300;
//    g_eeGeneral.calibSpanPos[i] = 0x300;
//  }
//  strncpy_P(g_eeGeneral.ownerName,PSTR(STR_ME), 10);
//  g_eeGeneral.chkSum = evalChkSum() ;
//	eeDirty(EE_GENERAL) ;
//}

//void modelDefault(uint8_t id)
//{
//  memset(&g_model, 0, sizeof(SKYModelData));
//  strncpy_P(g_model.name,PSTR(STR_MODEL), 10 );
//  g_model.name[5]='0'+(id+1)/10;
//  g_model.name[6]='0'+(id+1)%10;
//	g_model.modelVersion = MDSKYVERS;
//	g_model.trimInc = 1 ;

//  applyTemplate(0) ; //default 4 channel template
//  memcpy(ModelNames[id+1], g_model.name, sizeof(g_model.name));
//	eeDirty(EE_MODEL) ;
//}


static uint8_t EeFsRead(blkid_t blk, uint8_t ofs)
{
  uint8_t ret;
//#if defined(CPUARM)
  eeprom_read_block(&ret, (uint16_t)(blk*BS+ofs+BLOCKS_OFFSET), 1);
//#elif defined(SIMU)
//  eeprom_read_block(&ret, (const void*)(uint64_t)(blk*BS+ofs+BLOCKS_OFFSET), 1);
//#endif
  return ret;
}

static blkid_t EeFsGetLink(blkid_t blk)
{
  blkid_t ret;
  eeprom_read_block((uint8_t *)&ret, blk*BS+BLOCKS_OFFSET, sizeof(blkid_t));
  return ret;
}

static void EeFsSetLink(blkid_t blk, blkid_t val)
{
  static blkid_t s_link; // we write asynchronously, then nothing on the stack!
  s_link = val;
  eeWriteBlockCmp((uint8_t *)&s_link, (blk*BS)+BLOCKS_OFFSET, sizeof(blkid_t));
}

static uint8_t EeFsGetDat(blkid_t blk, uint8_t ofs)
{
  return EeFsRead(blk, ofs+sizeof(blkid_t));
}

static void EeFsSetDat(blkid_t blk, uint8_t ofs, uint8_t *buf, uint8_t len)
{
  eeWriteBlockCmp(buf, (blk*BS)+ofs+sizeof(blkid_t)+BLOCKS_OFFSET, len);
}

static void EeFsFlushFreelist()
{
  eeWriteBlockCmp((uint8_t *)&eeFs.freeList, offsetof(EeFs, freeList), sizeof(eeFs.freeList));
}

static void EeFsFlushDirEnt(uint8_t i_fileId)
{
  eeWriteBlockCmp((uint8_t *)&eeFs.files[i_fileId], offsetof(EeFs, files) + sizeof(DirEnt)*i_fileId, sizeof(DirEnt));
}

static void EeFsFlush()
{
  eeWriteBlockCmp((uint8_t *)&eeFs, 0, sizeof(eeFs));
}

//uint16_t EeFsGetFree()
//{
//  int32_t ret = freeBlocks * (BS-sizeof(blkid_t));
  
//	ret += eeFs.files[FILE_TMP].size;
//  ret -= eeFs.files[FILE_MODEL(g_eeGeneral.currModel)].size;
//  return (ret > 0 ? ret : 0);
//}

///// free one or more blocks
//static void EeFsFree(blkid_t blk)
//{
//  blkid_t i = blk;
//  blkid_t tmp;

//  freeBlocks++;

//  while ((tmp=EeFsGetLink(i))) {
//    i = tmp;
//    freeBlocks++;
//  }

//  EeFsSetLink(i, eeFs.freeList);
//  eeFs.freeList = blk; //chain in front
//  EeFsFlushFreelist();
//}

uint8_t tempBuffer[BLOCKS] ;

int8_t EeFsck()
{
  ENABLE_SYNC_WRITE(true);

//  uint8_t *bufp = (uint8_t *)&g_model;
  uint8_t *bufp = (uint8_t *)tempBuffer ;
  memclear(bufp, BLOCKS);
  blkidL_t blk ;

  blkid_t blocksCount;
  
	for (uint8_t i=0; i<=MAXRLCFILES; i++)
	{
    blocksCount = 0;
    blkid_t *startP = (i==MAXRLCFILES ? &eeFs.freeList : &eeFs.files[i].startBlk);
    blkid_t lastBlk = 0;
    blk = *startP;
    while (blk)
		{
      if (blk < FIRSTBLK || // bad blk index
          blk >= BLOCKS  || // bad blk indexchan
          bufp[blk])        // blk double usage
      {
        if (lastBlk) {
          EeFsSetLink(lastBlk, 0);
        }
        else {
          *startP = 0; // interrupt chain at startpos
          EeFsFlush();
        }
        blk = 0; // abort
      }
      else {
        blocksCount++;
        bufp[blk] = i+1;
        lastBlk   = blk;
        blk       = EeFsGetLink(blk);
      }
    }

  }

  freeBlocks = blocksCount;

  for (blk=FIRSTBLK; blk<BLOCKS; blk++)
	{
    if (!bufp[blk])
		{ // unused block
      freeBlocks++;
      EeFsSetLink(blk, eeFs.freeList);
      eeFs.freeList = blk; // chain in front
      EeFsFlushFreelist();
    }
  }

  ENABLE_SYNC_WRITE(false);

  return 0;
}

void EeFsFormat( uint8_t *eeprom )
{
	EepromImage = eeprom ;
  
	ENABLE_SYNC_WRITE(true);

  memclear(&eeFs, sizeof(eeFs));
  eeFs.version  = EEFS_VERS;
  eeFs.mySize   = sizeof(eeFs);
  eeFs.freeList = 0;
  eeFs.bs       = BS;
  for (blkidL_t i=FIRSTBLK; i<BLOCKS-1 ; i++)
	{
    EeFsSetLink(i, i+1) ;
  }
  EeFsSetLink(BLOCKS-1, 0);
  eeFs.freeList = FIRSTBLK;
  freeBlocks = BLOCKS ;
  EeFsFlush();

  ENABLE_SYNC_WRITE(false);
}

bool EeFsOpen()
{
  eeprom_read_block((uint8_t *)&eeFs, 0, sizeof(eeFs));

#ifdef SIMU
  if (eeFs.version != EEFS_VERS) {
    printf("bad eeFs.version (%d instead of %d)\n", eeFs.version, EEFS_VERS);
    fflush(stdout);
  }
  if (eeFs.mySize != sizeof(eeFs)) {
    printf("bad eeFs.mySize (%d instead of %d)\n", (int)eeFs.mySize, (int)sizeof(eeFs));
    fflush(stdout);
  }
#endif  

  return eeFs.version == EEFS_VERS && eeFs.mySize == sizeof(eeFs);
}

//bool EFile::exists(uint8_t i_fileId)
//{
//  return eeFs.files[i_fileId].startBlk;
//}

//void EFile::swap(uint8_t i_fileId1, uint8_t i_fileId2)
//{
//  DirEnt            tmp = eeFs.files[i_fileId1];
//  eeFs.files[i_fileId1] = eeFs.files[i_fileId2];
//  eeFs.files[i_fileId2] = tmp;

//  ENABLE_SYNC_WRITE(true);
//  EeFsFlushDirEnt(i_fileId1);
//  EeFsFlushDirEnt(i_fileId2);
//  ENABLE_SYNC_WRITE(false);
//}

//void EFile::rm(uint8_t i_fileId)
//{
//  blkid_t i = eeFs.files[i_fileId].startBlk;
//  memclear(&eeFs.files[i_fileId], sizeof(eeFs.files[i_fileId]));
//  ENABLE_SYNC_WRITE(true);
//  EeFsFlushDirEnt(i_fileId);
//  if (i) EeFsFree(i); //chain in
//  ENABLE_SYNC_WRITE(false);
//}

/*
 * Open file i_fileId for reading.
 * Return the file's type
 */
void EFile::openRd(uint8_t i_fileId)
{
  m_fileId = i_fileId;
  m_pos      = 0;
  m_currBlk  = eeFs.files[m_fileId].startBlk;
  m_ofs      = 0;
  s_write_err = ERR_NONE;       // error reasons */
}

void RlcFile::openRlc(uint8_t i_fileId)
{
  EFile::openRd(i_fileId);
  m_zeroes   = 0;
  m_bRlc     = 0;
}

uint8_t EFile::read(uint8_t *buf, uint8_t i_len)
{
  uint16_t len = eeFs.files[m_fileId].size - m_pos;
  if (i_len > len) i_len = len;

  uint8_t remaining = i_len;
  while (remaining) {
    if (!m_currBlk) break;
  
    *buf++ = EeFsGetDat(m_currBlk, m_ofs++);
    if (m_ofs >= BS-sizeof(blkid_t)) {
      m_ofs = 0;
      m_currBlk = EeFsGetLink(m_currBlk);
    }
    remaining--;
  }

  i_len -= remaining;
  m_pos += i_len;
  return i_len;
}

/*
 * Read runlength (RLE) compressed bytes into buf.
 */
uint16_t RlcFile::readRlc(uint8_t *buf, uint16_t i_len)
{
  uint16_t i = 0;
  for( ; 1; ) {
    uint8_t ln = qMin<uint16_t>(m_zeroes, i_len-i);
    memclear(&buf[i], ln);
    i        += ln;
    m_zeroes -= ln;
    if (m_zeroes) break;

    ln = qMin<uint16_t>(m_bRlc, i_len-i);
    uint8_t lr = read(&buf[i], ln);
    i        += lr ;
    m_bRlc   -= lr;
    if(m_bRlc) break;

    if (read(&m_bRlc, 1) !=1) break; // read how many bytes to read

    if (m_bRlc&0x80) { // if contains high byte
      m_zeroes  =(m_bRlc>>4) & 0x7;
      m_bRlc    = m_bRlc & 0x0f;
    }
    else if(m_bRlc&0x40) {
      m_zeroes  = m_bRlc & 0x3f;
      m_bRlc    = 0;
    }
  }
  return i;
}

void RlcFile::write1(uint8_t b)
{
  m_write1_byte = b;
  write(&m_write1_byte, 1);
}

void RlcFile::write(uint8_t *buf, uint8_t i_len)
{
  m_write_len = i_len;
  m_write_buf = buf;

  do {
    nextWriteStep();
  } while (IS_SYNC_WRITE_ENABLE() && m_write_len && !s_write_err);
}

void RlcFile::nextWriteStep()
{
  if (!m_currBlk && m_pos==0) {
    eeFs.files[FILE_TMP].startBlk = m_currBlk = eeFs.freeList;
    if (m_currBlk) {
      freeBlocks--;
      eeFs.freeList = EeFsGetLink(m_currBlk);
      m_write_step |= WRITE_FIRST_LINK;
      EeFsFlushFreelist();
      return;
    }
  }

  if ((m_write_step & 0x0f) == WRITE_FIRST_LINK) {
    m_write_step -= WRITE_FIRST_LINK;
    EeFsSetLink(m_currBlk, 0);
    return;
  }

  while (m_write_len) {
    if (!m_currBlk) {
      s_write_err = ERR_FULL;
      break;
    }
    if (m_ofs >= (BS-sizeof(blkid_t))) {
      m_ofs = 0;
      blkid_t nextBlk = EeFsGetLink(m_currBlk);
      if (!nextBlk) {
        if (!eeFs.freeList) {
          s_write_err = ERR_FULL;
          break;
        }
        m_write_step += WRITE_NEXT_LINK_1;
        EeFsSetLink(m_currBlk, eeFs.freeList);
        // TODO not good
        return;
      }
      m_currBlk = nextBlk;
    }
    switch (m_write_step & 0x0f) {
      case WRITE_NEXT_LINK_1:
        m_currBlk = eeFs.freeList;
        freeBlocks--;
        eeFs.freeList = EeFsGetLink(eeFs.freeList);
        m_write_step += 1;
        EeFsFlushFreelist();
        return;
      case WRITE_NEXT_LINK_2:
        m_write_step -= WRITE_NEXT_LINK_2;
        EeFsSetLink(m_currBlk, 0);
        return;
    }
    uint8_t tmp = BS-sizeof(blkid_t)-m_ofs; if(tmp>m_write_len) tmp = m_write_len;
    m_write_buf += tmp;
    m_write_len -= tmp;
    m_ofs += tmp;
    m_pos += tmp;
    EeFsSetDat(m_currBlk, m_ofs-tmp, m_write_buf-tmp, tmp);
    return;
  }

  if (s_write_err == ERR_FULL) {
//    s_global_warning = STR_EE_OFLOW ;
    m_write_step = 0;
    m_write_len = 0;
    m_cur_rlc_len = 0;
  }
  else if (!IS_SYNC_WRITE_ENABLE()) {
    nextRlcWriteStep();
  }
}

void RlcFile::create(uint8_t i_fileId, uint8_t typ, uint8_t sync_write)
{
  // all write operations will be executed on FILE_TMP
  openRlc(FILE_TMP); // internal use
  eeFs.files[FILE_TMP].typ      = typ;
  eeFs.files[FILE_TMP].size     = 0;
  m_fileId = i_fileId;
  ENABLE_SYNC_WRITE(sync_write);
}

/*
 * Copy file src to dst
 */
//bool RlcFile::copy(uint8_t i_fileDst, uint8_t i_fileSrc)
//{
//  EFile theFile2;
//  theFile2.openRd(i_fileSrc);

//  create(i_fileDst, FILE_TYP_MODEL/*optimization, only model files are copied. should be eeFs.files[i_fileSrc].typ*/, true);

//  uint8_t buf[BS-sizeof(blkid_t)];
//  uint8_t len;
//  while ((len=theFile2.read(buf, sizeof(buf))))
//  {
//    write(buf, len);
//    if (write_errno() != 0) {
//      ENABLE_SYNC_WRITE(false);
//      return false;
//    }
//  }

//  blkid_t fri=0;
//  if (m_currBlk && (fri=EeFsGetLink(m_currBlk)))
//    EeFsSetLink(m_currBlk, 0);

//  if (fri) EeFsFree(fri);  //chain in

//  eeFs.files[FILE_TMP].size = m_pos;
//  EFile::swap(m_fileId, FILE_TMP);

//  // s_sync_write is set to false in swap();
//  return true;
//}


void RlcFile::writeRlc(uint8_t i_fileId, uint8_t typ, uint8_t *buf, uint16_t i_len, uint8_t sync_write)
{
  create(i_fileId, typ, sync_write);

  m_write_step = WRITE_START_STEP;
  m_rlc_buf = buf;
  m_rlc_len = i_len;
  m_cur_rlc_len = 0;

  do {
    nextRlcWriteStep();
  } while ( m_write_step && !s_write_err) ;
}

void RlcFile::nextRlcWriteStep()
{
  uint8_t cnt    = 1;
  uint8_t cnt0   = 0;
  uint16_t i = 0;

  if (m_cur_rlc_len)
	{
    uint8_t tmp1 = m_cur_rlc_len;
    uint8_t *tmp2 = m_rlc_buf;
    m_rlc_buf += m_cur_rlc_len;
    m_cur_rlc_len = 0;
    write(tmp2, tmp1);
    return;
  }

  bool run0 = (m_rlc_buf[0] == 0);

  if (m_rlc_len==0) goto close;

  for (i=1; 1; i++) // !! laeuft ein byte zu weit !!
  {
    bool cur0 = m_rlc_buf[i] == 0;
    if (cur0 != run0 || cnt==0x3f || (cnt0 && cnt==0x0f) || i==m_rlc_len) {
      if (run0) {
        if (cnt<8 && i!=m_rlc_len)
          cnt0 = cnt; //aufbew fuer spaeter
        else {
          m_rlc_buf+=cnt;
          m_rlc_len-=cnt;
          write1(cnt|0x40);
          return;
        }
      }
      else{
        m_rlc_buf+=cnt0;
        m_rlc_len-=cnt0+cnt;
        m_cur_rlc_len=cnt;
        if(cnt0){
          write1(0x80 | (cnt0<<4) | cnt);
        }
        else{
          write1(cnt);
        }
        return;
      }
      cnt=0;
      if (i==m_rlc_len) break;
      run0 = cur0;
    }
    cnt++;
  }

  close:

   switch(m_write_step) {
     case WRITE_START_STEP:
     {
       blkid_t fri=0;

       if (m_currBlk && (fri=EeFsGetLink(m_currBlk))) {
         // TODO reuse EeFsFree!!!
         blkid_t prev_freeList = eeFs.freeList;
         eeFs.freeList = fri;
         freeBlocks++;
         while( EeFsGetLink(fri)) {
           fri = EeFsGetLink(fri);
           freeBlocks++;
         }
         m_write_step = WRITE_FREE_UNUSED_BLOCKS_STEP1;
         EeFsSetLink(fri, prev_freeList);
         return;
       }
     }

     case WRITE_FINAL_DIRENT_STEP:
     {
       m_currBlk = eeFs.files[FILE_TMP].startBlk;
       DirEnt & f = eeFs.files[m_fileId];
       eeFs.files[FILE_TMP].startBlk = f.startBlk;
       eeFs.files[FILE_TMP].size = f.size;
       f.startBlk = m_currBlk;
       f.size = m_pos;
       f.typ = eeFs.files[FILE_TMP].typ;
       m_write_step = WRITE_TMP_DIRENT_STEP;
       EeFsFlushDirEnt(m_fileId);
       return;
     }

     case WRITE_TMP_DIRENT_STEP:
       m_write_step = 0;
       EeFsFlushDirEnt(FILE_TMP);
       return;

     case WRITE_FREE_UNUSED_BLOCKS_STEP1:
       m_write_step = WRITE_FREE_UNUSED_BLOCKS_STEP2;
       EeFsSetLink(m_currBlk, 0);
       return;

     case WRITE_FREE_UNUSED_BLOCKS_STEP2:
       m_write_step = WRITE_FINAL_DIRENT_STEP;
       EeFsFlushFreelist();
       return;
   }
}

//void RlcFile::flush()
//{

//  ENABLE_SYNC_WRITE(true);

//  while (m_write_len && !s_write_err)
//    nextWriteStep();

//  while (isWriting() && !s_write_err)
//    nextRlcWriteStep();

//  ENABLE_SYNC_WRITE(false);
//}

#define CHECK_EEPROM_VARIANT() (1)

bool eeLoadGeneralRlc( struct t_radioData *radioData )
{
//  theFile.openRlc(FILE_GENERAL);
//  if (theFile.readRlc((uint8_t*)&g_eeGeneral, 1) == 1 && g_eeGeneral.myVers == MDSKYVERS)
//	{
    theFile.openRlc(FILE_GENERAL);
    memclear( &radioData->generalSettings, sizeof(EEGeneral)) ;
    if (theFile.readRlc((uint8_t*)&radioData->generalSettings, sizeof(EEGeneral)) <= sizeof(EEGeneral) && CHECK_EEPROM_VARIANT())
		{
			radioData->File_system[0].size = sizeof(EEGeneral) ;
      return true;
    }
//  }

  return false;
}

void eeLoadModelNameRlc( struct t_radioData *radioData, uint8_t id, unsigned char*buf, uint8_t len )
//void eeLoadModelName(uint8_t id, unsigned char *name)
{
  memclear( buf, sizeof(radioData->models[0].name));
  if (id < MAX_MODELS)
	{
    theFile.openRlc(id);
    theFile.readRlc((uint8_t*)buf, sizeof(radioData->models[0].name));
    buf[sizeof(radioData->models[0].name)] = 0 ;
  }
}

//bool eeModelExists(uint8_t id)
//{
//    return EFile::exists(FILE_MODEL(id));
//}

// TODO Now the 2 functions in eeprom_rlc.cpp and eeprom_raw.cpp are really close, should be merged.
void eeLoadModelRlc(  struct t_radioData *radioData, uint8_t id)
{
  if (id<MAX_MODELS)
	{
 	  uint16_t sz ;
//		if ( eeModelExists(id) )
//		{
	    theFile.openRlc(FILE_MODEL(id));
      sz = theFile.readRlc((uint8_t*)&radioData->models[id], sizeof(SKYModelData));

#ifdef SIMU
  	  if (sz > 0 && sz != sizeof(g_model))
			{
    	  printf("Model data read=%d bytes vs %d bytes\n", sz, (int)sizeof(SKYModelData));
    	}
#endif
//		}
//		else
//		{
//			sz = 0 ;
//		}
	  if (sz < 256)
		{
//    	modelDefault(id);
//      eeCheck(true);
	  }
  }
}


void ee32_read_model_namesRlc(struct t_radioData *radioData)
{
	uint32_t i ;

	for ( i = 1 ; i <= MAX_MODELS ; i += 1 )
	{
    eeLoadModelNameRlc( radioData, i, radioData->ModelNames[i], sizeof( radioData->models[0].name) ) ;
	}
}

void fill_file_indexRlc( struct t_radioData *radioData )
{
	uint32_t i ;
	for ( i = 0 ; i < MAX_MODELS + 1 ; i += 1 )
	{
    radioData->File_system[i].block_no = i * 2 ;
		radioData->File_system[i].size = eeFs.files[i].size ;
		radioData->File_system[i].sequence_no = 1 ;
	}
}

uint32_t rawloadFileRlc( struct t_radioData *radioData, uint8_t *eeprom )
{
	uint32_t i ;
	EepromImage = eeprom ;
	EeFsOpen() ;
	fill_file_indexRlc(radioData) ;
	ee32_read_model_namesRlc(radioData) ;
	DefaultModelType = 1 ;
	eeLoadGeneralRlc(radioData) ;
	for ( i = 0 ; i < MAX_MODELS ; i += 1 )
	{
    eeLoadModelRlc( radioData, i ) ;
	}
	radioData->valid = 1 ;

	return 1 ;
	
}







