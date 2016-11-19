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
#include "myeeprom.h"
#include "file.h"
#include "eeprom_rlc.h"





#define EEPROM_BUFFER_SIZE ((sizeof(ModelData) + sizeof( struct t_eeprom_header ) + 3)/4)

uint8_t *Eeprom ;

ModelData g_oldmodel ;
uint32_t DefaultModelType = 1 ;

struct t_eeprom_buffer
{
	struct t_eeprom_header header ;
	union t_eeprom_data
	{
		EEGeneral general_data ;
		SKYModelData model_data ;
		uint32_t words[ 1022 ] ;		// + 2 for header = 1024 words
	} data ;	
} Eeprom_buffer ;

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


// Read eeprom data starting at random address
uint32_t read32_eeprom_data( uint32_t eeAddress, uint8_t *buffer, uint32_t size )
{
	uint32_t i ;
	
	for ( i = 0 ; i < size ; i += 1 )
	{
		*buffer++ = *(Eeprom+eeAddress++ ) ;
	}
	return 1 ;
}

uint32_t get_current_block_number( uint32_t block_no, uint16_t *p_size, uint32_t *p_seq )
{
	struct t_eeprom_header b0 ;
	struct t_eeprom_header b1 ;
  uint32_t sequence_no ;
  uint16_t size ;
        read32_eeprom_data( block_no << 12, ( uint8_t *)&b0, sizeof(b0) ) ;		// Sequence # 0
        read32_eeprom_data( (block_no+1) << 12, ( uint8_t *)&b1, sizeof(b1) ) ;	// Sequence # 1

	if ( ee32_check_header( &b0 ) == 0 )
	{
		b0.sequence_no = 0 ;
		b0.data_size = 0 ;
		b0.flags = 0 ;
	}

	size = b0.data_size ;
  sequence_no = b0.sequence_no ;
	if ( ee32_check_header( &b0 ) == 0 )
	{
		if ( ee32_check_header( &b1 ) != 0 )
		{
  		size = b1.data_size ;
		  sequence_no = b1.sequence_no ;
			block_no += 1 ;
		}
		else
		{
			size = 0 ;
			sequence_no = 1 ;
		}
	}
	else
	{
		if ( ee32_check_header( &b1 ) != 0 )
		{
			if ( b1.sequence_no > b0.sequence_no )
			{
	  		size = b1.data_size ;
			  sequence_no = b1.sequence_no ;
				block_no += 1 ;
			}
		}
	}
  
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
//	Block_needs_erasing = erase ;		
  
	return block_no ;
}

void fill_file_index( struct t_radioData *radioData )
{
	uint32_t i ;
	for ( i = 0 ; i < MAX_IMODELS + 1 ; i += 1 )
	{
    radioData->File_system[i].block_no = get_current_block_number( i * 2, &radioData->File_system[i].size, &radioData->File_system[i].sequence_no ) ;
	}
}

void ee32LoadModelName( struct t_radioData *radioData, uint8_t id, unsigned char*buf, uint8_t len )
{
	if(id<=MAX_IMODELS)
  {
		memset(buf,' ',len);
		buf[len] = '\0' ;
    if ( radioData->File_system[id].size > sizeof( radioData->models[0].name) )
		{
      read32_eeprom_data( ( radioData->File_system[id].block_no << 12) + 8, ( uint8_t *)buf, sizeof( radioData->models[0].name) ) ;
		}
  }
}

void ee32_read_model_names(struct t_radioData *radioData)
{
	uint32_t i ;

	for ( i = 1 ; i <= MAX_IMODELS ; i += 1 )
	{
    ee32LoadModelName( radioData, i, radioData->ModelNames[i], sizeof( radioData->models[0].name) ) ;
	}
}



//bool ee32LoadGeneral()
//{
//	uint16_t size ;
	
//	size = File_system[0].size ;

//  memset(&g_eeGeneral, 0, sizeof(EEGeneral));

//	if ( size > sizeof(EEGeneral) )
//	{
//		size = sizeof(EEGeneral) ;
//	}

