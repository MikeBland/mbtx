/*
   @author    Nils Hцgberg
   @contact    nils.hogberg@gmail.com
    @coauthor(s):
     Victor Brutskiy, 4refr0nt@gmail.com, er9x adaptation

   Modified 2016 by Mike Blandford

   Original code from https://code.google.com/p/arducam-osd/wiki/arducam_osd

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mavlink.h"
#include <math.h>

//#include "include/mavlink/v1.0/mavlink_types.h"
//#include "include/mavlink/v1.0/ardupilotmega/mavlink.h"

//#include "../Libraries/GCS_MAVLink/GCS_MAVLink.h"
//#include "../GCS_MAVLink/include/mavlink/v1.0/mavlink_types.h"
//#include "../GCS_MAVLink/include/mavlink/v1.0/ardupilotmega/mavlink.h"

#define MAVLINK_STX 254

#define MAVLINK_MAX_PAYLOAD_LEN 255 ///< Maximum payload length
#define MAVLINK_NUM_CHECKSUM_BYTES 2

#ifndef MAVLINK_MESSAGE_LENGTHS
#define MAVLINK_MESSAGE_LENGTHS {9, 31, 12, 0, 14, 28, 3, 32, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 20, 2, 25, 23, 30, 101, 22, 26, 16, 14, 28, 32, 28, 28, 22, 22, 21, 6, 6, 37, 4, 4, 2, 2, 4, 2, 2, 3, 13, 12, 19, 17, 15, 15, 27, 25, 18, 18, 20, 20, 9, 34, 26, 46, 36, 0, 6, 4, 0, 21, 18, 0, 0, 0, 20, 0, 33, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 28, 56, 42, 33, 0, 0, 0, 0, 0, 0, 0, 26, 32, 32, 20, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 30, 18, 18, 51, 9, 0}
#endif

#ifndef MAVLINK_MESSAGE_CRCS
//                             0                                        10                               20                                            30                                              40                                           50                                          60                                          70                                 80                                90                               100                                  110                           120                           130                           140                           150                           160                166         170                           180                           190                           200                           210                           220                           230                           240                              250                 255
#define MAVLINK_MESSAGE_CRCS {50, 124, 137, 0, 237, 217, 104, 119, 0, 0, 0, 89, 0, 0, 0, 0, 0, 0, 0, 0, 214, 159, 220, 168, 24, 23, 170, 144, 67, 115, 39, 246, 185, 104, 237, 244, 222, 212, 9, 254, 230, 28, 28, 132, 221, 232, 11, 153, 41, 39, 214, 223, 141, 33, 15, 3, 100, 24, 239, 238, 30, 240, 183, 130, 130, 0, 148, 21, 0, 52, 124, 0, 0, 0, 20, 0, 152, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 231, 183, 63, 54, 0, 0, 0, 0, 0, 0, 0, 175, 102, 158, 208, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 204, 49, 170, 44, 83, 46, 0}
#endif

#define _MAV_PAYLOAD(msg) ((const char *)(&((msg)->payload64[0])))
#define _MAV_PAYLOAD_NON_CONST(msg) ((char *)(&((msg)->payload64[0])))
#define _MAV_RETURN_uint8_t(msg, wire_offset) (const uint8_t)_MAV_PAYLOAD(msg)[wire_offset]
#define _MAV_RETURN_int8_t(msg, wire_offset) (const int8_t)_MAV_PAYLOAD(msg)[wire_offset]
#define _MAV_RETURN_uint16_t(msg, wire_offset) *((uint16_t *)&_MAV_PAYLOAD(msg)[wire_offset])
#define _MAV_RETURN_uint32_t(msg, wire_offset) *((uint32_t *)&_MAV_PAYLOAD(msg)[wire_offset])
#define _MAV_RETURN_int16_t(msg, wire_offset) *((int16_t *)&_MAV_PAYLOAD(msg)[wire_offset])
#define _MAV_RETURN_float(msg, wire_offset) *((float *)&_MAV_PAYLOAD(msg)[wire_offset])
#define _MAV_RETURN_int32_t(msg, wire_offset) *((int32_t *)&_MAV_PAYLOAD(msg)[wire_offset])

#define MAVLINK_MSG_ID_HEARTBEAT 0
#define MAVLINK_MSG_ID_SYS_STATUS 1
#define MAVLINK_MSG_ID_RC_CHANNELS_RAW 35
#define MAVLINK_MSG_ID_GPS_RAW_INT 24
#define MAVLINK_MSG_ID_VFR_HUD 74
#define MAVLINK_MSG_ID_HWSTATUS 165
#define MAVLINK_MSG_ID_RADIO 166


#if 0

MAVLINK_HELPER uint16_t mavlink_finalize_message_chan(mavlink_message_t* msg, uint8_t system_id, uint8_t component_id, 
						      uint8_t chan, uint8_t length, uint8_t crc_extra)
{
	// This code part is the same for all messages;
	uint16_t checksum;
	msg->magic = MAVLINK_STX;
	msg->len = length;
	msg->sysid = system_id;
	msg->compid = component_id;
	// One sequence number per component
	msg->seq = mavlink_get_channel_status(chan)->current_tx_seq;
	mavlink_get_channel_status(chan)->current_tx_seq = mavlink_get_channel_status(chan)->current_tx_seq+1;
	checksum = crc_calculate((uint8_t*)&msg->len, length + MAVLINK_CORE_HEADER_LEN);
	crc_accumulate(crc_extra, &checksum);
	mavlink_ck_a(msg) = (uint8_t)(checksum & 0xFF);
	mavlink_ck_b(msg) = (uint8_t)(checksum >> 8);

	return length + MAVLINK_NUM_NON_PAYLOAD_BYTES;
}

static inline void mavlink_msg_request_data_stream_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint8_t req_stream_id, uint16_t req_message_rate, uint8_t start_stop)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[6];
	_mav_put_uint16_t(buf, 0, req_message_rate);
	_mav_put_uint8_t(buf, 2, target_system);
	_mav_put_uint8_t(buf, 3, target_component);
	_mav_put_uint8_t(buf, 4, req_stream_id);
	_mav_put_uint8_t(buf, 5, start_stop);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REQUEST_DATA_STREAM, buf, 6, 148);
#else
	mavlink_request_data_stream_t packet;
	packet.req_message_rate = req_message_rate;
	packet.target_system = target_system;
	packet.target_component = target_component;
	packet.req_stream_id = req_stream_id;
	packet.start_stop = start_stop;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_REQUEST_DATA_STREAM, (const char *)&packet, 6, 148);
#endif
}

#endif


typedef struct __mavlink_message
{
	uint16_t checksum; /// sent at end of packet
	uint8_t magic;   ///< protocol magic marker
	uint8_t len;     ///< Length of payload
	uint8_t seq;     ///< Sequence of packet
	uint8_t sysid;   ///< ID of message sender system/aircraft
	uint8_t compid;  ///< ID of the message sender component
	uint8_t msgid;   ///< ID of message in payload
	uint64_t payload64[(MAVLINK_MAX_PAYLOAD_LEN+MAVLINK_NUM_CHECKSUM_BYTES+7)/8];
} mavlink_message_t ;

typedef enum {
    MAVLINK_PARSE_STATE_UNINIT=0,
    MAVLINK_PARSE_STATE_IDLE,
    MAVLINK_PARSE_STATE_GOT_STX,
    MAVLINK_PARSE_STATE_GOT_SEQ,
    MAVLINK_PARSE_STATE_GOT_LENGTH,
    MAVLINK_PARSE_STATE_GOT_SYSID,
    MAVLINK_PARSE_STATE_GOT_COMPID,
    MAVLINK_PARSE_STATE_GOT_MSGID,
    MAVLINK_PARSE_STATE_GOT_PAYLOAD,
    MAVLINK_PARSE_STATE_GOT_CRC1
} mavlink_parse_state_t; ///< The state machine for the comm parser

typedef struct __mavlink_status
{
    uint8_t msg_received;               ///< Number of received messages
    uint8_t buffer_overrun;             ///< Number of buffer overruns
    uint8_t parse_error;                ///< Number of parse errors
    mavlink_parse_state_t parse_state;  ///< Parsing state machine
    uint8_t packet_idx;                 ///< Index in current packet
    uint8_t current_rx_seq;             ///< Sequence number of last packet received
    uint8_t current_tx_seq;             ///< Sequence number of last packet sent
    uint16_t packet_rx_success_count;   ///< Received packets
    uint16_t packet_rx_drop_count;      ///< Number of packet drops
} mavlink_status_t ;

enum MAV_DATA_STREAM
{
	MAV_DATA_STREAM_ALL=0, /* Enable all data streams | */
	MAV_DATA_STREAM_RAW_SENSORS=1, /* Enable IMU_RAW, GPS_RAW, GPS_STATUS packets. | */
	MAV_DATA_STREAM_EXTENDED_STATUS=2, /* Enable GPS_STATUS, CONTROL_STATUS, AUX_STATUS | */
	MAV_DATA_STREAM_RC_CHANNELS=3, /* Enable RC_CHANNELS_SCALED, RC_CHANNELS_RAW, SERVO_OUTPUT_RAW | */
	MAV_DATA_STREAM_RAW_CONTROLLER=4, /* Enable ATTITUDE_CONTROLLER_OUTPUT, POSITION_CONTROLLER_OUTPUT, NAV_CONTROLLER_OUTPUT. | */
	MAV_DATA_STREAM_POSITION=6, /* Enable LOCAL_POSITION, GLOBAL_POSITION/GLOBAL_POSITION_INT messages. | */
	MAV_DATA_STREAM_EXTRA1=10, /* Dependent on the autopilot | */
	MAV_DATA_STREAM_EXTRA2=11, /* Dependent on the autopilot | */
	MAV_DATA_STREAM_EXTRA3=12, /* Dependent on the autopilot | */
	MAV_DATA_STREAM_ENUM_END=13, /*  | */
};

