#include "pdi.h"
#ifdef PCBSKY
#include "AT91SAM3S4.h"
#include "core_cm3.h"
#include "ersky9x.h"
#include "myeeprom.h"
#define wdt_reset()	(WDT->WDT_CR = 0xA5000001)
#endif

#if defined(PCBX9D) || defined(PCB9XT)
#include "X9D/stm32f2xx.h"
#include "core_cm3.h"
extern void wdt_reset() ;
#endif

// 9Xtreme
// PPM on PA7
// SPort enable on PB2
// SPort data on PA2

//Taranis JTMS/JTCK usage
//Pads:
//1. Vcc
//2. JTMS - PA13 - use for PDI DATA
//3. JTCK - PA14 - use for PDI CLK
//4. Ground


#include <string.h>

#define NVM_COMMAND_REG	(XNVM_DATA_BASE+XNVM_CONTROLLER_BASE+XNVM_CONTROLLER_CMD_REG_OFFSET)
#define NVM_CTRLA_REG   (XNVM_DATA_BASE+XNVM_CONTROLLER_BASE+XNVM_CONTROLLER_CTRLA_REG_OFFSET)

uchar PdiSky = 0 ;

#ifdef PCB9XT
uchar PdiErrors0 = 0 ;
uchar PdiErrors1 = 0 ;
uchar PdiErrors2 = 0 ;
uchar PdiErrors3 = 0 ;
uchar PdiErrors4 = 0 ;
uchar PdiErrors5 = 0 ;
uchar PdiErrors6 = 0 ;
uchar PdiErrors7 = 0 ;
uchar PdiErrors8 = 0 ;
#endif

const static uint8_t pdi_key[8]={0xFF,0x88,0xD8,0xCD,0x45,0xAB,0x89,0x12};

#ifdef PCBSKY
// Timer now clocked at 8MHz (0.125uS) or 16MHz
void hw_delay( uint16_t time )
{
	time *= HwDelayScale ;
	TC0->TC_CHANNEL[0].TC_CCR = 5 ;	// Enable clock and trigger it (may only need trigger)
	while ( TC0->TC_CHANNEL[0].TC_CV < time )		// Value depends on MCK/2 (used 18MHz)
	{
		// Wait
	}
}
#endif

#if defined(PCBX9D) || defined(PCB9XT)
extern void hw_delay( uint16_t time ) ;
#endif

static void pdiWaitBit()
{
#ifdef PCBSKY
	hw_delay(3*HW_COUNT_PER_US) ;	// 3 uS
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	hw_delay(35) ;	// 3.5 uS
#endif
}