//	if ( size )
//	{
//    read32_eeprom_data( ( File_system[0].block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&g_eeGeneral, size ) ;
//	}
//	else
//	{
//		return false ;		// No data to load
//	}

////  for(uint8_t i=0; i<sizeof(g_eeGeneral.ownerName);i++) // makes sure name is valid
////  {
////      uint8_t idx = char2idx(g_eeGeneral.ownerName[i]);
////      g_eeGeneral.ownerName[i] = idx2char(idx);
////  }

////  if(g_eeGeneral.myVers<MDVERS)
////      sysFlags |= sysFLAG_OLD_EEPROM; // if old EEPROM - Raise flag

//  g_eeGeneral.myVers   =  MDVERS; // update myvers

//  uint16_t sum=0;
//  if(size>(sizeof(EEGeneral)-20)) for(uint8_t i=0; i<12;i++) sum+=g_eeGeneral.calibMid[i];
//	else return false ;
//  return g_eeGeneral.chkSum == sum;
//}



//EFile::EFile()
//{
//    memset(&eeprom,0,sizeof(eeprom));
//    eeFs = (EeFs*)&eeprom;

//    EeFsFormat();
//}

//void EFile::load(void* buf)
//{
//    memcpy(&eeprom,buf,EESIZE);
//}

//void EFile::save(void* buf)
//{
//    memcpy(buf,&eeprom,EESIZE);
//}

//void EFile::eeprom_read_block (void *pointer_ram, uint16_t pointer_eeprom, size_t size)
//{
//  memcpy(pointer_ram,&eeprom[pointer_eeprom],size);
//}
//void EFile::eeWriteBlockCmp(void *i_pointer_ram, uint16_t i_pointer_eeprom, size_t size)
//{
//  memcpy(&eeprom[i_pointer_eeprom],i_pointer_ram,size);
//}


//uint8_t EFile::EeFsRead(uint8_t blk,uint8_t ofs){
//  uint8_t ret;
//  eeprom_read_block(&ret,(uint16_t)(blk*BS+ofs),1);
//  return ret;
//}
//void EFile::EeFsWrite(uint8_t blk,uint8_t ofs,uint8_t val){
//  eeWriteBlockCmp(&val, (uint16_t)(blk*BS+ofs), 1);
//}

//uint8_t EFile::EeFsGetLink(uint8_t blk){
//  return EeFsRead( blk,0);
//}
//void EFile::EeFsSetLink(uint8_t blk,uint8_t val){
//  EeFsWrite( blk,0,val);
//}
//uint8_t EFile::EeFsGetDat(uint8_t blk,uint8_t ofs){
//  return EeFsRead( blk,ofs+1);
//}
//void EFile::EeFsSetDat(uint8_t blk,uint8_t ofs,uint8_t*buf,uint8_t len){
//  //EeFsWrite( blk,ofs+1,val);
//  eeWriteBlockCmp(buf, (uint16_t)(blk*BS+ofs+1), len);
//}
//void EFile::EeFsFlushFreelist()
//{
//  //eeWriteBlockCmp(eeFs->freeList,eeFs->freeList ,sizeof(eeFs->freeList));
//}
//void EFile::EeFsFlush()
//{
//  eeWriteBlockCmp(eeFs, 0,sizeof(eeFs));
//}

//uint16_t EFile::EeFsGetFree()
//{
//  uint16_t  ret = 0;
//  uint8_t i = eeFs->freeList;
//  while( i ){
//    ret += BS-1;
//    i = EeFsGetLink(i);
//  }
//  return ret;
//}
//void EFile::EeFsFree(uint8_t blk){///free one or more blocks
//  uint8_t i = blk;
//  while( EeFsGetLink(i)) i = EeFsGetLink(i);
//  EeFsSetLink(i,eeFs->freeList);
//  eeFs->freeList = blk; //chain in front
//  EeFsFlushFreelist();
//}
//uint8_t EFile::EeFsAlloc(){ ///alloc one block from freelist
//  uint8_t ret=eeFs->freeList;
//  if(ret){
//    eeFs->freeList = EeFsGetLink(ret);
//    EeFsFlushFreelist();
//    EeFsSetLink(ret,0);
//  }
//  return ret;
//}

