/**
  ******************************************************************************
  * @file    Project/ee_drivers/i2c_9xt.h
  * @author  X9D Application Team
  * @version V 0.2
  * @date    12-JLY-2012
  * @brief   Header for i2c_9xt.cpp module
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __I2C103_H
#define __I2C103_H

#define	I2C_CMD_WRITE  (0)
#define	I2C_CMD_READ   (1)

//#define SCL_H         do{I2C_GPIO->BSRRL = I2C_SCL;}while((I2C_GPIO->IDR  & I2C_SCL)== 0)
//#define SCL_L         do{I2C_GPIO->BSRRH  = I2C_SCL;}
//#define SDA_H         do{I2C_GPIO->BSRRL = I2C_SDA;}
//#define SDA_L         do{I2C_GPIO->BSRRH  = I2C_SDA;}
//#define SCL_read      (I2C_GPIO->IDR  & I2C_SCL)
//#define SDA_read      (I2C_GPIO->IDR  & I2C_SDA)
//#define WP_H          do{I2C_WP_GPIO->BSRRL = I2C_WP;}
//#define WP_L          do{I2C_WP_GPIO->BSRRH = I2C_WP;}

//extern uint8_t McpReadData ;

/* Exported functions ------------------------------------------------------- */
void I2C_Init( void );
void stop_I2C2( void ) ;
//void init23008( void ) ;
//void write23008( uint8_t outputs ) ;
//void read23008( void ) ;


void init_I2C2( void ) ;
void writeExtRtcAddress( void ) ;
void readExtRtc( void ) ;
void pollForRtcComplete( void ) ;
void writeExtRtc( uint8_t *ptr ) ;

#endif /* __I2C_9XT_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

