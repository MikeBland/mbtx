/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
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
 */

#include <stdint.h>
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "hal.h"
#include "../ersky9x.h"
#include "../logicio.h"

//extern void configure_pins( uint32_t pins, uint16_t config ) ;

#define REV9		1

#ifdef REV9

#ifndef PCBX10
#define ADC_CS_HIGH()						(ADC_SPI_GPIO->BSRRL = ADC_SPI_PIN_CS)
#define ADC_CS_LOW()						(ADC_SPI_GPIO->BSRRH = ADC_SPI_PIN_CS)
#endif

extern void hw_delay( uint16_t time ) ;
#define delay_01us(x) hw_delay( x )

#define  RESETCMD						0x4000

#define  MANUAL_MODE					0x1000//  manual mode    channel 0

//#define  AutoMode_1						0x2C00
//#define  AutoMode_2						0x3C00                     ////////
//#define ProgramReg_Auto1FRAME1   		0x8000
//#define ProgramReg_Auto1FRAME2   		0x0FFF
//#define ProgramReg_Auto2				0x92C0   //CH00---CH11
//#define ContinuedSelectMode				0x00  //ignores data on DI11--DI00

#define BUFFERSIZE						12

#define SAMPTIME	4		// sample time = 28 cycles

#define V_BATT		18


#ifdef PCBX10
//extern void delay_ms( uint32_t ms ) ;
//volatile uint16_t PwmInterruptCount ;
uint8_t SticksPwmDisabled = 0 ;

struct t_PWMcontrol
{
volatile uint32_t timer_capture_rising_time ;
volatile uint32_t timer_capture_value ;
volatile uint32_t timer_capture_period ;
} PWMcontrol[4] ;

//static void initSticksPwm()
//{
//	RCC->APB1ENR |= RCC_APB1ENR_TIM5EN ;		// Enable clock
	
//  configure_pins( (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3), PIN_PERIPHERAL | PIN_PORTA | PIN_PER_2 | PIN_NO_PULLUP ) ;

//  TIM5->CR1 &= ~TIM_CR1_CEN ; // Stop timer
//  TIM5->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ;		// 0.5uS
//  TIM5->ARR = 0xffffffff ;
//  TIM5->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0 ;
//  TIM5->CCMR2 = TIM_CCMR2_CC3S_0 | TIM_CCMR2_CC4S_0 ;
//  TIM5->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E ;
//  TIM5->DIER |= TIM_DIER_CC1IE|TIM_DIER_CC2IE|TIM_DIER_CC3IE|TIM_DIER_CC4IE ;
//  TIM5->EGR = 1; // Restart
//  TIM5->CR1 = TIM_CR1_CEN ; // Start timer

//  NVIC_SetPriority(TIM5_IRQn, 1 ) ;
//  NVIC_EnableIRQ(TIM5_IRQn) ;
//}

//static void disableSticksPwm()
//{
//  NVIC_DisableIRQ(TIM5_IRQn) ;
//  TIM5->CR1 &= ~TIM_CR1_CEN ; // Stop timer
//}

//extern "C" void TIM5_IRQHandler(void)
//{
//  uint32_t capture ;
//	uint32_t value ;
//	struct t_PWMcontrol *p ;

//	PwmInterruptCount += 1 ; // overflow may happen but we only use this to detect PWM / ADC on radio startup

