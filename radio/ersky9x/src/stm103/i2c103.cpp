/**
  ******************************************************************************
  * @file    Project/i2c103.cpp
  * @author  Mike Blandford
  * @date    7-October-2019
  * @brief   This file provides a set of functions needed to manage the
  *          communication to I2C peripheral
  ******************************************************************************
*/


#include "../ersky9x.h"
#include <stm32f10x.h>
//#include "stm32f2xx_gpio.h"
#include "logicio103.h"
//#include "stm32f2xx_rcc.h"
#include "hal.h"
#include "i2c103.h"
#include "../timers.h"
#include <stdlib.h>

//#define I2C_M23008ADDRESS		0x20
//#define I2C_ENCODER_ADDRESS	0x04

#define I2C_READ_OP 		0
#define I2C_WRITE_OP 		1

#define TWI_HIGH_SPEED 0 ;
#define TWI_LOW_SPEED 1 ;

// Hardware version using interrupts

#define FLAG_MASK         ((uint32_t)0x00FFFFFF)  /*<! I2C FLAG mask */
#define I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED        ((uint32_t)0x00070082)  /* BUSY, MSL, ADDR, TXE and TRA flags */
#define I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED           ((uint32_t)0x00030002)  /* BUSY, MSL and ADDR flags */
#define I2C_EVENT_MASTER_MODE_SELECT                      ((uint32_t)0x00030001)  /* BUSY, MSL and SB flag */
#define I2C_EVENT_MASTER_BYTE_RECEIVED                    ((uint32_t)0x00030040)  /* BUSY, MSL and RXNE flags */
#define I2C_EVENT_MASTER_BYTE_TRANSMITTING                ((uint32_t)0x00070080) /* TRA, BUSY, MSL, TXE flags */
#define I2C_EVENT_MASTER_BYTE_TRANSMITTED                 ((uint32_t)0x00070084)  /* TRA, BUSY, MSL, TXE and BTF flags */

int8_t Rtc_valid ;

void submitI2cRequest( struct t_I2C_request *ptr ) ;

static void I2C2_GPIO_Configuration(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN ; 			// Enable portB clock
	
  /* Configure I2C_EE pins: SCL and SDA */
	configure_pins( I2C_SCL_Pin | I2C_SDA_Pin, PIN_OP50 | I2C_PORT | PIN_AF_OD ) ;
	// If serial, SCL is TX and SDA is Rx, AltFunction = PER_7
}

void init_I2C2()
{
	I2C_TypeDef *pi2c = I2C2 ;

	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN ; 		// Enable I2C2 clock
	I2C2_GPIO_Configuration() ;
	
	pi2c->OAR2 = 0 ;
	pi2c->OAR1 = 0 ;
	// Using 30Mhz APB clock
	pi2c->CCR = 0xC000 | 3 ;	// for 400kHz

	pi2c->CR2 = 30 ;	// 30MHz, no interrupts
	pi2c->CR1 = I2C_CR1_PE ;

	pi2c->CR2 |= I2C_CR2_ITERREN | I2C_CR2_ITEVTEN ;
	NVIC_SetPriority( I2C2_EV_IRQn, 6 ) ; // Lower priority interrupt
	NVIC_SetPriority( I2C2_ER_IRQn, 6 ) ; // Lower priority interrupt
}

void stop_I2C2()
{
	I2C2->CR1 = 0 ;
	RCC->APB1ENR &= ~RCC_APB1ENR_I2C2EN ; // Disable I2C2 clock
}

//uint32_t I2C_debug[8] ;
uint32_t I2Clast ;


uint32_t I2C2_CheckEvent( uint32_t I2C_EVENT )
{
  uint32_t lastevent = 0 ;
  uint32_t flag1 = 0, flag2 = 0 ;

  /* Read the I2Cx status register */
  flag1 = I2C2->SR1 ;
  flag2 = I2C2->SR2 ;
  flag2 = flag2 << 16 ;

  /* Get the last event value from I2C status register */
  lastevent = (flag1 | flag2) ;
	I2Clast = lastevent ;
  lastevent &= FLAG_MASK ;

  /* Check whether the last event contains the I2C_EVENT */
  
	return ((lastevent & I2C_EVENT) == I2C_EVENT) ? 1 : 0 ;
}