mavlink_message_t MavlinkBuffer ;
mavlink_status_t MavlinkStatus ;

//bool Motor_armed ;
uint8_t HeartCounter ;


extern void storeTelemetryData( uint8_t index, uint16_t value ) ;
extern void storeRSSI( uint8_t value ) ;

//extern "C" uint32_t millis( void ) ;

//typedef char prog_char __attribute__((__progmem__));//,deprecated("prog_char type is deprecated.")));
//#define APM __attribute__(( section(".progmem.data") ))

// check actual data at https://github.com/diydrones/ardupilot/search?q=SEVERITY_HIGH
//const char MSG01[] = "ARMING MOTORS"; 
//const char MSG02[] = "PreArm: RC not calibrated"; 
//const char MSG03[] = "PreArm: Baro not healthy";
//const char MSG04[] = "PreArm: Compass not healthy";
//const char MSG05[] = "PreArm: Bad GPS Pos";
//const char MSG06[] = "compass disabled";
//const char MSG07[] = "check compass";
//const char MSG08[] = "Motor Test: RC not calibrated";
//const char MSG09[] = "Motor Test: vehicle not landed";
//const char MSG10[] = "Crash: Disarming";
//const char MSG11[] = "Parachute: Released";
//const char MSG12[] = "error setting rally point";
//const char MSG13[] = "AutoTune: Started";
//const char MSG14[] = "AutoTune: Stopped";
//const char MSG15[] = "Trim saved";
//const char MSG16[] = "EKF variance";
//const char MSG17[] = "verify_nav: invalid or no current nav";
//const char MSG18[] = "verify_conditon: invalid or no current condition";
//const char MSG19[] = "Disable fence failed (autodisable)";
//const char MSG20[] = "ESC Cal: restart board";
//const char MSG21[] = "ESC Cal: passing pilot thr to ESCs";
//const char MSG22[] = "Low Battery";
//const char MSG23[] = "Lost GPS";
//const char MSG24[] = "Fence disabled (autodisable)";
//const char *const MsgPointers[24] = {MSG01, MSG02, MSG03, MSG04, MSG05, MSG06, MSG07, MSG08, MSG09, MSG10, MSG11, MSG12, MSG13, MSG14, MSG15, MSG16, MSG17, MSG18, MSG19, MSG20, MSG21, MSG22, MSG23, MSG24};
//#define MAX_MSG 24

#define X25_INIT_CRC 0xffff
#define X25_VALIDATE_CRC 0xf0b8

/**
 * @brief Accumulate the X.25 CRC by adding one char at a time.
 *
 * The checksum function adds the hash of one char at a time to the
 * 16 bit checksum (uint16_t).
 *
 * @param data new char to hash
 * @param crcAccum the already accumulated checksum
 **/
static inline void crc_accumulate(uint8_t data, uint16_t *crcAccum)
{
	/*Accumulate one byte of data into the CRC*/
	uint8_t tmp;

	tmp = data ^ (uint8_t)(*crcAccum &0xff);
	tmp ^= (tmp<<4);
	*crcAccum = (*crcAccum>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);
}

