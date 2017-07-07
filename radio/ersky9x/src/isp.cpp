/*
	Created from:
  isp.c - part of USBasp

  Autor..........: Thomas Fischl <tfischl@gmx.de> 
  Description....: Provides functions for communication/programming
                   over ISP interface
  Licence........: Free under certain conditions. See Documentation.
  Creation Date..: 2005-02-23
  Last change....: 2005-04-20
*/

#include <stdint.h>
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/hal.h"
#include "ersky9x.h"
#include "logicio.h"
#include "isp.h"
#include "timers.h"

// SCK - PA8
// RST - PC10
// MOSI - PB6
// MISO - PB7


#define MOSI_HIGH()  (GPIOB->BSRRL = GPIO_Pin_M64_MOSI)
#define MOSI_LOW()   (GPIOB->BSRRH = GPIO_Pin_M64_MOSI)
#define SCK_HIGH()	 (GPIOA->BSRRL = GPIO_Pin_M64_SCK)
#define SCK_LOW()    (GPIOA->BSRRH = GPIO_Pin_M64_SCK)
#define RST_HIGH()	 (GPIOC->BSRRL = GPIO_Pin_M64_RST)
#define RST_LOW()    (GPIOC->BSRRH = GPIO_Pin_M64_RST)
#define READ_MISO()	 (GPIOB->IDR & GPIO_Pin_M64_MISO )

//void ispSetSCKOption(uchar option)
//{
//  /* use software spi */
//  ispTransmit = ispTransmit_sw ;
//}

const uint8_t SlaveData64[] = {
#include "../../er9x/src/slave64.lbm"
} ;

const uint8_t SlaveData2561[] = {
#include "../../er9x/src/slave2561.lbm"
} ;

int32_t newSlaveRevision()
{
	uint32_t i ;
	int32_t x = -1 ;
	for ( i = 156 ; i < 200 ; i += 1 )
	{
		if ( SlaveData64[i] == 'R' )
		{
			if ( SlaveData64[i+1] == 'e' )
			{
				if ( SlaveData64[i+2] == 'v' )
				{
					x = SlaveData64[i+3] ;
					break ;
				}
			}
		}
	}
	return x ;
}

void ispForceResetOff()
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
	configure_pins( GPIO_Pin_M64_RST, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTC ) ;
	RST_HIGH() ;   /* RST high */
}

#ifdef PCB9XT
uint16_t hwTimerValue() ;
#endif

void ispDelay()
{
	uint16_t startTime ;

	startTime = hwTimerValue() ;

	while( (uint16_t)( hwTimerValue() - startTime ) < 10 )
	{
		// Null body
	}	
}