#define I2C_TIMEOUT_MAX 1000
bool I2C2_WaitEvent(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (!I2C2_CheckEvent( event))
	{
    if ((timeout--) == 0) return false;
  }
  return true;
}

// General I2C operation
#define TWI_WRITE_ONE			12
#define TWI_READ_ONE			13
#define TWI_WRITE_BUFFER	14
#define TWI_READ_BUFFER		15
#define TWI_WRITE_COMMAND_BUFFER    16
#define TWI_READ_COMMAND_BUFFER     17
#define TWI_WAIT_BUFFER		18

#define I2C_DEV_MASK					0x00FF0000
#define I2C_ADDR_COUNT_MASK		0x00000300
#define I2C_READ_WRITE_MASK		0x00001000

// I2C States
#define I2C_STATE_IDLE						0
#define I2C_STATE_STARTING				1
#define I2C_STATE_DEVADDR_SENT    2
#define I2C_STATE_ADDR_2          3
#define I2C_STATE_ADDR_1          4
#define I2C_STATE_ADDR_0          5
#define I2C_STATE_RESTARTING      6
#define I2C_STATE_BEGIN_RECEIVE   7
#define I2C_STATE_BEGIN_SEND      8
#define I2C_STATE_RECEIVE         9 
#define I2C_STATE_SEND           10


struct t_I2C_request
{
	uint32_t mmr ;		// 0x00AARa00  AA=device address, a=address byte count, R=1 for read
	uint8_t *dataBuffer ;
	uint32_t address ;
	uint32_t dataSize ;
	struct t_I2C_request *next ;
	uint8_t commandByte ;
	uint8_t operationType ;
	uint8_t speed ;
	volatile uint8_t done ;	// 0-waiting, 1=OK, 2=failed
} ;

struct t_I2C_request EncoderI2cRequest ;
struct t_I2C_request ExtRtcI2cRequest ;
struct t_I2C_request ExtRtcI2cWriteRequest ;
//struct t_I2C_request GeneralI2cRequest ;
struct t_I2C_request *I2cHeadPointer ;
struct t_I2C_request *I2cTailPointer ;
struct t_I2C_request *I2cCurrentPointer ;

#define RTC_SIZE						8

uint8_t I2C_State ;
uint8_t ExternalRtc[RTC_SIZE] ;

static uint32_t fromBCD( uint8_t bcd_value )
{
	return ( ( ( bcd_value & 0xF0 ) * 10 ) >> 4 ) + ( bcd_value & 0x0F ) ;
}

static uint32_t toBCD( uint32_t value )
{
	div_t qr ;
	qr = div( value, 10 ) ;
	return ( qr.quot << 4 ) + qr.rem ;
}

void writeExtRtcAddress()
{
	ExtRtcI2cRequest.mmr = 0x00680100 ;	// writing, 1 byte addr
	ExtRtcI2cRequest.address = 0 ;
	ExtRtcI2cRequest.dataSize = 1 ;
	ExtRtcI2cRequest.dataBuffer = ( uint8_t*)&ExtRtcI2cRequest.address ;
	ExtRtcI2cRequest.operationType = I2C_WRITE_OP ;
	
	I2cCurrentPointer = &ExtRtcI2cRequest ;
	I2cCurrentPointer->done = 0 ;
	I2C2->CR2 &= ~I2C_CR2_ITEVTEN ;
	I2C2->CR1 |= I2C_CR1_START ;
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return ;
	I2C2->DR = (0x68 << 1) ;
	I2C2->CR2 |= I2C_CR2_ITEVTEN ;
	NVIC_EnableIRQ(I2C2_EV_IRQn) ;
	NVIC_EnableIRQ(I2C2_ER_IRQn) ;
}


uint8_t RtcState ;

