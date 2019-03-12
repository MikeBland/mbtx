/****************************************************************************
*  Copyright (c) 2018 by Michael Blandford. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License version 2 as
*	published by the Free Software Foundation.
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
****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef PCBSKY
#include "AT91SAM3S4.h"
#endif

#ifdef PCB9XT
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/i2c_9xt.h"
#endif

#ifdef PCBX9D
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#endif


#ifndef SIMU
#include "CoOS.h"
#endif


#include "ersky9x.h"
#include "myeeprom.h"
#include "drivers.h"
#include "bluetooth.h"
#include "sbus.h"
#include "audio.h"
#include "logicio.h"

// Strings for FrSky BT module (HM-10?)
//AT\r\n
//AT+NAME?\r\n
//AT+ADDR?\r\n
//AT+ROLE?\r\n
//AT+ROLE0\r\n
//AT+VERS?\r\n
//AT+CLEAR\r\n
//AT+RESET\r\n
//AT+CONxxxxxxxxxxxx\r\n
//AT+DISC?\r\n
//AT+CONNX\r\n
//AT+IMME?\r\n
//AT+IMME1\r\n
//AT+STAT?\r\n
//AT+TXPW0\r\n

// Sample responses
//Central:7CEC796B210A
//OK+ROLE:1
//OK+STAT:64
//OK+NAME:FrSkyBT
//OK+DISCS


//OK+
//AT+
//OK+DISCE
//OK+DISC:
//Connected
//Central
//Peripheral
//DisConnected
//B4994C761243
//0:/FIRMWARE
//AT+NAME?
//AT+ADDR?
//AT+ROLE?
//AT+ROLE0
//AT+VERS?
//AT+CLEAR
//AT+RESET
//AT+CONxxxxxxxxxxxx
//AT+DISC?
//AT+CONNX
//AT+IMME?
//AT+IMME1
//AT+STAT?
//AT+TXPW0




//HM-10 default 9600,8,n,1 - PSW = 000000
//HM-10 doesn't need /r/n
//AT+NAME? - OK+NAMEnnnnnnnn
//AT+BAUD?
//AT+BAUD4 - OK_set4  (0-9600, 1-19200, 2-38400, 3-57600, 4-115200 5,6,7,8
//AT+ROLE?
//AT+ROLE0, AT+ROLE1 - 0-slave, 1-master

// X9E module
// TTM:CIT-Xms X= "20", "50", "100", "200", "300", "400", "500", "1000", "1500", or "2000" 
// TTM:REN-nnnnn   Name
// TTM:BPS-115200
// TTM:MAC-?
// TTM:PID-"+ Data   where “Data”is for a two-byte product identification code (ranging from 0x0000 range to 0xFFFF


// Getting Master working
// AT+RMAAD - clears paired list
// AT+ROLE=1
// AT+RESET - may be needed
// AT+CMODE=0 - search specific
// AT+INQM=0,5,4 - 5 devices, 4*1.28 seconds search time
// (AT+PSWD=1234) - default value
// AT+INIT - may be needed (Error:(17) allowed
// AT+INQ - searches
// Replies:
// +INQ:address,type,signal
// Pick up address ad pass to:
// AT_RNAME?address
// Replies:
// +RNAME:name
// AT+PAIR=address,timeout
// AT+BIND=address - may not be needed
// AT+LINK=address



uint8_t BtType ;
uint8_t BtCurrentFunction ;
uint8_t BtEepromFunction ;
struct t_fifo128 Bt_fifo ;
uint8_t BtTxBuffer[64] ;
struct t_bt_control BtControl ;
struct btRemote_t BtRemote[3] ;
uint8_t BtBinAddr[6] ;
uint8_t BtPswd[8] ;
uint8_t BtName[16] ;
uint8_t BtRname[32] ;

OS_FlagID Bt_flag ;
struct t_serial_tx Bt_tx ;
struct t_serial_tx Com2_tx ;
struct t_serial_tx Com1_tx ;
uint8_t Com2TxBuffer[64] ;
uint8_t Com1TxBuffer[32] ;
uint8_t BtSbusFrame[28] ;
uint16_t BtLastSbusSendTime ;
uint8_t BtTempBuffer[100] ;
uint16_t BtRxTimer ;
uint8_t BtPreviousLinkIndex = 0xFF ;

// Data for BT and encoder over serial
struct t_bt_ser_pkt
{
	uint8_t bytes[3+32] ;
	uint8_t bt_count ;
} BtSerPkt ;

#define BT_ENABLE_HIGH	{ HC05_ENABLE_HIGH; BtSerPkt.bytes[0] |= 1 ; }
#define BT_ENABLE_LOW		{ HC05_ENABLE_LOW; BtSerPkt.bytes[0] &= ~1 ; }


#if defined(PCBX9D) && defined(REVNORM)
uint32_t txPdcBt( struct t_serial_tx *data )
{
	uint32_t count ;
	uint8_t *p ;
	uint8_t *s ;

	p = &BtSerPkt.bytes[3] ;
	s = data->buffer ;
	count = data->size ;
	while ( count-- )
	{
		*p++ = *s++ ;
	}
	data->ready = 0 ;
	BtSerPkt.bt_count = data->size ;
	return 1 ;
}

void btEncTx()
{
	BtSerPkt.bytes[1] = ~BtSerPkt.bytes[0] ;
	BtSerPkt.bytes[2] = 0 ;
						
	Com2_tx.buffer = 	BtSerPkt.bytes ;
	Com2_tx.size = BtSerPkt.bt_count + 3 ;
	txPdcCom2( &Com2_tx ) ;
}





#endif


void bt_send_buffer( void ) ;


extern uint8_t Activated ;



void checkFunction()
{
	if ( BtEepromFunction != g_model.BTfunction )
	{
		BtCurrentFunction = BtEepromFunction = g_model.BTfunction ;
	}
}

void scriptRequestBt()
{
	BtCurrentFunction = BT_SCRIPT ;
}

void scriptReleasetBt()
{
	BtCurrentFunction = BtEepromFunction ;
}

static void btPowerOn()
{
#ifdef PCBX9D
	if ( g_model.com2Function == COM2_FUNC_BT_ENC )
	{
		BtSerPkt.bytes[0] |= 2 ;
		return ;
	}
#endif

#ifdef PCBSKY
	configure_pins( PIO_PB3, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTB | PIN_NO_PULLUP ) ;
#endif	

#ifdef PCB9XT	
	if ( g_eeGeneral.btComPort == 1 )
	{
		configure_pins( GPIO_Pin_0, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA | PIN_PER_8 ) ;
	}
	else if ( g_eeGeneral.btComPort == 2 )
	{
		configure_pins( GPIO_Pin_10, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB | PIN_PER_7 ) ;
	}
#endif	

#ifdef PCBX7
	configure_pins( GPIO_Pin_10, PIN_PERIPHERAL | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB | PIN_PER_7 ) ;
#endif

}

static void btPowerOff()
{
#ifdef PCBX9D
	if ( g_model.com2Function == COM2_FUNC_BT_ENC )
	{
		BtSerPkt.bytes[0] &= ~2 ;
		return ;
	}
#endif

#ifdef PCBSKY
	configure_pins( PIO_PB3, PIN_ENABLE | PIN_LOW | PIN_OUTPUT | PIN_PORTB| PIN_NO_PULLUP ) ;
#endif	

#ifdef PCB9XT	
	if ( g_eeGeneral.btComPort == 1 )
	{
		configure_pins( GPIO_Pin_0, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTA ) ;
		GPIOA->BSRRH = GPIO_Pin_0 ;		// Set low
	}
	else if ( g_eeGeneral.btComPort == 2 )
	{
		configure_pins( GPIO_Pin_10, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB ) ;
		GPIOB->BSRRH = GPIO_Pin_10 ;		// Set low
	}
#endif	

#ifdef PCBX7
	configure_pins( GPIO_Pin_10, PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 | PIN_PORTB ) ;
	GPIOB->BSRRH = GPIO_Pin_10 ;		// Set low
#endif
}

void initBluetooth()
{
#ifdef PCBSKY
// Configure the ERASE pin as an output, low for Bluetooth use
	Pio *pioptr ;
	pioptr = PIOB ;
	pioptr->PIO_CODR = PIO_PB12 ;		// Set bit B12 LOW
	pioptr->PIO_PER = PIO_PB12 ;		// Enable bit B12 (ERASE)
	pioptr->PIO_OER = PIO_PB12 ;		// Set bit B12 as output
#endif	

#ifdef PCB9XT
// Configure pin PA5 as an output, low for Bluetooth use
	configure_pins( GPIO_Pin_5, PIN_PORTA | PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
	GPIOA->BSRRH = GPIO_Pin_5 ;		// Set low
#endif

#ifdef PCBX7
// Configure pin PA5 as an output, low for Bluetooth use
	configure_pins( GPIO_Pin_12, PIN_PORTE| PIN_OUTPUT | PIN_PUSHPULL | PIN_OS25 ) ;
	GPIOE->BSRRH = GPIO_Pin_12 ;		// Set low
#endif



}



void setBtBaudrate( uint32_t index )
{
	uint32_t brate ;
	if ( index == 1 )
	{
		brate = 9600 ;
	}
	else if ( index == 2 )
	{
		brate = 19200 ;
	}
	else if ( index == 3 )
	{
		brate = 57600 ;
	}
	else if ( index == 4 )
	{
		brate = 38400 ;
	}
	else
	{
		brate = 115200 ;
	}

#ifdef PCB9XT	
	if ( g_eeGeneral.btComPort == 1 )
	{
		UART4SetBaudrate ( brate ) ;
	}
	else if ( g_eeGeneral.btComPort == 2 )
	{
		Com3SetBaudrate ( brate ) ;
	}
#else
#ifdef PCBX7
	Com3SetBaudrate ( brate ) ;
#else
#if defined(PCBX9D) && defined(REVNORM)
	if ( index > 4 )
	{
		index = 0 ;
	}
	BtSerPkt.bytes[0] = ( BtSerPkt.bytes[0] & 0xE3 ) | index ;
	(void) brate ;
#else
	UART3_Configure( brate, Master_frequency ) ;		// Testing
#endif
#endif
#endif
}

uint32_t btAddressValid( uint8_t *address )
{
	uint8_t x ;
	x = *address++ ;
	x |= *address++ ;
	x |= *address++ ;
	x |= *address++ ;
	x |= *address++ ;
	x |= *address ;
	return x ;
}

uint32_t btAddressMatch( uint8_t *address1, uint8_t *address2 )
{
	uint32_t x ;
	for ( x = 0 ; x < 6 ; x += 1 )
	{
		if ( *address1++ != *address2++ )
		{
			return 0 ;
		}
	}
	return 1 ;
}

int32_t btSend( uint32_t length, uint8_t *data )
{
	uint8_t *end ;

	if ( length == 0 )
	{
		return Bt_tx.size ? 0 : 1 ;
	}
	if ( Bt_tx.size )
	{
		return 0 ;
	}
	if ( length > 64 )
	{
		return 0 ;
	}
	end = BtTxBuffer ;
	do
	{
		*end++ = *data++ ;
	} while ( --length ) ;
	Bt_tx.size = end - BtTxBuffer ;
	return 1 ;
}

void bt_send_buffer()
{
	Bt_tx.buffer = BtTxBuffer ;
	txPdcBt( &Bt_tx ) ;
	while ( Bt_tx.ready == 1 )
	{
		// Wait
		CoTickDelay(1) ;					// 2mS for now
	}
	Bt_tx.size = 0 ;
}

void flushBtFifo()
{
	uint16_t rxchar ;

	while ( ( rxchar = rxBtuart() ) != 0xFFFF )
	{
		// null body, flush fifo
	}
}

void btParse( uint8_t *dest, uint8_t *src, uint32_t length )
{
	// pick out data between : and \r
	uint8_t x ;
	uint32_t copy = 0 ;
	uint8_t bitfieldtype = BtControl.BtModuleType ;
	while ( (x = *src++) )
	{
		if ( x == ( ( bitfieldtype & BT_BITTYPE_CC41 ) ? '=' : ':' ) )
		{
			copy = 1 ;
		}
		else
		{
			if ( copy )
			{
				if ( x == '\r' )
				{
					break ;
				}
				*dest++ = x ;
				if ( --length == 0 )
				{
					break ;
				}
			}
		}
	}
	*dest = '\0' ;
	CoTickDelay(10) ;					// 20mS
}

uint32_t getBtOK( uint32_t errorAllowed, uint32_t timeout )
{
	uint16_t x ;
	uint32_t y ;
	uint16_t rxchar ;
	uint16_t a = 0 ;
	uint16_t b = 0 ;
	uint8_t bitfieldtype = BtControl.BtModuleType ;

	x = 'O' ;
	if ( bitfieldtype & (BT_BITTYPE_HC05 | BT_BITTYPE_CC41) )
	{
		x = 13 ;	// <CR>
	}
	for( y = 0 ; y < timeout ;  y += 1 )
	{
		if ( ( rxchar = rxBtuart() ) != 0xFFFF )
		{
			if ( rxchar == x )
			{
				if ( ( x == 'O' ) || ( x == 13 ) )
				{
					x = 'K' ;
					if ( bitfieldtype & (BT_BITTYPE_HC05 | BT_BITTYPE_CC41) )
					{
						x = 10 ;	// <LF>
					}
				}
				else
				{
					break ;			// Found "OK"
				}
			}
			if ( rxchar != 13 )
			{
				a = b ;
				b = rxchar ;
			}
		}
		else
		{
			CoTickDelay(1) ;					// 2mS
		}				 
	}
	if ( y < timeout )
	{
		if ( bitfieldtype & (BT_BITTYPE_HC05 | BT_BITTYPE_CC41) )
		{
			if ( errorAllowed == 0 )
			{
				if ( ( a != 'O') || ( b != 'K' ) )
				{
					return 0 ;
				}
			}
		}
		return 1 ;
	}
	else
	{
		return 0 ;
	}
}

uint32_t btTransaction( uint8_t *command, uint8_t *receive, uint32_t length )
{
	uint8_t x ;
	uint32_t y ;
	uint16_t rxchar ;
	uint8_t *end ;
	uint8_t a ;
	uint8_t b ;
	uint8_t bitfieldtype = BtControl.BtModuleType ;

	CoTickDelay(5) ;	// 10mS
	flushBtFifo() ;

	a = 'O' ;
	b = 'K' ;
	if ( bitfieldtype & BT_BITTYPE_CC41 )
	{
		a = '\r' ;
		b = '\n' ;
	}
	if ( command )
	{
		end = cpystr( &BtTxBuffer[0], command ) ;
		Bt_tx.size = end - BtTxBuffer ;

//uputs( (char *)command ) ;

		bt_send_buffer() ;

	}
	if ( receive == 0 )
	{
		return 1 ;
	}
	x = 0 ;
	for( y = 0 ; y < BT_POLL_TIMEOUT ;  y += 1 )
	{
		if ( ( rxchar = rxBtuart() ) != 0xFFFF )
		{
			if ( length )
			{
				*receive++ = rxchar ;
				length -= 1 ;
			}
			if ( rxchar == b )
			{
				if ( x == a )
				{
					*receive = '\0' ;
					break ;			// Found "OK" or (CRLF)
				}
			}
			x = rxchar ;
		}
		else
		{
			CoTickDelay(1) ;					// 2mS
		}				 
	}

	if ( y < BT_POLL_TIMEOUT )
	{
		if ( bitfieldtype & BT_BITTYPE_HC05 )
		{
			// flush the CR-LF
			while ( y < BT_POLL_TIMEOUT )
			{
				if ( ( rxchar = rxBtuart() ) != 0xFFFF )
				{
					if ( rxchar == 10 )
					{
						if ( x == 13 )
						{
							break ;			// Found CRLF
						}
					}
					x = rxchar ;
				}
				else
				{
					CoTickDelay(1) ;					// 2mS
				}				 
			}
		}
		return 1 ;
	}
	if ( length )
	{
		*receive = '!' ;
	}					
	return 0 ;
}



uint32_t getBtRole()
{
	uint8_t buffer[20] ;
	uint8_t btRole[4] ;
	uint8_t bitfieldtype = BtControl.BtModuleType ;

	if ( bitfieldtype & (BT_BITTYPE_CC41) )
	{
		btTransaction( (uint8_t *)"AT+ROLE\r\n", buffer, 19 ) ;
	}
	else
	{
		btTransaction( (uint8_t *)"AT+ROLE?\r\n", buffer, 19 ) ;
	}
	btParse( btRole, buffer, 3 ) ;
	if ( btRole[0] == '1' )
	{
		BtControl.BtMasterSlave = 2 ;
	}
	else if ( btRole[0] == '0' )
	{
		BtControl.BtMasterSlave = 1 ;
	}
	else
	{
		BtControl.BtMasterSlave = 0 ;
	}
	CoTickDelay(1) ;					// 2mS
	return BtControl.BtMasterSlave ;
}

uint32_t setBtRole( uint32_t role )
{
	uint8_t bitfieldtype = BtControl.BtModuleType ;

	if ( bitfieldtype & (BT_BITTYPE_CC41) )
	{
		cpystr( &BtTxBuffer[0], (uint8_t *)"AT+ROLE0\r\n" ) ;
		if ( role )
		{
			BtTxBuffer[7] = '1' ;
		}
		Bt_tx.size = 10 ;
	}
	else
	{
		cpystr( &BtTxBuffer[0], (uint8_t *)"AT+ROLE=0\r\n" ) ;
		if ( role )
		{
			BtTxBuffer[8] = '1' ;
		}
		Bt_tx.size = 11 ;
	}
	bt_send_buffer() ;
	return getBtOK(0, BT_POLL_TIMEOUT ) ;
}

uint32_t poll_bt_device()
{
	uint8_t bitfieldtype = BtControl.BtModuleType ;
	
	BtTxBuffer[0] = 'A' ;
	BtTxBuffer[1] = 'T' ;
	Bt_tx.size = 2 ;
	if ( bitfieldtype & (BT_BITTYPE_HC05 | BT_BITTYPE_CC41) )
	{
		BtTxBuffer[2] = 13 ;
		BtTxBuffer[3] = 10 ;
		Bt_tx.size = 4 ;
	}
	CoTickDelay(10) ;					// 20mS
	flushBtFifo() ;
	bt_send_buffer() ;
	return getBtOK(1, BT_POLL_TIMEOUT ) ;
}

uint32_t changeBtBaudrate( uint32_t baudIndex )
{
	uint16_t x ;
	uint8_t *p ;
	uint8_t bitfieldtype = BtControl.BtModuleType ;

	BtControl.BtBaudChangeIndex = baudIndex ;

	if ( bitfieldtype & BT_BITTYPE_CC41 )
	{
		x = 0 ;		// 9600
		if ( baudIndex == 0 )
		{
			x = 4 ;		// 115200
		}
		else if ( baudIndex == 2 )
		{
			x = 1 ;		// 19200		
		}
		else if ( baudIndex == 3 )
		{
			x = 3 ;		// 57600
		}
		else if ( baudIndex == 4 )
		{
			x = 2 ;		// 38400
		}
	}
	else
	{
		x = 4 ;		// 9600
		if ( baudIndex == 0 )
		{
			x = 8 ;		// 115200
		}
		else if ( baudIndex == 2 )
		{
			x = 5 ;		// 19200		
		}
		else if ( baudIndex == 3 )
		{
			x = 7 ;		// 57600
		}
		else if ( baudIndex == 4 )
		{
			x = 6 ;		// 38400
		}
	}
	p = cpystr( &BtTxBuffer[0], (uint8_t *)"AT+BAUD" ) ;

	*p++ = '0' + x ;
	if ( bitfieldtype & BT_BITTYPE_CC41 )
	{
		p = cpystr( p, (uint8_t *)"\r\n" ) ;
	}
	if ( bitfieldtype & BT_BITTYPE_HC05 )
	{
		p = cpystr( &BtTxBuffer[0], (uint8_t *)"AT+UART=" ) ;
		switch ( x )
		{
			case 4 :
				p = cpystr( p, (uint8_t *)"9600" ) ;
			break ;
			case 5 :
				p = cpystr( p, (uint8_t *)"19200" ) ;
			break ;
			case 6 :
				p = cpystr( p, (uint8_t *)"38400" ) ;
			break ;
			case 7 :
				p = cpystr( p, (uint8_t *)"57600" ) ;
			break ;
			case 8 :
				p = cpystr( p, (uint8_t *)"115200" ) ;
			break ;
		}
		p = cpystr( p, (uint8_t *)",0,0\r\n" ) ;
	}
	Bt_tx.size = p - BtTxBuffer ;
	bt_send_buffer() ;
	return getBtOK(0, BT_POLL_TIMEOUT ) ;
}

//AT+ADCN?
//+ADCN:<Param>
//OK
//Param: Authenticated Device
//Count
//Example:
//at+adcn?
//+ADCN:0 ----There is no authenticated device in the pair list.
//OK

void removeAddressList()
{
	btTransaction( (uint8_t *)"AT+RMAAD\r\n", 0, 0 ) ;
	CoTickDelay(10) ;					// 20mS
	getBtOK(0, 2000 ) ;
	CoTickDelay(10) ;					// 20mS
}


void checkAddressList()
{
	uint8_t buffer[12] ;
	uint8_t count[6] ;
	btTransaction( (uint8_t *)"AT+ADCN?\r\n", buffer, 11 ) ;
	btParse( count, buffer, 5 ) ;
	if ( count[0] != '0' )
	{
		removeAddressList() ;
	}
}


void btAddrHex2Bin()
{
	uint8_t chr ;
	uint32_t x ;
	uint32_t y ;
	uint32_t z ;
	uint32_t value ;

	x = 0 ;
	y = 0 ;
	z = 0 ;
	value = 0 ;

	for ( ;x<14; )
	{
		chr = BtRemote[0].address[x++] ;
		chr = toupper( chr ) ;
		if ( ( ( chr >= '0' ) && ( chr <= '9' ) ) || ( ( chr >= 'A' ) && ( chr <= 'F' ) ) )
		{
			chr -= '0' ;
			if ( chr > 9 )
			{
				chr -= 7 ;				
			}
			value <<= 4 ;
			value |= chr ;
			z += 1 ;
		}
		else
		{
			if ( z > 0 )
			{
				z = 0x80 ;
			}
		}
		if ( ( y == 0 ) && ( z > 3 ) )
		{
			BtBinAddr[0] = value >> 8 ;
			BtBinAddr[1] = value ;
			y = 1 ;
			value = 0 ;
			z = 0 ;
		}
		else if ( ( y == 1 ) && ( z > 1 ) )
		{
			BtBinAddr[2] = value ;
			y = 2 ;
			value = 0 ;
			z = 0 ;
		}
		else if ( z > 5 )
		{
			BtBinAddr[3] = value >> 16 ;
			BtBinAddr[4] = value >> 8 ;
			BtBinAddr[5] = value ;
			y = 3 ;
			break ;
		}
	}

	if ( y > 2 )
	{
		for ( y = 0 ; y < 4 ; y += 1 )
		{
			if ( btAddressValid( g_eeGeneral.btDevice[y].address ) )
			{
				if ( btAddressMatch( g_eeGeneral.btDevice[y].address, BtBinAddr ) )
				{
					break ;
				}
			}
			else
			{
				g_eeGeneral.btDevice[y].address[0] = BtBinAddr[0] ;
				g_eeGeneral.btDevice[y].address[1] = BtBinAddr[1] ;
				g_eeGeneral.btDevice[y].address[2] = BtBinAddr[2] ;
				g_eeGeneral.btDevice[y].address[3] = BtBinAddr[3] ;
				g_eeGeneral.btDevice[y].address[4] = BtBinAddr[4] ;
				g_eeGeneral.btDevice[y].address[5] = BtBinAddr[5] ;
				STORE_GENERALVARS ;
				break ;
			}
		}
	}
}

uint8_t b2hex( uint8_t c )
{
	c &= 0x0F ;
	if ( c > 9 )
	{
		c += 7 ;
	}
	c += '0' ;
	return c ;
}

uint8_t *btAddrBin2Hex( uint8_t *dest, uint8_t *source )
{
	uint8_t c ;
	
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	*dest++ = ',' ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	*dest++ = ',' ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	*dest = '\0' ;
	return dest ;
}

uint8_t *copyBtAddress( uint8_t *end, uint32_t index )
{
	uint32_t x ;
	uint32_t y ;
	uint8_t chr ;
	
	x = 0 ;
	y = 0 ;

	while ( ( chr = BtRemote[index].address[x] ) != ':' )
	{
		x += 1 ;
		y += 1 ;
		*end++ = chr ;
		if ( y > 4 )
		{
			break ;
		}
	}
	*end++ = ',' ;
	x += 1 ;	// Skip ':'
	y = 0 ;
	while ( ( chr = BtRemote[index].address[x] ) != ':' )
	{
		x += 1 ;
		y += 1 ;
		*end++ = chr ;
		if ( y > 4 )
		{
			break ;
		}
	}
	*end++ = ',' ;
	x += 1 ;	// Skip ':'
	y = 0 ;
	while ( ( chr = BtRemote[index].address[x] ) )
	{
		x += 1 ;
		y += 1 ;
		*end++ = chr ;
		if ( y > 8 )
		{
			break ;
		}
	}
	return end ;
}


uint32_t setBtName( uint8_t *name )	// Max 14 chars
{
	uint8_t *end ;
	uint8_t bitfieldtype = BtControl.BtModuleType ;

	end = cpystr( &BtTxBuffer[0], (uint8_t *)"AT+NAME" ) ;
	if ( bitfieldtype & (BT_BITTYPE_HC05) )
	{
		*end++ = '=' ;
	}
	end = cpystr( end, name ) ;
	if ( bitfieldtype & (BT_BITTYPE_HC05 | BT_BITTYPE_CC41) )
	{
		end = cpystr( end, (uint8_t *)"\r\n" ) ;
	}
	Bt_tx.size = end - BtTxBuffer ;
	bt_send_buffer() ;
	return getBtOK(0, BT_POLL_TIMEOUT ) ;
}


void getBtValues()
{
	uint8_t buffer[20] ;
	uint8_t bitfieldtype = BtControl.BtModuleType ;

	getBtRole() ;	
	CoTickDelay(10) ;					// 20mS
	if ( bitfieldtype & (BT_BITTYPE_HC05) )
	{
		checkAddressList() ;
		btTransaction( (uint8_t *)"AT+NAME?\r\n", buffer, 19 ) ;
		btParse( BtName, buffer, 19 ) ;
		btTransaction( (uint8_t *)"AT+PSWD?\r\n", buffer, 19 ) ;
	}
	else
	{
		btTransaction( (uint8_t *)"AT+PASS\r\n", buffer, 19 ) ;	// CC41
	}
	btParse( BtPswd, buffer, 6 ) ;
}

void btConfigure()
{
	uint32_t j ;
	
	BT_ENABLE_HIGH ;						// Set bit B12 HIGH
	CoTickDelay(10) ;					// 20mS
	BtControl.BtConfigure = 0x40 ;
	btTransaction( (uint8_t *)"AT+CLASS=0\r\n", 0, 0 ) ;
	CoTickDelay(10) ;					// 40mS
	getBtOK(0, BT_POLL_TIMEOUT ) ;
	BtControl.BtConfigure = 0x41 ;
	btTransaction( (uint8_t *)"AT+CMODE=1\r\n", 0, 0 ) ;
	CoTickDelay(10) ;					// 40mS
	getBtOK(0, BT_POLL_TIMEOUT ) ;
	BtControl.BtConfigure = 0x42 ;
	removeAddressList() ;
	BtControl.BtConfigure = 0x44 ;
	btTransaction( (uint8_t *)"AT+INQM=0,5,4\r\n", 0, 0 ) ;
	CoTickDelay(10) ;					// 40mS
	for ( j = 0 ; j < 12 ; j += 1 )
	{
		getBtOK(0, 225 ) ;
		BtControl.BtConfigure = 0x49 + j ;
	}
	BtControl.BtConfigure = 0 ;
	CoTickDelay(10) ;					// 20mS
	BT_ENABLE_HIGH ;						// Set bit B12 HIGH
	CoTickDelay(10) ;					// 20mS
}

uint32_t btLink( uint32_t index )
{
	uint32_t x ;
	uint32_t i ;
	uint8_t *end ;

	if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
	{

		btTransaction( (uint8_t *)"AT\r\n", 0, 0 ) ;
		CoTickDelay(10) ;					// 20mS
		getBtOK(0, BT_POLL_TIMEOUT ) ;
		CoTickDelay(10) ;					// 20mS
	
//		if ( BtPreviousLinkIndex != index )
//		{
//			end = cpystr( BtRname, (uint8_t *)"AT+BIND=" ) ;
//			end = btAddrBin2Hex( end, g_eeGeneral.btDevice[index].address ) ;
//			*end++ = '\r' ;
//			*end++ = '\n' ;
//			*end = '\0' ;
//			btTransaction( BtRname, 0, 0 ) ;
//			CoTickDelay(10) ;					// 40mS
//			i = getBtOK(0, 2000 ) ;
//		}
	 
		x = 0 ;
		end = cpystr( BtRname, (uint8_t *)"AT+LINK=" ) ;
		end = btAddrBin2Hex( end, g_eeGeneral.btDevice[index].address ) ;
		*end++ = '\r' ;
		*end++ = '\n' ;
		*end = '\0' ;
		btTransaction( BtRname, 0, 0 ) ;
		CoTickDelay(10) ;					// 40mS
		i = getBtOK(0, BT_POLL_TIMEOUT ) ;

		if ( i == 0 )
		{
			for ( x = 0 ; x < 10 ; x += 1 )
			{
				CoTickDelay(10) ;					// 40mS
				i = getBtOK(0, BT_POLL_TIMEOUT ) ;
				if ( i )
				{
					break ;
				}
			}
		}
		return i ;
	}
	else if ( g_eeGeneral.BtType == BT_TYPE_CC41 )
	{
		BtControl.BtLinking += 1 ;
		cpystr( BtRname, (uint8_t *)"AT+CONN1\r\n" ) ;
		btTransaction( BtRname, 0, 0 ) ;
		CoTickDelay(10) ;					// 40mS
		i = getBtOK(1, BT_POLL_TIMEOUT ) ;
		if ( i == 0 )
		{
			for ( x = 0 ; x < 10 ; x += 1 )
			{
				CoTickDelay(10) ;					// 40mS
				i = getBtOK(1, BT_POLL_TIMEOUT ) ;
				if ( i )
				{
					break ;
				}
			}
		}
		return i ;
	}
	return 0 ;
}

#if defined(PCBSKY) || defined(PCB9XT) || defined(PCBX7) || defined(PCBX9D)
void processBtRx( int32_t data, uint32_t rxTimeout )
{
	uint16_t rxchar ;

	if ( BtCurrentFunction == BT_TRAIN_TXRX )
	{
		if ( data == 2 )
		{
			BtControl.BtRxState = BT_RX_RECEIVE ;
			BtControl.BtSbusIndex = 0 ;
			BtControl.BtRxChecksum = 0 ;
		}
		else
		{
			if ( BtControl.BtRxState == BT_RX_STUFF )
			{
				data = data ^ 0x80 ;
				BtControl.BtRxState = BT_RX_RECEIVE ;
			}
			else
			{
				if ( data == 3 )
				{
					BtControl.BtRxState = BT_RX_STUFF ;
				}
			}
			if ( BtControl.BtRxState == BT_RX_RECEIVE )
			{
				BtSbusFrame[BtControl.BtSbusIndex++] = data ;
				BtControl.BtRxChecksum += data ;
				if ( BtControl.BtSbusIndex > 25 )
				{
					BtControl.BtRxOccured = 1 ;
					if ( BtControl.BtRxChecksum == 0 )
					{
						TrainerProfile *tProf = &g_eeGeneral.trainerProfile[g_model.trainerProfile] ;
						if ( processSBUSframe( BtSbusFrame, ( tProf->channel[0].source == TRAINER_BT ) ? g_ppmIns : 0, BtControl.BtSbusIndex ) )
						{
							BtControl.BtSbusReceived = 1 ;
						}
					}
					else
					{
						BtControl.BtBadChecksum = BtControl.BtRxChecksum ;
					}
					BtControl.BtRxState = BT_RX_IDLE ;
				}
			}
		}
		return ;
	}

	if ( rxTimeout )
	{
		TrainerProfile *tProf = &g_eeGeneral.trainerProfile[g_model.trainerProfile] ;
		if ( processSBUSframe( BtSbusFrame, ( tProf->channel[0].source == TRAINER_BT ) ? g_ppmIns : 0, BtControl.BtSbusIndex ) )
		{
			BtControl.BtSbusReceived = 1 ;
		}
		BtControl.BtSbusIndex = 0 ;	 
	}
	else
	{
		rxchar = data ;
		BtSbusFrame[BtControl.BtSbusIndex++] = rxchar ;
		if ( BtControl.BtSbusIndex > 27 )
		{
			BtControl.BtSbusIndex = 27 ;
		}
	}
}

#endif


void btConfigCheck()
{
	struct t_bt_control *pBtControl ;
	uint32_t x ;
	pBtControl = &BtControl ;
	
	if ( pBtControl->BtBaudrateChanged )
	{
		if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
		{
			BT_ENABLE_HIGH ;						// Set bit B12 HIGH
		}
		CoTickDelay(10) ;					// 20mS for now
		changeBtBaudrate( g_eeGeneral.bt_baudrate ) ;
		CoTickDelay(10) ;					// 20mS for now
		if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
		{
			BT_ENABLE_LOW ;							// Set bit B12 LOW
		}
		pBtControl->BtBaudrateChanged = 0 ;
	}
	
	if ( pBtControl->BtRoleChange & 0x80 )
	{
		BT_ENABLE_HIGH ;						// Set bit B12 HIGH
		uint8_t newRole = pBtControl->BtRoleChange & 0x01 ;
		pBtControl->BtRoleChange = 0x40 ;
		if ( newRole+1 != pBtControl->BtMasterSlave )
		{
			setBtRole( newRole ) ;
			getBtRole() ;
		}
		pBtControl->BtRoleChange = 0 ;
		BT_ENABLE_LOW ;							// Set bit B12 LOW
	}
	if ( pBtControl->BtNameChange & 0x80 )
	{
		uint8_t *pname = g_eeGeneral.btName ;
		if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
		{
			BT_ENABLE_HIGH ;						// Set bit B12 HIGH
			pname = BtName ;
		}
		pBtControl->BtNameChange = 0x40 ;
		setBtName( pname ) ;
		pBtControl->BtNameChange = 0 ;
		BT_ENABLE_LOW ;							// Set bit B12 LOW
	}
	if ( pBtControl->BtConfigure & 0x80 )
	{
		btConfigure() ;
	}
	if ( pBtControl->BtScan )
	{
		pBtControl->BtScan = 0 ;
		if ( g_eeGeneral.BtType >= BT_TYPE_HC05 )
		{
			uint32_t i ;
			uint32_t j ;
			uint16_t rxchar ;

			for ( i = 0 ; i < 100 ; i += 1 )
			{
				BtTempBuffer[0] = 0 ;
			}
			BT_ENABLE_HIGH ;						// Set bit B12 HIGH
			CoTickDelay(5) ;					// 10mS
			flushBtFifo() ;
			if ( pBtControl->BtScanInit == 0)
			{
				pBtControl->BtScanInit = 1 ;
				setBtRole( 1 ) ;
//						if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
//						{
					pBtControl->BtScanState = 1 ;
					CoTickDelay(20) ;					// 40mS
					btTransaction( (uint8_t *)"AT+INIT\r\n", 0, 0 ) ;
					CoTickDelay(10) ;					// 20mS
					i = getBtOK(1, BT_POLL_TIMEOUT ) ;
//						}
			}
			btTransaction( (uint8_t *)"AT+DISC\r\n", 0, 0 ) ;
			CoTickDelay(20) ;					// 40mS
			getBtOK(1, BT_POLL_TIMEOUT ) ;
			pBtControl->BtScanState = 2 ;
			CoTickDelay(100) ;					// 200mS
			flushBtFifo() ;
			btTransaction( (uint8_t *)"AT+INQ\r\n", 0, 0 ) ;
			pBtControl->BtScanState = 3 ;
			j = 0 ;
			x = 0 ;
			for ( i = 0 ; i < 3800 ; i += 1 )
			{
				while ( ( rxchar = rxBtuart() ) != 0xFFFF )
				{
					if ( rxchar == '+' )
					{
						x = 1 ;
					}
					if ( x && ( j < 99 ) )
					{
						BtTempBuffer[j++] = rxchar ;
					}
					if ( rxchar == '\r' )
					{
						x = 0 ;
					}
				}
				CoTickDelay(1) ;					// 2mS
			}
			BtTempBuffer[j] = 0 ;
			i = 0 ;
			x = 0 ;
			uint32_t y ;
			// Looking for: +INQ:2:72:D2224,3E0104,FFBC
			// Or +INQ:1 0x00158300E442
			pBtControl->NumberBtremotes = 0 ;
			if ( j >= 4 )
			{
				while ( i < j-3 )
				{
					if ( BtTempBuffer[i] == '+' )
					{
						if ( BtTempBuffer[i+1] == 'I' )
						{
							if ( BtTempBuffer[i+2] == 'N' )
							{
								if ( BtTempBuffer[i+3] == 'Q' )
								{
									y = 0 ;
									if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
									{											
										i += 5 ;		// Skip ':'
										while ( BtTempBuffer[i] != ',' )
										{
											if ( i >= j )
											{
												break ;
											}
											BtRemote[pBtControl->NumberBtremotes].address[y++] = BtTempBuffer[x++] = BtTempBuffer[i++] ;
											if ( y > 14 )
											{
												y = 14 ;
											}
										}
									}
									else if ( g_eeGeneral.BtType == BT_TYPE_CC41 )
									{
										if ( BtTempBuffer[i+4] == 'S' )
										{
											i += 5 ;
											continue ;
										}
										if ( BtTempBuffer[i+4] == 'E' )
										{
											break ;
										}
										i += 9 ;		// Skip ':'
										while ( BtTempBuffer[i] != 13 )
										{
											BtRemote[pBtControl->NumberBtremotes].address[y++] = BtTempBuffer[x++] = BtTempBuffer[i++] ;
											if ( y > 12 )
											{
												y = 12 ;
											}
											if ( i >= j )
											{
												break ;
											}
										}
									}
									BtTempBuffer[x++] = '\r' ;
									BtTempBuffer[x++] = '\n' ;
									if ( y > 2 )
									{
										BtRemote[pBtControl->NumberBtremotes].address[y] = '\0' ;
										pBtControl->NumberBtremotes += 1 ;
									}
								}
							}
						}
					}
					i += 1 ;
				}
			}
			BtTempBuffer[x] = '\0' ;
			pBtControl->BtScanState = 4 ;
			// Next we want the remote name
			if ( pBtControl->NumberBtremotes )
			{
				btAddrHex2Bin() ;
			}

			if ( pBtControl->NumberBtremotes )
			{
				if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
				{
					// Got a response from the +INQ
					// Send AT+RNAME?xxxx,xx,xxxxxx\r\n
					uint8_t *end ;
					x = 0 ;
					end = cpystr( BtRname, (uint8_t *)"AT+RNAME?" ) ;
					end = copyBtAddress( end, 0 ) ;

					*end++ = '\r' ;
					*end++ = '\n' ;
					*end = '\0' ;
							

					if ( btTransaction( BtRname, BtRemote[0].name, 14 ) )
					{
					}
					else
					{
						for ( i = 0 ; i < 7 ; i += 1 )
						{
							if ( btTransaction( 0, BtRemote[0].name, 14 ) )
							{
								break ;
							}
						}
					}
					btTransaction( (uint8_t *)"AT+STATE?\r\n", 0, 0 ) ;
					CoTickDelay(10) ;					// 40mS
					i = getBtOK(0, BT_POLL_TIMEOUT ) ;
					btTransaction( (uint8_t *)"AT+DISC\r\n", 0, 0 ) ;
					CoTickDelay(10) ;					// 40mS
					i = getBtOK(0, BT_POLL_TIMEOUT ) ;
						
					CoTickDelay(10) ;					// 40mS
				}
			}
			BT_ENABLE_LOW ;							// Set bit B12 LOW
		}
	}
}




/*
Commands to BT module
AT+VERSION 	Returns the software version of the module
AT+BAUDx 	Sets the baud rate of the module:
1 	1200
2 	2400
3 	4800
4 	9600 (Default)
5 	19200
6 	38400
7 	57600
8 	115200
9 	230400
AT+NAME<name here> 	Sets the name of the module

Any name can be specified up to 20 characters
AT+PINxxxx 	Sets the pairing password of the device

Any 4 digit number can be used, the default pincode is 1234
AT+PN 	Sets the parity of the module 

So we could send AT+VERSION at different baudrates until we get a response
Then we can change the baudrate to the required value.
Or maybe just AT and get OK back
*/