void ispConnect()
{
	init_hw_timer() ;
	
  /* all ISP pins are inputs before */
  /* now set output pins */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
	configure_pins( GPIO_Pin_M64_RST, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTC ) ;
	configure_pins( GPIO_Pin_M64_SCK, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
	configure_pins( GPIO_Pin_M64_MOSI, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB ) ;

  /* reset device */
	RST_LOW() ;   /* RST low */
  SCK_LOW() ;   /* SCK low */

  /* positive reset pulse > 2 SCK (target) */
  ispDelay();
	RST_HIGH() ;   /* RST high */
	clockWait(64) ;
	RST_LOW() ;   /* RST low */
}

void ispDisconnect()
{
  /* set all ISP pins inputs */
	RST_HIGH() ;   /* RST high */
//	configure_pins( GPIO_Pin_M64_RST, PIN_INPUT | PIN_PORTC ) ;
	configure_pins( GPIO_Pin_M64_SCK, PIN_INPUT | PIN_PORTA ) ;
	configure_pins( GPIO_Pin_M64_MOSI, PIN_INPUT | PIN_PORTB ) ;
}

uchar ispTransmit(uchar send_byte)
{

  uchar rec_byte = 0;
  uchar i;
  for (i = 0; i < 8; i++) {

    /* set MSB to MOSI-pin */
    if ((send_byte & 0x80) != 0)
		{
      MOSI_HIGH() ;  /* MOSI high */
    }
		else
		{
      MOSI_LOW() ; /* MOSI low */
    }
    /* shift to next bit */
    send_byte  = send_byte << 1;

    /* receive data */
    rec_byte = rec_byte << 1;
    if (READ_MISO() != 0) {
      rec_byte++;
    }

    /* pulse SCK */
    SCK_HIGH() ;     /* SCK high */
    ispDelay() ;
    SCK_LOW() ;    /* SCK low */
    ispDelay() ;
  }
  return rec_byte;
}

uchar ispEnterProgrammingMode()
{
  uchar check;
  uchar count = 32 ;

  while (count--)
	{
    ispTransmit(0xAC) ;
    ispTransmit(0x53) ;
    check = ispTransmit(0) ;
    ispTransmit(0) ;
    
    if (check == 0x53)
		{
      return 0;
    }

    /* pulse SCK */
    SCK_HIGH() ;     /* SCK high */
    ispDelay();
    SCK_LOW() ;    /* SCK low */
    ispDelay();
  }
  
  return 1;  /* error: device dosn't answer */
}

uchar ispReadFlash(unsigned int address)
{
  ispTransmit(0x20 | ((address & 1) << 3)) ;
  ispTransmit(address >> 9) ;
  ispTransmit(address >> 1) ;
  return ispTransmit(0) ;
}


void ispLoadFlashPageByte(unsigned int address, uchar data)
{

  /* 0xFF is value after chip erase, so skip programming 
  if (data == 0xFF) {
    return 0;
  }
  */

  ispTransmit(0x40 | ((address & 1) << 3));
  ispTransmit(address >> 9);
  ispTransmit(address >> 1);
  ispTransmit(data);

//  if (pollmode == 0)
//    return 0;

//  if (data == 0x7F)
//	{
//    clockWait(15); /* wait 4,8 ms */
//    return 0;
//  }
//	else
//	{
//    /* polling flash */
//    uchar retries = 30;
//    uint16_t startTime = hwTimerValue() ;
//    while (retries != 0)
//		{
//      if (ispReadFlash(address) != 0x7F)
//			{
//				return 0 ;
//      }
      
//      if ( (uint16_t)( hwTimerValue() - startTime ) > 3200 )
//			{
//				startTime = hwTimerValue() ;
//				retries --;
//      }
//    }
//    return 1; /* error */
//  }
}


void ispProgramPage(unsigned int address)
{
  ispTransmit(0x4C);
  ispTransmit(address >> 9);
  ispTransmit(address >> 1);
  ispTransmit(0);

  clockWait(15);

//  if (pollvalue == 0xFF)
//	{
//    return 0;
//  }
//	else
//	{
//    /* polling flash */
//    uchar retries = 30;
//    uint16_t startTime = hwTimerValue() ;

//    while (retries != 0)
//		{
//      if (ispReadFlash(address) != 0xFF)
//			{
//				return 0;
//      }

//      if ( (uint16_t)( hwTimerValue() - startTime ) > 3200 )
//			{
//				startTime = hwTimerValue() ;
//				retries --;
//      }
//    }
//    return 1; /* error */
//  }
}


//uchar ispReadEEPROM(unsigned int address)
//{
//  ispTransmit(0xA0);
//  ispTransmit(address >> 8);
//  ispTransmit(address);
//  return ispTransmit(0);
//}

uchar ispReadSignature(unsigned int address)
{
  ispTransmit(0x30) ;
  ispTransmit(0) ;
  ispTransmit(address&3) ;
  return ispTransmit(0) ;
}


void ispChipErase()
{
  ispTransmit(0xAC) ;
  ispTransmit(0x80) ;
  ispTransmit(0) ;
  ispTransmit(0) ;
  clockWait(50) ;
}

//uchar ispWriteEEPROM(unsigned int address, uchar data)
//{

//  ispTransmit(0xC0);
//  ispTransmit(address >> 8);
//  ispTransmit(address);
//  ispTransmit(data);

//  clockWait(30); // wait 9,6 ms 

//  return 0;
//  /* 
//  if (data == 0xFF) {
//    clockWait(30); // wait 9,6 ms 
//    return 0;
//  } else {

//    // polling eeprom 
//    uchar retries = 30; // about 9,6 ms 
//    uint16_t starttime = hwTimerValue() ;

//    while (retries != 0)
//		{
//      if (ispReadEEPROM(address) != 0xFF)
//	 		{
//				return 0 ;
//      }

//      if ( (uint16_t)( hwTimerValue() - startTime ) > 3200 )
//			{
//				starttime = hwTimerValue() ;
//				retries-- ;
//      }

//    }
//    return 1; // error 
//  }
//  */

//}

/* wait time * 320 us */
void clockWait(uint16_t time)
{
  uint32_t i ;
  for ( i = 0 ; i < time ; i += 1 )
	{
		hw_delay( 3200 ) ;	// Units of 0.1uS
  }
}

uint8_t IspStatus ;
uint8_t Signature[3] ;
uint8_t M64Program[64] ;

//void checkIspAccess()
//{
//	uint32_t i ;
//	ispConnect() ;
//	clockWait( 64 ) ;	// 20mS
//	if ( ( IspStatus = ispEnterProgrammingMode() ) == 0 )
//	{
//		Signature[0] = ispReadSignature(0) ;
//		Signature[1] = ispReadSignature(1) ;
//		Signature[2] = ispReadSignature(2) ;
//		for ( i = 0 ; i < 64 ; i += 1 )
//		{
//			M64Program[i] = ispReadFlash( i ) ;
//		}
//	}
//	ispDisconnect() ;
//	clockWait(64) ;
//}

void flashPage( uint8_t *data, uint16_t address, uint16_t size )
{
	uint32_t i ;
	for ( i = 0 ; i < size ; i += 1 )
	{
		ispLoadFlashPageByte( address, *data ) ;
		data += 1 ;
		address += 1 ;
	}
	while ( i < 256 )
	{
		ispLoadFlashPageByte( address, 0xFF ) ;
		address += 1 ;
		i += 1 ;
	}
	ispProgramPage( address - 256) ;
}



uint32_t flashAvr( uint8_t *data64, uint8_t *data2561, uint16_t size64, uint16_t size2561 )
{
	uint32_t i ;
	uint8_t *data ;
	uint16_t address ;
	uint16_t size ;
	uint8_t *verifyData ;

	ispConnect() ;
	clockWait( 64 ) ;	// 20mS
	wdt_reset() ;
	if ( ( IspStatus = ispEnterProgrammingMode() ) == 0 )
	{
		Signature[0] = ispReadSignature(0) ;
		Signature[1] = ispReadSignature(1) ;
		Signature[2] = ispReadSignature(2) ;
	}
	// choose file here
	data = data64 ;
	size = size64 ;
	if ( Signature[1] == 0x98 )
	{
		data = data2561 ;
		size = size2561 ;
	}
	verifyData = data ;

	ispChipErase() ;

	if ( Signature[1] == 0x98 )
	{
	  ispTransmit(0x4D) ;
  	ispTransmit(0) ;
	  ispTransmit(0) ;
  	ispTransmit(0) ;
	}

	for ( address = 0 ; address < size ; address += 256 )
	{
		i = 256 ;
		if ( address + 256 > size )
		{
			i = size - address ;
		}
		flashPage( data, address, i ) ;
		data += 256 ;
		wdt_reset() ;
	}
	for ( i = 0 ; i < 64 ; i += 1 )
	{
		M64Program[i] = ispReadFlash( i ) ;
	}
	ispDisconnect() ;
	clockWait(64) ;
	wdt_reset() ;

	for ( i = 0 ; i < 64 ; i += 1 )
	{
		if ( *verifyData++ != M64Program[i] )
		{
			return 1 ;
		}
	}
	return 0 ;
}

uint32_t updateSlave()
{
	return flashAvr( (uint8_t * )SlaveData64, (uint8_t * )SlaveData2561, sizeof(SlaveData64), sizeof(SlaveData2561) ) ;
}

void resetM64()
{
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
//	configure_pins( GPIO_Pin_M64_RST, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTC ) ;
//	RST_LOW() ;   /* RST active */
//	hw_delay( 20000 ) ;	// 2000 uS
//	RST_HIGH() ;   /* RST high */

//	init_hw_timer() ;
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
//	configure_pins( GPIO_Pin_M64_RST, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTC ) ;

	ispConnect() ;
	ispEnterProgrammingMode() ;
//	if ( ( IspStatus = ispEnterProgrammingMode() ) == 0 )
//	{
	ispReadSignature(1) ;
//	}
	ispDisconnect() ;
	clockWait(64) ;
	wdt_reset() ;
void initM64( void ) ;
	initM64() ;
}



