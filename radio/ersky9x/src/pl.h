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
 * - Dariusz Wocka
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
// ¹  use \300
// ¥  use \301
// æ  use \302
// Æ  use \303
// ê  use \304
// ³  use \305
// £  use \306
// ñ  use \307
// Ñ  use \310
// ó  use \311
// œ  use \312
// Œ  use \313
// ¿  use \314
// ¯  use \315
// Ÿ  use \316
//   use \317

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


#define ISTR_ON             " W\306"
#define ISTR_OFF            "WY\306"
#define ISTR_X_OFF_ON				FWx17 "\001" "\003" ISTR_OFF ISTR_ON

#define ISTR_ALTEQ	         "Wys=" 
#define ISTR_TXEQ			       "\003Tx=Swr"
#define ISTR_RXEQ		       "Rx="
#define ISTR_TRE012AG	     "TRE012AG"

// ISTR_YELORGRED indexed 3 char each
#define ISTR_YELORGRED	     "\003---\315\311\305PomCze"
#define ISTR_A_EQ		       "A ="
#define ISTR_SOUNDS	       "\006Warn1 ""Warn2 ""Cheap ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Crickt""Siren ""AlmClk""Ratata""Tick  ""Haptc1""Haptc2""Haptc3" "Haptc4"
#define ISTR_SWITCH_WARN	   "SPRAWD\317 PRZE\306\301CZNIKI"
//#define ISTR_SWITCH				   "PRZELACZNIKI"
#define ISTR_WARNING			    "SPRAWDZ"
// ISTR_TIMER exactly 5 chars long
#define ISTR_TIMER          "Stoper"

// ISTR_PPMCHANNELS indexed 4 char each
#define ISTR_PPMCHANNELS	   "CH"

#define ISTR_MAH_ALARM      "mAh Alarm"


// er9x.cpp
// ********
//#define ISTR_LIMITS		     "ZAKRESY"
#define ISTR_EE_LOW_MEM     "EEPROM ma\305o pam"
#define ISTR_ALERT		      " UWAGA"
#define ISTR_THR_NOT_IDLE   "GAZ NIE NA ZERZE !!"
#define ISTR_RST_THROTTLE   "Ustaw gaz na zero"
#define ISTR_PRESS_KEY_SKIP "Pomi\307-> wci\312nij jaki\312          przycisk"
#define ISTR_ALARMS_DISABLE "Alarmy wy\305\300czone"
#define ISTR_OLD_VER_EEPROM " Stara Wersia EEPROM   Sprawd\316 ustawienia"
#define ISTR_RESET_SWITCHES "Ustaw prze\305. na zero"
#define ISTR_LOADING        "\306aduj\304"
#define ISTR_MESSAGE        "INFO"
#define ISTR_PRESS_ANY_KEY  "wci\312nij przycisk"
#define ISTR_MSTACK_UFLOW   "Braki w MENU"
#define ISTR_MSTACK_OFLOW   "Pe\305ne MENU"

#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_CHANS_GV	     "\004P1  P2  P3  P\311\305 Ca\305yCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004P1  P2  P3  P\311\305 Ca\305yCYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#define ISTR_CHANS_EXTRA   "\004P4  P5  P6  P7  "
#endif
#ifdef PCBX9D
 #ifdef REV9E
#define ISTR_CHANS_GV	     "\004P1  P2  SL  P\311\305 Ca\305yCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004P1  P2  SL  P\311\305 Ca\305yCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#define ISTR_CHANS_EXTRA   "\004SR  S1  S2  P3  P4  "
 #else
  #ifdef PCBT12
#define ISTR_CHANS_GV	     "\004AUX4AUX5SL  P\311\305 Ca\305yCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
  #else
#define ISTR_CHANS_GV	     "\004S1  S2  SL  P\311\305 Ca\305yCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
  #endif
#define ISTR_CHANS_RAW	   "\004S1  S2  SL  P\311\305 Ca\305yCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
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
#define ISTR_TMR_MODE	     "\003WY\306W\306 RUsRU%ELsEL%THsTH%ALsAL%P1 P1%P2 P2%P3 P3%"

