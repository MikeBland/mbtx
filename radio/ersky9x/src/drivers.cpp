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



#include <stdint.h>
#include <stdlib.h>

#ifdef PCBSKY
 #ifndef PCBDUE
  #include "AT91SAM3S4.h"
 #else
	#include "sam3x8e.h"
 #endif
#ifndef SIMU
#include "core_cm3.h"
#endif
#endif

#include "ersky9x.h"
#include "myeeprom.h"
#include "drivers.h"
#include "logicio.h"
#include "lcd.h"
#include "debug.h"
#include "frsky.h"
#ifndef SIMU
#include "CoOS.h"
#endif

#if defined(PCBX9D) || defined(PCB9XT)
#include "diskio.h"
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/stm32f2xx_rcc.h"
#include "X9D/stm32f2xx_usart.h"
#include "X9D/hal.h"
#include "timers.h"
#endif


#define SERIAL_TRAINER	1


// Timer usage
// TIMER3 for input capture
// Timer4 to provide 0.5uS clock for input capture
// TIMER0 at full speed (MCK/2) for delay timing
// TIMER2 at 200Hz, provides 5mS for sound and 10mS tick on interrupt
// Timer1 used for DAC output timing
// Timer5 is currently UNUSED

uint16_t Scc_baudrate ;				// 0 for 125000, 1 for 115200

#ifdef PCBSKY
#ifdef REVX
volatile uint16_t Analog_values[NUMBER_ANALOG] ;
#else
volatile uint16_t Analog_values[NUMBER_ANALOG] ;
#endif
#endif
uint16_t Temperature ;				// Raw temp reading
uint16_t Max_temperature ;		// Max raw temp reading
uint16_t DsmRxTimeout ;
uint16_t WatchdogTimeout ;

#define RX_UART_BUFFER_SIZE	128

struct t_rxUartBuffer
{
	uint8_t fifo[RX_UART_BUFFER_SIZE] ;
	uint8_t *outPtr ;
} ;

uint8_t JetiTxBuffer[16] ;

struct t_rxUartBuffer TelemetryInBuffer ;

struct t_fifo128 Console_fifo ;
struct t_fifo128 Com3_fifo ;
struct t_fifo128 BtRx_fifo ;

struct t_fifo64 CaptureRx_fifo ;
struct t_fifo64 RemoteRx_fifo ;

struct t_16bit_fifo32 Jeti_fifo ;

struct t_fifo64 Sbus_fifo ;

#if defined(PCBX9D) || defined(PCB9XT)
struct t_fifo64 Telemetry_fifo ;
struct t_SportTx
{
	uint8_t *ptr ;
	uint16_t count ;
	uint8_t busy ;
} SportTx ;
#endif

struct t_serial_tx *Current_Com2 ;
#ifdef PCB9XT
struct t_serial_tx *Current_Com3 ;
#endif

volatile uint32_t Spi_complete ;

void putEvent( register uint8_t evt) ;
void per10ms( void ) ;
uint8_t getEvent( void ) ;
void pauseEvents(uint8_t event) ;
void killEvents(uint8_t event) ;

void UART_Configure( uint32_t baudrate, uint32_t masterClock) ;
void txmit( uint8_t c ) ;
uint16_t rxuart( void ) ;
void UART3_Configure( uint32_t baudrate, uint32_t masterClock) ;
void txmitBt( uint8_t c ) ;
int32_t rxBtuart( void ) ;

uint32_t keyState( enum EnumKeys enuk) ;
void init_spi( void ) ;
//void end_spi( void ) ;
uint32_t eeprom_read_status( void ) ;
uint32_t  eeprom_write_one( uint8_t byte, uint8_t count ) ;
void eeprom_write_enable( void ) ;
uint32_t spi_operation( uint8_t *tx, uint8_t *rx, uint32_t count ) ;
//uint32_t spi_action( uint8_t *command, uint8_t *tx, uint8_t *rx, uint32_t comlen, uint32_t count ) ;
uint32_t spi_PDC_action( uint8_t *command, uint8_t *tx, uint8_t *rx, uint32_t comlen, uint32_t count ) ;

void read_adc(void ) ;
void init_adc( void ) ;
void init_ssc( uint16_t baudrate ) ;
void disable_ssc( void ) ;

/** Usart Hw interface used by the console (UART0). */
#define CONSOLE_USART       UART0
/** Usart Hw ID used by the console (UART0). */
#define CONSOLE_ID          ID_UART0
/** Pins description corresponding to Rxd,Txd, (UART pins) */
#define CONSOLE_PINS        {PINS_UART}

/** Second serial baudrate 9600. */
#define SECOND_BAUDRATE    9600
/** Usart Hw interface used by the console (UART0). */
#define SECOND_USART       USART0
/** Usart Hw ID used by the console (UART0). */
#define SECOND_ID          ID_USART0
/** Pins description corresponding to Rxd,Txd, (UART pins) */
#define SECOND_PINS        {PINS_USART0}

#define BT_USART       UART1
#define BT_ID          ID_UART1

static uint8_t s_evt;
void putEvent( register uint8_t evt)
{
  s_evt = evt;
}

uint8_t menuPressed()
{
	if ( keys[KEY_MENU].isKilled() )
	{
		return 0 ;
	}
	return ( read_keys() & 2 ) == 0 ;
}

uint8_t getEvent()
{
  register uint8_t evt = s_evt;
  s_evt=0;
  return evt;
}

Key keys[NUM_KEYS] ;

void Key::input(bool val, EnumKeys enuk)
{
  //  uint8_t old=m_vals;
  m_vals <<= 1;  if(val) m_vals |= 1; //portbit einschieben
  m_cnt++;

  if(m_state && m_vals==0){  //gerade eben sprung auf 0
    if(m_state!=KSTATE_KILLED) {
      putEvent(EVT_KEY_BREAK(enuk));
      if(!( m_state == 16 && m_cnt<16)){
        m_dblcnt=0;
      }
        //      }
    }
    m_cnt   = 0;
    m_state = KSTATE_OFF;
  }
  switch(m_state){
    case KSTATE_OFF:
      if(m_vals==FFVAL){ //gerade eben sprung auf ff
        m_state = KSTATE_START;
        if(m_cnt>16) m_dblcnt=0; //pause zu lang fuer double
        m_cnt   = 0;
      }
      break;
      //fallthrough
    case KSTATE_START:
      putEvent(EVT_KEY_FIRST(enuk));
      m_dblcnt++;
#ifdef KSTATE_RPTDELAY
      m_state   = KSTATE_RPTDELAY;
#else
      m_state   = 16;
#endif
      m_cnt     = 0;
      break;
#ifdef KSTATE_RPTDELAY
    case KSTATE_RPTDELAY: // gruvin: longer delay before first key repeat
      if(m_cnt == 32) putEvent(EVT_KEY_LONG(enuk)); // need to catch this inside RPTDELAY time
      if (m_cnt == 40) {
        m_state = 16;
        m_cnt = 0;
      }
      break;
#endif
    case 16:
#ifndef KSTATE_RPTDELAY
      if(m_cnt == 32) putEvent(EVT_KEY_LONG(enuk));
      //fallthrough
#endif
    case 8:
    case 4:
    case 2:
      if(m_cnt >= 48)  { //3 6 12 24 48 pulses in every 480ms
        m_state >>= 1;
        m_cnt     = 0;
      }
      //fallthrough
    case 1:
      if( (m_cnt & (m_state-1)) == 0)  putEvent(EVT_KEY_REPT(enuk));
      break;

    case KSTATE_PAUSE: //pause
      if(m_cnt >= 64)      {
        m_state = 8;
        m_cnt   = 0;
      }
      break;

    case KSTATE_KILLED: //killed
      break;
  }
}

void pauseEvents(uint8_t event)
{
  event=event & EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].pauseEvents();
}

void killEvents(uint8_t event)
{
  event=event & EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].killEvents();
}



volatile uint8_t  g_blinkTmr10ms;

volatile uint16_t g_tmr10ms;
extern uint8_t StickScrollTimer ;

#ifdef PCBSKY
struct t_serial_tx FmsTx ;
uint8_t FmsTxBuffer[12] ;

void doFms()
{
	uint32_t i ;
	if ( g_tmr10ms & 1 )
	{
		// Send a Fms packet
		FmsTxBuffer[0] = 0xFF ;
		for ( i = 0 ; i < 8 ; i += 1 )
		{
			int32_t x = g_chans512[i] ;
			x /= 12 ;
			x += 128 ;
		FmsTxBuffer[i+1] = x ;
		}
		FmsTx.size = 9 ;		
		FmsTx.buffer = FmsTxBuffer ;
		txPdcCom2( &FmsTx ) ;
	}
}

#endif



void per10ms()
{
	register uint32_t i ;

  g_tmr10ms++;
  if (WatchdogTimeout)
	{
  	if (WatchdogTimeout>200)
		{
  		WatchdogTimeout = 200 ;			
		}
    WatchdogTimeout -= 1;
    wdt_reset();  // Retrigger hardware watchdog
  }

  g_blinkTmr10ms++;
  uint8_t enuk = KEY_MENU;
  uint8_t    in = ~read_keys() ;
	// Bits 3-6 are down, up, right and left
	// Try to only allow one at a 
#ifdef REVX
	static uint8_t current ;
	uint8_t dir_keys ;
	uint8_t lcurrent ;

	dir_keys = in & 0x78 ;		// Mask to direction keys
	if ( ( lcurrent = current ) )
	{ // Something already pressed
		if ( ( lcurrent & dir_keys ) == 0 )
		{
			lcurrent = 0 ;	// No longer pressed
		}
		else
		{
			in &= lcurrent | 0x06 ;	// current or MENU or EXIT allowed
		}
	}
	if ( lcurrent == 0 )
	{ // look for a key
		if ( dir_keys & 0x20 )	// right
		{
			lcurrent = 0x60 ;		// Allow L and R for 9X
		}
		else if ( dir_keys & 0x40 )	// left
		{
			lcurrent = 0x60 ;		// Allow L and R for 9X
		}
		else if ( dir_keys & 0x08 )	// down
		{
			lcurrent = 0x08 ;
		}
		else if ( dir_keys & 0x10 )	// up
		{
			lcurrent = 0x10 ;
		}
		in &= lcurrent | 0x06 ;	// current or MENU or EXIT allowed
	}
	current = lcurrent ;
#endif

  for( i=1; i<7; i++)
  {
		uint8_t value = in & (1<<i) ;
//#ifdef PCBSKY
#if !defined(SIMU)
		if ( value )
		{
			StickScrollTimer = STICK_SCROLL_TIMEOUT ;
		}
#endif
//#endif // PCBSKY
    //INP_B_KEY_MEN 1  .. INP_B_KEY_LFT 6
    keys[enuk].input(value,(EnumKeys)enuk);
    ++enuk;
  }

//  static const uint8_t crossTrim[]={
//    1<<INP_D_TRM_LH_DWN,
//    1<<INP_D_TRM_LH_UP,
//    1<<INP_D_TRM_LV_DWN,
//    1<<INP_D_TRM_LV_UP,
//    1<<INP_D_TRM_RV_DWN,
//    1<<INP_D_TRM_RV_UP,
//    1<<INP_D_TRM_RH_DWN,
//    1<<INP_D_TRM_RH_UP
//  };


	in = read_trims() ;

	for( i=1; i<256; i<<=1)
  {
    // INP_D_TRM_RH_UP   0 .. INP_D_TRM_LH_UP   7
    keys[enuk].input(in & i,(EnumKeys)enuk);
    ++enuk;
  }

#ifdef PCBSKY
#if !defined(SIMU)
	uint8_t value = ~PIOB->PIO_PDSR & 0x40 ;

extern uint8_t AnaEncSw ;
	value |= AnaEncSw ;
	keys[enuk].input( value,(EnumKeys)enuk); // Rotary Enc. Switch
	if ( value )
	{
		StickScrollTimer = STICK_SCROLL_TIMEOUT ;
	}
#endif

#endif

#ifdef PCB9XT
extern uint16_t M64Switches ;
extern uint8_t EncoderI2cData[] ;
	uint8_t value = (M64Switches & 0x0200) ? 1 : 0 ;
	if ( value == 0 )
	{
		value = EncoderI2cData[1] ? 1 : 0 ;
	}
	keys[enuk].input( value,(EnumKeys)enuk); // Rotary Enc. Switch
	if ( value )
	{
		StickScrollTimer = STICK_SCROLL_TIMEOUT ;
	}
#endif

#ifdef PCBX9D
#if !defined(SIMU)
#ifdef REV9E
	uint8_t value = ~GPIOF->IDR & PIN_BUTTON_ENCODER ;
#else
extern uint8_t AnaEncSw ;
	uint8_t value = AnaEncSw ;
#endif // REV9E
	keys[enuk].input( value,(EnumKeys)enuk); // Rotary Enc. Switch
	if ( value )
	{
		StickScrollTimer = STICK_SCROLL_TIMEOUT ;
	}
#endif
#endif

#if defined(PCBX9D) || defined(PCB9XT)
	sdPoll10mS() ;
#endif

#ifdef PCBSKY
	if ( g_model.com2Function == COM2_FUNC_FMS )
	{
		doFms() ;
	}
#endif

}


//void put_fifo32( struct t_fifo32 *pfifo, uint8_t byte )
//{
//	pfifo->fifo[pfifo->in] = byte ;
//#ifndef SIMU
//	__disable_irq() ;
//#endif
//	pfifo->count += 1 ;
//#ifndef SIMU
//		__enable_irq() ;
//#endif
//	pfifo->in = ( pfifo->in + 1) & 0x1F ;
//}

//int32_t get_fifo32( struct t_fifo32 *pfifo )
//{
//	int32_t rxbyte ;
//	if ( pfifo->count )						// Look for char available
//	{
//		rxbyte = pfifo->fifo[pfifo->out] ;
//#ifndef SIMU
//	__disable_irq() ;
//#endif
//		pfifo->count -= 1 ;
//#ifndef SIMU
//		__enable_irq() ;
//#endif
//		pfifo->out = ( pfifo->out + 1 ) & 0x1F ;
//		return rxbyte ;
//	}
//	return -1 ;
//}

void put_fifo64( struct t_fifo64 *pfifo, uint8_t byte )
{
  uint32_t next = (pfifo->in + 1) & 0x3f;
	if ( next != pfifo->out )
	{
		pfifo->fifo[pfifo->in] = byte ;
		pfifo->in = next ;
	}
}

int32_t get_fifo64( struct t_fifo64 *pfifo )
{
	int32_t rxbyte ;
	if ( pfifo->in != pfifo->out )				// Look for char available
	{
		rxbyte = pfifo->fifo[pfifo->out] ;
		pfifo->out = ( pfifo->out + 1 ) & 0x3F ;
		return rxbyte ;
	}
	return -1 ;
}

void put_fifo128( struct t_fifo128 *pfifo, uint8_t byte )
{
  uint32_t next = (pfifo->in + 1) & 0x7f;
	if ( next != pfifo->out )
	{
		pfifo->fifo[pfifo->in] = byte ;
		pfifo->in = next ;
	}
}

int32_t get_fifo128( struct t_fifo128 *pfifo )
{
	int32_t rxbyte ;
	if ( pfifo->in != pfifo->out )				// Look for char available
	{
		rxbyte = pfifo->fifo[pfifo->out] ;
		pfifo->out = ( pfifo->out + 1 ) & 0x7F ;
		return rxbyte ;
	}
	return -1 ;
}

//void put_fifo64( struct t_fifo64 *pfifo, uint8_t byte )
//{
//	pfifo->fifo[pfifo->in] = byte ;
//#ifndef SIMU
//	__disable_irq() ;
//#endif
//	pfifo->count += 1 ;
//#ifndef SIMU
//		__enable_irq() ;
//#endif
//	pfifo->in = ( pfifo->in + 1) & 0x3F ;
//}

//int32_t get_fifo64( struct t_fifo64 *pfifo )
//{
//	int32_t rxbyte ;
//	if ( pfifo->count )						// Look for char available
//	{
//		rxbyte = pfifo->fifo[pfifo->out] ;
//#ifndef SIMU
//	__disable_irq() ;
//#endif
//		pfifo->count -= 1 ;
//#ifndef SIMU
//		__enable_irq() ;
//#endif
//		pfifo->out = ( pfifo->out + 1 ) & 0x3F ;
//		return rxbyte ;
//	}
//	return -1 ;
//}


