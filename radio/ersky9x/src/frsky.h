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
#define FR_ALT_BAROd	5
#define TELEM_GPS_ALT		6
#define TELEM_GPS_ALTd		7
#define FR_GPS_SPEED	8 
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
#define FR_VSPD				39	// Moved from 0x30 (48)
#define FR_RXV				40
#define FR_A3					41
#define FR_A4					42

/* Extra data for Mavlink via FrSky */
#define FR_BASEMODE             43
#define FR_WP_DIST              44
#define FR_HEALTH               45
#define FR_MSG                  46
#define FR_HOME_DIR             47
#define FR_HOME_DIST            48
#define FR_CPU_LOAD             49
#define FR_GPS_HDOP             50
#define FR_WP_NUM               51
#define FR_WP_BEARING           52
#define FR_VCC                  53
/* Extra data for Mavlink via FrSky */
#define FR_AIRSPEED             54

#define FR_RBOX_B1_V						55
#define FR_RBOX_B1_A						56
#define FR_RBOX_B2_V            57
#define FR_RBOX_B2_A            58
#define FR_RBOX_B1_CAP          59
#define FR_RBOX_B2_CAP          60
#define FR_RBOX_SERVO	          61
#define FR_RBOX_STATE	          62
#define FR_CELL1			          63
#define FR_CELL2			          64
#define FR_CELL3			          65
#define FR_CELL4			          66
#define FR_CELL5			          67
#define FR_CELL6			          68
#define FR_CELL7			          69
#define FR_CELL8			          70
#define FR_CELL9			          71
#define FR_CELL10			          72
#define FR_CELL11			          73
#define FR_CELL12			          74

#define FR_CUST1			          75
#define FR_CUST2			          76
#define FR_CUST3			          77
#define FR_CUST4			          78
#define FR_CUST5			          79
#define FR_CUST6			          80
#define FR_CELLS_TOTAL1			    81
#define FR_CELLS_TOTAL2			    82
#define FR_SBEC_VOLT				    83
#define FR_SBEC_CURRENT			    84

#define FR_TRASH			85  // Used for invalid id
//#define FR_TRASH			43	// Used for invalid id

#define FR_SPORT_ALT	0xFF
#define FR_SPORT_GALT	0xFE

#define HUBDATALENGTH  86
//#define HUBDATALENGTH 44
#define HUBMINMAXLEN	9
#define HUBOFFSETLEN	7			// Items with an offset field

/* Extra data for Mavlink via FrSky - MAV_MODE_FLAG */
//#define MAV_MODE_FLAG_CUSTOM_MODE_ENABLED	  1 /* 0b00000001 Reserved for future use. | */
//#define MAV_MODE_FLAG_TEST_ENABLED          2	/* 0b00000010 system has a test mode enabled. This flag is intended for temporary system tests and should not be used for stable implementations. | */
//#define MAV_MODE_FLAG_AUTO_ENABLED          4	/* 0b00000100 autonomous mode enabled, system finds its own goal positions. Guided flag can be set or not, depends on the actual implementation. | */
//#define MAV_MODE_FLAG_GUIDED_ENABLED        8	/* 0b00001000 guided mode enabled, system flies MISSIONs / mission items. | */
//#define MAV_MODE_FLAG_STABILIZE_ENABLED    16	/* 0b00010000 system stabilizes electronically its attitude (and optionally position). It needs however further control inputs to move around. | */
//#define MAV_MODE_FLAG_HIL_ENABLED          32	/* 0b00100000 hardware in the loop simulation. All motors / actuators are blocked, but internal software is full operational. | */
//#define MAV_MODE_FLAG_MANUAL_INPUT_ENABLED 64	/* 0b01000000 remote control input is enabled. | */
#define MAV_MODE_FLAG_SAFETY_ARMED			128	// 0b10000000 MAV safety set to armed. Motors are enabled / running / can start. Ready to fly.
/* Extra data for Mavlink via FrSky */