#if defined(PCBLEM1)
#define ISTR_CHANS_GV	     "\004S1  S2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004S1  S2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#endif

// pers.cpp
// ********
#define ISTR_ME             "JA        "
#define ISTR_MODEL          "MODEL     "
#define ISTR_BAD_EEPROM     "Z\305y EEprom Data"
#define ISTR_EE_FORMAT      "EEPROM Format    "
#define ISTR_GENWR_ERROR    "B\305\300d zapisu"
#define ISTR_EE_OFLOW       "EEPROM pe\305ny"

// templates.cpp
// ***********
#define ISTR_T_S_4CHAN      "Proste 4-CH"
#define ISTR_T_TCUT         "T-Cut"
#define ISTR_T_STICK_TCUT   "Sticky T-Cut"
#define ISTR_T_V_TAIL       "Ogon-V"
#define ISTR_T_ELEVON       "Prze\305\\Delta"
#define ISTR_T_HELI_SETUP   "Ustaw.Heli"
#define ISTR_T_GYRO         "Ustaw.Gyro"
#define ISTR_T_SERVO_TEST   "Test Servo"
#define ISTR_T_RANGE_TEST   "Test Zasi\304gu"

// menus.cpp
// ***********
#define ISTR_TELEM_ITEMS	  "\004----A1= A2= RSSITSSIStp1Stp2Wys GwysPr\304dT1= T2= RPM FUELPoj1Poj2CvltBat.Amp.Poj.CtotOdbVAccXAccYAccZVspdGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7Moc.RxV KierA3= A4= SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 RTC TmOKAspdCel1Cel2Cel3Cel4Cel5Cel6RBv1RBa1RBv2RBa2RBm1RBm2RBSVRBSTCel7Cel8Cel9Cl10Cl11Cl12Cus1Cus2Cus3Cus4Cus5Cus6Fmd RunTModTCls1Cls2SbcVSbcA"
#define ISTR_TELEM_SHORT    "\004----TIM1TIM2BATTGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7"
#define ISTR_GV             "ZG"
#define ISTR_OFF_ON         "WY\306 W\306"
#define ISTR_HYPH_INV       FWx18 "\001" "\003---INV"
#define ISTR_VERSION        "WERSJA"
#define ISTR_Music	        "Music"
#define ISTR_SLAVE          "\007Drugi" 
#define ISTR_MENU_DONE      "  [MENU]-> DALEJ"
#define ISTR_CURVES         "KRZYWE"
#define ISTR_CURVE          "KRZYWA"
#define ISTR_GLOBAL_VAR     "FUNKCJE GLOBALNE"
#define ISTR_VALUE          "Warto\312\302"
#define ISTR_PRESET         "USTAWIENIA"
#define ISTR_CV             "KR"
#define ISTR_LIMITS         "ZAKRESY"
#define ISTR_COPY_TRIM      "Kopi\311j TRYM [MENU]"
#define ISTR_TELEMETRY      "TELEMETRIA"
#define ISTR_USR_PROTO      "ProtUrzyt"
#define ISTR_FRHUB_WSHHI    "\006FrSky WSHhi DSMx  Jeti  ArduP ArduC FrHub HubRawFrMav Mavlk Hitec AFHDS2"
#define ISTR_MET_IMP        "\003MetAng"
#define ISTR_A_CHANNEL      "A  kana\305"
//#define ISTR_ALRM           "alrm"
//#define ISTR_TELEMETRY2     "TELEMETRIA2"
#define ISTR_TX_RSSIALRM    "TxRSSIalrm"
#define ISTR_NUM_BLADES     "Ilo\312\302 \306opat"
#define ISTR_ALT_ALARM      "Alarm Wys."
#define ISTR_OFF122400      "\003WY\306122400"
#define ISTR_VOLT_THRES     "Pr\311g Nap.="
#define ISTR_GPS_ALTMAIN    "Wysoko\312\302 GPS"
#define ISTR_CUSTOM_DISP    "Ustaw Ekran nr."
#define ISTR_FAS_OFFSET     "FAS Offset"
#define ISTR_VARIO_SRC      "Vario: \317r\311d\305o"
#define ISTR_VSPD_A2        "\004----vspdA2  SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_2SWITCH        "\001Prze\305."
#define ISTR_2SENSITIVITY   "\001Czu\305o\312\302  "
#define ISTR_GLOBAL_VARS    "ODCHYLENIE"
#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENSk Sw GazLotP1 P2 P3 c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#ifdef PCBX9D
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENSk Sw GazLotP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   "\003WejP3 P4 P5 P6 "
#endif
#if defined(PCBX12D) || defined(PCBX10)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#define ISTR_TEMPLATES      "SZABLONY"
#define ISTR_CHAN_ORDER     "Kolejno\312\302 kana\305."
#define ISTR_SP_RETA        " RETA"
#define ISTR_CLEAR_MIXES    "KASUJ MIXY [MENU]"
#define ISTR_SAFETY_SW      "WY\306\301CZNIKI BEZPIECZ."
#define ISTR_NUM_VOICE_SW   "Nr.Prz.Dzwi\304ku"
#define ISTR_V_OPT1         "\007 8 Secs12 Secs16 Secs"
#define ISTR_VS             "PG"
#define ISTR_VOICE_OPT      "\006W\306    WY\306   BOTH  15Secs30Secs60SecsVaribl"
#define ISTR_CUST_SWITCH    "PRZE\306\301CZNIKI LOGICZNE"
#define ISTR_S              "S"
#define ISTR_15_ON          "\015W\305"
#define ISTR_EDIT_MIX       "EDYTUJ MIX"
#define ISTR_2SOURCE        "\001\317r\311d\305o"
#define ISTR_2WEIGHT        "\001Waga"
#define ISTR_FMTRIMVAL      "FmTrimVal"
#define ISTR_OFFSET         "Offset"
#define ISTR_2FIX_OFFSET    "\001Fix Offset"
#define ISTR_ENABLEEXPO     "\001W\305\300cz ExpoDr"
#define ISTR_2TRIM          "\001Trym"
#define ISTR_15DIFF         "\010R\311\314nicowe"
#define ISTR_Curve          "Krzywe"
#define ISTR_2WARNING       "\001Uwaga"
#define ISTR_2MULTIPLEX     "\001Multpx"
// ISTR_ADD_MULT_REP indexed 8 chars each
#define ISTR_ADD_MULT_REP   "\010Dodaj   Powiel  Zamie\307  "
#define ISTR_2DELAY_DOWN    "\001Op\311\316n. w D\311\305"
#define ISTR_2DELAY_UP      "\001Op\311\316n. w G\311r\304"
#define ISTR_2SLOW_DOWN     "\001Wolno w D\311\305"
#define ISTR_2SLOW_UP       "\001Wolno w G\311r\304"
#define ISTR_MAX_MIXERS     "Max zakres mixera: 32"
#define ISTR_PRESS_EXIT_AB  "Naci\312nij [EXIT] -> Przerwij"
#define ISTR_YES_NO         "\003TAK\013NIE"
#define ISTR_MENU_EXIT      "\003[MENU]\013[EXIT]"
#define ISTR_DELETE_MIX     "USU\310 MIX?"
#define ISTR_MIX_POPUP      "EDYCJA\0ZAWARTO\313\303\0KOPIUJ\0PRZENIE\313\0USU\310\0KASUJ WSZYSTKO\0TEMPLATES\0PASTE"
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
#define ISTR_DUPLICATING    "Kopiowanie modelu"
#define ISTR_SETUP          "Ustawienia modelu"
#define ISTR_NAME           "Nazwa"
#define ISTR_VOICE_INDEX    "G\305os \021MENU"
#define ISTR_TRIGGERA       "Stan   "
#define ISTR_TRIGGERB       "Spust  "
//ISTR_COUNT_DOWN_UP indexed, 10 chars each
#define ISTR_COUNT_DOWN_UP  "\012Licz w d\311\305Licz w g\311r"
#define ISTR_T_TRIM         "Trymer Gazu"
#define ISTR_T_EXPO         "Gaz-Expo-Dr"
// ISTR_TRIM_OPTIONS indexed 6 chars each
#define ISTR_TRIM_OPTIONS   FWx9 "\004" "\006Exp   PrecyzDok\305ad\313redniNiedok"
#define ISTR_TRIM_SWITCH    "Prze\305.Trymera"
#define ISTR_TRIM_INC       "Dok\305. Trymera"
#define ISTR_BEEP_CENTRE    "Sygna\305 Cnt"
#define ISTR_RETA123        "RETA1234"
#define ISTR_PROTO          "Proto"
// ISTR_21_USEC after \021 max 4 chars
#define ISTR_21_USEC        "\021uSek"
#define ISTR_13_RXNUM       "\014RxNum"
// ISTR_23_US after \023 max 2 chars
#define ISTR_23_US          "\023uS"
// ISTR_PPMFRAME_MSEC before \015 max 9 chars, after max 4 chars
#define ISTR_PPMFRAME_MSEC  "PPM FrLen\016mSek"
#define ISTR_SEND_RX_NUM    " Wy\312lij Rx Numer [MENU]"
#define ISTR_DSM_TYPE       " DSM Typ "
#define ISTR_PPM_1ST_CHAN   "1-y Kan."
#define ISTR_SHIFT_SEL      "Polaryz."
// ISTR_POS_NEG indexed 3 chars each
#define ISTR_POS_NEG        "\003POZNEG"
#define ISTR_E_LIMITS       "Wi\304ksze Zakresy"
#define ISTR_Trainer        "Trener"
#define ISTR_T2THTRIG       "T2ThTrig"
#define ISTR_AUTO_LIMITS    "Auto Zakres"
// ISTR_1_RETA indexed 1 char each
#define ISTR_1_RETA         "\001RETA"
#define ISTR_FL_MODE        "TRybLotu"
#define ISTR_SWITCH         "Prze\305."
#define ISTR_TRIMS          "Trymy"
#define ISTR_MODES          "TRYBY LOTU"
#define ISTR_SP_FM0         " TL0"
#define ISTR_SP_FM          " TL"
#define ISTR_HELI_SETUP     "USTAWIENIA HELI."
#define ISTR_SWASH_TYPE     "Swash Type"
#define ISTR_COLLECTIVE     "Collective"
#define ISTR_SWASH_RING     "Swash Ring"
#define ISTR_ELE_DIRECTION  "ELE_DIRECTION"
#define ISTR_AIL_DIRECTION  "AIL_DIRECTION"
#define ISTR_COL_DIRECTION  "COL_DIRECTION"
#define ISTR_HELI_TEXT			ISTR_SWASH_TYPE  "\037"  ISTR_COLLECTIVE  "\037"  ISTR_SWASH_RING  "\037"  ISTR_ELE_DIRECTION  "\037"  ISTR_AIL_DIRECTION  "\037"  ISTR_COL_DIRECTION
//#define ISTR_MODEL_POPUP    "WYBIERZ\0KOPIUJ\0ZOBACZ\0USU\310"
#define ISTR_MODEL_POPUP    "EDYTUJ\0WYBIERZ\0KOPIUJ\0PRZENIE\313\0USU\310\0KOPIA\0PRZYWR\311\303\0REPLACE\0NOTES"
#define ISTR_MODELSEL       "Wyb\311r modelu"
// ISTR_11_FREE after \011 max 4 chars
#define ISTR_11_FREE        "\011free"
#define ISTR_CALIBRATION    "KALIBRACJA"
// ISTR_MENU_TO_START after \003 max 15 chars
#define ISTR_MENU_TO_START  "\003[MENU] -> START"
// ISTR_SET_MIDPOINT after \005 max 11 chars
#define ISTR_SET_MIDPOINT   "\005USTAW W \313RODKU"
// ISTR_MOVE_STICKS after \003 max 15 chars
#define ISTR_MOVE_STICKS    "\003 RUSZAJ DR\301\315KAMI"
#define ISTR_ANA            "DR\301"
#define ISTR_DIAG           "DIAGNOST."
// ISTR_KEYNAMES indexed 5 chars each
#define ISTR_KEYNAMES       "\005 Menu Exit  D\311\305 G\311raPrawy Lewy"
#define ISTR_TRIM_M_P       "Trym- +"
// ISTR_OFF_PLUS_EQ indexed 3 chars each
#define ISTR_OFF_PLUS_EQ    "\003wy\305 += :="
// ISTR_CH1_4 indexed 3 chars each
#define ISTR_CH1_4          "\003ch1ch2ch3ch4"
#define ISTR_MULTIPLIER     "Multiplier"
#define ISTR_CAL            "Kal."
#define ISTR_MODE_SRC_SW    "\003tryb\012% \316r\311 prz"
#define ISTR_RADIO_SETUP    "USTAWIENIA RADIA"
#define ISTR_OWNER_NAME     "W\305a\312ciciel"
#define ISTR_BEEPER         "Sygna\305"
// ISTR_BEEP_MODES indexed 6 chars each
#define ISTR_BEEP_MODES     "\006Cicho ""  Brak""B.kr\311t""Kr\311tki""Normal""D\305ugi ""Bd\305ugi"
#define ISTR_SOUND_MODE     "Tryb d\316wi\304ku"
// ISTR_SPEAKER_OPTS indexed 10 chars each
#define ISTR_SPEAKER_OPTS   "\012Sygna\305    ""PiSpkr    ""BeeprVoice""PiSpkVoice""MegaSound "
#define ISTR_VOLUME         "Reg.G\305o\312no\312\302i"
#define ISTR_SPEAKER_PITCH  "Wysoko\312\302 ton\311w"
#define ISTR_HAPTICSTRENGTH "Si\305a wibracji"
#define ISTR_CONTRAST       "Kontrast"
#define ISTR_BATT_WARN      "Alarm baterii" 
// ISTR_INACT_ALARM m for minutes after \023 - single char
#define ISTR_INACT_ALARM    "AlarmBrakuAktyw. \023m"
#define ISTR_THR_REVERSE    "Odwr\311\302 GAZ"
#define ISTR_MINUTE_BEEP    "Sygna\305 co minut\304"
#define ISTR_BEEP_COUNTDOWN "Sygna\305 odliczania"
#define ISTR_FLASH_ON_BEEP  "B\305ysk na Sygna\305"
#define ISTR_LIGHT_SWITCH   "\313wiat\305o na prze\305."
#define ISTR_LIGHT_INVERT   "Pod\31wietlenie inv."
#define ISTR_LIGHT_AFTER    "\313wiat\305o na przyc."
#define ISTR_LIGHT_STICK    "\313wiat\305o na dr\300\314ek"
#define ISTR_SPLASH_SCREEN  "Ekran Startowy"
#define ISTR_SPLASH_NAME    "Nazwa Startowa"
#define ISTR_THR_WARNING    "Gaz Ostrze\314enie"
#define ISTR_DEAFULT_SW     "Domy\312lne Prze\305"
#define ISTR_MEM_WARN       "Pami\304\303 Uwaga"
#define ISTR_ALARM_WARN     "Alarm Ostrze\314enie"
#define ISTR_POTSCROLL      "Przes\311wanie Potencj."
#define ISTR_STICKSCROLL    "Przes\311wanie Dr. Gazu"
#define ISTR_BANDGAP        "Zakres Przerwy"
#define ISTR_ENABLE_PPMSIM  "Wy\305\300cz PPMSIM"
#define ISTR_CROSSTRIM      "CrossTrim"
#define ISTR_INT_FRSKY_ALRM "Alarmy Frsky "
#define ISTR_MODE           "Tryb"

