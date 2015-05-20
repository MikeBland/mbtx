/* 768
 * Voice for distance (every 50 m) and altitude (every 10 m) 
 * beep and voice switchable with UP[short] and DOWN[short]
 * added distance calculation (approximation)
 * added menu5
 * added HUB and Open9x parameter
 *
 * Author - Karl Szmutny <shadow@privy.de>
 * Author - Uphiearl and Jean-Pierre PARISY
 * Modified to accept NMEA records GGA and RMC - ReSt and Jean-Pierre PARISY
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
 */

#if defined(OPEN9X)
	#include "open9x.h"
	#define ATTRIB 2
#else  // defined(ER9X)
	#include "er9x.h"
	#define ATTRIB 1
#endif

#include "nmea.h"

#if defined(OPEN9X)
	#define my_lcd_puts lcd_puts
#else
	#define my_lcd_puts lcd_puts_P
#endif





#define MaxCount 2			//supress dist display for MaxCount xx seconds on startup
#define max_pass 1000
#define max2_pass 1.5*max_pass
#define EOS 0
#define DP 0x2E
#define LG_BUF 11				//14

#if defined(HUB)
	#define NB_LONG_BUF 10		//9  !!!!!!!!!!!!!!!!!!
	uint8_t lclreceived = 0;
#else
	#define NB_LONG_BUF 5
#endif

#define NB_SHORT_BUF 4
#define LONG_BUF(val)  (val)
#define SHORT_BUF(val)  (val+NB_LONG_BUF)
#define VALSTR(val)  (rbuf[val][0] ? rbuf[val] : val_unknown)
#define APSIZE (BSS | DBLSIZE)

uint8_t i;  						// working variable
uint8_t state;  				    		// currrent state
uint8_t rval, rpack;	    				// received items
uint8_t xval[NB_LONG_BUF+NB_SHORT_BUF];		// expected value
uint8_t xpack[NB_LONG_BUF+NB_SHORT_BUF];		// expected packet
uint8_t ibuf[NB_LONG_BUF];				// subscripts on long buffers values

char rbuf[NB_LONG_BUF][LG_BUF];				// long receive buffers
char sbuf[NB_SHORT_BUF];					// short receive buffers

const char val_unknown[] = "?";
int32_t rel_alt, prev_alt, lift_alt, v_prev_alt;	// integer values for altitude computations
int32_t home_alt, save_alt, max_alt, abs_alt;		// integer values for altitude computations
int32_t gpstimer=0;
int32_t gpstime=0;
uint8_t ggareceived, rmcreceived = 0;
uint8_t beep_on;
uint8_t show_timer=0;
uint8_t fixed=0;



/*    Received data
Data are received as packets, each packet is identified by a prefix of seven
characters ('$GPGGA,' , '$GPRMC,' , '$GPLCL')and is ended by one star plus two bytes checksum.
The values are terminated by a comma.

$GPGGA - Global Positioning System Fix Data, Time, Position and fix related data fora GPS receiver.

                                                      11
        1         2       3 4        5 6 7  8   9  10 |  12 13  14   15
        |         |       | |        | | |  |   |   | |   | |   |    |
    GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh<CR><LF>

Field Number:
  1) Universal Time Coordinated (UTC)
  2) Latitude
  3) N or S (North or South)
  4) Longitude
  5) E or W (East or West)
  6) GPS Quality Indicator,
     0 - fix not available,
     1 - GPS fix,
     2 - Differential GPS fix
  7) Number of satellites in view, 00 - 12
  8) Horizontal Dilution of precision
  9) Antenna Altitude above/below mean-sea-level (geoid)
 10) Units of antenna altitude, meters
 11) Geoidal separation, the difference between the WGS-84 earth
     ellipsoid and mean-sea-level (geoid), "-" means mean-sea-level
     below ellipsoid
 12) Units of geoidal separation, meters
 13) Age of differential GPS data, time in seconds since last SC104
     type 1 or 9 update, null field when DGPS is not used
 14) Differential reference station ID, 0000-1023
 *
 15) Checksum
 CrLf


$GPRMC - Recommended Minimum Navigation Information
                                                            12
        1         2 3       4 5        6 7   8   9    10  11|
        |         | |       | |        | |   |   |    |   | |
    RMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxx,x.x,a*hh<CR><LF>

 Field Number:
  1) UTC Time
  2) Status, V = Navigation receiver warning
  3) Latitude
  4) N or S
  5) Longitude
  6) E or W
  7) Speed over ground, knots
  8) Track made good, degrees true. = =  Course over ground (COG)
  9) Date, ddmmyy
 10) Magnetic Variation, degrees
 11) E or W
 12) Checksum
 CrLf


$GPLCL - Local HUB Data, especially Analog voltages and RPM
        1   2   3    4    5    6    7     8 9 
        |   |   |    |    |    |    |     | | 
    LCL,aaa,bbb,cccc,dddd,eeee,ffff,ggggg,x*hh<CR><LF>

 Field Number:
  1) Voltage 1
  2) Voltage 2
  3) Voltage 3
  4) Voltage 4
  5) Voltage 5
  6) Voltage 6
  7) Digital In
  8) RPM
  9) RSSI 0-100
 10) Checksum
 CrLf


*/

// GGA record prefix
#define PACK_GGA 0x47		// "G"
#define PACK_GGA3 0x41		// "A"
// value occurence number in the GGA packet
#define TIM 1
#define LAT 2
#define NOS 3
#define LON 4
#define EOW 5
#define FIX 6
#define SAT 7
#define DIL 8
#define ALT 9
#define MTR 10
#define GEO 11
#define MET 12
#define AGE 13
#define DIF 14

// RMC record prefix
#define PACK_RMC 0x52		// "R"
#define PACK_RMC2 0x4D		// "M"
#define PACK_RMC3 0x43		// "C"
// value occurence number in the RMC packet
#define TIM 1
#define NRW 2
#define LT1 3
#define NSO 4
#define LN1 5
#define EWE 6
#define SOG 7
#define COG 8
#define DAT 9
#define MAG 10
#define EAW 11

#if defined(HUB)
// LCL record prefix
#define PACK_LCL 0x4C		// "L"
#define PACK_LCL2 0x43		// "C"
#define PACK_LCL3 0x4C		// "L"
// value occurence number in the RMC packet
#define VOLT1 1
#define VOLT2 2
#define VOLT3 3
#define VOLT4 4
#define VOLT5 5
#define VOLT6 6
#define DIGIN 7
#define RPM 8
#define RSSI 9

