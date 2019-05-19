/*
 * Author - Mike Blandford
 *
 * Based on code named
 * OpenTX
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _PULSES_PXX2_H_
#define _PULSES_PXX2_H_

//#include "fifo.h"
//#include "io/frsky_pxx2.h"
//#include "./pxx.h"

#define PXX2_TYPE_C_MODULE          0x01
  #define PXX2_TYPE_ID_REGISTER     0x01
  #define PXX2_TYPE_ID_BIND         0x02
  #define PXX2_TYPE_ID_CHANNELS     0x03
  #define PXX2_TYPE_ID_TX_SETTINGS  0x04
  #define PXX2_TYPE_ID_RX_SETTINGS  0x05
  #define PXX2_TYPE_ID_HW_INFO      0x06
  #define PXX2_TYPE_ID_SHARE        0x07
  #define PXX2_TYPE_ID_RESET        0x08
  #define PXX2_TYPE_ID_TELEMETRY    0xFE

#define PXX2_TYPE_C_POWER_METER     0x02
  #define PXX2_TYPE_ID_POWER_METER  0x01
  #define PXX2_TYPE_ID_SPECTRUM     0x02

#define PXX2_TYPE_C_OTA             0xFE

#define PXX2_CHANNELS_FLAG0_FAILSAFE         (1 << 6)
#define PXX2_CHANNELS_FLAG0_RANGECHECK       (1 << 7)

#define PXX2_RX_SETTINGS_FLAG0_WRITE               (1 << 6)

#define PXX2_RX_SETTINGS_FLAG1_TELEMETRY_DISABLED  (1 << 7)
#define PXX2_RX_SETTINGS_FLAG1_READONLY            (1 << 6)
#define PXX2_RX_SETTINGS_FLAG1_FASTPWM             (1 << 4)




#define PXX2_INTERNAL_BAUDRATE		450000
#define PXX2_EXTERNAL_BAUDRATE		450000
#define PXX2_SPORT_BAUDRATE				450000

// for every 6mS frames, use 1666 to give 10 seconds
#define PXX2_FAILSAFE_RATE				1666

#define PXX2_LEN_RX_NAME							8
#define PXX2_LEN_REGISTRATION_ID			8
#define PXX2_MAX_RECEIVERS_PER_MODULE		3

#define MODULE_MODE_NORMAL							0
#define MODULE_MODE_GET_HARDWARE_INFO   1
#define MODULE_MODE_RECEIVER_SETTINGS   2
#define MODULE_MODE_REGISTER            3
#define MODULE_MODE_BIND                4
#define MODULE_MODE_SPECTRUM_ANALYSER   5
#define MODULE_MODE_SHARE               6
#define MODULE_MODE_GETSET_TX           7
#define MODULE_MODE_RESET								8

//#define BIND_START								0
//#define BIND_WAIT									1
//#define BIND_RX_NAME_SELECTED			2
//#define BIND_OK										3

//#define REGISTER_START           	0
//#define REGISTER_RX_NAME_RECEIVED 1
//#define REGISTER_RX_NAME_SELECTED 2
//#define REGISTER_OK               3


struct t_moduleSettings
{
	uint8_t mode ;
} ;

struct t_moduleControl
{
	uint8_t registerStep ;
	uint8_t bindStep ;
	uint8_t registerModuleIndex ;
	uint8_t bindReceiverId ;
	int16_t step ;
	uint16_t timeout ;
	uint16_t bindWaitTimeout ;
	uint16_t receiverSetupTimeout ;
	uint8_t bindReceiverCount ;
	uint8_t bindReceiverIndex ;
	uint8_t bindReceiverNameIndex ;
	uint8_t resetType ;
	uint8_t receiverSetupReceiverId ;
	uint8_t rxtxSetupState ;
	uint8_t receiverSetupTelemetryDisabled:1 ;
	uint8_t receiverSetupPwmRate:1 ;
	uint8_t channelMapping[8] ;
	uint8_t registerRxName[PXX2_LEN_RX_NAME] ;
	uint8_t bindReceiversNames[3][PXX2_LEN_RX_NAME] ;
	uint8_t modelRegistrationID[PXX2_LEN_REGISTRATION_ID] ;
	uint16_t hwVersion ; 
	uint16_t swVersion ;
	uint16_t rxHwVersion ; 
	uint16_t rxSwVersion ;
	uint8_t variant ;
	uint8_t rxModuleId ;
	uint8_t moduleId ;
	int8_t power ;
	uint8_t moduleExtAerial ;
	uint8_t optionsState ;
	uint8_t moduleRequest ;
} ;

enum PXX2RegisterSteps
{
  REGISTER_START,
  REGISTER_RX_NAME_RECEIVED,
  REGISTER_RX_NAME_SELECTED,
  REGISTER_OK
};

enum PXX2BindSteps
{
  BIND_START,
  BIND_RX_NAME_SELECTED,
  BIND_WAIT,
  BIND_OK
};

enum PXX2ReceiverStatus
{
  RECEIVER_SETTINGS_READ,
  RECEIVER_SETTINGS_WRITE,
  RECEIVER_SETTINGS_OK
};

enum PXX2ModuleStatus
{
  MODULE_SETTINGS_READ,
  MODULE_SETTINGS_WRITE,
  MODULE_SETTINGS_OK
};

enum SpectrumFields
{
  SPECTRUM_FREQUENCY,
  SPECTRUM_SPAN,
  SPECTRUM_FIELDS_MAX
} ;


void accessRecieveByte( uint16_t data, uint32_t module ) ;


extern struct t_moduleSettings ModuleSettings[2] ;
extern struct t_moduleControl ModuleControl[2] ;

//extern ModuleFifo intmoduleFifo;
//extern ModuleFifo extmoduleFifo;

//class Pxx2CrcMixin {
//  protected:
//    void initCrc()
//    {
//      crc = 0xFFFF;
//    }

//    void addToCrc(uint8_t byte)
//    {
//      crc -= byte;
//    }

//    uint16_t crc;
//};

//class Pxx2Transport: public DataBuffer<uint8_t, 64>, public Pxx2CrcMixin {
//  protected:
//    void addWord(uint32_t word)
//    {
//      addByte(word);
//      addByte(word >> 8);
//      addByte(word >> 16);
//      addByte(word >> 24);
//    }

//    void addByte(uint8_t byte)
//    {
//      Pxx2CrcMixin::addToCrc(byte);
//      addByteWithoutCrc(byte);
//    };

//    void addByteWithoutCrc(uint8_t byte)
//    {
//      *ptr++ = byte;
//    }
//};

//class Pxx2Pulses: public PxxPulses<Pxx2Transport> {
//  public:
//    void setupFrame(uint8_t module);

//  protected:
//    void setupHardwareInfoFrame(uint8_t module);

//    void setupRegisterFrame(uint8_t module);

//    void setupBindFrame(uint8_t module);

//    void setupShareMode(uint8_t module);

//    void setupReceiverSettingsFrame(uint8_t module);

//    void setupChannelsFrame(uint8_t module);

//    void setupSpectrumAnalyser(uint8_t module);

//    void addHead()
//    {
//      // send 7E, do not CRC
//      Pxx2Transport::addByteWithoutCrc(0x7E);

//      // reserve 1 byte for LEN
//      Pxx2Transport::addByteWithoutCrc(0x00);
//    }

//    void addFrameType(uint8_t type_c, uint8_t type_id)
//    {
//      // TYPE_C + TYPE_ID
//      // TODO optimization ? Pxx2Transport::addByte(0x26); // This one is CRC-ed on purpose

//      Pxx2Transport::addByte(type_c);
//      Pxx2Transport::addByte(type_id);
//    }

//    uint8_t addFlag0(uint8_t module);

//    void addFlag1(uint8_t module);

//    void addChannels(uint8_t module, uint8_t sendFailsafe, uint8_t firstChannel);

//    void addCrc()
//    {
//      Pxx2Transport::addByteWithoutCrc(Pxx2CrcMixin::crc >> 8);
//      Pxx2Transport::addByteWithoutCrc(Pxx2CrcMixin::crc);
//    }

//    void initFrame()
//    {
//      // init the CRC counter
//      initCrc();

//      // reset the frame pointer
//      Pxx2Transport::initBuffer();

//      // add the frame head
//      addHead();
//    }

//    void endFrame()
//    {
//      uint8_t size = getSize() - 2;

//      if (size > 0) {
//        // update the frame LEN = frame length minus the 2 first bytes
//        data[1] = getSize() - 2;

//        // now add the CRC
//        addCrc();
//      }
//      else {
//        Pxx2Transport::initBuffer();
//      }
//    }
//};

static const char * const PXX2modulesModels[] = {
  "---",
  "XJT",
  "ISRM",
  "ISRM-PRO",
  "ISRM-S",
  "R9M",
  "R9MLite",
  "R9MLite-PRO",
  "ISRM-N"
};

static const char * const PXX2receiversModels[] = {
  "---",
  "X8R",
  "RX8R",
  "RX8R-PRO",
  "RX6R",
  "RX4R",
  "G-RX8",
  "G-RX6",
  "X6R",
  "X4R",
  "X4R-SB",
  "XSR",
  "XSR-M",
  "RXSR",
  "S6R",
  "S8R",
  "XM",
  "XM+",
  "XMR",
  "R9",
  "R9-SLIM",
  "R9-SLIM+",
  "R9-MINI",
  "R9-MM",
  "R9-STAB",
};

#endif
