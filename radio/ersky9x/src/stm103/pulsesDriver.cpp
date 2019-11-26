
/****************************************************************************
*  Copyright (c) 2019 by Michael Blandford. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
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

// USART2 on PD5(Tx) and PD6(Rx)
// Need to remap usart2 to PD5 and PD6

#include <stm32f10x.h>
#include <stdlib.h>
#include "ersky9x.h"
#include "myeeprom.h"
#include "stm103/logicio103.h"
#include "drivers.h"

//-[ Main MCU ==> RF Module
#define	RF_CMD_TX_SET_ID				'a'				//Set model_id
#define	RF_CMD_TX_GET_ID				'b'				//Enquire model ID
#define	RF_CMD_TX_BIND_DSMX			'c'				//Binding DSMX
#define	RF_CMD_TX_BIND_DSM2			'd'				//Binding DSM2
//#define	RF_CMD_TX_BIND_DSMP		'D'				//Binding DSMP
#define	RF_CMD_TX_BIND_DSMP			'B'				//Binding DSMP
#define	RF_CMD_TX_DSMX_DATA			'e'				//DSMX transmission
#define	RF_CMD_TX_DSM2_DATA			'f'				//DSM2 transmission
#define	RF_CMD_TX_DSMX_TESTING	'g'				//DSMX normal test DEMO. Loop forever
#define	RF_CMD_TX_DSM2_TESTING	'h'				//DSM2 normal test
#define	RF_CMD_TX_WRITE_REG			'i'				//Write SDR register, uart1_rx_buffer[1]= address byte, uart1_rx_buffer[2] = data byte;
#define	RF_CMD_TX_READ_REG			'j'				//Read SDR register, uart1_rx_buffer[1]= address byte, uart1_rx_buffer[2] = data byte;
#define	RF_CMD_TX_SET_CH				'k'				//Set SDR channel
#define	RF_CMD_TX_STROBE				'l'				//SDR strobe command
#define	RF_CMD_TX_POWER					'm'				//set SDR transmission power	(0,1,2,3)

#define RF_POWER_BINDING				1		// 1 ~ 3
#define RF_POWER_RANGE_TEST				1		// 1 ~ 3
#define RF_POWER_NORMAL					3		// 1 ~ 3


#define MODECTRL_REG        0x01
#define PLL1_REG            0x0E
#define PLL2_REG            0x0F
#define PLL3_REG            0x10
#define PLL4_REG            0x11
#define PLL5_REG            0x12

//strobe command
#define CMD_SLEEP           0x80      //1000,xxxx   SLEEP mode
#define CMD_IDLE            0x90      //1001,xxxx   IDLE mode
#define CMD_STBY            0xA0      //1010,xxxx   Standby mode
#define CMD_PLL             0xB0      //1011,xxxx   PLL mode
#define CMD_RX              0xC0      //1100,xxxx   RX mode
#define CMD_TX              0xD0      //1101,xxxx   TX mode
#define CMD_TFR             0xE0      //1110,xxxx   TX FIFO reset
#define CMD_RFR             0xF0      //1111,xxxx   RX FIFO reset


//#define BIND_REPLY_DSMS_10B_MIN			0x01
//#define BIND_REPLY_DSMS_10B_MAX			0x02			// 0x01 ~ 0x02:		DSMS-10Bit

#define BIND_REPLY_DSM2_10B_MIN			0x01
#define BIND_REPLY_DSM2_10B_MAX			0x10			// 0x03 ~ 0x10:		DSM2-10Bit

#define BIND_REPLY_DSM2_11B_MIN			0x11
#define BIND_REPLY_DSM2_11B_MAX			0x12			// 0x11 ~ 0x12:		DSM2-11Bit

#define BIND_REPLY_DSMS_11B_MIN			0x13
#define BIND_REPLY_DSMS_11B_MAX			0xa2			// 0x13 ~ 0xa2:		DSMS-11Bit

#define BIND_REPLY_DSMX_11B_MIN			0xa3
#define BIND_REPLY_DSMX_11B_MAX			0xd2			// 0xa3 ~ 0xd2:		DSMX-11Bit

#define BIND_REPLY_DSMP_11B_MIN			0xd3
#define BIND_REPLY_DSMP_11B_MAX			0xfe			// 0xd3 ~ 0xfe:		DSMP-11Bit

#define PROTO_DSM2_10			0
#define PROTO_DSM2_11			1
#define PROTO_DSMS_11			2
#define PROTO_DSMX				3
#define PROTO_DSMP				4


//#define RF_GPIO2_EN1_Pin GPIO_PIN_7
//#define RF_GPIO2_EN1_GPIO_Port GPIOE
//#define RF_RST_Pin GPIO_PIN_8
//#define RF_RST_GPIO_Port GPIOE
//#define RF_POWER_EN_Pin GPIO_PIN_9
//#define RF_POWER_EN_GPIO_Port GPIOE
//#define RF_NSS_IN_Pin GPIO_PIN_12
//#define RF_NSS_IN_GPIO_Port GPIOB
//#define RF_PWM_IN_Pin GPIO_PIN_13
//#define RF_PWM_IN_GPIO_Port GPIOB
//#define RF_GPIO1_MISO_Pin GPIO_PIN_14
//#define RF_GPIO1_MISO_GPIO_Port GPIOB
//#define RF_GPIO1_MOSI_Pin GPIO_PIN_15
//#define RF_GPIO1_MOSI_GPIO_Port GPIOB
//#define RF_NSS_OUT_Pin GPIO_PIN_8
//#define RF_NSS_OUT_GPIO_Port GPIOD


typedef struct u_id
{
    uint16_t off0 ;
    uint16_t off2 ;
    uint32_t off4 ;
    uint32_t off8 ;
} u_id_t ;
u_id_t st_uid ;

uint8_t Id[6] ;
uint8_t BindState ;
uint8_t CurrentRfPower ;
uint16_t Packet0[7] ;
uint16_t Packet1[7] ;
uint16_t DsmBindTimer ;

/* STM32 96-bit Unique ID register */
//#define MMIO16(addr)  (*(volatile uint16_t *)(addr))
//#define MMIO32(addr)  (*(volatile uint32_t *)(addr))
//#define U_ID          0x1ffff7e8		// Unique ID Address

