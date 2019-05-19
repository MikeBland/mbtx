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
#include <string.h>

#include "../ersky9x.h"
#include "../templates.h"
#include "../myeeprom.h"
#include "../stringidx.h"
#include "../ff.h"
#include "../lcd.h"

uint16_t General_timer ;
uint16_t Model_timer ;
uint8_t Model_dirty ;

unsigned char ModelNames[MAX_MODELS+1][sizeof(g_model.name)+1] ;		// Allow for general

extern const char *readNames(void) ;
extern const char * readGeneral(void) ;
extern void eeLoadModel(uint8_t id) ;
extern bool eeModelExists(uint8_t id) ;
extern bool ee32CopyModel(uint8_t dst, uint8_t src) ;

#define RADIO_PATH           "/RADIO"   // no trailing slash = important

int16_t *const CalibMid[] =
{ 
	&g_eeGeneral.calibMid[0],
	&g_eeGeneral.calibMid[1],
	&g_eeGeneral.calibMid[2],
	&g_eeGeneral.calibMid[3],
	&g_eeGeneral.calibMid[4],
	&g_eeGeneral.calibMid[5],
	&g_eeGeneral.calibMid[6],
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
	&g_eeGeneral.x9dcalibMid,
#if REVPLUS
#ifdef REV9E
	&g_eeGeneral.xcalibMid[0],
	&g_eeGeneral.xcalibMid[1],
	&g_eeGeneral.xcalibMid[2],
#endif
	&g_eeGeneral.x9dPcalibMid
#endif
#if defined(PCBX12D) || defined(PCBX10)
	&g_eeGeneral.xcalibMid[0],
	&g_eeGeneral.xcalibMid[1],
	&g_eeGeneral.xcalibMid[2],
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
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
	&g_eeGeneral.x9dcalibSpanPos,
#if REVPLUS
#ifdef REV9E
	&g_eeGeneral.xcalibSpanPos[0],
	&g_eeGeneral.xcalibSpanPos[1],
	&g_eeGeneral.xcalibSpanPos[2],
#endif
	&g_eeGeneral.x9dPcalibSpanPos
#endif
#if defined(PCBX12D) || defined(PCBX10)
	&g_eeGeneral.xcalibSpanPos[0],
	&g_eeGeneral.xcalibSpanPos[1],
	&g_eeGeneral.xcalibSpanPos[2],
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
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
	&g_eeGeneral.x9dcalibSpanNeg,
#if REVPLUS
#ifdef REV9E
	&g_eeGeneral.xcalibSpanNeg[0],
	&g_eeGeneral.xcalibSpanNeg[1],
	&g_eeGeneral.xcalibSpanNeg[2],
#endif
	&g_eeGeneral.x9dPcalibSpanNeg
#endif
#if defined(PCBX12D) || defined(PCBX10)
	&g_eeGeneral.xcalibSpanNeg[0],
	&g_eeGeneral.xcalibSpanNeg[1],
	&g_eeGeneral.xcalibSpanNeg[2],
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
  g_eeGeneral.myVers   =  MDSKYVERS;
//  g_eeGeneral.currModel=  0;
#ifdef PCB9XT
  g_eeGeneral.contrast = 25 ;
#else
#if defined(PCBX12D) || defined(PCBX10)
  g_eeGeneral.contrast = 10;
#else
  g_eeGeneral.contrast = 18 ;
#endif
#endif
#if defined(PCBX12D) || defined(PCBX10)
  g_eeGeneral.vBatWarn = 88 ;
#else
  g_eeGeneral.vBatWarn = 65;
#endif
  g_eeGeneral.stickMode=  1;
	g_eeGeneral.disablePotScroll=  1;
#if defined(PCBX12D) || defined(PCBX10)
	g_eeGeneral.bright = 0 ;
#else
	g_eeGeneral.bright = 50 ;
#endif
	g_eeGeneral.volume = 2 ;
	g_eeGeneral.lightSw = MAX_SKYDRSWITCH ;	// ON
	g_eeGeneral.filterInput = 1 ;
	g_eeGeneral.beeperVal = 3 ;
	g_eeGeneral.gpsFormat = 1 ;
#if defined(PCBX12D) || defined(PCBX10)
	g_eeGeneral.rotaryDivisor = 2 ;
#endif

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
	g_model.trimInc = 2 ;
  
	applyTemplate(0); //default 4 channel template
#if defined(PCBX12D) || defined(PCBX10)
  memcpy(ModelNames[id+1], g_model.name, sizeof(g_model.name));
#endif

	// Set all mode trims to be copies of FM0
	for ( uint32_t i = 0 ; i < MAX_MODES ; i += 1 )
	{
		g_model.phaseData[i].trim[0] = TRIM_EXTENDED_MAX + 1 ;
		g_model.phaseData[i].trim[1] = TRIM_EXTENDED_MAX + 1 ;
		g_model.phaseData[i].trim[2] = TRIM_EXTENDED_MAX + 1 ;
		g_model.phaseData[i].trim[3] = TRIM_EXTENDED_MAX + 1 ;
	}
	g_model.Module[0].protocol = PROTO_OFF ;
	g_model.Module[1].protocol = PROTO_OFF ;
	g_model.modelVoice = -1 ;
	g_model.Module[0].pxxRxNum = id-1 ;
	g_model.Module[1].pxxRxNum = id-1 ;
	g_model.rxVratio = 132 ;
	eeDirty(EE_MODEL) ;
}