//int8_t EFile::EeFsck()
//{
//  uint8_t *bufp;
//  static uint8_t buffer[BLOCKS];
//  bufp = buffer;
//  memset(bufp,0,BLOCKS);
//  uint8_t blk ;
//  int8_t ret=0;
//  for(uint8_t i = 0; i <= MAXFILES; i++){
//    uint8_t *startP = i==MAXFILES ? &(eeFs->freeList) : &(eeFs->files[i].startBlk);
//    uint8_t lastBlk = 0;
//    blk = *startP;
//      //if(i == MAXFILES) blk = eeFs->freeList;
//      //    else              blk = eeFs->files[i].startBlk;
//    while(blk){
//      //      if(blk <  FIRSTBLK ) goto err_1; //bad blk index
//      //      if(blk >= BLOCKS   ) goto err_2; //bad blk index
//      //      if(bufp[blk])        goto err_3; //blk double usage
//      if( (   blk <  FIRSTBLK ) //goto err_1; //bad blk index
//          || (blk >= BLOCKS   ) //goto err_2; //bad blk index
//          || (bufp[blk]       ))//goto err_3; //blk double usage
//      {
//        if(lastBlk){
//          EeFsSetLink(lastBlk,0);
//        }else{
//          *startP = 0; //interrupt chain at startpos
//          EeFsFlush();
//        }
//        blk=0; //abort
//      }else{
//        bufp[blk] = i+1;
//        lastBlk   = blk;
//        blk       = EeFsGetLink(blk);
//      }
//    }
//  }
//  for(blk = FIRSTBLK; blk < BLOCKS; blk++){
//    if(bufp[blk]==0) {       //goto err_4; //unused block
//      EeFsSetLink(blk,eeFs->freeList);
//      eeFs->freeList = blk; //chain in front
//      EeFsFlushFreelist();
//    }
//  }
//  //  if(0){
//    //err_4: ret--;
//    //err_3: ret--;
//    //    err_2: ret--;
//    //    err_1: ret--;
//  //  }
//  return ret;
//}

//void EFile::format()
//{
//    EeFsFormat();
//}

//void EFile::EeFsFormat()
//{
////  if(sizeof(eeFs) != RESV){
////    extern void eeprom_RESV_mismatch();
////    eeprom_RESV_mismatch();
////  }
//  memset(eeFs,0, sizeof(EeFs));
//  eeFs->version  = EEFS_VERS;
//  eeFs->mySize   = sizeof(EeFs);
//  eeFs->freeList = 0;
//  eeFs->bs       = BS;
//  for(uint8_t i = FIRSTBLK; i < BLOCKS; i++) EeFsSetLink(i,i+1);
//  EeFsSetLink(BLOCKS-1, 0);
//  eeFs->freeList = FIRSTBLK;
//  EeFsFlush();
//}
//bool EFile::EeFsOpen()
//{
//  eeprom_read_block(&eeFs,0,sizeof(eeFs));

//  return eeFs->version == EEFS_VERS && eeFs->mySize  == sizeof(eeFs);
//}

//bool EFile::exists(uint8_t i_fileId)
//{
//  return eeFs->files[i_fileId].startBlk;
//}

//void EFile::swap(uint8_t i_fileId1,uint8_t i_fileId2)
//{
//  DirEnt            tmp = eeFs->files[i_fileId1];
//  eeFs->files[i_fileId1] = eeFs->files[i_fileId2];
//  eeFs->files[i_fileId2] = tmp;;
//  EeFsFlush();
//}

//void EFile::rm(uint8_t i_fileId){
//  uint8_t i = eeFs->files[i_fileId].startBlk;
//  memset(&(eeFs->files[i_fileId]), 0, sizeof(eeFs->files[i_fileId]));
//  EeFsFlush(); //chained out

