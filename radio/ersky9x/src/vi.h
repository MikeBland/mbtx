/****************************************************************************
*  Copyright (c) 2013 by Michael Blandford. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*  3. Neither the name of the author nor the names of its contributors may
*     be used to endorse or promote products derived from this software
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
*  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*  SUCH DAMAGE.
*
****************************************************************************
* Other Authors:
 * - Andre Bernet
 * - Bertrand Songis
 * - Bryan J. Rentoul (Gruvin)
 * - Cameron Weeks
 * - Erez Raviv
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini
 * - Thomas Husterer
*
****************************************************************************/

#define FWx4		"\030"
#define FWx5		"\036"
#define FWx9		"\066"
#define FWx10		"\074"
#define FWx11		"\102"
#define FWx12		"\110"
#define FWx13		"\116"
#define FWx14		"\124"
#define FWx15		"\132"
#define FWx16		"\140"
#define FWx17		"\146"
#define FWx18		"\152"

#define I_REMOVED						0


#define ISTR_ON             "MO "
#define ISTR_OFF            "TAT"
#define ISTR_X_OFF_ON				FWx17 "\001" "\003" ISTR_OFF ISTR_ON

#define ISTR_ALTEQ	         "Alt=" 
#define ISTR_TXEQ			       "\003Tx=Swr"
#define ISTR_RXEQ		       "Rx="
#define ISTR_TRE012AG	     "TRE012AG"

// ISTR_YELORGRED indexed 3 char each
#define ISTR_YELORGRED	     "\003---VagCamDo "
#define ISTR_A_EQ		       "A ="
#define ISTR_SOUNDS	       "\006Warn1 ""Warn2 ""Cheap ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Crickt""Siren ""AlmClk""Ratata""Tick  ""Haptc1""Haptc2""Haptc3" "Haptc4"
#define ISTR_SWITCH_WARN	   "Co cong tac dang mo!"
//#define ISTR_SWITCH				   "CO CONG" 
#define ISTR_WARNING			   "TAC DANG MO" 
// ISTR_TIMER exactly 5 chars long
#define ISTR_TIMER          "Timer"

// ISTR_PPMCHANNELS indexed 4 char each
#define ISTR_PPMCHANNELS	   "CH"

#define ISTR_MAH_ALARM      "mAh Alarm"


// er9x.cpp
// ********
//#define ISTR_LIMITS		     "QUA TAI"
#define ISTR_EE_LOW_MEM     "Bo nho EEPROM day"
#define ISTR_ALERT		      "CANHBAO"
#define ISTR_THR_NOT_IDLE   "Ga dang hoat dong"
#define ISTR_RST_THROTTLE   "Thiet lap lai ga"
#define ISTR_PRESS_KEY_SKIP "Nhan bat ki de bo qua"
#define ISTR_ALARMS_DISABLE "Bao dong da TAT"
#define ISTR_OLD_VER_EEPROM " Ban EEPROM da cu     K.TRA  CAI DAT/C.LAI"
#define ISTR_RESET_SWITCHES "Vui long tat cong tac"
#define ISTR_LOADING        "DANG TAI"
#define ISTR_MESSAGE        "TIN NHAN"
#define ISTR_PRESS_ANY_KEY  "Nhan phim bat ki"
#define ISTR_MSTACK_UFLOW   "Day mStack"
#define ISTR_MSTACK_OFLOW   "Doc mStack"

#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_CHANS_GV	     "\004P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#define ISTR_CHANS_EXTRA   "\004P4  P5  P6  P7  "
#endif
#ifdef PCBX9D
 #ifdef REV9E
#define ISTR_CHANS_GV	     "\004P1  P2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004P1  P2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#define ISTR_CHANS_EXTRA   "\004SR  S1  S2  P3  P4  "
 #else
  #ifdef PCBT12
#define ISTR_CHANS_GV	     "\004AUX4AUX5SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
  #else
#define ISTR_CHANS_GV	     "\004S1  S2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
  #endif
#define ISTR_CHANS_RAW	   "\004S1  S2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
	#if defined(PCBX7) || defined (PCBXLITE) || defined (PCBX3)
#define ISTR_CHANS_EXTRA   "\004SL  SR  P4  P5  P6  "
  #else
#define ISTR_CHANS_EXTRA   "\004SR  P3  P4  P5  P6  "
  #endif
 #endif
#endif

