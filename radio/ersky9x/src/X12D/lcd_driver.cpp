/*
 * Author - Mike Blandford
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x 
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

// Temp items to go to ersky9x.h
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define CONVERT_PTR_UINT(x) ((uint32_t)(x))
typedef const char pm_char;
typedef int32_t swsrc_t;
typedef uint16_t source_t;
//inline int divRoundClosest(const int n, const int d)
//{
//  return ((n < 0) ^ (d < 0)) ? ((n - d/2)/d) : ((n + d/2)/d);
//}


// For myeeprom.h
#ifndef PACK
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif
PACK(typedef struct {
  uint8_t type;
  int8_t  value;
}) CurveRef;

//#define	WHERE_TRACK		1

#include "../ersky9x.h"
#include "stddef.h"
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_ltdc.h"
#include "stm32f4xx_dma2d.h"
#include "stm32f4xx_fmc.h"
#include "board_horus.h"
#include "hal.h"
#include "../timers.h"
#include "../myeeprom.h"

extern "C" void lcdDrawSolidFilledRectDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) ;
#include "../lcd.h"
#include "lcd_driver.h"

#if defined(PCBT18)
  #define HBP  43
  #define VBP  12

  #define HSW  2
  #define VSW  4

  #define HFP  8
  #define VFP  8
#else
	#define HBP  42
	#define VBP  12

	#define HSW  2
	#define VSW  10

	#define HFP  3
	#define VFP  2
#endif

#define LCD_DIR_HORIZONTAL             0x0000
#define LCD_DIR_VERTICAL               0x0001

#define LCD_FIRST_LAYER                0
#define LCD_SECOND_LAYER               1

#define LCD_BKLIGHT_PWM_FREQ           300

uint32_t LcdFirstFrameBuffer[DISPLAY_BUFFER_SIZE/4] __SDRAM ;
uint32_t LcdSecondFrameBuffer[DISPLAY_BUFFER_SIZE/4] __SDRAM ;
uint32_t LcdBackupFrameBuffer[DISPLAY_BUFFER_SIZE/4] __SDRAM ;

//#ifdef INVERT_DISPLAY
uint8_t FontBuffer[65536*6] __SDRAM0 ;
//#endif

//#ifdef INVERT_DISPLAY
//uint16_t LcdBits ;
//uint16_t LcdErrors ;
//uint16_t Lcdy ;
//uint16_t Lcdx ;
//uint16_t Lcdc ;
//#endif

//#define LCD_FIRST_FRAME_BUFFER         SDRAM_BANK_ADDR
//#define LCD_SECOND_FRAME_BUFFER        (SDRAM_BANK_ADDR + DISPLAY_BUFFER_SIZE)
//#define LCD_BACKUP_FRAME_BUFFER        (SDRAM_BANK_ADDR + 2*DISPLAY_BUFFER_SIZE)

uint32_t CurrentFrameBuffer = (uint32_t)LcdFirstFrameBuffer;
uint32_t CurrentLayer = LCD_FIRST_LAYER;

//const uint8_t *ExtraHorusFont = NULL ;
//const uint8_t *ExtraHorusBigFont = NULL ;

#define NRST_LOW()   do { LCD_GPIO_NRST->BSRRH = LCD_GPIO_PIN_NRST; } while(0)
#define NRST_HIGH()  do { LCD_GPIO_NRST->BSRRL = LCD_GPIO_PIN_NRST; } while(0)

#define CR_MASK                     ((uint32_t)0xFFFCC0F8)  /* DMA2D CR Mask */

//uint8_t speaker[] = {
//4,8,0,
//0x38,0x38,0x7C,0xFE
//} ;

#ifdef INVERT_DISPLAY
//uint8_t *copyFonts( void ) ;
#endif
uint8_t *unpackAllFonts( uint8_t *fontPtr ) ;

// Temp from lcd.cpp
//uint16_t lcdColorTable[LCD_COLOR_COUNT];
void backlightEnable(uint8_t dutyCycle) ;

//void lcdColorsInit()
//{
//  lcdColorTable[TEXT_COLOR_INDEX] = BLACK;
//  lcdColorTable[TEXT_BGCOLOR_INDEX] = WHITE;
//  lcdColorTable[TEXT_INVERTED_COLOR_INDEX] = WHITE;
//  lcdColorTable[TEXT_INVERTED_BGCOLOR_INDEX] = RED;
//  lcdColorTable[LINE_COLOR_INDEX] = RGB(88, 88, 90);
//  lcdColorTable[SCROLLBOX_COLOR_INDEX] = RED;
//  lcdColorTable[MENU_TITLE_BGCOLOR_INDEX] = DARKGREY;
//  lcdColorTable[MENU_TITLE_COLOR_INDEX] = WHITE;
//  lcdColorTable[MENU_TITLE_DISABLE_COLOR_INDEX] = RGB(130, 1, 5);
//  lcdColorTable[HEADER_COLOR_INDEX] = DARKGREY;
//  lcdColorTable[ALARM_COLOR_INDEX] = RED;
//  lcdColorTable[WARNING_COLOR_INDEX] = YELLOW;
//  lcdColorTable[TEXT_DISABLE_COLOR_INDEX] = RGB(0x60, 0x60, 0x60);
//  lcdColorTable[CURVE_AXIS_COLOR_INDEX] = RGB(180, 180, 180);
//  lcdColorTable[CURVE_COLOR_INDEX] = RED;
//  lcdColorTable[CURVE_CURSOR_COLOR_INDEX] = RED;
//  lcdColorTable[TITLE_BGCOLOR_INDEX] = RED;
//  lcdColorTable[HEADER_BGCOLOR_INDEX] = DARKRED;
//}


#if defined(PCBX10)
void delay_ms(uint32_t ms)
{
  while (ms--) {
		hw_delay( 10000 ) ; // units of 0.1uS
  }
}
#endif

static void LCD_AF_GPIOConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

      /* GPIOs Configuration */
      /*
       +------------------------+-----------------------+----------------------------+
       +                       LCD pins assignment                                   +
       +------------------------+-----------------------+----------------------------
       |                        |  LCD_TFT G2 <-> PJ.09 |                            |
       |  LCD_TFT R3 <-> PJ.02  |  LCD_TFT G3 <-> PJ.10 |  LCD_TFT B3 <-> PJ.15      |
       |  LCD_TFT R4 <-> PJ.03  |  LCD_TFT G4 <-> PJ.11 |  LCD_TFT B4 <-> PK.03      |
       |  LCD_TFT R5 <-> PJ.04  |  LCD_TFT G5 <-> PK.00 |  LCD_TFT B5 <-> PK.04      |
       |  LCD_TFT R6 <-> PJ.05  |  LCD_TFT G6 <-> PK.01 |  LCD_TFT B6 <-> PK.05      |
       |  LCD_TFT R7 <-> PJ.06  |  LCD_TFT G7 <-> PK.02 |  LCD_TFT B7 <-> PK.06      |
       -------------------------------------------------------------------------------
                |  LCD_TFT HSYNC <-> PI.12  | LCDTFT VSYNC <->  PI.13 |
                |  LCD_TFT CLK   <-> PI.14  | LCD_TFT DE   <->  PK.07 ///
                 -----------------------------------------------------
                |LCD_TFT backlight <-> PA.04| LCD_CS <-> PI.10    |LCD_SCK<->PI.11
                 -----------------------------------------------------
  */
 /*GPIOI configuration*/

  GPIO_PinAFConfig(GPIOI,GPIO_PinSource12,GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOI,GPIO_PinSource13,GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOI,GPIO_PinSource14,GPIO_AF_LTDC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Speed =GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_Mode =GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType =GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd =GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOI,&GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOK, GPIO_PinSource7, GPIO_AF_LTDC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOK, &GPIO_InitStructure);

   /* GPIOJ configuration */
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource2, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource5, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource9, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOJ, GPIO_PinSource15, GPIO_AF_LTDC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | \
                               GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_15;

  GPIO_Init(GPIOJ, &GPIO_InitStructure);

  /* GPIOK configuration */
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource0, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource1, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource2, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource5, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOK, GPIO_PinSource6, GPIO_AF_LTDC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 ;

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOK, &GPIO_InitStructure);
}

//static void LCD_Backlight_Config(void)
//{
//  GPIO_InitTypeDef GPIO_InitStructure;
//  GPIO_InitStructure.GPIO_Pin = LCD_GPIO_PIN_BL;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_Init(LCD_GPIO_BL, &GPIO_InitStructure);
//  GPIO_PinAFConfig(LCD_GPIO_BL, LCD_GPIO_PinSource_BL, LCD_GPIO_AF_BL);
//}

static void LCD_NRSTConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = LCD_GPIO_PIN_NRST;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(LCD_GPIO_NRST, &GPIO_InitStructure);
}

static void delay3(uint32_t nCount)
{
  uint32_t index = 0;
  for(index = (1000 * 100 * nCount); index != 0; --index)
  {
    __asm("nop\n");
  }
}

static void lcd_reset(void)
{
  NRST_HIGH();
  delay3(1);

  NRST_LOW(); //  RESET();
  delay3(20);

  NRST_HIGH();
  delay3(30);
}

static void LCD_Init_LTDC(void)
{
  LTDC_InitTypeDef       LTDC_InitStruct;

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAI_N = 192 Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLL_LTDC = 192/3 = 64 Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 64/4 = 16 Mhz */
  //second pam is for audio
  //third pam is for LCD
  RCC_PLLSAIConfig(192, 6, 3);
  RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div4);  //Modify by Fy
  /* Enable PLLSAI Clock */
  RCC_PLLSAICmd(ENABLE);

  /* Wait for PLLSAI activation */
  while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET)
  {
  }

  /* LTDC Configuration *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;
  /* Initialize the vertical synchronization polarity as active low */
  LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;
  /* Initialize the data enable polarity as active low */
  LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;

  /* Configure R,G,B component values for LCD background color */
  LTDC_InitStruct.LTDC_BackgroundRedValue = 0;
  LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;
  LTDC_InitStruct.LTDC_BackgroundBlueValue = 0;

  /* Configure horizontal synchronization width */
  LTDC_InitStruct.LTDC_HorizontalSync = HSW;
  /* Configure vertical synchronization height */
  LTDC_InitStruct.LTDC_VerticalSync = VSW;
  /* Configure accumulated horizontal back porch */
  LTDC_InitStruct.LTDC_AccumulatedHBP = HBP;
  /* Configure accumulated vertical back porch */
  LTDC_InitStruct.LTDC_AccumulatedVBP = VBP;
  /* Configure accumulated active width */
  LTDC_InitStruct.LTDC_AccumulatedActiveW = LCD_W + HBP;
  /* Configure accumulated active height */
  LTDC_InitStruct.LTDC_AccumulatedActiveH = LCD_H + VBP;
  /* Configure total width */
  LTDC_InitStruct.LTDC_TotalWidth = LCD_W + HBP + HFP;
  /* Configure total height */
  LTDC_InitStruct.LTDC_TotalHeigh = LCD_H + VBP + VFP;

// init ltdc
  LTDC_Init(&LTDC_InitStruct);

//#if 0
//  LTDC_ITConfig(LTDC_IER_LIE, ENABLE);
//  NVIC_InitTypeDef NVIC_InitStructure;
//  NVIC_InitStructure.NVIC_IRQChannel = LTDC_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LTDC_IRQ_PRIO;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* Not used as 4 bits are used for the pr     e-emption priority. */;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init( &NVIC_InitStructure );

//  DMA2D_ITConfig(DMA2D_CR_TCIE, ENABLE);
//  NVIC_InitStructure.NVIC_IRQChannel = DMA2D_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA_SCREEN_IRQ_PRIO;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; /* Not used as 4 bits are used for the pr     e-emption priority. */;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init( &NVIC_InitStructure );

//  DMA2D->IFCR = (unsigned long)DMA2D_IFSR_CTCIF;
//#endif
}

/**
  * @brief  Initializes the LCD Layers.
  * @param  None
  * @retval None
  */
static void LCD_LayerInit()
{
  LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct;

  /* Windowing configuration */
  /* In this case all the active display area is used to display a picture then :
  Horizontal start = horizontal synchronization + Horizontal back porch = 30
  Horizontal stop = Horizontal start + window width -1 = 30 + 240 -1
  Vertical start   = vertical synchronization + vertical back porch     = 4
  Vertical stop   = Vertical start + window height -1  = 4 + 320 -1      */
  LTDC_Layer_InitStruct.LTDC_HorizontalStart = HBP + 1;
  LTDC_Layer_InitStruct.LTDC_HorizontalStop = (LCD_W + HBP);
  LTDC_Layer_InitStruct.LTDC_VerticalStart = VBP + 1;;
  LTDC_Layer_InitStruct.LTDC_VerticalStop = (LCD_H + VBP);

  /* Pixel Format configuration*/
  LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_RGB565;
  /* Alpha constant (255 totally opaque) */
  LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255;
  /* Default Color configuration (configure A,R,G,B component values) */
  LTDC_Layer_InitStruct.LTDC_DefaultColorBlue = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorRed = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;

  /* Configure blending factors */
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_CA;
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_CA;

  /* the length of one line of pixels in bytes + 3 then :
  Line Lenth = Active high width x number of bytes per pixel + 3
  Active high width         = LCD_W
  number of bytes per pixel = 2    (pixel_format : RGB565)
  */
  LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((LCD_W * 2) + 3);
  /* the pitch is the increment from the start of one line of pixels to the
  start of the next line in bytes, then :
  Pitch = Active high width x number of bytes per pixel */
  LTDC_Layer_InitStruct.LTDC_CFBPitch = (LCD_W * 2);

  /* Configure the number of lines */
  LTDC_Layer_InitStruct.LTDC_CFBLineNumber = LCD_H;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = (uint32_t)LcdFirstFrameBuffer;

  /* Initialize LTDC layer 1 */
  LTDC_LayerInit(LTDC_Layer1, &LTDC_Layer_InitStruct);

  /* Configure Layer 2 */
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_PAxCA;
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_PAxCA;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = (uint32_t)LcdSecondFrameBuffer;

  /* Initialize LTDC layer 2 */
  LTDC_LayerInit(LTDC_Layer2, &LTDC_Layer_InitStruct);

  /* LTDC configuration reload */
  LTDC_ReloadConfig(LTDC_IMReload);

  LTDC_LayerCmd(LTDC_Layer1, ENABLE);
  LTDC_LayerCmd(LTDC_Layer2, ENABLE);

  LTDC_ReloadConfig(LTDC_IMReload);

  /* dithering activation */
  LTDC_DitherCmd(ENABLE);
}

