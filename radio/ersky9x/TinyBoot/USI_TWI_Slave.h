/****************************************************************************
*  Copyright (c) 2011 by Michael Blandford. All rights reserved.
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
// Developed from Atmel AVR112: TWI Bootloader for devices without boot section
// and changed so that it actually works

static void USI_TWI_SLAVE_Init( void );
static void USI_TWI_SLAVE_ReadAndProcessPacket(void);
static void USI_TWI_SLAVE_Process_Start_Condition(void);
void USI_TWI_SLAVE_Process_Overflow_Condition(void);
 
uint8_t Run_app ;
#define Read_program		GPIOR2
 
#define TRUE                1
#define FALSE               0

 // TWI write states.
#define USI_TWI_SLAVE_WRITE_ADDR_HI_BYTE              (0x00)
#define USI_TWI_SLAVE_WRITE_ADDR_LO_BYTE              (0x01)
#define USI_TWI_SLAVE_WRITE_DATA_BYTE                 (0x02)
  
// TWI overflow states.
#define USI_TWI_SLAVE_OVERFLOW_STATE_NONE             (0x00)
#define USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_RX        (0x01)
#define USI_TWI_SLAVE_OVERFLOW_STATE_DATA_RX          (0x02)
#define USI_TWI_SLAVE_OVERFLOW_STATE_ACK_PR_TX        (0x03)
#define USI_TWI_SLAVE_OVERFLOW_STATE_PR_ACK_TX        (0x04)
#define USI_TWI_SLAVE_OVERFLOW_STATE_CMD_RX           (0x05) 
#define USI_TWI_SLAVE_OVERFLOW_STATE_DATA_TX_AVERSION (0x06) 

#define DDR_USI             DDRB
#define PORT_USI            PORTB
#define PIN_USI             PINB
#define PORT_USI_SDA        PORTB0
#define PORT_USI_SCL        PORTB2
#define PIN_USI_SDA         PINB0
#define PIN_USI_SCL         PINB2
#define USI_START_COND_INT  USISIF
#define USI_START_VECTOR    USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect





 