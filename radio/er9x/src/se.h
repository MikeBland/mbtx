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


#define STR_ON              "PA "
#define STR_OFF             "AV "

#define STR_ALTEQ           "Hjd=" 
#define STR_TXEQ		       "\003Tx=Swr"
#define STR_RXEQ            "Rx="
#define STR_RX  		        "Rx"
#define STR_TRE012AG        "TRE012AGG"

// STR_YELORGRED indexed 3 char each
#define STR_YELORGRED       "\003---GulOrgRod"
#define STR_A_EQ            "A ="
#define STR_SOUNDS          "\006Varn1 ""Varn2 ""Lamm  ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Syrsa ""Siren ""Alarm ""Ratata""Tick  ""Haptk1""Haptk2""Haptk3"
#define STR_SWITCH_WARN     "BrytarVarning"
// STR_TIMER exactly 5 chars long
#define STR_TIMER           "Timer"

// STR_PPMCHANNELS indexed 4 char each
#define STR_PPMCHANNELS     "\0044KN 6KN 8KN 10KN12KN14KN16KN"

#define STR_MAH_ALARM       "mAh Limit"


// er9x.cpp
// ********
#define STR_LIMITS          "GRANSER"
#define STR_EE_LOW_MEM      "EEPROM fullt"
#define STR_ALERT           "VARNING"
#define STR_THR_NOT_IDLE    "Gas ej avslagen"
#define STR_RST_THROTTLE    "Nollstall gas"
#define STR_PRESS_KEY_SKIP  "Knapptryck fortsatter"
#define STR_ALARMS_DISABLE  "Alarm Avslagna"
#define STR_OLD_VER_EEPROM  " Gammal EEPROM-ver.   KOLLA INSTALLNINGAR"
#define STR_RESET_SWITCHES  "Nollstall Brytarna"
#define STR_LOADING         "LADDAR "
#define STR_MESSAGE         "INFO"
#define STR_PRESS_ANY_KEY   "tryck ned Knapp"
#define STR_MSTACK_UFLOW    "mStack uflow"
#define STR_MSTACK_OFLOW    "mStack oflow"

#define STR_CHANS_GV        "\004P1  P2  P3  HALVFULLCYK1CYK2CYK3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8KN1 KN2 KN3 KN4 KN5 KN6 KN7 KN8 KN9 KN10KN11KN12KN13KN14KN15KN16SWCHGV1 GV2 GV3 GV4 GV5 GV6 GV7 THIS"
#define STR_CHANS_RAW       "\004P1  P2  P3  HALVFULLCYK1CYK2CYK3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8KN1 KN2 KN3 KN4 KN5 KN6 KN7 KN8 KN9 KN10KN11KN12KN13KN14KN15KN16SWCH"
#define STR_CH              "KN"
#define STR_TMR_MODE        "\003OFFON RUsRU%ELsEL%THsTH%ALsAL%P1 P1%P2 P2%P3 P3%"
#define STR_TRIGA_OPTS			"OFFON THsTH%"

// pers.cpp
// ********
#define STR_ME              "DITT NAMN "
#define STR_MODEL           "MODELL    "
#define STR_BAD_EEPROM      "EEprom Datafel"
#define STR_EE_FORMAT       "Formaterar EEPROM"
#define STR_GENWR_ERROR     "Skrivfel"
#define STR_EE_OFLOW        "EEPROM overflow"

// templates.cpp
// ***********
#define STR_T_S_4CHAN       "Normal 4-KN"
#define STR_T_TCUT          "GasKlippning"
#define STR_T_STICK_TCUT    "Seg Gasklippn"
#define STR_T_V_TAIL        "V-Tail"
#define STR_T_ELEVON        "Deltavinge"
#define STR_T_HELI_SETUP    "Heli-setup"
#define STR_T_GYRO          "Gyro-setup"
#define STR_T_SERVO_TEST16  "Servotest(16)"
#define STR_T_SERVO_TEST8   "Servotest(8)"