void backlightInit()
{
//  if (IS_HORUS_PROD())
//  if (0)
#if defined(PCBX12D)
	if ( isProdVersion() )
#endif
//  if (GPIOI->IDR & 0x0800)
	{
  	// PIN init
	  RCC_APB1PeriphClockCmd( BL_RCC_APB1Periph, ENABLE ) ;
  	GPIO_InitTypeDef GPIO_InitStructure;
  	GPIO_InitStructure.GPIO_Pin = BL_GPIO_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  	GPIO_Init(BL_GPIO, &GPIO_InitStructure);
  	GPIO_PinAFConfig(BL_GPIO, BL_GPIO_PinSource, BL_GPIO_AF);
	}
#if defined(PCBX12D)
	else
	{
  	// PIN init
  	GPIO_InitTypeDef GPIO_InitStructure;
  	GPIO_InitStructure.GPIO_Pin = BLP_GPIO_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  	GPIO_Init(BL_GPIO, &GPIO_InitStructure);
  	GPIO_PinAFConfig(BL_GPIO, BLP_GPIO_PinSource, BLP_GPIO_AF);
  }
#endif
  // TIMER init
#if defined(PCBX12D)
	if ( isProdVersion() )
//  if (IS_HORUS_PROD())
	{
    BL_TIMER->ARR = 100;
    BL_TIMER->PSC = BL_TIMER_FREQ / 10000 - 1; // 1kHz
    BL_TIMER->CCMR2 = TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2; // PWM
    BL_TIMER->CCER = TIM_CCER_CC4E;
    BL_TIMER->CCR4 = 0;
    BL_TIMER->EGR = 0;
    BL_TIMER->CR1 = TIM_CR1_CEN; // Counter enable
  }
  else
	{
	  
		RCC_APB1PeriphClockCmd( BL_RCC_APB1Periph, ENABLE ) ;
		
    BL_TIMER->ARR = 100;
    BL_TIMER->PSC = BL_TIMER_FREQ / 10000 - 1; // 1kHz
    BL_TIMER->CCMR2 = TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2; // PWM
    BL_TIMER->CCER = TIM_CCER_CC4E;
    BL_TIMER->CCR4 = 0;
    BL_TIMER->EGR = 0;
    BL_TIMER->CR1 = TIM_CR1_CEN; // Counter enable
    
		BLP_TIMER->ARR = 100;
    BLP_TIMER->PSC = BL_TIMER_FREQ / 10000 - 1; // 1kHz
    BLP_TIMER->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM
    BLP_TIMER->CCER = TIM_CCER_CC1E | TIM_CCER_CC1NE;
    BLP_TIMER->CCR1 = 100;
    BLP_TIMER->EGR = 1;
    BLP_TIMER->CR1 |= TIM_CR1_CEN; // Counter enable
    BLP_TIMER->BDTR |= TIM_BDTR_MOE;
  }
#elif defined(PCBX10)
  BL_TIMER->ARR = 100;
  BL_TIMER->PSC = BL_TIMER_FREQ / 10000 - 1; // 1kHz
  BL_TIMER->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE; // PWM mode 1
  BL_TIMER->CCER = TIM_CCER_CC3E | TIM_CCER_CC3NE;
  BL_TIMER->CCR3 = 100;
  BL_TIMER->EGR = TIM_EGR_UG;
  BL_TIMER->CR1 |= TIM_CR1_CEN; // Counter enable
  BL_TIMER->BDTR |= TIM_BDTR_MOE;
#endif
}


//void LCD_ControlLight(uint16_t dutyCycle)
//{
////  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//  TIM_OCInitTypeDef  TIM_OCInitStructure;

//  uint16_t freq = LCD_BKLIGHT_PWM_FREQ;

//  uint32_t temp = 0;
//  temp = 1000000 / freq - 1;

////  TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock / 1000000 - 1 ;
////  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
////  TIM_TimeBaseStructure.TIM_Period = temp;
////  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
////  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

////  TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

//	TIM8->CR1 = 0 ;
//	TIM8->ARR = temp ;
//	TIM8->PSC = SystemCoreClock / 1000000 - 1 ;
//  TIM8->RCR = 0 ;
//  TIM8->EGR = TIM_EGR_UG ;

//  unsigned long tempValue;
//  tempValue = (unsigned long)divRoundClosest((temp+1)*(100-dutyCycle), dutyCycle);

////  TIM_Cmd(TIM8, DISABLE);
//	TIM8->CR1 &= (uint16_t)~TIM_CR1_CEN ;

//  /* Channel 1 Configuration in PWM mode */
//  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
//  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
//  TIM_OCInitStructure.TIM_Pulse = (uint32_t)tempValue;
//  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
//  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
//  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
//  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;
//  TIM_OC1Init(TIM8, &TIM_OCInitStructure);

//  TIM8->CR1 |= TIM_CR1_CEN ;
////  TIM_Cmd(TIM8, ENABLE);
//  TIM8->BDTR |= TIM_BDTR_MOE ;
////  TIM_CtrlPWMOutputs(TIM8, ENABLE);
//}

/*********************************output****************************************/
/**
  * @brief  Initializes the LCD.
  * @param  None
  * @retval None
  */

const uint16_t font_stdsize_specs[] = {
#include "font_stdsize.specs"
};

const uint16_t font_midsize_specs[] = {
#include "font_midsize.specs"
};

const uint16_t font_smlsize_specs[] = {
#include "FontSmall.specs"
};

//const uint8_t font_smlsize[] = {
//#include "font_smlsize.lbm"
//};

const uint16_t font_tinsize_specs[] = {
#include "font_tinsize.specs"
};

const uint8_t font_tinsize[] = {
#include "font_tinsize.lbm"
};

const uint16_t font_dblsize_specs[] = {
#include "font_dblsize.specs"
};

const uint16_t font_xxlsize_specs[] = {
#include "font_xxlsize.specs"
};

const uint16_t font_stdsizebold_specs[] = {
#include "font_stdsizebold.specs"
};


//void sortFont( uint8_t *srcFont, uint16_t *specs, uint16_t lastSpec, char *fontName ) ;

void LCD_Init(void)
{
//	uint32_t save ;
  /* Reset the LCD --------------------------------------------------------*/
  LCD_NRSTConfig();
  lcd_reset();

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();

  LCD_Init_LTDC();

  //config backlight
	backlightInit() ;
//  LCD_Backlight_Config();

  //Max Value
	backlightEnable(100) ;
//  LCD_ControlLight(100);

//	save = CurrentFrameBuffer ;
	 
//	CurrentFrameBuffer = LCD_SECOND_FRAME_BUFFER ;
//	lcdDrawSolidFilledRectDMA( 0, 0, 480, 272, 0x001F ) ;
//	CurrentFrameBuffer = LCD_FIRST_FRAME_BUFFER ;
//	lcdDrawSolidFilledRectDMA( 0, 0, 480, 272, 0x001F ) ;
//	CurrentFrameBuffer = save ;
	DMA2D->FGCLUT[0] = 0 ;
	DMA2D->FGCLUT[1] = RGB32(160,0,0) ;
	DMA2D->FGCLUT[2] = RGB32(0,160,0) ;
	DMA2D->FGCLUT[3] = RGB32(160,160,0) ;
	DMA2D->FGCLUT[4] = RGB32(0,0,160) ;
	DMA2D->FGCLUT[5] = RGB32(160,0,160) ;
	DMA2D->FGCLUT[6] = RGB32(0,160,160) ;
	DMA2D->FGCLUT[7] = RGB32(192,192,192) ;
	DMA2D->FGCLUT[8] = RGB32(128,128,128) ;
	DMA2D->FGCLUT[9] = RGB32(255,0,0) ;
	DMA2D->FGCLUT[10] = RGB32(0,255,0) ;
	DMA2D->FGCLUT[11] = RGB32(255,255,0) ;
	DMA2D->FGCLUT[12] = RGB32(0,0,255) ;
	DMA2D->FGCLUT[13] = RGB32(255,0,255) ;
	DMA2D->FGCLUT[14] = RGB32(0,255,255) ;
	DMA2D->FGCLUT[15] = RGB32(255,255,255) ;

//	uint16_t last ;
//	uint32_t i ;
//	i = sizeof(font_dblsize_specs) ;
//	i /= 2 ;
//	last = font_dblsize_specs[i-1] ;
//	sortFont( (uint8_t *)font_dblsize, (uint16_t *)font_dblsize_specs, last, (char *)"FontDbl.lbm" ) ;

////	i = sizeof(font_tinsize_specs) ;
////	i /= 2 ;
////	last = font_tinsize_specs[i-1] ;
////	sortFont( (uint8_t *)font_tinsize, (uint16_t *)font_tinsize_specs, last, (char *)"FontTin.lbm" ) ;
////	i = sizeof(font_smlsize_specs) ;
////	i /= 2 ;
////	last = font_smlsize_specs[i-1] ;
////	sortFont( (uint8_t *)font_smlsize, (uint16_t *)font_smlsize_specs, last, (char *)"FontSml.lbm" ) ;
//	i = sizeof(font_xxlsize_specs) ;
//	i /= 2 ;
//	last = font_xxlsize_specs[i-1] ;
//	sortFont( (uint8_t *)font_xxlsize, (uint16_t *)font_xxlsize_specs, last, (char *)"FontXxl.lbm" ) ;
//	i = sizeof(font_stdsizebold_specs) ;
//	i /= 2 ;
//	last = font_stdsizebold_specs[i-1] ;
//	sortFont( (uint8_t *)font_stdsizebold, (uint16_t *)font_stdsizebold_specs, last, (char *)"FontStbBold.lbm" ) ;

}

uint8_t *unpackFont( uint8_t*Romfont, uint16_t *specs, uint8_t *fontPtr, uint32_t fontSize ) ;

/**
  * @brief  Sets the LCD Layer.
  * @param  Layerx: specifies the Layer foreground or background.
  * @retval None
  */
void LCD_SetLayer(uint32_t Layerx)
{
  if (Layerx == LCD_FIRST_LAYER)
  {
    CurrentFrameBuffer = (uint32_t)LcdFirstFrameBuffer;
    CurrentLayer = LCD_FIRST_LAYER;
  }
  else
  {
    CurrentFrameBuffer = (uint32_t)LcdSecondFrameBuffer;
    CurrentLayer = LCD_SECOND_LAYER;
  }
}

/**
  * @brief  Configure the transparency.
  * @param  transparency: specifies the transparency,
  *         This parameter must range from 0x00 to 0xFF.
  * @retval None
  */
void LCD_SetTransparency(uint8_t transparency)
{
  if (CurrentLayer == LCD_FIRST_LAYER) {
    LTDC_LayerAlpha(LTDC_Layer1, transparency);
  }
  else {
    LTDC_LayerAlpha(LTDC_Layer2, transparency);
  }
  LTDC_ReloadConfig(LTDC_IMReload);
}

void lcdInit(void)
{
	uint8_t *fontPtr ;
  /* Initialize the LCD */
  LCD_Init();
  LCD_LayerInit();

  /* Enable LCD display */
  LTDC_Cmd(ENABLE);

  /* Set Background layer */
  LCD_SetLayer(LCD_FIRST_LAYER);
  // lcdClear();
  LCD_SetTransparency(255);
  lcd_blank() ;
  LCD_SetTransparency(0);
  
  /* Set Foreground layer */
  LCD_SetLayer(LCD_SECOND_LAYER);
  LCD_SetTransparency(255);
  lcd_blank() ;
  LCD_SetTransparency(0);
#ifdef INVERT_DISPLAY
//	fontPtr = copyFonts() ;
	fontPtr = FontBuffer ;
#else
	fontPtr = FontBuffer ;
#endif

	fontPtr = unpackAllFonts( fontPtr ) ;

}

volatile uint8_t Dma2DdoneFlag ;
volatile uint8_t LcdClearing ;

//uint16_t LastDmaRectx ;
//uint16_t LastDmaRecty ;
//uint16_t LastDmaRectw ;
//uint16_t LastDmaRecth ;
//uint32_t LastFrameBuffer ;
//uint32_t LastAddr ;

//uint16_t LCDLastOp ;
//uint16_t LCDIntLastOp ;
//uint16_t LCDIntLastSr ;
//uint32_t LCDIntLastCr ;
//uint16_t DMA2Dtimer[5] ;

void startLcdDrawSolidFilledRectDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour)
{
#ifdef INVERT_DISPLAY
	x = (LCD_W-1) - x - (w - 1) ;
	if ( x >= LCD_W )
	{
		x += LCD_W ;
	}
	y = (LCD_H-1) - y - (h - 1) ;
	if ( y >= LCD_H )
	{
//		LcdBits |= 1 ;
//		LcdErrors += 1 ;
		y = 0 ;
	}

//	LastDmaRectx = x ;
//	LastDmaRecty = y ;
//	LastDmaRectw = w ;
//	LastDmaRecth = h ;
//	LastFrameBuffer = CurrentFrameBuffer ;

#endif
	uint32_t addr = CurrentFrameBuffer + 2*(LCD_W*y + x) ;
//	LastAddr = addr ;
  DMA2D_DeInit() ;

	DMA2D->CR = (DMA2D->CR & (uint32_t)CR_MASK) | DMA2D_R2M ;

	DMA2D->OPFCCR = (DMA2D->OPFCCR & ~(uint32_t)DMA2D_OPFCCR_CM ) | DMA2D_RGB565 ;
  
	DMA2D->OCOLR = colour ;

  DMA2D->OMAR = addr ;
  
	DMA2D->OOR = (DMA2D->OOR & ~(uint32_t)DMA2D_OOR_LO ) | (LCD_W - w) ;
  
  DMA2D->NLR = (DMA2D->NLR & ~(DMA2D_NLR_NL | DMA2D_NLR_PL) ) | ( h | (w << 16) ) ;
	
  /* Start Transfer */
	Dma2DdoneFlag = 0 ;
  DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
//  if ( DMA2D->ISR & DMA2D_ISR_TCIF )
//	{
//		LCDLastOp |= 0x20 ;
//	}
//	DMA2Dtimer[0] = getTmr2MHz() ;
//	DMA2D_StartTransfer();
	DMA2D->CR |= (uint32_t)DMA2D_CR_START ;
	
	NVIC_SetPriority( DMA2D_IRQn, 12 ) ;
  DMA2D->CR |= DMA2D_CR_TCIE ;
	NVIC_EnableIRQ(DMA2D_IRQn) ;
	 
}

//void startLcdDrawSolidFilledRectDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour)
//{
//	uint32_t addr = CurrentFrameBuffer + 2*(LCD_W*y + x);
//  uint8_t red = (0xF800 & colour) >> 11;
//  uint8_t blue = 0x001F & colour;
//  uint8_t green = (0x07E0 & colour) >> 5;

//  DMA2D_DeInit();

//  DMA2D_InitTypeDef DMA2D_InitStruct;
//  DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;
//  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
//  DMA2D_InitStruct.DMA2D_OutputGreen = green;
//  DMA2D_InitStruct.DMA2D_OutputBlue = blue;
//  DMA2D_InitStruct.DMA2D_OutputRed = red;
//  DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;
//  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = addr;
//  DMA2D_InitStruct.DMA2D_OutputOffset = (LCD_W - w);
//  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
//  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
//  DMA2D_Init(&DMA2D_InitStruct);

//  /* Start Transfer */
//  DMA2D_StartTransfer();
//	Dma2DdoneFlag = 0 ;
	
//	NVIC_SetPriority( DMA2D_IRQn, 12 ) ;
//  DMA2D->CR |= DMA2D_CR_TCIE ;
//	NVIC_EnableIRQ(DMA2D_IRQn) ;
//}

uint16_t DMA2DerrorCount ;
//uint16_t DMA2DabortCount ;
//uint16_t DMA2DintxCount ;


void waitDma2Ddone( uint16_t timeout )
{
	uint16_t t1 = getTmr2MHz() ;
	uint16_t t2 ;
#ifdef WHERE_TRACK
extern void notePosition( uint8_t byte ) ;
	notePosition('>') ;
#endif
	while ( Dma2DdoneFlag == 0 )
	{
		// wait
		t2 = getTmr2MHz() - t1 ;
		if ( t2 > timeout )		// 1.5mS, was 16mS
		{
			// Error timeout
			DMA2DerrorCount  += 1 ;

  		DMA2D_DeInit() ;	// Reset DMA2D
			
#ifdef WHERE_TRACK
extern void notePosition( uint8_t byte ) ;
	notePosition('X') ;
#endif
			return ;
		}
	}
//  if (DMA2D->CR & (uint32_t)DMA2D_CR_START)
//	{
//#ifdef WHERE_TRACK
//extern void notePosition( uint8_t byte ) ;
//	notePosition('Z') ;
//#endif
//	}
}

void pollDma2Ddone( uint16_t timeout )
{
	uint16_t t1 = getTmr2MHz() ;
	uint16_t t2 ;
#ifdef WHERE_TRACK
extern void notePosition( uint8_t byte ) ;
	notePosition('<') ;
#endif
 	while (DMA2D->CR & (uint32_t)DMA2D_CR_START)
	{
		// wait
		t2 = getTmr2MHz() - t1 ;
		if ( t2 > timeout )		// 1.5mS, was 16mS
		{
			// Error timeout
			DMA2DerrorCount  += 1 ;
  		DMA2D_DeInit() ;	// Reset DMA2D
  
#ifdef WHERE_TRACK
extern void notePosition( uint8_t byte ) ;
	notePosition('Y') ;
#endif
		  DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
			return ;
		}
	}
  DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
}

void lcdDrawSolidFilledRectDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
//#ifdef INVERT_DISPLAY
//	x = (LCD_W-1) - x - (w - 1) ;
//	y = (LCD_H-1) - y - (h - 1) ;
//#endif
  if (DMA2D->CR & (uint32_t)DMA2D_CR_START)
	{
#ifdef WHERE_TRACK
extern void notePosition( uint8_t byte ) ;
	notePosition('Q') ;
#endif
		DMA2D->CR |= DMA2D_CR_ABORT ;
//		DMA2DabortCount += 1 ;
	}
	
	startLcdDrawSolidFilledRectDMA( x, y, w, h, color) ;
	
	waitDma2Ddone(( w*h > 1000 ) ? 10000 : 3000) ;
	/* Wait for CTC Flag activation */
//  while (DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET);
}

#ifdef WHERE_TRACK
extern void notePosition( uint8_t byte ) ;
#endif


extern "C" void DMA2D_IRQHandler( void )
{
#ifdef WDOG_REPORT
#ifdef PCBSKY	
	GPBR->SYS_GPBR1 = 0x57 ;
#else
	RTC->BKP1R = 0x57 ;
#endif
#endif
#ifdef WHERE_TRACK
	notePosition('0'+Dma2DdoneFlag+((DMA2D->CR & DMA2D_CR_START)<< 1)) ;
	notePosition('@'+(DMA2D->ISR & 0x3F)) ;
#endif
//	DMA2Dtimer[4] = DMA2D->ISR ;
  DMA2D->CR &= ~DMA2D_CR_TCIE ;
  DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
	if (DMA2D->CR & DMA2D_CR_START)
	{
//		DMA2Dtimer[1] = getTmr2MHz() - DMA2Dtimer[0] ;
//		DMA2Dtimer[2] = 1 ;
//		LCDIntLastSr = DMA2Dtimer[4] ;
//		LCDIntLastCr = DMA2D->CR ;
		DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
  	DMA2D->CR |= DMA2D_CR_TCIE ;
//		DMA2DintxCount += 1 ;
//		LCDIntLastOp = LCDLastOp ;
		if (DMA2D->CR & DMA2D_CR_START)
		{
			return ;
		}
	  DMA2D->CR &= ~DMA2D_CR_TCIE ;
	}
//	if ( DMA2Dtimer[2] )
//	{
//		DMA2Dtimer[3] = getTmr2MHz() - DMA2Dtimer[0] ;
//	}
//	DMA2Dtimer[2] = 0 ;
	Dma2DdoneFlag	= 1 ;
	if ( LcdClearing )
	{
#ifdef WHERE_TRACK
	if ( LcdClearing )
	{
		notePosition('7') ;
	}
#endif
		if ( LcdClearing == 1 )
		{
//			LcdClearing = 2 ;
//			startLcdDrawSolidFilledRectDMA( 0, 128, 480-128, 272-128-22, 0 ) ;
//		}
//		else if ( LcdClearing == 2 )
//		{
#ifdef WHERE_TRACK
extern void notePosition( uint8_t byte ) ;
	if ( LcdClearing )
	{
		notePosition('6') ;
	}
#endif
			LcdClearing = 2 ;
//			startLcdDrawSolidFilledRectDMA( 480-128, 128, 128, 272-128-22-64, 0 ) ;
//	LCDLastOp = 'B' ;
//			startLcdDrawSolidFilledRectDMA( 0, 128+64+16+16+16, 480, 64-16-16-16, 0 ) ;
			
//			startLcdDrawSolidFilledRectDMA( 0, 240, 480, 16, 0 ) ;



			LcdClearing = 0 ;

		}
		else
		{
			LcdClearing = 0 ;
		}
	}
}


void lcd_clearBackground()
{
	LcdClearing = 1 ;
//	LCDLastOp = 'C' ;
//	startLcdDrawSolidFilledRectDMA( 0, 0, 480, 128+64+16+16+16, LcdBackground ) ;
	startLcdDrawSolidFilledRectDMA( 0, 0, 480, 242, LcdBackground ) ;


//	startLcdDrawSolidFilledRectDMA( 0, 0, 480, 128+64, 0xF800 + 0x07E0 + 0x001F ) ;
//	startLcdDrawSolidFilledRectDMA( 0, 0, 480, 128+64, 0x7800 + 0x03E0 + 0x000F ) ;
}

void waitLcdClearDdone()
{
	uint16_t t1 = getTmr2MHz() ;
	uint16_t t2 ;
#ifdef WHERE_TRACK
extern void notePosition( uint8_t byte ) ;
	if ( LcdClearing )
	{
		notePosition('9') ;
	}
#endif
	while ( LcdClearing )
	{
		// wait
		t2 = getTmr2MHz() - t1 ;
		if ( t2 > 32000 )		// 16mS
		{
			// Error timeout
			DMA2DerrorCount  += 1 ;
  		DMA2D_DeInit() ;	// Reset DMA2D
			return ;
		}
 #ifndef MIXER_TASK
extern void checkRunMixer() ;
extern uint8_t MaintenanceRunning ;
		if ( MaintenanceRunning == 0 )
		{
			checkRunMixer() ;
		}
 #endif
	}
#ifdef WHERE_TRACK
	notePosition('8') ;
#endif
}

#include "..\font.lbm"

#ifdef INVERT_DISPLAY
//const uint8_t RomFontSmall[] =
//{
//#include "font.lbm"
//} ;

//const uint8_t RomFont5x7h[] =
//{
////#include "..\font5x7h.lbm"
//#include "..\f7x5.lbm"
//} ;

//const uint8_t RomFont5x7hBold[] =
//{
//#include "..\font5x7hBold.lbm"
//} ;

//const uint8_t RomFont_dblsizeh[] =
//{
//#include "..\font_dblsizeh.lbm"
//} ;

//const uint8_t RomFont_12x8h[] =
//{
////#include "..\F12x8.lbm"
//#include "..\font12x8h.lbm"
//} ;


//const uint8_t Romfont_fr_h_extra[] =
//{
//#include "..\font5x7Frh.lbm"
//} ;
//const uint8_t Romfont_fr_h_big_extra[] =
//{
//#include "..\font_fr_10x14h.lbm"
//} ;
//const uint8_t Romfont_de_h_extra[] =
//{
//#include "..\font5x7Deh.lbm"
//} ;
//const uint8_t Romfont_de_h_big_extra[] =
//{
//#include "..\font_de_10x14h.lbm"
//} ;
//const uint8_t Romfont_se_h_extra[] =
//{
//#include "..\font5x7Seh.lbm"
//} ;
//const uint8_t Romfont_se_h_big_extra[] =
//{
//#include "..\font_se_10x14h.lbm"
//} ;
//const uint8_t Romfont_it_h_extra[] =
//{
//#include "..\font5x7Ith.lbm"
//} ;
//const uint8_t Romfont_pl_h_extra[] =
//{
//#include "..\font5x7Plh.lbm"
//} ;
//const uint8_t Romfont_xxl[] =
//{
//#include "..\fontxxl.lbm"
//} ;


//uint8_t *FontSmall ;
//uint8_t *Font_12x8h ;
//uint8_t *Font_dblsizeh ;
//uint8_t *Font5x7h ;
//uint8_t *Font5x7hBold ;
uint8_t *font_fr_h_extra ;
uint8_t *font_fr_h_big_extra ;
uint8_t *font_de_h_extra ;
uint8_t *font_de_h_big_extra ;
uint8_t *font_se_h_extra ;
uint8_t *font_se_h_big_extra ;
uint8_t *font_it_h_extra ;
uint8_t *font_pl_h_extra ;
//uint8_t *font_xxl ;


void invertFont( uint8_t *dest, uint8_t *source, uint32_t bytesPerChar, uint32_t size )
{
	uint32_t count ;
	uint32_t chars = size / bytesPerChar ;
	uint8_t byte ;

	while ( chars-- )
	{
		count = bytesPerChar ;
		source += bytesPerChar ;
		while ( count-- )
		{
			byte = *--source ;
			byte = ( byte << 4 ) | ( byte >> 4 ) ;
			*dest++ = byte ;
		}
		source += bytesPerChar ;
	}
}

//uint8_t *copyFonts()
//{
//	uint32_t i ;
//	uint8_t *dest = FontBuffer ;

