
/*
 * Current Author - Mike Blandford
 * Original Author - Rob Thomson & Bertrand Songis
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
#include <stdlib.h>
#include <string.h>
#ifdef PCBSKY
#include "AT91SAM3S4.h"
#endif
#if defined(PCBX9D) || defined(PCB9XT)
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/hal.h"
#endif

#if defined(PCBX12D) || defined(PCBX10)
#include "X12D/stm32f4xx.h"
#include "X12D/stm32f4xx_gpio.h"
#include "X12D/hal.h"
#endif

#if !defined(SIMU)
// Mike I think this include is not needed (already present in diskio.h)
//#include "core_cm3.h"
#endif
#include "ersky9x.h"
#include "myeeprom.h"
#include "audio.h"
#include "sound.h"
#include "diskio.h"
#include "ff.h"

#ifndef SIMU
#include "CoOS.h"
#endif

#ifdef PCB9XT
#define SOFTWARE_VOLUME	1
#endif

uint8_t MusicPlaying ;
uint8_t MusicInterrupted ;
uint8_t MusicPrevNext ;
uint8_t AudioActive ;

extern uint8_t CurrentVolume ;
extern uint8_t Activated ;
extern uint8_t MuteTimer ;
extern uint8_t AudioVoiceCountUnderruns ;

extern uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext ) ;

struct t_voice Voice ;
struct toneQentry ToneQueue[AUDIO_QUEUE_LENGTH] ;
uint8_t ToneQueueRidx ;
uint8_t ToneQueueWidx ;
uint8_t SdAccessRequest ;
uint8_t VoiceFlushing ;

/*static*/ uint16_t toneCount ;
/*static*/ uint16_t toneTimer ;
/*static*/ uint8_t toneTimeLeft ;
/*static*/ int8_t toneFreqIncr ;
/*static*/ uint8_t toneFrequency ;
/*static*/ uint8_t tonePause ;
/*static*/ uint8_t toneRepeat ;
/*static*/ uint8_t toneOrPause ;
/*static*/ uint8_t toneActive ;
/*static*/ uint8_t toneVarioVolume ;


//uint16_t Sine16k[32] =
//{
////	2048,2268,2471,2605,2648,2605,2471,2268,
////	2048,1826,1623,1490,1448,1490,1623,1826
//	2048,2165,2278,2381,2472,2547,2602,2636,
//	2648,2636,2602,2547,2472,2381,2278,2165,
//	2048,1931,1818,1715,1624,1549,1494,1460,
//	1448,1460,1494,1549,1624,1715,1818,1931
//} ;

int16_t Sine16kInt[32] =
{
	  0, 117, 230, 333, 424, 499, 554, 588,
	600, 588, 554, 499, 424, 333, 230, 117,
	   0,-117,-230,-333,-424,-499,-554,-588,
	-600,-588,-554,-499,-424,-333,-230,-117
} ;



void nextToneData( void ) ;

bool ToneFreeSlots()
{
	uint8_t temp ;
	temp = ToneQueueWidx ;
	temp += AUDIO_QUEUE_LENGTH ;
	temp -= ToneQueueRidx ;
	temp %= AUDIO_QUEUE_LENGTH ;
	temp = AUDIO_QUEUE_LENGTH - temp ;
	return temp >= AUDIO_QUEUE_FREESLOTS ;
}


audioQueue::audioQueue()
{
  aqinit();
}

// TODO should not be needed
void audioQueue::aqinit()
{
  //make sure haptic off by default
//  HAPTIC_OFF;

  toneTimeLeft = 0;
  tonePause = 0;

  t_queueRidx = 0;
  t_queueWidx = 0;

//  toneHaptic = 0;
//  hapticTick = 0;
  hapticMinRun = 0;
  buzzTimeLeft = 0 ;
  buzzPause = 0 ;
  t_hapticQueueRidx = 0;
  t_hapticQueueWidx = 0;

}

bool audioQueue::busy()
{
  return (toneTimeLeft > 0);
}


bool audioQueue::freeslots()
{
	uint8_t temp ;
	temp = t_queueWidx ;
	temp += AUDIO_QUEUE_LENGTH ;
	temp -= t_queueRidx ;
	temp %= AUDIO_QUEUE_LENGTH ;
	temp = AUDIO_QUEUE_LENGTH - temp ;
	return temp >= AUDIO_QUEUE_FREESLOTS ;
}

#ifdef PCBX9LITE
static const uint8_t HapticTable[] = {
	0, 40, 55, 70, 85, 100
} ;
#endif 


// heartbeat is responsibile for issueing the audio tones and general square waves
// it is essentially the life of the class.
// it is called every 10ms
void audioQueue::heartbeat()
{
  //haptic switch off happens in separate loop
//  if(hapticMinRun == 0){
//	hapticOff();	
//  } else {
//	hapticMinRun--;	
//  }	
	
  if ( buzzTimeLeft )
	{
		buzzTimeLeft -= 1 ; // time gets counted down
#if defined(REVPLUS) || defined(REV9E)
 		hapticOn(((g_eeGeneral.hapticStrength+5)) * 10); 
#else			
 #ifdef PCBX9LITE
 		if ( g_eeGeneral.hapticStrength > 5 )
		{
			g_eeGeneral.hapticStrength = 5 ;
		}
 		hapticOn( HapticTable[g_eeGeneral.hapticStrength] ) ; 
 #else			
 		hapticOn((g_eeGeneral.hapticStrength *  2 ) * 10); 
 #endif 
#endif 
// 		hapticMinRun = HAPTIC_SPINUP;
	}
	else
	{
		hapticOff() ;
    if (buzzPause > 0)
		{
      buzzPause -= 1 ;
    }
    else if (t_hapticQueueRidx != t_hapticQueueWidx)
		{
      buzzTimeLeft = queueHapticLength[t_hapticQueueRidx] ;
			uint8_t hapticSpinup = g_eeGeneral.hapticMinRun + 20 ;
			if ( buzzTimeLeft < hapticSpinup )
			{
				buzzTimeLeft = hapticSpinup ;
			}
      buzzPause = queueHapticPause[t_hapticQueueRidx] ;
      if (!queueHapticRepeat[t_hapticQueueRidx]--)
			{
        t_hapticQueueRidx = (t_hapticQueueRidx + 1) & (HAPTIC_QUEUE_LENGTH-1);
      }
		}
	}
 
//  if (toneTimeLeft )
//	{
		
//		if ( queueTone( toneFreq * 61 / 2, toneTimeLeft * 10, toneFreqIncr * 61 / 2, 0 ) )
//		{
//    			toneTimeLeft = 0 ; //time gets counted down
//		}

//		//this works - but really needs a delay added in.
//		// reason is because it takes time for the motor to spin up
//		// need to take this into account when the tone sent is really short!
//		// initial thoughts are a seconds queue to process haptic that gets
//		// fired from here.  end result is haptic events run for mix of 2 seconds?
		
////		if (toneHaptic)
////		{
////#ifdef REVPLUS		
////	    		hapticOn(((g_eeGeneral.hapticStrength+5)) * 10); 
////#else			
////	    		hapticOn((g_eeGeneral.hapticStrength *  2 ) * 10); 
////#endif 
////	    		hapticMinRun = HAPTIC_SPINUP;
////		}    
//  }
//  else
//	{
//    if ( tonePause )
//		{
//			if ( queueTone( 0, tonePause * 10, 0, 0 ) )
//			{
//    		tonePause = 0 ; //time gets counted down
//			}
// 		}
//		else
//		{  
//      if (t_queueRidx != t_queueWidx)
//			{
//        toneFreq = queueToneFreq[t_queueRidx];
//        toneTimeLeft = queueToneLength[t_queueRidx];
//        toneFreqIncr = queueToneFreqIncr[t_queueRidx];
//        tonePause = queueTonePause[t_queueRidx];
////        toneHaptic = queueToneHaptic[t_queueRidx];
////        hapticTick = 0;
//        if (!queueToneRepeat[t_queueRidx]--)
//				{
//          t_queueRidx = (t_queueRidx + 1) % AUDIO_QUEUE_LENGTH;
//        }
//      }
//    }
//  }
}

uint8_t audioQueue::getToneLength(uint8_t tLen)
{
  uint8_t result = tLen; // default
  if (g_eeGeneral.beeperVal == 2) {
    result /= 3;
  }
  else if (g_eeGeneral.beeperVal == 3)
	{
    result /= 2;
  }
  else if (g_eeGeneral.beeperVal == 5)
	{
    //long
    result *= 2;
  }
  else if (g_eeGeneral.beeperVal == 6)
	{
    //xlong
    result *= 3;
  }
  return result;
}

