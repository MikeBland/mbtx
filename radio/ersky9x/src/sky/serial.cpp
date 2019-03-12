/*
 * Author - Mike Blandford
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

#include "AT91SAM3S4.h"

#ifndef SIMU
#include "core_cm3.h"
#endif
#include "ersky9x.h"
#include "myeeprom.h"
#include "drivers.h"
#include "logicio.h"


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

// 9600 baud, bit time 104.16uS
#define BIT_TIME_9600		208
// 19200 baud, bit time 52.08
#define BIT_TIME_19200	104
// 38400 baud, bit time 26.04
#define BIT_TIME_38400	52
// 57600 baud, bit time 17.36uS
#define BIT_TIME_57600	35
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


extern struct t_serial_tx *Current_Com1 ;
extern struct t_serial_tx *Current_Com2 ;
extern struct t_telemetryTx TelemetryTx ;

uint16_t USART_ERRORS ;
uint16_t USART_FE ;
uint16_t USART_PE ;

#ifdef REVX
struct t_16bit_fifo32 Jeti_fifo ;
uint8_t JetiTxBuffer[16] ;
void put_16bit_fifo32( struct t_16bit_fifo32 *pfifo, uint16_t word ) ;
int32_t get_16bit_fifo32( struct t_16bit_fifo32 *pfifo ) ;
#endif

void putCaptureTime( struct t_softSerial *pss, uint16_t time, uint32_t value ) ;

uint8_t LastReceivedSportByte ;

#ifdef BLUETOOTH
 #ifdef BT_PDC
struct t_rxUartBuffer BtPdcFifo ;
void startPdcBtReceive() ;
static int32_t rxPdcBt() ;
 #else
struct t_fifo128 BtRx_fifo ;
 #endif
#endif

void startPdcBtReceive() ;

/**
 * Configures a UART peripheral with the specified parameters.
 *
 * baudrate  Baudrate at which the UART should operate (in Hz).
 * masterClock  Frequency of the system master clock (in Hz).
 * uses PA9 and PA10, RXD2 and TXD2
 */
void UART_Configure( uint32_t baudrate, uint32_t masterClock)
{
  register Uart *pUart = CONSOLE_USART ;

  /* Configure PIO */
	configure_pins( (PIO_PA9 | PIO_PA10), PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP ) ;

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

	pUart->UART_TCR = 0 ;
	pUart->UART_IDR = UART_IDR_TXBUFE ;
  
	/* Enable receiver and transmitter */
  pUart->UART_CR = UART_CR_RXEN | UART_CR_TXEN;
  pUart->UART_IER = UART_IER_RXRDY ;

	NVIC_SetPriority( UART0_IRQn, 2 ) ; // Lower priority interrupt
	NVIC_EnableIRQ(UART0_IRQn) ;

}

void com2_Configure( uint32_t baudrate, uint32_t invert, uint32_t parity )
{
	if ( invert )
	{
		CONSOLE_USART->UART_IDR = UART_IDR_RXRDY ;
		init_software_com2( baudrate, SERIAL_INVERT, parity ) ;
	}
	else
	{
		UART_Configure( baudrate, Master_frequency) ;
  	register Uart *pUart = CONSOLE_USART ;
		if ( parity )
		{
  		pUart->UART_MR =  0 ;  // NORMAL, Even Parity, 8 bit
		}
		else
		{
			pUart->UART_MR = 0x800 ;  // NORMAL, No Parity
		}
	}
}

//// Set up COM2 for SBUS (8E2), can't set 2 stop bits!
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

