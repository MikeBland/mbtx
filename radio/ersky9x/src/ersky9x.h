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

// This version for ARM based ERSKY9X board

#ifndef ersky9x_h
#define ersky9x_h

#include <stdint.h>

//#define FIX_MODE		1

#define ASSAN		1

#ifdef PCBX9D
//#define	SERIAL_HOST		1
#endif

#define TELEMETRY_LOST	1

//#define ENABLE_DSM_MATCH	1

#ifdef SIMU
#include "simpgmspace.h"
#else
// Items set to make things compile, will need to be sorted eventually
#define assert(x)
#ifdef PCBSKY
#define wdt_reset()	(WDT->WDT_CR = 0xA5000001)
#endif
#if defined(PCBX9D) || defined(PCB9XT)
#define wdt_reset()	(IWDG->KR = 0x0000AAAAL)
#endif
#endif

#ifdef PCBDUE
#define VERSION	"DUE-V1.00"
#else
#define VERSION	"V1.00"
#define VERSION9XT	"9xt-0.30"
#endif

#define GVARS		1

//#define DATE_STR "xx.xx.2012"
//#define TIME_STR "xx:xx:xx"
//#define SUB_VERS VERSION
//#define SVN_VERS "trunk-rxxx"
//#define MOD_VERS "Normal"

//#define STR2(s) #s
//#define DEFNUMSTR(s)  STR2(s)

//extern const char stamp1[] ;
//extern const char stamp2[] ;
//extern const char stamp3[] ;
//extern const char stamp4[] ;
//extern const char stamp5[] ;
extern const char Stamps[] ;

//extern const char *Str_OFF ;
//extern const char *Str_ON ;
//extern const char *Str_Switches ;
//extern const char *Str_Switch_warn ;

#define CPU_INT		int32_t
#define CPU_UINT	uint32_t

#define BITMASK(bit) (1<<(bit))

extern const char * const *Language ;
extern const char * const English[] ;
extern const char * const French[] ;
extern const char * const German[] ;
extern const char * const Norwegian[] ;
extern const char * const Swedish[] ;
extern const char * const Italian[] ;

#define PSTR(a) Language[a]
#define XPSTR(a)  (char *)a

#define PROGMEM	 const unsigned char
#define strcpy_P(a,b)	strcpy(a,b)
#define strncpy_P(a,b,c)	strncpy(a,b,c)
#define pgm_read_byte(p)	(*(p))


#ifdef PCBX9D
 #ifdef REVPLUS
	#ifdef REV9E
  #define NUMBER_ANALOG		10
  #else
  #define NUMBER_ANALOG		10
	#endif	// REV9E
 #else
  #define NUMBER_ANALOG		9
 #endif
#else // not PCBX9D
#ifdef REVX
#define NUMBER_ANALOG		10
#define CURRENT_ANALOG	8
#else
 #ifdef REVB
 #define NUMBER_ANALOG		10
 #define CURRENT_ANALOG	8
 #else
  #ifdef PCB9XT
   #define NUMBER_ANALOG	11
	 #define NUM_REMOTE_ANALOG	3
  #else
   #define NUMBER_ANALOG	8
  #endif
 #endif
#endif
#endif

#ifdef REV9E
#define NUM_EXTRA_ANALOG		3
#else
#define NUM_EXTRA_ANALOG		0
#endif	// REV9E

#ifndef NUM_REMOTE_ANALOG
 #define NUM_REMOTE_ANALOG	0
#endif

extern uint8_t SystemOptions ;

//#define SWITCHES_STR "THRRUDELEID0ID1ID2AILGEATRNSW1SW2SW3SW4SW5SW6SW7SW8SW9SWASWBSWCSWDSWESWFSWGSWHSWISWJSWKSWLSWMSWNSWO"
#define NUM_CSW  12 //number of custom switches
#define NUM_SKYCSW  24 //number of custom switches
#if defined(PCBSKY) || defined(PCB9XT)
#define CSW_INDEX	9	// Index of first custom switch
#endif
#ifdef PCBX9D
#define CSW_INDEX	9	// Index of first custom switch
//#define CSW_INDEX	22	// Index of first custom switch
#endif

#define DIM(arr) (sizeof((arr))/sizeof((arr)[0]))

#ifdef REV9E
#define KEY_PAGE	KEY_LEFT
#endif	// REV9E

enum EnumKeys {
    KEY_MENU ,
    KEY_EXIT ,
    KEY_DOWN ,
    KEY_UP  ,
    KEY_RIGHT ,
    KEY_LEFT ,
    TRM_LH_DWN  ,
    TRM_LH_UP   ,
    TRM_LV_DWN  ,
    TRM_LV_UP   ,
    TRM_RV_DWN  ,
    TRM_RV_UP   ,
    TRM_RH_DWN  ,
    TRM_RH_UP   ,
	  BTN_RE,

#if defined(PCBSKY) || defined(PCB9XT)
    //SW_NC     ,
    //SW_ON     ,
    SW_ThrCt  ,
    SW_RuddDR ,
    SW_ElevDR ,
    SW_ID0    ,
    SW_ID1    ,
    SW_ID2    ,
    SW_AileDR ,
    SW_Gear   ,
    SW_Trainer
#endif

#ifdef PCBX9D
  SW_SF2,
  SW_nu1,
  SW_nu2,
  SW_SC0,
  SW_SC1,
  SW_SC2,
  SW_nu3,
  SW_nu4,
  SW_SH2
#endif
};