void readExtRtc()
{
	if ( RtcState == 0 )
	{
		if ( I2cHeadPointer )
		{
			I2cCurrentPointer = I2cHeadPointer ;
			I2cHeadPointer = I2cHeadPointer->next ;
			I2cCurrentPointer->operationType = I2C_WRITE_OP ;
			I2cCurrentPointer->done = 0 ;
			I2C2->CR2 &= ~I2C_CR2_ITEVTEN ;
			I2C2->CR1 |= I2C_CR1_START ;
  		if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
  		  return ;
			I2C2->DR = (0x68 << 1) ;
			I2C2->CR2 |= I2C_CR2_ITEVTEN ;
			NVIC_EnableIRQ(I2C2_EV_IRQn) ;
			NVIC_EnableIRQ(I2C2_ER_IRQn) ;
			return ;
		}
		writeExtRtcAddress() ;
		RtcState = 1 ;
		return ;
	}
	RtcState = 0 ;
	ExtRtcI2cRequest.mmr = 0x00681100 ;	// reading, 1 byte addr
	ExtRtcI2cRequest.address = 0 ;
	ExtRtcI2cRequest.dataSize = RTC_SIZE ;
	ExtRtcI2cRequest.dataBuffer = ExternalRtc ;
	ExtRtcI2cRequest.operationType = I2C_READ_OP ;
//	ExtRtcI2cRequest.speed = TWI_LOW_SPEED ;
	I2cCurrentPointer = &ExtRtcI2cRequest ;
	I2cCurrentPointer->done = 0 ;
	I2C2->CR2 &= ~I2C_CR2_ITEVTEN ;
	I2C2->CR1 |= I2C_CR1_START ;
  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return ;
//	I2C_State = I2C_STATE_DEVADDR_SENT ;
	I2C2->DR = (0x68 << 1) | 1 ;
	I2C2->CR2 |= I2C_CR2_ITEVTEN ;
	NVIC_EnableIRQ(I2C2_EV_IRQn) ;
	NVIC_EnableIRQ(I2C2_ER_IRQn) ;
}

void writeExtRtc( uint8_t *ptr )
{
	uint32_t year ;
	ExtRtcI2cWriteRequest.mmr = 0x00680100 ;	// writing, 1 byte addr
	ExtRtcI2cWriteRequest.address = 0 ;
	ExtRtcI2cWriteRequest.dataSize = 8 ;
	ExtRtcI2cWriteRequest.dataBuffer = ExternalRtc ;
	ExternalRtc[0] = 0 ;
	ExternalRtc[1] = toBCD( *ptr++ ) ;
	ExternalRtc[2] = toBCD( *ptr++ ) ;
	ExternalRtc[3] = toBCD( *ptr++ ) ;
	ExternalRtc[4] = 1 ;
	ExternalRtc[5] = toBCD( *ptr++ ) ;
	ExternalRtc[6] = toBCD( *ptr++ ) ;
	year = *ptr++ ;
	year |= *ptr << 8 ;
	ExternalRtc[7] = toBCD( year - 2000 ) ;
	ExtRtcI2cWriteRequest.operationType = TWI_WRITE_BUFFER ;
	ExtRtcI2cWriteRequest.speed = TWI_LOW_SPEED ;
	submitI2cRequest( &ExtRtcI2cWriteRequest ) ;
}

void pollForRtcComplete()
{
	if ( ExtRtcI2cRequest.done )
	{
		ExtRtcI2cRequest.done = 0 ;
		if ( RtcState == 0 )
		{
			if ( ExternalRtc[6] )
			{
				Rtc_valid = 1 ;
				// Set the date and time
				t_time *p = &Time ;
				p->second = fromBCD( ExternalRtc[0] & 0x7F ) ;
				p->minute = fromBCD( ExternalRtc[1] & 0x7F ) ;
				p->hour = fromBCD( ExternalRtc[2] & 0x3F ) ;
				p->date = fromBCD( ExternalRtc[4] & 0x3F ) ;
				p->month = fromBCD( ExternalRtc[5] & 0x1F ) ;
				p->year = fromBCD( ExternalRtc[6] ) + 2000 ;
			}
		}
	}
}