void audioQueue::playNow(uint8_t tFreq, uint8_t tLen, uint8_t tPause,
    uint8_t tRepeat, uint8_t tHaptic, int8_t tFreqIncr)
{
	
//	if(!freeslots()){
//			return;
//	}
	
  if (g_eeGeneral.beeperVal)
	{
		
		queueTone( 1, tFreq ? tFreq + g_eeGeneral.speakerPitch + BEEP_OFFSET : 0, tFreqIncr, tLen, tPause, tRepeat ) ;	// Now
    
//		toneFreq = (tFreq ? tFreq + g_eeGeneral.speakerPitch + BEEP_OFFSET : 0); // add pitch compensator
//    toneTimeLeft = getToneLength(tLen);
//    tonePause = tPause;
		if ( tHaptic && !MuteTimer )
		{
  		buzzTimeLeft = toneTimeLeft ;
			uint8_t hapticSpinup = g_eeGeneral.hapticMinRun + 20 ;
			if ( buzzTimeLeft < hapticSpinup )
			{
				buzzTimeLeft = hapticSpinup ;
			}
			buzzPause = tPause ;
		}
    
//		toneFreqIncr = tFreqIncr;
//    t_queueWidx = t_queueRidx;

//    if (tRepeat) {
//      playASAP(tFreq, tLen, tPause, tRepeat-1, tHaptic, tFreqIncr);
//    }
  }
}

void audioQueue::playASAP(uint8_t tFreq, uint8_t tLen, uint8_t tPause,
    uint8_t tRepeat, uint8_t tHaptic, int8_t tFreqIncr)
{
			
	if ( tHaptic && !MuteTimer )
	{
    uint8_t next_hqueueWidx = (t_hapticQueueWidx + 1) % HAPTIC_QUEUE_LENGTH ;
		if (next_hqueueWidx != t_hapticQueueRidx)
		{
			queueHapticLength[t_hapticQueueWidx] = getToneLength(tLen) ;
			queueHapticPause[t_hapticQueueWidx] = tPause ;
			queueHapticRepeat[t_hapticQueueWidx] = tRepeat ;
      t_hapticQueueWidx = next_hqueueWidx ;
		}
	}
	
  if (g_eeGeneral.beeperVal)
	{
		queueTone( 0, tFreq ? tFreq + g_eeGeneral.speakerPitch + BEEP_OFFSET : 0, tFreqIncr, tLen, tPause, tRepeat ) ;	// Now
	}
//	if(!freeslots()){
//			return;
//	}	
	
//  if (g_eeGeneral.beeperVal)
//	{
//    uint8_t next_queueWidx = (t_queueWidx + 1) % AUDIO_QUEUE_LENGTH;
//    if (next_queueWidx != t_queueRidx)
//		{
//      queueToneFreq[t_queueWidx] = (tFreq ? tFreq + g_eeGeneral.speakerPitch + BEEP_OFFSET : 0); // add pitch compensator
//      queueToneLength[t_queueWidx] = getToneLength(tLen);
//      queueTonePause[t_queueWidx] = tPause;
////      queueToneHaptic[t_queueWidx] = tHaptic;
//      queueToneRepeat[t_queueWidx] = tRepeat;
//      queueToneFreqIncr[t_queueWidx] = tFreqIncr;
//      t_queueWidx = next_queueWidx;
//    }
//  }
}

void audioQueue::event(uint8_t e, uint8_t f, uint8_t hapticOff) {

  uint8_t beepVal = g_eeGeneral.beeperVal;
	hapticOff = hapticOff ? 0 : 1 ;
//	if (t_queueRidx == t_queueWidx) {		
	  switch (e) {
		    case AU_WARNING1:
		      playNow(BEEP_DEFAULT_FREQ, 10, 1, 0, hapticOff);
		      break;
		    case AU_WARNING2:
		      playNow(BEEP_DEFAULT_FREQ, 20, 1, 0, hapticOff);
		      break;
		    case AU_WARNING3:
		      playNow(BEEP_DEFAULT_FREQ, 30, 1, 0, hapticOff);
		      break;
	      case AU_CHEEP:
	        playASAP(BEEP_DEFAULT_FREQ+30,10,2,2,hapticOff,2);
	        break;
	      case AU_RING:
	        playASAP(BEEP_DEFAULT_FREQ+25,5,2,10,hapticOff);
	        playASAP(BEEP_DEFAULT_FREQ+25,5,10,1,hapticOff);
	        playASAP(BEEP_DEFAULT_FREQ+25,5,2,10,hapticOff);
	        break;
	      case AU_SCIFI:
	        playASAP(80,10,3,2,0,-1);
	        playASAP(60,10,3,2,0,1);
	        playASAP(70,10,1,0,2*hapticOff);
	        break;
	      case AU_ROBOT:
	        playASAP(70,5,1,1,hapticOff);
	        playASAP(50,15,2,1,hapticOff);
	        playASAP(80,15,2,1,hapticOff);
	        break;
	      case AU_CHIRP:
	        playASAP(BEEP_DEFAULT_FREQ+40,5,1,2,hapticOff);
	        playASAP(BEEP_DEFAULT_FREQ+54,5,1,3,hapticOff);
	        break;
	      case AU_TADA:
	        playASAP(50,5,5);
	        playASAP(90,5,5);
	        playASAP(110,3,4,2);
	        break;
	      case AU_CRICKET:
	        playASAP(80,5,10,3,hapticOff);
	        playASAP(80,5,20,1,hapticOff);
	        playASAP(80,5,10,3,hapticOff);
	        break;
	      case AU_SIREN:
	        playASAP(10,20,5,2,hapticOff,1);
	        break;
	      case AU_ALARMC:
	        playASAP(50,4,10,2,hapticOff);
	        playASAP(70,8,20,1,hapticOff);
	        playASAP(50,8,10,2,hapticOff);
	        playASAP(70,4,20,1,hapticOff);
	        break;
	      case AU_RATATA:
	        playASAP(BEEP_DEFAULT_FREQ+50,5,10,10,hapticOff);
	        break;
	      case AU_TICK:
	        playASAP(BEEP_DEFAULT_FREQ+50,5,50,2,hapticOff);
	        break;
	      case AU_HAPTIC1:
	        playASAP(0,20,10,0,1);
	        break;
	      case AU_HAPTIC2:
	        playASAP(0,15,20,1,1);
	        break;
	      case AU_HAPTIC3:
	        playASAP(0,15,20,2,1);
	        break;
	      case AU_HAPTIC4:
	        playASAP(0,20,0,0,1);
	        break;
		    case AU_ERROR:
		      playNow(BEEP_DEFAULT_FREQ, 40, 1, 0, hapticOff);
		      break;
		    case AU_KEYPAD_UP:
		      if (beepVal != BEEP_NOKEYS) {
		        playNow(BEEP_KEY_UP_FREQ, getToneLength(10), 1);
		      }
		      break;
		    case AU_KEYPAD_DOWN:
		      if (beepVal != BEEP_NOKEYS) {
		        playNow(BEEP_KEY_DOWN_FREQ, getToneLength(10), 1);
		      }
		      break;
		    case AU_TRIM_MOVE:
		      playNow(f, 6, 1);
		      break;
		    case AU_TRIM_MIDDLE:
		      playNow(BEEP_DEFAULT_FREQ, getToneLength(20), 2, 0, hapticOff);
		      break;
		    case AU_MENUS:
		      if (beepVal != BEEP_NOKEYS) {
		        playNow(BEEP_DEFAULT_FREQ, getToneLength(10), 2, 0, 0);
		      }
		      break;
		    case AU_POT_STICK_MIDDLE:
		      playNow(BEEP_DEFAULT_FREQ + 50, 10, 1, 0, 0);
		      break;
		    case AU_MIX_WARNING_1:
		      playNow(BEEP_DEFAULT_FREQ + 50, 10, 1, 1, hapticOff);
		      break;
		    case AU_MIX_WARNING_2:
		      playNow(BEEP_DEFAULT_FREQ + 52, 10, 1, 2, hapticOff);
		      break;
		    case AU_MIX_WARNING_3:
		      playNow(BEEP_DEFAULT_FREQ + 54, 10, 1, 3, hapticOff);
		      break;
		    case AU_TIMER_30:
		      playNow(BEEP_DEFAULT_FREQ + 50, 15, 3, 3, hapticOff);
		      break;
		    case AU_TIMER_20:
		      playNow(BEEP_DEFAULT_FREQ + 50, 15, 3, 2, hapticOff);
		      break;
		    case AU_TIMER_10:
		      playNow(BEEP_DEFAULT_FREQ + 50, 15, 3, 1, hapticOff);
		      break;
		    case AU_TIMER_LT3:
		      playNow(BEEP_DEFAULT_FREQ, 20, 25, 1, hapticOff);
		      break;
		    case AU_INACTIVITY:
		      playNow(70, 10, 2,2);
		      break;
		    case AU_TX_BATTERY_LOW:
		        playASAP(60, 20, 3, 2, 0, 1);
		        playASAP(80, 20, 3, 2, 1, -1);
		      break;
				
				case AU_VARIO_UP :
		      playNow(BEEP_DEFAULT_FREQ + g_model.varioExtraData.baseFrequency + g_model.varioExtraData.offsetFrequency + 1 + f, 10, 0, 0x80, 0, 3 ) ;
		    break ;
		    
				case AU_VARIO_DOWN :
		      playNow(BEEP_DEFAULT_FREQ + g_model.varioExtraData.baseFrequency - g_model.varioExtraData.offsetFrequency - 1 - f, 10, 0, 0x80, 0, -3 ) ;
		    break ;

		    case AU_LONG_TONE :
		      playNow( 40, 200, 0 ) ;
		    break ;

					 
		    default:
		      break;
	  }
//	}  
}