// Hardware switch mappings:
#if defined(PCBSKY) || defined(PCB9XT)
#define HSW_ThrCt			1
#define HSW_RuddDR		2
#define HSW_ElevDR		3
#define HSW_ID0				4
#define HSW_ID1				5
#define HSW_ID2				6
#define HSW_AileDR		7
#define HSW_Gear			8
#define HSW_Trainer		9
//#define HSW_Thr3pos0	35
//#define HSW_Thr3pos1	36
//#define HSW_Thr3pos2	37
//#define HSW_Rud3pos0	38
//#define HSW_Rud3pos1	39
//#define HSW_Rud3pos2	40
//#define HSW_Ele3pos0	41
//#define HSW_Ele3pos1	42
//#define HSW_Ele3pos2	43
//#define HSW_Ail3pos0	44
//#define HSW_Ail3pos1	45
//#define HSW_Ail3pos2	46
//#define HSW_Gear3pos0	47
//#define HSW_Gear3pos1	48
//#define HSW_Gear3pos2	49
//#define HSW_Ele6pos0	50
//#define HSW_Ele6pos1	51
//#define HSW_Ele6pos2	52
//#define HSW_Ele6pos3	53
//#define HSW_Ele6pos4	54
//#define HSW_Ele6pos5	55
//#define HSW_MAX				55


#define HSW_Thr3pos0	45	// Skip some values because of safety switch values
#define HSW_Thr3pos1	46
#define HSW_Thr3pos2	47
#define HSW_Rud3pos0	48
#define HSW_Rud3pos1	49
#define HSW_Rud3pos2	50
#define HSW_Ele3pos0	51
#define HSW_Ele3pos1	52
#define HSW_Ele3pos2	53
#define HSW_Ail3pos0	54
#define HSW_Ail3pos1	55
#define HSW_Ail3pos2	56
#define HSW_Gear3pos0	57
#define HSW_Gear3pos1	58
#define HSW_Gear3pos2	59
#define HSW_Ele6pos0	60
#define HSW_Ele6pos1	61
#define HSW_Ele6pos2	62
#define HSW_Ele6pos3	63
#define HSW_Ele6pos4	64
#define HSW_Ele6pos5	65
#define HSW_Pb1				66
#define HSW_Pb2				67
#define HSW_Pb3				68
#define HSW_Pb4				69
#define HSW_MAX				69

#define HSW_OFFSET ( HSW_Thr3pos0 - ( HSW_Trainer + NUM_SKYCSW + 1 ) )


//Bitfield for hardware switch mapping
#define	USE_THR_3POS	0x01
#define	USE_RUD_3POS	0x02
#define	USE_ELE_3POS	0x04
#define	USE_ELE_6POS	0x08
#define	USE_AIL_3POS	0x10
#define	USE_GEA_3POS	0x20
#define	USE_ELE_6PSB	0x40
#define	USE_PB1				0x80
#define	USE_PB2				0x100
#define	USE_PB3				0x200
#define	USE_PB4				0x400

//extern uint8_t switchMap[] ;
//extern uint8_t switchUnMap[] ;
uint16_t oneSwitchText( uint8_t swtch, uint16_t state ) ;
extern uint8_t Sw3PosList[] ;
extern uint8_t Sw3PosCount[] ;

void create6posTable( void ) ;
extern uint16_t SixPositionTable[5] ;

#endif // PCBSKY

void createSwitchMapping( void ) ;
int8_t switchUnMap( int8_t x ) ;
int8_t switchMap( int8_t x ) ;

#ifdef PCBX9D

#ifdef REV9E
#define HSW_SD2				1
#else
#define HSW_SF2				1
#endif
//#define HSW_SF2				2

#define HSW_SC0				4
#define HSW_SC1				5
#define HSW_SC2				6

//#define HSW_SH0				8
#define HSW_SH2				9

#define HSW_SB0				45	// Skip some values because of safety switch values
#define HSW_SB1				46
#define HSW_SB2				47
#define HSW_SE0				48
#define HSW_SE1				49
#define HSW_SE2				50
#define HSW_SA0				51
#define HSW_SA1				52
#define HSW_SA2				53
#ifdef REV9E
#define HSW_SF0				54
#define HSW_SF1				55
#define HSW_SF2				56
#else
#define HSW_SD0				54
#define HSW_SD1				55
#define HSW_SD2				56
#endif
#define HSW_SG0				57
#define HSW_SG1				58
#define HSW_SG2				59
#define HSW_Ele6pos0	60
#define HSW_Ele6pos1	61
#define HSW_Ele6pos2	62
#define HSW_Ele6pos3	63
#define HSW_Ele6pos4	64
#define HSW_Ele6pos5	65
#ifdef REV9E
#define HSW_SI0				66
#define HSW_SI1				67
#define HSW_SI2				68
#define HSW_SJ0				69
#define HSW_SJ1				70
#define HSW_SJ2				71
#define HSW_SK0				72
#define HSW_SK1				73
#define HSW_SK2				74
#define HSW_SL0				75
#define HSW_SL1				76
#define HSW_SL2				77
#define HSW_SM0				78
#define HSW_SM1				79
#define HSW_SM2				80
#define HSW_SN0				81
#define HSW_SN1				82
#define HSW_SN2				83
#define HSW_SO0				84
#define HSW_SO1				85
#define HSW_SO2				86
#define HSW_SP0				87
#define HSW_SP1				88
#define HSW_SP2				89
#define HSW_SQ0				90
#define HSW_SQ1				91
#define HSW_SQ2				92
#define HSW_SR0				93
#define HSW_SR1				94
#define HSW_SR2				95
#define HSW_MAX				95
#else
#define HSW_MAX				65
#endif	// REV9E