const char Switches[7] = "654321";
#endif

// end of packet
#define PACK_END 0x2a			//  *
// end of value
#define VAL_END 0x2c			//  ,

// stateful machine

//   Since the packets are sent continuously, we need to synchronize on the
//   reception of the three chars prefixing a packet, whatever they are.

// states values
#define WAIT_PACKET     1
#define WAIT_PACK_GGA1  2
#define WAIT_PACK_GGA2  3
#define WAIT_PACK_GGA3  4
#define WAIT_PACK_RMC2  5
#define WAIT_PACK_RMC3  6

#if defined(HUB)
	#define WAIT_PACK_LCL1  7
	#define WAIT_PACK_LCL2  8
	#define WAIT_PACK_LCL3  9
#endif

#define WAIT_VAL_END	10
#define READ_VALUE      11

void menuProcNMEA1(uint8_t event);
void menuProcNMEA2(uint8_t event);
void menuProcNMEA3(uint8_t event);
void menuProcNMEA4(uint8_t event);

#if defined(HUB)
void menuProcNMEA5(uint8_t event);
#endif

void title(char x);
void initval(uint8_t num, uint8_t pack, uint8_t val);
int32_t binary (char *str);
int32_t bintime (char *str);
uint16_t sqrt32(uint32_t n);
int32_t split (char *str, uint8_t n);
void question(uint8_t x, uint8_t y);


void getGpsPilotPosition();
uint16_t getGpsDistance();


uint16_t GpsDistance=0;
uint16_t LastDistance=0;
uint16_t MaxDistance=0;
uint16_t prev_dist;
//uint16_t SaveDistance=0;
uint8_t increasing=0;
uint8_t decreasing=0;


uint32_t pilotLatitude=0;
uint32_t pilotLongitude=0;
uint16_t pilotAltitude=0;

uint32_t gpsLatitude_bp=0;
uint32_t gpsLatitude_ap=0;
uint32_t gpsLongitude_bp=0;
uint32_t gpsLongitude_ap=0;

/*
Der Abstand zwischen zwei Breitengraden ist konstant und beträgt immer 111.3 km
Der Abstand zwischen zwei Längengraden ist am Äquator ebenfalls 111.3 km, verringert sich jedoch mit dem cos des Breitengrades
*/
uint32_t metersPerDegreeLat=1113;	 			  //111300 Meter Latitude pro Grad 
uint32_t metersPerDegreeLon;

uint32_t pilotLatitudeCosinus;
uint8_t count=0;
uint16_t passes=0;
uint16_t a_pass=0;
uint16_t d_pass=0;
uint16_t satellites=0;
//uint16_t maxpasses=0;


//===============================================================================
ISR (USART0_RX_vect)
{
    uint8_t rl;
//    uint8_t rh;                         //USART control and Status Register 0 B
    uint8_t iostat;                     //USART control and Status Register 0 A

    rl = UDR0;
    iostat = UCSR0A;                    //USART control and Status Register 0 A
    /*
   bit 	7		6		5		4	3		2		1		0
        RxC0	TxC0	UDRE0	FE0	DOR0	UPE0	U2X0	MPCM0

        RxC0: 	Receive complete
        TXC0: 	Transmit Complete
        UDRE0: 	USART Data Register Empty
        FE0:	Frame Error
        DOR0:	Data OverRun
        UPE0:	USART Parity Error
        U2X0:	Double Tx Speed
        MPCM0:	MultiProcessor Comms Mode
*/
    if (iostat & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))		// check for errors
    {
        rl=  count = fixed = xpack[0] = xpack[1] = xval[0] = xval[1] = 0;
//        initval (LONG_BUF(2), PACK_GGA, TIM);   // always get UTC time for timer
        state = WAIT_PACKET;			         // restart on error
    }
//    rh = UCSR0B;                       //USART control and Status Register 0 B
    /* bit 	7		6		5		4		3		2		1		0
        RxCIE0	TxCIE0	UDRIE0	RXEN0	TXEN0	UCSZ02	RXB80	TXB80

        RxCIE0: Receive complete int enable
        TXCIE0: Transmit Complete int enable
        UDRIE0: USART Data Register Empty int enable
        RXEN0:	Rx enable
        TXEN0:	Tx Enable
        UCSZ02:	Character Size bit 2
        RXB80:	Rx data bit 8
        TXB80:	Tx data bit 8
*/
    switch (state)
    {
    case WAIT_PACKET:
        switch (rl)
        {
        case PACK_GGA:					// found a new GGA packet  "G"
            state = WAIT_PACK_GGA2;		// wait for the 2nd char
            break;
        case PACK_RMC:					// found a new RMC packet  "R"
            state = WAIT_PACK_RMC2;		// wait for the 2nd char
            break;
#if defined(HUB)
        case PACK_LCL:					// found a new LCL packet  "L"
            state = WAIT_PACK_LCL2;		// wait for the 2nd char
            break;
#endif
        }
        break;

    case WAIT_PACK_GGA2:				// received 2nd char  "G"
        if (rl == PACK_GGA)
            state = WAIT_PACK_GGA3;			// wait for 3rd character "A"
        else
            state = WAIT_PACKET;		// restart if not "G"
        break;
    case WAIT_PACK_GGA3:				// received 3rd char	"A"
        if (rl == PACK_GGA3)				// found
        {
            state = WAIT_VAL_END;		// wait for ","
            rpack = PACK_GGA;			// "G"
        }
        else
            state = WAIT_PACKET;		// restart if not found
        break;

    case WAIT_PACK_RMC2:				// wait for 2nd char	"M"
        if (rl == PACK_RMC2)
            state = WAIT_PACK_RMC3;
        else
            state = WAIT_PACKET;		// restart if not found
        break;
    case WAIT_PACK_RMC3:				// wait for 3rd char	"C"
        if (rl == PACK_RMC3)
        {
            state = WAIT_VAL_END;		// wait for ","
            rpack = PACK_RMC;			// "R"
        }
        else
            state = WAIT_PACKET;		// restart if not found
        break;

#if defined(HUB)
    case WAIT_PACK_LCL2:				// wait for 2nd char	"C"
        if (rl == PACK_LCL2)
            state = WAIT_PACK_LCL3;
        else
            state = WAIT_PACKET;		// restart if not found
        break;
    case WAIT_PACK_LCL3:				// wait for 3rd char	"L"
        if (rl == PACK_LCL3)
        {
            state = WAIT_VAL_END;		// wait for ","
            rpack = PACK_LCL;			// "R"
        }
        else
            state = WAIT_PACKET;		// restart if not found
        break;
#endif

    case WAIT_VAL_END:
        if (rl == VAL_END)			// "," nach "GGA" oder "RMC" oder "LCL"
        {
            state = READ_VALUE;
            rval = 1;
            for (i = 0; i < NB_LONG_BUF; i++)	// clear buffer
                ibuf[i] = 0;
        }
        else
            state = WAIT_PACKET;		// restart if not found
        break;


    case READ_VALUE:
        switch (rl)
        {
        case PACK_END:				// "*"
            if (rpack == PACK_GGA)
                ggareceived = 1;
		else if (rpack == PACK_RMC)
		    rmcreceived =1;
#if defined(HUB)
		else if (rpack == PACK_LCL)
		    lclreceived =1;
#endif

            state = WAIT_PACKET;		// packet completed, wait for the next packet
            break;

        case VAL_END:					// comma found, value completed
            rval++;						// and get next value
            break;

        default:						// store the char in the corresponding buffer
            for (i = 0; i < NB_LONG_BUF; i++)
            {							// is it the expected value in the expected packet ?
                if (rpack == xpack[i] && rval == xval[i] && ibuf[i] < LG_BUF - 1)
                {						// yes, store the char
                    rbuf[i] [ibuf[i]] = rl;
                    ibuf[i]++;
                    rbuf[i] [ibuf[i]] = 0;
                }
            }
            for (i = NB_LONG_BUF; i < NB_LONG_BUF+NB_SHORT_BUF; i++) {
                if (rpack == xpack[i]   // is this the expected short value in the expected packet ?
                        &&  rval == xval[i])
                    sbuf[i-NB_LONG_BUF] = rl;      // yes, store the char
            }
        }
        break;
    }
}

