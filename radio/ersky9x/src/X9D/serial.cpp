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

#if defined(PCBX9D) || defined(PCB9XT)
#include "X9D/stm32f2xx.h"
//#include "X9D/stm32f2xx_gpio.h"
#include "X9D/stm32f2xx_rcc.h"
#include "X9D/stm32f2xx_usart.h"
#include "X9D/hal.h"
//#include "timers.h"
#endif

#if defined(PCBX12D) || defined(PCBX10)
#include "X12D/stm32f4xx.h"
#include "X12D/stm32f4xx_usart.h"
//#include "X12D/stm32f4xx_gpio.h"
#include "X12D/stm32f4xx_rcc.h"
#include "X12D/hal.h"
#endif

#include "ersky9x.h"
#include "myeeprom.h"
#include "drivers.h"
#include "logicio.h"
#include "frsky.h"
#include "timers.h"

#define USART_FLAG_ERRORS (USART_FLAG_ORE | USART_FLAG_FE | USART_FLAG_PE)

// States in LineState
#define LINE_IDLE			0
#define LINE_ACTIVE		1

// States in BitState
#define BIT_IDLE			0
#define BIT_ACTIVE		1
#define BIT_FRAMING		2

extern struct t_telemetryTx TelemetryTx ;
uint16_t USART_ERRORS ;
uint16_t USART_ORE ;
uint16_t USART_NE ;
uint16_t USART_FE ;
uint16_t USART_PE ;

#ifdef PCB9XT
uint16_t USART1_ORE ;
uint16_t USART2_ORE ;
#endif 

#ifndef PCBX12D
 #ifdef PCBX9D
  #ifndef PCBXLITE
   #ifndef PCBX9LITE
    #ifndef PCBX10
#define XJT_HEARTBEAT_BIT	0x0080		// PC7
    #endif 
   #endif 
  #endif 
 #endif 
#endif 

#if defined(PCBX12D) || defined(PCBX10)
#define XJT_HEARTBEAT_BIT	0x1000		// PD12
#endif 

#ifdef PCBX7
extern struct t_serial_tx *Current_Com3 ;
#endif 

#if defined(PCBX9LITE) && defined(X9LS)
extern struct t_serial_tx *Current_Com3 ;
#endif 

#ifdef PCB9XT
extern struct t_serial_tx *Current_Com3 ;
extern struct t_fifo128 Com3_fifo ;
extern struct t_fifo64 RemoteRx_fifo ;
#endif 

void putCaptureTime( struct t_softSerial *pss, uint16_t time, uint32_t value ) ;

uint8_t LastReceivedSportByte ;

#ifdef PCBX9LITE
#define EXTERNAL_RF_ON()      GPIO_SetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#define EXTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#endif // X3

#if defined(PCBX12D) || defined(PCBX10)
extern struct t_serial_tx *Current_Com6 ;
#endif

extern struct t_serial_tx *Current_Com2 ;

#ifndef PCBX12D
#ifndef PCBX10
void USART6_Sbus_configure()
{
 #ifdef PCBX9D
  #ifndef PCBX9LITE
	stop_xjt_heartbeat() ;
  #endif // X3
 #endif
#ifdef PCBX9LITE
	if ( g_model.Module[EXTERNAL_MODULE].protocol == PROTO_PXX )
	{
		return ;	// USART6 in use
	}
#endif // X3
	RCC->APB2ENR |= RCC_APB2ENR_USART6EN ;		// Enable clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
	configure_pins( 0x0080, PIN_PERIPHERAL | PIN_PORTC | PIN_PER_8 ) ;
	USART6->BRR = PeripheralSpeeds.Peri2_frequency / 100000 ;
	USART6->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE ;
	USART6->CR2 = 0 ;
	USART6->CR3 = 0 ;
	(void) USART6->DR ;
	NVIC_SetPriority( USART6_IRQn, 5 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(USART6_IRQn) ;

#ifdef PCBX9LITE
	EXTERNAL_RF_ON() ;
#endif // X3
}

void stop_USART6_Sbus()
{
#ifdef PCBX9LITE
	if ( g_model.Module[EXTERNAL_MODULE].protocol == PROTO_PXX )
	{
		return ;	// USART6 in use
	}
	EXTERNAL_RF_OFF() ;
#endif // X3
	
	configure_pins( 0x0080, PIN_INPUT | PIN_PORTC ) ;
  NVIC_DisableIRQ(USART6_IRQn) ;
 #ifdef PCBX9D
  #ifndef PCBX9LITE
	init_xjt_heartbeat() ;
  #endif // X3
 #endif
}

#ifndef PCBX9LITE
 #ifndef PCBXLITE
extern "C" void USART6_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x94 ;
#else
	RTC->BKP1R = 0x94 ;
#endif
#endif
	put_fifo64( &Sbus_fifo, USART6->DR ) ;	
}
 #endif // Xlite
