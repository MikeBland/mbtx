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
#ifndef eeprom_h
#define eeprom_h

#include <stdint.h>

#ifndef PACK
#if __GNUC__
#define PACK( __Declaration__ )      __Declaration__ __attribute__((__packed__))
#else
#define PACK( __Declaration__ )      __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif
#endif

#define MULTI_PROTOCOL	1

//eeprom data
//#define EE_VERSION 2
#ifdef SKY
#define MAX_MODELS  32
#else
#define MAX_MODELS  16
#endif
#define MAX_MIXERS  32
#define MAX_SKYMIXERS  48
#define MAX_CURVE5  8
#define MAX_CURVE9  8
#define MDVERS_r9   1
#define MDVERS_r14  2
#define MDVERS_r22  3
#define MDVERS_r77  4
#define MDVERS_r85  5
#define MDVERS_r261 6
#define MDVERS_r352 7
#define MDVERS_r365 8
#define MDVERS_r668 9
#define MDVERS_r803 10
#ifdef V2
#define MDVERS      11
#else
#define MDVERS      11
#endif

#define MDSKYVERS   1

#define	NUM_VOICE		8

//OBSOLETE - USE ONLY MDVERS NOW
//#define GENERAL_MYVER_r261 3
//#define GENERAL_MYVER_r365 4
//#define GENERAL_MYVER      5


// eeprom ver <9 => mdvers == 1
// eeprom ver >9 => mdvers ==2

#define WARN_THR_BIT  0x01
#define WARN_BEP_BIT  0x80
#define WARN_SW_BIT   0x02
#define WARN_MEM_BIT  0x04
#define WARN_BVAL_BIT 0x38

#define WARN_THR     (!(g_eeGeneral.warnOpts & WARN_THR_BIT))
#define WARN_BEP     (!(g_eeGeneral.warnOpts & WARN_BEP_BIT))
#define WARN_SW      (!(g_eeGeneral.warnOpts & WARN_SW_BIT))
#define WARN_MEM     (!(g_eeGeneral.warnOpts & WARN_MEM_BIT))
#define BEEP_VAL     ( (g_eeGeneral.warnOpts & WARN_BVAL_BIT) >>3 )

// Bits in stickGain
#define STICK_LV_GAIN	0x40
#define STICK_LH_GAIN	0x10
#define STICK_RV_GAIN	0x04
#define STICK_RH_GAIN	0x01

#define GENERAL_OWNER_NAME_LEN 10
#define MODEL_NAME_LEN         10

#define MAX_GVARS 7
#define NUM_SCALERS	4

#ifdef SKY
#undef MAX_PHASES
#define MAX_PHASES		4
#else
#define MAX_PHASES		4
#define MAX_MODES		4
#endif

#define V2_NUM_VOICE_ALARMS	10
#define NUM_VOICE_ALARMS	8
#define NUM_SAFETY				16
#define NUM_GVAR_ADJUST		4

PACK(typedef struct t_TrainerMix {
  uint8_t srcChn:3; //0-7 = ch1-8
  int8_t  swtch:5;
  int8_t  studWeight:6;
  uint8_t mode:2;   //off,add-mode,subst-mode
}) TrainerMix; //

PACK(typedef struct t_TrainerData {
  int16_t        calib[4];
  TrainerMix     mix[4];
}) TrainerData;

PACK(typedef struct t_V2TrainerMix {
    uint8_t srcChn:4 ; //0-7 = ch1-8
    uint8_t mode:2 ;   //off,add-mode,subst-mode
    int8_t  swtch ;
    int8_t  studWeight ;
})  V2TrainerMix ;

PACK(typedef struct t_V2TrainerData {
    int16_t        calib[4];
    V2TrainerMix     mix[4];
}) V2TrainerData ;

#ifdef SKY
#define EE_GEN t_OldEEGeneral
#else
 #ifndef V2
  #define EE_GEN t_EEGeneral
 #endif
#endif

#ifdef SKY
#define EEGEN_BITFIELDS \
	uint8_t   FRSkyYellow:4;\
  uint8_t   FRSkyOrange:4;\
  uint8_t   FRSkyRed:4;\
  uint8_t   hideNameOnSplash:1;\
  uint8_t   optrexDisplay:1;\
	uint8_t   unexpectedShutdown:1;\
  uint8_t   spare:1