//===============================================================================
void NMEA_Init (void)
{
    DDRE  &= ~(1 << DDE0);              // set RXD0 pin as input
    PORTE &= ~(1 << PORTE0);            // disable pullup on RXD0 pin

    // switch (Telem_baud)
    // {
    // case 1:
#undef BAUD
#define BAUD 9600
#include <util/setbaud.h>
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    //    break;
    // }

    UCSR0A &= ~(1 << U2X0);			// disable double speed operation
    // set 8N1
    UCSR0B = 0|(0<< RXCIE0)|(0<<TXCIE0)|(0<<UDRIE0)|(0<<RXEN0)|(0<<TXEN0)|(0<<UCSZ02);
    UCSR0C = 0|(1 << UCSZ01) | (1 << UCSZ00);
    while ( UCSR0A & (1 << RXC0) )
        UDR0;							// flush receive buffer
    fixed = home_alt = rel_alt = ggareceived =0;
    gpstimer = -1;
    beep_on=3;
}

//===============================================================================
/*
// TX Capabilities are not required for NMEA
void NMEA_DisableTXD (void)
 {
    UCSR0B &= ~(1 << TXEN0);            // disable TX
 }

 void NMEA_EnableTXD (void)
 {
    UCSR0B |=  (1 << TXEN0);            // enable TX
 }
*/

//===============================================================================
void NMEA_DisableRXD (void)
{
    UCSR0B &= ~(1 << RXEN0);            // disable RX
    UCSR0B &= ~(1 << RXCIE0);           // disable Interrupt
}

//===============================================================================

void NMEA_EnableRXD (void)
{
    for (i = 0; i < NB_LONG_BUF; i++)
    {
        ibuf[i] = 0;
        rbuf[i][0] = 0;
        xpack[i] = 0;
        xval[i] = 0;
    }
    state = WAIT_PACKET;					// wait for the next packet
    UCSR0B |=  (1 << RXEN0);				// enable RX
    UCSR0B |=  (1 << RXCIE0);				// enable Interrupt
}

//===============================================================================
void menuProcNMEA(uint8_t event)
{
    menuProcNMEA1(event);
}

//===============================================================================
// Start of NMEA menus 1-5 <<<<<<<<<<<<<<<<<<<<<<<<<<<

