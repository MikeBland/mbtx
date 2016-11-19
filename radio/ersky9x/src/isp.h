/*
  isp.h - part of USBasp

  Autor..........: Thomas Fischl <tfischl@gmx.de> 
  Description....: Provides functions for communication/programming
                   over ISP interface
  Licence........: Free under certain conditions. See Documentation.
  Creation Date..: 2005-02-23
  Last change....: 2005-04-20
*/

#ifndef __isp_h_included__
#define	__isp_h_included__

#ifndef uchar
#define	uchar	unsigned char
#endif

#define ISP_DELAY 1
#define ISP_SCK_SLOW 0
#define ISP_SCK_FAST 1

void resetM64() ;

/* Prepare connection to target device */
void ispConnect();

/* Close connection to target device */
void ispDisconnect();

/* read an write a byte from isp using software (slow) */
uchar ispTransmit(uchar send_byte);

/* read an write a byte from isp using hardware (fast) */
//uchar ispTransmit_hw(uchar send_byte);

/* enter programming mode */
uchar ispEnterProgrammingMode();

/* read byte from eeprom at given address */
uchar ispReadEEPROM(unsigned int address);

/* write byte to flash at given address */
void ispLoadFlashPageByte(unsigned int address, uchar data) ;

void ispProgramPage(unsigned int address) ;

/* read byte from flash at given address */
uchar ispReadFlash(unsigned int address);

/* write byte to eeprom at given address */
uchar ispWriteEEPROM(unsigned int address, uchar data);

/* pointer to sw or hw transmit function */
//uchar (*ispTransmit)(uchar);

/* set SCK speed. call before ispConnect! */
void ispSetSCKOption(uchar sckoption);

/*
  clock.h - part of USBasp

  Autor..........: Thomas Fischl <tfischl@gmx.de> 
  Description....: Provides functions for timing/waiting
  Licence........: Free under certain conditions. See Documentation.
  Creation Date..: 2005-02-23
  Last change....: 2006-11-16
*/

//#define F_CPU           12000000L   /* 12MHz */
//#define TIMERVALUE      TCNT0
//#define CLOCK_T_320us	60

//#ifdef __AVR_ATmega8__
//#define TCCR0B  TCCR0
//#endif

/* set prescaler to 64 */
//#define clockInit()  TCCR0B = (1 << CS01) | (1 << CS00);

/* wait time * 320 us */
void clockWait(uint16_t time) ;

// void checkIspAccess( void ) ;

void ispForceResetOff( void ) ;

uint32_t updateSlave( void ) ;
int32_t newSlaveRevision( void ) ;

#endif /* __isp_h_included__ */