#else
// #ifdef XSW_MOD
//  uint8_t   pb7backlight:1 ;      // valid only if (!(speakerMode & 2) || MegasoundSerial)
//  uint8_t   LVTrimMod:1;          // LV trim rerouted to (UP:PC0,DN:PC4)
// #define EEGEN_BITFIELDS
//	uint8_t   unused1;
//  uint8_t   unused2:2;
//  uint8_t   pb7backlight:1 ;
//  uint8_t   LVTrimMod:1;
//  uint8_t   hideNameOnSplash:1;
//  uint8_t   enablePpmsim:1;
//  uint8_t   blightinv:1;
//  uint8_t   stickScroll:1
// #else
 #define EEGEN_BITFIELDS \
	uint8_t   unused1;\
  uint8_t   unused2:4;\
  uint8_t   hideNameOnSplash:1;\
  uint8_t   enablePpmsim:1;\
  uint8_t   blightinv:1;\
  uint8_t   stickScroll:1
// #endif
#endif

#ifdef XSW_MOD
 #define SWITCH_MAPPING \
    uint8_t   switchSources[(MAX_XSWITCH+1)/2] // packed nibble array [0..(MAX_XSWITCH-1)]
		                                           // wasted 4b if MAX_XSWITCH is an odd number
#else
 #define SWITCH_MAPPING \
    uint8_t   switchMapping ;\
		uint8_t	ele2source:4 ;\
		uint8_t	ail2source:4 ;\
		uint8_t	rud2source:4 ;\
		uint8_t	gea2source:4 ;\
		uint8_t pb1source:4 ;\
		uint8_t pb2source:4 ;\
    uint8_t pg2Input:1 ;\
    uint8_t pb7Input:1 ;\
    uint8_t lcd_wrInput:1 ;\
    uint8_t  spare5:5 ;\
    uint8_t  exSwitchWarningStates
#endif
		 
PACK(typedef struct EE_GEN {
    uint8_t   myVers;
    int16_t   calibMid[7];
    int16_t   calibSpanNeg[7];
    int16_t   calibSpanPos[7];
    uint16_t  chkSum;
    uint8_t   currModel; //0..15
    uint8_t   contrast;
    uint8_t   vBatWarn;
    int8_t    vBatCalib;
    int8_t    lightSw;
    TrainerData trainer;
    uint8_t   view;
    uint8_t   disableThrottleWarning:1;
    uint8_t   disableSwitchWarning:1;
    uint8_t   disableMemoryWarning:1;
    uint8_t   beeperVal:3;
    uint8_t   reserveWarning:1;
    uint8_t   disableAlarmWarning:1;
    uint8_t   stickMode;
    int8_t    inactivityTimer;
    uint8_t   throttleReversed:1;
    uint8_t   minuteBeep:1;
    uint8_t   preBeep:1;
    uint8_t   flashBeep:1;
    uint8_t   disableSplashScreen:1;
    uint8_t   disablePotScroll:1;
    uint8_t   disableBG:1;
    uint8_t   frskyinternalalarm:1;
    uint8_t   spare_filter;
    uint8_t   lightAutoOff;
    uint8_t   templateSetup;  //RETA order according to chout_ar array
    int8_t    PPM_Multiplier;
		EEGEN_BITFIELDS ;
//#ifdef SKY
//		uint8_t   FRSkyYellow:4;
//    uint8_t   FRSkyOrange:4;
//    uint8_t   FRSkyRed:4;
//#else
//		uint8_t   unused1;
//#ifdef XSW_MOD
//    uint8_t   unused2:2;
//   	uint8_t   pb7backlight:1 ;      // valid only if (!(speakerMode & 2) || MegasoundSerial)
//    uint8_t   LVTrimMod:1;          // LV trim rerouted to (UP:PC0,DN:PC4)
//#else
//    uint8_t   unused2:4;
//#endif
//#endif
//    uint8_t   hideNameOnSplash:1;
//#ifdef SKY
//  	uint8_t   optrexDisplay:1;
//	  uint8_t   unexpectedShutdown:1;
//    uint8_t   spare:1;
//#else
//    uint8_t   enablePpmsim:1;
//    uint8_t   blightinv:1;
//    uint8_t   stickScroll:1;
//#endif
    uint8_t   speakerPitch;
    uint8_t   hapticStrength;
    uint8_t   speakerMode;
    uint8_t   lightOnStickMove;
    char      ownerName[GENERAL_OWNER_NAME_LEN];
#ifdef XSW_MOD
    uint16_t  switchWarningStates;
#else
    uint8_t   switchWarningStates;
#endif
		int8_t		volume ;
#ifdef SKY
	uint8_t 	bright ;			// backlight
  uint8_t   stickGain;
	uint8_t		mAh_alarm ;
	uint16_t	mAh_used ;
	uint16_t	run_time ;
	int8_t		current_calib ;
	uint8_t		bt_baudrate ;
	uint8_t		rotaryDivisor ;
	uint8_t   crosstrim:1;
	uint8_t   spare9:7;
#else
		uint8_t   res[3];
    uint8_t   crosstrim:1;
    uint8_t   FrskyPins:1 ;
    uint8_t   rotateScreen:1 ;
    uint8_t   serialLCD:1 ;
    uint8_t   SSD1306:1 ;
    uint8_t   TEZr90:1 ;
    uint8_t   MegasoundSerial:1 ;
    uint8_t   spare1:1 ;
		uint8_t		stickReverse ;
#endif
		uint8_t		customStickNames[16] ;
		SWITCH_MAPPING ;
//#ifdef XSW_MOD
//    uint8_t   switchSources[(MAX_XSWITCH+1)/2]; // packed nibble array [0..(MAX_XSWITCH-1)]
//#else                                           // wasted 4b if MAX_XSWITCH is an odd number
//    uint8_t   switchMapping ;
//		uint8_t	ele2source:4 ;
//		uint8_t	ail2source:4 ;
//		uint8_t	rud2source:4 ;
//		uint8_t	gea2source:4 ;
//		uint8_t pb1source:4 ;
//		uint8_t pb2source:4 ;
//    uint8_t pg2Input:1 ;
//    uint8_t pb7Input:1 ;
//    uint8_t lcd_wrInput:1 ;
//    uint8_t  spare5:5 ;
//    uint8_t  exSwitchWarningStates ;
//#endif
}) V1EEGeneral;


