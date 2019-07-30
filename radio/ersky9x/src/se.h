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
 * - Kjell Kernen
****************************************************************************/

// Special characters:
// å  use \300
// ä  use \301
// ö  use \302
// Å  use \303
// Ä  use \304
// Ö  use \305 

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


#define ISTR_ON              "P\303 "
#define ISTR_OFF             "AV "
#define ISTR_X_OFF_ON				FWx17 "\001" "\003" ISTR_OFF ISTR_ON

#define ISTR_ALTEQ           "Hjd=" 
#define ISTR_TXEQ			       "\003Tx=Swr"
#define ISTR_RXEQ            "Rx="
#define ISTR_TRE012AG        "TRE012AG"

// ISTR_YELORGRED indexed 3 char each
#define ISTR_YELORGRED       "\003---GulOrgRod"
#define ISTR_A_EQ            "A ="
#define ISTR_SOUNDS	       "\006Warn1 ""Warn2 ""Cheap ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Crickt""Siren ""AlmClk""Ratata""Tick  ""Haptc1""Haptc2""Haptc3" "Haptc4"
#define ISTR_SWITCH_WARN     "BrytarVarning"
//#define ISTR_SWITCH			     "Brytar"
#define ISTR_WARNING		     "Varning"
// ISTR_TIMER exactly 5 chars long
#define ISTR_TIMER           "Timer"

// ISTR_PPMCHANNELS indexed 4 char each
#define ISTR_PPMCHANNELS     "KN"

#define ISTR_MAH_ALARM       "mAh Alarm"


// er9x.cpp
// ********
#define ISTR_LIMITS          "GR\304NSER"
#define ISTR_EE_LOW_MEM      "EEPROM fullt"
#define ISTR_ALERT           "VARNING"
#define ISTR_THR_NOT_IDLE    "Gas ej avslagen"
#define ISTR_RST_THROTTLE    "Nollst\301ll gas"
#define ISTR_PRESS_KEY_SKIP  "Knapptryck forts\301tter"
#define ISTR_ALARMS_DISABLE  "Alarm Avslagna"
#define ISTR_OLD_VER_EEPROM  " Gammal EEPROM-ver.   KOLLA INST\304LLNINGAR"
#define ISTR_RESET_SWITCHES  "Nollst\301ll Brytarna"
#define ISTR_LOADING         "LADDAR "
#define ISTR_MESSAGE         "INFO"
#define ISTR_PRESS_ANY_KEY   "tryck ned Knapp"
#define ISTR_MSTACK_UFLOW    "mStack uflow"
#define ISTR_MSTACK_OFLOW    "mStack oflow"

#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_CHANS_GV        "\004P1  P2  P3  HALVFULLCYK1CYK2CYK3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8KN1 KN2 KN3 KN4 KN5 KN6 KN7 KN8 KN9 KN10KN11KN12KN13KN14KN15KN16KN17KN18KN19KN20KN21KN22KN23KN24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW       "\004P1  P2  P3  HALVFULLCYK1CYK2CYK3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8KN1 KN2 KN3 KN4 KN5 KN6 KN7 KN8 KN9 KN10KN11KN12KN13KN14KN15KN16KN17KN18KN19KN20KN21KN22KN23KN24SWCH"
#define ISTR_CHANS_EXTRA   "\004P4  P5  P6  P7  "
#endif
#ifdef PCBX9D
 #ifdef REV9E
#define ISTR_CHANS_GV        "\004P1  P2  SL  HALVFULLCYK1CYK2CYK3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8KN1 KN2 KN3 KN4 KN5 KN6 KN7 KN8 KN9 KN10KN11KN12KN13KN14KN15KN16KN17KN18KN19KN20KN21KN22KN23KN24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW       "\004P1  P2  SL  HALVFULLCYK1CYK2CYK3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8KN1 KN2 KN3 KN4 KN5 KN6 KN7 KN8 KN9 KN10KN11KN12KN13KN14KN15KN16KN17KN18KN19KN20KN21KN22KN23KN24SWCH"
#define ISTR_CHANS_EXTRA   "\004SR  S1  S2  P3  P4  "
 #else
  #ifdef PCBT12