#if defined(PCBX12D) || defined(PCBX10)
#define ISTR_CHANS_GV	     "\004S1  S2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004S1  S2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#define ISTR_CHANS_EXTRA   "\004SR  P3  P4  "
#endif

#define ISTR_CH	           "CH"
#define ISTR_TMR_MODE	     "\003TATMO RUsRU%ELsEL%THsTH%ALsAL%P1 P1%P2 P2%P3 P3%"

// pers.cpp
// ********
#define ISTR_ME             "TOI       "
#define ISTR_MODEL          "MO HINH   "
#define ISTR_BAD_EEPROM     "Du lieu EEprom loi"
#define ISTR_EE_FORMAT      "Dang xoa EEPROM"
#define ISTR_GENWR_ERROR    "Loi ghi"
#define ISTR_EE_OFLOW       "Day EEPROM"

// templates.cpp
// ***********
#define ISTR_T_S_4CHAN      "4 Kenh Don"
#define ISTR_T_TCUT         "Cat Ga"
#define ISTR_T_STICK_TCUT   "Cat Ga - Mo An Toan"
#define ISTR_T_V_TAIL       "V-Tail"
#define ISTR_T_ELEVON       "Elevon\\Delta"
#define ISTR_T_HELI_SETUP   "Cai Dat Heli"
#define ISTR_T_GYRO         "Cai Dat Gyro"
#define ISTR_T_SERVO_TEST   "Kiem Tra Servo"
#define ISTR_T_RANGE_TEST   "Kiem Tra Range"