PACK(typedef struct t_V2EEGeneral {
    uint8_t   myVers;
    int16_t   calibMid[7];
    int16_t   calibSpanNeg[7];
    int16_t   calibSpanPos[7];
    uint16_t  chkSum;
    uint8_t   currModel; //0..15
    uint8_t   contrast;
    uint8_t   vBatWarn;
    int8_t    vBatCalib;
    int8_t    lightSw;
    V2TrainerData trainer;
    uint8_t   view;
    uint8_t   disableThrottleWarning:1;
    uint8_t   disableSwitchWarning:1;
    uint8_t   disableMemoryWarning:1;
    uint8_t   beeperVal:3;
    uint8_t   unused_reserveWarning:1;
    uint8_t   disableAlarmWarning:1;
    uint8_t   stickMode;
    int8_t    inactivityTimer;
    uint8_t   throttleReversed:1;
    uint8_t   unused_minuteBeep:1;
    uint8_t   unused_preBeep:1;
    uint8_t   flashBeep:1;
    uint8_t   disableSplashScreen:1;
    uint8_t   disablePotScroll:1;
    uint8_t   disableBG:1;
    uint8_t   unused_frskyinternalalarm:1;		// Not used if no FRSKY_ALARMS
    uint8_t   unused_spare_filter ;		// No longer needed, left for eepe compatibility for now
    uint8_t   lightAutoOff;
    uint8_t   templateSetup;  //RETA order according to chout_ar array
    int8_t    PPM_Multiplier;
//    uint8_t   unused1;
#ifdef XSW_MOD
    uint8_t   unused2:2;
   	uint8_t   pb7backlight:1 ;      // valid only if (!(speakerMode & 2) || MegasoundSerial)
    uint8_t   LVTrimMod:1;          // LV trim rerouted to (UP:PC0,DN:PC4)
#else
    uint8_t   unused2:4;
#endif
    uint8_t   hideNameOnSplash:1;
    uint8_t   enablePpmsim:1;
    uint8_t   blightinv:1;
    uint8_t   stickScroll:1;
    uint8_t   speakerPitch;
    uint8_t   hapticStrength;
    uint8_t   speakerMode;
    uint8_t   lightOnStickMove;
    char      ownerName[GENERAL_OWNER_NAME_LEN];
//    uint8_t   switchWarningStates;
		int8_t		volume ;
//    uint8_t   res[3];
    uint8_t   crosstrim:1 ;
    uint8_t   FrskyPins:1 ;
    uint8_t   rotateScreen:1 ;
    uint8_t   serialLCD:1 ;
    uint8_t   SSD1306:1 ;
    uint8_t   TEZr90:1 ;
    uint8_t   MegasoundSerial:1 ;
    uint8_t   spare1:1 ;
		uint8_t		stickReverse ;
		uint8_t		customStickNames[16] ;
#ifdef XSW_MOD
    uint8_t   switchSources[(MAX_XSWITCH+1)/2]; // packed nibble array [0..(MAX_XSWITCH-1)]
#else                                           // wasted 4b if MAX_XSWITCH is an odd number
    uint8_t   switchMapping ;
		uint8_t	ele2source:4 ;
		uint8_t	ail2source:4 ;
		uint8_t	rud2source:4 ;
		uint8_t	gea2source:4 ;
		uint8_t pb1source:4 ;
		uint8_t pb2source:4 ;
    uint8_t pg2Input:1 ;
    uint8_t pb7Input:1 ;
    uint8_t lcd_wrInput:1 ;
    uint8_t  spare5:5 ;
#endif
}) V2EEGeneral;


