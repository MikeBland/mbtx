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

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#ifdef PCBSKY
#include "AT91SAM3S4.h"
#endif

#if defined(PCBX9D) || defined(PCB9XT)
#include "X9D/stm32f2xx.h"
#include "X9D/hal.h"
#include "debug.h"
#endif


#ifndef PCBX12D
#ifndef SIMU
#include "core_cm3.h"
#endif
#endif

#ifdef PCBX12D
#include "X12D/stm32f4xx.h"
#include "X12D/hal.h"
#endif

#include "ersky9x.h"
#include "timers.h"
#include "logicio.h"
#include "myeeprom.h"
#include "drivers.h"

extern int16_t g_chans512[] ;

#define INTERNAL_RF_ON()      GPIO_SetBits(GPIOPWRINT, PIN_INT_RF_PWR)
#define INTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWRINT, PIN_INT_RF_PWR)
#define EXTERNAL_RF_ON()      GPIO_SetBits(GPIOPWREXT, PIN_EXT_RF_PWR)
#define EXTERNAL_RF_OFF()     GPIO_ResetBits(GPIOPWREXT, PIN_EXT_RF_PWR)


extern void init_pxx(uint32_t port) ;
extern void disable_pxx(uint32_t port) ;

void setupPulsesPpmAll(uint32_t module) ;

#define BindBit 0x80

// To Do
#define NUM_MODULES 2

extern void init_ppm(uint32_t port) ;
extern void disable_ppm(uint32_t port) ;

#ifndef PCBSKY

//uint16_t *PulsePtr ;
uint16_t *TrainerPulsePtr ;
uint16_t PcmCrc ;
uint8_t PcmOnesCount ;
//uint8_t Current_protocol ;
uint8_t PxxFlag[2] = { 0, 0 } ;

volatile uint8_t Dsm_Type[2] = { 0, 0 } ;
uint8_t DsmInitCounter[2] = { 0, 0 } ;

extern uint16_t *ppmStreamPtr[NUM_MODULES];
extern uint16_t pulseStreamCount[NUM_MODULES] ;
extern uint16_t ppmStream[NUM_MODULES+1][20];
extern uint8_t s_current_protocol[NUM_MODULES] ;

static uint8_t Pass[2] ;
#endif

// TC0 - hardware timer
// TC1 - DAC clock
// TC2 - 5mS timer
// TC3 - input capture
// TC4 - input capture clock
// TC5 - Software COM1

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D)
//uint8_t DebugDsmPass ;
uint8_t PulsesPaused ;

#ifdef XFIRE
uint8_t Bit_pulses[64] ;			// Likely more than we need
#endif

// DSM2 control bits
#define BindBit 0x80
#define RangeCheckBit 0x20
#define FranceBit 0x10
#define DsmxBit  0x08
#define BadData 0x47


// States in LineState
#define LINE_IDLE			0
#define LINE_ACTIVE		1

#define BIT_TIME_100K		20

uint16_t LastTransition ;
//extern uint16_t BitTime ;
//extern uint16_t HtoLtime ;
//extern uint16_t LtoHtime ;
//extern uint8_t LineState ;
extern void putCaptureTime( struct t_softSerial *pss, uint16_t time, uint32_t value ) ;
//extern uint8_t SoftSerInvert ;
//extern uint8_t SoftSerialEvenParity ;
extern void start_timer11(void) ;
extern void stop_timer11(void) ;
extern uint8_t TrainerPolarity ;

uint8_t setupPulsesXfire() ;

uint16_t FailsafeCounter[2] ;

void pausePulses()
{
	PulsesPaused = 1 ;
}

void resumePulses()
{
	PulsesPaused = 0 ;
	if ( g_model.Module[1].protocol != PROTO_OFF )
	{
		EXTERNAL_RF_ON() ;
	}
	if ( g_model.Module[0].protocol != PROTO_OFF )
	{
		INTERNAL_RF_ON() ;
	}
}

void startPulses()
{
//	s_current_protocol[0] = g_model.protocol + 1 ;		// Not the same!
//	s_current_protocol[1] = g_model.xprotocol + 1 ;		// Not the same!
	setupPulses(0) ;// For DSM-9XR this won't be sent
	setupPulses(1) ;// For DSM-9XR this won't be sent
	Pass[0] = 0 ;		// Force a type 0 packet
	Pass[1] = 0 ;		// Force a type 0 packet
}


#endif
// Starts TIMER at 200Hz, 5mS period
#ifdef PCBSKY
void init5msTimer()
{
  register Tc *ptc ;
	register uint32_t timer ;

  PMC->PMC_PCER0 |= 0x02000000L ;		// Enable peripheral clock to TC2

	timer = Master_frequency / 12800 / 2 ;		// MCK/128 and 200 Hz

  ptc = TC0 ;		// Tc block 0 (TC0-2)
	ptc->TC_BCR = 0 ;			// No sync
	ptc->TC_BMR = 0 ;
	ptc->TC_CHANNEL[2].TC_CMR = 0x00008000 ;	// Waveform mode
	ptc->TC_CHANNEL[2].TC_RC = timer ;			// 10 Hz
	ptc->TC_CHANNEL[2].TC_RA = timer >> 1 ;
	ptc->TC_CHANNEL[2].TC_CMR = 0x0009C003 ;	// 0000 0000 0000 1001 1100 0000 0000 0011
																						// MCK/128, set @ RA, Clear @ RC waveform
	ptc->TC_CHANNEL[2].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
	
  NVIC_SetPriority(TC2_IRQn, 4) ;
	NVIC_EnableIRQ(TC2_IRQn) ;
	TC0->TC_CHANNEL[2].TC_IER = TC_IER0_CPCS ;
}

#ifdef DUE
void TC2_Handler()
#else
extern "C" void TC2_IRQHandler()
#endif
{
  register uint32_t dummy;

  /* Clear status bit to acknowledge interrupt */
  dummy = TC0->TC_CHANNEL[2].TC_SR;
	(void) dummy ;		// Discard value - prevents compiler warning

	interrupt5ms() ;
	
}


//void stop5msTimer( void )
//{
//	TC0->TC_CHANNEL[2].TC_CCR = TC_CCR0_CLKDIS ;
//	NVIC_DisableIRQ(TC2_IRQn) ;
//  PMC->PMC_PCDR0 |= 0x02000000L ;		// Disable peripheral clock to TC2
//}



// Starts TIMER0 at full speed (MCK/2) for delay timing
// @ 36MHz this is 18MHz
// This was 6 MHz, we may need to slow it to TIMER_CLOCK2 (MCK/8=4.5 MHz)
void init_hw_timer()
{
  register Tc *ptc ;

  PMC->PMC_PCER0 |= 0x00800000L ;		// Enable peripheral clock to TC0

  ptc = TC0 ;		// Tc block 0 (TC0-2)
	ptc->TC_BCR = 0 ;			// No sync
	ptc->TC_BMR = 2 ;
	ptc->TC_CHANNEL[0].TC_CMR = 0x00008001 ;	// Waveform mode MCK/8 for 36MHz osc.(Upset be write below)
	ptc->TC_CHANNEL[0].TC_RC = 0xFFF0 ;
	ptc->TC_CHANNEL[0].TC_RA = 0 ;
	ptc->TC_CHANNEL[0].TC_CMR = 0x00008040 ;	// 0000 0000 0000 0000 1000 0000 0100 0000, stop at regC, 18MHz
	ptc->TC_CHANNEL[0].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
}

//void stop_timer0( void )
//{
//	TC0->TC_CHANNEL[0].TC_CCR = TC_CCR0_CLKDIS ;		// Disable clock
//}

// PWM used for PPM generation, and LED Backlight
// Output pin PB5 not used, PA17 used as PWMH3 peripheral C
// PWM peripheral ID = 31 (0x80000000)
// Ensure PB5 is three state/input, used on REVB for MENU KEY

// Configure PWM3 as PPM drive, 
// PWM0 is LED backlight PWM on PWMH0
// This is PC18 peripheral B, Also enable PC22 peripheral B, this is PPM-JACK (PWML3)
//
// REVB board:
// PWML2, output as peripheral C on PA16, is for HAPTIC
// For testing, just drive it out with PWM
// PWML1 for PPM2 output as peripheral B on PC15
// For testing, just drive it out with PWM
void init_pwm()
{
#ifdef REVB
#else
	register Pio *pioptr ;
#endif
	register Pwm *pwmptr ;
	register uint32_t timer ;

  PMC->PMC_PCER0 |= ( 1 << ID_PWM ) ;		// Enable peripheral clock to PWM
  
	MATRIX->CCFG_SYSIO |= 0x00000020L ;				// Disable TDO let PB5 work!
	
	/* Configure PIO */
#ifdef REVB
#else
	pioptr = PIOB ;
	pioptr->PIO_PER = 0x00000020L ;		// Enable bit B5
	pioptr->PIO_ODR = 0x00000020L ;		// set as input
#endif

#ifdef REVB
	configure_pins( PIO_PA16, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_C | PIN_PORTA | PIN_NO_PULLUP ) ;
#endif

	configure_pins( PIO_PC18, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_B | PIN_PORTC | PIN_NO_PULLUP ) ;

#ifdef REVB
	configure_pins( PIO_PC22, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_B | PIN_PORTC | PIN_NO_PULLUP ) ;
#endif

	// Configure clock - depends on MCK frequency
	timer = Master_frequency / 2000000 ;
	timer |= ( Master_frequency / ( 32* 10000 ) ) << 16 ;
	timer &= 0x00FF00FF ;

	pwmptr = PWM ;
	pwmptr->PWM_CLK = 0x05000000 | timer ;	// MCK for DIVA, DIVA = 18 gives 0.5uS clock period @35MHz
																		// MCK/32 / timer = 10000Hz for CLKB
	
	// PWM0 for LED backlight
#ifdef REVX
	pwmptr->PWM_CH_NUM[0].PWM_CMR = 0x0000000C ;	// CLKB
#else
	pwmptr->PWM_CH_NUM[0].PWM_CMR = 0x0000000B ;	// MCK/1024
#endif
	pwmptr->PWM_CH_NUM[0].PWM_CPDR = 100 ;			// Period
	pwmptr->PWM_CH_NUM[0].PWM_CPDRUPD = 100 ;		// Period
	pwmptr->PWM_CH_NUM[0].PWM_CDTY = 40 ;				// Duty
	pwmptr->PWM_CH_NUM[0].PWM_CDTYUPD = 40 ;		// Duty
	pwmptr->PWM_ENA = PWM_ENA_CHID0 ;						// Enable channel 0

#ifdef REVB
	// PWM2 for HAPTIC drive 100Hz test
	pwmptr->PWM_CH_NUM[2].PWM_CMR = 0x0000000C ;	// CLKB
	pwmptr->PWM_CH_NUM[2].PWM_CPDR = 100 ;			// Period
	pwmptr->PWM_CH_NUM[2].PWM_CPDRUPD = 100 ;		// Period
	pwmptr->PWM_CH_NUM[2].PWM_CDTY = 40 ;				// Duty
	pwmptr->PWM_CH_NUM[2].PWM_CDTYUPD = 40 ;		// Duty
	pwmptr->PWM_OOV &= ~0x00040000 ;	// Force low
	pwmptr->PWM_OSS = 0x00040000 ;	// Force low
//	pwmptr->PWM_ENA = PWM_ENA_CHID2 ;						// Enable channel 2
#endif

}