void audioDefevent(uint8_t e)
{
//	if ( (g_eeGeneral.speakerMode & 1) == 0 )
//	{
//		buzzer_sound( 4 ) ;
//	}
//	else if ( ( g_eeGeneral.speakerMode & 1 )== 1 )
//	if ( ( g_eeGeneral.speakerMode & 1 )== 1 )
//	{
		audio.event(e, BEEP_DEFAULT_FREQ);
//		playTone( 2000, 60 ) ;		// 2KHz, 60mS
//	}
}

void audioNamedVoiceDefevent( uint8_t e, uint16_t v)
{
	if ( /*( g_eeGeneral.speakerMode & 2 ) && */Voice.VoiceLock == 0 )
	{
		putSystemVoice( v, 0 ) ;
	}
	else
	{
    audioDefevent( e ) ;
	}
}


void audioVoiceDefevent( uint8_t e, uint16_t v)
{
	if ( /*( g_eeGeneral.speakerMode & 2 ) && */Voice.VoiceLock == 0 )
	{
		putVoiceQueue( v ) ;
	}
	else
	{
    audioDefevent( e ) ;
	}
}

void putSysNumVoiceQueue( uint16_t value )
{
	putVoiceQueue( value | VLOC_NUMSYS ) ;
}

void voicePointName( uint8_t index )
{
	char name[10] ;
	char *p ;
	p = (char *)cpystr( (uint8_t *)name, (uint8_t *)"POINT_0" ) ;
	*(p-1) += index ;
	putNamedVoiceQueue( name, VLOC_SYSTEM + index + 6 ) ;
}

// index is 0 to 99
void voice0to99Name( uint16_t index )
{
	char name[10] ;
	div_t qr ;
	char *p = name ;

	*p++ = 'N' ;
	if ( index < 100 )
	{
		qr = div( index, 10 ) ;
		*p++ = qr.quot + '0' ;
		*p++ = qr.rem + '0' ;
		*p = '\0' ;
		putNamedVoiceQueue( name, VLOC_SYSTEM + index + 400 ) ;
	}
	else
	{
		if ( index < 1000 )
		{
			index /= 100 ;	// Gives 0 to 9 hundreds
			*p++ = index + '0' ;
			*p++ = '0' ;
			*p++ = '0' ;
			*p = '\0' ;
			putNamedVoiceQueue( name, VLOC_SYSTEM + index + 100 ) ;
		}
		else
		{
			index /= 1000 ;	// Gives 0 to 20 thousands
			qr = div( index, 10 ) ;
			if ( qr.quot )
			{
				*p++ = qr.quot + '0' ;
			}
			*p++ = qr.rem + '0' ;
			*p++ = '0' ;
			*p++ = '0' ;
			*p++ = '0' ;
			*p = '\0' ;
			putNamedVoiceQueue( name, VLOC_SYSTEM + index + 110 ) ;
		}
	}
}

// Announce a value using voice
void voice_numeric( int16_t value, uint8_t num_decimals, uint16_t units_index )
{
	uint8_t decimals = 0 ;
	uint8_t actualDecimals = 0 ;
	div_t qr ;
	uint32_t flag = 0 ;

//	if ( units_index > 127 )
//	{
//		void putSystemVoice( uint16_t sname, uint16_t value )
////		putSysNumVoiceQueue( units_index ) ;
//	}
	if ( value < 0 )
	{
		value = - value ;
		putSystemVoice( SV_MINUS, 0 ) ;
	}

	if ( num_decimals )
	{
		qr = div( value, num_decimals == 2 ? 100 : 10 ) ;
		actualDecimals = qr.rem ;
		value = qr.quot ;
	}

	qr = div( value, 100 ) ;
	if ( qr.quot )
	{
		decimals = qr.rem ;		// save in case no hundreds
		if ( qr.quot > 9 )		// Thousands
		{
			num_decimals = 0 ;
			flag = 1 ;
			qr = div( qr.quot, 10 ) ;
			if ( qr.quot < 21 )
			{
//				putVoiceQueue( qr.quot + 110 ) ;
//				putSysNumVoiceQueue( qr.quot + 110 ) ;
				voice0to99Name( qr.quot * 1000 ) ;
			}
			else
			{
//				putVoiceQueue( qr.quot + 400 ) ;
//				putSysNumVoiceQueue( qr.quot + 400 ) ;
				voice0to99Name( qr.quot ) ;
				putSystemVoice( SV_THOUSAND, V_THOUSAND ) ;
//				putVoiceQueue( V_THOUSAND | VLOC_NUMSYS ) ;
			}
			qr.quot = qr.rem ;			
		}
		if ( qr.quot )		// There are hundreds
		{
//			putVoiceQueue( qr.quot + 100 ) ;
//			putSysNumVoiceQueue( qr.quot + 100 ) ;
			voice0to99Name( qr.quot * 100 ) ;
//			putVoiceQueue( decimals + 400 ) ;
			if ( decimals )
			{
//				putSysNumVoiceQueue( decimals + 400 ) ;
				voice0to99Name( decimals ) ;
			}
			flag = 1 ;
		}
		else
		{
//			putVoiceQueue( decimals + 400 ) ;
//			putSysNumVoiceQueue( decimals + 400 ) ;
			if ( decimals )
			{
				voice0to99Name( decimals ) ;
			}
			flag = 1 ;
		}
		if ( ( flag == 0 ) && (qr.rem) )
		{
//			putVoiceQueue( qr.rem + 400 ) ;
//			putSysNumVoiceQueue( qr.rem + 400 ) ;
			voice0to99Name( qr.rem ) ;
		}
	}
	else
	{
//		putVoiceQueue( qr.rem + 400 ) ;
//		putSysNumVoiceQueue( qr.rem + 400 ) ;
		voice0to99Name( qr.rem ) ;
	}

	if ( num_decimals )
	{
		if ( num_decimals == 2 )
		{
			qr = div( actualDecimals, 10 ) ;
			voicePointName( qr.quot ) ;
//			putVoiceQueue( qr.quot + 6 ) ;		// Point x
//			putVoiceQueue( qr.rem + 400 ) ;
//			putSysNumVoiceQueue( qr.rem + 400 ) ;
			voice0to99Name( qr.rem ) ;
		}
		else
		{
			voicePointName( actualDecimals ) ;
//			putVoiceQueue( decimals + 6 ) ;		// Point x
		}
	}
		 
	if ( units_index )
	{
		putSystemVoice( units_index, 0 ) ;
	}
}

void putVoiceQueue( uint16_t value )
{
	struct t_voice *vptr ;
	vptr = &Voice ;
	
	if ( vptr->VoiceQueueCount < VOICE_Q_LENGTH )
	{
		vptr->VoiceQueue[vptr->VoiceQueueInIndex++] = value ;
		vptr->VoiceQueueInIndex &= ( VOICE_Q_LENGTH - 1 ) ;
		__disable_irq() ;
		vptr->VoiceQueueCount += 1 ;
		__enable_irq() ;
	}
}

const char SysVoiceNames[][VOICE_NAME_SIZE+1] =
{
	"ALERT",
	"Sw_Warn",
	"Thr_Warn",
	"WARNING",
	"ERROR",
	"FEET",
	"FOOT",
	"MINUS",
	"WELCOME",
	"LIMIT",
	"RPM",
	"FLT_BATT",
	"TX_VOLT",
	"CURRENT",
	"ALTITUDE",
	"POINT",
	"VOLTS",
	"VOLT",
	"MINUTES",
	"MINUTE",
	"PACKVOLT",
	"30SECOND",
	"20SECOND",
	"10SECOND",
	"PERCENT",
	"INACTV",
	"TXBATLOW",
	"DEGREES",
	"DEGREE",
	"RX_VOLT",
	"TEMPERAT",
	"AMPS",
	"AMP",
	"SECONDS",
	"SECOND",
	"DB",
	"METERS",
	"METER",
	"NO_TELEM",
	"RX_V_LOW",
	"TEMPWARN",
	"ALT_WARN",
	"WATT",
	"WATTS",
	"KNOT",
	"KNOTS",
	"MILLIAMP",
	"MILIAMPS",
	"MILAMP_H",
	"MLAMPS_H",
	"RSSI_LOW",
	"RSSICRIT",
	"RX_LOST",
	"CAP_WARN",
	"BT_LOST",
	"HUNDRED",
	"THOUSAND",
	"CST_WARN",
	"SHUTDOWN",
	"TRN_LOST",
	"KMS_HOUR"
} ;

void putSystemVoice( uint16_t sname, uint16_t value )
{
	const char *name ;

	name = SysVoiceNames[sname] ;
	putNamedVoiceQueue( name, VLOC_SYSTEM | value ) ;
}

void putUserVoice( char *name, uint16_t value )
{
	putNamedVoiceQueue( name, VLOC_USER + value ) ;
}

void putNamedVoiceQueue( const char *name, uint16_t value )
{
	struct t_voice *vptr ;
	vptr = &Voice ;
	
	if ( vptr->VoiceQueueCount < VOICE_Q_LENGTH )
	{
    memmove(vptr->NamedVoiceQueue[vptr->VoiceQueueInIndex], name, VOICE_NAME_SIZE ) ;
    vptr->NamedVoiceQueue[vptr->VoiceQueueInIndex][VOICE_NAME_SIZE] = '\0' ;
		vptr->VoiceQueue[vptr->VoiceQueueInIndex++] = value ; //0xE000 + (value & 0xFFF) ;		// Flag to say use name
		vptr->VoiceQueueInIndex &= ( VOICE_Q_LENGTH - 1 ) ;
		__disable_irq() ;
		vptr->VoiceQueueCount += 1 ;
		__enable_irq() ;
	}
}