uchar pdiInit()
{
#if defined(PCBX9D) || defined(PCB9XT)
	uint32_t temp ;
#endif
//12MHz/256 -> 46k
//	TCCR2=(0<<CS22)|(0<CS21)|(1<<CS20)|(0<<COM21)|(0<<COM20)|(1<<WGM21)|(0<<WGM20);
//	OCR2=32;
//	PORTB |= (1<<3); PORTB &= ~((1<<4)|(1<<5)|(1<<2));
//	DDRB |= (1<<3)|(1<<2); DDRB &= ~((1<<4)|(1<<5));

	
#ifdef PCBSKY
// check for REVX and also g_eeGeneral.ar9xBoard == 1 ;
#ifdef REVX
  PMC->PMC_PCER0 = 1<<ID_PIOA ;	// Enable clock to PIOA
	PIOA->PIO_PER = 0x02000000L | 0x00000040 | 0x00020000 ;
	PIOA->PIO_OER = 0x02000000L | 0x00000040 | 0x00020000 ;
	PIOA->PIO_MDDR = PIO_PA17 ;			// Push Pull O/p in A17
	PIOA->PIO_SODR = 0x02000000L ;	// Set bit A25 ON, enable SPort output
#else
// Use PA5 for inverted clock on AR9X, non-inverted for SKY
  PMC->PMC_PCER0 = 1<<ID_PIOA ;	// Enable clock to PIOA
	PIOA->PIO_PER = 0x00000040 | 0x00020000 ;
	PIOA->PIO_OER = 0x00000040 | 0x00020000 ;
	PIOA->PIO_MDDR = PIO_PA17 ;			// Push Pull O/p in A17
	if ( g_eeGeneral.ar9xBoard == 0 )
	{
		PdiSky = 1 ;
	}
#endif

#endif

#ifdef PCB9XT
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
	temp = GPIOA->MODER ;
	temp &= 0xFFFFF3FCF ;
	temp |= 0x00004010 ;
	GPIOA->MODER = temp ;
	temp = GPIOA->OSPEEDR ;
	temp &= 0xFFFFF3FCF ;
	temp |= 0x00008020 ;
	GPIOA->OSPEEDR = temp ;
	GPIOA->OTYPER &= 0x0000FF7B ;
	temp = GPIOB->MODER ;
	temp &= 0xFFFFFFCF ;
	temp |= 0x00000010 ;
	GPIOB->MODER = temp ;
	GPIOB->OTYPER &= 0x0000FFFB ;
	GPIOB->BSRRL = 0x0004 ;	// Set bit PB2 ON, enable SPort output
#endif

#ifdef PCBX9D
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
	temp = GPIOA->MODER ;
	temp &= 0xC3FFFFFF ;
	temp |= 0x14000000 ;
	GPIOA->MODER = temp ;
	temp = GPIOA->OSPEEDR ;
	temp &= 0xC3FFFFFF ;
	temp |= 0x28000000 ;
	GPIOA->OSPEEDR = temp ;
	GPIOA->PUPDR &= 0xC3FFFFFF ;
	GPIOA->OTYPER &= 0x00009FFF ;
#endif
	 
	pdiSetClk1();
	pdiSetData0();
#ifdef PCBSKY
	hw_delay(110*HW_COUNT_PER_US) ;	// 110 uS
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	hw_delay(2200) ;	// 220 uS
#endif

	pdiSetData1();
//	pdiSetClk1();
#ifdef PCBSKY
	hw_delay(10*HW_COUNT_PER_US) ;	// 10uS
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	hw_delay(250) ;	// 25uS
#endif
	pdiSendIdle();
	pdiSendIdle();

	pdiResetDev(1);

	uchar buf[9];
	buf[0]=XNVM_PDI_KEY_INSTR;
	memcpy(&buf[1],pdi_key,8);
	pdiSendBytes(buf,9);

	pdiSendIdle();

	pdiSendByte(XNVM_PDI_STCS_INSTR | XOCD_CTRL_REGISTER_ADDRESS);
	pdiSendByte(3);

	pdiSendIdle();

	uchar status = pdiWaitNVM() ;
	if (status==PDI_STATUS_OK)
	{
		pdiEnableTimerClock();
//		return PDI_STATUS_OK;
	}

#ifdef PCB9XT
	if (status != PDI_STATUS_OK )
	{
		PdiErrors0 += 1 ;
	}
#endif
	
	return status;
}

void pdiCleanup()
{
	pdiDisableTimerClock();
	pdiSendIdle();
	pdiResetDev(0);

#ifdef PCBSKY
#ifdef REVX
	PIOA->PIO_CODR = 0x02000000L ;	// Set bit A25 ON, enable SPort output
	PIOA->PIO_PDR = 0x02000000L | 0x00000040 | 0x00020000 ;
#else
	PIOA->PIO_PDR = 0x00000040 | 0x00020000 ;
#endif
#endif

#ifdef PCB9XT
	GPIOB->BSRRH = 0x0004 ;	// Set bit PB2 OFF, disable SPort output
#endif


//	DDRB &= ~((1<<3)|(1<<5)|(1<<2)|(1<<4));
//	PORTB &= ~((1<<3)|(1<<5)|(1<<2)|(1<<4));

//	switch(keep_reset)
//	{
//		case EXIT_RESET_DISABLED:
//			DDRB |= (1<<3);
//			pdiSetClk0();
//			hw_delay(10000) ; // 1 mS
//			pdiSetClk1();
//		break;

//		case EXIT_RESET_ENABLED:
//			DDRB |= (1<<3);
//			pdiSetClk0();
//		break;
//	}
}

void pdiEnableTimerClock()
{
	pdiSetData1();
//	TCCR2=(0<<CS22)|(0<CS21)|(1<<CS20)|(0<<COM21)|(1<<COM20)|(1<<WGM21)|(0<<WGM20);
}

void pdiDisableTimerClock()
{
	pdiSetData1();
//	TCCR2=(0<<CS22)|(0<CS21)|(1<<CS20)|(0<<COM21)|(0<<COM20)|(1<<WGM21)|(0<<WGM20);
}

uchar pdiTimerClockEnabled()
{
//	return (TCCR2 & (1<<COM20));
	return 1 ;
}