// SWITCHES_STR 3 chars each
#if defined(PCBSKY) || defined(PCB9XT)
#define ISWITCHES_STR "\003THRRUDELEID\200ID-ID\201AILGEATRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfTH\200TH-TH\201RU\200RU-RU\201EL\200EL-EL\201AI\200AI-AI\201GE\200GE-GE\2016P06P16P26P36P46P5PB1PB2PB3PB4"
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
#define ISWITCH_WARN_STR	   "UWAGA Wy\305\300czniki"
// CURV_STR indexed 3 chars each
// c17-c24 added for timer mode A display
#define ICURV_STR					 "\003---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24c25c26c27c28c29c30c31c32"
// CSWITCH_STR indexed 7 chars each
#define ICSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""Latch  F-Flop TimeOffNtmeOff1-Shot 1-ShotRv\140=val v&val  v1\140=v2 v=val  "

#define ISWASH_TYPE_STR     FWx17 "\004" "\004----""120 ""120X""140 ""90  "

#define ISTR_STICK_NAMES    "\005Sk  \0Sw  \0Gaz \0Lot "

#define ISTR_STAT           "STATYST."
#define ISTR_STAT2          "STATYSTYKA2"
// ISTR_TRIM_OPTS indexed 3 chars each
#define ISTR_TRIM_OPTS      "\003ExpExFFneMedCrs"
#define ISTR_TTM            "TrymGazu"
#define ISTR_FUEL           "Paliwo"
#define ISTR_12_RPM         "\012RPM"
#define ISTR_LON_EQ         "D\305U="
#define ISTR_ALT_MAX        "Wys=\011m   Max="
#define ISTR_SPD_KTS_MAX    "Pr\304=\011kts Max="
#define ISTR_LAT_EQ         "Sze=" "\037" ISTR_LON_EQ "\037" ISTR_ALT_MAX "\037" ISTR_SPD_KTS_MAX
#define ISTR_11_MPH         "\011mph"

