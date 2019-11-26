
#define SLAVE	1
#include "er9x.h"


// Protocol to 9X
// 0x01, 0x00, 64 bytes data, 0x01	- first line of display
// 0x01, 0x01, 64 bytes data, 0x01	- second line of display
// ...
// 0x01, 0x07, 64 bytes data, 0x01	- last line of display
// 0x01, 0x0F, 64 bytes data, 0x01	- last line of display



// 0x01, 0x10, 2 bytes protocol, 32 bytes data, 0x01  - 16 channel outputs
// 0x01, 0x11, 16 bit address, 8 bit data, 0x01  - Write EEPROM value
// 0x01, 0x12, 16 bit address, 0x01  - send 32 bytes EEPROM data @ address
// 0x01, 0x13, 2 bytes data - contrast and backlight, 0x01
// 0x01, 0x14, 2 bytes data - haptic on/off(1/0), haptic strength, 0x01
// 
// From 9X
// 0x01, 0x80, switches, buttons, trims, sticks, pots, enc_switch, enc_position, revision, 0x01
// 0x01, 0x81, 16 bit address, 32 bytes data, 0x01	- 32 bytes EEPROM data
// 0x01, 0x82, 16 bytes data, 0x01	- 8 trainer inputs

// Also do byte stuffing, 0x01 sent as 0x1B, 0x81 and 0x1B sent as 0x1B, 0x9B.


extern unsigned char DisplayBuf[] ;

#define Revision	8

const static prog_char APM RevisionString[] = { 'r', Revision/100+'0', (Revision%100)/10+'0', '.', Revision%10+'0', 0,  'R', 'e', 'v', Revision }   ;

//uint16_t xt = 0 ;		

//uint16_t EepromAddress ;
//uint8_t EepromRequested ;
uint8_t ControlsRequested ;
uint8_t TxBusy ;
uint8_t SaveMcusr ;
uint8_t SaveEEARH ;

uint8_t Contrast ;
uint8_t Orientation ;
uint8_t HapticStrength ;

uint8_t RestartCount ;

struct t_rotary Rotary ;

//uint8_t Protocol ;
//uint8_t SubProtocol ;
//uint16_t Channels[16] ;

volatile uint8_t sg_tmr10ms ;
volatile uint8_t tick10ms = 0 ;

volatile uint8_t OneSecondCounter ;
volatile uint8_t DoOneSecond ;

uint8_t mcusr ; // save the WDT (etc) flags

void sendBackupData( void ) ;

struct t_fifo64
{
	uint8_t fifo[64] ;
	uint8_t in ;
	uint8_t out ;
	volatile uint8_t count ;
} ;

struct t_fifo64 _9Xtreme_fifo ;

uint8_t TxBuffer[80] ;
uint8_t *TxPtr ;
uint8_t TxCount ;

static int16_t get_fifo64( struct t_fifo64 *pfifo )
{
	uint8_t rxbyte ;
	if ( pfifo->count )						// Look for char available
	{
		rxbyte = pfifo->fifo[pfifo->out] ;
		cli() ;
		pfifo->count -= 1 ;
		sei() ;
		pfifo->out = ( pfifo->out + 1 ) & 0x3F ;
		return rxbyte ;
	}
	return -1 ;
}

static void startTx( uint8_t count )
{
	TxPtr = TxBuffer ;
	TxCount = count ;
	TxBusy = 1 ;
  UCSR0B |= (1 << UDRIE0); // enable  UDRE0 interrupt
}

static void _9XtremeSerialTx()
{
  UCSR0B &= ~(1 << UDRIE0); // disable UDRE0 interrupt
	sei() ;
	if ( TxCount )
	{
		TxCount -= 1 ;
		UDR0 = *TxPtr++ ;
		cli() ;
  	UCSR0B |= (1 << UDRIE0); // enable  UDRE0 interrupt
	}
	else
	{
		TxBusy = 0 ;
	}
}