/* Read U_ID register */
//static void app_uid_read(struct u_id *id)
//{
//  id->off0 = MMIO16(U_ID + 0x0) ;
//  id->off2 = MMIO16(U_ID + 0x2) ;
//  id->off4 = MMIO32(U_ID + 0x4) ;
//  id->off8 = MMIO32(U_ID + 0x8) ;
//}

struct t_fifo128 RfTx_fifo ;
struct t_fifo128 RfRx_fifo ;

#define RF_POWER_ON()	GPIOE->BRR = 0x00000200
#define RF_POWER_OFF()	GPIOE->BSRR = 0x00000200

void initDsmUart()
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN ;		// Enable clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPDEN ; 			// Enable portD clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPEEN ; 			// Enable portE clock
	AFIO->MAPR |= AFIO_MAPR_USART2_REMAP ;
	configure_pins( 0x00000020, PIN_OP50 | PIN_AF_PP | PIN_PORTD ) ;		// Tx
	configure_pins( 0x00000040, PIN_INPUT | PIN_FLOAT | PIN_PORTD ) ;		// Rx
	// Rf Power
	configure_pins( 0x00000100, PIN_OP2 | PIN_GP_PP | PIN_PORTE | PIN_HIGH ) ;	// RF RST
	configure_pins( 0x00000200, PIN_OP2 | PIN_GP_PP | PIN_PORTE | PIN_LOW ) ;		// RF POWER
	
	configure_pins( 0x00000080, PIN_INPUT | PIN_PORTE | PIN_PULLU_D ) ;

	USART2->BRR = ( 72000000 / 806400 + 1 ) / 2 ;
	USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE ;
	USART2->CR2 = 0 ;
	USART2->CR3 = 0 ;
	NVIC_SetPriority( USART2_IRQn, 2 ) ; // Changed to 3 for telemetry receive
  NVIC_EnableIRQ(USART2_IRQn) ;
}

void initPulsesDsm()
{
//	app_uid_read( &st_uid ) ;
	
	// setting->qc_preset_id
//	Id[0] = (st_uid.off0 >> 8)  & 0xff ;
//	Id[1] = (st_uid.off2 >> 8)  & 0xff ;
//	Id[2] = (st_uid.off0 >> 0)  & 0xff ;
//	Id[3] = (st_uid.off2 >> 0)  & 0xff ;
//	Id[4] = (st_uid.off4 >> 8)  & 0xff ;
//	Id[5] = (st_uid.off8 >> 8)  & 0xff ;

	uint8_t *p = (uint8_t *) 0xF006 ;
	Id[0] = *p++ ;
	Id[1] = *p++ ;
	Id[2] = *p++ ;
	Id[3] = *p++ ;
	Id[4] = *p++ ;
	Id[5] = *p++ ;
	initDsmUart() ;
}

void putRf( uint8_t byte )
{
	struct t_fifo128 *pfifo = &RfTx_fifo ;
  
	uint32_t next = (pfifo->in + 1) & 0x7f;
	if ( next != pfifo->out )
	{
		USART2->CR1 &= ~USART_CR1_TXEIE ;	// Stop Tx interrupt
		pfifo->fifo[pfifo->in] = byte ;
		pfifo->in = next ;
		USART2->CR1 |= USART_CR1_TXEIE ;		// Start Tx interrupt
	}
}

