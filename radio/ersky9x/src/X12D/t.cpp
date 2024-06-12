#include <stdint.h>
#include <stdlib.h>
//#include <ctype.h>
#include <string.h>


#include "../ersky9x.h"
#include "../templates.h"
#include "../myeeprom.h"
#include "../audio.h"
#include "../sound.h"
#include "../lcd.h"
#include "../drivers.h"
#include "../file.h"

#include "../menus.h"
#include "../mixer.h"
#include "../timers.h"
#include "../pulses.h"
#include "../stringidx.h"

#include "../frsky.h"

#if defined(PCBX12D) || defined(PCBX10)
#include "../diskio.h"
#include "stm32f4xx_rcc.h"
#include "hal.h"
#include "sdio_sd.h"
#endif

#include "../sbus.h"

#include "../ff.h"
#include "../maintenance.h"

#include "../logicio.h"


#ifndef SIMU
#include "CoOS.h"
#endif

OS_STK main_stk[MAIN_STACK_SIZE] ;

#define EE_FILE_TYPE_GENERAL	'G'
#define EE_FILE_TYPE_MODEL		'M'


void ee32_update_name( uint32_t id, uint8_t *source ) ;
uint32_t xloadFile(const char * filename, uint8_t * data, uint16_t maxsize) ;
uint32_t xwriteFile( const char * filename, const uint8_t * data, uint16_t size, uint8_t type) ;

#define RADIO_PATH           "/RADIO"   // no trailing slash = important

struct t_backupRam
{
	uint32_t checksum ;
	uint16_t generalSize ;
	uint16_t modelSize ;
	uint32_t generalSequence ;
	uint32_t modelSequence ;
	uint16_t modelNumber ;
	uint8_t data[4096-18] ;
} ;


extern uint32_t CurrentFrameBuffer ;
extern uint8_t MultiData ;
extern uint16_t S_anaFilt[ANALOG_DATA_SIZE] ;				// Analog inputs after filtering
extern uint8_t CurrentVolume ;
//extern uint8_t PictureDrawn ;

uint32_t TimeCounter ;
uint32_t SecCounter ;
//uint8_t LastEvent ;
uint8_t ModelImageValid = 0 ;

SKYModelData TempModelData ;

//uint32_t FileSequenceNumber[MAX_MODELS + 1] ;

#include "../sticks.lbm"
#include "../font.lbm"
#define font_5x8_x20_x7f (font)

union t_fileHeader
{
	struct
	{
		uint32_t sequence ;
		uint8_t ver ;
		uint8_t type ;
		uint16_t size ;
	} ;
  char buf[8] ;
} ;

struct t_gmSave
{
	uint32_t csum ;
	EEGeneral general ;
	SKYModelData model ;
	unsigned char modelNames[MAX_MODELS+1][sizeof(g_model.name)+1] ;		// Allow for general
} ;

struct t_gmSave GmSave __CCM ;

extern uint16_t evalChkSum() ;

uint8_t PrototypeVersion = 0 ;

void CheckForPrototype()
{
	if (GPIOI->IDR & 0x0800)
	{
		PrototypeVersion = 0 ;
	}
	else
	{
		PrototypeVersion = 1 ;
	}
}

uint32_t isProdVersion()
{
	if ( PrototypeVersion )
	{
		return 0 ;
	}
	return 1 ;
}

//void enableBackupRam()
//{
//	PWR->CR |= PWR_CR_DBP ;
//	PWR->CSR |= PWR_CSR_BRE ;
//	while ( ( PWR->CSR & PWR_CSR_BRR) == 0 )
//	{
//		wdt_reset() ;
//	}
//	RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN ;
//}

void disableBackupRam()
{
	PWR->CR |= PWR_CR_DBP ;
	PWR->CSR |= PWR_CSR_BRE ;
	while ( ( PWR->CSR & PWR_CSR_BRR) == 0 )
	{
		wdt_reset() ;
	}
	RCC->AHB1ENR &= ~RCC_AHB1ENR_BKPSRAMEN ;
}

//uint32_t backupRamChecksum()
//{
//	uint32_t i ;
//	uint32_t csum ;
//	uint8_t *p = (uint8_t *) BKPSRAM_BASE ;
//	p += 4 ; // skip over checksum value
//	csum = 0 ;
//	for ( i = 4 ; i < 4096 ; i += 1 )
//	{
//		csum += *p++ ;
//	}
//	return csum ;
//}

//uint32_t checkBackupRam()
//{
//	struct t_backupRam *p = ( struct t_backupRam *) BKPSRAM_BASE ;
//	if ( backupRamChecksum() == p->checksum )
//	{
//		if ( p->checksum == 0 )
//		{
//			return 0 ;
//		}
//		return 1 ;
//	}
//	return 0 ;
//}