//eeprom modelspec
//expo[3][2][2] //[Norm/Dr][expo/weight][R/L]

PACK(typedef struct t_ExpoData {
  int8_t  expo[3][2][2];
  int8_t  drSw1;
  int8_t  drSw2;
}) ExpoData;


PACK(typedef struct t_LimitData {
  int8_t  min;
  int8_t  max;
  bool    revert;
  int16_t  offset;
}) LimitData;

#define MLTPX_ADD  0
#define MLTPX_MUL  1
#define MLTPX_REP  2

PACK(typedef struct t_MixData {
  uint8_t destCh;            //        1..NUM_CHNOUT
  uint8_t srcRaw;            //
  int8_t  weight;
  int8_t  swtch;
  int8_t	curve;             //0=symmetrisch 1=no neg 2=no pos
  uint8_t delayUp:4;
  uint8_t delayDown:4;
  uint8_t speedUp:4;         // Servogeschwindigkeit aus Tabelle (10ms Cycle)
  uint8_t speedDown:4;       // 0 nichts
  uint8_t carryTrim:1;
  uint8_t mltpx:2;           // multiplex method 0=+ 1=* 2=replace
#ifdef V2
  uint8_t hiResSlow:1 ;
//  uint8_t lateOffset:1 ;
#else
  uint8_t lateOffset:1 ;
#endif
  uint8_t mixWarn:2;         // mixer warning
  uint8_t disableExpoDr:1;
  uint8_t differential:1;
  int8_t  sOffset;
	uint8_t modeControl:5 ;
  uint8_t sw23pos:3 ;
}) MixData;


PACK(typedef struct t_CSwData { // Custom Switches data
  int8_t  v1; //input
  int8_t  v2; 		//offset
  uint8_t func:4;
  uint8_t andsw:4;
}) CSwData;

PACK(typedef struct t_CxSwData { // Extra Custom Switches data
    int8_t  v1; //input
    int8_t  v2; //offset
    uint8_t func ;
    int8_t andsw ;
}) CxSwData ;

PACK(typedef struct t_SafetySwData { // Custom Switches data
	union opt
	{
		struct ss
		{	
	    int8_t  swtch:6;
			uint8_t mode:2;
    	int8_t  val;
		} ss ;
		struct vs
		{
  		uint8_t vswtch:5 ;
			uint8_t vmode:3 ; // ON, OFF, BOTH, 15Secs, 30Secs, 60Secs, Varibl
    	uint8_t vval;
		} vs ;
	} opt ;
}) SafetySwData;

PACK(typedef struct t_VoiceSwData
{
  int8_t swtch ;
	uint8_t mode ; // ON, OFF, BOTH, ALL, ONCE
  uint8_t val ;
}) VoiceSwData ;


PACK(typedef struct t_V2SafetySwData
{ // Safety Switches data
	int8_t  swtch:7 ;
	uint8_t mode:1 ;	// 'S', 'X'
 	int8_t  val ;
}) V2SafetySwData ;

