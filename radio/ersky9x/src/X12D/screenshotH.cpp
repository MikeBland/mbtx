/*
 * Copied from (C) OpenTX
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

//#include "opentx.h"
#include "..\ersky9x.h"

void setFilenameDateTime( char *filename, uint32_t includeTime ) ;
uint16_t lcd_getPixel(uint32_t x, uint32_t y ) ;
void lcd_setPixel(uint32_t x, uint32_t y, uint16_t value ) ;

#define BMP_HEADERSIZE 0x76

#define BMP_BITS_PER_PIXEL 16

#define BMP_FILESIZE uint32_t(BMP_HEADERSIZE + (480 * 272) * BMP_BITS_PER_PIXEL / 8)

#define GET_RED(color) (((color) & 0xF800) >> 8)

#define GET_GREEN(color) (((color) & 0x07E0) >> 3)

#define GET_BLUE(color) (((color) & 0x001F) << 3)

const uint8_t BMP_HEADER[] = {
  'B',  'M',
  /* file size */ BMP_FILESIZE & 0xFF, (BMP_FILESIZE >> 8) & 0xFF, (BMP_FILESIZE >> 16) & 0xFF, (BMP_FILESIZE >> 24) & 0xFF,
  0x00, 0x00, 0x00, 0x00,
  /* header size */ BMP_HEADERSIZE, 0x00, 0x00, 0x00,
  /* extra header size */ 0x28, 0x00, 0x00, 0x00,
  /* width */ 480 & 0xFF, 480 >> 8, 0x00, 0x00,
  /* height */ 272 & 0xFF, 272 >> 8, 0x00, 0x00,
  /* planes */ 0x01, 0x00,
  /* depth */ BMP_BITS_PER_PIXEL, 0x00,
  0x03, 0x00,
  0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0xbc, 0x38, 0x00, 0x00, 0xbc, 0x38, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0xee, 0xee, 0xee, 0x00, 0xdd, 0xdd,
  0xdd, 0x00, 0xcc, 0xcc, 0xcc, 0x00, 0xbb, 0xbb, 0xbb, 0x00, 0xaa, 0xaa, 0xaa, 0x00, 0x99, 0x99,
  0x99, 0x00, 0x88, 0x88, 0x88, 0x00, 0x77, 0x77, 0x77, 0x00, 0x66, 0x66, 0x66, 0x00, 0x55, 0x55,
  0x55, 0x00, 0x44, 0x44, 0x44, 0x00, 0x33, 0x33, 0x33, 0x00, 0x22, 0x22, 0x22, 0x00, 0x11, 0x11,
  0x11, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint16_t PixelBuffer[32] ;

const char *screenshot()
{
  FIL bmpFile;
  UINT written;
  char filename[42]; // /SCREENSHOTS/screen-2013-01-01-123540.bmp
  DIR folder ;
	uint32_t bufferIndex ;
	FRESULT result ;

	result = f_opendir(&folder, "/screenshots") ;
  if (result != FR_OK)
	{
    if (result == FR_NO_PATH)
		{
//			WatchdogTimeout = 300 ;		// 3 seconds
      result = f_mkdir("/screenshots") ;
			wdt_reset() ;
    	if (result != FR_OK)
			{
      	return "SDCARD ERROR" ;
			}
		}
  }
	
//  // check and create folder here
//  strcpy(filename, SCREENSHOTS_PATH);
//  const char * error = sdCheckAndCreateDirectory(filename);
//  if (error) {
//    return error;
//  }

	
  char *tmp = (char *)cpystr((uint8_t *)filename, (uint8_t *)"/screenshots/screen") ;
	setFilenameDateTime( tmp, 1 ) ;
  cpystr((uint8_t *)&filename[12 + 7 + 18], (uint8_t *)".BMP") ;

  result = f_open(&bmpFile, filename, FA_CREATE_ALWAYS | FA_WRITE);
  if (result != FR_OK)
	{
    return "SD Error" ; //SDCARD_ERROR(result);
  }

  result = f_write(&bmpFile, BMP_HEADER, sizeof(BMP_HEADER), &written) ;
  if (result != FR_OK || written != sizeof(BMP_HEADER))
	{
    f_close(&bmpFile);
    return "SD Error" ; //SDCARD_ERROR(result);
  }

	bufferIndex = 0 ;
  for (int y = 272 - 1; y >= 0; y--)
	{
    for (uint32_t x = 0; x < 480 ; x++)
		{
			uint16_t pixel ;
      pixel = lcd_getPixel(x,y) ;
			lcd_setPixel( x, y, ~pixel ) ;
//      uint32_t dst = (0xFF << 24) + (GET_RED(pixel) << 16) + (GET_GREEN(pixel) << 8) + (GET_BLUE(pixel) << 0) ;
      
			PixelBuffer[bufferIndex++] = pixel ;
			if ( bufferIndex >= 32 )
			{
				bufferIndex = 0 ;
      	if (f_write(&bmpFile, (BYTE *)&PixelBuffer, sizeof(PixelBuffer), &written) != FR_OK || written != sizeof(PixelBuffer))
				{
      	  f_close(&bmpFile);
		  	  return "SD Error" ; //SDCARD_ERROR(result);
      	}
      }
    }
		wdt_reset() ;
  }
  f_close(&bmpFile) ;

  return 0 ;
}