//void initBackupRam()
//{
//	struct t_backupRam *p = ( struct t_backupRam *) BKPSRAM_BASE ;
//	uint32_t i ;
//	p->generalSize = 0 ;
//	p->modelSize = 0 ;
//	p->generalSequence = 0 ;
//	p->modelSequence = 0 ;
//	p->modelNumber = 0 ;
//	for ( i = 0 ; i < 4096-18 ; i += 1 )
//	{
//		p->data[i] = 0 ;
//	}
//	p->checksum = backupRamChecksum() ;
//}


uint32_t ccRamChecksum()
{
	uint32_t i ;
	uint32_t csum ;
	uint8_t *p = (uint8_t *)&GmSave.general ;
	csum = 0 ;
	for ( i = 0 ; i < sizeof(g_eeGeneral) ; i += 1 )
	{
		csum += *p++ ;
	}
	p = (uint8_t *)&GmSave.model ;
	for ( i = 0 ; i < sizeof(g_model) ; i += 1 )
	{
		csum += *p++ ;
	}
	p = (uint8_t *)&GmSave.modelNames ;
	for ( i = 0 ; i < sizeof(GmSave.modelNames) ; i += 1 )
	{
		csum += *p++ ;
	}
	return csum ;
}

uint32_t checkCCRam()
{
	if ( ccRamChecksum() == GmSave.csum )
	{
		if ( GmSave.csum == 0 )
		{
			return 0 ;
		}
		return 1 ;
	}
	return 0 ;
}

void writeGeneralToCCRam()
{
	uint8_t *p = (uint8_t *)&GmSave.general ;
	uint32_t i ;
	uint8_t *q ;
	q = (uint8_t *) &g_eeGeneral ;
	for ( i = 0 ; i < sizeof(g_eeGeneral) ; i += 1 )
	{
		*p++ = *q++ ;
	}
	GmSave.csum = ccRamChecksum() ;
}

void writeModelToCCRam( uint8_t index )
{
	uint8_t *p = (uint8_t *)&GmSave.model ;
	uint32_t i ;
	uint8_t *q ;
	q = (uint8_t *) &g_model ;
	for ( i = 0 ; i < sizeof(g_model) ; i += 1 )
	{
		*p++ = *q++ ;
	}
	GmSave.csum = ccRamChecksum() ;
}

void writeNamesToCCRam()
{
	uint8_t *p = (uint8_t *)&GmSave.modelNames ;
	uint32_t i ;
	uint8_t *q ;
	q = (uint8_t *) &ModelNames ;
	for ( i = 0 ; i < sizeof(GmSave.modelNames) ; i += 1 )
	{
		*p++ = *q++ ;
	}
	GmSave.csum = ccRamChecksum() ;
}


uint32_t readGeneralFromCCRam()
{
	uint32_t i ;
	uint8_t *q ;

	if ( !checkCCRam() )
	{
		return 0 ;
	}

//  memset(&g_eeGeneral, 0, sizeof(EEGeneral));
	uint8_t *p = (uint8_t *)&GmSave.general ;
	q = (uint8_t *) &g_eeGeneral ;
	for ( i = 0 ; i < sizeof(g_eeGeneral) ; i += 1 )
	{
		*q++ = *p++ ;
	}
	return 1 ;	
}

uint32_t readModelFromCCRam()
{
	uint32_t i ;
	uint8_t *q ;

	if ( !checkCCRam() )
	{
		return 0 ;
	}
	
//  memset(&g_model, 0, sizeof(g_model));
	q = (uint8_t *) &g_model ;
	uint8_t *p = (uint8_t *)&GmSave.model ;
	for ( i = 0 ; i < sizeof(g_model) ; i += 1 )
	{
		*q++ = *p++ ;
	}
	return 1 ;	
}

uint32_t readNamesFromCCRam()
{
	if ( !checkCCRam() )
	{
		return 0 ;
	}
	uint8_t *p = (uint8_t *)&GmSave.modelNames ;
	uint32_t i ;
	uint8_t *q ;
	q = (uint8_t *) &ModelNames ;
	for ( i = 0 ; i < sizeof(GmSave.modelNames) ; i += 1 )
	{
		*q++ = *p++ ;
	}
	return 1 ;	
}


//void writeGeneralToBackupRam()
//{
//	struct t_backupRam *p = ( struct t_backupRam *) BKPSRAM_BASE ;
//	uint32_t i ;
//	uint8_t *q ;
//	q = (uint8_t *) &g_eeGeneral ;
//	for ( i = 0 ; i < sizeof(g_eeGeneral) ; i += 1 )
//	{
//		p->data[i] = *q++ ;
//	}
//	p->generalSize = sizeof(g_eeGeneral) ;
//	p->checksum = backupRamChecksum() ;
//}

//void writeModelToBackupRam( uint8_t index )
//{
//	struct t_backupRam *p = ( struct t_backupRam *) BKPSRAM_BASE ;
//	uint32_t i ;
//	uint8_t *q ;
//	uint8_t *r ;
//	q = (uint8_t *) &g_model ;
//	r = p->data ;
//	r += (4096-18) - sizeof(g_model) ;
//	for ( i = 0 ; i < sizeof(g_model) ; i += 1 )
//	{
//		*r++ = *q++ ;
//	}
//	p->modelNumber = index ;
//	p->modelSize = sizeof(g_model) ;
//	p->checksum = backupRamChecksum() ;
//}

