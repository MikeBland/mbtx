/*
 * Author - Rob Thomson & Bertrand Songis
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

#include "er9x.h"

#define SPEAKER_OFF  PORTE &= ~(1 << OUT_E_BUZZER) // speaker output 'low'

static void HAPTIC_ON()
{
#ifdef XSW_MOD
	if ( g_eeGeneral.hapticStrength > 0)
#else
	if ( g_eeGeneral.pg2Input == 0)
#endif
	{
		PORTG |= (1<<2) ;   // PG2->HI
	}
}

static void HAPTIC_OFF()
{
#ifdef XSW_MOD
	if ( g_eeGeneral.hapticStrength > 0)
#else
	if ( g_eeGeneral.pg2Input == 0)
#endif
	{
		PORTG &= ~(1<<2) ;  // PG2->LO
	}
}


struct t_voice Voice ;

audioQueue::audioQueue()
{
  aqinit();
}

// TODO should not be needed
void audioQueue::aqinit()
{
  //make sure haptic off by default
  HAPTIC_OFF();

//  toneTimeLeft = 0;
//  tonePause = 0;

//  t_queueRidx = 0;
//  t_queueWidx = 0;

//  toneHaptic = 0;
//  hapticTick = 0;

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
//  return AUDIO_QUEUE_LENGTH - ((t_queueWidx + AUDIO_QUEUE_LENGTH - t_queueRidx) % AUDIO_QUEUE_LENGTH) >= AUDIO_QUEUE_FREESLOTS;
}


// heartbeat is responsibile for issueing the audio tones and general square waves
// it is essentially the life of the class.
// it is called every 10ms
void audioQueue::heartbeat()
{
  if (toneTimeLeft > 0) {
    toneTimeLeft--; //time gets counted down
    toneFreq += toneFreqIncr;
    if (toneHaptic){
      if (hapticTick-- > 0) {
        HAPTIC_ON(); // haptic output 'high'
      }
      else {
        HAPTIC_OFF(); // haptic output 'low'
        hapticTick = g_eeGeneral.hapticStrength;
      }
    }
  }
  else {
    SPEAKER_OFF;
    HAPTIC_OFF();

    //if (tonePause-- <= 0) {
    if (tonePause > 0) {
      tonePause--;
    } else  {  
      if (t_queueRidx != t_queueWidx) {
        toneFreq = queueToneFreq[t_queueRidx];
        toneTimeLeft = queueToneLength[t_queueRidx];
        toneFreqIncr = queueToneFreqIncr[t_queueRidx];
        tonePause = queueTonePause[t_queueRidx];
        toneHaptic = queueToneHaptic[t_queueRidx];
        hapticTick = 0;
        if (!queueToneRepeat[t_queueRidx]--) {
          t_queueRidx = (t_queueRidx + 1) % AUDIO_QUEUE_LENGTH;
        }
      }
    }
  }
}

inline uint8_t audioQueue::getToneLength(uint8_t tLen)
{
  uint8_t result = tLen; // default
  if (g_eeGeneral.beeperVal == 2) {
    result /= 3;
  }
  else if (g_eeGeneral.beeperVal == 3) {
    result /= 2;
  }
  else if (g_eeGeneral.beeperVal == 5) {
    //long
    result *= 2;
  }
  else if (g_eeGeneral.beeperVal == 6) {
    //xlong
    result *= 3;
  }
  return result;
}

void audioQueue::play(uint8_t tFreq, uint8_t tLen, uint8_t tPause, uint8_t flags )
{
	
	if(!freeslots()){
			return;
	}
	if ( SystemOptions & SYS_OPT_MUTE )
	{
		return ;
	}

  if (g_eeGeneral.beeperVal)
	{
	  int8_t tFreqIncr = (flags >> 6) ;
		uint8_t tRepeat = flags & 0x0F ;

	  if (tFreqIncr == 3) tFreqIncr = -1 ;

    if (tFreq > 0)
		{ //we dont add pitch if zero as this is a pause only event
      tFreq += g_eeGeneral.speakerPitch + BEEP_OFFSET; // add pitch compensator
    }
    tLen = getToneLength(tLen);

		if ( flags & PLAY_NOW )
		{
    	toneFreq = tFreq ; // add pitch compensator
    	toneTimeLeft = tLen ;
    	tonePause = tPause;
    	toneHaptic = flags & PLAY_HAPTIC ? 1 : 0 ;
    	hapticTick = 0;
    	toneFreqIncr = tFreqIncr ;
    	t_queueWidx = t_queueRidx;

//    	if (tRepeat) {
//    	  playASAP(tFreq, tLen, tPause, tRepeat-1, toneHaptic, 0 ) ;
//    	}
		}
		else
		{
			tRepeat += 1 ;			
		}

		if ( tRepeat )
		{
    	uint8_t next_queueWidx = (t_queueWidx + 1) % AUDIO_QUEUE_LENGTH;
    	if (next_queueWidx != t_queueRidx)
			{
    	  queueToneFreq[t_queueWidx] = tFreq ; // add pitch compensator
    	  queueToneLength[t_queueWidx] = tLen ;
    	  queueTonePause[t_queueWidx] = tPause;
    	  queueToneHaptic[t_queueWidx] = flags & PLAY_HAPTIC ? 1 : 0;
    	  queueToneRepeat[t_queueWidx] = tRepeat - 1;
    	  queueToneFreqIncr[t_queueWidx] = tFreqIncr;
    	  t_queueWidx = next_queueWidx;
			}
		}
  }
}

//void audioQueue::playASAP(uint8_t tFreq, uint8_t tLen, uint8_t tPause,
//    uint8_t tRepeat, uint8_t tHaptic, int8_t tFreqIncr)
//{
//	if(!freeslots()){
//			return;
//	}	
	
//  if (g_eeGeneral.beeperVal) {
//    uint8_t next_queueWidx = (t_queueWidx + 1) % AUDIO_QUEUE_LENGTH;
//    if (next_queueWidx != t_queueRidx) {
//      queueToneFreq[t_queueWidx] = (tFreq ? tFreq + g_eeGeneral.speakerPitch + BEEP_OFFSET : 0); // add pitch compensator
//      queueToneLength[t_queueWidx] = getToneLength(tLen);
//      queueTonePause[t_queueWidx] = tPause;
//      queueToneHaptic[t_queueWidx] = tHaptic;
//      queueToneRepeat[t_queueWidx] = tRepeat;
//      queueToneFreqIncr[t_queueWidx] = tFreqIncr;
//      t_queueWidx = next_queueWidx;
//    }
//  }
//}

void audioQueue::event(uint8_t e, uint8_t f) {

  uint8_t beepVal = g_eeGeneral.beeperVal;
	if (t_queueRidx == t_queueWidx) {		
	  switch (e) {
		    case AU_WARNING1:
		      play(BEEP_DEFAULT_FREQ, 10, 1, PLAY_HAPTIC | PLAY_NOW ) ;
		      break;
		    case AU_WARNING2:
		      play(BEEP_DEFAULT_FREQ, 20, 1, PLAY_HAPTIC | PLAY_NOW);
		      break;
	      case AU_CHEEP:
	        play(BEEP_DEFAULT_FREQ+30,10,2,2 | PLAY_HAPTIC | PLAY_INCREMENT(2) ) ;
	        break;
	      case AU_RING:
	        play(BEEP_DEFAULT_FREQ+25,5,2,10 |PLAY_HAPTIC);
	        play(BEEP_DEFAULT_FREQ+25,5,10,1 | PLAY_HAPTIC);
	        play(BEEP_DEFAULT_FREQ+25,5,2,10 | PLAY_HAPTIC);
	        break;
	      case AU_SCIFI:
	        play(80,10,3,2 | PLAY_INCREMENT(-1));
	        play(60,10,3,2 | PLAY_INCREMENT(1));
	        play(70,10,1, PLAY_HAPTIC );
	        break;
	      case AU_ROBOT:
	        play(70,5,1,1 | PLAY_HAPTIC);
	        play(50,15,2,1 | PLAY_HAPTIC);
	        play(80,15,2,1 | PLAY_HAPTIC);
	        break;
	      case AU_CHIRP:
	        play(BEEP_DEFAULT_FREQ+40,5,1,2 | PLAY_HAPTIC);
	        play(BEEP_DEFAULT_FREQ+54,5,1,3 | PLAY_HAPTIC);
	        break;
	      case AU_TADA:
	        play(50,5,5,0);
	        play(90,5,5,0);
	        play(110,3,4,2);
	        break;
	      case AU_CRICKET:
	        play(80,5,10,3 | PLAY_HAPTIC);
	        play(80,5,20,1 | PLAY_HAPTIC);
	        play(80,5,10,3 | PLAY_HAPTIC);
	        break;
	      case AU_SIREN:
	        play(10,20,5,2 | PLAY_HAPTIC | PLAY_INCREMENT(1));
	        break;
	      case AU_ALARMC:
	        play(50,4,10,2 | PLAY_HAPTIC);
	        play(70,8,20,1 | PLAY_HAPTIC);
	        play(50,8,10,2 | PLAY_HAPTIC);
	        play(70,4,20,1 | PLAY_HAPTIC);
	        break;
	      case AU_RATATA:
	        play(BEEP_DEFAULT_FREQ+50,5,10,10 | PLAY_HAPTIC);
	        break;
	      case AU_TICK:
	        play(BEEP_DEFAULT_FREQ+50,5,50,2 | PLAY_HAPTIC);
	        break;
	      case AU_HAPTIC1:
	        play(0,20,10,1 | PLAY_HAPTIC);
	        break;
	      case AU_HAPTIC2:
	        play(0,20,10,2 | PLAY_HAPTIC);
	        break;
	      case AU_HAPTIC3:
	        play(0,20,10,3 | PLAY_HAPTIC);
	        break;
		    case AU_INACTIVITY:
		      play(70, 10, 2, 2 | PLAY_NOW ) ;
		      break;
		    case AU_TX_BATTERY_LOW:
		        play(60, 20, 3, 2 | PLAY_INCREMENT(1));
		        play(80, 20, 3, 2 | PLAY_HAPTIC | PLAY_INCREMENT(-1));
		      break;
		    case AU_ERROR:
		      play(BEEP_DEFAULT_FREQ, 40, 1, PLAY_HAPTIC | PLAY_NOW);
		      break;
		    case AU_KEYPAD_UP:
		      if (beepVal != BEEP_NOKEYS) {
		        play(BEEP_KEY_UP_FREQ, 10, 1, PLAY_NOW );
		      }
		      break;
		    case AU_KEYPAD_DOWN:
		      if (beepVal != BEEP_NOKEYS) {
		        play(BEEP_KEY_DOWN_FREQ, 10, 1, PLAY_NOW);
		      }
		      break;
		    case AU_TRIM_MOVE:
		      play(f, 6, 1, PLAY_NOW);
		      break;
		    case AU_TRIM_MIDDLE:
		      play(BEEP_DEFAULT_FREQ, 10, 2, PLAY_HAPTIC | PLAY_NOW);
		      break;
		    case AU_MENUS:
		      if (beepVal != BEEP_NOKEYS) {
		        play(BEEP_DEFAULT_FREQ, 10, 2, PLAY_NOW);
		      }
		      break;
		    case AU_POT_STICK_MIDDLE:
		      play(BEEP_DEFAULT_FREQ + 50, 10, 1, PLAY_NOW);
		      break;
		    case AU_TIMER_30:
		      play(BEEP_DEFAULT_FREQ + 50, 15, 3, 3 | PLAY_HAPTIC | PLAY_NOW);
		      break;
		    case AU_TIMER_20:
		      play(BEEP_DEFAULT_FREQ + 50, 15, 3, 2 | PLAY_HAPTIC | PLAY_NOW);
		      break;
		    case AU_TIMER_10:
		      play(BEEP_DEFAULT_FREQ + 50, 15, 3, 1 | PLAY_HAPTIC | PLAY_NOW);
		      break;
		    case AU_TIMER_LT3:
		      play(BEEP_DEFAULT_FREQ, 20, 25, 1 | PLAY_HAPTIC | PLAY_NOW);
		      break;
		    case AU_WARNING3:
		      play(BEEP_DEFAULT_FREQ, 30, 1, PLAY_HAPTIC | PLAY_NOW);
		      break;
				case AU_MIX_WARNING_1:
		      play(BEEP_DEFAULT_FREQ + 50, 10, 1, 1 | PLAY_HAPTIC | PLAY_NOW);
		      break;
		    case AU_MIX_WARNING_2:
		      play(BEEP_DEFAULT_FREQ + 52, 10, 1, 2 | PLAY_HAPTIC | PLAY_NOW);
		      break;
		    case AU_MIX_WARNING_3:
		      play(BEEP_DEFAULT_FREQ + 54, 10, 1, 3 | PLAY_HAPTIC | PLAY_NOW);
		      break;
		    
				case AU_VARIO_UP :
		      play(BEEP_DEFAULT_FREQ + 60, 10, 0, PLAY_INCREMENT(1) | PLAY_NOW ) ;
		    break ;
		    
				case AU_VARIO_DOWN :
		      play(BEEP_DEFAULT_FREQ - 20, 10, 0, PLAY_INCREMENT(-1) | PLAY_NOW ) ;
		    break ;
		    
				default:
		      break;
	  }
	}  
}

void audioDefevent(uint8_t e)
{
  audio.event(e, BEEP_DEFAULT_FREQ);
}

void audioEvent( uint8_t e, uint16_t f )
{
  audio.event( e, f ) ;
}

void audioVoiceDefevent( uint8_t e, uint8_t v)
{
	if ( g_eeGeneral.speakerMode & 2 )
	{
		putVoiceQueue( v ) ;
	}
	else
	{
    audioDefevent( e ) ;
	}
}

#include <stdlib.h>

// Announce a value using voice
void voice_numeric( int16_t value, uint8_t num_decimals, uint8_t units_index )
{
	uint8_t decimals ;
	div_t qr ;
	uint8_t flag = 0 ;

	if ( units_index > 127 )
	{
		putVoiceQueue( units_index ) ;
	}

	if ( value < 0 )
	{
		value = - value ;
		putVoiceQueue( V_MINUS ) ;		
	}

	if ( num_decimals )
	{
		qr = div( value, num_decimals == 2 ? 100 : 10 ) ;
		decimals = qr.rem ;
		value = qr.quot ;
	}

	qr = div( value, 100 ) ;
	if ( qr.quot )
	{
		// At least 100
		num_decimals = 0 ;		// Cancel decimals
		if ( qr.quot > 9 )		// Thousands
		{
			flag = 1 ;
			div_t xr ;

			xr = div( qr.quot, 10 ) ;
			if ( xr.quot < 21 )
			{
				putVoiceQueue( xr.quot + 110 ) ;
			}
			else
			{
				putVoiceQueueUpper( xr.quot + 140 ) ;
				putVoiceQueue( V_THOUSAND ) ;
			}
			qr.quot = xr.rem ;			
		}
		if ( qr.quot )
		{
			putVoiceQueue( qr.quot + 100 ) ;
		}
		if ( flag == 0 )
		{
			if ( qr.rem )
			{
				putVoiceQueueUpper( qr.rem + 140 ) ;
			}
		}
		else
		{
			if ( qr.rem )
			{
				qr.rem -= qr.rem % 10 ;
				putVoiceQueueUpper( qr.rem + 140 ) ;
			}
		}
	}
	else
	{
		putVoiceQueueUpper( qr.rem + 140 ) ;
	}

	if ( num_decimals )
	{
		if ( num_decimals == 2 )
		{
			qr = div( decimals, 10 ) ;
			putVoiceQueue( qr.quot + 6 ) ;		// Point x
			putVoiceQueueUpper( qr.rem + 140 ) ;
		}
		else
		{
			putVoiceQueue( decimals + 6 ) ;		// Point x
		}
	}
		 
	if ( units_index && ( units_index < 128 ) )
	{
		putVoiceQueue( units_index ) ;
	}
	asm("") ;
}

//struct t_voice *voiceaddress()
//{
//	return &Voice ;
//}











																			      