/**
 * @brief Initiliaze the buffer for the X.25 CRC
 *
 * @param crcAccum the 16 bit X.25 CRC
 */
static inline void crc_init(uint16_t* crcAccum)
{
	*crcAccum = X25_INIT_CRC;
}

/**
 * @brief Calculates the X.25 checksum on a byte buffer
 *
 * @param  pBuffer buffer containing the byte array to hash
 * @param  length  length of the byte array
 * @return the checksum over the buffer bytes
 **/
static inline uint16_t crc_calculate(const uint8_t* pBuffer, uint16_t length)
{
	uint16_t crcTmp ;
	crc_init(&crcTmp) ;
	while (length--)
	{
		crc_accumulate(*pBuffer++, &crcTmp) ;
	}
  return crcTmp ;
}

/**
 * @brief Accumulate the X.25 CRC by adding an array of bytes
 *
 * The checksum function adds the hash of one char at a time to the
 * 16 bit checksum (uint16_t).
 *
 * @param data new bytes to hash
 * @param crcAccum the already accumulated checksum
 **/
static inline void crc_accumulate_buffer(uint16_t *crcAccum, const char *pBuffer, uint8_t length)
{
	const uint8_t *p = (const uint8_t *)pBuffer;
	while (length--)
	{
    crc_accumulate(*p++, crcAccum);
  }
}

static inline void mavlink_start_checksum(mavlink_message_t* msg)
{
	crc_init(&msg->checksum);
}

static inline void mavlink_update_checksum(mavlink_message_t* msg, uint8_t c)
{
	crc_accumulate(c, &msg->checksum);
}

static inline bool getBit( uint8_t value, uint8_t whichBit)
{
	return value & (1 << whichBit) ? 1 : 0;
}

//Mavlink::Mavlink(BetterStream* port)
//Mavlink::Mavlink()
//{
////   mavlink_comm_0_port = port;
//   mavlink_system.sysid = 12;
//   mavlink_system.compid = 1;
//   mavlink_system.type = 0;
//   mavlink_system.state = 0;

//   reset() ;
//   enable_mav_request = 1;
//   msg_timer = 0;

////   lastMAVBeat = 0;
////   batteryVoltage = 0;
////   current = 0;
////   batteryRemaining = 0;
////   gpsStatus = 0;
//   latitude = 0;
//   longitude = 0;
   
////	 gpsAltitude = 0;
////   gpsHdop = 999;
////   numberOfSatelites = 0;
////   gpsGroundSpeed = 0;
////   gpsCourse = 0;
////   altitude = 0.0f;
////   apmMode = 99;
//   course = 0;
//   home_course = 0;
////   throttle = 0;
////   accX = 0;
////   accY = 0;
////   accZ = 0;
//   apmBaseMode = 0;
////   wp_dist = 0;
////   wp_num = 0;
////   wp_bearing = 270;
////   sensors_health = 0;
////   last_message_severity = 0;
//   home_gps_alt = 0;
////   alt = 0;
////   gps_alt = 0;
//   home_distance = 0;
//   home_direction = 90;
////   lat = 0;
////   lon = 0;
//   cpu_load = 0;
//   cpu_vcc = 0;
////   status_msg = 0;
//   motor_armed = 0;
//	 msgCounter = 0 ;
   
//}

//Mavlink::~Mavlink(void)
//{
//}

//const float Mavlink::getMainBatteryVoltage()
//{
//   return batteryVoltage;
//}

//const float Mavlink::getBatteryCurrent()
//{
//   return current;
//}

//const int Mavlink::getFuelLevel()
//{
//   return batteryRemaining;
//}

//const int Mavlink::getGpsStatus()
//{
//   return gpsStatus;
//}

//long Mavlink::getLatitude()
//{
////   return gpsDdToDmsFormat(latitude / 10000000.0f);
////	 return latitude / 10000000.0f;
//   return latitude ;
//}

//long Mavlink::getLongitud()
//{
////   return gpsDdToDmsFormat(longitude / 10000000.0f);
////   return longitude / 10000000.0f;
//   return longitude ;
//}

//const float Mavlink::getGpsAltitude()
//{
//   setHomeVars();
//   return gps_alt/1000; //gpsAltitude / 1000;
//}

//const int Mavlink::getGpsHdop()
//{
//   if ( gpsHdop > 999 ) gpsHdop = 999;
//   return gpsHdop;
//}

//const int Mavlink::getTemp2()
//{   
//   return numberOfSatelites * 10 + gpsStatus;
//}

//const float Mavlink::getGpsGroundSpeed()
//{
//   return gpsGroundSpeed * 1000 ; //* 0.0194384f;
//}

//const float Mavlink::getAirspeed()
//{
//   return airspeed * 10 ;
//}

//const float Mavlink::getAltitude()
//{
//   return altitude;
//}

//const int Mavlink::getTemp1()
//{
//   return apmMode | ( apm_mav_type << 8 ) ;
//}

//const bool Mavlink::isArmed()
//{
//    return motor_armed;
//}
//const float Mavlink::getCourse()
//{
//   int new_course; 
//   if ( isArmed() ){
//      new_course = course - home_course;
//	} else {
//	  new_course = course;
//	  home_course = course;
//	}
//   if (new_course < 0) { 
//	    new_course += 360;
//	}
//    return new_course; // without normalization
//}

//const int Mavlink::getEngineSpeed()
//{
//   return throttle;
//}