/*  FrSky Hub Info
DataID Meaning       Unit   Range   Note
0x01   GPS altitude  m              Before”.”
0x02   Temperature1  °C     -30-250
0x03   RPM           BPS    0-60000
0x04   Fuel Level
     %      0, 25, 50, 75, 100
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
#define XJT_VERSION_ID     0xf106

// Special
#define UART_FIRST_ID      0xFD00
#define UART_LAST_ID	     0xFD0F

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
#define GPS_ALT_ID_8			0x82
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
#define AIRSPEED_FIRST_ID 0x0A00
#define AIRSPEED_LAST_ID  0x0A0f
#define AIRSPEED_ID_8			0xA0

#define FUEL_QTY_FIRST_ID  0x0A10
#define FUEL_QTY_LAST_ID   0x0A1F
#define FUEL_QTY_ID_8      0xA1

#define RBOX_BATT1_FIRST_ID    0x0b00
#define RBOX_BATT1_LAST_ID     0x0b0f
#define RBOX_BATT1_ID_8        0xb0

#define RBOX_BATT2_FIRST_ID    0x0b10
#define RBOX_BATT2_LAST_ID     0x0b1f
#define RBOX_BATT2_ID_8        0xb1
#define RBOX_STATE_FIRST_ID    0x0b20
#define RBOX_STATE_LAST_ID     0x0b2f
#define RBOX_STATE_ID_8        0xb2
#define RBOX_CNSP_FIRST_ID     0x0b30
#define RBOX_CNSP_LAST_ID      0x0b3f
#define RBOX_CNSP_ID_8         0xb3

#define S6R_FIRST_ID			     0x0c30
#define S6R_LAST_ID						 0x0c3f
#define S6R_ID_8			         0xc3

#define DIY_FIRST_ID           0x5000
#define DIY_LAST_ID            0x52ff
#define DIY_ID_8	             0x00

#define DIY_STREAM_FIRST_ID       0x5000
#define DIY_STREAM_LAST_ID        0x50ff

#define FACT_TEST_ID              0xf000

#define ESC_POWER_FIRST_ID        0x0b50
#define ESC_POWER_LAST_ID         0x0b5f
#define ESC_POWER_ID_8	        	0xb5
#define ESC_RPM_CONS_FIRST_ID     0x0b60
#define ESC_RPM_CONS_LAST_ID      0x0b6f
#define ESC_RPM_ID_8	        		0xb6
#define ESC_TEMPERATURE_FIRST_ID  0x0b70
#define ESC_TEMPERATURE_LAST_ID   0x0b7f
#define ESC_TEMPERATURE_ID_8     	0xb7

#define X8R_FIRST_ID              0x0c20
#define X8R_LAST_ID               0x0c2f

#define GASSUIT_TEMP_FIRST_ID     0x0d00
#define GASSUIT_TEMP_LAST_ID      0x0d0f
#define GASSUIT_SPEED_FIRST_ID    0x0d10
#define GASSUIT_SPEED_LAST_ID     0x0d1f
#define GASSUIT_FUEL_FIRST_ID     0x0d20
#define GASSUIT_FUEL_LAST_ID      0x0d2f

#define SP2UART_A_ID              0xfd00
#define SP2UART_B_ID              0xfd01

#define SBEC_POWER_FIRST_ID       0x0e50
#define SBEC_POWER_LAST_ID        0x0e5f
#define SBEC_POWER_ID_8     			0xe5

// Craft and Theory
#define	ARDUP_ID_8				0x00
#define	ARDUP_AP_STAT_ID	0x01
#define	ARDUP_GPS_STAT_ID	0x02
#define	ARDUP_BATT_ID			0x03
#define	ARDUP_HOME_ID			0x04
#define	ARDUP_VandYAW_ID	0x05
#define	ARDUP_ATTandRNGID	0x06
#define	ARDUP_PARAM_ID		0x07


#define BETA_VARIO_ID      0x8030
#define BETA_VARIO_ID_8		 0x03
#define BETA_BARO_ALT_ID   0x8010
#define BETA_ALT_ID_8		   0x01


// Default sensor data IDs (Physical IDs)

#define DATA_ID_VARIO 0x00
#define DATA_ID_FLVSS 0x01
#define DATA_ID_FAS		0x02
#define DATA_ID_GPS 	0x03
#define DATA_ID_RPM 	0x04
#define DATA_ID_SP2UH 0x05
#define DATA_ID_SP2UR 0x06

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

#define MAV_SYS_STATUS_SENSOR_3D_GYRO			 1  /* 0x01 3D gyro | */
#define MAV_SYS_STATUS_SENSOR_3D_ACCEL			 2  /* 0x02 3D accelerometer | */
#define MAV_SYS_STATUS_SENSOR_3D_MAG			 4  /* 0x04 3D magnetometer | */
#define MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE		 8  /* 0x08 absolute pressure | */
#define MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE     16  /* 0x10 differential pressure | */
#define MAV_SYS_STATUS_SENSOR_GPS			32  /* 0x20 GPS | */
#define MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW              64  /* 0x40 optical flow | */
#define MAV_SYS_STATUS_GEOFENCE                    1048576  /* 0x100000 geofence | */
#define MAV_SYS_STATUS_AHRS                        2097152  /* 0x200000 AHRS subsystem health | */
#define MAV_SYS_ERR_GYRO     "GYRO"
#define MAV_SYS_ERR_ACCEL    "ACCEL"
#define MAV_SYS_ERR_MAG      "MAG"
#define MAV_SYS_ERR_PRESSURE "PRESS"
#define MAV_SYS_ERR_AIRSPEED "AIRSP"
#define MAV_SYS_ERR_GPS      "GPS"
#define MAV_SYS_ERR_OPTICAL  "OPTIC"
#define MAV_SYS_ERR_GEOFENCE "FENCE"
#define MAV_SYS_ERR_AHRS     "AHRS"