#define ISTR_SINK_TONES	   "G\305os Opadania"


// ersky9x strings
#define ISTR_ST_CARD_STAT   "STATYST. KARTY PAM."
#define ISTR_4_READY        "\004Gotowy"
#define ISTR_NOT            "NIE"
#define ISTR_BOOT_REASON    "Spos\311b Uruchom.  "
#define ISTR_6_WATCHDOG     "\006PRZYCZYNA"
#define ISTR_5_UNEXPECTED   "\002NIEWYJA\31NIONY"
#define ISTR_6_SHUTDOWN     "\006ZAMYKANIE"
#define ISTR_6_POWER_ON     "\004 ZA\306\301CZENIE"
// ISTR_MONTHS indexed 3 chars each
#define ISTR_MONTHS         "\003XxxStyLutMarKwiMajCzeLipSieWrzPa\316LisGru"
#define ISTR_MENU_REFRESH   "[MENU] od\312wie\314"
#define ISTR_DATE_TIME      "DATA-CZAS"
#define ISTR_SEC            "Sek."
#define ISTR_MIN_SET        "Min.\015Ustaw"
#define ISTR_HOUR_MENU_LONG "Godz.\015[MENU]"
#define ISTR_DATE           "Data"
#define ISTR_MONTH          "Mies."
#define ISTR_YEAR_TEMP      "Rok\013Temp."
#define ISTR_YEAR           "Rok"
#define ISTR_BATTERY        "BATERIA"
#define ISTR_Battery        "Bateria"
#define ISTR_CURRENT_MAX    "Krzywa\016Max"
#define ISTR_CPU_TEMP_MAX   "CPU temp.\014C Max\024C"
#define ISTR_MEMORY_STAT    "STATYST. Pami\304ci"
#define ISTR_GENERAL        "G\305\311wne"
#define ISTR_Model          "Model"
#define ISTR_RADIO_SETUP2   "USTAWIENIA 2"
#define ISTR_BRIGHTNESS     "Jasno\312\302"
#define ISTR_CAPACITY_ALARM "Alarm pojemno\312ci"
#define ISTR_BT_BAUDRATE    "Bt pr\304dko\312\302"
#define ISTR_ROTARY_DIVISOR "Rotary Divisor"
#define ISTR_LV             "LV"
#define ISTR_LH             "LH"
#define ISTR_RV             "RV"
#define ISTR_RH             "RH"
#define ISTR_STICK_GAIN     "Stick Gain"
#define ISTR_STICK_DEADBAND "Dr\300\314ek Deadband"
#define ISTR_STICK_LV_GAIN  "Dr\300\314ek LV Gain"
#define ISTR_STICK_LH_GAIN  "Dr\300\314ek LH Gain"
#define ISTR_STICK_RV_GAIN  "Dr\300\314ek RV Gain"
#define ISTR_STICK_RH_GAIN  "Dr\300\314ek RH Gain"
#define ISTR_BIND					  "Po\305\300cz"
#define ISTR_RANGE					"Sprawd\316 zakres"