//Bitfield for hardware analog mapping
#define	USE_P1_ENC	0x01
#define	USE_P2_ENC	0x02
#define	USE_P3_ENC	0x03
#define ENC_MASK		0x03


#define HSW_OFFSET ( HSW_SB0 - ( HSW_SH2 + NUM_SKYCSW + 1 ) )

#endif // PCBX9D

#define	USE_P1_6POS		0x04
#define	USE_P2_6POS		0x08
#define	USE_P3_6POS		0x0C
#define	USE_AUX_6POS	0x10
#define MASK_6POS			0x1C

extern uint8_t MaxSwitchIndex ;		// For ON and OFF

// c17-c24 added for timer mode A display
//#define CURV_STR "\003---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24"
#define CURVE_BASE 7

//#define CSWITCH_STR  "----   v>ofs  v<ofs  |v|>ofs|v|<ofsAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""v1>=v2 ""v1<=v2 TimeOff"
#define CSW_LEN_FUNC 7


#define CS_OFF       0
#define CS_VPOS      1  //v>offset
#define CS_VNEG      2  //v<offset
#define CS_APOS      3  //|v|>offset
#define CS_ANEG      4  //|v|<offset
#define CS_AND       5
#define CS_OR        6
#define CS_XOR       7
#define CS_EQUAL     8
#define CS_NEQUAL    9
#define CS_GREATER   10
#define CS_LESS      11
#define CS_LATCH		 12
#define CS_FLIP			 13
#define CS_TIME	     14
#define CS_NTIME     15
#define CS_MONO		   16	// Monostable
#define CS_RMONO	   17	// Monostable with reset
#define CS_EXEQUAL   18	// V~=offset
#define CS_MAXF      18  //max function

#define CS_VOFS       0
#define CS_VBOOL      1
#define CS_VCOMP      2
#define CS_TIMER			3
#define CS_TMONO      4

uint8_t CS_STATE( uint8_t x) ;
//#define CS_STATE(x)   ((x)<CS_AND ? CS_VOFS : ((((x)<CS_EQUAL) || ((x)==CS_LATCH) || ((x)==CS_FLIP)) ? CS_VBOOL : ((x)<CS_TIME ? CS_VCOMP : CS_TIMER)))

#if defined(PCBSKY) || defined(PCB9XT)
#define SW_BASE      SW_ThrCt
#define SW_BASE_DIAG SW_ThrCt
#define MAX_PSWITCH   (SW_Trainer-SW_ThrCt+1)  // 9 physical switches
#define MAX_DRSWITCH (1+SW_Trainer-SW_ThrCt+1+NUM_CSW)
#define MAX_SKYDRSWITCH (1+SW_Trainer-SW_ThrCt+1+NUM_SKYCSW)
#endif

#ifdef PCBX9D
//#define SW_BASE      SW_SA0
#define SW_BASE      SW_SF2
//#define SW_BASE_DIAG SW_SA0
#define SW_BASE_DIAG SW_SF2
//#define MAX_PSWITCH   (SW_SH2-SW_SA0+1)  // 9 physical switches
#define MAX_PSWITCH   (SW_SH2-SW_SF2+1)  // 9 physical switches
//#define MAX_DRSWITCH (1+SW_SH2-SW_SA0+1+NUM_CSW)
//#define MAX_SKYDRSWITCH (1+SW_SH2-SW_SA0+1+NUM_SKYCSW)
#define MAX_SKYDRSWITCH (1+SW_SH2-SW_SF2+1+NUM_SKYCSW)
#endif


#define SWP_RUD (SW_RuddDR-SW_BASE)
#define SWP_ELE (SW_ElevDR-SW_BASE)
#define SWP_AIL (SW_AileDR-SW_BASE)
#define SWP_THR (SW_ThrCt-SW_BASE)
#define SWP_GEA (SW_Gear-SW_BASE)

#define SWP_RUDB (1<<SWP_RUD)
#define SWP_ELEB (1<<SWP_ELE)
#define SWP_AILB (1<<SWP_AIL)
#define SWP_THRB (1<<SWP_THR)
#define SWP_GEAB (1<<SWP_GEA)

#define SWP_ID0 (SW_ID0-SW_BASE)
#define SWP_ID1 (SW_ID1-SW_BASE)
#define SWP_ID2 (SW_ID2-SW_BASE)
#define SWP_ID0B (1<<SWP_ID0)
#define SWP_ID1B (1<<SWP_ID1)
#define SWP_ID2B (1<<SWP_ID2)

//Switch Position Illigal states
#define SWP_IL1 (0)
#define SWP_IL2 (SWP_ID0B | SWP_ID1B)
#define SWP_IL3 (SWP_ID0B | SWP_ID2B)
#define SWP_IL4 (SWP_ID1B | SWP_ID2B)
#define SWP_IL5 (SWP_ID0B | SWP_ID1B | SWP_ID2B)