#define ISTR_CHANS_GV        "\004AUX4AUX5SL  HALVFULLCYK1CYK2CYK3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8KN1 KN2 KN3 KN4 KN5 KN6 KN7 KN8 KN9 KN10KN11KN12KN13KN14KN15KN16KN17KN18KN19KN20KN21KN22KN23KN24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
  #else
#define ISTR_CHANS_GV        "\004S1  S2  SL  HALVFULLCYK1CYK2CYK3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8KN1 KN2 KN3 KN4 KN5 KN6 KN7 KN8 KN9 KN10KN11KN12KN13KN14KN15KN16KN17KN18KN19KN20KN21KN22KN23KN24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
  #endif
#define ISTR_CHANS_RAW       "\004S1  S2  SL  HALVFULLCYK1CYK2CYK3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8KN1 KN2 KN3 KN4 KN5 KN6 KN7 KN8 KN9 KN10KN11KN12KN13KN14KN15KN16KN17KN18KN19KN20KN21KN22KN23KN24SWCH"
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

#define ISTR_CH              "KN"
#define ISTR_TMR_MODE        "\003OFFON SRUsRU%ELsEL%THsTH%ALsAL%P1 P1%P2 P2%P3 P3%"

// pers.cpp
// ********
#define ISTR_ME              "DITT NAMN "
#define ISTR_MODEL           "MODELL    "
#define ISTR_BAD_EEPROM      "EEprom Datafel"
#define ISTR_EE_FORMAT       "Formaterar EEPROM"
#define ISTR_GENWR_ERROR     "Skrivfel"
#define ISTR_EE_OFLOW        "EEPROM overflow"

// templates.cpp
// ***********
#define ISTR_T_S_4CHAN       "Normal 4-KN"
#define ISTR_T_TCUT          "GasKlippning"
#define ISTR_T_STICK_TCUT    "Seg Gasklippn"
#define ISTR_T_V_TAIL        "V-Tail"
#define ISTR_T_ELEVON        "Deltavinge"
#define ISTR_T_HELI_SETUP    "Heli-setup"
#define ISTR_T_GYRO          "Gyro-setup"
#define ISTR_T_SERVO_TEST    "Servotest"
#define ISTR_T_RANGE_TEST   "Range Test"

