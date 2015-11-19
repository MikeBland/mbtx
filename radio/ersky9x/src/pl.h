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

// Special characters:
// ¹  use \800
// ¥  use \801
// æ  use \802
// Æ  use \803
// ê  use \804
// ³  use \805
// £  use \806
// ñ  use \807
// Ñ  use \808
// ó  use \809
// œ  use \810
// Œ  use \811
// ¿  use \812
// ¯  use \813
// Ÿ  use \814
//   use \815

#define FWx4		"\030"
#define FWx5		"\036"
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

#define ISTR_X_OFF_ON				FWx17"\001""\003"ISTR_OFF ISTR_ON

#define ISTR_ON             " W806"
#define ISTR_OFF            "WY806"

#define ISTR_ALTEQ	         "Wys=" 
#define ISTR_TXEQ			       "\003Tx=Swr"
#define ISTR_RXEQ		       "Rx="
#define ISTR_TRE012AG	     "TRE012AG"

// ISTR_YELORGRED indexed 3 char each
#define ISTR_YELORGRED	     "\003---813809805PomCze"
#define ISTR_A_EQ		       "A ="
#define ISTR_SOUNDS	       "\006Warn1 ""Warn2 ""Cheap ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Crickt""Siren ""AlmClk""Ratata""Tick  ""Haptc1""Haptc2""Haptc3"
#define ISTR_SWITCH_WARN	   "Prze805800czniki UWAGA"
// ISTR_TIMER exactly 5 chars long
#define ISTR_TIMER          "Stoper"

// ISTR_PPMCHANNELS indexed 4 char each
#define ISTR_PPMCHANNELS	   "CH"

#define ISTR_MAH_ALARM      "mAh Alarm"


// er9x.cpp
// ********
//#define ISTR_LIMITS		     "ZAKRESY"
#define ISTR_EE_LOW_MEM     "EEPROM ma805o pam"
#define ISTR_ALERT		      " UWA813AJ"
#define ISTR_THR_NOT_IDLE   "Gaz nie na zerze"
#define ISTR_RST_THROTTLE   "Kasuj Gaz"
#define ISTR_PRESS_KEY_SKIP "Dalej-> wci810nij przycisk"
#define ISTR_ALARMS_DISABLE "Alarmy wy805800czone"
#define ISTR_OLD_VER_EEPROM " Stara Wersia EEPROM   Sprawd814 ustawienia"
#define ISTR_RESET_SWITCHES "Prosz804 skasowa802 WY806"
#define ISTR_LOADING        "806aduj804"
#define ISTR_MESSAGE        "WIADOMO811803"
#define ISTR_PRESS_ANY_KEY  "wci810nij guzik"
#define ISTR_MSTACK_UFLOW   "mStack uflow"
#define ISTR_MSTACK_OFLOW   "mStack oflow"

#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_CHANS_GV	     "\004P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGV1 GV2 GV3 GV4 GV5 GV6 GV7 THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#endif
#ifdef PCB9XT
#define ISTR_CHANS_EXTRA   "\004P4  P5  P6  P7  "
#endif
#ifdef PCBX9D
#define ISTR_CHANS_GV	     "\004P1  P2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGV1 GV2 GV3 GV4 GV5 GV6 GV7 THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004P1  P2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#define ISTR_CHANS_EXTRA   "\004SR  P3  P4  P5  P6  "
#endif

#define ISTR_CH	           "CH"
#define ISTR_TMR_MODE	     "\003OFFON RUsRU%ELsEL%THsTH%ALsAL%P1 P1%P2 P2%P3 P3%"

// pers.cpp
// ********
#define ISTR_ME             "JA        "
#define ISTR_MODEL          "MODEL     "
#define ISTR_BAD_EEPROM     "Z805y EEprom Data"
#define ISTR_EE_FORMAT      "EEPROM Formatowanie"
#define ISTR_GENWR_ERROR    "N805800d zapisu"
#define ISTR_EE_OFLOW       "EEPROM overflow"