#endif // X3


#endif // #ifndef PCBX10
#endif // #ifndef PCBX12D


//#if defined(PCBX12D) || defined(PCBX10)
#if defined(PCBX12D)
void USART6_configure()
{
	if ( isProdVersion() == 0 )
	{
		RCC->AHB1ENR |= PROT_BT_RCC_AHB1Periph ;
		configure_pins( PROT_BT_EN_GPIO_PIN, PIN_OUTPUT | PIN_PORTA ) ;
		GPIOA->BSRRH = PROT_BT_EN_GPIO_PIN ;
	}
	else
	{
		RCC->AHB1ENR |= BT_RCC_AHB1Periph ;
		configure_pins( BT_EN_GPIO_PIN, PIN_PERIPHERAL | PIN_PORTI ) ;
		GPIOI->BSRRH = BT_EN_GPIO_PIN ;
	}
	RCC->APB2ENR |= RCC_APB2ENR_USART6EN ;		// Enable clock
	configure_pins( BT_TX_GPIO_PIN|BT_RX_GPIO_PIN, PIN_PERIPHERAL | PIN_PORTG | PIN_PER_8 ) ;
	configure_pins( BT_BRTS_GPIO_PIN, PIN_OUTPUT | PIN_PORTG ) ;
	GPIOG->BSRRL = BT_BRTS_GPIO_PIN ;
	USART6->BRR = PeripheralSpeeds.Peri2_frequency / 115200 ;
	USART6->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE ;
	USART6->CR2 = 0 ;
	USART6->CR3 = 0 ;
	(void) USART6->DR ;
	NVIC_SetPriority( USART6_IRQn, 5 ) ; // Lower priority interrupt
  NVIC_EnableIRQ(USART6_IRQn) ;
	
}

void USART6SetBaudrate( uint32_t baudrate )
{
	USART6->BRR = PeripheralSpeeds.Peri2_frequency / baudrate ;
}

#ifdef BLUETOOTH
uint32_t txPdcBt( struct t_serial_tx *data )
{
	data->ready = 1 ;
	Current_Com6 = data ;
	USART6->CR1 |= USART_CR1_TXEIE ;
	return 1 ;			// Sent OK
}
#endif

uint16_t Last6Rx ;
uint16_t Last6Status ;
uint16_t Last6Count ;

extern "C" void USART6_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x95 ;
#else
	RTC->BKP1R = 0x95 ;
#endif
#endif
  uint32_t status;
  uint8_t data;
	USART_TypeDef *puart = USART6 ;

  status = puart->SR ;
	Last6Status = status ;
	if ( ( status & USART_SR_TXE ) && (puart->CR1 & USART_CR1_TXEIE ) )
	{
		if ( Current_Com6 )
		{
			if ( Current_Com6->size )
			{
				puart->DR = *Current_Com6->buffer++ ;
				if ( --Current_Com6->size == 0 )
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
		Current_Com6->ready = 0 ;
	}
	
  while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS))
	{
    data = puart->DR ;
		Last6Rx = ( Last6Rx << 8 ) | data ;
		Last6Count += 1 ;

    if (!(status & USART_FLAG_ERRORS))
		{
#ifdef BLUETOOTH
			put_fifo128( &BtRx_fifo, data ) ;	
#else
			(void) data ;	
#endif
		}
		else
		{
			USART_ERRORS += 1 ;
			if ( status & USART_FLAG_ORE )
			{
				USART_ORE += 1 ;
			}
		}
    status = puart->SR ;
	}
}