void submitI2cRequest( struct t_I2C_request *ptr )
{
	ptr->done = 0 ;
	ptr->next = (struct t_I2C_request *) 0 ;
	if ( I2cHeadPointer )
	{
		I2cTailPointer->next = ptr ;
	}
	else
	{
		I2cHeadPointer = ptr ;
		I2cTailPointer = ptr ;
	}
//	i2c_check_for_request() ;
//	NVIC_EnableIRQ(TWI0_IRQn) ;
}


//uint32_t i2c2_result()
//{
//	return EncoderI2cRequest.done ;
//}

//uint32_t LastI2cEvent ;
//uint32_t LastI2cEvent1 ;
//uint16_t I2cHistory ;
//uint16_t I2cHistoryWhen0 ;
//uint16_t I2cHistoryWhen01 ;

extern "C" void I2C2_EV_IRQHandler( void )
{
  uint32_t lastevent = 0 ;
  uint32_t flag1 = 0, flag2 = 0 ;
//	uint16_t processed = 0 ;

  /* Read the I2Cx status register */
  flag1 = I2C2->SR1 ;
  flag2 = I2C2->SR2 ;	// May clear the ADDR flag!!!
  flag2 = flag2 << 16 ;

  /* Get the last event value from I2C status register */
  lastevent = (flag1 | flag2) ;

	if ( ( lastevent & I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) ==
					I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED )
	{
		I2C2->DR = *I2cCurrentPointer->dataBuffer++ ;
    if ( --(I2cCurrentPointer->dataSize) )
		{
			I2C2->CR2 |= I2C_CR2_ITBUFEN ;
		}
		else
		{
			I2C2->CR2 &= ~I2C_CR2_ITBUFEN ;
		}
//		processed = 1 ;
	}

	if ( I2cCurrentPointer->operationType == I2C_WRITE_OP )
	{
		if ( I2cCurrentPointer->dataSize == 0 )
		{
			if ( ( lastevent & I2C_EVENT_MASTER_BYTE_TRANSMITTED ) ==
					I2C_EVENT_MASTER_BYTE_TRANSMITTED )
			{
				I2cCurrentPointer->done = 1 ;
				NVIC_DisableIRQ(I2C2_EV_IRQn) ;
				NVIC_DisableIRQ(I2C2_ER_IRQn) ;
  			I2C2->CR1 |= I2C_CR1_STOP ;
				I2C2->CR2 &= ~I2C_CR2_ITBUFEN & ~I2C_CR2_ITEVTEN ;
//				processed = 2 ;
			}
		}
		else
		{
			if ( ( lastevent & I2C_EVENT_MASTER_BYTE_TRANSMITTING ) ==
					I2C_EVENT_MASTER_BYTE_TRANSMITTING )
			{
				I2C2->DR = *I2cCurrentPointer->dataBuffer++ ;
  	  	I2cCurrentPointer->dataSize -= 1 ;
//				processed = 3 ;
			}
		}
	}


	if ( ( lastevent & (I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED | 0x00040000) ) ==
					I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED )
	{
//		I2C_debug[1] = lastevent | (I2cCurrentPointer->dataSize << 28) ;

  	if ( I2cCurrentPointer->dataSize > 1)
		{
  	  I2C2->CR1 |= I2C_CR1_ACK;
  	}
		else
		{
  	  I2C2->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK) ;
		}
		I2C2->CR2 |= I2C_CR2_ITBUFEN ;
//		processed = 4 ;
	}
	
//		if ( I2C_State == I2C_STATE_DEVADDR_SENT )
//		{
//			I2C2->DR = I2cCurrentPointer->address ;
//			I2C_State = I2C_STATE_ADDR_0 ;
//		}
//		else if ( I2C_State == I2C_STATE_ADDR_0 )
//		{
//			I2C2->CR1 |= I2C_CR1_START ;
//		  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
//	    return ;
//			I2C_State = I2C_STATE_RECEIVE ;
//		}
//		else if ( I2C_State == I2C_STATE_RECEIVE )
//		{
//  		if ( I2cCurrentPointer->dataSize > 1)
//			{
//  		  I2C2->CR1 |= I2C_CR1_ACK;
//  		}
//			else
//			{
//  		  I2C2->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK) ;
//			}
			