// menus.cpp
// ***********
#define STR_TELEM_ITEMS     "\004----A1= A2= RSSITSSITim1Tim2Hjd GhjdGkmhT1= T2= RPM TANKMah1Mah2CvltBattAmpsMah CtotFasVAccXAccYAccZVkmhGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7FwatRxV Hdg A3= A4= SC1 SC2 SC3 SC4 TmOK"
#define STR_TELEM_SHORT     "\004----TIM1TIM2BATTGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7"
#define STR_GV              "GV"
#define STR_OFF_ON          "AV PA "
#define STR_HYPH_INV        "\003---INV"
#define STR_VERSION         "Version"
#define STR_TRAINER         "Trainer"
#define STR_SLAVE           "\007Slav" 
#define STR_MENU_DONE       "[MENU] FORTSATTER"
#define STR_CURVES          "Kurvor"
#define STR_CURVE           "KURVA"
#define STR_GLOBAL_VAR      "GLOBAL-VAR"
#define STR_VALUE           "Varde"
#define STR_PRESET          "DEFAULT"
#define STR_CV              "CV"
#define STR_LIMITS          "GRANSER"
#define STR_COPY_TRIM       "LAGRA TRIM[MENU]"
#define STR_TELEMETRY       "TELEMETRI"
#define STR_USR_PROTO_UNITS "Protokoll\037Units"
#define STR_USR_PROTO       "Protokoll"
#define STR_FRHUB_WSHHI     "\005FrHubWSHhi"
#define STR_MET_IMP         "\003MetImp"
#define STR_A_CHANNEL       "A  kanal"
#define STR_ALRM            "alrm"
#define STR_TELEMETRY2      "TELEMETRI2"
#define STR_TX_RSSIALRM     "TxRSSIalrm"
#define STR_NUM_BLADES      "Antal Blad"
//#if ALT_ALARM
//#define STR_ALT_ALARM       "HjdAlarm"
//#define STR_OFF122400       "\003OFF122400"
//#endif
#define STR_VOLT_THRES      "Volt-Grans="
#define STR_GPS_ALTMAIN     "GpsHjdMain"
#define STR_CUSTOM_DISP     "Anpassad Meny"
#define STR_FAS_OFFSET      "FAS Offset"
//#define STR_VARIO_SRC_IDX  "Vario: Input\000\132\002\004----vkmhA2  "
#define STR_VARIO_SRC       "Vario: Input"
#define STR_VSPD_A2         "\004----vkmhA2  "
#define STR_2SWITCH         "\001Brytare"
#define STR_2SENSITIVITY    "\001Noggrannhet"
#define STR_GLOBAL_VARS     "GlobalaVar"
#define STR_GV_SOURCE       "\003---RtmEtmTtmAtmRENRODHJDGASSKEP1 P2 P3 k1 k2 k3 k4 k5 k6 k7 k8 k9 k10k11k12k13k14k15k16k17k18k19k20k21k22k23k24"
#define STR_TEMPLATES       "MALLAR"
#define STR_CHAN_ORDER      "Kanalordning"
#define STR_SP_RETA         " RHGS"
#define STR_CLEAR_MIXES     "RADERA MIXAR[MENU]"
#define STR_SAFETY_SW       "SAKERHETSBRYTARE"
#define STR_SAFETY_SW2     "Safety Sws"
#define STR_NUM_VOICE_SW    "Nummer-rostbryt."
#define STR_V_OPT1          "\007 8 Sek.12 Sek.16 Sek."
#define STR_VS              "VS"
#define STR_VOICE_OPT       "\006PA    AV    BADA  15Sek.30Sek.60Sek.Variab"
#define STR_CUST_SWITCH     "LOGISKA BRYTARE"
//#define STR_S               "S"
#define STR_15_ON           "\015On"
#define STR_EDIT_MIX        "REDIGERA MIX"
#define STR_2SOURCE         "\001Input"
#define STR_2WEIGHT         "\001Vikt"
#define STR_OFFSET          "Offset"
#define STR_2FIX_OFFSET     "\001Fix Offset"
#define STR_FLMODETRIM      "\001FlygFasTrm"
#define STR_ENABLEEXPO		 "\001EnableExpoDR"
#define STR_2TRIM           "\001Trim"
#define STR_15DIFF          "\015Diff"
#define STR_Curve           "Kurva"
#define STR_2WARNING        "\001Varning"
#define STR_2MULTIPLEX      "\001Multpx"
// STR_ADD_MULT_REP indexed 8 chars each
#define STR_ADD_MULT_REP    "\010Addera  Multipl.Byt ut  "
#define STR_2DELAY_DOWN     "\001Droj  Ned"
#define STR_2DELAY_UP       "\001Droj  Upp"
#define STR_2SLOW_DOWN      "\001Sakta Ned"
#define STR_2SLOW_UP        "\001Sakta Upp"
#define STR_MAX_MIXERS_EXAB "Max mixerstorlek: 32\037\037Tryck [EXIT] avbryter"
#define STR_MAX_MIXERS      "Max mixerstorlek: 32"
#define STR_PRESS_EXIT_AB   "Tryck [EXIT] avbryter"
#define STR_YES_NO_MENU_EXIT "\003JA \013NEJ\037\003[MENU]\013[EXIT]"
#define STR_MENU_EXIT       "\003[MENU]\013[EXIT]"
#define STR_DELETE_MIX      "RADERA MIX?"
#define STR_MIX_POPUP       "EDIT\0ADDERA\0KOPIA\0FLYTTA\0RADERA\0CLEAR ALL"
#define STR_MIXER           "MIXER"
// CHR_S S for Slow
#define CHR_S               'S'
// CHR_D D for Delay
#define CHR_D               'D'
// CHR_d d for differential
#define CHR_d               'd'
#define STR_EXPO_DR         "Expo/Dr"
#define STR_4DR_HIMIDLO		 "\007\004DR Hog\004DR Med\004DR Lag"
#define STR_4DR_MID         "\004DR Med"
#define STR_4DR_LOW         "\004DR Lag"
#define STR_4DR_HI          "\004DR Hog"
#define STR_EXPO_TEXT			 "\002Expo\037\037\001Vikt\037\037DrBr1\037DrBr2"
#define STR_2EXPO           "\002Expo"
#define STR_DR_SW1          "DrBr1"
#define STR_DR_SW2          "DrBr2"
#define STR_DUP_MODEL       "DUPLICERA MODELL"
#define STR_DELETE_MODEL    "RADERA MODELL"
#define STR_DUPLICATING     "Duplicerar modell"
#define STR_SETUP           "SETUP"
#define STR_NAME            "Namn"
#define STR_VOICE_INDEX     "Rost-Index\021MENU"
#define STR_TIMER_TEXT		 "Timer\037TriggerA\037TriggerB\037Timer\037\037\037Reset Switch"
#define STR_TIMER_TEXT_X	 "Timer\037TriggerA\037TriggerB\037Timer\037Reset Switch"
#define STR_TRIGGER	        "TriggerA"
#define STR_TRIGGERB        "TriggerB"
//STR_COUNT_DOWN_UP indexed, 10 chars each
#define STR_COUNT_DOWN_UP   "\012Rakna ned Rakna upp "
#define STR_T_TRIM          "GasTrim"
#define STR_T_EXPO          "GasExpo-Dr"
#define STR_TRIM_INC        "GasOkning"
// STR_TRIM_OPTIONS indexed 6 chars each
#define STR_TRIM_OPTIONS    "\006Exp   xFin  Fin   MediumGrov  "
#ifdef V2
#define STR_TRIM_PAGE			 STR_TRIM_INC"\037"STR_TRIM_SWITCH"\037"STR_TRAINER"\037"STR_BEEP_CENTRE
#else
#define STR_TRIM_PAGE			 STR_TRIM_INC"\037"STR_TRIM_SWITCH"\037Hi.Res Slow/Delay\037"STR_TRAINER"\037"STR_BEEP_CENTRE
#endif
#define STR_TRIM_SWITCH     "TrimBr."
#define STR_BEEP_CENTRE     "Centerpip"
#define STR_RETA123         "RHGS123"
#define STR_PROTO           "Proto"
// STR_21_USEC after \021 max 4 chars
#define STR_21_USEC         "\021uSek"
#define STR_13_RXNUM        "\014RxNum"
// STR_23_US after \023 max 2 chars
#define STR_23_US           "\023uS"
// STR_PPMFRAME_MSEC before \015 max 9 chars, after max 4 chars
#define STR_PPMFRAME_MSEC   "PPM-ram  \015mSec"
#define STR_SEND_RX_NUM    "Bind  Range"
#define STR_DSM_TYPE        "DSM-typ"
#define STR_PXX_TYPE       " Type\037 Country\037Bind\037Range"
#ifdef MULTI_PROTOCOL
#define STR_MULTI_TYPE		"Protocol\037Type\037Power\037Bind     Autobind\037Range"
#define STR_MULTI_OPTION	"\013Option"
#define M_NONE_STR			"\004None"
#define M_NY_STR			"\001NY"
#define M_LH_STR			"\004HighLow "
#endif // MULTI_PROTOCOL
#define STR_1ST_CHAN_PROTO 	"Kanal 1\037Proto"
#define STR_PPM_1ST_CHAN    "Kanal 1"
#define STR_SHIFT_SEL       "SkiftVal"
// STR_POS_NEG indexed 3 chars each
#define STR_POS_NEG         "\003POSNEG"
#define STR_VOL_PAGE				STR_E_LIMITS"\037""Thr. Default\037"STR_THR_REVERSE"\037""Throttle Open""\037"STR_T_TRIM"\037"STR_T_EXPO
#define STR_E_LIMITS        "Granser++"
#define STR_Trainer         "Trainer"
#define STR_T2THTRIG        "G2GsTrig"
#define STR_AUTO_LIMITS     "AutoGranser"
// STR_1_RETA indexed 1 char each
#define STR_1_RETA          "\001RHGS"
#define STR_FL_MODE         "FlygFas"
#define STR_SWITCH_TRIMS   "Brytare\037Trimmar"
#define STR_SWITCH          "Brytare"
#define STR_TRIMS           "Trimmar"
#define STR_MODES           "FASER"
#define STR_SP_FM0          " FF0"
#define STR_SP_FM           " FF"
#define STR_HELI_SETUP      "HELIKOPTER"
#define STR_HELI_TEXT			 "Swash-typ\037Collective\037Swash Ring\037HJD-riktning\037SKEV-riktning\037COL-riktning"
#define STR_SWASH_TYPE      "Swash-typ"
#define STR_COLLECTIVE      "Collective"
#define STR_SWASH_RING      "Swash-ring"
#define STR_ELE_DIRECTION   "HJD-riktning"
#define STR_AIL_DIRECTION   "SKEV-riktning"
#define STR_COL_DIRECTION   "COL-riktning"