#endif // #ifdef PCBX12D

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


void ConsoleInit()
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

void com2_Configure( uint32_t baudrate, uint32_t invert, uint32_t parity )
{
	ConsoleInit() ;
	USART3->BRR = PeripheralSpeeds.Peri1_frequency / baudrate ;
	com2Parity( parity ) ;
}

#endif // #ifndef PCB9XT

void com1_Configure( uint32_t baudRate, uint32_t invert, uint32_t parity )
{
	// Serial configure  
	if ( invert )
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
#if defined(PCBXLITE) || defined(REV19)
	GPIOD->BSRRL = PIN_SPORT_ON ;		// output disable
#else
 #ifdef PCBX9LITE
	x9LiteSportOff() ;
//	GPIOD->BSRRL = PIN_SPORT_ON ;		// output disable
 #else
	GPIOD->BSRRH = PIN_SPORT_ON ;		// output disable
 #endif
#endif
#endif
#ifdef PCB9XT
// PB2 as SPort enable
	configure_pins( 0x00000004, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB ) ;
	configure_pins( 0x00000008, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PER_7 | PIN_PORTA ) ;
	configure_pins( 0x00000004, PIN_PERIPHERAL | PIN_PER_7 | PIN_PORTA ) ;
#else

#if defined(PCBXLITE) || defined(REV19)
	configure_pins( PIN_SPORT_ON, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_HIGH | PIN_PORTD ) ;
#else	
 #ifdef PCBX9LITE
	x9LiteSportOff() ;
 #else	            
	configure_pins( PIN_SPORT_ON, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_LOW | PIN_PORTD ) ;
 #endif
#endif
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
	if ( parity )
	{
		USART2->CR1 |= USART_CR1_PCE | USART_CR1_M ;	// Need 9th bit for parity
		USART2->CR2 |= 0x2000 ;	// 2 stop bits
	}
	if ( baudRate == 115200 )		// ASSAN DSM
	{
#ifdef PCB9XT
		GPIOB->BSRRL = 0x0004 ;		// output disable
#else
		GPIOD->BSRRL = 0x0010 ;		// output enable
#endif
	}
	NVIC_SetPriority( USART2_IRQn, 2 ) ; // Quite high priority interrupt
  NVIC_EnableIRQ(USART2_IRQn);
}

struct t_sendingSport
{
	uint8_t *buffer ;
	uint32_t count ;
} SendingSportPacket ;


void x9dHubTxStart( uint8_t *buffer, uint32_t count )
{
	if ( FrskyTelemetryType == FRSKY_TEL_HUB )		// Hub
	{	
		SendingSportPacket.buffer = buffer ;
		SendingSportPacket.count = count ;
		TelemetryTx.sportBusy = 1 ;
		USART2->CR1 |= USART_CR1_TXEIE ;
	}
}

uint32_t hubTxPending()
{
	return TelemetryTx.sportBusy ;
}

#ifndef PCB9XT
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
		USART3->CR1 |= USART_CR1_PCE | USART_CR1_M ;
	}
	else
	{
		USART3->CR1 &= ~(USART_CR1_PCE | USART_CR1_M) ;
	}
}
#endif

//uint16_t SportStartDebug ;