#if defined(PCBSKY) || defined(PCB9XT)
#define THR_WARN_MASK	0x0101
#define RUD_WARN_MASK	0x0202
#define ELE_WARN_MASK	0x0C04
#define IDX_WARN_MASK	0x0038
#define AIL_WARN_MASK	0x1040
#define GEA_WARN_MASK	0x2080
#endif


#define NUM_KEYS BTN_RE+1
#define TRM_BASE TRM_LH_DWN

#define _MSK_KEY_REPT    0x40
#define _MSK_KEY_DBL     0x10
#define IS_KEY_BREAK(key)  (((key)&0xf0)        ==  0x20)
#define EVT_KEY_BREAK(key) ((key)|                  0x20)
#define EVT_KEY_FIRST(key) ((key)|    _MSK_KEY_REPT|0x20)
#define EVT_KEY_REPT(key)  ((key)|    _MSK_KEY_REPT     )
#define EVT_KEY_LONG(key)  ((key)|0x80)
#define EVT_KEY_DBL(key)   ((key)|_MSK_KEY_DBL)
//#define EVT_KEY_DBL(key)   ((key)|0x10)
#define EVT_ENTRY               (0xff - _MSK_KEY_REPT)
#define EVT_ENTRY_UP            (0xfe - _MSK_KEY_REPT)
#define EVT_TOGGLE_GVAR         (0xfd - _MSK_KEY_REPT)
#define EVT_KEY_MASK             0x0f

#define HEART_TIMER_PULSES 1 ;
#define HEART_TIMER10ms 2;


#define INP_D_TRM_LH_UP   7
#define INP_D_TRM_LH_DWN  6
#define INP_D_TRM_RV_DWN  5
#define INP_D_TRM_RV_UP   4
#define INP_D_TRM_LV_DWN  3
#define INP_D_TRM_LV_UP   2
#define INP_D_TRM_RH_DWN  1
#define INP_D_TRM_RH_UP   0

#define RESX    (1<<10) // 1024
#define RESXu   1024u
#define RESXul  1024ul
#define RESXl   1024l
#define RESKul  100ul
#define RESX_PLUS_TRIM (RESX+128)

#define TRIM_EXTENDED_MAX	500

#define NUM_PPM     8
#define NUM_EXTRA_PPM     8
//number of real outputchannels CH1-CH16
#define NUM_CHNOUT  16
#define NUM_SKYCHNOUT  24
///number of real input channels (1-9) plus virtual input channels X1-X4
#define PPM_BASE    MIX_CYC3
#define CHOUT_BASE  (PPM_BASE+NUM_PPM)

#define NUM_FSW			16

#define NUM_STICKS	4

#define NUM_SCALERS	8

//#define SWASH_TYPE_STR   "---   ""120   ""120X  ""140   ""90    "

#define SWASH_TYPE_120   1
#define SWASH_TYPE_120X  2
#define SWASH_TYPE_140   3
#define SWASH_TYPE_90    4
#define SWASH_TYPE_NUM   4


#define MIX_P1    5
#define MIX_P2    6
#define MIX_P3    7
#if defined(PCBSKY) || defined(PCB9XT)
#define MIX_MAX   8
#define MIX_FULL  9
#define MIX_CYC1  10
#define MIX_CYC2  11
#define MIX_CYC3  12

#define	NUM_EXTRA_POTS 0

#endif

#define EXTRA_POTS_POSITION	8

#ifdef PCBX9D
#define MIX_MAX   8 
#define MIX_FULL  9 
#define MIX_CYC1  10
#define MIX_CYC2  11
#define MIX_CYC3  12
#define MIX_P4    200
#define MIX_P5    201
#define MIX_P6    202
#define MIX_P7    203
#define MIX_P8    204

#ifdef REV9E
#define	NUM_EXTRA_POTS 5
#else
 #ifdef REVPLUS
#define	NUM_EXTRA_POTS 2
 #else
#define	NUM_EXTRA_POTS 1
 #endif	// REVPLUS
#endif	// REV9E
#endif	// PCBX9D

#define EXTRA_POTS_START	120

#define DR_HIGH   0
#define DR_MID    1
#define DR_LOW    2
#define DR_EXPO   0
#define DR_WEIGHT 1
#define DR_RIGHT  0
#define DR_LEFT   1
#define DR_BOTH   2
#define DR_DRSW1  99
#define DR_DRSW2  98

#define DSW_THR  1
#define DSW_RUD  2
#define DSW_ELE  3
#define DSW_ID0  4
#define DSW_ID1  5
#define DSW_ID2  6
#define DSW_AIL  7
#define DSW_GEA  8
#define DSW_TRN  9
#define DSW_SW1  10
#define DSW_SW2  11
#define DSW_SW3  12
#define DSW_SW4  13
#define DSW_SW5  14
#define DSW_SW6  15
#define DSW_SW7   16
#define DSW_SW8   17
#define DSW_SW9   18
#define DSW_SWA   19
#define DSW_SWB   20
#define DSW_SWC   21

#define SCROLL_TH 64
#define INACTIVITY_THRESHOLD 256
#define THRCHK_DEADBAND 16
#define SPLASH_TIMEOUT  (4*100)  //400 msec - 4 seconds

uint8_t IS_THROTTLE( uint8_t x ) ;
uint8_t IS_EXPO_THROTTLE( uint8_t x ) ;

extern uint8_t Ee_lock ;

// Bit masks in Ee_lock
#define EE_LOCK      1
#define EE_TRIM_LOCK 2

