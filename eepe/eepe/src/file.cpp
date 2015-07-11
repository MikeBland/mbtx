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


#include "stdio.h"
#include <stdint.h>
#include "string.h"
#include "pers.h"
#include "file.h"
#include <QSettings>



EFile::EFile()
{
    QSettings settings("er9x-eePe", "eePe");
    memset(&eeprom,0,sizeof(eeprom));
    eeFs = (EeFs*)&eeprom;
		m_type = settings.value("processor", 1).toInt() ;
    EeFsFormat();
}

void EFile::load(void* buf)
{
    memcpy(&eeprom,buf,m_type ? EESIZE128 : EESIZE64 ) ;
}

void EFile::save(void* buf)
{
    memcpy(buf,&eeprom, m_type ? EESIZE128 : EESIZE64 ) ;
}

void EFile::eeprom_read_block (void *pointer_ram, uint16_t pointer_eeprom, size_t size)
{
  memcpy(pointer_ram,&eeprom[pointer_eeprom],size);
}
void EFile::eeWriteBlockCmp(void *i_pointer_ram, uint16_t i_pointer_eeprom, size_t size)
{
  memcpy(&eeprom[i_pointer_eeprom],i_pointer_ram,size);
}


uint8_t EFile::EeFsRead(uint8_t blk,uint8_t ofs){
  uint8_t ret;
  eeprom_read_block(&ret,(uint16_t)(blk*BS+ofs),1);
  return ret;
}
void EFile::EeFsWrite(uint8_t blk,uint8_t ofs,uint8_t val){
  eeWriteBlockCmp(&val, (uint16_t)(blk*BS+ofs), 1);
}

uint8_t EFile::EeFsGetLink(uint8_t blk){
  return EeFsRead( blk,0);
}
void EFile::EeFsSetLink(uint8_t blk,uint8_t val){
  EeFsWrite( blk,0,val);
}
uint8_t EFile::EeFsGetDat(uint8_t blk,uint8_t ofs){
  return EeFsRead( blk,ofs+1);
}
void EFile::EeFsSetDat(uint8_t blk,uint8_t ofs,uint8_t*buf,uint8_t len){
  //EeFsWrite( blk,ofs+1,val);
  eeWriteBlockCmp(buf, (uint16_t)(blk*BS+ofs+1), len);
}
void EFile::EeFsFlushFreelist()
{
  //eeWriteBlockCmp(eeFs->freeList,eeFs->freeList ,sizeof(eeFs->freeList));
}
void EFile::EeFsFlush()
{
  eeWriteBlockCmp(eeFs, 0,sizeof(eeFs));
}

uint16_t EFile::EeFsGetFree()
{
  uint16_t  ret = 0;
  uint8_t i = eeFs->freeList;
  while( i ){
    ret += BS-1;
    i = EeFsGetLink(i);
  }
  return ret;
}
void EFile::EeFsFree(uint8_t blk){///free one or more blocks
  uint8_t i = blk;
  while( EeFsGetLink(i)) i = EeFsGetLink(i);
  EeFsSetLink(i,eeFs->freeList);
  eeFs->freeList = blk; //chain in front
  EeFsFlushFreelist();
}
uint8_t EFile::EeFsAlloc(){ ///alloc one block from freelist
  uint8_t ret=eeFs->freeList;
  if(ret){
    eeFs->freeList = EeFsGetLink(ret);
    EeFsFlushFreelist();
    EeFsSetLink(ret,0);
  }
  return ret;
}