//		}
////		processed = 1 ;
//	}
	
	if ( ( lastevent & I2C_EVENT_MASTER_BYTE_RECEIVED ) ==
					I2C_EVENT_MASTER_BYTE_RECEIVED )
	{
    if ( I2cCurrentPointer->dataSize == 2)
		{
    	I2C2->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK) ;
//			I2C_debug[2] = lastevent | (I2cCurrentPointer->dataSize << 28) ;
    }
		else
		{
//			I2C_debug[3] = lastevent | (I2cCurrentPointer->dataSize << 28) ;
		}
    *I2cCurrentPointer->dataBuffer++ = I2C2->DR ;

    if ( --I2cCurrentPointer->dataSize == 0 )
		{
			I2cCurrentPointer->done = 1 ;
			NVIC_DisableIRQ(I2C2_EV_IRQn) ;
			NVIC_DisableIRQ(I2C2_ER_IRQn) ;
  		I2C2->CR1 |= I2C_CR1_STOP ;
			I2C2->CR2 &= ~I2C_CR2_ITBUFEN & ~I2C_CR2_ITEVTEN ;
		}
//		processed = 5 ;
	}
//	if ( processed == 0 )
//	{
//		I2cHistoryWhen01 = I2cHistoryWhen0 ;
//		I2cHistoryWhen0 = I2cHistory ;
//		LastI2cEvent1 = LastI2cEvent ;
//		LastI2cEvent = lastevent ;

//		NVIC_DisableIRQ(I2C2_EV_IRQn) ;
//		NVIC_DisableIRQ(I2C2_ER_IRQn) ;
// 		I2C2->CR1 |= I2C_CR1_STOP ;

//		I2C_debug[0] = lastevent | 0xF0000000 | (I2cCurrentPointer->dataSize << 24) | ((I2C_debug[4] << 20) & 0x00F00000 ) ;
//		lcd_outhex4(0,0, lastevent >> 16 ) ;
//		lcd_outhex4(30,0, lastevent ) ;
//  	refreshDisplay() ;
//	}
//	else
//	{
//		I2cHistory = ( I2cHistory << 4 ) | processed ;
//	}
}

extern "C" void I2C2_ER_IRQHandler( void )
{
  uint32_t lastevent = 0 ;
  uint32_t flag1 = 0, flag2 = 0 ;

  /* Read the I2Cx status register */
  flag1 = I2C2->SR1 ;
  flag2 = I2C2->SR2 ;
  flag2 = flag2 << 16 ;

  /* Get the last event value from I2C status register */
  lastevent = (flag1 | flag2) ;

	if ( ( lastevent & 0x00030400 ) == 0x00030400 )
	{
//		I2C_debug[6] = lastevent ;
		I2C2->SR1 = 0 ;		// Clear errors
		I2cCurrentPointer->done = 2 ;
		NVIC_DisableIRQ(I2C2_EV_IRQn) ;
		NVIC_DisableIRQ(I2C2_ER_IRQn) ;
 		I2C2->CR1 |= I2C_CR1_STOP ;
	}
	else
	{
//		I2C_debug[7] = lastevent ;
		I2C2->SR1 = 0 ;		// Clear errors
		I2cCurrentPointer->done = 2 ;
		NVIC_DisableIRQ(I2C2_EV_IRQn) ;
		NVIC_DisableIRQ(I2C2_ER_IRQn) ;
 		I2C2->CR1 |= I2C_CR1_STOP ;
	}
}

//static uint32_t read_I2C2( uint8_t *pBuffer, uint8_t devAddress, uint16_t numByteToRead )
//{
//	I2cCurrentPointer = &EncoderI2cRequest ;
//	I2cCurrentPointer->dataBuffer = pBuffer ;
//	I2cCurrentPointer->dataSize = numByteToRead ;
//	I2cCurrentPointer->operationType = I2C_READ_OP ;

//	I2cCurrentPointer->done = 0 ;

////  (void) I2C2->DR ;	// Clear any data left
////  (void) I2C2->SR1 ;
////  (void) I2C2->SR2 ;	// Clear status bits

////	I2C_debug[4] = 0 ;

//	// Generate start
//	I2C2->CR1 |= I2C_CR1_START ;
//  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
//    return 0 ;
////	I2C_debug[0] = I2Clast ;

