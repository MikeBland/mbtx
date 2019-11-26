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

#include "../ersky9x.h"

#include "stm32f10x.h"
//#include "X9D/stm32f2xx_gpio.h"
//#include "X9D/stm32f2xx_rcc.h"
#include "stm103/hal.h"
//#include "X9D/aspi.h"

#include "timers.h"
#include "lcd.h"
#include "stm103/logicio103.h"
#include "myeeprom.h"
//#include "mixer.h"
#ifndef SIMU
#include "CoOS.h"
#endif
#include <stdlib.h>
#include <string.h>

//#define GPIO_PinSource_HAPTIC           GPIO_PinSource8

#define LCD_CONTRAST_OFFSET            20
#define RESET_WAIT_DELAY_MS            300 // Wait time after LCD reset before first command
//#define WAIT_FOR_DMA_END()             do { } while (lcd_busy)


//#define LCD_W	 128
#define IS_LCD_RESET_NEEDED()          true

//bool lcdInitFinished = false;
//void lcdInitFinish();

//extern uint8_t DisplayBuf[DISPLAY_W*DISPLAY_H/8] ;

//void delay_ms(uint32_t ms)
//{
//  while (ms--) {
//		hw_delay( 10000 ) ; // units of 0.1uS
//  }
//}

void hw_delay( uint16_t time ) ;
static void LCD_BL_Config( void ) ;


#define lcdWriteCommand(byte) AspiCmd(byte)

//void lcdWriteCommand(uint8_t byte)
//{
//  LCD_A0_LOW();
//  LCD_NCS_LOW();
//  while ((SPI3->SR & SPI_SR_TXE) == 0) {
//    // Wait
//  }
//  (void)SPI3->DR; // Clear receive
//  LCD_SPI->DR = byte;
//  while ((SPI3->SR & SPI_SR_RXNE) == 0) {
//    // Wait
//  }
//  LCD_NCS_HIGH();
//}

#define LCD_NCS_HIGH()        LCD_NCS_GPIO->BSRR = LCD_NCS_PIN
#define LCD_NCS_LOW()         LCD_NCS_GPIO->BRR = LCD_NCS_PIN

#define LCD_A0_HIGH()         LCD_A0_GPIO->BSRR = LCD_A0_PIN
#define LCD_A0_LOW()          LCD_A0_GPIO->BRR = LCD_A0_PIN

#define LCD_RST_HIGH()        LCD_RST_GPIO->BSRR = LCD_RST_PIN
#define LCD_RST_LOW()         LCD_RST_GPIO->BRR = LCD_RST_PIN

#define LCD_CLK_HIGH()		    (LCD_CLK_GPIO->BSRR = LCD_CLK_PIN)
#define LCD_CLK_LOW()		    	(LCD_CLK_GPIO->BRR = LCD_CLK_PIN)

#define LCD_MOSI_HIGH()		    (LCD_MOSI_GPIO->BSRR = LCD_MOSI_PIN)
#define LCD_MOSI_LOW()		    (LCD_MOSI_GPIO->BRR = LCD_MOSI_PIN)

#define __no_operation     __NOP

void AspiCmd(uint8_t Command_Byte)
{
  uint32_t i = 8 ;
	LCD_A0_LOW() ;

  LCD_CLK_HIGH() ;
  LCD_NCS_LOW() ;  
 
  while (i--) 
  { 
    LCD_CLK_LOW(); 
    if( Command_Byte & 0x80 )
		{
      LCD_MOSI_HIGH() ;
    }
		else
		{
			LCD_MOSI_LOW() ;
		}
		Command_Byte=Command_Byte<<1 ;
		LCD_CLK_HIGH() ;  
    __no_operation() ;
  } 
  LCD_NCS_HIGH() ;  
  LCD_A0_HIGH() ;
}


void AspiData(uint8_t Para_data)
{
  uint32_t i = 8 ;
  LCD_CLK_HIGH();
  LCD_A0_HIGH();
  LCD_NCS_LOW();
  while (i--) 
  {
    if( Para_data & 0x80 )
    {
      LCD_MOSI_HIGH() ;
    }
		else
		{
			LCD_MOSI_LOW() ;
		}
    Para_data <<= 1 ;
    LCD_CLK_LOW() ;
    __no_operation() ;
    LCD_CLK_HIGH() ;
    __no_operation() ;
  }
  LCD_NCS_HIGH() ;
}