ISR(USART0_UDRE_vect)
{
	_9XtremeSerialTx() ;
}

uint8_t SlaveState ;
uint8_t SlaveStuff ;
uint8_t SlaveType ;
uint8_t *SlavePtr ;
uint8_t SlaveActionRequired ;
uint8_t SlaveDisplayRefresh ;
uint8_t *SlaveBufferEnd ;

#define SLAVE_RX_SIZE	80
uint8_t SlaveTempReceiveBuffer[SLAVE_RX_SIZE] ;

#define SlaveWaitSTX		0
#define SlaveGotSTX			1
#define SlaveData				2

static void processSlaveByte( uint8_t byte )
{
	switch ( SlaveState )
	{
		case SlaveWaitSTX :
			if ( byte == 1 )
			{
				SlaveState = SlaveGotSTX ;
			}
		break ;
		
		case SlaveGotSTX :
			if ( byte != 1 )
			{
				if ( byte == 0x1B )
				{
					SlaveStuff = 1 ;
				}
				else
				{
					if ( SlaveStuff )
					{
						byte &= 0x7F ;
						SlaveStuff = 0 ;
					}
					SlaveType = byte ;
					if ( (uint16_t)byte < 16 )
					{
						SlavePtr = &DisplayBuf[(uint16_t)byte<<6] ;
						SlaveBufferEnd = SlavePtr + 64 ;
					}
					else
					{
						SlavePtr = SlaveTempReceiveBuffer ;
						SlaveBufferEnd = SlavePtr + SLAVE_RX_SIZE ;
					}
					SlaveState = SlaveData ;
				}
			}
		break ;
		
		case SlaveData :
			if ( byte == 1 )
			{
				SlaveActionRequired = 1 ;
				SlaveState = SlaveWaitSTX ;
//				lcd_outhex4( 100, 0, ( SlaveType << 8 ) | Contrast ) ; 
			}
			else
			{
				if ( byte == 0x1B )
				{
					SlaveStuff = 1 ;
				}
				else
				{
					if ( SlaveStuff )
					{
						byte &= 0x7F ;
						SlaveStuff = 0 ;
					}
					if ( SlavePtr < SlaveBufferEnd )
					{
				  	*SlavePtr++ = byte ;
					}
				}
			}
		break ;
	}
}


// 0x01, 0x80, switches, buttons, trims, sticks, pots, 0x01
uint8_t fillTxBuffer( uint8_t *source, uint8_t type, uint8_t count )
{
	uint8_t *pdest = TxBuffer ;
	*pdest++ = 1 ;
	if ( type == 1 )
	{
		*pdest++ = 0x1B ;
		type |= 0x80 ;
	}
	*pdest++ = type ;
	while ( count )
	{
		type = *source++ ;
		if ( ( type == 1 ) || ( type == 0x1B ) )
		{
			*pdest++ = 0x1B ;
			type |= 0x80 ;
		}
		*pdest++ = type ;
		count -= 1 ;
	}
	*pdest++ = 1 ;
	return pdest - TxBuffer ;
}

#define ADC_VREF_TYPE 0x40

uint16_t s_anaFilt[8] ;