int8_t EFile::EeFsck()
{
  uint8_t *bufp;
  static uint8_t buffer[BLOCKS128];
  bufp = buffer;
  memset(bufp,0,BLOCKS128);
  uint16_t blk ;
  int8_t ret=0;
  for(uint8_t i = 0; i <= MAXFILES; i++){
    uint8_t *startP = i==MAXFILES ? &(eeFs->freeList) : &(eeFs->files[i].startBlk);
    uint8_t lastBlk = 0;
    blk = *startP;
      //if(i == MAXFILES) blk = eeFs->freeList;
      //    else              blk = eeFs->files[i].startBlk;
    while(blk){
      //      if(blk <  FIRSTBLK ) goto err_1; //bad blk index
      //      if(blk >= BLOCKS   ) goto err_2; //bad blk index
      //      if(bufp[blk])        goto err_3; //blk double usage
      if( (   blk <  FIRSTBLK ) //goto err_1; //bad blk index
          || (blk >= (m_type ? BLOCKS128 : BLOCKS64)   ) //goto err_2; //bad blk index
          || (bufp[blk]       ))//goto err_3; //blk double usage
      {
        if(lastBlk){
          EeFsSetLink(lastBlk,0);
        }else{
          *startP = 0; //interrupt chain at startpos
          EeFsFlush();
        }
        blk=0; //abort
      }else{
        bufp[blk] = i+1;
        lastBlk   = blk;
        blk       = EeFsGetLink(blk);
      }
    }
  }
  for(blk = FIRSTBLK; blk < (m_type ? BLOCKS128 : BLOCKS64); blk++){
    if(bufp[blk]==0) {       //goto err_4; //unused block
      EeFsSetLink(blk,eeFs->freeList);
      eeFs->freeList = blk; //chain in front
      EeFsFlushFreelist();
    }
  }
  //  if(0){
    //err_4: ret--;
    //err_3: ret--;
    //    err_2: ret--;
    //    err_1: ret--;
  //  }
  return ret;
}

void EFile::format()
{
    EeFsFormat();
}

void EFile::EeFsFormat()
{
//  if(sizeof(eeFs) != RESV){
//    extern void eeprom_RESV_mismatch();
//    eeprom_RESV_mismatch();
//  }
  memset(eeFs,0, sizeof(EeFs));
  eeFs->version  = m_type ? EEFS_VERS128 : EEFS_VERS ;
  eeFs->mySize   = sizeof(EeFs);
  eeFs->freeList = 0;
  eeFs->bs       = BS;
  for(uint8_t i = FIRSTBLK; i < (m_type ? BLOCKS128 : BLOCKS64) - 1 ; i++) EeFsSetLink(i,i+1);
  EeFsSetLink((m_type ? BLOCKS128 : BLOCKS64)-1, 0);
  eeFs->freeList = FIRSTBLK;
  EeFsFlush();
}
bool EFile::EeFsOpen()
{
	uint8_t eefs_vers ;
  eeprom_read_block(&eeFs,0,sizeof(eeFs));

	eefs_vers = m_type ? EEFS_VERS128 : EEFS_VERS ;
  return eeFs->version == eefs_vers && eeFs->mySize  == sizeof(eeFs);
}

bool EFile::exists(uint8_t i_fileId)
{
  return eeFs->files[i_fileId].startBlk;
}

void EFile::swap(uint8_t i_fileId1,uint8_t i_fileId2)
{
  DirEnt            tmp = eeFs->files[i_fileId1];
  eeFs->files[i_fileId1] = eeFs->files[i_fileId2];
  eeFs->files[i_fileId2] = tmp;;
  EeFsFlush();
}

void EFile::rm(uint8_t i_fileId){
  uint8_t i = eeFs->files[i_fileId].startBlk;
  memset(&(eeFs->files[i_fileId]), 0, sizeof(eeFs->files[i_fileId]));
  EeFsFlush(); //chained out

  if(i) EeFsFree( i ); //chain in
}

uint16_t EFile::size(uint8_t id){
  return eeFs->files[id].size;
}


