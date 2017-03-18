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

#define VERS 1

#ifdef V2
#define XSW_MOD	1
#endif

#include <stdint.h>
#include <string.h>

const uint8_t modn12x3[4][4]= {
  {1, 2, 3, 4},
  {1, 3, 2, 4},
  {4, 2, 3, 1},
  {4, 3, 2, 1} };

#ifdef V2
#define EEGeneral V2EEGeneral
#define ModelData V2ModelData
#else
#define EEGeneral V1EEGeneral
#define ModelData V1ModelData
#endif

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
  //SW_NC     ,
  //SW_ON     ,
#ifdef XSW_MOD
  SW_IDL ,            // ID0/ID1/ID2
  SW_ThrCt  ,         // TH^/TH-/THv
  SW_RuddDR ,         // RU^/RU-/RUv
  SW_ElevDR ,         // EL^/EL-/ELv
  SW_AileDR ,         // AI^/AI-/AIv
  SW_Gear   ,         // GE^/GE-/GEv
  SW_PB1,             // push button 1
  SW_PB2,             // push button 2
  SW_Trainer
#else
  SW_ThrCt  ,
  SW_RuddDR ,
  SW_ElevDR ,
  SW_ID0    ,
  SW_ID1    ,
  SW_ID2    ,
  SW_AileDR ,
  SW_Gear   ,
  SW_Trainer
#endif
};

#ifdef XSW_MOD

// physical switch base indices
#define PSW_BASE      SW_IDL        // the first physical/3-pos switch
#define PSW_3POS_END  SW_Gear       // the last 3-pos switch
#define PSW_END       SW_Trainer    // the last physical switch
#define XSW_BASE      SW_ThrCt      // the first extra switch
#define XSW_3POS_END  SW_Gear       // the last extra 3-pos switch
#define XSW_END       SW_PB2        // the last extra switch

// max values
#define MAX_PSW3POS     (PSW_3POS_END-PSW_BASE+1) // 6 physical 3-pos switches
#define MAX_PSW2POS     (PSW_END-PSW_3POS_END)    // 3 physical 2-pos switches
#define MAX_PSWITCH     (MAX_PSW3POS+MAX_PSW2POS) // 9 physical switches
#define MAX_XSWITCH     (XSW_END-XSW_BASE+1)      // 7 extra switches

// extra switch sources: 4bits [0..15]
#define SSW_NONE        0       // No source
#define SSW_PB7         1       // Backlight, parallel Voice card strobe
#define SSW_PC0         2       // NC - other LCD CS, rerouted LV trim UP
#define SSW_PC4         3       // LCD_WR - serial LCD SCL, rerouted LV trim DWN
#define SSW_PC6         4       // FrSky - rerouted ThrCt
#define SSW_PC7         5       // FrSky - rerouted AileDR
#define SSW_PG2         6       // Haptic
#define SSW_PG5         7       // M2561 only
#define SSW_XPB0        8       // voice module PB0 - D3 (EXT1)
#define SSW_XPD2        9       // voice module PD2 - CLK
#define SSW_XPD3        10      // voice module PD3 - D0 (TRIM_LV_DWN)
#define SSW_XPD4        11      // voice module PD4 - D1 (TRIM_LV_UP)
#define SSW_XPB1        12      // voice module PB1 - BL
#define SSW_XPC0        13      // voice module PC0 - BUSY
#define SSW_XPD7        14      // voice module PD7 - D2 (EXT2)


#define DSW____  0    // "---"
#define DSW_IDL  1
#define DSW_THR  2
#define DSW_RUD  3
#define DSW_ELE  4
#define DSW_AIL  5
#define DSW_GEA  6
#define DSW_PB1  7
#define DSW_PB2  8
#define DSW_TRN  9

#define DSW_ID0  SW_3POS_BASE
#define DSW_ID1  (DSW_ID0+1)
#define DSW_ID2  (DSW_ID0+2)

#define MAX_DRSWITCH    (1+MAX_PSWITCH+MAX_CSWITCH)
#define SW_3POS_BASE    (MAX_DRSWITCH+1)          // ID0,1,2,TH^,-,v,RU^,-,v,EL^,-,v,AI^,-,v,GE^,-,v
#define SW_3POS_END     (SW_3POS_BASE+3*MAX_PSW3POS-1)

#define MAX_CSWITCH     (NUM_CSW+EXTRA_CSW)       // max 18 custom switches (L1..L9,LA..LI)

#define V2TOGGLE_INDEX    (SW_3POS_END)

//#else	// !XSW_MOD
#endif  // XSW_MOD

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

#define HSW_Ele3pos0	31
#define HSW_Ele3pos1	32
#define HSW_Ele3pos2	33
#define HSW_Rud3pos0	34
#define HSW_Rud3pos1	35
#define HSW_Rud3pos2	36
#define HSW_Ail3pos0	37
#define HSW_Ail3pos1	38
#define HSW_Ail3pos2	39
#define HSW_Gear3pos0	40
#define HSW_Gear3pos1	41
#define HSW_Gear3pos2	42
#define HSW_Pb1				43
#define HSW_Pb2				44
#define HSW_MAX				44

#define V1TOGGLE_INDEX	HSW_MAX


#define SWITCHES_STR "THRRUDELEID0ID1ID2AILGEATRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI EL^EL-ELvRU^RU-RUvAI^AI-AIvGE^GE-GEvPB1PB2"
#define NUM_CSW  12 //number of custom switches
#define EXTRA_CSW	6
#define EXTRA_VOICE_SW	8