static void Delay(volatile unsigned int ms)
{
  volatile u8 i;
  while(ms != 0)
  {
    for(i=0;i<250;i++) {}
    for(i=0;i<75;i++) {}
    ms--;
  }
}

void lcdHardwareInit()
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN ; 			// Enable portA clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPDEN ; 			// Enable portA clock

	configure_pins( LCD_RST_PIN, LCD_RST_PORT | PIN_OP2 | PIN_GP_PP | PIN_HIGH ) ;
	configure_pins( LCD_A0_PIN, LCD_A0_PORT | PIN_OP2 | PIN_GP_PP | PIN_LOW ) ;
	configure_pins( LCD_CLK_PIN, LCD_CLK_PORT | PIN_OP2 | PIN_GP_PP | PIN_HIGH ) ;
	configure_pins( LCD_MOSI_PIN, LCD_MOSI_PORT | PIN_OP2 | PIN_GP_PP | PIN_LOW ) ;
	configure_pins( LCD_NCS_PIN, LCD_NCS_PORT | PIN_OP2 | PIN_GP_PP | PIN_HIGH ) ;

//  // APB1 clock / 2 = 133nS per clock
//  LCD_SPI->CR1 = 0; // Clear any mode error
//  LCD_SPI->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPOL | SPI_CR1_CPHA;
//  LCD_SPI->CR2 = 0;
//  LCD_SPI->CR1 |= SPI_CR1_MSTR;	// Make sure in case SSM/SSI needed to be set first
//  LCD_SPI->CR1 |= SPI_CR1_SPE;
  
//  LCD_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
//  LCD_DMA->HIFCR = LCD_DMA_FLAGS; // Write ones to clear bits
//  LCD_DMA_Stream->CR =  DMA_SxCR_PL_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0;
//  LCD_DMA_Stream->PAR = (uint32_t)&LCD_SPI->DR;

//  LCD_DMA_Stream->NDTR = LCD_W;

//  LCD_DMA_Stream->FCR = 0x05; // DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;

//	NVIC_SetPriority( LCD_DMA_Stream_IRQn, 8 ) ; // Lower priority interrupt
//  NVIC_EnableIRQ(LCD_DMA_Stream_IRQn);

	LCD_RST_LOW() ;
	Delay( 5 ) ;
	LCD_RST_HIGH() ;
}

void lcdStart()
{
  lcdWriteCommand(0xe2); // (14) Soft reset
//#ifdef PCBT12
//	lcdWriteCommand(0xA0); // Set seg
//  lcdWriteCommand(0xC8); // Set com
//#else  
  lcdWriteCommand(0xa3); // Set seg
  lcdWriteCommand(0xA0); // Set com
//#endif
  lcdWriteCommand(0xC8); // Set booster
  lcdWriteCommand(0x40); // 5x
  lcdWriteCommand(0xa6); // Set bias=1/6
  
	lcdWriteCommand(0x2C); // Set internal rb/ra=5.0
	lcdWriteCommand(0x2E); // Set internal rb/ra=5.0
  lcdWriteCommand(0x2f); // All built-in power circuits on
  lcdWriteCommand(0x26); // Set Vop
  lcdWriteCommand(0xAF); // Set Vop
  lcdWriteCommand(0xA4); // Set Vop
  lcdWriteCommand(0x81); // Set contrast
  lcdWriteCommand(0x08); // Set Vop
}

volatile bool lcd_busy ;

#define LCD_DCLK_HIGH()		    (gpio->BSRR = clkPin)
#define LCD_DCLK_LOW()	    	(gpio->BRR = clkPin)

#define LCD_gMOSI_HIGH()	    (gpio->BSRR = mosiPin)
#define LCD_gMOSI_LOW()		    (gpio->BRR = mosiPin)

uint16_t RefreshTime ;