void voiceSystemNameNumberAudio( uint16_t sname, uint16_t number, uint8_t audio )
{
	const char *name ;

#ifdef PCBSKY	
	if ( !CardIsPresent() )
#else
	if ( socket_is_empty() )
#endif
	{
    audioDefevent( audio ) ;
	}
	else
	{
		name = SysVoiceNames[sname] ;
		putNamedVoiceQueue( name, VLOC_SYSTEM + number ) ;
	}
}

uint8_t SaveVolume ;
TCHAR VoiceFilename[48] ;
uint8_t FileData[1024] ;
uint8_t BgFileData[1024] ;
FATFS g_FATFS ;
FIL Vfile ;
FIL BgFile ;
uint32_t SDlastError ;
uint32_t BgSizeLeft ;
extern uint8_t SectorsPerCluster ;

uint32_t BgSizePlayed ;
uint32_t BgTotalSize ;

UINT BgNread ;
uint8_t Bgindex ;
uint8_t BgLastBuffer ;
uint16_t BgFrequency ;
uint8_t PlayingFreq ;
int32_t ToneTime ;

struct toneQentry ToneEntry ;


// v_index Exxx - \\system\name
// v_index Dxxx - \\user\name
// v_index Cxxx - \\modelnames\name
// v_index 1xxx - \\system\(v_index & 0xFFF)

void buildFilename( uint32_t v_index, uint8_t *name )
{
	uint32_t x ;
	TCHAR *ptr ;
	uint8_t *dirName ;

	x = 0 ;
	ptr = (TCHAR *)cpystr( ( uint8_t*)VoiceFilename, ( uint8_t*)"\\voice\\" ) ;
	if (v_index & VLOC_MASK)
	{
		uint32_t index = v_index & VLOC_MASK ;
		dirName = ( uint8_t*)"system\\" ;	// 0xE000
		if ( index == VLOC_MNAMES )
		{
			dirName = ( uint8_t*)"modelNames\\" ;
		}
		else if ( index == VLOC_MUSIC )
		{
			ptr = &VoiceFilename[1] ;	// Discard 'voice\'
			dirName = ( uint8_t*)"music\\" ;
		}
		else if ( ( index == VLOC_USER ) || ( index == VLOC_NUMUSER ) )
		{
			dirName = ( uint8_t*)"user\\" ;
		}
		ptr = (TCHAR *)cpystr( (uint8_t *)ptr, dirName ) ;
		if ( ( index != VLOC_NUMSYS ) && ( index != VLOC_NUMUSER ) )
		{
			if ( index == VLOC_MUSIC )
			{
				ptr = (TCHAR *)cpystr( (uint8_t *)ptr, name ) ;
			}
			else
			{
				ptr = (TCHAR *)ncpystr( (uint8_t *)ptr, name, VOICE_NAME_SIZE ) ;
			}
			x = 1 ;
		}
	}
	if ( x == 0 )
	{
		v_index &= ~VLOC_MASK ;
		*ptr++ = '0' ;
		*(ptr + 2) = '0' + v_index % 10 ;
		x = v_index / 10 ;
		*(ptr + 1) = '0' + x % 10 ;
		x /= 10 ;
		*ptr = '0' + x % 10 ;
		ptr += 3 ;
	}
	while ( *(ptr-1) == ' ' )
	{
		ptr -= 1 ;
	}
	cpystr( ( uint8_t*)ptr, ( uint8_t*)".wav" ) ;
}

uint32_t lockOutVoice()
{
	uint8_t lockValue ;
	CoSchedLock() ;
	lockValue = Voice.VoiceLock ;
	if ( lockValue == 0 )
	{
		Voice.VoiceLock = 1 ;
	}
	CoSchedUnlock() ;
	return (lockValue == 0) ;
}

void unlockVoice()
{
	Voice.VoiceLock = 0 ;
}

#if defined(PCBX12D) || defined(PCBX10)
static const uint8_t SwVolume_scale[NUM_VOL_LEVELS] = 
{
	 0,  3,  5,   8,   12,  17,  24,  33,  45,  57,  70,  85,
	100, 118, 140, 163, 190, 207, 224, 239, 244, 248, 252, 255 	
} ;
#else
static const uint8_t SwVolume_scale[NUM_VOL_LEVELS] = 
{
	 0,  5,  10,   15,   30,  45,  60,  74,  84,  94,  104,  114,
	128, 164, 192, 210, 224, 234, 240, 244, 248, 251, 253, 255 	
} ;
#endif

static uint16_t swVolumeLevel()
{
	return SwVolume_scale[CurrentVolume] ;
}

uint32_t wavU16Convert( uint16_t *src, uint16_t *dest , uint32_t count, uint32_t addTone )
{
	uint16_t *saveDest = dest ;
	uint32_t returnValue = 1 ;
	uint32_t padSize ;
	if ( count > VOICE_BUFFER_SIZE )
	{
		count = VOICE_BUFFER_SIZE ;
	}
	padSize = VOICE_BUFFER_SIZE - count ;
#ifndef PCB9XT
	if ( ( g_eeGeneral.softwareVolume ) || addTone )
#endif
	{
		int32_t multiplier ;
		int32_t value ;
#ifndef PCB9XT
		if ( g_eeGeneral.softwareVolume )
		{
#endif
			multiplier = SwVolume_scale[CurrentVolume] ;
#ifndef PCB9XT
		}
		else
		{
			multiplier = 256 ;
		}
#endif
		if ( addTone )
		{
			if ( multiplier > 179 )
			{
				multiplier = 179 ;	// Limit to 70%
			}
		}
		while( count-- )
		{
			value = (int16_t) *src++ ;
			value *= multiplier ;
			value += 32768 * 256 ;
			*dest++ = value >> 12 ;
		}
	}
#ifndef PCB9XT
	else
	{
		while( count-- )
		{
			*dest++ = (uint16_t)( (int16_t )*src++ + 32768) >> 4 ;
		}
	}
#endif
	while( padSize-- )
	{
		*dest++ = 2048 ;
	}

	if ( addTone )
	{
		returnValue = toneFill( saveDest, TONE_ADD ) ;
	}
	return returnValue ;
}


//void wavU16ConvertPlusTone( uint16_t *src, uint16_t *dest, uint32_t count )
//{
//	int32_t toneMultiplier ;
//	int32_t voiceMultiplier ;
//	int32_t value ;
//	int32_t total = VOICE_BUFFER_SIZE - count ;

//	if ( toneTimeLeft == 0 )
//	{
//		nextToneData() ;
//	}

//#ifndef PCB9XT
//	if ( g_eeGeneral.softwareVolume )
//	{
//#endif
//		voiceMultiplier = SwVolume_scale[CurrentVolume] ;
//		if ( voiceMultiplier > 179 )
//		{
//			voiceMultiplier = 179 ;
//		}
//#ifndef PCB9XT
//	}
//	else
//	{
//#endif
//		voiceMultiplier = 179 ;
//#ifndef PCB9XT
//	}	
//#endif
//	while( count-- )
//	{
//		value = (int16_t) *src++ ;
//		value *= voiceMultiplier ;
//		value += 32768 * 256 ;
//		*dest++ = value >> 12 ;
//	}
//	while ( total-- )
//	{
//		*dest++ = 2048 ;
//	}

//// not software volume
//	voiceMultiplier = 179 ;	// 70%
//	while( count-- )
//	{
//		value = (int16_t) *src++ ;
//		value *= voiceMultiplier ;
//		value += 32768 * 256 ;
//		value >>= 12 ;
//		if ( toneTimer-- )
//		{
//			uint32_t y ;
//			toneCount += toneFrequency ;
//			y = ( toneCount & 0x01F0) >> 4 ;
//			if ( toneFrequency )
//			{
//				value += Sine16kInt[y] ;
//			}
//		}
//		else
//		{
//			if ( --toneTimeLeft == 0 )
//			{
//				break ;
//			}
//			else
//			{
//				if ( toneFrequency )
//				{
//					toneFrequency += toneFreqIncr ;
//				}
//				toneTimer = 160 ;
//			}
//		}
//		*dest++ = value ;
//	}




//}

void waitVoiceAllSent( uint32_t index )
{
	uint32_t x ;
	// Now wait for last buffer to have been sent
	x = 100 ;
 	while ( ( VoiceBuffer[index].flags & VF_SENT ) == 0 )
	{
		CoTickDelay(1) ;					// 2mS for now
		if ( --x == 0 )
		{
			break ;		// Timeout, 200 mS
		}
	}
	endVoice() ;
}

static uint32_t currentFrequency ;

