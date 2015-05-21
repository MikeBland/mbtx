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

//#define HAPTIC_OFF  PORTG &= ~(1<<2)
//#define HAPTIC_ON   PORTG |=  (1<<2)

//audio
#define AUDIO_QUEUE_LENGTH (8)  //8 seems to suit most alerts
#define AUDIO_QUEUE_FREESLOTS (3)  //free before we insert new sounds
#define BEEP_DEFAULT_FREQ  (70)
#define BEEP_OFFSET        (10)
#define BEEP_KEY_UP_FREQ   (BEEP_DEFAULT_FREQ+5)
#define BEEP_KEY_DOWN_FREQ (BEEP_DEFAULT_FREQ-5)


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

    void play(uint8_t tFreq, uint8_t tLen, uint8_t tPause, uint8_t flags = 0 ) ;

    bool busy();

    void event(uint8_t e,uint8_t f=BEEP_DEFAULT_FREQ);




inline void driver() {
#ifndef SIMU
  if (toneTimeLeft > 0) {	
					switch ((g_eeGeneral.speakerMode & 1)){					 
								case 0:
						        	//stock beeper. simply turn port on for x time!
							        if (toneTimeLeft > 0){
							            PORTE |=  (1<<OUT_E_BUZZER); // speaker output 'high'
							        } 	 
							        break;	 
							  case 1:
									    static uint8_t toneCounter;
									    toneCounter += toneFreq;
									    if ((toneCounter & 0x80) == 0x80) {
									      PORTE |= (1 << OUT_E_BUZZER);
									    } else {
									      PORTE &= ~(1 << OUT_E_BUZZER);
									    }							  	 
											break;						  	 
					}		 
	} else {
			PORTE &=  ~(1<<OUT_E_BUZZER); // speaker output 'low'
	}								  	     
#endif
}	 

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

	  uint8_t toneHaptic;
	  uint8_t hapticTick;
	  uint8_t queueToneHaptic[AUDIO_QUEUE_LENGTH];
	 // uint8_t toneCounter;

};

//wrapper function - dirty but results in a space saving!!!
extern audioQueue audio;

void audioDefevent(uint8_t e);
void audioVoiceDefevent(uint8_t e, uint8_t v) ;
void audioEvent( uint8_t e, uint16_t f ) ;

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


#define PLAY_REPEAT(x)            (x)                 /* Range 0 to 15 */
#define PLAY_NOW                  0x10
#define PLAY_HAPTIC				        0x20
#define PLAY_INCREMENT(x)         ((uint8_t)(((uint8_t)x) << 6))   /* -1, 0, 1, 2 */


// Bits in VoiceLatch
#define VOICE_CLOCK_BIT		0x01
#define VOICE_DATA_BIT		0x02
#define BACKLIGHT_BIT			0x04
#define SPARE_BIT					0x08

#define VOICE_Q_LENGTH		16

// Voice states
#define V_STARTUP					0
#define V_IDLE						1
#define V_CLOCKING				2
#define V_WAIT_BUSY_ON		3
#define V_WAIT_BUSY_OFF		4
#define V_WAIT_BUSY_DELAY	5
#define V_WAIT_START_BUSY_OFF		6

#define V_WAIT_RX						0
#define V_RECEIVING_COUNT		1
#define V_RECEIVING_VALUE		2

struct t_voice
{
	uint16_t VoiceQueue[VOICE_Q_LENGTH] ;
	uint8_t Backlight ;
	uint8_t VoiceLatch ;
	uint8_t VoiceCounter ;
	uint8_t VoiceTimer ;
	uint16_t VoiceSerial ;
	uint8_t VoiceState ;
//	uint8_t VoiceShift ;
	uint8_t VoiceQueueCount ;
	uint8_t VoiceQueueInIndex ;
	uint8_t VoiceQueueOutIndex ;
	uint8_t VoiceSerialData[5] ;
	uint8_t VoiceSerialCount ;
	uint8_t VoiceSerialIndex ;
	uint8_t VoiceSerialRxState ;
	uint8_t VoiceSerialValue ;
	uint8_t VoiceBacklightCount ;
//	uint8_t VoiceDebug ;
	void voice_process( void ) ;
} ;

extern struct t_voice *voiceaddress( void ) ;
extern struct t_voice Voice ;


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

#define	V_RSSI_WARN			70
#define	V_RSSI_CRITICAL	71

#define	V_THR_WARN			74
#define	V_SW_WARN				75

#define V_HUNDRED			 100
#define V_THOUSAND		 110

#if defined NMEA
  #define V_ALTITUDE				170
  #define V_DISTANCE				171
#endif





#endif // audio_h