//  if ( PWM_TIMER->SR & TIM_DIER_CC1IE )
//	{
//		capture = TIM5->CCR1 ;
//		p = &PWMcontrol[0] ;
//    if (TIM5->CCER & TIM_CCER_CC1P)
//		{
//	    value = capture - p->timer_capture_rising_time ;
//			p->timer_capture_value = value ;
//  		TIM5->CCER &= ~TIM_CCER_CC1P ; // Rising
//		}
//		else
//		{
//			p->timer_capture_period = capture - p->timer_capture_rising_time ;
//			p->timer_capture_rising_time = capture ;
//  		TIM5->CCER |= TIM_CCER_CC1P ; // Falling
//		}
//	}
//  if ( PWM_TIMER->SR & TIM_DIER_CC2IE )
//	{
//		capture = TIM5->CCR2 ;
//		p = &PWMcontrol[1] ;
//    if (TIM5->CCER & TIM_CCER_CC2P)
//		{
//	    value = capture - p->timer_capture_rising_time ;
//			p->timer_capture_value = value ;
//  		TIM5->CCER &= ~TIM_CCER_CC2P ; // Rising
//		}
//		else
//		{
//			p->timer_capture_period = capture - p->timer_capture_rising_time ;
//			p->timer_capture_rising_time = capture ;
//  		TIM5->CCER |= TIM_CCER_CC2P ; // Falling
//		}
//	}
//  if ( PWM_TIMER->SR & TIM_DIER_CC3IE )
//	{
//		capture = TIM5->CCR3 ;
//		p = &PWMcontrol[2] ;
//    if (TIM5->CCER & TIM_CCER_CC3P)
//		{
//	    value = capture - p->timer_capture_rising_time ;
//			p->timer_capture_value = value ;
//  		TIM5->CCER &= ~TIM_CCER_CC3P ; // Rising
//		}
//		else
//		{
//			p->timer_capture_period = capture - p->timer_capture_rising_time ;
//			p->timer_capture_rising_time = capture ;
//  		TIM5->CCER |= TIM_CCER_CC3P ; // Falling
//		}
//	}
//  if ( PWM_TIMER->SR & TIM_DIER_CC4IE )
//	{
//		capture = TIM5->CCR4 ;
//		p = &PWMcontrol[3] ;
//    if (TIM5->CCER & TIM_CCER_CC4P)
//		{
//	    value = capture - p->timer_capture_rising_time ;
//			p->timer_capture_value = value ;
//  		TIM5->CCER &= ~TIM_CCER_CC4P ; // Rising
//		}
//		else
//		{
//			p->timer_capture_period = capture - p->timer_capture_rising_time ;
//			p->timer_capture_rising_time = capture ;
//  		TIM5->CCER |= TIM_CCER_CC4P ; // Falling
//		}
//	}
//}
#endif


static void enableRtcBattery()
{
	ADC->CCR |= ADC_CCR_VBATE ;
}

void disableRtcBattery()
{
	ADC->CCR &= ~ADC_CCR_VBATE ;
}

uint16_t AdcBuffer[BUFFERSIZE+3] ;		// 2 for the on chip ADC + 1 for V_BATT
uint16_t VbattRtc ;