void setId()
{
	uint32_t i ;
	uint16_t t1 ;
	int32_t response ;

	putRf( RF_CMD_TX_SET_ID ) ;
//	putRf( 0xFF ) ;
	for ( i = 0 ; i < 6 ; i += 1 )
	{
//		putRf( 0 ) ;
		if ( i == 2 )
		{
			putRf( Id[i] + g_model.Module[0].pxxRxNum ) ;
		}
		else
		{
			putRf( Id[i] ) ;
		}
	}
	// We will wait for a reply (for now)

	t1 = getTmr2MHz() ;
	// Timeout of 500uS
	while ( (uint16_t)(getTmr2MHz() - t1) < 1000 )
	{
		response = get_fifo128( &RfRx_fifo ) ;
		if ( response != -1 )
		{
			if ( response == 'c' )
			{
				break ;
			}
		}
	}
}

void getId()
{
	uint32_t i ;
	uint16_t t1 ;
	int32_t response ;

	putRf( RF_CMD_TX_GET_ID ) ;
	// We will wait for the reply

	i = 0 ;
	t1 = getTmr2MHz() ;
	// Timeout of 500uS
	while ( (uint16_t)(getTmr2MHz() - t1) < 1000 )
	{
		response = get_fifo128( &RfRx_fifo ) ;
		if ( response != -1 )
		{
			if ( ++i >= 6 )
			{
				break ;
			}
		}
	}
}

void setRfPower(uint8_t value)
{
	uint16_t t1 ;
	int32_t response ;

	putRf( RF_CMD_TX_POWER ) ;
	putRf( value ) ;
	// We will wait for a reply (for now)
	CurrentRfPower = value ;

	t1 = getTmr2MHz() ;
	// Timeout of 500uS
	while ( (uint16_t)(getTmr2MHz() - t1) < 1000 )
	{
		response = get_fifo128( &RfRx_fifo ) ;
		if ( response != -1 )
		{
			if ( response == 'm' )
			{
				break ;
			}
		}
	}
}


uint16_t readSdrReg( uint8_t address )
{
	uint16_t t1 ;
	int32_t response ;
	
	putRf( RF_CMD_TX_READ_REG	) ;
	putRf( address ) ;
	t1 = getTmr2MHz() ;
	// Timeout of 500uS
	while ( (uint16_t)(getTmr2MHz() - t1) < 1000 )
	{
		response = get_fifo128( &RfRx_fifo ) ;
		if ( response != -1 )
		{
			return response & 0xFF ;
		}
	}
	return -1 ;
}

void writeSdrReg( uint8_t address, uint8_t data )
{
	uint16_t t1 ;
	int32_t response ;
	
	putRf( RF_CMD_TX_WRITE_REG ) ;
	putRf( address ) ;
	putRf( data ) ;
	t1 = getTmr2MHz() ;
	// Timeout of 500uS
	while ( (uint16_t)(getTmr2MHz() - t1) < 1000 )
	{
		response = get_fifo128( &RfRx_fifo ) ;
		if ( response != -1 )
		{
			if ( response == 'i' )
			{
				break ;
			}
		}
	}
}

void strobe( uint8_t value )
{
	uint16_t t1 ;
	int32_t response ;
	
	putRf( RF_CMD_TX_STROBE ) ;
	putRf( value ) ;
	t1 = getTmr2MHz() ;
	// Timeout of 500uS
	while ( (uint16_t)(getTmr2MHz() - t1) < 1000 )
	{
		response = get_fifo128( &RfRx_fifo ) ;
		if ( response != -1 )
		{
			if ( response == 'l' )
			{
				break ;
			}
		}
	}
}

#define	RF_TXBUF_ONCE	16

#define	THROTTLE_CH		(0<<3)							// Channel number
#define	AILERON_CH		(1<<3)							// Channel number
#define	ELEVATOR_CH		(2<<3)							// Channel number
#define	RUDDER_CH			(3<<3)							// Channel number
#define	GEAR_CH				(4<<3)
#define	FLAP_CH				(5<<3)							// FLAP / AUX1
#define	AUX2_CH				(6<<3)							// AUX2

// Send 18 bytes
//    Frame,Page
// 0 = 1,		0
// 1 = 0,		1
// 2 = 1,		0
// 3 = 0,		1

//app_RF_packet_phaser(&tx_buf, 1, 0, dsm_mode, id);

#define	RF_BUFF_PAGE1_DATA			1<<7
#define	RF_BUFF_PAGE0_DATA			0<<7

#define	RF_BUFF_P1_SERVO_01			2
#define	RF_BUFF_P1_SERVO_02			4
#define	RF_BUFF_P1_SERVO_03			6
#define	RF_BUFF_P1_SERVO_04			8
#define	RF_BUFF_P1_SERVO_05			10
#define	RF_BUFF_P1_SERVO_06			12
#define	RF_BUFF_P1_SERVO_07			14

#define	RF_BUFF_P0_SERVO_01			18
#define	RF_BUFF_P0_SERVO_02			20
#define	RF_BUFF_P0_SERVO_03			22
#define	RF_BUFF_P0_SERVO_04			24
#define	RF_BUFF_P0_SERVO_05			26
#define	RF_BUFF_P0_SERVO_06			28
#define	RF_BUFF_P0_SERVO_07			30

