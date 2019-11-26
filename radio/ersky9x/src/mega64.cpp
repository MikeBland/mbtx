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
// 0x01, 0x14, 2 bytes data - haptic on/off(1/0), haptic strength, 0x01
// 
// From 9X
// 0x01, 0x80, switches, buttons, trims, sticks, pots, enc_switch, enc_position, revision, 0x01
// 0x01, 0x81, 16 bit address, 32 bytes data, 0x01	- 32 bytes EEPROM data
// 0x01, 0x82, 16 bytes data, 0x01	- 8 trainer inputs
// 0x01, 0x83, 1 bytes data, 0x01	- RestartCount

// Also do byte stuffing, 0x01 sent as 0x1B, 0x81 and 0x1B sent as 0x1B, 0x9B.

// RemData: 12 bytes - buttons, trims, switches, Pot1, Pot2, Pot3, enc_switch, enc_pos, Revision


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
#include "string.h"
#include "isp.h"

#define PORT9X_ID          ID_USART1
#define PORT9X_BAUDRATE    200000
#define PORT9X_UART       USART1
#define PORT9X_UART_IRQn  UART1_IRQn

#define MAIN_TIMEOUT		30
#define BACKUP_TIMEOUT	30

uint8_t TxBuffer[140] ;
volatile uint8_t TxBusy ;
uint8_t DisplaySequence ;
uint8_t SendDisplay ;
uint8_t ResendDisplay ;

//uint8_t TempBuffer[6] ;

uint8_t M64Buttons ;
uint8_t M64Trims ;
uint8_t M64Contrast ;
uint8_t M64SetContrast ;
uint16_t M64Switches ;
uint8_t M64EncoderPosition ;
uint8_t M64Revision ;
uint8_t M64Received ;

//uint8_t ContrastCounter ;

uint8_t M64SetHaptic ;
uint8_t M64HapticOnOff ;
uint8_t M64HapticStrength ;

uint16_t M64Overruns ;
uint16_t M64CountErrors ;

uint8_t M64MainTimer ;
uint8_t M64BackupTimer ;
uint16_t M64ResetTimer ;
uint16_t M64UsedBackup ;
uint8_t M64ResetStatus ;
uint8_t M64ResetCount ;
uint8_t M64RestartCount ;
uint16_t M64RxCount ;
uint8_t M64DebugData[20] ;

uint8_t M64Display[1024] ;
uint8_t M64ReDisplay[1024] ;

uint16_t M64Analog[8] ;

//uint8_t EepromImage[4096] ;
//uint16_t EepromAddress ;
//uint8_t ReadingEeprom ;

struct t_fifo64 mega64_fifo ;
extern struct t_fifo64 RemoteRx_fifo ;
  
static void USART1_Configure( uint32_t baudrate )
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN ;		// Enable clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;			// Enable DMA2 clock

	configure_pins( GPIO_Pin_7, PIN_PERIPHERAL | PIN_PER_7 | PIN_PORTB | PIN_NO_PULLUP ) ;
	configure_pins( GPIO_Pin_6, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PER_7 | PIN_PORTB ) ;
//	GPIOB->MODER = (GPIOB->MODER & 0xFFFF0FFF ) | 0x0000A000 ;	// Alternate func.
//	GPIOB->AFR[0] = (GPIOB->AFR[0] & 0x00FFFFFF ) | 0x77000000 ;	// Alternate func.

	USART1->BRR = PeripheralSpeeds.Peri2_frequency / baudrate ;
