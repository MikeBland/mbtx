/*
 * Author - Erez Raviv <erezraviv@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
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

#ifndef pers_h
#define pers_h

//#include "file.h"

#define VERS 1

#include <stdint.h>
#include <string.h>

const uint8_t modn12x3[4][4]= {
  {1, 2, 3, 4},
  {1, 3, 2, 4},
  {4, 2, 3, 1},
  {4, 3, 2, 1} };



//convert from mode 1 to mode g_eeGeneral.stickMode
//NOTICE!  =>  1..4 -> 1..4
//#define CONVERT_MODE(x) (((x)<=4) ? modn12x3[g_eeGeneral.stickMode][((x)-1)] : (x))
#define CHANNEL_ORDER(x) (chout_ar[g_eeGeneral.templateSetup*4 + (x)-1])
#define THR_STICK       (2-(g_eeGeneral.stickMode&1))
#define ELE_STICK       (1+(g_eeGeneral.stickMode&1))
#define AIL_STICK       ((g_eeGeneral.stickMode&2) ? 0 : 3)
#define RUD_STICK       ((g_eeGeneral.stickMode&2) ? 3 : 0)


#define STK_RUD  1
#define STK_ELE  2
#define STK_THR  3
#define STK_AIL  4
#define STK_P1   5
#define STK_P2   6
#define STK_P3   7
#define NUM_TEMPLATES    DIM(n_Templates)
#define NUM_TEMPLATE_MIX 8
#define TEMPLATE_NLEN    15

#define TRIM_ON  0
#define TRIM_OFF 1

#define TRIM_EXTENDED_MAX	500

const uint8_t chout_ar[] = { //First number is 0..23 -> template setup,  Second is relevant channel out
1,2,3,4 , 1,2,4,3 , 1,3,2,4 , 1,3,4,2 , 1,4,2,3 , 1,4,3,2,
2,1,3,4 , 2,1,4,3 , 2,3,1,4 , 2,3,4,1 , 2,4,1,3 , 2,4,3,1,
3,1,2,4 , 3,1,4,2 , 3,2,1,4 , 3,2,4,1 , 3,4,1,2 , 3,4,2,1,
4,1,2,3 , 4,1,3,2 , 4,2,1,3 , 4,2,3,1 , 4,3,1,2 , 4,3,2,1    };


enum EnumKeys {
  KEY_MENU ,
  KEY_EXIT ,
  KEY_DOWN ,
  KEY_UP  ,
  KEY_RIGHT ,
  KEY_LEFT ,
  TRM_LH_DWN  ,
  TRM_LH_UP   ,
  TRM_LV_DWN  ,
  TRM_LV_UP   ,
  TRM_RV_DWN  ,
  TRM_RV_UP   ,
  TRM_RH_DWN  ,
  TRM_RH_UP   ,
  BTN_RE,
  //SW_NC     ,
  //SW_ON     ,
  SW_ThrCt  ,
  SW_RuddDR ,
  SW_ElevDR ,
  SW_ID0    ,
  SW_ID1    ,
  SW_ID2    ,
  SW_AileDR ,
  SW_Gear   ,
  SW_Trainer,
  SW_SF2 = SW_ThrCt,
  SW_nu1 = SW_ThrCt+1,
  SW_nu2 = SW_ThrCt+2,
  SW_SC0 = SW_ThrCt+3,
  SW_SC1 = SW_ThrCt+4,
  SW_SC2 = SW_ThrCt+5,
  SW_nu3 = SW_ThrCt+6,
  SW_nu4 = SW_ThrCt+7,
  SW_SH2 = SW_ThrCt+8,
//  SW_SD0 = SW_ThrCt+9,
//  SW_SD1 = SW_ThrCt+10,
//  SW_SD2 = SW_ThrCt+11,
//  SW_SE0 = SW_ThrCt+12,
//  SW_SE1 = SW_ThrCt+13,
//  SW_SE2 = SW_ThrCt+14,
//  SW_SF0 = SW_ThrCt+15,
//  SW_SF2 = SW_ThrCt+16,
//  SW_SG0 = SW_ThrCt+17,
//  SW_SG1 = SW_ThrCt+18,
//  SW_SG2 = SW_ThrCt+19,
//  SW_SH0 = SW_ThrCt+20,
//  SW_SH2 = SW_ThrCt+21
};

// Hardware switch mappings:
#define HSW_ThrCt			1
#define HSW_RuddDR		2
#define HSW_ElevDR		3
#define HSW_ID0				4
#define HSW_ID1				5
#define HSW_ID2				6
#define HSW_AileDR		7
#define HSW_Gear			8
#define HSW_Trainer		9

//#define HSW_Thr3pos0	35
//#define HSW_Thr3pos1	36
//#define HSW_Thr3pos2	37
//#define HSW_Rud3pos0	38
//#define HSW_Rud3pos1	39
//#define HSW_Rud3pos2	40
//#define HSW_Ele3pos0	41
//#define HSW_Ele3pos1	42
//#define HSW_Ele3pos2	43
//#define HSW_Ail3pos0	44
//#define HSW_Ail3pos1	45
//#define HSW_Ail3pos2	46
//#define HSW_Gear3pos0	47
//#define HSW_Gear3pos1	48
//#define HSW_Gear3pos2	49
//#define HSW_Ele6pos0	50
//#define HSW_Ele6pos1	51
//#define HSW_Ele6pos2	52
//#define HSW_Ele6pos3	53
//#define HSW_Ele6pos4	54
//#define HSW_Ele6pos5	55
//#define HSW_MAX				55

#define HSW_Thr3pos0	45	// Skip some values because of safety switch values
#define HSW_Thr3pos1	46
#define HSW_Thr3pos2	47
#define HSW_Rud3pos0	48
#define HSW_Rud3pos1	49
#define HSW_Rud3pos2	50
#define HSW_Ele3pos0	51
#define HSW_Ele3pos1	52
#define HSW_Ele3pos2	53
#define HSW_Ail3pos0	54
#define HSW_Ail3pos1	55
#define HSW_Ail3pos2	56
#define HSW_Gear3pos0	57
#define HSW_Gear3pos1	58
#define HSW_Gear3pos2	59
#define HSW_Ele6pos0	60
#define HSW_Ele6pos1	61
#define HSW_Ele6pos2	62
#define HSW_Ele6pos3	63
#define HSW_Ele6pos4	64
#define HSW_Ele6pos5	65
#define HSW_Pb1				66
#define HSW_Pb2				67
#define HSW_Pb3				68
#define HSW_Pb4				69
#define HSW_MAX				69
#define HSW_MAX_X9D		65

#define HSW_SF2				1
//#define HSW_SF2				2

#define HSW_SC0				4
#define HSW_SC1				5
#define HSW_SC2				6

//#define HSW_SH0				8
#define HSW_SH2				9

#define HSW_SB0				45	// Skip some values because of safety switch values
#define HSW_SB1				46
#define HSW_SB2				47
#define HSW_SE0				48
#define HSW_SE1				49
#define HSW_SE2				50
#define HSW_SA0				51
#define HSW_SA1				52
#define HSW_SA2				53
#define HSW_SD0				54
#define HSW_SD1				55
#define HSW_SD2				56
#define HSW_SG0				57
#define HSW_SG1				58
#define HSW_SG2				59

//Bitfield for hardware switch mapping
#define	USE_THR_3POS	0x01
#define	USE_RUD_3POS	0x02
#define	USE_ELE_3POS	0x04
#define	USE_ELE_6POS	0x08
#define	USE_AIL_3POS	0x10
#define	USE_GEA_3POS	0x20
#define	USE_ELE_6PSB	0x40
#define	USE_PB1				0x80
#define	USE_PB2				0x100
#define	USE_PB3				0x200
#define	USE_PB4				0x400

#define	USE_P1_6POS		0x04
#define	USE_P2_6POS		0x08
#define	USE_P3_6POS		0x0C
#define	USE_AUX_6POS	0x10
#define MASK_6POS			0x1C

#define SWITCHES_STR "THRRUDELEID0ID1ID2AILGEATRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfTH^TH-THvRU^RU-RUvEL^EL-ELvAI^AI-AIvGE^GE-GEv6P06P16P26P36P46P5PB1PB2"
#define XSWITCHES_STR "SF       SC^SC-SCv      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB^SB-SBvSE^SE-SEvSA^SA-SAvSD^SD-SDvSG^SG-SGv6P06P16P26P36P46P5PB1PB2"
#define NUM_CSW  12 //number of custom switches
#define NUM_SKYCSW  24 //number of custom switches
#define CSW_INDEX	9	// Index of first custom switch
#define NUM_FSW			16

//#define SW_BASE      SW_NC
#define SW_BASE      SW_ThrCt
#define SW_BASE_DIAG SW_ThrCt
//#define SWITCHES_STR "  NC  ON THR RUD ELE ID0 ID1 ID2 AILGEARTRNR"
#define MAX_DRSWITCH (1+SW_Trainer-SW_ThrCt+1+NUM_SKYCSW)
#define MAX_XDRSWITCH (1+SW_SH2-SW_SF2+1+NUM_SKYCSW)

#define SWP_ID0 (SW_ID0-SW_BASE)
#define SWP_ID1 (SW_ID1-SW_BASE)
#define SWP_ID2 (SW_ID2-SW_BASE)
#define SWP_ID0B (1<<SWP_ID0)
#define SWP_ID1B (1<<SWP_ID1)
#define SWP_ID2B (1<<SWP_ID2)

//Switch Position Illigal states
#define SWP_IL1 (0)
#define SWP_IL2 (SWP_ID0B | SWP_ID1B)
#define SWP_IL3 (SWP_ID0B | SWP_ID2B)
#define SWP_IL4 (SWP_ID1B | SWP_ID2B)
#define SWP_IL5 (SWP_ID0B | SWP_ID1B | SWP_ID2B)

#define CURV_STR "---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24"
#define CURVE_BASE 7
#define CSWITCH_STR  "----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""v1>=v2 ""v1<=v2 Timer  Ntimer 1-shot 1-shotRv~=val "
#define CSW_NUM_FUNC 19
#define CSW_LEN_FUNC 7

#define CS_OFF       0
#define CS_VPOS      1  //v>offset
#define CS_VNEG      2  //v<offset
#define CS_APOS      3  //|v|>offset
#define CS_ANEG      4  //|v|<offset
#define CS_AND       5
#define CS_OR        6
#define CS_XOR       7
#define CS_EQUAL     8
#define CS_NEQUAL    9
#define CS_GREATER   10
#define CS_LESS      11
#define CS_EGREATER  12
#define CS_LATCH     12
#define CS_ELESS     13
#define CS_FLIP			 13
#define CS_TIME	     14
#define CS_NTIME     15
#define CS_MONO			 16
#define CS_RMONO	   17	// Monostable with reset
#define CS_EXEQUAL   18	// V~=offset
#define CS_MAXF      18  //max function

#define CS_VOFS       0
#define CS_VBOOL      1
#define CS_VCOMP      2
#define CS_TIMER			3
#define CS_TMONO			4
//#define CS_STATE(x)   ((x)<CS_AND ? CS_VOFS : ((((x)<CS_EQUAL) || ((x)>=CS_LATCH)) ? CS_VBOOL : ((x)<CS_TIME ? CS_VCOMP : CS_TIMER)))

#define CHAR_FOR_NAMES " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-."
#define CHAR_FOR_NAMES_REGEX "[ A-Za-z0-9_.-]*"

#define SWASH_TYPE_120   1
#define SWASH_TYPE_120X  2
#define SWASH_TYPE_140   3
#define SWASH_TYPE_90    4

#define MIX_P1    5
#define MIX_P2    6
#define MIX_P3    7
#define MIX_MAX   8
#define MIX_FULL  9
#define MIX_CYC1  10
#define MIX_CYC2  11
#define MIX_CYC3  12

#define EXTRA_POTS_POSITION	8
#define EXTRA_POTS_START	120
#define NUM_EXTRA_POTS		1

#define PPM_BASE   (MIX_CYC3)
#define CHOUT_BASE (PPM_BASE+NUM_PPM)

#define CM(x,y,z) (CONVERT_MODE(x,y,z))  //good for SRC
#define CH(x) (CHOUT_BASE+(x))
#define CV(x) (CURVE_BASE+(x)-1)
#define CC(x) (CHANNEL_ORDER(x)) //need to invert this to work with dest

#define CURVE5(x) ((x)-1)
#define CURVE9(x) (MAX_CURVE5+(x)-1)

#define DR_HIGH   0
#define DR_MID    1
#define DR_LOW    2
#define DR_EXPO   0
#define DR_WEIGHT 1
#define DR_RIGHT  0
#define DR_LEFT   1
#define DR_DRSW1  99
#define DR_DRSW2  98

#define DSW_THR   1
#define DSW_RUD   2
#define DSW_ELE   3
#define DSW_ID0   4
#define DSW_ID1   5
#define DSW_ID2   6
#define DSW_AIL   7
#define DSW_GEA   8
#define DSW_TRN   9
#define DSW_SW1   10
#define DSW_SW2   11
#define DSW_SW3   12
#define DSW_SW4   13
#define DSW_SW5   14
#define DSW_SW6   15
#define DSW_SW7   16
#define DSW_SW8   17
#define DSW_SW9   18
#define DSW_SWA   19
#define DSW_SWB   20
#define DSW_SWC   21
#define DSW_SWD   22
#define DSW_SWE   23
#define DSW_SWF   24
#define DSW_SWG   25
#define DSW_SWH   26
#define DSW_SWI   27
#define DSW_SWJ   28
#define DSW_SWK   29
#define DSW_SWL   30
#define DSW_SWM   31
#define DSW_SWN   32
#define DSW_SWO   33


#define NUM_KEYS TRM_RH_UP+1
#define TRM_BASE TRM_LH_DWN


#define TMRMODE_NONE     0
#define TMRMODE_ABS      1
#define TMRMODE_THR      2
#define TMRMODE_THR_REL  3
#define MAX_ALERT_TIME   60

#define PROTO_PPM        0
#define PROTO_PXX        1
#define PROTO_DSM2       2
#define PROTO_MULTI			 3
#define PROTO_ASSAN			 4
#define PROTO_OFF		     15		// For X9D/9Xtreme
#define PROT_MAX         2
#define PROT_STR "PPM   PXX   DSM2  "
#define PROT_STR_LEN     6
#define DSM2_STR "LP4/LP5  DSM2only DSM2/DSMX"
#define DSM2_STR_LEN   9
#define LPXDSM2          0
#define DSM2only         1
#define DSM2_DSMX        2



#define GETADC_SING = 0
#define GETADC_OSMP = 1
#define GETADC_FILT = 2


/// liefert Dimension eines Arrays
#define DIM(arr) (sizeof((arr))/sizeof((arr)[0]))


#define EE_GENERAL 1
#define EE_MODEL   2
/// Markiert einen EEPROM-Bereich als dirty. der Bereich wird dann in
/// eeCheck ins EEPROM zurueckgeschrieben.
void eeCheck(uint8_t msk);
//void eeWriteGeneral();
void eeLoadModelName(uint8_t id,char*buf,uint8_t len);
void eeLoadModel(uint8_t id);
//void eeSaveModel(uint8_t id);
bool eeDuplicateModel(uint8_t id);
bool eeLoadGeneral();

#define NUM_PPM     8
#define NUM_EXTRA_PPM     8
//number of real outputchannels CH1-CH8
#define NUM_CHNOUT   16
#define NUM_SKYCHNOUT  24
///number of real input channels (1-9) plus virtual input channels X1-X4
#define NUM_XCHNRAW (NUM_CHNOUT+12+NUM_PPM) // NUMCH + P1P2P3+ AIL/RUD/ELE/THR + MAX/FULL + CYC1/CYC2/CYC3
#define NUM_SKYXCHNRAW (CHOUT_BASE+NUM_SKYCHNOUT) // NUMCH + P1P2P3+ AIL/RUD/ELE/THR + MAX/FULL + CYC1/CYC2/CYC3
///number of real output channels (CH1-CH8) plus virtual output channels X1-X4
#define NUM_XCHNOUT (NUM_CHNOUT) //(NUM_CHNOUT)//+NUM_VIRT)
#define NUM_SKYXCHNOUT (NUM_SKYCHNOUT) //(NUM_CHNOUT)//+NUM_VIRT)

#define MIX_3POS	(NUM_SKYXCHNRAW+1)


#define TMR_VAROFS  4

#define SUB_MODE_V     1
#define SUB_MODE_H     2
#define SUB_MODE_H_DBL 3
//uint8_t checkSubGen(uint8_t event,uint8_t num, uint8_t sub, uint8_t mode);



#include "myeeprom.h"


class EEPFILE
{
//    EFile *theFile;
    bool fileChanged;


public:
    EEPFILE();

    bool Changed();
    void setChanged(bool v);
    void saveFile(void* buf);

    //bool eeLoadModel(uint8_t id);
    bool eeModelExists(uint8_t id);
    void eeLoadModelName(uint8_t id,char*buf,uint8_t len);
    void eeLoadOwnerName(char*buf,uint8_t len);
    void getModelName(uint8_t id, char* buf);
    void modelDefault(uint8_t id);
    void DeleteModel(uint8_t id);
    bool eeLoadGeneral();
    void generalDefault();

    int  getModel(ModelData* model, uint8_t id);
    bool putModel(ModelData* model, uint8_t id);
    int  getGeneralSettings(EEGeneral* setData);
    bool putGeneralSettings(EEGeneral* setData);

    void formatEFile();
//    int size(int id) {return theFile->size(id);}

};


#endif //pers_h