void doTone()
{
	int32_t toneOver ;
	uint32_t v_index ;
	uint32_t x ;
					
	if ( SystemOptions & SYS_OPT_MUTE )
	{
		nextToneData() ;	// Discard item
		return ;
	}

	beginToneFill() ;
	currentFrequency = VoiceBuffer[0].frequency = 16000 ;		// sample rate
	toneFill( VoiceBuffer[0].dataw, TONE_SET ) ;
	VoiceBuffer[0].count = VOICE_BUFFER_SIZE ;
	toneFill( VoiceBuffer[1].dataw, TONE_SET ) ;
	VoiceBuffer[1].count = VOICE_BUFFER_SIZE ;
	VoiceBuffer[1].frequency = currentFrequency ;
	toneFill( VoiceBuffer[2].dataw, TONE_SET ) ;
	VoiceBuffer[2].count = VOICE_BUFFER_SIZE ;
	VoiceBuffer[2].frequency = currentFrequency ;
	startVoice( NUM_VOICE_BUFFERS ) ;
	
	for(x = 0;;)
	{
		
		while ( ( VoiceBuffer[x].flags & VF_SENT ) == 0 )
		{
			CoTickDelay(1) ;					// 2mS for now
		}
		toneOver = toneFill( VoiceBuffer[x].dataw, TONE_SET ) ;
		VoiceBuffer[x].count = VOICE_BUFFER_SIZE ;
		VoiceBuffer[x].frequency = currentFrequency ;
		appendVoice( x ) ;		// index of next buffer
		v_index = x ;		// Last buffer sent
		x += 1 ;
		if ( x > NUM_VOICE_BUFFERS - 1 )
		{
			x = 0 ;							
		}
		if ( toneOver )
		{
			// finish
			waitVoiceAllSent( v_index ) ;
//#ifdef PCBX12D
//	GPIOI->BSRRH = AUDIO_SD_GPIO_PIN ;	// Set low
//#endif
			break ;
		}
	}
}

void waitAudioAllSentWithTone( uint32_t x, uint32_t v_index)
{
	uint32_t toneMerging = 0 ;
	for(;;)
	{
		while ( ( VoiceBuffer[x].flags & VF_SENT ) == 0 )
		{
			CoTickDelay(1) ;					// 2mS for now
		}
		toneMerging = !toneFill( VoiceBuffer[x].dataw, TONE_SET ) ;
		VoiceBuffer[x].count = VOICE_BUFFER_SIZE ;
		VoiceBuffer[x].frequency = currentFrequency ;
		appendVoice( x ) ;		// index of next buffer
		v_index = x ;		// Last buffer sent
		x += 1 ;
		if ( x > NUM_VOICE_BUFFERS - 1 )
		{
			x = 0 ;							
		}
		if ( toneMerging == 0 )
		{
			// finish
			break ;
		}
	}
	waitVoiceAllSent( v_index ) ;
//#ifdef PCBX12D
//	GPIOI->BSRRH = AUDIO_SD_GPIO_PIN ;	// Set low
//#endif
}




// BgNread should be valid
void beginMusic( uint32_t offset, uint32_t size, uint32_t frequency )
{
	uint32_t x ;
	UINT nread ;
//	FRESULT fr ;
	uint16_t numBuffers = 0 ;

	nread = BgNread ;
	if ( nread == 0 )
	{
		f_read( &BgFile, BgFileData, VOICE_BUFFER_SIZE*2, &nread ) ;
//		fr = f_read( &BgFile, BgFileData, VOICE_BUFFER_SIZE*2, &nread ) ;
		size -= nread ;
	}
	if ( nread == 0 )
	{
		f_close( &BgFile ) ;
		MusicPlaying = MUSIC_STOPPED ;
		MusicInterrupted = 0 ;
		return ;
	}

	nread /= 2 ;
	for( x = 0 ; x < offset ; x += 1 )
	{
		BgFileData[x] = 0 ;
	}
	 	
	wavU16Convert( (uint16_t *)&BgFileData[0], VoiceBuffer[0].dataw, VOICE_BUFFER_SIZE, 0 ) ;
	VoiceBuffer[0].count = VOICE_BUFFER_SIZE ;
	size -= VOICE_BUFFER_SIZE * 2 - offset ;
	BgFrequency = currentFrequency = VoiceBuffer[0].frequency = frequency ;		// sample rate
	numBuffers = 1 ;

	// Buffer 1 is now set, now fill the rest
	for ( x = 1 ; x < NUM_VOICE_BUFFERS ; x += 1 )
	{
		f_read( &BgFile, BgFileData, VOICE_BUFFER_SIZE*2, &nread ) ;
//		fr = f_read( &BgFile, BgFileData, VOICE_BUFFER_SIZE*2, &nread ) ;
		if ( nread == 0 )
		{
			break ;
		}
		size -= nread ;
		nread /= 2 ;
		wavU16Convert( (uint16_t *)&BgFileData[0], VoiceBuffer[x].dataw, nread, 0 ) ;
		VoiceBuffer[x].count = nread ;
		numBuffers += 1 ;
	}
	if ( nread )
	{
		f_read( &BgFile, BgFileData, VOICE_BUFFER_SIZE*2, &nread ) ;
//		fr = f_read( &BgFile, BgFileData, VOICE_BUFFER_SIZE*2, &nread ) ;
		size -= nread ;
	}
	BgNread = nread ;

	startVoice( numBuffers ) ;
	BgLastBuffer = numBuffers - 1 ;
	MusicPlaying = MUSIC_PLAYING ;
	BgSizeLeft = size ;
}

extern TCHAR PlaylistDirectory[] ;
extern struct fileControl PlayFileControl ;
extern char PlayListNames[][MUSIC_NAME_LENGTH+2] ;
extern uint16_t PlayListCount ;
uint16_t PlaylistIndex ;
char CurrentPlayName[MUSIC_NAME_LENGTH+2] ;

void bgStart()
{
	FRESULT fr ;
	UINT nread ;
	uint32_t x ;
//	uint32_t w8or16 ;
	uint32_t size ;
	char plfilename[2*MUSIC_NAME_LENGTH+20] ;
	uint8_t *ptr ;
	
	if ( g_eeGeneral.musicType )
	{
		ptr = cpystr( (uint8_t *)plfilename, (uint8_t *)g_eeGeneral.musicVoiceFileName ) ;
		*ptr++ = '\\' ;
		if ( PlaylistIndex >= PlayListCount )
		{
			PlaylistIndex = 0 ;
		}
		g_eeGeneral.playListIndex = PlaylistIndex ;
		cpystr( ptr, (uint8_t *) PlayListNames[PlaylistIndex] ) ;
		cpystr( (uint8_t *)CurrentPlayName, (uint8_t *) PlayListNames[PlaylistIndex] ) ;
		buildFilename( VLOC_MUSIC, (uint8_t *)plfilename ) ;
	}
	else
	{
		cpystr( (uint8_t *)CurrentPlayName, g_eeGeneral.musicVoiceFileName ) ;
		buildFilename( VLOC_MUSIC, g_eeGeneral.musicVoiceFileName ) ;
	}
	fr = f_open( &BgFile, VoiceFilename, FA_READ ) ;
	if ( fr == FR_OK )
	{
		uint32_t offset ;
		fr = f_read( &BgFile, BgFileData, VOICE_BUFFER_SIZE*2, &nread ) ;
		x = BgFileData[34] + ( BgFileData[35] << 8 ) ;		// sample size
		if ( x != 16 )
		{
			fr = f_close( &BgFile ) ;
			MusicPlaying = MUSIC_STOPPED ;
		 	return ;	
		}
		x = BgFileData[24] + ( BgFileData[25] << 8 ) ;		// sample rate
		offset = 39 ;
		while ( BgFileData[offset] != 'a' )
		{
			size = BgFileData[offset+1] + ( BgFileData[offset+2] << 8 ) + ( BgFileData[offset+3] << 16 ) ;		// data size
			offset += 8 + size ;
			if ( offset > 300 )
			{
				break ;
			}
		}
		if ( offset <= 300 )
		{
			size = BgFileData[offset+1] + ( BgFileData[offset+2] << 8 ) + ( BgFileData[offset+3] << 16 ) + ( BgFileData[offset+4] << 24 ) ;		// data size
			BgTotalSize = size ;
			offset += 5 ;
			BgNread = nread ;
			beginMusic( offset, size, x ) ;
		}
		Bgindex = 0 ;
	}
	else
	{
		MusicPlaying = MUSIC_STOPPED ;
	}
}

extern uint16_t g_timeBgRead ;