void put_16bit_fifo32( struct t_16bit_fifo32 *pfifo, uint16_t word )
{
  uint32_t next = (pfifo->in + 1) & 0x1f;
	if ( next != pfifo->out )
	{
		pfifo->fifo[pfifo->in] = word ;
		pfifo->in = next ;
	}
}

int32_t get_16bit_fifo32( struct t_16bit_fifo32 *pfifo )
{
	int32_t rxbyte ;
	if ( pfifo->in != pfifo->out )				// Look for char available
	{
		rxbyte = pfifo->fifo[pfifo->out] ;
		pfifo->out = ( pfifo->out + 1 ) & 0x1F ;
		return rxbyte ;
	}
	return -1 ;
}


//void put_frsky_fifo( uint8_t c )
//{
//	FrskyFifo.fifo[FrskyFifo.in] = c ;
//	FrskyFifo.count++ ;
//	FrskyFifo.in = (FrskyFifo.in + 1) & 0x1F ;
//}

//int32_t get_frsky_fifo()
//{
//	uint32_t rxchar ;

//	if (FrskyFifo.count )						// Look for char available
//	{
//		rxchar = FrskyFifo.fifo[FrskyFifo.out] ;
//		__disable_irq() ;
//		FrskyFifo.count-- ;						// Protect from interrupts
//		__enable_irq() ;
//		FrskyFifo.out = ( FrskyFifo.out + 1 ) & 0x1F ;
//		return rxchar ;
//	}
//	return -1 ;
//}

#ifdef SERIAL_TRAINER
// 9600 baud, bit time 104.16uS
#define BIT_TIME_9600		208
// 100000 baud, bit time 10uS (SBUS)
#define BIT_TIME_100K		20
// 115200 baud, bit time 8.68uS
#define BIT_TIME_115K		17

// States in LineState
#define LINE_IDLE			0
#define LINE_ACTIVE		1

// States in BitState
#define BIT_IDLE			0
#define BIT_ACTIVE		1
#define BIT_FRAMING		2

uint8_t LineState ;
uint8_t CaptureMode ;
uint16_t BitTime ;
uint16_t HtoLtime ;
uint16_t LtoHtime ;
uint16_t Byte ;
uint8_t SoftSerInvert = 0 ;
uint8_t BitState ;
uint8_t BitCount ;
uint8_t Tc5Count ;
uint8_t SoftSerialEvenParity ;

uint16_t USART_ERRORS ;
uint16_t USART_ORE ;
uint16_t USART_NE ;
uint16_t USART_FE ;
uint16_t USART_PE ;

//uint16_t SerTimes[64] ;
//uint16_t SerValues[64] ;
//uint16_t SerBitStates[64] ;
//uint16_t SerBitCounts[64] ;
//uint16_t SerBitInputs[64] ;
//uint16_t SaveSerTimes[64] ;

//uint32_t SerIndex ;

//uint16_t SoftCounter ;
//uint16_t ParityByte ;
//uint16_t SaveTime ;
//uint16_t ParityTime ;
//uint16_t SaveHtoL ;
//uint16_t SaveLtoH ;
//uint16_t SaveTIM11 ;
//uint16_t SaveTIM7 ;
//uint16_t SaveValue ;
//uint16_t SaveCCR1 ;
//uint16_t TIM11Count ;
//uint16_t LastHtoL ;
//uint16_t LastLtoH ;
//uint16_t SaveLastHtoL ;
//uint16_t SaveLastLtoH ;

//uint16_t SaveSerIndex ;

// time in units of 0.5uS, value is 1 or 0
void putCaptureTime( uint16_t time, uint32_t value )
{
	time += BitTime/2 ;
	time /= BitTime ;		// Now number of bits
//	SaveTime = time ;

	if ( value == 3 )
	{
		return ;
	}

	if ( BitState == BIT_IDLE )
	{ // Starting, value should be 0
		BitState = BIT_ACTIVE ;
		BitCount = 0 ;
		if ( time > 1 )
		{
			Byte >>= time-1 ;
			BitCount = time-1 ;
		}
	}
	else
	{
		if ( value )
		{
			uint32_t len = SoftSerialEvenParity ? 9 : 8 ;
			while ( time )
			{
				if ( BitCount >= len )
				{ // Got a byte
					if ( len == 9 )
					{
						// check parity (even)
						uint32_t parity = Byte ;
						parity ^= parity >> 4 ;
						parity ^= parity >> 2 ;
						parity ^= parity >> 1 ;
						parity ^= Byte >> 8 ;
						if ( ( parity & 1 ) == 0 )
						{
//							SoftCounter += 1 ;
							put_fifo64( &CaptureRx_fifo, Byte ) ;
						}
						else
						{
//							ParityTime = SaveTime ;
//							ParityByte = Byte ;
							USART_PE += 1 ;
//							SaveHtoL = HtoLtime ;
//							SaveLtoH = LtoHtime ;
//							SaveTIM11 = TIM11->CNT ;
//							SaveTIM7 = TIM7->CNT ;
//							SaveValue = value ;
//							SaveCCR1 = TIM11->CCR1 ;
//							SaveLastHtoL = LastHtoL ;
//							SaveLastLtoH = LastLtoH ;
//							SaveSerTimes[0] = SerTimes[0] ;
//							SaveSerTimes[1] = SerTimes[1] ;
//							SaveSerTimes[2] = SerTimes[2] ;
//							SaveSerTimes[3] = SerTimes[3] ;
//							SaveSerTimes[4] = SerTimes[4] ;
//							SaveSerTimes[5] = SerTimes[5] ;
//							SaveSerTimes[6] = SerTimes[6] ;
//							SaveSerTimes[7] = SerTimes[7] ;
//							SaveSerTimes[8] = SerTimes[8] ;
//							SaveSerTimes[9] = SerTimes[9] ;
//							SaveSerIndex = SerIndex ;
						}
					}
					else
					{
						put_fifo64( &CaptureRx_fifo, Byte ) ;
					}
					BitState = BIT_IDLE ;
					break ;
				}
				else
				{
					Byte >>= 1 ;
					Byte |= ( len == 9 ) ? 0x100 : 0x80 ;
					time -= 1 ;
					BitCount += 1 ;
				}
			}
		}
		else
		{
			Byte >>= time ;
			BitCount += time ;
		}
	}
}

#endif