// menus.cpp
// ***********
#define ISTR_TELEM_ITEMS	  "\004----A1= A2= RSSITSSITim1Tim2Alt GaltGspdT1= T2= RPM FUELMah1Mah2CvltBattAmpsMah CtotFasVAccXAccYAccZVspdGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7FwatRxV Hdg A3= A4= SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 RTC TmOKAspdCel1Cel2Cel3Cel4Cel5Cel6RBv1RBa1RBv2RBa2RBm1RBm2RBSVRBSTCel7Cel8Cel9Cl10Cl11Cl12Cus1Cus2Cus3Cus4Cus5Cus6Fmd RunTModTCls1Cls2SbcVSbcA"
#define ISTR_TELEM_SHORT    "\004----TIM1TIM2BATTGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7"
#define ISTR_GV             "GV"
#define ISTR_OFF_ON         "TATMO "
#define ISTR_HYPH_INV       FWx18 "\001" "\003---INV"
#define ISTR_VERSION        "PHIEN BAN"
#define ISTR_Music	        "Music"
#define ISTR_SLAVE          "\007Slave" 
#define ISTR_MENU_DONE      "[MENU] HOAN THANH"
#define ISTR_CURVES         "CAC DO THI"
#define ISTR_CURVE          "DO THI"
#define ISTR_GLOBAL_VAR     "GLOBAL VAR"
#define ISTR_VALUE          "Gia tri"
#define ISTR_PRESET         "MAC DINH"
#define ISTR_CV             "CV"
#define ISTR_LIMITS         "GIOI HAN"
#define ISTR_COPY_TRIM      "SAO CHEP TRIM [MENU]"
#define ISTR_TELEMETRY      "DO TU XA "
#define ISTR_USR_PROTO      "UsrProto"
#define ISTR_FRHUB_WSHHI    "\006FrSky WSHhi DSMx  Jeti  ArduP ArduC FrHub HubRawFrMav Mavlk Hitec AFHDS2"
#define ISTR_MET_IMP        "\003MetImp"
#define ISTR_A_CHANNEL      "A  Kenh"
//#define ISTR_ALRM           "c.bao"
//#define ISTR_TELEMETRY2     "DO TU XA 2"
#define ISTR_TX_RSSIALRM    "TxRSSIalrm"
#define ISTR_NUM_BLADES     "Num Blades"
#define ISTR_ALT_ALARM      "AltAlarm"
#define ISTR_OFF122400      "\003TAT122400"
#define ISTR_VOLT_THRES     "Dien Ap gh="
#define ISTR_GPS_ALTMAIN    "GpsAltMain"
#define ISTR_CUSTOM_DISP    "C.Dat Hien Thi"
#define ISTR_FAS_OFFSET     "Bu lai FAS"
#define ISTR_VARIO_SRC      "Vario: Nguon"
#define ISTR_VSPD_A2        "\004----vspdA2  SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_2SWITCH        "\001Cong Tac"
#define ISTR_2SENSITIVITY   "\001Do Nhay"
#define ISTR_GLOBAL_VARS    "GLOBAL VARS"
#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 P3 c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#ifdef PCBX9D
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   "\003SR P3 P4 P5 P6 "
#endif
#if defined(PCBX12D) || defined(PCBX10)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#define ISTR_TEMPLATES      "MAU SAN CO"
#define ISTR_CHAN_ORDER     "Thu Tu Kenh"
#define ISTR_SP_RETA        " RETA"
#define ISTR_CLEAR_MIXES    "XOA HET MIX [MENU]"
#define ISTR_SAFETY_SW      "CONG TAC AN TOAN"
#define ISTR_NUM_VOICE_SW   "So A.Thanh CTac"
#define ISTR_V_OPT1         "\007 8 Giay12 Giay16 Giay"
#define ISTR_VS             "VS"
#define ISTR_VOICE_OPT      "\006MO    TAT   MO+TAT15Giay30Giay60Giay Khac "
#define ISTR_CUST_SWITCH    "CONG TAC LOGIC"
#define ISTR_S              "S"
#define ISTR_15_ON          "\015Mo"
#define ISTR_EDIT_MIX       "C.SUA MIX"
#define ISTR_2SOURCE        "\001Nguon"
#define ISTR_2WEIGHT        "\001Nang"
#define ISTR_FMTRIMVAL      "FmTrimVal"
#define ISTR_OFFSET         "Offset"
#define ISTR_2FIX_OFFSET    "\001Fix Offset"
#define ISTR_ENABLEEXPO     "\001EnableExpoDr"
#define ISTR_2TRIM          "\001Trim"
#define ISTR_15DIFF         "\010Diff"
#define ISTR_Curve          "Do thi"
#define ISTR_2WARNING       "\001Chu y"
#define ISTR_2MULTIPLEX     "\001GhepCH"
// ISTR_ADD_MULT_REP indexed 8 chars each
#define ISTR_ADD_MULT_REP   "\010Them VaoTang LenThay The"
#define ISTR_2DELAY_DOWN    "\001Tre Xuong"
#define ISTR_2DELAY_UP      "\001Tre Len"
#define ISTR_2SLOW_DOWN     "\001Xuong Cham"
#define ISTR_2SLOW_UP       "\001Len Cham"
#define ISTR_MAX_MIXERS     "Luong mix toi da: 32"
#define ISTR_PRESS_EXIT_AB  "An [EXIT] de huy bo"
#define ISTR_YES_NO         "\003DONG Y\013KHONG"
#define ISTR_MENU_EXIT      "\003[MENU]\013[EXIT]"
#define ISTR_DELETE_MIX     "XOA MIX?"
#define ISTR_MIX_POPUP      "CHINH SUA\0CHEN VAO\0SAO CHEP\0DI CHUYEN\0XOA\0XOA HET\0TEMPLATES\0PASTE"
#define ISTR_MIXER          "MIXER"
// CHR_S S for Slow
#define ICHR_S              "S"
// CHR_D D for Delay
#define ICHR_D              "D"
// CHR_d d for differential
#define ICHR_d              "d"
#define ISTR_EXPO_DR        "EXPO/DR"
#define ISTR_4DR_MID        "\004DR Mid"
#define ISTR_4DR_LOW        "\004DR Low"
#define ISTR_4DR_HI         "\004DR Hi"
#define ISTR_2EXPO          "\002Expo"
#define ISTR_DR_SW1         "DrSw1"
#define ISTR_DR_SW2         "DrSw2"
#define ISTR_DUP_MODEL      "S.CHEP MO HINH"
#define ISTR_DELETE_MODEL   "XOA MO HINH"
#define ISTR_DUPLICATING    "Dang s.chep mo hinh"
#define ISTR_SETUP          "C.Dat mo hinh"
#define ISTR_NAME           "Ten"
#define ISTR_VOICE_INDEX    "AmThanh\021MENU"
#define ISTR_TRIGGERA       "Trigger"
#define ISTR_TRIGGERB       "TriggerB"
//ISTR_COUNT_DOWN_UP indexed, 10 chars each
#define ISTR_COUNT_DOWN_UP  "\012Dem Xuong Dem Len   "
#define ISTR_T_TRIM         "Thr-Trim"
#define ISTR_T_EXPO         "T-Expo-Dr"
// ISTR_TRIM_OPTIONS indexed 6 chars each
#define ISTR_TRIM_OPTIONS   FWx9 "\004" "\006Exp   ExFineFine  MediumCoarse"
#define ISTR_TRIM_SWITCH    "Trim Sw"
#define ISTR_TRIM_INC       "Trim Inc"
#define ISTR_BEEP_CENTRE    "Beep Cnt"
#define ISTR_RETA123        "RETA1234"
#define ISTR_PROTO          "Proto"
// ISTR_21_USEC after \021 max 4 chars
#define ISTR_21_USEC        "\021uSec"
#define ISTR_13_RXNUM       "\014RxNum"
// ISTR_23_US after \023 max 2 chars
#define ISTR_23_US          "\023uS"
// ISTR_PPMFRAME_MSEC before \015 max 9 chars, after max 4 chars
#define ISTR_PPMFRAME_MSEC  "PPM FrLen\016mSec"
#define ISTR_SEND_RX_NUM    " Send Rx Number [MENU]"
#define ISTR_DSM_TYPE       " DSM Type"
#define ISTR_PPM_1ST_CHAN   "1st Chan"
#define ISTR_SHIFT_SEL      "Polarity"
// ISTR_POS_NEG indexed 3 chars each
#define ISTR_POS_NEG        "\003POSNEG"
#define ISTR_E_LIMITS       "E. Limits"
#define ISTR_Trainer        "Trainer"
#define ISTR_T2THTRIG       "T2ThTrig"
#define ISTR_AUTO_LIMITS    "Auto Limits"
// ISTR_1_RETA indexed 1 char each
#define ISTR_1_RETA         "\001RETA"
#define ISTR_FL_MODE        "C.DO BAY"
#define ISTR_SWITCH         "Cong tac"
#define ISTR_TRIMS          "Trims"
#define ISTR_MODES          "CHE DO"
#define ISTR_SP_FM0         " FM0"
#define ISTR_SP_FM          " FM"
#define ISTR_HELI_SETUP     "HELI C.DAT"
#define ISTR_SWASH_TYPE     "Swash Type"
#define ISTR_COLLECTIVE     "Collective"
#define ISTR_SWASH_RING     "Swash Ring"
#define ISTR_ELE_DIRECTION  "ELE Direction"
#define ISTR_AIL_DIRECTION  "AIL Direction"
#define ISTR_COL_DIRECTION  "COL Direction"
#define ISTR_HELI_TEXT			ISTR_SWASH_TYPE "\037" ISTR_COLLECTIVE "\037" ISTR_SWASH_RING "\037" ISTR_ELE_DIRECTION "\037" ISTR_AIL_DIRECTION "\037" ISTR_COL_DIRECTION
//#define ISTR_MODEL_POPUP    "SELECT\0COPY\0MOVE\0DELETE"
#define ISTR_MODEL_POPUP    "CHINH SUA\0CHON\0SAO CHEP\0DI CHUYEN\0XOA\0SAO LUU\0KHOI PHUC\0REPLACE\0NOTES"
#define ISTR_MODELSEL       "MODELSEL"
// ISTR_11_FREE after \011 max 4 chars
#define ISTR_11_FREE        "\011free"
#define ISTR_CALIBRATION    "CAN CHINH LAI"
// ISTR_MENU_TO_START after \003 max 15 chars
#define ISTR_MENU_TO_START  "\003[MENU] DE BAT DAU"
// ISTR_SET_MIDPOINT after \005 max 11 chars
#define ISTR_SET_MIDPOINT   "\005DIEM GIUA"
// ISTR_MOVE_STICKS after \003 max 15 chars
#define ISTR_MOVE_STICKS    "\003XE DICH STK/POT"
#define ISTR_ANA            "ANA"
#define ISTR_DIAG           "DIAG"
// ISTR_KEYNAMES indexed 5 chars each
#define ISTR_KEYNAMES       "\005 Menu ExitXuong  Len Phai Trai"
#define ISTR_TRIM_M_P       "Trim- +"
// ISTR_OFF_PLUS_EQ indexed 3 chars each
#define ISTR_OFF_PLUS_EQ    "\003off += :="
// ISTR_CH1_4 indexed 3 chars each
#define ISTR_CH1_4          "\003ch1ch2ch3ch4"
#define ISTR_MULTIPLIER     "Multiplier"
#define ISTR_CAL            "Cal"
#define ISTR_MODE_SRC_SW    "\003mode\012% src  sw"
#define ISTR_RADIO_SETUP    "CAI DAT TX"
#define ISTR_OWNER_NAME     "Chu So Huu"
#define ISTR_BEEPER         "Am Ban Phim"
// ISTR_BEEP_MODES indexed 6 chars each
#define ISTR_BEEP_MODES     "\006ImLang""NoKey ""R.Ngan""Ngan  ""Chuan ""Dai   ""R.Dai "
#define ISTR_SOUND_MODE     "Kieu A.Thanh"
// ISTR_SPEAKER_OPTS indexed 10 chars each
#define ISTR_SPEAKER_OPTS   "\012Beeper    ""PiSpkr    ""BeeprVoice""PiSpkVoice""MegaSound "
#define ISTR_VOLUME         "Am Luong"
#define ISTR_SPEAKER_PITCH  "Loa Pitch"
#define ISTR_HAPTICSTRENGTH "Cuong Do Haptic"
#define ISTR_CONTRAST       "Tuong Phan"
#define ISTR_BATT_WARN      "Bao PIN Yeu" 
// ISTR_INACT_ALARM m for minutes after \023 - single char
#define ISTR_INACT_ALARM    "Bao Khong Su Dung\023m"
#define ISTR_THR_REVERSE    "Dao Kenh Ga"
#define ISTR_MINUTE_BEEP    "Am Dem Phut"
#define ISTR_BEEP_COUNTDOWN "Am Dem Xuong"
#define ISTR_FLASH_ON_BEEP  "Am Khi Flash"
#define ISTR_LIGHT_SWITCH   "Cong Tac Den"
#define ISTR_LIGHT_INVERT   "Dao Den Nen"
#define ISTR_LIGHT_AFTER    "Den Khi An "
#define ISTR_LIGHT_STICK    "Den Di Stick"
#define ISTR_SPLASH_SCREEN  "Hinh Khi Mo TX"
#define ISTR_SPLASH_NAME    "Ten Khi Mo TX"
#define ISTR_THR_WARNING    "Chu Y Kenh Ga"
#define ISTR_DEAFULT_SW     "C.Tac Mac Dinh"
#define ISTR_MEM_WARN       "Chu Y Bo Nho"
#define ISTR_ALARM_WARN     "Chu Y Canh Bao"
#define ISTR_POTSCROLL      "POT D.Chuyen"
#define ISTR_STICKSCROLL    "STK D.Chuyen"
#define ISTR_BANDGAP        "BandGap"
#define ISTR_ENABLE_PPMSIM  "Mo PPMSIM"
#define ISTR_CROSSTRIM      "CrossTrim"
#define ISTR_INT_FRSKY_ALRM "Int. Frsky alarm"
#define ISTR_MODE           "Kieu"