//uint32_t readGeneralFromBackupRam()
//{
//	struct t_backupRam *p = ( struct t_backupRam *) BKPSRAM_BASE ;
//	uint32_t i ;
//	uint32_t j ;
//	uint8_t *q ;
	
////		initBackupRam() ;		
////		return 0 ;
//	if ( !checkBackupRam() )
//	{
//		initBackupRam() ;		
//		return 0 ;
//	}
	
//	j = p->generalSize ;
//	if ( j > sizeof(g_eeGeneral) )
//	{
//		j = sizeof(g_eeGeneral) ;
//	}
//  memset(&g_eeGeneral, 0, sizeof(EEGeneral));
//	q = (uint8_t *) &g_eeGeneral ;
//	for ( i = 0 ; i < j ; i += 1 )
//	{
//		*q++ = p->data[i] ;
//	}
//	return 1 ;	
//}

//uint32_t readModelFromBackupRam( uint8_t index )
//{
//	struct t_backupRam *p = ( struct t_backupRam *) BKPSRAM_BASE ;
//	uint32_t i ;
//	uint32_t j ;
//	uint8_t *q ;
//	uint8_t *r ;
	
//	if ( !checkBackupRam() )
//	{
//		return 0 ;
//	}
//	if ( p->modelNumber != index )
//	{
//		return 0 ;
//	}
	 
//	j = p->modelSize ;
//	if ( j > sizeof(g_model) )
//	{
//		j = sizeof(g_model) ;
//	}
//  memset(&g_model, 0, sizeof(g_model));
//	q = (uint8_t *) &g_model ;
//	r = p->data ;
//	r += (4096-18) - sizeof(g_model) ;
//	for ( i = 0 ; i < j ; i += 1 )
//	{
//		*q++ = *r++ ;
//	}
//	if ( g_model.modelVersion > 10 )
//	{
//		g_model.modelVersion = 4 ;
//	}
//	return 1 ;	
//}


void lcd_outhex8(uint16_t x,uint16_t y,uint32_t val)
{
	lcd_outhex4( x, y, val>>16 ) ;
	lcd_outhex4( x+FW*4, y, val ) ;
}

//#define IMAGE_BUFFER	(SDRAM_BANK_ADDR + 1024*1024)

uint16_t Image_Buffer[44000] __SDRAM ;
uint16_t Image_width ;
uint16_t Image_height ;
uint16_t XImage_width ;
uint16_t XImage_height ;


//struct t_filelists
//{
//	uint16_t VoiceUserIndex ;
//	uint16_t VoiceUserCount ;
//	uint16_t VoiceSystemIndex ;
//	uint16_t VoiceSystemCount ;
//	uint16_t VoiceNamesIndex ;
//	uint16_t VoiceNamesCount ;
//	uint16_t MusicIndex ;
//	uint16_t MusicCount ;
//	uint32_t Flags ;

//	TCHAR Filenames[1300][50] ;
//	uint32_t FileSize[1300] ;
//} FileLists __SDRAM ;

//// Flag bits
//#define FlVoiceUserValid		0x0001
//#define FlVoiceSystemValid	0x0002
//#define FlVoiceNamesValid		0x0004
//#define FlMusicValid				0x0008

//void initFileList()
//{
//	FileLists.VoiceUserIndex = 0 ;
//	FileLists.VoiceUserCount = 0 ;
//	FileLists.VoiceSystemIndex = 0 ;
//	FileLists.VoiceSystemCount = 0 ;
//	FileLists.VoiceNamesIndex = 0 ;
//	FileLists.VoiceNamesCount = 0 ;
//	FileLists.MusicIndex = 0 ;
//	FileLists.MusicCount = 0 ;
//	FileLists.Flags = 0 ;
//}

//// States of FileListLoader
//#define	FLL_START			0
//#define	FLL_VUSER1		1
//#define	FLL_VUSER2		2
//#define	FLL_VUSER3		3
//#define	FLL_VSYS1			4
//#define	FLL_VSYS2			5
//#define	FLL_VSYS3			6
//#define	FLL_VNAMES1		7
//#define	FLL_VNAMES2		8
//#define	FLL_VNAMES3		9
//#define	FLL_MUSIC1		10
//#define	FLL_MUSIC2		11
//#define	FLL_MUSIC3		12
//#define	FLL_DONE			13

//uint16_t FllState ;