#ifdef PCBSKY
// SPI i/f to EEPROM (4Mb)
// Peripheral ID 21 (0x00200000)
// Connections:
// SS   PA11 (peripheral A)
// MISO PA12 (peripheral A)
// MOSI PA13 (peripheral A)
// SCK  PA14 (peripheral A)
// Set clock to 3 MHz, AT25 device is rated to 70MHz, 18MHz would be better
void init_spi()
{
//	register Pio *pioptr ;
	register Spi *spiptr ;
	register uint32_t timer ;
	register uint8_t *p ;
	uint8_t spi_buf[4] ;

  PMC->PMC_PCER0 |= 0x00200000L ;		// Enable peripheral clock to SPI
  /* Configure PIO */
	configure_pins( 0x00007800, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;
//	pioptr = PIOA ;
//  pioptr->PIO_ABCDSR[0] &= ~0x00007800 ;	// Peripheral A bits 14,13,12,11
//  pioptr->PIO_ABCDSR[1] &= ~0x00007800 ;	// Peripheral A
//  pioptr->PIO_PDR = 0x00007800 ;					// Assign to peripheral
	
	spiptr = SPI ;
	timer = ( Master_frequency / 3000000 ) << 8 ;		// Baud rate 3Mb/s
	spiptr->SPI_MR = 0x14000011 ;				// 0001 0100 0000 0000 0000 0000 0001 0001 Master
	spiptr->SPI_CSR[0] = 0x01180009 | timer ;		// 0000 0001 0001 1000 xxxx xxxx 0000 1001
	NVIC_SetPriority( SPI_IRQn, 5 ) ; // Lower priority interrupt
	NVIC_EnableIRQ(SPI_IRQn) ;

	p = spi_buf ;
		
//	*p = 0x39 ;		// Unprotect sector command
//	*(p+1) = 0 ;
//	*(p+2) = 0 ;
//	*(p+3) = 0 ;		// 3 bytes address

//	spi_operation( p, spi_buf, 4 ) ;
	
	eeprom_write_enable() ;

	*p = 1 ;		// Write status register command
	*(p+1) = 0 ;
	spi_operation( p, spi_buf, 2 ) ;

}


//void end_spi()
//{
//	SPI->SPI_CR = 2 ;								// Disable
//	SPI->SPI_IDR = 0x07FF ;					// All interrupts off
//	NVIC_DisableIRQ(SPI_IRQn) ;
//}

extern "C" void SPI_IRQHandler()
{
	register Spi *spiptr ;

	spiptr = SPI ;
	SPI->SPI_IDR = 0x07FF ;			// All interrupts off
	spiptr->SPI_CR = 2 ;				// Disable
	(void) spiptr->SPI_RDR ;		// Dump any rx data
	(void) spiptr->SPI_SR ;			// Clear error flags
	spiptr->SPI_PTCR = SPI_PTCR_RXTDIS | SPI_PTCR_TXTDIS ;	// Stop transfers
	Spi_complete = 1 ;					// Indicate completion

// Power save
//  PMC->PMC_PCER0 &= ~0x00200000L ;		// Disable peripheral clock to SPI
	
}


void eeprom_write_enable()
{
	eeprom_write_one( 6, 0 ) ;
}

uint32_t eeprom_read_status()
{
	return eeprom_write_one( 5, 1 ) ;
}

uint32_t  eeprom_write_one( uint8_t byte, uint8_t count )
{
	register Spi *spiptr ;
	register uint32_t result ;
	
	spiptr = SPI ;
	spiptr->SPI_CR = 1 ;								// Enable
	(void) spiptr->SPI_RDR ;		// Dump any rx data
	
	spiptr->SPI_TDR = byte ;

	result = 0 ; 
	while( ( spiptr->SPI_SR & SPI_SR_RDRF ) == 0 )
	{
		// wait for received
		if ( ++result > 10000 )
		{
			break ;				
		}
	}
	if ( count == 0 )
	{
		spiptr->SPI_CR = 2 ;								// Disable
		return spiptr->SPI_RDR ;
	}
	(void) spiptr->SPI_RDR ;		// Dump the rx data
	spiptr->SPI_TDR = 0 ;
	result = 0 ; 
	while( ( spiptr->SPI_SR & SPI_SR_RDRF ) == 0 )
	{
		// wait for received
		if ( ++result > 10000 )
		{
			break ;				
		}
	}
	spiptr->SPI_CR = 2 ;								// Disable
	return spiptr->SPI_RDR ;
}

uint32_t spi_operation( register uint8_t *tx, register uint8_t *rx, register uint32_t count )
{
	register Spi *spiptr ;
	register uint32_t result ;

//  PMC->PMC_PCER0 |= 0x00200000L ;		// Enable peripheral clock to SPI

	result = 0 ; 
	spiptr = SPI ;
	spiptr->SPI_CR = 1 ;								// Enable
	(void) spiptr->SPI_RDR ;		// Dump any rx data
	while( count )
	{
		result = 0 ;
		while( ( spiptr->SPI_SR & SPI_SR_TXEMPTY ) == 0 )
		{
			// wait
			if ( ++result > 10000 )
			{
				result = 0xFFFF ;
				break ;				
			}
		}
		if ( result > 10000 )
		{
			break ;
		}
//		if ( count == 1 )
//		{
//			spiptr->SPI_CR = SPI_CR_LASTXFER ;		// LastXfer bit
//		}
		spiptr->SPI_TDR = *tx++ ;
		result = 0 ;
		while( ( spiptr->SPI_SR & SPI_SR_RDRF ) == 0 )
		{
			// wait for received
			if ( ++result > 10000 )
			{
				result = 0x2FFFF ;
				break ;				
			}
		}
		if ( result > 10000 )
		{
			break ;
		}
		*rx++ = spiptr->SPI_RDR ;
		count -= 1 ;
	}
	if ( result <= 10000 )
	{
		result = 0 ;
	}
	spiptr->SPI_CR = 2 ;								// Disable

// Power save
//  PMC->PMC_PCER0 &= ~0x00200000L ;		// Disable peripheral clock to SPI

	return result ;
}

uint32_t spi_PDC_action( uint8_t *command, uint8_t *tx, uint8_t *rx, uint32_t comlen, uint32_t count )
{
#ifndef SIMU
	register Spi *spiptr ;
//	register uint32_t result ;
	register uint32_t condition ;
	static uint8_t discard_rx_command[4] ;

//  PMC->PMC_PCER0 |= 0x00200000L ;		// Enable peripheral clock to SPI

	Spi_complete = 0 ;
	if ( comlen > 4 )
	{
		Spi_complete = 1 ;
		return 0x4FFFF ;		
	}
	condition = SPI_SR_TXEMPTY ;
	spiptr = SPI ;
	spiptr->SPI_CR = 1 ;				// Enable
	(void) spiptr->SPI_RDR ;		// Dump any rx data
	(void) spiptr->SPI_SR ;			// Clear error flags
	spiptr->SPI_RPR = (uint32_t)discard_rx_command ;
	spiptr->SPI_RCR = comlen ;
	if ( rx )
	{
		spiptr->SPI_RNPR = (uint32_t)rx ;
		spiptr->SPI_RNCR = count ;
		condition = SPI_SR_RXBUFF ;
	}
	spiptr->SPI_TPR = (uint32_t)command ;
	spiptr->SPI_TCR = comlen ;
	if ( tx )
	{
		spiptr->SPI_TNPR = (uint32_t)tx ;
	}
	else
	{
		spiptr->SPI_TNPR = (uint32_t)rx ;
	}
	spiptr->SPI_TNCR = count ;

	spiptr->SPI_PTCR = SPI_PTCR_RXTEN | SPI_PTCR_TXTEN ;	// Start transfers

	// Wait for things to get started, avoids early interrupt
	for ( count = 0 ; count < 1000 ; count += 1 )
	{
		if ( ( spiptr->SPI_SR & SPI_SR_TXEMPTY ) == 0 )
		{
			break ;			
		}
	}
	spiptr->SPI_IER = condition ; 
#endif
	return 0 ;
}

//#endif


/**
 * Configures a UART peripheral with the specified parameters.
 *
 * baudrate  Baudrate at which the UART should operate (in Hz).
 * masterClock  Frequency of the system master clock (in Hz).
 * uses PA9 and PA10, RXD2 and TXD2
 */
void UART_Configure( uint32_t baudrate, uint32_t masterClock)
{
//    const Pin pPins[] = CONSOLE_PINS;
  register Uart *pUart = CONSOLE_USART;
//	register Pio *pioptr ;

  /* Configure PIO */
	configure_pins( (PIO_PA9 | PIO_PA10), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;
//	pioptr = PIOA ;
//  pioptr->PIO_ABCDSR[0] &= ~(PIO_PA9 | PIO_PA10) ;	// Peripheral A
//  pioptr->PIO_ABCDSR[1] &= ~(PIO_PA9 | PIO_PA10) ;	// Peripheral A
//  pioptr->PIO_PDR = (PIO_PA9 | PIO_PA10) ;					// Assign to peripheral

	if ( baudrate < 10 )
	{
		switch ( baudrate )
		{
			case 0 :
			default :
				baudrate = 9600 ;
			break ;
			case 1 :
				baudrate = 19200 ;
			break ;
			case 2 :
				baudrate = 38400 ;
			break ;
			case 3 :
				baudrate = 57600 ;
			break ;
			case 4 :
				baudrate = 115200 ;
			break ;
		}
	}

  /* Configure PMC */
  PMC->PMC_PCER0 = 1 << CONSOLE_ID;

  /* Reset and disable receiver & transmitter */
  pUart->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX
                 | UART_CR_RXDIS | UART_CR_TXDIS;

  /* Configure mode */
  pUart->UART_MR =  0x800 ;  // NORMAL, No Parity

  /* Configure baudrate */
  /* Asynchronous, no oversampling */
  pUart->UART_BRGR = (masterClock / baudrate) / 16;

  /* Disable PDC channel */
  pUart->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

  /* Enable receiver and transmitter */
  pUart->UART_CR = UART_CR_RXEN | UART_CR_TXEN;
  pUart->UART_IER = UART_IER_RXRDY ;

	NVIC_SetPriority( UART0_IRQn, 5 ) ; // Lower priority interrupt
	NVIC_EnableIRQ(UART0_IRQn) ;

}

void com1Parity( uint32_t even )
{
  register Usart *pUsart = SECOND_USART;
	if ( even )
	{
  	pUsart->US_MR =  0x000000C0 ;  // NORMAL, Even Parity, 8 bit
	}
	else
	{
	  pUsart->US_MR =  0x000008C0 ;  // NORMAL, No Parity, 8 bit
	}
}


void com2Parity( uint32_t even )
{
  register Uart *pUart = CONSOLE_USART ;
	if ( even )
	{
  	pUart->UART_MR =  0 ;  // NORMAL, Even Parity, 8 bit
	}
	else
	{
		pUart->UART_MR = 0x800 ;  // NORMAL, No Parity
	}
}



// Set up COM2 for SBUS (8E2), can't set 2 stop bits!
void UART_Sbus_configure( uint32_t masterClock )
{
  register Uart *pUart = CONSOLE_USART;
	
	UART_Configure( 100000, masterClock ) ;
  pUart->UART_MR =  0 ;  // NORMAL, Even Parity
}

void UART_Sbus57600_configure( uint32_t masterClock )
{
  register Uart *pUart = CONSOLE_USART;
	
	UART_Configure( 57600, masterClock ) ;
  pUart->UART_MR =  0 ;  // NORMAL, Even Parity
}

void UART_9dataOdd1stop()
{
	
}
//void UART_Stop()
//{
//  CONSOLE_USART->UART_IDR = UART_IDR_RXRDY ;
//	NVIC_DisableIRQ(UART0_IRQn) ;
//}


uint32_t txPdcCom2( struct t_serial_tx *data )
{
	Uart *pUart=CONSOLE_USART ;
		
	if ( pUart->UART_TNCR == 0 )
	{
		Current_Com2 = data ;
		data->ready = 1 ;
#ifndef SIMU
	  pUart->UART_TPR = (uint32_t)data->buffer ;
#endif
		pUart->UART_TCR = data->size ;
		pUart->UART_PTCR = US_PTCR_TXTEN ;
		pUart->UART_IER = UART_IER_TXBUFE ;
		NVIC_SetPriority( UART0_IRQn, 5 ) ; // Lower priority interrupt
		NVIC_EnableIRQ(UART0_IRQn) ;
		return 1 ;			// Sent OK
	}
	return 0 ;				// Busy
	
}

extern "C" void UART0_IRQHandler()
{
	Uart *pUart=CONSOLE_USART ;
	if ( pUart->UART_SR & UART_SR_TXBUFE )
	{
		pUart->UART_IDR = UART_IDR_TXBUFE ;
		pUart->UART_PTCR = US_PTCR_TXTDIS ;
		Current_Com2->ready = 0 ;	
	}
	if ( pUart->UART_SR & UART_SR_RXRDY )
	{
		if ( ( g_model.com2Function == COM2_FUNC_SBUSTRAIN ) || ( g_model.com2Function == COM2_FUNC_SBUS57600 ) )
		{
			put_fifo64( &Sbus_fifo, CONSOLE_USART->UART_RHR ) ;	
		}
		else if ( g_model.com2Function == COM2_FUNC_BTDIRECT )	// BT <-> COM2
		{
			telem_byte_to_bt( CONSOLE_USART->UART_RHR ) ;
		}
		else
		{
			put_fifo128( &Console_fifo, CONSOLE_USART->UART_RHR ) ;	
		}	 
	}
}

void UART3_Configure( uint32_t baudrate, uint32_t masterClock)
{
//    const Pin pPins[] = CONSOLE_PINS;
  register Uart *pUart = BT_USART;
//	register Pio *pioptr ;

  /* Configure PIO */
	configure_pins( (PIO_PB2 | PIO_PB3), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTB | PIN_NO_PULLUP ) ;
//	pioptr = PIOB ;
//  pioptr->PIO_ABCDSR[0] &= ~(PIO_PB2 | PIO_PB3) ;	// Peripheral A
//  pioptr->PIO_ABCDSR[1] &= ~(PIO_PB2 | PIO_PB3) ;	// Peripheral A
//  pioptr->PIO_PDR = (PIO_PB2 | PIO_PB3) ;					// Assign to peripheral

  /* Configure PMC */
  PMC->PMC_PCER0 = 1 << BT_ID;

  /* Reset and disable receiver & transmitter */
  pUart->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX
                 | UART_CR_RXDIS | UART_CR_TXDIS;

  /* Configure mode */
  pUart->UART_MR =  0x800 ;  // NORMAL, No Parity

  /* Configure baudrate */
  /* Asynchronous, no oversampling */
  pUart->UART_BRGR = ( (masterClock / baudrate) + 8 ) / 16;
  
// Following only available for USARTS
//	baudrate = (masterClock * 8 / baudrate) / 16 ;
//  pUart->UART_BRGR = ( baudrate / 8 ) | ( ( baudrate & 7 ) << 16 ) ;	// Fractional part to allow 115200 baud

  /* Disable PDC channel */
  pUart->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

  /* Enable receiver and transmitter */
  pUart->UART_CR = UART_CR_RXEN | UART_CR_TXEN;
  pUart->UART_IER = UART_IER_RXRDY ;
	NVIC_SetPriority( UART1_IRQn, 5 ) ; // Lower priority interrupt
	NVIC_EnableIRQ(UART1_IRQn) ;

}

//void Bt_UART_Stop()
//{
//  BT_USART->UART_IDR = UART_IDR_RXRDY ;
//	NVIC_DisableIRQ(UART1_IRQn) ;
//}


// USART0 configuration, we will use this for FrSky etc
// Work in Progress, UNTESTED
// Uses PA5 and PA6 (RXD and TXD)
void UART2_Configure( uint32_t baudrate, uint32_t masterClock)
{
////    const Pin pPins[] = CONSOLE_PINS;
  register Usart *pUsart = SECOND_USART;
//	register Pio *pioptr ;

  /* Configure PIO */
	disable_software_com1() ;
	configure_pins( (PIO_PA5 | PIO_PA6), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;
	
#ifdef REVX
	configure_pins( PIO_PA25, PIN_ENABLE | PIN_LOW | PIN_OUTPUT | PIN_PORTA | PIN_NO_PULLUP ) ;
#endif

//	pioptr = PIOA ;
//  pioptr->PIO_ABCDSR[0] &= ~(PIO_PA5 | PIO_PA6) ;	// Peripheral A
//  pioptr->PIO_ABCDSR[1] &= ~(PIO_PA5 | PIO_PA6) ;	// Peripheral A
//  pioptr->PIO_PDR = (PIO_PA5 | PIO_PA6) ;					// Assign to peripheral

//  /* Configure PMC */
  PMC->PMC_PCER0 = 1 << SECOND_ID;

//  /* Reset and disable receiver & transmitter */
  pUsart->US_CR = US_CR_RSTRX | US_CR_RSTTX
	                 | US_CR_RXDIS | US_CR_TXDIS;

//  /* Configure mode */
  pUsart->US_MR =  0x000008C0 ;  // NORMAL, No Parity, 8 bit

//  /* Configure baudrate */
//  /* Asynchronous, no oversampling */
  pUsart->US_BRGR = (masterClock / baudrate) / 16;

//  /* Disable PDC channel */
  pUsart->US_PTCR = US_PTCR_RXTDIS | US_PTCR_TXTDIS;

//  /* Enable receiver and transmitter */
  pUsart->US_CR = US_CR_RXEN | US_CR_TXEN;

}

void UART2_9dataOdd1stop()
{
	
}

void UART2_timeout_enable()
{
  register Usart *pUsart = SECOND_USART;
  pUsart->US_CR = US_CR_STTTO ;
  pUsart->US_RTOR = 115 ;		// Bits @ 115200 ~= 1mS
  pUsart->US_IER = US_IER_TIMEOUT ;
	DsmRxTimeout = 0 ;
	NVIC_SetPriority( USART0_IRQn, 5 ) ; // Lower priority interrupt
	NVIC_EnableIRQ(USART0_IRQn) ;
	
}

void UART2_timeout_disable()
{
  register Usart *pUsart = SECOND_USART;
  pUsart->US_RTOR = 0 ;

  pUsart->US_IDR = US_IDR_TIMEOUT ;
	NVIC_DisableIRQ(USART0_IRQn) ;
	
}

//static uint8_t SPI2ndByte ;

//uint32_t CopyMode ;
//uint8_t Scount1 ;
//uint8_t Scount2 ;
//uint8_t Scount3 ;

//uint16_t Sstat1 ;
//uint16_t Sstat2 ;
//uint16_t Sstat3 ;

extern "C" void USART0_IRQHandler()
{
  register Usart *pUsart = SECOND_USART;

#ifdef REVX
	if ( (pUsart->US_MR & 0x0000000F) == 0x0000000E )
	{
//		if ( pUsart->US_IMR & US_IMR_TXRDY )
//		{
//			if ( pUsart->US_CSR & US_CSR_TXRDY )
//			{
//				Sstat1 = pUsart->US_CSR ;
//				Sstat2 = pUsart->US_IMR ;
//  			pUsart->US_IDR = US_IDR_TXRDY ;
//  			pUsart->US_IER = US_IER_TXEMPTY ;
//				pUsart->US_THR = SPI2ndByte ;
//				Scount1 += 1 ;
//			}
//		}
		if ( pUsart->US_IMR & US_IMR_TXEMPTY )
		{
			if ( pUsart->US_CSR & US_CSR_TXEMPTY )
			{
  			pUsart->US_PTCR = US_PTCR_RXTDIS | US_PTCR_TXTDIS;
				// SPI mode for JETI, must be finished
  			pUsart->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS ;
		//#ifdef REVX
				PIOA->PIO_CODR = 0x02000000L ;	// Set bit A25 OFF
		//#endif
//			  pUsart->US_MR =  0x000008C0 ;  // NORMAL, No Parity, 8 bit
	  		pUsart->US_MR =  0x000202C0 ;  // NORMAL, Odd Parity, 9 bit
  			pUsart->US_BRGR = (Master_frequency / 9600) / 16;
	  		pUsart->US_IDR = 0xFFFFFFFF ;
				(void) pUsart->US_RHR ;
  			pUsart->US_CR = US_CR_RXEN ;
	  		pUsart->US_IER = US_IER_RXRDY ;
				NVIC_EnableIRQ(USART0_IRQn) ;
//				pUsart->US_PTCR = US_PTCR_RXTEN ;
			}
		}
		return ;
	}

	if ( pUsart->US_MR & 0x00020000 ) // 9-bit => Jeti
	{
		if ( pUsart->US_CSR & US_CSR_RXRDY )
		{
			uint16_t x ;
			x = pUsart->US_RHR ;
			put_16bit_fifo32( &Jeti_fifo, x ) ; // pUsart->US_RHR ) ;	
		}
		return ;
	}
#endif

  if ( pUsart->US_IMR & US_IMR_TIMEOUT )
	{
	  pUsart->US_CR = US_CR_STTTO ;		// Clears timeout bit
		DsmRxTimeout = 1 ;
	}
  if ( pUsart->US_IMR & US_IMR_ENDTX )
	{
	 	if ( pUsart->US_CSR & US_CSR_ENDTX )
		{
			pUsart->US_IER = US_IER_TXEMPTY ;
			pUsart->US_IDR = US_IDR_ENDTX ;
		}
	}
// Disable Tx output
  if ( pUsart->US_IMR & US_CSR_TXEMPTY )
	{
 		if ( pUsart->US_CSR & US_CSR_TXEMPTY )
		{
#ifdef REVX
			PIOA->PIO_CODR = 0x02000000L ;	// Set bit A25 OFF
#endif
			pUsart->US_IDR = US_IDR_TXEMPTY ;
			(void) pUsart->US_RHR ;	// Flush receiver
			pUsart->US_CR = US_CR_RXEN ;
		}
	}
}

// set outPtr start of buffer
// give 1st buffer to Uart as RPR/RCR
// set outPtr start of buffer
// give 2nd buffer to Uart as RNPR/RNCR

// read RPR
// if RPRcopy in TelemetryInBuffer[TelemetryActiveBuffer]
// process chars up to RPRcopy
// else process remaining chars in buffer, give buffer to Uart as RNPR/RNCR
//      TelemetryActiveBuffer becomes other buffer


//uint8_t OutputBuffer[128] ;
//uint32_t OutIndex ;

//void charProcess( uint8_t byte )
//{
//	OutputBuffer[OutIndex++] = byte ;
//	OutIndex &= 0x007F ;	
//}

//void poll2ndUsart10mS()
//{
//	rxPdcUsart( charProcess ) ;	
//}



void startPdcUsartReceive()
{
  register Usart *pUsart = SECOND_USART;
	
	TelemetryInBuffer.outPtr = TelemetryInBuffer.fifo ;
#ifndef SIMU
	pUsart->US_RPR = (uint32_t)TelemetryInBuffer.fifo ;
	pUsart->US_RNPR = (uint32_t)TelemetryInBuffer.fifo ;
#endif
	pUsart->US_RCR = RX_UART_BUFFER_SIZE ;
	pUsart->US_RNCR = RX_UART_BUFFER_SIZE ;
	pUsart->US_PTCR = US_PTCR_RXTEN ;
}

//void endPdcUsartReceive()
//{
//  register Usart *pUsart = SECOND_USART;
	
//	pUsart->US_PTCR = US_PTCR_RXTDIS ;
//}

void rxPdcUsart( void (*pChProcess)(uint8_t x) )
{
#if !defined(SIMU)
  register Usart *pUsart = SECOND_USART;
	uint8_t *ptr ;
	uint8_t *endPtr ;

 //Find out where the DMA has got to
	endPtr = (uint8_t *)pUsart->US_RPR ;
	// Check for DMA passed end of buffer
	if ( endPtr > &TelemetryInBuffer.fifo[RX_UART_BUFFER_SIZE-1] )
	{
		endPtr = TelemetryInBuffer.fifo ;
	}
	
	ptr = TelemetryInBuffer.outPtr ;
	while ( ptr != endPtr )
	{
		(*pChProcess)(*ptr++) ;
		if ( ptr > &TelemetryInBuffer.fifo[RX_UART_BUFFER_SIZE-1] )		// last byte
		{
			ptr = TelemetryInBuffer.fifo ;
		}
	}
	TelemetryInBuffer.outPtr = ptr ;

	if ( pUsart->US_RNCR == 0 )
	{
		pUsart->US_RNPR = (uint32_t)TelemetryInBuffer.fifo ;
		pUsart->US_RNCR = RX_UART_BUFFER_SIZE ;
	}
#endif
}

#ifdef REVX
void jetiSendWord( uint16_t word )
{
  register Usart *pUsart = SECOND_USART;
	uint32_t i ;
	uint16_t parity = 0 ;

	JetiTxBuffer[0] = 0 ;	// Sends a 1
	JetiTxBuffer[1] = 0 ;	// Sends a 1
	JetiTxBuffer[2] = 0xFF ;	// Sends a 0 ( start )
	
	for ( i = 3 ; i < 12 ; i += 1 )
	{
		parity += word ;
		JetiTxBuffer[i] = ( word & 1 ) ? 0 : 0xFF ;
		word >>= 1 ;
	}
	JetiTxBuffer[12] = ( parity & 1 ) ? 0xFF : 0 ;
	JetiTxBuffer[13] = 0 ;	// Stop bit
	JetiTxBuffer[14] = 0 ;	// Stop bit
	JetiTxBuffer[15] = 0 ;	// Stop bit

	 
	NVIC_DisableIRQ(USART0_IRQn) ;
//  /* Disable PDC channel */
  pUsart->US_PTCR = US_PTCR_RXTDIS | US_PTCR_TXTDIS;
//#ifdef REVX
//	PIOA->PIO_SODR = 0x02000000L ;	// Set bit A25 ON, enable SPort output
//#endif
  pUsart->US_CR = US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS ;

//  /* Configure mode */
  pUsart->US_MR = 0x0000080E ;  // SPI mode, 5 bits

//  /* Configure baudrate */
  pUsart->US_BRGR = ( Master_frequency / 9600 / 8 ) ;

  pUsart->US_IDR = 0xFFFFFFFF ;
//  /* Enable transmitter */
  pUsart->US_CR = US_CR_TXEN ;

	(void) pUsart->US_RHR ;
	txPdcUsart( JetiTxBuffer, 16, 0 ) ;
//	pUsart->US_THR = ( word >> 7 ) | 0xFE ;
  pUsart->US_IER = US_IER_TXEMPTY ;
	NVIC_EnableIRQ(USART0_IRQn) ;
}

int32_t getJetiWord()
{
	return get_16bit_fifo32( &Jeti_fifo ) ;
}

#endif

uint32_t txPdcUsart( uint8_t *buffer, uint32_t size, uint32_t receive )
{
  register Usart *pUsart = SECOND_USART;

	if ( pUsart->US_TNCR == 0 )
	{
#ifdef REVX
		PIOA->PIO_SODR = 0x02000000L ;	// Set bit A25 ON, enable SPort output
#endif

		if ( receive == 0 )
		{
			pUsart->US_CR = US_CR_RXDIS ;
		}
#ifndef SIMU
	  pUsart->US_TNPR = (uint32_t)buffer ;
#endif
		pUsart->US_TNCR = size ;
		pUsart->US_PTCR = US_PTCR_TXTEN ;
		pUsart->US_IER = US_IER_ENDTX ;
		NVIC_SetPriority( USART0_IRQn, 3 ) ; // Quite high priority interrupt
		NVIC_EnableIRQ(USART0_IRQn) ;
		return 1 ;
	}
	return 0 ;
}

uint32_t txCom2Uart( uint8_t *buffer, uint32_t size )
{
	Uart *pUart=CONSOLE_USART ;

	if ( pUart->UART_TNCR == 0 )
	{
#ifndef SIMU
	  pUart->UART_TNPR = (uint32_t)buffer ;
#endif
		pUart->UART_TNCR = size ;
		pUart->UART_PTCR = US_PTCR_TXTEN ;
		return 1 ;
	}
	return 0 ;
}


uint32_t txPdcPending()
{
  register Usart *pUsart = SECOND_USART;
	uint32_t x ;

	__disable_irq() ;
	pUsart->US_PTCR = US_PTCR_TXTDIS ;		// Freeze DMA
	x = pUsart->US_TNCR ;				// Total
	x += pUsart->US_TCR ;				// Still to send
	pUsart->US_PTCR = US_PTCR_TXTEN ;			// DMA active again
	__enable_irq() ;

	return x ;
}


struct t_serial_tx *Current_bt ;

uint32_t txPdcBt( struct t_serial_tx *data )
{
  Uart *pUart=BT_USART ;
		
	if ( pUart->UART_TNCR == 0 )
	{
		Current_bt = data ;
		data->ready = 1 ;
#ifndef SIMU
	  pUart->UART_TPR = (uint32_t)data->buffer ;
#endif
		pUart->UART_TCR = data->size ;
		pUart->UART_PTCR = US_PTCR_TXTEN ;
		pUart->UART_IER = UART_IER_TXBUFE ;
		NVIC_SetPriority( USART1_IRQn, 5 ) ; // Lower priority interrupt
		NVIC_EnableIRQ(UART1_IRQn) ;
		return 1 ;			// Sent OK
	}
	return 0 ;				// Busy
}

//void end_bt_tx_interrupt()
//{
//  Uart *pUart=BT_USART ;
//	pUart->UART_IDR = UART_IDR_TXBUFE ;
//	NVIC_DisableIRQ(UART1_IRQn) ;
//}

extern "C" void UART1_IRQHandler()
{
  Uart *pUart=BT_USART ;
	if ( pUart->UART_SR & UART_SR_TXBUFE )
	{
		pUart->UART_IDR = UART_IDR_TXBUFE ;
		pUart->UART_PTCR = US_PTCR_TXTDIS ;
		Current_bt->ready = 0 ;	
	}
	if ( pUart->UART_SR & UART_SR_RXRDY )
	{
		put_fifo128( &BtRx_fifo, pUart->UART_RHR ) ;	
extern uint16_t BtCounters[4] ;
 		BtCounters[3] += 1 ;
	}
}


/**
 * Outputs a character on the UART line.
 *
 * This function is synchronous (i.e. uses polling).
 * c  Character to send.
 */
void txmit( uint8_t c )
{
  Uart *pUart=CONSOLE_USART ;

	/* Wait for the transmitter to be ready */
  while ( (pUart->UART_SR & UART_SR_TXEMPTY) == 0 ) ;

  /* Send character */
  pUart->UART_THR=c ;
}

// Outputs a string to the UART

uint16_t rxuart()
{
	return get_fifo128( &Console_fifo ) ;
  
//	Uart *pUart=CONSOLE_USART ;

//  if (pUart->UART_SR & UART_SR_RXRDY)
//	{
//		return pUart->UART_RHR ;
//	}
//	return 0xFFFF ;
}

void txmit2nd( uint8_t c )
{
  register Usart *pUsart = SECOND_USART;

	/* Wait for the transmitter to be ready */
  while ( (pUsart->US_CSR & US_CSR_TXEMPTY) == 0 ) ;

  /* Send character */
  pUsart->US_THR=c ;
}

uint16_t rx2nduart()
{
  register Usart *pUsart = SECOND_USART;

  if (pUsart->US_CSR & US_CSR_RXRDY)
	{
		return pUsart->US_RHR ;
	}
	return 0xFFFF ;
}

void txmitBt( uint8_t c )
{
  Uart *pUart=BT_USART ;
	uint32_t x ;

	/* Wait for the transmitter to be ready */
	x = 10000 ;
  while ( (pUart->UART_SR & UART_SR_TXEMPTY) == 0 )
	{
		if ( --x == 0 )
		{
			break ;			// Timeout so we don't hang
		}
	}
  /* Send character */
  pUart->UART_THR=c ;
}

int32_t rxBtuart()
{
	return get_fifo128( &BtRx_fifo ) ;
}

//void xread_9_adc()
//{
//	register Adc *padc ;
//	register uint32_t y ;
//	register uint32_t x ;
//	static uint16_t timer = 0 ;

//	padc = ADC ;
//#ifdef REVB
//#ifndef REVX
//	padc->ADC_CHER = 0x00000400 ;  // channel 10 on
//#endif
//#endif
	
//	y = padc->ADC_ISR ;		// Clear EOC flags
//	padc->ADC_CR = 2 ;		// Start conversion
//	x = 0 ;
//	while ( ( padc->ADC_ISR & 0x00008000 ) == 0 )
//	{
//		// wait for EOC15 flag
//		if ( ++x > 1000000 )
//		{
//			break ;		// Software timeout				
//		}
//	}
//#ifdef REVB
//#ifndef REVX
//	padc->ADC_CHDR = 0x00000400 ;  // channel 10 off
//#endif
//#endif
//	Analog_values[0] = ADC->ADC_CDR2 ;
//	Analog_values[1] = ADC->ADC_CDR9 ;
//	Analog_values[2] = ADC->ADC_CDR14 ;
//	Analog_values[3] = ADC->ADC_CDR1 ;
//	Analog_values[4] = ADC->ADC_CDR5 ;
//	Analog_values[5] = ADC->ADC_CDR13 ;
//	Analog_values[6] = ADC->ADC_CDR3 ;
//	Analog_values[7] = ADC->ADC_CDR4 ;

//#ifdef REVB
//	Analog_values[8] = ADC->ADC_CDR8 ;
	
//	x = ADC->ADC_CDR10 ;
//	y = Analog_values[9] ;
//	int32_t diff = x - y ;
//	if ( diff < 0 )
//	{
//		diff = -diff ;
//	}
//	if ( diff > 10 )
//	{
//		if ( ( ( g_tmr10ms - timer ) & 0x0000FFFF ) > 3 )
//		{
//			timer = g_tmr10ms ;
//			Analog_values[9] = x ;
//		}
//	}
//	else
//	{
//		timer = g_tmr10ms ;
//		Analog_values[9] = x ;
//	}

//#endif
//	Temperature = ( Temperature * 7 + ADC->ADC_CDR15 ) >> 3 ;	// Filter it
//	if ( Temperature > Max_temperature )
//	{
//		Max_temperature = Temperature ;		
//	}
	 
//}


// Read 8 (9 for REVB) ADC channels
// Documented bug, must do them 1 by 1
void read_adc()
{
	register Adc *padc ;
	register uint32_t y ;
	register uint32_t x ;
	static uint16_t timer = 0 ;

//	PMC->PMC_PCER0 |= 0x20000000L ;		// Enable peripheral clock to ADC

	padc = ADC ;
#ifdef REVB
#ifndef REVX
	padc->ADC_CHER = 0x00000400 ;  // channel 10 on
#endif
#endif
	y = padc->ADC_ISR ;		// Clear EOC flags
	for ( y = NUMBER_ANALOG+1 ; --y > 0 ; )		// Include temp sensor
	{
		padc->ADC_CR = 2 ;		// Start conversion
		x = 0 ;
		while ( ( padc->ADC_ISR & 0x01000000 ) == 0 )
		{
			// wait for DRDY flag
			if ( ++x > 1000000 )
			{
				break ;		// Software timeout				
			}
		}
		x = padc->ADC_LCDR ;		// Clear DRSY flag
	}
#ifdef REVB
#ifndef REVX
	padc->ADC_CHDR = 0x00000400 ;  // channel 10 off
#endif
#endif
	// Next bit may be done using the PDC
// Option on 9XR to increase ADC accuracy
//#ifdef REVX
//	int32_t calc = ADC->ADC_CDR2 ;
//	calc -= 2048 ;
//	calc *= 18 ;
//	calc >>= 4 ;
//	calc += 2048 ;
//	Analog_values[0] = calc ;
//	calc = ADC->ADC_CDR9 ;
//	calc -= 2048 ;
//	calc *= 18 ;
//	calc >>= 4 ;
//	calc += 2048 ;
//	Analog_values[1] = calc ;
//	calc = ADC->ADC_CDR14 ;
//	calc -= 2048 ;
//	calc *= 21 ;
//	calc >>= 4 ;
//	calc += 2048 ;
//	Analog_values[2] = calc ;
//	calc = ADC->ADC_CDR1 ;
//	calc -= 2048 ;
//	calc *= 21 ;
//	calc >>= 4 ;
//	calc += 2048 ;
//	Analog_values[3] = calc ;
//	Analog_values[4] = ADC->ADC_CDR5 ;
//	Analog_values[5] = ADC->ADC_CDR13 ;
//	Analog_values[6] = ADC->ADC_CDR3 ;
//	Analog_values[7] = ADC->ADC_CDR4 ;
//#else	
	Analog_values[0] = ADC->ADC_CDR2 ;
	Analog_values[1] = ADC->ADC_CDR9 ;
	Analog_values[2] = ADC->ADC_CDR14 ;
	Analog_values[3] = ADC->ADC_CDR1 ;
#ifndef REVX
	if ( g_eeGeneral.ar9xBoard == 0 )
	{
#endif
		Analog_values[4] = ADC->ADC_CDR5 ;
		Analog_values[6] = ADC->ADC_CDR3 ;
#ifndef REVX
	}
	else
	{
		Analog_values[6] = ADC->ADC_CDR5 ;
		Analog_values[4] = ADC->ADC_CDR3 ;
	}
#endif
	Analog_values[5] = ADC->ADC_CDR13 ;
	Analog_values[7] = ADC->ADC_CDR4 ;
//#endif
#ifdef REVB
	Analog_values[8] = ADC->ADC_CDR8 ;
//#ifdef REVX
	x = ADC->ADC_CDR10 ;
	if ( ( g_eeGeneral.extraPotsSource[0] != 1 ) && ( g_eeGeneral.extraPotsSource[1] != 1 ) )
	{
		y = Analog_values[9] ;
		int32_t diff = x - y ;
		if ( diff < 0 )
		{
			diff = -diff ;
		}
		if ( diff > 10 )
		{
			if ( ( ( g_tmr10ms - timer ) & 0x0000FFFF ) > 10 )
			{
				timer = g_tmr10ms ;
				Analog_values[9] = x ;
			}
		}
		else
		{
			timer = g_tmr10ms ;
			Analog_values[9] = x ;
		}
	}
	else
	{
		Analog_values[9] = x ;
	}
//#endif
#endif
	Temperature = ( Temperature * 7 + ADC->ADC_CDR15 ) >> 3 ;	// Filter it
	if ( Temperature > Max_temperature )
	{
		Max_temperature = Temperature ;		
	}


	AnalogData[0] = Analog_values[0] ;
	AnalogData[1] = Analog_values[1] ;
	AnalogData[2] = Analog_values[2] ;
	AnalogData[3] = Analog_values[3] ;
	AnalogData[4] = Analog_values[4] ;
	AnalogData[5] = Analog_values[5] ;
	AnalogData[6] = Analog_values[6] ;
	AnalogData[12] = Analog_values[7] ;
	if ( g_eeGeneral.ar9xBoard == 0 )
	{
		AnalogData[13] = Analog_values[8] ;
		AnalogData[7] = Analog_values[9] ;
	}
	else
	{
		AnalogData[7] = Analog_values[9] ;
		AnalogData[8] = Analog_values[8] ;
	}

// Power save
//  PMC->PMC_PCER0 &= ~0x20000000L ;		// Disable peripheral clock to ADC

}


// Settings for mode register ADC_MR
// USEQ off - silicon problem, doesn't work
// TRANSFER = 3
// TRACKTIM = 15 (16 clock periods)
// ANACH = 1
// SETTLING = 6 (not used if ANACH = 0)
// STARTUP = 6 (96 clock periods)
// PRESCAL = 9.0 MHz clock (between 1 and 20MHz)
// FREERUN = 0
// FWUP = 0
// SLEEP = 0
// LOWRES = 0
// TRGSEL = 0
// TRGEN = 0 (software trigger only)
// Gain/offset channels:
// Stick LV AD9  Gain:0x00080000 Offset:0x00000200
// Stick LH AD2  Gain:0x00000020 Offset:0x00000004
// Stick RV AD14 Gain:0x20000000 Offset:0x00004000
// Stick RH AD1  Gain:0x00000008 Offset:0x00000002
// Gains in ADC_CGR, offsets in ADC_COR

#define GAIN_LV			0x00080000
#define GAIN_LH	    0x00000020
#define GAIN_RV	    0x20000000
#define GAIN_RH	    0x00000008

#define GAIN_CURRENT	    0x00030000

#define OFF_LV			0x00000200
#define OFF_LH      0x00000004
#define OFF_RV      0x00004000
#define OFF_RH      0x00000002

// 9xR-PRO needs AD10 for extra 3-pos switch

void init_adc()
{
	register Adc *padc ;
	register uint32_t timer ;

	timer = ( Master_frequency / (9000000*2) - 1 ) << 8 ;
	// Enable peripheral clock ADC = bit 29
  PMC->PMC_PCER0 |= 0x20000000L ;		// Enable peripheral clock to ADC
	padc = ADC ;
	padc->ADC_MR = 0x3FB60000 | timer ;  // 0011 1111 1011 0110 xxxx xxxx 0000 0000
	padc->ADC_ACR = ADC_ACR_TSON ;			// Turn on temp sensor
#ifdef REVB
#ifdef REVX
	padc->ADC_CHER = 0x0000E73E ;  // channels 1,2,3,4,5,8,9,10,13,14,15
#else
	padc->ADC_CHER = 0x0000E33E ;  // channels 1,2,3,4,5,8,9,13,14,15
#endif
#else
	padc->ADC_CHER = 0x0000E23E ;  // channels 1,2,3,4,5,9,13,14,15
#endif
	padc->ADC_CGR = 0 ;  // Gain = 1, all channels
	padc->ADC_COR = 0 ;  // Single ended, 0 offset, all channels

//	padc->ADC_CGR = GAIN_LV | GAIN_LH | GAIN_RV | GAIN_RH ;
//	padc->ADC_COR = OFF_LV | OFF_LH | OFF_RV | OFF_RH ;
}

void set_stick_gain( uint32_t gains )
{
	register Adc *padc ;
	uint32_t gain ;
	uint32_t offset ;

	gain = GAIN_CURRENT ;
	if ( g_eeGeneral.ar9xBoard )
	{
		gain = 0 ;
	}	
	offset = 0 ;
	padc = ADC ;

	if ( gains & STICK_LV_GAIN )
	{
		gain |= GAIN_LV ;
		offset |= OFF_LV ;
	}
	if ( gains & STICK_LH_GAIN )
	{
		gain |= GAIN_LH ;
		offset |= OFF_LH ;
	}
	if ( gains & STICK_RV_GAIN )
	{
		gain |= GAIN_RV ;
		offset |= OFF_RV ;
	}
	if ( gains & STICK_RH_GAIN )
	{
		gain |= GAIN_RH ;
		offset |= OFF_RH ;
	}

	padc->ADC_CGR = gain ;
	padc->ADC_COR = offset ;

}

// Start TIMER3 for input capture
void start_timer3()
{
#ifndef SIMU
  register Tc *ptc ;
//	register Pio *pioptr ;

	// Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x04000000L ;		// Enable peripheral clock to TC3

  ptc = TC1 ;		// Tc block 1 (TC3-5)
	ptc->TC_BCR = 0 ;			// No sync
	ptc->TC_BMR = 2 ;
	ptc->TC_CHANNEL[0].TC_CMR = 0x00000000 ;	// Capture mode
	ptc->TC_CHANNEL[0].TC_CMR = 0x00090005 ;	// 0000 0000 0000 1001 0000 0000 0000 0101, XC0, A rise, B fall
	ptc->TC_CHANNEL[0].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)

	configure_pins( PIO_PC23, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_B | PIN_PORTC | PIN_PULLUP ) ;
//	pioptr = PIOC ;
//  pioptr->PIO_ABCDSR[0] |= 0x00800000 ;		// Peripheral B = TIOA3
//  pioptr->PIO_ABCDSR[1] &= ~0x00800000 ;	// Peripheral B
//	pioptr->PIO_PDR = 0x00800000L ;		// Disable bit C23 (TIOA3) Assign to peripheral
	NVIC_SetPriority( TC3_IRQn, 14 ) ; // Low priority interrupt
	NVIC_EnableIRQ(TC3_IRQn) ;
	ptc->TC_CHANNEL[0].TC_IER = TC_IER0_LDRAS ;
#endif
}

// Start Timer4 to provide 0.5uS clock for input capture
void start_timer4()
{
#ifndef SIMU
  register Tc *ptc ;
	register uint32_t timer ;

	timer = Master_frequency / (2*2000000) ;		// MCK/2 and 2MHz

	// Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x08000000L ;		// Enable peripheral clock to TC4

  ptc = TC1 ;		// Tc block 1 (TC3-5)
	ptc->TC_BCR = 0 ;			// No sync
	ptc->TC_BMR = 0 ;
	ptc->TC_CHANNEL[1].TC_CMR = 0x00008000 ;	// Waveform mode
	ptc->TC_CHANNEL[1].TC_RC = timer ;
	ptc->TC_CHANNEL[1].TC_RA = timer >> 1 ;
	ptc->TC_CHANNEL[1].TC_CMR = 0x0009C000 ;	// 0000 0000 0000 1001 1100 0000 0100 0000
																						// MCK/2, set @ RA, Clear @ RC waveform
	ptc->TC_CHANNEL[1].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
#endif
}

void start_ppm_capture()
{
	start_timer4() ;
	start_timer3() ;
}

//void end_ppm_capture()
//{
//	TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_LDRAS ;
//	NVIC_DisableIRQ(TC3_IRQn) ;
//}


#ifdef SERIAL_TRAINER


void setCaptureMode(uint32_t mode)
{
	CaptureMode = mode ;

	NVIC_DisableIRQ(PIOA_IRQn) ;
	TC1->TC_CHANNEL[2].TC_CCR = 2 ;		// Disable clock
		if ( mode == CAP_SERIAL )
	{
		LineState = LINE_IDLE ;
		BitTime = BIT_TIME_115K ;
		TC1->TC_CHANNEL[0].TC_CMR = 0x00090005 ;	// 0000 0000 0000 1001 0000 0000 0000 0101, XC0, A rise, B fall
		// Or for inverted operation:
//		TC1->TC_CHANNEL[0].TC_CMR = 0x00060005 ;	// 0000 0000 0000 0110 0000 0000 0000 0101, XC0, A fall, B rise
		TC1->TC_CHANNEL[0].TC_IER = TC_IER0_LDRBS ;		// Int on falling edge
	}
	else
	{
		TC1->TC_CHANNEL[0].TC_CMR = 0x00090005 ;	// 0000 0000 0000 1001 0000 0000 0000 0101, XC0, A rise, B fall
	}
}

void start_timer5()
{
#ifndef SIMU
  register Tc *ptc ;
//	register Pio *pioptr ;

	// Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x10000000L ;		// Enable peripheral clock to TC5

  ptc = TC1 ;		// Tc block 1 (TC3-5)
	ptc->TC_BCR = 0 ;			// No sync
	ptc->TC_BMR = 0x32 ;
	ptc->TC_CHANNEL[2].TC_CMR = 0x00000000 ;	// Capture mode
	ptc->TC_CHANNEL[2].TC_CMR = 0x00090007 ;	// 0000 0000 0000 1001 0000 0000 0000 0101, XC2, A rise, B fall
	ptc->TC_CHANNEL[2].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
	NVIC_SetPriority( TC5_IRQn, 14 ) ; // Low priority interrupt
	NVIC_EnableIRQ(TC5_IRQn) ;
#endif
}

void stop_timer5()
{
	TC1->TC_CHANNEL[2].TC_CCR = 0 ;		// Disable clock
	NVIC_DisableIRQ(TC5_IRQn) ;
}

// Handle software serial on COM1 input (for non-inverted input)
void init_software_com1(uint32_t baudrate, uint32_t invert, uint32_t parity)
{
	BitTime = 2000000 / baudrate ;

	SoftSerInvert = invert ? 0 : PIO_PA5 ;
	SoftSerialEvenParity = parity ? 1 : 0 ;

	TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_LDRAS ;		// No int on rising edge
	TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_LDRBS ;		// No int on falling edge
	TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_CPCS ;		// No compare interrupt
	LineState = LINE_IDLE ;
	CaptureMode = CAP_COM1 ;
	configure_pins( PIO_PA5, PIN_ENABLE | PIN_INPUT | PIN_PORTA ) ;
	PIOA->PIO_IER = PIO_PA5 ;
	NVIC_SetPriority( PIOA_IRQn, 0 ) ; // Highest priority interrupt
	NVIC_EnableIRQ(PIOA_IRQn) ;
	start_timer5() ;
}

void disable_software_com1()
{
	stop_timer5() ;
	CaptureMode = CAP_PPM ;
	PIOA->PIO_IDR = PIO_PA5 ;
	NVIC_DisableIRQ(PIOA_IRQn) ;
}

extern "C" void PIOA_IRQHandler()
{
  register uint32_t capture ;
  register uint32_t dummy ;
	
	capture =  TC1->TC_CHANNEL[0].TC_CV ;	// Capture time
	dummy = PIOA->PIO_ISR ;			// Read and clear status register
	(void) dummy ;		// Discard value - prevents compiler warning

	dummy = PIOA->PIO_PDSR ;
	if ( ( dummy & PIO_PA5 ) == SoftSerInvert )
	{
		// L to H transisition
		LtoHtime = capture ;
		TC1->TC_CHANNEL[2].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
		TC1->TC_CHANNEL[2].TC_RC = BitTime * 10 ;
		uint32_t time ;
		capture -= HtoLtime ;
		time = capture ;
		putCaptureTime( time, 0 ) ;
		TC1->TC_CHANNEL[2].TC_IER = TC_IER0_CPCS ;		// Compare interrupt
		(void) TC1->TC_CHANNEL[2].TC_SR ;
	}
	else
	{
		// H to L transisition
		HtoLtime = capture ;
		if ( LineState == LINE_IDLE )
		{
			LineState = LINE_ACTIVE ;
			putCaptureTime( 0, 3 ) ;
			TC1->TC_CHANNEL[2].TC_RC = capture + (BitTime * 20) ;
			(void) TC1->TC_CHANNEL[2].TC_SR ;
		}
		else
		{
			uint32_t time ;
			capture -= LtoHtime ;
			time = capture ;
			putCaptureTime( time, 1 ) ;
		}
		TC1->TC_CHANNEL[2].TC_IDR = TC_IDR0_CPCS ;		// No compare interrupt
	}
}

extern "C" void TC5_IRQHandler()
{
	uint32_t status ;

	status = TC1->TC_CHANNEL[2].TC_SR ;
	if ( status & TC_SR0_CPCS )
	{		
		uint32_t time ;
		time = TC1->TC_CHANNEL[0].TC_CV - LtoHtime ;
		putCaptureTime( time, 2 ) ;
		LineState = LINE_IDLE ;
		TC1->TC_CHANNEL[2].TC_IDR = TC_IDR0_CPCS ;		// No compare interrupt
	}
}

#endif

// Timer3 used for PPM_IN pulse width capture. Counter running at 16MHz / 8 = 2MHz
// equating to one count every half microisecond. (2 counts = 1us). Control channel
// count delta values thus can range from about 1600 to 4400 counts (800us to 2200us),
// corresponding to a PPM signal in the range 0.8ms to 2.2ms (1.5ms at center).
// (The timer is free-running and is thus not reset to zero at each capture interval.)
// Timer 4 generates the 2MHz clock to clock Timer 3

extern uint16_t SbusTimer ;

extern "C" void TC3_IRQHandler() //capture ppm in at 2MHz
{
  uint16_t capture ;
  static uint16_t lastCapt ;
  uint16_t val ;

	uint32_t status ;
	if ( CaptureMode == CAP_SERIAL )
	{
		if ( ( status = TC1->TC_CHANNEL[0].TC_SR ) & TC_SR0_LDRBS )
		{	// H->L edge
			capture = TC1->TC_CHANNEL[0].TC_RB ;
			HtoLtime = capture ;
			if ( LineState == LINE_IDLE )
			{
				LineState = LINE_ACTIVE ;
		  }
			else
			{
				uint32_t time ;
				capture -= LtoHtime ;
				time = capture ;
				putCaptureTime( time, 1 ) ;
			}
			TC1->TC_CHANNEL[0].TC_IER = TC_IER0_LDRAS ;		// Int on rising edge
			TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_LDRBS ;		// No int on falling edge
			TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_CPCS ;		// No compare interrupt
		}
		else if ( status & TC_SR0_LDRAS )
		{	// L->H edge
			capture = TC1->TC_CHANNEL[0].TC_RA ;
			LtoHtime = capture ;
			TC1->TC_CHANNEL[0].TC_RC = capture + (BitTime * 10) ;
			uint32_t time ;
			capture -= HtoLtime ;
			time = capture ;
			putCaptureTime( time, 0 ) ;
			TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_LDRAS ;		// No int on rising edge
			TC1->TC_CHANNEL[0].TC_IER = TC_IER0_LDRBS ;		// Int on falling edge
			TC1->TC_CHANNEL[0].TC_IER = TC_IER0_CPCS ;		// Compare interrupt
		}
		else
		{ // Compare interrupt
			uint32_t time ;
			time = TC1->TC_CHANNEL[0].TC_CV - LtoHtime ;
			putCaptureTime( time, 2 ) ;
			LineState = LINE_IDLE ;
			TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_LDRAS ;		// No int on rising edge
			TC1->TC_CHANNEL[0].TC_IER = TC_IER0_LDRBS ;		// Int on falling edge
			TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_CPCS ;		// No compare interrupt
		}
		return ;
	}

	status = TC1->TC_CHANNEL[0].TC_SR ;
	if ( CaptureMode == CAP_COM1 )
	{
		if ( status & TC_SR0_CPCS )
		{		
			uint32_t time ;
			time = TC1->TC_CHANNEL[0].TC_RC - LtoHtime ;
			putCaptureTime( time, 2 ) ;
			LineState = LINE_IDLE ;
			TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_CPCS ;		// No compare interrupt
		}
	}

	if ( status & TC_SR0_LDRAS )
	{
		capture = TC1->TC_CHANNEL[0].TC_RA ;
//		(void) TC1->TC_CHANNEL[0].TC_SR ;		// Acknowledgethe interrupt

  	val = (uint16_t)(capture - lastCapt) / 2 ;
  	lastCapt = capture;

  	// We prcoess g_ppmInsright here to make servo movement as smooth as possible
  	//    while under trainee control
  	if (val>4000 && val < 19000) // G: Prioritize reset pulse. (Needed when less than 8 incoming pulses)
  	  ppmInState = 1; // triggered
  	else
  	{
  		if(ppmInState && ppmInState<=16)
			{
  	  	if(val>800 && val<2200)
				{
					if ( g_eeGeneral.trainerSource == 0 )
					{
						ppmInValid = 100 ;
						if ( SbusTimer == 0 )
						{ // Not receiving trainer data over SBUS
	  		  	  g_ppmIns[ppmInState++ - 1] = (int16_t)(val - 1500)*(g_eeGeneral.PPM_Multiplier+10)/10 ;
							//+-500 != 512, but close enough.
						}
					}
		    }
				else
				{
  		    ppmInState=0; // not triggered
  	  	}
  	  }
  	}
	}
}

// Initialise the SSC to allow PXX/DSM output.
// TD is on PA17, peripheral A
void init_ssc( uint16_t baudrate )
{
//	register Pio *pioptr ;
	register Ssc *sscptr ;

	Scc_baudrate = baudrate ;

  PMC->PMC_PCER0 |= 0x00400000L ;		// Enable peripheral clock to SSC
	
//	pioptr = PIOA ;
//  pioptr->PIO_ABCDSR[0] &= ~0x00020000 ;	// Peripheral A bit 17
//  pioptr->PIO_ABCDSR[1] &= ~0x00020000 ;	// Peripheral A
//  pioptr->PIO_PDR = 0x00020000 ;					// Assign to peripheral
	
	sscptr = SSC ;
#ifdef ASSAN
	if ( g_model.protocol == PROTO_ASSAN )
	{
		sscptr->SSC_THR = 0 ;		// Make the output low.
		sscptr->SSC_TFMR = 0x00000007 ; 	//  0000 0000 0000 0000 0000 0000 0000 0111 (8 bit data, lsb)
		NVIC_SetPriority( SSC_IRQn, 7 ) ; // Lower priority interrupt
		NVIC_EnableIRQ(SSC_IRQn) ;
	}
	else
	{
		sscptr->SSC_THR = 0xFF ;		// Make the output high.
		sscptr->SSC_TFMR = 0x00000027 ; 	//  0000 0000 0000 0000 0000 0000 1010 0111 (8 bit data, lsb)
	}
#else
	sscptr->SSC_THR = 0xFF ;		// Make the output high.
	sscptr->SSC_TFMR = 0x00000027 ; 	//  0000 0000 0000 0000 0000 0000 1010 0111 (8 bit data, lsb)
#endif
	uint32_t divisor = 125000*2 ;
	if ( baudrate )
	{
		divisor = (baudrate == 1) ? 115200*2 : 100000*2 ;
	}
	sscptr->SSC_CMR = Master_frequency / divisor ;		// 8uS per bit
	sscptr->SSC_TCMR = 0 ;  	//  0000 0000 0000 0000 0000 0000 0000 0000
	sscptr->SSC_CR = SSC_CR_TXEN ;

	configure_pins( PIO_PA17, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA ) ;
#ifdef REVX
	if ( baudrate )
	{
		PIOA->PIO_MDDR = PIO_PA17 ;						// Push Pull O/p in A17
	}
	else
	{
		PIOA->PIO_MDER = PIO_PA17 ;						// Open Drain O/p in A17
	}
#else
	PIOA->PIO_MDDR = PIO_PA17 ;						// Push Pull O/p in A17
#endif
}

void disable_ssc()
{
	register Pio *pioptr ;
	register Ssc *sscptr ;

	// Revert back to pwm output
	pioptr = PIOA ;
	pioptr->PIO_PER = PIO_PA17 ;						// Assign A17 to PIO
	
	sscptr = SSC ;
	sscptr->SSC_CR = SSC_CR_TXDIS ;
}

//uint16_t SSCdebug ;

extern "C" void SSC_IRQHandler()
{
//	SSCdebug += 1 ;
	if ( SSC->SSC_IMR & SSC_IMR_TXBUFE )
	{
		SSC->SSC_IDR = SSC_IDR_TXBUFE ;	// Stop interrupt
		SSC->SSC_IER = SSC_IER_TXEMPTY ;
	}
	else
	{
		SSC->SSC_IDR = SSC_IDR_TXEMPTY ;	// Stop interrupt
		PIOA->PIO_PER = PIO_PA17 ;				// Assign A17 to PIO
  	SECOND_USART->US_CR = US_CR_RXEN ;
		dsmTelemetryStartReceive() ;
//		PIOA->PIO_PDR = PIO_PA5 ;					// Assign A5 to Peripheral
	}
//	configure_pins( PIO_PA17, PIN_INPUT | PIN_PORTA | PIN_PULLUP ) ;
}

#endif

#if defined(PCBX9D) || defined(PCB9XT)

//  /* Determine the integer part */
//  if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
//  {
//    /* Integer part computing in case Oversampling mode is 8 Samples */
//    integerdivider = ((25 * apbclock) / (2 * (USART_InitStruct->USART_BaudRate)));    
//  }
//  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
//  {
//    /* Integer part computing in case Oversampling mode is 16 Samples */
//    integerdivider = ((25 * apbclock) / (4 * (USART_InitStruct->USART_BaudRate)));    
//  }
//  tmpreg = (integerdivider / 100) << 4;

//  /* Determine the fractional part */
//  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

//  /* Implement the fractional part in the register */
//  if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
//  {
//    tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
//  }
//  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
//  {
//    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
//  }
  
//  /* Write to USART BRR register */
//  USARTx->BRR = (uint16_t)tmpreg;

//extern uint32_t Peri1_frequency ;
//extern uint32_t Peri2_frequency ;

void USART6_Sbus_configure()
{
	RCC->APB2ENR |= RCC_APB2ENR_USART6EN ;		// Enable clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
//	GPIOC->MODER = (GPIOC->MODER & 0xFFFFBFFF ) | 0x00008000 ;	// Alternate func.
//	GPIOC->AFR[0] = (GPIOC->AFR[0] & 0x0FFFFFFF ) | 0x80000000 ;	// Alternate func.
	configure_pins( 0x0080, PIN_PERIPHERAL | PIN_PORTC | PIN_PER_8 ) ;
	USART6->BRR = PeripheralSpeeds.Peri2_frequency / 100000 ;
	USART6->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_M | USART_CR1_PCE ;
	USART6->CR2 = 0 ;
	USART6->CR3 = 0 ;
	(void) USART6->DR ;
	NVIC_SetPriority( USART6_IRQn, 5 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(USART6_IRQn) ;
}

void stop_USART6_Sbus()
{
	configure_pins( 0x0080, PIN_INPUT | PIN_PORTC ) ;
  NVIC_DisableIRQ(USART6_IRQn) ;
}

extern "C" void USART6_IRQHandler()
{
	put_fifo64( &Sbus_fifo, USART6->DR ) ;	
}

#ifndef PCB9XT
void UART_Sbus_configure( uint32_t masterClock )
{
	USART3->BRR = PeripheralSpeeds.Peri1_frequency / 100000 ;
	USART3->CR1 |= USART_CR1_M | USART_CR1_PCE ;
}

void UART_Sbus57600_configure( uint32_t masterClock )
{
	USART3->BRR = PeripheralSpeeds.Peri1_frequency / 57600 ;
	USART3->CR1 |= USART_CR1_M | USART_CR1_PCE ;
}


void x9dConsoleInit()
{
	// Serial configure  
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN ;		// Enable clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
	GPIOB->MODER = (GPIOB->MODER & 0xFF0FFFFF ) | 0x00A00000 ;	// Alternate func.
	GPIOB->AFR[1] = (GPIOB->AFR[1] & 0xFFFF00FF ) | 0x00007700 ;	// Alternate func.
	USART3->BRR = PeripheralSpeeds.Peri1_frequency / CONSOLE_BAUDRATE ;		// 97.625 divider => 19200 baud
	USART3->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE ;
	USART3->CR2 = 0 ;
	USART3->CR3 = 0 ;
	NVIC_SetPriority( USART3_IRQn, 5 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(USART3_IRQn) ;
}
#endif

void x9dSPortInit( uint32_t baudRate, uint32_t mode, uint32_t invert, uint32_t parity )
{
	// Serial configure  
	if ( mode == SPORT_MODE_SOFTWARE )
	{
	  NVIC_DisableIRQ(USART2_IRQn) ;
		init_software_com1( baudRate, invert, parity ) ;
		return ;
	}
	disable_software_com1() ;

	RCC->APB1ENR |= RCC_APB1ENR_USART2EN ;		// Enable clock
#ifdef PCB9XT
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
	GPIOB->BSRRH = 0x0004 ;		// output disable
#else
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ; 		// Enable portD clock
	GPIOD->BSRRH = PIN_SPORT_ON ;		// output disable
#endif
#ifdef PCB9XT
// PB2 as SPort enable
	configure_pins( 0x00000004, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB ) ;
	configure_pins( 0x00000008, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PER_7 | PIN_PORTA ) ;
	configure_pins( 0x00000004, PIN_PERIPHERAL | PIN_PER_7 | PIN_PORTA ) ;
#else
	configure_pins( 0x00000010, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;
	GPIOD->MODER = (GPIOD->MODER & 0xFFFFC0FF ) | 0x00002900 ;	// Alternate func.
	GPIOD->AFR[0] = (GPIOD->AFR[0] & 0xF00FFFFF ) | 0x07700000 ;	// Alternate func.
#endif
	if ( baudRate > 10 )
	{
		USART2->BRR = PeripheralSpeeds.Peri1_frequency / baudRate ;
	}
	else if ( baudRate == 0 )
	{
		USART2->BRR = PeripheralSpeeds.Peri1_frequency / 57600 ;		// 16.25 divider => 57600 baud
	}
	else
	{
		USART2->BRR = PeripheralSpeeds.Peri1_frequency / 9600 ;		// 97.625 divider => 9600 baud
	}
	USART2->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE ;
	USART2->CR2 = 0 ;
	USART2->CR3 = 0 ;
	if ( baudRate == 115200 )		// ASSAN DSM
	{
#ifdef PCB9XT
		GPIOB->BSRRL = 0x0004 ;		// output disable
#else
		GPIOD->BSRRL = 0x0010 ;		// output enable
#endif
	}
	NVIC_SetPriority( USART2_IRQn, 5 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(USART2_IRQn);
}


void x9dHubTxStart( uint8_t *buffer, uint32_t count )
{
	SportTx.ptr = buffer ;
	SportTx.count = count ;
	SportTx.busy = 1 ;
	USART2->CR1 |= USART_CR1_TXEIE ;
}

uint32_t hubTxPending()
{
	return SportTx.busy ;
}

void x9dSPortTxStart( uint8_t *buffer, uint32_t count, uint32_t receive )
{
	SportTx.ptr = buffer ;
	SportTx.count = count ;
#ifdef PCB9XT
	GPIOB->BSRRL = 0x0004 ;		// output enable
#else
	GPIOD->BSRRL = 0x0010 ;		// output enable
#endif
	if ( receive == 0 )
	{
		USART2->CR1 &= ~USART_CR1_RE ;
	}
	USART2->CR1 |= USART_CR1_TXEIE ;
}

#if !defined(SIMU)

//#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE)
#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_FE | USART_FLAG_PE)

extern "C" void USART2_IRQHandler()
{
  uint32_t status;
  uint8_t data;

  status = USART2->SR ;

	if ( status & USART_SR_TXE )
	{
		if ( SportTx.count )
		{
			USART2->DR = *SportTx.ptr++ ;
			if ( --SportTx.count == 0 )
			{
				USART2->CR1 &= ~USART_CR1_TXEIE ;	// Stop Tx interrupt
				USART2->CR1 |= USART_CR1_TCIE ;	// Enable complete interrupt
			}
		}
	}

	if ( ( status & USART_SR_TC ) && (USART2->CR1 & USART_CR1_TCIE ) )
	{
		USART2->CR1 &= ~USART_CR1_TCIE ;	// Stop Complete interrupt
		if ( g_model.xprotocol != PROTO_ASSAN )
		{
#ifdef PCB9XT
			GPIOB->BSRRH = 0x0004 ;		// output disable
			SportTx.busy = 0 ;

#else
			GPIOD->BSRRH = PIN_SPORT_ON ;		// output disable
#endif
			USART2->CR1 |= USART_CR1_RE ;
		}
	}

  while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS))
	{
    data = USART2->DR;

//			put_fifo64( &Telemetry_fifo, data ) ;
    if (!(status & USART_FLAG_ERRORS))
		{
			put_fifo64( &Telemetry_fifo, data ) ;
		}
		else
		{
			USART_ERRORS += 1 ;
			if ( status & USART_FLAG_ORE )
			{
				USART_ORE += 1 ;
			}
			if ( status & USART_FLAG_NE )
			{
				USART_NE += 1 ;
			}
			if ( status & USART_FLAG_FE )
			{
				USART_FE += 1 ;
			}
			if ( status & USART_FLAG_PE )
			{
				USART_PE += 1 ;
			}
		}
    status = USART2->SR ;
  }
}

// Start TIMER7 at 2000000Hz
void start_2Mhz_timer()
{
	// Now for timer 7
	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN ;		// Enable clock
	
	TIM7->ARR = 0xFFFF ;
	TIM7->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ;		// 0.5uS
	TIM7->CR2 = 0 ;
	TIM7->CR2 = 0x20 ;
	TIM7->CR1 = TIM_CR1_CEN ;
}

#endif	// SIMU

uint16_t rxTelemetry()
{
	return get_fifo64( &Telemetry_fifo ) ;
}

#ifndef PCB9XT
void txmit( uint8_t c )
{
	/* Wait for the transmitter to be ready */
  while ( (USART3->SR & USART_SR_TXE) == 0 ) ;

  /* Send character */
	USART3->DR = c ;
}

uint16_t rxuart()
{
//  if (USART3->SR & USART_SR_RXNE)
//	{
//		return USART3->DR ;
//	}
//	return 0xFFFF ;
	return get_fifo128( &Console_fifo ) ;
}

extern "C" void USART3_IRQHandler()
{
	if ( ( g_model.com2Function == COM2_FUNC_SBUSTRAIN ) || ( g_model.com2Function == COM2_FUNC_SBUS57600 ) )
	{
		put_fifo64( &Sbus_fifo, USART3->DR ) ;	
	}
	else
	{
		put_fifo128( &Console_fifo, USART3->DR ) ;	
#ifdef BLUETOOTH
extern uint16_t BtCounters[4] ;
 		BtCounters[3] += 1 ;
#endif
	}	 
}
#endif

static void start_timer11()
{
#ifndef SIMU

	RCC->APB2ENR |= RCC_APB2ENR_TIM11EN ;		// Enable clock
	TIM11->PSC = (PeripheralSpeeds.Peri2_frequency*PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;		// 0.5uS
	TIM11->CCER = 0 ;
	TIM11->DIER = 0 ;
	TIM11->CCMR1 = 0 ;
	 
	TIM11->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 ;

	TIM11->CR1 = TIM_CR1_CEN ;

	NVIC_SetPriority( TIM1_TRG_COM_TIM11_IRQn, 14 ) ; // Low priority interrupt
	NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn) ;
#endif
}

static void stop_timer11()
{
	TIM11->CR1 = 0 ;
	NVIC_DisableIRQ(TIM1_TRG_COM_TIM11_IRQn) ;
}


//#ifdef PCB9XT
//// Soft Serial capture, PA3, Timer 9 channel 2
//void init_softSerial_capture()
//{
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
//	configure_pins( GPIO_Pin_3, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_3 | PIN_PULLUP) ;
//	RCC->APB2ENR |= RCC_APB2ENR_TIM9EN ;		// Enable clock
	
//	TIM9->ARR = 0xFFFF ;
//	TIM9->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ;		// 0.5uS
//	TIM9->CR2 = 0 ;
//	TIM9->CCMR1 = TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_CC2S_0 | TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_CC1S_1 ;
//	TIM9->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC2P ;
//	TIM9->SR &= ~(TIM_SR_CC2IF | TIM_SR_CC1IF) ;				// Clear flags
//	TIM9->DIER |= TIM_DIER_CC2IE | TIM_DIER_CC1IE ;
//	TIM9->CR1 = TIM_CR1_CEN ;
//  NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 0);
//	NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn) ;
//}
//#endif

// Handle software serial on COM1 input (for non-inverted input)
void init_software_com1(uint32_t baudrate, uint32_t invert, uint32_t parity )
{
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN ;		// Enable clock
	
	BitTime = 2000000 / baudrate ;
	SoftSerialEvenParity = parity ? 1 : 0 ;

//#ifdef PCB9XT
//	SoftSerInvert = invert ? 0 : GPIO_Pin_3 ;	// Port A3
//	LineState = LINE_IDLE ;
//	CaptureMode = CAP_COM1 ;
//	init_softSerial_capture() ;
//	start_timer11() ;
//	return ;
//#endif

#ifdef PCB9XT
	SoftSerInvert = invert ? 0 : GPIO_Pin_3 ;	// Port A3
#else
	SoftSerInvert = invert ? 0 : GPIO_Pin_6 ;	// Port D6
#endif

	LineState = LINE_IDLE ;
	CaptureMode = CAP_COM1 ;

#ifdef PCB9XT
#define EXT_BIT_MASK	0x00000008
	SYSCFG->EXTICR[0] = 0 ;
#else
#define EXT_BIT_MASK	0x00000040
	SYSCFG->EXTICR[1] = 0x0300 ;
#endif
	EXTI->IMR = EXT_BIT_MASK ;
	EXTI->RTSR = EXT_BIT_MASK ;
	EXTI->FTSR = EXT_BIT_MASK ;

#ifdef PCB9XT
	configure_pins( GPIO_Pin_3, PIN_INPUT | PIN_PORTA ) ;
	NVIC_SetPriority( EXTI3_IRQn, 0 ) ; // Highest priority interrupt
	NVIC_EnableIRQ( EXTI3_IRQn) ;
#else
	configure_pins( GPIO_Pin_6, PIN_INPUT | PIN_PORTD ) ;
	NVIC_SetPriority( EXTI9_5_IRQn, 0 ) ; // Highest priority interrupt
	NVIC_EnableIRQ( EXTI9_5_IRQn) ;
#endif
	
	start_timer11() ;
}

void disable_software_com1()
{
	CaptureMode = CAP_PPM ;
	stop_timer11() ;
	EXTI->IMR &= ~EXT_BIT_MASK ;
#ifdef PCB9XT
	NVIC_DisableIRQ( EXTI3_IRQn ) ;
#else
	NVIC_DisableIRQ( EXTI9_5_IRQn ) ;
#endif
}


#ifdef PCB9XT
void console9xtInit()
{
	// Serial configure  
	RCC->APB1ENR |= RCC_APB1ENR_UART4EN ;		// Enable clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	configure_pins( 0x00000001, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA | PIN_PER_8 ) ;
	configure_pins( 0x00000002, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_8 | PIN_PULLUP ) ;
	GPIOA->MODER = (GPIOA->MODER & 0xFFFFFFF0 ) | 0x0000000A ;	// Alternate func.
	GPIOA->AFR[0] = (GPIOA->AFR[0] & 0xFFFFFF00 ) | 0x00000088 ;	// Alternate func.
	UART4->BRR = PeripheralSpeeds.Peri1_frequency / CONSOLE_BAUDRATE ;		// 97.625 divider => 19200 baud
	UART4->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE ;
	UART4->CR2 = 0 ;
	UART4->CR3 = 0 ;
	NVIC_SetPriority( UART4_IRQn, 5 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(UART4_IRQn) ;
}

void com3Init( uint32_t baudrate )
{
	USART_TypeDef *puart = USART3 ;
	// Serial configure  
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN ;		// Enable clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
	configure_pins( GPIO_Pin_10, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB | PIN_PER_7 ) ;
	configure_pins( GPIO_Pin_11, PIN_PERIPHERAL | PIN_PORTB | PIN_PER_7 | PIN_PULLUP ) ;
	puart->BRR = PeripheralSpeeds.Peri1_frequency / baudrate ;
	puart->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE ;
	puart->CR2 = 0 ;
	puart->CR3 = 0 ;
	NVIC_SetPriority( USART3_IRQn, 5 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(USART3_IRQn) ;
}

void com3Stop()
{
	USART3->CR1 = 0 ;
	RCC->APB1ENR &= ~RCC_APB1ENR_USART3EN ;		// Enable clock
  NVIC_DisableIRQ(USART3_IRQn) ;
}

void Com3SetBaudrate ( uint32_t baudrate )
{
	USART3->BRR = PeripheralSpeeds.Peri1_frequency / baudrate ;
}

void com1Parity( uint32_t even )
{
	if ( even )
	{
		USART2->CR1 |= USART_CR1_PCE | USART_CR1_M ;
	}
	else
	{
		USART2->CR1 &= ~(USART_CR1_PCE | USART_CR1_M) ;
	}
}


void com2Parity( uint32_t even )
{
	if ( even )
	{
		UART4->CR1 |= USART_CR1_PCE | USART_CR1_M ;
	}
	else
	{
		UART4->CR1 &= ~(USART_CR1_PCE | USART_CR1_M) ;
	}
}

void com3Parity( uint32_t even )
{
	if ( even )
	{
		USART3->CR1 |= USART_CR1_PCE | USART_CR1_M ;
	}
	else
	{
		USART3->CR1 &= ~(USART_CR1_PCE | USART_CR1_M) ;
	}
}

void UART4SetBaudrate ( uint32_t baudrate )
{
	UART4->BRR = PeripheralSpeeds.Peri1_frequency / baudrate ;
}

extern "C" void USART3_IRQHandler()
{
  uint32_t status;
  uint8_t data;
	USART_TypeDef *puart = USART3 ;

  status = puart->SR ;
	if ( ( status & USART_SR_TXE ) && (puart->CR1 & USART_CR1_TXEIE ) )
	{
		if ( Current_Com3 )
		{
			if ( Current_Com3->size )
			{
				puart->DR = *Current_Com3->buffer++ ;
				if ( --Current_Com3->size == 0 )
				{
					puart->CR1 &= ~USART_CR1_TXEIE ;	// Stop Tx interrupt
					puart->CR1 |= USART_CR1_TCIE ;	// Enable complete interrupt
				}
			}
			else
			{
				puart->CR1 &= ~USART_CR1_TXEIE ;	// Stop Tx interrupt
			}
		}
	}
	
	if ( ( status & USART_SR_TC ) && (puart->CR1 & USART_CR1_TCIE ) )
	{
		puart->CR1 &= ~USART_CR1_TCIE ;	// Stop Complete interrupt
		Current_Com3->ready = 0 ;
	}
	
  while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS))
	{
    data = puart->DR ;

    if (!(status & USART_FLAG_ERRORS))
		{
			if ( g_eeGeneral.btComPort == 2 )
			{
				put_fifo128( &BtRx_fifo, data ) ;	
#ifdef BLUETOOTH
extern uint16_t BtCounters[4] ;
		 		BtCounters[3] += 1 ;
#endif
			}
			else
			{
				put_fifo128( &Com3_fifo, data ) ;	
			}
		}
		else
		{
			if ( status & USART_FLAG_ORE )
			{
				USART_ORE += 1 ;
			}
		}
    status = puart->SR ;
	}
}


int32_t rxBtuart()
{
	return get_fifo128( &BtRx_fifo ) ;
}

uint32_t txPdcBt( struct t_serial_tx *data )
{
	data->ready = 1 ;
	if ( g_eeGeneral.btComPort == 1 )
	{
		Current_Com2 = data ;
		UART4->CR1 |= USART_CR1_TXEIE ;
	}
	else if ( g_eeGeneral.btComPort == 2 )
	{
		Current_Com3 = data ;
		USART3->CR1 |= USART_CR1_TXEIE ;
	}
	return 1 ;			// Sent OK
}

//uint16_t Uart4Errors ;
//uint16_t Uart4Bad ;
//uint16_t Uart4Data ;
//uint16_t Uart4RxCount ;

extern "C" void UART4_IRQHandler()
{
  uint32_t status;
  uint8_t data;
	USART_TypeDef *puart = UART4 ;

  status = puart->SR ;
	if ( ( status & USART_SR_TXE ) && (puart->CR1 & USART_CR1_TXEIE ) )
	{
		if ( Current_Com2 )
		{
			if ( Current_Com2->size )
			{
				puart->DR = *Current_Com2->buffer++ ;
				if ( --Current_Com2->size == 0 )
				{
					puart->CR1 &= ~USART_CR1_TXEIE ;	// Stop Tx interrupt
					puart->CR1 |= USART_CR1_TCIE ;	// Enable complete interrupt
				}
			}
			else
			{
				puart->CR1 &= ~USART_CR1_TXEIE ;	// Stop Tx interrupt
			}
		}
	}
	
	if ( ( status & USART_SR_TC ) && (puart->CR1 & USART_CR1_TCIE ) )
	{
		puart->CR1 &= ~USART_CR1_TCIE ;	// Stop Complete interrupt
		Current_Com2->ready = 0 ;
	}
	
  while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS))
	{
    data = puart->DR;

    if (!(status & USART_FLAG_ERRORS))
		{
			if ( ( g_model.com2Function == COM2_FUNC_SBUSTRAIN ) || ( g_model.com2Function == COM2_FUNC_SBUS57600 ) )
			{
				put_fifo64( &Sbus_fifo, data ) ;
			}
			else
			{
				if ( g_eeGeneral.btComPort == 1 )
				{
					put_fifo128( &BtRx_fifo, data ) ;
#ifdef BLUETOOTH
extern uint16_t BtCounters[4] ;
			 		BtCounters[3] += 1 ;
#endif
				}
				else
				{
//					Uart4RxCount +=1 ;
					put_fifo128( &Console_fifo, data ) ;
				}
			}
		}
		else
		{
			if ( status & USART_FLAG_ORE )
			{
				USART_ORE += 1 ;
			}
			if ( status & USART_FLAG_NE )
			{
				USART_NE += 1 ;
			}
			if ( status & USART_FLAG_FE )
			{
				USART_FE += 1 ;
			}
			if ( status & USART_FLAG_PE )
			{
				USART_PE += 1 ;
			}
//			Uart4Bad = status ;
//			Uart4Errors += 1 ;
//			Uart4Data = data ;
		}
    status = puart->SR ;
	}
}

void txmit( uint8_t c )
{
	/* Wait for the transmitter to be ready */
  while ( (UART4->SR & USART_SR_TXE) == 0 ) ;

  /* Send character */
	UART4->DR = c ;
}

uint16_t rxuart()
{
	return get_fifo128( &Console_fifo ) ;
}

void UART_Sbus_configure( uint32_t masterClock )
{
	UART4->BRR = PeripheralSpeeds.Peri1_frequency / 100000 ;
	UART4->CR1 |= USART_CR1_M | USART_CR1_PCE ;
}

void UART_Sbus57600_configure( uint32_t masterClock )
{
	UART4->BRR = PeripheralSpeeds.Peri1_frequency / 57600 ;
	UART4->CR1 |= USART_CR1_M | USART_CR1_PCE ;
}


uint16_t RemBitTime ;
uint8_t RemLineState ;
uint16_t RemHtoLtime ;
uint16_t RemLtoHtime ;
uint8_t RemBitState ;
uint8_t RemBitCount ;
uint8_t RemByte ;
uint32_t RemIntCount ;
uint16_t RemLatency ;

// Handle software serial from M64
void init_software_remote()
{
	RemBitTime = 2000000 / 9600 ;
	RemLineState = LINE_IDLE ;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 	// Enable portB clock
	configure_pins( 0x0100, PIN_PERIPHERAL | PIN_PORTB | PIN_PER_3 | PIN_PULLUP ) ;
	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN ;		// Enable clock
	TIM10->ARR = 0xFFFF ;
	TIM10->PSC = (PeripheralSpeeds.Peri2_frequency*PeripheralSpeeds.Timer_mult2) / 2000000 - 1 ;		// 0.5uS
	TIM10->CCMR1 = TIM_CCMR1_CC1S_0 ;
	TIM10->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P | TIM_CCER_CC1NP ;
	TIM10->SR &= ~TIM_SR_CC1IF ;				// Clear flag
	TIM10->DIER |= TIM_DIER_CC1IE ;
	TIM10->CR1 = TIM_CR1_CEN ;
	NVIC_SetPriority( TIM1_UP_TIM10_IRQn, 3 ) ; // Lower priority interrupt
	NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn) ;
}

extern "C" void timer10_interrupt()
{
	if ( TIM10->SR & TIM_SR_CC1IF)
	{
  	uint16_t capture ;
		uint32_t level = GPIOB->IDR & 0x0100 ? 1 : 0 ;
		capture = TIM10->CCR1 ;	// Capture time
		TIM10->SR &= ~TIM_SR_CC1IF ;				// Clear flag
		RemIntCount += 1 ;
	
		{
			uint16_t ltime ;
			ltime = TIM10->CNT ;
			ltime -= capture ;
			if ( ltime > RemLatency )
			{
				RemLatency = ltime ;
			}
		}

		if ( level )
		{
			// L to H transisition
			RemLtoHtime = capture ;
			uint32_t time ;
			capture -= RemHtoLtime ;
			time = capture ;
			time += RemBitTime/2 ;
			time /= RemBitTime ;		// Now number of bits
			if ( RemBitState == BIT_IDLE )
			{ // Starting, value should be 0
				RemBitState = BIT_ACTIVE ;
				RemBitCount = 0 ;
				if ( time > 1 )
				{
					RemByte >>= time-1 ;
					RemBitCount = time-1 ;
					if ( RemBitCount >= 8 )
					{
						put_fifo64( &RemoteRx_fifo, RemByte ) ;
						RemBitState = BIT_IDLE ;
						RemLineState = LINE_IDLE ;
					}
				}
			}
			else
			{
				RemByte >>= time ;
				RemBitCount += time ;
			}
		}
		else
		{
			// H to L transisition
			RemHtoLtime = capture ;
			if ( RemLineState == LINE_IDLE )
			{
				RemLineState = LINE_ACTIVE ;
			}
			else
			{
				uint32_t time ;
				capture -= RemLtoHtime ;
				time = capture ;
				time += RemBitTime/2 ;
				time /= RemBitTime ;		// Now number of bits

				while ( time )
				{
					if ( RemBitCount >= 8 )
					{ // Got a byte
						put_fifo64( &RemoteRx_fifo, RemByte ) ;
						RemBitState = BIT_IDLE ;
						break ;
					}
					else
					{
						RemByte >>= 1 ;
						RemByte |= 0x80 ;
						time -= 1 ;
						RemBitCount += 1 ;
					}
				}
			}
		} 
	}
}


#endif

//uint16_t LastTransition ;
//uint16_t Missing1 ;
//uint16_t Missing2 ;
//uint16_t LastSr ;
//uint32_t DumpStack[32] ;



//uint32_t __get_PSP(void)
//{
//  uint32_t result=0;

//  __ASM volatile ("MRS %0, psp\n\t"
//                  "MOV r0, %0 \n\t"
//                  "BX  lr     \n\t"  : "=r" (result) );
//  return(result);
//}


#ifdef PCB9XT
//extern "C" void TIM1_BRK_TIM9_IRQHandler()
extern "C" void EXTI3_IRQHandler()
#else
extern "C" void EXTI9_5_IRQHandler()
#endif
{
  register uint32_t capture ;
  register uint32_t dummy ;

//#ifdef PCB9XT
//	dummy = TIM9->SR & (TIM_SR_CC2IF | TIM_SR_CC1IF) ;

//	if ( ( LastSr & 0x400 ) == 0 )
//	{
//		if ( TIM9->SR & 0x400 )
//		{
//			uint32_t i ;
//			i = __get_PSP() ;
//			DumpStack[0] = i ;
//			uint32_t *ptr = (uint32_t *) i ;

//			for ( i = 2 ; i < 14 ; i += 1 )
//			{
//				DumpStack[i] = *ptr++ ;
//			}
//		}
//		else
//		{
//			uint32_t i ;
//			i = __get_PSP() ;
//			DumpStack[1] = i ;
//			uint32_t *ptr = (uint32_t *) i ;
//			for ( i = 14 ; i < 26 ; i += 1 )
//			{
//				DumpStack[i] = *ptr++ ;
//			}
//		}
//	}
//	LastSr = TIM9->SR ;

//	// CC1 physical rising, CC2 falling edge
//	// So for inverted serial CC1 is HtoL, CC2 is LtoH
	
//	if ( LineState == LINE_IDLE )
//	{
//		SerIndex = 0 ;
//	}
	
//	if ( LastTransition == 0 )	// LtoH
//	{
//		capture = TIM9->CCR1 ;
//		LastTransition = 1 ;
//	}
//	else
//	{
//		capture = TIM9->CCR2 ;
//		LastTransition = 0 ;
//	}
//	SerTimes[SerIndex] = capture ;
//	if ( SerIndex < 63 )
//	{
//		SerIndex += 1 ;
//	}

//	if ( LastTransition == 0 )	// LtoH
//	{
//		// L to H transition
//		LtoHtime = capture ;
//		TIM11->CNT = 0 ;
//		TIM11->CCR1 = BitTime * 12 ;
//		uint32_t time ;
//		capture -= HtoLtime ;
//		time = capture ;
//		putCaptureTime( time, 0 ) ;
//		TIM11->DIER = TIM_DIER_CC1IE ;
//	}
//	else
//	{
//		// H to L transition
//		LastHtoL = HtoLtime ;
//		HtoLtime = capture ;
//		if ( LineState == LINE_IDLE )
//		{
//			LineState = LINE_ACTIVE ;
//			putCaptureTime( 0, 3 ) ;
//		}
//		else
//		{
//			uint32_t time ;
//			capture -= LtoHtime ;
//			time = capture ;
//			putCaptureTime( time, 1 ) ;
//		}
//		TIM11->DIER = 0 ;
//		TIM11->CCR1 = BitTime * 20 ;
//		TIM11->CNT = 0 ;
//		TIM11->SR = 0 ;
//	}
//	return ;
//#endif

	capture =  TIM7->CNT ;	// Capture time
	EXTI->PR = EXT_BIT_MASK ;
	
#ifdef PCB9XT
	dummy = GPIOA->IDR ;
	if ( ( dummy & GPIO_Pin_3 ) == SoftSerInvert )
#else
	dummy = GPIOD->IDR ;
	if ( ( dummy & GPIO_Pin_6 ) == SoftSerInvert )
#endif
	{
		// L to H transition
//		if ( LastTransition == 0 )
//		{
//			Missing1 += 1 ;
//		}
//		LastTransition = 0 ;
			 
//		LastLtoH = LtoHtime ;
		
		LtoHtime = capture ;
		TIM11->CNT = 0 ;
		TIM11->CCR1 = BitTime * 12 ;
		uint32_t time ;
		capture -= HtoLtime ;
		time = capture ;
		putCaptureTime( time, 0 ) ;
		TIM11->DIER = TIM_DIER_CC1IE ;
//		(void) TC1->TC_CHANNEL[2].TC_SR ;
	}
	else
	{
		// H to L transition
//		LastHtoL = HtoLtime ;
//		if ( LastTransition == 1 )
//		{
//			Missing2 += 1 ;
//		}
//		LastTransition = 1 ;
		
		HtoLtime = capture ;
		if ( LineState == LINE_IDLE )
		{
			LineState = LINE_ACTIVE ;
			putCaptureTime( 0, 3 ) ;
//			(void) TC1->TC_CHANNEL[2].TC_SR ;
		}
		else
		{
			uint32_t time ;
			capture -= LtoHtime ;
			time = capture ;
			putCaptureTime( time, 1 ) ;
//			TIM11->CCER = 0 ;
		}
		TIM11->DIER = 0 ;
		TIM11->CCR1 = BitTime * 20 ;
		TIM11->CNT = 0 ;
		TIM11->SR = 0 ;
//		TIM11->EGR = 0 ;
	}
}

extern "C" void TIM1_TRG_COM_TIM11_IRQHandler()
{
	uint32_t status ;

	status = TIM11->SR ;
	if ( status & TIM_SR_CC1IF )
	{		
//		TIM11Count += 1 ;
		uint32_t time ;
		time = TIM7->CNT - LtoHtime ;
		putCaptureTime( time, 2 ) ;
		LineState = LINE_IDLE ;
		TIM11->DIER = 0 ;
//		TIM11->EGR = 0 ;
		TIM11->SR = 0 ;
	}
	
}



#endif // X9D || SP

#ifdef PCB9XT
void init_spi()
{
	register uint8_t *p ;
	uint8_t spi_buf[4] ;
//  SPI_InitTypeDef  SPI_InitStructure;
//  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock for CS */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE);
  /* Enable GPIO clock for Signals */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE);
  /* Enable SPI clock, SPI1: APB2, SPI2: APB1 */
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN ;    // Enable clock

	// APB1 clock / 2 = 133nS per clock
	SPI1->CR1 = 0 ;		// Clear any mode error
	SPI1->CR2 = 0 ;
	SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPOL | SPI_CR1_CPHA ;
	SPI1->CR1 |= SPI_CR1_MSTR ;	// Make sure in case SSM/SSI needed to be set first
	SPI1->CR1 |= SPI_CR1_SPE ;

	configure_pins( GPIO_Pin_SPI_EE_CS, PIN_PUSHPULL | PIN_OS25 | PIN_OUTPUT | PIN_PORTA ) ;
	GPIOA->BSRRL = GPIO_Pin_SPI_EE_CS ;		// output disable
	configure_pins( GPIO_Pin_SPI_EE_SCK | GPIO_Pin_SPI_EE_MOSI, PIN_PUSHPULL | PIN_OS25 | PIN_PERIPHERAL | PIN_PORTB | PIN_PER_5 ) ;
	configure_pins( GPIO_Pin_SPI_EE_MISO, PIN_PERIPHERAL | PIN_PORTB | PIN_PULLUP | PIN_PER_5 ) ;

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;			// Enable DMA2 clock

	NVIC_SetPriority( DMA2_Stream0_IRQn, 9 ) ; // Lower priority interrupt
  NVIC_EnableIRQ( DMA2_Stream0_IRQn ) ;
	NVIC_SetPriority( DMA2_Stream3_IRQn, 9 ) ; // Lower priority interrupt
  NVIC_EnableIRQ( DMA2_Stream3_IRQn ) ;

	p = spi_buf ;
  
	eeprom_write_enable() ;
	
	*p = 1 ;		// Write status register command
	*(p+1) = 0 ;
	spi_operation( p, spi_buf, 2 ) ;
	      
	GPIOA->BSRRL = GPIO_Pin_SPI_EE_CS ;		// output disable
//#ifdef STM32_SD_USE_DMA
//  /* enable DMA clock */
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
//#endif
	
}

