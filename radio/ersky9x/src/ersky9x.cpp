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
#define __ERSKY9X_CPP__

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>


#ifdef PCBSKY
#include "AT91SAM3S4.h"
#ifndef SIMU
#include "core_cm3.h"
#endif
#endif


#include "ersky9x.h"
#include "myeeprom.h"
#include "audio.h"
#include "sound.h"
#include "lcd.h"
#include "drivers.h"

#ifdef PCBSKY
#include "file.h"
#endif

#include "menus.h"
#include "timers.h"
#include "logicio.h"
#include "pulses.h"
#include "stringidx.h"

#ifdef FRSKY
#include "frsky.h"
#endif

#ifdef PCBX9D
#include "analog.h"
#include "diskio.h"
#include "X9D/Eeprom_rlc.h"
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/stm32f2xx_rcc.h"
#include "X9D/hal.h"
#include "X9D/i2c_ee.h"

#include "X9D/usb_dcd_int.h"
#include "X9D/usb_bsp.h"
#include "X9D/usbd_conf.h"

extern "C" uint8_t USBD_HID_SendReport(USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len) ;

#endif

#ifdef PCB9XT
#include "file.h"
#include "analog.h"
#include "diskio.h"
#include "mega64.h"
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/stm32f2xx_rcc.h"
#include "X9D/i2c_9xt.h"
#include "X9D/hal.h"

#include "X9D/usb_dcd_int.h"
#include "X9D/usb_bsp.h"
#include "X9D/usbd_conf.h"

extern "C" uint8_t USBD_HID_SendReport(USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len) ;

#endif // PCB9XT

#include "sbus.h"

#include "ff.h"
#include "maintenance.h"


#ifndef SIMU
#include "CoOS.h"
#endif

//#define PCB_TEST_9XT	1

#ifndef SIMU
#define MAIN_STACK_SIZE		500
#ifdef BLUETOOTH
#define BT_STACK_SIZE			100
#endif
#define LOG_STACK_SIZE		350
#define DEBUG_STACK_SIZE	300
#define VOICE_STACK_SIZE	130+200

OS_TID MainTask;
OS_STK main_stk[MAIN_STACK_SIZE] ;

#ifdef BLUETOOTH
#define BT_TYPE_HC06		0
#define BT_TYPE_HC05		1
uint8_t BtType ;
OS_TID BtTask;
OS_STK Bt_stk[BT_STACK_SIZE] ;
#endif
OS_TID LogTask;
OS_STK Log_stk[LOG_STACK_SIZE] ;
OS_TID VoiceTask;
OS_STK voice_stk[VOICE_STACK_SIZE] ;

#ifdef	DEBUG
OS_TID DebugTask;
OS_STK debug_stk[DEBUG_STACK_SIZE] ;
#endif

#ifdef SERIAL_HOST
void host( void* pdata ) ;
#define HOST_STACK_SIZE	300
OS_TID HostTask ;
OS_STK Host_stk[HOST_STACK_SIZE] ;
uint8_t Host10ms ;

#endif

#endif


const char * const *Language = English ;

const uint8_t splashdata[] = { 'S','P','S',0,
#include "s9xsplash.lbm"
	'S','P','E',0};
//const uchar *s9xsplash = splashdata+4;

#include "debug.h"

//#ifdef PCBSKY
//uint8_t BtReceived[16] ;
//uint32_t BtRxIndex = 0 ;
//#endif

t_time Time ;

uint8_t unexpectedShutdown = 0;
uint8_t SdMounted = 0;
uint8_t CurrentTrainerSource ;
uint8_t HardwareMenuEnabled = 0 ;

#define SW_STACK_SIZE	6
uint8_t Last_switch[NUM_SKYCSW] ;
int8_t SwitchStack[SW_STACK_SIZE] ;

uint8_t NumExtraPots ;

uint16_t AnalogData[ANALOG_DATA_SIZE] ;


//#define TRUE	1
//#define FALSE	0

//#define bool uint32_t

// Soft power operation (SKY board)
// When the main power switch is turned on, the CPU starts, finds PC17 is high
// so RF Power is on, enables the pullup on PA8 (50K to 175K), and thus turns
// the soft power switch ON. Even with 175K pullup, assuming a hFE of the transistor
// of 100, the transistor collector current should be at least 1.46 mA. This is
// enough to pull the gate of the soft switch low. When you turn the power switch
// off, RF power goes off, PC17 goes low (could have an internal pull down resistor),
// so the CPU knows the power switch is off, CPU tidies up,
// disables the pullup on PA8 and turns itself off.

// If you plug the trainer cable in, with the power switch off, the soft power
// switch turns on, the CPU finds PC17 is low, and PA8 is high so power must be
// because of the trainer cable. As long as the voltage from the trainer power
// is at least 6 volts, PA8 will be read as a 1 (>2.31 volts (0.7*VDDIO)). The
// CPU turns the pullup resistor on PA8 on, thus holding the soft power ON.
// When you unplug the trainer cable, the voltage at PA8 pin will drop. Even
// with a 50K pullup resistor, and 0.7V across the transistor base-emitter,
// the voltage at PA8 will be less than 0.87V. A logic 0 is a voltage less
// than 0.99V (0.3*VDDIO). So the CPU will see a logic 0 on PA8 and know it
// is time to tidy up, then turn the soft power off.


/*=========================================================================*/
/*  DEFINE: All Structures and Common Constants                            */
/*=========================================================================*/

/*=========================================================================*/
/*  DEFINE: Prototypes                                                     */
/*=========================================================================*/


extern uint16_t g_timeMain;
extern uint16_t g_timeRfsh ;
extern uint16_t g_timeMixer ;

volatile int32_t Rotary_position ;
volatile int32_t Rotary_count ;
int32_t LastRotaryValue ;
int32_t Rotary_diff ;
uint8_t Vs_state[NUM_SKYCHNOUT+NUM_VOICE] ;
uint8_t Nvs_state[NUM_VOICE_ALARMS+NUM_EXTRA_VOICE_ALARMS] ;
int16_t Nvs_timer[NUM_VOICE_ALARMS+NUM_EXTRA_VOICE_ALARMS] ;
uint8_t CurrentVolume ;
uint8_t HoldVolume ;
int8_t RotaryControl ;
uint8_t ppmInValid = 0 ;
uint8_t Activated = 0 ;
uint8_t Tevent ;
uint16_t SbusTimer = 0 ;

#ifdef BLUETOOTH
void tmrBt_Handle( void ) ;
void bt_task(void* pdata) ;
#endif
void log_task(void* pdata) ;
void main_loop( void* pdata ) ;
void mainSequence( uint32_t no_menu ) ;
void doSplash( void ) ;
void perMain( uint32_t no_menu ) ;
#ifdef PCBSKY
void UART_Configure( uint32_t baudrate, uint32_t masterClock) ;
#endif
void txmit( uint8_t c ) ;
void uputs( char *string ) ;
uint16_t rxuart( void ) ;

#ifdef PCB_TEST_9XT
void test_loop( void* pdata ) ;
#endif

#ifdef PCBSKY
extern "C" void TC2_IRQHandler( void ) ;
//#ifdef SIMU
//#define sam_boot()
//#else
//extern "C" void sam_boot( void ) ;
//#endif
#ifdef	DEBUG
void handle_serial( void* pdata ) ;
#endif
#endif

#ifdef PCB9XT
#ifdef	DEBUG
void handle_serial( void* pdata ) ;
#endif
#endif


#if defined(PCBSKY) || defined(PCB9XT)
uint16_t SixPositionTable[5] ;
#endif

#ifdef PCBSKY
uint16_t anaIn( uint8_t chan ) ;
#endif
void getADC_single( void ) ;
void getADC_osmp( void ) ;
void getADC_filt( void ) ;
#ifdef PCBSKY
void read_adc( void ) ;
void init_adc( void ) ;
#endif

void checkTHR( void ) ;
void checkSwitches( void ) ;
void check_backlight( void ) ;
#if defined(PCBSKY) || defined(PCB9XT)
void checkQuickSelect( void ) ;
//void actionUsb( void ) ;
#endif

static uint8_t checkTrim(uint8_t event) ;

void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att) ;
void putsChn(uint8_t x,uint8_t y,uint8_t idx1,uint8_t att) ;
void putsDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att) ;//, bool nc) ;
const char *get_switches_string( void ) ;
bool getSwitch(int8_t swtch, bool nc, uint8_t level) ;
void init_soft_power( void ) ;
uint32_t check_soft_power( void ) ;
void soft_power_off( void ) ;
int8_t getGvarSourceValue( uint8_t src ) ;
static void	processAdjusters( void ) ;

#ifdef PCBSKY
#if defined(SIMU)
  #define init_rotary_encoder()
//  #define stop_rotary_encoder()
#else
  static void init_rotary_encoder( void ) ;
//  static void stop_rotary_encoder( void ) ;
#endif
#endif

#ifdef REV9E
  static void init_rotary_encoder( void ) ;
#endif // REV9E

/*=========================================================================*/
/*  DEFINE: Definition of all local Data                                   */
/*=========================================================================*/

uint32_t Master_frequency ;
//uint16_t Adc_data[32] ;
//volatile uint32_t Timer2_count ;		// Modified in interrupt routine
volatile uint32_t Tenms ;						// Modified in interrupt routine
volatile uint8_t tick10ms = 0 ;
volatile uint8_t tick5ms = 0 ;
uint16_t g_LightOffCounter ;
uint8_t  InactivityMonitor = 0 ;
volatile uint32_t PowerOnTime ;						// Modified in interrupt routine

uint16_t S_anaFilt[ANALOG_DATA_SIZE] ;				// Analog inputs after filtering
#ifdef PCBSKY
uint16_t Current_analogue ;
uint16_t Current_current ;
uint16_t Current_adjust = 32768 + 16384 ;
uint16_t Current_max ;
uint32_t Current_accumulator ;
uint32_t Current_used ;

uint16_t MAh_used ;
uint16_t Run_time ;
#endif

uint8_t sysFlags = 0 ;
uint8_t SystemOptions ;


//uint32_t Per10ms_action ;
//uint32_t Permenu_action ;

int16_t g_ppmIns[16];
uint8_t ppmInState = 0; //0=unsync 1..8= wait for value i-1
uint8_t Main_running ;
#ifdef PCBSKY
uint8_t CoProcAlerted ;
#endif
int main( void ) ;

EEGeneral  g_eeGeneral;
#if defined(PCBSKY) || defined(PCB9XT)
ModelData  g_oldmodel;
#endif
SKYModelData  g_model;

//const uint8_t chout_ar[] = { //First number is 0..23 -> template setup,  Second is relevant channel out
//                                1,2,3,4 , 1,2,4,3 , 1,3,2,4 , 1,3,4,2 , 1,4,2,3 , 1,4,3,2,
//                                2,1,3,4 , 2,1,4,3 , 2,3,1,4 , 2,3,4,1 , 2,4,1,3 , 2,4,3,1,
//                                3,1,2,4 , 3,1,4,2 , 3,2,1,4 , 3,2,4,1 , 3,4,1,2 , 3,4,2,1,
//                                4,1,2,3 , 4,1,3,2 , 4,2,1,3 , 4,2,3,1 , 4,3,1,2 , 4,3,2,1    };
const uint8_t bchout_ar[] = {
															0x1B, 0x1E, 0x27, 0x2D, 0x36, 0x39,
															0x4B, 0x4E, 0x63, 0x6C, 0x72, 0x78,
                              0x87, 0x8D, 0x93, 0x9C, 0xB1, 0xB4,
                              0xC6, 0xC9, 0xD2, 0xD8, 0xE1, 0xE4		} ;


//new audio object
audioQueue  audio;

#define	ALERT_TYPE	0
#define MESS_TYPE		1

const char *AlertMessage ;
uint8_t AlertType ;

uint8_t AlarmTimer = 100 ;		// Units of 10 mS
uint8_t AlarmCheckFlag = 0 ;
//uint8_t CsCheckFlag = 0 ;
uint8_t VoiceTimer = 10 ;		// Units of 10 mS
uint8_t VoiceCheckFlag = 0 ;
//#ifdef TELEMETRY_LOST
//uint8_t TelemetryStatus ;
//#endif
uint8_t DsmCheckTimer = 50 ;		// Units of 10 mS
uint8_t DsmCheckFlag = 0 ;
//uint8_t LogTimer = 0 ;
//uint8_t FileDisable = 0 ;
int16_t  CsTimer[NUM_SKYCSW] ;

//const char *Str_Switches = PSTR(SWITCHES_STR) ;

const char *Str_OFF = PSTR(STR_OFF) ;
const char *Str_ON = PSTR(STR_ON) ;



//#ifdef FIX_MODE
//const char modi12x3[]= "\004RUD ELE THR AIL " ;
// now use PSTR(STR_STICK_NAMES)
const char stickScramble[]= {
    0, 1, 2, 3,
    0, 2, 1, 3,
    3, 1, 2, 0,
    3, 2, 1, 0 };

//const char modeFix[] =
//{
//    1, 2, 3, 4,		// mode 1
//    1, 3, 2, 4,		// mode 2
//    4, 2, 3, 1,		// mode 3
//    4, 3, 2, 1		// mode 4
//} ;

//#else

//const char modi12x3[]=
//  "RUD ELE THR AIL " ;
////  "RUD THR ELE AIL "
////  "AIL ELE THR RUD "
////  "AIL THR ELE RUD ";
//// Now indexed using modn12x3

//const char modn12x3[]= {
//    1, 2, 3, 4,
//    1, 3, 2, 4,
//    4, 2, 3, 1,
//    4, 3, 2, 1 };
//#endif

//#ifdef FIX_MODE
uint8_t modeFixValue( uint8_t value )
{
	return stickScramble[g_eeGeneral.stickMode*4+value]+1 ;
}
//#endif

MenuFuncP g_menuStack[5];

uint8_t  g_menuStackPtr = 0;
uint8_t  EnterMenu = 0 ;


// Temporary to allow compile
uint8_t g_vbat100mV ;//= 98 ;
//int16_t calibratedStick[7] ;
//int16_t g_chans512[NUM_SKYCHNOUT] ;
uint8_t heartbeat ;
uint8_t heartbeat_running ;

#ifdef PCBSKY
uint16_t ResetReason ;
uint32_t ChipId ;
#endif
#if defined(PCBX9D) || defined(PCB9XT)
uint32_t ResetReason ;
uint32_t ChipId ;
#endif

//uint8_t Timer2_running = 0 ;
//uint8_t Timer2_pre = 0 ;
//uint16_t Timer2 = 0 ;

//#ifdef REV9E
//#define X9D_OFFSET		42
//uint16_t ProgressCounter ;
//extern uint8_t PowerState ;
//void progress( uint32_t x )
//{
//  wdt_reset();
//	g_LightOffCounter = 2000 ;
//	lcd_clear() ;
//	lcd_outhex4( 0, 0, x ) ;
//	lcd_outhex4( 0, FH, ++ProgressCounter ) ;
//	lcd_outhex4( 0, 2*FH, CurrentVolume ) ;
//	lcd_outhex4( 212-X9D_OFFSET, 0, x ) ;
//	lcd_outhex4( 212-X9D_OFFSET, FH, ++ProgressCounter ) ;
//	lcd_outhex4( 212-X9D_OFFSET, 2*FH, PowerState ) ;
//	lcd_outhex4( 212-X9D_OFFSET, 3*FH, check_soft_power() ) ;
//extern uint16_t SuCount ;
//	lcd_outhex4( 212-X9D_OFFSET, 4*FH, SuCount ) ;

//	lcd_outhex4( 50, 1*FH, GPIOD->MODER ) ;
//	lcd_outhex4( 50, 2*FH, GPIOD->IDR ) ;
//	lcd_outhex4( 50, 3*FH, GPIOD->ODR ) ;

//	lcd_outhex4( 90, 1*FH, DAC->CR ) ;
//	lcd_outhex4( 90, 2*FH, DMA1_Stream5->M0AR ) ;
//	lcd_outhex4( 90, 3*FH, TIM6->CNT ) ;
//	lcd_outhex4( 90, 4*FH, TIM6->ARR ) ;

//	lcd_outhex4( 212-X9D_OFFSET, 6*FH, GPIOE->MODER ) ;
//	lcd_outhex4( 212-X9D_OFFSET, 7*FH, GPIOE->AFR[0]>>16 ) ;

//	uint32_t i ;
//	for ( i = 0 ; i < 200 ; i += 1 )
//	{
//	  wdt_reset();
//		lcd_outhex4( 0, 7*FH, i ) ;
//		refreshDisplay() ;
//	}
//}
//#endif

void usbJoystickUpdate(void) ;

#if defined(PCB9XT) || defined(PCBX9D)
bool usbPlugged(void)
{
  return GPIO_ReadInputDataBit(GPIOA, PIN_FS_VBUS);
}
#endif


void handleUsbConnection()
{
#if (defined(PCBX9D) || defined(PCB9XT)) && !defined(SIMU)
  static bool usbStarted = false;

  if (!usbStarted && usbPlugged())
	{
    usbStarted = true;

    /*
      We used to initialize USB peripheral and driver here.
      According to my tests this is way too late. The USB peripheral
      therefore does not have enough information to start responding to 
      USB host request, which causes very slow USB device recognition, 
      multiple USB device resets, etc...

      If we want to change the USB profile, the procedure is simple:
        * USB cable must be disconnected
        * call usbDeInit();
        * call usbUnit(); which initializes USB with the new profile. 
          Obviously the usbInit() should be modified to have a runtime
          selection of the USB profile.
    */

//#if defined(USB_MASS_STORAGE)
//    opentxClose();
//    usbPluggedIn();
//#endif
  }
  if (usbStarted && !usbPlugged())
	{
    usbStarted = false;
  }
  
//#if defined(USB_JOYSTICK)
  if (usbStarted )
	{
    usbJoystickUpdate();
  }
//#endif
  
#endif //#if defined(PCB9XT) && !defined(SIMU)
}



#ifdef PCBX9D
//const char Switches_Str[] = "SA0SA1SA2SB0SB1SB2SC0SC1SC2SD0SD1SD2SE0SE1SE2SF0SF2SG0SG1SG2SH0SH2" ;
// Needs to be in pulses_driver.h
extern void init_no_pulses(uint32_t port) ;
extern void init_pxx(uint32_t port) ;

void init_i2s1( void ) ;
#endif

#if defined(PCBX9D) || defined(PCB9XT)
extern void initWatchdog( void ) ;
#endif

uint8_t throttleReversed()
{
	return g_model.throttleReversed ^	g_eeGeneral.throttleReversed ;
}

void setLanguage()
{
	switch ( g_eeGeneral.language )
	{
		case 1 :
			Language = French ;
			ExtraFont = font_fr_extra ;
			ExtraBigFont = font_fr_big_extra ;
		break ;
		case 2 :
			Language = German ;
			ExtraFont = font_de_extra ;
			ExtraBigFont = font_de_big_extra ;
		break ;
		case 3 :
			Language = Norwegian ;
			ExtraFont = font_se_extra ;
			ExtraBigFont = font_se_big_extra ;
		break ;
		case 4 :
			Language = Swedish ;
			ExtraFont = font_se_extra ;
			ExtraBigFont = font_se_big_extra ;
		break ;
		case 5 :
			Language = Italian ;
			ExtraFont = font_it_extra ;
			ExtraBigFont = NULL ;
		break ;
		case 6 :
			Language = Polish ;
			ExtraFont = font_pl_extra ;
			ExtraBigFont = NULL ;
		break ;
		case 7 :
			Language = Vietnamese ;
			ExtraFont = NULL ;
			ExtraBigFont = NULL ;
		break ;
		default :
			Language = English ;
			ExtraFont = NULL ;
			ExtraBigFont = NULL ;
		break ;
	}
}

const uint8_t csTypeTable[] =
{ CS_VOFS, CS_VOFS, CS_VOFS, CS_VOFS, CS_VBOOL, CS_VBOOL, CS_VBOOL,
 CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VBOOL, CS_VBOOL, CS_TIMER, CS_TIMER, CS_TMONO, CS_TMONO, CS_VOFS
} ;

uint8_t CS_STATE( uint8_t x)
{
	return csTypeTable[x-1] ;
}


static void checkAlarm() // added by Gohst
{
    if(g_eeGeneral.disableAlarmWarning) return;
    if(!g_eeGeneral.beeperVal) alert(PSTR(STR_ALRMS_OFF));
}

//static void checkCalibrate()
//{
//  if(g_eeGeneral.calibrateWarning) return ;
//  alert(XPSTR("Not Calibrated"));
//}

static void checkWarnings()
{
    if(sysFlags & sysFLAG_OLD_EEPROM)
    {
        alert(PSTR(STR_OLD_EEPROM)); //will update on next save
        sysFlags &= ~(sysFLAG_OLD_EEPROM); //clear flag
    }
}

inline uint8_t keyDown()
{
    return ~read_keys() & 0x7E ;
}

void clearKeyEvents()
{
    while(keyDown())
		{
			  // loop until all keys are up
#ifdef PCBSKY
//			if ( PIOC->PIO_PDSR & 0x02000000 )
//			{
				// Detected USB
//				break ;
//			}
#endif
//			if ( heartbeat_running )
//			{
//  			if(heartbeat == 0x3)
//  			{
//  			  heartbeat = 0;
//  			  wdt_reset();
//  			}
//			}
//			else
//			{
				wdt_reset() ;
//			}
		}	
    putEvent(0);
}

int32_t isAgvar(uint8_t value)
{
	if ( value >= 70 )
	{
		if ( value <= 76 )
		{
			return 1 ;
		}
	}
	return 0 ;
}

#ifdef BLUETOOTH
#define BT_115200		0
#define BT_9600			1
#define BT_19200		2
#define BT_57600		3
#define BT_38400		4

#define NUM_BT_BAUDRATES 5

uint8_t Bt_baudrate ;

void setBtBaudrate( uint32_t index )
{
	Bt_baudrate = index ;
	uint32_t brate ;
	if ( index == 1 )
	{
		brate = 9600 ;
	}
	else if ( index == 2 )
	{
		brate = 19200 ;
	}
	else if ( index == 3 )
	{
		brate = 57600 ;
	}
	else if ( index == 4 )
	{
		brate = 38400 ;
	}
	else
	{
		brate = 115200 ;
	}

//	if ( g_eeGeneral.bt_correction )
//	{
//		uint32_t value = g_eeGeneral.bt_correction ;
//		value += 100 ;
//		if ( value > 107 )
//		{
//			value -= 16 ;
//		}
//		brate = brate * value / 100 ;
//	}
#ifdef PCB9XT	
	if ( g_eeGeneral.btComPort == 1 )
	{
		UART4SetBaudrate ( brate ) ;
	}
	else if ( g_eeGeneral.btComPort == 2 )
	{
		Com3SetBaudrate ( brate ) ;
	}
#else
	UART3_Configure( brate, Master_frequency ) ;		// Testing
#endif
}
#endif

extern void maintenanceBackground( void ) ;

void update_mode(void* pdata)
{
	uint32_t displayTimer = 0 ;
	g_menuStack[0] = menuUpdate ;
	g_menuStack[1] = menuUp1 ;	// this is so the first instance of [MENU LONG] doesn't freak out!
	MaintenanceRunning = 1 ;
#if defined(PCBX9D) || defined(PCB9XT)
	x9dSPortInit( 57600, SPORT_MODE_HARDWARE, SPORT_POLARITY_NORMAL, 0 ) ; // Kick off at 57600 baud
#endif
#ifdef PCBSKY
	UART2_Configure( 57600, Master_frequency ) ;
	startPdcUsartReceive() ;
#endif
#ifdef PCB9XT
	BlSetAllColours( 0, 30, 60 ) ;
#endif

  while (1)
	{
		if ( g_menuStack[0] == menuUpdate )
		{
			if ( ( check_soft_power() == POWER_OFF )/* || ( goto_usb ) */ )		// power now off
			{
				soft_power_off() ;		// Only turn power off if necessary
			}
		}

	  static uint16_t lastTMR;
		uint16_t t10ms ;
		t10ms = get_tmr10ms() ;
  	tick10ms = ((uint16_t)(t10ms - lastTMR)) != 0 ;
	  lastTMR = t10ms ;

		maintenanceBackground() ;

		if(!tick10ms) continue ; //make sure the rest happen only every 10ms.

	  uint8_t evt=getEvent();

//	check_backlight() ;

    lcd_clear() ;
		if ( EnterMenu )
		{
			evt = EnterMenu ;
			EnterMenu = 0 ;
		}
		g_menuStack[g_menuStackPtr](evt);
		// Only update display every 40mS, improves SPort update throughput
		if ( ++displayTimer >= 4 )
		{
			displayTimer = 0 ;
    	refreshDisplay() ;
		}
		
		wdt_reset();

		if ( Tenms )
		{
			Tenms = 0 ;
		}
#ifndef SIMU
		sdPoll10mS() ;
#endif

#ifndef SIMU
		CoTickDelay(1) ;					// 2mS for now
#endif
	}
}


#ifdef PCBX9D

#define EXTERNAL_RF_ON()      GPIO_SetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#define EXTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR)

void checkTrainerSource()
{
	if ( CurrentTrainerSource	!= g_eeGeneral.trainerSource )
	{
		switch ( CurrentTrainerSource )
		{
			case 0 :
				stop_trainer_capture() ;
			break ;
			case 1 :
				stop_USART6_Sbus() ;
				EXTERNAL_RF_OFF() ;
			break ;
			case 2 :
				stop_cppm_on_heartbeat_capture() ;				
				EXTERNAL_RF_OFF() ;
			break ;
			case 3 :
				stop_trainer_ppm() ;
			break ;
		}
		CurrentTrainerSource = g_eeGeneral.trainerSource ;
		switch ( CurrentTrainerSource )
		{
			case 0 :
				init_trainer_capture() ;
				EXTERNAL_RF_OFF() ;
			break ;
			case 1 :
				USART6_Sbus_configure() ;
				EXTERNAL_RF_ON() ;
			break ;
			case 2 :
				init_cppm_on_heartbeat_capture()  ;
				EXTERNAL_RF_ON() ;
			break ;
			case 3 :	// Slave so output
				init_trainer_ppm() ;
				EXTERNAL_RF_OFF() ;
			break ;
		}
	}
}
#endif


void com2Configure()
{
	if ( g_model.com2Function == COM2_FUNC_SBUSTRAIN )
	{
		UART_Sbus_configure( Master_frequency ) ;
	}
	else if ( g_model.com2Function == COM2_FUNC_SBUS57600 )
	{
		UART_Sbus57600_configure( Master_frequency ) ;
	}
#ifdef PCBSKY
	else if( g_model.com2Function == COM2_FUNC_BTDIRECT )
	{
		UART_Configure( g_model.com2Baudrate-1, Master_frequency ) ;
	}
#endif	// PCBSKY
#ifdef PCBSKY
	else if ( g_model.com2Function == COM2_FUNC_FMS )
	{
		UART_Configure( 19200, Master_frequency ) ;
	}
#endif
#ifdef PCBX9D
	else if( g_model.com2Function == COM2_FUNC_CPPMTRAIN )
	{
		init_serial_trainer_capture() ;
	}
#endif
	else
	{
#ifdef PCBSKY
		UART_Configure( CONSOLE_BAUDRATE, Master_frequency ) ;
#endif
#ifdef PCBX9D
		x9dConsoleInit() ;
#endif
#ifdef PCB9XT
		console9xtInit() ;
#endif
	}	 
}

#ifdef PCBSKY
 #ifndef REVX
static void checkAr9x()
{
	uint32_t x = ChipId & 0x00000F00 ;
	if ( x <= 0x00000900 )
	{
		return ;
	}
	g_eeGeneral.ar9xBoard = 1 ;
	g_eeGeneral.softwareVolume = 1 ;
}
 #endif
#endif


#if defined(PCB9XT) || defined(PCBX9D)
extern "C" void usbInit(void) ;
#endif

#ifdef PCB9XT
#define SETBL_DELAY		2
static void delay_setbl( uint8_t r, uint8_t g, uint8_t b )
{
	return  ;
	uint16_t timer = 0 ;
	for ( timer = 0 ; timer < SETBL_DELAY ;  )
	{
		if ( Tenms )
		{
			Tenms = 0 ;
			timer += 1 ;
			wdt_reset() ;
		}
	}
	BlSetAllColours( r, g, b ) ;
	for ( timer = 0 ; timer < SETBL_DELAY ;  )
	{
		if ( Tenms )
		{
			Tenms = 0 ;
			timer += 1 ;
			wdt_reset() ;
		}
	}
}
#endif