#define ISTR_ALRMS_OFF			"Alarmy Wy\305\300czone"
#define ISTR_OLD_EEPROM			" Stara Wersja EEPROM   Sprawd\316 ustawienia"
#define ISTR_TRIGA_OPTS			"WY\306W\306 THsTH%"
#define ISTR_CHK_MIX_SRC		"Sprawd\316 \317r311d\305a MIX"

#define ISTR_BT_TELEMETRY		"BT Telemetria"
#define ISTR_FRSKY_COM_PORT "Telem. Com Port"
#define ISTR_INVERT_COM1		"Invert COM 1"
#define ISTR_LOG_SWITCH			"Prze\305 Log"
#define ISTR_LOG_RATE				"Warto\312\302Log\311w"
#define ISTR_6_BINDING			"\006PO\306\301CZENIE"
#define ISTR_RANGE_RSSI			"Sprawd\316 zakres RSSI:"
#define ISTR_FAILSAFE				"FAILSAFE"
#define ISTR_VOLUME_CTRL		"G\305o\312no\312\302"
//#ifdef ASSAN
//#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiAssan"
//#else
#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiXfireAcces"
//#endif
#define ISTR_TYPE						" Typ"
#define ISTR_COUNTRY				" Kraj"
#define ISTR_SP_FAILSAFE		" Failsafe"
#define ISTR_PPM2_START			"PPM2 Start Kan"
#define ISTR_FOLLOW					"\313LED\316"
#define ISTR_PPM2_CHANNELS	"PPM2 Kana\305y"
#define ISTR_FILTER_ADC			"Filtr ADC"
#define ISTR_SCROLLING			"Przesuwanie"
#define ISTR_ALERT_YEL			"Alert [\315\311\305]"
#define ISTR_ALERT_ORG			"Alert [Pom]"
#define ISTR_ALERT_RED			"Alert [Cze]"
#define ISTR_LANGUAGE				"J\304zyk"