#define	DSMX_THROTTLE_INDEX			RF_BUFF_P1_SERVO_01		// tx_buff offset
#define	DSMX_AILERON_INDEX			RF_BUFF_P0_SERVO_01		// tx_buff offset
#define	DSMX_ELEVATOR_INDEX			RF_BUFF_P0_SERVO_03		// tx_buff offset
#define	DSMX_RUDDER_INDEX			RF_BUFF_P1_SERVO_03		// tx_buff offset
#define	DSMX_GEAR_INDEX				RF_BUFF_P0_SERVO_04		// tx_buff offset
#define	DSMX_FLAP_INDEX				RF_BUFF_P0_SERVO_02		// tx_buff offset
#define	DSMX_AUX2_INDEX				RF_BUFF_P0_SERVO_05		// tx_buff offset
#define	DSMX_AUX3_INDEX				RF_BUFF_P1_SERVO_02		// tx_buff offset

// DSMS 11-Bit only
#define	DSMS_11B_THROTTLE_INDEX		RF_BUFF_P1_SERVO_07		// tx_buff offset
#define	DSMS_11B_AILERON_INDEX		RF_BUFF_P1_SERVO_01		// tx_buff offset
#define	DSMS_11B_ELEVATOR_INDEX		RF_BUFF_P1_SERVO_03		// tx_buff offset
#define	DSMS_11B_RUDDER_INDEX		RF_BUFF_P1_SERVO_05		// tx_buff offset
#define	DSMS_11B_GEAR_INDEX			RF_BUFF_P1_SERVO_04		// tx_buff offset
#define	DSMS_11B_FLAP_INDEX			RF_BUFF_P1_SERVO_02		// tx_buff offset
#define	DSMS_11B_AUX2_INDEX			RF_BUFF_P1_SERVO_06		// tx_buff offset

//			rf_txbuf_11b_phaser(servo_value.thu_uint11b, THROTTLE_CH, DSMX_THROTTLE_INDEX);
//			rf_txbuf_11b_phaser(servo_value.ral_uint11b, AILERON_CH,  DSMX_AILERON_INDEX);
//			rf_txbuf_11b_phaser(servo_value.ele_uint11b, ELEVATOR_CH, DSMX_ELEVATOR_INDEX);
//			rf_txbuf_11b_phaser(servo_value.rud_uint11b, RUDDER_CH,   DSMX_RUDDER_INDEX);

//			rf_txbuf_11b_phaser(servo_value.ger_uint11b, GEAR_CH, DSMX_GEAR_INDEX);
//			rf_txbuf_11b_phaser(servo_value.lal_uint11b, FLAP_CH, DSMX_FLAP_INDEX);
//			rf_txbuf_11b_phaser(servo_value.ax2_uint11b, AUX2_CH, DSMX_AUX2_INDEX);
//			rf_txbuf_11b_phaser(servo_value.ax3_uint11b, AUX3_CH, DSMX_AUX3_INDEX);

// Original has 32 byte buffer, first 2 bytes are left
// Rest are pre-filled with FF
// DSMX
// Thr(0) -> 2,3
// Ail(1) -> 18,19
// Ele(2) -> 22,23
// Rud(3) -> 6,7
// Gea(4) -> 24,25
// Flp(5) -> 20,21
// Ax1(6) -> 26,27
// Ax2(7) -> 4,5
// DSMS
// Thr(0) -> 14,15
// Ail(1) -> 2,3
// Ele(2) -> 6,7
// Rud(3) -> 10,11
// Gea(4) -> 8,9
// Flp(5) -> 4,5
// Ax1(6) -> 12,13
// Then send either 2-15 or 18-31

#define POWER_OFF_WAIT		50
#define POWER_ID_WAIT			(POWER_OFF_WAIT + 10)
#define POWER_RF_WAIT			(POWER_ID_WAIT + 5)

uint8_t BindDebugResult ;
uint8_t TestPausing ;