//  if(i) EeFsFree( i ); //chain in
//}

//uint16_t EFile::size(uint8_t id){
//  return eeFs->files[id].size;
//}


//uint8_t EFile::openRd(uint8_t i_fileId){
//  m_fileId = i_fileId;
//  m_pos      = 0;
//  m_currBlk  = eeFs->files[m_fileId].startBlk;
//  m_ofs      = 0;
//  m_bRlc     = 0;
//  m_err      = ERR_NONE;       //error reasons
//  return  eeFs->files[m_fileId].typ;
//}
//uint8_t EFile::read(uint8_t*buf,uint16_t i_len){
//  uint16_t len = eeFs->files[m_fileId].size - m_pos;
//  if(len < i_len) i_len = len;
//  len = i_len;
//  while(len)
//  {
//    if(!m_currBlk) break;
//    *buf++ = EeFsGetDat(m_currBlk, m_ofs++);
//    if(m_ofs>=(BS-1)){
//      m_ofs=0;
//      m_currBlk=EeFsGetLink(m_currBlk);
//    }
//    len--;
//  }
//  m_pos += i_len - len;
//  return i_len - len;
//}
//uint16_t EFile::readRlc(uint8_t*buf,uint16_t i_len){
//  uint16_t i;
//  for( i=0; i<i_len; ){
//    if((m_bRlc&0x7f) == 0) {
//      if(read(&m_bRlc,1)!=1) break; //read how many bytes to read
//    }
//    //assert(m_bRlc & 0x7f);
//    uint8_t l=m_bRlc&0x7f;
//    if((uint16_t)l>(i_len-i)) l = (uint8_t)(i_len-i);
//    if(m_bRlc&0x80){       // if contains high byte
//      memset(&buf[i],0,l); // write l zeros
//    }else{
//      uint8_t lr = read(&buf[i],l); // read and write l bytes
//      if(lr!=l) return i+lr;
//    }
//    i    += l;
//    m_bRlc -= l;
//  }
//  return i;
//}
//uint8_t EFile::write(uint8_t*buf,uint8_t i_len){
//  uint8_t len=i_len;
//  if(!m_currBlk && m_pos==0)
//  {
//    eeFs->files[m_fileId].startBlk = m_currBlk = EeFsAlloc();
//  }
//  while(len)
//  {
//    if(!m_currBlk) {
//      m_err = ERR_FULL;
//      break;
//    }
//    if(m_ofs>=(BS-1)){
//      m_ofs=0;
//      if( ! EeFsGetLink(m_currBlk) ){
//        EeFsSetLink(m_currBlk, EeFsAlloc());
//      }
//      m_currBlk = EeFsGetLink(m_currBlk);
//    }
//    if(!m_currBlk) {
//      m_err = ERR_FULL;
//      break;
//    }
//    uint8_t l = BS-1-m_ofs; if(l>len) l=len;
//    EeFsSetDat(m_currBlk, m_ofs, buf, l);
//    buf   +=l;
//    m_ofs +=l;
//    len   -=l;
//  }
//  m_pos += i_len - len;
//  return   i_len - len;
//}
//void EFile::create(uint8_t i_fileId, uint8_t typ){
//  openRd(i_fileId); //internal use
//  eeFs->files[i_fileId].typ      = typ;
//  eeFs->files[i_fileId].size     = 0;
//}
//void EFile::closeTrunc()
//{
//  uint8_t fri=0;
//  eeFs->files[m_fileId].size     = m_pos;
//  if(m_currBlk && ( fri = EeFsGetLink(m_currBlk)))    EeFsSetLink(m_currBlk, 0);
//  EeFsFlush(); //chained out

//  if(fri) EeFsFree( fri );  //chain in
//}

//uint16_t EFile::writeRlc(uint8_t i_fileId, uint8_t typ,uint8_t*buf,uint16_t i_len){
//  create(i_fileId,typ);
//  bool    state0 = true;
//  uint8_t cnt    = 0;
//  uint16_t i;

