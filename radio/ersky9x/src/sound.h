/****************************************************************************
*  Copyright (c) 2011 by Michael Blandford. All rights reserved.
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
****************************************************************************
*  History:
*
****************************************************************************/

#ifndef sound_h
#define sound_h

 #ifndef REVX
extern void write_coprocessor( uint8_t *ptr, uint32_t count ) ;
extern void read_coprocessor( void ) ;
extern uint8_t Coproc_read ;
extern int8_t Coproc_valid ;
#endif

 #ifdef REVX
extern void writeRTC( uint8_t *ptr ) ;
extern void readRTC( void ) ;
extern void setMFP( void ) ;
extern void clearMFP( void ) ;
extern void setRtcCAL( uint8_t value ) ;
#endif

extern void writeExtRtc( uint8_t *ptr ) ;

/* make sure the defines below always go in numeric order */
//#define AUDIO_TADA (0)
//#define AUDIO_WARNING1 (1)
//#define AUDIO_WARNING2 (2)
//#define AUDIO_WARNING3 (3)
//#define AUDIO_ERROR (4)
//#define AUDIO_KEYPAD_UP (5)
//#define AUDIO_KEYPAD_DOWN (6)
//#define AUDIO_TRIM_MOVE (7)
//#define AUDIO_TRIM_MIDDLE (8)
//#define AUDIO_MENUS (9)
//#define AUDIO_POT_STICK_MIDDLE (10)
//#define AUDIO_MIX_WARNING_1 (11)
//#define AUDIO_MIX_WARNING_2 (12)
//#define AUDIO_MIX_WARNING_3 (13)
//#define AUDIO_TIMER_30 (14)
//#define AUDIO_TIMER_20 (15)
//#define AUDIO_TIMER_10 (16)
//#define AUDIO_TIMER_LT3 (17)
//#define AUDIO_INACTIVITY (18)
//#define AUDIO_TX_BATTERY_LOW (19)


#define NUM_VOL_LEVELS	24

extern volatile uint8_t Buzzer_count ;


extern void start_sound( void ) ;
extern void buzzer_on( void ) ;
extern void buzzer_off( void ) ;
extern void buzzer_sound( uint8_t time ) ;
extern void set_frequency( uint32_t frequency ) ;
extern void start_dactimer( void ) ;
extern void init_dac( void ) ;
extern "C" void DAC_IRQHandler( void ) ;
extern void end_sound( void ) ;
extern void sound_5ms( void ) ;
extern void playTone( uint32_t frequency, uint32_t time ) ;
extern uint32_t queueTone( uint32_t frequency, uint32_t time, uint32_t frequency_increment, uint32_t lock ) ;
extern uint32_t unlockTone( void ) ;
extern void tone_start( register uint32_t time ) ;
extern void tone_stop( void ) ;
extern void init_twi( void ) ;
extern void setVolume( register uint8_t volume ) ;
extern "C" void TWI0_IRQHandler( void ) ;
extern void audioDefevent( uint8_t e ) ;
extern void hapticOff(void) ;
extern void hapticOn( uint32_t pwmPercent ) ;
void startVoice( uint32_t count ) ;			// count of filled in buffers
void appendVoice( uint32_t index ) ;		// index of next buffer
//extern void wavU8Convert( uint8_t *src, uint16_t *dest , uint32_t count ) ;
//extern void wavU16Convert( uint16_t *src, uint16_t *dest , uint32_t count ) ;
extern void endVoice( void ) ;
extern void setExternalOutput( uint8_t bit, uint8_t value ) ;

void init23008( void ) ;
void write23008( uint8_t outputs ) ;
void read23008( uint8_t *ptrData ) ;
void initLed( void ) ;
void writeLed( uint8_t value ) ;
void readLed( uint8_t *ptrData ) ;
uint32_t readI2cEncoder( uint8_t *ptrData ) ;
uint32_t hwreadI2cEncoder( uint8_t *ptrData ) ;

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
void initHaptic() ;
#endif

#ifdef REVX
extern void audioOn( void ) ;
extern void audioOff( void ) ;
#else
#define	audioOn()
#define	audioOff()
#endif

struct t_sound_globals
{
	uint32_t Next_freq ;
	volatile uint32_t Sound_time ;
	uint32_t Frequency ;
	volatile uint32_t Tone_timer ;		// Modified in interrupt routine
	uint32_t Frequency_increment ;
	uint32_t Next_frequency_increment ;
	volatile uint8_t Tone_ms_timer ;
	uint8_t toneLock ;
	uint8_t VoiceActive ;
	uint8_t VoiceRequest ;
//	uint8_t VoiceEnded ;
} ;


// Bits in t_VoiceBuffer.flags
#define VF_SENT			0x01
#define VF_LAST			0x02

#define	VOICE_BUFFER_SIZE		512
#define NUM_VOICE_BUFFERS		3

struct t_VoiceBuffer
{
	uint16_t count ;
	uint16_t flags ;
	uint32_t frequency ;
	union
	{
		uint16_t dataw[VOICE_BUFFER_SIZE] ;		// 46 mS at 11025 sample rate
		uint8_t datab[VOICE_BUFFER_SIZE*2] ;		// 46 mS at 11025 sample rate
	} ;
} ;

extern struct t_sound_globals Sound_g ;
extern struct t_VoiceBuffer VoiceBuffer[] ;

extern uint8_t AudioVoiceUnderrun ;

#endif