void x9dSPortTxStart( uint8_t *buffer, uint32_t count, uint32_t receive )
{
//SportStartDebug	+= 1 ;
	USART2->CR1 &= ~USART_CR1_TE ;
	
	SendingSportPacket.buffer = buffer ;
	SendingSportPacket.count = count ;
#ifdef PCB9XT
	GPIOB->BSRRL = 0x0004 ;		// output enable
#else
 #if defined(PCBXLITE) || defined(REV19)
	GPIOD->BSRRH = PIN_SPORT_ON ;		// output enable
 #else
  #ifdef PCBX9LITE
	x9LiteSportOn() ;
//	GPIOD->BSRRH = 0x0010 ;		// output enable
  #else
	GPIOD->BSRRL = 0x0010 ;		// output enable
  #endif
 #endif
#endif
	if ( receive == 0 )
	{
		USART2->CR1 &= ~USART_CR1_RE ;
	}
	USART2->CR1 |= USART_CR1_TXEIE | USART_CR1_TE ;	// Force an idle frame
}

#if !defined(SIMU)


uint16_t RxIntCount ;

extern "C" void USART2_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x96 ;
#else
	RTC->BKP1R = 0x96 ;
#endif
#endif
  uint32_t status;
  uint8_t data;
	
	RxIntCount += 0x1000 ;

  status = USART2->SR ;

	if ( status & USART_SR_TXE )
	{
		if ( SendingSportPacket.count )
		{
			USART2->DR = *SendingSportPacket.buffer++ ;
			if ( --SendingSportPacket.count == 0 )
			{
				USART2->CR1 &= ~USART_CR1_TXEIE ;	// Stop Tx interrupt
				USART2->CR1 |= USART_CR1_TCIE ;	// Enable complete interrupt
			}
		}
		else
		{
			USART2->CR1 &= ~USART_CR1_TXEIE ;	// Stop Tx interrupt
		}
	}

	if ( ( status & USART_SR_TC ) && (USART2->CR1 & USART_CR1_TCIE ) )
	{
		USART2->CR1 &= ~USART_CR1_TCIE ;	// Stop Complete interrupt
		{
#ifdef PCB9XT
			GPIOB->BSRRH = 0x0004 ;		// output disable
#else
	#if defined(PCBXLITE) || defined(REV19)
			GPIOD->BSRRL = PIN_SPORT_ON ;		// output disable
 #else
  #ifdef PCBX9LITE
			x9LiteSportOff() ;
//			GPIOD->BSRRL = PIN_SPORT_ON ;		// output disable
  #else
			GPIOD->BSRRH = PIN_SPORT_ON ;		// output disable
 #endif
 #endif
#endif
			TelemetryTx.sportCount = 0 ;
			TelemetryTx.sportBusy = 0 ;
			USART2->CR1 |= USART_CR1_RE ;
		}
	}

  while (status & (USART_FLAG_RXNE | USART_FLAG_ERRORS))
	{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x102 ;
#else
	RTC->BKP1R = 0x102 ;
#endif
#endif
		
    data = USART2->DR;

    if (!(status & USART_FLAG_ERRORS))
		{
			RxIntCount += 1 ;
			put_fifo128( &Com1_fifo, data ) ;
			if ( FrskyTelemetryType == FRSKY_TEL_SPORT )		// SPORT
			{
				if ( LastReceivedSportByte == 0x7E && TelemetryTx.sportCount > 0 && data == TelemetryTx.SportTx.index )
				{
					x9dSPortTxStart( TelemetryTx.SportTx.ptr, TelemetryTx.sportCount, 0 ) ;
      		TelemetryTx.SportTx.index = 0x7E ;
				}
				LastReceivedSportByte = data ;
			}
		}
		else
		{
			if ( status & USART_FLAG_FE )
			{
				USART_FE += 1 ;
				if ( data )
				{ // A 0 byte is a line break
					put_fifo128( &Com1_fifo, data ) ;
				}
			}
			else
			{
				put_fifo128( &Com1_fifo, data ) ;
			}
			USART_ERRORS += 1 ;
			if ( status & USART_FLAG_ORE )
			{
				USART_ORE += 1 ;
			}
			if ( status & USART_FLAG_NE )
			{
				USART_NE += 1 ;
			}
			if ( status & USART_FLAG_PE )
			{
				USART_PE += 1 ;
			}
		}
    status = USART2->SR ;
  }
}


