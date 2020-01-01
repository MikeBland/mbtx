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

#include "ersky9x.h"
#include "eeprom_rlc.h"
#include "stdio.h"
#include "inttypes.h"
#include "string.h"
#include "i2c_ee.h"
#include "myeeprom.h"
//#include "language.h"
#include "templates.h"
#include "drivers.h"
#include "stringidx.h"
#include "ff.h"
#ifndef SIMU
#include "CoOS.h"
#endif
#include "lcd.h"
  
#define memclear(p, s) memset(p, 0, s)

#define pulsesStarted() true
#define pausePulses()
#define pauseMixerCalculations()
#define resumePulses()
#define resumeMixerCalculations()

void eeCheck(bool immediately) ;
void ee32_read_model_names( void ) ;

unsigned char ModelNames[MAX_MODELS+1][sizeof(g_model.name)] ;		// Allow for general

SKYModelData TempModelStore ;
extern union t_xmem Xmem ;
extern union t_sharedMemory SharedMemory ;

extern void generalDefault() ;
extern void modelDefault(uint8_t id)  ;
extern void setFilenameDateTime( char *filename, uint32_t includeTime ) ;

// EEPROM driver
#if !defined(SIMU)
#define eepromInit()          I2C_EE_Init()
#define eeprom_read_block     I2C_EE_BufferRead
#define eeWriteBlockCmp       I2C_EE_BufferWrite
#else
#define eepromInit()
void eeWriteBlockCmp(const void *pointer_ram, uint16_t pointer_eeprom, size_t size);
#endif

uint8_t   s_eeDirtyMsk ;
uint8_t   s_write_err = 0;    // error reasons
RlcFile   theFile;  //used for any file operation
EeFs      eeFs;
uint16_t s_eeDirtyTime10ms ;

blkidL_t   freeBlocks = 0 ;

uint8_t  s_sync_write = false;

int16_t *const CalibMid[] =
{ 
	&g_eeGeneral.calibMid[0],
	&g_eeGeneral.calibMid[1],
	&g_eeGeneral.calibMid[2],
	&g_eeGeneral.calibMid[3],
	&g_eeGeneral.calibMid[4],
	&g_eeGeneral.calibMid[5],
	&g_eeGeneral.calibMid[6],
#ifdef PCBX9D
	&g_eeGeneral.x9dcalibMid,
#if defined(REVPLUS) || defined(REV9E)
#ifdef REV9E
	&g_eeGeneral.xcalibMid[0],
	&g_eeGeneral.xcalibMid[1],
	&g_eeGeneral.xcalibMid[2],
#endif
	&g_eeGeneral.x9dPcalibMid
#endif
#ifdef PCBX7
	&g_eeGeneral.x9dPcalibMid
#endif
#endif
} ;

int16_t *const CalibSpanPos[] =
{ 
	&g_eeGeneral.calibSpanPos[0],
	&g_eeGeneral.calibSpanPos[1],
	&g_eeGeneral.calibSpanPos[2],
	&g_eeGeneral.calibSpanPos[3],
	&g_eeGeneral.calibSpanPos[4],
	&g_eeGeneral.calibSpanPos[5],
	&g_eeGeneral.calibSpanPos[6],
#ifdef PCBX9D
	&g_eeGeneral.x9dcalibSpanPos,
#if defined(REVPLUS) || defined(REV9E)
#ifdef REV9E
	&g_eeGeneral.xcalibSpanPos[0],
	&g_eeGeneral.xcalibSpanPos[1],
	&g_eeGeneral.xcalibSpanPos[2],
#endif
	&g_eeGeneral.x9dPcalibSpanPos
#endif
#ifdef PCBX7
	&g_eeGeneral.x9dPcalibSpanPos
#endif
#endif
} ;

int16_t *const CalibSpanNeg[] =
{
	&g_eeGeneral.calibSpanNeg[0],
	&g_eeGeneral.calibSpanNeg[1],
	&g_eeGeneral.calibSpanNeg[2],
	&g_eeGeneral.calibSpanNeg[3],
	&g_eeGeneral.calibSpanNeg[4],
	&g_eeGeneral.calibSpanNeg[5],
	&g_eeGeneral.calibSpanNeg[6],
#ifdef PCBX9D
	&g_eeGeneral.x9dcalibSpanNeg,
#if defined(REVPLUS) || defined(REV9E)
#ifdef REV9E
	&g_eeGeneral.xcalibSpanNeg[0],
	&g_eeGeneral.xcalibSpanNeg[1],
	&g_eeGeneral.xcalibSpanNeg[2],
#endif
	&g_eeGeneral.x9dPcalibSpanNeg
#endif
#ifdef PCBX7
	&g_eeGeneral.x9dPcalibSpanNeg
#endif
#endif
} ;