// menus.cpp
// ***********
#define ISTR_TELEM_ITEMS     "\004----A1= A2= RSSITSSITim1Tim2Hjd GhjdGkmhT1= T2= RPM TANKMah1Mah2CvltBattAmpsMah CtotFasVAccXAccYAccZVkmhGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7FwatRxV Hdg A3= A4= SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 RTC TmOKAspdCel1Cel2Cel3Cel4Cel5Cel6RBv1RBa1RBv2RBa2RBm1RBm2RBSVRBSTCel7Cel8Cel9Cl10Cl11Cl12Cus1Cus2Cus3Cus4Cus5Cus6Fmd RunTModTCls1Cls2SbcVSbcA"
#define ISTR_TELEM_SHORT     "\004----TIM1TIM2BATTGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7"
#define ISTR_GV              "GV"
#define ISTR_OFF_ON          "AV P\304 "
#define ISTR_HYPH_INV        FWx18 "\001" "\003---INV"
#define ISTR_VERSION         "VERSION"
#define ISTR_Music	        "Music"
#define ISTR_SLAVE           "\007Slav" 
#define ISTR_MENU_DONE       "[MENU] FORTS\304TTER"
#define ISTR_CURVES          "KURVOR"
#define ISTR_CURVE           "KURVA"
#define ISTR_GLOBAL_VAR      "GLOBAL-VAR"
#define ISTR_VALUE           "V\301rde"
#define ISTR_PRESET          "DEFAULT"
#define ISTR_CV              "CV"
#define ISTR_LIMITS          "GR\304NSER"
#define ISTR_COPY_TRIM       "LAGRA TRIM[MENU]"
#define ISTR_TELEMETRY       "TELEMETRI"
#define ISTR_USR_PROTO       "Protokoll"
#define ISTR_FRHUB_WSHHI    "\006FrSky WSHhi DSMx  Jeti  ArduP ArduC FrHub HubRawFrMav Mavlk Hitec AFHDS2"
#define ISTR_MET_IMP         "\003MetImp"
#define ISTR_A_CHANNEL       "A  kanal"
//#define ISTR_ALRM            "alrm"
//#define ISTR_TELEMETRY2      "TELEMETRI2"
#define ISTR_TX_RSSIALRM     "TxRSSIalrm"
#define ISTR_NUM_BLADES      "Antal Blad"
#define ISTR_ALT_ALARM       "HjdAlarm"
#define ISTR_OFF122400       "\003OFF122400"
#define ISTR_VOLT_THRES      "Volt-Gr\301ns="
#define ISTR_GPS_ALTMAIN     "GpsHjdMain"
#define ISTR_CUSTOM_DISP     "Anpassad Meny"
#define ISTR_FAS_OFFSET      "FAS Offset"
#define ISTR_VARIO_SRC       "Vario: Input"
#define ISTR_VSPD_A2         "\004----vkmhA2  SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_2SWITCH         "\001Brytare"
#define ISTR_2SENSITIVITY    "\001Noggrannhet"
#define ISTR_GLOBAL_VARS     "GLOBALA VAR"
#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_GV_SOURCE       "\003---RtmEtmTtmAtmRENRodHjdGasSkeP1 P2 P3 k1 k2 k3 k4 k5 k6 k7 k8 k9 k10k11k12k13k14k15k16k17k18k19k20k21k22k23k24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#ifdef PCBX9D
#define ISTR_GV_SOURCE       "\003---RtmEtmTtmAtmRENRodHjdGasSkeP1 P2 SL SR k1 k2 k3 k4 k5 k6 k7 k8 k9 k10k11k12k13k14k15k16k17k18k19k20k21k22k23k24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#if defined(PCBX12D) || defined(PCBX10)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#define ISTR_TEMPLATES       "MALLAR"
#define ISTR_CHAN_ORDER      "Kanalordning"
#define ISTR_SP_RETA         " RHGS"
#define ISTR_CLEAR_MIXES     "RADERA MIXAR[MENU]"
#define ISTR_SAFETY_SW       "S\304KERHETSBRYTARE"
#define ISTR_NUM_VOICE_SW    "Nummer-r\302stbryt."
#define ISTR_V_OPT1          "\007 8 Sek.12 Sek.16 Sek."
#define ISTR_VS              "VS"
#define ISTR_VOICE_OPT       "\006P\303    AV    B\303DA  15Sek.30Sek.60Sek.Variab"
#define ISTR_CUST_SWITCH     "LOGISKA BRYTARE"
#define ISTR_S               "S"
#define ISTR_15_ON           "\015On"
#define ISTR_EDIT_MIX        "REDIGERA MIX"
#define ISTR_2SOURCE         "\001Input"
#define ISTR_2WEIGHT         "\001Vikt"
#define ISTR_FMTRIMVAL       "FmTrimVal"
#define ISTR_OFFSET          "Offset"
#define ISTR_2FIX_OFFSET     "\001FixtOffset"
#define ISTR_ENABLEEXPO     "\001EnableExpoDr"
#define ISTR_2TRIM           "\001Trim"
#define ISTR_15DIFF          "\010Diff"
#define ISTR_Curve           "Kurva"
#define ISTR_2WARNING        "\001Varning"
#define ISTR_2MULTIPLEX      "\001Multpx"
// ISTR_ADD_MULT_REP indexed 8 chars each
#define ISTR_ADD_MULT_REP    "\010Addera  Multipl.Byt ut  "
#define ISTR_2DELAY_DOWN     "\001Dr\302j  Ned"
#define ISTR_2DELAY_UP       "\001Dr\302j  Upp"
#define ISTR_2SLOW_DOWN      "\001Sakta Ned"
#define ISTR_2SLOW_UP        "\001Sakta Upp"
#define ISTR_MAX_MIXERS      "Max mixerstorlek: 32"
#define ISTR_PRESS_EXIT_AB   "Tryck [EXIT] avbryter"
#define ISTR_YES_NO          "\003JA \013NEJ"
#define ISTR_MENU_EXIT       "\003[MENU]\013[EXIT]"
#define ISTR_DELETE_MIX      "RADERA MIX?"
#define ISTR_MIX_POPUP       "EDIT\0ADDERA\0KOPIA\0FLYTTA\0RADERA\0CLEAR ALL\0TEMPLATES\0PASTE"
#define ISTR_MIXER           "MIXER"
// CHR_S S for Slow
#define ICHR_S               "S"
// CHR_D D for Delay
#define ICHR_D               "D"
// CHR_d d for differential
#define ICHR_d               "d"
#define ISTR_EXPO_DR         "EXPO/DR"
#define ISTR_4DR_MID         "\004DR Med"
#define ISTR_4DR_LOW         "\004DR L\300g"
#define ISTR_4DR_HI          "\004DR H\302g"
#define ISTR_2EXPO           "\002Expo"
#define ISTR_DR_SW1          "DrBr1"
#define ISTR_DR_SW2          "DrBr2"
#define ISTR_DUP_MODEL       "DUPLICERA MODELL"
#define ISTR_DELETE_MODEL    "RADERA MODELL"
#define ISTR_DUPLICATING     "Duplicerar modell"
#define ISTR_SETUP           "Model Setup"
#define ISTR_NAME            "Namn"
#define ISTR_VOICE_INDEX     "R\302st\021MENU"
#define ISTR_TRIGGERA        "Trigger"
#define ISTR_TRIGGERB        "TriggerB"
//ISTR_COUNT_DOWN_UP indexed, 10 chars each
#define ISTR_COUNT_DOWN_UP   "\012R\301kna ned R\301kna upp "
#define ISTR_T_TRIM          "GasTrim"
#define ISTR_T_EXPO          "GasExpo-Dr"
// ISTR_TRIM_OPTIONS indexed 6 chars each
#define ISTR_TRIM_OPTIONS    FWx9 "\004" "\006Exp   xFin  Fin   MediumGrov  "
#define ISTR_TRIM_SWITCH     "Insta-TrimBr."
#define ISTR_TRIM_INC        "GasOkning"
#define ISTR_BEEP_CENTRE     "Centerpip"
#define ISTR_RETA123         "RHGS1234"
#define ISTR_PROTO           "Proto"
// ISTR_21_USEC after \021 max 4 chars
#define ISTR_21_USEC         "\021uSek"
#define ISTR_13_RXNUM        "\014RxNum"
// ISTR_23_US after \023 max 2 chars
#define ISTR_23_US           "\023uS"
// ISTR_PPMFRAME_MSEC before \015 max 9 chars, after max 4 chars
#define ISTR_PPMFRAME_MSEC   "PPM-ram  \016mSec"
#define ISTR_SEND_RX_NUM     " S\301nd Rx-nummer [MENU]"
#define ISTR_DSM_TYPE        " DSM-typ"
#define ISTR_PPM_1ST_CHAN    "Kanal 1"
#define ISTR_SHIFT_SEL       "SkiftVal"
// ISTR_POS_NEG indexed 3 chars each
#define ISTR_POS_NEG         "\003POSNEG"
#define ISTR_E_LIMITS        "Gr\301nser++"
#define ISTR_Trainer         "Trainer"
#define ISTR_T2THTRIG        "G2GsTrig"
#define ISTR_AUTO_LIMITS     "AutoGr\301nser"
// ISTR_1_RETA indexed 1 char each
#define ISTR_1_RETA          "\001RHGS"
#define ISTR_FL_MODE         "FlygFas"
#define ISTR_SWITCH          "Brytare"
#define ISTR_TRIMS           "Trimmar"
#define ISTR_MODES           "FASER"
#define ISTR_SP_FM0          " FF0"
#define ISTR_SP_FM           " FF"
#define ISTR_HELI_SETUP      "HELIKOPTER"
#define ISTR_SWASH_TYPE      "Swash-typ"
#define ISTR_COLLECTIVE      "Collective"
#define ISTR_SWASH_RING      "Swash-ring"
#define ISTR_ELE_DIRECTION   "HJD-riktning"
#define ISTR_AIL_DIRECTION   "SKEV-riktning"
#define ISTR_COL_DIRECTION   "COL-riktning"
#define ISTR_HELI_TEXT			ISTR_SWASH_TYPE "\037" ISTR_COLLECTIVE "\037" ISTR_SWASH_RING "\037" ISTR_ELE_DIRECTION "\037" ISTR_AIL_DIRECTION "\037" ISTR_COL_DIRECTION
#define ISTR_MODEL_POPUP     "EDIT\0VALJ\0SEL/EDIT\0KOPIA\0FLYTTA\0RADERA\0BACKUP\0RESTORE\0REPLACE\0NOTES"
#define ISTR_MODELSEL        "MODELLVAL"
// ISTR_11_FREE after \011 max 4 chars
#define ISTR_11_FREE         "\011kvar"
#define ISTR_CALIBRATION     "KALIBRERING"
// ISTR_MENU_TO_START after \003 max 15 chars
#define ISTR_MENU_TO_START   "\003[MENU] STARTAR"
// ISTR_SET_MIDPOINT after \005 max 11 chars
#define ISTR_SET_MIDPOINT    "\005ANGE MITT"
// ISTR_MOVE_STICKS after \003 max 15 chars
#define ISTR_MOVE_STICKS     "\003R\305R SPAKAR/POTTAR"
#define ISTR_ANA             "ANA"
#define ISTR_DIAG            "DIAG"
// ISTR_KEYNAMES indexed 5 chars each
#define ISTR_KEYNAMES        "\005 Menu Exit Ned   UppH\302ger V\301ns"
#define ISTR_TRIM_M_P        "Trim- +"
// ISTR_OFF_PLUS_EQ indexed 3 chars each
#define ISTR_OFF_PLUS_EQ     "\003av  += :="
// ISTR_CH1_4 indexed 3 chars each
#define ISTR_CH1_4           "\003kn1kn2kn3kn4"
#define ISTR_MULTIPLIER      "Multiplier"
#define ISTR_CAL             "Kal"
#define ISTR_MODE_SRC_SW     "\003fas \012% inp  br"
#define ISTR_RADIO_SETUP     "INST\301LLNING 1"
#define ISTR_OWNER_NAME      "Namn"
#define ISTR_BEEPER          "Tuta"
// ISTR_BEEP_MODES indexed 6 chars each
#define ISTR_BEEP_MODES      "\006Tyst  ""EjKnp ""xKort ""Kort  ""Norm  ""L\300ng  ""xL\300ng "
#define ISTR_SOUND_MODE      "LjudTyp"
// ISTR_SPEAKER_OPTS indexed 10 chars each
#define ISTR_SPEAKER_OPTS    "\012Tuta      ""PiSpkr    ""R\302stTuta  ""PiSpkR\302st ""MegaSound "
#define ISTR_VOLUME          "Volym"
#define ISTR_SPEAKER_PITCH   "Tonh\302jd"
#define ISTR_HAPTICSTRENGTH  "Vibratorstyrka"
#define ISTR_CONTRAST        "Kontrast"
#define ISTR_BATT_WARN       "Batterivarning" 
// ISTR_INACT_ALARM m for minutes after \023 - single char
#define ISTR_INACT_ALARM     "Inaktivitetslarm\023m"
#define ISTR_THR_REVERSE     "Inverterad gas"
#define ISTR_MINUTE_BEEP     "Minutpip"
#define ISTR_BEEP_COUNTDOWN  "Nedr\301knigspip"
#define ISTR_FLASH_ON_BEEP   "Blink vid pip"
#define ISTR_LIGHT_SWITCH    "Ljusbrytare"
#define ISTR_LIGHT_INVERT    "Invertera ljus"
#define ISTR_LIGHT_AFTER     "Ljus aktiv. key"
#define ISTR_LIGHT_STICK     "Spak aktiv. ljus"
#define ISTR_SPLASH_SCREEN   "Startbild"
#define ISTR_SPLASH_NAME     "Startnamn"
#define ISTR_THR_WARNING     "Gasvarning"
#define ISTR_DEAFULT_SW      "Default Br"
#define ISTR_MEM_WARN        "MinnesVarning"
#define ISTR_ALARM_WARN      "AlarmVarning"
#define ISTR_POTSCROLL       "PotBl\301ddring"
#define ISTR_STICKSCROLL     "SpakBl\301ddring"
#define ISTR_BANDGAP         "BandGap"
#define ISTR_ENABLE_PPMSIM   "Aktiv. PPMSIM"
#define ISTR_CROSSTRIM       "KorsTrim"
#define ISTR_INT_FRSKY_ALRM  "Int. FrSky-larm"
#define ISTR_MODE            "Fas"

