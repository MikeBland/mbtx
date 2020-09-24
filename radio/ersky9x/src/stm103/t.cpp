/****************************************************************************
*  Copyright (c) 2019 by Michael Blandford. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*  3. Neither the name of the author nor the names of its contributors may
*     be used to endorse or promote products derived from this software
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
*  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*  SUCH DAMAGE.
*
****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <stm32f10x.h>
#include "ersky9x.h"
#include "stm103/logicio103.h"
#include "stm103/hal.h"
#include "lcd.h"
#include "sound.h"
#include "drivers.h"
#include "myeeprom.h"
#include "stringidx.h"
#include "stm32_sdio_sd.h"
#include "diskio.h"
#include "audio.h"
#include "timers.h"
#include "file.h"
#include "mixer.h"
#ifndef SIMU
#include "CoOS.h"
#endif
#include "frsky.h"
#include "menus.h"

void ee32_update_name( uint32_t id, uint8_t *source ) ;
uint32_t xloadFile(const char * filename, uint8_t * data, uint16_t maxsize) ;
uint32_t xwriteFile( const char * filename, const uint8_t * data, uint16_t size) ;

#define RADIO_PATH           "/RADIO"   // no trailing slash = important

struct fileControl
{
	uint32_t nameCount ;
	uint32_t vpos ;
	uint32_t hpos ;
	uint16_t index ;
	uint8_t ext[4] ;
} ;

SKYModelData TempModelData ;

extern union t_sharedMemory SharedMemory ;
extern struct fileControl PlayFileControl ;

extern uint16_t g_timeMixer ;
extern uint8_t CurrentVolume ;
extern MenuFuncP g_menuStack[];
extern uint8_t g_menuStackPtr ;

uint8_t ModelImageValid = 0 ;

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

uint16_t TrainerPpmStream[20] =
{
	2000,
	2200,
	2400,
	2600,
	2800,
	3000,
	3200,
	3400,
	45000-21600,
	0,0,0,0,0,0,0,0,0,0,0
} ;

extern uint16_t *TrainerPulsePtr ;

uint8_t MaintenanceRunning = 0 ;

void maintenance_receive_packet( uint8_t *packet, uint32_t check )
{
	
}

uint16_t TrainCapture ;

extern "C" void EXTI0_IRQHandler()
{
	uint16_t capture ;
  uint16_t val ;
  static uint16_t lastCapt ;
	
	EXTI->PR = 1 ;
	
	capture = TrainCapture ;

  val = (uint16_t)(capture - lastCapt) / 2 ;
  lastCapt = capture ;

  if ((val>4000) && (val < 19000)) // Prioritize reset pulse. (Needed when less than 8 incoming pulses)
	{
  	ppmInState = 1; // triggered
	}
  else
  {
  	if(ppmInState && (ppmInState<=16))
		{
  	  if((val>800) && (val<2200))
			{
				ppmInValid = 100 ;
//  		    g_ppmIns[ppmInState++ - 1] = (int16_t)(val - 1500)*(g_eeGeneral.PPM_Multiplier+10)/10; //+-500 != 512, but close enough.
  		  g_ppmIns[ppmInState++ - 1] = (int16_t)(val - 1500) ; //+-500 != 512, but close enough.

		  }else{
  		  ppmInState=0; // not triggered
  	  }
  	}
  }
}

extern volatile int32_t Rotary_position ;
extern volatile int32_t Rotary_count ;

extern "C" void EXTI15_10_IRQHandler()
{
	uint16_t capture ;
	if ( EXTI->PR & TRAINER_Pin )
	{
		capture = TIM7->CNT ;	// Capture time (2MHz)

		EXTI->PR = TRAINER_Pin ;
		TrainCapture = capture ;
		EXTI->SWIER |= 1 ;
	}
	if ( EXTI->PR & 0x6000 )
	{
		capture = GPIOENCODER->IDR ;	// Read Rotary encoder ( PC14, PC13 )
		capture >>= 13 ;
		capture &= 0x03 ;			// pick out the two bits
		EXTI->PR = 0x6000 ;
		if ( capture != ( Rotary_position & 0x03 ) )
		{
			if ( ( Rotary_position & 0x01 ) ^ ( ( capture & 0x02) >> 1 ) )
			{
				Rotary_count += 1 ;
			}
			else
			{
				Rotary_count -= 1 ;
			}
			Rotary_position &= ~0x03 ;
			Rotary_position |= capture ;
		}
	}
}

void stop_trainer_ppm()
{
	TIM2->CR1 = 0 ;
	NVIC_DisableIRQ(TIM2_IRQn) ;
}

void init_trainer_capture(uint32_t mode)
{
	stop_trainer_ppm() ;
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN ;	// Enable portC clock
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN ;
	EXTI->RTSR |= TRAINER_Pin ;	// Rising Edge
	EXTI->IMR |= TRAINER_Pin | 1 ;
	AFIO->EXTICR[2] |= 0x2000 ;		// PC11
	configure_pins( TRAINER_Pin, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | TRAINER_PORT ) ;
	
	NVIC_SetPriority( EXTI15_10_IRQn, 1 ) ; // Not quite highest priority interrupt
	NVIC_EnableIRQ( EXTI15_10_IRQn) ;
	NVIC_SetPriority( EXTI0_IRQn, 8 ) ; // Low
	NVIC_EnableIRQ( EXTI0_IRQn) ;
}

void stop_trainer_capture()
{
//	NVIC_DisableIRQ( EXTI15_10_IRQn) ;
	EXTI->IMR &= ~(TRAINER_Pin | 1) ;
	NVIC_DisableIRQ( EXTI0_IRQn) ;
}

#define PPM_CENTER 1500*2

void setupTrainerPulses()
{
  uint32_t i ;
	uint32_t total ;
	uint32_t pulse ;
	uint16_t *ptr ;
	uint32_t p = (g_model.ppmNCH + 4) ;
	if ( p > 16 )
	{
		p = 8 ;
	}
	int16_t PPM_range = g_model.extendedLimits ? 640*2 : 512*2;   //range of 0.7..1.7msec

	ptr = TrainerPpmStream ;

	total = 22500u*2; //Minimum Framelen=22.5 ms
  total += (int16_t(g_model.ppmFrameLength))*1000;

	p += g_model.startChannel ;
	for ( i = g_model.startChannel ; i < p ; i += 1 )
	{
  	pulse = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
		
		total -= pulse ;
		*ptr++ = pulse ;
	}
	*ptr++ = total ;
	*ptr = 0 ;
	TIM2->CCR1 = total - 1500 ;		// Update time
	TIM2->CCR2 = (g_model.ppmDelay*50+300)*2 ;
}

void init_trainer_ppm()
{
	stop_trainer_capture() ;	
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN ;			// Enable clock
	setupTrainerPulses() ;
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN ;	// Enable portC clock
	configure_pins( TRAINER_Pin, PIN_OP2 | PIN_GP_PP | TRAINER_PORT ) ;
	
	TrainerPulsePtr = TrainerPpmStream ;
  TIM2->ARR = *TrainerPulsePtr++ ;
  TIM2->PSC = 72000000  / 2000000 - 1 ;               // 0.5uS
	
  TIM2->CCMR1 = TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 ;		// PWM mode 1
	TIM2->CCR2 = 600 ;		// 300 uS pulse
 	TIM2->EGR = 1 ;
	TIM2->DIER |= TIM_DIER_CC2IE ;
	TIM2->DIER |= TIM_DIER_UIE ;
	TIM2->CR1 = TIM_CR1_CEN ;
  NVIC_SetPriority(TIM2_IRQn, 2);
	NVIC_EnableIRQ(TIM2_IRQn) ;
}

extern "C" void TIM2_IRQHandler()
{
  // PPM out update interrupt
  if ( (TIM2->DIER & TIM_DIER_UIE) && ( TIM2->SR & TIM_SR_UIF ) )
	{
  	if(g_model.trainPulsePol)
		{
			GPIOC->BRR = TRAINER_Pin ;
		}
		else
		{
			GPIOC->BSRR = TRAINER_Pin ;
		}
		TIM2->SR = TIMER2_5SR_MASK & ~TIM_SR_UIF ;                               // Clear flag
  	TIM2->ARR = *TrainerPulsePtr++ ;
  	if ( *TrainerPulsePtr == 0 )
		{
  	  TIM2->SR = TIMER2_5SR_MASK & ~TIM_SR_CC1IF ;                     // Clear this flag
  	  TIM2->DIER |= TIM_DIER_CC1IE ;  // Enable this interrupt
  		TIM2->DIER &= ~TIM_DIER_UIE ;   // stop this interrupt

  	}
  }
  
	if ( ( TIM2->DIER & TIM_DIER_CC2IE ) && ( TIM2->SR & TIM_SR_CC2IF ) )
	{
  	if(g_model.trainPulsePol)
		{
			GPIOC->BSRR = TRAINER_Pin ;
		}
		else
		{
			GPIOC->BRR = TRAINER_Pin ;
		}
  	TIM2->SR = TIMER2_5SR_MASK & ~TIM_SR_CC2IF ;                             // Clear flag
	}
  
	if ( ( TIM2->DIER & TIM_DIER_CC1IE ) && ( TIM2->SR & TIM_SR_CC1IF ) )
	{
  	// compare interrupt
  	TIM2->DIER &= ~TIM_DIER_CC1IE ;         // stop this interrupt
  	TIM2->SR = TIMER2_5SR_MASK & ~TIM_SR_CC1IF ;                             // Clear flag

  	setupTrainerPulses() ;

		TrainerPulsePtr = TrainerPpmStream ;
  	TIM2->SR = TIMER2_5SR_MASK & ~TIM_SR_UIF ;         // Clear this flag
  	TIM2->DIER |= TIM_DIER_UIE ;                       // Enable this interrupt
  }
}


void pausePulses()
{
}

void resumePulses()
{

}

uint32_t FileSize[8] ;

void com1_Configure( uint32_t a, uint32_t b, uint32_t c )
{
	
}

void com2_Configure( uint32_t a, uint32_t b, uint32_t c )
{
	
}


void ledInit()
{
	configure_pins( G_LED_Pin, PIN_OP2 | PIN_GP_PP | G_LED_PORT | PIN_LOW ) ;
	configure_pins( R_LED_Pin, PIN_OP2 | PIN_GP_PP | R_LED_PORT | PIN_LOW ) ;
	configure_pins( B_LED_Pin, PIN_OP2 | PIN_GP_PP | B_LED_PORT | PIN_LOW ) ;
}

void ledRed()
{
	R_LED_GPIO->BSRR = R_LED_Pin ;
}

void ledBlue()
{
	B_LED_GPIO->BSRR = B_LED_Pin ;
}

void ledGreen()
{
	G_LED_GPIO->BSRR = G_LED_Pin ;
}

void ledOff()
{
	R_LED_GPIO->BRR = R_LED_Pin ;
	G_LED_GPIO->BRR = G_LED_Pin ;
	B_LED_GPIO->BRR = B_LED_Pin ;
}



//void com2Configure()
//{
//	
//}

void init_software_com1( uint32_t a, uint32_t b, uint32_t c )
{
	
}

void mavlinkReceive( uint8_t data )
{
	
}

#define VOICE_FILE_TYPE_NAME	0
#define VOICE_FILE_TYPE_USER	1
#define VOICE_FILE_TYPE_MUSIC	2
#define VOICE_FILE_TYPE_SYSTEM	3
extern uint8_t VoiceFileType ;
FRESULT readBinDir( DIR *dj, FILINFO *fno, struct fileControl *fc ) ;
DIR Djp ;

uint32_t fillNames( uint32_t index, struct fileControl *fc )
{
	uint32_t i ;

	i = 0 ;

	FRESULT fr ;
	SharedMemory.FileList.Finfo.lfname = SharedMemory.FileList.Filenames[0] ;
	SharedMemory.FileList.Finfo.lfsize = 48 ;
	WatchdogTimeout = 300 ;		// 3 seconds
	DIR *pDj = &SharedMemory.FileList.Dj ;	
	if ( VoiceFileType == VOICE_FILE_TYPE_MUSIC )
	{
#ifdef WHERE_TRACK
	notePosition('m') ;
#endif
		pDj = &Djp ;
	}
	fr = f_readdir ( pDj, 0 ) ;					// rewind
	fr = f_readdir ( pDj, &SharedMemory.FileList.Finfo ) ;		// Skip .
	fr = f_readdir ( pDj, &SharedMemory.FileList.Finfo ) ;		// Skip ..
	i = 0 ;
#ifdef WHERE_TRACK
	notePosition('n') ;
#endif
	while ( i <= index )
	{
		WatchdogTimeout = 300 ;		// 3 seconds
		fr = readBinDir( pDj, &SharedMemory.FileList.Finfo, fc ) ;		// First entry
		FileSize[0] = SharedMemory.FileList.Finfo.fsize ;
		i += 1 ;
		if ( fr != FR_OK || SharedMemory.FileList.Finfo.fname[0] == 0 )
		{
			return 0 ;
		}
	}
#ifdef WHERE_TRACK
	notePosition('o') ;
#endif
	for ( i = 1 ; i < 7 ; i += 1 )
	{
		WatchdogTimeout = 300 ;		// 3 seconds
#ifdef WHERE_TRACK
	notePosition('p') ;
#endif
		SharedMemory.FileList.Finfo.lfname = SharedMemory.FileList.Filenames[i] ;
		fr = readBinDir( pDj, &SharedMemory.FileList.Finfo, fc ) ;		// First entry
		FileSize[i] = SharedMemory.FileList.Finfo.fsize ;
		if ( fr != FR_OK || SharedMemory.FileList.Finfo.fname[0] == 0 )
		{
			break ;
		}
	}
#ifdef WHERE_TRACK
	notePosition('q') ;
#endif
	return i ;
}


FRESULT readBinDir( DIR *dj, FILINFO *fno, struct fileControl *fc )
{
	FRESULT fr ;
	uint32_t loop ;

	do
	{
		loop = 0 ;
		fr = f_readdir ( dj, fno ) ;		// First entry

		if ( fr != FR_OK || fno->fname[0] == 0 )
		{
			break ;
		}
		if ( *fno->lfname == 0 )
		{
			cpystr( (uint8_t *)fno->lfname, (uint8_t *)fno->fname ) ;		// Copy 8.3 name
		}
		if ( fc->ext[0] )
		{
			int32_t len = strlen(fno->lfname) - 4 ;
			if ( fc->ext[3] )
			{
				len -= 1 ;			
			}
			if ( len < 0 )
			{
				loop = 1 ;
			}
			if ( fno->lfname[len] != '.' )
			{
				loop = 1 ;
			}
			if ( ( fno->lfname[len+1] & ~0x20 ) != fc->ext[0] )
			{
				loop = 1 ;
			}
			if ( ( fno->lfname[len+2] & ~0x20 ) != fc->ext[1] )
			{
				loop = 1 ;
			}
			if ( ( fno->lfname[len+3] & ~0x20 ) != fc->ext[2] )
			{
				loop = 1 ;
			}
			if ( fc->ext[3] )
			{
				if ( ( fno->lfname[len+4] & ~0x20 ) != fc->ext[3] )
				{
					loop = 1 ;
				}
			}
		}
		else // looking for a Directory
		{
			if ( ( fno->fattrib & AM_DIR ) == 0 )
			{
				loop = 1 ;
			}
		}	
	} while ( loop ) ;
	return fr ;
}


extern uint16_t PlayListCount ;
extern char PlayListNames[PLAYLIST_COUNT][MUSIC_NAME_LENGTH+2] ;
void copyFileName( char *dest, char *source, uint32_t size ) ;


uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext )
{
	uint32_t i ;
	FRESULT fr ;
	SharedMemory.FileList.Finfo.lfsize = 19 ;
	char filename[20] ;
	WatchdogTimeout = 300 ;		// 3 seconds

	fr = f_chdir( dir ) ;
	if ( fr == FR_OK )
	{
		fc->index = 0 ;
		fr = f_opendir( &Djp, (TCHAR *) "." ) ;
		if ( fr == FR_OK )
		{
			fc->ext[0] = *ext++ ;
			fc->ext[1] = *ext++ ;
			fc->ext[2] = *ext++ ;
			fc->ext[3] = *ext ;
			fc->index = 0 ;

			fr = f_readdir ( &Djp, 0 ) ;					// rewind
			fr = f_readdir ( &Djp, &SharedMemory.FileList.Finfo ) ;		// Skip .
			fr = f_readdir ( &Djp, &SharedMemory.FileList.Finfo ) ;		// Skip ..
			for ( i = 0 ; i < PLAYLIST_COUNT ; i += 1 )
			{
				WatchdogTimeout = 300 ;		// 3 seconds
				SharedMemory.FileList.Finfo.lfname = filename ;
				fr = readBinDir( &Djp, &SharedMemory.FileList.Finfo, fc ) ;		// First entry
				if ( fr != FR_OK || SharedMemory.FileList.Finfo.fname[0] == 0 )
				{
					break ;
				}
				copyFileName( PlayListNames[i], filename, MUSIC_NAME_LENGTH ) ;

			}
			PlayListCount = i ;
			return i ;
		}
	}
	return 0 ;
}


extern volatile int32_t Rotary_count ;
extern int32_t Rotary_diff ;
extern int8_t Pre_Rotary_diff[] ;
extern int32_t LastRotaryValue ;

void getADC_single() ;
void getADC_osmp() ;
void getADC_filt() ;



#define DISPLAY_CHAR_WIDTH	21
uint16_t LastFileMoveTime ;


uint32_t fileList(uint8_t event, struct fileControl *fc )
{
	uint32_t limit ;
	uint32_t result = 0 ;
  uint8_t maxhsize ;
	uint32_t i ;
			 
	limit = 6 ;
	if ( fc->nameCount < limit )
	{
		limit = fc->nameCount ;						
	}
	maxhsize = 0 ;
	for ( i = 0 ; i < limit ; i += 1 )
	{
		uint32_t x ;
		uint32_t len ;
		len = x = strlen( SharedMemory.FileList.Filenames[i] ) ;
		if ( x > maxhsize )
		{
			maxhsize = x ;							
		}
		if ( x > DISPLAY_CHAR_WIDTH )
		{
			if ( ( fc->hpos + DISPLAY_CHAR_WIDTH ) > x )
			{
				x = x - DISPLAY_CHAR_WIDTH ;
			}
			else
			{
				x = fc->hpos ;
			}
			len = DISPLAY_CHAR_WIDTH ;
		}
		else
		{
			x = 0 ;
		}
#ifdef WHERE_TRACK
	notePosition('i') ;
#endif
		lcd_putsn_P( 0, 16+FH*i, &SharedMemory.FileList.Filenames[i][x], len ) ;
	}

#if !defined(PCBTARANIS) || defined(REV9E)
	if ( event == 0 )
	{
extern int32_t Rotary_diff ;
		if ( Rotary_diff > 0 )
		{
			event = EVT_KEY_FIRST(KEY_DOWN) ;
		}
		else if ( Rotary_diff < 0 )
		{
			event = EVT_KEY_FIRST(KEY_UP) ;
		}
		Rotary_diff = 0 ;
	}
#endif

	if ( ( event == EVT_KEY_REPT(KEY_DOWN) ) || event == EVT_KEY_FIRST(KEY_DOWN) )
	{
		uint16_t now ;
		now = get_tmr10ms() ;
	  if((uint16_t)( now-LastFileMoveTime) > 4) // 50mS
		{
			LastFileMoveTime = now ;
			if ( fc->vpos < limit-1 )
			{
				fc->vpos += 1 ;
			}
			else
			{
				if ( fc->nameCount > limit )
				{
					fc->index += 1 ;
//		CoSchedLock() ;
#ifdef WHERE_TRACK
	notePosition('j') ;
#endif
					fc->nameCount = fillNames( fc->index, fc ) ;
//  	CoSchedUnlock() ;
				}
			}
		}
	}
	if ( ( event == EVT_KEY_REPT(KEY_UP)) || ( event == EVT_KEY_FIRST(KEY_UP) ) )
	{
		uint16_t now ;
		now = get_tmr10ms() ;
	  if((uint16_t)( now-LastFileMoveTime) > 4) // 50mS
		{
			LastFileMoveTime = now ;
			if ( fc->vpos > 0 )
			{
				fc->vpos -= 1 ;
			}
			else
			{
				if ( fc->index )
				{
					fc->index -= 1 ;
//		CoSchedLock() ;
#ifdef WHERE_TRACK
	notePosition('k') ;
#endif
					fc->nameCount = fillNames( fc->index, fc ) ;
//  	CoSchedUnlock() ;
				}
			}
		}
	}
//	if ( ( event == EVT_KEY_REPT(MKEY_RIGHT)) || ( event == EVT_KEY_FIRST(MKEY_RIGHT) ) )
//	{
//		if ( fc->hpos + DISPLAY_CHAR_WIDTH < maxhsize )	fc->hpos += 1 ;
//	}
//	if ( ( event == EVT_KEY_REPT(MKEY_LEFT)) || ( event == EVT_KEY_FIRST(MKEY_LEFT) ) )
//	{
//		if ( fc->hpos )	fc->hpos -= 1 ;
//	}
	if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
	{
		// Select file to flash
		killEvents(event);
		result = 1 ;
	}
	if ( event == EVT_KEY_LONG(BTN_RE) )
//	if ( checkForExitEncoderLong( event ) )
	{
		// Select file to flash
		result = 2 ;
	}
	if ( event == EVT_KEY_BREAK(KEY_MENU) )
	{
		// Tag file
		result = 3 ;
	}
#if defined(PCBTARANIS)
	lcd_filled_rect( 0, 2*FH+FH*fc->vpos, DISPLAY_CHAR_WIDTH*FW, 8, 0xFF, 0 ) ;
#else
	lcd_char_inverse( 0, 2*FH+FH*fc->vpos, DISPLAY_CHAR_WIDTH*FW, 0 ) ;
#endif
#ifdef WHERE_TRACK
	notePosition('l') ;
#endif
	return result ;
}

void dsmBindResponse( uint8_t mode, int8_t channels )
{
}




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
	res = xwriteFile( (char *)fname, (uint8_t *)&TempModelData, sizeof(g_model));
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

uint32_t xwriteFile( const char * filename, const uint8_t * data, uint16_t size)
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
			offset = offset == 0 ? 4096 : 0 ;	// Other copy
		}
		header.sequence = sequenceNo ;
		header.ver = 0 ;
		header.type = 'M' ;
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
	result = xwriteFile( RADIO_PATH "/radio.bin", (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral) ) ;
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
	result = xwriteFile( (char *)fname, (uint8_t *)&g_model, sizeof(g_model));
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
  
#if defined(PCBX10)
  g_eeGeneral.x9dcalibMid = 0x0400 ;
  g_eeGeneral.x9dcalibSpanNeg = 0x180 ;
  g_eeGeneral.x9dcalibSpanPos = 0x180 ;
#endif	
	return result ? 0 : "ERROR" ;
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
	xwriteFile( (char *)fname, (uint8_t *)&TempModelData, sizeof(g_model) ) ;
		 
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