// SWITCHES_STR 3 chars each
#if defined(PCBSKY) || defined(PCB9XT)
#define ISWITCHES_STR "\003THRRUDELEID0ID1ID2AILGEATRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfTH\200TH-TH\201RU\200RU-RU\201EL\200EL-EL\201AI\200AI-AI\201GE\200GE-GE\2016P06P16P26P36P46P5PB1PB2"
#define IHW_SWITCHES_STR     "\004sIDxsTHRsRUDsELEsAILsGEAsTRNL1  L2  L3  L4  L5  L6  L7  L8  L9  LA  LB  LC  LD  LE  LF  LG  LH  LI  LJ  LK  LL  LM  LN  LO  6POS"
//#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6P"
#define IHW_SWITCHARROW_STR  "\200-\201"
#endif
#ifdef PCBX9D
#ifdef REV9E
#define ISWITCHES_STR				 "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5"\
														 "SI\200SI-SI\201SJ\200SJ-SJ\201SK\200SK-SK\201SL\200SL-SL\201SM\200SM-SM\201SN\200SN-SN\201SO\200SO-SO\201SP\200SP-SP\201SQ\200SQ-SQ\201SR\200SR-SR\201"
#else
#ifdef PCBT12
#define ISWITCHES_STR "\003SG       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SF\200SF-SF\2016P06P16P26P36P46P5PB1PB2"
#else
#define ISWITCHES_STR "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5PB1PB2PB3"
#endif
#endif	// REV9E