#endif

uint16_t rxTelemetry()
{
	return get_fifo128( &Com1_fifo ) ;
}

#ifndef PCB9XT
void txmit( uint8_t c )
{
	/* Wait for the transmitter to be ready */
  while ( (USART3->SR & USART_SR_TXE) == 0 ) ;

  /* Send character */
	USART3->DR = c ;
}

#ifndef PCBX7
#ifndef PCBX9LITE
uint32_t txPdcCom2( struct t_serial_tx *data )
{
	data->ready = 1 ;
	Current_Com2 = data ;
	USART3->CR1 |= USART_CR1_TXEIE ;
	return 1 ;			// Sent OK
}

extern "C" void USART3_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x97 ;
#else
	RTC->BKP1R = 0x97 ;
#endif
#endif
  uint32_t status;
  uint8_t data;
	USART_TypeDef *puart = USART3 ;
	
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
    data = puart->DR ;
    if (!(status & USART_FLAG_ERRORS))
		{
			if ( ( g_model.com2Function == COM2_FUNC_SBUSTRAIN ) || ( g_model.com2Function == COM2_FUNC_SBUS57600 ) )
			{
				put_fifo64( &Sbus_fifo, data ) ;	
			}
			else
			{
				put_fifo128( &Com2_fifo, data ) ;	
			}	 
		}
		else
		{
			if ( status & USART_FLAG_ORE )
			{
#ifdef PCB9XT
				USART2_ORE += 1 ;
#else
				USART_ORE += 1 ;
#endif
			}
		}
    status = puart->SR ;
	}
}
#endif
#endif
#endif

void start_timer11()
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

void stop_timer11()
{
	TIM11->CR1 = 0 ;
	NVIC_DisableIRQ(TIM1_TRG_COM_TIM11_IRQn) ;
}

// Handle software serial on COM1 input (for non-inverted input)
void init_software_com1(uint32_t baudrate, uint32_t invert, uint32_t parity )
{
	struct t_softSerial *pss = &SoftSerial1 ;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN ;		// Enable clock
	
	pss->bitTime = 2000000 / baudrate ;
	pss->softSerialEvenParity = parity ? 1 : 0 ;

#ifdef PCB9XT
	pss->softSerInvert = invert ? 0 : GPIO_Pin_3 ;	// Port A3
#else
	pss->softSerInvert = invert ? 0 : GPIO_Pin_6 ;	// Port D6
#endif
	pss->pfifo = &Com1_fifo ;

	pss->lineState = LINE_IDLE ;
	CaptureMode = CAP_COM1 ;

#ifdef PCB9XT
#define EXT_BIT_MASK	0x00000008
	SYSCFG->EXTICR[0] = 0 ;
#else
#define EXT_BIT_MASK	0x00000040
	SYSCFG->EXTICR[1] |= 0x0300 ;
#endif
	EXTI->IMR |= EXT_BIT_MASK ;
	EXTI->RTSR |= EXT_BIT_MASK ;
	EXTI->FTSR |= EXT_BIT_MASK ;

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
//#else
//	NVIC_DisableIRQ( EXTI9_5_IRQn ) ;
#endif
}

#ifdef PCB9XT

void consoleInit()
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
	NVIC_SetPriority( UART4_IRQn, 2 ) ; // Changed to 3 for telemetry receive
  NVIC_EnableIRQ(UART4_IRQn) ;
}

void com2_Configure( uint32_t baudrate, uint32_t invert, uint32_t parity )
{
	consoleInit() ;
	UART4SetBaudrate( baudrate ) ;
	com2Parity( parity ) ;
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
	NVIC_SetPriority( USART3_IRQn, 2 ) ; // Priority interrupt to handle 115200 baud BT
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
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x9A ;
#else
	RTC->BKP1R = 0x9A ;
#endif
#endif
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
#ifdef PCB9XT
				USART2_ORE += 1 ;
#else
				USART_ORE += 1 ;
#endif
			}
		}
    status = puart->SR ;
	}
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