uint32_t txPdcCom2( struct t_serial_tx *data )
{
	Uart *pUart=CONSOLE_USART ;
		
	if ( pUart->UART_TCR == 0 )
	{
		Current_Com2 = data ;
		data->ready = 1 ;
#ifndef SIMU
	  pUart->UART_TPR = (uint32_t)data->buffer ;
#endif
		pUart->UART_TCR = data->size ;
		pUart->UART_PTCR = UART_PTCR_TXTEN ;
		pUart->UART_IER = UART_IER_TXBUFE ;
		NVIC_SetPriority( UART0_IRQn, 2 ) ; // Lower priority interrupt
		NVIC_EnableIRQ(UART0_IRQn) ;
		return 1 ;			// Sent OK
	}
	return 0 ;				// Busy
	
}

uint32_t txPdcCom1( struct t_serial_tx *data )
{
  register Usart *pUsart = SECOND_USART;

	if ( pUsart->US_TCR == 0 )
	{
		Current_Com1 = data ;
		data->ready = 1 ;
#ifndef SIMU
	  pUsart->US_TPR = (uint32_t)data->buffer ;
#endif
		pUsart->US_TCR = data->size ;
		pUsart->US_PTCR = US_PTCR_TXTEN ;
		pUsart->US_IER = US_IER_TXBUFE ;
		NVIC_SetPriority( USART0_IRQn, 3 ) ; // Quite high priority interrupt
		NVIC_EnableIRQ(USART0_IRQn) ;
		return 1 ;
	}
	return 0 ;
}

extern "C" void UART0_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x8B ;
#else
	RTC->BKP1R = 0x8B ;
#endif
#endif
	Uart *pUart=CONSOLE_USART ;
	if ( pUart->UART_IMR & UART_IMR_TXBUFE )
	{
		if ( pUart->UART_SR & UART_SR_TXBUFE )
		{
			pUart->UART_IDR = UART_IDR_TXBUFE ;
			pUart->UART_PTCR = UART_PTCR_TXTDIS ;
			Current_Com2->ready = 0 ;	
		}
	}
	if ( pUart->UART_SR & UART_SR_RXRDY )
	{
		uint8_t chr = CONSOLE_USART->UART_RHR ;
		if ( ( g_model.com2Function == COM2_FUNC_SBUSTRAIN ) || ( g_model.com2Function == COM2_FUNC_SBUS57600 ) )
		{
			put_fifo64( &Sbus_fifo, chr ) ;	
		}
#ifdef BLUETOOTH
		else if ( g_model.com2Function == COM2_FUNC_BTDIRECT )	// BT <-> COM2
		{
			telem_byte_to_bt( chr ) ;
		}
#endif
		else
		{
			put_fifo128( &Com2_fifo, chr ) ;	
#ifdef BLUETOOTH
			if ( g_model.com2Function == COM2_FUNC_TEL_BT2WAY )	// BT <-> COM2
			{
				telem_byte_to_bt( chr ) ;
			}
#endif
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
#ifdef BT_PDC
	startPdcBtReceive() ;
	NVIC_SetPriority( UART1_IRQn, 5 ) ; // Lower priority interrupt
#else
  pUart->UART_IER = UART_IER_RXRDY ;
	NVIC_SetPriority( UART1_IRQn, 3 ) ; // Lower priority interrupt
#endif
	NVIC_EnableIRQ(UART1_IRQn) ;

}

#ifdef BLUETOOTH
int32_t rxBtuart()
{
#ifdef BT_PDC
	return rxPdcBt() ;
#else	
	return get_fifo128( &BtRx_fifo ) ;
#endif
}
#endif

#ifdef BT_PDC
void startPdcBtReceive()
{
  register Uart *pUart = BT_USART;
	BtPdcFifo.outPtr = BtPdcFifo.fifo ;
#ifndef SIMU
	pUart->UART_RPR = (uint32_t)BtPdcFifo.fifo ;
	pUart->UART_RNPR = (uint32_t)BtPdcFifo.fifo ;
#endif
	pUart->UART_RCR = RX_UART_BUFFER_SIZE ;
	pUart->UART_RNCR = RX_UART_BUFFER_SIZE ;
	pUart->UART_PTCR = US_PTCR_RXTEN ;
}

void endPdcUsartReceive()
{
  register Uart *pUart = BT_USART;

	pUart->UART_PTCR = UART_PTCR_RXTDIS ;
}

static int32_t rxPdcBt()
{
#if !defined(SIMU)
  register Uart *pUart = BT_USART;
	uint8_t *ptr ;
	uint8_t *endPtr ;
  int32_t chr ;

 //Find out where the DMA has got to
	endPtr = (uint8_t *)pUart->UART_RPR ;
	// Check for DMA passed end of buffer
	if ( endPtr > &BtPdcFifo.fifo[RX_UART_BUFFER_SIZE-1] )
	{
		endPtr = BtPdcFifo.fifo ;
	}

	ptr = BtPdcFifo.outPtr ;
	if ( ptr != endPtr )
	{
		chr = *ptr++ ;
		if ( ptr > &BtPdcFifo.fifo[RX_UART_BUFFER_SIZE-1] )		// last byte
		{
			ptr = BtPdcFifo.fifo ;
		}
	 	BtPdcFifo.outPtr = ptr ;

	 	if ( pUart->UART_RNCR == 0 )
	 	{
	 		pUart->UART_RNPR = (uint32_t)BtPdcFifo.fifo ;
	 		pUart->UART_RNCR = RX_UART_BUFFER_SIZE ;
	 	}
	}
	else
	{
		return -1 ;
	}
#endif
	return chr ;
}
#endif

////void Bt_UART_Stop()
////{
////  BT_USART->UART_IDR = UART_IDR_RXRDY ;
////	NVIC_DisableIRQ(UART1_IRQn) ;
////}


// USART0 configuration, we will use this for FrSky etc
// Work in Progress, UNTESTED
// Uses PA5 and PA6 (RXD and TXD)
static void UART2_Configure( uint32_t baudrate, uint32_t masterClock)
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
	pUsart->US_IER = US_IER_RXRDY ;

	NVIC_SetPriority( USART0_IRQn, 2 ) ; // Quite high priority interrupt
	NVIC_EnableIRQ(USART0_IRQn) ;
	
}

