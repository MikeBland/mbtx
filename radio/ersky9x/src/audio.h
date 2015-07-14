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

#ifndef audio_h
#define audio_h

/*#define HAPTIC_OFF  PORTG &= ~(1<<2)
#define HAPTIC_ON   PORTG |=  (1<<2)
*/




//audio
#define AUDIO_QUEUE_LENGTH (20)  //20 seems to suit most alerts
#define AUDIO_QUEUE_FREESLOTS (3)  //free before we insert new sounds
#define BEEP_DEFAULT_FREQ  (70)
#define BEEP_OFFSET        (10)
#define BEEP_KEY_UP_FREQ   (BEEP_DEFAULT_FREQ+5)
#define BEEP_KEY_DOWN_FREQ (BEEP_DEFAULT_FREQ-5)

#define HAPTIC_SPINUP (25) //time haptic runs for to ensure motor starts up!
#define HAPTIC_QUEUE_LENGTH  4


/* the audio selection menu in the system config page */

// a line similar to this 
// "Warn1 ""Warn2 ""Warn3 ""Warn1 ""Cheap ""Ring  ""SciFi "....
// is needed in menu.cpp for the audo selection.
// these are directly related to the order of the audio items below.


/* make sure the defines below always go in numeric order */
#define AU_WARNING1 (0)
#define AU_WARNING2 (1)
#define AU_CHEEP (2)
#define AU_RING (3)
#define AU_SCIFI (4)
#define AU_ROBOT (5)
#define AU_CHIRP (6)
#define AU_TADA (7)
#define AU_CRICKET (8)
#define AU_SIREN (9)
#define AU_ALARMC (10)
#define AU_RATATA (11)
#define AU_TICK (12)
#define AU_HAPTIC1 (13)
#define AU_HAPTIC2 (14)
#define AU_HAPTIC3 (15)
// end of audio menu alerts.  
// not sure why but no more than 16 can be added?
// believe this is a menu size limitation
#define AU_INACTIVITY (16)
#define AU_TX_BATTERY_LOW (17)
#define AU_ERROR (18)
#define AU_KEYPAD_UP (19)
#define AU_KEYPAD_DOWN (20)
#define AU_TRIM_MOVE (21)
#define AU_TRIM_MIDDLE (22)
#define AU_MENUS (23)
#define AU_POT_STICK_MIDDLE (24)
#define AU_TIMER_30 (25)
#define AU_TIMER_20 (26)
#define AU_TIMER_10 (27)
#define AU_TIMER_LT3 (28)
#define AU_WARNING3 (29)
#define AU_MIX_WARNING_1 (30)
#define AU_MIX_WARNING_2 (31)
#define AU_MIX_WARNING_3 (32)
#define AU_VARIO_UP (33)
#define AU_VARIO_DOWN (34)
#define AU_LONG_TONE (35)

#define BEEP_QUIET (0)
#define BEEP_NOKEYS (1)
#define BEEP_XSHORT (2)
#define BEEP_SHORT (3)
#define BEEP_NORMAL (4)
#define BEEP_LONG (5)
#define BEEP_XLONG (6)

class audioQueue
{
  public:

    audioQueue();

    // only difference between these two functions is that one does the
    // interupt queue (Now) and the other queues for playing ASAP.
    void playNow(uint8_t tFreq, uint8_t tLen, uint8_t tPause, uint8_t tRepeat=0, uint8_t tHaptic=0, int8_t tFreqIncr=0);

    void playASAP(uint8_t tFreq, uint8_t tLen, uint8_t tPause, uint8_t tRepeat=0, uint8_t tHaptic=0, int8_t tFreqIncr=0);

    bool busy();

    void event(uint8_t e,uint8_t f=BEEP_DEFAULT_FREQ, uint8_t hapticOff = 0 ) ;



//inline void driver() {
//  if (toneTimeLeft > 0) {	
//					switch (g_eeGeneral.speakerMode){					
//								case 0:
//						        	//stock beeper. simply turn port on for x time!
//							        if (toneTimeLeft > 0){
//							            PORTE |=  (1<<OUT_E_BUZZER); // speaker output 'high'
//							        } 	
//							        break;	
//							  case 1:
//									    static uint8_t toneCounter;
//									    toneCounter += toneFreq;
//									    if ((toneCounter & 0x80) == 0x80) {
//									      PORTE |= (1 << OUT_E_BUZZER);
//									    } else {
//									      PORTE &= ~(1 << OUT_E_BUZZER);
//									    }							  	
//											break;						  	
//					}		
//	} else {
//			PORTE &=  ~(1<<OUT_E_BUZZER); // speaker output 'low'
//	}								  	     
//}	