#define ISTR_RSSI_WARN		  "RSSI Uwaga"
#define ISTR_RSSI_CRITICAL  "RSSI Krytyczne"
#define ISTR_RX_VOLTAGE		  "Rx Napi\304cie"
#define ISTR_DSM_WARNING	  "DSM Uwaga"
#define ISTR_FADESLOSSHOLDS "\006fades lossesholds "
#define ISTR_DSM_CRITICAL	  "DSM Krytyczne"
#define ISTR_BT_TRAINER		  "BT jako trener"
#define ISTR_MULTI_TEXT     "Protok\311\305\037Typ \037Autobind\037Zasilanie"
#define ISTR_MULTI_PROTO    "Protok\311\305"
#define ISTR_MULTI_TYPE	    "Typ"
#define ISTR_MULTI_AUTO	    "Autobind"
#define ISTR_MULTI_POWER    "Zasilanie\013Rate\023mS"
#define ISTR_MULTI_OPTION   "\013Opcje "

#define ISTR_Display		     "Ekran" 
#define ISTR_AudioHaptic		 "Audio/Wibr" 
#define ISTR_Alarms			     "Alarmy" 
#define ISTR_General		     "G\305\311wne" 
#define ISTR_Controls			   "Kontrola"
#define ISTR_Hardware			   "Sprz\304t"
#define ISTR_Calibration		 "Kalibracja" 
//#define ISTR_Trainer		     "Trener" 
#define ISTR_Version		     "Wersja" 
#define ISTR_ModuleRssi			 "FrSky xSSI"
#define ISTR_DateTime			   "Data-Czas" 
#define ISTR_DiagSwtch		   "DiagPrze\305"  
#define ISTR_DiagAna		     "DiagDr\300\314k" 