// templates.cpp
// ***********
#define ISTR_T_S_4CHAN      "Simple 4-CH"
#define ISTR_T_TCUT         "T-Cut"
#define ISTR_T_STICK_TCUT   "Sticky T-Cut"
#define ISTR_T_V_TAIL       "V-Tail"
#define ISTR_T_ELEVON       "Ele\\Delta"
#define ISTR_T_HELI_SETUP   "Heli Setup"
#define ISTR_T_GYRO         "Gyro Setup"
#define ISTR_T_SERVO_TEST   "Servo Test"
#define ISTR_T_RANGE_TEST   "Range Test"

// menus.cpp
// ***********
#define ISTR_TELEM_ITEMS	  "\004----A1= A2= RSSITSSITim1Tim2Alt GaltGspdT1= T2= RPM FUELMah1Mah2CvltBattAmpsMah CtotFasVAccXAccYAccZVspdGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7FwatRxV Hdg A3= A4= SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 RTC TmOK"
#define ISTR_TELEM_SHORT    "\004----TIM1TIM2BATTGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7"
#define ISTR_GV             "GV"
#define ISTR_OFF_ON         "WY806 W806"
#define ISTR_HYPH_INV       FWx18"\001""\003---INV"
#define ISTR_VERSION        "WERSJA"
#define ISTR_TRAINER        "TRENER"
#define ISTR_SLAVE          "\007Slave" 
#define ISTR_MENU_DONE      "[MENU] PO ZAKO808CZENIU"
#define ISTR_CURVES         "KRZYWE"
#define ISTR_CURVE          "KRZYWA"
#define ISTR_GLOBAL_VAR     "FUNKCJE GLOBALNE"
#define ISTR_VALUE          "Warto810802"
#define ISTR_PRESET         "USTAWIENIA"
#define ISTR_CV             "CV"
#define ISTR_LIMITS         "ZAKRESY"
#define ISTR_COPY_TRIM      "Kopi809j TRIM [MENU]"
#define ISTR_TELEMETRY      "TELEMETRIA"
#define ISTR_USR_PROTO      "UsrProto"
#define ISTR_FRHUB_WSHHI    "\005FrSkyWSHhiDSMx Jeti MavlkArduP"
#define ISTR_MET_IMP        "\003MetImp"
#define ISTR_A_CHANNEL      "A  kana805"
#define ISTR_ALRM           "alrm"
#define ISTR_TELEMETRY2     "TELEMETRIA2"
#define ISTR_TX_RSSIALRM    "TxRSSIalrm"
#define ISTR_NUM_BLADES     "Num Blades"
#define ISTR_ALT_ALARM      "Alarm Wys"
#define ISTR_OFF122400      "\003OFF122400"
#define ISTR_VOLT_THRES     "Volt Thres="
#define ISTR_GPS_ALTMAIN    "Wysoko810802 GPS"
#define ISTR_CUSTOM_DISP    "Ustawienia Ekranu"
#define ISTR_FAS_OFFSET     "FAS Offset"
#define ISTR_VARIO_SRC      "Vario: 815rud805o"
#define ISTR_VSPD_A2        "\004----vspdA2  SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_2SWITCH        "\001Prze805800cznik"
#define ISTR_2SENSITIVITY   "\001Czu805o810802"
#define ISTR_GLOBAL_VARS    "GLOBAL VARS"
#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 P3 c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24RtsEtsTtsAts"
#define ISTR_EXTRA_SOURCE   ""
#endif
#ifdef PCBX9D
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24RtsEtsTtsAts"
#define ISTR_EXTRA_SOURCE   "\003SR P3 P4 P5 P6 "
#endif
#define ISTR_TEMPLATES      "SZABLONY"
#define ISTR_CHAN_ORDER     "Channel Order"
#define ISTR_SP_RETA        " RETA"
#define ISTR_CLEAR_MIXES    "KASUJ MIXES [MENU]"
#define ISTR_SAFETY_SW      "SAFETY SWITCHES"
#define ISTR_NUM_VOICE_SW   "Number Voice Sw"
#define ISTR_V_OPT1         "\007 8 Secs12 Secs16 Secs"
#define ISTR_VS             "VS"
#define ISTR_VOICE_OPT      "\006ON    OFF   BOTH  15Secs30Secs60SecsVaribl"
#define ISTR_CUST_SWITCH    "LOGICAL SWITCHES"
#define ISTR_S              "S"
#define ISTR_15_ON          "\015On"
#define ISTR_EDIT_MIX       "EDYTUJ MIX "
#define ISTR_2SOURCE        "\001815r809d805o"
#define ISTR_2WEIGHT        "\001Weight"
#define ISTR_FMTRIMVAL      "FmTrimVal"
#define ISTR_OFFSET         "Offset"
#define ISTR_2FIX_OFFSET    "\001Fix Offset"
#define ISTR_ENABLEEXPO     "\001EnableExpoDr"
#define ISTR_2TRIM          "\001Trim"
#define ISTR_15DIFF         "\010Diff"
#define ISTR_Curve          "Krzywa"
#define ISTR_2WARNING       "\001Uwaga"
#define ISTR_2MULTIPLEX     "\001Multpx"
// ISTR_ADD_MULT_REP indexed 8 chars each
#define ISTR_ADD_MULT_REP   "\010Add     MultiplyReplace "
#define ISTR_2DELAY_DOWN    "\001Op809814 D809805"
#define ISTR_2DELAY_UP      "\001Op809813 G809ra"
#define ISTR_2SLOW_DOWN     "\001Wolno D809805"
#define ISTR_2SLOW_UP       "\001Wolno G809ra"
#define ISTR_MAX_MIXERS     "Max zakres mixera: 32"
#define ISTR_PRESS_EXIT_AB  "Naci810nij [EXIT] -> Przerwij"
#define ISTR_YES_NO         "\003TAK\013NIE"
#define ISTR_MENU_EXIT      "\003[MENU]\013[EXIT]"
#define ISTR_DELETE_MIX     "USU808 MIX?"
#define ISTR_MIX_POPUP      "EDYCJA\0ZAWARTO810802\0KOPIUJ\0ZOBACZ\0USU808\0KASUJ WSZYSTKO"
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
#define ISTR_DUP_MODEL      "KOPIUJ MODEL"
#define ISTR_DELETE_MODEL   "USU808 MODEL"
#define ISTR_DUPLICATING    "Kopiowa802 model"
#define ISTR_SETUP          "Model Setup"
#define ISTR_NAME           "Nazwa"
#define ISTR_VOICE_INDEX    "G805os\021MENU"
#define ISTR_TRIGGERA       "Trigger"
#define ISTR_TRIGGERB       "TriggerB"
//ISTR_COUNT_DOWN_UP indexed, 10 chars each
#define ISTR_COUNT_DOWN_UP  "\012Count DownCount Up  "
#define ISTR_T_TRIM         "T-Trim"
#define ISTR_T_EXPO         "T-Expo-Dr"
#define ISTR_TRIM_INC       "Trim Inc""\037"ISTR_TRIM_SWITCH
// ISTR_TRIM_OPTIONS indexed 6 chars each
#define ISTR_TRIM_OPTIONS   FWx14"\004""\006Exp   ExFineFine  MediumCoarse"
#define ISTR_TRIM_SWITCH    "Trim Sw"
#define ISTR_BEEP_CENTRE    "Beep Cnt"
#define ISTR_RETA123        "RETA1234"
#define ISTR_PROTO          "Proto"
// ISTR_21_USEC after \021 max 4 chars
#define ISTR_21_USEC        "\021uSec"
#define ISTR_13_RXNUM       "\014RxNum"
// ISTR_23_US after \023 max 2 chars
#define ISTR_23_US          "\023uS"
// ISTR_PPMFRAME_MSEC before \015 max 9 chars, after max 4 chars
#define ISTR_PPMFRAME_MSEC  " PPM FrLen\015mSec"
#define ISTR_SEND_RX_NUM    " Wy810lij Rx Numer [MENU]"
#define ISTR_DSM_TYPE       " DSM Type"
#define ISTR_PPM_1ST_CHAN   " 1st Chan"
#define ISTR_SHIFT_SEL      " Polaryzacja"
// ISTR_POS_NEG indexed 3 chars each
#define ISTR_POS_NEG        "\003POSNEG"
#define ISTR_E_LIMITS       "E. Zakres"
#define ISTR_Trainer        "Trener"
#define ISTR_T2THTRIG       "T2ThTrig"
#define ISTR_AUTO_LIMITS    "Auto Zakres"
// ISTR_1_RETA indexed 1 char each
#define ISTR_1_RETA         "\001RETA"
#define ISTR_FL_MODE        "FL MODE"
#define ISTR_SWITCH         "Switch"
#define ISTR_TRIMS          "Trims"
#define ISTR_MODES          "MODES"
#define ISTR_SP_FM0         " FM0"
#define ISTR_SP_FM          " FM"
#define ISTR_HELI_SETUP     "HELI SETUP"
#define ISTR_HELI_TEXT			ISTR_SWASH_TYPE"\037"ISTR_COLLECTIVE"\037"ISTR_SWASH_RING"\037"ISTR_ELE_DIRECTION"\037"ISTR_AIL_DIRECTION"\037"ISTR_COL_DIRECTION
#define ISTR_SWASH_TYPE     "Swash Type"
#define ISTR_COLLECTIVE     "Collective"
#define ISTR_SWASH_RING     "Swash Ring"
#define ISTR_ELE_DIRECTION  "SW Kierunek"
#define ISTR_AIL_DIRECTION  "LOT Kierunek"
#define ISTR_COL_DIRECTION  "COL Kieerunek"
//#define ISTR_MODEL_POPUP    "WYBIERZ\0KOPIUJ\0ZOBACZ\0USU808"
#define ISTR_MODEL_POPUP    "EDYTUJ\0WUBIERZ\0KOPIUJ\0ZOBACZ\0USU808\0KOPIA\0PRZYWRO803"
#define ISTR_MODELSEL       "MODELSEL"
// ISTR_11_FREE after \011 max 4 chars
#define ISTR_11_FREE        "\011free"
#define ISTR_CALIBRATION    "KALIBRACJA"
// ISTR_MENU_TO_START after \003 max 15 chars
#define ISTR_MENU_TO_START  "\003[MENU] TO START"
// ISTR_SET_MIDPOINT after \005 max 11 chars
#define ISTR_SET_MIDPOINT   "\005SET MIDPOINT"
// ISTR_MOVE_STICKS after \003 max 15 chars
#define ISTR_MOVE_STICKS    "\003MOVE STICKS/POTS"
#define ISTR_ANA            "ANA"
#define ISTR_DIAG           "DIAG"
// ISTR_KEYNAMES indexed 5 chars each
#define ISTR_KEYNAMES       "\005 Menu Exit Down   UpRight Left"
#define ISTR_TRIM_M_P       "Trim- +"
// ISTR_OFF_PLUS_EQ indexed 3 chars each
#define ISTR_OFF_PLUS_EQ    "\003off += :="
// ISTR_CH1_4 indexed 3 chars each
#define ISTR_CH1_4          "\003ch1ch2ch3ch4"
#define ISTR_MULTIPLIER     "Multiplier"
#define ISTR_CAL            "Cal"
#define ISTR_MODE_SRC_SW    "\003mode\012% src  sw"
#define ISTR_RADIO_SETUP    "USTAWIENIA RADIA"
#define ISTR_OWNER_NAME     "W805a810ciciel"
#define ISTR_BEEPER         "Sygna805"
// ISTR_BEEP_MODES indexed 6 chars each
#define ISTR_BEEP_MODES     "\006Cicho ""NoKey ""b.kru809tki""kru809tki""Normal""d805ugi""b.d805ugi"
#define ISTR_SOUND_MODE     "Rodzaj dzi804ku"
// ISTR_SPEAKER_OPTS indexed 10 chars each
#define ISTR_SPEAKER_OPTS   "\012Beeper    ""PiSpkr    ""BeeprVoice""PiSpkVoice""MegaSound "
#define ISTR_VOLUME         "G805o810no810802"
#define ISTR_SPEAKER_PITCH  " Speaker Pitch"
#define ISTR_HAPTICSTRENGTH " Haptic Strength"
#define ISTR_CONTRAST       "Kontrast"
#define ISTR_BATT_WARN      "Alarm baterii" 
// ISTR_INACT_ALARM m for minutes after \023 - single char
#define ISTR_INACT_ALARM    "Nieaktywny alarm\023m"
#define ISTR_THR_REVERSE    "Gaz odwr809802"
#define ISTR_MINUTE_BEEP    "Beep co minut804"
#define ISTR_BEEP_COUNTDOWN "Beep odliczanie"
#define ISTR_FLASH_ON_BEEP  "B805skanie na beep"
#define ISTR_LIGHT_SWITCH   "811wiat³o Wy805"
#define ISTR_LIGHT_INVERT   "811wiat³o invert"
#define ISTR_LIGHT_AFTER    "811wiat³o on key"
#define ISTR_LIGHT_STICK    "811wiat³o on Stk Mv"
#define ISTR_SPLASH_SCREEN  "Ekran Startowy"
#define ISTR_SPLASH_NAME    "Nazwa Startowa"
#define ISTR_THR_WARNING    "Gaz Ostrze812enie"
#define ISTR_DEAFULT_SW     "Default Sw"
#define ISTR_MEM_WARN       "Pami804803 Uwaga"
#define ISTR_ALARM_WARN     "Alarm Ostrze812enie"
#define ISTR_POTSCROLL      "Przewijanie Potencj."
#define ISTR_STICKSCROLL    "Przewijanie Dz. Gazu"
#define ISTR_BANDGAP        "BandGap"
#define ISTR_ENABLE_PPMSIM  "Wy805801cz PPMSIM"
#define ISTR_CROSSTRIM      "CrossTrim"
#define ISTR_INT_FRSKY_ALRM "Int. Frsky alarm"
#define ISTR_MODE           "Mode"