void pdiSetClk1()
{
// 9XR-PRO
#ifdef PCBSKY
#ifdef REVX
	PIOA->PIO_CODR = 0x00000040 ;
#else
  if ( PdiSky )
	{
		PIOA->PIO_SODR = 0x00000040 ;
	}
	else
	{
		PIOA->PIO_CODR = 0x00000040 ;
	}
#endif
#endif
#ifdef PCB9XT
	GPIOA->BSRRL = 0x0004 ;
#endif
//	PORTB|=(1<<3);
#ifdef PCBX9D
	GPIOA->BSRRH = 0x2000 ;
#endif
}

void pdiSetClk0()
{
// 9XR-PRO
#ifdef PCBSKY
#ifdef REVX
	PIOA->PIO_SODR = 0x00000040 ;
#else
  if ( PdiSky )
	{
		PIOA->PIO_CODR = 0x00000040 ;
	}
	else
	{
		PIOA->PIO_SODR = 0x00000040 ;
	}
#endif
#endif
#ifdef PCB9XT
	GPIOA->BSRRH = 0x0004 ;
#endif
//	PORTB&=~(1<<3);
#ifdef PCBX9D
	GPIOA->BSRRL = 0x2000 ;
#endif
}

void pdiSetData1()
{
// 9XR-PRO
#ifdef PCBSKY
	PIOA->PIO_SODR = 0x00020000 ;
	PIOA->PIO_OER = 0x00020000 ;
#endif
#ifdef PCB9XT
	GPIOA->BSRRL = 0x0080 ;
	GPIOA->MODER |= 0x00004000 ;
#endif
#ifdef PCBX9D
	GPIOA->BSRRL = 0x4000 ;
	GPIOA->MODER |= 0x10000000 ;
#endif


//	PORTB|=(1<<5);
//	PORTB&=~(1<<2);
//	DDRB|=(1<<5)|(1<<2);
}

void pdiSetData0()
{
// 9XR-PRO
#ifdef PCBSKY
	PIOA->PIO_CODR = 0x00020000 ;
	PIOA->PIO_OER = 0x00020000 ;
#endif
#ifdef PCB9XT
	GPIOA->BSRRH = 0x0080 ;
	GPIOA->MODER |= 0x00004000 ;
#endif
#ifdef PCBX9D
	GPIOA->BSRRH = 0x4000 ;
	GPIOA->MODER |= 0x10000000 ;
#endif
	
//	PORTB&=~((1<<5)|(1<<2));
//	DDRB|=(1<<5)|(1<<2);
}

void pdiSetDataIn()
{
// 9XR-PRO
#ifdef PCBSKY
	PIOA->PIO_ODR = 0x00020000 ;
#endif
#ifdef PCB9XT
	GPIOA->MODER &= 0xFFFF3FFF ;
#endif
#ifdef PCBX9D
	GPIOA->MODER &= 0xCFFFFFFF ;
#endif
	
//	PORTB&=~(1<<2);
//	DDRB&=~((1<<5)|(1<<2));
//	PORTB&=~(1<<5);
}

uchar pdiGetData()
{
// 9XR-PRO
#ifdef PCBSKY
	return (PIOA->PIO_PDSR & 0x00020000) ? 1 : 0 ;
#endif
#ifdef PCB9XT
	return (GPIOA->IDR & 0x0080) ? 1 : 0 ;
#endif
#ifdef PCBX9D
	return (GPIOA->IDR & 0x4000) ? 1 : 0 ;
#endif
//	return (PINB&(1<<4))?1:0;
}

void pdiSendIdle()
{
	pdiSendByteX(255,255);
}

void pdiSendBreak()
{
	pdiSendByteX(0,0);
}

#define PDI_BIT_START 1
#define PDI_BIT_PARITY 2
#define PDI_BIT_STOP 4

uchar byteParity(uchar b)
{
	b = b ^ (b >> 4) ;
	b = b ^ (b >> 2) ;
	b = b ^ (b >> 1) ;
	return b & 1 ;
}

void pdiSendByte(uchar b)
{
	uchar extra=PDI_BIT_STOP;
	if (byteParity(b)) extra|=PDI_BIT_PARITY;
	pdiSendByteX(b,extra);
}

void pdiSendBytes(uchar* ptr, uint16_t count )
{
	for(;count>0;count--,ptr++)
		pdiSendByte(*ptr);
}