#endif


#ifdef PCBX12D
// Starts TIMER at 200Hz, 5mS period
void init5msTimer()
{
  INTERRUPT_5MS_TIMER->ARR = 4999 ;     // 5mS
  INTERRUPT_5MS_TIMER->PSC = (PeripheralSpeeds.Peri1_frequency * PeripheralSpeeds.Timer_mult1) / 1000000 - 1 ; // 1uS from 30MHz
  INTERRUPT_5MS_TIMER->CCER = 0 ;
  INTERRUPT_5MS_TIMER->CCMR1 = 0 ;
  INTERRUPT_5MS_TIMER->EGR = 0 ;
  INTERRUPT_5MS_TIMER->CR1 = 5 ;
  INTERRUPT_5MS_TIMER->DIER |= 1 ;
  NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 7);
  NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn) ;
}

extern "C" void TIM8_TRG_COM_TIM14_IRQHandler()
{
  TIM14->SR &= ~TIM_SR_UIF ;
  interrupt5ms() ;
}

#define NUM_MODULES 2

extern uint8_t s_current_protocol[NUM_MODULES] ;

extern void init_ppm(uint32_t port) ;
extern void disable_ppm(uint32_t port) ;

#endif


#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D)
void init_hw_timer()
{
	// Timer13
	RCC->APB1ENR |= RCC_APB1ENR_TIM13EN ;		// Enable clock
	TIM13->ARR = 65535 ;
	TIM13->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 10000000 - 1 ;		// 0.1uS from 30MHz
	TIM13->CCER = 0 ;	
	TIM13->CCMR1 = 0 ;
	TIM13->EGR = 0 ;
	TIM13->CR1 = 1 ;
}


// delay in units of 0.1 uS up to 6.5535 mS
void hw_delay( uint16_t time )
{
	TIM13->CNT = 0 ;
	TIM13->EGR = 1 ;		// Re-start counter
	while ( TIM13->CNT < time )
	{
		// wait
	}
}

#endif



// Starts TIMER at 200Hz, 5mS period
#if defined(PCBX9D) || defined(PCB9XT)

uint8_t Dsm_mode_response = 0 ;

void init5msTimer()
{
	// Timer14
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN ;		// Enable clock
	TIM14->ARR = 4999 ;	// 5mS
	TIM14->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 1000000 - 1 ;		// 1uS from 30MHz
	TIM14->CCER = 0 ;	
	TIM14->CCMR1 = 0 ;
	TIM14->EGR = 0 ;
	TIM14->CR1 = 5 ;
	TIM14->DIER |= 1 ;
  NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 4 ) ;
	NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn) ;
}

//void stop5msTimer( void )
//{
//	TIM14->CR1 = 0 ;	// stop timer
//	NVIC_DisableIRQ(TIM8_TRG_COM_TIM14_IRQn) ;
//	RCC->APB1ENR &= ~RCC_APB1ENR_TIM14EN ;		// Disable clock
//}

extern "C" void TIM8_TRG_COM_TIM14_IRQHandler()
{
	TIM14->SR &= ~TIM_SR_UIF ;
	interrupt5ms() ;
}

#ifdef REV9E
uint16_t SuCount ;
#endif

#endif


#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D)

void setupPulses(unsigned int port)
{
	uint32_t requiredprotocol ;
  heartbeat |= HEART_TIMER_PULSES ;
#ifdef REV9E
	SuCount += 1 ;
//	progress( 0xA000 + port + ( SuCount << 4 ) ) ;
//	return ;
#endif
	

	if ( port == 0 )
	{
		requiredprotocol = g_model.Module[INTERNAL_MODULE].protocol ;
		if ( PulsesPaused )
		{
			requiredprotocol = PROTO_OFF ;
		}
  	if ( s_current_protocol[INTERNAL_MODULE] != requiredprotocol )
  	{
  	  switch( s_current_protocol[INTERNAL_MODULE] )
  	  {	// stop existing protocol hardware
#ifndef PCBX12D
  	    case PROTO_PPM:
					disable_main_ppm() ;
  	    break;
#endif
  	    case PROTO_PXX:
					disable_pxx(INTERNAL_MODULE) ;
  	    break;
#ifdef PCB9XT
	      case PROTO_DSM2:
		    case PROTO_MULTI:
					disable_dsm2(INTERNAL_MODULE) ;
	      break;
#endif
	//      case PROTO_DSM2:
	//				disable_ssc() ;
	//      break;
	//      case PROTO_PPM16 :
	//				disable_main_ppm() ;
	//      break ;
  	  }
		
  	  s_current_protocol[INTERNAL_MODULE] = requiredprotocol ;
  	  switch(s_current_protocol[INTERNAL_MODULE])
  	  { // Start new protocol hardware here
#ifndef PCBX12D
  	    case PROTO_PPM:
					init_main_ppm() ;
  	    break;
#endif
  	    case PROTO_PXX:
					init_pxx(INTERNAL_MODULE) ;
  	    break;
		    case PROTO_OFF:
				default :
					init_no_pulses( INTERNAL_MODULE ) ;
  	  	break;
#ifdef PCB9XT
	    	case PROTO_MULTI:
					init_multi(INTERNAL_MODULE) ;
					DsmInitCounter[port] = 0 ;
					Pass[port] = 0 ;
  	  	break;
	      case PROTO_DSM2:
					init_dsm2(INTERNAL_MODULE) ;
					DsmInitCounter[port] = 0 ;
					Pass[port] = 0 ;
	      break;
#endif
  	  }
  	}

	// Set up output data here
		switch(requiredprotocol)
  	{
#ifndef PCBX12D
		  case PROTO_PPM:
  	    setupPulsesPpmAll(INTERNAL_MODULE);		// Don't enable interrupts through here
  	  break;
#endif
  		case PROTO_PXX:
  	    setupPulsesPXX(INTERNAL_MODULE);
  	  break;
#ifdef PCB9XT
		  case PROTO_DSM2:
	    case PROTO_MULTI:
      	setupPulsesDsm2( ( g_model.Module[INTERNAL_MODULE].sub_protocol == DSM_9XR ) ? 12 : 6, INTERNAL_MODULE ) ; 
	    break;
#endif
	//	  case PROTO_DSM2:
	////      sei() ;							// Interrupts allowed here
	//      setupPulsesDsm2(6); 
	//    break;
	//  	case PROTO_PPM16 :
	//      setupPulsesPPM();		// Don't enable interrupts through here
	////      // PPM16 pulses are set up automatically within the interrupts
	////    break ;
  	}
	}
	else
	{
		requiredprotocol = g_model.Module[1].protocol ;
		if ( PulsesPaused )
		{
			requiredprotocol = PROTO_OFF ;
		}
  	if ( s_current_protocol[1] != requiredprotocol )
  	{
  	  switch( s_current_protocol[1] )
  	  {	// stop existing protocol hardware
  	    case PROTO_PPM:
					disable_ppm(EXTERNAL_MODULE) ;
  	    break;
  	    case PROTO_PXX:
					disable_pxx(EXTERNAL_MODULE) ;
  	    break;
	      case PROTO_DSM2:
		    case PROTO_MULTI:
					disable_dsm2(EXTERNAL_MODULE) ;
	      break;
//#ifdef ASSAN
//	      case PROTO_ASSAN :
//					disable_assan(EXTERNAL_MODULE) ;
//	      break;
//#endif
#ifdef XFIRE
	      case PROTO_XFIRE :
					disable_xfire(EXTERNAL_MODULE) ;
	      break;
#endif
	//      case PROTO_PPM16 :
	//				disable_main_ppm() ;
	//      break ;
  	  }
		
  	  s_current_protocol[EXTERNAL_MODULE] = requiredprotocol ;
  	  switch(s_current_protocol[EXTERNAL_MODULE])
  	  { // Start new protocol hardware here
  	    case PROTO_PPM:
				  setupPulsesPpmAll(EXTERNAL_MODULE) ;
					init_ppm(EXTERNAL_MODULE) ;
  	    break;
  	    case PROTO_PXX:
					init_pxx(EXTERNAL_MODULE) ;
  	    break;
		    case PROTO_OFF:
				default :
					init_no_pulses( EXTERNAL_MODULE ) ;
  	  	break;
	    	case PROTO_MULTI:
					init_multi(EXTERNAL_MODULE) ;
					DsmInitCounter[port] = 0 ;
					Pass[port] = 0 ;
  	  	break;
	      case PROTO_DSM2:
					init_dsm2(EXTERNAL_MODULE) ;
					DsmInitCounter[port] = 0 ;
					Pass[port] = 0 ;
	      break;
//#ifdef ASSAN
//	      case PROTO_ASSAN :
//					init_assan(EXTERNAL_MODULE) ;
//	      break;
//#endif
#ifdef XFIRE
	      case PROTO_XFIRE :
					init_xfire(EXTERNAL_MODULE) ;
	      break;
#endif
	//      case PROTO_PPM16 :
	//				init_main_ppm( 3000, 1 ) ;		// Initial period 1.5 mS, output on
	//      break ;
  	  }
  	}

	// Set up output data here
		switch(requiredprotocol)
  	{
		  case PROTO_PPM:
  	    setupPulsesPpmAll(EXTERNAL_MODULE);		// Don't enable interrupts through here
  	  break;
  		case PROTO_PXX:
  	    setupPulsesPXX(EXTERNAL_MODULE);
  	  break;
		  case PROTO_DSM2:
	    case PROTO_MULTI:
	//      sei() ;							// Interrupts allowed here
      	setupPulsesDsm2( ( g_model.Module[EXTERNAL_MODULE].sub_protocol == DSM_9XR ) ? 12 : 6, EXTERNAL_MODULE ) ; 
//	      setupPulsesDsm2(6); 
	    break;
//#ifdef ASSAN
//	    case PROTO_ASSAN :
//	      setupPulsesDsm2( 12, EXTERNAL_MODULE ) ;
////	      setupPulsesDsm2(g_model.xppmNCH) ; 
//	    break;
//#endif
#ifdef XFIRE
	    case PROTO_XFIRE :
				setupPulsesXfire() ;
//	      setupPulsesDsm2(g_model.xppmNCH) ; 
	    break;
#endif
	//  	case PROTO_PPM16 :
	//      setupPulsesPPM();		// Don't enable interrupts through here
	////      // PPM16 pulses are set up automatically within the interrupts
	////    break ;
  	}
	}
}
#endif

#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D)
//uint16_t PpmStream[20] =
//{
//	2000,
//	2200,
//	2400,
//	2600,
//	2800,
//	3000,
//	3200,
//	3400,
//	45000-21600,
//	0,0,0,0,0,0,0,0,0,0,0
//} ;

uint16_t TrainerPpmStream[20] =
{
	2000,
	2200,
	2400,
	2600,
	2800,
	3000,
	3200,
	3400,
	45000-21600,
	0,0,0,0,0,0,0,0,0,0,0
} ;



#define PPM_CENTER 1500*2

//void setupPulsesPpm()
//{
//  uint32_t i ;
//	uint32_t total ;
//	uint16_t *ptr ;
//	uint32_t p = (g_model.ppmNCH + 4) * 2 ;
//	if ( p > 16 )
//	{
//		p -= 13 ;
//	}
//  p += g_model.startChannel ; //Channels *2
//	if ( p > NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
//	{
//		p = NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ;	// Don't run off the end		
//	}