// SWITCHES_STR 3 chars each
#if defined(PCBSKY) || defined(PCB9XT)
#define ISWITCHES_STR "\003THRRUDELEID0ID1ID2AILGEATRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfTH\200TH-TH\201RU\200RU-RU\201EL\200EL-EL\201AI\200AI-AI\201GE\200GE-GE\2016P06P16P26P36P46P5PB1PB2"
//#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6P"
#define IHW_SWITCHARROW_STR  "\200-\201"
#endif
#ifdef PCBX9D
#ifdef REV9E
#define ISWITCHES_STR				 "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5"\
														 "SI\200SI-SI\201SJ\200SJ-SJ\201SK\200SK-SK\201SL\200SL-SL\201SM\200SM-SM\201SN\200SN-SN\201SO\200SO-SO\201SP\200SP-SP\201SQ\200SQ-SQ\201SR\200SR-SR\201"
#else
#define ISWITCHES_STR "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5"
#endif	// REV9E

#ifdef REV9E
#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSHSISJSKSLSMSNSOSPSQSR6PL1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"
#else
#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6PL1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"
#endif	// REV9E
#define IHW_SWITCHARROW_STR "\200-\201"
#endif
#define ISWITCH_WARN_STR	   "UWAGA Wy805800czniki"
// CURV_STR indexed 3 chars each
// c17-c24 added for timer mode A display
#define ICURV_STR					 "\003---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24"
// CSWITCH_STR indexed 7 chars each
#define ICSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""Latch  F-Flop TimeOffNtmeOff1-Shot 1-ShotRv\140=val"