////	Font_12x8h = dest ;
////	i = (sizeof(RomFont_12x8h) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)RomFont_12x8h, 16*32/2, i ) ;
//////	memmove( dest, RomFont_12x8h, i ) ;
////	dest += i ;

////	Font_dblsizeh = dest ;
////	i = (sizeof(RomFont_dblsizeh) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)RomFont_dblsizeh, 24*32/2, i ) ;
//////	memmove( dest, RomFont_dblsizeh , i ) ;
////	dest += i ;

////	Font5x7h = dest ;
////	i = (sizeof(RomFont5x7h) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)RomFont5x7h, 12*16/2, i ) ;
//////	memmove( dest, RomFont5x7h , i ) ;
////	dest += i ;

////	Font5x7hBold = dest ;
////	i = (sizeof(RomFont5x7hBold) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)RomFont5x7hBold, 12*16/2, i ) ;
//////	memmove( dest, RomFont5x7hBold , i ) ;
////	dest += i ;

////	font_fr_h_extra = dest ;
////	i = (sizeof(Romfont_fr_h_extra) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)Romfont_fr_h_extra, 12*16/2, i ) ;
//////	memmove( dest, Romfont_fr_h_extra, i ) ;
////	dest += i ;

////	font_fr_h_big_extra = dest ;
////	i = (sizeof(Romfont_fr_h_big_extra) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)Romfont_fr_h_big_extra, 24*32/2, i ) ;
//////	memmove( dest, Romfont_fr_h_big_extra, i ) ;
////	dest += i ;

////	font_de_h_extra = dest ;
////	i = (sizeof(Romfont_de_h_extra) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)Romfont_de_h_extra, 12*16/2, i ) ;
//////	memmove( dest, Romfont_de_h_extra, i ) ;
////	dest += i ;

////	font_de_h_big_extra = dest ;
////	i = (sizeof(Romfont_de_h_big_extra) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)Romfont_de_h_big_extra, 24*32/2, i ) ;
//////	memmove( dest, Romfont_de_h_big_extra, i ) ;
////	dest += i ;

////	font_se_h_extra = dest ;
////	i = (sizeof(Romfont_se_h_extra) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)Romfont_se_h_extra, 12*16/2, i ) ;
//////	memmove( dest, Romfont_se_h_extra, i ) ;
////	dest += i ;

////	font_se_h_big_extra = dest ;
////	i = (sizeof(Romfont_se_h_big_extra) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)Romfont_se_h_big_extra, 24*32/2, i ) ;
//////	memmove( dest, Romfont_se_h_big_extra, i ) ;
////	dest += i ;

////	font_it_h_extra = dest ;
////	i = (sizeof(Romfont_it_h_extra) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)Romfont_it_h_extra, 12*16/2, i ) ;
//////	memmove( dest, Romfont_it_h_extra, i ) ;
////	dest += i ;

////	font_pl_h_extra = dest ;
////	i = (sizeof(Romfont_pl_h_extra) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)Romfont_pl_h_extra, 12*16/2, i ) ;
//////	memmove( dest, Romfont_pl_h_extra, i ) ;
////	dest += i ;
	
////	font_xxl = dest ;
////	i = (sizeof(Romfont_xxl) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)Romfont_xxl, 48*32/2, i ) ;
//////	memmove( dest, Romfont_pl_h_extra, i ) ;
////	dest += i ;

////	FontSmall = dest ;
////	i = (sizeof(RomFontSmall) + 3) & ~3  ;
////	invertFont( dest, (uint8_t *)RomFontSmall, 12*16/2, i ) ;
////	dest += i ;

//	return dest ;
//}

#else
//const uint8_t FontSmall[] =
//{
//#include "font.lbm"
//} ;
//const uint8_t Font5x7h[] =
//{
//#include "..\f7x5.lbm"
////#include "..\font5x7h.lbm"
//} ;

//const uint8_t Font5x7hBold[] =
//{
//#include "..\font5x7hBold.lbm"
//} ;

//const uint8_t Font_dblsizeh[] =
//{
//#include "..\font_dblsizeh.lbm"
//} ;

//const uint8_t Font_12x8h[] =
//{
//#include "..\font12x8h.lbm"
////#include "..\F12x8.lbm"
//} ;


//const uint8_t font_fr_h_extra[] =
//{
//#include "..\font5x7Frh.lbm"
//} ;
//const uint8_t font_fr_h_big_extra[] =
//{
//#include "..\font_fr_10x14h.lbm"
//} ;
//const uint8_t font_de_h_extra[] =
//{
//#include "..\font5x7Deh.lbm"
//} ;
//const uint8_t font_de_h_big_extra[] =
//{
//#include "..\font_de_10x14h.lbm"
//} ;
//const uint8_t font_se_h_extra[] =
//{
//#include "..\font5x7Seh.lbm"
//} ;
//const uint8_t font_se_h_big_extra[] =
//{
//#include "..\font_se_10x14h.lbm"
//} ;
//const uint8_t font_it_h_extra[] =
//{
//#include "..\font5x7Ith.lbm"
//} ;
//const uint8_t font_pl_h_extra[] =
//{
//#include "..\font5x7Plh.lbm"
//} ;
//const uint8_t font_xxl[] =
//{
//#include "..\fontxxl.lbm"
//} ;




#endif

uint8_t *FontStd ;
uint8_t *FontMid ;
uint8_t *FontDbl ;
uint8_t *FontSml ;
uint8_t *FontTin ;
uint8_t *FontXxl ;
uint8_t *FontBold ;

// Modes
// Bit 0 - INVERS
// Bit 0 = 0 - black on white
// Bit 0 = 1 - white on black
// Bit 1 - BLINK
// Bit 7 - BOLD
// Bit 7 = 0 - Normal
// Bit 7 = 1 - Bold
// Bit 3 - CONDENSED
// Bit 3 = 0 - Normal
// Bit 3 = 1 - Condensed

//#define LCD_RED			0xF800
//#define LCD_GREEN		0x07E0
//#define LCD_BLUE		0x001F
//#define LCD_WHITE		0xFFFF
//#define LCD_BLACK		0
//#define LCD_GREY		0x8410
//#define LCD_STATUS_GREY	0x3BEF

// Coordinates are full 480 x 272
//void lcdDrawChDma( uint16_t x, uint16_t y, uint32_t w, uint32_t h, uint32_t mode, uint16_t colour, uint8_t *bitmap )
//{
//	uint32_t frontColour ;

//	frontColour = ( (colour & 0xF800) << 8 ) | ( (colour & 0xE000) << 3 ) ;
//	frontColour |= ( (colour & 0x07E0) << 5 ) | ( (colour & 0x0600) >> 1 ) ;
//	frontColour |= ( (colour & 0x001F) << 3 ) | ( (colour & 0x001C) >> 2 ) | 0xFF000000 ;

//#ifdef INVERT_DISPLAY
//	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ;
//#else
//	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ; // + (condensed ? 4 : 0) ;
//#endif

//	DMA2D_DeInit() ;
//	DMA2D->CR = (DMA2D->CR & (uint32_t)CR_MASK) | DMA2D_M2M_BLEND ;
//	DMA2D->OPFCCR = (DMA2D->OPFCCR & ~(uint32_t)DMA2D_OPFCCR_CM ) | DMA2D_RGB565 ;
//  DMA2D->OMAR = addr ;
//	DMA2D->OOR = (DMA2D->OOR & ~(uint32_t)DMA2D_OOR_LO ) | (LCD_W - w) ;
//  DMA2D->NLR = (DMA2D->NLR & ~(DMA2D_NLR_NL | DMA2D_NLR_PL) ) | ( h | (w << 16) ) ;

//	DMA2D->FGCLUT[0] = ( mode & INVERS ) ? frontColour : 0 ;
//	DMA2D->FGCLUT[15] = ( mode & INVERS ) ? 0 : frontColour ;
  
//  DMA2D->FGPFCCR = CM_L4 ;
//  DMA2D->FGMAR   = (uint32_t) bitmap ;
//  DMA2D->BGPFCCR = CM_RGB565 ;
//  DMA2D->BGMAR   = (uint32_t) addr ;
//	DMA2D->BGOR = (DMA2D->BGOR & ~(uint32_t)DMA2D_BGOR_LO ) | (LCD_W - w) ;
//  DMA2D->FGOR    = 0 ;
	
//  DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
//	DMA2D->CR |= (uint32_t)DMA2D_CR_START ;
//	pollDma2Ddone( 3000 ) ;
//}

//void lcdDrawCharBitmapDoubleDma( uint16_t x, uint16_t y, uint8_t chr, uint32_t mode, uint16_t colour )
//{
//	uint32_t condensed = mode & CONDENSED ;
//  uint8_t *q ;

//#ifdef INVERT_DISPLAY
//	x = (LCD_W-1) - x - ( condensed ? 15 : 23 ) ;
//	if ( x >= LCD_W )
//	{
//		x += LCD_W ;
//	}
//	y = (LCD_H-1) - y - 31 ;
//	if ( y >= LCD_H )
//	{
//		y = 0 ;
//	}
//#endif

////	w = 12 ;
//	if ( condensed )
//	{
//		if( chr < 0xC0 )
//		{
//			q = (uint8_t *) &Font_12x8h[(chr-0x20)*256] ;
//		}
//		else
//		{
//			q = (uint8_t *) &Font_12x8h[0] ;
//		}
//	}
//	else
//	{
//		if( chr < 0xC0 )
//		{
//			q = (uint8_t *) &Font_dblsizeh[(chr-0x20)*384] ;
//		}
//		else
//		{
//			if ( ExtraHorusBigFont )
//			{
//				q = (uint8_t *) &ExtraHorusBigFont[(chr-0xC0)*384] ;
//			}
//			else
//			{
//				q = (uint8_t *) &Font_dblsizeh[0] ;
//			}
//		}
//	}
  
//	lcdDrawChDma( x, y, condensed ? 16 : 24, 32, mode, colour, q ) ;
	
	
////	DMA2D_DeInit() ;
////	DMA2D->CR = (DMA2D->CR & (uint32_t)CR_MASK) | DMA2D_M2M_BLEND ;
////	DMA2D->OPFCCR = (DMA2D->OPFCCR & ~(uint32_t)DMA2D_OPFCCR_CM ) | DMA2D_RGB565 ;
////  DMA2D->OMAR = addr ;
////	DMA2D->OOR = (DMA2D->OOR & ~(uint32_t)DMA2D_OOR_LO ) | (LCD_W - w) ;
////  DMA2D->NLR = (DMA2D->NLR & ~(DMA2D_NLR_NL | DMA2D_NLR_PL) ) | ( 32 | (w << 16) ) ;

////	DMA2D->FGCLUT[0] = ( mode & INVERS ) ? frontColour | 0xFF000000 : backColour ;
////	DMA2D->FGCLUT[15] = ( mode & INVERS ) ? backColour : frontColour | 0xFF000000 ;
  
////  DMA2D->FGPFCCR = CM_L4 ;
////  DMA2D->FGMAR   = (uint32_t) q ;
////  DMA2D->BGPFCCR = CM_RGB565 ;
////  DMA2D->BGMAR   = (uint32_t) addr ;
////	DMA2D->BGOR = (DMA2D->BGOR & ~(uint32_t)DMA2D_BGOR_LO ) | (LCD_W - w) ;
  
////	DMA2D->FGOR    = 0 ;
	
////  DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
//////  DMA2D_StartTransfer() ;
////	DMA2D->CR |= (uint32_t)DMA2D_CR_START ;
////	pollDma2Ddone( 3000 ) ;
//////	if ( condensed )
//////	{
//////  	DMA2D->NLR = (DMA2D->NLR & ~(DMA2D_NLR_NL | DMA2D_NLR_PL) ) | ( 16 | (8 << 16) ) ;
//////	  DMA2D->OMAR = addr + 4 ;
//////		DMA2D->OOR = (DMA2D->OOR & ~(uint32_t)DMA2D_OOR_LO ) | (LCD_W - 8) ;
//////  	DMA2D->FGMAR   = ( (uint32_t) q ) + 2 ;
//////	  DMA2D->FGOR    = 4 ;
//////		DMA2D->IFCR = DMA2D_FLAG_TC ;
//////	  DMA2D_StartTransfer() ;
//////  	while (DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET);
//////	}
//}


//void lcdDrawCharBitmapDma( uint16_t x, uint16_t y, uint8_t chr, uint32_t mode, uint16_t colour )
//{
//	uint32_t condensed = mode & CONDENSED ;
//  uint8_t *q ;

//#ifdef INVERT_DISPLAY
//	x = (LCD_W-1) - x - 11 ;
//	if ( x >= LCD_W )
//	{
//		x += LCD_W ;
//	}
//	y = (LCD_H-1) - y - 15 ;
//	if ( y >= LCD_H )
//	{
//		y = 0 ;
//	}
//#endif

//	if ( condensed )
//	{
//		if( chr < 0xC0 )
//		{
//			if ( (chr >= 'A') && (chr <= 'F') )
//			{
//				chr += '\203'-'A' ;
//			}
//			q = (uint8_t *) &Font5x7h[(chr-0x20)*96] ;
//		}
//		else
//		{
//			if ( ExtraHorusFont )
//			{
//				q = (uint8_t *) &ExtraHorusFont[(chr-0xC0)*96] ;
//			}
//			else
//			{
//				q = (uint8_t *) &Font5x7h[0] ;
//			}
//		}
//	}
//	else
//	{
//		if( chr < 0xC0 )
//		{
//			q = (mode & BOLD) ? (uint8_t *) &Font5x7hBold[(chr-0x20)*96] : (uint8_t *) &Font5x7h[(chr-0x20)*96] ;
//		}
//		else
//		{
//			if ( mode & BOLD )
//			{
//				q = (uint8_t *) &Font5x7hBold[0] ;
//			}
//			else
//			{
//				if ( ExtraHorusFont )
//				{
//					q = (uint8_t *) &ExtraHorusFont[(chr-0xC0)*96] ;
//				}
//				else
//				{
//					q = (uint8_t *) &Font5x7h[0] ;
//				}
//			}
//		}
//	}