//	// Now set up pulses
//	int16_t PPM_range = g_model.extendedLimits ? 640*2 : 512*2;   //range of 0.7..1.7msec

//  //Total frame length = 22.5msec
//  //each pulse is 0.7..1.7ms long with a 0.3ms stop tail
//  //The pulse ISR is 2mhz that's why everything is multiplied by 2
//#ifdef PCBSKY
//  ptr = Pulses ;
//#endif
//#if defined(PCBX9D) || defined(PCB9XT)
//  ptr = ppmStream[0] ;
//#endif

//#ifdef PCBSKY
//	register Pwm *pwmptr ;
	
//	pwmptr = PWM ;
//	pwmptr->PWM_CH_NUM[3].PWM_CDTYUPD = (g_model.ppmDelay*50+300)*2; //Stoplen *2
	
//	uint32_t pol = ( CurrentTrainerSource == TRAINER_SLAVE ) ? g_model.trainPulsePol : g_model.pulsePol ;
	
//	if (pol == 0)
//	{
//		pwmptr->PWM_CH_NUM[3].PWM_CMR |= 0x00000200 ;	// CPOL
//	}
//	else
//	{
//		pwmptr->PWM_CH_NUM[3].PWM_CMR &= ~0x00000200 ;	// CPOL
//	}
//#endif
	
//	total = 22500u*2; //Minimum Framelen=22.5 ms
//  total += (int16_t(g_model.ppmFrameLength))*1000;

//	for ( i = g_model.startChannel ; i < p ; i += 1 )
//	{ //NUM_SKYCHNOUT+EXTRA_SKYCHANNELS
//  	int16_t v = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
//		total -= v ;
//    *ptr++ = v ; /* as Pat MacKenzie suggests */
//	}
//	if ( total<9000 )
//	{
//		total = 9000 ;
//	}
//	*ptr++ = total ;
//	*ptr = 0 ;
//#if defined(PCBX9D) || defined(PCB9XT)
//	TIM1->CCR2 = total - 1000 ;		// Update time
//	TIM1->CCR3 = (g_model.ppmDelay*50+300)*2 ;
//  if(!g_model.pulsePol)
//#ifdef PCB9XT
//	  TIM1->CCER &= ~TIM_CCER_CC3P ;
//#else
//    TIM1->CCER |= TIM_CCER_CC3P;
//#endif
//	else
//	{
//#ifdef PCB9XT
//    TIM1->CCER |= TIM_CCER_CC3P;
//#else
//	  TIM1->CCER &= ~TIM_CCER_CC3P ;
//#endif
//	}
//#endif
//}

//void setupPulsesPpmx()
//{
//	// To DO !!!
//  uint32_t i ;
//	uint32_t total ;
//	uint16_t *ptr ;
////  uint32_t p=8+g_model.xppmNCH*2 + g_model.startChannel ; //Channels *2
//	uint32_t p = (g_model.xppmNCH + 4) * 2 ;
//	if ( p > 16 )
//	{
//		p -= 13 ;
//	}
//  p += g_model.xstartChannel ; //Channels *2
//	if ( p > NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
//	{
//		p = NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ;	// Don't run off the end		
//	}
//	int16_t PPM_range = g_model.extendedLimits ? 640*2 : 512*2;   //range of 0.7..1.7msec

//  ptr = ppmStream[1] ;

//	total = 22500u*2; //Minimum Framelen=22.5 ms
//  total += (int16_t(g_model.xppmFrameLength))*1000;

//	for ( i = g_model.xstartChannel ; i < p ; i += 1 )
//	{
////  	pulse = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
		
//  	int16_t v = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
		
//		total -= v ;
//		*ptr++ = v ;
//	}
//	if ( total<9000 )
//	{
//		total = 9000 ;
//	}
//	*ptr++ = total ;
//	*ptr = 0 ;
//	TIM8->CCR2 = total - 1000 ;		// Update time
//	TIM8->CCR3 = (g_model.xppmDelay*50+300)*2 ;
//  if(!g_model.xpulsePol)
//#ifdef PCB9XT
//	  TIM8->CCER &= ~TIM_CCER_CC1NP ;
//#else
//    TIM8->CCER |= TIM_CCER_CC1NP;
//#endif
//	else
//	{
//#ifdef PCB9XT
//    TIM8->CCER |= TIM_CCER_CC1NP;
//#else
//	  TIM8->CCER &= ~TIM_CCER_CC1NP ;
//#endif
//	}
//}

// Pxx has 2 flags and 17 data bytes
// 17 data bytes = 136 bits
// If all bits are 1, then we need 27 stuff bits
// Total bits flags+data+stuff = 16 + 136 + 27 = 179 bits
// Total transisitions = 179*2 = 358

// Using Timer 1, also used by PPM output

//uint16_t PxxStream[400] ;		// Transisitions

//void init_pxx()
//{
//	// Timer1
//	setupPulsesPXX(0) ;
	
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
//	configure_pins( 0x0100, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
//	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;		// Enable clock
//	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN ;			// Enable DMA1 clock
	
//	TIM1->CR1 &= ~TIM_CR1_CEN ;
//	TIM1->ARR = 18000 ;			// 9mS
//	TIM1->CCR2 = 15000 ;		// Update time
//	TIM1->PSC = (Peri2_frequency*Timer_mult2) / 2000000 - 1 ;		// 0.5uS from 30MHz
//	TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P ;	
//	TIM1->CR2 = TIM_CR2_OIS1 ;			// O/P idle high
//	TIM1->BDTR = TIM_BDTR_MOE ;		// Enable outputs
//	TIM1->CCR1 = pxxStream[0][0] ;
//	TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_0 ;			// Force O/P high
// 	TIM1->EGR = 1 ;								// Restart

////	TIM1->SR &= ~TIM_SR_UIF ;				// Clear flag
////	TIM1->SR &= ~TIM_SR_CC2IF ;				// Clear flag
//	TIM1->DIER |= TIM_DIER_CC1DE ;		// Enable DMA on CC1 match
//	TIM1->DCR = 13 ;								// DMA to CC1

////	TIM1->CR1 = TIM_CR1_OPM ;				// Just run once
//	// Enable the DMA channel here, DMA2 stream 1, channel 6
//	DMA2_Stream1->CR &= ~DMA_SxCR_EN ;		// Disable DMA
//	DMA2->LIFCR = DMA_LIFCR_CTCIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CFEIF1 ; // Write ones to clear bits
//	DMA2_Stream1->CR = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MSIZE_0 
//										 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL ;
//	DMA2_Stream1->PAR = (uint32_t) &TIM1->DMAR ;
//	DMA2_Stream1->M0AR = (uint32_t) &pxxStream[0][1] ;
////	DMA2_Stream1->FCR = 0x05 ; //DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0 ;
////	DMA2_Stream1->NDTR = 100 ;
//	DMA2_Stream1->CR |= DMA_SxCR_EN ;		// Enable DMA

//	TIM1->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0 ;			// Toggle CC1 o/p
//	TIM1->SR &= ~TIM_SR_CC2IF ;				// Clear flag
//	TIM1->DIER |= TIM_DIER_CC2IE ;	// Enable this interrupt
//	TIM1->CR1 |= TIM_CR1_CEN ;
//	NVIC_EnableIRQ(TIM1_CC_IRQn) ;

//}

//void disable_pxx()
//{
//	NVIC_DisableIRQ(TIM1_CC_IRQn) ;
//	TIM1->DIER &= ~TIM_DIER_CC2IE ;
//	TIM1->CR1 &= ~TIM_CR1_CEN ;
//}


//const uint16_t CRCTable[]=
//{
//    0x0000,0x1189,0x2312,0x329b,0x4624,0x57ad,0x6536,0x74bf,
//    0x8c48,0x9dc1,0xaf5a,0xbed3,0xca6c,0xdbe5,0xe97e,0xf8f7,
//    0x1081,0x0108,0x3393,0x221a,0x56a5,0x472c,0x75b7,0x643e,
//    0x9cc9,0x8d40,0xbfdb,0xae52,0xdaed,0xcb64,0xf9ff,0xe876,
//    0x2102,0x308b,0x0210,0x1399,0x6726,0x76af,0x4434,0x55bd,
//    0xad4a,0xbcc3,0x8e58,0x9fd1,0xeb6e,0xfae7,0xc87c,0xd9f5,
//    0x3183,0x200a,0x1291,0x0318,0x77a7,0x662e,0x54b5,0x453c,
//    0xbdcb,0xac42,0x9ed9,0x8f50,0xfbef,0xea66,0xd8fd,0xc974,
//    0x4204,0x538d,0x6116,0x709f,0x0420,0x15a9,0x2732,0x36bb,
//    0xce4c,0xdfc5,0xed5e,0xfcd7,0x8868,0x99e1,0xab7a,0xbaf3,
//    0x5285,0x430c,0x7197,0x601e,0x14a1,0x0528,0x37b3,0x263a,
//    0xdecd,0xcf44,0xfddf,0xec56,0x98e9,0x8960,0xbbfb,0xaa72,
//    0x6306,0x728f,0x4014,0x519d,0x2522,0x34ab,0x0630,0x17b9,
//    0xef4e,0xfec7,0xcc5c,0xddd5,0xa96a,0xb8e3,0x8a78,0x9bf1,
//    0x7387,0x620e,0x5095,0x411c,0x35a3,0x242a,0x16b1,0x0738,
//    0xffcf,0xee46,0xdcdd,0xcd54,0xb9eb,0xa862,0x9af9,0x8b70,
//    0x8408,0x9581,0xa71a,0xb693,0xc22c,0xd3a5,0xe13e,0xf0b7,
//    0x0840,0x19c9,0x2b52,0x3adb,0x4e64,0x5fed,0x6d76,0x7cff,
//    0x9489,0x8500,0xb79b,0xa612,0xd2ad,0xc324,0xf1bf,0xe036,
//    0x18c1,0x0948,0x3bd3,0x2a5a,0x5ee5,0x4f6c,0x7df7,0x6c7e,
//    0xa50a,0xb483,0x8618,0x9791,0xe32e,0xf2a7,0xc03c,0xd1b5,
//    0x2942,0x38cb,0x0a50,0x1bd9,0x6f66,0x7eef,0x4c74,0x5dfd,
//    0xb58b,0xa402,0x9699,0x8710,0xf3af,0xe226,0xd0bd,0xc134,
//    0x39c3,0x284a,0x1ad1,0x0b58,0x7fe7,0x6e6e,0x5cf5,0x4d7c,
//    0xc60c,0xd785,0xe51e,0xf497,0x8028,0x91a1,0xa33a,0xb2b3,
//    0x4a44,0x5bcd,0x6956,0x78df,0x0c60,0x1de9,0x2f72,0x3efb,
//    0xd68d,0xc704,0xf59f,0xe416,0x90a9,0x8120,0xb3bb,0xa232,
//    0x5ac5,0x4b4c,0x79d7,0x685e,0x1ce1,0x0d68,0x3ff3,0x2e7a,
//    0xe70e,0xf687,0xc41c,0xd595,0xa12a,0xb0a3,0x8238,0x93b1,
//    0x6b46,0x7acf,0x4854,0x59dd,0x2d62,0x3ceb,0x0e70,0x1ff9,
//    0xf78f,0xe606,0xd49d,0xc514,0xb1ab,0xa022,0x92b9,0x8330,
//    0x7bc7,0x6a4e,0x58d5,0x495c,0x3de3,0x2c6a,0x1ef1,0x0f78
//};