void refreshSendByte( GPIO_TypeDef *gpio, uint32_t data, uint32_t mosiPin, uint32_t clkPin )
{
  if( data & 0x80 )
  {
    LCD_gMOSI_HIGH() ;
  }
	else
	{
		LCD_gMOSI_LOW() ;
	}
  LCD_DCLK_LOW() ;
  LCD_DCLK_LOW() ;	// Acts as NOP
  LCD_DCLK_HIGH() ;
//    	__no_operation() ;
  if( data & 0x40 )
  {
    LCD_gMOSI_HIGH() ;
  }
	else
	{
		LCD_gMOSI_LOW() ;
	}
  LCD_DCLK_LOW() ;
  LCD_DCLK_LOW() ;	// Acts as NOP
  LCD_DCLK_HIGH() ;
//    	__no_operation() ;
  if( data & 0x20 )
  {
    LCD_gMOSI_HIGH() ;
  }
	else
	{
		LCD_gMOSI_LOW() ;
	}
  LCD_DCLK_LOW() ;
  LCD_DCLK_LOW() ;	// Acts as NOP
  LCD_DCLK_HIGH() ;
//    	__no_operation() ;
  if( data & 0x10 )
  {
    LCD_gMOSI_HIGH() ;
  }
	else
	{
		LCD_gMOSI_LOW() ;
	}
  LCD_DCLK_LOW() ;
  LCD_DCLK_LOW() ;	// Acts as NOP
  LCD_DCLK_HIGH() ;
//    	__no_operation() ;
  if( data & 0x08 )
  {
    LCD_gMOSI_HIGH() ;
  }
	else
	{
		LCD_gMOSI_LOW() ;
	}
  LCD_DCLK_LOW() ;
  LCD_DCLK_LOW() ;	// Acts as NOP
  LCD_DCLK_HIGH() ;
//    	__no_operation() ;
  if( data & 0x04 )
  {
    LCD_gMOSI_HIGH() ;
  }
	else
	{
		LCD_gMOSI_LOW() ;
	}
  LCD_DCLK_LOW() ;
  LCD_DCLK_LOW() ;	// Acts as NOP
  LCD_DCLK_HIGH() ;
//    	__no_operation() ;
  if( data & 0x02 )
  {
    LCD_gMOSI_HIGH() ;
  }
	else
	{
		LCD_gMOSI_LOW() ;
	}
  LCD_DCLK_LOW() ;
  LCD_DCLK_LOW() ;	// Acts as NOP
  LCD_DCLK_HIGH() ;
//    	__no_operation() ;
  if( data & 0x01 )
  {
    LCD_gMOSI_HIGH() ;
  }
	else
	{
		LCD_gMOSI_LOW() ;
	}
  LCD_DCLK_LOW() ;
  LCD_DCLK_LOW() ;	// Acts as NOP
  LCD_DCLK_HIGH() ;
//    	__no_operation() ;
	
}


void refreshDisplay()
{
	uint16_t t0 = TIM7->CNT ;
  uint8_t *p = DisplayBuf ;
	GPIO_TypeDef *gpio = LCD_CLK_GPIO ;

  for (uint8_t y=0; y < 8; y += 1 )
	{
	  lcdWriteCommand( 0x10 ) ; // Column addr 0
  	lcdWriteCommand( 0xB0 | y ) ; // Page addr y
	  lcdWriteCommand( 0x00 ) ;

    LCD_A0_HIGH() ;
	  LCD_CLK_HIGH() ;
    LCD_NCS_LOW() ;
		for (uint32_t x = 0 ; x < DISPLAY_W ; x += 1 )
		{    
			uint32_t data ;
			data = *p++ ;

			refreshSendByte( gpio, data, LCD_MOSI_PIN, LCD_CLK_PIN ) ;
			 
		}
	}
  LCD_NCS_HIGH() ;

	t0 = TIM7->CNT - t0 ;
	RefreshTime = t0 / 2 ;
	 
//  if (!lcdInitFinished) {
//    lcdInitFinish();
//  }
	
//#ifndef PCBXLITE
// #ifndef PCBT12
//  #ifndef PCBX9LITE
//	if ( g_model.BTfunction == BT_LCDDUMP )
//	{
//		uint16_t time = get_tmr10ms() ;
//  	if((uint16_t)( time-ExtDisplayTime) >= 20) //200mS
//		{
//			ExtDisplayTime = time ;
//  		memcpy(&ExtDisplayBuf[1], DisplayBuf, sizeof(DisplayBuf));
//			ExtDisplayBuf[0] = 0xAA ;
//			ExtDisplayBuf[sizeof(ExtDisplayBuf)-1] = 0x55 ;
//			ExtDisplaySend = 1 ;
//		}
//	}
//  #endif // X3
// #endif
//#endif

//#ifdef PCBX9LITE
//	if ( g_model.com2Function == COM2_FUNC_LCD )
//	{
//		uint16_t time = get_tmr10ms() ;
//  	if((uint16_t)( time-ExtDisplayTime) >= 20) //200mS
//		{
//			ExtDisplayTime = time ;
//  		memcpy(&ExtDisplayBuf[1], DisplayBuf, sizeof(DisplayBuf));
//			ExtDisplayBuf[0] = 0xAA ;
//			ExtDisplayBuf[sizeof(ExtDisplayBuf)-1] = 0x55 ;
//			ExtDisplaySend = 1 ;
//		}
//	}
//#endif

//  uint8_t * p = DisplayBuf;
//  for (uint8_t y=0; y < 8; y++, p+=LCD_W) {
//    lcdWriteCommand(0x10); // Column addr 0
//    lcdWriteCommand(0xB0 | y); // Page addr y
//#ifdef PCBT12
//    lcdWriteCommand(0x0);
//#else
//    lcdWriteCommand(0x04);
//#endif    
//    LCD_NCS_LOW();
//    LCD_A0_HIGH();

//    lcd_busy = true;
//    LCD_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
//    LCD_DMA->HIFCR = LCD_DMA_FLAGS; // Write ones to clear bits
//    LCD_DMA_Stream->M0AR = (uint32_t)p;
//    LCD_DMA_Stream->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE; // Enable DMA & TC interrupts
//    LCD_SPI->CR2 |= SPI_CR2_TXDMAEN;
  
//    WAIT_FOR_DMA_END();

//    LCD_NCS_HIGH();
//    LCD_A0_HIGH();
//  }
}

