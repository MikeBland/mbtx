
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
// 
// From 9X
// 0x01, 0x80, switches, buttons, trims, sticks, pots, 0x01
// 0x01, 0x81, 16 bit address, 32 bytes data, 0x01	- 32 bytes EEPROM data
// 0x01, 0x82, 16 bytes data, 0x01	- 8 trainer inputs

// Also do byte stuffing, 0x01 sent as 0x1B, 0x81 and 0x1B sent as 0x1B, 0x9B.


extern uint8_t Arduino ;
extern unsigned char DisplayBuf[] ;

uint16_t EepromAddress ;
uint8_t EepromRequested ;
uint8_t ControlsRequested ;
uint8_t TxBusy ;

uint8_t Protocol ;
uint8_t SubProtocol ;
uint16_t Channels[16] ;

struct t_fifo64
{
	uint8_t fifo[64] ;
	uint8_t in ;
	uint8_t out ;
	volatile uint8_t count ;
} ;

struct t_fifo64 Arduino_fifo ;

uint8_t TxBuffer[80] ;
uint8_t *TxPtr ;
uint8_t TxCount ;

int16_t get_fifo64( struct t_fifo64 *pfifo )
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

void arduinoSerialRx()
{
  uint8_t stat ;
  uint8_t data ;

  stat = UCSR0A ; // USART control and Status Register 0 A
  (void) stat ;
	data = UDR0 ; // USART data register 0
	struct t_fifo64 *pfifo = &Arduino_fifo ;
	
	pfifo->fifo[pfifo->in] = data ;
	cli() ;
	pfifo->count += 1 ;
	sei() ;
	pfifo->in = ( pfifo->in + 1) & 0x3F ;
	
	cli() ;
  UCSR0B |= (1 << RXCIE0) ; // enable Interrupt
}

void startTx( uint8_t count )
{
	TxPtr = TxBuffer ;
	TxCount = count ;
	TxBusy = 1 ;
  UCSR0B |= (1 << UDRIE0); // enable  UDRE0 interrupt
}

void arduinoSerialTx()
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

uint8_t SlaveState ;
uint8_t SlaveStuff ;
uint8_t SlaveType ;
uint8_t *SlavePtr ;
uint8_t SlaveActionRequired ;
uint8_t SlaveDisplayRefresh ;

uint8_t SlaveTempReceiveBuffer[80] ;

#define SlaveWaitSTX		0
#define SlaveGotSTX			1
#define SlaveData				2

void processSlaveByte( uint8_t byte )
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
					if ( byte < 16 )
					{
						SlavePtr = &DisplayBuf[(uint16_t)byte<<6] ;
					}
					else
					{
						SlavePtr = SlaveTempReceiveBuffer ;
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
				  *SlavePtr++ = byte ;
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

extern uint16_t s_anaFilt[] ;

void sendControls()
{
	uint8_t count ;
	uint16_t analog ;
	uint8_t controls[20] ;


	getADC_osmp() ;
	count = ~PINB & 0x7E ;
	if ( keyState( (EnumKeys)(SW_Trainer) ) )
	{
		count |= 1 ;
	}
	controls[0] = count ;	// Buttons + Trainer switch
	controls[1] = ~PIND ;	// Trims
	controls[2] = getCurrentSwitchStates() ;
	
	analog = s_anaFilt[0] ;
	controls[3] = analog ;
	controls[4] = analog >> 8 ;
	analog = s_anaFilt[1] ;
	controls[5] = analog ;
	controls[6] = analog >> 8 ;
	analog = s_anaFilt[2] ;
	controls[7] = analog ;
	controls[8] = analog >> 8 ;
	analog = s_anaFilt[3] ;
	controls[9] = analog ;
	controls[10] = analog >> 8 ;
	analog = s_anaFilt[4] ;
	controls[11] = analog ;
	controls[12] = analog >> 8 ;
	analog = s_anaFilt[5] ;
	controls[13] = analog ;
	controls[14] = analog >> 8 ;
	analog = s_anaFilt[6] ;
	controls[15] = analog ;
	controls[16] = analog >> 8 ;
	analog = s_anaFilt[7] ;
	controls[17] = analog ;
	controls[18] = analog >> 8 ;

	count = fillTxBuffer( controls, 0x80, 19 ) ;
	startTx( count ) ;
}

void sendEeprom( uint16_t address )
{
	uint8_t data[34] ;
	uint8_t count ;
	data[0] = address ;
	data[1] = address >> 8 ;

  eeprom_read_block( &data[2], (const void*)address, 32 ) ;
	count = fillTxBuffer( data, 0x81, 34 ) ;
	startTx( count ) ;
}

void arduinoDueSlave()
{
	int16_t byte ;
	uint16_t address ;
	// Initialise serial at 100K baud
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
	 
	Arduino = 1 ;
	
  lcd_clear() ;
  lcd_puts_Pleft( 24, PSTR("Arduino Slave") ) ;
	refreshDiplay();

	for(;;)
	{
    static uint8_t lastTMR ;
    wdt_reset() ;
		while ( ( byte = get_fifo64( &Arduino_fifo ) ) != -1 )
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
						case 0x10 :		// Protocol + channels
							Protocol = SlaveTempReceiveBuffer[0] ;
							SubProtocol = SlaveTempReceiveBuffer[1] ;
							{
								uint8_t i ;
								uint8_t index = 0 ;
								for ( i = 2 ; i < 34 ; i += 2 )
								{
									// May need cli/sei round this
									Channels[index] = SlaveTempReceiveBuffer[i] | ( SlaveTempReceiveBuffer[i+1] <<  8 ) ;
									index += 1 ;
								}
							}
						break ;
							
						case 0x12 :
// 0x01, 0x12, 16 bit address, 0x01  - send 32 bytes EEPROM data @ address
							address = SlaveTempReceiveBuffer[0] | ( SlaveTempReceiveBuffer[1] <<  8 ) ;
							EepromAddress = address ;
							EepromRequested = 1 ;
						break ;
					}
				}
			}
		}
		uint8_t t10ms ;
		t10ms = g_tmr10ms ;
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
		}
		if ( TxBusy == 0 )
		{
			if ( EepromRequested )
			{
				sendEeprom( EepromAddress ) ;
				EepromRequested = 0 ;
			}
			else if ( ControlsRequested )
			{
				sendControls() ;
				ControlsRequested = 0 ;
			}
		}
	}
}


