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
#include <stdlib.h>
#include <string.h>

#ifdef PCBSKY
#include "AT91SAM3S4.h"
#include "core_cm3.h"
#endif
#include "ersky9x.h"
#include "myeeprom.h"
#include "audio.h"
#include "sound.h"
#include "lcd.h"
#include "debug.h"
#include "menus.h"
#include "drivers.h"
#include "logicio.h"
#include "file.h"
#include "stringidx.h"
#include "templates.h"
#include "pulses.h"
#include "mixer.h"
#include "frsky.h"
//#include <ctype.h>
#ifndef SIMU
#include "CoOS.h"
#endif

#ifdef PCBX9D
#include "X9D/eeprom_rlc.h"
#include "timers.h"
#include "analog.h"
#endif

#if defined(PCBLEM1)
#include "timers.h"
void eeLoadModel(uint8_t id) ;
#include <stm32f10x.h>
#endif


#if defined(PCBX12D) || defined(PCBX10)
#include "timers.h"
#include "analog.h"

#include "X12D/hal.h"

void eeLoadModel(uint8_t id) ;
//void writeModelToBackupRam( uint8_t index ) ;

#endif

#if defined(PCBX12D) || defined(PCBX10)
#define SCREEN_LINES		15

//#define	WHERE_TRACK		1
//void notePosition( uint8_t byte ) ;

#else
#define SCREEN_LINES		8
#endif

uint32_t doPopup( const char *list, uint16_t mask, uint8_t width, uint8_t event ) ;

uint8_t s_moveMode;

#define POPUP_NONE			0
#define POPUP_SELECT		1
#define POPUP_EXIT			2


#ifdef  BASIC
#include "basic/basic.h"
#endif


#ifdef PCB9XT
#include "timers.h"
#include "analog.h"
#include "mega64.h"
#include "isp.h"
#endif

#include "ff.h"
#include "maintenance.h"

#ifdef REVX
 #include "jeti.h"
#endif

#ifdef BLUETOOTH
#include "bluetooth.h"
#endif

#ifdef REV9E
 #define PAGE_NAVIGATION 1
#endif // REV9E
#ifdef PCBX7
 #ifndef PCBT12
  #define PAGE_NAVIGATION 1
 #endif
#endif // PCBX7
#ifdef PCBX9LITE
 #define PAGE_NAVIGATION 1
#endif // PCBX9LITE
#ifdef REV19
 #define PAGE_NAVIGATION 1
#endif // REV19
#ifdef PCBLEM1
 #define PAGE_NAVIGATION 1
#endif // PCBLEM1

//#ifdef PCBX10
// #define PAGE_NAVIGATION 1
//#endif // PCBX10

//#ifdef PCBX12D
// #define PAGE_NAVIGATION 1
//#endif // PCBX7

//#ifdef REVX
//#define USE_EXT_RTC		1
//#endif

//#define STACK_PROBES	1

#define PARAM_OFS   17*FW

extern char LastItem[] ;
extern uint8_t PrivateData[] ;

extern uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext ) ;
void setupFileNames( TCHAR *dir, struct fileControl *fc, char *ext ) ;
static struct fileControl FileControl = {	0,0,0,0, {0,0,0,0} } ;
struct fileControl PlayFileControl = {	0,0,0,0, {0,0,0,0} } ;

#ifdef PCBX9D
extern uint8_t ImageDisplay ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
extern uint8_t ImageDisplay ;
//uint8_t PictureDrawn = 0 ;
#endif

#ifndef PCBSKY
union t_xmem Xmem ;
#endif

extern int32_t Rotary_diff ;
extern int16_t AltOffset ;
extern uint8_t ExtraInputs ;
extern uint8_t MuteTimer ;

static uint8_t s_currIdx;
static uint8_t RestoreIndex ;
static uint8_t SubMenuFromIndex = 0 ;
uint8_t TrainerMode ;
void setCaptureMode(uint32_t mode) ;
extern uint8_t CurrentTrainerSource ;

#define ALPHA_NO_NAME		0x80
#define ALPHA_FILENAME	0x40
#define ALPHA_HEX				0x100

#ifdef REVX
uint16_t jeti_keys = JETI_KEY_NOCHANGE;
uint8_t JetiBuffer[36] ; // 32 characters
uint8_t JetiIndex ;
uint8_t JetiBufferReady ;
#endif

struct t_timer s_timer[2] ;

uint8_t RotaryState ;		// Defaults to ROTARY_MENU_LR
uint8_t CalcScaleNest = 0 ;

struct t_alpha Alpha ;

struct t_popupData PopupData ;

static uint8_t SubmenuIndex = 0 ;
static uint8_t IlinesCount ;
static uint8_t LastSubmenuIndex = 0 ;
static uint8_t UseLastSubmenuIndex = 0 ;
static uint8_t saveHpos ;
static uint8_t StatusTimer ;

#define VOICE_FILE_TYPE_NAME	0
#define VOICE_FILE_TYPE_USER	1
#define VOICE_FILE_TYPE_MUSIC	2
#define VOICE_FILE_TYPE_SYSTEM	3
uint8_t VoiceFileType ;
uint8_t FileSelectResult ;
char SelectedVoiceFileName[16] ;

extern struct t_multiSetting MultiSetting ;

void menuProcVoiceOne(uint8_t event) ;
void menuProcSelectVoiceFile(uint8_t event) ;
#ifdef PCBX9D
void menuProcSelectImageFile(uint8_t event) ;
#endif
#ifdef IMAGE_128
void menuProcSelectImageFile(uint8_t event) ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
void menuProcSelectImageFile(uint8_t event) ;
#endif

#define CLIP_NONE		0
#define CLIP_VOICE	1
#define CLIP_SWITCH	2

struct t_clipboard
{
	uint32_t content ;
	union
	{
		VoiceAlarmData clipvoice ;
		SKYCSwData clipswitch ;
	} ;
} ;

#ifdef ACCESS

#define ACC_NORMAL		0
#define ACC_REG				1
#define ACC_BIND			2
#define ACC_RXOPTIONS		3
#define ACC_TXOPTIONS		4
#define ACC_RX_HARDWARE	5
#define ACC_SHARE				6
#define ACC_SPECTRUM		7
#define ACC_ACCST_BIND			8

uint8_t AccState ;

#endif

struct t_clipboard Clipboard ;

int8_t getAndSwitch( SKYCSwData &cs ) ;
void processSwitches( void ) ;

static int16_t hdg_home ;// Extra data for Mavlink via FrSky
static uint8_t Columns ;

int8_t phyStick[4] ;

uint8_t s_traceBuf[MAXTRACE];
uint8_t s_traceWr;
uint16_t s_traceCnt;

const uint16_t UnitsVoice[] = {SV_FEET,SV_VOLTS,SV_DEGREES,SV_DEGREES,SV_MILAMP_H,SV_AMPS,SV_METRES,SV_WATTS,SV_PERCENT,SV_KMSPH,SV_DB } ;
const uint8_t UnitsText[] = { 'F','V','C','F','m','A','m','W','%', 'k', 'd' } ;
const uint8_t UnitsString[] = "\005Feet VoltsDeg_CDeg_FmAh  Amps MetreWattsPcentkms/hdb   " ;

const uint8_t DestString[] = "\005-----BaseMAmps mAh  VoltsFuel RBSV Cus1 Cus2 Cus3 Cus4 Cus5 Cus6 Aspd " ;

#define BaudString FWx13"\005""\006  AUTO  9600 19200 38400 57600115200"
#define BtBaudString "\006115200  9600 19200 57600 38400"

const uint8_t GvaString[] = "Global Voice Alerts" ;

const char HyphenString[] = "----" ;


uint16_t Multiprotocols[NUM_MULTI_PROTOCOLS+2] ;
#ifndef SMALL
#define MULTI_TEXT_SIZE	600
#else
#define MULTI_TEXT_SIZE	400
#endif
uint8_t MultiText[MULTI_TEXT_SIZE] ;
//uint8_t MultiMapping[64] ;
uint8_t AlphaEdited ;


//const uint8_t MultiRomData[] = 
//"Flysky\0Flysky\0V9x9\0V6x6\0V912\0CX20\0\0"
//"Hubsan\0\0"
//"FrskyD\0\0"
//"Hisky\0Hisky\0HK310\0\0"
//;


const uint8_t MultiData[] = 
"\001\005Flysky\0Flysky\0V9x9\0V6x6\0V912\0CX20\0"
"\002\003Hubsan\0H107\0H301\0H501\0"
"\003\002FrskyD\0D8\0Cloned\0"
"\004\002Hisky\0Hisky\0HK310\0"
"\005\003V2x2\0V2x2\0JXD506\0MR101\0"
"\006\005DSM\0DSM2_1F\0DSM2_2F\0DSMX_1F\0DSMX_2F\0AUTO\0"
"\007\005Devo\0008CH\00010CH\00012CH\0006CH\0007CH\0"
"\010\005YD717\0YD717\0SKYWLKR\0SYMAX4\000XINXUN\0NIHUI\0"
"\011\002KN\0WLTOYS\0FEILUN\0"
"\012\002SymaX\0SYMAX\0SYMAX5C\0"
"\013\005SLT\0SLT_V1\0SLT_V2\0Q100\0Q200\0MR100\0"
"\014\007CX10\0GREEN\0BLUE\0DM007\0---\0J3015_1\0J3015_2\0MK33041\0"
"\015\002CG023\0CG023\0YD829\0"
"\016\006Bayang\0Bayang\0H8S3D\000X16_AH\0IRDRONE\0DHD_D4\0QX100\0"
"\017\005FrskyX\0CH_16\0CH_8\0EU_16\0EU_8\0Cloned\0"
"\020\002ESky\0Std\0ET4\0"
"\021\005MT99xx\0MT\0H7\0YZ\0LS\0FY805\0"
"\022\007MJXq\0WLH08\000X600\000X800\0H26D\0E010\0H26WH\0PHOENIX\0"
"\023\0Shenqi\0"
"\024\002FY326\0FY326\0FY319\0"
"\025\0SFHSS\0"
"\026\0J6PRO\0"
"\027\0FQ777\0"
"\030\0ASSAN\0"
"\031\0FrskyV\0"
"\032\004HONTAI\0HONTAI\0JJRCX1\000X5C1\0FQ777_951\0"
"\033\0OpnLrs\0"
"\034\004AFHDS2A\0PWM_IBUS\0PPM_IBUS\0PWM_SBUS\0PPM_SBUS\0"
"\035\003Q2X2\0Q222\0Q242\0Q282\0"
"\036\006WK2x01\0WK2801\0WK2401\0W6_5_1\0W6_6_1\0W6_HEL\0W6_HEL_I\0"
"\037\004Q303\0Q303\0CX35\0CX10D\0CX10WD\0"
"\040\0GW008\0"
"\041\0DM002\0"
"\042\010CABELL\0CAB_V3\0C_TELEM\0-\0-\0-\0-\0F_SAFE\0UNBIND\0"
"\043\002ESKY150\0004CH\0007CH\0"
"\044\004H8_3D\0H8_3D\0H20H\0H20Mini\0H30Mini\0"
"\045\003CORONA\0COR_V1\0COR_V2\0FD_V3\0"
"\046\0CFlie\0"
"\047\003Hitec\0OPT_FW\0OPT_HUB\0Minima\0"
"\050\0WFLY\0"
"\051\0BUGS\0"
"\052\002BUGSMINI\0BUGSMINI\0BUGS3H\0"
"\053\001Traxxas\0RX6519\0"
"\054\0NCC1701\0"
"\055\003E01X\0E012\0E015\0E016H\0"
"\056\002V911S\0V911S\0E119\0"
"\057\002GD00X\0GD_V1\0GD_V2\0"
"\060\002V761\0003CH\0004CH\0"
"\061\0KF606\0"
"\062\002Redpine\0Fast\0Slow\0"
"\063\001Potensic\0A20\0"
"\064\001ZSX\000280\0"
"\065\001Flyzone\0FZ-410\0"
"\066\0Scanner\0"
"\067\002Frsky_RX\0RX\0CloneTX\0"
"\070\0AFHDS2A_RX\0"
"\071\002HoTT\0Sync\0No_Sync\0"
"\072\001FX816\0P38\0"
"\073\0Bayang_RX\0"
"\074\002Pelikan\0Pro\0Lite\0"
"\075\0Tiger\0"
"\076\002XK\000X450\000X420\0"
"\077\004XN_DUMP\000250K\0001M\0002M\0AUTO\0"
"\100\005FrskyX2\0CH_16\0CH_8\0EU_16\0EU_8\0Cloned\0"
"\101\004FrskyR9\000915MHz\000868MHz\000915_8ch\000868_8ch\0"
"\102\001PROPEL\00074-Z\0"
"\103\002LR12\0LR12\0LR12_6ch\0"
"\104\0Skyartec\0"
"\105\001ESKYv2\000150V2\0"
"\106\0DSM_RX\0"
"\107\0JJRC345\0"
"\110\0Q90C\0"
"\0" ;

#if defined(PCBX12D) || defined(PCBX10)

extern void lcdDrawIcon( uint16_t x, uint16_t y, const uint8_t * bitmap, uint8_t type ) ;

const uint8_t IconX12Logging[] =
{
8,8,8,
0xff,0x01,0xfd,0x81,0x81,0x01,0x01,0xff
//,
//0x03,0x02,0x02,0x02,0x02,0x02,0x01,0x00
} ;
const uint8_t Icon24log[] =
{
#if defined(PCBX10)
#include "icon24loginv.lbm"
#else
#include "icon24log.lbm"
#endif
} ;

const uint8_t IconMusic[] =
{
#if defined(PCBX10)
#include "iconMusicinv.lbm"
#else
#include "iconMusic.lbm"
#endif
} ;
const uint8_t IconMix[] =
{
#if defined(PCBX10)
#include "iconMixinv.lbm"
#else
#include "iconMix.lbm"
#endif
} ;
const uint8_t IconHeli[] =
{
#if defined(PCBX10)
#include "iconHeliinv.lbm"
#else
#include "iconHeli.lbm"
#endif
} ;
const uint8_t IconDrate[] =
{
#if defined(PCBX10)
#include "iconDrateinv.lbm"
#else
#include "iconDrate.lbm"
#endif
} ;
const uint8_t IconBt[] =
{
#if defined(PCBX10)
#include "iconBtinv.lbm"
#else
#include "iconBt.lbm"
#endif
} ;
const uint8_t IconProto[] =
{
#if defined(PCBX10)
#include "iconProtoinv.lbm"
#else
#include "iconProto.lbm"
#endif
} ;
const uint8_t IconLimits[] =
{
#if defined(PCBX10)
#include "iconLimitsinv.lbm"
#else
#include "iconLimits.lbm"
#endif
} ;
const uint8_t IconCurve[] =
{
#if defined(PCBX10)
#include "iconCurveinv.lbm"
#else
#include "iconCurve.lbm"
#endif
} ;

const uint8_t IconTelem[] =
{
#if defined(PCBX10)
#include "iconTeleminv.lbm"
#else
#include "iconTelem.lbm"
#endif
} ;

const uint8_t IconVoice[] =
{
#if defined(PCBX10)
#include "iconVoiceinv.lbm"
#else
#include "iconVoice.lbm"
#endif
} ;

const uint8_t IconLswitch[] =
{
#if defined(PCBX10)
#include "iconLswitchinv.lbm"
#else
#include "iconLswitch.lbm"
#endif
} ;

const uint8_t IconTimer[] =
{
#if defined(PCBX10)
#include "iconTimerinv.lbm"
#else
#include "iconTimer.lbm"
#endif
} ;

#else
const uint8_t IconLogging[] =
{
5,8,5,
0xff,0xC1,0xdf,0xdf,0xff
} ;
#endif





// TSSI set to zero on no telemetry data
const int8_t TelemIndex[] = { FR_A1_COPY, FR_A2_COPY,	FR_RXRSI_COPY, FR_TXRSI_COPY,	TIMER1, TIMER2,	// 0-5
															FR_ALT_BARO, TELEM_GPS_ALT, FR_GPS_SPEED, FR_TEMP1, FR_TEMP2, FR_RPM,	// 6-11
														  FR_FUEL, FR_A1_MAH, FR_A2_MAH, FR_CELL_MIN,                           // 12-15
															BATTERY, FR_CURRENT, FR_AMP_MAH, FR_CELLS_TOT, FR_VOLTS,              // 16-20
															FR_ACCX, FR_ACCY,	FR_ACCZ, FR_VSPD, V_GVAR1, V_GVAR2,	V_GVAR3,        // 21-27
															V_GVAR4, V_GVAR5, V_GVAR6, V_GVAR7, FR_WATT, FR_RXV, FR_COURSE,       // 28-34
															FR_A3, FR_A4, V_SC1, V_SC2, V_SC3, V_SC4, V_SC5, V_SC6, V_SC7, V_SC8, V_RTC, TMOK,	// 35-46
															FR_AIRSPEED, FR_CELL1,FR_CELL2,FR_CELL3,FR_CELL4,FR_CELL5,FR_CELL6,FR_RBOX_B1_V,FR_RBOX_B1_A,	// 47-55
															FR_RBOX_B2_V,FR_RBOX_B2_A, FR_RBOX_B1_CAP, FR_RBOX_B2_CAP,FR_RBOX_SERVO,FR_RBOX_STATE,	// 56-61
															FR_CELL7, FR_CELL8, FR_CELL9, FR_CELL10, FR_CELL11, FR_CELL12,                          // 62-67
															FR_CUST1,FR_CUST2,FR_CUST3,FR_CUST4,FR_CUST5,FR_CUST6,FMODE,RUNTIME,MODELTIME,					// 68-76
															FR_CELLS_TOTAL1, FR_CELLS_TOTAL2, FR_SBEC_VOLT, FR_SBEC_CURRENT } ;	// 77-84


// TXRSSI is always considered valid as it is forced to zero on loss of telemetry
// Values are 0 - always valid, 1 - need telemetry, 2 - need hub, 3 - Scaler

// Make scalers always OK! Removed again
const uint8_t TelemValid[] = { 
1, 1, 1,
#ifdef ACCESS
1,	// SWR
#else
0,	// SWR
#endif
0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 2, 2, 0, 2, 
2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2, 2, 3, 3, 3, 
3, 3, 3, 3, 3, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 2, 2, 2, 2 } ;

uint8_t MaskRotaryLong ;

uint32_t checkForMenuEncoderLong( uint8_t event )
{
	MaskRotaryLong = 1 ;
	uint32_t result ;
	result = ( event==EVT_KEY_LONG(KEY_MENU)) || ( event==EVT_KEY_LONG(BTN_RE) ) ;
	if ( result )
	{
  	s_editMode = 0 ;
    killEvents(event);
	}
	return result ;
}

uint32_t checkForMenuEncoderBreak( uint8_t event )
{
	return ( event==EVT_KEY_BREAK(KEY_MENU)) || ( event==EVT_KEY_BREAK(BTN_RE) ) ;
}

uint32_t checkForExitEncoderLong( uint8_t event )
{
//	return ( event == EVT_KEY_FIRST(KEY_EXIT) ) || ( event == EVT_KEY_LONG(BTN_RE) ) ;

	if ( event == EVT_KEY_FIRST(KEY_EXIT) )
	{
		return 1 ;
	}
	
	if ( g_eeGeneral.disableBtnLong == 0 )
	{
		if ( event == EVT_KEY_LONG(BTN_RE) )
		{
			return 1 ;
		}
	}   
	return 0 ;
}


uint16_t getUnitsVoice( uint16_t unit )
{
	return UnitsVoice[unit] ;
}


static void displayGPSformat( uint16_t x, uint16_t y, uint8_t attr )
{
	lcd_putsAttIdx(  x, y, XPSTR("\012DD mm.mmmmDD.dddddd "), g_eeGeneral.gpsFormat, attr ) ;
}

void displayGPSdata( uint16_t x, uint16_t y, uint16_t whole, uint16_t frac, uint16_t attr )
{
	div_t qr ;
	qr = div( whole, 100 ) ;
	if ( g_eeGeneral.gpsFormat )
	{
		int32_t value ;
	  lcd_outdezNAtt(x+3*FW+2, y, qr.quot, attr, -3);
  	lcd_putc(x+3*FW+2, y, '.') ;
		value = qr.rem ;
		value *= 10000 ;
		value += frac ;
		value *= 10 ;
		value /= 6 ;
    lcd_outdezNAtt(x+9*FW, y, value, attr, -6);
	}
	else
	{
    lcd_outdezNAtt(x+3*FW, y, qr.quot, attr, -3);
    lcd_outdezNAtt(x+6*FW, y, qr.rem, attr, -2);
    lcd_putc(x+6*FW, y, '.') ;
    lcd_outdezNAtt(x+10*FW, y, frac, attr, -4);
	}
}

#ifdef ARUNI
extern void displayChangeWarning(const char *s);
/*
#ifdef ARUNI
#define ISWITCHES_STR "\003SA SB SC ID\200ID-ID\201SD SE SF L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSA\200SA-SA\201SB\200SB-SB\201SC\200SC-SC\201SD\200SD-SD\201SE\200SE-SE\2016P06P16P26P36P46P5PB1PB2PB3PB4"
#define IHW_SWITCHES_STR     "\004sIDxsSAxsSBxsSCxsSDxsSExsSFxL1  L2  L3  L4  L5  L6  L7  L8  L9  LA  LB  LC  LD  LE  LF  LG  LH  LI  LJ  LK  LL  LM  LN  LO  6POS"
#else
#define ISWITCHES_STR "\003THRRUDELEID\200ID-ID\201AILGEATRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfTH\200TH-TH\201RU\200RU-RU\201EL\200EL-EL\201AI\200AI-AI\201GE\200GE-GE\2016P06P16P26P36P46P5PB1PB2PB3PB4"
#define IHW_SWITCHES_STR     "\004sIDxsTHRsRUDsELEsAILsGEAsTRNL1  L2  L3  L4  L5  L6  L7  L8  L9  LA  LB  LC  LD  LE  LF  LG  LH  LI  LJ  LK  LL  LM  LN  LO  6POS"
#endif
*/
const char *AltSwitchNames =
  "ID " "SA " "SB " "SC " "SD " "SE " "SF " ;
// IDx   THR   RUD   ELE   AIL   GEA   TRN

static const char *getSwitchName(uint8_t z, char *s)
{
  const char *p = PSTR(SWITCHES_STR) + 1;
  if (g_eeGeneral.altSwitchNames) {
    if (z <= 8) { // THR:0,RUD:1,ELE:2,IDv:3,ID-:4,ID^:5,AIL:6,GEA:7,TRN:8
      p = AltSwitchNames;
      if (z >= 3) {
        z -= 3;
        if (z < 3)
          goto next;  // IDv:0,ID-:1,ID^:2
      }
      // THR:0,RUD:1,ELE:2,AIL:3,GEA:4,TRN:5
      p += 3;   // skip IDx and bump to THR/SA
    } else if (z >= 34 && z <= 48) {
      z -= 34;  //THv:0,TH-:1,TH^:2,RUv:3,RU-:4,RU^:5,...,GEv:12,GE-:13,GE^:14
      p = AltSwitchNames + 3 + (z / 3) * 3;  // skip IDx and add an offset
    next:
      s[0] = p[0];
      s[1] = p[1];
      s[2] = PSTR(HW_SWITCHARROW_STR)[z % 3];
      return s;
    }
  }
  return (p + z * 3);
}

static const char *getHwSwitchName(uint8_t z, char *s)
{
  if (z > 6 || !g_eeGeneral.altSwitchNames)
    return (PSTR(HW_SWITCHES_STR) + 1 + z * 4) ;

  // Compose a hardware switch name from the base switch name
  // "sIDx" "sSAx" "sSBx" "sSCx" "sSDx" "sSEx" "sSFx"
  //  sIDx   sTHR   sRUD   sELE   sAIL   sGEA   sTRN
  const char *p = AltSwitchNames + z * 3;
  s[0] = 's';
  s[1] = p[0];
  s[2] = p[1];
  s[3] = (p[2] != ' ') ? p[2] : 'x';
  return s;
}

// exported to "ersky9x.cpp"
void putSwitchName(uint8_t x, uint8_t y, uint8_t z, uint8_t att)
{
  char s[3];
  lcd_putsnAtt(x, y, getSwitchName(z, s), 3, att);
}

static void putHwSwitchName(uint8_t x, uint8_t y, uint8_t z, uint8_t att)
{
  char s[4];
  lcd_putsnAtt(x, y, getHwSwitchName(z, s), 4, att);
}

static void setLastHwSwitch(uint8_t z)
{
  char s[4];
  ncpystr((uint8_t *)LastItem, (uint8_t *)getHwSwitchName(z, s), 4);
}
#else
void putSwitchName(uint8_t x, uint8_t y, int8_t z, uint8_t att)
{
  lcd_putsAttIdx(x, y, PSTR(SWITCHES_STR), z, att);
}

static void putHwSwitchName(uint8_t x, uint8_t y, uint8_t z, uint8_t att)
{
  lcd_putsAttIdx(x, y, PSTR(HW_SWITCHES_STR), z, att);
}

static void setLastHwSwitch(uint8_t z)
{
  setLastIdx( (char *)PSTR(HW_SWITCHES_STR), z ) ;
}
#endif  // ARUNI

//int8_t edit_dr_switch( uint8_t x, uint8_t y, int8_t drswitch, uint8_t attr, uint8_t flags, uint8_t event ) ;

// parse States
#define GET_INDEX 		0
#define GET_NAME			1
#define	GET_SUB_NAME	2
#define	SKIP_LINE			3

union t_sharedMemory SharedMemory ;

//FIL TextFile ;
//uint8_t TextMenuBuffer[16*21] ;
//uint8_t TextMenuStore[16*21+32] ;	// Allow for CRLF
//uint8_t TextLines ;
//uint8_t TextOffset ;
//uint8_t TextHelp ;
//uint8_t HelpTextPage ;
//uint8_t TextFileOpen ;

//This is used at startup to read "Multi.txt", then used to load
// Basic scripts, and also used by a standalone script for file access
FIL MultiBasicFile ;

static int32_t memCaseCmp( uint8_t *p, uint8_t *q, uint32_t length )
{
	int32_t a ;
	int32_t b ;
	while ( length-- )
	{
		a = toupper(*p++) ;
		b = toupper(*q++) ;
		a -= b ;
		if ( a )
		{
			break ;
		}
	}
	return a ;
}

static void parsexMultiData()
{
	uint8_t *src ;
	uint32_t index ;
	uint32_t count ;
	src = (uint8_t *)MultiData ;

	while(1)
	{
		index = *src++ ;
		if ( index == 0 )
		{
			break ;
		}
		count = *src++ ;

		Multiprotocols[index] = ( src - (uint8_t *)MultiData ) + (count << 11) ;
		for ( index = 0 ; index <= count ; index += 1 )
		{
			while ( *src++ != '\0' ) ;
		}
	}
}

void parseMultiData()
{
	uint8_t *text ;
	uint32_t index ;
	uint32_t offset ;
	uint32_t count ;
	uint32_t state ;
	uint8_t byte ;
	FRESULT result ;

  parsexMultiData() ;
	WatchdogTimeout = 300 ;		// 3 seconds
	index = 0 ;
	while ( !sd_card_ready() )
	{
		CoTickDelay(5) ;					// 10mS for now
		if ( ++index > 24 )
		{
			break ;
		}
		wdt_reset() ;
	}

	result = f_open( &MultiBasicFile, "/Multi.txt", FA_READ ) ;

	text = MultiText ;
	state = GET_INDEX ;
	index = 0 ;
	count = 0 ;
	offset = 0 ;
//	mfoffset = 

	if ( result == FR_OK )
	{
		for(;;)
		{
			uint32_t next = 0 ;
			byte = 0 ;
			f_read( &MultiBasicFile, &byte, 1, 0 ) ;
			if ( byte == '\r' )
			{
				continue ;
			}
			if ( byte == '\n' )
			{
				byte = ',' ;
				next = 1 ;
			}
			if ( byte == 0 )
			{
				next = 2 ;
				byte = ',' ;
			}
			switch ( state )
			{
				case GET_INDEX :
					if ( byte == '#' )
					{
						state = SKIP_LINE ;
						index = 0 ;					
					}
					else if ( byte == ',' )
					{
						state = GET_NAME ;
						offset = text - MultiText ;
					}
					else
					{
						index *= 10 ;
						index += byte - '0' ;
					}
				break ;

				case GET_NAME :
					if ( byte == ',' )
				 	{
						*text++ = '\0' ;
						if ( index < NUM_MULTI_PROTOCOLS )
						{
	//						nextEntry = 0x8000 | offset ;
							state = GET_SUB_NAME ;
						}
					}
					else
					{
						if ( text - MultiText < MULTI_TEXT_SIZE - 2 )
						{
							*text++ = byte ;
						}
					}
				break ;

				case GET_SUB_NAME :
					if ( byte == ',' )
				 	{
						*text++ = '\0' ;
						count += 1 ;
					}
					else
					{
						if ( text - MultiText < MULTI_TEXT_SIZE - 2 )
						{
							*text++ = byte ;
						}
					}
				break ;
				case SKIP_LINE :
				
				break ;
			}
			if ( next )
			{
				state = GET_INDEX ;
				// check to see if data matches flash data
				if ( index <= NUM_MULTI_PROTOCOLS )
				{
					if ( count > 8 )
					{
						count = 8 ;
					}
					if ( Multiprotocols[index] )
					{
						uint32_t xcount ;
						xcount = ( Multiprotocols[index] >> 11 ) & 0x000F ;
						if ( ( memCaseCmp( (uint8_t *)&MultiData[Multiprotocols[index] & 0x07FF], &MultiText[offset], text - &MultiText[offset] ) != 0 ) || ( count != xcount ) )
						{
							Multiprotocols[index] = 0x8000 | offset | count << 11 ;
						}
						else
						{
							text = &MultiText[offset] ;
						}
					}
					else
					{
						Multiprotocols[index] = 0x8000 | offset | count << 11 ;
					}
				}
				index = 0 ;
				count = 0 ;
				if ( next == 2 )
				{
					break ;
				}
			}
		}
	}
	*text = '\0' ;
	if ( result == FR_OK )
	{
		f_close( &MultiBasicFile ) ;
	}
//	offset = 0 ;
//	for ( index = 1 ; index < NUM_MULTI_PROTOCOLS ; index += 1 )
//	{
//		if ( Multiprotocols[index] & 0x8000 )
//		{
//			MultiMapping[offset++] = index - 1 ;
//		}
//	}
//	for ( index = 1 ; index < 64 ; index += 1 )
//	{
//		if ( index < NUM_MULTI_PROTOCOLS )
//		{
//			if ( ( Multiprotocols[index] & 0x8000 ) == 0 )
//			{
//				MultiMapping[offset++] = index - 1 ;
//			}
//		}
//		else
//		{
//			MultiMapping[offset++] = index - 1 ;
//		}
//	}
}

void displayMultiProtocol( uint32_t index, uint32_t y, uint8_t attr )
{
	char *p ;

	if ( index == 53 )
	{
		lcd_putsAtt( FW*10, y, "Scanner", attr ) ;
		return ;
	}
	if ( PrivateData[1] )
	{
		if ( ( PrivateData[0] & 0x04 ) == 0 )
		{
			lcd_putsAtt( FW*18, y, "N/A", BLINK ) ;
		}
	}
	if ( MultiSetting.protocol[0] )
	{
		lcd_putsAtt( FW*10, y, (const char *)MultiSetting.protocol, attr ) ;
		return ;
	}

	index += 1 ;
	if ( index <= NUM_MULTI_PROTOCOLS+1 )
	{
		uint16_t value = Multiprotocols[index] ;
		if ( value )
		{
			p = ( value & 0x8000 ) ? (char *)MultiText : (char *)MultiData ;
			value &= 0x07FF ;
			lcd_putsAtt( FW*10, y, &p[value], attr ) ;
			return ;
		}
	}
	lcd_outdezAtt( 17*FW, y, index, attr ) ;
}

uint32_t displayMultiSubProtocol( uint32_t index, uint32_t subIndex, uint32_t y, uint8_t attr )
{
	char *p ;
	
	if ( MultiSetting.subProtocol[0] )
	{
		lcd_putsAtt( 10*FW, y, (const char *)MultiSetting.subProtocol, attr ) ;
		return subIndex ;
	}
	index += 1 ;

	if ( index <= NUM_MULTI_PROTOCOLS )
	{
		uint16_t value = Multiprotocols[index] ;
		if ( value )
		{
			uint32_t limit = (value >> 11) & 0x0F ;
			if ( subIndex < limit )
			{
				p = ( value & 0x8000 ) ? (char *)MultiText : (char *)MultiData ;
				value &= 0x07FF ;
				uint8_t *list = (uint8_t *)&p[value] ;
				value = subIndex ;
				do
				{
					while ( *list )
					{
						list += 1 ;
					}
					list += 1 ;
				}
				while ( value-- ) ;
				lcd_putsAtt( FW*10, y, (char *)list, attr ) ;
			}
			else
			{
				if ( subIndex == 0 )
				{
					lcd_putsAtt( FW*10, y, "None", attr ) ;
	//				return 0 ;
				}
				else
				{
					lcd_outdezAtt( 21*FW, y, subIndex, attr ) ;
				}
			}
			return subIndex ;
		}
	}
	lcd_outdezAtt( 21*FW, y, subIndex, attr ) ;
	return subIndex ;
}

//uint8_t editMultiProtocol( uint8_t index, uint8_t edit )
//{
//	for ( uint32_t c = 0 ; c < 63 ; c += 1 )
//	{
//		if ( MultiMapping[c] == index )
//		{
//			index = c ;
//			break ;
//		}
//	}
//	if ( edit )
//	{
// 		CHECK_INCDEC_H_MODELVAR_0( index, 62 ) ;
//	}
//	return MultiMapping[index] ;
//}

SKYMixData *mixAddress( uint32_t index )
{
	SKYMixData *md2 = &g_model.mixData[index] ;
#if EXTRA_SKYMIXERS
	if ( index >= MAX_SKYMIXERS )
	{
		md2 = &g_model.exmixData[index-MAX_SKYMIXERS] ;
	}
#endif
	return md2 ;
}

uint32_t putTxSwr( uint8_t x, uint8_t y, uint8_t attr )
{
	uint32_t index = 0 ;
	if ( FrskyTelemetryType == FRSKY_TEL_SPORT )
	{
		index = 1 ;
	}
	if ( ( index == 0 ) && (attr & TSSI_TEXT) )
	{
		return 0 ;
	}
	lcd_putsAttIdx( x, y, PSTR(STR_TXEQ), index, attr ) ;
	setLastIdx( (char *) PSTR(STR_TXEQ), index ) ;
	return 1 ;
}

void putsAttIdxTelemItems( uint8_t x, uint8_t y, uint8_t index, uint8_t attr )
{
	if ( index == 4 )
	{
		if ( putTxSwr( x, y, attr ) )
		{
			return ;
		}
	}
	attr &= ~TSSI_TEXT ;
	if ( index >= TELEM_GAP_START + 8 )
	{
		index -= 8 ;
	}
	setLastTelemIdx( index ) ;
	lcd_putsAtt(x,y,LastItem,attr);
//	lcd_putsAttIdx( x, y, PSTR(STR_TELEM_ITEMS), index, attr ) ;
//	setLastIdx( (char *) PSTR(STR_TELEM_ITEMS), index ) ;
}

// knots to km/h, multiply by 1.852
// knots to mph, multiply by 1.15078
// type is 0 for km/h, 1 for mph
#ifndef SMALL
int16_t knots_to_other( int16_t knots, uint32_t type )
{
	if ( type )
	{
		return knots * 57539 / 50000 ;	// OK to 36000 kts
	}
	return (int32_t)knots * 1852 / 1000 ;
}
#endif

int16_t m_to_ft( int16_t metres )
{
	int16_t result ;

  // m to ft *105/32
	result = metres * 3 ;
	metres >>= 2 ;
	result += metres ;
	metres >>= 2 ;
  return result + (metres >> 1 );
}

int16_t c_to_f( int16_t degrees )
{
  degrees += 18 ;
  degrees *= 115 ;
  degrees >>= 6 ;
  return degrees ;
}

int16_t calc_scaler( uint8_t index, uint16_t *unit, uint8_t *num_decimals)
{
	int32_t value ;
	int32_t exValue ;
	uint8_t lnest ;
	ScaleData *pscaler ;
	ExtScaleData *epscaler ;
	
	lnest = CalcScaleNest ;
	if ( lnest > 5 )
	{
		return 0 ;
	}
	CalcScaleNest = lnest + 1 ;
	// process
	pscaler = &g_model.Scalers[index] ;
	epscaler = &g_model.eScalers[index] ;
	if ( pscaler->source )
	{
		value = getValue( pscaler->source - 1 ) ;
		if ( ( pscaler->source == NUM_SKYXCHNRAW+1 ) || ( pscaler->source == NUM_SKYXCHNRAW+2 ) )
		{
			value = scale_telem_value( value, pscaler->source - NUM_SKYXCHNRAW-1, NULL ) ;
		}
	}
	else
	{
		value = 0 ;
	}
	if ( !pscaler->offsetLast )
	{
		value += pscaler->offset ;
	}
	uint16_t t ;
	t = pscaler->mult + ( pscaler->multx << 8 ) ;
	value *= t+1 ;
	t = pscaler->div + ( pscaler->divx << 8 ) ;
	value /= t+1 ;
	if ( epscaler->mod )
	{
		value %= epscaler->mod+1 ;
	}
	if ( epscaler->exSource )
	{
		exValue = getValue( epscaler->exSource - 1 ) ;
		if ( ( epscaler->exSource == NUM_SKYXCHNRAW+1 ) || ( epscaler->exSource == NUM_SKYXCHNRAW+2 ) )
		{
			exValue = scale_telem_value( exValue, epscaler->exSource - NUM_SKYXCHNRAW-1, NULL ) ;
		}
		if ( pscaler->exFunction )
		{
			switch ( pscaler->exFunction )
			{
				case 1 :	// Add
					value += exValue ;
				break ;
				case 2 :	// Subtract
					value -= exValue ;
				break ;
				case 3 :	// Multiply
					value *= exValue ;
				break ;
				case 4 :	// Divide
					if ( exValue )
					{
						value /= exValue ;
					}
				break ;
				case 5 :	// Mod
					if ( exValue )
					{
						value %= exValue ;
					}
				break ;
				case 6 :	// Min
					if ( exValue )
					{
						if ( exValue < value )
						{
							value = exValue ;
						}
					}
				break ;
			}
		}
	}
	CalcScaleNest = lnest ;
	if ( pscaler->offsetLast )
	{
		value += pscaler->offset ;
	}
	if ( pscaler->neg )
	{
		value = -value ;
	}
	// Limit to an int16_t
	if ( value > 32767 )
	{
		value = 32767 ;
	}
	if ( value < -32768 )
	{
		value = -32768 ;
	}
	if ( unit )
	{
		*unit = pscaler->unit ;
	}
	if ( num_decimals )
	{
		*num_decimals = pscaler->precision ;
	}
	if ( epscaler->dest )
	{
		store_telemetry_scaler( epscaler->dest, value ) ;
	}

	return value ;
}
									 
uint8_t telemItemValid( uint8_t index )
{
	uint8_t x ;

	if ( index >= TELEM_GAP_START + 8 )
	{
		index -= 8 ;
	}
	x = pgm_read_byte( &TelemValid[index] ) ;
	if ( x == 3 )
	{
		// A scaler
		uint8_t i = g_model.Scalers[index-TEL_ITEM_SC1].source - 1 ;
  	if( (i<CHOUT_BASE+NUM_SKYCHNOUT) || ( i >= EXTRA_POTS_START - 1 ) )
		{
			x = 0 ;
		}
		else
		{
			i -= CHOUT_BASE+NUM_SKYCHNOUT ;
			x = pgm_read_byte( &TelemValid[i] ) ;
			if ( x == 3 )
			{
				x = 0 ;
			}
			else
			{
				index = i ;
				if ( index >= TELEM_GAP_START + 8 )
				{
					index -= 8 ;
				}
			}
		}
	}
	if ( x == 0 )
	{
		return 1 ;
	}
	if ( x == 1 )
	{
		if ( frskyStreaming )
		{
			int8_t i = TelemIndex[index] ;
			if ( i >= 0 )
			{
				if (TelemetryDataValid[i] )
				{
					return 1 ;
				}
				return 0 ;
			}
			return 1 ;
		}
	}
	else // if ( frskyUsrStreaming )
	{
		int8_t i = TelemIndex[index] ;
		if ( i >= 0 )
		{
			if (TelemetryDataValid[i] )
			{
				return 1 ;
			}
			return 0 ;
		}
		return 1 ;
	}
	return 0 ;	
}

#define WBAR2 (50/2)
void singleBar( uint8_t x0, uint8_t y0, int16_t val )
{
  int16_t limit = (g_model.extendedLimits ? 1280 : 1024);
  int8_t l = (abs(val) * WBAR2 + 512) / limit ;
  if(l>WBAR2)  l =  WBAR2;  // prevent bars from going over the end - comment for debugging

#if defined(PCBX12D) || defined(PCBX10)
	pushPlotType( PLOT_BLACK ) ;
#endif
  lcd_hlineStip(x0-WBAR2,y0,WBAR2*2+1,0x55);
  lcd_vline(x0,y0-2,5);
  if(val>0)
	{
    x0+=1;
  }else{
    x0-=l;
  }
  lcd_hline(x0,y0+1,l);
  lcd_hline(x0,y0-1,l);
#if defined(PCBX12D) || defined(PCBX10)
		popPlotType() ;
#endif
}


void voiceMinutes( int16_t value )
{
	voice_numeric( value, 0, (abs(value) == 1) ? SV_MINUTE : SV_MINUTES ) ;
}


void voice_telem_item( int8_t index )
{
	int16_t value ;
	uint8_t spoken = 0 ;
	uint16_t unit = 0 ;
	uint8_t num_decimals = 0 ;

	value = get_telemetry_value( index ) ;
	if (telemItemValid( index ) == 0 )
	{
//#ifdef TELEMETRY_LOST
//		if ( frskyStreaming )
//		{
//#endif
		putSystemVoice( SV_NO_TELEM, V_NOTELEM ) ;
//#ifdef TELEMETRY_LOST
//		}
//#endif
		spoken = 1 ;
	}
	if ( index >= TELEM_GAP_START + 8 )
	{
		index -= 8 ;
	}
	index = pgm_read_byte( &TelemIndex[index] ) ;

  switch (index)
	{
		case V_SC1 :
		case V_SC2 :
		case V_SC3 :
		case V_SC4 :
		case V_SC5 :
		case V_SC6 :
		case V_SC7 :
		case V_SC8 :
			value = calc_scaler( index-V_SC1, &unit, &num_decimals ) ;
			unit = getUnitsVoice( unit ) ;
		break ;
		
		case BATTERY:
		case FR_VOLTS :
		case FR_CELLS_TOT :
		case FR_CELLS_TOTAL1 :
		case FR_CELLS_TOTAL2 :
			unit = SV_VOLTS ;			
			num_decimals = 1 ;
		break ;

		case FR_CELL_MIN:
			if ( value > 445 )
			{
				value = 0 ;
			}
		case FR_CELL1:
		case FR_CELL2:
		case FR_CELL3:
		case FR_CELL4:
		case FR_CELL5:
		case FR_CELL6:
		case FR_CELL7:
		case FR_CELL8:
		case FR_CELL9:
		case FR_CELL10:
		case FR_CELL11:
		case FR_CELL12:
		case FR_RBOX_B1_V :
		case FR_RBOX_B2_V :
		case FR_SBEC_VOLT :
			unit = SV_VOLTS ;			
			num_decimals = 2 ;
		break ;
			
		case TIMER1 :
		case TIMER2 :
		{	
			div_t qr ;
			qr = div( value, 60 ) ;
			voiceMinutes( qr.quot ) ;
			value = qr.rem ;
			unit = SV_SECONDS ;			
		}
		break ;

    case V_RTC :
			voice_numeric( Time.hour, 0, 0 ) ;
			voice_numeric( Time.minute, 0, 0 ) ;
			spoken = 1 ;
		break ;
		 
		case V_GVAR1 :
		case V_GVAR2 :
		case V_GVAR3 :
		case V_GVAR4 :
		case V_GVAR5 :
		case V_GVAR6 :
		case V_GVAR7 :
			value = g_model.gvars[index-V_GVAR1].gvar ;
		break ;

		case FR_A3:
		case FR_A4:
		{	
			uint8_t channel = index-FR_A3 ;
			
			uint8_t ltype = g_model.frsky.channels[channel].units3_4 ;
			unit = SV_VOLTS ;			
			num_decimals = 2 ;
			if (ltype == 3)
			{
				unit = SV_AMPS ;
			}
			else if (ltype == 1)
			{
				unit = 0 ;
				num_decimals = 0 ;
			}
		}
		break ;
		 
		case FR_A1_COPY:
    case FR_A2_COPY:
		{	
			uint8_t channel = index-FR_A1_COPY ;
			
			uint8_t ltype = g_model.frsky.channels[channel].units ;
			value = A1A2toScaledValue( channel, &num_decimals ) ;
			unit = SV_VOLTS ;			
			if (ltype == 3)
			{
				unit = SV_AMPS ;
			}
			else if (ltype == 1)
			{
				unit = 0 ;
				num_decimals = 0 ;
			}
		}
		break ;

    case FR_RXV :
			value = convertRxv( value ) ;			
			unit = SV_VOLTS ;			
			num_decimals = 1 ;
		break ;

		case FR_ALT_BARO:
		case TELEM_GPS_ALT :
      unit = SV_METRES ;
			if (g_model.FrSkyUsrProto == 1)  // WS How High
			{
      	if ( g_model.FrSkyImperial )
        	unit = SV_FEET ;
			}
      else
			{
				if ( g_model.FrSkyImperial )
      	{
	        // m to ft *105/32
  	      value = m_to_ft( value ) ;
    	    unit = SV_FEET ;
      	}
			}
			if ( value < 1000 )
			{
				num_decimals = 1 ;
			}
			else
			{
				value /= 10 ;
			}
		break ;

#ifdef MAVLINK 
// Extra data for Mavlink via FrSky		 
		case FR_WP_DIST:
  		if ( g_model.FrSkyImperial )
  		{
				value = m_to_ft(value) ;
			}
		break ;
// Extra data for Mavlink via FrSky
#endif

		case FR_RBOX_B1_A :
		case FR_RBOX_B2_A :
		case FR_SBEC_CURRENT :
			num_decimals = 2 ;
      unit = SV_AMPS ;
		break ;
		
		case FR_RBOX_B1_CAP :
		case FR_RBOX_B2_CAP :
      unit = SV_MILAMP_H ;
		break ;
		
		case FR_CURRENT :
			num_decimals = 1 ;
      unit = SV_AMPS ;
		break ;

		case FR_AIRSPEED :
			num_decimals = 1 ;
      unit = SV_METRES ;
			if ( g_model.FrSkyImperial )
			{
       	// m to ft *105/32
       	value = m_to_ft( value ) ;
	      unit = 'f' ;
			}
		break ;
			 
		case FR_TEMP1:
		case FR_TEMP2:
			unit = SV_DEGREES ;			
  		if ( g_model.FrSkyImperial )
  		{
				value = c_to_f(value) ;
			}
		break ;

		case FR_WATT :
			unit = SV_WATTS ;
		break ;

		case FR_FUEL :
			unit = SV_PERCENT ;
		break ;

		case FR_VSPD :
			num_decimals = 1 ;
			value /= 10 ;
		break ;

	}

	if ( spoken == 0 )
	{
		voice_numeric( value, num_decimals, unit ) ;
	}
}


// This routine converts an 8 bit value for custom switch use
int16_t convertTelemConstant( int8_t channel, int8_t value)
{
  int16_t result;

	if ( channel >= TELEM_GAP_START + 8 )
	{
		channel -= 8 ;
	}
	channel = TelemIndex[channel] ;
	result = value + 125 ;
	if ( ( channel <= V_GVAR7 ) && ( channel >= V_GVAR1 ) )
	{
		return value ;
	}

  switch (channel)
	{
    case V_RTC :
      result *= 12 ;
    break;
    case MODELTIME :
      result *= 20 ;
    break ;
    case RUNTIME :
      result *= 3 ;
    break ;
    case TIMER1 :
    case TIMER2 :
      result *= 10 ;
    break;
    case FR_ALT_BARO:
    case TELEM_GPS_ALT:
			if ( result > 63 )
			{
      	result *= 2 ;
      	result -= 64 ;
			}
			if ( result > 192 )
			{
      	result *= 2 ;
      	result -= 192 ;
			}
			if ( result > 448 )
			{
      	result *= 2 ;
      	result -= 488 ;
			}
			result *= 10 ;		// Allow for decimal place
      if ( g_model.FrSkyImperial )
      {
        // m to ft *105/32
        value = m_to_ft( result ) ;
      }
    break;
    case FR_RPM:
      result *= 100;
    break;
    case FR_TEMP1:
    case FR_TEMP2:
      result -= 30;
    break;
    case FR_A1_MAH:
    case FR_A2_MAH:
		case FR_AMP_MAH :
		case FR_RBOX_B1_CAP :
		case FR_RBOX_B2_CAP :
      result *= 50;
    break;

		case FR_CELL1:
		case FR_CELL2:
		case FR_CELL3:
		case FR_CELL4:
		case FR_CELL5:
		case FR_CELL6:
		case FR_CELL7:
		case FR_CELL8:
		case FR_CELL9:
		case FR_CELL10:
		case FR_CELL11:
		case FR_CELL12:
		case FR_CELL_MIN:
		case FR_SBEC_VOLT :
      result *= 2;
		break ;
		case FR_CELLS_TOT :
		case FR_VOLTS :
		case FR_CELLS_TOTAL1 :
		case FR_CELLS_TOTAL2 :
      result *= 2;
		break ;
		case FR_RBOX_B1_V :
		case FR_RBOX_B2_V :
      result *= 4 ;
		break ;
		case FR_RBOX_B1_A :
		case FR_RBOX_B2_A :
		case FR_SBEC_CURRENT :
      result *= 20;
		break ;
    case FR_WATT:
      result *= 8 ;
    break;
		case FR_VSPD :
			result = value * 10 ;
		break ;
		case FMODE :
			result = value ;
		break ;
  }
  return result;
}


int16_t get_telemetry_value( int8_t channel )
{
//#ifndef TELEMETRY_LOST
	if (telemItemValid( channel ) == 0 )
	{
		return 0 ;
	}
//#endif	
	if ( channel >= TELEM_GAP_START + 8 )
	{
		channel -= 8 ;
	}
	channel = pgm_read_byte( &TelemIndex[channel] ) ;
	if ( ( channel <= V_SC8 ) && ( channel >= V_SC1 ) )	// A Scaler
	{
		return calc_scaler(channel-V_SC1, 0, 0 ) ;
	}
	if ( ( channel <= V_GVAR7 ) && ( channel >= V_GVAR1 ) )	// A GVAR
	{
		return g_model.gvars[channel-V_GVAR1].gvar ;
	}
  switch (channel)
	{
    case RUNTIME :
    return g_eeGeneral.totalElapsedTime / 60 ;

    case MODELTIME :
    return g_model.totalTime / 60 ;

    case TIMER1 :
    case TIMER2 :
    return s_timer[channel+2].s_timerVal ;
    
    case BATTERY :
    return g_vbat100mV ;

    case FR_ALT_BARO :
		return FrskyHubData[channel] + AltOffset ;

    case TMOK :
		return TmOK ;
    
		case V_RTC :
		return Time.hour * 60 + Time.minute ;
    
		case FR_WATT :
		return (uint32_t)FrskyHubData[FR_VOLTS] * (uint32_t)FrskyHubData[FR_CURRENT] / (uint32_t)100 ;
    
		case FMODE :
		return getFlightPhase() ;
    
		default :
		return FrskyHubData[channel] ;
  }
}

void displayTimer( uint8_t x, uint8_t y, uint8_t timer, uint8_t att )
{
	struct t_timer *tptr = &s_timer[timer] ;
  att |= (tptr->s_timerState==TMR_BEEPING ? BLINK : 0);
  putsTime( x, y, tptr->s_timerVal, att, att ) ;
}

char *arduFlightMode( uint16_t mode )
{
//Arduplane flight mode numbers:

//0 Manual
//1 CIRCLE
//2 STABILIZE
//3 TRAINING
//4 ACRO
//5 FBWA
//6 FBWB
//7 CRUISE
//8 AUTOTUNE
//10 Auto
//11 RTL
//12 Loiter
//15 Guided

//Arducopter flight mode numbers:

//0 Stabilize
//1 Acro
//2 AltHold
//3 Auto
//4 Guided
//5 Loiter
//6 RTL
//7 Circle
//9 Land
//11 Drift
//13 Sport
//14 Flip
//15 AutoTune
//16 PosHold
//17 Brake

// To add:
// 18: Throw
// 19: Avoid ADSB
// 20: Guided No GPS
// 21: Smart RTL
// 22: Flow Hold
// 23: Follow


#define STR_MAV_FM_STAB  "STAB"
#define STR_MAV_FM_ACRO  "ACRO"
#define STR_MAV_FM_2     "A-Hold"
#define STR_MAV_FM_AUTO  "AUTO"
#define STR_MAV_FM_GUIDED "GUIDED"
#define STR_MAV_FM_LOITER "LOITER"
#define STR_MAV_FM_RTL 	 "RTL"
#define STR_MAV_FM_CIRCLE "CIRCLE"
//#define STR_MAV_FM_8     "MODE8"
#define STR_MAV_FM_9 		 "LAND"
#define STR_MAV_FM_11		 "DRIFT"
#define STR_MAV_FM_13		 "SPORT"
#define STR_MAV_FM_14		 "FLIP"
#define STR_MAV_FM_ATUNE "A-TUNE"
#define STR_MAV_FM_16		 "POSHOLD"
#define STR_MAV_FM_17		 "BRAKE"
#define STR_MAV_FM_MAN	 "MANUAL"
#define STR_MAV_FM_TRAIN "TRAIN"
#define STR_MAV_FM_FBWA	 "FBWA"
#define STR_MAV_FM_FBWB	 "FBWB"
#define STR_MAV_FM_CRUISE "CRUISE"
#define STR_MAV_FM_FLIP	 "FLIP"
#define STR_MAV_FM_SRTL	 "SRTL"
#define STR_MAV_FM_X		 "UNKNOWN"

	uint32_t type = mode >> 8 ;
	mode &= 0xFF ;

	const char *s ;
	if ( g_model.telemetryProtocol == TELEMETRY_MAVLINK )
	{
		type = type == 1  ;
	}
	else
	{
		type = g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ;
	}
	switch ( mode )
	{
		case  0 : s = type ? STR_MAV_FM_MAN : STR_MAV_FM_STAB ; break ;
		case  1 : s = type ? STR_MAV_FM_CIRCLE : STR_MAV_FM_ACRO ; break	;
		case  2 : s = type ? STR_MAV_FM_STAB : STR_MAV_FM_2 ; break	;
		case  3 : s = type ? STR_MAV_FM_TRAIN : STR_MAV_FM_AUTO ; break	;
		case  4 : s = type ? STR_MAV_FM_ACRO : STR_MAV_FM_GUIDED ; break	;
		case  5 : s = type ? STR_MAV_FM_FBWA : STR_MAV_FM_LOITER ; break	;
		case  6 : s = type ? STR_MAV_FM_FBWB : STR_MAV_FM_RTL ; break	;
		case  7 : s = type ? STR_MAV_FM_CRUISE : STR_MAV_FM_CIRCLE ; break	;
		case  8 : s = type ? STR_MAV_FM_ATUNE : STR_MAV_FM_X ; break	;
		case  9 : s = type ? STR_MAV_FM_X : STR_MAV_FM_9 ; break	;
		case  10 : s = type ? STR_MAV_FM_AUTO : STR_MAV_FM_X ; break	;
		case  11 : s = type ? STR_MAV_FM_RTL : STR_MAV_FM_11 ; break	;
		case  12 : s = type ? STR_MAV_FM_LOITER : STR_MAV_FM_X ; break	;
		case  13 : s = type ? STR_MAV_FM_X: STR_MAV_FM_13 ; break	;
		case  14 : s = type ? STR_MAV_FM_X: STR_MAV_FM_FLIP ; break	;
		case  15 : s = type ? STR_MAV_FM_GUIDED: STR_MAV_FM_ATUNE ; break	;
		case  16 : s = type ? STR_MAV_FM_X : STR_MAV_FM_16 ; break	;
		case  17 : s = type ? STR_MAV_FM_X : STR_MAV_FM_17 ; break	;
		case  21 : s = STR_MAV_FM_SRTL ; break ;
		default : s = STR_MAV_FM_X ; break ;
	}
	return (char *) s ;
}

// Styles
#define TELEM_LABEL				0x01
#define TELEM_UNIT    		0x02
#define TELEM_UNIT_LEFT		0x04
#define TELEM_VALUE_RIGHT	0x08
#define TELEM_NOTIME_UNIT	0x10
#define TELEM_ARDUX_NAME	0x20
#define TELEM_CONSTANT		0x80

uint8_t putsTelemetryChannel(uint8_t x, uint8_t y, int8_t channel, int16_t val, uint8_t att, uint8_t style)
{
	uint16_t unit = ' ' ;
	uint8_t xbase = x ;
	uint8_t fieldW = FW ;
	uint8_t displayed = 0 ;
	uint8_t valid = telemItemValid( channel ) | (style & TELEM_CONSTANT ) ;
	uint8_t mappedChannel = ( channel >= TELEM_GAP_START + 8 ) ? channel - 8 : channel ;
	if ( style & TELEM_LABEL )
	{
		uint8_t displayed = 0 ;
		int8_t index = TelemIndex[mappedChannel] ;
		uint8_t special = 0 ;

		if ( (index >= V_SC1) && (index < V_SC1 + NUM_SCALERS) )
		{
			special = 1 ;
		}
		if ( (index >= FR_CUST1) && (index <= FR_CUST6) )
		{
			special = 2 ;
		}

		if ( special )
		{
			uint8_t *p ;
			uint8_t text[6] ;
			if ( special & 1 )
			{
				index -= V_SC1 ;
				p = &g_model.Scalers[index].name[0] ;
			}
			else
			{
				index -= FR_CUST1 ;
				p = &g_model.customTelemetryNames[index*4] ;
			}
			text[0] = *p++ ;
			if ( text[0] && (text[0] != ' ') )
			{
				text[1] = *p++ ;
				text[2] = *p++ ;
				text[3] = *p ;
				text[4] = 0 ;
				lcd_putsAtt( x, y, (const char *)text, 0 ) ;
				displayed = 1 ;
			}
		}
		if ( displayed == 0 )
		{
  		putsAttIdxTelemItems( x, y, channel+1, 0 ) ;
		}
		x += 4*FW ;
		if ( att & DBLSIZE )
		{
			x += 4 ;
			y -= FH ;												
			fieldW += FW ;
		}
	}

	if (style & TELEM_VALUE_RIGHT)
	{
		att &= ~LEFT ;
	}
	channel = TelemIndex[mappedChannel] ;
  switch (channel)
	{
		case V_SC1 :
		case V_SC2 :
		case V_SC3 :
		case V_SC4 :
		case V_SC5 :
		case V_SC6 :
		case V_SC7 :
		case V_SC8 :
		{
			int16_t cvalue ;
			uint8_t precision ;
			cvalue = calc_scaler( channel-V_SC1, &unit, &precision ) ;
			if ( precision == 1 )
			{
				att |= PREC1 ;
			}
			else if ( precision == 2 )
			{
				att |= PREC2 ;
			}
			// Sort units here
			unit = UnitsText[unit] ;
			if ( (style & TELEM_CONSTANT) == 0)
			{
				val = cvalue ;
			}
		}	
		break ;

    case MODELTIME :
    case RUNTIME :
//		break ;
    case V_RTC :
			style |= TELEM_NOTIME_UNIT ;
		case TIMER1 :
    case TIMER2 :
			style &= ~( TELEM_UNIT | TELEM_UNIT_LEFT ) ;
			
			if ( (att & DBLSIZE) == 0 )
			{
				x -= 4 ;
			}
			if ( style & TELEM_LABEL )
			{
				x += FW+4 ;
			}
//			att &= DBLSIZE ;
	  	if (att & DBLSIZE)
  		{
				if ( (att & CONDENSED) )
				{
					x += 2 ;
				}
			}		
      putsTime(x-FW-3+5, y, val, att, att) ;
			displayed = 1 ;
			if ( !(style & TELEM_NOTIME_UNIT) )
			{
    		unit = channel + 2 + '1';
			}
			xbase -= FW ;
    break ;

		case FR_A3 :
    case FR_A4 :
		{	
    	unit = g_model.frsky.channels[channel-FR_A3].units3_4 ;
			switch ( unit )
			{
				case 1 :
					unit = ' ' ;
				break ;
				case 3 :
					unit = 'A' ;
				break ;
				default :
					unit = 'v' ;
				break ;
			}
			att |= PREC2 ;
		}
    break ;

		case FR_A1_COPY:
    case FR_A2_COPY:
      channel -= FR_A1_COPY ;
      // no break
      // A1 and A2
			if ( valid || BLINK_ON_PHASE )
			{
				unit = putsTelemValue( (style & TELEM_VALUE_RIGHT) ? xbase+62 : x-fieldW, y, val, channel, att|NO_UNIT/*|blink*/ ) ;
			}
			displayed = 1 ;
    break ;

		case FR_RXV:
  		unit = 'v' ;
			att |= PREC1 ;
			val = convertRxv( val ) ;
    break ;

    case FR_TEMP1:
			if ( ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) ) && (style & TELEM_ARDUX_NAME) )
			{
				char *s ;
				s = arduFlightMode( FrskyHubData[FR_TEMP1] ) ;
				lcd_putsAtt( x, y, s, frskyUsrStreaming ? 0 : BLINK ) ;
				displayed = 1 ;
				break ;
			}
    case FR_TEMP2:
			unit = 'C' ;
  		if ( g_model.FrSkyImperial )
  		{
				val = c_to_f(val) ;
  		  unit = 'F' ;
				x -= fieldW ;
  		}
    break;
    
		case FR_AIRSPEED :
			att |= PREC1 ;
			unit = 'm' ;
			if ( g_model.FrSkyImperial )
			{
       	// m to ft *105/32
       	val = m_to_ft( val ) ;
	      unit = 'f' ;
			}
    break;

		case FR_ALT_BARO:
    case TELEM_GPS_ALT:
      unit = 'm' ;
			if ( g_model.FrSkyImperial )
			{
				if (g_model.FrSkyUsrProto == 0)  // Not WS How High
				{
        	// m to ft *105/32
        	val = m_to_ft( val ) ;
				}
        unit = 'f' ;
			}
			if ( val < 1000 )
			{
				att |= PREC1 ;
			}
			else
			{
				val /= 10 ;
			}
    break;

		case FR_CURRENT :
			att |= PREC1 ;
      unit = 'A' ;
		break ;

		case FR_CELL_MIN:
			if ( val > 445 )
			{
				val = 0 ;
			}
		case FR_CELL1:
		case FR_CELL2:
		case FR_CELL3:
		case FR_CELL4:
		case FR_CELL5:
		case FR_CELL6:
		case FR_CELL7:
		case FR_CELL8:
		case FR_CELL9:
		case FR_CELL10:
		case FR_CELL11:
		case FR_CELL12:
		case FR_RBOX_B1_V :
		case FR_RBOX_B2_V :
		case FR_SBEC_VOLT :
			att |= PREC2 ;
      unit = 'v' ;
		break ;
		case FR_RBOX_B1_A :
		case FR_RBOX_B2_A :
		case FR_SBEC_CURRENT :
			att |= PREC2 ;
      unit = 'A' ;
		break ;
		case FR_RBOX_B1_CAP :
		case FR_RBOX_B2_CAP :
      unit = 'm' ;
		break ;
		case FR_CELLS_TOT :
		case FR_VOLTS :
		case FR_CELLS_TOTAL1 :
		case FR_CELLS_TOTAL2 :
			att |= PREC1 ;
      unit = 'v' ;
		break ;
		case BATTERY:
			att |= PREC1 ;
      unit = 'v' ;
		break ;
		case FR_WATT :
      unit = 'w' ;
		break ;

		case FR_RPM :
  		lcd_outdezNAtt( (style & TELEM_VALUE_RIGHT) ? xbase+62 : x, y, (uint16_t)val, att, 5 ) ;
			displayed = 1 ;
		break ;

		case FR_VSPD :
			att |= PREC1 ;
			val /= 10 ;
		break ;

		case FR_FUEL :
      unit = '%' ;
		break ;

//		case FR_GPS_LAT :
//			displayGPSdata( x, y-FH, FrskyHubData[FR_GPS_LAT], FrskyHubData[FR_GPS_LATd], LEADING0 | att ) ;
//			lcd_putcAtt( x+50, y-FH, FrskyHubData[FR_LAT_N_S], att ) ;
//		break ;
//		case FR_GPS_LONG :
//			displayGPSdata( x, y, FrskyHubData[FR_GPS_LONG], FrskyHubData[FR_GPS_LONGd], LEADING0 | att ) ;
//			lcd_putcAtt( x+50, y, FrskyHubData[FR_LONG_E_W], att ) ;
//		break ;

		default:
    break;
  }
	if ( !displayed )
	{
		if ( valid || BLINK_ON_PHASE || (style & TELEM_CONSTANT) )
		{
  		lcd_outdezAtt( (style & TELEM_VALUE_RIGHT) ? xbase+62 : x, y, val, att ) ;
		}
		else
		{
			Lcd_lastPos = x+1 ;	// Put the unit in the correct place
		}
	}
	if ( style & ( TELEM_UNIT | TELEM_UNIT_LEFT ) )
	{
		if ( style & TELEM_UNIT_LEFT )
		{
			x = xbase + FW + 4 ;			
			att &= ~DBLSIZE ;			 
		}
		else
		{
			x = Lcd_lastPos ;
		}
  	lcd_putcAtt( x, y, unit, att & ~CONDENSED );
	}
	return unit ;
}

void dispGvar( uint8_t x, uint8_t y, uint8_t gvar, uint8_t attr )
{
	lcd_putsAtt( x, y, PSTR(STR_GV), attr ) ;
	lcd_putcAtt( x+2*FW, y, gvar+'0', attr ) ;
}

int16_t gvarMenuItem(uint8_t x, uint8_t y, int16_t value, int8_t min, int8_t max, uint16_t attr, uint8_t event )
{
  uint8_t invers = attr&(INVERS|BLINK);

  if ( attr & GVAR_100 )
	{
		attr &= ~GVAR_100 ;
		if (value >= 101)
		{
			dispGvar( x-3*FW, y, (uint8_t)value - 100, attr ) ;
	    if (invers) value = checkIncDec16( (uint8_t)value, 101, 107, EE_MODEL);
		}
		else
		{
  	  lcd_outdezAtt(x, y, value, attr ) ;
    	if (invers) CHECK_INCDEC_H_MODELVAR( value, min, max);
		}
		if (invers)
		{
			uint32_t toggle = 0 ;
			if ( Tevent == EVT_TOGGLE_GVAR )
			{
				toggle = 1 ;
			}
			if ( getEventDbl(EVT_KEY_FIRST(BTN_RE)) > 1 )
			{
    		killEvents(EVT_KEY_FIRST(BTN_RE)) ;
				toggle = 1 ;
			}
			if ( getEventDbl(EVT_KEY_FIRST(KEY_MENU)) > 1 )
			{
    		killEvents(EVT_KEY_FIRST(KEY_MENU)) ;
				toggle = 1 ;
			}
			if ( toggle )
			{
  	  	value = ((value >= 101) ? g_model.gvars[(uint8_t)value-101].gvar : 101);
		    eeDirty(EE_MODEL) ;
			}
		}
		return value ;
	}
  if ( attr & GVAR_250 )
	{
		attr &= ~GVAR_250 ;
		if (value >= 501)
		{
			dispGvar( x-3*FW, y, (uint8_t)value - 500, attr ) ;
  	  if (invers) value = checkIncDec16( value, 501, 505, EE_MODEL);
  	}
		else
		{
	    lcd_outdezAtt(x, y, value, attr ) ;
  	  if (invers) value = checkIncDec16( value, -350, 350, EE_MODEL ) ;
		}
		if (invers)
		{
			uint32_t toggle = 0 ;
			if ( Tevent == EVT_TOGGLE_GVAR )
			{
				toggle = 1 ;
			}
			if ( getEventDbl(EVT_KEY_FIRST(BTN_RE)) > 1 )
			{
    		killEvents(EVT_KEY_FIRST(BTN_RE)) ;
				toggle = 1 ;
			}
			if ( getEventDbl(EVT_KEY_FIRST(KEY_MENU)) > 1 )
			{
    		killEvents(EVT_KEY_FIRST(KEY_MENU)) ;
				toggle = 1 ;
			}
			if ( toggle )
			{
				s_editMode = 0 ;
  	  	value = ((value >= 501) ? g_model.gvars[value-501].gvar : 501 ) ;
		    eeDirty(EE_MODEL) ;
			}
		}
		return value ;
	}
	if (value >= 126 || value <= -126)
	{
		dispGvar( x-3*FW, y, (uint8_t)value - 125, attr ) ;
    if (invers) value = checkIncDec16( (uint8_t)value, 126, 130, EE_MODEL);
  }
  else
	{
    lcd_outdezAtt(x, y, value, attr ) ;
    if (invers) CHECK_INCDEC_H_MODELVAR( value, min, max);
  }
  
	if (invers)
	{
		uint32_t toggle = 0 ;
		if ( Tevent == EVT_TOGGLE_GVAR )
		{
			toggle = 1 ;
		}
		if ( getEventDbl(EVT_KEY_FIRST(BTN_RE)) > 1 )
		{
   		killEvents(EVT_KEY_FIRST(BTN_RE)) ;
			toggle = 1 ;
		}
		if ( getEventDbl(EVT_KEY_FIRST(KEY_MENU)) > 1 )
		{
   		killEvents(EVT_KEY_FIRST(KEY_MENU)) ;
			toggle = 1 ;
		}
		if ( toggle )
		{
    	value = ((value >= 126 || value <= -126) ? g_model.gvars[(uint8_t)value-126].gvar : 126);
	    eeDirty(EE_MODEL) ;
		}
	}
	return value;
}

bool getSwitchDr( int8_t swtch )
{
	uint8_t aswitch = abs(swtch) ;
	if ( ( aswitch <= HSW_FM6 ) && ( aswitch >= HSW_FM0 ) )
	{
		aswitch -= HSW_FM0 ;
		aswitch = getFlightPhase() == aswitch ;
		return (swtch < 0) ? !aswitch : aswitch ;
	}
	else
	{
		return getSwitch00( swtch ) ;
	}
}

uint8_t get_dr_state(uint8_t x)
{
	ExpoData *ped ;

	ped = &g_model.expoData[x] ;
	
 	return (!getSwitchDr( ped->drSw1) ? DR_HIGH :
    !getSwitchDr( ped->drSw2) ? DR_MID : DR_LOW) ;
}

static uint8_t mapPots( uint8_t value )
{
	if ( value >= EXTRA_POTS_POSITION )
	{
		if ( value >= EXTRA_POTS_START )
		{
			if ( value < EXTRA_POTS_START + 8 )
			{
    	#ifdef ARUNI
    	  uint8_t b = 1 << (value - EXTRA_POTS_START);  // b={P4:1,P5:2,P6:4,P7:8}
    	  value = (EXTRA_POTS_POSITION - 1) ;
    	  while (b) {
    	    if (ExtraPotBits & b) {
    	      value++;
    	    }
    	    b >>= 1;
    	  }
    	#else
				value -= ( EXTRA_POTS_START - EXTRA_POTS_POSITION ) ;
    	#endif
			}
			else
			{
				value -= 8 ;
				value += NumExtraPots ;
			}
		}
		else
		{
			value += NumExtraPots ;
		}
	}
#if defined(PCBX7) || defined (PCBXLITE)
	if ( value > 7 )
	{
		value -= 1 ;
	}
#endif // PCBX7
#ifdef PCBX9LITE
	if ( value > 6 )
	{
		value -= 2 ;
	}
#endif // PCBX9LITE
#ifdef PCBLEM1
	if ( value > 6 )
	{
		value -= 2 ;
	}
#endif // PCBLEM1
	return value ;
}

uint8_t unmapPots( uint8_t value )
{
#if defined(PCBX7) || defined (PCBXLITE)
	if ( value > 6 )
	{
		value += 1 ;
	}
#endif // PCBX7
#ifdef PCBX9LITE
	if ( value > 5 )
	{
		value += 2 ;
	}
#endif // PCBX9LITE
#ifdef PCBLEM1
	if ( value > 5 )
	{
		value += 2 ;
	}
#endif // PCBLEM1
	
	if ( value >= EXTRA_POTS_POSITION )
	{
		if ( value < EXTRA_POTS_POSITION + NumExtraPots )
		{
    #ifdef ARUNI
			uint8_t n = (value - EXTRA_POTS_POSITION);  // 0 <= n < NumExtraPots
			value = EXTRA_POTS_START;
      for (uint8_t i = 0; i < NUM_POSSIBLE_EXTRA_POTS; i++) {
        if (ExtraPotBits & (1 << i)) {
          if (n == 0)
            break;
          n--;
        }
        value++;
      }
    #else
			value += EXTRA_POTS_START - EXTRA_POTS_POSITION ;
    #endif
		}
		else
		{
			value -= NumExtraPots ;
			if ( value >= EXTRA_POTS_START )
			{
				value += 8 ;
			}
		}
	}
	return value ;
}

// rounded square
#define DO_RSQUARE(xx,yy,ww)         \
{uint8_t x,y,w ; x = xx; y = yy; w = ww ; \
    lcd_vline(x-w/2,y-w/2+1,w-2);  \
    lcd_hline(x-w/2+1,y+w/2,w-2);  \
    lcd_vline(x+w/2,y-w/2+1,w-2);  \
    lcd_hline(x-w/2+1,y-w/2,w-2);}

// sharp square
#define DO_SQUARE(xx,yy,ww)         \
{uint8_t x,y,w ; x = xx; y = yy; w = ww ; \
    lcd_vline(x-w/2,y-w/2,w);  \
    lcd_hline(x-w/2+1,y+w/2,w-2);  \
    lcd_vline(x+w/2,y-w/2,w);  \
    lcd_hline(x-w/2+1,y-w/2,w-2);}

#define DO_CROSS(xx,yy,ww)          \
    lcd_vline(xx,yy-ww/2,ww);  \
    lcd_hline(xx-ww/2,yy,ww);  \

#if defined(PCBX12D) || defined(PCBX10)
#define V_BAR(xx,yy,ll)       \
    lcd_vline(xx-1,yy-ll,ll); \
    lcd_vline(xx  ,yy-ll,ll); \
    lcd_vline(xx+1,yy-ll,ll);
#else
#define V_BAR(xx,yy,ll)       \
    lcd_vline(xx-1,yy-ll,ll); \
    lcd_vline(xx  ,yy-ll,ll); \
    lcd_vline(xx+1,yy-ll,ll);
#endif

#define NO_HI_LEN 25

#if defined(PCBX12D) || defined(PCBX10)
#define WCHART 44
#define X0     (170-WCHART-2 - 2 )
#define Y0     48
#define WCHARTl 44l
#define X0l     (170l-WCHARTl-2)
#define Y0l     48l
#else
#define WCHART 29
#define X0     (128-WCHART-2 - 2 )
#define Y0     32
#define WCHARTl 29l
#define X0l     (128l-WCHARTl-2)
#define Y0l     32l
#endif
#define RESX    (1<<10) // 1024
#define RESXu   1024u
#define RESXul  1024ul
#define RESXl   1024l
#define RESKul  100ul
#define RESX_PLUS_TRIM (RESX+128)


//void menuProcSetup(uint8_t event) ;
//void menuProcSetup1(uint8_t event) ;
//void menuProcSetup2(uint8_t event) ;
void menuProcDiagAna(uint8_t event) ;
void menuProcDiagKeys(uint8_t event) ;
void menuProcDiagCalib(uint8_t event) ;
void menuProcSDstat(uint8_t event) ;
void menuProcBoot(uint8_t event) ;
#ifdef PCB9XT
void menuProcSlave(uint8_t event) ;
#endif
void menuProcTrainer(uint8_t event) ;
void menuProcDiagVers(uint8_t event) ;
void menuProcProtocol(uint8_t event) ;
//void menuXProcProtocol(uint8_t event) ;
void menuProcTrainProtocol(uint8_t event) ;
void menuModelPhases(uint8_t event) ;
void menuProcModelSelect(uint8_t event) ;
void menuProcMix(uint8_t event) ;
void menuProcExpoAll(uint8_t event);
void menuProcLimits(uint8_t event);
void menuProcCurve(uint8_t event);
void menuProcSwitches(uint8_t event);
void menuProcSafetySwitches(uint8_t event);
void menuProcTemplates(uint8_t event);
void menuProcTelemetry(uint8_t event) ;
void menuProcDate(uint8_t event) ;
void menuProcRestore(uint8_t event) ;
void menuProcIndex(uint8_t event) ;
void menuProcModelIndex(uint8_t event) ;
void menuCellScaling(uint8_t event) ;
void menuEditNotes(uint8_t event) ;

#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX7) || defined(PCBX9LITE) || defined(PCBX9D) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
#ifdef BLUETOOTH
void menuProcBt(uint8_t event) ;
void menuProcBtScan(uint8_t event) ;
void menuProcBtConfigure(uint8_t event) ;
void menuProcBtAddressList(uint8_t event) ;
#endif
#endif

void menuProcBattery(uint8_t event) ;
void menuProcStatistic(uint8_t event) ;
#ifdef NEW_VARIO
void menuNewVario(uint8_t event) ;
#endif
void menuProcMusic(uint8_t event) ;
void menuProcMusicList(uint8_t event) ;
void menuProcStatistic2(uint8_t event) ;
#ifndef SMALL
void menuProcDsmDdiag(uint8_t event) ;
#endif
void menuProcAlpha(uint8_t event) ;
#ifndef SMALL
void menuProcTrainDdiag(uint8_t event) ;
void menuProcS6R(uint8_t event) ;
#endif
void menuDebug(uint8_t event) ;
#if defined(LUA) || defined(BASIC)
void menuScript(uint8_t event) ;
#endif
#ifdef IMAGE_128
void menuImage(uint8_t event) ;
#endif
void menuTextHelp(uint8_t event) ;
void menuVario(uint8_t event) ;
void menuSensors(uint8_t event) ;

void menuProcVoiceAlarm(uint8_t event) ;
void menuProcGlobalVoiceAlarm(uint8_t event) ;

#ifndef SMALL
void menuScanDisplay(uint8_t event) ;
#endif

#ifdef PCBLEM1
#define MENUTESTINCLUDE
#endif

#ifdef MENUTESTINCLUDE
void menuTest(uint8_t event) ;
#endif

uint8_t getMixerCount( void ) ;
bool reachMixerCountLimit( void ) ;


enum MainViews
{
  e_outputValues,
  e_outputBars,
  e_inputs1,
  e_timer2,
  e_telemetry,
#if defined(PCBX12D) || defined(PCBX10)
  e_picture,
#endif
  MAX_VIEWS
};

int16_t calibratedStick[NUMBER_ANALOG+NUM_POSSIBLE_EXTRA_POTS];

int16_t ex_chans[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS];          // Outputs + intermidiates
uint8_t s_pgOfs;
uint8_t s_editMode;
uint8_t s_editing;
uint8_t s_noHi;
uint8_t scroll_disabled;
int8_t scrollLR;
int8_t scrollUD;
uint8_t InverseBlink ;
uint8_t EditType ;
uint8_t EditColumns ;

int16_t g_chans512[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS];

#include "sticks.lbm"
#ifndef SMALL
#include "s6rimg.lbm"
#endif

enum EnumTabStat
{
	e_battery,
	e_stat1,
	e_stat2,
#ifdef NEW_VARIO
	e_vario,
#endif
	e_music,
	e_music1,
#ifndef SMALL
	e_dsm,
	e_traindiag,
#endif
//#if defined(LUA) || defined(BASIC)
//	e_Script,
//#endif
	e_debug,
  e_Setup3,
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX7) || defined(PCBX9LITE) || defined(PCBX9D) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
//#ifdef IMAGE_128
//	e_image,
//#endif
#ifdef BLUETOOTH
	e_bluetooth,
#endif
#endif
#ifdef MENUTESTINCLUDE
	e_test,
#endif
#ifdef PCB9XT
	e_Slave,
#endif
#ifndef SMALL
	e_s6r,
#endif
  e_Boot
} ;

MenuFuncP menuTabStat[] =
{
	menuProcBattery,
	menuProcStatistic,
	menuProcStatistic2,
#ifdef NEW_VARIO
	menuNewVario,
#endif
	menuProcMusic,
	menuProcMusicList,
#ifndef SMALL
	menuProcDsmDdiag,
	menuProcTrainDdiag,
#endif
//#if defined(LUA) || defined(BASIC)
//	menuScript,
//#endif
	menuDebug,
	menuProcSDstat,
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX7) || defined(PCBX9LITE) || defined(PCBX9D) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
//#ifdef IMAGE_128
//	menuImage,
//#endif
#ifdef BLUETOOTH
	menuProcBt,
#endif
#endif
#ifdef MENUTESTINCLUDE
	menuTest,
#endif
#ifdef PCB9XT
	menuProcSlave,
#endif
#ifndef SMALL
	menuProcS6R,
#endif
	menuProcBoot
} ;


// Rotary Encoder states
#define RE_IDLE				0
#define RE_MENU_LR		1
#define RE_MENU_UD		2
#define RE_ITEMLR			3
#define RE_ITEM_EDIT	4

uint8_t Re_state ;

void displayNext()
{
	lcd_putsAtt( 17*FW+2, 7*FH, XPSTR("[->]"), BLINK ) ;
}

uint8_t locateMappedItem( uint8_t value, uint8_t *options, uint32_t count )
{
	uint32_t c ;
	for ( c = 0 ; c < count ; c += 1 )
	{
		if ( options[c] == value )
		{
			value = c ;
			break ;
		}
	}
	if ( c >= count )
	{
		return 0 ;
	}
	return value ;
	
}

// Check a mapped value
uint8_t checkOutOfOrder( uint8_t value, uint8_t *options, uint32_t count )
{
	uint8_t index = locateMappedItem( value, options, count ) ;
	CHECK_INCDEC_H_MODELVAR_0( index, count-1 ) ;
	index = options[index] ;
	return index ;
}

const char *get_curve_string()
{
    return PSTR(CURV_STR)	;
}	

void menu_lcd_onoff( uint8_t x,uint8_t y, uint8_t value, uint8_t mode )
{
  if (value)
	{
    lcd_putc(x+1, y, '\202');
	}
	lcd_hbar( x, y, 7, 7, mode ? 100 : 0 ) ;
}

#define EDIT_DR_SWITCH_EDIT		0x01
#define EDIT_DR_SWITCH_MOMENT	0x02
#define EDIT_DR_SWITCH_FMODE	0x04

int8_t edit_dr_switch( uint8_t x, uint8_t y, int8_t drswitch, uint8_t attr, uint8_t flags, uint8_t event )
{
	if ( flags & EDIT_DR_SWITCH_MOMENT )
	{
		putsMomentDrSwitches( x, y, drswitch, attr ) ;
	}
	else
	{
		putsDrSwitches( x,  y, drswitch, attr ) ;
	}
	if(flags & EDIT_DR_SWITCH_EDIT)
	{
		if ( flags & EDIT_DR_SWITCH_FMODE )
		{
//			CHECK_INCDEC_MODELSWITCH( drswitch, -MaxSwitchIndex, MaxSwitchIndex) ;
			uint8_t sign = 0 ;
			int8_t mode = drswitch ;
			if ( drswitch < 0 )
			{
				sign = 1 ;
				mode = -drswitch ;
			}
			if ( ( mode <= HSW_FM6 ) && ( mode >= HSW_FM0 ) )
			{
				drswitch = mode - HSW_FM0 + MaxSwitchIndex ;
				if ( sign )
				{
					drswitch = -drswitch ;
				}
			}
			else
			{
				drswitch = switchUnMap( drswitch ) ;
			}
  		drswitch = checkIncDec16( drswitch, -MaxSwitchIndex - MAX_MODES, MaxSwitchIndex + MAX_MODES, EE_MODEL|INCDEC_SWITCH ) ;
			if ( abs(drswitch) >= MaxSwitchIndex )
			{
				sign = 0 ;
				if ( drswitch < 0 )
				{
					sign = 1 ;
					drswitch = -drswitch ;
				}
				drswitch += HSW_FM0 - MaxSwitchIndex ;
				if ( sign )
				{
					drswitch = -drswitch ;
				}
			}
			else
			{
				drswitch = switchMap( drswitch ) ;
			}
		}
		else if ( flags & EDIT_DR_SWITCH_MOMENT )
		{
#if defined(PCBSKY) || defined(PCB9XT)
			int16_t value = drswitch ;
			if ( value < -HSW_MAX )
			{
				value += 256 ;
			}
			if ( value > HSW_MAX )
			{
				value = switchUnMap( value - HSW_MAX ) + MaxSwitchIndex - 1 ;
			}
			else
			{
				value = switchUnMap( drswitch ) ;
			}
      CHECK_INCDEC_H_MODELVAR( value ,(1-MaxSwitchIndex),(-2+2*MaxSwitchIndex));
			if ( value >= MaxSwitchIndex )
			{
				drswitch = switchMap( value - MaxSwitchIndex + 1 ) + HSW_MAX ;
			}
			else
			{
				drswitch = switchMap( value ) ;
			}
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
			int16_t value = drswitch ;
			if ( value < -HSW_MAX )
			{
				value += 256 ;
			}
			if ( value > HSW_MAX )
			{
				value = switchUnMap( value - HSW_MAX ) + MaxSwitchIndex - 1 ;
			}
			else
			{
				value = switchUnMap( drswitch ) ;
			}
      CHECK_INCDEC_H_MODELVAR( value ,(1-MaxSwitchIndex),(-2+2*MaxSwitchIndex));
			if ( value >= MaxSwitchIndex )
			{
				drswitch = switchMap( value - MaxSwitchIndex + 1 ) + HSW_MAX ;
			}
			else
			{
				drswitch = switchMap( value ) ;
			}
#endif
		}
		else
		{
			CHECK_INCDEC_MODELSWITCH( drswitch, -MaxSwitchIndex, MaxSwitchIndex) ;
		}
	}
	return drswitch ;
}

void validateText( uint8_t *text, uint32_t length )
{
	for( uint8_t i=0 ; i<length ; i += 1 ) // makes sure text is valid
  {
		if ( text[i] < ' ' )
		{
			text[i] = ' ' ;
		}
//    uint8_t idx = char2idx(text[i]);
//    text[i] = idx2char(idx);
  }
}

void validateName( uint8_t *text, uint32_t length )
{
	for(uint8_t i=0; i<length ; i += 1) // makes sure name is valid
	{
		uint8_t idx = char2idx( text[i] ) ;
		text[i] = idx2char( idx ) ;
	}
}

void alphaEditName( uint8_t x, uint8_t y, uint8_t *name, uint8_t len, uint16_t type, uint8_t *heading )
{
	if ( ( type & ALPHA_NO_NAME ) == 0 )
	{
		lcd_puts_Pleft( y, PSTR( STR_NAME ) ) ;
	}
	Alpha.AlphaHex = ( type & ALPHA_HEX ) ? 1 : 0 ;
	type &= ~ALPHA_HEX ;
	validateText( name, len ) ;
	lcd_putsnAtt( x, y, (const char *)name, len, 0 ) ;
	if ( type & ~ALPHA_NO_NAME )
	{
		lcd_rect( x-1, y-1, len*FW+2, 9 ) ;
		if ( checkForMenuEncoderBreak( Tevent ) )
		{
			Alpha.AlphaLength = len ;
			Alpha.PalphaText = name ;
			Alpha.PalphaHeading = heading ;
			s_editMode = 0 ;
    	killEvents(Tevent) ;
			Tevent = 0 ;
			pushMenu(menuProcAlpha) ;
		}
	}
}

void lcd_xlabel_decimal( uint8_t x, uint8_t y, uint16_t value, uint8_t attr, const char *s )
{
  lcd_outdezAtt( x, y, value, attr ) ;
	lcd_puts_Pleft( y, s ) ;
}

uint8_t checkIndexed( uint8_t y, const char *s, uint8_t value, uint8_t edit )
{
	uint8_t x ;
	uint8_t max ;
	if ( s )
	{
		x = *s++ ;
		max = *s++ ;
		lcd_putsAttIdx( x, y, s, value, edit ? InverseBlink: 0 ) ;
	}
	else
	{
		x = PARAM_OFS ;
		max = 1 ;
		menu_lcd_onoff( x, y, value, edit ) ;
	}
	if ( value > max )
	{
		value = max ;
	}
	if(edit)
	{
		if ( ( EditColumns == 0 ) || ( s_editMode ) )
		{
			value = checkIncDec( value, 0, max, EditType ) ;
		}
	}
	return value ;
}

uint8_t hyphinvMenuItem( uint8_t value, uint8_t y, uint8_t condition )
{
	return checkIndexed( y, PSTR( STR_HYPH_INV), value, condition ) ;
}

uint16_t xnormalize( int16_t alpha )
{
   if ( alpha > 360 ) alpha -= 360 ;
   if ( alpha <   0 ) alpha += 360 ;
   return alpha ;
}

int8_t rxsin100( int16_t a )
{
	int32_t b ;
	int32_t neg = 0 ;
  a = xnormalize( a ) ;
	if ( a > 180 )
	{
		neg = 1 ;
		a -= 180 ;
	}
	if ( a > 90 )
	{
		a = 180 - a ;
	}
// Angle in degrees (0-90) = a
// 3823-a*a/20 = b ;
// 3823-(b*a*a/19697) = c
// sin = c*a/1880
	b = 3283 - a * a / 20 ;
	b = 3283 - b * a * a / 19697 ;
	b = b * a / 1880 ;
	if ( neg )
	{
		b = -b ;
	}
	return b ;
}

int8_t rxcos100( int16_t a )
{
	a += 90 ;
	return rxsin100( xnormalize( a ) ) ;
}

#ifdef BIG_SCREEN
#define RIGHT_POSITION	239
#else
#define RIGHT_POSITION	127
#endif

void DisplayScreenIndex(uint8_t index, uint8_t count, uint8_t attr)
{
	uint8_t x ;
	if ( RotaryState == ROTARY_MENU_LR )
	{
		attr = BLINK ;
	}
	lcd_outdezAtt(RIGHT_POSITION,0,count,attr);
	x = 1+(RIGHT_POSITION+1)-FW*(count>9 ? 3 : 2) ;
  lcd_putcAtt(x,0,'/',attr);
  lcd_outdezAtt(x-1,0,index+1,attr);
}


uint8_t g_posHorz ;
uint8_t M_longMenuTimer ;
uint8_t M_lastVerticalPosition ;

#if defined(PCBLEM1)
uint8_t M_longBtnTimer ;
#endif

//#define MAXCOL(row) (horTab ? pgm_read_byte(horTab+min(row, horTabMax)) : (const uint8_t)0)


uint8_t MState2::check_columns( uint8_t event, uint8_t maxrow)
{
	return check( event, 0, NULL, 0, &Columns, 0, maxrow ) ;
}

uint8_t MAXCOL( uint8_t row, const uint8_t *horTab, uint8_t horTabMax)
{
	return (horTab ? pgm_read_byte(horTab+min(row, horTabMax)) : (const uint8_t)0) ;
}

#define INC(val,max) if(val<max) {val++;} else {val=0;}
#define DEC(val,max) if(val>0  ) {val--;} else {val=max;}

uint8_t MState2::check(uint8_t event, uint8_t curr, MenuFuncP *menuTab, uint8_t menuTabSize, prog_uint8_t *horTab, uint8_t horTabMax, uint8_t maxrow)
{
	uint8_t l_posHorz ;
	l_posHorz = g_posHorz ;
	
	int16_t c4, c5 ;
	struct t_p1 *ptrp1 ;

    //check pot 2 - if changed -> scroll menu
    //check pot 3 if changed -> cursor down/up
    //we do this in these brackets to prevent it from happening in the main screen
	c4 = calibratedStick[4] ;		// Read only once
	c5 = calibratedStick[5] ;		// Read only once
    
	ptrp1 = &P1values ;
		
  scrollLR = ( ptrp1->p2valprev-c4)/SCROLL_TH;
  scrollUD = ( ptrp1->p3valprev-c5)/SCROLL_TH;

  if(scrollLR) ptrp1->p2valprev = c4;
  if(scrollUD) ptrp1->p3valprev = c5;

  if(scroll_disabled || g_eeGeneral.disablePotScroll)
  {
    scrollLR = 0;
    scrollUD = 0;
    scroll_disabled = 0;
  }

	M_lastVerticalPosition = m_posVert ;
  
	if(scrollLR || scrollUD || ptrp1->p1valdiff)
	{
		g_LightOffCounter = g_eeGeneral.lightAutoOff*500; // on keypress turn the light on 5*100
		InacCounter = 0 ;
	}

	if (menuTab)
	{
    uint8_t attr = m_posVert==0 ? INVERS : 0;


    if(m_posVert==0)
    {
			if ( RotaryState == ROTARY_MENU_LR )
			{
				if ( Rotary_diff > 0 )
				{
   				scrollLR = -1;
				}
				else if ( Rotary_diff < 0 )
				{
   				scrollLR = 1;
				}
				Rotary_diff = 0 ;
        if(event==EVT_KEY_BREAK(BTN_RE))
				{
					RotaryState = ROTARY_MENU_UD ;
		      event = 0 ;
		      Tevent = 0 ;
				}
			}
			else if ( RotaryState == ROTARY_MENU_UD )
			{
        if(event==EVT_KEY_BREAK(BTN_RE))
				{
					RotaryState = ROTARY_MENU_LR ;
		      event = 0 ;
		      Tevent = 0 ;
				}
			}
      if( scrollLR && !s_editMode)
      {
        int8_t cc = curr - scrollLR ;
        if(cc<1) cc = menuTabSize-1 ;
        if(cc>(menuTabSize-1)) cc = 0 ;

        if(cc!=curr)
        {
          chainMenu((MenuFuncP)pgm_read_adr(&menuTab[cc]));
					return event ;
        }

        scrollLR = 0;
      }

      if(event==EVT_KEY_FIRST(KEY_LEFT))
      {
        uint8_t index ;
        if(curr>0)
          index = curr ;
        else
          index = menuTabSize ;

       	chainMenu((MenuFuncP)pgm_read_adr(&menuTab[index-1]));
				return event ;
      }

      if(event==EVT_KEY_FIRST(KEY_RIGHT))
      {
        uint8_t index ;
        if(curr < (menuTabSize-1))
          index = curr +1 ;
        else
          index = 0 ;
        chainMenu((MenuFuncP)pgm_read_adr(&menuTab[index]));
				return event ;
      }
    }
		else
		{
			if ( s_editMode == 0 ) RotaryState = ROTARY_MENU_UD ;
		}

    DisplayScreenIndex(curr, menuTabSize, attr);
  }
	else if ( RotaryState == ROTARY_MENU_LR )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
  uint8_t maxcol = MAXCOL(m_posVert, horTab, horTabMax);
		
 if ( maxrow != 0xFF )
 {
	if ( RotaryState == ROTARY_MENU_UD )
	{
		static uint8_t lateUp = 0 ;
		if ( lateUp )
		{
			lateUp = 0 ;
			l_posHorz = MAXCOL(m_posVert, horTab, horTabMax) ;
		}
		if ( Rotary_diff > 0 )
		{
    	INC(l_posHorz,maxcol) ;
			if ( l_posHorz == 0 )
			{
				INC(m_posVert,maxrow);
			}
		}
		else if ( Rotary_diff < 0 )
		{
			if ( l_posHorz == 0 )
			{
      	DEC(m_posVert,maxrow);
				lateUp = 1 ;
				l_posHorz = 0 ;
			}
			else
			{
      	DEC(l_posHorz,maxcol) ;
			}
		}
		Rotary_diff = 0 ;
    if(event==EVT_KEY_BREAK(BTN_RE))
		{
			RotaryState = ROTARY_VALUE ;
		}
	}
	else if ( RotaryState == ROTARY_VALUE )
	{
		if ( s_editMode )
		{
			MaskRotaryLong = 1 ;
		}
		if ( (event==EVT_KEY_BREAK(BTN_RE)) || ( s_editMode == 0 ) )
		{
			RotaryState = ROTARY_MENU_UD ;
		}
	}

	{
		uint8_t timer = M_longMenuTimer ;
		if ( menuPressed() )
		{
			if ( timer < 255 )
			{
				timer += 1 ;
			}
		}
		else
		{
			timer = 0 ;
		}
		if ( timer > 60 )
		{
			s_editMode = 1 ;
			RotaryState = ROTARY_VALUE ;
		}
		M_longMenuTimer = timer ;
	}
#ifdef PCBLEM1
	{
		uint8_t timer = M_longBtnTimer ;
		if ( encoderPressed() )
		{
			if ( timer < 255 )
			{
				timer += 1 ;
			}
		}
		else
		{
			timer = 0 ;
		}
		if ( timer > 60 )
		{
    	if ( ( event == 0 ) || ( event == EVT_KEY_REPT(BTN_RE) ) )
			{
				event = EVT_KEY_LONG(KEY_EXIT) ;
			}			
		}
		M_longBtnTimer = timer ;
	}
#endif

 } 

  maxcol = MAXCOL(m_posVert, horTab, horTabMax) ;
	EditColumns = maxcol ;
  l_posHorz = min(l_posHorz, maxcol ) ;

  if(!s_editMode)
  {
    if(scrollUD)
    {
      int8_t cc = m_posVert - scrollUD;
      if(cc<1) cc = 0;
      if(cc>=maxrow) cc = maxrow;
      m_posVert = cc;

      l_posHorz = min(l_posHorz, MAXCOL(m_posVert, horTab, horTabMax));
      BLINK_SYNC;

      scrollUD = 0;
    }

    if(m_posVert>0 && scrollLR)
    {
      int8_t cc = l_posHorz - scrollLR;
      if(cc<1) cc = 0;
      if(cc>=MAXCOL(m_posVert, horTab, horTabMax)) cc = MAXCOL(m_posVert, horTab, horTabMax);
      l_posHorz = cc;

      BLINK_SYNC;
      //            scrollLR = 0;
    }
  }

  switch(event)
  {
    case EVT_ENTRY:
        init() ;
        l_posHorz = 0 ;
				M_lastVerticalPosition = m_posVert ;
    case EVT_ENTRY_UP :
        s_editMode = false;
    break;
    case EVT_KEY_BREAK(BTN_RE):
    case EVT_KEY_FIRST(KEY_MENU):
        if ( (m_posVert > 0) || (!menuTab) )
				{
	 				if ( maxrow != 0xFF )
					{
						s_editMode = !s_editMode;
						if ( s_editMode )
						{
							RotaryState = ROTARY_VALUE ;
						}
					}
				}
    break;
    case EVT_KEY_LONG(KEY_EXIT):
        s_editMode = false;
        popMenu(true); //return to uppermost, beeps itself
//				event = EVT_EXIT ;
    break;
        //fallthrough
    case EVT_KEY_LONG(BTN_RE):
				if ( g_eeGeneral.disableBtnLong )
				{	
					break ;
				}
				if ( MaskRotaryLong )
				{
					break ;
				}
				killEvents(event) ;
    case EVT_KEY_BREAK(KEY_EXIT):
        if(s_editMode) {
            s_editMode = false;
            break;
        }
//#ifdef PCBLEM1
        popMenu();  //beeps itself
//#endif  
//				event = EVT_EXIT ;
    break;

    case EVT_KEY_REPT(KEY_RIGHT):  //inc
        if(l_posHorz==maxcol) break;
    case EVT_KEY_FIRST(KEY_RIGHT)://inc
        if(!horTab || s_editMode)break;
        INC(l_posHorz,maxcol);
				if ( maxcol )
				{
					event = 0 ;
					Tevent = 0 ;
				}
        BLINK_SYNC;
    break;

    case EVT_KEY_REPT(KEY_LEFT):  //dec
        if(l_posHorz==0) break;
    case EVT_KEY_FIRST(KEY_LEFT)://dec
        if(!horTab || s_editMode)break;
        DEC(l_posHorz,maxcol);
				if ( maxcol )
				{
					event = 0 ;
					Tevent = 0 ;
				}
        BLINK_SYNC;
    break;

    case EVT_KEY_REPT(KEY_DOWN):  //inc
        if(m_posVert==maxrow) break;
    case EVT_KEY_FIRST(KEY_DOWN): //inc
        if(s_editMode)break;
        INC(m_posVert,maxrow);
        l_posHorz = min(l_posHorz, MAXCOL(m_posVert, horTab, horTabMax));
        BLINK_SYNC;
    break;

    case EVT_KEY_REPT(KEY_UP):  //dec
        if(m_posVert==0) break;
    case EVT_KEY_FIRST(KEY_UP): //dec
        if(s_editMode)break;
        DEC(m_posVert,maxrow);
        l_posHorz = min(l_posHorz, MAXCOL(m_posVert, horTab, horTabMax));
        BLINK_SYNC;
    break;
  }
	s_editing = s_editMode || P1values.p1valdiff ;
//	InverseBlink = (!maxcol || s_editMode) ? BLINK : INVERS ;
	g_posHorz = l_posHorz ;
	InverseBlink = (s_editMode) ? BLINK : INVERS ;
	Columns = 0 ;
	MaskRotaryLong = 0 ;
	return event ;
}

#if defined(PCBX12D) || defined(PCBX10)

#define BOX_WIDTH     35
#define BAR_HEIGHT    (BOX_WIDTH-1l)
#define MARKER_WIDTH  5
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define BOX_LIMIT     (BOX_WIDTH-MARKER_WIDTH)
#define LBOX_CENTERX  (  SCREEN_WIDTH/4 + 10 - 2-6)
#define RBOX_CENTERX  (3*SCREEN_WIDTH/4 - 10 + 1 + 24+6)
#define BOX_CENTERY  (SCREEN_HEIGHT-9-BOX_WIDTH/2-1+12)

#else

#define BOX_WIDTH     23
#define BAR_HEIGHT    (BOX_WIDTH-1l)
#define MARKER_WIDTH  5
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define BOX_LIMIT     (BOX_WIDTH-MARKER_WIDTH)
#if defined(PCBX12D) || defined(PCBX10)
#define LBOX_CENTERX  (  SCREEN_WIDTH/4 + 10 - 2)
#define RBOX_CENTERX  (3*SCREEN_WIDTH/4 - 10 + 1 + 24)
#else
#ifdef ARUNI
#define SLIDER_WIDTH  2
#define SLIDER_HEIGHT 3
#define LBOX_CENTERX  (  SCREEN_WIDTH/4 + 10)
#else
#define LBOX_CENTERX  (  SCREEN_WIDTH/4 + 10-1)
#endif
#define RBOX_CENTERX  (3*SCREEN_WIDTH/4 - 10)
#endif
#define BOX_CENTERY  (SCREEN_HEIGHT-9-BOX_WIDTH/2-1)
//#define RBOX_CENTERY  (SCREEN_HEIGHT-9-BOX_WIDTH/2)

#endif

#ifdef ARUNI
void lcd_rect_filled(uint8_t x, uint8_t y, uint8_t w, uint8_t h )
{
  for (uint8_t i = 0; i < w; i++)
    lcd_vline(x+i, y, h ) ;
}
#endif

void telltale( uint8_t centrex, int8_t xval, int8_t yval )
{
  DO_SQUARE( centrex, BOX_CENTERY, BOX_WIDTH ) ;
  DO_CROSS( centrex, BOX_CENTERY,3 ) ;
	DO_RSQUARE( centrex +( xval*BOX_LIMIT/(2*RESX/16)), BOX_CENTERY-( yval*BOX_LIMIT/(2*RESX/16)), MARKER_WIDTH ) ;
}


void doMainScreenGrphics()
{
	int8_t *cs = phyStick ;
	
#if defined(PCBX12D) || defined(PCBX10)
	pushPlotType( PLOT_BLACK ) ;
#endif

#ifdef ARUNI
  uint8_t shift = 0;
  if (NumExtraPots)
    shift = 1;

	telltale( LBOX_CENTERX + shift, cs[0], cs[1] ) ;
	telltale( RBOX_CENTERX - shift, cs[3], cs[2] ) ;

  if (ExtraPotBits & 1)
	{
	  lcd_rect_filled( LBOX_CENTERX+shift-BOX_WIDTH/2-SLIDER_WIDTH,
        BOX_CENTERY-(calibratedStick[7]/((2*RESX)/(BOX_WIDTH-2)))-SLIDER_HEIGHT/2,
        SLIDER_WIDTH, SLIDER_HEIGHT ) ;
  }
  if (ExtraPotBits & 2)
	{
    lcd_rect_filled( RBOX_CENTERX-shift+BOX_WIDTH/2+1,
        BOX_CENTERY-(calibratedStick[8]/((2*RESX)/(BOX_WIDTH-2)))-SLIDER_HEIGHT/2,
        SLIDER_WIDTH, SLIDER_HEIGHT ) ;
  }

  // Optimization by Mike Blandford
  {
    uint8_t x, y, len ;			// declare temporary variables
    for( x = -5, y = 4 ; y < 7 ; x += 5, y += 1 )
    {
      len = ((calibratedStick[y]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
      V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-9, len ) ; // HTC: shifted up by 1
    }
  }
#else
	telltale( LBOX_CENTERX + X12SMALL_OFFSET, cs[0], cs[1] ) ;
	telltale( RBOX_CENTERX + X12SMALL_OFFSET, cs[3], cs[2] ) ;
    
    // Optimization by Mike Blandford
    {
			  int8_t x ;
				uint8_t y, len ;			// declare temporary variables
#if defined(PCBSKY) || defined(PCB9XT)
				uint32_t maxy ;
				maxy = 7 ;
				x = -5 ;
				if ( g_eeGeneral.extraPotsSource[0] )
				{
					x = - 8 ;
					maxy = 8 ;
				}

        for( y = 4 ; y < maxy ; x += 5, y += 1 )
        {
            len = ((calibratedStick[y]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
            V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len )
        }
#endif
#if defined(PCBX9D)
#ifdef PCBX7
        for( x = -8, y = 4 ; y < 8 ; x += 5, y += 1 )
        {
					if ( y == 4 )
					{
						if (g_eeGeneral.extraPotsSource[0] == 0 )
						{
							continue ;
						}
					}
					if ( y == 7 )
					{
						if (g_eeGeneral.extraPotsSource[1] == 0 )
						{
							continue ;
						}
					}
        	uint8_t z = y - 1 ;
					if ( z == 6 )
					{
						z = 7 ;
					}
					else if ( z == 3 )
					{
						z = 6 ;
					}
          len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
          V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;
        }
        
//				for( x = -3, y = 5 ; y < 7 ; x += 5, y += 1 )
//        {
//        	uint8_t z = y - 1 ;
////					if ( z == 6 )
////					{
////						z = 7 ;
////					}
////					else if ( z == 3 )
////					{
////						z = 6 ;
////					}
//          len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
//          V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;
//        }
#else // PCBX7
#ifdef PCBXLITE
        for( x = -4, y = 4 ; y < 6 ; x += 5, y += 1 )
        {
        	uint8_t z = y ;
//					if ( z == 6 )
//					{
//						z = 7 ;
//					}
//					else 
//					if ( z == 3 )
//					{
//						z = 6 ;
//					}
          len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
          V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;
        }
#else // PCBXLITE
#ifdef PCBX9LITE
				x = 1 ;
				y = 4 ;
        uint8_t z = y ;
        len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
        V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;

				if (g_eeGeneral.extraPotsSource[0] )
				{
					x = -4 ;
					y = 5 ;
    	    uint8_t z = y ;
      	  len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
        	V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;
				}

				if (g_eeGeneral.extraPotsSource[1] )
				{
					x = 6 ;
					y = 6 ;
    	    uint8_t z = y ;
      	  len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
        	V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;
				}


#else // PCBX9LITE
#if defined(REVPLUS) || defined(REVNORM) || defined(REV9E) || defined(REV19)
        for( x = -8, y = 4 ; y < 8 ; x += 5, y += 1 )
        {
        	uint8_t z = y - 1 ;
					if ( z == 6 )
					{
						z = 7 ;
					}
					else if ( z == 3 )
					{
						z = 6 ;
					}
          len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
          V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;
        }
#endif // norm/plus/x9e
#endif // PCBX9LITE
#endif // PCBXLITE
#endif // PCBX7
#endif // PCBX9D
#if defined(PCBX12D) || defined(PCBX10)
        for( x = -15, y = 2 ; y < 9 ; x += 5, y += 1 )
        for( x = -15, y = 2 ; y < 9 ; x += 5, y += 1 )
        {
        	uint8_t z = y ;
#if defined(PCBX10)
					if ( ( y == 5 ) || ( y == 3 ) || ( y == 7 ) )
#endif
#if defined(PCBX12D)
					if ( y == 5 )
#endif
					{
						continue ;
					}
					if ( z == 7 )
					{
						z = 9 ;
					}
					else if ( z == 3 )
					{
						z = 8 ;
					}
					else if ( z == 2 )
					{
						z = 6 ;
					}
					else if ( z == 6 )
					{
						z = 5 ;
					}
					else if ( z == 8 )
					{
						z = 7 ;
					}
          len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
          V_BAR(SCREEN_WIDTH/2+12+x + X12SMALL_OFFSET,SCREEN_HEIGHT-8+11, len ) ;
				}
#endif
#ifdef PCBLEM1
				x = 1 ;
				y = 4 ;
        uint8_t z = y ;
        len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
        V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;

//				if (g_eeGeneral.extraPotsSource[0] )
//				{
//					x = -4 ;
//					y = 5 ;
//    	    uint8_t z = y ;
//      	  len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
//        	V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;
//				}

//				if (g_eeGeneral.extraPotsSource[1] )
//				{
//					x = 6 ;
//					y = 6 ;
//    	    uint8_t z = y ;
//      	  len = ((calibratedStick[z]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
//        	V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-8, len ) ;
//				}
#endif
    }
#if defined(PCBX12D) || defined(PCBX10)
		popPlotType() ;
#endif
#endif // ARUNI
}

extern int16_t calcExpo( uint8_t channel, int16_t value ) ;

static uint8_t s_curveChan ;
static uint8_t s_expoChan ;
static uint8_t SingleExpoChan ;

#define XD (X0-2)

#define GRAPH_FUNCTION_CURVE		0
#define GRAPH_FUNCTION_EXPO			1

void drawFunction( uint8_t xpos, uint8_t function )
{
  int8_t yv ;
  int8_t prev_yv = 127 ;
	
	pushPlotType( PLOT_BLACK ) ;
	for ( int8_t xv = -WCHART ; xv <= WCHART ; xv++ )
	{
		if ( function == GRAPH_FUNCTION_CURVE )
		{
    	yv = intpol(xv * RESX / WCHART, s_curveChan) * WCHART / RESX ;
		}
		else
		{
    	yv = (calcExpo( s_expoChan, xv * RESX / WCHART) * WCHART + RESX/2) / RESX ;
		}
    if (prev_yv == 127)
		{
			prev_yv = yv ;
		}
		uint8_t len = abs(yv-prev_yv) ;
    if (len <= 1)
		{
    	lcd_plot(xpos + xv, Y0 - yv) ;
		}
		else
		{
      uint8_t tmp = (prev_yv < yv ? 0 : len-1 ) ;
      lcd_vline(xpos+xv, Y0 - yv - tmp, len ) ;
		}	
		if ( yv )
		{
     	lcd_plot(xpos + xv, Y0 ) ;
		}
		prev_yv = yv ;
	}
	popPlotType() ;
}


void drawCurve( uint8_t offset )
{
  uint8_t cv9 = s_curveChan >= MAX_CURVE5 ;
 	int8_t *crv ;
	uint32_t points = cv9 ? 9 : 5 ;
 	crv = cv9 ? g_model.curves9[s_curveChan-MAX_CURVE5] : g_model.curves5[s_curveChan];
	if ( s_curveChan >= MAX_CURVE5 + MAX_CURVE9 )
	{
		if ( s_curveChan <= MAX_CURVE5 + MAX_CURVE9 + 1 )
		{ // An xy curve
			cv9 = 2 ;
			crv = ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 ) ? g_model.curvexy : g_model.curve2xy ;
		}
		else
		{
			crv = g_model.curve6 ;
			cv9 = 3 ;
			points = 6 ;
		}
	}
	pushPlotType( PLOT_BLACK ) ;
	lcd_vline(XD, Y0 - WCHART, WCHART * 2);
  
	for(uint8_t i=0; i < points ; i++)
  {
    uint8_t xx ;
		if ( cv9 == 2 )
		{
    	xx = XD-1+crv[i+9]*WCHART/100 ;
		}
		else
		{
			if ( cv9 == 3 )
			{
				xx = XD-1-WCHART+i*WCHART*2/5 ;
			}
			else
			{
				xx = XD-1-WCHART+i*WCHART/(cv9 ? 4 : 2);
			}
		}
    uint8_t yy = Y0-crv[i]*WCHART/100;

    if(offset==i)
    {
			lcd_rect( xx-1, yy-2, 5, 5 ) ;
    }
    else
    
		{
			lcd_rect( xx, yy-1, 3, 3 ) ;
    }
  }

	drawFunction( XD, GRAPH_FUNCTION_CURVE ) ;
	
	popPlotType() ;
}

void menuProcCurveOne(uint8_t event)
{
  uint8_t cv9 = s_curveChan >= MAX_CURVE5 ;
	uint32_t points = cv9 ? 9 : 5 ;
	if ( s_curveChan >= MAX_CURVE5 + MAX_CURVE9 )
	{
		cv9 = 2 ;
	}
	if ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 + 2 )
	{
		points = 6 ;
		cv9 = 3 ;
	}
	static int8_t dfltCrv;

	TITLE(PSTR(STR_CURVE)) ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, ( cv9 == 2 ) ? 17 : points ) ;
		
		if ( event == EVT_ENTRY )
		{
			dfltCrv = 0 ;
		}
		lcd_outdezAtt(7*FW, 0, s_curveChan+1, INVERS);

	uint8_t  preset = points ;
	uint8_t  sub    = mstate2.m_posVert ;
	uint8_t blink = InverseBlink ;
	int8_t *crv ;
	if ( ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 ) || ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 + 1 ) )
	{ // The xy curve
		crv = ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 ) ? g_model.curvexy : g_model.curve2xy ;
		uint8_t i ;
		uint8_t j ;
		uint8_t k ;
		j = sub > 8 ? 2 : 0 ;
		k = sub & 1 ;
		sub >>= 1 ;
		if ( k == 0 )
		{
			sub += 9 ;
		}
		for ( i = 0; i < 7; i++)
		{
  	  uint8_t y = i * FH + 8 ;
  	  uint8_t attr = (k==0) && (sub == j+i+9) ? blink : 0 ;
			lcd_outdezAtt(4 * FW, y, crv[j+i+9], attr);
  	  attr = (k==1) && (sub == j+i) ? blink : 0 ;
    	lcd_outdezAtt(8 * FW, y, crv[j+i], attr);
		}
		int8_t min = -100 ;
		int8_t max = 100 ;
		if ( k == 0) // x value
		{
			if ( sub > 9 )
			{
				min = crv[sub-1] ;
			}
			if ( sub < 17 )
			{
				max = crv[sub+1] ;
			}
		}
		CHECK_INCDEC_H_MODELVAR( crv[sub], min, max ) ;
		if ( sub > 8 )
		{
			sub -= 9 ;
		}
// Draw the curve
		drawCurve( sub ) ;
	}
	else
	{
		crv = cv9 ? g_model.curves9[s_curveChan-MAX_CURVE5] : g_model.curves5[s_curveChan];
		if ( s_curveChan == MAX_CURVE5 + MAX_CURVE9 + 2 )
		{
			crv = g_model.curve6 ;
		}

		for (uint8_t i = 0; i < 5; i++)
		{
  	  uint8_t y = i * FH + 16;
  	  uint8_t attr = sub == i ? blink : 0;
  	  lcd_outdezAtt(4 * FW, y, crv[i], attr);
			if( cv9 )
			{
				if ( points == 6 )
				{
					if ( i == 0 )
					{
			    	attr = sub == i + 5 ? blink : 0;
	  	  		lcd_outdezAtt(8 * FW, y, crv[i + 5], attr);
					}
				}
				else if ( i < 4 )
				{
			    attr = sub == i + 5 ? blink : 0;
  	  		lcd_outdezAtt(8 * FW, y, crv[i + 5], attr);
				}
			}
		}
		lcd_putsAtt( 2*FW, 7*FH,PSTR(STR_PRESET), (sub == preset) ? blink : 0);


		if( sub==preset) 
		{
			if ( s_editMode )
			{
				int8_t t ;
				Tevent = event ;
				t = dfltCrv ;
	  	  dfltCrv = checkIncDec( dfltCrv, -4, 4, 0);
	  	  if (dfltCrv != t)
				{
					uint8_t offset = cv9 ? 4 : 2 ;
					if ( points == 6 )
					{
						for (int8_t i = -5 ; i <= 5 ; i += 2 )
						{
						 	crv[(i+5)/2] = i*dfltCrv* 25 / 5 ;
						}
				  }
					else
					{
						for (int8_t i = -offset; i <= offset; i++) crv[i+offset] = i*dfltCrv* 25 / offset ;
					}
	  	    STORE_MODELVARS;        
	  	  }
			}
		} 
		else  /*if(sub>0)*/
		{
		 CHECK_INCDEC_H_MODELVAR( crv[sub], -100,100);
		}

// Draw the curve
		drawCurve( sub ) ;
	}
}



void menuProcCurve(uint8_t event)
{
	TITLE( PSTR(STR_CURVES) ) ;
	static MState2 mstate2 ;
	EditType = EE_MODEL ;

	mstate2.check_columns(event,1+MAX_CURVE5+MAX_CURVE9-1-1+1+1+1) ;

  int8_t  sub    = mstate2.m_posVert ;

//#if defined(PCBX12D) || defined(PCBX10)
//  uint8_t t_pgOfs = evalOffsetLarge(sub);
//#else
	uint8_t t_pgOfs = evalOffset( sub ) ;
//#endif

  switch (event)
	{
    case EVT_KEY_FIRST(KEY_RIGHT):
    case EVT_KEY_FIRST(KEY_MENU):
    case EVT_KEY_BREAK(BTN_RE):
        s_curveChan = sub;
     		killEvents(event);
				Tevent = 0 ;
        pushMenu(menuProcCurveOne);
    break;
  }

    uint8_t y    = 1*FH;
    for (uint8_t i = 0; i < SCREEN_LINES-1; i++)
		{
      uint8_t k = i + t_pgOfs;
      uint8_t attr = sub == k ? INVERS : 0;
      if(y>(SCREEN_LINES-1)*FH) break;
      lcd_putsAtt(   FW*0, y,PSTR(STR_CV),attr);
      lcd_outdezAtt( (k<9) ? FW*3-1 : FW*4-2, y,k+1 ,attr);

      y += FH ;
    }


 		s_curveChan = sub ;
		drawCurve( 100 ) ;
}

//void sticksToTrims() // copy state of 3 primary to subtrim
//{
//	uint8_t thisPhase ;
//	thisPhase = getFlightPhase() ;
//	CurrentPhase = thisPhase ;

//extern int16_t rawSticks[4] ;

//  for(uint8_t i=0; i<4; i++)
//	{
//		if(!IS_THROTTLE(i))
//		{
//			int16_t v = rawSticks[i] / 2 ;
//			int16_t original_trim = getTrimValue(thisPhase, i);
//			if ( ( v > 10 ) || ( v < -10 ) )
//			{
//				v = max(min( original_trim + v,125),-125) ;
//				setTrimValue(thisPhase, i, v ) ;
//			}
//		}
//	}
//  STORE_MODELVARS_TRIM;
//  audioDefevent(AU_WARNING2);
//}

extern int16_t rawSticks[4] ;

void setStickCenter(uint32_t toSubTrims ) // copy state of 3 primary to subtrim
{
	uint8_t thisPhase ;
  int16_t zero_chans512_before[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS];
  int16_t zero_chans512_after[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS];

	thisPhase = getFlightPhase() ;
	CurrentPhase = thisPhase ;

	if ( ( !g_model.instaTrimToTrims) || toSubTrims )
	{
  	perOut(zero_chans512_before,NO_TRAINER | NO_INPUT | FADE_FIRST | FADE_LAST); // do output loop - zero input channels
  	perOut(zero_chans512_after,NO_TRAINER | FADE_FIRST | FADE_LAST); // do output loop - zero input channels

  	for(uint8_t i=0; i<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS; i++)
  	{
			LimitData *pld = &g_model.limitData[i] ;
	#if EXTRA_SKYCHANNELS
			if ( i >= NUM_SKYCHNOUT )
			{
				pld = &g_model.elimitData[i-NUM_SKYCHNOUT] ;
			}
	#endif			
  	  int16_t v = pld->offset;
  	  v += pld->revert ?
  	                (zero_chans512_before[i] - zero_chans512_after[i]) :
  	                (zero_chans512_after[i] - zero_chans512_before[i]);
  	  pld->offset = max(min(v,1000),-1000); // make sure the offset doesn't go haywire
  	}
	}

  for(uint8_t i=0; i<4; i++)
	{
		if(!IS_THROTTLE(i))
		{
			
			if ( ( g_model.instaTrimToTrims) && !toSubTrims )
			{
				int16_t v = rawSticks[i] / 2 ;
				int16_t original_trim = getTrimValue(thisPhase, i);
				if ( ( v > 10 ) || ( v < -10 ) )
				{
					v = max(min( original_trim + v,125),-125) ;
					setTrimValue(thisPhase, i, v ) ;
				}
			}
			else
			{
				int16_t original_trim = getTrimValue(thisPhase, i);
      	for (uint8_t phase=0; phase<MAX_MODES; phase +=  1)
				{
      	  int16_t trim = getRawTrimValue(phase, i);
      	  if (trim <= TRIM_EXTENDED_MAX)
					{
      	    setTrimValue(phase, i, trim - original_trim);
					}
				}
			}
		}
	}
  STORE_MODELVARS_TRIM;
  audioDefevent(AU_WARNING2);
}

void menuProcLimits(uint8_t event)
{
	TITLE(PSTR(STR_LIMITS));
	EditType = EE_MODEL ;
	static MState2 mstate2;
	event = mstate2.check_columns( event, NUM_SKYCHNOUT+2-1-1+EXTRA_SKYCHANNELS ) ;
	Columns = 3 ;

	uint8_t y = 0;
	uint8_t k = 0;
	uint8_t  sub = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
	uint8_t t_pgOfs ;

//#if defined(PCBX12D) || defined(PCBX10)
//  t_pgOfs = evalOffsetLarge(sub);
//#else
	t_pgOfs = evalOffset( sub ) ;
//#endif

	if ( sub < NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
	{
    lcd_outdez( 13*FW, 0, g_chans512[sub]/2 + 1500 ) ;
	}

	switch(event)
	{
    case EVT_KEY_LONG(KEY_MENU):
        if(sub>=0 && sub<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)
				{
            int16_t v = g_chans512[sub - t_pgOfs];
            LimitData *ld = &g_model.limitData[sub];
#if EXTRA_SKYCHANNELS
						if ( sub >= NUM_SKYCHNOUT )
						{
							ld = &g_model.elimitData[sub-NUM_SKYCHNOUT] ;
						}
#endif			
            switch (subSub) {
            case 0:
                ld->offset = (ld->revert) ? -v : v;
                STORE_MODELVARS;
                break;
            }
        }
				if(sub==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)
				{
  			  //last line available - add the "copy trim menu" line
  			  s_noHi = NO_HI_LEN;
  			  killEvents(event);
  				s_editMode = 0 ;
  			  setStickCenter(1); //if highlighted and menu pressed - copy trims
				}
        break;
	}
//  lcd_puts_P( 4*FW, 1*FH,PSTR("subT min  max inv"));
  int8_t limit = (g_model.extendedLimits ? 125 : 100);
	for(uint8_t i=0; i<SCREEN_LINES-1; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    if(k==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS) break;
    LimitData *ld = &g_model.limitData[k];
#if EXTRA_SKYCHANNELS
		if ( k >= NUM_SKYCHNOUT )
		{
			ld = &g_model.elimitData[k-NUM_SKYCHNOUT] ;
		}
#endif			
    int16_t v = g_chans512[k] - ( (ld->revert) ? -ld->offset : ld->offset) ;
		
    char swVal = '-';  // '-', '<', '>'
		if( v >  50) swVal = ld->revert ? 127 : 126 ;// Switch to raw inputs?  - remove trim!
    if( v < -50) swVal = ld->revert ? 126 : 127 ;
    putsChn(0,y,k+1,0);
    lcd_putc(12*FW+FW/2, y, swVal ) ; //'<' : '>'
    
		for(uint8_t j=0; j<4;j++)
		{
        uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
				uint8_t active = (attr && (s_editMode || P1values.p1valdiff)) ;
				if ( active )
				{
					StickScrollAllowed = 3 ;		// Block L/R while editing
				}
				int16_t value ;
				int16_t t = 0 ;
				if ( g_model.sub_trim_limit )
				{
					if ( ( t = ld->offset ) )
					{
						if ( t > g_model.sub_trim_limit )
						{
							t = g_model.sub_trim_limit ;
						}
						else if ( t < -g_model.sub_trim_limit )
						{
							t = -g_model.sub_trim_limit ;
						}
					}
				}
				value = t / 10 ;
      switch(j)
      {
        case 0:
            lcd_outdezAtt(  7*FW+3, y,  ld->offset, attr|PREC1);
            if(active)
						{
							StepSize = 50 ;
              ld->offset = checkIncDec16( ld->offset, -1000, 1000, EE_MODEL);
            }
        break;
        case 1:
					value += (int8_t)(ld->min-100) ;
					if ( value < -125 )
					{
						value = -125 ;						
					}
          lcd_outdezAtt(  12*FW, y, value,   attr);
            if(active) {
                ld->min -=  100;
		            CHECK_INCDEC_H_MODELVAR( ld->min, -limit,25);
                ld->min +=  100;
            }
        break;
        case 2:
					value += (int8_t)(ld->max+100) ;
					if ( value > 125 )
					{
						value = 125 ;						
					}
          lcd_outdezAtt( 17*FW, y, value,    attr);
					if ( t )
					{
						lcd_rect( 9*FW-4, y-1, 56, 9 ) ;
					}
            if(active) {
                ld->max +=  100;
                CHECK_INCDEC_H_MODELVAR( ld->max, -25,limit);
                ld->max -=  100;
            }
        break;
        case 3:
					ld->revert = hyphinvMenuItem( ld->revert, y, attr ) ;
        break;
      }
    }
	}
	if(k==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)
	{
    //last line available - add the "copy trim menu" line
    uint8_t attr = (sub==NUM_SKYCHNOUT+EXTRA_SKYCHANNELS) ? INVERS : 0;
		if ( attr )
		{
			Columns = 0 ;
		}
    lcd_putsAtt(  3*FW,y,PSTR(STR_COPY_TRIM),s_noHi ? 0 : attr);
	}
}



static uint8_t onoffItem( uint8_t value, uint8_t y, uint8_t condition )
{
	return checkIndexed( y, 0, value, condition ) ;
}

static uint8_t offonItem( uint8_t value, uint8_t y, uint8_t condition )
{
	return 1-onoffItem( 1-value, y, condition ) ;
}

static uint8_t onoffMenuItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition )
{
    lcd_puts_Pleft(y, s);
		return onoffItem( value, y, condition ) ;
}

static uint8_t offonMenuItem( uint8_t value, uint8_t y, const char *s, uint8_t condition )
{
	return 1-onoffMenuItem( 1-value, y, s, condition ) ;
}

const uint8_t LogLookup[] =
{
 1, 2, 3, 4, 7, 8, 9,10,11,12,
13,16,17,18,19,20,21,25,26,27,
28,29,30,31,32,33,34,35,36,37,38,
39,40,41,42,43,44,45,48,49,50,
51,52,53,54,55,56,57,58,59,60,
61,62,63,64,65,66,67,68,69,70,
71,72,73,74,75,76,77,78,79,80,
81
} ;

const uint8_t LogRateMap[] = { 2, 0, 1 } ;


void menuLogging(uint8_t event)
{
	TITLE(XPSTR("Logging"));
	static MState2 mstate2;
	event = mstate2.check_columns( event, 1+sizeof(LogLookup)+5+4 ) ;

	uint8_t sub = mstate2.m_posVert ;
	uint8_t blink = InverseBlink ;
//#if defined(PCBX12D) || defined(PCBX10)
//  uint8_t t_pgOfs = evalOffsetLarge(sub);
//#else
	int8_t t_pgOfs = evalOffset( sub ) ;
//#endif
  uint8_t k ;

  for( CPU_UINT j=0 ; j < SCREEN_LINES-1 ; j += 1 )
	{
		uint8_t y = (1+j)*FH ;
    k=j+t_pgOfs;
    uint8_t attr = (sub==k) ? blink : 0 ;

		if ( k == 0 )
		{
			lcd_puts_Pleft( y, PSTR(STR_LOG_SWITCH) ) ;
			g_model.logSwitch = edit_dr_switch( 15*FW, y, g_model.logSwitch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
		}
		else if ( k == 1 )
		{
			lcd_puts_Pleft( y, PSTR(STR_LOG_RATE) ) ;
			lcd_putsAttIdx( 15*FW, y, XPSTR("\0041.0s2.0s0.5s"), g_model.logRate, attr ) ;
 			if(attr)
			{
				
				g_model.logRate = checkOutOfOrder( g_model.logRate, (uint8_t *)LogRateMap, 3 ) ;
				
//				uint8_t x = g_model.logRate + 1 ;
//				if ( x > 2 )
//				{
//					x = 0 ;
//				}
//				CHECK_INCDEC_H_MODELVAR_0( x, 2 ) ;
//				g_model.logRate = (x == 0) ? 2 : x - 1 ;
   		}
		}
		else if ( k == 2 )
		{
			lcd_puts_Pleft( y, XPSTR("New File") ) ;
			g_model.logNew = onoffItem( g_model.logNew, y, attr ) ;
		}
		else if ( k == 3 )
		{
//			lcd_puts_Pleft( y, XPSTR("Data Timeout(s)") ) ;
//			lcd_outdezAtt( 18*FW, y, g_model.telemetryTimeout + 25, attr|PREC1 ) ;
			lcd_xlabel_decimal( 18*FW, y, g_model.telemetryTimeout + 25, attr|PREC1, XPSTR("Data Timeout(s)") ) ;
 			if(attr)
			{
				CHECK_INCDEC_H_MODELVAR_0( g_model.telemetryTimeout, 75 ) ;
   		}
		}
		else
		{
			uint32_t index = LogLookup[k-4]-1 ;
#ifndef BITBAND_SRAM_REF
			uint32_t bit ;
			uint32_t offset ;
			uint32_t value ;
#endif
			if ( k == 4+sizeof(LogLookup) )
			{
				index = LOG_LAT ;
				lcd_puts_Pleft( y, XPSTR("Lat") ) ;
			}
			else if ( k == 5+sizeof(LogLookup) )
			{
				index = LOG_LONG ;
				lcd_puts_Pleft( y, XPSTR("Long") ) ;
			}
			else if ( k == 6+sizeof(LogLookup) )
			{
				index = LOG_BTRX ;
				lcd_puts_Pleft( y, XPSTR("BtRx") ) ;
			}
			else if ( k > 6+sizeof(LogLookup) )
			{
				index = LOG_STK_THR + k - (7+sizeof(LogLookup)) ;
				lcd_putsAttIdx( 0, y, XPSTR("\007Stk-THRStk-AILStk-ELEStk-RUD"), index - LOG_STK_THR, attr ) ;
  			
//				lcd_outdezAtt( 10*FW, y, index, 0 ) ;
//  			lcd_outdezAtt( 12*FW, y, index - LOG_STK_THR, 0 ) ;

				lcd_putc( 122, y, '\202');
			}
			else
			{
				uint32_t idx ;
				idx = index ;
				if ( idx > TELEM_GAP_START )
				{
					idx += 8 ;
				}
				putsAttIdxTelemItems( 0, y, idx+1, 0 ) ;
				if ( index < HUBDATALENGTH )
				{
					if ( telemItemValid( idx ) )
					{
    				lcd_putc( 122, y, '\202');
					}
//					int8_t i ;
//					if ( index >= TELEM_GAP_START + 8 )
//					{
//						index -= 8 ;
//					}
//					i = TelemIndex[index] ;
//					if ( i >= 0 )
//					{
//						if (TelemetryDataValid[i] )
//						{
//    					lcd_putc( 122, y, '\202');
//						}
//					}
//					else
//					{
//						if ( TelemValid[index] == 0 )
//						{
//    					lcd_putc( 122, y, '\202');
//						}
//					}
////					else
////					{
////						if ( ( i >= CELL_1 ) && ( i <= CELL_12 ) )
////						{
////							i += FR_CELL1 - CELL_1 ;
////							if (TelemetryDataValid[i] )
////							{
////    						lcd_putc( 122, y, '\202');
////							}
////						}
////					}
					if ( attr )
					{
						putsTelemetryChannel( 12*FW, 0, idx, get_telemetry_value(index), 0, TELEM_UNIT ) ;
					}
				}
			}

#ifdef BITBAND_SRAM_REF
			uint32_t *p ;
			p = (uint32_t *) (BITBAND_SRAM_BASE + ((uint32_t)&g_model.LogDisable - BITBAND_SRAM_REF) * 32) ;
			p[index] = offonItem( p[index], y, attr ) ;
#else
			offset = index >> 5 ;
			bit = 1 << (index & 0x0000001F) ;
			value = 0 ;
			if ( g_model.LogDisable[offset] & bit )
			{
				value = 1 ;
			}
			value = offonItem( value, y, attr ) ;
			if ( value )
			{
				g_model.LogDisable[offset]	|= bit ;
			}
			else
			{
				g_model.LogDisable[offset]	&= ~bit ;
			}
#endif
		}
	}
}

#ifdef BLOCKING
void menuBlocking(uint8_t event)
{
	TITLE(XPSTR("Blocking"));
	static MState2 mstate2;
	event = mstate2.check_columns( event, sizeof(LogLookup)-1 ) ;

	uint8_t sub = mstate2.m_posVert ;
	uint8_t blink = InverseBlink ;
  uint8_t t_pgOfs = evalOffset(sub);
  uint8_t k ;

  for( CPU_UINT j=0 ; j<7 ; j += 1 )
	{
		uint8_t y = (1+j)*FH ;
    k=j+t_pgOfs;
    uint8_t attr = (sub==k) ? blink : 0 ;

		uint32_t index = LogLookup[k]-1 ;
		uint32_t bit ;
		uint32_t offset ;
		uint32_t value ;

		putsAttIdxTelemItems( 0, y, index+1, 0 ) ;
		if ( index < HUBDATALENGTH )
		{
			int8_t i ;
			if ( index >= TELEM_GAP_START + 8 )
			{
				index -= 8 ;
			}
			i = TelemIndex[index] ;
			if ( i >= 0 )
			{
				if (TelemetryDataValid[i] )
				{
    			lcd_putc( 122, y, '\202');
				}
			}
			if ( attr )
			{
				putsTelemetryChannel( 12*FW, 0, index, get_telemetry_value(index), 0, TELEM_UNIT ) ;
			}
			if ( i >= 0 )
			{
				offset = i >> 5 ;
				bit = 1 << (i & 0x0000001F) ;
				value = 0 ;
				if ( g_model.LogNotExpected[offset] & bit )
				{
					value = 1 ;
				}
				value = onoffItem( value, y, attr ) ;
				if ( value )
				{
					g_model.LogNotExpected[offset]	|= bit ;
				}
				else
				{
					g_model.LogNotExpected[offset]	&= ~bit ;
				}
			}
			else
			{
  			lcd_putcAtt( PARAM_OFS, y, ' ', attr ) ;
			}
		}
	}
}
#endif

#if defined(PCBSKY) || defined(PCB9XT)
//const char SW_3_IDX[] = "\004sIDxsTHRsRUDsELEsAILsGEAsTRNL1  L2  L3  L4  L5  L6  L7  L8  L9  LA  LB  LC  LD  LE  LF  LG  LH  LI  LJ  LK  LL  LM  LN  LO  6POS" ;
#define NUM_MIX_SWITCHES	(7+NUM_SKYCSW)
#endif
#ifdef PCBX9D
 #ifdef REV9E
#define NUM_MIX_SWITCHES	(19+NUM_SKYCSW)
 #else // X9E
  #ifdef PCBX7
#define NUM_MIX_SWITCHES	(6+NUM_SKYCSW)
  #else // PCBX7
   #ifdef PCBX9LITE
#define NUM_MIX_SWITCHES	(5+NUM_SKYCSW)
   #else // PCBX9LITE
    #if defined(REVPLUS) || defined(REVNORM) || defined(REV19)
#define NUM_MIX_SWITCHES	(8+NUM_SKYCSW)
    #endif // norm/plus
		#ifdef PCBXLITE
#define NUM_MIX_SWITCHES	(4+NUM_SKYCSW)
    #endif // Xlite
   #endif // PCBX9LITE
  #endif // PCBX7
 #endif	// REV9E
#endif // X9D

#if defined(PCBX12D) || defined(PCBX10)
#define NUM_MIX_SWITCHES	(9+NUM_SKYCSW)
#endif

#if defined(PCBLEM1)
#define NUM_MIX_SWITCHES	(6+NUM_SKYCSW)
#endif


void putsChnOpRaw( uint8_t x, uint8_t y, uint8_t source, uint8_t switchSource, uint8_t output, uint8_t attr )
//void putsChnOpRaw( uint8_t x, uint8_t y, SKYMixData *md2, uint8_t attr )
{
//#if defined(PCBSKY) || defined(PCB9XT)
//lcd_outhex4( 0, 0, source ) ;
//lcd_outhex4( 30, 0, switchSource ) ;
//#endif
	
	if ( source == MIX_3POS )
	{
//#if defined(PCBSKY) || defined(PCB9XT)
//    lcd_putsAttIdx( x, y, PSTR(HW_SWITCHES_STR), switchSource, attr ) ;
//		setLastIdx( (char *) PSTR(HW_SWITCHES_STR), switchSource ) ;
////		lcd_putsAttIdx( x, y, SW_3_IDX,switchSource, attr ) ;
////		setLastIdx( (char *) SW_3_IDX, switchSource ) ;
//#endif
//#if defined(PCBX9D) || defined(PCBX12D)
//    lcd_putsAttIdx( x, y, PSTR(HW_SWITCHES_STR), switchSource, attr ) ;
//		setLastIdx( (char *) PSTR(HW_SWITCHES_STR), switchSource ) ;
//#endif
    putHwSwitchName( x, y, switchSource, attr ) ;
    setLastHwSwitch( switchSource ) ;
	}
	else
	{
		putsChnRaw( x, y, source, attr| MIX_SOURCE ) ;
		if ( output )
		{
			if ( ( ( source >= CHOUT_BASE + 1 ) && ( source < CHOUT_BASE + NUM_SKYCHNOUT + 1 ) ) ||
					 ( ( source >= EXTRA_CHANS_BASE + 1 ) && ( source < EXTRA_CHANS_BASE + EXTRA_SKYCHANNELS + 1 ) ) )
			{
				lcd_putsAtt( x, y, XPSTR("OP"), attr ) ;
				LastItem[0] = 'O' ;
				LastItem[1] = 'P' ;
			}
		}
	}
}

//uint16_t NumSw1 ;
//uint16_t NumSw2 ;

static uint8_t mapMixSource( uint8_t index, uint8_t switchSource )
{
	uint8_t inputIndex = index ;
	if ( index == MIX_3POS )
	{
		index += switchSource ;
#ifdef PCBX7
			if ( switchSource > 4 )
			{
				index -= 1 ;
			}
			if ( switchSource > 6 )
			{
				index -= 1 ;
			}
#endif // PCBX7
#ifdef PCBX9LITE
			if ( switchSource > 4 )
			{
				index -= 1 ;
			}
			if ( switchSource > 6 )
			{
				index -= 1 ;
			}
#endif // PCBX9LITE
#ifdef PCBLEM1
			if ( switchSource > 5 )
			{
				index -= 2 ;
			}
#endif // PCBX7
#ifdef PCBX9D
			if ( ( g_eeGeneral.analogMapping & MASK_6POS ) == 0 )
			{
				if ( switchSource > 8 )
				{
					index -= 1 ;
				}
			}
#endif

	}
	else
	{
		if ( index > MIX_3POS )
		{
			if ( index < EXTRA_POTS_START )
			{
				uint32_t num_mix_switches = NUM_MIX_SWITCHES ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
				if ( g_eeGeneral.analogMapping & MASK_6POS )
				{
					num_mix_switches += 1 ;
				}
#endif
				index += num_mix_switches-1 ;
			}
		}
	}
	if ( inputIndex < EXTRA_POTS_START )
	{
		if ( inputIndex <= MIX_TRIMS_START )
		{
			if ( inputIndex > CHOUT_BASE )
			{
				index += NUM_EXTRA_PPM ;
				if ( inputIndex >= MIX_3POS )
				{
					index += EXTRA_SKYCHANNELS ;
				}	
			}
			if ( inputIndex > EXTRA_PPM_BASE )
			{
				if ( inputIndex <= EXTRA_CHANS_BASE )
				{
					index = inputIndex - EXTRA_PPM_BASE + CHOUT_BASE ;
				}
				else
				{
					index = inputIndex - EXTRA_CHANS_BASE + MIX_3POS-1 + NUM_EXTRA_PPM ;
				}
			}
		}
	}
	return mapPots( index ) ;
}

uint8_t unmapMixSource( uint8_t index, uint8_t *switchSource )
{
	uint32_t num_mix_switches = NUM_MIX_SWITCHES ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
	if ( g_eeGeneral.analogMapping & MASK_6POS )
	{
		num_mix_switches += 1 ;
	}
#endif
//#ifdef PCBX12D
//	num_mix_switches += 1 ;
//#endif
	index = unmapPots( index ) ;
	if ( index < EXTRA_POTS_START ) // 120
	{
	 if ( index < MIX_TRIMS_START + num_mix_switches )
	 {
		if ( index > PPM_BASE + NUM_PPM )	// 20
		{
			if ( index <= PPM_BASE + NUM_PPM + NUM_EXTRA_PPM ) // 28
			{
				index += EXTRA_PPM_BASE - (PPM_BASE + NUM_PPM) + (num_mix_switches-1) ;	// + 40 + 31
			}
			else
			{
				index -= NUM_EXTRA_PPM ;	// -8
				if ( index > CHOUT_BASE + NUM_SKYCHNOUT ) // 44
				{
					if ( index <= CHOUT_BASE + NUM_SKYCHNOUT + EXTRA_SKYCHANNELS ) // 52
					{
						index += EXTRA_CHANS_BASE - (CHOUT_BASE + NUM_SKYCHNOUT) + (num_mix_switches-1) ; // 24 + 31
					}
					else
					{
						index -= EXTRA_SKYCHANNELS ; // -8
					}
				}
			}
		}
	 }
	}
	if ( index >= MIX_3POS )
	{
		if ( index > (uint32_t)MIX_3POS + (num_mix_switches-1) ) // 44 + 31
		{
			if ( index < EXTRA_POTS_START )
			{
				index -= (num_mix_switches-1) ;
			}
		}
		else
		{
			index -= MIX_3POS ;
#ifdef PCBX7
			if ( index > 3 )
			{
				index += 1 ;
			}
			if ( index > 5 )
			{
				index += 1 ;
			}
#endif // PCBX7
#ifdef PCBX9LITE
			if ( index > 3 )
			{
				index += 1 ;
			}
			if ( index > 5 )
			{
				index += 1 ;
			}
#endif // PCBX9LITE
#ifdef PCBLEM1
			if ( index > 3 )
			{
				index += 2 ;
			}
#endif // PCBX9LITE
#ifdef PCBX9D
			if ( ( g_eeGeneral.analogMapping & MASK_6POS ) == 0 )
			{
				if ( index > 7 )
				{
					index += 1 ;
				}
			}
#endif
			*switchSource = index ;
			index = MIX_3POS ;
		}
	}
	return index ;
}

const uint8_t SwitchFunctionMap[] = { 0,1,2,3,4,18,21,19,5,6,7,8,9,10,11,20,12,13,14,15,16,17} ;

static int8_t s_curItemIdx;
static uint8_t s_moveItemIdx;
static int8_t s_currDestCh;
static bool   s_currMixInsMode;

uint8_t TextIndex ;
uint8_t TextOption ;
uint8_t TextTimer ;
uint8_t TextSearching ;
uint8_t TextFound ;
uint8_t TextType ;
uint8_t TextResult ;

void menuTextHelp(uint8_t event)
{
	static MState2 mstate2;
	uint8_t item[8] ;
	uint32_t i ;
	uint32_t index ;
	uint32_t type = TextType ;
#ifdef BIG_SCREEN
	uint32_t width = 6 ;
#else
	uint32_t width = 4 ;
#endif
	uint32_t length = 5 ;
	uint8_t max = NUM_TELEM_ITEMS ;
	uint32_t num_mix_switches = NUM_MIX_SWITCHES ;
	if ( type == 1 )
	{
#ifdef PCBX7
		max = NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots - 1 ;
#else
 #ifdef PCBX9LITE
		max = NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots - 2 ;
 #else // PCBX9LITE
		max = NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots ;
 #endif // PCBX9LITE
#endif // PCBX7
	}
	else if ( type == 2 )
	{
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
		if ( g_eeGeneral.analogMapping & MASK_6POS )
		{
			num_mix_switches += 1 ;
		}
#endif
//#ifdef PCBX12D
//		num_mix_switches += 1 ;
//#endif
#ifdef PCBX7
		max = NUM_SKYXCHNRAW+1+MAX_GVARS+1+NUM_SCALERS+8+NumExtraPots + (num_mix_switches-1) - 1 + EXTRA_SKYCHANNELS - 1 + 4 ;
#else // PCBX7
 #if defined(PCBX9LITE) || defined(PCBLEM1)
		max = NUM_SKYXCHNRAW+1+MAX_GVARS+1+NUM_SCALERS+8+NumExtraPots + (num_mix_switches-1) - 1 + EXTRA_SKYCHANNELS - 2 + 4 ;
 #else // PCBX9LITE
		max = NUM_SKYXCHNRAW+1+MAX_GVARS+1+NUM_SCALERS+8+NumExtraPots + (num_mix_switches-1) - 1 + EXTRA_SKYCHANNELS + 4 ;
 #endif // PCBX9LITE
#endif // PCBX7
		if ( TextOption )
		{
			TextOption = 1 ;
		}
	}
	else if ( type == 3 )
	{
		max = CS_MAXF ;
#ifdef BIG_SCREEN
		width = 3 ;
#else
		width = 2 ;
#endif
		length = 7 ;
	}

	uint8_t rows = max + 1 ;
	uint8_t lastCol = (rows) % width ;
	if ( lastCol == 0 )
	{
		lastCol = width - 1 ;
	}
	else
	{
		lastCol -= 1 ;
	}

	rows -= 1 ;
	rows /= width ;

	TITLE(XPSTR("SELECT"));

	if ( event == EVT_ENTRY )
	{
		TextFound = 0 ;
		TextSearching = 0 ;
		TextResult = 0 ;
		TextTimer = 40 ;
		FRESULT result ;
		result = f_open( &SharedMemory.TextControl.TextFile, "/Helptel.txt", FA_READ ) ;
		if ( result == FR_OK )
		{
			SharedMemory.TextControl.TextFileOpen = 1 ;
			f_gets( (TCHAR *)SharedMemory.TextControl.TextMenuStore, 30, &SharedMemory.TextControl.TextFile ) ;
		}
	}
	else if ( checkForMenuEncoderBreak( event ) )
	{
		if ( type == 0 )
		{
			if ( TextIndex > TELEM_GAP_START )
			{
				TextIndex += 8 ;
			}
		}
		TextResult = 1 ;
  	s_editMode = 0 ;
		killEvents(event) ;
		popMenu() ;
	}

	event = mstate2.check_columns( event, rows ) ;
	if ( event == EVT_ENTRY )
	{
		if ( type == 2 )
		{
			TextIndex -= 1 ;
		}
		else if ( type == 3 )
		{
			TextIndex = locateMappedItem( TextIndex, (uint8_t *)SwitchFunctionMap, CS_MAXF ) ;
		}
		mstate2.m_posVert = TextIndex / width ;
		g_posHorz = TextIndex % width ;

	}

	uint8_t sub = mstate2.m_posVert ;
	Columns = sub == rows ? lastCol : width - 1 ;
	item[0] = '\0' ;
	
  index = s_pgOfs ;
	int8_t x = sub - index ;
  if(sub<1) index = 0 ;
	else if(x>(SCREEN_LINES-3)) index = sub - (SCREEN_LINES-3) ;
  else if(x<0) index = sub ;
	s_pgOfs = index ;

	index *= width ;
	for ( i = 0 ; i < width * (SCREEN_LINES-2) ; i += 1 )
	{
#ifdef BIG_SCREEN
		uint8_t x = (i%6)* 31 ;
		if ( width == 3 )
		{
			x = (i%3)* 62 ;
		}
#else
		uint8_t x = (i%4)* 31 ;
		if ( width == 2 )
		{
			x = ( i & 1 ) * 62 ;
		}
#endif
		uint8_t y = ((i/width)+1) * FH ;
		uint8_t att = (sub) * width + g_posHorz ;
		if ( index > max )
		{
			break ;
		}
		if ( att == index )
		{
			if ( TextIndex != att )
			{
				TextTimer = 15 ;
			}
			if ( TextResult == 0 )
			{
				TextIndex = att ;
			}
			att = INVERS ;
		}
		else
		{
			att = 0 ;
		}
		if ( type == 0 )
		{
			uint32_t idx ;
			idx = index ;
			if ( idx > TELEM_GAP_START )
			{
				idx += 8 ;
			}
			putsAttIdxTelemItems( x, y, idx, att | TSSI_TEXT ) ;
		}
		else if ( type == 1 )
		{
			putsChnRaw( x, y, unmapPots( index ), att | TSSI_TEXT ) ;
		}
		else if ( type == 2 )
		{
			uint8_t idx ;
			uint8_t swidx ;
			swidx = 0 ;
			idx = index + 1 ;
			idx = unmapMixSource( idx, &swidx ) ;
//			idx = unmapPots( idx ) ;
//			if ( idx >= MIX_3POS )
//			{
//				if ( idx > (uint32_t)MIX_3POS + (num_mix_switches-1) )
//				{
//					if ( idx < EXTRA_POTS_START )
//					{
//						idx -= (num_mix_switches-1) ;
//					}
//				}
//				else
//				{
//					swidx = idx - MIX_3POS ;
//					idx = MIX_3POS ;
//				}
//			}
			putsChnOpRaw( x, y, idx, swidx, TextOption, att ) ;
		}
		else if ( type == 3 )
		{
			uint8_t offset = SwitchFunctionMap[index] ;
			lcd_putsAttIdx( x, y, PSTR(CSWITCH_STR), offset, att ) ;
			setLastIdx( (char *) PSTR(CSWITCH_STR), offset ) ;
		}
		if ( att )
		{
			ncpystr( item, (uint8_t *)LastItem, length ) ;
		}
		index += 1 ;
	}
	if ( TextTimer )
	{
		if ( --TextTimer == 0 )
		{
			// Start looking in file
			if ( SharedMemory.TextControl.TextFileOpen )
			{
				f_lseek( &SharedMemory.TextControl.TextFile, 0 ) ;
				TextFound = 0 ;
				TextSearching = 1 ;
			}
		}
	}
	if ( TextSearching )
	{
		uint32_t i ;
		for ( i = 0 ; i < 10 ; i += 1 )
		{
					
			TCHAR *read ;
			read = f_gets( (TCHAR *)SharedMemory.TextControl.TextMenuBuffer, 30, &SharedMemory.TextControl.TextFile ) ;
			if ( !read )
			{
				TextSearching = 0 ;
				break ;
			}
			if ( strncmp( (char *)item, (char *)SharedMemory.TextControl.TextMenuBuffer, strlen((char *)item) ) == 0 )
			{
				TextFound = strlen((char *)item) ;
				for ( i =0 ; i < 3 ; i += 1 )
				{
					if ( SharedMemory.TextControl.TextMenuBuffer[TextFound++] == ',' )
					{
						break ;
					}
				}
				TextSearching = 0 ;
				break ;
			}
		}
	}

#ifdef BIG_SCREEN
	lcd_hline( 0, (SCREEN_LINES-1)*FH-1, 127 ) ;
#else
	lcd_hline( 0, 55, 127 ) ;
#endif
	if ( TextFound )
	{
		lcd_putsAtt( 0, (SCREEN_LINES-1)*FH, (char *)&SharedMemory.TextControl.TextMenuBuffer[TextFound], 0 ) ;
	}
//	else
//	{
//		lcd_putsAtt( 0, 7*FH, (char *)item, 0 ) ;
//	}

//	lcd_outdez( 8*FW, 0, TextIndex ) ;
//	lcd_outdez( 11*FW, 0, TextTimer ) ;
//	lcd_outdez( 14*FW, 0, ttt ) ;
//	lcd_outdez( 17*FW, 0, uuu ) ;

//  lcd_outhex4( 5*FW, 7*FH, strlen((char *)item) ) ;
//  lcd_outhex4( 9*FW, 7*FH, TextMenuBuffer[1] ) ;
//  lcd_outhex4( 13*FW, 7*FH, TextMenuBuffer[2] ) ;
//  lcd_outhex4( 17*FW, 7*FH, TextMenuBuffer[3] ) ;

//	lcd_outdez( 7*FW, 7*FH,  TextDebug1 ) ;
//	lcd_outdez( 10*FW, 7*FH, TextDebug2 ) ;
//	lcd_outdez( 13*FW, 7*FH, TextDebug3 ) ;
//	lcd_putc( 15*FW, 0, TextMenuBuffer[0] ) ;

}

void copyFileName( char *dest, char *source, uint32_t size )
{
	uint32_t i ;
	for ( i = 0 ; i < size ; i += 1 )
	{
		char c ;
		c = source[i] ;
		if ( c == '.' )
		{
			break ;
		}
		dest[i] = c ;
	}
	while ( i < size )
	{
		dest[i++] = '\0' ;
	}
}



void menuSelectScript(uint8_t event, uint32_t type)
{
	struct fileControl *fc = &FileControl ;
	uint32_t i ;

  TITLE( "SELECT SCRIPT" ) ;

  switch(event)
	{
    case EVT_ENTRY:
			if ( type )
			{
//				setupFileNames( (TCHAR *)"/SCRIPTS/TELEMETRY", fc, (char *)"BAS" ) ;
				setupFileNames( (TCHAR *)"\\SCRIPTS\\MODEL", fc, (char *)"BAS" ) ;
			}
			else
			{
				setupFileNames( (TCHAR *)"\\SCRIPTS\\TELEMETRY", fc, (char *)"BAS" ) ;
			}
    break ;
	}
	
	i = fileList( event, &FileControl ) ;
	if ( i == 1 )	// Select
	{
		// Restore the model
		cpystr( (uint8_t *)SelectedVoiceFileName, (uint8_t *)SharedMemory.FileList.Filenames[fc->vpos] ) ;
		killEvents(event) ;
    popMenu() ;
	}
	else if ( i == 2 )	// EXIT
	{
    killEvents(event) ;
    popMenu() ;
	}
	FileSelectResult = i ;
}

void menuSelectTelemetryScript(uint8_t event)
{
	menuSelectScript( event, 0 ) ;
}

void menuSelectModelScript(uint8_t event)
{
	menuSelectScript( event, 1 ) ;
}

void menuCustomTelemetry(uint8_t event)
{
	TITLE(PSTR(STR_CUSTOM_DISP));
	static MState2 mstate2;
#ifdef WIDE_SCREEN	
	ImageDisplay = 0 ;
#if defined(PCBX12D) || defined(PCBX10)
	uint32_t rows = 6 ;
	uint32_t page1limit = 1 ;
	uint32_t page2limit = 3 ;
#else
	uint32_t rows = 3 ;
	uint32_t page1limit = 1 ;
#endif
	if ( g_model.customDisplay1Extra[6] == 0 )
	{
		rows += 2 ;
		page1limit += 2 ;
#if defined(PCBX12D) || defined(PCBX10)
		page2limit += 2 ;
#endif
	}
	if ( g_model.customDisplay2Extra[6] == 0 )
	{
		rows += 2 ;
#if defined(PCBX12D) || defined(PCBX10)
		page2limit += 2 ;
#endif
	}
	event = mstate2.check_columns( event, rows ) ;
	
	Columns = 2 ;

	uint8_t sub = mstate2.m_posVert ;

	uint8_t *pindex ;
	uint8_t *epindex ;
	uint8_t chr = '1' ;
//  lcd_puts_Pleft( FH, PSTR(STR_CUSTOM_DISP) );
	pindex = g_model.customDisplayIndex ;
	epindex = g_model.customDisplay1Extra ;
	
	if ( sub <= page1limit )
	{
		lcd_puts_P( 20*FW-4, 7*FH, XPSTR("->") ) ;
	}
	
	uint8_t subN = 0 ;
	uint8_t subSub = g_posHorz;
	if ( sub >= page1limit + 1 )
	{
		subN = page1limit + 1 ;
		pindex = g_model.customDisplay2Index ;
		epindex = g_model.customDisplay2Extra ;
		chr = '2' ;
	}
#if defined(PCBX12D) || defined(PCBX10)
	if ( sub >= page2limit + 1 )
	{
		subN = page1limit + 1 ;
		pindex = &g_model.customDisplay1Extra[3] ;
		chr = 'X' ;
	}
#endif
	lcd_putc( 15*FW, 0, chr ) ;

	if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		subSub = g_posHorz = saveHpos ;
		if ( TextResult )
		{
			uint8_t index = sub - 1 ;
#if defined(PCBX12D) || defined(PCBX10)
			if ( sub >= page2limit )
			{
				index -= page2limit ;
				pindex[index] = TextIndex ;
			}
			else
#endif
			{
				if ( index >= page1limit + 1 )
				{
					index -= page1limit + 1 ;
				}
				if ( subSub == 2 )
				{
					epindex[index] = TextIndex ;
				}
				else
				{
					pindex[index*2+subSub] = TextIndex ;
				}
			}
	    eeDirty(EE_MODEL) ;
		}
		else if ( FileSelectResult == 1 )
		{
	 		copyFileName( (char *)pindex, SelectedVoiceFileName, 6 ) ;
			basicLoadModelScripts() ;
   		eeDirty(EE_MODEL) ;		// Save it
		}
	}

#if defined(PCBX12D) || defined(PCBX10)
 if ( sub <= page2limit )
 {
#endif
	lcd_puts_Pleft( FH, XPSTR("Type") ) ;
	uint32_t oldValue = epindex[6] ;
	epindex[6] = checkIndexed( FH, XPSTR(FWx7"\001""\006ValuesScript"), epindex[6], (sub==subN) ) ;
	if ( epindex[6] != oldValue )
	{
		memset( pindex, epindex[6] ? ' ' : 0, 6 ) ;
		if ( epindex[6] == 0 )
		{
			basicLoadModelScripts() ;
		}
	}
	subN++;

	if ( epindex[6] )
	{
    uint8_t attr = sub==subN ? InverseBlink : 0 ;
		alphaEditName( 12*FW, 2*FH, pindex, 6, attr, (uint8_t *)XPSTR( "Script File") ) ;
		validateName( pindex, 6 ) ;
	  if( attr )
		{
			if ( AlphaEdited )
			{
				AlphaEdited = 0 ;
				basicLoadModelScripts() ;
			}
			if ( checkForMenuEncoderLong( event ) )
			{
				TextResult = 0 ;
      	pushMenu( menuSelectTelemetryScript ) ;
			}
		} 
	}
	else
	{
		for (uint8_t j=0; j<3; j++)
		{
			for ( uint8_t k = 0 ; k < 3 ; k += 1 )
			{
				uint8_t x = k * 64 ;
				uint8_t y = ( j*2 + 2 ) * FH ;
			  uint8_t attr = ((sub==subN && subSub==k)) ? InverseBlink : 0 ;
				if ( attr )
				{
					if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
					{
					// Long MENU pressed
//					if ( sub >= page1limit + 1 )
//					{
//						sub -= page1limit + 1 ;
//					}
						saveHpos = subSub ;
						TextIndex = (k == 2) ? epindex[j] : pindex[j*2+k] ;
						if ( TextIndex >= TELEM_GAP_START + 8 )
						{
							TextIndex -= 8 ;
						}
  			  	TextType = 0 ;
						killEvents(event) ;
						pushMenu(menuTextHelp) ;
					}
				}
				uint8_t *p = &pindex[j*2+k] ;
				if ( k == 2 )
				{
					p = &epindex[j] ;
				}
				if ( *p )
				{
					uint32_t index = *p - 1 ;
					putsAttIdxTelemItems( x, y, *p, attr | TSSI_TEXT ) ;
					putsTelemetryChannel( x+5*FW-1, y+FH, index, get_telemetry_value(index), 0, TELEM_UNIT ) ;
					if ( index < 2 )		// A1 or A2
					{
  					lcd_outdezAtt( x+52, y, get_telemetry_value(index), 0 ) ;
						lcd_putc( x+32, y, '[' ) ;
						lcd_putc( x+54, y, ']' ) ;
					}
				}
				else
				{
  	  		lcd_putsAtt(  x, y, HyphenString, attr ) ;
				}
  			if ((sub==subN && subSub==k))
				{ 
					uint8_t val = *p ;
					if ( val >= TELEM_GAP_START + 8 )
					{
						val -= 8 ;
					}
					CHECK_INCDEC_H_MODELVAR_0( val, NUM_TELEM_ITEMS) ;
					if ( val > TELEM_GAP_START )
					{
						val += 8 ;
					}
					*p = val ;
				}
			}
			subN++;
		}
	}
#if defined(PCBX12D) || defined(PCBX10)
 }
 else
 {
	Columns = 0 ;
	subN = page2limit + 1 ;
	for ( uint8_t k = 0 ; k < 3 ; k += 1 )
	{
		uint8_t x = 128 ;
		uint8_t y = ( k*2 + 2 ) * FH ;
	  uint8_t attr = (sub==subN) ? InverseBlink : 0 ;
		if ( attr )
		{
			if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
			{
				TextIndex = pindex[k] ;
				if ( TextIndex >= TELEM_GAP_START + 8 )
				{
					TextIndex -= 8 ;
				}
		  	TextType = 0 ;
				killEvents(event) ;
				pushMenu(menuTextHelp) ;
			}
		}
		uint8_t *p = &pindex[k] ;
		if ( *p )
		{
			uint32_t index = *p - 1 ;
			putsAttIdxTelemItems( x, y, *p, attr | TSSI_TEXT ) ;
			putsTelemetryChannel( x+5*FW-1, y+FH, index, get_telemetry_value(index), 0, TELEM_UNIT ) ;
			if ( index < 2 )		// A1 or A2
			{
				lcd_outdezAtt( x+52, y, get_telemetry_value(index), 0 ) ;
				lcd_putc( x+32, y, '[' ) ;
				lcd_putc( x+54, y, ']' ) ;
			}
		}
		else
		{
  		lcd_putsAtt(  x, y, HyphenString, attr ) ;
		}
		uint8_t val = *p ;
		if ( val >= TELEM_GAP_START + 8 )
		{
			val -= 8 ;
		}
		CHECK_INCDEC_H_MODELVAR_0( val, NUM_TELEM_ITEMS) ;
		if ( val > TELEM_GAP_START )
		{
			val += 8 ;
		}
		*p = val ;
//		if (sub==subN) CHECK_INCDEC_H_MODELVAR_0( *p, NUM_TELEM_ITEMS) ;
		subN++ ;
	}
 }
#endif
#else
	uint32_t rows = 3 ;
	uint32_t page1limit = 1 ;
	if ( g_model.customDisplay1Extra[6] == 0 )
	{
		rows += 5 ;
		page1limit += 5 ;
	}
	if ( g_model.customDisplay2Extra[6] == 0 )
	{
		rows += 5 ;
	}
	event = mstate2.check_columns( event, rows ) ;
	
	uint8_t  sub   = mstate2.m_posVert ;

	uint8_t *pindex ;
	uint8_t *epindex ;
	uint8_t chr = '1' ;
//  lcd_puts_Pleft( FH, PSTR(STR_CUSTOM_DISP) );
	pindex = g_model.customDisplayIndex ;
	epindex = g_model.customDisplay1Extra ;
	
	if ( sub <= page1limit )
	{
		lcd_puts_P( 20*FW-4, 7*FH, XPSTR("->") ) ;
	}
	
	uint8_t subN = 0 ;
	if ( sub >= page1limit + 1 )
	{
		subN = page1limit + 1 ;
		pindex = g_model.customDisplay2Index ;
		epindex = g_model.customDisplay2Extra ;
		chr = '2' ;
	}
	lcd_putc( 15*FW, 0, chr ) ;

	if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		if ( TextResult )
		{
			uint8_t index = sub - 1 ;
			if ( index >= page1limit + 1 )
			{
				index -= page1limit + 1 ;
			}
			pindex[index] = TextIndex ;
	    eeDirty(EE_MODEL) ;
		}
		else if ( FileSelectResult == 1 )
		{
	 		copyFileName( (char *)pindex, SelectedVoiceFileName, 6 ) ;
			basicLoadModelScripts() ;
   		eeDirty(EE_MODEL) ;		// Save it
		}
	}

	lcd_puts_Pleft( FH, XPSTR("Type") ) ;
	uint32_t oldValue = epindex[6] ;
	epindex[6] = checkIndexed( FH, XPSTR(FWx7"\001""\006ValuesScript"), epindex[6], (sub==subN) ) ;
	if ( epindex[6] != oldValue )
	{
		memset( pindex, epindex[6] ? ' ' : 0, 6 ) ;
		if ( epindex[6] == 0 )
		{
			basicLoadModelScripts() ;
		}
	}
	subN++;

	if ( epindex[6] )
	{
    uint8_t attr = sub==subN ? InverseBlink : 0 ;
		alphaEditName( 12*FW, 2*FH, pindex, 6, attr, (uint8_t *)XPSTR( "Script File") ) ;
		validateName( pindex, 6 ) ;
	  if( attr )
		{
			if ( AlphaEdited )
			{
				AlphaEdited = 0 ;
				basicLoadModelScripts() ;
			}
			if ( checkForMenuEncoderLong( event ) )
			{
				TextResult = 0 ;
      	pushMenu( menuSelectTelemetryScript ) ;
			}
		} 
	}
	else
	{
		for (uint8_t j=0; j<6; j++)
		{
			uint8_t x = ( j & 1 ) ? 32 : 0 ;
			uint8_t y = ( j + 2 ) * FH ;
		  uint8_t attr = ((sub==subN) ? InverseBlink : 0);
			if ( attr )
			{
				if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
				{
					// Long MENU pressed
//					if ( sub >= page1limit + 1 )
//					{
//						sub -= page1limit + 1 ;
//					}
//					TextIndex = pindex[sub-1] ;
					TextIndex = pindex[j] ;
					if ( TextIndex >= TELEM_GAP_START + 8 )
					{
						TextIndex -= 8 ;
					}
  			  TextType = 0 ;
  			  killEvents(event) ;
					pushMenu(menuTextHelp) ;
				}
			}
			if ( pindex[j] )
			{
				uint32_t index = pindex[j] - 1 ;
				putsAttIdxTelemItems( x, y, pindex[j], attr | TSSI_TEXT ) ;
				putsTelemetryChannel( 90, y, index, get_telemetry_value(index), 0, TELEM_UNIT ) ;
				if ( index < 2 )		// A1 or A2
				{
  				lcd_outdezAtt( 120, y, get_telemetry_value(index), 0 ) ;
					lcd_putc( 98, y, '[' ) ;
					lcd_putc( 122, y, ']' ) ;
				}
			}
			else
			{
  	  	lcd_putsAtt(  x, y, HyphenString, attr ) ;
			}
  		if(sub==subN)
			{
				uint8_t val = pindex[j] ;
				if ( val >= TELEM_GAP_START + 8 )
				{
					val -= 8 ;
				}
				CHECK_INCDEC_H_MODELVAR_0( val, NUM_TELEM_ITEMS) ;
				if ( val > TELEM_GAP_START )
				{
					val += 8 ;
				}
				pindex[j] = val ;
			}
			subN++;
		}
	}
#endif
}

// FrSky WSHhi DSMx  Jeti  ArduP ArduC FrHub HubRaw FrMav Mavlk"

#if defined(PCBSKY) || defined(PCB9XT)
 #ifdef REVX
  #define MAX_TEL_OPTIONS		11
  const uint8_t TelOptions[] = {1,2,3,4,5,6,7,8,10,11,12} ;
 #else
  #define MAX_TEL_OPTIONS			10
  const uint8_t TelOptions[] = {1,2,3,5,6,7,8,10,11,12} ;
 #endif
#endif

#ifdef PCBX9D
 #define MAX_TEL_OPTIONS		8
 const uint8_t TelOptions[] = {1,2,3,5,6,7,11,12} ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
 #define MAX_TEL_OPTIONS		8
 const uint8_t TelOptions[] = {1,2,3,5,6,7,11,12} ;
#endif

#ifdef PCBLEM1
 #define MAX_TEL_OPTIONS		8
 const uint8_t TelOptions[] = {1,2,3,5,6,7,11,12} ;
#endif

// 0 Telemetry
// 1 SbusTrain
// 2 Sbus57600
// 3 BTdirect 
// 4 CppmTrain
// 5 LCDdump  
// 6 Tel+BTdir
// 7 Script
// 8 BT/ENC

#ifdef PCBSKY
 #define MAX_COM2_OPTIONS		7
 const uint8_t Com2Options[] = {0,1,2,3,5,6,7} ;
#endif

#ifdef PCBX9D
 #ifdef PCBX9LITE
 #define MAX_COM2_OPTIONS		5
 const uint8_t Com2Options[] = {0,1,2,4,7} ;
 #else
 #define MAX_COM2_OPTIONS		7
 const uint8_t Com2Options[] = {0,1,2,3,4,7,8} ;
 #endif
#endif

#if defined(PCBX12D)// || defined(PCBX10)
 #define MAX_COM2_OPTIONS		5
 const uint8_t Com2Options[] = {0,1,2,3,4} ;
#endif

#ifdef PCB9XT
 #define MAX_COM2_OPTIONS		3
 const uint8_t Com2Options[] = {0,1,2,7} ;
#endif

#ifdef PCBLEM1
 #define MAX_COM2_OPTIONS		1
 const uint8_t Com2Options[] = {0,1} ;
#endif


void menuProcTelemetry(uint8_t event)
{
	uint32_t page2SizeExtra = 0 ;
//	if ( g_model.telemetryProtocol == TELEMETRY_DSM )
//	page2SizeExtra = 1 ;
	if ( FrskyTelemetryType == FRSKY_TEL_DSM )
	{
		page2SizeExtra += 4 ;
	}

	EditType = EE_MODEL ;

#if not (defined(PCBX10))
#define TDATAITEMS	22
#else
#define TDATAITEMS	20
#endif
//#if defined(PCBSKY) || defined(PCB9XT)
// #ifdef REVX
//  #define TDATAITEMS	22
// #else
//  #define TDATAITEMS	21
// #endif
//#endif

//#ifdef PCBX9D
//  #define TDATAITEMS	21
//#endif
//#ifdef PCBX12D
//  #define TDATAITEMS	21
//#endif

		 
	TITLE(PSTR(STR_TELEMETRY));
	static MState2 mstate2;
		event = mstate2.check_columns( event, TDATAITEMS-1+page2SizeExtra ) ;

	uint8_t  sub   = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz ;
uint8_t blink;
uint8_t y = 2*FH;

//	switch(event)
//	{
//    case EVT_KEY_BREAK(KEY_DOWN):
//    case EVT_KEY_BREAK(KEY_UP):
//    case EVT_KEY_BREAK(KEY_LEFT):
//    case EVT_KEY_BREAK(KEY_RIGHT):
//      if(s_editMode)
//			{
//         FrskyAlarmSendState |= 0x30 ;	 // update Fr-Sky module when edit mode exited
//         FRSKY_setModelAlarms(); // update Fr-Sky module when edit mode exited
//			}
//    break ;
//		case EVT_ENTRY :
//  		FrskyAlarmSendState |= 0x40 ;		// Get RSSI/TSSI alarms
//		break ;
//	}

	blink = InverseBlink ;
	uint8_t subN = 0 ;
	if ( sub <= TDATAITEMS+page2SizeExtra - 1 )
	{
		lcd_puts_P( 20*FW-4, 7*FH, XPSTR("->") ) ;
	}

 if ( sub < 7 )
 {
	lcd_puts_Pleft(FH, PSTR(STR_USR_PROTO));
	{
		uint8_t b ;
		uint8_t attr = 0 ;

		b = g_model.telemetryProtocol ;
//		for ( uint32_t c = 0 ; c < MAX_TEL_OPTIONS ; c += 1 )
//		{
//			if ( TelOptions[c] == b )
//			{
//				b = c ;
//				break ;
//			}
//		}
		if(sub==subN)
		{
			Columns = 1 ;

			if (subSub==0)
			{
				attr = blink ;
				b = checkOutOfOrder( b, (uint8_t *)TelOptions, MAX_TEL_OPTIONS ) ;

//				CHECK_INCDEC_H_MODELVAR( b,0,MAX_TEL_OPTIONS-1) ;
			}
		}
//		b = TelOptions[b] ;
		g_model.telemetryProtocol = b ;
		switch ( b )
		{
			case TELEMETRY_WSHHI :
				g_model.FrSkyUsrProto = 1 ;
				g_model.DsmTelemetry = 0 ;
			break ;

			case TELEMETRY_DSM :
				g_model.FrSkyUsrProto = 0 ;
				g_model.DsmTelemetry = 1 ;
			break ;

			case TELEMETRY_FRSKY :
			default :
				g_model.FrSkyUsrProto = 0 ;
				g_model.DsmTelemetry = 0 ;
			break ;
		}
		b -= 1 ;
		lcd_putsAttIdx( 10*FW, FH, PSTR(STR_FRHUB_WSHHI), b, attr ) ;

		attr = 0 ;
		b = g_model.FrSkyImperial ;
		if(sub==subN && subSub==1)
		{
			attr = blink ;
			CHECK_INCDEC_H_MODELVAR_0(b,1); g_model.FrSkyImperial = b ;
		}
		lcd_putsAttIdx( 17*FW, FH, PSTR(STR_MET_IMP), b, attr ) ;
	
	}
	subN++;

	for (int i=0; i<4; i++)
	{
		uint32_t index = i & 1 ;
		uint8_t unit ;
    lcd_puts_Pleft(y, PSTR(STR_A_CHANNEL)) ;
    lcd_putc(FW, y, '1'+i);
		if ( i < 2 )
		{
    	putsTelemValue(16*FW, y, 255, i, (sub==subN && subSub==0 ? blink:0)|NO_UNIT ) ;
    	putsTelemValue( 21*FW, y, frskyTelemetry[i].value, i,  NO_UNIT ) ;
    	unit = g_model.frsky.channels[index].units ;

    	if (sub==subN)
			{
				Columns = 1 ;
				if ( (s_editMode || P1values.p1valdiff))
				{
    	    switch (subSub)
					{
    	    	case 0:
    	        g_model.frsky.channels[i].lratio = checkIncDec16( g_model.frsky.channels[i].lratio, 0, 1000, EE_MODEL);
    	      break;
	  	      case 1:
    	        CHECK_INCDEC_H_MODELVAR_0( g_model.frsky.channels[i].units, 3);
    	      break;
    	    }
				}
    	}
		}
		else
		{
			lcd_outdezAtt( 16*FW, y, g_model.frsky.channels[index].ratio3_4, (sub==subN && subSub==0 ? blink:0)|PREC2 ) ;
			lcd_outdezAtt( 21*FW, y, FrskyHubData[FR_A3+index], PREC2 ) ;
    	unit = g_model.frsky.channels[index].units3_4 ;
    	if (sub==subN)
			{
				Columns = 1 ;
				if ( (s_editMode || P1values.p1valdiff))
				{
    	    switch (subSub)
					{
    	    	case 0:
    	        g_model.frsky.channels[index].ratio3_4 = checkIncDec16( g_model.frsky.channels[index].ratio3_4, 0, 4800, EE_MODEL);
    	      break;
	  	      case 1:
    	        CHECK_INCDEC_H_MODELVAR_0( g_model.frsky.channels[index].units3_4, 3);
    	      break;
    	    }
				}
    	}
		}
    lcd_putsAttIdx(16*FW, y, XPSTR("\001v-VA"), unit, (sub==subN && subSub==1 ? blink:0));
   	subN++; y+=FH;

//    for (int j=0; j<2; j++)
//		{
//        uint8_t ag;
//        uint8_t al;
//        al = ALARM_LEVEL(i, j);
//        ag = ALARM_GREATER(i, j);
//        lcd_putsAtt(4, y, PSTR(STR_ALRM), 0);
//        lcd_putsAttIdx(6*FW, y, PSTR(STR_YELORGRED),al,(sub==subN && subSub==0 ? blink:0));
//        lcd_putsnAtt(11*FW, y, XPSTR("<>")+ag,1,(sub==subN && subSub==1 ? blink:0));
//        putsTelemValue(16*FW, y, g_model.frsky.channels[i].alarms_value[j], i, (sub==subN && subSub==2 ? blink:0)|NO_UNIT ) ;

//        if(sub==subN)
//				{
//					Columns = 2 ;
//					if ((s_editMode || P1values.p1valdiff))
//					{
//            uint8_t original ;
//            uint8_t value ;
//            switch (subSub)
//						{
//            	case 0:
//                value = checkIncDec( al, 0, 3, EE_MODEL) ;
//                original = g_model.frsky.channels[i].alarms_level ;
//                g_model.frsky.channels[i].alarms_level = j ? ( (original & 0xF3) | value << 2 ) : ( (original & 0xFC) | value ) ;
//              break;
//            	case 1:
//                value = checkIncDec( ag, 0, 3, EE_MODEL) ;
//                original = g_model.frsky.channels[i].alarms_greater ;
//                g_model.frsky.channels[i].alarms_greater = j ? ( (original & 0xFD) | value << 1 ) : ( (original & 0xFE) | value ) ;
//                if(checkIncDec_Ret)
//                    FRSKY_setModelAlarms();
//              break;
//            	case 2:
//                g_model.frsky.channels[i].alarms_value[j] = checkIncDec16( g_model.frsky.channels[i].alarms_value[j], 0, 255, EE_MODEL);
//              break;
//            }
//					}
//        }
//        subN++; y+=FH;
//    }
	}
	uint8_t attr = (sub==subN) ? InverseBlink : 0 ;
// 	lcd_puts_Pleft( y, XPSTR( "Rx Voltage") ) ;
//	lcd_outdezAtt( 16*FW, y, g_model.rxVratio, attr|PREC1 ) ;
	lcd_xlabel_decimal( 16*FW, y, g_model.rxVratio, attr|PREC1, XPSTR( "Rx Voltage") ) ;
  lcd_putc(Lcd_lastPos, y, 'v' ) ;
	lcd_outdezAtt( 21*FW, y, convertRxv( FrskyHubData[FR_RXV] ), PREC1 ) ;
	if( attr) { g_model.rxVratio = checkIncDec16( g_model.rxVratio, 0, 255, EE_MODEL ) ; }
	subN++; y+=FH;

	lcd_putsAtt( 0, y, XPSTR("Cell Scaling"), sub==subN ? INVERS : 0 ) ;
	if ( sub == subN )
	{
		if ( checkForMenuEncoderLong( event ) )
		{
    	pushMenu( menuCellScaling ) ;
		}
	}
 }
#if defined(PCBSKY) || defined(PCB9XT)
 else if ( sub < 9+page2SizeExtra) // sub>=7
#else
 else if ( sub < 9+page2SizeExtra) // sub>=7
#endif
 {
	uint8_t subN = 7 ;
	uint8_t b ;

	if (sub==subN)
	{
		Columns = 1 ;
	}
 	lcd_puts_Pleft( y, XPSTR( "RSSI Warn""\037""RSSI Critical") ) ;
	uint8_t attr = ( ( (sub==subN) && (subSub==0) ) ? InverseBlink : 0) ;
	int8_t offset ;
	offset = rssiOffsetValue( 0 ) ;
	lcd_outdezAtt( 15*FW, y, g_model.rssiOrange + offset, attr ) ;
  if( attr) CHECK_INCDEC_H_MODELVAR( g_model.rssiOrange, -18, 30 ) ;
	attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
	b = 1-g_model.enRssiOrange ;
	menu_lcd_onoff( PARAM_OFS+1, y, b, attr ) ;
  if( attr) { CHECK_INCDEC_H_MODELVAR_0( b, 1 ) ; g_model.enRssiOrange = 1-b ; }
	subN++; y+=FH;
		
	if (sub==subN)
	{
		Columns = 1 ;
	}
	attr = ( ( (sub==subN) && (subSub==0) ) ? InverseBlink : 0) ;
	offset = rssiOffsetValue( 1 ) ;
	lcd_outdezAtt( 15*FW, y, g_model.rssiRed + offset, attr ) ;
	if( attr) CHECK_INCDEC_H_MODELVAR( g_model.rssiRed, -17, 30 ) ;
	attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
	b = 1-g_model.enRssiRed ;
	menu_lcd_onoff( PARAM_OFS+1, y, b, attr ) ;
  if( attr) { CHECK_INCDEC_H_MODELVAR_0( b, 1 ) ; g_model.enRssiRed = 1-b ; }

	subN++; y+=FH;
 
	if ( page2SizeExtra )
	{
 		lcd_puts_Pleft( y, XPSTR( "DSM 'A' as RSSI") ) ;
 		menu_lcd_onoff( PARAM_OFS, y, g_model.dsmAasRssi, sub==subN ) ;
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.dsmAasRssi, 1);
		subN++; y+=FH;
		
		if (sub==subN)
		{
			Columns = 1 ;
		}
 		lcd_puts_Pleft( y, XPSTR( "DSM Warning") ) ;
		
		g_model.dsmLinkData.sourceWarn = checkIndexed( y, XPSTR(FWx13"\002""\006fades lossesholds "), g_model.dsmLinkData.sourceWarn, (sub==subN) && (subSub==0) ) ;
		attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
		lcd_outdezAtt( 21*FW, y, g_model.dsmLinkData.levelWarn, attr ) ;
  	if( attr) { CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.levelWarn, 40 ) ; }
		subN++; y+=FH;

		if (sub==subN)
		{
			Columns = 1 ;
		}
 		lcd_puts_Pleft( y, XPSTR( "DSM Critical") ) ;
		g_model.dsmLinkData.sourceCritical = checkIndexed( y, XPSTR(FWx13"\002""\006fades lossesholds "), g_model.dsmLinkData.sourceCritical, (sub==subN) && (subSub==0) ) ;
		attr = ( ( (sub==subN) && (subSub==1) ) ? InverseBlink : 0) ;
		lcd_outdezAtt( 21*FW, y, g_model.dsmLinkData.levelCritical, attr ) ;
  	if( attr) { CHECK_INCDEC_H_MODELVAR_0( g_model.dsmLinkData.levelCritical, 40 ) ; }
		subN++; y+=FH;

 		lcd_puts_Pleft( y, XPSTR( "DSM Vario") ) ;
		g_model.dsmVario = checkIndexed( y, XPSTR(FWx13"\005""\0040.250.5 1.0 1.5 2.0 3.0 "), g_model.dsmVario, (sub==subN) ) ;
		subN++; y+=FH;
	}





//	else
//	{
//		if ( (sub==subN) || (sub==subN+1))
//		{
//			if ( M_lastVerticalPosition	< mstate2.m_posVert )
//			{
//				mstate2.m_posVert = 12-4 ;
//			}
//			else if ( M_lastVerticalPosition > mstate2.m_posVert )
//			{
//				mstate2.m_posVert = 9-4 ;
//			}
//		}
//	}

 }
#if defined(PCBSKY) || defined(PCB9XT)
 else if ( sub < 14+page2SizeExtra) // sub>=8
#else
 else if ( sub < 14+page2SizeExtra) // sub>=8
#endif
	{
#if defined(PCBSKY) || defined(PCB9XT)
		uint8_t subN = 9+page2SizeExtra ;
#else
		uint8_t subN = 9+page2SizeExtra ;
#endif
		y = FH ;
		
		uint8_t attr = 0 ;
  	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR( g_model.numBlades, 1, 127 ) ; }
		lcd_xlabel_decimal( 14*FW, y, g_model.numBlades, attr, PSTR(STR_NUM_BLADES) ) ;
		y += FH ;
		subN++;

		attr = 0 ;
  	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.FrSkyAltAlarm, 2 ) ; }
		lcd_puts_Pleft( y, PSTR(STR_ALT_ALARM));
  	lcd_putsAttIdx(11*FW, y, PSTR(STR_OFF122400),g_model.FrSkyAltAlarm, attr ) ;
  	y += FH ;
  	subN++;
  
		lcd_puts_Pleft( y, PSTR(STR_VOLT_THRES));
  	lcd_outdezNAtt(  14*FW, y, g_model.frSkyVoltThreshold * 2 ,((sub==subN) ? blink:0) | PREC2, 4);
  	if(sub==subN)
		{
  	  g_model.frSkyVoltThreshold=checkIncDec16( g_model.frSkyVoltThreshold, 0, 210, EE_MODEL);
  	}
  	y += FH ;
		subN++;
	
  	lcd_puts_Pleft( y, PSTR(STR_GPS_ALTMAIN) ) ;
  	menu_lcd_onoff( PARAM_OFS, y, g_model.FrSkyGpsAlt, sub==subN ) ;
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.FrSkyGpsAlt, 1);
  	y += FH ;
		subN++;
		
		if (sub==subN)
		{
			Columns = 1 ;
		}
		uint16_t value = g_model.frskyAlarms.alarmData[0].frskyAlarmLimit << 6 ;
		lcd_puts_Pleft(y, PSTR(STR_MAH_ALARM));
  	attr = ((sub==subN && subSub==0) ? InverseBlink : 0);
		uint8_t active = (attr && s_editMode) ;
  	lcd_outdezAtt( 14*FW, y, value, attr ) ;
		if ( active )
		{
  		g_model.frskyAlarms.alarmData[0].frskyAlarmLimit = checkIncDec16( g_model.frskyAlarms.alarmData[0].frskyAlarmLimit, 0, 200, EE_MODEL);
		}
  	attr = ((sub==subN && subSub==1) ? InverseBlink : 0);
		active = (attr && s_editMode) ;
		lcd_putsAttIdx(15*FW, y, PSTR(STR_SOUNDS), g_model.frskyAlarms.alarmData[0].frskyAlarmSound,attr);
		if ( active )
		{
  		CHECK_INCDEC_H_MODELVAR_0( g_model.frskyAlarms.alarmData[0].frskyAlarmSound, 15 ) ;
		}
  	subN++;


	}
	else if ( sub <= 20 + page2SizeExtra )
	{
#if defined(PCBSKY) || defined(PCB9XT)
		uint8_t subN = 14+page2SizeExtra ;
#else
		uint8_t subN = 14+page2SizeExtra ;
#endif
		y = FH ;
  	lcd_puts_Pleft( y, PSTR(STR_BT_TELEMETRY) );
#ifdef REVX
		g_model.bt_telemetry = checkIndexed( y, XPSTR(FWx15"\002""\005  Off   RxTx/Rx"), g_model.bt_telemetry, (sub==subN) ) ;
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.bt_telemetry, 2);
#else
  	menu_lcd_onoff( PARAM_OFS, y, g_model.bt_telemetry, sub==subN ) ;
  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.bt_telemetry, 1);
#endif
  	y += FH ;
		subN += 1 ;
  	
		lcd_puts_Pleft( y, PSTR(STR_FRSKY_COM_PORT) );
    uint8_t attr = (sub == subN) ? blink : 0 ;
  	lcd_putcAtt( 17*FW, y, g_model.frskyComPort + '1', attr ) ;
		if (attr) CHECK_INCDEC_H_MODELVAR_0( g_model.frskyComPort, 1 ) ;
	  y += FH ;
		subN += 1 ;

#ifdef REVX
		uint8_t previous = g_model.telemetryRxInvert ;
		previous |= g_model.telemetry2RxInvert << 1 ;
		uint8_t next = previous ;

		lcd_puts_Pleft( y, XPSTR("Com Port Invert") );
		attr = 0 ;
		if(sub==subN)
		{
			attr = InverseBlink ;
			CHECK_INCDEC_H_MODELVAR_0( next, 3 ) ;
		}
		lcd_putsAttIdx(17*FW, y, XPSTR("\004None   1   2BOTH"), next, attr ) ;
		if ( next != previous )
		{
			g_model.telemetryRxInvert = next & 0x01 ;
			g_model.telemetry2RxInvert = (next >> 1 ) & 0x01 ;
			TelemetryType = TEL_UNKNOWN ;
		}
//		uint8_t previous = g_model.telemetryRxInvert ;
//		lcd_puts_Pleft( y, PSTR(STR_INVERT_COM1) );
//  	menu_lcd_onoff( PARAM_OFS, y, g_model.telemetryRxInvert, sub==subN ) ;
//  	if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.telemetryRxInvert, 1);
//		if ( g_model.telemetryRxInvert != previous )
//		{
//			if ( g_model.telemetryRxInvert )
//			{
//				setMFP() ;
//			}
//			else
//			{
//				clearMFP() ;
//			}
//  	}
#else
		uint8_t previous = g_model.telemetryRxInvert ;
		previous |= g_model.telemetry2RxInvert << 1 ;
		uint8_t next = previous ;

		lcd_puts_Pleft( y, XPSTR("Com Port Invert") );
		attr = 0 ;
		if(sub==subN)
		{
			attr = InverseBlink ;
#ifdef PCBSKY
			CHECK_INCDEC_H_MODELVAR_0( next, 2 ) ;
#else
			CHECK_INCDEC_H_MODELVAR_0( next, 1 ) ;
#endif
		}
		lcd_putsAttIdx(17*FW, y, XPSTR("\004None   1   2"), next, attr ) ;

		if ( next != previous )
		{
			g_model.telemetryRxInvert = next & 0x01 ;
			g_model.telemetry2RxInvert = (next >> 1 ) & 0x01 ;
			TelemetryType = TEL_UNKNOWN ;
		}
#endif
	  y += FH ;
		subN += 1 ;
		 
		lcd_puts_Pleft( y, PSTR(STR_FAS_OFFSET) );
    attr = PREC1 ;
		if ( (sub == subN) )
		{
			attr = blink | PREC1 ;
      CHECK_INCDEC_H_MODELVAR_0( g_model.FASoffset, 15 ) ;
		}
  	lcd_outdezAtt( 15*FW, y, g_model.FASoffset, attr ) ;
	  y += FH ;
		subN += 1 ;

#if not (defined(PCBX10))
		lcd_puts_Pleft( y, XPSTR("COM2 Func.") );
		uint8_t b = g_model.com2Function ;
    attr = 0 ;
		if ( (sub == subN) )
		{
			attr = blink ;
			b = checkOutOfOrder( b, (uint8_t *)Com2Options, MAX_COM2_OPTIONS ) ;
		}
//		lcd_putsAttIdx(12*FW, y, XPSTR("\011TelemetrySbusTrainSbus57600BTdirect CppmTrainLCDdump  Tel+BTdirScript   "), g_model.com2Function, attr ) ;

//#ifdef PCBSKY
//	  	CHECK_INCDEC_H_MODELVAR_0( g_model.com2Function, 6 ) ;
//#endif
//#ifdef PCBX9D
//	  	CHECK_INCDEC_H_MODELVAR_0( g_model.com2Function, 4 ) ;
//#endif
//#ifdef PCBX12D
//	  	CHECK_INCDEC_H_MODELVAR_0( g_model.com2Function, 4 ) ;	// No LCD DUmp
//#endif
//#ifdef PCB9XT
//	  	CHECK_INCDEC_H_MODELVAR_0( g_model.com2Function, 2 ) ;
//#endif
//		}
#ifdef PCBSKY
		lcd_putsAttIdx(12*FW, y, XPSTR("\011TelemetrySbusTrainSbus57600BTdirect Unused   LCDdump  Tel+BTdirScript   "), g_model.com2Function, attr ) ;
#endif
#ifdef PCBX9D
		lcd_putsAttIdx(12*FW, y, XPSTR("\011TelemetrySbusTrainSbus57600CppmTrainLCDdump  Unused   Unused   Script   BT/Enc   "), g_model.com2Function, attr ) ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		lcd_putsAttIdx(12*FW, y, XPSTR("\011TelemetrySbusTrainSbus57600BTdirect CppmTrain"), g_model.com2Function, attr ) ;
#endif
#ifdef PCB9XT
		lcd_putsAttIdx(12*FW, y, XPSTR("\011TelemetrySbusTrainSbus57600Unused   Unused   Unused   Unused   Script   "), g_model.com2Function, attr ) ;
#endif
		if ( g_model.com2Function != b )
		{
			g_model.com2Function = b ;
			
//#if defined(PCBX9D) || defined(PCBX12D)
#if defined(PCBX9D)
			if ( b == 3 )
			{
				stop_serial_trainer_capture() ;
			}
#endif
			com2Configure() ;
			if( g_model.com2Function == COM2_FUNC_TELEMETRY )
			{
				if ( g_model.frskyComPort == 1 )
				{
					telemetry_init( TEL_FRSKY_HUB ) ;
				}
			}
		}
	  y += FH ;
		subN += 1 ;
		
		lcd_puts_Pleft( y, XPSTR("COM2 Baudrate") );
		b = g_model.com2Baudrate ;

		g_model.com2Baudrate = checkIndexed( y, XPSTR(BaudString), g_model.com2Baudrate, (sub==subN) ) ;
#endif

#ifdef PCBSKY
		if ( b != g_model.com2Baudrate )
		{
			if ( ( g_model.com2Function == COM2_FUNC_BTDIRECT )
					 || ( g_model.com2Function == COM2_FUNC_TEL_BT2WAY ) )
			{
				if ( g_model.com2Baudrate )
				{
					com2_Configure( g_model.com2Baudrate-1, SERIAL_NORM, SERIAL_NO_PARITY ) ;
				}
			}
		}
#endif	// PCBSKY
	  y += FH ;
		subN += 1 ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
		lcd_puts_Pleft( y, XPSTR("COM1 Baudrate") );
		b = g_model.telemetryBaudrate ;
		g_model.telemetryBaudrate = checkIndexed( y, XPSTR(BaudString), g_model.telemetryBaudrate, (sub==subN) ) ;
	  y += FH ;
		subN += 1 ;
#endif	// PCBSKY

	}
	else
	{
		uint8_t subN = TDATAITEMS + page2SizeExtra-1 ;
		// Vario
   	for( CPU_UINT j=0 ; j<7 ; j += 1 )
		{
//			uint8_t b ;
      uint8_t attr = (sub==subN) ? blink : 0 ;
			uint8_t y = (1+j)*FH ;

			switch ( j )
			{
				case 0 :
//					lcd_puts_Pleft( y, PSTR(STR_VARIO_SRC) ) ;
//					lcd_putsAttIdx( 15*FW, y, PSTR(STR_VSPD_A2), g_model.varioData.varioSource, attr ) ;
//   		  	if(attr)
//					{
//						CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.varioSource, 2+NUM_SCALERS ) ;
//   		  	}
//				break ;
				
//				case 1 :
//					lcd_puts_Pleft( y, PSTR(STR_2SWITCH) ) ;
//					g_model.varioData.swtch = edit_dr_switch( 15*FW, y, g_model.varioData.swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
//				break ;

//				case 2 :
//					lcd_puts_Pleft( y, PSTR(STR_2SENSITIVITY) ) ;
// 					lcd_outdezAtt( 17*FW, y, g_model.varioData.param, attr) ;
//   			  if(attr)
//					{
//						CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.param, 50 ) ;
//	   		  }
//				break ;

//				case 3 :
//					lcd_puts_Pleft( y, XPSTR("Base Freq.") ) ;
// 					lcd_outdezAtt( 17*FW, y, g_model.varioExtraData.baseFrequency, attr) ;
//   			  if(attr)
//					{
//						CHECK_INCDEC_H_MODELVAR( g_model.varioExtraData.baseFrequency, -50, 50 ) ;
//	   		  }
//				break ;

//				case 4 :
//					lcd_puts_Pleft( y, XPSTR("Offset Freq.") ) ;
// 					lcd_outdezAtt( 17*FW, y, g_model.varioExtraData.offsetFrequency, attr) ;
//   			  if(attr)
//					{
//						CHECK_INCDEC_H_MODELVAR( g_model.varioExtraData.offsetFrequency, -20, 20 ) ;
//	   		  }
//				break ;

//				case 5 :
//	        b = g_model.varioData.sinkTones ;
//					g_model.varioData.sinkTones = offonMenuItem( b, y, PSTR(STR_SINK_TONES), attr ) ;
//				break ;

//				case 6 :
					lcd_puts_Pleft( y, XPSTR("Current Source" ) ) ;
					lcd_putsAttIdx( 15*FW, y, XPSTR("\004----A1  A2  Fas SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "), g_model.currentSource, attr ) ;
   			  if(attr)
					{
						CHECK_INCDEC_H_MODELVAR_0( g_model.currentSource, 3 + NUM_SCALERS ) ;
	   		  }
				break ;

			}

			subN += 1 ;
		}
	}

}

void menuCellScaling(uint8_t event)
{
	static MState2 mstate2 ;
	mstate2.check_columns(event, 12-1 ) ;	
  lcd_puts_Pleft( 0, XPSTR( "Cell Scaling" ) ) ;
	uint8_t y = 0 ;
	uint8_t k ;
	uint8_t t_pgOfs ;
	uint32_t index ;
  int8_t sub = mstate2.m_posVert ;
//#if defined(PCBX12D) || defined(PCBX10)
//  t_pgOfs = evalOffsetLarge(sub);
//#else
	t_pgOfs = evalOffset( sub ) ;
//#endif
	for(uint8_t i=0; i<SCREEN_LINES-1; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    uint8_t attr ;
  	lcd_puts_Pleft( y, XPSTR( "Cell\023v" ) ) ;
		lcd_outdez(  6*FW, y, k+1 ) ;
		index = k + FR_CELL1 ;
		attr = TelemetryDataValid[index] ? 0 : BLINK ;
 		lcd_outdezAtt(  19*FW-2, y, FrskyHubData[index], attr | PREC2 ) ;
    attr = ((sub==k) ? InverseBlink : 0);
		uint32_t scaling = 1000 + g_model.cellScalers[k] ;
 		lcd_outdezAtt(  12*FW, y, scaling/10, attr | PREC2 ) ;
 		lcd_outdezAtt(  13*FW-1, y, scaling%10, attr ) ;
    if( attr )
		{
    	CHECK_INCDEC_H_MODELVAR( g_model.cellScalers[k], -50, 50 ) ;
		}
	}
}



extern uint8_t frskyRSSIlevel[2] ;
extern uint8_t frskyRSSItype[2] ;

void menuSetFailsafe(uint8_t event)
{
	static MState2 mstate2 ;
#ifdef ACCESS
	mstate2.check_columns(event, 24-1+1+1+1 ) ;	
#else
	mstate2.check_columns(event, 16-1+1+1+1 ) ;	
#endif
  lcd_puts_Pleft( 0, XPSTR( "Set Failsafe" ) ) ;
  int8_t sub = mstate2.m_posVert ;
	uint8_t y = 0;
	uint8_t k = 0;
	uint8_t t_pgOfs ;
	int32_t value ;
//#if defined(PCBX12D) || defined(PCBX10)
//  t_pgOfs = evalOffsetLarge(sub);
//#else
	t_pgOfs = evalOffset( sub ) ;
//#endif
	uint32_t module = s_currIdx ? 1 : 0 ;
	
	StickScrollAllowed = 0 ;		// Block while editing

	if ( event == EVT_ENTRY )
	{
		StatusTimer = 0 ;
	}
	if ( StatusTimer )
	{
		StatusTimer -= 1 ;
	}

//	switch(event)
//	{
//    case EVT_KEY_LONG(KEY_MENU):
//			FailsafeCounter[module] = 5 ;		// Send failsafe values soon
//      killEvents(event);
//  	  s_editMode = 0 ;
//		break ;
//	}		
	for(uint8_t i=0; i<SCREEN_LINES-1; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    uint8_t attr = ((sub==k) ? InverseBlink : 0);
		uint8_t active = (attr && s_editMode ) ;

		if ( k == 0 )
		{
		  lcd_puts_Pleft( y, XPSTR( "Mode" ) ) ;
			EditColumns = 1 ;
			g_model.Module[module].failsafeMode = checkIndexed( y, XPSTR(FWx9"\004""\007Not Set     Rx Custom   HoldNoPulse"), g_model.Module[module].failsafeMode, attr ) ;
			EditColumns = 0 ;
		}
		else if ( k == 1 )
		{
			if ( attr )
			{
    		if ( checkForMenuEncoderLong( event ) )
				{
					FailsafeCounter[module] = 6 ;		// Send failsafe values soon
					StatusTimer = 50 ;
				}
			}
		  lcd_putsAtt( 0, y, XPSTR( "Send Now" ), StatusTimer ? 0 : attr ) ;
		}
		else if ( k == 2 )
//		{
//			uint8_t b ;
//			b = g_model.Module[module].failsafeRepeat ;
//     	g_model.Module[module].failsafeRepeat = offonMenuItem( b, y, XPSTR("Repeat Send"), sub == k ) ;
//			if ( b != g_model.Module[module].failsafeRepeat )
//			{
//				if ( b == 0 )
//				{
//					FailsafeCounter[module] = 6 ;		// Send failsafe values soon
//				}
//				else
//				{
//					FailsafeCounter[module] = 0 ;		// Stop sending
//				}
//			}
//		}
//		else if ( k == 3 )
		{
			if ( attr )
			{
    		if ( checkForMenuEncoderLong( event ) )
				{
					uint32_t j ;
#ifdef ACCESS
					for ( j = 0 ; j < 24 ; j += 1 )
#else
					for ( j = 0 ; j < 16 ; j += 1 )
#endif
					{
						value = g_chans512[j] * 100 / RESX ;
  					if(value > 125)
						{
							value = 125 ;
						}	
  					if(value < -125 )
						{
							value = -125 ;
						}	
#ifdef ACCESS
						if ( j >= 16 )
						{
							g_model.accessFailsafe[module][j-16] = value ;
						}
						else
						{
							g_model.Module[module].failsafe[j] = value ;
						}
#else
						g_model.Module[module].failsafe[j] = value ;
#endif
					}
					StatusTimer = 50 ;
		    	eeDirty(EE_MODEL) ;
				}
			}
		  lcd_putsAtt( 0, y, XPSTR( "Use Actual" ), StatusTimer ? 0 : attr ) ;
		}
		else
		{
			putsChn(0,y,k-2,0);
#ifdef ACCESS
			int8_t *pValue ;
			pValue = ( (k-3) < 16 ) ? &g_model.Module[module].failsafe[k-3] : &g_model.accessFailsafe[module][k-3-16] ;
			value = *pValue ;
			lcd_outdezAtt(  7*FW+3, y, value, attr ) ;
			lcd_putc(  7*FW+5, y, '%' ) ;
    	if(active)
			{
  	    *pValue = checkIncDec16( value, -125, 125, EE_MODEL ) ;
    	}
#else  	  
			value = g_model.Module[module].failsafe[k-3] ;
			lcd_outdezAtt(  7*FW+3, y, value, attr ) ;
			lcd_putc(  7*FW+5, y, '%' ) ;
    	if(active)
			{
  	    g_model.Module[module].failsafe[k-3] = checkIncDec16( value, -125, 125, EE_MODEL ) ;
    	}
#endif
		}
	}
}

void menuCustomCheck(uint8_t event)
{
	static MState2 mstate2 ;
	mstate2.check_columns(event, 3-1 ) ;
  lcd_puts_Pleft( 0, PSTR( STR_CUSTOM_CHECK ) ) ;
	
  int8_t sub = mstate2.m_posVert;
	
	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	CustomCheckData *pdata ;
	pdata = &g_model.customCheck ;

	lcd_puts_Pleft( FH, XPSTR("Source\037Min.\037Max.") ) ;
	for (uint8_t k = 0 ; k < 3 ; k += 1 )
	{
    uint8_t y = (k+1) * FH ;
    uint8_t i = k ;
		uint8_t attr = (sub==i ? InverseBlink : 0);
		switch(i)
		{
      case 0 :	// Source
			{	
				uint8_t x = unmapPots( pdata->source ) ;
				
//				lcd_puts_Pleft( y, XPSTR("Source") ) ;
				putsChnRaw( 11*FW, y, x, attr ) ;
				if( attr ) CHECK_INCDEC_H_MODELVAR_0( pdata->source, 7 + NumExtraPots ) ; // NUM_SKYXCHNRAW+NUM_TELEM_ITEMS ) ;
			}
			break ;
			case 1 :
//				lcd_puts_Pleft( y, XPSTR("Min.") ) ;
				lcd_outdezAtt( 13*FW, y, pdata->min, attr) ;
				if( attr ) CHECK_INCDEC_H_MODELVAR( pdata->min, -125, 125 ) ;
			break ;
      case 2 :	// offset
//				lcd_puts_Pleft( y, XPSTR("Max.") ) ;
				lcd_outdezAtt( 13*FW, y, pdata->max, attr) ;
				if( attr ) CHECK_INCDEC_H_MODELVAR( pdata->max, -125, 125 ) ;
			break ;
		}
	}
}



uint16_t scalerDecimal( uint8_t y, uint16_t val, uint8_t attr )
{
  lcd_outdezAtt( 13*FW, y, val+1, attr ) ;
	if (attr) val = checkIncDec16( val, 0, 2047, EE_MODEL);
	return val ;
}

static uint8_t s_scalerSource ;

void menuScaleOne(uint8_t event)
{
	static MState2 mstate2 ;
	mstate2.check_columns(event, 13-1 ) ;
  lcd_puts_Pleft( 0, XPSTR("SC  =") ) ;
	uint8_t index = s_currIdx ;
  lcd_putc( 2*FW, 0, index+'1' ) ;
	
  int8_t sub = mstate2.m_posVert;
//#if defined(PCBX12D) || defined(PCBX10)
//  evalOffsetLarge(sub);
//#else
	evalOffset( sub ) ;
//#endif
	
	putsTelemetryChannel( 8*FW, 0, index+TEL_ITEM_SC1, 0, 0, TELEM_UNIT ) ;

	ScaleData *pscaler ;
	ExtScaleData *epscaler ;
	pscaler = &g_model.Scalers[index] ;
	epscaler = &g_model.eScalers[index] ;
	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	else if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		if ( TextResult )
		{
			if ( s_scalerSource )
			{
				epscaler->exSource = unmapPots( TextIndex ) ;
			}
			else
			{
				pscaler->source = unmapPots( TextIndex ) ;
			}
	    eeDirty(EE_MODEL) ;
		}
	}
  
	for (uint8_t k = 0 ; k < SCREEN_LINES-1 ; k += 1 )
	{
		uint16_t t ;
    uint8_t y = (k+1) * FH ;
    uint8_t i = k + s_pgOfs;
		uint8_t attr = (sub==i ? InverseBlink : 0);
		switch(i)
		{
      case 0 :	// Source
			case 7 :	// exSource
			{	
				uint8_t x ;
				
				if ( i == 0 )
				{				
					lcd_puts_Pleft( y, XPSTR("Source") ) ;
					x = pscaler->source ;
					s_scalerSource = 0 ;
				}
				else
				{
					lcd_puts_Pleft( y, XPSTR("ex Source") ) ;
					x = epscaler->exSource ;
					s_scalerSource = 1 ;
				}
				putsChnRaw( 11*FW, y, x, attr ) ;
				if( attr )
				{
					x = mapPots( x ) ;
					CHECK_INCDEC_H_MODELVAR( x, 0, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots ) ;
					if ( i == 0 )
					{
						pscaler->source = unmapPots( x ) ;
					}
					else
					{
						epscaler->exSource = unmapPots( x ) ;
					}
					if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
					{
						// Long MENU pressed
						TextIndex = mapPots( x ) ;
  				  TextType = 1 ;
  				  killEvents(event) ;
						pushMenu(menuTextHelp) ;
					}
				}
			}
			break ;
			case 1 :	// name
				alphaEditName( 11*FW-2, y, (uint8_t *)pscaler->name, sizeof(pscaler->name), attr, (uint8_t *)XPSTR( "Scaler Name") ) ;
			break ;
      case 2 :	// offset
//				lcd_puts_Pleft( y, PSTR(STR_OFFSET) ) ;
//				lcd_outdezAtt( 13*FW, y, pscaler->offset, attr) ;
				lcd_xlabel_decimal( 13*FW, y, pscaler->offset, attr, PSTR(STR_OFFSET) ) ;
				if ( attr )
				{
					StepSize = 100 ;
					pscaler->offset = checkIncDec16( pscaler->offset, -32000, 32000, EE_MODEL ) ;
				}
			break ;
      case 3 :	// mult
				lcd_puts_Pleft( y, XPSTR("Multiplier") ) ;
				t = pscaler->mult + ( pscaler->multx << 8 ) ;
				t = scalerDecimal( y, t, attr ) ;
				pscaler->mult = t ;
				pscaler->multx = t >> 8 ;
			break ;
      case 4 :	// div
				lcd_puts_Pleft( y, XPSTR("Divisor") ) ;
				t = pscaler->div + ( pscaler->divx << 8 ) ;
				t = scalerDecimal( y, t, attr ) ;
				pscaler->div = t ;
				pscaler->divx = t >> 8 ;
			break ;
      case 5 :	// mod
				lcd_puts_Pleft( y, XPSTR("Mod Value") ) ;
				epscaler->mod = scalerDecimal( y, epscaler->mod, attr ) ;
			break ;
      case 6 :	// offsetLast
				lcd_puts_Pleft( y, XPSTR("Offset At") ) ;
				lcd_putsAttIdx( 11*FW, y, XPSTR("\005FirstLast "), pscaler->offsetLast, attr ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->offsetLast, 1 ) ;
			break ;
			case 8 :
				lcd_puts_Pleft( y, XPSTR("Function") ) ;
				pscaler->exFunction = checkIndexed( y, XPSTR(FWx13"\006""\010--------Add     SubtractMultiplyDivide  Mod     Min     "), pscaler->exFunction, attr ) ;
			break ;
			case 9 :	// unit
				lcd_puts_Pleft( y, XPSTR("Unit") ) ;
				lcd_putsAttIdx( 11*FW, y, XPSTR(UnitsString), pscaler->unit, attr ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->unit, 10 ) ;
			break ;
      case 10 :	// sign
				lcd_puts_Pleft( y, XPSTR("Sign") ) ;
  			lcd_putcAtt( 11*FW, y, pscaler->neg ? '-' : '+', attr ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->neg, 1 ) ;
			break ;
      case 11 :	// precision
				lcd_puts_Pleft( y, XPSTR("Decimals") ) ;
				lcd_outdezAtt( 13*FW, y, pscaler->precision, attr) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( pscaler->precision, 2 ) ;
			break ;
      case 12 :	// Dest
				lcd_puts_Pleft( y, XPSTR("Dest") ) ;
#define NUM_SCALE_DESTS		13
				if( attr )
				{
					CHECK_INCDEC_H_MODELVAR( epscaler->dest, 0, NUM_SCALE_DESTS ) ;
				}
				lcd_putsAttIdx( 11*FW, y, XPSTR(DestString), epscaler->dest, attr ) ;
			break ;
		}
	}

}

void menuProcAdjust(uint8_t event)
{
	TITLE(XPSTR("GVAR Adjust"));
	EditType = EE_MODEL ;
	static MState2 mstate2;
 	event = mstate2.check_columns(event, NUM_GVAR_ADJUST + EXTRA_GVAR_ADJUST - 1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
	uint8_t y = FH ;
	
  uint8_t t_pgOfs ;
//#if defined(PCBX12D) || defined(PCBX10)
//  t_pgOfs = evalOffsetLarge(sub);
//#else
	t_pgOfs = evalOffset( sub ) ;
//#endif
  uint8_t k ;
	Columns = 3 ;
	
	for (uint8_t i=0; i<SCREEN_LINES-1; i++ )
	{
		GvarAdjust *pgvaradj ;
    y=(i+1)*FH;
    k=i+t_pgOfs;
		pgvaradj = ( k >= NUM_GVAR_ADJUST ) ? &g_model.egvarAdjuster[k - NUM_GVAR_ADJUST] : &g_model.gvarAdjuster[k] ;
  	lcd_putc( 0, y, 'A' ) ;
  	lcd_puts_P( 3*FW+2, y, XPSTR("GV") ) ;
		if ( k < 9 )
		{
  		lcd_putc( 1*FW, y, k+'1' ) ;
		}
		else
		{
  		lcd_putc( 1*FW, y, k > 18 ? '2' : '1' ) ;
  		lcd_putc( 2*FW, y, ( (k+1) % 10) +'0' ) ;
		}

		if ( sub==k )
		{
  		lcd_puts_Pleft( 0, XPSTR("\015GV =") ) ;
  		lcd_putc( 15*FW, 0, pgvaradj->gvarIndex+'1' ) ;
			lcd_outdez( 20*FW, 0, g_model.gvars[pgvaradj->gvarIndex].gvar) ;
		} 
		for( uint32_t j = 0 ; j < 4 ; j += 1 )
		{
      uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
			uint8_t active = (attr && (s_editMode) ) ;
			if ( j == 0 )
			{
				lcd_putcAtt( 5*FW+2, y, pgvaradj->gvarIndex+'1', attr ) ;
				if ( active )
				{
 	        CHECK_INCDEC_H_MODELVAR_0( pgvaradj->gvarIndex, 6 ) ;
				}
			}
			else if ( j == 1 )
			{
				uint32_t old = pgvaradj->function ;
				pgvaradj->function = checkIndexed( y, XPSTR(FWx7"\010""\005-----Add  Set CSet V+/-  Inc/0Dec/0+/Lim-/Lim"), pgvaradj->function, attr ) ;
				if ( pgvaradj->function != old )
				{
					if ( ( old < 4 ) || ( old > 6 ) )
					{
						if ( pgvaradj->function >= 4 )
						{
							pgvaradj->switch_value = 0 ;
						}
					}
					else
					{
						if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
						{
							pgvaradj->switch_value = 0 ;
						}
					}
				}
			}
			else if ( j == 2 )
			{
				pgvaradj->swtch = edit_dr_switch( 13*FW-1, y, pgvaradj->swtch, attr, active ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			}
			else
			{
				if ( ( pgvaradj->function < 4 ) || ( pgvaradj->function > 6 ) )
				{
					if ( pgvaradj->function == 3 )
					{
						lcd_putsAttIdx( 18*FW, y, PSTR(STR_GV_SOURCE), pgvaradj->switch_value, attr ) ;
#if defined(PCBSKY) || defined(PCB9XT)
		  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgvaradj->switch_value, 68 ) ;
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
		  			if(active) CHECK_INCDEC_H_MODELVAR_0( pgvaradj->switch_value, 69 ) ;
#endif
					}
					else
					{
						lcd_outdezAtt( 21*FW, y, pgvaradj->switch_value, attr ) ;
						if ( active )
						{
 	      	  	CHECK_INCDEC_H_MODELVAR( pgvaradj->switch_value, -125, 125 ) ;
						}
					}
				}
				else
				{
					pgvaradj->switch_value = edit_dr_switch( 17*FW, y, pgvaradj->switch_value, attr, active ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				}
			}
		}
	}
}

void menuProcScalers(uint8_t event)
{
	TITLE(XPSTR("Scalers"));
	EditType = EE_MODEL ;
	static MState2 mstate2;
 	event = mstate2.check_columns(event, NUM_SCALERS-1 ) ;
	
	uint8_t sub = mstate2.m_posVert ;
	uint8_t y = FH ;
	
#if defined(PCBX12D) || defined(PCBX10)
  uint8_t t_pgOfs = 0 ;
#else
  uint8_t t_pgOfs = (sub == 7 ) ? 1 : 0 ;
#endif
  uint8_t k ;
	
  switch (event)
	{
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE) :
        s_currIdx = sub ;
        killEvents(event);
        pushMenu(menuScaleOne) ;
		break;
  }
	
#if defined(PCBX12D) || defined(PCBX10)
	for (uint8_t i=0; i<8; i++ )
#else
	for (uint8_t i=0; i<7; i++ )
#endif
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
  	lcd_puts_Pleft( y, XPSTR("SC\011+\015*\022/") ) ;
  	lcd_putc( 2*FW, y, k+'1' ) ;
		putsChnRaw( 4*FW, y, g_model.Scalers[k].source, 0 ) ;
		lcd_outdezAtt( 12*FW+3, y, g_model.Scalers[k].offset, 0) ;
		lcd_outdezAtt( 16*FW, y, g_model.Scalers[k].mult+1, 0) ;
		lcd_outdezAtt( 21*FW, y, g_model.Scalers[k].div+1, 0) ;
	}
	lcd_char_inverse( 0, (sub-t_pgOfs+1)*FH, 126, 0 ) ;
}

void menuProcGlobals(uint8_t event)
{
	TITLE(PSTR(STR_GLOBAL_VARS));
	EditType = EE_MODEL ;
	static MState2 mstate2;
	
	event = mstate2.check_columns(event, MAX_GVARS - 1 ) ;

	uint8_t subN = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
	uint8_t y = FH ;
	
 if ( subN < MAX_GVARS )
 {
	Columns = 2 ;
	for (CPU_UINT i=0; i<MAX_GVARS; i++ )
	{
    lcd_puts_Pleft(y, PSTR(STR_GV));
		lcd_putc( 2*FW, y, i+'1') ;
		for(uint8_t j=0; j<3;j++)
		{
      uint8_t attr = ((subN==i && subSub==j) ? InverseBlink : 0);
			uint8_t active = (attr && (s_editMode) ) ;
			GvarData *pgvar ;
			pgvar = &g_model.gvars[i] ;
      if ( j == 0 )
			{
       	putsDrSwitches( 6*FW, y, g_model.gvswitch[i] ,attr );
  			if(active) CHECK_INCDEC_MODELSWITCH( g_model.gvswitch[i], -MaxSwitchIndex, MaxSwitchIndex) ;
			}
			else if ( j == 1 )
			{
				if ( pgvar->gvsource >= EXTRA_POTS_START )
				{
					lcd_putsAttIdx( 12*FW-4, y, PSTR(STR_CHANS_EXTRA), pgvar->gvsource - EXTRA_POTS_START, attr ) ;
				}
				else
				{
#ifdef PCBT12
					if ( ( pgvar->gvsource == 10 ) || ( pgvar->gvsource == 11 ) )
					{
						lcd_putsAttIdx( 12*FW-4, y, "\004AUX4AUX5", pgvar->gvsource - 10, attr ) ;
					}
					else
#endif 
					lcd_putsAttIdx( 12*FW-4, y, PSTR(STR_GV_SOURCE), pgvar->gvsource, attr ) ;
				}
					// STR_GV_SOURCE
#if defined(PCBSKY) || defined(PCB9XT)
  			if(active)
				{
					uint32_t value = pgvar->gvsource ;
					// map pots etc.
					if ( value >= EXTRA_POTS_START )
					{
						value += 69 - EXTRA_POTS_START ;
					}
//					uint32_t oldValue = value ;
					CHECK_INCDEC_H_MODELVAR_0( value, 68 + NumExtraPots ) ;
					if ( value > 68 )
					{
						value += EXTRA_POTS_START - 69 ;
					}
					pgvar->gvsource = value ;
				}
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
  			if(active)
				{ 
#ifdef PCBT12
					uint32_t value = pgvar->gvsource ;
					uint32_t oldValue = value ;
					CHECK_INCDEC_H_MODELVAR_0( value, 69 ) ;
					if ( value != oldValue )
					{
						if ( ( value == 12 ) || ( value == 13 ) )
						{
							if ( value > oldValue )
							{
								value = 14 ;
							}
							else
							{
								value = 11 ;
							}
						}
					}
					pgvar->gvsource = value ;
#else
					CHECK_INCDEC_H_MODELVAR_0( pgvar->gvsource, 69 ) ;
#endif
				}
#endif
			}
			else
			{
				lcd_outdezAtt( 19*FW, y, pgvar->gvar, attr) ;
  			if(active) CHECK_INCDEC_H_MODELVAR(  pgvar->gvar, -125, 125 ) ;
			}
		}
		y += FH ;
	}
 }
}

void setTextBuffer( uint8_t *source, uint32_t number )
{
	uint32_t i ;
	uint32_t j = 0 ;
	uint32_t k = 0 ;
	uint8_t *p = source ;
 	memset( SharedMemory.TextControl.TextMenuBuffer,' ', 16*21 ) ;
	for ( i= 0 ; i < number ; i += 1 )
	{
		uint8_t c = *p++ ;
		if ( c == 10 )
		{
			j = 0 ;
			k += 21 ;
			SharedMemory.TextControl.TextLines += 1 ;
			if ( k >= 16*21 )
			{
				break ;
			}
		}
		if ( c < ' ' )
		{
			continue ;
		}
		if ( j >= 21 )
		{
			j = 0 ;
			k += 21 ;
			if ( k >= 16*21 )
			{
				break ;
			}
		}
		SharedMemory.TextControl.TextMenuBuffer[k+j] = c ;
		j += 1 ;
	}
	SharedMemory.TextControl.TextLines = k / 21 ;
	if ( j )
	{
		SharedMemory.TextControl.TextLines += 1 ;
	}
}

void writeModelNotes()
{
	uint8_t filename[50] ;
	FRESULT result ;
  UINT written ;
	uint8_t *p = SharedMemory.TextControl.TextMenuStore ;
	uint8_t *q ;
	uint32_t i ;
	uint32_t j ;
	setModelFilename( filename, g_eeGeneral.currModel+1, FILE_TYPE_TEXT ) ;

	for ( i = 0 ; i < 16 ; i += 1 )
	{
		q = &SharedMemory.TextControl.TextMenuBuffer[i*21+20] ;
		j = 21 ;
		do
		{
			if ( *q != ' ' )
			{
				break ;
			}
			q -= 1 ;
			j -= 1 ;
		} while ( j ) ;
		q = &SharedMemory.TextControl.TextMenuBuffer[i*21] ;
		while ( j )
		{
			*p++ = *q++ ;
			j -= 1 ;
		}
		*p++ = '\n' ;
	}
	j = p - SharedMemory.TextControl.TextMenuStore ;	// # bytes to write
  result = f_open( &SharedMemory.TextControl.TextFile, (TCHAR *)filename, FA_OPEN_ALWAYS | FA_WRITE) ;
	if ( result == FR_OK )
	{
 		f_write( &SharedMemory.TextControl.TextFile, (BYTE *)&SharedMemory.TextControl.TextMenuStore, j, &written ) ;
	}
	f_close( &SharedMemory.TextControl.TextFile ) ;
}

FRESULT readModelNotes()
{
	uint8_t filename[50] ;
	FRESULT result ;
	UINT nread ;

	setModelFilename( filename, g_eeGeneral.currModel+1, FILE_TYPE_TEXT ) ;
 	result = f_open( &SharedMemory.TextControl.TextFile, (TCHAR *)filename, FA_READ ) ;
	SharedMemory.TextControl.TextLines = 0 ;
	if ( result == FR_OK )
	{
		result = f_read( &SharedMemory.TextControl.TextFile, SharedMemory.TextControl.TextMenuStore, 16*21+32, &nread ) ;
		if ( result == FR_OK )
		{
			setTextBuffer( SharedMemory.TextControl.TextMenuStore, nread ) ;
		}
	}
	f_close( &SharedMemory.TextControl.TextFile ) ;
	return result ;
}

void displayNotes( uint8_t y, uint8_t lines )
{
	uint32_t i ;
	uint32_t j ;
	for ( i = 0 ; i < lines*21 ; y += FH, i += 21 )
	{
		j = SharedMemory.TextControl.TextOffset * 21 + i ;
		lcd_putsnAtt( 0, y, (const char *)&SharedMemory.TextControl.TextMenuBuffer[j], 21, 0 ) ;
	}
}


void menuEditNotes(uint8_t event)
{
	FRESULT result ;
	TITLE( XPSTR("Edit Notes") ) ;
	static MState2 mstate2 ;
	event = mstate2.check_columns( event, 15 ) ;
  uint8_t t_pgOfs ;
  uint8_t y ;
//  uint8_t k = 0;
  int8_t  sub    = mstate2.m_posVert ;

  t_pgOfs = evalOffset(sub);

	if ( event == EVT_ENTRY )
	{
		// read the notes here
		result = readModelNotes() ;
		if ( result != FR_OK )
		{
			setTextBuffer( (uint8_t *)"", 0 ) ;
		}
		Alpha.AlphaChanged = 0 ;
	}
  SharedMemory.TextControl.TextOffset = t_pgOfs ;
	y = sub - t_pgOfs + 1 ;
	displayNotes( FH, (SCREEN_LINES - 1) ) ;
	lcd_char_inverse( 0, y*FH, 126, 0 ) ;
  lcd_puts_Pleft( 0, XPSTR("\014Line")) ;
  lcd_outdez( 18*FW, 0, sub+1 ) ;
	if ( checkForMenuEncoderBreak( event ) )
	{
		Alpha.AlphaLength = 21 ;
		Alpha.PalphaText = &SharedMemory.TextControl.TextMenuBuffer[sub*21] ;
		Alpha.PalphaHeading = (uint8_t *)"Note text" ;
		s_editMode = 0 ;
    killEvents(event) ;
		pushMenu(menuProcAlpha) ;
		Alpha.AlphaChanged = 1 ;
	}
	if ( (event == EVT_KEY_BREAK(KEY_EXIT) ) || (event == EVT_KEY_LONG(KEY_EXIT) ) )
	{
		if ( Alpha.AlphaChanged )
		{
			writeModelNotes() ;		
		}
	}
}

#ifndef NO_TEMPLATES
void menuProcTemplates(uint8_t event)  //Issue 73
{
	TITLE( PSTR(STR_TEMPLATES) ) ;
	static MState2 mstate2 ;
	event = mstate2.check_columns( event, NUM_TEMPLATES-1 ) ;
	EditType = EE_MODEL ;

    uint8_t t_pgOfs ;
    uint8_t y = 0;
    uint8_t k = 0;
    int8_t sub = mstate2.m_posVert ;

    t_pgOfs = evalOffset(sub);

		if ( checkForMenuEncoderLong( event ) )
		{
      //apply mixes or delete
      s_noHi = NO_HI_LEN ;
      applyTemplate(sub) ;
      audioDefevent(AU_WARNING2) ;
    }

    y=1*FH;
    for(uint8_t i=0; i<(SCREEN_LINES - 1); i++)
		{
        k=i+t_pgOfs;
        if(k==NUM_TEMPLATES) break;

        //write mix names here
        lcd_outdezNAtt(3*FW, y, k+1, (sub==k ? INVERS : 0) + LEADING0,2);
        lcd_putsAtt(  4*FW, y, PSTR(n_Templates[k]), (s_noHi ? 0 : (sub==k ? INVERS  : 0)));
        y+=FH;
    }

}
#endif

#ifdef PCBX9LITE
#define NO_VOICE_SWITCHES 1
#endif

void menuProcSafetySwitches(uint8_t event)
{
	
	TITLE(PSTR(STR_SAFETY_SW));
	EditType = EE_MODEL ;
	static MState2 mstate2;
#ifdef NO_VOICE_SWITCHES
	event = mstate2.check_columns(event, NUM_SKYCHNOUT+1+1+NUM_VOICE-1 - 1 - 1 ) ;
#else
	event = mstate2.check_columns(event, NUM_SKYCHNOUT+1+1+NUM_VOICE-1 - 1 ) ;
#endif

	uint8_t y = 0;
	uint8_t k = 0;
	int8_t  sub    = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
  uint8_t t_pgOfs ;

//#if defined(PCBX12D) || defined(PCBX10)
//  t_pgOfs = evalOffsetLarge(sub);
//#else
	t_pgOfs = evalOffset( sub ) ;
//#endif

	if ( sub )
	{
    lcd_outdez( 20*FW, 0, g_chans512[sub-1]/2 + 1500 ) ;
	}

	for(uint8_t i=0; i<SCREEN_LINES-1; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
#ifndef NO_VOICE_SWITCHES
		if ( k == 0 )
		{
			uint8_t attr = 0 ;
    	if(sub==k)
			{
				Columns = 0 ;
				attr = InverseBlink ;
#if EXTRA_SKYCHANNELS
  	    CHECK_INCDEC_H_MODELVAR( g_model.numVoice, -8, NUM_SKYCHNOUT ) ;
#else
  	    CHECK_INCDEC_H_MODELVAR_0( g_model.numVoice, NUM_SKYCHNOUT ) ;
#endif
			}	
			lcd_xlabel_decimal( 18*FW, y, g_model.numVoice+NUM_VOICE, attr, PSTR(STR_NUM_VOICE_SW) ) ;
		}
  	else // if(k<NUM_SKYCHNOUT+1+NUM_VOICE)
#endif
		{
#ifdef NO_VOICE_SWITCHES
			uint8_t numSafety = NUM_SKYCHNOUT + EXTRA_SKYCHANNELS ; 
    	SKYSafetySwData *sd = &g_model.safetySw[k];
#else
			uint8_t numSafety = NUM_SKYCHNOUT - g_model.numVoice ;
    	SKYSafetySwData *sd = &g_model.safetySw[k-1];
#endif
			if ( sub==k )
			{
				Columns = 2 ;
			}
#ifdef NO_VOICE_SWITCHES
    	if ( k >= NUM_SKYCHNOUT )
			{
				sd = (SKYSafetySwData*)&g_model.voiceSwitches[k-NUM_SKYCHNOUT];
			}
#else
    	if ( k-1 >= NUM_SKYCHNOUT )
			{
				sd = (SKYSafetySwData*)&g_model.voiceSwitches[k-1-NUM_SKYCHNOUT];
			}
#endif
    	uint8_t attr = (getSwitch00(sd->opt.ss.swtch) ) ? INVERS : 0 ;

#ifdef NO_VOICE_SWITCHES
    	putsChn(0,y,k+1,attr);
#else
    	putsChn(0,y,k,attr);
#endif
			if ( k <= numSafety )
			{
  	  	for(uint8_t j=0; j<5;j++)
				{
    		  attr = ((sub==k && subSub==j) ? InverseBlink : 0);
					uint8_t active = (attr && (s_editMode || P1values.p1valdiff)) ;
  	  	  if (j == 0)
					{
						lcd_putsAttIdx( 5*FW-3, y, XPSTR("\001SAVX"), sd->opt.ss.mode, attr ) ;
	      	  if(active)
						{
							uint8_t b = sd->opt.ss.mode ;
	  	        CHECK_INCDEC_H_MODELVAR_0( sd->opt.ss.mode, 3 ) ;
#ifdef PCBX9LITE
							if ( ( k < numSafety ) & ( b == 0 ) )
#else
							if ( ( k < 8 ) & ( b == 0 ) )
#endif
							{
								if ( sd->opt.ss.mode != 0 )
								{
									sd->opt.ss.mode = 3 ;
								}
							}
#ifdef PCBX9LITE
							if ( ( k < numSafety ) & ( b == 3 ) )
							{
								if ( sd->opt.ss.mode != 3 )
								{
									sd->opt.ss.mode = 0 ;
								}
							}
#endif
  	  	    }
					}
	    	  else if (j == 1)
  	  	  {
						int8_t max = MaxSwitchIndex ;
						if ( sd->opt.ss.mode == 2 )
						{
							max = MaxSwitchIndex+3 ;
						}	 
//						if ( ( sd->opt.ss.swtch > MAX_SKYDRSWITCH ) && ( sd->opt.ss.swtch <= MAX_SKYDRSWITCH + 3 ) )
//						{
//							lcd_putsAttIdx( 6*FW, y, PSTR(STR_V_OPT1), sd->opt.ss.swtch-MAX_SKYDRSWITCH-1, attr ) ;
//						}
//						else
						{
         	  	putsDrSwitches(6*FW, y, sd->opt.ss.swtch, attr);
						}
	    	    if(active)
						{
              CHECK_INCDEC_MODELSWITCH( sd->opt.ss.swtch, -MaxSwitchIndex, max ) ;
    		    }
					}
					else if ( j == 2 )
					{
						int8_t min, max ;
						if ( sd->opt.ss.mode == 1 )
						{
							min = 0 ;
							max = 15 ;
							sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
							lcd_putsAttIdx(14*FW, y, PSTR(STR_SOUNDS), sd->opt.ss.val,attr);
						}
						else if ( sd->opt.ss.mode == 2 )
						{
							if ( sd->opt.ss.swtch > MAX_SKYDRSWITCH )
							{
								min = 0 ;
								max = NUM_TELEM_ITEMS-1 ;
								sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
  							putsAttIdxTelemItems( 15*FW, y, sd->opt.ss.val+1, attr ) ;
							}
							else
							{
								min = -128 ;
								max = 111 ;
								sd->opt.ss.val = limit( min, sd->opt.ss.val, max) ;
        				lcd_outdezAtt( 15*FW, y, sd->opt.ss.val+128, attr);
							}
						}
						else
						{
        			lcd_putc( 14*FW, y, '.' ) ;
							min = -125 ;
							max = 125 ;
        			lcd_outdezAtt(  14*FW, y, sd->opt.ss.val, attr);
// Option to display current channel value, before limits etc., for failsafe
						}
  	  	    if(active)
						{
		          CHECK_INCDEC_H_MODELVAR( sd->opt.ss.val, min,max);
    	  	  }
					}
					else if ( j == 3 )
					{
						if ( ( sd->opt.ss.mode == 0 ) || ( sd->opt.ss.mode == 3 ) )
						{
    					if(sub==k)
							{							
								Columns = 3 ;
							}
        			lcd_outdezAtt(  15*FW+3, y, sd->opt.ss.tune, attr);
	  	  	    if(active)
							{
			          CHECK_INCDEC_H_MODELVAR( sd->opt.ss.tune, 0, 9 ) ;
    		  	  }
						}
					}
					else
					{
						if ( sd->opt.ss.mode == 3 )
						{
    					if(sub==k)
							{							
								Columns = 4 ;
							}
							int8_t temp = sd->opt.ss.source ;
							if ( temp == 0 )
							{
								temp = 3 ;
							}
							else
							{
								if ( temp > 0 )
								{
									temp += 4 ;
								}
								else
								{
									temp -= 4 ;
								}
							}
							if ( temp < 0 )
							{
								temp = - temp ;
								lcd_putc( 16*FW, y, '!' ) ;
							}
							if ( temp > 7 )
							{ // AN extra pot
								temp += EXTRA_POTS_START - 8 ;
							}
							putsChnRaw( 17*FW, y, temp, attr ) ;
	    	    	if(active)
							{
								temp = sd->opt.ss.source ;
								CHECK_INCDEC_H_MODELVAR( temp, -3-NumExtraPots , 3+NumExtraPots ) ;
								sd->opt.ss.source = temp ;
    		    	}
						}
					}
    	  }
    	}
			else
			{
	  	  lcd_puts_Pleft( y, PSTR(STR_VS) ) ;
    		for(uint8_t j=0; j<3;j++)
				{
    	    uint8_t attr = ((sub==k && subSub==j) ? InverseBlink : 0);
					uint8_t active = (attr && (s_editMode || P1values.p1valdiff)) ;
    		  if (j == 0)
					{
    		    if(active)
						{
    			    CHECK_INCDEC_MODELSWITCH( sd->opt.vs.vswtch, -MaxSwitchIndex+1, MaxSwitchIndex-1 ) ;
    		    }
  			    putsDrSwitches(5*FW, y, sd->opt.vs.vswtch, attr);
					}
    		  else if (j == 1)
    		  {
    		    if(active)
						{
    			    CHECK_INCDEC_H_MODELVAR_0( sd->opt.vs.vmode, 6 ) ;
    		    }
						lcd_putsAttIdx( 10*FW, y, PSTR(STR_VOICE_OPT), sd->opt.vs.vmode, attr ) ;
					}
					else
					{
						uint8_t max ;
						if ( sd->opt.vs.vmode > 5 )
						{
							max = NUM_TELEM_ITEMS-1 ;
							sd->opt.vs.vval = limit( (uint8_t)0, sd->opt.vs.vval, max) ;
							putsAttIdxTelemItems( 16*FW, y, sd->opt.vs.vval+1, attr ) ;
						}
						else
						{
							// Allow 251-255 to represent GVAR3-GVAR7
							max = 255 ;
							if ( sd->opt.vs.vval <= 250 )
							{
	  						lcd_outdezAtt( 19*FW, y, sd->opt.vs.vval, attr) ;
							}	
							else
							{
								dispGvar( 16*FW, y, sd->opt.vs.vval-248, attr ) ;
							}
						}
    			  if(active)
						{
    		      sd->opt.vs.vval = checkIncDec16( sd->opt.vs.vval, 0, max, EE_MODEL);
							if ( checkForMenuEncoderLong( event ) )
							{
								if ( sd->opt.vs.vval <= 250 )
								{
									putVoiceQueue( sd->opt.vs.vval | VLOC_NUMUSER  ) ;
								}
							}
    			  }
					}	 
			  }
			}
		}
//		else
//		{
//			// Function(s)
//			lcd_puts_Pleft( y, PSTR("F1") ) ;
//  	 	for( uint8_t j=0 ; j<3 ; j += 1 )
//			{
//  	    uint8_t attr = ((sub==k && subSub==j) ? (s_editMode ? BLINK : INVERS) : 0) ;
//				uint8_t active = (attr && s_editMode) ;
//  	 		if (j == 0)
//				{
//					putsDrSwitches( 5*FW, y, Function[0].swtch , attr ) ;
//  	 		  if(active)
//					{
//						Function[0].swtch = checkIncDec( event, Function[0].swtch, -MAX_DRSWITCH+1, MAX_DRSWITCH, 0 ) ;
//  	 		  }
//				}
//  	 		else if (j == 1)
//  	 		{
//					lcd_putsAttIdx( 10*FW, y, PSTR("\005-----variovarA2"), Function[0].func, attr ) ;
//  	 		  if(active)
//					{
//						Function[0].func = checkIncDec( event, Function[0].func, 0, 2, 0 ) ;
//  	 		  }
//				}
//				else
//				{
// 					lcd_outdezAtt( 17*FW, y, Function[0].param[0], attr) ;
//  	 		  if(active)
//					{
//						Function[0].param[0] = (uint8_t)checkIncDec( event, (int8_t)Function[0].param[0], 0, 50, 0 ) ;
//  	 		  }
//				}	
//			}
//		}
	}
}

void editOneSwitchItem( uint8_t event, uint32_t item, uint32_t index )
{
	SKYCSwData &cs = g_model.customSw[index] ;
	uint8_t cstate = CS_STATE(cs.func) ;
	switch ( item )
	{
		case 0 :
			cs.func = checkOutOfOrder( cs.func, (uint8_t *)SwitchFunctionMap, CS_MAXF ) ;
//  	  CHECK_INCDEC_H_MODELVAR_0( cs.func, CS_MAXF);
			if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
			{
				// Long MENU pressed
				TextIndex = cs.func ;
				TextType = 3 ;
				saveHpos = item ;
				killEvents(event) ;
				pushMenu(menuTextHelp) ;
			}
	  	if(cstate != CS_STATE(cs.func) )
  		{
  		  cs.v1 = 0 ;
  		  cs.v2 = 0 ;
	  	}
		break ;
		
		case 1 :
      switch (cstate)
			{
      	case (CS_VCOMP):
      	case (CS_VOFS):
				case (CS_U16):
				{
 					uint8_t x = mapPots( cs.v1u ) ;
					CHECK_INCDEC_H_MODELVAR( x, 0, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots-1+1 ) ;
					cs.v1u = unmapPots( x ) ;
					if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
					{
						// Long MENU pressed
						TextIndex = x ;
  					TextType = 1 ;
						saveHpos = item ;
  					killEvents(event) ;
						pushMenu(menuTextHelp) ;
					}
				}
   	    break;
      	case (CS_VBOOL):
				case CS_TMONO :
					CHECK_INCDEC_MODELSWITCH( cs.v1, -MaxSwitchIndex,MaxSwitchIndex);
   	    break;
      	case (CS_TIMER):
 					CHECK_INCDEC_H_MODELVAR( cs.v1, -50,99);
   	    break;
						
				default:
          break;
      }
		break ;

		case 2:
			switch (cstate)
			{
        case (CS_VOFS):
				case CS_TMONO :
          CHECK_INCDEC_H_MODELVAR( cs.v2, -125,125) ;
        break ;
        case (CS_VBOOL):
          CHECK_INCDEC_MODELSWITCH( cs.v2, -MaxSwitchIndex,MaxSwitchIndex);
        break ;
        case (CS_VCOMP):
				{
					uint8_t x = mapPots( cs.v2u ) ;
					CHECK_INCDEC_H_MODELVAR( x, 0, NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots-1 ) ;
					cs.v2u = unmapPots( x ) ;
					if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
					{
						// Long MENU pressed
						TextIndex = x ;
  					TextType = 1 ;
						saveHpos = item ;
  					killEvents(event) ;
						pushMenu(menuTextHelp) ;
					}
				}
        break ;
        case (CS_TIMER):
            CHECK_INCDEC_H_MODELVAR( cs.v2, -50,99);
        break ;
            
				case (CS_U16):
				{	
					int32_t y ;
					y = (uint8_t) cs.v2 ;
					y |= cs.bitAndV3 << 8 ;
					y -= 32768 ;
					StepSize = 128 ;
					y = checkIncDec16( y, -32768, 32767, EE_MODEL ) ;
					y += 32768 ;
					cs.bitAndV3 = y >> 8 ;
					cs.v2 = y ;
				}
        break ;

        default:
        break ;
			}
		break ;

    case 3:
		{	
			int32_t x ;
			x = getAndSwitch( cs ) ;
	    CHECK_INCDEC_MODELSWITCH( x, -MaxSwitchIndex+1,MaxSwitchIndex-1);
#if defined(PCBSKY) || defined(PCB9XT)
			if ( x == 9 )
			{
				x = 9+NUM_SKYCSW+1 ;
			}
			if ( x == -9 )
			{
				x = -(9+NUM_SKYCSW+1) ;
			}
			if ( ( x > 9 ) && ( x <= 9+NUM_SKYCSW+1 ) )
			{
				x -= 1 ;
			}
			if ( ( x < -9 ) && ( x >= -(9+NUM_SKYCSW+1) ) )
			{
				x += 1 ;
			}
#endif
			cs.andsw = x ;
		}
		break;
		
		case 4 :
			CHECK_INCDEC_H_MODELVAR_0( g_model.switchDelay[index], 25 ) ;
		break ;
	}
	
}

void displayLogicalSwitch( uint16_t x, uint16_t y, uint32_t index )
{
	uint8_t attr ;
  attr = (getSwitch00(CSW_INDEX+index+1) ) ? INVERS : 0 ;
  lcd_putc( x, y, 'L' ) ;
  lcd_putcAtt( x+FW-1, y, index + (index>8 ? 'A'-9: '1'), attr ) ;
}

void menuSwitchOne(uint8_t event)
{
	static MState2 mstate2 ;
	uint8_t attr ;
	uint32_t index ;
	uint8_t blink = InverseBlink ;
	uint32_t rows = 5 ;
	if ( Clipboard.content == CLIP_SWITCH )
	{
		rows += 1 ;
	}

	TITLEP( XPSTR("SWITCH") ) ;

	mstate2.check_columns(event, rows ) ;
//	uint8_t x = TITLEP( PSTR(STR_EDIT_MIX));

	int8_t sub = mstate2.m_posVert ;
	index = s_currIdx ;
	SKYCSwData &cs = g_model.customSw[index] ;

	if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		sub = mstate2.m_posVert = saveHpos ;
		
		if ( TextResult )
		{
			if ( sub == 0 )
			{
				cs.func = SwitchFunctionMap[TextIndex] ;
			}
			else if ( sub == 1 )
			{
				cs.v1u = unmapPots( TextIndex ) ;
			}
			else
			{
				cs.v2u = unmapPots( TextIndex ) ;
			}
	 	  eeDirty(EE_MODEL) ;
		}
	}

	uint8_t cstate = CS_STATE(cs.func) ;

	//write SW name here
	displayLogicalSwitch( 7*FW, 0, index ) ;

	lcd_puts_Pleft( FH, XPSTR("Function\037v1\037v2\037AND\037Delay\037Copy") ) ;

  if(cstate == CS_VOFS)
  {
		lcd_puts_Pleft( 3*FH, XPSTR("val") ) ;
		putsChnRaw( 9*FW, 2*FH, cs.v1u, sub == 1 ? blink : 0 ) ;

    if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
		{
			int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
			putsTelemetryChannel( 9*FW, 3*FH, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, sub==2 ? blink : 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT ) ;
		}
    else
		{
    	lcd_outdezAtt( 12*FW, 3*FH, cs.v2, sub==2 ? blink : 0 ) ;
		}
  }
  else if(cstate == CS_VBOOL)
  {
    putsDrSwitches( 9*FW, 2*FH, cs.v1, sub==1 ? blink : 0 ) ;
    putsDrSwitches( 9*FW, 3*FH, cs.v2, sub==2 ? blink : 0 ) ;
  }
  else if(cstate == CS_VCOMP)
  {
    putsChnRaw( 9*FW, 2*FH, cs.v1u, sub==1 ? blink : 0 ) ;
    putsChnRaw( 9*FW, 3*FH, cs.v2u, sub==2 ? blink : 0 ) ;
  }
	else if(cstate == CS_TIMER)
	{
		int8_t x ;
		uint8_t att = 0 ;
		x = cs.v1 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
	  lcd_puts_Pleft( 2*FH, XPSTR("Off") ) ;
    lcd_outdezAtt( 12*FW, 2*FH, x+1  ,att | (sub==1 ? blink : 0) ) ;
		att = 0 ;
		x = cs.v2 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
	  lcd_puts_Pleft( 3*FH, XPSTR("On") ) ;
    lcd_outdezAtt( 12*FW, 3*FH, x+1 , att | (sub==2 ? blink : 0 ) ) ;
	}
	else if(cstate == CS_TMONO)
	{
    putsDrSwitches( 9*FW, 2*FH, cs.v1  ,sub==1 ? blink : 0);
		uint8_t att = 0 ;
		int8_t x ;
		x = cs.v2 ;
		if ( x < 0 )
		{
			x = -x-1 ;
			att = PREC1 ;
		}
		lcd_puts_Pleft( 3*FH, XPSTR("Time") ) ;
    lcd_outdezAtt( 12*FW, 3*FH, x+1 , att | (sub==2 ? blink : 0 ) ) ;
	}
	else// cstate == U16
	{
		uint16_t x ;
		x = (uint8_t) cs.v2 ;
		x |= cs.bitAndV3 << 8 ;
    putsChnRaw( 9*FW, 2*FH, cs.v1u, sub==1 ? blink : 0);
    lcd_outdezNAtt( 14*FW, 3*FH, x ,sub==2 ? blink : 0,5);
	}

	attr = 0 ;
	if ( sub == 0 )
	{
		attr = InverseBlink ;
	}
	lcd_putsAttIdx( 9*FW, FH, PSTR(CSWITCH_STR), cs.func, attr ) ;

	putsDrSwitches( 8*FW, 4*FH, getAndSwitch( cs ),(sub==3 ? blink : 0 ) ) ;

	attr = 0 ;
	if ( sub == 4 )
	{
		attr = blink ;
	}
	lcd_outdezAtt( 12*FW, 5*FH, g_model.switchDelay[index], attr|PREC1 ) ;

	if ( sub <= 4 )
	{
		editOneSwitchItem( event, sub, index ) ;
	}
	TextResult = 0 ;

	if ( sub == 5 )
	{
		lcd_char_inverse( 0, 6*FH, 24, 0 ) ;
		if ( checkForMenuEncoderLong( event ) )
		{
			Clipboard.clipswitch = cs ;
			Clipboard.content = CLIP_SWITCH ;
		}
	}

	if ( Clipboard.content == CLIP_SWITCH )
	{
  	lcd_puts_Pleft( 7*FH, XPSTR("Paste") ) ;
		if ( sub == 6 )
		{
			lcd_char_inverse( 0, 7*FH, 30, 0 ) ;
			if ( checkForMenuEncoderLong( event ) )
			{
				cs = Clipboard.clipswitch ;
			}
		}
	}
}

//#ifdef SMALL
#define SWITCH_SUB_ONLY
//#endif

void menuProcSwitches(uint8_t event)
{
	TITLE(PSTR(STR_CUST_SWITCH));
	EditType = EE_MODEL ;
#ifndef SWITCH_SUB_ONLY
	Columns = 4 ;
#endif
	static MState2 mstate2;
#ifndef SWITCH_SUB_ONLY
	event = mstate2.check_columns(event, NUM_SKYCSW+1-1-1+NUM_SKYCSW ) ;
#else
	event = mstate2.check_columns(event, NUM_SKYCSW+1-1-1 ) ;
#endif

	uint8_t y = 0;
	uint8_t k = 0;
	int8_t  sub    = mstate2.m_posVert ;
#ifndef SWITCH_SUB_ONLY
	uint8_t subSub = g_posHorz;
#endif
	uint8_t t_pgOfs ;

//#if defined(PCBX12D) || defined(PCBX10)
//  t_pgOfs = evalOffsetLarge(sub);
//#else
	t_pgOfs = evalOffset( sub ) ;
//#endif

#ifndef SWITCH_SUB_ONLY
	if ( event == EVT_ENTRY_UP )
	{
		// Returned from editing
		subSub = g_posHorz = saveHpos ;
    SKYCSwData &cs = g_model.customSw[sub];
		
		if ( TextResult )
		{
			if ( subSub == 0 )
			{
				cs.func = SwitchFunctionMap[TextIndex] ;
			}
			else if ( subSub == 1 )
			{
				cs.v1u = unmapPots( TextIndex ) ;
			}
			else
			{
				cs.v2u = unmapPots( TextIndex ) ;
			}
	 	  eeDirty(EE_MODEL) ;
		}
	}
#endif

	for(uint8_t i=0; i<SCREEN_LINES-1; i++)
	{
    y=(i+1)*FH;
    k=i+t_pgOfs;
    uint8_t attr ;
		uint8_t m = k ;
		if ( k >= NUM_SKYCSW )
		{
			m -= NUM_SKYCSW ;
			Columns = 0 ;
		}
    SKYCSwData &cs = g_model.customSw[m];

		//write SW names here
		displayLogicalSwitch( 0, y, m ) ;
    
#ifndef SWITCH_SUB_ONLY
		attr = (sub==k ? InverseBlink  : 0);
#else
		attr = (sub==k ? INVERS : 0);
#endif
	 if ( k < NUM_SKYCSW )
	 {
#ifndef SWITCH_SUB_ONLY
		lcd_putsAttIdx( 2*FW+1, y, PSTR(CSWITCH_STR),cs.func,subSub==0 ? attr : 0);
#else
		lcd_putsAttIdx( 2*FW+1, y, PSTR(CSWITCH_STR),cs.func, 0);
#endif

    uint8_t cstate = CS_STATE(cs.func);

    if(cstate == CS_VOFS)
    {
#ifndef SWITCH_SUB_ONLY
			putsChnRaw(    10*FW-6, y, cs.v1u  ,subSub==1 ? attr : 0);
#else
			putsChnRaw(    10*FW-6, y, cs.v1u  , 0);
#endif
	    if ( ( (cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || ( cs.v1u >= EXTRA_POTS_START + 8) )
 			{
				int16_t value = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
#ifndef SWITCH_SUB_ONLY
				putsTelemetryChannel( 18*FW-8, y, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, subSub==2 ? attr : 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT);
#else
				putsTelemetryChannel( 18*FW-8, y, cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, value, 0, TELEM_NOTIME_UNIT | TELEM_UNIT| TELEM_CONSTANT);
#endif
			}
      else
			{
#ifndef SWITCH_SUB_ONLY
        lcd_outdezAtt( 18*FW-9, y, cs.v2  ,subSub==2 ? attr : 0);
#else
        lcd_outdezAtt( 18*FW-9, y, cs.v2  ,0);
#endif
			}
    }
    else if(cstate == CS_VBOOL)
    {
#ifndef SWITCH_SUB_ONLY
      putsDrSwitches(10*FW-6, y, cs.v1  ,subSub==1 ? attr : 0);
      putsDrSwitches(14*FW-7, y, cs.v2  ,subSub==2 ? attr : 0);
#else
      putsDrSwitches(10*FW-6, y, cs.v1  , 0);
      putsDrSwitches(14*FW-7, y, cs.v2  , 0);
#endif
    }
    else if(cstate == CS_VCOMP)
    {
#ifndef SWITCH_SUB_ONLY
      putsChnRaw(    10*FW-6, y, cs.v1u  ,subSub==1 ? attr : 0);
      putsChnRaw(    14*FW-4, y, cs.v2u  ,subSub==2 ? attr : 0);
#else
      putsChnRaw(    10*FW-6, y, cs.v1u  , 0);
      putsChnRaw(    14*FW-4, y, cs.v2u  , 0);
#endif
    }
		else if(cstate == CS_TIMER)
		{
			int8_t x ;
			uint8_t att = 0 ;
			x = cs.v1 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
	    lcd_puts_Pleft( y, PSTR(STR_15_ON) ) ;
#ifndef SWITCH_SUB_ONLY
      lcd_outdezAtt( 13*FW-5, y, x+1  ,att | (subSub==1 ? attr : 0) ) ;
#else
      lcd_outdezAtt( 13*FW-5, y, x+1  ,att ) ;
#endif
			att = 0 ;
			x = cs.v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
#ifndef SWITCH_SUB_ONLY
      lcd_outdezAtt( 18*FW-3, y, x+1 , att | (subSub==2 ? attr : 0 ) ) ;
#else
      lcd_outdezAtt( 18*FW-3, y, x+1 , att ) ;
#endif
		}
		else if(cstate == CS_TMONO)
		{
#ifndef SWITCH_SUB_ONLY
      putsDrSwitches(10*FW-6, y, cs.v1  ,subSub==1 ? attr : 0);
#else
      putsDrSwitches(10*FW-6, y, cs.v1  , 0);
#endif
			uint8_t att = 0 ;
			int8_t x ;
			x = cs.v2 ;
			if ( x < 0 )
			{
				x = -x-1 ;
				att = PREC1 ;
			}
#ifndef SWITCH_SUB_ONLY
      lcd_outdezAtt( 17*FW-2, y, x+1 , att | (subSub==2 ? attr : 0 ) ) ;
#else
      lcd_outdezAtt( 17*FW-2, y, x+1 , att ) ;
#endif
		}
		else// cstate == U16
		{
			uint16_t x ;
			x = cs.v2u ;
			x |= cs.bitAndV3 << 8 ;
#ifndef SWITCH_SUB_ONLY
      putsChnRaw( 10*FW-6-FW, y, cs.v1  ,subSub==1 ? attr : 0);
      lcd_outdezNAtt( 18*FW-9, y, x  ,subSub==2 ? attr : 0,5);
#else
      putsChnRaw( 10*FW-6-FW, y, cs.v1  , 0);
      lcd_outdezNAtt( 18*FW-9, y, x  , 0,5);
#endif
		}
//    lcd_putc( 19*FW+3, y, cs.andsw ? 'S' : '-') ;
		
#ifndef SWITCH_SUB_ONLY
		putsDrSwitches( 18*FW-3, y, getAndSwitch( cs ),(subSub==3 ? attr : 0)) ;
#else
		putsDrSwitches( 18*FW-3, y, getAndSwitch( cs ),0 ) ;
#endif

#ifdef BIG_SCREEN
		lcd_puts_P( 22*FW, y, XPSTR("Delay" ) ) ;
		lcd_outdezAtt( 31*FW, y, g_model.switchDelay[m], PREC1 ) ;
#endif

#ifndef SWITCH_SUB_ONLY
    if((s_editMode || P1values.p1valdiff) && attr)
		{
			editOneSwitchItem( event, subSub, sub ) ;
		}
#endif			
#ifndef SWITCH_SUB_ONLY
		if ( subSub == 4 )
		{
#endif			
			if ( attr )
			{
#ifndef SWITCH_SUB_ONLY
				lcd_char_inverse( 12, y, 127, BLINK ) ;
#else
 #ifdef BIG_SCREEN
 				lcd_char_inverse( 12, y, 31*FW-11, 0 ) ;
 #else
				lcd_char_inverse( 12, y, 127-12, 0 ) ;
				lcd_puts_P( 102, 0, XPSTR("Dy" ) ) ;
				lcd_outdezAtt( 21*FW, 0, g_model.switchDelay[m], PREC1 ) ;
 #endif
#endif
				if ( checkForMenuEncoderBreak( event ) )
				{
					// Long MENU pressed
		      s_currIdx = sub ;
  				TextType = 0 ;
					pushMenu(menuSwitchOne) ;
  	  		s_editMode = false ;
				}
			}
#ifndef SWITCH_SUB_ONLY
		}
#endif
	 }
#ifndef SWITCH_SUB_ONLY
	 else
	 {
			lcd_puts_Pleft( y, XPSTR("\004delay=" ) ) ;
			lcd_outdezAtt( 14*FW, y, g_model.switchDelay[m], attr|PREC1 ) ;
			if ( attr )
			{
				CHECK_INCDEC_H_MODELVAR_0( g_model.switchDelay[m], 25 ) ;
			}
	 }
#endif
	}
}

void deleteMix(uint8_t idx)
{
#if EXTRA_SKYMIXERS
	while ( idx < MAX_SKYMIXERS+EXTRA_SKYMIXERS-2 )
	{
		*mixAddress(idx) = *mixAddress(idx+1) ;
		idx += 1 ;
	}
  memset(&g_model.exmixData[EXTRA_SKYMIXERS-1],0,sizeof(SKYMixData));
#else		
		memmove(&g_model.mixData[idx],&g_model.mixData[idx+1],
            (MAX_SKYMIXERS-(idx+1))*sizeof(SKYMixData));
    memset(&g_model.mixData[MAX_SKYMIXERS-1],0,sizeof(SKYMixData));
#endif
	STORE_MODELVARS;

}

void insertMix(uint8_t idx, uint8_t copy)
{
  SKYMixData *md = mixAddress( idx ) ;

#if EXTRA_SKYMIXERS
	uint8_t xdx ;
	xdx = MAX_SKYMIXERS+EXTRA_SKYMIXERS-1 ;
	while ( xdx > idx )
	{
		*mixAddress(xdx) = *mixAddress(xdx-1) ;
		xdx -= 1 ;
	}

	if ( copy )
	{
		*md = *(md-1) ;
	}
#else
  memmove(md+1,md, (MAX_SKYMIXERS-(idx+1))*sizeof(SKYMixData) );
	if ( copy )
	{
    memmove( md, md-1, sizeof(SKYMixData) ) ;
	}
#endif
	else
	{
   	memset(md,0,sizeof(SKYMixData));
   	md->destCh      = s_currDestCh; //-s_mixTab[sub];
   	md->srcRaw      = s_currDestCh; //1;   //
#if defined(PCBLEM1)
    md->weight      = 80 ;
#else
    md->weight      = 100 ;
#endif
		md->lateOffset  = 1 ;
	}
	s_curItemIdx = idx ;
}

void put_curve( uint8_t x, uint8_t y, int8_t idx, uint8_t attr )
{
	if ( idx < 0 )
	{
    lcd_putcAtt( x-FW, y, '!', attr ) ;
		idx = -idx + 6 ;
	}
	lcd_putsAttIdx( x, y,get_curve_string(),idx,attr);
}

void displayVoiceRate( uint8_t x, uint8_t y, uint8_t rate, uint8_t attr )
{
	if ( ( rate >= 4 ) && ( rate <=32 ) )
	{
    lcd_outdezAtt(x+4*FW,y,rate-2, attr ) ;
	}
	else
	{
		if ( rate > 32 )
		{
			rate -= 29 ;
		}
		lcd_putsAttIdx( x, y, XPSTR("\004  ON OFFBOTH ALLONCE"),rate, attr ) ;
	}
}

static void memswap( void *a, void *b, uint8_t size )
{
	uint8_t *x ;
	uint8_t *y ;
	uint8_t temp ;

	x = (unsigned char *) a ;
	y = (unsigned char *) b ;
	while ( size-- )
	{
		temp = *x ;
		*x++ = *y ;
		*y++ = temp ;
	}
}


VoiceAlarmData *voiceAddress( uint32_t index, uint8_t mode )
{
  if ( index >= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
	{
		index -= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ;
		mode = 1 ;
	}	
	return mode ? &g_eeGeneral.gvad[index] : ( ( index >= NUM_VOICE_ALARMS) ? &g_model.vadx[index - NUM_VOICE_ALARMS] : &g_model.vad[index] ) ;
}

static void deleteVoice(VoiceAlarmData *pvad)
{
	pvad->source = 0 ;
	pvad->func = 0 ;
	pvad->swtch = 0 ;
	pvad->rate = 0 ;
	pvad->fnameType  = 0 ;
	pvad->haptic = 0 ;
	pvad->vsource = 0  ;
	pvad->offset = 0 ;
	pvad->delay = 0 ;
	memset( (uint8_t *)pvad->file.name, 0, sizeof(pvad->file.name) ) ;
}

#ifdef MOVE_VOICE

void menuPasteVoice(uint8_t event)
{
	uint8_t action ;
	action = yesNoMenuExit( event, XPSTR("Paste Voice Alert?") ) ;
  
	switch( action )
	{
    case YN_YES :
		{	
			*voiceAddress( s_currIdx, 0 ) = Clipboard.clipvoice ;
			MuteTimer = 5 ;
		}
      //fallthrough
		case YN_NO :
//      pushMenu(menuProcMix);
    break;
  }
}


void menuDeleteVoice(uint8_t event)
{
	uint8_t action ;
	action = yesNoMenuExit( event, XPSTR("Delete Voice Alert?") ) ;
  
	switch( action )
	{
    case YN_YES :
     	deleteVoice(voiceAddress( s_currIdx, 0 )) ;
      //fallthrough
		case YN_NO :
//      pushMenu(menuProcMix);
    break;
  }
}

void moveVoice(uint8_t idx, uint8_t dir, uint8_t mode) //true=inc=down false=dec=up
{
	VoiceAlarmData *psrc ;
	VoiceAlarmData *pdest ;
	uint32_t rows = mode ? NUM_GLOBAL_VOICE_ALARMS-1 : NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS-1 ;
	psrc = voiceAddress( idx, mode ) ;
	if(idx==0 && !dir)
	{
		return ;
	}
	if( idx > rows || (idx == rows && dir ) )
	{
		return ;
	}
	uint8_t tidx = dir ? idx+1 : idx-1;
	pdest = voiceAddress( tidx, mode ) ;
  //flip between idx and tgt
  memswap( pdest, psrc, sizeof(VoiceAlarmData) ) ;
	s_curItemIdx = s_moveItemIdx = tidx ;
	MuteTimer = 5 ;

	if ( mode )
	{
  	STORE_GENERALVARS;
	}
	else
	{
		STORE_MODELVARS;
	}
}

void voicepopup( uint8_t event )
{
	uint8_t mask = 0x1D ;
	if ( Clipboard.content == CLIP_VOICE )
	{
		mask = 0x9D ;
	}
	uint8_t popaction = doPopup( PSTR(STR_MIX_POPUP), mask, 11, event ) ;	// Edit,copy,move,Delete
  if ( popaction == POPUP_SELECT )
	{
		uint8_t popidx = PopupData.PopupSel ;
		if ( popidx == 0 )	// Edit
		{
	    pushMenu(menuProcVoiceOne) ;
		}
		else if ( popidx == 3 )	// Move
		{
			s_moveMode = 1 ;
		}
		else if ( popidx == 2 )	// Copy
		{
			VoiceAlarmData *pvad ;
			pvad = voiceAddress( s_currIdx, 0 ) ;
			Clipboard.clipvoice = *pvad ;
			Clipboard.content = CLIP_VOICE ;
		}
		else if ( popidx == 4 )	// Delete
		{
			killEvents(event);
			Tevent = 0 ;
			pushMenu(menuDeleteVoice);
		}
		else if ( popidx == 7 )	// Paste
		{
			killEvents(event);
			Tevent = 0 ;
			pushMenu(menuPasteVoice) ;
		}
		PopupData.PopupActive = 0 ;
	}
	s_moveItemIdx = s_curItemIdx ;
}
#endif

// FUnctions need to include ON, OFF and BOTH possibly
void menuProcVoiceOne(uint8_t event)
{
	TITLE( PSTR( STR_Voice_Alarm ));
	static MState2 mstate2;
	VoiceAlarmData *pvad ;
  if ( s_currIdx >= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
	{
		EditType = EE_GENERAL ;
		uint8_t z = s_currIdx - ( NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ) ;
		pvad = &g_eeGeneral.gvad[z] ;
		lcd_putc( 12*FW, 0, 'G' ) ;
		lcd_outdezAtt( 14*FW-1, 0, z+1, 0 ) ;
	}
	else
	{
		pvad = (s_currIdx >= NUM_VOICE_ALARMS) ? &g_model.vadx[s_currIdx - NUM_VOICE_ALARMS] : &g_model.vad[s_currIdx] ;
		lcd_outdezAtt( 13*FW, 0, s_currIdx+1, 0 ) ;
	}
#ifndef HAPTIC
	uint32_t rows = pvad->fnameType ? 10-1: 9-1 ;
#else
	uint32_t rows = pvad->fnameType ? 11-1: 10-1 ;
#endif
#ifndef MOVE_VOICE
	if ( Clipboard.content == CLIP_VOICE )
	{
		rows += 1 ;
	}
#endif
	event = mstate2.check_columns( event, rows ) ;
	int8_t sub = mstate2.m_posVert ;
	
	if ( event == EVT_ENTRY )
	{
		RotaryState = ROTARY_MENU_UD ;
	}
	else if ( event == EVT_ENTRY_UP )
	{
		if ( sub == 0 )
		{
			// Returned from editing
			if ( TextResult )
			{
				pvad->source = unmapPots( TextIndex ) ;
	  	  eeDirty(EE_MODEL) ;
			}
		}
		else
		{
		 	// From menuProcSelectUvoiceFile
			if ( FileSelectResult == 1 )
			{
		 		copyFileName( (char *)pvad->file.name, SelectedVoiceFileName, 8 ) ;
	   		eeDirty(EE_MODEL) ;		// Save it
			}
		}
	}

	
  for(uint8_t i=0; i<=rows; i++)
  {
		uint8_t j = i ;
		uint8_t y = (i+1)*FH;
    uint8_t attr ;

#ifdef HAPTIC
#define V1P1		6
#else
#define V1P1		5
#endif

		if ( pvad->fnameType == 0 )
		{
			if ( j >= V1P1+4 )
			{
				j += 1 ;
			}
		}
    attr = sub==i ? InverseBlink : 0 ;

#ifndef BIG_SCREEN
		if ( sub < V1P1 )
		{
			displayNext() ;
#endif
	    switch(j)
			{
    	  case 0 :	// source
  	  		lcd_puts_Pleft( FH, XPSTR("Source") ) ;
					putsChnRaw( 16*FW, y, pvad->source, attr ) ;
					if ( attr )
					{
						uint8_t x = mapPots( pvad->source ) ;
						CHECK_INCDEC_H_MODELVAR( x, 0,NUM_SKYXCHNRAW+NUM_TELEM_ITEMS+NumExtraPots ) ;
						pvad->source = unmapPots( x ) ;
						if ( ( event == EVT_KEY_LONG(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
						{
							// Long MENU pressed
							TextIndex = mapPots( pvad->source ) ;
  					  TextType = 1 ;
  					  killEvents(event) ;
							pushMenu(menuTextHelp) ;
						}
					}
					if ( pvad->source )
					{
						int16_t value ;
						value = getValue( pvad->source - 1 ) ;
  	  			lcd_puts_Pleft( FH, XPSTR("\007(\016)") ) ;
    				if ( ( (pvad->source > CHOUT_BASE+NUM_SKYCHNOUT) && ( pvad->source < EXTRA_POTS_START ) ) || ( pvad->source >= EXTRA_POTS_START + 8) )
 						{
							putsTelemetryChannel( 12*FW, FH, pvad->source-CHOUT_BASE-NUM_SKYCHNOUT-1, value, 0, TELEM_NOTIME_UNIT | TELEM_UNIT ) ;
						}
						else
						{
							lcd_outdezAtt( 12*FW, FH, value, 0 ) ;
						}
					}
				break ;

				case 1 :	// func;
  	  		lcd_puts_Pleft( y, XPSTR("Function") ) ;
					lcd_putsAttIdx( 13*FW, y, XPSTR("\007-------v>val  v<val  |v|>val|v|<valv\140=val v=val  v & val|d|>valv%val=0"), pvad->func, attr ) ;	// v1>v2  v1<v2  
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->func, 9 ) ;
					}	
				break ;

				case 2 :
				{	
  	  		lcd_puts_Pleft( y, XPSTR("Value") ) ;
    			if ( ( (pvad->source > CHOUT_BASE+NUM_SKYCHNOUT) && ( pvad->source < EXTRA_POTS_START ) ) || ( pvad->source >= EXTRA_POTS_START + 8) )
		 			{
						putsTelemetryChannel( 20*FW, y, pvad->source-CHOUT_BASE-NUM_SKYCHNOUT-1, pvad->offset, attr, TELEM_NOTIME_UNIT | TELEM_UNIT | TELEM_CONSTANT ) ;
					}
					else
					{
						lcd_outdezAtt( FW*20, y, pvad->offset, attr ) ;
					}
					if ( attr )
					{
						pvad->offset = checkIncDec16( pvad->offset, -32000, 32000, EE_MODEL ) ;
					}
				}
				break ;
			
				case 3 :	 // swtch ;
  	  		lcd_puts_Pleft( y, XPSTR("Switch") ) ;
   	  		putsDrSwitches(16*FW, y, pvad->swtch, attr);
	    		if(attr)
					{
      	    CHECK_INCDEC_MODELSWITCH( pvad->swtch, -MaxSwitchIndex, MaxSwitchIndex+1 ) ;
    			}
				break ;

				case 4 :	 // rate ;
  	  		lcd_puts_Pleft( y, XPSTR("Trigger") ) ;
					displayVoiceRate( 16*FW, y, pvad->rate, attr ) ;

	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->rate, 33 ) ;
					}	
				break ;

#ifdef HAPTIC
				case 5 :	 // haptic ;
  	  		lcd_puts_Pleft( y, PSTR( STR_HAPTIC ) ) ;
					lcd_putsAttIdx( 13*FW, y, XPSTR("\007-------Haptic1Haptic2Haptic3"), pvad->haptic, attr ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->haptic, 3 ) ;
					}
				break ;
#endif
#ifndef BIG_SCREEN
			}

		}
		else
		{
			y = (i-(V1P1-1))*FH ;
	    switch(j)
			{
#endif
				case V1P1 :	 // delay
  	  		lcd_puts_Pleft( y, XPSTR("On Delay") ) ;
  				lcd_outdezAtt(FW*18-3,y,pvad->delay,attr|PREC1);
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->delay, 50 ) ;
					}
				break ;
				
				case V1P1+1 :	 // vsource:2
  	  		lcd_puts_Pleft( y, XPSTR("Play Source") ) ;
					lcd_putsAttIdx( 14*FW, y, XPSTR("\006No    BeforeAfter "), pvad->vsource, attr ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->vsource, 2 ) ;
					}
				break ;

				case V1P1+2 :
  	  		lcd_puts_Pleft( y, XPSTR("On no Telemetry") ) ;
					lcd_putsAttIdx( 17*FW, y, XPSTR("\004PlayMute"), pvad->mute, attr ) ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->mute, 1 ) ;
					}
				break ;

				case V1P1+3 :	 // fnameType:3 ;
				{	
  	  		lcd_puts_Pleft( y, XPSTR("FileType") ) ;
					lcd_putsAttIdx( 14*FW, y, XPSTR("\007-------   NameGV/SC/# EffectSysName"),pvad->fnameType,attr ) ;
					uint8_t previous = pvad->fnameType ;
	    		if(attr)
					{
      	    CHECK_INCDEC_H_MODELVAR_0( pvad->fnameType, 4 ) ;
					}
					if ( pvad->fnameType != previous )
					{
						if ( pvad->fnameType )
						{
							pvad->file.name[0] = 0 ;
							pvad->file.name[1] = 0 ;
						}
						else
						{
							pvad->file.vfile = 0 ;
						}
					}
				}
				break ;
			
				case V1P1+4 :	 // filename ;
//				if ( pvad->fnameType )
//				{	
  	  		lcd_puts_Pleft( y, pvad->fnameType == 3 ? XPSTR("Sound effect") : XPSTR("Voice File") ) ;
					if ( ( pvad->fnameType == 1 ) || ( pvad->fnameType == 4 ) )	// Name
					{
						if ( pvad->file.name[0] == 0 )
						{
							memset( (uint8_t *)pvad->file.name, ' ', sizeof(pvad->file.name) ) ;
						}
						// Need to edit name here
						alphaEditName( 12*FW, y, (uint8_t *)pvad->file.name, sizeof(pvad->file.name), attr | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FileName") ) ;
						validateName( pvad->file.name, sizeof(pvad->file.name) ) ;
	  				if( attr )
						{
							if ( checkForMenuEncoderLong( event ) )
							{
								VoiceFileType = ( pvad->fnameType == 4 ) ? VOICE_FILE_TYPE_SYSTEM : VOICE_FILE_TYPE_USER ;
      				 	pushMenu( menuProcSelectVoiceFile ) ;
							}
						} 
					}
					else if ( pvad->fnameType == 2 )	// Number
					{
						uint16_t value = pvad->file.vfile ;
						if ( value )
						{
							if ( value > 500 )
							{
								value -= 500 ;
							}
							else
							{
								value += 15 ;
							}
						}
	  	  		if(attr)
						{
	  	    		value = checkIncDec16( value, 0, 515, EE_MODEL);
						}
						if ( value )
						{
							if ( value > 15 )
							{
								value -= 15 ;
							}
							else
							{
								value += 500 ;
							}
						}
						pvad->file.vfile = value ;
						if ( pvad->file.vfile > 500)
						{
							if ( pvad->file.vfile > 507 )
							{
								lcd_putsAtt( 18*FW, y, XPSTR("SC"), attr ) ;
								lcd_putcAtt( 20*FW, y, pvad->file.vfile - 507 + '0', attr ) ;
							}
							else
							{
								dispGvar( 18*FW, y, pvad->file.vfile - 500, attr ) ;
							}
  					}
  					else
						{
	      	    lcd_outdezAtt( FW*20, y, pvad->file.vfile, attr ) ;
  					}
						if (attr)
						{
							if ( checkForMenuEncoderLong( event ) )
							{
								if ( pvad->file.vfile <= 500)
								{
									putVoiceQueue( pvad->file.vfile | VLOC_NUMUSER  ) ;
								}
							}
						}
					}
					else if ( pvad->fnameType == 3 )	// Audio
					{
						lcd_putsAttIdx(15*FW, y, PSTR(STR_SOUNDS), pvad->file.vfile, attr ) ;
		  	  	if(attr)
						{
#ifdef HAPTIC
							CHECK_INCDEC_H_MODELVAR( pvad->file.vfile, 0, 16 ) ;
#else
							CHECK_INCDEC_H_MODELVAR( pvad->file.vfile, 0, 12 ) ;
#endif
						}
					}
				break ;
//				}
				
#ifndef MOVE_VOICE
				case V1P1+5 :	 // Blank ;
  	  		lcd_puts_Pleft( y, XPSTR("Delete") ) ;
					lcd_putsAtt( 12*FW, y, XPSTR("MENU LONG"), attr ) ;
  				if( attr )
					{
						if ( checkForMenuEncoderLong( event ) )
						{
				     	deleteVoice( pvad ) ;
//							if ( i == 10 )
//							{
								mstate2.m_posVert = 0 ;
//							}
	    				eeDirty(EE_MODEL) ;
						}
					}
				break ;
			
				case V1P1+6 :	 // Copy ;
  	  		lcd_puts_Pleft( y, XPSTR("Copy") ) ;
	    		if(attr)
					{
						lcd_char_inverse( 0, y, 24, 0 ) ;
						if ( checkForMenuEncoderLong( event ) )
						{
							Clipboard.clipvoice = *pvad ;
							Clipboard.content = CLIP_VOICE ;
						}
					}
				break ;

				case V1P1+7 :	 // Paste ;
  	  		lcd_puts_Pleft( y-FH, XPSTR("\012Paste") ) ;
	    		if(attr)
					{
						lcd_char_inverse( 60, y-FH, 30, 0 ) ;
						if ( checkForMenuEncoderLong( event ) )
						{
							*pvad = Clipboard.clipvoice ;
							MuteTimer = 5 ;
						}
					}
				break ;
#endif
#ifndef BIG_SCREEN
			}
#endif
		}
	}
}

int8_t qRotary()
{
	int8_t diff = 0 ;

	if ( Rotary_diff > 0)
	{
		diff = 1 ;
	}
	else if ( Rotary_diff < 0)
	{
		diff = -1 ;
	}
	Rotary_diff = 0 ;
	return diff ;
}


// mode = 0, model alarms
// mode = 1, global alarms
void menuVoice(uint8_t event, uint8_t mode)
{
	uint32_t rows = mode ? NUM_GLOBAL_VOICE_ALARMS : NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS + 1 + 1 ;
	TITLE(PSTR(STR_Voice_Alarms)) ;
	static MState2 mstate2 ;
	
#ifdef MOVE_VOICE
	if ( s_moveMode )
	{
		int8_t moveByRotary ;
		moveByRotary = qRotary() ;		// Do this now, check_simple destroys rotary data
		if ( moveByRotary )
		{
			if ( moveByRotary > 0 )
			{
				event = EVT_KEY_FIRST(KEY_DOWN) ;
			}
			else
			{
				event = EVT_KEY_FIRST(KEY_UP) ;
			}
		}
		uint8_t v = mstate2.m_posVert ;
		if ( ( ( v == 0 ) && ( event == EVT_KEY_FIRST(KEY_UP) ) ) 
				 || ( ( v == (mode ? NUM_GLOBAL_VOICE_ALARMS - 1 : NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS - 1 ) ) && ( event == EVT_KEY_FIRST(KEY_DOWN) ) ) )
		{
			event = 0 ;
		}
		Tevent = event ;
	}
	
	if ( !PopupData.PopupActive )
	{
		mstate2.check_columns(event, rows - 1 ) ;
	}
	
#else	
	mstate2.check_columns(event, rows - 1 ) ;
#endif

  int8_t sub = mstate2.m_posVert ;

//#if defined(PCBX12D) || defined(PCBX10)
//  evalOffsetLarge(sub);
//#else
	evalOffset( sub ) ;
//#endif

  switch (event)
	{
#ifdef MOVE_VOICE
	    case EVT_ENTRY_UP:
	    case EVT_ENTRY:
        s_moveMode = false ;
  	  break;
#endif
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE) :
	    if(sub == NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
			{
    	  pushMenu(menuProcGlobalVoiceAlarm) ;
	 	    killEvents(event);
			}
			else if( sub < NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
			{
	      s_currIdx = sub + (mode ? NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS : 0) ;
#ifdef MOVE_VOICE
				s_curItemIdx = s_currIdx ;
				if ( s_moveMode )
				{
	  	  	s_moveMode = false ;
  	  		s_editMode = false ;
					RotaryState = ROTARY_MENU_UD ;
		 	    killEvents(event);
  	  		break;
				}
				// Else fall through    
				if ( !PopupData.PopupActive )
				{
					PopupData.PopupIdx = 0 ;
					PopupData.PopupActive = 1 ;
 			    killEvents(event);
					event = 0 ;		// Kill this off
				}
#else
    	  pushMenu(menuProcVoiceOne) ;
	 	    killEvents(event);
#endif
			}
		break;
  }

#ifdef MOVE_VOICE
	if ( s_moveMode )
	{
		int8_t dir ;
		uint8_t xevent = event ;
		if ( event == EVT_KEY_REPT(KEY_DOWN) )
		{
			xevent = EVT_KEY_FIRST(KEY_DOWN) ;
		}
		if ( event == EVT_KEY_REPT(KEY_UP) )
		{
			xevent = EVT_KEY_FIRST(KEY_UP) ;
		}
		
		if ( ( dir = (xevent == EVT_KEY_FIRST(KEY_DOWN) ) ) || xevent == EVT_KEY_FIRST(KEY_UP) )
		{
			moveVoice( s_curItemIdx, dir, mode ) ; //true=inc=down false=dec=up - Issue 49
			if ( mode == 0 )
			{
				if ( sub >= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
				{
					sub = mstate2.m_posVert = NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS - 1 ;
				}
			}
		}
	}
#endif

	uint8_t k = 0 ;
  uint8_t y = 1*FH ;
	for( uint8_t i = 0 ; i < ( mode ? 8 : SCREEN_LINES-1) ; i += 1 )
//  for (uint8_t i = 0 ; i < 7 ; i += 1 )
	{
    k = i + s_pgOfs ;
    uint8_t attr = sub == k ? INVERS : 0 ;
		VoiceAlarmData *pvad ;
      
//		if(y>7*FH) break ;

	  if ( k < NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
		{
    	if ( mode )
	//		k >= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
			{
	//			uint8_t z = k - ( NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ) ;
				lcd_puts_Pleft( y, XPSTR("GVA") ) ;
				lcd_putc( 3*FW, y, '1' + k ) ;
				pvad = &g_eeGeneral.gvad[k] ;
			}
			else
			{
				lcd_puts_Pleft( y, XPSTR("VA") ) ;
  		  lcd_outdezAtt( (k<9) ? FW*3-1 : FW*4-2, y, k+1, 0 ) ;
				pvad = (k >= NUM_VOICE_ALARMS) ? &g_model.vadx[k - NUM_VOICE_ALARMS] : &g_model.vad[k] ;
			}
			putsChnRaw( 5*FW, y, pvad->source, 0 ) ;
			putsDrSwitches( 9*FW, y, pvad->swtch, 0 ) ;
			displayVoiceRate( 13*FW, y, pvad->rate, 0 ) ;
    	switch ( pvad->fnameType )
			{
				case 1 :
					lcd_putc( 19*FW, y, 'N' ) ;
				break ;
				case 2 :
					lcd_putc( 19*FW, y, '#' ) ;
				break ;
				case 3 :
					lcd_putc( 19*FW, y, 'A' ) ;
				break ;
				case 4 :
					lcd_putc( 19*FW, y, 'S' ) ;
				break ;
			}
		
			if (pvad->haptic)
			{
				lcd_putc( 20*FW, y, 'H' ) ;
			}
			if ( attr )
			{
#ifdef MOVE_VOICE
				if ( s_moveMode )
				{
					lcd_rect( 0, y, 127, 8 ) ;
				}
				else
				{
					lcd_char_inverse( 0, y, 20*FW, 0 ) ;
				}
#else
				lcd_char_inverse( 0, y, 20*FW, 0 ) ;
#endif
			}
		}
		else
		{
  		if( k == NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
			{
  		  //last line available - add the global voice alarms line
  		  uint8_t attr = ( sub == NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ) ? INVERS : 0;
  		  lcd_putsAtt( 0, y, XPSTR(GvaString), attr ) ;
			}
  		else
			{
  		  lcd_puts_Pleft( y ,XPSTR("Flush Switch") ) ;
  		  uint8_t attr = ( sub == NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS + 1 ) ? InverseBlink : 0;
				g_model.voiceFlushSwitch = edit_dr_switch( 17*FW, y, g_model.voiceFlushSwitch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			}
		}
		y += FH ;
  }

#ifdef MOVE_VOICE
	s_curItemIdx = sub ;
	if ( PopupData.PopupActive )
	{
		Tevent = event ;
		voicepopup( event ) ;
    s_editMode = false;
	}
#endif
}

void menuProcVoiceAlarm(uint8_t event)
{
	EditType = EE_MODEL ;
	menuVoice( event, 0 ) ;
}

void menuProcGlobalVoiceAlarm(uint8_t event)
{
	EditType = EE_GENERAL ;
	menuVoice( event, 1 ) ;
}

static uint8_t editSlowDelay( uint8_t y, uint8_t attr, uint8_t value)
{
  if(attr)  value = checkIncDec16( value, 0, 250, EE_MODEL ); //!! bitfield
	uint8_t lval = value ;
  lcd_outdezAtt(FW*18-3,y,lval,attr|PREC1);
	return value ;
}


uint16_t extendedValueEdit( int16_t value, uint8_t extValue, uint8_t attr, uint8_t y, uint8_t event, uint16_t x )
{
	if ( (value <= -126) || (value >= 126) )
	{
		// A gvar ;
		if ( value < 0 )
		{
			value += 256 ;
		}
		value += 501 - 126 ;
	}
	else
	{
		if ( extValue == 1 )
		{
			value += 125 ; 
		}
		else if ( extValue == 3 )
		{
			value -= 125 ; 
		}
		else if ( extValue == 2 )
		{
			if ( value < 0 )
			{
				value -= 250 ;
			}
			else
			{
				value += 250 ;
			}
		}
	}
	value = gvarMenuItem( x, y, value, -125, 125, attr | GVAR_250, event ) ;
	if ( value > 500 )
	{
		value -= 501 - 126 ;
		if ( value > 128 )
		{
			value -= 256 ;
		}
		extValue = 0 ;
//		md2->weight = value ;
	}
	else
	{
		if ( value > 125 )
		{
			extValue = 1 ;
			value -= 125 ;
			if ( value > 125 )
			{
				extValue = 2 ;
				value -= 125 ;
			}
		}
		else if ( value < -125 )
		{
			extValue = 3 ;
			value += 125 ;
			if ( value < -125 )
			{
				extValue = 2 ;
				value += 125 ;
			}
		}
		else
		{
			extValue = 0 ;
//			md2->weight = lweight ;
		}
	}
	return ( value & 0xFF ) | ( extValue << 8 ) ;
}





void menuProcMixOne(uint8_t event)
{
	uint8_t numItems = 16 ;
	SKYMixData *md2 = mixAddress( s_curItemIdx ) ;
  
	static MState2 mstate2 ;
	mstate2.check_columns(event, numItems-1 ) ;
	uint8_t x = TITLEP( PSTR(STR_EDIT_MIX));

		uint32_t num_mix_switches = NUM_MIX_SWITCHES ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
		if ( g_eeGeneral.analogMapping & MASK_6POS )
		{
			num_mix_switches += 1 ;
		}
#endif
#if defined(PCBX12D) || defined(PCBX10)
		num_mix_switches += 1 ;
#endif
		if ( event == EVT_ENTRY )
		{
			RotaryState = ROTARY_MENU_UD ;
		}
		if ( event == EVT_ENTRY_UP )
		{
			SingleExpoChan = 0 ;
			if ( TextResult )
			{
				md2->srcRaw = unmapMixSource( TextIndex + 1, &md2->switchSource ) ;
	    	eeDirty(EE_MODEL) ;
			}
		}
		
		putsChn(x+1*FW,0,md2->destCh,0);
    int8_t  sub    = mstate2.m_posVert;

//#if defined(PCBX12D) || defined(PCBX10)
//    evalOffsetLarge(sub);
//#else
    evalOffset(sub);
//#endif

    for(uint8_t k=0; k<SCREEN_LINES-1 ; k++)
    {
      uint8_t y = (k+1) * FH;
      uint8_t i = k + s_pgOfs;
      uint8_t attr = sub==i ? InverseBlink : 0;
  		uint8_t b ;

      switch(i)
			{
        case 0:
				{	
          lcd_puts_P(  2*FW,y,PSTR(STR_2SOURCE));
					uint32_t value = md2->srcRaw ;
					putsChnOpRaw( FW*14,y, value, md2->switchSource, md2->disableExpoDr, attr ) ;
					if ( attr )
					{
						uint8_t x = mapMixSource( value, md2->switchSource ) ;
#if defined(PCBX7) || defined (PCBXLITE) || defined (PCBX9LITE)
						CHECK_INCDEC_H_MODELVAR( x, 1,NUM_SKYXCHNRAW+1+MAX_GVARS+1+NUM_SCALERS+8+NumExtraPots + (num_mix_switches-1) + EXTRA_SKYCHANNELS-1+4 ) ;
#else // PCBX7
						CHECK_INCDEC_H_MODELVAR( x, 1,NUM_SKYXCHNRAW+1+MAX_GVARS+1+NUM_SCALERS+8+NumExtraPots + (num_mix_switches-1) + EXTRA_SKYCHANNELS+4 ) ;
#endif // PCBX7
//						if ( md2->srcRaw > 4 )
//						{
							if ( ( event == EVT_KEY_BREAK(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
							{
								// Long MENU pressed
								TextIndex = x ;
  						  TextType = 2 ;
								TextOption = md2->disableExpoDr ;
  						  killEvents(event) ;
								pushMenu(menuTextHelp) ;
							}
//						}	
						md2->srcRaw = unmapMixSource( x, &md2->switchSource ) ;
						
  					if ( checkForMenuEncoderLong( event ) )
						{
							if ( ( md2->srcRaw) && ( md2->srcRaw <= 4 ) )
							{
								SingleExpoChan = 1 ;
								TextResult = 0 ;
								s_expoChan = md2->srcRaw-1 ;
        				pushMenu(menuProcExpoAll);
							}
						}
					}
				}
        break ;
        case 1:
				{	
          lcd_puts_P(  2*FW,y,PSTR(STR_2WEIGHT));
					uint16_t result = extendedValueEdit( md2->weight, md2->extWeight, attr, y, event, FW*16 ) ;
					md2->weight = result & 0x00FF ;
					md2->extWeight = result >> 8 ;
//					int16_t lweight = md2->weight ;
//					if ( (lweight <= -126) || (lweight >= 126) )
//					{
//						// A gvar ;
//						if ( lweight < 0 )
//						{
//							lweight += 256 ;
//						}
//						lweight += 501 - 126 ;
//					}
//					else
//					{
//						if ( md2->extWeight == 1 )
//						{
//							lweight += 125 ; 
//						}
//						else if ( md2->extWeight == 3 )
//						{
//							lweight -= 125 ; 
//						}
//					}
//					lweight = gvarMenuItem( FW*16, y, lweight, -125, 125, attr | GVAR_250, event ) ;
//					if ( lweight > 500 )
//					{
//						lweight -= 501 - 126 ;
//						if ( lweight > 128 )
//						{
//							lweight -= 256 ;
//						}
//						md2->extWeight = 0 ;
//						md2->weight = lweight ;
//					}
//					else
//					{
//						if ( lweight > 125 )
//						{
//							md2->extWeight = 1 ;
//							md2->weight = lweight - 125 ;
//						}
//						else if ( lweight < -125 )
//						{
//							md2->extWeight = 3 ;
//							md2->weight = lweight + 125 ;
//						}
//						else
//						{
//							md2->extWeight = 0 ;
//							md2->weight = lweight ;
//						}
//					}
				}
        break;
        case 2:
				{	
          lcd_puts_P( FW, y, PSTR(STR_OFFSET) ) ;
					uint16_t result = extendedValueEdit( md2->sOffset, md2->extOffset, attr, y, event, FW*16 ) ;
					md2->sOffset = result & 0x00FF ;
					md2->extOffset = result >> 8 ;
//					md2->sOffset = gvarMenuItem( FW*16, y, md2->sOffset, -125, 125, attr, event ) ;
				}
        break;
        case 3:
						md2->lateOffset = onoffMenuItem( md2->lateOffset, y, PSTR(STR_2FIX_OFFSET), attr ) ;
            break;
        case 4:
						if ( ( md2->srcRaw <=4 ) )
						{
    					lcd_puts_Pleft( y, PSTR(STR_ENABLEEXPO) ) ;
							md2->disableExpoDr = offonItem( md2->disableExpoDr, y, attr ) ;
						}
						else
						{
							md2->disableExpoDr = onoffMenuItem( md2->disableExpoDr, y, XPSTR("\001Use Output   "), attr ) ;
						}
            break;
        case 5:
						md2->carryTrim = offonMenuItem( md2->carryTrim, y, PSTR(STR_2TRIM), attr ) ;
            break;
        
				case 6:
					{	
					 	uint8_t value = md2->differential ;
						if ( value == 0 )
						{
							if ( md2->curve <= -28 )
							{
								value = 2 ;		// Expo
							}
						}
	          lcd_putsAtt(  1*FW, y, PSTR(STR_Curve), (value == 0) ? attr : 0 ) ;
	          lcd_putsAtt(  1*FW, y, PSTR(STR_15DIFF), (value == 1) ? attr : 0 ) ;
	          lcd_putsAtt(  1*FW, y, XPSTR("\021Expo"), (value == 2) ? attr : 0 ) ;
					 	uint8_t value2 = value ;
    		    if(attr) CHECK_INCDEC_H_MODELVAR_0( value2, 2) ;
					 	if ( value != value2 )
						{
							if ( value2 == 2 )
							{
								md2->curve = -128 ;
							}
							else
							{
								md2->curve = 0 ;
							}
							md2->differential = value2 & 1 ;	// 0 and 2 turn it off
						}
					}
        break ;

        case 7:
						if ( md2->differential )		// Non zero for curve
						{	
		          md2->curve = gvarMenuItem( 12*FW, y, md2->curve, -100, 100, attr /*( m_posHorz==1 ? attr : 0 )*/, event ) ;
						}
						else
						{
							if ( md2->curve <= -28 )
							{
								int8_t value = md2->curve + 128 ;	// 0 to 100
            		lcd_outdezAtt(FW*17,y,value,attr|LEFT);
            		if(attr) CHECK_INCDEC_H_MODELVAR_0( value, 100 ) ;
								md2->curve = value - 128 ;
							}
							else
							{
								put_curve( 2*FW, y, md2->curve, attr ) ;
          	  	if(attr)
								{
        		  		CHECK_INCDEC_H_MODELVAR( md2->curve, -MAX_CURVE5-MAX_CURVE9-1-1-1, MAX_CURVE5+MAX_CURVE9+7-1+1+1+1);
									if ( event==EVT_KEY_FIRST(KEY_MENU) || event == EVT_KEY_BREAK(BTN_RE)  )
									{
										if ( md2->curve>=CURVE_BASE )
										{
          	  	    	s_curveChan = md2->curve-CURVE_BASE;
	          		      pushMenu(menuProcCurveOne);
										}
										if ( md2->curve < 0 )
										{
          	  	    	s_curveChan = -md2->curve-1 ;
	          		      pushMenu(menuProcCurveOne);
										}
									}
          	  	}
        		  }
						}
        break ;

        case 8:
            lcd_puts_P(  2*FW,y,PSTR(STR_2SWITCH));
            putsDrSwitches(13*FW,  y,md2->swtch,attr);
            if(attr) CHECK_INCDEC_MODELSWITCH( md2->swtch, -MaxSwitchIndex, MaxSwitchIndex);
            break;

        case 9:
					{	
						uint8_t b = 1 ;
						lcd_puts_P( FW, y, PSTR(STR_MODES) ) ;
//            lcd_puts_Pleft( y,XPSTR("\001MODES"));
						
						if ( attr )
						{
							Columns = 6 ;
						}
  					
						for ( uint8_t p = 0 ; p<MAX_MODES+1 ; p++ )
						{
							uint8_t z = md2->modeControl ;
    					lcd_putcAtt( (9+p)*(FW+1), y, '0'+p, ( z & b ) ? 0 : INVERS ) ;
							if( attr && ( g_posHorz == p ) )
							{
								lcd_rect( (9+p)*(FW+1)-1, y-1, FW+2, 9 ) ;
								if ( event==EVT_KEY_BREAK(KEY_MENU) || event==EVT_KEY_BREAK(BTN_RE) ) 
								{
									md2->modeControl ^= b ;
      						eeDirty(EE_MODEL) ;
    							s_editMode = false ;
								}
							}
							b <<= 1 ;
						}
					}
        break ;

        case 10:
            lcd_puts_P(  2*FW,y,PSTR(STR_2WARNING));
						b = md2->mixWarn ;
            if(b)
                lcd_outdezAtt(FW*13,y,b,attr|LEFT);
            else
                lcd_putsAtt(  FW*13,y,PSTR(STR_OFF),attr);
            if(attr) { CHECK_INCDEC_H_MODELVAR_0( b, 3); md2->mixWarn = b ; }
            break;
        case 11:
            lcd_puts_P(  2*FW,y,PSTR(STR_2MULTIPLEX));
            lcd_putsAttIdx(13*FW, y,PSTR(STR_ADD_MULT_REP),md2->mltpx,attr);
            if(attr) CHECK_INCDEC_H_MODELVAR_0( md2->mltpx, 2); //!! bitfield
            break;
        case 12:
            lcd_puts_P(  2*FW,y,PSTR(STR_2DELAY_DOWN));
						md2->delayUp = editSlowDelay( y, attr, md2->delayUp ) ;	// Sense was wrong
            break;
        case 13:
            lcd_puts_P(  2*FW,y,PSTR(STR_2DELAY_UP));
						md2->delayDown = editSlowDelay( y, attr, md2->delayDown ) ;	// Sense was wrong
            break;
        case 14:
            lcd_puts_P(  2*FW,y,PSTR(STR_2SLOW_DOWN));
						md2->speedDown = editSlowDelay( y, attr, md2->speedDown ) ;
            break;
        case 15:
            lcd_puts_P(  2*FW,y,PSTR(STR_2SLOW_UP));
						md2->speedUp = editSlowDelay( y, attr, md2->speedUp ) ;
            break;
      }
    }


}

int8_t s_mixMaxSel;

void moveMix(uint8_t idx, uint8_t dir) //true=inc=down false=dec=up - Issue 49
{
		SKYMixData *src = mixAddress( idx ) ;
    if(idx==0 && !dir)
		{
      if (src->destCh>0)
			{
			  src->destCh--;
			}
			STORE_MODELVARS;
			return ;
		}

#if EXTRA_SKYMIXERS
    if(idx>MAX_SKYMIXERS+EXTRA_SKYMIXERS || (idx==MAX_SKYMIXERS+EXTRA_SKYMIXERS && dir)) return ;
#else
    if(idx>MAX_SKYMIXERS || (idx==MAX_SKYMIXERS && dir)) return ;
#endif
    uint8_t tdx = dir ? idx+1 : idx-1;
		SKYMixData *tgt = mixAddress( tdx ) ;

    if((src->destCh==0) || (src->destCh>NUM_SKYCHNOUT+EXTRA_SKYCHANNELS) || (tgt->destCh>NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)) return;

    if(tgt->destCh!=src->destCh)
		{
        if ((dir)  && (src->destCh<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS)) src->destCh++;
        if ((!dir) && (src->destCh>0))          src->destCh--;
				STORE_MODELVARS;
        return;
    }

    //flip between idx and tgt
    memswap( tgt, src, sizeof(SKYMixData) ) ;
		s_moveItemIdx = tdx ;
    
		STORE_MODELVARS;
}

void menuMixersLimit(uint8_t event)
{
    switch(event)
    {
    case  EVT_KEY_FIRST(KEY_EXIT):
        killEvents(event);
        popMenu(true);
        pushMenu(menuProcMix);
        break;
    }
    lcd_puts_Pleft(2*FH, PSTR(STR_MAX_MIXERS));
    lcd_outdezAtt(20*FW, 2*FH, getMixerCount(),0);

    lcd_puts_Pleft(4*FH, PSTR(STR_PRESS_EXIT_AB));
}

uint8_t getMixerCount()
{
    uint8_t mixerCount = 0;
    uint8_t dch ;

#if EXTRA_SKYMIXERS
    for(uint8_t i=0;i<MAX_SKYMIXERS+EXTRA_SKYMIXERS;i++)
#else		
		for(uint8_t i=0;i<MAX_SKYMIXERS;i++)
#endif
    {
        dch = mixAddress( i )->destCh ;
        if ((dch!=0) && (dch<=NUM_SKYCHNOUT+EXTRA_SKYCHANNELS))
        {
            mixerCount++;
        }
    }
    return mixerCount;
}

bool reachMixerCountLimit()
{
    // check mixers count limit
#if EXTRA_SKYMIXERS
    if (getMixerCount() >= MAX_SKYMIXERS+EXTRA_SKYMIXERS)
#else
    if (getMixerCount() >= MAX_SKYMIXERS)
#endif
    {
        pushMenu(menuMixersLimit);
        return true;
    }
    else
    {
        return false;
    }
}

uint8_t mixToDelete;

#define YN_NONE	0
#define YN_YES	1
#define YN_NO		2

uint8_t yesNoMenuExit( uint8_t event, const char * s )
{
	uint8_t reply = YN_NONE ;
	lcd_puts_Pleft(1*FH, s ) ;	
  lcd_puts_Pleft( 5*FH,PSTR(STR_YES_NO));
  lcd_puts_Pleft( 6*FH,PSTR(STR_MENU_EXIT));

  switch(event)
	{
    case EVT_ENTRY:
      audioDefevent(AU_WARNING1);
    break ;

    case EVT_KEY_FIRST(KEY_MENU):
		case EVT_KEY_BREAK(BTN_RE):
			reply = YN_YES ;
    break ;

		case EVT_KEY_LONG(BTN_RE):
			if ( g_eeGeneral.disableBtnLong )
			{
				break ;
			}		
    case EVT_KEY_FIRST(KEY_EXIT):
			reply = YN_NO ;
    break;
  }
	if ( reply != YN_NONE )
	{
		killEvents(event);
    popMenu(false);
	}
	return reply ;
}


void menuDeleteMix(uint8_t event)
{
	uint8_t action ;
	action = yesNoMenuExit( event, (mixToDelete== 0xFF) ? XPSTR("Clear ALL mixes?") : PSTR(STR_DELETE_MIX) ) ;
  
	switch( action )
	{
    case YN_YES :
			if (mixToDelete == 0xFF)
			{
        clearMixes() ;
			}
			else
			{
      	deleteMix(mixToDelete);
			}	 
      //fallthrough
		case YN_NO :
//      pushMenu(menuProcMix);
    break;
  }
}


static uint8_t popupProcess( uint8_t event, uint8_t max )
{
	int8_t popidxud = qRotary() ;
	uint8_t popidx = PopupData.PopupIdx ;

	if ( PopupData.PopupTimer )
	{
		if ( BLINK_ON_PHASE )
		{
			if ( --PopupData.PopupTimer == 0 )
			{
				// Timeout popup
				PopupData.PopupActive = 0 ;
				return POPUP_EXIT ;
			}
		}
	}
	else
	{
		PopupData.PopupTimer = 255 ;
  }

//  event = Tevent ;
	switch(event)
	{
    case EVT_KEY_BREAK(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE):
			PopupData.PopupActive = 0 ;
			PopupData.PopupTimer = 0 ;
		return POPUP_SELECT ;
    
		case EVT_KEY_FIRST(KEY_UP) :
			popidxud = -1 ;
		break ;
    
		case EVT_KEY_FIRST(KEY_DOWN) :
			popidxud = 1 ;
		break ;
    
		case EVT_KEY_LONG(BTN_RE) :
			if ( g_eeGeneral.disableBtnLong )
			{
				break ;
			}		
		case EVT_KEY_BREAK(KEY_EXIT) :
			killEvents( event ) ;
			Tevent = 0 ;
			PopupData.PopupActive = 0 ;
			PopupData.PopupTimer = 0 ;
		return POPUP_EXIT ;
	}

	if (popidxud > 0)
	{
		if ( popidx < max )
		{
			popidx += 1 ;
		}
		else
		{
			popidx = 0 ;			
		}
	}
	else if (popidxud < 0)
	{		
		if ( popidx )
		{
			popidx -= 1 ;
		}
		else
		{
			popidx = max ;
		}
	}

	if (popidxud )
	{
		if ( PopupData.PopupTimer )
		{
			PopupData.PopupTimer = 255 ;
		}
	}	

	PopupData.PopupIdx = popidx ;
	return POPUP_NONE ;
}

//extern uint16_t LCDLastOp ;

#if defined(PCBX12D) || defined(PCBX10)
static uint32_t popupDisplay( const char *list, uint16_t mask, uint8_t width )
{
	uint32_t entries = 0 ;
	uint8_t y = FH ;

	while ( mask )
	{
		if ( mask & 1 )
		{
			uint32_t len = strlen( list ) ;
//			lcd_puts_P( 3*FW + X12OFFSET, y, " " ) ;
			lcd_puts_P( 4*FW + X12OFFSET, y, (const char *)(list) ) ;
			lcd_putsn_P( (4+len)*FW + X12OFFSET, y, "              ", width-len-1 ) ;
			entries += 1 ;
			y += FH ;
		}
		mask >>= 1 ;
		while ( *list )
		{
			list += 1 ;			
		}		
		list += 1 ;			
	}
//	if ( y >= 7*FH )
//	{
//		y -= 1 ;
//	}
//	LCDLastOp = 'N' ;
	lcdDrawSolidFilledRectDMA( (3*FW + X12OFFSET)*2, 0, 12, y*2, LcdBackground ) ;
//	LCDLastOp = 'O' ;
	lcdDrawSolidFilledRectDMA( (3*FW + X12OFFSET)*2, 1, width*FW*2, 15, LcdBackground ) ;
//	lcd_rect( 3*FW + X12OFFSET, 0, width*FW, y+1 ) ;
//	LCDLastOp = 'P' ;
	lcdDrawSolidFilledRectDMA( (3*FW + X12OFFSET)*2, 0, width*FW*2, 2, LCD_BLACK) ;
//	LCDLastOp = 'Q' ;
	lcdDrawSolidFilledRectDMA( (3*FW + X12OFFSET)*2, y*2, width*FW*2, 2, LCD_BLACK) ;
//	LCDLastOp = 'R' ;
	lcdDrawSolidFilledRectDMA( (3*FW + X12OFFSET)*2, 0, 2, y*2, LCD_BLACK) ;
//	LCDLastOp = 'S' ;
	lcdDrawSolidFilledRectDMA( (3*FW + X12OFFSET+width*FW)*2, 0, 2, y*2, LCD_BLACK) ;
	lcd_char_inverse( 4*FW + X12OFFSET, (PopupData.PopupIdx+1)*FH, (width-2)*FW, 0 ) ;

	return entries ;
}
#else
static uint32_t popupDisplay( const char *list, uint16_t mask, uint8_t width )
{
	uint32_t entries = 0 ;
	uint8_t y = 0 ;

	while ( mask )
	{
		if ( mask & 1 )
		{
			lcd_putsn_P( 3*FW + X12OFFSET, y, "                ", width ) ;
			lcd_puts_P( 4*FW + X12OFFSET, y, (const char *)(list) ) ;
			entries += 1 ;
			y += FH ;
		}
		mask >>= 1 ;
		while ( *list )
		{
			list += 1 ;			
		}		
		list += 1 ;			
	}
	if ( y >= 7*FH )
	{
		y -= 1 ;
	}
	lcd_rect( 3*FW + X12OFFSET, 0, width*FW, y+1 ) ;
	lcd_char_inverse( 4*FW + X12OFFSET, (PopupData.PopupIdx)*FH, (width-2)*FW, 0 ) ;

	return entries ;
}
#endif

static uint8_t popTranslate( uint8_t popidx, uint16_t mask )
{
	uint8_t position ;
	popidx += 1 ;
	for ( position = 0 ; position < 16 ; position += 1 )
	{
		if ( mask & 1 )
		{
			if ( --popidx == 0)
			{
				break ;
			}
		}
		mask >>= 1 ;
	}
	return position ;
}


uint32_t doPopup( const char *list, uint16_t mask, uint8_t width, uint8_t event )
{
	CPU_UINT count = popupDisplay( list, mask, width ) ;
	uint8_t popaction = popupProcess( event, count - 1 ) ;
	uint8_t popidx = PopupData.PopupIdx ;
	PopupData.PopupSel = popTranslate( popidx, mask ) ;
	return popaction ;
}

void mixpopup( uint8_t event )
{
	uint8_t popaction = doPopup( PSTR(STR_MIX_POPUP), 0x7F, 11, event ) ;
	
  if ( popaction == POPUP_SELECT )
	{
		uint8_t popidx = PopupData.PopupSel ;
		if ( popidx == 1 )
		{
      if ( !reachMixerCountLimit())
      {
				s_currMixInsMode = 1 ;
      	insertMix(++s_curItemIdx, 0 ) ;
  	    s_moveMode=false;
			}
		}
		if ( popidx < 2 )
		{
	    pushMenu(menuProcMixOne) ;
		}
		else if ( popidx == 4 )		// Delete
		{
			mixToDelete = s_curItemIdx;
			killEvents(event);
			Tevent = 0 ;
			pushMenu(menuDeleteMix);
		}
		else if ( popidx == 5 )		// Clear all
		{
			mixToDelete = 0xFF ;
			killEvents(event);
			Tevent = 0 ;
			pushMenu(menuDeleteMix);
		}
		else if ( popidx == 6 )		// Templates
		{
			pushMenu( menuProcTemplates ) ;
		}
		else
		{
			if( popidx == 2 )	// copy
			{
     		insertMix(++s_curItemIdx, 1 ) ;
			}
			// PopupIdx == 2 or 3, copy or move
			s_moveMode = 1 ;
		}
		PopupData.PopupActive = 0 ;
	}
	s_moveItemIdx = s_curItemIdx ;

}



void menuProcMix(uint8_t event)
{
	TITLE(PSTR(STR_MIXER));
	EditType = EE_MODEL ;
	static MState2 mstate2;
	
	if ( s_moveMode )
	{
		int8_t moveByRotary ;
		moveByRotary = qRotary() ;		// Do this now, check_simple destroys rotary data
		if ( moveByRotary )
		{
			if ( moveByRotary > 0 )
			{
				event = EVT_KEY_FIRST(KEY_DOWN) ;
			}
			else
			{
				event = EVT_KEY_FIRST(KEY_UP) ;
			}
		}
		uint8_t v = mstate2.m_posVert ;
		if ( ( ( v == 0 ) && ( event == EVT_KEY_FIRST(KEY_UP) ) ) 
				 || ( ( v == s_mixMaxSel ) && ( event == EVT_KEY_FIRST(KEY_DOWN) ) ) )
		{
			event = 0 ;
		}
		Tevent = event ;
	}
	
	if ( !PopupData.PopupActive )
	{
		mstate2.check_columns(event,s_mixMaxSel-1) ;
	}

  int8_t  sub    = mstate2.m_posVert;
	int8_t	menulong = 0 ;

    switch(event)
    {
	    case EVT_ENTRY:
        s_moveMode=false;
  	  break;
    
			case EVT_KEY_FIRST(KEY_MENU):
	    case EVT_KEY_BREAK(BTN_RE):
				if ( s_moveMode )
				{
	  	  	s_moveMode = false ;
  	  		s_editMode = false ;
					RotaryState = ROTARY_MENU_UD ;
  	  		break;
				}
				// Else fall through    
				if ( !PopupData.PopupActive )
				{
		  		killEvents(event);
					Tevent = 0 ;			// Prevent changing weight to/from Gvar
					menulong = 1 ;
				}
  	  break;
    }

//#if defined(PCBX12D) || defined(PCBX10)
//    uint8_t t_pgOfs = evalOffsetLarge(sub);
//#else
		uint8_t t_pgOfs = evalOffset( sub ) ;
//#endif
    
	if ( PopupData.PopupActive )
	{
		Tevent = 0 ;
	}
		
  uint8_t mix_index = 0 ;
  uint8_t current = 0 ;

	if ( s_moveMode )
	{
		int8_t dir ;
		
		if ( ( dir = (event == EVT_KEY_FIRST(KEY_DOWN) ) ) || event == EVT_KEY_FIRST(KEY_UP) )
		{
			moveMix( s_curItemIdx, dir ) ; //true=inc=down false=dec=up - Issue 49
		}
	}

  for ( uint8_t chan=1 ; chan <= NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ; chan += 1 )
	{
    SKYMixData *pmd = mixAddress( mix_index ) ;
    
    if ( t_pgOfs <= current && current-t_pgOfs < SCREEN_LINES-1)
		{
      putsChn(1, (current-t_pgOfs+1)*FH, chan, 0) ; // show CHx
    }

		uint8_t firstMix = mix_index ;
		
#if EXTRA_SKYMIXERS
		if (mix_index < MAX_SKYMIXERS + EXTRA_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)
#else
		if (mix_index<MAX_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)
#endif
		{
    	do
			{
				if (t_pgOfs <= current )
				{
					if ( current-t_pgOfs < SCREEN_LINES-1 )
					{
    	  	  uint8_t y = (current-t_pgOfs+1)*FH ;
    				uint8_t attr = 0 ;

						if ( !s_moveMode && (sub == current) )
						{
							s_curItemIdx = mix_index ;
							s_currDestCh = chan ;		// For insert
							if ( menulong )
							{
								PopupData.PopupIdx = 0 ;
								PopupData.PopupActive = 1 ;
								event = 0 ;		// Kill this off
							}
							if ( PopupData.PopupActive == 0 )
							{
    						attr = INVERS ;
							}
						}
        	  if(firstMix != mix_index) //show prefix only if not first mix
        	 		lcd_putsAttIdx( 4*FW-8, y, XPSTR("\001+*R"),pmd->mltpx,0 ) ;

						putsChnOpRaw( 8*FW ,y, pmd->srcRaw, pmd->switchSource, pmd->disableExpoDr, attr ) ;
						if ( attr )
						{
							uint32_t value = g_chans512[chan-1] ;
							singleBar( 96, 4, value ) ;
					    lcd_outdez( 11*FW, 0, value/2 + 1500 ) ;
						}
//						putsChnOpRaw( 8*FW, y, pmd, 0 ) ;
extern uint8_t swOn[] ;
						attr = 0 ;
						if ( swOn[mix_index] || pmd->srcRaw == MIX_MAX || pmd->srcRaw == MIX_FULL )
						{
							attr |= BOLD ;
						}
						int16_t lweight = pmd->weight ;
						if ( (lweight <= -126) || (lweight >= 126) )
						{
							// A gvar ;
							if ( lweight < 0 )
							{
								lweight += 256 ;
							}
							lweight += 501 - 126 ;
						}
						else
						{
							if ( pmd->extWeight == 1 )
							{
								lweight += 125 ; 
							}
							else if ( pmd->extWeight == 3 )
							{
								lweight -= 125 ; 
							}
							else if ( pmd->extWeight == 2 )
							{
								if ( lweight < 0 )
								{
									lweight -= 250 ;
								}
								else
								{
									lweight += 250 ;
								}
							}
						}
						gvarMenuItem( 7*FW+FW/2, y, lweight, -125, 125, attr | GVAR_250, 0 ) ;
//						pmd->weight = gvarMenuItem( 7*FW+FW/2, y, pmd->weight, -125, 125, attr | GVAR_250, 0 ) ;

#ifndef PCBX12D
 #ifndef PCBX10
					 if ( g_eeGeneral.altMixMenu )
 #endif
#endif
					 {
#if defined(PCBX12D) || defined(PCBX10)
					 	uint16_t x = 128-76 ;
#else
					 	uint16_t x = 0 ;
#endif
						if (sub == current)
						{
#if defined(PCBX12D) || defined(PCBX10)
							pushPlotType( PLOT_COLOUR ) ;
#endif
							lcd_vline( 78+x, 8, 55 ) ;
#if defined(PCBX12D) || defined(PCBX10)
							popPlotType() ;
#endif
							lcd_puts_P( 13*FW+2+x, FH, XPSTR("Ofst") ) ;
							extendedValueEdit( pmd->sOffset, pmd->extOffset, 0, FH, 0, FW*21+1+x ) ;

							lcd_puts_P( 13*FW+2+x, 2*FH, XPSTR("Sw") ) ;
	            putsDrSwitches( 17*FW+2+x, 2*FH , pmd->swtch, 0 ) ;

// ??? Curve setting, Flight modes
							
						 	uint8_t value = pmd->differential ;
							if ( value == 0 )
							{
								if ( pmd->curve <= -28 )
								{
									value = 2 ;		// Expo
								}
							}
//							if ( value || pmd->curve )
//							{
								lcd_putsAttIdx( 13*FW+2+x, 3*FH, XPSTR("\003CveDifExp"), value, 0 ) ;
								switch ( value )
								{
									case 0 :
										put_curve( 18*FW+2+x, 3*FH, pmd->curve, 0 ) ;
									break ;
									case 1 :
				      	    gvarMenuItem( FW*21+1+x, 3*FH, pmd->curve, -100, 100, 0, 0 ) ;
									break ;
									case 2 :
            			lcd_outdezAtt( FW*21+1+x, 3*FH, pmd->curve + 128, 0 ) ;
									break ;
								}
//							}
							uint8_t b = 1 ;
							uint8_t z = pmd->modeControl ;
							lcd_puts_P( 13*FW+2+x, 4*FH, XPSTR("M") ) ;
							uint32_t modeCount = 0 ;
							uint32_t modeIndex = 0 ;
							for ( uint8_t p = 0 ; p<MAX_MODES+1 ; p++ )
							{
								if ( ( z & b ) == 0 )
								{
    							lcd_putcAtt( (14+p)*FW+2+x, 4*FH, '0'+p, 0 ) ;
									modeCount += 1 ;
									modeIndex = p ;
								}
								b <<= 1 ;
							}
							if ( modeCount == 1 )
							{
								if ( modeIndex )
								{
									modeIndex -= 1 ;
									if ( g_model.phaseData[modeIndex].name[0] )
									{
										lcd_puts_P( 13*FW+2+x, 6*FH, XPSTR("[      ]") ) ;
										lcd_putsnAtt( 14*FW+2+x, 5*FH, g_model.phaseData[modeIndex].name, 6, /*BSS*/ 0 ) ;
									}
								}
							}
							lcd_puts_P( 13*FW+2+x, 6*FH, XPSTR("D    :") ) ;
						  lcd_outdezAtt( FW*18+2+x, 6*FH, pmd->delayUp, PREC1 ) ;
						  lcd_outdezAtt( FW*21+1+x, 6*FH, pmd->delayDown, PREC1 ) ;

							lcd_puts_P( 13*FW+2+x, 7*FH, XPSTR("S    :") ) ;
						  lcd_outdezAtt( FW*18+2+x, 7*FH, pmd->speedDown, PREC1 ) ;
						  lcd_outdezAtt( FW*21+1+x, 7*FH, pmd->speedUp, PREC1 ) ;
						}					 	
						if ( s_moveMode )
						{
							if ( s_moveItemIdx == mix_index )
							{
								lcd_char_inverse( 4*FW+x, y, 76-4*FW, 0 ) ;
								s_curItemIdx = mix_index ;
								sub = mstate2.m_posVert = current ;
							}
						}
					 }
#ifndef PCBX12D
#ifndef PCBX10
					 else
#endif
#endif
					 {
    	  	  if( pmd->swtch) putsDrSwitches( 12*FW, y, pmd->swtch, 0 ) ; //tattr);
						if(pmd->curve)
						{
							if ( pmd->differential ) lcd_putcAtt(    16*FW, y, *PSTR(CHR_d), 0 ) ;
							else
							{
								if ( ( pmd->curve > -28 ) && ( pmd->curve <= 27 ) )
								{
	    	  	  		put_curve( 16*FW, y, pmd->curve, 0 ) ;
								}
								else
								{
									if ( pmd->curve != -128 )
									{
										lcd_putcAtt( 16*FW, y, 'E', 0 ) ;
									}
								}
							}
						}
						char cs = ' ';
        	  if (pmd->speedDown || pmd->speedUp)
        	    cs = *PSTR(CHR_S);
        	  if (pmd->delayUp || pmd->delayDown)
        	    cs = (cs == *PSTR(CHR_S) ? '*' : *PSTR(CHR_D));
        	  lcd_putc(20*FW+1, y, cs ) ;

						if ( s_moveMode )
						{
							if ( s_moveItemIdx == mix_index )
							{
								lcd_char_inverse( 4*FW, y, 17*FW, 0 ) ;
								s_curItemIdx = mix_index ;
								sub = mstate2.m_posVert = current ;
							}
						}
					 }
					}
					else
					{
						if ( current-t_pgOfs == 7 )
						{
							if ( s_moveMode )
							{
								if ( s_moveItemIdx == mix_index )
								{
									mstate2.m_posVert += 1 ;								
								}
							}
						}
					}
				}
				current += 1 ;
				mix_index += 1 ;
		    pmd = mixAddress( mix_index ) ;
#if EXTRA_SKYMIXERS
    	} while ( (mix_index<MAX_SKYMIXERS+EXTRA_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)) ;
#else
    	} while ( (mix_index<MAX_SKYMIXERS && /* pmd->srcRaw && */ pmd->destCh == chan)) ;
#endif
		}
		else
		{
			if (sub == current)
			{
				s_currDestCh = chan ;		// For insert
				s_curItemIdx = mix_index ;
				lcd_rect( 0, (current-t_pgOfs+1)*FH-1, 25, 9 ) ;
				if ( menulong )		// Must need to insert here
				{
      		if ( !reachMixerCountLimit())
      		{
      			insertMix(s_curItemIdx, 0 ) ;
  	    		s_moveMode=false;
	      		pushMenu(menuProcMixOne) ;
						break ;
      		}
				}
			}
			current += 1 ;
		}
	}
//  if ( t_pgOfs <= current && (current-t_pgOfs) < 7)
//	{
//   	lcd_puts_Pleft( 7*FH,XPSTR("Templates\020MENU") ) ;
//		if (sub == s_mixMaxSel )
//		{
//			lcd_char_inverse( 0, 7*FH, 21*FW, 0 ) ;
//			if ( event == EVT_KEY_FIRST(KEY_MENU) || event == EVT_KEY_BREAK(BTN_RE) )
//			{
//				pushMenu( menuProcTemplates ) ;
//			}
//		}
//	}
	
	if ( PopupData.PopupActive )
	{
		Tevent = event ;
		mixpopup( event ) ;
    s_editMode = false;
	}
	s_mixMaxSel = current ;

}


uint16_t expou(uint16_t x, uint16_t k)
{
    // k*x*x*x + (1-k)*x
    return ((unsigned long)x*x*x/0x10000*k/(RESXul*RESXul/0x10000) + (RESKul-k)*x+RESKul/2)/RESKul;
}
// expo-funktion:
// ---------------
// kmplot
// f(x,k)=exp(ln(x)*k/10) ;P[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]
// f(x,k)=x*x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=1+(x-1)*(x-1)*(x-1)*k/10 + (x-1)*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]

int16_t expo(int16_t x, int16_t k)
{
    if(k == 0) return x;
    int32_t   y;
    bool    neg =  x < 0;
    if(neg)   x = -x;
    if(k<0){
        y = RESXu-expou(RESXu-x,-k);
    }else{
        y = expou(x,k);
    }
    return neg? -y:y;
}


#ifdef EXTENDED_EXPO
/// expo with y-offset
class Expo
{
    uint16_t   c;
    int16_t    d,drx;
public:
    void     init(uint8_t k, int8_t yo);
    static int16_t  expou(uint16_t x,uint16_t c, int16_t d);
    int16_t  expo(int16_t x);
};
void    Expo::init(uint8_t k, int8_t yo)
{
    c = (uint16_t) k  * 256 / 100;
    d = (int16_t)  yo * 256 / 100;
    drx = d * ((uint16_t)RESXu/256);
}
int16_t Expo::expou(uint16_t x,uint16_t c, int16_t d)
{
    uint16_t a = 256 - c - d;
    if( (int16_t)a < 0 ) a = 0;
    uint32_t res =  ((uint32_t)x * x * x / 0x10000 * a / (RESXul*RESXul/0x10000) +
                     (uint32_t)x                   * c
                     ) / 256;
    return (int16_t)res;
}
int16_t  Expo::expo(int16_t x)
{
    if(c==256 && d==0) return x;
    if(x>=0) return expou(x,c,d) + drx;
    return -expou(-x,c,-d) + drx;
}
#endif


void editExpoVals(uint8_t event, uint8_t edit, uint8_t x, uint8_t y, uint8_t which, uint8_t exWt, uint8_t stkRL)
{
    uint8_t  invBlk = (edit) ? InverseBlink : 0 ;
		uint8_t doedit ;
		int8_t *ptr ;			// volatile forces compiler to produce 'better' code
		ExpoData *eptr ;

		doedit = edit ? EDIT_DR_SWITCH_EDIT | EDIT_DR_SWITCH_FMODE : EDIT_DR_SWITCH_FMODE ;

		eptr = &g_model.expoData[s_expoChan] ;
    
		if(which==DR_DRSW1) {
				eptr->drSw1 = edit_dr_switch( x, y, eptr->drSw1, invBlk, doedit, event ) ;
    }
    else if(which==DR_DRSW2) {
				eptr->drSw2 = edit_dr_switch( x, y, eptr->drSw2, invBlk, doedit, event ) ;
    }
    else
		{
				ptr = &eptr->expo[which][exWt][stkRL] ;
        if(exWt==DR_EXPO)
				{
            
					*ptr = gvarMenuItem( x, y, *ptr, -100, 100, invBlk, event ) ;
        }
        else
				{
					*ptr = gvarMenuItem( x, y, *ptr+100, 0, 100, invBlk, event ) - 100 ;
        }
		}
}

void menuProcExpoAll(uint8_t event)
{
	TITLE(PSTR(STR_EXPO_DR));
	EditType = EE_MODEL ;
	static MState2 mstate2;
	uint32_t count = 5-1 ;
	if ( SingleExpoChan )
	{
		count -= 1 ;
	}
	event = mstate2.check_columns( event, count ) ;
	
	uint8_t stkVal ;
	int8_t  sub = mstate2.m_posVert ;
	int8_t subN ;
	if ( SingleExpoChan )
	{
		sub += 1 ;
	}
	if( sub )
	{
		StickScrollAllowed = 0 ;
	}

	uint8_t l_expoChan = s_expoChan ;
	subN = 0 ;
	{
    uint8_t attr = 0 ;
		if ( sub == subN )
		{
			if ( SingleExpoChan == 0 )
			{
				s_expoChan = l_expoChan = checkIncDec( s_expoChan, 0, 3, 0 ) ;
				attr = InverseBlink ;
			}
		}		 
		putsChnRaw(0,FH,l_expoChan+1,attr) ;
	}

	uint8_t expoDrOn = get_dr_state(l_expoChan);
	switch (expoDrOn)
	{
    case DR_MID:
      lcd_puts_Pleft( FH,PSTR(STR_4DR_MID));
    break;
    case DR_LOW:
      lcd_puts_Pleft( FH,PSTR(STR_4DR_LOW));
    break;
    default: // DR_HIGH:
      lcd_puts_Pleft( FH,PSTR(STR_4DR_HI));
    break;
	}

	subN += 1 ;
	stkVal = DR_BOTH ;
	if(calibratedStick[l_expoChan]> 100) stkVal = DR_RIGHT;
	if(calibratedStick[l_expoChan]<-100) stkVal = DR_LEFT;
	if(IS_EXPO_THROTTLE(l_expoChan)) stkVal = DR_RIGHT;

	lcd_puts_Pleft(2*FH,PSTR(STR_2EXPO));
	editExpoVals( event, (stkVal != DR_RIGHT) && (sub==subN), 4*FW, 3*FH, expoDrOn ,DR_EXPO, DR_LEFT ) ;
	editExpoVals( event, (stkVal != DR_LEFT) && (sub==subN), 8*FW, 3*FH, expoDrOn ,DR_EXPO, DR_RIGHT ) ;

	subN += 1 ;
	lcd_puts_Pleft(4*FH,PSTR(STR_2WEIGHT));
	editExpoVals( event, (stkVal != DR_RIGHT) && (sub==subN), 4*FW, 5*FH, expoDrOn ,DR_WEIGHT, DR_LEFT ) ;
	editExpoVals( event, (stkVal != DR_LEFT) && (sub==subN), 8*FW, 5*FH, expoDrOn ,DR_WEIGHT, DR_RIGHT ) ;

	subN += 1 ;
	lcd_puts_Pleft(6*FH,PSTR(STR_DR_SW1));
	editExpoVals( event, sub==subN,5*FW, 6*FH, DR_DRSW1 , 0,0);
	subN += 1 ;
	lcd_puts_Pleft(7*FH,PSTR(STR_DR_SW2));
	editExpoVals( event, sub==subN,5*FW, 7*FH, DR_DRSW2 , 0,0);

	pushPlotType( PLOT_BLACK ) ;
	
	lcd_vline(XD - (IS_EXPO_THROTTLE(s_expoChan) ? WCHART : 0), Y0 - WCHART, WCHART * 2);

	drawFunction( XD, GRAPH_FUNCTION_EXPO ) ;

	int16_t x512  = calibratedStick[s_expoChan];
	int16_t y512 = calcExpo( l_expoChan, x512 ) ;
  
	lcd_outdezAtt( 19*FW, 6*FH,x512*25/((signed) RESXu/4), 0 );
	lcd_outdezAtt( 14*FW, 1*FH,y512*25/((signed) RESXu/4), 0 );
	
	int16_t xv = (x512 * WCHART + RESX/2) / RESX + XD ;
  int16_t yv = Y0 - (y512 * WCHART + RESX/2) / RESX ;

	lcd_vline( xv, yv-3, 7 ) ;
	lcd_hline( xv-3, yv, 7 ) ;
	
	popPlotType() ;
}


uint8_t char2idx(char c)
{
	uint8_t ret ;
    for(ret=0;;ret++)
    {
        char cc= s_charTab[ret] ;
        if(cc==0) return 0;
        if(cc==c) return ret;
    }
}
char idx2char(uint8_t idx)
{
    if(idx < NUMCHARS) return s_charTab[idx] ;
    return ' ';
}

uint8_t DupIfNonzero = 0 ;
int8_t DupSub ;

void menuDeleteDupModel(uint8_t event)
{
	uint8_t action ;
  lcd_putsnAtt(1,2*FH, (char *)ModelNames[DupSub],sizeof(g_model.name),/*BSS*/0);
  lcd_putc(sizeof(g_model.name)*FW+FW,2*FH,'?');
	action = yesNoMenuExit( event, DupIfNonzero ? PSTR(STR_DUP_MODEL) : PSTR(STR_DELETE_MODEL) ) ;

	switch(action)
	{
    case YN_YES :
      if ( DupIfNonzero )
      {
        message(PSTR(STR_DUPLICATING));
        if(eeDuplicateModel(DupSub))
        {
          audioDefevent(AU_MENUS);
          DupIfNonzero = 2 ;		// sel_editMode = false;
        }
        else audioDefevent(AU_WARNING1);
      }
      else
      {
				ee32_delete_model( DupSub-1 ) ;
      }
    break;
		
		case YN_NO :
//      pushMenu(menuProcModelSelect);
    break;
  }
}

void dsmDisplayABLRFH()
{
	for ( uint32_t i = 0 ; i < 6 ; i += 1 )
	{
		uint16_t value = DsmABLRFH[i] ;
		lcd_putc( i*3*(FW+1), 7*FH, "ABLRFH"[i] ) ;
		if ( value == 0 )		// 0xFFFF + 1
		{
		  lcd_puts_P( (i+1)*3*(FW+1)-2-(2*FW), 7*FH, XPSTR("--") ) ;
		}
		else
		{
			if ( value > 1000 )
			{
				value = 1000 ;
			}
		  lcd_outdezAtt( (i+1)*3*(FW+1)-1, 7*FH, value - 1, 0 ) ;
		}
	}
}

uint8_t DsmResponseFlag ;
// Bit 7 = DSMX, Bit 6 = 11mS, bits 3-0 channels
uint8_t MultiResponseData ;

uint8_t BindTimer ;


void menuRangeBind(uint8_t event)
{
	static uint8_t binding = 0 ;
	uint8_t *ptrFlag ;

//extern uint8_t SerialData[2][28] ;
//	lcd_outhex4( 0, 0, (SerialData[0][0] << 8 ) | (SerialData[0][1] ) ) ;
//	lcd_outhex4( 40, 0, (SerialData[1][0] << 8 ) | (SerialData[1][1] ) ) ;

//extern uint8_t SerialExternalData[] ;
//extern uint8_t Multi2Data[] ;
//	lcd_outhex4( 0, 0, ( SerialExternalData[0] << 8 ) | (SerialExternalData[1] ) ) ;
//	lcd_outhex4( 40, 0, ( Multi2Data[0] << 8 ) | ( Multi2Data[1] ) ) ;

	ptrFlag = &BindRangeFlag[s_currIdx] ;
	uint8_t flag = *ptrFlag & PXX_BIND ;
  if ( event == EVT_ENTRY )
	{
		binding = flag ? 1 : 0 ;
		BindTimer = 255 ;
//		if ( g_model.Module[0].protocol == PROTO_MULTI )
//		{
//			if ( (g_model.Module[0].sub_protocol & 0x3F) == M_DSM )
//			{
//				MultiResponseFlag = 0 ;
//			}
//		}
//		if ( g_model.Module[1].protocol == PROTO_MULTI )
//		{
//			if ( (g_model.Module[1].sub_protocol & 0x3F) == M_DSM )
//			{
//				MultiResponseFlag = 0 ;
//			}
//		}
		DsmResponseFlag = 0 ;
	}

	if ( checkForExitEncoderLong( event ) )
	{
    killEvents(event);
		*ptrFlag = 0 ;
		popMenu(false) ;
	}

	if ( binding && ( flag == 0 ) )
	{
		*ptrFlag = 0 ;
		popMenu(false) ;
	}

	lcd_puts_Pleft( 2*FH, (binding) ? PSTR(STR_6_BINDING) : PSTR(STR_RANGE_RSSI) ) ;

	if ( binding == 0 )
	{
		lcd_outdezAtt( 12 * FW, 4*FH, FrskyHubData[FR_RXRSI_COPY], DBLSIZE);
//		{
//			lcd_outdezAtt( 20 * FW, 4*FH, FrskyHubData[FR_TEMP2], DBLSIZE) ;
//		}

	}

	if ( ( ( g_model.Module[0].protocol == PROTO_MULTI ) && ( s_currIdx == 0 ) ) ||
		( ( g_model.Module[1].protocol == PROTO_MULTI ) && ( s_currIdx == 1 ) ) )
	{
		if ( binding )
		{
			if ( PrivateData[1] && BindTimer < 170 )
			{
				if ( ( PrivateData[0] & 0x08 ) == 0 )
				{
					*ptrFlag = 0 ;
					popMenu(false) ;
				}
			}
		}
	}

	if ( ( ( g_model.Module[0].protocol == PROTO_MULTI ) && ( (g_model.Module[0].sub_protocol & 0x3F) == M_DSM ) ) ||
		( ( g_model.Module[1].protocol == PROTO_MULTI ) && ( (g_model.Module[1].sub_protocol & 0x3F) == M_DSM ) ) )
	{
		if ( DsmResponseFlag )
		{
			uint32_t module = s_currIdx ;
//			if ( ptrFlag == &BindRangeFlag[1] )
//			{
//				module = 1 ;
//			}

			lcd_outhex4( 0, 6*FH, MultiResponseData ) ;

			lcd_puts_Pleft( 7*FH, XPSTR("DSM2 22mS\015ch") ) ;
			if ( MultiResponseData & 0x80 )
			{
				lcd_putc( 3*FW, 7*FH, 'X' ) ;
			}
			if ( MultiResponseData & 0x40 )
			{
				lcd_puts_Pleft( 7*FH, "\00511" ) ;
			}
  		lcd_outdez( 13*FW-2, 7*FH, MultiResponseData & 0x0F ) ;

			int8_t x ;
			uint8_t y ;
			
			if ( *ptrFlag )
			{
				*ptrFlag = 0 ;
				x = MultiResponseData & 0x0F ;	// # channels
				y = (MultiResponseData & 0xC0 ) >> 2 ;	// DSMX, 11mS
				if ( module )
				{
					g_model.xoption_protocol = (g_model.xoption_protocol & 0xF0) | x ;
					y |= g_model.xppmNCH & 0x8F ;
					g_model.xppmNCH = y ;	// Set DSM2/DSMX
					
					g_model.Module[module].option_protocol = (g_model.Module[module].option_protocol & 0xF0) | x ;
					y |= g_model.Module[module].channels & 0x8F ;
					g_model.Module[module].channels = y ;	// Set DSM2/DSMX
				}
				else
				{
					g_model.option_protocol = (g_model.option_protocol & 0xF0) | x ;
					y |= g_model.ppmNCH & 0x8F ;
					g_model.ppmNCH = y ;	// Set DSM2/DSMX
					
					g_model.Module[module].option_protocol = (g_model.Module[module].option_protocol & 0xF0) | x ;
					y |= g_model.Module[module].channels & 0x8F ;
					g_model.Module[module].channels = y ;	// Set DSM2/DSMX
				}
				protocolsToModules() ;
				STORE_MODELVARS ;
			}
		}
	}
	
	if ( ( g_model.Module[1].protocol == PROTO_DSM2 ) && ( g_model.Module[1].sub_protocol == DSM_9XR ) )
	{
		if ( DsmResponseFlag )
		{
			lcd_outhex4( 0, 6*FH, (uint8_t)g_model.dsmMode ) ;

			lcd_puts_Pleft( 7*FH, XPSTR("DSM2 22mS\015ch") ) ;
			if ( g_model.dsmMode & 0x01 )
			{
				lcd_putc( 3*FW, 7*FH, 'X' ) ;
			}
			if ( g_model.dsmMode & 0x02 )
			{
				lcd_puts_Pleft( 7*FH, "\00511" ) ;
			}
  		lcd_outdez( 13*FW-2, 7*FH, g_model.Module[1].channels ) ;
		}
	}


	if ( --BindTimer == 0 )
	{
  	audioDefevent(AU_WARNING2) ;
	}

//#if defined(PCBX10) && defined(PCBREV_EXPRESS)
//extern uint8_t DsmTxPacket[3][11] ;
//	lcd_outhex2( 0,  10*FH, DsmTxPacket[0][0] ) ;
//	lcd_outhex2( 12, 10*FH, DsmTxPacket[0][1] ) ;
//	lcd_outhex2( 24, 10*FH, DsmTxPacket[0][2] ) ;
//	lcd_outhex2( 36, 10*FH, DsmTxPacket[0][3] ) ;
//	lcd_outhex2( 48, 10*FH, DsmTxPacket[0][4] ) ;
//	lcd_outhex2( 60, 10*FH, DsmTxPacket[0][5] ) ;
//	lcd_outhex2( 72, 10*FH, DsmTxPacket[0][6] ) ;
//	lcd_outhex2( 84, 10*FH, DsmTxPacket[0][7] ) ;
//	lcd_outhex2( 96, 10*FH, DsmTxPacket[0][8] ) ;
//	lcd_outhex2( 108,10*FH, DsmTxPacket[0][9] ) ;
//	lcd_outhex2( 120,10*FH, DsmTxPacket[0][10] ) ;
	
//	lcd_outhex2( 0,  11*FH, DsmTxPacket[1][0] ) ;
//	lcd_outhex2( 12, 11*FH, DsmTxPacket[1][1] ) ;
//	lcd_outhex2( 24, 11*FH, DsmTxPacket[1][2] ) ;
//	lcd_outhex2( 36, 11*FH, DsmTxPacket[1][3] ) ;
//	lcd_outhex2( 48, 11*FH, DsmTxPacket[1][4] ) ;
//	lcd_outhex2( 60, 11*FH, DsmTxPacket[1][5] ) ;
//	lcd_outhex2( 72, 11*FH, DsmTxPacket[1][6] ) ;
//	lcd_outhex2( 84, 11*FH, DsmTxPacket[1][7] ) ;
//	lcd_outhex2( 96, 11*FH, DsmTxPacket[1][8] ) ;
//	lcd_outhex2( 108,11*FH, DsmTxPacket[1][9] ) ;
//	lcd_outhex2( 120,11*FH, DsmTxPacket[1][10] ) ;

//	lcd_outhex2( 0,  12*FH, DsmTxPacket[2][0] ) ;
//	lcd_outhex2( 12, 12*FH, DsmTxPacket[2][1] ) ;
//	lcd_outhex2( 24, 12*FH, DsmTxPacket[2][2] ) ;
//	lcd_outhex2( 36, 12*FH, DsmTxPacket[2][3] ) ;
//	lcd_outhex2( 48, 12*FH, DsmTxPacket[2][4] ) ;
//	lcd_outhex2( 60, 12*FH, DsmTxPacket[2][5] ) ;
//	lcd_outhex2( 72, 12*FH, DsmTxPacket[2][6] ) ;
//	lcd_outhex2( 84, 12*FH, DsmTxPacket[2][7] ) ;
//	lcd_outhex2( 96, 12*FH, DsmTxPacket[2][8] ) ;
//	lcd_outhex2( 108,12*FH, DsmTxPacket[2][9] ) ;
//	lcd_outhex2( 120,12*FH, DsmTxPacket[2][10] ) ;
//#endif

}

void menuBindOptions(uint8_t event)
{
//	uint8_t b ;
	struct t_module *pModule = &g_model.Module[s_currIdx] ;
	TITLE(XPSTR("Bind Options"));
	static MState2 mstate2;
	event = mstate2.check_columns( event, 2 ) ;
  switch(event)
  {
//		case EVT_ENTRY:
//			PxxExtra[s_currIdx] = 0 ;
//		break ;
	
    case EVT_KEY_BREAK(BTN_RE):
    case EVT_KEY_FIRST(KEY_MENU):
			if ( mstate2.m_posVert == 0 )
		  {
#ifdef ACCESS
				if ( g_model.Module[s_currIdx].protocol == PROTO_ACCESS )
				{
					ModuleControl[s_currIdx].bindStep = BIND_START ;
					ModuleSettings[s_currIdx].mode = MODULE_MODE_BIND ;
					AccState = ACC_ACCST_BIND ;
					popMenu( 0 ) ;
				}
				else
#endif
				{
		  		BindRangeFlag[s_currIdx] = PXX_BIND ;		    	//send bind code
					chainMenu( menuRangeBind ) ;
				}
			}
		break ;
	}
	int8_t sub = mstate2.m_posVert ;
	
	lcd_puts_Pleft( 1*FH, PSTR(STR_BIND) ) ;
	if(sub == 0)
	{
		lcd_char_inverse( 0, 1*FH, 4*FW, 0 ) ;
	}
//	b = PxxExtra[s_currIdx] & 1 ;
	pModule->highChannels = checkIndexed( 2*FH, XPSTR("\0""\001""\012Chans 1-8 Chans 9-16"), pModule->highChannels, (sub==1) ) ;
//	PxxExtra[s_currIdx] =	( PxxExtra[s_currIdx] & ~1 ) | b ;
//	b = (PxxExtra[s_currIdx] & 2) ? 1 : 0 ;
	pModule->disableTelemetry = checkIndexed( 3*FH, XPSTR("\0""\001""\014Telemetry   No Telemetry"), pModule->disableTelemetry, (sub==2) ) ;
//	PxxExtra[s_currIdx] =	( PxxExtra[s_currIdx] & ~2 ) | ( b ? 2 : 0 ) ;
}

static void editTimer( uint8_t sub, uint8_t event )
{
	uint8_t subN ;
	uint8_t timer ;
#ifdef HAPTIC
	if ( sub < 8 )
#else
	if ( sub < 7 )
#endif
	{
		subN = 0 ;
		timer = 0 ;
	}
	else
	{
#ifdef HAPTIC
		subN = 8 ;
#else
		subN = 7 ;
#endif
		timer = 1 ;
	}
	t_TimerMode *ptm ;
	
	ptm = &g_model.timer[timer] ;

	uint8_t y = 0 ;
	uint8_t blink = InverseBlink ;

 	putsTime(14*FW-1, y, ptm->tmrVal,(sub==subN ? blink:0),(sub==subN ? blink:0) ) ;

#ifndef NOPOTSCROLL
	    if(sub==subN)	// Use s_editing???
#else
	    if(sub==subN)
#endif
			{
				int16_t temp = 0 ;
				CHECK_INCDEC_H_MODELVAR( temp, -60 ,60 ) ;
				ptm->tmrVal += temp ;
        if((int16_t)ptm->tmrVal < 0) ptm->tmrVal=0;
				
			}
			y += FH ;
		subN++ ;

    	lcd_puts_Pleft(    y, PSTR(STR_TRIGGERA));
			uint8_t attr = 0 ;
    	if(sub==subN)
			{
   			attr = blink ;
				CHECK_INCDEC_H_MODELVAR_0( ptm->tmrModeA, 1+2+24+8 ) ;
			}
    	putsTmrMode(16*FW,y,attr, timer, 1 ) ;
			y += FH ;
		subN++ ;
  	
  	  lcd_puts_Pleft(    y, PSTR(STR_TRIGGERB));
  	  attr = 0 ;
  	  if(sub==subN)
			{
	   		attr = blink ;
			}
			uint8_t doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			ptm->tmrModeB = edit_dr_switch( 16*FW, y, ptm->tmrModeB, attr, doedit, event ) ;
			y += FH ;
		subN++ ;
	 
  	  lcd_puts_Pleft( y, PSTR(STR_TIMER) );
  	  attr = 0 ;
  	  if(sub==subN)
			{
				attr = blink ; CHECK_INCDEC_H_MODELVAR_0( ptm->tmrDir,1) ;
			}
  	  lcd_putsAttIdx(  10*FW, y, PSTR(STR_COUNT_DOWN_UP), ptm->tmrDir, attr ) ;
			y += FH ;
		subN++ ;

			uint8_t b = timer ? g_model.timer2Mbeep : g_model.timer1Mbeep ;
  	  (timer ? g_model.timer2Mbeep : g_model.timer1Mbeep) = onoffMenuItem( b, y, PSTR(STR_MINUTE_BEEP), sub ==subN ) ;
			y += FH ;
		subN++;

			b = timer ? g_model.timer2Cdown : g_model.timer1Cdown ;
  	  (timer ? g_model.timer2Cdown : g_model.timer1Cdown) = onoffMenuItem( b, y, PSTR(STR_BEEP_COUNTDOWN), sub==subN  ) ;
			y += FH ;
			subN++;

  	  lcd_puts_Pleft( y, PSTR( STR_RESET_SWITCH ));
  	  attr = 0 ;
			if(sub==subN)
			{
				attr = blink ;
			}

			int16_t sw = (timer==0) ? g_model.timer1RstSw : g_model.timer2RstSw ;
			doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			sw = edit_dr_switch( 16*FW, y, sw, attr, doedit, event ) ;
			if ( timer == 0 )
			{
				g_model.timer1RstSw = sw ;
			}
			else
			{
				g_model.timer2RstSw = sw ;
			}
			y += FH ;
			subN++;
  	  
#ifdef HAPTIC
			lcd_puts_Pleft( y, XPSTR("Haptic"));
  	  attr = 0 ;
			if(sub==subN)
			{
				attr = blink ;
			}
			sw = (timer==0) ? g_model.timer1Haptic : g_model.timer2Haptic ;
			sw = checkIndexed( y, XPSTR(FWx12"\003""\006  NoneMinute Cdown  Both"), sw, (sub==subN) ) ;
			if ( timer == 0 )
			{
				g_model.timer1Haptic = sw ;
			}
			else
			{
				g_model.timer2Haptic = sw ;
			}
			y += FH ;
#endif
}



#define OPTION_NONE			0
#define OPTION_OPTION		1
#define OPTION_RFTUNE		2
#define OPTION_VIDFREQ	3
#define OPTION_FIXEDID	4
#define OPTION_TELEM		5
#define OPTION_SRVFREQ	6
#define OPTION_MAXTHR		7
#define OPTION_RFCHAN		8

void multiOption( uint32_t x, uint32_t y, int32_t option, uint32_t attr, uint32_t protocol )
{
	uint32_t display = 1 ;
	if (MultiSetting.valid & 4 )
	{
		if ( protocol != M_DSM )
		{
			protocol = 256 + (MultiSetting.subData >> 4) ;
		}
	}

	switch ( protocol )
	{
		case M_DSM :
			if ( ( option >= 4 ) && ( option <= 12 ) )
			{
				lcd_putsAttIdx( x-4*FW, y, XPSTR("\004 4ch 5ch 6ch 7ch 8ch 9ch10ch11ch12ch"), option-4, attr ) ;
				return ;
			}
		break ;

		case M_FRSKYX2 :
		case M_FRSKYX :
		case M_FrskyD :
		case M_FRSKYV :
		case M_SFHSS :
		case M_CORONA :
		case M_Hitec :
		case 256 + OPTION_RFTUNE :
			lcd_puts_Pleft( y, XPSTR("\013Freq.") ) ;
			display = 0 ;
		break ;
		
		case M_Devo :
		case 256 + OPTION_FIXEDID :
			if ( ( option >= 0 ) && ( option <= 1 ) )
			{
				lcd_putsAttIdx( x-6*FW, y, XPSTR("\006AutoIDFixdID"), option, attr ) ;
				return ;
			}
		break ;

		case 256 + OPTION_SRVFREQ :
		case M_AFHD2SA :
		{
			uint8_t xoption = option ;
			if ( xoption & 0x80 )
			{
				lcd_puts_Pleft( y, XPSTR("\013T-") ) ;
				xoption &= 0x7F ;
				option = xoption ;
			}
			if ( xoption <= 70 )
			{
				lcd_puts_Pleft( y, XPSTR("\015Rate") ) ;
				display = 0 ;
				option = xoption ;
				option *= 5 ;
				option += 50 ;
			}
		}
		break ;

		case 256 + OPTION_VIDFREQ :
		case M_Hubsan :
			lcd_puts_Pleft( y, XPSTR("\013VTX   ") ) ;
			display = 0 ;
		break ;

		case 256 + OPTION_TELEM :
		case M_BAYANG :
			if ( ( option >= 0 ) && ( option <= 1 ) )
			{
				lcd_puts_Pleft( y, XPSTR("\013Telem.") ) ;
				lcd_putsAttIdx( 20*FW, y, XPSTR("\001NY"), option, attr ) ;
				return ;
			}
		break ;
		case 256 + OPTION_RFCHAN :
			lcd_puts_Pleft( y, XPSTR("\013Rf Chan.") ) ;
		break ;
	}
	if ( display )
	{
		lcd_puts_Pleft( y, PSTR(STR_MULTI_OPTION) ) ;
	}
	lcd_outdezAtt( x, y, option, attr ) ;
}


//void menuXProcProtocol(uint8_t event)
//{
//	menuProcProtocol(event) ;
	// now copy edited values to structure(s)
//	protocolsToModules() ;
//}

//#if 0
// PPM PXX DSM MULTI XFIRE
#ifdef PCBX9D
 #ifdef DISABLE_PXX
const uint8_t ProtocolOptions[2][5] = { {1,PROTO_PPM}, {4,PROTO_PPM,PROTO_DSM2,PROTO_MULTI,PROTO_XFIRE} };
 #else
  #ifdef PCBX9LITE
 #ifdef ACCESS
const uint8_t ProtocolOptions[2][7] = { {1,PROTO_ACCESS}, {6,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI,PROTO_XFIRE,PROTO_ACCESS} };
 #else
const uint8_t ProtocolOptions[2][7] = { {1,PROTO_PXX}, {5,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI,PROTO_XFIRE} };
 #endif
	#else
 #ifdef ACCESS
const uint8_t ProtocolOptions[2][7] = { {3,PROTO_PPM,PROTO_PXX,PROTO_ACCESS}, {6,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI,PROTO_XFIRE,PROTO_ACCESS} };
 #else
const uint8_t ProtocolOptions[2][7] = { {2,PROTO_PPM,PROTO_PXX}, {5,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI,PROTO_XFIRE} };
 #endif
  #endif // X3
 #endif
#endif
#if defined(PCBX12D) || defined(PCBX10)
 #ifdef DISABLE_PXX
  #if defined(PCBT16)
const uint8_t ProtocolOptions[2][5] = { {2,PROTO_PPM,PROTO_MULTI}, {4,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI} } ;
	#else
   #ifdef ACCESS
const uint8_t ProtocolOptions[2][7] = { {1,PROTO_ACCESS}, {6,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI,PROTO_XFIRE,PROTO_ACCESS} };
	 #else
const uint8_t ProtocolOptions[2][5] = { {1,PROTO_PPM}, {3,PROTO_PPM,PROTO_DSM2,PROTO_MULTI} } ;
   #endif
  #endif
 #else
const uint8_t ProtocolOptions[2][5] = { {2,PROTO_PPM,PROTO_PXX}, {4,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI} };
 #endif
#endif
#ifdef PCBSKY
 #ifdef REVX
const uint8_t ProtocolOptions[2][6] = { {3,PROTO_PPM,PROTO_DSM2,PROTO_MULTI}, {5,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI,PROTO_XFIRE} };
 #else
const uint8_t ProtocolOptions[2][5] = { {3,PROTO_PPM,PROTO_DSM2,PROTO_MULTI}, {4,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI} };
 #endif
#endif
#ifdef PCB9XT
const uint8_t ProtocolOptions[2][6] = { {4,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI}, {5,PROTO_PPM,PROTO_PXX,PROTO_DSM2,PROTO_MULTI,PROTO_XFIRE} };
#endif

#ifdef PCBLEM1
const uint8_t ProtocolOptions[2][6] = { {1,PROTO_DSM2}, {1,PROTO_DSM2} };
#endif


uint32_t checkProtocolOptions( uint8_t module )
{
#if defined(PCBX12D) || defined(PCBX10)
	if ( module == 0 )
	{
 #ifdef DISABLE_PXX
  #if defined(PCBT16)
		g_model.Module[module].protocol = PROTO_MULTI ;
	#else
		g_model.Module[module].protocol = PROTO_PPM ;
	#endif
 #else
  #ifdef ACCESS
		g_model.Module[module].protocol = PROTO_ACCESS ;
  #else
   #if defined(PCBT16)
		g_model.Module[module].protocol = PROTO_MULTI ;
	 #else
		g_model.Module[module].protocol = PROTO_PXX ;
  	#endif
  #endif
 #endif
		return 1 ;
	}
#endif
	uint8_t *options = (uint8_t *) ProtocolOptions[module] ;
	uint32_t count = *options++ ;
	uint8_t index = g_model.Module[module].protocol ;
	uint8_t save = index ;
//	index = 0 ;
//	for ( uint32_t c = 0 ; c < count ; c += 1 )
//	{
//		if ( options[c] == save )
//		{
//			index = c ;
//			break ;
//		}
//	}
//	CHECK_INCDEC_H_MODELVAR_0( index, count-1 ) ;
//	index = options[index] ;
//	g_model.Module[module].protocol = index ;
	
	index = checkOutOfOrder( index, options, count ) ;
	g_model.Module[module].protocol = index ;
	
	if ( save != index )
	{
		g_model.Module[module].sub_protocol = 0 ;
		return 1 ;
	}
	return 0 ;
}

void displayProtocol( uint16_t x, uint16_t y, uint8_t value, uint8_t attr )
{
	if ( value != PROTO_OFF )
	{
		lcd_putsAttIdx( x, y, PSTR(STR_PROT_OPT), value, attr ) ;
	}
	else
	{
		lcd_putsAtt( x, y, XPSTR( "OFF" ), attr ) ;
	}
}

void displayModuleName( uint16_t x, uint16_t y, uint8_t index, uint8_t attr )
{
	lcd_putsAttIdx( x, y, XPSTR("\010InternalExternal"), index, attr ) ;
}

uint8_t EditingModule ;
#ifdef PCBX9LITE
const uint8_t PxxProtocolMap[] = { 0, 2, 3 } ;
#endif

#ifdef ACCESS

void popupMessage( char *message )
{
	uint32_t y ;
	for ( y = 2*FH ; y < 7*FH ; y += FH )
	{
		lcd_puts_P( 2*FW + X12OFFSET, y, "                 " ) ;
	}
	lcd_puts_P( 3*FW + X12OFFSET, 4*FH, message ) ;
	lcd_puts_P( 15*FW + X12OFFSET, 6*FH, "Ok" ) ;
	lcd_rect( 2*FW + X12OFFSET + 1, 2*FH, 17*FW-2, 5*FH ) ;
}


uint8_t AccEntry ;

void editAccessRegister( uint8_t module, uint8_t event )
{
	TITLE( XPSTR("Register") ) ;
	static MState2 mstate2 ;

	if ( ( event == EVT_KEY_BREAK(KEY_EXIT) ) || ( event == EVT_KEY_LONG(KEY_EXIT) ) )
	{
		ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
		AccState = ACC_NORMAL ;
		return ;
	}
	event = mstate2.check_columns( event, ( ModuleControl[module].registerStep != REGISTER_START ) ? 3 : 1 ) ;
	int8_t  sub = mstate2.m_posVert ;
	uint8_t subN = 0 ;
  
	uint8_t attr = sub==subN ? InverseBlink : 0 ;
	lcd_puts_Pleft( 1*FH, XPSTR( "Radio ID") ) ;
	EditType = EE_GENERAL ;
	alphaEditName( 11*FW, FH, g_eeGeneral.radioRegistrationID, 8, attr | ALPHA_NO_NAME, (uint8_t *)XPSTR( "Radio ID") ) ;
	EditType = EE_MODEL ;
	subN += 1 ;
	
	lcd_puts_Pleft( 2*FH, XPSTR("Rx UID") ) ;
	attr = sub==subN ? InverseBlink : 0 ;
	if ( attr )
	{
  	CHECK_INCDEC_H_MODELVAR_0( ModuleControl[module].registerModuleIndex, 2 ) ;
	}
	lcd_outdezAtt( 12*FW, 2*FH, ModuleControl[module].registerModuleIndex, attr ) ;
	subN += 1 ;

	attr = sub==subN ? InverseBlink : 0 ;
  if ( ModuleControl[module].registerStep != REGISTER_START )
//			 ModuleControl[module].registerRxName[0] )
	{
//		lcd_puts_Pleft( 3*FH, XPSTR("Found") ) ;
		alphaEditName( 10*FW, 3*FH, ModuleControl[module].registerRxName, PXX2_LEN_RX_NAME, attr, (uint8_t *)XPSTR( "Rx Name") ) ;
	}
	else
	{
		lcd_puts_Pleft( 3*FH, XPSTR("Waiting") ) ;
	}
	
	subN += 1 ;

	attr = 0 ;
	if ( sub==subN )
	{
		attr = INVERS ;
		if ( checkForMenuEncoderBreak( event ) )
		{
  		if ( ModuleControl[module].registerStep != REGISTER_START )
			{
				ModuleControl[module].registerStep = REGISTER_RX_NAME_SELECTED ;
				ModuleControl[module].timeout = 0 ;
				ModuleSettings[module].mode = MODULE_MODE_REGISTER ;
			}
			killEvents( event ) ;
			s_editMode = false ;
		}
	}
  lcd_putsAtt( 0, 4*FH, XPSTR("REGISTER"), attr ) ;

	if ( ModuleControl[module].registerStep == REGISTER_OK )
	{
		ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
		popupMessage( XPSTR("Registration Ok") ) ;
		if ( checkForMenuEncoderBreak( event ) )
		{		
			AccState = ACC_NORMAL ;
			killEvents( event ) ;
			s_editMode = false ;
		}
	}
}

void editAccessShare( uint8_t module, uint8_t event )
{
	TITLE( XPSTR("Sharing") ) ;

	// Need to determine when the module is shared to clear the name
	if ( ( event == EVT_KEY_BREAK(KEY_EXIT) ) || ( event == EVT_KEY_LONG(KEY_EXIT) ) )
	{
		ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
		AccState = ACC_NORMAL ;
		return ;
	}

	lcd_putsnAtt( 0, 2*FH, (char *)g_model.Access[module].receiverName[ModuleControl[module].bindReceiverIndex], PXX2_LEN_RX_NAME, 0 ) ;
	 
}

void editAccstAccessBind( uint8_t module, uint8_t event )
{
	TITLE( XPSTR("Bind") ) ;
//	static MState2 mstate2 ;
	
//	if ( event == EVT_ENTRY )
//	{
//		ModuleControl[module].bindReceiverCount = 0 ;
//		ModuleControl[module].timeout = 0 ;
//		ModuleControl[module].bindStep = BIND_START ;
//		ModuleSettings[module].mode = MODULE_MODE_BIND ;
//	}
  if ( event == EVT_ENTRY )
	{
		BindTimer = 255 ;
	}
	if ( event == EVT_KEY_BREAK(KEY_EXIT) )
	{
		ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
		AccState = ACC_NORMAL ;
		return ;
	}
	if ( --BindTimer == 0 )
	{
  	audioDefevent(AU_WARNING2) ;
	}
}

void editAccessBind( uint8_t module, uint8_t event )
{
	TITLE( XPSTR("Bind") ) ;
	static MState2 mstate2 ;

	if ( event == EVT_KEY_BREAK(KEY_EXIT) )
	{
		ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
		AccState = ACC_NORMAL ;
		return ;
	}
  if ( ModuleControl[module].bindReceiverCount )
	{
		lcd_puts_Pleft( FH, XPSTR("Select") ) ;
		uint32_t i ;
		uint32_t y = 2*FH ;

		event = mstate2.check_columns( event, ModuleControl[module].bindReceiverCount - 1 ) ;
		int8_t  sub    = mstate2.m_posVert ;
		uint8_t subN = 0 ;
		for ( i = 0 ; i < ModuleControl[module].bindReceiverCount ; i += 1 )
		{
			uint8_t attr = (sub == subN) ? InverseBlink : 0 ;
			lcd_putsnAtt( 0, y, (char *)ModuleControl[module].bindReceiversNames[i], PXX2_LEN_RX_NAME, attr ) ;
			if ( attr && checkForMenuEncoderBreak( event ) )
			{
				ModuleControl[module].bindReceiverNameIndex = i ;
				ModuleControl[module].bindStep = BIND_RX_NAME_SELECTED ;
				killEvents( event ) ;
				s_editMode = false ;
			}
			subN += 1 ;
			y += FH ;	 
		}
	}
	else
	{
		lcd_puts_Pleft( FH, XPSTR("Waiting") ) ;
	}
	
//	if ( ModuleSettings[module].mode == MODULE_MODE_BIND )
//	{
  	if ( ModuleControl[module].bindStep == BIND_OK )
		{
			popupMessage( XPSTR("Bind Ok") ) ;
			if ( checkForMenuEncoderBreak( event ) )
			{		
//				ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
				AccState = ACC_NORMAL ;
			}
		}
//	}
}

void menuDeleteReceiver(uint8_t event)
{
	uint8_t action ;
	if ( s_currIdx < 2 )
	{
		action = yesNoMenuExit( event, (ModuleControl[s_currIdx].resetType == 0x01) ? XPSTR("Delete Receiver?") : XPSTR("Reset Receiver?") ) ;

		switch( action )
		{
  	  case YN_YES :
	 			ModuleSettings[s_currIdx].mode = MODULE_MODE_RESET ;
       	memset(g_model.Access[s_currIdx].receiverName[ModuleControl[s_currIdx].bindReceiverIndex], 0, PXX2_LEN_RX_NAME ) ;
			case YN_NO :
  	  break;
  	}
	}
	else
	{
    popMenu(false) ;
	}
}


void accesspopup( uint8_t event, uint8_t module )
{
	uint8_t popaction = doPopup( XPSTR("Bind\0Options\0Share\0Delete\0Reset\0Hardware"), 0x3F, 10, event ) ;
  if ( popaction == POPUP_SELECT )
	{
		uint8_t popidx = PopupData.PopupSel ;
		if ( popidx == 0 )	// Bind
		{
			killEvents(event);
			Tevent = 0 ;
			ModuleControl[module].bindStep = BIND_START ;
			ModuleControl[module].bindReceiverCount = 0 ;
			ModuleControl[module].timeout = 0 ;
			ModuleSettings[module].mode = MODULE_MODE_BIND ;
			AccState = ACC_BIND ;
		}
		else if ( popidx == 1 )	// Options
		{
			killEvents(event);
			Tevent = 0 ;
			ModuleControl[module].optionsState = 0 ;
			AccState = ACC_RXOPTIONS ;
		}
		else if ( popidx == 2 )	// Share
		{
			ModuleSettings[module].mode = MODULE_MODE_SHARE ;
			s_currIdx = module ;
			AccState = ACC_SHARE ;
			killEvents(event);
			Tevent = 0 ;
		}
		else if ( popidx == 3 )	// Delete
		{
			pushMenu(menuDeleteReceiver) ;
			ModuleControl[module].resetType = 0x01 ;
			s_currIdx = module ;
			killEvents(event);
			Tevent = 0 ;
//			pushMenu(menuAccessDelete);
		}
		else if ( popidx == 4 )	// Reset
		{
			pushMenu(menuDeleteReceiver) ;
			ModuleControl[module].resetType = 0xFF ;
			s_currIdx = module ;
			killEvents(event);
			Tevent = 0 ;
		}
		else if ( popidx == 5 )	// Hardware
		{
			ModuleControl[module].step = ModuleControl[module].bindReceiverIndex ;
			ModuleControl[module].timeout = 0 ;
			ModuleSettings[module].mode = MODULE_MODE_GET_HARDWARE_INFO ;
			AccState = ACC_RX_HARDWARE ;
			killEvents(event);
			Tevent = 0 ;
		}
		PopupData.PopupActive = 0 ;
	}
}

void editAccessRxOptions( uint8_t module, uint8_t event )
{
	// Rx UID in ModuleControl[module].bindReceiverIndex	
	TITLE( XPSTR("Receiver Options") ) ;
	static MState2 mstate2 ;
	EditType = 0 ;

	if ( ( event == EVT_KEY_BREAK(KEY_EXIT) ) || ( event == EVT_KEY_LONG(KEY_EXIT) ) )
	{
		ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
		AccState = ACC_NORMAL ;
		return ;
	}

	if ( ModuleControl[module].optionsState < 2 )
	{
		// Need to read settings from receiver
		// channelMapping
		lcd_puts_Pleft( 4*FH, XPSTR("    Reading") ) ;
		mstate2.m_posVert = 0 ;
		if ( ModuleControl[module].optionsState == 0 )
		{
			ModuleControl[module].rxtxSetupState = RECEIVER_SETTINGS_READ ;
			ModuleControl[module].receiverSetupTimeout = get_tmr10ms() - 200 ;
			ModuleControl[module].receiverSetupReceiverId = ModuleControl[module].bindReceiverIndex ;
			ModuleSettings[module].mode = MODULE_MODE_RECEIVER_SETTINGS ;
			ModuleControl[module].optionsState = 1 ;
		}
		if ( ModuleControl[module].rxtxSetupState == RECEIVER_SETTINGS_OK )
		{
			ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
			ModuleControl[module].optionsState = 2 ;
		}
		return ;
	}
	
	event = mstate2.check_columns( event, 9 ) ;
	int8_t  sub = mstate2.m_posVert ;
	uint8_t subN = 0 ;
	uint8_t y = 1*FH;
  uint8_t t_pgOfs ;
	uint8_t before ;
	uint8_t after ;
	uint8_t needWrite = 0 ;

	t_pgOfs = evalOffset( sub ) ;

	for(;;)
	{
		if(t_pgOfs<=subN)
		{
			before = ModuleControl[module].receiverSetupTelemetryDisabled ;
			after = checkIndexed( y, XPSTR("\0""\001""\014Telemetry   No Telemetry"), before, (sub==subN) ) ;
			if ( after != before )
			{
				needWrite = 1 ;
				ModuleControl[module].receiverSetupTelemetryDisabled = after ;
			}
			if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
		} 
		subN += 1 ;
		if(t_pgOfs<=subN)
		{
			before = ModuleControl[module].receiverSetupPwmRate ;
			after = checkIndexed( y, XPSTR("\0""\001""\00418mS 9mS"), before, (sub==subN) ) ;
			if ( after != before )
			{
				needWrite = 1 ;
				ModuleControl[module].receiverSetupPwmRate = after ;
			}
		
			if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
		} 
		subN += 1 ;
	
		for ( uint32_t i = 0 ; i < 8 ; i += 1 )
		{
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, XPSTR("Pin  CH") ) ;
				lcd_putc( 3*FW, y, i+'1' ) ;
				lcd_outdezAtt( 9*FW-2, y, ModuleControl[module].channelMapping[i]+1, (sub == subN) ? InverseBlink : 0 ) ;
			
				// edit value here
				if (sub == subN)
				{
					before = ModuleControl[module].channelMapping[i] ;
  				after = checkIncDec( before, 0, 23, 0 ) ;
					if ( after != before )
					{
						needWrite = 1 ;
						ModuleControl[module].channelMapping[i] = after ;
					}
				}
				if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
			}
			subN += 1 ;
		}
		break ;
	}

	if ( needWrite )
	{
		needWrite = 0 ;
		ModuleControl[module].rxtxSetupState = RECEIVER_SETTINGS_WRITE ;
		ModuleControl[module].receiverSetupTimeout = get_tmr10ms() ;
		ModuleSettings[module].mode = MODULE_MODE_RECEIVER_SETTINGS ;

	}

}

void editAccessTxOptions( uint8_t module, uint8_t event )
{
	// Rx UID in ModuleControl[module].bindReceiverIndex	
	TITLE( XPSTR("Module Options") ) ;
	static MState2 mstate2 ;
	EditType = 0 ;

	if ( ( event == EVT_KEY_BREAK(KEY_EXIT) ) || ( event == EVT_KEY_LONG(KEY_EXIT) ) )
	{
		ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
		AccState = ACC_NORMAL ;
		return ;
	}

	if ( ModuleControl[module].optionsState < 2 )
	{
		// Need to read settings from receiver
		// channelMapping
		lcd_puts_Pleft( 4*FH, ModuleControl[module].rxtxSetupState == MODULE_SETTINGS_READ ? XPSTR("    Reading") : XPSTR("    Writing") ) ;
		mstate2.m_posVert = 0 ;
		if ( ModuleControl[module].optionsState == 0 )
		{
			ModuleControl[module].moduleRequest = 0 ;
			ModuleControl[module].timeout = get_tmr10ms() - 100 ;
			ModuleControl[module].rxtxSetupState = MODULE_SETTINGS_READ ;
			ModuleSettings[module].mode = MODULE_MODE_GETSET_TX ;
			ModuleControl[module].optionsState = 1 ;
		}
		if ( ModuleControl[module].rxtxSetupState == MODULE_SETTINGS_OK )
		{
			ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
			ModuleControl[module].optionsState = 2 ;
		}
		return ;
	}
	
	event = mstate2.check_columns( event, 2 ) ;
	int8_t  sub = mstate2.m_posVert ;
	uint8_t subN = 0 ;
	uint8_t y = 1*FH;
  uint8_t t_pgOfs ;
	uint8_t before ;
	uint8_t after ;
//	uint8_t needWrite = 0 ;

	t_pgOfs = evalOffset( sub ) ;

	for(;;)
	{
		if(t_pgOfs<=subN)
		{
			uint8_t attr = 0 ;
			lcd_puts_Pleft( y, XPSTR("Power") ) ;
			if (sub == subN)
			{
				attr = InverseBlink ;
				before = ModuleControl[module].power ;
  			after = checkIncDec( before, -127, 127, 0 ) ;
				if ( after != before )
				{
					ModuleControl[module].power = after ;
				}
			}
			lcd_outdezAtt( 15*FW, y, ModuleControl[module].power, attr ) ;
			if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
		} 
		subN += 1 ;
		
		if(t_pgOfs<=subN)
		{
			ModuleControl[module].moduleExtAerial = onoffMenuItem( ModuleControl[module].moduleExtAerial, y, XPSTR("Use External Ant."), sub==subN ) ;
//			g_model.Module[module].externalAntenna = ModuleControl[module].moduleExtAerial ;
		  if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
		}
		subN += 1 ;
		
		if(t_pgOfs<=subN)
		{
			lcd_puts_Pleft( y, XPSTR("Write") ) ;
			if(sub==subN)
			{
				lcd_char_inverse( 0, y, 5*FW, 0 ) ;
				if ( checkForMenuEncoderBreak( event ) )
				{
					ModuleControl[module].optionsState = 0 ;
					ModuleControl[module].moduleRequest = 1 ;
					ModuleSettings[module].mode = MODULE_MODE_GETSET_TX ;
					killEvents( event ) ;
  				s_editMode = false ;
			  }
			}
		  if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
		}
		subN += 1 ;

		break ;
	}
}


void editAccessRxHardware( uint8_t module, uint8_t event )
{
	TITLE( XPSTR("Receiver Hardware") ) ;

	if ( ( event == EVT_KEY_BREAK(KEY_EXIT) ) || ( event == EVT_KEY_LONG(KEY_EXIT) ) )
	{
		ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
		AccState = ACC_NORMAL ;
		return ;
	}

	if ( ModuleSettings[module].mode == MODULE_MODE_NORMAL )
	{
		lcd_puts_Pleft( FH, XPSTR("Hardware") ) ;
		lcd_puts_Pleft( 2*FH, XPSTR("Software") ) ;
		lcd_puts_Pleft( 3*FH, XPSTR("Type") ) ;
		lcd_puts_Pleft( 4*FH, XPSTR("Variant") ) ;
		
		lcd_outhex4( 12*FW, FH, ModuleControl[module].rxHwVersion ) ;
		lcd_outhex4( 12*FW, 2*FH, ModuleControl[module].rxSwVersion ) ;
		lcd_putc( 12*FW, 3*FH, ModuleControl[module].rxModuleId + '0' ) ;
		lcd_puts_P( 14*FW, 3*FH, PXX2receiversModels[ModuleControl[module].rxModuleId] ) ;
		lcd_outhex4( 12*FW, 4*FH, ModuleControl[module].variant ) ;
	}
}

void editAccessSpectrum( uint8_t module, uint8_t event )
{
	uint32_t i ;
	uint32_t j ;
	TITLE( XPSTR("Spectrum Analyser") ) ;
	EditType = 0 ;
	static MState2 mstate2;
	Columns = 1 ;
	event = mstate2.check_columns( event, 0 ) ;
	uint8_t subSub = g_posHorz;
		 
	// Could request to turn off receiver

	// If exiting menu	 
	if ( ( event == EVT_KEY_BREAK(KEY_EXIT) ) || ( event == EVT_KEY_LONG(KEY_EXIT) ) )
	{
		// Need a 1 sec delay
		lcd_puts_Pleft( 3*FH, "\006STOPPING" ) ;
	  refreshDisplay() ;
		ModuleSettings[module].mode = MODULE_MODE_NORMAL ;
		AccState = ACC_NORMAL ;
		for ( i = 0 ; i < 50 ; i += 1 )
		{
			CoTickDelay(10) ;			// 20mS for now
			wdt_reset() ;
		}
		killEvents( event ) ;
		return ;
	}

	if ( ModuleSettings[module].mode != MODULE_MODE_SPECTRUM_ANALYSER )
	{
		for ( i = 0 ; i < SPECTRUM_NUM_BARS ; i += 1 )
		{
			SharedMemory.SpectrumAnalyser.bars[i] = 0 ;
		}
    SharedMemory.SpectrumAnalyser.span = 40000000 ;  // 40MHz
    SharedMemory.SpectrumAnalyser.freq = 2440000000U ;  // 2440MHz
    SharedMemory.SpectrumAnalyser.step = SharedMemory.SpectrumAnalyser.span / 128 ; // LCD_W ;
		ModuleControl[module].step = 0 ;
		ModuleSettings[module].mode = MODULE_MODE_SPECTRUM_ANALYSER ;
	}

	// Handle editing the fields
  for ( i = 0 ; i < 2 ; i += 1 )
	{
    uint8_t attr = (subSub == i ? (s_editMode>0 ? INVERS|BLINK : INVERS) : 0);

    switch (i)
		{
      case 0 :
			{
        uint16_t frequency = SharedMemory.SpectrumAnalyser.freq / 1000000 ;
        uint16_t newValue ;
//        lcdDrawText(1, 10, "F:", 0);
//        lcdDrawNumber(lcdLastRightPos + 2, 10, frequency, attr);
//        lcdDrawText(lcdLastRightPos + 2, 10, "MHz", 0);
				lcd_puts_Pleft( FH, XPSTR( "\002F:\010MHz" ) ) ;
				lcd_outdezAtt( 8*FW-3, FH, SharedMemory.SpectrumAnalyser.freq / 1000000, attr ) ;
        if (attr)
				{
          newValue = uint32_t(checkIncDec16( frequency, 2400, 2485, 0)) ;
          SharedMemory.SpectrumAnalyser.freq = (uint32_t) newValue * 1000000 ;
					if ( newValue != frequency )
					{
						for ( j = 0 ; j < SPECTRUM_NUM_BARS ; j += 1 )
						{
							SharedMemory.SpectrumAnalyser.bars[j] = 0 ;
						}
						ModuleControl[module].step = 0 ;
					}
        }
        break ;
      }

      case 1 :
			{	
        uint8_t span = SharedMemory.SpectrumAnalyser.span / 1000000 ;
        uint16_t newValue ;
//        lcdDrawText(lcdLastRightPos + 5, 10, "S:", 0);
//        lcdDrawNumber(lcdLastRightPos + 2, 10, reusableBuffer.spectrumAnalyser.span/1000000, attr);
//        lcdDrawText(lcdLastRightPos + 2, 10, "MHz", 0);
				lcd_puts_Pleft( FH, XPSTR( "\015S:\022MHz" ) ) ;
 				lcd_outdezAtt(  17*FW, FH, SharedMemory.SpectrumAnalyser.span / 1000000, attr ) ;
        if (attr)
				{
          newValue = checkIncDec16( span, 1, 80, 0 ) ;
          SharedMemory.SpectrumAnalyser.span = (uint32_t) newValue * 1000000 ;
					if ( newValue != span )
					{
    				SharedMemory.SpectrumAnalyser.step = SharedMemory.SpectrumAnalyser.span / 128 ; // LCD_W ;
						for ( j = 0 ; j < SPECTRUM_NUM_BARS ; j += 1 )
						{
							SharedMemory.SpectrumAnalyser.bars[j] = 0 ;
						}
						ModuleControl[module].step = 0 ;
					}
        }
			}
      break ;
    }
  }
		 
	// Display the data
	pushPlotType( PLOT_BLACK ) ;
  uint8_t peak_y = 1;
//  uint8_t peak_x = 0;
  for ( i = 0 ; i < 128 /*LCD_W*/ ; i += 1 )
  {
    uint8_t h = min<uint8_t >( SharedMemory.SpectrumAnalyser.bars[i] >> 1, 128) ;
    if (h > peak_y)
    {
//      peak_x = i;
      peak_y = h;
    }
		lcd_vline( i + X12OFFSET, /*LCD_H*/64-h, h ) ;
  }
	popPlotType() ;

// Handle the peak bar??
//  int8_t y = max<int8_t>(FH, LCD_H - peak_y - FH);
//  lcdDrawNumber(min<uint8_t>(100, peak_x), y, ((reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2) + peak_x * (reusableBuffer.spectrumAnalyser.span / 128)) / 1000000, TINSIZE);
//  lcdDrawText(lcdLastRightPos, y, "M", TINSIZE);
		 
		 
}


uint8_t byteToBCD( uint8_t value )
{
	return ( ( value >> 4) * 10  ) + (value & 0x0F) ;
}


void editAccessProtocol( uint8_t module, uint8_t event, uint8_t start )
{
	static MState2 mstate2 ;
	uint32_t i ;
	
	EditType = EE_MODEL ;	
	
	if ( AccState == ACC_REG )
	{
		editAccessRegister( module, event ) ;
	}
	else if ( AccState == ACC_BIND )
	{
		editAccessBind( module, event ) ;
	}
	else if ( AccState == ACC_ACCST_BIND )
	{
		editAccstAccessBind( module, event ) ;
	}
	else if ( AccState == ACC_RXOPTIONS )
	{
		editAccessRxOptions( module, event ) ;
	}
	else if ( AccState == ACC_TXOPTIONS )
	{
		editAccessTxOptions( module, event ) ;
	}
	else if ( AccState == ACC_RX_HARDWARE )
	{
		editAccessRxHardware( module, event ) ;
	}
	else if ( AccState == ACC_SHARE )
	{
		editAccessShare( module, event ) ;
	}
	else if ( AccState == ACC_SPECTRUM )
	{
		editAccessSpectrum( module, event ) ;
	}
	else
	{
		struct t_module *pModule = &g_model.Module[module] ;
		struct t_access *pAccess = &g_model.Access[module] ;
		TITLE( XPSTR("Protocol") ) ;

		if ( ( event == EVT_ENTRY ) || AccEntry )
		{
			AccState = ACC_NORMAL ;
			mstate2.m_posVert = 0 ;
			AccEntry = 0 ;
		}
	 
		int8_t  sub    = mstate2.m_posVert ;
  	uint8_t t_pgOfs ;
		uint8_t blink = InverseBlink ;
		uint32_t rows = 13 ;
		if (pAccess->type)
		{
			rows = 8 ;
		}

	//#if defined(PCBX12D) || defined(PCBX10)
	//  t_pgOfs = evalOffsetLarge(sub);
	//#else
		t_pgOfs = evalOffset( sub ) ;
	//#endif

		displayModuleName( 9*FW, 0, module, 0 ) ;
	
		if ( !PopupData.PopupActive )
		{
			event = mstate2.check_columns( event, rows ) ;
		}

		uint8_t y = 1*FH;

		uint8_t subN = 0 ;

		for(;;)
		{
			if(t_pgOfs<=subN)
			{
				uint8_t value ;
				uint8_t newvalue ;
				value = (pModule->protocol == PROTO_OFF) ? 0 : 1 ;
				newvalue = onoffMenuItem( value, y, XPSTR("Enable"), sub==subN ) ;
				if ( newvalue == 0 )
				{
					pModule->protocol = PROTO_OFF ;
				}
  			y+=FH ;
			}
			subN += 1 ;

			if(t_pgOfs<=subN)
			{
				uint8_t attr = 0 ;
				lcd_puts_Pleft( y, PSTR(STR_PROTO));//sub==2 ? INVERS:0);
				if ( sub==subN )
				{
					attr |= blink ;
					checkProtocolOptions( module ) ;
				}
				lcd_puts_P( 6*FW, y, XPSTR("ACCESS") ) ;
				if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
			} 
			subN += 1 ;
	
			if(t_pgOfs<=subN)
			{
//				uint8_t attr = 0 ;
				lcd_puts_Pleft( y, XPSTR("Mode"));
				pAccess->type = checkIndexed( y, XPSTR(FWx10"\001""\012ACCESS    ACCST(D16)"), pAccess->type, (sub==subN) ) ;
				if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
			} 
			subN += 1 ;
	
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, XPSTR("Channels"));
				if (pAccess->type)
				{
					pModule->channels = checkIndexed( y, XPSTR(FWx10"\001""\00216 8"), pModule->channels, (sub==subN) ) ;
				}
				else
				{
					pAccess->numChannels = checkIndexed( y, XPSTR(FWx17"\002""\002 81624"), pAccess->numChannels, (sub==subN) ) ;
				}
				if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
			}
			subN += 1 ;

			// Could remove this, editable in Register menu
//			if(t_pgOfs<=subN)
//			{
//				uint8_t attr = 0 ;
//				attr = (sub == subN) ? blink : 0 ;
//				alphaEditName( 11*FW, y, g_eeGeneral.radioRegistrationID, sizeof(g_eeGeneral.radioRegistrationID), attr, (uint8_t *)XPSTR( "Password") ) ;
//				if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
//			}
//			subN += 1 ;

			if(t_pgOfs<=subN)
			{
				uint8_t attr = 0 ;
			  lcd_puts_Pleft( y, XPSTR("RxNum") ) ;
			  if(sub==subN)
				{
					attr = blink ;
  			  CHECK_INCDEC_H_MODELVAR_0( pModule->pxxRxNum, 63 ) ;
				}
	  	  lcd_outdezAtt(  21*FW, y,  pModule->pxxRxNum, attr ) ;
				if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
			}
			subN += 1 ;

			if (pAccess->type == 0)
			{
				if(t_pgOfs<=subN)
				{
					lcd_puts_Pleft( y, XPSTR("Register") ) ;
				  if(sub==subN)
					{
						lcd_char_inverse( 0, y, 8*FW, 0 ) ;
						if ( checkForMenuEncoderBreak( event ) )
						{
							ModuleControl[module].registerRxName[0] = '\0' ; 
							ModuleControl[module].registerStep = REGISTER_START ;
							ModuleControl[module].timeout = 0 ;
							ModuleSettings[module].mode = MODULE_MODE_REGISTER ;
							AccState = ACC_REG ;
							killEvents( event ) ;
							s_editMode = false ;
					  }
					}
					if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
				}
				subN += 1 ;

				for ( i = 0 ; i < 3 ; i += 1 )
				{
					if(t_pgOfs<=subN)
					{
						uint8_t attr = 0 ;
						uint32_t nameValid = ( ( g_model.Access[module].receiverName[i][0] ) && ( g_model.Access[module].receiverName[i][0] > ' ' ) ) ;
						if (sub==subN)
						{
							attr = InverseBlink ;
							if ( checkForMenuEncoderBreak( event ) )
							{
								if ( nameValid )
								{
									if ( !PopupData.PopupActive )
									{
										PopupData.PopupIdx = 0 ;
										PopupData.PopupActive = 1 ;
 				  				  killEvents(event);
										event = 0 ;		// Kill this off
										ModuleControl[module].bindReceiverIndex = i ;
									}
								
								
									// Handle options - Bind, Options, Share, Delete and Reset
							
									// Temporary, clear name only and reset Rx
	//								g_model.Access[module].receiverName[i][0] = 0 ;

	//								ModuleControl[module].bindReceiverIndex = i ;
	//								ModuleSettings[module].mode = MODULE_MODE_RESET ;


								}
								else
								{
									ModuleControl[module].bindStep = BIND_START ;
									ModuleControl[module].bindReceiverIndex = i ;
									ModuleControl[module].bindReceiverId = i ;
									ModuleControl[module].bindReceiverCount = 0 ;
									ModuleControl[module].timeout = 0 ;
									ModuleSettings[module].mode = MODULE_MODE_BIND ;
									AccState = ACC_BIND ;
								}
  	  					s_editMode = false ;
								killEvents( event ) ;
							}
	//						if ( (ModuleControl[module].bindReceiverIndex == i) && (ModuleSettings[module].mode == MODULE_MODE_RESET) )
	//						{
	//							lcd_puts_P( 19*FW, y, XPSTR("RS") ) ;
	//						}
						}
						lcd_puts_Pleft( y, XPSTR("Receiver") ) ;
						lcd_putc( 8*FW, y, '1' + i ) ;
						if ( nameValid )
						{
							lcd_putsnAtt( 11*FW, y, (char *)g_model.Access[module].receiverName[i], PXX2_LEN_RX_NAME, attr ) ;
						}			
						else
						{
		//					if ( ( attr ) && (AccState == ACC_BIND) )
		//					{
		//						if ( ModuleControl[module].bindReceiverCount )
		//						{
							
		//						}
		//						else
		//						{
		//							lcd_putsAtt( 11*FW, y, XPSTR("----"), BLINK ) ;
		//						}
		//					}
		//					else
		//					{
								lcd_putsAtt( 11*FW, y, XPSTR("BIND"), attr ) ;
		//					}
						}
						if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
					}
					subN += 1 ;
				}
			}

			if(t_pgOfs<=subN)
			{
				lcd_putsAtt( 0, y, XPSTR("Failsafe"), sub==subN ? INVERS : 0 ) ;
				if ( ( PrivateData[1] && ( ( PrivateData[0] & 0x20 ) == 0 ) ) )
				{
					lcd_putsAtt( FW*9, y, "N/A", BLINK ) ;
				}
				else
				{
					if ( pModule->failsafeMode == 0 )
					{
	    			lcd_puts_Pleft( y, XPSTR("\012(Not Set)") ) ;
					}
					if ( sub == subN )
					{
						if ( checkForMenuEncoderLong( event ) )
						{
							s_currIdx = module ;
    				  pushMenu( menuSetFailsafe ) ;
						}
					}
				}
			  if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
			} ;
			subN += 1 ;

			if (pAccess->type == 0)
			{
				if(t_pgOfs<=subN)
				{
					lcd_puts_Pleft( y, XPSTR("Hardware") ) ;
					if ( ModuleControl[module].hwVersion )
					{
						lcd_putc( 11*FW, y, '/' ) ;
						lcd_outdezAtt( 11*FW-1, y, byteToBCD( ModuleControl[module].hwVersion), PREC1 ) ;
						lcd_outdezAtt( 14*FW, y, byteToBCD( ModuleControl[module].swVersion), PREC1 ) ;
	//					lcd_putc( 19*FW, y, ModuleControl[module].moduleId + '0' ) ;
						if ( ModuleControl[module].variant < 4 )
						{
							lcd_putsAttIdx( 14*FW+2, y, XPSTR("\006      FCC   EU-LBTFLEX"), ModuleControl[module].variant, 0 ) ;
						}
					}
				  if(sub==subN)
					{
						lcd_char_inverse( 0, y, 8*FW, 0 ) ;
						if ( checkForMenuEncoderBreak( event ) )
						{
							ModuleControl[module].step = -1 ;
							ModuleControl[module].timeout = 0 ;
							ModuleSettings[module].mode = MODULE_MODE_GET_HARDWARE_INFO ;
							killEvents( event ) ;
  	  				s_editMode = false ;
					  }
					}
					if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
				}
				subN += 1 ;

				if(t_pgOfs<=subN)
				{
					lcd_puts_Pleft( y, XPSTR("Options") ) ;
				  if(sub==subN)
					{
						lcd_char_inverse( 0, y, 7*FW, 0 ) ;
						if ( checkForMenuEncoderBreak( event ) )
						{
							ModuleControl[module].optionsState = 0 ;
							AccState = ACC_TXOPTIONS ;
							killEvents( event ) ;
 	  					s_editMode = false ;
					  }
					}
					if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
				}
				subN += 1 ;

				if(t_pgOfs<=subN)
				{
					lcd_puts_Pleft( y, XPSTR("Spectrum") ) ;
				  if(sub==subN)
					{
						lcd_char_inverse( 0, y, 8*FW, 0 ) ;
						if ( checkForMenuEncoderBreak( event ) )
						{
	//						ModuleControl[module].optionsState = 0 ;
							AccState = ACC_SPECTRUM ;
							killEvents( event ) ;
							s_editMode = 0 ;
					  }
					}
			
					if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
				}
				subN += 1 ;
			}
			
			if (pAccess->type )
			{
				if(t_pgOfs<=subN)
				{
					lcd_puts_Pleft( y, PSTR(STR_COUNTRY) ) ;
					pModule->country = checkIndexed( y, XPSTR(FWx10"\002""\003AmeJapEur"), pModule->country, (sub==subN) ) ;
				  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				}subN++;

				if(t_pgOfs<=subN)
				{
					lcd_puts_Pleft( y, PSTR(STR_BIND) ) ;
  		  	if(sub==subN)
					{
						lcd_char_inverse( 0, y, 4*FW, 0 ) ;
						if ( checkForMenuEncoderLong( event ) )
						{
							s_currIdx = module ;
							pushMenu( menuBindOptions ) ;
						}
					}
					if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				}	subN += 1 ;
			}

			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
  			if(sub==subN)
				{
					lcd_char_inverse( 0, y, 11*FW, 0 ) ;
					if ( checkForMenuEncoderBreak( event ) )
					{
  		  	  BindRangeFlag[module] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
						s_currIdx = module ;
						pushMenu(menuRangeBind) ;
						killEvents( event ) ;
 	  				s_editMode = false ;
					}
				}
				if((y+=FH)>(SCREEN_LINES-1)*FH) break ;
			}
			subN += 1 ;
			break ;
		}
		if ( PopupData.PopupActive )
		{
			Tevent = event ;
			accesspopup( event, module ) ;
  	  s_editMode = false;
		}
	}
}
#endif

#ifndef SMALL
void menuScanDisplay(uint8_t event)
{
	TITLE(XPSTR("Spectrum"));
	static MState2 mstate2;
	mstate2.check_columns(event, 1-1) ;

#if defined(PCBX12D) || defined(PCBX10)
extern uint8_t ScannerData[] ;
	uint32_t i ;
	uint32_t j = 0 ;
	for ( i = 0 ; i < 124 ; i += 1 )
	{
		uint8_t x ;
		x = (ScannerData[j] + ScannerData[j+1]) / 4 ;
		if ( x > 94 )
		{
			x = 94 ;
		}
	  lcd_vline(i+58, 104-x, x ) ;
//	  lcd_vline(i+1, 104-x, x ) ;
		j += 2 ;
	}
#else	 
extern uint8_t ScannerData[] ;
	uint32_t i ;
	uint32_t j = 0 ;
	for ( i = 0 ; i < 62 ; i += 1 )
	{
		uint8_t x ;
		x = (ScannerData[j] + ScannerData[j+1]  + ScannerData[j+2]  + ScannerData[j+3] ) / 20 ;
		if ( x > 48 )
		{
			x = 48 ;
		}
	  lcd_vline(i*2, 63-x, x ) ;
	  lcd_vline(i*2+1, 63-x, x ) ;
		j += 4 ;
	}
#endif
}
#endif


void editOneProtocol( uint8_t event )
{
	uint8_t need_bind_range = 0 ;
	EditType = EE_MODEL ;
	uint8_t dataItems = 6 ;
//#ifdef PCBLEM1
//	dataItems += 1 ;
//#endif
	uint8_t module = EditingModule ;
	if ( module )
	{
		module = 1 ;
	}
	struct t_module *pModule = &g_model.Module[module] ;

//lcd_outhex4( 25*FW, FH, g_model.Module[1].protocol ) ;
//extern uint8_t s_current_protocol[] ;
//lcd_outhex4( 25*FW, 2*FH, s_current_protocol[1] ) ;
//extern uint16_t RfOffDebug1 ;
//extern uint16_t RfOffDebug2 ;
//extern uint16_t RfOffDebug3 ;
//extern uint16_t RfOffDebug4 ;
//extern uint16_t RfOffDebug5 ;
//extern uint16_t RfOffDebug6 ;
//extern uint16_t RfOffDebug7 ;
//extern uint16_t RfOffDebug8 ;
//extern uint16_t RfOffDebug9 ;

//lcd_outhex4( 25*FW, 3*FH, RfOffDebug1 ) ;
//lcd_outhex4( 25*FW, 4*FH, RfOffDebug2 ) ;
//lcd_outhex4( 25*FW, 5*FH, RfOffDebug3 ) ;
//lcd_outhex4( 25*FW, 6*FH, RfOffDebug4 ) ;
//lcd_outhex4( 25*FW, 7*FH, RfOffDebug5 ) ;
//lcd_outhex4( 25*FW, 8*FH, RfOffDebug6 ) ;
//lcd_outhex4( 25*FW, 9*FH, RfOffDebug7 ) ;
//lcd_outhex4( 25*FW, 10*FH, RfOffDebug8 ) ;
//lcd_outhex4( 25*FW, 11*FH, RfOffDebug9 ) ;


//#if defined(PCBX10)
//extern uint16_t ExtRfOffPos ;
//extern uint16_t ExtRfOnPos ;
//  lcd_outhex4( 25*FW, 12*FH, ExtRfOffPos ) ;
//  lcd_outhex4( 25*FW, 13*FH, ExtRfOnPos ) ;
//#endif

//lcd_outhex4( 31*FW, 3*FH, TIM2->ARR ) ;
//lcd_outhex4( 31*FW, 4*FH, TIM2->CNT ) ;
//lcd_outhex4( 31*FW, 5*FH, TIM2->CCR2 ) ;
//lcd_outhex4( 31*FW, 6*FH, TIM2->CCER ) ;
//lcd_outhex4( 31*FW, 7*FH, TIM2->CCMR2 ) ;
//lcd_outhex4( 31*FW, 8*FH, TIM2->DIER ) ;
//lcd_outhex4( 31*FW, 9*FH, TIM2->CR1 ) ;
//lcd_outhex4( 31*FW, 10*FH, TIM1->ARR ) ;
//lcd_outhex4( 31*FW, 11*FH, TIM1->CCR2 ) ;


#ifdef ACCESS
	if ( pModule->protocol == PROTO_ACCESS )
	{
		editAccessProtocol( module, event, 0 ) ;
		return ;
	}
#endif

#ifdef REVX
	if ( module && (pModule->protocol == PROTO_PPM) )
	{ // External module open-drain option
		dataItems += 1 ;
	}
#endif
	if (pModule->protocol == PROTO_PXX)
	{
		dataItems += 4 ;
		need_bind_range |= 5 ;
		if ( pModule->sub_protocol == 3 )	// R9M
		{
			dataItems += 2 ;
		}
		if ( pModule->sub_protocol == 0 )	// D16
		{
#ifdef PCBSKY
			dataItems += ( extraInputInUse( 4 ) == 0 ) ? 2 : 1 ;
#else
			dataItems += 1 ;
#endif
		}
#ifdef ALLOW_EXTERNAL_ANTENNA
		if ( module == 0 )
		{
			dataItems += 1 ;
		}
#endif
	}
	if (pModule->protocol == PROTO_MULTI)
	{
		dataItems += 5 ;
		need_bind_range |= 5 ;
		if ( ( pModule->ppmFrameLength < 0 ) || ( pModule->ppmFrameLength > 4 ) )
		{
			pModule->ppmFrameLength = 0 ;
		}
		if ( event == EVT_ENTRY)
		{
			MultiSetting.protocol[0] = 0 ;
			MultiSetting.subProtocol[0] = 0 ;
			MultiSetting.timeout = 1 ;
		}
	}
	if (pModule->protocol == PROTO_DSM2)
	{
#ifdef ENABLE_DSM_MATCH  		
		if (pModule->sub_protocol >= 3)
		{
			dataItems += 1 ;
		}
#endif
		{
			need_bind_range |= 4 ;
#ifndef PCBLEM1
		  if ( pModule->sub_protocol == DSM_9XR )
#endif
			{
				dataItems += 1 ;
				need_bind_range |= 1 ;
			}
		}
	}

#ifdef XFIRE
	if ( module && (pModule->protocol == PROTO_XFIRE ) )
	{
#ifdef REVX
		dataItems -= 4 ;
#else
		dataItems -= 2 ;
#endif
	}
#endif
	
	if ( pModule->protocol == PROTO_OFF )
	{
		dataItems -= 5 ;
	}

	uint8_t blink = InverseBlink ;

	TITLE( XPSTR("Protocol") ) ;
	static MState2 mstate2 ;

	displayModuleName( 9*FW, 0, module, 0 ) ;
//	lcd_puts_Pleft( 0, "\011Internal") ;
//	if ( module )
//	{
//		lcd_puts_Pleft( 0, "\011Ex" ) ;
//	}
	event = mstate2.check_columns( event, dataItems-1 ) ;

	int8_t  sub    = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz;
  uint8_t t_pgOfs ;

//#if defined(PCBX12D) || defined(PCBX10)
//  t_pgOfs = evalOffsetLarge(sub);
//#else
	t_pgOfs = evalOffset( sub ) ;
//#endif

	uint8_t y = 1*FH;

	uint8_t subN = 0 ;

	if(t_pgOfs<=subN)
	{
		uint8_t value ;
		uint8_t newvalue ;
		value = (pModule->protocol == PROTO_OFF) ? 0 : 1 ;
		newvalue = onoffMenuItem( value, y, XPSTR("Enable"), sub==subN ) ;
		if ( newvalue != value )
		{
#if defined(PCBX12D) || defined (PCBXLITE) || defined (PCBX9D) || defined (PCBX9LITE) || defined(PCBX10)
			if ( module == 0 )
			{
#if defined (PCBX9LITE) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
				killEvents(event) ;
				value = newvalue ? PROTO_ACCESS : PROTO_OFF ;
#else
 #ifdef DISABLE_PXX
				value = newvalue ? PROTO_PPM : PROTO_OFF ;
 #else
  #ifdef PCBT16
				value = newvalue ? PROTO_MULTI : PROTO_OFF ;
  #else
				value = newvalue ? 1 : PROTO_OFF ;
  #endif
 #endif
#endif
			}
			else
#endif
			{
#if defined (PCBXLITE) || defined (PCBX9LITE)
				value = newvalue ? 1 : PROTO_OFF ;
#else
 #if defined (PCBLEM1)
				value = newvalue ? 2 : PROTO_OFF ;
 #else
				value = newvalue ? 0 : PROTO_OFF ;
 #endif
#endif
			}
		}
		else
		{
			value = pModule->protocol ;
		}
		if ( pModule->protocol != value )
		{
			pModule->protocol = value ;
#ifdef ACCESS
			if ( value == PROTO_ACCESS )
			{
				AccEntry = 1 ;
				return ;
			}
#endif
		}
  	y+=FH ;
	} subN++ ;

	if ( pModule->protocol != PROTO_OFF )
	{
		if(t_pgOfs<=subN)
		{
  		lcd_puts_Pleft( y, PSTR(STR_PROTO));//sub==2 ? INVERS:0);
			uint8_t attr = 0 ;
			if ( (sub==subN) && ( subSub==0 ) )
			{
				attr |= blink ;
#ifdef ACCESS
				if (checkProtocolOptions( module ) )
				{
					if ( g_model.Module[module].protocol == PROTO_ACCESS )
					{
						AccEntry = 1 ;
					}
				}
#else
				checkProtocolOptions( module ) ;
#endif

			}
			displayProtocol( 6*FW, y, pModule->protocol, attr ) ;
//			lcd_putsAttIdx( 6*FW, y, PSTR(STR_PROT_OPT), pModule->protocol, attr ) ;
			if ( pModule->protocol == PROTO_PPM )
			{
				uint8_t x ;
				attr = 0 ;
				if ( sub==subN )
				{
				 	Columns = 2 ;
				}
				lcd_puts_Pleft( y, PSTR(STR_23_US) );
				x = 11*FW ;
				if ( ( pModule->channels > 8 ) || (pModule->channels < -4) )
				{
					pModule->channels = 0 ;		// Correct if wrong from DSM
				}
				uint8_t chans = pModule->channels + 4 ;
//					attr = LEFT ;
				attr = 0 ;
				if ( (sub==subN) && ( subSub==1 ) )
				{
					attr |= blink ;
				}
				lcd_outdezAtt(  x, y, chans + 4, attr ) ;
				if ( attr )
				{
 		      CHECK_INCDEC_H_MODELVAR_0( chans, 12 ) ;
					pModule->channels = chans - 4 ;
				}
  			lcd_putsAtt( Lcd_lastPos, y, PSTR( STR_PPMCHANNELS ), attr ) ;
				attr = 0 ;
				if ( (sub==subN) && ( subSub==2 ) )
				{
					attr |= blink ;
				}
				lcd_outdezAtt(  x+7*FW-1, y,  (pModule->ppmDelay*50)+300, attr);
				if ( attr )
				{
					CHECK_INCDEC_H_MODELVAR( pModule->ppmDelay,-4,10);
				}
			}
			else
			{
				attr = 0 ;
				if ( sub==subN )
				{
				 	Columns = 1 ;
				}
				if ( (sub==subN) && ( subSub==1 ) )
				{
					attr |= blink ;
				}
#ifndef PCBLEM1
		  	if ( ( pModule->protocol == PROTO_DSM2) && ( pModule->sub_protocol == DSM_9XR ) )
  			{
				  lcd_puts_Pleft( y, XPSTR("\013Chans") );
 	    		lcd_outdezAtt(  21*FW, y, pModule->channels, attr ) ;
				}
				else
#endif
				{
				  lcd_puts_Pleft( y, PSTR(STR_13_RXNUM) );
	 		    lcd_outdezAtt(  21*FW, y,  pModule->pxxRxNum, attr ) ;
				}
				if ( attr )
				{
#ifndef PCBLEM1
			  	if ( ( pModule->protocol == PROTO_DSM2) && ( pModule->sub_protocol == DSM_9XR ) )
					{
  	          CHECK_INCDEC_H_MODELVAR( pModule->channels, 6, 14) ;
					}
					else
#endif
					{
						uint8_t max = 63 ;
//						if ( pModule->protocol == PROTO_MULTI )
//						{
//							max = 15 ;
//						}
  		      CHECK_INCDEC_H_MODELVAR_0( pModule->pxxRxNum, max ) ;
					}
				}
			}
			if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
		} subN += 1 ;

//#ifdef PCBLEM1
//		if(t_pgOfs<=subN)
//		{
//			uint8_t attr = 0 ;
//			if ( sub==subN )
//			{
//				attr |= blink ;
//			}
//			lcd_puts_Pleft( y, XPSTR("Chans") );
// 	    lcd_outdezAtt(  21*FW, y, pModule->channels, attr ) ;
//			if ( attr )
//			{
//        CHECK_INCDEC_H_MODELVAR( pModule->channels, 6, 14) ;
//			}
//			if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
//		} subN += 1 ;
//#endif
			 
		if(t_pgOfs<=subN)
		{
	  	lcd_puts_Pleft( y, XPSTR("Detect Telem."));
			g_model.ForceTelemetryType = checkIndexed( y, XPSTR(FWx15"\001""\006  AutoSelect"), g_model.ForceTelemetryType, (sub==subN) ) ;
			if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
		}
		subN += 1 ;

		if ( pModule->protocol == PROTO_MULTI )
		{
			uint8_t attr = 0 ;
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_PROTO));
				uint8_t oldValue = pModule->sub_protocol & 0x3F ;
				oldValue |= pModule->exsub_protocol << 6 ;
				uint8_t svalue = oldValue ;
				
//				svalue = editMultiProtocol( svalue, ( sub == subN ) ) ;

				if ( sub == subN )
				{
					attr = blink ;
			 		CHECK_INCDEC_H_MODELVAR_0( svalue, 127 ) ;	// Limited to 8 bits
				}

				displayMultiProtocol( svalue, y, attr ) ;

				pModule->sub_protocol = ( svalue & 0x3F) + (pModule->sub_protocol & 0xC0) ;
				pModule->exsub_protocol = svalue >> 6 ;
				if( svalue != oldValue )
				{
					pModule->channels &= 0x8F ;
					TelemetryType = TEL_UNKNOWN ;
					MultiSetting.protocol[0] = 0 ;
					MultiSetting.subProtocol[0] = 0 ;
					MultiSetting.timeout = 1 ;
				}
				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			}
			subN++;
		
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_TYPE));
				uint8_t x = (pModule->sub_protocol & 0x3F ) | (pModule->exsub_protocol << 6 ) ;

#ifndef SMALL  			
				if ( x == M_SCANNER )
				{
					lcd_puts_Pleft( y, XPSTR("Display Scan"));
					if(sub==subN)
					{
						lcd_char_inverse( 0, y, 12*FW, 0 ) ;
						if ( checkForMenuEncoderBreak( event ) )
						{
							pushMenu(menuScanDisplay) ;
						}
					}
				}
				else
#endif
				{
					attr = (pModule->channels >> 4) &0x07 ;
					uint8_t oldValue = attr ;

					if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( attr, 7 ) ;
					if ( attr != oldValue )
					{
						MultiSetting.subProtocol[0] = 0 ;
					}
					attr = displayMultiSubProtocol( x, attr, y, (sub==subN) ? blink : 0 ) ;
					pModule->channels = ( attr << 4) + (pModule->channels & 0x8F);
				}
				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			}
			subN += 1 ;
		
			if(t_pgOfs<=subN)
			{
				uint32_t x = (pModule->sub_protocol & 0x3F ) | (pModule->exsub_protocol << 6 ) ;
				lcd_puts_Pleft( y, PSTR(STR_MULTI_AUTO));
				uint8_t value = (pModule->sub_protocol>>6)&0x01 ;
				lcd_putsAttIdx(  9*FW, y, XPSTR(M_NY_STR), value, (sub==subN && subSub==0 ? blink:0) );
				multiOption( 21*FW, y, pModule->option_protocol, (sub==subN && subSub==1 ? blink:0), x ) ;
				if(sub==subN)
				{
					Columns = 1;
					if(subSub==0 && s_editing)
					{
						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
						pModule->sub_protocol = (value<<6) + (pModule->sub_protocol&0xBF);
					}
					if(subSub==1 && s_editing)
						CHECK_INCDEC_H_MODELVAR(pModule->option_protocol, -128, 127);
				}
				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			}
			subN++;
		
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_MULTI_POWER));
				// Power stored in ppmNCH bit7 & Option stored in option_protocol
				uint8_t value = (pModule->channels>>7)&0x01 ;
				lcd_putsAttIdx(  6*FW, y, XPSTR(M_LH_STR), value, ((sub==subN && subSub == 0) ? blink:0) );
				lcd_putsAttIdx(  17*FW, y, XPSTR("\002 7 8 91011"), pModule->ppmFrameLength, ((sub==subN && subSub == 1) ? blink:0) );
				if(sub==subN)
				{
					Columns = 1;
					if(subSub==0 && s_editing)
					{
						uint8_t oldValue = value ;
						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
						pModule->channels = (value<<7) + (pModule->channels&0x7F);
						if ( oldValue != value )
						{
							checkMultiPower() ;
							killEvents(event) ;
						}
					}
					if(subSub==1 && s_editing)
						CHECK_INCDEC_H_MODELVAR_0( pModule->ppmFrameLength, 4);
				}
				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			}
			subN += 1 ;
			if(t_pgOfs<=subN)
			{
				lcd_putsAtt( 0, y, XPSTR("Failsafe"), sub==subN ? INVERS : 0 ) ;
				if ( ( PrivateData[1] && ( ( PrivateData[0] & 0x20 ) == 0 ) ) )
				{
					lcd_putsAtt( FW*9, y, "N/A", BLINK ) ;
				}
				else
				{
					if ( pModule->failsafeMode == 0 )
					{
	    			lcd_puts_Pleft( y, XPSTR("\012(Not Set)") ) ;
					}
					if ( sub == subN )
					{
						if ( checkForMenuEncoderLong( event ) )
						{
							s_currIdx = module ;
    				  pushMenu( menuSetFailsafe ) ;
						}
					}
				}
			  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			}subN++;
		}

		if ( pModule->protocol == PROTO_DSM2 )
		{
			if(t_pgOfs<=subN)
  		{
			  lcd_puts_Pleft( y, PSTR(STR_DSM_TYPE));
  			int8_t x ;
  		  x = pModule->sub_protocol ;
#ifdef PCBLEM1
			  lcd_putsAttIdx( 10*FW, y, "\010DSM2(10)DSM2(11)DSMS    DSMX    DSMP    ", x, (sub==subN ? blink:0) ) ;
				if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( pModule->sub_protocol,3);
#else
			  lcd_putsAttIdx( 10*FW, y, XPSTR(DSM2_STR), x, (sub==subN ? blink:0) ) ;
				if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( pModule->sub_protocol,3);

				if (pModule->sub_protocol == DSM_9XR)
				{
					if ( ( pModule->channels < 6 ) || ( pModule->channels > 14 ) )
					{
						pModule->channels = 6 ;		// Correct if wrong from DSM
					}
				}
#endif
  		  if((y+=FH)>(SCREEN_LINES-1)*FH) return;
  		}
			subN += 1 ;
		}

		if ( pModule->protocol == PROTO_PPM )
		{
			if(t_pgOfs<=subN)
  		{
 	  	  lcd_puts_Pleft(    y, PSTR(STR_PPMFRAME_MSEC));
				uint8_t attr = PREC1 ;
 	  	  if(sub==subN) { attr = blink | PREC1 ; CHECK_INCDEC_H_MODELVAR( pModule->ppmFrameLength,-20,20) ; }
 	  	  lcd_outdezAtt(  14*FW-1, y, (int16_t)pModule->ppmFrameLength*5 + 225, attr ) ;
  		  if((y+=FH)>(SCREEN_LINES-1)*FH) return;
  		}
			subN += 1 ;

#ifdef REVX
			if ( module == 1 )
			{
				if(t_pgOfs<=subN)
  	  	{
  	  		lcd_puts_Pleft(    y, XPSTR(" PPM Drive"));
  				uint8_t x ;
  	  		x = pModule->ppmOpenDrain ;
					pModule->ppmOpenDrain = checkIndexed( y, XPSTR(FWx12"\001""\011OpenDrainFullDrive"), pModule->ppmOpenDrain, (sub==subN) ) ;
					if ( x != pModule->ppmOpenDrain )
					{
						module_output_active() ;		// Changes drive as well
					}
					if((y+=FH)>(SCREEN_LINES-1)*FH) return;
  	  	}
				subN += 1 ;
			}
#endif

			if(t_pgOfs<=subN)
			{
				  uint8_t attr = 0 ;
				  lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
				  if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( pModule->pulsePol,1);}
				  lcd_putsAttIdx( 11*FW, y, PSTR(STR_POS_NEG),pModule->pulsePol,attr );
				  if((y+=FH)>(SCREEN_LINES-1)*FH) return;
			}subN++;
		}

		if (pModule->protocol == PROTO_PXX)
		{
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_TYPE) ) ;
#ifdef PCBX9LITE
				{
					uint8_t attr = 0 ;
					if (sub==subN)
					{
						attr = InverseBlink ;
					}
					lcd_putsAttIdx( 10*FW, y, XPSTR("\006D16(X)D8(D) LRP   R9M   "), pModule->sub_protocol, attr ) ;
					if ( attr )
					{
						pModule->sub_protocol = checkOutOfOrder( pModule->sub_protocol, (uint8_t *)PxxProtocolMap, 3 ) ;
					}
				}
#else
				pModule->sub_protocol = checkIndexed( y, XPSTR(FWx10"\003""\006D16(X)D8(D) LRP   R9M   "), pModule->sub_protocol, (sub==subN) ) ;
#endif
			  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			}subN++;
			if(t_pgOfs<=subN)
			{
			  lcd_puts_Pleft( y, XPSTR("Chans") );
				pModule->channels = checkIndexed( y, XPSTR(FWx10"\001""\00216 8"), pModule->channels, (sub==subN) ) ;
			  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			}subN++;
			if ( pModule->sub_protocol == 3 )	// R9M
			{
				if(t_pgOfs<=subN)
				{
					uint32_t temp ;
			  	lcd_puts_Pleft( y, XPSTR("Flex mode") );
					temp = checkIndexed( y, XPSTR(FWx10"\002\006""   OFF915MHz868MHz"), pModule->r9MflexMode, (sub==subN) ) ;
					if ( temp )
					{
						if ( temp != pModule->r9MflexMode )
						{
							AlertType = ALERT_TYPE ;
							AlertMessage = "\001Requires R9M non-\037\001certified firmware\037\001Check Frequency for\037\006your region" ;
						}
					}
//					if ( pModule->country == 2 )
//					{
//						if ( temp == 1 )
//						{
//							if ( pModule->r9MflexMode == 2 )
//							{
//								temp = 0 ;
//							}
//							else
//							{
//								temp = 2 ;
//							}
//						}
//					}
					pModule->r9MflexMode = temp ;
					if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				}subN++;
					
				if(t_pgOfs<=subN)
				{
					char *s ; 
					s = XPSTR(FWx10"\003""\004  10 100 5001000") ;
					if ( ( pModule->country == 2 ) && (pModule->r9MflexMode == 0) )
					{
						s = XPSTR(FWx10"\003""\011  25(8ch) 25(16ch)200(16ch)500(16ch)") ;
					}
				  lcd_puts_Pleft( y, XPSTR("Power (mW)") );
					pModule->r9mPower = checkIndexed( y, s, pModule->r9mPower, (sub==subN) ) ;
				  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				}subN++;
			}

			if ( pModule->sub_protocol == 0 )	// D16
			{
				if(t_pgOfs<=subN)
				{
					pModule->pxxDoubleRate = onoffMenuItem( pModule->pxxDoubleRate, y, XPSTR("Double Rate"), sub==subN ) ;
				  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
				}subN++;
				
#ifdef PCBSKY
				if ( extraInputInUse( 4 ) == 0 )
				{
					if(t_pgOfs<=subN)
					{
						pModule->pxxHeartbeatPB14 = onoffMenuItem( pModule->pxxHeartbeatPB14, y, XPSTR("PB14 Heartbeat"), sub==subN ) ;
extern uint16_t XjtHbeatOffset ;
						if ( ( XjtHbeatOffset >= 0x2400 ) && ( XjtHbeatOffset <= 0x2600 ) )
						{
							lcd_putc( 122, y, '\202');
						}
					  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
					}subN++;
				}
#endif

#ifdef ALLOW_EXTERNAL_ANTENNA
				if ( module == 0 )
				{
					if(t_pgOfs<=subN)
					{
						pModule->externalAntenna = onoffMenuItem( pModule->externalAntenna, y, XPSTR("Use External Ant."), sub==subN ) ;
					  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
					}subN++;
				}
#endif
			}

			if(t_pgOfs<=subN)
			{
				lcd_putsAtt( 0, y, XPSTR("Failsafe"), sub==subN ? INVERS : 0 ) ;
				if ( pModule->failsafeMode == 0 )
				{
	    		lcd_puts_Pleft( y, XPSTR("\012(Not Set)") ) ;
				}
				if ( sub == subN )
				{
					if ( checkForMenuEncoderLong( event ) )
					{
						s_currIdx = module ;
    			  pushMenu( menuSetFailsafe ) ;
					}
				}
			  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			}subN++;
			
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_COUNTRY) ) ;
				pModule->country = checkIndexed( y, XPSTR(FWx10"\002""\003AmeJapEur"), pModule->country, (sub==subN) ) ;
			  if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			}subN++;
		}

		if(t_pgOfs<=subN)
		{
			uint8_t attr = 0 ;
 			lcd_puts_Pleft( y, PSTR(STR_PPM_1ST_CHAN));
 			if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( pModule->startChannel, 16 ) ; }
			lcd_outdezAtt(  14*FW, y, pModule->startChannel + 1, attr ) ;
			if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
		} subN += 1 ;

		uint8_t brNeeded = need_bind_range ;
		if ( brNeeded & 1 )
		{
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_BIND) ) ;
				if ( (pModule->protocol == PROTO_MULTI ) && ( PrivateData[1] ) )
				{
					div_t qr ;
					lcd_puts_Pleft( y, XPSTR("\006(Ver  . .  .  )" ) ) ;
					lcd_putc( 11*FW, y, PrivateData[1] + '0' ) ;
					lcd_putc( 13*FW, y, PrivateData[2] + '0' ) ;
					qr = div( PrivateData[3], 10 ) ;
					lcd_putc( 16*FW, y, qr.rem + '0' ) ;
					if ( qr.quot )
					{
						lcd_putc( 15*FW, y, qr.quot + '0' ) ;
					}
					qr = div( PrivateData[4], 10 ) ;
					lcd_putc( 19*FW, y, qr.rem + '0' ) ;
					if ( qr.quot )
					{
						lcd_putc( 18*FW, y, qr.quot + '0' ) ;
					}
					if ( PrivateData[0] & 0x08 )
					{
						lcd_char_inverse( 0*FW, y, 4*FW, 1 ) ;
					}
				}
		  	if ( ( pModule->protocol == PROTO_DSM2) && ( pModule->sub_protocol == DSM_9XR ) )
				{
					// Display mode
					lcd_puts_Pleft( y, XPSTR("\006(Mode  )" ) ) ;
					lcd_putc( 12*FW, y, (g_model.dsmMode & 0x07) + '0' ) ;
					
// Temp					
//					if ( sub==subN )
//					{
//						uint8_t att ;
//						att = 0 ;
//					 	Columns = 1 ;
//						if ( (sub==subN) && ( subSub==1 ) )
//						{
//							att = INVERS ;
//						}
//						g_model.dsmMode = checkIncDec( g_model.dsmMode, 0, 7, 0 ) ;
//						lcd_putcAtt( 12*FW, y, (g_model.dsmMode & 0x07) + '0', att ) ;
//					}



// End of Temp

				}

				// XJT version display here
				if ( pModule->protocol == PROTO_PXX )
				{
					lcd_puts_Pleft( y, XPSTR("\006(Ver      )" ) ) ;
extern uint16_t XjtVersion ;
					lcd_outdezAtt(  14*FW, y, XjtVersion, 0 ) ;
				}
  		  if(sub==subN)
				{
					lcd_char_inverse( 0, y, 4*FW, 0 ) ;
					if ( checkForMenuEncoderLong( event ) )
					{
						s_currIdx = module ;
						if ( pModule->protocol == PROTO_PXX )
						{
							pushMenu( menuBindOptions ) ;
						}
						else
						{
	  					BindRangeFlag[module] = PXX_BIND ;		    	//send bind code
#ifdef PCBLEM1
void startDsmBind() ;
							startDsmBind() ;
#endif
							pushMenu( menuRangeBind ) ;
						}
					}
				}
				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			} subN += 1 ;
		}
		if ( brNeeded & 5 )
		{
			if(t_pgOfs<=subN)
			{
				lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
				if ( (pModule->protocol == PROTO_MULTI ) && ( PrivateData[1] ) )
				{
//						lcd_outdezAtt(  13*FW, y, PrivateData[0], 0 ) ;
//						lcd_outdezAtt(  17*FW, y, FrskyHubData[FR_RXRSI_COPY], 0 ) ;
//						lcd_outdezAtt(  17*FW, y, RxLqi, 0 ) ;
					uint8_t sp = pModule->sub_protocol & 0x3F ;
					sp |= pModule->exsub_protocol << 6 ;
					if ( ( sp == M_FRSKYX ) || ( sp == M_FrskyD ) || ( sp == M_FRSKYX2 ) )
					{
						lcd_puts_Pleft( y, XPSTR("\017Lqi") ) ;
						lcd_outdezAtt(  21*FW, y, TxLqi, 0 ) ;
					}
				}
  		  if(sub==subN)
				{
					lcd_char_inverse( 0, y, 11*FW, 0 ) ;
					if ( checkForMenuEncoderLong( event ) )
					{
  		  	  BindRangeFlag[module] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
						s_currIdx = module ;
#ifdef PCBLEM1
void startDsmRangeCheck() ;
						startDsmRangeCheck() ;
#endif
						pushMenu(menuRangeBind) ;
					}
				}
				if((y+=FH)>(SCREEN_LINES-1)*FH) return ;
			} subN += 1 ;
		}
	} 
}


void menuProcProtocol(uint8_t event)
{
#ifdef PCBT12
	uint8_t dataItems = 2 ;
#else
 #if defined(PCBLEM1)				
	uint8_t dataItems = 2 ;
 #else
	uint8_t dataItems = 3 ;
 #endif	
#endif	
	uint8_t need_range = 0 ;
	uint32_t i ;
//	EditType = EE_MODEL ;
//	uint8_t dataItems = 15 ;
//#ifdef REVX
//	if (g_model.Module[1].protocol == PROTO_PPM)
//	{ // External module open-drain option
//		dataItems += 1 ;
//	}
//#endif

#ifdef PCBT12
	g_model.Module[0].protocol = PROTO_OFF ;
#endif
	switch ( g_model.Module[0].protocol )
	{
		case PROTO_PXX :
		case PROTO_MULTI :
		case PROTO_DSM2 :
			need_range = 1 ;
			dataItems += 1 ;
		break ;
	}

#if defined(PCBLEM1)				
	g_model.Module[1].protocol = PROTO_OFF ;
#endif
	switch ( g_model.Module[1].protocol )
	{
		case PROTO_PXX :
		case PROTO_MULTI :
		case PROTO_DSM2 :
			need_range |= 2 ;
			dataItems += 1 ;
		break ;
	}



//	if (g_model.Module[0].protocol == PROTO_PXX)
//	{
//		dataItems += 4 ;
//		need_bind_range |= 5 ;
//	}
//	if (g_model.Module[1].protocol == PROTO_PXX)
//	{
//		dataItems += 4 ;
//		need_bind_range |= 0x0A ;
//	}
//	if (g_model.Module[0].protocol == PROTO_MULTI)
//	{
//		dataItems += 4 ;
//		need_bind_range |= 5 ;
//		if ( ( g_model.Module[0].ppmFrameLength < 0 ) || ( g_model.Module[0].ppmFrameLength > 4 ) )
//		{
//			g_model.Module[0].ppmFrameLength = 0 ;
//		}
//	}
//	if (g_model.Module[1].protocol == PROTO_MULTI)
//	{
//		dataItems += 4 ;
//		need_bind_range |= 0x0A ;
//		if ( ( g_model.Module[1].ppmFrameLength < 0 ) || ( g_model.Module[1].ppmFrameLength > 4 ) )
//		{
//			g_model.Module[1].ppmFrameLength = 0 ;
//		}
//	}
//	if (g_model.Module[0].protocol == PROTO_DSM2)
//	{
//#ifdef ENABLE_DSM_MATCH  		
//		if (g_model.Module[0].sub_protocol >= 3)
//		{
//			dataItems += 4 ;
//		}
//#endif
//		{
//			need_bind_range |= 4 ;
//		  if ( g_model.Module[0].sub_protocol == DSM_9XR )
//			{
//				dataItems += 4 ;
//				need_bind_range |= 1 ;
//			}
//		}
//	}
//	if (g_model.Module[1].protocol == PROTO_DSM2)
//	{
//#ifdef ENABLE_DSM_MATCH  		
//		if (g_model.Module[1].sub_protocol >= 3)
//		{
//			dataItems += 4 ;
//		}
//#endif
//		{
//			need_bind_range |= 8 ;
//		  if ( g_model.Module[1].sub_protocol == DSM_9XR )
//			{
//				dataItems += 1 ;
//				need_bind_range |= 2 ;
//			}
//		}
//	}

//#ifdef REVX
//	if ( g_model.Module[1].protocol == PROTO_XFIRE )
//	{
//		dataItems -= 4 ;
//	}
//#endif

//	if ( g_model.Module[0].protocol == PROTO_OFF )
//	{
//		dataItems -= 5 ;
//	}
//	if ( g_model.Module[1].protocol == PROTO_OFF )
//	{
//		dataItems -= 5 ;
//	}

	uint8_t blink = InverseBlink ;
	
	TITLE( XPSTR("Protocol") ) ;
	static MState2 mstate2 ;

	event = mstate2.check_columns( event, dataItems-1 ) ;
	
	int8_t  sub    = mstate2.m_posVert ;
//	uint8_t subSub = g_posHorz;
//  uint8_t t_pgOfs ;

//	t_pgOfs = evalOffset(sub);

	uint8_t y = 1*FH;

	uint8_t subN = 0 ;


	uint8_t attr = 0 ;

	if ( sub == subN )
	{
		attr = blink ;
	}
	lcd_putsAtt( 0, y, PSTR(STR_Trainer), attr ) ;
		
  y += FH ;
	subN += 1 ;

#ifdef PCBT12
	i = 1 ;
#else
 #if defined(PCBLEM1)				
	i = 0 ;
 #else
	for ( i = 0 ; i < 2 ; i += 1 )
 #endif
#endif
	{
		
		attr = 0 ;

		if ( sub == subN )
		{
			attr = blink ;
		}
		displayModuleName( 0, y, i, attr ) ;

		displayProtocol( 10*FW, y, g_model.Module[i].protocol, 0 ) ;
  	y += FH ;
		subN += 1 ;

	}
		
#ifdef PCBT12
	if ( sub <= 1 )
#else
 #if defined(PCBLEM1)				
	if ( sub <= 1 )
 #else
	if ( sub <= 2 )
 #endif
#endif
	{
  	switch (event)
		{
  		case EVT_KEY_FIRST(KEY_MENU) :
  		case EVT_KEY_BREAK(BTN_RE) :
				killEvents(event) ;
				switch ( sub )
				{
					case 0 :
						pushMenu(menuProcTrainProtocol) ;
					break ;
					case 1 :
#ifdef PCBT12
						EditingModule = 1 ;
#else
						EditingModule = 0 ;
#endif
						pushMenu( editOneProtocol ) ;
					break ;
#ifndef PCBT12
 #ifndef PCBLEM1
					case 2 :
						EditingModule = 1 ;
						pushMenu( editOneProtocol ) ;
					break ;
 #endif
#endif
				}

			break ;
		}
	}
	
//	uint32_t module ;
//	for ( module = 0 ; module < 2 ; module += 1 )
//	{
//		struct t_module *pModule = &g_model.Module[module] ;
		
//		if(t_pgOfs<=subN)
//		{
//			uint8_t value ;
//			uint8_t newvalue ;
//			value = (pModule->protocol == PROTO_OFF) ? 0 : 1 ;
//			newvalue = onoffMenuItem( value, y, XPSTR("Internal Module"), sub==subN ) ;
//			if ( module )
//			{
//				lcd_puts_Pleft( y, "Ex" ) ;
//			}
//			if ( newvalue != value )
//			{
//#ifdef PCBX12D
//				value = newvalue ? 1 : PROTO_OFF ;
//#else
//				value = newvalue ? 0 : PROTO_OFF ;
//#endif
//			}
//			else
//			{
//				value = pModule->protocol ;
//			}
//			if ( pModule->protocol != value )
//			{
//				pModule->protocol = value ;
//			}
//  	  if((y+=FH)>7*FH) return;
//		} subN++;
	
//		if ( pModule->protocol != PROTO_OFF )
//		{
//			if(t_pgOfs<=subN)
//			{
//  		  lcd_puts_Pleft( y, PSTR(STR_PROTO));//sub==2 ? INVERS:0);
//				uint8_t attr = 0 ;
//				if ( (sub==subN) && ( subSub==0 ) )
//				{
//					attr |= blink ;
//					checkProtocolOptions( module ) ;
//				}
//				lcd_putsAttIdx(  6*FW, y, PSTR(STR_PROT_OPT), pModule->protocol, attr ) ;
//				if ( pModule->protocol == PROTO_PPM )
//			  {
//					uint8_t x ;
//					attr = 0 ;
//					if ( sub==subN )
//					{
//				 		Columns = 2 ;
//					}
//				  lcd_puts_Pleft( y, PSTR(STR_23_US) );
//					x = 11*FW ;
//					if ( ( pModule->channels > 8 ) || (pModule->channels < -4) )
//					{
//						pModule->channels = 0 ;		// Correct if wrong from DSM
//					}
//					uint8_t chans = pModule->channels + 4 ;
////					attr = LEFT ;
//					attr = 0 ;
//					if ( (sub==subN) && ( subSub==1 ) )
//					{
//						attr |= blink ;
//					}
//					lcd_outdezAtt(  x, y, chans + 4, attr ) ;
//					if ( attr )
//					{
// 		      	CHECK_INCDEC_H_MODELVAR_0( chans, 12 ) ;
//						pModule->channels = chans - 4 ;
//					}
//  				lcd_putsAtt( Lcd_lastPos, y, PSTR( STR_PPMCHANNELS ), attr ) ;
//					attr = 0 ;
//					if ( (sub==subN) && ( subSub==2 ) )
//					{
//						attr |= blink ;
//					}
//					lcd_outdezAtt(  x+7*FW-1, y,  (pModule->ppmDelay*50)+300, attr);
//					if ( attr )
//					{
//						CHECK_INCDEC_H_MODELVAR( pModule->ppmDelay,-4,10);
//					}
//				}
//				else
//				{
//					attr = 0 ;
//					if ( sub==subN )
//					{
//				 		Columns = 1 ;
//					}
//					if ( (sub==subN) && ( subSub==1 ) )
//					{
//						attr |= blink ;
//					}
//		  	  if ( ( pModule->protocol == PROTO_DSM2) && ( pModule->sub_protocol == DSM_9XR ) )
//  			  {
//				    lcd_puts_Pleft( y, XPSTR("\013Chans") );
// 	    		  lcd_outdezAtt(  21*FW, y, pModule->channels, attr ) ;
//					}
//					else
//					{
//				    lcd_puts_Pleft( y, PSTR(STR_13_RXNUM) );
//	 		      lcd_outdezAtt(  21*FW, y,  pModule->pxxRxNum, attr ) ;
//					}
//					if ( attr )
//					{
//			  	  if ( ( pModule->protocol == PROTO_DSM2) && ( pModule->sub_protocol == DSM_9XR ) )
//						{
//  	          	CHECK_INCDEC_H_MODELVAR( pModule->channels, 6, 14) ;
//						}
//						else
//						{
//							uint8_t max = 124 ;
//							if ( pModule->protocol == PROTO_MULTI )
//							{
//								max = 15 ;
//							}
//  		      	CHECK_INCDEC_H_MODELVAR_0( pModule->pxxRxNum, max ) ;
//						}
//					}
//				}
//			  if((y+=FH)>7*FH) return ;
//			} subN += 1 ;

//			if(t_pgOfs<=subN)
//			{
//	  		lcd_puts_Pleft( y, XPSTR("Detect Telem."));
//				g_model.ForceTelemetryType = checkIndexed( y, XPSTR(FWx15"\001""\006  AutoSelect"), g_model.ForceTelemetryType, (sub==subN) ) ;
//				if((y+=FH)>7*FH) return ;
//			}
//			subN += 1 ;

//			if ( pModule->protocol == PROTO_MULTI )
//			{
//				uint8_t attr = 0 ;
//				if(t_pgOfs<=subN)
//				{
//					lcd_puts_Pleft( y, PSTR(STR_MULTI_PROTO));
//					uint8_t oldValue = pModule->sub_protocol ;
//					uint8_t svalue = oldValue & 0x3F ;
				
//					svalue = editMultiProtocol( svalue, ( sub == subN ) ) ;

//					if ( sub == subN )
//					{
//						attr = blink ;
//					}

//					displayMultiProtocol( svalue, y, attr ) ;

//					pModule->sub_protocol = svalue + (pModule->sub_protocol & 0xC0) ;
//					if(pModule->sub_protocol != oldValue)
//					{
//						pModule->channels &= 0x8F ;
//						TelemetryType = TEL_UNKNOWN ;
//					}
//					if((y+=FH)>7*FH) return ;
//				}
//				subN++;
		
//				if(t_pgOfs<=subN)
//				{
//					lcd_puts_Pleft( y, PSTR(STR_MULTI_TYPE));
//					uint8_t x = pModule->sub_protocol&0x3F ;
  			
//					attr = (pModule->channels >> 4) &0x07 ;
//					if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( attr, 7 ) ;
//					attr = displayMultiSubProtocol( x, attr, y, (sub==subN) ? blink : 0 ) ;
//					pModule->channels = ( attr << 4) + (pModule->channels & 0x8F);

//					if((y+=FH)>7*FH) return ;
//				}
//				subN += 1 ;
		
//				if(t_pgOfs<=subN)
//				{
//					lcd_puts_Pleft( y, PSTR(STR_MULTI_AUTO));
//					uint8_t value = (pModule->sub_protocol>>6)&0x01 ;
//					lcd_putsAttIdx(  9*FW, y, XPSTR(M_NY_STR), value, (sub==subN && subSub==0 ? blink:0) );
//					multiOption( 21*FW, y, pModule->option_protocol, (sub==subN && subSub==1 ? blink:0), pModule->sub_protocol & 0x3F ) ;
//					if(sub==subN)
//					{
//						Columns = 1;
//						if(subSub==0 && s_editing)
//						{
//							CHECK_INCDEC_H_MODELVAR_0(value, 1 );
//							pModule->sub_protocol = (value<<6) + (pModule->sub_protocol&0xBF);
//						}
//						if(subSub==1 && s_editing)
//							CHECK_INCDEC_H_MODELVAR(pModule->option_protocol, -128, 127);
//					}
//					if((y+=FH)>7*FH) return ;
//				}
//				subN++;
		
//				if(t_pgOfs<=subN)
//				{
//					lcd_puts_Pleft( y, PSTR(STR_MULTI_POWER));
//					// Power stored in ppmNCH bit7 & Option stored in option_protocol
//					uint8_t value = (pModule->channels>>7)&0x01 ;
//					lcd_putsAttIdx(  6*FW, y, XPSTR(M_LH_STR), value, ((sub==subN && subSub == 0) ? blink:0) );
//					lcd_putsAttIdx(  17*FW, y, XPSTR("\002 7 8 91011"), pModule->ppmFrameLength, ((sub==subN && subSub == 1) ? blink:0) );
//					if(sub==subN)
//					{
//						Columns = 1;
//						if(subSub==0 && s_editing)
//						{
//							uint8_t oldValue = value ;
//							CHECK_INCDEC_H_MODELVAR_0(value, 1 );
//							pModule->channels = (value<<7) + (pModule->channels&0x7F);
//							if ( oldValue != value )
//							{
//								checkMultiPower() ;
//								killEvents(event) ;
//							}
//						}
//						if(subSub==1 && s_editing)
//							CHECK_INCDEC_H_MODELVAR_0( pModule->ppmFrameLength, 4);
//					}
//					if((y+=FH)>7*FH) return ;
//				}
//				subN += 1 ;
//			}

//			if ( pModule->protocol == PROTO_DSM2 )
//			{
//				if(t_pgOfs<=subN)
//  		  {
//			    lcd_puts_Pleft( y, PSTR(STR_DSM_TYPE));
//  			  int8_t x ;
//  		  	x = pModule->sub_protocol ;
//			    lcd_putsAttIdx( 10*FW, y, XPSTR(DSM2_STR), x, (sub==subN ? blink:0) ) ;
//  			  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( pModule->sub_protocol,3);
//  		  	if((y+=FH)>7*FH) return;
//  		  }
//				subN += 1 ;
//			}

////			if(t_pgOfs<=subN)
////			{
////			  lcd_puts_Pleft( y, PSTR(STR_PPM2_START));
//////				if ( pModule->startchannel == 0 )
//////				{
//////					lcd_putsAtt( 15*FW, y, PSTR(STR_FOLLOW), (sub==subN) ? INVERS : 0 ) ;
//////				}
//////				else
//////				{
////				lcd_outdezAtt(  17*FW, y, pModule->startChannel, (sub==subN) ? INVERS : 0 ) ;
//////				}
////			  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( pModule->startChannel, 16 ) ;
////			  if((y+=FH)>7*FH) return ;
////			} subN += 1 ;

//			if ( pModule->protocol == PROTO_PPM )
//			{
//				if(t_pgOfs<=subN)
//  		  {
// 	  	    lcd_puts_Pleft(    y, PSTR(STR_PPMFRAME_MSEC));
//					uint8_t attr = PREC1 ;
// 	  	    if(sub==subN) { attr = blink | PREC1 ; CHECK_INCDEC_H_MODELVAR( pModule->ppmFrameLength,-20,20) ; }
// 	  	    lcd_outdezAtt(  14*FW-1, y, (int16_t)pModule->ppmFrameLength*5 + 225, attr ) ;
//  		  	if((y+=FH)>7*FH) return;
//  		  }
//				subN += 1 ;

//#ifdef REVX
//				if ( module == 1 )
//				{
//					if(t_pgOfs<=subN)
//  	  		{
//  	  		  lcd_puts_Pleft(    y, XPSTR(" PPM Drive"));
//  				  uint8_t x ;
//  	  			x = pModule->ppmOpenDrain ;
//						pModule->ppmOpenDrain = checkIndexed( y, XPSTR(FWx12"\001""\011OpenDrainFullDrive"), pModule->ppmOpenDrain, (sub==subN) ) ;
//						if ( x != pModule->ppmOpenDrain )
//						{
//							module_output_active() ;		// Changes drive as well
//						}
//						if((y+=FH)>7*FH) return;
//  	  		}
//					subN += 1 ;
//				}
//#endif

//				if(t_pgOfs<=subN)
//				{
//				  	uint8_t attr = 0 ;
//				    lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
//				    if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( pModule->pulsePol,1);}
//				    lcd_putsAttIdx( 11*FW, y, PSTR(STR_POS_NEG),pModule->pulsePol,attr );
//				    if((y+=FH)>7*FH) return;
//				}subN++;
//			}

//			if (pModule->protocol == PROTO_PXX)
//			{
//				if(t_pgOfs<=subN)
//				{
//					lcd_puts_Pleft( y, PSTR(STR_TYPE) ) ;
//					pModule->sub_protocol = checkIndexed( y, XPSTR(FWx10"\002""\006D16(X)D8(D) LRP   "), pModule->sub_protocol, (sub==subN) ) ;
//			    if((y+=FH)>7*FH) return ;
//				}subN++;
//				if(t_pgOfs<=subN)
//				{
//			    lcd_puts_Pleft( y, XPSTR("Chans") );
//					pModule->channels = checkIndexed( y, XPSTR(FWx10"\001""\00216 8"), pModule->channels, (sub==subN) ) ;
//			    if((y+=FH)>7*FH) return ;
//				}subN++;
//				if(t_pgOfs<=subN)
//				{
//    			lcd_puts_Pleft( y, XPSTR("Failsafe") ) ;
//					pModule->failsafeMode = checkIndexed( y, XPSTR(FWx9"\004""\007Not Set     Rx Custom   HoldNoPulse"), pModule->failsafeMode, sub==subN ) ;
//					if ( sub == subN )
//					{
//						if ( checkForMenuEncoderLong( event ) )
//						{
//							s_currIdx = module ;
//    			   	pushMenu( menuSetFailsafe ) ;
//						}
//					}
//			    if((y+=FH)>7*FH) return ;
//				}subN++;
			
//				if(t_pgOfs<=subN)
//				{
//					lcd_puts_Pleft( y, PSTR(STR_COUNTRY) ) ;
//					pModule->country = checkIndexed( y, XPSTR(FWx10"\002""\003AmeJapEur"), pModule->country, (sub==subN) ) ;
//			    if((y+=FH)>7*FH) return ;
//				}subN++;
//			}

//			if(t_pgOfs<=subN)
//			{
//				uint8_t attr = 0 ;
// 				lcd_puts_Pleft( y, PSTR(STR_PPM_1ST_CHAN));
// 				if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( pModule->startChannel, 16 ) ; }
//				lcd_outdezAtt(  14*FW, y, pModule->startChannel + 1, attr ) ;
//			  if((y+=FH)>7*FH) return ;
//			} subN += 1 ;

//			uint8_t brNeeded = need_bind_range ;
//			if ( module )
//			{
//				brNeeded >>= 1 ;
//			}

//			if ( brNeeded & 1 )
//			{
//				if(t_pgOfs<=subN)
//				{
//					lcd_puts_Pleft( y, PSTR(STR_BIND) ) ;
//					if ( (pModule->protocol == PROTO_MULTI ) && ( PrivateData[1] ) )
//					{
//						div_t qr ;
//						lcd_puts_Pleft( y, XPSTR("\006(Ver  . .  .  )" ) ) ;
//						lcd_putc( 11*FW, y, PrivateData[1] + '0' ) ;
//						lcd_putc( 13*FW, y, PrivateData[2] + '0' ) ;
//						qr = div( PrivateData[3], 10 ) ;
//						lcd_putc( 16*FW, y, qr.rem + '0' ) ;
//						if ( qr.quot )
//						{
//							lcd_putc( 15*FW, y, qr.quot + '0' ) ;
//						}
//						qr = div( PrivateData[4], 10 ) ;
//						lcd_putc( 19*FW, y, qr.rem + '0' ) ;
//						if ( qr.quot )
//						{
//							lcd_putc( 18*FW, y, qr.quot + '0' ) ;
//						}
//						if ( PrivateData[0] & 0x08 )
//						{
//							lcd_char_inverse( 0*FW, y, 4*FW, 1 ) ;
//						}
//					}

//  		    if(sub==subN)
//					{
//						lcd_char_inverse( 0, y, 4*FW, 0 ) ;
//						if ( checkForMenuEncoderLong( event ) )
//						{
//  		  		  BindRangeFlag[module] = PXX_BIND ;		    	//send bind code or range check code
//							s_currIdx = module ;
//							pushMenu(menuRangeBind) ;
//						}
//					}
//					if((y+=FH)>7*FH) return ;
//				} subN += 1 ;
//			}


	for ( i = 1 ; i < 4 ; i <<= 1 )
	{
		if ( need_range & i )
		{
			lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
			displayModuleName( 12*FW, y, i > 1, 0 ) ;
//			lcd_puts_Pleft( y, "\014Internal") ;
//			if ( i & 2 )
//			{
//				lcd_puts_Pleft( y, "\014Ex") ;
//			}
  		if(sub==subN)
			{
				lcd_char_inverse( 0, y, 11*FW, 0 ) ;
				if ( checkForMenuEncoderLong( event ) )
				{
					uint8_t module = 0 ;
					if ( i & 2 )
					{
						module = 1 ;
					}
  		  	BindRangeFlag[module] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
					s_currIdx = module ;
					pushMenu(menuRangeBind) ;
				}
			}
			y+=FH ;
			subN += 1 ;
		}
	}

//			if ( need_range & 1 )
//			{
////				if(t_pgOfs<=subN)
////				{
//				lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
//				lcd_puts_Pleft( y, "\014Internal") ;
//  		  if(sub==subN)
//				{
//					lcd_char_inverse( 0, y, 11*FW, 0 ) ;
//					if ( checkForMenuEncoderLong( event ) )
//					{
//  		  	  BindRangeFlag[0] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
//						s_currIdx = 0 ;
//						pushMenu(menuRangeBind) ;
//					}
//				}
//				y+=FH ;
//				subN += 1 ;
//			}
//			if ( need_range & 2 )
//			{
////				if(t_pgOfs<=subN)
////				{
//				lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
//				lcd_puts_Pleft( y, "\014External") ;
//  		  if(sub==subN)
//				{
//					lcd_char_inverse( 0, y, 11*FW, 0 ) ;
//					if ( checkForMenuEncoderLong( event ) )
//					{
//  		  	  BindRangeFlag[1] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
//						s_currIdx = 1 ;
//						pushMenu(menuRangeBind) ;
//					}
//				}
////				y+=FH ;
////				subN += 1 ;
//			}









//		}
//	} // module for loop
	
	
	
	
	 
}
//#endif



void menuProcTrainProtocol(uint8_t event)
{
	TITLE(PSTR(STR_Trainer));
	static MState2 mstate2;
	event = mstate2.check_columns( event, 5 ) ;

	int8_t  sub    = mstate2.m_posVert ;
	uint8_t subN = 0 ;
	uint8_t y = 1*FH;
	uint8_t blink = InverseBlink ;
	 
	uint8_t attr = 0 ;
	lcd_puts_Pleft( y, XPSTR("Trainer Polarity"));
	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.trainPulsePol,1);}
	lcd_putsAttIdx( 17*FW, y, PSTR(STR_POS_NEG),g_model.trainPulsePol,attr );
  
	y += FH ;
 	subN += 1 ;
		
	attr = 0 ;
	lcd_puts_Pleft( y, XPSTR("Channels"));
	if (sub==subN)
	{
		attr |= blink ;
 		CHECK_INCDEC_H_MODELVAR_0( g_model.ppmNCH, 12 ) ;
	}
	lcd_outdezAtt(  20*FW, y, g_model.ppmNCH + 4, attr ) ;

	y += FH ;
 	subN += 1 ;
		
	attr = PREC1 ;
	lcd_puts_Pleft(    y, PSTR(STR_PPMFRAME_MSEC));
	if (sub==subN)
	{
		attr |= blink ;
 		CHECK_INCDEC_H_MODELVAR( g_model.ppmFrameLength, -12, 12 ) ;
	}
 	lcd_outdezAtt(  14*FW-1, y, g_model.ppmFrameLength*5 + 225, attr ) ;

	y += FH ;
 	subN += 1 ;
		
	attr = 0 ;
  lcd_puts_Pleft( y, XPSTR("Delay\023uS") );
	if (sub==subN)
	{
		attr |= blink ;
		CHECK_INCDEC_H_MODELVAR( g_model.ppmDelay,-4,10);
	}
	lcd_outdezAtt(  11*FW+7*FW-1, y, (g_model.ppmDelay*50)+300, attr);

	y += FH ;
 	subN += 1 ;
		
	attr = 0 ;
	
	lcd_puts_Pleft( y, PSTR(STR_PPM_1ST_CHAN));
	if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.startChannel, 16 ) ; }
	lcd_outdezAtt(  14*FW, y, g_model.startChannel + 1, attr ) ;

	y += FH ;
 	subN += 1 ;
		
	uint8_t value = g_model.trainerProfile + 1 ;
	if ( g_model.traineron == 0 )
	{
		value = 0 ;
	}
  lcd_puts_Pleft( y, PSTR(STR_Trainer) ) ;
  if(value)
	{
    lcd_outdezAtt( PARAM_OFS+3*FW, y, value-1, (sub==subN ? blink : 0) ) ;
    lcd_putc( 8*FW, y, '(' ) ;
    lcd_putc( 15*FW, y, ')' ) ;
		validateText( g_eeGeneral.trainerProfile[value-1].profileName, 6 ) ;
		lcd_putsnAtt( 9*FW, y, (const char *)g_eeGeneral.trainerProfile[value-1].profileName, 6, 0 ) ;
  }
  else
	{
    lcd_putsAtt(PARAM_OFS, y, PSTR(STR_OFF), (sub==subN ? blink:0) ) ;
	}
  if(sub==subN)
	{
		CHECK_INCDEC_H_MODELVAR_0( value, 4 ) ;
		MaskRotaryLong = 1 ;
		if ( ( value > 0) && ( checkForMenuEncoderLong( event ) ) )
		{
			SingleExpoChan = 1 ;
			s_expoChan = value-1 ;
			s_editMode = 0 ;
      killEvents(event);
      pushMenu(menuProcTrainer) ; 
		}
	}

	g_model.traineron = value > 0 ;
	if ( value )
	{
		g_model.trainerProfile = value - 1 ;
	}
	 
}

//void menuProcProtocol(uint8_t event)
//{
//	uint8_t need_bind_range = 0 ;
//	EditType = EE_MODEL ;

//	uint8_t dataItems = 13 ;
////	uint8_t dataItems = 12 ;

//#ifdef REVX
//	if (g_model.Module[1].protocol == PROTO_PPM)
//	{
//		dataItems += 1 ;
//	}
//#endif

//	if (g_model.protocol == PROTO_PXX)
//	{
//		dataItems += 4 ;
//		need_bind_range |= 1 ;
//	}
//	if (g_model.protocol == PROTO_MULTI)
//	{
//		dataItems += 4 ;
//		need_bind_range = 1 ;
//		if ( ( g_model.ppmFrameLength < 0 ) || ( g_model.ppmFrameLength > 4 ) )
//		{
//			g_model.ppmFrameLength = 0 ;
//		}
//	}
//	if (g_model.protocol == PROTO_DSM2)
//	{
//#ifdef ENABLE_DSM_MATCH  		
//		if (g_model.sub_protocol >= 3)
//		{
//			dataItems += 1 ;
//		}
//#endif
//		{
//			need_bind_range = 4 ;
//		}
//	}
//#if defined(PCBX9D) || defined(PCB9XT)
//	if (g_model.xprotocol == PROTO_PXX)
//	{
//		dataItems += 4 ;
//		need_bind_range |= 2 ;
//	}
//	if (g_model.xprotocol == PROTO_MULTI)
//	{
//		dataItems += 4 ;
//		need_bind_range |= 2 ;
//		if ( ( g_model.xppmFrameLength < 0 ) || ( g_model.xppmFrameLength > 4 ) )
//		{
//			g_model.xppmFrameLength = 0 ;
//		}
//	}
//	if (g_model.xprotocol == PROTO_DSM2)
//	{
//#ifdef ENABLE_DSM_MATCH  		
//		if (g_model.xsub_protocol >= 3)
//		{
//			dataItems += 1 ;
//		}
//#endif
//		{
//			need_bind_range |= 8 ;
//		}
//	}
//	if ( g_model.protocol == PROTO_OFF )
//	{
//		dataItems -= 4 ;
//	}
//	if ( g_model.xprotocol == PROTO_OFF )
//	{
//		dataItems -= 4 ;
//	}
//#endif

//#ifdef PCBSKY
//	if ( g_model.protocol == PROTO_OFF )
//	{
//		dataItems -= 4 ;
//	}
//	if ( g_model.xprotocol == PROTO_OFF )
//	{
//		dataItems -= 4 ;
//	}
//	if ( g_model.xprotocol == PROTO_MULTI )
//	{
//		dataItems += 4 ;
//	}

//#endif

//uint8_t blink = InverseBlink ;

//	TITLE( XPSTR("Protocol") ) ;
//	static MState2 mstate2 ;

//	event = mstate2.check_columns( event, dataItems-1-1 ) ;

//	int8_t  sub    = mstate2.m_posVert ;
//	uint8_t subSub = g_posHorz;
//  uint8_t t_pgOfs ;

//	t_pgOfs = evalOffset(sub);

//	uint8_t y = 1*FH;

//	uint8_t subN = 0 ;


//	if(t_pgOfs<=subN)
//	{
//		lcd_puts_Pleft( y, XPSTR("New") ) ;
//		if(sub==subN)
//		{
//			lcd_char_inverse( 0, y, 3*FW, 0 ) ;
//			if (event == EVT_KEY_LONG(KEY_MENU) )
//			{
// 		   	pushMenu( menuYProcProtocol ) ;
//		  	s_editMode = 0 ;
// 		    killEvents(event);
//			}
//		}	
//	  if((y+=FH)>7*FH) return ;
// 	} subN += 1 ;

//	if(t_pgOfs<=subN)
//	{
//		uint8_t attr = (sub==subN) ? INVERS : 0 ;
//		lcd_puts_Pleft( y, XPSTR("Trainer Polarity"));
//		if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.trainPulsePol,1);}
//		lcd_putsAttIdx( 17*FW, y, PSTR(STR_POS_NEG),g_model.trainPulsePol,attr );
//	  if((y+=FH)>7*FH) return ;
// 	} subN += 1 ;

//#ifndef PCBX9D
// #ifndef PCB9XT
//	#ifdef PCBSKY
//	if(t_pgOfs<=subN)
//	{
//		uint8_t value ;
//		uint8_t newvalue ;
//		value = (g_model.xprotocol == PROTO_OFF) ? 0 : 1 ;
//		newvalue = onoffMenuItem( value, y, XPSTR("Internal Module"), sub==subN ) ;
//		if ( newvalue != value )
//		{
//			value = newvalue ? 0 : PROTO_OFF ;
//		}
//		else
//		{
//			value = g_model.xprotocol ;
//		}
//		if ( g_model.xprotocol != value )
//		{
//			 g_model.xprotocol = value ;
//		}
//    if((y+=FH)>7*FH) return;
//	} subN++;

//	if ( g_model.xprotocol != PROTO_OFF )
//	{
//		if(t_pgOfs<=subN)
//		{
//  	  lcd_puts_Pleft( y, PSTR(STR_PROTO));//sub==2 ? INVERS:0);
//			uint32_t b = g_model.xprotocol ;
//			if ( b == PROTO_DSM2 )
//			{
//				b = 1 ;
//			}
//			else if ( b == PROTO_MULTI )
//			{
//				b = 2 ;
//			}
//			uint8_t attr = 0 ;
//			if ( (sub==subN) && ( subSub==0 ) )
//			{
//				attr |= blink ;
//			}
//			lcd_putsAttIdx(  6*FW, y, XPSTR("\005PPM  DSM  Multi"), b, attr );
//			if ( attr ) CHECK_INCDEC_H_MODELVAR_0( b, 2 ) ;
//			if ( b == 1 )
//			{
//				b = PROTO_DSM2 ;
//			}
//			else if ( b == 2 )
//			{
//				b = PROTO_MULTI ;
//			}
//			if ( b != g_model.xprotocol )
//			{
//				g_model.xsub_protocol = 0 ;
//			}
//			g_model.xprotocol = b ;

//			if ( b == PROTO_PPM )
//			{
//				uint8_t x ;
//				attr = 0 ;
//				if ( sub==subN )
//				{
//			 		Columns = 2 ;
//				}
//			  lcd_puts_Pleft( y, PSTR(STR_21_USEC) );
//				x = 11*FW ;
//				if ( ( g_model.ppm2NCH > 12 ) || (g_model.ppm2NCH < -2) )
//				{
//					g_model.ppm2NCH = 0 ;		// Correct if wrong from DSM
//				}
//				uint8_t chans = g_model.ppm2NCH + 2 ;
//				chans *= 2 ;
//				if ( chans > 12 )
//				{
//					chans -= 13 ;
//				}
//				if ( (sub==subN) && ( subSub==1 ) )
//				{
//					attr |= blink ;
//				}
//				lcd_outdezAtt(  x, y, chans+4, attr ) ;
//				if ( attr )
//				{
// 	      	CHECK_INCDEC_H_MODELVAR_0( chans, 12 ) ;
//					if ( chans & 1 )	// odd
//					{
//						chans /= 2 ;
//						chans += 7 ;
//					}
//					else
//					{
//						chans /= 2 ;
//					}
//					g_model.ppm2NCH = chans - 2 ;
//				}
//  			lcd_putsAtt( Lcd_lastPos, y, PSTR( STR_PPMCHANNELS ), attr ) ;
//				attr = 0 ;
//				if ( (sub==subN) && ( subSub==2 ) )
//				{
//					attr |= blink ;
//				}
//				lcd_outdezAtt(  x+7*FW-1, y,  (g_model.xppmDelay*50)+300, attr);
//				if ( attr )
//				{
//					CHECK_INCDEC_H_MODELVAR( g_model.xppmDelay,-4,10);
//				}
//			}
//			else
//			{
//				attr = 0 ;
//				if ( sub==subN )
//				{
//			 		Columns = 1 ;
//				}
//		    lcd_puts_Pleft( y, PSTR(STR_13_RXNUM) );
//				if ( (sub==subN) && ( subSub==1 ) )
//				{
//					attr |= blink ;
//				}
// 	      lcd_outdezAtt(  21*FW, y,  g_model.xPxxRxNum, attr ) ;

//				if ( attr )
//				{
//  	       CHECK_INCDEC_H_MODELVAR_0( g_model.xPxxRxNum,124);
//				}
//			}
//		  if((y+=FH)>7*FH) return ;
//		} subN += 1 ;

//		if ( g_model.xprotocol == PROTO_MULTI )
//		{
//			uint8_t attr = 0 ;
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_MULTI_PROTO));
//				uint8_t oldValue = g_model.xsub_protocol ;
//				uint8_t svalue = oldValue & 0x3F ;
				
//				svalue = editMultiProtocol( svalue, ( sub == subN ) ) ;

//				if ( sub == subN )
//				{
//					attr = blink ;
//				}

//				displayMultiProtocol( svalue, y, attr ) ;

//				g_model.xsub_protocol = svalue + (g_model.xsub_protocol & 0xC0) ;
//				if(g_model.xsub_protocol==oldValue)
//					attr=(g_model.xppmNCH >> 4) &0x07 ;
//				else
//				{
//					attr=0;
//					TelemetryType = TEL_UNKNOWN ;
//				}
//				if((y+=FH)>7*FH) return ;
//			}
//			subN++;
		
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_MULTI_TYPE));
//				uint8_t x = g_model.xsub_protocol&0x3F ;
  			
//				if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( attr, 7 ) ;
//				attr = displayMultiSubProtocol( x, attr, y, (sub==subN) ) ;
//				g_model.xppmNCH = ( attr << 4) + (g_model.xppmNCH & 0x8F);

//				if((y+=FH)>7*FH) return ;
//			}
//			subN++;
		
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_MULTI_AUTO));
//				uint8_t value = (g_model.xsub_protocol>>6)&0x01 ;
//				lcd_putsAttIdx(  9*FW, y, XPSTR(M_NY_STR), value, (sub==subN && subSub==0 ? blink:0) );
//				multiOption( 21*FW, y, g_model.xoption_protocol, (sub==subN && subSub==1 ? blink:0), g_model.xsub_protocol & 0x3F ) ;
//				if(sub==subN)
//				{
//					Columns = 1;
//					if(subSub==0 && s_editing)
//					{
//						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
//						g_model.xsub_protocol = (value<<6) + (g_model.xsub_protocol&0xBF);
//					}
//					if(subSub==1 && s_editing)
//						CHECK_INCDEC_H_MODELVAR(g_model.xoption_protocol, -128, 127);
//				}
//				if((y+=FH)>7*FH) return ;
//			}
//			subN++;
		
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_MULTI_POWER));
//				// Power stored in ppmNCH bit7 & Option stored in option_protocol
//				uint8_t value = (g_model.xppmNCH>>7)&0x01 ;
//				lcd_putsAttIdx(  6*FW, y, XPSTR(M_LH_STR), value, ((sub==subN && subSub == 0) ? blink:0) );
//				lcd_putsAttIdx(  17*FW, y, XPSTR("\002 7 8 91011"), g_model.xppmFrameLength, ((sub==subN && subSub == 1) ? blink:0) );
//				if(sub==subN)
//				{
//					Columns = 1;
//					if(subSub==0 && s_editing)
//					{
//						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
//						g_model.xppmNCH = (value<<7) + (g_model.xppmNCH&0x7F);
//					}
//					if(subSub==1 && s_editing)
//						CHECK_INCDEC_H_MODELVAR_0( g_model.xppmFrameLength, 4);
//				}
//				if((y+=FH)>7*FH) return ;
//			}
//			subN++;
//		}


//		if ( g_model.xprotocol == PROTO_DSM2 )
//		{
//			if(t_pgOfs<=subN)
//  	  {
//		    lcd_puts_Pleft( y, PSTR(STR_DSM_TYPE));
//  		  int8_t x ;
//  	  	x = g_model.xsub_protocol ;
//		    lcd_putsAttIdx( 10*FW, y, XPSTR(DSM2_STR), x, (sub==subN ? blink:0) ) ;
//  		  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.xsub_protocol,2);
//  	  	if((y+=FH)>7*FH) return;
//  	  }
//			subN += 1 ;
//		}

//		if(t_pgOfs<=subN)
//		{
//		  lcd_puts_Pleft( y, PSTR(STR_PPM2_START));
//			if ( g_model.startPPM2channel == 0 )
//			{
//				lcd_putsAtt( 15*FW, y, PSTR(STR_FOLLOW), (sub==subN) ? INVERS : 0 ) ;
//			}
//			else
//			{
//				lcd_outdezAtt(  17*FW, y, g_model.startPPM2channel, (sub==subN) ? INVERS : 0 ) ;
//			}
//		  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.startPPM2channel, 17 ) ;
//		  if((y+=FH)>7*FH) return ;
//		} subN += 1 ;


//		if ( g_model.xprotocol == PROTO_PPM )
//		{
//			if(t_pgOfs<=subN)
//  	  {
// 	      lcd_puts_Pleft(    y, PSTR(STR_PPMFRAME_MSEC));
//				uint8_t attr = PREC1 ;
// 	      if(sub==subN) { attr = INVERS | PREC1 ; CHECK_INCDEC_H_MODELVAR( g_model.xppmFrameLength,-20,20) ; }
// 	      lcd_outdezAtt(  14*FW-1, y, (int16_t)g_model.xppmFrameLength*5 + 225, attr ) ;
//  	  	if((y+=FH)>7*FH) return;
//  	  }
//			subN += 1 ;
			
//			if(t_pgOfs<=subN)
//			{
//			  	uint8_t attr = 0 ;
//			    lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
//			    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.xpulsePol,1);}
//			    lcd_putsAttIdx( 11*FW, y, PSTR(STR_POS_NEG),g_model.xpulsePol,attr );
//			    if((y+=FH)>7*FH) return;
//			}subN++;
//		}

//		if ( g_model.xprotocol == PROTO_MULTI )
//		{
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_BIND) ) ;
//      	if(sub==subN)
//				{
//					lcd_char_inverse( FW, y, 4*FW, 0 ) ;
//					if ( event==EVT_KEY_LONG(KEY_MENU))
//					{
//   		    	BindRangeFlag[1] = PXX_BIND ;		    	//send bind code or range check code
//						s_currIdx = 1 ;
//						pushMenu(menuRangeBind) ;
//					}
//					s_editMode = 0 ;
//				}
//				if((y+=FH)>7*FH) return ;
//		 	} subN += 1 ;
//		}
		 
//		if ( ( g_model.xprotocol == PROTO_DSM2 ) || ( g_model.xprotocol == PROTO_MULTI ) )
//		{
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
//      	if(sub==subN)
//				{
//					lcd_char_inverse( FW, y, 11*FW, 0 ) ;
//					if ( event==EVT_KEY_LONG(KEY_MENU))
//					{
//    	    	BindRangeFlag[1] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
//						s_currIdx = 1 ;
//						pushMenu(menuRangeBind) ;
//					}
//					s_editMode = 0 ;
//				}
//			  if((y+=FH)>7*FH) return ;
//		 	} subN += 1 ;
//		}

//	}
//	#endif // PCBSKY
// #endif // nPCB9XT
//#endif // nPCBX9D

//	uint8_t protocol = g_model.protocol ;
	
//#if defined(PCBX9D) || defined(PCB9XT)
//	if(t_pgOfs<=subN)
//	{
//		uint8_t value ;
//		uint8_t newvalue ;
//		value = (protocol == PROTO_OFF) ? 0 : 1 ;
//		newvalue = onoffMenuItem( value, y, XPSTR("Internal Module"), sub==subN ) ;
//		if ( newvalue != value )
//		{
//			value = newvalue ? 0 : PROTO_OFF ;
//		}
//		else
//		{
//			value = protocol ;
//		}
//		if ( protocol != value )
//		{
//			 g_model.protocol = protocol = value ;
//		}
//    if((y+=FH)>7*FH) return;
//	} subN++;

//	if ( protocol != PROTO_OFF )
//	{
//#endif

//#ifdef PCBSKY
//	if(t_pgOfs<=subN)
//	{
//		uint8_t value ;
//		uint8_t newvalue ;
//		value = (protocol == PROTO_OFF) ? 0 : 1 ;
//		newvalue = onoffMenuItem( value, y, XPSTR("External Module"), sub==subN ) ;
//		if ( newvalue != value )
//		{
//			value = newvalue ? 0 : PROTO_OFF ;
//		}
//		else
//		{
//			value = protocol ;
//		}
//		if ( protocol != value )
//		{
//			 g_model.protocol = protocol = value ;
//		}
//    if((y+=FH)>7*FH) return;
//	} subN++;

//	if ( protocol != PROTO_OFF )
//	{
//#endif
//		if(t_pgOfs<=subN)
//		{
//  	  lcd_puts_Pleft(    y, PSTR(STR_PROTO));//sub==2 ? INVERS:0);
//			lcd_putsAttIdx(  6*FW, y, PSTR(STR_PROT_OPT), protocol, (sub==subN && subSub==0 ? blink:0) );
//  	  if ( protocol == PROTO_PPM )
//			{
//				uint8_t x ;
//			  lcd_puts_Pleft( y, PSTR(STR_21_USEC) );
//				x = 10*FW ;
//				if ( ( g_model.ppmNCH > 12 ) || (g_model.ppmNCH < -2) )
//				{
//					g_model.ppmNCH = 0 ;		// Correct if wrong from DSM
//				}
//				uint8_t channels = (g_model.ppmNCH + 4) * 2 ;
//				if ( channels > 16 )
//				{
//					channels -= 13 ;
//				}
//				uint8_t attr = LEFT ;
//				if ( (sub==subN) && ( subSub==1 ) )
//				{
//					attr |= blink ;
//				}
//				lcd_outdezAtt(  x, y, channels, attr ) ;
//  			lcd_putsAtt( Lcd_lastPos, y, PSTR( STR_PPMCHANNELS ), attr ) ;
//				lcd_outdezAtt(  x+7*FW-1, y,  (g_model.ppmDelay*50)+300, (sub==subN && subSub==2 ? blink:0));
//  	  }
//  	  else if ( ( protocol == PROTO_PXX) || ( ( protocol == PROTO_DSM2) && (g_model.sub_protocol != DSM_9XR) ) || (protocol == PROTO_MULTI) )
//  	  {
//			    lcd_puts_Pleft( y, PSTR(STR_13_RXNUM) );
//  	      lcd_outdezAtt(  21*FW, y,  g_model.pxxRxNum, (sub==subN && subSub==1 ? blink:0));
//  	  }
//  	  else if ( ( ( protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) ) )
//  	  {
//		    lcd_puts_Pleft( y, XPSTR("\013Chans") );
// 	      lcd_outdezAtt(  21*FW, y,  g_model.ppmNCH, (sub==subN && subSub==1 ? blink:0));
//			}

//		  if(sub==subN)
//			{
//			 Columns = (protocol == PROTO_PPM) ? 2 : 1 ;
//			 if (protocol == PROTO_XFIRE)
//			 {
//			 		Columns = 0 ;
//			 }
//			 if (s_editing )
//			 {
			
//#if defined(PCBX9D) || defined(PCB9XT)
// #ifdef PCB9XT
//			  uint8_t prot_max = PROT_MAX - 0 ; // 3 ;
// #else
//			  uint8_t prot_max = PROT_MAX - 2 ;		// No DSM on internal
//#endif
//#else
// #ifdef XFIRE
//			  uint8_t prot_max = PROT_MAX + 1 ;
// #else
//			  uint8_t prot_max = PROT_MAX ;
// #endif
//#endif
//  	    switch (subSub)
//				{
//  	     	case 0:
//  	        CHECK_INCDEC_H_MODELVAR_0( g_model.protocol, prot_max ) ;
//						if ( protocol != g_model.protocol )
//						{
//							g_model.ppmNCH = 0 ;
//							g_model.sub_protocol = 0 ;
//						}
//  	      break;
//  	      case 1:
//  	        if (protocol == PROTO_PPM)
//						{
//							uint8_t chans = g_model.ppmNCH + 2 ;
//							chans *= 2 ;
//							if ( chans > 12 )
//							{
//								chans -= 13 ;
//							}
// 	          	CHECK_INCDEC_H_MODELVAR_0( chans, 12 ) ;
//							if ( chans & 1 )	// odd
//							{
//								chans /= 2 ;
//								chans += 7 ;
//							}
//							else
//							{
//								chans /= 2 ;
//							}
//							g_model.ppmNCH = chans - 2 ;
//						}
//  	  			else if ( protocol == PROTO_PXX)
//						{
//  	        	CHECK_INCDEC_H_MODELVAR_0( g_model.pxxRxNum,124);
//						}
//  	  			else if ( protocol == PROTO_DSM2)
//						{
//							if ( g_model.sub_protocol == DSM_9XR )
//							{
//  	          	CHECK_INCDEC_H_MODELVAR( g_model.ppmNCH,6,14) ;
//							}
//							else
//							{
//  	        		CHECK_INCDEC_H_MODELVAR_0( g_model.pxxRxNum,124);
//							}
//						}
//						else if (protocol == PROTO_MULTI)
//						{
//  	        	CHECK_INCDEC_H_MODELVAR_0( g_model.pxxRxNum,15);
//						}
//  	      break;
//  	      case 2:
//  	        if (g_model.protocol == PROTO_PPM)
//  	          CHECK_INCDEC_H_MODELVAR( g_model.ppmDelay,-4,10);
//  	       break;
//  	    }
//			 }
//  	  }
//  	  if((y+=FH)>7*FH) return;
//	}subN++;

//		if(g_model.protocol == PROTO_PPM)
//		{
//			if(t_pgOfs<=subN)
//  	  {
//  	      lcd_puts_Pleft(    y, PSTR(STR_PPMFRAME_MSEC));
//					uint8_t attr = PREC1 ;
//  	      if(sub==subN) { attr = INVERS | PREC1 ; CHECK_INCDEC_H_MODELVAR( g_model.ppmFrameLength,-20,20) ; }
//  	      lcd_outdezAtt(  14*FW-1, y, (int16_t)g_model.ppmFrameLength*5 + 225, attr ) ;
//  	  	if((y+=FH)>7*FH) return;
//  	  }
//			subN += 1 ;

//#ifdef REVX
//			if(t_pgOfs<=subN)
//  	  {
//  	    lcd_puts_Pleft(    y, XPSTR(" PPM Drive"));
//  		  uint8_t x ;
//  	  	x = g_model.ppmOpenDrain ;
//				g_model.ppmOpenDrain = checkIndexed( y, XPSTR(FWx12"\001""\011OpenDrainFullDrive"), g_model.ppmOpenDrain, (sub==subN) ) ;
//				if ( x != g_model.ppmOpenDrain )
//				{
//					module_output_active() ;		// Changes drive as well
//				}
//				if((y+=FH)>7*FH) return;
//  	  }
//			subN += 1 ;
//#endif
//		}

//		if (protocol == PROTO_PXX)
//		{
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_TYPE) ) ;
//				g_model.sub_protocol = checkIndexed( y, XPSTR(FWx10"\002""\006D16(X)D8(D) LRP   "), g_model.sub_protocol, (sub==subN) ) ;
//		    if((y+=FH)>7*FH) return ;
//			}subN++;
//			if(t_pgOfs<=subN)
//			{
//		    lcd_puts_Pleft( y, XPSTR("Chans") );
//				g_model.ppmNCH = checkIndexed( y, XPSTR(FWx10"\001""\00216 8"), g_model.ppmNCH, (sub==subN) ) ;
//		    if((y+=FH)>7*FH) return ;
//			}subN++;
//			if(t_pgOfs<=subN)
//			{
//    		lcd_puts_Pleft( y, XPSTR("Failsafe") ) ;
//				g_model.failsafeMode[0] = checkIndexed( y, XPSTR(FWx9"\004""\007Not Set     Rx Custom   HoldNoPulse"), g_model.failsafeMode[0], sub==subN ) ;
//				if ( sub == subN )
//				{
//					if (event == EVT_KEY_LONG(KEY_MENU) )
//					{
//						s_currIdx = 0 ;
//    		   	pushMenu( menuSetFailsafe ) ;
//  			  	s_editMode = 0 ;
//    		    killEvents(event);
//					}
//				}
//		    if((y+=FH)>7*FH) return ;
//			}subN++;
//		}
		
//		if (protocol == PROTO_MULTI)
//		{
//			uint8_t attr = 0 ;
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_MULTI_PROTO));
//				uint8_t oldValue = g_model.sub_protocol ;
//				uint8_t svalue = oldValue & 0x3F ;

//				svalue = editMultiProtocol( svalue, ( sub == subN ) ) ;

//				if ( sub == subN )
//				{
//					attr = blink ;
//				}
//				displayMultiProtocol( svalue, y, attr ) ;

//				g_model.sub_protocol = svalue + (g_model.sub_protocol & 0xC0) ;
//				if(g_model.sub_protocol==oldValue)
//					attr=(g_model.ppmNCH >> 4) &0x07 ;
//				else
//				{
//					attr=0;
//					TelemetryType = TEL_UNKNOWN ;
//				}
//				if((y+=FH)>7*FH) return ;
//			}
//			subN++;
			
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_MULTI_TYPE));
//				uint8_t x = g_model.sub_protocol&0x3F ;
				
//  			if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( attr, 7 ) ;
//				attr = displayMultiSubProtocol( x, attr, y, (sub==subN) ) ;
//				g_model.ppmNCH = ( attr << 4) + (g_model.ppmNCH & 0x8F);
				
//				if((y+=FH)>7*FH) return ;
//			}
//			subN++;

//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_MULTI_AUTO));
//				uint8_t value = (g_model.sub_protocol>>6)&0x01 ;
//				lcd_putsAttIdx(  9*FW, y, XPSTR(M_NY_STR), value, (sub==subN && subSub==0 ? blink:0) );
//				multiOption( 21*FW, y, g_model.option_protocol, (sub==subN && subSub==1 ? blink:0), g_model.sub_protocol & 0x3F ) ;
//				if(sub==subN)
//				{
//					Columns = 1;
//					if(subSub==0 && s_editing)
//					{
//						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
//						g_model.sub_protocol = (value<<6) + (g_model.sub_protocol&0xBF);
//					}
//					if(subSub==1 && s_editing)
//						CHECK_INCDEC_H_MODELVAR(g_model.option_protocol, -128, 127);
//				}
//				if((y+=FH)>7*FH) return ;
//			}
//			subN++;

//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_MULTI_POWER));
//				// Power stored in ppmNCH bit7 & Option stored in option_protocol
//				uint8_t value = (g_model.ppmNCH>>7)&0x01 ;
//				lcd_putsAttIdx(  6*FW, y, XPSTR(M_LH_STR), value, ((sub==subN && subSub == 0) ? blink:0) );
//				lcd_putsAttIdx(  17*FW, y, XPSTR("\002 7 8 91011"), g_model.ppmFrameLength, ((sub==subN && subSub == 1) ? blink:0) );
//				if(sub==subN)
//				{
//					Columns = 1;
//					if(subSub==0 && s_editing)
//					{
//						CHECK_INCDEC_H_MODELVAR_0(value, 1 );
//						g_model.ppmNCH = (value<<7) + (g_model.ppmNCH&0x7F);
//					}
//					if(subSub==1 && s_editing)
//						CHECK_INCDEC_H_MODELVAR_0( g_model.ppmFrameLength, 4);
//				}
//				if((y+=FH)>7*FH) return ;
//			}
//			subN++;

//		}
		
//		if (protocol == PROTO_DSM2)
//		{
//			if(t_pgOfs<=subN)
//			{
//		    lcd_puts_Pleft(    y, PSTR(STR_DSM_TYPE));
//  		  int8_t x ;
//  	  	x = g_model.sub_protocol ;
//		    lcd_putsAttIdx( 10*FW, y, XPSTR(DSM2_STR), x, (sub==subN ? blink:0) ) ;
//#ifdef PCBSKY
//  		  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.sub_protocol,3);
//#else        
//#ifdef PCB9XT
//				if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.sub_protocol,2);
//#else        
//				if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.sub_protocol,3);
//#endif
//				if ( g_model.sub_protocol != x )
//				{
//					g_model.ppmNCH = ( g_model.sub_protocol == 3 ) ? 12 : 6 ;
//				}
//#endif
//  	  	if((y+=FH)>7*FH) return ;
//			} subN += 1 ;
//		}

//#ifdef ENABLE_DSM_MATCH  		
//		if ( ( ( protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) ) )
//		{
//			lcd_puts_Pleft( y, PSTR(STR_13_RXNUM)+1 );
//      lcd_outdezAtt(  21*FW, y,  g_model.pxxRxNum, (sub==subN) ? blink:0);
  	  
//			CHECK_INCDEC_H_MODELVAR_0( g_model.pxxRxNum,124);
			
//  	  if((y+=FH)>7*FH) return ;
//			subN++ ;
//		}
//#endif // ENABLE_DSM_MATCH  		 
  	
//#ifdef PCBSKY
//		if (protocol == PROTO_PXX)
//#else
//		if (protocol == PROTO_PXX)
//#endif
//		{
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_COUNTRY) ) ;
//				g_model.country = checkIndexed( y, XPSTR(FWx10"\002""\003AmeJapEur"), g_model.country, (sub==subN) ) ;
//		    if((y+=FH)>7*FH) return ;
//			}subN++;
			
//		}
	
//#if defined(PCBX9D) || defined(PCB9XT)
//	}
//#endif
//#ifdef PCBSKY
//	}
//#endif

//#if defined(PCBX9D) || defined(PCB9XT)
//	if ( g_model.protocol != PROTO_OFF )
//	{
//#endif
//#ifdef PCBSKY
//	if ( g_model.protocol != PROTO_OFF )
//	{
//#endif
//		if(t_pgOfs<=subN)
//		{
//			{
//				uint8_t attr = 0 ;
//  			lcd_puts_Pleft( y, PSTR(STR_PPM_1ST_CHAN));
//  			if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.startChannel, 16 ) ; }
//				lcd_outdezAtt(  14*FW, y, g_model.startChannel + 1, attr ) ;
//			}
//		  if((y+=FH)>7*FH) return ;
//		} subN += 1 ;
//#if defined(PCBX9D) || defined(PCB9XT)
//	}
//#endif
//#ifdef PCBSKY
//	}
//#endif

//  if(g_model.protocol == PROTO_PPM)
//	{
//		if(t_pgOfs<=subN) {
//		  	uint8_t attr = 0 ;
//		    lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
//		    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.pulsePol,1);}
//		    lcd_putsAttIdx( 11*FW, y, PSTR(STR_POS_NEG),g_model.pulsePol,attr );
//		    if((y+=FH)>7*FH) return;
//		}subN++;
//	}
	
//	if ( need_bind_range & 5 )
//	{
//			if ( need_bind_range & 1 )
//			{
//				if(t_pgOfs<=subN)
//				{
//					lcd_puts_Pleft( y, PSTR(STR_BIND) ) ;
//      		if(sub==subN)
//					{
//						lcd_char_inverse( FW, y, 4*FW, 0 ) ;
//						if ( event==EVT_KEY_LONG(KEY_MENU))
//						{
//    		    	BindRangeFlag[0] = PXX_BIND ;		    	//send bind code or range check code
//							s_currIdx = 0 ;
//							pushMenu(menuRangeBind) ;
//						}
//						s_editMode = 0 ;
//					}
//				  if((y+=FH)>7*FH) return ;
//		 		} subN += 1 ;
//			}
			
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
//      	if(sub==subN)
//				{
//					lcd_char_inverse( FW, y, 11*FW, 0 ) ;
//					if ( event==EVT_KEY_LONG(KEY_MENU))
//					{
//    	    	BindRangeFlag[0] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
//						s_currIdx = 0 ;
//						pushMenu(menuRangeBind) ;
//					}
//					s_editMode = 0 ;
//				}
//			  if((y+=FH)>7*FH) return ;
//		 	} subN += 1 ;
//	}

//#if defined(PCBX9D) || defined(PCB9XT)

//	protocol = g_model.xprotocol ;
	
//	if(t_pgOfs<=subN)
//	{
//		uint8_t value ;
//		uint8_t newvalue ;
//		value = (protocol == PROTO_OFF) ? 0 : 1 ;
//		newvalue = onoffMenuItem( value, y, XPSTR("External Module"), sub==subN ) ;
//		if ( newvalue != value )
//		{
//			value = newvalue ? 0 : PROTO_OFF ;
//		}
//		else
//		{
//			value = protocol ;
//		}
//		if ( protocol != value )
//		{
//			 g_model.xprotocol = protocol = value ;
//		}
//    if((y+=FH)>7*FH) return;
//	} subN++;

//	if ( protocol != PROTO_OFF )
//	{
//		if(t_pgOfs<=subN)
//		{
//  	  lcd_puts_Pleft(    y, PSTR(STR_PROTO));//sub==2 ? INVERS:0);
//			lcd_putsAttIdx(  6*FW, y, PSTR(STR_PROT_OPT), protocol, (sub==subN && subSub==0 ? blink:0) );
//  	  if ( protocol == PROTO_PPM )
//			{
//				uint8_t x ;
//			  lcd_puts_Pleft( y, PSTR(STR_21_USEC) );
//				x = 10*FW ;
//				if ( g_model.xppmNCH > 12 )
//				{
//					g_model.xppmNCH = 0 ;		// Correct if wrong from DSM
//				}
//				uint8_t channels = (g_model.xppmNCH + 4) * 2 ;
//				if ( channels > 16 )
//				{
//					channels -= 13 ;
//				}
//				uint8_t attr = LEFT ;
//				if ( (sub==subN) && ( subSub==1 ) )
//				{
//					attr |= blink ;
//				}
//				lcd_outdezAtt(  x, y, channels, attr ) ;
//  			lcd_putsAtt( Lcd_lastPos, y, PSTR( STR_PPMCHANNELS ), attr ) ;
//				lcd_outdezAtt(  x+7*FW-1, y,  (g_model.xppmDelay*50)+300, (sub==subN && subSub==2 ? blink:0));
//  	  }
//  	  else if ( ( protocol == PROTO_PXX) || ( ( protocol == PROTO_DSM2) && (g_model.xsub_protocol != DSM_9XR) ) || (protocol == PROTO_MULTI) )
//  	  {
//			    lcd_puts_Pleft( y, PSTR(STR_13_RXNUM) );
//  	      lcd_outdezAtt(  21*FW, y,  g_model.xPxxRxNum, (sub==subN && subSub==1 ? blink:0));
//  	  }
//  	  else if ( ( ( protocol == PROTO_DSM2) && ( g_model.xsub_protocol == DSM_9XR ) ) )
//  	  {
//		    lcd_puts_Pleft( y, XPSTR("\013Chans") );
// 	      lcd_outdezAtt(  21*FW, y,  g_model.xppmNCH, (sub==subN && subSub==1 ? blink:0));
//			}

//		  if(sub==subN)
//			{
//			 Columns = (protocol == PROTO_PPM) ? 2 : 1 ;
//			 if (s_editing )
//			 {
// #ifdef XFIRE
//			  uint8_t prot_max = PROT_MAX + 1 ;
// #else
//			  uint8_t prot_max = PROT_MAX ;
// #endif
//  	    switch (subSub)
//				{
//  	     	case 0:
//  	        CHECK_INCDEC_H_MODELVAR_0( g_model.xprotocol, prot_max ) ;
//						protocol = g_model.xprotocol ;
//  	      break;
//  	      case 1:
//  	        if (g_model.xprotocol == PROTO_PPM)
//						{
//							uint8_t chans = g_model.xppmNCH + 2 ;
//							chans *= 2 ;
//							if ( chans > 12 )
//							{
//								chans -= 13 ;
//							}
//  	          CHECK_INCDEC_H_MODELVAR_0( chans, 12 ) ;
//							if ( chans & 1 )	// odd
//							{
//								chans /= 2 ;
//								chans += 7 ;
//							}
//							else
//							{
//								chans /= 2 ;
//							}
//							g_model.xppmNCH = chans - 2 ;
//						}
//  	  			else if ( ( protocol == PROTO_PXX) || ( protocol == PROTO_DSM2) )
//						{
//  	          	CHECK_INCDEC_H_MODELVAR_0( g_model.xPxxRxNum,124);
//						}
//  	  			else if ( protocol == PROTO_DSM2)
//						{
//							if ( g_model.sub_protocol == DSM_9XR )
//							{
//  	          	CHECK_INCDEC_H_MODELVAR( g_model.xppmNCH,6,14) ;
//							}
//							else
//							{
//  	        		CHECK_INCDEC_H_MODELVAR_0( g_model.xPxxRxNum,124);
//							}
//						}
//						else if (protocol == PROTO_MULTI)
//						{
//  	        	CHECK_INCDEC_H_MODELVAR_0( g_model.xPxxRxNum,15);
//						}
//  	      break;
//  	      case 2:
//  	        if (g_model.xprotocol == PROTO_PPM)
//  	          CHECK_INCDEC_H_MODELVAR( g_model.xppmDelay,-4,10);
//  	       break;
//  	    }
//			 }
//  	  }
//  	  if((y+=FH)>7*FH) return;
//	}subN++;

//  	if(g_model.xprotocol == PROTO_PPM)
//		{
//			if(t_pgOfs<=subN)
//  	  {
//  	      lcd_puts_Pleft(    y, PSTR(STR_PPMFRAME_MSEC));
//					uint8_t attr = PREC1 ;
//  	      if(sub==subN) { attr = INVERS | PREC1 ; CHECK_INCDEC_H_MODELVAR( g_model.xppmFrameLength,-20,20) ; }
//  	      lcd_outdezAtt(  14*FW-1, y, (int16_t)g_model.xppmFrameLength*5 + 225, attr ) ;
//  	  	if((y+=FH)>7*FH) return;
//  	  }
//			subN += 1 ;
//		}

//		if (protocol == PROTO_PXX)
//		{
//			if(t_pgOfs<=subN)
//			{
//				lcd_puts_Pleft( y, PSTR(STR_TYPE) ) ;
//			g_model.xsub_protocol = checkIndexed( y, XPSTR(FWx10"\002""\006D16(X)D8(D) LRP   "), g_model.xsub_protocol, (sub==subN) ) ;
//				if((y+=FH)>7*FH) return ;
//			}subN++;
//			if(t_pgOfs<=subN)
//			{
//		    lcd_puts_Pleft( y, XPSTR("Chans") );
//				g_model.xppmNCH = checkIndexed( y, XPSTR(FWx10"\001""\00216 8"), g_model.xppmNCH, (sub==subN) ) ;
//		    if((y+=FH)>7*FH) return ;
//			}subN++;
//			if(t_pgOfs<=subN)
//			{
//    		lcd_puts_Pleft( y, XPSTR("Failsafe") ) ;
//				g_model.failsafeMode[0] = checkIndexed( y, XPSTR(FWx9"\004""\007Not Set     Rx Custom   HoldNoPulse"), g_model.failsafeMode[0], sub==subN ) ;
//				if ( sub == subN )
//				{
//					if (event == EVT_KEY_LONG(KEY_MENU) )
//					{
//						s_currIdx = 0 ;
//    		   	pushMenu( menuSetFailsafe ) ;
//  			  	s_editMode = 0 ;
//    		    killEvents(event);
//					}
//				}
//		    if((y+=FH)>7*FH) return ;
//			}subN++;
//		}
	
//		if (protocol == PROTO_MULTI)
//		{
//			lcd_puts_Pleft( y, PSTR(STR_MULTI_PROTO));
//			uint8_t oldValue = g_model.xsub_protocol ;
//			uint8_t attr = 0 ;
//			uint8_t svalue = oldValue & 0x3F ;
			
//			svalue = editMultiProtocol( svalue, ( sub == subN ) ) ;

//			if ( sub == subN )
//			{
//				attr = blink ;
//			}
//			if ( sub == subN )
//			{
//				attr = blink ;
//			}
//			displayMultiProtocol( svalue, y, attr ) ;
//			g_model.xsub_protocol = svalue + (g_model.xsub_protocol & 0xC0) ;
//			if(g_model.xsub_protocol==oldValue)
//				attr=(g_model.xppmNCH >> 4) &0x07 ;
//			else
//			{
//				attr=0;
//				TelemetryType = TEL_UNKNOWN ;
//			}
//			y += FH ;
//			subN++;
//			lcd_puts_Pleft( y, PSTR(STR_MULTI_TYPE));
////			char *s ;
//			uint8_t x = g_model.xsub_protocol&0x3F ;
				
// 			if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( attr, 7 ) ;
//			attr = displayMultiSubProtocol( x, attr, y, (sub==subN) ) ;
//			g_model.xppmNCH = ( attr << 4) + (g_model.xppmNCH & 0x8F);
			
//			y += FH ;
//			subN++;

//			lcd_puts_Pleft( y, PSTR(STR_MULTI_AUTO));
//			uint8_t value = (g_model.xsub_protocol>>6)&0x01 ;
//			lcd_putsAttIdx(  9*FW, y, XPSTR(M_NY_STR), value, (sub==subN && subSub==0 ? blink:0) );
//			multiOption( 21*FW, y, g_model.xoption_protocol, (sub==subN && subSub==1 ? blink:0), g_model.xsub_protocol & 0x1F ) ;
//			if(sub==subN)
//			{
//				Columns = 1;
//				if(subSub==0 && s_editing)
//				{
//					CHECK_INCDEC_H_MODELVAR_0(value, 1 );
//					g_model.xsub_protocol = (value<<6) + (g_model.xsub_protocol&0xBF);
//				}
//				if(subSub==1 && s_editing)
//					CHECK_INCDEC_H_MODELVAR(g_model.xoption_protocol, -128, 127);
//			}
//			y += FH ;
//			subN++;
			
//			lcd_puts_Pleft( y, PSTR(STR_MULTI_POWER));
//			// Power stored in xppmNCH bit7 & Option stored in option_protocol
//			value = (g_model.xppmNCH>>7)&0x01 ;
//			lcd_putsAttIdx(  6*FW, y, XPSTR(M_LH_STR), value, ((sub==subN && subSub == 0) ? blink:0) );
//			lcd_putsAttIdx(  17*FW, y, XPSTR("\002 7 8 91011"), g_model.xppmFrameLength, ((sub==subN && subSub == 1) ? blink:0) );
//			if(sub==subN)
//			{
//				Columns = 1;
//				if(subSub==0 && s_editing)
//				{
//					CHECK_INCDEC_H_MODELVAR_0(value, 1 );
//					g_model.xppmNCH = (value<<7) + (g_model.xppmNCH&0x7F);
//				}
//				if(subSub==1 && s_editing)
//					CHECK_INCDEC_H_MODELVAR_0( g_model.xppmFrameLength, 4);
//			}
//			y += FH ;
//			subN++;
//		}
		
//		if (protocol == PROTO_DSM2)
//		{
//			if(t_pgOfs<=subN)
//			{
//		    lcd_puts_Pleft(    y, PSTR(STR_DSM_TYPE));
//  		  int8_t x ;
//  	  	x = g_model.xsub_protocol ;
//		    if ( x > 3 ) x = 3 ;
//  		  g_model.xsub_protocol = x ;
//		    lcd_putsAttIdx( 10*FW, y, XPSTR(DSM2_STR), x, (sub==subN ? blink:0) ) ;
//  		  if(sub==subN) CHECK_INCDEC_H_MODELVAR_0( g_model.xsub_protocol,3);
//  	  	if((y+=FH)>7*FH) return ;
//			} subN += 1 ;
//		}
			 
//		if (protocol == PROTO_PXX)
//		{
//			if(t_pgOfs<=subN)
//			{
//		  	uint8_t attr = 0 ;
//				lcd_puts_Pleft( y, PSTR(STR_COUNTRY) ) ;
//				uint8_t lcountry = g_model.xcountry ;
//		    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( lcountry, 2 ) ;}
//				g_model.xcountry = lcountry ;
//  		  lcd_putsAttIdx( 10*FW, y, XPSTR("\003AmeJapEur"), lcountry, attr );
//		    if((y+=FH)>7*FH) return ;
//			}subN++;
			
//		}
//	}

//	if ( g_model.xprotocol != PROTO_OFF )
//	{
//		if(t_pgOfs<=subN)
//		{
//			{
//				uint8_t attr = 0 ;
//  			lcd_puts_Pleft( y, PSTR(STR_PPM_1ST_CHAN));
//  			if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.xstartChannel, 16 ) ; }
//				lcd_outdezAtt(  14*FW, y, g_model.xstartChannel + 1, attr ) ;
//			}
//		  if((y+=FH)>7*FH) return ;
//		} subN += 1 ;
//	}

//	if ( protocol != PROTO_OFF )
//	{
//  	if(g_model.xprotocol == PROTO_PPM)
//		{
//			if(t_pgOfs<=subN) {
//			  	uint8_t attr = 0 ;
//			    lcd_puts_Pleft(    y, PSTR(STR_SHIFT_SEL));
//			    if(sub==subN) { attr = INVERS ; CHECK_INCDEC_H_MODELVAR_0( g_model.xpulsePol,1);}
//			    lcd_putsAttIdx( 11*FW, y, PSTR(STR_POS_NEG),g_model.xpulsePol,attr );
//			    if((y+=FH)>7*FH) return;
//			}subN++;
//		}
//	}
//#endif

//#if defined(PCBX9D) || defined(PCB9XT)
//	if ( need_bind_range & 2 )
//	{
//		if(t_pgOfs<=subN)
//		{
//			lcd_puts_Pleft( y, PSTR(STR_BIND) ) ;
//      if(sub==subN)
//			{
//				lcd_char_inverse( FW, y, 4*FW, 0 ) ;
//				if ( event==EVT_KEY_LONG(KEY_MENU))
//				{
// 		    	BindRangeFlag[1] = PXX_BIND ;		    	//send bind code or range check code
//					s_currIdx = 1 ;
//					pushMenu(menuRangeBind) ;
//				}
//				s_editMode = 0 ;
//			}
//			if((y+=FH)>7*FH) return ;
//		} subN += 1 ;
//	}		
//	if ( need_bind_range & 0x0A )
//	{
//		if(t_pgOfs<=subN)
//		{
//			lcd_puts_Pleft( y, PSTR(STR_RANGE) ) ;
//      if(sub==subN)
//			{
//				lcd_char_inverse( FW, y, 11*FW, 0 ) ;
//				if ( event==EVT_KEY_LONG(KEY_MENU))
//				{
//    	    BindRangeFlag[1] = PXX_RANGE_CHECK ;		    	//send bind code or range check code
//					s_currIdx = 1 ;
//					pushMenu(menuRangeBind) ;
//				}
//				s_editMode = 0 ;
//			}
//    }		
//	}
//#endif
//}


void putsTrimMode( uint8_t x, uint8_t y, uint8_t phase, uint8_t idx, uint8_t att )
{
  int16_t v = getRawTrimValue(phase, idx);

  if (v > TRIM_EXTENDED_MAX)
	{
//		if ( v > TRIM_EXTENDED_MAX+MAX_MODES+1 )
//		{
//    	lcd_putcAtt(x, y, '+', att ) ;
//		}
//		else
//		{
	    uint8_t p = v - TRIM_EXTENDED_MAX - 1;
  	  if (p >= phase) p += 1 ;
    	lcd_putcAtt(x, y, '0'+p, att);
//		}
  }
  else
	{
  	lcd_putsAttIdx( x, y, PSTR(STR_1_RETA), idx, att ) ;
  }
}


void menuPhaseOne(uint8_t event)
{
  PhaseData *phase = &g_model.phaseData[s_currIdx] ;
	TITLE(PSTR(STR_FL_MODE)) ;

	static MState2 mstate2 ;
	mstate2.check_columns(event,6-1) ;
  lcd_putc( 8*FW, 0, '1'+s_currIdx ) ;

  int8_t sub = mstate2.m_posVert;
  int8_t editMode = s_editMode;

	for (uint8_t i = 0 ; i < 6 ; i += 1 )
	{
    uint8_t y = (i+2) * FH;
		uint8_t attr = (sub==i ? InverseBlink : 0);
    
		switch(i)
		{
      case 0 : // switch
				lcd_puts_Pleft( y, PSTR(STR_SWITCH) ) ;
				phase->swtch = edit_dr_switch( 10*FW, y, phase->swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			break;

      case 1 : // switch
				lcd_puts_Pleft( y, PSTR(STR_SWITCH) ) ;
				phase->swtch2 = edit_dr_switch( 10*FW, y, phase->swtch2, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			break;

      case 2 : // trims
				if ( attr )
				{
					Columns = 3 ;
				}
				lcd_puts_Pleft( y, PSTR(STR_TRIMS) ) ;
        for ( uint8_t t = 0 ; t<NUM_STICKS ; t += 1 )
				{
          putsTrimMode( (10+t)*FW, y, s_currIdx+1, t, (g_posHorz==t) ? attr : 0 ) ;
          if (attr && g_posHorz==t && ((editMode>0) || P1values.p1valdiff))
					{
            int16_t v = phase->trim[t] ;
            if (v < TRIM_EXTENDED_MAX)
						{
							v = TRIM_EXTENDED_MAX;
						}
						int16_t u = v ;
            v = checkIncDec16( v, TRIM_EXTENDED_MAX, TRIM_EXTENDED_MAX+MAX_MODES, EE_MODEL ) ;
            
						if (v != u)
						{
              if (v == TRIM_EXTENDED_MAX) v = 0 ;
//							if ( v == TRIM_EXTENDED_MAX+MAX_MODES+1 )
//							{
//								v = 2000 ;
//							}
  						phase->trim[t] = v ;
            }
          }
        }
      break;
      
			case 3 : // fadeIn
				lcd_puts_Pleft( y, XPSTR("Fade In") ) ;
    		lcd_outdezAtt(14*FW, y, phase->fadeIn * 5, attr | PREC1 ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( phase->fadeIn, 15 ) ;
			break ;
      
			case 4 : // fadeOut
				lcd_puts_Pleft( y, XPSTR("Fade Out") ) ;
		    lcd_outdezAtt(14*FW, y, phase->fadeOut * 5, attr | PREC1 ) ;
  			if( attr ) CHECK_INCDEC_H_MODELVAR_0( phase->fadeOut, 15 ) ;
			break ;
			
			case 5 : // Name
				alphaEditName( 11*FW-2, y, (uint8_t *)phase->name, sizeof(phase->name), attr, (uint8_t *)XPSTR( "Mode Name") ) ;
			break ;
	  }
	}
}

void menuModelPhases(uint8_t event)
{
	uint32_t i ;
  uint8_t attr ;
	
	TITLE(PSTR(STR_MODES));
	static MState2 mstate2;
  
	event = mstate2.check_columns( event, 7-1-1 ) ;
	
	uint8_t  sub    = mstate2.m_posVert ;

  switch (event)
	{
    case EVT_KEY_FIRST(KEY_MENU) :
    case EVT_KEY_BREAK(BTN_RE) :
       s_currIdx = sub ;
       killEvents(event);
       pushMenu(menuPhaseOne) ;
		break;
  }
    
  lcd_putc( 0, 1*FH, 'F' ) ;
  lcd_putc( 1*FW-1, 1*FH, '0' ) ;
	lcd_puts_P( 16*FW+2, 1*FH, XPSTR("RETA") ) ;

  for ( i=0 ; i<MAX_MODES ; i += 1 )
	{
    uint8_t y=(i+2)*FH ;
		PhaseData *p = &g_model.phaseData[i] ;
    attr = (i == sub) ? INVERS : 0 ;
    lcd_putc( 0, y, 'F' ) ;
    lcd_putc( 1*FW-1, y, '1'+i ) ;
		lcd_putsnAtt( 2*FW-1, y, g_model.phaseData[i].name, 6, /*BSS*/ attr ) ;
    putsDrSwitches( 8*FW, y, p->swtch, attr ) ;
    putsDrSwitches( 12*FW+1, y, p->swtch2, attr ) ;
    for ( uint8_t t = 0 ; t < NUM_STICKS ; t += 1 )
		{
			putsTrimMode( (16+t)*FW+2, y, i+1, t, attr ) ;
		}
		if ( p->fadeIn || p->fadeOut )
		{
	    lcd_putcAtt( 20*FW+2, y, '*', attr ) ;
		}
	}	 
	i = getFlightPhase() ;
	pushPlotType( PLOT_BLACK ) ;
	lcd_rect( 0, (i+1)*FH-1, 2*FW-1, 9 ) ;
	popPlotType() ;
}

const char *BackResult ;

void menuProcModelSelect(uint8_t event)
{
  static MState2 mstate2;
  TITLE(PSTR(STR_MODELSEL));

  int8_t subOld  = mstate2.m_posVert;
	
	if ( !PopupData.PopupActive )
	{
		RotaryState = ROTARY_MENU_UD ;
		event = mstate2.check_columns( event, MAX_MODELS-1 ) ;
	}

#ifdef PCBX9D
    lcd_puts_Pleft(  0, PSTR(STR_11_FREE));
    lcd_outdez(  18*FW, 0, EeFsGetFree());
#endif

  int8_t  sub    = mstate2.m_posVert;
  static uint8_t sel_editMode;
  if ( DupIfNonzero == 2 )
  {
      sel_editMode = false ;
      DupIfNonzero = 0 ;
  }
  
	if(sub-s_pgOfs < 1)        s_pgOfs = max(0,sub-1);
  else if(sub-s_pgOfs > (SCREEN_LINES-4) )  s_pgOfs = min(MAX_MODELS-(SCREEN_LINES-2), sub-(SCREEN_LINES-4));
  for(uint8_t i=0; i < SCREEN_LINES-2 ; i++)
	{
    uint8_t y=(i+2)*FH;
    uint8_t k=i+s_pgOfs;
    lcd_outdezNAtt(  3*FW, y, k+1, ((sub==k) ? INVERS : 0) + LEADING0,2);
    if(k==g_eeGeneral.currModel) lcd_putc(1,  y,'*');
    lcd_putsn_P(  4*FW, y, (char *)ModelNames[k+1],sizeof(g_model.name) ) ;
#ifdef PCBX9D
		if (ModelNames[k+1][0])
 		{
    	lcd_outdez( 20*FW, y, eeFileSize(k+1) ) ;
		}
#endif
		
		if ( (sub==k) && (sel_editMode ) )
		{
			lcd_rect( 0, y-1, 125, 9 ) ;
		}
  }

	if ( PopupData.PopupActive )
	{
		uint16_t mask ;
		if ( g_eeGeneral.currModel == mstate2.m_posVert )
		{
			mask = 0x259 ;
		}
		else
		{
			mask = ( eeModelExists( mstate2.m_posVert ) == 0 ) ?  0x96 :  0x017E ;
		}

		uint8_t popaction = doPopup( PSTR(STR_MODEL_POPUP), mask, 10, event ) ;
		
  	if ( popaction == POPUP_SELECT )
		{
			uint8_t popidx = PopupData.PopupSel ;
			if ( popidx == 0 )	// edit
			{
				RotaryState = ROTARY_MENU_LR ;
				chainMenu(menuProcModelIndex) ;
			}
			else if ( ( popidx == 1 ) || ( popidx == 2 ) )	// select or SEL/EDIT
			{

#ifdef REVX
uint32_t checkRssi(uint32_t swappingModels, uint32_t rssi) ;
 				checkRssi(1, 0) ;
#else
uint32_t checkRssi(uint32_t swappingModels) ;
				checkRssi(1) ;
#endif
				
				WatchdogTimeout = 300 ;		// 3 seconds
				stopMusic() ;
				pausePulses() ;
	      STORE_MODELVARS ;			// To make sure we write model persistent timer
#if defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
extern void eeSaveAll() ;
				eeSaveAll() ;
#else
				ee32_check_finished() ;
#endif
				g_eeGeneral.currModel = mstate2.m_posVert;
#ifdef PCBX9D				
//				init_trainer_capture(0) ;
				eeLoadModel( g_eeGeneral.currModel ) ;
#endif
#if defined(PCBLEM1)				
				eeLoadModel( g_eeGeneral.currModel ) ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
				eeLoadModel( g_eeGeneral.currModel ) ;
//				writeModelToBackupRam( g_eeGeneral.currModel+1 ) ;
//extern uint8_t PictureDrawn ;
				loadModelImage() ;
//				PictureDrawn = 0 ;
#endif
#if defined(PCBSKY) || defined(PCB9XT)
//#ifdef PCBSKY
//				setCaptureMode(0) ;
//#else
//				init_trainer_capture(0) ;
//#endif
//				TrainerMode = 0 ;	// Temporary
        ee32WaitLoadModel(g_eeGeneral.currModel); //load default values
#endif
				protocolsToModules() ;
  			checkTHR();
				checkCustom() ;
				checkSwitches() ;
				checkMultiPower() ;
 				perOut( g_chans512, NO_DELAY_SLOW | FADE_FIRST | FADE_LAST ) ;
				SportStreamingStarted = 0 ;
				speakModelVoice() ;
        resetTimers();
				VoiceCheckFlag100mS |= 2 ;// Set switch current states
				processSwitches() ;	// Guarantee unused switches are cleared
				telemetry_init( decodeTelemetryType( g_model.telemetryProtocol ) ) ;
				resumePulses() ;
#ifdef LUA
				luaLoadModelScripts() ;
#endif
#ifdef BASIC
				basicLoadModelScripts() ;
#endif
        STORE_GENERALVARS;
				if ( ( PopupData.PopupActive == 2 ) || ( popidx == 2 ) )
				{
					chainMenu(menuProcModelIndex) ;
				}
				else
				{
	        popMenu(true) ;
				}
			}
			else if ( popidx == 5 )		// Delete
			{
       	killEvents(event);
       	DupIfNonzero = 0 ;
				DupSub = sub + 1 ;
       	pushMenu(menuDeleteDupModel);
			}
			else if( popidx == 3 )	// copy
			{
				{
 	        DupIfNonzero = 1 ;
 	        DupSub = sub + 1 ;
 	        pushMenu(menuDeleteDupModel);//menuProcExpoAll);
				}
			}
			else if( popidx == 6 )	// backup
			{
				WatchdogTimeout = 300 ;		// 3 seconds
				BackResult = ee32BackupModel( mstate2.m_posVert+1 ) ;
				AlertType = MESS_TYPE ;
				AlertMessage = BackResult ;
			}
			else if( popidx == 7 )	// restore
			{
				RestoreIndex = mstate2.m_posVert+1 ;
       	pushMenu( menuProcRestore ) ;				
			}
			else if( popidx == 8 )	// replace
			{
				WatchdogTimeout = 300 ;		// 3 seconds
				BackResult = ee32BackupModel( mstate2.m_posVert+1 ) ;
				AlertType = MESS_TYPE ;
				AlertMessage = BackResult ;
				RestoreIndex = mstate2.m_posVert+1 ;
       	pushMenu( menuProcRestore ) ;				
			}
			else if( popidx == 9 )	// Notes
			{
       	pushMenu( menuEditNotes ) ;
			}
			else // Move = 4
			{
 	    	sel_editMode = true ;
			}
		}
	}
	else
	{
  	switch(event)
  	{
  	//case  EVT_KEY_FIRST(KEY_MENU):
	  	case  EVT_KEY_FIRST(KEY_EXIT):
  	    if(sel_editMode)
				{
 	        sel_editMode = false;
  	    }
      break ;

	  	case  EVT_KEY_FIRST(KEY_LEFT):
  		case  EVT_KEY_FIRST(KEY_RIGHT):
  	    if(g_eeGeneral.currModel != mstate2.m_posVert)
  	    {
          killEvents(event);
					PopupData.PopupIdx = 0 ;
					PopupData.PopupActive = 2 ;
  	    }
				else
				{
					RotaryState = ROTARY_MENU_LR ;
		      if(event==EVT_KEY_FIRST(KEY_LEFT))  chainMenu(menuProcModelIndex);//{killEvents(event);popMenu(true);}

		      if(event==EVT_KEY_FIRST(KEY_RIGHT)) chainMenu(menuProcModelIndex);
				}
 	    break;
  		
			case  EVT_KEY_FIRST(KEY_MENU) :
			case  EVT_KEY_BREAK(BTN_RE) :
				if(sel_editMode)
				{
  	    	sel_editMode = false ;
				}
				else
				{
          killEvents(event);
					PopupData.PopupIdx = 0 ;
					PopupData.PopupActive = 1 ;
				}	 
  	    s_editMode = 0 ;
  	  break;
  	
			case  EVT_KEY_LONG(BTN_RE) :
			if ( g_eeGeneral.disableBtnLong )
			{
				break ;
			}		
			case  EVT_KEY_LONG(KEY_EXIT):  // make sure exit long exits to main
  	    popMenu(true);
      break;

  		case EVT_ENTRY:
  	    sel_editMode = false;
				PopupData.PopupActive = 0 ;
  	    mstate2.m_posVert = g_eeGeneral.currModel ;
    	break;
  	}
	}

  if(sel_editMode && subOld!=sub)
	{
		ee32SwapModels( subOld+1, sub+1 ) ;

		if ( sub == g_eeGeneral.currModel )
		{
			g_eeGeneral.currModel = subOld ;
  	  STORE_GENERALVARS ;     //eeWriteGeneral();
		}
		else if ( subOld == g_eeGeneral.currModel )
		{
			g_eeGeneral.currModel = sub ;
  	  STORE_GENERALVARS ;     //eeWriteGeneral();
		}
  }
}

#ifdef ARUNI
extern uint8_t SixPosCaptured;
static uint8_t SixPosCalibration[6];
#endif

#if defined(PCBSKY) || defined(PCB9XT)
void menuConfigPots(uint8_t event)
{
	uint32_t i ;
	uint32_t nPots ;
	uint8_t j ;
	nPots = 3 + countExtraPots() ;
#ifdef PCB9XT
	if ( nPots > 7 )
	{
		nPots = 7 ;
	}
#endif
	TITLE( XPSTR( "Config. Pots") ) ;
	static MState2 mstate2 ;
	mstate2.check_columns( event, nPots-1 ) ;
	uint8_t sub = mstate2.m_posVert ;
	for ( i = 0 ; i < nPots ; i += 1 )
	{
		uint32_t k ;
		uint8_t b ;
		j = 1 << i ;
		b = g_eeGeneral.potDetents & j ? 1 : 0 ;
    uint8_t y = (i+1) * FH ;
		k = i + 5 ;
		if ( i > 2 )
		{
			k = EXTRA_POTS_START + i - 3 ;
		}
		putsChnRaw( 0, y, k, 0 ) ;
		lcd_puts_Pleft( y, "\004has Detent" ) ;
		k = (sub == i) ? InverseBlink : 0  ;
		menu_lcd_onoff( PARAM_OFS+1, y, b, k ) ;
  	if( k )
		{
			CHECK_INCDEC_H_MODELVAR_0( b, 1 ) ;
			if ( b )
			{
				g_eeGeneral.potDetents |= j ;
			}
			else
			{
				g_eeGeneral.potDetents &= ~j ;
			}
		}
	}
}
#endif



uint32_t calibration( uint8_t event )
{
	TITLE(PSTR(STR_Calibration)) ;
	
#if defined(PCBSKY) || defined(PCB9XT)
 #ifdef ARUNI
  uint8_t xpotbits = 0;
	uint32_t numAnalogCals = NUM_ANALOG_CALS + countExtraPots(&xpotbits) ;
 #else // ARUNI
	uint32_t numAnalogCals = NUM_ANALOG_CALS + countExtraPots() ;
 #endif // ARUNI
#else
 #if defined(PCBX7) // || defined (PCBXLITE)
	uint32_t numAnalogCals = NUM_ANALOG_CALS + countExtraPots() ;
 #else
	uint32_t numAnalogCals = NUM_ANALOG_CALS ;
 #endif
#endif

#ifdef ARUNI
  uint16_t z = g_model.beepANACenter3;
  z <<= 8;
  z |= g_model.beepANACenter;
  uint8_t sixpos = ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2;
  if (sixpos > 0)
    sixpos += 3;
  for(uint32_t i=0; i < numAnalogCals ; i++, z>>=1)
#else
  for(uint32_t i=0; i < numAnalogCals ; i++)
#endif
	{ //get low and high vals for sticks and trims
		
		int16_t vt = anaIn( i ) ;
#ifdef ARUNI
    if (sixpos > 0 && i == sixpos && SixPosCaptured < 6) {
      uint8_t j, v8 = (vt >> 4);  // truncated 11b analog input to 7b
      // insertion sorting: 6 positional values in ascending order
      for (j = 0; j < SixPosCaptured; j++) {
        uint8_t pv = SixPosCalibration[j];
        if (v8 < pv) {
          if ((pv - v8) > 1)
            break;    // certainly lesser
        } else if (v8 > pv) {
          if ((v8 - pv) > 1)
            continue; // certainly greater
        }
        goto next;    // pretty much same value
      }
      // make a room for the new value:
      // block move SixPosCalibration[j:(SixPosCaptured-1)] to
      // SixPosCalibration[(j+1):SixPosCaptured]
      for (uint8_t p = SixPosCaptured; p > j; --p) {
        SixPosCalibration[p] = SixPosCalibration[p-1];
      }
      SixPosCalibration[j] = v8;  // add the new value
      SixPosCaptured++;
    next:;
    }
#endif
    SharedMemory.Cal_data.loVals[i] = min(vt,SharedMemory.Cal_data.loVals[i]);
    SharedMemory.Cal_data.hiVals[i] = max(vt,SharedMemory.Cal_data.hiVals[i]);
    if(i>=4)
		{
#ifdef ARUNI
      if ((z & (1<<i)) == 0 || i == sixpos)
#else// defined(PCBX7)
			uint32_t j = i - 4 ;
			j = 1 << j ;
			if ( ( g_eeGeneral.potDetents & j ) == 0 )
//			}
//#elif defined(PCBX9D) || defined(PCBX12D)
//	    if(i < 6)			
#endif
				SharedMemory.Cal_data.midVals[i] = (SharedMemory.Cal_data.loVals[i] + SharedMemory.Cal_data.hiVals[i])/2;
		}
  }

  scroll_disabled = SharedMemory.Cal_data.idxState; // make sure we don't scroll while calibrating

  switch(event)
  {
	  case EVT_ENTRY:
      SharedMemory.Cal_data.idxState = 0;
    break;

//#if defined(PCBSKY) || defined(PCB9XT)
//  	case EVT_KEY_LONG(KEY_MENU):
//      if(SharedMemory.Cal_data.idxState == 0)
//			{
//				killEvents( event ) ;
//				pushMenu(menuConfigPots) ;
//			}
//#endif
    break;

  	case EVT_KEY_BREAK(KEY_MENU):
  	case EVT_KEY_BREAK(BTN_RE):
      SharedMemory.Cal_data.idxState++;
      if(SharedMemory.Cal_data.idxState==3)
      {
#ifdef ARUNI
        if (sixpos > 0 && SixPosCaptured == 6) {
          for (uint8_t j = 0; j < 5; j++) {
		        uint16_t v8 = SixPosCalibration[j] + SixPosCalibration[j+1];
		        g_eeGeneral.SixPositionTable[j] = v8; // set 8b break point
          }
        }
        SixPosCaptured |= 0x80;   // calibration done
#endif
        audioDefevent(AU_MENUS);
        STORE_GENERALVARS;     //eeWriteGeneral();
        SharedMemory.Cal_data.idxState = 0;
				return 0 ;
      }
    break;
  }

	switch(SharedMemory.Cal_data.idxState)
  {
  	case 0:
      //START CALIBRATION
      //[MENU]
//#if defined(PCBSKY) || defined(PCB9XT)
//      lcd_puts_Pleft( 1*FH, XPSTR(" MENU LONG for Pots") ) ;//, 15, sub>0 ? INVERS : 0);
//#endif
      lcd_puts_Pleft( 3*FH, PSTR(STR_MENU_TO_START) ) ;//, 15, sub>0 ? INVERS : 0);
    break;

	  case 1: //get mid
      //SET MIDPOINT
      //[MENU]
      
  		for(uint8_t i=0; i<numAnalogCals; i++)
      {
        SharedMemory.Cal_data.loVals[i] =  15000;
        SharedMemory.Cal_data.hiVals[i] = -15000;
        SharedMemory.Cal_data.midVals[i] = anaIn( i );
#ifdef ARUNI
        if (sixpos > 0 && i == sixpos)
        {
          // init SixPosCalibration[] with max value (0xFF)
          memset(SixPosCalibration, 0xFF, sizeof(SixPosCalibration));
          SixPosCaptured = 0;   // start calibration
        }
#endif
      }
      lcd_puts_Pleft( 2*FH, PSTR(STR_SET_MIDPOINT) ) ;//, 12, sub>0 ? INVERS : 0);
      lcd_puts_P(3*FW, 3*FH, PSTR(STR_MENU_DONE) ) ;//, 16, sub>0 ? BLINK : 0);
    break;

	  case 2:
      //MOVE STICKS/POTS
      //[MENU]
			StickScrollAllowed = 0 ;

      for(uint8_t i=0; i<numAnalogCals; i++)
			{
	      if(abs(SharedMemory.Cal_data.loVals[i]-SharedMemory.Cal_data.hiVals[i])>50)
				{
          *CalibMid[i]  = SharedMemory.Cal_data.midVals[i] ;
          int16_t v = SharedMemory.Cal_data.midVals[i] - SharedMemory.Cal_data.loVals[i] ;
          *CalibSpanNeg[i] = v - v/64 ;
          v = SharedMemory.Cal_data.hiVals[i] - SharedMemory.Cal_data.midVals[i] ;
          *CalibSpanPos[i] = v - v/64 ;
        }
			}
  		g_eeGeneral.chkSum = evalChkSum() ;
      lcd_puts_Pleft( 2*FH, PSTR(STR_MOVE_STICKS) ) ; //, 16, sub>0 ? INVERS : 0);
      lcd_puts_P(3*FW, 3*FH, PSTR(STR_MENU_DONE) ) ; //, 16, sub>0 ? BLINK : 0);
    break;
  }

  doMainScreenGrphics() ;
#if defined(PCBSKY) || defined(PCB9XT)
// Next bar display not needed, the first extra pot is in the middle 
//	if ( g_eeGeneral.extraPotsSource[0] )
//	{
//		uint8_t len ;
//    len = ((calibratedStick[7]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
//    V_BAR( 26, SCREEN_HEIGHT-8, len ) ;
//	}
	if ( g_eeGeneral.extraPotsSource[1] )
	{
		uint8_t len ;
    len = ((calibratedStick[8]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
    V_BAR( 101, SCREEN_HEIGHT-8, len ) ;
	}
#endif
//#ifdef PCBX7
//	if ( g_eeGeneral.extraPotsSource[0] )
//	{
//		uint8_t len ;
//    len = ((calibratedStick[6]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
//    V_BAR( 26, SCREEN_HEIGHT-8, len ) ;
//	}
//	if ( g_eeGeneral.extraPotsSource[1] )
//	{
//		uint8_t len ;
//    len = ((calibratedStick[7]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
//    V_BAR( 101, SCREEN_HEIGHT-8, len ) ;
//	}
//#endif
#ifdef PCBX9D
#if defined(REVPLUS) || defined(REV9E)
	if ( ( g_eeGeneral.analogMapping & MASK_6POS ) == USE_P3_6POS )
	{
		uint8_t len ;
    len = ((calibratedStick[8]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
    V_BAR( 26, SCREEN_HEIGHT-8, len ) ;
	}
#endif
#endif
#if defined(REV9E)
	uint8_t len ;
  len = ((calibratedStick[8]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
  V_BAR( 26, SCREEN_HEIGHT-8, len ) ;
  len = ((calibratedStick[9]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
  V_BAR( 101, SCREEN_HEIGHT-8, len ) ;
#endif
	
	return 1 ;	 
}



#ifndef SMALL
void startupCalibration()
{
	uint8_t event ;
	uint32_t result ;
extern void getADC_osmp( void ) ;

	getADC_osmp();
	calibration( EVT_ENTRY ) ;
	do
	{
		getADC_osmp();
  	perOut(g_chans512, 0) ;
		event = getEvent() ;
		lcd_clear() ;
		result = calibration( event ) ;
  	refreshDisplay() ;
		wdt_reset() ;
		CoTickDelay(5) ;					// 10mS for now
		if ( event == EVT_KEY_FIRST(KEY_EXIT) )
		{
			return ;
		}
extern uint32_t check_power_or_usb( void ) ;
		if ( check_power_or_usb() ) return ;		// Usb on or power off
	}	while ( result ) ;
}
#endif


void menuProcDiagCalib(uint8_t event)
{
	TITLE(PSTR(STR_Calibration));
	static MState2 mstate2 ;
	mstate2.check_columns( event, 2-1 ) ;

  mstate2.m_posVert = 0 ; // make sure we don't scroll or move cursor here

	calibration( event ) ;

}

uint16_t Current ;
uint32_t Current_sum ;
uint8_t Current_count ;

#if defined(PCBSKY) || defined(PCB9XT)
#define NUM_ANA_ITEMS		3
#endif
#ifdef PCBX9D
#define NUM_ANA_ITEMS		2
#endif
#if defined(PCBX12D) || defined(PCBX10)
#define NUM_ANA_ITEMS		2
#endif

#ifdef PCBLEM1
#define NUM_ANA_ITEMS		2
#endif

void menuProcDiagAna(uint8_t event)
{
	register uint32_t i ;
	TITLE(PSTR(STR_ANA));
	static MState2 mstate2;
	mstate2.check_columns(event, NUM_ANA_ITEMS-1) ;

#ifdef PCBX9D
	ImageDisplay = 0 ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
	ImageDisplay = 0 ;
#endif

#ifdef PCBSKY
 #ifndef REVA
	if ( g_eeGeneral.ar9xBoard == 0 )
	{
		Current_sum += Current_current ;
		if ( ++Current_count > 49 )
		{
			Current = Current_sum / 50 ;
			Current_sum = 0 ;
			Current_count = 0 ;
		}
	}
 #endif
#endif

  int8_t  sub    = mstate2.m_posVert ;
	StickScrollAllowed = 0 ;
  for(i=0; i<8; i++)
  {
    uint8_t y=i*FH;
#if defined(PCBX7) || defined (PCBXLITE)
		lcd_putsAttIdx( 4*FW-3, y, XPSTR("\002LHLVRVRHA5A6  BT"), i, 0 ) ;
#else // PCBX7 || PCBXLITE
#if defined(PCBX9LITE) || defined(PCBLEM1)
		lcd_putsAttIdx( 4*FW-3, y, XPSTR("\002LHLVRVRHA5    BT"), i, 0 ) ;
#else // PCBX9LITE
  #ifdef ARUNI
		lcd_putsAttIdx( 4*FW-3, y, XPSTR("\002LHLVRVRHP1P2P3BT"), i, 0 ) ;
  #else
		lcd_putsAttIdx( 4*FW-3, y, XPSTR("\002LHLVRVRHA5A6A7BT"), i, 0 ) ;
  #endif
#endif // PCBX9LITE
#endif // PCBX7
#if defined(PCBX7) || defined (PCBXLITE)
    if(i<6) lcd_outhex4( 6*FW, y,anaIn(i));
#else // PCBX7 || XLITE
#ifdef PCBX9LITE
    if(i<5) lcd_outhex4( 6*FW, y,anaIn(i));
#else // PCBX9LITE
    lcd_outhex4( 6*FW, y,anaIn(i));
#endif // PCBX9LITE
#endif // PCBX7
		uint8_t index = i ;
		if ( i < 4 )
		{
 			index = stickScramble[g_eeGeneral.stickMode*4+i] ;
 		}
#if defined(PCBX7) || defined (PCBXLITE)
    if(i<6)  lcd_outdezAtt(15*FW, y, (int32_t)calibratedStick[index]*1000/1024, PREC1);
#else // PCBX7 || XLITE
#if defined(PCBX9LITE) || defined(PCBLEM1)
    if(i<5)  lcd_outdezAtt(15*FW, y, (int32_t)calibratedStick[index]*1000/1024, PREC1);
#else // PCBX9LITE
    if(i<7)  lcd_outdezAtt(15*FW, y, (int32_t)calibratedStick[index]*1000/1024, PREC1);
#endif // PCBX9LITE
#endif // PCBX7
    if(i==7)
		{
			putsVBat(14*FW,y,(sub==1 ? InverseBlink : 0)|PREC1);
    	lcd_outhex4( 6*FW, y,anaIn(12));
		}
  }

#ifdef ARUNI
  if (ExtraPotBits & 1) {
		lcd_puts_Pleft( 2*FH, XPSTR("\020P4"));
  	lcd_outhex4( 17*FW, 3*FH, anaIn(7));
  }
  if (ExtraPotBits & 2) {
		lcd_puts_Pleft( 4*FH, XPSTR("\020P5"));
  	lcd_outhex4( 17*FW, 5*FH, anaIn(8));
  }
#else
#ifdef PCBSKY
	if ( g_eeGeneral.ar9xBoard )
	{
		lcd_puts_Pleft( 4*FH, XPSTR("\020AD10"));
  	lcd_outhex4( 17*FW, 5*FH,Analog_values[8]);
	}
#endif

#if defined(PCBSKY) || defined(PCB9XT)
 #ifndef REVA
	if ( g_eeGeneral.ar9xBoard )
	{
		lcd_puts_Pleft( 4*FH, XPSTR("\020AD8"));
	}
	else
	{
	  lcd_putc( 18*FW, 2*FH, 'A' ) ;
  	lcd_putc( 19*FW, 2*FH, '9' ) ;
	}
  lcd_outhex4( 17*FW, 3*FH,Analog_values[9]);
 #endif
#endif
#endif  // ARUNI

#if defined(PCBX7) || defined (PCBXLITE)
	if ( g_eeGeneral.extraPotsSource[0] )
	{
		lcd_puts_P( 4*FW-3, 6*FH, XPSTR("SR") ) ;
    lcd_outhex4( 6*FW, 6*FH, anaIn(6) ) ;
    lcd_outdezAtt(15*FW, 6*FH, (int32_t)calibratedStick[6]*1000/1024, PREC1);
	}
	if ( g_eeGeneral.extraPotsSource[1] )
	{
		lcd_puts_P( 18*FW, 5*FH, XPSTR("P3") ) ;
    lcd_outhex4( 17*FW, 6*FH, anaIn(7) ) ;
    lcd_outdezAtt(20*FW, 7*FH, (int32_t)calibratedStick[7]*1000/1024, PREC1);
	}
#endif // PCBX7 || XLITE
#if defined (PCBX9LITE)
	if ( g_eeGeneral.extraPotsSource[0] )
	{
		lcd_puts_P( 4*FW-3, 5*FH, XPSTR("SL") ) ;
    lcd_outhex4( 6*FW, 5*FH, anaIn(5) ) ;
    lcd_outdezAtt(15*FW, 5*FH, (int32_t)calibratedStick[5]*1000/1024, PREC1);
	}
	if ( g_eeGeneral.extraPotsSource[1] )
	{
		lcd_puts_P( 4*FW-3, 6*FH, XPSTR("SR") ) ;
    lcd_outhex4( 6*FW, 6*FH, anaIn(6) ) ;
    lcd_outdezAtt(15*FW, 6*FH, (int32_t)calibratedStick[6]*1000/1024, PREC1);
	}
#endif // PCBX9LITE

#ifdef PCB9XT
	lcd_outhex4( 100, 5*FH, Analog_values[5] ) ;
	lcd_outhex4( 100, 6*FH, Analog_values[6] ) ;
	lcd_outhex4( 100, 7*FH, Analog_values[7] ) ;
#endif

#if defined(PCBX12D) || defined(PCBX10)

#if defined(PCBX10)
//extern uint16_t AdcBuffer[] ;
//  lcd_outhex4( 23*FW, 0*FH,AdcBuffer[0]);
//  lcd_outhex4( 23*FW, 1*FH,AdcBuffer[1]);
//  lcd_outhex4( 23*FW, 2*FH,AdcBuffer[2]);
//  lcd_outhex4( 23*FW, 3*FH,AdcBuffer[3]);

//  lcd_outhex4( 23*FW, 8*FH,AdcBuffer[4]);
//  lcd_outhex4( 23*FW, 9*FH,AdcBuffer[5]);
//  lcd_outhex4( 23*FW, 10*FH,AdcBuffer[6]);
//  lcd_outhex4( 23*FW, 11*FH,AdcBuffer[7]);
//  lcd_outhex4( 23*FW, 12*FH,AdcBuffer[8]);
//  lcd_outhex4( 23*FW, 13*FH,AdcBuffer[9]);
//  lcd_outhex4( 23*FW, 14*FH,AdcBuffer[10]);
//  lcd_outhex4( 23*FW, 15*FH,AdcBuffer[11]);

//  lcd_outhex4( 0, 8*FH, anaIn(4));
//  lcd_outhex4( 0, 9*FH, anaIn(5));
//  lcd_outhex4( 0, 10*FH,anaIn(6));
//  lcd_outhex4( 0, 11*FH,anaIn(7));
//  lcd_outhex4( 0, 12*FH,anaIn(8));
//  lcd_outhex4( 0, 13*FH,anaIn(9));
//  lcd_outhex4( 0, 14*FH,anaIn(10));
//  lcd_outhex4( 0, 15*FH,anaIn(11));

#endif

  lcd_putc( 18*FW, 2*FH, 'A' ) ;
  lcd_putc( 19*FW, 2*FH, '9' ) ;
  lcd_outhex4( 17*FW, 3*FH,anaIn(7));
  lcd_outdezAtt(20*FW, 4*FH, (int32_t)calibratedStick[7]*1000/1024, PREC1);
  lcd_putc( 18*FW, 5*FH, 'A' ) ;
  lcd_putc( 19*FW, 5*FH, '1' ) ;
  lcd_putc( 20*FW, 5*FH, '0' ) ;
  lcd_outhex4( 17*FW, 6*FH,anaIn(9));
  lcd_outdezAtt(20*FW, 7*FH, (int32_t)calibratedStick[8]*1000/1024, PREC1);
  lcd_putc( 22*FW, 5*FH, 'A' ) ;
  lcd_putc( 23*FW, 5*FH, '1' ) ;
  lcd_putc( 24*FW, 5*FH, '1' ) ;
  lcd_outhex4( 21*FW, 6*FH,anaIn(10));
  lcd_outdezAtt(24*FW, 7*FH, (int32_t)calibratedStick[9]*1000/1024, PREC1);
  lcd_outhex4( 25*FW, 6*FH,anaIn(11));
  lcd_outhex4( 25*FW, 7*FH,anaIn(12));
#endif

#ifdef PCBX9D
#if defined(REVPLUS) || defined(REV9E)
  lcd_putc( 18*FW, 2*FH, 'A' ) ;
  lcd_putc( 19*FW, 2*FH, '9' ) ;
  lcd_outhex4( 17*FW, 3*FH,anaIn(7));
  lcd_outdezAtt(20*FW, 4*FH, (int32_t)calibratedStick[7]*1000/1024, PREC1);
  lcd_putc( 18*FW, 5*FH, 'A' ) ;
  lcd_putc( 19*FW, 5*FH, '1' ) ;
  lcd_putc( 20*FW, 5*FH, '0' ) ;
  lcd_outhex4( 17*FW, 6*FH,anaIn(8));
  lcd_outdezAtt(20*FW, 7*FH, (int32_t)calibratedStick[8]*1000/1024, PREC1);
#else
#ifndef PCBX7
#ifndef PCBXLITE
#ifndef PCBX9LITE
  lcd_putc( 18*FW, 5*FH, 'A' ) ;
  lcd_putc( 19*FW, 5*FH, '9' ) ;
  lcd_outhex4( 17*FW, 6*FH,anaIn(7));
  lcd_outdezAtt(20*FW, 7*FH, (int32_t)calibratedStick[7]*1000/1024, PREC1);
#endif // PCBX9LITE
#endif // PCBXLITE
#endif // PCBX7
#endif

#ifdef REV9E
  lcd_putc( 22*FW, 5*FH, 'A' ) ;
  lcd_putc( 23*FW, 5*FH, '1' ) ;
  lcd_putc( 24*FW, 5*FH, '1' ) ;
  lcd_outhex4( 21*FW, 6*FH,anaIn(10));
  lcd_outdezAtt(24*FW, 7*FH, (int32_t)calibratedStick[9]*1000/1024, PREC1);
  lcd_outhex4( 25*FW, 6*FH,anaIn(11));
  lcd_outhex4( 25*FW, 7*FH,anaIn(12));
#endif

#endif
  if(sub==1)
  {
    scroll_disabled = 1;
		if ( s_editMode )
		{
	    CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatCalib, -127, 127);
		}
  }
#ifdef PCBSKY
  if(sub==2)
	{
		if ( s_editMode )
		{
	    CHECK_INCDEC_H_GENVAR( g_eeGeneral.current_calib, -49, 49 ) ;
		}
	}
  scroll_disabled = 1;
#ifndef REVX
	if ( g_eeGeneral.ar9xBoard == 0 )
#endif
	{
		lcd_puts_Pleft( 5*FH, XPSTR("\022mA"));
	  lcd_outdezAtt( 21*FW, 6*FH, Current/10 , (sub==2 ? InverseBlink : 0) ) ;
	}
#endif

}

void putc_0_1( uint8_t x, uint8_t y, uint8_t value )
{
  lcd_putcAtt( x, y, value+'0', value ? INVERS : 0 ) ;
}


void menuProcDiagKeys(uint8_t event)
{
	TITLE(PSTR(STR_DIAG));
	static MState2 mstate2;
	mstate2.check_columns(event, 1-1) ;
	uint8_t x ;

//#ifdef PCBX12D
//	lcd_puts_Pleft( 0, XPSTR( "\016Type=" ) ) ;
//	lcd_putc( 20*FW, 0, (GPIOI->IDR & 0x0800) ? '1' : '0' ) ;
//#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)

#ifdef REV9E
	x = 7*FW;
#else  
	x = 8*FW;
#endif	// REV9E
  for(uint8_t i=0; i<8; i++)
	{
    uint8_t y=i*FH; //+FH;
#ifdef PCBX7
		if ( ( i != 4) && ( i != 6 ) )
#endif // PCBX7
#ifdef PCBXLITE
		if ( i < 4 )
#endif // PCBXLITE
#ifdef PCBX9LITE
		if ( ( i != 3) && ( i != 4) && ( i != 6 ) )
#endif // PCBX9LITE
#ifdef PCBLEM1
		if ( ( i != 4) && ( i != 5 ) )
#endif // PCBX7
		{
			putHwSwitchName( x, y, i, 0 ) ;
			uint32_t pos = switchPosition( i ) ;
			lcd_putc(x+2*FW, y, PSTR(HW_SWITCHARROW_STR)[pos] ) ;
		}
	}

#ifndef REV9E
#ifndef PCBX12D
#ifndef PCBX10
	if ( g_eeGeneral.analogMapping & MASK_6POS )
#endif
#endif
	{
		uint8_t k = HSW_Ele6pos0 ;
		k += switchPosition(k) ;
		putSwitchName( 13*FW, 0, k-HSW_OFFSET, 0 ) ;
	}
#endif	// nREV9E

#ifdef REV9E
	x = 11*FW - 2;
  for(uint8_t i=0; i<8; i++)
	{
    uint8_t y=i*FH; //+FH;
		putHwSwitchName( x, y, i+8, 0 ) ;
		uint32_t pos = switchPosition( i+8 ) ;
		lcd_putc(x+2*FW, y, PSTR(HW_SWITCHARROW_STR)[pos] ) ;
	}
  x=14*FW;
  for(uint8_t i=0; i<2; i++)
	{
    uint8_t y=i*FH; //+FH;
		putHwSwitchName( x, y, i+16, 0 ) ;
		uint32_t pos = switchPosition( i+16 ) ;
		lcd_putc(x+2*FW, y, PSTR(HW_SWITCHARROW_STR)[pos] ) ;
	}
#endif	// REV9E

#endif	// PCBX9D

#if defined(PCBSKY) || defined(PCB9XT)
  x = 7*FW;
  for(uint8_t i=0; i<9; i++)
  {
    uint8_t y=i*FH; //+FH;
		uint8_t displayType = 0 ;
    if(i>(SW_ID0-SW_BASE_DIAG)) y-=FH; //overwrite ID0
    uint8_t t=keyState((EnumKeys)(SW_BASE_DIAG+i));
		switch ( i )
		{
			case 0 :	// THR
				if ( g_eeGeneral.switchMapping & USE_THR_3POS )
				{
					t = switchPosition( HSW_Thr3pos0 ) ;
					displayType = 1 ;
				}
			break ;
			case 1 :	// RUD
				if ( g_eeGeneral.switchMapping & USE_RUD_3POS )
				{
					t = switchPosition( HSW_Rud3pos0 ) ;
					displayType = 1 ;
				}
			break ;
			case 2 :	// ELE
				if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
				{
					t = switchPosition( HSW_Ele3pos0 ) ;
					displayType = 1 ;
				}
			break ;
			case 3 :	// IDx
				t = switchPosition( HSW_ID0 ) ;
				displayType = 1 ;
			break ;
			case 6 :	// AIL
				y -= FH ;
				if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
				{
					t = switchPosition( HSW_Ail3pos0 ) ;
					displayType = 1 ;
				}
			break ;
			case 7 :	// GEA
				y -= FH ;
				if ( g_eeGeneral.switchMapping & USE_GEA_3POS )
				{
					t = switchPosition( HSW_Gear3pos0 ) ;
					displayType = 1 ;
				}
			break ;
			case 8 :	// TRN
				y -= FH ;
			break ;
		}
		if ( ( i < 4 ) || ( i > 5 ) )
		{
    	putsDrSwitches(x,y,i+1,0); //ohne off,on
			if ( displayType == 0 )
			{
    		lcd_putcAtt(x+FW*4+2,  y,t+'0',t ? INVERS : 0);
			}
			else
			{
				lcd_putc(x+FW*4+2, y, PSTR(HW_SWITCHARROW_STR)[t] ) ;
			}
		}
  }
#endif

  x=0;
  for(uint8_t i=0; i<6; i++)
  {
   	uint8_t y=(5-i)*FH+2*FH;
#ifdef PCBX7
 #ifndef PCBT12
		if ( ( i < 2 ) || ( i == 5 ) )
 #endif // PCBT12
#endif // PCBX7
#ifdef PCBX9LITE
		if ( ( i < 2 ) || ( i == 5 ) )
#endif // PCBX9LITE
#ifdef PCBLEM1
		if ( i < 2 )
#endif // PCBLEM1
		{
    	bool t=keyState((EnumKeys)(KEY_MENU+i));
   		lcd_putsAttIdx(  x, y, PSTR(STR_KEYNAMES),i,0);
#ifdef PCBX7
 #ifndef PCBT12
			if ( i == 5 )
			{
			  lcd_puts_Pleft( y,XPSTR(" Page") ) ;
			}
 #endif // PCBT12
#endif // PCBX7
#ifdef PCBX9LITE
			if ( i == 5 )
			{
			  lcd_puts_Pleft( y,XPSTR(" Page") ) ;
			}
#endif // PCBX9LITE
    	lcd_putcAtt(x+FW*5+2,  y,t+'0',t);
		}		 
#ifdef PCBLEM1
		if ( i == 3 )
		{
			lcd_puts_Pleft( y,XPSTR(" EncA") ) ;
			bool t = (GPIOC->IDR & 0x2000) ? 0 : 1 ;
    	lcd_putcAtt(x+FW*5+2,  y,t+'0',t);
		}
		if ( i == 4 )
		{
			lcd_puts_Pleft( y,XPSTR(" EncB") ) ;
			bool t = (GPIOC->IDR & 0x4000) ? 0 : 1 ;
    	lcd_putcAtt(x+FW*5+2,  y,t+'0',t);
		}
#endif // PCBLEM1
#ifdef PCBX9LITE
		if ( i == 3 )
		{
			lcd_puts_Pleft( y,XPSTR(" EncA") ) ;
			bool t = (GPIOE->IDR & 0x0400) ? 0 : 1 ;
    	lcd_putcAtt(x+FW*5+2,  y,t+'0',t);
		}
		if ( i == 4 )
		{
			lcd_puts_Pleft( y,XPSTR(" EncB") ) ;
			bool t = (GPIOE->IDR & 0x1000) ? 0 : 1 ;
    	lcd_putcAtt(x+FW*5+2,  y,t+'0',t);
		}
#endif // PCBX9LITE
  }
#ifdef PCBXLITE
  lcd_puts_Pleft(1*FH,XPSTR("Shift") ) ;
  bool t = (GPIOE->IDR & 0x0100) ? 0 : 1 ;
#else
  lcd_puts_Pleft(1*FH,XPSTR("  Enc") ) ;
  bool t=keyState((EnumKeys)(BTN_RE));
#endif
  lcd_putcAtt(x+FW*5+2, 1*FH,t+'0',t);

  x=14*FW;
  lcd_putsn_P(x, 3*FH,PSTR(STR_TRIM_M_P),7);
  for(uint8_t i=0; i<4; i++)
  {
    uint8_t y=i*FH+FH*4;
#if defined(PCBX12D) || defined(PCBX10)
  	lcd_HiResimg( x*2, y*2, sticksHiRes, i, 0 ) ;
#else
    lcd_img(    x,       y, sticks,i,0);
#endif
    bool tm=keyState((EnumKeys)(TRM_BASE+2*i));
    bool tp=keyState((EnumKeys)(TRM_BASE+2*i+1));
    lcd_putcAtt(x+FW*4,  y, tm+'0',tm ? INVERS : 0);
    lcd_putcAtt(x+FW*6,  y, tp+'0',tp ? INVERS : 0);
  }

#if defined(PCBSKY) || defined(PCB9XT)
	if ( g_eeGeneral.analogMapping & MASK_6POS )
	{
		uint8_t k = HSW_Ele6pos0 ;
		k += switchPosition(k) ;
		putSwitchName( 13*FW, 0, k-HSW_OFFSET, 0 ) ;
	}
#endif
	 
#if defined(PCBSKY) || defined(PCB9XT)
	if ( g_eeGeneral.switchMapping & USE_PB1 )
	{
	  lcd_puts_Pleft(7*FH,XPSTR("\010PB1") ) ;
		putc_0_1( FW*11+2, 7*FH, getSwitch00(HSW_Pb1) ) ;
	}
	
	if ( g_eeGeneral.switchMapping & ( USE_PB2 | USE_PB3 | USE_PB4 ) )
	{
	  lcd_puts_Pleft(1*FH,XPSTR("\017PB") ) ;
	}
	if ( g_eeGeneral.switchMapping & USE_PB2 )
	{
	  lcd_puts_Pleft(1*FH,XPSTR("\0212") ) ;
		putc_0_1( FW*17, 2*FH, getSwitch00(HSW_Pb2) ) ;
	}
	if ( g_eeGeneral.switchMapping & USE_PB3 )
	{
	  lcd_puts_Pleft(1*FH,XPSTR("\0223") ) ;
		putc_0_1( FW*18, 2*FH, getSwitch00(HSW_Pb3) ) ;
	}
	if ( g_eeGeneral.switchMapping & USE_PB4 )
	{
	  lcd_puts_Pleft(1*FH,XPSTR("\0234") ) ;
		putc_0_1( FW*19, 2*FH, getSwitch00(HSW_Pb4) ) ;
	}
#endif

#if defined(PCBX12D) || defined(PCBX10)
  lcd_puts_Pleft(1*FH,XPSTR("\016PB1234") ) ;
	putc_0_1( FW*16, 2*FH, getSwitch00(HSW_Pb1) ) ;
	putc_0_1( FW*17, 2*FH, getSwitch00(HSW_Pb2) ) ;
	putc_0_1( FW*18, 2*FH, getSwitch00(HSW_Pb3) ) ;
	putc_0_1( FW*19, 2*FH, getSwitch00(HSW_Pb4) ) ;
#endif
#ifdef PCBX10
		{
			lcd_puts_Pleft( 8*FH,XPSTR(" EncA") ) ;
			bool t = (GPIOH->IDR & 0x0400) ? 0 : 1 ;
    	lcd_putcAtt(x+FW*5+2, 8*FH,t+'0',t);
		}
		{
			lcd_puts_Pleft( 9*FH,XPSTR(" EncB") ) ;
			bool t = (GPIOH->IDR & 0x0800) ? 0 : 1 ;
    	lcd_putcAtt(x+FW*5+2,  9*FH,t+'0',t);
		}
#endif // PCBX10

#ifdef PCBX9D
	
	configure_pins( 0x6000, PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
	
 #if defined(PCBX9LITE) && defined(X9LS)
  lcd_puts_Pleft(1*FH,XPSTR("\017PB1") ) ;
	putc_0_1( FW*19+2, 1*FH, getSwitch00(HSW_Pb1) ) ;
  lcd_puts_Pleft(2*FH,XPSTR("\017PB2") ) ;
	putc_0_1( FW*19+2, 2*FH, getSwitch00(HSW_Pb2) ) ;
 #else
	if ( g_eeGeneral.switchMapping & USE_PB1 )
	{
	  lcd_puts_Pleft(1*FH,XPSTR("\017PB1") ) ;
		putc_0_1( FW*19+2, 1*FH, getSwitch00(HSW_Pb1) ) ;
	}
	
	if ( g_eeGeneral.switchMapping & USE_PB2 )
	{
	  lcd_puts_Pleft(2*FH,XPSTR("\017PB2") ) ;
		putc_0_1( FW*19+2, 2*FH, getSwitch00(HSW_Pb2) ) ;
	}
 #endif
	 
#ifdef PCBX9LITE
	if ( g_eeGeneral.switchMapping & USE_PB3 )
	{
	  lcd_puts_Pleft(6*FH,XPSTR("\010PB3") ) ;
		putc_0_1( FW*12, 6*FH, getSwitch00(HSW_Pb3) ) ;
	}
#endif
#endif

}

void menuProcDiagVers(uint8_t event)
{
	TITLE(PSTR(STR_Version));
	static MState2 mstate2;
	mstate2.check_columns(event, 1-1) ;
  lcd_puts_Pleft( 2*FH,Stamps ) ;
		
#ifdef PCBSKY
#ifndef REVX
	if ( g_eeGeneral.ar9xBoard == 0 )
	{
		lcd_puts_Pleft( 7*FH, XPSTR("Co Proc"));
    lcd_outhex4( 10*FW-3, 7*FH, (Coproc_valid << 8 ) + Coproc_read ) ;
	}
#endif
 #endif
}


#ifdef PCBSKY
#ifdef REVX
#define NUM_SETUP1_ITEMS	13
#define NUM_SETUP1_PG1		7
#else
#define NUM_SETUP1_ITEMS	14
#define NUM_SETUP1_PG1		8
#endif
#endif
#ifdef PCBX9D
#if defined(REVPLUS) || defined(REV9E)
#define NUM_SETUP1_ITEMS	7
#define NUM_SETUP1_PG1		6
#else
#define NUM_SETUP1_ITEMS	6
#define NUM_SETUP1_PG1		5
#endif
#endif


#ifndef PCBXLITE	
 #ifndef PCBT12
  #ifndef PCBX9LITE
   #ifndef PCBX12D
    #ifndef PCBX10
void menuProcRSSI(uint8_t event)
{
	TITLE( XPSTR( "Module RSSI" ) ) ;
	static MState2 mstate2 ;
	event = mstate2.check_columns( event, 2-1 ) ;
	Columns = 1 ;
	
	uint8_t  sub = mstate2.m_posVert ;
	uint8_t subSub = g_posHorz ;
	uint8_t blink;
	uint8_t y = 1*FH;

	switch(event)
	{
    case EVT_KEY_BREAK(KEY_DOWN):
    case EVT_KEY_BREAK(KEY_UP):
    case EVT_KEY_BREAK(KEY_LEFT):
    case EVT_KEY_BREAK(KEY_RIGHT):
      if(s_editMode)
         FrskyAlarmSendState |= 0x30 ;	 // update Fr-Sky module when edit mode exited
    break ;
		case EVT_ENTRY :
  		FrskyAlarmSendState |= 0x40 ;		// Get RSSI/TSSI alarms
		break ;
	}
	blink = InverseBlink ;
	
	uint8_t subN = 0 ;

	for (uint8_t j=0; j<2; j++)
	{
  	lcd_puts_Pleft( y, PSTR(STR_TX_RSSIALRM) );
  	if ( j == 1 )
  	{
  	    lcd_putcAtt( 0, y, 'R', 0 ) ;
  	}
  	lcd_putsAttIdx(11*FW, y, PSTR(STR_YELORGRED),frskyRSSItype[j],(sub==subN && subSub==0 ? blink:0));
  	lcd_outdezNAtt(17*FW, y, frskyRSSIlevel[j], (sub==subN && subSub==1 ? blink:0), 3);

  	if(sub==subN && (s_editMode || P1values.p1valdiff))
		{
 	    switch (subSub)
			{
  	    case 0:
  	   	  frskyRSSItype[j] = checkIncDec( frskyRSSItype[j], 0, 3, 0 ) ;
  	    break;
  	    case 1:
  	   	  frskyRSSIlevel[j] = checkIncDec16( frskyRSSIlevel[j], 0, 120, 0 ) ;
  	    break;
  	  }
  	}
  	subN++; y+=FH;
	}
}
    #endif
   #endif
  #endif // X3
 #endif
#endif

// From Bertrand, allow trainer inputs without using mixers.
// Raw trianer inputs replace raw sticks.
// Only first 4 PPMin may be calibrated.
void menuProcTrainer(uint8_t event)
{
#define NUM_TRAINER_LINES	7	
	uint8_t y;
	uint32_t lines = NUM_TRAINER_LINES-1 ;
	y = g_eeGeneral.CurrentTrainerProfile ;
	if ( SingleExpoChan )
	{
		y = s_expoChan ;
	}
	
	TrainerProfile *tProf = &g_eeGeneral.trainerProfile[y] ;
	if ( ( g_model.trainerProfile	!= y ) || ( tProf->channel[0].source == TRAINER_SLAVE ) )
	{
		lines -= 1 ;
	}
	
	TITLE(PSTR(STR_Trainer));
	static MState2 mstate2;
	static const uint8_t mstate_tab[] = {1, 1, 3, 3, 3, 3, 0/*, 0*/} ;

	 
		event = mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1,lines) ;

	if ( event == EVT_ENTRY_UP )
	{ // From alphaEdit
		g_posHorz = 0 ;
	}

	int8_t  sub    = mstate2.m_posVert;
	uint8_t subSub = g_posHorz;
	bool    edit;
	uint8_t blink = InverseBlink ;


#if defined(PCBSKY) || defined(PCB9XT)
	if ( check_soft_power() == POWER_TRAINER )		// On trainer power
	{ // i am the slave
		if ( tProf->channel[0].source == TRAINER_JACK )
		{
	    lcd_puts_P( 7*FW, 3*FH, PSTR(STR_SLAVE) ) ;
	    return ;
		}
	}
#endif

	uint8_t attr = (sub==0 && subSub==0) ? blink : 0 ;
	lcd_putcAtt( 45, 0, y + '0', attr ) ;
	
	if ( SingleExpoChan )
	{
		attr = 0 ;
	}
	if ( attr )
	{
		CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.CurrentTrainerProfile, 3 ) ;
	}

	attr = 0 ;
	lcd_puts_Pleft( 0, XPSTR("\011Src:"));
	if ( ( sub == 0 ) && ( subSub == 1 ) )
	{
		attr = blink ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
#ifdef PCBX7
		CHECK_INCDEC_H_GENVAR_0( tProf->channel[0].source, 5 ) ;
#else
 #ifdef PCBXLITE
		CHECK_INCDEC_H_GENVAR_0( tProf->channel[0].source, 2 ) ;
 #else
  #if defined(PCBX9LITE) && defined(X9LS)
		CHECK_INCDEC_H_GENVAR_0( tProf->channel[0].source, 5 ) ;
  #else
   #if defined(PCBX10) && defined(PCBREV_EXPRESS)
		CHECK_INCDEC_H_GENVAR_0( tProf->channel[0].source, 5 ) ;
   #else
		CHECK_INCDEC_H_GENVAR_0( tProf->channel[0].source, 4 ) ;
   #endif
  #endif
 #endif
#endif
#else
		CHECK_INCDEC_H_GENVAR_0( tProf->channel[0].source, 4 ) ;
#endif
	}

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
#if defined(PCBX7) || (defined(PCBX9LITE) && defined(X9LS)) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
  lcd_putsAttIdx(13*FW, 0, XPSTR("\010Jack PPMSbus    Cppm    Slave   JackSBUSBT      "), tProf->channel[0].source, attr ) ;
#else
  lcd_putsAttIdx(13*FW, 0, XPSTR("\010Jack PPMSbus    Cppm    Slave   JackSBUS"), tProf->channel[0].source, attr ) ;
#endif
	if ( tProf->channel[0].source == TRAINER_SLAVE )
	{ // i am the slave
	    lcd_puts_P(7*FW, 3*FH, PSTR(STR_SLAVE));
	    return;
	}
#else
  lcd_putsAttIdx(13*FW, 0, XPSTR("\010Jack PPMBT      COM2    Slave   JackSBUS"), tProf->channel[0].source, attr ) ;
#endif

	y = FH;
	attr = ( (sub == 1) && ( subSub == 0 ) ) ? blink : 0 ;
	alphaEditName( 1, y, tProf->profileName,
								  6, attr|ALPHA_NO_NAME, (uint8_t *)XPSTR( "Profile Name") ) ;

	attr = 0 ;
	if ( ( sub == 1 ) && ( subSub == 1 ) )
	{
		attr = blink ;
	}
	uint8_t b = tProf->channel[1].source ;
	tProf->channel[1].source = onoffMenuItem( tProf->channel[1].source, y, XPSTR("\011Invert:"), attr ) ;
	if ( tProf->channel[1].source != b )
	{
		CurrentTrainerSource = 255 ;	// Invalid
	}

	y += FH;

	lcd_puts_P(3*FW, 2*FH, XPSTR("mode   % src  sw"));
	sub -= 2 ;
	y = 3*FH;

	for (uint8_t i=0; i<4; i++)
	{
		TrainerChannel *tChan = &tProf->channel[i] ;
	  putsChnRaw(0, y, i+1, 0);

	  edit = (sub==i && subSub==0);
    lcd_putsAttIdx(4*FW, y, PSTR(STR_OFF_PLUS_EQ),tChan->mode, edit ? blink : 0);
	  if (edit && s_editMode)
	    CHECK_INCDEC_H_GENVAR_0( tChan->mode, 2); //!! bitfield

	  edit = (sub==i && subSub==1);
	  lcd_outdezAtt(11*FW, y, tChan->studWeight, edit ? blink : 0);
	  if (edit && s_editMode)
	    CHECK_INCDEC_H_GENVAR( tChan->studWeight, -125, 125) ; //!! bitfield

	  edit = (sub==i && subSub==2);
    lcd_putsAttIdx(12*FW, y, PSTR(STR_CH1_4),tChan->srcChn, edit ? blink : 0);
	  if (edit && s_editMode)
	    CHECK_INCDEC_H_GENVAR_0( tChan->srcChn, 3); //!! bitfield

	  edit = (sub==i && subSub==3);
	  putsDrSwitches(15*FW, y, tChan->swtch, edit ? blink : 0);
	  if (edit && s_editMode)
	    CHECK_INCDEC_GENERALSWITCH(event, tChan->swtch, -MaxSwitchIndex, MaxSwitchIndex) ;

	  y += FH;
	}

	if ( lines == NUM_TRAINER_LINES-1 )
	{
		edit = (sub==4);
		lcd_putsAtt(0*FW, y, PSTR(STR_CAL), edit ? blink : 0);
		for (uint8_t i=0; i<4; i++)
		{
			TrainerChannel *tChan = &tProf->channel[i] ;
  	  uint8_t x = (i*9+15)*FW/2;
  	  lcd_outdezAtt(x , y, (g_ppmIns[i]-tChan->calib)*2, PREC1);
		}
		if (edit)
		{
		  if (event==EVT_KEY_FIRST(KEY_MENU))
			{
  	    s_editMode = false ;
				for (uint8_t i=0; i<4; i++)
				{
					TrainerChannel *tChan = &tProf->channel[i] ;
					tChan->calib = g_ppmIns[i] ;
		  	}
				STORE_GENERALVARS;     //eeWriteGeneral();
		  	audioDefevent(AU_MENUS);
		  }
		}
	}
}


uint16_t s_timeCumTot;		// Total tx on time (secs)
static uint16_t s_time;
static uint8_t s_cnt;

// Timer triggers:
// OFF - disabled
// ABS AND no switch - always running
// THs AND no switch - throttle stick
// TH% AND no switch - throttle %
// cx% AND no switch - channel %
// ABS AND switch/switchm - controlled by switch
// THs AND switch/switchm - both throttle stick AND switch
// TH% AND switch/switchm - both throttle % AND switch
// cx% AND switch/switchm - both channel % AND switch

// 1. Test modeA, if OFF, timer OFF
// 2. Test modeB, if no switch, or switch is ON, timer may run, else OFF
// 3. Test modeA, if ABS timer is ON
// 4. Test modeA, if THs, timer ON if throttle not 0
// 4. Test modeA, if Th% or cx%, timer on as %

void timer(int16_t throttle_val)
{
	
	int16_t val ;
	uint8_t timer ;
	int8_t tma ;
  int16_t tmb ;
  uint16_t tv ;
  
  s_cnt++;			// Number of times val added in
	for( timer = 0 ; timer < 2 ; timer += 1 )
	{
		struct t_timer *ptimer = &s_timer[timer] ;
		uint8_t resetting = 0 ;
		if ( timer == 0 )
		{
			tmb = g_model.timer1RstSw ;
		}
		else
		{
			tmb = g_model.timer2RstSw ;
		}
		if ( tmb )
		{
			if ( tmb < -HSW_MAX )
			{
				tmb += 256 ;
			}

    	if(tmb>(HSW_MAX))	 // toggeled switch
			{
    	  uint8_t swPos = getSwitch00( tmb-(HSW_MAX) ) ;
				if ( swPos != ptimer->lastResetSwPos )
				{
					ptimer->lastResetSwPos = swPos ;
					if ( swPos )	// Now on
					{
						resetting = 1 ;
					}
				}
			}
			else
			{
				if ( getSwitch00( tmb) )
				{
					resetting = 1 ;
				}
			}
		}
		if ( resetting )
		{
			if ( timer == 0 )
			{
				resetTimern(0) ;
//				resetTimer1() ;
			}
			else
			{
				resetTimern(1) ;
//				resetTimer2() ;
			}
		}
		
		tma = g_model.timer[timer].tmrModeA ;
    tmb = g_model.timer[timer].tmrModeB ;
		if ( tmb < -HSW_MAX )
		{
			tmb += 256 ;
		}

// code for cx%
		val = throttle_val ;
   	if(tma>=TMR_VAROFS) // Cxx%
		{
 	    val = g_chans512[tma-TMR_VAROFS] ;
		}		

		val = ( val + RESX ) / (RESX/16) ;

		if ( tma != TMRMODE_NONE )		// Timer is not off
		{ // We have a triggerA so timer is running 
    	if(tmb>(HSW_MAX))	 // toggeled switch
			{
    	  if(!(ptimer->sw_toggled | ptimer->s_sum | s_cnt | s_time | ptimer->lastSwPos)) ptimer->lastSwPos = 0 ;  // if initializing then init the lastSwPos
    	  uint8_t swPos = getSwitch00( tmb-(HSW_MAX) ) ;
    	  if(swPos && !ptimer->lastSwPos)  ptimer->sw_toggled = !ptimer->sw_toggled;  //if switch is flipped first time -> change counter state
    	  ptimer->lastSwPos = swPos;
    	}
    	else
			{
				if ( tmb )
				{
    	  	ptimer->sw_toggled = getSwitch00( tmb ); //normal switch
				}
				else
				{
					ptimer->sw_toggled = 1 ;	// No trigger B so use as active
				}
			}
		}

		if ( ( ptimer->sw_toggled == 0 ) || resetting )
		{
			val = 0 ;
		}

    ptimer->s_sum += val ;   // Add val in
    if( ( (uint16_t)( get_tmr10ms()-s_time) ) < 100 )		// BEWARE of 32 bit processor extending 16 bit values
		{
			if ( timer == 0 )
			{
				continue ; //1 sec
			}
			else
			{
				return ;
			}
		}
    val     = ptimer->s_sum/s_cnt;   // Average of val over last 100mS
    ptimer->s_sum  -= val*s_cnt;     //rest (remainder not added in)

	  ptimer->s_timeCumAbs += 1;
		if ( timer == 0 )
		{
    	s_timeCumTot += 1;
			g_eeGeneral.totalElapsedTime += 1 ;
			g_model.totalTime += 1 ;
		}
		else
		{
	    s_cnt   = 0;    // ready for next 100mS
			s_time += 100;  // 100*10mS passed
		}
    if(val) ptimer->s_timeCumThr       += 1;
		if ( !resetting )
		{
    	if(ptimer->sw_toggled) ptimer->s_timeCumSw += 1;
		}
    ptimer->s_timeCum16ThrP            += val>>1;	// val/2

    tv = ptimer->s_timerVal = g_model.timer[timer].tmrVal ;
    if(tma == TMRMODE_NONE)
		{
			ptimer->s_timerState = TMR_OFF;
		}
    else
		{
			if ( tma==TMRMODE_ABS )
			{
				if ( tmb == 0 ) ptimer->s_timerVal -= ptimer->s_timeCumAbs ;
	    	else ptimer->s_timerVal -= ptimer->s_timeCumSw ; //switch
			}
	    else if(tma<TMR_VAROFS-1) ptimer->s_timerVal -= ptimer->s_timeCumThr;	// stick
		  else ptimer->s_timerVal -= ptimer->s_timeCum16ThrP/16 ; // stick% or Cx%
		}   
		 
    switch(ptimer->s_timerState)
    {
    case TMR_OFF:
        if(tma != TMRMODE_NONE) ptimer->s_timerState=TMR_RUNNING;
        break;
    case TMR_RUNNING:
        if(ptimer->s_timerVal<0 && tv) ptimer->s_timerState=TMR_BEEPING;
        break;
    case TMR_BEEPING:
        if(ptimer->s_timerVal <= -MAX_ALERT_TIME)   ptimer->s_timerState=TMR_STOPPED;
        if(tv == 0)       ptimer->s_timerState=TMR_RUNNING;
        break;
    case TMR_STOPPED:
        break;
    }

//		if ( timer == 0 )
//		{

  	  if(ptimer->last_tmr != ptimer->s_timerVal)  //beep only if seconds advance
    	{
    		ptimer->last_tmr = ptimer->s_timerVal;
        if(ptimer->s_timerState==TMR_RUNNING)
        {
					uint8_t audioControl ;
					uint8_t hapticControl = 0 ;
					if ( timer == 0 )
					{
						audioControl = g_eeGeneral.preBeep | g_model.timer1Cdown ;
						if ( audioControl )
						{
							if ( g_model.timer1Haptic & 2 )
							{
								hapticControl = 1 ;
							}
						}
					}
					else
					{
						audioControl = g_model.timer2Cdown ;
						if ( audioControl )
						{
							if ( g_model.timer2Haptic & 2 )
							{
								hapticControl = 1 ;
							}
						}
					}
            if(audioControl && g_model.timer[timer].tmrVal) // beep when 30, 15, 10, 5,4,3,2,1 seconds remaining
            {
              	if(ptimer->s_timerVal==30) {audioNamedVoiceDefevent( AU_TIMER_30, SV_30SECOND );if ( hapticControl ){audioDefevent(AU_HAPTIC1);}}
              	if(ptimer->s_timerVal==20) {audioNamedVoiceDefevent( AU_TIMER_20, SV_20SECOND );if ( hapticControl ){audioDefevent(AU_HAPTIC1);}}
                if(ptimer->s_timerVal==10) {audioNamedVoiceDefevent( AU_TIMER_10, SV_10SECOND );if ( hapticControl ){audioDefevent(AU_HAPTIC1);}}
                if(ptimer->s_timerVal<= 5)
								{
									if(ptimer->s_timerVal>= 0)
									{
										audioVoiceDefevent(AU_TIMER_LT3, ptimer->s_timerVal | VLOC_NUMSYS ) ;
										if ( hapticControl ){audioDefevent(AU_HAPTIC1);}
									}
								}
								if(g_eeGeneral.flashBeep && (ptimer->s_timerVal==30 || ptimer->s_timerVal==20 || ptimer->s_timerVal==10 || ptimer->s_timerVal<=3))
                    g_LightOffCounter = FLASH_DURATION;
            }
						div_t mins ;
						mins = div( g_model.timer[timer].tmrDir ? g_model.timer[timer].tmrVal- ptimer->s_timerVal : ptimer->s_timerVal, 60 ) ;
					hapticControl = 0 ;
					if ( timer == 0 )
					{
						audioControl = g_eeGeneral.minuteBeep | g_model.timer1Mbeep ;
						if ( audioControl )
						{
							if ( g_model.timer1Haptic & 1 )
							{
								hapticControl = 1 ;
							}
						}
					}
					else
					{
						audioControl = g_model.timer2Mbeep ;
						if ( audioControl )
						{
							if ( g_model.timer1Haptic & 1 )
							{
								hapticControl = 1 ;
							}
						}
					}
            if( audioControl && ((mins.rem)==0)) //short beep every minute
            {
							if ( mins.quot )
							{
								voiceMinutes( mins.quot ) ;
								if ( hapticControl )
								{
									audioDefevent(AU_HAPTIC1);
								}
							}
              if(g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
            }
        }
        else if(ptimer->s_timerState==TMR_BEEPING)
        {
					if ( ( timer == 0 ) && g_eeGeneral.preBeep )
					{
            audioDefevent(AU_TIMER_LT3);
            if(g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
					}
        }
    	}
//		}
    if( g_model.timer[timer].tmrDir) ptimer->s_timerVal = tv-ptimer->s_timerVal; //if counting backwards - display backwards
	}
}

TCHAR PlaylistDirectory[30] ;
char PlayListNames[PLAYLIST_COUNT][MUSIC_NAME_LENGTH+2] ;
uint16_t PlayListCount ;
extern uint16_t PlaylistIndex ;
extern char CurrentPlayName[] ;

void menuProcMusic(uint8_t event)
{
	EditType = EE_GENERAL ;
	uint32_t rows = 2 ;
	if ( MusicPlaying == MUSIC_STOPPED )
	{
		rows = 5 ;
	}
	if ( ( MusicPlaying == MUSIC_PLAYING ) || ( MusicPlaying == MUSIC_PAUSED ) )
	{
		rows = 6 ;
	}
	MENU( PSTR(STR_Music), menuTabStat, e_music, rows, {0} ) ;

	if ( event == EVT_ENTRY_UP )
	{ // From menuProcSelectUvoiceFile
		if ( FileSelectResult == 1 )
		{
			copyFileName( (char *)g_eeGeneral.musicVoiceFileName, SelectedVoiceFileName, MUSIC_NAME_LENGTH ) ;
			g_eeGeneral.musicVoiceFileName[MUSIC_NAME_LENGTH] = '\0' ;

			if ( g_eeGeneral.musicType )
			{
				// load playlist
				cpystr( cpystr( (uint8_t *)PlaylistDirectory, (uint8_t *)"\\music\\" ), g_eeGeneral.musicVoiceFileName ) ;
				fillPlaylist( PlaylistDirectory, &PlayFileControl, (char *)"WAV" ) ;
				g_eeGeneral.playListIndex = PlaylistIndex = 0 ;
			}
			STORE_GENERALVARS ;
		}
	}

  if ( ( event == EVT_KEY_FIRST(KEY_MENU) ) || ( event == EVT_KEY_BREAK(BTN_RE) ) )
	{
		if ( mstate2.m_posVert != 1 )
		{
			killEvents(event) ;
			s_editMode = 0 ;
			event = 0 ;
		}
		switch ( mstate2.m_posVert )
		{
			case 2 :
				VoiceFileType = VOICE_FILE_TYPE_MUSIC ;
      	pushMenu( menuProcSelectVoiceFile ) ;
			break ;
			case 4 :
				if ( MusicPlaying == MUSIC_STOPPED )
				{
					MusicPlaying = MUSIC_STARTING ;
				}
				else
				{
					MusicPlaying = MUSIC_STOPPING ;
				}
			break ;

			case 5 :
				if ( MusicPlaying == MUSIC_PAUSED )
				{
					MusicPlaying = MUSIC_RESUMING ;
				}
				else
				{
					MusicPlaying = MUSIC_PAUSING ;
				}
			break ;
		}
	}
	if ( g_eeGeneral.musicType )
	{
		if ( MusicPlaying == MUSIC_PLAYING )
		{
			if ( mstate2.m_posVert == 5 )
			{
			  if ( event == EVT_KEY_FIRST(KEY_LEFT) )
				{
					killEvents(event) ;
					MusicPrevNext = MUSIC_NP_PREV ;
				}
			  if ( event == EVT_KEY_FIRST(KEY_RIGHT) )
				{
					killEvents(event) ;
					MusicPrevNext = MUSIC_NP_NEXT ;
				}
			}
		}
	}

  lcd_puts_Pleft( 1*FH, XPSTR("Type")) ;
	
	uint32_t b = g_eeGeneral.musicType ;
	g_eeGeneral.musicType = checkIndexed( 1*FH, XPSTR(FWx15"\001""\004NameList"), b, (mstate2.m_posVert == 1) ) ;
	if ( g_eeGeneral.musicType != b )
	{
		g_eeGeneral.musicVoiceFileName[0] = '\0' ;
	}

  lcd_puts_Pleft( 2*FH, XPSTR("File")) ;
	lcd_putsAtt( 6*FW, 2*FH, (char *)g_eeGeneral.musicVoiceFileName, 0 ) ;
  
	g_eeGeneral.musicLoop = onoffMenuItem( g_eeGeneral.musicLoop, 3*FH, XPSTR("Loop"), mstate2.m_posVert == 3 ) ;
	
	if ( rows == 5 )
	{
  	lcd_puts_Pleft( 4*FH, XPSTR("Start")) ;
		if ( mstate2.m_posVert == 5 )
		{
			mstate2.m_posVert = 4 ;
		}
	}
	else
	{
		lcd_puts_Pleft( 4*FH, XPSTR("Stop")) ;
		lcd_puts_Pleft( 5*FH, MusicPlaying == MUSIC_PAUSED ? XPSTR("\007Resume") : XPSTR("\007Pause")) ;
		if ( g_eeGeneral.musicType )
		{
			if ( MusicPlaying == MUSIC_PLAYING )
			{
				lcd_puts_Pleft( 5*FH, XPSTR("Prev<\017>Next")) ;
			}
		}
		lcd_puts_Pleft( 6*FH, CurrentPlayName ) ;
	}

	if ( mstate2.m_posVert == 2 )
	{
		lcd_rect( 6*FW-1, 2*FH-1, MUSIC_NAME_LENGTH*FW+2, 9 ) ;
	}

	if ( mstate2.m_posVert == 4 )
	{
		lcd_char_inverse( 0, 4*FH, 30, 0 ) ;
	}
	else if ( mstate2.m_posVert == 5 )
	{
		lcd_char_inverse( 7*FW, 5*FH, 36, 0 ) ;
	}

extern uint32_t BgSizePlayed ;
extern uint32_t BgTotalSize ;

		lcd_hbar( 10, 57, 101, 6, (BgTotalSize - BgSizePlayed) * 100 / BgTotalSize ) ;
	
}

void menuProcMusicList(uint8_t event)
{
	uint32_t i ;
	uint32_t j ;
	if ( PlayListCount < 7 )
	{
		i = 1 ;
		j = PlayListCount ;
	}
	else
	{
		i = PlayListCount - 6 ;
		j = 7 ;
	}
	MENU( XPSTR("Music List"), menuTabStat, e_music1, i, {0} ) ;
	int8_t  sub    = mstate2.m_posVert;
	
	for ( i = 0 ; i < j ; i += 1 )
	{
		lcd_puts_P( 0, (i+1)*FH, PlayListNames[i+sub] ) ;
	}

}



uint16_t g_timeMain;
uint16_t g_timeRfsh ;
uint16_t g_timeMixer ;
//uint16_t g_timePXX;
uint16_t g_timeBgRead ;

void menuProcStatistic2(uint8_t event)
{
	MENU(PSTR(STR_STAT2), menuTabStat, e_stat2, 1, {0} ) ;

  switch(event)
  {
    case EVT_KEY_FIRST(KEY_MENU):
      g_timeMain = 0;
      audioDefevent(AU_MENUS) ;
    break;
    case EVT_KEY_LONG(KEY_MENU):
			g_eeGeneral.totalElapsedTime = 0 ;
    break;
  }

  lcd_puts_Pleft( 1*FH, XPSTR("On Time")) ;
  lcd_putcAtt( 11*FW+3, 1*FH, ':', 0 ) ;
	div_t qr ;
	qr = div( g_eeGeneral.totalElapsedTime, 60 ) ;
  putsTime( 9*FW, FH*1, qr.quot, 0, 0 ) ;
  lcd_outdezNAtt( 14*FW, 1*FH, qr.rem, LEADING0, 2 ) ;

  lcd_puts_Pleft( 2*FH, XPSTR("tmain          ms"));
  lcd_outdezAtt(14*FW , 2*FH, (g_timeMain)/20 ,PREC2);

#if defined(PCBLEM1)
	lcd_puts_Pleft( 3*FH, XPSTR("Refresh time(uS)"));
extern uint16_t RefreshTime ;
  lcd_outdezAtt(20*FW , 3*FH, RefreshTime, 0 ) ;
#endif

extern uint32_t MixerRate ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	lcd_puts_Pleft( 4*FH, XPSTR("Mixer Rate"));
#endif
  lcd_outdezAtt(20*FW , 4*FH, MixerRate, 0 ) ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
  lcd_puts_Pleft( 3*FH, XPSTR("trefresh       ms"));
  lcd_outdezAtt(14*FW , 3*FH, (g_timeRfsh)/20 ,PREC2);
  lcd_puts_Pleft( 4*FH, XPSTR("tmixer         ms"));
  lcd_outdezAtt(14*FW , 4*FH, (g_timeMixer)/20 ,PREC2);

//	lcd_puts_Pleft( 1*FH, XPSTR("ttimer1        us"));
//  lcd_outdezAtt(14*FW , 1*FH, (g_timePXX)/2 ,0);
#endif

//#ifdef PCBX12D
//static uint16_t tt ;
//static uint16_t ct ;
//static uint16_t mt ;
//static uint16_t count ;
//extern uint16_t TestTime ;
//extern uint16_t ClearTime ;
//extern uint16_t MenuTime ;
//if ( ++count > 25 )
//{
//	tt = TestTime ;
//	ct = ClearTime ;
//	mt = MenuTime ;
//}
//	lcd_outhex4( 134, 2*FH, TestTime ) ; lcd_outdezAtt( 190, 2*FH, tt/2, 0 ) ;
//	lcd_outhex4( 134, 3*FH, ClearTime ) ; lcd_outdezAtt( 190, 3*FH, ct/2, 0 ) ;
//	lcd_outhex4( 134, 4*FH, MenuTime ) ; lcd_outdezAtt( 190, 4*FH, mt/2, 0 ) ;
//#endif

#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
  lcd_puts_Pleft( 3*FH, XPSTR("tBgRead        ms"));
  lcd_outdezAtt(14*FW , 3*FH, (g_timeBgRead)/20 ,PREC2);
#endif
  
extern uint8_t AudioVoiceCountUnderruns ;
	lcd_puts_Pleft( 5*FH, XPSTR("Voice underruns"));
  lcd_outdezAtt( 20*FW, 5*FH, AudioVoiceCountUnderruns, 0 ) ;

  lcd_puts_P( 3*FW,  6*FH, PSTR(STR_MENU_REFRESH));

//extern uint32_t IdleCount ;
  
//	lcd_outhex4( 0,  7*FH, IdleCount >> 16 ) ;
//  lcd_outhex4( 30, 7*FH, IdleCount ) ;

extern uint32_t IdlePercent ;
  lcd_puts_Pleft( 7*FH, XPSTR("Idle time\016%"));
  lcd_outdezAtt( 14*FW-1, 7*FH, IdlePercent ,PREC2);
//  lcd_outhex4( 75, 7*FH, IdlePercent ) ;
}


#ifdef IMAGE_128
#if defined(PCBSKY) || defined(PCB9XT)
extern uint8_t ModelImage[] ;
extern uint8_t ModelImageValid ;

void roundBox5( uint32_t x, uint32_t y )
{
	lcd_vline( x-2, y-1, 3 ) ;
	lcd_vline( x+2, y-1, 3 ) ;
	lcd_hline( x-1, y-2, 3 ) ;
	lcd_hline( x-1, y+2, 3 ) ;
}

void menuImage(uint8_t event)
{
 	uint32_t i ;
	int8_t *cs = phyStick ;
  
	MENU(XPSTR("Image"), menuTabStat, e_image, 1, {0/*, 0*/}) ;
	lcd_clear() ;
	if ( ModelImageValid )
	{
		lcd_bitmap( 64, 0, ModelImage, 64, 4 /*bytes*/, 0 ) ;
	}

 	lcd_putsnAtt(0, 0, g_model.name, sizeof(g_model.name), BOLD ) ;
	putsVBat( 6*FW+1, 2*FH, DBLSIZE|NO_UNIT|CONDENSED);
  lcd_putc( 6*FW+2, 3*FH, 'V');

  DO_SQUARE( LBOX_CENTERX, (SCREEN_HEIGHT-9-BOX_WIDTH/2-1)+3, BOX_WIDTH ) ;
  DO_CROSS( LBOX_CENTERX, (SCREEN_HEIGHT-9-BOX_WIDTH/2-1)+3,3 ) ;
	DO_SQUARE( LBOX_CENTERX +( cs[0]/((2*RESX/16)/BOX_LIMIT)), ((SCREEN_HEIGHT-9-BOX_WIDTH/2-1)+3)-( cs[1]/((2*RESX/16)/BOX_LIMIT)), MARKER_WIDTH ) ;
  
	DO_SQUARE( RBOX_CENTERX, (SCREEN_HEIGHT-9-BOX_WIDTH/2-1)+3, BOX_WIDTH ) ;
  DO_CROSS( RBOX_CENTERX, (SCREEN_HEIGHT-9-BOX_WIDTH/2-1)+3,3 ) ;
	DO_SQUARE( RBOX_CENTERX +( cs[3]/((2*RESX/16)/BOX_LIMIT)), ((SCREEN_HEIGHT-9-BOX_WIDTH/2-1)+3)-( cs[2]/((2*RESX/16)/BOX_LIMIT)), MARKER_WIDTH ) ;
//	telltale( 30, cs[0], cs[1] ) ;

  {
    uint8_t x, y, len ;			// declare temporary variables
#if defined(PCBSKY) || defined(PCB9XT)
		uint32_t maxy ;
		maxy = 7 ;
		x = -5 ;
		if ( g_eeGeneral.extraPotsSource[0] )
		{
			x = - 8 ;
			maxy = 8 ;
		}

    for( y = 4 ; y < maxy ; x += 5, y += 1 )
    {
      len = ((calibratedStick[y]+RESX)/((RESX*2)/BAR_HEIGHT))+1 ;  // calculate once per loop
      V_BAR(SCREEN_WIDTH/2+x,SCREEN_HEIGHT-6, len )
    }
#endif
	}

	lcd_hline( LBOX_CENTERX-12, 61, 25 ) ;
	lcd_hline( RBOX_CENTERX-12, 61, 25 ) ;
	lcd_vline( LBOX_CENTERX-15, 34, 25 ) ;
	lcd_vline( RBOX_CENTERX+15, 34, 25 ) ;

  for( i=0 ; i<4 ; i++ )
	{
  	static uint8_t x[4]    = {LBOX_CENTERX,LBOX_CENTERX-15,RBOX_CENTERX+15,RBOX_CENTERX	} ;
		int16_t valt = getTrimValue( CurrentPhase, i ) ;
		uint8_t centre = (valt == 0) ;
  	int8_t val = max((int8_t)-(15+1),min((int8_t)(15+1),(int8_t)(valt/10)));
  	uint8_t xm, ym ;
		xm = modeFixValue( i ) ;
    xm = x[xm-1] ;
		
		pushPlotType( PLOT_BLACK ) ;
    if( (i == 1) || ( i == 2 ))
		{
  	  ym = 47 - val ;
			if ( centre )
			{
				lcd_vline( xm-1, ym-1, 3 ) ;
				lcd_vline( xm+1, ym-1, 3 ) ;
			}
			else
			{
				if ( valt < 0 )
				{
					lcd_hline( xm-1, ym+1, 3 ) ;
					plotType = PLOT_WHITE ;
					lcd_hline( xm-1, ym-1, 3 ) ;
				}
				else
				{
					lcd_hline( xm-1, ym-1, 3 ) ;
					plotType = PLOT_WHITE ;
					lcd_hline( xm-1, ym+1, 3 ) ;
				}
				lcd_hline( xm-1, ym, 3 ) ;
			}
			
		}
		else
		{
  	  ym=61;
			xm += val ; 

			if ( centre )
			{
				lcd_hline( xm-1, ym-1, 3 ) ;
				lcd_hline( xm-1, ym+1, 3 ) ;
			}
			else
			{
				if ( valt < 0 )
				{
					lcd_vline( xm-1, ym-1, 3 ) ;
					pushPlotType( PLOT_WHITE ) ;
					lcd_vline( xm+1, ym-1, 3 ) ;
					popPlotType() ;
				}
				else
				{
					lcd_vline( xm+1, ym-1, 3 ) ;
					pushPlotType( PLOT_WHITE ) ;
					lcd_vline( xm-1, ym-1, 3 ) ;
					popPlotType() ;
				}
				lcd_vline( xm, ym-1, 3 ) ;
		  }
		}
		roundBox5( xm, ym ) ;
		popPlotType() ;
	}

//extern uint8_t LoadImageResult ;
//extern TCHAR ImageFilename[] ;

//lcd_puts_Pleft( 6*FH, ImageFilename ) ;
//lcd_outhex4( 60,  7*FH, ModelImageValid ) ;
//lcd_outhex4( 90,  7*FH, LoadImageResult ) ;


}
#endif
#endif


#if defined(LUA) || defined(BASIC)

//	Run a Lua using:
//    getSelectionFullPath(lfn);
//    luaExec(lfn);
//

#ifdef LUA
void luaExec(const char * filename) ;

extern unsigned char *heap ;
extern unsigned char *EndOfHeap ;
#define availableMemory() ((unsigned int)(EndOfHeap - heap))
#endif

uint8_t ScriptDirNeeded ;

extern struct t_loadedScripts LoadedScripts[3] ;

void menuScript(uint8_t event)
{
	static MState2 mstate2 ;
	struct fileControl *fc = &FileControl ;
	uint32_t i ;
#ifndef BASIC
	uint32_t j ;
#endif
	 
//#if defined(PCBX7) || defined(REV9E)
	if ( event == 0 )
	{
extern int32_t Rotary_diff ;
		if ( RotaryState != ROTARY_MENU_LR )
		{
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
	}
//#endif
	uint8_t saveEvent = event ;
	uint8_t savedRotaryState = RotaryState ;

//	MENU(XPSTR("SCRIPT"), menuTabStat, e_Script, 1, {0} ) ;
  TITLE( "SCRIPT" ) ;

	event = mstate2.check_columns( event, 0 ) ;
	event = saveEvent ;
#ifndef BASIC
	i = availableMemory() ;
	j = i / 10000 ;
	i %= 10000 ;
	lcd_outdezAtt( 15*FW, 0, i, LEADING0 ) ;
	lcd_outdezAtt( 15*FW-FWNUM*4, 0, j, 0 ) ;
#endif

	if ( ( event == EVT_ENTRY ) || ScriptDirNeeded )
	{
		ScriptDirNeeded = 0 ;
		WatchdogTimeout = 300 ;		// 3 seconds
#ifdef LUA
		setupFileNames( (TCHAR *)"/SCRIPTS", fc, g_model.basic_lua ? (char *)"LUA" : (char *)"BAS" ) ;
#else
		setupFileNames( (TCHAR *)"/SCRIPTS", fc, (char *)"BAS" ) ;
#endif
	}
	
	if ( ( event == EVT_KEY_BREAK(BTN_RE) ) || (event == EVT_KEY_BREAK(KEY_MENU)) )
	{
		if ( savedRotaryState == ROTARY_MENU_LR )
		{
			event = 0 ;
		}
	}
	
//	switch(event)
//	{
//    case EVT_ENTRY:
//			WatchdogTimeout = 200 ;		// 2 seconds
//#ifdef LUA
//			setupFileNames( (TCHAR *)"/SCRIPTS", fc, (char *)"LUA" ) ;
//#else
//			setupFileNames( (TCHAR *)"/SCRIPTS", fc, (char *)"BAS" ) ;
//#endif
//    break ;
////		case EVT_KEY_LONG(KEY_EXIT):
////	    killEvents(event) ;
////			popMenu(false) ;
////    break ;
////#if defined(PCBX7) || defined(REV9E)
////		case EVT_KEY_BREAK(BTN_RE):
////			event = 0 ;
////    break ;
////#endif
//		case EVT_KEY_BREAK(BTN_RE) :
//		case EVT_KEY_BREAK(KEY_MENU) :
//			if ( savedRotaryState == ROTARY_MENU_LR )
//			{
//				event = 0 ;
//			}
			
//    break ;
			
//	}

	i = fileList( event, &FileControl ) ;
	if ( i == 1 )	// Select
	{
		TCHAR ScriptFilename[60] ;
		cpystr( cpystr( (uint8_t *)ScriptFilename, (uint8_t *)"/SCRIPTS/" ), (uint8_t *)SharedMemory.FileList.Filenames[fc->vpos] ) ;
		WatchdogTimeout = 300 ;		// 3 seconds
#ifdef LUA
		if ( g_model.basic_lua )
		{
			luaExec(ScriptFilename) ;
			RotaryState = ROTARY_MENU_UD ;
		}
		else
		{
			if ( loadBasic( ScriptFilename, BASIC_LOAD_ALONE ) == 0 )
			{
				// Didn't load
				basicLoadModelScripts() ;
			}
			RotaryState = ROTARY_MENU_UD ;
		}
#else
		if ( loadBasic( ScriptFilename, BASIC_LOAD_ALONE ) == 0 )
		{
			// Didn't load
			basicLoadModelScripts() ;
		}
		RotaryState = ROTARY_MENU_UD ;
//		basicExec(ScriptFilename) ;
#endif
	}
//	else if ( i == 2 )	// EXIT
//	{
//    killEvents(event) ;
////    popMenu() ;
//	}
//#ifdef BASIC


//extern uint8_t BasicLoadedType ;

//	lcd_outhex4( 0, 6*FH, BasicLoadedType ) ;
	
//extern uint8_t LoadingIndex ;
	
////extern uint8_t BasicErrorText[] ;
////	lcd_puts_Pleft( 5*FH, (char *)BasicErrorText ) ;

//#endif
}
#endif

extern uint8_t RawLogging ;

//#ifdef PCBXLITE
//struct t_PWMcontrol
//{
//volatile uint32_t timer_capture_rising_time ;
//volatile uint32_t timer_capture_value ;
//volatile uint32_t timer_capture_period ;
//} ;
//#endif

#ifdef PCBLEM1
extern void testReceive( uint8_t channel, uint8_t drate, uint8_t deviation, uint8_t clock ) ;
extern void stopTestReceive() ;
extern void testPauseOutput( uint32_t mode ) ;

 #ifdef MENUTESTINCLUDE
void menuTest(uint8_t event)
{
	static uint8_t counter = 0 ;
	static uint8_t channel = 0 ;
	static uint8_t drate = 19 ;
	static uint8_t l_Deviation = 9 ;
	static uint8_t l_clock = 1 ;
	MENU(XPSTR("Test"), menuTabStat, e_test, 5, {0} ) ;

  switch(event)
	{
		case EVT_ENTRY :
			channel = 10 ;
		break ;
		case EVT_KEY_LONG(KEY_MENU) :
			counter = 201 ;
			testPauseOutput( 1 ) ;
  		s_editMode = 0 ;
			killEvents(event) ;
		break ;
	}

	if ( counter )
	{
		if ( counter == 190 )
		{
			testReceive( channel, drate, l_Deviation, l_clock ) ;
		}
		if ( --counter == 0 )
		{
			stopTestReceive() ;
			testPauseOutput( 0 ) ;
		}
	} 


	uint8_t attr ;

	attr = mstate2.m_posVert == 1 ? InverseBlink : 0 ;

	lcd_puts_Pleft( 1*FH, "Channel" ) ;
	lcd_outdezAtt( 15*FW, 1*FH, channel, attr ) ;
	if(attr)
	{
		channel = checkIncDec( channel, 2, 120, 0 ) ;
	}

	attr = mstate2.m_posVert == 2 ? InverseBlink : 0 ;
	
	lcd_puts_Pleft( 2*FH, "Data rate" ) ;
	lcd_outdezAtt( 15*FW, 2*FH, drate, attr ) ;
	if(attr)
	{
		drate = checkIncDec( drate, 0, 60, 0 ) ;
	}

	attr = mstate2.m_posVert == 3 ? InverseBlink : 0 ;
	
	lcd_puts_Pleft( 3*FH, "Deviation" ) ;
	lcd_outdezAtt( 15*FW, 3*FH, l_Deviation, attr ) ;
	if(attr)
	{
		l_Deviation = checkIncDec( l_Deviation, 1, 19, 0 ) ;
	}

	attr = mstate2.m_posVert == 4 ? InverseBlink : 0 ;
	lcd_puts_Pleft( 4*FH, "Clock" ) ;
	lcd_outdezAtt( 15*FW, 4*FH, l_clock, attr ) ;
	if(attr)
	{
		l_clock = checkIncDec( l_clock, 0, 1, 0 ) ;
	}




}
 #endif // MENUTESTINCLUDE
#endif

#if defined(PCBX12D) || defined(PCBX10)
const uint8_t Icon24_1[] =
{
#include "icon24_1inv.lbm"
} ;

extern void lcdDrawBitmapDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t * bitmap, uint8_t type) ;

#endif

void menuDebug(uint8_t event)
{
	MENU(XPSTR("DEBUG"), menuTabStat, e_debug, 1, {0} ) ;

  switch(event)
	{
		case EVT_KEY_LONG(KEY_MENU):
			RawLogging += 1 ;
			if ( RawLogging > 2 )
			{
				RawLogging = 0 ;
			}
			killEvents(event) ;
		break ;
#if defined(PCBX10)
		case EVT_KEY_LONG(KEY_UP):
			// Force watchdog reset
	  	NVIC_SystemReset() ;
			for(;;)
			{
			}
		break ;
#endif
	}


#ifdef PCBT16

#endif

#if defined(PCBX9D)

//extern uint16_t HbDebug1 ;
//extern uint16_t HbDebug2 ;
//extern uint16_t HbDebug3 ;

//  lcd_outhex4(  0, 2*FH, HbDebug1 ) ;
//  lcd_outhex4( 30, 2*FH, HbDebug2 ) ;
//  lcd_outhex4( 60, 2*FH, XjtHeartbeatCapture.value ) ;
//  lcd_outhex4( 90, 2*FH, XjtHeartbeatCapture.valid ) ;
//  lcd_outhex4(  0, 3*FH, HbDebug3 ) ;

#endif

#if defined(PCBX12D) || defined(PCBX10)

	uint8_t *p = (uint8_t *)Icon24_1 ;
	uint16_t w ;
	uint16_t h ;
	w = *p++ ;
	h = *p++ ;
	lcdDrawBitmapDMA( 300, 10, w, h, p, 0 ) ;

#endif

#if defined(X9LS)

#endif

#ifdef ACCESS

#endif

#ifdef PCBXLITE

#endif

#ifdef PCBX10
  
	
	lcd_outhex4( 0,  3*FH, GPIOA->IDR & 0x40 ) ;
  lcd_outhex4( 6*FW, 3*FH, GPIOB->IDR ) ;
  lcd_outhex4( 12*FW, 3*FH, GPIOC->IDR ) ;
  lcd_outhex4( 18*FW, 3*FH, GPIOD->IDR ) ;

  lcd_outhex8( (uint16_t)0*FW, (uint16_t)4*FH, (uint32_t)GPIOA->MODER ) ;
  lcd_outhex8( (uint16_t)12*FW, (uint16_t)4*FH, (uint32_t)GPIOA->PUPDR ) ;
  lcd_outhex8( (uint16_t)24*FW, (uint16_t)4*FH, (uint32_t)GPIOA->AFR[0] ) ;
	
	 
//	lcd_puts_Pleft( 3*FH, XPSTR("BI=") ) ;
//  lcd_outhex4( 3*FW, 3*FH, GPIOB->IDR & 0x0008 ) ;
//	lcd_puts_P( 9*FW, 3*FH, XPSTR("BO=") ) ;
//  lcd_outhex4( 12*FW, 3*FH, GPIOB->ODR & 0x0008 ) ;

//	lcd_puts_P( 17*FW, 3*FH, XPSTR("DI=") ) ;
//  lcd_outhex4( 20*FW, 3*FH, GPIOD->IDR & 0x1000 ) ;

//extern uint16_t ExtRfOffPos ;
//extern uint16_t ExtRfOnPos ;
//  lcd_outhex4( 16*FW, 4*FH, ExtRfOffPos ) ;
//  lcd_outhex4( 21*FW, 4*FH, ExtRfOnPos ) ;



//  lcd_outhex4( 50, 3*FH, GPIOC->IDR ) ;
//  lcd_outhex4( 0,  4*FH, GPIOE->IDR ) ;
//  lcd_outhex4( 25,  4*FH, GPIOF->IDR ) ;
//  lcd_outhex4( 50,  4*FH, GPIOG->IDR ) ;
//  lcd_outhex4( 75,  4*FH, GPIOH->IDR ) ;
//  lcd_outhex4( 100,  4*FH, GPIOI->IDR & 0x8BFF ) ;
//  lcd_outhex4( 125,  4*FH, GPIOJ->IDR & 0x8E73) ;

////	lcd_outhex4( 0,  9*FH, GPIOC->MODER >> 16 ) ;
////	lcd_outhex4( 30,  9*FH, GPIOC->MODER ) ;

#endif

#if defined(PCBSKY) || defined(PCB9XT)

#ifndef SMALL

//#define PXX_DELAYS	1

//#define SPI_PCB9XT 1

//#ifdef SPI_PCB9XT
//extern uint32_t DebugSpiEnc ;

//  lcd_outhex4( 0,  2*FH, DebugSpiEnc >> 16 ) ;
//  lcd_outhex4( 30,  2*FH, DebugSpiEnc ) ;

//#else


#ifdef PXX_DELAYS
extern uint16_t PxxTime[2] ;
static uint16_t OldPxxTime[2] ;
static uint16_t counter ;	

	if ( ++counter > 49 )
	{
		counter = 0 ;
		OldPxxTime[0] = PxxTime[0] ;
		OldPxxTime[1] = PxxTime[1] ;
	}
	lcd_outhex4( 0,  1*FH, OldPxxTime[0] ) ;
  lcd_outhex4( 30, 1*FH, OldPxxTime[1] ) ;
  
#else	
	lcd_outhex4( 0,  1*FH, FrskyHubData[FR_VCC     ] ) ;
  lcd_outhex4( 30, 1*FH, FrskyHubData[FR_CURRENT ] ) ;
  lcd_outhex4( 60, 1*FH, FrskyHubData[FR_ALT_BARO] ) ;
  lcd_outhex4( 90, 1*FH, FrskyHubData[FR_BASEMODE] ) ;

  lcd_outhex4( 0,  2*FH, FrskyHubData[FR_WP_DIST ] ) ;
  lcd_outhex4( 30, 2*FH, FrskyHubData[FR_HEALTH  ] ) ;
  lcd_outhex4( 60, 2*FH, FrskyHubData[FR_MSG     ] ) ;
  lcd_outhex4( 90, 2*FH, FrskyHubData[FR_HOME_DIR] ) ;
#endif

#ifndef PCB9XT
  lcd_outhex4( 0,  3*FH, FrskyHubData[FR_HOME_DIST] ) ;
  lcd_outhex4( 30, 3*FH, FrskyHubData[FR_CPU_LOAD ] ) ;
  lcd_outhex4( 60, 3*FH, FrskyHubData[FR_GPS_HDOP ] ) ;
  lcd_outhex4( 90, 3*FH, FrskyHubData[FR_WP_NUM   ] ) ;

  lcd_outhex4( 0,  4*FH, FrskyHubData[FR_WP_BEARING] ) ;
  lcd_outhex4( 30, 4*FH, FrskyHubData[FR_VOLTS     ] ) ;
  lcd_outhex4( 60, 4*FH, FrskyHubData[FR_TEMP2     ] ) ;
  lcd_outhex4( 90, 4*FH, FrskyHubData[FR_FUEL      ] ) ;
#endif

#ifdef PCB9XT
  lcd_outhex4( 0,  3*FH, GPIOA->IDR ) ;
  lcd_outhex4( 25, 3*FH, GPIOB->IDR ) ;
  lcd_outhex4( 50, 3*FH, GPIOC->IDR ) ;
  lcd_outhex4( 75, 3*FH, check_soft_power() ) ;

//extern uint16_t TIM3Captures ;
//  lcd_outhex4( 0,  2*FH, TIM3Captures ) ;
//extern uint8_t TrainerMode ;
//  lcd_outhex4( 25,  2*FH, TrainerMode ) ;
//	TrainerProfile *tProf = &g_eeGeneral.trainerProfile[g_model.trainerProfile] ;
//  lcd_outhex4( 50,  2*FH, tProf->channel[0].source ) ;
//extern uint16_t SbusCounter ;
//  lcd_outhex4( 75,  2*FH, SbusCounter ) ;
//  lcd_outhex4( 100,  2*FH, CurrentTrainerSource ) ;

//  lcd_outhex2( 0, 3*FH, Sbus_fifo.fifo[0] ) ;
//  lcd_outhex2( 13, 3*FH, Sbus_fifo.fifo[1] ) ;
//  lcd_outhex2( 26, 3*FH, Sbus_fifo.fifo[2] ) ;
//  lcd_outhex2( 39, 3*FH, Sbus_fifo.fifo[3] ) ;
//  lcd_outhex2( 52, 3*FH, Sbus_fifo.fifo[4] ) ;
//  lcd_outhex2( 65, 3*FH, Sbus_fifo.fifo[5] ) ;
//  lcd_outhex2( 78, 3*FH, Sbus_fifo.fifo[6] ) ;
//  lcd_outhex2( 91, 3*FH, Sbus_fifo.fifo[7] ) ;
//  lcd_outhex2( 104, 3*FH, Sbus_fifo.fifo[8] ) ;
  
//	lcd_outhex2( 0,  4*FH, Sbus_fifo.fifo[9] ) ;
//  lcd_outhex2( 13, 4*FH, Sbus_fifo.fifo[10] ) ;
//  lcd_outhex2( 26, 4*FH, Sbus_fifo.fifo[11] ) ;
//  lcd_outhex2( 39, 4*FH, Sbus_fifo.fifo[12] ) ;
//  lcd_outhex2( 52, 4*FH, Sbus_fifo.fifo[13] ) ;
//  lcd_outhex2( 65, 4*FH, Sbus_fifo.fifo[14] ) ;
//  lcd_outhex2( 78, 4*FH, Sbus_fifo.fifo[15] ) ;
//  lcd_outhex2( 91, 4*FH, Sbus_fifo.fifo[16] ) ;
//  lcd_outhex2( 104,4*FH, Sbus_fifo.fifo[17] ) ;

//	lcd_outhex2( 0,  5*FH, Sbus_fifo.fifo[18] ) ;
//  lcd_outhex2( 13, 5*FH, Sbus_fifo.fifo[19] ) ;
//  lcd_outhex2( 26, 5*FH, Sbus_fifo.fifo[20] ) ;
//  lcd_outhex2( 39, 5*FH, Sbus_fifo.fifo[21] ) ;
//  lcd_outhex2( 52, 5*FH, Sbus_fifo.fifo[22] ) ;
//  lcd_outhex2( 65, 5*FH, Sbus_fifo.fifo[23] ) ;
//  lcd_outhex2( 78, 5*FH, Sbus_fifo.fifo[24] ) ;
//  lcd_outhex2( 91, 5*FH, Sbus_fifo.fifo[25] ) ;
//  lcd_outhex2( 104,5*FH, Sbus_fifo.fifo[26] ) ;



#endif

// #endif // SPI_9XT

#ifdef REVX
  lcd_outhex4( 0,  5*FH, PIOA->PIO_PDSR >> 16 ) ;
  lcd_outhex4( 20,  5*FH, PIOA->PIO_PDSR ) ;
  lcd_outhex4( 43,  5*FH, PIOB->PIO_PDSR >> 16 ) ;
  lcd_outhex4( 63,  5*FH, PIOB->PIO_PDSR ) ;
  lcd_outhex4( 86,  5*FH, PIOC->PIO_PDSR >> 16 ) ;
  lcd_outhex4( 106,  5*FH, PIOC->PIO_PDSR ) ;
#else
  
#ifndef PCB9XT
	lcd_outhex4( 0,  5*FH, FrskyHubData[FR_TEMP1     ] ) ;
  lcd_outhex4( 30, 5*FH, FrskyHubData[FR_A1_COPY   ] ) ;
extern uint8_t MultiId[] ;
  lcd_outhex4( 60,  5*FH, MultiId[1] | (MultiId[0] << 8 ) ) ;
  lcd_outhex4( 90,  5*FH, MultiId[3] | (MultiId[2] << 8 ) ) ;
#else
extern uint8_t MultiId[] ;
  lcd_outhex4( 0,  5*FH, MultiId[1] | (MultiId[0] << 8 ) ) ;
  lcd_outhex4( 30,  5*FH, MultiId[3] | (MultiId[2] << 8 ) ) ;
#endif

#endif	// REVX
#endif	// nSMALL
#endif	// PCBSKY


#if defined(PCBSKY)
#ifdef REVX

extern uint8_t BtDebugText[] ;
	lcd_puts_Pleft( 1*FH, (char *)BtDebugText ) ;

#endif	// REVX
#endif	// PCBSKY

//#ifdef PCBX7
//extern uint8_t BtRnameDebug[] ;
//	uint32_t x ;
//	uint32_t y ;
//	y = FH ;
//	for ( x = 0 ; x < 10 ; x += 2 )
//	{
//		lcd_outhex4( 12*x, y, BtRnameDebug[x+1] | (BtRnameDebug[x] << 8 ) ) ;
//	}
//	y = 2*FH ;
//	for ( x = 10 ; x < 20 ; x += 2 )
//	{
//		lcd_outhex4( 12*(x-10), y, BtRnameDebug[x+1] | (BtRnameDebug[x] << 8 ) ) ;
//	}
//	y = 3*FH ;
//	for ( x = 20 ; x < 30 ; x += 2 )
//	{
//		lcd_outhex4( 12*(x-20), y, BtRnameDebug[x+1] | (BtRnameDebug[x] << 8 ) ) ;
//	}
//	y = 4*FH ;
//	for ( x = 30 ; x < 40 ; x += 2 )
//	{
//		lcd_outhex4( 12*(x-30), y, BtRnameDebug[x+1] | (BtRnameDebug[x] << 8 ) ) ;
//	}

//#endif	// PCBX7

//#ifdef PCBX9LITE
//extern uint8_t PxxSerial[] ;
//  lcd_outhex4( 0,  2*FH, PxxSerial[0] ) ;
//  lcd_outhex4( 30, 2*FH, PxxSerial[1] ) ;
//  lcd_outhex4( 60, 2*FH, PxxSerial[2] ) ;
//  lcd_outhex4( 90, 2*FH, PxxSerial[3] ) ;
//  lcd_outhex4( 0,  3*FH, PxxSerial[4] ) ;
//  lcd_outhex4( 30, 3*FH, PxxSerial[5] ) ;
//  lcd_outhex4( 60, 3*FH, PxxSerial[6] ) ;
//  lcd_outhex4( 90, 3*FH, PxxSerial[7] ) ;
//  lcd_outhex4( 0,  4*FH, PxxSerial[8] ) ;
//  lcd_outhex4( 30, 4*FH, PxxSerial[9] ) ;
//  lcd_outhex4( 60, 4*FH, PxxSerial[10] ) ;
//  lcd_outhex4( 90, 4*FH, PxxSerial[11] ) ;
//extern uint16_t PxxDebug0 ;
//extern uint16_t PxxDebug1 ;
//  lcd_outhex4( 0,  5*FH, PxxDebug0 ) ;
//  lcd_outhex4( 30, 5*FH, PxxDebug1 ) ;
//#endif	// PCBX9LITE

#if defined(PCBX12D) || defined(PCBX10)
//extern uint16_t HbeatCounter ;
//  lcd_outhex4( 0, 3*FH, GPIOA->IDR ) ;
//  lcd_outhex4( 25, 3*FH, DAC->CR ) ;
//extern uint16_t LoadImageCount ;
//  lcd_outhex4( 0, 3*FH, LoadImageCount ) ;


//extern uint8_t LoadImageResult ;
//  lcd_outhex4( 25, 2*FH, LoadImageResult ) ;

//  lcd_outhex4( 0, 8*FH, RTC->BKP2R >> 16 ) ;
//  lcd_outhex4( 25, 8*FH, RTC->BKP2R ) ;
//  lcd_outhex4( 50, 8*FH, RTC->BKP3R >> 16 ) ;
//  lcd_outhex4( 75, 8*FH, RTC->BKP3R ) ;
//  lcd_outhex4( 100, 8*FH, RTC->BKP4R >> 16 ) ;
//  lcd_outhex4( 125, 8*FH, RTC->BKP4R ) ;
//  lcd_outhex4( 150, 8*FH, RTC->BKP5R >> 16 ) ;
//  lcd_outhex4( 175, 8*FH, RTC->BKP5R ) ;

//  lcd_outhex4( 0,   9*FH, RTC->BKP6R >> 16 ) ;
//  lcd_outhex4( 25,  9*FH, RTC->BKP6R ) ;
//  lcd_outhex4( 50,  9*FH, RTC->BKP7R >> 16 ) ;
//  lcd_outhex4( 75,  9*FH, RTC->BKP7R ) ;
//  lcd_outhex4( 100, 9*FH, RTC->BKP8R >> 16 ) ;
//  lcd_outhex4( 125, 9*FH, RTC->BKP8R ) ;
//  lcd_outhex4( 150, 9*FH, RTC->BKP9R >> 16 ) ;
//  lcd_outhex4( 175, 9*FH, RTC->BKP9R ) ;

//  lcd_outhex4( 0,   10*FH, RTC->BKP10R >> 16 ) ;
//  lcd_outhex4( 25,  10*FH, RTC->BKP10R ) ;
//  lcd_outhex4( 50,  10*FH, RTC->BKP11R >> 16 ) ;
//  lcd_outhex4( 75,  10*FH, RTC->BKP11R ) ;
//  lcd_outhex4( 100, 10*FH, RTC->BKP12R >> 16 ) ;
//  lcd_outhex4( 125, 10*FH, RTC->BKP12R ) ;
//  lcd_outhex4( 150, 10*FH, RTC->BKP13R >> 16 ) ;
//  lcd_outhex4( 175, 10*FH, RTC->BKP13R ) ;

//  lcd_outhex4( 0,   11*FH, RTC->BKP14R >> 16 ) ;
//  lcd_outhex4( 25,  11*FH, RTC->BKP14R ) ;
//  lcd_outhex4( 50,  11*FH, RTC->BKP15R >> 16 ) ;
//  lcd_outhex4( 75,  11*FH, RTC->BKP15R ) ;
//  lcd_outhex4( 100, 11*FH, RTC->BKP16R >> 16 ) ;
//  lcd_outhex4( 125, 11*FH, RTC->BKP16R ) ;
//  lcd_outhex4( 150, 11*FH, RTC->BKP17R >> 16 ) ;
//  lcd_outhex4( 175, 11*FH, RTC->BKP17R ) ;

#endif

//#if defined(PCBX9D) || defined(PCBX12D)
#if defined(PCBX9D)

//extern uint16_t XjtHbeatOffset ;
//  lcd_outhex4( 0,  3*FH, XjtHbeatOffset ) ;
//  lcd_outhex4( 25,  3*FH, XjtHeartbeatCapture.valid ) ;
//extern uint16_t HbCapDebug ;
//extern uint16_t HbCapDebug1 ;
//extern uint16_t HbCapDebug2 ;
//  lcd_outhex4( 50,  3*FH, HbCapDebug ) ;
//  lcd_outhex4( 75,  3*FH, HbCapDebug1 ) ;
//  lcd_outhex4( 100,  3*FH, HbCapDebug2 ) ;
//  lcd_outhex4( 0,  2*FH, GPIOC->MODER >> 16 ) ;
//  lcd_outhex4( 25,  2*FH, GPIOC->MODER ) ;
//  lcd_outhex4( 50,  2*FH, SYSCFG->EXTICR[2] ) ;
//  lcd_outhex4( 75,  2*FH, EXTI->IMR ) ;
//  lcd_outhex4( 100,  2*FH, EXTI->PR ) ;

//	uint8_t *pt = (uint8_t *)&g_model.Module[1] ;

//  lcd_outhex4( 0,  2*FH, *pt++ ) ;
//  lcd_outhex4( 30,  2*FH, *pt++ ) ;
//  lcd_outhex4( 60,  2*FH, *pt++ ) ;
//  lcd_outhex4( 90,  2*FH, *pt++ ) ;

//  lcd_outhex4( 0,  3*FH, *pt++ ) ;
//  lcd_outhex4( 30,  3*FH, *pt++ ) ;
//  lcd_outhex4( 60,  3*FH, *pt++ ) ;
//  lcd_outhex4( 90,  3*FH, *pt++ ) ;

//  lcd_outhex4( 0,  4*FH, *pt++ ) ;
//  lcd_outhex4( 30,  4*FH, *pt++ ) ;
//  lcd_outhex4( 60,  4*FH, *pt++ ) ;
//  lcd_outhex4( 90,  4*FH, *pt++ ) ;


#endif

//#ifdef PCBX9D
//extern uint16_t XFDebug1 ;
//extern uint16_t XFDebug2 ;
//extern uint16_t XFDebug3 ;
//extern uint16_t XFDebug4 ;
//extern uint16_t XFDebug5 ;

//  lcd_outhex4( 0,  4*FH, XFDebug1 ) ;
//  lcd_outhex4( 30, 4*FH, XFDebug2 ) ;
//  lcd_outhex4( 60, 4*FH, XFDebug3 ) ;
//  lcd_outhex4( 90, 4*FH, XFDebug4 ) ;
//  lcd_outhex4( 0,  5*FH, XFDebug5 ) ;



//#endif

extern uint16_t FrameLossCount ;
extern uint8_t TxRssi ;
  lcd_outhex4( 0*FW, 5*FH, FrameLossCount ) ;
  lcd_outhex4( 6*FW, 5*FH, TxRssi ) ;

	lcd_puts_Pleft( 6*FH, XPSTR("Raw Logging") ) ;
	lcd_putsAttIdx( PARAM_OFS, 6*FH, XPSTR("\003OFFBinHex"), RawLogging, 0 ) ;
//	menu_lcd_onoff( PARAM_OFS, 6*FH, RawLogging, 0 ) ;

extern uint16_t TelRxCount ;
	lcd_puts_Pleft( 7*FH, XPSTR("TelRxCount") ) ;
  lcd_outhex4( 12*FW,  7*FH, TelRxCount ) ;
  lcd_outhex4( 17*FW,  7*FH, FrskyTelemetryType | (TelemetryType << 8) ) ;

//#ifdef PCB9XT
//extern uint16_t I2CencCounter ;
//	lcd_puts_Pleft( 5*FH, XPSTR("\014I2C=") ) ;
//  lcd_outhex4( 17*FW,  5*FH, I2CencCounter ) ;
//#endif
}

uint8_t TrainerPolarity ;	// Input polarity

extern struct t_fifo64 CaptureRx_fifo ;

#ifdef LUA
int32_t luaFindValueIndexByName( const char * name ) ;
#endif

//extern unsigned char *heap ;
//extern unsigned char *EndOfHeap ;
extern uint32_t _ebss ;
extern uint32_t _estack ;

#ifndef SMALL
void menuProcTrainDdiag(uint8_t event)
{
	MENU(XPSTR("Train diag"), menuTabStat, e_traindiag, 3, {0} ) ;
	
	int8_t sub = mstate2.m_posVert ;

  lcd_puts_Pleft( 1*FH, XPSTR("Trainer Mode") ) ;
	lcd_putsAttIdx(16*FW, FH, XPSTR("\004NormSer Com1"), TrainerMode, (sub == 1) ? INVERS : 0 ) ;
#ifndef SMALL
  lcd_puts_Pleft( 2*FH, XPSTR("Trainer In Pol") ) ;
	lcd_putsAttIdx(16*FW, 2*FH, XPSTR("\003NegPos"), TrainerPolarity, (sub == 2) ? INVERS : 0 ) ;
#endif

#ifndef SMALL
//uint32_t size = (uint32_t)&_estack - (uint32_t)&_ebss ;

//	lcd_outhex4( 0, 4*FH, size >> 16 ) ;
//	lcd_outhex4( 24, 4*FH, size ) ;
#endif


#if defined(PCBLEM1)


#endif

	if(sub==1)
  {
		uint8_t b = TrainerMode ;
//		TrainerMode = checkIncDec16( TrainerMode, 0, 2, 0 ) ;
#ifndef REVX
		if ( g_model.telemetryRxInvert )
		{
			TrainerMode = 0 ;	// software serial in use
		}
#endif	// nREVX
		if ( TrainerMode != b )
		{
			if ( TrainerMode == 2 )
			{
#ifdef PCBSKY
				setCaptureMode( 0 ) ;			
#else
				init_trainer_capture( 0 ) ;
#endif
				init_software_com1( 9600, 0, 0 ) ;
			}
			else
			{
#ifdef PCBSKY
				configure_pins( (PIO_PA5 | PIO_PA6), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;
				setCaptureMode( TrainerMode ) ;			
#else
				init_trainer_capture( TrainerMode ) ;
#endif
			}
		}
  }
#ifndef SMALL
	else if(sub==2)
  {
		uint8_t b = TrainerPolarity ;
		TrainerPolarity = checkIncDec16( TrainerPolarity, 0, 1, 0 ) ;
		if ( TrainerPolarity != b )
		{
			if ( TrainerMode == 1 )
			{
#ifdef PCBSKY
				setCaptureMode( TrainerMode ) ;			
#else
				init_trainer_capture( TrainerMode ) ;
#endif
			}
		}
	}
#endif
}
#endif

static uint8_t Caps = 0 ;

void displayKeys( uint8_t x, uint8_t y, char *text, uint8_t count )
{
	while ( count )
	{
		char c = *text++ ;
		lcd_putc( x+1, y, c ) ;
		lcd_rect( x-2, y-1, 11, 10 ) ;
		count -= 1 ;
		x += 10 ;
	}
}

void menuSensors(uint8_t event)
{
	TITLE(XPSTR("Sensors"));
	static MState2 mstate2 ;
	mstate2.check_columns( event, NUMBER_EXTRA_IDS + 6 ) ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t blink = InverseBlink ;
	uint8_t subN = 0 ;

	if ( sub < 6 )
	{
		displayNext() ;
    for( uint32_t i = 0 ; i < 6 ; i += 1 )
		{
			uint8_t y = (1+i) * FH ;
      lcd_putsAttIdx( FW*5, y, PSTR(STR_TELEM_ITEMS), i + 69, 0 ) ;
//			if ( sub == subN )
//			{
//				SubMenuCall = 0x80 + i + 5 ;
//			}
			alphaEditName( 11*FW, y, &g_model.customTelemetryNames[i*4], 4, sub==subN, (uint8_t *)XPSTR( "Custom Name") ) ;
//																				  (uint8_t *)&PSTR(STR_TELEM_ITEMS)[(i+68)*5+1] ) ;
	 		y += FH ;
			subN += 1 ;
		}
	}
	else
	{
		subN = 6 ;

 		for( CPU_UINT j=0 ; j < NUMBER_EXTRA_IDS+1 ; j += 1 )
		{
			uint8_t attr = (sub==subN) ? blink : 0 ;
			uint8_t y = (1+j)*FH ;

			if ( j < g_model.extraSensors )
			{
				lcd_outhex4( 0, y, g_model.extraId[j].id ) ;
				lcd_putsAttIdx( 11*FW, y, XPSTR(DestString), g_model.extraId[j].dest, attr ) ;
 			  if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.extraId[j].dest, NUM_SCALE_DESTS ) ;
 			  }
			}
			else
			{
				if ( j < NUMBER_EXTRA_IDS )
				{
					lcd_puts_Pleft( y, HyphenString ) ;
 			  	if(attr)
					{				
						lcd_char_inverse( 0, y, 4*FW, 0 ) ;
					}
				}
				else
				{
					lcd_puts_Pleft( y, XPSTR("Clear All") ) ;
 			  	if(attr)
					{				
						lcd_char_inverse( 0, y, 9*FW, 0 ) ;
						if ( checkForMenuEncoderLong( event ) )
						{
							uint32_t i ;
							for ( i = 0 ; i < NUMBER_EXTRA_IDS ; i += 1 )
							{
								g_model.extraId[i].dest = 0 ;
							}
							g_model.extraSensors = 0 ;
						}
					}
				}
			}
			subN += 1 ;
		}
	}
}

void menuVario(uint8_t event)
{
	TITLE(XPSTR("Vario"));
	static MState2 mstate2 ;
	mstate2.check_columns( event, 6 ) ;
	uint8_t sub = mstate2.m_posVert ;
	uint8_t blink = InverseBlink ;

	uint8_t subN = 0 ;
 	for( CPU_UINT j=0 ; j<7 ; j += 1 )
	{
		uint8_t b ;
		uint8_t attr = (sub==subN) ? blink : 0 ;
		uint8_t y = (1+j)*FH ;

		switch ( j )
		{
			case 0 :
				lcd_puts_Pleft( y, PSTR(STR_VARIO_SRC) ) ;
				lcd_putsAttIdx( 15*FW, y, PSTR(STR_VSPD_A2), g_model.varioData.varioSource, attr ) ;
   		  if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.varioSource, 2+NUM_SCALERS ) ;
   		  }
			break ;
				
			case 1 :
				lcd_puts_Pleft( y, PSTR(STR_2SWITCH) ) ;
				g_model.varioData.swtch = edit_dr_switch( 15*FW, y, g_model.varioData.swtch, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
			break ;

			case 2 :
				lcd_puts_Pleft( y, PSTR(STR_2SENSITIVITY) ) ;
 				lcd_outdezAtt( 17*FW, y, g_model.varioData.param, attr) ;
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.varioData.param, 50 ) ;
	   		}
			break ;

			case 3 :
				lcd_puts_Pleft( y, XPSTR("Base Freq.") ) ;
 				lcd_outdezAtt( 17*FW, y, g_model.varioExtraData.baseFrequency, attr) ;
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( g_model.varioExtraData.baseFrequency, -50, 50 ) ;
	   		}
			break ;

			case 4 :
				lcd_puts_Pleft( y, XPSTR("Offset Freq.") ) ;
 				lcd_outdezAtt( 17*FW, y, g_model.varioExtraData.offsetFrequency, attr) ;
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR( g_model.varioExtraData.offsetFrequency, -20, 20 ) ;
	   		}
			break ;

			case 5 :
				lcd_puts_Pleft( y, XPSTR("Volume") ) ;
				if ( g_model.varioExtraData.volume )
				{
 					lcd_outdezAtt( 17*FW, y, g_model.varioExtraData.volume, attr) ;
				}
				else
				{
					lcd_putsAtt( FW*15, y, "Vol", attr ) ;
				}
   			if(attr)
				{
					CHECK_INCDEC_H_MODELVAR_0( g_model.varioExtraData.volume, NUM_VOL_LEVELS-1 ) ;
	   		}
			break ;

			case 6 :
	      b = g_model.varioData.sinkTones ;
				g_model.varioData.sinkTones = offonMenuItem( b, y, PSTR(STR_SINK_TONES), attr ) ;
			break ;
		}
		subN += 1 ;
	}
}

const char AlphaSource[] =   "0123456789qwertyuiop[asdfghjkl\200zxcvbnm_-. " ;
const char AlphaSource2[] = "!\":?%^&*()QWERTYUIOP]ASDFGHJKL\201ZXCVBNM<>, " ;

void menuProcAlpha(uint8_t event)
{
	struct t_alpha *Palpha = &Alpha ;
	char *keytext ;
	static uint8_t lastPosition ;
	static uint8_t lastChar ;

	lcd_puts_Pleft( 0, Palpha->PalphaHeading ? (char *)Palpha->PalphaHeading : PSTR( STR_NAME ) ) ;
	static MState2 mstate2 ;

#ifdef PAGE_NAVIGATION 
	uint8_t cursorMove = 0 ;
	if ( event == EVT_KEY_FIRST(KEY_LEFT) )
	{
		event = 0 ;
	}
	if ( event == EVT_KEY_BREAK(KEY_LEFT) )
	{
		event = 0 ;
		cursorMove = 1 ;
	}
	if ( event == EVT_KEY_LONG(KEY_LEFT) )
	{
		killEvents(event) ;
		event = 0 ;
		cursorMove = 2 ;
	}
#endif

	mstate2.check_columns( event, 5 ) ;
	keytext = Caps ? (char *)&AlphaSource2[0] : (char *)&AlphaSource[0] ;
	displayKeys( 2, 3*FH-4, keytext, 10 ) ;
	displayKeys( 5, 4*FH-3, &keytext[10], 11 ) ;
	displayKeys( 8, 5*FH-2, &keytext[21], 10 ) ;
	displayKeys( 11, 6*FH-1, &keytext[31], 10 ) ;
	lcd_puts_Pleft( 7*FH, XPSTR("CAP SPACE DEL INS < >") ) ;

//	displayKeys( 2, 9*FH-4, (char *)AlphaSource2, 22 ) ;
//	displayKeys( 5, 10*FH-3, (char *)AlphaSource2, 22 ) ;
//	displayKeys( 9, 11*FH-2, (char *)AlphaSource2, 22 ) ;
//	displayKeys( 11, 12*FH-1, (char *)AlphaSource2, 22 ) ;
//extern uint16_t DMA2DerrorCount ;
//  lcd_outhex4(  160, 1*FH, DMA2DerrorCount ) ;
//extern uint16_t DMA2DabortCount ;
//  lcd_outhex4(  160, 3*FH, DMA2DabortCount ) ;
//extern uint16_t DMA2DintxCount ;
//  lcd_outhex4(  160, 4*FH, DMA2DintxCount ) ;
//extern uint16_t LCDIntLastOp ;
//  lcd_outhex4(  160, 5*FH, LCDIntLastOp ) ;
//extern uint16_t LCDIntLastSr ;
//  lcd_outhex4(  160, 6*FH, LCDIntLastSr ) ;
//extern uint32_t LCDIntLastCr ;
//  lcd_outhex8(  160, 7*FH, LCDIntLastCr ) ;
//extern uint16_t DMA2Dtimer[5] ;
//  lcd_outhex4(  0, 14*FH, DMA2Dtimer[1] ) ;
//  lcd_outhex4(  40, 14*FH, DMA2Dtimer[3] ) ;

	int8_t sub = mstate2.m_posVert ;
	if ( event == EVT_ENTRY )
	{
		Palpha->AlphaIndex = 0 ;
		Palpha->lastSub = sub ;
		AlphaEdited = 0 ;
		mstate2.m_posVert = 1 ;
		lastPosition = 0 ;
		lastChar = Palpha->PalphaText[0] ;
	}
	
	if ( Palpha->lastSub != sub )
	{
		if ( sub == 0 )
		{
			g_posHorz = Palpha->AlphaIndex ;
		}
		Palpha->lastSub = sub ;
	}

	uint8_t index = 0 ;
	switch ( sub )
	{
		case 0 :
			Columns = Palpha->AlphaLength-1 ;
		break ;
		case 1 :
			Columns = 9 ;
		break ;
		case 2 :
			index = 10 ;
			Columns = 10 ;
		break ;
		case 3 :
			index = 21 ;
			Columns = 9 ;
		break ;
		case 4 :
			index = 31 ;
			Columns = 9 ;
		break ;
		case 5 :
			index = 40 ;
			Columns = 5 ;
		break ;
	}
	index += g_posHorz ;

	if  ( checkForMenuEncoderBreak( event ) )
	{
		if ( ( sub == 5 ) && ( g_posHorz != 1 ) )
		{
			if ( g_posHorz == 0 )
			{
				Caps = !Caps ;
			}
			if ( g_posHorz == 2 )
			{
				uint32_t i ;
				for ( i = Palpha->AlphaIndex ; i < (uint32_t)Palpha->AlphaLength-1 ; i += 1 )
				{
					Palpha->PalphaText[i] = Palpha->PalphaText[i+1] ;
				}
				Palpha->PalphaText[Palpha->AlphaLength-1] = Palpha->AlphaHex ? '0' : ' ' ;
				AlphaEdited = 1 ;
   			eeDirty( EditType & (EE_GENERAL|EE_MODEL));
			}
			if ( g_posHorz == 3 )
			{
				if ( Palpha->AlphaIndex < Palpha->AlphaLength-1 )
				{
					uint32_t i ;
					for ( i = Palpha->AlphaLength-1 ; i > Palpha->AlphaIndex ; i -= 1 )
					{
						Palpha->PalphaText[i] = Palpha->PalphaText[i-1] ;
					}
				}
				Palpha->PalphaText[Palpha->AlphaIndex] = Palpha->AlphaHex ? '0' : ' ' ; ;
				AlphaEdited = 1 ;
    		eeDirty( EditType & (EE_GENERAL|EE_MODEL));
			}
			if ( g_posHorz == 4 )
			{
				if ( Palpha->AlphaIndex )
				{
					Palpha->AlphaIndex -= 1 ;
				}
			}
			if ( g_posHorz == 5 )
			{
				if ( Palpha->AlphaIndex < Palpha->AlphaLength-1 )
				{
					Palpha->AlphaIndex += 1 ;
				}
			}
		}
		else if (sub>0)
		{
			char chr = keytext[index] ;
			if ( !( ( sub == 5 ) && ( g_posHorz != 1 ) ) )
			{
				uint32_t set = 1 ;
				if ( Palpha->AlphaHex )
				{
					if ( ( chr >= 'a' ) && ( chr <= 'f' ) )
					{
						chr -= ('a' - 'A') ;
					}
					if ( ( chr < '0' ) || ( chr > 'F' ) || ( (chr >'9') && (chr < 'A') ) )
					{
						set = 0 ;
					}
				}
				if ( set )
				{
					lastPosition = Palpha->AlphaIndex ;
					lastChar = Palpha->PalphaText[lastPosition] ;
					Palpha->PalphaText[Palpha->AlphaIndex] = chr ;
					if ( Palpha->AlphaIndex < Palpha->AlphaLength-1 )
					{
						Palpha->AlphaIndex += 1 ;
					}
					AlphaEdited = 1 ;
    			eeDirty( EditType & (EE_GENERAL|EE_MODEL));
				}
			}
		}
		else
		{
			mstate2.m_posVert = 1 ;
		}
	}
#ifdef PAGE_NAVIGATION 
	if ( cursorMove )
	{
		if ( cursorMove == 1 )
		{
			if ( Palpha->AlphaIndex < Palpha->AlphaLength-1 )
			{
				Palpha->AlphaIndex += 1 ;
			}
			else
			{
				Palpha->AlphaIndex = 0 ;
			}
		}
		else
		{
			if ( Palpha->AlphaIndex )
			{
				Palpha->AlphaIndex -= 1 ;
			}
			else
			{
				Palpha->AlphaIndex = Palpha->AlphaLength-1 ;
			}
		}
	}
#endif
	
	if ( event == EVT_KEY_LONG(KEY_MENU) )
	{
    killEvents(event) ;
		Caps = !Caps ;
	}
	if ( getEventDbl(EVT_KEY_FIRST(BTN_RE)) > 1 )
	{
  	killEvents(EVT_KEY_FIRST(BTN_RE)) ;
		Caps = !Caps ;
		Palpha->AlphaIndex = lastPosition ;
		Palpha->PalphaText[lastPosition] = lastChar ;
	}
	
	if ( sub == 0 )
	{
		Palpha->AlphaIndex = g_posHorz ;
	}

  lcd_putsnAtt( 1, FH, (char *)Palpha->PalphaText, Palpha->AlphaLength, 0 ) ;
	if ( ( sub != 0 ) || ( BLINK_ON_PHASE ) )
	{
		lcd_rect( Palpha->AlphaIndex*FW, FH-1, 7, 9 ) ;
	}

	if ( sub>0 )
	{
		if ( g_posHorz > Columns )
		{
			g_posHorz = Columns ;
		}
		uint8_t x = g_posHorz * 10 + 1 ;
		uint8_t w = 9 ;
		if ( sub == 2 )
		{
			x += 3 ;
		}
		else if ( sub == 3 )
		{
			x += 6 ;
		}
		else if ( sub == 4 )
		{
			x += 9 ;
		}
		else if ( sub == 5 )
		{
			if ( g_posHorz == 0 )
			{
				x = 0 ;
				w = 3*FW ;
			}
			else if ( g_posHorz == 1 )
			{
				w = 5*FW ;
				x = 4*FW ;
			}
			else if ( g_posHorz == 4 )
			{
				w = 1*FW ;
				x = 18*FW ;
			}
			else if ( g_posHorz == 5 )
			{
				w = 1*FW ;
				x = 20*FW ;
			}
			else
			{
				x = (g_posHorz-2) * (4* FW) + 10*FW ;
				w = 3*FW ;
			}
		}
		lcd_char_inverse( x, (sub+2)*FH+sub-5, w, 0 ) ;
	}
	s_editMode = 0 ;
}

#ifndef SMALL
void s6rRequest( uint8_t type, uint8_t index, uint8_t value )
{
	static uint8_t sportPacket[8] ;
	S6Rdata.valid = 0 ;
	sportPacket[0] = type ;
	sportPacket[1] = 0x30 ;
	sportPacket[2] = 0x0C ;
	sportPacket[3] = index ;
	sportPacket[4] = value ;
	sportPacket[5] = 0 ;
	sportPacket[6] = 0 ;
	
//#ifdef ACCESS
//	accessSportPacketSend( sportPacket, 0x16 ) ;
//#else
	sportPacketSend( sportPacket, 0x16 ) ;
//#endif
}

void menuProcS6R(uint8_t event)
{
	static uint8_t state ;
	static uint8_t index ;
	static uint8_t lastSub ;
	static uint8_t valid ;
	uint8_t min ;
	uint8_t max ;
	static int16_t value ;
	static uint8_t calState ;
#ifdef PAGE_NAVIGATION
	static uint8_t rotaryLocal ;
#endif

	MENU(XPSTR("SxR Config."), menuTabStat, e_s6r, 28, {0} ) ;
	int8_t sub = mstate2.m_posVert ;

	if ( StatusTimer )
	{
		if ( --StatusTimer == 0 )
		{
			state = ( sub != 27 ) ? 0 : 1 ;
		}
	}
	if ( event == EVT_ENTRY )
	{
		state = 0 ;
		calState = 0 ;
		S6Rdata.valid = 0 ;
		lastSub = 0 ;
#ifdef PAGE_NAVIGATION
		rotaryLocal = 0 ;
#endif
		StatusTimer = 0 ;
	}
	if ( event == EVT_KEY_BREAK(KEY_MENU) )
	{
		if ( valid == 0 )
		{
			if ( sub != 27 )
			{
				s6rRequest( 0x30, index, 0 ) ;
				state = 1 ;			
				StatusTimer = 230 ;
			}
		}
		s_editMode = 0 ;
	}
	if ( event == EVT_KEY_LONG(KEY_MENU) )
	{
		if ( valid || sub == 27 )
		{
			s6rRequest( 0x31, index, value ) ;
	    killEvents(event) ;
			StatusTimer = 100 ;
			if ( sub == 27 )
			{
				valid = 0 ;
				StatusTimer = 50 ;
			}
			state = 2 ;
		}
		s_editMode = 0 ;
	}

	if ( state == 1 )
	{
		if ( S6Rdata.valid )
		{
			state = 0 ;
			if ( index == S6Rdata.fieldIndex )
			{
				valid = 1 ;
				value = S6Rdata.value ;
			}
		}
		lcd_puts_Pleft( 4*FH, "\016Reading" ) ;
	}
	if ( state == 2 )
	{
		lcd_puts_Pleft( 4*FH, "\016Writing" ) ;
	}
	if ( lastSub != sub )
	{
		valid = 0 ;
		lastSub = sub ;
	}
	min = 0 ;
	max = 2 ;
	switch ( sub )
	{
		case 1 :
			lcd_puts_Pleft( 2*FH, "Wing type" ) ;
			index = 0x80 ;
			if ( valid )
			{
  			lcd_putsAttIdx( 12*FW, 3*FH, XPSTR("\006NormalDelta VTail "), value, InverseBlink ) ;
			}
		break ;
		case 2 :
			lcd_puts_Pleft( 2*FH, "Mounting" ) ;
			index = 0x81 ;
			max = 3 ;
			if ( valid )
			{
  			lcd_putsAttIdx( 12*FW, 3*FH, XPSTR("\006Hor   HorRevVer   VerRev"), value, InverseBlink ) ;
			}
			lcd_img( 12, 4*FH, value<2 ? S6Rimg1 : S6Rimg3, value<3 ? value & 1 : 2, 0 ) ;
			lcd_puts_Pleft( 7*FH, "<-Heading" ) ;
		break ;
		case 3 :
		case 4 :
		case 5 :
		case 6 :
		case 7 :
			lcd_puts_Pleft( 2*FH, "\005Dir." ) ;
			index = 0x82 + sub - 3 ;
  		lcd_putsAttIdx( 0, 2*FH, XPSTR("\004Ail Ele Rud Ail2Ele2"), sub-3, 0 ) ;
			if ( sub == 6 )
			{
				index = 0x9A ;
			}
			if ( sub == 7 )
			{
				index = 0x9B ;
			}
			max = 2 ;
			if ( valid )
			{
				uint8_t t = (value+1) ;
				if ( value == 0x80 )
				{
					t = 2 ;
				}
  			lcd_putsAttIdx( 12*FW, 3*FH, XPSTR("\006NormalInversOff   "), t, InverseBlink ) ;
			}
		break ;
		case 8 :
		case 9 :
		case 10 :
		case 11 :
		case 12 :
		case 13 :
		case 14 :
		case 15 :
		case 16 :
			lcd_puts_Pleft( 2*FH, sub < 11 ? "\004Stab Gain" : sub < 13 ? "\004Auto Level Gain" : sub < 15 ? "\004Upright Gain" : "\004Crab Gain" ) ;
  		lcd_putsAttIdx( 0, 2*FH, XPSTR("\003AilEleRudAilEleEleRudAilRud"), sub-8, 0 ) ;
			index = 0x85 + sub - 8 ;
			if ( sub > 12 )
			{
				index += 2 ;
			}
			if ( sub > 15 )
			{
				index += 1 ;
			}
			max = 200 ;
			if ( valid )
			{
				lcd_outdez( 17*FW, 3*FH, value ) ;
			}
		break ;
		case 17 :
		case 18 :
		case 19 :
		case 20 :
		case 21 :
		case 22 :
			lcd_puts_Pleft( 2*FH, sub < 19 ? "\004auto angle offset" : sub < 21 ? "\004up angle offset" : "\004crab angle offset" ) ;
  		lcd_putsAttIdx( 0, 2*FH, XPSTR("\003AilEleEleRudAilRud"), sub-17, 0 ) ;
			index = "\x91\x92\x95\x96\x97\x99"[sub-17] ;
			min = 0x6C ;
			max = 0x94 ;
			if ( valid )
			{
				lcd_outdez( 17*FW, 3*FH, value-128 ) ;
			}
		break ;
		case 23 :
		case 24 :
		case 25 :
		case 26 :
//			lcd_puts_Pleft( 2*FH, "Active" ) ;
  		lcd_putsAttIdx( 0, 2*FH, XPSTR("\006ActiveAux1  Aux2  QuickM"), sub-23, 0 ) ;
			index = "\x9C\xA8\xA9\xAA"[sub-23] ;
//			index = 0x9C ;
			max = 1 ;
			if ( valid )
			{
  			lcd_putsAttIdx( 12*FW, 3*FH, XPSTR("\007DisableEnable "), value, InverseBlink ) ;
			}
		break ;
		case 27 :
			lcd_puts_Pleft( 1*FH, "Calibration, place\037the S6R as shown\037press MENU LONG" ) ;
			lcd_img( 12, 4*FH, calState<2 ? S6Rimg1 : calState<4 ? S6Rimg2 : S6Rimg3, calState & 1, 0 ) ;
			if ( calState == 1 )
			{
				lcd_puts_Pleft( 7*FH, "Inverted" ) ;
			}
			index = 0x9D ;
			value = calState ;
			if ( valid )
			{
				calState += 1 ;
				calState %= 6 ;
				valid = 0 ;
			}
		break ;
	}
	if ( sub != 27 )
	{
		if ( valid && sub )
		{
			if ( state == 0 )
			{
				if ( ( (index >= 0x82) && (index <= 0x84) ) || (index == 0x9A ) || ( index == 0x9B ) )
				{
					value = value == 255 ? 0 : value == 0 ? 1 : 2 ;
				}
				if ( event != EVT_KEY_FIRST(KEY_MENU) )
				{
#ifdef PAGE_NAVIGATION
					if ( event == EVT_KEY_FIRST(BTN_RE) )
					{
						killEvents( event ) ;
						Tevent = event = 0 ;
						if ( RotaryState != ROTARY_VALUE )
						{
							rotaryLocal = ROTARY_VALUE ;
						}
						else
						{
							rotaryLocal = ROTARY_MENU_UD ;
						}
					}
					RotaryState = rotaryLocal ;
					s_editMode = ( rotaryLocal == ROTARY_VALUE ) ;
#endif	// PAGE_NAVIGATION
					value = checkIncDec16( value, min, max, NO_MENU_ONLY_EDIT ) ;
				}
				if ( ( (index >= 0x82) && (index <= 0x84) ) || (index == 0x9A ) || ( index == 0x9B ) )
				{
					value = value == 0 ? 255 : value == 1 ? 0 : 0x80 ;
				}
			}
		}
		else
		{
			lcd_puts_Pleft( 3*FH, "\014-----" ) ;
		}
	}

//  lcd_outhex4( 100, 5*FH, S6Rdata.valid ) ;
//  lcd_outhex4( 100, 6*FH, S6Rdata.fieldIndex ) ;
//  lcd_outhex4( 100, 7*FH, S6Rdata.value ) ;
}
#endif //n SMALL

#ifndef SMALL
uint16_t DsmFrameRequired ;

void menuProcDsmDdiag(uint8_t event)
{
	
	MENU(XPSTR("DSM diag"), menuTabStat, e_dsm, 2, {0} ) ;
  
	int8_t sub = mstate2.m_posVert ;

  lcd_outhex4( 0, 2*FH, DsmFrameRequired ) ;
  if(sub==1)
  {
		lcd_char_inverse( 0, 2*FH, 20, 0 ) ;
		DsmFrameRequired = checkIncDec16( DsmFrameRequired, 0, 511, 0 ) ;
  }

  lcd_outhex4( 0, 1*FH, FrskyTelemetryType ) ;
  lcd_outhex4( 24, 1*FH, g_model.dsmMode ) ;

#ifdef PCB9XT

extern uint8_t numPktBytes ;
extern uint8_t frskyRxBuffer[];
  lcd_outhex4( 0, 3*FH, numPktBytes ) ;

	lcd_outhex4( 0 , 4*FH, (frskyRxBuffer[0]<< 8) | frskyRxBuffer[1] ) ;
	lcd_outhex4( 24, 4*FH, (frskyRxBuffer[2]<< 8) | frskyRxBuffer[3] ) ;
	lcd_outhex4( 48, 4*FH, (frskyRxBuffer[4]<< 8) | frskyRxBuffer[5] ) ;
	lcd_outhex4( 72, 4*FH, (frskyRxBuffer[6]<< 8) | frskyRxBuffer[7] ) ;

	lcd_outhex4( 0 , 5*FH, (frskyRxBuffer[8]<< 8) | frskyRxBuffer[9] ) ;
	lcd_outhex4( 24, 5*FH, (frskyRxBuffer[10]<< 8) | frskyRxBuffer[11] ) ;
	lcd_outhex4( 48, 5*FH, (frskyRxBuffer[12]<< 8) | frskyRxBuffer[13] ) ;
	lcd_outhex4( 72, 5*FH, (frskyRxBuffer[14]<< 8) | frskyRxBuffer[15] ) ;

//  lcd_outhex4( 0, 6*FH, USART2->CR1 ) ;
//  lcd_outhex4( 24, 6*FH, USART2->CR2 ) ;
//  lcd_outhex4( 48, 6*FH, USART2->BRR ) ;
extern uint16_t RxIntCount ;
	lcd_outhex4( 72, 6*FH, RxIntCount ) ;

extern uint16_t USART_ERRORS ;
extern uint16_t USART_FE ;
extern uint16_t USART_PE ;
extern uint16_t USART_ORE ;
extern uint16_t USART1_ORE ;
extern uint16_t USART2_ORE ;
extern uint16_t USART_NE ;
  lcd_outhex4( 0, 7*FH, USART_ERRORS ) ;
  lcd_outhex4( 24, 7*FH, USART_FE ) ;
  lcd_outhex4( 48, 7*FH, USART_PE ) ;
  lcd_outhex4( 72, 7*FH, USART_ORE ) ;
  lcd_outhex4( 96, 7*FH, USART_NE ) ;
  
	lcd_outhex4( 0, 6*FH, USART1_ORE ) ;
	lcd_outhex4( 24, 6*FH, USART2_ORE ) ;

extern uint16_t M64Overruns ;
  lcd_outhex4( 96, 3*FH, M64Overruns ) ;

  lcd_outhex4( 96, 2*FH, SCB->AIRCR ) ;

#endif

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
extern uint16_t USART_ERRORS ;
extern uint16_t USART_FE ;
extern uint16_t USART_PE ;
extern uint16_t USART_ORE ;
extern uint16_t USART_NE ;
  lcd_outhex4( 0, 7*FH, USART_ERRORS ) ;
  lcd_outhex4( 24, 7*FH, USART_FE ) ;
  lcd_outhex4( 48, 7*FH, USART_PE ) ;
  lcd_outhex4( 72, 7*FH, USART_ORE ) ;
  lcd_outhex4( 96, 7*FH, USART_NE ) ;
#endif

#if defined(PCBSKY) || defined(PCBLEM1)
extern uint8_t DsmDebug[20] ;
extern uint8_t DsmControlDebug[20] ;
//extern uint16_t DsmControlCounter ;

  lcd_outhex4( 0, 3*FH,  (DsmDebug[17] << 8) | DsmDebug[0] ) ;
  lcd_outhex4( 24, 3*FH, (DsmDebug[1]<< 8) | DsmDebug[2] ) ;
  lcd_outhex4( 48, 3*FH, (DsmDebug[3]<< 8) | DsmDebug[4] ) ;
  lcd_outhex4( 72, 3*FH, (DsmDebug[5]<< 8) | DsmDebug[6] ) ;
  lcd_outhex4( 96, 3*FH, (DsmDebug[7]<< 8) | DsmDebug[8] ) ;
  lcd_outhex4( 0, 4*FH, (DsmDebug[9]<< 8) | DsmDebug[10] ) ;
  lcd_outhex4( 24, 4*FH, (DsmDebug[11]<< 8) | DsmDebug[12] ) ;
  lcd_outhex4( 48, 4*FH, (DsmDebug[13]<< 8) | DsmDebug[14] ) ;
  lcd_outhex4( 72, 4*FH, (DsmDebug[15]<< 8) | DsmDebug[16] ) ;

  lcd_outhex4( 0, 6*FH,  (DsmControlDebug[17] << 8) | DsmControlDebug[0] ) ;
  lcd_outhex4( 24, 6*FH, (DsmControlDebug[1]<< 8) | DsmControlDebug[2] ) ;
  lcd_outhex4( 48, 6*FH, (DsmControlDebug[3]<< 8) | DsmControlDebug[4] ) ;
  lcd_outhex4( 72, 6*FH, (DsmControlDebug[5]<< 8) | DsmControlDebug[6] ) ;
  lcd_outhex4( 96, 6*FH, (DsmControlDebug[7]<< 8) | DsmControlDebug[8] ) ;
  lcd_outhex4( 72, 7*FH, (DsmControlDebug[9]<< 8) | DsmControlDebug[10] ) ;
  lcd_outhex4( 96, 7*FH, (DsmControlDebug[11]<< 8) | DsmControlDebug[12] ) ;
//  lcd_outhex4( 48, 7*FH, (DsmControlDebug[13]<< 8) | DsmControlDebug[14] ) ;
//  lcd_outhex4( 72, 7*FH, (DsmControlDebug[15]<< 8) | DsmControlDebug[16] ) ;
  
//	lcd_outhex4( 96, 7*FH, DsmControlCounter ) ;

#if defined(PCBSKY)
extern uint16_t USART_ERRORS ;
extern uint16_t USART_FE ;
extern uint16_t USART_PE ;
  lcd_outhex4( 0, 7*FH, USART_ERRORS ) ;
  lcd_outhex4( 24, 7*FH, USART_FE ) ;
  lcd_outhex4( 48, 7*FH, USART_PE ) ;
#endif
extern uint16_t DsmDbgCounters[20] ;
	lcd_outhex4( 24, 2*FH, DsmDbgCounters[6] ) ;

	lcd_outhex4( 0, 5*FH, DsmDbgCounters[10] ) ;
	lcd_outhex4( 24, 5*FH, DsmDbgCounters[11] ) ;
	lcd_outhex4( 48, 5*FH, DsmDbgCounters[12] ) ;
	lcd_outhex4( 72, 5*FH, DsmDbgCounters[13] ) ;

#endif
}
#endif


#if defined(PCBSKY) || defined(PCBLEM1)
struct t_i2cTime
{
	uint8_t setCode ;
	uint8_t Time[7] ;
} TimeToSet ;
#endif

void dispMonth( uint8_t x, uint8_t y, uint32_t month, uint8_t attr)
{
	if ( month > 12 )
	{
		month = 0 ;		
	}
  lcd_putsAttIdx( x, y, PSTR(STR_MONTHS),month, attr ) ;
}

void disp_datetime( uint8_t y )
{
	lcd_puts_Pleft( y, XPSTR("  -   -      :  :"));
	lcd_outdezNAtt( 19*FW-2, y, Time.second, LEADING0, 2 ) ;
	lcd_outdezNAtt( 16*FW-1, y, Time.minute, LEADING0, 2 ) ;
	lcd_outdezNAtt( 13*FW,   y, Time.hour, LEADING0, 2 ) ;
	lcd_outdezNAtt( 2*FW,    y, Time.date, LEADING0, 2 ) ;
	lcd_outdezNAtt( 10*FW,   y, Time.year, LEADING0, 4 ) ;
	dispMonth( 3*FW, y, Time.month, 0 ) ;
}

t_time EntryTime ;

#ifdef REVX
#define DATE_COUNT_ITEMS	8
#else
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
#define DATE_COUNT_ITEMS	8
#else
#define DATE_COUNT_ITEMS	7
#endif
#endif

void menuProcDate(uint8_t event)
{
	TITLE(PSTR(STR_DateTime));
	static MState2 mstate2;
	static const uint8_t mstate_tab[] = {0} ;
	
		event = mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1,DATE_COUNT_ITEMS-1) ;

#ifdef PCBLEM1
	if ( g_eeGeneral.externalRtcType == 0 )
	{
		lcd_puts_Pleft( 3*FH, XPSTR( "\005Not Present" ) ) ;
	}
#endif
  switch(event)
  {
    case EVT_ENTRY :
			{	
				t_time *p = &EntryTime ;

				if ( Time.date == 0 )
				{
					Time.date = 1 ;
				}
				if ( Time.month == 0 )
				{
					Time.month = 1 ;
				}
				p->second = Time.second ;
				p->minute = Time.minute ;
				p->hour   = Time.hour ;
				p->date   = Time.date ;
				p->month  = Time.month ;
				p->year   = Time.year ;
				if ( p->year < 2000 )
				{
					p->year = 2000 + ( p->year % 100 ) ;
					
				}
			}
    break;
		
    case EVT_KEY_LONG(KEY_MENU) :
#if defined(PCBSKY) || defined(PCBLEM1)
			TimeToSet.setCode = 0x74 ;		// Tiny SET TIME CODE command
			TimeToSet.Time[0] = EntryTime.second ;
			TimeToSet.Time[1] = EntryTime.minute ;
			TimeToSet.Time[2] = EntryTime.hour ;
			TimeToSet.Time[3] = EntryTime.date ;
			TimeToSet.Time[4] = EntryTime.month ;
			TimeToSet.Time[5] = (uint8_t) EntryTime.year ;
			TimeToSet.Time[6] = EntryTime.year >> 8 ;
#endif
#ifdef REVX
			writeRTC( (uint8_t *) &TimeToSet.Time[0] ) ;
#else
#ifdef PCBSKY
#ifndef ARUNI
			if ( g_eeGeneral.ar9xBoard == 0 )
			{
				write_coprocessor( (uint8_t *) &TimeToSet, 8 ) ;
			}
			else
#endif
			{
#ifndef SMALL
				writeExtRtc( (uint8_t *) &TimeToSet.Time[0] ) ;
#endif
			}
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
			rtcSetTime( &EntryTime ) ;
#endif
#endif
#ifdef PCBLEM1
			writeExtRtc( (uint8_t *) &TimeToSet.Time[0] ) ;
#endif
      killEvents(event);
			s_editMode = 0 ;
    break;
	}		 
//#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D)
//		disp_datetime( 1*FH ) ;
//#endif

#if defined(PCBSKY) || defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
 #ifndef REVA
#ifdef PCBLEM1
	if ( g_eeGeneral.externalRtcType == 1 )
#endif
	{
		disp_datetime( 1*FH ) ;
 	}
 #endif

//    lcd_outhex4( 17*FW+4, 6*FH, (Coproc_valid << 8 ) + Coproc_read ) ;
#ifndef REVX
#ifdef PCBSKY
#ifndef ARUNI
//extern uint8_t Co_proc_status[] ;
		if ( g_eeGeneral.ar9xBoard == 0 )
		{
//			lcd_outdezAtt( 21*FW, 7*FH, (uint8_t)Co_proc_status[8], 0 ) ;		// Co-Proc temperature

			uint8_t temp = ExtraInputs ;
  	  lcd_putc( 18*FW, 6*FH, temp & 0x20 ? '1' : '0' ) ;
    	lcd_putc( 19*FW, 6*FH, temp & 0x10 ? '1' : '0' ) ;
	    lcd_putc( 20*FW, 6*FH, temp & 0x08 ? '1' : '0' ) ;
		}
#endif
#endif
#endif
    int8_t  sub    = mstate2.m_posVert;

		for (uint8_t subN=1; subN<8; subN++)
		{
	  	uint16_t attr = ((sub==subN) ? InverseBlink : 0) | LEADING0 ;
			switch ( subN )
			{
				case 1 :
			  	lcd_puts_Pleft( 2*FH, PSTR(STR_SEC) );
					lcd_outdezNAtt( 7*FW, 2*FH, EntryTime.second, attr, 2 ) ;
			  	if(sub==subN)  EntryTime.second = checkIncDec( EntryTime.second, 0, 59, 0 ) ;
				break ;
				case 2 :
			  	lcd_puts_Pleft( 3*FH, PSTR(STR_MIN_SET) );
					lcd_outdezNAtt( 7*FW, 3*FH, EntryTime.minute, attr, 2 ) ;
			  	if(sub==subN)  EntryTime.minute = checkIncDec( EntryTime.minute, 0, 59, 0 ) ;
				break ;
				case 3 :
			  	lcd_puts_Pleft( 4*FH, PSTR(STR_HOUR_MENU_LONG) );
					lcd_outdezNAtt( 7*FW, 4*FH, EntryTime.hour, attr, 2 ) ;
			  	if(sub==subN)  EntryTime.hour = checkIncDec( EntryTime.hour, 0, 23, 0 ) ;
				break ;
				case 4 :
			  	lcd_puts_Pleft( 5*FH, PSTR(STR_DATE) );
					lcd_outdezNAtt( 7*FW, 5*FH, EntryTime.date, attr, 2 ) ;
			  	if(sub==subN)  EntryTime.date = checkIncDec( EntryTime.date, 1, 31, 0 ) ;
				break ;
				case 5 :
			  	lcd_puts_Pleft( 6*FH, PSTR(STR_MONTH) );
					dispMonth( 5*FW+3, 6*FH, EntryTime.month, attr ) ;
			  	if(sub==subN)  EntryTime.month = checkIncDec( EntryTime.month, 1, 12, 0 ) ;
				break ;
				case 6 :
//#ifndef REVX
//					if ( g_eeGeneral.ar9xBoard == 0 )
//					{
//						lcd_puts_Pleft( 7*FH, PSTR(STR_YEAR_TEMP) );
//					}
//					else
//					{
//					lcd_puts_Pleft( 7*FH, PSTR(STR_YEAR) );
//					}
//#else			  	
					lcd_puts_Pleft( 7*FH, PSTR(STR_YEAR) );
//#endif
					lcd_outdezNAtt( 9*FW-2, 7*FH, EntryTime.year, attr, 4 ) ;
			  	if(sub==subN)  EntryTime.year = checkIncDec16( EntryTime.year, 0, 2999, 0 ) ;
				break ;
#ifdef REVX
				case 7 :
					uint8_t previous = g_eeGeneral.rtcCal ;
			  	lcd_puts_P( 12*FW, 5*FH, PSTR(STR_CAL) );
					lcd_outdezAtt( 20*FW, 5*FH, g_eeGeneral.rtcCal, attr & ~LEADING0 ) ;
			  	if(sub==subN) CHECK_INCDEC_H_GENVAR( g_eeGeneral.rtcCal, -127, 127 ) ;
					if ( g_eeGeneral.rtcCal != previous )
					{
						previous = g_eeGeneral.rtcCal ;
						if ( g_eeGeneral.rtcCal < 0 )
						{
							previous = -g_eeGeneral.rtcCal | 0x80 ;
						}
						setRtcCAL( previous ) ;						
					}
				break ;
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
				case 7 :
					int8_t previous = g_eeGeneral.rtcCal ;
			  	lcd_puts_P( 12*FW, 5*FH, PSTR(STR_CAL) );
					lcd_outdezAtt( 20*FW, 5*FH, g_eeGeneral.rtcCal, attr & ~LEADING0 ) ;
			  	if(sub==subN) CHECK_INCDEC_H_GENVAR( g_eeGeneral.rtcCal, -31, 31 ) ;
					if ( g_eeGeneral.rtcCal != previous )
					{
						previous = g_eeGeneral.rtcCal ;
						uint32_t sign = 0 ;
						if ( g_eeGeneral.rtcCal < 0 )
						{
							sign = 0x80 ;
							previous = -g_eeGeneral.rtcCal ;
						}
						rtcSetCal( sign, previous ) ;						
					}
				break ;
#endif
			}
#ifdef PCB9XT
extern uint16_t VbattRtc ;
		  	lcd_puts_P( 11*FW, 7*FH, XPSTR("RTC V") );
				lcd_outdezAtt( 20*FW, 7*FH, VbattRtc*330/2048, PREC2 ) ;
#endif
#ifdef PCBX9D
extern uint16_t VbattRtc ;
		  	lcd_puts_P( 11*FW, 7*FH, XPSTR("RTC V") );
				lcd_outdezAtt( 20*FW, 7*FH, VbattRtc*300/2048, PREC2 ) ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
extern uint16_t VbattRtc ;
		  	lcd_puts_P( 11*FW, 7*FH, XPSTR("RTC V") );
				lcd_outdezAtt( 20*FW, 7*FH, VbattRtc*600/2048, PREC2 ) ;
#endif
		}
#endif
}

//extern DIR Dj ;
extern DIR Djp ;

void setupFileNames( TCHAR *dir, struct fileControl *fc, char *ext )
{
	FRESULT fr ;
	if ( !sd_card_ready() )
	{
		fc->nameCount = 0 ;
	}
	fr = f_chdir( dir ) ;
	if ( fr == FR_OK )
	{
		fc->index = 0 ;
		fr = f_opendir( ( VoiceFileType == VOICE_FILE_TYPE_MUSIC )? &Djp : &SharedMemory.FileList.Dj, (TCHAR *) "." ) ;
		if ( fr == FR_OK )
		{
			WatchdogTimeout = 300 ;		// 3 seconds
			fc->ext[0] = *ext++ ;
			fc->ext[1] = *ext++ ;
			fc->ext[2] = *ext++ ;
			fc->ext[3] = *ext ;
			fc->index = 0 ;
			fc->nameCount = fillNames( 0, fc ) ;
			fc->hpos = 0 ;
			fc->vpos = 0 ;
		}
	}
}


void menuProcRestore(uint8_t event)
{
//	FRESULT fr ;
	struct fileControl *fc = &FileControl ;
	uint32_t i ;

  TITLE( "RESTORE MODEL" ) ;

  switch(event)
	{
    case EVT_ENTRY:
			setupFileNames( (TCHAR *)"\\MODELS", fc, (char *)"EEPM" ) ;
    break ;
	}
	
	i = fileList( event, &FileControl ) ;
	if ( i == 1 )	// Select
	{
		// Restore the model
    Tevent = 0 ;
    lcd_clear();
    lcd_putsAtt(64-9*FW,0*FH,XPSTR("Restoring"),DBLSIZE);
    refreshDisplay();
		clearKeyEvents() ;

		TCHAR RestoreFilename[60] ;
		cpystr( cpystr( (uint8_t *)RestoreFilename, (uint8_t *)"/MODELS/" ), (uint8_t *) SharedMemory.FileList.Filenames[fc->vpos] ) ;
		WatchdogTimeout = 300 ;		// 3 seconds
		BackResult = ee32RestoreModel( RestoreIndex, RestoreFilename ) ;
		AlertType = MESS_TYPE ;
		AlertMessage = BackResult ;
    popMenu() ;
	}
	else if ( i == 2 )	// EXIT
	{
    killEvents(event) ;
    popMenu() ;
	}
}

void menuProcSelectVoiceFile(uint8_t event)
{
//	FRESULT fr ;
	struct fileControl *fc = &FileControl ;
	uint32_t i ;

  TITLE( "Select File" ) ;

#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x0300 ;
#else
	RTC->BKP1R = 0x0300 ;
#endif
#endif
#ifdef WHERE_TRACK
	notePosition('d') ;
#endif
		 
  switch(event)
	{
    case EVT_ENTRY:
		{
			TCHAR * directory ;
			i = 0 ;
			directory = (TCHAR *)"\\voice\\user" ;
			if ( VoiceFileType == VOICE_FILE_TYPE_NAME )
			{
				directory = (TCHAR *)"\\voice\\modelNames" ;
			}
			else if ( VoiceFileType == VOICE_FILE_TYPE_SYSTEM )
			{
				directory = (TCHAR *)"\\voice\\system" ;
			}
			else if ( VoiceFileType == VOICE_FILE_TYPE_MUSIC )
			{
				directory = (TCHAR *)"\\music" ;
				if ( g_eeGeneral.musicType )
				{
					i = 1 ;
				}
			}
				 
#ifdef WHERE_TRACK
	notePosition('e') ;
#endif
			setupFileNames( directory, fc, i ? (char *)"\0" : (char *)"WAV" ) ;
		}
    break ;
	}
	
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x0301 ;
#else
	RTC->BKP1R = 0x0301 ;
#endif
#endif
#ifdef WHERE_TRACK
	notePosition('f') ;
#endif
	i = fileList( event, &FileControl ) ;
#ifdef WHERE_TRACK
	notePosition('g') ;
#endif
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x0302 ;
#else
	RTC->BKP1R = 0x0302 ;
#endif
#endif
	if ( i == 1 )	// Select
	{
		cpystr( (uint8_t *)SelectedVoiceFileName, (uint8_t *) SharedMemory.FileList.Filenames[fc->vpos] ) ; 
		killEvents(event) ;
    popMenu() ;
	}
	else if ( i == 2 )	// EXIT
	{
    killEvents(event) ;
    popMenu() ;
	}
	else if ( i == 3 )	// Tag
	{
		if ( VoiceFileType != VOICE_FILE_TYPE_MUSIC )
		{		
			char name[12] ;
  	  killEvents(event) ;
			copyFileName( name, SharedMemory.FileList.Filenames[fc->vpos], 8 ) ;
			putNamedVoiceQueue( name, (VoiceFileType == VOICE_FILE_TYPE_NAME) ? VLOC_MNAMES : VLOC_USER ) ;
		}
		else
		{
			cpystr( (uint8_t *)SelectedVoiceFileName, (uint8_t *)SharedMemory.FileList.Filenames[fc->vpos] ) ;
			killEvents(event) ;
    	popMenu() ;
			i = 1 ;
		}
	}
	else if ( i == 0 )	// No Files
	{
		if ( checkForExitEncoderLong( event ) )
		{
			killEvents(event) ;
  	  popMenu(false) ;
		}
	}
	FileSelectResult = i ;
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x0303 ;
#else
	RTC->BKP1R = 0x0303 ;
#endif
#endif
#ifdef WHERE_TRACK
	notePosition('h') ;
#endif
}


#if defined(PCBX9D) || defined(IMAGE_128) || defined(PCBX12D) || defined(PCBX10)

void menuProcSelectImageFile(uint8_t event)
{
	struct fileControl *fc = &FileControl ;
	uint32_t i ;

  TITLE( "SELECT FILE" ) ;

  switch(event)
	{
    case EVT_ENTRY:
			setupFileNames( (TCHAR *)"\\IMAGES", fc, (char *)"BMP" ) ;
    break ;
	}
	
	i = fileList( event, &FileControl ) ;
	if ( i == 1 )	// Select
	{
		// Restore the model
		cpystr( (uint8_t *)SelectedVoiceFileName, (uint8_t *)SharedMemory.FileList.Filenames[fc->vpos] ) ;
		killEvents(event) ;
    popMenu() ;
	}
	else if ( i == 2 )	// EXIT
	{
    killEvents(event) ;
    popMenu() ;
	}
	FileSelectResult = i ;
}
#endif


#ifdef NEW_VARIO

struct t_newvario NewVario ;

void menuNewVario(uint8_t event)
{
	EditType = 0 ;
	struct t_newvario *p = &NewVario ;

	MENU(XPSTR("Vario"), menuTabStat, e_vario, 7, {0} ) ;

//	if ( event == EVT_ENTRY )
//	{
//		struct t_vario *p = &NewVario ;
//		if ( p->defaulted == 0 )
//		{
			
//		}
		
//	}
	 
  lcd_puts_Pleft( 1*FH, PSTR(STR_VARIO_SRC) ) ;
	p->varioSource = checkIndexed( 1*FH, XPSTR(FWx15"\001""\004vspdA2  "), p->varioSource, (mstate2.m_posVert == 1) ) ;

  lcd_puts_Pleft( 2*FH, XPSTR("Centre Min.") ) ;
  lcd_outdezAtt( PARAM_OFS, 2*FH, -5 + p->varioCenterMin, ((mstate2.m_posVert == 2) ? INVERS : 0)|PREC1) ;
	if (mstate2.m_posVert == 2)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioCenterMin, -16, 5+min(10, p->varioCenterMax+5)) ;
	}

  lcd_puts_Pleft( 3*FH, XPSTR("Centre Max.") ) ;
  lcd_outdezAtt( PARAM_OFS, 3*FH, 5 + p->varioCenterMax, ((mstate2.m_posVert == 3) ? INVERS : 0)|PREC1) ;
	if (mstate2.m_posVert == 3)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioCenterMax, 5+max(-10, p->varioCenterMin-5), 15) ;
	}

//            case 2:
//              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.frsky.varioCenterSilent, 1);
//              break;
//          }

  lcd_puts_Pleft( 4*FH, XPSTR("Vario Min.") ) ;
  lcd_outdezAtt( PARAM_OFS, 4*FH, p->varioMin-10, ((mstate2.m_posVert == 4) ? INVERS : 0)) ;
	if (mstate2.m_posVert == 4)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioMin, -7, 7 ) ;
	}

  lcd_puts_Pleft( 5*FH, XPSTR("Vario Max.") ) ;
  lcd_outdezAtt( PARAM_OFS, 5*FH, p->varioMax+10, ((mstate2.m_posVert == 5) ? INVERS : 0)) ;
	if (mstate2.m_posVert == 5)
	{
		CHECK_INCDEC_H_MODELVAR( p->varioMax, -7, 7 ) ;
	}

	lcd_puts_Pleft( 6*FH, PSTR(STR_2SWITCH) ) ;
	p->swtch = edit_dr_switch( PARAM_OFS, 6*FH, p->swtch, ((mstate2.m_posVert == 6) ? INVERS : 0), (mstate2.m_posVert == 6) ? EDIT_DR_SWITCH_EDIT : 0, event ) ;

//	int16_t varioCenterMin ;
//	int16_t varioCenterMax ;
//	int16_t varioMax ;
//	int16_t varioMin ;
//	int16_t varioPitch ;
//	uint16_t defaulted ;


}
#endif


void menuProcBattery(uint8_t event)
{
	MENU(PSTR(STR_BATTERY), menuTabStat, e_battery, 1, {0} ) ;

  switch(event)
  {
  	case EVT_KEY_BREAK(KEY_MENU):
      g_timeMain = 0;
#ifdef PCBSKY
			Current_max = 0 ;
#endif
    break;
    case EVT_KEY_LONG(KEY_MENU):
#ifdef PCBSKY
			MAh_used = 0 ;
			Current_used = 0 ;
#endif
      audioDefevent(AU_MENUS) ;
    	killEvents(event) ;
    break;
  	
//		case EVT_ENTRY :
//extern void convertFont() ;
//		 convertFont() ;
//    break;

  }

		lcd_puts_Pleft( 2*FH, PSTR(STR_Battery));
		putsVolts( 13*FW, 2*FH, g_vbat100mV, 0 ) ;

#ifdef PCBSKY
#ifndef REVA
		if ( ( g_eeGeneral.ar9xBoard == 0 ) && ( g_eeGeneral.extraPotsSource[0] != 2 ) && ( g_eeGeneral.extraPotsSource[1] != 2 ) )
		{
			Current_sum += Current_current ;
			if ( ++Current_count > 49 )
			{
				Current = Current_sum / 500 ;
				Current_sum = 0 ;
				Current_count = 0 ;
			}
			lcd_puts_Pleft( 3*FH, PSTR(STR_CURRENT_MAX));
	  	lcd_outdezAtt( 13*FW, 3*FH, Current, 0 ) ;
	  	lcd_outdezAtt( 20*FW, 3*FH, Current_max/10, 0 ) ;
			lcd_puts_Pleft( 4*FH, XPSTR("mAh\017[MENU]"));
	  	lcd_outdezAtt( 12*FW, 4*FH, MAh_used + Current_used/3600 ,PREC1 ) ;
		}
		lcd_puts_Pleft( 6*FH, PSTR(STR_CPU_TEMP_MAX));
	  lcd_outdezAtt( 12*FW-2, 6*FH, (((((int32_t)Temperature - 838 ) * 621 ) >> 11 ) - 20) ,0 ) ;
	  lcd_outdezAtt( 20*FW-2, 6*FH, (((((int32_t)Max_temperature - 838 ) * 621 ) >> 11 ) - 20) ,0 ) ;
#endif // REVA

		disp_datetime( 5*FH ) ;
#endif // PCBSKY

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	disp_datetime( 5*FH ) ;
#endif

#ifndef SMALL
extern uint32_t Master_frequency ;
 	lcd_outdezAtt( 5*FW, 7*FH, Master_frequency/1000000, 0 ) ;
#endif
}



//#ifdef JETI
#ifdef REVX

void menuProcJeti(uint8_t event)
{
    TITLE(XPSTR("JETI"));

    switch(event)
    {
    case EVT_KEY_FIRST(KEY_EXIT):
        chainMenu(menuProc0);
        break;
    }

    for (uint8_t i = 0; i < 16; i++)
    {
        lcd_putcAtt((i+2)*FW, 1*FH, JetiBuffer[i], 0 ) ;
        lcd_putcAtt((i+2)*FW, 2*FH, JetiBuffer[i+16], 0 ) ;
    }

extern struct t_16bit_fifo32 Jeti_fifo ;

	lcd_outhex4( 50,  0*FH, JetiIndex ) ;

	lcd_outhex4( 0,  3*FH, Jeti_fifo.fifo[0] ) ;
	lcd_outhex4( 25, 3*FH, Jeti_fifo.fifo[1] ) ;
	lcd_outhex4( 50, 3*FH, Jeti_fifo.fifo[2] ) ;
	lcd_outhex4( 75, 3*FH, Jeti_fifo.fifo[3] ) ;
	lcd_outhex4( 100,3*FH, Jeti_fifo.fifo[4] ) ;

	lcd_outhex4( 0,  4*FH, Jeti_fifo.fifo[5] ) ;
	lcd_outhex4( 25, 4*FH, Jeti_fifo.fifo[6] ) ;
	lcd_outhex4( 50, 4*FH, Jeti_fifo.fifo[7] ) ;
	lcd_outhex4( 75, 4*FH, Jeti_fifo.fifo[8] ) ;
	lcd_outhex4( 100,4*FH, Jeti_fifo.fifo[9] ) ;

	lcd_outhex4( 0,  5*FH, Jeti_fifo.fifo[10] ) ;
	lcd_outhex4( 25, 5*FH, Jeti_fifo.fifo[11] ) ;
	lcd_outhex4( 50, 5*FH, Jeti_fifo.fifo[12] ) ;
	lcd_outhex4( 75, 5*FH, Jeti_fifo.fifo[13] ) ;
	lcd_outhex4( 100,5*FH, Jeti_fifo.fifo[14] ) ;

	lcd_outhex4( 0,  6*FH, Jeti_fifo.fifo[15] ) ;
	lcd_outhex4( 25, 6*FH, Jeti_fifo.fifo[16] ) ;
	lcd_outhex4( 50, 6*FH, Jeti_fifo.fifo[17] ) ;
	lcd_outhex4( 75, 6*FH, Jeti_fifo.fifo[18] ) ;
	lcd_outhex4( 100,6*FH, Jeti_fifo.fifo[19] ) ;

	lcd_outhex4( 0,  7*FH, Jeti_fifo.fifo[20] ) ;
	lcd_outhex4( 25, 7*FH, Jeti_fifo.fifo[21] ) ;
	lcd_outhex4( 50, 7*FH, Jeti_fifo.fifo[22] ) ;
	lcd_outhex4( 75, 7*FH, Jeti_fifo.fifo[23] ) ;
	lcd_outhex4( 100,7*FH, Jeti_fifo.fifo[24] ) ;

    switch(event)
		{
    	case EVT_KEY_FIRST(KEY_UP):
				jeti_keys &= JETI_KEY_UP ;
			break ;

    	case EVT_KEY_FIRST(KEY_DOWN):
				jeti_keys &= JETI_KEY_DOWN ;
			break ;
    	
			case EVT_KEY_FIRST(KEY_LEFT):
				jeti_keys &= JETI_KEY_LEFT ;
			break ;
    	
			case EVT_KEY_FIRST(KEY_RIGHT):
				jeti_keys &= JETI_KEY_RIGHT ;
			break ;
		}
        JetiBufferReady = 0;    // invalidate buffer
				
				if ( jeti_keys != JETI_KEY_NOCHANGE )
				{
					JetiIndex = 0 ;
					jetiSendWord( jeti_keys ) ;
				}


        jeti_keys = JETI_KEY_NOCHANGE ;
}
#endif

void menuProcStatistic(uint8_t event)
{
	MENU(PSTR(STR_STAT), menuTabStat, e_stat1, 1, {0} ) ;

  lcd_puts_Pleft( FH*0, XPSTR("\016TOT\037\001TME\016TSW\037\001STK\016ST%"));

  putsTime(    6*FW, FH*1, s_timer[0].s_timeCumAbs, 0, 0);
  putsTime(   11*FW, FH*1, s_timer[0].s_timeCumSw,      0, 0);

  putsTime(    6*FW, FH*2, s_timer[0].s_timeCumThr, 0, 0);
  putsTime(   11*FW, FH*2, s_timer[0].s_timeCum16ThrP/16, 0, 0);

  putsTime(   11*FW, FH*0, s_timeCumTot, 0, 0);

  uint16_t traceRd = s_traceCnt>MAXTRACE ? s_traceWr : 0;
  uint8_t x=5;
  uint8_t y=60;
  
#if defined(PCBX12D) || defined(PCBX10)
	pushPlotType( PLOT_BLACK ) ;
#endif
	lcd_hline(x-3,y,120+3+3);
  lcd_vline(x,y-32,32+3);

  for(uint8_t i=0; i<120; i+=6)
  {
    lcd_vline(x+i+6,y-1,3);
  }
  for(uint8_t i=1; i<=120; i++)
  {
    lcd_vline(x+i,y-s_traceBuf[traceRd],s_traceBuf[traceRd]);
    traceRd++;
    if(traceRd>=MAXTRACE) traceRd=0;
    if(traceRd==s_traceWr) break;
  }
#if defined(PCBX12D) || defined(PCBX10)
		popPlotType() ;
#endif
}

void resetTimern( uint32_t timer )
{
  struct t_timer *tptr = &s_timer[timer] ;
	tptr->s_timerState = TMR_OFF; //is changed to RUNNING dep from mode
  tptr->s_timeCumThr=0;
  tptr->s_timeCumSw=0;
  tptr->s_timeCum16ThrP=0;
	tptr->s_sum = 0 ;
	tptr->last_tmr = g_model.timer[timer].tmrVal ;
	tptr->s_timerVal = ( g_model.timer[timer].tmrDir ) ? 0 : tptr->last_tmr ;
  tptr->s_timeCumAbs = 0 ;
}

//void resetTimer1()
//{
//	resetTimern( 0 ) ;
//}

//void resetTimer2()
//{
//	resetTimern( 1 ) ;
//}

void resetTimers()
{
	resetTimern(0) ;
	resetTimern(1) ;
//	resetTimer1() ;
//	resetTimer2() ;
}

int16_t AltOffset = 0 ;

void displayTemp( uint8_t sensor, uint8_t x, uint8_t y, uint8_t size )
{
	if ( ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) ) && ( sensor == 1 ) )
	{
		putsTelemetryChannel( x, y, (int8_t)sensor+TEL_ITEM_T1-1, FrskyHubData[FR_TEMP1+sensor-1], size | LEFT, TELEM_ARDUX_NAME ) ;
		
	}
	else
	{
		putsTelemetryChannel( x, y, (int8_t)sensor+TEL_ITEM_T1-1, FrskyHubData[FR_TEMP1+sensor-1], size | LEFT, 
																( size & DBLSIZE ) ? (TELEM_LABEL | TELEM_UNIT_LEFT) : (TELEM_LABEL | TELEM_UNIT) ) ;
	}
}


static int8_t io_subview = 0 ;

void switchDisplay( uint8_t j, uint8_t a )
{
#ifdef PCBXLITE
	if ( a >= 24 )
	{
		a -= 2 ;
	}
#endif	
#ifdef PCBX9LITE
	if ( a >= 24 )
	{
		a -= 1 ;
	}
#endif	
#ifdef PCBLEM1
	if ( a >= 24 )
	{
		a -= 2 ;
	}
#endif // PCBLEM1
	uint8_t b = a + 3 ;
	uint8_t y = 4*FH ;
#if defined(PCBSKY) || defined(PCB9XT)
#ifdef ARUNI
  uint8_t x1 = (2+j*15)*FW-2 ;
  uint8_t x2 = (2+j*15)*FW ;
#else
  uint8_t x1 = (2+j*15)*FW-2 ;
  uint8_t x2 = (2+j*15)*FW-2 ;
#endif
	for(uint8_t i=a; i<b; y += FH, i += 1 )
	{
		if ( i == 0 )	// THR
		{
			if ( g_eeGeneral.switchMapping & USE_THR_3POS )
			{
				uint8_t k = HSW_Thr3pos0 ;
				k += switchPosition(k) ;
				putSwitchName(x1, y, k-HSW_OFFSET, 0) ;
				continue ;
			}
		}
		if ( i == 2 )	// ELE
		{
			if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
			{
				uint8_t k = HSW_Ele3pos0 ;
				k += switchPosition(k) ;
				putSwitchName(x1, y, k-HSW_OFFSET, 0) ;
				continue ;
			}
			if ( g_eeGeneral.switchMapping & USE_ELE_6POS )
			{
				uint8_t k = HSW_Ele6pos0 ;
				k += switchPosition(k) ;
				putSwitchName(x1, y, k-HSW_OFFSET, 0) ;
				continue ;
			}
		}
		if ( i == 1 )	// RUD
		{
			if ( g_eeGeneral.switchMapping & USE_RUD_3POS )
			{
				uint8_t k = HSW_Rud3pos0 ;
				k += switchPosition(k) ;
				putSwitchName(x1, y, k-HSW_OFFSET, 0) ;
				continue ;
			}
		}
		if ( i == 6 )	// AIL
		{
			if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
			{
				uint8_t k = HSW_Ail3pos0 ;
				k += switchPosition(k) ;
				putSwitchName(x2, y, k-HSW_OFFSET, 0) ;
				continue ;
			}
		}
		if ( i == 7 )	// GEA
		{
			if ( g_eeGeneral.switchMapping & USE_GEA_3POS )
			{
				uint8_t k = HSW_Gear3pos0 ;
				k += switchPosition(k) ;
				putSwitchName(x2, y, k-HSW_OFFSET, 0) ;
				continue ;
			}
		}
		if ( i == 8 )	// TRN
		{
			uint8_t k = HSW_ID0 ;
			k += switchPosition(k) ;
			putSwitchName(x2, y, k-1, getSwitch00(i+1) ? INVERS : 0 ) ;
			continue ;
		}
		putSwitchName(x2, y, i, getSwitch00(i+1) ? INVERS : 0) ;
	}
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	for(uint8_t i=a; i<b; y += FH, i += 1 )
	{
#ifdef PCBX7
		uint32_t p ;
		p = i ;
		if ( p > 3 )
		{
			p += 1 ;
		}
		if ( p > 5 )
		{
			p += 1 ;
		}
#endif // PCBX7
#ifdef PCBXLITE
		uint32_t p ;
		p = i ;
		if ( p > 3 )
		{
			p += 4 ;
		}
#endif // PCBXLITE
#ifdef PCBX9LITE
		uint32_t p ;
		p = i ;
		if ( p > 2 )
		{
			p += 2 ;
		}
		if ( p > 5 )
		{
			p += 1 ;
		}
#endif // PCBX9LITE
#ifdef PCBLEM1
		uint32_t p ;
		p = i ;
		p = i ;
		if ( p > 3 )
		{
			p += 2 ;
		}
#endif // PCBLEM1
#ifdef REV9E
		if ( i < 18 )
#else
#if defined(PCBX7) || defined (PCBXLITE) || defined(PCBX9LITE) || defined(PCBLEM1)
		if ( p < 8 )
#else // PCBX7
		if ( i < 8 )
#endif // PCBX7
#endif	// REV9E
		{
#if defined(PCBX7) || defined (PCBXLITE) || defined(PCBX9LITE) || defined(PCBLEM1)
			putHwSwitchName((2+j*15)*FW-2, y, p, 0 ) ;
			uint32_t pos = switchPosition( p ) ;
#else // PCBX7
#if defined(PCBX12D) || defined(PCBX10)
			putHwSwitchName((0+j*(19+4))*FW-2 + X12SMALL_OFFSET, y, i, 0 ) ;
#else
			putHwSwitchName((2+j*15)*FW-2, y, i, 0 ) ;
#endif
			uint32_t pos = switchPosition( i ) ;
#endif // PCBX7
#if defined(PCBX12D) || defined(PCBX10)
			lcd_putc((2+j*(19+4))*FW-2 + X12SMALL_OFFSET, y, PSTR(HW_SWITCHARROW_STR)[pos] ) ;
#else
			lcd_putc((4+j*15)*FW-2, y, PSTR(HW_SWITCHARROW_STR)[pos] ) ;
#endif
		}
		else
		{
			uint32_t m = 0 ;
#ifndef PCBX12D
 #ifndef PCBX10
			if ( g_eeGeneral.analogMapping & MASK_6POS )
 #endif
#endif
			{
				m = 1 ;
#ifdef REV9E
				if ( i == 18 )
#else
#if defined(PCBX7) || defined (PCBXLITE) || defined(PCBX9LITE) || defined(PCBLEM1)
				if ( p == 8 )
#else // PCBX7
				if ( i == 8 )
#endif // PCBX7
#endif	// REV9E
				{
					uint8_t k = HSW_Ele6pos0 ;
					k += switchPosition(k) ;
#if defined(PCBX12D) || defined(PCBX10)
					putSwitchName((0+j*(19+4))*FW-2 + X12SMALL_OFFSET, y, k-HSW_OFFSET, 0) ;
#else
					putSwitchName((2+j*15)*FW-2, y, k-HSW_OFFSET, 0) ;
#endif
					continue ;
				}
			}

#ifdef REV9E
			putSwitchName((2+j*15)*FW-2, y, i-10+1-m, getSwitch00(i-10+2-m) ? INVERS : 0 ) ;
#else
#if defined(PCBX7) || defined (PCBXLITE) || defined(PCBX9LITE) || defined(PCBLEM1)
			putSwitchName((2+j*15)*FW-2, y, p+1-m, getSwitch00(p+2-m) ? INVERS : 0 ) ;
#else // PCBX7
#if defined(PCBX12D) || defined(PCBX10)
			putSwitchName((0+j*(19+4))*FW-2 + X12SMALL_OFFSET, y, i+1-m, getSwitch00(i+2-m) ? INVERS : 0 ) ;
#else
			putSwitchName((2+j*15)*FW-2, y, i+1-m, getSwitch00(i+2-m) ? INVERS : 0 ) ;
#endif
#endif // PCBX7
#endif	// REV9E
		}
	}
#endif
}


#define NUM_HELP_PAGES	2
const uint8_t HelpText0[] = "Enable the Hardware\nand EEPROM\nMenus: Power on\nholding the left\nhorizontal trim left\n\nMore->" ;
const uint8_t HelpText1[] = "Access Bootloader by\nPower on holding\nboth horizontal trims\nto the centre.\n\n"
														"MUTE: Power on with\nMENU & EXIT\nboth pressed." ;

void menuProcText(uint8_t event)
{
//	uint8_t filename[50] ;
	FRESULT result ;
//	UINT nread ;

	if ( event == EVT_ENTRY )
	{
	 if ( SharedMemory.TextControl.TextHelp )
	 {
	 		SharedMemory.TextControl.HelpTextPage = 0 ;
			uint8_t *p = (uint8_t *) HelpText0 ;
			setTextBuffer( p, strlen( ( char *)p ) ) ;
	 }
	 else
	 {
 		result = readModelNotes() ;
//		setModelFilename( filename, g_eeGeneral.currModel+1, FILE_TYPE_TEXT ) ;
//  	result = f_open( &SharedMemory.TextControl.TextFile, (TCHAR *)filename, FA_READ ) ;
//		SharedMemory.TextControl.TextLines = 0 ;
//		if ( result == FR_OK )
//		{
//			result = f_read( &SharedMemory.TextControl.TextFile, SharedMemory.TextControl.TextMenuStore, 16*21+32, &nread ) ;
//			if ( result == FR_OK )
//			{
//				setTextBuffer( SharedMemory.TextControl.TextMenuStore, nread ) ;
//			}
//		}
//		f_close( &SharedMemory.TextControl.TextFile ) ;
		if ( result != FR_OK )
		{
		 	memset( SharedMemory.TextControl.TextMenuBuffer,' ', 16*21 ) ;
  		strncpy_P( (char *)&SharedMemory.TextControl.TextMenuBuffer[63+3], XPSTR("No Notes Found"), 14 );
			SharedMemory.TextControl.TextLines = 8 ;
		}
	 }
		SharedMemory.TextControl.TextOffset = 0 ;
	}
	if ( checkForExitEncoderLong( event ) )
	{
		killEvents(event) ;
    popMenu(false) ;
	}
	if ( ( event == EVT_KEY_FIRST(KEY_UP) ) || ( event== EVT_KEY_REPT(KEY_UP) ) )
	{
		if ( SharedMemory.TextControl.TextOffset )
		{
			SharedMemory.TextControl.TextOffset -= 1 ;
		}
	}
	if ( ( event == EVT_KEY_FIRST(KEY_DOWN) ) || ( event== EVT_KEY_REPT(KEY_DOWN) ) )
	{
		if ( SharedMemory.TextControl.TextLines > 8 )
		{
			if ( SharedMemory.TextControl.TextOffset < SharedMemory.TextControl.TextLines - 8 )
			{
				SharedMemory.TextControl.TextOffset += 1 ;
			}
		}
	}
	if ( SharedMemory.TextControl.TextHelp )
	{
		if ( ( event == EVT_KEY_FIRST(KEY_RIGHT) ) || ( event== EVT_KEY_REPT(KEY_RIGHT) ) )
		{
			SharedMemory.TextControl.HelpTextPage += 1 ;
			if ( SharedMemory.TextControl.HelpTextPage >= NUM_HELP_PAGES )
			{
				SharedMemory.TextControl.HelpTextPage = 0 ;
			}
//	 		memset( SharedMemory.TextControl.TextMenuBuffer,' ', 16*21 ) ;
			uint8_t *p = (uint8_t *) ( SharedMemory.TextControl.HelpTextPage ? HelpText1 : HelpText0 ) ;
			setTextBuffer( p, strlen( ( char *)p ) ) ;
		}
		if ( ( event == EVT_KEY_FIRST(KEY_LEFT) ) || ( event== EVT_KEY_REPT(KEY_LEFT) ) )
		{
			if ( SharedMemory.TextControl.HelpTextPage == 0 )
			{
				SharedMemory.TextControl.HelpTextPage = NUM_HELP_PAGES - 1 ;
			}
			else
			{
				SharedMemory.TextControl.HelpTextPage -= 1 ;
			}
//	 		memset( SharedMemory.TextControl.TextMenuBuffer,' ', 16*21 ) ;
			uint8_t *p = (uint8_t *) ( SharedMemory.TextControl.HelpTextPage ? HelpText1 : HelpText0 ) ;
			setTextBuffer( p, strlen( ( char *)p ) ) ;
		}
	}
	// display buffer
	displayNotes( 0, 8 ) ;

//	uint32_t i ;
//	uint32_t j ;
//	uint8_t y = 0 ;
//	for ( i = 0 ; i < 8*21 ; y += FH, i += 21 )
//	{
//		j = SharedMemory.TextControl.TextOffset * 21 + i ;
//		lcd_putsnAtt( 0, y, (const char *)&SharedMemory.TextControl.TextMenuBuffer[j], 21, 0 ) ;
//	}
}


void displayTrims()
//void displayTrims( uint32_t small )
{
 	uint32_t i ;
#if defined(PCBX12D) || defined(PCBX10)
	pushPlotType( PLOT_BLACK ) ;
#endif
  for( i=0 ; i<4 ; i++ )
  {
#if defined(PCBX12D) || defined(PCBX10)
#define TL (27+16)
#define TDIV 3
#else
#define TL 27
#define TDIV 4
#endif
  	//                        LH LV RV RH
#if defined(PCBX12D) || defined(PCBX10)
  	static uint8_t x[4]    = {128*1/4+2 + X12SMALL_OFFSET-16, 4 , 239-4, 128*3/4-2+24 + X12SMALL_OFFSET+16};
#else
  	static uint8_t x[4]    = {128*1/4+2, 4, 128-4, 128*3/4-2};
#endif
  	register uint8_t xm, ym ;
		xm = modeFixValue( i ) ;
    xm = x[xm-1] ;

		register int16_t valt = getTrimValue( CurrentPhase, i ) ;
		uint8_t centre = (valt == 0) ;
    int8_t val = max((int8_t)-(TL+1),min((int8_t)(TL+1),(int8_t)(valt/TDIV)));
    if( (i == 1) || ( i == 2 ))
		{
#if defined(PCBX12D) || defined(PCBX10)
  	  ym=31 + 16 + 4 + 4 + 4 ;
#else
  	  ym=31;
#endif
  	  lcd_vline(xm,   ym-TL, TL*2);

      if((i == 1) || !(g_model.thrTrim))
			{
        lcd_vline(xm-1, ym-1,  3);
        lcd_vline(xm+1, ym-1,  3);
 	    }
 	    ym -= val;
  	}
		else
		{
#if defined(PCBX12D) || defined(PCBX10)
  	  ym=59 + 32 + 8 + 8 + 8 ;
#else
  	  ym=59;
#endif
  	  lcd_hline(xm-TL,ym,    TL*2);
  	  lcd_hline(xm-1, ym-1,  3);
  	  lcd_hline(xm-1, ym+1,  3);
  	  xm += val;
  	}
  	DO_RSQUARE(xm,ym,7) ;
		if ( centre )
		{
#if defined(PCBX12D) || defined(PCBX10)
			pushPlotType( PLOT_WHITE ) ;
      DO_SQUARE(xm,ym,5) ;
			popPlotType() ;
#else
			plotType = PLOT_BLACK ;
      DO_SQUARE(xm,ym,5) ;
			plotType = PLOT_XOR ;
#endif
		}
	}
#if defined(PCBX12D) || defined(PCBX10)
		popPlotType() ;
#endif
}


#define MAIN_POPUP_ENABLED	1

void navigateCustomTelemetry(uint8_t event, uint32_t mode )
{
  uint8_t view = g_model.mview & 0xf;
  uint8_t tview = g_model.mview & 0x70 ;
	uint32_t changed = 0 ;
	if ( view == e_telemetry )
	{
		if ( ( mode == 0 ) || ( tview <= 0x10 ) )
		{
			// A custom telemetry screen
			switch ( event )
			{
#ifdef PAGE_NAVIGATION
		    case EVT_KEY_BREAK(KEY_MENU) :			// ***************
#endif	// PAGE_NAVIGATION
  	  	case EVT_KEY_BREAK(KEY_RIGHT) :
				{
					uint32_t t_limit = 0x40 ;
					tview += 0x10 ;
					if ( ( FrskyTelemetryType == FRSKY_TEL_DSM ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) )
					{
						t_limit = 0x50 ;
					}
					if ( tview > t_limit )
					{
						tview = 0 ;
					}
					changed = 1 ;

        }
				break ;
	    	case EVT_KEY_BREAK(KEY_LEFT):
				{
					tview -= 0x10 ;
					uint32_t t_limit = 0x40 ;
					if ( ( FrskyTelemetryType == FRSKY_TEL_DSM ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) )
					{
						t_limit = 0x50 ;
					}
					if ( tview > 0x60 )
					{
						tview = t_limit ;
					}
					changed = 1 ;
        }
				break ;
			}
			if ( changed )
			{
				killEvents(event) ;
        g_model.mview = e_telemetry | tview ;
				eeModelChanged() ;
        audioDefevent(AU_MENUS);
			}
		}
	}
}


void actionMainPopup( uint8_t event )
{
	uint16_t mask = 0x143F ;

  if(PopupData.PopupActive == 1)
	{
		mask = 0x23C0 ;
	}
  else if(PopupData.PopupActive == 3)
	{
		mask = 0x183F ;
	}
		
	uint8_t popaction = doPopup( PSTR( STR_MAIN_POPUP ), mask, 16, event ) ;
																 
	UseLastSubmenuIndex = 0 ;
  if ( popaction == POPUP_SELECT )
	{
		uint8_t popidx = PopupData.PopupSel ;
		if ( popidx == 0 )	// Model Select
		{
      pushMenu(menuProcModelSelect) ;
		}
		else if( popidx == 1 )	// Edit Model
		{
			RotaryState = ROTARY_MENU_UD ;
	  	pushMenu(menuProcModelIndex) ;
		}
		else if( popidx == 2 )	// Last Menu
		{
			UseLastSubmenuIndex = 1 ;
      pushMenu(lastPopMenu());
		}
		else if ( popidx == 3 )	// Radio Setup
		{
      pushMenu(menuProcIndex) ;
		}
		else if( popidx == 4 )	// Statistics
		{
			RotaryState = ROTARY_MENU_LR ;
	  	pushMenu(menuProcBattery) ;
		}
		else if( popidx == 5 )	// Notes
		{
			SharedMemory.TextControl.TextHelp = 0 ;
	  	pushMenu(menuProcText) ;
		}
		else if( popidx == 6 )	// Zero Alt.
		{
			resetTelemetry( TEL_ITEM_RESET_ALT ) ;
		}
		else if( popidx == 7 )	// A1 Offset
		{
      if ( g_model.frsky.channels[0].units == 3 )		// Current (A)
			{
				resetTelemetry( TEL_ITEM_RESET_A1OFF ) ;
			}
		}
		else if( popidx == 8 )	// A2 Offset
		{
      if ( g_model.frsky.channels[1].units == 3 )		// Current (A)
			{
				resetTelemetry( TEL_ITEM_RESET_A2OFF ) ;
			}
		}
		else if( popidx == 9 )	// GPS reset
		{
				resetTelemetry( TEL_ITEM_RESET_GPS ) ;
		}
		else if( popidx == 10 )	// Help
		{
			SharedMemory.TextControl.TextHelp = 1 ;
	  	pushMenu(menuProcText) ;
		}
		else if( popidx == 11 )	// Main Display
		{
			g_model.mview = 0 ;
		}
		else if( popidx == 12 )	// Run Script
		{
//				SharedMemory.TextControl.TextHelp = 1 ;
			RotaryState = ROTARY_MENU_UD ;
	  	pushMenu(menuScript) ;
		}
		else if( popidx == 13 )	// Reset Telemetry
		{
      resetTelemetry( TEL_ITEM_RESET_ALL ) ;
		}
	}
}

#if defined(PCBX12D) || defined(PCBX10)
extern uint8_t ModelImageValid ;
//void updatePicture()
//{
//	uint32_t validSize ;
	
//extern uint16_t Image_width ;
//extern uint16_t Image_height ;
//	validSize = ModelImageValid ;
//	if ( Image_width > 128 )
//	{
//		validSize = 0 ;
//	}
//	if ( Image_height > 64 )
//	{
//		validSize = 0 ;
//	}
	
//	if ( PictureDrawn < 4 )
//	{
//		if ( validSize )
//		{
//	extern void lcd_picture( uint16_t i_x, uint16_t i_y ) ;
//			lcd_picture( 480-128, 128+64+16 ) ;
//		}
//		else
//		{
//			lcdDrawSolidFilledRectDMA( 480-128, 128+64+16, 128, 64, 0 ) ;
//			pushPlotType( PLOT_WHITE ) ;
//			lcd_vline( 240-64, 64+32+8, 31 ) ;
//			lcd_vline( 239, 64+32+8, 31 ) ;
//			lcd_hline( 240-64, 64+32+8, 64 ) ;
//			lcd_hline( 240-64, 95+32+8, 64 ) ;
//			popPlotType() ;
//			lcd_putsAtt( 240-64+26, 68+32+10, "No", INVERS ) ;
//			lcd_putsAtt( 240-64+26-9, 76+32+10, "Image", INVERS ) ;
//		}
//		PictureDrawn += 1 ;
//	}
	
//}
#endif

void menuProc0(uint8_t event)
{
  static uint8_t trimSwLock;
  uint8_t view = g_model.mview & 0xf;
  uint8_t tview = g_model.mview & 0x70 ;
	if ( tview > 0x50 )
	{
		tview = 0x50 ;
	}
	
	StickScrollAllowed = 0 ;

#if defined(PCBX12D) || defined(PCBX10)
	
#endif

#ifdef MAIN_POPUP_ENABLED

	if ( ! PopupData.PopupActive )
	{
#ifdef PCBLEM1
	if ( event == 0 )
	{
extern int32_t Rotary_diff ;
		if ( Rotary_diff > 0 )
		{
			event = EVT_KEY_BREAK(KEY_DOWN) ;
		}
		else if ( Rotary_diff < 0 )
		{
			event = EVT_KEY_BREAK(KEY_UP) ;
		}
		Rotary_diff = 0 ;
	}
#endif
		
		switch(event)
		{
	    case EVT_KEY_LONG(BTN_RE) :		// Nav Popup
    	case EVT_KEY_LONG(KEY_MENU) : // Nav Popup
				PopupData.PopupActive = 2 ;
				PopupData.PopupIdx = 0 ;
      	killEvents(event) ;
				event = 0 ;
				Tevent = 0 ;
    	break ;

#ifndef PAGE_NAVIGATION
	    case EVT_KEY_BREAK(KEY_MENU) :			// ***************
#endif	// nPAGE_NAVIGATION
			case EVT_KEY_BREAK(BTN_RE) :
				if ( view == e_telemetry )
				{
					PopupData.PopupActive = 1 ;
					PopupData.PopupIdx = 0 ;
      		killEvents(event) ;
					event = 0 ;
					Tevent = 0 ;
				}
    	break ;

#ifdef PAGE_NAVIGATION
	    case EVT_KEY_BREAK(KEY_MENU) :			// ***************
#endif	// PAGE_NAVIGATION
  	  case EVT_KEY_BREAK(KEY_RIGHT) :
        if(view <= e_inputs1)
				{
					int8_t x ;
					x = io_subview ;
#ifdef REV9E
					if ( ++x > ((view == e_inputs1) ? 6 : 2) ) x = 0 ;
#else
					if ( ++x > ((view <= e_inputs1) ? 4 : 2) ) x = 0 ;
#endif	// REV9E
					io_subview = x ;
				}	
        if(view == e_telemetry)
				{
					navigateCustomTelemetry( event, 0 ) ;
//					uint32_t t_limit = 0x40 ;
//					tview += 0x10 ;
//					if ( ( FrskyTelemetryType == 2 ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) )
//					{
//						t_limit = 0x50 ;
//					}
//					if ( tview > t_limit )
//					{
//						tview = 0 ;
//					}
					
//            g_model.mview = e_telemetry | tview ;
//						eeModelChanged() ;
//            //            STORE_GENERALVARS;     //eeWriteGeneral();
//            //            eeDirty(EE_GENERAL);
//            audioDefevent(AU_MENUS);
        }
      break ;

#ifndef PAGE_NAVIGATION
	    case EVT_KEY_BREAK(KEY_LEFT):
        if(view <= e_inputs1)
				{
					int8_t x ;
					x = io_subview ;
#if EXTRA_SKYCHANNELS
					if ( --x < 0 ) x = (view < e_inputs1) ? 3 : (view == e_inputs1) ? 4 : 2 ;
#else
					if ( --x < 0 ) x = (view <= e_inputs1) ? 4 : 2 ;
#endif
					io_subview = x ;
				}	
        else if(view == e_telemetry)
				{
					navigateCustomTelemetry( event, 0 ) ;
        }
      break ;
#endif	// nPAGE_NAVIGATION

#ifndef PAGE_NAVIGATION
	    case EVT_KEY_LONG(KEY_RIGHT) :
        pushMenu(menuProcModelSelect) ;
        scroll_disabled = 1 ;
        killEvents(event) ;
			break ;
    
			case EVT_KEY_LONG(KEY_LEFT) :
        pushMenu(menuProcIndex) ;
        scroll_disabled = 1 ;
        killEvents(event) ;
			break ;
#endif	// nPAGE_NAVIGATION
    
			case EVT_KEY_FIRST(KEY_EXIT) :
        if(s_timer[0].s_timerState==TMR_BEEPING)
				{
          s_timer[0].s_timerState = TMR_STOPPED ;
          audioDefevent(AU_MENUS) ;
        }
//        else if (view == e_telemetry)
//				{
//          resetTelemetry() ;
//          audioDefevent(AU_MENUS) ;
//        }
			break ;

#ifdef PAGE_NAVIGATION
			case EVT_KEY_BREAK(KEY_LEFT) :
	      if(view>0)
  	      view = view - 1 ;
    	  else
      	  view = MAX_VIEWS-1 ;
  	    audioDefevent(AU_KEYPAD_DOWN) ;
	      g_model.mview = view | tview ;
				eeModelChanged() ;
				io_subview = 0 ;
	    break ;
			
			case EVT_KEY_LONG(KEY_LEFT) :
        killEvents(event) ;
				view += 1 ;
    	  if( view>=MAX_VIEWS) view = 0 ;
    	  audioDefevent(AU_KEYPAD_UP) ;
    	  g_model.mview = view | tview ;
				eeModelChanged() ;
				io_subview = 0 ;
	    break ;
#endif // PAGE_NAVIGATION

#ifdef PCBLEM1
			case EVT_KEY_BREAK(KEY_UP) :
				view += 1 ;
    	  if( view>=MAX_VIEWS) view = 0 ;
    	  audioDefevent(AU_KEYPAD_UP) ;
    	  g_model.mview = view | tview ;
				eeModelChanged() ;
				io_subview = 0 ;
    	break;

			case EVT_KEY_BREAK(KEY_DOWN) :
	      if(view>0)
  	      view = view - 1 ;
    	  else
      	  view = MAX_VIEWS-1 ;
  	    audioDefevent(AU_KEYPAD_DOWN) ;
	      g_model.mview = view | tview ;
				eeModelChanged() ;
				io_subview = 0 ;
	    break ;
#endif
#ifndef PAGE_NAVIGATION
			case EVT_KEY_BREAK(KEY_UP) :
				view += 1 ;
    	  if( view>=MAX_VIEWS) view = 0 ;
    	  audioDefevent(AU_KEYPAD_UP) ;
    	  g_model.mview = view | tview ;
				eeModelChanged() ;
				io_subview = 0 ;
    	break;

			case EVT_KEY_BREAK(KEY_DOWN) :
	      if(view>0)
  	      view = view - 1 ;
    	  else
      	  view = MAX_VIEWS-1 ;
  	    audioDefevent(AU_KEYPAD_DOWN) ;
	      g_model.mview = view | tview ;
				eeModelChanged() ;
				io_subview = 0 ;
	    break ;

	    case EVT_KEY_LONG(KEY_DOWN):
 				view = e_telemetry ;
				g_model.mview = view | tview ;
				eeModelChanged() ;
      	audioDefevent(AU_MENUS) ;
    	  killEvents(event) ;
  	  break;
    
			case EVT_KEY_LONG(KEY_UP) :
  		  pushMenu(menuProcBattery) ;
				RotaryState = ROTARY_MENU_LR ;
    	  killEvents(event) ;
    	break ;
#endif	// nPAGE_NAVIGATION

	    case EVT_KEY_LONG(KEY_EXIT) :
        resetTimers() ;
//        resetTelemetry() ;
        audioDefevent(AU_MENUS) ;
			break ;
    	
			case EVT_ENTRY:
        killEvents(KEY_EXIT) ;
        killEvents(KEY_UP) ;
        killEvents(KEY_DOWN) ;
        trimSwLock = true ;
				io_subview = 0 ;
			break ;

		}
	} // !PopupActive
#else

 if ( ! PopupData.PopupActive )
 {
	switch(event)
	{
    case  EVT_KEY_LONG(KEY_MENU):// go to last menu
        if( (view == e_telemetry) && ((tview & 0x70) == 0x30 ) )
        {
          AltOffset = -FrskyHubData[FR_ALT_BARO] ;
        }
        else if( (view == e_telemetry) && ( ( (tview & 0x70) == 0 ) !! ((tview & 0x70) == 0x10 )) )
        {
#ifdef WIDE_SCREEN	
          for (uint32_t i=0; i<9; i++)
#else
          for (uint32_t i=0; i<6; i++)
#endif
					{
						uint32_t j ;
						j = tview == 0 ? g_model.customDisplayIndex[i]-1 : g_model.customDisplay2Index[i]-1 ;
#ifdef WIDE_SCREEN	
						if ( i >= 6 )
						{
							j = tview == 0 ? g_model.customDisplay1Extra[i-6]-1 : g_model.customDisplay2Extra[i-6]-1 ;
						}
#endif
						// Might check for a Scaler as well to see if ALT is its source
						if ( j == TEL_ITEM_BALT )
						{
            	AltOffset = -FrskyHubData[FR_ALT_BARO] ;
						}
						if ( ( j >= TEL_ITEM_SC1 ) && (j <= TEL_ITEM_SC8 ) )
						{
							j = g_model.Scalers[j-TEL_ITEM_SC1].source - 1 ;
							if ( j == TEL_ITEM_BALT )
							{
    	        	AltOffset = -FrskyHubData[FR_ALT_BARO] ;
							}
						}
						if ( j == TEL_ITEM_A1 )
						{
            	if ( g_model.frsky.channels[0].type == 3 )		// Current (A)
							{
				    	  frskyTelemetry[0].setoffset() ;
							}
						}
						if ( j == TEL_ITEM_A2 )
						{
            	if ( g_model.frsky.channels[1].type == 3 )		// Current (A)
							{
				    	  frskyTelemetry[1].setoffset() ;
							}
						}
					}
        }
        else if( (view == e_telemetry) && ((tview & 0x70) == 0x40 ) )	// GPS
				{
					struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;
			
					maxMinPtr->hubMax[FR_GPS_SPEED] = 0 ;
					maxMinPtr->hubMax[TELEM_GPS_ALT] = 0 ;
				}
        else
        {
#ifdef PAGE_NAVIGATION
  	  pushMenu(menuProcBattery);
#else
    case  EVT_KEY_LONG(BTN_RE):// go to last menu
		        scroll_disabled = 1;
            pushMenu(lastPopMenu());
//            killEvents(event);
#endif	//
        }
        killEvents(event);
    break;

#ifdef PAGE_NAVIGATION
    case EVT_KEY_LONG(BTN_RE):
#endif	// PAGE_NAVIGATION
    case EVT_KEY_LONG(KEY_RIGHT):
        scroll_disabled = 1;
        pushMenu(menuProcModelSelect);
        killEvents(event);
        break;

#ifdef PAGE_NAVIGATION
    case EVT_KEY_BREAK(KEY_MENU):			// ***************
#endif	// PAGE_NAVIGATION
    case EVT_KEY_BREAK(KEY_RIGHT):
        if(view <= e_inputs1)
				{
					int8_t x ;
					x = io_subview ;
#ifdef REV9E
					if ( ++x > ((view == e_inputs1) ? 6 : 2) ) x = 0 ;
#else
					if ( ++x > ((view == e_inputs1) ? 4 : 2) ) x = 0 ;
#endif	// REV9E
					io_subview = x ;
				}	
        if(view == e_telemetry)
				{
					tview += 0x10 ;
					if ( tview > ( ( FrskyTelemetryType != FRSKY_TEL_DSM ) ? 0x30 : 0x40 ) )
					{
						tview = 0 ;
					}
					
            g_model.mview = e_telemetry | tview ;
						eeModelChanged() ;
            audioDefevent(AU_MENUS);
        }
        break;
#ifndef PAGE_NAVIGATION
    case EVT_KEY_BREAK(KEY_LEFT):
        if(view <= e_inputs1)
				{
					int8_t x ;
					x = io_subview ;
					if ( --x < 0 ) x = (view == e_inputs1) ? 4 : 2 ;
					io_subview = x ;
				}	
        if(view == e_telemetry)
				{
					tview -= 0x10 ;
					if ( tview > 0x40 )
					{
						tview = ( FrskyTelemetryType != FRSKY_TEL_DSM ) ? 0x30 : 0x40 ;
					}
            g_model.mview = e_telemetry | tview ;
						eeModelChanged() ;
            audioDefevent(AU_MENUS);
        }
        break;
#endif	// nPAGE_NAVIGATION
    case EVT_KEY_LONG(KEY_LEFT):
        scroll_disabled = 1;
        pushMenu(menuProcIndex);
        killEvents(event);
        break;
		case EVT_KEY_BREAK(KEY_UP) :
			view += 1 ;
      if( view>=MAX_VIEWS) view = 0 ;
      g_model.mview = view | tview ;
			eeModelChanged() ;
      audioDefevent(AU_KEYPAD_UP) ;
			io_subview = 0 ;
    break;
#ifdef PAGE_NAVIGATION
		case EVT_KEY_BREAK(KEY_PAGE) :
#endif	// PAGE_NAVIGATION
		case EVT_KEY_BREAK(KEY_DOWN) :
      if(view>0)
        view = view - 1;
      else
        view = MAX_VIEWS-1;
      g_model.mview = view | tview ;
			eeModelChanged() ;
      audioDefevent(AU_KEYPAD_DOWN) ;
			io_subview = 0 ;
    break;
    case EVT_KEY_LONG(KEY_UP):
  	  pushMenu(menuProcBattery);
			RotaryState = ROTARY_MENU_LR ;
      killEvents(event);
    break;
    case EVT_KEY_LONG(KEY_DOWN):
#if defined(ARDUPILOT)
        ARDUPILOT_EnableRXD(); // enable ArduPilot-Telemetry reception
        chainMenu(menuProcArduPilot);
#elif defined(NMEA)
        NMEA_EnableRXD(); // enable NMEA-Telemetry reception
        chainMenu(menuProcNMEA);
#else
			
 #ifdef REVX
			if ( g_model.telemetryProtocol == TELEMETRY_JETI )
			{
				chainMenu(menuProcJeti);
			}			        
			else
			{
 #endif
 				view = e_telemetry ;
				g_model.mview = view | tview ;
				eeModelChanged() ;
 #ifdef REVX
			}
 #endif
      audioDefevent(AU_MENUS);
#endif
      killEvents(event);
    break;
    case EVT_KEY_FIRST(KEY_EXIT):
        if(s_timer[0].s_timerState==TMR_BEEPING) {
            s_timer[0].s_timerState = TMR_STOPPED;
            audioDefevent(AU_MENUS);
        }
//        else if (view == e_telemetry) {
//            resetTelemetry();
//            audioDefevent(AU_MENUS);
//        }
        break;
    case EVT_KEY_LONG(KEY_EXIT):
        resetTimers();
//        resetTelemetry();
        audioDefevent(AU_MENUS);
        break;
    case EVT_ENTRY:
        killEvents(KEY_EXIT);
        killEvents(KEY_UP);
        killEvents(KEY_DOWN);
        trimSwLock = true;
				io_subview = 0 ;
        break;

  }
 } // !PopupActive

#endif	// MAIN_POPUP_ENABLED


	{
		uint8_t tsw ;
		tsw = getSwitch00(g_model.trimSw) ;
		if( tsw && !trimSwLock) setStickCenter(0) ;
		trimSwLock = tsw ;
	}

  if (view != e_telemetry)
	{
	  register uint8_t x = FW*2;
  	register uint8_t att = (g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0) | DBLSIZE;
  	register uint32_t i ;

#if defined(PCBX12D) || defined(PCBX10)
		putsDblSizeName( X12SMALL_OFFSET+FW*5/2-3, 0 ) ;
#else
		putsDblSizeName( 0 ) ;
#endif		
    putsVBat( 6*FW+1 + X12SMALL_OFFSET, 2*FH, att|NO_UNIT|CONDENSED);
    lcd_putc( 6*FW+2 + X12SMALL_OFFSET, 3*FH, 'V');

#if defined(PCBX12D) || defined(PCBX10)
		displayTimer( x+18*FW-1 + X12SMALL_OFFSET, FH*2, 0, DBLSIZE|CONDENSED ) ;
#else
		displayTimer( x+14*FW-1, FH*2, 0, DBLSIZE|CONDENSED ) ;
#endif
    putsTmrMode(x+7*FW-FW/2 + X12SMALL_OFFSET,FH*3,0, 0, 0 ) ;

		i = getFlightPhase() ;
		if ( i )
		{
			if (g_model.phaseData[i-1].name[0] != ' ' )
			{
				lcd_putsnAtt( 6*FW+2 + X12SMALL_OFFSET, 2*FH, g_model.phaseData[i-1].name, 6, /*BSS*/ 0 ) ;
			}
			else
			{
  			lcd_putc( 6*FW+2 + X12SMALL_OFFSET, 2*FH, 'F' ) ;
  			lcd_putc( 7*FW+2 + X12SMALL_OFFSET, 2*FH, '0'+ i ) ;
			}
#ifndef ARUNI
			lcd_rect( 6*FW+1 + X12SMALL_OFFSET, 2*FH-1, 6*FW+2, 9 ) ;   // put a bounding box
#endif
		}
		else
		{
    	lcd_putsAttIdx( 6*FW+2 + X12SMALL_OFFSET, 2*FH,PSTR(STR_TRIM_OPTS),g_model.trimInc, 0);
			if ( g_model.thrTrim )
			{
				lcd_puts_P(x+8*FW-FW/2-1 + X12SMALL_OFFSET,2*FH,PSTR(STR_TTM));
			}
		}

  	//trim sliders
		displayTrims() ;
//		displayTrims( 0 ) ;
 	}
  else
	{
		uint8_t i ;
		i = getFlightPhase() ;
		if ( i && g_model.phaseData[i-1].name[0] )
		{
			lcd_putsnAtt( 2*FW + X12SMALL_OFFSET, 0, g_model.phaseData[i-1].name, 6, /*BSS*/ 0 ) ;
		}
		else
		{
    	lcd_putsnAtt(0 + X12SMALL_OFFSET, 0, g_model.name, sizeof(g_model.name), INVERS);
		}
    
		uint8_t att = (g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0);
    putsVBat(14*FW + X12SMALL_OFFSET,0,att);
		displayTimer( 18*FW+5 + X12SMALL_OFFSET, 0, 0, 0 ) ;
  }
  
	if(view<e_inputs1)
	{
#if defined(PCBX12D) || defined(PCBX10)
		if ( io_subview > 1 )
		{
			io_subview = 1 ;
		}
#else
 #if EXTRA_SKYCHANNELS
		if ( io_subview > 3 )
 #else
		if ( io_subview > 2 )
 #endif
		{
			io_subview = 0 ;
		}
#endif
#if defined(PCBX12D) || defined(PCBX10)
#define INDICATOR_OFFSET	(45 + X12SMALL_OFFSET)
#define VALUE_SPACING			34
#else
#define INDICATOR_OFFSET	33
#define VALUE_SPACING			28
#endif
#if defined(PCBX12D) || defined(PCBX10)
		pushPlotType( PLOT_BLACK ) ;
    lcd_hlineStip(INDICATOR_OFFSET, 33, 64, 0x55 ) ;
    lcd_hlineStip(INDICATOR_OFFSET, 34, 64, 0x55 ) ;
		popPlotType() ;
#else
 #if EXTRA_SKYCHANNELS
    lcd_hlineStip(INDICATOR_OFFSET, 33, 64, 0x55 ) ;
    lcd_hlineStip(INDICATOR_OFFSET, 34, 64, 0x55 ) ;
 #else
    lcd_hlineStip(INDICATOR_OFFSET+5, 33, 54, 0x55 ) ;
    lcd_hlineStip(INDICATOR_OFFSET+5, 34, 54, 0x55 ) ;
 #endif
#endif

		if ( ( io_subview == 0 ) || ( BLINK_ON_PHASE ) )
		{
#if defined(PCBX12D) || defined(PCBX10)
			pushPlotType( PLOT_BLACK ) ;
    	lcd_hlineStip(INDICATOR_OFFSET + io_subview * 32, 33, 32, 0xAA ) ;
    	lcd_hlineStip(INDICATOR_OFFSET + io_subview * 32, 34, 32, 0xAA ) ;
			popPlotType() ;
#else
 #if EXTRA_SKYCHANNELS
    	lcd_hlineStip(INDICATOR_OFFSET + io_subview * 16, 33, 16, 0xAA ) ;
    	lcd_hlineStip(INDICATOR_OFFSET + io_subview * 16, 34, 16, 0xAA ) ;
 #else
    	lcd_hlineStip(INDICATOR_OFFSET+5 + io_subview * 18, 33, 18, 0xAA ) ;
    	lcd_hlineStip(INDICATOR_OFFSET+5 + io_subview * 18, 34, 18, 0xAA ) ;
 #endif
#endif
		}

    register uint32_t i ;
#if defined(PCBX12D) || defined(PCBX10)
    for( i=0; i<16; i++)
#else
    for( i=0; i<8; i++)
#endif
    {
      uint8_t x0,y0;
#if defined(PCBX12D) || defined(PCBX10)
			uint8_t chan = 16 * io_subview + i ;
#else
			uint8_t chan = 8 * io_subview + i ;
#endif
      int16_t val = g_chans512[chan];
      switch(view)
      {
        case e_outputValues:
#if defined(PCBX12D) || defined(PCBX10)
          x0 = i%4 ;
          x0 *= VALUE_SPACING ;
          y0 = i/4*(FH+1)+40;
#else
          x0 = i%4 ;
          x0 *= VALUE_SPACING ;
          y0 = i/4*FH+40;
#endif
              // *1000/1024 = x - x/8 + x/32
#define GPERC(x)  (x - x/32 + x/128)
          lcd_outdezAtt( x0+6*FW + X12SMALL_OFFSET , y0, GPERC(val),PREC1 ) ;
        break;
            
				case e_outputBars:
#define WBAR2 (50/2)
#if defined(PCBX12D) || defined(PCBX10)
          x0       = i<8 ? 128/4+2 : 128*3/4-2+24;
          y0       = 38+(i%8)*6;
#else
          x0       = i<4 ? 128/4+2 : 128*3/4-2;
          y0       = 38+(i%4)*5;
#endif
					singleBar( x0 + X12SMALL_OFFSET, y0, val ) ;
				break;
      }
    }
  }
    else if(view == e_telemetry)
		{
#ifdef PCBX9D
			ImageDisplay = 1 ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
			ImageDisplay = 1 ;

			if ( tview > 0x10 )
			{
				pushPlotType( PLOT_BLACK ) ;
				lcd_vline( 127, 8, 48 ) ;
				popPlotType() ;
    		for (uint32_t i=6; i<9; i++)
				{
					uint32_t j ;
					j = g_model.customDisplay1Extra[i-3] ;
					if ( j )
					{
						uint32_t index = j-1 ;
						uint32_t x = 129 ;
						uint32_t y = (i-6)*2*FH + 2*FH ;
						uint8_t style = TELEM_LABEL|TELEM_UNIT|TELEM_UNIT_LEFT|TELEM_VALUE_RIGHT ;
	          if ( index == TEL_ITEM_T1 )
						{
							if ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) )
							{
								style = TELEM_ARDUX_NAME ;
							}	
						}
						if ( index > TELEM_GAP_START )
						{
							index += 8 ;
						}
						putsTelemetryChannel( x, y, index, get_telemetry_value(index),
												 DBLSIZE|CONDENSED, style ) ;
					}
				}
			}
#endif

#ifndef PCBX12D
 #ifndef PCBX10
extern uint8_t LogsRunning ;
			if ( LogsRunning & 1 )
			{
				if ( BLINK_ON_PHASE )
				{
					lcd_img( 10*FW+1 + X12SMALL_OFFSET, 0, IconLogging, 0, 0 ) ;
				}
			}
 #endif
#endif
//        static enum AlarmLevel alarmRaised[2];
        int16_t value ;
				{
            uint8_t y0, x0 ; //, blink;
//            for (int i=0; i<2; i++)
//						{
//              alarmRaised[i] = FRSKY_alarmRaised(i);
//            }
            if ( tview == 0x20 )
            {
                x0 = 0;
                for (int i=0; i<2; i++)
								{
                  if (g_model.frsky.channels[i].lratio)
									{
//                            blink = (alarmRaised[i] ? INVERS : 0);
                    lcd_puts_P(x0, 3*FH, PSTR(STR_A_EQ) ) ;
                    lcd_putc(x0+FW, 3*FH, '1'+i);
                    x0 += 3*FW;
                    putsTelemValue( x0-2, 2*FH, frskyTelemetry[i].value, i,  DBLSIZE|LEFT ) ;
                    if ( g_model.frsky.channels[i].units == 3 )		// Current (A)
										{
                      lcd_outdezAtt(x0+FW, 4*FH,  FrskyHubData[FR_A1_MAH+i], 0);
										}
										else
										{
                      putsTelemValue(x0+FW, 4*FH, FrskyHubMaxMin.hubMin[i], i, 0 ) ;
										}
                      putsTelemValue(x0+3*FW, 4*FH, FrskyHubMaxMin.hubMax[i], i, LEFT ) ;
                  }
                  x0 = 11*FW-2;
                }
								// Fuel Gauge
								if (TelemetryDataValid[FR_FUEL] )
//                if (frskyUsrStreaming)
								{
                	lcd_puts_Pleft( 1*FH, PSTR(STR_FUEL)) ;
									x0 = FrskyHubData[FR_FUEL] ;		// Fuel gauge value
									lcd_hbar( 25, 9, 102, 6, x0 ) ;
								}
                lcd_puts_Pleft( 6*FH, PSTR(STR_RXEQ));
               	lcd_outdezAtt(3 * FW - 2, 5*FH, FrskyHubData[FR_RXRSI_COPY], DBLSIZE|LEFT);
#ifdef ACCESS
								if (TelemetryDataValid[3] )
								{
									putTxSwr( 11 * FW - 2, 6*FH, 0 ) ;
                	lcd_outdezAtt(14 * FW - 4, 5*FH, FrskyHubData[FR_TXRSI_COPY], DBLSIZE|LEFT);
								}
#else								 
								putTxSwr( 11 * FW - 2, 6*FH, 0 ) ;
                lcd_outdezAtt(14 * FW - 4, 5*FH, FrskyHubData[FR_TXRSI_COPY], DBLSIZE|LEFT);
#endif                
								struct t_hub_max_min *maxMinPtr = &FrskyHubMaxMin ;
								
								lcd_outdezAtt(4 * FW, 7*FH, maxMinPtr->hubMin[2], 0);
                lcd_outdezAtt(6 * FW, 7*FH, maxMinPtr->hubMax[2], LEFT);
                lcd_outdezAtt(15 * FW - 2, 7*FH, maxMinPtr->hubMin[3], 0);
                lcd_outdezAtt(17 * FW - 2, 7*FH, maxMinPtr->hubMax[3], LEFT);
            }
            else if ( tview == 0x30 )
            {
                if (frskyUsrStreaming)
                {
									displayTemp( 1, 0, 2*FH, DBLSIZE ) ;
									displayTemp( 2, 14*FW, 7*FH, 0 ) ;

                  lcd_puts_Pleft( 2*FH, PSTR(STR_12_RPM));
                  lcd_outdezNAtt(13*FW, 1*FH, (uint16_t)FrskyHubData[FR_RPM], DBLSIZE|LEFT, 5);

                  value = FrskyHubData[FR_ALT_BARO] + AltOffset ;
									putsTelemetryChannel( 0, 4*FH, TEL_ITEM_BALT, value, DBLSIZE | LEFT | CONDENSED, (TELEM_LABEL | TELEM_UNIT_LEFT)) ;

									lcd_puts_P(10*FW-2, 4*FH, XPSTR("Amax"));
									putsTelemetryChannel( 14*FW, 3*FH, TEL_ITEM_BALT, FrskyHubMaxMin.hubMax[FR_ALT_BARO] + AltOffset, DBLSIZE | LEFT | CONDENSED, 0 ) ;
//									putsTelemValue(17*FW, 4*FH, FrskyHubMaxMin.hubMax[FR_ALT_BARO] + AltOffset, 4, LEFT ) ;
								}	
                if (g_model.frsky.channels[0].lratio)
                {
//                    blink = (alarmRaised[0] ? INVERS+BLINK : 0);
                    lcd_puts_Pleft( 6*FH, XPSTR("A1="));
                    putsTelemValue( 3*FW-2, 5*FH, frskyTelemetry[0].value, 0,  DBLSIZE|LEFT ) ;
                }
                if (g_model.frsky.channels[1].lratio)
                {
//                    blink = (alarmRaised[1] ? INVERS+BLINK : 0);
                    lcd_puts_P(11*FW-2, 6*FH, XPSTR("A2="));
                    putsTelemValue( 14*FW-2, 5*FH, frskyTelemetry[1].value, 1,  DBLSIZE|LEFT ) ;
                }
                lcd_puts_Pleft( 7*FH, PSTR(STR_RXEQ) );
                lcd_outdezAtt(3 * FW, 7*FH, FrskyHubData[FR_RXRSI_COPY], LEFT);
#ifdef ACCESS
								if (TelemetryDataValid[3] )
								{
									putTxSwr( 8 * FW , 7*FH, 0 ) ;
  	              lcd_outdezAtt(11 * FW, 7*FH, FrskyHubData[FR_TXRSI_COPY], LEFT);
								}
#else								 
								putTxSwr( 8 * FW , 7*FH, 0 ) ;
                lcd_outdezAtt(11 * FW, 7*FH, FrskyHubData[FR_TXRSI_COPY], LEFT);
#endif
            }
            else if ( tview == 0x40 )
            {
							uint8_t blink = BLINK ;
							uint16_t mspeed ;
							uint8_t vdisp = FrskyBattCells[1] ? s_timeCumTot & 2 : 0 ;
              if (frskyUsrStreaming)
							{
								blink = 0 ;
							}
//  							displayGPSformat( 4*FW, 1*FH, 0 ) ;
                lcd_puts_Pleft( 2*FH, PSTR(STR_LAT_EQ)) ;
								displayGPSdata( 3*FW, 2*FH, FrskyHubData[FR_GPS_LAT], FrskyHubData[FR_GPS_LATd], LEADING0 | blink ) ;
								displayGPSdata( 3*FW, 3*FH, FrskyHubData[FR_GPS_LONG], FrskyHubData[FR_GPS_LONGd], LEADING0 | blink ) ;
//								div_t qr ;
//								qr = div( FrskyHubData[FR_GPS_LAT], 100 ) ;
//								if ( g_eeGeneral.gpsFormat )
//								{
//									int32_t value ;
//	                lcd_outdezNAtt(6*FW+2, 2*FH, qr.quot, LEADING0 | blink, -3);
//  	              lcd_putc(6*FW+2, 2*FH, '.') ;
//									value = qr.rem ;
//									value *= 10000 ;
//									value += FrskyHubData[FR_GPS_LATd] ;
//									value *= 10 ;
//									value /= 6 ;
//                	lcd_outdezNAtt(12*FW, 2*FH, value, LEADING0 | blink, -6);
//									qr = div( FrskyHubData[FR_GPS_LONG], 100 ) ;
//									lcd_outdezNAtt(6*FW+2, 3*FH, qr.quot, LEADING0 | blink, -3);
//      	          lcd_putc(6*FW+2, 3*FH, '.') ;
//									value = qr.rem ;
//									value *= 10000 ;
//									value += FrskyHubData[FR_GPS_LONGd] ;
//									value *= 10 ;
//									value /= 6 ;
//                	lcd_outdezNAtt(12*FW, 3*FH, value, LEADING0 | blink, -6);
//								}
//								else
//								{
//                	lcd_outdezNAtt(6*FW, 2*FH, qr.quot, LEADING0 | blink, -3);
//                	lcd_outdezNAtt(9*FW, 2*FH, qr.rem, LEADING0 | blink, -2);
//                	lcd_putc(9*FW, 2*FH, '.') ;
//                	lcd_outdezNAtt(13*FW, 2*FH, FrskyHubData[FR_GPS_LATd], LEADING0 | blink, -4);
//									qr = div( FrskyHubData[FR_GPS_LONG], 100 ) ;
//                	lcd_outdezNAtt(6*FW, 3*FH, qr.quot, LEADING0 | blink, -3);
//                	lcd_outdezNAtt(9*FW, 3*FH, qr.rem, LEADING0 | blink, -2);
//                	lcd_putc(9*FW, 3*FH, '.') ;
//                	lcd_outdezNAtt(13*FW, 3*FH, FrskyHubData[FR_GPS_LONGd], LEADING0 | blink, -4);
//								}
								lcd_putcAtt( 14*FW, 2*FH, FrskyHubData[FR_LAT_N_S], blink ) ;
								lcd_putcAtt( 14*FW, 3*FH, FrskyHubData[FR_LONG_E_W], blink ) ;
								uint16_t alt ;
								alt = FrskyHubMaxMin.hubMax[TELEM_GPS_ALT] ;
                if ( g_model.FrSkyImperial )
								{
									alt = m_to_ft( alt ) ;
								}
                lcd_outdezAtt(20*FW, 4*FH, alt, PREC1);
                lcd_outdez(20*FW, 3*FH, FrskyHubData[FR_COURSE] );
                
								mspeed = FrskyHubMaxMin.hubMax[FR_GPS_SPEED] ;
                if ( g_model.FrSkyImperial )
								{
									lcd_puts_Pleft( 5*FH, PSTR(STR_11_MPH)) ;
									mspeed = ( mspeed * 589 ) >> 9 ;
								}
                lcd_outdezAtt(20*FW, 5*FH, mspeed, blink );
								lcd_puts_Pleft( 6*FH, vdisp ? XPSTR("V7=\007V8=\016V9=""\037""10=\00711=\01612=") : XPSTR("V1=\007V2=\016V3=""\037""V4=\007V5=\016V6=")) ;
              if (frskyUsrStreaming)
							{
								uint16_t alt ;
								alt = FrskyHubData[TELEM_GPS_ALT] ;
								mspeed = FrskyHubData[FR_GPS_SPEED] ;
                if ( g_model.FrSkyImperial )
								{
									mspeed = ( mspeed * 589 ) >> 9 ;
									alt = m_to_ft( alt ) ;
									lcd_putc( 9*FW, 4*FH, 'f' ) ;
								}
								lcd_outdezAtt(8 * FW, 4*FH, alt, PREC1 ) ;
                lcd_outdezAtt(8*FW, 5*FH, mspeed, 0);		// Speed
								{
									uint8_t x, y ;
									x = 6*FW ;
									y = 6*FH ;
									uint8_t totalCells = vdisp ? FrskyBattCells[1] : FrskyBattCells[0] ;
									if ( vdisp )
									{
										vdisp = 6 ;
									}
		  						for (uint8_t k=0 ; k<totalCells ; k++)
									{
										uint8_t blink=0;
										if ( k == 3 )
										{
											x = 6*FW ;
											y = 7*FH ;
										}
										if ((FrskyHubData[FR_CELL1+k + vdisp] < g_model.frSkyVoltThreshold * 2))
										{
										  blink = BLINK ;
										}
  									lcd_outdezNAtt( x, y, FrskyHubData[FR_CELL1+k + vdisp], blink | PREC2, 4 ) ;
										x += 7*FW ;
										if ( k == 5 )		// Max 6 cels displayable
										{
											break ;											
										}
	      					}
								}
              }
						}
            else if ( ( tview == 0 ) || ( tview == 0x10 ) )	// CUSTOM TELEMETRY DISPLAY
            {
						 uint32_t scr = g_model.customDisplay1Extra[6] ;
						 if ( tview == 0x10 )
						 {
						 	scr = g_model.customDisplay2Extra[6] ;
						 }				 
						 if ( scr )
						 {
							lcd_puts_Pleft( 3*FH, XPSTR( "\004SCRIPT ERROR" ) ) ;
						 }
						 else
						 {
#if defined(PCBX12D) || defined(PCBX10)
							pushPlotType( PLOT_BLACK ) ;
#endif
							lcd_vline( 63, 8, 48 ) ;
#if defined(PCBX12D) || defined(PCBX10)
							popPlotType() ;
#endif
							
#ifdef WIDE_SCREEN
							ImageDisplay = 0 ;
#if defined(PCBX12D) || defined(PCBX10)
							pushPlotType( PLOT_BLACK ) ;
#endif
							lcd_vline( 127, 8, 48 ) ;
#if defined(PCBX12D) || defined(PCBX10)
							popPlotType() ;
#endif
		          for (uint32_t i=0; i<9; i++)
#else
    		      for (uint32_t i=0; i<6; i++)
#endif
							{
								uint32_t j ;
								j = tview == 0 ? g_model.customDisplayIndex[i] : g_model.customDisplay2Index[i] ;
#ifdef WIDE_SCREEN	
								if ( i >= 6 )
								{
									j = tview == 0 ? g_model.customDisplay1Extra[i-6] : g_model.customDisplay2Extra[i-6] ;
								}
#endif
								if ( j )
								{
									uint32_t index = j-1 ;
#ifdef WIDE_SCREEN	
									uint32_t x = (i&1)?65:1 ;
									uint32_t y = (i&0x0E)*FH+2*FH ;
									if ( i >= 6 )
									{
										x = 129 ;
										y = (i-6)*2*FH + 2*FH ;
									}
#else
									uint32_t x = (i&1)?65:1 ;
									uint32_t y = (i&0x0E)*FH+2*FH ;
#endif
									
									uint8_t style = TELEM_LABEL|TELEM_UNIT|TELEM_UNIT_LEFT|TELEM_VALUE_RIGHT ;
	                if ( index == TEL_ITEM_T1 )
									{
										if ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) )
										{
											style = TELEM_ARDUX_NAME ;
										}	
									}
									putsTelemetryChannel( x, y, index, get_telemetry_value(index),
															 DBLSIZE|CONDENSED, style ) ;
								}
							}

                y0 = 7*FH;
                
#ifdef WIDE_SCREEN
								lcd_puts_P( X12SMALL_OFFSET, y0, PSTR(STR_RXEQ) ) ;
#else
								lcd_puts_Pleft( y0, PSTR(STR_RXEQ) ) ;
#endif
								lcd_hbar( 20 + X12SMALL_OFFSET, 57, 43, 6, FrskyHubData[FR_RXRSI_COPY] ) ;
#ifdef ACCESS
								if (TelemetryDataValid[3] )
								{
									putTxSwr( 110 + X12SMALL_OFFSET, y0, 0 ) ;
									lcd_hbar( 65 + X12SMALL_OFFSET, 57, 43, 6, FrskyHubData[FR_TXRSI_COPY] ) ;
								}
#else								 
								putTxSwr( 110 + X12SMALL_OFFSET, y0, 0 ) ;
								lcd_hbar( 65 + X12SMALL_OFFSET, 57, 43, 6, FrskyHubData[FR_TXRSI_COPY] ) ;
#endif
						 }
            }
            else// if ( tview == 0x50 )
						{
						 if ( ( g_model.telemetryProtocol == TELEMETRY_ARDUCOPTER ) || ( g_model.telemetryProtocol == TELEMETRY_ARDUPLANE ) || ( g_model.telemetryProtocol == TELEMETRY_MAVLINK ) )
						 {
								// Mavlink over FrSky
								uint8_t attr = 0 ;
								lcd_putsAtt(  0 * FW, 0 * FH, XPSTR("          "), 0 ); // clear inversed model name from screen
								// line 0, left - Battary remaining
								uint8_t batt =  FrskyHubData[FR_FUEL] ;
								if ( batt < 20 ) { attr = BLINK ; } else {attr = 0 ;}
								lcd_hbar( 01, 1, 10, 5, batt ) ;
								lcd_rect( 11, 2, 2, 3 ) ;
								if (batt > 99) batt = 99 ;
								lcd_outdezAtt ( 4*FW,   0*FH, batt, attr ) ;
								lcd_putcAtt	  ( 4*FW+1, 0*FH, '%',  attr ) ;
								// line 0, V, volt bat
 				        attr = PREC1 ;
 				        lcd_outdezAtt( 9 * FW+2, 0, FrskyHubData[FR_VOLTS], attr ) ;
								lcd_putc   ( 9 * FW+3, 0, 'v'); //"v"
								// line 0, V, volt cpu
								// line 1 - Flight mode
 				        uint8_t blink = BLINK;
 				        if (frskyUsrStreaming) blink = 0 ;
								
#define STR_MAV_ARMED  		 "ARMED"
#define STR_MAV_DISARMED   "DISARM"
#define STR_MAV_NODATA 		 "NODATA"
								if (frskyUsrStreaming)
								{
									if ( FrskyHubData[FR_BASEMODE] & MAV_MODE_FLAG_SAFETY_ARMED )
									{
										lcd_putsAtt(  1 * FW, 1 * FH, XPSTR(STR_MAV_ARMED), blink) ;
									}
									else
									{
										lcd_putsAtt(  0 * FW, 1 * FH, XPSTR(STR_MAV_DISARMED), blink) ;
									}
								}
								else
								{
									lcd_putsAtt(  0 * FW, 1 * FH, XPSTR(STR_MAV_NODATA), BLINK) ; // NO DRV
								}

//							  lcd_outdez( 15 * FW, 3 * FH, FrskyHubData[FR_BASEMODE] ) ;

								const char *s ;
								s = arduFlightMode( FrskyHubData[FR_TEMP1] ) ;
								lcd_putsAtt( 15*FW-1, 1*FH, s, blink ) ;

//		 line 2 - "GPS fix" converted from TEMP2
								uint8_t gps_fix ;
								uint8_t gps_sat ;
								gps_sat = FrskyHubData[FR_TEMP2] / 10 ;
 				        gps_fix = FrskyHubData[FR_TEMP2] - gps_sat * 10 ;
#define STR_MAV_GPS_NO_GPS   "No GPS"
#define STR_MAV_GPS_NO_FIX	 "No Fix" // 1
#define STR_MAV_GPS_2DFIX	 "2D Fix" // 2
#define STR_MAV_GPS_3DFIX	 "3D Fix" // 3
								switch ( gps_fix )
								{
									case 1: 	lcd_putsAtt( 0 * FW, 2*FH, XPSTR(STR_MAV_GPS_NO_FIX), BLINK); break;
									case 2: 	lcd_puts_P ( 0 * FW, 2*FH, XPSTR(STR_MAV_GPS_2DFIX )); break;
									case 3:
									case 4: 	lcd_puts_P ( 0 * FW, 2*FH, XPSTR(STR_MAV_GPS_3DFIX )); break;
									default : lcd_putsAtt( 0 * FW, 2*FH, XPSTR(STR_MAV_GPS_NO_GPS), INVERS); break;
								}
//		line 3 left "SAT"
#define STR_MAV_GPS_SAT_COUNT "sat"
								lcd_puts_P( 0 * FW,   3*FH, XPSTR(STR_MAV_GPS_SAT_COUNT) ); // sat
								lcd_outdez( 7 * FW-2, 3*FH, gps_sat ) ;

//		 line 4 left "HDOP"
                int16_t val;
								val = FrskyHubData[FR_GPS_HDOP];
								if (val <= 200)
								{
                	attr  = PREC2;
				   				blink = 0;
				 				}
								else
								{
                  attr = PREC2 | BLINK;
				   				blink = BLINK;
								}
#define STR_MAV_GPS_HDOP     "hdop"
								lcd_putsAtt( 0 * FW,   4*FH, XPSTR(STR_MAV_GPS_HDOP), blink ) ; 
                lcd_outdezAtt( 7 * FW-2, 4*FH, val, attr ) ; // hdop
 				        
								if (frskyUsrStreaming) blink = 0 ;

//		 line 5 left  "heartbeat"
								lcd_hbar( 0, 5*FH+1, 40, 4, frskyUsrStreaming ) ; // heartbeat
//		 line 2 right "Baro Alt"
#define STR_MAV_ALT          "Alt"
								lcd_puts_P( 15 * FW-2, 2*FH, XPSTR(STR_MAV_ALT) ) ; // Alt
								val = get_telemetry_value(TEL_ITEM_BALT) ;
								attr = 0 ;
								if ( g_model.FrSkyImperial )
								{
      					 	val = m_to_ft( val ) ;
								}
                if ( val < 1000 )
								{
                  attr |= PREC1 ;
                }
								else
								{
                  val /= 10 ;
                }
                lcd_outdezAtt( 21 * FW+1, 2*FH, val, attr ) ;

//		 line 3 right "GPS Alt"
#define STR_MAV_GALT         "gAl"
								lcd_puts_P( 15 * FW-2, 3*FH, XPSTR(STR_MAV_GALT) ); // GAlt
								val = get_telemetry_value(TEL_ITEM_GALT) ;
								if ( g_model.FrSkyImperial )
								{
      					 	val = m_to_ft( val ) ;
								}
								lcd_outdezAtt( 21 * FW+1, 3*FH, val, PREC1 ) ;
//		 line 4 right "Home Distance"
#define STR_MAV_HOME         "dth"
								lcd_puts_P( 15 * FW-2, 4*FH, XPSTR(STR_MAV_HOME) ); // home
								lcd_outdez( 21 * FW+1, 4*FH, FrskyHubData[FR_HOME_DIST] ) ;
//		 line 5 right "Current, Milliampers"
#define STR_MAV_CURRENT          "Cur"
								lcd_puts_P( 15 * FW-2, 5*FH, XPSTR(STR_MAV_CURRENT) ); // "Cur"
		        		lcd_putcAtt( 20 * FW+3, 5*FH, 'A', CONDENSED );
                lcd_outdezAtt( 20 * FW+3, 5*FH, FrskyHubData[FR_CURRENT], PREC1 ) ;
//		 line 6 left "Rssi"
								lcd_puts_P( 0 * FW, 6*FH, XPSTR("Rssi") ); // "Rssi"
								uint8_t rx = FrskyHubData[FR_RXRSI_COPY];
								if (rx > 100) rx = 100;
                lcd_outdezAtt( 7 * FW-3, 6*FH, rx, blink);
//         line 6 right  "Rcq"
                lcd_puts_P( 15*FW-2, 6*FH, XPSTR( ( FrskyTelemetryType == FRSKY_TEL_SPORT ) ? "Swr" : "Rcq") ); // "Rcq"
								uint8_t tx = FrskyHubData[FR_TXRSI_COPY];
								if (tx > 100) tx = 100;
                lcd_outdezAtt( 21*FW+1, 6*FH, tx, blink);

//		 line 7 
                lcd_puts_P( 0 * FW, 7*FH, XPSTR("Rx") );
								lcd_hbar( 14, 57, 49, 6, FrskyHubData[FR_RXRSI_COPY] ) ;
                lcd_puts_P( 19 * FW+1, 7*FH, XPSTR("Tx") );
								lcd_hbar( 65, 57, 49, 6, FrskyHubData[FR_TXRSI_COPY] ) ;

//				 THROTTLE bar, %
//								lcd_vbar( 0, 1 * FH, 4, 6 * FH - 1, FrskyHubData[FR_RPM] ) ;

								
// Mavlink - home direction and distance on graphic?								
								uint8_t x0 = 64 ;
								uint8_t y0 = 31 ;
								uint8_t r  = 22 ;
								uint8_t x ;
								uint8_t y ;
								DO_SQUARE( x0, y0, r * 2 + 1 ) ;
								int16_t hdg = FrskyHubData[FR_COURSE] ;
								if  ( FrskyHubData[FR_BASEMODE] & MAV_MODE_FLAG_SAFETY_ARMED ) // armed
								{   // we save head position before arming
									if ( hdg_home == 0 ) hdg_home = hdg;
								} else {
									hdg_home = 0 ;
								}
								hdg = hdg - hdg_home + 270 ; // use SIMPLE mode 
								if ( FrskyHubData[FR_HOME_DIR] )
								{
									hdg = FrskyHubData[FR_HOME_DIR] ;
								}
								for (int32_t i = -3 ; i < r ; i += 1 )
								{
									x = x0 + ( i * rxcos100 ( hdg ) ) / 100; 
									y = y0 + ( i * rxsin100 ( hdg ) ) / 100;
									lcd_plot(x, y);
								}
						 }
						 else
						 {
							// Spektrum format screen
							lcd_puts_P( 11*FW-1 + X12SMALL_OFFSET, 0*FH, XPSTR("tssi ")) ;	// Space to blank battery voltage
							lcd_puts_Pleft( 2*FH, XPSTR("Vbat")) ;
							lcd_puts_Pleft( 2*FH, XPSTR("\013RxV")) ;
							lcd_puts_Pleft( 4*FH, XPSTR("AMP\013Temp")) ;
							lcd_puts_Pleft( 6*FH, XPSTR("RPM\021DSM2")) ;
							
							lcd_vline( 63, 8, 32 ) ;

              lcd_outdezAtt( 17*FW-1, 0*FH, FrskyHubData[FR_RXRSI_COPY], 0 ) ;
              lcd_outdezAtt( 61, 1*FH, FrskyHubData[FR_VOLTS], PREC1|DBLSIZE ) ;
							lcd_outdezAtt( 125, 1*FH, convertRxv( FrskyHubData[FR_RXV] ), PREC1|DBLSIZE ) ;
							lcd_outdezAtt( 61, 3*FH, FrskyHubData[FR_CURRENT], PREC1|DBLSIZE ) ;
              lcd_outdezAtt( 125, 3*FH, FrskyHubData[FR_TEMP1], DBLSIZE ) ;
              lcd_outdezAtt( 14*FW, 5*FH, FrskyHubData[FR_RPM], DBLSIZE ) ;

							if ( ( g_model.Module[0].protocol == PROTO_MULTI ) || ( g_model.Module[1].protocol == PROTO_MULTI ) )
							{
								if ( ( g_model.Module[0].sub_protocol & 0x3F ) == M_DSM )
								{
									if ( (g_model.Module[0].channels & 0x60 )== 0x20)
									{
										lcd_putc( 20*FW, 6*FH, 'X' ) ;
									}
								}
								if ( ( g_model.Module[1].sub_protocol & 0x3F ) == M_DSM )
								{
									if ( (g_model.Module[1].channels & 0x60 ) == 0x20)
									{
										lcd_putc( 20*FW, 6*FH, 'X' ) ;
									}
								}
							}
							else if ( g_model.dsmMode & ORTX_USE_DSMX )
							{
								lcd_putc( 20*FW, 6*FH, 'X' ) ;
							}
							dsmDisplayABLRFH() ;
						 }
						}

        }
    }
  else if(view<e_timer2)
	{
		doMainScreenGrphics() ;

    int8_t a = io_subview ;
#if defined(PCBSKY) || defined(PCB9XT)
		if ( a != 0 ) a = a * 6 + 3 ;		// 0, 9, 15
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
		if ( a != 0 ) a = a * 6 ;		// 0, 6, 12
#endif
    uint8_t j ;
		for ( j = 0 ; j < 2 ; j += 1 )
		{
#if defined(PCBSKY) || defined(PCB9XT)
			if ( j == 1 )
			{
				a = io_subview ;
				a += 1 ;
				a *= 6 ;		// 6, 12, 18
			}
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
			if ( j == 1 )
			{
				a += 3 ;
			}
#endif
			switchDisplay( j, a ) ;
		}
#if defined(PCBX12D) || defined(PCBX10)
extern uint16_t Image_width ;
extern uint16_t Image_height ;
		if ( ModelImageValid )
		{
	extern void lcd_picture( uint16_t i_x, uint16_t i_y, uint16_t maxHeight ) ;
			lcd_picture( (480-Image_width)/2, 128+18+32 - Image_height / 2, 84 ) ;
		}
#endif
	}
#if defined(PCBX12D) || defined(PCBX10)
  else if(view == e_picture)
	{
extern uint16_t Image_width ;
extern uint16_t Image_height ;
		
		if ( ModelImageValid )
		{
	extern void lcd_picture( uint16_t i_x, uint16_t i_y, uint16_t maxHeight ) ;
			lcd_picture( (480-Image_width)/2, 140 - Image_height / 2, 114 ) ;
		}
		else
		{
//	LCDLastOp = 'T' ;
			lcdDrawSolidFilledRectDMA( (480-128)/2, 96, 128, 64, 0 ) ;
			lcd_putsAtt( (240-64)/2+26, 52, "No", INVERS ) ;
			lcd_putsAtt( (240-64)/2+26-9, 60, "Image", INVERS ) ;
		}
	}
#endif
  else  // New Timer2 display
  {
#if defined(PCBX12D) || defined(PCBX10)
		displayTimer( 30+5*FW+12 + X12SMALL_OFFSET, FH*5, 1, DBLSIZE ) ;
//    putsTime( FW*17+3+24, 5*FH, Time.hour * 60 + Time.minute, 0, 0 ) ;
#else		
		displayTimer( 30+5*FW, FH*5, 1, DBLSIZE ) ;
//    putsTime( FW*17+3, 5*FH, Time.hour * 60 + Time.minute, 0, 0 ) ;
#endif
    putsTmrMode( 30-2*FW-FW/2 + X12SMALL_OFFSET,FH*6, 0, 1, 0 ) ;
#if defined(PCBX12D) || defined(PCBX10)
extern uint16_t Image_width ;
extern uint16_t Image_height ;
		if ( ModelImageValid )
		{
	extern void lcd_picture( uint16_t i_x, uint16_t i_y, uint16_t maxHeight ) ;
			lcd_picture( (480-Image_width)/2, 128+18+8+8+4 - Image_height / 2, 92 ) ;
		}
#endif
  }

	if ( PopupData.PopupActive )
	{
		actionMainPopup( event ) ;
	}
}

int16_t intpol(int16_t x, uint8_t idx) // -100, -75, -50, -25, 0 ,25 ,50, 75, 100
{
#define D9 (RESX * 2 / 8)
#define D5 (RESX * 2 / 4)
#define D6 (RESX * 2 / 5)
    uint32_t cv9 = idx >= MAX_CURVE5;
		int8_t *crv ;
		if ( idx == MAX_CURVE5 + MAX_CURVE9 )
		{ // The xy curve
			crv = g_model.curvexy ;
			cv9 = 2 ;
		}
		else if ( idx == MAX_CURVE5 + MAX_CURVE9 + 1)
		{ // The xy curve
			crv = g_model.curve2xy ;
			cv9 = 2 ;
		}
		else if ( idx == MAX_CURVE5 + MAX_CURVE9 + 2 )
		{
			crv = g_model.curve6 ;
			cv9 = 3 ;
		}
		else
		{
    	crv = cv9 ? g_model.curves9[idx-MAX_CURVE5] : g_model.curves5[idx];
		}
    int16_t erg;

    x+=RESXu;
    if(x < 0)
		{
      erg = (int16_t)crv[0] * (RESX/4);
    }
		else if(x >= (RESX*2))
		{
			if ( cv9 == 3 )
			{
      	erg = (int16_t)crv[5] * (RESX/4);
			}
			else
			{
      	erg = (int16_t)crv[(cv9 ? 8 : 4)] * (RESX/4);
			}
    }
		else
		{
			int16_t deltax ;
			div_t qr ;
			if ( cv9 == 2 ) // xy curve
			{
		    int16_t a = 0 ;
				int16_t b ;
				int16_t c ;
				uint32_t i ;

				// handle end points
  	    c = RESX + calc100toRESX(crv[17]) ;
				if ((uint16_t)x>c)
				{
					return calc100toRESX(crv[8]) ;
				}
  	    b = RESX + calc100toRESX(crv[9]) ;
				if ((uint16_t)x<b)
				{
					return calc100toRESX(crv[0]) ;
				}

				for ( i = 0 ; i < 8 ; i += 1 )
				{
	        a = b ;
  	      b = (i==7 ? c : RESX + calc100toRESX(crv[i+10]));
    	    if ((uint16_t)x<=b) break;
				}
				qr.quot = i ;
				qr.rem = x - a ;
				deltax = b - a ;
			}
			else
			{
        if(cv9 == 3)
				{
					qr = div( x, D6 ) ;
					deltax = D6 ;
				}
				else if ( cv9 )
				{
					qr = div( x, D9 ) ;
					deltax = D9 ;
        }
				else
				{
					qr = div( x, D5 ) ;
					deltax = D5 ;
        }
  	  }
			int32_t y1 = (int16_t)crv[qr.quot] * (RESX/4) ;
			int32_t deltay = (int16_t)crv[qr.quot+1] * (RESX/4) - y1 ;
			erg = y1 + ( qr.rem ) * deltay / deltax ;
		}
    return erg / 25; // 100*D5/RESX;
}

int16_t calcExpo( uint8_t channel, int16_t value )
{
  uint8_t expoDrOn = get_dr_state(channel);
  uint8_t stkDir = value > 0 ? DR_RIGHT : DR_LEFT ;

  if(IS_THROTTLE(channel) && g_model.thrExpo)
	{
    value  = 2*expo((value+RESX)/2,REG100_100(g_model.expoData[channel].expo[expoDrOn][DR_EXPO][DR_RIGHT])) ;
    stkDir = DR_RIGHT ;
  }
  else
    value  = expo(value,REG100_100(g_model.expoData[channel].expo[expoDrOn][DR_EXPO][stkDir])) ;

  value = (int32_t)value * (REG(g_model.expoData[channel].expo[expoDrOn][DR_WEIGHT][stkDir]+100, 0, 100))/100 ;
  if (IS_THROTTLE(channel) && g_model.thrExpo) value -= RESX;
	return value ;
}

void menuProcSDstat(uint8_t event)
{
	MENU(PSTR(STR_ST_CARD_STAT), menuTabStat, e_Setup3, 1, {0} ) ;

#ifdef PCBX9D
	if ( event == EVT_ENTRY )
	{
extern void sdInit( void ) ;
		sdInit() ;
	}
#endif
	 
	uint32_t present ;
	lcd_puts_Pleft( 1*FH, XPSTR("Present"));

#ifdef PCBSKY
	present = CardIsPresent() ;
#else
extern DWORD socket_is_empty( void ) ;
	present = !socket_is_empty() ;
#endif
	lcd_putsAttIdx( 9*FW, 1*FH, XPSTR("\003No Yes"), present, 0 ) ;
	
	lcd_puts_Pleft( 2*FH, PSTR(STR_4_READY));

extern uint8_t SectorsPerCluster ;
  lcd_outdezAtt( 127, 1*FH, SectorsPerCluster/2, 0 ) ;
#ifndef SIMU


#ifdef PCBSKY
#ifndef SMALL
	uint32_t i ;
	uint8_t x, y ;
#endif
#endif

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
extern uint8_t CardType ;
extern uint32_t sdMounted( void ) ;
	lcd_outhex4( 10*FW, 2*FH, CardType ) ;
	lcd_outhex4( 16*FW, 2*FH, sdMounted() ) ;
	lcd_outhex4( 10*FW, 3*FH, Card_state ) ;
#endif

#ifdef PCBSKY
	lcd_outhex4( 10*FW, 2*FH, Card_state ) ;
	 
#ifndef SMALL
extern uint32_t SDlastError ;
	lcd_outhex4( 16*FW, 2*FH, SDlastError ) ;
	 
		y = 3*FH ;
		x = 4*FW ;
		lcd_puts_Pleft( y, XPSTR("CID"));
		for ( i = 0 ; i < 4 ; i += 1 )
		{
		  lcd_outhex4( x, y, Card_ID[i] >> 16 ) ;
		  lcd_outhex4( x+4*FW, y, Card_ID[i] ) ;
			x += 8*FW ;
			if ( i == 1 )
			{
				y += FH ;
				x = 4*FW ;				
			}			 
		}
		y = 5*FH ;
		x = 4*FW ;
		lcd_puts_Pleft( y, XPSTR("CSD"));
		for ( i = 0 ; i < 4 ; i += 1 )
		{
		  lcd_outhex4( x, y, Card_CSD[i] >> 16 ) ;
		  lcd_outhex4( x+4*FW, y, Card_CSD[i] ) ;
			x += 8*FW ;
			if ( i == 1 )
			{
				y += FH ;
				x = 4*FW ;				
			}			 
		}
		y = 7*FH ;
		x = 4*FW ;
		lcd_puts_Pleft( y, XPSTR("SCR"));
		for ( i = 0 ; i < 2 ; i += 1 )
		{
		  lcd_outhex4( x, y, Card_SCR[i] >> 16 ) ;
		  lcd_outhex4( x+4*FW, y, Card_SCR[i] ) ;
			x += 8*FW ;
		}
#endif
#endif
	if (sd_card_ready() )
	{
	}
	else
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_NOT));
	}
#endif
}


uint8_t evalOffset(int8_t sub)
{
	uint8_t max = (SCREEN_LINES - 2) ;
  uint8_t t_pgOfs = s_pgOfs ;
	int8_t x = sub-t_pgOfs ;
    if(sub<1) t_pgOfs=0;
    else if(x>max) t_pgOfs = sub-max;
    else if(x<max-(SCREEN_LINES - 2)) t_pgOfs = sub-max+(SCREEN_LINES - 2);
		return (s_pgOfs = t_pgOfs) ;
}

//#if defined(PCBX12D) || defined(PCBX10)
//uint8_t evalOffsetLarge(int8_t sub)
//{
//	uint8_t max = (SCREEN_LINES - 2) ;
//  uint8_t t_pgOfs = s_pgOfs ;
//	int8_t x = sub-t_pgOfs ;
//    if(sub<1) t_pgOfs=0;
//    else if(x>max) t_pgOfs = sub-max;
//    else if(x<max-(SCREEN_LINES - 2)) t_pgOfs = sub-max+(SCREEN_LINES - 2);
//		return (s_pgOfs = t_pgOfs) ;
//}
//#endif

#define EBACK_START		0
#define EBACK_OPEN		1
#define EBACK_WRITE		2

#define EBACK_DONE		10

#define ERESTORE_START				0
#define ERESTORE_FILL_NAMES		1
#define ERESTORE_SELECT				2
#define ERESTORE_CONFIRM			3
#define ERESTORE_OPEN					4
#define ERESTORE_READ					5

#define ERESTORE_DONE		10


#if defined(PCBSKY) || defined(PCB9XT)
extern uint16_t General_timer ;
extern uint16_t Model_timer ;
#endif

const char *EepromErrorText ;

void menuBackupEeprom( uint8_t event )
{
	static uint16_t count ;
	static uint8_t state ;
	TITLE(XPSTR("Backup EEPROM"));
	
	switch ( event )
	{
		case EVT_ENTRY :
			state = EBACK_START ;
			EepromErrorText = NULL ;
#if defined(PCBSKY) || defined(PCB9XT)
			if ( General_timer )
			{
				General_timer = 1 ;		// Make these happen soon
			}
			if ( Model_timer )
			{
				Model_timer = 1 ;
			}
#endif

		break ;

    case EVT_KEY_BREAK(KEY_EXIT):
			if ( state == EBACK_DONE )
			{
				popMenu( false ) ;
				// need a reboot if restoring!!
				// Or call eereadall()
			}
		break ;
	}

	switch ( state )
	{
		case EBACK_START :
			lcd_puts_Pleft( 4*FH, XPSTR("\006Preparing") ) ;
			if ( ee32_check_finished() )
			{
				state = EBACK_OPEN ;
			}
		break ;

		case EBACK_OPEN :
			WatchdogTimeout = 300 ;		// 3 seconds
			count = 0 ;
			EepromErrorText = openBackupEeprom() ;
			if ( EepromErrorText == NULL)
			{
				state = EBACK_WRITE ;
			}
			else
			{
				state = EBACK_DONE ;
			}
		break ;
		
		case EBACK_WRITE :
			WatchdogTimeout = 300 ;		// 3 seconds
#if defined(PCBSKY) || defined(PCB9XT)
			if ( count < 256 )
#else
			if ( count < 64 )
#endif
			{
				EepromErrorText = processBackupEeprom( count ) ;
				uint32_t i ;
#if defined(PCBSKY) || defined(PCB9XT)
				uint32_t width = count / 4 ;
#else
				uint32_t width = count ;
#endif
				lcd_hline( 0, 5*FH-1, 65 ) ;
				lcd_hline( 0, 6*FH, 65 ) ;
				lcd_vline( 64, 5*FH, 8 ) ;
				for ( i = 0 ; i <= width ; i += 1 )
				{
					lcd_vline( i, 5*FH, 8 ) ;
				}
				count += 1 ;
				if ( EepromErrorText )
				{
					state = EBACK_DONE ;
					closeBackupEeprom() ;					
				}
			}
			else
			{
				state = EBACK_DONE ;
				closeBackupEeprom() ;					
			}
		break ;
		
		case EBACK_DONE :
			if ( EepromErrorText == NULL)
			{
				lcd_puts_Pleft( 4*FH, XPSTR("\004EEPROM Saved") ) ;
			}
			else
			{
				lcd_puts_Pleft( 4*FH, EepromErrorText ) ;
			}
		break ;
	}
}

void menuRestoreEeprom( uint8_t event )
{
	static uint16_t count ;
	static uint8_t state ;
	struct fileControl *fc = &FileControl ;
	TITLE(XPSTR("Restore EEPROM"));
	
	switch ( event )
	{
		case EVT_ENTRY :
			state = ERESTORE_START ;
			EepromErrorText = NULL ;
#if defined(PCBSKY) || defined(PCB9XT)
			if ( General_timer )
			{
				General_timer = 1 ;		// Make these happen soon
			}
			if ( Model_timer )
			{
				Model_timer = 1 ;
			}
#endif

		break ;

    case EVT_KEY_BREAK(KEY_EXIT):
			if ( state == ERESTORE_DONE )
			{
				popMenu( false ) ;
				// need a reboot if restoring!!
				// Or call eereadall()
			}
		break ;
	}

	switch ( state )
	{
		case EBACK_START :
			lcd_puts_Pleft( 4*FH, XPSTR("\006Preparing") ) ;
			if ( ee32_check_finished() )
			{
				state = ERESTORE_FILL_NAMES ;
			}
		break ;

		case ERESTORE_FILL_NAMES :
			setupFileNames( (TCHAR *)"/EEPROM", fc, (char *)"BIN" ) ;
			state = ERESTORE_SELECT ;
		break ;
			
		case ERESTORE_SELECT :
		{
			uint32_t i ;
			i = fileList( event, &FileControl ) ;
			if ( i == 1 )	// Select
			{
				state = ERESTORE_CONFIRM ;
			}
			else if ( i == 2 )	// EXIT
			{
  		  killEvents(event) ;
  		  popMenu() ;
			}
		}
		break ;

		case ERESTORE_CONFIRM :
			lcd_puts_Pleft( 2*FH, "Restore EEPROM from" ) ;
			lcd_putsnAtt( 0, 4*FH, SharedMemory.FileList.Filenames[fc->vpos], 21, 0 ) ;
			if ( event == EVT_KEY_LONG(KEY_MENU) )
			{
				state = ERESTORE_OPEN ;
			}
			if ( event == EVT_KEY_LONG(KEY_EXIT) )
			{
				state = ERESTORE_SELECT ;		// Canceled
			}
		break ;

		case ERESTORE_OPEN :
			WatchdogTimeout = 300 ;		// 3 seconds
			count = 0 ;
			EepromErrorText = openRestoreEeprom( SharedMemory.FileList.Filenames[fc->vpos] ) ;
			if ( EepromErrorText == NULL)
			{
				state = ERESTORE_READ ;
			}
			else
			{
				state = ERESTORE_DONE ;
			}
		break ;
		
		case ERESTORE_READ :
#if defined(PCBSKY) || defined(PCB9XT)
			if ( count < 256 )
#else
			if ( count < 64 )
#endif
			{
				EepromErrorText = processRestoreEeprom( count ) ;
				uint32_t i ;
#if defined(PCBSKY) || defined(PCB9XT)
				uint32_t width = count / 4 ;
#else
				uint32_t width = count ;
#endif
				lcd_hline( 0, 5*FH-1, 65 ) ;
				lcd_hline( 0, 6*FH, 65 ) ;
				lcd_vline( 64, 5*FH, 8 ) ;
				for ( i = 0 ; i <= width ; i += 1 )
				{
					lcd_vline( i, 5*FH, 8 ) ;
				}
				count += 1 ;
				if ( EepromErrorText )
				{
					state = ERESTORE_DONE ;
#if defined(PCBSKY) || defined(PCB9XT)
					closeBackupEeprom() ;					
#endif
				}
			}
			else
			{
				state = ERESTORE_DONE ;
				closeBackupEeprom() ;					
#if defined(PCBSKY) || defined(PCB9XT)
				init_eeprom() ;
#endif
				eeReadAll() ;
				createSwitchMapping() ;
#if defined(PCBSKY) || defined(PCB9XT)
				create6posTable() ;
#endif 
			}
		break ;
		
		case ERESTORE_DONE :
			if ( EepromErrorText == NULL)
			{
				lcd_puts_Pleft( 4*FH, XPSTR("\004(EEPROM Restored)") ) ;
			}
			else
			{
				lcd_puts_Pleft( 4*FH, EepromErrorText ) ;
			}
		break ;
	}
}



#if defined(PCBX12D) || defined(PCBX10)
void menuColour( uint8_t event, uint8_t mode )
{
	uint32_t red ;
	uint32_t green ;
	uint32_t blue ;
	uint16_t colour ;

	if ( mode )
	{
		TITLE(XPSTR("Text Colour"));
		colour = LcdForeground ;
	}
	else
	{
		TITLE(XPSTR("Background Colour"));
		colour = LcdBackground ;
	}
	static MState2 mstate2 ;
	mstate2.check_columns(event, 2 ) ;

  int8_t  sub    = mstate2.m_posVert ;
	red = colour >> 11 ;
	green = ( colour >> 6 ) & 0x1F ;
	blue = colour & 0x1F ;

	uint8_t y = 2*FH;
	uint8_t blink = InverseBlink ;
	uint8_t subN = 0 ;

	lcd_puts_Pleft( y, XPSTR("Red")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, red,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( red, 31 ) ;
	y += FH ;
	subN += 1 ;
	 
	lcd_puts_Pleft( y, XPSTR("Green")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, green,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( green, 31 ) ;
	y += FH ;
	subN += 1 ;
	
	lcd_puts_Pleft( y, XPSTR("Blue")) ;
 	lcd_outdezAtt(PARAM_OFS+2*FW, y, blue,(sub==subN ? blink : 0) );
  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( blue, 31 ) ;
//	y += FH ;
//	subN += 1 ;

	if ( mode == 0 )
	{
		if ( (red < 6) && (green < 6) && (blue < 6) )
		{
			red = 6 ;
			green = 6 ;
			blue = 6 ;
		}
	}

	colour = ( red << 11 ) | ( green << 6 ) | ( blue ) ;
	
	if ( mode )
	{
		g_eeGeneral.textColour = LcdForeground = colour ;
	}
	else
	{
		g_eeGeneral.backgroundColour = LcdBackground = colour ;
	}
}

void menuBackground( uint8_t event )
{
	menuColour( event, 0 ) ;	
}

void menuForeground( uint8_t event )
{
	menuColour( event, 1 ) ;	
}

#endif

const char Str_General[] =     "General" ;
			
//#define M_INDEX			0
//#define M_DISPLAY		1
//#define M_AUDIO			2
//#define M_ALARMS		3
//#define M_GENERAL		4
//#define M_CONTROLS	5
//#define M_HARDWARE	6
//#define M_CALIB			7
//#define M_BLUETOOTH	8
//#define M_TRAINER		9
//#define M_VERSION		10
//#define M_MODULE		11
//#define M_EEPROM		12
//#define M_DATE			13
//#define M_DIAGKEYS	14
//#define M_DIAGANA		15

enum GENERAL_INDEX
{
	M_INDEX,
	M_DISPLAY,
	M_AUDIO,
	M_ALARMS,
	M_GENERAL,
	M_CONTROLS,
	M_HARDWARE,
	M_CALIB,
#ifdef BLUETOOTH
	M_BLUETOOTH,
#endif
	M_TRAINER,
	M_VERSION,
#ifndef PCBXLITE
 #ifndef PCBT12
  #ifndef PCBX9LITE
   #ifndef PCBX12D
    #ifndef PCBX10
     #ifndef PCBLEM1
	M_MODULE,
     #endif // PCBLEM1
    #endif // X10
   #endif // X12
  #endif // X9Lite
 #endif
#endif
	M_EEPROM,
	M_DATE,
	M_DIAGKEYS,
	M_DIAGANA,
	M_COUNT
} ;


#if defined(PCBX12D) || defined(PCBX10)

//#define LARGE_INDEX 1

const uint8_t *m_icons[] = {
IconMix,
IconHeli,
IconLimits,
IconDrate,
0,
IconCurve,
IconLswitch,
IconMusic,
0,
0,
IconTelem,
IconVoice,
IconTimer,
0,
IconProto
};
	
const uint8_t *g_icons[] = {
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
};


void attachIcon( uint32_t x, uint32_t y, const uint8_t *icon )
{
	uint32_t w ;
	uint32_t h ;
	w = *icon ;
	h = *(icon+1) ;
	x = 2*x - (w+4) ;
	y = 2*y - (h/2-8) ;
	lcdDrawIcon( x, y, icon, 0 ) ;
}


static void displayIndexIcons( const uint8_t *icons[], uint8_t extra, uint8_t lines )
{
	uint8_t offset = 7 ;
	if ( extra == 8 )
	{
		offset = 8 ;
		if ( icons[7] )
		{
			attachIcon( 89 + X12OFFSET, 10, icons[7] ) ;
		}
	}
	for ( uint8_t i = 0 ; i < lines ; i += 1 )
	{
		if ( icons[i] )
		{
			attachIcon( 1 + X12OFFSET, (i+1)*(14)+10, icons[i] ) ;
		}
		if ( i < extra )
		{
			if ( icons[i+offset] )
			{
				attachIcon( 89 + X12OFFSET, (i+1)*(14)+10, icons[i+offset] ) ;
			}
		}
	} 
}

static void displayIndex( const uint16_t *strings, uint8_t extra, uint8_t lines, uint8_t highlight )
{
	uint8_t offset = 7 ;
#ifdef LARGE_INDEX	
//	if ( extra == 8 )
//	{
//		offset = 8 ;

//		lcd_putsAtt( 122, 9, PSTR(strings[7]), DBLSIZE | CONDENSED ) ;
//	}
//	for ( uint8_t i = 0 ; i < lines ; i += 1 )
//	{
//		lcd_putsAtt( 32, i*13+22, PSTR(strings[i]), DBLSIZE | CONDENSED ) ;
//		if ( i < extra )
//		{
//			lcd_putsAtt( 122, i*13+22, PSTR(strings[i+offset]), DBLSIZE | CONDENSED ) ;
//		}
//	} 
	
//	pushPlotType( PLOT_BLACK ) ;
//	lcd_vline( 120, 10, 104 ) ;
//	popPlotType() ;

//	if ( highlight )
//	{
//		if ( highlight > 7 )
//		{
//			lcd_char_inverse( 122, (highlight-offset)*13+9, 80, 0, 13 ) ;
//		}
//		else
//		{
//			lcd_char_inverse( 32, (highlight)*13+9, 87, 0, 13 ) ;
//		}
//	}
	
#else
	if ( extra == 8 )
	{
		offset = 8 ;
		
		lcd_puts_P( 89 + X12OFFSET, 10, PSTR(strings[7]) ) ;
	}
	for ( uint8_t i = 0 ; i < lines ; i += 1 )
	{
		lcd_puts_P( 1 + X12OFFSET, (i+1)*(14)+10, PSTR(strings[i]) ) ;
		if ( i < extra )
		{
			lcd_puts_P( 89 + X12OFFSET, (i+1)*(14)+10, PSTR(strings[i+offset]) ) ;
		}
	} 
	
	pushPlotType( PLOT_BLACK ) ;
	lcd_vline( 67 + X12OFFSET, 10, 104 ) ;
	popPlotType() ;

	if ( highlight )
	{
		if ( highlight > 7 )
		{
			lcd_char_inverse( 88 + X12OFFSET, (highlight-offset)*(14)+10, 60, 0 ) ;
		}
		else
		{
			lcd_char_inverse( 0 + X12OFFSET, (highlight)*(14)+10, 67, 0 ) ;
		}
	}
#endif
}
#else
static void displayIndex( const uint16_t *strings, uint8_t extra, uint8_t lines, uint8_t highlight )
{
	uint8_t offset = 7 ;
	if ( extra == 8 )
	{
		offset = 8 ;
		
		lcd_puts_P( 69 + X12OFFSET, 0, PSTR(strings[7]) ) ;
	}
	for ( uint8_t i = 0 ; i < lines ; i += 1 )
	{
		lcd_puts_P( 1 + X12OFFSET, (i+1)*FH, PSTR(strings[i]) ) ;
		if ( i < extra )
		{
			lcd_puts_P( 69 + X12OFFSET, (i+1)*FH, PSTR(strings[i+offset]) ) ;
		}
	} 
	
	lcd_vline( 67 + X12OFFSET, 0, 63 ) ;

	if ( highlight )
	{
		if ( highlight > 7 )
		{
			lcd_char_inverse( 68 + X12OFFSET, (highlight-offset)*FH, 60, 0 ) ;
		}
		else
		{
			lcd_char_inverse( 0 + X12OFFSET, highlight*FH, 67, 0 ) ;
		}
	}
}
#endif

static uint8_t SubMenuCall = 0 ;

static uint8_t indexProcess( uint8_t event, MState2 *pmstate, uint8_t extra )
{
	if (event == EVT_ENTRY)
	{
		pmstate->m_posVert = SubmenuIndex - 1 ;
		SubmenuIndex = 0 ;
	}
	if (event == EVT_ENTRY_UP)
	{
		SingleExpoChan = 0 ;
		pmstate->m_posVert = SubmenuIndex - 1 ;
		if ( SubMenuCall )
		{
			pmstate->m_posVert = SubMenuCall & 0x1F ;
			g_posHorz = ( SubMenuCall >> 5 ) & 3 ;
			SubMenuCall = 0 ;
		}
		else
		{
			SubmenuIndex = 0 ;
		}
	}
	
	if ( UseLastSubmenuIndex )
	{
		SubmenuIndex = LastSubmenuIndex & 0x7F ;
		UseLastSubmenuIndex = 0 ;
	}
	
	if ( SubmenuIndex )
	{
  	if ( ( checkForExitEncoderLong( event ) ) && ( MaskRotaryLong == 0 ) )
		{
      if(s_editMode)
			{
        s_editMode = false;
			}
			else
			{
				pmstate->m_posVert = SubmenuIndex - 1 ;
				SubmenuIndex = 0 ;
				killEvents(event) ;
				audioDefevent(AU_MENUS) ;
			}
			event = 0 ;
		}
	}
	else
	{
		uint8_t pv = pmstate->m_posVert ;
		if (event == EVT_KEY_FIRST(KEY_RIGHT) )
		{
			if ( pv < ((extra == 8) ? 7 : extra) )
			{
				pv += (extra == 8) ? 8 : 7 ;
			}
		}
		if (event == EVT_KEY_FIRST(KEY_LEFT) )
		{
			if ( pv >= ((extra == 8) ? 8 : 7) )
			{
				pv -= (extra == 8) ? 8 : 7 ;
			}
		}

 		if ( (event == EVT_KEY_FIRST(KEY_MENU) ) ||(event == EVT_KEY_BREAK(BTN_RE) ) )
		{
			SubmenuIndex = pv + 1 ;
			LastSubmenuIndex = SubmenuIndex | 0x80 ;
			pv = 0 ;
			killEvents(event) ;
			event = EVT_ENTRY ;
			Tevent = EVT_ENTRY ;
			g_posHorz = 0 ;
		}
		pmstate->m_posVert = pv ;
	}
	return event ;
}

//#if defined(PCBSKY) || defined(PCBLEM1)
#if defined(PCBSKY)
void edit_stick_gain(uint8_t x, uint8_t y, const char *s, uint8_t stick_b, uint8_t edit)
{
#ifndef ARUNI
  lcd_puts_Pleft( y,PSTR(STR_STICK_GAIN));
#endif
	lcd_puts_P( x*FW, y, s);
	uint8_t lastValue = g_eeGeneral.stickGain ;
	uint8_t b = g_eeGeneral.stickGain & stick_b ? 1 : 0 ;
 	lcd_putcAtt( (x+3)*FW, y, b ? '2' : '1', edit ? BLINK:0 );
 	if(edit)
	{
		CHECK_INCDEC_H_GENVAR_0( b,1);
		g_eeGeneral.stickGain = (lastValue & ~stick_b) | (b ? stick_b : 0 ) ;
	}
}
#endif

void edit_stick_deadband(uint8_t x, uint8_t y, const char *s, uint8_t ch, uint8_t edit)
{
	uint8_t attr = 0 ;
	if(edit)
	{
    attr = InverseBlink ;
    CHECK_INCDEC_H_MODELVAR_0( g_eeGeneral.stickDeadband[ch], 15 ) ;
  }
#ifndef ARUNI
  lcd_puts_Pleft( y,PSTR(STR_STICK_DEADBAND));
#endif
	lcd_puts_P( x*FW, y, s);
	lcd_outdezAtt( 20*FW, y, g_eeGeneral.stickDeadband[ch], attr) ;
}

#ifdef PCBSKY
void edit_xpot_source(uint8_t y, const char *s, uint8_t i, uint8_t a)
{
	lcd_puts_Pleft( y, s );
	g_eeGeneral.extraPotsSource[i] = checkIndexed(y,
      XPSTR(FWx17"\002\004NONEAD10AD8 "), g_eeGeneral.extraPotsSource[i], a);
#ifdef ARUNI	
	NumExtraPots = NUM_EXTRA_POTS + countExtraPots(&ExtraPotBits);
#else
	NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
#endif
}
#endif



uint8_t edit3posSwitchSource( uint8_t y, uint8_t value, uint16_t mask, uint8_t condition, uint8_t allowAna )
{
	uint16_t sm = g_eeGeneral.switchMapping ;
#ifdef REVX
	value = checkIndexed( y, XPSTR(FWx17"\005\004NONELCD2LCD6LCD7DAC1ELE "), value, condition ) ;
#else
#ifdef PCB9XT
	value = checkIndexed( y, XPSTR(FWx17"\010\004NONEEXT1EXT2EXT3EXT4EXT5EXT6EXT7EXT8"), value, condition ) ;
#else
#ifdef PCBX9D
 #ifdef PCBX7
	value = checkIndexed( y, XPSTR(FWx17"\004\004NONEJTMSJTCKEXT1EXT2"), value, condition ) ;
 #else
  #ifdef PCBX9LITE
	value = checkIndexed( y, XPSTR(FWx17"\005\004NONEJTMSJTCKPD14PC2 PC3 "), value, condition ) ;
  #else
	value = checkIndexed( y, XPSTR(FWx17"\002\004NONEJTMSJTCK"), value, condition ) ;
  #endif
 #endif
#else
	uint8_t map[12] = { 0,1,2,3,4,5,6,7,8,10,9 } ;
	uint32_t count = 11 ;
	if ( g_eeGeneral.ar9xBoard == 0 )
	{
		if ( allowAna == 0 )
		{	// EXT4,5,6 come from SKY coprocessor
			count = 10 ;

//			value = checkIndexed( y, XPSTR(FWx17"\011\004NONELCD2LCD6LCD7DAC1ELE EXT4EXT5EXT6ANA PA25"), value, condition ) ;
		}
//		else
//		{
//			value = checkIndexed( y, XPSTR(FWx17"\010\004NONELCD2LCD6LCD7DAC1ELE EXT4EXT5EXT6"), value, condition ) ;
//		}
		lcd_putsAttIdx( 17*FW, y, XPSTR("\004NONELCD2LCD6LCD7DAC1ELE EXT4EXT5EXT6ANA PA25"), value, condition ? InverseBlink: 0 ) ;

	}
	else
	{
		count = 8 ;
		map[6] = 7 ;
		map[7] = 6 ;
		if ( allowAna == 0 )
		{
			count = 7 ;
//			if ( g_eeGeneral.ar9xBoard == 0 )
//			{			
//				value = checkIndexed( y, XPSTR(FWx17"\006\004NONEEXT1EXT2EXT3PB14ELE ANA "), value, condition ) ;
//			}
//			else
//			{
//				value = checkIndexed( y, XPSTR(FWx17"\006\004NONEEXT1EXT2EXT3PB14ELE AD10PA25"), value, condition ) ;
//			}
		}
//		else
//		{
//			value = checkIndexed( y, XPSTR(FWx17"\005\004NONEEXT1EXT2EXT3PB14ELE PA25"), value, condition ) ;
//		}
		lcd_putsAttIdx( 17*FW, y, XPSTR("\004NONEEXT1EXT2EXT3PB14ELE AD10PA25"), value, condition ? InverseBlink: 0 ) ;
	}
	if ( condition )
	{
		value = checkOutOfOrder( value, map, count ) ;
	}
#endif
#endif
#endif
	g_eeGeneral.switchMapping = sm & ~(mask) ;
	if ( value )
	{
		g_eeGeneral.switchMapping |= mask ;
	} 
	if ( sm != g_eeGeneral.switchMapping )
	{
		createSwitchMapping() ;
	}
	return value ;
}

#ifdef PCB9XT
void editExtraPot( uint8_t y, uint32_t index, uint32_t active )
{
	lcd_puts_Pleft( y, index ? XPSTR( "POT5" ) : XPSTR( "POT4") ) ;
	uint8_t value = g_eeGeneral.extraPotsSource[index] ;
	value = checkIndexed( y, XPSTR(FWx17"\004\004NONEAIP1AIP2AIP3AIP4"), value, active ) ;
	g_eeGeneral.extraPotsSource[index] = value ;
	NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
}
#endif	// PCB9XT

#if defined(X9LS) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
//#if defined(X9LS)
const uint8_t BtFunctionMap[] = { 0,4,5,6 } ;
#else
const uint8_t BtFunctionMap[] = { 0,4,1,2,3,5,6 } ;
#endif

void menuProcIndex(uint8_t event)
{
	static MState2 mstate;
	EditType = EE_GENERAL ;

	event = indexProcess( event, &mstate, M_COUNT-1-7 ) ;

//#if defined(PCBT12)
//	event = indexProcess( event, &mstate, 6 ) ;
//#else
// #if defined (PCBXLITE)
//	event = indexProcess( event, &mstate, 7 ) ;
// #else
//	event = indexProcess( event, &mstate, 8 ) ;
// #endif		 
//#endif		 
	event = mstate.check_columns( event, IlinesCount-1 ) ;

		switch ( SubmenuIndex )
		{
			case M_CALIB :
        pushMenu(menuProcDiagCalib) ;
			break ;
			case M_TRAINER :
        pushMenu(menuProcTrainer) ;
			break ;
			case M_VERSION :
        pushMenu(menuProcDiagVers) ;
			break ;
			case M_DATE :
        pushMenu(menuProcDate) ;
			break ;
#ifndef PCBXLITE	
 #ifndef PCBT12
  #ifndef PCBX9LITE
   #ifndef PCBX12D
    #ifndef PCBX10
      #ifndef PCBLEM1
			case M_MODULE :
        pushMenu(menuProcRSSI) ;
			break ;
     #endif
    #endif
   #endif
  #endif // X3
 #endif
#endif
			case M_DIAGKEYS :
        pushMenu(menuProcDiagKeys) ;
			break ;
			case M_DIAGANA :
        pushMenu(menuProcDiagAna) ;
			break ;
			default :
				if ( event == EVT_ENTRY )
				{
					audioDefevent(AU_MENUS) ;
					event = 0 ;
				}
			break ;
		}

	uint32_t sub = mstate.m_posVert ;
	uint32_t y = FH ;
	uint8_t blink = InverseBlink ;

	switch ( SubmenuIndex )
	{
		case 0 :
#if defined(PCBX12D) || defined(PCBX10)
//			lcd_putsAtt(0 + X12OFFSET-33,0,"Radio Setup",INVERS|DBLSIZE|CONDENSED) ;
			lcd_putsAtt(0 + X12OFFSET+36,0,"Radio Setup",INVERS) ;
#else
			lcd_putsAtt(0 + X12OFFSET,0,"Radio Setup",INVERS) ;
#endif
			IlinesCount = M_COUNT-1 ;

//#ifdef PCBT12
//			IlinesCount = 13 ;
//#else
// #ifdef PCBXLITE	 
//			IlinesCount = 14 ;
// #else
//			IlinesCount = 15 ;
// #endif
//#endif
			sub += 1 ;
			
			
static const uint16_t in_Strings[] = {
STR_Display,
STR_AudioHaptic,
STR_Alarms,
STR_General,
STR_Controls,
STR_Hardware,
STR_Calibration,
#ifdef BLUETOOTH
STR_Bluetooth,
#endif
STR_Trainer,
STR_Version,
#ifndef PCBXLITE	
 #ifndef PCBT12
  #ifndef PCBX9LITE
   #ifndef PCBX12D
    #ifndef PCBX10
     #ifndef PCBLEM1
STR_ModuleRssi,
     #endif // PCBLEM1
    #endif // X1
   #endif // X12
  #endif // X3
 #endif
#endif
STR_Eeprom,
STR_DateTime,
STR_DiagSwtch,
STR_DiagAna
};
	

			displayIndex( in_Strings, M_COUNT-1-7, 7, sub ) ;
#if defined(PCBX12D) || defined(PCBX10)
			displayIndexIcons( g_icons, M_COUNT-1-7, 7 ) ;
#endif

//#if defined(PCBT12)
//			displayIndex( in_Strings, 6, 7, sub ) ;
//#else
// #if defined (PCBXLITE)
//			displayIndex( in_Strings, 7, 7, sub ) ;
// #else
//			displayIndex( in_Strings, 8, 7, sub ) ;
// #endif		 
//#endif		 
		break ;
		
		case M_DISPLAY :
		{	
			uint8_t subN = 0 ;
			
      TITLE( PSTR(STR_Display) ) ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
#if defined(REVPLUS) || defined(REV9E)
			IlinesCount = 7 ;
 #else            
  #if defined(PCBX12D) || defined(PCBX10)
			IlinesCount = 8 ;
  #else            
			IlinesCount = 6 ;
  #endif            
 #endif            
#else            
 #ifdef PCBSKY
 			y = 0 ;
  #ifndef REVX
			IlinesCount = 8 ;
			if ( g_eeGeneral.ar9xBoard )
			{
				IlinesCount -= 1 ;
			}
  #else            
			IlinesCount = 8 ;
  #endif            
 #endif            
#endif            
#ifdef PCBLEM1
			IlinesCount = 6 ;
#endif            
#ifdef PCB9XT
 			y = 0 ;
			IlinesCount = 9 ;
		 
		 if ( sub < 7 )
		 {
			displayNext() ;
		 	
#endif            
  		uint8_t attr ;
			attr = (sub==subN) ? blink : 0 ;
      if( attr )
			{
				uint8_t oldValue = g_eeGeneral.contrast ;
#if defined(REVPLUS) || defined(REV9E)
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, 0, 60);
#else
 #if defined(PCBX7) || defined(PCBX9LITE)
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, 10, 42);
 #else
  #ifdef PCBLEM1
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, 5, 20);
	#else
				CHECK_INCDEC_H_GENVAR( g_eeGeneral.contrast, 10, 45);
  #endif
 #endif
#endif
				if ( g_eeGeneral.contrast != oldValue )
				{
					lcdSetRefVolt(g_eeGeneral.contrast);
				}
      }
#if defined(PCBSKY) || defined(PCB9XT)
      lcd_puts_P( 8*FW, y, PSTR(STR_CONTRAST) ) ;
#else
      lcd_puts_Pleft( y, PSTR(STR_CONTRAST) ) ;
#endif
      lcd_outdezAtt( PARAM_OFS+2*FW, y, g_eeGeneral.contrast, (sub==subN) ? blink : 0) ;
			y += FH ;
			subN += 1 ;
  		
			lcd_puts_Pleft( y, PSTR(STR_BRIGHTNESS)) ;
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
#if defined(REVPLUS) || defined(REV9E)
		  lcd_puts_P(11*FW, y, XPSTR( "COLOUR"));
#endif
#endif
#ifdef PCB9XT
		  lcd_puts_P(11*FW, y, XPSTR( "RED"));
			uint8_t oldBrightness = g_eeGeneral.bright ;
			int8_t b ;
			b = g_eeGeneral.bright ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			attr = 0 ;
      if(sub==subN)
			{
				attr = INVERS ;
			}
			b = gvarMenuItem(PARAM_OFS+2*FW, y, b, 0, 100, attr | GVAR_100, event ) ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			g_eeGeneral.bright = b ;

#else
			 
			lcd_outdezAtt( PARAM_OFS+2*FW, y, 100-g_eeGeneral.bright, (sub==subN) ? blink : 0 ) ;
      if(sub==subN)
			{
				uint8_t b ;
				b = 100 - g_eeGeneral.bright ;
				CHECK_INCDEC_H_GENVAR_0( b, 100 ) ;
				g_eeGeneral.bright = 100 - b ;
#endif
#ifdef PCBSKY
				PWM->PWM_CH_NUM[0].PWM_CDTYUPD = g_eeGeneral.bright ;
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
#if defined(REVPLUS) || defined(REV9E) || defined(REV19)
				backlight_set( g_eeGeneral.bright, 0 ) ;
#else
				backlight_set( g_eeGeneral.bright ) ;
#endif
#endif
#ifdef PCB9XT
				if ( oldBrightness != g_eeGeneral.bright )
				{
					uint8_t b ;
					b = g_eeGeneral.bright ;
					if ( b < 101 )
					{
						b = 100 - b ;
					}
					BlSetColour( b, 0 ) ;
				}
#else
			}
#endif
			y += FH ;
			subN += 1 ;

#if defined(REVPLUS) || defined(PCB9XT) || defined(REV9E)
#ifndef PCB9XT
 			lcd_puts_Pleft(    y, PSTR(STR_BRIGHTNESS));
#endif
#ifdef PCB9XT
   		lcd_puts_P(11*FW, y, XPSTR( "GREEN"));
			oldBrightness = g_eeGeneral.bright_white ;
			b = g_eeGeneral.bright_white ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			attr = 0 ;
      if(sub==subN)
			{
				attr = INVERS ;
			}
			b = gvarMenuItem(PARAM_OFS+2*FW, y, b, 0, 100, attr | GVAR_100, event ) ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			g_eeGeneral.bright_white = b ;
#else
   		lcd_puts_P(11*FW, y, XPSTR( "WHITE"));
			lcd_outdezAtt( PARAM_OFS+2*FW, y, 100-g_eeGeneral.bright_white, (sub==subN) ? blink : 0 ) ;
      if(sub==subN)
			{
				uint8_t b ;
				b = 100 - g_eeGeneral.bright_white ;
				CHECK_INCDEC_H_GENVAR_0( b, 100 ) ;
				g_eeGeneral.bright_white = 100 - b ;
#endif
#ifdef PCB9XT
				if ( oldBrightness != g_eeGeneral.bright_white )
				{
					uint8_t b ;
					b = g_eeGeneral.bright_white ;
					if ( b < 101 )
					{
						b = 100 - b ;
					}
					BlSetColour( b, 1 ) ;
				}
#else
				backlight_set( g_eeGeneral.bright_white, 1 ) ;
			}
#endif
			y += FH ;
			subN++;
#endif
#ifdef PCB9XT
   		lcd_puts_P( 11*FW, y, XPSTR( "BLUE"));
			oldBrightness = g_eeGeneral.bright_blue ;
			b = g_eeGeneral.bright_blue ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			attr = 0 ;
      if(sub==subN)
			{
				attr = INVERS ;
			}
			b = gvarMenuItem(PARAM_OFS+2*FW, y, b, 0, 100, attr | GVAR_100, event ) ;
			if ( b < 101 )
			{
				b = 100 - b ;
			}
			g_eeGeneral.bright_blue = b ;
			if ( oldBrightness != g_eeGeneral.bright_blue )
			{
				uint8_t b ;
				b = g_eeGeneral.bright_blue ;
				if ( b < 101 )
				{
					b = 100 - b ;
				}
				BlSetColour( b, 2 ) ;
			}
			y += FH ;
			subN++;
#endif
      
			lcd_puts_Pleft( y,PSTR(STR_LIGHT_SWITCH));
      putsDrSwitches(PARAM_OFS-FW,y,g_eeGeneral.lightSw,sub==subN ? blink : 0);
      if(sub==subN) CHECK_INCDEC_GENERALSWITCH(event, g_eeGeneral.lightSw, -MaxSwitchIndex, MaxSwitchIndex) ;
			
//#ifdef PCBX12D
//			lcd_outhex4( 22*FW, 5*FH, BL_TIMER->CCR4 ) ;
//			lcd_outhex4( 22*FW, 6*FH, BL_TIMER->CNT ) ;
//#endif
			
			y += FH ;
			subN++;

			for ( uint8_t i = 0 ; i < 2 ; i += 1 )
			{
				uint8_t b ;
    	  lcd_puts_Pleft( y,( i == 0) ? PSTR(STR_LIGHT_AFTER) : PSTR(STR_LIGHT_STICK) );
				b = ( i == 0 ) ? g_eeGeneral.lightAutoOff : g_eeGeneral.lightOnStickMove ;

    	  if(b) {
    	      lcd_outdezAtt(PARAM_OFS, y, b*5,LEFT|(sub==subN ? blink : 0));
    	      lcd_putc(Lcd_lastPos, y, 's');
    	  }
    	  else
    	      lcd_putsAtt(PARAM_OFS, y, PSTR(STR_OFF),(sub==subN ? blink:0));
    	  if(sub==subN) CHECK_INCDEC_H_GENVAR_0( b, 600/5);
				if ( i == 0 )
				{
					g_eeGeneral.lightAutoOff = b ;
				}
				else
				{
					g_eeGeneral.lightOnStickMove = b ;
				}
  			y += FH ;
				subN += 1 ;
			}
		
#ifdef PCBSKY
#ifndef REVX
			if ( g_eeGeneral.ar9xBoard == 0 )
			{
		  	g_eeGeneral.optrexDisplay = onoffMenuItem( g_eeGeneral.optrexDisplay, y, XPSTR("Optrex Display"), sub==subN ) ;
  			y += FH ;
				subN += 1 ;
			}
#endif
#endif

#ifndef PCB9XT
			g_eeGeneral.flashBeep = onoffMenuItem( g_eeGeneral.flashBeep, y, PSTR(STR_FLASH_ON_BEEP), sub==subN ) ;
  		y += FH ;
			subN += 1 ;
#endif
#ifdef PCBSKY
			{
				uint8_t b = g_eeGeneral.rotateScreen ;
	  		g_eeGeneral.rotateScreen = onoffMenuItem( b, y, PSTR(STR_ROTATE_SCREEN), sub==subN ) ;
				if ( g_eeGeneral.rotateScreen != b )
				{
					lcdSetOrientation() ;
				}
#ifdef REVX
	  		y += FH ;
				subN += 1 ;
				b = g_eeGeneral.reverseScreen ;
	  		g_eeGeneral.reverseScreen = onoffMenuItem( b, y, PSTR(STR_REVERSE_SCREEN), sub==subN ) ;
				if ( g_eeGeneral.reverseScreen != b )
				{
					lcdSetOrientation() ;
				}
#endif
			}
#endif
#ifdef PCB9XT
		 }
		 else
		 {
				int8_t b ;
			 	subN = 7 ;
				y = FH ;
				g_eeGeneral.flashBeep = onoffMenuItem( g_eeGeneral.flashBeep, y, PSTR(STR_FLASH_ON_BEEP), sub==subN ) ;
  			y += FH ;
				subN += 1 ;
				b = g_eeGeneral.reverseScreen ;
	  		g_eeGeneral.reverseScreen = onoffMenuItem( b, y, PSTR(STR_REVERSE_SCREEN), sub==subN ) ;
				if ( g_eeGeneral.reverseScreen != b )
				{
					lcdSetOrientation() ;
				}
		 }
#endif
#if defined(PCBX12D) || defined(PCBX10)
			attr = (sub==subN) ? blink : 0 ;
			lcd_putsAtt( 0, y, "Background Colour", attr ) ;
      if( sub==subN )
			{
				if ( checkForMenuEncoderBreak( Tevent ) )
				{
					SubMenuCall = 0x86 ;
					pushMenu( menuBackground ) ;
				}
			}
  		y += FH ;
			subN += 1 ;
			attr = (sub==subN) ? blink : 0 ;
			lcd_putsAtt( 0, y, "Text Colour", attr ) ;
      if( sub==subN )
			{
				if ( checkForMenuEncoderBreak( Tevent ) )
				{
					SubMenuCall = 0x87 ;
					pushMenu( menuForeground ) ;
				}
			}
#endif
			 
		}
		break ;
		
		case M_AUDIO :
		{	
			uint8_t current_volume ;
			uint8_t subN = 0 ;
#if defined(PCBX12D) || defined(PCBX10)
 #ifdef HAPTIC
			IlinesCount = g_eeGeneral.welcomeType == 2 ? 11 : 10 ;
 #else
			IlinesCount = g_eeGeneral.welcomeType == 2 ? 9 : 8 ;
 #endif      
#else
 #ifdef HAPTIC
			IlinesCount = g_eeGeneral.welcomeType == 2 ? 10 : 9 ;
 #else
			IlinesCount = g_eeGeneral.welcomeType == 2 ? 8 : 7 ;
 #endif      
#endif      
			TITLE( PSTR(STR_AudioHaptic) ) ;
  		
#ifndef BIG_SCREEN
#ifdef HAPTIC
			if ( sub < 6 )
#else
			if ( sub < 4 )
#endif
			{
				displayNext() ;
#endif

				lcd_puts_Pleft( y, PSTR(STR_VOLUME));
				current_volume = g_eeGeneral.volume ;
				lcd_outdezAtt( PARAM_OFS+2*FW, y, current_volume, (sub==subN) ? blink : 0 ) ;
  			if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( current_volume,NUM_VOL_LEVELS-1);
					if ( current_volume != g_eeGeneral.volume )
					{
						setVolume( g_eeGeneral.volume = current_volume ) ;
					}
				}
  			y += FH ;
				subN += 1 ;
      
	    	uint8_t b ;
  	  	b = g_eeGeneral.beeperVal ;
      	lcd_puts_Pleft( y,PSTR(STR_BEEPER));
      	lcd_putsAttIdx(PARAM_OFS - FW - 4, y, PSTR(STR_BEEP_MODES),b,(sub==subN ? blink:0));
	    	if(sub==subN) { CHECK_INCDEC_H_GENVAR_0( b, 6); g_eeGeneral.beeperVal = b ; }
  			y += FH ;
				subN += 1 ;

		// audio start by rob
      	lcd_puts_P(0, y,PSTR(STR_SPEAKER_PITCH));
      	lcd_outdezAtt(PARAM_OFS+2*FW,y,g_eeGeneral.speakerPitch,(sub==subN ? blink : 0) );
      	if(sub==subN) CHECK_INCDEC_H_GENVAR( g_eeGeneral.speakerPitch, 1, 100) ;
  			y += FH ;
				subN += 1 ;
	 
#ifdef HAPTIC
      	lcd_puts_P(0, y,PSTR(STR_HAPTICSTRENGTH));
      	lcd_outdezAtt(PARAM_OFS+2*FW,y,g_eeGeneral.hapticStrength,(sub==subN ? blink : 0) );
      	if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.hapticStrength, 5) ;
  			y += FH ;
				subN += 1 ;

      	lcd_puts_P(0, y,XPSTR("Haptic Min Run"));
      	lcd_outdezAtt(PARAM_OFS+2*FW,y,g_eeGeneral.hapticMinRun+20,(sub==subN ? blink : 0) );
      	if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.hapticMinRun, 20 ) ;
  			y += FH ;
				subN += 1 ;
#endif

    		lcd_putsAtt(  0, y, XPSTR(GvaString), sub==subN ? INVERS : 0 ) ;
      	if( sub==subN )
				{
					if ( checkForMenuEncoderBreak( Tevent ) )
					{
						SubMenuCall = 0x85 ;
						pushMenu(menuProcGlobalVoiceAlarm) ;
					}
				}
#ifdef BIG_SCREEN
  			y += FH ;
				subN += 1 ;
#else
			}
			else
			{
#ifdef HAPTIC
			 	subN = 6 ;
#else
			 	subN = 4 ;
#endif
#endif
  	    g_eeGeneral.preBeep = onoffMenuItem( g_eeGeneral.preBeep, y, PSTR(STR_BEEP_COUNTDOWN), sub == subN ) ;
  			y += FH ;
				subN += 1 ;

	      g_eeGeneral.minuteBeep = onoffMenuItem( g_eeGeneral.minuteBeep, y, PSTR(STR_MINUTE_BEEP), sub == subN ) ;
  			y += FH ;
				subN += 1 ;

		 		lcd_puts_Pleft( y, XPSTR( "Welcome Type") ) ;
				g_eeGeneral.welcomeType = checkIndexed( y, XPSTR(FWx15"\002""\006System  NoneCustom"), g_eeGeneral.welcomeType, sub==subN ) ;
  			y += FH ;
				subN += 1 ;

				if ( g_eeGeneral.welcomeType == 2 )
				{
					// FileName
			 		lcd_puts_Pleft( y, XPSTR( "FileName") ) ;
    			uint8_t attr = sub == subN ? InverseBlink : 0 ;
					SubMenuCall = 0x89 ;
					alphaEditName( 10*FW-2, y, (uint8_t *)g_eeGeneral.welcomeFileName, sizeof(g_eeGeneral.welcomeFileName), (sub==subN) | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FileName") ) ;
					validateName( g_eeGeneral.welcomeFileName, sizeof(g_eeGeneral.welcomeFileName) ) ;
  				if( attr )
					{
						if ( checkForMenuEncoderLong( event ) )
						{
							VoiceFileType = VOICE_FILE_TYPE_USER ;
     				 	pushMenu( menuProcSelectVoiceFile ) ;
						}
						if ( event == EVT_ENTRY_UP )
						{
							if ( FileSelectResult == 1 )
							{
		 						copyFileName( (char *)g_eeGeneral.welcomeFileName, SelectedVoiceFileName, 8 ) ;
	   						eeDirty(EE_GENERAL) ;		// Save it
							}
						}
					} 
	  			y += FH ;
					subN += 1 ;
				}
#if defined(PCBX12D) || defined(PCBX10)
		 		lcd_puts_Pleft( y, XPSTR( "Hardware Volume") ) ;
				{
					uint8_t b ;
					uint16_t attr ;
					attr = 0 ;
					b = g_eeGeneral.unused_PPM_Multiplier ;
      		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.unused_PPM_Multiplier, 0, 127) ; }
	      	lcd_outdezAtt(PARAM_OFS+2*FW-2, y, g_eeGeneral.unused_PPM_Multiplier, attr);
//uint8_t I2C_read_volume() ;
//					lcd_outdez( PARAM_OFS+80, y, I2C_read_volume() ) ;
					if ( b != g_eeGeneral.unused_PPM_Multiplier )
					{
						b = g_eeGeneral.unused_PPM_Multiplier ;
void I2C_set_volume( register uint8_t volume ) ;
						I2C_set_volume( b ) ;
					}
				}
#endif
#ifndef BIG_SCREEN
			}
#endif
		}
		break ;

		case M_ALARMS	:
		{
			uint8_t subN = 0 ;
#ifdef PCBSKY
#define NUM_ALARMS			7
#else
#define NUM_ALARMS			6
#endif
			IlinesCount = NUM_ALARMS ;
			if ( g_eeGeneral.ar9xBoard )
			{
				IlinesCount -= 1 ;
			}

//	    if( g_eeGeneral.frskyinternalalarm == 1)
//			{
//				IlinesCount += 3 ;
//			}

  		uint8_t attr ;
			TITLE( PSTR(STR_Alarms) ) ;
			
//			if ( sub < NUM_ALARMS-1 )
//			{
//				displayNext() ;
  			attr = LEFT ;
      	lcd_puts_Pleft( y,PSTR(STR_BATT_WARN));
      	if(sub==subN) { attr = blink | LEFT ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.vBatWarn, 40, 120); } //5-10V
      	putsVolts(PARAM_OFS, y, g_eeGeneral.vBatWarn, attr);
  			y += FH ;
				subN += 1 ;

  			attr = 0 ;//LEFT ;
      	lcd_puts_Pleft( y,PSTR(STR_INACT_ALARM));
      	if(sub==subN) { attr = blink ; CHECK_INCDEC_H_GENVAR( g_eeGeneral.inactivityTimer, -10, 110); } //0..120minutes
      	lcd_outdezAtt(PARAM_OFS+2*FW-2, y, g_eeGeneral.inactivityTimer+10, attr);
  			y += FH ;
				subN += 1 ;

	  		lcd_puts_P( FW, y, PSTR(STR_VOLUME) ) ;
				int8_t value = g_eeGeneral.inactivityVolume + ( NUM_VOL_LEVELS-3 ) ;
				lcd_outdezAtt( PARAM_OFS+2*FW, y, value, (sub==subN) ? blink : 0 ) ;
	  		if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( value,NUM_VOL_LEVELS-1);
					g_eeGeneral.inactivityVolume = value - ( NUM_VOL_LEVELS-3 ) ;
				} 	
	  		y += FH ;
				subN += 1 ;
      
#ifdef PCBSKY
				if ( g_eeGeneral.ar9xBoard == 0 )
				{
  				lcd_puts_Pleft( y, PSTR(STR_CAPACITY_ALARM));
					lcd_outdezAtt( PARAM_OFS+2*FW, y, g_eeGeneral.mAh_alarm*50, (sub==subN) ? blink : 0 ) ;
  				if(sub==subN)
					{
						CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.mAh_alarm,100);
					}
 					y += FH ;
					subN += 1 ;
				}
#endif
		
      	uint8_t b = g_eeGeneral.disableThrottleWarning;
      	g_eeGeneral.disableThrottleWarning = offonMenuItem( b, y, PSTR(STR_THR_WARNING), sub == subN ) ;
 				y += FH ;
				subN += 1 ;

      	b = g_eeGeneral.disableAlarmWarning;
      	g_eeGeneral.disableAlarmWarning = offonMenuItem( b, y, PSTR(STR_ALARM_WARN), sub == subN ) ;
 				y += FH ;
				subN += 1 ;

      	b = g_eeGeneral.disableRxCheck;
      	g_eeGeneral.disableRxCheck = offonMenuItem( b, y, XPSTR("Receiver Warning"), sub == subN ) ;
 				y += FH ;
				subN += 1 ;

//			}
//			else
//			{
//				subN = NUM_ALARMS-1 ;
//      	g_eeGeneral.frskyinternalalarm = onoffMenuItem( g_eeGeneral.frskyinternalalarm, y, PSTR(STR_INT_FRSKY_ALRM), sub == subN ) ;
// 				y += FH ;
//				subN += 1 ;
				    
//	    	if( g_eeGeneral.frskyinternalalarm == 1)
//				{
//					for ( uint8_t i = 0 ; i < 3 ; i += 1 )
//					{
//						uint8_t b ;
					
//						b = g_eeGeneral.FRSkyYellow ;    // Done here to stop a compiler warning
								
//						if ( i == 0 )
//						{
//						  lcd_puts_P(0, y,PSTR(STR_ALERT_YEL));
//						}
//						else if ( i == 1 )
//						{
//						  b = g_eeGeneral.FRSkyOrange ;
//						  lcd_puts_P(0, y,PSTR(STR_ALERT_ORG));
//						}
//						else if ( i == 2 )
//						{
//						  b = g_eeGeneral.FRSkyRed ;
//						  lcd_puts_P(0, y,PSTR(STR_ALERT_RED));
//						}
//						lcd_putsAttIdx(PARAM_OFS - FW - 4, y, PSTR(STR_SOUNDS),b,(sub==subN ? blink:0));
//						if(sub==subN)
//						{
//							CHECK_INCDEC_H_GENVAR_0( b, 15);
//							if ( i == 0 )
//							{
//								g_eeGeneral.FRSkyYellow = b ;
//							}
//							else if ( i == 1 )
//							{
//								g_eeGeneral.FRSkyOrange = b ;
//							}
//							else if ( i == 2 )
//							{
//								g_eeGeneral.FRSkyRed = b ;
//							}
//							if (checkIncDec_Ret)
//							{
//								audio.event( b, 0, 1 ) ;
//							}
//						}
//				 		y += FH ;
//						subN += 1 ;
//					}
//				}			  
//			}
		}
		break ;

		case M_GENERAL :
		{
			uint8_t subN = 0 ;

// Add 1 to these if using _eeGeneral.disableBtnLong
#if defined(PCBSKY) || defined(PCB9XT)
 #ifdef PCBSKY
			IlinesCount = 10+1 ;
 #else
			IlinesCount = 9+1 ;
 #endif
#else
 #if defined(PCBX12D) || defined(PCBX10)
			IlinesCount = 8+1+1 ;
 #else
			IlinesCount = 8+1 ;
 #endif
#endif
			TITLE( XPSTR(Str_General) ) ;

//#if defined(PCBSKY) || defined(PCB9XT)
		 if ( sub < 6 )
		 {
			displayNext() ;
//#endif

			SubMenuCall = 0x80 ;
			alphaEditName( 10*FW-2, y, (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName), sub==subN, (uint8_t *)XPSTR( "Owner Name") ) ;
			validateName( (uint8_t *)g_eeGeneral.ownerName, sizeof(g_eeGeneral.ownerName) ) ;
 			y += FH ;
			subN += 1 ;

	    lcd_puts_Pleft( y,PSTR(STR_LANGUAGE));
#ifdef SMALL
	    lcd_putsAttIdx( 10*FW, y, XPSTR("\012   ENGLISH  FRANCAIS   DEUTSCH"),g_eeGeneral.language,(sub==subN ? blink:0));
	    if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.language, 2 ) ;
#else
	    lcd_putsAttIdx( 10*FW, y, XPSTR("\012   ENGLISH  FRANCAIS   DEUTSCH NORWEGIAN   SWEDISH   ITALIAN    POLISHVIETNAMESE   SPANISH"),g_eeGeneral.language,(sub==subN ? blink:0));
	    if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.language, 8 ) ;
#endif
			setLanguage() ;
 			y += FH ;
			subN += 1 ;

      uint8_t b = g_eeGeneral.disableSplashScreen;
      g_eeGeneral.disableSplashScreen = offonMenuItem( b, y, PSTR(STR_SPLASH_SCREEN), sub == subN ) ;
 			y += FH ;
			subN += 1 ;

      b = g_eeGeneral.hideNameOnSplash;
      g_eeGeneral.hideNameOnSplash = offonMenuItem( b, y, PSTR(STR_SPLASH_NAME), sub == subN ) ;
 			y += FH ;
			subN += 1 ;

      b = 1-g_eeGeneral.disablePotScroll ;
			b |= g_eeGeneral.stickScroll << 1 ;
      lcd_puts_Pleft( y,PSTR(STR_SCROLLING));
      lcd_putsAttIdx(PARAM_OFS-2*FW, y, XPSTR("\005 NONE  POTSTICK BOTH"),b,(sub==subN ? blink:0));
      if(sub==subN) CHECK_INCDEC_H_GENVAR_0( b, 3 ) ;
			g_eeGeneral.stickScroll = b >> 1 ;
			g_eeGeneral.disablePotScroll = 1 - ( b & 1 ) ;
 			y += FH ;
			subN += 1 ;

      b = g_eeGeneral.forceMenuEdit ;
      g_eeGeneral.forceMenuEdit = onoffMenuItem( b, y, PSTR(STR_MENU_ONLY_EDIT), sub == subN ) ;
 			y += FH ;
			subN += 1 ;

		 }
		 else
		 {
		 	subN = 6 ;

      g_eeGeneral.disableBtnLong = onoffMenuItem( g_eeGeneral.disableBtnLong, y, XPSTR("No ENC. as exit"), sub == subN ) ;
 			y += FH ;
			subN += 1 ;

#if defined(PCBSKY) || defined(PCB9XT)
  		lcd_puts_Pleft( y, PSTR(STR_BT_BAUDRATE));
  		lcd_putsAttIdx(  PARAM_OFS-3*FW, y, XPSTR(BtBaudString),g_eeGeneral.bt_baudrate,(sub==subN ? blink:0));
  		if(sub==subN)
			{
				CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.bt_baudrate,4);
			}
 			y += FH ;
			subN += 1 ;
#endif
  		lcd_puts_Pleft( y, XPSTR("GPS Format") ) ;
  		displayGPSformat( 11*FW, y, (sub==subN ? blink:0) ) ;
  		if(sub==subN)
			{
				CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.gpsFormat,1) ;
			}
 			y += FH ;
			subN += 1 ;

#ifdef PCBSKY
  		lcd_putsAtt(0, y, XPSTR("Run Maintenance"), (sub==subN) ? INVERS : 0 ) ;
  		if(sub==subN)
			{
				if ( event == EVT_KEY_LONG(KEY_MENU) )
				{
					GPBR->SYS_GPBR0 = 0x5555AAAA ;
					// save EEPROM and reboot
					prepareForShutdown() ;
  				uint16_t tgtime = get_tmr10ms() ;
	  			while( (uint16_t)(get_tmr10ms() - tgtime ) < 50 ) // 50 - Half second
					{
						wdt_reset() ;
						if ( ee32_check_finished() )
						{
							if ( read_keys() & 2 )
							{							
								break ;
							}
						}
  					tgtime = get_tmr10ms() ;
					}
		  		NVIC_SystemReset() ;
				}
			}
 			y += FH ;
			subN += 1 ;
#endif

      uint8_t b = g_eeGeneral.altMixMenu ;
      g_eeGeneral.altMixMenu = onoffMenuItem( b, y, XPSTR("Mix Menu Details"), sub == subN ) ;

#if defined(PCBX12D) || defined(PCBX10)
 			y += FH ;
			subN += 1 ;
			lcd_puts_Pleft( y,XPSTR("ScreenShot Sw"));
      putsDrSwitches(PARAM_OFS-FW,y,g_eeGeneral.screenShotSw,sub==subN ? blink : 0);
      if(sub==subN) CHECK_INCDEC_GENERALSWITCH(event, g_eeGeneral.screenShotSw, -MaxSwitchIndex, MaxSwitchIndex) ;
#endif

//#if defined(PCBSKY) || defined(PCB9XT)
  	 }	
//#endif

		}			 
		break ;

		case M_CONTROLS :
		{
			uint8_t subN = 0 ;
			IlinesCount = 8 ;
			TITLE( PSTR(STR_Controls) ) ;

#ifndef BIG_SCREEN
			if ( sub < 4 )
			{
				displayNext() ;
#endif
				uint8_t attr = sub==subN ? blink : 0 ;
	  		lcd_puts_Pleft( y, PSTR(STR_CHAN_ORDER) ) ;//   RAET->AETR
				uint8_t bch = bchout_ar[g_eeGeneral.templateSetup] ;
    		for ( uint8_t i = 4 ; i > 0 ; i -= 1 )
				{
					uint8_t letter ;
					letter = *(PSTR(STR_SP_RETA) +(bch & 3) + 1 ) ;
  				lcd_putcAtt( (16+i)*FW, y, letter, attr ) ;
					bch >>= 2 ;
				}
	  		if(attr) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.templateSetup, 23 ) ;
 				y += FH ;
				subN += 1 ;
      
				uint8_t ct = g_eeGeneral.crosstrim + ( g_eeGeneral.xcrosstrim << 1 ) ;
        lcd_puts_Pleft(y, PSTR(STR_CROSSTRIM));
				ct = checkIndexed( y, XPSTR(FWx17"\002\003OFFON Vtg"), ct, (sub==subN) ) ;
				g_eeGeneral.crosstrim = ct ;
				g_eeGeneral.xcrosstrim = ct >> 1 ;
//				g_eeGeneral.crosstrim = onoffMenuItem( g_eeGeneral.crosstrim, y, PSTR(STR_CROSSTRIM), sub == subN ) ;
 				y += FH ;
				subN += 1 ;

				uint8_t oldValue = g_eeGeneral.throttleReversed ;
				g_eeGeneral.throttleReversed = onoffMenuItem( oldValue, y, PSTR(STR_THR_REVERSE), sub == subN ) ;
				if ( g_eeGeneral.throttleReversed != oldValue )
				{
  				checkTHR() ;
				}
 				y += FH ;
				subN += 1 ;

	    	lcd_puts_Pleft( y, PSTR(STR_MODE) );
				for ( uint8_t i = 0 ; i < 4 ; i += 1 )
				{
#if defined(PCBX12D) || defined(PCBX10)
 					lcd_HiResimg( (6+4*i)*FW*2, y*2, sticksHiRes, i, 0 ) ;
#else
					lcd_img((6+4*i)*FW, y, sticks, i, 0 ) ;
#endif
				}
 				y += FH ;
    
 				attr = 0 ;
				uint8_t mode = g_eeGeneral.stickMode ;
      	if(sub==subN)
				{
					attr = INVERS ;
					if ( s_editMode )
					{
					 	attr = BLINK ;
						
						CHECK_INCDEC_H_GENVAR_0( mode,3);
						if ( mode != g_eeGeneral.stickMode )
						{
							g_eeGeneral.stickScroll = 0 ;
							g_eeGeneral.stickMode = mode ;							
						}
					}
				}
      	lcd_putcAtt( 3*FW, y, '1'+g_eeGeneral.stickMode,attr);
      	for(uint8_t i=0; i<4; i++) putsChnRaw( (6+4*i)*FW, y, modeFixValue( i ), 0 ) ;//sub==3?INVERS:0);
 				y += FH ;
				subN += 1 ;
#ifndef BIG_SCREEN
			}
			else
			{
				subN = 4 ;
#endif
				if ( sub >= IlinesCount )
				{
					sub = mstate.m_posVert = IlinesCount-1 ;
				}
				// Edit custom stick names
      	for(uint8_t i=0; i<4; i++)
				{
      		lcd_putsAttIdx( FW*5, y, PSTR(STR_STICK_NAMES), i, 0 ) ;
					if ( sub == subN )
					{
						SubMenuCall = 0x80 + i + 5 ;
					}
					alphaEditName( 11*FW, y, &g_eeGeneral.customStickNames[i*4], 4, sub==subN, (uint8_t *)&PSTR(STR_STICK_NAMES)[i*5+1] ) ;
	 				y += FH ;
					subN += 1 ;
				}
#ifndef BIG_SCREEN
			}
#endif
		}			 
		break ;


// Filter ADC - ALL
// Rotary divisor - Not X7 or XLITE
// I2C Function - 9XT only
// BT Com port - 9XT only



		case M_HARDWARE :
		{
			uint8_t subN = 0 ;
#ifdef PCBSKY
			uint8_t num = 9 + 2 + 2 + 4 + 1 + 1 + 4 + 1 ;
 #ifndef REVX
			num += 1 ;
 #endif
			IlinesCount = num ;
#else
 #ifdef PCB9XT
			IlinesCount = 4 + 4 + 1 + 7 + 5 + 1 ;
 #else
  #ifdef REV9E
			IlinesCount = 4 + 5 + 1 + 3 + 3 ;
  #else
   #ifdef PCBX7
    #ifdef PCBT12
			IlinesCount = 1 + 4 + 3 ;
		#else
			IlinesCount = 7 + 4 + 1 + 1 + 2 ;
		#endif
   #else
    #if defined(PCBX12D) || defined(PCBX10)
			IlinesCount = 2 + 5 + 6 + 4 + 1 ;
    #else
     #ifdef PCBXLITE
			IlinesCount = 6 + 4 + 3 ;
     #else
      #ifdef PCBX9LITE
 		   #if defined(X9LS)
			IlinesCount = 3+1+2 + 4 + 1 + 2 - 2 ;
			#define HW_PAGE1	4
			 #else
			IlinesCount = 3+1+2 + 4 + 1 + 2 ;
			#define HW_PAGE1	6
       #endif
			#define HW_PAGE2	4
      #else
			 #ifdef PCBLEM1
			IlinesCount = 7 ;
			 #else
			IlinesCount = 6 + 4 + 1 + 1 + 4 ;
       #endif
      #endif
     #endif
    #endif
   #endif
  #endif
 #endif
#endif

#ifdef PCBSKY
 #ifndef REVX
			uint32_t page2 = 12 ;
			uint32_t page3 = 13 ;
 #else
  #ifdef PCB9XT
			uint32_t page2 = 12 ;
			uint32_t page3 = 13 ;
  #else
			uint32_t page2 = 11 ;
			uint32_t page3 = 12 ;
  #endif
 #endif
			if ( g_eeGeneral.analogMapping & MASK_6POS )
			{
				IlinesCount += 6 ;
				page3 += 6 ;
			}
#endif // PCBSKY

#ifdef PCBSKY
			if ( g_eeGeneral.ar9xBoard )
			{
				num += 1 ;
				page2 += 1 ;
				page3 += 1 ;
				IlinesCount += 2 ;
			}
			else
			{
				IlinesCount += 1 ;
			}
#endif
#if defined(PCBX12D) || defined(PCBX10)
			uint32_t page2 = 7 ;
			uint32_t page3 = 13 ;
//			IlinesCount += 6 ;
//			page3 += 6 ;
#endif

//#ifdef PCBLEM1
//			uint32_t page2 = 7 ;
//#endif

#ifdef PCB9XT
			uint32_t page2 = 8 ;
			uint32_t page3 = 9 ;
			if ( g_eeGeneral.analogMapping & MASK_6POS )
			{
				IlinesCount += 6 ;
				page3 += 6 ;
			}
#endif
#ifdef PCBX9D
	#ifdef PCBX7
   #ifndef PCBT12
			uint32_t page2 = 11 ;
			uint32_t page3 = 12 ;
	 #endif
	#else
   #ifndef PCBXLITE
    #ifdef REV9E
			uint32_t page2 = 9 ;
			uint32_t page3 = 10 ;
	  #else
     #ifdef PCBX9LITE
			uint32_t page2 = HW_PAGE1+HW_PAGE2 ;
			uint32_t page3 = page2+1 ;
	   #else
			uint32_t page2 = 10 ;
			uint32_t page3 = 11 ;
     #endif
    #endif
   #endif
	#endif
#ifndef PCBXLITE
 #ifndef PCBT12
			if ( g_eeGeneral.analogMapping & MASK_6POS )
			{
				IlinesCount += 6 ;
				page3 += 6 ;
			}
 #endif
#endif
#endif

			TITLE( PSTR(STR_Hardware) ) ;
			if ( HardwareMenuEnabled == 0 )
			{
				IlinesCount = 0 ;
        lcd_puts_Pleft( 3*FH,XPSTR(" Disabled (see Help)"));
				break ;
			}
			
#ifdef PCBSKY
			if ( sub < 6 )
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
 #ifdef REV9E
			if ( sub < 4 )
 #else
  #if defined(PCBX12D) || defined(PCBX10)
			if ( sub < 2 )
  #else
   #if defined(PCBX7) || defined (PCBXLITE)
    #ifdef PCBT12
			if ( sub < 1 )
    #else
			if ( sub < 6 )
    #endif
   #else
    #ifdef PCBX9LITE
 			if ( sub < HW_PAGE1 )
    #else // X3
 			if ( sub < 5 )
    #endif // X3
   #endif
  #endif
 #endif
#endif
#ifdef PCBLEM1
			if ( sub < 6 )
#endif
#ifdef PCB9XT
			if ( sub < 4 )
#endif
			{
				displayNext() ;
        lcd_puts_Pleft( y,PSTR(STR_FILTER_ADC));
        lcd_putsAttIdx(PARAM_OFS, y, XPSTR("\004SINGOSMPFILT"),g_eeGeneral.filterInput,(sub==subN ? blink:0));
        if(sub==subN) CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.filterInput, 2);
 				y += FH ;
				subN += 1 ;

#ifndef PCBX7
 #ifndef PCBX9LITE
  #ifndef PCBLEM1
  			lcd_puts_Pleft( y, PSTR(STR_ROTARY_DIVISOR));
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\001142"),g_eeGeneral.rotaryDivisor,(sub==subN ? blink:0));
  			if(sub==subN)
				{
					uint8_t oldValue = g_eeGeneral.rotaryDivisor ;
					CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.rotaryDivisor,2);
					if ( g_eeGeneral.rotaryDivisor != oldValue )
					{
extern volatile int32_t Rotary_count ;
extern int32_t LastRotaryValue ;
						Rotary_count = 0 ;
						Rotary_diff = 0 ;
						LastRotaryValue = 0 ;
					}
				}
 				y += FH ;
				subN += 1 ;
  #endif // nPCBLEM1
 #endif // X3
#endif

#ifdef PCB9XT
		 		lcd_puts_Pleft( y, XPSTR( "I2C Function") ) ;
				uint8_t oldValue = g_eeGeneral.enableI2C ;
				g_eeGeneral.enableI2C = checkIndexed( y, XPSTR(FWx15"\002""\004 Off I2CCOM3"), g_eeGeneral.enableI2C, (sub==subN) ? blink:0 ) ;
				if ( oldValue != g_eeGeneral.enableI2C )
				{
					// Change function
				}
 				y += FH ;
				subN += 1 ;

		 		lcd_puts_Pleft( y, XPSTR( "Bt Com Port") ) ;
				g_eeGeneral.btComPort = checkIndexed( y, XPSTR(FWx15"\002""\004NONECOM2COM3"), g_eeGeneral.btComPort, (sub==subN) ? blink:0 ) ;
 				y += FH ;
				subN += 1 ;
#endif

// If X9Dplus, allow configuration of S3 as POT or 6-pos switch

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)

#if defined(REVPLUS) || defined(REV9E)
#define NUM_MAP_POTS	3
#else
 #ifdef PCBX9LITE
#define NUM_MAP_POTS	1
 #else // X3
#define NUM_MAP_POTS	2
 #endif // X3
#endif

#ifndef REV9E
 #ifndef PCBX12D
	#ifndef PCBX7
	 #ifndef PCBX9LITE
    #ifndef PCBX10
				uint32_t value ;
				uint32_t oldValue ;
    #endif	// nX10
	 #endif	// nX3
	#endif	// nX7
 #endif	// nX12D
#endif	// nREV9E
#ifndef REV9E
 #ifndef PCBX12D
	#ifndef PCBX7
   #ifndef PCBXLITE
    #ifndef PCBX9LITE
     #ifndef PCBX10
  			lcd_puts_Pleft( y, XPSTR("Encoder"));
				value = g_eeGeneral.analogMapping & ENC_MASK ;
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\002--P1P2P3"), value, (sub==subN ? BLINK:0));
				oldValue = value ;
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_MAP_POTS ) ;
 				g_eeGeneral.analogMapping = ( g_eeGeneral.analogMapping & ~ENC_MASK ) | value ;
				if ( value != oldValue )
				{
					init_adc2() ;
				}
				y += FH ;
				subN += 1 ;
     #endif	// nX12D
	  #endif	// nX3
   #endif // nXlite
	#endif	// nX7
 #endif	// nX12D
#endif	// nREV9E

#ifdef PCBXLITE
  			lcd_puts_Pleft( y, XPSTR("Switch C"));
				value = g_eeGeneral.ailsource ;
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\0052-Pos3-Pos"), value, (sub==subN ? BLINK:0));
				oldValue = value ;
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, 1 ) ;
				g_eeGeneral.ailsource = value ;
				if ( value != oldValue )
				{
					createSwitchMapping() ;
				}
				y += FH ;
				subN += 1 ;

  			lcd_puts_Pleft( y, XPSTR("Switch D"));
				value = g_eeGeneral.rudsource ;
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\0052-Pos3-Pos"), value, (sub==subN ? BLINK:0));
				oldValue = value ;
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, 1 ) ;
				g_eeGeneral.rudsource = value ;
				if ( value != oldValue )
				{
					createSwitchMapping() ;
				}
				y += FH ;
				subN += 1 ;

#endif // Xlite

#ifndef PCBX12D
#ifndef PCBX10
//  			lcd_puts_Pleft( y, XPSTR("6 Pos. Switch"));
//				value = ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2 ;
//	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\002--P1P2P3"), value, (sub==subN ? blink:0));
//				oldValue = value ;
//				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_MAP_POTS ) ;
// 				g_eeGeneral.analogMapping = ( g_eeGeneral.analogMapping & ~MASK_6POS ) | ( value << 2 ) ;
//				if ( value != oldValue )
//				{
//					createSwitchMapping() ;
//				}
//				y += FH ;
//				subN += 1 ;

#ifndef PCBT12
 #if defined(PCBX9LITE) && defined(X9LS)
 #else
  			lcd_puts_Pleft( y, XPSTR("PB1 Switch\037PB2 Switch"));
				g_eeGeneral.pb1source = edit3posSwitchSource( y, g_eeGeneral.pb1source, USE_PB1, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;
				g_eeGeneral.pb2source = edit3posSwitchSource( y, g_eeGeneral.pb2source, USE_PB2, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;
#endif
#ifdef PCBX9LITE
  			lcd_puts_Pleft( y, XPSTR("PB3 Switch"));
				g_eeGeneral.pb3source = edit3posSwitchSource( y, g_eeGeneral.pb3source, USE_PB3, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;
#endif

#ifdef PCBX9LITE
  			lcd_puts_Pleft( y, XPSTR("Pot 2\037Pot 3"));
				g_eeGeneral.extraPotsSource[0] = checkIndexed(y,
      			XPSTR(FWx17"\002\004NONEPC2 PC3 "), g_eeGeneral.extraPotsSource[0], (sub==subN));
				NumExtraPots = countExtraPots() ;
				y += FH ;
				subN += 1 ;
				g_eeGeneral.extraPotsSource[1] = checkIndexed(y,
      			XPSTR(FWx17"\002\004NONEPC2 PC3 "), g_eeGeneral.extraPotsSource[1], (sub==subN));
				NumExtraPots = countExtraPots() ;
				y += FH ;
				subN += 1 ;

#endif

#ifdef PCBX7
  			lcd_puts_Pleft( y, XPSTR("Pot 3\037Pot 4"));
				g_eeGeneral.extraPotsSource[0] = checkIndexed(y,
      			XPSTR(FWx17"\002\004NONEPC5 PB1 "), g_eeGeneral.extraPotsSource[0], (sub==subN));
				NumExtraPots = countExtraPots() ;
				y += FH ;
				subN += 1 ;
				g_eeGeneral.extraPotsSource[1] = checkIndexed(y,
      			XPSTR(FWx17"\002\004NONEPC5 PB1 "), g_eeGeneral.extraPotsSource[1], (sub==subN));
				NumExtraPots = countExtraPots() ;
				y += FH ;
				subN += 1 ;

  			lcd_puts_Pleft( y, PSTR(STR_ROTARY_DIVISOR));
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\001142"),g_eeGeneral.rotaryDivisor,(sub==subN ? blink:0));
  			if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0(g_eeGeneral.rotaryDivisor,2);
				}
 				y += FH ;
				subN += 1 ;

#endif	// X7
#endif	// T12

#endif	// nX10
#endif	// nX12D

#endif	// PCBX9D

#ifdef PCBSKY
				uint8_t lastValue = g_eeGeneral.stickGain ;
//				uint8_t b ;
        edit_stick_gain(12, y, PSTR(STR_LV), STICK_LV_GAIN, (sub==subN) ) ;
 				y += FH ;
				subN += 1 ;
  		
        edit_stick_gain(12, y, PSTR(STR_LH), STICK_LH_GAIN, (sub==subN) ) ;
 				y += FH ;
				subN += 1 ;

        edit_stick_gain(12, y, PSTR(STR_RV), STICK_RV_GAIN, (sub==subN) ) ;
 				y += FH ;
				subN += 1 ;

        edit_stick_gain(12, y, PSTR(STR_RH), STICK_RH_GAIN, (sub==subN) ) ;
				if ( lastValue != g_eeGeneral.stickGain )
				{
					set_stick_gain( g_eeGeneral.stickGain ) ;
				}
#endif // PCBSKY
#ifndef PCBLEM1
			}
#endif			
			
#ifdef PCBSKY
			else if ( sub < page2 )
			{
				displayNext() ;
				subN = 6 ;
#endif // PCBSKY

#ifdef PCB9XT
			else if ( sub < page2 )
			{
				displayNext() ;
				subN = 4 ;
#endif // PCB9XT

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
 #ifdef REV9E
			else if ( sub < 9 )
			{
				subN = 4 ;
 #else
	#if defined(PCBX12D) || defined(PCBX10)
			else if ( sub < page2 )
			{
				displayNext() ;
				subN = 2 ;
  #else
	 #ifdef PCBX7
    #ifdef PCBT12
			else if ( sub < 5 )
			{
				subN = 1 ;
		#else
			else if ( sub < 11 )
			{
				subN = 6 ;
		#endif
   #else
	  #ifdef PCBXLITE
			else if ( sub < 10 )
			{
				subN = 6 ;
    #else
     #ifdef PCBX9LITE
			else if ( sub < HW_PAGE1+HW_PAGE2 )
			{
				subN = HW_PAGE2 ;
     #else
			else if ( sub < 10 )
			{
				subN = 5 ;
     #endif
    #endif
   #endif
  #endif
 #endif
#endif // PCBX9D
//				uint8_t attr = 0 ;
//	  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_eeGeneral.stickDeadband[0], 15 ) ; }
//				lcd_xlabel_decimal( 20*FW, y, g_eeGeneral.stickDeadband[0], attr, XPSTR("Stick LV deadband") ) ;
        edit_stick_deadband(15, y, PSTR(STR_LV), 1, (sub==subN));
				y += FH ;
  			subN++;

//				attr = 0 ;
//	  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_eeGeneral.stickDeadband[0], 15 ) ; }
//				lcd_xlabel_decimal( 20*FW, y, g_eeGeneral.stickDeadband[0], attr, XPSTR("Stick LH deadband") ) ;
        edit_stick_deadband(15, y, PSTR(STR_LH), 0, (sub==subN));
  			y += FH ;
  			subN++;

//				attr = 0 ;
//	  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_eeGeneral.stickDeadband[2], 15 ) ; }
//				lcd_xlabel_decimal( 20*FW, y, g_eeGeneral.stickDeadband[2], attr, XPSTR("Stick RV deadband") ) ;
        edit_stick_deadband(15, y, PSTR(STR_RV), 2, (sub==subN));
  			y += FH ;
  			subN++;
//				attr = 0 ;
//	  		if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_eeGeneral.stickDeadband[3], 15 ) ; }
//				lcd_xlabel_decimal( 20*FW, y, g_eeGeneral.stickDeadband[3], attr, XPSTR("Stick RH deadband") ) ;
        edit_stick_deadband(15, y, PSTR(STR_RH), 3, (sub==subN));
  			y += FH ;
  			subN++;

#if defined(PCBSKY) || defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
 #ifndef PCBXLITE
  #ifndef PCBT12
   #ifndef PCBX9LITE
				{
					uint8_t lastValue = g_eeGeneral.softwareVolume ;
			  	g_eeGeneral.softwareVolume = onoffMenuItem( lastValue, y, XPSTR("Software Vol."), sub==subN ) ;
					if ( lastValue != g_eeGeneral.softwareVolume )
					{
						setVolume( g_eeGeneral.volume ) ;
					}
				}
	  		y += FH ;
				subN += 1 ;
   #endif // X3
  #endif
 #endif
#endif

#ifdef PCBSKY
#ifndef REVX
		  	g_eeGeneral.ar9xBoard = onoffMenuItem( g_eeGeneral.ar9xBoard, y, XPSTR("AR9X Board"), sub==subN ) ;
	  		y += FH ;
				subN += 1 ;
#endif
#endif

#ifdef PCBSKY
				if ( g_eeGeneral.ar9xBoard )
				{
					lcd_puts_Pleft( y, XPSTR("RTC") );
					g_eeGeneral.externalRtcType = checkIndexed( y, XPSTR("\052""\001""\006  NONEDS3231"), g_eeGeneral.externalRtcType, (sub==subN) ) ;// FWx07
	  			y += FH ;
					subN += 1 ;
				}
#endif
#ifdef PCBLEM1
				lcd_puts_Pleft( y, XPSTR("RTC") );
				g_eeGeneral.externalRtcType = checkIndexed( y, XPSTR("\052""\001""\006  NONEDS3231"), g_eeGeneral.externalRtcType, (sub==subN) ) ;// FWx07
  			y += FH ;
				subN += 1 ;
#endif
			
#ifndef PCBLEM1
			}
#endif
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D)
#ifndef PCBXLITE
 #ifndef PCBT12
			else if ( sub < page3 )
			{
				uint32_t value ;
				uint32_t oldValue ;
				subN = page2 ;
  			lcd_puts_Pleft( y, XPSTR("6 Pos. Switch"));
				value = ( g_eeGeneral.analogMapping & MASK_6POS ) >> 2 ;



#ifdef PCBX9D
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\002--P1P2P3P4"), value, (sub==subN ? blink:0));
#endif



#ifdef PCBSKY
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\003---P1 P2 P3 AUX"), value, (sub==subN ? INVERS:0));
#define NUM_ANA_INPUTS	4
#endif
#ifdef PCB9XT
	  		lcd_putsAttIdx( 15*FW, y, XPSTR("\003---P1 P2 P3 "), value, (sub==subN ? INVERS:0));
#define NUM_ANA_INPUTS	3
#endif
				oldValue = value ;
#ifdef PCBX9D
 #ifdef PCBX7
				if(sub==subN)
				{
					uint32_t max = NUM_MAP_POTS ;
					if ( g_eeGeneral.extraPotsSource[0] )
					{
						max += 1 ;
					}
					if ( g_eeGeneral.extraPotsSource[1] )
					{
						max += 1 ;
					}
					CHECK_INCDEC_H_GENVAR_0( value, max ) ;
				}
 #else
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_MAP_POTS ) ;
 #endif
#else
				if(sub==subN) CHECK_INCDEC_H_GENVAR_0( value, NUM_ANA_INPUTS ) ;
#endif
 				g_eeGeneral.analogMapping = ( g_eeGeneral.analogMapping & ~MASK_6POS ) | ( value << 2 ) ;
				if ( value != oldValue )
				{
					createSwitchMapping() ;
				}
				y += FH ;
				subN += 1 ;
				if ( g_eeGeneral.analogMapping & MASK_6POS )
				{
					uint32_t i ;
					uint16_t value ;
					value = g_eeGeneral.analogMapping & MASK_6POS ;
#ifdef PCBSKY
					if ( value == USE_AUX_6POS )
					{
						value = Analog_values[9] ;
					}
					else
					{
						value = Analog_values[ (value >> 2) + 3] ;
					}
#endif
#ifdef PCB9XT
					value = M64Analog[ (value >> 2) + 3] ;
#endif
#ifdef PCBX9D
			  	value >>= 2 ;
					value += 3 ;
					if ( value > 5 )
					{
						value = 9 ;
					}
					value = Analog_values[value] ;
#endif
					lcd_outhex4( 12*FW, 0*FH, value ) ;
					lcd_putsAttIdx( 17*FW, 0*FH, PSTR(SWITCHES_STR), HSW_Ele6pos0 + switchPosition(HSW_Ele6pos0)-HSW_OFFSET, 0 ) ;

					for ( i = 0 ; i < 6 ; i += 1 )
					{
  					lcd_puts_Pleft( y, XPSTR("6P0:"));
  					lcd_putc( 2*FW, y, i+'0' ) ;
						if ( i < 5 )
						{
							lcd_outhex4( 12*FW, y, SixPositionTable[i] ) ;
						}
						lcd_outhex4( 5*FW, y, g_eeGeneral.SixPositionCalibration[i] ) ;
						if ( sub == subN )
						{
							lcd_char_inverse( 5*FW, y, 20, 0 ) ;
							if ( Tevent==EVT_KEY_BREAK(KEY_MENU) ) // || Tevent == EVT_KEY_BREAK(BTN_RE)  )
					    {
								g_eeGeneral.SixPositionCalibration[i] = value ;
								s_editMode = 0 ;
								create6posTable() ;
							}	
						}
						y += FH ;
						subN += 1 ;
					}
				}
			}
 #endif // nT12
#endif // nXLITE
#endif

#ifndef PCBLEM1

#if defined(PCBX12D) || defined(PCBX10)
			else if ( sub < page3 )
			{
				displayNext() ;
				subN = page2 ;
					uint32_t i ;
					uint16_t value ;
					value = AnalogData[10] ;
					lcd_outhex4( 12*FW, 0*FH, value ) ;
					lcd_putsAttIdx( 17*FW, 0*FH, PSTR(SWITCHES_STR), HSW_Ele6pos0 + switchPosition(HSW_Ele6pos0)-HSW_OFFSET, 0 ) ;
					for ( i = 0 ; i < 6 ; i += 1 )
					{
  					lcd_puts_Pleft( y, XPSTR("6P0:"));
  					lcd_putc( 2*FW, y, i+'0' ) ;
						if ( i < 5 )
						{
							lcd_outhex4( 12*FW, y, SixPositionTable[i] ) ;
						}
						lcd_outhex4( 5*FW, y, g_eeGeneral.SixPositionCalibration[i] ) ;
						if ( sub == subN )
						{
							lcd_char_inverse( 5*FW, y, 20, 0 ) ;
							if ( Tevent==EVT_KEY_BREAK(KEY_MENU) ) // || Tevent == EVT_KEY_BREAK(BTN_RE)  )
					    {
								g_eeGeneral.SixPositionCalibration[i] = value ;
								s_editMode = 0 ;
								create6posTable() ;
							}	
						}
						y += FH ;
						subN += 1 ;
					}
			}
#endif


#if defined(PCBSKY) || defined(PCB9XT)
			else if ( sub < page3+7 )
			{
				subN = page3 ;
//ELE NONE EXT1, EXT2, EXT3, DAC1, ANA, EXT4, EXT5, EXT6
//AIL NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6
//GEA NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6
//RUD NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6
//THR NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6
//PB1 NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6
//PB1 NONE EXT1, EXT2, EXT3, DAC1, ELE, EXT4, EXT5, EXT6

//ELE has extra option if ANA of ANA3, AN6A, AN6B

				if ( sub >= IlinesCount )
				{
					sub = mstate.m_posVert = IlinesCount-1 ;
				}
  			lcd_puts_Pleft( y, XPSTR("ELE Switch"));
				uint16_t sm = g_eeGeneral.switchMapping ;
				uint8_t value = g_eeGeneral.elesource ;
#ifndef PCB9XT
				if ( value > 5 )
				{
					value += 2 ;
				}
				if ( value >101 )
				{
					value -= 102-6 ;
				}
#endif	// nPCB9XT

#ifdef REVX
				value = checkIndexed( y, XPSTR(FWx17"\007\004NONELCD2LCD6LCD7DAC1ANA 6PSA6PSB"), value, (sub==subN) ) ;
#else
#ifdef PCB9XT
				value = checkIndexed( y, XPSTR(FWx17"\010\004NONEEXT1EXT2EXT3EXT4EXT5EXT6EXT7EXT8"), value, (sub==subN) ) ;
#else
				if ( g_eeGeneral.ar9xBoard == 0 )
				{
					value = checkIndexed( y, XPSTR(FWx17"\012\004NONELCD2LCD6LCD7DAC1ANA 6PSA6PSBEXT4EXT5EXT6"), value, (sub==subN) ) ;
				}
				else
				{
					value = checkIndexed( y, XPSTR(FWx17"\007\004NONEEXT1EXT2EXT3PB14AD106PSA6PSB"), value, (sub==subN) ) ;
				}
#endif	// PCB9XT
#endif

#ifndef PCB9XT
				if ( value > 5 )
				{
					value -= 2 ;
					if ( value < 6 )
					{
						value += 102-6 ;
					}
				}
#endif	// nPCB9XT
				
				g_eeGeneral.elesource = value ;
				g_eeGeneral.switchMapping = sm & ~(USE_ELE_3POS | USE_ELE_6POS | USE_ELE_6PSB ) ;
				if ( value )
				{
					uint16_t mask = USE_ELE_3POS ;
					if ( value == 100 )
					{
						mask = USE_ELE_6POS ;
					}
					else if ( value == 101 )
					{
						mask = USE_ELE_6PSB ;
					}
					g_eeGeneral.switchMapping |= mask ;
				} 
				if ( sm != g_eeGeneral.switchMapping )
				{
					createSwitchMapping() ;
					sm = g_eeGeneral.switchMapping ;
				}
 				y += FH ;
				subN++;

				{
				  lcd_puts_Pleft( y, XPSTR("THR Switch"));
					g_eeGeneral.thrsource = edit3posSwitchSource( y, g_eeGeneral.thrsource, USE_THR_3POS, (sub==subN), 1 ) ;
				}	
 				y += FH ;
				subN++;

					
				{
				  lcd_puts_Pleft( y, XPSTR("RUD Switch"));
					g_eeGeneral.rudsource = edit3posSwitchSource( y, g_eeGeneral.rudsource, USE_RUD_3POS, (sub==subN), 1 ) ;
				}	
 				y += FH ;
				subN++;
					
				{
				  lcd_puts_Pleft( y, XPSTR("AIL Switch"));
					g_eeGeneral.ailsource = edit3posSwitchSource( y, g_eeGeneral.ailsource, USE_AIL_3POS, (sub==subN), 1 ) ;
				}	
 				y += FH ;
				subN++;
					
				{
				  lcd_puts_Pleft( y, XPSTR("GEA Switch"));
					g_eeGeneral.geasource = edit3posSwitchSource( y, g_eeGeneral.geasource, USE_GEA_3POS, (sub==subN), 1 ) ;
				}	
					
	 				y += FH ;
					subN++;
				
  			lcd_puts_Pleft( y, XPSTR("PB1 Switch\037PB2 Switch"));
				g_eeGeneral.pb1source = edit3posSwitchSource( y, g_eeGeneral.pb1source, USE_PB1, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;
				g_eeGeneral.pb2source = edit3posSwitchSource( y, g_eeGeneral.pb2source, USE_PB2, (sub==subN), 0 ) ;
			}
#endif // PCBSKY
			else
			{
#if defined(PCBSKY) || defined(PCB9XT)
				subN = page3+7 ;
#else
#ifdef REV9E
				subN = page3 ;
#else
#if defined(PCBX12D) || defined(PCBX10)
				subN = 13 ;
#else
 #ifdef PCBXLITE
				subN = 10 ;
 #else
  #ifdef PCBT12
				subN = 5 ;
	#else
				subN = page3 ;
	#endif
 #endif
#endif
#endif
#endif

#if defined(PCBSKY) || defined(PCB9XT)
  			lcd_puts_Pleft( y, XPSTR("PB3 Switch\037PB4 Switch"));
				g_eeGeneral.pb3source = edit3posSwitchSource( y, g_eeGeneral.pb3source, USE_PB3, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;
				g_eeGeneral.pb4source = edit3posSwitchSource( y, g_eeGeneral.pb4source, USE_PB4, (sub==subN), 0 ) ;
				y += FH ;
				subN += 1 ;
#endif

#if defined(PCBSKY) || defined(PCB9XT)
#ifdef PCB9XT
				editExtraPot( y, 0, uint8_t (sub==subN) ) ;
 				y += FH ;
				subN++;
#endif

#ifdef PCBSKY
//  			lcd_puts_Pleft( y, XPSTR("POT4"));
//				uint8_t value = g_eeGeneral.extraPotsSource[0] ;
//				value = checkIndexed( y, XPSTR(FWx17"\002\004NONEAD10AD8 "), value, (sub==subN) ) ;
//				g_eeGeneral.extraPotsSource[0] = value ;
//				NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
        edit_xpot_source( y, XPSTR("POT4"), 0, (sub==subN) ) ;
 				y += FH ;
				subN++;
#endif

#ifdef PCB9XT
				editExtraPot( y, 1, uint8_t (sub==subN) ) ;
 				y += FH ;
				subN++;
#else
//	  			lcd_puts_Pleft( y, XPSTR("POT5"));
//					value = g_eeGeneral.extraPotsSource[1] ;
//					value = checkIndexed( y, XPSTR(FWx17"\002\004NONEAD10AD8 "), value, (sub==subN) ) ;
//					g_eeGeneral.extraPotsSource[1] = value ;
//					NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
	        edit_xpot_source( y, XPSTR("POT5"), 1, (sub==subN) ) ;
 					y += FH ;
					subN++;
#endif	// PCB9XT

#endif // PCBSKY !! PCB9XT

#ifdef REV9E
  			lcd_puts_Pleft( y, XPSTR("POT3"));
				g_eeGeneral.extraPotsSource[0] = onoffItem( g_eeGeneral.extraPotsSource[0], y, (sub==subN) ) ;
				NumExtraPots = NUM_EXTRA_POTS - 2 + countExtraPots() ;
 				y += FH ;
				subN++;
  			lcd_puts_Pleft( y, XPSTR("POT4"));
				g_eeGeneral.extraPotsSource[1] = onoffItem( g_eeGeneral.extraPotsSource[1], y, (sub==subN) ) ;
				NumExtraPots = NUM_EXTRA_POTS - 2 + countExtraPots() ;
				y += FH ;
				subN++;

				uint32_t i ;
				for ( i = 0 ; i < 3 ; i += 1 )
				{
					uint32_t mask = 1 << i ;
	  			lcd_puts_Pleft( y, XPSTR("Switch ") ) ;
					lcd_putc( 7*FW, y, 'I' + i ) ;
					uint32_t temp = g_eeGeneral.ailsource & mask ;
					uint8_t value = temp ? 1 : 0 ;
					value = onoffItem( value, y, (sub==subN) ) ;
					value <<= i ;
					g_eeGeneral.ailsource = ( g_eeGeneral.ailsource & ~mask ) | value ;
					if ( value != temp )
					{
						createSwitchMapping() ;
					}
					y += FH ;
					subN++;
				}
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
#ifndef REV9E
				{
					uint8_t value = g_eeGeneral.potDetents & 1 ;
			  	value = onoffMenuItem( value, y, XPSTR("P1 has Detent"), sub==subN ) ;
					g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~1) | value ;
				}
 				y += FH ;
				subN += 1 ;

#ifndef PCBX9LITE
				{
					uint8_t value = (g_eeGeneral.potDetents >> 1) & 1 ;
			  	value = onoffMenuItem( value, y, XPSTR("P2 has Detent"), sub==subN ) ;
					g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~2) | ( value << 1 ) ;
				}
 				y += FH ;
				subN += 1 ;
#endif // X3
#ifndef PCBX7
 #ifndef PCBXLITE
  #ifndef PCBX9LITE
				{
					uint8_t value = (g_eeGeneral.potDetents >> 2) & 1 ;
			  	value = onoffMenuItem( value, y, XPSTR("P3/SL has Detent"), sub==subN ) ;
					g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~4) | ( value << 2 ) ;
				}
 				y += FH ;
				subN += 1 ;

				{
					uint8_t value = (g_eeGeneral.potDetents >> 3) & 1 ;
			  	value = onoffMenuItem( value, y, XPSTR("P4/SR has Detent"), sub==subN ) ;
					g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~8) | ( value << 3 ) ;
				}
 				y += FH ;
				subN += 1 ;

  #endif // X3
 #endif // PCBXLITE
#endif // PCBX7
#endif // nREV9E
#endif // PCBX9D

#if defined(PCBSKY) || defined(PCB9XT)
				uint8_t attr = 0 ;
				if ( sub == subN )
				{
					attr = blink ;
					if ( checkForMenuEncoderBreak( event ) )
					{
		    		killEvents( event ) ;
						SubMenuCall = 0x80 + subN ;
						pushMenu(menuConfigPots) ;
					}
				}
				lcd_putsAtt( 0, y, XPSTR("Pot Detents"), attr ) ;
 				y += FH ;
				subN += 1 ;
#endif // PCBSKY

#endif // PCBLEM1        

#ifdef PCBLEM1
			}
			else
			{
				subN = 6 ;
#endif			

				lcd_puts_Pleft( y,XPSTR("Stick Reverse:"));
 				y += FH ;
				for ( uint8_t i = 0 ; i < 4 ; i += 1 )
				{
#if defined(PCBX12D) || defined(PCBX10)
  				lcd_HiResimg( (6+4*i)*FW*2, y*2, sticksHiRes, i, 0 ) ;
#else
					lcd_img((6+4*i)*FW, y, sticks, i, 0 ) ;
#endif
					if (g_eeGeneral.stickReverse & (1<<i)) lcd_char_inverse( (6+4*i)*FW, y, 3*FW, 0 ) ;
				}
    		if(sub==subN)
				{
					CHECK_INCDEC_H_GENVAR_0( g_eeGeneral.stickReverse, 15 ) ;
					lcd_rect( 6*FW-1, y-1, 15*FW+2, 9 ) ;
				}
 				y += FH ;
				subN += 1 ;
			}
		}			 
		break ;

		case M_EEPROM :
		{
			uint8_t subN = 0 ;
			
			IlinesCount = 2 ;

			TITLE( XPSTR("EEPROM") ) ;
			if ( HardwareMenuEnabled == 0 )
			{
				IlinesCount = 0 ;
        lcd_puts_Pleft( 3*FH,XPSTR(" Disabled (see Help)"));
				break ;
			}
      lcd_putsAtt( 0, y, XPSTR("Backup EEPROM"), (sub==subN) ? INVERS : 0 ) ;
			y += FH ;
			subN += 1 ;
      lcd_putsAtt( 0, y,XPSTR("Restore EEPROM"), (sub==subN) ? INVERS : 0 ) ;

			if ( Tevent==EVT_KEY_BREAK(KEY_MENU) ) // || Tevent == EVT_KEY_BREAK(BTN_RE)  )
			{
  	  	s_editMode = 0 ;
				if ( sub == 0 )
				{
					pushMenu(menuBackupEeprom) ;
				}
				if ( sub == 1 )
				{
					pushMenu(menuRestoreEeprom) ;
				}
			}
		}			 
		break ;

//#define BT_TYPE_HC06		0
//#define BT_TYPE_HC05		1

#ifdef BLUETOOTH
		case M_BLUETOOTH :
		{	
			uint8_t subN = 0 ;

			if ( Tevent == EVT_ENTRY )
			{
      	BtControl.BtStateRequest = 1 ;
			}

			TITLE( XPSTR("BlueTooth") ) ;
#ifdef BLUETOOTH
 #ifdef BT_COMMAND_DEBUG
			IlinesCount = 3 ;
 #else
			IlinesCount = 5 ;
 #endif
#else
			IlinesCount = 0 ;
#endif
			y = 0 ;
			if ( g_eeGeneral.BtType >= BT_TYPE_HC05 )
			{
				IlinesCount += 3 ;
			}
			if ( g_eeGeneral.BtType == BT_TYPE_HC06 )
			{
				IlinesCount -= 2 ;
			}
			lcd_puts_Pleft( y,XPSTR("\013Type"));
			// Change 001 to 002 to enable CC-41
#if defined(X9LS) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
extern uint8_t BtStatus ;
			IlinesCount = (BtStatus && ( BtControl.BtMasterSlave == 2 ) ) ? 7 : 6 ;
			g_eeGeneral.BtType = BT_TYPE_PARA ;
			checkIndexed( y, XPSTR(FWx16"\001""\005PARA "), 0, (sub==subN) ) ;
			g_eeGeneral.BtType = BT_TYPE_PARA ;
#else
			
 #ifndef SMALL
			g_eeGeneral.BtType = checkIndexed( y, XPSTR(FWx16"\004""\005HC-06HC-05CC-41HM-10PARA "), g_eeGeneral.BtType, (sub==subN) ) ;
 #else
			g_eeGeneral.BtType = checkIndexed( y, XPSTR(FWx16"\003""\005HC-06HC-05CC-41HM-10"), g_eeGeneral.BtType, (sub==subN) ) ;
 #endif
#endif
#ifdef BLUETOOTH
			BtControl.BtModuleType = 1 << g_eeGeneral.BtType ;
#endif
			y += FH ;
			subN += 1 ;

#ifdef BLUETOOTH
			EditType = EE_MODEL ;
			lcd_puts_Pleft( y, XPSTR("BT Function") );
			{
				uint8_t attr = (sub == subN) ? InverseBlink : 0 ;
				lcd_putsAttIdx( 13*FW, y, XPSTR("\007    OFF  TrnRxTrnTxRxLcdDumpTelem  Script FrPARA "), g_model.BTfunction, attr ) ;
				if (attr)
				{
#if defined(X9LS) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
//#if defined(X9LS)
					g_model.BTfunction = checkOutOfOrder( g_model.BTfunction, (uint8_t *)BtFunctionMap, 4 ) ;
#else
					g_model.BTfunction = checkOutOfOrder( g_model.BTfunction, (uint8_t *)BtFunctionMap, 7 ) ;
#endif 
				}
			}
//			g_model.BTfunction = checkIndexed( y, XPSTR(FWx13"\003""\007    OFF  TrnRxTrnTxRxLcdDumpTelem  "), g_model.BTfunction, (sub==subN) ) ;
			y += FH ;
			subN += 1 ;
			EditType = EE_GENERAL ;

extern uint8_t BtName[] ;
//extern uint8_t BtNameChange ;

			uint8_t attr = (sub == subN) ? INVERS : 0 ;
				
			SubMenuCall = 0x82 ;
			uint8_t *pname = g_eeGeneral.btName ;
			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
			{
				pname = BtName ;
			}
			alphaEditName( 7*FW, y, pname, 14, attr, 0 ) ;
			if ( event == EVT_ENTRY_UP )
			{
				// Name was edited
				if ( AlphaEdited )
				{
#ifdef BLUETOOTH
					if ( BtControl.BtNameChange == 0 )
					{
						BtControl.BtNameChange = 0x80 ;
					}
#endif
					AlphaEdited = 0 ;
				}
			}

			y += FH ;
			subN += 1 ;

//extern uint8_t BtRoleChange ;
//extern uint8_t BtMasterSlave ;
			if ( g_eeGeneral.BtType >= BT_TYPE_HC05 )
			{
				lcd_puts_Pleft( y, XPSTR("BT Role") ) ;
#ifdef BLUETOOTH
	    	lcd_putsAttIdx( 12*FW, y, XPSTR("\007UNKNOWNSLAVE  MASTER "), BtControl.BtMasterSlave, (sub==subN) ? blink:0 ) ;
				uint8_t newRole = 0 ;
				if ( BtControl.BtMasterSlave == 2 )
				{
					newRole = 1 ;
				}
				if ( (sub == subN) )
				{
					newRole = checkIncDec( newRole, 0, 1, 0 ) ;
					if ( newRole+1 != BtControl.BtMasterSlave )
					{
						if ( BtControl.BtRoleChange == 0 )
						{
							BtControl.BtRoleChange = 0x80 | newRole ;
						}
					}
				}
#endif
				y += FH ;
				subN += 1 ;
			}
			
			if ( g_eeGeneral.BtType >= BT_TYPE_HC05 )
			{
				EditType = EE_MODEL ;
#ifndef X9LS
 #if not (defined(PCBX10) && defined(PCBREV_EXPRESS))
				g_model.autoBtConnect = onoffMenuItem( g_model.autoBtConnect, y, XPSTR("Auto Connect"), sub==subN ) ;
				subN += 1 ;
				y += FH ;				 
 #endif
#endif
				uint8_t attr = (sub == subN) ? INVERS : 0 ;
				lcd_putsAtt( 0, y, XPSTR("Scan"), attr ) ; 
				if ( attr )
				{
					if ( checkForMenuEncoderBreak( event ) )
					{
						s_editMode = 0 ;
						SubMenuCall = 0x85 ;
    		   	pushMenu( menuProcBtScan ) ;
					}
				}
				EditType = EE_GENERAL ;
				subN += 1 ;
#ifndef X9LS
 #if not (defined(PCBX10) && defined(PCBREV_EXPRESS))

				attr = (sub == subN) ? INVERS : 0 ;
				lcd_putsAtt( 10*FW, y, XPSTR("Configure"), attr ) ; 
				if ( attr )
				{
					if ( checkForMenuEncoderBreak( event ) )
					{
						s_editMode = 0 ;
						SubMenuCall = 0x86 ;
   		   		pushMenu( menuProcBtConfigure ) ;
					}
				}
				subN += 1 ;
				y += FH ;				 
 #endif
#endif

#if defined(X9LS) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
//#if defined(X9LS)
#ifndef SMALL
				if ( g_eeGeneral.BtType == BT_TYPE_PARA )
				{
					lcd_puts_Pleft( 6*FH, BtStatus ? "Connected" : "Cleared" ) ; 
				}
				else
#endif
#endif
				{
extern uint8_t BtState[] ;
					lcd_puts_Pleft( 6*FH, (char *)BtState ) ; 
				}
extern uint8_t BtPswd[] ;

				attr = (sub == subN) ? INVERS : 0 ;
				lcd_putsAtt( 12*FW, y, XPSTR("List"), attr ) ; 
				
				if ( attr )
				{
					if ( checkForMenuEncoderBreak( event ) )
					{
						s_editMode = 0 ;
						SubMenuCall = 0x87 ;
    			  pushMenu( menuProcBtAddressList ) ;
					}
				}
				subN += 1 ;
				y += FH ;				 

#if defined(X9LS) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
//#if defined(X9LS)
				if ( ( g_eeGeneral.BtType == BT_TYPE_PARA ) && BtStatus && (BtControl.BtMasterSlave == 2) )
				{
					attr = (sub == subN) ? INVERS : 0 ;
					lcd_putsAtt( 10*FW, y, XPSTR("Clear"), attr ) ; 
					if ( attr )
					{
						if ( checkForMenuEncoderBreak( event ) )
						{
							s_editMode = 0 ;
							SubMenuCall = 0x86 ;
							BtControl.BtLinkRequest = 0x40 ;
						}
					}
				}
#endif

				if ( ( g_eeGeneral.BtType == BT_TYPE_PARA ) || ( g_eeGeneral.BtType == BT_TYPE_HC05 ) )
				{
					lcd_puts_Pleft( FH*7, XPSTR("MAC:")) ;
extern uint8_t BtMAC[] ;
					lcd_putsAtt( 4*FW, 7*FH, (char *)BtMAC, 0 ) ;
				}
				else
				{
					lcd_puts_Pleft( FH*7, XPSTR("Pswd\013Br")) ;
					lcd_putsAtt( 5*FW-3, 7*FH, (char *)BtPswd, 0 ) ;
#ifdef BLUETOOTH
  				lcd_putsAttIdx(  14*FW, 7*FH, XPSTR(BtBaudString), BtControl.BtCurrentBaudrate, 0 ) ;
#endif
				}	 

			}
#endif

		}	
		break ;
#endif

	}
}

#define M_MINDEX			0
#define M_MIXER				1
#define M_HELI				2
#define M_LIMITS			3
#define M_EXPO				4
#define M_MODES				5
#define M_CURVE				6
#define M_SWITCHES		7
#define M_MUSIC				8
#define M_SAFETY			9
#define M_GLOBALS			10
#define M_TELEMETRY		11
#define M_VOICE				12
#define M_TIMERS			13
#define M_MGENERAL		14
#define M_PROTOCOL		15

void menuProcModelIndex(uint8_t event)
{
	static MState2 mstate ;
	EditType = EE_MODEL ;
	if ( PopupData.PopupActive == 0 )
	{
		event = indexProcess( event, &mstate, 8 ) ;
		event = mstate.check_columns( event, IlinesCount-1 ) ;
	}
					
	SubMenuFromIndex = 1 ;
	switch ( SubmenuIndex )
	{
		case M_MIXER :
			if ( PopupData.PopupActive == 0 )
			{
				PopupData.PopupIdx = 0 ;
				PopupData.PopupActive = 3 ;
				SubmenuIndex = 0 ;
			}
		break ;
		case M_SWITCHES :
      pushMenu(menuProcSwitches) ;
		break ;
		case M_TELEMETRY :
			if ( PopupData.PopupActive == 0 )
			{
				PopupData.PopupIdx = 0 ;
				PopupData.PopupActive = 2 ;
				SubmenuIndex = 0 ;
			}
		break ;
		case M_LIMITS :
      pushMenu(menuProcLimits) ;
		break ;
		case M_VOICE :
      pushMenu(menuProcVoiceAlarm) ;
		break ;
		case M_CURVE :
      pushMenu(menuProcCurve) ;
		break ;
		case M_EXPO :
			SingleExpoChan = 0 ;
			s_expoChan = 0 ;
      pushMenu(menuProcExpoAll) ;
		break ;
		case M_MODES :
      pushMenu(menuModelPhases) ;
		break ;
		case M_GLOBALS :
			if ( PopupData.PopupActive == 0 )
			{
				PopupData.PopupIdx = 0 ;
				PopupData.PopupActive = 1 ;
				SubmenuIndex = 0 ;
			}
		break ;
		case M_PROTOCOL :
      pushMenu(menuProcProtocol) ;
		break ;
		case M_SAFETY :
      pushMenu(menuProcSafetySwitches) ;
		break ;
		default :
			SubMenuFromIndex = 0 ;
			if ( event == EVT_ENTRY )
			{
				audioDefevent(AU_MENUS) ;
				event = 0 ;
			}
		break ;
	}

	uint32_t sub = mstate.m_posVert ;
	uint32_t y = FH ;
	uint8_t blink = InverseBlink ;

	switch ( SubmenuIndex )
	{
		case M_MINDEX :
#if defined(PCBX12D) || defined(PCBX10)
//			lcd_putsAtt(0 + X12OFFSET-33,0,"Model Setup",INVERS|DBLSIZE|CONDENSED) ;
			lcd_putsAtt(0 + X12OFFSET+36,0,"Model Setup",INVERS) ;
#else
			lcd_putsAtt(0 + X12OFFSET,0,"MODEL SETUP",INVERS) ;
#endif
			IlinesCount = 15 ;
			sub += 1 ;

static const uint16_t in_Strings[] = {
STR_Mixer,
STR_heli_setup,
STR_limits,
STR_Expo,
STR_Modes,
STR_Curves,
STR_Cswitches,
STR_Music,
STR_Safety,
STR_Globals,
STR_Telemetry,
STR_Voice,
STR_Timer,
STR_General,
STR_Protocol
};
	

			 
			if ( PopupData.PopupActive )
			{
				if ( PopupData.PopupActive == 1 )
				{
					sub = M_GLOBALS ;
				}
				else if ( PopupData.PopupActive == 2 )
				{
					sub = M_TELEMETRY ;
				}
				else
				{
					sub = M_MIXER ;
				}
			}
			
			displayIndex( in_Strings, 8, 7, sub ) ;
#if defined(PCBX12D) || defined(PCBX10)
			displayIndexIcons( m_icons, 8, 7 ) ;
#endif			
			if ( PopupData.PopupActive )
			{
				uint32_t mask ;
				if ( PopupData.PopupActive == 1 )
				{
					mask = 0x0007 ; ;
				}
				else if ( PopupData.PopupActive == 2 )
				{
#ifdef BLOCKING
					mask = 0x0398 ;
#else
					mask = 0x0698 ;
#endif
				}
				else
				{
					mask = 0x0060 ;
				}
				uint8_t popaction = doPopup( PSTR( STR_POPUP_GLOBALS ), mask, 13, event ) ;
  			if ( popaction == POPUP_SELECT )
				{
					uint8_t popidx = PopupData.PopupSel ;
					if ( popidx == 0 )	// gvars
					{
    	  		pushMenu(menuProcGlobals) ;
					}
					else if ( popidx == 1 )	// adjusters
					{
    	  		pushMenu(menuProcAdjust) ;
					}
					else if ( popidx == 2 )	// scalers
					{
    	  		pushMenu(menuProcScalers) ;
					}
					else if ( popidx == 3 )	// Telemetry
					{
    	  		pushMenu(menuProcTelemetry) ;
					}
					else if ( popidx == 4 )	// Custom
					{
    	  		pushMenu(menuCustomTelemetry) ;
					}
					else if ( popidx == 5 )	// Mixer
					{
			      pushMenu(menuProcMix) ;
					}
					else if ( popidx == 6 )	// Templates
					{
    	  		pushMenu(menuProcTemplates) ;
					}
					else if ( popidx == 7 )	// Logging
					{
    	  		pushMenu(menuLogging) ;
					}
#ifdef BLOCKING
					else if ( popidx == 8 )	// Tel Blocking
					{
    	  		pushMenu(menuBlocking) ;
					}
#endif
					else if ( popidx == 9 )	// Vario
					{
    	  		pushMenu(menuVario) ;
					}
					else if ( popidx == 10 )	// Sensors
					{
    	  		pushMenu(menuSensors) ;
					}
					SubmenuIndex = sub ;
				}
  			if ( popaction == POPUP_EXIT )
				{
					SubmenuIndex = 0 ;
					mstate.m_posVert = sub - 1 ;
				}
			}
		break ;

		case M_MUSIC :
		{	
			uint8_t subN = 0 ;
      TITLE( PSTR( STR_Music ) ) ;
			IlinesCount = 5 ;
			uint8_t attr = 0 ;
			EditType = EE_MODEL ;
			
   		lcd_puts_Pleft( y, XPSTR( "Configure" ) ) ;
			if ( sub == subN )
			{
				lcd_char_inverse( 0, y, 9*FW, 0 ) ;
	 			if ( checkForMenuEncoderBreak( event ) )
				{
					SubMenuCall = 0x80 ;
   		   	pushMenu( menuProcMusic ) ;
 			  	s_editMode = 0 ;
   		    killEvents(event);
				}
			}
			y += FH ;
			subN++ ;

  	  if(sub==subN)
			{
	   		attr = InverseBlink ;
			}
			lcd_puts_Pleft( y, XPSTR("Start Switch"));
			uint8_t doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			g_model.musicData.musicStartSwitch = edit_dr_switch( 16*FW, y, g_model.musicData.musicStartSwitch, attr, doedit, event ) ;
			y += FH ;
			subN++ ;
			attr = 0 ;
  	  if(sub==subN)
			{
	   		attr = InverseBlink ;
			}
			lcd_puts_Pleft( y, XPSTR("Pause Switch"));
			doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			g_model.musicData.musicPauseSwitch = edit_dr_switch( 16*FW, y, g_model.musicData.musicPauseSwitch, attr, doedit, event ) ;
			y += FH ;
			subN++ ;
			attr = 0 ;
  	  if(sub==subN)
			{
	   		attr = InverseBlink ;
			}
			lcd_puts_Pleft( y, XPSTR("Previous Switch"));
			doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			g_model.musicData.musicPrevSwitch = edit_dr_switch( 16*FW, y, g_model.musicData.musicPrevSwitch, attr, doedit, event ) ;
			y += FH ;
			subN++ ;
			attr = 0 ;
  	  if(sub==subN)
			{
	   		attr = InverseBlink ;
			}
			lcd_puts_Pleft( y, XPSTR("Next Switch"));
			doedit = attr ? EDIT_DR_SWITCH_MOMENT | EDIT_DR_SWITCH_EDIT : EDIT_DR_SWITCH_MOMENT ;
			g_model.musicData.musicNextSwitch = edit_dr_switch( 16*FW, y, g_model.musicData.musicNextSwitch, attr, doedit, event ) ;
		}
		break ;
		
		case M_TIMERS :
		{	
			uint8_t subN = 0 ;
			uint32_t t ;
			uint32_t secs ;
			div_t qr ;
      TITLE( PSTR( STR_TIMER ) ) ;
#ifdef HAPTIC
			IlinesCount = 17 ;
#else
			IlinesCount = 15 ;
#endif
			 
#ifdef HAPTIC
			if ( sub < 16 )
#else
			if ( sub < 14 )
#endif
			{
				lcd_puts_P( 20*FW-4, 7*FH, XPSTR("->") ) ;
#ifdef HAPTIC
				lcd_putcAtt( 6*FW, 0, ( sub < 8 ) ? '1' : '2', BLINK ) ;
#else
				lcd_putcAtt( 6*FW, 0, ( sub < 7 ) ? '1' : '2', BLINK ) ;
#endif
				editTimer( sub, event ) ;
			}
			else
			{
#ifdef HAPTIC
				subN = 16 ;
#else
				subN = 14 ;
#endif
				lcd_puts_Pleft( y, PSTR( STR_TOTAL_TIME ));
				t = g_model.totalTime ;
				secs = t % 60 ;
				t = t / 60 ;	// minutes
				qr = div( t, 60 ) ;
				lcd_outdezNAtt( 20*FW-3, y, secs, LEADING0, 2 ) ;
				lcd_putcAtt(17*FW, y, ':', 0 ) ;
				lcd_outdezNAtt( 17*FW, y, (uint16_t)qr.rem, LEADING0, 2 ) ;
				lcd_putcAtt(14*FW+3, y, ':', 0 ) ;
				lcd_outdezAtt( 14*FW+3, y, (uint16_t)qr.quot, 0 ) ;

      	if(sub==subN)
				{
					if (event == EVT_KEY_LONG(KEY_MENU) )
					{
						g_model.totalTime = 0 ;
    		    killEvents( event ) ;
					}
				}
			}
		}
		break ;

		case M_HELI :
		{	
			uint8_t subN = 0 ;
  		uint8_t attr ;
			IlinesCount = 6 ;
			TITLE( PSTR(STR_HELI_SETUP) ) ;
	  	
			lcd_puts_Pleft(    y, PSTR(STR_HELI_TEXT));
			g_model.swashType = checkIndexed( y, PSTR(SWASH_TYPE_STR), g_model.swashType, (sub==subN) ) ;
			y += FH ;
			subN += 1 ;
		
			attr = 0 ;
			if(sub==subN)
			{
				attr = blink ;
				uint8_t x = mapPots( g_model.swashCollectiveSource ) ;
				CHECK_INCDEC_H_MODELVAR_0( x, NUM_SKYXCHNRAW+NumExtraPots ) ;
				g_model.swashCollectiveSource = unmapPots( x ) ;
			}
    	putsChnRaw(17*FW, y, g_model.swashCollectiveSource, attr ) ;
			y += FH ;
			subN += 1 ;

			attr = 0 ;
  	  if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.swashRingValue, 100) ; }
    	lcd_outdezAtt(20*FW, y, g_model.swashRingValue, attr ) ;
			y += FH ;
			subN += 1 ;

			g_model.swashInvertELE = hyphinvMenuItem( g_model.swashInvertELE, y, sub==subN ) ;
			y += FH ;
			subN += 1 ;
			
			g_model.swashInvertAIL = hyphinvMenuItem( g_model.swashInvertAIL, y, sub==subN ) ;
			y += FH ;
			subN += 1 ;

			g_model.swashInvertCOL = hyphinvMenuItem( g_model.swashInvertCOL, y, sub==subN ) ;
		}
		break ;

		case M_MGENERAL :
		{	
			uint8_t subN = 0 ;
			uint8_t attr = 0 ;
			static uint8_t voiceCall = 0 ;			
      TITLE( XPSTR(Str_General) ) ;

#ifndef SMALL
			lcd_puts_Pleft( 0, XPSTR("\016(ver  )") ) ;
//    	lcd_putc( 18*FW, 0, '(' ) ;
//    	lcd_putc( 20*FW, 0, ')' ) ;
    	lcd_putc( 19*FW, 0, g_model.modelVersion + '0' ) ;
#endif

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
 #if defined(PCBX7) || defined (PCBXLITE) || defined(PCBT12) || defined(PCBX9LITE) || defined(PCBLEM1)
			IlinesCount = 21 ;//+ 1 ;
 #else
			IlinesCount = 23 ;//+ 1 ;
      #define SCRIPT_CHOICE	1
 #endif
#else // PCBX9D
 #ifdef IMAGE_128
			IlinesCount = 22 ;//+ 1 ;
 #else
  #if defined(PCBX7) || defined (PCBXLITE)
			IlinesCount = 18 ;//+ 1 ;
  #else
			IlinesCount = 21 ;//+ 1 ;
  #endif
 #endif
#endif // PCBX9D

			if ( voiceCall )
			{
				if ( FileSelectResult == 1 )
				{
#if defined(PCBX9D) || defined(IMAGE_128) || defined(PCBX12D) || defined(PCBX10)
					if ( voiceCall == 2 )
					{
						copyFileName( g_model.modelImageName, SelectedVoiceFileName, 10 ) ;
						loadModelImage() ;
					}
					else
#endif
					if ( voiceCall == 3 )
					{
						copyFileName( (char *)g_model.backgroundScript, SelectedVoiceFileName, 6 ) ;
						basicLoadModelScripts() ;
					}
					else
					{
						g_model.modelVoice = -1 ;
						copyFileName( g_model.modelVname, SelectedVoiceFileName, 8 ) ;
					}
	  		  eeDirty(EE_MODEL) ;		// Save it
					FileSelectResult = 0 ;
				}
				voiceCall = 0 ;
			}

//#ifdef TRIMS_SCALED
// #define EXTRA_GENERAL	1
//#else
 #define EXTRA_GENERAL	0
//#endif

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
			if ( sub < 18 + EXTRA_GENERAL )
#else
			if ( sub < 18 + EXTRA_GENERAL )
#endif
			{
				displayNext() ;
			}
			if ( sub < 7 + EXTRA_GENERAL )
			{
				SubMenuCall = 0x80 ;
				alphaEditName( 11*FW-2, y, (uint8_t *)g_model.name, sizeof(g_model.name), sub==subN, (uint8_t *)XPSTR( "Model Name") ) ;
				validateName( (uint8_t *)g_model.name, sizeof(g_model.name) ) ;
				
				y += FH ;
				subN += 1 ;

    		if(sub==subN)
				{
					if ( checkForMenuEncoderLong( event ) )
					{
						voiceCall = 1 ;
						SubMenuCall = 0x81 ;
						VoiceFileType = VOICE_FILE_TYPE_NAME ;
    		   	pushMenu( menuProcSelectVoiceFile ) ;				
					}
					if ( g_posHorz == 0 )
					{
						attr = InverseBlink ;
  			    CHECK_INCDEC_H_MODELVAR( g_model.modelVoice, -1, 49 ) ;
					}
				}
				
				if(sub==subN)
				{
					SubMenuCall = 0xA1 ;
				}
//				uint8_t type = ( (sub==subN) && (g_posHorz) ) ? EE_MODEL : 0 ;
				lcd_puts_Pleft( y,XPSTR("Voice File"));
				uint8_t type = (sub==subN) ;
				alphaEditName( 11*FW-2, y, (uint8_t *)g_model.modelVname, sizeof(g_model.modelVname), type | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FileName") ) ;
				validateName( (uint8_t *)g_model.modelVname, sizeof(g_model.modelVname) ) ;

				 
//				if ( g_model.modelVoice == -1 )
//				{
//					if(sub==subN)
//					{
//						Columns = 1 ;
//						SubMenuCall = 0xA1 ;
//					}
//					uint8_t type = ( (sub==subN) && (g_posHorz) ) ? EE_MODEL : 0 ;
//					alphaEditName( 9*FW-2, y, (uint8_t *)g_model.modelVname, sizeof(g_model.modelVname), type, (uint8_t *)XPSTR( "FileName") ) ;
//				}
//				else
//				{
//					if (event == EVT_KEY_BREAK(KEY_MENU) )
//					{
//						if(sub==subN)
//						{
//							if ( g_eeGeneral.forceMenuEdit == 0 )
//							{
//								putVoiceQueue( ( g_model.modelVoice + 260 ) | VLOC_NUMUSER  ) ;
//								s_editMode = 0 ;
//							}
//						}
//					}
//				}
//    		lcd_puts_Pleft( y, PSTR(STR_VOICE_INDEX) ) ;
//				if ( g_model.modelVoice == -1 )
//				{
//    			lcd_putcAtt( 7*FW+2, y, '>', attr) ;
//				}
//				else
//				{
//    			lcd_outdezAtt(  10*FW+2, y, (int16_t)g_model.modelVoice + 260 ,attr);
//				}
				y += FH ;
				subN += 1 ;

				uint8_t subSub = g_posHorz ;
				g_model.thrTrim = onoffMenuItem( g_model.thrTrim, y, PSTR(STR_T_TRIM), ( sub==subN ) && (subSub==0 ) ) ;
				attr = 0 ;
				uint8_t t = 100 - g_model.throttleIdleScale ;
				if ( sub==subN )
				{
					Columns = 1 ;
					if ( subSub == 1 )
					{
						attr = InverseBlink ;
						CHECK_INCDEC_H_MODELVAR_0( t, 100 ) ;
						g_model.throttleIdleScale = 100 - t ;
					}
				}
				lcd_outdezAtt(  127, y, t, attr ) ;
				y += FH ;
				subN += 1 ;
			
				g_model.thrExpo = onoffMenuItem( g_model.thrExpo, y, PSTR(STR_T_EXPO), sub==subN) ;
				y += FH ;
				subN += 1 ;

				lcd_puts_Pleft( y,PSTR(STR_LIGHT_SWITCH));
  			attr = 0 ;
  			if(sub==subN) { attr = InverseBlink ; }
				g_model.mlightSw = edit_dr_switch( 17*FW, y, g_model.mlightSw, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				if ( g_model.mlightSw == 0 )
				{
					putsMomentDrSwitches( 14*FW, y, g_eeGeneral.lightSw, 0 ) ;
    			lcd_putcAtt( 12*FW, y, '(', 0 ) ;
    			lcd_putcAtt( 17*FW, y, ')', 0 ) ;
				}
				y += FH ;
				subN += 1 ;

				g_model.useCustomStickNames = onoffMenuItem( g_model.useCustomStickNames, y, PSTR( STR_CUSTOM_STK_NAMES ), sub==subN) ;
				y += FH ;
				subN += 1 ;
				lcd_puts_Pleft(    y, PSTR(STR_TRIM_INC));
				
				g_model.trimInc = checkIndexed( y, PSTR(STR_TRIM_OPTIONS), g_model.trimInc, (sub==subN) ) ;
//				y += FH ;
//				subN += 1 ;

			}


#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
			else if ( sub < 13 + EXTRA_GENERAL )
#else
			else if ( sub < 13 + EXTRA_GENERAL )
#endif
			{
				subN = 7 + EXTRA_GENERAL ;

  			attr = 0 ;
				lcd_puts_Pleft( y, PSTR(STR_TRIM_SWITCH));
  			if(sub==subN) { attr = InverseBlink ; }
				g_model.trimSw = edit_dr_switch( 17*FW, y, g_model.trimSw, attr, attr ? EDIT_DR_SWITCH_EDIT : 0, event ) ;
				y += FH ;
				subN += 1 ;

				lcd_puts_Pleft( y, XPSTR(" Modify:")) ;
				g_model.instaTrimToTrims = checkIndexed( y, XPSTR(FWx13"\001""\010SubTrims   Trims"), g_model.instaTrimToTrims, sub==subN ) ;
				y += FH ;
				subN += 1 ;
	 
				g_model.extendedLimits = onoffMenuItem( g_model.extendedLimits, y, PSTR(STR_E_LIMITS), sub==subN) ;
    //---------------------------------                           /* ReSt V */
    //   If extended Limits are switched off, min and max limits must be limited to +- 100         
        if (!g_model.extendedLimits)
        {
			    for(uint8_t i=0; i<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS; i++)
          {
      	    LimitData *ld = &g_model.limitData[i] ;
#if EXTRA_SKYCHANNELS
						if ( i >= NUM_SKYCHNOUT )
						{
							ld = &g_model.elimitData[i-NUM_SKYCHNOUT] ;
						}
#endif			
            if (ld->min < 0) ld->min = 0;
            if (ld->max > 0) ld->max = 0;
          }
        }                                 
    //-----------------------------------                           /* ReSt A */

				
				y += FH ;
				subN += 1 ;

				uint8_t oldValue = g_model.throttleReversed ;
				g_model.throttleReversed = onoffMenuItem( oldValue, y, PSTR(STR_THR_REVERSE), sub == subN ) ;
				if ( g_model.throttleReversed != oldValue )
				{
  				checkTHR() ;
				}
 				y += FH ;
				lcd_putc( 15*FW, y, throttleReversed() ? '\201' : '\200' ) ;
    		lcd_puts_Pleft( y, PSTR( STR_THROTTLE_OPEN ) ) ;
 				
				y += FH ;
				subN += 1 ;

  			attr = 0 ;
				oldValue = g_model.throttleIdle ;
    		lcd_puts_Pleft( y, PSTR( STR_THR_DEFAULT ) ) ;
				g_model.throttleIdle = checkIndexed( y, XPSTR(FWx15"\001""\006   EndCentre"), oldValue, sub==subN ) ;
				if ( g_model.throttleIdle != oldValue )
				{
  				checkTHR() ;
				}
//#if !defined(PCBX9D) && !defined(PCBX12D)
				y += FH ;
				subN += 1 ;

    		lcd_puts_Pleft( y, PSTR( STR_CUSTOM_CHECK ) ) ;
				if ( sub == subN )
				{
					lcd_char_inverse( 0, y, 12*FW, 0 ) ;
					if (event == EVT_KEY_BREAK(KEY_MENU) )
					{
						SubMenuCall = 0x80 + 12 ;
    		   	pushMenu( menuCustomCheck ) ;
  			  	s_editMode = 0 ;
    		    killEvents(event);
					}
				}
//#endif
			}

 
 #if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
			else if ( sub < 18 + EXTRA_GENERAL )
 #else
			else if ( sub < 18 + EXTRA_GENERAL )
 #endif
			{
 #if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
				subN = 13 + EXTRA_GENERAL ;
#else			
				subN = 13 + EXTRA_GENERAL ;
#endif
  			attr = PREC1 ;
  			lcd_puts_Pleft(    y, PSTR(STR_AUTO_LIMITS));
  			if(sub==subN) { attr = InverseBlink | PREC1 ; CHECK_INCDEC_H_MODELVAR_0( g_model.sub_trim_limit, 100 ) ; }
  			lcd_outdezAtt(  20*FW, y, g_model.sub_trim_limit, attr ) ;
					 
				y += FH ;
				subN += 1 ;
#ifdef REV9E
				uint32_t states = g_model.modelswitchWarningStates ;
#else				
				uint16_t states = g_model.modelswitchWarningStates ;
#endif

				uint8_t b = (states & 1) ;
				b = offonMenuItem( b, y, PSTR(STR_SWITCH_WARN), sub==subN ) ;
    		g_model.modelswitchWarningStates = (states & ~1) | b ;
				y += FH ;
				subN += 1 ;

#if defined(PCBSKY) || defined(PCB9XT)
				uint8_t enables = 0x3F ;
				uint16_t temp = g_model.modelswitchWarningDisables ;

				if ( temp & THR_WARN_MASK )
				{
					enables &= ~0x01 ;
				}
				if ( temp & RUD_WARN_MASK )
				{
					enables &= ~0x02 ;
				}
				if ( temp & ELE_WARN_MASK )
				{
					enables &= ~0x04 ;
				}
				if ( temp & IDX_WARN_MASK )
				{
					enables &= ~0x08 ;
				}
				if ( temp & AIL_WARN_MASK )
				{
					enables &= ~0x10 ;
				}
				if ( temp & GEA_WARN_MASK )
				{
					enables &= ~0x20 ;
				}

				uint8_t subSub = g_posHorz ;
    		for(uint8_t i=0;i<6;i++)
				{
					lcd_putsnAtt((11+i)*(FW+1), y, XPSTR("TREIAG")+i,1, ((enables & (1<<i)) ? INVERS : 0 ) );
					if ( (sub==subN) )
					{
						if ( ( (subSub)==i ) && ( BLINK_ON_PHASE ) )
						{
							lcd_rect( (11+i)*(FW+1)-1, y-1, FW+2, 9 ) ;
						}
					}
				}
    		if(sub==subN)
				{
					Columns = 5 ;
        	if((event==EVT_KEY_FIRST(KEY_MENU)) || (event == EVT_KEY_BREAK(BTN_RE)) || P1values.p1valdiff)
					{
            killEvents(event);
            s_editMode = false;
            enables ^= (1<<(subSub));
            STORE_MODELVARS;
	        }
  		  }
				temp = 0 ;
				if ( ( enables & 1 ) == 0 ) temp |= THR_WARN_MASK ;
				if ( ( enables & 2 ) == 0 ) temp |= RUD_WARN_MASK ;
				if ( ( enables & 4 ) == 0 ) temp |= ELE_WARN_MASK ;
				if ( ( enables & 8 ) == 0 ) temp |= IDX_WARN_MASK ;
				if ( ( enables & 0x10 ) == 0 ) temp |= AIL_WARN_MASK ;
				if ( ( enables & 0x20 ) == 0 ) temp |= GEA_WARN_MASK ;
				g_model.modelswitchWarningDisables = temp ;
				subN += 1 ;
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
 #if defined(PCBX12D) || defined(PCBX10)
				uint8_t enables = 0xFF ;
 #else
				uint8_t enables = 0x7F ;
 #endif
				uint16_t temp = g_model.modelswitchWarningDisables ;

				if ( temp & 0x0003 )
				{
					enables &= ~0x01 ;
				}
				if ( temp & 0x000C )
				{
					enables &= ~0x02 ;
				}
				if ( temp & 0x0030 )
				{
					enables &= ~0x04 ;
				}
				if ( temp & 0x00C0 )
				{
					enables &= ~0x08 ;
				}
				if ( temp & 0x0300 )
				{
					enables &= ~0x10 ;
				}
				if ( temp & 0x0C00 )
				{
					enables &= ~0x20 ;
				}
				if ( temp & 0x3000 )
				{
					enables &= ~0x40 ;
				}
 #if defined(PCBX12D) || defined(PCBX10)
				if ( temp & 0xC000 )
				{
					enables &= ~0x80 ;
				}
 #endif
				uint8_t subSub = g_posHorz ;
 #if defined(PCBX12D) || defined(PCBX10)
    		for(uint8_t i=0;i<8;i++)
 #else
    		for(uint8_t i=0;i<7;i++)
 #endif
				{
 #if defined(PCBLEM1)
					if ( i != 4)
 #endif // PCBLEM1
 #ifdef PCBX7
  #ifdef PCBT12
					if ( i != 4)
  #else
					if ( ( i != 4) && ( i != 6 ) )
  #endif
 #endif // PCBX7
 #ifdef PCBXLITE
					if ( i < 4 )
 #endif // PCBXLITE
 #ifdef PCBX9LITE
					if ( ( i != 3) && ( i != 4) && ( i != 6 ) )
 #endif // PCBX9LITE
					{
						uint8_t attr = 0 ;
						if ( (enables & (1<<i)) )
						{
							attr = INVERS ;
						}
						if ( sub == subN )
						{
 #ifdef PCBX7
  #ifdef PCBT12
							if ( ( ( i < 4 ) && ( subSub==i ) ) || ( (i == 5) && (subSub == 4) ) || ( (i == 6) && (subSub == 5) ) )
  #else
							if ( ( subSub==i ) || ( (i == 5) && (subSub == 4) ) )
  #endif
							{
								attr = BLINK ;	
							}
 #else // PCBX7
  #ifdef PCBXLITE
							if ( subSub==i )
							{
								attr = BLINK ;	
							}
  #else // PCBXLITE
   #ifdef PCBX9LITE
							if ( ( ( i < 3 ) && ( subSub==i ) ) || ( (i == 5) && (subSub == 3) ) )
   #else // PCBX9LITE
    #if defined(PCBLEM1)
							if ( ( ( i < 4 ) && ( subSub==i ) ) || ( (i == 5) && (subSub == 4) ) || ( (i == 6) && (subSub == 5) ) )
		#else					
							if ( subSub==i )
    #endif // PCBLEM1
   #endif // PCBX9LITE
							{
								attr = BLINK ;	
							}
  #endif // PCBX9LITE
 #endif // PCBX7
						}
 #if defined(PCBT12) || defined(PCBLEM1)
						lcd_putsnAtt((13+i)*FW, y, XPSTR("ABCDEGH")+i,1, attr );
 #else
						lcd_putsnAtt((13+i)*FW, y, XPSTR("ABCDEFG6")+i,1, attr );
 #endif
					}
				}
    		if(sub==subN)
				{
 #ifdef PCBX7
  #ifdef PCBT12
					Columns = 5 ;
  #else
					Columns = 4 ;
  #endif
 #else // PCBX7
  #if defined(PCBXLITE) || defined(PCBX9LITE)
					Columns = 3 ;
  #else // PCBXLITE/X3
   #if defined(PCBX12D) || defined(PCBX10)
					Columns = 7 ;
   #else
					Columns = 6 ;
   #endif // PCBX12D
  #endif // PCBXLITE/X3
 #endif // PCBX7
 #if defined(PCBLEM1)
					Columns = 5 ;
 #endif // PCBLEM1
        	if((event==EVT_KEY_FIRST(KEY_MENU)) || (event == EVT_KEY_BREAK(BTN_RE)) || P1values.p1valdiff)
					{
						uint32_t shift = subSub ;
 #ifdef PCBX7
						if ( shift >= 4 )
						{
							shift += 1 ;
						}
 #endif // PCBX7
 #if defined(PCBLEM1)
						if ( shift >= 4 )
						{
							shift += 1 ;
						}
 #endif // PCBLEM1
//#ifdef PCBXLITE
//						if ( ( shift == 2 ) && (g_eeGeneral.ailsource == 0) )
//						{
//							shift = 5 ;
//						}
//#endif // PCBXLITE
            killEvents(event);
            s_editMode = false;
            enables ^= (1<<(shift));
            STORE_MODELVARS;
	        }
  		  }
				temp = 0 ;
				if ( ( enables & 1 ) == 0 )    temp |= 0x0003 ;
				if ( ( enables & 2 ) == 0 )    temp |= 0x000C ;
				if ( ( enables & 4 ) == 0 )    temp |= 0x0030 ;
				if ( ( enables & 8 ) == 0 )    temp |= 0x00C0 ;
				if ( ( enables & 0x10 ) == 0 ) temp |= 0x0300 ;
				if ( ( enables & 0x20 ) == 0 ) temp |= 0x0C00 ;
				if ( ( enables & 0x40 ) == 0 ) temp |= 0x3000 ;
 #if defined(PCBX12D) || defined(PCBX10)
				if ( ( enables & 0x80 ) == 0 ) temp |= 0xC000 ;
 #endif
				g_model.modelswitchWarningDisables = temp ;
				subN += 1 ;
#endif
		    lcd_puts_Pleft(    y, PSTR(STR_DEAFULT_SW)) ;
		 		states = g_model.modelswitchWarningStates >> 1 ;
 #ifdef PCBT12
				if ( states & 0x4000 )
				{
					states &= 0x0FFF ;
					states |= 0x2000 ;
				}
//				lcd_outhex4(0,0,g_model.modelswitchWarningStates) ;
//				lcd_outhex4(30,0,states) ;
 #endif
 #ifdef REV9E
				states |= (uint32_t)g_model.xmodelswitchWarningStates << 14 ;
 #endif
        
 #if defined(PCBSKY) || defined(PCB9XT)
				y += FH ;
				uint16_t index ;
						
				index = oneSwitchText( HSW_ThrCt, states ) ;
				attr = index >> 8 ;
				index &= 0x00FF ;
				putSwitchName(0*FW+5, y, index, attr ) ;
		
				index = oneSwitchText( HSW_RuddDR, states ) ;
				attr = index >> 8 ;
				index &= 0x00FF ;
				putSwitchName(3*FW+7, y, index, attr ) ;
		
				index = oneSwitchText( HSW_ElevDR, states ) ;
				attr = index >> 8 ;
				index &= 0x00FF ;
				putSwitchName(6*FW+9, y, index, attr ) ;

				index = oneSwitchText( HSW_ID0, states ) & 0x00FF ;
				putSwitchName(9*FW+11, y, index, 0 ) ;

				index = oneSwitchText( HSW_AileDR, states ) ;
				attr = index >> 8 ;
				index &= 0x00FF ;
				putSwitchName(12*FW+13, y, index, attr ) ;
		
				index = oneSwitchText( HSW_Gear, states ) ;
				attr = index >> 8 ;
				index &= 0x00FF ;
				putSwitchName(15*FW+15, y, index, attr ) ;
		    
				if(sub==subN)
				{
					lcd_rect( 0*FW+4-1, y-1, 18*FW+2+12, 9 ) ;
 #endif
 #if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
				y += FH ;
    		for (uint8_t i=0 ; i<7 ; i += 1 )
				{
  #ifdef PCBX7
   #ifdef PCBT12
					if ( i != 4)
   #else
					if ( ( i != 4) && ( i != 6 ) )
   #endif
  #endif // PCBX7
  #ifdef PCBLEM1
					if ( i != 4)
  #endif // PCBLEM1
  #ifdef PCBXLITE
					if ( i < 4 )
  #endif // PCBXLITE
  #ifdef PCBX9LITE
					if ( ( i != 3) && ( i != 4) && ( i != 6 ) )
  #endif
					{
  #ifdef PCBT12
						uint32_t j ;
						j = i ;
						if ( j > 4 )
						{
							j += 1 ;
						}
	    		  lcd_putc( 2*FW+i*(2*FW+3), y, 'A'+j ) ;
  #else
   #ifdef PCBLEM1
						uint32_t j ;
						j = i ;
						if ( j > 4 )
						{
							j += 1 ;
						}
	    		  lcd_putc( 2*FW+i*(2*FW+3), y, 'A'+j ) ;
   #else
	    		  lcd_putc( 2*FW+i*(2*FW+3), y, 'A'+i ) ;
   #endif
  #endif
						lcd_putc( 3*FW+i*(2*FW+3), y, PSTR(HW_SWITCHARROW_STR)[states & 0x03] ) ;
					}
    		  states >>= 2 ;
    		}
  #if defined(PCBX12D) || defined(PCBX10)
   		  lcd_putc( 2*FW+7*(2*FW+3), y, '6' ) ;
extern uint32_t switches_states ;
				lcd_putc( 3*FW+7*(2*FW+3), y, ( (switches_states >> 16) & 0x07) + '0' ) ;
  #endif
    		if(sub==subN)
				{
  #if defined(PCBX12D) || defined(PCBX10)
					lcd_rect( 2*FW-2, y-1, 17*FW+2+18+1, 9 ) ;
  #else
					lcd_rect( 2*FW-2, y-1, 14*FW+2+18+1, 9 ) ;
  #endif
 #endif
		      if (event==EVT_KEY_FIRST(KEY_MENU) || event==EVT_KEY_FIRST(BTN_RE))
					{
    		    killEvents(event);
 #if defined(PCBSKY) || defined(PCB9XT)
			      g_model.modelswitchWarningStates = (getCurrentSwitchStates() << 1 ) ;// states ;
 #endif
 #if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
			  		getMovedSwitch() ;	// loads switches_states
  #ifdef REV9E
extern uint32_t switches_states ;
				    g_model.modelswitchWarningStates = ((switches_states & 0x3FFF) << 1 ) ;// states ;
				    g_model.xmodelswitchWarningStates = (switches_states >> 14 ) ;// states ;
  #else
extern uint32_t switches_states ;
   #if defined(PCBT12)
						uint32_t ss = switches_states ;
						if ( ss & 0x8000 )
						{
							ss &= ~0x3000 ;
							ss |= 0x2000 ;
						}

				    g_model.modelswitchWarningStates = ((ss & 0x7FFF) << 1 ) ;// states ;
   #else
	  #if defined(PCBLEM1)
						uint32_t ss = switches_states & ~0x0F00 ;
						if ( ss & 0x2000 )
						{
							ss &= ~0x3C00 ;
							ss |= 0x0800 ;
						}
						if ( ss & 0x8000 )
						{
							ss &= ~0xF000 ;
							ss |= 0x2000 ;
						}
				    g_model.modelswitchWarningStates = ((ss & 0x7FFF) << 1 ) ;// states ;
    #else
				    g_model.modelswitchWarningStates = ((switches_states & 0x3FFF) << 1 ) ;// states ;
    #endif
    #if defined(PCBX12D) || defined(PCBX10)
				    g_model.modelswitchWarningStates = ((switches_states & 0x7FFF) << 1 ) ;// states ;
 						g_model.xmodelswitchWarningStates = switches_states >> 15 ; 
    #endif
   #endif
  #endif
 #endif
    		    s_editMode = false ;
		        STORE_MODELVARS ;
					}
//#ifdef PCBX12D
//extern uint32_t switches_states ;
//					lcd_outhex4( 140, y, switches_states >> 16 ) ;
//					lcd_outhex4( 166, y, switches_states ) ;
//#endif
				}
				y += FH ;
				subN += 1 ;

		  	attr = 0 ;
    		lcd_puts_Pleft(    y, PSTR(STR_VOLUME_CTRL));
		    if(sub==subN) { attr = blink ; CHECK_INCDEC_H_MODELVAR_0( g_model.anaVolume, 7 ) ; }

#if defined(PCBSKY) || defined(PCB9XT)
		    lcd_putsAttIdx( 17*FW, y, XPSTR("\003---P1 P2 P3 GV4GV5GV6GV7"),g_model.anaVolume, attr ) ;
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
#ifdef PCBX7
#ifdef PCBT12
    		lcd_putsAttIdx( 17*FW, y, XPSTR("\004----AUX4AUX5GV3 GV4 GV5 GV6 GV7 "),g_model.anaVolume, attr ) ;
#else
    		lcd_putsAttIdx( 17*FW, y, XPSTR("\003---S1 S2 GV3GV4GV5GV6GV7"),g_model.anaVolume, attr ) ;
#endif
#else // PCBX7
 #if defined(PCBX9LITE)
    		lcd_putsAttIdx( 17*FW, y, XPSTR("\003---S1 GV2GV3GV4GV5GV6GV7"),g_model.anaVolume, attr ) ;
 #else // PCBX9LITE
    		lcd_putsAttIdx( 17*FW, y, XPSTR("\003---S1 S2 SL SR GV5GV6GV7"),g_model.anaVolume, attr ) ;
 #endif // PCBX9LITE
#endif // PCBX7
#endif
#if defined(PCBLEM1)
    		lcd_putsAttIdx( 17*FW, y, XPSTR("\003---S1 GV2GV3GV4GV5GV6GV7"),g_model.anaVolume, attr ) ;
#endif
				y += FH ;
				subN += 1 ;

//#ifndef PCBLEM1
 
//#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
			}
			else
			{
				subN = 18 ;//+ 1 ;
//#endif

#ifdef REV9E
				uint8_t subSub = g_posHorz ;
#else
 #if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
				uint8_t subSub = g_posHorz ;
 #else
				uint8_t subSub = g_posHorz ;
//				subSub = g_posHorz ;
 #endif
#endif
		    lcd_puts_Pleft(    y, PSTR(STR_BEEP_CENTRE));
#if defined(PCBSKY) || defined(PCB9XT)
				uint32_t width = NumExtraPots ? 7 : 6 ;
    		for(uint8_t i=0;i<=width;i++)
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
#if defined(PCBX7) || defined (PCBXLITE)
    		for(uint8_t i=0;i<6;i++)
#else // PCBX7
 #if defined(PCBX9LITE) || defined(PCBLEM1)
    		for(uint8_t i=0;i<5;i++)
 #else // PCBX9LITE
    		for(uint8_t i=0;i<8;i++)
 #endif // PCBX9LITE
#endif // PCBX7
#endif
				{
					lcd_putsnAtt((10+i)*FW, y, PSTR(STR_RETA123)+i,1, (((subSub)==i) && (sub==subN)) ? BLINK : ((g_model.beepANACenter & (1<<i)) ? INVERS : 0 ) );
				}
    		if(sub==subN)
				{
#if defined(PCBSKY) || defined(PCB9XT)
					Columns = width ;
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
					Columns = 5 ;
#if defined(PCBX7) || defined (PCBXLITE)
#else // PCBX7
 #if defined(PCBX9LITE)
					Columns = 4 ;
 #else // PCBX9LITE
					Columns = 7 ;
 #endif // PCBX9LITE
#endif // PCBX7
#endif
#if defined(PCBLEM1)
					Columns = 4 ;
#endif // PCBLEM1
        	if((event==EVT_KEY_FIRST(KEY_MENU)) || (event == EVT_KEY_BREAK(BTN_RE)) || P1values.p1valdiff)
					{
            killEvents(event);
            s_editMode = false;
            g_model.beepANACenter ^= (1<<(subSub));
            STORE_MODELVARS;
	        }
  		  }
#if defined(PCBX9D) || defined(IMAGE_128) || defined(PCBX12D) || defined(PCBX10)
//#ifdef PCBX9D
#ifndef PCBX7
#ifndef PCBXLITE
#ifndef PCBX9LITE
				y += FH ;
				subN += 1 ;
    		
				lcd_puts_Pleft( y, XPSTR( "Image Name") ) ;
				uint8_t type = ALPHA_NO_NAME ;
				if(sub==subN)
				{
					Columns = 1 ;
#if defined(PCBX12D) || defined(PCBX10)
					SubMenuCall = 0x93 ;
#else
					SubMenuCall = 0x92 ;
#endif
					type |= EE_MODEL ;
					if ( checkForMenuEncoderLong( event ) )
					{
						voiceCall = 2 ;
    		   	pushMenu( menuProcSelectImageFile ) ;				
					}
				}
				if ( voiceCall == 0 )
				{
					alphaEditName( 11*FW-2, y, (uint8_t *)g_model.modelImageName, sizeof(g_model.modelImageName), type | ALPHA_NO_NAME, (uint8_t *)XPSTR( "FIlename") ) ;
					validateName( (uint8_t *)g_model.modelImageName, sizeof(g_model.modelImageName) ) ;
				}
#endif // PCBX9LITE
#endif // PCBXLITE
#endif // PCBX7
#endif

				y += FH ;
				subN += 1 ;
				attr = ALPHA_NO_NAME ;
				lcd_puts_Pleft( y, XPSTR( "Bg Script") ) ;
//				uint8_t type = ALPHA_NO_NAME | EE_MODEL ;
				if(sub==subN)
				{
					attr |= blink ;
				}
				alphaEditName( 11*FW, y, g_model.backgroundScript, 6, attr, (uint8_t *)XPSTR( "Script File") ) ;
				validateName( g_model.backgroundScript, 6 ) ;
	  		if( attr & ~ALPHA_NO_NAME )
				{
					if ( AlphaEdited )
					{
						AlphaEdited = 0 ;
						basicLoadModelScripts() ;
					}
					if ( checkForMenuEncoderLong( event ) )
					{
						TextResult = 0 ;
						voiceCall = 3 ;
    		  	pushMenu( menuSelectModelScript ) ;
#if defined(PCBX9D) || defined(IMAGE_128) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
						SubMenuCall = 0x93 ;
#else
						SubMenuCall = 0x92 ;
#endif
					}
				} 
				y += FH ;
				subN += 1 ;
				g_model.disableThrottleCheck = onoffMenuItem( g_model.disableThrottleCheck, y, XPSTR("Disable Thr Chk"), sub==subN) ;
#ifdef SCRIPT_CHOICE				
				y += FH ;
				subN += 1 ;
				lcd_puts_Pleft( y, "Script Type");
				g_model.basic_lua = checkIndexed( y, XPSTR(FWx16"\001""\005Basic  LUA"), g_model.basic_lua, (sub==subN) ) ;
#endif
//#endif // n PCBLEM1
			}
		}	
		break ;
	}	
}

#ifdef BLUETOOTH
extern struct t_fifo128 BtRx_fifo ;
extern uint16_t BtRxTimer ;

void menuProcBt(uint8_t event)
{
  MENU(XPSTR("BT"), menuTabStat, e_bluetooth, 7, {0/*, 0*/});

#ifdef PCBSKY

extern uint16_t BtLastSbusSendTime ;
extern uint16_t BtParaDebug ;

	lcd_outhex4(50, 0, BtLastSbusSendTime ) ;
	lcd_outhex4(80, 0, BtParaDebug ) ;

 #ifdef BT_PDC
	uint16_t *p = (uint16_t *)&BtPdcFifo ;
 #else
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
 #endif
#endif
#ifdef PCB9XT
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
#endif
#if defined(PCBX7) || defined(PCBX9D) || (defined(PCBX10) && defined(PCBREV_EXPRESS))
	uint16_t *p = (uint16_t *)&BtRx_fifo ;
#endif
	uint32_t i ;
	uint16_t x ;
#ifndef SMALL
	p += mstate2.m_posVert * 5 ;
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 1*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 2*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 3*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 4*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 5*FH, x ) ;
	}
#else
	uint32_t j ;
	p += mstate2.m_posVert * 5 ;
	for ( j = 0 ; j < 5 ; j += 1 )
	{
		for ( i = 0 ; i < 5 ; i += 1 )
		{
			x = *p++ ;
			x = ( x >> 8 ) | ( x << 8 ) ;
			lcd_outhex4( i*25, (j+1)*FH, x ) ;
		}
	}
#endif	 
#ifndef PCB9XT
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x = ( x >> 8 ) | ( x << 8 ) ;
		lcd_outhex4( i*25, 6*FH, x ) ;
	}
#endif

	lcd_outdez( 25, 7*FH, BtControl.BtBadChecksum ) ;

//	lcd_outdez( 50, 7*FH, BtControl.BtLinking ) ;
	 
#ifdef BLUETOOTH
	lcd_outdez( 75, 7*FH, BtRxTimer ) ;
	lcd_outdez( 100, 7*FH, BtControl.BtCurrentLinkIndex ) ;
	lcd_outdez( 125, 7*FH, BtControl.BtBaudChangeIndex ) ;
#endif

#if defined(PCBX10) && defined(PCBREV_EXPRESS)

	lcd_outhex4( 25*FW, 1*FH, USART6->CR1 ) ;
	lcd_outhex4( 25*FW, 2*FH, USART6->SR ) ;
	lcd_outhex4( 25*FW, 3*FH, USART6->BRR ) ;
	lcd_outhex4( 25*FW, 4*FH, GPIOG->IDR ) ;


#endif
}

void menuProcBtAddressList(uint8_t event)
{
  lcd_puts_Pleft( 0, XPSTR("Bt Address List") ) ;
	static MState2 mstate2 ;
	if ( PopupData.PopupActive == 0 )
	{
		mstate2.check_columns(event, IlinesCount ) ;
	}
	
	uint8_t sub = mstate2.m_posVert ;
	if ( event == EVT_ENTRY_UP )
	{
		if ( AlphaEdited )
		{
			uint32_t i ;
			uint8_t *p ;
			uint8_t chr ;
			uint8_t value ;
			p = g_eeGeneral.btDevice[s_curItemIdx].address ;
			for ( i = 0 ; i < 12 ; i += 2 )
			{
				chr = Alpha.copyOfText[i] - '0' ;
				if ( chr > 9 )
				{
					chr -= 7 ;
				}
				value = chr << 4 ;
				chr = Alpha.copyOfText[i+1] - '0' ;
				if ( chr > 9 )
				{
					chr -= 7 ;
				}
				value |= (chr & 0x0F) ;
				*p++ = value ;
			}
		}
	}

	uint8_t addressText[16] ;
	uint32_t i ;
	uint32_t j ;
	btDeviceData *btData = &g_eeGeneral.btDevice[0] ;
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		uint8_t attr = 0 ;
		j = btData->address[0] | btData->address[1] | btData->address[2] ;
		if ( j )
		{
			if ( i == sub )
			{
				attr = INVERS ;
			}
			
			btAddrBin2Hex( addressText, btData->address, 1 ) ;
		  lcd_putsnAtt( 1, (i+1)*FH, (char *)addressText, 12, attr ) ;
			lcd_putsnAtt( 15*FW, (i+1)*FH, (char *)btData->name, 6, attr ) ;
			if ( g_model.btDefaultAddress == i )
			{
				lcd_rect( 0, (i+1)*FH-1, 128, 9 ) ;
			}
		}
		else
		{
			break ;
		}
		btData += 1 ;
	}
	IlinesCount = i ? i-1 : 0 ;
	if ( g_model.btDefaultAddress >= i )
	{
		g_model.btDefaultAddress -= 1 ;
	}

	if ( PopupData.PopupActive == 0 )
	{
		if ( checkForMenuEncoderBreak( event ) )
		{
			PopupData.PopupIdx = 0 ;
			PopupData.PopupActive = 1 ;
			killEvents(event) ;
			Tevent = event = 0 ;
			s_editMode = 0 ;
		}
	}

	if ( PopupData.PopupActive )
	{
		uint8_t popaction = doPopup( XPSTR("Set Default\0Edit Name\0Link\0Delete\0Edit MAC"), 0x1F, 12, event ) ;
  	if ( popaction == POPUP_SELECT )
		{
			uint8_t popidx = PopupData.PopupSel ;
			if ( popidx == 0 )	// Set Default
			{
				g_model.btDefaultAddress = sub ;
	      STORE_MODELVARS ;
			}
			if ( popidx == 1 )	// Edit Name
			{
				Tevent = EVT_KEY_BREAK(KEY_MENU) ;
				EditType = EE_GENERAL ;
				alphaEditName( 16*FW, (sub+1)*FH, g_eeGeneral.btDevice[sub].name, 6, 1, 0 ) ;
				EditType = EE_MODEL ;
			}
			if ( popidx == 2 )	// Link
			{
				BtControl.BtLinkRequest = 0x80 + sub ;
			}
			if ( popidx == 3 )	// Delete
			{
				for ( i = sub ; i < 3 ; i += 1 )
				{
					g_eeGeneral.btDevice[i] = g_eeGeneral.btDevice[i+1] ;
				}
    		memset(&g_eeGeneral.btDevice[3],0,sizeof(g_eeGeneral.btDevice[3]));
				if ( g_model.btDefaultAddress == sub )
				{
					if ( g_model.btDefaultAddress )
					{
						g_model.btDefaultAddress -= 1 ;
					}
				}
	      STORE_MODELVARS ;
        STORE_GENERALVARS ;
			}
			if ( popidx == 4 )	// Edit Mac
			{
				Alpha.AlphaHex = 1 ;
				Alpha.AlphaLength = 12 ;
				btAddrBin2Hex( Alpha.copyOfText, g_eeGeneral.btDevice[sub].address, 1 ) ;
				Alpha.PalphaText = Alpha.copyOfText ;
				Alpha.PalphaHeading = (uint8_t *)"Edit MAC" ;
				s_curItemIdx = sub ;
				pushMenu(menuProcAlpha) ;
			}
		}
	}
}

//extern uint8_t BtConfigure ;
void menuProcBtConfigure(uint8_t event)
{
	uint32_t i ;

  lcd_puts_Pleft( 0, XPSTR("Bt Configure") ) ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, 0 ) ;

	if ( event == EVT_ENTRY )
	{
		if ( BtControl.BtCurrentBaudrate != g_eeGeneral.bt_baudrate )
		{
			BtControl.BtBaudrateChanged = 1 ;
		}
		BtControl.BtConfigure = 0x80 ;
	}

	lcd_puts_Pleft( 2*FH, (BtControl.BtConfigure) ? XPSTR( "\006Configuring" ) : XPSTR("\007Configured") ) ;

	if ( BtControl.BtConfigure )
	{
		uint32_t width ;
		width = BtControl.BtConfigure & 0x3F ;
		if ( width > 17 )
		{
			width = 17 ;
		}
		width *= 4 ;
		lcd_hline( 0, 5*FH-1, 69 ) ;
		lcd_hline( 0, 6*FH, 69 ) ;
		lcd_vline( 68, 5*FH, 8 ) ;
		for ( i = 0 ; i <= width ; i += 1 )
		{
			lcd_vline( i, 5*FH, 8 ) ;
		}
	} 
}

//extern uint8_t BtScan ;
//extern uint8_t BtScanState ;
extern uint8_t BtTempBuffer[] ;
void menuProcBtScan(uint8_t event)
{
	uint32_t i ;
	uint32_t x ;
	uint32_t y ;
  lcd_puts_Pleft( 0, XPSTR("Bt Scan") ) ;
	static MState2 mstate2 ;
	mstate2.check_columns(event, 0 ) ;
	
	if ( event == EVT_ENTRY )
	{
		BtControl.BtScanState = 0 ;
		BtControl.BtScan = 1 ;
	}

	lcd_puts_Pleft( 2*FH, (BtControl.BtScanState < 3) ? XPSTR( "\006Preparing" ) : ( (BtControl.BtScanState < 4) ? XPSTR( "\006Scanning" ) : XPSTR("\007Scanned") ) )  ;
	
	if ( BtControl.BtScanState > 2 )
	{
		uint8_t chr ;
		x = 0 ;
		y = 3*FH ;
		i = 0 ;
		while ( (chr = BtTempBuffer[i]) )
		{
			i += 1 ;
			if ( chr == '\r' )
			{
				x = 0 ;
				y += FH ;
			}
			else
			{
				if ( chr >= ' ' )
				{
					if ( y < 7*FH )
					{
						lcd_putc( x, y, chr ) ;
						x += FW ;
						if ( x > 20*FW )
						{
							x = 0 ;
							y += FH ;
						}
					}
				}
			}
		}
		if ( BtControl.NumberBtremotes )
		{
			i = 0 ;
			x = 0 ;
			while ( (chr = BtRemote[0].name[i++] ) )
			{
				if ( chr >= ' ' )
				{
					lcd_putc( x, 7*FH, chr ) ;
					x += FW ;
				}
				if ( x > 14*FW )
				{
					break ;
				}
			}
		}
	}
	lcd_outhex4( 100, 0*FH, BtControl.BtScanState ) ;

}



#endif // BLUETOOTH

#ifdef PCB9XT
void menuProcSlave(uint8_t event)
{
  MENU(XPSTR("Slave"), menuTabStat, e_Slave, 1, {0/*, 0*/});

  if ( event == EVT_KEY_LONG(KEY_MENU) )
	{
		updateSlave() ;
		initM64() ;
	}

	lcd_puts_Pleft( 2*FH, XPSTR("Revision") ) ;
  lcd_outdezAtt( 14*FW, 2*FH, M64Revision, PREC1 ) ;
	lcd_puts_Pleft( 3*FH, XPSTR("Available") ) ;
	int32_t x = newSlaveRevision() ;
	lcd_outdezAtt( 14*FW, 3*FH, x, PREC1 ) ;
	lcd_puts_Pleft( 5*FH, XPSTR("Update  (MENU LONG)") ) ;
	lcd_puts_Pleft( 7*FH, XPSTR("Reset Count") ) ;

extern uint16_t RemBitMeasure ;
extern uint16_t M64UsedBackup ;
  lcd_outdez( 14*FW, 6*FH, M64UsedBackup ) ;
  lcd_outdez( 20*FW, 6*FH, RemBitMeasure ) ;

  lcd_outdez( 14*FW, 7*FH, M64ResetCount ) ;
  lcd_outdez( 20*FW, 7*FH, M64RestartCount ) ;

//extern uint16_t M64RxCount ;
//extern uint8_t M64MainTimer ;
//extern uint8_t SlaveState ;
//	lcd_outhex4( 17*FW, 2*FH, M64RxCount ) ;
//	lcd_outhex4( 17*FW, 3*FH, M64MainTimer ) ;
//	lcd_outhex4( 17*FW, 4*FH, SlaveState ) ;
}
#endif // PCB9XT

extern uint32_t ChipId ;
void menuProcBoot(uint8_t event)
{
  MENU(PSTR(STR_BOOT_REASON), menuTabStat, e_Boot, 1, {0/*, 0*/});

#if defined(PCBSKY) || defined(PCB9XT)
	lcd_puts_Pleft( 7*FH, XPSTR("Chip") ) ;
	lcd_outhex4( 5*FW, 7*FH, ChipId >> 16 ) ;
	lcd_outhex4( 9*FW, 7*FH, ChipId ) ;
#endif	

#ifdef PCBSKY
	if ( ( ResetReason & RSTC_SR_RSTTYP ) == (2 << 8) )	// Watchdog
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_6_WATCHDOG) ) ;
	}
	else if ( unexpectedShutdown )
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_5_UNEXPECTED) ) ;
		lcd_puts_Pleft( 3*FH, PSTR(STR_6_SHUTDOWN) ) ;
	}
	else
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_6_POWER_ON) ) ;
	}
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
#if defined(PCBLEM1)
	if ( ResetReason & RCC_CSR_IWDGRSTF )	// Watchdog
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_6_WATCHDOG) ) ;
#else
	if ( ResetReason & RCC_CSR_WDGRSTF )	// Watchdog
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_6_WATCHDOG) ) ;
#ifdef PCB9XT
extern uint32_t WatchdogPosition ;
	lcd_outhex4( 0, 3*FH, WatchdogPosition ) ;
#endif
#endif
	}
	else if ( unexpectedShutdown )
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_5_UNEXPECTED) ) ;
		lcd_puts_Pleft( 3*FH, PSTR(STR_6_SHUTDOWN) ) ;
	}
	else
	{
		lcd_puts_Pleft( 2*FH, PSTR(STR_6_POWER_ON) ) ;
	}
#endif
  lcd_outdez( 20*FW, 6*FH, ( ResetReason >> 8 ) & 7 ) ;
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	lcd_outhex4( 0, 6*FH, ResetReason >> 16 ) ;
	lcd_outhex4( 25, 6*FH, ResetReason ) ;
	lcd_outhex4( 0, 5*FH, RCC->BDCR ) ;
	lcd_outhex4( 25, 5*FH, RCC->CSR ) ;

#ifdef WDOG_REPORT
extern uint16_t WdogIntValue ;
	lcd_outhex4( 100, 5*FH, WdogIntValue ) ;
#endif

#endif

#ifdef WDOG_REPORT
#if defined(PCBX12D) || defined(PCBX10)
extern uint16_t WdogIntValue ;
 	lcd_putc( 90, 5*FH, '>') ;
	lcd_outhex4( 100, 5*FH, WdogIntValue ) ;
#endif
#endif

#ifdef STACK_PROBES
extern uint32_t stackSpace( uint32_t stack ) ;

	lcd_outhex4( 0, 4*FH, stackSpace(0) ) ;
	lcd_outhex4( 30, 4*FH, stackSpace(1) ) ;
	lcd_outhex4( 60, 4*FH, stackSpace(2) ) ;
#ifndef PCBX12D
 #ifndef PCBX10
	lcd_outhex4( 90, 4*FH, stackSpace(3) ) ;
#endif
#endif
#endif

}

#if defined(PCBX12D) || defined(PCBX10)

#define STATUS_VERTICAL		240

//extern uint8_t speaker[] ;
//extern const uint8_t SpeakerHiRes[] ;
const uint8_t SpeakerHiRes[] = {
8,16,0,
0xC0,0xC0,0xC0,0xE0,0xF0,0xF8,0xFC,0xFC,
0x07,0x07,0x07,0x0F,0x1F,0x3F,0x7F,0x7F
} ;




void displayStatusLine( uint32_t scriptPercent )
{
//	lcdDrawSolidFilledRectDMA( 0, STATUS_VERTICAL-2, 480, 18, LCD_STATUS_GREY ) ;
	lcdDrawSolidFilledRectDMA( 0, STATUS_VERTICAL, 480, 32, LCD_STATUS_GREY ) ;
	putsTime( 224, (STATUS_VERTICAL/2)+4, Time.hour*60+Time.minute, 0, 0, LCD_BLACK, LCD_STATUS_GREY ) ;

	lcd_HiResimg( 180*2, (STATUS_VERTICAL)-1+8, SpeakerHiRes, 0, 0, LCD_BLACK, LCD_STATUS_GREY ) ;
//	lcd_img( 180, (STATUS_VERTICAL/2), speaker, 0, 0, LCD_BLACK, LCD_STATUS_GREY ) ;
extern uint8_t CurrentVolume ;
	pushPlotType( PLOT_BLACK ) ;
	lcd_hbar( 185, (STATUS_VERTICAL/2)+1+4, 23, 6, (CurrentVolume*100+16)/23 ) ;
	popPlotType() ;

extern uint8_t LogsRunning ;
	if ( LogsRunning & 1 )
	{
		if ( BLINK_ON_PHASE )
		{
			lcdDrawIcon( 330, STATUS_VERTICAL+6, Icon24log, 0x40 ) ;
//			lcd_img( 165, (STATUS_VERTICAL/2), IconX12Logging, 0, 0, LCD_BLACK, LCD_STATUS_GREY ) ;
		}
	}
extern uint32_t IdlePercent ;
  lcd_outdezNAtt( 4*FW, (STATUS_VERTICAL/2)+4, IdlePercent ,PREC2, 4, LCD_BLACK, LCD_STATUS_GREY ) ;
	if ( scriptPercent )
	{
extern uint32_t BasicExecTime ;
	  lcd_outdezNAtt( 8*FW, (STATUS_VERTICAL/2)+4, BasicExecTime ,PREC1, 4, LCD_BLACK, LCD_STATUS_GREY ) ;
	}
}
#endif


