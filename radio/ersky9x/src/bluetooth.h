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

#ifndef bluetooth_h
#define bluetooth_h

#if defined(PCBX9D) && defined(REVNORM)
#define BT_WITH_ENCODER		1
#endif

#define BT_115200		0
#define BT_9600			1
#define BT_19200		2
#define BT_57600		3
#define BT_38400		4

#define NUM_BT_BAUDRATES 5

#ifdef PCBSKY
#define HC05_ENABLE_HIGH		(PIOB->PIO_SODR = PIO_PB12)			// Set bit B12 HIGH
#define HC05_ENABLE_LOW			(PIOB->PIO_CODR = PIO_PB12)			// Set bit B12 LOW
#endif // PCBSKY

#ifdef PCB9XT
#define HC05_ENABLE_HIGH		(GPIOA->BSRRL = GPIO_Pin_5)			// Set bit PA5 HIGH
#define HC05_ENABLE_LOW			(GPIOA->BSRRH = GPIO_Pin_5)			// Set bit PA5 LOW
#endif // PCB9XT

#ifdef PCBX7
#define HC05_ENABLE_HIGH		(GPIOE->BSRRL = GPIO_Pin_12)			// Set bit PE12 HIGH
#define HC05_ENABLE_LOW			(GPIOE->BSRRH = GPIO_Pin_12)			// Set bit PE12 LOW
#endif // PCB9XT

#if defined(PCBX9LITE)
#define HC05_ENABLE_HIGH					// Nothing
#define HC05_ENABLE_LOW						// Nothing
#endif

#ifdef BT_WITH_ENCODER
#define HC05_ENABLE_HIGH					// Nothing
#define HC05_ENABLE_LOW						// Nothing
#endif

#define BT_ROLE_SLAVE		0
#define BT_ROLE_MASTER	1

#define BT_POLL_TIMEOUT		800

#define BT_RX_IDLE		0
#define BT_RX_RECEIVE	1
#define BT_RX_STUFF		2

// Bits in btBits
#define BT_RX_TIMEOUT				1
#define BT_SLAVE_SEND_SBUS	2
#define BT_IS_SLAVE					4
#define BT_RX_DATA					8

struct t_bt_control
{
	uint8_t BtCurrentBaudrate ;
	uint8_t BtLinkRequest ;
	uint8_t BtScan ;
	uint8_t BtScanState ;
	uint8_t BtBaudrateChanged ;
	uint8_t BtConfigure ;
	uint8_t BtScanInit ;
	uint8_t BtCurrentLinkIndex ;
	uint8_t BtRoleChange ;
	uint8_t BtNameChange ;
	uint8_t BtStateRequest ;
	uint8_t BtMasterSlave ;
	uint8_t BtReady ;
//	uint8_t BtLinking ;
	uint8_t BtRxState ;
	uint8_t BtSbusIndex ;
	uint8_t BtSbusReceived ;
	uint8_t BtRxOccured ;
	uint8_t BtRxChecksum ;
	uint8_t BtBadChecksum ;
	uint8_t BtBaudChangeIndex ;
	uint8_t BtLinked ;
	uint8_t NumberBtremotes ;
	uint8_t Bt_ok ;
	uint8_t BtModuleType ;
} ;

extern struct t_bt_control BtControl ;


extern struct t_fifo128 Bt_fifo ;
extern OS_FlagID Bt_flag ;
extern uint16_t BtRxTimer ;

void bt_task(void* pdata) ;
void setBtBaudrate( uint32_t index ) ;
void initBluetooth() ;
void getBtState( void ) ;


#ifdef BT_WITH_ENCODER
void btEncTx( void ) ;
#endif

uint8_t *btAddrBin2Hex( uint8_t *dest, uint8_t *source , uint32_t skipCommas = 0 ) ;

#endif // bluetooth_h