void menuProcNMEA1(uint8_t event)
{
    if (rmcreceived) initval (LONG_BUF(0), PACK_RMC, TIM);		// sets rbuf[0][.]
    passes +=1;
    count=0;
    switch(event)						// new event received, branch accordingly
    {
    case EVT_KEY_BREAK(KEY_LEFT):
#if defined(HUB)
        chainMenu(menuProcNMEA5);
#else
        chainMenu(menuProcNMEA4);
#endif
        break;
    case EVT_KEY_BREAK(KEY_RIGHT):
        chainMenu(menuProcNMEA2);
        break;
    case EVT_KEY_LONG(KEY_UP):
        NMEA_DisableRXD();
        chainMenu(menuProcStatistic);
        break;
    case EVT_KEY_LONG(KEY_DOWN):
        NMEA_DisableRXD();

#if defined(OPEN9X)
        chainMenu(menuMainView);
#else
        chainMenu(menuProc0);
#endif

        break;
    case EVT_KEY_FIRST(KEY_MENU):
        if (show_timer == 0) {
            show_timer = 1;
            if (gpstimer <= 0)
                gpstimer = bintime(rbuf[0]);
        }
        else
            show_timer = 0;
        break;
    case EVT_KEY_FIRST(KEY_EXIT):
        if ((show_timer == 1) &&(rbuf[0][0])) {
            gpstimer = bintime(rbuf[0]);		// get actual GPS time ->resets timer to 00:00
		}
        break;
    }
    /*
    How to use:

    You choose the values to be displayed using the function:

      initval(<number>, <packet>, <value>);
      -------------------------------------

    That means that "<value>" of "<packet>" is stored in the <number> buffer.
    The first <number> is 0.

	$GPGGA,125109.272,5014.7262,N,01123.9966,E,2,07,1.2,624.7,M,47.7,M,1.9,0000*77
	$GPRMC,125109.272,A,5014.7262,N,01123.9966,E,33.439922,47.98,230711,,*09
    Here are the packet names and the associated value names:

    Position packet (beginning with "GGA"): "PACK_GGA"
    value names: "TIM", "LAT", "NOS", "LON", "EOW", "FIX", "SAT", "DIL", "ALT", "MTR", "GEO", "MET", "AGE", "DIF",

    Required minimum packet (beginning with "RMC"): "PACK_RMC"
       value names: "TIM", "NRW", "LT1", "NSO", "LN1", "EWE", "SOG", "COG", "DAT", "MAG", "EAW"

    The buffers are accessed using the macro "VALSTR(<n>)", where "<n>" is "0"
    for the first buffer, and "1" for the second buffer.

    sbuf is a single character buffer and contains the last value of a field ????
    rbuf[x][y] contains the full field

    When a value is missing, it is replaced by the contents of val_unknown ("?").
*/

    title ('1');
    my_lcd_puts        (   2*FW,   1*FH, PSTR("UTC-Time      Sat"));

    if (ggareceived) {
	  ggareceived=0;
	  passes=0;
	  initval (SHORT_BUF(1), PACK_GGA, FIX);				// -> sbuf[1]
	  fixed=(sbuf[1]>0x30) ? ATTRIB : 0 ;
    }

    if (rmcreceived) {
	  rmcreceived=0;
	  initval (LONG_BUF(1), PACK_RMC, DAT);					// sets rbuf[1][.]
	  gpstime=bintime(rbuf[0]);
    }

    if (rbuf[0][0]) {						// show always if RMC data have been received
	  initval (LONG_BUF(4), PACK_GGA, SAT);				// -> rbuf[4]			
	  lcd_putsnAtt  (  19*FW,   1*FH, &rbuf[4][0], 2, 16+fixed);	// satellites in view, invers if Fixed
	
     	  lcd_putsnAtt  (   2*FW,   2*FH, &rbuf[0][0], 2, APSIZE);		// hours
        lcd_putcAtt   (   6*FW,   2*FH, ':', DBLSIZE);			// ":"
     	  lcd_putsnAtt  (   8*FW,   2*FH, &rbuf[0][2], 2, APSIZE);		// minutes
        lcd_putcAtt   (  12*FW,   2*FH, ':', DBLSIZE);			// ":"
     	  lcd_putsnAtt  (  14*FW,   2*FH, &rbuf[0][4], 2, APSIZE);		// seconds
    }
    else
     	  lcd_putsAtt   (   2*FW,   2*FH, val_unknown, APSIZE);		// "?"


    if (show_timer)  {							// show the Timer when data have been received

        my_lcd_puts    (   2*FW,   4*FH, PSTR("Timer"));			// display "Timer"
     	  putsTime      (   5*FW,   5*FH, (gpstime-gpstimer), DBLSIZE, DBLSIZE);	// display difference as mm:ss
    }
    else
    {
     	  my_lcd_puts      ( 2*FW,   4*FH, PSTR("Date"));			// show the UTC Date	

        if (rbuf[1][0])	{
     	      lcd_putsnAtt( 2*FW,   5*FH, &rbuf[1][0], 2, APSIZE);		// year
           	lcd_putcAtt ( 6*FW,   5*FH, '/', DBLSIZE);			// "/" 
            lcd_putsnAtt( 8*FW,   5*FH, &rbuf[1][2], 2, APSIZE);		// month
     	      lcd_putcAtt (12*FW,   5*FH, '/', DBLSIZE);			// "/"
           	lcd_putsnAtt(14*FW,   5*FH, &rbuf[1][4], 2, APSIZE);		// day
        }
    } 
     question(9,3);		// large blinking Questionmark in case of timeout

}