//void fileListLoader()
//{
//	switch ( FllState )
//	{
//		case FLL_START :
//			initFileList() ;
//			FllState = FLL_VUSER1 ;
//		break ;
//		case FLL_VUSER1	:
//		break ;
//		case FLL_VUSER2	:
//		break ;
//		case FLL_VUSER3	:
//		break ;
//		case FLL_VSYS1 :
//		break ;
//		case FLL_VSYS2 :
//		break ;
//		case FLL_VSYS3 :
//		break ;
//		case FLL_VNAMES1 :
//		break ;
//		case FLL_VNAMES2 :
//		break ;
//		case FLL_VNAMES3 :
//		break ;
//		case FLL_MUSIC1 :
//		case FLL_MUSIC2 :
//		break ;
//		case FLL_MUSIC3 :
//		break ;
//		case FLL_DONE :
//		break ;
//	}
//}





//uint16_t Mimage[8196] ;
uint16_t *SdramImage = Image_Buffer ;
uint8_t ImageRow[576] ;
TCHAR ImageFilename[60] ;
uint8_t LoadImageResult = 0xFF ;
uint8_t TNAME[VOICE_NAME_SIZE+4] ;

//uint16_t LoadImageCount ;

uint32_t fourBytesToLong( uint8_t *p )
{
	uint32_t x ;

	x = *p++ ;
	x |= *p++ << 8 ;
	x |= *p++ << 16 ;
	x |= *p++ << 24 ;
	return x ;
}

uint32_t loadModelImage()
{
  FIL imageFile ;
	FRESULT result ;
  UINT nread ;
	TCHAR *ptr ;
	uint32_t row ;
	uint32_t col ;
	uint32_t offset ;
	uint32_t numRows ;
	uint32_t numCols ;
	uint8_t *p ;
	uint16_t *dest ;
	uint16_t value ;
	uint8_t name[VOICE_NAME_SIZE+4] ;
//  uint8_t palette[2];
//  uint8_t temp[64];
	ModelImageValid = 0 ;
//	LoadImageCount += 1 ;
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

//	result = f_read(&imageFile, (uint8_t *)Mimage, 16384, &nread) ;
	result = f_read(&imageFile, ImageRow, 54, &nread) ;
	// bytes 18-21 = #columns
	// bytes 22-25 = #rows
// openTx = 192 x 114 ?
	numCols = fourBytesToLong( &ImageRow[18] ) ;
	numRows = fourBytesToLong( &ImageRow[22] ) ;
	offset = fourBytesToLong( &ImageRow[10] ) ;
	
	XImage_width = numCols ;
	XImage_height = numRows ;

	if ( ( numCols < 128 ) || ( numCols > 192 ) )
	{
		LoadImageResult = 3 ;
 	 	return 1 ;	// Error
	} 
	if ( ( numRows < 64 ) || ( numRows > 114 ) )
	{
		LoadImageResult = 3 ;
 	 	return 1 ;	// Error
	} 
	
	if ( offset > 54 )
	{
		// skip to start of image
		result = f_read(&imageFile, ImageRow, offset-54, &nread) ;
	}
		 
	uint16_t xrow = 0 ;
#ifdef INVERT_DISPLAY
	for ( row = 1 ; row <= numRows ; row += 1 )
#else
	for ( row = numRows ; row ; row -= 1 )
#endif
	{
		result = f_read(&imageFile, ImageRow, numCols*3, &nread) ;
		if ( nread != numCols*3 )
		{
			LoadImageResult = 2 ;
  	 	return 1 ;	// Error
		}
#ifdef INVERT_DISPLAY
		dest = SdramImage + numCols * (row) - 1 ;
#else
		dest = SdramImage + numCols * (row-1) ;
#endif
		p = ImageRow ;
		for ( col = 0 ; col < numCols ; col += 1 )
		{
			// b,g,r
			value = (*p++ & 0x00F8) >> 3 ;
			value |= (*p++ & 0x00FC) << 3 ;
			value |= (*p++ & 0x00F8) << 8 ;
#ifdef INVERT_DISPLAY
			*dest-- = value ;
#else
			*dest++ = value ;
#endif
		}
		xrow += 1 ;
		if ( ( row & 0x07 ) == 0 )
		{
			CoTickDelay(1) ;					// 2mS, let voice run
		}
	}
	f_close(&imageFile) ;
	Image_width = numCols ;
	Image_height = numRows ;

	ModelImageValid = 1 ;
	LoadImageResult = 0 ;
//	PictureDrawn = 0 ;
	return 0 ;
}


//uint32_t loadModelImageBin()
//{
//  FIL imageFile ;
//	FRESULT result ;
//  UINT nread ;
//	TCHAR *ptr ;
//	uint8_t name[VOICE_NAME_SIZE+4] ;
////  uint8_t palette[2];
////  uint8_t temp[64];
//	ModelImageValid = 0 ;
//  memmove( name, g_model.modelImageName, VOICE_NAME_SIZE+2 ) ;
//	for ( uint8_t i=VOICE_NAME_SIZE+1 ; i ; i -= 1)
//	{
//		if ( name[i] == ' ' )
//		{
//			name[i] = '\0' ;
//		}
//		else
//		{
//			break ;
//		}
//	}
//  memmove( TNAME, name, VOICE_NAME_SIZE+2 ) ;
//  name[VOICE_NAME_SIZE+2] = '\0' ;
//  TNAME[VOICE_NAME_SIZE+2] = '\0' ;
//	ptr = (TCHAR *)cpystr( ( uint8_t*)ImageFilename, ( uint8_t*)"\\IMAGES\\" ) ;
//	ptr = (TCHAR *)cpystr( (uint8_t *)ptr, name ) ;
//	cpystr( ( uint8_t*)ptr, ( uint8_t*)".bin" ) ;