void dataPacket( uint32_t type )
{
	uint16_t pulse ;
	int32_t response ;

  heartbeat |= HEART_TIMER_PULSES ;

	if ( ( g_model.Module[0].protocol == PROTO_DSM2 ) && ( TestPausing == 0 ) )
	{
		if ( BindRangeFlag[0] & PXX_BIND )
		{
			if ( type == 0 )	// Every 22mS
			{
				// process bind sequence
				switch ( BindState )
				{
					case 0 :
  					if((uint16_t)( get_tmr10ms()-DsmBindTimer) >= POWER_OFF_WAIT) // 0.50 sec
						{
							RF_POWER_ON() ;
							BindState = 1 ;
							while ( get_fifo128( &RfRx_fifo ) != -1 )
							{
								// Flush Rx fifo
							}
						}
					break ;	

					case 1 :
  					if((uint16_t)( get_tmr10ms()-DsmBindTimer) >= POWER_ID_WAIT) // 0.60 sec
						{
							setId() ;
							BindState = 2 ;
						}
					break ;	

					case 2 :
  					if((uint16_t)( get_tmr10ms()-DsmBindTimer) >= POWER_RF_WAIT) // 0.65 sec
						{
							setRfPower(1) ;
							BindState = 3 ;
						}
					break ;	
				
					case 3 :
						putRf( RF_CMD_TX_BIND_DSMX ) ;
						BindState = 4 ;
					break ;	

					case 4 :
						response = get_fifo128( &RfRx_fifo ) ;
						if ( response != -1 )
						{
							BindDebugResult = response ;
	//						if ( ( response >= BIND_REPLY_DSMX_11B_MIN ) && ( response <= BIND_REPLY_DSMX_11B_MAX ) )
							if ( ( response >= BIND_REPLY_DSMX_11B_MIN ) && ( response <= BIND_REPLY_DSMP_11B_MAX ) )
							{
								// Bound in DSMX mode
								g_model.Module[0].sub_protocol = PROTO_DSMX ;	// DSMX
								BindRangeFlag[0] = 0 ;
							}
							if ( ( response >= BIND_REPLY_DSMS_11B_MIN ) && ( response <= BIND_REPLY_DSMS_11B_MAX ) )
							{
								// Bound in DSMX mode
								g_model.Module[0].sub_protocol = PROTO_DSMS_11 ;	// DSMS
								BindRangeFlag[0] = 0 ;
							}
							if ( ( response >= BIND_REPLY_DSM2_10B_MIN ) && ( response <= BIND_REPLY_DSM2_10B_MAX ) )
							{
								// Bound in DSM2 mode
								g_model.Module[0].sub_protocol = PROTO_DSM2_10 ;	// DSM2 (10)
								BindRangeFlag[0] = 0 ;
							}
							if ( ( response >= BIND_REPLY_DSM2_11B_MIN ) && ( response <= BIND_REPLY_DSM2_11B_MAX ) )
							{
								// Bound in DSM2 mode
								g_model.Module[0].sub_protocol = PROTO_DSM2_11 ;	// DSM2 (11)
								BindRangeFlag[0] = 0 ;
							}
							BindState = 5 ;
						}
					break ;	
				}
			}
			return ;	// Binding so don't send data packets
		}
		if ( CurrentRfPower != RF_POWER_NORMAL )
		{
			if ( ( BindRangeFlag[0] & (PXX_RANGE_CHECK | PXX_BIND) ) == 0 )
			{		
				setRfPower(RF_POWER_NORMAL) ;	
			}
		}

		if ( g_model.Module[0].sub_protocol == PROTO_DSMX )
		{
			uint16_t *p ;
			putRf( RF_CMD_TX_DSMX_DATA ) ;
	
			putRf( ( (type == 0) || (type == 2) ) ? 1 : 0 ) ;
	
			putRf( Id[2] + g_model.Module[0].pxxRxNum ) ;
			putRf( Id[3] ) ;

			p = (type > 1) ? Packet1 : Packet0 ;
			pulse = *p++ ;
			if ( type > 1 )
			{
				pulse |= 0x8000 ;	// RF_BUFF_PAGE1_DATA
			}
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;

			pulse = *p++ ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
			pulse = *p++ ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
			pulse = *p++ ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
			pulse = *p++ ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
			pulse = *p++ ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
			pulse = *p ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
		}
		else if ( g_model.Module[0].sub_protocol == PROTO_DSMS_11 )
		{
			if (type < 2)	// Send as soon after packet creation as possible		
			{
				uint16_t *p ;
				putRf( RF_CMD_TX_DSMX_DATA ) ;
	
	//			putRf( 1 ) ;
				putRf( (type == 0) ? 1 : 0 ) ;
	
				putRf( Id[2] + g_model.Module[0].pxxRxNum ) ;
				putRf( Id[3] ) ;

				p = Packet1 ;
				pulse = *p++ ;
				putRf( pulse >> 8 ) ;
				putRf( pulse ) ;
				pulse = *p++ ;
				putRf( pulse >> 8 ) ;
				putRf( pulse ) ;
				pulse = *p++ ;
				putRf( pulse >> 8 ) ;
				putRf( pulse ) ;
				pulse = *p++ ;
				putRf( pulse >> 8 ) ;
				putRf( pulse ) ;
				pulse = *p++ ;
				putRf( pulse >> 8 ) ;
				putRf( pulse ) ;
				pulse = *p++ ;
				putRf( pulse >> 8 ) ;
				putRf( pulse ) ;
				pulse = *p ;
				putRf( pulse >> 8 ) ;
				putRf( pulse ) ;
			 
			}
		}
		else if ( ( g_model.Module[0].sub_protocol == PROTO_DSM2_10 )	// DSM2 (10 bit)
								|| ( g_model.Module[0].sub_protocol == PROTO_DSM2_11 ) )	// DSM2 (11 bit)
		{
			uint16_t *p ;

			if ( ( g_model.Module[0].sub_protocol == PROTO_DSM2_10 ) && ( type > 1 ) )
			{
				return ;
			}
			putRf( RF_CMD_TX_DSM2_DATA ) ;
			putRf( ( (type == 0) || (type == 2) ) ? 1 : 0 ) ;
			putRf( 0xFF-Id[2] ) ;
			putRf( 0xFF-Id[3] ) ;

			p = (type > 1) ? Packet1 : Packet0 ;

			pulse = *p++ ;
			if ( type > 1 )
			{
				pulse |= 0x8000 ;	// RF_BUFF_PAGE1_DATA
			}
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;

			pulse = *p++ ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
			pulse = *p++ ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
			pulse = *p++ ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
			pulse = *p++ ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
			pulse = *p++ ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
			pulse = *p ;
			putRf( pulse >> 8 ) ;
			putRf( pulse ) ;
		}
	}
}