//void crc( uint8_t data )
//{
//    //	uint8_t i ;

//  PcmCrc =(PcmCrc<<8) ^(CRCTable[((PcmCrc>>8)^data)&0xFF]);
//}


// PPM output
// Timer 1, channel 1 on PA8 for prototype
// Pin is AF1 function for timer 1
void init_main_ppm()
{
  setupPulses(0) ;
	init_ppm(0) ;
	// Timer1
//	setupPulsesPpm() ;
//	PulsePtr = PpmStream ;
	
//	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ; 		// Enable portA clock
//	configure_pins( 0x0100, PIN_PERIPHERAL | PIN_PORTA | PIN_PER_1 | PIN_OS25 | PIN_PUSHPULL ) ;
//	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN ;		// Enable clock
	
//	TIM1->ARR = *PulsePtr++ ;
//	TIM1->PSC = (Peri2_frequency*Timer_mult2) / 2000000 - 1 ;		// 0.5uS from 30MHz
//	TIM1->CCER = TIM_CCER_CC1E ;	
//	TIM1->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC2PE ;			// PWM mode 1
//	TIM1->CCR1 = 600 ;		// 300 uS pulse
//	TIM1->BDTR = TIM_BDTR_MOE ;
// 	TIM1->EGR = 1 ;
//	TIM1->DIER = TIM_DIER_UDE ;

//	TIM1->SR &= ~TIM_SR_UIF ;				// Clear flag
//	TIM1->SR &= ~TIM_SR_CC2IF ;				// Clear flag
//	TIM1->DIER |= TIM_DIER_CC2IE ;
//	TIM1->DIER |= TIM_DIER_UIE ;

//	TIM1->CR1 = TIM_CR1_CEN ;
//	NVIC_EnableIRQ(TIM1_CC_IRQn) ;
//	NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn) ;

}

void disable_main_ppm()
{
	disable_ppm(0) ;
//	NVIC_DisableIRQ(TIM1_CC_IRQn) ;
//	NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn) ;
//	TIM1->DIER &= ~TIM_DIER_CC2IE & ~TIM_DIER_UIE ;
//	TIM1->CR1 &= ~TIM_CR1_CEN ;
}


//extern "C" void TIM1_CC_IRQHandler()
//{
//	TIM1->DIER &= ~TIM_DIER_CC2IE ;		// stop this interrupt
//	TIM1->SR &= ~TIM_SR_CC2IF ;				// Clear flag

//	setupPulses() ;

//	if ( Current_protocol == PROTO_PPM )
//	{

//		PulsePtr = PpmStream ;
	
//		TIM1->DIER |= TIM_DIER_UDE ;


//		TIM1->SR &= ~TIM_SR_UIF ;					// Clear this flag
//		TIM1->DIER |= TIM_DIER_UIE ;				// Enable this interrupt
//	}
//	else if ( Current_protocol == PROTO_PXX )
//	{
//		DMA2_Stream1->CR &= ~DMA_SxCR_EN ;		// Disable DMA
//		DMA2->LIFCR = DMA_LIFCR_CTCIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CFEIF1 ; // Write ones to clear bits
//		DMA2_Stream1->M0AR = (uint32_t) &pxxStream[1] ;
//		DMA2_Stream1->CR |= DMA_SxCR_EN ;		// Enable DMA
//		TIM1->CCR1 = pxxStream[0] ;
//		TIM1->DIER |= TIM_DIER_CC2IE ;	// Enable this interrupt
//	}
//}

//extern "C" void TIM1_UP_TIM10_IRQHandler()
//{
//	TIM1->SR &= ~TIM_SR_UIF ;				// Clear flag

//	TIM1->ARR = *PulsePtr++ ;
//	if ( *PulsePtr == 0 )
//	{
//		TIM1->SR &= ~TIM_SR_CC2IF ;			// Clear this flag
//		TIM1->DIER |= TIM_DIER_CC2IE ;	// Enable this interrupt
//	}
//}


//void hwTimerStart()
//{
//	TIM13->CNT = 0 ;
//	TIM13->EGR = 1 ;		// Re-start counter
//}

#ifdef PCB9XT
uint16_t hwTimerValue()
{
	return TIM13->CNT ;
}
#endif

// Trainer PPM output PC9, Timer 3 channel 4, (Alternate Function 2)
// Horus:
// Trainer PPM output PC7, Timer 3 channel 2, (Alternate Function 2)
// Use channel 3 not 1 for main timing
void setupTrainerPulses()
{
  uint32_t i ;
	uint32_t total ;
	uint32_t pulse ;
	uint16_t *ptr ;
	uint32_t p = (g_model.ppmNCH + 4) ;
	if ( p > 16 )
	{
		p = 8 ;
	}
	int16_t PPM_range = g_model.extendedLimits ? 640*2 : 512*2;   //range of 0.7..1.7msec

  if(g_model.trainPulsePol)
	{
#ifdef PCBX12D
		TIM3->CCER = TIM_CCER_CC2E | TIM_CCER_CC2P ;
	}
	else
	{
		TIM3->CCER = TIM_CCER_CC2E ;
#else
		TIM3->CCER = TIM_CCER_CC4E | TIM_CCER_CC4P ;
	}
	else
	{
		TIM3->CCER = TIM_CCER_CC4E ;
#endif
	}
  ptr = TrainerPpmStream ;

	total = 22500u*2; //Minimum Framelen=22.5 ms
  total += (int16_t(g_model.ppmFrameLength))*1000;

	p += g_model.startChannel ;
	for ( i = g_model.startChannel ; i < p ; i += 1 )
	{
  	pulse = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
		
		total -= pulse ;
		*ptr++ = pulse ;
	}
	*ptr++ = total ;
	*ptr = 0 ;
#ifdef PCBX12D
	TIM3->CCR3 = total - 1000 ;		// Update time
	TIM3->CCR2 = (g_model.ppmDelay*50+300)*2 ;
#else
	TIM3->CCR1 = total - 1000 ;		// Update time
	TIM3->CCR4 = (g_model.ppmDelay*50+300)*2 ;
#endif  
}


// Trainer PPM output PC9, Timer 3 channel 4, (Alternate Function 2)
// Horus:
// Trainer PPM output PC7, Timer 3 channel 2, (Alternate Function 2)
// Use channel 3 not 1 for main timing
void init_trainer_ppm()
{
	setupTrainerPulses() ;
	TrainerPulsePtr = TrainerPpmStream ;
	
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
	configure_pins( PIN_TR_PPM_OUT, PIN_PERIPHERAL | PIN_PORTC | PIN_PER_2 | PIN_OS25 | PIN_PUSHPULL ) ;
	configure_pins( PIN_TRNDET, PIN_INPUT | PIN_PULLUP | PIN_PORTB ) ;
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN ;            // Enable clock
	
  TIM3->ARR = *TrainerPulsePtr++ ;
  TIM3->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ;               // 0.5uS
  
  if(g_model.trainPulsePol)
	{
#ifdef PCBX12D
		TIM3->CCER = TIM_CCER_CC2E | TIM_CCER_CC2P ;
	}
	else
	{
		TIM3->CCER = TIM_CCER_CC2E ;
#else
		TIM3->CCER = TIM_CCER_CC4E | TIM_CCER_CC4P ;
	}
	else
	{
		TIM3->CCER = TIM_CCER_CC4E ;
#endif
	}
	
#ifdef PCBX12D
  TIM3->CCMR1 = TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE ;		// PWM mode 1
	TIM3->CCR2 = 600 ;		// 300 uS pulse
#else
  TIM3->CCMR2 = TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE ;		// PWM mode 1
	TIM3->CCR4 = 600 ;		// 300 uS pulse
#endif
	TIM3->BDTR = TIM_BDTR_MOE ;
 	TIM3->EGR = 1 ;
//	TIM8->DIER = TIM_DIER_UDE ;

	TIM3->SR &= ~TIM_SR_UIF ;				// Clear flag
#ifdef PCBX12D
#else
	TIM3->SR &= ~TIM_SR_CC3IF ;				// Clear flag
	TIM3->DIER |= TIM_DIER_CC3IE ;
#endif
	TIM3->DIER |= TIM_DIER_UIE ;

	TIM3->CR1 = TIM_CR1_CEN ;
  NVIC_SetPriority(TIM3_IRQn, 7);
	NVIC_EnableIRQ(TIM3_IRQn) ;
}

// TODO - testing
void stop_trainer_ppm()
{
	configure_pins( PIN_TR_PPM_OUT, PIN_INPUT | PIN_PORTC ) ;
	TIM3->DIER = 0 ;
	TIM3->CR1 &= ~TIM_CR1_CEN ;				// Stop counter
	NVIC_DisableIRQ(TIM3_IRQn) ;				// Stop Interrupt
}

// Trainer capture, PC8, Timer 3 channel 3
// Soft Serial capture, PC8, Timer 3 channel 3
// Horus:
// Trainer capture, PC6, Timer 3 channel 3
// Soft Serial capture, PC6, Timer 3 channel 1, (Alternate Function 2)
void init_trainer_capture(uint32_t mode)
{
	struct t_softSerial *pss = &SoftSerial1 ;
	
	if ( CaptureMode != CAP_COM1 )
	{	
		CaptureMode = mode ;
	}
	
	stop_trainer_ppm() ;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
	configure_pins( PIN_TR_PPM_IN, PIN_PERIPHERAL | PIN_PORTC | PIN_PER_2 | PIN_PULLUP ) ;
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN ;		// Enable clock
	
	TIM3->ARR = 0xFFFF ;
	TIM3->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ;		// 0.5uS
	TIM3->CR2 = 0 ;
#ifndef PCBX12D
	if ( mode == CAP_SERIAL )
	{
		pss->lineState = LINE_IDLE ;
		pss->bitTime = BIT_TIME_100K ;
		pss->softSerialEvenParity = 1 ;
		pss->softSerInvert = TrainerPolarity ;
#ifdef PCBX12D
#else
		TIM3->CCMR2 = TIM_CCMR2_IC4F_0 | TIM_CCMR2_IC4F_1 | TIM_CCMR2_CC4S_1 | TIM_CCMR2_IC3F_0 | TIM_CCMR2_IC3F_1 | TIM_CCMR2_CC3S_0 ;
		TIM3->CCER = TIM_CCER_CC3E | TIM_CCER_CC4E | TIM_CCER_CC4P ;
		TIM3->SR &= ~(TIM_SR_CC4IF | TIM_SR_CC3IF) ;				// Clear flags
		TIM3->DIER |= TIM_DIER_CC4IE | TIM_DIER_CC3IE ;
#endif
  	NVIC_SetPriority(TIM3_IRQn, 0 ) ;
		start_timer11() ;
	}
	else
#endif
	{
#ifdef PCBX12D
		TIM3->CCMR1 = TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1 | TIM_CCMR1_CC1S_0 ;
		TIM3->CCER = TIM_CCER_CC1E ;
		TIM3->SR &= ~TIM_SR_CC1IF ;				// Clear flag
		TIM3->DIER |= TIM_DIER_CC1IE ;
#else
		TIM3->CCMR2 = TIM_CCMR2_IC3F_0 | TIM_CCMR2_IC3F_1 | TIM_CCMR2_CC3S_0 ;
		TIM3->CCER = TIM_CCER_CC3E ;	 
		TIM3->SR &= ~TIM_SR_CC3IF ;				// Clear flag
		TIM3->DIER |= TIM_DIER_CC3IE ;
#endif
  	NVIC_SetPriority(TIM3_IRQn, 7);
	}
	
	TIM3->CR1 = TIM_CR1_CEN ;
	NVIC_EnableIRQ(TIM3_IRQn) ;
}