//	result = f_open( &imageFile, ImageFilename, FA_READ) ;
//  if (result != FR_OK)
//	{
//		LoadImageResult = 1 ;
//   	return 1 ;	// Error
//  }

//	result = f_read(&imageFile, (uint8_t *)Mimage, 16384, &nread) ;
//	f_close(&imageFile) ;

//	if ( nread != 16384 )
//	{
//		LoadImageResult = 2 ;
//   	return 1 ;	// Error
//	}
//	ModelImageValid = 1 ;
//	LoadImageResult = 0 ;
//	return 0 ;
//}

extern unsigned char ModelNames[MAX_MODELS+1][sizeof(g_model.name)+1] ;		// Allow for general

void setModelAFilename( uint8_t *fname, uint8_t id )
{
	uint8_t *p ;
//	p = cpystr( fname, (uint8_t *)"RADIO/model" ) ;
	p = cpystr( fname, (uint8_t *) RADIO_PATH "/model" ) ;
	*p++ = '0'+(id+1)/10 ;
	*p++ = '0'+(id+1)%10 ;
	*p++ = 'A' ;
	cpystr( p, (uint8_t *)".bin" ) ;
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

void ee32_delete_model( uint8_t id )
{
	uint8_t fname[30] ;
	uint8_t buffer[sizeof(g_model.name)+1] ;
  memset( buffer, ' ', sizeof(g_model.name) ) ;
	ee32_update_name( id + 1, buffer ) ;
	setModelAFilename( fname, id ) ;
	f_unlink( (TCHAR *)fname ) ;
}

bool eeModelExists(uint8_t id)
{
	if ( ModelNames[id+1][0] != '\0' && strncmp( (const char *)ModelNames[id+1], "          ", sizeof(g_model.name) ) )
	{
    return 1 ;
	}
	else
	{
		return 0 ;
	}
}

void ee32SwapModels(uint8_t id1, uint8_t id2)
{
	uint32_t m1Valid = 0 ;
	uint32_t m2Valid = 0 ;
//	uint32_t i ;
	uint8_t fname1[30] ;
	uint8_t fname2[30] ;
	uint8_t fnameTemp[30] ;
	// id values are the +1 type
	
	if ( eeModelExists(id1-1) )
	{
		m1Valid = 1 ;
	}
	if ( eeModelExists(id2-1) )
	{
		m2Valid = 1 ;
	}

	setModelAFilename( fname1, id1-1 ) ;
	setModelAFilename( fname2, id2-1 ) ;
	if ( m1Valid == 0 )
	{ // Copying blank entry
		if ( m2Valid == 0 )
		{ // To blank entry
			return ;	// Nothing to do
		}
		// No id1
		f_rename ( (TCHAR *)fname2, (TCHAR *)fname1 ) ;
	}
	else
	{
		if ( m2Valid == 0 )
		{ // To blank entry
			f_rename ( (TCHAR *)fname1, (TCHAR *)fname2 ) ;
		}
		else
		{
			setModelAFilename( fnameTemp, 98 ) ;
			f_rename ( (TCHAR *)fname1, (TCHAR *)fnameTemp ) ;
			f_rename ( (TCHAR *)fname2, (TCHAR *)fname1 ) ;
			f_rename ( (TCHAR *)fnameTemp, (TCHAR *)fname2 ) ;
		}
	}

  unsigned char tmp[sizeof(g_model.name)];
  memcpy(tmp, ModelNames[id1], sizeof(g_model.name));
  memcpy(ModelNames[id1], ModelNames[id2], sizeof(g_model.name));
  memcpy(ModelNames[id2], tmp, sizeof(g_model.name));

//	struct t_backupRam *p = ( struct t_backupRam *) BKPSRAM_BASE ;
//	i = p->modelNumber ;
//	if ( i == id1 )
//	{
//		p->modelNumber = id2 ;
//	}
//	else
//	{
//		if ( i == id2 )
//		{
//			p->modelNumber = id1 ;
//		}
//	}
}

static const uint8_t base64digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;

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


// write XML file
FRESULT writeXMLfile( FIL *archiveFile, uint8_t *data, uint32_t size, UINT *totalWritten, uint8_t *name )
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
  result = f_write( archiveFile, (BYTE *)name, sizeof(g_model.name), &written) ;
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

uint8_t Xfilename[50] ;

const char *ee32RestoreModel( uint8_t modelIndex, char *filename )
{
//  return "NOT DONE" ;
	uint8_t *bptr ;
  FIL archiveFile ;
  UINT nread ;
	FRESULT result ;
	int32_t answer ;
	uint8_t fname[50] ;
	
	bptr = cpystr( fname, (uint8_t *)"/MODELS/" ) ;
	cpystr( bptr, (uint8_t *)filename ) ;

	result = f_open( &archiveFile, (TCHAR *)filename, FA_READ) ;
  if (result != FR_OK)
	{
   	return "OPEN ERROR" ;
  }

	memset(( uint8_t *)&TempModelData, 0, sizeof(g_model));
	
	answer = readXMLfile( &archiveFile,  ( uint8_t *)&TempModelData, sizeof(TempModelData), &nread ) ;
	
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
  
	setModelAFilename( (uint8_t *)fname, modelIndex-1 ) ;
	uint32_t res	;
	res = xwriteFile( (char *)fname, (uint8_t *)&TempModelData, sizeof(g_model), EE_FILE_TYPE_MODEL ) ;
  if ( res == 0 )
	{
		uint8_t *p ;
		p = cpystr( Xfilename, (uint8_t *)fname ) ;
		cpystr( p, (uint8_t *)" ERROR" ) ;
		return (char *)Xfilename ;
//		return "ERROR" ;
	}
  memcpy(ModelNames[modelIndex], TempModelData.name, sizeof(g_model.name));
  return "MODEL RESTORED" ;
}



const char *ee32BackupModel( uint8_t modelIndex )
{
	FRESULT result ;
  DIR archiveFolder ;
  FIL archiveFile ;
  UINT written ;
	uint8_t filename[50] ;
	uint32_t res ;

	setModelAFilename( filename, modelIndex-1 ) ;

	memset(( uint8_t *)&TempModelData, 0, sizeof(g_model));
	res = xloadFile( (char *)filename, (uint8_t *)&TempModelData, sizeof(g_model)) ;
	if ( res == 0 )
	{
		return "FAILED" ;
	}
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

	result = writeXMLfile( &archiveFile, (uint8_t *)&TempModelData, sizeof(g_model), &written, ModelNames[modelIndex] ) ;
  
	f_close(&archiveFile) ;
  if (result != FR_OK ) //	|| written != size)
	{
    return "WRITE ERROR" ;
  }

  return "MODEL SAVED" ;
}

const char *openBackupEeprom()
{
  return "NOT DONE" ;
}

const char *processBackupEeprom( uint16_t blockNo )
{
  return "NOT DONE" ;
}

void closeBackupEeprom()
{
}

const char *processRestoreEeprom( uint16_t blockNo )
{
  return "NOT DONE" ;
}

const char *openRestoreEeprom( char *filename )
{
  return "NOT DONE" ;
}


//void pausePulses()
//{
	
//}

//void resumePulses()
//{
	
//}


extern void lcdRefresh(void) ;
void refreshDisplay()
{
	lcdRefresh() ;
}

extern int32_t Rotary_diff ;
extern "C" void pwrOff(void) ;
void soft_power_off()
{
	pwrOff() ;
}

#define POWER_STATE_OFF				0
#define POWER_STATE_START			1
#define POWER_STATE_RUNNING		2
#define POWER_STATE_STOPPING	3
#define POWER_STATE_STOPPED		4

uint8_t PowerState = POWER_STATE_OFF ;

uint32_t check_soft_power()
{
	uint32_t switchValue ;
	
	switchValue = GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) == Bit_RESET ;
	switch ( PowerState )
	{
		case POWER_STATE_OFF :
		default :
			PowerState = POWER_STATE_START ;
   		return POWER_ON ;
		break ;
			
		case POWER_STATE_START :
			if ( !switchValue )
			{
				PowerState = POWER_STATE_RUNNING ;
			}
   		return POWER_ON ;
		break ;

		case POWER_STATE_RUNNING :
			if ( switchValue )
			{
				PowerState = POWER_STATE_STOPPING ;
   			return POWER_X9E_STOP ;
			}
   		return POWER_ON ;
		break ;

		case POWER_STATE_STOPPING :
			if ( !switchValue )
			{
				PowerState = POWER_STATE_STOPPED ;
 				return POWER_OFF ;
			}
 			return POWER_X9E_STOP ;
		break ;

		case POWER_STATE_STOPPED :
 			return POWER_OFF ;
		break ;
	}
}


