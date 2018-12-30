/**
  ******************************************************************************
  * @file    Project/spi/spi.c 
  * @author  X9D Application Team
  * @Hardware version V0.2
  * @date    11-July-2012
  * @brief   This file provides spi Init(Analog spi),read and write.
             driver for LCD.
  * *
  ******************************************************************************
*/

#include "../ersky9x.h"
#include "X9D/stm32f2xx.h"
#include "X9D/stm32f2xx_gpio.h"
#include "X9D/stm32f2xx_rcc.h"
#include "X9D/hal.h"
#include "X9D/aspi.h"
#include "logicio.h"

#define __no_operation     __NOP

/**
**********send command to lcd**************
*/
//hardware spi
/*u16 spiCmd(u8 addr)
{
	u8 readValue;
	
	SPI_RS_LOW();
    SPI_NCS_LOW();
	
	SPI_WRITE_BYTE(addr);
	SPI_WAIT_DONE();

	readValue=SPI_READ_BYTE();
	SPI_NCS_HIGH();
    return	readValue;
}*/

//Anolog spi
#ifndef REVPLUS
#ifndef REV9E
void AspiCmd(uint8_t Command_Byte)
{
    int i=8; 
    LCD_A0_LOW();

    LCD_CLK_HIGH();
    LCD_NCS_LOW();  
 
    while (i--) 
    { 
	    LCD_CLK_LOW(); 
	    if(Command_Byte&0x80)
	{
        LCD_MOSI_HIGH();
    }
	else LCD_MOSI_LOW();

	Command_Byte=Command_Byte<<1; 
	LCD_CLK_HIGH();  
    } 
    LCD_NCS_HIGH();  
    LCD_A0_HIGH();
}


//Anolog spi
void AspiData(uint8_t Para_data)
{
    int i=8;
    LCD_CLK_HIGH();
    LCD_A0_HIGH();
    LCD_NCS_LOW();
    while (i--) 
    {
        if(Para_data&0x80)
        {
            LCD_MOSI_HIGH();
        }else LCD_MOSI_LOW();
        Para_data<<=1;
        LCD_CLK_LOW();
        __no_operation();
        LCD_CLK_HIGH();
        __no_operation();
    }
    LCD_NCS_HIGH();
    LCD_A0_HIGH();  
}
#endif // X9E
#endif // REVPLUS

#if defined(REVPLUS) || defined(REV9E)
void AspiCmd(uint8_t Command_Byte)
{
  LCD_A0_LOW() ;
  LCD_NCS_LOW() ;  
	while ( ( SPI3->SR & SPI_SR_TXE ) == 0 )
	{
		// wait
	}
	(void)SPI3->DR ;		// Clear receive
	SPI3->DR = Command_Byte ;
	while ( ( SPI3->SR & SPI_SR_RXNE ) == 0 )
	{
		// wait
	}
  LCD_NCS_HIGH() ;
}

#endif