//  I2C2->DR = (devAddress << 1) | 1 ;

//	// Now use interrupts
//	NVIC_EnableIRQ(I2C2_EV_IRQn) ;
//	NVIC_EnableIRQ(I2C2_ER_IRQn) ;

////  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
////    return 0 ;

////	I2C_debug[1] = I2Clast ;

////  if (numByteToRead > 1)
////	{
////    I2C2->CR1 |= I2C_CR1_ACK;
////  }
////	else
////	{
////    I2C2->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK) ;
////	}
  
////	while (numByteToRead)
////	{
////    if (numByteToRead == 1)
////		{
////    	I2C2->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK) ;
////    }
////    if (!I2C2_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
////      return 0 ;

////    if (numByteToRead == 1)
////		{
////			I2C_debug[3] = I2Clast ;
////		}
////		else
////		{
////			I2C_debug[2] = I2Clast ;
////		}

////    *pBuffer++ = I2C2->DR ;
////    numByteToRead -= 1 ;
////  }

////  I2C2->CR1 |= I2C_CR1_STOP;
////  return 1 ;
//  return 0 ;

//}

//uint32_t write_I2C2( uint8_t *pBuffer, uint8_t devAddress, uint16_t numByteToWrite )
//{
//	I2cCurrentPointer = &EncoderI2cRequest ;
//	I2cCurrentPointer->dataBuffer = pBuffer ;
//	I2cCurrentPointer->dataSize = numByteToWrite ;
//	I2cCurrentPointer->operationType = I2C_WRITE_OP ;
//	I2cCurrentPointer->done = 0 ;

////  (void) I2C2->DR ;	// Clear any data left
////  (void) I2C2->SR1 ;
////  (void) I2C2->SR2 ;	// Clear status bits

////	I2C_debug[4] = 0 ;

//	// Generate start
//	I2C2->CR1 |= I2C_CR1_START ;
//  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
//    return 0 ;
////	I2C_debug[0] = I2Clast ;

//  I2C2->DR = (devAddress << 1) ;	// Write

//	// Now use interrupts
//	NVIC_EnableIRQ(I2C2_EV_IRQn) ;
//	NVIC_EnableIRQ(I2C2_ER_IRQn) ;

////  if (!I2C2_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
////    return 0 ;

////	I2C_debug[1] = I2Clast ;

////  if (numByteToRead > 1)
////	{
////    I2C2->CR1 |= I2C_CR1_ACK;
////  }
////	else
////	{
////    I2C2->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK) ;
////	}
  
////	while (numByteToRead)
////	{
////    if (numByteToRead == 1)
////		{
////    	I2C2->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK) ;
////    }
////    if (!I2C2_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
////      return 0 ;

////    if (numByteToRead == 1)
////		{
////			I2C_debug[3] = I2Clast ;
////		}
////		else
////		{
////			I2C_debug[2] = I2Clast ;
////		}

////    *pBuffer++ = I2C2->DR ;
////    numByteToRead -= 1 ;
////  }

////  I2C2->CR1 |= I2C_CR1_STOP;
////  return 1 ;
//  return 0 ;

//}


//uint32_t hwreadI2cEncoder( uint8_t *ptrData )
//{
//	return read_I2C2( ptrData, I2C_ENCODER_ADDRESS, 2 ) ;
//}

//#if 0
//extern "C" void I2C2_EV_IRQHandler( void )
//{
//  uint32_t lastevent = 0 ;
//  uint32_t flag1 = 0, flag2 = 0 ;
//	uint32_t processed = 0 ;

////	I2C_debug[4] += 1 ;

//  /* Read the I2Cx status register */
//  flag1 = I2C2->SR1 ;
//  flag2 = I2C2->SR2 ;	// May clear the ADDR flag!!!
//  flag2 = flag2 << 16 ;

//  /* Get the last event value from I2C status register */
//  lastevent = (flag1 | flag2) ;

