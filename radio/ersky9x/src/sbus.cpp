/****************************************************************************
*  Copyright (c) 2014 by Michael Blandford. All rights reserved.
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
****************************************************************************
*  History:
*
****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include "ersky9x.h"
#include "drivers.h"
#include "sbus.h"

uint8_t SbusFrame[28] ;
uint8_t SbusIndex = 0 ;
uint16_t SbusTimer = 0 ;

uint32_t processSBUSframe( uint8_t *sbus, int16_t *pulses, uint32_t size )
{
	uint32_t inputbitsavailable = 0 ;
	uint32_t i ;
	uint32_t inputbits = 0 ;
	uint8_t sum ;
	sum = 0 ;
	if ( *sbus++ != 0x0F )
	{
		return 0 ;		// Not a valid SBUS frame
	}
	if ( size < 23 )
	{
		return 0 ;
	}
	for ( i = 0 ; i < 16 ; i += 1 )
	{
		while ( inputbitsavailable < 11 )
		{
			uint8_t temp ;
			temp = *sbus++ ;
			if ( i < 4 )
			{
				sum |= temp ;
			}
			inputbits |= temp << inputbitsavailable ;
			inputbitsavailable += 8 ;
		}
		if ( pulses )
		{
			*pulses++ = ( (int32_t)( inputbits & 0x7FF ) - 0x3E0 ) * 5 / 8 ;
		}
		inputbitsavailable -= 11 ;
		inputbits >>= 11 ;
	}
	if ( pulses )
	{
		if ( ( *sbus & 0x08 ) || ( sum == 0 ) )
		{
			if ( ppmInValid )
			{
				ppmInValid = 1 ;	// Will count down to 0 and announce soon
			}
		}
		else
		{
			if ( ( (*sbus & 0x04) == 0 ) && sum )
			{
				ppmInValid = 100 ;
			}
		}
	}
	SbusTimer = 1000 ;
	return 1 ;
}

uint16_t SbusCounter ;
uint16_t SbusCounter1 ;

void processSbusInput()
{
	SbusCounter += 1 ;
	uint16_t rxchar ;
	uint32_t active = 0 ;
	while ( ( rxchar = get_fifo64( &Sbus_fifo ) ) != 0xFFFF )
	{
		SbusCounter1 += 1 ;
		
		active = 1 ;
		SbusFrame[SbusIndex++] = rxchar ;
		if ( SbusIndex > 27 )
		{
			SbusIndex = 27 ;
		}
	}
	if ( active )
	{
		SbusTimer = getTmr2MHz() ;
		return ;
	}
	else
	{
		if ( SbusIndex )
		{
			if ( ( uint16_t)( getTmr2MHz() - SbusTimer ) > 1000 )	// 500 uS
			{
				processSBUSframe( SbusFrame, g_ppmIns, SbusIndex ) ;
				SbusIndex = 0 ;	 
			}
		}
	}
}