void testPauseOutput( uint32_t mode )
{
	TestPausing = mode ;
}

static uint8_t UseClockOutput ;
static uint8_t DcValue ;

extern "C" void EXTI1_IRQHandler()
{
	EXTI->PR = 2 ;
	if ( ++DcValue > 240 )
	{
		DcValue = 40 ;
	}
////	strobe( CMD_STBY ) ;
//	writeSdrReg( MODECTRL_REG, 0x61 ) ; // ADC measurment enable
//	writeSdrReg( 0x35, 0x30 ) ;
//	writeSdrReg( 0x2A, DcValue ) ;
////	strobe( CMD_RX ) ;
}

extern "C" void EXTI9_5_IRQHandler()
{
	EXTI->PR = 0x0080 ;
	if ( GPIOE->IDR & 0x80 )
	{
		// Start receive
		TIM8->CNT = 0 ;
		if ( UseClockOutput )
		{
			configure_pins( 0x0040, PIN_OP10 | PIN_AF_PP | PIN_PORTC ) ;
		}
		else
		{
			configure_pins( 0x0040, PIN_INPUT | PIN_FLOAT | PIN_PORTC ) ;
		}
	}
	else
	{
		// Stop receive
		configure_pins( 0x0040, PIN_OP10 | PIN_GP_PP | PIN_PORTC | PIN_HIGH ) ;
		EXTI->SWIER |= 2 ;
	}
}

void stopGpioInt()
{
	EXTI->IMR &= ~0x0080 ;
	EXTI->IMR &= ~2 ;
	NVIC_DisableIRQ( EXTI9_5_IRQn) ;
	NVIC_DisableIRQ( EXTI1_IRQn) ;
}


void initGpioInt()
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN ;	// Enable clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN ;	// Enable portCclock
	RCC->APB2ENR |= RCC_APB2ENR_IOPEEN ;	// Enable portEclock
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN ;
	configure_pins( 0x0080, PIN_INPUT | PIN_FLOAT | PIN_PORTE ) ;
	configure_pins( 0x0040, PIN_OP10 | PIN_GP_PP | PIN_PORTC | PIN_HIGH ) ;
	AFIO->EXTICR[1] |= 0x4000 ;		// PE7
	EXTI->RTSR |= 0x0080 ;	// Rising Edge
	EXTI->FTSR |= 0x0080 ;	// Falling Edge
	EXTI->IMR |= 0x0080 ;
	EXTI->IMR |= 2 ;
	NVIC_SetPriority( EXTI9_5_IRQn, 1 ) ; // Not quite highest priority interrupt
	NVIC_EnableIRQ( EXTI9_5_IRQn) ;
	NVIC_SetPriority( EXTI1_IRQn, 8 ) ; // Low
	NVIC_EnableIRQ( EXTI1_IRQn) ;

	TIM8->CR1 = 0 ;
	TIM8->CCER = 0 ;
	TIM8->PSC = 0 ;			// 72 MHz
	TIM8->ARR = 719 ;		// 10 uS
	TIM8->CCR1 = 360 ;
  TIM8->CCER = TIM_CCER_CC1E ;
  TIM8->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM
	TIM8->EGR = 0 ;
//  TIM8->BDTR = TIM_BDTR_MOE ;             // Enable outputs
	TIM8->CNT = 0 ;
	TIM8->CR1 = 1 ;

}






void testReceive( uint8_t channel, uint8_t drate, uint8_t deviation, uint8_t clock )
{
	strobe( CMD_STBY ) ;
	writeSdrReg( MODECTRL_REG, 0x61 ) ; // ADC measurment enable
	writeSdrReg( 0x39, drate ) ; // Data rate 100kHz
	writeSdrReg( 0x16, deviation ) ;
	writeSdrReg( PLL1_REG, channel ) ;
	writeSdrReg( 0x2E, 0x1F ) ;
	writeSdrReg( 0x35, 0x30 ) ;
	writeSdrReg( 0x2A, 126 ) ;
	writeSdrReg( 0x0B, 0x21 ) ;
	DcValue = 126 ;
	strobe( CMD_RX ) ;
	UseClockOutput = clock ;
	initGpioInt() ;
}