void bt_task(void* pdata)
{
	uint32_t x ;
	int32_t y ;
	uint16_t lastTimer = 0 ;
	uint32_t btBits = 0 ;
	struct t_bt_control *pBtControl ;
	pBtControl = &BtControl ;
	pBtControl->BtModuleType = 1 << g_eeGeneral.BtType ;

	while ( Activated == 0 )
	{
		CoTickDelay(10) ;					// 20mS
	}

	while ( (x = CoGetOSTime() ) < 400 )
	{
		CoTickDelay(10) ;					// 20mS
	}

#ifdef PCB9XT
	if ( g_eeGeneral.enableI2C == 2 )
	{
		if ( g_eeGeneral.btComPort == 2 )
		{
			stop_I2C2() ;
			com3Init( 115200 ) ;
		}
	}
#endif
#ifdef PCBX7
	com3Init( 115200 ) ;
#endif

	Bt_flag = CoCreateFlag(TRUE,0) ;
	Bt_tx.size = 0 ;

// Look for BT module baudrate, try 115200, and 9600
// Already initialised to g_eeGeneral.bt_baudrate
// 0 : 115200, 1 : 9600, 2 : 19200, 3 : 57600, 4 : 38400

	BtCurrentFunction = BtEepromFunction = g_model.BTfunction ;

	for(;;)
	{
		btPowerOff() ;
		while ( BtCurrentFunction == BT_OFF )
		{
			CoTickDelay(50) ;					// 100mS
			checkFunction() ;
		}
		btPowerOn() ;
		
		x = g_eeGeneral.bt_baudrate ;

		if ( g_eeGeneral.BtType >= BT_TYPE_HC05 )
		{
			BT_ENABLE_HIGH ;						// Set bit B12 HIGH
			CoTickDelay(5) ;					// 10mS
		}

		uint32_t found = 0 ;
		do
		{
			pBtControl->Bt_ok = poll_bt_device() ;		// Do we get a response?

			for ( y = 0 ; y <= NUM_BT_BAUDRATES ; y += 1 )
			{
				if ( pBtControl->Bt_ok == 0 )
				{
					x += 1 ;
					if ( x >= NUM_BT_BAUDRATES )
					{
						x = 0 ;
					}
					setBtBaudrate( x ) ;
					CoTickDelay(5) ;					// 10mS
					pBtControl->Bt_ok = poll_bt_device() ;		// Do we get a response?
				}
			}

			if ( pBtControl->Bt_ok )
			{
				pBtControl->Bt_ok = x + 1 ;		
				pBtControl->BtCurrentBaudrate = x ;
				if ( x != g_eeGeneral.bt_baudrate )
				{
					x = g_eeGeneral.bt_baudrate ;
					// Need to change Bt Baudrate
					pBtControl->Bt_ok = changeBtBaudrate( x ) ;
					CoTickDelay(10) ;	// 20mS, give time for the <crlf> to be sent
					// Continue with the current baudrate, HC-05 needs power cycle to change
				}
				else
				{
					found = 1 ;
				}
			}
			checkFunction() ;
			if ( BtCurrentFunction == BT_OFF )
			{
				break ;
			}
		} while (found == 0 ) ;
		
		if ( BtCurrentFunction == BT_OFF )
		{
			continue ;
		}
		CoTickDelay(1) ;					// 2mS
		if ( g_eeGeneral.BtType == BT_TYPE_CC41 )
		{
			CoTickDelay(1) ;					// 2mS
			getBtValues() ;
			CoTickDelay(1) ;					// 2mS
			pBtControl->BtNameChange = 0x80 ;
		}
		if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
		{
			CoTickDelay(1) ;					// 2mS
			getBtValues() ;
			CoTickDelay(1) ;					// 2mS
			btTransaction( (uint8_t *)"AT+INIT\r\n", 0, 0 ) ;
			CoTickDelay(10) ;					// 20mS
			getBtOK(1, BT_POLL_TIMEOUT ) ;
			CoTickDelay(10) ;					// 20mS
			btTransaction( (uint8_t *)"AT\r\n", 0, 0 ) ;
			CoTickDelay(10) ;					// 20mS
			getBtOK(0, BT_POLL_TIMEOUT ) ;
			CoTickDelay(10) ;					// 20mS
			BT_ENABLE_LOW ;							// Set bit B12 LOW
		}

		pBtControl->BtCurrentLinkIndex = g_model.btDefaultAddress ;

		pBtControl->BtReady = 1 ; 
		while(1)
		{
			checkFunction() ;
			if ( BtCurrentFunction == BT_OFF )
			{
				break ;
			}
			
			if ( Bt_tx.size == 0 )
			{
				btConfigCheck() ;
			}
			
			if ( ( g_eeGeneral.BtType == BT_TYPE_HC06 )
					 || ( ( g_eeGeneral.BtType >= BT_TYPE_HC05 ) && ( pBtControl->BtMasterSlave == 1 ) ) )
			{
				btBits |= BT_IS_SLAVE ;			
			}
			else
			{
				btBits &= ~BT_IS_SLAVE ;
			}

	#ifndef PCB9XT 
	#ifndef PCBX7
			if ( ( g_model.com2Function == COM2_FUNC_BTDIRECT )
					 || ( g_model.com2Function == COM2_FUNC_TEL_BT2WAY ) )
			{
				if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
				{
					BT_ENABLE_HIGH ;						// Set bit B12 HIGH
				}
				// Send data to COM2
				if ( Bt_tx.ready == 0 )	// Buffer available
				{
					Bt_tx.size = 0 ;
					while ( ( y = get_fifo128( &Bt_fifo ) ) != -1 )
					{
						BtTxBuffer[Bt_tx.size++] = y ;
						if ( Bt_tx.size > 63 )
						{
							break ;
						}	
					}
					if ( Bt_tx.size )
					{
						Bt_tx.buffer = BtTxBuffer ;
						txPdcBt( &Bt_tx ) ;
					}
				}

				if ( Com2_tx.ready == 0 )	// Buffer available
				{
					Com2_tx.size = 0 ;
					while( ( x = rxBtuart() ) != (uint32_t)-1 )
					{
						Com2TxBuffer[Com2_tx.size++] = x ;
						if ( Com2_tx.size > 63 )
						{
							break ;
						}	
					}
					if ( Com2_tx.size )
					{
						Com2_tx.buffer = Com2TxBuffer ;
						txPdcCom2( &Com2_tx ) ;
					}
				}
				CoTickDelay(1) ;					// 2mS for now
			}
			else
	#endif	// nPCBX7
	#endif	// nPCB9XT
			if ( ( BtCurrentFunction == BT_LCDDUMP ) && ( pBtControl->BtMasterSlave == 1 ) )	// LcdDump and SLAVE
			{
				while ( ( y = rxBtuart() ) != -1 )
				{
	extern uint8_t ExternalKeys ;
	extern uint8_t ExternalSet ;
					ExternalKeys = y ;
					ExternalSet = 50 ;
				}
				CoTickDelay(5) ;					// 10mS for now
			}
			else if ( BtCurrentFunction == BT_SCRIPT )
			{
				if ( Bt_tx.size )
				{
					bt_send_buffer() ;
				}
				BtRxTimer = 100 ;		// keep running
				CoTickDelay(2) ;		// 4mS, request will only be avery 10mS
			}
			else
			{
				x = CoWaitForSingleFlag( Bt_flag, 1 ) ;		// Wait for data in Fifo
				if ( BtCurrentFunction == BT_OFF )
				{
					continue ;
				}
				if ( x == E_OK )
				{
					// We have some data in the Fifo
					while ( ( y = get_fifo128( &Bt_fifo ) ) != -1 )
					{
						BtTxBuffer[Bt_tx.size++] = y ;
						if ( Bt_tx.size > 63 )
						{
							bt_send_buffer() ;
						}
					}
				}
				else if ( Bt_tx.size )
				{
					bt_send_buffer() ;
				}
				else if ( pBtControl->BtLinkRequest )
				{
					if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
					{
						BT_ENABLE_HIGH ;						// Set bit B12 HIGH
						CoTickDelay(10) ;					// 40mS
						btLink( pBtControl->BtLinkRequest & 3 ) ;
						pBtControl->BtCurrentLinkIndex = pBtControl->BtLinkRequest & 3 ;
						pBtControl->BtLinked = 1 ;
						BtRxTimer = 1000 ;
						pBtControl->BtRxOccured = 0 ;
						CoTickDelay(10) ;					// 40mS
						BT_ENABLE_LOW ;							// Set bit B12 LOW
						pBtControl->BtLinkRequest = 0 ;
					}
					else if ( g_eeGeneral.BtType == BT_TYPE_CC41 )
					{
						btLink( 1 ) ;
						pBtControl->BtCurrentLinkIndex = 1 ;
						pBtControl->BtLinked = 1 ;
						BtRxTimer = 1000 ;
						pBtControl->BtRxOccured = 0 ;
						pBtControl->BtLinkRequest = 0 ;
					}
				}

				if ( ( pBtControl->BtMasterSlave == 2 ) && ( g_model.autoBtConnect ) && 
						btAddressValid( g_eeGeneral.btDevice[pBtControl->BtCurrentLinkIndex].address ) )
				{
					pBtControl->BtLinked = 1 ;
				}

				if ( ( ( pBtControl->BtMasterSlave == 2 ) && ( BtCurrentFunction == BT_TRAIN_TXRX ) && pBtControl->BtLinked )
						 || ( btBits & BT_SLAVE_SEND_SBUS ) )
				{
					uint32_t i ;
					if ( btBits & BT_SLAVE_SEND_SBUS )
					{
						btBits &= ~BT_SLAVE_SEND_SBUS ;
					}
					else
					{
						uint16_t x ;
						do
						{
							CoTickDelay(1) ;					// 2mS
							x = getTmr2MHz() - BtLastSbusSendTime ;
						} while ( x < 28000 ) ;
						BtLastSbusSendTime += 28000 ;
	//					} while ( x < 40000 ) ;
	//					BtLastSbusSendTime += 40000 ;
					}
					// Send Sbus Frame
  				uint8_t *p = BtTxBuffer ;
					uint32_t outputbitsavailable = 0 ;
					uint32_t outputbits = 0 ;
					uint8_t checksum = 0x0F ;

					*p++ = 2 ;		// Marker 
					*p++ = 0x0F ;
					for ( i = 0 ; i < 16 ; i += 1 )
				 	{
						int16_t x = g_chans512[i] ;
						x *= 4 ;
						x += x > 0 ? 4 : -4 ;
						x /= 5 ;
						x += 0x3E0 ;
						if ( x < 0 )
						{
							x = 0 ;
						}
						if ( x > 2047 )
						{
							x = 2047 ;
						}
						outputbits |= x << outputbitsavailable ;
						outputbitsavailable += 11 ;
						while ( outputbitsavailable >= 8 )
						{
							uint8_t j = outputbits ;
							checksum += j ;
							if ( ( j == 2 ) || ( j == 3 ) )
							{
								j ^= 0x80 ;
								*p++ = 3 ;		// "stuff"
							}
  	          *p++ = j ;
							outputbits >>= 8 ;
							outputbitsavailable -= 8 ;
						}
					}
					*p++ = 0 ;
					*p++ = 0 ;
					checksum = -checksum ;
					if ( ( checksum == 2 ) || ( checksum == 3 ) )
					{
						*p++ = 3 ;		// "stuff"
						checksum ^= 0x80 ;
					}
					*p++ = checksum ;
					Bt_tx.size = p - BtTxBuffer ;
					bt_send_buffer() ;
				}

				if ( BtCurrentFunction == BT_TRAIN_TXRX )
				{
					while( ( x = rxBtuart() ) != (uint32_t)-1 )
					{
						if ( BtRxTimer < 100 )
						{
							BtRxTimer = 100 ;
						}
						processBtRx( x, 0 ) ;
						if ( pBtControl->BtSbusReceived )
						{
							pBtControl->BtSbusReceived = 0 ;
							if ( btBits & BT_IS_SLAVE )
							{
								btBits |= BT_SLAVE_SEND_SBUS ;						
							}
						}
						lastTimer = getTmr2MHz() ;
						btBits &= ~BT_RX_TIMEOUT ;
					}
				}
				else
				{
	#ifdef REVX
					if ( g_model.bt_telemetry > 1 )
					{
						if ( g_model.frskyComPort )				
						{
							if ( Com2_tx.ready == 0 )	// Buffer available
							{
								Com2_tx.size = 0 ;
								while( ( x = rxBtuart() ) != (uint32_t)-1 )
								{
									Com2TxBuffer[Com2_tx.size++] = x ;
									if ( Com2_tx.size > 63 )
									{
										break ;
									}	
								}
								if ( Com2_tx.size )
								{
									Com2_tx.buffer = Com2TxBuffer ;
									txPdcCom2( &Com2_tx ) ;
								}
							}
						}
						else
						{
							if ( Com1_tx.ready == 0 )	// Buffer available
							{
								Com1_tx.size = 0 ;
								while( ( x = rxBtuart() ) != (uint32_t)-1 )
								{
									Com1TxBuffer[Com1_tx.size++] = x ;
									if ( Com1_tx.size > 31 )
									{
										break ;
									}	
								}
								if ( Com1_tx.size )
								{
									Com1_tx.buffer = Com1TxBuffer ;
									txPdcCom1( &Com1_tx ) ;
								}
							}
						}
					}
					else
	#endif
					{				 
						while( ( x = rxBtuart() ) != (uint32_t)-1 )
						{
							if ( BtRxTimer < 100 )
							{
								BtRxTimer = 100 ;
							}
							processBtRx( x, 0 ) ;
							lastTimer = getTmr2MHz() ;
							btBits &= ~BT_RX_TIMEOUT ;
						}
						if ( (btBits & BT_RX_TIMEOUT) == 0 )
						{
							if ( ( ( getTmr2MHz() - lastTimer ) & 0x0000FFFF ) > 10000 )
							{
								btBits |= BT_RX_TIMEOUT ;
								processBtRx( 0, 1 ) ;
							}
						}
					}
				}
			}

			if ( BtRxTimer == 0 )
			{
				if ( pBtControl->BtLinked )
				{
					if ( pBtControl->BtRxOccured )
					{
						putSystemVoice( SV_BT_LOST, 0 ) ;
					}
					if ( g_model.autoBtConnect )
					{
						pBtControl->BtCurrentLinkIndex =  g_model.btDefaultAddress ;
					}

					if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
					{
						BT_ENABLE_HIGH ;						// Set bit B12 HIGH
						CoTickDelay(10) ;					// 40mS
					}
					btLink(pBtControl->BtCurrentLinkIndex) ;
					BtRxTimer = 1000 ;
					pBtControl->BtRxOccured = 0 ;
					if ( g_eeGeneral.BtType == BT_TYPE_HC05 )
					{
						CoTickDelay(10) ;					// 40mS
						BT_ENABLE_LOW ;							// Set bit B12 LOW
					}
				}
			}
		}
	}
}



