#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "pff.h"
#include "fdiskio.h"

#ifdef SERIAL_VOICE
#define SERIAL_RECEIVE	1
#define TRIM_ON_DATA		1
#endif


//#define FCC(c1,c2,c3,c4)	(((DWORD)c4<<24)+((DWORD)c3<<16)+((WORD)c2<<8)+(BYTE)c1)	/* FourCC */

//#define sbi(reg, bit) reg |= (1<<bit)
#define cbi(reg, bit) reg &= ~(1<<bit)
#define ckbi(reg, bit) reg & (1<<bit)

//#define LED_ON()	PORTC |= _BV(1) //define pin for LED
//#define LED_OFF()	PORTC &= ~_BV(1)

#ifdef SERIAL_VOICE
#define BUSY_ON()	Busy = 0 
#define BUSY_OFF()	Busy = 0x80 
#else
#define BUSY_ON()	PORTC |= _BV(0) //define pin for BUSY
#define BUSY_OFF()	PORTC &= ~_BV(0)
#endif

#ifndef F_CPU
#define F_CPU 12000000UL
#endif

#define BAUDRATE 38400
#define UBRR (F_CPU/(8*BAUDRATE)-1)

#define TESTING	0
#define YMODEM	1
//#define SERIAL_VOICE	1

void delay_ms (WORD);	/* Defined in asmfunc.S */
void delay_us (WORD);	/* Defined in asmfunc.S */
void put_audio_fifo( WORD ) ;	/* Defined in asmfunc.S */
uint8_t  pollBoot( void ) ;
void tx_single_byte( uint8_t byte ) ;
void ymount( void ) ;
uint8_t ymodem_Receive ( void ) ;
void checkSerial( void ) ;

volatile uint8_t Serial_busy ;
uint8_t XferMode = 0 ;
uint8_t BootReason = 0 ;
#ifdef SERIAL_RECEIVE
extern uint8_t rx_fifo_running ;
extern uint8_t LastSerialData ;
uint8_t SerialStartupCount ;
#endif

#ifdef SERIAL_VOICE
volatile uint8_t TenMs ;

uint8_t TxData[4] ;
uint8_t TxCount ;
uint8_t TxIndex ;
#endif

uint8_t Busy ;

/*---------------------------------------------------------*/
/* Work Area                                               */
/*---------------------------------------------------------*/

volatile BYTE FifoRi, FifoWi, FifoCt;	/* FIFO controls */
volatile WORD Command;	/* Control commands */

union t_buff
{
	BYTE bytes[256];		/* Audio output FIFO */
	DWORD dwords[64] ;
	uint8_t filename[116] ;
} ;

// Assembly languags accesses Abuff directly by the name Abuff
// This union provides a 512 byte buffer
struct t_audio_buffers
{
	union t_buff Buff ;
	BYTE DirBuffer[256] ;
} ;

struct t_audio_buffers Abuff ;

BYTE InMode, Cmd;	/* Input mode and received command value */

FATFS Fs;			/* File system object */
extern DIR Dir;			/* Directory object */
FILINFO Fno;		/* File information */

WORD rb;			/* Return value. Put this here to avoid avr-gcc's bug */

uint8_t CommandTimeout ;
uint8_t Volume ;

uint8_t LastSerialRx ;



//static BYTE play ( WORD fn ) ;
static void audio_off (void) ;
uint8_t playWav( uint16_t fileNum ) ;


/*---------------------------------------------------------*/
/* Sub-routines                                            */
/*---------------------------------------------------------*/


// C versions of assembler code for SPI.
// These generate similar code, but may 'inline' in C functions
//inline uint8_t xrcv_spi()
//{
//	SPDR = 0xFF ;
//	while(( SPSR & 0x80 ) == 0 )
//		;
//	return SPDR ;
//}

//uint8_t xxmit_spi( uint8_t byte )
//{
//	SPDR = byte ;
//	while(( SPSR & 0x80 ) == 0 )
//		;
//	return SPDR ;
//}