void stop_trainer_capture()
{
	TIM3->DIER = 0 ;
	TIM3->CR1 &= ~TIM_CR1_CEN ;				// Stop counter
	NVIC_DisableIRQ(TIM3_IRQn) ;				// Stop Interrupt
	if ( CaptureMode == CAP_SERIAL )
	{
		stop_timer11() ;
		CaptureMode = 0 ;
	}
}

#ifndef PCBX12D
void init_serial_trainer_capture()
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN ; 		// Enable portB clock
	configure_pins( 0x0800, PIN_PERIPHERAL | PIN_PORTB | PIN_PER_1 ) ;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN ;		// Enable clock
	
	TIM2->ARR = 0xFFFF ;
	TIM2->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ;		// 0.5uS
	TIM2->CR2 = 0 ;
	TIM2->CCMR2 = TIM_CCMR2_IC4F_0 | TIM_CCMR2_IC4F_1 | TIM_CCMR2_CC4S_0 ;
	TIM2->CCER = TIM_CCER_CC4E ;
	TIM2->SR &= ~TIM_SR_CC4IF ;				// Clear flag
	TIM2->DIER |= TIM_DIER_CC4IE ;
	TIM2->CR1 = TIM_CR1_CEN ;
  NVIC_SetPriority(TIM2_IRQn, 7);
	NVIC_EnableIRQ(TIM2_IRQn) ;
}

void stop_serial_trainer_capture()
{
	TIM2->DIER = 0 ;
	TIM2->CR1 &= ~TIM_CR1_CEN ;				// Stop counter
	NVIC_DisableIRQ(TIM2_IRQn) ;				// Stop Interrupt
}

// Capture interrupt for trainer input
extern "C" void TIM2_IRQHandler()
{
  
	uint16_t capture = 0 ;
  static uint16_t lastCapt ;
  uint16_t val ;
	uint32_t doCapture = 0 ;
	
  // What mode? in or out?
  if ( (TIM2->DIER & TIM_DIER_CC4IE ) && ( TIM2->SR & TIM_SR_CC4IF ) )
    // capture mode
	{	
		capture = TIM2->CCR4 ;
		doCapture = 1 ;
	}

	TrainerProfile *tProf = &g_eeGeneral.trainerProfile[g_model.trainerProfile] ;
	if ( tProf->channel[0].source != TRAINER_JACK )
	{
		doCapture = 0 ;
	}

	if ( doCapture )
	{
  	val = (uint16_t)(capture - lastCapt) / 2 ;
  	lastCapt = capture;

  	// We prcoess g_ppmInsright here to make servo movement as smooth as possible
  	//    while under trainee control
  	if ((val>4000) && (val < 19000)) // G: Prioritize reset pulse. (Needed when less than 8 incoming pulses)
		{
  	  ppmInState = 1; // triggered
		}
  	else
  	{
  		if(ppmInState && (ppmInState<=16))
			{
  	  	if((val>800) && (val<2200))
				{
					ppmInValid = 100 ;
//  		    g_ppmIns[ppmInState++ - 1] = (int16_t)(val - 1500)*(g_eeGeneral.PPM_Multiplier+10)/10; //+-500 != 512, but close enough.
  		    g_ppmIns[ppmInState++ - 1] = (int16_t)(val - 1500) ; //+-500 != 512, but close enough.

		    }else{
  		    ppmInState=0; // not triggered
  	  	}
  	  }
  	}
	}
}

void init_cppm_on_heartbeat_capture()
{
#ifdef PCBX9D
	stop_xjt_heartbeat() ;
#endif
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN ; 		// Enable portC clock
	configure_pins( 0x0080, PIN_PERIPHERAL | PIN_PORTC | PIN_PER_2 ) ;
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN ;		// Enable clock
	
	TIM3->ARR = 0xFFFF ;
	TIM3->PSC = (PeripheralSpeeds.Peri1_frequency*PeripheralSpeeds.Timer_mult1) / 2000000 - 1 ;		// 0.5uS
	TIM3->CR2 = 0 ;
	TIM3->CCMR1 = TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_CC2S_0 ;
	TIM3->CCER = TIM_CCER_CC2E ;	 
	TIM3->SR &= ~TIM_SR_CC2IF ;				// Clear flag
	TIM3->DIER |= TIM_DIER_CC2IE ;
	TIM3->CR1 = TIM_CR1_CEN ;
  NVIC_SetPriority(TIM3_IRQn, 7);
	NVIC_EnableIRQ(TIM3_IRQn) ;
}

void stop_cppm_on_heartbeat_capture()
{
	TIM3->DIER = 0 ;
	TIM3->CR1 &= ~TIM_CR1_CEN ;				// Stop counter
	NVIC_DisableIRQ(TIM3_IRQn) ;				// Stop Interrupt
#ifdef PCBX9D
	init_xjt_heartbeat() ;
#endif
}
#endif

//uint16_t CapTimes[12] ;
//uint16_t CapLevels[12] ;
//uint16_t CapIndex ;
//uint16_t CapCount ;

//void putCapValues( uint16_t time, uint16_t level )
//{
//	if ( CapIndex < 12 )
//	{
//		CapTimes[CapIndex] = time ;
//		CapLevels[CapIndex] = level ;
//		CapIndex += 1 ;
//	}
//}

//uint16_t TIM3Captures ;

// Capture interrupt for trainer input
extern "C" void TIM3_IRQHandler()
{
  
	uint16_t capture = 0 ;
  static uint16_t lastCapt ;
  uint16_t val ;
	uint32_t doCapture = 0 ;
	struct t_softSerial *pss = &SoftSerial1 ;
	if ( CaptureMode == CAP_SERIAL )
	{
//		TIM3Captures += 1 ;
		// CC3 physical rising, CC4 falling edge
		// So for inverted serial CC3 is HtoL, CC4 is LtoH
		if ( LastTransition == 0 )	// LtoH
		{
			capture = TIM3->CCR4 ;
			LastTransition = 1 ;
		}
		else
		{
			capture = TIM3->CCR3 ;
			LastTransition = 0 ;
		}

  	val = LastTransition ;
		if ( pss->softSerInvert )
		{
			val = !val ;
		}
		
		if ( val == 0 )	// LtoH
		{
			// L to H transition
			pss->LtoHtime = capture ;
			TIM11->CNT = 0 ;
			TIM11->CCR1 = pss->bitTime * 12 ;
			uint32_t time ;
			capture -= pss->HtoLtime ;
			time = capture ;
//			putCapValues( time, 0 ) ;
			putCaptureTime( pss, time, 0 ) ;
			TIM11->DIER = TIM_DIER_CC1IE ;
		}
		else
		{
			// H to L transition
			pss->HtoLtime = capture ;
			if ( pss->lineState == LINE_IDLE )
			{
				pss->lineState = LINE_ACTIVE ;
//				if ( ++CapCount > 200 )
//				{
//					CapIndex = 0 ;
//					CapCount = 0 ;
//				}
//				putCapValues( 0, 3 ) ;
				putCaptureTime( pss, 0, 3 ) ;
			}
			else
			{
				uint32_t time ;
				capture -= pss->LtoHtime ;
				time = capture ;
//				putCapValues( time, 1 ) ;
				putCaptureTime( pss, time, 1 ) ;
			}
			TIM11->DIER = 0 ;
			TIM11->CCR1 = pss->bitTime * 20 ;
			TIM11->CNT = 0 ;
			TIM11->SR = 0 ;
		}
		return ;
	}
	else
	{
  	// What mode? in or out?
#ifdef PCBX12D
  	if ( (TIM3->DIER & TIM_DIER_CC1IE ) && ( TIM3->SR & TIM_SR_CC1IF ) )
  	  // capture mode
		{	
			capture = TIM3->CCR1 ;
			doCapture = 1 ;
		}
#else
  	if ( (TIM3->DIER & TIM_DIER_CC3IE ) && ( TIM3->SR & TIM_SR_CC3IF ) )
  	  // capture mode
		{	
			capture = TIM3->CCR3 ;
			doCapture = 1 ;
		}
#endif

#ifndef PCBX12D
  	if ( (TIM3->DIER & TIM_DIER_CC2IE ) && ( TIM3->SR & TIM_SR_CC2IF ) )
  	  // capture mode
		{	
			capture = TIM3->CCR2 ;
			doCapture = 1 ;
		}
#endif

		if ( doCapture )
		{
  		val = (uint16_t)(capture - lastCapt) / 2 ;
  		lastCapt = capture;

  		// We prcoess g_ppmInsright here to make servo movement as smooth as possible
  		//    while under trainee control
  		if ((val>4000) && (val < 19000)) // G: Prioritize reset pulse. (Needed when less than 8 incoming pulses)
			{
  		  ppmInState = 1; // triggered
			}
  		else
  		{
  			if(ppmInState && (ppmInState<=16))
				{
  		  	if((val>800) && (val<2200))
					{
						ppmInValid = 100 ;
	//  		    g_ppmIns[ppmInState++ - 1] = (int16_t)(val - 1500)*(g_eeGeneral.PPM_Multiplier+10)/10; //+-500 != 512, but close enough.
  			    g_ppmIns[ppmInState++ - 1] = (int16_t)(val - 1500) ; //+-500 != 512, but close enough.

			    }else{
  			    ppmInState=0; // not triggered
  		  	}
  		  }
  		}
		}

  	// PPM out compare interrupt
  	if ( ( TIM3->DIER & TIM_DIER_CC1IE ) && ( TIM3->SR & TIM_SR_CC1IF ) )
		{
  	  // compare interrupt
  	  TIM3->DIER &= ~TIM_DIER_CC1IE ;         // stop this interrupt
  	  TIM3->SR &= ~TIM_SR_CC1IF ;                             // Clear flag

  	  setupTrainerPulses() ;

			TrainerPulsePtr = TrainerPpmStream ;
  	  TIM3->DIER |= TIM_DIER_UDE ;
  	  TIM3->SR &= ~TIM_SR_UIF ;                                       // Clear this flag
  	  TIM3->DIER |= TIM_DIER_UIE ;                            // Enable this interrupt
  	}

  	// PPM out update interrupt
  	if ( (TIM3->DIER & TIM_DIER_UIE) && ( TIM3->SR & TIM_SR_UIF ) )
		{
  	  TIM3->SR &= ~TIM_SR_UIF ;                               // Clear flag
  	  TIM3->ARR = *TrainerPulsePtr++ ;
  	  if ( *TrainerPulsePtr == 0 )
			{
  	    TIM3->SR &= ~TIM_SR_CC1IF ;                     // Clear this flag
  	    TIM3->DIER |= TIM_DIER_CC1IE ;  // Enable this interrupt
  	  }
  	}
	}
}