#define ISWASH_TYPE_STR     FWx17"\004""\004----""120 ""120X""140 ""90  "

#define ISTR_STICK_NAMES    "\005Rud \0Ele \0Thr \0Ail "

#define ISTR_STAT           "STAT"
#define ISTR_STAT2          "STAT2"
// ISTR_TRIM_OPTS indexed 3 chars each
#define ISTR_TRIM_OPTS      "\003ExpExFFneMedCrs"
#define ISTR_TTM            "TTm"
#define ISTR_FUEL           "Paliwo"
#define ISTR_12_RPM         "\012RPM"
#define ISTR_LAT_EQ         "Lat=""\037"ISTR_LON_EQ"\037"ISTR_ALT_MAX"\037"ISTR_SPD_KTS_MAX
#define ISTR_LON_EQ         "Lon="
#define ISTR_ALT_MAX        "Wys=\011m   Max="
#define ISTR_SPD_KTS_MAX    "Pr804=\011kts Max="
#define ISTR_11_MPH         "\011mph"

#define ISTR_SINK_TONES	   "Sink Tones"


// ersky9x strings
#define ISTR_ST_CARD_STAT   "SD CARD STAT"
#define ISTR_4_READY        "\004Gotowy"
#define ISTR_NOT            "NIE"
#define ISTR_BOOT_REASON    "BOOT REASON"
#define ISTR_6_WATCHDOG     "\006WATCHDOG"
#define ISTR_5_UNEXPECTED   "\005UNEXPECTED"
#define ISTR_6_SHUTDOWN     "\006WY806801CZAM"
#define ISTR_6_POWER_ON     "\006W806801CZAM"
// ISTR_MONTHS indexed 3 chars each
#define ISTR_MONTHS         "\003XxxStyLutMarKwiMajCzeLipSieWrzPa814LisGru"
#define ISTR_MENU_REFRESH   "[MENU] od810wierz"
#define ISTR_DATE_TIME      "DATA-CZAS"
#define ISTR_SEC            "Sec."
#define ISTR_MIN_SET        "Min.\015Set"
#define ISTR_HOUR_MENU_LONG "Godz\012MENU LONG"
#define ISTR_DATE           "Data"
#define ISTR_MONTH          "Miesi800c"
#define ISTR_YEAR_TEMP      "Rok\013Temp."
#define ISTR_YEAR           "Rok"
#define ISTR_BATTERY        "BATERIA"
#define ISTR_Battery        "Bateria"
#define ISTR_CURRENT_MAX    "Krzywa\016Max"
#define ISTR_CPU_TEMP_MAX   "CPU temp.\014C Max\024C"
#define ISTR_MEMORY_STAT    "MEMORY STAT"
#define ISTR_GENERAL        "General"
#define ISTR_Model          "Model"
#define ISTR_RADIO_SETUP2   "RADIO SETUP2"
#define ISTR_BRIGHTNESS     "Jasno810802"
#define ISTR_CAPACITY_ALARM "Capacity Alarm"
#define ISTR_BT_BAUDRATE    "Bt baudrate"
#define ISTR_ROTARY_DIVISOR "Rotary Divisor"
#define ISTR_STICK_LV_GAIN  "Stick LV Gain"
#define ISTR_STICK_LH_GAIN  "Stick LH Gain"
#define ISTR_STICK_RV_GAIN  "Stick RV Gain"
#define ISTR_STICK_RH_GAIN  "Stick RH Gain"
#define ISTR_BIND					  " Bind"
#define ISTR_RANGE					" Range Check"

