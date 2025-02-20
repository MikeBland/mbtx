
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
//#include <ctype.h>
#ifdef ARUNI
#include <string.h>
#endif
#ifdef PCBSKY
#include "AT91SAM3S4.h"
#ifndef SIMU
#include "core_cm3.h"
#endif
#endif

//#define LATENCY 1

#include "ersky9x.h"
#include "myeeprom.h"
#include "trims.h"
#include "audio.h"
#include "sound.h"
#include "lcd.h"
#include "drivers.h"

#ifdef PCBSKY
#include "file.h"
#endif

#include "menus.h"
#include "mixer.h"
#include "timers.h"
#if defined(PCBX12D) || defined(PCBX10) || defined(REV19)
#include "X12D/stm32f4xx_gpio.h"
#endif
#ifdef PCBLEM1
#include "stm103/logicio103.h"
#else
#include "logicio.h"
#endif
#include "pulses.h"
#include "stringidx.h"


#include "frsky.h"

#ifdef PCBLEM1
#include <stm32f10x.h>
#include "stm103/hal.h"
#include "stm103/stm32_sdio_sd.h"
#include "stm103/i2c103.h"
#include "diskio.h"
#include "file.h"
#include "analog.h"
#endif
#ifdef PCBX9D
#include "analog.h"
#include "diskio.h"
#include "X9D/eeprom_rlc.h"
 #ifdef REV19
#include "X12D/stm32f4xx.h"
//#include "X12D/stm32f4xx_gpio.h"
#include "X12D/stm32f4xx_rcc.h"
#include "X12D/usb_dcd_int.h"
#include "X12D/usb_bsp.h"
#include "X12D/usbd_conf.h"
 #else
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/stm32f2xx_rcc.h"
#include "X9D/usb_dcd_int.h"
#include "X9D/usb_bsp.h"
#include "X9D/usbd_conf.h"
 #endif
#include "X9D/hal.h"
#include "X9D/i2c_ee.h"


extern "C" uint8_t USBD_HID_SendReport(USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len) ;

#endif // PCBX9D

#include "gvars.h"

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

#ifdef  LUA
#include "lua/lua_api.h"
#endif
#ifdef  BASIC
#include "basic/basic.h"
#endif

#ifdef INPUTS
extern const uint8_t IconInput[] ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
#include "analog.h"
#include "diskio.h"
#include "X12D/stm32f4xx_rcc.h"
#include "X12D/i2c_ee.h"
#include "X12D/hal.h"
#include "X12D/sdio_sd.h"
#include "X12D/usb_dcd_int.h"
#include "X12D/usb_bsp.h"
#include "X12D/usbd_conf.h"

#ifdef TOUCH
#include "X12D/tp_gt911.h"

#endif

extern "C" uint8_t USBD_HID_SendReport(USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len) ;

uint32_t ee32_check_finished()
{
	return 1 ;
}
extern uint32_t loadModelImage( void ) ;
extern void ee32_process( void ) ;
#endif

#ifdef PCBLEM1
uint32_t ee32_check_finished()
{
	return 1 ;
}
#endif


uint16_t MainStart ;

#include "sbus.h"

#include "ff.h"
#include "maintenance.h"


#ifndef SIMU
#include "CoOS.h"
#endif

#include "../../common/hand.lbm"

#ifdef BLUETOOTH
#include "bluetooth.h"
#endif

#ifdef TOUCH
extern void lcdDrawIcon( uint16_t x, uint16_t y, const uint8_t * bitmap, uint8_t type ) ;

const uint8_t IconHplus[] =
{
#if defined(PCBT18)
#include "X12D/IconHplus.lbm"
#else
#include "X12D/IconHplusinv.lbm"
#endif
} ;
const uint8_t IconHminus[] =
{
#if defined(PCBT18)
#include "X12D/IconHminus.lbm"
#else
#include "X12D/IconHminusinv.lbm"
#endif
} ;
const uint8_t IconH2plus[] =
{
#if defined(PCBT18)
#include "X12D/IconH2plus.lbm"
#else
#include "X12D/IconH2plusinv.lbm"
#endif
} ;
const uint8_t IconH2minus[] =
{
#if defined(PCBT18)
#include "X12D/IconHdminus.lbm"
#else
#include "X12D/IconHdminusinv.lbm"
#endif
} ;
const uint8_t IconHtoggle[] =
{
#if defined(PCBT18)
#include "X12D/IconHtoggle.lbm"
#else
#include "X12D/IconHtoggleinv.lbm"
#endif
} ;
extern const uint8_t IconHedit[] =
{
#if defined(PCBT18)
#include "X12D/IconHedit.lbm"
#else
#include "X12D/IconHeditinv.lbm"
#endif
} ;
#endif

#ifdef USE_VARS
#include "vars.h"
#endif

//#define PCB_TEST_9XT	1
//#define SERIAL_TEST_PRO	1

//#ifdef PCBXLITE
//#define WHERE_DEBUG	1
//#endif

#ifdef PCB9XT
//#define WHERE_DEBUG	1
//#define WHERE_STORE	1
#endif
//#define STARTUP_DEBUG 1
//#define STACK_PROBES	1

#if defined(PCBX12D) || defined(PCBX10)
//#define	WHERE_TRACK		1
//struct t_where
//{
//	uint32_t index ;
//	uint32_t count ;
//	uint8_t data[4000] ;
//	uint32_t stack[100] ;
//} ;

//struct t_where WhereTrack __CCM ;


//FIL g_oPosFile = {0};

//void initPosition()
//{
//	struct t_where *p = &WhereTrack ;
//	p->index = 0 ;
//	p->count = 0 ;
//}

//void notePosition( uint8_t byte )
//{
//	struct t_where *p = &WhereTrack ;
//	if ( p->index > 4000 )
//	{
//		p->index = 0 ;
//	}
//	if ( p->count > 4000 )
//	{
//		p->count = 4000 ;
//	}
//	p->data[p->index++] = byte ;
//	p->count += 1 ;
//	if ( p->count > 4000 )
//	{
//		p->count = 4000 ;
//	}
//}

//void dumpStack( uint32_t *ptr )
//{
//	uint32_t i ;
//	uint32_t *q ;
//	struct t_where *p = &WhereTrack ;
//	q = p->stack ;
//	*q++ = (uint32_t) ptr ;
//	for ( i = 0 ; i < 50 ; i += 1 )
//	{
//		*q++ = *ptr++ ;
//	}
//}


//void dumpPositions()
//{
//  FRESULT result ;
//	uint32_t start ;
//	uint32_t width ;
//	uint32_t lcount ;
//	uint8_t byte ;
//	struct t_where *p = &WhereTrack ;
//  result = f_open(&g_oPosFile, "/pos.txt", FA_OPEN_ALWAYS | FA_WRITE) ;

//	lcount = p->count ;
//	if ( lcount >= 4000 )
//	{
//		lcount = 4000 ;
//	}
//	if ( lcount == 4000 )
//	{
//		start = p->index ;
//		if ( start >= 4000 )
//		{
//			start = 0 ;
//		}
//	}
//	else
//	{
//		start = 0 ;
//	}

//	width = 0 ;
//	while ( lcount )
//	{
//		byte = p->data[start++] ;
//		f_putc ( byte, &g_oPosFile ) ;
//		if ( ++width > 79 )
//		{
//			width = 0 ;
//			f_putc ( '\r', &g_oPosFile ) ;
//			f_putc ( '\n', &g_oPosFile ) ;
//		}
//		lcount -= 1 ;
//		if ( start >= 4000 )
//		{
//			start = 0 ;
//		}
//	}

////	for ( lcount = 0 ; lcount < 50 ; lcount += 1 )
////	{
////		f_printf(&g_oPosFile, "\n%08lX", p->stack[lcount] ) ;
////	}
 
//  f_close(&g_oPosFile) ;
//}

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

#if defined(REV19)
 #define PAGE_NAVIGATION 1
#endif // REV19


#ifndef SIMU
#ifdef BLUETOOTH
#define BT_STACK_SIZE			(160 + STACK_EXTRA)
#endif
#define LOG_STACK_SIZE		(350 + STACK_EXTRA)
#define DEBUG_STACK_SIZE	(300 + STACK_EXTRA)
#define VOICE_STACK_SIZE	(130+200 + STACK_EXTRA)

//#ifdef PCBSKY
#define CHECKRSSI		1
//#endif
//#ifdef PCB9XT
//#define CHECKRSSI		1
//#endif

#ifdef MIXER_TASK
#define MIXER_STACK_SIZE		(250 + STACK_EXTRA)

OS_TID MixerTask ;
OS_STK Mixer_stk[MIXER_STACK_SIZE] __attribute__ ((aligned (4))) ;

#endif

OS_TID MainTask;
//#if defined(PCBX12D) || defined(PCBX10)
//extern OS_STK main_stk[MAIN_STACK_SIZE] ;
//#else
OS_STK main_stk[MAIN_STACK_SIZE] __attribute__ ((aligned (8))) ;
//#endif

#ifdef BLUETOOTH
OS_TID BtTask;
OS_STK Bt_stk[BT_STACK_SIZE] __attribute__ ((aligned (4))) ;
#endif
OS_TID LogTask;
OS_STK Log_stk[LOG_STACK_SIZE] __attribute__ ((aligned (4))) ;
OS_TID VoiceTask;
OS_STK voice_stk[VOICE_STACK_SIZE] __attribute__ ((aligned (4))) ;

#ifdef	DEBUG
OS_TID DebugTask;
OS_STK debug_stk[DEBUG_STACK_SIZE] __attribute__ ((aligned (4))) ;
#endif

#ifdef SERIAL_HOST
void host( void* pdata ) ;
#define HOST_STACK_SIZE	300
OS_TID HostTask ;
OS_STK Host_stk[HOST_STACK_SIZE] ;
uint8_t Host10ms ;

#endif

#endif

#ifdef PCBLEM1
uint16_t PortE0 ;
uint16_t PortE1 ;
uint16_t PortB0 ;
uint16_t PortB1 ;
uint16_t PortD0 ;
uint16_t PortD1 ;
uint16_t PortC0 ;
uint16_t PortC1 ;
#endif

uint16_t LastRotEvent ;
uint16_t RotencSpeed ;
uint16_t RotencCount ;

#ifdef TOUCH
t_touchControl TouchControl ;
#endif

#ifdef PCBX9LITE
void ledBlue( void ) ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
uint8_t LastShotSwitch ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
extern "C" void HardFault_Handler(void)
{
	for(;;)
	{
		RTC->BKP1R = 0x0100 ;
	}
}

extern "C" void BusFault_Handler(void)
{
	for(;;)
	{
		RTC->BKP1R = 0x0101 ;
	}
}

extern "C" void Xdefault_Handler(void)
{
	for(;;)
	{
		RTC->BKP1R = 0x0102 ;
	}
}
#endif

#ifdef USB_JOYSTICK
extern "C" void startJoystick(void) ;
extern "C" uint8_t HIDDJoystickDriver_Change( uint8_t *data ) ;
extern "C" void USBD_Connect(void) ;
extern "C" void USBD_Disconnect(void) ;
#endif

void processSwitches( void ) ;
uint32_t check_power_or_usb( void ) ;

#ifdef BASIC
uint8_t ScriptFlags ;
#endif

extern uint8_t TrainerPolarity ;

uint32_t IdleCount ;
uint32_t IdlePercent ;
uint32_t BasicExecTime ;

#ifdef PCBSKY
uint8_t HwDelayScale = 1 ;
#endif

//uint8_t UserTimer1 ;

//#define SLAVE_RESET	1

uint8_t toupper(unsigned char c)
{
	if (((c)>='a')&&((c)<='z'))
	{
		c -= 0x20 ;
	}
	return c ;
}


#ifdef SLAVE_RESET
uint8_t SlaveResetSwitch ;
uint8_t SlavePanicSwitch ;
void panicDebugMenu() ;

extern uint8_t SlaveTempReceiveBuffer[] ;
extern uint8_t RemData[] ;


void menuProcPanic( uint8_t event )
{
	TITLE( XPSTR( "Panic" )) ;
	static MState2 mstate2;
	event = mstate2.check_columns( event, 0 ) ;
	uint8_t *p	= SlaveTempReceiveBuffer ;
	uint16_t x ;
	uint32_t i ;
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x <<= 8 ;
		x |= *p++ ;
		PUT_HEX4( i*25, 1*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x <<= 8 ;
		x |= *p++ ;
		PUT_HEX4( i*25, 2*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x <<= 8 ;
		x |= *p++ ;
		PUT_HEX4( i*25, 3*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x <<= 8 ;
		x |= *p++ ;
		PUT_HEX4( i*25, 4*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x <<= 8 ;
		x |= *p++ ;
		PUT_HEX4( i*25, 5*FH, x ) ;
	}
	p = RemData ;
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x <<= 8 ;
		x |= *p++ ;
		PUT_HEX4( i*25, 6*FH, x ) ;
	}
	for ( i = 0 ; i < 5 ; i += 1 )
	{
		x = *p++ ;
		x <<= 8 ;
		x |= *p++ ;
		PUT_HEX4( i*25, 7*FH, x ) ;
	}
	
}
#endif

const char * const *Language = English ;

const uint8_t splashdata[] = { 'S','P','S',0,
#ifdef PCBX9LITE
#include "sTxsplashFr.lbm"
#else
#include "sTxsplash.lbm"
#endif	
	'S','P','E',0};

#include "debug.h"

t_time Time ;

uint8_t unexpectedShutdown = 0;
uint8_t SdMounted = 0;
uint8_t SectorsPerCluster ;
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
uint8_t CurrentTrainerSource ;
#endif
uint8_t HardwareMenuEnabled = 0 ;

#define SW_STACK_SIZE	6
uint8_t Last_switch[NUM_SKYCSW] ;
uint8_t Now_switch[NUM_SKYCSW] ;
int16_t CsTimer_lastVal[NUM_SKYCSW] ;
int8_t SwitchStack[SW_STACK_SIZE] ;

uint8_t MuteTimer ;

uint8_t NumExtraPots ;
#ifdef ARUNI
uint8_t ExtraPotBits;
#endif

uint16_t AnalogData[ANALOG_DATA_SIZE] ;

#if defined(PCBX12D) || defined(PCBX10)
//extern uint8_t PictureDrawn ;
//extern void updatePicture( void ) ;
#endif

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

//uint16_t MixerRunAtTime ;
void runMixer( void ) ;

volatile int32_t Rotary_position ;
volatile int32_t Rotary_count ;
int32_t LastRotaryValue ;
int8_t Pre_Rotary_diff[4] ;
int32_t Rotary_diff ;
uint8_t Vs_state[NUM_SKYCHNOUT+NUM_VOICE+EXTRA_SKYCHANNELS] ;

struct t_NvsControl
{
	uint8_t nvs_state ;
	uint8_t nvs_delay ;
	int16_t nvs_timer ;
	int16_t nvs_last_value ;
} NvsControl[NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS + NUM_GLOBAL_VOICE_ALARMS] ;
uint8_t CurrentVolume ;
uint8_t HoldVolume ;
int8_t RotaryControl ;
uint8_t ppmInValid = 0 ;
uint8_t Activated = 0 ;
uint8_t Tevent ;
extern uint16_t SbusTimer ;

struct t_MusicSwitches MusicSwitches ;

void mixer_loop(void* pdata) ;
void log_task(void* pdata) ;
void main_loop( void* pdata ) ;
void mainSequence( uint32_t no_menu ) ;
void doSplash( void ) ;
void perMain( uint32_t no_menu ) ;
static void processVoiceAlarms( void ) ;
#ifdef PCBSKY
void UART_Configure( uint32_t baudrate, uint32_t masterClock) ;
#endif
void txmit( uint8_t c ) ;
void uputs( char *string ) ;
#if not (defined(PCBX10))
uint16_t rxCom2( void ) ;
#endif

#ifdef PCB_TEST_9XT
void test_loop( void* pdata ) ;
#endif

#ifdef PCBSKY
extern "C" void TC2_IRQHandler( void ) ;
#ifdef	DEBUG
void handle_serial( void* pdata ) ;
#endif
#endif

#ifdef PCB9XT
#ifdef	DEBUG
void handle_serial( void* pdata ) ;
#endif
#endif

#if defined(PCBX12D) || defined(PCBX10)
#ifdef	DEBUG
void handle_serial( void* pdata ) ;
#endif
#endif

#ifdef ARUNI
uint8_t SixPosCaptured;
uint8_t SixPosValue;
uint8_t SixPosDelay;
#elif defined(PCBSKY) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX9D) || defined(PCBX10)

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

#ifdef PCBX9D
void check6pos( void ) ;
#endif

void check_backlight( void ) ;
#if defined(PCBSKY) || defined(PCB9XT)
void checkQuickSelect( void ) ;
#endif

//void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att) ;
//void putsChn( coord_t x, coord_t y,uint8_t idx1, LcdFlags att) ;
//void putsDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att) ;//, bool nc) ;
const char *get_switches_string( void ) ;
bool getSwitch(int8_t swtch, bool nc, uint8_t level) ;
#if defined(PCBX12D) || defined(PCBX10)
extern "C" void init_soft_power( void ) ;
#else
void init_soft_power( void ) ;
#endif
uint32_t check_soft_power( void ) ;
void soft_power_off( void ) ;
int16_t getGvarSourceValue( uint8_t src ) ;
static void	processAdjusters( void ) ;

#ifdef PCBSKY
#if defined(SIMU)
  #define init_rotary_encoder()
#else
  static void init_rotary_encoder( void ) ;
#endif
#endif

#ifdef PCBX7
 #ifndef PCBT12
  static void init_rotary_encoder( void ) ;
 #endif
#endif // PCBX7
#ifdef REV9E
  static void init_rotary_encoder( void ) ;
#endif // REV9E
#if defined(PCBX12D) || defined(PCBX10)
  void init_rotary_encoder( void ) ;
#endif // PCBX12D

#ifdef PCBX9LITE
  static void init_rotary_encoder( void ) ;
#endif // PCBX9LITE

#ifdef REV19
  static void init_rotary_encoder( void ) ;
#endif // REV19

/*=========================================================================*/
/*  DEFINE: Definition of all local Data                                   */
/*=========================================================================*/

uint32_t Master_frequency ;
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

int16_t g_ppmIns[16];
uint8_t ppmInState = 0; //0=unsync 1..8= wait for value i-1
uint8_t Main_running ;
#ifdef PCBSKY
uint8_t CoProcAlerted ;
#endif
uint8_t LastVoiceFlushSwitch ;
int main( void ) ;

EEGeneral  g_eeGeneral;
//SKYModelData  g_model;
//ProtocolData Protocols[2] ;

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
uint8_t VoiceTimer = 10 ;		// Units of 10 mS
uint8_t VoiceCheckFlag100mS = 0 ;
//uint8_t CheckFlag50mS = 0 ;
uint8_t CheckFlag20mS = 0 ;
uint8_t CheckTimer = 2 ;
uint8_t DsmCheckTimer = 50 ;		// Units of 10 mS
uint8_t DsmCheckFlag = 0 ;

//const char *Str_OFF = PSTR(STR_OFF) ;
//const char *Str_ON = PSTR(STR_ON) ;



const char stickScramble[]= {
    0, 1, 2, 3,
    0, 2, 1, 3,
    3, 1, 2, 0,
    3, 2, 1, 0 };

uint8_t modeFixValue( uint8_t value )
{
	return stickScramble[g_eeGeneral.stickMode*4+value]+1 ;
}

#if defined(PCBX12D) || defined(PCBX10)
 #ifndef MIXER_TASK
void checkRunMixer()
{
	uint16_t wasRunAt ;
 	if ( ((uint16_t)getTmr2MHz() - MixerRunAtTime ) > 4000 )
 	{
		wasRunAt = MixerRunAtTime ;
 		runMixer() ;
		MixerRunAtTime = wasRunAt + 4000 ;
 	}
}
 #endif
#endif

#if defined(PCBX12D) || defined(PCBX10)
MenuFuncP g_menuStack[8];
#else
MenuFuncP g_menuStack[6];
#endif

uint8_t  g_menuStackPtr = 0;
uint8_t  EnterMenu = 0 ;


// Temporary to allow compile
uint8_t g_vbat100mV ;//= 98 ;
uint8_t heartbeat ;
uint8_t heartbeat_running ;
#if defined(PCBX12D) || defined(PCBX10)
uint32_t HbeatCounter ;
#endif

#ifdef PCBSKY
uint16_t ResetReason ;
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
uint32_t ResetReason ;
#endif
uint32_t ChipId ;

//#ifdef REV9E
//#define X9D_OFFSET		42
//uint16_t ProgressCounter ;
//extern uint8_t PowerState ;
//void progress( uint32_t x )
//{
//  wdt_reset();
//	g_LightOffCounter = 2000 ;
//	lcd_clear() ;
//	PUT_HEX4( 0, 0, x ) ;
//	PUT_HEX4( 0, FH, ++ProgressCounter ) ;
//	PUT_HEX4( 0, 2*FH, CurrentVolume ) ;
//	PUT_HEX4( 212-X9D_OFFSET, 0, x ) ;
//	PUT_HEX4( 212-X9D_OFFSET, FH, ++ProgressCounter ) ;
//	PUT_HEX4( 212-X9D_OFFSET, 2*FH, PowerState ) ;
//	PUT_HEX4( 212-X9D_OFFSET, 3*FH, check_soft_power() ) ;
//extern uint16_t SuCount ;
//	PUT_HEX4( 212-X9D_OFFSET, 4*FH, SuCount ) ;

//	PUT_HEX4( 50, 1*FH, GPIOD->MODER ) ;
//	PUT_HEX4( 50, 2*FH, GPIOD->IDR ) ;
//	PUT_HEX4( 50, 3*FH, GPIOD->ODR ) ;

//	PUT_HEX4( 90, 1*FH, DAC->CR ) ;
//	PUT_HEX4( 90, 2*FH, DMA1_Stream5->M0AR ) ;
//	PUT_HEX4( 90, 3*FH, TIM6->CNT ) ;
//	PUT_HEX4( 90, 4*FH, TIM6->ARR ) ;

//	PUT_HEX4( 212-X9D_OFFSET, 6*FH, GPIOE->MODER ) ;
//	PUT_HEX4( 212-X9D_OFFSET, 7*FH, GPIOE->AFR[0]>>16 ) ;

//	uint32_t i ;
//	for ( i = 0 ; i < 200 ; i += 1 )
//	{
//	  wdt_reset();
//		PUT_HEX4( 0, 7*FH, i ) ;
//		refreshDisplay() ;
//	}
//}
//#endif


#if defined(LUA) || defined(BASIC)
uint32_t mainScreenDisplaying()
{
	return g_menuStack[g_menuStackPtr] == menuProc0 ;
}
#endif

void usbJoystickUpdate(void) ;

#if defined(PCB9XT) || defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
static bool usbPlugged(void)
{
  return GPIO_ReadInputDataBit(GPIOA, PIN_FS_VBUS);
}
#endif

#ifdef PCBSKY
#ifdef USB_JOYSTICK
static bool usbPlugged(void)
{
	return PIOC->PIO_PDSR & PIO_PC25 ;
}
#endif
#endif

void handleUsbConnection()
{
#if (defined(PCBX9D) || defined(PCB9XT)) && !defined(SIMU) || defined(PCBX12D) || defined(PCBX10)
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

  }
  if (usbStarted && !usbPlugged())
	{
    usbStarted = false;
  }
  
  if (usbStarted )
	{
    usbJoystickUpdate();
  }
  
#endif //#if defined(PCB9XT) && !defined(SIMU)

#ifdef PCBSKY
#ifdef USB_JOYSTICK
  static bool usbStarted = false;

  if (!usbStarted && usbPlugged())
	{
    usbStarted = true ;
	  WatchdogTimeout = 200 ;
		USBD_Connect() ;
  }
  if (usbStarted && !usbPlugged())
	{
    usbStarted = false ;
	  WatchdogTimeout = 200 ;
		USBD_Disconnect() ;
  }
  
  if (usbStarted )
	{
    usbJoystickUpdate();
  }
  
#endif
#endif

}



#ifdef PCBX9D
// Needs to be in pulses_driver.h
extern void init_no_pulses(uint32_t port) ;
extern void init_pxx(uint32_t port) ;

void init_i2s1( void ) ;
#endif

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
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
#ifdef SMALL
		case 1 :
 #ifdef FRENCH
			Language = French ;
 #ifndef PROP_TEXT
			ExtraFont = font_fr_extra ;
			ExtraBigFont = font_fr_big_extra ;
 #endif // PROP_TEXT
 #else // German
			Language = German ;
 #ifndef PROP_TEXT
			ExtraFont = font_de_extra ;
			ExtraBigFont = font_de_big_extra ;
 #endif // PROP_TEXT
 #endif
		break ;
#else
		case 1 :
			Language = French ;
#if defined(PCBX12D) || defined(PCBX10)
//			ExtraHorusFont = font_fr_h_extra ;
//			ExtraHorusBigFont = font_fr_h_big_extra ;
#else
 #ifndef PROP_TEXT
			ExtraFont = font_fr_extra ;
			ExtraBigFont = font_fr_big_extra ;
 #endif // PROP_TEXT
#endif		
		break ;
		case 2 :
			Language = German ;
#if defined(PCBX12D) || defined(PCBX10)
//			ExtraHorusFont = font_de_h_extra ;
//			ExtraHorusBigFont = font_de_h_big_extra ;
#else
 #ifndef PROP_TEXT
			ExtraFont = font_de_extra ;
			ExtraBigFont = font_de_big_extra ;
 #endif // PROP_TEXT
#endif		
		break ;
#endif
#ifndef SMALL
#ifndef PCBLEM1
		case 3 :
			Language = Norwegian ;
#if defined(PCBX12D) || defined(PCBX10)
//			ExtraHorusFont = font_se_h_extra ;
//			ExtraHorusBigFont = font_se_h_big_extra ;
#else
 #ifndef PROP_TEXT
			ExtraFont = font_se_extra ;
			ExtraBigFont = font_se_big_extra ;
 #endif // PROP_TEXT
#endif		
		break ;
		case 4 :
			Language = Swedish ;
#if defined(PCBX12D) || defined(PCBX10)
//			ExtraHorusFont = font_se_h_extra ;
//			ExtraHorusBigFont = font_se_h_big_extra ;
#else
 #ifndef PROP_TEXT
			ExtraFont = font_se_extra ;
			ExtraBigFont = font_se_big_extra ;
 #endif // PROP_TEXT
#endif		
		break ;
		case 5 :
			Language = Italian ;
#if defined(PCBX12D) || defined(PCBX10)
//			ExtraHorusFont = font_it_h_extra ;
//			ExtraHorusBigFont = NULL ;
#else
 #ifndef PROP_TEXT
			ExtraFont = font_it_extra ;
			ExtraBigFont = NULL ;
 #endif // PROP_TEXT
#endif		
		break ;
		case 6 :
			Language = Polish ;
#if defined(PCBX12D) || defined(PCBX10)
//			ExtraHorusFont = font_pl_h_extra ;
//			ExtraHorusBigFont = NULL ;
#else
 #ifndef PROP_TEXT
			ExtraFont = font_pl_extra ;
			ExtraBigFont = NULL ;
 #endif // PROP_TEXT
#endif		
		break ;
		case 7 :
			Language = Vietnamese ;
#if defined(PCBX12D) || defined(PCBX10)
//			ExtraHorusFont = NULL ;
//			ExtraHorusBigFont = NULL ;
#else
 #ifndef PROP_TEXT
			ExtraFont = NULL ;
			ExtraBigFont = NULL ;
 #endif // PROP_TEXT
#endif		
		break ;
		case 8 :
			Language = Spanish ;
#if defined(PCBX12D) || defined(PCBX10)
//			ExtraHorusFont = NULL ;
//			ExtraHorusBigFont = NULL ;
#else
 #ifndef PROP_TEXT
			ExtraFont = NULL ;
			ExtraBigFont = NULL ;
 #endif // PROP_TEXT
#endif		
		break ;
#endif		
#endif		
		default :
			Language = English ;
#if defined(PCBX12D) || defined(PCBX10)
//			ExtraHorusFont = NULL ;
//			ExtraHorusBigFont = NULL ;
#else
 #ifndef PROP_TEXT
			ExtraFont = NULL ;
			ExtraBigFont = NULL ;
 #endif // PROP_TEXT
#endif		
		break ;
	}
}

const uint8_t csTypeTable[] =
{ CS_VOFS, CS_VOFS, CS_VOFS, CS_VOFS, CS_VBOOL, CS_VBOOL, CS_VBOOL,
 CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VCOMP, CS_VBOOL, CS_VBOOL, CS_TIMER, 
 CS_TIMER, CS_TMONO, CS_TMONO, CS_VOFS, CS_U16, CS_VCOMP, CS_VOFS, CS_2VAL, CS_VOFS, CS_VOFS
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
#if defined(REV9E) || defined(PCBX7) || defined(PCBX12D) || defined(PCBX9LITE) || defined(PCBX10) || defined(PCBLEM1)
#ifdef PCBX7
 #ifndef PCBT12
	uint8_t value = (~GPIOE->IDR & PIN_BUTTON_ENCODER) ? 0x80 : 0 ;
 #endif
#endif // PCBX7
#ifdef PCBX9LITE
	uint8_t value = (~GPIOE->IDR & PIN_BUTTON_ENCODER) ? 0x80 : 0 ;
#endif // PCBX9LITE
#ifdef REV9E
	uint8_t value = (~GPIOF->IDR & PIN_BUTTON_ENCODER) ? 0x80 : 0 ;
#endif // REV9E
#if defined(PCBX12D)
	uint8_t value = (~GPIOC->IDR & 0x0002) ? 0x80 : 0 ;
#endif // PCBX12D
#if defined(PCBX10)
	uint8_t value = (~GPIOI->IDR & 0x0100) ? 0x80 : 0 ;
#endif // PCBX12D
#ifdef PCBLEM1
	uint8_t value = (~ENC_SW_GPIO->IDR & ENC_SW_PIN) ? 0x80 : 0 ;
#endif // PCBLEM1
 #ifndef PCBT12
	return (~read_keys() & 0x7E ) | value ; 
 #else		
	return ~read_keys() & 0x7E ;
 #endif
#else		
 #ifdef PCBSKY
	uint8_t value = (~PIOB->PIO_PDSR & 0x40) ? 0x80 : 0 ;
	return (~read_keys() & 0x7E ) | value ; 
 #else	
	return ~read_keys() & 0x7E ;
 #endif
#endif
}

void clearKeyEvents()
{
    while(keyDown())
		{
			  // loop until all keys are up
#if defined(PCBX12D)	// || defined(PCBX10)
 #ifndef MIXER_TASK
			getADC_single() ;	// For nav joystick on left
 #endif
#endif
			wdt_reset() ;
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

#ifdef PCBSKY
 #ifdef REVX
  #ifdef SERIAL_TEST_PRO

uint32_t Tbrates[6] = {	2400, 4800, 9600, 19200, 38400, 57600 } ;
uint8_t TbIndex ;

void menuDiag(uint8_t event)
{
	static uint32_t counter = 0 ;
	static uint32_t c1 = 0 ;
	static uint32_t c2 = 0 ;
//	static uint32_t op = 1 ;
	static uint32_t mode = 0 ;
	static uint8_t rxChar = '?' ;
	static uint8_t rx2Char = '?' ;
	static uint8_t reboot = 0 ;
	static uint8_t txChar = '!' ;
	int16_t rx ;

	uint32_t value ;
	static uint32_t state = 0 ;
	static uint32_t timer = 0 ;

  TITLE( "DIAG" ) ;
  switch(event)
	{
		case EVT_ENTRY :
#ifdef REVX
			init_twi() ;
 			USART0->US_CR = US_CR_RXDIS | US_CR_TXDIS ;
 			
//			USART0->US_CR = US_CR_RXEN | US_CR_TXEN ;
			USART0->US_IDR = US_IDR_TXEMPTY ;
			configure_pins( PIO_PA25, PIN_ENABLE | PIN_LOW | PIN_OUTPUT | PIN_PORTA | PIN_NO_PULLUP ) ;
			PIOA->PIO_SODR = 0x02000000L ;	// Set bit A25 ON, enable SPort output
			
			configure_pins( PIO_PA6, PIN_ENABLE | PIN_HIGH | PIN_OUTPUT | PIN_PORTA | PIN_NO_PULLUP ) ;
			configure_pins( PIO_PA5, PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_NO_PULLUP ) ;
			clearMFP() ;

#endif
		break ;
			
		case EVT_KEY_LONG(BTN_RE) :
			if ( g_eeGeneral.disableBtnLong == 0 )
			{
				break ;
			}		
    case EVT_KEY_LONG(KEY_EXIT):
			reboot = 1 ;
		break ;
	}

	PIOA->PIO_SODR = 0x02000000L ;	// Set bit A25 ON, enable SPort output
	switch ( state )
	{
		case 0 :
			if ( ++counter > 99 )
			{
				counter = 0 ;
				state = 1 ;
			}
		break ;
	
		case 1 :
			PIOA->PIO_CODR = 0x00000040L ;	// Set bit A6 low
			TC0->TC_CHANNEL[0].TC_CCR = 5 ;	// Enable clock and trigger it (may only need trigger)
			while ( TC0->TC_CHANNEL[0].TC_CV < 5000 )		// Value depends on MCK/8
			{
				// Wait
				if ( ( PIOA->PIO_PDSR & 0x00000020 ) == 0 )
				{
					break ;
				}
			}
			timer = TC0->TC_CHANNEL[0].TC_CV ;
			state = 2 ;
		break ;

		case 2 :
			if ( ++counter > 99 )
			{
				counter = 0 ;
				state = 3 ;
			}
		break ;
	
		case 3 :
			PIOA->PIO_SODR = 0x00000040L ;	// Set bit A6 high
			TC0->TC_CHANNEL[0].TC_CCR = 5 ;	// Enable clock and trigger it (may only need trigger)
			while ( TC0->TC_CHANNEL[0].TC_CV < 5000 )		// Value depends on MCK/8
			{
				// Wait
				if ( PIOA->PIO_PDSR & 0x00000020 )
				{
					break ;
				}
			}
			timer = TC0->TC_CHANNEL[0].TC_CV ;
			state = 0 ;
			if ( ++c2 > 11 )
			{
				state = 4 ;
			}
			if ( ++c1 > 9 )
			{
				c1 = 0 ;
				clearMFP() ;
			}
			if ( c1 > 4 )
			{
				setMFP() ;
			}
		break ;

		case 4 :
			com1_Configure( 2400, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
			com2_Configure( 2400, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
			state = 5 ;
			USART0->US_MR = (USART0->US_MR & 0xFFFF3FFF ) | 0x00008000 ;	// Local loopback
			counter = 0 ;
		break ;

		default :
		break ;
	}
	if ( state < 4 )
	{
		value = PIOA->PIO_PDSR ;
		PUTS_ATT_LEFT( 3*FH, (value & 0x0040) ? "High" : "Low " ) ;
		PUTS_ATT_LEFT( 4*FH, (value & 0x0020) ? "High" : "Low " ) ;
		PUT_HEX4( 0, 5*FH, timer ) ;
		PUT_HEX4( 0, 56, state ) ;
		PUT_HEX4( 40, 56, c1 ) ;
	}
	else
	{
		rx = get_fifo128( &Com1_fifo ) ;
		if ( rx != -1 )
		{
			rxChar = rx ;
			if ( rxChar == 'F' )
			{
				uint32_t brate ;
				TbIndex += 1 ;
				if ( TbIndex > 5 )
				{
					TbIndex = 0 ;
					mode = 1 ;
					USART0->US_MR = (USART0->US_MR & 0xFFFF3FFF ) ;	// Normal
				}
				brate = Tbrates[TbIndex] ;
				com1_Configure( brate, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
				com2_Configure( brate, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
			}
		}
		if ( TbIndex < 4 )
		{
			PUTS_NUMX( 30, 2*FH, Tbrates[TbIndex] ) ;
		}
		else if ( TbIndex == 4 )
		{
			PUTS_ATT_LEFT( 2*FH, "38400" ) ;
		}
		else
		{
			PUTS_ATT_LEFT( 2*FH, "57600" ) ;
		}
		PUTC( 64, 32 , rxChar ) ;
		if ( ++counter > 99 )
		{
			counter = 0 ;
			txChar = rxChar ;
			if ( txChar == '?' )
			{
				txChar = 'A' ;
			}
			else
			{
				txChar += 1 ;
				if ( txChar > 'F' )
				{
					txChar = 'A' ;
				}
			}
	//		txmit( txChar ) ;		
			PIOA->PIO_SODR = 0x02000000L ;	// Set bit A25 ON, enable SPort output
			USART0->US_THR = txChar ;
			c1 += 1 ;
		}
		PUT_HEX4( 0, 56, c1 ) ;
		PUTC( 60, 56 , txChar ) ;
		rx = get_fifo128( &Com2_fifo ) ;
		if ( rx != -1 )
		{
			rx2Char = rx ;
		}
		PUTC( 80, 32 , rx2Char ) ;
		if ( mode == 0 )
		{
			lcd_puts_Pleft( 1*FH, "Local Loop" ) ;
		}
	}

	if ( reboot )
	{
		if ( (~read_keys() & 0x7E) == 0 )
		{
		  NVIC_SystemReset() ;
		}
	}
}

void diag_mode(void* pdata)
{
	uint32_t displayTimer = 0 ;
	static uint32_t begin = 0 ;
	com1_Configure( 2400, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
	com2_Configure( 2400, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
	g_menuStack[0] = menuDiag ;
	g_menuStack[1] = menuDiag ;	// this is so the first instance of [MENU LONG] doesn't freak out!

  while (1)
	{
		if ( ( check_soft_power() == POWER_OFF )/* || ( goto_usb ) */ )		// power now off
		{
			soft_power_off() ;		// Only turn power off if necessary
		}
	  static uint16_t lastTMR;
		uint16_t t10ms ;
		t10ms = get_tmr10ms() ;
  	tick10ms = ((uint16_t)(t10ms - lastTMR)) != 0 ;
	  lastTMR = t10ms ;
		if(!tick10ms) continue ; //make sure the rest happen only every 10ms.
	  uint8_t evt=getEvent() ;
		if ( begin == 0 )
		{
			evt = EVT_ENTRY ;
			begin = 1 ;
		}

    lcd_clear() ;
//		if ( EnterMenu )
//		{
//			evt = EnterMenu ;
//			EnterMenu = 0 ;
//		}
//	 	Tevent = evt ;
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
		CoTickDelay(1) ;					// 2mS for now
	}
}
  #endif
 #endif
#endif



extern void maintenanceBackground( void ) ;

uint8_t SetByEncoder ;

void maintenance_mode(void* pdata)
{
	uint32_t displayTimer = 0 ;
#ifndef PCBLEM1
	g_menuStack[0] = menuUpdate ;
	g_menuStack[1] = menuUp1 ;	// this is so the first instance of [MENU LONG] doesn't freak out!
#endif
	MaintenanceRunning = 1 ;
#if defined(PCBSKY) || defined (ARUNI)
	init_main_ppm( 10000, 0 ) ;		// Default for now, initial period 1.5 mS, output off
	init_ppm2( 10000, 0 ) ;
#else	
 #ifndef PCBLEM1
//	init_no_pulses( 0 ) ;
//	init_no_pulses( 1 ) ;
 #endif
#endif

#if defined(PCBT16)
	g_model.Module[0].protocol = PROTO_OFF ;
	g_model.Module[1].protocol = PROTO_OFF ;
#endif
	 
	com1_Configure( 57600, SERIAL_NORM, SERIAL_NO_PARITY ) ; // Kick off at 57600 baud
#ifdef ACCESS
#ifdef PCBX9LITE
	configure_pins( INTMODULE_TX_GPIO_PIN, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB | PIN_LOW ) ;
	GPIO_ResetBits( INTMODULE_TX_GPIO, INTMODULE_TX_GPIO_PIN) ;
#endif
#endif
#ifdef PCB9XT
	BlSetAllColours( 0, 30, 60 ) ;
#endif
	{
		uint8_t evt=getEvent() ;
		killEvents( evt ) ;
		putEvent(0) ;
	}
#ifdef PCBX9LITE
	ledBlue() ;
#endif
  
//#if defined(PCBTX16S)	
//extern void com3Init( uint32_t baudrate ) ;
//	com3Init( 115200 ) ;
//extern void txmit3( uint8_t c ) ;
//	txmit3( 'H' ) ;
//	txmit3( 'e' ) ;
//	txmit3( 'l' ) ;
//	txmit3( 'l' ) ;
//	txmit3( 'o' ) ;
//	txmit3( 13 ) ;
//	txmit3( 10 ) ;
//#endif
	
	while (1)
	{
#ifndef PCBLEM1
		if ( (g_menuStackPtr==0) && (g_menuStack[0] == menuUpdate) )
#endif
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

#ifndef PCBLEM1
		maintenanceBackground() ;
#endif
#ifdef PCBX7
 #ifndef PCBT12
//extern void checkRotaryEncoder() ;
//		checkRotaryEncoder() ;
 #endif
#endif // PCBX7
#ifdef REV9E
extern void checkRotaryEncoder() ;
		checkRotaryEncoder() ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
extern void checkRotaryEncoder() ;
  	checkRotaryEncoder() ;
#endif
		if(!tick10ms) continue ; //make sure the rest happen only every 10ms.
#ifdef PCBX9LITE
extern void checkRotaryEncoder() ;
//		checkRotaryEncoder() ;
#endif // PCBX9LITE
	  uint8_t evt=getEvent();
//#if defined(REV9E) || defined(PCBX7)
#if defined(PCBX12D) || defined(PCBX10)
		waitLcdClearDdone() ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		{
			int32_t x ;
			x = Rotary_count >> 1 ;
			Rotary_diff = x - LastRotaryValue ;
			LastRotaryValue = x ;
		}
#else
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
#endif
		SetByEncoder = 0 ;
		if ( evt == 0 )
		{
	extern int32_t Rotary_diff ;
			if ( Rotary_diff > 0 )
			{
				evt = EVT_KEY_FIRST(KEY_DOWN) ;
				SetByEncoder = 1 ;
			}
			else if ( Rotary_diff < 0 )
			{
				evt = EVT_KEY_FIRST(KEY_UP) ;
				SetByEncoder = 1 ;
			}
			Rotary_diff = 0 ;
		}
//#endif

    lcd_clear() ;
		if ( EnterMenu )
		{
			evt = EnterMenu ;
			EnterMenu = 0 ;
		}
	 	Tevent = evt ;
		g_menuStack[g_menuStackPtr](evt);
		// Only update display every 40mS, improves SPort update throughput
		if ( ++displayTimer >= 4 )
		{
			displayTimer = 0 ;
    	refreshDisplay() ;
	#if defined(PCBX12D) || defined(PCBX10)
			lcd_clearBackground() ;	// Start clearing other frame
	#endif
		}
		
		wdt_reset();

		if ( Tenms )
		{
			Tenms = 0 ;
		}
#ifndef SIMU
		sdPoll10mS() ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
		if ( g_eeGeneral.screenShotSw )
		{
			if ( getSwitch00( g_eeGeneral.screenShotSw ) )
			{
				if ( LastShotSwitch == 0 )
				{
extern const char *screenshot() ;
					rtc_gettime( &Time ) ;
					screenshot() ;
				}
				LastShotSwitch = 1 ;
			}
			else
			{
				LastShotSwitch = 0 ;
			}
		}
#endif


#ifndef SIMU
		CoTickDelay(1) ;					// 2mS for now
#endif
	}
}



#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
extern uint8_t TrainerMode ;
#define EXTERNAL_RF_ON()      GPIO_SetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#define EXTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#endif

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
void checkTrainerSource()
{
	uint32_t tSource = g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[0].source ;
	if ( CurrentTrainerSource	!= tSource )
	{
		switch ( CurrentTrainerSource )
		{
			case 0 :
				stop_trainer_capture() ;
			break ;
#ifndef PCBX12D
 #ifndef PCBX10
  #ifndef REV19
   #ifndef PCBX7ACCESS
			case 1 :
				stop_USART6_Sbus() ;
				if ( g_model.Module[1].protocol == PROTO_OFF )
				{
					EXTERNAL_RF_OFF() ;
				}
			break ;
			case 2 :
				stop_cppm_on_heartbeat_capture() ;				
				if ( g_model.Module[1].protocol == PROTO_OFF )
				{
					EXTERNAL_RF_OFF() ;
				}
			break ;
   #endif
  #endif
 #endif
#endif
#if defined(PCBX10) && defined(PCBREV_EXPRESS)
			case 1 :
				stop_USART_Sbus() ;
				if ( g_model.Module[1].protocol == PROTO_OFF )
				{
					EXTERNAL_RF_OFF() ;
				}
			break ;
			case 2 :
				stop_cppm_on_heartbeat_capture() ;				
				if ( g_model.Module[1].protocol == PROTO_OFF )
				{
					EXTERNAL_RF_OFF() ;
				}
			break ;
#endif
			case 3 :
				stop_trainer_ppm() ;
			break ;
			case 4 :
				stop_trainer_capture() ;
				TrainerMode = 0 ;
//				init_trainer_capture(0) ;
			break ;
		}
		CurrentTrainerSource = tSource ;
		switch ( CurrentTrainerSource )
		{
			case 0 :
				init_trainer_capture(CAP_PPM) ;
//				EXTERNAL_RF_OFF() ;
			break ;
#ifndef PCBX12D
 #ifndef PCBX10
  #ifndef REV19
   #ifndef PCBX7ACCESS
			case 1 :
				USART6_Sbus_configure() ;
				EXTERNAL_RF_ON() ;
			break ;
			case 2 :
				init_cppm_on_heartbeat_capture()  ;
				EXTERNAL_RF_ON() ;
			break ;
   #endif
  #endif
 #endif
#endif
#if defined(PCBX10) && defined(PCBREV_EXPRESS)
			case 1 :
				USART_Sbus_configure() ;
				EXTERNAL_RF_ON() ;
			break ;
			case 2 :
				init_cppm_on_heartbeat_capture()  ;
				EXTERNAL_RF_ON() ;
			break ;
#endif
			case 3 :	// Slave so output
				init_trainer_ppm() ;
				if ( g_model.Module[1].protocol == PROTO_OFF )
				{
					EXTERNAL_RF_OFF() ;
				}
//#if defined(PCBX10)
//	ExtRfOffPos = 6 ;
//#endif
			break ;
			case 4 :
				init_trainer_capture(CAP_PPM) ;
				uint32_t tPolarity = g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[1].source ;
				TrainerPolarity = tPolarity ;
				TrainerMode = CAP_SERIAL ;
				init_trainer_capture( CAP_SERIAL ) ;
			break ;
		}
	}
}
#endif

#ifdef PCB9XT

// 0 Jack PPM
// 1 BT
// 2 COM2
// 3 Slave
// 4 JACK SBUS

void checkTrainerSource()
{
	uint32_t tSource = g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[0].source ;
	if ( g_model.traineron == 0 )
	{
		tSource = TRAINER_JACK ;
	}
	
	//static uint8_t trainerActive = 0 ;
	if ( check_soft_power() == POWER_TRAINER )		// On trainer power
	{
		tSource = TRAINER_SLAVE ;
	}	
		
	if ( CurrentTrainerSource	!= tSource )
	{
		switch ( CurrentTrainerSource )
		{
			case TRAINER_JACK :
				stop_trainer_capture() ;
			break ;
			case TRAINER_BT :
				stop_trainer_capture() ;
			break ;
			case TRAINER_COM2 :
			break ;
			case TRAINER_SLAVE :
				stop_trainer_ppm() ;
			break ;
			case TRAINER_J_SBUS :
				stop_trainer_capture() ;
				TrainerMode = 0 ;
			break ;
		}
		CurrentTrainerSource = tSource ;
		switch ( tSource )
		{
			case TRAINER_JACK :
				init_trainer_capture(CAP_PPM) ;
//				EXTERNAL_RF_OFF() ;
			break ;
			case TRAINER_BT :
			break ;
			case TRAINER_COM2 :
			break ;
			case TRAINER_SLAVE :	// Slave so output
				init_trainer_ppm() ;
				if ( g_model.Module[1].protocol == PROTO_OFF )
				{
					EXTERNAL_RF_OFF() ;
				}
			break ;
			case TRAINER_J_SBUS :
				init_trainer_capture(CAP_PPM) ;
				uint32_t tPolarity = g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[1].source ;
				TrainerPolarity = tPolarity ;
				TrainerMode = 1 ;
				init_trainer_capture( CAP_SERIAL ) ;
			break ;
		}
	}	 
//		if ( trainerActive == 0 )
//		{
//			stop_trainer_capture() ;
//			trainerActive = 1 ;
//			init_trainer_ppm() ;
//		}
//	}
//	else
//	{
//		if ( trainerActive )
//		{
//			stop_trainer_ppm() ;
//			trainerActive = 0 ;
//			init_trainer_capture(0) ;
//		}
//	}
}
#endif

#ifdef PCBLEM1

void checkTrainerSource()
{
	uint32_t tSource = g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[0].source ;
		
	if ( CurrentTrainerSource	!= tSource )
	{
		switch ( CurrentTrainerSource )
		{
			case 0 :
			case 1 :
			case 2 :
			case 4 :
				stop_trainer_capture() ;
			break ;
			case TRAINER_SLAVE :
				stop_trainer_ppm() ;
			break ;
		}
		CurrentTrainerSource = tSource ;
		switch ( tSource )
		{
			case 0 :
			case 1 :
			case 2 :
			case 4 :
				init_trainer_capture(CAP_PPM) ;
			break ;
			case TRAINER_SLAVE :	// Slave so output
				init_trainer_ppm() ;
			break ;
		}
	}	 
}
#endif

#if not (defined(PCBX10))
void com2Configure()
{
	if ( g_model.com2Function == COM2_FUNC_SBUSTRAIN )
	{
#ifndef PCBLEM1
		UART_Sbus_configure( Master_frequency ) ;
#endif
	}
	else if ( g_model.com2Function == COM2_FUNC_SBUS57600 )
	{
#ifndef PCBLEM1
		UART_Sbus57600_configure( Master_frequency ) ;
#endif
	}
#ifdef PCBSKY
	else if ( ( g_model.com2Function == COM2_FUNC_BTDIRECT )
					 || ( g_model.com2Function == COM2_FUNC_TEL_BT2WAY ) )
	{
		UART_Configure( g_model.com2Baudrate-1, Master_frequency ) ;
	}
#endif	// PCBSKY
#ifdef PCBSKY
//	else if ( g_model.com2Function == COM2_FUNC_FMS )
//	{
//		UART_Configure( 19200, Master_frequency ) ;
//	}
	else if ( g_model.com2Function == COM2_FUNC_LCD )
	{
		UART_Configure( 115200, Master_frequency ) ;
	}
#endif
#ifdef PCBX9D
	else if( g_model.com2Function == COM2_FUNC_CPPMTRAIN )
	{
		init_serial_trainer_capture() ;
	}
	else if ( g_model.com2Function == COM2_FUNC_LCD )
	{
		com2_Configure( 115200, SERIAL_NORM, 0 ) ;
	}
	else if ( g_model.com2Function == COM2_FUNC_BT_ENC )
	{
		com2_Configure( 100000, SERIAL_NORM, 0 ) ;
	}
#endif
	else
	{
#ifdef PCBSKY
		com2_Configure( CONSOLE_BAUDRATE, SERIAL_NORM, SERIAL_NO_PARITY ) ;
#endif
#ifdef PCBX9D
		ConsoleInit() ;
#endif
#ifdef PCB9XT
		consoleInit() ;
#endif
#if defined(PCBX12D) // || defined(PCBX10)
		ConsoleInit() ;
#endif
	}	 
}
#endif

#ifdef PCBSKY
 #ifndef SMALL
static void checkAr9x()
{
 #ifndef REVX
	uint32_t x = ChipId ;
	if ( ( x & 0x00000F00 )<= 0x00000900 )
	{
		g_eeGeneral.ar9xBoard = 0 ;
		return ;
	}
	if ( x & 0x00000080 )
	{ // M4 chip so not guaranteed to be AR9X
		return ;
	}
	g_eeGeneral.ar9xBoard = 1 ;
	g_eeGeneral.softwareVolume = 1 ;
 #ifdef ARUNI
	g_eeGeneral.altSwitchNames = 1 ;
  SixPosCaptured = 0x80;
 #endif
 #else
 	g_eeGeneral.ar9xBoard = 0 ;
 #endif
}
 #endif
#endif


#if defined(PCB9XT) || defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
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

//#ifdef PCB9XT
//uint16_t PpmTestH ;
//uint16_t PpmTestL ;

//void CheckPpm()
//{
//	if ( ( PpmTestH != 0x0080 ) || ( PpmTestL != 0 ) )
//	{
//		alert(XPSTR("PPM Output\037Self Test Failure"));
//	}
//}
//#endif

//#ifdef REVX
//uint16_t SportTest ;
//void checkSport()
//{
//	if ( SportTest )
//	{
//		alert(XPSTR("SPort Output\037Self Test Failure"));
//	}
//}
//#endif

//#ifdef REVX
//static void sportTest()
//{
//	// Test SPort output signal
//	SportTest = 0 ;
//	configure_pins( (PIO_PA25 | PIO_PA6), PIN_ENABLE | PIN_HIGH | PIN_OUTPUT | PIN_PORTA | PIN_NO_PULLUP ) ;
//	configure_pins( PIO_PA5 , PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_NO_PULLUP ) ;
//	TC0->TC_CHANNEL[0].TC_CCR = 5 ;	// Enable clock and trigger it (may only need trigger)
//	while ( TC0->TC_CHANNEL[0].TC_CV < 32*40 )		// Value depends on MCK/2)
//	{
//		// Wait
//	}
//	uint32_t input ;
//	input = PIOA->PIO_PDSR & PIO_PA5 ;
//	PIOA->PIO_CODR = PIO_PA6 ;
//	TC0->TC_CHANNEL[0].TC_CCR = 5 ;	// Enable clock and trigger it (may only need trigger)
//	while ( TC0->TC_CHANNEL[0].TC_CV < 32*40 )		// Value depends on MCK/2)
//	{
//		// Wait
//	}
//	input ^= PIOA->PIO_PDSR & PIO_PA5 ;
//	PIOA->PIO_CODR = PIO_PA25 ;	// Set bit A25 OFF, disable SPort output
//	if ( ( input & PIO_PA25 ) != 0 )
//	{
//		SportTest = 1 ;
//	}
//}	
//#endif

#if defined(PCBX9LITE)
void ledOff()
{
  GPIO_ResetBits(LED_RED_GPIO, LED_RED_GPIO_PIN);
  GPIO_ResetBits(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
  GPIO_ResetBits(LED_GREEN_GPIO, LED_GREEN_GPIO_PIN);
}

void ledRed()
{
  ledOff();
  GPIO_SetBits(LED_RED_GPIO, LED_RED_GPIO_PIN);
}
void ledGreen()
{
  ledOff();
  GPIO_SetBits(LED_GREEN_GPIO, LED_GREEN_GPIO_PIN);
}

void ledBlue()
{
  ledOff();
  GPIO_SetBits(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
}

#endif // X3

#if defined(PCBX7)
void ledOff()
{
  GPIO_ResetBits(LED_RED_GPIO, LED_RED_GPIO_PIN);
  GPIO_ResetBits(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
 #ifndef PCBX7ACCESS
  GPIO_ResetBits(LED_GREEN_GPIO, LED_GREEN_GPIO_PIN);
 #endif
}

#ifdef PCBT12
void ledRed()
{
  ledOff();
  GPIO_SetBits(LED_RED_GPIO, LED_RED_GPIO_PIN);
}
#endif

 #ifndef PCBX7ACCESS
void ledGreen()
{
  ledOff();
  GPIO_SetBits(LED_GREEN_GPIO, LED_GREEN_GPIO_PIN);
}
 #endif

#ifdef PCBX7ACCESS
void ledBlue()
{
  ledOff();
  GPIO_SetBits(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
}
#endif

#ifdef PCBT12
void ledBlue()
{
  ledOff();
  GPIO_SetBits(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
}
#endif
#endif // PCBX7

#if defined (PCBXLITE)
void ledOff()
{
  GPIO_SetBits(LED_RED_GPIO, LED_RED_GPIO_PIN);
  GPIO_SetBits(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
#ifndef PCBXLITES
  GPIO_SetBits(LED_GREEN_GPIO, LED_GREEN_GPIO_PIN);
#endif
}

//void ledRed()
//{
//  ledOff();
//  GPIO_SetBits(LED_RED_GPIO, LED_RED_GPIO_PIN);
//}

#ifndef PCBXLITES
void ledGreen()
{
  ledOff();
  GPIO_ResetBits(LED_GREEN_GPIO, LED_GREEN_GPIO_PIN);
}
#endif
#ifdef PCBXLITES
void ledBlue()
{
  ledOff();
  GPIO_ResetBits(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
}
#endif

//void ledBlue()
//{
//  ledOff();
//  GPIO_SetBits(LED_BLUE_GPIO, LED_BLUE_GPIO_PIN);
//}
#endif // PCBXLITE

#ifdef PCBLEM1
void ledInit() ;
void ledRed() ;
void ledBlue() ;
void ledGreen() ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
void ledInit() ;
void ledRed() ;
void ledBlue() ;
#if defined(PCBTX16S)
void ledGreen() ;
#endif
void lcd_outhex8(uint16_t x,uint16_t y,uint32_t val) ;
extern void lcdInit(void) ;
void lcdColorsInit(void) ;
#endif // PCBX12D

#if defined(PCBX12D) || defined(PCBX10) || defined(REV19)
#define MAIN_STACK_REQUIRED	550
#else
#define MAIN_STACK_REQUIRED	250
#endif

#ifndef SMALL
uint32_t MainStack[MAIN_STACK_REQUIRED] ;
#endif

#ifdef STACK_PROBES
uint32_t stackSpace( uint32_t stack )
{
	uint32_t i ;
	switch ( stack )
	{
		case 0 :
			for ( i = 0 ; i < MAIN_STACK_SIZE ; i += 1 )
			{
				if ( main_stk[i] != 0x55555555 )
				{
					break ;
				}
			}
		return i ;

		case 1 :
			for ( i = 0 ; i < LOG_STACK_SIZE ; i += 1 )
			{
				if ( Log_stk[i] != 0x55555555 )
				{
					break ;
				}
			}
		return i ;

		case 2 :
			for ( i = 0 ; i < VOICE_STACK_SIZE ; i += 1 )
			{
				if ( voice_stk[i] != 0x55555555 )
				{
					break ;
				}
			}
		return i ;

#ifdef BLUETOOTH
		case 3 :
			for ( i = 0 ; i < BT_STACK_SIZE ; i += 1 )
			{
				if ( Bt_stk[i] != 0x55555555 )
				{
					break ;
				}
			}
		return i ;
#endif
#ifdef MIXER_TASK
		case 4 :
			for ( i = 0 ; i < MIXER_STACK_SIZE ; i += 1 )
			{
				if ( Mixer_stk[i] != 0x55555555 )
				{
					break ;
				}
			}
		return i ;
#endif
#ifndef SMALL
		case 5 :
			for ( i = 0 ; i < MAIN_STACK_REQUIRED ; i += 1 )
			{
				if ( MainStack[i] != 0x55555555 )
				{
					break ;
				}
			}
		return i ;
#endif
	}
	return 0 ;
}
#endif


//#if defined(PCBX12D) || defined(PCBX10)
//void where( uint8_t chr )
//{
//	lcd_clear() ;
//	PUTC( 0, 0, chr ) ;
//	PUT_HEX4( 0, 16, ~read_keys() ) ;
//	refreshDisplay() ;
//}
//#else
//#define where( t )
//#endif

#ifdef STARTUP_DEBUG	

#define LCD_A0    0x00000080L
void startupDebugInit()
{
//	uint32_t i ;
//	uint32_t j ;
  PMC->PMC_PCER0 = (1<<ID_PIOC) ;				// Enable clock PIOC
  PMC->PMC_PCER0 = (1<<ID_PIOA) ;				// Enable clock PIOA
  PMC->PMC_PCER0 = (1<<ID_PIOB) ;				// Enable clock PIOA
	configure_pins( LCD_A0, PIN_ENABLE | PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
	configure_pins( PIO_PC18, PIN_ENABLE | PIN_OUTPUT | PIN_PORTC | PIN_NO_PULLUP | PIN_HIGH ) ;
	
	PIOC->PIO_SODR = PIO_PC18 ;		// Set bit C18 HIGH
//	for ( j = 0 ; j < 2 ; j += 1 )
//	{
//		for ( i = 0 ; i < 500000 ; i += 1 )
//		{
//  		wdt_reset() ;
//		}
//		PIOC->PIO_CODR = PIO_PC18 ;		// Set bit C18 LOW
//		for ( i = 0 ; i < 500000 ; i += 1 )
//		{
//  		wdt_reset() ;
//		}
//		PIOC->PIO_SODR = PIO_PC18 ;		// Set bit C18 HIGH
//	}
	init_hw_timer() ;
	lcdInit() ;
}

extern "C" void cppstartupDebugInit()
{
	uint32_t i ;
//extern uint8_t ErcLcd ;
//extern uint8_t LcdLock ;
//extern uint8_t Lcd_lastPos ;
//extern const uint8_t *ExtraFont ;
//extern const uint8_t *ExtraBigFont ;
//	ErcLcd = 0 ;
//	LcdLock = 0 ;
//	Lcd_lastPos = 0 ;
//	ExtraFont = NULL ;
//	ExtraBigFont = NULL ;
//	ResetReason = RSTC->RSTC_SR ;
//	g_eeGeneral.rotateScreen = 0 ;
//  g_eeGeneral.optrexDisplay = 1 ;
//	g_model.com2Function = 0 ;
//	g_model.BTfunction = 0 ;
//	MATRIX->CCFG_SYSIO |= 0x000010F0L ;		// Disable syspins, enable B4,5,6,7,12
//	Master_frequency = 12000000L ;
//extern unsigned long _stext;
//  *((uint32_t*)0xE000ED08) = (uint32_t)&_stext;

  PMC->PMC_PCER0 = (1<<ID_PIOC) ;				// Enable clock PIOC
	configure_pins( PIO_PC18, PIN_ENABLE | PIN_OUTPUT | PIN_PORTC | PIN_NO_PULLUP ) ;
	PIOC->PIO_SODR = PIO_PC18 ;		// Set bit C18 HIGH
	for ( i = 0 ; i < 500000 ; i += 1 )
	{
  	wdt_reset() ;
	}
	PIOC->PIO_CODR = PIO_PC18 ;		// Set bit C18 LOW
	for ( i = 0 ; i < 500000 ; i += 1 )
	{
  	wdt_reset() ;
	}
}

#endif

#ifdef WHERE_DEBUG
void where( uint8_t chr )
{
//	lcd_clear() ;
#ifdef WHERE_STORE
	uint8_t *p = (uint8_t *) BKPSRAM_BASE ;
	*p = chr ;
#else
	uint32_t i ;
extern uint16_t LcdForeground	;
	LcdForeground = LCD_GREEN ;
	PUTC_ATT( 0, 0, chr, DBLSIZE ) ;
//	PUT_HEX4( 0, 16, ~read_keys() ) ;
	refreshDisplay() ;
  wdt_reset();
  WatchdogTimeout = 100 ;
	for ( i = 0 ; i < 2000000 ; i += 1 )
	{
  	wdt_reset() ;
	}
#endif
}

#ifdef PCBSKY
extern "C" void cppwhere( uint8_t chr ) 
{
	uint32_t i ;
	PIOC->PIO_CODR = PIO_PC18 ;		// Set bit C18 LOW
	for ( i = 0 ; i < 500000 ; i += 1 )
	{
  	wdt_reset() ;
	}
	PIOC->PIO_SODR = PIO_PC18 ;		// Set bit C18 HIGH
	for ( i = 0 ; i < 500000 ; i += 1 )
	{
  	wdt_reset() ;
	}
}
#endif
#endif

#ifdef PCB9XT
uint32_t WatchdogPosition ;
//static void enableBackupRam()
//{
//	PWR->CR |= PWR_CR_DBP ;
////	PWR->CSR |= PWR_CSR_BRE ;
////	while ( ( PWR->CSR & PWR_CSR_BRR) == 0 )
////	{
////		wdt_reset() ;
////	}
//	RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN ;
//}

#endif	

//void WDT_IRQHandler (void)
//{
//	uint32_t *ptr ;
//	uint32_t data ;
//	ptr = (uint32_t *) __get_MSP() ;

//	for(;;)
//	{
//		wdt_reset() ;
//		lcd_clear() ;

//		lcd_puts_Pleft(0, "WWWW") ;

////		data = *ptr ;
////		PUT_HEX4( 0, 0, data >> 16) ;
////		PUT_HEX4( 30, 0, data ) ;
////		data = ptr[1] ;
////		PUT_HEX4( 0, 1*FH, data >> 16) ;
////		PUT_HEX4( 30, 1*FH, data ) ;
////		data = ptr[2] ;
////		PUT_HEX4( 0, 2*FH, data >> 16) ;
////		PUT_HEX4( 30, 2*FH, data ) ;
////		data = ptr[3] ;
////		PUT_HEX4( 0, 3*FH, data >> 16) ;
////		PUT_HEX4( 30, 3*FH, data ) ;
////		data = ptr[4] ;
////		PUT_HEX4( 0, 4*FH, data >> 16) ;
////		PUT_HEX4( 30, 4*FH, data ) ;
////		data = ptr[5] ;
////		PUT_HEX4( 0, 5*FH, data >> 16) ;
////		PUT_HEX4( 30, 5*FH, data ) ;
////		data = ptr[6] ;
////		PUT_HEX4( 0, 6*FH, data >> 16) ;
////		PUT_HEX4( 30, 6*FH, data ) ;
////		data = ptr[7] ;
////		PUT_HEX4( 0, 7*FH, data >> 16) ;
////		PUT_HEX4( 30, 7*FH, data ) ;
//		refreshDisplay() ;
//	}
//}

#ifdef WDOG_REPORT
uint16_t WdogIntValue ;
#endif

#ifdef PCBLEM1
static void init_rotary_encoder()
{
  register uint32_t dummy ;
	
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN ;	// Enable portC clock
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN ;
	configure_pins( ENC_SW_PIN | ENC_A_PIN | ENC_B_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | ENC_SW_PORT ) ;
	g_eeGeneral.rotaryDivisor = 1 ;
	
	dummy = GPIOENCODER->IDR ;	// Read Rotary encoder ( PC14, PC13 )
	dummy >>= 13 ;
	dummy &= 0x03 ;			// pick out the two bits
	Rotary_position &= ~0x03 ;
	Rotary_position |= dummy ;
	
	AFIO->EXTICR[3] |= 0x0220 ;		// PC14,13
	EXTI->RTSR |= 0x6000 ;	// Rising Edge
	EXTI->FTSR |= 0x6000 ;	// Falling Edge
	EXTI->IMR |= 0x6000 ;
	
	NVIC_SetPriority( EXTI15_10_IRQn, 1 ) ; // Not quite highest priority interrupt
	NVIC_EnableIRQ( EXTI15_10_IRQn) ;
}

//void checkRotaryEncoder()
//{
//  register uint32_t dummy ;
	
//	dummy = GPIOENCODER->IDR ;	// Read Rotary encoder ( PE6, PE5 )
//	dummy >>= 13 ;
//	dummy &= 0x03 ;			// pick out the two bits
//	if ( dummy != ( Rotary_position & 0x03 ) )
//	{
//		if ( ( Rotary_position & 0x01 ) ^ ( ( dummy & 0x02) >> 1 ) )
//		{
//			Rotary_count += 1 ;
//		}
//		else
//		{
//			Rotary_count -= 1 ;
//		}
//		Rotary_position &= ~0x03 ;
//		Rotary_position |= dummy ;
//	}
//}

void initSwitches()
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN ; 			// Enable portB clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPDEN ; 			// Enable portD clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPEEN ; 			// Enable portE clock

	configure_pins( SW1_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW1_PORT ) ;
	configure_pins( SW2_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW2_PORT ) ;
	configure_pins( SW3_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW3_PORT ) ;
	configure_pins( SW4_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW4_PORT ) ;
	configure_pins( SW5_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW5_PORT ) ;
	configure_pins( SW7_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW7_PORT ) ;
	configure_pins( SW8_PIN, PIN_INPUT | PIN_PULLU_D | PIN_PULLUP | SW8_PORT ) ;
	 
}


#endif // PCBLEM1

#if defined(PCBX10) && defined(PCBREV_EXPRESS)		

//uint32_t eespi_operation( register uint8_t *tx, register uint8_t *rx, register uint32_t txcount, uint32_t rxcount )
//{
//	register SPI_TypeDef *spiptr = SPI2 ;
//	register uint32_t result ;
	
//	GPIOI->BSRRH = 0x0001 ;		// output enable
//	(void) spiptr->DR ;		// Dump any rx data
//	while( txcount )
//	{
//		result = 0 ;
//		while( ( spiptr->SR & SPI_SR_TXE ) == 0 )
//		{
//			// wait
//			if ( ++result > 10000 )
//			{
//				result = 0xFFFF ;
//				break ;				
//			}
//		}
//		if ( result > 10000 )
//		{
//			break ;
//		}
//		spiptr->DR = *tx++ ;
//		result = 0 ;
//		while( ( spiptr->SR & SPI_SR_RXNE ) == 0 )
//		{
//			// wait for received
//			if ( ++result > 10000 )
//			{
//				result = 0x2FFFF ;
//				break ;				
//			}
//		}
//		if ( result > 10000 )
//		{
//			break ;
//		}
//		(void) spiptr->DR ;		// Dump any rx data
//		txcount -= 1 ;
//	}

//	tx = rx ; 
//	while( rxcount )
//	{
//		result = 0 ;
//		while( ( spiptr->SR & SPI_SR_TXE ) == 0 )
//		{
//			// wait
//			if ( ++result > 10000 )
//			{
//				result = 0xFFFF ;
//				break ;				
//			}
//		}
//		if ( result > 10000 )
//		{
//			break ;
//		}
//		spiptr->DR = *tx++ ;
//		result = 0 ;
//		while( ( spiptr->SR & SPI_SR_RXNE ) == 0 )
//		{
//			// wait for received
//			if ( ++result > 10000 )
//			{
//				result = 0x2FFFF ;
//				break ;				
//			}
//		}
//		if ( result > 10000 )
//		{
//			break ;
//		}
//		*rx++ = spiptr->DR ;		// Dump any rx data
//		rxcount -= 1 ;
//	}
//	if ( result <= 10000 )
//	{
//		result = 0 ;
//	}
//	result = 0 ; 
//	GPIOI->BSRRL = 0x0001 ;		// output disable
	
//	return result ;
//}

//uint8_t Eedata[256] ;
//FIL g_oEeFile = {0};

//void dumpSerialFlash()
//{
//	uint8_t command[16] ;
//	FRESULT fresult ;
//	UINT nwritten ;
//	uint32_t i ;
//	// init SPI2
//  /* Enable GPIO clock for Signals */
//  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOI, ENABLE);
//  /* Enable SPI clock, SPI1: APB2, SPI2: APB1 */
//  RCC->APB1ENR |= RCC_APB1ENR_SPI2EN ;    // Enable clock

//	// APB1 clock / 4 = 133nS per clock
//	SPI2->CR1 = 0 ;		// Clear any mode error
//	SPI2->CR2 = 0 ;
//	SPI2->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_0 ;
//	SPI2->CR1 |= SPI_CR1_MSTR ;	// Make sure in case SSM/SSI needed to be set first
//	SPI2->CR1 |= SPI_CR1_SPE ;

//	configure_pins( 0x0001, PIN_PUSHPULL | PIN_OS25 | PIN_OUTPUT | PIN_PORTI ) ;
//	GPIOI->BSRRL = 0x0001 ;		// output disable
//	configure_pins( 0x000A, PIN_PUSHPULL | PIN_OS25 | PIN_PERIPHERAL | PIN_PORTI | PIN_PER_5 ) ;
//	configure_pins( 0x0004, PIN_PERIPHERAL | PIN_PORTI | PIN_PULLUP | PIN_PER_5 ) ;

//  f_open(&g_oEeFile, "/EE.txt", FA_OPEN_ALWAYS | FA_WRITE) ;

//	command[0] = 3 ;
//	command[1] = 0 ;
//	command[2] = 0 ;
//	command[3] = 0 ;

//	for ( i = 0 ; i < 256 ; i += 1 )
//	{
//		command[2] = i ;
//		eespi_operation( command, Eedata, 4, 256 ) ;
//		fresult = f_write( &g_oEeFile, Eedata, 256, &nwritten ) ;
//		wdt_reset() ;
//	}
//  f_close(&g_oEeFile) ;
	 
//}
#endif

//uint32_t SaveHeap[4] ;

void xmain( void ) ;
#ifdef STACK_PROBES
uint32_t StackAtOsStart ;
#endif

#if defined(PCBX9D) || defined(PCB9XT)
 #ifndef REV19
void __set_MSP(uint32_t topOfMainStack)
{
  __ASM volatile ("MSR msp, %0\n\t" : : "r" (topOfMainStack) );
}
 #endif
//uint32_t __get_MSP(void)
//{
//	__ASM volatile ("mrs r0, msp\n") ;
//}
#endif

#ifndef SMALL
int main( void )
{
#ifdef STACK_PROBES
	uint32_t i ;
	for ( i = 0 ; i < MAIN_STACK_REQUIRED ; i += 1 )
	{
		MainStack[i] = 0x55555555 ;
	}
#endif
//	__set_MSP((uint32_t) &MainStack[MAIN_STACK_REQUIRED]) ;
	xmain() ;
}
#endif

#ifndef SMALL
void xmain( void )
#else
int main( void )
#endif
{
#ifdef PCBX9D
 #ifdef LUA
//void enableBackupRam()
	RCC->APB1ENR |= RCC_APB1ENR_PWREN ;
	PWR->CR |= PWR_CR_DBP ;
	RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN ;
	uint32_t tempi ;
	uint32_t *p = (uint32_t *) BKPSRAM_BASE ;
	for ( tempi = 0 ; tempi < 1024 ; tempi += 1 )
	{
		*p++ = 0 ;
	}
 #endif	
#endif	
//extern unsigned char *heap ;
//	SaveHeap[0] = (uint32_t)heap ;
#ifdef PCBLEM1	
	AFIO->MAPR = AFIO_MAPR_SWJ_CFG_DISABLE ;
	NVIC->ICER[0] = 0xFFFFFFFF ;
	NVIC->ICER[1] = 0xFFFFFFFF ;
	NVIC->ICER[2] = 0xFFFFFFFF ;
	NVIC->ICER[3] = 0xFFFFFFFF ;
	NVIC->ICER[4] = 0xFFFFFFFF ;
	NVIC->ICER[5] = 0xFFFFFFFF ;
	NVIC_SetPriorityGrouping( 3 ) ;
	SysTick->CTRL = 0 ;
	ledInit() ;
	ledGreen() ;
#endif

#if defined(PCBTX16S)
//	testFD() ;
#endif


#ifdef WDOG_REPORT
#ifdef PCBSKY	
	WdogIntValue = GPBR->SYS_GPBR1 ;
	GPBR->SYS_GPBR1 = 0 ;
#else
	RCC->APB1ENR |= RCC_APB1Periph_PWR ;
//  PWR_BackupAccessCmd(ENABLE);
	WdogIntValue = RTC->BKP1R ;
	RTC->BKP1R = 0 ;
#endif
#endif

#ifdef PCB9XT
//	enableBackupRam() ;
//void disableBackupRam()
	{
		PWR->CR |= PWR_CR_DBP ;
		PWR->CSR &= ~PWR_CSR_BRE ;
//		while ( ( PWR->CSR & PWR_CSR_BRR) == 0 )
//		{
//			wdt_reset() ;
//		}
		RCC->AHB1ENR &= ~RCC_AHB1ENR_BKPSRAMEN ;
	}
#endif
#ifdef STARTUP_DEBUG	 
	startupDebugInit() ;
#endif  	 
	 
#ifdef PCBX7
 #if defined(PCBX7ACCESS)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
	configure_pins( GPIO_Pin_1, PIN_PORTB | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
	configure_pins( GPIO_Pin_4, PIN_PORTC | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
 #else
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
	// Only configure green LED, allow others to be used as analog inputs
	configure_pins( GPIO_Pin_4, PIN_PORTC | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
 #endif
#ifdef PCBT12
	configure_pins( GPIO_Pin_5, PIN_PORTC | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
	configure_pins( GPIO_Pin_1, PIN_PORTB | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
#endif
//	ledOff() ;
//	ledRed() ;
	init_soft_power() ;
#endif // PCBX7

#ifdef PCBX9LITE
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
	configure_pins( LED_GREEN_GPIO_PIN | LED_RED_GPIO_PIN | LED_BLUE_GPIO_PIN, PIN_PORTE | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
#endif // PCBX9LITE

#ifdef PCBXLITE

#ifndef PCBXLITES
	configure_pins( LED_GREEN_GPIO_PIN, PIN_PORTE | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
#endif
#ifdef PCBXLITES
	configure_pins( LED_BLUE_GPIO_PIN, PIN_PORTE | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
#endif

	init_soft_power() ;
#endif // PCBXLITE


#if defined(PCBX12D) || defined(PCBX10)
  RCC_AHB1PeriphClockCmd(PWR_RCC_AHB1Periph | LCD_RCC_AHB1Periph | KEYS_RCC_AHB1Periph_GPIO | ADC_RCC_AHB1Periph | SERIAL_RCC_AHB1Periph | TELEMETRY_RCC_AHB1Periph | AUDIO_RCC_AHB1Periph | HAPTIC_RCC_AHB1Periph, ENABLE);
  RCC_APB1PeriphClockCmd(INTERRUPT_5MS_APB1Periph | TIMER_2MHz_APB1Periph | SERIAL_RCC_APB1Periph | TELEMETRY_RCC_APB1Periph | AUDIO_RCC_APB1Periph, ENABLE);
  RCC_APB2PeriphClockCmd(LCD_RCC_APB2Periph | ADC_RCC_APB2Periph | HAPTIC_RCC_APB2Periph, ENABLE);

	// PCB rev pin
	configure_pins( PCBREV_GPIO_PIN, PCBREV_PORT | PIN_INPUT | PIN_PULLUP ) ;

	ledInit() ;
#if defined(PCBTX16S)
	ledGreen() ;
#else
	ledRed() ;
#endif
//extern void backlightInit(void) ;
//extern void backlightEnable(uint8_t dutyCycle) ;
//	backlightInit() ;
//	backlightEnable(0) ;
//
#endif // PCBX12D
	
	uint32_t i ;
#if defined(PCBX12D) || defined(PCBX10)
	for ( i = 0 ; i < 91 ; i += 1 )
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	for ( i = 0 ; i < 81 ; i += 1 )
#endif
#ifdef PCBSKY
	for ( i = 0 ; i < 35 ; i += 1 )
#endif
#ifdef PCBLEM1	
	for ( i = 0 ; i < 76 ; i += 1 )
#endif
	{
		NVIC->IP[i] = 0x80 ;
	}

#if defined(PCBX12D) || defined(PCBX10)
 #if not defined(PCBTX16S)
extern void CheckForPrototype(void) ;
	CheckForPrototype() ;
 #endif
#endif

#ifdef PCBSKY
	register Pio *pioptr ;
	module_output_low() ;
#endif

#ifdef PCBSKY
	ResetReason = RSTC->RSTC_SR ;
	ChipId = CHIPID->CHIPID_CIDR ;
#ifndef SMALL
 #ifdef LUA
extern unsigned char *EndOfHeap ;
	if ( ChipId & 0x0080 )
	{
		EndOfHeap = (unsigned char *)0x20020000L ;
		HwDelayScale = 2 ;
	}
 #endif
#endif

#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBXLITE) || defined(PCBX10)
	ChipId = *((uint16_t *)0x1FFF7A22) ;
	ResetReason = RCC->CSR ;
  RCC->CSR |= RCC_CSR_RMVF ;
#ifdef PCB9XT
	if ( ResetReason & RCC_CSR_WDGRSTF ) // watchdog
	{
		uint8_t *p = (uint8_t *) BKPSRAM_BASE ;
		WatchdogPosition = *p ;
		*p = 0 ;
	}
#endif	 
#endif

#ifdef PCBLEM1
	ChipId = *((uint16_t *)0x1FFFF7E0) ;
	ResetReason = RCC->CSR ;
  RCC->CSR |= RCC_CSR_RMVF ;
#endif

#ifdef PCBSKY
  PMC->PMC_PCER0 = (1<<ID_PIOC)|(1<<ID_PIOB)|(1<<ID_PIOA)|(1<<ID_UART0) ;				// Enable clocks to PIOB and PIOA and PIOC and UART0
	
	MATRIX->CCFG_SYSIO |= 0x000010F0L ;		// Disable syspins, enable B4,5,6,7,12

// Configure the ERASE pin as an output, low for Bluetooth use
//	pioptr = PIOB ;
//	pioptr->PIO_CODR = PIO_PB12 ;		// Set bit B12 LOW
//	pioptr->PIO_PER = PIO_PB12 ;		// Enable bit B12 (ERASE)
//	pioptr->PIO_OER = PIO_PB12 ;		// Set bit B12 as output

	pioptr = PIOA ;

 #ifndef REVA
	init_soft_power() ;
 #else	
	// On REVB, PA21 is used as AD8, and measures current consumption.
	pioptr->PIO_PER = PIO_PA21 ;		// Enable bit A21 (EXT3)
	pioptr->PIO_OER = PIO_PA21 ;		// Set bit A21 as output
	pioptr->PIO_SODR = PIO_PA21 ;	// Set bit A21 ON
 #endif
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	init_soft_power() ;
#endif

//#ifdef PCB9XT
//// Configure pin PA5 as an output, low for Bluetooth use
//	configure_pins( GPIO_Pin_5, PIN_PORTA | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
//	GPIOA->BSRRH = GPIO_Pin_5 ;		// Set low
//#endif

//#ifdef PCBX7
//// Configure pin PA5 as an output, low for Bluetooth use
//	configure_pins( GPIO_Pin_12, PIN_PORTE| PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
//	GPIOE->BSRRH = GPIO_Pin_12 ;		// Set low
//#endif

#ifdef BLUETOOTH
	initBluetooth() ;
#endif

//#if defined(PCBX9D) || defined(PCB9XT)
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
// #if defined(PCBX12D)
//extern void initLongWatchdog(uint32_t time) ;
//	initLongWatchdog(2) ;
// #else
	initWatchdog() ;
// #endif
#endif

#ifdef PCBSKY
	pioptr = PIOC ;
	pioptr->PIO_PER = PIO_PC25 ;		// Enable bit C25 (USB-detect)

	if ( ( pioptr->PIO_PDSR & 0x02000000 ) == 0 )
	{
		// USB not the power source
		WDT->WDT_MR = 0x3FFF207F ;				// Enable watchdog 0.5 Secs
//		WDT->WDT_MR = 0x3FFF107F ;				// Enable watchdog 0.5 Secs as an interrupt
//		NVIC_SetPriority( WDT_IRQn, 0 ) ; // Not quite highest priority interrupt
//		NVIC_EnableIRQ( WDT_IRQn) ;
	}

#ifdef REVA
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

//#ifdef PCBX9D
// #ifdef LATENCY
//	configure_pins( 0x6000, PIN_OUTPUT | PIN_PORTA | PIN_PUSHPULL | PIN_OS25 | PIN_NO_PULLUP ) ;
// #endif
//#endif

	setup_switches() ;

//#ifdef PCBX7
//	configure_pins( PIN_SW_EXT1, PIN_LOW | PIN_OUTPUT | PIN_PULLUP | PIN_PORTC ) ;
//	configure_pins( PIN_SW_EXT2, PIN_LOW | PIN_OUTPUT | PIN_PULLUP | PIN_PORTD ) ;
//#endif

#ifdef PCBSKY
  // Enable PCK2 on PB3, This is for testing of Timer 2 working
	// It will be used as serial data to the Bluetooth module
	pioptr->PIO_ABCDSR[0] |=  PIO_PB3 ;	// Peripheral B
  pioptr->PIO_ABCDSR[1] &= ~PIO_PB3 ;	// Peripheral B
  pioptr->PIO_PDR = PIO_PB3 ;					// Assign to peripheral
	PMC->PMC_SCER |= 0x0400 ;								// PCK2 enabled
	PMC->PMC_PCK[2] = 2 ;										// PCK2 is PLLA

	com2_Configure( CONSOLE_BAUDRATE, SERIAL_NORM, SERIAL_NO_PARITY ) ;
#endif

#ifdef PCBX9D
 #ifndef PCBX7
  #ifndef PCBXLITE
   #ifndef PCBX9LITE
	ConsoleInit() ;
   #endif // PCBX9LITE
  #endif // PCBXLITE
 #endif // PCBX7
#endif
#ifdef PCB9XT
	consoleInit() ;
#endif
	
	init5msTimer() ;
#ifdef PCBXLITE
  WatchdogTimeout = 200 ;
#else
  WatchdogTimeout = 100 ;
#endif

	init_hw_timer() ;

#if defined(PCBX9LITE)
	x9lCheckSportEnable() ;
#endif
	 
//#ifdef PCB9XT
//	// Test PPM output signal
//	// On port A pin 7
//	configure_pins( PIN_EXTPPM_OUT, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
//	GPIOA->BSRRL = PIN_EXTPPM_OUT ;		// High
//	hw_delay( 40 ) ; // units of 0.1uS
//	PpmTestH = GPIOA->IDR & 0x0080 ;
//	GPIOA->BSRRH = PIN_EXTPPM_OUT ;		// Low
//	hw_delay( 40 ) ; // units of 0.1uS
//	PpmTestL = GPIOA->IDR & 0x0080 ;
//#endif

//#ifdef REVX
//	sportTest() ;
//#endif

#if defined(PCBX12D) || defined(PCBX10)
	__enable_irq() ;
#endif
	init_adc() ;

#ifdef PCBX9D
	I2C_EE_Init() ;
 #ifndef PCBX7
	setVolume( 0 ) ;
 #endif // PCBX7
#endif

#ifdef PCBLEM1
	init_I2C2() ;
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
#ifndef PCBX7
#ifndef REV9E
#ifndef PCBXLITE
#ifndef PCBX9LITE
	disk_initialize( 0 ) ;
	sdInit() ;
#endif
#endif
#endif
#endif
#endif

#if defined(PCBX12D) || defined(PCBX10)
	I2C_EE_Init() ;
	SD_Init() ;	// low level
	sdInit() ;	// mount

//void enableBackupRam(void) ;
//	enableBackupRam() ;

void disableBackupRam(void) ;
	disableBackupRam() ;

#endif

	__enable_irq() ;

#ifdef PCB9XT
	delay_setbl( 100, 0, 0 ) ;
#endif

#if defined(PCBX12D) || defined(PCBX10)

//#if defined(PCBX10)
//	uint32_t pq ;
//	pq = 0 ;
//	for( pq = 0 ; pq < 3 ; pq += 1 )
//	{
//		uint32_t i ;
//		ledRed() ;
//		for ( i = 0 ; i < 2000000 ; i += 1 )
//		{
//			wdt_reset() ;
//			asm("nop") ;
//		}
//		ledBlue() ;
//		for ( i = 0 ; i < 2000000 ; i += 1 )
//		{
//			wdt_reset() ;
//			asm("nop") ;
//		}
//	}
//#endif
	wdt_reset() ;
	
//	lcdColorsInit() ;
#endif
	
	lcdInit() ;

#if defined(PCBX12D) || defined(PCBX10)
	init_trims() ;
#endif

#if defined(PCBT16)
  uint32_t tms = 15 ;
	while (tms--)
	{
		hw_delay( 10000 ) ; // units of 0.1uS
  }
#endif

#if defined(PCBX12D) || defined(PCBX10)
#ifdef WHERE_TRACK
notePosition('3') ;
#endif
//extern uint16_t LCDLastOp ;
//	LCDLastOp = 'M' ;
	lcdDrawSolidFilledRectDMA( 0, 0, 480, 272, 0x001F ) ;
	refreshDisplay() ;

	uint32_t j ;
	j = 0 ;
	for( j = 0 ; j < 3 ; j += 1 )
	{
		uint32_t i ;
		ledRed() ;
		for ( i = 0 ; i < 100000 ; i += 1 )
		{
			asm("nop") ;
		}
		ledBlue() ;
		for ( i = 0 ; i < 100000 ; i += 1 )
		{
			asm("nop") ;
		}
	}
 #if defined(PCBTX16S)
	ledGreen() ;
 #else
	ledRed() ;
 #endif
#endif


#if defined(PCBX7) || defined (PCBXLITE) || defined (REV9E) || defined (PCBX9LITE) || defined(PCBLEM1)
//	ledOff() ;
//	ledBlue() ;
	init_trims() ;
#if defined(PCBLEM1)
	// need to wait at least 10 mS for trims to be valid
  uint32_t ms = 15 ;
	while (ms--)
	{
		hw_delay( 10000 ) ; // units of 0.1uS
  }
#endif

#if defined(PCBX9D) || defined(PCB9XT)
	if ( ( ResetReason & ( RCC_CSR_WDGRSTF | RCC_CSR_SFTRSTF ) ) == 0 ) // Not watchdog or soft reset
#endif
#if defined(PCBLEM1)
	if ( ( ResetReason & ( RCC_CSR_IWDGRSTF | RCC_CSR_SFTRSTF ) ) == 0 ) // Not watchdog or soft reset
#endif
	{
		uint8_t dtimer = 0 ;
#if defined(PCBT12) || defined(PCBLEM1)
		uint32_t dbounceTimer = 0 ;
#endif
		while ( get_tmr10ms() < 150 )
		{
			uint32_t switchValue ;
#if defined(PCBLEM1)
			switchValue = ( GPIOE->IDR & 0x0800 ) | ( DC_POWER_GPIO->IDR & DC_POWER_Pin ) ;
#else
			switchValue = GPIO_ReadInputDataBit(GPIOPWRSENSE, PIN_PWR_STATUS) == Bit_RESET ;
#endif
			wdt_reset() ;
	
			if ( get_tmr10ms() != dtimer )
			{
				dtimer = get_tmr10ms() ;
				if ( dtimer & 1 )
				{
					lcd_clear() ;
#ifdef PROP_TEXT
					PUTS_ATT( 4*FW, 3*FH, "STARTING", DBLSIZE ) ;
#else
					PUTS_ATT( 3*FW, 3*FH, "STARTING", DBLSIZE ) ;
#endif
					lcd_hbar( 13, 49, 102, 6, dtimer * 100 / 150 ) ;
					refreshDisplay() ;
				}
			}		 

			if ( !switchValue )
			{
#if defined(PCBLEM1)
#endif
#if defined(PCBT12) || defined(PCBLEM1)
			 if (++dbounceTimer > 3 )
			 {
#endif
				// Don't power on
				soft_power_off() ;		// Only turn power off if necessary
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
#ifndef REV9E
 #ifndef PCBLEM1
					switchValue = GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) == Bit_RESET ;
 #else
					switchValue = ( GPIOE->IDR & 0x0800 ) ;
 #endif

					if ( switchValue )
					{
		  			NVIC_SystemReset() ;
					}
#endif
				}
#if defined(PCBT12) || defined(PCBLEM1)
			 }
#endif
			}
#if defined(PCBT12) || defined(PCBLEM1)
			else
			{
				dbounceTimer = 0 ;
			}
#endif
#ifdef PCBXLITE
			if ( ( ( read_trims() & 0x01 ) == 0x01 ) && ( (GPIOE->IDR & 0x0100) == 0 ) )
			{
				break ;
			}
#else	
			if ( ( read_trims() & 0x80 ) == 0x80 )
			{
//				if ( ( read_trims() & 0x81 )== 0x81 )
//				{				
//					break ; // To maintenance mode
//				}
//				if ( get_tmr10ms() > 80 )
//				{
				break ;
//				}
			}	
#endif
		}
	}
#ifndef REV9E
 #ifndef PCBLEM1
  #ifdef PCBX9LITE
   #if defined(X9LS)
	ledBlue() ;
   #else
	ledRed() ;
   #endif
  #else
   #ifndef PCBX7ACCESS
    #ifdef PCBXLITES
	ledBlue() ;
	  #else
	ledGreen() ;
	  #endif
   #else
	ledBlue() ;
   #endif
  #endif
 #endif
#endif
#ifdef PCBT12
	ledBlue() ;	// Green Led!
#endif
#endif // PCBX7 | PCBXLITE

//#if defined(PCBX10)
//extern void wdDump() ;
//	wdDump() ;
//#endif


//#ifdef REV9E
//// Check for real power on
//	if ( ( ResetReason & ( RCC_CSR_WDGRSTF | RCC_CSR_SFTRSTF ) ) == 0 ) // Not watchdog or soft reset
//	{
//		uint8_t dtimer = 0 ;
//		while ( get_tmr10ms() < 150 )
//		{
//			uint32_t switchValue ;
//			switchValue = GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) == Bit_RESET ;
//			wdt_reset() ;

//			if ( get_tmr10ms() != dtimer )
//			{
//				dtimer = get_tmr10ms() ;
//				if ( dtimer & 1 )
//				{
//					lcd_clear() ;
//					lcd_putsAtt( 3*FW, 3*FH, "STARTING", DBLSIZE ) ;
//					lcd_hbar( 13, 49, 102, 6, dtimer * 100 / 150 ) ;
//					refreshDisplay() ;
//				}
//			}		 

//			if ( !switchValue )
//			{
//				// Don't power on
//				soft_power_off() ;		// Only turn power off if necessary
//				for(;;)
//				{
//					wdt_reset() ;
// 					PWR->CR |= PWR_CR_CWUF;
// 					/* Select STANDBY mode */
// 					PWR->CR |= PWR_CR_PDDS;
// 					/* Set SLEEPDEEP bit of Cortex System Control Register */
// 					SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
// 					/* Request Wait For Event */
// 					__WFE();
//				}
//			}
//		}
//	}
//#endif

#if defined(PCBX12D) || defined(PCBX10)
// Check for real power on
//		backlight_set( 0 ) ;
//		lcd_clearBackground() ;	// Start clearing other frame
//		waitLcdClearDdone() ;
//		lcd_putsAtt( 3*FW, 3*FH, "STARTING", DBLSIZE ) ;
//		refreshDisplay() ;
	
	if ( ( ResetReason & ( RCC_CSR_WDGRSTF | RCC_CSR_SFTRSTF ) ) == 0 ) // Not watchdog or soft reset
	{
		backlight_set( 0 ) ;
	  uint16_t tgtime = get_tmr10ms() ;		// 1 sec
		uint8_t dtimer ;

		dtimer = tgtime ;
//		lcd_clear() ;
//		lcd_putsAtt( 3*FW + X12OFFSET, 3*FH, "STARTING", DBLSIZE ) ;
//		refreshDisplay() ;


		while ( (uint16_t)(get_tmr10ms() - tgtime ) < 70 )
//		while ( get_tmr10ms() < 25 )
		{
			uint32_t switchValue ;
			switchValue = GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) == Bit_RESET ;
			wdt_reset() ;
			uint16_t now = get_tmr10ms() ;
			if ( now != dtimer )
			{
				dtimer = now ;
				if ( ( dtimer & 1) == 0 )
				{
					LcdBackground = LCD_GREEN ;
					lcd_clear() ;
					PUTS_ATT( 3*FW + X12OFFSET, 3*FH, "STARTING", DBLSIZE ) ;
					lcd_hbar( 13 + X12OFFSET, 49, 102, 6, (dtimer - tgtime) * 1000 / (97*7) ) ;
					refreshDisplay() ;
				}
			}
			
			if ( !switchValue )
			{
				// Don't power on
				soft_power_off() ;		// Only turn power off if necessary
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
			}
			
			if ( ( read_trims() & 0x80 ) == 0x80 )
			{
				break ;
			}	
		}
	}
	else
	{
#define SOFTRESET_REQUEST 0xCAFEDEAD
		RCC->APB1ENR |= RCC_APB1Periph_PWR ;
//  PWR_BackupAccessCmd(ENABLE);
  	RCC_RTCCLKCmd(ENABLE);

#define PWR_OFFSET               (PWR_BASE - PERIPH_BASE)
#define CR_OFFSET                (PWR_OFFSET + 0x00)
#define DBP_BitNumber            0x08
#define CR_DBP_BB                (PERIPH_BB_BASE + (CR_OFFSET * 32) + (DBP_BitNumber * 4))
		*(__IO uint32_t *) CR_DBP_BB = ENABLE ;
	
		if ( ResetReason & RCC_CSR_SFTRSTF ) // Soft reset
		{
			if ( RTC->BKP0R == SOFTRESET_REQUEST )
			{
				ResetReason &= ~RCC_CSR_SFTRSTF ;
			}
		}
		RTC->BKP0R = 0 ;
	}

#endif

#ifdef REV9E
	disk_initialize( 0 ) ;
	sdInit() ;
#endif
#if defined(PCBX7) || defined (PCBXLITE) || defined (PCBX9LITE)
	disk_initialize( 0 ) ;
	sdInit() ;
#endif

#ifdef PCBX9D
  lcd_clear() ;
	refreshDisplay() ;
#endif

	g_menuStack[0] =  menuProc0 ;

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

//	uint16_t timer = 0 ;
//  configure_pins( GPIO_Pin_9, PIN_OUTPUT | PIN_PORTB | PIN_OS25 | PIN_PUSHPULL ) ;

//	for(;;)
//	{
//		GPIOB->BSRRL = GPIO_Pin_9 ;

//		lcd_clear() ;
//		lcd_putsAtt( 3*FW, 3*FH, "HIGH", DBLSIZE ) ;
//		refreshDisplay() ;
//		for ( timer = 0 ; timer < 500 ;  )
//		{
//			if ( Tenms )
//			{
//				Tenms = 0 ;
//				timer += 1 ;
//				wdt_reset() ;
//			}
//  	  if ( keyDown() )
//			{
//				break ;
//			}
//		}
// 	  if ( keyDown() )
//		{
//			break ;
//		}
	
//		GPIOB->BSRRH = GPIO_Pin_9 ;
//		lcd_clear() ;
//		lcd_putsAtt( 3*FW, 3*FH, "LOW", DBLSIZE ) ;
//		refreshDisplay() ;
	
//		for ( timer = 0 ; timer < 500 ;  )
//		{
//			if ( Tenms )
//			{
//				Tenms = 0 ;
//				timer += 1 ;
//				wdt_reset() ;
//			}
//  	  if ( keyDown() )
//			{
//				break ;
//			}
//		}
// 	  if ( keyDown() )
//		{
//			break ;
//		}
	
	
//	}
//	clearKeyEvents() ;
//	GPIOB->BSRRL = GPIO_Pin_9 ;
//  configure_pins( GPIO_Pin_9, PIN_PERIPHERAL | PIN_PORTB | PIN_PER_2 | PIN_OS25 | PIN_PUSHPULL ) ;

//#endif

//#ifdef PCB9XT

//#define BACKLIGHT_TEST	1
 
// #ifdef BACKLIGHT_TEST
//	{
//		BlSetColour( 0, 3 ) ;	

//		for(;;)
//		{
//			uint16_t timer = 0 ;
////			for ( timer = 0 ; timer < 2 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;}}
//			BlSetColour( 100, BL_RED ) ;	
//			for ( timer = 0 ; timer < 200 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;wdt_reset() ;}}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}
//			BlSetColour( 0, BL_RED ) ;	
////			for ( timer = 0 ; timer < 2 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;}}
//			BlSetColour( 100, BL_GREEN ) ;	
//			for ( timer = 0 ; timer < 200 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;wdt_reset() ;}}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}
//			BlSetColour( 0, BL_GREEN ) ;	
////			for ( timer = 0 ; timer < 2 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;}}
//			BlSetColour( 100, BL_BLUE ) ;	
//			for ( timer = 0 ; timer < 200 ;  ) { if ( Tenms ){Tenms = 0 ;timer += 1 ;wdt_reset() ;}}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}

//			for ( timer = 100 ; timer > 0 ;  )
//			{
//				if ( Tenms )
//				{
//					Tenms = 0 ;
//					timer -= 1 ;
//					BlSetColour( timer, BL_BLUE ) ;	
//					wdt_reset() ;
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
//					wdt_reset() ;
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
//					wdt_reset() ;
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
//					wdt_reset() ;
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
//					wdt_reset() ;
//				}
//			}
//			if ( check_soft_power() == POWER_OFF ){soft_power_off() ;}
//		}
//	}	
// #endif
//#endif

#ifdef PCBX9D
	init_trims() ;
//#ifndef PCBXLITE
	initHaptic() ;
//#endif  	 
	start_2Mhz_timer() ;
	setVolume( 0 ) ;
	start_sound() ;
#endif

#ifdef PCB9XT
	init_trims() ;
	start_2Mhz_timer() ;
	start_sound() ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
	initHaptic() ;
	start_2Mhz_timer() ;
	setVolume( 0 ) ;
	start_sound() ;
		
#endif

#ifdef PCBLEM1
	init_trims() ;
	start_2Mhz_timer() ;
	setVolume( 5 ) ;
	start_sound() ;
#endif

#if defined(PCBSKY) || defined(PCB9XT)
	init_spi() ;
	init_eeprom() ;	
#endif

#ifdef PCBSKY
	start_ppm_capture() ;	// 2MHz timer
	end_ppm_capture() ;
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

#ifdef PCBXLITE
  WatchdogTimeout = 200 ;
#endif
	 
#if defined(PCBX12D) || defined(PCBX10)
	WatchdogTimeout = 300 ;
#endif

#ifdef PCBLEM1
	start_2Mhz_timer() ;
	
	SD_Init() ;
extern void sdInit( void ) ;
	sdInit() ;	// Mount
	disk_initialize( 0 ) ;
#endif

#ifdef PCBLEM1
	CoInitOS() ;
#endif
#if defined(PCBSKY) || defined(PCB9XT)
	lcd_clear() ;
	refreshDisplay() ;
	PUTS_ATT_LEFT( FH, XPSTR("EEPROM Check") ) ;
	lcd_hbar( 4, 6*FH+4, 64, 7, 0 ) ;
	refreshDisplay() ;
#endif
	
#if defined(PCBX12D) || defined(PCBX10)
	uint32_t needReadEeprom = 1 ;
	if ( ResetReason & RCC_CSR_WDGRSTF ) // watchdog
	{
extern uint32_t checkCCRam() ;
		if ( checkCCRam() )
		{
extern uint32_t readGeneralFromCCRam() ;
extern uint32_t readModelFromCCRam() ;
extern uint32_t readNamesFromCCRam() ;
			readGeneralFromCCRam() ;
			readModelFromCCRam() ;
			readNamesFromCCRam() ;
			needReadEeprom = 0 ;

// Temp, part of eeReadAll
			uint32_t red ;
			uint32_t green ;
			uint32_t blue ;
	
			red = g_eeGeneral.backgroundColour >> 11 ;
			green = ( g_eeGeneral.backgroundColour >> 6 ) & 0x3F ;
			blue = g_eeGeneral.backgroundColour & 0x1F ;

			if ( (red < 6) && (green < 6) && (blue < 6) )
			{
				g_eeGeneral.backgroundColour = LCD_BACKGROUND ;
			}
  		LcdBackground = g_eeGeneral.backgroundColour ;
			LcdForeground = g_eeGeneral.textColour ;
		
		
		}
	}
	if ( needReadEeprom  )
#endif
	{	
		eeReadAll() ;
#if defined(PCBX12D) || defined(PCBX10)

// Save in case of watchdog reboot

void writeGeneralToCCRam() ;
void writeModelToCCRam( uint8_t index ) ;
void writeNamesToCCRam() ;

		writeGeneralToCCRam() ;
		writeModelToCCRam(g_eeGeneral.currModel) ;
		writeNamesToCCRam() ;

#endif
	}

#if defined(PCBX7) || defined (PCBXLITE) || defined (PCBT12) || defined (PCBX9LITE)
	g_eeGeneral.softwareVolume = 1 ;
#endif // PCBX7
	protocolsToModules() ;
#ifdef PCBSKY
 #ifndef SMALL
	checkAr9x() ;
 #else	
	g_eeGeneral.ar9xBoard = 0 ;
 #endif
#endif

//#if defined(PCBX7ACCESS)
// #ifdef WDOG_REPORT
//	if ( ( ( ResetReason & RCC_CSR_WDGRSTF ) == RCC_CSR_WDGRSTF ) || unexpectedShutdown )	// watchdog
//	{
//		g_model.Module[0].protocol = PROTO_OFF ;
//	}
// #endif
//#endif

#if defined(PCBX12D) || defined(PCBX10)
	g_eeGeneral.softwareVolume = 1 ;
#endif

#ifdef PCBLEM1
	g_eeGeneral.softwareVolume = 1 ;
#endif

	 
#ifdef PCB9XT
	g_eeGeneral.physicalRadioType = PHYSICAL_9XTREME ;
#endif
#ifdef PCBSKY
 #ifndef REVX
	g_eeGeneral.physicalRadioType =	g_eeGeneral.ar9xBoard ? PHYSICAL_AR9X : PHYSICAL_SKY ;
 #else
	g_eeGeneral.physicalRadioType = PHYSICAL_9XRPRO ;
 #endif
#endif

#ifdef PCBX9D
 #if defined(REVPLUS) || defined(REV9E)
  #ifdef REV9E
		g_eeGeneral.physicalRadioType = PHYSICAL_TARANIS_X9E ;
  #else
		g_eeGeneral.physicalRadioType = PHYSICAL_TARANIS_PLUS ;
  #endif
 #else
  #ifdef PCBX7
   #ifdef PCBT12
		g_eeGeneral.physicalRadioType = PHYSICAL_T12 ;
	 #else
		g_eeGeneral.physicalRadioType = PHYSICAL_QX7 ;
   #endif
  #else
   #ifdef PCBXLITE
		g_eeGeneral.physicalRadioType = PHYSICAL_XLITE ;
   #else
    #ifdef PCBX9LITE
		g_eeGeneral.physicalRadioType = PHYSICAL_X9LITE ;
    #else
		g_eeGeneral.physicalRadioType = PHYSICAL_TARANIS ;
    #endif
   #endif
  #endif
 #endif
#endif

#if defined(PCBX10)
 #if defined(PCBT16)
  #if defined(PCBT18)
	g_eeGeneral.physicalRadioType = PHYSICAL_TX18S ;
  #else
   #if defined(PCBTX16S)
	g_eeGeneral.physicalRadioType = PHYSICAL_TX16S ;
   #else
	g_eeGeneral.physicalRadioType = PHYSICAL_T16 ;
   #endif
  #endif
 #else
  #if defined(PCBREV_EXPRESS)
	g_eeGeneral.physicalRadioType = PHYSICAL_X10E ;
  #else
	g_eeGeneral.physicalRadioType = PHYSICAL_X10 ;
  #endif
 #endif
#endif

#if defined(PCBX12D)
	g_eeGeneral.physicalRadioType = PHYSICAL_HORUS ;
#endif
	 
#ifdef PCBLEM1
	g_eeGeneral.physicalRadioType = PHYSICAL_LEM1 ;
#endif

	SportStreamingStarted = 0 ;
	setLanguage() ;
	lcdSetRefVolt(g_eeGeneral.contrast) ;


#ifdef PCB9XT
	delay_setbl( 0, 100, 0 ) ;
#endif

#ifdef PCBX9D
#ifndef REV9E
#ifndef PCBXLITE
#ifndef GPIOENCODER
	init_adc2() ;
#endif
#endif // nPCBXLITE
#endif // nREV9E
#endif

#ifdef PCBX9LITE
	setup_switches() ;	// Again to configure extra switch inputs
	init_adc() ;	// Again to configure extra analog inputs
#endif

	createSwitchMapping() ;
#ifdef PCBSKY
#ifndef ARUNI
	create6posTable() ;
#endif
	init_rotary_encoder() ;
#endif 

#if defined(PCBX12D) || defined(PCBX10)
	create6posTable() ;
#endif 

#ifdef PCB9XT
	create6posTable() ;
	if ( g_eeGeneral.enableI2C == 1 )
	{
		init_I2C2() ;
	}
#endif 

#ifdef PCBX9D
	create6posTable() ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
	checkTheme( &g_eeGeneral.theme[g_eeGeneral.selectedTheme] ) ;
#endif
#if defined(PCBX12D) || defined(PCBX10) || defined(TOUCH)
	dimBackColour() ;
#endif

#ifdef PCBX7
 #ifndef PCBT12
	init_rotary_encoder() ;
 #endif
#endif // PCBX7
#ifdef PCBX9LITE
	init_rotary_encoder() ;
#endif // PCBX9LITE
#ifdef REV9E
	init_rotary_encoder() ;
#endif // REV9E
#if defined(PCBX12D) || defined(PCBX10)
	init_rotary_encoder() ;
 #if defined(PCBX10)
	g_eeGeneral.rotaryDivisor = 2 ;
 #endif // PCBX10
#endif // X12D
#ifdef PCBLEM1
	initSwitches() ;
	init_rotary_encoder() ;
#endif // PCBLEM1
#ifdef REV19
	init_rotary_encoder() ;
#endif // REV19


#ifdef PCB9XT
	delay_setbl( 100, 0, 100 ) ;
#endif
#if not (defined(PCBX10))
	com2Configure() ;
#endif
#ifdef PCB9XT
	delay_setbl( 100, 100, 0 ) ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
	// Delay to let switches and trims stabilise
  uint32_t ms = 20 ;
	while (ms--)
	{
		hw_delay( 10286 ) ; // units of 0.0972222uS
  }
 #endif

	// At this point, check for "maintenance mode"
#ifdef PCBSKY
	if ( ( ( read_trims() & 0x81 )== 0x81 ) || ( GPBR->SYS_GPBR0 == 0x5555AAAA ) )
#else
#ifdef PCBXLITE
	if ( ( ( read_trims() & 0x01 ) == 0x01 ) && ( (GPIOE->IDR & 0x0100) == 0 ) )
#else	
	if ( ( read_trims() & 0x81 )== 0x81 )
#endif
#endif
	{
		// Do maintenance mode
#ifdef PCBSKY
		GPBR->SYS_GPBR0 = 0 ;
#endif
#ifndef SIMU
		CoInitOS();
#if defined(PCBX12D) || defined(PCBX10)
	backlight_set( 0 ) ;
#endif

#ifndef PCBLEM1
		MainTask = CoCreateTask( maintenance_mode,NULL,5,&main_stk[MAIN_STACK_SIZE-1],MAIN_STACK_SIZE);
		CoStartOS();
		while(1) ;
#endif
#endif
	
	}

#ifdef PCBSKY
	lcdSetOrientation() ;
#endif

#ifdef PCB9XT
	backlight_on() ;
#endif
#ifdef PCBXLITE
	while ( ( read_trims() & 0x40 )== 0x40 )
#else
	while ( ( read_trims() & 0x01 )== 0x01 )
#endif
	{
		wdt_reset() ;
		HardwareMenuEnabled = 1 ;
	  WatchdogTimeout = 100 ;
#if defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
		lcd_clear() ;
#endif
		PUTS_ATT_LEFT( FH, XPSTR("Hardware Menu Enabled") ) ;

//#ifdef PCBTX16S
//  PUT_HEX4( 0,  3*FH, GPIOA->IDR & 0x40 ) ;
//  PUT_HEX4( 6*FW, 3*FH, read_trims() ) ;
//  PUT_HEX4( 12*FW, 3*FH, GPIOB->IDR ) ;
//  PUT_HEX4( 18*FW, 3*FH, GPIOC->IDR ) ;

//  lcd_outhex8( (uint16_t)0*FW, (uint16_t)4*FH, (uint32_t)GPIOA->MODER ) ;
//  lcd_outhex8( (uint16_t)12*FW, (uint16_t)4*FH, (uint32_t)GPIOA->PUPDR ) ;
//  lcd_outhex8( (uint16_t)24*FW, (uint16_t)4*FH, (uint32_t)GPIOA->AFR[0] ) ;
//#endif		
		
		refreshDisplay() ;

#ifdef PCBSKY
		if ( ( ( ResetReason & RSTC_SR_RSTTYP ) == (2 << 8) ) || unexpectedShutdown )	// watchdog
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
		if ( ( ( ResetReason & RCC_CSR_WDGRSTF ) == RCC_CSR_WDGRSTF ) || unexpectedShutdown )	// watchdog
#endif
#if defined(PCBLEM1)
		if ( ( ( ResetReason & RCC_CSR_IWDGRSTF ) == RCC_CSR_IWDGRSTF ) || unexpectedShutdown )	// watchdog
#endif
		{
			break ;
		}

		// For RM TX16S testing, break on EXIT
		if ( ( read_keys() & (0x02 << KEY_EXIT) ) == 0 )
		{
			break ;
		}

	}

#ifdef PCBSKY
 #ifdef REVX
  #ifdef SERIAL_TEST_PRO
	if ( ( read_trims() & 0x81 )== 0x80 )
	{
		// diagnostics
#ifndef SIMU
		CoInitOS();

		MainTask = CoCreateTask( diag_mode,NULL,5,&main_stk[MAIN_STACK_SIZE-1],MAIN_STACK_SIZE);
		CoStartOS();
		while(1) ;
#endif
	}
  #endif
 #endif
#endif

  resetTimers();
	if ( g_eeGeneral.unexpectedShutdown )
	{
		unexpectedShutdown = 1 ;
	}

#ifdef PCBSKY
	start_sound() ;
#ifdef REVX
	if ( g_model.telemetryRxInvert )
	{
		setMFP() ;
	}
	else
	{
		clearMFP() ;
	}
#endif
	setBtBaudrate( g_eeGeneral.bt_baudrate ) ;
	// Set ADC gains here
	set_stick_gain( g_eeGeneral.stickGain ) ;
#endif

#ifndef PCBLEM1
	telemetry_init( decodeTelemetryType( g_model.telemetryProtocol ) ) ;
#endif

#if defined(PCBSKY) || defined(PCB9XT)
#ifdef PCB9XT
	if ( (ResetReason & RCC_CSR_WDGRSTF) == 0 )	// not Watchdog
#else
	if ( ( ResetReason & RSTC_SR_RSTTYP ) != (2 << 8) )	// not Watchdog
#endif
	{
  	checkQuickSelect();
	}
#endif

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
#if defined(PCBLEM1)	
	if ( (ResetReason & RCC_CSR_IWDGRSTF) == 0 )	// not Watchdog
#else
	if ( (ResetReason & RCC_CSR_WDGRSTF) == 0 )	// not Watchdog
#endif
	{
  	uint8_t i = keyDown(); //check for keystate
		if ( ( i & 6 ) == 6 )
		{
			SystemOptions |= SYS_OPT_MUTE ;
  		while ( keyDown() )
			{
				wdt_reset() ;
				PUTS_ATT_LEFT( FH, XPSTR("Mute Activated") ) ;
				refreshDisplay() ;
			}
		}
	}
#endif

#ifdef PCBSKY
	PWM->PWM_CH_NUM[0].PWM_CDTYUPD = g_eeGeneral.bright ;
	MAh_used = g_eeGeneral.mAh_used ;

#ifdef USB_JOYSTICK
	startJoystick() ;
#endif

#endif
#ifdef PCBX9D
#if defined(REVPLUS) || defined(REV9E) || defined(REV19)
	backlight_set( g_eeGeneral.bright, 0 ) ;
	backlight_set( g_eeGeneral.bright_white, 1 ) ;
#else
	backlight_set( g_eeGeneral.bright ) ;
#endif
	usbInit() ;
#endif
#ifdef PCBLEM1
	backlight_set( g_eeGeneral.bright ) ;
#endif


#if defined(PCBX12D) || defined(PCBX10)
//	WatchdogTimeout = 200 ;			
	if ( ( ResetReason & ( RCC_CSR_WDGRSTF | RCC_CSR_SFTRSTF ) ) == 0 ) // Not watchdog or soft reset
	{
		IWDG->KR = 0x5555 ;		// Unlock registers
		IWDG->RLR = 2000 ;			// 2.0 seconds nominal
		wdt_reset() ;
  	usbInit() ;
		IWDG->KR = 0x5555 ;		// Unlock registers
		IWDG->RLR = 500 ;			// 0.5 seconds nominal
	}
	
	
 	backlight_set( 0 ) ;
 #if defined(PCBTX16S)
	if ( g_eeGeneral.bright > 30 )
 #else
	if ( g_eeGeneral.bright > 30 )
 #endif
	{
		backlight_set( 0 ) ;
	}
	else
	{
		backlight_set( g_eeGeneral.bright ) ;
	}
#endif

#ifdef PCB9XT
  usbInit() ;
//	backlight_on() ;
#endif

	uint16_t x ;
	x = g_eeGeneral.volume ;
	if ( g_model.anaVolume )	// Only check if on main screen
	{
		uint16_t divisor ;
#ifdef PCBSKY
		if ( g_model.anaVolume < 4 )
#endif
#ifdef PCBX9D
#if defined(PCBX9LITE)
		if ( g_model.anaVolume < 2 )
#else // X3
#if defined(PCBX7) || defined (PCBXLITE)
		if ( g_model.anaVolume < 3 )
#else // PCBX7
		if ( g_model.anaVolume < 5 )
#endif // PCBX7
#endif
#endif // X3
#if defined(PCBX12D) || defined(PCBX10)
		if ( g_model.anaVolume < 5 )
#endif
#ifdef PCBLEM1
		if ( g_model.anaVolume < 2 )
#endif		
		{
		  getADC_single() ;
			x = anaIn(g_model.anaVolume+3) ;
#ifdef PCBXLITE
			x = scaleAnalog( x, g_model.anaVolume+3) + RESX ;
#endif
			divisor = 2048 ;
			x = x * (NUM_VOL_LEVELS-1) / divisor ;
		}
		// Not checking for GVARS yet

	}
	setVolume( x ) ;

	// Choose here between PPM and PXX

	g_menuStack[1] = menuProcModelSelect ;	// this is so the first instance of [MENU LONG] doesn't freak out!

  //we assume that startup is like pressing a switch and moving sticks.  Hence the lightcounter is set
  //if we have a switch on backlight it will be able to turn on the backlight.
  if(g_eeGeneral.lightAutoOff > g_eeGeneral.lightOnStickMove)
    g_LightOffCounter = g_eeGeneral.lightAutoOff*500;
  else //if(g_eeGeneral.lightAutoOff <= g_eeGeneral.lightOnStickMove)
    g_LightOffCounter = g_eeGeneral.lightOnStickMove*500;
  check_backlight();

	// moved here and logic added to only play statup tone if splash screen enabled.
  // that way we save a bit, but keep the option for end users!
    
	if ( g_eeGeneral.welcomeType == 0 )
	{
		if(!g_eeGeneral.disableSplashScreen)
    {
			voiceSystemNameNumberAudio( SV_WELCOME, V_HELLO, AU_TADA ) ;
    }
	}
	else if ( g_eeGeneral.welcomeType == 2 )
	{
		putNamedVoiceQueue( (char *)g_eeGeneral.welcomeFileName, VLOC_USER ) ;
	}

#if defined(PCBX10) && defined(PCBREV_EXPRESS)		
//	dumpSerialFlash() ;
#endif

#ifndef SIMU

#ifndef PCBLEM1
	CoInitOS() ;
#endif

#ifdef STACK_PROBES
#ifdef MIXER_TASK
	for ( i = 0 ; i < MIXER_STACK_SIZE ; i += 1 )
	{
		Mixer_stk[i] = 0x55555555 ;
	}
#endif
#ifdef BLUETOOTH
	for ( i = 0 ; i < BT_STACK_SIZE ; i += 1 )
	{
		Bt_stk[i] = 0x55555555 ;
	}
#endif
	for ( i = 0 ; i < LOG_STACK_SIZE ; i += 1 )
	{
		Log_stk[i] = 0x55555555 ;
	}
	for ( i = 0 ; i < VOICE_STACK_SIZE ; i += 1 )
	{
		voice_stk[i] = 0x55555555 ;
	}
	for ( i = 0 ; i < MAIN_STACK_SIZE ; i += 1 )
	{
		main_stk[i] = 0x55555555 ;
	}
#endif

#ifdef BLUETOOTH
	BtTask = CoCreateTask(bt_task,NULL,19,&Bt_stk[BT_STACK_SIZE-1],BT_STACK_SIZE);
#endif

	MainTask = CoCreateTask( main_loop,NULL,5,&main_stk[MAIN_STACK_SIZE-1],MAIN_STACK_SIZE);

	LogTask = CoCreateTask(log_task,NULL,17,&Log_stk[LOG_STACK_SIZE-1],LOG_STACK_SIZE);

	VoiceTask = CoCreateTaskEx( voice_task,NULL,5,&voice_stk[VOICE_STACK_SIZE-1], VOICE_STACK_SIZE, 2, FALSE );

#ifdef MIXER_TASK
	MixerTask = CoCreateTask( mixer_loop,NULL,2,&Mixer_stk[MIXER_STACK_SIZE-1],MIXER_STACK_SIZE);
#endif

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

#if defined(PCBX12D) || defined(PCBX10)
 #ifdef WHERE_TRACK
	if ( ResetReason & RCC_CSR_WDGRSTF ) // watchdog
	{
		dumpPositions() ;
		initPosition() ;
	}
	else
	{
		initPosition() ;
	}
 #endif 
#endif 

#ifdef STACK_PROBES
 #ifndef SMALL
	StackAtOsStart = __get_MSP() ;
 #endif
#endif

#ifndef SMALL
// swap stack
 #if defined(PCBX9D) || defined(PCB9XT)
//	uint32_t stkp = __get_MSP() ;
//extern uint32_t _estack ;	
//	uint32_t amount = (uint32_t)&_estack - stkp ;
//	uint32_t newstkp = (uint32_t)&MainStack[MAIN_STACK_REQUIRED] - amount ;
	uint32_t newstkp = (uint32_t)&MainStack[MAIN_STACK_REQUIRED] ;
//	for ( uint32_t i = 0 ; i < amount/4 ; i += 1 )
//	{
//		((uint32_t *)newstkp)[i] = ((uint32_t *)stkp)[i] ;
//	}
	__set_MSP( newstkp ) ;
 #endif
#endif
	CoStartOS();

	while(1);
#endif
  /*
   * Prevent compiler warnings
   */

  /*
   * This return here make no sense.
   * But to prevent the compiler warning:
   * "return type of 'main' is not 'int'
   * we use an int as return :-)
   */
//  return(0);
}

#ifndef SIMU


extern const char *openLogs( void ) ;
extern void writeLogs( void ) ;
extern void closeLogs( void ) ;

uint8_t LogsRunning = 0 ;
uint32_t LogTimer = 0 ;
uint16_t LogCounter = 0 ;
extern uint8_t RawLogging ;
extern void rawStartLogging() ;

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
			if ( ( RawLogging ) && ( LogsRunning & 1 ) )
			{
				writeLogs() ;
			}
		} while( (uint16_t)(get_tmr10ms() - tgtime ) < 10 ) ;
  	tgtime += 10 ;
		LogTimer += 1 ;
		LogCounter += 1 ;

		if ( g_model.logSwitch )
		{
			if ( getSwitch00( g_model.logSwitch ) )
			{	// logs ON
				if ( ( LogsRunning & 1 ) == 0 )
				{	// were off
					LogsRunning = 3 ;		// On and changed
					LogTimer = 0 ;
					LogCounter = 0 ;
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
					rawStartLogging() ;
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
				uint32_t writeNow = 0 ;
				// log Data (depending on Rate)
				if ( g_model.logRate == 3 )		// 0.1 secs
				{
					if ( LogCounter >= 2 )
					{
						writeNow = 1 ;
					}
				}
				else if ( g_model.logRate == 2 )		// 0.5 secs
				{
					if ( LogCounter >= 5 )
					{
						writeNow = 1 ;
					}
				}
				else if ( g_model.logRate )		// 2.0 secs
				{
					if ( LogCounter >= 20 )
					{
						writeNow = 1 ;
					}
				}
				else
				{
					if ( LogCounter >= 10 )
					{
						writeNow = 1 ;
					}
				}
				if ( writeNow )
				{
					if ( RawLogging == 0 )
					{					
						writeLogs() ;
					}
					LogCounter = 0 ;
				}
			}
		}
	}
}


#endif	// SIMU

#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX7) || defined(PCBX9LITE)
#ifdef BLUETOOTH
void telem_byte_to_bt( uint8_t data )
{
#ifndef SIMU
        put_fifo128( &Bt_fifo, data ) ;
        CoSetFlag( Bt_flag ) ;                  // Tell the Bt task something to do
#endif
}
#endif
#endif

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

#if defined(ARUNI)
uint32_t countExtraPots(uint8_t *extraPotBits)
{
	uint32_t count = 0 ;
  uint8_t bits = 0;
	if ( g_eeGeneral.extraPotsSource[0] )
	{
		count = 1 ;
    bits = 1;
	}
	if ( g_eeGeneral.extraPotsSource[1] )
	{
		count += 1 ;
    bits |= 2;
	}
  *extraPotBits = bits;
	return count ;
}
#else
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
#endif // ARUNI
#endif

#if defined(REV9E) || defined(PCBX7) || defined(PCBX9LITE)
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
	return count ;
}
#endif

void speakModelVoice()
{
	if ( g_model.modelVoice == -1 )
	{
		putNamedVoiceQueue( g_model.modelVname, VLOC_MNAMES ) ;
	}
	else
	{
		putVoiceQueue( ( g_model.modelVoice + 260 ) | VLOC_NUMUSER  ) ;
	}
}

void prepareForShutdown()
{
	if ( LogsRunning & 1 )
	{
		closeLogs() ;
	}
  g_eeGeneral.unexpectedShutdown = 0 ;
	g_eeGeneral.SavedBatteryVoltage = g_vbat100mV ;
	STORE_MODELVARS ;			// To make sure we write model persistent timer
  STORE_GENERALVARS ;		// To make sure we write "unexpectedShutdown"
	
}

#ifdef PCBSKY
 #ifndef SMALL
  #ifndef REVX
	// AR9X
uint16_t OldRssi[2] ;
uint16_t OldRssTimer ;
  #endif
 #endif
#endif


#ifdef POWER_BUTTON
	static uint8_t powerIsOn = 1 ;
#endif

#define RSSI_POWER_OFF	1
#define RSSI_STAY_ON		0

#ifdef CHECKRSSI
#ifdef REVX
uint32_t checkRssi(uint32_t swappingModels, uint32_t rssi)
#else
uint32_t checkRssi(uint32_t swappingModels)
#endif
{
	static uint16_t timer ;
#ifdef PCBSKY
	uint32_t i ;
#endif
  if(g_eeGeneral.disableRxCheck) return RSSI_POWER_OFF ;

#ifdef REVX
	if ( ( TelemetryData[FR_RXRSI_COPY] == 0 ) && ( rssi == 0 ) )
#else
	if ( TelemetryData[FR_RXRSI_COPY] == 0 )
#endif
	{
#ifdef PCBSKY
 #ifndef SMALL
  #ifndef REVX
	// AR9X
		if ( OldRssi[0] == 0 )
  #endif
 #endif
#endif
		return RSSI_POWER_OFF ;
	}

  // first - display warning

	lcd_clear();
#if defined(PCBX12D) || defined(PCBX10)
  lcd_img( 1 + X12OFFSET, 0, HandImage,0,0, LCD_RED ) ;
#else
  lcd_img( 1, 0, HandImage,0,0 ) ;
#endif
  PUTS_ATT(36 + X12OFFSET,0*FH,XPSTR("Receiver"),DBLSIZE|CONDENSED);
  PUTS_ATT(36 + X12OFFSET,2*FH,PSTR(STR_WARNING),DBLSIZE|CONDENSED);
#ifdef PCBSKY
	PUTS_P(0 + X12OFFSET,5*FH,  XPSTR("Rx was still powered") ) ;
#else
	PUTS_P(0 + X12OFFSET,5*FH,  XPSTR("Rx still powered") ) ;
#endif
	PUTS_P(0 + X12OFFSET,7*FH,  PSTR(STR_PRESS_KEY_SKIP) ) ;
  refreshDisplay();
  clearKeyEvents();
	if ( (uint16_t)(get_tmr10ms() - timer ) > 49 )
	{
		putSystemVoice( SV_ALERT, 0 ) ;
		timer = get_tmr10ms() ;
	}
  
#ifdef PCBSKY
	i = 0 ;
#endif
  while (1)
  {
#ifdef SIMU
    if (!main_thread_running) return;
    sleep(1/*ms*/);
#endif
		check_backlight() ;

#ifdef PCBSKY
		if ( TelemetryData[FR_RXRSI_COPY]  )
		{
			i = 0 ;
		}
		if ( ++i > 300 )
		{
			return RSSI_POWER_OFF ;
		}
#else
		if ( TelemetryData[FR_RXRSI_COPY] == 0 )
		{
			return RSSI_POWER_OFF ;
		}
#endif
    if( keyDown() )
    {
			clearKeyEvents() ;
			return RSSI_POWER_OFF ;
    }
    wdt_reset();
		CoTickDelay(5) ;					// 10mS for now
#ifdef MIXER_TASK
		mainSequence( NO_MENU ) ;
#else
		getADC_osmp() ;
		perOutPhase(g_chans512, 0);
		check_frsky( 0 ) ;
#endif
//#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D)
		if ( swappingModels == 0 )
		{
			if ( ( check_soft_power() != POWER_OFF ) )		// power back on?
			{
				return RSSI_STAY_ON ;
	    }
//#endif	
#ifdef POWER_BUTTON
 #if defined(PCBXLITE)
			if ( GPIO_ReadInputDataBit(GPIOPWRSENSE, PIN_PWR_STATUS) == Bit_RESET )
 #else
  #if defined(PCBLEM1)
			if ( GPIOE->IDR & 0x0800 )
  #else
			if ( GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) == Bit_RESET )
  #endif
 #endif
			{
				if ( powerIsOn > 3 )
				{
					if ( ++powerIsOn > 9 )
					{
						return RSSI_STAY_ON ;
					}
				}
			}
			else
			{
				powerIsOn = 4 ;
			}
#endif
		}

	}
}
#endif


#ifdef MIXER_TASK
void mixer_loop(void* pdata)
{
	while ( Activated == 0 )
	{
		CoTickDelay(5) ;					// 10mS
	}
	for(;;)
	{
		runMixer() ;
		CoTickDelay(1) ;					// 2mS for now
	}
}
#endif

// This is the main task for the RTOS
void main_loop(void* pdata)
{
#if defined(PCBX12D) || defined(PCBX10)
	LastShotSwitch = 1 ;
#endif
#ifdef REVX
	uint32_t rssi ;
#endif
#ifdef PCBX9D
 #ifdef REV9E
	NumExtraPots = NUM_EXTRA_POTS - 2 ;
 #else
  #ifdef PCBX7
	NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
	#else
   #if defined(PCBX9LITE)
	NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
   #else
	NumExtraPots = NUM_EXTRA_POTS ;
   #endif
  #endif
 #endif
#else // X9D
#if defined(PCBX12D) || defined(PCBX10)
	NumExtraPots = NUM_EXTRA_POTS ;
#else // X12D
#ifdef ARUNI
	NumExtraPots = NUM_EXTRA_POTS + countExtraPots(&ExtraPotBits) ;
#else
	NumExtraPots = NUM_EXTRA_POTS + countExtraPots() ;
#endif // ARUNI
#endif // X12D
#endif // X9D

#ifdef PCB9XT
	backlight_on() ;
#endif

#ifdef PCBSKY
	if ( ( ( ResetReason & RSTC_SR_RSTTYP ) != (2 << 8) ) && !unexpectedShutdown )	// Not watchdog
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
	if ( ( ( ResetReason & RCC_CSR_WDGRSTF ) != RCC_CSR_WDGRSTF ) && !unexpectedShutdown )	// Not watchdog
#endif
#if defined(PCBLEM1)
	if ( ( ( ResetReason & RCC_CSR_IWDGRSTF ) != RCC_CSR_IWDGRSTF ) && !unexpectedShutdown )	// Not watchdog
#endif
	{
		uint8_t evt ;
		doSplash() ;

#ifndef SMALL
		if(sysFlags & sysFLAG_FORMAT_EEPROM)
		{
			sysFlags &= ~(sysFLAG_FORMAT_EEPROM) ; //clear flag
		}
#endif

		getADC_single();
		
		checkCustom() ;
//#ifndef PCBLEM1
  	checkSwitches();
//#endif
		checkAlarm();
		checkWarnings();
#ifdef PCBX9D
		check6pos() ;
#endif  	 
#ifndef PCBLEM1
		checkMultiPower() ;
#endif
		clearKeyEvents(); //make sure no keys are down before proceeding
		wdt_reset() ;
  	WatchdogTimeout = 200 ;
		VoiceCheckFlag100mS |= 6 ;// Set switch current states (global)
		processVoiceAlarms() ;
		VoiceCheckFlag100mS = 0 ;
		speakModelVoice() ;
		sortTelemText() ;

#ifndef PCBLEM1
		parseMultiData() ;
#endif
		evt = getEvent() ;
		killEvents( evt ) ;
#ifdef stm32f205
		disableRtcBattery() ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		disableRtcBattery() ;
#endif

#ifdef TOUCH
	touchPanelInit() ;
#endif


#ifdef LUA
		if ( g_model.basic_lua )
		{
			luaLoadModelScripts() ;
		}
		else
		{
			startBasic() ;
			basicLoadModelScripts() ;
		}
#endif
	}

#ifdef USE_VARS
	initVars() ;
#endif

#ifndef LUA
 #ifdef BASIC
	basicLoadModelScripts() ;
 #endif
#endif
	VoiceCheckFlag100mS |= 2 ;// Set switch current states
	processSwitches() ;	// Guarantee unused switches are cleared

#if defined(PCBX12D)
	// Turn BT on
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN ; 		// Enable portG clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	configure_pins( BT_BRTS_GPIO_PIN, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTG | PIN_LOW ) ;
	configure_pins( BT_EN_GPIO_PIN, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA | PIN_LOW ) ;
	configure_pins( BT_TX_GPIO_PIN, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTG | PIN_HIGH ) ;
#endif


// RM TX16S 0.2453

// Preload battery voltage
  int32_t ab = anaIn(12);

  ab = ( ab + ab*(g_eeGeneral.vBatCalib)/128 ) * 4191 ;
#ifdef PCBSKY
        ab /= 55296  ;
        g_vbat100mV = ab + 3 + 3 ;// Also add on 0.3V for voltage drop across input diode
#endif
#ifdef PCBX9D
 #if defined(PCBX9LITE) || defined(PCBX7ACCESS)
        ab /= 71089  ;
 #else
  #if defined(PCBXLITE)
        ab /= 64626  ;
  #else
        ab /= 57165  ;
  #endif
 #endif
        g_vbat100mV = ab ;
#endif
#ifdef PCB9XT
        ab /= 64535  ;
        g_vbat100mV = ab ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
 #if defined(PCBTX16S)
        ab /= 63802  ;
 #else
        ab /= 68631  ;
 #endif
        g_vbat100mV = ab ;
#endif

#if defined(PCBLEM1)
        ab /= 86670  ;
        g_vbat100mV = ab ;
#endif

#ifdef PCBSKY
	// Must do this to start PPM2 as well
 	perOut( g_chans512, NO_DELAY_SLOW | FADE_FIRST | FADE_LAST ) ;
	init_main_ppm( 10000, 0 ) ;		// Default for now, initial period 1.5 mS, output off
	init_ppm2( 10000, 0 ) ;
	startPulses() ;		// using the required protocol
	start_ppm_capture() ;
	checkTrainerSource() ;

#endif

#ifdef PCBLEM1
extern void initPulsesDsm( void ) ;
	initPulsesDsm()	;
	CoTickDelay(50) ;					// 100mS for now
extern void initDsmModule( void ) ;
	initDsmModule() ;
extern void startDsmPulses( void ) ;
	startDsmPulses() ;

	init_trainer_capture(0) ;

#endif

#if defined(PCBX12D) || defined(PCBX10)
#endif

#ifdef PCBX9D
// Switches PE2,7,8,9,13,14
#ifdef PCBXLITE
	configure_pins( 0x7D80, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
#else
	configure_pins( 0x6384, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
#endif

	init_no_pulses( 0 ) ;
	init_no_pulses( 1 ) ;

	init_trainer_capture(0) ;

	rtcInit() ;

	init_xjt_heartbeat() ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
	init_no_pulses( 0 ) ;
	init_no_pulses( 1 ) ;
	init_trainer_capture(0) ;
	
#ifndef PCBREV_EXPRESS
	init_xjt_heartbeat() ;
#endif
	rtcInit() ;
#endif


#ifdef PCB9XT
	init_no_pulses( 0 ) ;
	init_no_pulses( 1 ) ;

	init_trainer_capture(0) ;

	rtcInit() ;

#endif

	heartbeat_running = 1 ;

  if (!g_eeGeneral.unexpectedShutdown)
	{
    g_eeGeneral.unexpectedShutdown = 1;
    STORE_GENERALVARS ;
  }

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
 	perOut( g_chans512, NO_DELAY_SLOW | FADE_FIRST | FADE_LAST ) ;
	startPulses() ;		// using the required protocol
#endif

#if defined(PCBX9D) || defined(IMAGE_128) || defined(PCBX12D) || defined(PCBX10)
#ifndef PCBX7
#ifndef PCBX9LITE
#ifndef PCBXLITE
extern uint8_t ModelImageValid ;
	if ( !ModelImageValid )
	{
		loadModelImage() ;
	}
#endif	
#endif	
#endif	
#endif	
	Activated = 1 ;


#ifdef PCBSKY
	if ( ( ( ResetReason & RSTC_SR_RSTTYP ) != (2 << 8) ) && !unexpectedShutdown )	// Not watchdog
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
	if ( ( ( ResetReason & RCC_CSR_WDGRSTF ) != RCC_CSR_WDGRSTF ) && !unexpectedShutdown )	// Not watchdog
#endif
#if defined(PCBLEM1)
	if ( ( ( ResetReason & RCC_CSR_IWDGRSTF ) != RCC_CSR_IWDGRSTF ) && !unexpectedShutdown )	// Not watchdog
#endif
	{
		if ( g_vbat100mV > g_eeGeneral.SavedBatteryVoltage + 4 )
		{
			uint8_t result ;
  		clearKeyEvents() ;
			while(1)
			{
				lcd_clear() ;

 				PUTS_P(0 + X12OFFSET,2*FH,  XPSTR("\002Battery Charged?") ) ;
				PUTS_P(0 + X12OFFSET,3*FH,  XPSTR("\004Reset Timer?") ) ;
			  PUTS_ATT_LEFT( 5*FH,PSTR(STR_YES_NO));
			  PUTS_ATT_LEFT( 6*FH,PSTR(STR_MENU_EXIT));
			  refreshDisplay() ;

				result = keyDown() & 0x86 ;
      	if( result )
	      {
				  clearKeyEvents() ;
					if ( result & 0x82 )
					{
						g_eeGeneral.totalElapsedTime = 0 ;
#ifdef PCBSKY
 #ifndef ARUNI
						MAh_used = 0 ;
						Current_used = 0 ;
 #endif
#endif
						result = getEvent() ;
						killEvents(result) ;
					}
    	    break ;
	      }
		    wdt_reset();
				CoTickDelay(5) ;					// 10mS for now
				if ( check_power_or_usb() ) break ;		// Usb on or power off
			}
		}
	}

#ifdef PCBLEM1
	uint32_t buttonSeenOff = 0 ;
	Time.second = 10 ;
	Time.minute = 11 ;
	Time.hour = 13 ;
	Time.date = 9 ;
	Time.month = 9 ;
	Time.year = 2019 ;
	g_eeGeneral.rotaryDivisor = 1 ;
#endif

#ifdef REVX
	rssi = 0 ;
#endif


	while (1)
	{
#ifdef PCBSKY
 #ifndef SMALL
  #ifndef REVX
	// AR9X
		if ( Tenms )
		{
			if ( TelemetryData[FR_RXRSI_COPY] )
			{
				OldRssi[1] = OldRssi[0] = TelemetryData[FR_RXRSI_COPY] ;
			}
			else if ( ++OldRssTimer > 99 )
			{
				OldRssTimer = 0 ;
				OldRssi[0] = OldRssi[1] ;
				OldRssi[1] = TelemetryData[FR_RXRSI_COPY] ;
			}
		}
  #endif
 #endif
#endif
		
#if (not defined(REVA)) || defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
		
 #ifdef POWER_BUTTON
		uint8_t stopMenus = MENUS ;
 		static uint16_t tgtime = 0 ;
  #if defined(PCBXLITE) || defined(PCBX9LITE)
		if ( GPIO_ReadInputDataBit(GPIOPWRSENSE, PIN_PWR_STATUS) == Bit_RESET )
  #else // Lite
   #ifdef PCBLEM1
static uint8_t PBstate ;
		if ( GPIOE->IDR & 0x0800 )
		{
			if ( PBstate == 0 )
			{
  			tgtime = get_tmr10ms() ;
//				PBtimer = 60 ;
				PBstate = 1 ;
			}
			if ( PBstate )
			{
				if ( (uint16_t)(get_tmr10ms() - tgtime ) > 65 )
				{
					PBstate = 2 ;
				}
			}
			if ( buttonSeenOff && (PBstate == 2) )
   #else
			if ( GPIO_ReadInputDataBit(GPIOPWR, PIN_PWR_STATUS) == Bit_RESET )
   #endif // PCBLEM1
  #endif // Lite
			{
				if ( powerIsOn == 1 )
				{
					powerIsOn = 2 ;
  				tgtime = get_tmr10ms() ;
				}
				else
				{
					if ( powerIsOn == 2 )
					{
						stopMenus = NO_MENU ;
						if ( ( get_tmr10ms() & 3 ) == 0 )
						{
							lcd_clear() ;
#ifdef PROP_TEXT
							PUTS_ATT( 3*FW + X12OFFSET, 3*FH, "STOPPING", DBLSIZE ) ;
#else
							PUTS_ATT( 3*FW + X12OFFSET, 3*FH, "STOPPING", DBLSIZE ) ;
#endif
							uint8_t dtimer = get_tmr10ms() - tgtime ;
  #if defined(PCBX12D) || defined(PCBX10)
							dtimer = ( 80 - dtimer ) * 100 / 80 ;
							pushPlotType( PLOT_BLACK ) ;
  #else
							dtimer = ( 150 - dtimer ) * 100 / 150 ;
  #endif
							lcd_hbar( 13 + X12OFFSET, 49, 102, 6, dtimer ) ;
  #if defined(PCBX12D) || defined(PCBX10)
							popPlotType() ;
  #endif
							refreshDisplay() ;
						}
  #if defined(PCBX12D) || defined(PCBX10)
						if ( (uint16_t)(get_tmr10ms() - tgtime ) > 80 )
  #else
						if ( (uint16_t)(get_tmr10ms() - tgtime ) > 150 )
  #endif
						{
							powerIsOn = 3 ;
						}
					}
				}
			}
			else
			{
				powerIsOn = 1 ;
			}
			if ( powerIsOn >= 3 )
 #else // POWER_BUTTON
  #ifdef PCBSKY
   #ifdef REVX
			if ( TelemetryData[FR_RXRSI_COPY] )
			{
				rssi = 350 ;
			}
			else
			{
				if ( rssi )
				{
					rssi -= 1 ;
				}
			}
   #endif //  REVX
			static uint16_t tgtime = 0 ;
			uint32_t powerState ;
			powerState = check_soft_power() ;
			if ( ( powerState == POWER_ON ) || ( powerState == POWER_TRAINER ) )
			{
				tgtime = get_tmr10ms() ;	// Re-trigger timer
			}
			else
			{
				if ( powerState == POWER_OFF )
				{
					if ( (uint16_t)(get_tmr10ms() - tgtime ) < 40 )
					{ // Allow time for trainer power to establish if necessary
						powerState = POWER_ON ;
					}
				}
			}
			if ( powerState == POWER_OFF )
  #else // PCBSKY
			if ( ( check_soft_power() == POWER_OFF ) )		// power now off
  #endif // PCBSKY
// #endif
 #endif // POWER_BUTTON
			{
 #ifdef CHECKRSSI
   #ifdef REVX
 				if ( checkRssi(0, rssi) == RSSI_STAY_ON )
  #else
				if ( checkRssi(0) == RSSI_STAY_ON )
  #endif
				{
  #ifdef POWER_BUTTON
					powerIsOn = 3 ;
  #endif
					continue ;
				}
 #endif
			// Time to switch off
				putSystemVoice( SV_SHUTDOWN, AU_TADA ) ;
				lcd_clear() ;
				PUTS_P( 4*FW + X12OFFSET, 3*FH, PSTR(STR_SHUT_DOWN) ) ;

#if defined(PCBT16)
extern uint8_t InternalMultiStopUpdate ;
				InternalMultiStopUpdate = 1 ;
#endif

//#ifdef PCBX12D
//	PUT_HEX4( 20, FH, AudioActive ) ;
//	PUT_HEX4( 100, FH, (uint16_t)(get_tmr10ms() - tgtime ) ) ;
//#endif

 #ifdef PCBSKY
			// Stop pulses out at this point, hold pin low
				module_output_low() ;
 #endif // PCBSKY

				refreshDisplay() ;

			// Wait for OK to turn off
			// Currently wait 1 sec, needs to check eeprom finished

 #ifdef PCBSKY
  #ifndef ARUNI
				if ( ( g_eeGeneral.ar9xBoard == 0 ) && ( g_eeGeneral.extraPotsSource[0] != 2 ) && ( g_eeGeneral.extraPotsSource[1] != 2 ) )
				{
					MAh_used += Current_used/3600 ;
					if ( g_eeGeneral.mAh_used != MAh_used )
					{
						g_eeGeneral.mAh_used = MAh_used ;
					}
				}
  #endif // ARUNI
 #endif // PCBSKY
				prepareForShutdown() ;
 #ifdef stm32f205
				disableRtcBattery() ;
 #endif
	  		uint16_t tgtime = get_tmr10ms() ;
	  		uint16_t long_tgtime = tgtime ;
 #if defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
				uint32_t saved = 0 ;
 #endif
 #ifdef EETYPE_RLC
				uint32_t saved = 0 ;
 #endif

 #if defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
		  	while( (uint16_t)(get_tmr10ms() - tgtime ) < 300 ) // 3 seconds
  			{
					if ( (uint16_t)(get_tmr10ms() - tgtime ) > 270 )
 #else
  #if defined(PCBX7) || defined (PCBXLITE) || defined (PCBX9LITE)
		  	while( (uint16_t)(get_tmr10ms() - tgtime ) < 170 ) // 50 - Half second
  			{
					if ( (uint16_t)(get_tmr10ms() - tgtime ) > 160 )
  #else
		  	while( (uint16_t)(get_tmr10ms() - tgtime ) < 70 ) // 50 - Half second
  			{
					if ( (uint16_t)(get_tmr10ms() - tgtime ) > 60 )
  #endif
 #endif
					{
 #ifndef POWER_BUTTON
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
					if ( AudioActive )
					{
						if ( (uint16_t)(get_tmr10ms() - long_tgtime ) < 600 )		// 6 seconds
						{
							tgtime = get_tmr10ms() ;
						}
					}

 #if defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
					lcd_clear() ;
					PUTS_P( 4*FW + X12OFFSET, 3*FH, PSTR(STR_SHUT_DOWN) ) ;
//	PUT_HEX4( 20, FH, AudioActive ) ;
//	PUT_HEX4( 100, FH, (uint16_t)(get_tmr10ms() - tgtime ) ) ;
//extern void dumpModelAsText() ;
//						dumpModelAsText() ;
extern void eeSaveAll() ;
					if ( ! saved )
					{
						eeSaveAll() ;
						saved = 1 ;
						tgtime = get_tmr10ms() ;
	  				WatchdogTimeout = 300 ;

					}
 #endif
 #if defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
					if ( (uint16_t)(get_tmr10ms() - tgtime ) < 200 )
					{
						PUTS_NP( 5*FW + X12OFFSET, 5*FH, "EEPROM BUSY", 11 ) ;
						wdt_reset() ;
						
//	lcd_outdez( 3*FW, 7*FH, WatchdogTimeout ) ;

 #else

  #ifdef EETYPE_RLC
					if ( ! saved )
					{
						saved = 1 ;
						PUTS_NP( 5*FW, 5*FH, "EEPROM BUSY", 11 ) ;
						refreshDisplay() ;
						ee32_check_finished() ;
						PUTS_NP( 5*FW + X12OFFSET, 5*FH, "\223\223\223\223\223\223\223\223\223\223\223", 11 ) ;
 						tgtime = get_tmr10ms() ;
					}
  #else

					if ( ee32_check_finished() == 0 )
					{
						PUTS_NP( 5*FW, 5*FH, "EEPROM BUSY", 11 ) ;
						tgtime = get_tmr10ms() ;
  #endif
 #endif

 #ifdef PCB9XT
extern uint16_t General_timer ;
extern uint16_t Model_timer ;
extern uint8_t	Eeprom32_process_state ;
extern uint8_t Ee32_general_write_pending ;
extern uint8_t Ee32_model_write_pending ;
extern uint8_t Ee32_model_delete_pending ;
  
	PUT_HEX4( 0, 6*FH, GPIOC->IDR ) ;
  PUT_HEX4( 25, 6*FH, check_soft_power() ) ;

	PUTS_NUMX( 3*FW, 7*FH, General_timer ) ;
	PUTS_NUMX( 7*FW, 7*FH, Model_timer ) ;
	PUTS_NUMX( 10*FW, 7*FH, Eeprom32_process_state ) ;
	PUTS_NUMX( 13*FW, 7*FH, Ee32_general_write_pending ) ;
	PUTS_NUMX( 16*FW, 7*FH, Ee32_model_write_pending ) ;
	PUTS_NUMX( 19*FW, 7*FH, Ee32_model_delete_pending ) ;


 #endif
 #ifndef EETYPE_RLC
				}
				else
				{
//#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
#if defined(PROP_TEXT)
					PUTS_NP( 5*FW + X12OFFSET, 5*FH, "\223\223\223\223\223\223\223\223\223\223\223", 11 ) ;
#else
					PUTS_NP( 5*FW + X12OFFSET, 5*FH, "           ", 11 ) ;
#endif
				}
 #endif
 #ifdef POWER_BUTTON
				if ( check_soft_power() == POWER_X9E_STOP )	// button still pressed
				{
  				tgtime = get_tmr10ms() ;
				}
  #if defined(PCBX7) || defined (PCBXLITE) || defined (PCBX9LITE)
				CoTickDelay(1) ;	// Make sure QX7 starts playing now
  #endif

 #endif
//	PUT_HEX4( 20, 6*FH, check_soft_power() ) ;
//extern uint8_t PowerState ;
//	PUT_HEX4( 20, 7*FH, PowerState ) ;
//	PUT_HEX4( 60, 7*FH, powerIsOn ) ;
				refreshDisplay() ;
  		}

 #ifndef POWER_BUTTON
  #ifdef PCBSKY
   #ifdef REVX
			powerState = check_soft_power() ;
			if ( ( powerState == POWER_ON ) || ( powerState == POWER_TRAINER ) )
   #else
			if ( check_soft_power() == POWER_ON )
   #endif // REVX
			{
				module_output_active() ;
				startPulses() ;		// using the required protocol
				wdt_reset() ;
				break ;	// Power back on
			}
  #endif // PCBSKY
 #endif // POWER_BUTTON
 #ifdef PCBLEM1
		}
	}
	else
	{
		PBstate = 0 ;
		powerIsOn = 1 ;
		buttonSeenOff = 1 ;
	}
	if ( powerIsOn >= 3 )
	{
		lcd_clear() ;
		PUTS_P( 4*FW, 3*FH, "POWER OFF" ) ;
		refreshDisplay() ;
		break ;
	}
 #else

				lcd_clear() ;
				PUTS_NP( 6*FW, 3*FH, "POWER OFF", 9 ) ;
 #if defined(REV9E) || defined(PCBX12D) || defined(PCBX7) || defined (PCBX9LITE) || defined(PCBX10)
//#if defined(REV9E)
extern uint8_t PowerState ;
	PUT_HEX4( 20, 0, PowerState ) ;
 #endif
				refreshDisplay() ;
				
 #if defined(REV9E) || defined(PCBX12D) || defined(PCBX7) || defined (PCBX9LITE) || defined(PCBX10)
//#if defined(REV9E)
extern uint8_t PowerState ;
				while ( PowerState < 4 )
				{
					wdt_reset() ;
					lcd_clear() ;
					check_soft_power() ;
					PUTS_NP( 6*FW + X12OFFSET, 3*FH, "POWER OFF", 9 ) ;
					refreshDisplay() ;
				}
 #endif
				soft_power_off() ;		// Only turn power off if necessary
 #ifdef PCBX7
					lcdOff() ;
 #endif // PCBX7
 #ifdef PCBX9LITE
					lcdOff() ;
 #endif // PCBX9LITE
 #if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
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
  #ifndef POWER_BUTTON
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
#endif

#ifdef SERIAL_HOST
		mainSequence( NO_MENU ) ;
#else
 #ifdef POWER_BUTTON
		mainSequence( stopMenus ) ;
 #else
		mainSequence( MENUS ) ;
 #endif
#endif

#ifndef SIMU
//		CoTickDelay(2) ;					// 4mS for now
		CoTickDelay(1) ;					// 2mS for now
#endif
	}

#ifdef PCBSKY
	RSTC->RSTC_CR = 0xA5000000 | RSTC_CR_PROCRST | RSTC_CR_PERRST ;
#endif
	
#ifdef PCBLEM1
	POWER_ON_GPIO->BRR = POWER_ON_Pin ;
	for ( ; ; )
	{
	}
#endif
	
}


uint16_t getTmr2MHz()
{
#ifdef PCBSKY
	return TC1->TC_CHANNEL[0].TC_CV ;
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	return TIM7->CNT ;
#endif
}

uint32_t OneSecTimer ;
uint8_t StickScrollAllowed ;
uint8_t StickScrollTimer ;

extern int16_t AltOffset ;

static void almess( const char * s, uint8_t type )
{
	const char *h ;
#if defined(PCBX12D) || defined(PCBX10)
  PUTS_P( X12OFFSET, 4*FW, s ) ;
#else
  lcd_clear();
  PUTS_ATT_LEFT(4*FW,s);
#endif
	if ( type == ALERT_TYPE)
	{
    PUTS_P(64-6*FW + X12OFFSET,7*FH,"press any Key");
		h = PSTR(STR_ALERT) ;
//#ifdef PCBX12D
//  lcd_img( 1 + X12OFFSET, 0, HandImage,0,0, LCD_RED ) ;
//#else
//  lcd_img( 1, 0, HandImage,0,0 ) ;
//#endif

	}
	else
	{
		h = PSTR(STR_MESSAGE) ;
	}
  PUTS_ATT(64-7*FW + X12OFFSET,0*FH, h,DBLSIZE);
#if defined(PCBX12D) || defined(PCBX10)
#else
//  refreshDisplay();
#endif
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
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
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

uint32_t rssiOffsetValue( uint32_t type )
{
	uint32_t offset = 45 ;

#if defined(PCBX9D) || defined(PCB9XT)
	if ( ( ( g_model.Module[1].protocol == PROTO_DSM2) && ( g_model.Module[1].sub_protocol == DSM_9XR ) ) || ( g_model.telemetryProtocol == TELEMETRY_DSM ) )
#else
	if ( ( ( g_model.Module[1].protocol == PROTO_DSM2) && ( g_model.Module[1].sub_protocol == DSM_9XR ) ) || ( g_model.telemetryProtocol == TELEMETRY_DSM ) )
#endif // PCBX9D
	{
		if ( !g_model.dsmAasRssi )
		{
			offset = 20 ;
		}
	}
	if ( type )
	{
		offset -= 3 ;
		if ( offset == 17 )
		{
			offset = 18 ;
		}
	}				 
	return offset ;
}


static void processVoiceAlarms()
{
	uint32_t i ;
	uint32_t curent_state ;
	uint32_t flushSwitch ;
	VoiceAlarmData *pvad = &g_model.vad[0] ;
	i = 0 ;
	if ( VoiceCheckFlag100mS & 4 )
	{
		i = NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ;
	}
	flushSwitch = getSwitch00( g_model.voiceFlushSwitch ) ;
	if ( ( VoiceCheckFlag100mS & 2 ) == 0 )
	{
		if ( flushSwitch && ( LastVoiceFlushSwitch == 0 ) )
		{
			flushVoiceQueue() ;			
		}
	}
	LastVoiceFlushSwitch = flushSwitch ;
  for ( ; i < NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS + NUM_GLOBAL_VOICE_ALARMS ; i += 1 )
	{
		struct t_NvsControl *pc = &NvsControl[i] ;
		uint32_t play = 0 ;
		uint32_t functionTrue = 0 ;
		curent_state = 0 ;
		int16_t ltimer = pc->nvs_timer ;
	 	if ( i == NUM_VOICE_ALARMS )
		{
			pvad = &g_model.vadx[0] ;
		}
	 	if ( i == NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
		{
			pvad = &g_eeGeneral.gvad[0] ;
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
				case 7 :
					x = (x & y) != 0 ;
				break ;
				case 8 :
				case 10 :
				{	
  				int16_t z ;
					uint8_t update = 0 ;
					z = x - pc->nvs_last_value ;
          if (pvad->func == 10 )
					{
            if (y >= 0)
						{
              x = (z >= y) ;
              if (z < 0)
                update = true ;
            }
            else
						{
              x = (z <= y) ;
              if (z > 0)
                update = true ;
            }
					}	
          else
					{
            x = (abs(z) >= y);
          }

          if ( x || update )
					{
						pc->nvs_last_value = x ;
					}
				}
				break ;
				case 9 :
					if ( y )
					{
						x = (x % y) == 0 ;
					}
					else
					{
						x = 0 ;
					}
				break ;
			}
			functionTrue = x ;
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
				if ( pvad->swtch == MAX_SKYDRSWITCH + 1 )
				{
					if ( getFlightPhase() == 0 )
					{
						x = 0 ;
					}
				}
				else if ( getSwitch00( pvad->swtch ) == 0 )
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
				if ( pvad->swtch == MAX_SKYDRSWITCH + 1 )
				{
					curent_state = getFlightPhase() ? 1 : 0 ;
				}
				else
				{
					curent_state = getSwitch00( pvad->swtch ) ;
				}
				if ( curent_state == 0 )
				{
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

		if ( ( VoiceCheckFlag100mS & 2 ) == 0 )
		{
		 if ( pvad->rate == 3 )	// All
		 {
		 		uint32_t pos ;
				pos = 1 ;
				if ( pvad->func && ( functionTrue == 0 ) )
				{
					pos = 0 ;
				}
		 		if ( pos )
				{
					if ( pvad->swtch == MAX_SKYDRSWITCH + 1 )
					{
						pos = getFlightPhase() ;
					}
					else
					{
						pos = switchPosition( pvad->swtch ) ;
					}
					uint32_t state = pc->nvs_state ;
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
						pc->nvs_state = state ;
					}
				}
				else
				{
					pc->nvs_state = 0x40 ;
				}
		 }
		 else
		 {
			if ( play == 1 )
			{
				if ( pc->nvs_state == 0 )
				{ // just turned ON
					if ( ( pvad->rate == 0 ) || ( pvad->rate == 2 ) )
					{ // ON
						if ( pvad->delay )
						{
							pc->nvs_delay = pvad->delay + 1 ;
						}
						ltimer = 0 ;
					}
				}
				else
				{ // just turned OFF
					if ( pvad->rate == 1 )
					{
						if ( pvad->func == 8 )	// |d|>val
						{
							if ( pvad->delay )
							{
								pc->nvs_delay = pvad->delay + 1 ;
								play = 0 ;
							}
						}
					}
				}
				pc->nvs_state = 1 ;
				if ( ( pvad->rate == 1 ) )
				{
					play = 0 ;
				}
				if ( pc->nvs_delay )
				{
					if ( --pc->nvs_delay )
					{
						play = 0 ;
					}
				}
			}
			else
			{
				if ( ( pvad->func == 8 ) && ( pc->nvs_delay ) )	// |d|>val
				{
					play = 0 ;
					if ( --pc->nvs_delay == 0 )
					{
						play = 1 ;
					}
				}
				else
				{
					pc->nvs_delay = 0 ;
					if ( pc->nvs_state == 1 )
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
				}
				pc->nvs_state = 0 ;
			}
			if ( pvad->rate == 33 )
			{
				play = 0 ;
				ltimer = -1 ;
			}
		 }
		}
		else //( ( VoiceCheckFlag100mS & 2 ) != 0 )
		{
		 	uint32_t pos ;
			if ( pvad->func == 8 )	// |d|>val
			{
				pc->nvs_last_value = getValue( pvad->source - 1 ) ;
			}
			if ( pvad->rate == 3 )
			{
				if ( pvad->swtch == MAX_SKYDRSWITCH + 1 )
				{
					pos = getFlightPhase() ;
				}
				else
				{
					pos = switchPosition( pvad->swtch ) ;
				}
			}
			else
			{
				pos = play ;
			}
			pc->nvs_state = pos ;
			play = 0 ;
			if ( pvad->rate == 33 )	// ONCE
			{
	 			if ( i >= NUM_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
				{	// Global alert
					if ( VoiceCheckFlag100mS & 4 )
					{
						play = 1 ;
					}
				}
				else
				{
					play = 1 ;
				}
			}
			ltimer = -1 ;
		}

		if ( pvad->mute )
		{
			if ( pvad->source > ( CHOUT_BASE + NUM_SKYCHNOUT ) )
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
				if ( pvad->rate >= 4 )	// A time or ONCE
				{
					ltimer = 0 ;
				}
			}
			if ( ltimer == 0 )
			{
				if ( pvad->vsource == 1 )
				{
					doVoiceAlarmSource( pvad ) ;
				}
				if ( pvad->fnameType == 0 )	// None
				{
					// Nothing!
				}
				else if ( ( pvad->fnameType == 1 ) || ( pvad->fnameType == 4 ) )	// Name
				{
					char name[10] ;
					char *p ;
					p = (char *)ncpystr( (uint8_t *)name, pvad->file.name, 8 ) ;
					if ( name[0] && ( name[0] != ' ' ) )
					{
						if ( play >= 2 )
						{
							while ( *(p-1) == ' ' )
							{
								p -= 1 ;
							}
							*(p-1) += ( play - 1 ) ;
						}
						if ( pvad->fnameType == 4 )
						{
							putNamedVoiceQueue( name, VLOC_SYSTEM ) ;
						}
						else
						{
							putUserVoice( name, 0 ) ;
						}
					}
				}
				else if ( pvad->fnameType == 2 )	// Number
				{
					uint16_t value = pvad->file.vfile ;
					uint32_t t = 508 ;
//#if MULTI_GVARS
//					if ( g_model.flightModeGvars )
//					{
//						t = 510 ;
//					}
//#endif
					if ( value >= t )
					{
						value = calc_scaler( value-t, 0, 0 ) ;
					}
					else if ( value > 500 )
					{
						value = getGvar(value-501) ;
					}
					putVoiceQueue( ( value + ( play - 1 ) ) | VLOC_NUMUSER ) ;
				}
				else
				{ // Audio
					int16_t index ;
					index = pvad->file.vfile ;
					if ( index == 16 )
					{
						index = AU_HAPTIC4 ;
					}
					audio.event( index, 0, 1 ) ;
				}
				if ( pvad->vsource == 2 )
				{
					doVoiceAlarmSource( pvad ) ;
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
		pc->nvs_timer = ltimer ;
	}
	
	if ( MuteTimer )
	{
		MuteTimer -= 1 ;
	}
}

#define MUSIC_SWITCH_SAME 0
#define MUSIC_SWITCH_ON		1
#define MUSIC_SWITCH_OFF	2

extern uint16_t PlayListCount ;
extern uint16_t PlaylistIndex ;

uint32_t musicSwitch( int16_t mSwitch, uint8_t *state )
{
  if(mSwitch)
	{
		if ( mSwitch < -HSW_MAX )
		{
			mSwitch += 256 ;
		}
		if ( VoiceCheckFlag100mS & 2 )
		{
  	  *state = getSwitch00( mSwitch-(HSW_MAX) ) ;
			return MUSIC_SWITCH_SAME ;
		}
		
  	if(mSwitch>(HSW_MAX))	 // toggeled switch
		{
  	  uint8_t swPos = getSwitch00( mSwitch-(HSW_MAX) ) ;
			if ( swPos != *state )
			{
				*state = swPos ;
				if ( swPos )	// Now on
				{
					return MUSIC_SWITCH_ON ;
				}
			}
		}
		else
		{
			// normal switch
  	  uint8_t swPos = getSwitch00( mSwitch ) ;
			if ( swPos != *state )
			{
				*state = swPos ;
				if ( swPos )	// Now on
				{
					return MUSIC_SWITCH_ON ;
				}
				else
				{ // now off
					return MUSIC_SWITCH_OFF ;
				}
			}
		}
	}
	return MUSIC_SWITCH_SAME ;
}

void processMusicSwitches()
{
  int16_t mSwitch ;
	uint32_t state ;
	mSwitch = g_model.musicData.musicStartSwitch ;
	state = musicSwitch( mSwitch, &MusicSwitches.LastMusicStartSwitchState ) ;

	if ( state == MUSIC_SWITCH_ON )	// Now on
	{
		if ( ( MusicPlaying == MUSIC_PLAYING ) || ( MusicPlaying == MUSIC_PAUSED ) )
		{
			MusicPlaying = MUSIC_STOPPING ;
		}
		else if ( MusicPlaying == MUSIC_STOPPED )
		{
			MusicPlaying = MUSIC_STARTING ;
		}
	}
	else if ( state == MUSIC_SWITCH_OFF )
	{
		if ( ( MusicPlaying == MUSIC_PLAYING ) || ( MusicPlaying == MUSIC_PAUSED ) )
		{
			MusicPlaying = MUSIC_STOPPING ;
		}
	}

	mSwitch = g_model.musicData.musicPauseSwitch ;
	state = musicSwitch( mSwitch, &MusicSwitches.LastMusicPauseSwitchState ) ;
	if ( state == MUSIC_SWITCH_ON )	// Now on
	{
		if ( MusicPlaying == MUSIC_PLAYING )
		{
			MusicPlaying = MUSIC_PAUSING ;
		}
		else if ( MusicPlaying == MUSIC_PAUSED )
		{
			MusicPlaying = MUSIC_RESUMING ;
		}
		else if ( MusicPlaying == MUSIC_PAUSING )
		{
			MusicPlaying = MUSIC_PLAYING ;
		}
	}
	else if ( state == MUSIC_SWITCH_OFF )
	{
		if ( MusicPlaying == MUSIC_PAUSED )
		{
			MusicPlaying = MUSIC_RESUMING ;
		}
		else if ( MusicPlaying == MUSIC_PAUSING )
		{
			MusicPlaying = MUSIC_PLAYING ;
		}
	}
	
	mSwitch = g_model.musicData.musicPrevSwitch ;
	state = musicSwitch( mSwitch, &MusicSwitches.LastMusicPrevSwitchState ) ;
	if ( g_eeGeneral.musicType )
	{
		if ( MusicPlaying == MUSIC_PLAYING )
		{
			if ( state == MUSIC_SWITCH_ON )	// Now on
			{
				MusicPrevNext = MUSIC_NP_PREV ;
			}
		}
	}

	mSwitch = g_model.musicData.musicNextSwitch ;
	state = musicSwitch( mSwitch, &MusicSwitches.LastMusicNextSwitchState ) ;
	if ( g_eeGeneral.musicType )
	{
		if ( MusicPlaying == MUSIC_PLAYING )
		{
			if ( state == MUSIC_SWITCH_ON )	// Now on
			{
				MusicPrevNext = MUSIC_NP_NEXT ;
			}
		}
	}
}

// every 20mS
void processSwitchTimer( uint32_t i )
{
  SKYCSwData &cs = g_model.customSw[i];
//  uint8_t cstate = CS_STATE(cs.func);

//  if(cstate == CS_TIMER)
//	{
		int16_t y ;
		y = CsTimer_lastVal[i] ;
		if ( y == 0 )
		{
			int8_t z ;
			z = cs.v1 ;
			if ( z >= 0 )
			{
				z = -z-1 ;
				y = z * 50 ;
			}
			else
			{
				y = z * 5 ;
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
					y = z * 50 - 1 ;
				}
				else
				{
					y = -(z*5)-1 ;
				}
			}
		}
		else  // if ( CsTimer_lastVal[i] > 0 )
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
						y = z * 50 ;					
					}
					else
					{
						y = z * 5 ;
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
		CsTimer_lastVal[i] = y ;
//	}
}

//// Every 20mS
//static void processLatchFflop()
//{
//	uint32_t i ;
//	for ( i = 0 ; i < NUM_SKYCSW ; i += 1 )
//	{
// 	  SKYCSwData &cs = g_model.customSw[i];
//		if ( cs.func == CS_FLIP )
//		{
//		  if (getSwitch00( cs.v1) )
//			{
//				if ( ( Last_switch[i] & 2 ) == 0 )
//				{
//					 Clock it!
//			    if (getSwitch00( cs.v2) )
//					{
//						Last_switch[i] = 3 ;
//					}
//					else
//					{
//						Last_switch[i] = 2 ;
//					}
//				}
//			}
//			else
//			{
//				Last_switch[i] &= ~2 ;
//			}
//		}
//	}
//}

// Every 20mS
void processSwitches()
{
	uint32_t cs_index ;
	for ( cs_index = 0 ; cs_index < NUM_SKYCSW ; cs_index += 1 )
	{
  	SKYCSwData &cs = g_model.customSw[cs_index] ;
  	uint8_t ret_value = false ;

  	if( cs.func )
		{
  		int8_t a = cs.v1 ;
  		int8_t b = cs.v2 ;
  		int16_t x = 0 ;
  		int16_t y = 0 ;
  		uint8_t s = CS_STATE( cs.func ) ;

  		if ( (s == CS_VOFS) || (s == CS_2VAL) )
  		{
  		  x = getValue(cs.v1u-1);
    		if ( ( ( cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || (cs.v1u >= EXTRA_POTS_START + 8) )
				{
  		    y = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, cs.v2 ) ;
				}
  		  else
				{
  		  	y = calc100toRESX(cs.v2);
				}
  		}
  		else if(s == CS_VCOMP)
  		{
 		    x = getValue(cs.v1u-1);
 		    y = getValue(cs.v2u-1);
  		}

  		switch (cs.func)
			{
	  		case (CS_VPOS):
  		    ret_value = (x>y);
  	    break;
  			case (CS_VNEG):
  		    ret_value = (x<y) ;
  	    break;
  			case (CS_APOS):
	  	    ret_value = (abs(x)>y) ;
  		  break;
	  		case (CS_ANEG):
  		    ret_value = (abs(x)<y) ;
  	    break;
				case CS_VEQUAL :
  		    ret_value = (x == y) ;
  	    break;
				case CS_EXEQUAL:
					if ( isAgvar( cs.v1 ) )
					{
						x *= 10 ;
						y *= 10 ;
					}
  		  	ret_value = abs(x-y) < 32 ;
  			break;
	
				case CS_VXEQUAL:
					if ( isAgvar( cs.v1 ) || isAgvar( cs.v2 ) )
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
  			  bool res1 = getSwitch(a,0,0) ;
  			  bool res2 = getSwitch(b,0,0) ;
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
	  		case (CS_NTIME):
					processSwitchTimer( cs_index ) ;
					ret_value = CsTimer_lastVal[cs_index] >= 0 ;
  			break ;
				case (CS_TIME):
				{	
					processSwitchTimer( cs_index ) ;
  			  ret_value = CsTimer_lastVal[cs_index] >= 0 ;
					int8_t x = getAndSwitch( cs ) ;
					if ( x )
					{
					  if (getSwitch00( x ) )
						{
							if ( ( Last_switch[cs_index] & 2 ) == 0 )
							{ // Triggering
								ret_value = 1 ;
							}	
						}
					}
				}
  			break ;
  			case (CS_MONO):
  			case (CS_RMONO):
				{
					if ( VoiceCheckFlag100mS & 2 )
					{
						// Resetting, retrigger any monostables
						Last_switch[cs_index] &= ~2 ;
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
						if ( ( Last_switch[cs_index] & 2 ) == 0 )
						{
							// Trigger monostable
							uint32_t trigger = 1 ;
							if ( ( cs.func == CS_RMONO ) )
							{
								if ( ! andSwOn )
								{
									trigger = 0 ;
								}
							}
							if ( trigger )
							{
								Last_switch[cs_index] = 3 ;
								int16_t x ;
								x = cs.v2 * 5 ;
								if ( x < 0 )
								{
									x = -x ;
								}
								else
								{
									x += 5 ;
									x *= 10 ;
								}
								CsTimer_lastVal[cs_index] = x ;							
							}
						}
					}
					else
					{
						Last_switch[cs_index] &= ~2 ;
					}
					int16_t y ;
					y = CsTimer_lastVal[cs_index] ;
					if ( Now_switch[cs_index] < 2 )	// not delayed
					{
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
								Last_switch[cs_index] &= ~1 ;
							}
							CsTimer_lastVal[cs_index] = y ;
						}
					}
 			  	ret_value = CsTimer_lastVal[cs_index] > 0 ;
				}
  			break ;
  
				case (CS_LATCH) :
		  		if (getSwitch00( cs.v1) )
					{
						Last_switch[cs_index] = 1 ;
					}
					else
					{
					  if (getSwitch00( cs.v2) )
						{
							Last_switch[cs_index] = 0 ;
						}
					}
  			  ret_value = Last_switch[cs_index] & 1 ;
  			break ;
  			
				case (CS_FLIP) :
		  		if (getSwitch00( cs.v1) )
					{
						if ( ( Last_switch[cs_index] & 2 ) == 0 )
						{
							// Clock it!
					    if (getSwitch00( cs.v2) )
							{
								Last_switch[cs_index] = 3 ;
							}
							else
							{
								Last_switch[cs_index] = 2 ;
							}
						}
					}
					else
					{
						Last_switch[cs_index] &= ~2 ;
					}
  			  ret_value = Last_switch[cs_index] & 1 ;
  			break ;
  			case (CS_BIT_AND) :
				{	
  			  x = getValue(cs.v1u-1);
					y = cs.v2u ;
					y |= cs.bitAndV3 << 8 ;
  			  ret_value = ( x & y ) != 0 ;
				}
  			break ;
				case CS_RANGE :
				{
					int16_t z ;
    			if ( ( ( cs.v1u > CHOUT_BASE+NUM_SKYCHNOUT) && ( cs.v1u < EXTRA_POTS_START ) ) || (cs.v1u >= EXTRA_POTS_START + 8) )
					{
  		  	  z = convertTelemConstant( cs.v1u-CHOUT_BASE-NUM_SKYCHNOUT-1, (int8_t)cs.bitAndV3 ) ;
					}
  		  	else
					{
  		  		z = calc100toRESX((int8_t)cs.bitAndV3) ;
					}
  		    ret_value = (x >= y) && (x <= z) ;
				}			 
  			break ;
				case CS_DELTAGE :
				case CS_MOD_D_GE :
				{
					uint32_t update = 0 ;
//          if (LS_LAST_VALUE(mixerCurrentFlightMode, idx) == CS_LAST_VALUE_INIT) {
//            LS_LAST_VALUE(mixerCurrentFlightMode, idx) = x;
//          }
//          int16_t diff = x - LS_LAST_VALUE(mixerCurrentFlightMode, idx);
          int16_t diff = x - CsTimer_lastVal[cs_index] ;
          if (cs.func == CS_DELTAGE )
					{
            if (y >= 0)
						{
              ret_value = (diff >= y);
              if (diff < 0)
                update = true;
            }
            else
						{
              ret_value = (diff <= y);
              if (diff > 0)
                update = true;
            }
          }
          else
					{
            ret_value = (abs(diff) >= y);
          }
          if (ret_value || update)
					{
//            LS_LAST_VALUE(mixerCurrentFlightMode, idx) = x ;
            CsTimer_lastVal[cs_index] = x ;
          }
				}			 
  			break ;
  			default:
  		    ret_value = false;
 		    break;
  		}

			
			
			int8_t z = getAndSwitch( cs ) ;
			if ( z )
			{
				switch ( cs.exfunc )
				{
					case 0 :
  		    	ret_value &= getSwitch( z, 0, 0 ) ;
					break ;
					case 1 :
  		    	ret_value |= getSwitch( z, 0, 0 ) ;
					break ;
					case 2 :
  		    	ret_value ^= getSwitch( z, 0, 0 ) ;
					break ;
				}
			}
			
//			if ( ret_value )
//			{
//				int8_t x = getAndSwitch( cs ) ;
//				if ( x )
//				{
//  		    ret_value = getSwitch( x, 0, 0 ) ;
//				}
//			}
			
			if ( ( cs.func < CS_LATCH ) || ( cs.func > CS_RMONO ) )
			{
				Last_switch[cs_index] = ret_value ;
			}
			if ( Now_switch[cs_index] == 0 )	// was off
			{
				if ( ret_value )
				{
					if ( g_model.switchDelay[cs_index] )
					{
						ret_value = g_model.switchDelay[cs_index] * 10 ;
					}
				}
			}
			else
			{
				if ( Now_switch[cs_index] > 1 )	// delayed
				{
					if ( ret_value )
					{
						uint8_t temp = Now_switch[cs_index] - 2 ;
						if ( temp )
						{
							ret_value = temp ;
						}
					}
				}
			}
			Now_switch[cs_index] = ret_value ;
		}
		else // no function
		{
			if ( VoiceCheckFlag100mS & 2 )
			{
				Now_switch[cs_index] = 0 ;
			}
		}
	}
}

#ifdef ARUNI
inline bool qSixPosCalibrating()
{ // six position switch is being calibrated
  return ((SixPosCaptured & 0x80) == 0);
}
#endif

uint32_t MixerRate ;
uint32_t MixerCount ;
uint32_t MixerXRate ;

uint8_t AlarmTimers[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS] ;

#ifdef PCB9XT
uint8_t SpiEncoderValid ;
uint8_t EncoderI2cData[2] ;
#endif
//uint16_t I2CencCounter ;

#if defined(PCBX10)// && defined(PCBREV_EXPRESS)
uint8_t LcdResetSwitch ;
#endif

#if defined(PCBX10)// && defined(PCBREV_EXPRESS)
extern "C" void SDRAM_Init() ;
#endif

#ifdef TOUCH
uint16_t TouchDebounceCount = 0 ;
uint16_t TouchRepeat = 0 ;
#endif

void mainSequence( uint32_t no_menu )
{
	CalcScaleNest = 0 ;

//#if defined(PCBX10)// && defined(PCBREV_EXPRESS)

//	if ( getSwitch00( CSW_INDEX + NUM_SKYCSW ) )
//	{
//		if ( LcdResetSwitch == 0 )
//		{
//			LcdResetSwitch = 1 ;
//			SDRAM_Init() ;
//			lcdColorsInit() ;
//			lcdInit() ;
//			AlertMessage = "LCD Reset" ;
//		}
//	}
//	else
//	{
//		LcdResetSwitch = 0 ;
//	}
//#endif

#ifdef PCBSKY
	static uint32_t coProTimer = 0 ;
#endif
#ifdef PCBLEM1
	static uint32_t RtcTimer = 0 ;
#endif
#ifdef PCB9XT
	static uint32_t EncoderTimer = 0 ;
#endif
  uint16_t t0 = getTmr2MHz();
	MainStart = t0 ;
#ifdef NO_VOICE_SWITCHES
	CPU_UINT numSafety = NUM_SKYCHNOUT + EXTRA_SKYCHANNELS ; 
#else
	CPU_UINT numSafety = NUM_SKYCHNOUT - g_model.numVoice ;
#endif

	perMain( no_menu ) ;		// Allow menu processing
#ifdef WHERE_TRACK
	notePosition('T'+heartbeat) ;
#endif
	if(heartbeat == 0x3)
	{
    wdt_reset();
    heartbeat = 0;
#if defined(PCBX12D) || defined(PCBX10)
		HbeatCounter += 1 ;
#endif
  }
//#ifdef PCBX9D
// #ifdef LATENCY
//	configure_pins( 0x6000, PIN_OUTPUT | PIN_PORTA | PIN_PUSHPULL | PIN_OS25 | PIN_NO_PULLUP ) ;
// #endif
//#endif

#if defined(PCBX12D) || defined(PCBX10)
	checkTrainerSource() ;
#endif

#if defined(PCBLEM1)
	checkTrainerSource() ;
#endif

#ifdef PCBX9D
	checkTrainerSource() ;
	if ( ( g_model.com2Function == COM2_FUNC_SBUSTRAIN ) || ( g_model.com2Function == COM2_FUNC_SBUS57600 ) || ( CurrentTrainerSource == TRAINER_SBUS ) )
#endif
#ifdef PCBSKY
	if ( ( g_model.com2Function == COM2_FUNC_SBUSTRAIN ) || ( g_model.com2Function == COM2_FUNC_SBUS57600 ) )
#endif
	{
		TrainerProfile *tProf = &g_eeGeneral.trainerProfile[g_model.trainerProfile] ;
		if ( tProf->channel[0].source != TRAINER_JACK )
		{
			processSbusInput() ;
		}
	}

extern uint8_t TrainerMode ;
	if ( TrainerMode == 1 )
	{
		TrainerProfile *tProf = &g_eeGeneral.trainerProfile[g_model.trainerProfile] ;
		if ( tProf->channel[0].source == TRAINER_J_SBUS )
		{
			processSbusInput() ;
		}
	}

	if ( Tenms )
	{
		
		Tenms = 0 ;
#if defined(PCBSKY) || defined(PCB9XT)
		ee32_process() ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		ee32_process() ;
#endif
#ifdef PCBSKY
 #ifndef ARUNI
		if ( ( g_eeGeneral.ar9xBoard == 0 ) && ( g_eeGeneral.extraPotsSource[0] != 2 ) && ( g_eeGeneral.extraPotsSource[1] != 2 ) )
		{
			Current_accumulator += Current_current ;
		}
 #endif
#endif
#ifdef PCBX9D
		eePoll() ;
#endif
#ifdef PCB9XT
		m64_10mS() ;
#ifdef SLAVE_RESET
		if ( getSwitch00( 32 ) ) // LN
		{
			if ( SlaveResetSwitch == 0 )
			{
extern void resetM64() ;
				resetM64() ;
extern uint8_t M64ResetCount ;
				M64ResetCount += 1 ;
			}
			SlaveResetSwitch = 1 ;
		}
		else
		{
			SlaveResetSwitch = 0 ;
		}
		
		g_model.customSw[21].func = CS_AND ;
		if ( getSwitch00( 31 ) ) // LM
		{
			if ( SlavePanicSwitch == 0 )
			{
				panicDebugMenu() ;
				SlavePanicSwitch = 1 ;
			}
		}
		else
		{
			SlavePanicSwitch = 0 ;
		}
#endif // SLAVE_RESET
#endif // PCB9XT
#if defined(PCB9XT) || defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
		handleUsbConnection() ;
#endif

#ifdef PCBSKY
#ifdef USB_JOYSTICK
		handleUsbConnection() ;
#endif
#endif


		if ( ++OneSecTimer >= 100 )
		{
			OneSecTimer -= 100 ;
#ifdef PCBSKY
 #ifndef ARUNI
			if ( ( g_eeGeneral.ar9xBoard == 0 ) && ( g_eeGeneral.extraPotsSource[0] != 2 ) && ( g_eeGeneral.extraPotsSource[1] != 2 ) )
			{
				Current_used += Current_accumulator / 100 ;			// milliAmpSeconds (but scaled)
				Current_accumulator = 0 ;
			}
 #endif
#endif

			if ( StickScrollTimer )
			{
				StickScrollTimer -= 1 ;				
			}
#ifdef REV9E
void updateTopLCD( uint32_t time, uint32_t batteryState ) ;
void setTopRssi( uint32_t rssi ) ;
void setTopVoltage( uint32_t volts ) ;
void setTopOpTime( uint32_t hours, uint32_t mins, uint32_t secs ) ;

static uint8_t RssiTestCount ;
			setTopRssi( TelemetryData[FR_RXRSI_COPY] ) ;
			setTopVoltage( g_vbat100mV ) ;
			setTopOpTime( Time.hour, Time.minute, Time.second ) ;
			if ( ++RssiTestCount > 149 )
			{
				RssiTestCount = 0 ;
			}
			uint32_t bstate = g_vbat100mV ;
			if ( bstate < 92 )
			{
				bstate = 0 ;
			}
			else
			{
				bstate -= 92 ;
				bstate /= 2 ;
			}
			updateTopLCD( s_timer[0].s_timerVal, bstate ) ;
#endif
			uint32_t x ;
static uint32_t saveIdleCount ;
//			x = ( IdleCount - saveIdleCount ) * 10000 ;
			x = ( IdleCount - saveIdleCount ) ;
//			x /= 2000000 ;
			x /= 200 ;
			saveIdleCount = IdleCount ;
			if ( x > 9999 )
			{
				x = 9999 ;
			}
			IdlePercent = x ;

extern uint32_t TotalExecTime ;
			
			BasicExecTime = TotalExecTime / 2000 ;
			TotalExecTime = 0 ;

		}
#ifndef SIMU
		sdPoll10mS() ;
#endif

#ifdef TOUCH
		if ( TouchEventOccured )
		{
			uint16_t count ;
			TouchEventOccured = 0 ;
			
			if(g_eeGeneral.lightAutoOff*500>g_LightOffCounter) g_LightOffCounter = g_eeGeneral.lightAutoOff*500 ;

			if ( (count = touchReadPoints()) )
			{
//				if ( TouchControl.count == 0 )
				{
					TouchControl.x = TouchData.points[0].x ;
					TouchControl.y = TouchData.points[0].y ;

					if ( TouchDebounceCount )
					{
						TouchDebounceCount = 0 ;
					}
					if ( TouchControl.event != TEVT_DOWN )
					{
						TouchRepeat = 0 ;
						TouchControl.repeat = 0 ;
						TouchControl.event = TEVT_DOWN ;
						TouchControl.startx = TouchControl.x ;
						TouchControl.starty = TouchControl.y ;
					}
					else
					{
						TouchControl.deltax = (int16_t)TouchControl.x - (int16_t)TouchControl.startx ;
						TouchControl.deltay = (int16_t)TouchControl.y - (int16_t)TouchControl.starty ;
					}
					if ( ++TouchRepeat > 35 )
					{
						if ( TouchRepeat > 42 )
						{
							TouchRepeat = 36 ;
							TouchControl.repeat = 1 ;
						}
					}
				}
//				else
//				{
//					if ( ++TouchRepeat > 24 )
//					{
//						if ( TouchRepeat > 29 )
//						{
//							TouchRepeat = 25 ;
//							TouchControl.repeat = 1 ;
//						}
//					}
//					// or sliding !!
//				}
				TouchControl.count = count ;
				TouchUpdated = 1 ;
			}
			else
			{
				if ( TouchControl.count )
				{
//					TouchControl.event = TEVT_UP ;
					TouchDebounceCount = 5 ;
					TouchUpdated = 2 ;
					TouchControl.count = 0 ;
					TouchControl.repeat = 0 ;
				}
			}
			TouchBacklight = 1 ;

		}
		else
		{
			if ( TouchDebounceCount )
			{
				if ( --TouchDebounceCount == 0 )
				{
					TouchControl.event = TEVT_UP ;
					TouchUpdated = 1 ;
				}
			}
		}

#endif


 #ifdef PCBSKY
		if ( ++coProTimer > 9 )
		{
			coProTimer -= 10 ;
			
#ifndef REVX
	 		if ( g_eeGeneral.ar9xBoard )
			{
				// Read external RTC here
#ifndef SMALL
extern void readExtRtc() ;
				readExtRtc() ;
#endif
			}	
			else
#endif
			{
#ifndef ARUNI
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
#endif  // nREVX
#endif  // nARUNI
			}
		}


#ifndef SMALL
#ifndef REVX
	 	if ( g_eeGeneral.ar9xBoard )
		{
extern void pollForRtcComplete() ;
			pollForRtcComplete() ;
		}
#endif
#endif

#endif

#ifdef PCBLEM1
		if ( g_eeGeneral.externalRtcType == 1 )
		{
			if ( ++RtcTimer > 9 )
			{
				RtcTimer -= 10 ;
				// Read external RTC here
				readExtRtc() ;
			}	
			pollForRtcComplete() ;
		}
#endif


#ifdef PCB9XT
		static uint8_t lastPosition = 0 ;
		if ( g_eeGeneral.enableI2C == 1 )
		{
			if ( ++EncoderTimer > 4 )
			{
				EncoderTimer = 0 ;
				hwreadI2cEncoder( EncoderI2cData ) ;
			}
extern uint32_t i2c2_result() ;

			if ( EncoderTimer == 1 )
			{
				if ( i2c2_result() == 1 )
				{
//					I2CencCounter += 1 ;
					if ( lastPosition != EncoderI2cData[0] )
					{
						int8_t diff = EncoderI2cData[0] - lastPosition ;
						if ( diff < 9 && diff > -9 )
						{
							Rotary_count += diff ;
						}
						lastPosition = EncoderI2cData[0] ;
					}
				
				}
			}
		}
//extern uint8_t SpiEncoderValid ;
		if ( SpiEncoderValid )
		{
			if ( lastPosition != EncoderI2cData[0] )
			{
				int8_t diff = EncoderI2cData[0] - lastPosition ;
				if ( diff < 9 && diff > -9 )
				{
					Rotary_count += diff ;
				}
				lastPosition = EncoderI2cData[0] ;
			}
		}

#endif

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
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
				altitude = TelemetryData[FR_ALT_BARO] + AltOffset ;
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
    }


    // this var prevents and alarm sounding if an earlier alarm is already sounding
    // firing two alarms at once is pointless and sounds rubbish!
    // this also means channel A alarms always over ride same level alarms on channel B
    // up to debate if this is correct!
    //				bool AlarmRaisedAlready = false;

    if (frskyStreaming)
		{
//      enum AlarmLevel level[4] ;
      // RED ALERTS
//      if( (level[0]=FRSKY_alarmRaised(0,0)) == alarm_red) FRSKY_alarmPlay(0,0);
//      else if( (level[1]=FRSKY_alarmRaised(0,1)) == alarm_red) FRSKY_alarmPlay(0,1);
//      else	if( (level[2]=FRSKY_alarmRaised(1,0)) == alarm_red) FRSKY_alarmPlay(1,0);
//      else if( (level[3]=FRSKY_alarmRaised(1,1)) == alarm_red) FRSKY_alarmPlay(1,1);
//      // ORANGE ALERTS
//      else	if( level[0] == alarm_orange) FRSKY_alarmPlay(0,0);
//      else if( level[1] == alarm_orange) FRSKY_alarmPlay(0,1);
//      else	if( level[2] == alarm_orange) FRSKY_alarmPlay(1,0);
//      else if( level[3] == alarm_orange) FRSKY_alarmPlay(1,1);
//      // YELLOW ALERTS
//      else	if( level[0] == alarm_yellow) FRSKY_alarmPlay(0,0);
//      else if( level[1] == alarm_yellow) FRSKY_alarmPlay(0,1);
//      else	if( level[2] == alarm_yellow) FRSKY_alarmPlay(1,0);
//      else if( level[3] == alarm_yellow) FRSKY_alarmPlay(1,1);
						
			// Check for current alarm
			if ( g_model.currentSource )
			{
				if ( g_model.frskyAlarms.alarmData[0].frskyAlarmLimit )
				{
					if ( ( TelemetryData[FR_AMP_MAH] >> 6 ) >= g_model.frskyAlarms.alarmData[0].frskyAlarmLimit )
					{
						putSystemVoice( SV_CAP_WARN, V_CAPACITY ) ;
					}
					uint32_t value ;
					value = g_model.frskyAlarms.alarmData[0].frskyAlarmLimit ;
					value <<= 6 ;
					value = 100 - ( TelemetryData[FR_AMP_MAH] * 100 / value ) ;
					TelemetryData[FR_FUEL] = value ;
				}
			}
    }
		
		// Now for the Safety/alarm switch alarms
		{
			uint32_t i ;
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
  }
	
	
	// New switch voices
	// New entries, Switch, (on/off/both), voice file index

	if ( CheckFlag20mS )
	{
		CheckFlag20mS = 0 ;
//		processLatchFflop() ;
		processSwitches() ;
	}

//	if ( CheckFlag50mS )
//	{
//		CheckFlag50mS = 0 ;
//		// Process all switches for delay etc.
////		processTimer() ;
//	}

	if ( VoiceCheckFlag100mS )	// Every 100mS
  {
		uint32_t i ;
		static uint32_t timer ;
		static uint32_t delayTimer = 0 ;

		if ( VoiceCheckFlag100mS & 1 )
		{
    	for ( i = 0 ; i < HUBDATALENGTH ; i += 1 )
			{
				if (TelemetryDataValid[i] )
				{
					TelemetryDataValid[i] -= 1 ;
				}
			}
			
			TrimInUse[0] <<= 1 ;
			TrimInUse[1] <<= 1 ;
			TrimInUse[2] <<= 1 ;
			TrimInUse[3] <<= 1 ;
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

			uint32_t redAlert = 0 ;
			static uint8_t redCounter ;
			static uint8_t orangeCounter ;
			uint8_t rssiValue = TelemetryData[FR_RXRSI_COPY] ;

			if ( frskyStreaming )
			{
				int8_t offset ;
				if ( g_model.enRssiRed == 0 )
				{
					offset = rssiOffsetValue( 1 ) ;

					if ( rssiValue && rssiValue < g_model.rssiRed + offset )
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
					offset = rssiOffsetValue( 0 ) ;
					if ( rssiValue && rssiValue < g_model.rssiOrange + offset )
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
			for ( i = 0 ; i < NUM_SCALERS ; i += 1 )
			{
				if ( g_model.eScalers[i].dest )
				{
					calc_scaler( i, 0, 0 ) ;					
				}
			}
		}
		 
		for ( i = numSafety ; i < NUM_SKYCHNOUT+NUM_VOICE ; i += 1 )
		{
			uint32_t curent_state ;
			uint32_t mode ;
			uint32_t value ;
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
					value = getGvar(value-248) ; //Gvars 3-7
				}
			}
			
			if ( sd->opt.vs.vswtch )		// Configured
			{
				curent_state = getSwitch00( sd->opt.vs.vswtch ) ;
				if ( ( VoiceCheckFlag100mS & 2 ) == 0 )
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
			// Check CVLT and CTOT every 100 mS
    	if (frskyUsrStreaming)
		  {
//				uint16_t total_volts = 0 ;
				uint16_t total_1volts = 0 ;
				uint16_t total_2volts = 0 ;
				CPU_UINT audio_sounded = 0 ;
				uint16_t low_cell = 440 ;		// 4.4V

				uint32_t firstDeviceLimit = 6 ;

				if ( FrskyBattCells[0] > 6 )
				{
				 	firstDeviceLimit = FrskyBattCells[0] ;
				}

				for (uint32_t k=0 ; k<12 ; k++)
				{
					uint32_t index = k < 6 ? TEL_ITEM_CELL1 : TEL_ITEM_CELL7 - 6 ;
					index += k ;
					if ( telemItemValid( index ) )
					{
						if ( k < firstDeviceLimit )
						{
							total_1volts += TelemetryData[FR_CELL1+k] ;
						}
						else
						{
							total_2volts += TelemetryData[FR_CELL1+k] ;
						}
						if ( TelemetryData[FR_CELL1+k] < low_cell )
						{
							low_cell = TelemetryData[FR_CELL1+k] ;
						}
						if ( AlarmCheckFlag > 1 )
						{
							if ( audio_sounded == 0 )
							{
		  	  		  if ( TelemetryData[FR_CELL1+k] < g_model.frSkyVoltThreshold * 2 )
								{
		  	  		    audioDefevent(AU_WARNING3);
									audio_sounded = 1 ;
								}
				  		}
						}
	  			}
					// Now we have total volts available
					if ( total_1volts + total_2volts )
					{
						TelemetryData[FR_CELLS_TOT] = (total_1volts + total_2volts + 4 ) / 10 ; 	// Some rounding up
						TelemetryDataValid[FR_CELLS_TOT] = 40 + g_model.telemetryTimeout ;
						
						if ( total_1volts )
						{
							TelemetryData[FR_CELLS_TOTAL1] = (total_1volts + 4)  / 10 ;
							TelemetryDataValid[FR_CELLS_TOTAL1] = 40 + g_model.telemetryTimeout ;
					  }
						if ( total_2volts )
						{
							TelemetryData[FR_CELLS_TOTAL2] = (total_2volts + 4)  / 10 ;
							TelemetryDataValid[FR_CELLS_TOTAL2] = 40 + g_model.telemetryTimeout ;
						}
						if ( low_cell < 440 )
						{
							TelemetryData[FR_CELL_MIN] = low_cell ;
						}
					}
				}
			}
		}

		// Now test for the voice alarms
		processVoiceAlarms() ;
		processMusicSwitches() ;

		VoiceCheckFlag100mS = 0 ;

		// Vario
		{

			static uint8_t varioRepeatRate = 0 ;
			static uint8_t sounded = 0 ;

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
						sounded = 0 ;
					}
					int16_t vspd ;
					if ( g_model.varioData.varioSource == 1 )
					{
						vspd = TelemetryData[FR_VSPD] ;

						if ( g_model.varioData.param > 1 )
						{
							vspd /= g_model.varioData.param ;
						}
					}
					else if ( g_model.varioData.varioSource == 2 )
					{
						vspd = TelemetryData[FR_A2_COPY] - 128 ;
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
						if ( vspd < 0 )
						{
							vspd = -vspd ;
							if (!g_model.varioData.sinkTones )
							{
								if ( vspd > 25 )		// OpenXsensor
								{
									if ( sounded != 2 )
									{
										sounded = 2 ;
										varioRepeatRate = 0 ;
  	  	     		    audio.event( AU_VARIO_DOWN, vspd/25 ) ;
									}
								}
							}
						}
						else
						{
							if ( vspd > 25 )			// OpenXsensor
							{
								if ( sounded != 1 )
								{
									sounded = 1 ;
									varioRepeatRate = 0 ;
		  	        	audio.event( AU_VARIO_UP, vspd/25 ) ;
								}
							}
						}
						if ( vspd < 75 )
						{
							new_rate = 8 ;
						}
						else if ( vspd < 100 )
						{
							new_rate = 7 ;
						}
						else if ( vspd < 125 )
						{
							new_rate = 6 ;
						}
						else if ( vspd < 150 )
						{
							new_rate = 5 ;
						}
						else if ( vspd < 175 )
						{
							new_rate = 4 ;
						}
						else if ( vspd < 200 )
						{
							new_rate = 3 ;
						}
						else
						{
							new_rate = 2 ;
						}
					}
					else
					{
						if (g_model.varioData.sinkTones )
						{
							if ( sounded == 0 )
							{
								new_rate = 20 ;
								sounded = 3 ;
								varioRepeatRate = 0 ;
  	    	  		audio.event( AU_VARIO_UP, 0 ) ;
							}
						}
					}
					if ( varioRepeatRate == 0 )
					{
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
#endif
	return 0 ;
}

void check_backlight()
{
	int8_t sw = g_model.mlightSw ;
	if ( !sw )
	{
		sw = g_eeGeneral.lightSw ;
	}
  if(getSwitch00(sw) || g_LightOffCounter)
	{
		BACKLIGHT_ON ;
	}
  else
	{
    BACKLIGHT_OFF ;
	}
}

uint16_t stickMoveValue()
{
    uint16_t sum = 0 ;
		uint32_t i ;
    for( i=0; i<4; i++)
		{
      sum += anaIn(i) ;
		}
	return sum ;
}

static uint32_t hasStickMoved( uint16_t value )
{
  if(abs(( (int16_t)value-(int16_t)stickMoveValue()))>160)
		return 1 ;
	return 0 ;
}

void doSplash()
{
	uint32_t anaCount = 0 ;
	uint32_t j ;
	if( !g_eeGeneral.disableSplashScreen )
  {
   	check_backlight() ;
    lcd_clear();
		refreshDisplay();
    lcdSetRefVolt(g_eeGeneral.contrast);
  	clearKeyEvents();

#if defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
// #ifndef MIXER_TASK
		for ( uint32_t i = 0 ; i < 25 ; i += 1 )
		{
			getADC_osmp();
		}
// #endif
#else
		for ( uint32_t i = 0 ; i < 10 ; i += 1 )
		{
			getADC_osmp();
		}
#endif
// #ifndef MIXER_TASK
		getADC_osmp();
// #endif
  	uint16_t inacSum = stickMoveValue() ;

  	uint16_t tgtime = get_tmr10ms() + SPLASH_TIMEOUT;  
  	uint16_t scrtime = get_tmr10ms() ;

		j = 62 ;
  	while(tgtime > get_tmr10ms())
  	{
			if ( scrtime < get_tmr10ms() )
			{
				scrtime += 4 ;
				uint32_t p ;
				coord_t x ;
				coord_t y ;
				uint32_t z ;
				lcd_clear();
#if defined(PCBX12D) || defined(PCBX10)
  	 		lcd_img(0 + X12OFFSET, 32, &splashdata[4],0,0, 0x03E0 );
				if(!g_eeGeneral.hideNameOnSplash)
					PUTS_ATT_N_COLOUR( 0*FW + X12OFFSET, 11*FH, g_eeGeneral.ownerName ,sizeof(g_eeGeneral.ownerName), 0, 0x03E0 ) ;
#else
  	 		lcd_img(0 + X12OFFSET, 0, &splashdata[4],0,0 );
				if(!g_eeGeneral.hideNameOnSplash)
					PUTS_ATT_N( 0*FW + X12OFFSET, 7*FH, g_eeGeneral.ownerName ,sizeof(g_eeGeneral.ownerName),0);
#endif					 

				if ( j )
				{
#if defined(PCBX12D) || defined(PCBX10)
					plotType = PLOT_BLACK ;
					p = 32 ;
#else
					plotType = PLOT_WHITE ;
					p = 0 ;
#endif					 
					x = 126 ;
					z = 64 ;
					for ( y = 0 ; y < j ; y += 2 )
					{
						lcd_vline( y + X12OFFSET, p, z ) ;
						lcd_vline( 127-y + X12OFFSET, p, z ) ;
						lcd_rect( y+1 + X12OFFSET, p, x, z ) ;
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
// #ifndef MIXER_TASK
        getADC_osmp();
// #endif
#endif

			if ( ( keyDown() ) )
			{
				return ;  //wait for key release
			}
			 
			if ( hasStickMoved( inacSum ) )
			{
				if ( ++anaCount > 5 )
				{
					return ;  //wait for key release
				}
			}
			else
			{
				anaCount = 0 ;
			}
			if ( check_power_or_usb() )
			{
				return ;		// Usb on or power off
			}
			check_backlight() ;
  	  wdt_reset();
  	}
	}
}


//global helper vars
//bool    checkIncDec_Ret;
//struct t_p1 P1values ;
uint8_t LongMenuTimer ;
uint8_t StepSize = 20 ;

#ifdef TOUCH
void displayAdjustIcons()
{
	lcdDrawIcon( LCD_W-TICON_SIZE-2, 60, IconHplus, 0 ) ;
	lcdDrawIcon( LCD_W-TICON_SIZE-2, 110, IconHminus, 0 ) ;

	lcdDrawIcon( LCD_W-TICON_SIZE-2, 10, IconH2plus, 0 ) ;
	lcdDrawIcon( LCD_W-TICON_SIZE-2, 160, IconH2minus, 0 ) ;

}
#endif

#ifdef TOUCH
int16_t TouchAdjustValue ;	// 
#endif

int16_t checkIncDec16( int16_t val, int16_t i_min, int16_t i_max, uint8_t i_flags)
{
  int16_t newval = val;
  uint8_t kpl=KEY_RIGHT, kmi=KEY_LEFT, kother = -1;
	uint32_t editAllowed = 1 ;
#ifdef TOUCH
	uint32_t largeChange = 0 ;
	TouchAdjustValue = 0 ;
#endif
	
	if ( g_eeGeneral.forceMenuEdit && (s_editMode == 0) && ( i_flags & NO_MENU_ONLY_EDIT) == 0 )
	{
		editAllowed = 0 ;
	}

		uint8_t event = Tevent ;
#ifdef TOUCH
 	if( !(i_min==0 && i_max==1 ) )
	{
		if ( s_editMode )
		{
			displayAdjustIcons() ;
			if ( TouchUpdated )
	  	{
				TouchUpdated = 0 ;
				if ( ( TouchControl.event == TEVT_UP ) || (TouchControl.repeat) )
				{
					if ( TouchControl.x > 430 )
					{
						if ( ( TouchControl.y >= 10 ) && ( TouchControl.y <= 110 ) )
						{
							if ( event == 0 )
							{
								event = EVT_KEY_FIRST(KEY_RIGHT) ;
								TouchAdjustValue = 1 ;
								if ( TouchControl.y <= 60 )
								{
									largeChange = 1 ;
									TouchAdjustValue = StepSize ;
								}
							}
						}
						else if ( ( TouchControl.y >= 110 ) && ( TouchControl.y <= 210 ) )
						{
							if ( event == 0 )
							{
								event = EVT_KEY_FIRST(KEY_LEFT) ;
								TouchAdjustValue = -1 ;
								if ( TouchControl.y >= 160 )
								{
									largeChange = 1 ;
									TouchAdjustValue = -StepSize ;
								}
							}
						}
					}
					else
					{
						s_editMode = 0 ;		// Finished
					}
				}
			}
		}
		else
		{
			lcdDrawIcon( LCD_W-TICON_SIZE-2, 110, IconHedit, 0 ) ;
			if ( TouchUpdated )
	  	{
				TouchUpdated = 0 ;
				if ( TouchControl.event == TEVT_UP )
				{
					if ( TouchControl.x > 430 )
					{
						if ( ( TouchControl.y >= 110 ) && ( TouchControl.y <= 160 ) )
						{
							s_editMode = !s_editMode;
						}
					}
				}
			}
		}
	}
	else
	{
		// display toggle icon and handle it
		lcdDrawIcon( LCD_W-TICON_SIZE-2, 110, IconHtoggle, 0 ) ;
		if ( TouchUpdated )
	  {
			TouchUpdated = 0 ;
			if ( TouchControl.event == TEVT_UP )
			{
				if ( TouchControl.x > 430 )
				{
					if ( ( TouchControl.y >= 110 ) && ( TouchControl.y <= 160 ) )
					{
						if ( event == 0 )
						{
							event = EVT_KEY_BREAK(BTN_RE) ;
						}
					}
				}
			}
		}
	}

#endif
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
#ifdef TOUCH
			if ( largeChange )
			{
    		newval += StepSize ;
			}		 
			else
#endif
			{			
#ifdef PCBXLITE
				if ((GPIOE->IDR & 0x0100) == 0 )
#else
				if ( menuPressed() )
#endif
				{
    			newval += StepSize ;
					LongMenuTimer = 254 ;
				}		 
				else
				{
  	  		newval += 1 ;
				}
			}
			audioDefevent(AU_KEYPAD_UP);
#if defined(PCBX10)
    	kother = KEY_DOWN ;
#else
    	kother=kmi;
#endif
		}

  }else if(event==EVT_KEY_FIRST(kmi) || event== EVT_KEY_REPT(kmi) || (s_editMode && (event==EVT_KEY_FIRST(KEY_DOWN) || event== EVT_KEY_REPT(KEY_DOWN))) )
	{
		if ( editAllowed )
		{
#ifdef TOUCH
			if ( largeChange )
			{
    		newval -= StepSize ;
			}		 
			else
#endif
			{
#ifdef PCBXLITE
				if ((GPIOE->IDR & 0x0100) == 0 )
#else
				if ( menuPressed() )
#endif
				{
    			newval -= StepSize ;
					LongMenuTimer = 254 ;
				}		 
				else
				{
    			newval -= 1 ;
				}
			}
			audioDefevent(AU_KEYPAD_DOWN);
#if defined(PCBX10)
    	kother = KEY_UP ;
#else
    	kother=kpl;
#endif
		}

  }
#if defined(PCBX10)
	if ( s_editMode )
	{
	  if((kother != (uint8_t)-1) && keyState((EnumKeys)kother))
		{
	    newval = -val ;
  	  killEvents(KEY_UP);
    	killEvents(KEY_DOWN);

		}
	}
#endif
  if((kother != (uint8_t)-1) && keyState((EnumKeys)kother))
	{
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
     	killEvents(EVT_KEY_FIRST(KEY_MENU)) ; // Allow Dbl for BTN_RE
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
			else
			{
				// look for trim switches
				uint32_t i ;

				for( i = HSW_Etrmdn ; i < HSW_Etrmdn + 8 ; i += 1 )
				{
					if ( hwKeyState( i ) )
					{
						newval = switchUnMap( i ) ;
						break ;
					}
				}
			}
		}
  }

  //change values based on P1
//  newval -= P1values.p1valdiff;
	if ( RotaryState == ROTARY_VALUE )
	{
//		newval += ( menuPressed() || encoderPressed() ) ? Rotary_diff * 20 : Rotary_diff ;

		if ( Rotary_diff )
		{
			if ( Rotary_diff > 0 )
			{
#ifdef PCBXLITE
				if ((GPIOE->IDR & 0x0100) == 0 )
#else
				if ( menuPressed() )
#endif
				{
					newval += StepSize ;
					LongMenuTimer = 254 ;
				}
				else
				{					
					newval += RotencSpeed * Rotary_diff ;
				}
			}
			else
			{
#ifdef PCBXLITE
				if ((GPIOE->IDR & 0x0100) == 0 )
#else
				if ( menuPressed() )
#endif
				{
					newval -= StepSize ;
					LongMenuTimer = 254 ;
				}
				else
				{					
					newval += RotencSpeed * Rotary_diff ;
//					newval -= RotencSpeed ;
				}
			}
		}

//#ifdef PCBX12D
//		Rotary_diff = 0 ;
//#endif
	}
  if(newval>i_max)
  {
    newval = i_max;
		if ( event != EVT_KEY_REPT(KEY_MENU) )
		{
    	killEvents(event) ;
		}
    audioDefevent(AU_KEYPAD_UP);
  }
  else if(newval < i_min)
  {
    newval = i_min;
		if ( event != EVT_KEY_REPT(KEY_MENU) )
		{
    	killEvents(event) ;
		}
    audioDefevent(AU_KEYPAD_DOWN);
  }
  if(newval != val)
	{
//		if ( menuPressed() )
//		{
//			LongMenuTimer = 255 ;
//		}
    if(newval==0)
		{
			if ( event )
			{
   	  	pauseEvents(event);
			}
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
//    checkIncDec_Ret = true;
  }
//  else {
//    checkIncDec_Ret = false;
//  }
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

#if defined(USB_JOYSTICK) || defined(PCBSKY)
#ifndef SIMU
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
extern USB_OTG_CORE_HANDLE USB_OTG_dev;
#endif
#ifdef PCBSKY
extern uint8_t HIDDJoystickDriver_Change( uint8_t *data ) ;
#define HID_IN_PACKET 9
#endif

/*
  Prepare and send new USB data packet

  The format of HID_Buffer is defined by
  USB endpoint description can be found in 
  file usb_hid_joystick.c, variable HID_JOYSTICK_ReportDesc
*/

#ifdef USB_JOYSTICK
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
  for (int i = 0; i < 8; ++i) {
    int16_t value = g_chans512[i] / 8;
    if ( value > 127 ) value = 127;
    else if ( value < -127 ) value = -127;
    HID_Buffer[i+1] = static_cast<int8_t>(value);  
  }

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10)
  USBD_HID_SendReport (&USB_OTG_dev, HID_Buffer, HID_IN_PACKET );
#endif
#ifdef PCBSKY
	HIDDJoystickDriver_Change( HID_Buffer ) ;
#endif
}
#endif
#endif
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
	
	value = phyStick[index] ;
	value /= 8 ;
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

#if defined(PCBX12D) || defined(PCBX10)
//extern uint8_t LastEvent ;
#endif

// Auto repeat on trim switches??
//	if ( ( key >= HSW_Etrmdn ) && ( key <= HSW_Ttrmup ) )

static void	processAdjusters()
{
static uint8_t GvAdjLastSw[NUM_GVAR_ADJUST + EXTRA_GVAR_ADJUST][2] ;
	for ( CPU_UINT i = 0 ; i < NUM_GVAR_ADJUST + EXTRA_GVAR_ADJUST ; i += 1 )
	{
		GvarAdjust *pgvaradj ;
		pgvaradj = ( i >= NUM_GVAR_ADJUST ) ? &g_model.egvarAdjuster[i - NUM_GVAR_ADJUST] : &g_model.gvarAdjuster[i] ;
		uint32_t idx = pgvaradj->gvarIndex ;
	
		int8_t sw0 = pgvaradj->swtch ;
		int8_t sw1 = 0 ;
		uint32_t switchedON = 0 ;
		int32_t value ;
//#if MULTI_GVARS
//		value = 7 ;
//		if ( g_model.flightModeGvars )
//		{
//			value = 12 ;
//		}
//		if ( pgvaradj->gvarIndex >= value )
//		{
//			value = getTrimValueAdd( CurrentPhase, idx - value  ) ;
//		}
//		else
//		{
//			value = getGvarFm(idx, CurrentPhase ) ;
//		}
//#else			
		if ( pgvaradj->gvarIndex > 6 )
		{
			value = getTrimValueAdd( CurrentPhase, idx - 7  ) ;
		}
		else
		{
			value = getGvar(idx) ;
		}
//#endif
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
//#if MULTI_GVARS
//		if ( g_model.flightModeGvars )
//		{
//			if ( pgvaradj->gvarIndex > 11 )
//			{
//				if(value > 125)
//				{
//					value = 125 ;
//				}	
//  			if(value < -125 )
//				{
//					value = -125 ;
//				}	
//				setTrimValueAdd( CurrentPhase, idx - 12, value ) ;
//			}
//			else
//			{
//				int32_t minmax ;
//				minmax = GVAR_MAX-g_model.xgvars[idx].max ;
//				if(value > minmax)
//				{
//					value = minmax ;
//				}	
//				minmax = GVAR_MIN+g_model.xgvars[idx].min ;
//  			if(value < minmax )
//				{
//					value = minmax ;
//				}	
//				setGVarFm( idx, value, CurrentPhase ) ;
//			}
//		}
//		else
//		{
//			if(value > 125)
//			{
//				value = 125 ;
//			}	
//  		if(value < -125 )
//			{
//				value = -125 ;
//			}	
//			if ( pgvaradj->gvarIndex > 6 )
//			{
//				setTrimValueAdd( CurrentPhase, idx - 7, value ) ;
//			}
//			else
//			{
//				setGVar( idx, value ) ;
//			}
//		}
//#else			 
		if(value > 125)
		{
			value = 125 ;
		}	
  	if(value < -125 )
		{
			value = -125 ;
		}	
		if ( pgvaradj->gvarIndex > 6 )
		{
			setTrimValueAdd( CurrentPhase, idx - 7, value ) ;
		}
		else
		{
			setGVar( idx, value ) ;
		}
//#endif	
	}
}

#ifndef GPIOENCODER
 #ifdef PCBX9D
uint8_t AnaEncSw = 0 ;
 #endif

 #ifdef PCBX9D
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
 #endif
#endif

uint8_t GvarSource[4] ;

#ifdef USE_VARS
// type = 0 for Var, 1 for GVAR
int16_t getGVSourceValue( uint8_t src, uint32_t type )
{
	int16_t value ;

	if ( src == 0 )
	{
		return 0 ;
	}
	if ( src >= EXTRA_POTS_START )
	{
		value = calibratedStick[src-EXTRA_POTS_START+7] ;
		if ( type )
		{
			value /= 8 ;
		}
	}
	else if ( src <= 4 )
	{
		value = getTrimValueAdd( CurrentPhase, src - 1 ) ;
		TrimInUse[src-1] |= 1 ;
		GvarSource[src-1] = 1 ;
	}
	else if ( src == 5 )	// REN
	{
		value = 0 ;
	}
	else if ( src <= 9 )	// Stick
	{
		value = calibratedStick[ src-5 - 1 ] ;
		if ( type )
		{
			value /= 8 ;
		}
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 12 )	// Pot
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
	else if ( src <= 13 )	// Pot
#endif
#if defined(PCBX12D) || defined(PCBX10)
	else if ( src <= 13 )	// Pot
#endif
	{
		value = calibratedStick[ ( src-6)] ;
		if ( type )
		{
			value /= 8 ;
		}
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 36 )	// Chans
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
	else if ( src <= 37 )	// Pot
#endif
#if defined(PCBX12D) || defined(PCBX10)
	else if ( src <= 37 )	// Pot
#endif
	{
#if defined(PCBSKY) || defined(PCB9XT)
		value = ex_chans[src-13] ;
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
		value = ex_chans[src-14] ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		value = ex_chans[src-14] ;
#endif
		if ( type )
		{
			value *= 100 ;
		}
		else
		{
			value *= 1000 ;
		}
		value /= 1024 ;
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 44 )	// Scalers
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
	else if ( src <= 45 )	// Scalers
#endif
#if defined(PCBX12D) || defined(PCBX10)
	else if ( src <= 45 )	// Scalers
#endif
	{
#if defined(PCBSKY) || defined(PCB9XT)
		value = calc_scaler( src-37, 0, 0 ) ;
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
		value = calc_scaler( src-38, 0, 0 ) ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		value = calc_scaler( src-38, 0, 0 ) ;
#endif
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 68 )	// Scalers
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
	else if ( src <= 69 )	// Scalers
#endif
#if defined(PCBX12D) || defined(PCBX10)
	else if ( src <= 69 )	// Scalers
#endif
	{ // Outputs
		int32_t x ;
#if defined(PCBSKY) || defined(PCB9XT)
		x = g_chans512[src-45] ;
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
		x = g_chans512[src-46] ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		x = g_chans512[src-46] ;
#endif
		x *= 100 ;
		value = x / 1024 ;
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 68+NUM_RADIO_VARS )	// Scalers
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
	else if ( src <= 69+NUM_RADIO_VARS )	// Scalers
#endif
#if defined(PCBX12D) || defined(PCBX10)
	else if ( src <= 69+NUM_RADIO_VARS )	// Scalers
#endif
	{ // Radio Vars
#if defined(PCBSKY) || defined(PCB9XT)
		value = g_eeGeneral.radioVar[src-69] ;
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
		value = g_eeGeneral.radioVar[src-70] ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		value = g_eeGeneral.radioVar[src-70] ;
#endif
	}
	else
	{
		value = 0 ;
	}
	if ( type )
	{
		if ( value > 125 )
		{
			value = 125 ;
		}
		if ( value < -125 )
		{
			value = -125 ;
		}
	}
	else
	{
		if ( value > 1000 )
		{
			value = 1000 ;
		}
		if ( value < -1000 )
		{
			value = -1000 ;
		}
	}
 	return value ;
}

int16_t getVarSourceValue( uint8_t src )
{
	return getGVSourceValue( src, 0 ) ;
}

#endif


int16_t getGvarSourceValue( uint8_t src )
{
#ifdef USE_VARS
	return getGVSourceValue( src, 1 ) ;
#else	
	int16_t value ;

	if ( src >= EXTRA_POTS_START )
	{
		value = calibratedStick[src-EXTRA_POTS_START+7] / 8 ;
	}
	else if ( src <= 4 )
	{
		value = getTrimValueAdd( CurrentPhase, src - 1 ) ;
		TrimInUse[src-1] |= 1 ;
		GvarSource[src-1] = 1 ;
	}
	else if ( src == 5 )	// REN
	{
		value = 0 ;
	}
	else if ( src <= 9 )	// Stick
	{
		value = calibratedStick[ src-5 - 1 ] / 8 ;
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 12 )	// Pot
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
	else if ( src <= 13 )	// Pot
#endif
#if defined(PCBX12D) || defined(PCBX10)
	else if ( src <= 13 )	// Pot
#endif
	{
		value = calibratedStick[ ( src-6)] / 8 ;
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 36 )	// Chans
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
	else if ( src <= 37 )	// Pot
#endif
#if defined(PCBX12D) || defined(PCBX10)
	else if ( src <= 37 )	// Pot
#endif
	{
#if defined(PCBSKY) || defined(PCB9XT)
		value = ex_chans[src-13] * 100 / 1024 ;
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
		value = ex_chans[src-14] * 100 / 1024 ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		value = ex_chans[src-14] * 100 / 1024 ;
#endif
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 44 )	// Scalers
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
	else if ( src <= 45 )	// Scalers
#endif
#if defined(PCBX12D) || defined(PCBX10)
	else if ( src <= 45 )	// Scalers
#endif
	{
#if defined(PCBSKY) || defined(PCB9XT)
		value = calc_scaler( src-37, 0, 0 ) ;
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
		value = calc_scaler( src-38, 0, 0 ) ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		value = calc_scaler( src-38, 0, 0 ) ;
#endif
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 68 )	// Scalers
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
	else if ( src <= 69 )	// Scalers
#endif
#if defined(PCBX12D) || defined(PCBX10)
	else if ( src <= 69 )	// Scalers
#endif
	{ // Outputs
		int32_t x ;
#if defined(PCBSKY) || defined(PCB9XT)
		x = g_chans512[src-45] ;
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
		x = g_chans512[src-46] ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		x = g_chans512[src-46] ;
#endif
		x *= 100 ;
		value = x / 1024 ;
	}
#if defined(PCBSKY) || defined(PCB9XT)
	else if ( src <= 68+NUM_RADIO_VARS )	// Scalers
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
	else if ( src <= 69+NUM_RADIO_VARS )	// Scalers
#endif
#if defined(PCBX12D) || defined(PCBX10)
	else if ( src <= 69+NUM_RADIO_VARS )	// Scalers
#endif
	{ // Radio Vars
#if defined(PCBSKY) || defined(PCB9XT)
		value = g_eeGeneral.radioVar[src-69] ;
#endif
#if defined(PCBX9D) || defined(PCBLEM1)
		value = g_eeGeneral.radioVar[src-70] ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
		value = g_eeGeneral.radioVar[src-70] ;
#endif
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
#endif
}


#if defined(PCBX12D) || defined(PCBX10)
uint16_t TestTime ;
uint16_t TestNote ;

uint16_t ClearTime ;
uint16_t MenuTime ;
uint16_t Xcounter ;
#endif

#ifdef  LUA
extern char lua_warning_info[] ;
#endif

uint8_t ScriptActive ;

void runMixer()
{
#ifdef ARUNI
	if ( g_eeGeneral.filterInput == 0 )
	{
		getADC_single() ;
	}
	else if ( g_eeGeneral.filterInput == 3 )
	{
		getADC_filt() ;
	}
	else
	{
		getADC_osmp() ;   // filterInput: 1 or 2
	}
  if (qSixPosCalibrating() || g_eeGeneral.sixPosDelayFilter) {
    if (SixPosDelay && --SixPosDelay == 0) {
      uint8_t sixpos = (( g_eeGeneral.analogMapping & MASK_6POS ) >> 2) + 3;
      S_anaFilt[sixpos] = (((uint16_t)SixPosValue) << 4); // make 7b to 11b
    }
  }
#else
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
#endif
#if (defined(PCBSKY) && !defined(ARUNI))
	if ( ( g_eeGeneral.ar9xBoard == 0 ) && ( g_eeGeneral.extraPotsSource[0] != 2 ) && ( g_eeGeneral.extraPotsSource[1] != 2 ) )
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
	{
		MixerCount += 1 ;		
		uint16_t t1 = getTmr2MHz() ;
//		MixerRunAtTime = t1 ;
		perOutPhase(g_chans512, 0);
		t1 = getTmr2MHz() - t1 ;
		g_timeMixer = t1 ;
	}
}

static void updateVbat()
{
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

        int32_t ab = anaIn(12);

        ab = ( ab + ab*(g_eeGeneral.vBatCalib)/128 ) * 4191 ;
//        ab = (uint16_t) ab / (g_eeGeneral.disableBG ? 240 : BandGap ) ;  // ab might be more than 32767
#ifdef PCBSKY
        ab /= 55296  ;
        g_vbat100mV = ( (ab + g_vbat100mV + 1) >> 1 ) + 3 ;  // Filter it a bit => more stable display
								// Also add on 0.3V for voltage drop across input diode
#endif
#ifdef PCBX9D
 #if defined(PCBX9LITE) || defined(PCBX7ACCESS)
        ab /= 71089  ;
 #else
  #if defined(PCBXLITE)
        ab /= 64626  ;
  #else
        ab /= 57165  ;
  #endif
 #endif
        g_vbat100mV = ( (ab + g_vbat100mV + 1) >> 1 ) ;  // Filter it a bit => more stable display
#endif
#ifdef PCB9XT
        ab /= 64535  ;
        g_vbat100mV = ( (ab + g_vbat100mV + 1) >> 1 ) ;  // Filter it a bit => more stable display
#endif
#if defined(PCBX12D) || defined(PCBX10)
 #if defined(PCBTX16S)
        ab /= 63802  ;
 #else
        ab /= 68631  ;
 #endif
        g_vbat100mV = ( (ab + g_vbat100mV + 1) >> 1 ) ;  // Filter it a bit => more stable display
#endif

 #if defined(PCBLEM1)
        ab /= 86670  ;
        g_vbat100mV = ( (ab + g_vbat100mV + 1) >> 1 ) ;  // Filter it a bit => more stable display
 #endif

        static uint8_t s_batCheck;
        s_batCheck+=16;
        if(s_batCheck==0)
				{
					if( (g_vbat100mV<g_eeGeneral.vBatWarn) && (g_vbat100mV>49) )
					{
						voiceSystemNameNumberAudio( SV_TXBATLOW, V_BATTERY_LOW, AU_TX_BATTERY_LOW ) ;
            if (g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
					}
#ifdef PCBSKY
 #ifndef ARUNI
					else if ( ( g_eeGeneral.mAh_alarm ) && ( ( MAh_used + Current_used/3600 ) / 500 >= g_eeGeneral.mAh_alarm ) )
					{
						if ( ( g_eeGeneral.ar9xBoard == 0 ) && ( g_eeGeneral.extraPotsSource[0] != 2 ) && ( g_eeGeneral.extraPotsSource[1] != 2 ) )
						{
							voiceSystemNameNumberAudio( SV_TXBATLOW, V_BATTERY_LOW, AU_TX_BATTERY_LOW ) ;
						}
					}
 #endif
#endif
        }
    break ;

  }
}

static void checkStickScroll()
{
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
}


void perMain( uint32_t no_menu )
{
  static uint16_t lastTMR;
	uint16_t t10ms ;
	
	t10ms = get_tmr10ms() ;
  tick10ms = ((uint16_t)(t10ms - lastTMR)) != 0 ;
  lastTMR = t10ms ;

#ifndef MIXER_TASK
	runMixer() ;
#endif

	if(tick5ms)
	{
#ifdef PCBLEM1
void checkDsmTelemetry5ms() ;
		checkDsmTelemetry5ms() ;
#endif
		check_frsky( 1 ) ;
		tick5ms = 0 ;

		if (!tick10ms)
		{
			// Run background script here
			basicTask( 0, SCRIPT_BACKGROUND ) ;
		}
	}

	if(!tick10ms) return ; //make sure the rest happen only every 10ms.

#ifdef SERIAL_HOST
	Host10ms = 1 ;
#endif
	if ( ppmInValid )
	{
		if ( --ppmInValid == 0 )
		{
			// Announce trainer lost
			putSystemVoice( SV_TRN_LOST, 0 ) ;
		}
	}

//#ifdef PCBSKY
//extern void usbMassStorage( void ) ;
//	usbMassStorage() ;
//#endif

#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX7) || defined (PCBX9LITE)
#ifdef BLUETOOTH	
	if ( BtRxTimer )
	{
		BtRxTimer -= 1 ;
	}
#endif
#endif
	if ( SbusTimer )
	{
		SbusTimer -= 1 ;
	}

	heartbeat |= HEART_TIMER10ms;
  
#ifdef PCB9XT
	processAnalogSwitches() ;
#endif // PCB9XT
#if defined(PCBX12D) || defined(PCBX10)
		uint16_t t1 ;
		t1 = getTmr2MHz();
		TestTime = t1 - TestNote ;
		TestNote = t1 ;
#endif
	
#if defined(PCBX12D) || defined(PCBX10) || defined(MULTI_EVENTS)
	uint8_t evt = 0 ;
#ifdef USE_VARS
	uint8_t tempEvent = 0 ;
#endif
	if ( ( lastTMR & 1 ) == 0 )
	{
		evt = peekEvent() ;
#ifndef SMALL
		tempEvent = evt ;
#endif
  	int8_t  k = (evt & EVT_KEY_MASK) - TRM_BASE ;
  	if ( ( k >= 0 ) && ( k < 8 ) )
		{		
			evt=getEvent() ;
  		checkTrim(evt) ;
		}
		if ( ( lastTMR & 3 ) == 0 )
		{
			evt = getEvent() ;
		}
		else
		{
			evt = 0 ;
		}
	}
#else
	uint8_t evt=getEvent();
  evt = checkTrim(evt);
#endif

#if defined(PCBX12D) || defined(PCBX10)
	if ( t10ms & 1 )
	{
		ledRed() ;
	}
	else
	{
		ledBlue() ;
	}
//	if ( evt )
//	{
//		LastEvent = evt ;
//	}
#endif

		if ( ( evt == 0 ) || ( evt == EVT_KEY_REPT(KEY_MENU) ) || ( evt == EVT_KEY_REPT(BTN_RE) ) )
		{
			uint8_t timer = LongMenuTimer ;
			if ( menuPressed() || encoderPressed() )
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
			if ( timer >= 150 )
			{
				if ( timer != 255 )
				{
					if ( ( lastTMR & 3 ) == 0 )
					{
						evt = EVT_TOGGLE_GVAR ;
//						killEvents( KEY_MENU ) ;
//						killEvents( BTN_RE ) ;
						timer = 255 ;
						s_editMode = 0 ;
					}
				}
			}
			LongMenuTimer = timer ;
		}

//#ifdef PCBSKY
//	int16_t p1d ;

//	struct t_p1 *ptrp1 ;
//	ptrp1 = &P1values ;
	
//	int16_t c6 = calibratedStick[6] ;
//  p1d = ( ptrp1->p1val-c6 )/32;
//  if(p1d)
//	{
//    p1d = (ptrp1->p1valprev-c6)/2;
//    ptrp1->p1val = c6 ;
//  }
//  ptrp1->p1valprev = c6 ;
//  if ( g_eeGeneral.disablePotScroll )
//  {
//    p1d = 0 ;
//	}
//	ptrp1->p1valdiff = p1d ;
//#endif

#if defined(PCBX12D) || defined(PCBX10) || defined(MULTI_EVENTS)
	if ( ( lastTMR & 3 ) == 0 )
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
		Pre_Rotary_diff[0] = x - LastRotaryValue ;
		LastRotaryValue = x ;
	}
//#if defined(PCBX12D) || defined(PCBX10) || defined(MULTI_EVENTS)
//	else
//	{
//		Pre_Rotary_diff[0] = 0 ;
//	}
//#endif

#if defined(PCBX12D) || defined(PCBX10) || defined(MULTI_EVENTS)
	if ( ( lastTMR & 3 ) == 0 )
#endif
	{
		Rotary_diff = Pre_Rotary_diff[3] ;
		Pre_Rotary_diff[3] = Pre_Rotary_diff[2] ;
		Pre_Rotary_diff[2] = Pre_Rotary_diff[1] ;
		Pre_Rotary_diff[1] = Pre_Rotary_diff[0] ;
		Pre_Rotary_diff[0] = 0 ;
		if ( evt == EVT_KEY_FIRST(BTN_RE) )
		{
			Rotary_diff = 0 ;
			Pre_Rotary_diff[3] = 0 ;
			Pre_Rotary_diff[2] = 0 ;
			Pre_Rotary_diff[1] = 0 ;
		}
	}

	if ( Rotary_diff )
	{
		uint16_t now = get_tmr10ms() ;
		uint16_t delay = now - LastRotEvent ;

		if ( delay <= 8 )
		{
			if ( ++RotencCount > 12 )
			{
				RotencCount = 12 ;
			}
			if ( RotencCount > 5 )
			{
				RotencSpeed = (RotencCount-2) / 2 ;
//				RotencSpeed = (delay<4) ? 10 : delay < 6 ? 4 : 2 ;
			}
		}
		else
		{
			RotencSpeed = 1 ;
			RotencCount = 0 ;
		}
		LastRotEvent = now ;
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
#ifndef PCBLEM1
	
	// Clobber option if telemetry script running?????	
	
	if ( option && ( PopupData.PopupActive == 0 ) && ( g_eeGeneral.enableEncMain == 0 ) )
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
//#if MULTI_GVARS
//			if ( g_model.flightModeGvars )
//			{
//				for( uint8_t i = 0 ; i < 12 ; i += 1 )
//				{
//					uint8_t *psource ;
//					psource = ( ( (uint8_t*)&g_model.gvars) + i ) ;
					
//					if ( *psource == 5 )	// REN
//					{
//						int16_t value = getGvar(i) + Rotary_diff ;
//						setGVar( i, limit( (int16_t)-125, value, (int16_t)125 ) ) ;
//				  }
//				}
//			}
//			else
//#endif
			{
				for( uint32_t i = 0 ; i < MAX_GVARS ; i += 1 )
				{
					if ( g_model.gvars[i].gvsource == 5 )	// REN
					{
						if ( getSwitch( g_model.gvswitch[i], 1, 0 ) )
						{
							int16_t value = getGvar(i) + Rotary_diff ;
							setGVar( i, limit( (int16_t)-125, value, (int16_t)125 ) ) ;
						}
				  }
				}
			}
			Rotary_diff = 0 ;
		}
	}
#endif

//	if ( g_eeGeneral.disablePotScroll || option )
	if ( 1 )
	{			 
		if ( g_model.anaVolume )	// NOT Only check if on main screen
		{
			static uint16_t oldVolValue ;
			uint16_t x ;
			uint16_t divisor ;
#if defined(PCBSKY) || defined(PCB9XT)
			if ( g_model.anaVolume < 4 )
#endif
#ifdef PCBX9D
#ifdef PCBX9LITE
		if ( g_model.anaVolume < 2 )
#else
#if defined(PCBX7) || defined (PCBXLITE)
		if ( g_model.anaVolume < 3 )
#else // PCBX7
		if ( g_model.anaVolume < 5 )
#endif // PCBX7
#endif // PCBX9LITE
#endif
#if defined(PCBX12D) || defined(PCBX10)
			if ( g_model.anaVolume < 5 )
#endif // PCBX12D
#ifdef PCBLEM1
		if ( g_model.anaVolume < 2 )
#endif		
			{
				x = calibratedStick[g_model.anaVolume+3] + 1024 ;
				divisor = 2044 ;
			}
			else
			{
				x = getGvar(g_model.anaVolume-1) + 125 ;
				divisor = 249 ;
			}
			if ( abs( oldVolValue - x ) > 4 ) // (divisor/125  ) )
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
    usbStarted = 1 ;
  }
#endif

	checkStickScroll() ;

	GvarSource[0] = 0 ;
	GvarSource[1] = 0 ;
	GvarSource[2] = 0 ;
	GvarSource[3] = 0 ;
	
//#if MULTI_GVARS
//	if ( g_model.flightModeGvars )
//	{
//		for( uint32_t i = 0 ; i < 12 ; i += 1 )
//		{
//			uint8_t *psource ;
//			psource = ( ( (uint8_t*)&g_model.gvars) + i ) ;
//			uint8_t src = *psource ;
//			if ( src )
//			{
//				int16_t value ;
//				if ( src == 5 )	// REN
//				{
//					value = getGvar(i) ;	// Adjusted elsewhere
//				}
//				else
//				{
//					value = getGvarSourceValue( src ) ;
//				}
//				setGVar( i, limit( (int16_t)-125, value, (int16_t)125 ) ) ;
//			}
//		}
//	}
//	else
//#endif
	{
		for( uint32_t i = 0 ; i < MAX_GVARS ; i += 1 )
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
					value = getGvar(i) ;	// Adjusted elsewhere
				}
				else
				{
					value = getGvarSourceValue( src ) ;
				}
				setGVar( i, limit( (int16_t)-125, value, (int16_t)125 ) ) ;
			}
		}
	}
	check_frsky( 0 ) ;

#ifdef USE_VARS
	if ( tempEvent != EVT_TOGGLE_GVAR )
	{
		uint32_t k ;
	  k = (tempEvent & EVT_KEY_MASK) - TRM_BASE ;
  	if ( k > 8 )
		{
			tempEvent = 0 ;
		}	
	}
	processVars(tempEvent) ;
#endif

// Here, if waiting for EEPROM response, don't action menus

#ifdef BT_WITH_ENCODER
	btEncTx() ;
#endif

	if ( no_menu == 0 )
	{
		static uint8_t alertKey ;
#if defined(PCBX12D) || defined(PCBX10)
//	 if ( ( lastTMR & 1 ) == 0 )
//	 {
//		uint16_t t1 ;
//		t1 = getTmr2MHz();
		waitLcdClearDdone() ;
#else
		if ( ScriptActive == 0 )
		{
	    lcd_clear() ;
		}
#endif

//#if defined(PCBX12D) || defined(PCBX10)
//		t1 = getTmr2MHz() - t1 ;
//		if ( ++Xcounter > 50 )
//		{
//			ClearTime = t1 ;
//			Xcounter = 0 ;
//		}
//	 }
//#endif

#if defined(PCBX12D) || defined(PCBX10)
			DisplayOffset = 0 ;
#endif
		 
		if ( AlertMessage )
		{
#if defined(PCBX12D) || defined(PCBX10) || defined(MULTI_EVENTS)
		 if ( ( lastTMR & 3 ) == 0 )
		 {
#endif
			almess( AlertMessage, AlertType ) ;
#ifdef LUA
			if ( lua_warning_info[0] )
			{
				coord_t i = 0 ;
				coord_t j = 4*FH ;
				uint8_t c ;
				char *p = lua_warning_info ;
				while ( ( c = *p ) )
				{
					PUTC( i, j, c ) ;
#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
					i = LcdNextPos ;
#else
					i = Lcd_lastPos ;
#endif
					if ( i > 20*FW )
					{
						i = 0 ;
						j += FH ;
						if ( j > 7*FH )
						{
							break ;
						}
					}
					p += 1 ;
				}
  		  refreshDisplay() ;
#if defined(PCBX12D) || defined(PCBX10)
				lcd_clearBackground() ;	// Start clearing other frame
#endif
			  wdt_reset() ;		// 
			}
			else
#endif
#ifdef BASIC
			if ( BasicErrorText[0] )
			{
				uint8_t i = 0 ;
				uint8_t j = 4*FH ;
				uint8_t c ;
				char *p = (char *)BasicErrorText ;
				while ( ( c = *p ) )
				{
					PUTC( i , j, c ) ;
#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
					i = LcdNextPos ;
#else
					i = Lcd_lastPos ;
#endif
					if ( i > 20*FW )
					{
						i = 0 ;
						j += FH ;
						if ( j > 7*FH )
						{
							break ;
						}
					}
					p += 1 ;
				}
  		  refreshDisplay() ;
#if defined(PCBX12D) || defined(PCBX10)
				lcd_clearBackground() ;	// Start clearing other frame
#endif
			  wdt_reset() ;		//
			}
			else
#endif
			{
  		  refreshDisplay() ;
#if defined(PCBX12D) || defined(PCBX10)
				lcd_clearBackground() ;	// Start clearing other frame
#endif
			  wdt_reset() ;		//
			}
			
			uint8_t key = keyDown() ;
			killEvents( evt ) ;
			evt = 0 ;
			if ( alertKey )
			{
				if ( alertKey == 1 )
				{
					if( key == 0 )
					{
						alertKey = 2 ;
					}
				}
				else if ( alertKey == 2 )
				{
					if( key )
					{
						alertKey = 3 ;
					}
				}
				else
				{
					if( key == 0 )
					{
						AlertMessage = 0 ;
#ifdef LUA
						if ( lua_warning_info[0] )
						{
							lua_warning_info[0] ='\0' ;
						}
#endif
#ifdef BASIC
						if ( BasicErrorText[0] )
						{
							BasicErrorText[0] ='\0' ;
						}
#endif
					}
				}
			}
			else if ( key )
			{
				alertKey = 1 ;
			}
			else
			{
				alertKey = 2 ;
			}
#if defined(PCBX12D) || defined(PCBX10) || defined(MULTI_EVENTS)
		 }
#endif
		}
		else
		{
			alertKey = 0 ;
    	
#ifdef MULTI_EVENTS
			if ( ( lastTMR & 3 ) == 0 )
#endif
			{
				if ( EnterMenu )
				{
					evt = EnterMenu ;
					EnterMenu = 0 ;
					audioDefevent(AU_MENUS);
				}
		 		StepSize = 20 ;
	 			Tevent = evt ;
			}

#ifdef PCBX9D
			{
extern uint8_t ImageDisplay ;
extern uint8_t ImageX ;
extern uint8_t ImageY ;
				ImageX = 130 ;
				ImageY = 32 ;
				ImageDisplay = 1 ;
			} 
#endif
#if defined(PCBX12D) || defined(PCBX10)
			{
extern uint8_t ImageDisplay ;
extern uint8_t ImageX ;
extern uint8_t ImageY ;
				ImageX = 130 ;
				ImageY = 32 ;
				ImageDisplay = 1 ;
			} 
#endif

#ifdef WHERE_TRACK
			notePosition('a') ;
#endif
  
#if defined(LUA) || defined(BASIC)
  // if Lua standalone, run it and don't clear the screen (Lua will do it)
  					// else if Lua telemetry view, run it and don't clear the screen
  // else clear screen and show normal menus
//
  	
//#ifdef PAGE_NAVIGATION
			int8_t evtAsRotary = 0 ;
			if ( evt == 0 )
			{
extern int32_t Rotary_diff ;
				if ( Rotary_diff > 0 )
				{
					evt = EVT_ROTARY_RIGHT ;
//					evt = EVT_KEY_FIRST(KEY_DOWN) ;
					evtAsRotary = 1 ;
				}
				else if ( Rotary_diff < 0 )
				{
					evt = EVT_ROTARY_LEFT ;
//					evt = EVT_KEY_FIRST(KEY_UP) ;
					evtAsRotary = 1 ;
				}
			}
//#endif
				uint32_t refreshNeeded = 0 ;
				uint32_t telemetryScriptRunning = 0 ;
#ifdef LUA
			if ( g_model.basic_lua )
			{
#ifdef MULTI_EVENTS
			if ( ( lastTMR & 3 ) == 0 )
			{
#endif
				refreshNeeded = luaTask( evt, RUN_STNDAL_SCRIPT, true ) ;
  			if ( !refreshNeeded )
				{
    			refreshNeeded = luaTask( evt, RUN_TELEM_FG_SCRIPT, true ) ;
  			}
  			if ( refreshNeeded )
				{
					refreshNeeded = 2 ;
	//				if (evtAsRotary)
	//				{
	//					Rotary_diff = 0 ;
	//				}
				}
#ifdef MULTI_EVENTS
			}
#endif
			}
			else
#endif
			{
extern uint32_t TotalExecTime ;
				uint16_t execTime = getTmr2MHz() ;
				refreshNeeded = basicTask( evt|(evtAsRotary<<8), SCRIPT_LCD_OK	| SCRIPT_STANDALONE ) ;
				execTime = (uint16_t)(getTmr2MHz() - execTime) ;
				TotalExecTime += execTime ;
				if ( refreshNeeded == 3 )
				{
					refreshNeeded = 0 ;
					if ( sd_card_ready() )
					{
						evt = 0 ;
					}
					// standalone finished so:
					basicLoadModelScripts() ;
				}
				else
				{
	  			if ( !refreshNeeded )
					{
    				refreshNeeded = basicTask( PopupData.PopupActive ? 0 : evt|(evtAsRotary<<8), SCRIPT_TELEMETRY ) ;
						if ( refreshNeeded )
						{
							telemetryScriptRunning = 1 ;
						}
	  			}
				}
			}
			if ( refreshNeeded )
			{
				ScriptActive = 1 ;
			
				if ( PopupData.PopupActive )
				{
					actionMainPopup( evt ) ;
				}
				else
				{
					if (evtAsRotary)
					{
						Rotary_diff = 0 ;
					}
					if ( telemetryScriptRunning )
					{
						if ( ( evt==EVT_KEY_LONG(KEY_MENU)) || ( evt==EVT_KEY_LONG(BTN_RE) ) )
						{
							PopupData.PopupActive = 3 ;
							PopupData.PopupIdx = 0 ;
    	  			killEvents(evt) ;
							evt = 0 ;
							Tevent = 0 ;
						}
					}
				}
#ifdef WIDE_SCREEN	
extern uint8_t ImageDisplay ;
				ImageDisplay = 0 ;
#endif
			}
	  	else
#endif
			{
#ifdef MULTI_EVENTS
				if ( ( lastTMR & 3 ) == 0 )
#endif
				{
					ScriptActive = 0 ;
				}
#if defined(LUA) || defined(BASIC)
				if (evtAsRotary)
				{
					evt = 0 ;
				}
#endif

#if defined(PCBX12D) || defined(PCBX10)
				t1 = getTmr2MHz();
//			updatePicture() ;
#endif

#ifdef MULTI_EVENTS
			if ( ( lastTMR & 3 ) == 0 )
			{
#endif
				if ( evtAsRotary )
				{
					evt = 0 ;
				}
				g_menuStack[g_menuStackPtr](evt);
				refreshNeeded = 4 ;
#ifdef MULTI_EVENTS
			}
			else
			{
				refreshNeeded = 0 ;
			}
#endif
 #if defined(PCBX12D) || defined(PCBX10)
				t1 = getTmr2MHz() - t1 ;
				if ( Xcounter == 0 )
				{
					MenuTime = t1 ;
				}
 #endif
			}
	#if defined(PCBX9D) || defined(PCBSKY) || defined(PCB9XT)
			if ( ( refreshNeeded == 2 ) || ( ( refreshNeeded == 4 ) ) ) // && ( ( lastTMR & 3 ) == 0 ) ) )
	#endif
  #if defined(PCBX12D) || defined(PCBX10)
		 if ( ( lastTMR & 3 ) == 0 )
		 {
			if ( ( refreshNeeded == 2 ) || ( ( refreshNeeded == 4 ) ) ) // && ( ( lastTMR & 3 ) == 0 ) ) )
	#endif
			{
  #if defined(PCBX12D) || defined(PCBX10)
					displayStatusLine(ScriptActive) ;
					ScriptActive = 0 ;
					if ( g_eeGeneral.screenShotSw )
					{
						if ( getSwitch00( g_eeGeneral.screenShotSw ) )
						{
							if ( LastShotSwitch == 0 )
							{
extern const char *screenshot() ;
								screenshot() ;
							}
							LastShotSwitch = 1 ;
						}
						else
						{
							LastShotSwitch = 0 ;
						}
					}
	#endif
					uint16_t t1 = getTmr2MHz() ;
					refreshDisplay() ;
//#ifdef BASIC
//				BasicRunWithoutRefresh = 0 ;
//#endif
#if defined(PCBX12D) || defined(PCBX10)
					lcd_clearBackground() ;	// Start clearing other frame
#endif
					t1 = getTmr2MHz() - t1 ;
					g_timeRfsh = t1 ;
			}
#if defined(PCBX12D) || defined(PCBX10)
		 }
#endif
		}
	}

#if defined(PCBX12D) || defined(PCBX10)
 #ifndef MIXER_TASK
	checkRunMixer() ;
 #endif
#endif
//#ifdef PCBX12D
//	if ( getSwitch00( HSW_Pb4 ) )
//	{
//		CoSchedLock() ;
//		for(;;)
//		{
//			// Force watchdog reboot
//		}
//  	CoSchedUnlock() ;
//	}
//#endif


#ifdef PCBSKY
	checkTrainerSource() ;
#endif

#ifdef PCB9XT
	checkTrainerSource() ;
#endif

//#if defined(PCBLEM1)
//	checkTrainerSource() ;
//#endif

	updateVbat() ;

  InactivityMonitor = 0; //reset this flag
		
	AUDIO_HEARTBEAT();  // the queue processing

}

#if defined(PCBX7) || defined(PCBX9LITE)
 #ifndef PCBT12
static void init_rotary_encoder()
{
#ifdef PCBX9LITE
  register uint32_t capture ;
	
	configure_pins( 0x1400, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
	g_eeGeneral.rotaryDivisor = 2 ;

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN ;		// Enable clock
	SYSCFG->EXTICR[2] |= 0x0400 ;		// PE10
	SYSCFG->EXTICR[3] |= 0x0004 ;		// PE12
	EXTI->RTSR |= 0x1400 ;	// Rising Edge
	EXTI->FTSR |= 0x1400 ;	// Falling Edge
	EXTI->IMR |= 0x1400 ;

	capture = GPIOENCODER->IDR & 0x1400 ;
	capture >>= 10 ;
	capture = (capture & 1) | ( ( capture >> 1 ) & 2 ) ;	// pick out the two bits
	Rotary_position &= ~0x03 ;
	Rotary_position |= capture ;

	NVIC_SetPriority( EXTI15_10_IRQn, 1 ) ; // Not quite highest priority interrupt
	NVIC_EnableIRQ( EXTI15_10_IRQn) ;
#else
  register uint32_t capture ;
	
	configure_pins( 0x0A00, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
	g_eeGeneral.rotaryDivisor = 2 ;

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN ;		// Enable clock
	SYSCFG->EXTICR[2] |= 0x4000 ;		// PE11
	EXTI->RTSR |= 0x0800 ;	// Rising Edge
	EXTI->FTSR |= 0x0800 ;	// Falling Edge
	EXTI->IMR |= 0x0800 ;

	capture = GPIOENCODER->IDR & 0x0A00 ;
	capture >>= 10 ;
	capture = (capture & 1) | ( ( capture >> 1 ) & 2 ) ;	// pick out the two bits
	Rotary_position = capture ;

	NVIC_SetPriority( EXTI15_10_IRQn, 1 ) ; // Not quite highest priority interrupt
	NVIC_EnableIRQ( EXTI15_10_IRQn) ;
#endif
}

  #ifdef PCBX9LITE

//volatile int32_t IRotary_position ;
//volatile int32_t IRotary_count ;

extern "C" void EXTI15_10_IRQHandler()
{
  register uint32_t capture ;
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x5B ;
#else
	RTC->BKP1R = 0x5B ;
#endif
#endif

	capture = GPIOENCODER->IDR & 0x1400 ;
	EXTI->PR = 0x1400 ;
	capture >>= 10 ;
	capture = (capture & 1) | ( ( capture >> 1 ) & 2 ) ;	// pick out the two bits
	if ( capture != ( Rotary_position & 0x03 ) )
	{
		if ( ( Rotary_position & 0x01 ) ^ ( ( capture & 0x02) >> 1 ) )
		{
//			if ( (Rotary_position & 0x03) == 3 )
//			{
				Rotary_count += 1 ;
			}
			else
			{
				Rotary_count -= 1 ;
//			}
		}
		Rotary_position &= ~0x03 ;
		Rotary_position |= capture ;
	}
}

  #else

extern "C" void EXTI15_10_IRQHandler()
{
  register uint32_t capture ;

#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x5A ;
#else
	RTC->BKP1R = 0x5A ;
#endif
#endif
	capture = GPIOENCODER->IDR & 0x0A00 ;
	EXTI->PR = 0x0800 ;
	capture >>= 9 ;
	capture = (capture & 1) | ( ( capture >> 1 ) & 2 ) ;	// pick out the two bits
	
	if ( capture != ( Rotary_position & 0x03 ) )
	{
		uint32_t old = ( Rotary_position & 0x01 ) ^ ( ( Rotary_position & 0x02) >> 1 ) ;
		if ( ( capture & 0x01 ) ^ ( ( capture & 0x02) >> 1 ) )
		{
			// negative
			Rotary_count -= old + 1 ;
			
//			if ( old )
//			{
//				Rotary_count -= 2 ;
//			}
//			else
//			{
//				Rotary_count -= 1 ;
//			}
		}
		else
		{
	 		// positive
			Rotary_count += 2 - old ;
			
//			if ( old )
//			{
//				Rotary_count += 1 ;
//			}
//			else
//			{
//				Rotary_count += 2 ;
//			}
		}
		Rotary_position &= ~0x03 ;
		Rotary_position |= capture ;
	}
}



//void checkRotaryEncoder()
//{
//  register uint32_t dummy ;
	
//	dummy = GPIOENCODER->IDR ;	// Read Rotary encoder ( PE11, PE9 )
//	dummy >>= 9 ;
//	dummy = (dummy & 1) | ( ( dummy >> 1 ) & 2 ) ;	// pick out the two bits
//	if ( dummy != ( Rotary_position & 0x03 ) )
//	{
//		if ( ( Rotary_position & 0x01 ) ^ ( ( dummy & 0x02) >> 1 ) )
//		{
//			Rotary_count -= 1 ;
//		}
//		else
//		{
//			Rotary_count += 1 ;
//		}
//		Rotary_position &= ~0x03 ;
//		Rotary_position |= dummy ;
//	}
//}
  #endif // X3
 #endif

#endif // PCBX7/X3

#if defined(REV19)
static void init_rotary_encoder()
{
  register uint32_t capture ;
	
	configure_pins( 0x0C00, PIN_INPUT | PIN_PULLUP | PIN_PORTE ) ;
	g_eeGeneral.rotaryDivisor = 2 ;

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN ;		// Enable clock
	SYSCFG->EXTICR[2] |= 0x4400 ;		// PE10 & PE11
	EXTI->RTSR |= 0x0C00 ;	// Rising Edge
	EXTI->FTSR |= 0x0C00 ;	// Falling Edge
	EXTI->IMR |= 0x0C00 ;

	capture = GPIOENCODER->IDR & 0x0C00 ;
	capture >>= 10 ;
	capture = (capture & 3) ;	// pick out the two bits
	Rotary_position &= ~0x03 ;
	Rotary_position |= capture ;

	NVIC_SetPriority( EXTI15_10_IRQn, 1 ) ; // Not quite highest priority interrupt
	NVIC_EnableIRQ( EXTI15_10_IRQn) ;
}

////volatile int32_t IRotary_position ;
////volatile int32_t IRotary_count ;

extern "C" void EXTI15_10_IRQHandler()
{
  register uint32_t capture ;

	capture = GPIOENCODER->IDR & 0x0C00 ;
	EXTI->PR = 0x0C00 ;
	capture >>= 10 ;
	capture = (capture & 3) ;	// pick out the two bits
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
#endif

//void checkRotaryEncoder()
//{
//  register uint32_t dummy ;
	
//	dummy = GPIOENCODER->IDR ;	// Read Rotary encoder ( PE12, PE10 )
//	dummy >>= 10 ;
//	dummy = (dummy & 1) | ( ( dummy >> 1 ) & 2 ) ;	// pick out the two bits
//	if ( dummy != ( Rotary_position & 0x03 ) )
//	{
//		if ( ( Rotary_position & 0x01 ) ^ ( ( dummy & 0x02) >> 1 ) )
//		{
//			Rotary_count -= 1 ;
//		}
//		else
//		{
//			Rotary_count += 1 ;
//		}
//		Rotary_position &= ~0x03 ;
//		Rotary_position |= dummy ;
//	}
//}







#ifdef REV9E
static void init_rotary_encoder()
{
  register uint32_t dummy ;
	
	configure_pins( 0x3000, PIN_INPUT | PIN_PULLUP | PIN_PORTD ) ;
	g_eeGeneral.rotaryDivisor = 2 ;
	
	dummy = GPIOENCODER->IDR ;	// Read Rotary encoder ( PE6, PE5 )
	dummy >>= 12 ;
	dummy &= 0x03 ;			// pick out the two bits
	Rotary_position &= ~0x03 ;
	Rotary_position |= dummy ;
}

void checkRotaryEncoder()
{
  register uint32_t dummy ;
	
	dummy = GPIOENCODER->IDR ;	// Read Rotary encoder ( PE6, PE5 )
	dummy >>= 12 ;
	dummy &= 0x03 ;			// pick out the two bits
	if ( dummy != ( Rotary_position & 0x03 ) )
	{
		if ( ( Rotary_position & 0x01 ) ^ ( ( dummy & 0x02) >> 1 ) )
		{
			Rotary_count -= 1 ;
		}
		else
		{
			Rotary_count += 1 ;
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

// uint32_t TempTimer ;

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

extern "C" void PIOC_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x59 ;
#else
	RTC->BKP1R = 0x59 ;
#endif
#endif
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
}
#endif
#endif

//extern uint8_t UserTimer1 ;
#ifdef PCBLEM1
uint8_t ExitTimer ;
#endif

void interrupt5ms()
{
	static uint32_t pre_scale ;		// Used to get 10 Hz counter

#ifdef PCBLEM1
	if ( pre_scale & 1 )
	{
		PortE1 = GPIOE->IDR ;
		PortB1 = GPIOB->IDR ;
		PortD1 = GPIOD->IDR ;
		PortC1 = GPIOC->IDR ;
		SW_DET1_GPIO->BSRR = SW_DET1_PIN ;
		SW_DET3_GPIO->BRR = SW_DET3_PIN ;
		
		if ( GPIOE->IDR & 0x0800 )	// Power button
		{
			ExitTimer = 1 ;
		}
		else
		{
			ExitTimer = 0 ;
		}
	}
	else
	{
		PortE0 = GPIOE->IDR ;
		PortB0 = GPIOB->IDR ;
		PortD0 = GPIOD->IDR ;
		PortC0 = GPIOC->IDR ;
		SW_DET1_GPIO->BRR = SW_DET1_PIN ;
		SW_DET3_GPIO->BSRR = SW_DET3_PIN ;
	}
#endif

	sound_5ms() ;

#ifdef WDOG_REPORT
 #ifdef PCBSKY	
	GPBR->SYS_GPBR1 |= 0x1000 ;
 #else
	RTC->BKP1R |= 0x1000 ;
 #endif
#endif


#if defined(PCBX7) || defined(PCBX9LITE)
 #ifndef PCBT12
  #ifndef PCBX9LITE
//extern void checkRotaryEncoder() ;
//		checkRotaryEncoder() ;
  #endif // X3
 #endif
#endif // PCBX7
#ifdef REV9E
	checkRotaryEncoder() ;
#endif // REV9E

#ifdef PCB9XT
	checkRotaryEncoder() ;
#endif
#if defined(PCBX12D) || defined(PCBX10)
extern void checkRotaryEncoder() ;
  checkRotaryEncoder() ;
#endif
//#ifdef PCBLEM1
//	checkRotaryEncoder() ;
//#endif

	tick5ms = 1 ;
	
	if ( ++pre_scale >= 2 )
	{
#ifdef WDOG_REPORT
 #ifdef PCBSKY	
	GPBR->SYS_GPBR1 |= 0x2000 ;
 #else
	RTC->BKP1R |= 0x2000 ;
 #endif
#endif
		
//		if ( TempTimer == 0 )
//		{
//			TempTimer = 5 ;
////				PE.08 ;
//			configure_pins( 0x0200, PIN_OUTPUT | PIN_PORTE ) ;
//			GPIOE->ODR &= ~0x0200 ;
//		}
//		else
//		{
//			TempTimer -= 1 ;
//			if ( TempTimer == 1 )
//			{
//				GPIOE->ODR |= 0x0200 ;
//			}
//		}
		
		PowerOnTime += 1 ;
		Tenms |= 1 ;			// 10 mS has passed
		pre_scale = 0 ;
//		if ( UserTimer1 )
//		{
//			UserTimer1 -= 1 ;
//		}

  	per10ms();
#ifdef WDOG_REPORT
 #ifdef PCBSKY	
	GPBR->SYS_GPBR1 |= 0x4000 ;
 #else
	RTC->BKP1R |= 0x4000 ;
 #endif
#endif
		if (--AlarmTimer == 0 )
		{
			AlarmTimer = 100 ;		// Restart timer
			AlarmCheckFlag += 1 ;	// Flag time to check alarms
			MixerRate = MixerCount ;
			MixerCount = 0 ;
		}
		if ( --CheckTimer == 0 )
		{
			CheckTimer = 2 ;
			__disable_irq() ;
			CheckFlag20mS = 1 ;
			__enable_irq() ;
		}

		if (--VoiceTimer == 0 )
		{
			VoiceTimer = 10 ;		// Restart timer
//			CheckFlag50mS = 1 ;
			__disable_irq() ;
			VoiceCheckFlag100mS |= 1 ;	// Flag time to check alarms
			__enable_irq() ;
		}
//		if (VoiceTimer == 5 )
//		{
//			CheckFlag50mS = 1 ;
////#ifdef PCBX7
////	GPIOC->ODR ^= PIN_SW_EXT1 ;
////	GPIOD->ODR ^= PIN_SW_EXT2 ;
////#endif
//		}
		
		if (--DsmCheckTimer == 0 )
		{
			DsmCheckTimer = 50 ;		// Restart timer
			DsmCheckFlag |= 1 ;	// Flag time to check alarms
		}
#if defined(PCBX12D) || defined(PCBX10)
extern uint32_t TimeCounter ;
extern uint32_t SecCounter ;

		TimeCounter += 1 ;
		if ( TimeCounter >= 100 )
		{
			TimeCounter = 0 ;
			SecCounter += 1 ;
		}
#endif
	}
#ifdef WDOG_REPORT
 #ifdef PCBSKY	
	GPBR->SYS_GPBR1 |= 0x8000 ;
 #else
	RTC->BKP1R |= 0x8000 ;
 #endif
#endif
}

#ifdef PCBLEM1
extern "C" void TIM4_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x58 ;
#else
	RTC->BKP1R = 0x58 ;
#endif
#endif
	TIM4->SR = TIMER2_5SR_MASK & ~TIM_SR_UIF ;
	interrupt5ms() ;
}

// TO logicio
static const uint8_t SwitchIndices[] = {HSW_SA0,HSW_SB0,HSW_SC0,HSW_SD0,HSW_SE0,HSW_SF2,HSW_SG2,HSW_SH2} ;

uint32_t switchPosition( uint32_t swtch )
{
	if ( swtch < sizeof(SwitchIndices) )
	{
		swtch = SwitchIndices[swtch] ;
	}
	if ( hwKeyState( swtch ) )
	{
		return 0 ;
	}
	swtch += 1 ;
	if ( hwKeyState( swtch ) )
	{
		return 1 ;			
	}
	return 2 ;
}

extern uint8_t switchMapTable[] ;
#endif




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
#if defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
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


#ifdef ARUNI
static uint8_t qSixPosDelayFiltering(uint8_t x, uint16_t y)
{
  if (qSixPosCalibrating() || g_eeGeneral.sixPosDelayFilter) {
    uint8_t sixpos = (( g_eeGeneral.analogMapping & MASK_6POS ) >> 2) + 3;
    if (x == sixpos) {
      uint8_t v7 = (y >> 4);  // 11b to 7b truncation
      uint8_t diff;
      if (v7 > SixPosValue) {
        diff = v7 - SixPosValue;
      } else {
        diff = SixPosValue - v7;
      }
      if (diff > 1) {         // sixpos value changed
        SixPosValue = v7;     // save truncated 7b analog value
        SixPosDelay = 25;     // 25x10ms=250ms delay filtering
      }                       // ala retriggerable one-shot
      return 1;
    }
  }
  return 0;
}
#endif

void getADC_single()
{
	register uint32_t x ;
	uint16_t temp ;
	uint32_t numAnalog = ANALOG_DATA_SIZE ;

	read_adc() ;

	for( x = 0 ; x < numAnalog ; x += 1 )
	{
		temp = AnalogData[x] >> 1;
#ifdef ARUNI
    if (qSixPosDelayFiltering(x, temp))
      continue;
#endif
		S_anaFilt[x] = temp ;
	}
}

#if defined(PCBSKY) || defined(PCB9XT)
#define OSMP_SAMPLES	4
#define OSMP_TOTAL		16384
#define OSMP_ROUNDUP	0
#define OSMP_SHIFT		3
#endif
#ifdef PCBX9D
#define OSMP_SAMPLES	4
#define OSMP_TOTAL		16384
#define OSMP_ROUNDUP	0
#define OSMP_SHIFT		3
//#define OSMP_SAMPLES	8
//#define OSMP_TOTAL		32768
//#define OSMP_SHIFT		4
#endif
#if defined(PCBX12D) || defined(PCBX10)
#define OSMP_SAMPLES	4
#define OSMP_TOTAL		16384
#define OSMP_ROUNDUP	0
#define OSMP_SHIFT		3
#endif

#if defined(PCBLEM1)
#define OSMP_SAMPLES	4
#define OSMP_TOTAL		16384
#define OSMP_ROUNDUP	4
#define OSMP_SHIFT		3
#endif

void getADC_osmp()
{
	register uint32_t x ;
	register uint32_t y ;
	uint32_t numAnalog = ANALOG_DATA_SIZE ;

	uint16_t temp[ANALOG_DATA_SIZE] ;
//#ifndef PCBLEM1
	static uint16_t next_ana[ANALOG_DATA_SIZE] ;
//#endif

#ifdef ARUNI
  uint32_t osmp_samples = OSMP_SAMPLES ;
  uint32_t osmp_shift = OSMP_SHIFT ;
	if ( g_eeGeneral.filterInput == 2 )
	{
    osmp_samples *= 2 ;
    osmp_shift += 1 ;
  }
#endif

	for( x = 0 ; x < numAnalog ; x += 1 )
	{
		temp[x] = 0 ;
	}
#ifdef ARUNI
	for( y = 0 ; y < osmp_samples ; y += 1 )
#else
	for( y = 0 ; y < OSMP_SAMPLES ; y += 1 )
#endif
	{
		read_adc() ;
		
		for( x = 0 ; x < numAnalog ; x += 1 )
		{
			temp[x] += AnalogData[x] ;
		}
	}
	for( x = 0 ; x < ANALOG_DATA_SIZE ; x += 1 )
	{
#ifdef ARUNI
		uint16_t y = temp[x] >> osmp_shift ;
    if (qSixPosDelayFiltering(x, y))
      continue;
#else
		uint16_t y = (temp[x] + OSMP_ROUNDUP) >> OSMP_SHIFT ;
#endif
//#ifndef PCBLEM1
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
//#endif
		S_anaFilt[x] = y ;
	}
}

void getADC_filt()
{
	register uint32_t x ;
	static uint16_t t_ana[2][ANALOG_DATA_SIZE] ;
	uint16_t temp ;
	uint32_t numAnalog = ANALOG_DATA_SIZE ;


	read_adc() ;
	for( x = 0 ; x < numAnalog ; x += 1 )
	{
    uint16_t y = AnalogData[x];
#ifdef ARUNI
    if (qSixPosDelayFiltering(x, y >> 1))
      continue;
#endif
		temp = S_anaFilt[x] ;
		temp = temp/2 + (t_ana[1][x] >> 2 ) ;
		S_anaFilt[x] = temp ;
		t_ana[1][x] = ( t_ana[1][x] + t_ana[0][x] ) >> 1 ;
		t_ana[0][x] = ( t_ana[0][x] + y ) >> 1 ;
	}	 
}

char LastItem[8] ;

void setLastIdx( char *s, uint8_t idx )
{
	uint8_t length ;
	length = (uint8_t) *s++ ;

	ncpystr( (uint8_t *)LastItem, (uint8_t *)s+length*idx, length ) ;
}

void setLastTelemIdx( uint8_t idx )
{
	uint8_t *s ;
	uint32_t x ;
	if ( ( idx >= 69 ) && ( idx <= 74 ) ) // A Custom sensor
	{
		x = idx - 69 ;
		x *= 4 ;
		s = &g_model.customTelemetryNames[x] ;
		if ( *s && (*s != ' ' ) )
		{
			ncpystr( (uint8_t *)LastItem, s, 4 ) ;
			return ;
		}
	}
	if ( ( idx >= 38 ) && ( idx <= 45 ) )	// A Scaler
	{
		x = idx - 38 ;
		s = g_model.Scalers[x].name ;
		if ( *s && (*s != ' ' ) )
		{
			ncpystr( (uint8_t *)LastItem, s, 4 ) ;
			return ;
		}
	}
	setLastIdx( (char *) PSTR(STR_TELEM_ITEMS), idx ) ;
}


//#if defined(PCBX12D) || defined(PCBX10)
//void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att, uint16_t colour , uint16_t bgColour )
//#else
//void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att)
//#endif
//{
//	uint8_t chanLimit = NUM_SKYXCHNRAW ;
//	uint8_t mix = att & MIX_SOURCE ;
//	LastItem[0] = '\0' ;
//	if ( mix )
//	{
//		chanLimit += MAX_GVARS + 1 + 1 ;
//		att &= ~MIX_SOURCE ;		
//	}
//  if(idx==0)
//		ncpystr( (uint8_t *)LastItem, (uint8_t *) XPSTR("----"), 4 ) ;
//  else if(idx<=4)
//	{
//		const char *ptr = "" ;
//		if ( g_model.useCustomStickNames )
//		{
//			ptr = ( char *)g_eeGeneral.customStickNames+4*(idx-1) ;
//		}
//		if ( *ptr && (*ptr != ' ' ) )
//		{
//			ncpystr( (uint8_t *)LastItem, (uint8_t *)ptr, 4 ) ;
//		}
//		else
//		{
//			setLastIdx( (char *) PSTR(STR_STICK_NAMES), idx-1 ) ;
//		}
//	}
//  else if(idx<=chanLimit)
//		setLastIdx( (char *) PSTR(STR_CHANS_GV), idx-5 ) ;
//	else if(idx < EXTRA_POTS_START)
//	{
//		if ( mix )
//		{
//			idx += TEL_ITEM_SC1-(chanLimit-NUM_SKYXCHNRAW) ;
//			if ( idx - NUM_SKYXCHNRAW > TEL_ITEM_SC1 + NUM_SCALERS )
//			{
//				uint8_t *ptr ;
//				idx -= TEL_ITEM_SC1 + NUM_SCALERS - 8 + NUM_SKYXCHNRAW ;
//#if EXTRA_SKYCHANNELS							
//				if ( idx > 16 )
//				{
//					if ( idx <= 16 + 8  )
//					{
//						idx += 8 ;
//						ptr = cpystr( (uint8_t *)LastItem, (uint8_t *)"CH" ) ;
//						*ptr++ = (idx / 10) + '0' ;
//						*ptr = (idx % 10) + '0' ;
//					}
//					else
//					{
//						ptr = ncpystr( (uint8_t *)LastItem, (uint8_t *)&PSTR(STR_GV_SOURCE)[1+3*(idx-24)], 3 ) ;
//					}
//				}
//				else
//#endif
//				{
//					ptr = cpystr( (uint8_t *)LastItem, (uint8_t *)"PPM1" ) ;
//					if ( idx == 9 )
//					{
//						*(ptr-1) = '9' ;
//					}
//					else
//					{
//						*ptr = '0' + idx - 10 ;
//					}
//				}
//				*(ptr+1) = '\0' ;
//#if defined(PCBX12D) || defined(PCBX10)
//				lcd_putsAttColour(x,y,LastItem,att, colour, bgColour ) ;
//#else
//				lcd_putsAtt(x,y,LastItem,att);
//#endif
//				return ;
//			}
//		}
//		setLastTelemIdx( idx-NUM_SKYXCHNRAW ) ;
////		setLastIdx( (char *) PSTR(STR_TELEM_ITEMS), idx-NUM_SKYXCHNRAW ) ;
//	}
//	else
//	{
//		if(idx < EXTRA_POTS_START + 8)
//		{
//			setLastIdx( (char *) PSTR(STR_CHANS_EXTRA), idx-EXTRA_POTS_START ) ;
//		}
//		else
//		{
//#ifdef INPUTS
//			if ( mix )
//			{
//				if ( idx >= 224 )	// Input
//				{
//					displayInputName( x, y, idx-224+1, att ) ;
////					lcd_img( x, y, IconInput, 0, 0 ) ;
////					lcd_outdezNAtt(x+3*FW, y, idx-224, att + LEADING0, 2) ;
//					return ;
//				}
//			}
//			else
//#endif
//			{
//				// Extra telemetry items
//				setLastTelemIdx( idx-NUM_SKYXCHNRAW-8 ) ;
////			setLastIdx( (char *) PSTR(STR_TELEM_ITEMS), idx-NUM_SKYXCHNRAW-8 ) ;
//			}
//		}
//	}
//#if defined(PCBX12D) || defined(PCBX10)
//	lcd_putsAttColour(x,y,LastItem,att, colour, bgColour ) ;
//#else
//	lcd_putsAtt(x,y,LastItem,att);
//#endif
//}

void putsChn( coord_t x, coord_t y,uint8_t idx1, LcdFlags att)
{
#if defined(PCBX12D) || defined(PCBX10)
	x *= 2 ;
	y *= 2 ;
	if ( idx1 == 0 )
	{
//    lcd_putsnAtt(x,y,XPSTR("--- "),4,att);
		lcdDrawSizedText( x, y, XPSTR("--- "), 4, att ) ;
	}
	else
	{
    lcdDrawSizedText( x, y, PSTR(STR_CH), 2, att ) ;
		lcdDrawNumber( LcdNextPos, y, idx1, LEFT, 0 ) ;
//  	lcd_outdezAtt(x1,y,idx1,att);
//    lcd_putsnAtt(x,y,PSTR(STR_CH),2,att);
	}

#else
	if ( idx1 == 0 )
	{
    PUTS_ATT_N(x,y,XPSTR("--- "),4,att);
	}
	else
	{
		coord_t x1 ;
		x1 = x + 4*FW-2 ;
		if ( idx1 < 10 )
		{
			x1 -= FWNUM ;			
		}
  	PUTS_NUM(x1,y,idx1,att);
    PUTS_ATT_N(x,y,PSTR(STR_CH),2,att);
	}
#endif
}

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
uint8_t MaxSwitchIndex = MAX_SKYDRSWITCH ;		// For ON and OFF
#endif

#if defined(PCBSKY) || defined(PCB9XT)

uint8_t switchMapTable[100] ;
uint8_t switchUnMapTable[100] ;
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
	*p++ = HSW_Ttrmup ;
	*p++ = HSW_Ttrmdn ;
	*p++ = HSW_Rtrmup ;
	*p++ = HSW_Rtrmdn ;
	*p++ = HSW_Atrmup ;
	*p++ = HSW_Atrmdn ;
	*p++ = HSW_Etrmup ;
	*p++ = HSW_Etrmdn ;
	for ( uint32_t i = 10 ; i <=33 ; i += 1  )
	{
		*p++ = i ;	// Custom switches
	}
	*p = MAX_SKYDRSWITCH ;
	MaxSwitchIndex = p - switchMapTable ;
	*++p = MAX_SKYDRSWITCH+1 ;
	*++p = MAX_SKYDRSWITCH+2 ;
//	*++p = MAX_SKYDRSWITCH+3 ;
//	*++p = MAX_SKYDRSWITCH+4 ;
//	*++p = MAX_SKYDRSWITCH+5 ;

	for ( uint32_t i = 0 ; i <= (uint32_t)MaxSwitchIndex+2 ; i += 1  )
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

#ifndef ARUNI
void create6posTable()
{
	uint32_t i ;

	for ( i = 0 ; i < 5 ; i += 1 )
	{
		uint32_t j ;
		j = (g_eeGeneral.SixPositionCalibration[i+1] + g_eeGeneral.SixPositionCalibration[i]) / 2 ;
		if ( g_eeGeneral.SixPositionCalibration[5] >  g_eeGeneral.SixPositionCalibration[0] )
		{
#ifdef PCBSKY
			j = 4095 - j ;
#else
			j = 2047 - j ;		// 9XT is 11 bit
#endif
		}
		SixPositionTable[i] = j ;
	}
}
#endif // nARUNI

#endif

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
uint8_t switchMapTable[100] ;
uint8_t switchUnMapTable[100] ;

// So, I think I map SA0/1/2 to ELE 3-pos, SC0/1/2 to ID0/1/2, SD0/1/2 to AIL 3-pos, 
// SE0/1/2 to RUD 3-pos, SG0/1/2 to GEA 3-pos, SB0/1/2 to THR 3-pos,
// SF0/2 to THR/RUD and SH0/2 to GEA/TRN.

void createSwitchMapping()
{
	uint8_t *p = switchMapTable ;
	
	*p++ = 0 ;
	*p++ = HSW_SA0 ;
	*p++ = HSW_SA1 ;
	*p++ = HSW_SA2 ;
	
	*p++ = HSW_SB0 ;
	*p++ = HSW_SB1 ;
	*p++ = HSW_SB2 ;

	*p++ = HSW_SC0 ;
#ifdef PCBXLITE
	if (g_eeGeneral.ailsource)
	{
#endif
	*p++ = HSW_SC1 ;
#ifdef PCBXLITE
	}
#endif
	*p++ = HSW_SC2 ;
	 
#ifndef PCBX9LITE
	*p++ = HSW_SD0 ;
 #ifdef PCBXLITE
	if (g_eeGeneral.rudsource)
	{
 #endif
	*p++ = HSW_SD1 ;
 #ifdef PCBXLITE
	}
 #endif
	*p++ = HSW_SD2 ;
#endif // nX3
	 
#ifndef PCBX9LITE
#ifndef PCBX7
#ifndef PCBXLITE
#ifndef PCBLEM1
	*p++ = HSW_SE0 ;
	*p++ = HSW_SE1 ;
	*p++ = HSW_SE2 ;
#endif
#endif
#endif
#endif // nX3

#ifndef PCBXLITE
#ifndef PCBLEM1
	*p++ = HSW_SF2 ;
#endif
#endif

#ifndef PCBX9LITE
#ifndef PCBX7
#ifndef PCBXLITE
#ifndef PCBLEM1
	*p++ = HSW_SG0 ;
	*p++ = HSW_SG1 ;
#endif
	*p++ = HSW_SG2 ;
#endif
#endif
#endif // nX3
	 
#ifndef PCBXLITE
	*p++ = HSW_SH2 ;
#endif

#ifdef REV9E
	if ( g_eeGeneral.ailsource & 1 )
	{
		*p++ = HSW_SI0 ;
		*p++ = HSW_SI1 ;
		*p++ = HSW_SI2 ;
	}
	if ( g_eeGeneral.ailsource & 2 )
	{
		*p++ = HSW_SJ0 ;
		*p++ = HSW_SJ1 ;
		*p++ = HSW_SJ2 ;
	}
	if ( g_eeGeneral.ailsource & 4 )
	{
		*p++ = HSW_SK0 ;
		*p++ = HSW_SK1 ;
		*p++ = HSW_SK2 ;
	}
//	*p++ = HSW_SL0 ;
//	*p++ = HSW_SL1 ;
//	*p++ = HSW_SL2 ;
//	*p++ = HSW_SM0 ;
//	*p++ = HSW_SM1 ;
//	*p++ = HSW_SM2 ;
//	*p++ = HSW_SN0 ;
//	*p++ = HSW_SN1 ;
//	*p++ = HSW_SN2 ;
//	*p++ = HSW_SO0 ;
//	*p++ = HSW_SO1 ;
//	*p++ = HSW_SO2 ;
//	*p++ = HSW_SP0 ;
//	*p++ = HSW_SP1 ;
//	*p++ = HSW_SP2 ;
//	*p++ = HSW_SQ0 ;
//	*p++ = HSW_SQ1 ;
//	*p++ = HSW_SQ2 ;
//	*p++ = HSW_SR0 ;
//	*p++ = HSW_SR1 ;
//	*p++ = HSW_SR2 ;
#endif	// REV9E


#ifndef PCBLEM1
 #if ( defined(PCBX9LITE) && defined(X9LS) ) || defined(PCBXLITES)
	*p++ = HSW_Pb1 ;
	*p++ = HSW_Pb2 ;
 #else
	if ( g_eeGeneral.switchMapping & USE_PB1 )
	{
		*p++ = HSW_Pb1 ;
	}
	if ( g_eeGeneral.switchMapping & USE_PB2 )
	{
		*p++ = HSW_Pb2 ;
	}
 #endif
 #ifndef PCBX12D
  #ifndef PCBX10
	if ( g_eeGeneral.switchMapping & USE_PB3 )
	{
		*p++ = HSW_Pb3 ;
	}
  #endif
 #endif
#endif

#ifndef PCBX12D
 #ifndef PCBX10
	if ( g_eeGeneral.analogMapping & MASK_6POS )
 #endif
#endif
	{
		*p++ = HSW_Ele6pos0 ;
		*p++ = HSW_Ele6pos1 ;
		*p++ = HSW_Ele6pos2 ;
		*p++ = HSW_Ele6pos3 ;
		*p++ = HSW_Ele6pos4 ;
		*p++ = HSW_Ele6pos5 ;
	}
	
#if defined(PCBX12D) || defined(PCBX10)
	*p++ = HSW_Pb1 ;
	*p++ = HSW_Pb2 ;
	*p++ = HSW_Pb3 ;
	*p++ = HSW_Pb4 ;
#endif
	*p++ = HSW_Ttrmup ;
	*p++ = HSW_Ttrmdn ;
	*p++ = HSW_Rtrmup ;
	*p++ = HSW_Rtrmdn ;
	*p++ = HSW_Atrmup ;
	*p++ = HSW_Atrmdn ;
	*p++ = HSW_Etrmup ;
	*p++ = HSW_Etrmdn ;
	 
	for ( uint32_t i = 10 ; i <=33 ; i += 1  )
	{
		*p++ = i ;	// Custom switches
	}
	*p = MAX_SKYDRSWITCH ;
	MaxSwitchIndex = p - switchMapTable ;
	*++p = MAX_SKYDRSWITCH+1 ;
	*++p = MAX_SKYDRSWITCH+2 ;

	for ( uint32_t i = 0 ; i <= (uint32_t)MaxSwitchIndex+2 ; i += 1  )
	{
		switchUnMapTable[switchMapTable[i]] = i ;
	}
}

#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
void create6posTable()
{
	uint32_t i ;

	for ( i = 0 ; i < 5 ; i += 1 )
	{
		uint32_t j ;
		j = (g_eeGeneral.SixPositionCalibration[i+1] + g_eeGeneral.SixPositionCalibration[i]) / 2 ;
		if ( g_eeGeneral.SixPositionCalibration[5] >  g_eeGeneral.SixPositionCalibration[0] )
		{
			j = 4095 - j ;
		}
		SixPositionTable[i] = j ;
	}
}
#endif

#endif

int8_t switchUnMap( int8_t x )
{
	uint32_t sign = 0 ;
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
	uint32_t sign = 0 ;
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


void putsMomentDrSwitches( coord_t x, coord_t y,int8_t idx1, LcdFlags att)
{
  int16_t tm = idx1 ;
#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
	if ( tm < -HSW_MAX )
	{
		tm += 256 ;
	}
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
  if(abs(tm)>(HSW_MAX))	 //momentary on-off
#endif
#if defined(PCBSKY) || defined(PCB9XT)
  if(abs(tm)>(HSW_MAX))	 //momentary on-off
#endif
	{
#if defined(PCBX12D) || defined(PCBX10) | defined(PROP_TEXT)
  	PUTC_ATT(x+3*FW,  y, 'm', att ) ;
		x -= FW ;
#else
  	PUTC_ATT(x+3*FW,  y, 'm', att ) ;
#endif
		if ( tm > 0 )
		{
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
			tm -= HSW_MAX ;
#endif
#if defined(PCBSKY) || defined(PCB9XT)
			tm -= HSW_MAX ;
#endif
		}
	}			 
#if defined(PCBX12D) || defined(PCBX10) | defined(PROP_TEXT)
  putsDrSwitches( x+4*FW, y, tm, att|LUA_RIGHT ) ;
#else
  putsDrSwitches( x+3*FW, y, tm, att|LUA_RIGHT ) ;
#endif
}

//#ifndef ARUNI
//void putSwitchName(uint8_t x, uint8_t y, int8_t z, uint8_t att); // @menus.cpp
//#endif

//void putsDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att)//, bool nc)
//{
//	if ( idx1 == 0 )
//	{
//    lcd_putsAtt(x+FW,y,XPSTR("---"),att) ;
//		return ;
//	}
//	else if ( idx1 == MAX_SKYDRSWITCH )
//	{
//    lcd_putsAtt(x+FW,y,PSTR(STR_ON),att) ;
//		return ;
//	}
//	else if ( idx1 == -MAX_SKYDRSWITCH )
//	{
//    lcd_putsAtt(x+FW,y,PSTR(STR_OFF),att) ;
//		return ;
//	}
//	else if ( idx1 == MAX_SKYDRSWITCH + 1 )
//	{
//    lcd_putsAtt(x+FW,y,XPSTR("Fmd"),att) ;
//		return ;
//	}

//	if ( idx1 < 0 )
//	{
//  	lcd_putcAtt(x,y, '!',att);
//	}
//	int8_t z ;
//	z = idx1 ;
//	if ( z < 0 )
//	{
//		z = -idx1 ;			
//	}
//	if ( ( z <= HSW_Ttrmup ) && ( z >= HSW_Etrmdn ) )
//	{
//		z -= HSW_Etrmdn ;
//	  lcd_putsAttIdx(x+FW,y,XPSTR("\003EtuEtdAtuAtdRtuRtdTtuTtd"),z,att) ;
//		return ;
//	}
//	if ( ( z <= HSW_FM7 ) && ( z >= HSW_FM0 ) )
//	{
//		z -= HSW_FM0 ;
//  	lcd_putsAttIdx( x+FW, y, XPSTR("\003FM0FM1FM2FM3FM4FM5FM6FM7"), z, att ) ;
//		return ;
//	}
//	z -= 1 ;
//#if defined(PCBSKY) || defined(PCB9XT)
////		z *= 3 ;
//	if ( z > MAX_SKYDRSWITCH )
//	{
//		z -= HSW_OFFSET - 1 ;
//	}
//  putSwitchName(x+FW,y,z,att) ;
//#endif

//#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
////		z *= 3 ;
//	if ( z > MAX_SKYDRSWITCH )
//	{
//		z -= HSW_OFFSET - 1 ;
//	}
//  putSwitchName(x+FW,y,z,att) ;
//#endif
//}

#if defined(PCBX12D) || defined(PCBX10) || defined(TOUCH)
void putsDrSwitchesColour(uint8_t x,uint8_t y,int8_t idx1, LcdFlags att, uint16_t fcolour, uint16_t bcolour)//, bool nc)
{
	uint16_t oldBcolour = LcdBackground ;
	uint16_t oldFcolour = LcdForeground ;
	LcdBackground = bcolour ;
	LcdForeground = fcolour ;
	putsDrSwitches( x, y, idx1, att ) ;
	LcdBackground = oldBcolour ;
	LcdForeground = oldFcolour ;
}
#endif

//Type 1-trigA, 2-trigB, 0 best for display
void putsTmrMode(coord_t x, coord_t y, LcdFlags attr, uint8_t timer, uint8_t type )
{
  int8_t tm = g_model.timer[timer].tmrModeA ;
	if ( ( type == 2 ) || ( ( type == 0 ) && ( tm == 1 ) ) )
	{
		putsMomentDrSwitches( x, y, g_model.timer[timer].tmrModeB, attr ) ;
		return ;
	}
	if ( type < 2 )		// 0 or 1
	{
	  if(tm<TMR_VAROFS)
		{
        PUTS_ATT_N(  x, y, PSTR(STR_TRIGA_OPTS)+3*abs(tm),3,attr);
  	}
		else
		{
  		tm -= TMR_VAROFS - 7 ;
      PUTS_AT_IDX( x, y, get_curve_string(), tm, attr ) ;
#if defined(PCBSKY) || defined(PCB9XT)
			if ( tm < 9 + 7 )	// Allow for 7 offset above
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
			if ( tm < 9 )
#endif
			{
				x -= FW ;		
			}
  		PUTC_ATT(x+3*FW,  y,'%',attr);
		}
	}
}

const char *get_switches_string()
{
  return PSTR(SWITCHES_STR)+1	;
}	

int16_t getValue(uint8_t i)
{
  if(i<7) return calibratedStick[i];//-512..512
	if ( i >= EXTRA_POTS_START-1 )
	{
		if ( i >= EXTRA_POTS_START-1+8 )
		{
			return get_telemetry_value( i-CHOUT_BASE-NUM_SKYCHNOUT ) ;
		}
  	if( i >= EXTRA_PPM_START )
		{
			if ( i < EXTRA_PPM_START + NUM_EXTRA_PPM )
			{
				return g_ppmIns[ i + NUM_PPM - EXTRA_PPM_START ] ;
			}
			else
			{
				return ex_chans[i+NUM_SKYCHNOUT-EXTRA_CHANNELS_START] ;
			}
		}

#ifdef PCBX7
		return calibratedStick[i-EXTRA_POTS_START+7] ;
#endif
#ifdef PCBX9LITE
		return calibratedStick[i-EXTRA_POTS_START+6] ;
#endif
		return calibratedStick[i-EXTRA_POTS_START+8] ;
	}
  if(i<PPM_BASE) return 0 ;
	else if(i<CHOUT_BASE)
	{
		int16_t x ;
		x = g_ppmIns[i-PPM_BASE] ;
		if(i<PPM_BASE+4)
		{
			x -= g_eeGeneral.trainerProfile[g_model.trainerProfile].channel[i-PPM_BASE].calib ;
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
  uint32_t cs_index ;
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
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
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

  uint32_t dir = swtch>0;
  if(abs(swtch)<(MAX_SKYDRSWITCH-NUM_SKYCSW))
	{
    if(!dir) return ! keyState((enum EnumKeys)(SW_BASE-swtch-1));
    return            keyState((enum EnumKeys)(SW_BASE+swtch-1));
  }

  //use putsChnRaw
  //input -> 1..4 -> sticks,  5..8 pots
  //MAX,FULL - disregard
  //ppm
  
	ret_value = Now_switch[cs_index] & 1 ;
	
	return swtch>0 ? ret_value : !ret_value ;

}

#ifndef WIDE_SCREEN
const uint8_t speaker[] = {
4,8,0,
0x1C,0x1C,0x3E,0x7F
} ;
#endif

#if defined(PCBX12D) || defined(PCBX10)
void putsDblSizeName( coord_t x, coord_t y )
#else
void putsDblSizeName( coord_t y )
#endif
{
#if defined(PCBX12D) || defined(PCBX10)
	uint32_t length = sizeof(g_model.name)-1 ;
	while ( (g_model.name[length] == ' ' ) || (g_model.name[length] == 0 ) )
	{
		length -= 1 ;
		if ( length == 0 )
		{
			break ;
		}
	}
	length += 1 ;
	uint32_t i ;
//	uint32_t j ;
//	i = sizeof(g_model.name) - length - 1 ;
	i = getTextWidth(g_model.name, length, DBLSIZE ) ;
	i /= 2 ;
	x -= i ;
	lcdDrawSizedText( x, y, g_model.name, length, DBLSIZE, LCD_BLUE ) ;

//	for( i = 0 ; i <= length ; i += 1 )
//	{
//		lcd_putcAttColour( x + FW*2+i*2*FW-i-2, y, g_model.name[i],DBLSIZE, 0x001F, LcdBackground );
//	}

#else
	uint32_t i ;
	for( i = 0 ; i < sizeof(g_model.name) ; i += 1 )
	{
#ifdef WIDE_SCREEN
		PUTC_ATT(FW*2+i*2*FW-i-2, y, g_model.name[i],DBLSIZE);
	}
#else
		PUTC_ATT(FW*2-4+i*(2*FW-4), y, g_model.name[i],DBLSIZE|CONDENSED);
	}
#ifdef PCBLEM1
	if ( g_eeGeneral.externalRtcType )
#endif
	{
		putsTime( 105, 0, Time.hour*60+Time.minute, 0, 0 ) ;
	}
	lcd_img( 91, 8, speaker, 0, 0 ) ;
	lcd_hbar( 96, 9, 23, 6, (CurrentVolume*100+16)/23 ) ;
#endif
#endif
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
uint32_t switches_states = 0 ;
#endif

#endif
#if defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
uint32_t switches_states = 0 ;
#endif

int8_t getMovedSwitch()
{
	uint32_t skipping = 0 ;
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
	
	for ( uint32_t i=8 ; i>0 ; i-- )
	{
    uint8_t prev = (switches_states & mask) >> (i*2-2) ;
		uint8_t next = 0 ;	// Avoid compiler warning
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
     	switches_states = (switches_states & (~mask)) | (next << (i*2-2));
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
			mask = getSwitch00( HSW_Pb3 ) << 3 ;
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
			mask = getSwitch00( HSW_Pb4 ) << 4 ;
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
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
#ifdef REV9E
  for (uint32_t i=0 ; i<18 ; i += 1 )
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
			{
        result = -(1+(3*5)) ;
				if (next!=0) result = -result ;
			}
//        result = 1+(3*5)+(next!=0) ;
      else if (i==6)
        result = 1+(3*5)+1+next ;
      else if (i==7)
			{
        result = -(1+(3*5)+1+3) ;
				if (next!=0) result = -result ;
			}
//        result = 1+(3*5)+2+3+(next!=0) ;
      else
        result = 1+(3*i)-4+next ;
    }
  }
#else // REV9E
#if defined(PCBX7) || defined (PCBXLITE) || defined (PCBX9LITE) || defined(PCBLEM1)
#ifdef PCBT12
  for (uint32_t i=0 ; i<8 ; i += 1 )
#else
  for (uint32_t i=0 ; i<8 ; i += 1 )
#endif
	{
    uint16_t mask = (0x03 << (i*2)) ;
    uint8_t prev = (switches_states & mask) >> (i*2) ;
		uint8_t next = switchPosition( i ) ;

    if (prev != next)
		{
      switches_states = (switches_states & (~mask)) | (next << (i*2));
#ifdef PCBXLITE
      if (i<2)
#else
 #if defined (PCBX9LITE)
      if (i<3)
 #else
      if (i<4)
 #endif			
#endif			
			{
        result = 1+(3*i)+next;
			}
#ifdef PCBXLITE
			else if (i==2)
			{
				if (g_eeGeneral.ailsource)
				{
        	result = 1+(3*i)+next;
				}
				else
				{
        	result = 1+(3*i)+ ( next ? 1 : 0 ) ;
				}
			}
			else if (i==3)
			{
        result = 1+(3*i) ;
				if (g_eeGeneral.ailsource == 0)
				{
					result -= 1 ;
				}
				if (g_eeGeneral.rudsource)
				{
					result += next ;
				}
				else
				{
					if ( next )
					{
        		result += 1 ;
					}
				}
			}
			else
			{
        result = 0 ;
#else
 #if defined (PCBX9LITE)
			else if (i==3)
			{
				result = 0 ;
			}
#endif
			else if (i==4)
			{
				result = 0 ;
			}
      else if (i==5)
			{
 #if defined (PCBX9LITE)
        result = -(1+(3*3)) ;
#else
        result = -(1+(3*4)) ;
#endif
				if (next!=0) result = -result ;
			}
      else if (i==6)
        result = 0 ;
#ifdef PCBT12
      else if (i==7)
			{
        result = -(1+(3*4)) ;
				if (next!=0) result = -result ;
			}
#endif
      else
			{
 #if defined (PCBX9LITE)
        result = -(1+(3*3)+1) ;
#else
        result = -(1+(3*4)+1) ;
#endif
				if (next!=0) result = -result ;
#endif
			}
    }
#ifdef PCBXLITE
	if ( g_eeGeneral.pb1source )
	{
		if ( result == 0 )
		{
			mask = getSwitch00( HSW_Pb1 ) << 14 ;
			if ( ( mask ^ switches_states ) & 0x4000 )
			{
				if ( mask )
				{
					result = HSW_Pb1 ;
				}
				switches_states ^= 0x4000 ;
			}
		}
	}
	if ( g_eeGeneral.pb2source )
	{
		if ( result == 0 )
		{
			mask = getSwitch00( HSW_Pb2 ) << 15 ;
			if ( ( mask ^ switches_states ) & 0x8000 )
			{
				if ( mask )
				{
					result = HSW_Pb1 ;
				}
				switches_states ^= 0x8000 ;
			}
		}
	}
		
#endif
  }
#else // PCBX7 || PCBXLITE
  for (uint32_t i=0 ; i<8 ; i += 1 )
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
        result = -(1+(3*5)) ;
				if (next!=0) result = -result ;
			}
      else if (i==6)
        result = 1+(3*5)+1+next;
      else
			{
        result = -(1+(3*5)+1+3) ;
				if (next!=0) result = -result ;
			}
    }
  }
#if defined(PCBX12D) || defined(PCBX10)
// 6-pos switch
	uint32_t prev = (switches_states & 0x00070000) >> 16 ;
	uint32_t next = switchPosition( HSW_Ele6pos0 ) ;
	if (prev != next)
	{
		result = 21 + next ;
	}

  switches_states = (switches_states & 0xFFF8FFFF) | (next << 16 ) ;
	// Now check PB1-PB4

	if ( result == 0 )
	{
		if ( getSwitch00( HSW_Pb1 ) )
		{
			result = HSW_Pb1 ;
		}
		else if ( getSwitch00( HSW_Pb2 ) )
		{
			result = HSW_Pb2 ;
		}
		else if ( getSwitch00( HSW_Pb3 ) )
		{
			result = HSW_Pb3 ;
		}
		else if ( getSwitch00( HSW_Pb4 ) )
		{
			result = HSW_Pb4 ;
		}
		if ( result )
		{
			result = switchUnMap( result ) ;
		}
	}


#endif

#endif // PCBX7
#endif // REV9E
#endif

  if ( skipping )
    result = 0 ;

  return result ;
}

void checkQuickSelect()
{
  uint8_t i = keyDown(); //check for keystate
  uint32_t j;
  
	if ( ( i & 6 ) == 6 )
	{
		SystemOptions |= SYS_OPT_MUTE ;
 		while ( keyDown() )
		{
			wdt_reset() ;
			PUTS_ATT_LEFT( FH, XPSTR("Mute Activated") ) ;
			refreshDisplay() ;
		}
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
			protocolsToModules() ;
#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
#if defined(PCBX12D) || defined(PCBX10)
void eeLoadModel(uint8_t id) ;
#endif
	    eeLoadModel(g_eeGeneral.currModel = j);
			protocolsToModules() ;
#endif
#ifdef LUA
			luaLoadModelScripts() ;
#endif
#ifdef BASIC
			basicLoadModelScripts() ;
#endif
	    STORE_GENERALVARS;
		}
    lcd_clear();
    PUTS_ATT(64-7*FW,0*FH,PSTR(STR_LOADING),DBLSIZE);

#if defined(PCBX12D) || defined(PCBX10)
		putsDblSizeName( 0, 3*FH ) ;
#else
		putsDblSizeName( 3*FH ) ;
#endif
    refreshDisplay();
    clearKeyEvents(); // wait for user to release key
  }
}

void alertMessages( const char * s, const char * t )
{
  lcd_clear();
  PUTS_ATT(64-5*FW,0*FH,PSTR(STR_ALERT),DBLSIZE);
  PUTS_P(0,4*FH,s);
  PUTS_P(0,5*FH,t);
  PUTS_P(0,6*FH,  PSTR(STR_PRESS_KEY_SKIP) ) ;
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
		
		if ( check_power_or_usb() )
		{
			 return ;		// Usb on or power off
    }
		if(getSwitch00(g_eeGeneral.lightSw) || getSwitch00(g_model.mlightSw) || g_eeGeneral.lightAutoOff || defaults)
      {BACKLIGHT_ON;}
    else
      {BACKLIGHT_OFF;}
  }
}

void message(const char * s)
{
	almess( s, MESS_TYPE ) ;
}


#ifdef PCBX9D
void check6pos()
{
	if ( g_eeGeneral.analogMapping & MASK_6POS )
	{
		if ( g_eeGeneral.SixPositionCalibration[0] == 0 )
		{
			if ( g_eeGeneral.SixPositionCalibration[1] == 0 )
			{
				// 6POS enabled but not calibrated
    		alert(XPSTR("6 Pos switch enabled\037 But NOT calibrated"));
			}
		}
	}
}
#endif  	 

uint8_t checkThrottlePosition()
{
  uint8_t thrchn=(2-(g_eeGeneral.stickMode&1));//stickMode=0123 -> thr=2121
	int16_t v = scaleAnalog( anaIn(thrchn), thrchn ) ;
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

void checkMultiPower()
{
	uint32_t warning = 0 ;
	if ( g_model.Module[0].protocol == PROTO_MULTI )
	{
		if ( (g_model.Module[0].channels>>7) & 0x01 )
		{
			warning = 1 ;
		}
	}
	if ( g_model.Module[1].protocol == PROTO_MULTI )
	{
		if ( (g_model.Module[1].channels>>7) & 0x01 )
		{
			warning = 1 ;
		}
	}
	if ( warning )
	{
  	alert(XPSTR("Multi on LOW power"));
		voiceSystemNameNumberAudio( SV_WARNING, V_ERROR, AU_WARNING1 ) ;
	}
}


void checkTHR()
{
  if(g_eeGeneral.disableThrottleWarning) return;
  if(g_model.disableThrottleCheck) return;

#ifndef SIMU
  getADC_single();   // if thr is down - do not display warning at all
#endif

	if ( checkThrottlePosition() )
	{
		return ;
	}

  // first - display warning

	lcd_clear();
#if defined(PCBX12D) || defined(PCBX10)
  lcd_img( 1 + X12OFFSET, 0, HandImage,0,0, LCD_RED ) ;
#else
  lcd_img( 1, 0, HandImage,0,0 ) ;
#endif
  PUTS_ATT(36 + X12OFFSET,0*FH,XPSTR("THROTTLE"),DBLSIZE|CONDENSED);
  PUTS_ATT(36 + X12OFFSET,2*FH,PSTR(STR_WARNING),DBLSIZE|CONDENSED);
	PUTS_P(0 + X12OFFSET,5*FH,  PSTR(STR_THR_NOT_IDLE) ) ;
	PUTS_P(0 + X12OFFSET,6*FH,  PSTR(STR_RST_THROTTLE) ) ;
	PUTS_P(0 + X12OFFSET,7*FH,  PSTR(STR_PRESS_KEY_SKIP) ) ;
  refreshDisplay();
  clearKeyEvents();
	putSystemVoice( SV_TH_WARN, V_THR_WARN ) ;
  
	//loop until throttle stick is low
  while (1)
  {
//#if defined(PCBX12D) || defined(PCBX10)
//		PictureDrawn = 0 ;
//#endif
		 
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
	
	putSystemVoice( SV_CUSTOM_WARN, V_CUSTOM_WARN ) ;

  while (1)
  {
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
		putsChnRaw( 9*FW, 2*FH, unmapPots( pdata->source ), 0 ) ;
		PUTS_NUM( 5*FW, 3*FH, pdata->min, 0) ;
		PUTS_NUM( 11*FW, 3*FH, value, 0) ;
		PUTS_NUM( 17*FW, 3*FH, pdata->max, 0) ;
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
//PUT_HEX4( 25, 3*FH, index ) ;
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
  putSwitchName( x, 5*FH, idx, 0) ;
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
#if defined(PCBX12D) || defined(PCBX10)
	if ( switches_states & 0x10000 ) i |= 0x0080 ;
	if ( switches_states & 0x20000 ) i |= 0x2000 ;
#else
	if ( switches_states & 0x4000 ) i |= 0x0080 ;
	if ( switches_states & 0x8000 ) i |= 0x2000 ;
#endif
	return i ;
}

void checkSwitches()
{
#if defined(PCBX12D) || defined(PCBX10)
	uint32_t warningStates ;
#else
	uint16_t warningStates ;
#endif

#ifdef PCB9XT
		read_adc() ; // needed for 3/6 pos ELE switch
		processAnalogSwitches() ;
		processAnalogSwitches() ;		// Make sure the values are processed at startup.
#endif // PCB9XT
	 
	warningStates = g_model.modelswitchWarningStates ;
#if defined(PCBX12D) || defined(PCBX10)
	warningStates |= (uint32_t)(g_model.xmodelswitchWarningStates & 0x07) << 14 ;
#endif
  
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

#if defined(PCBX12D) || defined(PCBX10)
    uint32_t i = getCurrentSwitchStates() ;
		i = ( i & 0x00003FFF ) | ( ( i & 0x00070000 ) >> 2 ) ;
		i &= ~g_model.modelswitchWarningDisables ;
		if ( g_model.modelswitchWarningDisables & 0xC000 ) // 6-pos
		{
			i &= ~0x00010000 ;
		}
#else
    uint16_t i = getCurrentSwitchStates() ;
		i &= ~g_model.modelswitchWarningDisables ;
#endif

		if ( first )
		{
 			clearKeyEvents();
			first = 0 ;
			if( i != warningStates )
			{
				putSystemVoice( SV_SW_WARN, V_SW_WARN ) ;
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

  			lcd_clear();
		    lcd_img( 1, 0, HandImage,0,0 ) ;
			  PUTS_ATT(36,0*FH,PSTR(STR_SWITCH),DBLSIZE|CONDENSED);
  			PUTS_ATT(36,2*FH,PSTR(STR_WARNING),DBLSIZE|CONDENSED);
  			PUTS_P(0,7*FH,  PSTR(STR_PRESS_KEY_SKIP) ) ;

#ifdef ARUNI
        if(x & THR_WARN_MASK)
            putWarnSwitch(2 + 0*FW, 0 );
        if(x & RUD_WARN_MASK)
            putWarnSwitch(2 + 3*FW + FW/2, 1 );
        if(x & ELE_WARN_MASK)
            putWarnSwitch(2 + 7*FW, 2 );

        if(x & IDX_WARN_MASK)
        {
            if(i & SWP_ID0B)
                putWarnSwitch(2 + 10*FW + FW/2, 3 );
            if(i & SWP_ID1B)
                putWarnSwitch(2 + 10*FW + FW/2, 4 );
            if(i & SWP_ID2B)
                putWarnSwitch(2 + 10*FW + FW/2, 5 );
        }

        if(x & AIL_WARN_MASK)
            putWarnSwitch(2 + 14*FW, 6 );
        if(x & GEA_WARN_MASK)
            putWarnSwitch(2 + 17*FW + FW/2, 7 );
#else
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
#endif


#endif
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
// To Do
#ifndef SIMU
		  getADC_single() ;	// For 6-pos switch
#endif
			getMovedSwitch() ;	// loads switches_states

#if defined(PCBX12D) || defined(PCBX10)
    	uint32_t ss = switches_states ;
			ss = ( ss & 0x00003FFF ) | ( ( ss & 0x00070000 ) >> 2 ) ;
#else
    	uint16_t ss = switches_states ;
#endif
#if defined(PCBT12)
			if ( ss & 0x8000 )
			{
				ss &= ~0xF000 ;
				ss |= 0x2000 ;
			}
#endif			
#if defined(PCBLEM1)
			ss &= ~0x0F00 ;
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
#endif			
			ss &= ~g_model.modelswitchWarningDisables ;
			if ( g_model.modelswitchWarningDisables & 0xC000 ) // 6-pos
			{
				ss &= ~0x00010000 ;
			}

#ifdef PCBX7
 #ifdef PCBT12
			if ( ( ss & 0x3CFF ) == warningStates )	// Miss E and G, inc H
 #else
			if ( ( ss & 0x0CFF ) == warningStates )	// Miss E and G
 #endif
#else
 #ifdef PCBXLITE
			if ( ( ss & 0x00FF ) == (warningStates & 0x00FF ) )
 #else
  #ifdef PCBX9LITE
			if ( ( ss & 0x0C3F ) == (warningStates & 0x0C3F) )	// Miss D, E and G
  #else // X3
#if defined(PCBX12D) || defined(PCBX10)
			if ( ( ss & 0x0001FFFF ) == warningStates )
   #else
			if ( ( ss & 0x3FFF ) == warningStates )
   #endif
  #endif // X3
 #endif
#endif
			{
				return ;
			}
#if defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
// To Do
		if ( keyDown() )
		{
	    clearKeyEvents();
			return ;
		}
#endif
		lcd_clear();
#if defined(PCBX12D) || defined(PCBX10)
    lcd_img( 1 + X12OFFSET, 0, HandImage,0,0, LCD_RED ) ;
#else
    lcd_img( 1, 0, HandImage,0,0 ) ;
#endif
	  PUTS_ATT(32 + X12OFFSET,0*FH,XPSTR("SWITCH"),DBLSIZE);
	  PUTS_ATT(32 + X12OFFSET,2*FH,XPSTR("WARNING"),DBLSIZE);
		PUTS_P(0 + X12OFFSET,7*FH,  PSTR(STR_PRESS_KEY_SKIP) ) ;

		for ( uint32_t i = 0 ; i < 7 ; i += 1 )
		{
#ifdef PCBX7
#ifdef PCBT12
			if ( i == 4 )
#else
			if ( ( i == 4 ) || ( i == 6 ) )
#endif
			{
				continue ;	// Skip E and G
			}
#endif
#ifdef PCBLEM1
			if ( ( i == 4 ))// || ( i == 5 ) )
			{
				continue ;	// Skip E and G
			}
#endif
#ifdef PCBX9LITE
			if ( ( i == 3 ) || ( i == 4 ) || ( i == 6 ) )
			{
				continue ;	// Skip D, E and G
			}
#endif
#ifdef PCBXLITE
			if ( i >= 4 )
			{
				continue ;	// Skip E and G
			}
#endif
 		  uint16_t mask = ( 0x03 << (i*2) ) ;
 		  uint8_t attr = ((warningStates & mask) == (ss & mask)) ? 0 : INVERS ;
			if ( ~g_model.modelswitchWarningDisables & mask )
			{
#ifdef PCBT12
				uint32_t j ;
				j = i ;
				if ( j > 4 )
				{
					j += 1 ;
				}
  		  PUTC_ATT( 3*FW+i*(2*FW+2) + X12OFFSET, 5*FH, 'A'+j, attr ) ;
#else
 #ifdef PCBLEM1
				uint32_t j ;
				j = i ;
				if ( j > 4 )
				{
					j += 1 ;
				}
  		  PUTC_ATT( 3*FW+i*(2*FW+2) + X12OFFSET, 5*FH, 'A'+j, attr ) ;
 #else
  		  PUTC_ATT( 3*FW+i*(2*FW+2) + X12OFFSET, 5*FH, 'A'+i, attr ) ;
 #endif
#endif
				PUTC_ATT( 4*FW+i*(2*FW+2) + X12OFFSET, 5*FH, PSTR(HW_SWITCHARROW_STR)[(warningStates & mask) >> (i*2)], attr ) ;
			}
		}
#if defined(PCBX12D) || defined(PCBX10)
		if ( ~g_model.modelswitchWarningDisables & 0xC000 )
		{
		  LcdFlags attr = ((warningStates & 0x1C000) == (ss & 0x1C000)) ? 0 : INVERS ;
  		PUTC_ATT( 3*FW+7*(2*FW+2) + X12OFFSET, 5*FH, '6', attr ) ;
			PUTC_ATT( 4*FW+7*(2*FW+2) + X12OFFSET, 5*FH, ((warningStates & 0x0001C000) >> 14) + '0', attr ) ;
		}

//		PUT_HEX4( 190, 4*FH, ss >> 16 ) ;
//		PUT_HEX4( 216, 4*FH, ss ) ;
//		PUT_HEX4( 190, 5*FH, warningStates >> 16 ) ;
//		PUT_HEX4( 216, 5*FH, warningStates ) ;
//		PUT_HEX4( 190, 6*FH, switches_states >> 16 ) ;
//		PUT_HEX4( 216, 6*FH, switches_states ) ;
		
//		PUT_HEX4( 190, 7*FH, g_model.modelswitchWarningDisables ) ;
//		PUT_HEX4( 216, 7*FH, switchPosition( HSW_Ele6pos0 ) ) ;

//		PUT_HEX4( 0, 0*FH, hwKeyState( HSW_Ele6pos0 ) ) ;
//		PUT_HEX4( 0, 1*FH, hwKeyState( HSW_Ele6pos1 ) ) ;
//		PUT_HEX4( 0, 2*FH, hwKeyState( HSW_Ele6pos2 ) ) ;
//		PUT_HEX4( 0, 3*FH, hwKeyState( HSW_Ele6pos3 ) ) ;
//		PUT_HEX4( 0, 4*FH, hwKeyState( HSW_Ele6pos4 ) ) ;
//		PUT_HEX4( 0, 5*FH, hwKeyState( HSW_Ele6pos5 ) ) ;




#endif

#endif
    refreshDisplay() ;


    wdt_reset();

		if ( check_power_or_usb() ) return ;		// Usb on or power off

		check_backlight() ;

#ifdef SIMU
    if (!main_thread_running) return;
    sleep(1/*ms*/);
#endif
		CoTickDelay(5) ;	// 10mS
  }
}

#ifdef ARUNI
void displayChangeWarning(const char *s)
{
  lcd_img( 1, 0, HandImage,0,0 ) ;
  PUTS_ATT(36,1*FH,PSTR(STR_WARNING),DBLSIZE|CONDENSED);
	PUTS_P(0,4*FH,  s ) ;
	PUTS_P((strlen(s)+1)*FW, 4*FH, PSTR(STR_CHANGE_MAY) ) ;
	PUTS_P(0,5*FH,  PSTR(STR_BRICK_RADIO) ) ;
	PUTS_P(0,7*FH,  PSTR(STR_MENU_PROCEED) ) ;
}
#endif

#ifdef SLAVE_RESET
void panicDebugMenu()
{
	popMenu(true) ; //return to uppermost, beeps itself
	pushMenu(menuProcPanic) ;
}
#endif


MenuFuncP lastPopMenu()
{
  return  g_menuStack[g_menuStackPtr+1];
}

extern union t_sharedMemory SharedMemory ;

void leavingMenu()
{
	if ( SharedMemory.TextControl.TextFileOpen )
	{
		f_close( &SharedMemory.TextControl.TextFile ) ;
		SharedMemory.TextControl.TextFileOpen = 0 ;
	}
}

void popMenu(bool uppermost)
{
	leavingMenu() ;
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
	leavingMenu() ;
  g_menuStack[g_menuStackPtr] = newMenu;
	EnterMenu = EVT_ENTRY ;
}

void pushMenu(MenuFuncP newMenu)
{
	leavingMenu() ;
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

int8_t REG100_100(int8_t x)
{
//#if MULTI_GVARS
//	if ( g_model.flightModeGvars )
//	{
//  	int8_t result = x;
//		if ( x > 100 )
//		{
//    	result = getGvar(x-113) ;
//			if ( result > 100 )
//			{
//				result = 100 ;
//			}
//			if ( result < -100 )
//			{
//				result = -100 ;
//			}
//		}
//		return result ;
//	}
//#endif
	return REG( x, -100, 100 ) ;
}

int8_t REG(int8_t x, int8_t min, int8_t max)
{
  int8_t result = x ;
  if (x >= 126 || x <= -126)
	{
    x = (uint8_t)x - 126 ;
    result = getGvar(x) ;
    if (result < min)
		{
			setGVar( x, (result = min) ) ;
    }
    if (result > max)
		{
			setGVar( x, (result = max) ) ;
    }
  }
  return result ;
}

uint8_t REGisGvar( int8_t x )
{
  return (x >= 126 || x <= -126) ;	
}

uint8_t IS_EXPO_THROTTLE( uint8_t x )
{
	if ( g_model.thrExpo )
	{
		return IS_THROTTLE( x ) ;
	}
	return 0 ;
}

//void checkXyCurve()
//{
//	uint32_t a ;
//	uint32_t b ;
//	a = ( g_model.curvexy[9] == 0 ) ;
//	b = ( g_model.curve2xy[9] == 0 ) ;

//	if ( a | b )
//	{
//		uint32_t i ;
//		int8_t j = -100 ;
//		for ( i = 9 ; i < 18 ; j += 25, i += 1 )
//		{
//			if ( a )
//			{
//				g_model.curvexy[i] = j ;
//			}
//			if ( b )
//			{
//				g_model.curve2xy[i] = j ;
//			}
//		}
//	}
////	if ( g_model.curve2xy[9] == 0 )
////	{
////		uint32_t i ;
////		int8_t j = -100 ;
////		for ( i = 9 ; i < 18 ; j += 25, i += 1 )
////		{
////		}
////	}
//}

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
	if ( g_model.curve2xy[9] == 0 )
	{
		uint32_t i ;
		int8_t j = -100 ;
		for ( i = 9 ; i < 18 ; j += 25, i += 1 )
		{
			g_model.curve2xy[i] = j ;
		}
	}
}

void protocolsToModules()
{
	if ( g_model.modelVersion < 4 )
	{
	
#ifndef PCBLEM1
		int8_t temp ;
#endif
#ifdef PCBSKY
		g_model.Module[1].protocol = g_model.protocol ;
		g_model.Module[1].country = g_model.country ;
		g_model.Module[1].ppmOpenDrain = g_model.ppmOpenDrain ;
		g_model.Module[1].pulsePol = g_model.pulsePol ;
		if ( g_model.Module[1].protocol == PROTO_PPM )
		{
			temp = (g_model.ppmNCH) * 2 ;
			if ( temp > 8 )
			{
				temp -= 13 ;
			}
		}
		else
		{
			temp = g_model.ppmNCH ;
		}
		g_model.Module[1].channels = temp ;
		g_model.Module[1].startChannel = g_model.startChannel ;
		g_model.Module[1].sub_protocol = g_model.sub_protocol ;
		g_model.Module[1].pxxRxNum = g_model.pxxRxNum ;
		g_model.Module[1].ppmDelay = g_model.ppmDelay ;
		g_model.Module[1].ppmFrameLength = g_model.ppmFrameLength ;
		g_model.Module[1].option_protocol = g_model.option_protocol ;
		g_model.Module[1].failsafeMode = g_model.failsafeMode[0] ;
		for ( temp = 0 ; temp < 16 ; temp += 1 )
		{
			uint32_t a = temp ;
			uint32_t b = temp / 8 ;
			if ( b )
			{
				a -= 8 ;
			}
			g_model.Module[1].failsafe[temp] = g_model.accessFailsafe[b][a] ;
		}
	 
		g_model.Module[0].protocol = g_model.xprotocol ;
		g_model.Module[0].country = g_model.xcountry ;
		g_model.Module[0].pulsePol = g_model.xpulsePol ;
		if ( g_model.Module[0].protocol == PROTO_PPM )
		{
			temp = (g_model.ppm2NCH) * 2 ;
			if ( temp > 8 )
			{
				temp -= 13 ;
			}
		}
		else
		{
			temp = g_model.xppmNCH ;
		}
		g_model.Module[0].channels = temp ;
		g_model.Module[0].startChannel = g_model.xstartChannel ;
		temp = g_model.startPPM2channel ;
		if ( temp )
		{
			temp -= 1 ;
		}
		g_model.Module[0].startChannel = temp ;
		g_model.Module[0].sub_protocol = g_model.xsub_protocol ;
		g_model.Module[0].pxxRxNum = g_model.xPxxRxNum ;
		g_model.Module[0].ppmDelay = g_model.xppmDelay ;
		g_model.Module[0].ppmFrameLength = g_model.xppmFrameLength ;
		g_model.Module[0].option_protocol = g_model.xoption_protocol ;
		g_model.Module[0].failsafeMode = g_model.failsafeMode[1] ;
#endif
#if defined(PCB9XT) || defined(PCBX9D) || defined(PCBX12D) || defined(PCBX10)
		g_model.Module[0].protocol = g_model.protocol ;
		g_model.Module[0].country = g_model.country ;
		g_model.Module[0].pulsePol = g_model.pulsePol ;
		temp = g_model.ppmNCH ;
		if ( g_model.Module[0].protocol == PROTO_PPM )
		{
			temp *= 2 ;
			if ( temp > 8 )
			{
				temp -= 13 ;
			}
		}
		g_model.Module[0].channels = temp ;
		g_model.Module[0].startChannel = g_model.startChannel ;
		g_model.Module[0].sub_protocol = g_model.sub_protocol ;
		g_model.Module[0].pxxRxNum = g_model.pxxRxNum ;
		g_model.Module[0].ppmDelay = g_model.ppmDelay ;
		g_model.Module[0].ppmFrameLength = g_model.ppmFrameLength ;
		g_model.Module[0].option_protocol = g_model.option_protocol ;
		g_model.Module[0].failsafeMode = g_model.failsafeMode[0] ;
		for ( temp = 0 ; temp < 16 ; temp += 1 )
		{
			uint32_t a = temp ;
			uint32_t b = temp / 8 ;
			if ( b )
			{
				a -= 8 ;
			}
			g_model.Module[0].failsafe[temp] = g_model.accessFailsafe[b][a] ;
		}
	
		g_model.Module[1].protocol = g_model.xprotocol ;
		g_model.Module[1].country = g_model.xcountry ;
		g_model.Module[1].pulsePol = g_model.xpulsePol ;
		temp = g_model.xppmNCH ;
		if ( g_model.Module[1].protocol == PROTO_PPM )
		{
			temp *= 2 ;
			if ( temp > 8 )
			{
				temp -= 13 ;
			}
		}
		g_model.Module[1].channels = temp ;
		g_model.Module[1].startChannel = g_model.xstartChannel ;
		g_model.Module[1].sub_protocol = g_model.xsub_protocol ;
		g_model.Module[1].pxxRxNum = g_model.xPxxRxNum ;
		g_model.Module[1].ppmDelay = g_model.xppmDelay ;
		g_model.Module[1].ppmFrameLength = g_model.xppmFrameLength ;
		g_model.Module[1].option_protocol = g_model.xoption_protocol ;
		g_model.Module[1].failsafeMode = g_model.failsafeMode[1] ;
		for ( temp = 0 ; temp < 16 ; temp += 1 )
		{
			uint32_t a = temp ;
			uint32_t b = temp / 8 ;
			if ( b )
			{
				a -= 8 ;
			}
			g_model.Module[1].failsafe[temp] = g_model.accessFailsafe[b][a] ;
		}
#endif
		g_model.modelVersion = 4 ;
		STORE_MODELVARS ;
	}
}

/*** EOF ***/
 