//  //RLE compression:
//  //rb = read byte
//  //if (rb | 0x80) write rb & 0x7F zeros
//  //else write rb bytes
//  for( i=0; i<=i_len; i++)
//  {
//    bool nst0 = buf[i] == 0;                   
//    if( nst0 && !state0 && buf[i+1]!=0) nst0 = false ;
//    if(nst0 != state0 || cnt>=0x7f || i==i_len){
//      if(state0){  
//        if(cnt>0){
//          cnt|=0x80;
//          if( write(&cnt,1)!=1)           goto error;
//          cnt=0;
//        }
//      }else{
//        if(cnt>0) {
//          if( write(&cnt,1) !=1)            goto error;
//          uint8_t ret=write(&buf[i-cnt],cnt);
//          if( ret !=cnt) { cnt-=ret;        goto error;}
//          cnt=0;
//        }
//      }
//      state0 = nst0;
//    }
//    cnt++;
//  }
//  if(0){
//    error:
//    i_len = i - (cnt & 0x7f);
//  }
//  closeTrunc();
//  return i_len;
//}
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
  dest->RxNum_unused = source->RxNum ;
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
    dst->disableExpoDr = src->disableExpoDr ;
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
		dst->ratio = src->ratio ;
		dst->alarms_value[0] = src->alarms_value[0] ;
		dst->alarms_value[1] = src->alarms_value[1] ;
		dst->alarms_level = src->alarms_level ;
		dst->alarms_greater = src->alarms_greater ;
		dst->type = src->type ;
		dst->gain = 1 ;
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