void com1_Configure( uint32_t baudrate, uint32_t invert, uint32_t parity )
{
	if ( invert )
	{
		SECOND_USART->US_IDR = US_IDR_RXRDY ;
		init_software_com1( baudrate, SERIAL_INVERT, parity ) ;
	}
	else
	{
		UART2_Configure( baudrate, Master_frequency ) ;	
  	register Usart *pUsart = SECOND_USART;
		if ( parity )
		{
  		pUsart->US_MR =  0x000000C0 ;  // NORMAL, Even Parity, 8 bit
		}
		else
		{
		  pUsart->US_MR =  0x000008C0 ;  // NORMAL, No Parity, 8 bit
		}
//		com1Parity( parity ) ;
//		com1_timeout_disable() ;
	}
}

void UART2_9dataOdd1stop()
{
	
}

// This is for Com 1
extern "C" void USART0_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x8C ;
#else
	RTC->BKP1R = 0x8C ;
#endif
#endif
  register Usart *pUsart = SECOND_USART;
	uint32_t status ;

#ifdef REVX
	if ( g_model.bt_telemetry > 1 )
	{
		if ( pUsart->US_IMR & US_IMR_TXBUFE )
		{
			if ( pUsart->US_CSR & US_CSR_TXBUFE )
			{
				pUsart->US_IDR = US_IDR_TXBUFE ;
				pUsart->US_PTCR = US_PTCR_TXTDIS ;
				Current_Com1->ready = 0 ;	
			}
		}
	}

	if ( (pUsart->US_MR & 0x0000000F) == 0x0000000E )
	{
////		if ( pUsart->US_IMR & US_IMR_TXRDY )
////		{
////			if ( pUsart->US_CSR & US_CSR_TXRDY )
////			{
////				Sstat1 = pUsart->US_CSR ;
////				Sstat2 = pUsart->US_IMR ;
////  			pUsart->US_IDR = US_IDR_TXRDY ;
////  			pUsart->US_IER = US_IER_TXEMPTY ;
////				pUsart->US_THR = SPI2ndByte ;
////				Scount1 += 1 ;
////			}
////		}
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
		if ( pUsart->US_CSR & US_CSR_TIMEOUT )
		{
			pUsart->US_RTOR = 0 ;		// Off
			pUsart->US_IDR = US_IDR_TIMEOUT ;
			txPdcUsart( TelemetryTx.SportTx.ptr, TelemetryTx.sportCount, 0 ) ;
		}
		
//	  pUsart->US_CR = US_CR_STTTO ;		// Clears timeout bit
//		DsmRxTimeout = 1 ;
	}
  if ( pUsart->US_IMR & US_IMR_ENDTX )
	{
	 	if ( pUsart->US_CSR & US_CSR_ENDTX )
		{
			TelemetryTx.sportCount = 0 ;
			pUsart->US_IER = US_IER_TXEMPTY ;
			pUsart->US_IDR = US_IDR_ENDTX ;
		}
	}