#ifndef PCBX10
static u16 SPIx_ReadWriteByte(uint16_t value)
{
    while(SPI_I2S_GetFlagStatus(ADC_SPI,SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(ADC_SPI,value);

    while(SPI_I2S_GetFlagStatus(ADC_SPI,SPI_I2S_FLAG_RXNE) ==RESET);
    return SPI_I2S_ReceiveData(ADC_SPI);
}

static void ADS7952_Init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	RCC_AHB1PeriphClockCmd(ADC_RCC_AHB1Periph,ENABLE);
	RCC_APB2PeriphClockCmd(ADC_RCC_APB2Periph,ENABLE);

	GPIO_InitStructure.GPIO_Pin = ADC_SPI_PIN_MISO|ADC_SPI_PIN_SCK|ADC_SPI_PIN_MOSI;
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(ADC_SPI_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = ADC_SPI_PIN_CS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(ADC_SPI_GPIO, &GPIO_InitStructure);

	GPIO_PinAFConfig(ADC_SPI_GPIO, ADC_SPI_PinSource_SCK, ADC_GPIO_AF);
	GPIO_PinAFConfig(ADC_SPI_GPIO, ADC_SPI_PinSource_MISO, ADC_GPIO_AF);
	GPIO_PinAFConfig(ADC_SPI_GPIO, ADC_SPI_PinSource_MOSI, ADC_GPIO_AF);

	SPI_I2S_DeInit(ADC_SPI);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(ADC_SPI,&SPI_InitStructure);
	SPI_Cmd(ADC_SPI,ENABLE);
	SPI_I2S_ITConfig(ADC_SPI,SPI_I2S_IT_TXE,DISABLE);
	SPI_I2S_ITConfig(ADC_SPI,SPI_I2S_IT_RXNE,DISABLE);

  ADC_CS_HIGH();
	delay_01us(1) ;
  ADC_CS_LOW() ;
  SPIx_ReadWriteByte(RESETCMD);
  ADC_CS_HIGH();
	delay_01us(1) ;
  ADC_CS_LOW();
  SPIx_ReadWriteByte(MANUAL_MODE);
  ADC_CS_HIGH();
    
//		SPIx_ReadWriteByte(ProgramReg_Auto2 );
//    ADC_CS_HIGH();

//    asm("nop");
//    ADC_CS_LOW();
//    SPIx_ReadWriteByte(AutoMode_2);
//    ADC_CS_HIGH();

//    asm("nop");
//    ADC_CS_LOW();
//    SPIx_ReadWriteByte(ContinuedSelectMode);
//    ADC_CS_HIGH();
//	asm("nop");
}
#endif

void init_adc()
{
#ifndef PCBX10
  ADS7952_Init() ;
#endif

	RCC->APB2ENR |= RCC_APB2ENR_ADC3EN ;			// Enable clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN ;			// Enable clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;		// Enable DMA2 clock
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

#ifdef PCBX10
//	initSticksPwm() ;
//  delay_ms(20) ;
//  if (PwmInterruptCount < 8 )
//	{
    SticksPwmDisabled = true ;
//		disableSticksPwm() ;
//  }
#endif

#ifdef PCBX12D
  configure_pins( PIN_STICK_J5 | PIN_STICK_J6, PIN_ANALOG | PIN_PORTF ) ;
#endif

#ifdef PCBX10
  if ( SticksPwmDisabled )
	{
	  configure_pins( (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3), PIN_ANALOG | PIN_PORTA ) ;
	}
  configure_pins( ADC_GPIOC_PINS, PIN_ANALOG | PIN_PORTC ) ;
  configure_pins( ADC_GPIOF_PINS | ADC_GPIOF_EXTRA_PINS , PIN_ANALOG | PIN_PORTF ) ;
#endif
	 
	ADC3->CR1 = ADC_CR1_SCAN ;
	ADC3->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS ;
#ifdef PCBX12D
	ADC3->SQR1 = (2-1) << 20 ;		// NUMBER_ANALOG Channels
	ADC3->SQR3 = STICK_J5 + (STICK_J6<<5) ;
#endif
	 
#ifdef PCBX10
  if ( SticksPwmDisabled )
	{
		ADC3->SQR1 = (12-1) << 20 ;		// NUMBER_ANALOG Channels
		ADC3->SQR2 = SLIDER_L + (SLIDER_R<<5) + (FLAP2<<10) + (BATTERY_V<<15) + (EXT1<<20) + (EXT2<<25) ;
		ADC3->SQR3 = STICK_LH + (STICK_LV<<5) + (STICK_RV<<10) + (STICK_RH<<15) + (FLAP1<<20) + (FLAP3<<25) ;
	}
	else
	{
		ADC3->SQR1 = (10-1-4) << 20 ;		// NUMBER_ANALOG Channels
		ADC3->SQR3 = FLAP1 + (FLAP3<<5) + (SLIDER_L<<10) + (SLIDER_R<<15) + (FLAP2<<20) + (BATTERY_V<<25) ;
	}
#endif	
	
	ADC3->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12)
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) ;
	ADC3->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) 
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27) ;
	ADC->CCR = 0 ; //ADC_CCR_ADCPRE_0 ;		// Clock div 2
	
	ADC1->CR1 = ADC_CR1_SCAN ;
	ADC1->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS ;
	ADC1->SQR1 = 0 << 20 ;		// NUMBER_ANALOG Channels
	ADC1->SQR3 = V_BATT ;
	ADC1->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12)
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) ;
	ADC1->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) 
								+ (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27) ;
	ADC->CCR = 0 ; //ADC_CCR_ADCPRE_0 ;		// Clock div 2

  // Enable the DMA channel here, DMA2 stream 1, channel 2
	DMA2_Stream1->CR = DMA_SxCR_PL | DMA_SxCR_CHSEL_1 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC ;
	DMA2_Stream1->PAR = (uint32_t) &ADC3->DR ;
	DMA2_Stream1->M0AR = (uint32_t) &AdcBuffer[BUFFERSIZE] ;
	DMA2_Stream1->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0 ;
	
	enableRtcBattery() ;
}