extern "C" void UART4_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x9B ;
#else
	RTC->BKP1R = 0x9B ;
#endif
#endif
  uint32_t status;
  uint8_t data;
	USART_TypeDef *puart = UART4 ;

  status = puart->SR ;
	
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
//#endif
				}
				else
				{
					put_fifo128( &Com2_fifo, data ) ;
				}
			}
		}
		else
		{
			if ( status & USART_FLAG_ORE )
			{
#ifdef PCB9XT
				USART1_ORE += 1 ;
#else
				USART_ORE += 1 ;
#endif
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
    status = puart->SR ;
	}
	
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
	
}

void txmit( uint8_t c )
{
	/* Wait for the transmitter to be ready */
  while ( (UART4->SR & USART_SR_TXE) == 0 ) ;

  /* Send character */
	UART4->DR = c ;
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
uint16_t RemBitMeasure ;

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
	TIM10->SR = TIMER9_14SR_MASK & ~TIM_SR_CC1IF ;				// Clear flag
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
		TIM10->SR = TIMER9_14SR_MASK & ~TIM_SR_CC1IF ;				// Clear flag
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
			if ( capture < 250 )
			{
				RemBitMeasure = capture ;
			}
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
				if ( capture < 250 )
				{
					RemBitMeasure = capture ;
				}

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

#ifdef PCBX7
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
	NVIC_SetPriority( USART3_IRQn, 2 ) ; // Priority interrupt to handle 115200 baud BT
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

extern "C" void USART3_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x9C ;
#else
	RTC->BKP1R = 0x9C ;
#endif
#endif
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
#ifdef BLUETOOTH
			put_fifo128( &BtRx_fifo, data ) ;
#else
			(void) data ;	
#endif
		}
		else
		{
			if ( status & USART_FLAG_ORE )
			{
#ifdef PCB9XT
				USART2_ORE += 1 ;
#else
				USART_ORE += 1 ;
#endif
			}
		}
    status = puart->SR ;
	}
}


#ifdef BLUETOOTH
uint32_t txPdcBt( struct t_serial_tx *data )
{
	data->ready = 1 ;
	Current_Com3 = data ;
	USART3->CR1 |= USART_CR1_TXEIE ;
	return 1 ;			// Sent OK
}
#endif

#endif

#ifdef PCB9XT
extern "C" void EXTI3_IRQHandler()
#else
extern "C" void EXTI9_5_IRQHandler()
#endif
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x9D ;
#else
	RTC->BKP1R = 0x9D ;
#endif
#endif
  register uint32_t capture ;
  register uint32_t dummy ;
	struct t_softSerial *pss = &SoftSerial1 ;

	capture =  TIM7->CNT ;	// Capture time
	
#ifdef PCBX9D
 #ifndef PCBXLITE
	if ( EXTI->PR & XJT_HEARTBEAT_BIT )
	{
		XjtHeartbeatCapture.value = capture ;
		EXTI->PR = XJT_HEARTBEAT_BIT ;
	}
	if ( ( EXTI->PR & EXT_BIT_MASK ) == 0 )
	{
		return ;
	}
 #endif	// XLITE
#endif // X9D
	EXTI->PR = EXT_BIT_MASK ;
	
#ifdef PCB9XT
	dummy = GPIOA->IDR ;
	if ( ( dummy & GPIO_Pin_3 ) == pss->softSerInvert )
#else
	dummy = GPIOD->IDR ;
	if ( ( dummy & GPIO_Pin_6 ) == pss->softSerInvert )