//	USART1->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_M | USART_CR1_PCE | USART_CR1_TE ;
	USART1->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE ;
	USART1->CR2 = 0 ;
	USART1->CR3 = 0 ;
	(void) USART1->DR ;
	NVIC_SetPriority( USART1_IRQn, 2 ) ; // Slightly higher priority interrupt
  NVIC_EnableIRQ(USART1_IRQn) ;
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
		M64RxCount += 1 ;
		struct t_fifo64 *pfifo = &mega64_fifo ;
  	uint32_t next = (pfifo->in+1) & 0x3f;
		if ( next != pfifo->out )
		{
			pfifo->fifo[pfifo->in] = data ;
			pfifo->in = next ;
		}
	}

	if ( status & USART_SR_ORE )
	{
		M64Overruns += 1 ;
		data = USART1->DR ; // USART data register
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
//		put_fifo64( &Com2_fifo, CONSOLE_USART->UART_RHR ) ;	
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

//uint16_t rxCom2()
//{
//	return get_fifo64( &Com2_fifo ) ;
  
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



// Using USART1
// DMA2, channel 4, stream 7

uint32_t txPdcUsart( uint8_t *buffer, uint32_t size )
{
	DMA2_Stream7->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	DMA2_Stream7->CR = DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0
										 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 ;
	DMA2_Stream7->PAR = (uint32_t) &USART1->DR ;
	DMA2_Stream7->M0AR = (uint32_t) buffer ;
////	DMA2_Stream7->FCR = 0x05 ; //DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0 ;
	DMA2_Stream7->NDTR = size ;
	DMA2->HIFCR = DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7 ; // Write ones to clear bits
	DMA2_Stream7->CR |= DMA_SxCR_EN ;		// Enable DMA
	USART1->CR3 |= USART_CR3_DMAT ;
	TxBusy = 1 ;
	DMA2_Stream7->CR |= DMA_SxCR_TCIE ;
	NVIC_SetPriority( DMA2_Stream7_IRQn, 5 ) ; // Lower priority interrupt
  NVIC_EnableIRQ( DMA2_Stream7_IRQn ) ;
	return 0 ;
}

extern "C" void DMA2_Stream7_IRQHandler()
{
	USART1->CR3 &= ~USART_CR3_DMAT ;
	DMA2_Stream7->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	DMA2_Stream7->CR &= ~DMA_SxCR_TCIE ;	// Stop interrupt
	TxBusy = 0 ;
}


//void USART1_Handler()
//{
//  register Usart *pUsart = PORT9X_USART ;
//	uint32_t status ;
//  uint8_t data ;
	
//	status = pUsart->US_CSR ;

//	if ( status & US_CSR_RXRDY )
//	{
//		data = pUsart->US_RHR ; // USART data register
//		struct t_fifo64 *pfifo = &mega64_fifo ;
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
uint8_t SlaveRxCount ;

#define SLAVE_RX_SIZE		80
uint8_t SlaveTempReceiveBuffer[SLAVE_RX_SIZE] ;

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
				SlaveRxCount = SlavePtr - SlaveTempReceiveBuffer ;
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
					if ( SlavePtr < &SlaveTempReceiveBuffer[SLAVE_RX_SIZE] )
					{
				  	*SlavePtr++ = byte ;
					}
				}
			}
		break ;
	}
}

uint8_t RemData[20] ;
uint8_t RemIndex ;
uint8_t RemState ;
uint8_t RemStuff ;
uint8_t RemCsum ;
uint8_t RemValid ;
uint16_t RemOkCount ;
uint16_t RemBadCount ;