void getADC_osmp()
{
    //  uint16_t temp_ana[8] = {0};
    uint16_t temp_ana ;
    //	uint8_t thro_rev_chan = g_eeGeneral.throttleReversed ? THR_STICK : 10 ;  // 10 means don't reverse
    for (uint8_t adc_input=0;adc_input<8;adc_input++)
		{
//        temp_ana = 0 ;
//        for (uint8_t i=0; i<2;i++) {  // Going from 10bits to 11 bits.  Addition = n.  Loop 2 times
            ADMUX=adc_input|ADC_VREF_TYPE;
            // Start the AD conversion
#if defined(CPUM128) || defined(CPUM2561)
			asm(" rjmp 1f") ;
			asm("1:") ;
			asm(" rjmp 1f") ;
			asm("1:") ;
#endif

      ADCSRA|=0x40;
      // Wait for the AD conversion to complete
      while (ADCSRA & 0x40);
//            ADCSRA|=0x10;
      //      temp_ana[adc_input] += ADCW;
      temp_ana = ADC;
      ADCSRA|=0x40;
      // Wait for the AD conversion to complete
      while (ADCSRA & 0x40);
//        }

#if defined(CPUM128) || defined(CPUM2561)
      temp_ana += ADC;
      ADCSRA|=0x40;
      // Wait for the AD conversion to complete
      while (ADCSRA & 0x40);
      temp_ana += ADC;
      ADCSRA|=0x40;
      // Wait for the AD conversion to complete
      while (ADCSRA & 0x40);
      temp_ana += ADC;
      temp_ana >>= 1 ;
#else

//        temp_ana /= 2; // divide by 2^n to normalize result.
        //    if(adc_input == thro_rev_chan)
        //        temp_ana = 2048 -temp_ana;

        //		s_anaFilt[adc_input] = temp_ana[adc_input] / 2; // divide by 2^n to normalize result.
			temp_ana += ADC ;
#endif
      s_anaFilt[adc_input] = temp_ana ;
        //    if(IS_THROTTLE(adc_input) && g_eeGeneral.throttleReversed)
        //        s_anaFilt[adc_input] = 2048 - s_anaFilt[adc_input];
    }
}

void sendControls()
{
	uint8_t count ;
	uint16_t analog ;
	uint8_t controls[24] ;
	uint16_t *ptrAna = s_anaFilt ;
	FORCE_INDIRECT(ptrAna) ;

	getADC_osmp() ;
	count = ~PINB & 0x7E ;
	if ( keyState( (EnumKeys)(SW_Trainer) ) )
	{
		count |= 1 ;
	}
	controls[0] = count ;	// Buttons + Trainer switch
	controls[1] = ~PIND ;	// Trims
	controls[2] = getCurrentSwitchStates() ;
	
	analog = *ptrAna++ ;
	controls[3] = analog ;
	controls[4] = analog >> 8 ;
	analog = *ptrAna++ ;
	controls[5] = analog ;
	controls[6] = analog >> 8 ;
	analog = *ptrAna++ ;
	controls[7] = analog ;
	controls[8] = analog >> 8 ;
	analog = *ptrAna++ ;
	controls[9] = analog ;
	controls[10] = analog >> 8 ;
	analog = *ptrAna++ ;
	controls[11] = analog ;
	controls[12] = analog >> 8 ;
	analog = *ptrAna++ ;
	controls[13] = analog ;
	controls[14] = analog >> 8 ;
	analog = *ptrAna++ ;
	controls[15] = analog ;
	controls[16] = analog >> 8 ;
	
//	analog = *ptrAna ;
	controls[17] = 0 ;
	controls[18] = 0 ;

	struct t_rotary *protary = &Rotary ;
	FORCE_INDIRECT(protary) ;
	controls[19] = protary->RotEncoder & 0x20 ;	// enc_switch
	controls[20] = protary->RotCount ;					// enc_position
	controls[21] = Revision ;					// Firmware revision

	uint16_t csum = 0 ;
	for ( count = 0 ; count < 22 ; count += 1 )
	{
		csum += controls[count] ;
	}
	controls[17] = csum ;
	controls[18] = (csum >> 8) | 0x80 ;

	count = fillTxBuffer( controls, 0x80, 22 ) ;
	startTx( count ) ;
}

void sendRestarts()
{
	uint8_t count ;
	uint8_t data[20] ;
	
	data[0] = RestartCount ;
	data[1] = PINA ;
	data[2] = PINB ;
	data[3] = PINC ;
	data[4] = PIND ;
	data[5] = PINE ;
	data[6] = PING ;
	data[7] = PORTB ;
	data[8] = DDRB ;
	data[9] = SaveEEARH ;
	data[10] = mcusr ;
	data[11] = Contrast ;
	count = fillTxBuffer( data, 0x83, 12 ) ;
	startTx( count ) ;
}