void pdiSendByteX(uchar byte,uchar extra)
{
	pdiSetClk0();
	if (extra&PDI_BIT_START) pdiSetData1(); else pdiSetData0();
	pdiWaitBit();
	pdiSetClk1();
	pdiWaitBit();

	uchar bit;
	for(bit=1;bit;bit<<=1)
	{
		pdiSetClk0();
		if (byte & bit) pdiSetData1(); else pdiSetData0();
		pdiWaitBit();
		pdiSetClk1();
		pdiWaitBit();
	}

	pdiSetClk0();
	if (extra&PDI_BIT_PARITY) pdiSetData1(); else pdiSetData0();
	pdiWaitBit();
	pdiSetClk1();
	pdiWaitBit();

	pdiSetClk0();
	if (extra&PDI_BIT_STOP) pdiSetData1(); else pdiSetData0();
	pdiWaitBit();
	pdiSetClk1();
	pdiWaitBit();

	pdiSetClk0();
	if (extra&PDI_BIT_STOP) pdiSetData1(); else pdiSetData0();
	pdiWaitBit();
	pdiSetClk1();
	pdiWaitBit();
}

uchar pdiReadByte(uchar timeout,uchar *result)
{
	uchar in=PDI_STATUS_TIMEOUT,ret=PDI_STATUS_OK;
	pdiSetDataIn();
	for(;timeout>0;timeout--)
	{
		pdiSetClk0();
//		pdiSetDataIn();
		pdiWaitBit();
		pdiSetClk1();
		in=pdiGetData();
		pdiWaitBit();
		if (in==0) break;
	}
#ifdef PCB9XT
	if (in )
	{
		PdiErrors1 += 1 ;
	}
#endif

	if (in) return PDI_STATUS_TIMEOUT;

	uchar bit,byte=0,parity;
	for(bit=1;bit;bit<<=1)
	{
		pdiSetClk0();
		pdiWaitBit();
		pdiSetClk1();
		if (pdiGetData()) byte|=bit;
		pdiWaitBit();
	}
	*result=byte;

	pdiSetClk0();
	pdiWaitBit();
	pdiSetClk1();
	parity=pdiGetData();
	pdiWaitBit();

	if (parity!=byteParity(byte)) ret=PDI_STATUS_PARITY;

	pdiSetClk0();
	pdiWaitBit();
	pdiSetClk1();
	if ((ret==PDI_STATUS_OK)&&(!pdiGetData())) ret=PDI_STATUS_BADSTOP;
	pdiWaitBit();

	pdiSetClk0();
	pdiWaitBit();
	pdiSetClk1();
	if ((ret==PDI_STATUS_OK)&&(!pdiGetData())) ret=PDI_STATUS_BADSTOP;
	pdiWaitBit();

#ifdef PCB9XT
	if (ret != PDI_STATUS_OK )
	{
		PdiErrors2 += 1 ;
	}
#endif
	return ret;
}

uchar pdiReadCtrl(uint32_t addr, uchar *value)
{
	uchar ret;
	uchar buf[5];

	buf[0]=XNVM_PDI_LDS_INSTR | XNVM_PDI_LONG_ADDRESS_MASK | XNVM_PDI_BYTE_DATA_MASK;
	memmove(buf+1,&addr,4);
	pdiSendBytes(buf,5);
	ret = pdiReadByte(250,value);
	pdiSendBreak();
	pdiSendIdle();
	pdiSendBreak();
	pdiSendIdle();
	return ret;
}

uchar pdiWaitNVM()
{
	uchar retry=100;
	for(;retry>0;retry--)
	{
	 	wdt_reset() ;
		uchar status;
		if (pdiReadCtrl(XNVM_CONTROLLER_BASE+XNVM_DATA_BASE+XNVM_CONTROLLER_STATUS_REG_OFFSET, &status)==PDI_STATUS_OK)
		{
			if ((status & XNVM_NVM_BUSY)==0)
			{
				return PDI_STATUS_OK;
			}
		}
	}
#ifdef PCB9XT
	PdiErrors3 += 1 ;
#endif
	return PDI_STATUS_NVM_TIMEOUT;
}

uchar pdiWriteCtrl(uint32_t addr,uint8_t value)
{
	uchar cmd[6];
	cmd[0]= XNVM_PDI_STS_INSTR | XNVM_PDI_LONG_ADDRESS_MASK | XNVM_PDI_BYTE_DATA_MASK;
	memmove(cmd+1,&addr,4);
	cmd[5]=value;
	pdiSendBytes(cmd,6);
	return PDI_STATUS_OK;
}