#define EE_GENERAL 1
#define EE_MODEL   2
#define EE_TRIM    4           // Store model because of trim
#define INCDEC_SWITCH   0x08

// Bits in SystemOptions
#define SYS_OPT_HARDWARE_EDIT	1
#define SYS_OPT_MUTE					2

#define TMR_VAROFS  4

#define TMRMODE_NONE     0
#define TMRMODE_ABS      1
#define TMRMODE_THR      2
#define TMRMODE_THR_REL  3
#define MAX_ALERT_TIME   20

#define PROTO_PPM        0
#define PROTO_PXX        1
#define PROTO_DSM2       2
#define PROTO_MULTI      3
#define PROTO_ASSAN      4
#ifdef ASSAN
#define PROT_MAX         4
#else
#define PROT_MAX         3
#endif
#define PROTO_PPM16			 3		// No longer needed
#define PROTO_OFF		     15		// For X9D
#define PROT_STR_LEN      6
#ifdef ASSAN
#define DSM2_STR "\011LP4/LP5  DSM2only DSM2/DSMX9XR-DSM  "
#else
#define DSM2_STR "\011LP4/LP5  DSM2only DSM2/DSMX9XR-DSM  "
#endif
#define DSM2_STR_LEN   9
#define LPXDSM2          0
#define DSM2only         1
#define DSM2_DSMX        2
#define DSM_9XR		       3

#define MULTI_STR "\006FlyskyHubsanFrsky Hisky V2x2  DSM2  Devo  YD717 KN    SymaX SLT   CX10  CG023 "
//#define MULTI_STR_LEN    7
#define M_Flysky           0
#define M_FLYSKY_STR "\006FlyskyV9x9  V6x6  V912  "
#define M_Hubsan           1
#define M_Frsky            2
#define M_Hisky            3
#define M_V2x2             4
#define M_DSM2             5
#define M_DSM2_STR "\004DSM2DSMX"
#define M_Devo	  	       6
#define M_YD717	           7
#define M_KN	  	         8
#define M_YD717_STR "\007YD717  SKYWLKRSYMAX2 XINXUN NIHUI  "
#define M_SymaX	           9
#define M_SYMAX_STR "\007SYMAX  SYMAX5CSYMAX4 "
#define M_SLT		  		     10
#define M_CX10		       11
#define M_CX10_STR "\005GREENBLUE DM007"
#define M_CG023		       12
#define M_NONE_STR "\004None"
#define M_NY_STR "\001NY"
#define M_LH_STR "\004HighLow "


// PXX_SEND_RXNUM == BIND
#define PXX_BIND			     0x01
#define PXX_SEND_FAILSAFE  0x10
#define PXX_RANGE_CHECK		 0x20
#define PXX_DSMX			     0x02

#define POWER_OFF			0
#define POWER_ON			1
#define POWER_TRAINER	2
#define POWER_X9E_STOP	3


extern uint8_t pxxFlag;
extern uint8_t pxxFlag_x ;
extern uint8_t InactivityMonitor ;
extern uint16_t InacCounter ;

#define FLASH_DURATION 50

extern uint16_t g_LightOffCounter;


template<class t> inline t min(t a, t b){ return a<b?a:b; }
template<class t> inline t max(t a, t b){ return a>b?a:b; }
template<class t> inline t limit(t mi, t x, t ma){ return min(max(mi,x),ma); }

// This doesn't need protection on this processor
#define get_tmr10ms() g_tmr10ms

#define sysFLAG_OLD_EEPROM (0x01)
extern uint8_t sysFlags;
extern uint8_t StickScrollAllowed ;
extern uint8_t StepSize ;

const char s_charTab[]=" ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-.";
#define NUMCHARS (sizeof(s_charTab)-1)

///number of real input channels (1-9) plus virtual input channels X1-X4
#define PPM_BASE    MIX_CYC3
#define CHOUT_BASE  (PPM_BASE+NUM_PPM)

//extern const char *Str_telemItems ;
extern const int8_t TelemIndex[] ;
extern int16_t convertTelemConstant( int8_t channel, int8_t value) ;
extern int16_t getValue(uint8_t i) ;
#define NUM_TELEM_ITEMS 47

#define NUM_XCHNRAW (CHOUT_BASE+NUM_CHNOUT) // NUMCH + P1P2P3+ AIL/RUD/ELE/THR + MAX/FULL + CYC1/CYC2/CYC3
#define NUM_SKYXCHNRAW (CHOUT_BASE+NUM_SKYCHNOUT) // NUMCH + P1P2P3+ AIL/RUD/ELE/THR + MAX/FULL + CYC1/CYC2/CYC3
///number of real output channels (CH1-CH8) plus virtual output channels X1-X4
#define NUM_XCHNOUT (NUM_CHNOUT) //(NUM_CHNOUT)//+NUM_VIRT)
#define NUM_SKYXCHNOUT (NUM_SKYCHNOUT) //(NUM_CHNOUT)//+NUM_VIRT)

#define MIX_3POS	(NUM_SKYXCHNRAW+1)

inline int32_t calc100toRESX(register int8_t x)
{
  return ((int32_t)x*655)>>6 ;
}

inline int16_t calc1000toRESX( register int32_t x)  // improve calc time by Pat MacKenzie
{
    register int32_t y = x>>5;
    x+=y;
    y=y>>2;
    x-=y;
    return x+(y>>2);
    //  return x + x/32 - x/128 + x/512;
}