#define STR_MODEL_POPUP     "EDIT\0VALJ\0SEL/EDIT\0KOPIA\0FLYTTA\0RADERA"
#define STR_MODELSEL        "MODELLVAL"
// STR_11_FREE after \011 max 4 chars
#define STR_11_FREE         "\011kvar"
#define STR_CALIBRATION     "Kalibrering"
// STR_MENU_TO_START after \003 max 15 chars
#define STR_MENU_TO_START   "\003[MENU] STARTAR"
// STR_SET_MIDPOINT after \005 max 11 chars
#define STR_SET_MIDPOINT    "\005ANGE MITT"
// STR_MOVE_STICKS after \003 max 15 chars
#define STR_MOVE_STICKS     "\003ROR SPAKAR/POTTAR"
#define STR_ANA             "ANA"
#define STR_DIAG            "DIAG"
#define STR_KEYNAMES       " Vans\037Hoger\037\002Upp\037  Ned\037 Exit\037 Menu"
#define STR_TRIM_M_P        "Trim- +"
// STR_OFF_PLUS_EQ indexed 3 chars each
#define STR_OFF_PLUS_EQ     "\003av  += :="
// STR_CH1_4 indexed 3 chars each
#define STR_CH1_4           "\003kn1kn2kn3kn4"
#define STR_MULTIPLIER      "Multiplier"
#define STR_CAL             "Kal"
#define STR_MODE_SRC_SW     "\003fas \012% inp  br"
#define STR_RADIO_SETUP     "General"
#define STR_OWNER_NAME      "Namn"
#define STR_BEEPER          "Tuta"
// STR_BEEP_MODES indexed 6 chars each
#define STR_BEEP_MODES      "\006Tyst  ""EjKnp ""xKort ""Kort  ""Norm  ""Lang  ""xLang "
#define STR_SOUND_MODE      "LjudTyp"
// STR_SPEAKER_OPTS indexed 10 chars each
#define STR_SPEAKER_OPTS    "\012Beeper    ""PiSpkr    ""BeeprVoice""PiSpkVoice""MegaSound "
#define STR_VOLUME          "Volym"
#define STR_SPEAKER_PITCH   " Tonhojd"
#define STR_HAPTICSTRENGTH  " Vibratorstyrka"
#define STR_CONTRAST        "Kontrast"
#define STR_BATT_WARN       "Batterivarning" 
// STR_INACT_ALARM m for minutes after \023 - single char
#define STR_INACT_ALARM     "Inaktivitetslarm\023m"
#define STR_THR_REVERSE     "Inverterad gas"
#define STR_MINUTE_BEEP     "Minutpip"
#define STR_BEEP_COUNTDOWN  "Nedraknigspip"
#define STR_FLASH_ON_BEEP   "Blink vid pip"
#define STR_LIGHT_SW_TEXT  "Ljusbrytare\037\037Ljus av efter\023s\037Spak aktiv. ljus\023s"
#define STR_LIGHT_SWITCH    "Ljusbrytare"
#define STR_LIGHT_INVERT    "Invertera ljus"
#define STR_LIGHT_AFTER     "Ljus av efter\023s"
#define STR_LIGHT_STICK     "Spak aktiv. ljus\023s"
#define STR_SPLASH_SCREEN   "Startbild"
#define STR_SPLASH_NAME     "Startnamn"
#define STR_THR_WARNING     "Gasvarning"
#define STR_DEAFULT_SW_PAGE "Default Bw\037CustomStkNames\037AutoGranser\037Throttle Default"
#define STR_DEAFULT_SW      "Default Br"
#define STR_MEM_WARN        "MinnesVarning"
#define STR_ALARM_WARN      "AlarmVarning"
#define STR_POTSCROLL       "PotBladdring"
#define STR_STICKSCROLL     "SpakBladdring"
#define STR_BANDGAP         "BandGap"
#define STR_ENABLE_PPMSIM   "Aktiv. PPMSIM"
#define STR_CROSSTRIM       "KorsTrim"
#define STR_INT_FRSKY_ALRM  "Int. FrSky-larm"
#define STR_MODE            "Fas"

