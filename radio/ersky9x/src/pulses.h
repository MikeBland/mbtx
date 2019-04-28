/****************************************************************************
*  Copyright (c) 2013 by Michael Blandford. All rights reserved.
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
* Other Authors:
 * - Andre Bernet
 * - Bertrand Songis
 * - Bryan J. Rentoul (Gruvin)
 * - Cameron Weeks
 * - Erez Raviv
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini
 * - Thomas Husterer
*
****************************************************************************/

// For use with Orange DSM module - 9XR PRO
#define ORTX_USE_DSMX			0x01 //- dsmX flag, else dsm2 use
#define ORTX_USE_11mS			0x02 //- 11 mSec flag, else 22 mSec
#define ORTX_USE_11bit		0x04 //- 11 bit (2048) flag, else 10 bit (1024) resolution
#define ORTX_USE_TM				0x08 //- have a telemetry answer
#define ORTX_USE_FUTABA		0x10 //- futaba channel "AETR"
#define ORTX_USE_DEVO			0x20 //- walkera DEVO
#define ORTX_AUTO_MODE		0x40 //- auto DSM2/DSMX
#define ORTX_BIND_FLAG		0x80

// Control bits
#define BindBit 0x80
#define RangeCheckBit 0x20
#define BadData 0x47

extern void module_output_low( void ) ;
extern void module_output_active( void ) ;
extern void InternalOutputLow( void ) ;
extern void InternalOutputActive( void ) ;


extern void init_main_ppm( uint32_t period, uint32_t out_enable ) ;
extern void init_ppm2( uint32_t period, uint32_t out_enable ) ;
extern void disable_main_ppm( void ) ;
//extern void disable_ppm2( void ) ;
extern void perOut( int16_t *chanOut, uint8_t att ) ;
extern void startPulses( void ) ;
extern void setupPulses( void ) ;
extern void setupPulsesPPM( void ) ;
extern void setupPulsesPPM2( void ) ;
extern void setupPulsesDsm2(uint8_t chns) ;
extern void setupPulsesPXX( void ) ;
extern void dsmBindResponse( uint8_t mode, int8_t channels ) ;
extern void checkTrainerSource( void ) ;
extern void pausePulses( void ) ;
extern void resumePulses( void ) ;
#ifdef ACCESS
extern void setupPulsesAccess( uint32_t module ) ;
#endif

extern void setMultiSerialArray( uint8_t *data, uint32_t module ) ;
extern uint16_t CRCTable(uint8_t val) ;
extern uint16_t scaleForPXX( uint8_t i ) ;
extern void dsmBindResponse( uint8_t mode, int8_t channels ) ;
extern void setDsmHeader( uint8_t *dsmDat, uint32_t module ) ;