uint16_t evalChkSum()
{
  uint16_t sum=0;
	uint16_t *p ;
	p = ( uint16_t *)g_eeGeneral.calibMid ;
  for (int i=0; i<12;i++)
	{
    sum += *p++ ;
	}
  return sum;
}

void generalDefault()
{
  memset(&g_eeGeneral,0,sizeof(g_eeGeneral));
  g_eeGeneral.myVers   =  MDSKYVERS ;
  g_eeGeneral.currModel=  0;
  g_eeGeneral.contrast = 30;
  g_eeGeneral.vBatWarn = 65;
  g_eeGeneral.stickMode=  1;
	g_eeGeneral.disablePotScroll=  1;
#if defined(PCBX12D) || defined(PCBX10)
	g_eeGeneral.bright = 100 ;
#else
	g_eeGeneral.bright = 50 ;
#endif
	g_eeGeneral.volume = 2 ;
	g_eeGeneral.lightSw = MAX_SKYDRSWITCH ;	// ON
	g_eeGeneral.filterInput = 1 ;
	g_eeGeneral.gpsFormat = 1 ;

  for (int i = 0; i < NUM_ANALOG_CALS ; ++i )
	{
		*CalibMid[i] = 0x400 ;
		*CalibSpanPos[i] = 0x300 ;
		*CalibSpanNeg[i] = 0x300 ;
  }
  strncpy_P(g_eeGeneral.ownerName,PSTR(STR_ME), 10);
  g_eeGeneral.chkSum = evalChkSum() ;
	eeDirty(EE_GENERAL) ;
	sysFlags |= sysFLAG_FORMAT_EEPROM ;
}

void modelDefault(uint8_t id)
{
  memset(&g_model, 0, sizeof(SKYModelData));
  strncpy_P(g_model.name,PSTR(STR_MODEL), 10 );
  g_model.name[5]='0'+(id+1)/10;
  g_model.name[6]='0'+(id+1)%10;
	g_model.modelVersion = 4 ;
	g_model.trimInc = 1 ;

  applyTemplate(0) ; //default 4 channel template
  memcpy(ModelNames[id+1], g_model.name, sizeof(g_model.name));
	// Set all mode trims to be copies of FM0
	for ( uint32_t i = 0 ; i < MAX_MODES ; i += 1 )
	{
		g_model.phaseData[i].trim[0] = TRIM_EXTENDED_MAX + 1 ;
		g_model.phaseData[i].trim[1] = TRIM_EXTENDED_MAX + 1 ;
		g_model.phaseData[i].trim[2] = TRIM_EXTENDED_MAX + 1 ;
		g_model.phaseData[i].trim[3] = TRIM_EXTENDED_MAX + 1 ;
	}
//#ifdef PCB9XT
//	g_model.protocol = PROTO_OFF ;
//	g_model.xprotocol = PROTO_OFF ;
	g_model.Module[0].protocol = PROTO_OFF ;
	g_model.Module[1].protocol = PROTO_OFF ;
	eeDirty(EE_MODEL) ;
}


static uint8_t EeFsRead(blkid_t blk, uint8_t ofs)
{
  uint8_t ret;
#if defined(CPUARM)
  eeprom_read_block(&ret, (uint16_t)(blk*BS+ofs+BLOCKS_OFFSET), 1);
#elif defined(SIMU)
  eeprom_read_block(&ret, (const void*)(uint64_t)(blk*BS+ofs+BLOCKS_OFFSET), 1);
#endif
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

uint16_t EeFsGetFree()
{
  int32_t ret = freeBlocks * (BS-sizeof(blkid_t));
  
	ret += eeFs.files[FILE_TMP].size;
  ret -= eeFs.files[FILE_MODEL(g_eeGeneral.currModel)].size;
  return (ret > 0 ? ret : 0);
}

/// free one or more blocks
static void EeFsFree(blkid_t blk)
{
  blkid_t i = blk;
  blkid_t tmp;

  freeBlocks++;

  while ((tmp=EeFsGetLink(i))) {
    i = tmp;
    freeBlocks++;
  }

  EeFsSetLink(i, eeFs.freeList);
  eeFs.freeList = blk; //chain in front
  EeFsFlushFreelist();
}

uint8_t tempBuffer[BLOCKS] ;

int8_t EeFsck()
{
  ENABLE_SYNC_WRITE(true);

//  uint8_t *bufp = (uint8_t *)&g_model;
  uint8_t *bufp = (uint8_t *)tempBuffer ;
  memclear(bufp, BLOCKS);
  blkidL_t blk ;

  blkid_t blocksCount;
  
	for (uint8_t i=0; i<=MAXFILES; i++)
	{
    blocksCount = 0;
    blkid_t *startP = (i==MAXFILES ? &eeFs.freeList : &eeFs.files[i].startBlk);
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
    	wdt_reset();
    }
   	wdt_reset();

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
    wdt_reset();
  }

  ENABLE_SYNC_WRITE(false);

  return 0;
}