//#define SW_BASE      SW_NC
#define SW_BASE      SW_ThrCt
#define SW_BASE_DIAG SW_ThrCt
//#define SWITCHES_STR "  NC  ON THR RUD ELE ID0 ID1 ID2 AILGEARTRNR"
#ifndef V2
#define MAX_DRSWITCH (1+SW_Trainer-SW_ThrCt+1+NUM_CSW)
#endif
#define PHY_SWITCH		(1+SW_Trainer-SW_ThrCt+1)

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

#define CURV_STR     "---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16"
#define CURVE_BASE 7
#define CSWITCH_STR  "----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""v1>=v2 ""v1<=v2 Timer  v~=val "
#define CSW_NUM_FUNC 15
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
#define CS_LATCH	   12
#define CS_ELESS     13
#define CS_FLIP	     13
#define CS_TIME	     14
#define CS_MAXF      14  //max function

#define CS_VOFS       0
#define CS_VBOOL      1
#define CS_VCOMP      2
#define CS_TIMER			3

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

#ifndef V2
#define DSW_THR   1
#define DSW_RUD   2
#define DSW_ELE   3
#define DSW_ID0   4
#define DSW_ID1   5
#define DSW_ID2   6
#define DSW_AIL   7
#define DSW_GEA   8
#endif
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


#define	USE_THR_3POS	0x01
#define	USE_RUD_3POS	0x02
#define	USE_ELE_3POS	0x04
#define	USE_ELE_6POS	0x08
#define	USE_AIL_3POS	0x10
#define	USE_GEA_3POS	0x20
#define	USE_PB1				0x40
#define	USE_PB2				0x80

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
#define PROTO_PPM16			 3
#define PROTO_PPMSIM		 4
#define PROTO_MULTI			 5
#define PROT_MAX         5
#define PROT_STR "PPM   PXX   DSM2  PPM16 "
#define PROT_STR_LEN     6
#define DSM2_STR "LP4/LP5  DSM2only DSM2/DSMX"
#define DSM2_STR_LEN   9
#define LPXDSM2          0
#define DSM2only         1
#define DSM2_DSMX        2


// MULTI options
#define M_Flysky          0
#define M_Hubsan          1
#define M_Frsky           2
#define M_Hisky           3
#define M_V2x2            4
#define M_DSM2            5
#define M_Devo	  	      6
#define M_YD717	          7
#define M_KN	  	        8
#define M_SymaX	          9
#define M_SLT		  		   10
#define M_CX10		       11
#define M_CG023		       12
#define M_BAYANG	       13
#define M_FRSKYX	       14
#define M_ESKY		       15
#define M_MT99XX	       16
#define M_MJXQ		       17
#define M_SHENQI				 18
#define M_FY326					 19
#define M_SFHSS					 20
#define M_J6PRO					 21
#define M_FQ777					 22
#define M_ASSAN					 23
#define M_FRSKYV	       24
#define M_HONTAI	       25
#define M_OPENLRS	       26
#define M_AFHDS2A	       27
#define M_Q2X2		       28


#define GETADC_SING = 0
#define GETADC_OSMP = 1
#define GETADC_FILT = 2


/// liefert Dimension eines Arrays
#define DIM(arr) (sizeof((arr))/sizeof((arr)[0]))

// RadioData types
#define	TYPE_AVR2K					0
#define TYPE_AVR4K					1
#define TYPE_SKY						2
#define TYPE_SKYR				 		3
#define TYPE_TARANIS				4
#define TYPE_TARANIS_PLUS		5

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
//number of real outputchannels CH1-CH8
#define NUM_CHNOUT   16
///number of real input channels (1-9) plus virtual input channels X1-X4
#define NUM_XCHNRAW (NUM_CHNOUT+12+NUM_PPM) // NUMCH + P1P2P3+ AIL/RUD/ELE/THR + MAX/FULL + CYC1/CYC2/CYC3
///number of real output channels (CH1-CH8) plus virtual output channels X1-X4
#define NUM_XCHNOUT (NUM_CHNOUT) //(NUM_CHNOUT)//+NUM_VIRT)

#define MIX_3POS	(NUM_XCHNRAW+1)


#define TMR_VAROFS  4
//#define TMR_VAROFS  16

#define SUB_MODE_V     1
#define SUB_MODE_H     2
#define SUB_MODE_H_DBL 3
//uint8_t checkSubGen(uint8_t event,uint8_t num, uint8_t sub, uint8_t mode);


#ifndef SKY
#ifdef V2
#include <QString>
#include <QComboBox>
#endif
#include "myeeprom.h"
#include "file.h"


class EEPFILE
{
    EFile *theFile;
    bool fileChanged;

public:
    EEPFILE();

    bool Changed();
    void setChanged(bool v);
    bool loadFile(void* buf);
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
#ifdef V2
    int  getGeneralSettings(V2EEGeneral* setData);
    bool putGeneralSettings(V2EEGeneral* setData);
#else
    int  getGeneralSettings(EEGeneral* setData);
    bool putGeneralSettings(EEGeneral* setData);
#endif

    int eesize() {return theFile->m_type ? EESIZE128 : EESIZE64;}
    void setSize( int size) { mee_type = size ; theFile->m_type = mee_type ; }
		
		void formatEFile();
    int size(int id) {return theFile->size(id);}
    int freespace() {return theFile->freespace();}
  	uint8_t  mee_type ;     // 0 = M64, 1 = M128

};
#endif



#endif //pers_h