//	switch ( I2C_State )
//	{
//		case I2C_STATE_IDLE			  :
//			NVIC_DisableIRQ(I2C2_EV_IRQn) ;
//			NVIC_DisableIRQ(I2C2_ER_IRQn) ;
//		break ;
//		case I2C_STATE_STARTING	  :
//		if ( ( lastevent & I2C_EVENT_MASTER_MODE_SELECT ) ==
//					I2C_EVENT_MASTER_MODE_SELECT )
//		{
//			uint8_t daddr = I2cCurrentPointer->mmr >> 16 ;
//			uint8_t addrSize = ( I2cCurrentPointer->mmr & I2C_ADDR_COUNT_MASK) >> 8 ;
			
//			if ( addrSize == 0 )
//			{
//				if (I2cCurrentPointer->mmr & I2C_READ_WRITE_MASK)
//				{
//					daddr |= 1 ;		// Read, else write
//				}
//			}
//			I2C_State = I2C_STATE_DEVADDR_SENT ;
//			I2C2->DR = daddr  ;
//			processed = 1 ;
//		}
//		break ;
//		case I2C_STATE_DEVADDR_SENT     :
			
//// Possible next operation
//// No address and read
//// No address and write
//// Address
//		if ( ( lastevent & I2C_EVENT_MASTER_MODE_SELECT ) ==
//					I2C_EVENT_MASTER_MODE_SELECT )
//		{
//			uint8_t addrSize = ( I2cCurrentPointer->mmr & I2C_ADDR_COUNT_MASK) >> 8 ;
//			if ( addrSize )
//			{
//				if ( addrSize == 3 )
//				{
//					I2C_State = I2C_STATE_ADDR_2 ;
//					I2C2->DR = I2cCurrentPointer->address >> 16 ;
//				}
//				else if ( addrSize == 2 )
//				{
//					I2C_State = I2C_STATE_ADDR_1 ;
//					I2C2->DR = I2cCurrentPointer->address >> 8 ;
//				}
//				else
//				{
//					I2C_State = I2C_STATE_ADDR_0 ;
//					I2C2->DR = I2cCurrentPointer->address ;
//				}
//			}
//			else
//			{
//				if (I2cCurrentPointer->mmr & I2C_READ_WRITE_MASK)
//				{
//					// read
//					I2C_State = I2C_STATE_BEGIN_RECEIVE ;
//				}
//				else
//				{
//					// write
//					I2C2->DR = *I2cCurrentPointer->dataBuffer++ ;
//  		  	I2cCurrentPointer->dataSize -= 1 ;
//					I2C_State = I2C_STATE_SEND ;
//				}
				
//			}
//			processed = 1 ;
//		}
//		break ;
//		case I2C_STATE_ADDR_2     :
			
//			I2C_EVENT_MASTER_BYTE_TRANSMITTING
					
//					I2C_State = I2C_STATE_ADDR_1 ;
//					I2C2->DR = I2cCurrentPointer->address >> 8 ;
//		break ;
//		case I2C_STATE_ADDR_1     :
//			I2C_EVENT_MASTER_BYTE_TRANSMITTING
//					I2C_State = I2C_STATE_ADDR_0 ;
//					I2C2->DR = I2cCurrentPointer->address ;
//		break ;
//		case I2C_STATE_ADDR_0     :
//			I2C_EVENT_MASTER_BYTE_TRANSMITTED
//		break ;
//		case I2C_STATE_RESTARTING :
//		break ;
//		case I2C_STATE_BEGIN_RECEIVE    :
//		break ;
//		case I2C_STATE_BEGIN_SEND    :
//		break ;
//		case I2C_STATE_RECEIVE    :
//		break ;
//		case I2C_STATE_SEND       :
//		break ;
			
			
			
			
			
			
			
//	}


//	if ( ( lastevent & I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) ==
//					I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED )
//	{
//		I2C2->DR = *I2cCurrentPointer->dataBuffer++ ;
//    I2cCurrentPointer->dataSize -= 1 ;
//	}