//	lcdDrawChDma( x, y, 12, 16, mode, colour, q ) ;


////#ifdef INVERT_DISPLAY
//////	uint16_t ty ;
//////	uint16_t tx ;
//////	uint16_t tc ;
//////	ty = y ;
//////	tx = x ;
//////	tc = chr ;
////	x = (LCD_W-1) - x - 11 ;
////	if ( x >= LCD_W )
////	{
////		x += LCD_W ;
////	}
////	y = (LCD_H-1) - y - 15 ;
////	if ( y >= LCD_H )
////	{
//////		LcdBits |= 4 ;
//////		LcdErrors += 1 ;
//////		Lcdx = tx ;
//////		Lcdy = ty ;
//////		Lcdc = tc ;
////		y = 0 ;
////	}
////#endif
////	uint32_t backColour ;
//////	switch ( background )
//////	{
//////		case LCD_WHITE :
//////			backColour = 0xFFFFFFFF ;
//////		 break ;
//////		case LCD_LIGHT_GREY :
//////		default :
//////			backColour = 0xFFC0C0C0 ; //0xFFFFFFFF ;
//////		break ;
//////		case LCD_STATUS_GREY :
//////			backColour = 0xFF397D7B ;
//////		break ;
//////		case LCD_GREEN :
//////			backColour = 0xFF00FF00 ;
//////		break ;
//////		case LCD_RED :
//////			backColour = 0xFFFF0000 ;
//////		break ;
//////		case LCD_BLUE :
//////			backColour = 0xFF0000FF ;
//////		break ;
//////	}

////	backColour = ( (background & 0xF800) << 8 ) | ( (background & 0xE000) << 3 ) ;
////	backColour |= ( (background & 0x07E0) << 5 ) | ( (background & 0x0600) >> 1 ) ;
////	backColour |= ( (background & 0x001F) << 3 ) | ( (background & 0x001C) >> 2 ) ;

////	uint32_t frontColour ;
////	frontColour = ( (colour & 0xF800) << 8 ) | ( (colour & 0xE000) << 3 ) ;
////	frontColour |= ( (colour & 0x07E0) << 5 ) | ( (colour & 0x0600) >> 1 ) ;
////	frontColour |= ( (colour & 0x001F) << 3 ) | ( (colour & 0x001C) >> 2 ) ;

////#ifdef INVERT_DISPLAY
////	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ;
////#else
////	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ; // + (condensed ? 4 : 0) ;
////#endif
//////#ifdef INVERT_DISPLAY
//////	uint32_t w = condensed ? 8 : 12 ;
//////#else
//////	uint32_t w = condensed ? 2 : 12 ;
//////#endif
////	uint32_t w = 12 ;

//////	q = (mode & BOLD) ? (uint8_t *) &Font5x7hBold[(chr-0x20)*96] : (uint8_t *) &Font5x7h[(chr-0x20)*96] ;
////	DMA2D_DeInit() ;
////	DMA2D->CR = (DMA2D->CR & (uint32_t)CR_MASK) | DMA2D_M2M_BLEND ;
////	DMA2D->OPFCCR = (DMA2D->OPFCCR & ~(uint32_t)DMA2D_OPFCCR_CM ) | DMA2D_RGB565 ;
////  DMA2D->OMAR = addr ;
////	DMA2D->OOR = (DMA2D->OOR & ~(uint32_t)DMA2D_OOR_LO ) | (LCD_W - w) ;
////  DMA2D->NLR = (DMA2D->NLR & ~(DMA2D_NLR_NL | DMA2D_NLR_PL) ) | ( 16 | (w << 16) ) ;

////	DMA2D->FGCLUT[0] = ( mode & INVERS ) ? frontColour | 0xFF000000 : backColour ;
////	DMA2D->FGCLUT[15] = ( mode & INVERS ) ? backColour : frontColour | 0xFF000000 ;
  
////  DMA2D->FGPFCCR = CM_L4 ;
////  DMA2D->FGMAR   = (uint32_t) q ;
////  DMA2D->BGPFCCR = CM_RGB565 ;
////  DMA2D->BGMAR   = (uint32_t) addr ;
////	DMA2D->BGOR = (DMA2D->BGOR & ~(uint32_t)DMA2D_BGOR_LO ) | (LCD_W - w) ;

//////#ifdef INVERT_DISPLAY
//////  DMA2D->FGOR    = condensed ? 4 : 0 ;
//////#else
//////  DMA2D->FGOR    = condensed ? 10 : 0 ;
//////#endif
////  DMA2D->FGOR    = 0 ;
	
////  DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
//////  DMA2D_StartTransfer() ;
////	DMA2D->CR |= (uint32_t)DMA2D_CR_START ;
////	pollDma2Ddone( 3000 ) ;
//////	if ( condensed )
//////	{
//////#ifdef INVERT_DISPLAY
//////  	DMA2D->NLR = (DMA2D->NLR & ~(DMA2D_NLR_NL | DMA2D_NLR_PL) ) | ( 16 | (2 << 16) ) ;
//////	  DMA2D->OMAR = addr + 16 ;
//////		DMA2D->OOR = (DMA2D->OOR & ~(uint32_t)DMA2D_OOR_LO ) | (LCD_W - 2) ;
//////  	DMA2D->FGMAR   = ( (uint32_t) q ) + 5 ;
//////	  DMA2D->FGOR    = 10 ;
//////#else
//////  	DMA2D->NLR = (DMA2D->NLR & ~(DMA2D_NLR_NL | DMA2D_NLR_PL) ) | ( 16 | (8 << 16) ) ;
//////	  DMA2D->OMAR = addr + 4 ;
//////		DMA2D->OOR = (DMA2D->OOR & ~(uint32_t)DMA2D_OOR_LO ) | (LCD_W - 8) ;
//////  	DMA2D->FGMAR   = ( (uint32_t) q ) + 2 ;
//////	  DMA2D->FGOR    = 4 ;
//////#endif
//////  	DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
////////	  DMA2D_StartTransfer() ;
//////	DMA2D->CR |= (uint32_t)DMA2D_CR_START ;
//////		pollDma2Ddone( 3000 ) ;
//////	}
//}

//void lcdDrawCharBitmapTransparent( uint16_t x, uint16_t y, uint8_t chr, uint32_t mode, uint16_t colour )
//{
//  uint8_t *q ;
//#ifdef INVERT_DISPLAY
//	x = (LCD_W-1) - x - 11 ;
//	if ( x >= LCD_W )
//	{
//		x += LCD_W ;
//	}
//	y = (LCD_H-1) - y - 15 ;
//	if ( y >= LCD_H )
//	{
//		y = 0 ;
//	}
//#endif

//#ifdef INVERT_DISPLAY
//	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ;
//#else
//	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ; // + (condensed ? 4 : 0) ;
//#endif
////	uint32_t w = 12 ;
	
//	if( chr < 0xC0 )
//	{
//		q = (mode & BOLD) ? (uint8_t *) &Font5x7hBold[(chr-0x20)*96] : (uint8_t *) &Font5x7h[(chr-0x20)*96] ;
//	}
//	else
//	{
//		if ( mode & BOLD )
//		{
//			q = (uint8_t *) &Font5x7hBold[0] ;
//		}
//		else
//		{
//			if ( ExtraHorusFont )
//			{
//				q = (uint8_t *) &ExtraHorusFont[(chr-0xC0)*96] ;
//			}
//			else
//			{
//				q = (uint8_t *) &Font5x7h[0] ;
//			}
//		}
//	}
//	uint32_t i ;
//	uint32_t j ;
//	uint16_t *p ;
//	uint8_t c ;
//	for ( i = 0 ; i < 16 ; i += 1 )
//	{
//		p = (uint16_t *) addr ;
//		for ( j = 0 ; j < 6 ; j += 1 )
//		{
//			c = *q++ ;
//			if ( c & 0x0F )
//			{
//				*p = colour ;
//			}
//			p += 1 ;
//			if ( c & 0xF0 )
//			{
//				*p = colour ;
//			}
//			p += 1 ;
//		}
//		addr += LCD_W * 2 ;
//	}
//}

//void lcdDrawCharSmall( uint16_t x, uint16_t y, uint8_t chr, uint32_t mode, uint16_t colour )
//{
//  uint8_t *q ;
//#ifdef INVERT_DISPLAY
//	x = (LCD_W-1) - x - 5 ;
//	if ( x >= LCD_W )
//	{
//		x += LCD_W ;
//	}
//	y = (LCD_H-1) - y - 7 ;
//	if ( y >= LCD_H )
//	{
//		y = 0 ;
//	}
//#endif

//#ifdef INVERT_DISPLAY
//	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 + 8 ;
//#else
//	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ; // + (condensed ? 4 : 0) ;
//#endif

//////	uint32_t w = 12 ;
	
//	if( chr < 0xC0 )
//	{
//		q = (uint8_t *) &font[(chr-0x20)*5] ;
//	}
//	else
//	{
//		q = (uint8_t *) &font[0] ;
//	}

//	uint32_t i ;
//	uint32_t j ;
//	uint32_t mask ;
//	uint16_t *p ;
//	uint8_t c ;
//	for ( i = 0 ; i < 5 ; i += 1 )
//	{
//#ifdef INVERT_DISPLAY
//		mask = 0x80 ;
//#else
//		mask = 0x01 ;
//#endif
//		p = (uint16_t *) addr ;
//		c = *q++ ;
//		for ( j = 0 ; j < 8 ; j += 1 )
//		{
//			if ( c & mask )
//			{
//				*p = colour ;
//			}
//			p += LCD_W ;
//#ifdef INVERT_DISPLAY
//			mask >>= 1 ;
//#else
//			mask <<= 1 ;
//#endif

//		}
//#ifdef INVERT_DISPLAY
//		addr -= 2 ;
//#else
//		addr += 2 ;
//#endif
//	}
//}

//void lcd_putsSmall( uint16_t x, uint16_t y, uint8_t *p, uint16_t colour )
//{
//	uint8_t c ;
//	while ( ( c = *p++ ) != 0 )
//	{
//		lcdDrawCharSmall( x, y, c, 0, colour ) ;
//		x += 6 ;		
//	}
//}

//void lcd_putsnSmall( uint16_t x, uint16_t y, uint8_t *p, uint8_t len, uint16_t colour )
//{
//	uint8_t c ;
//  while(len!=0)
//	{
//		c = *p++ ;
//		if ( c == 0 )
//		{
//			break ;
//		}
//		lcdDrawCharSmall( x, y, c, 0, colour ) ;
//		x += 6 ;		
//    len -= 1 ;
//	}
//}

//void lcdDrawCharxxlTransparent( uint16_t x, uint16_t y, uint8_t chr, uint32_t mode, uint16_t colour )
//{
//  uint8_t *q ;
//#ifdef INVERT_DISPLAY
//	x = (LCD_W-1) - x - 31 ;
//	if ( x >= LCD_W )
//	{
//		x += LCD_W ;
//	}
//	y = (LCD_H-1) - y - 47 ;
//	if ( y >= LCD_H )
//	{
//		y = 0 ;
//	}
//#endif

////#ifdef INVERT_DISPLAY
////	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ;
////#else
////	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ; // + (condensed ? 4 : 0) ;
////#endif
////	uint32_t w = 12 ;

//	if ( chr == '-' )
//	{
//		chr = ':' + 1 ;
//	}
//	if ( ( chr < '0' ) || ( chr > ':' + 1 ) )
//	{
//		lcdDrawCharBitmapDma( x, y, chr, mode, colour ) ;
//		return ;
//	}

//	q = (uint8_t *) &font_xxl[(chr-'0')*16*48] ;

//	lcdDrawChDma( x, y, 32, 48, mode, colour, q ) ;
	
////	uint32_t i ;
////	uint32_t j ;
////	uint16_t *p ;
////	uint8_t c ;
////	for ( i = 0 ; i < 48 ; i += 1 )
////	{
////		p = (uint16_t *) addr ;
////		for ( j = 0 ; j < 8 ; j += 1 )
////		{
////			c = *q++ ;
////			if ( c & 0x0F )
////			{
////				*p = colour ;
////			}
////			p += 1 ;
////			if ( c & 0xF0 )
////			{
////				*p = colour ;
////			}
////			p += 1 ;
////			c = *q++ ;
////			if ( c & 0x0F )
////			{
////				*p = colour ;
////			}
////			p += 1 ;
////			if ( c & 0xF0 )
////			{
////				*p = colour ;
////			}
////			p += 1 ;
////		}
////		addr += LCD_W * 2 ;
////	}
//}

//void lcdDrawCharDoubleTransparent( uint16_t x, uint16_t y, uint8_t chr, uint32_t mode, uint16_t colour )
//{
////	uint32_t backColour ;
////	uint32_t frontColour ;
//	uint32_t condensed = mode & CONDENSED ;