int main( void )
{
#ifdef PCBSKY
	register Pio *pioptr ;
#endif

#ifdef PCBSKY
	ResetReason = RSTC->RSTC_SR ;
	ChipId = CHIPID->CHIPID_CIDR ;
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	ChipId = *((uint16_t *)0x1FFF7A22) ;
	ResetReason = RCC->CSR ;
  RCC->CSR |= RCC_CSR_RMVF ;
#endif

//#endif

#ifdef PCBSKY
  PMC->PMC_PCER0 = (1<<ID_PIOC)|(1<<ID_PIOB)|(1<<ID_PIOA)|(1<<ID_UART0) ;				// Enable clocks to PIOB and PIOA and PIOC and UART0
	
	MATRIX->CCFG_SYSIO |= 0x000010F0L ;		// Disable syspins, enable B4,5,6,7,12

// Configure the ERASE pin as an output, low for Bluetooth use
	pioptr = PIOB ;
	pioptr->PIO_CODR = PIO_PB12 ;		// Set bit B12 LOW
	pioptr->PIO_PER = PIO_PB12 ;		// Enable bit B12 (ERASE)
	pioptr->PIO_OER = PIO_PB12 ;		// Set bit B12 as output

	pioptr = PIOA ;

 #ifdef REVB	
	init_soft_power() ;
 #else	
	// On REVB, PA21 is used as AD8, and measures current consumption.
	pioptr->PIO_PER = PIO_PA21 ;		// Enable bit A21 (EXT3)
	pioptr->PIO_OER = PIO_PA21 ;		// Set bit A21 as output
	pioptr->PIO_SODR = PIO_PA21 ;	// Set bit A21 ON
 #endif
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	init_soft_power() ;
#endif


#ifdef PCB9XT
// Configure pin PA5 as an output, low for Bluetooth use
	configure_pins( GPIO_Pin_5, PIN_PORTA | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
	GPIOA->BSRRH = GPIO_Pin_5 ;		// Set low
#endif

#if defined(PCBX9D) || defined(PCB9XT)
	initWatchdog() ;
#endif

#ifdef PCBSKY
	pioptr = PIOC ;
	pioptr->PIO_PER = PIO_PC25 ;		// Enable bit C25 (USB-detect)

	if ( ( pioptr->PIO_PDSR & 0x02000000 ) == 0 )
	{
		// USB not the power source
		WDT->WDT_MR = 0x3FFF207F ;				// Enable watchdog 0.5 Secs
	}

#ifdef REVB	
#else	
	// Configure RF_power (PC17) and PPM-jack-in (PC19), neither need pullups
	pioptr->PIO_PER = 0x000A0000L ;		// Enable bit C19, C17
	pioptr->PIO_ODR = 0x000A0000L ;		// Set bits C19 and C17 as input
#endif
#endif

#ifdef PCBSKY
	config_free_pins() ;
#endif

	init_keys() ;
	
#ifdef PCBSKY
	initExtraInput() ;		// PB14/DAC1 as input
#endif

	setup_switches() ;

#ifdef PCBSKY
  // Enable PCK2 on PB3, This is for testing of Timer 2 working
	// It will be used as serial data to the Bluetooth module
	pioptr->PIO_ABCDSR[0] |=  PIO_PB3 ;	// Peripheral B
  pioptr->PIO_ABCDSR[1] &= ~PIO_PB3 ;	// Peripheral B
  pioptr->PIO_PDR = PIO_PB3 ;					// Assign to peripheral
	PMC->PMC_SCER |= 0x0400 ;								// PCK2 enabled
	PMC->PMC_PCK[2] = 2 ;										// PCK2 is PLLA

	UART_Configure( CONSOLE_BAUDRATE, Master_frequency ) ;
#endif

#ifdef PCBX9D
	x9dConsoleInit() ;
#endif
#ifdef PCB9XT
	console9xtInit() ;
//	uputs( (char *)"Hello\r\n" ) ;
#endif

	init5msTimer() ;
	
	init_hw_timer() ;
	init_adc() ;

#ifdef PCBX9D
	I2C_EE_Init() ;
	setVolume( 0 ) ;
#endif

#ifdef PCBSKY
	init_pwm() ;
#ifndef SIMU
	init_SDcard() ;
#endif
#endif

#ifdef PCBX9D
	// SD card detect pin
	configure_pins( GPIO_Pin_CP, PIN_PORTD | PIN_INPUT | PIN_PULLUP ) ;
#endif
	
#ifdef PCB9XT
	// SD card detect pin
	configure_pins( GPIO_Pin_CP, PIN_PORTC | PIN_INPUT | PIN_PULLUP ) ;
#endif
	
#if defined(PCBX9D) || defined(PCB9XT)
//  sdInit() ;
	disk_initialize( 0 ) ;
	sdInit() ;
#endif


	__enable_irq() ;

#ifdef PCB9XT
	delay_setbl( 100, 0, 0 ) ;
#endif

	lcd_init() ;
#ifdef PCBSKY
	lcdSetOrientation() ;
#endif

#ifdef PCBX9D
  lcd_clear() ;
	refreshDisplay() ;
#endif

	g_menuStack[0] =  menuProc0 ;

//progress( 9 ) ;

#ifdef PCB9XT
	initM64() ;
	init_software_remote() ;
	if ( ( ResetReason & RCC_CSR_WDGRSTF ) != RCC_CSR_WDGRSTF ) // && !unexpectedShutdown )	// Not watchdog
	{
		uint16_t timer = 0 ;
		configure_pins( 0x0004, PIN_PORTA | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25  ) ;
		GPIOA->BSRRH = 0x0004 ;			// Pin low
		BlSetColour( 100, BL_RED ) ;	

		while ( timer < 100 )
		{
			checkM64() ;
			if ( Tenms )
			{
				m64_10mS() ;				
				Tenms = 0 ;
				timer += 1 ;
				wdt_reset() ;
			}
			if ( m64ReceiveStatus() & 1 )
			{
				break ;
			}
			if ( ( timer == 25 ) || ( timer == 75 ) )
			{
				GPIOA->BSRRL = 0x0004 ;			// Pin High
				BlSetColour( 0, BL_RED ) ;	
			}
			if ( timer == 50 )
			{
				GPIOA->BSRRH = 0x0004 ;			// Pin low
				BlSetColour( 100, BL_RED ) ;	
			}
		}
		if ( m64ReceiveStatus() == 0 )
		{
			// M64 not sending
//extern void checkIspAccess() ;
//			checkIspAccess() ;
			uint32_t delay ;
uint32_t updateSlave() ;
			BlSetAllColours( 0, 0, 100 ) ;
#ifndef PCB_TEST_9XT
			if ( updateSlave() )
			{
				// Failed
				BlSetAllColours( 100, 0, 0 ) ;
				delay = 250 ;
			}
			else
			{
				// OK
				BlSetAllColours( 0, 100, 0 ) ;
				delay = 100 ;
			}
			while (delay)
			{
				if ( Tenms )
				{
					Tenms = 0 ;
					delay -= 1 ;
					wdt_reset() ;
				}
			}
#endif			
			initM64() ;
		}
		GPIOA->BSRRL = 0x0004 ;			// Pin High
		BlSetAllColours( 0, 0, 0 ) ;
	}
	else
	{
		delay_setbl( 0, 0, 100 ) ;
	}
	
	lcd_clear() ;
	refreshDisplay() ;
#endif

//#ifdef PCB9XT

////#define BACKLIGHT_TEST	1
 
// #ifdef BACKLIGHT_TEST
//	{
//		BlSetColour( 0, 3 ) ;	

//		for(;;)
//		{
//			uint16_t timer = 0 ;
////			for ( timer = 0 ; timer < 2 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;}}
//			BlSetColour( 100, BL_RED ) ;	
//			for ( timer = 0 ; timer < 200 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;}}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}
//			BlSetColour( 0, BL_RED ) ;	
////			for ( timer = 0 ; timer < 2 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;}}
//			BlSetColour( 100, BL_GREEN ) ;	
//			for ( timer = 0 ; timer < 200 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;}}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}
//			BlSetColour( 0, BL_GREEN ) ;	
////			for ( timer = 0 ; timer < 2 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;}}
//			BlSetColour( 100, BL_BLUE ) ;	
//			for ( timer = 0 ; timer < 200 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;}}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}

//			for ( timer = 100 ; timer > 0 ;  )
//			{
//				if ( Tenms )
//				{
//					Tenms = 0 ;
//					timer -= 1 ;
//					BlSetColour( timer, BL_BLUE ) ;	
//				}
//			}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}
//			for ( timer = 0 ; timer < 100 ;  )
//			{
//				if ( Tenms )
//				{
//					Tenms = 0 ;
//					timer += 1 ;
//					BlSetColour( timer, BL_GREEN ) ;	
//				}
//			}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}
//			for ( timer = 100 ; timer > 0 ;  )
//			{
//				if ( Tenms )
//				{
//					Tenms = 0 ;
//					timer -= 1 ;
//					BlSetColour( timer, BL_GREEN ) ;	
//				}
//			}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}
//			for ( timer = 0 ; timer < 100 ;  )
//			{
//				if ( Tenms )
//				{
//					Tenms = 0 ;
//					timer += 1 ;
//					BlSetColour( timer, BL_RED ) ;	
//				}
//			}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}
//			for ( timer = 100 ; timer > 0 ;  )
//			{
//				if ( Tenms )
//				{
//					Tenms = 0 ;
//					timer -= 1 ;
//					BlSetColour( timer, BL_RED ) ;	
//				}
//			}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}
//		}
//	}	
// #endif
//#endif


#ifdef PCBX9D
//	I2C_EE_Init() ;
//progress( 8 ) ;
//	setVolume( 12 ) ;
//progress( 7 ) ;
	init_trims() ;
	initHaptic() ;
	start_2Mhz_timer() ;
	setVolume( 0 ) ;
	start_sound() ;
//progress( 6 ) ;
#endif

#ifdef PCB9XT
	init_trims() ;
	start_2Mhz_timer() ;
	start_sound() ;
#endif


#if defined(PCBSKY) || defined(PCB9XT)
	init_spi() ;
	init_eeprom() ;	

//progress( 0 ) ;

//	if ( ( ( ResetReason & RSTC_SR_RSTTYP ) != (2 << 8) ) && !unexpectedShutdown )	// Not watchdog
//	{
//		pioptr = PIOC ;
//		if ( pioptr->PIO_PDSR & 0x02000000 )
//		{
//			g_eeGeneral.optrexDisplay = 1 ;
//			lcd_clear() ;
//			refreshDisplay() ;
//			g_eeGeneral.optrexDisplay = 0 ;
//			actionUsb() ;
//		}
//	}
#endif


#ifdef PCB_TEST_9XT
	initWatchdog() ;
	setLanguage() ;
	CoInitOS() ;
	VoiceTask = CoCreateTaskEx( voice_task,NULL,5,&voice_stk[VOICE_STACK_SIZE-1], VOICE_STACK_SIZE, 2, FALSE );
	MainTask = CoCreateTask( test_loop,NULL,5,&main_stk[MAIN_STACK_SIZE-1],MAIN_STACK_SIZE);

	CoStartOS() ;
	while(1) ;

#endif
	
	eeReadAll() ;
#ifdef PCBSKY
 #ifndef REVX
	checkAr9x() ;
 #endif
#endif

	SportStreamingStarted = 0 ;
	setLanguage() ;
	lcdSetRefVolt(g_eeGeneral.contrast) ;


#ifdef PCB9XT
	delay_setbl( 0, 100, 0 ) ;
#endif

#ifdef PCBX9D
#ifndef REV9E
	init_adc2() ;
#endif // nREV9E
#endif

	createSwitchMapping() ;
#ifdef PCBSKY
	create6posTable() ;
	init_rotary_encoder() ;
#endif 

#ifdef PCB9XT
//	BlSetAllColours( 0, 100, 0 ) ;
	create6posTable() ;
	if ( g_eeGeneral.enableI2C == 1 )
	{
		init_I2C2() ;
	}
#endif 


#ifdef REV9E
	init_rotary_encoder() ;
#endif // REV9E

#ifdef PCB9XT
	delay_setbl( 100, 0, 100 ) ;
#endif
	com2Configure() ;
#ifdef PCB9XT
	delay_setbl( 100, 100, 0 ) ;
#endif


//progress( 1 ) ;

	// At this point, check for "maintenance mode"
	if ( ( read_trims() & 0x81 )== 0x81 )
	{
		// Do maintenance mode
#ifndef SIMU
		CoInitOS();

		MainTask = CoCreateTask( update_mode,NULL,5,&main_stk[MAIN_STACK_SIZE-1],MAIN_STACK_SIZE);
		CoStartOS();
		while(1) ;
#endif
	
	}

	if ( ( read_trims() & 0x01 )== 0x01 )
	{
		HardwareMenuEnabled = 1 ;
	}

  resetTimer();
	if ( g_eeGeneral.unexpectedShutdown )
	{
		unexpectedShutdown = 1 ;
	}

#ifdef PCBSKY
	start_sound() ;
	setBtBaudrate( g_eeGeneral.bt_baudrate ) ;
	// Set ADC gains here
	set_stick_gain( g_eeGeneral.stickGain ) ;
#endif

#ifdef FRSKY
#ifdef PCBSKY
	telemetry_init( decodeTelemetryType( g_model.telemetryProtocol ) ) ;
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	telemetry_init( decodeTelemetryType( g_model.telemetryProtocol ) ) ;
//	telemetry_init( TEL_FRSKY_SPORT ) ;
#endif
#endif


#if defined(PCBSKY) || defined(PCB9XT)
  checkQuickSelect();
#endif

#ifdef PCBX9D
	{
  	uint8_t i = keyDown(); //check for keystate
		if ( ( i & 6 ) == 6 )
		{
			SystemOptions |= SYS_OPT_MUTE ;
		}
	}
#endif

#ifdef PCBSKY
	PWM->PWM_CH_NUM[0].PWM_CDTYUPD = g_eeGeneral.bright ;
	MAh_used = g_eeGeneral.mAh_used ;
#endif
#ifdef PCBX9D
#if REVPLUS
	backlight_set( g_eeGeneral.bright, 0 ) ;
	backlight_set( g_eeGeneral.bright_white, 1 ) ;
#else
	backlight_set( g_eeGeneral.bright ) ;
#endif
	usbInit() ;
#endif

#ifdef PCB9XT
  usbInit() ;
//	backlightReset() ;
	backlight_on() ;
//	hw_delay( 2000 ) ; // 200uS
//	backlight_on() ;
#endif

//  lcdSetRefVolt(g_eeGeneral.contrast) ;

	uint16_t x ;
	x = g_eeGeneral.volume ;
	if ( g_model.anaVolume )	// Only check if on main screen
	{
		uint16_t divisor ;
#ifdef PCBSKY
		if ( g_model.anaVolume < 4 )
#endif
#ifdef PCBX9D
		if ( g_model.anaVolume < 5 )
#endif
		{
		  getADC_single() ;
			x = anaIn(g_model.anaVolume+3) ;
			divisor = 2048 ;
			x = x * (NUM_VOL_LEVELS-1) / divisor ;
		}
	}
	setVolume( x ) ;

//progress( 2 ) ;
	// Choose here between PPM and PXX

	g_menuStack[1] = menuProcModelSelect ;	// this is so the first instance of [MENU LONG] doesn't freak out!

  //we assume that startup is like pressing a switch and moving sticks.  Hence the lightcounter is set
  //if we have a switch on backlight it will be able to turn on the backlight.
  if(g_eeGeneral.lightAutoOff > g_eeGeneral.lightOnStickMove)
    g_LightOffCounter = g_eeGeneral.lightAutoOff*500;
  if(g_eeGeneral.lightAutoOff <= g_eeGeneral.lightOnStickMove)
    g_LightOffCounter = g_eeGeneral.lightOnStickMove*500;
  check_backlight();

	// moved here and logic added to only play statup tone if splash screen enabled.
  // that way we save a bit, but keep the option for end users!
//  if((g_eeGeneral.speakerMode & 1) == 1)
//	{
    if(!g_eeGeneral.disableSplashScreen)
    {
			voiceSystemNameNumberAudio( SV_WELCOME, V_HELLO, AU_TADA ) ;
//			audioVoiceDefevent( AU_TADA, V_HELLO ) ;
    }
//  }

#ifndef SIMU

	CoInitOS();

#ifdef BLUETOOTH
	BtTask = CoCreateTask(bt_task,NULL,19,&Bt_stk[BT_STACK_SIZE-1],BT_STACK_SIZE);
#endif

	MainTask = CoCreateTask( main_loop,NULL,5,&main_stk[MAIN_STACK_SIZE-1],MAIN_STACK_SIZE);

	LogTask = CoCreateTask(log_task,NULL,17,&Log_stk[LOG_STACK_SIZE-1],LOG_STACK_SIZE);

//#ifdef PCBSKY
	VoiceTask = CoCreateTaskEx( voice_task,NULL,5,&voice_stk[VOICE_STACK_SIZE-1], VOICE_STACK_SIZE, 2, FALSE );
//#endif 
//#ifdef PCBX9D
//	VoiceTask = CoCreateTaskEx( voice_task,NULL,5,&voice_stk[VOICE_STACK_SIZE-1], VOICE_STACK_SIZE, 2, FALSE );
//#endif 

#ifdef	DEBUG
	DebugTask = CoCreateTaskEx( handle_serial,NULL,18,&debug_stk[DEBUG_STACK_SIZE-1],DEBUG_STACK_SIZE, 1, FALSE );
#endif 

#ifdef SERIAL_HOST
	HostTask = CoCreateTaskEx( host, NULL,19, &Host_stk[HOST_STACK_SIZE-1], HOST_STACK_SIZE, 1, FALSE ) ;
#endif 

#ifdef REV9E
void initTopLcd() ;
	initTopLcd() ;
#endif 

	Main_running = 1 ;

//progress( 3 ) ;

	CoStartOS();

	while(1);
#endif
  /*
   * Prevent compiler warnings
   */
//  (void)c;

  /*
   * This return here make no sense.
   * But to prevent the compiler warning:
   * "return type of 'main' is not 'int'
   * we use an int as return :-)
   */
  return(0);
}

#ifndef SIMU
//void tmrBt_Handle( void )
//{
//	CoSetFlag(Bt1SFlag);		// 1 second return,set flag
//}


#ifdef PCBSKY
#define HC05_ENABLE_HIGH		(PIOB->PIO_SODR = PIO_PB12)			// Set bit B12 HIGH
#define HC05_ENABLE_LOW			(PIOB->PIO_CODR = PIO_PB12)			// Set bit B12 LOW
#endif // PCBSKY

#ifdef PCB9XT
#define HC05_ENABLE_HIGH		(GPIOA->BSRRL = GPIO_Pin_5)			// Set bit PA5 HIGH
#define HC05_ENABLE_LOW			(GPIOA->BSRRH = GPIO_Pin_5)			// Set bit PA5 LOW
#endif // PCB9XT


#ifdef BLUETOOTH
OS_FlagID Bt_flag ;
struct t_fifo128 Bt_fifo ;
struct t_serial_tx Bt_tx ;
struct t_serial_tx Com2_tx ;
uint8_t BtTxBuffer[32] ;
uint8_t Com2TxBuffer[32] ;
uint8_t BtLinked ;

//uint8_t BtStartDebug[128] ;
//uint8_t BtStartIndex ;

//void btStartLog( uint8_t c )
//{
//	if ( BtStartIndex < 128 )
//	{
//		BtStartDebug[BtStartIndex++] = c ;
//	}
//}

struct btRemote_t BtRemote[3] ;
uint8_t NumberBtremotes ;

uint16_t Bt_debug ;

uint8_t BtMode[4] ;
uint8_t BtPswd[6] ;
uint8_t BtName[16] ;
uint8_t BtRole[4] ;
uint8_t BtIac[10] ;

uint16_t BtCounters[4] ;
uint16_t BtTotals[4] ;

uint8_t BtMasterSlave = 0 ;
uint16_t BtLastSbusSendTime ;

uint32_t btAddressValid( uint8_t *address )
{
	uint8_t x ;
	x = *address++ ;
	x |= *address++ ;
	x |= *address++ ;
	x |= *address++ ;
	x |= *address++ ;
	x |= *address ;
	return x ;
}

uint32_t btAddressMatch( uint8_t *address1, uint8_t *address2 )
{
	uint32_t x ;
	for ( x = 0 ; x < 6 ; x += 1 )
	{
		if ( *address1++ != *address2++ )
		{
			return 0 ;
		}
	}
	return 1 ;
}

void bt_send_buffer()
{
	Bt_tx.buffer = BtTxBuffer ;
	txPdcBt( &Bt_tx ) ;
	while ( Bt_tx.ready == 1 )
	{
		// Wait
		CoTickDelay(1) ;					// 2mS for now
	}
	Bt_tx.size = 0 ;
}

#define BT_POLL_TIMEOUT		800

void flushBtFifo()
{
	uint16_t rxchar ;

#ifdef PCBSKY
	txmit( '[' ) ;
#endif
	while ( ( rxchar = rxBtuart() ) != 0xFFFF )
	{
#ifdef PCBSKY
txmit( rxchar ) ;
#endif
		// null body, flush fifo
	}
#ifdef PCBSKY
	txmit( ']' ) ;
#endif
}

uint32_t getBtOK( uint32_t errorAllowed, uint32_t timeout )
{
	uint16_t x ;
	uint32_t y ;
	uint16_t rxchar ;
	uint16_t a = 0 ;
	uint16_t b = 0 ;

	x = 'O' ;
	if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
	{
		x = 13 ;	// <CR>
	}
	for( y = 0 ; y < timeout ;  y += 1 )
	{
		if ( ( rxchar = rxBtuart() ) != 0xFFFF )
		{
#ifdef PCBSKY
txmit( rxchar ) ;
#endif
//			btStartLog( rxchar ) ;

			if ( rxchar == x )
			{
				if ( ( x == 'O' ) || ( x == 13 ) )
				{
					x = 'K' ;
					if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
					{
						x = 10 ;	// <LF>
					}
				}
				else
				{
					break ;			// Found "OK"
				}
			}
			if ( rxchar != 13 )
			{
				a = b ;
				b = rxchar ;
			}
		}
		else
		{
			CoTickDelay(1) ;					// 2mS
		}				 
	}
	if ( y < timeout )
	{
		if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
		{
			if ( errorAllowed == 0 )
			{
				if ( ( a != 'O') || ( b != 'K' ) )
				{
					return 0 ;
				}
			}
		}
		return 1 ;
	}
	else
	{
		return 0 ;
	}
}

uint32_t poll_bt_device()
{
#ifdef PCBSKY
txmit('P') ;
#endif
	
	BtTxBuffer[0] = 'A' ;
	BtTxBuffer[1] = 'T' ;
	Bt_tx.size = 2 ;
	if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
	{
		BtTxBuffer[2] = 13 ;
		BtTxBuffer[3] = 10 ;
		Bt_tx.size = 4 ;
	}
	CoTickDelay(10) ;					// 20mS
	flushBtFifo() ;
	bt_send_buffer() ;
	return getBtOK(1, BT_POLL_TIMEOUT ) ;
}

uint8_t BtBaudChangeCount = 0 ;
uint8_t BtBaudChangeIndex = 9 ;

uint32_t changeBtBaudrate( uint32_t baudIndex )
{
	uint16_t x ;

	BtBaudChangeCount += 1 ;
	BtBaudChangeIndex = baudIndex ;

	x = 4 ;		// 9600
	if ( baudIndex == 0 )
	{
		x = 8 ;		// 115200
	}
	else if ( baudIndex == 2 )
	{
		x = 5 ;		// 19200		
	}
	else if ( baudIndex == 3 )
	{
		x = 7 ;		// 57600
	}
	else if ( baudIndex == 4 )
	{
		x = 6 ;		// 38400
	}
	cpystr( &BtTxBuffer[0], (uint8_t *)"AT+BAUD" ) ;

	BtTxBuffer[7] = '0' + x ;
	Bt_tx.size = 8 ;
	if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
	{
		uint8_t *p ;
		p = cpystr( &BtTxBuffer[0], (uint8_t *)"AT+UART=" ) ;
		switch ( x )
		{
			case 4 :
				p = cpystr( p, (uint8_t *)"9600" ) ;
			break ;
			case 5 :
				p = cpystr( p, (uint8_t *)"19200" ) ;
			break ;
			case 6 :
				p = cpystr( p, (uint8_t *)"38400" ) ;
			break ;
			case 7 :
				p = cpystr( p, (uint8_t *)"57600" ) ;
			break ;
			case 8 :
				p = cpystr( p, (uint8_t *)"115200" ) ;
			break ;
		}
		p = cpystr( p, (uint8_t *)",0,0\r\n" ) ;
		Bt_tx.size = p - BtTxBuffer ;

//		uint32_t i ;
//		txmit('{') ;
//		for ( i = 0 ; i < Bt_tx.size ; i +=1 )
//		{
//			txmit( BtTxBuffer[i] ) ;
//		}
//		txmit('}') ;


	}
	bt_send_buffer() ;
	return getBtOK(0, BT_POLL_TIMEOUT ) ;
}

// Getting Master working
// AT+RMAAD - clears paired list
// AT+ROLE=1
// AT+RESET - may be needed
// AT+CMODE=0 - search specific
// AT+INQM=0,5,4 - 5 devices, 4*1.28 seconds search time
// (AT+PSWD=1234) - default value
// AT+INIT - may be needed (Error:(17) allowed
// AT+INQ - searches
// Replies:
// +INQ:address,type,signal
// Pick up address ad pass to:
// AT_RNAME?address
// Replies:
// +RNAME:name
// AT+PAIR=address,timeout
// AT+BIND=address - may not be needed
// AT+LINK=address

uint32_t btTransaction( uint8_t *command, uint8_t *receive, uint32_t length )
{
	uint16_t x ;
	uint32_t y ;
	uint16_t rxchar ;
	uint8_t *end ;

	CoTickDelay(5) ;	// 10mS
	flushBtFifo() ;

	if ( command )
	{
		end = cpystr( &BtTxBuffer[0], command ) ;
		Bt_tx.size = end - BtTxBuffer ;

uputs( (char *)command ) ;

//		uint32_t i ;
//		txmit('{') ;
//		for ( i = 0 ; i < Bt_tx.size ; i +=1 )
//		{
//			txmit( BtTxBuffer[i] ) ;
//		}
//		txmit('}') ;

		bt_send_buffer() ;
	}
	if ( receive == 0 )
	{
		return 1 ;
	}
	x = 0 ;
	for( y = 0 ; y < BT_POLL_TIMEOUT ;  y += 1 )
	{
		if ( ( rxchar = rxBtuart() ) != 0xFFFF )
		{
#ifdef PCBSKY
			txmit( rxchar ) ;
#endif
			if ( length )
			{
//				if ( ( rxchar >= ' ' ) && ( rxchar < 0x80 ) )
//				{
				*receive++ = rxchar ;
				length -= 1 ;
//				}
			}
			if ( rxchar == 'K' )
			{
				if ( x == 'O' )
				{
					*receive = '\0' ;
					break ;			// Found "OK"
				}
			}
			x = rxchar ;
		}
		else
		{
			CoTickDelay(1) ;					// 2mS
		}				 
	}

	if ( y < BT_POLL_TIMEOUT )
	{
		if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
		{
			// flush the CR-LF
			while ( y < BT_POLL_TIMEOUT )
			{
				if ( ( rxchar = rxBtuart() ) != 0xFFFF )
				{
#ifdef PCBSKY
txmit( rxchar ) ;
#endif
					if ( rxchar == 10 )
					{
						if ( x == 13 )
						{
							break ;			// Found CRLF
						}
					}
					x = rxchar ;
				}
				else
				{
					CoTickDelay(1) ;					// 2mS
				}				 
			}
		}
		return 1 ;
	}
	if ( length )
	{
		*receive = '!' ;
	}					
	return 0 ;
}

void btParse( uint8_t *dest, uint8_t *src, uint32_t length )
{
	// pick out data between : and \r
	uint8_t x ;
	uint32_t copy = 0 ;
	while ( (x = *src++) )
	{
		if ( x == ':' )
		{
			copy = 1 ;
		}
		else
		{
			if ( copy )
			{
				if ( x == '\r' )
				{
					break ;
				}
				*dest++ = x ;
				if ( --length == 0 )
				{
					break ;
				}
			}
		}
	}
	*dest = '\0' ;
}



#define BT_ROLE_SLAVE		0
#define BT_ROLE_MASTER	1


uint32_t getBtRole()
{
	uint8_t buffer[20] ;
	btTransaction( (uint8_t *)"AT+ROLE?\r\n", buffer, 19 ) ;
	btParse( BtRole, buffer, 3 ) ;
	if ( BtRole[0] == '1' )
	{
		BtMasterSlave = 2 ;
	}
	else if ( BtRole[0] == '0' )
	{
		BtMasterSlave = 1 ;
	}
	else
	{
		BtMasterSlave = 0 ;
	}
	CoTickDelay(1) ;					// 2mS
	return BtMasterSlave ;
}

uint32_t setBtRole( uint32_t role )
{
	cpystr( &BtTxBuffer[0], (uint8_t *)"AT+ROLE=0\r\n" ) ;
	if ( role )
	{
		BtTxBuffer[8] = '1' ;
	}
	Bt_tx.size = 11 ;
	bt_send_buffer() ;
	return getBtOK(0, BT_POLL_TIMEOUT ) ;
}

uint32_t setBtName( uint8_t *name )	// Max 14 chars
{
	uint8_t *end ;
	end = cpystr( &BtTxBuffer[0], (uint8_t *)"AT+NAME" ) ;
	if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
	{
		*end++ = '=' ;
	}
	end = cpystr( end, name ) ;
	if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
	{
		end = cpystr( end, (uint8_t *)"\r\n" ) ;
	}
	Bt_tx.size = end - BtTxBuffer ;
	bt_send_buffer() ;
	return getBtOK(0, BT_POLL_TIMEOUT ) ;
}

void getBtValues()
{
	uint8_t buffer[20] ;

	getBtRole() ;	
	CoTickDelay(10) ;					// 20mS
	btTransaction( (uint8_t *)"AT+NAME?\r\n", buffer, 19 ) ;
	btParse( BtName, buffer, 19 ) ;
	CoTickDelay(10) ;					// 20mS
//	btTransaction( (uint8_t *)"AT+CMODE?\r\n", buffer, 19 ) ;
//	btParse( BtMode, buffer, 3 ) ;
//	CoTickDelay(1) ;					// 2mS
	btTransaction( (uint8_t *)"AT+PSWD?\r\n", buffer, 19 ) ;
	btParse( BtPswd, buffer, 5 ) ;
	CoTickDelay(10) ;					// 20mS
//	btTransaction( (uint8_t *)"AT+IAC?\r\n", buffer, 19 ) ;
//	btParse( BtIac, buffer, 9 ) ;
//	CoTickDelay(1) ;					// 2mS
}

#define BT_RX_IDLE		0
#define BT_RX_RECEIVE	1
#define BT_RX_STUFF		2

//int16_t BtChannels[16] ;
uint8_t BtFirstByte ; 
uint8_t BtChannelNumber ;
uint8_t BtRxState ;
uint8_t BtRxChecksum ;
uint8_t BtBadChecksum ;

uint8_t BtSbusFrame[28] ;
uint8_t BtSbusIndex = 0 ;
uint8_t BtSbusReceived ;
uint8_t BtRxOccured ;

#if defined(PCBSKY) || defined(PCB9XT)
void processBtRx( int32_t x, uint32_t rxTimeout )
{
	uint16_t rxchar ;

	if ( g_model.BTfunction == 2 )
	{
		if ( x == 2 )
		{
			BtRxState = BT_RX_RECEIVE ;
			BtSbusIndex = 0 ;
			BtRxChecksum = 0 ;
		}
		else
		{
			if ( BtRxState == BT_RX_STUFF )
			{
				x = x ^ 0x80 ;
				BtRxState = BT_RX_RECEIVE ;
			}
			else
			{
				if ( x == 3 )
				{
					BtRxState = BT_RX_STUFF ;
				}
			}
			if ( BtRxState == BT_RX_RECEIVE )
			{
				BtSbusFrame[BtSbusIndex++] = x ;
				BtRxChecksum += x ;
				if ( BtSbusIndex > 25 )
				{
					BtCounters[1] += 1 ;
					BtRxOccured = 1 ;
					if ( BtRxChecksum == 0 )
					{
						if ( processSBUSframe( BtSbusFrame, ( g_eeGeneral.trainerSource == 1 ) ? g_ppmIns : 0, BtSbusIndex ) )
						{
							BtSbusReceived = 1 ;
						}
					}
					else
					{
						BtBadChecksum = BtRxChecksum ;
					}
					BtRxState = BT_RX_IDLE ;
				}
			}
		}
		return ;
	}

	if ( rxTimeout )
	{
		BtCounters[1] += 1 ;
//		if ( ppmInValid == 100 )
//		{
//			ppmInValid = 99 ;
//		}
		if ( processSBUSframe( BtSbusFrame, ( g_eeGeneral.trainerSource == 1 ) ? g_ppmIns : 0, BtSbusIndex ) )
		{
			BtSbusReceived = 1 ;
		}
		if ( ppmInValid == 100 )
		{
			BtCounters[2] += 1 ;
		}
		BtSbusIndex = 0 ;	 
	}
	else
	{
		rxchar = x ;
		BtSbusFrame[BtSbusIndex++] = rxchar ;
		if ( BtSbusIndex > 27 )
		{
			BtSbusIndex = 27 ;
		}
	}
}

	
//	if ( rxTimeout )
//	{
//		BtRxState = BT_RX_IDLE ;
//	}
//	switch ( BtRxState )
//	{
//		case BT_RX_IDLE :
//			BtChannelNumber = x ;
//			BtRxState = BT_RX_FIRST ;
//		break ;

//		case BT_RX_FIRST :
//			BtFirstByte = x ;
//			BtRxState = BT_RX_SECOND ;
//		break ;
	
//		case BT_RX_SECOND :
//		{
//			uint16_t value ;
//			value = x ;
//			value |= (uint16_t)BtFirstByte << 8 ;
//			if ( BtChannelNumber < 16 )
//			{
////				BtChannels[BtChannelNumber] = value ;
//				if ( g_model.BTfunction )
//				{
//  	    	g_ppmIns[BtChannelNumber] = (int16_t)value - 1500 ;
//					ppmInValid = 100 ;
//				}
//			}
//			BtRxState = BT_RX_IDLE ;
//		}
//		break ;
//	}
//}
#endif

//void simulatorDialog::onTimerSerialEvent()
//{
//  uint8_t serialCmd[3] = {0,0,0};

//  TxOutputs outputs;
//  simulator->getValues(outputs);

//    // Send current values to serial
//  if ( port )
//  {
//    if (port->isOpen())
//    {
//	    for (int i=0; i<=7; i++)
//			{
//  	  	uint chval = (uint) map(outputs.chans[i], -1023, 1024, 1000, 2000);
//    		serialCmd[0] = i; // Channel
//  	  	serialCmd[2] = chval & 0xFF; // 1st byte of value
//	    	serialCmd[1] = (chval >> 8) & 0xFF; // 2nd byte of value
//    		port->write( QByteArray::fromRawData ( ( char *)serialCmd, 3 ), 3 );
//    	}
//    }
//  }
//}


/*
Commands to BT module
AT+VERSION 	Returns the software version of the module
AT+BAUDx 	Sets the baud rate of the module:
1 	1200
2 	2400
3 	4800
4 	9600 (Default)
5 	19200
6 	38400
7 	57600
8 	115200
9 	230400
AT+NAME<name here> 	Sets the name of the module

Any name can be specified up to 20 characters
AT+PINxxxx 	Sets the pairing password of the device

Any 4 digit number can be used, the default pincode is 1234
AT+PN 	Sets the parity of the module 

So we could send AT+VERSION at different baudrates until we get a response
Then we can change the baudrate to the required value.
Or maybe just AT and get OK back
*/

uint8_t Bt_ok ;
uint8_t BtBaudrateChanged ;
uint8_t BtScan ;
uint8_t BtScanState ;
uint8_t BtTempBuffer[100] ;
uint8_t BtRoleChange ;
uint8_t BtNameChange ;
uint8_t BtConfigure ;
uint8_t BtScanInit = 0 ;
uint8_t BtRname[32] ;
uint8_t BtCurrentBaudrate ;
uint16_t BtRxTimer ;
uint8_t BtBinAddr[6] ;
uint8_t BtLinkRequest ;
uint8_t BtCurrentLinkIndex ;
uint8_t BtPreviousLinkIndex = 0xFF ;

void btCountersToTotals()
{
	BtTotals[0] = BtCounters[0] ;
	BtTotals[1] = BtCounters[1] ;
	BtTotals[2] = BtCounters[2] ;
	BtTotals[3] = BtCounters[3] ;
	BtCounters[0] = 0 ;
	BtCounters[1] = 0 ;
	BtCounters[2] = 0 ;
	BtCounters[3] = 0 ;
}

void btAddrHex2Bin()
{
	uint8_t chr ;
	uint32_t x ;
	uint32_t y ;
	uint32_t value ;

	x = 0 ;
	y = 0 ;

	value = 0 ;
	for ( ;; )
	{
		chr = BtRemote[0].address[x++] ;
		chr = toupper( chr ) ;
		if ( ( ( chr >= '0' ) && ( chr <= '9' ) ) || ( ( chr >= 'A' ) && ( chr <= 'F' ) ) )
		{
			chr -= '0' ;
			if ( chr > 9 )
			{
				chr -= 7 ;				
			}
			value <<= 4 ;
			value |= chr ;			
		}
		else
		{
			if ( y == 0 )
			{
				BtBinAddr[0] = value >> 8 ;
				BtBinAddr[1] = value ;
			}
			else if ( y == 1 )
			{
				BtBinAddr[2] = value ;
			}
			else
			{
				BtBinAddr[3] = value >> 16 ;
				BtBinAddr[4] = value >> 8 ;
				BtBinAddr[5] = value ;
				break ;
			}
			y += 1 ;
			value = 0 ;
		}
	}
	for ( y = 0 ; y < 4 ; y += 1 )
	{
		if ( btAddressValid( g_eeGeneral.btDevice[y].address ) )
		{
			if ( btAddressMatch( g_eeGeneral.btDevice[y].address, BtBinAddr ) )
			{
				break ;
			}
		}
		else
		{
			g_eeGeneral.btDevice[y].address[0] = BtBinAddr[0] ;
			g_eeGeneral.btDevice[y].address[1] = BtBinAddr[1] ;
			g_eeGeneral.btDevice[y].address[2] = BtBinAddr[2] ;
			g_eeGeneral.btDevice[y].address[3] = BtBinAddr[3] ;
			g_eeGeneral.btDevice[y].address[4] = BtBinAddr[4] ;
			g_eeGeneral.btDevice[y].address[5] = BtBinAddr[5] ;
			STORE_GENERALVARS ;
			break ;
		}
	}
}

uint8_t b2hex( uint8_t c )
{
	c &= 0x0F ;
	if ( c > 9 )
	{
		c += 7 ;
	}
	c += '0' ;
	return c ;
}

uint8_t *btAddrBin2Hex( uint8_t *dest, uint8_t *source )
{
	uint8_t c ;
	
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	*dest++ = ',' ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	*dest++ = ',' ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	*dest = '\0' ;
	return dest ;
	
//	uint8_t d ;
//	uint8_t zeroSupress = 1 ;

//	c = *source++ ;
//	d = b2hex( c >> 4 ) ;
//	if ( zeroSupress )
//	{
//		if ( d != '0' )
//		{
//			zeroSupress = 0 ;
//		}
//	}
//	if ( zeroSupress == 0 )
//	{
//		*dest++ = d ;
//	}
//	d = b2hex( c ) ;
//	if ( zeroSupress )
//	{
//		if ( d != '0' )
//		{
//			zeroSupress = 0 ;
//		}
//	}
//	if ( zeroSupress == 0 )
//	{
//		*dest++ = d ;
//	}
//	c = *source++ ;
//	d = b2hex( c >> 4 ) ;
//	if ( zeroSupress )
//	{
//		if ( d != '0' )
//		{
//			zeroSupress = 0 ;
//		}
//	}
//	if ( zeroSupress == 0 )
//	{
//		*dest++ = d ;
//	}
//	*dest++ = b2hex( c ) ;
//	*dest++ = ',' ;
	
//	zeroSupress = 1 ;
	
//	c = *source++ ;
//	d = b2hex( c >> 4 ) ;
//	if ( zeroSupress )
//	{
//		if ( d != '0' )
//		{
//			zeroSupress = 0 ;
//		}
//	}
//	if ( zeroSupress == 0 )
//	{
//		*dest++ = d ;
//	}
//	*dest++ = b2hex( c ) ;
//	*dest++ = ',' ;
	
//	zeroSupress = 1 ;
	
//	c = *source++ ;
//	d = b2hex( c >> 4 ) ;
//	if ( zeroSupress )
//	{
//		if ( d != '0' )
//		{
//			zeroSupress = 0 ;
//		}
//	}
//	if ( zeroSupress == 0 )
//	{
//		*dest++ = d ;
//	}
//	d = b2hex( c ) ;
//	if ( zeroSupress )
//	{
//		if ( d != '0' )
//		{
//			zeroSupress = 0 ;
//		}
//	}
//	if ( zeroSupress == 0 )
//	{
//		*dest++ = d ;
//	}
//	c = *source++ ;
//	d = b2hex( c >> 4 ) ;
//	if ( zeroSupress )
//	{
//		if ( d != '0' )
//		{
//			zeroSupress = 0 ;
//		}
//	}
//	if ( zeroSupress == 0 )
//	{
//		*dest++ = d ;
//	}
//	d = b2hex( c ) ;
//	if ( zeroSupress )
//	{
//		if ( d != '0' )
//		{
//			zeroSupress = 0 ;
//		}
//	}
//	if ( zeroSupress == 0 )
//	{
//		*dest++ = d ;
//	}
//	c = *source++ ;
//	d = b2hex( c >> 4 ) ;
//	if ( zeroSupress )
//	{
//		if ( d != '0' )
//		{
//			zeroSupress = 0 ;
//		}
//	}
//	if ( zeroSupress == 0 )
//	{
//		*dest++ = d ;
//	}
//	*dest++ = b2hex( c ) ;
//	*dest = '\0' ;
//	return dest ;
}

uint8_t *copyBtAddress( uint8_t *end, uint32_t index )
{
	uint32_t x ;
	uint8_t chr ;
	
	x = 0 ;

	while ( ( chr = BtRemote[index].address[x] ) != ':' )
	{
		x += 1 ;
		*end++ = chr ;
	}
	*end++ = ',' ;
	x += 1 ;	// Skip ':'
	while ( ( chr = BtRemote[index].address[x] ) != ':' )
	{
		x += 1 ;
		*end++ = chr ;
	}
	*end++ = ',' ;
	x += 1 ;	// Skip ':'
	while ( ( chr = BtRemote[index].address[x] ) )
	{
		x += 1 ;
		*end++ = chr ;
	}
	return end ;
}

uint32_t btLink( uint32_t index )
{
	uint32_t x ;
	uint32_t i ;
	uint8_t *end ;

	btTransaction( (uint8_t *)"AT\r\n", 0, 0 ) ;
	CoTickDelay(10) ;					// 20mS
	getBtOK(0, BT_POLL_TIMEOUT ) ;
	CoTickDelay(10) ;					// 20mS
	
//	btTransaction( (uint8_t *)"AT+ADCN\r\n", 0, 0 ) ;
//	CoTickDelay(10) ;					// 40mS
//	i = getBtOK(0, 2000 ) ;
//	CoTickDelay(10) ;					// 20mS
//txmit('|') ;
	
//	btTransaction( (uint8_t *)"AT+MRAD?\r\n", 0, 0 ) ;
//	CoTickDelay(10) ;					// 40mS
//	i = getBtOK(0, 2000 ) ;
//	CoTickDelay(10) ;					// 20mS
//txmit('|') ;
	
//	end = cpystr( BtRname, (uint8_t *)"AT+FSAD=" ) ;
//	end = btAddrBin2Hex( end, g_eeGeneral.btDevice[index].address ) ;
//	*end++ = '\r' ;
//	*end++ = '\n' ;
//	*end = '\0' ;
//	btTransaction( BtRname, 0, 0 ) ;
//	CoTickDelay(10) ;					// 40mS
//	i = getBtOK(0, 2000 ) ;
//txmit('|') ;
	
	if ( BtPreviousLinkIndex != index )
	{
		end = cpystr( BtRname, (uint8_t *)"AT+BIND=" ) ;
		end = btAddrBin2Hex( end, g_eeGeneral.btDevice[index].address ) ;
		*end++ = '\r' ;
		*end++ = '\n' ;
		*end = '\0' ;
		btTransaction( BtRname, 0, 0 ) ;
		CoTickDelay(10) ;					// 40mS
		i = getBtOK(0, 2000 ) ;
	}
	 
	x = 0 ;
	end = cpystr( BtRname, (uint8_t *)"AT+LINK=" ) ;
	end = btAddrBin2Hex( end, g_eeGeneral.btDevice[index].address ) ;
	*end++ = '\r' ;
	*end++ = '\n' ;
	*end = '\0' ;
	btTransaction( BtRname, 0, 0 ) ;
	CoTickDelay(10) ;					// 40mS
	i = getBtOK(0, BT_POLL_TIMEOUT ) ;

#ifdef PCBSKY
txmit(i+'=') ;
#endif
	if ( i == 0 )
	{
		for ( x = 0 ; x < 10 ; x += 1 )
		{
			CoTickDelay(10) ;					// 40mS
			i = getBtOK(0, BT_POLL_TIMEOUT ) ;
#ifdef PCBSKY
txmit(i+'=') ;
#endif
			if ( i )
			{
#ifdef PCBSKY
txmit('V') ;
#endif
				break ;
			}
#ifdef PCBSKY
txmit('U') ;
#endif
		}
	}
	return i ;
}


void btConfigure()
{
	uint32_t i ;
	uint32_t j ;
	
	HC05_ENABLE_HIGH ;						// Set bit B12 HIGH
	CoTickDelay(10) ;					// 20mS
	BtConfigure = 0x40 ;
	btTransaction( (uint8_t *)"AT+CLASS=0\r\n", 0, 0 ) ;
	CoTickDelay(10) ;					// 40mS
	i = getBtOK(0, BT_POLL_TIMEOUT ) ;
#ifdef PCBSKY
txmit(i+'=') ;
#endif
	BtConfigure = 0x41 ;
	btTransaction( (uint8_t *)"AT+CMODE=0\r\n", 0, 0 ) ;
	CoTickDelay(10) ;					// 40mS
	i = getBtOK(0, BT_POLL_TIMEOUT ) ;
#ifdef PCBSKY
txmit(i+'=') ;
#endif
	BtConfigure = 0x42 ;
//	btTransaction( (uint8_t *)"AT+INIT\r\n", 0, 0 ) ;
//	CoTickDelay(10) ;					// 20mS
//	i = getBtOK(1, BT_POLL_TIMEOUT ) ;
//	BtConfigure = 0x43 ;
	btTransaction( (uint8_t *)"AT+RMAAD\r\n", 0, 0 ) ;
	CoTickDelay(10) ;					// 40mS
	i = getBtOK(0, 2000 ) ;
	BtConfigure = 0x44 ;
#ifdef PCBSKY
txmit('R') ;
txmit(i+'=') ;
#endif
// AT+PAIR=address,timeout
//	uint8_t *end ;
//	for ( j = 0 ; j < 4 ; j += 1 )
//	{
//		if ( btAddressValid( g_eeGeneral.btDevice[j].address ) )
//		{
//			end = cpystr( BtRname, (uint8_t *)"AT+PAIR=" ) ;
//			end = btAddrBin2Hex( end, g_eeGeneral.btDevice[j].address ) ;
//			end = cpystr( end, (uint8_t *)",5\r\n" ) ;
//			btTransaction( BtRname, 0, 0 ) ;
//			CoTickDelay(10) ;					// 40mS
//			i = getBtOK(0, BT_POLL_TIMEOUT ) ;
//txmit('A') ;
//txmit(i+'=') ;
//			if ( i == 0 )
//			{
//				uint32_t k ;
//				for ( k = 0 ; k < 12 ; k += 1 )
//				{
//					i = getBtOK(0, BT_POLL_TIMEOUT ) ;
//txmit('B') ;
//txmit(i+'=') ;
//				}
//			}
//			BtConfigure = 0x45 + j ;
//		}
//		else
//		{
//			break ;
//		}
//	}
	btTransaction( (uint8_t *)"AT+INQM=0,5,4\r\n", 0, 0 ) ;
	CoTickDelay(10) ;					// 40mS
	for ( j = 0 ; j < 12 ; j += 1 )
	{
		i = getBtOK(0, 225 ) ;
#ifdef PCBSKY
txmit(i+'=') ;
#endif
		BtConfigure = 0x49 + j ;
	}
	BtConfigure = 0 ;
	CoTickDelay(10) ;					// 20mS
	HC05_ENABLE_HIGH ;						// Set bit B12 HIGH
	CoTickDelay(10) ;					// 20mS

#ifndef PCBSKY
	(void ) i ;
#endif
}


// Bits in btBits
#define BT_RX_TIMEOUT				1
#define BT_SLAVE_SEND_SBUS	2
#define BT_IS_SLAVE					4
#define BT_RX_DATA					8

void bt_task(void* pdata)
{
	uint32_t x ;
	int32_t y ;
	uint16_t lastTimer = 0 ;
//	uint32_t receiveTimeout = 1 ;
	uint32_t btBits = 0 ;

	while ( Activated == 0 )
	{
		CoTickDelay(10) ;					// 20mS
	}

	while ( (x = CoGetOSTime() ) < 400 )
	{
		CoTickDelay(10) ;					// 20mS
	}

#ifdef PCB9XT
	if ( g_eeGeneral.enableI2C == 2 )
	{
		if ( g_eeGeneral.btComPort == 2 )
		{
			stop_I2C2() ;
			com3Init( 115200 ) ;
		}
	}
#endif

//	static uint32_t count ;
	Bt_flag = CoCreateFlag(TRUE,0) ;
	Bt_tx.size = 0 ;

// Look for BT module baudrate, try 115200, and 9600
// Already initialised to g_eeGeneral.bt_baudrate
// 0 : 115200, 1 : 9600, 2 : 19200, 3 : 57600, 4 : 38400
	
	x = g_eeGeneral.bt_baudrate ;

	if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
	{
		HC05_ENABLE_HIGH ;						// Set bit B12 HIGH
		CoTickDelay(5) ;					// 10mS
	}

	uint32_t found = 0 ;
	do
	{
		Bt_ok = poll_bt_device() ;		// Do we get a response?

		for ( y = 0 ; y <= NUM_BT_BAUDRATES ; y += 1 )
		{
			if ( Bt_ok == 0 )
			{
				x += 1 ;
				if ( x >= NUM_BT_BAUDRATES )
				{
					x = 0 ;
				}
				setBtBaudrate( x ) ;
				CoTickDelay(5) ;					// 10mS
				Bt_ok = poll_bt_device() ;		// Do we get a response?
			}
		}

		if ( Bt_ok )
		{
			Bt_ok = x + 1 ;		
			BtCurrentBaudrate = x ;
			if ( x != g_eeGeneral.bt_baudrate )
			{
				x = g_eeGeneral.bt_baudrate ;
				// Need to change Bt Baudrate
				Bt_ok = changeBtBaudrate( x ) ;
				CoTickDelay(10) ;	// 20mS, give time for the <crlf> to be sent
//				setBtBaudrate( x ) ;
				// Continue with the current baudrate, HC-05 needs power cycle to change
			}
			else
			{
				found = 1 ;
			}
		}
//		else
//		{
//	//		txmit( 'S' ) ;
//		btStartLog( 'X' ) ;
//		btStartLog( '0' + g_eeGeneral.bt_baudrate ) ;
//			setBtBaudrate( g_eeGeneral.bt_baudrate ) ;
//		}

	} while (found == 0 ) ;

	CoTickDelay(1) ;					// 2mS
	if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
	{
		CoTickDelay(1) ;					// 2mS
		getBtValues() ;
		CoTickDelay(1) ;					// 2mS
		btTransaction( (uint8_t *)"AT+INIT\r\n", 0, 0 ) ;
		CoTickDelay(10) ;					// 20mS
		getBtOK(1, BT_POLL_TIMEOUT ) ;
		CoTickDelay(10) ;					// 20mS
		btTransaction( (uint8_t *)"AT\r\n", 0, 0 ) ;
		CoTickDelay(10) ;					// 20mS
		getBtOK(0, BT_POLL_TIMEOUT ) ;
		CoTickDelay(10) ;					// 20mS
		HC05_ENABLE_LOW ;							// Set bit B12 LOW
	}

	BtCurrentLinkIndex = g_model.btDefaultAddress ;
	
	while(1)
	{
		BtCounters[2] += 1 ;
		if ( ( g_eeGeneral.BtType == BT_TYPE_HC06 )
				 || ( ( g_eeGeneral.BtType == BT_TYPE_HC05 ) && ( BtMasterSlave == 1 ) ) )
		{
			btBits |= BT_IS_SLAVE ;			
		}
		else
		{
			btBits &= ~BT_IS_SLAVE ;
		}

#ifndef PCB9XT 
		if ( g_model.com2Function == COM2_FUNC_BTDIRECT )	// BT <-> COM2
		{
			static uint32_t pqrs = 0 ;
			if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
			{
				HC05_ENABLE_HIGH ;						// Set bit B12 HIGH
			}
			// Send data to COM2
			if ( Bt_tx.ready == 0 )	// Buffer available
			{
				Bt_tx.size = 0 ;
				while ( ( y = get_fifo128( &Bt_fifo ) ) != -1 )
				{
					BtTxBuffer[Bt_tx.size++] = y ;
					if ( Bt_tx.size > 31 )
					{
						break ;
					}	
				}
				if ( Bt_tx.size )
				{
					Bt_tx.buffer = BtTxBuffer ;
					txPdcBt( &Bt_tx ) ;
				}
			}

			if ( ++pqrs > 500 )
			{
				pqrs = 0 ;
				Com2TxBuffer[Com2_tx.size++] = '@' ;
				Com2_tx.buffer = Com2TxBuffer ;
				txPdcCom2( &Com2_tx ) ;
				Bt_ok += 1 ;
			}
			if ( Com2_tx.ready == 0 )	// Buffer available
			{
				Com2_tx.size = 0 ;
				while( ( x = rxBtuart() ) != (uint32_t)-1 )
				{
					Com2TxBuffer[Com2_tx.size++] = x ;
					if ( Com2_tx.size > 31 )
					{
						break ;
					}	
				}
				if ( Com2_tx.size )
				{
					Com2_tx.buffer = Com2TxBuffer ;
					txPdcCom2( &Com2_tx ) ;
				}
			}
			CoTickDelay(1) ;					// 2mS for now
		}
		else
#endif	// nPCB9XT
		{
		
			x = CoWaitForSingleFlag( Bt_flag, 1 ) ;		// Wait for data in Fifo
			if ( x == E_OK )
			{
				// We have some data in the Fifo
				while ( ( y = get_fifo128( &Bt_fifo ) ) != -1 )
				{
					BtTxBuffer[Bt_tx.size++] = y ;
					if ( Bt_tx.size > 31 )
					{
						bt_send_buffer() ;
					}
				}
			}
			else if ( Bt_tx.size )
			{
				bt_send_buffer() ;
			}
			else if ( BtBaudrateChanged )
			{
				if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
				{
					HC05_ENABLE_HIGH ;						// Set bit B12 HIGH
				}
				CoTickDelay(10) ;					// 20mS for now
				changeBtBaudrate( g_eeGeneral.bt_baudrate ) ;
				CoTickDelay(10) ;					// 20mS for now
//				setBtBaudrate( g_eeGeneral.bt_baudrate ) ;
				if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
				{
					HC05_ENABLE_LOW ;							// Set bit B12 LOW
				}
				BtBaudrateChanged = 0 ;
			}
			else if ( BtRoleChange & 0x80 )
			{
				HC05_ENABLE_HIGH ;						// Set bit B12 HIGH
				uint8_t newRole = BtRoleChange & 0x01 ;
				BtRoleChange = 0x40 ;
				if ( newRole+1 != BtMasterSlave )
				{
					setBtRole( newRole ) ;
					getBtRole() ;
				}
				BtRoleChange = 0 ;
				HC05_ENABLE_LOW ;							// Set bit B12 LOW
			}
			else if ( BtNameChange & 0x80 )
			{
				uint8_t *pname = g_eeGeneral.btName ;
				if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
				{
					HC05_ENABLE_HIGH ;						// Set bit B12 HIGH
					pname = BtName ;
				}
				BtNameChange = 0x40 ;
				setBtName( pname ) ;
				BtNameChange = 0 ;
				HC05_ENABLE_LOW ;							// Set bit B12 LOW
			}
			else if ( BtConfigure & 0x80 )
			{
				btConfigure() ;
			}
			else if ( BtScan )
			{
				BtScan = 0 ;
				if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
				{
					uint32_t i ;
					uint32_t j ;
					uint16_t rxchar ;

					HC05_ENABLE_HIGH ;						// Set bit B12 HIGH
					CoTickDelay(1) ;					// 2mS
					flushBtFifo() ;
					if ( BtScanInit == 0)
					{
						BtScanInit = 1 ;
						setBtRole( 1 ) ;
						for ( i = 0 ; i < 100 ; i += 1 )
						{
							BtTempBuffer[0] = 0 ;
						}
						BtScanState = 1 ;
						CoTickDelay(20) ;					// 40mS
	//					btTransaction( (uint8_t *)"AT+RESET\r\n", 0, 0 ) ;
	//					CoTickDelay(20) ;					// 40mS
	//					getBtOK(0) ;
	//					btTransaction( (uint8_t *)"AT+RMAAD\r\n", 0, 0 ) ;
	//					CoTickDelay(20) ;					// 40mS
	//					getBtOK(0) ;
						btTransaction( (uint8_t *)"AT+INIT\r\n", 0, 0 ) ;
						CoTickDelay(10) ;					// 20mS
						i = getBtOK(1, BT_POLL_TIMEOUT ) ;
//						txmit(i+'=') ;
					}
					BtScanState = 2 ;
					CoTickDelay(100) ;					// 200mS
					flushBtFifo() ;
					btTransaction( (uint8_t *)"AT+INQ\r\n", 0, 0 ) ;
//					txmit('~') ;
					BtScanState = 3 ;
					j = 0 ;
					x = 0 ;
					for ( i = 0 ; i < 2800 ; i += 1 )
					{
						while ( ( rxchar = rxBtuart() ) != 0xFFFF )
						{
#ifdef PCBSKY
							txmit( rxchar ) ;
#endif
							if ( rxchar == '+' )
							{
								x = 1 ;
							}
							if ( x && ( j < 99 ) )
							{
								BtTempBuffer[j++] = rxchar ;
							}
							if ( rxchar == '\r' )
							{
								x = 0 ;
							}
						}
						CoTickDelay(1) ;					// 2mS
					}
					BtTempBuffer[j] = 0 ;
					i = 0 ;
					x = 0 ;
					uint32_t y ;
					// Looking for: +INQ:2:72:D2224,3E0104,FFBC
					NumberBtremotes = 0 ;
					if ( j >= 4 )
					{
						while ( i < j-3 )
						{
							if ( BtTempBuffer[i] == '+' )
							{
								if ( BtTempBuffer[i+1] == 'I' )
								{
									if ( BtTempBuffer[i+2] == 'N' )
									{
										if ( BtTempBuffer[i+3] == 'Q' )
										{
											y = 0 ;
											i += 5 ;		// Skip ':'
											while ( BtTempBuffer[i] != ',' )
											{
												BtRemote[NumberBtremotes].address[y++] = BtTempBuffer[x++] = BtTempBuffer[i++] ;
												if ( y > 14 )
												{
													y = 14 ;
												}
												if ( i >= j )
												{
													break ;
												}
											}
											BtRemote[NumberBtremotes].address[y] = '\0' ;
											BtTempBuffer[x++] = '\r' ;
											BtTempBuffer[x++] = '\n' ;
											NumberBtremotes += 1 ;
										}
									}
								}
							}
							i += 1 ;
						}
					}
//					txmit('W') ;
					BtTempBuffer[x] = '\0' ;
					BtScanState = 4 ;
					// Next we want the remote name
					if ( NumberBtremotes )
					{
						btAddrHex2Bin() ;
					}

					if ( NumberBtremotes )
					{
						// Got a response from the +INQ
						// Send AT+RNAME?xxxx,xx,xxxxxx\r\n
						uint8_t *end ;
						x = 0 ;
						end = cpystr( BtRname, (uint8_t *)"AT+RNAME?" ) ;
						end = copyBtAddress( end, 0 ) ;

						*end++ = '\r' ;
						*end++ = '\n' ;
						*end = '\0' ;
						if ( btTransaction( BtRname, BtRemote[0].name, 14 ) )
						{
//							txmit('Y') ;
						}
						else
						{
							for ( i = 0 ; i < 7 ; i += 1 )
							{
								if ( btTransaction( 0, BtRemote[0].name, 14 ) )
								{
//									txmit('V') ;
									break ;
								}
//								txmit('U') ;
							}
						}
//						txmit('X') ;
						btTransaction( (uint8_t *)"AT+STATE?\r\n", 0, 0 ) ;
						CoTickDelay(10) ;					// 40mS
						i = getBtOK(0, BT_POLL_TIMEOUT ) ;
//						txmit(i+'=') ;
						btTransaction( (uint8_t *)"AT+DISC\r\n", 0, 0 ) ;
						CoTickDelay(10) ;					// 40mS
						i = getBtOK(0, BT_POLL_TIMEOUT ) ;
//						txmit(i+'=') ;
						
//						x = 0 ;
//						end = cpystr( BtRname, (uint8_t *)"AT+LINK=" ) ;
//						while ( ( chr = BtRemote[0].address[x] ) != ':' )
//						{
//							x += 1 ;
//							*end++ = chr ;
//						}
//						*end++ = ',' ;
//						x += 1 ;	// Skip ':'
//						while ( ( chr = BtRemote[0].address[x] ) != ':' )
//						{
//							x += 1 ;
//							*end++ = chr ;
//						}
//						*end++ = ',' ;
//						x += 1 ;	// Skip ':'
//						while ( ( chr = BtRemote[0].address[x] ) )
//						{
//							x += 1 ;
//							*end++ = chr ;
//						}
//						x += 1 ;
//						*end++ = '\r' ;
//						*end++ = '\n' ;
//						*end = '\0' ;
//						btTransaction( BtRname, 0, 0 ) ;
//						CoTickDelay(10) ;					// 40mS
//						i = getBtOK(0, BT_POLL_TIMEOUT ) ;
						
//						txmit(i+'=') ;
//						if ( i == 0 )
//						{
//							for ( x = 0 ; x < 5 ; x += 1 )
//							{
//								CoTickDelay(10) ;					// 40mS
//								i = getBtOK(0, BT_POLL_TIMEOUT ) ;
//								if ( i )
//								{
//									break ;
//								}
//							}
//						}


//						i = btLink(0) ;
//						if ( i )
//						{
//							BtLinked = 1 ;
//							BtRxTimer = 1000 ;
//							BtRxOccured = 0 ;
//						}
//						btTransaction( (uint8_t *)"AT+STATE?\r\n", 0, 0 ) ;
						CoTickDelay(10) ;					// 40mS
//						i = getBtOK(0, BT_POLL_TIMEOUT * 2 ) ;
//						txmit(i+'=') ;
					}
					HC05_ENABLE_LOW ;							// Set bit B12 LOW
				}
			}
			else if ( BtLinkRequest )
			{
				if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
				{
//					uint32_t i ;
					HC05_ENABLE_HIGH ;						// Set bit B12 HIGH
					CoTickDelay(10) ;					// 40mS
//					i = btLink( BtLinkRequest & 3 ) ;
					btLink( BtLinkRequest & 3 ) ;
					BtCurrentLinkIndex = BtLinkRequest & 3 ;
//					if ( i )
//					{
						BtLinked = 1 ;
						BtRxTimer = 1000 ;
						BtRxOccured = 0 ;
//					}
					CoTickDelay(10) ;					// 40mS
					HC05_ENABLE_LOW ;							// Set bit B12 LOW
					BtLinkRequest = 0 ;
				}
			}

//		txmitBt( 'X' ) ;		// Send an X to Bluetooth every second for testing
//#ifdef PCBSKY
			if ( ( BtMasterSlave == 2 ) && ( g_model.autoBtConnect ) && 
					btAddressValid( g_eeGeneral.btDevice[BtCurrentLinkIndex].address ) )
			{
				BtLinked = 1 ;
			}

			if ( ( ( BtMasterSlave == 2 ) && ( g_model.BTfunction == 2 ) && BtLinked )
					 || ( btBits & BT_SLAVE_SEND_SBUS ) )
			{
				uint32_t i ;
				if ( btBits & BT_SLAVE_SEND_SBUS )
				{
					btBits &= ~BT_SLAVE_SEND_SBUS ;
				}
				else
				{
					uint16_t x ;
					do
					{
						CoTickDelay(1) ;					// 2mS
						x = getTmr2MHz() - BtLastSbusSendTime ;
					} while ( x < 28000 ) ;
					BtLastSbusSendTime += 28000 ;
//					} while ( x < 40000 ) ;
//					BtLastSbusSendTime += 40000 ;
				}
				// Send Sbus Frame
  			uint8_t *p = BtTxBuffer ;
				uint32_t outputbitsavailable = 0 ;
				uint32_t outputbits = 0 ;
				uint8_t checksum = 0x0F ;

				*p++ = 2 ;		// Marker 
				*p++ = 0x0F ;
				for ( i = 0 ; i < 16 ; i += 1 )
			 	{
					int16_t x = g_chans512[i] ;
					x *= 4 ;
					x += x > 0 ? 4 : -4 ;
					x /= 5 ;
					x += 0x3E0 ;
					if ( x < 0 )
					{
						x = 0 ;
					}
					if ( x > 2047 )
					{
						x = 2047 ;
					}
					outputbits |= x << outputbitsavailable ;
					outputbitsavailable += 11 ;
					while ( outputbitsavailable >= 8 )
					{
						uint8_t j = outputbits ;
						checksum += j ;
						if ( ( j == 2 ) || ( j == 3 ) )
						{
							j ^= 0x80 ;
							*p++ = 3 ;		// "stuff"
						}
            *p++ = j ;
						outputbits >>= 8 ;
						outputbitsavailable -= 8 ;
					}
				}
				*p++ = 0 ;
				*p++ = 0 ;
				checksum = -checksum ;
				if ( ( checksum == 2 ) || ( checksum == 3 ) )
				{
					*p++ = 3 ;		// "stuff"
					checksum ^= 0x80 ;
				}
				*p++ = checksum ;
				Bt_tx.size = p - BtTxBuffer ;
				bt_send_buffer() ;
				BtCounters[0] += 1 ;
			}

			if ( g_model.BTfunction == 2 )
			{
				while( ( x = rxBtuart() ) != (uint32_t)-1 )
				{
					if ( BtRxTimer < 100 )
					{
						BtRxTimer = 100 ;
					}
					processBtRx( x, 0 ) ;
					if ( BtSbusReceived )
					{
						BtSbusReceived = 0 ;
						if ( btBits & BT_IS_SLAVE )
						{
							btBits |= BT_SLAVE_SEND_SBUS ;						
						}
					}
					lastTimer = getTmr2MHz() ;
					btBits &= ~BT_RX_TIMEOUT ;
				}
			}
			else
			{
				while( ( x = rxBtuart() ) != (uint32_t)-1 )
				{
					if ( BtRxTimer < 100 )
					{
						BtRxTimer = 100 ;
					}
					processBtRx( x, 0 ) ;
					lastTimer = getTmr2MHz() ;
					btBits &= ~BT_RX_TIMEOUT ;
				}
				if ( (btBits & BT_RX_TIMEOUT) == 0 )
				{
					if ( ( ( getTmr2MHz() - lastTimer ) & 0x0000FFFF ) > 10000 )
					{
						btBits |= BT_RX_TIMEOUT ;
						processBtRx( 0, 1 ) ;
					}
				}
			}
		}

		if ( BtRxTimer == 0 )
		{
			if ( BtLinked )
			{
//				uint32_t i ;
				if ( BtRxOccured )
				{
					putSystemVoice( SV_BT_LOST, 0 ) ;
				}
				if ( g_model.autoBtConnect )
				{
					BtCurrentLinkIndex =  g_model.btDefaultAddress ;
				}

				if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
				{
					HC05_ENABLE_HIGH ;						// Set bit B12 HIGH
					CoTickDelay(10) ;					// 40mS
				}
//				i = btLink(BtCurrentLinkIndex) ;
				btLink(BtCurrentLinkIndex) ;
//				if ( i )
//				{
					BtRxTimer = 1000 ;
					BtRxOccured = 0 ;
//				}
				if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
				{
					CoTickDelay(10) ;					// 40mS
					HC05_ENABLE_LOW ;							// Set bit B12 LOW
				}
			}
		}

//#endif
	}
}
#endif	// BLUETOOTH

extern const char *openLogs( void ) ;
extern void writeLogs( void ) ;
extern void closeLogs( void ) ;

uint8_t LogsRunning = 0 ;
uint16_t LogTimer = 0 ;

void log_task(void* pdata)
{
	while ( Activated == 0 )
	{
		CoTickDelay(10) ;					// 20mS
	}
	
  uint16_t tgtime = get_tmr10ms() ;		// 1 sec
	
	while(1)
	{
		// This needs to be a bit more accurate than
		// just a delay to get the correct logging rate
		do
		{
			CoTickDelay(5) ;					// 10mS
		} while( (uint16_t)(get_tmr10ms() - tgtime ) < 50 ) ;
//		LogTimer = 0 ;
  	tgtime += 50 ;
		LogTimer += 1 ;

		if ( g_model.logSwitch )
		{
			if ( getSwitch00( g_model.logSwitch ) )
			{	// logs ON
				if ( ( LogsRunning & 1 ) == 0 )
				{	// were off
					LogsRunning = 3 ;		// On and changed
					LogTimer = 0 ;
				}
			}
			else
			{	// logs OFF
				if ( LogsRunning & 1 )
				{	// were on
					LogsRunning = 2 ;		// Off and changed
				}
			}

			if ( LogsRunning & 2 )
			{
				if ( LogsRunning & 1 )
				{
					const char *result ;
					// Start logging
					SdAccessRequest = 1 ;
					while (lockOutVoice() == 0 )
					{
						CoTickDelay(1) ;					// 2mS
					}
					SdAccessRequest = 0 ;
					result = openLogs() ;
					unlockVoice() ;
					if ( result != NULL )
					{
    				audioDefevent( AU_SIREN ) ;
					}
				}
				else
				{
					// Stop logging
					closeLogs() ;
				}
				LogsRunning &= ~2 ;				
			}

			if ( LogsRunning & 1 )
			{
				// log Data (depending on Rate)
				LogsRunning += 0x40 ;
				uint8_t mask = 0x40 ;
				if ( g_model.logRate == 2 )		// 0.5 secs
				{
					mask = 0 ;
				}
				else if ( g_model.logRate )		// 2.0 secs
				{
					mask = 0xC0 ;
				}
				if ( ( LogsRunning & mask ) == 0 )
				{
					writeLogs() ;
				}
			}
		}
	}
}


#endif	// SIMU

#if defined(PCBSKY) || defined(PCB9XT)
void telem_byte_to_bt( uint8_t data )
{
#ifndef SIMU
        put_fifo128( &Bt_fifo, data ) ;
        CoSetFlag( Bt_flag ) ;                  // Tell the Bt task something to do
#endif
}
#endif

//uint32_t UsbTimer = 0 ;
//extern void usbMassStorage( void ) ;

uint16_t PowerStatus ;


#ifdef PCB_TEST_9XT

void wait500()
{
	CoTickDelay(50) ;					// 0.5 secs
	wdt_reset() ;
	CoTickDelay(50) ;					// 0.5 secs
	wdt_reset() ;
	CoTickDelay(50) ;					// 0.5 secs
	wdt_reset() ;
	CoTickDelay(50) ;					// 0.5 secs
	wdt_reset() ;
	CoTickDelay(50) ;					// 0.5 secs
	wdt_reset() ;
}

void test_loop( void* pdata )
{
	uint32_t timer = 0 ;
	Activated = 1 ;
	soft_power_off() ;
	setVolume( 8 ) ;

	while (1)
	{
		BlSetAllColours( 100, 0, 0 ) ;
		if ( timer == 2 )
		{
			putVoiceQueue( 423 ) ;
		}
		if ( timer == 0 )
		{
			audioDefevent( AU_SIREN ) ;
		}
		wait500() ;					// 0.5 secs
		BlSetAllColours( 0, 100, 0 ) ;
		wait500() ;					// 0.5 secs
		BlSetAllColours( 0, 0, 100 ) ;
		wait500() ;					// 0.5 secs
		BlSetAllColours( 100, 100, 100 ) ;
		wait500() ;					// 0.5 secs
		timer += 1 ;
		if ( timer > 3 )
		{
			timer = 0 ;
		}
	}
}
#endif


#ifndef PCBX9D
uint32_t countExtraPots()
{
	uint32_t count = 0 ;
	if ( g_eeGeneral.extraPotsSource[0] )
	{
		count = 1 ;
	}
	if ( g_eeGeneral.extraPotsSource[1] )
	{
		count += 1 ;
	}
	if ( g_eeGeneral.extraPotsSource[2] )
	{
		count += 1 ;
	}
	if ( g_eeGeneral.extraPotsSource[3] )
	{
		count += 1 ;
	}
	return count ;
}
#endif

// This is the main task for the RTOS
void main_loop(void* pdata)
{
#ifdef PCBX9D
	NumExtraPots = NUM_EXTRA_POTS ;
#else
	NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
#endif

#ifdef PCB9XT
	backlight_on() ;
#endif

#ifdef PCBSKY
	if ( ( ( ResetReason & RSTC_SR_RSTTYP ) != (2 << 8) ) && !unexpectedShutdown )	// Not watchdog
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	if ( ( ( ResetReason & RCC_CSR_WDGRSTF ) != RCC_CSR_WDGRSTF ) && !unexpectedShutdown )	// Not watchdog
#endif
	{
		doSplash() ;
  	getADC_single();
  	checkTHR();
		checkCustom() ;
  	checkSwitches();
		checkAlarm();
//		checkCalibrate() ;
		checkWarnings();
		clearKeyEvents(); //make sure no keys are down before proceeding

		if ( g_model.modelVoice == -1 )
		{
			putNamedVoiceQueue( g_model.modelVname, VLOC_MNAMES ) ;
		}
		else
		{
			putVoiceQueue( ( g_model.modelVoice + 260 ) | VLOC_NUMUSER ) ;
		}
	}
	VoiceCheckFlag |= 2 ;// Set switch current states


// Preload battery voltage
#ifdef PCBSKY
  int32_t ab = anaIn(12);
#endif
#ifdef PCBX9D
  int32_t ab = anaIn(12);
#endif
#ifdef PCB9XT
  int32_t ab = anaIn(12);
#endif

  ab = ( ab + ab*(g_eeGeneral.vBatCalib)/128 ) * 4191 ;
#ifdef PCBSKY
        ab /= 55296  ;
        g_vbat100mV = ab + 3 + 3 ;// Also add on 0.3V for voltage drop across input diode
#endif
#ifdef PCBX9D
        ab /= 57165  ;
        g_vbat100mV = ab ;
#endif
#ifdef PCB9XT
        ab /= 64535  ;
        g_vbat100mV = ab ;
#endif

#ifdef PCBSKY
	// Must do this to start PPM2 as well
	init_main_ppm( 3000, 0 ) ;		// Default for now, initial period 1.5 mS, output off
 	perOut( g_chans512, NO_DELAY_SLOW | FADE_FIRST | FADE_LAST ) ;
	startPulses() ;		// using the required protocol

	start_ppm_capture() ;

//  FrskyAlarmSendState |= 0x40 ;
#endif

#ifdef PCBX9D
// Switches PE2,7,8,9,13,14
	configure_pins( 0x6384, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;

	init_no_pulses( 0 ) ;
	init_no_pulses( 1 ) ;

//	init_trainer_ppm() ;

	init_trainer_capture() ;

	rtcInit() ;
#endif

#ifdef PCB9XT
	init_no_pulses( 0 ) ;
	init_no_pulses( 1 ) ;

//	init_trainer_ppm() ;

	init_trainer_capture() ;

	rtcInit() ;

#endif

	heartbeat_running = 1 ;

  if (!g_eeGeneral.unexpectedShutdown)
	{
    g_eeGeneral.unexpectedShutdown = 1;
    STORE_GENERALVARS ;
  }

#if defined(PCBX9D) || defined(PCB9XT)
 	perOut( g_chans512, NO_DELAY_SLOW | FADE_FIRST | FADE_LAST ) ;
	startPulses() ;		// using the required protocol
#endif

	Activated = 1 ;
  
	while (1)
	{

#if defined(REVB) || defined(PCBX9D) || defined(PCB9XT)
		
#ifdef REV9E
		uint32_t powerState = check_soft_power() ;
		if ( ( powerState == POWER_X9E_STOP ) || ( powerState == POWER_OFF ) )		// power now off
#else
// #ifdef PCB9XT
//		PowerStatus = GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) ;
//		PowerStatus = check_soft_power() ;
//    if ( 0 )
// #else
		if ( ( check_soft_power() == POWER_OFF ) )		// power now off
// #endif
#endif
		{
			// Time to switch off
			lcd_clear() ;
			lcd_putsn_P( 4*FW, 3*FH, "SHUTTING DOWN", 13 ) ;

#ifdef REV9E
extern uint8_t PowerState ;
	lcd_outhex4( 20, 0, PowerState ) ;
#endif

#ifdef PCBSKY
			// Stop pulses out at this point, hold pin low
			module_output_low() ;
#endif


//#ifdef PCBSKY
//			if ( goto_usb )
//			{
//				lcd_putsn_P( 7*FW, 4*FH, "TO USB", 6 ) ;
//			}
//#endif
			refreshDisplay() ;

			// Wait for OK to turn off
			// Currently wait 1 sec, needs to check eeprom finished

			if ( LogsRunning & 1 )
			{
				closeLogs() ;
			}

  		g_eeGeneral.unexpectedShutdown = 0;
#ifdef PCBSKY
			MAh_used += Current_used/3600 ;
			if ( g_eeGeneral.mAh_used != MAh_used )
			{
				g_eeGeneral.mAh_used = MAh_used ;
			}
#endif
      STORE_GENERALVARS ;		// To make sure we write "unexpectedShutdown"

  		uint16_t tgtime = get_tmr10ms() ;
	  	while( (uint16_t)(get_tmr10ms() - tgtime ) < 70 ) // 50 - Half second
//	  	while( (uint16_t)(get_tmr10ms() - tgtime ) < 200 ) // 50 - Half second
  		{
				if ( (uint16_t)(get_tmr10ms() - tgtime ) > 60 )
				{
#ifndef REV9E				
					if ( check_soft_power() == POWER_ON )
					{
#ifdef PCBSKY
						if ( MaintenanceRunning == 0)
						{					
							module_output_active() ;
						}
#endif
						break ;		// Power back on
					}
#endif // nREV9E
				}
				wdt_reset() ;

				if ( ee32_check_finished() == 0 )
				{
					lcd_putsn_P( 5*FW, 5*FH, "EEPROM BUSY", 11 ) ;
					tgtime = get_tmr10ms() ;
				}
				else
				{
					lcd_putsn_P( 5*FW, 5*FH, "           ", 11 ) ;
				}

// 	lcd_outhex4( 0, 7*FH, tgtime ) ;
// 	lcd_outhex4( 25, 7*FH, get_tmr10ms() ) ;
//	uint32_t dbgVal = (uint16_t)(get_tmr10ms() - tgtime ) ;
// 	lcd_outhex4( 50, 7*FH, dbgVal >> 16 ) ;
// 	lcd_outhex4( 75, 7*FH, dbgVal ) ;

//extern uint8_t s_eeDirtyMsk ;
//	lcd_outhex4( 132, 1*FH, s_eeDirtyMsk ) ;

				refreshDisplay() ;
  		}

#ifndef REV9E				 
#ifdef PCBSKY
			if ( check_soft_power() == POWER_ON )
			{
				module_output_active() ;
				startPulses() ;		// using the required protocol
				wdt_reset() ;
				break ;	// Power back on
			}
#endif
#endif

//			if ( check_soft_power() == 0 )
//			{
				lcd_clear() ;
				lcd_putsn_P( 6*FW, 3*FH, "POWER OFF", 9 ) ;
#ifdef REV9E
extern uint8_t PowerState ;
	lcd_outhex4( 20, 0, PowerState ) ;
#endif
				refreshDisplay() ;
//			  lcdSetRefVolt(0);
				
//#ifdef REV9E
//	  		tgtime = get_tmr10ms() ;
//		  	while( (uint16_t)(get_tmr10ms() - tgtime ) < 300 ) // 50 - Half second
//  			{
//					wdt_reset() ;
//				}	
//#endif
#ifdef REV9E
extern uint8_t PowerState ;
				while ( PowerState < 4 )
				{
					wdt_reset() ;
					lcd_clear() ;
					check_soft_power() ;
					lcd_putsn_P( 6*FW, 3*FH, "POWER OFF", 9 ) ;
					refreshDisplay() ;
				}
#endif
				soft_power_off() ;		// Only turn power off if necessary
#if defined(PCBX9D) || defined(PCB9XT)
				for(;;)
				{
					wdt_reset() ;
  				PWR->CR |= PWR_CR_CWUF;
  				/* Select STANDBY mode */
  				PWR->CR |= PWR_CR_PDDS;
  				/* Set SLEEPDEEP bit of Cortex System Control Register */
  				SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  				/* Request Wait For Event */
  				__WFE();
				}
#else
				for(;;)
				{
					wdt_reset() ;
#ifndef REV9E				 
					if ( check_soft_power() == POWER_ON )
					{
						init_soft_power() ;
						module_output_active() ;
						startPulses() ;		// using the required protocol
						wdt_reset() ;
						break ;	// Power back on
					}
#endif
				}
#endif // PCBX9D

//			}
		}
#endif
//#ifdef PCBSKY
//		if ( goto_usb )
//		{
//			break ;		
//		}
//#endif
#ifdef SERIAL_HOST
		mainSequence( NO_MENU ) ;
#else
		mainSequence( MENUS ) ;
#endif
#ifndef SIMU
		CoTickDelay(1) ;					// 2mS for now
#endif
	}

#ifdef PCBSKY
	RSTC->RSTC_CR = 0xA5000000 | RSTC_CR_PROCRST | RSTC_CR_PERRST ;
//	actionUsb() ;
#endif
}

//#ifdef PCBSKY
//void actionUsb()
//{
//#if !defined(SIMU)
//	lcd_clear() ;
//	lcd_putcAtt( 48, 24, 'U', DBLSIZE ) ;
//	lcd_putcAtt( 60, 24, 'S', DBLSIZE ) ;
//	lcd_putcAtt( 72, 24, 'B', DBLSIZE ) ;
//	refreshDisplay() ;

//	// This might be replaced by a software reset
//	// Any interrupts that have been enabled must be disabled here
//	// BEFORE calling sam_boot()
//	SysTick->CTRL = 0 ;				// Turn off systick

//	stop_rotary_encoder() ;
//	endPdcUsartReceive() ;		// Terminate any serial reception
//	end_bt_tx_interrupt() ;
//	soft_power_off() ;
//	end_ppm_capture() ;
//	end_spi() ;
//	end_sound() ;
//	TC0->TC_CHANNEL[2].TC_IDR = TC_IDR0_CPCS ;
//	stop_timer0() ;
//	TC0->TC_CHANNEL[1].TC_CCR = TC_CCR0_CLKDIS ;
//	stop5msTimer() ;
//	TC0->TC_CHANNEL[2].TC_CCR = TC_CCR0_CLKDIS ;
//	TC1->TC_CHANNEL[0].TC_CCR = TC_CCR0_CLKDIS ;
//	TC1->TC_CHANNEL[1].TC_CCR = TC_CCR0_CLKDIS ;
//	TC1->TC_CHANNEL[2].TC_CCR = TC_CCR0_CLKDIS ;
//	PWM->PWM_DIS = PWM_DIS_CHID0 | PWM_DIS_CHID1 | PWM_DIS_CHID2 | PWM_DIS_CHID3 ;	// Disable all
////	PWM->PWM_IDR1 = PWM_IDR1_CHID0 ;
//	disable_main_ppm() ;
//	disable_ppm2() ;

////	PWM->PWM_IDR1 = PWM_IDR1_CHID3 ;
////	NVIC_DisableIRQ(PWM_IRQn) ;
//	disable_ssc() ;
//	UART_Stop() ;
//	Bt_UART_Stop() ;
//	sam_boot() ;
//#endif
//}
//#endif

uint16_t getTmr2MHz()
{
#ifdef PCBSKY
	return TC1->TC_CHANNEL[0].TC_CV ;
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	return TIM7->CNT ;
#endif
}

uint32_t OneSecTimer ;
uint8_t StickScrollAllowed ;
uint8_t StickScrollTimer ;

#ifdef FRSKY
extern int16_t AltOffset ;
#endif

//uint16_t Debug_analog[8] ;
static void almess( const char * s, uint8_t type )
{
	const char *h ;
  lcd_clear();
  lcd_puts_Pleft(4*FW,s);
	if ( type == ALERT_TYPE)
	{
    lcd_puts_P(64-6*FW,7*FH,"press any Key");
		h = PSTR(STR_ALERT) ;
	}
	else
	{
		h = PSTR(STR_MESSAGE) ;
	}
  lcd_putsAtt(64-7*FW,0*FH, h,DBLSIZE);
  refreshDisplay();
}

int8_t getAndSwitch( SKYCSwData &cs )
{
#if defined(PCBSKY) || defined(PCB9XT)
	int8_t x = 0 ;
	if ( cs.andsw )	// Code repeated later, could be a function
	{
		x = cs.andsw ;
		if ( ( x > 8 ) && ( x <= 9+NUM_SKYCSW ) )
		{
			x += 1 ;
		}
		if ( ( x < -8 ) && ( x >= -(9+NUM_SKYCSW) ) )
		{
			x -= 1 ;
		}
		if ( x == 9+NUM_SKYCSW+1 )
		{
			x = 9 ;			// Tag TRN on the end, keep EEPROM values
		}
		if ( x == -(9+NUM_SKYCSW+1) )
		{
			x = -9 ;			// Tag TRN on the end, keep EEPROM values
		}
	}
	return x ;
#endif
#ifdef PCBX9D
	return cs.andsw ;
#endif
}

void doVoiceAlarmSource( VoiceAlarmData *pvad )
{
	if ( pvad->source )
	{
		// SORT OTHER values here
		if ( pvad->source >= NUM_XCHNRAW )
		{
			voice_telem_item( pvad->source - NUM_SKYXCHNRAW - 1 ) ;
		}
		else
		{
			int16_t value ;
			value = getValue( pvad->source - 1 ) ;
			voice_numeric( value, 0, 0 ) ;
		}
	}
}


static void processVoiceAlarms()
{
	uint32_t i ;
	uint32_t curent_state ;
	VoiceAlarmData *pvad = &g_model.vad[0] ;
	for ( i = 0 ; i < NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ; i += 1 )
	{
		uint32_t play = 0 ;
		curent_state = 0 ;
		int16_t ltimer = Nvs_timer[i] ;
	 	if ( i == NUM_VOICE_ALARMS )
		{
			pvad = &g_model.vadx[0] ;
		}
		if ( pvad->func )		// Configured
		{
  		int16_t x ;
			int16_t y = pvad->offset ;
			x = getValue( pvad->source - 1 ) ;
  		switch (pvad->func)
			{
				case 1 :
					x = x > y ;
				break ;
				case 2 :
					x = x < y ;
				break ;
				case 3 :
					x = abs(x) > y ;
				break ;
				case 4 :
					x = abs(x) < y ;
				break ;
				case 5 :
				{
					if ( isAgvar( pvad->source ) )
					{
						x *= 10 ;
						y *= 10 ;
					}
    			x = abs(x-y) < 32 ;
				}
				break ;
				case 6 :
					x = x == y ;
				break ;
			}
// Start of invalid telemetry detection
//					if ( pvad->source > ( CHOUT_BASE - NUM_SKYCHNOUT ) )
//					{ // Telemetry item
//						if ( !telemItemValid( pvad->source - 1 - CHOUT_BASE - NUM_SKYCHNOUT ) )
//						{
//							x = 0 ;	// Treat as OFF
//						}
//					}
// End of invalid telemetry detection
			if ( pvad->swtch )
			{
				if ( getSwitch00( pvad->swtch ) == 0 )
				{
					x = 0 ;
				}
			}
			if ( x == 0 )
			{
				ltimer = 0 ;
			}
			else
			{
				play = 1 ;
			}
		}
		else // No function
		{
			if ( pvad->swtch )
			{
				curent_state = getSwitch00( pvad->swtch ) ;
				if ( curent_state == 0 )
				{
//							Nvs_state[i] = 0 ;
					ltimer = -1 ;
				}
			}
			else// No switch, no function
			{ // Check for source with numeric rate
				if ( pvad->rate >= 4 )	// A time
				{
					if ( pvad->vsource )
					{
						play = 1 ;
					}
				}
			}
		}
		play |= curent_state ;

		if ( ( VoiceCheckFlag & 2 ) == 0 )
		{
		 if ( pvad->rate == 3 )	// All
		 {
		 		uint32_t pos = switchPosition( pvad->swtch ) ;
				uint32_t state = Nvs_state[i] ;
				play = 0 ;
				if ( state != pos )
				{
					if ( state > 0x80 )
					{
						if ( --state == 0x80 )
						{
							state = pos ;
							ltimer = 0 ;
							play = pos + 1 ;
						}
					}
					else
					{
						state = 0x83 ;
					}
					Nvs_state[i] = state ;
				}
//				else
//				{
//					play = 0 ;
//				}
		 }
		 else
		 {
			if ( play == 1 )
			{
				if ( Nvs_state[i] == 0 )
				{ // just turned ON
					if ( ( pvad->rate == 0 ) || ( pvad->rate == 2 ) )
					{ // ON
						ltimer = 0 ;
					}
				}
				Nvs_state[i] = 1 ;
				if ( ( pvad->rate == 1 ) )
				{
					play = 0 ;
				}
			}
			else
			{
				if ( Nvs_state[i] == 1 )
				{
					if ( ( pvad->rate == 1 ) || ( pvad->rate == 2 ) )
					{
						ltimer = 0 ;
						play = 1 ;
						if ( pvad->rate == 2 )
						{
							play = 2 ;
						}
					}
				}
				Nvs_state[i] = 0 ;
			}
			if ( pvad->rate == 33 )
			{
				play = 0 ;
				ltimer = -1 ;
			}
		 }
		}
		else
		{
			Nvs_state[i] = ( pvad->rate == 3 ) ? switchPosition( pvad->swtch ) : play ;
			play = ( pvad->rate == 33 ) ? 1 : 0 ;
			ltimer = -1 ;
		}

		if ( pvad->mute )
		{
			if ( pvad->source > ( CHOUT_BASE - NUM_SKYCHNOUT ) )
			{ // Telemetry item
				if ( !telemItemValid( pvad->source - 1 - CHOUT_BASE - NUM_SKYCHNOUT ) )
				{
					play = 0 ;	// Mute it
				}
			}
		}

		if ( play )
		{
			if ( ltimer < 0 )
			{
				if ( pvad->rate >= 3 )	// A time or ONCE
				{
					ltimer = 0 ;
				}
			}
			if ( ltimer == 0 )
			{
				if ( pvad->vsource == 1 )
				{
					doVoiceAlarmSource( pvad ) ;
//					if ( pvad->source )
//					{
//						// SORT OTHER values here
//						if ( pvad->source >= NUM_SKYXCHNRAW )
//						{
//							voice_telem_item( pvad->source - NUM_SKYXCHNRAW - 1 ) ;
//						}
//						else
//						{
//							if ( pvad->source )
//							{
//								int16_t value ;
//								value = getValue( pvad->source - 1 ) ;
//								voice_numeric( value, 0, 0 ) ;
//							}
//						}
//					}
				}
				if ( pvad->fnameType == 0 )	// None
				{
					// Nothing!
				}
				else if ( pvad->fnameType == 1 )	// Name
				{
					char name[10] ;
					char *p ;
					p = (char *)ncpystr( (uint8_t *)name, pvad->file.name, 8 ) ;
					if ( play >= 2 )
					{
						*(p-1) += ( play - 1 ) ;
					}
					putUserVoice( name, 0 ) ;
				}
				else if ( pvad->fnameType == 2 )	// Number
				{
					uint16_t value = pvad->file.vfile ;
					if ( value > 507 )
					{
						value = calc_scaler( value-508, 0, 0 ) ;
					}
					else if ( value > 500 )
					{
						value = g_model.gvars[value-501].gvar ;
					}
					putVoiceQueue( ( value + ( play - 1 ) ) | VLOC_NUMUSER ) ;
				}
				else
				{ // Audio
					audio.event( pvad->file.vfile, 0, 1 ) ;
				}
				if ( pvad->vsource == 2 )
				{
					doVoiceAlarmSource( pvad ) ;
//					if ( pvad->source )
//					{
//						// SORT OTHER values here
//						if ( pvad->source >= NUM_SKYXCHNRAW )
//						{
//							voice_telem_item( pvad->source - NUM_SKYXCHNRAW - 1 ) ;
//						}
//						else
//						{
//							if ( pvad->source )
//							{
//								int16_t value ;
//								value = getValue( pvad->source - 1 ) ;
//								voice_numeric( value, 0, 0 ) ;
//							}
//						}
//					}
				}
        if ( pvad->haptic )
				{
					audioDefevent( (pvad->haptic > 1) ? ( ( pvad->haptic == 3 ) ? AU_HAPTIC3 : AU_HAPTIC2 ) : AU_HAPTIC1 ) ;
				}
				if ( ( pvad->rate < 4 ) || ( pvad->rate > 32 ) )	// Not a time
				{
					ltimer = -1 ;
				}
				else
				{
					ltimer = 1 ;
				}
			}
			else if ( ltimer > 0 )
			{
				ltimer += 1 ;
				if ( ltimer > ( (pvad->rate-2) * 10 ) )
				{
					ltimer = 0 ;
				}
			}
		}
		pvad += 1 ;
		Nvs_timer[i] = ltimer ;
	}
}


uint32_t MixerRate ;
uint32_t MixerCount ;

uint8_t AlarmTimers[NUM_SKYCHNOUT] ;

uint8_t EncoderI2cData[2] ;

void mainSequence( uint32_t no_menu )
{
	CalcScaleNest = 0 ;

#ifdef PCBSKY
	static uint32_t coProTimer = 0 ;
#endif
#ifdef PCB9XT
	static uint32_t EncoderTimer = 0 ;
#endif
  uint16_t t0 = getTmr2MHz();
	CPU_UINT numSafety = NUM_SKYCHNOUT - g_model.numVoice ;
	
	if ( g_eeGeneral.filterInput == 1 )
	{
		getADC_osmp() ;
	}
	else if ( g_eeGeneral.filterInput == 2 )
	{
		getADC_filt() ;
	}
	else
	{
		getADC_single() ;
	}
#ifdef PCBSKY
	if ( g_eeGeneral.ar9xBoard == 0 )
	{

		uint16_t temp ;
		temp = AnalogData[13] ;
		temp = temp - Current_adjust / temp ;
		{
			uint16_t min_current ;
			min_current = 21000 / g_vbat100mV + 155 ;
			temp = temp * (127 + g_eeGeneral.current_calib) * 25 / 4096 ;
			if ( temp < min_current )
			{
				temp = min_current ;
				if ( Current_adjust > 128 )
				{
					Current_adjust -= 128 ;
				}
				else
				{
					Current_adjust = 0 ;
				}
			}
		}
		Current_current = ( Current_current * 3 + temp + 2 ) >> 2 ;

		if ( Current_current > Current_max )
		{
			Current_max = Current_current ;
		}
	}
#endif

#ifdef PCB9XT
	checkM64() ;
#endif
	perMain( no_menu ) ;		// Allow menu processing
	if(heartbeat == 0x3)
	{
    wdt_reset();
    heartbeat = 0;
  }
//#ifdef REV9E
//  wdt_reset() ;
//#endif

#ifdef PCBX9D
	checkTrainerSource() ;
	if ( ( g_model.com2Function == COM2_FUNC_SBUSTRAIN ) || ( g_model.com2Function == COM2_FUNC_SBUS57600 ) || ( CurrentTrainerSource == 1 ) )
#endif
#ifdef PCBSKY
	if ( ( g_model.com2Function == COM2_FUNC_SBUSTRAIN ) || ( g_model.com2Function == COM2_FUNC_SBUS57600 ) )
#endif
	{
		processSbusInput() ;
	}



	if ( Tenms )
	{
		
		Tenms = 0 ;
#if defined(PCBSKY) || defined(PCB9XT)
		ee32_process() ;
#endif
#ifdef PCBSKY
		if ( g_eeGeneral.ar9xBoard == 0 )
		{
			Current_accumulator += Current_current ;
		}
#endif
#ifdef PCBX9D
		eePoll() ;
#endif
#ifdef PCB9XT
		m64_10mS() ;
#endif
#if defined(PCB9XT) || defined(PCBX9D)
		handleUsbConnection() ;
#endif

		if ( ++OneSecTimer >= 100 )
		{
			OneSecTimer -= 100 ;
#ifdef PCBSKY
			if ( g_eeGeneral.ar9xBoard == 0 )
			{
				Current_used += Current_accumulator / 100 ;			// milliAmpSeconds (but scaled)
				Current_accumulator = 0 ;
			}
#endif

#if defined(PCBSKY) || defined(PCB9XT)
extern void btCountersToTotals() ;
			btCountersToTotals() ;
#endif

			if ( StickScrollTimer )
			{
				StickScrollTimer -= 1 ;				
			}
			MixerRate = MixerCount ;
			MixerCount = 0 ;
		}
#ifndef SIMU
		sdPoll10mS() ;
#endif

 #ifdef PCBSKY
		if ( ++coProTimer > 9 )
		{
			coProTimer -= 10 ;
			
	 		if ( g_eeGeneral.ar9xBoard )
			{
				// Read external RTC here
extern void readExtRtc() ;
				readExtRtc() ;
			}	
			else
			{
#ifndef REVX
				if ( CoProcAlerted == 0 )
				{
					if ( Coproc_valid == 1 )
					{
						if ( (Coproc_read & 0x80) == 0 )
						{
							if ( Coproc_read < 6 )
							{
    	    			alert( "Update Co-Processor" ) ;
							}
							CoProcAlerted = 1 ;
						}
					}
				}
				read_coprocessor() ;
#else
				readRTC() ;
//			readI2cEncoder( EncoderI2cData ) ;
#endif
			}
		}

	 	if ( g_eeGeneral.ar9xBoard )
		{
extern void pollForRtcComplete() ;
			pollForRtcComplete() ;
		}

#endif

#ifdef PCB9XT
		if ( g_eeGeneral.enableI2C == 1 )
		{
			if ( ++EncoderTimer > 4 )
			{
				EncoderTimer -= 5 ;
	//			if ( readI2cEncoder( EncoderI2cData ) )
				hwreadI2cEncoder( EncoderI2cData ) ;
			}
extern uint32_t i2c2_result() ;

			if ( EncoderTimer == 1 )
			{
				if ( i2c2_result() == 1 )
				{
					static uint8_t lastPosition = 0 ;
					if ( lastPosition != EncoderI2cData[0] )
					{
						int8_t diff = EncoderI2cData[0] - lastPosition ;
	//					{
						
	//						int32_t x = diff ;
	//						if ( g_eeGeneral.rotaryDivisor == 1)
	//						{
	//							x = diff >> 2 ;
	//						}
	//						else if ( g_eeGeneral.rotaryDivisor == 2)
	//						{
	//							x = diff >> 1 ;
	//						}
	//						else
	//						{
	//							x = diff ;
	//						}
	//						diff = x ;
	//					}
						if ( diff < 9 && diff > -9 )
						{
							Rotary_count += diff ;
						}
						lastPosition = EncoderI2cData[0] ;
					}
				
				}
			}
		}
#endif

#if defined(PCBX9D) || defined(PCB9XT)
	rtc_gettime( &Time ) ;
#endif
	}
	processAdjusters() ;

	t0 = getTmr2MHz() - t0;
  if ( t0 > g_timeMain ) g_timeMain = t0 ;
  if ( AlarmCheckFlag > 1 )		// Every 1 second
  {
    AlarmCheckFlag = 0 ;
    // Check for alarms here
    // Including Altitude limit

#ifdef FRSKY
    if (frskyUsrStreaming)
    {
      int16_t limit = g_model.FrSkyAltAlarm ;
      int16_t altitude ;
      if ( limit )
      {
        if (limit == 2)  // 400
        {
          limit = 400 ;	//ft
        }
        else
        {
          limit = 122 ;	//m
        }
				altitude = FrskyHubData[FR_ALT_BARO] + AltOffset ;
				altitude /= 10 ;									
				if (g_model.FrSkyUsrProto == 0)  // Hub
				{
      		if ( g_model.FrSkyImperial )
					{
        		altitude = m_to_ft( altitude ) ;
					}
				}
        if ( altitude > limit )
        {
          audioDefevent(AU_WARNING2) ;
        }
      }
			uint16_t total_volts = 0 ;
			CPU_UINT audio_sounded = 0 ;
			uint8_t low_cell = 220 ;		// 4.4V
			uint8_t totalCells = FrskyBattCells[0] + FrskyBattCells[1] ;
		  for (uint8_t k=0 ; k<totalCells ; k++)
			{
				total_volts += FrskyVolts[k] ;
				if ( FrskyVolts[k] < low_cell )
				{
					low_cell = FrskyVolts[k] ;
				}

				if ( audio_sounded == 0 )
				{
		      if ( FrskyVolts[k] < g_model.frSkyVoltThreshold )
					{
		        audioDefevent(AU_WARNING3);
						audio_sounded = 1 ;
					}
			  }
	  	}
			// Now we have total volts available
			FrskyHubData[FR_CELLS_TOT] = total_volts / 5 ;
			if ( low_cell < 220 )
			{
				FrskyHubData[FR_CELL_MIN] = low_cell ;
			}
    }


    // this var prevents and alarm sounding if an earlier alarm is already sounding
    // firing two alarms at once is pointless and sounds rubbish!
    // this also means channel A alarms always over ride same level alarms on channel B
    // up to debate if this is correct!
    //				bool AlarmRaisedAlready = false;

    if (frskyStreaming)
		{
      enum AlarmLevel level[4] ;
      // RED ALERTS
      if( (level[0]=FRSKY_alarmRaised(0,0)) == alarm_red) FRSKY_alarmPlay(0,0);
      else if( (level[1]=FRSKY_alarmRaised(0,1)) == alarm_red) FRSKY_alarmPlay(0,1);
      else	if( (level[2]=FRSKY_alarmRaised(1,0)) == alarm_red) FRSKY_alarmPlay(1,0);
      else if( (level[3]=FRSKY_alarmRaised(1,1)) == alarm_red) FRSKY_alarmPlay(1,1);
      // ORANGE ALERTS
      else	if( level[0] == alarm_orange) FRSKY_alarmPlay(0,0);
      else if( level[1] == alarm_orange) FRSKY_alarmPlay(0,1);
      else	if( level[2] == alarm_orange) FRSKY_alarmPlay(1,0);
      else if( level[3] == alarm_orange) FRSKY_alarmPlay(1,1);
      // YELLOW ALERTS
      else	if( level[0] == alarm_yellow) FRSKY_alarmPlay(0,0);
      else if( level[1] == alarm_yellow) FRSKY_alarmPlay(0,1);
      else	if( level[2] == alarm_yellow) FRSKY_alarmPlay(1,0);
      else if( level[3] == alarm_yellow) FRSKY_alarmPlay(1,1);
						
			// Check for current alarm
			if ( g_model.currentSource )
			{
				if ( g_model.frskyAlarms.alarmData[0].frskyAlarmLimit )
				{
					if ( ( FrskyHubData[FR_AMP_MAH] >> 6 ) >= g_model.frskyAlarms.alarmData[0].frskyAlarmLimit )
					{
						putSystemVoice( SV_CAP_WARN, V_CAPACITY ) ;
					}
					uint32_t value ;
					value = g_model.frskyAlarms.alarmData[0].frskyAlarmLimit ;
					value <<= 6 ;
					value = 100 - ( FrskyHubData[FR_AMP_MAH] * 100 / value ) ;
					FrskyHubData[FR_FUEL] = value ;
				}
			}
    }
#endif
		
		// Now for the Safety/alarm switch alarms
		{
			uint8_t i ;
			static uint8_t periodCounter ;
 			
			periodCounter += 0x11 ;
			periodCounter &= 0xF7 ;
			if ( periodCounter > 0x5F )
			{
				periodCounter &= 0x0F ;
			}

			for ( i = 0 ; i < numSafety ; i += 1 )
			{
    		SKYSafetySwData *sd = &g_model.safetySw[i] ;
				if (sd->opt.ss.mode == 1)
				{
					if ( AlarmTimers[i] == 0 )
					{
						if(getSwitch00( sd->opt.ss.swtch))
						{
							audio.event( /*((g_eeGeneral.speakerMode & 1) == 0) ? 1 :*/ sd->opt.ss.val ) ;
							AlarmTimers[i] = 40 ;
						}
					}
				}
				if (sd->opt.ss.mode == 2)
				{
					if ( sd->opt.ss.swtch > MAX_SKYDRSWITCH )
					{
						switch ( sd->opt.ss.swtch - MAX_SKYDRSWITCH -1 )
						{
							case 0 :
								if ( ( periodCounter & 3 ) == 0 )
								{
									voice_telem_item( sd->opt.ss.val ) ;
								}
							break ;
							case 1 :
								if ( ( periodCounter & 0xF0 ) == 0 )
								{
									voice_telem_item( sd->opt.ss.val ) ;
								}
							break ;
							case 2 :
								if ( ( periodCounter & 7 ) == 2 )
								{
									voice_telem_item( sd->opt.ss.val ) ;
								}
							break ;
						}
					}
					else if ( ( periodCounter & 3 ) == 0 )		// Every 4 seconds
					{
						if(getSwitch00( sd->opt.ss.swtch))
						{
							putVoiceQueue( ( sd->opt.ss.val + 128 ) | VLOC_NUMUSER ) ;
						}
					}
				}
			}
		}
#ifdef REV9E
void updateTopLCD( uint32_t time, uint32_t batteryState ) ;
void setTopRssi( uint32_t rssi ) ;
void setTopVoltage( uint32_t volts ) ;
void setTopOpTime( uint32_t hours, uint32_t mins, uint32_t secs ) ;

static uint8_t RssiTestCount ;
		setTopRssi( RssiTestCount ) ;
		setTopVoltage( RssiTestCount ) ;
		setTopOpTime( RssiTestCount%100, RssiTestCount%100, RssiTestCount%100 ) ;
		if ( ++RssiTestCount > 149 )
		{
			RssiTestCount = 0 ;
		}
		updateTopLCD( Time.hour*60+Time.minute, RssiTestCount%6 ) ;
#endif
  }
	// New switch voices
	// New entries, Switch, (on/off/both), voice file index

	if ( VoiceCheckFlag )	// Every 100mS
  {
		uint32_t i ;
		static uint32_t timer ;
		static uint32_t delayTimer = 0 ;
    
		if ( VoiceCheckFlag & 1 )
		{
//#ifdef TELEMETRY_LOST
//			if ( TelemetryStatus && ( frskyStreaming == 0 ) )
//			{ // We have lost telemetry
//				putSystemVoice( SV_NO_TELEM, V_NOTELEM ) ;
//			}
//			TelemetryStatus = frskyStreaming ;
//#endif
			timer += 1 ;
			if ( delayTimer )
			{
				delayTimer -= 1 ;
			}

			uint8_t redAlert = 0 ;
			static uint8_t redCounter ;
			static uint8_t orangeCounter ;
			uint8_t rssiValue = FrskyHubData[FR_RXRSI_COPY] ;

			if ( frskyStreaming )
			{
				if ( g_model.enRssiRed == 0 )
				{
#ifdef ASSAN
					int8_t offset ;
#ifdef PCBX9D
					if ( ( ( g_model.xprotocol == PROTO_DSM2) && ( g_model.xsub_protocol == DSM_9XR ) ) || (g_model.xprotocol == PROTO_ASSAN) )
#else
					if ( ( ( g_model.protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) ) || (g_model.protocol == PROTO_ASSAN) )
#endif // PCBX9D
					{
						offset = 18 ;
					}
					else
					{
						offset = 42 ;
					}
					if ( rssiValue && rssiValue < g_model.rssiRed + offset )
#else // ASSAN
					if ( rssiValue && rssiValue < g_model.rssiRed + 42 )
#endif // ASSAN
					{
						// Alarm
						redAlert = 1 ;
						orangeCounter += 1 ;
						if ( ++redCounter > 3 )
						{
							if ( delayTimer == 0 )
							{
								putSystemVoice( SV_RSSICRIT, V_RSSI_CRITICAL ) ;
								delayTimer = 40 ;	// 4 seconds
							}
							redCounter = 0 ;
						}
					}
					else
					{
						redCounter = 0 ;
					}
				}
				if ( ( redAlert == 0 ) && ( g_model.enRssiOrange == 0 ) )
				{
#ifdef ASSAN
					int8_t offset ;
#ifdef PCBX9D
					if ( ( ( g_model.xprotocol == PROTO_DSM2) && ( g_model.xsub_protocol == DSM_9XR ) ) || (g_model.xprotocol == PROTO_ASSAN) )
#else
					if ( ( ( g_model.protocol == PROTO_DSM2) && ( g_model.sub_protocol == DSM_9XR ) ) || (g_model.protocol == PROTO_ASSAN) )
#endif // PCBX9D
					{
						offset = 20 ;
					}
					else
					{
						offset = 45 ;
					}
					if ( rssiValue && rssiValue < g_model.rssiOrange + offset )
#else // ASSAN
					if ( rssiValue && rssiValue < g_model.rssiOrange + 45 )
#endif // ASSAN
					{
						// Alarm
						if ( ++orangeCounter > 3 )
						{
							if ( delayTimer == 0 )
							{
								putSystemVoice( SV_RSSI_LOW, V_RSSI_WARN ) ;
								delayTimer = 40 ;	// 4 seconds
							}
							orangeCounter = 0 ;
						}
					}
					else
					{
						orangeCounter = 0 ;
					}
				}
			}

			for ( i = 0 ; i < numSafety ; i += 1 )
			{
				if ( AlarmTimers[i] )
				{
					AlarmTimers[i] -= 1 ;
				}
    		SKYSafetySwData *sd = &g_model.safetySw[i] ;
				if (sd->opt.ss.mode == 2)
				{
					if ( sd->opt.ss.swtch <= MAX_SKYDRSWITCH )
					{
						if ( AlarmTimers[i] == 0 )
						{
							if(getSwitch00( sd->opt.ss.swtch))
							{
								putVoiceQueue( ( sd->opt.ss.val + 128 ) | VLOC_NUMUSER ) ;
								AlarmTimers[i] = 40 ;		// 4 seconds
							}
						}
			    }
				}
			}
		}
		 
		for ( i = numSafety ; i < NUM_SKYCHNOUT+NUM_VOICE ; i += 1 )
		{
			uint8_t curent_state ;
			uint8_t mode ;
			uint8_t value ;
    	SKYSafetySwData *sd = &g_model.safetySw[i];
    	if ( i >= NUM_SKYCHNOUT )
			{
				sd = (SKYSafetySwData*) &g_model.voiceSwitches[i-NUM_SKYCHNOUT];
			}
			
			mode = sd->opt.vs.vmode ;
			value = sd->opt.vs.vval ;
			if ( mode <= 5 )
			{
				if ( value > 250 )
				{
					value = g_model.gvars[value-248].gvar ; //Gvars 3-7
				}
			}
			
			if ( sd->opt.vs.vswtch )		// Configured
			{
				curent_state = getSwitch00( sd->opt.vs.vswtch ) ;
				if ( ( VoiceCheckFlag & 2 ) == 0 )
				{
					if ( ( mode == 0 ) || ( mode == 2 ) )
					{ // ON
						if ( ( Vs_state[i] == 0 ) && curent_state )
						{
							putVoiceQueue( ( sd->opt.vs.vval ) | VLOC_NUMUSER  ) ;
						}
					}
					if ( ( mode == 1 ) || ( mode == 2 ) )
					{ // OFF
						if ( ( Vs_state[i] == 1 ) && !curent_state )
						{
							uint8_t x ;
							x = sd->opt.vs.vval ;
							if ( mode == 2 )
							{
								x += 1 ;							
							}
							putVoiceQueue( x | VLOC_NUMUSER  ) ;
						}
					}
					if ( mode > 5 )
					{
						if ( ( Vs_state[i] == 0 ) && curent_state )
						{
							voice_telem_item( sd->opt.vs.vval ) ;
						}					
					}
					else if ( mode > 2 )
					{ // 15, 30 or 60 secs
						if ( curent_state )
						{
							uint16_t mask ;
							mask = 150 ;
							if ( mode == 4 ) mask = 300 ;
							if ( mode == 5 ) mask = 600 ;
							if ( timer % mask == 0 )
							{
								putVoiceQueue( sd->opt.vs.vval | VLOC_NUMUSER  ) ;
							}
						}
					}
				}
				Vs_state[i] = curent_state ;
			}
		}

		{
			uint8_t i ;
		
			for ( i = 0 ; i < NUM_SKYCSW ; i += 1 )
			{
  	  	SKYCSwData &cs = g_model.customSw[i];
  	  	uint8_t cstate = CS_STATE(cs.func);

  	  	if(cstate == CS_TIMER)
				{
					int16_t y ;
					y = CsTimer[i] ;
					if ( y == 0 )
					{
						int8_t z ;
						z = cs.v1 ;
						if ( z >= 0 )
						{
							z = -z-1 ;
							y = z * 10 ;					
						}
						else
						{
							y = z ;
						}
					}
					else if ( y < 0 )
					{
						if ( ++y == 0 )
						{
							int8_t z ;
							z = cs.v2 ;
							if ( z >= 0 )
							{
								z += 1 ;
								y = z * 10 - 1 ;
							}
							else
							{
								y = -z-1 ;
							}
						}
					}
					else  // if ( CsTimer[i] > 0 )
					{
						y -= 1 ;
					}

					int8_t x = getAndSwitch( cs ) ;
					if ( x )
					{
		        if (getSwitch00( x) == 0 )
					  {
							Last_switch[i] = 0 ;
							if ( cs.func == CS_NTIME )
							{
								int8_t z ;
								z = cs.v1 ;
								if ( z >= 0 )
								{
									z = -z-1 ;
									y = z * 10 ;					
								}
								else
								{
									y = z ;
								}
							}
							else
							{
								y = -1 ;
							}
						}
						else
						{
							Last_switch[i] = 2 ;
						}
					}
					CsTimer[i] = y ;
				}
				if ( cs.func == CS_LATCH )
				{
		      if (getSwitch00( cs.v1) )
					{
						Last_switch[i] = 1 ;
					}
					else
					{
			      if (getSwitch00( cs.v2) )
						{
							Last_switch[i] = 0 ;
						}
					}
				}
				if ( cs.func == CS_FLIP )
				{
		      if (getSwitch00( cs.v1) )
					{
						if ( ( Last_switch[i] & 2 ) == 0 )
						{
							// Clock it!
			      	if (getSwitch00( cs.v2) )
							{
								Last_switch[i] = 3 ;
							}
							else
							{
								Last_switch[i] = 2 ;
							}
						}
					}
					else
					{
						Last_switch[i] &= ~2 ;
					}
				}
				if ( ( cs.func == CS_MONO ) || ( cs.func == CS_RMONO ) )
				{
					if ( VoiceCheckFlag & 2 )
					{
						// Resetting, retrigger any monostables
						Last_switch[i] &= ~2 ;
					}
					int8_t andSwOn = 1 ;
					if ( ( cs.func == CS_RMONO ) )
					{
						andSwOn = getAndSwitch( cs ) ;
						if ( andSwOn )
						{
							andSwOn = getSwitch00( andSwOn) ;
						}
						else
						{
							andSwOn = 1 ;
						}
					}
					
		      if (getSwitch00( cs.v1) )
					{
						if ( ( Last_switch[i] & 2 ) == 0 )
						{
							// Trigger monostable
							uint8_t trigger = 1 ;
							if ( ( cs.func == CS_RMONO ) )
							{
								if ( ! andSwOn )
								{
									trigger = 0 ;
								}
							}
							if ( trigger )
							{
								Last_switch[i] = 3 ;
								int16_t x ;
								x = cs.v2 ;
								if ( x < 0 )
								{
									x = -x ;
								}
								else
								{
									x += 1 ;
									x *= 10 ;
								}
								CsTimer[i] = x ;							
							}
					  }
					}
					else
					{
						Last_switch[i] &= ~2 ;
					}
					int16_t y ;
					y = CsTimer[i] ;
					if ( y )
					{
						if ( ( cs.func == CS_RMONO ) )
						{
							if ( ! andSwOn )
							{
								y = 1 ;
							}	
						}
						if ( --y == 0 )
						{
							Last_switch[i] &= ~1 ;
						}
						CsTimer[i] = y ;
					}
				}
			}
		}

		// Now also check for resetting timers
		// Now done in timer()
//		if ( g_model.timer1RstSw )
//		{
//			if ( getSwitch( g_model.timer1RstSw, 0, 0) )
//			{
//				resetTimer1() ;
//			}
//		}
		
//		if ( g_model.timer2RstSw )
//		{
//			if ( getSwitch( g_model.timer2RstSw, 0, 0) )
//			{
//				resetTimer2() ;
//			}
//		}

//extern VoiceAlarmData vad[NUM_VOICE_ALARMS] ;	// Temporary for RAM based testing
		// Now test for the new voice alarms
		processVoiceAlarms() ;

		VoiceCheckFlag = 0 ;

		// Vario
		{

			static uint8_t varioRepeatRate = 0 ;

			if ( g_model.varioData.varioSource ) // Vario enabled
			{
				if ( getSwitch00( g_model.varioData.swtch ) )
				{
					uint8_t new_rate = 0 ;
					if ( varioRepeatRate )
					{
						varioRepeatRate -= 1 ;
					}
					if ( varioRepeatRate == 0 )
					{
						int16_t vspd ;
						if ( g_model.varioData.varioSource == 1 )
						{
							vspd = FrskyHubData[FR_VSPD] ;

							if ( g_model.varioData.param > 1 )
							{
								vspd /= g_model.varioData.param ;
							}
						}
						else if ( g_model.varioData.varioSource == 2 )
						{
							vspd = FrskyHubData[FR_A2_COPY] - 128 ;
							if ( ( vspd < 3 ) && ( vspd > -3 ) )
							{
								vspd = 0 ;							
							}
							vspd *= g_model.varioData.param ;
						}
						else
						{
							// A Scaler
							vspd = calc_scaler( g_model.varioData.varioSource-3, 0, 0 ) ;
							if ( g_model.varioData.param > 1 )
							{
								vspd /= g_model.varioData.param ;
							}
						}
						if ( vspd )
						{
							{
								if ( vspd < 0 )
								{
									vspd = -vspd ;
									if (!g_model.varioData.sinkTones )
									{
										if ( vspd > 25 )		// OpenXsensor
										{
  	       		    		audio.event( AU_VARIO_DOWN, vspd/15 ) ;
										}
									}
								}
								else
								{
									if ( vspd > 25 )			// OpenXsensor
									{
	  	        		  audio.event( AU_VARIO_UP, vspd/15 ) ;
									}
								}
								if ( vspd < 75 )
								{
									new_rate = 8 ;
								}
								else if ( vspd < 125 )
								{
									new_rate = 6 ;
								}
								else if ( vspd < 175 )
								{
									new_rate = 4 ;
								}
								else
								{
									new_rate = 2 ;
								}
							}
						}
						else
						{
							if (g_model.varioData.sinkTones )
							{
								new_rate = 20 ;
  	      		  audio.event( AU_VARIO_UP ) ;
							}
						}
						varioRepeatRate = new_rate ;
					}
				}
			}
		}	
	}

	if ( DsmCheckFlag )
	{
		DsmCheckFlag = 0 ;
		static uint8_t criticalCounter = 0 ;
		static uint8_t warningCounter = 0 ;

		// Check DSM telemetry for fades/frame losses here
		uint32_t fades ;
		fades = DsmABLRFH[0] + DsmABLRFH[1] + DsmABLRFH[2] + DsmABLRFH[3] ;
		uint32_t critical = 0 ;
		
		if ( g_model.dsmLinkData.levelCritical )
		{
			switch ( g_model.dsmLinkData.sourceCritical )
			{
				case 0 :	// fades
					if ( ( fades - LastDsmfades ) >= g_model.dsmLinkData.levelCritical )
					{
						critical = 1 ;
					}
				break ;
				case 1 :	// losses
					if ( ( DsmABLRFH[4] - LastDsmFH[0] ) >= g_model.dsmLinkData.levelCritical )
					{
						critical = 1 ;
					}
				break ;
				case 2 :	// holdss
					if ( ( DsmABLRFH[5] - LastDsmFH[1] ) >= g_model.dsmLinkData.levelCritical )
					{
						critical = 1 ;
					}
				break ;
			}
		}
		if ( critical == 0 )
		{
			if ( g_model.dsmLinkData.levelWarn )
			{
				uint32_t warning = 0 ;
				switch ( g_model.dsmLinkData.sourceWarn )
				{
					case 0 :	// fades
						if ( ( fades - LastDsmfades ) >= g_model.dsmLinkData.levelWarn )
						{
							warning = 1 ;
						}
					break ;
					case 1 :	// losses
						if ( ( DsmABLRFH[4] - LastDsmFH[0] ) >= g_model.dsmLinkData.levelWarn )
						{
							warning = 1 ;
						}
					break ;
					case 2 :	// holdss
						if ( ( DsmABLRFH[5] - LastDsmFH[1] ) >= g_model.dsmLinkData.levelWarn )
						{
							warning = 1 ;
						}
					break ;
				}
				if ( warning )
				{
					if ( warningCounter == 0 )
					{
						putSystemVoice( SV_RSSI_LOW, V_RSSI_WARN ) ;
						warningCounter = 11 ;
					}
				}
			}
		}
		else
		{
			if ( criticalCounter == 0 )
			{
				putSystemVoice( SV_RSSICRIT, V_RSSI_CRITICAL ) ;
				criticalCounter = 11 ;
			}
		}
		LastDsmfades = fades ;
		LastDsmFH[0] = DsmABLRFH[4] ;
		LastDsmFH[1] = DsmABLRFH[5] ;
		if ( criticalCounter )
		{
			criticalCounter -= 1 ;			
		}
		if ( warningCounter )
		{
			warningCounter -= 1 ;
		}
	}
}

uint32_t check_power_or_usb()
{
#ifndef SIMU
#ifdef REV9E
		uint32_t powerState = check_soft_power() ;
		if ( ( powerState == POWER_X9E_STOP ) || ( powerState == POWER_OFF ) )		// power now off
#else
	if ( check_soft_power() == POWER_OFF )		// power now off
#endif
	{
		return 1 ;
	}
// #ifdef PCBSKY
//	if ( PIOC->PIO_PDSR & 0x02000000 )
//	{
//		return 1 ;			// Detected USB
//	}
// #endif
#endif
	return 0 ;
}

void check_backlight()
{
//#ifdef REV9E
//		BACKLIGHT_ON ;
//#else
  if(getSwitch00(g_eeGeneral.lightSw) || getSwitch00(g_model.mlightSw ) || g_LightOffCounter)
	{
		BACKLIGHT_ON ;
	}
  else
	{
    BACKLIGHT_OFF ;
	}
//#endif
}

#ifdef SPLASH_DEBUG
uint16_t SplashDebug[9] ;
#endif

uint16_t stickMoveValue()
{
    uint16_t sum = 0 ;
		uint8_t i ;
    for( i=0; i<4; i++)
		{
#ifdef SPLASH_DEBUG
			SplashDebug[i] = anaIn(i) ;
#endif
      sum += anaIn(i) ;
		}
	return sum ;
}

static uint32_t hasStickMoved( uint16_t value )
{
  if(abs(int16_t( value-stickMoveValue()))>160)
		return 1 ;
	return 0 ;
}


void doSplash()
{
	uint32_t i ;
	uint32_t j ;

#ifdef SPLASH_DEBUG
	SplashDebug[8] = 0 ;
#endif

	if( (!g_eeGeneral.disableSplashScreen) || ( HardwareMenuEnabled ) )
  {
   	check_backlight() ;
    lcd_clear();
		refreshDisplay();
    lcdSetRefVolt(g_eeGeneral.contrast);
  	clearKeyEvents();

#ifndef SIMU
  	for( i=0; i<32; i++)
    	getADC_filt(); // init ADC array
#endif

  	uint16_t inacSum = stickMoveValue() ;

#ifdef SPLASH_DEBUG
  	for( i=0; i<4 ; i++)
			SplashDebug[i+4] = SplashDebug[i] ;
#endif

  	uint16_t tgtime = get_tmr10ms() + SPLASH_TIMEOUT;  
  	uint16_t scrtime = get_tmr10ms() ;

		j = 62 ;
  	while(tgtime > get_tmr10ms())
  	{
			if ( scrtime < get_tmr10ms() )
			{
				scrtime += 4 ;
				uint8_t p ;
				uint8_t x ;
				uint8_t y ;
				uint8_t z ;
				lcd_clear();
  	 		lcd_img(0, 0, &splashdata[4],0,0);
				if(!g_eeGeneral.hideNameOnSplash)
				lcd_putsnAtt( 0*FW, 7*FH, g_eeGeneral.ownerName ,sizeof(g_eeGeneral.ownerName),0);

				if ( j )
				{
					plotType = PLOT_WHITE ;
					 
					p = 0 ;
					x = 126 ;
					z = 64 ;
					for ( y = 0 ; y < j ; y += 2 )
					{
						lcd_vline( y, p, z ) ;
						lcd_vline( 127-y, p, z ) ;
						lcd_rect( y+1, p, x, z ) ;
						p += 1 ;
						z -= 2 ;
						x -= 4 ;
					}	
					j -= 2 ;
					plotType = PLOT_XOR ;
				}

  			refreshDisplay();
			}

#ifdef SIMU
        if (!main_thread_running) return;
        sleep(1/*ms*/);
#else
        getADC_filt();
#endif

			uint8_t xxx ;
			if ( ( xxx = keyDown() ) )
			{
#ifdef SPLASH_DEBUG
				SplashDebug[8] = xxx ;
#endif
				return ;  //wait for key release
			}
				  
			if ( hasStickMoved( inacSum ) )
			{
#ifdef SPLASH_DEBUG
				SplashDebug[8] = 0xF000 ;
#endif
		   return ;  //wait for key release
			}
//			if(keyDown() || (tsum!=inacSum))   return;  //wait for key release

			if ( check_power_or_usb() )
			{
#ifdef SPLASH_DEBUG
				SplashDebug[8] = 0xC000 ;
#endif
				return ;		// Usb on or power off
			}
			check_backlight() ;
  	  wdt_reset();
  	}
	}
}


//global helper vars
bool    checkIncDec_Ret;
struct t_p1 P1values ;
static uint8_t LongMenuTimer ;
uint8_t StepSize = 20 ;

//int16_t checkIncDec16(uint8_t event, int16_t val, int16_t i_min, int16_t i_max, uint8_t i_flags)
int16_t checkIncDec16( int16_t val, int16_t i_min, int16_t i_max, uint8_t i_flags)
{
  int16_t newval = val;
  uint8_t kpl=KEY_RIGHT, kmi=KEY_LEFT, kother = -1;
//	uint8_t skipPause = 0 ;
	uint8_t editAllowed = 1 ;
	
	if ( g_eeGeneral.forceMenuEdit and s_editMode == 0 )
	{
		editAllowed = 0 ;
	}

		uint8_t event = Tevent ;
//  if(event & _MSK_KEY_DBL){
//    uint8_t hlp=kpl;
//    kpl=kmi;
//    kmi=hlp;
//    event=EVT_KEY_FIRST(EVT_KEY_MASK & event);
//  }
  if(event==EVT_KEY_FIRST(kpl) || event== EVT_KEY_REPT(kpl) || (s_editMode && (event==EVT_KEY_FIRST(KEY_UP) || event== EVT_KEY_REPT(KEY_UP))) )
	{
		if ( editAllowed )
		{
			if ( menuPressed() )
			{
    		newval += StepSize ;
			}		 
			else
			{
    		newval += 1 ;
			}
			audioDefevent(AU_KEYPAD_UP);
    	kother=kmi;
		}

  }else if(event==EVT_KEY_FIRST(kmi) || event== EVT_KEY_REPT(kmi) || (s_editMode && (event==EVT_KEY_FIRST(KEY_DOWN) || event== EVT_KEY_REPT(KEY_DOWN))) )
	{
		if ( editAllowed )
		{
			if ( menuPressed() )
			{
    		newval -= StepSize ;
			}		 
			else
			{
    		newval -= 1 ;
			}
			audioDefevent(AU_KEYPAD_DOWN);
    	kother=kpl;
		}

  }
  if((kother != (uint8_t)-1) && keyState((EnumKeys)kother)){
    newval=-val;
    killEvents(kmi);
    killEvents(kpl);
  }
  if(i_min==0 && i_max==1 )
	{
		if ( (event==EVT_KEY_FIRST(KEY_MENU) || event==EVT_KEY_BREAK(BTN_RE)) )
		{
      s_editMode = false;
      newval=!val;
      killEvents(event);
//			skipPause = 1 ;
			if ( event==EVT_KEY_BREAK(BTN_RE) )
			{
				RotaryState = ROTARY_MENU_UD ;
			}
			event = 0 ;
		}
		else
		{
			newval &= 1 ;
		}
	}
//  if (s_editMode>0 && (i_flags & INCDEC_SWITCH))
  if ( i_flags & INCDEC_SWITCH )
	{
		if ( s_editMode )
		{
    	int8_t swtch = getMovedSwitch();
    	if (swtch)
			{
	#if defined(PCBSKY) || defined(PCB9XT)
				swtch = switchUnMap( swtch ) ;
	#endif
    	  newval = swtch ;
    	}
		}
  }

  //change values based on P1
  newval -= P1values.p1valdiff;
	if ( RotaryState == ROTARY_VALUE )
	{
		newval += ( menuPressed() ) ? Rotary_diff * 20 : Rotary_diff ;
	}
  if(newval>i_max)
  {
    newval = i_max;
    killEvents(event);
    audioDefevent(AU_KEYPAD_UP);
  }
  else if(newval < i_min)
  {
    newval = i_min;
    killEvents(event);
    audioDefevent(AU_KEYPAD_DOWN);
  }
  if(newval != val)
	{
		if ( menuPressed() )
		{
			LongMenuTimer = 255 ;
		}
    if(newval==0)
		{
//			if ( !skipPause )
//			{
     	  pauseEvents(event);
//			}
			if (newval>val)
			{
				audioDefevent(AU_KEYPAD_UP);
			}
			else
			{
				audioDefevent(AU_KEYPAD_DOWN);
			}		
    }
    eeDirty(i_flags & (EE_GENERAL|EE_MODEL));
    checkIncDec_Ret = true;
  }
  else {
    checkIncDec_Ret = false;
  }
	StepSize = 20 ;
  return newval;
}

int8_t checkIncDec( int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags)
{
  return checkIncDec16(i_val,i_min,i_max,i_flags);
}

int8_t checkIncDecSwitch( int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags)
{
	i_val = switchUnMap( i_val ) ;
  return switchMap( checkIncDec16(i_val,i_min,i_max,i_flags) ) ;
//#if PCBSKY
//	i_val = switchUnMap( i_val ) ;
//  return switchMap( checkIncDec16(event,i_val,i_min,i_max,i_flags) ) ;
//#endif
//#if PCBX9D
//	i_val = switchUnMap( i_val ) ;
//  return switchMap( checkIncDec16(event,i_val,i_min,i_max,i_flags) ) ;
////  return checkIncDec16(event,i_val,i_min,i_max,i_flags) ;
//#endif
}

int8_t checkIncDec_hm( int8_t i_val, int8_t i_min, int8_t i_max)
{
  return checkIncDec(i_val,i_min,i_max,EE_MODEL);
}

int8_t checkIncDec_hm0( int8_t i_val, int8_t i_max)
{
  return checkIncDec(i_val,0,i_max,EE_MODEL);
}

int8_t checkIncDec_hg( int8_t i_val, int8_t i_min, int8_t i_max)
{
  return checkIncDec(i_val,i_min,i_max,EE_GENERAL);
}

int8_t checkIncDec_hg0( int8_t i_val, int8_t i_max)
{
  return checkIncDec(i_val,0,i_max,EE_GENERAL);
}

//int8_t *TrimPtr[4] = 
//{
//  &g_model.trim[0],
//  &g_model.trim[1],
//  &g_model.trim[2],
//  &g_model.trim[3]
//} ;

#if defined(USB_JOYSTICK) && !defined(SIMU)
extern USB_OTG_CORE_HANDLE USB_OTG_dev;

/*
  Prepare and send new USB data packet

  The format of HID_Buffer is defined by
  USB endpoint description can be found in 
  file usb_hid_joystick.c, variable HID_JOYSTICK_ReportDesc
*/
void usbJoystickUpdate(void)
{
  static uint8_t HID_Buffer[HID_IN_PACKET];
  
  //buttons
  HID_Buffer[0] = 0; //buttons
  for (int i = 0; i < 8; ++i) {
    if ( g_chans512[i+8] > 0 ) {
      HID_Buffer[0] |= (1 << i);
    } 
  }

  //analog values
  //uint8_t * p = HID_Buffer + 1;
  for (int i = 0; i < 8; ++i) {
    int16_t value = g_chans512[i] / 8;
    if ( value > 127 ) value = 127;
    else if ( value < -127 ) value = -127;
    HID_Buffer[i+1] = static_cast<int8_t>(value);  
  }

  USBD_HID_SendReport (&USB_OTG_dev, HID_Buffer, HID_IN_PACKET );
}
#endif


const static uint8_t rate[8] = { 0, 0, 100, 40, 16, 7, 3, 1 } ;
uint32_t calcStickScroll( uint32_t index )
{
	uint32_t direction ;
	int32_t value ;

	if ( ( g_eeGeneral.stickMode & 1 ) == 0 )
	{
		index ^= 3 ;
	}
	
//#ifdef FIX_MODE
	value = phyStick[index] ;
	value /= 8 ;
//#else
//	value = calibratedStick[index] / 128 ;
//#endif
	direction = value > 0 ? 0x80 : 0 ;
	if ( value < 0 )
	{
		value = -value ;			// (abs)
	}
	if ( value > 7 )
	{
		value = 7 ;			
	}
	value = rate[(uint8_t)value] ;
	if ( value )
	{
		StickScrollTimer = STICK_SCROLL_TIMEOUT ;		// Seconds
	}
	return value | direction ;
}

static void	processAdjusters()
{
static uint8_t GvAdjLastSw[NUM_GVAR_ADJUST][2] ;
	for ( CPU_UINT i = 0 ; i < NUM_GVAR_ADJUST ; i += 1 )
	{
		GvarAdjust *pgvaradj ;
		pgvaradj = &g_model.gvarAdjuster[i] ;
		uint32_t idx = pgvaradj->gvarIndex ;
	
		int8_t sw0 = pgvaradj->swtch ;
		int8_t sw1 = 0 ;
		uint32_t switchedON = 0 ;
		int32_t value = g_model.gvars[idx].gvar ;
		if ( sw0 )
		{
			sw0 = getSwitch00(sw0) ;
			if ( !GvAdjLastSw[i][0] && sw0 )
			{
    		switchedON = 1 ;
			}
			GvAdjLastSw[i][0] = sw0 ;
		}
		if ( ( pgvaradj->function > 3 ) && ( pgvaradj->function < 7 ) )
		{
			sw1 = pgvaradj->switch_value ;
			if ( sw1 )
			{
				sw1 = getSwitch00(sw1) ;
				if ( !GvAdjLastSw[i][1] && sw1 )
				{
    			switchedON |= 2 ;
				}
				GvAdjLastSw[i][1] = sw1 ;
			}
		}

		switch ( pgvaradj->function )
		{
			case 1 :	// Add
				if ( switchedON & 1 )
				{
     			value += pgvaradj->switch_value ;
				}
			break ;

			case 2 :
				if ( switchedON & 1 )
				{
     			value = pgvaradj->switch_value ;
				}
			break ;

			case 3 :
				if ( switchedON & 1 )
				{
					if ( pgvaradj->switch_value == 5 )	// REN
					{
						value = RotaryControl ;	// Adjusted elsewhere
					}
					else
					{
						value = getGvarSourceValue( pgvaradj->switch_value ) ;
					}
				}
			break ;

			case 4 :
				if ( switchedON & 1 )
				{
     			value += 1 ;
				}
				if ( switchedON & 2 )
				{
     			value -= 1 ;
				}
			break ;
			
			case 5 :
				if ( switchedON & 1 )
				{
     			value += 1 ;
				}
				if ( switchedON & 2 )
				{
     			value = 0 ;
				}
			break ;

			case 6 :
				if ( switchedON & 1 )
				{
     			value -= 1 ;
				}
				if ( switchedON & 2 )
				{
     			value = 0 ;
				}
			break ;
			
			case 7 :
				if ( switchedON & 1 )
				{
     			value += 1 ;
					if ( value > pgvaradj->switch_value )
					{
						value = pgvaradj->switch_value ;
					}
				}
			break ;
			
			case 8 :
				if ( switchedON & 1 )
				{
     			value -= 1 ;
					if ( value < pgvaradj->switch_value )
					{
						value = pgvaradj->switch_value ;
					}
				}
			break ;

		}
  	if(value > 125)
		{
			value = 125 ;
		}	
  	if(value < -125 )
		{
			value = -125 ;
		}	
		g_model.gvars[idx].gvar = value ;
	}
}

uint8_t AnaEncSw = 0 ;

void valueprocessAnalogEncoder( uint32_t x )
{
	uint32_t y ;
	if ( x < 0x56F )
	{
		if ( x < 0x465 )
		{
			y = ( x < 0x428 ) ? 4 : 0 ;
		}
		else
		{
			y = ( x < 0x4FB ) ? 6 : 2 ;
		}
	}
	else
	{
		if ( x < 0x68B )
		{
			y = ( x < 0x5CE ) ? 5 : 1 ;
		}
		else
		{
			y = ( x < 0x780 ) ? 7 : 3 ;
		}
	}
	AnaEncSw = y & 4 ;
	y &= 3 ;

  uint32_t dummy ;
	
	dummy = y & 1 ;
	if ( y & 2 )
	{
		dummy |= 4 ;
	}
	if ( dummy != ( Rotary_position & 0x05 ) )
	{
		int32_t increment ;
		if ( ( Rotary_position & 0x01 ) ^ ( ( dummy & 0x04) >> 2 ) )
		{
			increment = -1 ;
		}
		else
		{
			increment = 1 ;
		}
		increment += Rotary_count ;
		if ( y == 3 )
		{
			if ( g_eeGeneral.rotaryDivisor == 1)
			{	// Rest position + div by 4
				if ( increment > 0 )
				{
					increment += 2 ;
					increment &= 0xFFFFFFFC ;
				}
				else
				{
					if ( ( increment & 3 ) != 3 )
					{
						increment &= 0xFFFFFFFC ;
					}
				}
			}
		}
		Rotary_count = increment ;
		Rotary_position = ( Rotary_position & ~0x45 ) | dummy ;
	}
}

#ifdef PCBSKY
void processAnaEncoder()
{
	read_adc() ;		// need to just read the 1 channel
	uint32_t x = Analog_values[5] >> 1 ;
	valueprocessAnalogEncoder( x ) ;
}
#endif

uint8_t GvarSource[4] ;

int8_t getGvarSourceValue( uint8_t src )
{
	int16_t value ;
	
	if ( src <= 4 )
	{
//#ifdef FIX_MODE
		value = getTrimValue( CurrentPhase, src - 1 ) ;
//#else
//		value = getTrimValue( CurrentPhase, convert_mode_helper(src) - 1 ) ;
//#endif
		GvarSource[src-1] = 1 ;
//				g_model.gvars[i].gvar = *TrimPtr[ convert_mode_helper(g_model.gvars[i].gvsource) - 1 ] ;
	}
	else if ( src == 5 )	// REN
	{
		value = 0 ;
	}
	else if ( src <= 9 )	// Stick
	{
//#ifdef FIX_MODE
		value = calibratedStick[ src-5 - 1 ] / 8 ;
//#else
//		value = calibratedStick[ convert_mode_helper( src-5) - 1 ] / 8 ;
//#endif
//				g_model.gvars[i].gvar = limit( -125, calibratedStick[ convert_mode_helper(g_model.gvars[i].gvsource-5) - 1 ] / 8, 125 ) ;
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 12 )	// Pot
#endif
#ifdef PCBX9D
	else if ( src <= 13 )	// Pot
#endif
	{
		value = calibratedStick[ ( src-6)] / 8 ;
//				g_model.gvars[i].gvar = limit( -125, calibratedStick[ (g_model.gvars[i].gvsource-6)] / 8, 125 ) ;
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 36 )	// Chans
#endif
#ifdef PCBX9D
	else if ( src <= 37 )	// Pot
#endif
	{
#if defined(PCBSKY) || defined(PCB9XT)
		value = ex_chans[src-13] * 100 / 1024 ;
#endif
#ifdef PCBX9D
		value = ex_chans[src-14] * 100 / 1024 ;
#endif
//				g_model.gvars[i].gvar = limit( -125, ex_chans[g_model.gvars[i].gvsource-13] / 10, 125 ) ;
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 44 )	// Scalers
#endif
#ifdef PCBX9D
	else if ( src <= 45 )	// Scalers
#endif
	{
#if defined(PCBSKY) || defined(PCB9XT)
		value = calc_scaler( src-37, 0, 0 ) ;
#endif
#ifdef PCBX9D
		value = calc_scaler( src-38, 0, 0 ) ;
#endif
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 68 )	// Scalers
#endif
#ifdef PCBX9D
	else if ( src <= 69 )	// Scalers
#endif
	{ // Outputs
		int32_t x ;
#if defined(PCBSKY) || defined(PCB9XT)
		x = g_chans512[src-45] ;
#endif
#ifdef PCBX9D
		x = g_chans512[src-46] ;
#endif
		x *= 100 ;
		value = x / 1024 ;
	}
	else
	{
		value = 0 ;
	}
	if ( value > 125 )
	{
		value = 125 ;
	}
	if ( value < -125 )
	{
		value = -125 ;
	}
 	return value ;
}

void perMain( uint32_t no_menu )
{
  static uint16_t lastTMR;
	uint16_t t10ms ;
	
//	processAnaEncoder() ;
	
	t10ms = get_tmr10ms() ;
  tick10ms = ((uint16_t)(t10ms - lastTMR)) != 0 ;
  lastTMR = t10ms ;

	{
		MixerCount += 1 ;		
		uint16_t t1 = getTmr2MHz() ;
		perOutPhase(g_chans512, 0);
		t1 = getTmr2MHz() - t1 ;
		g_timeMixer = t1 ;
	}

	if(tick5ms)
	{
		check_frsky( 1 ) ;
		tick5ms = 0 ;
	}

	if(!tick10ms) return ; //make sure the rest happen only every 10ms.

#ifdef SERIAL_HOST
	Host10ms = 1 ;
#endif
	if ( ppmInValid )
	{
		ppmInValid -= 1 ;
	}

#if defined(PCBSKY) || defined(PCB9XT)
	if ( BtRxTimer )
	{
		BtRxTimer -= 1 ;
	}
#endif
	if ( SbusTimer )
	{
		SbusTimer -= 1 ;
	}

	heartbeat |= HEART_TIMER10ms;
  
#ifdef PCB9XT
	processAnalogSwitches() ;
#endif // PCB9XT
	
	uint8_t evt=getEvent();
  evt = checkTrim(evt);

		if ( ( evt == 0 ) || ( evt == EVT_KEY_REPT(KEY_MENU) ) )
		{
			uint8_t timer = LongMenuTimer ;
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
			if ( timer == 200 )
			{
				evt = EVT_TOGGLE_GVAR ;
				timer = 255 ;
			}
			LongMenuTimer = timer ;
		}

#ifdef PCBSKY
	int16_t p1d ;

	struct t_p1 *ptrp1 ;
	ptrp1 = &P1values ;
//	FORCE_INDIRECT(ptrp1) ;
	
	int16_t c6 = calibratedStick[6] ;
  p1d = ( ptrp1->p1val-c6 )/32;
  if(p1d)
	{
    p1d = (ptrp1->p1valprev-c6)/2;
    ptrp1->p1val = c6 ;
  }
  ptrp1->p1valprev = c6 ;
  if ( g_eeGeneral.disablePotScroll )
  {
    p1d = 0 ;
	}
	ptrp1->p1valdiff = p1d ;
#endif

	{
		int32_t x ;
		if ( g_eeGeneral.rotaryDivisor == 1)
		{
			x = Rotary_count >> 2 ;
		}
		else if ( g_eeGeneral.rotaryDivisor == 2)
		{
			x = Rotary_count >> 1 ;
		}
		else
		{
			x = Rotary_count ;
		}
		Rotary_diff = x - LastRotaryValue ;
		LastRotaryValue = x ;
	}

	{
		uint16_t a = 0 ;
		uint16_t b = 0 ;
    
		uint16_t lightoffctr ;
		lightoffctr = g_LightOffCounter ;

		if(lightoffctr) lightoffctr -= 1 ;
		if( evt | Rotary_diff )
		{
			a = g_eeGeneral.lightAutoOff*500 ; // on keypress turn the light on 5*100
			InacCounter = 0 ;
		}
		if(InactivityMonitor) b = g_eeGeneral.lightOnStickMove*500 ;
		if(a>lightoffctr) lightoffctr = a ;
		if(b>lightoffctr) lightoffctr = b ;
		g_LightOffCounter = lightoffctr ;
	}
	check_backlight() ;
// Handle volume
	uint8_t requiredVolume ;
	requiredVolume = g_eeGeneral.volume ;

	uint8_t option = g_menuStack[g_menuStackPtr] == menuProc0 ;
	if ( option && ( PopupData.PopupActive == 0 ) )
	{
		if ( Rotary_diff )
		{
			int16_t x = RotaryControl ;
			x += Rotary_diff ;
			if ( x > 125 )
			{
				RotaryControl = 125 ;
			}
			else if ( x < -125 )
			{
				RotaryControl = -125 ;
			}
			else
			{
				RotaryControl = x ;					
			}

			// GVARS adjust
			for( uint8_t i = 0 ; i < MAX_GVARS ; i += 1 )
			{
				if ( g_model.gvars[i].gvsource == 5 )	// REN
				{
					if ( getSwitch( g_model.gvswitch[i], 1, 0 ) )
					{
						int16_t value = g_model.gvars[i].gvar + Rotary_diff ;
						g_model.gvars[i].gvar = limit( (int16_t)-125, value, (int16_t)125 ) ;
					}
			  }
			}
			Rotary_diff = 0 ;
		}
	}


	if ( g_eeGeneral.disablePotScroll || option )
	{			 
		if ( g_model.anaVolume )	// Only check if on main screen
		{
			static uint16_t oldVolValue ;
			uint16_t x ;
			uint16_t divisor ;
#if defined(PCBSKY) || defined(PCB9XT)
			if ( g_model.anaVolume < 4 )
#endif
#ifdef PCBX9D
			if ( g_model.anaVolume < 5 )
#endif
			{
				x = calibratedStick[g_model.anaVolume+3] + 1024 ;
				divisor = 2048 ;
			}
			else
			{
				x = g_model.gvars[g_model.anaVolume].gvar + 125 ;
				divisor = 250 ;
			}
			if ( abs( oldVolValue - x ) > (divisor/125  ) )
			{
				oldVolValue = x ;
			}
			else
			{
				x = oldVolValue ;
			}
			requiredVolume = x * (NUM_VOL_LEVELS-1) / divisor ;
		}
	}
	if ( HoldVolume )
	{
		requiredVolume = HoldVolume ;
	}
	if ( requiredVolume != CurrentVolume )
	{
		setVolume( requiredVolume ) ;
	}

#ifdef PCBX9D
  static uint8_t usbStarted = 0 ;
  if ( !usbStarted && usbPlugged() )
	{
//    usbStart() ;
    usbStarted = 1 ;
  }
#endif
	 
	if ( g_eeGeneral.stickScroll )
	{
	 	if ( StickScrollTimer )
		{
			static uint8_t repeater ;
			uint32_t direction ;
			int32_t value ;
		
			if ( repeater < 128 )
			{
				repeater += 1 ;
			}
			value = calcStickScroll( 2 ) ;
			direction = value & 0x80 ;
			value &= 0x7F ;
			if ( value )
			{
		 		if ( StickScrollAllowed & 2 )
				{
					if ( repeater > value )
					{
						repeater = 0 ;
						if ( direction )
						{
							putEvent(EVT_KEY_FIRST(KEY_UP));
						}
						else
						{
							putEvent(EVT_KEY_FIRST(KEY_DOWN));
						}
					}
				}
			}
			else
			{
				value = calcStickScroll( 3 ) ;
				direction = value & 0x80 ;
				value &= 0x7F ;
				if ( value )
				{
			 		if ( StickScrollAllowed & 1 )
					{
						if ( repeater > value )
						{
							repeater = 0 ;
							if ( direction )
							{
								putEvent(EVT_KEY_FIRST(KEY_RIGHT));
							}
							else
							{
								putEvent(EVT_KEY_FIRST(KEY_LEFT));
							}
						}
					}
				}
			}
		}
	}
	else
	{
		StickScrollTimer = 0 ;		// Seconds
	}	
	StickScrollAllowed = 3 ;

#if GVARS
	GvarSource[0] = 0 ;
	GvarSource[1] = 0 ;
	GvarSource[2] = 0 ;
	GvarSource[3] = 0 ;
	for( uint8_t i = 0 ; i < MAX_GVARS ; i += 1 )
	{
		// ToDo, test for trim inputs here
		if ( g_model.gvars[i].gvsource )
		{
			int16_t value ;
			uint8_t src = g_model.gvars[i].gvsource ;
			if ( g_model.gvswitch[i] )
			{
				if ( !getSwitch00( g_model.gvswitch[i] ) )
				{
					continue ;
				}
			}
			
			if ( src == 5 )	// REN
			{
				value = g_model.gvars[i].gvar ;	// Adjusted elsewhere
			}
			else
			{
				value = getGvarSourceValue( src ) ;
			}

//			else
//			{
//static uint8_t GvLastSw[12] ;
//#ifdef PCBSKY
//				uint8_t j = ( src - 69 ) * 2 ;
//#endif
//#ifdef PCBX9D
//				uint8_t j = ( src - 70 ) * 2 ;
//#endif
//				uint8_t sw0 = getSwitch00(DSW_SW1+j) ;
//				value = g_model.gvars[i].gvar ;
//				if ( !GvLastSw[j] && sw0 )
//				{
//      		value += 1 ;
//				}
//				GvLastSw[j] = sw0 ;
//				sw0 = getSwitch00(DSW_SW2+j) ;
//				if ( sw0 )
//				{
//      		value = 0 ;
//				}
//			}
			g_model.gvars[i].gvar = limit( (int16_t)-125, value, (int16_t)125 ) ;
		}
	}
#endif

#ifdef FRSKY
	check_frsky( 0 ) ;
#endif

// Here, if waiting for EEPROM response, don't action menus

	if ( no_menu == 0 )
	{
		static uint8_t alertKey ;
    lcd_clear();
		if ( AlertMessage )
		{
			almess( AlertMessage, AlertType ) ;
			uint8_t key = keyDown() ;
			if ( alertKey )
			{
				if( key == 0 )
				{
					AlertMessage = 0 ;
				}
			}
			else if ( key )
			{
				alertKey = 1 ;
			}
		}
		else
		{
			alertKey = 0 ;
    	
			if ( EnterMenu )
			{
				evt = EnterMenu ;
				EnterMenu = 0 ;
				audioDefevent(AU_MENUS);
			}
	 		StepSize = 20 ;
	 		Tevent = evt ;
			
			g_menuStack[g_menuStackPtr](evt);
		}
#ifdef PCBX9D
		if ( ( lastTMR & 3 ) == 0 )
#endif
#if defined(PCBSKY) || defined(PCB9XT)
		if ( ( lastTMR & 3 ) == 0 )
#endif
		{
			uint16_t t1 = getTmr2MHz() ;

//			lcd_outhex4( 76, 0, PowerStatus ) ;
//extern uint16_t General_timer ;
//extern uint16_t Model_timer ;
//			lcd_outhex4( 76, FH, General_timer ) ;
//			lcd_outhex4( 100, FH, Model_timer ) ;

  	  refreshDisplay();
			t1 = getTmr2MHz() - t1 ;
			g_timeRfsh = t1 ;
		}
	}

#ifdef PCBSKY
	if ( check_soft_power() == POWER_TRAINER )		// On trainer power
	{
		PIOC->PIO_PDR = PIO_PC22 ;						// Disable bit C22 Assign to peripheral
	}
	else
	{
		PIOC->PIO_PER = PIO_PC22 ;						// Enable bit C22 as input
	}
#endif

#ifdef PCB9XT
	static uint8_t trainerActive = 0 ;
	if ( check_soft_power() == POWER_TRAINER )		// On trainer power
	{
		if ( trainerActive == 0 )
		{
			stop_trainer_capture() ;
			trainerActive = 1 ;
			init_trainer_ppm() ;
		}
	}
	else
	{
		if ( trainerActive )
		{
			stop_trainer_ppm() ;
			trainerActive = 0 ;
			init_trainer_capture() ;
		}
	}
#endif

  switch( get_tmr10ms() & 0x1f )
	{ //alle 10ms*32

    case 2:
//        //check v-bat
////        Calculation By Mike Blandford
////        Resistor divide on battery voltage is 5K1 and 2K7 giving a fraction of 2.7/7.8
////        If battery voltage = 10V then A2D voltage = 3.462V
////        11 bit A2D count is 1417 (3.462/5*2048).
////        1417*18/256 = 99 (actually 99.6) to represent 9.9 volts.
////        Erring on the side of low is probably best.

#if defined(PCBSKY) || defined(PCB9XT)
        int32_t ab = anaIn(12);
#endif
#ifdef PCBX9D
        int32_t ab = anaIn(12);
#endif

        ab = ( ab + ab*(g_eeGeneral.vBatCalib)/128 ) * 4191 ;
//        ab = (uint16_t) ab / (g_eeGeneral.disableBG ? 240 : BandGap ) ;  // ab might be more than 32767
#ifdef PCBSKY
        ab /= 55296  ;
        g_vbat100mV = ( (ab + g_vbat100mV + 1) >> 1 ) + 3 ;  // Filter it a bit => more stable display
								// Also add on 0.3V for voltage drop across input diode
#endif
#ifdef PCBX9D
        ab /= 57165  ;
        g_vbat100mV = ( (ab + g_vbat100mV + 1) >> 1 ) ;  // Filter it a bit => more stable display
#endif
#ifdef PCB9XT
        ab /= 64535  ;
        g_vbat100mV = ( (ab + g_vbat100mV + 1) >> 1 ) ;  // Filter it a bit => more stable display
#endif

        static uint8_t s_batCheck;
        s_batCheck+=16;
        if(s_batCheck==0)
				{
					if( (g_vbat100mV<g_eeGeneral.vBatWarn) && (g_vbat100mV>49) )
					{
						voiceSystemNameNumberAudio( SV_TXBATLOW, V_BATTERY_LOW, AU_TX_BATTERY_LOW ) ;
//            audioVoiceDefevent(AU_TX_BATTERY_LOW, V_BATTERY_LOW | 0x1000 ) ;
            if (g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
					}
#ifdef PCBSKY
					else if ( ( g_eeGeneral.mAh_alarm ) && ( ( MAh_used + Current_used/3600 ) / 500 >= g_eeGeneral.mAh_alarm ) )
					{
						voiceSystemNameNumberAudio( SV_TXBATLOW, V_BATTERY_LOW, AU_TX_BATTERY_LOW ) ;
//            audioVoiceDefevent(AU_TX_BATTERY_LOW, V_BATTERY_LOW | 0x1000 ) ;
					}
#endif
        }
    break ;

  }
  InactivityMonitor = 0; //reset this flag
		
	AUDIO_HEARTBEAT();  // the queue processing

}

#ifdef REV9E
static void init_rotary_encoder()
{
//	configure_pins( 0x0060, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
	configure_pins( 0x3000, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
	g_eeGeneral.rotaryDivisor = 2 ;
}

void checkRotaryEncoder()
{
  register uint32_t dummy ;
	
	dummy = GPIOENCODER->IDR ;	// Read Rotary encoder ( PE6, PE5 )
//	dummy >>= 5 ;
	dummy >>= 12 ;
	dummy &= 0x03 ;			// pick out the two bits
	if ( dummy != ( Rotary_position & 0x03 ) )
	{
		if ( ( Rotary_position & 0x01 ) ^ ( ( dummy & 0x02) >> 1 ) )
		{
			Rotary_count -= 1 ;
//			Rotary_count += 1 ;
		}
		else
		{
			Rotary_count += 1 ;
//			Rotary_count -= 1 ;
		}
		Rotary_position &= ~0x03 ;
		Rotary_position |= dummy ;
	}
}
#endif	// REV9E

#ifdef PCB9XT
extern uint8_t M64EncoderPosition ;
void checkRotaryEncoder()
{
	static uint8_t lastPosition = 0 ;
	if ( lastPosition != M64EncoderPosition )
	{
		int8_t diff = M64EncoderPosition - lastPosition ;
		if ( diff < 9 && diff > -9 )
		{
			Rotary_count += diff ;
		}
		lastPosition = M64EncoderPosition ;
	}
}
#endif	// PCB9XT

#ifdef PCBSKY
#if !defined(SIMU)
static void init_rotary_encoder()
{
  register uint32_t dummy;

	configure_pins( PIO_PC19 | PIO_PC21, PIN_ENABLE | PIN_INPUT | PIN_PORTC | PIN_PULLUP ) ;	// 19 and 21 are rotary encoder
	configure_pins( PIO_PB6, PIN_ENABLE | PIN_INPUT | PIN_PORTB | PIN_PULLUP ) ;		// rotary encoder switch
	PIOC->PIO_IER = PIO_PC19 | PIO_PC21 ;
	dummy = PIOC->PIO_PDSR ;		// Read Rotary encoder (PC19, PC21)
	dummy >>= 19 ;
	dummy &= 0x05 ;			// pick out the three bits
	Rotary_position &= ~0x45 ;
	Rotary_position |= dummy ;
	NVIC_SetPriority( PIOC_IRQn, 1 ) ; // Lower priority interrupt
	NVIC_EnableIRQ(PIOC_IRQn) ;
	LastRotaryValue = Rotary_count ;
}

//static void stop_rotary_encoder()
//{
//	NVIC_DisableIRQ(PIOC_IRQn) ;
//	PIOC->PIO_IDR = PIO_PC19 | PIO_PC21 ;
//}

extern "C" void PIOC_IRQHandler()
{
  register uint32_t dummy;
	
	dummy = PIOC->PIO_ISR ;			// Read and clear status register
	(void) dummy ;		// Discard value - prevents compiler warning

	dummy = PIOC->PIO_PDSR ;		// Read Rotary encoder (PC19, PC21)
	dummy >>= 19 ;
	dummy &= 0x05 ;			// pick out the three bits
	if ( dummy != ( Rotary_position & 0x05 ) )
	{
		if ( ( Rotary_position & 0x01 ) ^ ( ( dummy & 0x04) >> 2 ) )
		{
			Rotary_count -= 1 ;
		}
		else
		{
			Rotary_count += 1 ;
		}
		Rotary_position &= ~0x45 ;
		Rotary_position |= dummy ;
	}
	// Now for testing only
//	dummy = PIOB->PIO_PDSR & 0x40 ;		// Read the switch
//	if ( ( Rotary_position & 0x40 ) != dummy )
//	{
//		Rotary_position ^= 0x40 ;
//	}
}
#endif
#endif

void interrupt5ms()
{
	static uint32_t pre_scale ;		// Used to get 10 Hz counter


	sound_5ms() ;

#ifdef REV9E
	checkRotaryEncoder() ;
#endif // REV9E

#ifdef PCB9XT
	checkRotaryEncoder() ;
#endif

	tick5ms = 1 ;
	
	if ( ++pre_scale >= 2 )
	{
		PowerOnTime += 1 ;
		Tenms |= 1 ;			// 10 mS has passed
		pre_scale = 0 ;
  	per10ms();
		if (--AlarmTimer == 0 )
		{
			AlarmTimer = 100 ;		// Restart timer
			AlarmCheckFlag += 1 ;	// Flag time to check alarms
		}
		if (--VoiceTimer == 0 )
		{
			VoiceTimer = 10 ;		// Restart timer
			__disable_irq() ;
			VoiceCheckFlag |= 1 ;	// Flag time to check alarms
			__enable_irq() ;
		}
		
		if (--DsmCheckTimer == 0 )
		{
			DsmCheckTimer = 50 ;		// Restart timer
			DsmCheckFlag |= 1 ;	// Flag time to check alarms
		}
	}
}




// For SKY board
// ADC channels are assigned to:
// AD1  stick_RH
// AD2  stick_LH
// AD3  PI#T_TRIM
// AD4  battery
// AD5  HOV_PIT
// AD9  stick_LV
// AD13 HOV_THR
// AD14 stick_RV
// AD15 Chip temperature
// Peripheral ID 29 (0x20000000)
// Note ADC sequencing won't work as it only operates on channels 0-7
//      and we need 9, 13 and 14 as well
// ALSO: Errata says only one channel converted for each trigger

// Needed implementation (11 bit result)
// SINGLE - 1 read then >> 1
// OVERSAMPLE - 4 reads - average = sum then >> 3
// FILTERED - 1 read but filter processing

// Filtering algorithm
// o/p = half previous o/p + temp1
// temp1 = average temp1 and temp0
// temp0 = average new reading and temp0

uint16_t LastAnaIn[4] ; //[NUMBER_ANALOG+NUM_EXTRA_ANALOG] ;

#ifndef SIMU
uint16_t anaIn(uint8_t chan)
{
	
#ifdef PCB9XT
  volatile uint16_t *p = &S_anaFilt[chan] ;
	if ( ( chan >= 7 ) && ( chan <= 10 ) )
	{
		uint32_t x = 7 ;
		if ( g_eeGeneral.extraPotsSource[chan-7] )
		{
			x += g_eeGeneral.extraPotsSource[chan-7] - 1 ;
		}
		p = &S_anaFilt[x] ;
	}
#endif
#ifdef PCBSKY
//  static uint8_t crossAna[]={1,5,7,0,4,6,2,3,8};
//  volatile uint16_t *p = &S_anaFilt[crossAna[chan]] ;
  volatile uint16_t *p = &S_anaFilt[chan] ;
	if ( ( chan >= 7 ) && ( chan <= 8 ) )
	{
		uint32_t x = 7 ;
		if ( g_eeGeneral.extraPotsSource[chan-7] )
		{
			x += g_eeGeneral.extraPotsSource[chan-7] - 1 ;
		}
		p = &S_anaFilt[x] ;
	}
#endif
#ifdef PCBX9D
  volatile uint16_t *p = &S_anaFilt[chan] ;
#endif
  uint16_t temp = *p ;
  int16_t t1 ;
	if ( chan < 4 )	// A stick
	{
		if ( g_eeGeneral.stickReverse & ( 1 << chan ) )
		{
			temp = 2048 - temp ;
		}
		t1 = temp - LastAnaIn[chan] ;
		if ( ( t1 < 2 ) && ( t1 > -2 ) )
		{
			temp = LastAnaIn[chan] ;
		}
		else
		{
			LastAnaIn[chan] = temp ;
		}
	}
#ifdef PCBX9D
		if ( chan == 5 )
		{
			temp = 2048 - temp ;
		}
#endif
  return temp ;
}
#endif

uint32_t getAnalogIndex( uint32_t index )
{
	uint32_t z = index ;
	if ( index == 8 )
	{
		if ( g_eeGeneral.ar9xBoard == 1 )
		{
			if ( g_eeGeneral.extraPotsSource[0] == 1 )
			{
				z = 9 ;
			}
		}
		else
		{
			z = 9 ;
		}
	}
	return z ;
}


uint16_t g_timeAdc ;
//void ogetADC_single()
//{
//	register uint32_t x ;
//	uint16_t temp ;
//	uint32_t numAnalog = NUMBER_ANALOG+NUM_EXTRA_ANALOG ;

//#if defined(PCBSKY) || defined(PCB9XT)
//	if ( g_eeGeneral.extraPotsSource[0] )
//	{
//		numAnalog += 1 ;
//	}
//#endif

//#ifdef PCBSKY
//  uint16_t t0 = getTmr2MHz();
//	read_adc() ;
//	t0 = getTmr2MHz() - t0;
//  if ( t0 > g_timeAdc ) g_timeAdc = t0 ;
//#endif
//#if defined(PCBX9D) || defined(PCB9XT)
//		read_adc() ;
//#endif

//	for( x = 0 ; x < numAnalog ; x += 1 )
//	{
//		uint32_t z = x ;
//		if ( x > 7 )
//		{
//			z = getAnalogIndex(x) ;
//		}
//		temp = Analog_values[z] ;
//#ifdef PCBX9D
//		if ( (x==1) || (x==3) )
//		{
//			temp = 4096 - temp ;
//		}
//#endif
//		S_anaFilt[x] = temp >> 1 ;
////		S_anaFilt[x] = temp ;
//	}
//}

void getADC_single()
{
	register uint32_t x ;
	uint16_t temp ;
	uint32_t numAnalog = ANALOG_DATA_SIZE ;

#ifdef PCBSKY
  uint16_t t0 = getTmr2MHz();
#endif
	
	read_adc() ;

#ifdef PCBSKY
	t0 = getTmr2MHz() - t0;
  if ( t0 > g_timeAdc ) g_timeAdc = t0 ;
#endif

	for( x = 0 ; x < numAnalog ; x += 1 )
	{
		temp = AnalogData[x] ;
		S_anaFilt[x] = temp >> 1 ;
	}
}

#if defined(PCBSKY) || defined(PCB9XT)
#define OSMP_SAMPLES	4
#define OSMP_TOTAL		16384
#define OSMP_SHIFT		3
#endif
#ifdef PCBX9D
#define OSMP_SAMPLES	4
#define OSMP_TOTAL		16384
#define OSMP_SHIFT		3
//#define OSMP_SAMPLES	8
//#define OSMP_TOTAL		32768
//#define OSMP_SHIFT		4
#endif

//uint16_t AdcDebug[OSMP_SAMPLES] ;

//void ogetADC_osmp()
//{
//	register uint32_t x ;
//	register uint32_t y ;
//	uint32_t numAnalog = NUMBER_ANALOG+NUM_EXTRA_ANALOG ;

//#if defined(PCBSKY) || defined(PCB9XT)
//	if ( g_eeGeneral.extraPotsSource[0] )
//	{
//		numAnalog += 1 ;
//	}
//#endif
//	uint16_t temp[NUMBER_ANALOG+NUM_POSSIBLE_EXTRA_POTS] ;
//	static uint16_t next_ana[NUMBER_ANALOG+NUM_POSSIBLE_EXTRA_POTS] ;

//	for( x = 0 ; x < numAnalog ; x += 1 )
//	{
//		temp[x] = 0 ;
//	}
//	for( y = 0 ; y < OSMP_SAMPLES ; y += 1 )
//	{
//#ifdef PCBSKY
//		read_adc() ;
//#endif
//#if defined(PCBX9D) || defined(PCB9XT)
//		read_adc() ;
//#endif
//		for( x = 0 ; x < numAnalog ; x += 1 )
//		{
//			uint32_t z = x ;
//			if ( x > 7 )
//			{
//				z = getAnalogIndex(x) ;
//			}
//			temp[x] += Analog_values[z] ;
////			if ( x == 3 )
////			{
////				AdcDebug[y] = Analog_values[x] ;
////			}
//		}
//	}
//#ifdef PCBX9D
//	temp[1] = OSMP_TOTAL - temp[1] ;
//	temp[3] = OSMP_TOTAL - temp[3] ;
//#endif
//	for( x = 0 ; x < NUMBER_ANALOG+NUM_EXTRA_ANALOG ; x += 1 )
//	{
//		uint16_t y = temp[x] >> OSMP_SHIFT ;
//		uint16_t z = S_anaFilt[x] ;
//		uint16_t w = next_ana[x] ;
		
//		int16_t diff = abs( (int16_t) y - z ) ;

//		next_ana[x] = y ;
//		if ( diff < 10 )
//		{
//			if ( y > z )
//			{
//				if ( w > z )
//				{
//					y = z + 1 ;
//				}
//				else
//				{
//					y = z ;
//				}
//			}
//			else if ( y < z )
//			{
//				if ( w < z )
//				{
//					y = z - 1 ;
//				}
//				else
//				{
//					y = z ;
//				}
//			}
//		}
//		S_anaFilt[x] = y ;
////		S_anaFilt[x] = temp[x] >> 2 ;
//	}
//}

void getADC_osmp()
{
	register uint32_t x ;
	register uint32_t y ;
	uint32_t numAnalog = ANALOG_DATA_SIZE ;

	uint16_t temp[ANALOG_DATA_SIZE] ;
	static uint16_t next_ana[ANALOG_DATA_SIZE] ;

	for( x = 0 ; x < numAnalog ; x += 1 )
	{
		temp[x] = 0 ;
	}
	for( y = 0 ; y < OSMP_SAMPLES ; y += 1 )
	{
		read_adc() ;
		
		for( x = 0 ; x < numAnalog ; x += 1 )
		{
			temp[x] += AnalogData[x] ;
		}
	}
	for( x = 0 ; x < ANALOG_DATA_SIZE ; x += 1 )
	{
		uint16_t y = temp[x] >> OSMP_SHIFT ;
		uint16_t z = S_anaFilt[x] ;
		uint16_t w = next_ana[x] ;
		
		int16_t diff = abs( (int16_t) y - z ) ;

		next_ana[x] = y ;
		if ( diff < 10 )
		{
			if ( y > z )
			{
				if ( w > z )
				{
					y = z + 1 ;
				}
				else
				{
					y = z ;
				}
			}
			else if ( y < z )
			{
				if ( w < z )
				{
					y = z - 1 ;
				}
				else
				{
					y = z ;
				}
			}
		}
		S_anaFilt[x] = y ;
	}
}


//void ogetADC_filt()
//{
//	register uint32_t x ;
//	static uint16_t t_ana[2][NUMBER_ANALOG+NUM_POSSIBLE_EXTRA_POTS] ;
//	uint16_t temp ;
//	uint32_t numAnalog = NUMBER_ANALOG+NUM_EXTRA_ANALOG ;

//#if defined(PCBSKY) || defined(PCB9XT)
//	if ( g_eeGeneral.extraPotsSource[0] )
//	{
//		numAnalog += 1 ;
//	}
//#endif

//#ifdef PCBSKY
//	read_adc() ;
//#endif
//#if defined(PCBX9D) || defined(PCB9XT)
//		read_adc() ;
//#endif
//	for( x = 0 ; x < numAnalog ; x += 1 )
//	{
//		temp = S_anaFilt[x] ;
//#ifdef PCBX9D
//		if ( (x==1) || (x==3) )
//		{
//			temp = 2048 - temp ;
////			temp = 4096 - temp ;
//		}
//#endif
//		temp = temp/2 + (t_ana[1][x] >> 2 ) ;
////		temp = temp/2 + (t_ana[1][x] >> 1 ) ;
//#ifdef PCBX9D
//		if ( (x==1) || (x==3) )
//		{
//			temp = 2048 - temp ;
////			temp = 4096 - temp ;
//		}
//#endif
//		S_anaFilt[x] = temp ;
//		t_ana[1][x] = ( t_ana[1][x] + t_ana[0][x] ) >> 1 ;
//		uint32_t z = x ;
//		if ( x > 7 )
//		{
//			z = getAnalogIndex(x) ;
//		}
//		t_ana[0][x] = ( t_ana[0][x] + Analog_values[z] ) >> 1 ;
//	}	 
//}

void getADC_filt()
{
	register uint32_t x ;
	static uint16_t t_ana[2][ANALOG_DATA_SIZE] ;
	uint16_t temp ;
	uint32_t numAnalog = ANALOG_DATA_SIZE ;


	read_adc() ;
	for( x = 0 ; x < numAnalog ; x += 1 )
	{
		temp = S_anaFilt[x] ;
		temp = temp/2 + (t_ana[1][x] >> 2 ) ;
		S_anaFilt[x] = temp ;
		t_ana[1][x] = ( t_ana[1][x] + t_ana[0][x] ) >> 1 ;
		t_ana[0][x] = ( t_ana[0][x] + AnalogData[x] ) >> 1 ;
	}	 
}

uint32_t getFlightPhase()
{
	uint32_t i ;
  for ( i = 0 ; i < MAX_MODES ; i += 1 )
	{
    PhaseData *phase = &g_model.phaseData[i];
    if ( phase->swtch && getSwitch00( phase->swtch ) )
		{
      return i + 1 ;
    }
  }
  return 0 ;
}

int16_t getRawTrimValue( uint8_t phase, uint8_t idx )
{
	if ( phase )
	{
		return g_model.phaseData[phase-1].trim[idx] ;
	}	
	else
	{
//		return *TrimPtr[idx] ;
		return g_model.trim[idx] ;
	}
}

uint32_t getTrimFlightPhase( uint8_t phase, uint8_t idx )
{
  for ( uint32_t i=0 ; i<MAX_MODES ; i += 1 )
	{
    if (phase == 0) return 0;
    int16_t trim = getRawTrimValue( phase, idx ) ;
    if ( trim <= TRIM_EXTENDED_MAX )
		{
			return phase ;
		}
    uint32_t result = trim-TRIM_EXTENDED_MAX-1 ;
    if (result >= phase)
		{
			result += 1 ;
		}
    phase = result;
  }
  return 0;
}


int16_t getTrimValue( uint8_t phase, uint8_t idx )
{
  return getRawTrimValue( getTrimFlightPhase( phase, idx ), idx ) ;
}


void setTrimValue(uint8_t phase, uint8_t idx, int16_t trim)
{
	if ( phase )
	{
		phase = getTrimFlightPhase( phase, idx ) ;
	}
	if ( phase )
	{
    if(trim < -125 || trim > 125)
//    if(trim < -500 || trim > 500)
		{
			trim = ( trim > 0 ) ? 125 : -125 ;
//			trim = ( trim > 0 ) ? 500 : -500 ; For later addition
		}	
  	g_model.phaseData[phase-1].trim[idx] = trim ;
	}
	else
	{
    if(trim < -125 || trim > 125)
		{
			trim = ( trim > 0 ) ? 125 : -125 ;
		}	
  // 	*TrimPtr[idx] = trim ;
		g_model.trim[idx] = trim ;
	}
  STORE_MODELVARS_TRIM ;
}


static uint8_t checkTrim(uint8_t event)
{
  int8_t  k = (event & EVT_KEY_MASK) - TRM_BASE;
  int8_t  s = g_model.trimInc;
  
//    if (s>1) s = 1 << (s-1);  // 1=>1  2=>2  3=>4  4=>8
		if ( s == 4 )
		{
			s = 8 ;			  // 1=>1  2=>2  3=>4  4=>8
		}
		else
		{
			if ( s == 3 )
			{
				s = 4 ;			  // 1=>1  2=>2  3=>4  4=>8
			}
		}

  if( (k>=0) && (k<8) && !IS_KEY_BREAK(event)) // && (event & _MSK_KEY_REPT))
  {
    //LH_DWN LH_UP LV_DWN LV_UP RV_DWN RV_UP RH_DWN RH_UP
    uint8_t idx = k/2;
		
// SORT idx for stickmode if FIX_MODE on
//#ifdef FIX_MODE
				idx = stickScramble[g_eeGeneral.stickMode*4+idx] ;
//#endif
		if ( g_eeGeneral.crosstrim )
		{
			idx = 3 - idx ;			
		}
		uint32_t phaseNo = getTrimFlightPhase( CurrentPhase, idx ) ;
    int16_t tm = getTrimValue( phaseNo, idx ) ;
    int8_t  v = (s==0) ? (abs(tm)/4)+1 : s;
//#ifdef FIX_MODE
    bool thrChan = (2 == idx) ;
//#else
//		bool thrChan = ((2-(g_eeGeneral.stickMode&1)) == idx);
//#endif
		bool thro = (thrChan && (g_model.thrTrim));
    if(thro) v = 2; // if throttle trim and trim trottle then step=2

		if ( GvarSource[idx] )
		{
			v = 1 ;
		}

    if(thrChan && throttleReversed()) v = -v;  // throttle reversed = trim reversed
    int16_t x = (k&1) ? tm + v : tm - v;   // positive = k&1

    if(((x==0)  ||  ((x>=0) != (tm>=0))) && (!thro) && (tm!=0))
		{
			setTrimValue( phaseNo, idx, 0 ) ;
      killEvents(event);
      audioDefevent(AU_TRIM_MIDDLE);
    }
		else if(x>-125 && x<125)
		{
			setTrimValue( phaseNo, idx, x ) ;
//      STORE_MODELVARS_TRIM;
      //if(event & _MSK_KEY_REPT) warble = true;
			if(x <= 125 && x >= -125)
			{
//				if(g_eeGeneral.speakerMode == 0){
//					audioDefevent(AU_TRIM_MOVE);
//				}
//				else
//				{
					audio.event(AU_TRIM_MOVE,(abs(x)/4)+60);
//				}
			}	
    }
    else
    {
			setTrimValue( phaseNo, idx, (x>0) ? 125 : -125 ) ;
//      STORE_MODELVARS_TRIM;
			if(x <= 125 && x >= -125){
//				if(g_eeGeneral.speakerMode == 0){
//					audioDefevent(AU_TRIM_MOVE);
//				} else {
					audio.event(AU_TRIM_MOVE,(-abs(x)/4)+60);
//				}
			}	
    }
    return 0;
  }
  return event;
}


void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att)
{
	uint8_t chanLimit = NUM_SKYXCHNRAW ;
	uint8_t mix = att & MIX_SOURCE ;
	if ( mix )
	{
#if GVARS
		chanLimit += MAX_GVARS + 1 + 1 ;
#else
		chanLimit += 1 ;
#endif
		att &= ~MIX_SOURCE ;		
	}
  if(idx==0)
		lcd_putsAtt(x,y,XPSTR("----"),att);
  else if(idx<=4)
//#ifdef FIX_MODE
	{
		const char *ptr = "" ;
		if ( g_model.useCustomStickNames )
		{
			ptr = ( char *)g_eeGeneral.customStickNames+4*(idx-1) ;
		}
		if ( *ptr && (*ptr != ' ' ) )
		{
			lcd_putsnAtt( x, y, ptr, 4, att ) ;
		}
		else
		{
      lcd_putsAttIdx(x,y,PSTR(STR_STICK_NAMES),(idx-1),att) ;
		}
	}
//#else
//    lcd_putsnAtt(x,y,&modi12x3[(modn12x3[g_eeGeneral.stickMode*4+(idx-1)]-1)*4],4,att) ;
//#endif
  else if(idx<=chanLimit)
#if GVARS
    lcd_putsAttIdx(x,y,PSTR(STR_CHANS_GV),(idx-5),att);
#else
    lcd_putsAttIdx(x,y,PSTR(STR_CHANS_RAW),(idx-5),att);
#endif
  
//#if NUM_EXTRA_POTS
	else if(idx < EXTRA_POTS_START)
//#else
//	else
//#endif	// NUM_EXTRA_POTS
	{
		if ( mix )
		{
			idx += TEL_ITEM_SC1-(chanLimit-NUM_SKYXCHNRAW) ;
			if ( idx - NUM_SKYXCHNRAW > TEL_ITEM_SC1 + NUM_SCALERS )
			{
				lcd_putsAtt(x,y,XPSTR("PPM"),att) ;
				idx -= TEL_ITEM_SC1 + NUM_SCALERS - 8 + NUM_SKYXCHNRAW ;
				if ( idx == 9 )
				{
					lcd_putcAtt( x+3*FW, y, '9', att  ) ;
				}
				else
				{
  				lcd_outdezAtt( x+5*FW-2, y,idx, att ) ;
				}
				return ;
			}
		}
  	lcd_putsAttIdx(x,y,PSTR(STR_TELEM_ITEMS),(idx-NUM_SKYXCHNRAW),att);
	}
//#if NUM_EXTRA_POTS
	else
	{
  	lcd_putsAttIdx(x,y,PSTR(STR_CHANS_EXTRA),(idx-EXTRA_POTS_START),att);
	}
//#endif	// NUM_EXTRA_POTS
}

void putsChn(uint8_t x,uint8_t y,uint8_t idx1,uint8_t att)
{
	if ( idx1 == 0 )
	{
    lcd_putsnAtt(x,y,XPSTR("--- "),4,att);
	}
	else
	{
		uint8_t x1 ;
		x1 = x + 4*FW-2 ;
		if ( idx1 < 10 )
		{
			x1 -= FWNUM ;			
		}
//		lcd_outdezNAtt(uint8_t x,uint8_t y,int32_t val,uint8_t mode,int8_t len)
//  	lcd_outdezNAtt(x+2*FW,y,idx1,LEFT|att,2);
  	lcd_outdezAtt(x1,y,idx1,att);
    lcd_putsnAtt(x,y,PSTR(STR_CH),2,att);
	}
  // !! todo NUM_CHN !!
//  lcd_putsnAtt(x,y,XPSTR("--- CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16"
//                        "CH17CH18CH19CH20CH21CH22CH23CH24CH25CH26CH27CH28CH29CH30")+4*idx1,4,att);
}

#if PCBX9D
uint8_t MaxSwitchIndex = MAX_SKYDRSWITCH ;		// For ON and OFF
#endif

#if defined(PCBSKY) || defined(PCB9XT)

uint8_t switchMapTable[80] ;
uint8_t switchUnMapTable[80] ;
uint8_t MaxSwitchIndex ;		// For ON and OFF
uint8_t Sw3PosList[8] ;
uint8_t Sw3PosCount[8] ;

void createSwitchMapping()
{
	uint8_t *p = switchMapTable ;
	uint16_t map = g_eeGeneral.switchMapping ;
	*p++ = 0 ;
	if ( map & USE_THR_3POS )
	{
		*p++ = HSW_Thr3pos0 ;
		*p++ = HSW_Thr3pos1 ;
		*p++ = HSW_Thr3pos2 ;
	}
	else
	{
		*p++ = HSW_ThrCt ;
	}
	
	if ( map & USE_RUD_3POS )
	{
		*p++ = HSW_Rud3pos0 ;
		*p++ = HSW_Rud3pos1 ;
		*p++ = HSW_Rud3pos2 ;
	}
	else
	{
		*p++ = HSW_RuddDR ;
	}

	if ( map & USE_ELE_3POS )
	{
		*p++ = HSW_Ele3pos0 ;
		*p++ = HSW_Ele3pos1 ;
		*p++ = HSW_Ele3pos2 ;
	}
	else if ( map & USE_ELE_6POS )
	{
		*p++ = HSW_Ele6pos0 ;
		*p++ = HSW_Ele6pos1 ;
		*p++ = HSW_Ele6pos2 ;
		*p++ = HSW_Ele6pos3 ;
		*p++ = HSW_Ele6pos4 ;
		*p++ = HSW_Ele6pos5 ;
	}
	else
	{
		*p++ = HSW_ElevDR ;
	}

	if ( g_eeGeneral.analogMapping & MASK_6POS )
	{
		*p++ = HSW_Ele6pos0 ;
		*p++ = HSW_Ele6pos1 ;
		*p++ = HSW_Ele6pos2 ;
		*p++ = HSW_Ele6pos3 ;
		*p++ = HSW_Ele6pos4 ;
		*p++ = HSW_Ele6pos5 ;
	}

	*p++ = HSW_ID0 ;
	*p++ = HSW_ID1 ;
	*p++ = HSW_ID2 ;
	
	if ( map & USE_AIL_3POS )
	{
		*p++ = HSW_Ail3pos0 ;
		*p++ = HSW_Ail3pos1 ;
		*p++ = HSW_Ail3pos2 ;
	}
	else
	{
		*p++ = HSW_AileDR ;
	}

	if ( map & USE_GEA_3POS )
	{
		*p++ = HSW_Gear3pos0 ;
		*p++ = HSW_Gear3pos1 ;
		*p++ = HSW_Gear3pos2 ;
	}
	else
	{
		*p++ = HSW_Gear ;
	}
	*p++ = HSW_Trainer ;
	if ( map & USE_PB1 )
	{
		*p++ = HSW_Pb1 ;
	}
	if ( map & USE_PB2 )
	{
		*p++ = HSW_Pb2 ;
	}
	if ( map & USE_PB3 )
	{
		*p++ = HSW_Pb3 ;
	}
	if ( map & USE_PB4 )
	{
		*p++ = HSW_Pb4 ;
	}
	for ( uint32_t i = 10 ; i <=33 ; i += 1  )
	{
		*p++ = i ;	// Custom switches
	}
	*p = MAX_SKYDRSWITCH ;
	MaxSwitchIndex = p - switchMapTable ;
	*++p = MAX_SKYDRSWITCH+1 ;
	*++p = MAX_SKYDRSWITCH+2 ;
	*++p = MAX_SKYDRSWITCH+3 ;
	*++p = MAX_SKYDRSWITCH+4 ;
	*++p = MAX_SKYDRSWITCH+5 ;

	for ( uint32_t i = 0 ; i <= (uint32_t)MaxSwitchIndex+5 ; i += 1  )
	{
		switchUnMapTable[switchMapTable[i]] = i ;
	}

	uint32_t index = 1 ;
	Sw3PosList[0] = HSW_ID0 ;
	Sw3PosCount[0] = 3 ;
	Sw3PosCount[index] = 2 ;
	Sw3PosList[index] = HSW_ThrCt ;
	if (map & USE_THR_3POS )
	{
		Sw3PosCount[index] = 3 ;
		Sw3PosList[index] = HSW_Thr3pos0 ;
	}
	Sw3PosCount[++index] = 2 ;
	Sw3PosList[index] = HSW_RuddDR ;
	if (map & USE_RUD_3POS )
	{
		Sw3PosCount[index] = 3 ;
		Sw3PosList[index] = HSW_Rud3pos0 ;
	}
	Sw3PosCount[++index] = 2 ;
	Sw3PosList[index] = HSW_ElevDR ;
	if ( map & USE_ELE_3POS )
	{
		Sw3PosCount[index] = 3 ;
		Sw3PosList[index] = HSW_Ele3pos0 ;
	}
	if ( map & USE_ELE_6POS )
	{
		Sw3PosCount[index] = 6 ;
		Sw3PosList[index] = HSW_Ele6pos0 ;
	}
	Sw3PosCount[++index] = 2 ;
	Sw3PosList[index] = HSW_AileDR ;
	if (map & USE_AIL_3POS )
	{
		Sw3PosCount[index] = 3 ;
		Sw3PosList[index] = HSW_Ail3pos0 ;
	}
	Sw3PosCount[++index] = 2 ;
	Sw3PosList[index] = HSW_Gear ;
	if (map & USE_GEA_3POS )
	{
		Sw3PosCount[index] = 3 ;
		Sw3PosList[index] = HSW_Gear3pos0 ;
	}
	Sw3PosCount[++index] = 2 ;
	Sw3PosList[index] = HSW_Trainer ;
}

void create6posTable()
{
	uint32_t i ;


	for ( i = 0 ; i < 5 ; i += 1 )
	{
		uint32_t j = i ;
		if ( g_eeGeneral.SixPositionCalibration[5] >  g_eeGeneral.SixPositionCalibration[0] )
		{
			j = 4 - i ;
		}
		SixPositionTable[j] = 
		(g_eeGeneral.SixPositionCalibration[i+1] + g_eeGeneral.SixPositionCalibration[i]) / 2 ;
	}
}

#endif

#ifdef PCBX9D
uint8_t switchMapTable[90] ;
uint8_t switchUnMapTable[90] ;

// So, I think I map SA0/1/2 to ELE 3-pos, SC0/1/2 to ID0/1/2, SD0/1/2 to AIL 3-pos, 
// SE0/1/2 to RUD 3-pos, SG0/1/2 to GEA 3-pos, SB0/1/2 to THR 3-pos,
// SF0/2 to THR/RUD and SH0/2 to GEA/TRN.

void createSwitchMapping()
{
	uint8_t *p = switchMapTable ;
	
//	uint8_t map = g_eeGeneral.switchMapping ;
	
	*p++ = 0 ;
	*p++ = HSW_SA0 ;
	*p++ = HSW_SA1 ;
	*p++ = HSW_SA2 ;
	
	*p++ = HSW_SB0 ;
	*p++ = HSW_SB1 ;
	*p++ = HSW_SB2 ;

	*p++ = HSW_SC0 ;
	*p++ = HSW_SC1 ;
	*p++ = HSW_SC2 ;
	
#ifdef REV9E
	*p++ = HSW_SF0 ;
	*p++ = HSW_SF1 ;
	*p++ = HSW_SF2 ;
#else
	*p++ = HSW_SD0 ;
	*p++ = HSW_SD1 ;
	*p++ = HSW_SD2 ;
#endif
	 
	*p++ = HSW_SE0 ;
	*p++ = HSW_SE1 ;
	*p++ = HSW_SE2 ;

//	*p++ = HSW_SF0 ;
#ifdef REV9E
	*p++ = HSW_SD2 ;
#else
	*p++ = HSW_SF2 ;
#endif

	*p++ = HSW_SG0 ;
	*p++ = HSW_SG1 ;
	*p++ = HSW_SG2 ;
	
//	*p++ = HSW_SH0 ;
	*p++ = HSW_SH2 ;

#ifdef REV9E
	*p++ = HSW_SI0 ;
	*p++ = HSW_SI1 ;
	*p++ = HSW_SI2 ;
	*p++ = HSW_SJ0 ;
	*p++ = HSW_SJ1 ;
	*p++ = HSW_SJ2 ;
	*p++ = HSW_SK0 ;
	*p++ = HSW_SK1 ;
	*p++ = HSW_SK2 ;
	*p++ = HSW_SL0 ;
	*p++ = HSW_SL1 ;
	*p++ = HSW_SL2 ;
	*p++ = HSW_SM0 ;
	*p++ = HSW_SM1 ;
	*p++ = HSW_SM2 ;
	*p++ = HSW_SN0 ;
	*p++ = HSW_SN1 ;
	*p++ = HSW_SN2 ;
	*p++ = HSW_SO0 ;
	*p++ = HSW_SO1 ;
	*p++ = HSW_SO2 ;
	*p++ = HSW_SP0 ;
	*p++ = HSW_SP1 ;
	*p++ = HSW_SP2 ;
	*p++ = HSW_SQ0 ;
	*p++ = HSW_SQ1 ;
	*p++ = HSW_SQ2 ;
	*p++ = HSW_SR0 ;
	*p++ = HSW_SR1 ;
	*p++ = HSW_SR2 ;
#endif	// REV9E

	if ( g_eeGeneral.analogMapping & MASK_6POS )
	{
		*p++ = HSW_Ele6pos0 ;
		*p++ = HSW_Ele6pos1 ;
		*p++ = HSW_Ele6pos2 ;
		*p++ = HSW_Ele6pos3 ;
		*p++ = HSW_Ele6pos4 ;
		*p++ = HSW_Ele6pos5 ;
	}
	 
	for ( uint32_t i = 10 ; i <=33 ; i += 1  )
	{
		*p++ = i ;	// Custom switches
	}
	*p = MAX_SKYDRSWITCH ;
	MaxSwitchIndex = p - switchMapTable ;
	*++p = MAX_SKYDRSWITCH+1 ;
	*++p = MAX_SKYDRSWITCH+2 ;
	*++p = MAX_SKYDRSWITCH+3 ;
	*++p = MAX_SKYDRSWITCH+4 ;
	*++p = MAX_SKYDRSWITCH+5 ;

	for ( uint32_t i = 0 ; i <= (uint32_t)MaxSwitchIndex+5 ; i += 1  )
	{
		switchUnMapTable[switchMapTable[i]] = i ;
	}
}

#endif

int8_t switchUnMap( int8_t x )
{
	uint8_t sign = 0 ;
	if ( x < 0 )
	{
		sign = 1 ;
		x = -x ;
	}
	x = switchUnMapTable[x] ;
	if ( sign )
	{
		x = -x ;
	}
	return x ;
}

int8_t switchMap( int8_t x )
{
	uint8_t sign = 0 ;
	if ( x < 0 )
	{
		sign = 1 ;
		x = -x ;
	}
	x = switchMapTable[x] ;
	if ( sign )
	{
		x = -x ;
	}
	return x ;
}



void putsMomentDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att)
{
  int16_t tm = idx1 ;
#if defined(PCBSKY) || defined(PCB9XT)
	if ( tm < -HSW_MAX )
	{
		tm += 256 ;
	}
#endif
#ifdef PCBX9D
  if(abs(tm)>(HSW_MAX))	 //momentary on-off
//  if(abs(tm)>=(MAX_SKYDRSWITCH))	 //momentary on-off
#endif
#if defined(PCBSKY) || defined(PCB9XT)
  if(abs(tm)>(HSW_MAX))	 //momentary on-off
#endif
	{
  	lcd_putcAtt(x+3*FW,  y,'m',att);
		if ( tm > 0 )
		{
#ifdef PCBX9D
			tm -= HSW_MAX ;
//			tm -= MAX_SKYDRSWITCH - 1 ;
#endif
#if defined(PCBSKY) || defined(PCB9XT)
			tm -= HSW_MAX ;
#endif
		}
	}			 
  putsDrSwitches( x-1*FW, y, tm, att ) ;
}

void putsDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att)//, bool nc)
{
	if ( idx1 == 0 )
	{
    lcd_putsAtt(x+FW,y,XPSTR("---"),att);return;
	}
	else if ( idx1 == MAX_SKYDRSWITCH )
	{
    lcd_putsAtt(x+FW,y,PSTR(STR_ON),att);return;
	}
	else if ( idx1 == -MAX_SKYDRSWITCH )
	{
    lcd_putsAtt(x+FW,y,PSTR(STR_OFF),att);return;
	}

	if ( idx1 < 0 )
	{
  	lcd_putcAtt(x,y, '!',att);
	}
#if defined(PCBSKY) || defined(PCB9XT)
	int8_t z ;
	z = idx1 ;
	if ( z < 0 )
	{
		z = -idx1 ;			
	}
	z -= 1 ;
//		z *= 3 ;
	if ( z > MAX_SKYDRSWITCH )
	{
		z -= HSW_OFFSET - 1 ;
	}
  lcd_putsAttIdx(x+FW,y,PSTR(SWITCHES_STR),z,att) ;
#endif

#ifdef PCBX9D
	int8_t z ;
	z = idx1 ;
	if ( z < 0 )
	{
		z = -idx1 ;			
	}
	z -= 1 ;
//		z *= 3 ;
	if ( z > MAX_SKYDRSWITCH )
	{
		z -= HSW_OFFSET - 1 ;
	}
  lcd_putsAttIdx(x+FW,y,PSTR(SWITCHES_STR),z,att) ;
//	pstr = get_switches_string()+3*(abs(idx1)-1) ;
//  lcd_putsnAtt(x+FW,y,pstr,3,att);
//	if ( att & CONDENSED )
//	{
//		pstr += 2 ;
//		att &= ~CONDENSED ;
//  	lcd_putcAtt(x+3*FW-1, y,*pstr,att);
//	}
#endif
}

//Type 1-trigA, 2-trigB, 0 best for display
void putsTmrMode(uint8_t x, uint8_t y, uint8_t attr, uint8_t timer, uint8_t type )
{
  int8_t tm = g_model.timer[timer].tmrModeA ;
	if ( type < 2 )		// 0 or 1
	{
	  if(tm<TMR_VAROFS) {
        lcd_putsnAtt(  x, y, PSTR(STR_TRIGA_OPTS)+3*abs(tm),3,attr);
//    return;
  	}
		else
		{
  		tm -= TMR_VAROFS - 7 ;
      lcd_putsAttIdx(  x, y, get_curve_string(), tm, attr ) ;
#if defined(PCBSKY) || defined(PCB9XT)
			if ( tm < 9 + 7 )	// Allow for 7 offset above
#endif
#ifdef PCBX9D
			if ( tm < 9 )
#endif
			{
				x -= FW ;		
			}
  		lcd_putcAtt(x+3*FW,  y,'%',attr);
		}
	}
	if ( ( type == 2 ) || ( ( type == 0 ) && ( tm == 1 ) ) )
	{
		putsMomentDrSwitches( x, y, g_model.timer[timer].tmrModeB, attr ) ;
//    int16_t tm = g_model.timer[timer].tmrModeB;
//#ifdef PCBSKY
//		if ( tm < -HSW_MAX )
//		{
//			tm += 256 ;
//		}
//#endif
//#ifdef PCBX9D
//    if(abs(tm)>=(MAX_SKYDRSWITCH))	 //momentary on-off
//#endif
//#ifdef PCBSKY
//    if(abs(tm)>(HSW_MAX))	 //momentary on-off
//#endif
//		{
//  	  lcd_putcAtt(x+3*FW,  y,'m',attr);
//			if ( tm > 0 )
//			{
//#ifdef PCBX9D
//				tm -= MAX_SKYDRSWITCH - 1 ;
//#endif
//#ifdef PCBSKY
//				tm -= HSW_MAX ;
//#endif
//			}
//		}			 
//   	putsDrSwitches( x-1*FW, y, tm, attr );
	}
}

const char *get_switches_string()
{
  return PSTR(SWITCHES_STR)+1	;
}	

int16_t getValue(uint8_t i)
{
  if(i<7) return calibratedStick[i];//-512..512
//#ifdef PCBX9D
// #if NUM_EXTRA_POTS
	if ( i >= EXTRA_POTS_START-1 )
	{
		return calibratedStick[i-EXTRA_POTS_START+8] ;
	}
// #endif
//#endif
  if(i<PPM_BASE) return 0 ;
	else if(i<CHOUT_BASE)
	{
		int16_t x ;
		x = g_ppmIns[i-PPM_BASE] ;
		if(i<PPM_BASE+4)
		{
			x -= g_eeGeneral.trainer.calib[i-PPM_BASE] ;
		}
		return x*2;
	}
	else if(i<CHOUT_BASE+NUM_SKYCHNOUT) return ex_chans[i-CHOUT_BASE];
  else if(i<CHOUT_BASE+NUM_SKYCHNOUT+NUM_TELEM_ITEMS)
	{
		return get_telemetry_value( i-CHOUT_BASE-NUM_SKYCHNOUT ) ;
	}
  return 0 ;
}



bool getSwitch00( int8_t swtch )
{
	return getSwitch( swtch, 0, 0 ) ;
}

bool getSwitch(int8_t swtch, bool nc, uint8_t level)
{
  bool ret_value ;
  uint8_t cs_index ;
  uint8_t aswitch ;
  
	aswitch = abs(swtch) ;
 	SwitchStack[level] = aswitch ;
	
	cs_index = aswitch-(MAX_SKYDRSWITCH-NUM_SKYCSW);

	{
		int32_t index ;
		for ( index = level - 1 ; index >= 0 ; index -= 1 )
		{
			if ( SwitchStack[index] == aswitch )
			{ // Recursion on this switch taking place
    		ret_value = Last_switch[cs_index] & 1 ;
		    return swtch>0 ? ret_value : !ret_value ;
			}
		}
	}
  if ( level > SW_STACK_SIZE - 1 )
  {
    ret_value = Last_switch[cs_index] & 1 ;
    return swtch>0 ? ret_value : !ret_value ;
  }

	if ( swtch == 0 )
	{
    return nc ;
	}
	else if ( swtch == MAX_SKYDRSWITCH )
	{
    return true ;
	}
	else if ( swtch == -MAX_SKYDRSWITCH )
	{
    return false ;
	}

#if defined(PCBSKY) || defined(PCB9XT)
	if ( abs(swtch) > MAX_SKYDRSWITCH )
	{
		uint8_t value = hwKeyState( abs(swtch) ) ;
		if ( swtch > 0 )
		{
			return value ;
		}
		else
		{
			return ! value ;
		}
	}
#endif
#ifdef PCBX9D
	if ( abs(swtch) > MAX_SKYDRSWITCH )
	{
		uint8_t value = hwKeyState( abs(swtch) ) ;
		if ( swtch > 0 )
		{
			return value ;
		}
		else
		{
			return ! value ;
		}
	}
//	if ( swtch > MAX_SKYDRSWITCH )
//	{
//		return false ;
//	}
#endif

  uint8_t dir = swtch>0;
  if(abs(swtch)<(MAX_SKYDRSWITCH-NUM_SKYCSW)) {
    if(!dir) return ! keyState((enum EnumKeys)(SW_BASE-swtch-1));
    return            keyState((enum EnumKeys)(SW_BASE+swtch-1));
  }

  //use putsChnRaw
  //input -> 1..4 -> sticks,  5..8 pots
  //MAX,FULL - disregard
  //ppm
  SKYCSwData &cs = g_model.customSw[cs_index];
  if(!cs.func) return false;

  int8_t a = cs.v1;
  int8_t b = cs.v2;
  int16_t x = 0;
  int16_t y = 0;
#ifndef TELEMETRY_LOST
	uint8_t valid = 1 ;
#endif
  // init values only if needed
  uint8_t s = CS_STATE(cs.func);

  if(s == CS_VOFS)
  {
      x = getValue(cs.v1-1);
#ifdef FRSKY
      if ( (cs.v1 > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1 < EXTRA_POTS_START ) )
			{
        y = convertTelemConstant( cs.v1-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
#ifndef TELEMETRY_LOST
				valid = telemItemValid( cs.v1-CHOUT_BASE-NUM_SKYCHNOUT-1 ) ;
#endif
			}
      else
#endif
      y = calc100toRESX(cs.v2);
  }
  else if(s == CS_VCOMP)
  {
      x = getValue(cs.v1-1);
      y = getValue(cs.v2-1);
  }

  switch (cs.func) {
  case (CS_VPOS):
      ret_value = (x>y);
      break;
  case (CS_VNEG):
      ret_value = (x<y) ;
      break;
  case (CS_APOS):
  {
      ret_value = (abs(x)>y) ;
  }
      break;
  case (CS_ANEG):
  {
      ret_value = (abs(x)<y) ;
  }
      break;
	case CS_EXEQUAL:
		if ( isAgvar( cs.v1 ) )
		{
			x *= 10 ;
			y *= 10 ;
		}
    ret_value = abs(x-y) < 32 ;
  break;
		
  case (CS_AND):
  case (CS_OR):
  case (CS_XOR):
  {
    bool res1 = getSwitch(a,0,level+1) ;
    bool res2 = getSwitch(b,0,level+1) ;
    if ( cs.func == CS_AND )
    {
      ret_value = res1 && res2 ;
    }
    else if ( cs.func == CS_OR )
    {
      ret_value = res1 || res2 ;
    }
    else  // CS_XOR
    {
      ret_value = res1 ^ res2 ;
    }
  }
  break;

  case (CS_EQUAL):
      ret_value = (x==y);
      break;
  case (CS_NEQUAL):
      ret_value = (x!=y);
      break;
  case (CS_GREATER):
      ret_value = (x>y);
      break;
  case (CS_LESS):
      ret_value = (x<y);
      break;
//  case (CS_EGREATER):
//      ret_value = (x>=y);
//      break;
//  case (CS_ELESS):
//      ret_value = (x<=y);
//      break;
  case (CS_NTIME):
		ret_value = CsTimer[cs_index] >= 0 ;
  break ;
	case (CS_TIME):
	{	
    ret_value = CsTimer[cs_index] >= 0 ;
		int8_t x = getAndSwitch( cs ) ;
		if ( x )
		{
		  if (getSwitch( x, 0, level+1) )
			{
				if ( ( Last_switch[cs_index] & 2 ) == 0 )
				{ // Triggering
					ret_value = 1 ;
				}	
			}
		}
	}
  break;
  case (CS_MONO):
  case (CS_RMONO):
    ret_value = CsTimer[cs_index] > 0 ;
  break ;
  
	case (CS_LATCH) :
  case (CS_FLIP) :
    ret_value = Last_switch[cs_index] & 1 ;
  break ;
  default:
      ret_value = false;
      break;
  }
#ifndef TELEMETRY_LOST
	if ( valid == 0 )			// Catch telemetry values not present
	{
     ret_value = false;
	}
#endif
	if ( ret_value )
	{
		int8_t x = getAndSwitch( cs ) ;
		if ( x )
		{
      ret_value = getSwitch( x, 0, level+1) ;
		}
	}
	if ( cs.func < CS_LATCH )
	{
		Last_switch[cs_index] = ret_value ;
	}
	return swtch>0 ? ret_value : !ret_value ;

}

void putsDblSizeName( uint8_t y )
{
	for(uint8_t i=0;i<sizeof(g_model.name);i++)
		lcd_putcAtt(FW*2+i*2*FW-i-2, y, g_model.name[i],DBLSIZE);
}


#if defined(PCBSKY) || defined(PCB9XT)
static uint16_t switches_states = 0 ;
static uint8_t trainer_state = 0 ;
#endif
#ifdef PCBX9D
#ifdef REV9E
uint32_t switches_states = 0 ;
uint8_t extSwitches_states = 0 ;
#else
uint16_t switches_states = 0 ;
#endif

#endif

int8_t getMovedSwitch()
{
	uint8_t skipping = 0 ;
  int8_t result = 0;

	static uint16_t s_last_time = 0;

	uint16_t time = get_tmr10ms() ;
  if ( (uint16_t)(time - s_last_time) > 10)
	{
		skipping = 1 ;
	}
  s_last_time = time ;

#if defined(PCBSKY) || defined(PCB9XT)
  uint16_t mask = 0xC000 ;
	uint16_t map = g_eeGeneral.switchMapping ;
	
	for ( uint8_t i=8 ; i>0 ; i-- )
	{
    uint8_t prev = (switches_states & mask) >> (i*2-2) ;
		uint8_t next ;
		uint8_t swtchIndex = i ;
		switch ( i )
		{
			case 8 :// Gear
				if ( map & USE_GEA_3POS )
				{
					next = switchPosition( HSW_Gear3pos0 ) ;
					swtchIndex = HSW_Gear3pos0 + next ;
				}
				else
				{
			  	next = getSwitch00(i) ;
				}
			break ;

			case 7 :// Ail
				if ( map & USE_AIL_3POS )
				{
					next = switchPosition( HSW_Ail3pos0 ) ;
					swtchIndex = HSW_Ail3pos0 + next ;
				}
				else
				{
			  	next = getSwitch00(i) ;
				}
			break ;

			case 6 :// ID2
			case 5 :// ID1
			case 4 :// ID0
			  next = getSwitch00(i) ;
			break ;
			 
			case 3 :// ELE
				if ( map & USE_ELE_3POS )
				{
					next = switchPosition( HSW_Ele3pos0 ) ;
					swtchIndex = HSW_Ele3pos0 + next ;
				}
				else if ( map & USE_ELE_6POS )
				{
					if ( switches_states & 0x80)
					{
						prev += 4 ;
					}
					next = switchPosition( HSW_Ele6pos0 ) ;
					swtchIndex = HSW_Ele6pos0 + next ;
				}
				else
				{
			  	next = getSwitch00(i) ;
				}
			break ;

			case 2 :// RUD
				if ( map & USE_RUD_3POS )
				{
					next = switchPosition( HSW_Rud3pos0 ) ;
					swtchIndex = HSW_Rud3pos0 + next ;
				}
				else
				{
			  	next = getSwitch00(i) ;
				}
			break ;
			
			case 1 :// THR
				if ( map & USE_THR_3POS )
				{
					next = switchPosition( HSW_Thr3pos0 ) ;
					swtchIndex = HSW_Thr3pos0 + next ;
				}
				else
				{
			  	next = getSwitch00(i) ;
				}
			break ;
		}
    if (prev != next)
		{
			if ( i == 3 )
			{
				if ( map & USE_ELE_6POS )
				{
      		switches_states = (switches_states & (~0x80)) | ((next > 3) ? 0x80 : 0);
					next &= 3 ;
				}
			}
//			if ( i == 4 )
//			{
//      	switches_states = (switches_states & (~0x40)) | ((next > 3) ? 1 : 0) ;
//			}
//			else
//			{
      	switches_states = (switches_states & (~mask)) | (next << (i*2-2));
//			}
			if ( swtchIndex > 8 )
			{
				result = swtchIndex ;				
			}
			else
			{
				if ( ( i>=4 ) && (i<=6) )
				{
					if ( next )
					{
						result = i ;
					}
				}
				else
				{
        	result = next ? i : -i ;
				}
			}
		}
		mask >>= 2 ;
  }
	if ( result == 0 )
	{
		mask = getSwitch00( 9 ) ;
		if ( ( mask ^ trainer_state ) & 1 )
		{
			if ( mask )
			{
				result = 9 ;
			}
			trainer_state ^= 1 ;
		}
	}
	if ( map & USE_PB1 )
	{
		if ( result == 0 )
		{
			mask = getSwitch00( HSW_Pb1 ) << 1 ;
			if ( ( mask ^ trainer_state ) & 2 )
			{
				if ( mask )
				{
					result = HSW_Pb1 ;
				}
				trainer_state ^= 2 ;
			}
		}
	}
	if ( map & USE_PB2 )
	{
		if ( result == 0 )
		{
			mask = getSwitch00( HSW_Pb2 ) << 2 ;
			if ( ( mask ^ trainer_state ) & 4 )
			{
				if ( mask )
				{
					result = HSW_Pb2 ;
				}
				trainer_state ^= 4 ;
			}
		}
	}
	if ( map & USE_PB3 )
	{
		if ( result == 0 )
		{
			mask = getSwitch00( HSW_Pb3 ) << 1 ;
			if ( ( mask ^ trainer_state ) & 8 )
			{
				if ( mask )
				{
					result = HSW_Pb3 ;
				}
				trainer_state ^= 8 ;
			}
		}
	}
	if ( map & USE_PB4 )
	{
		if ( result == 0 )
		{
			mask = getSwitch00( HSW_Pb4 ) << 1 ;
			if ( ( mask ^ trainer_state ) & 16 )
			{
				if ( mask )
				{
					result = HSW_Pb4 ;
				}
				trainer_state ^= 16 ;
			}
		}
	}
#endif
#ifdef PCBX9D
#ifdef REV9E
  for (uint8_t i=0 ; i<18 ; i += 1 )
	{
    uint32_t mask ;
    uint8_t prev ;
		
		if ( i < 16 )
		{
    	mask = (0x03 << (i*2)) ;
    	prev = (switches_states & mask) >> (i*2) ;
		}
		else
		{
    	mask = (0x03 << ((i-16)*2)) ;
    	prev = (extSwitches_states & mask) >> ((i-16)*2) ;
		}
		uint8_t next = switchPosition( i ) ;

    if (prev != next)
		{
			if ( i < 16 )
			{
      	switches_states = (switches_states & (~mask)) | (next << (i*2));
			}
			else
			{
      	extSwitches_states = (extSwitches_states & (~mask)) | (next << ((i-16)*2));
			}
      if (i<5)
        result = 1+(3*i)+next ;
      else if (i==5)
        result = 1+(3*5)+(next!=0) ;
      else if (i==6)
        result = 1+(3*5)+2+next ;
      else if (i==7)
        result = 1+(3*5)+2+3+(next!=0) ;
      else
        result = 1+(3*i)-4+next ;
    }
  }
#else // REV9E
  for (uint8_t i=0 ; i<8 ; i += 1 )
	{
    uint16_t mask = (0x03 << (i*2)) ;
    uint8_t prev = (switches_states & mask) >> (i*2) ;
		uint8_t next = switchPosition( i ) ;

    if (prev != next)
		{
      switches_states = (switches_states & (~mask)) | (next << (i*2));
      if (i<5)
        result = 1+(3*i)+next;
      else if (i==5)
			{
        result = 1+(3*5) ;
				if (next!=0) result = -result ;
			}
      else if (i==6)
        result = 1+(3*5)+1+next;
      else
			{
        result = 1+(3*5)+1+3 ;
				if (next!=0) result = -result ;
			}
    }
  }
#endif // REV9E
#endif

  if ( skipping )
    result = 0 ;

  return result ;
}

void checkQuickSelect()
{
  uint8_t i = keyDown(); //check for keystate
  uint8_t j;
  
	if ( ( i & 6 ) == 6 )
	{
		SystemOptions |= SYS_OPT_MUTE ;
		return ;
	}
	
	for(j=1; j<8; j++)
      if(i & (1<<j)) break;
  j--;

  if(j<6)
	{
#if defined(PCBSKY) || defined(PCB9XT)
    if(!eeModelExists(j))
#endif
#ifdef PCBX9D
    if(!eeModelExists(j))
#endif
			return ;
    if( g_eeGeneral.currModel != j )
		{
#if defined(PCBSKY) || defined(PCB9XT)
	    ee32LoadModel(g_eeGeneral.currModel = j);
#endif
#ifdef PCBX9D
	    eeLoadModel(g_eeGeneral.currModel = j);
#endif
	    STORE_GENERALVARS;
		}
    lcd_clear();
    lcd_putsAtt(64-7*FW,0*FH,PSTR(STR_LOADING),DBLSIZE);

		putsDblSizeName( 3*FH ) ;

    refreshDisplay();
    clearKeyEvents(); // wait for user to release key
  }
}

void alertMessages( const char * s, const char * t )
{
  lcd_clear();
  lcd_putsAtt(64-5*FW,0*FH,PSTR(STR_ALERT),DBLSIZE);
  lcd_puts_P(0,4*FH,s);
  lcd_puts_P(0,5*FH,t);
  lcd_puts_P(0,6*FH,  PSTR(STR_PRESS_KEY_SKIP) ) ;
//  lcdSetRefVolt(g_eeGeneral.contrast);
}


void alert(const char * s, bool defaults)
{
	if ( Main_running )
	{
#define MESS_TYPE		1

		AlertType = ALERT_TYPE ;
		AlertMessage = s ;
		return ;
	}

	almess( s, ALERT_TYPE ) ;

	lcdSetRefVolt(defaults ? 0x22 : g_eeGeneral.contrast);

	voiceSystemNameNumberAudio( SV_ERROR, V_ERROR, AU_WARNING2 ) ;
//  audioVoiceDefevent(AU_ERROR, V_ERROR | 0x1000);
  clearKeyEvents();
  while(1)
  {
#ifdef SIMU
    if (!main_thread_running) return;
    sleep(1/*ms*/);
#endif

    if(keyDown())
    {
	    clearKeyEvents();
      return;  //wait for key release
    }
    wdt_reset();
		
		if ( check_power_or_usb() ) return ;		// Usb on or power off
    
		if(getSwitch00(g_eeGeneral.lightSw) || getSwitch00(g_model.mlightSw) || g_eeGeneral.lightAutoOff || defaults)
      {BACKLIGHT_ON;}
    else
      {BACKLIGHT_OFF;}
  }
}

void message(const char * s)
{
	almess( s, MESS_TYPE ) ;
//  lcdSetRefVolt(g_eeGeneral.contrast);
}

//int16_t tanaIn( uint8_t chan )
//{
// 	int16_t v = anaIn(chan) ;
//	return  (g_eeGeneral.throttleReversed) ? -v : v ;
//}

uint8_t checkThrottlePosition()
{
  uint8_t thrchn=(2-(g_eeGeneral.stickMode&1));//stickMode=0123 -> thr=2121
	int16_t v = scaleAnalog( anaIn(thrchn), thrchn ) ;
	
//	if ( g_model.throttleIdle == 2 )
//	{
//  	if(v >= RESX - THRCHK_DEADBAND  )
//  	{
//  		return 1 ;
//  	}
//	}
//	else 
	if ( g_model.throttleIdle )
	{
		if ( abs( v ) < THRCHK_DEADBAND )
		{
			return 1 ;
		}
	}
	else
	{
  	if(v <= -RESX + THRCHK_DEADBAND )
  	{
  		return 1 ;
  	}
	}
	return 0 ;
}

void checkTHR()
{
  if(g_eeGeneral.disableThrottleWarning) return;

//  uint8_t thrchn=(2-(g_eeGeneral.stickMode&1));//stickMode=0123 -> thr=2121

//#ifdef SIMU
//  int16_t lowLim = THRCHK_DEADBAND - 1024 ;
//#else
#ifndef SIMU
  getADC_single();   // if thr is down - do not display warning at all
#endif
//	int16_t lowLim = g_eeGeneral.calibMid[thrchn] ;

//	lowLim = (( g_eeGeneral.throttleReversed || ( g_model.throttleIdle == 2 ) ) ? (- lowLim) - g_eeGeneral.calibSpanPos[thrchn] : lowLim - g_eeGeneral.calibSpanNeg[thrchn]);
//	if ( g_model.throttleIdle == 2 )
//	{
//		lowLim = -lowLim ;
//		lowLim -= THRCHK_DEADBAND ;
//	}
//	else
//	{
//		lowLim += THRCHK_DEADBAND ;
//	}
//#endif

	if ( checkThrottlePosition() )
	{
		return ;
	}

  // first - display warning
  alertMessages( PSTR(STR_THR_NOT_IDLE), PSTR(STR_RST_THROTTLE) ) ;
  refreshDisplay();
  clearKeyEvents();
	putSystemVoice( SV_TH_WARN, V_THR_WARN ) ;
//	audioVoiceDefevent( AU_WARNING1, V_ALERT ) ;
  
	//loop until throttle stick is low
  while (1)
  {
//		static uint32_t counter = 0 ;
#ifdef SIMU
      if (!main_thread_running) return;
      sleep(1/*ms*/);
#else
      getADC_single();
#endif
			check_backlight() ;

			if ( checkThrottlePosition() )
			{
				return ;
			}
      if( keyDown() )
      {
			  clearKeyEvents() ;
        return;
      }
      wdt_reset();
			CoTickDelay(1) ;					// 2mS for now

		if ( check_power_or_usb() ) return ;		// Usb on or power off

  }
}

int32_t readControl( uint8_t channel )
{
	int32_t value ;
	value = scaleAnalog( anaIn(channel), channel ) ;
	value *= 100 ;
	value /= 1024 ;
	return value ;
}
				 
void checkCustom()
{
	CustomCheckData *pdata ;
	pdata = &g_model.customCheck ;
	int32_t value ;
	uint32_t timer ;
	uint8_t idx = pdata->source - 1 ;
	if ( pdata->source == 0 )
	{
		return ;
	}
	if ( idx < 4 )
	{
		idx = stickScramble[g_eeGeneral.stickMode*4+idx] ;
	}

	
#ifndef SIMU
  getADC_osmp() ;
#endif
	
	value = readControl( idx ) ;
	if ( ( value >= pdata->min ) && ( value <= pdata->max ) )
	{
		return ;
	}

  clearKeyEvents();

  timer = 0 ;
  while (1)
  {
//		static uint32_t counter = 0 ;
#ifdef SIMU
    if (!main_thread_running) return;
    sleep(1/*ms*/);
#else
    getADC_single();
#endif
		check_backlight() ;

		value = readControl( idx ) ;
		if ( ( value >= pdata->min ) && ( value <= pdata->max ) )
		{
			if ( ++timer > 99 )
			{
				return ;
			}
		}
		else
		{
			timer = 0 ;
		}
	  alertMessages( XPSTR("Custom Check"), XPSTR("Set Control") ) ;
		putsChnRaw( 9*FW, 2*FH, pdata->source, 0 ) ;
		lcd_outdezAtt( 5*FW, 3*FH, pdata->min, 0) ;
		lcd_outdezAtt( 11*FW, 3*FH, value, 0) ;
		lcd_outdezAtt( 17*FW, 3*FH, pdata->max, 0) ;
		refreshDisplay();

    if( keyDown() )
    {
			clearKeyEvents() ;
      return;
    }
    wdt_reset();
		CoTickDelay(5) ;					// 10mS for now

		if ( check_power_or_usb() ) return ;		// Usb on or power off
  }
}


uint16_t oneSwitchText( uint8_t swtch, uint16_t states )
{
	uint8_t index = swtch - 1 ;
	uint8_t attr = 0 ;
#if defined(PCBSKY) || defined(PCB9XT)
	uint8_t sm = g_eeGeneral.switchMapping ;

	switch ( swtch )
	{
		case HSW_ThrCt :
			if ( sm & USE_THR_3POS )
			{
				index = HSW_Thr3pos0 - HSW_OFFSET ;
				if ( states & 0x0001 ) index += 1 ;
				if ( states & 0x0100 ) index += 2 ;
			}
			else
			{
				if (states & 0x0101) attr = INVERS ;
			}
		break ;

		case HSW_RuddDR :
			if ( sm & USE_RUD_3POS )
			{
				index = HSW_Rud3pos0 - HSW_OFFSET ;
				if ( states & 0x0002 ) index += 1 ;
				if ( states & 0x0200 ) index += 2 ;
			}
			else
			{
				if (states & 0x0202) attr = INVERS ;
			}
		break ;
	
		case HSW_ElevDR :
			if ( sm & USE_ELE_3POS )
			{
				index = HSW_Ele3pos0 - HSW_OFFSET ;
				if ( states & 0x0004 ) index += 1 ;
				if ( states & 0x0400 ) index += 2 ;
			}
			else if ( sm & USE_ELE_6POS )
			{
				index = HSW_Ele6pos0 - HSW_OFFSET ;
				if ( states & 0x0004 ) index += 1 ;
				if ( states & 0x0400 ) index += 2 ;
				if ( states & 0x0800 ) index += 4 ;
//lcd_outhex4( 25, 3*FH, index ) ;
			}
			else
			{
				if (states & 0x0C04) attr = INVERS ;
			}
		break ;

		case HSW_ID0 :
			if ( states & 0x10 )
			{
				index += 1 ;
			}
			if ( states & 0x20 )
			{
				index += 2 ;
			}
		break ;

		case HSW_AileDR :
			if ( sm & USE_AIL_3POS )
			{
				index = HSW_Ail3pos0 - HSW_OFFSET ;
				if ( states & 0x0040 ) index += 1 ;
				if ( states & 0x1000 ) index += 2 ;
			}
			else
			{
				if (states & 0x1040) attr = INVERS ;
			}
		break ;
	 
		case HSW_Gear :
			if ( sm & USE_GEA_3POS )
			{
				index = HSW_Gear3pos0 - HSW_OFFSET ;
				if ( states & 0x0080 ) index += 1 ;
				if ( states & 0x2000 ) index += 2 ;
			}
			else
			{
				if (states & 0x2080) attr = INVERS ;
			}
		break ;

	}
	
#endif	 
	return (attr << 8 ) | index ;
}


void putWarnSwitch( uint8_t x, uint8_t idx )
{
	if ( ( idx == 2 ) || ( idx == 1 ) )
	{
		idx = oneSwitchText( idx+1, getCurrentSwitchStates() ) ;
	}
  lcd_putsAttIdx( x, 2*FH, PSTR(SWITCHES_STR), idx, 0) ;
}

uint16_t getCurrentSwitchStates()
{
  uint16_t i = 0 ;
	getMovedSwitch() ;
	i = switches_states & 1 ;
	if ( switches_states & 2 )			i |= 0x0100 ;
	if ( switches_states & 4 )			i |= 0x0002 ;
	if ( switches_states & 8 )			i |= 0x0200 ;
	if ( switches_states & 0x10 ) 	i |= 0x0004 ;
	if ( switches_states & 0x20 ) 	i |= 0x0400 ;
	if ( switches_states & 0x40 ) 	i |= 0x0008 ;
	if ( switches_states & 0x80 ) 	i |= 0x0800 ;
	if ( switches_states & 0x100 )  i |= 0x0010 ;
	if ( switches_states & 0x400 )  i |= 0x0020 ;
	if ( switches_states & 0x1000 ) i |= 0x0040 ;
	if ( switches_states & 0x2000 ) i |= 0x1000 ;
	if ( switches_states & 0x4000 ) i |= 0x0080 ;
	if ( switches_states & 0x8000 ) i |= 0x2000 ;

//  for( uint8_t j=0; j<8; j++ )
//  {
//    bool t=keyState( (EnumKeys)(SW_BASE_DIAG+7-j) ) ;
//		i <<= 1 ;
//    i |= t ;
//  }
	return i ;
}

void checkSwitches()
{
	uint16_t warningStates ;

#ifdef PCB9XT
		read_adc() ; // needed for 3/6 pos ELE switch
		processAnalogSwitches() ;
		processAnalogSwitches() ;		// Make sure the values are processed at startup.
#endif // PCB9XT
	 
	warningStates = g_model.modelswitchWarningStates ;
  
	if( warningStates & 1 ) return ; // if warning is on
	warningStates >>= 1 ;

#if defined(PCBSKY) || defined(PCB9XT)
	uint8_t x = warningStates & SWP_IL5;
  if(x==SWP_IL1 || x==SWP_IL2 || x==SWP_IL3 || x==SWP_IL4 || x==SWP_IL5) //illegal states for ID0/1/2
  {
    warningStates &= ~SWP_IL5; // turn all off, make sure only one is on
    warningStates |=  SWP_ID0B;
		g_model.modelswitchWarningStates = (warningStates << 1) ;
  }
#endif
	
#if defined(PCBSKY) || defined(PCB9XT)
	uint8_t first = 1 ;
#endif
	//loop until all switches are reset

 	warningStates &= ~g_model.modelswitchWarningDisables ;
	while (1)
  {
#if defined(PCBSKY) || defined(PCB9XT)
		read_adc() ; // needed for 3/6 pos ELE switch
#ifdef PCB9XT
		processAnalogSwitches() ;
#endif // PCB9XT

    uint16_t i = getCurrentSwitchStates() ;
		i &= ~g_model.modelswitchWarningDisables ;

		if ( first )
		{
 			clearKeyEvents();
			first = 0 ;
			if( i != warningStates )
			{
				putSystemVoice( SV_SW_WARN, V_SW_WARN ) ;
//				audioVoiceDefevent( AU_WARNING1, V_ALERT ) ;
			}
		}

    if( (i==warningStates) || (keyDown())) // check state against settings
    {
        return;  //wait for key release
    }

        //show the difference between i and switch?
        //show just the offending switches.
        //first row - THR, GEA, AIL, ELE, ID0/1/2
        uint16_t x = i ^ warningStates ;

		    alertMessages( PSTR(STR_SWITCH_WARN), PSTR(STR_RESET_SWITCHES) ) ;
//				lcd_outhex4( 0, 0, g_model.modelswitchWarningStates ) ;
//				lcd_outhex4( 25, 0, i ) ;
//				lcd_outhex4( 50, 0, warningStates ) ;
//				lcd_outhex4( 75, 0, AnalogSwitches ) ;
//        lcd_putsnAtt(0*FW, 2*FH, PSTR("                      "), 22, 0);

        if(x & SWP_THRB)
            putWarnSwitch(2 + 0*FW, 0 );
        if(x & 0x0202)
            putWarnSwitch(2 + 3*FW + FW/2, 1 );
        if(x & 0x0C04)
            putWarnSwitch(2 + 7*FW, 2 );

        if(x & SWP_IL5)
        {
            if(i & SWP_ID0B)
                putWarnSwitch(2 + 10*FW + FW/2, 3 );
            if(i & SWP_ID1B)
                putWarnSwitch(2 + 10*FW + FW/2, 4 );
            if(i & SWP_ID2B)
                putWarnSwitch(2 + 10*FW + FW/2, 5 );
        }

        if(x & 0x1040)
            putWarnSwitch(2 + 14*FW, 6 );
        if(x & 0x2080)
            putWarnSwitch(2 + 17*FW + FW/2, 7 );

//lcd_outhex4( 0, 3*FH, i ) ;
//lcd_outhex4( 25, 3*FH, warningStates ) ;

#endif
#ifdef PCBX9D
// To Do
  		getMovedSwitch() ;	// loads switches_states

    	uint16_t ss = switches_states ;
			ss &= ~g_model.modelswitchWarningDisables ;

			if ( ( ss & 0x3FFF ) == warningStates )
			{
				return ;
			}
#ifdef PCBX9D
// To Do
		if ( keyDown() )
		{
	    clearKeyEvents();
			return ;
		}
#endif
 		alertMessages( PSTR(STR_SWITCH_WARN), PSTR(STR_RESET_SWITCHES) ) ;
 		for ( uint8_t i = 0 ; i < 7 ; i += 1 )
		{
 		  uint16_t mask = ( 0x03 << (i*2) ) ;
 		  uint8_t attr = ((warningStates & mask) == (ss & mask)) ? 0 : INVERS ;
			if ( ~g_model.modelswitchWarningDisables & mask )
			{
  		  lcd_putcAtt( 3*FW+i*(2*FW+2), 2*FH, 'A'+i, attr ) ;
				lcd_putcAtt( 4*FW+i*(2*FW+2), 2*FH, PSTR(HW_SWITCHARROW_STR)[(warningStates & mask) >> (i*2)], attr ) ;
			}
		}
#endif
    refreshDisplay() ;


    wdt_reset();

		if ( check_power_or_usb() ) return ;		// Usb on or power off

		check_backlight() ;

#ifdef SIMU
    if (!main_thread_running) return;
    sleep(1/*ms*/);
#endif
  }
}


MenuFuncP lastPopMenu()
{
  return  g_menuStack[g_menuStackPtr+1];
}

void popMenu(bool uppermost)
{
  if(g_menuStackPtr>0 || uppermost)
	{
    g_menuStackPtr = uppermost ? 0 : g_menuStackPtr-1;
 		EnterMenu = EVT_ENTRY_UP ;
  }else{
    alert(PSTR(STR_MSTACK_UFLOW));
  }
}

void chainMenu(MenuFuncP newMenu)
{
  g_menuStack[g_menuStackPtr] = newMenu;
	EnterMenu = EVT_ENTRY ;
}

void pushMenu(MenuFuncP newMenu)
{

  if(g_menuStackPtr >= DIM(g_menuStack)-1)
  {
    alert(PSTR(STR_MSTACK_OFLOW));
    return;
  }
	EnterMenu = EVT_ENTRY ;
  g_menuStack[++g_menuStackPtr] = newMenu ;
}

uint8_t *ncpystr( uint8_t *dest, uint8_t *source, uint8_t count )
{
  while ( (*dest++ = *source++) )
	{
		if ( --count == 0 )
		{
			*dest++ = '\0' ;
			break ;
		}
	}	
  return dest - 1 ;
}

uint8_t *cpystr( uint8_t *dest, uint8_t *source )
{
  while ( (*dest++ = *source++) )
    ;
  return dest - 1 ;
}

#if GVARS
int8_t REG100_100(int8_t x)
{
	return REG( x, -100, 100 ) ;
}

int8_t REG(int8_t x, int8_t min, int8_t max)
{
  int8_t result = x;
  if (x >= 126 || x <= -126) {
    x = (uint8_t)x - 126;
    result = g_model.gvars[x].gvar ;
    if (result < min) {
      g_model.gvars[x].gvar = result = min;
//      eeDirty( EE_MODEL | EE_TRIM ) ;
    }
    if (result > max) {
      g_model.gvars[x].gvar = result = max;
//      eeDirty( EE_MODEL | EE_TRIM ) ;
    }
  }
  return result;
}
#endif

uint8_t IS_EXPO_THROTTLE( uint8_t x )
{
	if ( g_model.thrExpo )
	{
		return IS_THROTTLE( x ) ;
	}
	return 0 ;
}

uint8_t IS_THROTTLE( uint8_t x )
{
//#ifdef FIX_MODE
	return x == 2 ;
//#else
//	uint8_t y ;
//	y = g_eeGeneral.stickMode&1 ;
//	y = 2 - y ;
//	return (((y) == x) && (x<4)) ;
//#endif
}

void checkXyCurve()
{
	if ( g_model.curvexy[9] == 0 )
	{
		uint32_t i ;
		int8_t j = -100 ;
		for ( i = 9 ; i < 18 ; j += 25, i += 1 )
		{
			g_model.curvexy[i] = j ;
		}
	}
}

/*** EOF ***/