uint32_t spi_operation( register uint8_t *tx, register uint8_t *rx, register uint32_t count )
{
	register SPI_TypeDef *spiptr = SPI1 ;
	register uint32_t result ;
	
	GPIOA->BSRRH = GPIO_Pin_SPI_EE_CS ;		// output enable
	(void) spiptr->DR ;		// Dump any rx data
	while( count )
	{
		result = 0 ;
		while( ( spiptr->SR & SPI_SR_TXE ) == 0 )
		{
			// wait
			if ( ++result > 10000 )
			{
				result = 0xFFFF ;
				break ;				
			}
		}
		if ( result > 10000 )
		{
			break ;
		}
//		if ( count == 1 )
//		{
//			spiptr->SPI_CR = SPI_CR_LASTXFER ;		// LastXfer bit
//		}
		spiptr->DR = *tx++ ;
		result = 0 ;
		while( ( spiptr->SR & SPI_SR_RXNE ) == 0 )
		{
			// wait for received
			if ( ++result > 10000 )
			{
				result = 0x2FFFF ;
				break ;				
			}
		}
		if ( result > 10000 )
		{
			break ;
		}
		*rx++ = spiptr->DR ;
		count -= 1 ;
	}
	if ( result <= 10000 )
	{
		result = 0 ;
	}
	result = 0 ; 
	
	return result ;
}