#define ISTR_Mixer		      "Mixer" 
#define ISTR_Cswitches		  "Prze\305 Log." 
#define ISTR_Telemetry		  "Telemetria" 
#define ISTR_limits			    "Zakresy" 
#define ISTR_Bluetooth		  "BlueTooth" 
#define ISTR_heli_setup			"Helikopter" 
#define ISTR_Curves			    "Krzywe" 
#define ISTR_Expo				    "Expo/D.Rate" 
#define ISTR_Globals		    "Globalne" 
#define ISTR_Timer		      "Stoper" 
#define ISTR_Modes			 		"Tryby Lotu" 
#define ISTR_Voice		      "D\316wi\304kAudio" 
#define ISTR_Protocol			  "Protok\311\305" 
#define ISTR_Safety					"Wy\305 Bezp."
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

//"Aktualne \317r\311d\305o"
//"\004----A1  A2  FASVSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
//"SC  ="
//"\317r\311d\305o"
//"Mno\314nik"
//"Dzielnik"
//"Jednostka"
//"Sygn"
//"Dzie\312\304tne"
//"Offset At"
//"\005PierwszyOstatni "
//"Prze\805 G\305os "
//"Funkcje"
//"\007----   v>val  v<val  |v|>val|v|<valW\306     WY\306    BOTH   "
//"PRZE\306\301CZNIK"
//"Ocena"
//"\017Raz "
//"Offset"
//"FileType"
//"\006  NazwaNumerWibr."
//"Pliki G\305osu"
//"\006Wibr.1Wibr.2Wibr.3"
// SKY "\003IDxTHRRUDELEAILGEATRN"
// X9D "\002SASBSCSDSESFSGSH"
//"\002TRYBY"
// SKY "\004sIDxsTHRsRUDsELEsAILsGEAsTRN"
// X9D "\002SASBSCSDSESFSGSH"
//"Kasuj Prze\305\300cznik"
// SKY "\003---P1 P2 P3 GV4GV5GV6GV7"
// X9D "\003---P1 P2 SL SR GV5GV6GV7"
//"Wewn\304trzny"
//"\003AmeJapEur"
//"Zewn\304trzny"
//"Wi\304cej"
//"Mniej"
//"Nazwa"
//"Co Proc"
//"W\805. Czas"
//"tstoper1        us"
//"\013rssi"
//"Vbat"
//"\013RxV"
//"AMP\013Temp"
//"RPM\021DSM2"
//"USTAWIENIA"
//"EKRAN"
//"AudioDrganie"
//"Alarmy"
//"G\305\311wne"
//"Kontrola"
//"Sprz\304t"
//"Kalibracja"
//"Trener"
//"Wersja"
//"Data-Czas"
//"DiagPrze\305"
//"DiagDr\300\314k"
//"EKRAN"
//"NIEBIESKI"
//"BIA\306Y"
//"Optrex EKRAN"
//"AUDIO/WIBRACJA"
//"ALARMY"
//"[Nast\304pny]"
//"G\305\311wne"
//"\012   ENGLISH  FRANCAIS   DEUTSCH NORWEGIAN   SWEDISH  ITALIAN   POLISH"
//"\005NIE  POT  DR\301\315ERAZEM"
//"KONTROLA"
//"Sprz\304t"
//"ELE Prze\300\300cznik"
//"\0042POZ3POZ6POZ"
//"THR Prze\305\300cznik"
//"\0042POZ3POZ"
//"RYD Prze\305\300cznik"
//"GEAR Prze\305\300cznik"
//"USTAWIENIA MODELI"
//"Mixer"
//"C.Prze\305\300czniki"
//"Telemtria"
//"Zakresy"
//"EKRAN"
//"MEKRAN"
//"\001Min czas wibracji"