//void sendEeprom( uint16_t address )
//{
//	uint8_t data[34] ;
//	uint8_t count ;
//	data[0] = address ;
//	data[1] = address >> 8 ;

//  eeprom_read_block( &data[2], (const void*)address, 32 ) ;
//	count = fillTxBuffer( data, 0x81, 34 ) ;
//	startTx( count ) ;
//}

//void per10ms()
//{
//  uint8_t enuk = KEY_MENU;
//  uint8_t    in = ~PINB;
	
//  for(uint8_t i=1; i<7; i++)
//  {
//    //INP_B_KEY_MEN 1  .. INP_B_KEY_LFT 6
//    keys[enuk].input(in & 2,(EnumKeys)enuk);
//    ++enuk;
//		in >>= 1 ;
//  }
//}


// Clocks every 10 mS
#ifdef CPUM2561
ISR(TIMER0_COMPA_vect, ISR_NOBLOCK) //10ms timer
#else
ISR(TIMER0_COMP_vect, ISR_NOBLOCK) //10ms timer
#endif
{ 
#ifdef CPUM2561
  OCR0A += 156 ;			// Interrupt every 128 uS
#else
  OCR0 += 156 ;			// Interrupt every 128 uS
#endif

	uint8_t tmr ;
//  g_tmr10ms++;				// 16 bit sized
//	g8_tmr10ms += 1 ;		// byte sized
//  g_blinkTmr10ms++;
  tmr = sg_tmr10ms + 1 ;
	sg_tmr10ms = tmr ;
	if ( ++OneSecondCounter	> 99 )
	{
		OneSecondCounter = 0 ;
		DoOneSecond = 1 ;
	}
//	per10ms() ;
	
// end 10ms event

}

#ifndef SIMU
ISR(USART0_RX_vect)
{
  uint8_t stat;
  uint8_t data;
  
//  static uint8_t numPktBytes = 0;
//  static uint8_t dataState = frskyDataIdle;
  
	UCSR0B &= ~(1 << RXCIE0); // disable Interrupt
	sei() ;
  
  stat = UCSR0A ; // USART control and Status Register 0 A
  (void) stat ;
	data = UDR0 ; // USART data register 0
	struct t_fifo64 *pfifo = &_9Xtreme_fifo ;
	
	pfifo->fifo[pfifo->in] = data ;
	cli() ;
	pfifo->count += 1 ;
	sei() ;
	pfifo->in = ( pfifo->in + 1) & 0x3F ;
	
	cli() ;
  UCSR0B |= (1 << RXCIE0) ; // enable Interrupt
}
#endif


static uint8_t readAilIp()
{
// #if (!(defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA)))
// 	return PINE & (1<<INP_E_AileDR);
// #else
 	return PINC & (1<<INP_C_AileDR);
// #endif
}


bool keyState(EnumKeys enuk)
{
  uint8_t xxx = 0 ;
	uint8_t ping = PING ;
	uint8_t pine = PINE ;
//  if(enuk < (int)DIM(keys))  return keys[enuk].state() ? 1 : 0;

  switch((uint8_t)enuk){
    case SW_ElevDR : xxx = pine & (1<<INP_E_ElevDR);
    break ;

    case SW_AileDR :
			xxx = readAilIp() ;
    break ;

    case SW_RuddDR : xxx = ping & (1<<INP_G_RuddDR);
    break ;
      //     INP_G_ID1 INP_E_ID2
      // id0    0        1
      // id1    1        1
      // id2    1        0
    case SW_ID0    : xxx = ~ping & (1<<INP_G_ID1);
    break ;
    case SW_ID1    : xxx = (ping & (1<<INP_G_ID1)) ; if ( xxx ) xxx = (PINE & (1<<INP_E_ID2));
    break ;
    case SW_ID2    : xxx = ~pine & (1<<INP_E_ID2);
    break ;
    case SW_Gear   : xxx = pine & (1<<INP_E_Gear);
    break ;
    //case SW_ThrCt  : return PINE & (1<<INP_E_ThrCt);

// #if (!(defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA)))
//     case SW_ThrCt  : xxx = pine & (1<<INP_E_ThrCt);
// #else
    case SW_ThrCt  : xxx = PINC & (1<<INP_C_ThrCt); //shad974: rerouted inputs to free up UART0
// #endif
		break ;

    case SW_Trainer: xxx = pine & (1<<INP_E_Trainer);
    break ;
    default:;
  }
  if ( xxx )
  {
    return 1 ;
  }
  return 0;
}