/*
const float Mavlink::getAccX()
{
   return accX;
}
   
const float Mavlink::getAccY()
{
   return accY;
}

const float Mavlink::getAccZ()
{
   return accZ;
}

const int Mavlink::getYear()
{
   return 0;
}

const int Mavlink::getTime()
{
   return 0;
}

const int Mavlink::getDate()
{
   return 0;
}
*/
//const int Mavlink::getBaseMode()
//{
//   return apmBaseMode;
//}
//const int Mavlink::getWP_dist()
//{
//   return wp_dist;
//}
//const int Mavlink::getWP_num()
//{
//   return wp_num;
//}
//const int Mavlink::getWP_bearing()
//{
//   int new_bearing = wp_bearing + 270;
//   if (new_bearing >= 360) {
//	  new_bearing -= 360;
//   }
//   return new_bearing; // normalized value
//}
//const int Mavlink::getHealth()
//{
//   return sensors_health;
//}
//const int Mavlink::getHome_dir()
//{
//   setHomeVars();
//   return home_direction; // normalized value
//}
//const int Mavlink::getHome_dist()
//{
//   setHomeVars();
//   return home_distance;
//}
//const int Mavlink::getCpu_load()
//{
//   return cpu_load;
//}
//const int Mavlink::getVcc()
//{
//   return cpu_vcc;
//}
//const int Mavlink::getStatus_msg()
//{
//	if ( msg_timer <= 0 ){ 
//        status_msg = 0;
//	}
//    return status_msg;
//}
//const int Mavlink::parse_msg()
//{
//	int i = 0;
//	bool success = false;
//  while ( (i <= MAX_MSG) && !success )
//	{
//    if ( compare_msg((char*)pgm_read_word(&(MsgPointers[i]))) )
//		{
//	     success = true;
//	  }
//	  i++;
//	}
//	if (success)
//	{
//       return i;
//	}
//	else
//	{
//       return 99 ;
//	}
//}
//const bool Mavlink::compare_msg(char *string)
//{
//    bool eq = true;
//	int i = 0;
//  while (pgm_read_byte(string)!='\0' && eq)
//	{
//	   if ( pgm_read_byte(string) == last_message_text[i] )
//	   {
//	      // do nothing
//	   } else {
//	     eq = false;
//	   }
//	   string++;
//	   i++;
//	}
//	return eq;
//}
//const int Mavlink::getNCell()
//{
//   return ncell;
//}
//const int Mavlink::getCell()
//{
//   return cell;
//}

// We receive the GPS coordinates in ddd.dddd format
// FrSky wants the dd mm.mmm format so convert.
float gpsDdToDmsFormat(float ddm)
{
   int deg = (int)ddm;
   float min_dec = (ddm - deg) * 60.0f;
   float sec = (min_dec - (int)min_dec) * 60.0f;

   return (float)deg * 100.0f + (int)min_dec + sec / 100.0f;
}

//void makeRateRequest()
//{
//  const int  maxStreams = 7;
//  const unsigned short MAVStreams[maxStreams] =
//	{
//      MAV_DATA_STREAM_RAW_SENSORS,
//      MAV_DATA_STREAM_EXTENDED_STATUS,
//      MAV_DATA_STREAM_RC_CHANNELS,
//      MAV_DATA_STREAM_POSITION,
//      MAV_DATA_STREAM_EXTRA1, 
//      MAV_DATA_STREAM_EXTRA2,
//      MAV_DATA_STREAM_EXTRA3
//  };
    
//	const unsigned int MAVRates[maxStreams] = {0x02, 0x02, 0x02, 0x02, 0x05, 0x05, 0x02};
//  for (int i=0; i < maxStreams; i++)
//	{
//    mavlink_msg_request_data_stream_send(MAVLINK_COMM_0,
//      apm_mav_system, apm_mav_component, MAVStreams[i], MAVRates[i], 1);
//  }
//}

uint8_t mavlink_parse_char( uint8_t c, mavlink_message_t* r_message, mavlink_status_t* r_mavlink_status)
{
//	  default message crc function. You can override this per-system to
//	  put this data in a different memory segment

//#if MAVLINK_CRC_EXTRA
#ifndef MAVLINK_MESSAGE_CRC
	static const uint8_t mavlink_message_crcs[256] = MAVLINK_MESSAGE_CRCS ;
#define MAVLINK_MESSAGE_CRC(msgid) mavlink_message_crcs[msgid]
#endif
//#endif

	mavlink_message_t* rxmsg = &MavlinkBuffer ; ///< The currently decoded message
	mavlink_status_t* status = &MavlinkStatus ; ///< The current decode status
	int bufferIndex = 0;

	status->msg_received = 0;

	switch (status->parse_state)
	{
		case MAVLINK_PARSE_STATE_UNINIT:
		case MAVLINK_PARSE_STATE_IDLE:
			if (c == MAVLINK_STX)
			{
				status->parse_state = MAVLINK_PARSE_STATE_GOT_STX;
				rxmsg->len = 0;
				rxmsg->magic = c;
				mavlink_start_checksum(rxmsg);
			}
		break ;

		case MAVLINK_PARSE_STATE_GOT_STX:
			if (status->msg_received 
/* Support shorter buffers than the
   default maximum packet size */
#if (MAVLINK_MAX_PAYLOAD_LEN < 255)
				|| c > MAVLINK_MAX_PAYLOAD_LEN
#endif
				)
			{
				status->buffer_overrun++;
				status->parse_error++;
				status->msg_received = 0;
				status->parse_state = MAVLINK_PARSE_STATE_IDLE;
			}
			else
			{
				// NOT counting STX, LENGTH, SEQ, SYSID, COMPID, MSGID, CRC1 and CRC2
				rxmsg->len = c;
				status->packet_idx = 0;
				mavlink_update_checksum(rxmsg, c);
				status->parse_state = MAVLINK_PARSE_STATE_GOT_LENGTH;
			}
		break;

		case MAVLINK_PARSE_STATE_GOT_LENGTH:
			rxmsg->seq = c;
			mavlink_update_checksum(rxmsg, c);
			status->parse_state = MAVLINK_PARSE_STATE_GOT_SEQ;
		break;

		case MAVLINK_PARSE_STATE_GOT_SEQ:
			rxmsg->sysid = c;
			mavlink_update_checksum(rxmsg, c);
			status->parse_state = MAVLINK_PARSE_STATE_GOT_SYSID;
		break;

		case MAVLINK_PARSE_STATE_GOT_SYSID:
			rxmsg->compid = c;
			mavlink_update_checksum(rxmsg, c);
			status->parse_state = MAVLINK_PARSE_STATE_GOT_COMPID;
		break;

		case MAVLINK_PARSE_STATE_GOT_COMPID:
			rxmsg->msgid = c;
			mavlink_update_checksum(rxmsg, c);
			if (rxmsg->len == 0)
			{
				status->parse_state = MAVLINK_PARSE_STATE_GOT_PAYLOAD;
			}
			else
			{
				status->parse_state = MAVLINK_PARSE_STATE_GOT_MSGID;
			}
		break;

		case MAVLINK_PARSE_STATE_GOT_MSGID:
			
			_MAV_PAYLOAD_NON_CONST(rxmsg)[status->packet_idx++] = (char)c;
			mavlink_update_checksum(rxmsg, c);
			if (status->packet_idx == rxmsg->len)
			{
				status->parse_state = MAVLINK_PARSE_STATE_GOT_PAYLOAD;
			}
		break;

		case MAVLINK_PARSE_STATE_GOT_PAYLOAD:
			mavlink_update_checksum(rxmsg, MAVLINK_MESSAGE_CRC(rxmsg->msgid));
			if (c != (rxmsg->checksum & 0xFF))
			{
				status->parse_error++;
				status->msg_received = 0;
				status->parse_state = MAVLINK_PARSE_STATE_IDLE;
				if (c == MAVLINK_STX)
				{
					status->parse_state = MAVLINK_PARSE_STATE_GOT_STX;
					rxmsg->len = 0;
					mavlink_start_checksum(rxmsg);
				}
			}
			else
			{
				status->parse_state = MAVLINK_PARSE_STATE_GOT_CRC1;
				_MAV_PAYLOAD_NON_CONST(rxmsg)[status->packet_idx] = (char)c;
			}
		break;

		case MAVLINK_PARSE_STATE_GOT_CRC1:
			if (c != (rxmsg->checksum >> 8))
			{
				status->parse_error++;
				status->msg_received = 0;
				status->parse_state = MAVLINK_PARSE_STATE_IDLE;
				if (c == MAVLINK_STX)
				{
					status->parse_state = MAVLINK_PARSE_STATE_GOT_STX;
					rxmsg->len = 0;
					mavlink_start_checksum(rxmsg);
				}
			}
			else
			{
				status->msg_received = 1;
				status->parse_state = MAVLINK_PARSE_STATE_IDLE;
				_MAV_PAYLOAD_NON_CONST(rxmsg)[status->packet_idx+1] = (char)c;
				memcpy(r_message, rxmsg, sizeof(mavlink_message_t));
			}
		break;
	}

	bufferIndex++;
	// If a message has been sucessfully decoded, check index
	if (status->msg_received == 1)
	{
		//while(status->current_seq != rxmsg->seq)
		//{
		//	status->packet_rx_drop_count++;
		//               status->current_seq++;
		//}
		status->current_rx_seq = rxmsg->seq;
		// Initial condition: If no packet has been received so far, drop count is undefined
		if (status->packet_rx_success_count == 0) status->packet_rx_drop_count = 0;
		// Count this packet as received
		status->packet_rx_success_count++;
	}

	r_mavlink_status->current_rx_seq = status->current_rx_seq+1;
	r_mavlink_status->packet_rx_success_count = status->packet_rx_success_count;
	r_mavlink_status->packet_rx_drop_count = status->parse_error;
	status->parse_error = 0;
	
	
	return status->msg_received;
}

