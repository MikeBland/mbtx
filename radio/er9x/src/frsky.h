/*
 * Author - Bertrand Songis <bsongis@gmail.com>
 *
 * frsky.cpp original authors - Bryan J.Rentoul (Gruvin) <gruvin@gmail.com> and Philip Moss Adapted from jeti.cpp code by Karl
 * Szmutny <shadow@privy.de>* 
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

#ifndef FRSKY_H
#define FRSKY_H


// Mapped indices for Hub Data
#define FR_A1_COPY		0
#define FR_A2_COPY		1
#define FR_RXRSI_COPY	2
#define FR_TXRSI_COPY	3
#define FR_ALT_BARO		4
#define FR_GPS_ALT		5
#define FR_GPS_SPEED	6 
#define FR_ALT_BAROd	7
#define FR_GPS_ALTd		8
#define FR_GPS_SPEEDd	9 
#define FR_TEMP1			10
#define FR_TEMP2			11
#define FR_RPM				12
#define FR_FUEL				13
#define FR_A1_MAH			14
#define FR_A2_MAH			15
#define FR_CELL_V			16
#define FR_COURSE			17
#define FR_COURSEd		18
#define FR_GPS_DATMON	19
#define FR_GPS_YEAR		20
#define FR_GPS_HRMIN	21
#define FR_GPS_SEC		22
#define FR_GPS_LONG		23
#define FR_GPS_LONGd	24
#define FR_LONG_E_W		25
#define FR_GPS_LAT		26
#define FR_GPS_LATd		27
#define FR_LAT_N_S		28
#define FR_ACCX				29
#define FR_ACCY				30
#define FR_ACCZ				31
#define FR_CURRENT		32
// next 2 moved from 58 and 59
#define FR_V_AMP			33
#define FR_V_AMPd			34
#define FR_CELL_MIN		35
#define FR_AMP_MAH		36
#define FR_CELLS_TOT	37
#define FR_VOLTS			38
#define FR_VSPD				39
#define FR_RXV				40
#define FR_A3					41
#define FR_A4					42
#define FR_TRASH			43	// Used for invalid id

#define FR_SPORT_ALT	0xFF
#define FR_SPORT_GALT	0xFE

#define HUBDATALENGTH 44
#define HUBMINMAXLEN	7			// Items with a min and max field
//#define HUBOFFSETLEN	7			// Items with an offset field

#if defined(VARIO)
#define VARIO_QUEUE_LENGTH          5
#define VARIO_SPEED_LIMIT           10 //m/s
#define VARIO_SPEED_LIMIT_MUL       10 //to get 0.1m/s steps
#define VARIO_SPEED_LIMIT_DOWN_OFF  (100+1) //100 steps + OFF
#define VARIO_SPEED_LIMIT_UP_CENTER 10
#define VARIO_SPEED_LIMIT_UP_MAX    (10+30)

struct t_vario
{
	int16_t  VarioAltitudeQueue[VARIO_QUEUE_LENGTH]; //circular buffer
	int32_t  VarioAltitude_cm;
	uint8_t  VarioAltitudeQueuePointer;     // circular-buffer pointer
	int16_t  VarioSpeed;
} ;

#endif

/*  FrSky Hub Info
DataID Meaning       Unit   Range   Note
0x01   GPS altitude  m              Before”.”
0x02   Temperature1  °C     -30-250
0x03   RPM           BPS    0-60000
0x04   Fuel Level    %      0, 25, 50, 75, 100
0x05   Temperature2  °C     -30-250
0x06   Cell Volt     1/500v 0-4.2v, top 4 bits are cell #
0x09   GPS altitude  m              After “.”
0x10   Altitude      m      0-9999  Before “.”
0x11   GPS speed     Knots          Before “.”
0x12   Longitude     dddmm.mmmm     Before “.”
0x13   Latitude      ddmm.mmmm      Before “.”
0x14   Course        degree 0-360   Before “.”
0x15   Date/Month
0x16   Year
0x17   Hour /Minute
0x18   Second
0x19   GPS speed     Knots          After “.”
0x1A   Longitude     dddmm.mmmm     After “.”
0x1B   Latitude      ddmm.mmmm      After “.”
0x1C   Course        degree 0-360   After “.”
0x21   Altitude      m              After "."
0x22   Long - E/W
0x23   Lat. N/S
0x24   Acc-x         1/256g -8g ~ +8g
0x25   Acc-y         1/256g -8g ~ +8g
0x26   Acc-z         1/256g -8g ~ +8g
0x28   Current       1A   0-100A
0x29   VerticalSpeed
0x3A   Voltage(amp sensor) 0.5v 0-48V Before “.”
0x3B   Voltage(amp sensor)            After “.”

DataID Meaning       Unit   Range   Note
0x01   GPS altitude  m              Before”.”
0x02   Temperature1  °C     -30-250
0x03   RPM           BPS    0-60000
0x04   Fuel Level    %      0, 25, 50, 75, 100
0x05   Temperature2  °C     -30-250
0x06   Volt          1/500v 0-4.2v
0x07
0x08
0x09   GPS altitude  m              After “.”
0x0A
0x0B
0x0C
0x0D
0x0E
0x0F
0x10   Altitude      m      0-9999
0x11   GPS speed     Knots          Before “.”
0x12   Longitude     dddmm.mmmm     Before “.”
0x13   Latitude      ddmm.mmmm      Before “.”
0x14   Course        degree 0-360   Before “.”
0x15   Date/Month
0x16   Year
0x17   Hour /Minute
0x18   Second
0x19   GPS speed     Knots          After “.”
0x1A   Longitude     dddmm.mmmm     After “.”
0x1B   Latitude      ddmm.mmmm      After “.”
0x1C   Course        degree 0-360   After “.”
0x1D
0x1E
0x1F
0x20
0x21   Altitude      m              After "."
0x22   E/W
0x23   N/S
0x24   Acc-x         1/256g -8g ~ +8g
0x25   Acc-y         1/256g -8g ~ +8g
0x26   Acc-z         1/256g -8g ~ +8g
0x27
0x28   Current       1A   0-100A
0x29   VerticalSpeed
// . . .
0x3A   Voltage(amp sensor) 0.5v 0-48V Before “.”
0x3B   Voltage(amp sensor)            After “.”
  
 */