void dsmBindResponse( uint8_t mode, int8_t channels )
{
	// Process mode here
	uint8_t dsm_mode_response ;
//#ifdef ASSAN
//	if ( g_model.xprotocol == PROTO_ASSAN )
//	{
//		dsm_mode_response = mode & ( ORTX_USE_DSMX | ORTX_USE_11mS | ORTX_USE_11bit | ORTX_USE_TM ) ;
//		g_model.dsmMode = dsm_mode_response ;
//  	STORE_MODELVARS ;
//		PxxFlag[0] &= ~PXX_BIND ;
//		PxxFlag[1] &= ~PXX_BIND ;
//	}
//	else
//#endif
	{
		dsm_mode_response = mode & ( ORTX_USE_DSMX | ORTX_USE_11mS | ORTX_USE_11bit | ORTX_AUTO_MODE ) ;
		if ( g_model.Module[1].protocol != PROTO_MULTI )
		{
#if defined(PCBX9D) || defined(PCB9XT)
			if ( ( g_model.Module[1].channels != channels ) || ( g_model.dsmMode != ( dsm_mode_response | 0x80 ) ) )
			{
				g_model.xppmNCH = channels ;
				g_model.Module[1].channels = channels ;
#else
			if ( ( g_model.Module[1].channels != channels ) || ( g_model.dsmMode != ( dsm_mode_response | 0x80 ) ) )
			{
				g_model.Module[1].channels = channels ;
#endif
				g_model.dsmMode = dsm_mode_response | 0x80 ;
		  	STORE_MODELVARS ;
			}
		}
		else
		{
extern uint8_t MultiResponseData ;
		dsm_mode_response = channels ;
		if ( mode & 0x80 )
		{
			dsm_mode_response |= 0x80 ;
		}
		if ( mode & 0x10 )
		{
			dsm_mode_response |= 0x40 ;
		}
		MultiResponseData = dsm_mode_response ;
extern uint8_t MultiResponseFlag ;
			MultiResponseFlag = 1 ;
		}
	}
}


#ifdef XFIRE

// CRC8 implementation with polynom = x^8+x^7+x^6+x^4+x^2+1 (0xD5)
unsigned char crc8tab[256] = {
  0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54,
  0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
  0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06,
  0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
  0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0,
  0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
  0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2,
  0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
  0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9,
  0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
  0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B,
  0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
  0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D,
  0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
  0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F,
  0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
  0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB,
  0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
  0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9,
  0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
  0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F,
  0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
  0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D,
  0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
  0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26,
  0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
  0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74,
  0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
  0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82,
  0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
  0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0,
  0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
};

uint8_t crc8(const uint8_t * ptr, uint32_t len)
{
  uint8_t crc = 0;
  for ( uint32_t i=0 ; i<len ; i += 1 )
	{
    crc = crc8tab[crc ^ *ptr++] ;
  }
  return crc;
}

#define CROSSFIRE_CH_CENTER         0x3E0
#define CROSSFIRE_CH_BITS           11
#define CROSSFIRE_CHANNELS_COUNT		16

#define MODULE_ADDRESS							0xEE
#define CHANNELS_ID									0x16

// Range for pulses (channels output) is [-1024:+1024]
uint8_t setupPulsesXfire()
{
	uint32_t startChan ;
	startChan = g_model.startChannel ;
  uint8_t *buf = Bit_pulses ;
  *buf++ = MODULE_ADDRESS ;
  *buf++ = 24 ; // 1(ID) + 22 + 1(CRC)
  uint8_t *crc_start = buf ;
  *buf++ = CHANNELS_ID ;
  uint32_t bits = 0 ;
  uint32_t bitsavailable = 0 ;
  for (uint32_t i=0 ; i < CROSSFIRE_CHANNELS_COUNT ; i += 1 )
	{
    uint32_t val = limit(0, CROSSFIRE_CH_CENTER + (((g_chans512[startChan+i]) * 4) / 5), 2*CROSSFIRE_CH_CENTER) ;
    bits |= val << bitsavailable ;
    bitsavailable += CROSSFIRE_CH_BITS ;
    while (bitsavailable >= 8)
		{
      *buf++ = bits ;
      bits >>= 8 ;
      bitsavailable -= 8 ;
    }
  }
  *buf++ = crc8( crc_start, 23) ;
  return buf - Bit_pulses ;
}


#endif // XFIRE




#endif


#ifndef PCBSKY

#define PPM_CENTER 1500*2

void setupPulsesPpmAll(uint32_t module)
{
  uint32_t i ;
	int32_t total ;
	uint16_t *ptr ;
	uint32_t p = (g_model.Module[module].channels + 8) ;
//	if ( p > 16 )
//	{
//		p -= 13 ;
//	}
  p += g_model.Module[module].startChannel ; //Channels *2
	if ( p > NUM_SKYCHNOUT+EXTRA_SKYCHANNELS )
	{
		p = NUM_SKYCHNOUT+EXTRA_SKYCHANNELS ;	// Don't run off the end		
	}

	// Now set up pulses
	int16_t PPM_range = g_model.extendedLimits ? 640*2 : 512*2;   //range of 0.7..1.7msec

  //Total frame length = 22.5msec
  //each pulse is 0.7..1.7ms long with a 0.3ms stop tail
  //The pulse ISR is 2mhz that's why everything is multiplied by 2
#ifdef PCBSKY
  ptr = Pulses2;
	if ( module )
	{
  	ptr = Pulses ;
	}
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D)
  ptr = ppmStream[module] ;
#endif

#ifdef PCBSKY
	register Pwm *pwmptr ;
	uint32_t pwmChannel = module ? 1 : 3 ;
	
	pwmptr = PWM ;
	pwmptr->PWM_CH_NUM[pwmChannel].PWM_CDTYUPD = (g_model.Module[module].ppmDelay*50+300)*2; //Stoplen *2
	
	uint32_t pol = ( CurrentTrainerSource == TRAINER_SLAVE ) ? g_model.trainPulsePol : g_model.Module[module].pulsePol ;
	
	if (pol == 0)
	{
		pwmptr->PWM_CH_NUM[pwmChannel].PWM_CMR |= 0x00000200 ;	// CPOL
	}
	else
	{
		pwmptr->PWM_CH_NUM[pwmChannel].PWM_CMR &= ~0x00000200 ;	// CPOL
	}
#endif
	
	total = 22500u*2; //Minimum Framelen=22.5 ms
  total += (int16_t(g_model.Module[module].ppmFrameLength))*1000;

	for ( i = g_model.Module[module].startChannel ; i < p ; i += 1 )
	{ //NUM_SKYCHNOUT+EXTRA_SKYCHANNELS
  	int16_t v = max( (int)min(g_chans512[i],PPM_range),-PPM_range) + PPM_CENTER;
		total -= v ;
    *ptr++ = v ; /* as Pat MacKenzie suggests */
	}
	if ( total<9000 )
	{
		total = 9000 ;
	}
	*ptr++ = total ;
	*ptr = 0 ;

#ifdef PCBX12D
	pulseStreamCount[module] = ptr - ppmStream[module] ;
#endif

#ifdef PCBX12D
	TIM_TypeDef *pTimer ;
	if ( isProdVersion() )
	{
		if ( module )
		{
			pTimer = EXTMODULE_TIMER ;
		}
		else
		{
			pTimer = EXTMODULE_TIMER ;
		}
	}
	else
	{
		if ( module )
		{
			pTimer = EXTMODULE_TIMER ;
		}
		else
		{
			pTimer = EXTMODULE_TIMER ;
//			pTimer = PROT_INTMODULE_TIMER ;
		}
	}
	pTimer->CCR2 = total - 2000 ;		// Update time
#endif
#if defined(PCBX9D) || defined(PCB9XT)
	TIM_TypeDef *pTimer = TIM1 ;
	uint16_t polarityBit = TIM_CCER_CC3P ;
	if ( module )
	{
		pTimer = TIM8 ;
		polarityBit = TIM_CCER_CC1NP ;
	}
	pTimer->CCR2 = total - 1000 ;		// Update time
	pTimer->CCR3 = (g_model.Module[module].ppmDelay*50+300)*2 ;
  if(!g_model.Module[module].pulsePol)
#ifdef PCB9XT
	  TIM1->CCER &= ~polarityBit ;
#else
    TIM1->CCER |= polarityBit;
#endif
	else
	{
#ifdef PCB9XT
    TIM1->CCER |= polarityBit;
#else
	  TIM1->CCER &= ~polarityBit ;
#endif
	}
#endif
}

extern uint16_t dsm2Stream[][400] ;
uint16_t *dsm2StreamPtr[2] ;
uint16_t dsm2Value[2] ;
uint8_t dsm2Index[2] = {0,0} ;

void _send_1(uint8_t v, uint32_t module )
{
  if (dsm2Index[module] == 0)
    v -= 2;
  else
    v += 2;

  dsm2Value[module] += v;
  *dsm2StreamPtr[module]++ = dsm2Value[module];

  dsm2Index[module] = (dsm2Index[module]+1) % 2;
}

uint8_t BITLEN_Serial = (8*2) ;

uint32_t DsmPassIndex ;

void sendByteDsm2(uint8_t b, uint32_t module) //max 10 changes 0 10 10 10 10 1
{
	uint32_t protocol = g_model.Module[1].protocol ;
#ifdef PCB9XT
	if ( module == INTERNAL_MODULE )
	{
		protocol = g_model.Module[0].protocol ;
	}
#endif
	uint8_t parity = 0x80 ;
	uint8_t count = 8 ;
	uint8_t bitLen ;
	if ( protocol != PROTO_DSM2 )
	{ // SBUS & MULTI
		parity = 0 ;
		bitLen = b ;
		for( uint8_t i=0; i<8; i++)
		{
			parity += bitLen & 0x80 ;
			bitLen <<= 1 ;
		}
		parity &= 0x80 ;
		count = 9 ;
	}
    bool lev = 0;
    uint8_t len = BITLEN_Serial; //max val: 9*16 < 256
    for (uint8_t i=0; i<=count; i++)
		{ //8Bits + Stop=1
        bool nlev = b & 1; //lsb first
        if (lev == nlev)
				{
          len += BITLEN_Serial;
        }
        else
				{
          _send_1(len, module); // _send_1(nlev ? len-5 : len+3);
          len = BITLEN_Serial;
          lev = nlev;
        }
				b = (b>>1) | parity ; //shift in parity or stop bit
				parity = 0x80 ;				// Now a stop bit
    }
    _send_1(len+BITLEN_Serial, module); // _send_1(len+BITLEN_DSM2+3); // 2 stop bits
}

void putDsm2Flush(uint32_t module)
{
  dsm2StreamPtr[module]--; //remove last stopbits and
  *dsm2StreamPtr[module]++ = 45000 ; // Past the 44000 of the ARR
}

static uint8_t dsmDat[2][2+6*2]={{0xFF,0x00, 0x00,0xAA, 0x05,0xFF, 0x09,0xFF, 0x0D,0xFF, 0x13,0x54, 0x14,0xAA},
																 {0xFF,0x00, 0x00,0xAA, 0x05,0xFF, 0x09,0xFF, 0x0D,0xFF, 0x13,0x54, 0x14,0xAA}};


