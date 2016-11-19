/****************************************************************************
*  Copyright (c) 2015 by Michael Blandford. All rights reserved.
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
*
****************************************************************************/

extern uint8_t M64Buttons ;
extern uint8_t M64Trims ;
extern uint16_t M64Switches ;
extern uint16_t M64Analog[] ;

extern uint8_t M64Contrast ;
extern uint8_t M64SetContrast ;

extern uint16_t M64Overruns ;
extern uint16_t M64CountErrors ;
extern uint8_t M64Received ;
extern uint8_t M64Revision ;
extern uint8_t M64ResetCount ;
extern uint8_t M64RestartCount ;

extern uint8_t M64SetHaptic ;
extern uint8_t M64HapticOnOff ;
extern uint8_t M64HapticStrength ;

//void poll_mega64( void ) ;
void checkM64( void ) ;
void displayToM64( void ) ;
void initM64( void ) ;
void m64_10mS( void ) ;
uint32_t m64ReceiveStatus( void ) ;