//int16_t mult( int16_t mx )
//{
//	return mx * Volume ;
//}

//void c_fwd_blk_part( uint8_t *dest, uint16_t offset, uint16_t count)
//{
//	uint16_t x ;		//  Number of bytes to receive
	
//	x = 514 - offset - count ;
//	while ( offset-- )
//	{
//		xrcv_spi() ;		
//	}
//	if ( dest )
//	{ // memory
//		do
//		{
//			*dest++ = xrcv_spi() ;
//		} while( count-- ) ;
//	}
//	else
//	{ // wave
//		if ( GPIOR0 & 0x80 )
//		{
//			count >>= 1 ;	//if (16bit data) count /= 2;
//		}
//		if ( GPIOR0 & 0x40 )
//		{
//			count >>= 1 ;	//if (stereo data) count /= 2;
//		}

//		do
//		{
//			uint8_t x ;
//			uint16_t y ;
			
//			dest = &Buff[FifoWi] ;
//			while ( FifoCt == 252 )
//				;
		
//			x = xrcv_spi() ;		 
//			y &= 0xFF00 ;
//			if ( GPIOR0 & 0x80 )
//			{
//				y |= x ;
//				x = xrcv_spi() ;
//				x -= 0x80 		 
//			}
//			y |= x << 8 ;
//			if ( GPIOR0 & 0x40 )
//			{
				
//			}
//			*dest++ = y ;
//			*dest++ = y >> 8 ;
//			cli() ;
//			FifoCt += 2 ;
//			sei() ;
//			FifoWi += 2 ;
//		} while( --count ) ;

//	}
	
//	while ( x-- )
//	{
//		xrcv_spi() ;		
//	}
//}


void setBeeperVolume( uint8_t volume )
{
	volume >>= 1 ;		// gives 3, 2, 1 and 0
	PORTC &= 0xCF ;		// bits 5 and 4 = 0
	if ( volume == 3 )
	{
		DDRC &= 0xCF ;		// Both inputs
		DIDR0 = 0x30 ;		// Analog
	}
	else if ( volume == 2 )
	{
		DDRC &= 0xDF ;		// One input
		DDRC |= 0x20 ;		// One output
		DIDR0 = 0x20 ;		// Analog
	}
	else if ( volume == 1 )
	{
		DDRC &= 0xEF ;		// One input
		DDRC |= 0x10 ;		// One output
		DIDR0 = 0x10 ;		// Analog
	}
	else	// volume == 0
	{
		DDRC |= 0x30 ;		// Both output
		DIDR0 = 0 ;				// Both digital
	}
}