//==============================================================================
void menuProcNMEA2(uint8_t event)
{
    static int8_t set_home;
    passes +=1;

    switch(event)
    {
// Menu navigation
    case EVT_KEY_BREAK(KEY_LEFT):
        chainMenu(menuProcNMEA1);
        break;
  
    case EVT_KEY_BREAK(KEY_RIGHT):
        chainMenu(menuProcNMEA3);
        break;

//Beep setting
    case EVT_KEY_BREAK(KEY_UP):					
        killEvents(event);
	  beep_on = beep_on ^ 1;					// toggle Beep ON/OFF				
        audioDefevent(AU_MENUS); 					// short blip 
        break;

    case EVT_KEY_LONG(KEY_UP):
        NMEA_DisableRXD();
        chainMenu(menuProcStatistic);
        break;

//Voice setting
    case EVT_KEY_BREAK(KEY_DOWN):					// Switch Voice on/off
        killEvents(event);
	  beep_on = beep_on ^ 2;					// toggle Voice ON/OFF
        audioDefevent(AU_MENUS); 					// short blip
	  v_prev_alt=0;
	  prev_dist=0;
        break;

    case EVT_KEY_LONG(KEY_DOWN):
        NMEA_DisableRXD();

#if defined(OPEN9X)
        chainMenu(menuMainView);
#else
        chainMenu(menuProc0);
#endif

/*
        break;
//Beep setting
    case EVT_KEY_FIRST(KEY_UP):					
        killEvents(event);
	  beep_on = beep_on ^ 1;					// toggle Beep ON/OFF				
        audioDefevent(AU_MENUS); 					// short blip 
        break;

//Voice setting
    case EVT_KEY_FIRST(KEY_DOWN):					// Switch Voice on/off
        killEvents(event);
	  beep_on = beep_on ^ 2;					// toggle Voice ON/OFF
        audioDefevent(AU_MENUS); 					// short blip
	  v_prev_alt=0;
	  prev_dist=0;
        break;
*/
/*
Altitude setting:
	        Set a home position for altitude. Normally used before starting the model
		  when GPS has got a fix.

	        MENU[short]         		-->	alternating relative and absolute altitudes
		  MENU[long]			-->   set home altitude to current altitude
		  EXIT[long]			-->   reset max altitude to 0

	  Switch ON / OFF short beep with positive lift
		  DOWN[short]			-->	Toggle Positive lift Beep on/off

	  Switch ON / OFF voice for altitude and distance
		  UP[short]				-->	Toggle voice message of altitude and distance on/off		
*/


    case EVT_KEY_BREAK(KEY_MENU):		// Menu short
        if (!home_alt)				// umschalten zwischen absoluter und relativer Höhe
            home_alt = save_alt;
        else
            home_alt=0;	

	  if (save_alt==0)			// wenn noch keine Home Höhe gesetzt war, wird sie es jetzt, weil sonst
							// das Umschalten keine Wirkung zeigt
	      save_alt = home_alt = abs_alt;			// absolute altitude
        audioDefevent(AU_MENUS);  						// short blip for non negative lift
        break;

    case EVT_KEY_LONG(KEY_MENU):		// Menu long, set home position
        killEvents(event);
	  set_home=1;
        save_alt = home_alt =abs_alt ;				// Home altitude auf aktuelle absolute Höhe setzen 
	  max_alt=0;							// es ist irritierend, wenn max_alt bestehen bliebe
	  d_pass = max_pass>>1;

        audioDefevent(AU_MENUS); 						// short blip
        break;

    case EVT_KEY_BREAK(KEY_EXIT):		// Exit short resets max_alt and MaxDistance
	  MaxDistance = max_alt = 0;
        audioDefevent(AU_MENUS); 						// short blip
	  break;

    case EVT_KEY_LONG(KEY_EXIT):		// Exit long resets all distance and altitude values
        killEvents(event);
	 pilotLatitude=pilotLongitude=LastDistance=MaxDistance=save_alt = home_alt = abs_alt=max_alt=increasing=decreasing=0;
        audioDefevent(AU_MENUS); 						// short blip
        break;
    }


	title ('2');

	my_lcd_puts         (   1*FW,   1*FH, PSTR("Altitude Sat   Max"));
	my_lcd_puts         (   16*FW,   3*FH, PSTR("Home"));
	my_lcd_puts         (   1*FW,   4*FH, PSTR("Lift") );

	lcd_putsAtt (16*FW, 5*FH, PSTR("Beep"), ((beep_on==1)|(beep_on==3)));  
	lcd_putsAtt (16*FW, 6*FH, PSTR("Voice"), (beep_on >=2));  

/*
	lcd_putcAtt( 16*FW, 5*FH, 'B', ((beep_on==1)|(beep_on==3)));  	//first letter inverted if active
      my_lcd_puts(   17*FW,   5*FH, PSTR("eep") );

	lcd_putcAtt( 16*FW, 6*FH, 'V', (beep_on>=2)) ;				//first letter inverted if active
      my_lcd_puts( 17*FW, 6*FH, PSTR("oice") );
*/

      lcd_outdezNAtt(  20*FW,   4*FH, home_alt, PREC1, 6);		// display home_alt, small characters 

	rmcreceived=0;		// RMC Pack not used

//----------------------------------------
	if (ggareceived)   // at least one second has elapsed
	{
      	ggareceived = 0;
		passes=0;
		if (count < MaxCount) count +=1;


		initval (LONG_BUF(0), PACK_GGA, ALT);				// -> rbuf[0]
		initval (LONG_BUF(1), PACK_GGA, GEO);				// -> rbuf[1]

		initval (SHORT_BUF(0), PACK_GGA, MTR);			// -> sbuf[0]


		initval (SHORT_BUF(1), PACK_GGA, FIX);				// -> sbuf[1]
		initval (SHORT_BUF(2), PACK_GGA, SAT);				// -> sbuf[2]

		fixed=(sbuf[1]>0x30) ? ATTRIB : 0 ;
		satellites=sbuf[2];


        /*      ALT and GEO have one single digit following the decimal point
        e.g. ALT=359.7   GEO=47.7
        The altitude over mean sea level is to be calculated as:
        altitude minus geoidal separation  
        */

		abs_alt = binary(rbuf[0]) - binary(rbuf[1]);		// alt - geo  that is absolute altitude

		if (abs_alt> max_alt) max_alt=abs_alt;			// hold max altitude relative to 0 m

		rel_alt=abs_alt - home_alt;					// alt - geo - home  altitude relative to home


		initval (LONG_BUF(2), PACK_GGA, LAT);				// -> rbuf[2]
		gpsLatitude_bp=split((rbuf[2]),DP);				// Latitude before decimal point
		gpsLatitude_ap=split((rbuf[2]),EOS); 				// Latitude after decimal point


		initval (LONG_BUF(3), PACK_GGA, LON);				// -> rbuf[3]
		gpsLongitude_bp=split((rbuf[3]),DP);				// Longitude before decimal point
		gpsLongitude_ap=split((rbuf[3]),EOS); 				// Longitude after decimal point

		if ((a_pass >= max_pass) | ((a_pass + d_pass) > max2_pass))
		{	
			// Höhe ändert sich
			uint16_t alt_diff = (rel_alt > v_prev_alt) ? rel_alt - v_prev_alt: v_prev_alt - rel_alt ;

			if ((fixed) && (beep_on & 0x2) && (alt_diff >= 100))		// diff of 10 m
		  	{
	      		v_prev_alt = rel_alt;			// kann auch negativ sein
				if (rel_alt > 0)				// nur positive Werte ansagen. voice kann nicht negative
				{
					a_pass = 0;
					putVoiceQueue(V_ALTITUDE) ;						//!!!!!!!!!!!!!!!!
					voice_numeric(rel_alt/10, 0,V_METRES);				//!!!!!!!!!!!!!!!!
				}
			}
		}
		lift_alt = rel_alt - prev_alt;
		prev_alt = rel_alt;
											//!!!!!!!!!!!!!!
		if ((lift_alt >= 0) && (fixed) && (beep_on & 0x1))			// GGA record must have Fix> 0	
			audioDefevent(AU_MENUS); 						// short blip for non negative lift
											//!!!!!!!!!!!!!!!!!
	}
//----------------------------------------

// Shown value depends on home_alt.
// For home_alt=0 this is altitude relativeto sea level
// For home_alt > 0 this is altitude relative to home level

	lcd_outdezNAtt(  20*FW,   2*FH, (max_alt-home_alt), PREC1, 6);	// max_altitude rel 0 or home

//----------------------------------------
	if (pilotLatitude | (set_home))				// show distance only, when home position has been set
	{
		my_lcd_puts    (  11*FW,   4*FH, PSTR("Dist") );
		GpsDistance=getGpsDistance();


		if ((d_pass >= max_pass) | ((a_pass + d_pass) > max2_pass))
		{
			uint16_t dist_diff = (GpsDistance > prev_dist) ? GpsDistance - prev_dist: prev_dist - GpsDistance ;
			if ( (fixed) && (beep_on & 0x2) && (dist_diff >= 500) )		// diff of 50 m 
			{
				prev_dist=GpsDistance ;
				putVoiceQueue(V_DISTANCE) ;
				voice_numeric(GpsDistance/10, 0, V_METRES) ;
				d_pass = 0;
			}
		}
// GetGpsDistance intermittently returns a wrong, much to high value (e.g.3284.2 instead of 15.3)
// Try to catch it and avoid MaxDistance to go incorrectly high
// MaxDistance is only set, if two consecutive values are greater than MaxDistance

            if (count==MaxCount) 
		{
			if (set_home) 
			{
				LastDistance=MaxDistance=increasing=decreasing=0;
				getGpsPilotPosition();	
				pilotAltitude = abs_alt;
				set_home=0;
			}

			if (GpsDistance > MaxDistance) 
			{
				if (increasing)	
				{
					MaxDistance=LastDistance;
					increasing=0;
				}
				else	
				{
					increasing=1;
				}
		      }
     	      	else increasing=0;
	
			LastDistance=GpsDistance;
		      lcd_outdezNAtt(  15*FW,   5*FH, MaxDistance, PREC1, 6);   // Distance 
   		      lcd_outdezNAtt(  15*FW,   6*FH, GpsDistance, PREC1, 6);   // Distance 
           	}
		// on a communication loss, after a delay of some seconds, the last values are displayed
		else if (passes > 300) 
		{
			lcd_outdezNAtt(  15*FW,   5*FH, MaxDistance, PREC1, 6);   	// show last MaxDistance 
			lcd_outdezNAtt(  15*FW,   6*FH, LastDistance, PREC1, 6);   	// show LastDistance 
			lcd_putsAtt    ( 12*FW,  2*FH, val_unknown, 22);	// large blinking Questionmark above Dist.
		}
	}
//--------for Test only --------------------------------
/*
	initval (LONG_BUF(4), PACK_GGA, SAT);			// -> rbuf[4]	ER9X		open9x
	lcd_putsnAtt  (  12*FW,   2*FH, &rbuf[4][0], 2, 16);		//   small 		normal
	lcd_putsnAtt  (  12*FW,   3*FH, &rbuf[4][0], 2, 17);		//   invers		BLINK
	lcd_putsnAtt  (  12*FW,   4*FH, &rbuf[4][0], 2, 18);		//   blink		INVERS
	lcd_putsnAtt  (  12*FW,   5*FH, &rbuf[4][0], 2, 20);		//   big normal
*/
//----------------------------------------

#if defined(HUB)
	initval (LONG_BUF(5), PACK_LCL, RSSI);						// -> rbuf[4]			
	lcd_putsnAtt  (  12*FW,   3*FH, &rbuf[5][0], 3, 16);		//   small normal
	my_lcd_puts   (  14*FW,   3*FH, PSTR("%") );
#endif



	if (rbuf[0][0])	 
	{	
		initval (LONG_BUF(4), PACK_GGA, SAT);						// -> rbuf[4]			
		lcd_putsnAtt  (13*FW,   1*FH, &rbuf[4][0], 2, 16+fixed);	// satellites in view, //invers if Fixed	
		if (sbuf[1]>0x30)	
		{							// & GGA has FIX > "0"
			lcd_outdezNAtt(  10*FW,   2*FH, rel_alt, DBLSIZE|PREC1, 7);	// actual altitude
			lcd_putcAtt   (  10*FW,   3*FH, sbuf[0]+0x20, 0);	// dimension [m] as lower case

			lcd_outdezNAtt(  8*FW,   5*FH, lift_alt, DBLSIZE|PREC1, 6);	// lift

			my_lcd_puts    (  5*FW,   4*FH, PSTR("[ /s]") );
			lcd_putcAtt   (  6*FW,   4*FH, sbuf[0]+0x20, 0);	// dimension [m/s] as lower case
		}
	}
//----------------------------------------

//  in case we do not receive GGA packages(due to poor receiption) the passes count will increase accordingly.
    question(12,2);		// large blinking Questionmark in case of timeout
}