static void poll_mega64()
{
	int16_t byte ;
	while ( ( byte = get_fifo64( &mega64_fifo ) ) != -1 )
	{
		processSlaveByte( byte ) ;
		if (SlaveActionRequired)
		{
			SlaveActionRequired = 0 ;
			if ( SlaveType == 0x80 )
			{
				uint32_t valid = 0 ;
				
				if ( SlaveRxCount == 22 )	// Check in case of overrun error
				{
					// validate checksum
					if (  SlaveTempReceiveBuffer[18] & 0x80 )
					{
						uint16_t rcsum = SlaveTempReceiveBuffer[17] | ( SlaveTempReceiveBuffer[18] << 8 ) ;
						uint16_t csum = 0 ;
						uint32_t count ;
						SlaveTempReceiveBuffer[17] = 0 ;
						SlaveTempReceiveBuffer[18] = 0 ;

						for ( count = 0 ; count < 22 ; count += 1 )
						{
							csum += SlaveTempReceiveBuffer[count] ;
						}
						csum |= 0x8000 ;
						if ( rcsum == csum )
						{
							valid = 1 ;
						}
					}
					else
					{
						valid = 1 ;
					}
				}
				if ( valid )
				{
					uint16_t switches ;
					M64MainTimer = MAIN_TIMEOUT ;
					byte = SlaveTempReceiveBuffer[0] ;
					M64Buttons = byte & 0x7E ;
					M64Trims = SlaveTempReceiveBuffer[1] ;
					switches = SlaveTempReceiveBuffer[2] | ( ( byte & 1 ) << 8 ) ;
					if ( SlaveTempReceiveBuffer[19] & 0x20 )
					{
						switches |= 0x0200 ;	// Encoder switch
					}
					M64Switches = switches ;
					M64Analog[0] = SlaveTempReceiveBuffer[3] | ( SlaveTempReceiveBuffer[4] << 8 ) ;
					M64Analog[1] = SlaveTempReceiveBuffer[5] | ( SlaveTempReceiveBuffer[6] << 8 ) ;
					M64Analog[2] = SlaveTempReceiveBuffer[7] | ( SlaveTempReceiveBuffer[8] << 8 ) ;
					M64Analog[3] = SlaveTempReceiveBuffer[9] | ( SlaveTempReceiveBuffer[10] << 8 ) ;
					M64Analog[4] = SlaveTempReceiveBuffer[11] | ( SlaveTempReceiveBuffer[12] << 8 ) ;
					M64Analog[5] = SlaveTempReceiveBuffer[13] | ( SlaveTempReceiveBuffer[14] << 8 ) ;
					M64Analog[6] = SlaveTempReceiveBuffer[15] | ( SlaveTempReceiveBuffer[16] << 8 ) ;
					M64Analog[7] = SlaveTempReceiveBuffer[17] | ( SlaveTempReceiveBuffer[18] << 8 ) ;
					M64EncoderPosition = SlaveTempReceiveBuffer[20] ;
					M64Revision = SlaveTempReceiveBuffer[21] ;
					M64Received = 1 ;
				}
				else
				{
					M64CountErrors += 1 ;
					
				}
			}
			else if ( SlaveType == 0x83 )	// RestartCount
			{
				M64RestartCount = SlaveTempReceiveBuffer[0] ;
				uint32_t i ;
				for ( i = 0 ; i < 12 ; i += 1 )
				{
					M64DebugData[i] = SlaveTempReceiveBuffer[i] ;
				}
			}
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
		}
	}
	while ( ( byte = get_fifo64( &RemoteRx_fifo ) ) != -1 )
	{
		if ( RemState == 0 )
		{
			if ( byte == 0x55 )
			{
				RemState = 1 ;
				RemIndex = 0 ;
				RemCsum = 0 ;
			}
		}
		else
		{
			if ( RemStuff )
			{
				RemStuff = 0 ;
				byte |= 0x50 ;
			}
			else
			{
				if ( byte == 0x56 )
				{
					RemStuff = 1 ;
					continue ;
				}
			}
			RemData[RemIndex++] = byte ;
			if ( RemIndex == 13 )
			{
				RemState = 0 ;
				if ( RemCsum == RemData[12] )
				{
					// Accept data
					M64BackupTimer = BACKUP_TIMEOUT ;
					RemValid = 1 ;
					RemOkCount += 1 ;
					if ( M64MainTimer == 0 )
					{
						// Nothing received over the main connection
						M64UsedBackup += 1 ;
						int16_t byte ;
						uint16_t switches ;
						
						byte = RemData[0] ;
						M64Buttons = byte & 0x7E ;
						M64Trims = RemData[1] ;
						switches = RemData[2] | ( ( byte & 1 ) << 8 ) ;
						if ( RemData[9] & 0x20 )
						{
							switches |= 0x0200 ;	// Encoder switch
						}
						M64Switches = switches ;
						M64Analog[4] = RemData[3] | ( RemData[4] << 8 ) ;
						M64Analog[5] = RemData[5] | ( RemData[6] << 8 ) ;
						M64Analog[6] = RemData[7] | ( RemData[8] << 8 ) ;
						M64EncoderPosition = RemData[10] ;
						M64Revision = RemData[11] ;
					}
				}
				else
				{
					RemValid = 0 ;
					RemBadCount += 1 ;
				}
			}
			else
			{
				RemCsum += byte ;
			}

		}
	}
}