// Disable Tx output
  if ( pUsart->US_IMR & US_IMR_TXEMPTY )
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
	
	status = pUsart->US_CSR ;
	if ( status & US_CSR_RXRDY )
	{
    uint8_t data = pUsart->US_RHR ;
		if ( status & US_CSR_RXBRK )
		{
			if ( data )
			{
				put_fifo128( &Com1_fifo, data ) ;
			}
			pUsart->US_CR = US_CR_RSTSTA ;
		}
		else
		{
			put_fifo128( &Com1_fifo, data ) ;
		}

// 0x00, 0xA1, 0x22, 0x83, 0xE4, 0x45, 0xC6, 0x67, 0x48, 0xE9, 0x6A, 0xCB, 0xAC, 0x0D,
// 0x8E, 0x2F, 0xD0, 0x71, 0xF2, 0x53, 0x34, 0x95, 0x16, 0xB7, 0x98, 0x39, 0xBA, 0x1B
		
		if ( LastReceivedSportByte == 0x7E && TelemetryTx.sportCount > 0 && data == TelemetryTx.SportTx.index )
		{
			pUsart->US_RTOR = 60 ;		// Bits @ 57600 ~= 1050uS
			pUsart->US_CR = US_CR_RETTO | US_CR_STTTO ;
			pUsart->US_IER = US_IER_TIMEOUT ;
			TelemetryTx.SportTx.index = 0x7E ;
#ifdef REVX
			if ( g_model.Module[1].protocol == PROTO_MULTI )
			{
				PIOA->PIO_SODR = 0x02000000L ;	// Set bit A25 ON, enable SPort output
			}
#endif
    }
    LastReceivedSportByte = data ;
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



//void startPdcUsartReceive()
//{
//  register Usart *pUsart = SECOND_USART;
	
//	TelemetryInBuffer.outPtr = TelemetryInBuffer.fifo ;
//#ifndef SIMU
//	pUsart->US_RPR = (uint32_t)TelemetryInBuffer.fifo ;
//	pUsart->US_RNPR = (uint32_t)TelemetryInBuffer.fifo ;
//#endif
//	pUsart->US_RCR = RX_UART_BUFFER_SIZE ;
//	pUsart->US_RNCR = RX_UART_BUFFER_SIZE ;
//	pUsart->US_PTCR = US_PTCR_RXTEN ;
//}

//void endPdcUsartReceive()
//{
//  register Usart *pUsart = SECOND_USART;
	
//	pUsart->US_PTCR = US_PTCR_RXTDIS ;
//}

//void rxPdcUsart( void (*pChProcess)(uint8_t x) )
//{
//#if !defined(SIMU)
//  register Usart *pUsart = SECOND_USART;
//	uint8_t *ptr ;
//	uint8_t *endPtr ;

// //Find out where the DMA has got to
//	endPtr = (uint8_t *)pUsart->US_RPR ;
//	// Check for DMA passed end of buffer
//	if ( endPtr > &TelemetryInBuffer.fifo[RX_UART_BUFFER_SIZE-1] )
//	{
//		endPtr = TelemetryInBuffer.fifo ;
//	}
	
//	ptr = TelemetryInBuffer.outPtr ;
//	while ( ptr != endPtr )
//	{
//		(*pChProcess)(*ptr++) ;
//		if ( ptr > &TelemetryInBuffer.fifo[RX_UART_BUFFER_SIZE-1] )		// last byte
//		{
//			ptr = TelemetryInBuffer.fifo ;
//		}
//	}
//	TelemetryInBuffer.outPtr = ptr ;

//	if ( pUsart->US_RNCR == 0 )
//	{
//		pUsart->US_RNPR = (uint32_t)TelemetryInBuffer.fifo ;
//		pUsart->US_RNCR = RX_UART_BUFFER_SIZE ;
//	}
//#endif
//}

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


#ifdef BLUETOOTH
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
		NVIC_SetPriority( USART1_IRQn, 3 ) ; // Lower priority interrupt
		NVIC_EnableIRQ(UART1_IRQn) ;
		return 1 ;			// Sent OK
	}
	return 0 ;				// Busy
}
#endif