void eeprom_write_enable()
{
	eeprom_write_one( 6, 0 ) ;
	GPIOA->BSRRL = GPIO_Pin_SPI_EE_CS ;		// output disable
}

uint32_t eeprom_read_status()
{
	uint32_t result ;
	
	result = eeprom_write_one( 5, 1 ) ;
	GPIOA->BSRRL = GPIO_Pin_SPI_EE_CS ;		// output disable
	return result ;
}

uint32_t  eeprom_write_one( uint8_t byte, uint8_t count )
{
	register SPI_TypeDef *spiptr = SPI1 ;
	register uint32_t result ;
	
	GPIOA->BSRRH = GPIO_Pin_SPI_EE_CS ;		// output enable
	(void) spiptr->DR ;		// Dump any rx data
	
	spiptr->DR = byte ;

	result = 0 ; 
	while( ( spiptr->SR & SPI_SR_RXNE ) == 0 )
	{
		// wait for received
		if ( ++result > 10000 )
		{
			break ;				
		}
	}
	if ( count == 0 )
	{
		return spiptr->DR ;
	}
	(void) spiptr->DR ;		// Dump the rx data
	spiptr->DR = 0 ;
	result = 0 ; 
	while( ( spiptr->SR & SPI_SR_RXNE ) == 0 )
	{
		// wait for received
		if ( ++result > 10000 )
		{
			break ;				
		}
	}
	return spiptr->DR ;
}