PACK(typedef struct t_FrSkyChannelData {
  uint8_t   ratio;                // 0.0 means not used, 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
  uint8_t   alarms_value[2];      // 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
  uint8_t   alarms_level:4;
  uint8_t   alarms_greater:2;     // 0=LT(<), 1=GT(>)
  uint8_t   type:2;               // 0=volts, 1=raw, 2=volts*2, 3=Amps
}) FrSkyChannelData;

//PACK(typedef struct t_FrSkyData {
//    FrSkyChannelData channels[2];
//}) FrSkyData;

PACK(typedef struct t_V2FrSkyChannelData
{
 	uint16_t   ratio ;                // 0.0 means not used, 0.1V steps EG. 6.6 Volts = 66. 25.1V = 251, etc.
	uint8_t unit ;
}) V2FrSkyChannelData;

PACK(typedef struct t_FrSkyalarms
{
	uint8_t frskyAlarmType ;
	uint8_t frskyAlarmLimit ;
	uint8_t frskyAlarmSound ;
}) FrSkyAlarmData;

PACK(typedef struct t_FrSkyData {
    FrSkyChannelData channels[2];
		uint8_t voltSource:2 ;
		uint8_t ampSource:2 ;
		uint8_t FASoffset:4 ;			// 0.0 to 1.5
		uint8_t frskyAlarmLimit ;
		uint8_t frskyAlarmSound ;
//		FrSkyAlarmData alarmData[4] ;
}) FrSkyData;

PACK(typedef struct t_V2FrSkyData {
    V2FrSkyChannelData channels[2];
		uint8_t FASoffset ;			// 0.0 to 1.5
		int8_t rxRssiLow ;
		int8_t rxRssiCritical ;
//		uint8_t frskyAlarmLimit ;		// For mAh
//		uint8_t frskyAlarmSound ;		// For mAh
//		FrSkyAlarmData alarmData[4] ;
}) V2FrSkyData;

PACK(typedef struct t_gvar {
	int8_t gvar ;
	uint8_t gvsource ;
//	int8_t gvswitch ;
}) GvarData ;

PACK(typedef struct t_V2gvar {
	int8_t gvar ;
	uint8_t gvsource ;
	int8_t gvswitch ;
}) V2GvarData ;

PACK(typedef struct t_PhaseData {
	// Trim store as -1001 to -1, trim value-501, 0-5 use trim of phase 0-5
  int16_t trim[4];     // -500..500 => trim value, 501 => use trim of phase 0, 502, 503, 504 => use trim of modes 1|2|3|4 instead
  int8_t swtch;        // Try 0-5 use trim of phase 0-5, 1000-2000, trim + 1500 ???
  uint8_t fadeIn:4;
  uint8_t fadeOut:4;
//	uint8_t name[6] ;
}) PhaseData;

PACK(typedef struct t_V2PhaseData {
	// Trim store as -1001 to -1, trim value-501, 0-5 use trim of phase 0-5
  int16_t trim[4];     // -500..500 => trim value, 501 => use trim of phase 0, 502, 503, 504 => use trim of modes 1|2|3|4 instead
  int8_t swtch;        // Try 0-5 use trim of phase 0-5, 1000-2000, trim + 1500 ???
  uint8_t fadeIn:4;
  uint8_t fadeOut:4;
	uint8_t name[6] ;
}) V2PhaseData;
	 
PACK(typedef struct t_FunctionData { // Function data
  int8_t  swtch ; //input
  uint8_t func ;
  uint8_t param ;
}) FunctionData ;

PACK(typedef struct t_Vario
{
  uint8_t varioSource ;
  int8_t  swtch ;
  uint8_t sinkTonesOff:1 ;
  uint8_t spare:1 ;
  uint8_t param:6 ;
}) VarioData ;

// Scale a value
PACK(typedef struct t_scale
{
  uint8_t source ;
	int16_t offset ;
	uint8_t mult ;
	uint8_t div ;
	uint8_t unit ;
	uint8_t neg:1 ;
	uint8_t precision:2 ;
	uint8_t offsetLast:1 ;
	uint8_t spare:4 ;
	uint8_t name[4] ;
}) ScaleData ;

PACK(typedef struct t_extScale
{
	uint8_t mod ;
	uint8_t dest ;
} ) ExtScaleData ;