void stopTestReceive()
{
	stopGpioInt() ;
	strobe(CMD_STBY); //enter Standby
	writeSdrReg( 0x16, 0x17 ) ;
	writeSdrReg( 0x39, 0 ) ; // Data rate 1MHz
	writeSdrReg( 0x2A, 0x80 ) ;
	writeSdrReg( 0x2E, 0x3F ) ;
	writeSdrReg( 0x0B, 0x23 ) ;
}

//uint8_t Registers[64] ;

void initDsmModule()
{
	setId() ;
	setRfPower(RF_POWER_NORMAL) ;	
	getId() ;

//	uint32_t i ;

//	for ( i = 0 ; i < 64 ; i += 1 )
//	{
//		Registers[i] = readSdrReg( i ) ;
//	}
}


// Need interrupts at 0, 4, 11 and 15mS
void startDsmPulses()
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;			// Enable clock
	
	TIM1->CR1 = 0 ;
	TIM1->CCER = 0 ;
	TIM1->PSC = 72000000 / 10000 - 1 ;		// 100uS from 30MHz
	TIM1->ARR = 219 ;		// 22mS
	TIM1->CCR1 = 20 ;
	TIM1->CCR2 = 60 ;
	TIM1->CCR3 = 130 ;
	TIM1->CCR4 = 170 ;
	TIM1->CCMR1 = 0 ;
	TIM1->CCMR2 = 0 ;

	TIM1->EGR = 0 ;
	TIM1->CNT = 180 ;	// Make sure update is first interrupt
	TIM1->CR1 = 1 ;
	TIM1->SR = TIMER1_8SR_MASK & ~(TIM_SR_CC1IF | TIM_SR_CC2IF | TIM_SR_CC3IF | TIM_SR_CC4IF | TIM_SR_UIF ) ;	// Clear flags
	TIM1->DIER = TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_CC3IE | TIM_DIER_CC4IE | TIM_DIER_UIE ;
	NVIC_SetPriority( TIM1_CC_IRQn, 3 ) ; // low
  NVIC_EnableIRQ(TIM1_CC_IRQn) ;
	NVIC_SetPriority(TIM1_UP_IRQn, 6 ) ;
	NVIC_EnableIRQ(TIM1_UP_IRQn) ;
}

extern "C" void TIM1_UP_IRQHandler()
{
	int16_t value ;
	
	TIM1->SR = TIMER1_8SR_MASK & ~TIM_SR_UIF ;	// Clear flag
	// Build packets
	if ( ( g_model.Module[0].sub_protocol == PROTO_DSMX ) || ( g_model.Module[0].sub_protocol == PROTO_DSM2_11 ) )
	{
		value = g_chans512[0] ;
		Packet1[0] = limit(0, ((value*349)>>9)+1024,2047) | (0 << 11) ;
		value = g_chans512[1] ;
		Packet0[0] = limit(0, ((value*349)>>9)+1024,2047) | (1 << 11) ;
		
		value = g_chans512[7] ;
		Packet1[1] = limit(0, ((value*349)>>9)+1024,2047) | (7 << 11) ;
		value = g_chans512[5] ;
		Packet0[1] = limit(0, ((value*349)>>9)+1024,2047) | (5 << 11) ;
		
		value = g_chans512[3] ;
		Packet1[2] = limit(0, ((value*349)>>9)+1024,2047) | (3 << 11) ;
		value = g_chans512[2] ;
		Packet0[2] = limit(0, ((value*349)>>9)+1024,2047) | (2 << 11) ;
		
		Packet1[3] = 0xFFFF ;
		value = g_chans512[4] ;
		Packet0[3] = limit(0, ((value*349)>>9)+1024,2047) | (4 << 11) ;
		
		Packet1[4] = 0xFFFF ;
		value = g_chans512[6] ;
		Packet0[4] = limit(0, ((value*349)>>9)+1024,2047) | (6 << 11) ;
		
		Packet1[5] = 0xFFFF ;
		Packet0[5] = 0xFFFF ;
		
		Packet1[6] = 0xFFFF ;
		Packet0[6] = 0xFFFF ;
	}
	else if ( g_model.Module[0].sub_protocol == PROTO_DSMS_11 )
	{
		value = g_chans512[1] ;
		Packet1[0] = limit(0, ((value*349)>>9)+1024,2047) | (1 << 11) ;
			
		value = g_chans512[5] ;
		Packet1[1] = limit(0, ((value*349)>>9)+1024,2047) | (5 << 11) ;
			
		value = g_chans512[2] ;
		Packet1[2] = limit(0, ((value*349)>>9)+1024,2047) | (2 << 11) ;
			
		value = g_chans512[4] ;
		Packet1[3] = limit(0, ((value*349)>>9)+1024,2047) | (4 << 11) ;
			
		value = g_chans512[3] ;
		Packet1[4] = limit(0, ((value*349)>>9)+1024,2047) | (3 << 11) ;
			
		value = g_chans512[6] ;
		Packet1[5] = limit(0, ((value*349)>>9)+1024,2047) | (6 << 11) ;
			
		value = g_chans512[0] ;
		Packet1[6] = limit(0, ((value*349)>>9)+1024,2047) | (0 << 11) ;
		
	}
	else if ( g_model.Module[0].sub_protocol == PROTO_DSM2_10 )	// DSM2 (10 bit)
	{
		
		value = g_chans512[1] ;
		Packet0[0] = ( limit(0, ((value*349)>>9)+1024,2047) >> 1 ) | (1 << 10) ;
		
		value = g_chans512[5] ;
		Packet0[1] = ( limit(0, ((value*349)>>9)+1024,2047) >> 1 ) | (5 << 10) ;
		
		value = g_chans512[4] ;
		Packet0[2] = ( limit(0, ((value*349)>>9)+1024,2047) >> 1 ) | (4 << 10) ;
		
		value = g_chans512[2] ;
		Packet0[3] = ( limit(0, ((value*349)>>9)+1024,2047) >> 1 ) | (2 << 10) ;

		value = g_chans512[6] ;
		Packet0[4] = ( limit(0, ((value*349)>>9)+1024,2047) >> 1 ) | (6 << 10) ;

		value = g_chans512[0] ;
		Packet0[5] = ( limit(0, ((value*349)>>9)+1024,2047) >> 1 ) | (0 << 10) ;
		
		value = g_chans512[3] ;
		Packet0[6] = ( limit(0, ((value*349)>>9)+1024,2047) >> 1 ) | (3 << 10) ;

	}
}