uint32_t spi_PDC_action( uint8_t *command, uint8_t *tx, uint8_t *rx, uint32_t comlen, uint32_t count )
{
	static uint8_t discard_rx_command[4] ;

	Spi_complete = 0 ;
	if ( comlen > 4 )
	{
		Spi_complete = 1 ;
		return 0x4FFFF ;		
	}
	
	GPIOA->BSRRH = GPIO_Pin_SPI_EE_CS ;		// output enable
	DMA2_Stream0->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	DMA2_Stream3->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	
	spi_operation( command, discard_rx_command, comlen ) ;	// send command

	if ( rx )
	{
		DMA2_Stream0->CR = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0 | DMA_SxCR_PL_0 
											  | DMA_SxCR_MINC ;
		DMA2_Stream0->PAR = (uint32_t) &SPI1->DR ;
		DMA2_Stream0->M0AR = (uint32_t) rx ;
		DMA2_Stream0->NDTR = count ;
		DMA2->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0 ; // Write ones to clear bits
		SPI1->CR2 |= SPI_CR2_RXDMAEN ;
		DMA2_Stream0->CR |= DMA_SxCR_EN ;		// Enable DMA
	}
	if ( tx )
	{
		DMA2_Stream3->M0AR = (uint32_t) tx ;
	}
	else
	{
		DMA2_Stream3->M0AR = (uint32_t) rx ;
	}
	DMA2_Stream3->NDTR = count ;
	DMA2_Stream3->CR = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0 | DMA_SxCR_PL_0 
											  | DMA_SxCR_MINC | DMA_SxCR_DIR_0 ;
	DMA2_Stream3->PAR = (uint32_t) &SPI1->DR ;
	DMA2->LIFCR = DMA_LIFCR_CTCIF3 | DMA_LIFCR_CHTIF3 | DMA_LIFCR_CTEIF3 | DMA_LIFCR_CDMEIF3 | DMA_LIFCR_CFEIF3 ; // Write ones to clear bits
	SPI1->CR2 |= SPI_CR2_TXDMAEN ;
	DMA2_Stream3->CR |= DMA_SxCR_EN ;		// Enable DMA
	if ( tx )
	{
		DMA2_Stream3->CR |= DMA_SxCR_TCIE ;
	}
	else
	{
		DMA2_Stream0->CR |= DMA_SxCR_TCIE ;
	}
	// Wait for things to get started, avoids early interrupt
//	for ( count = 0 ; count < 1000 ; count += 1 )
//	{
//		if ( ( spiptr->SPI_SR & SPI_SR_TXEMPTY ) == 0 )
//		{
//			break ;			
//		}
//	}
	return 0 ;
}