mavlink_message_t Msg ;
mavlink_status_t Mstatus ;

static inline uint8_t mavlink_msg_heartbeat_get_type(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  4);
}

static inline uint32_t mavlink_msg_heartbeat_get_custom_mode(const mavlink_message_t* msg)
{
//	return _MAV_RETURN_uint32_t(msg,  0);
	uint32_t *p = (uint32_t *)((msg)->payload64) ;
	return *p ;
}

static inline uint8_t mavlink_msg_heartbeat_get_base_mode(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  6);
}

static inline int16_t mavlink_msg_sys_status_get_current_battery(const mavlink_message_t* msg)
{
//	return _MAV_RETURN_int16_t(msg,  16);
	int16_t *p = (int16_t *)((msg)->payload64) ;
	return p[8] ;
}

static inline float mavlink_msg_vfr_hud_get_alt(const mavlink_message_t* msg)
{
//	return _MAV_RETURN_float(msg,  8);
	float *p = (float *)((msg)->payload64) ;
	return p[2] ;
}

static inline uint16_t mavlink_msg_hwstatus_get_Vcc(const mavlink_message_t* msg)
{
//	return _MAV_RETURN_uint16_t(msg,  0);
	uint16_t *p = (uint16_t *)((msg)->payload64) ;
	return p[0] ;
}

static inline uint16_t mavlink_msg_sys_status_get_load(const mavlink_message_t* msg)
{
//	return _MAV_RETURN_uint16_t(msg,  12);
	uint16_t *p = (uint16_t *)((msg)->payload64) ;
	return p[6] ;
}

static inline uint16_t mavlink_msg_sys_status_get_voltage_battery(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  14);
}

static inline int16_t mavlink_msg_vfr_hud_get_heading(const mavlink_message_t* msg)
{
//	return _MAV_RETURN_int16_t(msg,  16);
	int16_t *p = (int16_t *)((msg)->payload64) ;
	return p[8] ;
}

static inline float mavlink_msg_vfr_hud_get_climb(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  12);
}

static inline int8_t mavlink_msg_sys_status_get_battery_remaining(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int8_t(msg,  30);
}

static inline int32_t mavlink_msg_gps_raw_int_get_lat(const mavlink_message_t* msg)
{
//	return _MAV_RETURN_int32_t(msg,  8);
	int32_t *p = (int32_t *)((msg)->payload64) ;
	return p[2] ;
}

static inline int32_t mavlink_msg_gps_raw_int_get_lon(const mavlink_message_t* msg)
{
//	return _MAV_RETURN_int32_t(msg,  12);
	int32_t *p = (int32_t *)((msg)->payload64) ;
	return p[3] ;
}

static inline int32_t mavlink_msg_gps_raw_int_get_alt(const mavlink_message_t* msg)
{
//	return _MAV_RETURN_int32_t(msg,  16);
	int32_t *p = (int32_t *)((msg)->payload64) ;
	return p[4] ;
}
static inline uint16_t mavlink_msg_gps_raw_int_get_epv(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  22);
}

static inline uint16_t mavlink_msg_gps_raw_int_get_eph(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  20);
}

static inline uint8_t mavlink_msg_gps_raw_int_get_satellites_visible(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  29);
}

static inline uint8_t mavlink_msg_gps_raw_int_get_fix_type(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  28);
}

static inline uint8_t mavlink_msg_radio_get_rssi(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  4);
}

static inline uint8_t mavlink_msg_rc_channels_raw_get_rssi(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  21);
}


		//				value = ctelemetry.getTemp1() ;
		//				id = TEMP1 ;
		//				value = ctelemetry.getAltitude() * 100.0f ;
		//				id = ALT_FIRST_ID ;
//				value = ctelemetry.getCourse() * 100.0f ;
//				id = GPS_COURS_FIRST_ID ;
//				value = ctelemetry.getMainBatteryVoltage() * 100.0f ;
//					id = VFAS_FIRST_ID ;
		//				value = ctelemetry.getBaseMode();
		//				id = BASEMODE ;
//				value = ctelemetry.getTemp2() ;
//				id = TEMP2 ;
		//				value = ctelemetry.getBatteryCurrent() ;
		//				id = CURR_FIRST_ID ;

//				value = ctelemetry.getEngineSpeed() ;
//				id = RPM_FIRST_ID ;
		//				value = ctelemetry.getVcc() ;
		//				id = VCC ;