//#ifdef INVERT_DISPLAY
//	x = (LCD_W-1) - x - ( condensed ? 15 : 23 ) ;
//	if ( x >= LCD_W )
//	{
//		x += LCD_W ;
//	}
//	y = (LCD_H-1) - y - 31 ;
//	if ( y >= LCD_H )
//	{
////		LcdBits |= 2 ;
////		LcdErrors += 1 ;
//		y = 0 ;
//	}
//#endif

////	frontColour = ( (colour & 0xF800) << 8 ) | ( (colour & 0xE000) << 3 ) ;
////	frontColour |= ( (colour & 0x07E0) << 5 ) | ( (colour & 0x0600) >> 1 ) ;
////	frontColour |= ( (colour & 0x001F) << 3 ) | ( (colour & 0x001C) >> 2 ) ;
	
//  uint8_t *q ;
//	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ;
//	uint32_t w = condensed ? 16 : 24 ;
////	w = 12 ;
//	if ( condensed )
//	{
//		if( chr < 0xC0 )
//		{
//			q = (uint8_t *) &Font_12x8h[(chr-0x20)*256] ;
//		}
//		else
//		{
//			q = (uint8_t *) &Font_12x8h[0] ;
//		}
//	}
//	else
//	{
//		if( chr < 0xC0 )
//		{
//			q = (uint8_t *) &Font_dblsizeh[(chr-0x20)*384] ;
//		}
//		else
//		{
//			if ( ExtraHorusBigFont )
//			{
//				q = (uint8_t *) &ExtraHorusBigFont[(chr-0xC0)*384] ;
//			}
//			else
//			{
//				q = (uint8_t *) &Font_dblsizeh[0] ;
//			}
//		}
//	}

//  // width w, height 32?
//	w /= 2 ;
//	uint32_t i ;
//	uint32_t j ;
//	uint16_t *p ;
//	uint8_t c ;
//	for ( i = 0 ; i < 32 ; i += 1 )
//	{
//		p = (uint16_t *) addr ;
//		for ( j = 0 ; j < w ; j += 1 )
//		{
//			c = *q++ ;
//			if ( c & 0x0F )
//			{
//				*p = colour ;
//			}
//			p += 1 ;
//			if ( c & 0xF0 )
//			{
//				*p = colour ;
//			}
//			p += 1 ;
//		}
//		addr += LCD_W * 2 ;
//	}
//}




void lcdDrawBitmapDMA(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t * bitmap, uint8_t type)
{
	uint8_t mode ;
//  if ((uint32_t(bitmap) & 0x03) != 0)
//    return;
#ifdef INVERT_DISPLAY
	x = (LCD_W-1) - x - (w - 1) ;
	if ( x >= LCD_W )
	{
		x += LCD_W ;
	}
	y = (LCD_H-1) - y - (h - 1) ;
	if ( y >= LCD_H )
	{
		y = 0 ;
	}
#endif

	if ( (type & 1) == 0 )
	{
		mode = CM_L4 ;
	}
	else
	{
		mode = CM_A4 ;
	}
  uint32_t addr = CurrentFrameBuffer + 2*(LCD_W*y + x) ;

  DMA2D_DeInit();

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M_PFC ;
//  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M_BLEND ;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = addr;
  DMA2D_InitStruct.DMA2D_OutputGreen = 0 ;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0 ;
  DMA2D_InitStruct.DMA2D_OutputRed = 0 ;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0 ;
  DMA2D_InitStruct.DMA2D_OutputOffset = (LCD_W - w);
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
//  /*!< Initialize the DMA2D foreground memory address */
//  DMA2D_FG_InitStruct->DMA2D_FGMA = 0x00;
//  /*!< Initialize the DMA2D foreground offset */
//  DMA2D_FG_InitStruct->DMA2D_FGO = 0x00;
//  /*!< Initialize the DMA2D foreground color mode */
//  DMA2D_FG_InitStruct->DMA2D_FGCM = CM_ARGB8888;
//  /*!< Initialize the DMA2D foreground CLUT color mode */
//  DMA2D_FG_InitStruct->DMA2D_FG_CLUT_CM = CLUT_CM_ARGB8888;
//  /*!< Initialize the DMA2D foreground CLUT size */
//  DMA2D_FG_InitStruct->DMA2D_FG_CLUT_SIZE = 0x00;
//  /*!< Initialize the DMA2D foreground alpha mode */
//  DMA2D_FG_InitStruct->DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
//  /*!< Initialize the DMA2D foreground alpha value */
//  DMA2D_FG_InitStruct->DMA2D_FGPFC_ALPHA_VALUE = 0x00;
//  /*!< Initialize the DMA2D foreground blue value */
//  DMA2D_FG_InitStruct->DMA2D_FGC_BLUE = 0x00;
//  /*!< Initialize the DMA2D foreground green value */
//  DMA2D_FG_InitStruct->DMA2D_FGC_GREEN = 0x00;
//  /*!< Initialize the DMA2D foreground red value */
//  DMA2D_FG_InitStruct->DMA2D_FGC_RED = 0x00;
//  /*!< Initialize the DMA2D foreground CLUT memory address */
//  DMA2D_FG_InitStruct->DMA2D_FGCMAR = 0x00;
  
	DMA2D_FG_InitStruct.DMA2D_FGC_RED = 0xFF ;
//	DMA2D_FG_InitStruct.DMA2D_FGC_BLUE = 0xFF ;
//	DMA2D_FG_InitStruct.DMA2D_FGC_GREEN = 0xFF ;
	
	DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(bitmap);
  DMA2D_FG_InitStruct.DMA2D_FGO = 0;
//  DMA2D_FG_InitStruct.DMA2D_FGCM = type ; //CM_RGB565;
  DMA2D_FG_InitStruct.DMA2D_FGCM = mode ;
//  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_RGB565 ;
  
//	DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = COMBINE_ALPHA_VALUE ;// NO_MODIF_ALPHA_VALUE ;
	DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE ;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0 ;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

//  DMA2D->BGCOLR = 0 ;
//  DMA2D->BGPFCCR = CM_L4 ;
//  DMA2D->BGPFCCR = (1 << 16) ;
//	DMA2D->BGMAR = DMA2D->OMAR ;
//	DMA2D->BGOR = DMA2D->OOR ;
//	DMA2D->BGOR = 0 ;

	DMA2D->FGCLUT[0] = 0xFFFFFFFF ;
	DMA2D->FGCLUT[15] = 0 ;
	DMA2D->BGCLUT[0] = 0xFFFFFFFF ;
	DMA2D->BGCLUT[15] = 0xFFFFFFFF ;

	DMA2D->FGCLUT[0] = 0 ;
	DMA2D->FGCLUT[15] = RGB32(255,255,255) ;

	if ( type & 0xC0 )
	{
		uint32_t backColour ;
		uint16_t background = LcdBackground ;
		if ( type & 0x40 )
		{
			background = LCD_STATUS_GREY ;
		}

		backColour = ( (background & 0xF800) << 8 ) | ( (background & 0xE000) << 3 ) ;
		backColour |= ( (background & 0x07E0) << 5 ) | ( (background & 0x0600) >> 1 ) ;
		backColour |= ( (background & 0x001F) << 3 ) | ( (background & 0x001C) >> 2 ) ;
		DMA2D->FGCLUT[15] = backColour ;
		
	}

//static int _DMA_DrawBitmapA4(void * pSrc, void * pDst,  U32 OffSrc, U32 OffDst, U32 PixelFormatDst, U32 xSize, U32 ySize) {
//  U8 * pRD;
//  U8 * pWR;
//  U32 NumBytes, Color, Index;

//  //
//  // Check size of conversion buffer
//  //
//  NumBytes = ((xSize + 1) & ~1) * ySize / 2 ;
//  if ((NumBytes > sizeof(_aBuffer)) || (NumBytes == 0)) {
//    return 1;
//  }
//  //
//  // Conversion (swapping nibbles)
//  //
//  pWR = (U8 *)_aBuffer;
//  pRD = (U8 *)pSrc;
//  do {
//    *pWR++ = _aMirror[*pRD++];
//  } while (--NumBytes);
//  //
//  // Get current drawing color (ABGR)
//  //
//  Index = LCD_GetColorIndex();
//  Color = LCD_Index2Color(Index);
//  //
//  // Set up operation mode
//  //
//  DMA2D->CR = 0x00020000UL;
//  //
//  // Set up source
//  //
//#if (GUI_USE_ARGB == 0)
//  DMA2D->FGCOLR  = ((Color & 0xFF) << 16)  // Red
//                 |  (Color & 0xFF00)       // Green
//                 | ((Color >> 16) & 0xFF); // Blue
//#else
//  DMA2D->FGCOLR  = Color;
//#endif
//  DMA2D->FGMAR   = (U32)_aBuffer;
//  DMA2D->FGOR    = 0;
//  DMA2D->FGPFCCR = 0xA;                    // A4 bitmap
//  DMA2D->NLR     = (U32)((xSize + OffSrc) << 16) | ySize;
//  DMA2D->BGMAR   = (U32)pDst;
//  DMA2D->BGOR    = OffDst - OffSrc;
//  DMA2D->BGPFCCR = PixelFormatDst;
//  DMA2D->OMAR    = DMA2D->BGMAR;
//  DMA2D->OOR     = DMA2D->BGOR;
//  DMA2D->OPFCCR  = DMA2D->BGPFCCR;
//  //
//  // Execute operation
//  //
//  _DMA_ExecOperation();
//  return 0;
//}













  /* Configures the FG memory address */
//  DMA2D->FGMAR = CONVERT_PTR_UINT(bitmap) ;

  /* Configures the FG offset */
//  DMA2D->FGOR &= ~(uint32_t)DMA2D_FGOR_LO;
//  DMA2D->FGOR |= 0 ;	// FGO = FG offset

  /* Configures foreground Pixel Format Convertor */
//  DMA2D->FGPFCCR &= (uint32_t)PFCCR_MASK;
//  fg_clutcolormode = CLUT_CM_ARGB8888 << 4;
//  fg_clutsize = 0 << 8;
//  fg_alpha_mode = NO_MODIF_ALPHA_VALUE << 16;
//  fg_alphavalue = 0 << 24;
//  DMA2D->FGPFCCR |= (CM_RGB565 | fg_clutcolormode | fg_clutsize | fg_alpha_mode | fg_alphavalue);

  /* Configures foreground color */
//  DMA2D->FGCOLR &= ~(DMA2D_FGCOLR_BLUE | DMA2D_FGCOLR_GREEN | DMA2D_FGCOLR_RED);
//  DMA2D->FGCOLR |= 0 ;

  /* Configures foreground CLUT memory address */
//  DMA2D->FGCMAR = 0 ;

  /* Start Transfer */
  DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
//  DMA2D_StartTransfer();
	DMA2D->CR |= (uint32_t)DMA2D_CR_START ;

  /* Wait for CTC Flag activation */
	pollDma2Ddone( 3000 ) ;
}

void lcdDrawIcon( uint16_t x, uint16_t y, const uint8_t * bitmap, uint8_t type )
{
	uint16_t w ;
	uint16_t h ;
	w = *bitmap++ ;
	h = *bitmap++ ;
	lcdDrawBitmapDMA( x, y, w, h, bitmap, type ) ;
}

void DMAcopyImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *image, uint16_t reduceWidth )
{
  DMA2D_DeInit();
	if ( reduceWidth )
	{
		w -= reduceWidth/2 ;
	}
#ifdef INVERT_DISPLAY
	x = (LCD_W-1) - x - (w - 1) ;
	if ( x >= LCD_W )
	{
		x += LCD_W ;
	}
	y = (LCD_H-1) - y - (h - 1) ;
	if ( y >= LCD_H )
	{
//		LcdBits |= 8 ;
//		LcdErrors += 1 ;
		y = 0 ;
	}
#endif
	if ( reduceWidth )
	{
		w -= reduceWidth/2 ;
	}
  uint32_t addr = CurrentFrameBuffer + 2*(LCD_W*y + x);

  DMA2D_InitTypeDef DMA2D_InitStruct;
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = addr ;
  DMA2D_InitStruct.DMA2D_OutputGreen = 0;
  DMA2D_InitStruct.DMA2D_OutputBlue = 0;
  DMA2D_InitStruct.DMA2D_OutputRed = 0;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
  DMA2D_InitStruct.DMA2D_OutputOffset = (LCD_W - w);
  DMA2D_InitStruct.DMA2D_NumberOfLine = h;
  DMA2D_InitStruct.DMA2D_PixelPerLine = w;
  DMA2D_Init(&DMA2D_InitStruct);

  DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
  DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
	if ( reduceWidth )
	{
		image += reduceWidth/2 ;
  	DMA2D_FG_InitStruct.DMA2D_FGO = reduceWidth ;
//  	DMA2D_FG_InitStruct.DMA2D_FGO = 0;
	}
	else
	{
  	DMA2D_FG_InitStruct.DMA2D_FGO = 0;
	}
  DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(image);
  DMA2D_FG_InitStruct.DMA2D_FGCM = CM_RGB565;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
  DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
  DMA2D_FGConfig(&DMA2D_FG_InitStruct);

  /* Start Transfer */
  DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
//	DMA2D_StartTransfer();
	DMA2D->CR |= (uint32_t)DMA2D_CR_START ;

  /* Wait for CTC Flag activation */
	pollDma2Ddone( 7000 ) ;
}




//void DMAcopy(void * src, void * dest, int len)
//{
//  DMA2D_DeInit();