#define ISTR_ALRMS_OFF			"Alarmy Wy805800czone"
#define ISTR_OLD_EEPROM			" Stara Wersja EEPROM   Sprawd814 ustawienia"
#define ISTR_TRIGA_OPTS			"OFFON THsTH%"
#define ISTR_CHK_MIX_SRC		"CHECK MIX SOURCES"

#define ISTR_BT_TELEMETRY		"BT Telemetria"
#define ISTR_FRSKY_COM_PORT "FrSky Com Port"
#define ISTR_INVERT_COM1		"Invert COM 1"
#define ISTR_LOG_SWITCH			"Log Switch"
#define ISTR_LOG_RATE				"Log Rate"
#define ISTR_6_BINDING			"\006BINDING"
#define ISTR_RANGE_RSSI			"RANGE CHECK RSSI:"
#define ISTR_FAILSAFE				"FAILSAFE"
#define ISTR_VOLUME_CTRL		"G8050810no810802"
#define ISTR_PROT_OPT				"\005PPM  PXX  DSM2 MultiAssan"
#define ISTR_TYPE						" Typ"
#define ISTR_COUNTRY				" Kraj"
#define ISTR_SP_FAILSAFE		" Failsafe"
#define ISTR_PPM2_START			"PPM2 StartChan"
#define ISTR_FOLLOW					"Follow"
#define ISTR_PPM2_CHANNELS	"PPM2 Channels"
#define ISTR_FILTER_ADC			"Filter ADC"
#define ISTR_SCROLLING			"Scrolling"
#define ISTR_ALERT_YEL			"Uwa812aj [Yel]"
#define ISTR_ALERT_ORG			"Uwa812aj [Org]"
#define ISTR_ALERT_RED			"Uwa812aj [Red]"
#define ISTR_LANGUAGE				"J804zyk"