#ifdef REV9E
#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSHSISJSKSLSMSNSOSPSQSR6PL1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"
#else
#ifdef PCBT12
#define IHW_SWITCHES_STR     "\002SASBSCSDSESGSFSH6PL1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"
#else
#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6PL1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"
#endif
#endif	// REV9E
#define IHW_SWITCHARROW_STR "\200-\201"
#endif
#if defined(PCBX12D) || defined(PCBX10)
#define ISWITCHES_STR				 "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5PB1PB2PB3PB4"
#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6PL1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"
#define IHW_SWITCHARROW_STR  "\200-\201"
#endif
#define ISWITCH_WARN_STR	   "Chu Y Cong Tac"
// CURV_STR indexed 3 chars each
// c17-c24 added for timer mode A display
#define ICURV_STR					 "\003---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24c25c26c27c28c29c30c31c32"
// CSWITCH_STR indexed 7 chars each
#define ICSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""Latch  F-Flop TimeOffNtmeOff1-Shot 1-ShotRv\140=val v&val  v1\140=v2 v=val  "

#define ISWASH_TYPE_STR     FWx17 "\004" "\004----""120 ""120X""140 ""90  "

#define ISTR_STICK_NAMES    "\005Rud \0Ele \0Thr \0Ail "

#define ISTR_STAT           "STAT"
#define ISTR_STAT2          "STAT2"
// ISTR_TRIM_OPTS indexed 3 chars each
#define ISTR_TRIM_OPTS      "\003ExpExFFneMedCrs"
#define ISTR_TTM            "TTm"
#define ISTR_FUEL           "Fuel"
#define ISTR_12_RPM         "\012RPM"
#define ISTR_LON_EQ         "Lon="
#define ISTR_ALT_MAX        "Alt=\011m   Max="
#define ISTR_SPD_KTS_MAX    "Spd=\011kts Max="
#define ISTR_LAT_EQ         "Lat=" "\037" ISTR_LON_EQ "\037" ISTR_ALT_MAX "\037" ISTR_SPD_KTS_MAX
#define ISTR_11_MPH         "\011mph"

