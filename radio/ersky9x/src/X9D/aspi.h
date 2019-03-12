/**
  ******************************************************************************
  * @file    Project/spi/spi.h 
  * @author  X9D Application Team
  * @Hardware version V0.2
  * @date    11-July-2012
  * @brief   spi.c' Header file.
  * *
  ******************************************************************************
*/
#ifndef _ASPI_H_
#define _ASPI_H_

#include "stm32f2xx.h"

#if defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)

#define LCD_NCS_HIGH()        LCD_NCS_GPIO->BSRRL = LCD_NCS_GPIO_PIN
#define LCD_NCS_LOW()         LCD_NCS_GPIO->BSRRH = LCD_NCS_GPIO_PIN

#define LCD_A0_HIGH()         LCD_SPI_GPIO->BSRRL = LCD_A0_GPIO_PIN
#define LCD_A0_LOW()          LCD_SPI_GPIO->BSRRH = LCD_A0_GPIO_PIN

#define LCD_RST_HIGH()        LCD_RST_GPIO->BSRRL = LCD_RST_GPIO_PIN
#define LCD_RST_LOW()         LCD_RST_GPIO->BSRRH = LCD_RST_GPIO_PIN

#else // PCBX7
#if defined(REVPLUS) || defined(REV9E)
#define	LCD_NCS_HIGH()		    (GPIO_LCD_NCS->BSRRL = PIN_LCD_NCS)
#define	LCD_NCS_LOW()		    	(GPIO_LCD_NCS->BSRRH = PIN_LCD_NCS)
#else // REVPLUS
#define	LCD_NCS_HIGH()		    (GPIO_LCD->BSRRL = PIN_LCD_NCS)
#define	LCD_NCS_LOW()		    	(GPIO_LCD->BSRRH = PIN_LCD_NCS)
#endif // REVPLUS

#define LCD_A0_HIGH()         (GPIO_LCD->BSRRL = PIN_LCD_A0)
#define LCD_A0_LOW()          (GPIO_LCD->BSRRH = PIN_LCD_A0)

#if defined(REVPLUS) || defined(REV9E)
#define LCD_RST_HIGH()		    (GPIO_LCD_RST->BSRRL = PIN_LCD_RST)
#define LCD_RST_LOW()		    	(GPIO_LCD_RST->BSRRH = PIN_LCD_RST)
#else // REVPLUS
#define LCD_RST_HIGH()		    (GPIO_LCD->BSRRL = PIN_LCD_RST)
#define LCD_RST_LOW()		    	(GPIO_LCD->BSRRH = PIN_LCD_RST)
#endif // REVPLUS
#endif // PCBX7

#define LCD_CLK_HIGH()		    (GPIO_LCD->BSRRL = PIN_LCD_CLK)
#define LCD_CLK_LOW()		    	(GPIO_LCD->BSRRH = PIN_LCD_CLK)

#define LCD_MOSI_HIGH()		    (GPIO_LCD->BSRRL = PIN_LCD_MOSI)
#define LCD_MOSI_LOW()		    (GPIO_LCD->BSRRH = PIN_LCD_MOSI)

void AspiCmd(u8 Command_Byte);
void AspiData(u8 Para_data);
//u16 spiRegAccess(u8 addrByte, u8 writeValue);

#endif