void BgPlaying()
{
	uint32_t x ;
	uint32_t size ;
	uint32_t v_index ;
	UINT nread ;
//	FRESULT fr ;
	uint32_t toneMerging = 0 ;

	size = BgSizeLeft ;
	v_index = BgLastBuffer ;
	nread = BgNread ;

	for(x = Bgindex;;)
	{
		uint32_t amount = VOICE_BUFFER_SIZE*2 ;
							
		if ( size < amount )
		{
			amount = size ;
		}

		if ( nread == 0 )
		{
			uint16_t t1 = getTmr2MHz() ;
			f_read( &BgFile, (uint8_t *)BgFileData, amount, &nread ) ;		// Read next buffer
//			fr = f_read( &BgFile, (uint8_t *)BgFileData, amount, &nread ) ;		// Read next buffer
			t1 = getTmr2MHz() - t1 ;
			g_timeBgRead = t1 ;
			size -= nread ;
		}
		BgSizePlayed = size ;
		if ( nread == 0 )
		{
			if ( toneMerging )
			{
				waitAudioAllSentWithTone( x, v_index ) ;
			}
			else
			{
				waitVoiceAllSent( v_index ) ;
			}
			f_close( &BgFile ) ;
			MusicPlaying = MUSIC_STOPPED ;
			break ;
		}
	  while ( ( VoiceBuffer[x].flags & VF_SENT ) == 0 )
		{
			uint32_t stop = 0 ;
			if ( Voice.VoiceQueueCount )
			{
				MusicInterrupted = 160 ;
				stop = 1 ;
			}
//			if (ToneQueueRidx != ToneQueueWidx)
//			{
//				MusicInterrupted = 1 ;
//				stop = 1 ;
//			}
			if ( (MusicPlaying == MUSIC_PAUSING ) || (MusicPlaying == MUSIC_STOPPING ) || (MusicPlaying == MUSIC_STARTING ) )
			{
				stop = 1 ;
			}
			if ( MusicPrevNext )
			{
				if ( g_eeGeneral.musicType )
				{				
					stop = 1 ;
				}
				else
				{
					MusicPrevNext = MUSIC_NP_NORMAL ;
				}
			}
			
			if ( stop )
			{
				
				// wait for all buffers sent
				if ( toneMerging )
				{
					waitAudioAllSentWithTone( x, v_index ) ;
				}
				else
				{
					waitVoiceAllSent( v_index ) ;
				}
//				timer = 100 ;
// 				while ( ( VoiceBuffer[v_index].flags & VF_SENT ) == 0 )
//				{
//					CoTickDelay(1) ;					// 2mS for now
//					if ( --timer == 0 )
//					{
//						break ;		// Timeout, 200 mS
//					}
//				}
//				endVoice() ;
				if (MusicPlaying == MUSIC_PAUSING )
				{
					MusicPlaying = MUSIC_PAUSED ;
				}
				if ( MusicPlaying == MUSIC_STOPPING )
				{
					MusicPlaying = MUSIC_STOPPED ;
					f_close( &BgFile ) ;
				}
				if ( MusicPlaying == MUSIC_STARTING )
				{
					f_close( &BgFile ) ;
				}
				if ( MusicPrevNext )
				{
					if ( g_eeGeneral.musicType )
					{
						f_close( &BgFile ) ;
						if ( MusicPrevNext == MUSIC_NP_PREV )
						{
							if ( PlaylistIndex == 0 )
							{
								PlaylistIndex = PlayListCount - 1 ;
							}
							else
							{
								PlaylistIndex -= 1 ;
							}
						}
						else
						{
							PlaylistIndex += 1 ;
							if ( PlaylistIndex >= PlayListCount )
							{
								PlaylistIndex = 0 ;
							}
						}
						MusicPlaying = MUSIC_STARTING ;
					}
					MusicPrevNext = MUSIC_NP_NORMAL ;
				}
				BgSizeLeft = size ;
				BgNread = nread ;
//				Bgindex = x ;
				return ;
			}
			CoTickDelay(1) ;					// 2mS for now
		}
		if ( AudioVoiceUnderrun )
		{
			// We weren't quick enough
			AudioVoiceCountUnderruns += 100 ;
			AudioVoiceUnderrun = 0 ;
			endVoice() ;
		}
		{
			if ( nread < VOICE_BUFFER_SIZE * 2 )
			{
				uint32_t index ;
				for ( index = nread ; index < VOICE_BUFFER_SIZE * 2 ; index += 1 )
				{
					BgFileData[index] = 0 ;
				}
			}
//			nread /= 2 ;
			if ( toneMerging == 0 )
			{
				if (ToneQueueRidx != ToneQueueWidx)
				{
					toneMerging = 1 ;
					beginToneFill() ;
				}											
			}
			toneMerging = !wavU16Convert( (uint16_t*)&BgFileData[0], VoiceBuffer[x].dataw, VOICE_BUFFER_SIZE, toneMerging ) ;
		}
// May no longer need these lines									
//		if ( nread == 1 )
//		{
//			nread = 2 ;
//			VoiceBuffer[x].dataw[1] = VoiceBuffer[x].dataw[0] ;
//		}
// End of possible removal
//		while ( nread < VOICE_BUFFER_SIZE )
//		{
//			VoiceBuffer[x].dataw[nread++] = 2048 ;		// Pad last buffer
//		}
		VoiceBuffer[x].count = VOICE_BUFFER_SIZE ;
		VoiceBuffer[x].frequency = BgFrequency ;
		appendVoice( x ) ;		// index of next buffer
		v_index = x ;		// Last buffer sent
		x += 1 ;
		if ( x > NUM_VOICE_BUFFERS - 1 )
		{
			x = 0 ;							
		}
		if ( (int32_t)size <= 0 )
		{
			if ( toneMerging )
			{
				waitAudioAllSentWithTone( x, v_index ) ;
			}
			else
			{
				waitVoiceAllSent( v_index ) ;
			}
			f_close( &BgFile ) ;
			MusicPlaying = MUSIC_STOPPED ;
			if ( g_eeGeneral.musicType )
			{
				PlaylistIndex += 1 ;
				if ( PlaylistIndex >= PlayListCount )
				{
					PlaylistIndex = 0 ;
				}
				else
				{
					MusicPlaying = MUSIC_STARTING ;
				}
			}
			if ( g_eeGeneral.musicLoop )
			{
				MusicPlaying = MUSIC_STARTING ;
			}
			break ;								
		}
		nread = 0 ;
	}
	BgSizeLeft = size ;
}

void stopMusic()
{
	if ( MusicPlaying != MUSIC_STOPPED )
	{
		MusicPlaying = MUSIC_STOPPING ;
	}
}

void flushVoiceQueue()
{
	VoiceFlushing = 0x80 + Voice.VoiceQueueInIndex ;
}

uint8_t DebugUnderRun ;