//extern "C" void LCD_DMA_Stream_IRQHandler()
//{
//  LCD_DMA_Stream->CR &= ~DMA_SxCR_TCIE; // Stop interrupt
//  LCD_DMA->HIFCR = LCD_DMA_FLAG_INT; // Clear interrupt flag
//  LCD_SPI->CR2 &= ~SPI_CR2_TXDMAEN;
//  LCD_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA

//  while (LCD_SPI->SR & SPI_SR_BSY) {
//    /* Wait for SPI to finish sending data
//    The DMA TX End interrupt comes two bytes before the end of SPI transmission,
//    therefore we have to wait here.
//    */
//  }
//  LCD_NCS_HIGH();
//  lcd_busy = false;
//}

/*
  Proper method for turning of LCD module. It must be used,
  otherwise we might damage LCD crystals in the long run!
*/
void lcdOff()
{
//  WAIT_FOR_DMA_END();

//  /*
//  LCD Sleep mode is also good for draining capacitors and enables us
//  to re-init LCD without any delay
//  */
//  lcdWriteCommand(0xAE); // LCD sleep
//  delay_ms(3); // Wait for caps to drain
}

void lcdReset()
{
//  LCD_RST_LOW();
//  delay_ms(150);
//  LCD_RST_HIGH();
}


//static void backlightInit()
//{
//#ifdef PCBXLITE
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;
//  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;    // Enable clock
// #ifdef XLITE_PROTO	
//	configure_pins( GPIO_Pin_10, PIN_PERIPHERAL | PIN_PER_1 | PIN_PORTA | PIN_PUSHPULL | PIN_OS2 | PIN_NO_PULLUP ) ;
//  TIM1->ARR = 100;
//  TIM1->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 10000 - 1 ;
//  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // PWM
//  TIM1->CCER = TIM_CCER_CC3E;
//  TIM1->CCR1 = 100;
//	TIM1->BDTR |= TIM_BDTR_MOE ;
//  TIM1->EGR = 0;
//  TIM1->CR1 = TIM_CR1_CEN; // Counter enable
// #else
//	configure_pins( BACKLIGHT_GPIO_PIN, PIN_PERIPHERAL | PIN_PER_1 | PIN_PORTA | PIN_PUSHPULL | PIN_OS2 | PIN_NO_PULLUP ) ;
//  TIM1->ARR = 100;
//  TIM1->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 10000 - 1 ;
//  TIM1->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM
//  TIM1->CCER = TIM_CCER_CC1E;
//  TIM1->CCR1 = 100;
//	TIM1->BDTR |= TIM_BDTR_MOE ;
//  TIM1->EGR = 0;
//  TIM1->CR1 = TIM_CR1_CEN; // Counter enable
// #endif
//#else	
// #ifdef PCBX9LITE
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;
//  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;    // Enable clock
//	configure_pins( BACKLIGHT_GPIO_PIN, PIN_PERIPHERAL | PIN_PER_1 | PIN_PORTA | PIN_PUSHPULL | PIN_OS2 | PIN_NO_PULLUP ) ;
//  TIM1->ARR = 100;
//  TIM1->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 10000 - 1 ;
//  TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // PWM
//  TIM1->CCER = TIM_CCER_CC3E;
//  TIM1->CCR1 = 100;
//	TIM1->BDTR |= TIM_BDTR_MOE ;
//  TIM1->EGR = 0;
//  TIM1->CR1 = TIM_CR1_CEN; // Counter enable
// #else // X3
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN ;
//  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN ;    // Enable clock
  