bool ee32LoadGeneral(struct t_radioData *radioData)
{
	uint16_t size ;
	
	size = radioData->File_system[0].size ;

	if ( size > sizeof(struct t_OldEEGeneral) )
	{
		// models are to default to ver 2
		DefaultModelType = 2 ;
	}
  
	memset( &radioData->generalSettings, 0, sizeof(EEGeneral));

	if ( size > sizeof(EEGeneral) )
	{
		size = sizeof(EEGeneral) ;
	}

	if ( size )
	{
    read32_eeprom_data( ( radioData->File_system[0].block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&radioData->generalSettings, size ) ;
	}
	else
	{
		return false ;		// No data to load
	}

//  if(g_eeGeneral.myVers<MDVERS)
//      sysFlags |= sysFLAG_OLD_EEPROM; // if old EEPROM - Raise flag

//  g_eeGeneral.myVers   =  MDVERS; // update myvers

  uint16_t sum=0;
  if(size>(sizeof(EEGeneral)-20)) for(uint8_t i=0; i<12;i++) sum+=radioData->generalSettings.calibMid[i];
	else return false ;
  return radioData->generalSettings.chkSum == sum;
}

void ee32LoadModel(struct t_radioData *radioData, uint8_t id)
{
	uint16_t size ;
	uint8_t version = 255 ;

    if(id<MAX_IMODELS)
    {
			size =  radioData->File_system[id+1].size ;
			if ( sizeof(SKYModelData) < 720 )
			{
				version = 0 ;
			}
			memset(&radioData->models[id], 0, sizeof(SKYModelData));

			if ( size < 720 )
			{
				memset(&g_oldmodel, 0, sizeof(g_oldmodel));
				if ( size > sizeof(g_oldmodel) )
				{
					size = sizeof(g_oldmodel) ;
				}
			}
       
			if ( size > sizeof(SKYModelData) )
			{
				size = sizeof(SKYModelData) ;
			}
			 
      if(size<256) // if not loaded a fair amount
      {
//        modelDefault(id) ;
      }
			else
			{
				if ( size < 720 )
				{
					read32_eeprom_data( ( radioData->File_system[id+1].block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&g_oldmodel, size ) ;
					convertModel( &radioData->models[id], &g_oldmodel ) ;
				}
				else
				{
					read32_eeprom_data( ( radioData->File_system[id+1].block_no << 12) + sizeof( struct t_eeprom_header), ( uint8_t *)&radioData->models[id], size ) ;
				}	 
				
				if ( version != 255 )
				{
          radioData->models[id].modelVersion = 0 ;					// default version number
				}
			}

//      for(uint8_t i=0; i<sizeof(g_model.name);i++) // makes sure name is valid
//      {
//          uint8_t idx = char2idx(radioData->models[id].name[i]);
//          radioData->models[id].name[i] = idx2char(idx);
//      }
      
//      radioData->models[id].modelVersion = MDSKYVERS ; //update mdvers

			if ( radioData->models[id].numBlades == 0 )
			{
				radioData->models[id].numBlades = radioData->models[id].xnumBlades + 2 ;				
			}
//			if ( radioData->models[id].protocol == PROTO_PPM16 )			// PPM16 ?
//			{
//				radioData->models[id].protocol = PROTO_PPM ;
//			}
    }
}


uint32_t rawloadFile( struct t_radioData *radioData, uint8_t *eeprom )
{
	uint32_t i ;
	Eeprom  = eeprom ;
	fill_file_index(radioData) ;
	ee32_read_model_names(radioData) ;
	DefaultModelType = 1 ;
	ee32LoadGeneral(radioData) ;
	for ( i = 0 ; i < MAX_IMODELS ; i += 1 )
	{
		ee32LoadModel( radioData, i ) ;
	}
	radioData->valid = 1 ;

	return 1 ;
	
}


uint32_t rawsaveFile( t_radioData *radioData, uint8_t *eeprom )
{
	uint8_t csum ;
	uint32_t i ;

	memset( eeprom, 0xFF, EEFULLSIZE ) ;
	if ( ( radioData->type == 1 ) || ( radioData->type == 2 ) )
	{
		// Taranis type
		EeFsFormat( eeprom ) ;
    RlcFile *filePtr = new RlcFile ;
    filePtr->writeRlc( FILE_GENERAL, FILE_TYP_GENERAL, (uint8_t *) &radioData->generalSettings, sizeof(EEGeneral), 1 ) ;
		for ( i = 1 ; i <= MAX_IMODELS ; i += 1 )
		{
			if ( radioData->File_system[i].size )
			{		
      	filePtr->writeRlc( i, FILE_TYP_GENERAL, (uint8_t*) &radioData->models[i-1], sizeof(SKYModelData), 1 ) ;
			}
		}
		delete filePtr ;
		return 1 ;
	}
  memset( &Eeprom_buffer, 0xFF, sizeof( Eeprom_buffer ) ) ;

 	Eeprom_buffer.header.sequence_no = 1 ;
	Eeprom_buffer.header.data_size = sizeof(EEGeneral) ;
	Eeprom_buffer.header.flags = 0 ;
	csum = byte_checksum( ( uint8_t *) &Eeprom_buffer.header, 7 ) ;
	Eeprom_buffer.header.hcsum = csum ;

	memcpy( &Eeprom_buffer.data.general_data, &radioData->generalSettings, sizeof(EEGeneral) ) ;
	memcpy( eeprom, &Eeprom_buffer, 4096 ) ;		// Write block 0

	for ( i = 1 ; i <= MAX_IMODELS ; i += 1 )
	{
		if ( radioData->File_system[i].size )
		{
    	memset( &Eeprom_buffer, 0xFF, sizeof( Eeprom_buffer ) ) ;
	 		Eeprom_buffer.header.sequence_no = 1 ;
			Eeprom_buffer.header.data_size = sizeof(SKYModelData) ;
			Eeprom_buffer.header.flags = 0 ;
			csum = byte_checksum( ( uint8_t *) &Eeprom_buffer.header, 7 ) ;
			Eeprom_buffer.header.hcsum = csum ;
			memcpy( &Eeprom_buffer.data.model_data, &radioData->models[i-1], sizeof(SKYModelData) ) ;
			memcpy( eeprom + 8192*i, &Eeprom_buffer, 4096 ) ;		// Write block 2*n
		}
		
	}
  return 1 ;
}