//	if ( I2cCurrentPointer->operationType == I2C_WRITE_OP )
//	{
//		if ( I2cCurrentPointer->dataSize == 0 )
//		{
//			if ( ( lastevent & I2C_EVENT_MASTER_BYTE_TRANSMITTED ) ==
//					I2C_EVENT_MASTER_BYTE_TRANSMITTED )
//			{
//				I2cCurrentPointer->done = 1 ;
//				NVIC_DisableIRQ(I2C2_EV_IRQn) ;
//				NVIC_DisableIRQ(I2C2_ER_IRQn) ;
//  			I2C2->CR1 |= I2C_CR1_STOP ;
//			}
//		}
//		else
//		{
//			if ( ( lastevent & I2C_EVENT_MASTER_BYTE_TRANSMITTING ) ==
//					I2C_EVENT_MASTER_BYTE_TRANSMITTING )
//			{
//				I2C2->DR = *I2cCurrentPointer->dataBuffer++ ;
//  	  	I2cCurrentPointer->dataSize -= 1 ;
//			}
//		}
//	}


//	if ( ( lastevent & I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ) ==
//					I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED )
//	{
////		I2C_debug[1] = lastevent | (I2cCurrentPointer->dataSize << 28) ;
		
//  	if ( I2cCurrentPointer->dataSize > 1)
//		{
//  	  I2C2->CR1 |= I2C_CR1_ACK;
//  	}
//		else
//		{
//  	  I2C2->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK) ;
//		}
//		processed = 1 ;
//	}
	
//	if ( ( lastevent & I2C_EVENT_MASTER_BYTE_RECEIVED ) ==
//					I2C_EVENT_MASTER_BYTE_RECEIVED )
//	{
//    if ( I2cCurrentPointer->dataSize == 2)
//		{
//    	I2C2->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK) ;
////			I2C_debug[2] = lastevent | (I2cCurrentPointer->dataSize << 28) ;
//    }
////		else
////		{
////			I2C_debug[3] = lastevent | (I2cCurrentPointer->dataSize << 28) ;
////		}
//    *I2cCurrentPointer->dataBuffer++ = I2C2->DR ;

//    if ( --I2cCurrentPointer->dataSize == 0 )
//		{
//			I2cCurrentPointer->done = 1 ;
//			NVIC_DisableIRQ(I2C2_EV_IRQn) ;
//			NVIC_DisableIRQ(I2C2_ER_IRQn) ;
//  		I2C2->CR1 |= I2C_CR1_STOP ;
//		}
//		processed = 1 ;
//	}
////	if ( processed == 0 )
////	{
////		I2C_debug[0] = lastevent | 0xF0000000 | (I2cCurrentPointer->dataSize << 24) | ((I2C_debug[4] << 20) & 0x00F00000 ) ;
////	}
//}

//// This is called from an interrupt routine, or
//// interrupts must be disabled while it is called
//// from elsewhere.
//void i2c_check_for_request()
//{
//	if ( I2C_State != I2C_STATE_IDLE )
//	{
//		return ;		// Busy
//	}

//	if ( I2cHeadPointer )
//	{
//		I2cCurrentPointer = I2cHeadPointer ;
//		I2cHeadPointer = I2cHeadPointer->next ;

//		I2cCurrentPointer->done = 0 ;

//		if ( I2cCurrentPointer->operationType == TWI_WRITE_ONE )
//		{
//			I2cCurrentPointer->operationType = TWI_WRITE_BUFFER ;
//			I2cCurrentPointer->dataBuffer = &I2cCurrentPointer->commandByte ;
//			I2cCurrentPointer->dataSize = 1 ;
//		}
//		else
//		{
//			if ( I2cCurrentPointer->operationType == TWI_READ_ONE )
//			{
//				I2cCurrentPointer->operationType = TWI_READ_BUFFER ;
//				I2cCurrentPointer->dataSize = 1 ;
//			}
//		}
//  	if ( I2cCurrentPointer->dataSize > 1)
//		{
//  	  I2C2->CR1 |= I2C_CR1_ACK;
//  	}
//		else
//		{
//  	  I2C2->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK) ;
//		}

//		I2C_State = I2C_STATE_STARTING ;
//		I2C2->CR1 |= I2C_CR1_START ;
		
//		// Now use interrupts
//		NVIC_EnableIRQ(I2C2_EV_IRQn) ;
//		NVIC_EnableIRQ(I2C2_ER_IRQn) ;
//	}
//}
//#endif