// Pass in an open file, the header at 0 and at 4K will be checked
// to see which is the most recent, the offset to the header of the most recent
// is returned, with the corresponding data size if required
// and the sequence number if required
uint32_t get_current_block_number( FIL *pfile, uint16_t *p_size, uint32_t *p_seq )
{
	union t_fileHeader header ;
  uint32_t sequence_no = 0 ;
  uint16_t size = 0 ;
  UINT read ;
	uint32_t offset = 0 ;
	FRESULT result ;

  if (f_size( pfile ) >= 8)
	{
		read = 0 ;
	  result = f_read( pfile, (uint8_t *)header.buf, 8, &read ) ;
  	if (result == FR_OK && read == 8)
		{
			sequence_no = header.sequence ;
			size = header.size ;
			// offset already set to 0
  	}
		f_lseek( pfile, 4096 ) ;
		read = 0 ;
	  result = f_read( pfile, (uint8_t *)header.buf, 8, &read ) ;
  	if (result == FR_OK && read == 8)
		{
			if ( header.sequence > sequence_no )
			{
				sequence_no = header.sequence ;
				size = header.size ;
				offset = 4096 ;				
			}
  	}
		f_lseek( pfile, 8192 ) ;
		read = 0 ;
	  result = f_read( pfile, (uint8_t *)header.buf, 8, &read ) ;
  	if (result == FR_OK && read == 8)
		{
			if ( header.sequence > sequence_no )
			{
				sequence_no = header.sequence ;
				size = header.size ;
				offset = 8192 ;				
			}
  	}
	}
  
  if ( p_size )
	{
		*p_size = size ;
	}
  if ( p_seq )
	{
		*p_seq = sequence_no ;
	}
  
	return offset ;
}