//	configure_pins( GPIO_Pin_13, PIN_PERIPHERAL | PIN_PER_2 | PIN_PORTD | PIN_PUSHPULL | PIN_OS2 | PIN_NO_PULLUP ) ;
	
//	TIM4->ARR = 100;
//  TIM4->PSC = (PeripheralSpeeds.Peri2_frequency*PeripheralSpeeds.Timer_mult2) / 10000 - 1 ;
//  TIM4->CCMR1 = TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM
//  TIM4->CCER = TIM_CCER_CC2E;
//  TIM4->CCR2 = 100;
//  TIM4->EGR = 0;
//  TIM4->CR1 = TIM_CR1_CEN; // Counter enable
// #endif // X3
//#endif
//}


/*
  Starts LCD initialization routine. It should be called as
  soon as possible after the reset because LCD takes a lot of
  time to properly power-on.

  Make sure that delay_ms() is functional before calling this function!
*/
void lcdInit()
{
	LCD_BL_Config() ;
	lcdHardwareInit() ;

  lcdStart() ;

//  if (IS_LCD_RESET_NEEDED()) {
//    lcdReset();
//  }
//	backlightInit() ;
//	backlight_set( 50 ) ;
}

/*
  Finishes LCD initialization. It is called auto-magically when first LCD command is
  issued by the other parts of the code.
*/
void lcdInitFinish()
{
//  lcdInitFinished = true;

  /*
    LCD needs longer time to initialize in low temperatures. The data-sheet
    mentions a time of at least 150 ms. The delay of 1300 ms was obtained
    experimentally. It was tested down to -10 deg Celsius.

    The longer initialization time seems to only be needed for regular Taranis,
    the Taranis Plus (9XE) has been tested to work without any problems at -18 deg Celsius.
    Therefore the delay for T+ is lower.
    
    If radio is reset by watchdog or boot-loader the wait is skipped, but the LCD
    is initialized in any case.

    This initialization is needed in case the user moved power switch to OFF and
    then immediately to ON position, because lcdOff() was called. In any case the LCD
    initialization (without reset) is also recommended by the data sheet.
  */

//  if (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
//#if !defined(BOOT)
//    while (g_tmr10ms < (RESET_WAIT_DELAY_MS/10)); // wait measured from the power-on
//#else
//    delay_ms(RESET_WAIT_DELAY_MS);
//#endif
//  }

	  
  lcdStart() ;
//  lcdWriteCommand(0xAF); // dc2=1, IC into exit SLEEP MODE, dc3=1 gray=ON, dc4=1 Green Enhanc mode disabled
//  delay_ms(20); // needed for internal DC-DC converter startup
}

//#ifndef PCBXLITE

//#ifdef PCBX9LITE
//void initHaptic()
//{
//  RCC_AHB1PeriphClockCmd(HAPTIC_RCC_AHB1Periph, ENABLE);
//  GPIO_InitTypeDef GPIO_InitStructure;
//  GPIO_InitStructure.GPIO_Pin = HAPTIC_GPIO_PIN ;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
//  GPIO_Init( HAPTIC_GPIO, &GPIO_InitStructure);

//  GPIO_PinAFConfig( HAPTIC_GPIO, HAPTIC_GPIO_PinSource, HAPTIC_GPIO_AF);

//	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN ;		// Enable clock
//	HAPTIC_TIMER->ARR = 100 ;
//	HAPTIC_TIMER->PSC = (PeripheralSpeeds.Peri2_frequency*PeripheralSpeeds.Timer_mult2) / 10000 - 1 ;		// 100uS from 30MHz
//	HAPTIC_TIMER->CCMR1 = TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 ;	// PWM
//	HAPTIC_TIMER->CCER = TIM_CCER_CC2E ;
	
//	HAPTIC_TIMER->HAPTIC_COUNTER_REGISTER = 0 ;
//	HAPTIC_TIMER->EGR = 0 ;
//	HAPTIC_TIMER->CR1 = TIM_CR1_CEN ;				// Counter enable
//}