uchar pdiResetDev(uchar reset)
{
	uchar buf[2];
	buf[0]=XNVM_PDI_STCS_INSTR | XOCD_RESET_REGISTER_ADDRESS;
	buf[1]=reset?XOCD_RESET_SIGNATURE:0;
	pdiSendBytes(buf,2);
	return PDI_STATUS_OK;
}

uchar pdiChipErase()
{
	uint8_t status ;
	uint32_t retryCount ;

	pdiWriteCtrl( NVM_COMMAND_REG, XNVM_CMD_CHIP_ERASE ) ;
	/* Write the CMDEX to execute command */
	pdiWriteCtrl( NVM_CTRLA_REG, XNVM_CTRLA_CMDEX ) ;
	
	/* Wait until the NVM controller is no longer busy */
	retryCount = 10 ;
	while ( retryCount )
	{
		status = pdiWaitNVM() ;
		if (status == PDI_STATUS_OK )
		{
			break ;
		}
		retryCount -= 1 ;
	}
	if (status != PDI_STATUS_OK )
	{
		pdiEnableTimerClock() ;
	}	
	return status ;
}


uchar pdiWriteFuse( uint32_t address, uint8_t value )
{
	uchar cmd[6] ;
	uint32_t register_address;

	pdiWriteCtrl( NVM_COMMAND_REG, XNVM_CMD_WRITE_FUSE ) ;

	cmd[0] = XNVM_PDI_STS_INSTR | XNVM_PDI_LONG_ADDRESS_MASK | XNVM_PDI_BYTE_DATA_MASK ;

	register_address = XNVM_FUSE_BASE + address ;

	memmove((cmd + 1), (uint8_t*)&register_address, 4);
	cmd[5] = value;

	pdiSendBytes(cmd,6);

	uchar status = pdiWaitNVM() ;
	if (status==PDI_STATUS_OK)
	{
		pdiEnableTimerClock();
	}
	return status;
}


uchar pdiSetPointer(uint32_t addr)
{
	uchar cmd[5];
	cmd[0]=XNVM_PDI_ST_INSTR | XNVM_PDI_LD_PTR_ADDRESS_MASK | XNVM_PDI_LONG_DATA_MASK;
	memmove(cmd+1,&addr,4);
	pdiSendBytes(cmd,5);
	return 0;
}

uchar pdiReadBlock(uint32_t addr,uchar* data,uchar len)
{
	uchar ret=PDI_STATUS_OK;

	uchar retry=20;
	for(;retry>0;retry--)
	{
	 	wdt_reset() ;
		pdiWriteCtrl(XNVM_DATA_BASE+XNVM_CONTROLLER_BASE
		     +XNVM_CONTROLLER_CMD_REG_OFFSET,XNVM_CMD_READ_NVM_PDI);
		pdiSetPointer(addr);

		if (len>1)
		{
			pdiSendByte(XNVM_PDI_REPEAT_INSTR | XNVM_PDI_BYTE_DATA_MASK);
			pdiSendByte(len-1);
		}

		pdiSendByte(XNVM_PDI_LD_INSTR | XNVM_PDI_LD_PTR_STAR_INC_MASK | XNVM_PDI_BYTE_DATA_MASK);

		uchar *dst=data;
		uchar i;
		for(i=0;i<len;i++,dst++)
		{
			ret=pdiReadByte(250,dst);
			if (ret!=PDI_STATUS_OK) break;
		}
		pdiSendBreak();
		pdiSendIdle();
		pdiSendBreak();
		pdiSendIdle();
			if (ret==PDI_STATUS_OK) break;
	}

#ifdef PCB9XT
	if ( ret != PDI_STATUS_OK )
	{
		PdiErrors4 += 1 ;
	}
#endif
	return ret;
}


//		#define XMEGA_CRC_LENGTH_BYTES               3