extern uint32_t Current_used ;
extern uint16_t Current_current ;
extern uint16_t Current_max ;
extern uint16_t MAh_used ;
extern uint16_t Run_time ;

//#ifdef FIX_MODE
extern const char stickScramble[] ;
//extern const char modeFix[] ;
uint8_t modeFixValue( uint8_t value ) ;
//#else
//extern const char modn12x3[] ;
//#endif

extern const uint8_t bchout_ar[] ;

//convert from mode 1 to mode g_eeGeneral.stickMode
//NOTICE!  =>  1..4 -> 1..4
extern uint8_t convert_mode_helper(uint8_t x) ;

#define CONVERT_MODE(x)  (((x)<=4) ? convert_mode_helper(x) : (x))
//#define CHANNEL_ORDER(x) (chout_ar[g_eeGeneral.templateSetup*4 + (x)-1])
#define CHANNEL_ORDER(x) ( ( (bchout_ar[g_eeGeneral.templateSetup] >> (6-(x-1) * 2)) & 3 ) + 1 )
#define THR_STICK       (2-(g_eeGeneral.stickMode&1))
#define ELE_STICK       (1+(g_eeGeneral.stickMode&1))
#define AIL_STICK       ((g_eeGeneral.stickMode&2) ? 0 : 3)
#define RUD_STICK       ((g_eeGeneral.stickMode&2) ? 3 : 0)

#define STORE_MODELVARS_TRIM   eeDirty(EE_MODEL|EE_TRIM)
#define STORE_MODELVARS   eeDirty(EE_MODEL)
#define STORE_GENERALVARS eeDirty(EE_GENERAL)

#ifdef PCBSKY
#define BACKLIGHT_ON    (PWM->PWM_CH_NUM[0].PWM_CDTY = g_eeGeneral.bright)
#define BACKLIGHT_OFF   (PWM->PWM_CH_NUM[0].PWM_CDTY = 100)
#endif

#if defined(PCBX9D) || defined(PCB9XT)
extern void backlight_on( void ) ;
extern void backlight_off( void ) ;
#ifdef REVPLUS
extern void backlight_w_on( void ) ;
extern void backlight_w_off( void ) ;
#define BACKLIGHT_ON        backlight_on();backlight_w_on()
#define BACKLIGHT_OFF       backlight_off();backlight_w_off()
#else
#define BACKLIGHT_ON        backlight_on()
#define BACKLIGHT_OFF       backlight_off()
#endif
#endif

// Options for mainSequence()
#define NO_MENU		1
#define MENUS			0

extern int16_t *const CalibMid[] ;
extern int16_t *const CalibSpanPos[] ;
extern int16_t *const CalibSpanNeg[] ;

#ifdef PCBX9D
 #if REVPLUS
  #ifdef REV9E
	 #define NUM_ANALOG_CALS	12
	#else
	 #define NUM_ANALOG_CALS	9
	#endif
 #else
  #define NUM_ANALOG_CALS	8
 #endif
#else
 #define NUM_ANALOG_CALS	7
#endif


extern uint16_t evalChkSum( void ) ;

struct t_calib
{
	int16_t midVals[NUM_ANALOG_CALS];
	int16_t loVals[NUM_ANALOG_CALS];
	int16_t hiVals[NUM_ANALOG_CALS];
	uint8_t idxState;
} ;

uint8_t menuPressed( void ) ;

struct t_alpha
{
	uint8_t AlphaIndex ;
	uint8_t lastSub ;
	uint8_t AlphaLength ;
	uint8_t *PalphaText ;
	uint8_t *PalphaHeading ;
} ;

union t_xmem
{
//	struct MixTab s_mixTab[MAX_MIXERS+NUM_XCHNOUT+1] ;	
	struct t_calib Cal_data ;
#ifndef CPUARM
	char buf[sizeof(g_model.name)+5];
#endif
	struct t_alpha Alpha ;
//#if defined(CPUM128) || defined(CPUM2561)
//  uint8_t file_buffer[256];
//#else
//  uint8_t file_buffer[128];
//#endif
} ;

extern union t_xmem Xmem ;


typedef void (*MenuFuncP)(uint8_t event);

typedef struct
{ 
	unsigned char second;   //enter the current time, date, month, and year
	unsigned char minute;
	unsigned char hour;                                     
	unsigned char date;       
	unsigned char month;
	unsigned int year;      
 } t_time ;

extern t_time Time ;

extern bool    checkIncDec_Ret;//global helper vars
extern uint8_t s_editMode;     //global editmode

int16_t checkIncDec16( int16_t i_pval, int16_t i_min, int16_t i_max, uint8_t i_flags);
int8_t checkIncDec( int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags);
int8_t checkIncDecSwitch( int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags);
int8_t checkIncDec_hm( int8_t i_val, int8_t i_min, int8_t i_max);
//int8_t checkIncDec_vm(uint8_t event, int8_t i_val, int8_t i_min, int8_t i_max);
int8_t checkIncDec_hg( int8_t i_val, int8_t i_min, int8_t i_max);
int8_t checkIncDec_hg0( int8_t i_val, int8_t i_max);
int8_t checkIncDec_hm0( int8_t i_val, int8_t i_max);

#define CHECK_INCDEC_H_GENVAR( var, min, max)     \
    var = checkIncDec_hg(var,min,max)

#define CHECK_INCDEC_H_GENVAR_0( var, max)     \
    var = checkIncDec_hg0( var, max )

