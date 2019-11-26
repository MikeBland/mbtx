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




void init5msTimer( void ) ;
void stop5msTimer( void ) ;

#ifdef PCBSKY
extern void init_hw_timer( void ) ;
//extern void stop_timer0( void ) ;
extern void init_pwm( void ) ;
#endif

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
extern void init_hw_timer( void ) ;
extern void hw_delay( uint16_t time ) ;
#endif

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)

// For use with Orange DSM module - 9XR PRO/ASSAN
#define ORTX_USE_DSMX			0x01 //- dsmX flag, else dsm2 use
#define ORTX_USE_11mS			0x02 //- 11 mSec flag, else 22 mSec
#define ORTX_USE_11bit		0x04 //- 11 bit (2048) flag, else 10 bit (1024) resolution
#define ORTX_USE_TM				0x08 //- have a telemetry answer
#define ORTX_USE_FUTABA		0x10 //- futaba channel "AETR"
#define ORTX_USE_DEVO			0x20 //- walkera DEVO
#define ORTX_AUTO_MODE		0x40 //- auto DSM2/DSMX
#define ORTX_BIND_FLAG		0x80

//extern uint32_t Peri1_frequency ;
//extern uint32_t Peri2_frequency ;
//extern uint32_t Timer_mult1 ;
//extern uint32_t Timer_mult2 ;
//extern void hwTimerStart( void ) ;
//extern uint16_t hwTimerValue( void ) ;
extern void init_main_ppm( void ) ;
extern void disable_main_ppm( void ) ;
extern void init_trainer_ppm( void ) ;
extern void stop_trainer_ppm( void ) ;
extern void init_trainer_capture( uint32_t mode ) ;
extern void stop_trainer_capture( void ) ;
extern void setupPulsesPXX(uint8_t module) ;
extern void setupPulses( unsigned int port ) ;
extern void setupPulsesPpm( void ) ;
extern void setupPulsesPpmx( void ) ;
extern void init_no_pulses(uint32_t port) ;
extern void disable_no_pulses(uint32_t port) ;
extern void init_dsm2(uint32_t port) ;
extern void init_multi(uint32_t port) ;
extern void disable_dsm2(uint32_t port) ;
extern void init_xfire(uint32_t port) ;
extern void disable_xfire(uint32_t port) ;
void setupPulsesDsm2(uint8_t channels, uint32_t module ) ;
extern void init_cppm_on_heartbeat_capture( void ) ;
extern void stop_cppm_on_heartbeat_capture( void ) ;
extern void init_serial_trainer_capture( void ) ;
extern void stop_serial_trainer_capture( void ) ;

#endif

#if defined(PCBLEM1)
extern void init_trainer_capture( uint32_t mode ) ;

#endif


