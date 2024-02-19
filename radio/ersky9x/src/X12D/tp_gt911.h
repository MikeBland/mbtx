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


#define GT911_MAX_TP            5
#define GT911_CFG_NUMER         0x6A

#define GT911_READ_XY_REG               0x814E
#define GT911_CLEARBUF_REG              0x814E
#define GT911_CONFIG_REG                0x8047
#define GT911_COMMAND_REG               0x8040
#define GT911_PRODUCT_ID_REG            0x8140
#define GT911_VENDOR_ID_REG             0x814A
#define GT911_CONFIG_VERSION_REG        0x8047
#define GT911_CONFIG_CHECKSUM_REG       0x80FF
#define GT911_FIRMWARE_VERSION_REG      0x8144

#ifndef PACK
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

PACK(typedef struct
{
  uint8_t track;
  uint16_t x ;
  uint16_t y ;
  uint16_t size ;
  uint8_t reserved ;
}) TouchPoint ;

PACK(struct t_TouchData
{
//  uint8_t pointsCount;
  union
  {
    TouchPoint points[GT911_MAX_TP] ;
    uint8_t data[GT911_MAX_TP * sizeof(TouchPoint)] ;
  } ;
}) ;

extern uint8_t TouchEventOccured ;
extern uint8_t TouchUpdated ;
extern uint8_t TouchBacklight ;
extern struct t_TouchData TouchData ;

uint32_t touchPanelInit( void ) ;
uint32_t touchReadPoints( void ) ;