//bool eeDuplicateModel(uint8_t id)
//{
//	return 0 ;
//}
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

	ee32CopyModel( i+1, id ) ;

	return true ;
}

//uint32_t readGeneralFromBackupRam(void) ;
//uint32_t readModelFromBackupRam( uint8_t index) ;

void eeReadAll()
{
  DIR folder ;
  char filename[10] ;
  FRESULT result ;

  cpystr( (uint8_t *)filename, (uint8_t *)RADIO_PATH ) ;
	result = f_opendir( &folder, filename ) ;
  if (result != FR_OK)
	{
    if (result == FR_NO_PATH)
      result = f_mkdir(filename) ;
  }
	
//	if ( !readGeneralFromBackupRam() )
	{
		if ( readGeneral() )
		{
			generalDefault() ;
		}
	}

	uint32_t red ;
	uint32_t green ;
	uint32_t blue ;
	
	red = g_eeGeneral.backgroundColour >> 11 ;
	green = ( g_eeGeneral.backgroundColour >> 6 ) & 0x1F ;
	blue = g_eeGeneral.backgroundColour & 0x1F ;

	if ( (red < 6) && (green < 6) && (blue < 6) )
	{
		g_eeGeneral.backgroundColour = LCD_BACKGROUND ;
	}
  LcdBackground = g_eeGeneral.backgroundColour ;
//	if ( !readModelFromBackupRam(g_eeGeneral.currModel + 1) )
	{
		eeLoadModel(g_eeGeneral.currModel) ;
	}
	readNames() ;
}


//void eeReadAll()
//{
//  if(!ee32LoadGeneral() )
//  {
//    alert((char const *)PSTR(STR_BAD_EEPROM), true);
//#ifdef PCB9XT
//    g_eeGeneral.contrast = 25 ;
//#else
//    g_eeGeneral.contrast = 18 ;
//#endif
//    message(PSTR(STR_EE_FORMAT));
//    generalDefault();

//    modelDefault(0);
//		STORE_GENERALVARS;
//    STORE_MODELVARS;        
//  }
//	else
//	{
//  	ee32LoadModel(g_eeGeneral.currModel);
//	}

//	// Now update the trainer values if necessary.
//  uint32_t i ;
//	for ( i = 0 ; i < 4 ; i += 1 )
//	{
//		if ( g_eeGeneral.Xtrainer.mix[i].swtch != -16 )
//		{
//			g_eeGeneral.XexTrainer[i].swtch = g_eeGeneral.Xtrainer.mix[i].swtch ;
//			g_eeGeneral.XexTrainer[i].studWeight = g_eeGeneral.Xtrainer.mix[i].studWeight * 13 / 4 ;
//			g_eeGeneral.Xtrainer.mix[i].swtch = -16 ;
//			STORE_GENERALVARS ;
//		}
//	}
//}


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


void ee32StoreGeneral()
{
//	General_dirty = 1 ;
	General_timer = 500 ;		// 5 seconds timeout before writing
}

void ee32StoreModel( uint8_t modelNumber, uint8_t trim )
{
	Model_dirty = modelNumber + 1 ;
	Model_timer = 500 ;	
	ee32_update_name( Model_dirty, (uint8_t *)&g_model ) ;		// In case it's changed
}


void eeModelChanged()
{
	if ( Model_timer == 0 )
	{
		ee32StoreModel( g_eeGeneral.currModel, EE_MODEL ) ;
		Model_timer = 30000 ;	// 5 minutes
	}
}


void eeDirty(uint8_t msk)
{
  if(!msk) return ;

	// New file system operations
	if ( msk & EE_GENERAL )
	{
		ee32StoreGeneral() ;
	}
	if ( msk & EE_MODEL )
	{
		ee32StoreModel( g_eeGeneral.currModel, msk & EE_TRIM ) ;
	}

}

//void writeGeneralToBackupRam(void) ;
//void writeModelToBackupRam( uint8_t index ) ;

void ee32_process()
{
	if ( General_timer )
	{
		if ( --General_timer == 0 )
		{
			// Time to write g_eeGeneral
//			writeGeneralToBackupRam() ;
		}
	}
	if ( Model_timer )
	{
		if ( --Model_timer == 0 )
		{
			// Time to write model
//			writeModelToBackupRam(Model_dirty) ;
		}
	}
}