#define CHECK_INCDEC_H_MODELVAR( var, min, max)     \
    var = checkIncDec_hm(var,min,max)

#define CHECK_INCDEC_H_MODELVAR_0( var, max)     \
    var = checkIncDec_hm0( var,max)

#define CHECK_INCDEC_MODELSWITCH( var, min, max) \
  var = checkIncDecSwitch(var,min,max,EE_MODEL|INCDEC_SWITCH)

#define CHECK_INCDEC_GENERALSWITCH( event, var, min, max) \
  var = checkIncDecSwitch(var,min,max,EE_GENERAL|INCDEC_SWITCH)


extern uint8_t heartbeat ;
extern int16_t g_chans512[NUM_SKYCHNOUT];
//extern uint8_t eeprom[4096] ;
extern uint8_t BtAsPpm ;
extern uint8_t BtBaudrateChanged ;

uint8_t char2idx(char c);
char idx2char(uint8_t idx);

//extern volatile uint32_t Timer2_count ;		// Modified in interrupt routine
extern volatile uint16_t g_tmr10ms ;
extern volatile uint8_t  g_blinkTmr10ms;
extern volatile uint8_t tick10ms ;
extern uint32_t Master_frequency ;

extern void alert(const char * s, bool defaults=false);
extern void message(const char * s);

void resetTimer();
void resetTimer1( void ) ;
void resetTimer2( void ) ;

//extern uint8_t Timer2_running ;
//extern uint16_t Timer2 ;
//void resetTimer2( void ) ;

extern void putsTime(uint8_t x,uint8_t y,int16_t tme,uint8_t att,uint8_t att2) ;
extern void putsVolts(uint8_t x,uint8_t y, uint8_t volts, uint8_t att) ;
extern void putsVBat(uint8_t x,uint8_t y,uint8_t att) ;
extern void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att) ;
extern void putsChn(uint8_t x,uint8_t y,uint8_t idx1,uint8_t att) ;
extern void putsDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att) ; //, bool nc) ;
extern void putsMomentDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att) ;
extern void putsTmrMode(uint8_t x, uint8_t y, uint8_t attr, uint8_t timer, uint8_t type ) ;
extern const char *get_switches_string( void ) ;
void putsDblSizeName( uint8_t y ) ;
void clearKeyEvents( void ) ;

extern void interrupt5ms() ;
extern uint16_t getTmr2MHz( void ) ;

extern int16_t intpol(int16_t x, uint8_t idx);

extern uint16_t anaIn(uint8_t chan) ;

extern int8_t phyStick[4] ;
extern int16_t ex_chans[NUM_SKYCHNOUT];

extern void modelDefault( uint8_t id ) ;
extern uint8_t VoiceCheckFlag ;

//void eeWaitComplete( void ) ;
void eeDirty(uint8_t msk);
//void eeCheck(bool immediately=false ) ;
void eeReadAll( void ) ;
//void eeLoadModelName(uint8_t id,char*buf,uint8_t len);
//uint16_t eeFileSize(uint8_t id);
//void eeLoadModel(uint8_t id);
//void eeSaveModel(uint8_t id);
bool eeDuplicateModel(uint8_t id);
//bool eeModelExists(uint8_t id);

extern char idx2char(uint8_t idx) ;
extern uint8_t char2idx(char c) ;

extern int16_t            g_ppmIns[];
extern uint8_t ppmInState ; //0=unsync 1..8= wait for value i-1
extern uint8_t ppmInValid ;

/// goto given Menu, but substitute current menu in menuStack
extern void    chainMenu(MenuFuncP newMenu);
/// goto given Menu, store current menu in menuStack
extern void    pushMenu(MenuFuncP newMenu);
///deliver address of last menu which was popped from
extern MenuFuncP lastPopMenu();
/// return to last menu in menustack
/// if uppermost is set true, thenmenu return to uppermost menu in menustack
void    popMenu(bool uppermost=false);

#define NO_TRAINER 0x01
#define NO_INPUT   0x02
#define FADE_FIRST	0x20
#define FADE_LAST		0x40
#define NO_DELAY_SLOW	0x80

// Timeout, in seconds, stick scroll remains active
#define STICK_SCROLL_TIMEOUT		5

extern bool getSwitch00( int8_t swtch ) ;
extern bool getSwitch(int8_t swtch, bool nc, uint8_t level = 0 ) ;
extern int8_t getMovedSwitch( void ) ;
extern uint8_t g_vbat100mV ;
//extern uint16_t Timer2 ;
extern void doSplash( void ) ;
extern void mainSequence( uint32_t no_menu ) ;
#ifdef FRSKY
extern uint8_t putsTelemValue(uint8_t x, uint8_t y, int16_t val, uint8_t channel, uint8_t att ) ;
extern void telem_byte_to_bt( uint8_t data ) ;
extern int16_t scale_telem_value( int16_t val, uint8_t channel, uint8_t *dplaces ) ;
#endif
uint8_t telemItemValid( uint8_t index ) ;

extern int16_t get_telemetry_value( int8_t channel ) ;

extern void putVoiceQueue( uint16_t value ) ;
void voice_numeric( int16_t value, uint8_t num_decimals, uint16_t units_index ) ;
extern void voice_telem_item( int8_t index ) ;
extern uint8_t *cpystr( uint8_t *dest, uint8_t *source ) ;
extern uint8_t *ncpystr( uint8_t *dest, uint8_t *source, uint8_t count ) ;
extern int16_t m_to_ft( int16_t metres ) ;

