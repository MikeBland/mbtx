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

#include "pers.h"
#include "file.h"
#include <QSettings>

#define FILE_TYP_GENERAL 1
#define FILE_TYP_MODEL   2


EEPFILE::EEPFILE()
{
    fileChanged = false;
//    theFile = new EFile();
    generalDefault();
}

bool EEPFILE::Changed()
{
    return fileChanged;
}

//bool EEPFILE::loadFile( RadioData &radioData,void* buf )
//{
    
		
		
		
////		theFile->load(buf);


//    EEGeneral g_eeGeneral;
//    int sz = getGeneralSettings(&g_eeGeneral);

//    if(sz<40) return false; //if it's too small then we have a corrupted memory

//    if(g_eeGeneral.myVers>MDVERS || (g_eeGeneral.myVers<MDVERS_r261 && g_eeGeneral.myVers!=4)) return false;

//    int16_t sum=0;
//    for(int i=0; i<12;i++) sum+=g_eeGeneral.calibMid[i];

//    return g_eeGeneral.chkSum == sum;
//}

void EEPFILE::saveFile(void* buf)
{
//    theFile->save(buf);
}

void EEPFILE::setChanged(bool v)
{
    fileChanged = v;
}

void EEPFILE::generalDefault()
{
  EEGeneral g_eeGeneral;
  memset(&g_eeGeneral,0,sizeof(g_eeGeneral));
  memset(&g_eeGeneral.ownerName,' ',sizeof(g_eeGeneral.ownerName));
  g_eeGeneral.myVers   =  MDSKYVERS ;
  g_eeGeneral.currModel=  0;
  g_eeGeneral.contrast = 30;
  g_eeGeneral.vBatWarn = 90;
  g_eeGeneral.stickMode=  1;
	g_eeGeneral.disablePotScroll=  1;
	g_eeGeneral.bright = 50 ;
	g_eeGeneral.volume = 2 ;
  for (int i = 0; i < 7; ++i) {
    g_eeGeneral.calibMid[i]     = 0x300;
    g_eeGeneral.calibSpanNeg[i] = 0x400;
    g_eeGeneral.calibSpanPos[i] = 0x300;
  }
  int16_t sum=0;
  for(int i=0; i<12;i++) sum+=g_eeGeneral.calibMid[i];
  g_eeGeneral.chkSum = sum;

  QSettings settings("er9x-eePskye", "eePskye");
  g_eeGeneral.templateSetup = settings.value("default_channel_order", 0).toInt();
  g_eeGeneral.stickMode = settings.value("default_mode", 1).toInt();

  putGeneralSettings(&g_eeGeneral);
}

bool EEPFILE::eeLoadGeneral()
{
  EEGeneral g_eeGeneral;
  int ret = getGeneralSettings(&g_eeGeneral);

  if (ret<(int)(sizeof(EEGeneral)-20)) return false;

  uint16_t sum=0;
  for(int i=0; i<12;i++) sum+=g_eeGeneral.calibMid[i];
  putGeneralSettings(&g_eeGeneral);
  return g_eeGeneral.chkSum == sum;
}

void EEPFILE::modelDefault(uint8_t id)
{
  ModelData g_model;
  memset(&g_model, 0, sizeof(g_model));
  memset(&g_model.name, ' ', sizeof(g_model.name));
  strcpy(g_model.name,"MODEL");
  g_model.name[5]='0'+(id+1)/10;
  g_model.name[6]='0'+(id+1)%10;
//  g_model.mdVers = MDVERS;
  g_model.trimInc = 2 ;

  for(uint8_t i= 0; i<4; i++){
    g_model.mixData[i].destCh = i+1;
    g_model.mixData[i].srcRaw = i+1;
    g_model.mixData[i].weight = 100;
  }

  putModel(&g_model,id);
}

void EEPFILE::DeleteModel(uint8_t id)
{
//    theFile->rm(id);
}