//void end_bt_tx_interrupt()
//{
//  Uart *pUart=BT_USART ;
//	pUart->UART_IDR = UART_IDR_TXBUFE ;
//	NVIC_DisableIRQ(UART1_IRQn) ;
//}

extern "C" void UART1_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x8D ;
#else
	RTC->BKP1R = 0x8D ;
#endif
#endif
  Uart *pUart=BT_USART ;
	if ( pUart->UART_SR & UART_SR_TXBUFE )
	{
		pUart->UART_IDR = UART_IDR_TXBUFE ;
		pUart->UART_PTCR = US_PTCR_TXTDIS ;
		Current_bt->ready = 0 ;	
	}
#ifndef BT_PDC
	if ( pUart->UART_SR & UART_SR_RXRDY )
	{
		put_fifo128( &BtRx_fifo, pUart->UART_RHR ) ;	
	}
#endif
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

//// Outputs a string to the UART

//uint16_t rxCom2()
//{
//	return get_fifo128( &Com2_fifo ) ;
//}

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

//void txmitBt( uint8_t c )
//{
//  Uart *pUart=BT_USART ;
//	uint32_t x ;

//	/* Wait for the transmitter to be ready */
//	x = 10000 ;
//  while ( (pUart->UART_SR & UART_SR_TXEMPTY) == 0 )
//	{
//		if ( --x == 0 )
//		{
//			break ;			// Timeout so we don't hang
//		}
//	}
//  /* Send character */
//  pUart->UART_THR=c ;
//}

//int32_t rxBtuart()
//{
//	return get_fifo128( &BtRx_fifo ) ;
//}


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
	TC1->TC_CHANNEL[2].TC_CCR = 2 ;		// Disable clock
	NVIC_DisableIRQ(TC5_IRQn) ;
}

void init_software_com(uint32_t baudrate, uint32_t invert, uint32_t parity, uint32_t port )
{
	struct t_softSerial *pss = &SoftSerial1 ;
	if ( port == 0 )
	{
		pss->softwareComBit = PIO_PA5 ;
		pss->pfifo = &Com1_fifo ;
	}
	else
	{
		pss->softwareComBit = PIO_PA9 ;
		pss->pfifo = &Com2_fifo ;
	}
	pss->bitTime = 2000000 / baudrate ;

	pss->softSerInvert = invert ? 0 : pss->softwareComBit ;
	pss->softSerialEvenParity = parity ? 1 : 0 ;

	TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_LDRAS ;		// No int on rising edge
	TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_LDRBS ;		// No int on falling edge
	TC1->TC_CHANNEL[0].TC_IDR = TC_IDR0_CPCS ;		// No compare interrupt
	pss->lineState = LINE_IDLE ;
	CaptureMode = CAP_COM1 ;
	configure_pins( pss->softwareComBit, PIN_ENABLE | PIN_INPUT | PIN_PORTA ) ;
	PIOA->PIO_IER = pss->softwareComBit ;
	NVIC_SetPriority( PIOA_IRQn, 0 ) ; // Highest priority interrupt
	NVIC_EnableIRQ(PIOA_IRQn) ;
	start_timer5() ;
}