#define ISTR_SINK_TONES	   "Sink"


// ersky9x strings
#define ISTR_ST_CARD_STAT   "THE NHO STAT"
#define ISTR_4_READY        "\004S.Sang"
#define ISTR_NOT            "NOT"
#define ISTR_BOOT_REASON    "BOOT REASON"
#define ISTR_6_WATCHDOG     "\006WATCHDOG"
#define ISTR_5_UNEXPECTED   "\005UNEXPECTED"
#define ISTR_6_SHUTDOWN     "\006TAT TX"
#define ISTR_6_POWER_ON     "\006MO TX"
// ISTR_MONTHS indexed 3 chars each
#define ISTR_MONTHS         "\003XxxT 1T 2T 3T 4T 5T 6T 7T 8T 9T10T11T12"
#define ISTR_MENU_REFRESH   "[MENU] de lam moi"
#define ISTR_DATE_TIME      "NGAY-GIO"
#define ISTR_SEC            "Giay:"
#define ISTR_MIN_SET        "Phut:\014C.DAT"
#define ISTR_HOUR_MENU_LONG "Gio :\012GIU MENU"
#define ISTR_DATE           "Ngay"
#define ISTR_MONTH          "Thang"
#define ISTR_YEAR_TEMP      "Nam\013N.Do:"
#define ISTR_YEAR           "Nam"
#define ISTR_BATTERY        "PIN"
#define ISTR_Battery        "Pin"
#define ISTR_CURRENT_MAX    "Current\016Max"
#define ISTR_CPU_TEMP_MAX   "N.Do CPU:\014C Max\024C"
#define ISTR_MEMORY_STAT    "MEMORY STAT"
#define ISTR_GENERAL        "Tong Quat"
#define ISTR_Model          "Mo Hinh"
#define ISTR_RADIO_SETUP2   "CAI DAT TX 2"
#define ISTR_BRIGHTNESS     "Do Sang"
#define ISTR_CAPACITY_ALARM "Canh Bao Capacity"
#define ISTR_BT_BAUDRATE    "T.Do Baud BT"
#define ISTR_ROTARY_DIVISOR "Nac Khi Xoay"
#define ISTR_LV             "LV"
#define ISTR_LH             "LH"
#define ISTR_RV             "RV"
#define ISTR_RH             "RH"
#define ISTR_STICK_GAIN     "Stick Gain"
#define ISTR_STICK_DEADBAND "Stick Deadband"
#define ISTR_STICK_LV_GAIN  "Stick LV Gain"
#define ISTR_STICK_LH_GAIN  "Stick LH Gain"
#define ISTR_STICK_RV_GAIN  "Stick RV Gain"
#define ISTR_STICK_RH_GAIN  "Stick RH Gain"
#define ISTR_BIND					  "Ghep"
#define ISTR_RANGE					"K.Tra P.Vi"