#ifndef PCBX10
const uint16_t adcCommands[14] =
{
	MANUAL_MODE | ( STICK_J1	<< 7 ),
	MANUAL_MODE | ( STICK_J2	<< 7 ),
	MANUAL_MODE | ( STICK_J3	<< 7 ),
	MANUAL_MODE | ( STICK_J4	<< 7 ),
	MANUAL_MODE | ( FLAP1			<< 7 ),
	MANUAL_MODE | ( FLAP3			<< 7 ),
	MANUAL_MODE | ( SLIDER_L	<< 7 ),
	MANUAL_MODE | ( SLIDER_R	<< 7 ),
	MANUAL_MODE | ( SLIDER_MR << 7 ),
	MANUAL_MODE | ( BATTERY_V	<< 7 ),
	MANUAL_MODE | ( SLIDER_ML << 7 ),
	MANUAL_MODE | ( FLAP2			<< 7 ),
	MANUAL_MODE | ( 0 << 7 ),
	MANUAL_MODE | ( 0 << 7 )
} ;
#endif

void read_adc()
{
	static uint16_t SixPosTimer ;
#ifdef PCBX12D
	uint32_t adcIndex = 0;
#endif
//	uint16_t command = MANUAL_MODE ;	// Channel 0
	uint32_t i ;
#ifdef PCBX12D
	uint16_t *pCommands = (uint16_t *)adcCommands ;
#endif

	// Start on chip ADC read
	DMA2_Stream1->CR &= ~DMA_SxCR_EN ;		// Disable DMA
	ADC3->SR &= ~(uint32_t) ( ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR ) ;
	ADC1->SR &= ~(uint32_t) ( ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR ) ;
	DMA2->LIFCR = DMA_LIFCR_CTCIF1 | DMA_LIFCR_CHTIF1 |DMA_LIFCR_CTEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CFEIF1 ; // Write ones to clear bits
#ifdef PCBX10
	DMA2_Stream1->M0AR = (uint32_t) &AdcBuffer[0] ;
#else
	DMA2_Stream1->M0AR = (uint32_t) &AdcBuffer[BUFFERSIZE] ;
#endif
#ifdef PCBX12D
	DMA2_Stream1->NDTR = 2 ;
#endif
#ifdef PCBX10
	if ( SticksPwmDisabled )
	{
		DMA2_Stream1->NDTR = 12 ;
	}
	else
	{
		DMA2_Stream1->NDTR = 6 ;
	}
#endif
	DMA2_Stream1->CR |= DMA_SxCR_EN ;		// Enable DMA
	ADC3->CR2 |= (uint32_t)ADC_CR2_SWSTART ;
	ADC1->CR2 |= (uint32_t)ADC_CR2_SWSTART ;
	 
#ifdef PCBX12D
  ADC_CS_LOW() ;
	delay_01us(1) ;
  i = 0x0fff & SPIx_ReadWriteByte(*pCommands++) ;	// Discard
  ADC_CS_HIGH();
	delay_01us(1) ;
  
	ADC_CS_LOW() ;
	delay_01us(1) ;
  i = 0x0fff & SPIx_ReadWriteByte(*pCommands++) ;	// Discard
  ADC_CS_HIGH();
	delay_01us(1) ;
	
	for( adcIndex = 0 ; adcIndex<12 ; adcIndex++ )
	{
	  ADC_CS_LOW() ;
		delay_01us(1) ;
	  AdcBuffer[adcIndex] = 0x0fff & SPIx_ReadWriteByte(*pCommands++) ;	// Discard
	  ADC_CS_HIGH();
		delay_01us(1) ;
	}
#endif

#ifdef PCBX10
	for ( i = 0 ; i < 22000 ; i += 1 )
#else
	for ( i = 0 ; i < 20000 ; i += 1 )
#endif
	{
		if ( DMA2->LISR & DMA_LISR_TCIF1 )
		{
			break ;
		}
	}
	// On chip ADC read should have finished

	if ( 1 )
//  if (GPIOI->IDR & 0x0800)
	{
#ifdef PCBX10
	 if ( SticksPwmDisabled )
	 {
		AnalogData[1] = 4095 - AdcBuffer[0] ;
		if ( isProdVersion() )
		{
			AnalogData[0] = AdcBuffer[1] ;
		}
		else
		{
			AnalogData[0] = AdcBuffer[1] ;
		}
		AnalogData[3] = 4095 - AdcBuffer[2] ;
		AnalogData[2] = AdcBuffer[3] ;
#else
		AnalogData[0] = AdcBuffer[0] ;
		if ( isProdVersion() )
		{
			AnalogData[1] = 4095 - AdcBuffer[1] ;
		}
		else
		{
			AnalogData[1] = 4095 - AdcBuffer[1] ;
		}
		AnalogData[2] = AdcBuffer[2] ;
		AnalogData[3] = 4095 - AdcBuffer[3] ;
#endif

#ifdef PCBX10
	 }
	 else
	 {
		struct t_PWMcontrol *p ;
		p = &PWMcontrol[0] ;

		uint16_t value ;
		value = 0x0800 ;
		if ( p->timer_capture_period )
		{
			value = p->timer_capture_value * 4095 / p->timer_capture_period ;
		}
		if ( value < 4096 )
		{
			AnalogData[0] = value ;
		}
		p += 1 ;
		value = 0x0800 ;
		if ( p->timer_capture_period )
		{
			value = p->timer_capture_value * 4095 / p->timer_capture_period ;
		}
		if ( value < 4096 )
		{
			AnalogData[1] = 4095 - value ;
		}
		p += 1 ;
		value = 0x0800 ;
		if ( p->timer_capture_period )
		{
			value = p->timer_capture_value * 4095 / p->timer_capture_period ;
		}
		if ( value < 4096 )
		{
			AnalogData[3] = value ;
		}
		p += 1 ;
		value = 0x0800 ;
		if ( p->timer_capture_period )
		{
			value = p->timer_capture_value * 4095 / p->timer_capture_period ;
		}
		if ( value < 4096 )
		{
			AnalogData[2] = 4095 - value ;
		}
	 	
	 }
#endif		
	}
//	else
//	{
//		uint32_t t ;
//		AnalogData[0] = AdcBuffer[0] ;
//		AnalogData[1] = AdcBuffer[1] ;
//		t = AdcBuffer[12] ;
//		if ( t > 0x0800 )
//		{
//			if ( t < 0x0C00 )
//			{
//				t = 0x0800 ;
//			}
//			else
//			{
//				t -= 0x0C00 ;
//				t *= 2 ;
//				t += 0x0800 ;
//			}
//		}	
//		else
//		{
//			if ( t > 0x0400 )
//			{
//				t = 0x0800 ;
//			}
//			else
//			{
////				t += 0x0200 ;
//				t *= 2 ;
////				t -= 0x0400 ;
//			}
//		}
//		AnalogData[3] = t ;
//		t = AdcBuffer[13] ;
//		if ( t > 0x0800 )
//		{
//			if ( t < 0x0C00 )
//			{
//				t = 0x0800 ;
//			}
//			else
//			{
//				t -= 0x0C00 ;
//				t *= 2 ;
//				t += 0x0800 ;
//			}
//		}	
//		else
//		{
//			if ( t > 0x0400 )
//			{
//				t = 0x0800 ;
//			}
//			else
//			{
////				t += 0x0200 ;
//				t *= 2 ;
////				t -= 0x0400 ;
//			}
//		}
//		AnalogData[2] = t ;
//	}
#ifdef PCBX10
 if ( SticksPwmDisabled )
 {
#endif		
#ifdef PCBX10
	AnalogData[4] = AdcBuffer[4] ;
	AnalogData[5] = AdcBuffer[5] ;
	AnalogData[6] = 4095 - AdcBuffer[6] ;
	AnalogData[7] = 4095 - AdcBuffer[7] ;
	uint16_t temp1 = AdcBuffer[8] ;
#else
	AnalogData[4] = 4095 - AdcBuffer[4] ;
	AnalogData[5] = 4095 - AdcBuffer[5] ;
	AnalogData[6] = AdcBuffer[6] ;
	AnalogData[7] = AdcBuffer[7] ;
	uint16_t temp1 = AdcBuffer[11] ;
#endif
	AnalogData[8] = 4095 - AdcBuffer[8] ;
	AnalogData[12] = AdcBuffer[9] ;
	AnalogData[9] = 4095 - AdcBuffer[10] ;
	uint16_t temp = AnalogData[10] ;
	uint16_t temp2 ;
	if ( temp1 > temp )
	{
		temp2 = temp1 - temp ;
	}
	else
	{
		temp2 = temp - temp1 ;
	}
	if ( temp2 > 20 )   // changed by more than 20?
	{
		uint16_t time = 10 ;
		if ( temp > 0x0380 )
		{
			if ( temp1 < 0x0100 )
			{
				time = 30 ;
			}
		}
		if ((uint16_t)(get_tmr10ms() - SixPosTimer) > time )
		{
			AnalogData[10] = temp1 ;
		}
	}
	else
	{
		SixPosTimer = get_tmr10ms() ;
	}
	AnalogData[11] = AdcBuffer[13] ;
	AnalogData[13] = AdcBuffer[12] ;
#ifdef PCBX10
 }
 else
 {
	AnalogData[4] = 4095 - AdcBuffer[0] ;
	AnalogData[5] = 4095 - AdcBuffer[1] ;
	AnalogData[6] = AdcBuffer[2] ;
	AnalogData[7] = AdcBuffer[3] ;
	AnalogData[8] = 4095 - AdcBuffer[4] ;
	AnalogData[12] = AdcBuffer[5] ;
	AnalogData[9] = 4095 - AdcBuffer[6] ;
	uint16_t temp = AnalogData[10] ;
	uint16_t temp1 = AdcBuffer[7] ;
	uint16_t temp2 ;
	if ( temp1 > temp )
	{
		temp2 = temp1 - temp ;
	}
	else
	{
		temp2 = temp - temp1 ;
	}
	if ( temp2 > 20 )   // changed by more than 20?
	{
		uint16_t time = 10 ;
		if ( temp > 0x0380 )
		{
			if ( temp1 < 0x0100 )
			{
				time = 30 ;
			}
		}
		if ((uint16_t)(get_tmr10ms() - SixPosTimer) > time )
		{
			AnalogData[10] = temp1 ;
		}
	}
	else
	{
		SixPosTimer = get_tmr10ms() ;
	}
//	AnalogData[11] = AdcBuffer[13] ;
//	AnalogData[13] = AdcBuffer[12] ;
 	
 }
#endif
	if (ADC->CCR & ADC_CCR_VBATE )
	{
		VbattRtc = ADC1->DR ;
	}
}