uint16_t getCurrentSwitchStates()
{
  uint8_t i = 0 ;
  for( uint8_t j=0; j<8; j++ )
  {
    bool t=keyState( (EnumKeys)(SW_BASE_DIAG+7-j) ) ;
		i <<= 1 ;
    i |= t ;
  }
	return i ;
}

static void pollRotary()
{
	// Rotary Encoder polling
	PORTA = 0 ;			// No pullups
	DDRA = 0x1F ;		// Top 3 bits input
	asm(" rjmp 1f") ;
	asm("1:") ;
//	asm(" nop") ;
//	asm(" nop") ;
	uint8_t rotary ;
	rotary = PINA ;
	DDRA = 0xFF ;		// Back to all outputs
	rotary &= 0xE0 ;
//	RotEncoder = rotary ;

	struct t_rotary *protary = &Rotary ;
	FORCE_INDIRECT(protary) ;

	protary->RotEncoder = rotary ; // just read the lcd pin
	
	rotary &= 0xDF ;
	if ( rotary != protary->RotPosition )
	{
		uint8_t x ;
		x = protary->RotPosition & 0x40 ;
		x <<= 1 ;
		x ^= rotary & 0x80 ;
		if ( x )
		{
			protary->RotCount -= 1 ;
		}
		else
		{
			protary->RotCount += 1 ;
		}
		protary->RotPosition = rotary ;
	}
//	if ( protary->TrotCount != protary->LastTrotCount )
//	{
//		protary->RotCount = protary->LastTrotCount = protary->TrotCount ;
//	}
}

//static uint8_t Debug1 ;

static void ioinit0()
{
	DDRA = 0xff;  PORTA = 0x00;
	DDRB = 0x81;  PORTB = 0x7E ; //pullups keys+nc, drive Haptic low to disable TEZ
	DDRC = 0x3e;  PORTC = 0xc1; //pullups nc
	DDRD = 0x00;  PORTD = 0xff; //all D inputs pullups keys
	DDRE = 0x08;  PORTE = 0xff-(1<<OUT_E_BUZZER); //pullups + buzzer 0
	DDRF = 0x00;  PORTF = 0x00; //all F inputs anain - pullups are off
	//DDRG = 0x10;  PORTG = 0xff; //pullups + SIM_CTL=1 = phonejack = ppm_in
	DDRG = 0x14; PORTG = 0xfB; //pullups + SIM_CTL=1 = phonejack = ppm_in, Haptic output and off (0)
	
	PORTG |=  (1<<OUT_G_SIM_CTL); // 1=ppm-in as out
  DDRE |= 0x80 ;					// Bit 7 output
	PORTE |= 0x80 ;

#ifdef CPUM2561
  mcusr = MCUSR; // save the WDT (etc) flags
	SaveMcusr = mcusr ;
  MCUSR = 0; // must be zeroed before disabling the WDT
	MCUCR = 0x80 ;
	MCUCR = 0x80 ;	// Must be done twice
#else
  mcusr = MCUCSR;
  MCUCSR = 0x80;
  MCUCSR = 0x80;	// Must be done twice
#endif
//RebootReason = mcusr ;
#ifdef CPUM2561
	if ( mcusr == 0 )
	{
    wdt_enable(WDTO_60MS) ;
	}
#endif

	ADMUX=ADC_VREF_TYPE;
	ADCSRA=0x85 ;

#ifdef CPUM2561
	TCCR0B  = (5 << CS00);//  Norm mode, clk/1024
	OCR0A   = 156;
	TIMSK0 |= (1<<OCIE0A) ;
#else
	TCCR0  = (7 << CS00);//  Norm mode, clk/1024
	OCR0   = 156;
	TIMSK	 = (1<<OCIE0) ;
#endif
	// TCNT1 2MHz Pulse generator
	TCCR1A = (0<<WGM10);
//#ifdef CPUM2561
////	TIMSK3 |= (1<<ICIE3);
//#else
//	// ETIMSK =0 at power on, just set the bits
////	ETIMSK = (1<<TICIE3);
//#endif
	
}