uint16_t getCurrentSwitchStates( void ) ;

extern uint32_t check_soft_power( void ) ;
extern uint32_t getFlightPhase( void ) ; 
extern int16_t getRawTrimValue( uint8_t phase, uint8_t idx ) ;
extern int16_t getTrimValue( uint8_t phase, uint8_t idx ) ;
extern void setTrimValue(uint8_t phase, uint8_t idx, int16_t trim) ;

extern void checkSwitches( void ) ;
extern void checkTHR( void ) ;
extern void checkCustom( void ) ;
extern void setLanguage( void ) ;
extern void checkXyCurve( void ) ;

#define TMR_OFF     0
#define TMR_RUNNING 1
#define TMR_BEEPING 2
#define TMR_STOPPED 3

struct t_timer
{
	uint16_t s_sum ;
	uint8_t lastSwPos ;
	uint8_t sw_toggled ;
	uint16_t s_timeCumSw ;  //laufzeit in 1/16 sec
	uint8_t  s_timerState ;
	uint8_t lastResetSwPos;
	uint16_t s_timeCumThr ;  //gewichtete laufzeit in 1/16 sec
	uint16_t s_timeCum16ThrP ; //gewichtete laufzeit in 1/16 sec
	int16_t  s_timerVal ;
	int16_t last_tmr ;
	uint16_t s_timeCumAbs;  //laufzeit in 1/16 sec
} ;

extern struct t_timer s_timer[] ;

// Rotary encoder movement states
#define	ROTARY_MENU_LR		0
#define	ROTARY_MENU_UD		1
#define	ROTARY_SUBMENU_LR	2
#define	ROTARY_VALUE			3

extern uint8_t RotaryState ;		// Defaults to ROTARY_MENU_LR
extern void valueprocessAnalogEncoder( uint32_t x ) ;

extern uint8_t Tevent ;

#if GVARS
extern int8_t REG100_100(int8_t x) ;
extern int8_t REG(int8_t x, int8_t min, int8_t max) ;
#endif

enum PowerState
{
  e_power_on,
  e_power_trainer,
  e_power_usb,
  e_power_off
} ;

struct t_p1
{
	int16_t p1val ;
	int16_t p1valdiff ;
  int16_t p1valprev ;
	int16_t p2valprev ;
	int16_t p3valprev ;
} ;

extern struct t_p1 P1values ;

#ifdef PCBSKY
extern uint16_t ResetReason ;
#endif
#if defined(PCBX9D) || defined(PCB9XT)
extern uint32_t ResetReason ;
#endif
extern uint8_t unexpectedShutdown ;
extern uint8_t SdMounted ;

#if defined(PCBX9D) || defined(PCB9XT)
#include "X9D/stm32f2xx.h"
#include "X9D/rtc.h"
#include "X9D/stm32f2xx_rtc.h"

#define MASTER_FREQUENCY 60000000
#define PERI1_FREQUENCY 15000000
#define PERI2_FREQUENCY 30000000
#define TIMER_MULT1			2
#define TIMER_MULT2			2

void rtcSetTime( t_time *t ) ;
void rtc_gettime( t_time *t ) ;
void rtcInit( void ) ;
#endif

extern void setVolume( uint8_t value ) ;
extern uint8_t HoldVolume ;

#define	ALERT_TYPE	0
#define MESS_TYPE		1

extern const char *AlertMessage ;
extern uint8_t AlertType ;

#if defined(PCBX9D) || defined(PCB9XT)
#define INTERNAL_MODULE 0
#define EXTERNAL_MODULE 1
#define TRAINER_MODULE  2
#endif

// COM2 Functions
#define COM2_FUNC_TELEMETRY		0
#define COM2_FUNC_SBUSTRAIN		1
#define COM2_FUNC_SBUS57600		2
#ifdef PCBSKY
#define COM2_FUNC_BTDIRECT		3
#define COM2_FUNC_FMS					4
#endif
#ifdef PCBX9D
#define COM2_FUNC_CPPMTRAIN		3
#endif
/** Console baudrate 9600. */
#define CONSOLE_BAUDRATE    115200

void com2Configure( void ) ;

extern uint8_t TmOK ;

uint8_t throttleReversed( void ) ;


struct btRemote_t
{
	uint8_t address[16] ;
	uint8_t name[16] ;
} ;
extern uint8_t BtCurrentBaudrate ;
extern uint8_t BtLinkRequest ;

#ifdef PCBSKY
extern struct btRemote_t BtRemote[] ;
extern uint8_t NumberBtremotes ;
#endif

extern uint8_t HardwareMenuEnabled ;


#if defined(PCBX9D) || defined(PCB9XT)
struct t_PeripheralSpeeds
{
	uint32_t Peri1_frequency ;
	uint32_t Peri2_frequency ;
	uint32_t Timer_mult1 ;
	uint32_t Timer_mult2 ;
} ;

extern struct t_PeripheralSpeeds PeripheralSpeeds ;

#endif

#define SCC_BAUD_125000		0
#define SCC_BAUD_115200		1

uint8_t *btAddrBin2Hex( uint8_t *dest, uint8_t *source ) ;
uint32_t btAddressValid( uint8_t *address ) ;
uint32_t btAddressMatch( uint8_t *address1, uint8_t *address2 ) ;

#endif