#define ISTR_RSSI_WARN		  "RSSI Warn"
#define ISTR_RSSI_CRITICAL  "RSSI Critical"
#define ISTR_RX_VOLTAGE		  "Rx Napi804cie"
#define ISTR_DSM_WARNING	  "DSM Uwaga"
#define ISTR_FADESLOSSHOLDS "\006fades lossesholds "
#define ISTR_DSM_CRITICAL	  "DSM Krytyczne"
#define ISTR_BT_TRAINER		  "BT as Trener"
#define ISTR_MULTI_TYPE     "Protocol\037Type\037Autobind\037Power"
#define ISTR_MULTI_OPTION   "\014Option"

#define ISTR_Display		     "Ekran" 
#define ISTR_AudioHaptic		 "AudioHaptic" 
#define ISTR_Alarms			     "Alarms" 
#define ISTR_General		     "General" 
#define ISTR_Controls			   "Controls"
#define ISTR_Hardware			   "Hardware"
#define ISTR_Calibration		 "Kalibracja" 
//#define ISTR_Trainer		     "Trener" 
#define ISTR_Version		     "Wersja" 
#define ISTR_ModuleRssi			 "FrSky xSSI"
#define ISTR_DateTime			   "Data-Czas" 
#define ISTR_DiagSwtch		   "DiagSwtch"  
#define ISTR_DiagAna		     "DiagAna" 