static void ioinit1()
{
  DDRE &= ~(1 << DDE0) ;    // set RXD0 pin as input
  PORTE &= ~(1 << PORTE0) ; // disable pullup on RXD0 pin

 	UCSR0A &= ~(1 << U2X0) ; // disable double speed operation.
 	UBRR0L = 4 ;		// 200000 baud
 	UBRR0H = 0 ;
  // set 8 N1
  UCSR0B = 0 | (0 << RXCIE0) | (0 << TXCIE0) | (0 << UDRIE0) | (0 << RXEN0) | (0 << TXEN0) | (0 << UCSZ02) ;
  UCSR0C = 0 | (1 << UCSZ01) | (1 << UCSZ00) ;
  
  while (UCSR0A & (1 << RXC0)) UDR0 ; // flush receive buffer

  UCSR0B |= (1 << RXEN0);  // enable RX
  UCSR0B |= (1 << RXCIE0); // enable Interrupt
  UCSR0B |= (1 << TXEN0) ; // enable TX, but not interrupt yet
}

static uint8_t iocheck()
{
	uint8_t result = 0 ;
	if ( UBRR0L != 4 )
	{
		result = 1 ;
	}
	if ( UBRR0H != 0 )
	{
		result = 1 ;
	}
#ifdef CPUM2561
	if ( TCCR0B != (5 << CS00) )
#else
	if ( TCCR0 != (7 << CS00) )
#endif
	{
		result = 1 ;
	}
	return result ;
}