// .35 seconds
#define FRSKY_TIMEOUT10ms 			35
#define FRSKY_USR_TIMEOUT10ms		90

enum AlarmLevel {
  alarm_off = 0,
  alarm_yellow = 1,
  alarm_orange = 2,
  alarm_red = 3
};

//#define ALARM_GREATER(channel, alarm) ((g_model.frsky.channels[channel].alarms_greater >> alarm) & 1)
//#define ALARM_LEVEL(channel, alarm) ((g_model.frsky.channels[channel].alarms_level >> (2*alarm)) & 3)

struct FrskyData {
  uint8_t value;
  uint8_t raw;
//  uint8_t min;
//  uint8_t max;
	uint8_t offset ;
	uint16_t averaging_total ;
	uint8_t averageCount ;
  void set(uint8_t value, uint8_t copy);
	void setoffset();
};

struct t_s6r
{
	uint8_t fieldIndex ;
	uint8_t valid ;
	int16_t value ;
} ;

extern struct t_s6r S6Rdata ;

//struct Frsky_current_info
//{
//uint16_t Amp_hour_boundary ;
//uint16_t Amp_hour_prescale ;
//} ;
 
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
//extern void process_frsky_q( void ) ;

//extern Frsky_current_info Frsky_current[2] ;

extern uint8_t AltitudeDecimals ;
extern uint8_t AltitudeZeroed ;

// Global Fr-Sky telemetry data variables
extern uint8_t frskyStreaming; // >0 (true) == data is streaming in. 0 = nodata detected for some time
extern uint8_t frskyUsrStreaming; // >0 (true) == user data is streaming in. 0 = no user data detected for some time
extern uint8_t FrskyAlarmSendState;
extern FrskyData frskyTelemetry[4];
//extern FrskyData frskyRSSI[2];
extern int16_t FrskyHubData[] ;
extern uint8_t TelemetryDataValid[] ;
//extern int16_t FrskyHubMin[] ;
//extern int16_t FrskyHubMax[] ;
//extern uint16_t FrskyVolts[];
extern uint8_t FrskyBattCells[] ;
extern uint8_t FrskyAlarmCheckFlag ;
//extern uint8_t MaxGpsSpeed ;
//extern uint16_t MaxGpsAlt ;

void FRSKY_Init( uint8_t brate ) ;
void telemetry_init( uint8_t telemetryType ) ;
void FRSKY10mspoll(void);
uint32_t FRSKY_setTxPacket( uint8_t type, uint8_t value, uint8_t p1, uint8_t p2 ) ;
void check_frsky( uint32_t fivems ) ;
uint8_t decodeTelemetryType( uint8_t telemetryType ) ;

void dsmTelemetryStartReceive( void ) ;
uint16_t convertRxv( uint16_t value ) ;
void processSportData( uint8_t *packet, uint32_t receiver ) ;

void FRSKY_setModelAlarms(void) ;

enum AlarmLevel FRSKY_alarmRaised(uint8_t idx, uint8_t alarm=2) ;
void FRSKY_alarmPlay(uint8_t idx, uint8_t alarm) ;
void resetTelemetry( uint32_t item );
extern void frskyTransmitBuffer( uint32_t size ) ;
extern uint8_t FrskyTelemetryType ;
extern uint8_t JetiTxReady ;
extern uint16_t JetiTxChar ;
extern uint8_t SportStreamingStarted ;

#define TEL_ITEM_RESET_ALT		0
#define TEL_ITEM_RESET_A1OFF	1
#define TEL_ITEM_RESET_A2OFF	2
#define TEL_ITEM_RESET_GPS		3
#define TEL_ITEM_RESET_ALL		4

struct t_hub_max_min
{
	int16_t hubMin[HUBMINMAXLEN] ;
	int16_t hubMax[HUBMINMAXLEN] ;
} ;

// Values for TelemetryType
#define TEL_FRSKY_HUB		0
#define TEL_FRSKY_SPORT	1
#define TEL_JETI				2
#define TEL_MAVLINK			3
#define TEL_ARDUPILOT		4
#define TEL_DSM					5
#define TEL_ASSAN	      6
#define TEL_MULTI	      7
#define TEL_HUB_RAW			8
#define TEL_XFIRE				9
#define TEL_AFHD2SA		 10
#define TEL_HITEC			 11
#define TEL_UNKNOWN			255
extern uint8_t TelemetryType ;