//===============================================================================
void menuProcNMEA3(uint8_t event)
{
    passes +=1;
    count=increasing=0;

    switch(event)
    {
    case EVT_KEY_BREAK(KEY_LEFT):
        chainMenu(menuProcNMEA2);
        break;
    case EVT_KEY_BREAK(KEY_RIGHT):
        chainMenu(menuProcNMEA4);
        break;
    case EVT_KEY_LONG(KEY_UP):
        NMEA_DisableRXD();
        chainMenu(menuProcStatistic);
        break;
    case EVT_KEY_LONG(KEY_DOWN):
        NMEA_DisableRXD();

#if defined(OPEN9X)
        chainMenu(menuMainView);
#else
        chainMenu(menuProc0);
#endif

        break;
    }
    if (rmcreceived) {
	    rmcreceived=0;
	    passes=0;
	    initval (LONG_BUF(0), PACK_RMC, SOG);
	    initval (LONG_BUF(1), PACK_RMC, COG);
    }
    if (ggareceived) {
	    initval (SHORT_BUF(1), PACK_GGA, FIX);				// -> sbuf[1]
	    fixed=(sbuf[1]>0x30) ? ATTRIB : 0 ;
	    ggareceived=0;
//	    passes=0;
    }

    title ('3');
    my_lcd_puts        (   0*FW,   1*FH, PSTR("GrndSpeed[knt]  Sat"));
    my_lcd_puts        (   1*FW,   4*FH, PSTR("Course over ground") );
    if (rbuf[0][0])				// if first position is 00, buffer is empty, taken as false 
    {							// any other value is true
        uint8_t i = 0;
        while (rbuf[0][i])
        {
            if (rbuf[0][i] == '.')		// find decimal point and insert End of String 3 positions higher
            {
                rbuf[0][i+3] = 0;
                break;
            }
            i++;
        }
        lcd_putsAtt   (   2*FW,   2*FH, VALSTR(0), APSIZE);			// speed over ground
        lcd_putsAtt   (   2*FW,   5*FH, VALSTR(1), APSIZE);		// course over ground

	  initval (LONG_BUF(4), PACK_GGA, SAT);					// -> rbuf[4]			
	  lcd_putsnAtt  (19*FW,   1*FH, &rbuf[4][0], 2, 16+fixed);	// satellites in view, //?invers if Fixed	

    }
    question(9,3);		// large blinking Questionmark in case of timeout
}