//void hapticOff()
//{
//	HAPTIC_TIMER->HAPTIC_COUNTER_REGISTER = 0 ;
//}

//// pwmPercent 0-100
//void hapticOn( uint32_t pwmPercent )
//{
//	if ( pwmPercent > 100 )
//	{
//		pwmPercent = 100 ;		
//	}
//	HAPTIC_TIMER->HAPTIC_COUNTER_REGISTER = pwmPercent ;
//}

//#else // X3
//void initHaptic()
//{
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOHAPTIC, ENABLE);
//  GPIO_InitTypeDef GPIO_InitStructure;
//  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_HAPTIC;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
//  GPIO_Init(GPIOHAPTIC, &GPIO_InitStructure);

//  GPIO_PinAFConfig(GPIOHAPTIC, GPIO_PinSource_HAPTIC ,GPIO_AF_TIM10);

//	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN ;		// Enable clock
//	TIM10->ARR = 100 ;
//	TIM10->PSC = (PeripheralSpeeds.Peri2_frequency*PeripheralSpeeds.Timer_mult2) / 10000 - 1 ;		// 100uS from 30MHz
//	TIM10->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 ;	// PWM
//	TIM10->CCER = TIM_CCER_CC1E ;
	
//	TIM10->CCR1 = 0 ;
//	TIM10->EGR = 0 ;
//	TIM10->CR1 = TIM_CR1_CEN ;				// Counter enable
//}

//void hapticOff()
//{
//	TIM10->CCR1 = 0 ;
//}

//// pwmPercent 0-100
//void hapticOn( uint32_t pwmPercent )
//{
//	if ( pwmPercent > 100 )
//	{
//		pwmPercent = 100 ;		
//	}
//#ifndef PCBXLITE

//#ifdef PCBX9LITE
//void initHaptic()
//{
//  RCC_AHB1PeriphClockCmd(HAPTIC_RCC_AHB1Periph, ENABLE);
//  GPIO_InitTypeDef GPIO_InitStructure;
//  GPIO_InitStructure.GPIO_Pin = HAPTIC_GPIO_PIN ;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
//  GPIO_Init( HAPTIC_GPIO, &GPIO_InitStructure);

//  GPIO_PinAFConfig( HAPTIC_GPIO, HAPTIC_GPIO_PinSource, HAPTIC_GPIO_AF);

//	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN ;		// Enable clock
//	HAPTIC_TIMER->ARR = 100 ;
//	HAPTIC_TIMER->PSC = (PeripheralSpeeds.Peri2_frequency*PeripheralSpeeds.Timer_mult2) / 10000 - 1 ;		// 100uS from 30MHz
//	HAPTIC_TIMER->CCMR1 = TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 ;	// PWM
//	HAPTIC_TIMER->CCER = TIM_CCER_CC2E ;
	
//	HAPTIC_TIMER->HAPTIC_COUNTER_REGISTER = 0 ;
//	HAPTIC_TIMER->EGR = 0 ;
//	HAPTIC_TIMER->CR1 = TIM_CR1_CEN ;				// Counter enable
//}

//void hapticOff()
//{
//	HAPTIC_TIMER->HAPTIC_COUNTER_REGISTER = 0 ;
//}

//// pwmPercent 0-100
//void hapticOn( uint32_t pwmPercent )
//{
//	if ( pwmPercent > 100 )
//	{
//		pwmPercent = 100 ;		
//	}
//	HAPTIC_TIMER->HAPTIC_COUNTER_REGISTER = pwmPercent ;
//}

//#else // X3
//void initHaptic()
//{
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOHAPTIC, ENABLE);
//  GPIO_InitTypeDef GPIO_InitStructure;
//  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_HAPTIC;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
//  GPIO_Init(GPIOHAPTIC, &GPIO_InitStructure);

//  GPIO_PinAFConfig(GPIOHAPTIC, GPIO_PinSource_HAPTIC ,GPIO_AF_TIM10);

//	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN ;		// Enable clock
//	TIM10->ARR = 100 ;
//	TIM10->PSC = (PeripheralSpeeds.Peri2_frequency*PeripheralSpeeds.Timer_mult2) / 10000 - 1 ;		// 100uS from 30MHz
//	TIM10->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 ;	// PWM
//	TIM10->CCER = TIM_CCER_CC1E ;
	
