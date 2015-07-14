/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
  On the Due it is attached to digital pin 13.
 */

// Protocol to 9X
// 0x01, 0x00, 64 bytes data, 0x01	- first line of display
// 0x01, 0x01, 64 bytes data, 0x01	- second line of display
// ...
// 0x01, 0x07, 64 bytes data, 0x01	- last line of display
// 0x01, 0x0F, 64 bytes data, 0x01	- last line of display



// 0x01, 0x10, 2 bytes protocol, 32 bytes data, 0x01  - 16 channel outputs
// 0x01, 0x11, 16 bit address, 8 bit data, 0x01  - Write EEPROM value
// 0x01, 0x12, 16 bit address, 0x01  - send 32 bytes EEPROM data @ address
// 0x01, 0x13, 2 bytes data - contrast and backlight, 0x01
// 
// From 9X
// 0x01, 0x80, switches, buttons, trims, sticks, pots, 0x01
// 0x01, 0x81, 16 bit address, 32 bytes data, 0x01	- 32 bytes EEPROM data
// 0x01, 0x82, 16 bytes data, 0x01	- 8 trainer inputs

// Also do byte stuffing, 0x01 sent as 0x1B, 0x81 and 0x1B sent as 0x1B, 0x9B.

// Allocation of serial ports:
// UART is connected to 16U2 device - debug only
// UART - PA8(A) - RX Due0, PA9(A) - TX Due1, both to 16U2
// USART0 - PA10(A) - RX1 Due19, PA11(A) - TX1 Due18
// USART1 - PA12(A) - RX2 Due17, PA13(A) - TX2 Due16
// USART2 - UART only TX-PB20/AD11, RX-PB21/AD14
// USART3 - PD5(B) - RX3 Due15, PD4(B) - TX3 Due14

// SSC TD PA16 (B) - AD7 needs to parallel with PWMH3 (PC9 (B) ) - PIN 41
// Note, 8 PWM channels are available on the 3X8

// UART use for debug via 16U2?
// USART0 use for COM1 == telemetry
// USART1 use for 9x comms
// USART2 use for Bluetooth



#include <stdint.h>
#include <stdlib.h>
#include "ersky9x.h"
#include "logicio.h"
#include "myeeprom.h"
#include "lcd.h"
#include "drivers.h"
#include "CoOS.h"

#define PORT9X_ID          ID_UART4
#define PORT9X_BAUDRATE    200000
#define PORT9X_UART       UART4
#define PORT9X_UART_IRQn  UART4_IRQn

uint8_t TxBuffer[140] ;
volatile uint8_t TxBusy ;
uint8_t DisplaySequence ;

uint8_t TempBuffer[6] ;

uint8_t Buttons ;
uint8_t Trims ;
uint16_t Switches ;

uint16_t Analog[8] ;

//uint8_t EepromImage[4096] ;
//uint16_t EepromAddress ;
//uint8_t ReadingEeprom ;

struct t_fifo64 Arduino_fifo ;
  
void USART1_Configure( uint32_t baudrate, uint32_t masterClock)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN ;		// Enable clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock

	configure_pins( GPIO_Pin_7, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_7 | PIN_PORTB | PIN_NO_PULLUP ) ;
	configure_pins( GPIO_Pin_6, PIN_PERIPHERAL | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PER_7 | PIN_PORTB ) ;

	USART1->BRR = PeripheralSpeeds.Peri2_frequency / 100000 ;
	USART1->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_M | USART_CR1_PCE ;
	USART1->CR2 = 0 ;
	USART1->CR3 = 0 ;
	(void) USART1->DR ;
	NVIC_SetPriority( USART1_IRQn, 4 ) ; // Lower priority interrupt
//  NVIC_EnableIRQ(USART1_IRQn) ;
}

extern "C" void USART1_IRQHandler()
{
//  register Usart *pUsart = PORT9X_USART ;
	uint32_t status ;
  uint8_t data ;
	
	status = USART1->SR ;

	if ( status & USART_SR_RXNE )
	{
		data = USART1->DR ; // USART data register
		struct t_fifo64 *pfifo = &Arduino_fifo ;
  	uint32_t next = (pfifo->in+1) & 0x3f;
		if ( next != pfifo->out )
		{
			pfifo->fifo[pfifo->in] = data ;
			pfifo->in = next ;
		}
	}

//  if ( pUsart->US_IMR & US_IMR_ENDTX )
//	{
//	 	if ( status & US_CSR_ENDTX )
//		{
//			pUsart->US_IDR = US_IDR_ENDTX ;
//			pUsart->US_PTCR = US_PTCR_TXTDIS ;
//			TxBusy = 0 ;
//		}
//	}
//	else
//	{
//		pUsart->US_IDR = 0xFFFFFFFF ;
//	}
}