#endif
	{
		// L to H transition
		pss->LtoHtime = capture ;
		TIM11->CNT = 0 ;
		TIM11->CCR1 = pss->bitTime * 12 ;
		uint32_t time ;
		capture -= pss->HtoLtime ;
		time = capture ;
		putCaptureTime( pss, time, 0 ) ;
		TIM11->DIER = TIM_DIER_CC1IE ;
	}
	else
	{
		// H to L transition
		pss->HtoLtime = capture ;
		if ( pss->lineState == LINE_IDLE )
		{
			pss->lineState = LINE_ACTIVE ;
			putCaptureTime( pss, 0, 3 ) ;
		}
		else
		{
			uint32_t time ;
			capture -= pss->LtoHtime ;
			time = capture ;
			putCaptureTime( pss, time, 1 ) ;
		}
		TIM11->DIER = 0 ;
		TIM11->CCR1 = pss->bitTime * 20 ;
		TIM11->CNT = 0 ;
		TIM11->SR = 0 ;
	}
}

extern "C" void TIM1_TRG_COM_TIM11_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x9E ;
#else
	RTC->BKP1R = 0x9E ;
#endif
#endif
	uint32_t status ;
	struct t_softSerial *pss = &SoftSerial1 ;

	status = TIM11->SR ;
	if ( status & TIM_SR_CC1IF )
	{		
		uint32_t time ;
		time = TIM7->CNT - pss->LtoHtime ;
		putCaptureTime( pss, time, 2 ) ;
		pss->lineState = LINE_IDLE ;
		TIM11->DIER = 0 ;
		TIM11->SR = 0 ;
	}
	
}

//#endif // nX12D


#if defined(PCBX9LITE)

uint32_t X9lSportOn ;
uint32_t X9lSportOff ;

void x9lCheckSportEnable()
{
	uint32_t readHigh ;
	uint32_t readLow ;

	configure_pins( PIN_SPORT_ON | PIN_SPORT_TX, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTD ) ;
	configure_pins( PIN_SPORT_RX, PIN_INPUT | PIN_PORTD ) ;

	GPIOD->BSRRH = PIN_SPORT_ON ;		// output enable
	GPIOD->BSRRH = PIN_SPORT_TX ;	// Clear Tx bit
	hw_delay( 40 ) ;	// 4uS
	readLow = GPIOD->IDR & PIN_SPORT_RX ? 1 : 0 ;
	GPIOD->BSRRL = PIN_SPORT_TX ;	// Set Tx bit
	hw_delay( 40 ) ;	// 4uS
	readHigh = GPIOD->IDR & PIN_SPORT_RX ? 1 : 0 ;

	if ( readHigh && !readLow )
	{
		X9lSportOn = PIN_SPORT_ON << 16 ;
		X9lSportOff = PIN_SPORT_ON ;
	}
	else
	{
		X9lSportOn = PIN_SPORT_ON ;
		X9lSportOff = PIN_SPORT_ON << 16 ;
	}
	x9LiteSportOff() ;
}


#ifdef BLUETOOTH
extern "C" void USART3_IRQHandler()
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x9C ;
#else
	RTC->BKP1R = 0x9C ;
#endif
#endif
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
#ifdef BLUETOOTH
			put_fifo128( &BtRx_fifo, data ) ;
#else
			(void) data ;	
#endif
		}
		else
		{
			if ( status & USART_FLAG_ORE )
			{
#ifdef PCB9XT
				USART2_ORE += 1 ;
#else
				USART_ORE += 1 ;
#endif
			}
		}
    status = puart->SR ;
	}
}

uint32_t txPdcBt( struct t_serial_tx *data )
{
	data->ready = 1 ;
	Current_Com3 = data ;
	USART3->CR1 |= USART_CR1_TXEIE ;
	return 1 ;			// Sent OK
	return 1 ;			// Sent OK
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
	NVIC_SetPriority( USART3_IRQn, 2 ) ; // Priority interrupt to handle 115200 baud BT
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

uint32_t txPdcCom2( struct t_serial_tx *data )
{
	return 1 ;
}

#endif



#endif


