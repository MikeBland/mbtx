/****************************************************************************
*  Copyright (c) 2021 by Michael Blandford. All rights reserved.
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

#include <stdint.h>

#include "ersky9x.h"

#include "X12D/stm32f4xx_rcc.h"
//#include "stm32h7xx.h"
//#include "gpioInit.h"
#include "tp_gt911.h"
#include "logicio.h"
//#include "stm32h7xx_ll_i2c.h"

//#define TOUCH_INT_RCC_AHB1Periph        RCC_AHB1Periph_GPIOH
//#define TOUCH_INT_GPIO                  GPIOH
//#define TOUCH_INT_GPIO_PIN              GPIO_Pin_2    // PH.02

//#define TOUCH_RST_RCC_AHB1Periph        RCC_AHB1Periph_GPIOF
//#define TOUCH_RST_GPIO                  GPIOF
//#define TOUCH_RST_GPIO_PIN              GPIO_Pin_10   // PF.10

uint32_t touchRead( uint16_t regAddr, uint8_t *destAddr, uint32_t count ) ;

#if defined (PCBT18)
const uint8_t TOUCH_GT911_Cfg[] = {
    GT911_CFG_NUMER,  // 0x8047 Config version
    0xE0,             // 0x8048 X output map : x 480
    0x01,
    0x10,  // 0x804A Y ouptut max : y 272
    0x01,
    GT911_MAX_TP,  // 0x804C Touch number
    0x0C,          // 0x804D Module switch 1 : bit4= xy change Int mode
    0x00,          // 0x804E Module switch 2
    0x01,          // 0x804F Shake_Count
    0x18,          // 0x8050 Filter
    0x28,          // 0x8051 Larger touch
    0x0F,          // 0x8052 Noise reduction
    0x50,          // 0x8053 Screen touch level
    0x3C,          // 0x8054 Screen touch leave
    0x03,          // 0x8055 Low power control
    0x0F,          // 0x8056 Refresh rate
    0x01,          // 0x8057 X threshold
    0x01,          // 0x8058 Y threshold
    0x00,          // 0x8059 Reserved
    0x00,          // 0x805A Reserved
    0x00,          // 0x805B Space (top, bottom)
    0x00,          // 0x805C Space (left, right)
    0x04,          // 0x805D Mini filter
    0x18,          // 0x805E Strech R0
    0x1A,          // 0x805F Strech R1
    0x1E,          // 0x8060 Strech R2
    0x14,          // 0x8061 Strech RM
    0x87,          // 0x8062 Drv groupA num
    0x29,          // 0x8063 Drv groupB num
    0x0A,          // 0x8064 Sensor num
    0x4B,          // 0x8065 FreqA factor
    0x4D,          // 0x8066 FreqB factor
    0xD3,          // 0x8067 Panel bit freq
    0x07,
    0x00,  // 0x8069 Reserved
    0x00,
    0x00,  // 0x806B Panel tx gain
    0x19,  // 0x806C Panel rx gain
    0x02,  // 0x806D Panel dump shift
    0x10,  // 0x806E Drv frame control
    0x32,  // 0x806F Charging level up
    0x00,  // 0x8070 Module switch 3
    0x00,  // 0x8071 Gesture distance
    0x00,  // 0x8072 Gesture long press time
    0x00,  // 0x8073 X/Y slope adjust
    0x03,  // 0x8074 Gesture control
    0x00,  // 0x8075 Gesture switch 1
    0x00,  // 0x8076 Gesture switch 2
    0x00,  // 0x8077 Gesture refresh rate
    0x00,  // 0x8078 Gesture touch level
    0x00,  // 0x8079 New green wake up level
    0x32,  // 0x807A Freq hopping start
    0x73,  // 0x807B Freq hopping end
    0x94,  // 0x807C Noise detect time
    0xD5,  // 0x807D Hopping flag
    0x02,  // 0x807E Hopping flag
    0x07,  // 0x807F Noise threshold
    0x00,  // 0x8080 Nois min threshold old
    0x00,  // 0x8081 Reserved
    0x04,  // 0x8082 Hopping sensor group
    0xAE,  // 0x8083 Hopping Seg1 normalize
    0x36,  // 0x8084 Hopping Seg1 factor
    0x00,  // 0x8085 Main clock adjust
    0x94,  // 0x8086 Hopping Seg2 normalize
    0x40,  // 0x8087 Hopping Seg2 factor
    0x00,  // 0x8088 Reserved
    0x7E,  // 0x8089 Hopping Seg3 normalize
    0x4C,  // 0x808A Hopping Seg3 factor
    0x00,  // 0x808B Reserved
    0x6D,  // 0x808C Hopping Seg4 normalize
    0x59,  // 0x808D Hopping Seg4 factor
    0x00,  // 0x808E Reserved
    0x5D,  // 0x808F Hopping Seg5 normalize
    0x6A,  // 0x8090 Hopping Seg5 factor
    0x00,  // 0x8091 Reserved
    0x5D,  // 0x8092 Hopping Seg6 normalize
    0x00,  // 0x8093 Key1
    0x00,  // 0x8094 Key2
    0x00,  // 0x8095 Key3
    0x00,  // 0x8096 Key4
    0x00,  // 0x8097 Key area
    0x00,  // 0x8098 Key touch level
    0x00,  // 0x8099 Key leave level
    0x00,  // 0x809A Key sens
    0x00,  // 0x809B Key sens
    0x00,  // 0x809C Key restrain
    0x00,  // 0x809D Key restrain time
    0x00,  // 0x809E Large gesture touch
    0x00,  // 0x809F Reserved
    0x00,  // 0x80A0 Reserved
    0x00,  // 0x80A1 Hotknot noise map
    0x00,  // 0x80A2 Link threshold
    0x00,  // 0x80A3 Pxy threshold
    0x00,  // 0x80A4 GHot dump shift
    0x00,  // 0x80A5 GHot rx gain
    0x00,  // 0x80A6 Freg gain
    0x00,  // 0x80A7 Freg gain 1
    0x00,  // 0x80A8 Freg gain 2
    0x00,  // 0x80A9 Freg gain 3
    0x00,  // 0x80AA Reserved
    0x00,  // 0x80AB Reserved
    0x00,  // 0x80AC Reserved
    0x00,  // 0x80AD Reserved
    0x00,  // 0x80AE Reserved
    0x00,  // 0x80AF Reserved
    0x00,  // 0x80B0 Reserved
    0x00,  // 0x80B1 Reserved
    0x00,  // 0x80B2 Reserved
    0x00,  // 0x80B3 Combine dis
    0x00,  // 0x80B4 Split set
    0x00,  // 0x80B5 Reserved
    0x00,  // 0x80B6 Reserved
    0x0A,  // 0x80B7 Sensor CH0
    0x0C,  // 0x80B8 Sensor CH1
    0x0E,  // 0x80B9 Sensor CH2
    0x10,  // 0x80BA Sensor CH3
    0x12,  // 0x80BB Sensor CH4
    0x14,  // 0x80BC Sensor CH5
    0x16,  // 0x80BD Sensor CH6
    0x18,  // 0x80BE Sensor CH7
    0x1A,  // 0x80BF Sensor CH8
    0x1C,  // 0x80C0 Sensor CH9
    0xFF,  // 0x80C1 Sensor CH10
    0xFF,  // 0x80C2 Sensor CH11
    0xFF,  // 0x80C3 Sensor CH12
    0xFF,  // 0x80C4 Sensor CH13
    0xFF,  // 0x80C5 Reserved
    0xFF,  // 0x80C6 Reserved
    0xFF,  // 0x80C7 Reserved
    0xFF,  // 0x80C8 Reserved
    0xFF,  // 0x80C9 Reserved
    0xFF,  // 0x80CA Reserved
    0xFF,  // 0x80CB Reserved
    0xFF,  // 0x80CC Reserved
    0xFF,  // 0x80CD Reserved
    0xFF,  // 0x80CE Reserved
    0xFF,  // 0x80CF Reserved
    0xFF,  // 0x80D0 Reserved
    0xFF,  // 0x80D1 Reserved
    0xFF,  // 0x80D2 Reserved
    0xFF,  // 0x80D3 Reserved
    0xFF,  // 0x80D4 Reserved
    0x00,  // 0x80D5 Driver CH0
    0x02,  // 0x80D6 Driver CH1
    0x04,  // 0x80D7 Driver CH2
    0x06,  // 0x80D8 Driver CH3
    0x08,  // 0x80D9 Driver CH4
    0x0A,  // 0x80DA Driver CH5
    0x0C,  // 0x80DB Driver CH6
    0x1D,  // 0x80DC Driver CH7
    0x1E,  // 0x80DD Driver CH8
    0x1F,  // 0x80DE Driver CH9
    0x20,  // 0x80DF Driver CH10
    0x21,  // 0x80E0 Driver CH11
    0x22,  // 0x80E1 Driver CH12
    0x24,  // 0x80E2 Driver CH13
    0x26,  // 0x80E3 Driver CH14
    0x28,  // 0x80E4 Driver CH15
    0xFF,  // 0x80E5 Driver CH16
    0xFF,  // 0x80E6 Driver CH17
    0xFF,  // 0x80E7 Driver CH18
    0xFF,  // 0x80E8 Driver CH19
    0xFF,  // 0x80E9 Driver CH20
    0xFF,  // 0x80EA Driver CH21
    0xFF,  // 0x80EB Driver CH22
    0xFF,  // 0x80EC Driver CH23
    0xFF,  // 0x80ED Driver CH24
    0xFF,  // 0x80EE Driver CH25
    0xFF,  // 0x80EF Reserved
    0xFF,  // 0x80F0 Reserved
    0xFF,  // 0x80F1 Reserved
    0xFF,  // 0x80F2 Reserved
    0xFF,  // 0x80F3 Reserved
    0xFF,  // 0x80F4 Reserved
    0xFF,  // 0x80F5 Reserved
    0xFF,  // 0x80F6 Reserved
    0xFF,  // 0x80F7 Reserved
    0xFF,  // 0x80F8 Reserved
    0xFF,  // 0x80F9 Reserved
    0xFF,  // 0x80FA Reserved
    0xFF,  // 0x80FB Reserved
    0xFF,  // 0x80FC Reserved
    0xFF,  // 0x80FD Reserved
    0xFF   // 0x80FE Reserved
};

#else

//GT911 param table
const uint8_t TOUCH_GT911_Cfg[] =
  {
    GT911_CFG_NUMER,     // 0x8047 Config version
    0xE0,                // 0x8048 X output map : x 480
    0x01,
    0x10,                // 0x804A Y ouptut max : y 272
    0x01,
    GT911_MAX_TP,        // 0x804C Touch number
    0x3C,                // 0x804D Module switch 1 : bit4= xy change Int mode
    0x20,                // 0x804E Module switch 2
    0x22,                // 0x804F Shake_Count
    0x0A,                // 0x8050 Filter
    0x28,                // 0x8051 Larger touch
    0x0F,                // 0x8052 Noise reduction
    0x5A,                // 0x8053 Screen touch level
    0x3C,                // 0x8054 Screen touch leave
    0x03,                // 0x8055 Low power control
    0x0F,                // 0x8056 Refresh rate
    0x01,                // 0x8057 X threshold
    0x01,                // 0x8058 Y threshold
    0x00,                // 0x8059 Reserved
    0x00,                // 0x805A Reserved
    0x11,                // 0x805B Space (top, bottom)
    0x11,                // 0x805C Space (left, right)
    0x08,                // 0x805D Mini filter
    0x18,                // 0x805E Strech R0
    0x1A,                // 0x805F Strech R1
    0x1E,                // 0x8060 Strech R2
    0x14,                // 0x8061 Strech RM
    0x87,                // 0x8062 Drv groupA num
    0x29,                // 0x8063 Drv groupB num
    0x0A,                // 0x8064 Sensor num
    0xCF,                // 0x8065 FreqA factor
    0xD1,                // 0x8066 FreqB factor
    0xB2,                // 0x8067 Panel bit freq
    0x04,
    0x00,                // 0x8069 Reserved
    0x00,
    0x00,                // 0x806B Panel tx gain
    0xD8,                // 0x806C Panel rx gain
    0x02,                // 0x806D Panel dump shift
    0x1D,                // 0x806E Drv frame control
    0x00,                // 0x806F Charging level up
    0x01,                // 0x8070 Module switch 3
    0x00,                // 0x8071 Gesture distance
    0x00,                // 0x8072 Gesture long press time
    0x00,                // 0x8073 X/Y slope adjust
    0x00,                // 0x8074 Gesture control
    0x00,                // 0x8075 Gesture switch 1
    0x00,                // 0x8076 Gesture switch 2
    0x00,                // 0x8077 Gesture refresh rate
    0x00,                // 0x8078 Gesture touch level
    0x00,                // 0x8079 New green wake up level
    0xB4,                // 0x807A Freq hopping start
    0xEF,                // 0x807B Freq hopping end
    0x94,                // 0x807C Noise detect time
    0xD5,                // 0x807D Hopping flag
    0x02,                // 0x807E Hopping flag
    0x07,                // 0x807F Noise threshold
    0x00,                // 0x8080 Nois min threshold old
    0x00,                // 0x8081 Reserved
    0x04,                // 0x8082 Hopping sensor group
    0x6E,                // 0x8083 Hopping Seg1 normalize
    0xB9,                // 0x8084 Hopping Seg1 factor
    0x00,                // 0x8085 Main clock adjust
    0x6A,                // 0x8086 Hopping Seg2 normalize
    0xC4,                // 0x8087 Hopping Seg2 factor
    0x00,                // 0x8088 Reserved
    0x66,                // 0x8089 Hopping Seg3 normalize
    0xCF,                // 0x808A Hopping Seg3 factor
    0x00,                // 0x808B Reserved
    0x62,                // 0x808C Hopping Seg4 normalize
    0xDB,                // 0x808D Hopping Seg4 factor
    0x00,                // 0x808E Reserved
    0x5E,                // 0x808F Hopping Seg5 normalize
    0xE8,                // 0x8090 Hopping Seg5 factor
    0x00,                // 0x8091 Reserved
    0x5E,                // 0x8092 Hopping Seg6 normalize
    0x00,                // 0x8093 Key1
    0x00,                // 0x8094 Key2
    0x00,                // 0x8095 Key3
    0x00,                // 0x8096 Key4
    0x00,                // 0x8097 Key area
    0x00,                // 0x8098 Key touch level
    0x00,                // 0x8099 Key leave level
    0x00,                // 0x809A Key sens
    0x00,                // 0x809B Key sens
    0x00,                // 0x809C Key restrain
    0x00,                // 0x809D Key restrain time
    0x00,                // 0x809E Large gesture touch
    0x00,                // 0x809F Reserved
    0x00,                // 0x80A0 Reserved
    0x00,                // 0x80A1 Hotknot noise map
    0x00,                // 0x80A2 Link threshold
    0x00,                // 0x80A3 Pxy threshold
    0x00,                // 0x80A4 GHot dump shift
    0x00,                // 0x80A5 GHot rx gain
    0x00,                // 0x80A6 Freg gain
    0x00,                // 0x80A7 Freg gain 1
    0x00,                // 0x80A8 Freg gain 2
    0x00,                // 0x80A9 Freg gain 3
    0x00,                // 0x80AA Reserved
    0x00,                // 0x80AB Reserved
    0x00,                // 0x80AC Reserved
    0x00,                // 0x80AD Reserved
    0x00,                // 0x80AE Reserved
    0x00,                // 0x80AF Reserved
    0x00,                // 0x80B0 Reserved
    0x00,                // 0x80B1 Reserved
    0x00,                // 0x80B2 Reserved
    0x00,                // 0x80B3 Combine dis
    0x00,                // 0x80B4 Split set
    0x00,                // 0x80B5 Reserved
    0x00,                // 0x80B6 Reserved
    0x14,                // 0x80B7 Sensor CH0
    0x12,                // 0x80B8 Sensor CH1
    0x10,                // 0x80B9 Sensor CH2
    0x0E,                // 0x80BA Sensor CH3
    0x0C,                // 0x80BB Sensor CH4
    0x0A,                // 0x80BC Sensor CH5
    0x08,                // 0x80BD Sensor CH6
    0x06,                // 0x80BE Sensor CH7
    0x04,                // 0x80BF Sensor CH8
    0x02,                // 0x80C0 Sensor CH9
    0xFF,                // 0x80C1 Sensor CH10
    0xFF,                // 0x80C2 Sensor CH11
    0xFF,                // 0x80C3 Sensor CH12
    0xFF,                // 0x80C4 Sensor CH13
    0x00,                // 0x80C5 Reserved
    0x00,                // 0x80C6 Reserved
    0x00,                // 0x80C7 Reserved
    0x00,                // 0x80C8 Reserved
    0x00,                // 0x80C9 Reserved
    0x00,                // 0x80CA Reserved
    0x00,                // 0x80CB Reserved
    0x00,                // 0x80CC Reserved
    0x00,                // 0x80CD Reserved
    0x00,                // 0x80CE Reserved
    0x00,                // 0x80CF Reserved
    0x00,                // 0x80D0 Reserved
    0x00,                // 0x80D1 Reserved
    0x00,                // 0x80D2 Reserved
    0x00,                // 0x80D3 Reserved
    0x00,                // 0x80D4 Reserved
    0x28,                // 0x80D5 Driver CH0
    0x26,                // 0x80D6 Driver CH1
    0x24,                // 0x80D7 Driver CH2
    0x22,                // 0x80D8 Driver CH3
    0x21,                // 0x80D9 Driver CH4
    0x20,                // 0x80DA Driver CH5
    0x1F,                // 0x80DB Driver CH6
    0x1E,                // 0x80DC Driver CH7
    0x1D,                // 0x80DD Driver CH8
    0x0C,                // 0x80DE Driver CH9
    0x0A,                // 0x80DF Driver CH10
    0x08,                // 0x80E0 Driver CH11
    0x06,                // 0x80E1 Driver CH12
    0x04,                // 0x80E2 Driver CH13
    0x02,                // 0x80E3 Driver CH14
    0x00,                // 0x80E4 Driver CH15
    0xFF,                // 0x80E5 Driver CH16
    0xFF,                // 0x80E6 Driver CH17
    0xFF,                // 0x80E7 Driver CH18
    0xFF,                // 0x80E8 Driver CH19
    0xFF,                // 0x80E9 Driver CH20
    0xFF,                // 0x80EA Driver CH21
    0xFF,                // 0x80EB Driver CH22
    0xFF,                // 0x80EC Driver CH23
    0xFF,                // 0x80ED Driver CH24
    0xFF,                // 0x80EE Driver CH25
    0x00,                // 0x80EF Reserved
    0x00,                // 0x80F0 Reserved
    0x00,                // 0x80F1 Reserved
    0x00,                // 0x80F2 Reserved
    0x00,                // 0x80F3 Reserved
    0x00,                // 0x80F4 Reserved
    0x00,                // 0x80F5 Reserved
    0x00,                // 0x80F6 Reserved
    0x00,                // 0x80F7 Reserved
    0x00,                // 0x80F8 Reserved
    0x00,                // 0x80F9 Reserved
    0x00,                // 0x80FA Reserved
    0x00,                // 0x80FB Reserved
    0x00,                // 0x80FC Reserved
    0x00,                // 0x80FD Reserved
    0x00                 // 0x80FE Reserved
  };

#endif

uint8_t TouchEventOccured ;
uint8_t TouchUpdated ;
uint8_t TouchBacklight ;

struct t_TouchData TouchData ;

extern void hw_delay( uint16_t time ) ;	// units of 0.1uS
extern void swDelay( uint32_t time ) ;


void swDelay( uint32_t time )
{
	while ( time )
	{
		hw_delay(10000) ;
		time -= 1 ;
	}
}


#define TOUCH_INT_RCC_AHB1Periph        RCC_AHB1Periph_GPIOH
#define TOUCH_INT_GPIO                  GPIOH
#define TOUCH_INT_GPIO_PIN              GPIO_Pin_2    // PH.02
#define TOUCH_INT_PORT								  PIN_PORTH

#define TOUCH_RST_RCC_AHB1Periph        RCC_AHB1Periph_GPIOF
#define TOUCH_RST_GPIO                  GPIOF
#define TOUCH_RST_GPIO_PIN              GPIO_Pin_10   // PF.10
#define TOUCH_RST_PORT								  PIN_PORTF

#define TOUCH_INT_EXTI_LINE1            EXTI_Line2
#define TOUCH_INT_EXTI_IRQn1            EXTI2_IRQn
#define TOUCH_INT_EXTI_IRQHandler1      EXTI2_IRQHandler
#define TOUCH_INT_EXTI_PortSource       EXTI_PortSourceGPIOH
#define TOUCH_INT_EXTI_PinSource1       EXTI_PinSource2

#define TOUCH_INT_STATUS()              (GPIO_ReadInputDataBit(TOUCH_INT_GPIO, TOUCH_INT_GPIO_PIN))

// First I2C Bus
#if defined(RADIO_T18)
  #define I2C_RCC_AHB1Periph           RCC_AHB1Periph_GPIOH
  #define I2C_RCC_APB1Periph           RCC_APB1Periph_I2C3
  #define I2C                          I2C3
  #define I2C_GPIO                     GPIOH
  #define I2C_SCL_BIT			             GPIO_Pin_7  // PH.07
  #define I2C_SDA_BIT			             GPIO_Pin_8  // PH.08
  #define I2C_GPIO_AF                  GPIO_AF_I2C3
  #define I2C_SCL_GPIO_PinSource       GPIO_PinSource7
  #define I2C_SDA_GPIO_PinSource       GPIO_PinSource8
	#define I2C_PORT										 PIN_PORTH
#else
  #define I2C_RCC_AHB1Periph           RCC_AHB1Periph_GPIOB
  #define I2C_RCC_APB1Periph           RCC_APB1Periph_I2C1
  #define I2C                          I2C1
  #define I2C_GPIO                     GPIOB
  #define I2C_SDA_GPIO                 GPIOB
  #define I2C_SCL_GPIO                 GPIOB
  #define I2C_SCL_BIT			             GPIO_Pin_8  // PB.08
  #define I2C_SDA_BIT			             GPIO_Pin_9  // PB.09
  #define I2C_GPIO_AF                  GPIO_AF_I2C1
  #define I2C_SCL_GPIO_PinSource       GPIO_PinSource8
  #define I2C_SDA_GPIO_PinSource       GPIO_PinSource9
	#define I2C_PORT										 PIN_PORTB
#endif
#define I2C_CLK_RATE                      400000

#define	EE_CMD_WRITE  (0)
#define	EE_CMD_READ   (1)

// 400kHz
#define	I2C_delay()   hw_delay( 25 )

#define SCL_H         I2C_SCL_GPIO->BSRRL = I2C_SCL_BIT
#define SCL_L         I2C_SCL_GPIO->BSRRH  = I2C_SCL_BIT
#define SDA_H         I2C_SDA_GPIO->BSRRL = I2C_SDA_BIT
#define SDA_L         I2C_SDA_GPIO->BSRRH  = I2C_SDA_BIT
#define SDA_read      (I2C_SDA_GPIO->IDR  & I2C_SDA_BIT)



static short I2C_START()
{
  SDA_H;
  I2C_delay();
  SCL_H;
  I2C_delay();
  // if (!SDA_read) return 0;
  SDA_L;
  I2C_delay();
  // if (SDA_read) return 0;
  SCL_L;
  I2C_delay();
  return 1;
}

static void I2C_STOP()
{
  SCL_L;
  I2C_delay();
  SDA_L;
  I2C_delay();
  SCL_H;
  I2C_delay();
  SDA_H;
  I2C_delay();
}

static void I2C_ACK()
{
  SCL_L;
  I2C_delay();
  SDA_L;
  I2C_delay();
  SCL_H;
  I2C_delay();
  SCL_L;
  I2C_delay();
}
static void I2C_NO_ACK()
{
  SCL_L;
  I2C_delay();
  SDA_H;
  I2C_delay();
  SCL_H;
  I2C_delay();
  SCL_L;
  I2C_delay();
}

static short I2C_WAIT_ACK()
{
  short i=50;
  SCL_L;
  I2C_delay();
  SDA_H;
  I2C_delay();
  SCL_H;
  I2C_delay();
  while(i) {
    if(SDA_read) {
      I2C_delay();
      i--;
    }
    else {
      i=2;
      break;
    }
  }
  SCL_L;
  I2C_delay();

  return i;
} 

static void I2C_SEND_DATA(char SendByte)
{
  short i=8;
  while (i--)
	 {
    SCL_L;
//    I2C_delay();
    if (SendByte & 0x80)
      SDA_H;
    else
      SDA_L;
    SendByte <<= 1;
    I2C_delay();
    SCL_H;
    I2C_delay();
  }
  SCL_L;
  I2C_delay();
}

static char I2C_READ()
{ 
  short i=8;
  char ReceiveByte=0;

  SDA_H;
  while (i--) {
    ReceiveByte <<= 1;
    SCL_L;
    I2C_delay();
    SCL_H;
    I2C_delay();
    if (SDA_read) {
      ReceiveByte|=0x01;
    }
  }
  SCL_L;
  return ReceiveByte;
} 


void i2cWrite( uint32_t devAddr, uint8_t *buffer, uint32_t count )
{
	I2C_START() ;
	I2C_SEND_DATA( devAddr ) ;
	I2C_WAIT_ACK() ;
  while (count--)
	{
    I2C_SEND_DATA(*buffer++) ;
		I2C_WAIT_ACK() ;
	}
	I2C_STOP() ;
}

void i2cRead( uint32_t devAddr, uint8_t *buffer, uint32_t count )
{
	I2C_START() ;
	I2C_SEND_DATA( devAddr | 1 ) ;
	I2C_WAIT_ACK() ;
  while (count--)
	{
		*buffer++ = I2C_READ() ;
    if ( count == 0)
		{
			I2C_NO_ACK() ;
    }
		else
		{
			I2C_ACK() ;
		}
  }
	I2C_STOP() ;
}


void TOUCH_AF_GPIOConfig(void)
{
	configure_pins( TOUCH_INT_GPIO_PIN, TOUCH_INT_PORT | PIN_HIGH | PIN_OUTPUT | PIN_OS50 ) ;
	configure_pins( TOUCH_RST_GPIO_PIN, TOUCH_RST_PORT | PIN_HIGH | PIN_OUTPUT | PIN_OS50 ) ;
}

void TOUCH_AF_INT_Change(void)
{
	configure_pins( TOUCH_INT_GPIO_PIN, TOUCH_INT_PORT | PIN_PULLUP | PIN_INPUT ) ;
}

void I2C_GPIOConfig(void)
{
	configure_pins( I2C_SCL_BIT | I2C_SDA_BIT, I2C_PORT | PIN_PULLUP | PIN_HIGH | PIN_ODRAIN | PIN_OUTPUT | PIN_OS100 ) ;
}

void initTouchExti()
{
	SYSCFG->EXTICR[0] |= 0x0700 ;		// PH2
	EXTI->RTSR |= GPIO_Pin_2 ;	// Rising Edge
	EXTI->IMR |= GPIO_Pin_2 ;
	
	NVIC_SetPriority( EXTI2_IRQn, 5 ) ;
	NVIC_EnableIRQ( EXTI2_IRQn) ;
}

void I2C_Init()
{
	I2C_GPIOConfig() ;
//	I2C->CR1 = 0 ;
//	I2C->TIMINGR = 0x90320309 ;
//	I2C->CR1 |= 1 ;
	initTouchExti() ;
}


extern "C" void EXTI2_IRQHandler(void)
{
	if ( EXTI->PR & TOUCH_INT_GPIO_PIN )
	{
//    if (g_eeGeneral.backlightMode & e_backlight_mode_keys)
//		{
//      // on touch turn the light on
//      resetBacklightTimeout();
//    }
		TouchEventOccured = 1 ;
		EXTI->PR = TOUCH_INT_GPIO_PIN ;
  }
}

uint32_t touchPanelInit(void)
{
//	TOUCH_AF_ExtiConfig() ;
	
	TOUCH_AF_GPIOConfig() ; //SET RST=OUT INT=OUT INT=LOW
//	I2C4_Init() ;

	TOUCH_RST_GPIO->BSRRH = TOUCH_RST_GPIO_PIN ;	// RST low
	TOUCH_INT_GPIO->BSRRL = TOUCH_INT_GPIO_PIN ;	// INT high

	hw_delay( 2000 ) ;	// 0.1uS units

	TOUCH_RST_GPIO->BSRRL = TOUCH_RST_GPIO_PIN ;	// RST high
	swDelay( 6 ) ;	// mS
	TOUCH_INT_GPIO->BSRRH = TOUCH_INT_GPIO_PIN ;	// INT low

	swDelay( 50 ) ;	// mS

//	TOUCH_INT_GPIO->BSRR = TOUCH_INT_GPIO_PIN << 16 ;	// INT low

//	swDelay( 5 ) ;	// mS
    
	TOUCH_AF_INT_Change();  //Set INT INPUT INT=LOW

	I2C_Init() ;

//	touchRead( 0x8140, (uint8_t *)&TouchDebug[3], 4 ) ;
//	touchRead( 0x8047, (uint8_t *)&TouchDebug[5], 2 ) ;

	return 0 ;
}



uint32_t touchRead( uint16_t regAddr, uint8_t *destAddr, uint32_t count )
{
	uint8_t txBuffer[8] ;
	txBuffer[0] = regAddr >> 8 ;
	txBuffer[1] = regAddr ;
	i2cWrite( 0x28, txBuffer, 2 ) ;
	
//	I2C4ReadDebug[4] = count ;
	
	i2cRead( 0x28, destAddr, count ) ;
	return 0 ;
}

uint32_t touchWrite( uint16_t regAddr, uint8_t *sourceAddr, uint32_t count )
{
	uint8_t txBuffer[16] ;
	uint32_t i ;
	uint8_t *ptr ;

	if ( count > 14 )
	{
		return 1 ;
	}
	txBuffer[0] = regAddr >> 8 ;
	txBuffer[1] = regAddr ;
	ptr = &txBuffer[2] ;
	i = 0 ;
	while ( i < count )
	{
		*ptr++ = *sourceAddr++ ;
		i += 1 ;
	}
	i2cWrite( 0x28, txBuffer, count + 2 ) ;
	return 0 ;
}

uint32_t touchReadPoints()
{
	uint8_t status ;
	uint32_t x ;

#ifdef WDOG_REPORT
	RTC->BKP1R = 0x0500 ;
#endif

	touchRead( 0x814E, &status, 1 ) ;
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x0502 ;
#endif
	if ( status & 0x80 )
	{
		x = status & 0x0F ;
		if ( x > 4 )
		{
			x = 4 ;
		}
		if ( x )
		{
			touchRead( 0x814F, TouchData.data, x * 8 ) ;
		}
	}
	else
	{
		x = 0 ;
	}
//	if ( x )
//	{
		status = 0 ;
		touchWrite( 0x814E, &status, 1 ) ;
//	}
#ifdef WDOG_REPORT
	RTC->BKP1R = 0x0501 ;
#endif
	return x ;
}

uint32_t touchReadData()
{
	touchRead( 0x8047, TouchData.data, 8 ) ;
	return 1 ;
}