    // heartbeat is responsibile for issueing the audio tones and general square waves
    // it is essentially the life of the class.
    void heartbeat();

    bool freeslots();

    //inline bool empty() {
    //  return (t_queueRidx == t_queueWidx);
    //}

  protected:
    void aqinit(); // To stop constructor being compiled twice
    inline uint8_t getToneLength(uint8_t tLen);

  private:

    uint8_t t_queueRidx;
    uint8_t t_queueWidx;

    uint8_t t_hapticQueueRidx ;
    uint8_t t_hapticQueueWidx ;
  	uint8_t buzzTimeLeft ;
  	uint8_t buzzPause ;

    uint8_t toneFreq;
    int8_t toneFreqIncr;
    uint8_t toneTimeLeft;
    uint8_t tonePause;
    

    // queue arrays
    uint8_t queueToneFreq[AUDIO_QUEUE_LENGTH];
    int8_t queueToneFreqIncr[AUDIO_QUEUE_LENGTH];
    uint8_t queueToneLength[AUDIO_QUEUE_LENGTH];
    uint8_t queueTonePause[AUDIO_QUEUE_LENGTH];
    uint8_t queueToneRepeat[AUDIO_QUEUE_LENGTH];

    uint8_t queueHapticLength[HAPTIC_QUEUE_LENGTH];
    uint8_t queueHapticPause[HAPTIC_QUEUE_LENGTH];
    uint8_t queueHapticRepeat[HAPTIC_QUEUE_LENGTH];

    uint8_t hapticMinRun;
//    uint8_t toneHaptic;
//    uint8_t hapticTick;
//    uint8_t queueToneHaptic[AUDIO_QUEUE_LENGTH];
	 // uint8_t toneCounter;

};

void beginToneFill( void ) ;
//int32_t xtoneFill( uint16_t *buffer, uint32_t frequency, uint32_t timeMs ) ;
int32_t toneFill( uint16_t *buffer ) ;

//wrapper function - dirty but results in a space saving!!!
extern audioQueue audio;

void audioDefevent(uint8_t e);
void audioVoiceDefevent( uint8_t e, uint8_t v) ;

#define AUDIO_KEYPAD_UP()   audioDefevent(AU_KEYPAD_UP)
#define AUDIO_KEYPAD_DOWN() audioDefevent(AU_KEYPAD_DOWN)
#define AUDIO_MENUS()       audioDefevent(AU_MENUS)
#define AUDIO_WARNING1()    audioDefevent(AU_WARNING1)
#define AUDIO_WARNING2()    audioDefevent(AU_WARNING2)
#define AUDIO_ERROR()       audioDefevent(AU_ERROR)

#define IS_AUDIO_BUSY()     audio.busy()

#define AUDIO_TIMER_30()    audioDefevent(AU_TIMER_30)
#define AUDIO_TIMER_20()    audioDefevent(AU_TIMER_20)
#define AUDIO_TIMER_10()    audioDefevent(AU_TIMER_10)
#define AUDIO_TIMER_LT3()   audioDefevent(AU_TIMER_LT3)
#define AUDIO_MINUTE_BEEP() audioDefevent(AU_WARNING1)
#define AUDIO_INACTIVITY()  audioDefevent(AU_INACTIVITY)
#define AUDIO_MIX_WARNING_1() audioDefevent(AU_MIX_WARNING_1)
#define AUDIO_MIX_WARNING_3() audioDefevent(AU_MIX_WARNING_3)


#define AUDIO_DRIVER()      audio.driver()
#define AUDIO_HEARTBEAT()   audio.heartbeat()

#define VOICE_Q_LENGTH		16

struct t_voice
{
	uint8_t VoiceQueueCount ;
	uint8_t VoiceQueueInIndex ;
	uint8_t VoiceQueueOutIndex ;
	uint8_t VoiceLock ;
	uint16_t VoiceQueue[VOICE_Q_LENGTH] ;
	uint8_t NamedVoiceQueue[VOICE_Q_LENGTH][VOICE_NAME_SIZE+1] ;
} ;