//		#define XMEGA_NVM_REG_ADDR0                  0x00
//		#define XMEGA_NVM_REG_ADDR1                  0x01
//		#define XMEGA_NVM_REG_ADDR2                  0x02
//		#define XMEGA_NVM_REG_DAT0                   0x04
//		#define XMEGA_NVM_REG_DAT1                   0x05
//		#define XMEGA_NVM_REG_DAT2                   0x06
//		#define XMEGA_NVM_REG_CMD                    0x0A
//		#define XMEGA_NVM_REG_CTRLA                  0x0B
//		#define XMEGA_NVM_REG_CTRLB                  0x0C
//		#define XMEGA_NVM_REG_INTCTRL                0x0D
//		#define XMEGA_NVM_REG_STATUS                 0x0F
//		#define XMEGA_NVM_REG_LOCKBITS               0x10

//		#define XMEGA_NVM_BIT_CTRLA_CMDEX            (1 << 0)

//		#define XMEGA_NVM_CMD_NOOP                   0x00
//		#define XMEGA_NVM_CMD_CHIPERASE              0x40
//		#define XMEGA_NVM_CMD_READNVM                0x43
//		#define XMEGA_NVM_CMD_LOADFLASHPAGEBUFF      0x23
//		#define XMEGA_NVM_CMD_ERASEFLASHPAGEBUFF     0x26
//		#define XMEGA_NVM_CMD_ERASEFLASHPAGE         0x2B
//		#define XMEGA_NVM_CMD_WRITEFLASHPAGE         0x2E
//		#define XMEGA_NVM_CMD_ERASEWRITEFLASH        0x2F
//		#define XMEGA_NVM_CMD_FLASHCRC               0x78
//		#define XMEGA_NVM_CMD_ERASEAPPSEC            0x20
//		#define XMEGA_NVM_CMD_ERASEAPPSECPAGE        0x22
//		#define XMEGA_NVM_CMD_WRITEAPPSECPAGE        0x24
//		#define XMEGA_NVM_CMD_ERASEWRITEAPPSECPAGE   0x25
//		#define XMEGA_NVM_CMD_APPCRC                 0x38
//		#define XMEGA_NVM_CMD_ERASEBOOTSEC           0x68
//		#define XMEGA_NVM_CMD_ERASEBOOTSECPAGE       0x2A
//		#define XMEGA_NVM_CMD_WRITEBOOTSECPAGE       0x2C
//		#define XMEGA_NVM_CMD_ERASEWRITEBOOTSECPAGE  0x2D
//		#define XMEGA_NVM_CMD_BOOTCRC                0x39
//		#define XMEGA_NVM_CMD_READUSERSIG            0x03
//		#define XMEGA_NVM_CMD_ERASEUSERSIG           0x18
//		#define XMEGA_NVM_CMD_WRITEUSERSIG           0x1A
//		#define XMEGA_NVM_CMD_READCALIBRATION        0x02
//		#define XMEGA_NVM_CMD_READFUSE               0x07
//		#define XMEGA_NVM_CMD_WRITEFUSE              0x4C
//		#define XMEGA_NVM_CMD_WRITELOCK              0x08
//		#define XMEGA_NVM_CMD_LOADEEPROMPAGEBUFF     0x33
//		#define XMEGA_NVM_CMD_ERASEEEPROMPAGEBUFF    0x36
//		#define XMEGA_NVM_CMD_ERASEEEPROM            0x30
//		#define XMEGA_NVM_CMD_ERASEEEPROMPAGE        0x32
//		#define XMEGA_NVM_CMD_WRITEEEPROMPAGE        0x34
//		#define XMEGA_NVM_CMD_ERASEWRITEEEPROMPAGE   0x35
//		#define XMEGA_NVM_CMD_READEEPROM             0x06
///** Writes page addressed memory to the target's memory spaces.
// *
// *  \param[in]  WriteBuffCommand  Command to send to the device to write a byte to the memory page buffer
// *  \param[in]  EraseBuffCommand  Command to send to the device to erase the memory page buffer
// *  \param[in]  WritePageCommand  Command to send to the device to write the page buffer to the destination memory
// *  \param[in]  PageMode          Bitfield indicating what operations need to be executed on the specified page
// *  \param[in]  WriteAddress      Start address to write the page data to within the target's address space
// *  \param[in]  WriteBuffer       Buffer to source data from
// *  \param[in]  WriteSize         Number of bytes to write
// *
// *  \return Boolean \c true if the command sequence complete successfully
// */
//bool pdiWritePageMemory(const uint8_t WriteBuffCommand,
//                              const uint8_t EraseBuffCommand,
//                              const uint8_t WritePageCommand,
//                              const uint8_t PageMode,
//                              const uint32_t WriteAddress,
//                              const uint8_t* WriteBuffer,
//                              uint16_t WriteSize)