#define ISTR_ALRMS_OFF			"Tat Canh Bao"
#define ISTR_OLD_EEPROM			" Ban EEPROM da cu     K.TRA  CAI DAT/C.LAI"
#define ISTR_TRIGA_OPTS			"TATMO THsTH%"
#define ISTR_CHK_MIX_SRC		"K.TRA MIX SOURCES"

#define ISTR_BT_TELEMETRY		"BT Do Tu Xa"
#define ISTR_FRSKY_COM_PORT "Cong COM Telem"
#define ISTR_INVERT_COM1		"Dao COM 1"
#define ISTR_LOG_SWITCH			"Cong Tac Logic"
#define ISTR_LOG_RATE				"P.Vi Logic"
#define ISTR_6_BINDING			"\006DANG GHEP"
#define ISTR_RANGE_RSSI			"K.TRA P.VI RSSI:"
#define ISTR_FAILSAFE				"FAILSAFE"
#define ISTR_VOLUME_CTRL		"D.Chinh Am Luong"
//#ifdef ASSAN
//#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiAssan"
//#else
#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiXfireAcces"
//#endif
#define ISTR_TYPE						" Loai"
#define ISTR_COUNTRY				" Quoc Gia"
#define ISTR_SP_FAILSAFE		" Failsafe"
#define ISTR_PPM2_START			"PPM2 StartChan"
#define ISTR_FOLLOW					"Follow"
#define ISTR_PPM2_CHANNELS	"Cac Kenh PPM2"
#define ISTR_FILTER_ADC			"Bo Loc ADC"
#define ISTR_SCROLLING			"Di Chuyen"
#define ISTR_ALERT_YEL			"C.Bao [Vag]"
#define ISTR_ALERT_ORG			"C.Bao [Cam]"
#define ISTR_ALERT_RED			"C.Bao [Do ]"
#define ISTR_LANGUAGE				"Ngon Ngu"

#define ISTR_RSSI_WARN		  "Chu Y RSSI"
#define ISTR_RSSI_CRITICAL  "Gioi han RSSI"
#define ISTR_RX_VOLTAGE		  "Dien Ap Rx"
#define ISTR_DSM_WARNING	  "Chu Y DSM"
#define ISTR_FADESLOSSHOLDS "\006fades lossesholds "
#define ISTR_DSM_CRITICAL	  "Gioi Han DSM"
#define ISTR_BT_TRAINER		  "BT as Trainer"
#define ISTR_MULTI_TEXT     "Protocol\037Loai\037T.D Ghep\037P.Vi"
#define ISTR_MULTI_PROTO    "Protocol"
#define ISTR_MULTI_TYPE	    "Loai"
#define ISTR_MULTI_AUTO	    "T.D Ghep"
#define ISTR_MULTI_POWER    "P.Vi\013Rate\023mS"
#define ISTR_MULTI_OPTION   "\013T.Chon"

#define ISTR_Display		     "Display" 
#define ISTR_AudioHaptic		 "AudioHaptic" 
#define ISTR_Alarms			     "Alarms" 
#define ISTR_General		     "General" 
#define ISTR_Controls			   "Controls"
#define ISTR_Hardware			   "Hardware"
#define ISTR_Calibration		 "Calibration" 
//#define ISTR_Trainer		     "Trainer" 
#define ISTR_Version		     "Version" 
#define ISTR_ModuleRssi			 "FrSky xSSI"
#define ISTR_DateTime			   "Date-Time" 
#define ISTR_DiagSwtch		   "DiagSwtch"  
#define ISTR_DiagAna		     "DiagAna" 