static void slave()
{
	int16_t byte ;
	uint8_t backupTimer = 0 ;
//	uint16_t address ;
	// Initialise serial at 200K baud
	ioinit1() ;
	sei() ;
	 
  lcd_clear() ;
  lcd_puts_Pleft( 24, PSTR("\0079Xtreme") ) ;
  lcd_puts_Pleft( 7*FH, RevisionString ) ;
	refreshDiplay();

//	for ( byte = 0 ; byte < 12500 ; byte += 1 )
//	{
//		_delay_us(16) ;
//    wdt_reset() ;
//	}

	for(;;)
	{
    static uint8_t lastTMR ;
    wdt_reset() ;
		while ( ( byte = get_fifo64( &_9Xtreme_fifo ) ) != -1 )
		{
			processSlaveByte( byte ) ;
			if (SlaveActionRequired)
			{
				SlaveActionRequired = 0 ;
				if ( SlaveType < 16)
				{
					SlaveDisplayRefresh = 1 ;
				}
				else
				{
					switch ( SlaveType )
					{
//						case 0x10 :		// Protocol + channels
//							Protocol = SlaveTempReceiveBuffer[0] ;
//							SubProtocol = SlaveTempReceiveBuffer[1] ;
//							{
//								uint8_t i ;
//								uint8_t index = 0 ;
//								for ( i = 2 ; i < 34 ; i += 2 )
//								{
//									// May need cli/sei round this
//									Channels[index] = SlaveTempReceiveBuffer[i] | ( SlaveTempReceiveBuffer[i+1] <<  8 ) ;
//									index += 1 ;
//								}
//							}
//						break ;
						
						case 0x13 :
							Contrast = SlaveTempReceiveBuffer[0] ;
//							lcdSetRefVolt( 27 ) ;
							lcdSetRefVolt( Contrast ) ;
							if ( Orientation != SlaveTempReceiveBuffer[1] )
							{
								Orientation = SlaveTempReceiveBuffer[1] ;
								lcdSetOrientation() ;
							}
						break ;

						case 0x14 :	 
							HapticStrength = SlaveTempReceiveBuffer[1] ;
							if ( SlaveTempReceiveBuffer[0] & 1 )
							{
								PORTB |= 1 ;
							}
							else
							{
								PORTB &= ~1 ;
							}
						break ;
//						case 0x12 :
//// 0x01, 0x12, 16 bit address, 0x01  - send 32 bytes EEPROM data @ address
//							address = SlaveTempReceiveBuffer[0] | ( SlaveTempReceiveBuffer[1] <<  8 ) ;
//							EepromAddress = address ;
//							EepromRequested = 1 ;
//						break ;
					}
				}
			}
		}
		pollRotary() ;

		uint8_t t10ms ;
		t10ms = sg_tmr10ms ;
    tick10ms = t10ms - lastTMR ;
    lastTMR = t10ms ;
		
		if(tick10ms) //make sure the rest happen only every 10ms.
		{
			ControlsRequested = 1 ;
			
			if ( SlaveDisplayRefresh )
			{
				SlaveDisplayRefresh = 0 ;
				refreshDiplay() ;
			}
			if ( iocheck() )
			{
				ioinit0() ;
				ioinit1() ;
				RestartCount += 1 ;
			}
		}
		if ( TxBusy == 0 )
		{
			if ( DoOneSecond )
			{
				DoOneSecond = 0 ;
				sendRestarts() ;
			}
			else

//			if ( EepromRequested )
//			{
//				sendEeprom( EepromAddress ) ;
//				EepromRequested = 0 ;
//			}
//			else 
			if ( ControlsRequested )
			{
				sendControls() ;
				ControlsRequested = 0 ;
				if ( ++backupTimer > 2 )
				{
					sendBackupData() ;
					backupTimer = 0 ;
				}
			}

//			if ( ++Debug1 > 19 )
//			{
//				Debug1 = 0 ;
////				lcd_outhex4( 100, 0, ( SlaveType << 8 ) | Contrast ) ; 
//				refreshDiplay() ;
//			}
		}
	}
}

int main(void)
{
	EEARH = 0xFF ;
	SaveEEARH = EEARH ;

	ioinit0() ;

	lcd_init() ;

  wdt_enable(WDTO_500MS);
	slave() ;
}

// Backup data transmission to avoid using pogo pins


#define PULSES_WORD_SIZE	200		// 72=((2+2*6)*10)/2+2
#define PULSES_BYTE_SIZE	(PULSES_WORD_SIZE * 2)

union p2mhz_t 
{
    uint16_t pword[PULSES_WORD_SIZE] ;  // 72
    uint8_t pbyte[PULSES_BYTE_SIZE] ;		// 144
} pulses2MHz ;

uint16_t *pulses2MHzptr = pulses2MHz.pword ;

static uint16_t *Serial_pulsePtr = pulses2MHz.pword ;

#define BITLEN_SERIAL (104*2) // 9600 Baud
static void sendByteSerial(uint8_t b) //max 10changes 0 10 10 10 10 1
{
    bool    lev = 0;
    uint16_t len = BITLEN_SERIAL; //max val: 9*16 < 256
		uint16_t *ptr ;

		ptr = pulses2MHzptr ;
    for( uint8_t i=0; i<=8; i++){ //8Bits + Stop=1
        bool nlev = b & 1; //lsb first
        if(lev == nlev){
            len += BITLEN_SERIAL;
        }else{
						*ptr++ = len -1 ;
            len  = BITLEN_SERIAL;
            lev  = nlev;
        }
        b = (b>>1) | 0x80; //shift in stop bit
    }
    *ptr++ = len+BITLEN_SERIAL-1 ; // 2 stop bits
	  pulses2MHzptr = ptr ;
}