// SWITCHES_STR 3 chars each
#ifdef XSW_MOD
#define SWITCHES_STR        "\003IDLGASRODHJDSKELANPB1PB2TRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI ID0ID1ID2TH\200TH-TH\201RU\200RU-RU\201EL\200EL-EL\201AI\200AI-AI\201GE\200GE-GE\201"
#else
#if defined(CPUM128) || defined(CPUM2561)
#define SWITCHES_STR        "\003GASRODHJDID0ID1ID2SKELANTRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI EL\200EL-EL\201RU\200RU-RU\201AI\200AI-AI\201GE\200GE-GE\201PB1PB2"
#else
#define SWITCHES_STR        "\003GASRODHJDID0ID1ID2SKELANTRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC "
#endif
#endif  // XSW_MOD

#define SWITCH_WARN_STR     "Brytarvarning"
// CURV_STR indexed 3 chars each
#define CURV_STR            "\003---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16"
// CSWITCH_STR indexed 7 chars each
#ifdef VERSION3
#if defined(CPUM128) || defined(CPUM2561)
#define CSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    v1==v2 v1!=v2 v1>v2  v1<v2  Latch  F-Flop TimeOffv\140=val 1-Shot 1-ShotR"
#else
#define CSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    v1==v2 v1!=v2 v1>v2  v1<v2  Latch  F-Flop TimeOffv\140=val "
#endif
#else
#define CSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""v1>=v2 ""v1<=v2 TimeOffv1\140=val"
#endif