//void UART_Handler()
//{
////	if ( ( g_model.com2Function == 1 ) || ( g_model.com2Function == 2 ) )
////	{
////		put_fifo64( &Sbus_fifo, CONSOLE_USART->UART_RHR ) ;	
////	}
////	else
////	{
//		put_fifo64( &Console_fifo, CONSOLE_USART->UART_RHR ) ;	
////	}	 
//}

//void txmit( uint8_t c )
//{
//  Uart *pUart=CONSOLE_USART ;

//	/* Wait for the transmitter to be ready */
//  while ( (pUart->UART_SR & UART_SR_TXEMPTY) == 0 ) ;

//  /* Send character */
//  pUart->UART_THR=c ;
//}

//uint16_t rxuart()
//{
//	return get_fifo64( &Console_fifo ) ;
  
////	Uart *pUart=CONSOLE_USART ;

////  if (pUart->UART_SR & UART_SR_RXRDY)
////	{
////		return pUart->UART_RHR ;
////	}
////	return 0xFFFF ;
//}



//void initPort9xUsart( uint32_t baudrate )
//{
//	configure_pins( PIO_PA12 | PIO_PA13, PIN_PERIPHERAL | PIN_PER_A | PIN_INPUT | PIN_PORTA | PIN_NO_PULLUP ) ;
  
//	PMC->PMC_PCER0 = 1 << PORT9X_ID;
//  Usart *pUsart = PORT9X_USART ;
//  pUsart->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS;
//  pUsart->US_MR =  0x000008C0 ;  // NORMAL, No Parity, 8 bit
//  pUsart->US_BRGR = ( Master_frequency / baudrate ) / 16 ;
//  pUsart->US_PTCR = US_PTCR_RXTDIS | US_PTCR_TXTDIS;
//  pUsart->US_CR = US_CR_RXEN | US_CR_TXEN ;
//	pUsart->US_IER = US_IER_RXRDY ;
//}

//uint32_t txPdcUsart( uint8_t *buffer, uint32_t size )
//{
//  register Usart *pUsart = PORT9X_USART ;

//	if ( pUsart->US_TNCR == 0 )
//	{
//	  pUsart->US_TNPR = (uint32_t)buffer ;
//		pUsart->US_TNCR = size ;
//		pUsart->US_PTCR = US_PTCR_TXTEN ;
//		(void) pUsart->US_CSR ;
//		pUsart->US_IER = US_IER_ENDTX ;
//		TxBusy = 1 ;
//		NVIC_EnableIRQ(PORT9X_USART_IRQn) ;
//		return 1 ;
//	}
//	return 0 ;
//}

//void USART1_Handler()
//{
//  register Usart *pUsart = PORT9X_USART ;
//	uint32_t status ;
//  uint8_t data ;
	
//	status = pUsart->US_CSR ;

//	if ( status & US_CSR_RXRDY )
//	{
//		data = pUsart->US_RHR ; // USART data register
//		struct t_fifo64 *pfifo = &Arduino_fifo ;
//  	uint32_t next = (pfifo->in+1) & 0x3f;
//		if ( next != pfifo->out )
//		{
//			pfifo->fifo[pfifo->in] = data ;
//			pfifo->in = next ;
//		}
//	}

//  if ( pUsart->US_IMR & US_IMR_ENDTX )
//	{
//	 	if ( status & US_CSR_ENDTX )
//		{
//			pUsart->US_IDR = US_IDR_ENDTX ;
//			pUsart->US_PTCR = US_PTCR_TXTDIS ;
//			TxBusy = 0 ;
//		}
//	}
////	else
////	{
////		pUsart->US_IDR = 0xFFFFFFFF ;
////	}
//}

uint8_t fillTxBuffer( uint8_t *source, uint8_t type, uint8_t count )
{
	uint8_t *pdest = TxBuffer ;
	*pdest++ = 1 ;
	if ( type == 1 )
	{
		*pdest++ = 0x1B ;
		type |= 0x80 ;
	}
	*pdest++ = type ;
	while ( count )
	{
		type = *source++ ;
		if ( ( type == 1 ) || ( type == 0x1B ) )
		{
			*pdest++ = 0x1B ;
			type |= 0x80 ;
		}
		*pdest++ = type ;
		count -= 1 ;
	}
	*pdest++ = 1 ;
	return pdest - TxBuffer ;
}