void EeFsFormat()
{
  ENABLE_SYNC_WRITE(true);

  memclear(&eeFs, sizeof(eeFs));
  eeFs.version  = EEFS_VERS;
  eeFs.mySize   = sizeof(eeFs);
  eeFs.freeList = 0;
  eeFs.bs       = BS;
  for (blkidL_t i=FIRSTBLK; i<BLOCKS-1 ; i++)
	{
    EeFsSetLink(i, i+1) ;
	  wdt_reset() ;
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

bool EFile::exists(uint8_t i_fileId)
{
  return eeFs.files[i_fileId].startBlk;
}

/*
 * Swap two files in eeprom
 */
void EFile::swap(uint8_t i_fileId1, uint8_t i_fileId2)
{
  DirEnt            tmp = eeFs.files[i_fileId1];
  eeFs.files[i_fileId1] = eeFs.files[i_fileId2];
  eeFs.files[i_fileId2] = tmp;

  ENABLE_SYNC_WRITE(true);
  EeFsFlushDirEnt(i_fileId1);
  EeFsFlushDirEnt(i_fileId2);
  ENABLE_SYNC_WRITE(false);
}

void EFile::rm(uint8_t i_fileId)
{
  blkid_t i = eeFs.files[i_fileId].startBlk;
  memclear(&eeFs.files[i_fileId], sizeof(eeFs.files[i_fileId]));
  ENABLE_SYNC_WRITE(true);
  EeFsFlushDirEnt(i_fileId);
  if (i) EeFsFree(i); //chain in
  ENABLE_SYNC_WRITE(false);
}

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
    uint8_t ln = min<uint16_t>(m_zeroes, i_len-i);
    memclear(&buf[i], ln);
    i        += ln;
    m_zeroes -= ln;
    if (m_zeroes) break;

    ln = min<uint16_t>(m_bRlc, i_len-i);
    uint8_t lr = read(&buf[i], ln);
    i        += lr ;
    m_bRlc   -= lr;
    if(m_bRlc) break;

    if (read(&m_bRlc, 1) !=1) break; // read how many bytes to read

    assert(m_bRlc & 0x7f);

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
bool RlcFile::copy(uint8_t i_fileDst, uint8_t i_fileSrc)
{
  EFile theFile2;
  theFile2.openRd(i_fileSrc);

  create(i_fileDst, FILE_TYP_MODEL/*optimization, only model files are copied. should be eeFs.files[i_fileSrc].typ*/, true);

  uint8_t buf[BS-sizeof(blkid_t)];
  uint8_t len;
  while ((len=theFile2.read(buf, sizeof(buf))))
  {
    write(buf, len);
    if (write_errno() != 0) {
      ENABLE_SYNC_WRITE(false);
      return false;
    }
  }

  blkid_t fri=0;
  if (m_currBlk && (fri=EeFsGetLink(m_currBlk)))
    EeFsSetLink(m_currBlk, 0);

  if (fri) EeFsFree(fri);  //chain in

  eeFs.files[FILE_TMP].size = m_pos;
  EFile::swap(m_fileId, FILE_TMP);

  assert(!m_write_step);

  // s_sync_write is set to false in swap();
  return true;
}

//#if defined(SDCARD)
//extern FIL g_oLogFile;
//const pm_char * eeBackupModel(uint8_t i_fileSrc)
//{
//  char *buf = reusableBuffer.modelsel.mainname;
//  DIR archiveFolder;
//  UINT written;

//  // we must close the logs as we reuse the same FIL structure
//  closeLogs();

//  // check and create folder here
//  strcpy_P(buf, STR_MODELS_PATH);
//  FRESULT result = f_opendir(&archiveFolder, buf);
//  if (result != FR_OK) {
//    if (result == FR_NO_PATH)
//      result = f_mkdir(buf);
//    if (result != FR_OK)
//      return SDCARD_ERROR(result);
//  }

//  buf[sizeof(MODELS_PATH)-1] = '/';
//  eeLoadModelName(i_fileSrc, &buf[sizeof(MODELS_PATH)]);
//  buf[sizeof(MODELS_PATH)+sizeof(g_model.name)] = '\0';

//  uint8_t i = sizeof(MODELS_PATH)+sizeof(g_model.name)-1;
//  uint8_t len = 0;
//  while (i>sizeof(MODELS_PATH)-1) {
//    if (!len && buf[i])
//      len = i+1;
//    if (len) {
//      if (buf[i])
//	buf[i] = idx2char(buf[i]);
//      else
//        buf[i] = '_'; 	
//    }
//    i--;
//  }

//  if (len == 0) {
//    uint8_t num = i_fileSrc + 1;
//    strcpy_P(&buf[sizeof(MODELS_PATH)], STR_MODEL);
//    buf[sizeof(MODELS_PATH) + PSIZE(TR_MODEL)] = (char)((num / 10) + '0');
//    buf[sizeof(MODELS_PATH) + PSIZE(TR_MODEL) + 1] = (char)((num % 10) + '0');
//    len = sizeof(MODELS_PATH) + PSIZE(TR_MODEL) + 2;
//  }

//  strcpy_P(&buf[len], STR_MODELS_EXT);

//#ifdef SIMU
//  printf("SD-card backup filename=%s\n", buf); fflush(stdout);
//#endif

//  result = f_open(&g_oLogFile, buf, FA_CREATE_ALWAYS | FA_WRITE);
//  if (result != FR_OK) {
//    return SDCARD_ERROR(result);
//  }

//  EFile theFile2;
//  theFile2.openRd(FILE_MODEL(i_fileSrc));

//  *(uint32_t*)&buf[0] = O9X_FOURCC;
//  buf[4] = g_eeGeneral.version;
//  buf[5] = 'M';
//  *(uint16_t*)&buf[6] = eeModelSize(i_fileSrc);

//  result = f_write(&g_oLogFile, buf, 8, &written);
//  if (result != FR_OK || written != 8) {
//    f_close(&g_oLogFile);
//    return SDCARD_ERROR(result);
//  }

//  while ((len=theFile2.read((uint8_t *)buf, 15))) {
//    result = f_write(&g_oLogFile, (uint8_t *)buf, len, &written);
//    if (result != FR_OK || written != len) {
//      f_close(&g_oLogFile);
//      return SDCARD_ERROR(result);
//    }
//  }

//  f_close(&g_oLogFile);
//  return NULL;
//}

//const pm_char * eeRestoreModel(uint8_t i_fileDst, char *model_name)
//{
//  char *buf = reusableBuffer.modelsel.mainname;
//  UINT read;

//  // we must close the logs as we reuse the same FIL structure
//  closeLogs();

//  strcpy_P(buf, STR_MODELS_PATH);
//  buf[sizeof(MODELS_PATH)-1] = '/';
//  strcpy(&buf[sizeof(MODELS_PATH)], model_name);
//  strcpy_P(&buf[strlen(buf)], STR_MODELS_EXT);

//  FRESULT result = f_open(&g_oLogFile, buf, FA_OPEN_EXISTING | FA_READ);
//  if (result != FR_OK) {
//    return SDCARD_ERROR(result);
//  }

//  if (f_size(&g_oLogFile) < 8) {
//    f_close(&g_oLogFile);
//    return STR_INCOMPATIBLE;
//  }

//  result = f_read(&g_oLogFile, (uint8_t *)buf, 8, &read);
//  if (result != FR_OK || read != 8) {
//    f_close(&g_oLogFile);
//    return SDCARD_ERROR(result);
//  }

////  if (*(uint32_t*)&buf[0] != O9X_FOURCC || (uint8_t)buf[4] != MDSKYVERS || buf[5] != 'M') {
////    f_close(&g_oLogFile);
////    return STR_INCOMPATIBLE;
////  }

//  if (eeModelExists(i_fileDst)) {
//    eeDeleteModel(i_fileDst);
//  }

//  theFile.create(FILE_MODEL(i_fileDst), FILE_TYP_MODEL, true);

//  do {
//    result = f_read(&g_oLogFile, (uint8_t *)buf, 15, &read);
//    if (result != FR_OK) {
//      ENABLE_SYNC_WRITE(false);
//      f_close(&g_oLogFile);
//      return SDCARD_ERROR(result);
//    }
//    if (read > 0) {
//      theFile.write((uint8_t *)buf, read);
//      if (write_errno() != 0) {
//        ENABLE_SYNC_WRITE(false);
//        f_close(&g_oLogFile);
//        return STR_EE_OFLOW;
//      }
//    }
//  } while (read == 15);

//  blkid_t fri=0;
//  if (theFile.m_currBlk && (fri=EeFsGetLink(theFile.m_currBlk)))
//    EeFsSetLink(theFile.m_currBlk, 0);

//  if (fri) EeFsFree(fri);  //chain in

//  eeFs.files[FILE_TMP].size = theFile.m_pos;
//  EFile::swap(theFile.m_fileId, FILE_TMP); // s_sync_write is set to false in swap();

//  f_close(&g_oLogFile);
//  return NULL;
//}
//#endif // SD CARD

void RlcFile::writeRlc(uint8_t i_fileId, uint8_t typ, uint8_t *buf, uint16_t i_len, uint8_t sync_write)
{
  create(i_fileId, typ, sync_write);

  m_write_step = WRITE_START_STEP;
  m_rlc_buf = buf;
  m_rlc_len = i_len;
  m_cur_rlc_len = 0;
#if defined (EEPROM_PROGRESS_BAR)
  m_ratio = (typ == FILE_TYP_MODEL ? 100 : 10);
#endif

  do {
    nextRlcWriteStep();
		if ( sync_write )
		{
			wdt_reset() ;
		}
  } while (IS_SYNC_WRITE_ENABLE() && m_write_step && !s_write_err);
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
        assert(cnt0==0);
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

void RlcFile::flush()
{

  ENABLE_SYNC_WRITE(true);

  while (m_write_len && !s_write_err)
	{
    nextWriteStep();
		wdt_reset() ;
	}

  while (isWriting() && !s_write_err)
	{
    nextRlcWriteStep();
		wdt_reset() ;
	}

  ENABLE_SYNC_WRITE(false);
}

#if defined (EEPROM_PROGRESS_BAR)
void RlcFile::DisplayProgressBar(uint8_t x)
{
  if (s_eeDirtyMsk || isWriting() || eeprom_buffer_size) {
    uint8_t len = s_eeDirtyMsk ? 1 : limit((uint8_t)1, (uint8_t)(7 - (m_rlc_len/m_ratio)), (uint8_t)7);
    lcd_filled_rect(x+1, 0, 5, FH, SOLID, ERASE);
    lcd_filled_rect(x+2, 7-len, 3, len);
  }
}
#endif

#define CHECK_EEPROM_VARIANT() (1)

bool eeLoadGeneral()
{
  theFile.openRlc(FILE_GENERAL);
  if (theFile.readRlc((uint8_t*)&g_eeGeneral, 1) == 1 && g_eeGeneral.myVers == MDSKYVERS)
	{
    theFile.openRlc(FILE_GENERAL);
    if (theFile.readRlc((uint8_t*)&g_eeGeneral, sizeof(g_eeGeneral)) <= sizeof(EEGeneral) && CHECK_EEPROM_VARIANT())
		{
      return true;
    }
  }

#ifdef SIMU
  printf("EEPROM version %d (%d) instead of %d (%d)\n", g_eeGeneral.myVers, g_eeGeneral.variant, EEPROM_VER, EEPROM_VARIANT);
  fflush(stdout);
#endif

  return false;
}

//#ifdef REVPLUS
uint8_t ModelImage[64*32*4/8] ;
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
  uint8_t palette[16];
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
	result = f_read(&imageFile, ModelImage, 118, &nread) ;

	for (uint8_t i=0; i<16; i += 1 )
	{
		palette[i] = ModelImage[54+4*i] >> 4 ;
	}
	
	result = f_read(&imageFile, ModelImage, 64*32*4/8, &nread) ;
	f_close(&imageFile) ;
	if ( nread != 64*32*4/8 )
	{
		LoadImageResult = 2 ;
   	return 1 ;	// Error
	}
	uint32_t i ;
	for ( i = 0 ; i < 64*32*4/8 ; i += 1 )
	{
		uint8_t y ;
		uint8_t z ;
		uint8_t x = ModelImage[i] ;
		z = palette[x>>4] ;
		y = palette[x&0x0F] | (z << 4 ) ;
		ModelImage[i] = y ^ 0xFF ;
	}
	ModelImageValid = 1 ;
	LoadImageResult = 0 ;
	return 0 ;
}
//#endif


void eeLoadModelName(uint8_t id, unsigned char *name)
{
  memclear(name, sizeof(g_model.name));
  if (id <= MAX_MODELS) {
    theFile.openRlc(id);
    theFile.readRlc((uint8_t*)name, sizeof(g_model.name));
  }
}

//void eeLoadModelHeader(uint8_t id, ModelHeader *header)
//{
//  memclear(header, sizeof(ModelHeader));
//  if (id < MAX_MODELS) {
//    theFile.openRlc(FILE_MODEL(id));
//    theFile.readRlc((uint8_t*)header, sizeof(ModelHeader));
//  }
//}

bool eeModelExists(uint8_t id)
{
    return EFile::exists(FILE_MODEL(id));
}

// TODO Now the 2 functions in eeprom_rlc.cpp and eeprom_raw.cpp are really close, should be merged.
void eeLoadModel(uint8_t id)
{
  if (id<MAX_MODELS)
	{
		WatchdogTimeout = 200 ;		// 2 seconds
//    watchdogSetTimeout(500/*5s*/);

#if defined(SDCARD)
    closeLogs();
#endif

    if (pulsesStarted()) {
      pausePulses();
    }

    pauseMixerCalculations();

 	  uint16_t sz ;
		if ( eeModelExists(id) )
		{
	    theFile.openRlc(FILE_MODEL(id));
  	  sz = theFile.readRlc((uint8_t*)&g_model, sizeof(g_model));

#ifdef SIMU
  	  if (sz > 0 && sz != sizeof(g_model))
			{
    	  printf("Model data read=%d bytes vs %d bytes\n", sz, (int)sizeof(SKYModelData));
    	}
#endif
		}
		else
		{
			sz = 0 ;
		}
	  if (sz < 256)
		{
    	modelDefault(id);
      eeCheck(true);
	  }
		
		validateName( (uint8_t *)g_model.name, sizeof(g_model.name) ) ;

//    resetAll();

//    if (pulsesStarted()) {
//      checkAll();
//      resumePulses();
//    }

//    activeSwitches = 0;
//    activeFnSwitches = 0;
//    activeFunctions = 0;
//    memclear(lastFunctionTime, sizeof(lastFunctionTime));

//    for (uint8_t i=0; i<MAX_TIMERS; i++) {
//      if (g_model.timers[i].persistent) {
//        timersStates[i].val = g_model.timers[i].value;
//      }
//    }

		checkXyCurve() ;
//#ifdef REVPLUS
		loadModelImage() ;
//#endif
    resumeMixerCalculations();
    // TODO pulses should be started after mixer calculations ...
		ppmInValid = 0 ;

#if defined(FRSKY)
//    frskySendAlarms();
#endif

#if defined(SDCARD)
//    refreshModelAudioFiles();
#endif

//    LOAD_MODEL_BITMAP();

//    SEND_FAILSAFE_1S();
  }
}


// TODO merge this code with eeprom_arm.cpp one
void eeReadAll()
{
	
  if (!EeFsOpen() ||
       EeFsck() < 0 ||
      !eeLoadGeneral())
  {
     generalDefault();

//    ALERT(STR_ALERT, STR_BAD_EEPROM, AU_BAD_EEPROM);
    alert((char const *)PSTR(STR_BAD_EEPROM), true);

//    if (pwrCheck() == e_power_off) {
      // the radio has been powered off during the ALERT
//      pwrOff();
//    }
    message(PSTR(STR_EE_FORMAT));
//    MESSAGE(STR_ALERT, STR_EE_FORMAT, NULL, AU_EEPROM_FORMATTING);

    EeFsFormat();

    theFile.writeRlc(FILE_GENERAL, FILE_TYP_GENERAL, (uint8_t*)&g_eeGeneral, sizeof(EEGeneral), true);

    modelDefault(0);

    theFile.writeRlc(FILE_MODEL(0), FILE_TYP_MODEL, (uint8_t*)&g_model, sizeof(g_model), true);
  }
  else
	{
  	eeLoadModel(g_eeGeneral.currModel) ;
  }
	ee32_read_model_names() ;

//  stickMode = g_eeGeneral.stickMode;

//  for (uint8_t i=0; languagePacks[i]!=NULL; i++) {
//    if (!strncmp(g_eeGeneral.ttsLanguage, languagePacks[i]->id, 2)) {
//      currentLanguagePackIdx = i;
//      currentLanguagePack = languagePacks[i];
//    }
//  }

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


void eeCheck(bool immediately)
{
  if (immediately)
	{
    eeFlush() ;
  }

  if (s_eeDirtyMsk & EE_GENERAL)
	{
    s_eeDirtyMsk -= EE_GENERAL;
    theFile.writeRlc(FILE_GENERAL, FILE_TYP_GENERAL, (uint8_t*)&g_eeGeneral, sizeof(EEGeneral), immediately);
    if (!immediately) return;
  }

  if (s_eeDirtyMsk & EE_MODEL)
	{
    s_eeDirtyMsk = 0;
		ee32_update_name( FILE_MODEL(g_eeGeneral.currModel), (uint8_t *)&g_model ) ;		// In case it's changed
    theFile.writeRlc(FILE_MODEL(g_eeGeneral.currModel), FILE_TYP_MODEL, (uint8_t*)&g_model, sizeof(g_model), immediately);
  }
}

bool eeDuplicateModel(uint8_t id)
{
  uint32_t i;
  for( i=id ; i<MAX_MODELS; i++)
  {
    if(! eeModelExists(i) ) break;
  }
  if(i>=MAX_MODELS)
	{
  	for( i=0 ; i<id ; i++)
		{
    	if(! eeModelExists(i) ) break;
		}
  	if(i>=id) return false; //no free space in directory left
	}

	eeCopyModel( i, id-1 ) ;

	return true ;
}

bool eeCopyModel(uint8_t dst, uint8_t src)
{
  if (theFile.copy(FILE_MODEL(dst), FILE_MODEL(src))) {
    memcpy(ModelNames[dst+1], ModelNames[src+1], sizeof(g_model.name));
		return true;
  }
  else {
    return false;
  }
}

void ee32SwapModels(uint8_t id1, uint8_t id2)
{
//  EFile::swap(FILE_MODEL(id1), FILE_MODEL(id2));
  EFile::swap(id1, id2) ; // Make compatible with SKY board

  unsigned char tmp[sizeof(g_model.name)];
  memcpy(tmp, ModelNames[id1], sizeof(g_model.name));
  memcpy(ModelNames[id1], ModelNames[id2], sizeof(g_model.name));
  memcpy(ModelNames[id2], tmp, sizeof(g_model.name));
}

void ee32_read_model_names()
{
	uint32_t i ;

	for ( i = 1 ; i <= MAX_MODELS ; i += 1 )
	{
		eeLoadModelName( i, ModelNames[i] ) ;
    wdt_reset();
	}
}

void ee32_delete_model( uint8_t x)
{
	EFile::rm(FILE_MODEL(x)) ;
  memset(ModelNames[x+1], 0, sizeof(g_model.name)) ;
}

void ee32WaitLoadModel(uint8_t x)
{
	ee32_check_finished() ;
	eeLoadModel( x ) ;
}

//void ee32StoreGeneral()
//{
	
//}

//void ee32StoreModel( uint8_t modelNumber, uint8_t trim )
//{
	
//}

void eeModelChanged()
{
	if ( ( s_eeDirtyMsk & EE_MODEL ) == 0 )
	{
	  s_eeDirtyMsk |= EE_MODEL ;
  	s_eeDirtyTime10ms  = get_tmr10ms() - 30000 ;
	}
}

void eeDirty(uint8_t msk)
{
  if(!msk) return;
  s_eeDirtyMsk      |= msk;
  s_eeDirtyTime10ms  = get_tmr10ms() ;
}

#define WRITE_DELAY_10MS 500
#define TIME_TO_WRITE() (s_eeDirtyMsk && (get_tmr10ms() - s_eeDirtyTime10ms) >= WRITE_DELAY_10MS )

void eePoll()
{
	if (theFile.isWriting())
		theFile.nextWriteStep() ;
	else if (TIME_TO_WRITE())
		eeCheck(false) ;
}

uint32_t ee32_check_finished()
{
	eeCheck(1) ;
	return 1 ;
}

void setModelFilename( uint8_t *filename, uint8_t modelIndex, uint32_t type )
{
	uint8_t *bptr ;
	uint32_t i ;
	
	bptr = cpystr( filename, type ? (uint8_t *)"/TEXT/" : (uint8_t *)"/MODELS/" ) ;
  memcpy( bptr, ModelNames[modelIndex], sizeof(g_model.name)) ;
//					TempModelStore.name, sizeof(g_model.name)) ;
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
// MDVERS (2)
  bytes[0] = MDSKYVERS + '0' ;
  bytes[1] = 0 ;
  result = f_write( archiveFile, bytes, 1, &written) ;
	total += written ;
  result = f_write( archiveFile, (BYTE *)"</Version>\n  <Name>", 19, &written) ;
	total += written ;
  result = f_write( archiveFile, (BYTE *)TempModelStore.name, sizeof(g_model.name), &written) ;
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
			bytes[i++] = base64digits[fragment];
		}
		bytes[i++] = ( size == 2 ) ? base64digits[(data[1] << 2) & 0x3c] : '=' ;
		bytes[i++] = '=' ;
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
  uint16_t size ;
	FRESULT result ;
  DIR archiveFolder ;
  FIL archiveFile ;
  UINT written ;
	uint8_t filename[50] ;

	ee32_check_finished() ;

	theFile.openRlc(modelIndex);
  size = theFile.readRlc((uint8_t*)&TempModelStore, sizeof(g_model));

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

//  result = f_write(&archiveFile, (uint8_t *)&Eeprom_buffer.data.sky_model_data, size, &written) ;
	result = writeXMLfile( &archiveFile, (uint8_t *)&TempModelStore, size, &written) ;
  
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

	ee32_check_finished() ;
  
	result = f_open( &archiveFile, (TCHAR *)filename, FA_READ) ;
  if (result != FR_OK)
	{
   	return "OPEN ERROR" ;
  }

	memset(( uint8_t *)&TempModelStore, 0, sizeof(g_model));
	
	answer = readXMLfile( &archiveFile,  ( uint8_t *)&TempModelStore, sizeof(g_model), &nread ) ;
	
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

	theFile.writeRlc(modelIndex, FILE_TYP_MODEL, (uint8_t *)&TempModelStore, sizeof(g_model), true ) ;

	ee32_read_model_names() ;		// Update

  return "MODEL RESTORED" ;

}

#define EEPROM_PATH           "/EEPROM"   // no trailing slash = important

uint16_t AmountEeBackedUp ;
//FIL g_eebackupFile = {0};

const char *openRestoreEeprom( char *filename )
{
  FRESULT result;

	AmountEeBackedUp = 0 ;

extern uint32_t sdMounted( void ) ;
  if ( sdMounted() == 0 )
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


const char *openBackupEeprom()
{
  FRESULT result;
  DIR folder;
  char filename[34]; // /EEPROM/eeprom-2013-01-01.bin

	AmountEeBackedUp = 0 ;

extern uint32_t sdMounted( void ) ;
  if ( sdMounted() == 0 )
    return "NO SD CARD" ;

  strcpy( filename, EEPROM_PATH ) ;

  result = f_opendir( &folder, filename) ;
  if (result != FR_OK)
	{
    if (result == FR_NO_PATH)
      result = f_mkdir(filename) ;
    if (result != FR_OK)
      return "SD CARD ERROR" ; // SDCARD_ERROR(result) ;
  }

  strcpy( &filename[7], "/eeprom" ) ;
	setFilenameDateTime( &filename[14], 0 ) ;
  strcpy_P(&filename[14+11], ".bin" ) ;

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
	I2C_EE_BufferRead( Xmem.file_buffer, (blockNo << 9), 512 ) ;
	result = f_write( &SharedMemory.g_eebackupFile, ( BYTE *)&Xmem.file_buffer, 512, &written ) ;
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
	result = f_read( &SharedMemory.g_eebackupFile, Xmem.file_buffer, 512, &nread ) ;
	CoTickDelay(1) ;					// 2mS
// Write eeprom here
	I2C_EE_BufferWrite( Xmem.file_buffer, (blockNo << 9), 512 ) ;
//	write32_eeprom_2K( (uint32_t)blockNo << 11, Eeprom_buffer.data.buffer2K ) ;
	
//  read32_eeprom_data( (blockNo << 11), ( uint8_t *)&Eeprom_buffer.data.buffer2K, 2048, 0 ) ;
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