//    DMA2D_InitTypeDef DMA2D_InitStruct;
//    DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M;
//    DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
//    DMA2D_InitStruct.DMA2D_OutputMemoryAdd = CONVERT_PTR_UINT(dest);
//    DMA2D_InitStruct.DMA2D_OutputGreen = 0;
//    DMA2D_InitStruct.DMA2D_OutputBlue = 0;
//    DMA2D_InitStruct.DMA2D_OutputRed = 0;
//    DMA2D_InitStruct.DMA2D_OutputAlpha = 0;
//    DMA2D_InitStruct.DMA2D_OutputOffset = 0;
//    DMA2D_InitStruct.DMA2D_NumberOfLine = LCD_H;
//    DMA2D_InitStruct.DMA2D_PixelPerLine = LCD_W;
//    DMA2D_Init(&DMA2D_InitStruct);

//    DMA2D_FG_InitTypeDef DMA2D_FG_InitStruct;
//    DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);
//    DMA2D_FG_InitStruct.DMA2D_FGMA = CONVERT_PTR_UINT(src);
//    DMA2D_FG_InitStruct.DMA2D_FGO = 0;
//    DMA2D_FG_InitStruct.DMA2D_FGCM = CM_RGB565;
//    DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_MODE = NO_MODIF_ALPHA_VALUE;
//    DMA2D_FG_InitStruct.DMA2D_FGPFC_ALPHA_VALUE = 0;
//    DMA2D_FGConfig(&DMA2D_FG_InitStruct);

//    /* Start Transfer */
//  	DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
////    DMA2D_StartTransfer();
//	DMA2D->CR |= (uint32_t)DMA2D_CR_START ;

//    /* Wait for CTC Flag activation */
//		pollDma2Ddone() ;
//}

//void lcdStoreBackupBuffer()
//{
//  uint16_t * src = (uint16_t *)CurrentFrameBuffer, * dest = (uint16_t *)LcdBackupFrameBuffer;
//  DMAcopy(src, dest, DISPLAY_BUFFER_SIZE);
//}

//int lcdRestoreBackupBuffer()
//{
//  uint16_t * dest = (uint16_t *)CurrentFrameBuffer, * src = (uint16_t *)LcdBackupFrameBuffer;
//  DMAcopy(src, dest, DISPLAY_BUFFER_SIZE);
//  return 1;
//}

void lcdRefresh()
{
//extern uint8_t ImageDisplay ;
//	if ( ImageDisplay )
//	{
//		putsTime( 187, 1*FH, Time.hour*60+Time.minute, 0, 0 ) ;
//		lcd_img( 171, 2*FH, speaker, 0, 0 ) ;
//extern uint8_t CurrentVolume ;
//		lcd_hbar( 176, 2*FH+1, 24, 6, CurrentVolume*100/23 ) ;
//		lcd_hline( 157, 31, 61 ) ;
//		lcd_vline( 156, 0, 64 ) ;
//	}
	
	hw_delay( 50 ) ;	// Make sure last DMA transfer has completed
  
	LCD_SetTransparency(255);
  if (CurrentLayer == LCD_FIRST_LAYER)
    LCD_SetLayer(LCD_SECOND_LAYER);
  else
    LCD_SetLayer(LCD_FIRST_LAYER);
  LCD_SetTransparency(0);
}

void backlightEnable(uint8_t dutyCycle)
{
//  if (dutyCycle < 5)
//	{
//    dutyCycle = 5;
//  }
  
#if defined(PCBX12D)
//  if (IS_HORUS_PROD())
//  if (0)
	if ( isProdVersion() )
//  if (GPIOI->IDR & 0x0800)
	{
    BL_TIMER->CCR4 = (100 - dutyCycle) ;
  }
  else
	{
    
		BL_TIMER->CCR4 = (100 - dutyCycle) ;
    
		BLP_TIMER->CCR1 = dutyCycle ;
  }
#elif defined(PCBX10)
  BL_TIMER->CCR3 = dutyCycle ;
#endif
}

uint16_t BacklightBrightness ;

void backlight_on()
{
	backlightEnable(BacklightBrightness) ;
}

void backlight_off()
{
	backlightEnable(g_eeGeneral.bright_white) ;
}

void backlight_set( uint16_t brightness )
{
	BacklightBrightness = brightness ;
	backlightEnable(BacklightBrightness) ;
}


// Temporary
//#include "ff.h"

////#include "font.lbm"
//FIL FontFile = {0} ;

//extern uint16_t lcd_putcAttDblColour(uint16_t x,uint16_t y,const char c,uint8_t mode, uint16_t colour, uint16_t background ) ;
//void convertFont()
//{
//// 16+16+64+3 = 99
//// Space to /
//// 0-9 to ?
//// @, A-Z, a-z, ...
//// ^, v, tick

//	uint8_t c[32] ;
////	uint8_t d[16] ;
//	uint16_t *p ;
//	uint32_t i ;
//	uint32_t j ;
//	uint32_t mask ;
//	uint32_t chrIndex ;
	
//  f_open(&FontFile, "\\fontLC", FA_OPEN_ALWAYS | FA_WRITE ) ;
	
//	for ( chrIndex = 0 ; chrIndex < 96 ; chrIndex += 1 )
//	{
//		lcdDrawSolidFilledRectDMA( 0, 0, 40, 40, LCD_WHITE ) ;
//		lcd_putcAttDblColour( 0, 0, chrIndex+0x20, DBLSIZE | CONDENSED, LCD_BLACK, LCD_WHITE ) ;
	
//		p = ( uint16_t *) CurrentFrameBuffer ;
//		for ( j = 0 ; j < 32 ; j += 1 )
//		{
//			for ( i = 0 ; i < 16 ; i += 2 )
//			{
//				mask = *(p+i) ;
//				c[i/2] = (mask == LCD_WHITE) ? 0 : 0x0F ;
//				mask = *(p+i+1) ;
//				c[i/2] |= (mask == LCD_WHITE) ? 0 : 0xF0 ;
//			}
//			for ( i = 0 ; i < 8 ; i += 1 )
//			{
//				f_printf(&FontFile, "0x%02x,", c[i] ) ;
//			}
//			if ( j == 0 )
//			{
//				f_printf( &FontFile, " // '%c'", chrIndex+32 ) ;
//			}
//			f_printf(&FontFile, "\n" ) ;
//			p += LCD_W ;
//		}
//    wdt_reset();  // Retrigger hardware watchdog
//  }
//  f_close(&FontFile) ;
//}

uint16_t lcd_getPixel(uint32_t x, uint32_t y )
{
#ifdef INVERT_DISPLAY
	x = (LCD_W-x)-1 ;
	y = (LCD_H-y)-1 ;
#endif	
	uint32_t pbuffer = (uint32_t)LcdFirstFrameBuffer ;
	if ( CurrentFrameBuffer == (uint32_t)LcdFirstFrameBuffer )
	{
		pbuffer = (uint32_t)LcdSecondFrameBuffer ;
	}

//  uint16_t *addr = (uint16_t *)(CurrentFrameBuffer + 2*(LCD_W*y + x)) ;
  uint16_t *addr = (uint16_t *)(pbuffer + 2*(LCD_W*y + x)) ;
//  uint16_t *addr = (uint16_t *)(CurrentFrameBuffer + 2*(LCD_W*y + x)) ;
	return *addr ;
}

void lcd_setPixel(uint32_t x, uint32_t y, uint16_t value )
{
#ifdef INVERT_DISPLAY
	x = (LCD_W-x)-1 ;
	y = (LCD_H-y)-1 ;
#endif	
	uint32_t pbuffer = (uint32_t)LcdFirstFrameBuffer ;
	if ( CurrentFrameBuffer == (uint32_t)LcdFirstFrameBuffer )
	{
		pbuffer = (uint32_t)LcdSecondFrameBuffer ;
	}

//  uint16_t *addr = (uint16_t *)(CurrentFrameBuffer + 2*(LCD_W*y + x)) ;
  uint16_t *addr = (uint16_t *)(pbuffer + 2*(LCD_W*y + x)) ;
	*addr = value ;
}

uint8_t FontLangOffsetStd[] = { 0, 19, 24, 73, 73, 79, 81, 0, 61 } ;
uint8_t FontLangOffsetDbl[] = { 0, 0, 5, 54, 54, 60, 62, 0, 42 } ;

//uint8_t getMappedChar(uint8_t c)
//{
//  uint8_t result;
//  result = 0 ;
//  if (c)
//	{
//		if ( ( c >= 0x80) && ( c < 0xC0 ) )
//		{
//			switch ( g_eeGeneral.language )
//			{
//				case L_FRENCH :
//			    result = 119 + c - 0x80;
//				break ;
//				case L_GERMAN :
//    			result = 124 + c - 0x80;
//				break ;
//				case L_NORWEGIAN :
//				case L_SWEDISH :
//    			result = 173 + c - 0x80;
//				break ;
//				case L_ITALIAN :
//    			result = 179 + c - 0x80;
//				break ;
//				case L_POLISH :
//    			result = 181 + c - 0x80;
//				break ;
//				case L_SPANISH :
//    			result = 161 + c - 0x80;
//				break ;
//			}
//		}	
//	  else if (c < 0xC0)
//		{
//	    result = c - 0x20 ;
//		}	
//	  else
//		{
////	    result = c - 0xC0 + 96 ;
//	    result = 0 ;
//		}	
//	}
//  return result;
//}



//uint32_t rle_decode_8bit(unsigned char* dest, unsigned int dest_size, const unsigned char* src)
//{
//  //unsigned char count = 0;
//  unsigned char prev_byte = 0;
//  bool prev_byte_valid = false;

//  const unsigned char* dest_end = dest + dest_size;
//  unsigned char* d = dest;

//  while(d < dest_end)
//	{
        
//    *d++ = *src;
//    if (prev_byte_valid && (*src == prev_byte))
//		{

//      src++;

//      if (d + *src > dest + dest_size)
//			{
//        return -1;
//      }

//      memset(d, prev_byte, *src);
//      d += *src++;
//      prev_byte_valid = false;
//    }
//    else
//		{
//      prev_byte = *src++;
//      prev_byte_valid = true;
//    }
//    wdt_reset();  // Retrigger hardware watchdog
//  }

//  return d - dest ;
//}


//static uint32_t decompressFont(const uint8_t* font)
//{
//  uint16_t width = *((uint16_t *)font);
//  uint16_t height = *(((uint16_t *)font)+1);

//  size_t font_size = width * height;

//  uint8_t *dec_buf = (uint8_t*)0xD0600000 ;

//  // copy width / height
//  memcpy(dec_buf,font,4);

//  return rle_decode_8bit(dec_buf+4, font_size, font+4);
//}

//FIL FontFile = {0} ;
//#include "ff.h"

//void sortFont( uint8_t *srcFont, uint16_t *specs, uint16_t lastSpec, char *fontName )
//{
//	uint32_t size ;
//	uint32_t i ;
//	uint32_t j ;
//	uint32_t k ;
//  uint8_t *dec_buf = (uint8_t*)0xD0600000 ;
//  uint8_t *out_buf = (uint8_t*)0xD0400000 ;
//	uint16_t index ;
//	uint16_t offset ;
//	uint16_t width ;
//  uint16_t ww = *((uint16_t *)srcFont) ;
//  uint16_t height = *(((uint16_t *)srcFont)+1);
//	FRESULT result ;

//	size = decompressFont( srcFont ) + 4 ;
//	j = 4 ;

//  dec_buf = (uint8_t*)0xD0600000 ;
//  out_buf = (uint8_t*)0xD0400000 ;

//  memcpy(out_buf,dec_buf,4) ;

//	index = 0 ;
//	dec_buf += 4 ;
//	out_buf += 4 ;

//	do
//	{
//		offset = specs[index] ;
//		width = specs[index+1] - offset ;
//		index += 1 ;

//  	for ( i = 0 ; i < height ; i += 1 )	// rows
//		{
//	    const uint8_t *q = dec_buf + i*ww + offset ;
//    	for ( j = 0 ; j < width ; j += 1 )
//			{
//				*out_buf++ = *q++ ;
//			}
//	    wdt_reset();  // Retrigger hardware watchdog
//		}
//		offset = specs[index] ;
//	} while ( offset < lastSpec ) ;


//	j = 4 ;
//  out_buf = (uint8_t*)0xD0400000 ;
////	out_buf += 4 ;
		 
//	for ( i = 4 ; i < size ; i += 2 )
//	{
//		uint8_t byte ;
//		byte = out_buf[i+1] ;
//		byte |= (out_buf[i] << 4 ) ;
//		out_buf[j] = byte ;
//		j += 1 ;
//	}

//	size = size / 2 + 2 ;

////  UINT written ;

//  result = f_open(&FontFile, fontName, FA_OPEN_ALWAYS | FA_WRITE) ;
//	(void)result ;
  
//	out_buf = (uint8_t*)0xD0400000 ;

//	index = 0 ;
//	offset = specs[index] ;
//	width = specs[index+1] - offset ;
//	k = width * height / 2 ;
	
//	for ( i = 0 ; i < 4 ; i += 1 )
//	{
//		f_printf(&FontFile, "0x%02X,", out_buf[i] ) ;
//	}
//	f_puts("\n", &FontFile) ;
//	j = 0 ;

//	for ( i = 4 ; i < size ; i += 1 )
////	for ( i = 4 ; i < 1004 ; i += 1 )
//	{
//		f_printf(&FontFile, "0x%02X,", out_buf[i] ) ;
//	  wdt_reset();  // Retrigger hardware watchdog
//		j += 1 ;
//		if ( j == k )
//		{
//			j = 0 ;
//			f_puts("\n", &FontFile) ;
//			index += 1 ;
//			offset = specs[index] ;
//			width = specs[index+1] - offset ;
//			k = width * height / 2 ;
//		}
//	}
//  f_close(&FontFile) ;
//}