static
BYTE chk_input (void)	/* 0:Not changed, 1:Changed */
{
	static uint8_t GpioCopy ;
	uint8_t t ;
	uint8_t x ;

#ifdef SERIAL_VOICE
	checkSerial() ;
#endif

	if (ckbi(GPIOR0,5))
	{
		if ((Command<=0xFFF7)&(Command>=0xFFF0))
		{
			//VOLUME
			cbi(GPIOR0,5);
			Volume = ( ( Command & 7 ) + 1 ) << 5 ;
			setBeeperVolume( Command & 7 ) ;
			return 0;
		}
		cbi(GPIOR0,5);
		return 1;
	}
	// No command received, check for timeout, or bootloader request
	if ( pollBoot() )
	{
		// Request for PC communication
		return 2 ;
	}
	t = GPIOR0 & 0x1F ;		// a_clock and count bits
	x = GpioCopy ;
	if ( t == x )
	{ // no input
#ifdef SERIAL_VOICE
		if ( TenMs )
		{ // 10mS passed
			TenMs = 0 ;
			if ( ++CommandTimeout > 19)
#else
    if ( TIFR2 & (1 << TOV2) )
    { // 21mS passed
			TIFR2 = (1 << TOV2) ;                   // Clear timer 2 overflow flag
			if ( ++CommandTimeout > 9)
#endif				 
			{ // Around 200 mS with no input
				// Abandon any received command (safely)
#ifdef SERIAL_VOICE
				if ( SerialStartupCount < 32 )	// For 6.4 seconsd
				{
					if ( ( SerialStartupCount & 7) == 0 )
					{
						LastSerialData = 0xFF ;	// Force send status
					}
					SerialStartupCount += 1 ;
				}
#endif				 
				cli() ;
				t = GPIOR0 & 0x1F ;		// a_clock and count bits
				if ( t == x )
				{
					GPIOR0 &= 0b11010000 ;
					GpioCopy = GPIOR0 & 0x1F ;
				}
				sei() ;
				CommandTimeout = 0 ;

#if TESTING
			// Debug test of serial Tx
				tx_single_byte( 'X' ) ;
				PORTB ^= 0x02 ;		// Toggle backlight
				static uint8_t timer = 0 ;

				if ( ++timer > 50 )
				{
					timer = 0 ;
					BUSY_ON() ;
					playWav( 40 ) ;	/* Play corresponding audio file */
					BUSY_OFF() ;
					audio_off();	/* Disable audio output */
				}
#endif				 
			}
		}
	}
	else
	{
		GpioCopy = t ;
		CommandTimeout = 0 ;
	}
	return 0;
}


static
void ramp (		/* Ramp-up/down audio output (anti-pop feature) */
	int dir		/* 0:Ramp-down, 1:Ramp-up */
)
{
	BYTE v, d, n;


	if (dir) {
		v = 0; d = 1;
	} else {
		v = 128; d = 0xFF;
	}

	n = 128;
	do {
		v += d;
//		OCR1A = v; OCR1B = v;
		OCR0A = v; OCR0B = v;
		delay_us(100);
	} while (--n);
}


void audio_on (void)	/* Enable audio output functions */
{
	if ( TCCR0A == 0 )
	{
		
		TCCR0A = 0b10100011;	/* Start TC0 with OC0A/OC0B PWM enabled */
		TCCR0B = 0b00000001;
		ramp(1);				/* Ramp-up to center level */
	}

	if (!TCCR1B) {
		FifoCt = 0; FifoRi = 0; FifoWi = 0;		/* Reset audio FIFO */
		
		TCCR1A = 0b00000000;	/* Enable TC`.ck = 2MHz as interval timer */
		TCCR1B = 0b00001010;
		
		TIMSK1 = _BV(OCIE1A);

	}
//	sei();
}


static
void audio_off (void)	/* Disable audio output functions */
{
	OCR0A = 128 ;
	OCR0B = 128 ;
	
	if (TCCR1B) {
		TCCR1B = 0;				/* Stop audio timer */
//		ramp(0);				/* Ramp-down to GND level */
//		TCCR0A = 0;	TCCR0B = 0;	/* Stop PWM */
	}
//	cli();
}


#if 0 //debug
static
void wait_status (void)	/* Wait for a code change */
{

//	BYTE n;


	if (Cmd) return;

	audio_off();	/* Disable audio output */

//	for (;;) {
//		n = 10;				/* Wait for a code change at active mode (100ms max) */
//		do {
//			delay_ms(10);
//			chk_input();
//		} while (--n && !Cmd);
//		if (Cmd) break;		/* Return if any code change is detected within 100ms */
//
//		cli();							/* Enable pin change interrupt */
//		GIMSK = _BV(PCIE1);
//		WDTCR = _BV(WDE) | _BV(WDCE);	/* Disable WDT */
//		WDTCR = 0;
//		sleep_enable();					/* Wait for a code change at power-down mode */
//		sei();
//		sleep_cpu();
//		sleep_disable();
//		wdt_reset();					/* Enable WDT (1s) */
//		WDTCR = _BV(WDE) | 0b110;
//		GIMSK = 0;						/* Disable pin change interrupt */
//	} 
}
#endif


//static
//DWORD load_header (void)	/* 2:I/O error, 4:Invalid file, >=1024:Ok(number of samples) */
//{
//	DWORD sz, f;
//	BYTE b, al = 0;

//	//GPIOR0:	7bit - HIGH:16bit samples, LOW:8bit samples
//	//			6bit - HIGH:stereo, LOW:mono
//	//			5bit - HIGH:new PLAY command recieve
//	//			4bit - last a_clock state
//	//			3:0bits - used for command bit counter


//	/* Check RIFF-WAVE file header */
//	if (pf_read(Abuff.Buff.bytes, 12, &rb)) return 2;
//	if (rb != 12 || LD_DWORD(Abuff.Buff.dwords+2) != FCC('W','A','V','E')) return 4;

//	for (;;) {
//		if (pf_read(Abuff.Buff.bytes, 8, &rb)) return 2;		/* Get Chunk ID and size */
//		if (rb != 8) return 4;
//		sz = LD_DWORD(Abuff.Buff.dwords+1);		/* Chunk size */

//		switch (LD_DWORD(Abuff.Buff.dwords)) {	/* Switch by chunk type */
//		case FCC('f','m','t',' ') :		/* 'fmt ' chunk */
//			if (sz & 1) sz++;
//			if (sz > 128 || sz < 16) return 4;		/* Check chunk size */
//			if (pf_read(Abuff.Buff.bytes, sz, &rb)) return 2;	/* Get the chunk content */
//			if (rb != sz) return 4;
//			if (Abuff.Buff.bytes[0] != 1) return 4;				/* Check coding type (1: LPCM) */

//			b = Abuff.Buff.bytes[2];

//			if (b < 1 && b > 2) return 4; 			/* Check channels (1/2: Mono/Stereo) */
			
//			if (b==2) GPIOR0 |= _BV(6); else GPIOR0 &= ~_BV(6);
//			//GPIOR0 = al = b;						/* Save channel flag */
//			al = b;						/* Save channel flag */

//			b = Abuff.Buff.bytes[14];

//			if (b != 8 && b != 16) return 4;		/* Check resolution (8/16 bit) */

//			if (b==16) GPIOR0 |= _BV(7); else GPIOR0 &= ~_BV(7);  
//			//GPIOR0 |= b;							/* Save resolution flag */

//			if (b & 16) al <<= 1;
//			f = LD_DWORD(Abuff.Buff.dwords+1);					/* Check sampling freqency (8k-48k) */
//			if (f < 8000 || f > 48000) return 4;


//			// **** We must set interval timer corresponding F_CPU
//			OCR1A = (BYTE)(F_CPU/8/f) - 1;		/* Set interval timer (sampling period) */
//			// **** END We must set interval timer corresponding F_CPU

//			break;

//		case FCC('d','a','t','a') :		/* 'data' chunk (start to play) */
//			if (!al) return 4;							/* Check if format valid */
//			if (sz < 1024 || (sz & (al - 1))) return 4;	/* Check size */
//			if (Fs.fptr & (al - 1)) return 4;			/* Check offset */
//			return sz;

//		case FCC('D','I','S','P') :		/* 'DISP' chunk (skip) */
//		case FCC('f','a','c','t') :		/* 'fact' chunk (skip) */
//		case FCC('L','I','S','T') :		/* 'LIST' chunk (skip) */
//			if (sz & 1) sz++;
//			if (pf_lseek(Fs.fptr + sz)) return 2;
//			break;

//		default :						/* Unknown chunk */
//			return 4;
//		}
//	}
//}


//static
//BYTE play (		/* 0:Normal end, 1:Continue to play, 2:Disk error, 3:No file, 4:Invalid file */
//	WORD fn		/* File number (0-511) */
//)
//{
//	DWORD sz, spa, sza;
//	FRESULT res;
//	WORD btr, n ;
//	BYTE i, rc;


////	sei();
//	if (InMode >= 2) Cmd = 0;	/* Clear command code (Edge triggered) */

//	/* Open an audio file "nnn.WAV" (nnn=001..255) */
////	i = 2; n = fn; //for 3-sign name
//	i = 3; n = fn; //for 4-sign name
//	do {
//		Abuff.Buff.bytes[i] = (BYTE)(n % 10) + '0'; n /= 10;
//	} while (i--);
////	strcpy_P((char*)&Abuff.Buff.bytes[3], PSTR(".WAV")); //for 3-sign name
//	strcpy_P((char*)&Abuff.Buff.bytes[4], PSTR(".WAV")); //for 4-sign name
//	res = pf_open((char*)Abuff.Buff.bytes);
//	if (res == FR_NO_FILE) return 3;
//	if (res != FR_OK) return 2;
//	/* Get file parameters */
//	sz = load_header();
//	if (sz <= 4) return (BYTE)sz;	/* Invalid format */
//	spa = Fs.fptr; sza = sz;		/* Save offset and size of audio data */

//	BUSY_ON();
//	audio_on();		/* Enable audio output */

//	for (;;) {
////		if (pf_read(0, 512 - (Fs.fptr % 512), &rb) != FR_OK) {		/* Snip sector unaligned part */
//		if (pf_read(0, 512 - (spa % 512), &rb) != FR_OK) {		/* Snip sector unaligned part */
//			rc = 2; break;
//		}
//		sz -= rb;
//		do {
//			/* Forward a bunch of the audio data to the FIFO */
//			btr = (sz > 1024) ? 1024 : (WORD)sz;
//			pf_read(0, btr, &rb);
//			if (btr != rb) {
//				rc = 2; break;
//			}
//			sz -= rb;

//			/* Check input code change */
////			rc = 0;
////			if (chk_input()) {
////				switch (InMode) {
////				case 3: 	/* Mode 3: Edge triggered (retriggerable) */
////					if (Cmd) rc = 1;	/* Restart by a code change but zero */
////					break;
////				case 2:		/* Mode 2: Edge triggered */
////					Cmd = 0;			/* Ignore code changes while playing */
////					break;
////				case 1:		/* Mode 1: Level triggered (sustained to end of the file) */
////					if (Cmd && Cmd != fn) rc = 1;	/* Restart by a code change but zero */
////					break;
////				default:	/* Mode 0: Level triggered */
////					if (Cmd != fn) rc = 1;	/* Restart by a code change */
////				}
////			}
//			rc = 0;
//			if (chk_input() && (Command==0xFFFF)) 
//				{
//					rc = 1;	/* Restart by a code change but zero */
//					break;
//				}
//			//$%#%#	rc = 1;
////			}
//		} while (!rc && rb == 1024);	/* Repeat until all data read or code change */

////		if (rc || !Cmd || InMode >= 2) break;
////		if (rc) break;
//		break;

//		if (pf_lseek(spa) != FR_OK) {	/* Return top of audio data */
//			rc = 3; break;
//		}
//		sz = sza;
//	}

//	while (FifoCt) ;			/* Wait for audio FIFO empty */
//	OCR0A = 0x80; OCR0B = 0x80;	/* Return DAC out to center */


//	BUSY_OFF();
////	cli();
//	return rc;

//}

#ifdef __AVR_ATmega88__
#define BOOTADDR	0x1E00
#endif

#ifdef __AVR_ATmega88P__
#define BOOTADDR	0x1E00
#endif

#ifdef __AVR_ATmega168__
#define BOOTADDR	0x3E00
#endif

#ifdef __AVR_ATmega328__
#define BOOTADDR	0x7E00
#endif

#ifdef __AVR_ATmega328P__
#define BOOTADDR	0x7E00
#endif



void goToBootloader()
{
	TCCR2B = 0 ;
	TIMSK2 &= ~_BV(OCIE2A);
				
	UCSR0B |= ( 1 << TXEN0) ;	// Enable TX
	register void (*p)() ;
	p = (void *)BOOTADDR ;
	cli() ;
  MCUSR = 0 ;
  SP=RAMEND;  // This is done by hardware reset
	(*p)() ;
}

/*-----------------------------------------------------------------------*/
/* Main                                                                  */

int main (void)
{
	BYTE rc ;//, i;

	MCUSR = 0;								// Clear reset status
//	WDTCR = _BV(WDE) | 0b110;				// Enable WDT (1s)
//	set_sleep_mode(SLEEP_MODE_PWR_DOWN);	// Select power down mode for sleep
//	PCMSK0 = 0b11111000;					// Select pin change interrupt pins (SW1..SW8)
//	PCMSK1 = 0b01110000;
//	PCMSK2 = 0b00010000;
//	GIMSK = _BV(PCIE2);

	for(;;)
	{
		/* Initialize ports */
		DDRD  = 0b01100000;
#ifdef TRIM_ON_DATA
		PORTD = 0b10011000;
#else
		PORTD = 0b00000000;
#endif
		DDRB  = 0b00101110;
//		PORTB = 0b00011100;		
#ifdef TRIM_ON_DATA
		PORTB = 0b00011111 ;	// Backlight ON
#else
		PORTB = 0b00011110 ;	// Backlight ON
#endif

		DDRC  = 0b00000001;
		PORTC = 0b00001110;		// Pullups

		BUSY_ON() ;
	//	BUSY_OFF() ;

		DDRC |= 0b00111100;		// DEBUG

	//	delay_ms(200);

		// Set up serial I/F at 38400
	// **** We must set this bandwidth corresponding F_CPU
		UBRR0H = UBRR>>8 ;
		UBRR0L = UBRR ;
	// END**** We must set this bandwidth corresponding F_CPU
		UCSR0C = 0x06 ;
		UCSR0B = ( 1 << RXEN0) ;		// Do NOT enable Tx, allow trim to work
		UCSR0A = _BV(U2X0); //Double speed mode USART0
		DDRD &= ~0x01 ;		// RX pin as input
		PORTD |= 0x01 ;		// With a pullup
	
		DDRD &= ~0x02 ;			// Configure Tx pin as input to allow trim to work
		PORTD &= ~0x02 ;		// low, so no pullup

	// Set up Timer2 for timeouts
	// **** We must set this timeouts corresponding F_CPU
#ifdef SERIAL_VOICE
		UCSR0B |= ( 1 << TXEN0) ;	// Enable TX
		OCR2A = F_CPU / 32 / 4000 - 1;
		TCCR2A = 0b00000010;
		TCCR2B = 0b00000011;
		TIMSK2 |= _BV(OCIE2A);
#ifdef SERIAL_RECEIVE
		rx_fifo_running = 1 ;
		LastSerialData = 0xFF ;
#endif
#else
	// Counts round every 21mS at 12 MHz
		TCCR2A = 0 ;
		TCNT2 = 0 ;
		TCCR2B = 7 ;		// Clock div 1024
#endif
	// END**** We must set this timeouts corresponding F_CPU

		GPIOR0 = 0x10;	// Last a_clock set high
		GPIOR1 = 0;
		GPIOR2 = 0;
		EICRA = 0b00000011;
#ifndef SERIAL_RECEIVE
		EIMSK = 0b00000001; //select interrupt on rising edge on PD2(INT0)
#endif
		sei();
	//	i=0;
	
	//	tx_single_byte( 'X' ) ;

#ifdef TRIM_ON_DATA
		if ( ( PIND & 0b00011000 ) != 0b00011000 )
#else
		if ( ( PINC & 0b00001100 ) != 0b00001100 )
#endif
		{
			goToBootloader() ;
		}

		for (;;)
		{
			uint8_t request ;

	//		if (pf_mount(&Fs) == FR_OK)
			ymount() ;
			{	// Initialize FS

				BUSY_OFF() ;
				GPIOR0 = 0x10;
				GPIOR1 = 0;
				GPIOR2 = 0;

				/* Main loop */
				rc = 0 ;
				do
				{
					CommandTimeout = 0 ;
#ifndef SERIAL_VOICE
					TIFR2 = (1 << TOV2) ;			// Clear timer 2 overflow flag
#endif
					{
						while (! ( request = chk_input() ) )
							;
						if ( request == 2 )
						{
							break ;		// From 'do'
						}
					}
					if (Command!=0xFFFF)
					{
	//					printf("%d", Command);
						BUSY_ON() ;
						rc = playWav(Command);				/* Play corresponding audio file */
	//					rc = play(0001);				/* Play corresponding audio file */
						BUSY_OFF() ;
						audio_off();	/* Disable audio output */
						if (rc != 1) Cmd = 0;		/* Clear code when normal end or error */
					}
				} while (rc != 2);				/* Continue while no disk error */
				if ( request == 2 )
				{
					break ;		// From 'for'
				}
				BUSY_ON() ;
			}
			if ( pollBoot() )
			{
	//			tx_single_byte( 'P' ) ;
				break ;
			}
		}

		EIMSK &= ~0b00000001; // disable interrupt on rising edge on PD2(INT0)
		XferMode = 1 ;				// Now in serial transfer mode
		UCSR0B |= ( 1 << TXEN0) ;	// Enable TX

		OCR2A = F_CPU / 32 / 4000 - 1;
		TCCR2A = 0b00000010;
		TCCR2B = 0b00000011;
		TIMSK2 |= _BV(OCIE2A);
	
		tx_single_byte( '\r' ) ;
		tx_single_byte( '\n' ) ;
		tx_single_byte( '>' ) ;

		ymount() ;
	 
		for(;;)
		{ // Now in PC communication mode
			if ( ymodem_Receive() )
			{
				break ;
			}
		}
	}
}

uint8_t pollBoot()
{

#ifdef SERIAL_RECEIVE
	uint8_t x = 0 ;
	if ( BootReason )
	{
		x = BootReason ;
		BootReason = 0 ;
	}
	return x ;
#endif

  if ((UCSR0A & _BV(RXC0)))
	{
		uint8_t chr ;
  	chr = UDR0 ;
		if ( chr == 0x20 )
		{
			if ( LastSerialRx == 0x30 )
			{
				// Go to Bootloader
				goToBootloader() ;
			}
		}
		else if ( ( chr == 0x1B ) || ( chr == 0x1C ) )
		{
			tx_single_byte( 'E' ) ;
			if ( LastSerialRx == 0x1B )
			{
				tx_single_byte( 'F' ) ;
				return ( BootReason = (chr - 0x1A) ) ;
			}
		}

#if TESTING
		else if ( (chr >= '1') && (chr <= '9') )
		{
			tx_single_byte( chr ) ;
			BUSY_ON() ;
			playWav( chr - '0') ;	/* Play corresponding audio file */
			BUSY_OFF() ;
			audio_off();		/* Disable audio output */
		}
#endif
		
		LastSerialRx = chr ;
	}
	return 0 ;
}

void tx_single_byte( uint8_t byte )
{
	while ( Serial_busy )
	{
		// wait
	}
	UCSR0B |= ( 1 << TXEN0) ;	// Enable TX
	UDR0 = byte ;
	UCSR0A = ( 1 << TXC0 ) | _BV(U2X0) ;		// CLEAR flag
	Serial_busy = 1 ;
	UCSR0B |= ( 1 << TXCIE0 ) ;	// Enable interrupt
}

ISR(USART_TX_vect)
{
	DDRD &= ~0x02 ;			// Configure pin as input
	PORTD &= ~0x02 ;		// low, so no pullup
	UCSR0B &= ~( 1 << TXCIE0 ) ;	// Disable interrupt
	
	if ( XferMode == 0 )
	{
		UCSR0B &= ~( 1 << TXEN0 ) ;	// Disable TX
	}

	Serial_busy = 0 ;
}

//ISR(TIMER1_COMPA)
//{
//	uint8_t x = FifoCt ;
//	if ( x >= 2 )
//	{
//		x -= 2 ;
//		FifoCt = x ;
//		uint8_t rp = FifoRi ;
//		uint8_t *p = &Abuff.Buff.bytes[rp] ;
//		OCR0A = *p ;
//		OCR0B = *(p+1) ;
//		rp += 2 ;
//		FifoRi = rp ;
//	}
//}