uint8_t pdiWritePageMemory( uint32_t writeAddress, uint8_t* writeBuffer, uint16_t writeSize )
{
	// Wait until the NVM controller is no longer busy
	uint8_t status = pdiWaitNVM() ;
	uint32_t boot = writeAddress >= 0x8000 ? 1 : 0 ;
	if (status != PDI_STATUS_OK )
	{
		pdiEnableTimerClock();
#ifdef PCB9XT
		PdiErrors5 += 1 ;
#endif
		return status;
	}	

	writeAddress += XNVM_FLASH_BASE ;
	// Send the memory buffer erase command to the target
	pdiSetPointer( 0 ) ;
	pdiWriteCtrl( NVM_COMMAND_REG, XNVM_CMD_ERASE_FLASH_PAGE_BUFFER ) ;

	// Set CMDEX bit in NVM CTRLA register to start the buffer erase
	pdiWriteCtrl( NVM_CTRLA_REG, XNVM_CTRLA_CMDEX ) ;

	if (writeSize)
	{
		/* Wait until the NVM controller is no longer busy */
		status = pdiWaitNVM() ;
		if (status != PDI_STATUS_OK )
		{
			pdiEnableTimerClock() ;
#ifdef PCB9XT
			PdiErrors6 += 1 ;
#endif
			return status ;
		}	

		// Send the memory buffer write command to the target
		pdiWriteCtrl( NVM_COMMAND_REG, XNVM_CMD_LOAD_FLASH_PAGE_BUFFER ) ;

		// Load the PDI pointer register with the start address we want to write to
		pdiSetPointer( writeAddress ) ;

		// Send the REPEAT command with the specified number of bytes to write
		pdiSendByte(XNVM_PDI_REPEAT_INSTR | XNVM_PDI_BYTE_DATA_MASK);
		pdiSendByte(writeSize-1);

		// Send a ST command with indirect access and post-increment to write the bytes */
		pdiSendByte(XNVM_PDI_ST_INSTR | XNVM_PDI_LD_PTR_STAR_INC_MASK | XNVM_PDI_BYTE_DATA_MASK) ;
		pdiSendBytes( writeBuffer, writeSize ) ;
	}

	status = pdiWaitNVM() ;
#ifdef PCB9XT
	if (status != PDI_STATUS_OK )
	{
		PdiErrors7 += 1 ;
	}
#endif
	if (status != PDI_STATUS_OK )
	{
		pdiEnableTimerClock() ;
		return status ;
	}	
	// Send the memory write command to the target */
	if ( boot )
	{
		pdiWriteCtrl( NVM_COMMAND_REG, XNVM_CMD_ERASE_AND_WRITE_BOOT_PAGE) ;
	}
	else
	{
		pdiWriteCtrl( NVM_COMMAND_REG, XNVM_CMD_ERASE_AND_WRITE_APP_SECTION) ;
	}
	// Send the address of the first page location to write the memory page */
	pdiWriteCtrl( writeAddress, 0 ) ;
	
	/* Wait until the NVM controller is no longer busy */
	status = pdiWaitNVM() ;
	if (status != PDI_STATUS_OK )
	{
		pdiEnableTimerClock() ;
	}	
#ifdef PCB9XT
	if (status != PDI_STATUS_OK )
	{
		PdiErrors8 += 1 ;
	}
#endif
	return status ;
}

//Erase and program the flash memory
//1. Erase the flash page buffer.
//i. Use the “ST ptr” command to set the address 0x00000000.
//ii. Write “Erase Flash Page Buffer” (0x26) command to the NVM controller.
//iii. Dummy write the flash page buffer with “ST *(ptr++)” command.
//iv. Polling the NVM busy bit until it has been cleared.

//2. Load the flash page buffer.
//i. Write the “Load Flash Page Buffer” (0x23) command to the NVM controller.
//ii. Use the “ST ptr” command to set the start address.
//iii. Set the data length into the repeat counter with “REPEAT” command.
//iv. Send the “ST *(ptr++)” command to the PDI controller.
//v. Send the writing data through the PDIBUS.

//3. Write “Erase & Write Application Section Page” (0x25) command to the NVM controller.
//4. Use the “ST ptr” command to set the start address.
//5. Dummy write the flash page with “ST *(ptr++)” command.
//6. Polling the NVM busy bit until it has been cleared.