PACK(typedef struct t_voiceAlarm
{
  uint8_t source ;
	uint8_t func;
  int8_t  swtch ;
	uint8_t rate ;
	uint8_t fnameType:3 ;
	uint8_t haptic:2 ;
	uint8_t vsource:2 ;
	uint8_t mute:1 ;
//	uint8_t res1 ;			// Spare for expansion
  int16_t  offset ;		//offset
//	union
//	{
		int16_t vfile ;
//		uint8_t name[8] ;
//	} file ;
}) VoiceAlarmData ;

typedef struct t_gvarAdjust
{
	uint8_t function:4 ;
	uint8_t gvarIndex:4 ;
	int8_t swtch ;
	int8_t switch_value ;
} GvarAdjust ;

typedef struct t_TimerMode
{
  uint16_t  tmrVal ;
	uint8_t   tmrModeA ;          // timer trigger source -> off, abs, stk, stk%, cx%
  int8_t    tmrModeB ;           // timer trigger source -> !sw, none, sw, m_sw
//	int8_t		tmrRstSw ;
  uint8_t   tmrDir ;					// Timer direction
//  uint8_t   tmrDir:1 ;					// Timer direction
//	uint8_t 	tmrCdown:1 ;
//	uint8_t 	tmrMbeep:1 ;
} __attribute__((packed)) TimerMode ;

typedef struct t_V2TimerMode
{
  uint16_t  tmrVal ;
	uint8_t   tmrModeA ;          // timer trigger source -> off, abs, stk, stk%, cx%
  int8_t    tmrModeB ;           // timer trigger source -> !sw, none, sw, m_sw
	int8_t		tmrRstSw ;
  uint8_t   tmrDir ;					// Timer direction
	uint8_t 	tmrCdown:1 ;
	uint8_t 	tmrMbeep:1 ;
} __attribute__((packed)) V2TimerMode ;


typedef struct t_V2Ppm
{
  int8_t    ppmNCH;						// Also RxNum
  int8_t    ppmDelay;
  int8_t    ppmFrameLength;    //0=22.5  (10msec-30msec) 0.5msec increments
} __attribute__((packed)) V2Ppm ;

typedef struct t_V2Pxx
{
  uint8_t RxNum ;
  uint8_t sub_protocol ;		// sub_protocol
  uint8_t country ;
} __attribute__((packed)) V2Pxx ;

typedef struct t_V2Dsm
{
  uint8_t RxNum ;
  uint8_t sub_protocol ;		// sub_protocol
} __attribute__((packed)) V2Dsm ;

typedef struct t_V2Multi
{
  uint8_t RxNum:4 ;
  uint8_t sub_sub_protpcol:4 ;
  uint8_t sub_protocol ;		// sub_protocol
	uint8_t option_protocol;		// Option byte for MULTI protocol
} __attribute__((packed)) V2Multi ;


typedef struct t_V2Protocol
{
	uint8_t protocol:4 ;
	uint8_t ppmStart:4 ;					// Start channel for PPM
	union
	{
		V2Ppm ppm ;
		V2Pxx pxx ;
		V2Dsm dsm ;
		V2Multi multi ;
	} ;
} __attribute__((packed)) V2Protocol ;