extern "C" void DMA2_Stream0_IRQHandler()
{
//#ifdef PCB9XT
//	txmit('p') ;
//	p4hex( DMA2_Stream0->NDTR ) ;
//#endif
	DMA2_Stream3->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	DMA2_Stream0->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	DMA2_Stream0->CR &= ~DMA_SxCR_TCIE ;		// Stop interrupt
	SPI1->CR2 &= ~SPI_CR2_TXDMAEN & ~SPI_CR2_RXDMAEN ;
	Spi_complete = 1 ;
	GPIOA->BSRRL = GPIO_Pin_SPI_EE_CS ;		// output disable
}

uint16_t SpiTxeTime ;
uint16_t SpiBsyTime ;

extern "C" void DMA2_Stream3_IRQHandler()
{
#ifdef PCB9XT
	if ( DMA2_Stream3->NDTR )
	{
		DMA2->LIFCR = DMA_LIFCR_CTCIF3 | DMA_LIFCR_CFEIF3 ; // Write ones to clear bits
		DMA2_Stream3->CR |= DMA_SxCR_EN ;		// Enable DMA
		return ;
	}
	// Wait for TXE=1
	uint32_t i ;
	i = 0 ;
	while ( ( SPI1->SR & SPI_SR_TXE ) == 0 )
	{
		if ( ++i > 10000 )
		{
			break ;		// Software timeout				
		}
	}
	SpiTxeTime = i ;
	// Then wait for BSY == 0
	i = 0 ;
	while ( SPI1->SR & SPI_SR_BSY )
	{
		if ( ++i > 10000 )
		{
			break ;		// Software timeout				
		}
	}
	SpiBsyTime = i ;
//	txmit('q') ;
//	p4hex( DMA2_Stream3->NDTR ) ;
#endif
	DMA2_Stream3->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	DMA2_Stream3->CR &= ~DMA_SxCR_TCIE ;		// Stop interrupt
	SPI1->CR2 &= ~SPI_CR2_TXDMAEN & ~SPI_CR2_RXDMAEN ;
	Spi_complete = 1 ;
	GPIOA->BSRRL = GPIO_Pin_SPI_EE_CS ;		// output disable
}
#endif // PCB9XT