//===============================================================================
void menuProcNMEA4(uint8_t event)
{
    passes +=1;
    switch(event)						// new event received, branch accordingly
    {
    case EVT_KEY_BREAK(KEY_LEFT):
        chainMenu(menuProcNMEA3);
        break;
    case EVT_KEY_BREAK(KEY_RIGHT):
#if defined(HUB)
        chainMenu(menuProcNMEA5);
#else
        chainMenu(menuProcNMEA1);
#endif
        break;
    case EVT_KEY_LONG(KEY_UP):
        NMEA_DisableRXD();
        chainMenu(menuProcStatistic);
        break;
    case EVT_KEY_LONG(KEY_DOWN):
        NMEA_DisableRXD();

#if defined(OPEN9X)
        chainMenu(menuMainView);
#else
        chainMenu(menuProc0);
#endif

        break;
    }

    if (ggareceived) {
	    ggareceived=0;
	    passes=0;

	    // expecting LAT value in POS packet to be stored in the first buffer
	    initval (LONG_BUF(0), PACK_GGA, LAT);
	    initval (SHORT_BUF(0), PACK_GGA, NOS);
	    // and LON value in POS packet stored in the second buffer
	    initval (LONG_BUF(1), PACK_GGA, LON);
	    initval (SHORT_BUF(3), PACK_GGA, EOW);

	    initval (SHORT_BUF(1), PACK_GGA, FIX);				// -> sbuf[1]
	    fixed=(sbuf[1]>0x30) ? ATTRIB : 0 ;
    }	

    // title of the screen
    title ('4');
    my_lcd_puts        (   3*FW,   1*FH, PSTR("Latitude     Sat"));    // line 1 column 3
    my_lcd_puts        (   3*FW,   4*FH, PSTR("Longitude"));   // line 4 column 5

    // first buffer into line 2 column 2
    if (rbuf[0][0])
    {
        lcd_putcAtt   (  13*FW,   1*FH, sbuf[0], 0);          // N or S

	  initval (LONG_BUF(4), PACK_GGA, SAT);					// -> rbuf[4]			
	  lcd_putsnAtt  (19*FW,   1*FH, &rbuf[4][0], 2, 16+fixed);	// satellites in view, //?invers if Fixed	

        lcd_putsnAtt  (   1*FW,   2*FH, rbuf[0], 2, APSIZE);
        lcd_putcAtt   (   5*FW,   2*FH, '@',0);
        lcd_putsAtt   (   6*FW,   2*FH, &rbuf[0][2], APSIZE);	// minutes with small decimal point

        lcd_putcAtt   (  13*FW,   4*FH, sbuf[3], 0);          // E or W
        lcd_putsnAtt  (   0*FW,   5*FH, rbuf[1], 3, APSIZE);
        lcd_putcAtt   (   6*FW,   5*FH, '@',0);
        lcd_putsAtt   (   7*FW,   5*FH, &rbuf[1][3], APSIZE);	// minutes with small decimal point

    }
    question(9,3);		// large blinking Questionmark in case of timeout


}



//===============================================================================
#if defined(HUB)
void menuProcNMEA5(uint8_t event)
{
    passes +=1;
    switch(event)
    {
    case EVT_KEY_BREAK(KEY_LEFT):
        chainMenu(menuProcNMEA4);
        break;
    case EVT_KEY_BREAK(KEY_RIGHT):
        chainMenu(menuProcNMEA1);
        break;
    case EVT_KEY_LONG(KEY_UP):
        NMEA_DisableRXD();
        chainMenu(menuProcStatistic);
        break;
    case EVT_KEY_LONG(KEY_DOWN):
        NMEA_DisableRXD();

#if defined(OPEN9X)
        chainMenu(menuMainView);
#else
        chainMenu(menuProc0);
#endif

        break;
    }

    title ('5');

    if (ggareceived) {
	    initval (SHORT_BUF(1), PACK_GGA, FIX);				// -> sbuf[1]
	    fixed=(sbuf[1]>0x30) ? ATTRIB : 0 ;
	    my_lcd_puts        (   0*FW,   1*FH, PSTR("HUB Data        Sat"));

	    initval (LONG_BUF(6), PACK_GGA, SAT);					// -> rbuf[4]			
	    lcd_putsnAtt  (19*FW,   1*FH, &rbuf[6][0], 2, 16+fixed);	// satellites in view, //?invers if Fixed	
	    ggareceived=0;
	    passes=0;
    }
	if (lclreceived) {
		    lclreceived=0;
	    }
	my_lcd_puts ( 0*FW,   3*FH, PSTR("V1=    V3=    V5="));    
	my_lcd_puts ( 0*FW,   4*FH, PSTR("V2=    V4=    V6="));   
	my_lcd_puts ( 0*FW,   7*FH, PSTR("RPM=        Sw="));  

      initval (LONG_BUF(0), PACK_LCL, VOLT1);					// -> rbuf[0]			
	if (rbuf[0][0])				// if first position is 00, buffer is empty, taken as false 
	    {							// any other value is true
	      lcd_outdezNAtt(   6*FW,   3*FH, binary(&rbuf[0][0]), PREC2, 3);		
      	initval (LONG_BUF(1), PACK_LCL, VOLT2);					// -> rbuf[1]			
	      lcd_outdezNAtt(   6*FW,   4*FH, binary(&rbuf[1][0]), PREC2, 3);		
      	initval (LONG_BUF(2), PACK_LCL, VOLT3);					// -> rbuf[2]			
	      lcd_outdezNAtt(   13*FW,   3*FH, binary(&rbuf[2][0]), PREC2, 4);		
      	initval (LONG_BUF(3), PACK_LCL, VOLT4);					// -> rbuf[3]			
	      lcd_outdezNAtt(   13*FW,   4*FH, binary(&rbuf[3][0]), PREC2, 4);		
      	initval (LONG_BUF(4), PACK_LCL, VOLT5);					// -> rbuf[4]			
	      lcd_outdezNAtt(   21*FW,   3*FH, binary(&rbuf[4][0]), PREC2, 4);		
      	initval (LONG_BUF(5), PACK_LCL, VOLT6);					// -> rbuf[5]		
	      lcd_outdezNAtt(   21*FW,   4*FH, binary(&rbuf[5][0]), PREC2, 4);		

      	initval (LONG_BUF(7), PACK_LCL, RPM);					// -> rbuf[7]			
	      lcd_outdezNAtt( 11*FW,   6*FH, binary(&rbuf[7][0]), DBLSIZE, 5);		


      	initval (LONG_BUF(8), PACK_LCL, DIGIN);					// -> rbuf[8]
		int32_t LongByte = binary(rbuf[8]) ;    
		// shift bit 5 to high order position; 
		int8_t MyByte=LongByte <<2;
		for (i=0;i<6;++i) {
			//if High order bit is ON value is negative, show number invers
			lcd_putsnAtt  ((15+i)*FW,   7*FH, &Switches[i], 1, 16+(MyByte < 0));		
			MyByte= MyByte <<1;
		}
	    }
    question(8,3);		// large blinking Questionmark in case of timeout
}
#endif

//==============================================================================
void question(uint8_t x, uint8_t y)
{
    if (passes>250) {
	   count=0;
	   fixed=0;
         lcd_putsAtt    ( x*FW,  y*FH, val_unknown, 22);		// large blinking Questionmark.
    }
	d_pass += 1;
	a_pass += 1;
	if (d_pass >= max_pass) d_pass = max_pass;
	if (a_pass >= max_pass) a_pass = max_pass;
}
//===============================================================================
void title(char x)
{
#if defined(HUB)
    lcd_putsAtt (0*FW, 0*FH, PSTR("  GPS NMEA data ?/5  "), INVERS);
#else
    lcd_putsAtt (0*FW, 0*FH, PSTR("  GPS NMEA data ?/4  "), INVERS);
#endif

    lcd_putcAtt(16*FW, 0*FH, x, INVERS);
}