static void setupPulsesSerial( uint8_t *data, uint8_t count )
{
	uint8_t csum = 0 ;
  pulses2MHzptr = pulses2MHz.pword ;

	sendByteSerial( 0x55 ) ;
  while ( count )
	{
		uint8_t byte = *data ;
		csum += byte ;
		if ( ( byte == 0x55 ) || ( byte == 0x56 ) )
		{
			sendByteSerial( 0x56 ) ;
			sendByteSerial( byte & 0x0F ) ;
		}
		else
		{
			sendByteSerial( byte ) ;
		}
		data += 1 ;
		count -= 1 ;
	}
	sendByteSerial( csum ) ;
	sendByteSerial( 0 ) ;
	uint16_t *ptr ;
	ptr = pulses2MHzptr ;
	*ptr = 0 ;
  Serial_pulsePtr = pulses2MHz.pword ;
}

uint8_t BackupData[20] ;
static uint8_t PulsePol ;

void sendBackupData()
{
	uint16_t *ptrAna = &s_anaFilt[4] ;
	uint8_t *ptrData = BackupData ;
	FORCE_INDIRECT(ptrData) ;
	uint8_t value ;

	TCNT1 = 0 ;
	PORTE |= 0x80 ;
	PulsePol = 0 ;
  
	value = ~PINB & 0x7E ;
	if ( keyState( (EnumKeys)(SW_Trainer) ) )
	{
		value |= 1 ;
	}
	ptrData[0] = value ;
	ptrData[1] = ~PIND ;	// Trims
	ptrData[2] = getCurrentSwitchStates() ;

	uint16_t analog = *ptrAna++ ;
	ptrData[3] = analog ;
	ptrData[4] = analog >> 8 ;
	analog = *ptrAna++ ;
	ptrData[5] = analog ;
	ptrData[6] = analog >> 8 ;
	analog = *ptrAna ;
	ptrData[7] = analog ;
	ptrData[8] = analog >> 8 ;
	struct t_rotary *protary = &Rotary ;
	FORCE_INDIRECT(protary) ;
	ptrData[9] = protary->RotEncoder & 0x20 ;	// enc_switch
	ptrData[10] = protary->RotCount ;					// enc_position
	ptrData[11] = Revision ;					// Firmware revision

	setupPulsesSerial( ptrData, 12 ) ;
	
	OCR1A = 1 ;
	TCCR1B = (1 << WGM12) | (2<<CS10); // CTC OCR1A, 16MHz / 8
#ifdef CPUM2561
  TIMSK1 |= (1<<OCIE1A);
#else
  TIMSK |= (1<<OCIE1A);
#endif
}


ISR(TIMER1_COMPA_vect) //2MHz pulse generation
{
	static uint16_t *pulsePtr = pulses2MHz.pword ;
  uint16_t *xpulsePtr ;

  if(PulsePol)
  {
      PORTE |= 0x80 ; // (1<<OUT_B_PPM);
      PulsePol = 0;
  }else{
      PORTE &= ~0x80 ;		// (1<<OUT_B_PPM);
      PulsePol = 1;
  }

	xpulsePtr = pulsePtr ;	// read memory once

  OCR1A  = *xpulsePtr++;
  
	if( *xpulsePtr == 0)
  {
    PORTE |= 0x80 ; // (1<<OUT_B_PPM);
    xpulsePtr = pulses2MHz.pword ;
#ifdef CPUM2561
    TIMSK1 &= ~(1<<OCIE1A) ;
#else
    TIMSK &= ~(1<<OCIE1A) ;
#endif
  }
	pulsePtr = xpulsePtr ;	// write memory back
}