void setupPulsesDsm2(uint8_t channels, uint32_t module )
{
  uint16_t required_baudrate ;
	
	uint32_t protocol = g_model.Module[module].protocol ;
	uint32_t sub_protocol = g_model.Module[module].sub_protocol ;
  
	required_baudrate = SCC_BAUD_125000 ;
	
	if( (protocol == PROTO_DSM2) && ( sub_protocol == DSM_9XR ) )
	{
		required_baudrate = SCC_BAUD_115200 ;
		Dsm_Type[module] = 1 ;
		// Consider inverting COM1 here
	}
	else if(protocol == PROTO_MULTI)
	{
		required_baudrate = SCC_BAUD_100000 ;
		Dsm_Type[module] = 0 ;
	}
	else
	{
		Dsm_Type[module] = 0 ;
	}

	if ( required_baudrate != Scc_baudrate )
	{
		if ( required_baudrate == SCC_BAUD_125000 )
		{
			BITLEN_Serial = (8*2) ;
		}
		else if ( required_baudrate == SCC_BAUD_100000 )
		{
			BITLEN_Serial = (10*2) ;
		}
		else
		{
			BITLEN_Serial = 17 ;
		}
		Scc_baudrate = required_baudrate ;
	}

	if ( Dsm_Type[module] )
	{
		uint8_t channels = 12 ;
		uint8_t flags = g_model.dsmMode ;
		if ( flags == 0 )
		{
			if ( Dsm_Type[module] == 1 )
			{
				flags = ORTX_AUTO_MODE ;
			}
		}
		else
		{
			flags &= 0x7F ;
			channels = g_model.Module[module].channels ;
		}

#ifdef PCB9XT
		if ( (dsmDat[module][0]&BindBit) && (!keyState(SW_Trainer) ) )
#else
		if ( (dsmDat[module][0]&BindBit) && (!keyState(SW_SH2) ) )
#endif
		{
			dsmDat[module][0] &= ~BindBit	;
		}

		uint8_t startChan = g_model.Module[module].startChannel ;

 		dsm2StreamPtr[module] = dsm2Stream[module] ;
  	dsm2Index[module] = 0 ;
  	dsm2Value[module] = 100 ;
  	*dsm2StreamPtr[module]++ = dsm2Value[module] ;
		sendByteDsm2( 0xAA, module );
		if ( Pass[module] == 0 )
		{
  		sendByteDsm2( Pass[module], module ) ;		// Actually is a 0
			// Do init packet
			if ( (PxxFlag[module] & PXX_BIND) || (dsmDat[module][0]&BindBit) )
			{
				flags |= ORTX_BIND_FLAG ;
			}
			// Need to choose dsmx/dsm2 as well
  		sendByteDsm2( flags, module ) ;
  		sendByteDsm2( (PxxFlag[module] & PXX_RANGE_CHECK) ? 4: 7, module ) ;		// 
  		sendByteDsm2( channels, module ) ;			// Max channels
			sendByteDsm2( g_model.Module[module].pxxRxNum, module ) ;	// Model number
	  	putDsm2Flush(module);
			Pass[module] = 1 ;
		}
		else
		{
			DsmPassIndex = 0 ;
			if ( Pass[module] == 2 )
			{
				startChan += 7 ;
			}
  		sendByteDsm2( Pass[module], module );
	
			for(uint8_t i=0 ; i<7 ; i += 1 )
			{
				uint16_t pulse ;
				int16_t value = g_chans512[startChan] ;
				if ( ( flags & ORTX_USE_11bit ) == 0 )
				{
					pulse = limit(0, ((value*13)>>5)+512,1023) | (startChan << 10) ;
				}
				else
				{
					pulse = limit(0, ((value*349)>>9)+1024,2047) | (startChan << 11) ;
				}
				startChan += 1 ;
				if ( startChan <= channels )
				{
  		  	sendByteDsm2( pulse >> 8, module ) ;
    			sendByteDsm2( pulse & 0xff, module ) ;
				}
				else
				{
    			sendByteDsm2( 0xff, module ) ;
    			sendByteDsm2( 0xff, module ) ;
				}
			}
	  	putDsm2Flush(module);
			if ( ++Pass[module] > 2 )
			{
				Pass[module] = 1 ;				
			}
			if ( channels < 8 )
			{
				Pass[module] = 1 ;				
			}
			DsmInitCounter[module] += 1 ;
			if( DsmInitCounter[module] > 100)
			{
				DsmInitCounter[module] = 0 ;
				Pass[module] = 0 ;
			}
			if (dsmDat[module][0]&BindBit)
			{
				Pass[module] = 0 ;		// Stay here
			}
		}
	}
	else
	{
  	dsm2Index[module] = 0 ;
 		dsm2StreamPtr[module] = dsm2Stream[module] ;
  	dsm2Value[module] = 100;
  	*dsm2StreamPtr[module]++ = dsm2Value[module];
		if(protocol == PROTO_DSM2)
		{
 			if (dsmDat[module][0]&BadData)  //first time through, setup header
			{
  			switch(sub_protocol)
  			{
  				case LPXDSM2:
  				  dsmDat[module][0]= 0x80;
  				break;
  				case DSM2only:
  				  dsmDat[module][0]=0x90;
  				break;
  				default:
  				  dsmDat[module][0]=0x98;  //dsmx, bind mode
  				break;
  			}
    	}
	#ifdef PCB9XT
  		if((dsmDat[module][0]&BindBit)&&(!keyState(SW_Trainer)))  dsmDat[module][0]&=~BindBit;		//clear bind bit if trainer not pulled
	#else
  		if((dsmDat[module][0]&BindBit)&&(!keyState(SW_SH2)))  dsmDat[module][0]&=~BindBit;		//clear bind bit if trainer not pulled
	#endif
 			if ((!(dsmDat[module][0]&BindBit))&& (PxxFlag[module] & PXX_RANGE_CHECK)) dsmDat[module][0]|=RangeCheckBit;   //range check function
 			else dsmDat[module][0]&=~RangeCheckBit;
		}
		else // Multi
		{
			dsmDat[module][0] = sub_protocol+1;
			if (PxxFlag[module] & PXX_BIND)	dsmDat[module][0] |=BindBit;		//set bind bit if bind menu is pressed
			if (PxxFlag[module] & PXX_RANGE_CHECK)	dsmDat[module][0] |=RangeCheckBit;		//set bind bit if bind menu is pressed
		}
 		
		if ( protocol == PROTO_MULTI )
		{
			uint32_t outputbitsavailable = 0 ;
			uint32_t outputbits = 0 ;
			uint32_t i ;
			if ( module == 0 )
			{
				sendByteDsm2( ( ( (g_model.Module[module].sub_protocol+1) & 0x3F) > 31 ) ? 0x54 : 0x55, module ) ;
			}
			else
			{
				sendByteDsm2( ( ( (g_model.Module[module].sub_protocol+1) & 0x3F) > 31 ) ? 0x54 : 0x55, module ) ;
			}
			sendByteDsm2( dsmDat[module][0], module ) ;
			
			uint8_t x ;
			if ( module == 0 )
			{
				x = g_model.Module[module].channels ;
				{
					sendByteDsm2(( x & 0xF0) | ( g_model.Module[module].pxxRxNum & 0x0F ), module );
					sendByteDsm2(g_model.Module[module].option_protocol, module);
				}
			}
			else
			{
				x = g_model.Module[module].channels ;
				{
					sendByteDsm2(( x & 0xF0) | ( g_model.Module[module].pxxRxNum & 0x0F ), module );
					sendByteDsm2(g_model.Module[module].option_protocol, module);
			  }
			}
			
			for ( i = 0 ; i < 16 ; i += 1 )
			{
				int16_t x = g_chans512[g_model.Module[module].startChannel+i] ;
				x *= 4 ;
				x += x > 0 ? 4 : -4 ;
				x /= 5 ;
				x += 0x400 ;
				if ( x < 0 )
				{
					x = 0 ;
				}
				if ( x > 2047 )
				{
					x = 2047 ;
				}
				outputbits |= (uint32_t)x << outputbitsavailable ;
				outputbitsavailable += 11 ;
				while ( outputbitsavailable >= 8 )
				{
					uint32_t j = outputbits ;
					sendByteDsm2(j, module) ;
					outputbits >>= 8 ;
					outputbitsavailable -= 8 ;
				}
			}

	  	putDsm2Flush(module);
		}
		else
		{
			dsmDat[module][1] = g_model.Module[module].pxxRxNum ;  //DSM2 Header second byte for model match
  		for( uint8_t i = 0 ; i<channels; i += 1 )
  		{
				uint16_t pulse = limit(0, ((g_chans512[g_model.xstartChannel+i]*13)>>5)+512,1023);
 				dsmDat[module][2+2*i] = (i<<2) | ((pulse>>8)&0x03);
  			dsmDat[module][3+2*i] = pulse & 0xff;
  		}

	  	for (int i=0; i<14; i++)
			{
	  	  sendByteDsm2(dsmDat[module][i], module) ;
	  	}
	  	putDsm2Flush(module) ;
		}
	}
#ifdef PCBX12D
	pulseStreamCount[module] = dsm2StreamPtr[module] - dsm2Stream[module] ;
#endif
}


extern uint16_t pxxStream[2][400];
uint16_t *PtrPxx ;
uint16_t PxxValue ;
uint16_t *PtrPxx_x ;
uint16_t PxxValue_x ;

#ifdef PCBX12D
uint8_t PxxSerial[50] ;
uint8_t *PtrSerialPxx ;
#endif

const uint16_t CRC_Short[]=
{
   0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
   0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7 };

uint16_t CRCTable(uint8_t val)
{
	return CRC_Short[val&0x0F] ^ (0x1081 * (val>>4));
}


static void crc( uint8_t data )
{
  PcmCrc=(PcmCrc<<8) ^ CRCTable((PcmCrc>>8)^data) ;
}


inline __attribute__ ((always_inline)) void putPcmPart( uint8_t value )
{
	uint16_t lpxxValue = PxxValue += 18 ;
	
	*PtrPxx++ = lpxxValue ;
	lpxxValue += 14 ;
	if ( value )
	{
		lpxxValue += 16 ;
	}
	*PtrPxx++ = lpxxValue ;	// Output 0 for this time
	PxxValue = lpxxValue ;
}

void putPcmFlush()
{
	*PtrPxx++ = 19000 ;		// Past the 18000 of the ARR
}


void putPcmBit( uint8_t bit )
{
    if ( bit )
    {
        PcmOnesCount += 1 ;
        putPcmPart( 1 ) ;
    }
    else
    {
        PcmOnesCount = 0 ;
        putPcmPart( 0 ) ;
    }
    if ( PcmOnesCount >= 5 )
    {
        putPcmBit( 0 ) ;				// Stuff a 0 bit in
    }
}

void putPcmByte( uint8_t byte )
{
    crc( byte ) ;
#ifdef PCBX12D
  if ( byte == 0x7E )
	{
		*PtrSerialPxx++ = 0x7D ;
		byte = 0x5E ;
  }
  else if ( byte == 0x7D )
	{
		*PtrSerialPxx++ = 0x7D ;
		byte = 0x5D ;
	}
	*PtrSerialPxx++ = byte ;
#else
    uint8_t i ;
    for ( i = 0 ; i < 8 ; i += 1 )
    {
        putPcmBit( byte & 0x80 ) ;
        byte <<= 1 ;
    }
#endif
}


void putPcmHead()
{
    // send 7E, do not CRC
#ifdef PCBX12D
	*PtrSerialPxx++ = 0x7E ;
#else
    // 01111110
    putPcmPart( 0 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 1 ) ;
    putPcmPart( 0 ) ;
#endif
}


uint16_t PcmCrc_x ;
uint8_t PcmOnesCount_x ;

void crc_x( uint8_t data )
{
    //	uint8_t i ;

  PcmCrc_x =(PcmCrc_x<<8) ^ CRCTable((PcmCrc_x>>8)^data) ;
}