// SWITCHES_STR 3 chars each
#if defined(PCBSKY) || defined(PCB9XT)
#define ISWITCHES_STR        "\003GASRODHJDID\200ID-ID\201SKELANTRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfTH\200TH-TH\201RU\200RU-RU\201EL\200EL-EL\201AI\200AI-AI\201GE\200GE-GE\2016P06P16P26P36P46P5PB1PB2PB3PB4"
#define IHW_SWITCHES_STR     "\004sIDxsTHRsRUDsELEsAILsGEAsTRNL1  L2  L3  L4  L5  L6  L7  L8  L9  LA  LB  LC  LD  LE  LF  LG  LH  LI  LJ  LK  LL  LM  LN  LO  6POS"
//#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6P"
#define IHW_SWITCHARROW_STR  "\200-\201"
#endif
#ifdef PCBX9D
#ifdef REV9E
#define ISWITCHES_STR        "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5"\
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
#define IHW_SWITCHARROW_STR  "\200-\201"
#endif
#if defined(PCBX12D) || defined(PCBX10)
#define ISWITCHES_STR				 "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5PB1PB2PB3PB4"
#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6PL1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"
#define IHW_SWITCHARROW_STR  "\200-\201"
#endif
#define ISWITCH_WARN_STR     "Brytarvarning"
// CURV_STR indexed 3 chars each
// c17-c24 added for timer mode A display
#define ICURV_STR					 "\003---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24c25c26c27c28c29c30c31c32"
// CSWITCH_STR indexed 7 chars each
#define ICSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""Latch  F-Flop TimeOffNtmeOff1-Shot 1-ShotRv\140=val v&val  v1\140=v2 v=val  "