//				value = ctelemetry.getCpu_load();
//				id = CPU_LOAD ;
//				value = ctelemetry.getHome_dir() ;
//				id = HOME_DIR ;
//				value = ctelemetry.getWP_bearing() ;
//				id = WP_BEARING ;

//					long gpsLatitude = ctelemetry.getLatitude() ;
//					value = 0 ;
//					if (gpsLatitude < 0)
//					{
//						value |= 0x40000000 ;
//						gpsLatitude = - gpsLatitude ;
//					}
//					uint32_t temp = gpsLatitude  * 6 ;
//					temp /= 100 ;
//					temp &= 0x3FFFFFFF ;
//					value |= temp ;
//					id = GPS_LONG_LATI_FIRST_ID ;

//					long gpsLongitude = ctelemetry.getLongitud() ;
//					value = 0x80000000 ;
//					if (gpsLongitude < 0)
//					{
//						value |= 0x40000000 ;
//						gpsLongitude = - gpsLongitude ;
//					}
//					uint32_t temp = gpsLongitude * 6 ;
//					temp /= 100 ;
//					temp &= 0x3FFFFFFF ;
//					value |= temp ;
//					id = GPS_LONG_LATI_FIRST_ID ;

//				value = ctelemetry.getFuelLevel() ;
//				id = FUEL ;
//				value = ctelemetry.getGpsAltitude() ;
//				id = GPS_ALT_FIRST_ID ;
//				value = ctelemetry.getGpsHdop() ;
//				id = GPS_HDOP ;

//						value = ctelemetry.getWP_dist() ;
//						id = WP_DIST ;
//						value = ctelemetry.getWP_num() ;
//						id = WP_NUM ;
//						value = ctelemetry.getWP_bearing() ;
//						id = WP_BEARING ;
//						value = ctelemetry.getHome_dist() ;
//						id = HOME_DIST ;
//						value = ctelemetry.getStatus_msg() ;
//						id = STATUS_MSG ;
//						value = ctelemetry.getHealth() ;
//						id = HEALTH ;

//						value = ctelemetry.getGpsGroundSpeed(); // / 1.84f;
//						id = GPS_SPEED_FIRST_ID ;
//						value = ctelemetry.getAirspeed();
//						id = AIR_SPEED_FIRST_ID ;

//					uint8_t nCells = ctelemetry.getNCell() ;
//					uint32_t cval = ctelemetry.getCell() ;
//					value = ( cval << 20 ) | ( cval << 8 ) | ( nCells << 4 ) ;
//					id = CELLS_FIRST_ID ;
//					value |= sportCellIndex ;
//					if ( sportCellIndex > nCells )
//					{
//						id = 0xFFFF ;
//					}
//					sportCellIndex += 2 ;
//					if ( sportCellIndex > 4 )
//					{
//						sportCellIndex = 0 ;
//					}


//				value = ctelemetry.getVspd() * 100.0f ;
//				id = VARIO_FIRST_ID ;

//extern uint8_t frskyUsrStreaming; // >0 (true) == user data is streaming in. 0 = no user data detected for some time