void voice_task(void* pdata)
{
	uint32_t v_index ;
	FRESULT fr ;
	UINT nread ;
	uint32_t x ;
//	uint32_t w8or16 ;
	uint32_t mounted = 0 ;
	uint32_t size ;
	uint32_t toneMerging = 0 ;
	uint8_t *name ;
	uint32_t playListRead = 0 ;
//	uint32_t restarting = 0 ;

//#ifdef PCB9XT
//	uint32_t muted ;
//	muted = 0 ;
//#endif
	for(;;)
	{
		while ( !sd_card_ready() )
		{
			CoTickDelay(5) ;					// 10mS for now
			if ( Activated == 0 )
			{
#ifndef SIMU
				sdPoll10mS() ;
#endif
				ToneQueueWidx = ToneQueueRidx ;		// Discard Tone queue
			}
		 	// Play a tone
			else
			{
				if (ToneQueueRidx != ToneQueueWidx)
				{
//#ifdef PCBX12D
//	GPIOI->BSRRL = AUDIO_SD_GPIO_PIN ;	// Set high
//#endif
					doTone() ;
				}
			}
		}
extern uint32_t sdMounted( void ) ;
#ifdef PCBSKY
		if ( mounted == 0 )
#else
		if ( sdMounted() == 0 )
#endif
		{
  		fr = f_mount(0, &g_FATFS) ;
		}
		else
		{
			fr = FR_OK ;
		}

		if ( fr == FR_OK)
		{
		 SdMounted = mounted = 1 ;
		 SectorsPerCluster = g_FATFS.csize ;
		 CoTickDelay( SdAccessRequest ? 10 : 1) ;					// 4mS for now

		 if ( ( g_eeGeneral.musicType ) && ( playListRead == 0 ) )
		 {
				// load playlist
				cpystr( cpystr( (uint8_t *)PlaylistDirectory, (uint8_t *)"\\music\\" ), g_eeGeneral.musicVoiceFileName ) ;
				fillPlaylist( PlaylistDirectory, &PlayFileControl, (char *)"WAV" ) ;
				PlaylistIndex = g_eeGeneral.playListIndex ;
				if ( PlaylistIndex > PlayListCount )
				{
					PlaylistIndex = 0 ;
				}
				playListRead = 1 ;
		 }
	 
		 while ( ( Voice.VoiceQueueCount == 0 ) && (ToneQueueRidx == ToneQueueWidx) )
		 {
				AudioActive = 0 ;
//#ifdef PCB9XT
//		 	if ( muted == 0 )
//			{
//				uint32_t value = 1900 ;
//				do
//				{
//					DAC->DHR12R1 = value ;
//					value -= 100 ;
//		 			CoTickDelay(1) ;					// 2mS
//				} while ( value ) ;
//				muted = 1 ;
//			}
//#endif
			if ( MusicInterrupted )
			{
				if ( --MusicInterrupted == 0 )
				{
					if ( MusicPlaying == MUSIC_PLAYING )
					{
//#ifdef PCBX12D
//	GPIOI->BSRRL = AUDIO_SD_GPIO_PIN ;	// Set high
//#endif
						beginMusic( 0, BgSizeLeft, BgFrequency ) ;
					}
				}
			}
			if ( MusicPlaying == MUSIC_STARTING )
			{
//#ifdef PCBX12D
//	GPIOI->BSRRL = AUDIO_SD_GPIO_PIN ;	// Set high
//#endif
				bgStart() ;
			}
			else if ( MusicPlaying == MUSIC_RESUMING )
			{
//#ifdef PCBX12D
//	GPIOI->BSRRL = AUDIO_SD_GPIO_PIN ;	// Set high
//#endif
				beginMusic( 0, BgSizeLeft, BgFrequency ) ;
			}
			else if ( MusicPlaying == MUSIC_STOPPING )
			{
				waitVoiceAllSent( Bgindex ) ;
				f_close( &BgFile ) ;
				MusicPlaying = MUSIC_STOPPED ;
//#ifdef PCBX12D
//	GPIOI->BSRRH = AUDIO_SD_GPIO_PIN ;	// Set low
//#endif
			}
			else if (MusicPlaying == MUSIC_PAUSING )
			{
				MusicPlaying = MUSIC_PAUSED ;
			}
			
			if ( ( MusicPlaying == MUSIC_PLAYING ) && (!MusicInterrupted ) )
			{
				BgPlaying() ;
			}	
			else
			{
		 		CoTickDelay(3) ;					// 6mS for now
			}
		 }
//#ifdef PCB9XT
//		 if ( muted )
//		 {
//				uint32_t value = 10 ;
//				do
//				{
//					DAC->DHR12R1 = value ;
//					value += 100 ;
//		 			CoTickDelay(1) ;					// 2mS
//				} while ( value < 2010 ) ;
//			DAC->DHR12R1 = 2010 ;
//		 	muted = 0 ;
//		 }
//#endif
		 if ( MusicInterrupted )
		 {
		 	MusicInterrupted = 160 ;
		 }

		 if ( VoiceFlushing )
		 {
		 	VoiceFlushing &= 0x7F ;
			while ( ( Voice.VoiceQueueOutIndex != VoiceFlushing ) && (Voice.VoiceQueueCount) )
			{
				Voice.VoiceQueueOutIndex += 1 ;
				Voice.VoiceQueueOutIndex &= ( VOICE_Q_LENGTH - 1 ) ;
				__disable_irq() ;
				Voice.VoiceQueueCount -= 1 ;
				__enable_irq() ;
			}
		 	VoiceFlushing = 0 ;
		 }

		 if ( Voice.VoiceQueueCount )
		 {
		 	uint32_t processed = 0 ;
		 	AudioActive = 1 ;
//			ToneQueueWidx = ToneQueueRidx ;		// Discard Tone queue
		 	
//#ifdef PCBX12D
//	GPIOI->BSRRL = AUDIO_SD_GPIO_PIN ;	// Set high
//#endif
			name = Voice.NamedVoiceQueue[Voice.VoiceQueueOutIndex] ;
			v_index = Voice.VoiceQueue[Voice.VoiceQueueOutIndex] ;
			if ( ( SystemOptions & SYS_OPT_MUTE ) || MuteTimer )
			{
				Voice.VoiceQueueOutIndex += 1 ;
				Voice.VoiceQueueOutIndex &= ( VOICE_Q_LENGTH - 1 ) ;
				__disable_irq() ;
				Voice.VoiceQueueCount -= 1 ;
				__enable_irq() ;
				continue ;
			}

			if ( (v_index & VOLUME_MASK) == VOLUME_MASK )
			{
				v_index &= ~VOLUME_MASK ;
				if ( v_index == (uint16_t)~VOLUME_MASK )
				{
					HoldVolume = 0 ;
					setVolume( SaveVolume ) ;
				}
				else
				{
					HoldVolume = v_index ;
					SaveVolume = CurrentVolume ;
					setVolume( v_index ) ;
				}
				processed = 1 ;
			}
			else
			{
				CoSchedLock() ;
				if ( Voice.VoiceLock == 0 )
				{
					Voice.VoiceLock = 1 ;
  				CoSchedUnlock() ;

					processed = 1 ;
					buildFilename( v_index, name ) ;
					x = v_index ;	// For inactivity alarm
					fr = f_open( &Vfile, VoiceFilename, FA_READ ) ;
					if ( fr != FR_OK )
					{
						CoTickDelay(1) ;					// 2mS for now
						if ( (v_index & VLOC_MASK) == VLOC_SYSTEM )
						{
							v_index &= ~VLOC_MASK ;
							if ( v_index )
							{
								buildFilename( v_index | VLOC_NUMSYS , name ) ;
								fr = f_open( &Vfile, VoiceFilename, FA_READ ) ;
							}
						}
						else
						{
							if ( ( (v_index & VLOC_MASK) == VLOC_NUMSYS ) || ( (v_index & VLOC_MASK) == VLOC_NUMUSER ) )
							v_index &= ~VLOC_MASK ;
							if ( v_index )
							{
								buildFilename( v_index, name ) ;
								fr = f_open( &Vfile, VoiceFilename, FA_READ ) ;
							}
						}
					}
					CoTickDelay(1) ;					// 2mS for now
					if ( fr == FR_OK )
					{
						uint32_t offset ;
						fr = f_read( &Vfile, FileData, VOICE_BUFFER_SIZE*2, &nread ) ;
						x = FileData[34] + ( FileData[35] << 8 ) ;		// sample size
						if ( x == 16 )
						{
							
								x = FileData[24] + ( FileData[25] << 8 ) ;		// sample rate

							offset = 39 ;
							while ( FileData[offset] != 'a' )
							{
								size = FileData[offset+1] + ( FileData[offset+2] << 8 ) + ( FileData[offset+3] << 16 ) ;		// data size
								offset += 8 + size ;
								if ( offset > 300 )
								{
									break ;
								}
							}
							if ( offset <= 300 )
							{
								uint32_t y ;
								size = FileData[offset+1] + ( FileData[offset+2] << 8 ) + ( FileData[offset+3] << 16 ) ;		// data size
								offset += 5 ;
						
								size -= VOICE_BUFFER_SIZE-offset ;

								{
									wavU16Convert( (uint16_t*)&FileData[offset], &VoiceBuffer[0].dataw[offset/2], VOICE_BUFFER_SIZE-offset/2, 0 ) ;
									y = offset/2 ;
									VoiceBuffer[0].count = VOICE_BUFFER_SIZE ;
									size -= VOICE_BUFFER_SIZE ;
								}
								uint32_t amount ;
								VoiceBuffer[0].frequency = x ;		// sample rate
								currentFrequency = VoiceBuffer[0].frequency = x ;		// sample rate

								for( x = 0 ; x < y ; x += 1 )
								{
									VoiceBuffer[0].dataw[x] = 2048 ;
								}
								 
								{
									fr = f_read( &Vfile, FileData, VOICE_BUFFER_SIZE*2, &nread ) ;
									wavU16Convert( (uint16_t*)&FileData[0], VoiceBuffer[1].dataw, VOICE_BUFFER_SIZE, 0 ) ;
									size -= nread ;
								}
								VoiceBuffer[1].count = VOICE_BUFFER_SIZE ;
					
								amount = VOICE_BUFFER_SIZE*2 ;

								for ( x = 2 ; x < NUM_VOICE_BUFFERS ; x += 1 )
								{
									{
										fr = f_read( &Vfile, &FileData[0], amount, &nread ) ;		// Read next buffer
										wavU16Convert( (uint16_t *)&FileData[0], VoiceBuffer[x].dataw, VOICE_BUFFER_SIZE, 0 ) ;
									}
									size -= nread ;
									VoiceBuffer[x].count = VOICE_BUFFER_SIZE ;
								}
								startVoice( NUM_VOICE_BUFFERS ) ;
								CoTickDelay(1) ;					// 2mS for now
								for(x = 0;;)
								{
									if ( size < amount )
									{
										amount = size ;								
									}
									fr = f_read( &Vfile, (uint8_t *)FileData, amount, &nread ) ;		// Read next buffer
									size -= nread ;
									if ( nread == 0 )
									{
										break ;
									}
									if ( nread < VOICE_BUFFER_SIZE * 2 )
									{
										uint32_t index ;
										for ( index = nread ; index < VOICE_BUFFER_SIZE * 2 ; index += 1 )
										{
											FileData[index] = 0 ;
										}
									}
		  						while ( ( VoiceBuffer[x].flags & VF_SENT ) == 0 )
									{
										CoTickDelay(1) ;					// 2mS for now
										if ( DebugUnderRun )
										{
											DebugUnderRun = 0 ;
											CoTickDelay(70) ;
										}
									}
									if ( AudioVoiceUnderrun )
									{
										// We weren't quick enough
										AudioVoiceCountUnderruns += 1 ;
										AudioVoiceUnderrun = 0 ;
										endVoice() ;
//										break ;
									}
									{
										nread /= 2 ;
										if ( toneMerging == 0 )
										{
											if (ToneQueueRidx != ToneQueueWidx)
											{
												toneMerging = 1 ;
												beginToneFill() ;
											}											
										}
										toneMerging = !wavU16Convert( (uint16_t*)&FileData[0], VoiceBuffer[x].dataw, VOICE_BUFFER_SIZE, toneMerging ) ;
									}
									VoiceBuffer[x].count = VOICE_BUFFER_SIZE ;
									VoiceBuffer[x].frequency = currentFrequency ;
//									if ( restarting )
//									{
//										startVoice( 1 ) ;
//										restarting = 0 ;
//									}
//									else
//									{
										appendVoice( x ) ;		// index of next buffer
//									}
									v_index = x ;		// Last buffer sent
									x += 1 ;
									if ( x > NUM_VOICE_BUFFERS - 1 )
									{
										x = 0 ;							
									}
									if ( (int32_t)size <= 0 )
									{
										break ;								
									}
								}
//								}
							}
							fr = f_close( &Vfile ) ;
							// Now wait for last buffer to have been sent

							if ( toneMerging )
							{
								waitAudioAllSentWithTone( x, v_index ) ;
							}
							else
							{
								waitVoiceAllSent( v_index ) ;
							}
//#ifdef PCBX12D
//	GPIOI->BSRRH = AUDIO_SD_GPIO_PIN ;	// Set low
//#endif
						}
					}
					else if (fr != FR_NO_FILE)			// There is no file to open
					{
						SDlastError = fr ;
						SdMounted = mounted = 0 ;
					}
					else
					{
						if ( (x & VLOC_MASK) == VLOC_SYSTEM )
						{
							if ( v_index == V_INACTIVE )
							{
								setVolume( g_eeGeneral.inactivityVolume + ( NUM_VOL_LEVELS-3 ) ) ;
    						audioDefevent( AU_INACTIVITY ) ;
							}
						}
					}
					Voice.VoiceLock = 0 ;
				}
				else
				{
  				CoSchedUnlock() ;
				}
			}
			if ( processed )
			{
				Voice.VoiceQueueOutIndex += 1 ;
				Voice.VoiceQueueOutIndex &= ( VOICE_Q_LENGTH - 1 ) ;
				__disable_irq() ;
				Voice.VoiceQueueCount -= 1 ;
				__enable_irq() ;
			}
		 }
		 else
		 {
		 	
		 	// Play a tone
			if (ToneQueueRidx != ToneQueueWidx)
			{
//#ifdef PCBX12D
//	GPIOI->BSRRL = AUDIO_SD_GPIO_PIN ;	// Set high
//#endif
				doTone() ;
			}
			else
			{
	 			CoTickDelay(3) ;					// 6mS for now
			}
		 }
		}
		else
		{
			SDlastError = fr ;
		 	// Play a tone
			if (ToneQueueRidx != ToneQueueWidx)
			{
				AudioActive = 1 ;
//#ifdef PCBX12D
//	GPIOI->BSRRL = AUDIO_SD_GPIO_PIN ;	// Set high
//#endif
				doTone() ;
			}

		}
		CoTickDelay(1) ;					// 2mS for now
	} // for(;;)
}