void EEPFILE::eeLoadModelName(uint8_t id,char*buf,uint8_t len)
{
  if(id<MAX_MODELS)
  {
    //eeprom_read_block(buf,(void*)modelEeOfs(id),sizeof(g_model.name));

    memset(buf,' ',len);
    *buf='0'+(id+1)/10; buf++;
    *buf='0'+(id+1)%10; buf++;
    *buf=':';           buf++;buf++;

    if(!eeModelExists(id))  // make sure we don't add junk
    {
        *buf=0; //terminate str
        return;
    }

//    theFile->openRd(FILE_MODEL(id));
//    uint16_t res = theFile->readRlc((uint8_t*)buf,sizeof(ModelData().name));
//    if(res == sizeof(ModelData().name) )
//    {
//      //buf+=len-5;
//      for(int i=0; i<(len-4); i++)
//      {
//          if(*buf==0) *buf=' ';
//          buf++;
//      }
//      *buf=0;buf--;
//      uint16_t sz=theFile->size(FILE_MODEL(id));
//      while(sz){ --buf; *buf='0'+sz%10; sz/=10;}
//    }
  }
}

void EEPFILE::eeLoadOwnerName(char*buf,uint8_t len)
{
    EEGeneral g_eeGeneral;
    int ret = getGeneralSettings(&g_eeGeneral);

    memset(buf,0,len);

    if (ret<(int)(sizeof(EEGeneral)-20)) return;
    memcpy(buf,&g_eeGeneral.ownerName,sizeof(g_eeGeneral.ownerName));
}

void EEPFILE::getModelName(uint8_t id, char* buf)
{
    ModelData g_model;
    memset(buf,0,sizeof(g_model.name)+1);
    if(getModel(&g_model,id))
        memcpy(buf,&g_model.name,sizeof(g_model.name));
}

bool EEPFILE::eeModelExists(uint8_t id)
{
  ModelData g_model;
  return getModel(&g_model, id);
}


int EEPFILE::getModel(ModelData* model, uint8_t id)
{
    int sz = 0;

    if(id<MAX_MODELS)
    {
//      theFile->openRd(FILE_MODEL(id));
//      sz = theFile->readRlc((uint8_t*)model, sizeof(ModelData));
    }

    return sz;
}

bool EEPFILE::putModel(ModelData* model, uint8_t id)
{
    int sz = 0;

    if(id<MAX_MODELS)
    {
//        sz = theFile->writeRlc(FILE_MODEL(id), FILE_TYP_MODEL, (uint8_t*)model, sizeof(ModelData));
//        sz = theFile->writeRlc(FILE_TMP, FILE_TYP_MODEL, (uint8_t*)model, sizeof(ModelData));
//        if(sz == sizeof(ModelData))
//            theFile->swap(FILE_MODEL(id),FILE_TMP); // do not need temp file, writeRlc should write the actual file
    }

    return (sz == sizeof(ModelData));
}

int EEPFILE::getGeneralSettings(EEGeneral* setData)
{
//    theFile->openRd(FILE_GENERAL);
//    int sz = theFile->readRlc((uint8_t*)setData, sizeof(EEGeneral));

	int sz = 0 ;
    return sz;
}

void EEPFILE::formatEFile()
{
//    theFile->format();
    generalDefault();
    fileChanged = true;
}

bool EEPFILE::putGeneralSettings(EEGeneral* setData)
{
    int sz = 0;

//    sz = theFile->writeRlc(FILE_GENERAL, FILE_TYP_GENERAL, (uint8_t*)setData, sizeof(EEGeneral));
//    sz = theFile->writeRlc(FILE_TMP, FILE_TYP_GENERAL, (uint8_t*)setData, sizeof(EEGeneral));
//    if(sz == sizeof(EEGeneral)) theFile->swap(FILE_GENERAL,FILE_TMP);

    return (sz == sizeof(EEGeneral));
}