#define ISWASH_TYPE_STR      FWx17"\004""\004----""120 ""120X""140 ""90  "

#define ISTR_STICK_NAMES     "\0054Rod \0Hjd \0Gas \0Ske "

#define ISTR_STAT            "STAT"
#define ISTR_STAT2           "STAT2"
// ISTR_TRIM_OPTS indexed 3 chars each
#define ISTR_TRIM_OPTS       "\003ExpExFFneMedCrs"
#define ISTR_TTM             "TTm"
#define ISTR_FUEL            "Tank"
#define ISTR_12_RPM          "\012RPM"
#define ISTR_LON_EQ          "Lon="
#define ISTR_ALT_MAX         "Hjd=\011m   Max="
#define ISTR_SPD_KTS_MAX     "Kmh=\011kts Max="
#define ISTR_LAT_EQ         "Lat=" "\037" ISTR_LON_EQ "\037" ISTR_ALT_MAX "\037" ISTR_SPD_KTS_MAX
#define ISTR_11_MPH          "\011mph"

#define ISTR_SINK_TONES      "Sjunktoner"


// ersky9x strings
#define ISTR_ST_CARD_STAT    "SDKORT-STAT."
#define ISTR_4_READY         "\004Redo"
#define ISTR_NOT             "EJ"
#define ISTR_BOOT_REASON     "BOOT-ORSAK"
#define ISTR_6_WATCHDOG      "\006WATCHDOG"
#define ISTR_5_UNEXPECTED    "\005OV\304NTAT"
#define ISTR_6_SHUTDOWN      "\006AVSLUT"
#define ISTR_6_POWER_ON      "\006UPPSTART"
// ISTR_MONTHS indexed 3 chars each
#define ISTR_MONTHS          "\003XxxJanFebMarAprMajJunJulAugSepOktNovDec"
#define ISTR_MENU_REFRESH    "[MENU] uppdaterar"
#define ISTR_DATE_TIME       "TIDPUNKT"
#define ISTR_SEC             "Sek."
#define ISTR_MIN_SET         "Min.\015Set"
#define ISTR_HOUR_MENU_LONG  "Tim.\012MENU LONG"
#define ISTR_DATE            "Datum"
#define ISTR_MONTH           "M\300nad"
#define ISTR_YEAR_TEMP       "\303r  \013Temp."
#define ISTR_YEAR            "\303r  "
#define ISTR_BATTERY         "BATTERI"
#define ISTR_Battery         "Batteri"
#define ISTR_CURRENT_MAX     "Str\302m  \016Max"
#define ISTR_CPU_TEMP_MAX    "CPU temp.\014C Max\024C"
#define ISTR_MEMORY_STAT     "MINNES-STAT."
#define ISTR_GENERAL         "Generell"
#define ISTR_Model           "Modell"
#define ISTR_RADIO_SETUP2    "INST\304LLNING 2"
#define ISTR_BRIGHTNESS      "Ljusstyrka"
#define ISTR_CAPACITY_ALARM  "Kapacitetslarm"
#define ISTR_BT_BAUDRATE     "BThastighet"
#define ISTR_ROTARY_DIVISOR  "Rotary Divisor"
#define ISTR_LV             "LV"
#define ISTR_LH             "LH"
#define ISTR_RV             "RV"
#define ISTR_RH             "RH"
#define ISTR_STICK_GAIN      "Spak-Gain"
#define ISTR_STICK_DEADBAND  "Spak-Deadband"
#define ISTR_STICK_LV_GAIN   "Spak-gain VV"
#define ISTR_STICK_LH_GAIN   "Spak-gain VH"
#define ISTR_STICK_RV_GAIN   "Spak-gain HV"
#define ISTR_STICK_RH_GAIN   "Spak-gain HH"
#define ISTR_BIND            "Bind"
#define ISTR_RANGE           "R\301ckvidd"