#define SWASH_TYPE_STR     "\004----""120 ""120X""140 ""90  "

#if defined(CPUM128) || defined(CPUM2561)
#define STR_STICK_NAMES     "Rod \0Hjd \0Gas \0Ske "
#else
#define STR_STICK_NAMES     "Rod Hjd Gas Ske "
#endif

#define STR_STAT            "STAT"
#define STR_STAT2           "STAT2"
// STR_TRIM_OPTS indexed 3 chars each
#define STR_TRIM_OPTS       "\003ExpExFFneMedCrs"
#define STR_TTM             "TTm"
#define STR_FUEL            "Tank"
#define STR_12_RPM          "\012RPM"
#define STR_LAT_EQ          "Lat="
#define STR_LON_EQ          "Lon="
#define STR_ALT_MAX         "Hjd=\011m   Max="
#define STR_SPD_KTS_MAX     "Kmh=\011kts Max="
#define STR_11_MPH          "\011mph"

#define STR_SINK_TONES      "Sjunktoner"
#define STR_FRSKY_MOD      "Frsky Mod Done"
#define STR_TEZ_R90				 "TelemetrEZ>=r90"

// ersky9x strings
#define STR_ST_CARD_STAT    "SDKORT-STAT."
#define STR_4_READY         "\004Redo"
#define STR_NOT             "EJ"
#define STR_BOOT_REASON     "BOOT-ORSAK"
#define STR_6_WATCHDOG      "\006WATCHDOG"
#define STR_5_UNEXPECTED    "\005OVANTAT"
#define STR_6_SHUTDOWN      "\006AVSLUT"
#define STR_6_POWER_ON      "\006UPPSTART"
// STR_MONTHS indexed 3 chars each
#define STR_MONTHS          "\003XxxJanFebMarAprMajJunJulAugSepOktNovDec"
#define STR_MENU_REFRESH    "[MENU] uppdaterar"
#define STR_DATE_TIME       "TIDPUNKT"
#define STR_SEC             "Sek."
#define STR_MIN_SET         "Min.\015Set"
#define STR_HOUR_MENU_LONG  "Tim.\012MENU LONG"
#define STR_DATE            "Datum"
#define STR_MONTH           "Manad"
#define STR_YEAR_TEMP       "Ar  \013Temp."
#define STR_YEAR            "Ar  "
#define STR_BATTERY         "BATTERI"
#define STR_Battery         "Batteri"
#define STR_CURRENT_MAX     "Strom  \016Max"
#define STR_CPU_TEMP_MAX    "CPU temp.\014C Max\024C"
#define STR_MEMORY_STAT     "MINNES-STAT."
#define STR_GENERAL         "Generell"
#define STR_Model           "Modell"
#define STR_RADIO_SETUP2    "INSTALLNING 2"
#define STR_BRIGHTNESS      "Ljusstyrka"
#define STR_CAPACITY_ALARM  "Kapacitetslarm"
#define STR_BT_BAUDRATE     "BThastighet"
#define STR_ROTARY_DIVISOR  "Rotary Divisor"
#define STR_STICK_LV_GAIN   "Spak-gain VV"
#define STR_STICK_LH_GAIN   "Spak-gain VH"
#define STR_STICK_RV_GAIN   "Spak-gain HV"
#define STR_STICK_RH_GAIN   "Spak-gain HH"

#define STR_DISPLAY					"Display"
#define STR_HARDWARE				"Hardware" ;
#define STR_ALARMS					"Alarms" ;
#define STR_CONTROLS				"Controls" ;
#define STR_AUDIOHAPTIC			"AudioHaptic" ;
#define STR_DIAGSWTCH				"DiagSwtch" ;
#define STR_DIAGANA					"DiagAna" ;

#define STR_PROTOCOL				"Protocol"
#define STR_MIXER2  	   		"Mixer"
#define STR_CSWITCHES 	   	"L.Switches"
#define STR_VOICE   	   		"Voice"
#define STR_VOICEALA   	   	"Voice Alarms"
#define STR_CLEAR_ALL_MIXES "Clear ALL mixes?"

#define STR_MAIN_POPUP			"Model Select\0Model Setup\0Last Menu\0Radio Setup\0Statistics"
#define MODEL_SETUP_OFFSET	13
#define RADIO_SETUP_OFFSET	35