void mavlinkReceive( uint8_t data )
{
//  unsigned long sensors_enabled;
//  unsigned long health;

    //trying to grab msg  
	if(mavlink_parse_char( data, &Msg, &Mstatus)) 
	{
 //		msgCounter += 1 ;
		frskyUsrStreaming = FRSKY_USR_TIMEOUT10ms ;
		
    switch(Msg.msgid)
		{
      case MAVLINK_MSG_ID_HEARTBEAT:
      {
				uint16_t value1 ;
				uint16_t value2 ;
				if ( HeartCounter < 5 )
				{
					HeartCounter += 1 ;
				}
        
//				apm_mav_system    = Msg.sysid;
//        apm_mav_component = Msg.compid;
        
				value1 = mavlink_msg_heartbeat_get_type(&Msg) ; // apm_mav_type
        value2 = (unsigned int)mavlink_msg_heartbeat_get_custom_mode(&Msg) ; // apmMode
				storeTelemetryData( FR_TEMP1, value2 | ( value1 << 8 ) ) ;

				value1 = mavlink_msg_heartbeat_get_base_mode(&Msg);
//				apmBaseMode       = mavlink_msg_heartbeat_get_base_mode(&Msg);
				storeTelemetryData( FR_BASEMODE, value1 ) ;
//        if (getBit( value1, MOTORS_ARMED))
//				{
//          Motor_armed = 1 ;
//        }
//				else
//				{
//          Motor_armed = 0 ;
//        }
//        return MAVLINK_MSG_ID_HEARTBEAT;
      }
      break ;

			case MAVLINK_MSG_ID_SYS_STATUS:
      {
//				float batteryVoltage = (mavlink_msg_sys_status_get_voltage_battery(&Msg) / 100.0f) * 0.5238f ;
				float batteryVoltage = (mavlink_msg_sys_status_get_voltage_battery(&Msg) / 100.0f) ;
				storeTelemetryData( FR_VOLTS, batteryVoltage ) ;
        
//				batteryVoltage = (mavlink_msg_sys_status_get_voltage_battery(&Msg) / 1000.0f); // Volts, Battery voltage, in millivolts (1 = 1 millivolt)
//        ncell = batteryVoltage / 43f ;
//        ncell++ ;
				
//				if (ncell > 5) ncell = 5;
//        cell = 50 * (batteryVoltage / float(ncell));
//				if (cell < 1000) cell = 0;

////				current          = mavlink_msg_sys_status_get_current_battery(&Msg) / 10; //0.1A Battery current, in 10*milliamperes (1 = 10 milliampere)         
				storeTelemetryData( FR_CURRENT, mavlink_msg_sys_status_get_current_battery(&Msg) / 10 ) ;
////				batteryRemaining = mavlink_msg_sys_status_get_battery_remaining(&Msg); //Remaining battery energy: (0%: 0, 100%: 100)
				storeTelemetryData( FR_FUEL, mavlink_msg_sys_status_get_battery_remaining(&Msg) ) ;
        
//				sensors_enabled  = mavlink_msg_sys_status_get_onboard_control_sensors_enabled(&Msg);
//        health           = mavlink_msg_sys_status_get_onboard_control_sensors_health(&Msg); // Bitmask showing which onboard controllers and sensors are operational or have an error:  Value of 0: not enabled. Value of 1: enabled. Indices defined by ENUM MAV_SYS_STATUS_SENSOR
        
//				cpu_load         = mavlink_msg_sys_status_get_load(&Msg) / 10; // Maximum usage in percent of the mainloop time, (0%: 0, 100%: 1000) should be always below 1000
				storeTelemetryData( FR_CPU_LOAD, mavlink_msg_sys_status_get_load(&Msg) / 10 ) ;

//				sensors_health   = 0; // Default - All sensors status: ok
//        if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_3D_GYRO )
//        if (!( health & MAV_SYS_STATUS_SENSOR_3D_GYRO ))                    sensors_health |= MAV_SYS_STATUS_SENSOR_3D_GYRO;
//        if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_3D_ACCEL )
//        if (!( health & MAV_SYS_STATUS_SENSOR_3D_ACCEL ))                   sensors_health |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
//        if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_3D_MAG )
//        if (!( health & MAV_SYS_STATUS_SENSOR_3D_MAG ))                     sensors_health |= MAV_SYS_STATUS_SENSOR_3D_MAG;
//        if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE )
//        if (!( health & MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE ))          sensors_health |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
//        if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE )
//        if (!( health & MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE ))      sensors_health |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
//        if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_GPS )
//        if (!( health & MAV_SYS_STATUS_SENSOR_GPS ))                        sensors_health |= MAV_SYS_STATUS_SENSOR_GPS;
//        if ( sensors_enabled & MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW )
//        if (!( health & MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW ))               sensors_health |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
//        if ( sensors_enabled & MAV_SYS_STATUS_GEOFENCE )
//        if (!( health & MAV_SYS_STATUS_GEOFENCE ))                          sensors_health |= MAV_SYS_STATUS_GEOFENCE;
//        if ( sensors_enabled & MAV_SYS_STATUS_AHRS )
//        if (!( health & MAV_SYS_STATUS_AHRS ))                              sensors_health |= MAV_SYS_STATUS_AHRS;
////        return MAVLINK_MSG_ID_SYS_STATUS ;
	    }
  	  break ;

    	case MAVLINK_MSG_ID_GPS_RAW_INT:
      {
				uint16_t value1 ;
				uint16_t value2 ;
				int32_t gps ;
  			uint32_t value ;
				uint32_t temp ;
		
				gps = mavlink_msg_gps_raw_int_get_lat(&Msg) ;
				value = 0 ;
				if (gps < 0)
				{
					value = 1 ;
					gps = -gps ;
				}
				storeTelemetryData( FR_LAT_N_S, ( value & 1 ) ? 'S' : 'N' ) ;
				value = gps * 6 ;
				value /= 100 ;
				value &= 0x3FFFFFFF ;
				uint16_t bp ;
				temp = value / 10000 ;
				bp = (temp/ 60 * 100) + (temp % 60) ;
				storeTelemetryData( FR_GPS_LAT, bp ) ;
				storeTelemetryData( FR_GPS_LATd, value % 10000 ) ;

        gps = mavlink_msg_gps_raw_int_get_lon(&Msg);
				value = 0 ;
				if (gps < 0)
				{
					value = 1 ;
					gps = - gps ;
				}
				storeTelemetryData( FR_LONG_E_W, ( value & 1 ) ? 'W' : 'E' ) ;
				value = gps * 6 ;
				value /= 100 ;
				value &= 0x3FFFFFFF ;
				temp = value / 10000 ;
				bp = (temp/ 60 * 100) + (temp % 60) ;
				storeTelemetryData( FR_GPS_LONG, bp ) ;
				storeTelemetryData( FR_GPS_LONGd, value % 10000 ) ;

        value2 = mavlink_msg_gps_raw_int_get_fix_type(&Msg);
        value1 = mavlink_msg_gps_raw_int_get_satellites_visible(&Msg);
				storeTelemetryData( FR_TEMP2, value1 * 10 + value2 ) ;
				
//				gpsHdop           = mavlink_msg_gps_raw_int_get_eph(&Msg);
				storeTelemetryData( FR_GPS_HDOP, mavlink_msg_gps_raw_int_get_eph(&Msg) ) ;

//        gpsAltitude       = mavlink_msg_gps_raw_int_get_alt(&Msg); // meters * 1000
				storeTelemetryData( FR_SPORT_GALT, mavlink_msg_gps_raw_int_get_alt(&Msg) / 100 ) ;
//        gpsCourse         = mavlink_msg_gps_raw_int_get_cog(&Msg);
        
////				return MAVLINK_MSG_ID_GPS_RAW_INT;
      }
      break ;

      case MAVLINK_MSG_ID_VFR_HUD:
      {
				float fvalue ;
				int16_t course ;
//        lastMAVBeat = millis(); // we waiting only HUD packet
//        airspeed = mavlink_msg_vfr_hud_get_airspeed(&Msg);
//        gpsGroundSpeed = mavlink_msg_vfr_hud_get_groundspeed(&Msg); // Current ground speed in m/s
        course = mavlink_msg_vfr_hud_get_heading(&Msg); // 0..360 deg, 0=north
				if ( course < 0 )
				{
					course += 360 ;
				}
				storeTelemetryData( FR_COURSE, course ) ;
//        throttle = mavlink_msg_vfr_hud_get_throttle(&Msg);
        fvalue = mavlink_msg_vfr_hud_get_alt(&Msg)  * 100.0f ; // meters
				storeTelemetryData( FR_SPORT_ALT, fvalue ) ;
				fvalue = mavlink_msg_vfr_hud_get_climb(&Msg) ;
				storeTelemetryData( FR_VSPD, fvalue ) ;
////        return MAVLINK_MSG_ID_VFR_HUD;
      }
      break ;
            
//			case MAVLINK_MSG_ID_ATTITUDE:
//      {
////                accX = ToDeg(mavlink_msg_attitude_get_pitch(&Msg));
////                accY = ToDeg(mavlink_msg_attitude_get_roll(&Msg));
////                accZ = ToDeg(mavlink_msg_attitude_get_yaw(&Msg));
//        return MAVLINK_MSG_ID_ATTITUDE;
//      }
//      break ;

//      case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:
//      {
//            //nav_roll = mavlink_msg_nav_controller_output_get_nav_roll(&Msg);
//            //nav_pitch = mavlink_msg_nav_controller_output_get_nav_pitch(&Msg);
//            //nav_bearing = mavlink_msg_nav_controller_output_get_nav_bearing(&Msg);
//         wp_bearing = mavlink_msg_nav_controller_output_get_target_bearing(&Msg);
//         wp_dist = mavlink_msg_nav_controller_output_get_wp_dist(&Msg);
//            //alt_error = mavlink_msg_nav_controller_output_get_alt_error(&Msg);
//            //aspd_error = mavlink_msg_nav_controller_output_get_aspd_error(&Msg);
//            //xtrack_error = mavlink_msg_nav_controller_output_get_xtrack_error(&Msg);
////         return MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT;
//      }
//      break ;
            
//			case MAVLINK_MSG_ID_MISSION_CURRENT:
//      {
//      	wp_num = (uint8_t)mavlink_msg_mission_current_get_seq(&Msg);
////      	return MAVLINK_MSG_ID_MISSION_CURRENT;
//      }
//      break ;


      case MAVLINK_MSG_ID_RADIO :
			{
				uint8_t value ;
				value = mavlink_msg_radio_get_rssi(&Msg) ;
				if ( value )
				{
					storeRSSI( value ) ;
				}
			}
      break ;

			case MAVLINK_MSG_ID_RC_CHANNELS_RAW :
			{
				uint8_t value ;
				value = mavlink_msg_rc_channels_raw_get_rssi(&Msg) ;
				if ( value )
				{
					storeRSSI( value ) ;
				}
			}
      break ;
      case MAVLINK_MSG_ID_HWSTATUS:  
      {
				storeTelemetryData( FR_VCC, mavlink_msg_hwstatus_get_Vcc(&Msg) / 100 ) ;

////				return MAVLINK_MSG_ID_HWSTATUS;
      }
      break;
        
//			case MAVLINK_MSG_ID_STATUSTEXT:
//      {   
//        last_message_severity = mavlink_msg_statustext_get_severity(&Msg);
//        mavlink_msg_statustext_get_text(&Msg, last_message_text);
//				if (last_message_severity >= 3)
//				{
//			    status_msg = parse_msg();
//					msg_timer = MSG_TIMER;
//				}
////        return MAVLINK_MSG_ID_STATUSTEXT;
//      }
//      break ;
         
//			default:
////        return Msg.msgid;
//      break;
    }
  }
   // Update global packet drops counter
//    packet_drops += status.packet_rx_drop_count;
//    parse_error += status.parse_error;
//   return -1;
}
//void Mavlink::printMessage(SoftwareSerial* serialPort, IFrSkyDataProvider* dataProvider, int msg)
//{
//    if (msg == MAVLINK_MSG_ID_STATUSTEXT)
//    {
//        serialPort->println("");
//        serialPort->print(msg);
//        serialPort->print(":");
//        serialPort->print(last_message_text);
//        serialPort->print(": status_msg=");
//        serialPort->println(status_msg);
//    } else if (msg == MAVLINK_MSG_ID_HEARTBEAT) {
//    } else if (msg == MAVLINK_MSG_ID_SYS_STATUS) {
//    } else if (msg == MAVLINK_MSG_ID_GPS_RAW_INT) {
//    } else if (msg == MAVLINK_MSG_ID_VFR_HUD) {
//    } else if (msg == MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT) {
//    } else if (msg == MAVLINK_MSG_ID_ATTITUDE) {
//    } else if (msg == MAVLINK_MSG_ID_MISSION_CURRENT) {
//    } else if (msg == MAVLINK_MSG_ID_HWSTATUS) {
//    } else {
////        serialPort->print("RCV: MAVLINK_MSG_ID ");
////        serialPort->println(msg);
//    }
//}
//void Mavlink::reset()
//{
//   lastMAVBeat = 0;
//// Comment any line for keep last received data
//   batteryVoltage = 0;
//   current = 0;
//   batteryRemaining = 0;
//   gpsStatus = 0;
////   latitude = 0;   
////   longitude = 0;
//   gpsAltitude = 0;
//   gpsHdop = 999;
//   numberOfSatelites = 0;
//   gpsGroundSpeed = 0;
//   gpsCourse = 0;
//   altitude = 0.0f;
//   alt = 0;
//   apmMode = 99; // if no mavlink packets
////   course = 0;
//   throttle = 0;
////   accX = 0;
////   accY = 0;
////   accZ = 0;
//   last_message_severity = 0;
//   status_msg = 0;
//   cpu_load = 0;
//   cpu_vcc = 0;
////   apmBaseMode = 0; // if system armed, stay armed if link down
//   wp_dist = 0;
//   wp_num = 0;
//   wp_bearing = 270;
//   sensors_health = 0;
////   home_gps_alt = 0;
//   alt = 0;
//   gps_alt = 0;
//   lat = 0;
//   lon = 0;
//}