uint32_t xwriteFile( const char * filename, const uint8_t * data, uint16_t size, uint8_t type)
{
  FIL file ;
  UINT written ;
	uint32_t offset ;
	uint32_t sequenceNo ;
	uint32_t retValue = 0 ;
	union t_fileHeader header ;

  FRESULT result = f_open(&file, filename, FA_OPEN_ALWAYS | FA_WRITE | FA_READ ) ;
  if (result == FR_OK)
	{
		if ( f_size( &file) < 8 )	// A new file
		{
			offset = 0 ;
			sequenceNo = 0x00000045 ;
		}
		else
		{
			offset = get_current_block_number( &file, 0, &sequenceNo ) ;
			sequenceNo += 1 ;
			offset = offset == 0 ? 8192 : 0 ;	// Other copy
		}
		header.sequence = sequenceNo ;
		header.ver = 0 ;
		header.type = type ;
		header.size = size ;
		f_lseek( &file, offset ) ;
	  result = f_write(&file, ( BYTE *)header.buf, 8, &written ) ;
	  result = f_write(&file, data, size, &written) ;
  	if (result == FR_OK && written == size )
		{
			retValue = 1 ;
		}
	}
  f_close(&file) ;
	return retValue ;
}

//"RADIO/radio.bin"
const char *writeGeneral()
{
	uint32_t result	;
	result = xwriteFile( RADIO_PATH "/radio.bin", (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral), EE_FILE_TYPE_GENERAL ) ;
	result = xwriteFile( RADIO_PATH "/radiosky.bin", (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral), EE_FILE_TYPE_GENERAL ) ;
  return result ? 0 : "ERROR" ;
}

//uint8_t TempBuffer[4096] ;

//void tempWriteBackupRam( FIL *file )
//{
//	uint8_t *p = ( uint8_t *) BKPSRAM_BASE ;
//	uint32_t i ;
//	uint8_t *q = TempBuffer ;
//  UINT written ;
//	for ( i = 0 ; i < 4096 ; i += 1 )
//	{
//		*q++ = *p++ ;
//	}
//  f_write( file, (BYTE *)TempBuffer, 4096, &written) ;
//}

const char *writeNames()
{
  FIL file;
  UINT written;

//	f_unlink ( RADIO_PATH "/Mnamesbk.bin" ) ;
//	f_rename ( RADIO_PATH "/Mnames.bin", RADIO_PATH "/Mnamesbk.bin" ) ;
	 
  FRESULT result = f_open(&file, RADIO_PATH "/Mnames.bin", FA_OPEN_ALWAYS | FA_WRITE) ;
  if (result != FR_OK)
	{
		return "SD card error" ;
//    return SDCARD_ERROR(result);
  }
  result = f_write(&file, (BYTE *)ModelNames, sizeof(ModelNames), &written) ;
  if (result != FR_OK || written != sizeof(ModelNames) )
	{
    f_close(&file) ;
		return "SD card error" ;
//    return SDCARD_ERROR(result);
  }

//	tempWriteBackupRam( &file ) ;
  f_close(&file) ;
  return NULL ;
}

const char *readNames()
{
  FIL file;
  UINT read ;
	
  FRESULT result = f_open(&file, RADIO_PATH "/Mnames.bin", FA_OPEN_EXISTING | FA_READ) ;
  if (result != FR_OK)
	{
		return "SD card error" ;
//    return SDCARD_ERROR(result);
  }
  result = f_read(&file, (BYTE *)ModelNames, sizeof(ModelNames), &read ) ;
  if (result != FR_OK || read != sizeof(ModelNames) )
	{
    f_close(&file) ;
		return "SD card error" ;
//    return SDCARD_ERROR(result);
  }

  f_close(&file) ;
  return NULL;
}

const char *writeModel(uint32_t id)
{
	uint8_t fname[30] ;
	
	setModelAFilename( fname, id ) ;
	uint32_t result	;
	result = xwriteFile( (char *)fname, (uint8_t *)&g_model, sizeof(g_model), EE_FILE_TYPE_MODEL );
  return result ? 0 : "ERROR" ;
}