// Values in EEPROM
#define TELEMETRY_UNDEFINED		0		// To detect not yet configured
#define TELEMETRY_FRSKY				1
#define TELEMETRY_WSHHI				2
#define TELEMETRY_DSM					3
#define TELEMETRY_JETI				4
#define TELEMETRY_ARDUPLANE		5
#define TELEMETRY_ARDUCOPTER	6
#define TELEMETRY_FRHUB				7
#define TELEMETRY_HUBRAW			8
#define TELEMETRY_FRMAV				9
#define TELEMETRY_MAVLINK			10
#define TELEMETRY_HITEC				11
#define TELEMETRY_AFHDS2A			12

// Values in FrskyTelemetryType
#define FRSKY_TEL_HUB		0
#define FRSKY_TEL_SPORT	1
#define FRSKY_TEL_DSM		2
#define FRSKY_TEL_AFH		3
#define FRSKY_TEL_XFIRE	4
#define FRSKY_TEL_HITEC	5


extern uint16_t DsmABLRFH[] ;
extern uint32_t LastDsmfades ;
extern uint16_t LastDsmFH[] ;

extern uint8_t RxLqi ;
extern uint8_t TxLqi ;

extern struct t_hub_max_min FrskyHubMaxMin ;

extern uint16_t A1A2toScaledValue( uint8_t channel, uint8_t *dplaces ) ;
extern uint16_t logAxScale( uint8_t channel, uint8_t *dps ) ;
extern void store_telemetry_scaler( uint8_t index, int16_t value ) ;

// Crossfire telemetry
#define XFIRE_BAUD_RATE 400000

// Device address
#define BROADCAST_ADDRESS              0x00
#define RADIO_ADDRESS                  0xEA
#define MODULE_ADDRESS                 0xEE

// Frame id
#define CRSF_GPS_ID                         0x02
#define CRSF_BATTERY_ID                     0x08
#define CRSF_LINK_ID                        0x14
#define CRSF_CHANNELS_ID                    0x16
#define CRSF_ATTITUDE_ID                    0x1E
#define CRSF_FLIGHT_MODE_ID                 0x21
#define CRSF_PING_DEVICES_ID                0x28
#define CRSF_DEVICE_INFO_ID                 0x29
#define CRSF_REQUEST_SETTINGS_ID            0x2A


// LogEnable index
#define LOG_A1		0
#define LOG_A2		1
#define LOG_RSSI	2
#define LOG_TSSI	3
#define LOG_ALT		6
#define LOG_GALT	7
#define LOG_GSPD	8
#define LOG_TEMP1	9
#define LOG_TEMP2	10
#define LOG_RPM		11
#define LOG_FUEL	12
#define LOG_MAH1	13//
#define LOG_MAH2	14//
#define LOG_CVLT	15
#define LOG_BATT	16
#define LOG_AMPS	17
#define LOG_MAH		18
#define LOG_CTOT	19
#define LOG_FASV	20
#define LOG_ACCX	21//
#define LOG_ACCY	22//
#define LOG_ACCZ	23//
#define LOG_VSPD	24
#define LOG_GVAR1	25
#define LOG_GVAR2	26
#define LOG_GVAR3	27
#define LOG_GVAR4	28
#define LOG_GVAR5	29
#define LOG_GVAR6	30
#define LOG_GVAR7	31
#define LOG_FWAT	32//
#define LOG_RXV   33
#define LOG_HDG   34
#define LOG_A3    35
#define LOG_A4    36
#define LOG_SC1		37
#define LOG_SC2		38
#define LOG_SC3		39
#define LOG_SC4		40
#define LOG_SC5		41
#define LOG_SC6		42
#define LOG_SC7		43
#define LOG_SC8		44

#define LOG_RTC		45//
#define LOG_TMOK	46//
#define LOG_ASPD	47

#define LOG_CEL1	48
#define LOG_CEL2	49
#define LOG_CEL3	50
#define LOG_CEL4	51
#define LOG_CEL5	52
#define LOG_CEL6	53

#define LOG_RBV1	54
#define LOG_RBA1  55
#define LOG_RBV2  56
#define LOG_RBA2  57
#define LOG_RBM1  58
#define LOG_RBM2  59
#define LOG_RBSV  60
#define LOG_RBST  61

#define LOG_CEL7	62
#define LOG_CEL8	63
#define LOG_CEL9	64
#define LOG_CEL10	65
#define LOG_CEL11	66
#define LOG_CEL12	67

#define LOG_CUST1	  68
#define LOG_CUST2	  69
#define LOG_CUST3	  70
#define LOG_CUST4	  71
#define LOG_CUST5	  72
#define LOG_CUST6	  73
#define LOG_CTOTAL1 74
#define LOG_CTOTAL2 75
#define LOG_SBECV		76
#define LOG_SBECA		77

#define LOG_STK_THR 100
#define LOG_STK_AIL 101
#define LOG_STK_ELE 102
#define LOG_STK_RUD 103

#define LOG_BTRX  125
#define LOG_LAT	  126
#define LOG_LONG  127




#endif