//	TIM10->CCR1 = 0 ;
//	TIM10->EGR = 0 ;
//	TIM10->CR1 = TIM_CR1_CEN ;				// Counter enable
//}

//void hapticOff()
//{
//	TIM10->CCR1 = 0 ;
//}

//// pwmPercent 0-100
//void hapticOn( uint32_t pwmPercent )
//{
//	if ( pwmPercent > 100 )
//	{
//		pwmPercent = 100 ;		
//	}
//	TIM10->CCR1 = pwmPercent ;
//}
//#endif // X3
//#else // XLITE

//void initHaptic()
//{
//}
//void hapticOff()
//{
//}

//void hapticOn( uint32_t pwmPercent )
//{
//}
//#endif // PCBXLITE
//	TIM10->CCR1 = pwmPercent ;
//}
//#endif // X3
//#else // XLITE

//void initHaptic()
//{
//}
//void hapticOff()
//{
//}

//void hapticOn( uint32_t pwmPercent )
//{
//}
//#endif // PCBXLITE

uint16_t BacklightBrightness ;

void backlight_on()
{
	TIM3->CCR4 = 100 - BacklightBrightness ;
//#ifdef XLITE_PROTO	
//	TIM1->CCR3 = 100 - BacklightBrightness ;
//#else	
//	TIM1->CCR1 = 100 - BacklightBrightness ;
//#endif
}

void backlight_off()
{
	TIM3->CCR4 = 0 ;
//#ifdef XLITE_PROTO	
//	TIM1->CCR3 = 0 ;
//#else	
//	TIM1->CCR1 = 0 ;
//#endif
}

void backlight_set( uint16_t brightness )
{
	BacklightBrightness = brightness ;
	TIM3->CCR4 = 100 - BacklightBrightness ;
//#ifdef XLITE_PROTO	
//	TIM1->CCR3 = 100 - BacklightBrightness ;
//#else	
//	TIM1->CCR1 = 100 - BacklightBrightness ;
//#endif
}

//uint8_t speaker[] = {
//4,8,0,
//0x38,0x38,0x7C,0xFE
//} ;

/**Init the Backlight GPIO */
static void LCD_BL_Config()
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN ; 			// Enable portCclock
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN ;			// Enable clock
	
	configure_pins( BACKLIGHT_Pin, BACKLIGHT_PORT | PIN_OP2 | PIN_GP_PP ) ;

//	configure_pins( GPIO_Pin_BL, PIN_PERIPHERAL | PIN_PER_3 | PIN_PORTB | PIN_PUSHPULL | PIN_OS2 | PIN_NO_PULLUP ) ;
	TIM3->CR1 = 0 ;
	TIM3->CCER = 0 ;
	TIM3->ARR = 100 ;
	TIM3->PSC = 72000000 / 10000 - 1 ;		// 100uS from 30MHz
	TIM3->CCMR2 = 0x6000 ;	// PWM

	BacklightBrightness = 80 ;
	TIM3->CCR4 = BacklightBrightness ;
	TIM3->EGR = 0 ;
	TIM3->CR1 = 1 ;
	TIM3->SR = TIMER2_5SR_MASK & (~TIM_SR_UIF & ~TIM_SR_CC4IF ) ;				// Clear flag
	TIM3->DIER = TIM_DIER_UIE | TIM_DIER_CC4IE ;
	NVIC_SetPriority( TIM3_IRQn, 5 ) ; // low
  NVIC_EnableIRQ(TIM3_IRQn) ;
}

extern "C" void TIM3_IRQHandler()
{
	uint32_t status = TIM3->SR ;
	TIM3->SR = TIMER2_5SR_MASK & (~TIM_SR_UIF & ~TIM_SR_CC4IF ) ;				// Clear flag
	
	GPIOC->BSRR = ( ( status & TIM_SR_UIF ) && (TIM3->CCR4 != 0) ) ? BACKLIGHT_Pin : BACKLIGHT_Pin << 16 ;
	
//	if ( ( status & TIM_SR_UIF ) && (TIM3->CCR4 != 0) )
//	{
//		GPIOC->BSRR = BACKLIGHT_Pin ;
//	}
//	else
//	{
//		GPIOC->BRR = BACKLIGHT_Pin ;
//	}
}

/** Init the anolog spi gpio
*/
//static void LCD_Hardware_Init()
//{
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_LCD, ENABLE);
//#if defined(REVPLUS) || defined(REV9E)
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_LCD_RST, ENABLE);
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_LCD_NCS, ENABLE);
//#endif  
//  GPIO_InitTypeDef GPIO_InitStructure;
  