////------------------ Home Distance and Direction Calculation ----------------------------------
//void Mavlink::setHomeVars()
//{
//    float dstlon, dstlat;
//    long bearing;

//    if ( isArmed() ){
//       if ( gpsStatus > 2 ) {
//          gps_alt = (gpsAltitude - home_gps_alt);
//	   } else {
//	      gps_alt = 0;
//	   }
//       if (ok) {
//          //DST to Home
//          lat = latitude / 10000000.0f;
//          lon = longitude/ 10000000.0f;
//          dstlat = fabs(home_lat - lat) * 111319.5;
//          dstlon = fabs(home_lon - lon) * 111319.5 * scaleLongDown;
//          home_distance = sqrt((dstlat)*(dstlat) + (dstlon)*(dstlon));
//          //DIR to Home
//          dstlon = (home_lon - lon); //OffSet_X
//          dstlat = (home_lat - lat) * scaleLongUp; //OffSet Y
//          bearing = 90 + (atan2(dstlat, -dstlon) * 57.295775); //absolut home direction
//          if(bearing < 0) bearing += 360;   //normalization
//          bearing = bearing - 180;          //absolut return direction
//          if(bearing < 0) bearing += 360;   //normalization
//          bearing = bearing - course;       //relative home direction
//          if(bearing < 0) bearing += 360;   //normalization
//          home_direction = bearing;
//       } else {
//          home_distance = 0;
//          home_direction = 90;
//       }
//    } else {
//	   home_course = course;
//       home_direction = 90;
//       home_distance = 0;
//       gps_alt = gpsAltitude;
//       if ( gpsStatus > 2 ) {
//          home_gps_alt = gpsAltitude;
//          lat = latitude / 10000000.0f;
//          lon = longitude/ 10000000.0f;
//          home_lat = lat;
//          home_lon = lon;
//          // shrinking factor for longitude going to poles direction
//          rads = fabs(home_lat) * 0.0174532925;
//          scaleLongDown = cos(rads);
//          scaleLongUp   = 1.0f/cos(rads);
//          ok = true;
//       } else {
//          ok = false;
//		  home_gps_alt = 0;
//		  lat = 0.0f;
//		  lon = 0.0f;
//          home_lat = 0.0f;
//          home_lon = 0.0f;
//	   }
//    }
//}