// FrSky new DATA IDs for SPORT (2 bytes)
// Receiver specific
#define RSSI_ID            0xf101
#define ADC1_ID            0xf102
#define ADC2_ID            0xf103
#define BATT_ID            0xf104
#define SWR_ID             0xf105

// MAVLINK values for IDs
//#define FR_ID_SPEED               0x0830 
//#define FR_ID_VFAS                 0x0210 
//#define FR_ID_CURRENT         0x0200 
//#define FR_ID_RPM                  0x050F      
//#define FR_ID_ALTITUDE         0x0100
//#define FR_ID_FUEL                 0x0600   
//#define FR_ID_ADC1                0xF102   
//#define FR_ID_ADC2                0xF103      
//#define FR_ID_LATLONG         0x0800
//#define FR_ID_CAP_USED       0x0600
//#define FR_ID_VARIO               0x0110
//#define FR_ID_CELLS              0x0300     
//#define FR_ID_CELLS_LAST   0x030F      
//#define FR_ID_HEADING          0x0840
//#define FR_ID_ACCX               0x0700
//#define FR_ID_ACCY               0x0710
//#define FR_ID_ACCZ               0x0720
//#define FR_ID_T1                     0x0400
//#define FR_ID_T2                     0x0410
//#define FR_ID_GPS_ALT          0x0820


// Sensors
#define ALT_FIRST_ID       0x0100
#define ALT_LAST_ID        0x010f
#define ALT_ID_8					0x10
#define VARIO_FIRST_ID     0x0110
#define VARIO_LAST_ID      0x011f
#define VARIO_ID_8				0x11
#define CURR_FIRST_ID      0x0200
#define CURR_LAST_ID       0x020f
#define CURR_ID_8					0x20
#define VFAS_FIRST_ID      0x0210
#define VFAS_LAST_ID       0x021f
#define VFAS_ID_8					0x21
#define CELLS_FIRST_ID     0x0300
#define CELLS_LAST_ID      0x030f
#define CELLS_ID_8				0x30
#define T1_FIRST_ID        0x0400
#define T1_LAST_ID         0x040f
#define T1_ID_8						0x40
#define T2_FIRST_ID        0x0410
#define T2_LAST_ID         0x041f
#define T2_ID_8						0x41
#define RPM_FIRST_ID       0x0500
#define RPM_LAST_ID        0x050f
#define RPM_ID_8					0x50
#define FUEL_FIRST_ID      0x0600
#define FUEL_LAST_ID       0x060f
#define FUEL_ID_8					0x60
#define ACCX_FIRST_ID      0x0700
#define ACCX_LAST_ID       0x070f
#define ACCX_ID_8					0x70
#define ACCY_FIRST_ID      0x0710
#define ACCY_LAST_ID       0x071f
#define ACCY_ID_8					0x71
#define ACCZ_FIRST_ID      0x0720
#define ACCZ_LAST_ID       0x072f
#define ACCZ_ID_8					0x72
#define GPS_SPEED_FIRST_ID 0x0830
#define GPS_SPEED_LAST_ID  0x083f
#define GPS_SPEED_ID_8		0x83
#define GPS_LA_LO_FIRST_ID	0x0800
#define GPS_LA_LO_LAST_ID	0x080F
#define GPS_LA_LO_ID_8		0x80
#define GPS_ALT_FIRST_ID 0x0820
#define GPS_ALT_LAST_ID  0x082f
#define GPS_ALT_ID_8		0x82
#define GPS_HDG_FIRST_ID 0x0840
#define GPS_HDG_LAST_ID  0x084f
#define GPS_HDG_ID_8			0x84
#define GPS_TIME_FIRST_ID 0x0850
#define GPS_TIME_LAST_ID  0x085f
#define GPS_TIME_ID_8			0x85