#define ISTR_Mixer		      "Mixer" 
#define ISTR_Cswitches		  "L.Switches" 
#define ISTR_Telemetry		  "Telemetria" 
#define ISTR_limits			    "Zakresy" 
#define ISTR_Bluetooth		  "BlueTooth" 
#define ISTR_heli_setup			"Heli" 
#define ISTR_Curves			    "Krzywa" 
#define ISTR_Expo				    "Expo/Dr" 
#define ISTR_Globals		    "Globals" 
#define ISTR_Timer		      "Stopery" 
#define ISTR_Modes			 		"Modes" 
#define ISTR_Voice		      "G³os" 
#define ISTR_Protocol			  "Protok809805" 
#define ISTR_Safety					"Safety Sws"


//"Aktualne 815r809d805o"
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
//"G805os Switch"
//"Funkcje"
//"\007----   v>val  v<val  |v|>val|v|<valW806     WY806    BOTH   "
//"PRZE806801CZNIK"
//"Rate"
//"\017Once"
//"Offset"
//"FileType"
//"\006  NazwaNumerHaptic"
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
//"Nazwa"
//"Co Proc"
//"On Time"
//"ttimer1        us"
//"\013rssi"
//"Vbat"
//"\013RxV"
//"AMP\013Temp"
//"RPM\021DSM2"
//"USTAWIENIA"
//"EKRAN"
//"AudioHaptic"
//"Alarmy"
//"G805809wne"
//"Kontrola"
//"Hardware"
//"Kalibracja"
//"Trener"
//"Wersja"
//"Data-Czas"
//"DiagSwtch"
//"DiagAna"
//"EKRAN"
//"NIEBIESKI"
//"BIA806Y"
//"Optrex EKRAN"
//"AUDIO/HAPTIC"
//"ALARMY"
//"[Nast804pny]"
//"GENERAL"
//"\012   ENGLISH  FRANCAIS   DEUTSCH NORWEGIAN   SWEDISH  POLISH"
//"\005NONE POT  STICKBOTH "
//"KONTROLA"
//"HARDWARE"
//"SW Prze805800cznik"
//"\0042POS3POS6POS"
//"GAZ Prze805800cznik"
//"\0042POS3POS"
//"SK Prze805800cznik"
//"GAZ Prze805800cznik"
//"USTAWIENIA MODELI"
//"Mixer"
//"C.Prze805800czniki"
//"Telemtria"
//"Zakresy"
//"EKRAN"
//"MEKRAN"





