/****************************************************************************
*  Copyright (c) 2012 by Michael Blandford. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*
****************************************************************************/

#define PIN_INPUT				0x0000
#define	PIN_OP10				0x0001
#define	PIN_OP2					0x0002
#define	PIN_OP50				0x0003

#define PIN_ANALOG			0x0000
#define PIN_FLOAT				0x0004
#define PIN_PULLU_D			0x0008

#define PIN_PULLUP			0x0010

#define PIN_GP_PP				0x0000
#define PIN_GP_OD				0x0004
#define PIN_AF_PP				0x0008
#define PIN_AF_OD				0x000C

#define PIN_MODE_MASK		0x000F

#define PIN_PORTA				0x0000
#define PIN_PORTB				0x0100
#define PIN_PORTC				0x0200
#define PIN_PORTD				0x0300
#define PIN_PORTE				0x0400
#define PIN_PORTF				0x0500
#define PIN_PORTG				0x0600
#define PIN_PORTH				0x0700
#define PIN_PORTI				0x0800
#define PIN_PORTJ				0x0900
#define PIN_PORTK				0x0A00
#define PIN_LOW					0x0000
#define PIN_HIGH				0x1000

#define PIN_PORT_MASK		0x0F00

#define GPIO_Pin_0                 ((uint16_t)0x0001)  /* Pin 0 selected */
#define GPIO_Pin_1                 ((uint16_t)0x0002)  /* Pin 1 selected */
#define GPIO_Pin_2                 ((uint16_t)0x0004)  /* Pin 2 selected */
#define GPIO_Pin_3                 ((uint16_t)0x0008)  /* Pin 3 selected */
#define GPIO_Pin_4                 ((uint16_t)0x0010)  /* Pin 4 selected */
#define GPIO_Pin_5                 ((uint16_t)0x0020)  /* Pin 5 selected */
#define GPIO_Pin_6                 ((uint16_t)0x0040)  /* Pin 6 selected */
#define GPIO_Pin_7                 ((uint16_t)0x0080)  /* Pin 7 selected */
#define GPIO_Pin_8                 ((uint16_t)0x0100)  /* Pin 8 selected */
#define GPIO_Pin_9                 ((uint16_t)0x0200)  /* Pin 9 selected */
#define GPIO_Pin_10                ((uint16_t)0x0400)  /* Pin 10 selected */
#define GPIO_Pin_11                ((uint16_t)0x0800)  /* Pin 11 selected */
#define GPIO_Pin_12                ((uint16_t)0x1000)  /* Pin 12 selected */
#define GPIO_Pin_13                ((uint16_t)0x2000)  /* Pin 13 selected */
#define GPIO_Pin_14                ((uint16_t)0x4000)  /* Pin 14 selected */
#define GPIO_Pin_15                ((uint16_t)0x8000)  /* Pin 15 selected */
#define GPIO_Pin_All               ((uint16_t)0xFFFF)  /* All pins selected */

extern void configure_pins( uint32_t pins, uint16_t config ) ;
extern void init_trims( void ) ;
extern void init_keys( void ) ;
extern uint32_t read_keys( void ) ;
extern uint32_t read_trims( void ) ;
extern uint32_t keyState( enum EnumKeys enuk) ;
extern uint32_t hwKeyState( uint8_t key ) ;
extern void init_trims( void ) ;
extern void setup_switches( void ) ;
extern void config_free_pins( void ) ;

uint32_t switchPosition( uint32_t swtch ) ;

#define GPIO_ResetBits( port, bits ) (port->BRR = bits)
#define GPIO_SetBits( port, bits ) (port->BSRR = bits)
#define GPIO_ReadInputDataBit( port, bit) ( (port->IDR & bit) ? Bit_SET : Bit_RESET)