// Backlight driver
// Reset/sync, >50uS low
// 0 bit  High - 0.5uS   Low - 2.0uS
// 1 bit  High - 1.2uS   Low - 1.3uS
// All +/-150nS
// Timer 4, channel 4, PortB bit 9
// Also needs Timer 4 channel 1 to cause DMA transfer
// Timer 4 counts up to 2.5uS then resets, channel 4 outputs signal
// Channel 1 requests DMA to channel 4 at 2uS
// At end of DMA, need to work out when to stop timer.
// Maybe set channel 4 compare to past 2.5uS
// DMA1, stream 0, channel 2, triggered by TIM4 CH1

// Testing on a X9D plus, on Ext PPM out PA7
// TIM2, ch2 and TIM2 Ch4 for DMA on CH3, Stream1

#ifdef PCB9XT

uint32_t BlLastLevel ;
uint16_t BlData[27] ;
uint8_t BlColours[3] ;
uint8_t BlChanged ;
uint8_t BlCount ;
uint8_t BlSending ;


// The value is 24 bits, 8 red, 8 green, 8 blue right justified
void backlightSet( uint32_t value )
{
	uint16_t *blptr = BlData ;
	uint32_t i ;
	value <<= 8 ;
	for ( i = 0 ; i < 24 ; i += 1 )
	{
//		*blptr++ = ( value & 0x80000000 ) ? 12 : 5 ;
		*blptr++ = ( value & 0x80000000 ) ? 6 : 3 ;
		value <<= 1 ;
	}
	*blptr++ = 40 ;
	*blptr++ = 40 ;
	*blptr = 40 ;
}

void BlSetAllColours( uint32_t rlevel, uint32_t glevel, uint32_t blevel )
{
	if ( rlevel > 100 )
	{ // A GVAR
		int32_t x ;
		x = g_model.gvars[(uint8_t)rlevel-101].gvar ;
		if ( x < 0 )
		{
			x = 0 ;
		}
		if ( x > 100 )
		{
			x = 100 ;
		}
		rlevel = x ;		
	}
	rlevel *= 255 ;
	rlevel /= 100 ;
	if ( rlevel > 255 )
	{
		rlevel = 255 ;
	}
	BlColours[BL_RED] = rlevel ;

	if ( glevel > 100 )
	{ // A GVAR
		int32_t x ;
		x = g_model.gvars[(uint8_t)glevel-101].gvar ;
		if ( x < 0 )
		{
			x = 0 ;
		}
		if ( x > 100 )
		{
			x = 100 ;
		}
		glevel = x ;		
	}
	glevel *= 255 ;
	glevel /= 100 ;
	if ( glevel > 255 )
	{
		glevel = 255 ;
	}
	BlColours[BL_GREEN] = glevel ;
	BlSetColour( blevel, BL_BLUE ) ;
}

// Level is 0 to 100%
// colour is 0 red, 1 green, 2 blue, 3 all
void BlSetColour( uint32_t level, uint32_t colour )
{
	if ( colour > 3 )
	{
		return ;
	}
	if ( level > 100 )
	{ // A GVAR
		int32_t x ;
		x = g_model.gvars[(uint8_t)level-101].gvar ;
		if ( x < 0 )
		{
			x = 0 ;
		}
		if ( x > 100 )
		{
			x = 100 ;
		}
		level = x ;		
	}
	level *= 255 ;
	level /= 100 ;
	if ( level > 255 )
	{
		level = 255 ;
	}
	if ( colour == 3 )
	{
		BlColours[0] = level ;
		BlColours[1] = level ;
		BlColours[2] = level ;
	}
	else
	{
		BlColours[colour] = level ;
	}

	level = (BlColours[0] << 16 ) | (BlColours[1] << 8 ) | (BlColours[2] ) ;
	if ( level != BlLastLevel )
	{
		BlLastLevel = level ;
		BlChanged = 1 ;
		if ( BlSending == 0 )
		{
			backlightSend() ;
		}		 
	}
}

void backlightSend()
{
	BlSending = 1 ;
	if ( BlChanged )
	{
		backlightSet( BlLastLevel ) ;
		BlChanged = 0 ;
		BlCount = 2 ;
	}

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ;           // Enable portB clock
  configure_pins( GPIO_Pin_9, PIN_PERIPHERAL | PIN_PORTB | PIN_PER_2 | PIN_OS25 | PIN_PUSHPULL ) ;
	
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN ;		// Enable clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN ;		// Enable DMA1 clock

  TIM4->CR1 &= ~TIM_CR1_CEN ;
	TIM4->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 10000000 - 1 ;		// 0.1uS
//  TIM4->ARR = 24 ;             // 2.5uS
//  TIM4->CCR1 = 19 ;            // Update time
  TIM4->ARR = 12 ;             // 1.3uS
  TIM4->CCR1 = 9 ;            // Update time
	TIM4->CCER = TIM_CCER_CC4E ;
	TIM4->CNT = 65536-710 ;
  TIM4->CCR4 = BlData[0] ;		// Past end
	TIM4->DIER |= TIM_DIER_CC1DE ;          // Enable DMA on CC1 match
	TIM4->CCMR2 = TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 ;                     // Force O/P high

  // Enable the DMA channel here, DMA1 stream 0, channel 2
  DMA1_Stream0->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  DMA1->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0 ; // Write ones to clear bits
  DMA1_Stream0->CR = DMA_SxCR_CHSEL_1 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0 
                 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 ;// | DMA_SxCR_PFCTRL ;
  DMA1_Stream0->PAR = (uint32_t)(&TIM4->CCR4);
  DMA1_Stream0->M0AR = (uint32_t)(&BlData[1]);
	DMA1_Stream0->NDTR = 26 ;
  DMA1_Stream0->CR |= DMA_SxCR_EN ;               // Enable DMA

  TIM4->SR &= ~TIM_SR_CC1IF ;                             // Clear flag
  TIM4->CR1 |= TIM_CR1_CEN ;
	DMA1_Stream0->CR |= DMA_SxCR_TCIE ;
	NVIC_SetPriority( DMA1_Stream0_IRQn, 5 ) ; // Lower priority interrupt
  NVIC_EnableIRQ( DMA1_Stream0_IRQn ) ;
}

//void backlightReset()
//{
//  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ;           // Enable portB clock
//  configure_pins( GPIO_Pin_9, PIN_OUTPUT | PIN_PORTB | PIN_OS25 | PIN_PUSHPULL ) ;
//	GPIOB->BSRRL = GPIO_Pin_9 ;		// output high
//	hw_delay( 500 ) ; // 50uS
//	GPIOB->BSRRH = GPIO_Pin_9 ;		// output low
//	hw_delay( 500 ) ; // 50uS
//	GPIOB->BSRRL = GPIO_Pin_9 ;		// output high
//	hw_delay( 500 ) ; // 50uS
//}

extern "C" void DMA1_Stream0_IRQHandler()
{
	if ( --BlCount )
	{
		backlightSend() ;
		return ;
	}
	DMA1_Stream0->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	DMA1_Stream0->CR &= ~DMA_SxCR_TCIE ;		// Stop interrupt
  TIM4->CR1 &= ~TIM_CR1_CEN ;

	BlSending = 0 ;
	if ( BlChanged )
	{
		backlightSend() ;
	}
}


#endif // PCB9XT