uint8_t SlaveState ;
uint8_t SlaveStuff ;
uint8_t SlaveType ;
uint8_t *SlavePtr ;
uint8_t SlaveActionRequired ;
uint8_t SlaveDisplayRefresh ;

uint8_t SlaveTempReceiveBuffer[80] ;

#define SlaveWaitSTX		0
#define SlaveGotSTX			1
#define SlaveData				2

void processSlaveByte( uint8_t byte )
{
	switch ( SlaveState )
	{
		case SlaveWaitSTX :
			if ( byte == 1 )
			{
				SlaveState = SlaveGotSTX ;
			}
		break ;
		
		case SlaveGotSTX :
			if ( byte != 1 )
			{
				if ( byte == 0x1B )
				{
					SlaveStuff = 1 ;
				}
				else
				{
					if ( SlaveStuff )
					{
						byte &= 0x7F ;
						SlaveStuff = 0 ;
					}
					SlaveType = byte ;
					SlavePtr = SlaveTempReceiveBuffer ;
					SlaveState = SlaveData ;
				}
			}
		break ;
		
		case SlaveData :
			if ( byte == 1 )
			{
				SlaveActionRequired = 1 ;
				SlaveState = SlaveWaitSTX ;
			}
			else
			{
				if ( byte == 0x1B )
				{
					SlaveStuff = 1 ;
				}
				else
				{
					if ( SlaveStuff )
					{
						byte &= 0x7F ;
						SlaveStuff = 0 ;
					}
				  *SlavePtr++ = byte ;
				}
			}
			
		break ;
	}

}


// the loop function runs over and over again forever
//void loop()
//{
//	uint8_t count ;
//	int16_t byte ;

//	if ( Tenms )
//	{
//		Tenms = 0 ;
//		SendProtocol = 1 ;
//	}
//	if ( SendProtocol )
//	{
//		if ( TxBusy == 0  )
//		{
//			uint8_t index = 0 ;
//			uint8_t buffer[34] ;
//			buffer[0] = 0 ;		// PPM
//			buffer[1] = 0 ;		// sub-protocol/ num channels for PPM

//			for ( count = 2 ; count < 34 ; count += 2 )
//			{
//				buffer[count] = Channel[index] ;
//				buffer[count+1] = Channel[index] >> 8 ;
//				index += 1 ;
//			}
//			count = fillTxBuffer( buffer, 0x10, 34 ) ;
//			txPdcUsart( TxBuffer, count ) ;
//			SendProtocol = 0 ;
//		}
//	}
//	if ( OnehundredmS )
//	{
////		StartDelay = 1 ;
//		lcd_clear() ;
//		lcd_puts_Pleft( 0, "Arduino Due" ) ;
//		lcd_outhex4( 17*FW, 0, HexCounter ) ;
//		lcd_puts_Pleft( 7*FH, "Bottom Line\020End" ) ;

//		lcd_puts_Pleft( 2*FH, "Buttons" ) ;
//		lcd_outhex4( 10*FW, 2*FH, Buttons ) ;
//		lcd_puts_Pleft( 3*FH, "Trims" ) ;
//		lcd_outhex4( 10*FW, 3*FH, Trims ) ;
//		lcd_puts_Pleft( 4*FH, "Switches" ) ;
//		lcd_outhex4( 10*FW, 4*FH, Switches ) ;

//		lcd_outhex4( 16*FW, 2*FH, Analog[0] ) ;
//		lcd_outhex4( 16*FW, 3*FH, Analog[1] ) ;
//		lcd_outhex4( 16*FW, 4*FH, Analog[2] ) ;
//		lcd_outhex4( 0, 5*FH, Analog[3] ) ;
//		lcd_outhex4( 26, 5*FH, Analog[4] ) ;
//		lcd_outhex4( 52, 5*FH, Analog[5] ) ;
//		lcd_outhex4( 78, 5*FH, Analog[6] ) ;
//		lcd_outhex4( 104, 5*FH, Analog[7] ) ;

//		lcd_outhex4( 0, 6*FH, EepromAddress ) ;
		
//		lcd_char_inverse( 0, 7*FH, 127, 1 ) ;
//		SendDisplay = 1 ;
//		OnehundredmS = 0 ;
//	}
	
//	if ( SendDisplay )
//	{
//		if ( TxBusy == 0 )
//		{
//			count = fillTxBuffer( DisplayBuf, 0, 64 ) ;
//			DisplaySequence = 0x81 ;
//			txPdcUsart( TxBuffer, count ) ;
//			SendDisplay = 0 ;
//		}
//	}