struct toneQentry
{
	uint8_t toneFreq ;
	int8_t toneFreqIncr ;
	uint8_t toneTimeLeft ;
	uint8_t tonePause ;
	uint8_t toneRepeat ;
} ;						    


extern struct t_voice Voice ;
extern struct toneQentry ToneQueue[] ;

extern void putVoiceQueue( uint16_t value ) ;
extern void putNamedVoiceQueue( const char *name, uint16_t value ) ;
extern void putSystemVoice( uint16_t sname, uint16_t value ) ;
extern void putUserVoice( char *name, uint16_t value ) ;
extern void voice_task(void* pdata) ;
extern bool ToneFreeSlots( void ) ;
extern void queueTone( uint8_t place, uint8_t freq, int8_t freqInc, uint8_t time, uint8_t pause, uint8_t repeat ) ;


// Defines for voice messages

#define	V_ZERO					0
#define	V_ONE						1
#define	V_TWO						2
#define	V_THREE					3

#define V_WARNING				21
#define V_ERROR					22
#define V_ALERT					23
#define V_FEET					24
#define V_FOOT					25
#define V_MINUS					26

#define	V_HELLO					28
#define V_LIMIT					29
#define V_RPM						30
#define V_FBATTLOW			31
#define V_POINT					35
#define V_VOLTS					36
#define V_VOLT					37
#define	V_MINUTES				38
#define	V_MINUTE				39
#define	V_FORTY					40

#define	V_30SECS				41
#define	V_20SECS				42
#define	V_10SECS				43


#define V_PERCENT				44
#define	V_INACTIVE			45
#define	V_BATTERY_LOW		46
#define	V_DEGREES				47
#define	V_DEGREE				48
#define	V_CAPACITY			49

#define V_AMPS					51
#define V_AMP						52
#define	V_SECONDS				53
#define	V_SECOND				54
#define	V_DB						55
#define	V_METRES				56
#define	V_METRE					57
#define	V_NOTELEM				58
#define	V_WATTS					59

#define V_RSSI_CRITICAL	71
#define V_RSSI_WARN			70

#define V_THR_WARN			74
#define V_SW_WARN				75

#define V_HUNDRED			 100
#define V_THOUSAND		 110

#if defined NMEA
  #define V_ALTITUDE				170
  #define V_DISTANCE				171
#endif


// Defines for system voice names
#define	SV_ALERT			 0
#define	SV_SW_WARN		 1
#define	SV_TH_WARN		 2
#define	SV_WARNING     3
#define	SV_ERROR       4
#define	SV_FEET        5
#define	SV_FOOT        6
#define	SV_MINUS       7
#define	SV_WELCOME     8
#define	SV_LIMIT       9
#define	SV_RPM        10
#define	SV_FLT_BATT   11
#define	SV_TX_VOLT    12
#define	SV_CURRENT    13
#define	SV_ALTITUDE   14
#define	SV_POINT      15
#define	SV_VOLTS      16
#define	SV_VOLT       17
#define	SV_MINUTES    18
#define	SV_MINUTE     19
#define	SV_PACKVOLT   20
#define	SV_30SECOND   21
#define	SV_20SECOND   22
#define	SV_10SECOND   23
#define	SV_PERCENT    24
#define	SV_INACTV     25
#define	SV_TXBATLOW   26
#define	SV_DEGREES    27
#define	SV_DEGREE     28
#define	SV_RX_VOLT    29
#define	SV_TEMPERAT   30
#define	SV_AMPS       31
#define	SV_AMP        32
#define	SV_SECONDS    33
#define	SV_SECOND     34
#define	SV_DB         35
#define	SV_METERS     36
#define	SV_METER      37
#define	SV_NO_TELEM   38
#define	SV_RX_V_LOW   39
#define	SV_TEMPWARN   40
#define	SV_ALT_WARN   41
#define	SV_WATT       42
#define	SV_WATTS      43
#define	SV_KNOT       44
#define	SV_KNOTS      45
#define	SV_MILLIAMP   46
#define	SV_MILIAMPS   47
#define	SV_MILAMP_H   48
#define	SV_MLAMPS_H   49
#define	SV_RSSI_LOW   50
#define	SV_RSSICRIT   51
#define	SV_RX_LOST    52
#define	SV_CAP_WARN   53


#endif // audio_h     