//===============================================================================
void initval(uint8_t num, uint8_t pack, uint8_t val)
{
    if (xpack[num] != pack || xval[num] != val)
    {
        if (num < NB_LONG_BUF) {
            ibuf[num] = rbuf[num][0] = 0;
        }
        else
            sbuf[num-NB_LONG_BUF] = '?';
        xpack[num] = pack;
        xval[num] = val;
        state = WAIT_PACKET;			// synchronize to the next packet
    }
}

//===============================================================================
int32_t binary (char *str)
{
    int32_t numval = 0;
    uint8_t sign = 0;

    while (*str) {
        if (*str == '-')
            sign = 1;
        else if (*str >= '0' && *str <= '9')
            numval = numval * 10 + (*str - '0');
        str++;
    }
    if (sign)
        numval = -numval;
    return numval;
}

//===============================================================================
// split (rbuf[0],DP) returns binary value to Decimal Point
// split (rbuf[0],EOS) returns binary value after decimal point to EndOfString

int32_t split (char *str, uint8_t n)
{
	int32_t numval = 0;
	uint8_t sign = 1;
	
	// n=0 search decimal point
	if (n==0) {
		while (*str != DP) {
			str++;
		}
	}
	// convert til decimal point or end of string
	while (*str != n) {
		if (*str >= '0' && *str<= '9')
			numval = numval * 10 + (*str - '0');
		else if (*str == '-')
			sign = -1;
		str++;
	}      

	numval=numval*sign;
	return numval;
}

//===============================================================================
int32_t bintime (char *str)
{
    int32_t numval=0;

    if (*str) {
        numval = ((str[0] - '0') * 10l) + (str[1] - '0');					// hours
        numval = numval * 3600l;
        numval = numval + (((  (str[2] - '0') * 10l) + (str[3] - '0')) * 60l);	// minutes
        numval = numval + ((str[4] - '0') * 10l) + (str[5] - '0');			// seconds
    }
    return numval;
}


//===============================================================================
uint16_t sqrt32(uint32_t n)
{
  uint16_t c = 0x8000;
  uint32_t g = 0x8000;

  for(;;) {
    if(g*g > n)
      g ^= c;
    c >>= 1;
    if(c == 0)
      return g;
    g |= c;
  }
}

//=================================================================================
inline void getGpsPilotPosition()
{	
  pilotLatitude=(((uint32_t)gpsLatitude_bp / 100) * 1000000) + (((gpsLatitude_bp % 100) * 10000 + gpsLatitude_ap)*5)/3;
  pilotLongitude=(((uint32_t)gpsLongitude_bp / 100) * 1000000) + (((gpsLongitude_bp % 100) * 10000 + gpsLongitude_ap)*5)/3;

// pilotLatitude=50247528;		//50247528  = 50.247528 Grad  cos=0.63947
// pilotLongitude=11397561;	//11397561

//Umrechnug des Winkels von Grad in Bogenmaß RAD
uint32_t lat_rad = ((pilotLatitude / 100000) * 174532)/10000;  	// 1/10 grad 50.2° = 8761  (eigentlich = 0.8761)

//Berechnung des Cos nach der Taylorschen Reihe
// cos(x) = 1 - x2/2! + x4/4! +x6/6! ...
//Abbruch nach dem zweiten Glied, da dies im benötigten Bereich von +-90° genau genug ist

 pilotLatitudeCosinus = (100000000 - ((lat_rad/2) * lat_rad)) /1000; // 1 - x²/2 = 61622    eigentlich -> 0.61622

//Berechnung von x4/4!                                               //   x4/4!  =  2469    eigentlich -> 0.02469 
 lat_rad /= 100;					// --> 87 
 lat_rad *= lat_rad; 		 		// --> 7569
 lat_rad = (lat_rad * lat_rad)/24000;	// --> 2387
 pilotLatitudeCosinus += lat_rad ; 		//= 61622 + 2387   --> + 0.02387 -> 64009

//Meter longitude per Grad  1113 * 64009  -> 7124 bei Lat=50,2° = 712.4m
 metersPerDegreeLon= (1113 * pilotLatitudeCosinus)/10000;
}


//==========================================

inline uint16_t getGpsDistance()
{
  uint32_t lat = (((uint32_t)gpsLatitude_bp / 100) * 1000000) + (((gpsLatitude_bp % 100) * 10000 + gpsLatitude_ap)*5)/3;

  uint32_t lon = (((uint32_t)gpsLongitude_bp / 100) * 1000000) + (((gpsLongitude_bp % 100) * 10000 + gpsLongitude_ap)*5)/3;

// lat=50247528;	//50247528		1000 = ca 111 m
// lon=11387561;	//11397561		1000 = ca 70 m

  uint32_t lat_len = (lat > pilotLatitude) ? lat - pilotLatitude : pilotLatitude - lat;
  uint32_t lon_len = (lon > pilotLongitude) ? lon - pilotLongitude : pilotLongitude - lon;
  uint32_t alt_len = (abs_alt > pilotAltitude) ? abs_alt - pilotAltitude : pilotAltitude - abs_alt;


  lat_len = (lat_len * metersPerDegreeLat) / 1000;			// * 1113 
  lon_len = (lon_len * metersPerDegreeLon) / 10000;			// * 1113 * cos(lat)

 uint32_t temp_len = (sqrt32(lat_len*lat_len + lon_len*lon_len));  


//um den Gesamtabstand mit der Höhe zu rechnen muß erneut quadriert und Wurzel gezogen werden
 lon_len = sqrt32(temp_len*temp_len + alt_len*alt_len); // Ergebnis in 1/10tel Meter

return lon_len;


}
//===============================================================================

/*
EXT=NHT
Size after:
AVR Memory Usage
----------------
Device: atmega64

Program:   58938 bytes (89.9% Full)
(.text + .data + .bootloader)

Data:       3585 bytes (87.5% Full)
(.data + .bss + .noinit)


EXT=NOH
Size after:
AVR Memory Usage
----------------
Device: atmega64

Program:   60830 bytes (92.8% Full)
(.text + .data + .bootloader)

Data:       3613 bytes (88.2% Full)
(.data + .bss + .noinit)


*/