// Handle software serial on COM1 input (for non-inverted input)
void init_software_com1(uint32_t baudrate, uint32_t invert, uint32_t parity)
{
	init_software_com(baudrate, invert, parity, 0 ) ;
}

static void disable_software_com( uint32_t bit)
{
	stop_timer5() ;
	CaptureMode = CAP_PPM ;
	PIOA->PIO_IDR = bit ;
	NVIC_DisableIRQ(PIOA_IRQn) ;
}

void disable_software_com1()
{
	disable_software_com( PIO_PA5 ) ;
}

void init_software_com2(uint32_t baudrate, uint32_t invert, uint32_t parity)
{
	init_software_com(baudrate, invert, parity, 1 ) ;
}

void disable_software_com2()
{
	disable_software_com( PIO_PA9 ) ;
}

extern "C" void PIOA_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x8E ;
#else
	RTC->BKP1R = 0x8E ;
#endif
#endif
  register uint32_t capture ;
  register uint32_t dummy ;
	
	capture =  TC1->TC_CHANNEL[0].TC_CV ;	// Capture time
	struct t_softSerial *pss = &SoftSerial1 ;
	dummy = PIOA->PIO_ISR ;			// Read and clear status register
	(void) dummy ;		// Discard value - prevents compiler warning

	dummy = PIOA->PIO_PDSR ;
	if ( ( dummy & pss->softwareComBit ) == pss->softSerInvert )
	{
		// L to H transition
		pss->LtoHtime = capture ;
		TC1->TC_CHANNEL[2].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
		TC1->TC_CHANNEL[2].TC_RC = pss->bitTime * 10 ;
		uint32_t time ;
		capture -= pss->HtoLtime ;
		time = capture ;
		putCaptureTime( pss, time, 0 ) ;
		TC1->TC_CHANNEL[2].TC_IER = TC_IER0_CPCS ;		// Compare interrupt
		(void) TC1->TC_CHANNEL[2].TC_SR ;
	}
	else
	{
		// H to L transition
		pss->HtoLtime = capture ;
		if ( pss->lineState == LINE_IDLE )
		{
			pss->lineState = LINE_ACTIVE ;
//			putCaptureTime( pss, 0, 3 ) ;
			TC1->TC_CHANNEL[2].TC_RC = capture + (pss->bitTime * 20) ;
			(void) TC1->TC_CHANNEL[2].TC_SR ;
		}
		else
		{
			uint32_t time ;
			capture -= pss->LtoHtime ;
			time = capture ;
			putCaptureTime( pss, time, 1 ) ;
		}
		TC1->TC_CHANNEL[2].TC_IDR = TC_IDR0_CPCS ;		// No compare interrupt
	}
}

extern "C" void TC5_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x91 ;
#else
	RTC->BKP1R = 0x91 ;
#endif
#endif
	uint32_t status ;
	struct t_softSerial *pss = &SoftSerial1 ;

	status = TC1->TC_CHANNEL[2].TC_SR ;
	if ( status & TC_SR0_CPCS )
	{		
		uint32_t time ;
		time = TC1->TC_CHANNEL[0].TC_CV - pss->LtoHtime ;
		putCaptureTime( pss, time, 2 ) ;
		pss->lineState = LINE_IDLE ;
		TC1->TC_CHANNEL[2].TC_IDR = TC_IDR0_CPCS ;		// No compare interrupt
	}
}


