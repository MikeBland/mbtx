/****************************************************************************
*  Copyright (c) 2012 by Michael Blandford. All rights reserved.
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

extern volatile uint16_t Analog_values[] ;

extern void init_adc( void ) ;
extern void init_adc2() ;
extern uint32_t read_adc( void ) ;
extern void stop_adc( void ) ;
void disableRtcBattery( void ) ;

#ifdef PCB9XT

extern uint16_t AnalogSwitches ;
// Bits in AnalogSwitches ;
#define AS_LV_TRIM_UP		0x0004
#define AS_LV_TRIM_DN		0x0002
#define AS_RV_TRIM_UP		0x0001
#define AS_AIL_SW				0x0020
#define AS_THR_SW				0x0010
#define AS_RV_TRIM_DN		0x0008
#define AS_IDL2_SW			0x0100
#define AS_IDL1_SW			0x0080
#define AS_GEA_SW				0x0040


void processAnalogSwitches( void ) ;
#endif // PCB9XT