//  /*!< Configure lcd CLK\ MOSI\ A0pin in output pushpull mode *************/
//  GPIO_InitStructure.GPIO_Pin =PIN_LCD_MOSI | PIN_LCD_CLK | PIN_LCD_A0;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_Init(GPIO_LCD, &GPIO_InitStructure);
  
//  /*!< Configure lcd NCS pin in output pushpull mode ,PULLUP *************/
//#if defined(REVPLUS) || defined(REV9E)
//	GPIO_InitStructure.GPIO_Pin = PIN_LCD_NCS ;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
//  GPIO_Init(GPIO_LCD_NCS, &GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_Pin = PIN_LCD_RST ;
//  GPIO_Init(GPIO_LCD_RST, &GPIO_InitStructure);

//#else  
//	GPIO_InitStructure.GPIO_Pin = PIN_LCD_NCS | PIN_LCD_RST ;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  
//  GPIO_Init(GPIO_LCD, &GPIO_InitStructure);
//#endif  
//}

//static void LCD_Init()
//{
//  LCD_BL_Config() ;
    
//  AspiCmd(0x25);   //Temperature compensation curve definition: 0x25 = -0.05%/oC
//  AspiCmd(0x2b);   //Panel loading set ,Internal VLCD.
//  AspiCmd(0xEA);	//set bias=1/10 :Command table NO.27
//  AspiCmd(0x81);	//Set Vop
//  AspiCmd(25+CONTRAST_OFS);		//0--255
//  AspiCmd(0xA6);	//inverse display off
//  AspiCmd(0xD1);	//SET RGB:Command table NO.21 .SET RGB or BGR.  D1=RGB
//  AspiCmd(0xD5);	//set color mode 4K and 12bits  :Command table NO.22
//  AspiCmd(0xA0);	//line rates,25.2 Klps
//  AspiCmd(0xC8);	//SET N-LINE INVERSION
//  AspiCmd(0x1D);	//Disable NIV
//  AspiCmd(0xF1);	//Set CEN
//  AspiCmd(0x3F);	// 1/64DUTY
//  AspiCmd(0x84);	//Disable Partial Display
//  AspiCmd(0xC4);	//MY=1,MX=0
//  AspiCmd(0x89);	//WA=1,column (CA) increment (+1) first until CA reaches CA boundary, then RA will increment by (+1).

//  AspiCmd(0xF8);	//Set Window Program Enable  ,inside modle
//  AspiCmd(0xF4);   //starting column address of RAM program window.
//  AspiCmd(0x00);
//  AspiCmd(0xF5);   //starting row address of RAM program window.
//  AspiCmd(0x60);
//  AspiCmd(0xF6);   //ending column address of RAM program window.
//  AspiCmd(0x47);
//  AspiCmd(0xF7);   //ending row address of RAM program window.
//  AspiCmd(0x9F);

//  AspiCmd(0xAF);	//dc2=1,IC into exit SLEEP MODE,	 dc3=1  gray=ON 开灰阶	,dc4=1  Green Enhanc mode disabled	  绿色增强模式关

//}

//void lcdInit()
//{
//	GPIO_TypeDef *gpiod = GPIOD ;

//  LCD_BL_Config();
//  LCD_Hardware_Init();
//#if defined(REVPLUS) || defined(REV9E)
//	initLcdSpi() ;
//#endif
  
//	gpiod->BSRRL = PIN_LCD_RST ;		// RST high
//  Delay(5);

//  gpiod->BSRRH = PIN_LCD_RST ;		// RST low
//  Delay(120); //11ms

//	gpiod->BSRRL = PIN_LCD_RST ;		// RST high
//  Delay(2500);
 
//  AspiCmd(0xE2);			// System Reset

//  Delay(2500);

//  LCD_Init();
//  Delay(120);
//  AspiCmd(0xAF);	//dc2=1, IC into exit SLEEP MODE, dc3=1 gray=ON, dc4=1 Green Enhanc mode disabled

//}

void lcdSetRefVolt(uint8_t val)
{
  lcdWriteCommand(0x81); // Set contrast
  lcdWriteCommand(val); // Set Vop
//	AspiCmd(0x81);	//Set Vop
//  AspiCmd(val+CONTRAST_OFS);		//0--255

}