uint8_t EFile::openRd(uint8_t i_fileId){
  m_fileId = i_fileId;
  m_pos      = 0;
  m_currBlk  = eeFs->files[m_fileId].startBlk;
  m_ofs      = 0;
  m_bRlc     = 0;
  m_err      = ERR_NONE;       //error reasons
  return  eeFs->files[m_fileId].typ;
}
uint8_t EFile::read(uint8_t*buf,uint16_t i_len){
  uint16_t len = eeFs->files[m_fileId].size - m_pos;
  if(len < i_len) i_len = len;
  len = i_len;
  while(len)
  {
    if(!m_currBlk) break;
    *buf++ = EeFsGetDat(m_currBlk, m_ofs++);
    if(m_ofs>=(BS-1)){
      m_ofs=0;
      m_currBlk=EeFsGetLink(m_currBlk);
    }
    len--;
  }
  m_pos += i_len - len;
  return i_len - len;
}
uint16_t EFile::readRlc(uint8_t*buf,uint16_t i_len){
  uint16_t i;
  for( i=0; i<i_len; ){
    if((m_bRlc&0x7f) == 0) {
      if(read(&m_bRlc,1)!=1) break; //read how many bytes to read
    }
    //assert(m_bRlc & 0x7f);
    uint8_t l=m_bRlc&0x7f;
    if((uint16_t)l>(i_len-i)) l = (uint8_t)(i_len-i);
    if(m_bRlc&0x80){       // if contains high byte
      memset(&buf[i],0,l); // write l zeros
    }else{
      uint8_t lr = read(&buf[i],l); // read and write l bytes
      if(lr!=l) return i+lr;
    }
    i    += l;
    m_bRlc -= l;
  }
  return i;
}
uint8_t EFile::write(uint8_t*buf,uint8_t i_len){
  uint8_t len=i_len;
  if(!m_currBlk && m_pos==0)
  {
    eeFs->files[m_fileId].startBlk = m_currBlk = EeFsAlloc();
  }
  while(len)
  {
    if(!m_currBlk) {
      m_err = ERR_FULL;
      break;
    }
    if(m_ofs>=(BS-1)){
      m_ofs=0;
      if( ! EeFsGetLink(m_currBlk) ){
        EeFsSetLink(m_currBlk, EeFsAlloc());
      }
      m_currBlk = EeFsGetLink(m_currBlk);
    }
    if(!m_currBlk) {
      m_err = ERR_FULL;
      break;
    }
    uint8_t l = BS-1-m_ofs; if(l>len) l=len;
    EeFsSetDat(m_currBlk, m_ofs, buf, l);
    buf   +=l;
    m_ofs +=l;
    len   -=l;
  }
  m_pos += i_len - len;
  return   i_len - len;
}
void EFile::create(uint8_t i_fileId, uint8_t typ){
  openRd(i_fileId); //internal use
  eeFs->files[i_fileId].typ      = typ;
  eeFs->files[i_fileId].size     = 0;
}
void EFile::closeTrunc()
{
  uint8_t fri=0;
  eeFs->files[m_fileId].size     = m_pos;
  if(m_currBlk && ( fri = EeFsGetLink(m_currBlk)))    EeFsSetLink(m_currBlk, 0);
  EeFsFlush(); //chained out

  if(fri) EeFsFree( fri );  //chain in
}

uint16_t EFile::writeRlc(uint8_t i_fileId, uint8_t typ,uint8_t*buf,uint16_t i_len){
  create(i_fileId,typ);
  bool    state0 = true;
  uint8_t cnt    = 0;
  uint16_t i;

  //RLE compression:
  //rb = read byte
  //if (rb | 0x80) write rb & 0x7F zeros
  //else write rb bytes
  for( i=0; i<=i_len; i++)
  {
    bool nst0 = buf[i] == 0;                   
    if( nst0 && !state0 && buf[i+1]!=0) nst0 = false ;
    if(nst0 != state0 || cnt>=0x7f || i==i_len){
      if(state0){  
        if(cnt>0){
          cnt|=0x80;
          if( write(&cnt,1)!=1)           goto error;
          cnt=0;
        }
      }else{
        if(cnt>0) {
          if( write(&cnt,1) !=1)            goto error;
          uint8_t ret=write(&buf[i-cnt],cnt);
          if( ret !=cnt) { cnt-=ret;        goto error;}
          cnt=0;
        }
      }
      state0 = nst0;
    }
    cnt++;
  }
  if(0){
    error:
    i_len = i - (cnt & 0x7f);
  }
  closeTrunc();
  return i_len;
}