const uint8_t Romfont_std_packed[] =
{
#include "FontStd.lbm"
} ;

const uint8_t Romfont_mid_packed[] =
{
#include "FontMid.lbm"
} ;

const uint8_t Romfont_dbl_packed[] =
{
#include "FontDbl.lbm"
} ;

const uint8_t Romfont_xxl_packed[] =
{
#include "FontXxl.lbm"
} ;

const uint8_t Romfont_bold_packed[] =
{
#include "FontStdBold.lbm"
} ;

const uint8_t Romfont_small_packed[] =
{
#include "FontSmall.lbm"
} ;


uint8_t *unpackAllFonts( uint8_t *fontPtr )
{
	FontStd = fontPtr ;
	uint32_t i ;
	for ( i = 0 ; i < DISPLAY_BUFFER_SIZE/4 ; i += 1 )
	{
		LcdBackupFrameBuffer[i] = 0 ;		// set to BLACK
	}
	
	// Fix up arrow
//	uint8_t *p = FontStd ;
//	uint16_t height = *(((uint16_t *)p)+1) ;
//	uint16_t offset ;
//	uint16_t width ;
//	uint16_t *specs = (uint16_t *)font_stdsize_specs ;
//	offset = specs[96] ;
//	width = specs[97] - offset ;
//	p += 4 + offset*height ;
//#ifdef INVERT_DISPLAY
//	p += width-1 ;
//	while ( height)
//	{
//		*p = 0 ;
//		*(p-width+1) = 0 ;
//		p += width ;
//		height -= 1 ;
//	}
//#else
//	while ( height)
//	{
//		*p = 0 ;
//		*(p+width-1) = 0 ;
//		p += width ;
//		height -= 1 ;
//	}
//#endif
	
	fontPtr = unpackFont((uint8_t*)Romfont_std_packed, (uint16_t *)font_stdsize_specs, fontPtr, sizeof(Romfont_std_packed) ) ;
	FontMid = fontPtr ;
	fontPtr = unpackFont((uint8_t*)Romfont_mid_packed, (uint16_t *)font_midsize_specs, fontPtr, sizeof(Romfont_mid_packed) ) ;
	FontDbl = fontPtr ;
	fontPtr = unpackFont((uint8_t*)Romfont_dbl_packed, (uint16_t *)font_dblsize_specs, fontPtr, sizeof(Romfont_dbl_packed) ) ;
	FontBold = fontPtr ;
	fontPtr = unpackFont((uint8_t*)Romfont_bold_packed, (uint16_t *)font_stdsizebold_specs, fontPtr, sizeof(Romfont_bold_packed) ) ;
	FontXxl = fontPtr ;
	fontPtr = unpackFont((uint8_t*)Romfont_xxl_packed, (uint16_t *)font_xxlsize_specs, fontPtr, sizeof(Romfont_xxl_packed) ) ;
	FontSml = fontPtr ;
	fontPtr = unpackFont((uint8_t*)Romfont_small_packed, (uint16_t *)font_smlsize_specs, fontPtr, sizeof(Romfont_small_packed) ) ;

	return fontPtr ;
}


uint8_t *unpackFont( uint8_t *romFont, uint16_t *specs, uint8_t *fontPtr, uint32_t fontSize )
{
	uint32_t i ;
  uint8_t *dec_buf = romFont ;
  uint8_t *out_buf = fontPtr ;
	uint8_t byte ;

  memcpy(out_buf,dec_buf,4) ;
	out_buf += 4 ;
	dec_buf += 4 ;
	
#ifdef INVERT_DISPLAY
	uint16_t height = *(((uint16_t *)romFont)+1) ;
	uint32_t j ;
	uint16_t offset ;
	uint16_t width ;
	j = 0 ;
	uint8_t *p ;
	do
	{
		offset = specs[j] ;
		width = specs[j+1] - offset ;
		width *= height / 2 ;
		dec_buf += width ;
		p = dec_buf ;
		while ( width-- )
		{
			byte = *--p ;
			*out_buf++ = (byte << 4) | (byte&0x0F) ;
			byte &= 0xF0 ;
			*out_buf++ = byte | (byte >> 4) ;
		}
		j += 1 ;
	} while ( dec_buf < (uint8_t*)romFont + fontSize ) ;
#else	
	for ( i = 4 ; i < fontSize ; i += 1 )
	{
		byte = *dec_buf++ ;
		*out_buf++ = (byte & 0xF0) | (byte >> 4) ;
		*out_buf++ = (byte << 4) | (byte&0x0F) ;
	}
#endif

	i = (uint32_t) out_buf ;
	i = ( i + 3 ) & ~3 ;
	return (uint8_t *)i ;

}


extern coord_t LcdLastRightPos ;
extern coord_t LcdNextPos ;
extern coord_t LcdLastLeftPos ;

// Coordinates are full 480 x 272
void lcdDrawPropChDma( uint16_t x, uint16_t y, uint32_t w, uint32_t h, uint32_t mode, uint16_t colour, uint8_t *bitmap )
{
	uint32_t frontColour ;

	if ( ( mode & INVERS ) )
//	if ( ( mode & INVERS ) && ( colour == LCD_BLACK ) )
	{
		colour = LcdBackground ;
	}
	frontColour = ( (colour & 0xF800) << 8 ) | ( (colour & 0xE000) << 3 ) ;
	frontColour |= ( (colour & 0x07E0) << 5 ) | ( (colour & 0x0600) >> 1 ) ;
	frontColour |= ( (colour & 0x001F) << 3 ) | ( (colour & 0x001C) >> 2 ) | 0xFF000000 ;

#ifdef INVERT_DISPLAY
	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ;
#else
	uint32_t addr = CurrentFrameBuffer + (LCD_W*y + x)*2 ; // + (condensed ? 4 : 0) ;
#endif

	DMA2D_DeInit() ;
	DMA2D->CR = (DMA2D->CR & (uint32_t)CR_MASK) | DMA2D_M2M_BLEND ;
	DMA2D->OPFCCR = (DMA2D->OPFCCR & ~(uint32_t)DMA2D_OPFCCR_CM ) | DMA2D_RGB565 ;
  DMA2D->OMAR = addr ;
	DMA2D->OOR = (DMA2D->OOR & ~(uint32_t)DMA2D_OOR_LO ) | (LCD_W - w) ;
  DMA2D->NLR = (DMA2D->NLR & ~(DMA2D_NLR_NL | DMA2D_NLR_PL) ) | ( h | (w << 16) ) ;

	DMA2D->FGCLUT[0] = ( mode & INVERS ) ? frontColour : 0 ;
//	if ( mode & INVERS )
//	{
//		DMA2D->FGCLUT[1] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[2] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[3] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[4] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[5] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[6] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[7] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[8] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[9] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[10] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[11] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[11] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[13] = 0 ; // frontColour ;
//		DMA2D->FGCLUT[14] = 0 ; // frontColour ;
//	}
	DMA2D->FGCLUT[15] = ( mode & INVERS ) ? 0 : frontColour ;
  DMA2D->BGCOLR = 0 ;
  DMA2D->FGCOLR = frontColour ;
  
  DMA2D->FGPFCCR = (mode & ICON_CM_A4) ? CM_A4 :	CM_A8 ;
//  DMA2D->FGPFCCR = CM_A8 ;
  DMA2D->FGMAR   = (uint32_t) bitmap ;
  DMA2D->BGPFCCR = CM_RGB565 ;
  DMA2D->BGMAR   = ( mode & INVERS ) ? ((uint32_t)LcdBackupFrameBuffer) + (LCD_W*y + x)*2 : (uint32_t) addr ;
	DMA2D->BGOR = (DMA2D->BGOR & ~(uint32_t)DMA2D_BGOR_LO ) | (LCD_W - w) ;
  DMA2D->FGOR    = 0 ;
	
  DMA2D->IFCR = DMA2D_IFCR_CTCIF ;
	DMA2D->CR |= (uint32_t)DMA2D_CR_START ;
	pollDma2Ddone( 3000 ) ;
}

void lcdDrawPropImage( uint16_t x, uint16_t y, uint32_t w, uint32_t h, uint32_t mode, uint16_t colour, uint8_t *bitmap )
{
#ifdef INVERT_DISPLAY
	x = (LCD_W-1) - x - (w-1) ;
	if ( x >= LCD_W )
	{
		x += LCD_W ;
	}
	y = (LCD_H-1) - y - (h-1) ;
	if ( y >= LCD_H )
	{
		y = 0 ;
	}
#endif
	lcdDrawPropChDma( x, y, w, h, mode, colour, bitmap ) ;
}


//void testDrawPropCharMid( uint16_t x, uint16_t y, uint8_t ch, uint16_t colour, uint16_t mode )
//{
//	uint16_t index ;
//	uint16_t offset ;
//	uint16_t width ;
	
//	index = ch - 0x20 ;
//	offset = font_midsize_specs[index] ;
//	width = font_midsize_specs[index+1] - offset ;
//	uint8_t *q = FontMid + 4 + offset*28 ;

//#ifdef INVERT_DISPLAY
//	x = (LCD_W-1) - x - (width-1) ;
//	if ( x >= LCD_W )
//	{
//		x += LCD_W ;
//	}
//	y = (LCD_H-1) - y - 27 ;
//	if ( y >= LCD_H )
//	{
//		y = 0 ;
//	}
//#endif
	
//	lcdDrawPropChDma( x, y, width, 28, mode, colour, q ) ;
	
//}


void lcdDrawChar( uint16_t x, uint16_t y, uint8_t ch, uint16_t mode, uint16_t colour )
{
	uint16_t index ;
	uint16_t offset ;
	uint16_t width ;
	uint16_t height ;
	uint16_t *specs ;
	uint8_t *font ;
  uint32_t fontsize = FONTSIZE(mode) ;
	uint32_t languageOffset = 0 ;

	switch ( fontsize )
	{
		case MIDSIZE :
			specs = (uint16_t *)font_midsize_specs ;
			font = FontMid ;
		break ;
		case DBLSIZE :
			specs = (uint16_t *)font_dblsize_specs ;
			font = FontDbl ;
			languageOffset = FontLangOffsetDbl[g_eeGeneral.language] ;
		break ;
		case XXLSIZE :
			specs = (uint16_t *)font_xxlsize_specs ;
			font = FontXxl ;
		break ;
		case SMLSIZE :
		case TINSIZE :
			languageOffset = FontLangOffsetDbl[g_eeGeneral.language] ;
			specs = (uint16_t *)font_smlsize_specs ;
			font = FontSml ;
		break ;
		default :
			specs = (uint16_t *)font_stdsize_specs ;
			font = FontStd ;
			languageOffset = FontLangOffsetStd[g_eeGeneral.language] ;
			if ( mode & BOLD)
			{
				specs = (uint16_t *)font_stdsizebold_specs ;
				font = FontBold ;
			}
		break ;

	}
  height = *(((uint16_t *)font)+1) ;
	if ( ch >= 0xC0 )
	{
		ch += languageOffset - 64 ;
	}
	index = ch - 0x20 ;
	offset = specs[index] ;
	width = specs[index+1] - offset ;

	uint8_t *q = font + 4 + offset*height ;

	// Fix up arrow offset
	if ( ( index == 96 ) && (font == FontStd) )
	{
#ifdef INVERT_DISPLAY
		q -= 1 ;
#else
		q += 1 ;
#endif
	}

  LcdNextPos = x + width ;

#ifdef INVERT_DISPLAY
	x = (LCD_W-1) - x - (width-1) ;
	if ( x >= LCD_W )
	{
		x += LCD_W ;
	}
	y = (LCD_H-1) - y - (height-1) ;
	if ( y >= LCD_H )
	{
		y = 0 ;
	}
#endif
	
	if ( (mode & BLINK) && BLINK_ON_PHASE )
	{
		mode ^= INVERS ;
	}
	
	lcdDrawPropChDma( x, y, width, height, mode, colour, q ) ;
 
}

void testDrawPropChar( uint16_t x, uint16_t y, uint8_t ch, uint16_t colour, uint16_t mode )
{
	lcdDrawChar( x, y, ch, mode, colour ) ;
}

void lcdDrawChar( uint16_t x, uint16_t y, uint8_t ch, uint16_t mode )
{
	lcdDrawChar( x, y, ch, mode, LcdForeground ) ;
}

void lcdDrawChar( uint16_t x, uint16_t y, uint8_t ch )
{
	lcdDrawChar( x, y, ch, 0, LcdForeground ) ;
}

uint32_t colourCharWidth( unsigned char ch, LcdFlags flags)
{
  uint32_t fontsize = FONTSIZE(flags) ;
	uint16_t *specs ;
	if ( ch < 0x20 )
	{
		return 0 ;
	}
	ch -= 0x20 ;

	switch ( fontsize )
	{
		case MIDSIZE :
			specs = (uint16_t *)font_midsize_specs ;
		break ;
		case DBLSIZE :
			specs = (uint16_t *)font_dblsize_specs ;
		break ;
		case XXLSIZE :
			specs = (uint16_t *)font_xxlsize_specs ;
		break ;
		default :
			specs = (uint16_t *)font_stdsize_specs ;
			if ( flags & BOLD)
			{
				specs = (uint16_t *)font_stdsizebold_specs ;
			}
		break ;
	}
	return specs[ch+1] - specs[ch] ;

//	return 0 ;
}