typedef struct t_ModelData {
  char      name[MODEL_NAME_LEN];             // 10 must be first for eeLoadModelName
//    uint8_t   reserved_spare;  //used to be MDVERS - now depreciated
  uint8_t   modelVoice ;		// Index to model name voice (261+value)
  int8_t    tmrMode;              // timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
  uint8_t   tmrDir:1;    //0=>Count Down, 1=>Count Up
  uint8_t   traineron:1;  // 0 disable trainer, 1 allow trainer
  uint8_t   xt2throttle:1 ;  // Start timer2 using throttle
  uint8_t   FrSkyUsrProto:1 ;  // Protocol in FrSky User Data, 0=FrSky Hub, 1=WS HowHigh
  uint8_t   FrSkyGpsAlt:1 ;  	// Use Gps Altitude as main altitude reading
  uint8_t   FrSkyImperial:1 ;  // Convert FrSky values to imperial units
  uint8_t   FrSkyAltAlarm:2;
  uint16_t  tmrVal;
  uint8_t   protocol:4 ;
  uint8_t   country:2 ;
#ifdef MULTI_PROTOCOL
    uint8_t   xsub_protocol:2 ;		// sub_protocol is the extended version
#else
    uint8_t   sub_protocol:2 ;		// sub_protocol
#endif // MULTI_PROTOCOL
  int8_t    ppmNCH;
  uint8_t   thrTrim:1;            // Enable Throttle Trim
  uint8_t   xnumBlades:2;					// RPM scaling
	uint8_t   mixTime:1 ;						// Scaling for slow/delay
  uint8_t   thrExpo:1;            // Enable Throttle Expo
	uint8_t   ppmStart:3 ;					// Start channel for PPM
  int8_t    trimInc;              // Trim Increments
  int8_t    ppmDelay;
  int8_t    trimSw;
  uint8_t   beepANACenter;        // 1<<0->A1.. 1<<6->A7
  uint8_t   pulsePol:1;
  uint8_t   extendedLimits:1;
  uint8_t   swashInvertELE:1;
  uint8_t   swashInvertAIL:1;
  uint8_t   swashInvertCOL:1;
  uint8_t   swashType:3;
  uint8_t   swashCollectiveSource;
  uint8_t   swashRingValue;
  int8_t    ppmFrameLength;    //0=22.5  (10msec-30msec) 0.5msec increments
  MixData   mixData[MAX_MIXERS];
  LimitData limitData[NUM_CHNOUT];
  ExpoData  expoData[4];
  int8_t    trim[4];
  int8_t    curves5[MAX_CURVE5][5];
  int8_t    curves9[MAX_CURVE9][9];
  CSwData   customSw[NUM_CSW];
  uint8_t   frSkyVoltThreshold ;
  int8_t    tmrModeB;
  uint8_t   numVoice:5;		// 0-16, rest are Safety switches
	uint8_t		anaVolume:3 ;	// analog volume control
  SafetySwData  safetySw[NUM_CHNOUT];
  FrSkyData frsky;
	uint8_t numBlades ;
	uint8_t frskyoffset[2] ;		// Offsets for A1 and A2
  uint16_t  tmr2Val;
  int8_t    tmr2Mode;              // timer2 trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
  int8_t		tmr2ModeB;
//#ifdef XSW_MOD
//		uint16_t switchWarningStates ;
//#else
	uint8_t switchWarningStates ;
//#endif
	uint8_t sub_trim_limit ;
	uint8_t CustomDisplayIndex[6] ;
	GvarData gvars[MAX_GVARS] ;
	PhaseData phaseData[MAX_PHASES] ;
	VarioData varioData ;
	uint8_t modelVersion ;
#ifdef MULTI_PROTOCOL
	int8_t sub_protocol;		// Extending sub_protocol values for MULTI protocol
	int8_t option_protocol;		// Option byte for MULTI protocol
	uint8_t telemetryProtocol ;
	int8_t pxxFailsafe[13];		// Currently unused
#else
	int8_t sparepxxFailsafe[2];		// Currently unused
	uint8_t telemetryProtocol ;
	int8_t pxxFailsafe[13];		// Currently unused
#endif // MULTI_PROTOCOL
	SafetySwData xvoiceSw[EXTRA_VOICE_SW] ;
  CxSwData xcustomSw[EXTRA_CSW];
	uint8_t   currentSource ;
	uint8_t   altSource ;
  uint8_t phaseNames[MAX_PHASES][6] ;
	ScaleData Scalers[NUM_SCALERS] ;
	int8_t timer1RstSw ;
	int8_t timer2RstSw ;
	uint8_t timer1Cdown:1 ;
	uint8_t timer2Cdown:1 ;
	uint8_t timer1Mbeep:1 ;
	uint8_t timer2Mbeep:1 ;
	uint8_t useCustomStickNames:1 ;
	uint8_t throttleIdle:1 ;
  uint8_t throttleReversed:1;
  uint8_t tmr2Dir:1;    //0=>Count Down, 1=>Count Up
	int8_t gvswitch[MAX_GVARS] ;
  VoiceAlarmData vad[NUM_VOICE_ALARMS] ;
//#ifndef XSW_MOD
  uint8_t  exSwitchWarningStates ;
//#endif
  int8_t    curvexy[18] ;
} __attribute__((packed)) V1ModelData;