// For linking
//uint16_t Analog_values[NUMBER_ANALOG] __DMA;

uint16_t getAnalogValue(uint32_t value)
{
  return AdcBuffer[value] ;
//  return 0; // Analog_values[value];
}

#else

#define STICK_LV    3
#define STICK_LH    2
#define STICK_RV    0
#define STICK_RH    1
#define POT_L       13
#define POT_R       5
#define SLIDE_L     9
#define SLIDE_R     8
#define SWITCHES1   11
#define SWITCHES2   7
#define SWITCHES3   10
#define SWITCHES4   6
#define BATTERY     14

// Sample time should exceed 1uS
#define SAMPTIME    4   // sample time = 28 cycles

uint16_t Analog_values[NUMBER_ANALOG] __DMA;

#define NUMBER_ANALOG_ADC1      13

#if 0
const int8_t ana_direction[NUMBER_ANALOG] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
#endif

void adcInit()
{
  // Enable clocks
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  RCC->AHB1ENR |= ADC_RCC_AHB1Periph_GPIO | RCC_AHB1ENR_DMA2EN;

  configure_pins(ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH |
                 ADC_GPIO_PIN_POT_R1 | ADC_GPIO_SWITCHES_PIN_R3 | ADC_GPIO_SWITCHES_PIN_R4, PIN_ANALOG | PIN_PORTA);

  configure_pins(ADC_GPIO_PIN_POT_R2 | ADC_GPIO_PIN_POT_L2, PIN_ANALOG | PIN_PORTB);

  configure_pins(ADC_GPIO_PIN_POT_L1 | ADC_GPIO_SWITCHES_PIN_L3 | ADC_GPIO_SWITCHES_PIN_L4 | ADC_GPIO_PIN_BATT, PIN_ANALOG | PIN_PORTC);

  ADC1->CR1 = ADC_CR1_SCAN;
  ADC1->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
  ADC1->SQR1 = (BATTERY<<0) + ((NUMBER_ANALOG_ADC1-1) << 20); // bits 23:20 = number of conversions
  ADC1->SQR2 = (SLIDE_L<<0) + (SLIDE_R<<5) + (SWITCHES1<<10) + (SWITCHES2<<15) + (SWITCHES3<<20) + (SWITCHES4<<25); // conversions 7 to 12
  ADC1->SQR3 = (STICK_LH<<0) + (STICK_LV<<5) + (STICK_RV<<10) + (STICK_RH<<15) + (POT_L<<20) + (POT_R<<25); // conversions 1 to 6
  ADC1->SMPR1 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) + (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24);
  ADC1->SMPR2 = SAMPTIME + (SAMPTIME<<3) + (SAMPTIME<<6) + (SAMPTIME<<9) + (SAMPTIME<<12) + (SAMPTIME<<15) + (SAMPTIME<<18) + (SAMPTIME<<21) + (SAMPTIME<<24) + (SAMPTIME<<27) ;

  ADC->CCR = 0 ; //ADC_CCR_ADCPRE_0 ;             // Clock div 2

  DMA2_Stream0->CR = DMA_SxCR_PL | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC;
  DMA2_Stream0->PAR = CONVERT_PTR_UINT(&ADC1->DR);
  DMA2_Stream0->M0AR = CONVERT_PTR_UINT(Analog_values);
  DMA2_Stream0->NDTR = NUMBER_ANALOG_ADC1;
  DMA2_Stream0->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0 ;
}

void adcRead()
{
  DMA2_Stream0->CR &= ~DMA_SxCR_EN ;              // Disable DMA
  ADC1->SR &= ~(uint32_t) ( ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR ) ;
  DMA2->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 |DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0 ; // Write ones to clear bits
  DMA2_Stream0->CR |= DMA_SxCR_EN ;               // Enable DMA
  ADC1->CR2 |= (uint32_t)ADC_CR2_SWSTART ;
  for (unsigned int i=0; i<10000; i++) {
    if (DMA2->LISR & DMA_LISR_TCIF0) {
      break;
    }
  }
  DMA2_Stream0->CR &= ~DMA_SxCR_EN ;              // Disable DMA
}

// TODO
void adcStop()
{
}

uint16_t getAnalogValue(uint32_t value)
{
  // return Analog_values[ana_mapping[value]];
  return 0; // Analog_values[value];
}

#endif