inline __attribute__ ((always_inline)) void putPcmPart_x( uint8_t value )
{
	uint16_t lpxxValue = PxxValue_x += 18 ;
	
	*PtrPxx_x++ = lpxxValue ;
	lpxxValue += 14 ;
	if ( value )
	{
		lpxxValue += 16 ;
	}
	*PtrPxx_x++ = lpxxValue ;	// Output 0 for this time
	PxxValue_x = lpxxValue ;
}

void putPcmFlush_x()
{
	*PtrPxx_x++ = 19000 ;		// Past the 18000 of the ARR
}


void putPcmBit_x( uint8_t bit )
{
    if ( bit )
    {
        PcmOnesCount_x += 1 ;
        putPcmPart_x( 1 ) ;
    }
    else
    {
        PcmOnesCount_x = 0 ;
        putPcmPart_x( 0 ) ;
    }
    if ( PcmOnesCount_x >= 5 )
    {
        putPcmBit_x( 0 ) ;				// Stuff a 0 bit in
    }
}



void putPcmByte_x( uint8_t byte )
{
    uint8_t i ;

    crc_x( byte ) ;

    for ( i = 0 ; i < 8 ; i += 1 )
    {
        putPcmBit_x( byte & 0x80 ) ;
        byte <<= 1 ;
    }
}


void putPcmHead_x()
{
    // send 7E, do not CRC
    // 01111110
    putPcmPart_x( 0 ) ;
    putPcmPart_x( 1 ) ;
    putPcmPart_x( 1 ) ;
    putPcmPart_x( 1 ) ;
    putPcmPart_x( 1 ) ;
    putPcmPart_x( 1 ) ;
    putPcmPart_x( 1 ) ;
    putPcmPart_x( 0 ) ;
}

uint32_t PxxTestCounter ;


uint16_t scaleForPXX( uint8_t i )
{
	int16_t value ;

//#ifdef REVX
//  value = g_chans512[i] *3 / 4 + 2250 ;
//	return value ;
//#else 
	value = ( i < 24 ) ? g_chans512[i] *3 / 4 + 1024 : 0 ;
	return limit( (int16_t)1, value, (int16_t)2046 ) ;
//#endif
}

void setupPulsesPXX(uint8_t module)
{
  uint8_t i ;
  uint16_t chan ;
  uint16_t chan_1 ;
	uint8_t lpass ;

	if ( module == 0 )
	{
		lpass = Pass[module] ;
#ifndef PCBX12D
		PtrPxx = &pxxStream[module][0] ;
		PxxValue = 0 ;
#else
		PtrSerialPxx = PxxSerial ;
#endif
    
		PcmCrc = 0 ;
#ifndef PCBX12D
  	PcmOnesCount = 0 ;
  	putPcmPart( 0 ) ;
  	putPcmPart( 0 ) ;
  	putPcmPart( 0 ) ;
  	putPcmPart( 0 ) ;
#endif
  	putPcmHead(  ) ;  // sync byte
  	putPcmByte( g_model.Module[module].pxxRxNum ) ;     // putPcmByte( g_model.rxnum ) ;  //
 		uint8_t flag1;
 		if (PxxFlag[module] & PXX_BIND)
		{
 		  flag1 = (g_model.Module[module].sub_protocol<< 6) | (g_model.Module[module].country << 1) | PxxFlag[module] ;
 		}
 		else
		{
 		  flag1 = (g_model.Module[module].sub_protocol << 6) | PxxFlag[module] ;
		}	
		
		if ( ( flag1 & (PXX_BIND | PXX_RANGE_CHECK )) == 0 )
		{
  		if (g_model.Module[module].failsafeMode != FAILSAFE_NOT_SET && g_model.Module[module].failsafeMode != FAILSAFE_RX )
			{
    		if ( FailsafeCounter[module]-- == 0 )
				{
      		FailsafeCounter[module] = 1000 ;
      		flag1 |= PXX_SEND_FAILSAFE ;
				}
    		if ( ( FailsafeCounter[module] == 0 ) && (g_model.Module[module].sub_protocol == 0 ) )
				{
      		flag1 |= PXX_SEND_FAILSAFE ;
				}
			}
		}
		
		putPcmByte( flag1 ) ;     // First byte of flags
  	putPcmByte( 0 ) ;     // Second byte of flags
		
		uint8_t startChan = g_model.Module[module].startChannel ;
		if ( lpass & 1 )
		{
			startChan += 8 ;			
		}
		chan = 0 ;
  	for ( i = 0 ; i < 8 ; i += 1 )		// First 8 channels only
  	{																	// Next 8 channels would have 2048 added
    	if (flag1 & PXX_SEND_FAILSAFE)
			{
				if ( g_model.Module[module].failsafeMode == FAILSAFE_HOLD )
				{
					chan_1 = 2047 ;
				}
				else if ( g_model.Module[module].failsafeMode == FAILSAFE_NO_PULSES )
				{
					chan_1 = 0 ;
				}
				else
				{
					// Send failsafe value
					int32_t value ;
					value = ( startChan < 16 ) ? g_model.Module[module].failsafe[startChan] : 0 ;
					value = ( value *3933 ) >> 9 ;
					value += 1024 ;					
					chan_1 = limit( (int16_t)1, (int16_t)value, (int16_t)2046 ) ;
				}
			}
			else
			{
				chan_1 = scaleForPXX( startChan ) ;
			}
 			if ( lpass & 1 )
			{
				chan_1 += 2048 ;
			}
			startChan += 1 ;
			
//			if ( ( lpass & 1 ) == 0 )
//			{
//				if ( startChan == 1 )
//				{
//					chan_1 = 1024 ;
//					if ( ++PxxTestCounter > 10 )
//					{
//						chan_1 = 1024 * 3 / 4 + 1024 ;
//  					GPIO_ResetBits(GPIOA, PIN_EXTPPM_OUT) ; // Set high
//					}
//					else
//					{
//  					GPIO_SetBits(GPIOA, PIN_EXTPPM_OUT) ; // Set high
//					}
//					if ( PxxTestCounter > 21 )
//					{
//						PxxTestCounter = 0 ;
//					}
//				}
//			}
			
			if ( i & 1 )
			{
	  	  putPcmByte( chan ) ; // Low byte of channel
				putPcmByte( ( ( chan >> 8 ) & 0x0F ) | ( chan_1 << 4) ) ;  // 4 bits each from 2 channels
  		  putPcmByte( chan_1 >> 4 ) ;  // High byte of channel
			}
			else
			{
				chan = chan_1 ;
			}
  	}
		putPcmByte( 0 ) ;		// Antenna flag
  	chan = PcmCrc ;		        // get the crc
  	putPcmByte( chan >> 8 ) ; // Checksum hi
  	putPcmByte( chan ) ; 			// Checksum lo
  	putPcmHead(  ) ;      // sync byte
#ifndef PCBX12D
  	putPcmFlush() ;
#endif
#ifdef PCBX12D
		pulseStreamCount[module] = PtrSerialPxx - PxxSerial ;
#endif
		if (g_model.Module[module].sub_protocol == 1 )		// D8
		{
			lpass = 0 ;
		}
		else
		{
			lpass += 1 ;
			if ( g_model.Module[module].channels == 1 )
			{
				lpass = 0 ;
			}
		}
		Pass[module] = lpass ;
	}  
	else
	{
		lpass = Pass[module] ;
		PtrPxx_x = &pxxStream[module][0] ;
		
		PcmCrc_x = 0 ;
  	PcmOnesCount_x = 0 ;
		PxxValue_x = 0 ;
  	putPcmPart_x( 0 ) ;
  	putPcmPart_x( 0 ) ;
  	putPcmPart_x( 0 ) ;
  	putPcmPart_x( 0 ) ;
  	putPcmHead_x(  ) ;  // sync byte
  	putPcmByte_x( g_model.Module[module].pxxRxNum ) ;     // putPcmByte( g_model.rxnum ) ;  //
 		uint8_t flag1;
 		if (PxxFlag[module] & PXX_BIND)
		{
 		  flag1 = (g_model.Module[module].sub_protocol<< 6) | (g_model.Module[module].country << 1) | PxxFlag[module] ;
 		}
 		else
		{
 		  flag1 = (g_model.Module[module].sub_protocol << 6) | PxxFlag[module] ;
		}	

		if ( ( flag1 & (PXX_BIND | PXX_RANGE_CHECK )) == 0 )
		{
  		if (g_model.Module[module].failsafeMode != FAILSAFE_NOT_SET && g_model.Module[module].failsafeMode != FAILSAFE_RX )
			{
    		if ( FailsafeCounter[module]-- == 0 )
				{
      		FailsafeCounter[module] = 1000 ;
      		flag1 |= PXX_SEND_FAILSAFE ;
				}
    		if ( ( FailsafeCounter[1] == 0 ) && (g_model.Module[module].sub_protocol == 0 ) )
				{
      		flag1 |= PXX_SEND_FAILSAFE ;
				}
			}
		}
		
		putPcmByte_x( flag1 ) ;     // First byte of flags
  	putPcmByte_x( 0 ) ;     // Second byte of flags
		uint8_t startChan = g_model.Module[module].startChannel ;
		if ( lpass & 1 )
		{
			startChan += 8 ;			
		}
		chan = 0 ;
  	for ( i = 0 ; i < 8 ; i += 1 )		// First 8 channels only
  	{																	// Next 8 channels would have 2048 added
    	if (flag1 & PXX_SEND_FAILSAFE)
			{
				if ( g_model.Module[module].failsafeMode == FAILSAFE_HOLD )
				{
					chan_1 = 2047 ;
				}
				else if ( g_model.Module[module].failsafeMode == FAILSAFE_NO_PULSES )
				{
					chan_1 = 0 ;
				}
				else
				{
					// Send failsafe value
					int32_t value ;
					value = ( startChan < 16 ) ? g_model.Module[module].failsafe[startChan] : 0 ;
					value = ( value *3933 ) >> 9 ;
					value += 1024 ;					
					chan_1 = limit( (int16_t)1, (int16_t)value, (int16_t)2046 ) ;
				}
			}
			else
			{
				chan_1 = scaleForPXX( startChan ) ;
			}
			if ( lpass & 1 )
			{
				chan_1 += 2048 ;
			}
			startChan += 1 ;
  	  
			if ( i & 1 )
			{
				putPcmByte_x( chan ) ; // Low byte of channel
				putPcmByte_x( ( ( chan >> 8 ) & 0x0F ) | ( chan_1 << 4) ) ;  // 4 bits each from 2 channels
  		  putPcmByte_x( chan_1 >> 4 ) ;  // High byte of channel
			}
			else
			{
				chan = chan_1 ;
			}
  	}
		putPcmByte_x( 0 ) ;
  	chan = PcmCrc_x ;		        // get the crc
  	putPcmByte_x( chan >> 8 ) ; // Checksum hi
  	putPcmByte_x( chan ) ; 			// Checksum lo
  	putPcmHead_x(  ) ;      // sync byte
  	putPcmFlush_x() ;
#ifdef PCBX12D
		pulseStreamCount[module] = PtrPxx_x - pxxStream[module] ;
#endif
		if (g_model.Module[module].sub_protocol == 1 )		// D8
		{
			lpass = 0 ;
		}
		else
		{
			lpass += 1 ;
			if ( g_model.Module[module].channels == 1 )
			{
				lpass = 0 ;
			}
		}
		Pass[module] = lpass ;
	}
}



#endif