PACK(typedef struct t_V2ModelData
{
  char      name[MODEL_NAME_LEN];             // 10 must be first for eeLoadModelName
  uint8_t   modelVoice ;		// Index to model name voice (260+value)
	V2TimerMode	timer[2] ;
  uint8_t   unused_tmrDir:1;    //0=>Count Down, 1=>Count Up
  uint8_t   traineron:1;  // 0 disable trainer, 1 allow trainer
  uint8_t   unused_xt2throttle:1 ;  // Start timer2 using throttle
  uint8_t   FrSkyUsrProto:1 ;  // Protocol in FrSky User Data, 0=FrSky Hub, 1=WS HowHigh
  uint8_t   FrSkyGpsAlt:1 ;  	// Use Gps Altitude as main altitude reading
  uint8_t   FrSkyImperial:1 ;  // Convert FrSky values to imperial units
  uint8_t   unused_FrSkyAltAlarm:2;
  uint8_t   protocol:4 ;
  uint8_t   country:2 ;
  uint8_t   unused_xsub_protocol:2 ;	// sub_protocol is the extended version
  uint8_t   sub_protocol ;						// sub_protocol
	int8_t		option_protocol;		// Option byte for MULTI protocol
  int8_t    ppmNCH;						// Also RxNum
  int8_t    ppmFrameLength;    //0=22.5  (10msec-30msec) 0.5msec increments
  int8_t    ppmDelay;
  
	uint8_t   thrTrim:1;            // Enable Throttle Trim
	uint8_t   unused_xnumBlades:2;					// RPM scaling
	uint8_t   unused_mixTime:1 ;		// Scaling for slow/delay
  uint8_t   thrExpo:1;            // Enable Throttle Expo
	uint8_t   ppmStart:3 ;					// Start channel for PPM
  uint8_t   pulsePol:1;
  uint8_t   extendedLimits:1;
  uint8_t   swashInvertELE:1;
  uint8_t   swashInvertAIL:1;
  uint8_t   swashInvertCOL:1;
  uint8_t   swashType:3;
  uint8_t   swashCollectiveSource;
  uint8_t   swashRingValue;
  MixData   mixData[MAX_MIXERS];
  LimitData limitData[NUM_CHNOUT];
  ExpoData  expoData[4];
  int8_t    trim[4];
  int8_t    curves5[MAX_CURVE5][5];
  int8_t    curves9[MAX_CURVE9][9];
  CxSwData	customSw[NUM_CSW+EXTRA_CSW];
	VarioData varioData ;
  int8_t  trimInc;              // Trim Increments (0-4)
  int8_t  trimSw;
	uint8_t beepANACenter;        // 1<<0->A1.. 1<<6->A7
	uint8_t numBlades ;
	V2GvarData gvars[MAX_GVARS] ;
	V2SafetySwData safetySw[NUM_SAFETY];
	uint8_t CustomDisplayIndex[2][6] ;

	uint8_t modelVersion ;		// Keep at offset 0x2F0 from start of structure!
  
  V2FrSkyData frsky;
#ifdef XSW_MOD
	uint16_t switchWarningStates ;
#else
	uint8_t switchWarningStates ;
  uint8_t exSwitchWarningStates ;	// May combine as 16 bit value
#endif
	uint8_t sub_trim_limit ;
	uint8_t	customStickNames[16] ;
  V2PhaseData phaseData[MAX_PHASES] ;
	VoiceSwData voiceSw[EXTRA_VOICE_SW] ;
	ScaleData Scalers[NUM_SCALERS] ;
	uint8_t	anaVolume ;	// analog volume control
	uint8_t  currentSource ;
	uint8_t useCustomStickNames:2 ;
	uint8_t throttleIdle:1 ;
  uint8_t throttleReversed:1;
  GvarAdjust gvarAdjuster[NUM_GVAR_ADJUST] ;
	VoiceAlarmData vad[V2_NUM_VOICE_ALARMS] ;
//	int8_t unused_pxxFailsafe[16] ;	// Currently unused
	uint8_t telemetryProtocol ;
  int8_t    curvexy[18] ;
	ExtScaleData eScalers[NUM_SCALERS] ;
}) V2ModelData ;



#define TOTAL_EEPROM_USAGE (sizeof(ModelData)*MAX_MODELS + sizeof(EEGeneral))


//extern EEGeneral g_eeGeneral;
//extern ModelData g_model;







#ifdef SKY
#undef MAX_PHASES
#define MAX_PHASES		6
#endif






#endif
/*eof*/