//	if ( OneSecond )
//	{
//		HexCounter += 1 ;
//		if ( OutputState )
//		{
//			OutputState = 0 ;
////			PIOA->PIO_SODR = 0x0800 ;
//			PIOB->PIO_SODR = 0x08000000 ;

//		}
//		else
//		{
//			OutputState = 1 ;
//			PIOB->PIO_CODR = 0x08000000 ;
////			PIOA->PIO_CODR = 0x0800 ;
////		  PORT9X_USART->US_THR = 0x2A ;
//		}
////		UART->UART_THR = 0x55 + Console_fifo.in ;
//		OneSecond = 0 ;
//	}

//	if ( ReadingEeprom == 1 )
//	{
//		if ( StartDelay )
//		{
//			if ( EepromAddress < 4096 )
//			{
//				if ( TxBusy == 0 )
//				{
//					uint8_t temp[2] ;
//					temp[0] = EepromAddress ;
//					temp[0] = EepromAddress >> 8 ;
//					// 0x01, 0x12, 16 bit address, 0x01  - send 32 bytes EEPROM data @ address
//					count = fillTxBuffer( temp, 0x12, 2 ) ;
//					ReadingEeprom = 2 ;
//					txPdcUsart( TxBuffer, count ) ;
//				}
//			}
//			else
//			{
//				ReadingEeprom = 0 ;	// Done
//			}
//		}
//	}
	 
//	if ( DisplaySequence )
//	{
//		if ( TxBusy == 0 )
//		{
//			count = fillTxBuffer( &DisplayBuf[64*(DisplaySequence & 0x0F)], DisplaySequence & 0x0F, 64 ) ;
//			txPdcUsart( TxBuffer, count ) ;
//			DisplaySequence += 1 ;
//			if ( DisplaySequence > 0x8F)
//			{
//				DisplaySequence = 0 ;
//			}
//		}
//	}
	
//	while ( ( byte = get_fifo64( &Arduino_fifo ) ) != -1 )
//	{
//		processSlaveByte( byte ) ;
//		if (SlaveActionRequired)
//		{
//			SlaveActionRequired = 0 ;
//			if ( SlaveType == 0x80 )
//			{
//				byte = SlaveTempReceiveBuffer[0] ;
//				Buttons = byte & 0x7E ;
//				Trims = SlaveTempReceiveBuffer[1] ;
//				Switches = SlaveTempReceiveBuffer[2] | ( ( byte & 1 ) << 8 ) ;
//				Analog[0] = SlaveTempReceiveBuffer[3] | ( SlaveTempReceiveBuffer[4] << 8 ) ;
//				Analog[1] = SlaveTempReceiveBuffer[5] | ( SlaveTempReceiveBuffer[6] << 8 ) ;
//				Analog[2] = SlaveTempReceiveBuffer[7] | ( SlaveTempReceiveBuffer[8] << 8 ) ;
//				Analog[3] = SlaveTempReceiveBuffer[9] | ( SlaveTempReceiveBuffer[10] << 8 ) ;
//				Analog[4] = SlaveTempReceiveBuffer[11] | ( SlaveTempReceiveBuffer[12] << 8 ) ;
//				Analog[5] = SlaveTempReceiveBuffer[13] | ( SlaveTempReceiveBuffer[14] << 8 ) ;
//				Analog[6] = SlaveTempReceiveBuffer[15] | ( SlaveTempReceiveBuffer[16] << 8 ) ;
//				Analog[7] = SlaveTempReceiveBuffer[17] | ( SlaveTempReceiveBuffer[18] << 8 ) ;
//			}
//			else if ( SlaveType == 0x81 )	// EEPROM data
//			{
//				uint16_t address ;
//				uint32_t i ;
//				address = SlaveTempReceiveBuffer[0] | ( SlaveTempReceiveBuffer[1] << 8 ) ;
//				for ( i = 2 ; i < 34 ; i += 1 )
//				{
//					EepromImage[address++] = SlaveTempReceiveBuffer[i] ;
//				}
//				EepromAddress += 32 ;
//				ReadingEeprom = 1 ;
//			}
//		}
//	}
//  wdt_reset() ;
//}

//void main_loop(void* pdata)
//{
//	EepromAddress = 0 ;
//	ReadingEeprom = 1 ;
//	Activated = 1 ;

//	for(;;)
//	{
////		PIOA->PIO_SODR = 0x0800 ;
//		loop() ;
//		CoTickDelay(1) ;		// 2mS, needed to allow lower priority tasks to run
////		PIOA->PIO_CODR = 0x0800 ;
//	}
//}