#define A3_FIRST_ID      0x0900
#define A3_LAST_ID       0x090f
#define A3_ID_8						0x90
#define A4_FIRST_ID      0x0910
#define A4_LAST_ID       0x091f
#define A4_ID_8						0x91



#define BETA_VARIO_ID      0x8030
#define BETA_VARIO_ID_8		 0x03
#define BETA_BARO_ALT_ID   0x8010
#define BETA_ALT_ID_8		   0x01


// Conversion table
// 0100		FR_ALT_BARO
// 0110   FR_VSPD
// 0200   FR_CURRENT
// 0210   FR_VOLTS
// 0300
// 0400   FR_TEMP1
// 0410   FR_TEMP2
// 0500		FR_RPM
// 0600   FR_FUEL
// 0700   FR_ACCX
// 0710   FR_ACCY
// 0720   FR_ACCZ
// 0830		FR_GPS_SPEED



#ifdef N2F
#define FRSKY_TIMEOUT10ms 			200
#define FRSKY_USR_TIMEOUT10ms 	250
#else
// .20 seconds
#define FRSKY_TIMEOUT10ms 			35
#define FRSKY_USR_TIMEOUT10ms		90
#endif

//enum AlarmLevel {
//  alarm_off = (uint8_t) 0,
//  alarm_yellow = (uint8_t)1,
//  alarm_orange = (uint8_t)2,
//  alarm_red = (uint8_t)3
//};

struct FrskyData {
  uint8_t value;
  uint8_t raw;
//  uint8_t min;
//  uint8_t max;
	uint8_t offset ;
	int16_t averaging_total ;
	uint8_t averageCount ;
  void set(uint8_t value, uint8_t copy);
	void setoffset();
};

struct Frsky_current_info
{
uint16_t Amp_hour_boundary ;
uint16_t Amp_hour_prescale ;
} ;

struct FrSky_Q_item_t
{
	uint8_t index ;
	uint16_t value ;	
} ;

struct FrSky_Q_t
{
	uint8_t in_index ;
	uint8_t out_index ;
	volatile uint8_t count ;
	struct FrSky_Q_item_t items[8] ;
} ;

extern void put_frsky_q( uint8_t index, uint16_t value ) ;
extern void process_frsky_q( void ) ;
 
//extern Frsky_current_info Frsky_current[2] ;

extern uint8_t AltitudeDecimals ;

// Global Fr-Sky telemetry data variables
extern uint8_t frskyStreaming; // >0 (true) == data is streaming in. 0 = nodata detected for some time
extern uint8_t frskyUsrStreaming; // >0 (true) == user data is streaming in. 0 = no user data detected for some time
extern uint8_t FrskyAlarmSendState;
extern FrskyData frskyTelemetry[4];
//extern FrskyData frskyRSSI[2];
extern int16_t FrskyHubData[] ;
//extern int16_t FrskyHubMin[] ;
//extern int16_t FrskyHubMax[] ;
extern uint8_t FrskyVolts[];
extern uint8_t FrskyBattCells;
extern uint8_t FrskyAlarmCheckFlag ;
//extern uint8_t MaxGpsSpeed ;
//extern int16_t MaxGpsAlt ;

void FRSKY_Init( uint8_t brate ) ;	// 0 for 9600, 1 for 57600
// static void FRSKY10mspoll(void);
void FRSKY_setTxPacket( uint8_t type, uint8_t value, uint8_t p1, uint8_t p2 ) ;
void check_frsky( void ) ;

void FRSKY_setModelAlarms(void) ;

//enum AlarmLevel FRSKY_alarmRaised(uint8_t idx, uint8_t alarm=2) ;
//enum AlarmLevel FRSKY_alarmRaised(uint8_t idx) ;
//void FRSKY_alarmPlay(uint8_t idx, uint8_t alarm) ;
void resetTelemetry();

#if defined(CPUM128) || defined(CPUM2561)
void FRSKY_disable( void ) ;
#endif

struct t_hub_max_min
{
	int16_t hubMin[HUBMINMAXLEN] ;
	int16_t hubMax[HUBMINMAXLEN] ;
} ;

extern struct t_hub_max_min FrskyHubMaxMin ;

#endif