extern "C" void TIM1_CC_IRQHandler()
{
  uint32_t status ;
  status = TIM1->SR ;
	if ( status & TIM_SR_CC1IF )
	{
		TIM1->SR = TIMER1_8SR_MASK & ~(TIM_SR_CC1IF) ;	// Clear flag
		dataPacket(0) ;
	}
	else if ( status & TIM_SR_CC2IF )
	{
		TIM1->SR = TIMER1_8SR_MASK & ~(TIM_SR_CC2IF) ;	// Clear flag
		dataPacket(1) ;
	}
	else if ( status & TIM_SR_CC3IF )
	{
		TIM1->SR = TIMER1_8SR_MASK & ~(TIM_SR_CC3IF) ;	// Clear flag
		dataPacket(2) ;
	}
	else if ( status & TIM_SR_CC4IF )
	{
		TIM1->SR = TIMER1_8SR_MASK & ~(TIM_SR_CC4IF) ;	// Clear flag
		dataPacket(3) ;
	}
}

extern "C" void USART2_IRQHandler()
{
  uint32_t status ;
	USART_TypeDef *puart = USART2 ;

  status = puart->SR ;
	if ( ( status & USART_SR_TXE ) && (puart->CR1 & USART_CR1_TXEIE ) )
	{
		struct t_fifo128 *pfifo = &RfTx_fifo ;
		if ( pfifo->in != pfifo->out )				// Look for char available
		{
			puart->DR = pfifo->fifo[pfifo->out] ;
			pfifo->out = ( pfifo->out + 1 ) & 0x7F ;
		}
		else
		{
			puart->CR1 &= ~USART_CR1_TXEIE ;	// Stop Tx interrupt
		}
	}
	else
	{
		if ( ( status & USART_SR_RXNE ) && (puart->CR1 & USART_CR1_RXNEIE ) )
		{
			struct t_fifo128 *pfifo = &RfRx_fifo ;
  		
			uint32_t next = (pfifo->in + 1) & 0x7f ;
//			if ( next != pfifo->out )
			{
				pfifo->fifo[pfifo->in] = puart->DR ;
				pfifo->in = next ;
			}
//			else
//			{
//				(void) puart->DR ;
//			}
		}
	}
}

void startDsmBind()
{
	DsmBindTimer = get_tmr10ms() ;
	BindState = 0 ;
	RF_POWER_OFF() ;
}

void startDsmRangeCheck()
{
	setRfPower(1) ;
}


void frsky_receive_byte( uint8_t data ) ;

void checkDsmTelemetry5ms()
{
	static uint8_t noData = 0 ;	
	int32_t byte ;

	if ( BindRangeFlag[0] & PXX_BIND )
	{
		return ;
	}

	byte = get_fifo128( &RfRx_fifo ) ;
	if ( byte == -1 )
	{
		noData = 1 ;
		return ;
	}
	if ( noData )
	{
		noData = 0 ;
		put_fifo128( &Com1_fifo, 0xAA ) ;
		put_fifo128( &Com1_fifo, 0x1F ) ;
	}
	put_fifo128( &Com1_fifo, byte ) ;
	while ( ( byte = get_fifo128( &RfRx_fifo ) ) != -1 )
	{
		put_fifo128( &Com1_fifo, byte ) ;
	}
}