uint32_t xloadFile(const char * filename, uint8_t * data, uint16_t maxsize)
{
  FIL file ;
  UINT read ;
	uint32_t offset ;
	uint32_t sequenceNo ;
	uint16_t size ;
	uint32_t retValue = 0 ;
  FRESULT result = f_open(&file, filename, FA_OPEN_EXISTING | FA_READ);
  if (result == FR_OK)
	{
		offset = get_current_block_number( &file, &size, &sequenceNo ) ;
  	size = min<uint16_t>(maxsize, size ) ;
		if ( size )
		{
			f_lseek( &file, offset + 8 ) ; // Past header
			
  		result = f_read( &file, data, size, &read ) ;
		  if (result == FR_OK && read == size)
			{
				retValue = 1 ;
			}
		}
	  f_close(&file);
	}
  return retValue ;
}


void setRadioFilename( uint8_t *fname, uint8_t which )
{
	uint8_t *q ;
	q = cpystr( fname, (uint8_t *) RADIO_PATH "/radio" ) ;
	if ( which )
	{
		*q++ = which ;
	}
	cpystr( q, (uint8_t *)".bin" ) ;
}

const char *readGeneral()
{
	uint32_t result	;
  memset(&g_eeGeneral, 0, sizeof(EEGeneral));
	result = xloadFile( RADIO_PATH "/radio.bin", (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral) ) ;
  
//#if defined(PCBX10)
//  g_eeGeneral.x9dcalibMid = 0x0400 ;
//  g_eeGeneral.x9dcalibSpanNeg = 0x180 ;
//  g_eeGeneral.x9dcalibSpanPos = 0x180 ;
//#endif	
	
	if ( result == 0 )
	{
		return "ERROR" ;
	}
  uint16_t sum=0;
  sum = evalChkSum() ;
  if ( g_eeGeneral.chkSum == sum )
	{
		return 0 ;
	}
	// Possibly openTx file!!
	f_rename ( (TCHAR *)RADIO_PATH "/radio.bin", (TCHAR *)RADIO_PATH "/radioopn.bin" ) ;
  memset(&g_eeGeneral, 0, sizeof(EEGeneral));
	result = xloadFile( RADIO_PATH "/radiosky.bin", (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral) ) ;
	
	if ( result == 0 )
	{
		return "ERROR" ;
	}
  sum = evalChkSum() ;
  if ( g_eeGeneral.chkSum == sum )
	{
		return 0 ;
	}
	return "ERROR" ;
}

const char * readModel(uint32_t id)
{
	uint8_t fname[30] ;
	setModelAFilename( fname, id ) ;

	uint32_t result	;
  memset(&g_model, 0, sizeof(g_model));
	result = xloadFile( (char *)fname, (uint8_t *)&g_model, sizeof(g_model)) ;
	if ( g_model.modelVersion > 10 )
	{
		g_model.modelVersion = 4 ;
	}
	ModelImageValid = 0 ;
	checkXyCurve() ;
  return result ? 0 : "ERROR" ;
}

void eeLoadModel(uint8_t id)
{
	if ( readModel(id) )
	{
		modelDefault(id) ;
		STORE_MODELVARS ;
	}
	else
	{
  	memcpy(ModelNames[id+1], g_model.name, sizeof(g_model.name) ) ;
	}
}

//void init_eeprom()
//{
//	uint32_t i ;
//	for ( i = 0 ; i < MAX_MODELS + 1 ; i += 1 )
//	{
//		FileSequenceNumber = 0 ;	// Unknown
//	}
//}

bool ee32CopyModel(uint8_t dst, uint8_t src)
{
	uint8_t fname[30] ;
	setModelAFilename( fname, src-1 ) ;
	wdt_reset() ;
	xloadFile( (char *)fname, (uint8_t *)&TempModelData, sizeof(g_model) ) ;
	setModelAFilename( fname, dst-1 ) ;
	wdt_reset() ;
	xwriteFile( (char *)fname, (uint8_t *)&TempModelData, sizeof(g_model), EE_FILE_TYPE_MODEL ) ;
		 
	memcpy( ModelNames[dst], ModelNames[src], sizeof(g_model.name)) ;

  return true;
}

void eeSaveAll()
{
	wdt_reset() ;
//	writeGeneralToBackupRam() ;
//	writeModelToBackupRam( g_eeGeneral.currModel + 1 ) ;
//	wdt_reset() ;
//	CoTickDelay(1) ;					// 2mS for now
	writeGeneral() ;	
	wdt_reset() ;
	CoTickDelay(1) ;					// 2mS for now
	writeModel(g_eeGeneral.currModel) ;	
	wdt_reset() ;
	CoTickDelay(1) ;					// 2mS for now
	writeNames() ;
	wdt_reset() ;
	CoTickDelay(1) ;					// 2mS for now
}

uint8_t ExternalSet ;