#define ISTR_Mixer		      "Mixer" 
#define ISTR_Cswitches		  "L.Switches" 
#define ISTR_Telemetry		  "Telemetry" 
#define ISTR_limits			    "Limits" 
#define ISTR_Bluetooth		  "BlueTooth" 
#define ISTR_heli_setup			"Heli" 
#define ISTR_Curves			    "Curves" 
#define ISTR_Expo				    "Expo/D.Rate" 
#define ISTR_Globals		    "Globals" 
#define ISTR_Timer		      "Timers" 
#define ISTR_Modes			 		"Modes" 
#define ISTR_Voice		      "VoiceAudio" 
#define ISTR_Protocol			  "Protocol" 
#define ISTR_Safety					"Safety Sws"
#define ISTR_Eeprom			     "EEPROM" 

#define ISTR_MAIN_POPUP			"Model Select\0Model Setup\0Last Menu\0Radio Setup\0Statistics\0Notes\0Zero Alt.\0Zero A1 Offs\0Zero A2 Offs\0Reset GPS\0Help\0Main Display\0Run Script\0Reset Telemetry"
#define ISTR_ROTATE_SCREEN			"Rotate Screen"
#define ISTR_REVERSE_SCREEN			"Reverse Screen"
#define ISTR_MENU_ONLY_EDIT			"MENU only Edit"
#define ISTR_Voice_Alarm				"Voice Alarm"
#define ISTR_Voice_Alarms				"Voice Alarms"
#define ISTR_CUSTOM_CHECK				"Custom Check"
#define ISTR_CUSTOM_STK_NAMES		"CustomStkNames"
#define ISTR_HAPTIC							"Haptic"
#define ISTR_RESET_SWITCH				"Reset Switch"
#define ISTR_THROTTLE_OPEN			"Throttle Open"
#define ISTR_THR_DEFAULT				"Thr. Default"
#define ISTR_TOTAL_TIME					"Total Time"
#define ISTR_POPUP_GLOBALS			"GVARS\0GVadjusters\0Scalers\0Telemetry\0Custom\0Mixer\0Templates\0Logging\0Blocking\0Vario\0Sensors"

#define ISTR_SHUT_DOWN					"Shutting Down"

//"Current Source"
//"\004----A1  A2  FASVSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
//"SC  ="
//"Source"
//"Multiplier"
//"Divisor"
//"Unit"
//"Sign"
//"Decimals"
//"Offset At"
//"\005FirstLast "
//"Voice Switch"
//"Function"
//"\007----   v>val  v<val  |v|>val|v|<valON     OFF    BOTH   "
//"Switch"
//"Rate"
//"\017Once"
//"Offset"
//"FileType"
//"\006  NameNumberHaptic"
//"Voice File"
//"\006Haptc1Haptc2Haptc3"
// SKY "\003IDxTHRRUDELEAILGEATRN"
// X9D "\002SASBSCSDSESFSGSH"
//"\002MODES"
// SKY "\004sIDxsTHRsRUDsELEsAILsGEAsTRN"
// X9D "\002SASBSCSDSESFSGSH"
//"Reset Switch"
// SKY "\003---P1 P2 P3 GV4GV5GV6GV7"
// X9D "\003---P1 P2 SL SR GV5GV6GV7"
//"Internal"
//"\003AmeJapEur"
//"External"
//"Fade In"
//"Fade Out"
//"Name"
//"Co Proc"
//"On Time"
//"ttimer1        us"
//"\013rssi"
//"Vbat"
//"\013RxV"
//"AMP\013Temp"
//"RPM\021DSM2"
//"SETTINGS"
//"Display"
//"AudioHaptic"
//"Alarms"
//"General"
//"Controls"
//"Hardware"
//"Calibration"
//"Trainer"
//"Version"
//"Date-Time"
//"DiagSwtch"
//"DiagAna"
//"DISPLAY"
//"BLUE"
//"WHITE"
//"Optrex Display"
//"AUDIO/HAPTIC"
//"ALARMS"
//"[Next]"
//"GENERAL"
//"\012   ENGLISH  FRANCAIS   DEUTSCH NORWEGIAN   SWEDISH"
//"\005NONE POT  STICKBOTH "
//"CONTROLS"
//"HARDWARE"
//"ELE  switch"
//"\0042POS3POS6POS"
//"THR  switch"
//"\0042POS3POS"
//"RUD  switch"
//"GEAR switch"
//"MODEL SETTINGS"
//"Mixer"
//"C.Switches"
//"Telemetry"
//"Limits"
//"Display"
//"MDISPLAY"