//static uint8_t toneIndex ;
//static uint16_t toneTime ;

void beginToneFill()
{
//	toneTimer = 160 ;
//	toneCount = 0 ;
	toneActive = 0 ;
//	toneTimeLeft = ToneEntry.toneTimeLeft ;
//	toneFreqIncr = ToneEntry.toneFreqIncr ;
//	frequency = ToneEntry.toneFreq ;
//	tonePause = ToneEntry.tonePause ;
//	toneRepeat = ToneEntry.toneRepeat ;
}


void queueTone( uint8_t place, uint8_t freq, int8_t freqInc, uint8_t time, uint8_t pause, uint8_t repeat )
{
	struct toneQentry *ptr_queue ;
  uint8_t nextWidx ;

	if ( place )	// playNow so put at front
	{
		ToneQueueWidx = ToneQueueRidx ;		// Discard queue
	}
	else
	{
		if ( !ToneFreeSlots() )
		{
			return ;
		}
	}
	if ( freq )
	{
  	nextWidx = (ToneQueueWidx + 1) % AUDIO_QUEUE_LENGTH;
  	if (nextWidx != ToneQueueRidx)
		{
			ptr_queue = &ToneQueue[ToneQueueWidx] ;
			ptr_queue->toneFreq = freq ;
			ptr_queue->toneFreqIncr = freqInc ;
			ptr_queue->toneTimeLeft = time ;
			ptr_queue->tonePause = pause ;
			ptr_queue->toneRepeat = repeat ;
			ToneQueueWidx = nextWidx ;
		}
	}
}



void nextToneData()
{
//	if ( SdMounted && Voice.VoiceQueueCount )
//	{
//		ToneQueueWidx = ToneQueueRidx ;		// Discard queue
//		return ;
//	}

	if ( toneActive == 0 )
	{
		if (ToneQueueRidx != ToneQueueWidx)
		{
			ToneEntry = ToneQueue[ToneQueueRidx] ;
			ToneQueueRidx = (ToneQueueRidx + 1) % AUDIO_QUEUE_LENGTH;
			toneTimeLeft = ToneEntry.toneTimeLeft ;
			toneFreqIncr = ToneEntry.toneFreqIncr ;
			toneFrequency = ToneEntry.toneFreq ;
			tonePause = ToneEntry.tonePause ;
			toneRepeat = ToneEntry.toneRepeat & 0x7F ;
			toneVarioVolume = ToneEntry.toneRepeat & 0x80 ? 1 : 0 ;
			toneOrPause = 0 ;		// Tone
			toneTimer = 160 ;
//			if ( tonePause || toneRepeat)
//			{
				toneActive = 1 ;
//			}
		}
		return ;
	}
	 
	if ( toneOrPause == 0 )		// Tone
	{ // Look at pause
		if ( tonePause )
		{
			toneOrPause = 1 ;		// Pause
			toneTimer = 160 ;
			toneFrequency = 0 ;
			toneTimeLeft = tonePause ;
			tonePause = 0 ;
			if ( toneRepeat == 0 )
			{
				toneActive = 0 ;
			}
		}
	}
	else
	{ // End of pause, check for repeat
		if ( toneRepeat )
		{
			toneRepeat -= 1 ;
			toneTimer = 160 ;
			toneCount = 0 ;
			toneTimeLeft = ToneEntry.toneTimeLeft ;
			toneFreqIncr = ToneEntry.toneFreqIncr ;
			toneFrequency = ToneEntry.toneFreq ;
			tonePause = ToneEntry.tonePause ;
			toneOrPause = 0 ;		// Tone
			if ( (tonePause == 0) && (toneRepeat == 0) )
			{
				toneActive = 0 ;
			}
		}
	}
}

// This for 16kHz sample rate
// Returns +ve or zero, remaining time
//   -ve offset into buffer of blank time
int32_t toneFill( uint16_t *buffer, uint32_t add )
{
	uint32_t i = 0 ;
	uint32_t y ;
	uint32_t multiplier ;
	int32_t value ;
	multiplier = swVolumeLevel() ;

	if ( toneTimeLeft == 0 )
	{
		nextToneData() ;
	}

	if ( toneVarioVolume )
	{
		if ( multiplier )
		{	// Mute if master volume is zero
			if ( g_model.varioExtraData.volume )
			{
				// 0 means use master volume
				multiplier = SwVolume_scale[g_model.varioExtraData.volume] ;
			}
		}
	}
	
	if ( toneFrequency < BEEP_DEFAULT_FREQ - 5 )
	{
		multiplier *= 15 ;
		multiplier /= 10 ;
	}

	while ( toneTimeLeft )
	{
		while ( i < 512 )
		{
			if ( toneTimer-- )
			{
				toneCount += toneFrequency ;
				if ( currentFrequency == 32000 )
				{
					y = ( toneCount & 0x03E0) >> 5 ;
				}
				else
				{
					y = ( toneCount & 0x01F0) >> 4 ;
				}
#ifndef PCB9XT
				if ( g_eeGeneral.softwareVolume )
				{
#endif
					if ( add == TONE_SET )
					{
						if ( toneFrequency )
						{
							value = (int32_t)Sine16kInt[y] ;
							value *= multiplier ;
							value += 2048 * 256 ;
							value >>= 8 ;
						}
						else
						{
							value = 2048 ;
						}
						*buffer++ = value ;
					}
					else
					{ // adding
						if ( toneFrequency )
						{
							value = (int32_t)Sine16kInt[y] ;
							value *= multiplier ;
							value >>= 8 ;
							*buffer++ += value ;
						}
					}
#ifndef PCB9XT
				}
				else
				{
					if ( add == TONE_SET )
					{
						*buffer++ = toneFrequency ? Sine16kInt[y]+2048 : 2048 ;
					}
					else
					{
						if ( toneFrequency )
						{
							*buffer++ += Sine16kInt[y] ;
						}
					}
				}
#endif
				i += 1 ;
			}
			else
			{
				if ( --toneTimeLeft == 0 )
				{
					break ;
				}
				else
				{
					if ( toneFrequency )
					{
						toneFrequency += toneFreqIncr ;
					}
					if ( currentFrequency == 32000 )
					{
						toneTimer = 320 ;
					}
					else
					{
						toneTimer = 160 ;
					}
				}
			}
		}
		if ( i >= 512 )
		{
			break ;
		}
		nextToneData() ;
	}
	if ( add == TONE_SET )
	{
		while ( i < 512 )
		{
  		*buffer++ = 2048 ;
			i += 1 ;
		}
	}
	return toneTimeLeft == 0 ;
}

// This for 16kHz sample rate
// Returns +ve or zero, remaining time
//   -ve offset into buffer of blank time
//int32_t xtoneFill( uint16_t *buffer, uint32_t frequency, uint32_t timeMs )
//{
//	uint32_t i = 0 ;
//	uint32_t y ;

//	while ( i < 512 )
//	{
//		toneCount += frequency ;
//		y = ( toneCount & 0x01F0) >> 4 ;
//		*buffer++ = Sine16k[y] ;
//		i += 1 ;
//		if ( --timeMs == 0 )
//		{
//			break ;
//		}
//	}
//	y = i ;
//	if ( i )
//	{ // sine finished, but not buffer
//		while ( i < 512 )
//		{
//	  	*buffer++ = 2048 ;
//			i += 1 ;
//		}
//	}
//	return timeMs > 0 ? timeMs : -(int32_t)y ;
//}