#define ISTR_ALRMS_OFF       "Alarm Avslagna"
#define ISTR_OLD_EEPROM      " Gammal EEPROM-ver.   KOLLA INST\304LLNINGAR"
#define ISTR_TRIGA_OPTS      "AV ON THsTH%"
#define ISTR_CHK_MIX_SRC     "KOLLA MIX-INPUT"

#define ISTR_BT_TELEMETRY    "BT-Telemetri"
#define ISTR_FRSKY_COM_PORT  "Telem. COM-port"
#define ISTR_INVERT_COM1     "Invert.COM1"
#define ISTR_LOG_SWITCH      "Loggbrytare"
#define ISTR_LOG_RATE        "Loggdata"
#define ISTR_6_BINDING       "\006BINDER"
#define ISTR_RANGE_RSSI      "R\304CKVIDD RSSI:"
#define ISTR_FAILSAFE        "FAILSAFE"
#define ISTR_VOLUME_CTRL     "Volymkontroll"
//#ifdef ASSAN
//#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiAssan"
//#else
#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiXfireAcces"
//#endif
#define ISTR_TYPE            " Typ"
#define ISTR_COUNTRY         " Land"
#define ISTR_SP_FAILSAFE     " Failsafe"
#define ISTR_PPM2_START      "PPM2 Startkanal"
#define ISTR_FOLLOW          "F\302lj"
#define ISTR_PPM2_CHANNELS   "PPM2 Kanaler"
#define ISTR_FILTER_ADC      "ADC-filter"
#define ISTR_SCROLLING       "Bl\301ddring"
#define ISTR_ALERT_YEL       "Varn. [Gul]"
#define ISTR_ALERT_ORG       "Varn. [Org]"
#define ISTR_ALERT_RED       "Varn. [Rod]"
#define ISTR_LANGUAGE        "Spr\300k"

#define ISTR_RSSI_WARN		  "RSSI Warn"
#define ISTR_RSSI_CRITICAL  "RSSI Critical"
#define ISTR_RX_VOLTAGE		  "Rx Voltage"
#define ISTR_DSM_WARNING	  "DSM Warning"
#define ISTR_FADESLOSSHOLDS "\006fades lossesholds "
#define ISTR_DSM_CRITICAL	  "DSM Critical"
#define ISTR_BT_TRAINER		  "BT as Trainer"
#define ISTR_MULTI_TEXT     "Protocol\037Type\037Autobind\037Power"
#define ISTR_MULTI_PROTO    "Protocol"
#define ISTR_MULTI_TYPE	    "Type"
#define ISTR_MULTI_AUTO	    "Autobind"
#define ISTR_MULTI_POWER    "Power\013Rate\023mS"
#define ISTR_MULTI_OPTION   "\013Option"

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

//"RSSI Warn"
//"RSSI Critical"
//"Rx Voltage"
//"DSM Warning"
//"\006fades lossesholds "
//"DSM Critical"
//"BT as Trainer"
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
//"\001Haptic Min Run"