// the loop function runs over and over again forever
void checkM64()
{
	uint8_t count ;
//	int16_t byte ;

	poll_mega64() ;
	
	if ( M64SetHaptic )
	{
		if ( TxBusy == 0 )
		{
			uint8_t buf[2] ;
			buf[0] = M64HapticOnOff ;
			buf[1] = M64HapticStrength ;
			count = fillTxBuffer( buf, 0x14, 2 ) ;
			txPdcUsart( TxBuffer, count ) ;
			M64SetHaptic = 0 ;
		}
	}
	
	if ( M64SetContrast )
	{
		if ( TxBusy == 0 )
		{
			uint8_t buf[2] ;
			buf[0] = M64Contrast ;
			buf[1] = g_eeGeneral.rotateScreen ? 1 : 0 ;
			if (g_eeGeneral.reverseScreen)
			{
				buf[1] |= 2 ;
			}
			count = fillTxBuffer( buf, 0x13, 2 ) ;
			txPdcUsart( TxBuffer, count ) ;
			M64SetContrast = 0 ;
		}
	}
	
	if ( SendDisplay )
	{
		if ( TxBusy == 0 )
		{
			count = fillTxBuffer( M64Display, 0, 64 ) ;
			DisplaySequence = 0x81 ;
			txPdcUsart( TxBuffer, count ) ;
			SendDisplay = 0 ;
		}
	}

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
	 
	if ( DisplaySequence )
	{
		
		if ( TxBusy == 0 )
		{
			count = fillTxBuffer( &M64Display[64*(DisplaySequence & 0x0F)], DisplaySequence & 0x0F, 64 ) ;
			txPdcUsart( TxBuffer, count ) ;
			DisplaySequence += 1 ;
			if ( DisplaySequence > 0x8F)
			{
				DisplaySequence = 0 ;
//				if ( ++ContrastCounter > 9 )
//				{
//					ContrastCounter = 0 ;
//					M64SetContrast = 1 ;
//				}
				if ( ResendDisplay )
				{
					memcpy( M64Display, M64ReDisplay, sizeof(M64Display) ) ;
					ResendDisplay = 0 ;
					SendDisplay = 1 ;
				}
			}
		}
	}



	 
//	while ( ( byte = get_fifo64( &mega64_fifo ) ) != -1 )
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
}

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

void displayToM64()
{
	if ( DisplaySequence == 0 )
	{
		memcpy( M64Display, DisplayBuf, sizeof(M64Display) ) ;
		SendDisplay = 1 ;
	}
	else
	{
		memcpy( M64ReDisplay, DisplayBuf, sizeof(M64ReDisplay) ) ;
		ResendDisplay = 1 ;
	}
}

void initM64()
{
	ispForceResetOff() ;
	USART1_Configure( PORT9X_BAUDRATE ) ;
}

void m64_10mS()
{
	if ( M64MainTimer )
	{
		M64MainTimer -= 1 ;
		M64ResetStatus = 1 ;
	}
	if ( M64BackupTimer )
	{
		M64BackupTimer -= 1 ;
		M64ResetStatus = 1 ;
	}
	if ( M64ResetTimer )
	{
		M64ResetTimer -= 1 ;
	}
	if ( M64ResetStatus )
	{
		if ( M64ResetTimer == 0 )
		{
			if ( ( M64MainTimer == 0 ) && ( M64BackupTimer == 0 ) )
			{
				// lost M64
				M64ResetStatus = 0 ;
				M64ResetCount += 1 ;
				resetM64() ;
				M64ResetTimer = 250 ;
			}
		}
	}
}

uint32_t m64ReceiveStatus()
{
	uint32_t result = 0 ;
	if ( M64MainTimer )
	{
		result |= 1 ;
	}
	if ( M64BackupTimer )
	{
		result |= 2 ;
	}
	return result ;
}


