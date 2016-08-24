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
 * - Fabian Schurig <fabian.schurig.94@gmail.com>
 * - Max Mäusezahl
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


#define STR_ON             "AN "
#define STR_OFF            "AUS"

#define STR_ALTEQ	         "Hoe=" 
#define STR_TXEQ		       "\003Sn=Swr"
#define STR_RXEQ		       "Em="
#define STR_RX  		       "Em"
#define STR_TRE012AG	     "TRE012AGG"

// STR_YELORGRED je genau 3 Zeichen lang
#define STR_YELORGRED	     "\003---GelOrgRot"
#define STR_A_EQ		       "A ="
#define STR_SOUNDS	       "\006Warn1 ""Warn2 ""Cheap ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Crickt""Siren ""AlmClk""Ratata""Tick  ""Haptc1""Haptc2""Haptc3"
#define STR_SWITCH_WARN	   "Schalter Warnung" 
// STR_TIMER genau 5 Zeichen lang 
#define STR_TIMER          "Timer"			

// STR_PPMCHANNELS je genau 4 Zeichen lang
#define STR_PPMCHANNELS	   "\0044CH 6CH 8CH 10CH12CH14CH16CH"

#define STR_MAH_ALARM      "mAh Limit"


// er9x.cpp
// ********
#define STR_LIMITS		     "GRENZEN"
#define STR_EE_LOW_MEM     "EEPROM wenig Speicher"
#define STR_ALERT		       " ALARM"
#define STR_THR_NOT_IDLE   "Gas nicht im Ruhezstd"
#define STR_RST_THROTTLE   "setze auf Standgas"
#define STR_PRESS_KEY_SKIP "bel. Taste druecken"
#define STR_ALARMS_DISABLE "Alarm ist deaktiviert"
#define STR_OLD_VER_EEPROM " EEPROM ist veraltet   TESTE EINSTELL/KALIB"
#define STR_RESET_SWITCHES "Schalter ausschalten"
#define STR_LOADING        "LAEDT"
#define STR_MESSAGE        "NACHRICHT"
#define STR_PRESS_ANY_KEY  "Taste druecken"
#define STR_MSTACK_UFLOW   "mStack uflow"
#define STR_MSTACK_OFLOW   "mStack oflow"

#define STR_CHANS_GV	     "\004P1  P2  P3  HALBVOLLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16SWCHGV1 GV2 GV3 GV4 GV5 GV6 GV7 THIS"
#define STR_CHANS_RAW	     "\004P1  P2  P3  HALBVOLLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16SWCH"
#define STR_CH	           "CH"
#define STR_TMR_MODE	     "\003AUSAN Se0Se%Ho0Ho%Ga0Ga%Qu0Qu%P1 P1%P2 P2%P3 P3%" // OFF=AUS=Timer aus ; ABS=AN=Timer an ; RUs=Se0=Seite bei 0 ; Ru%=Se%=Seite bei x% usw.
#define STR_TRIGA_OPTS			"OFFAN GasGa%"

// pers.cpp
// ********
#define STR_ME             "ICH       "
#define STR_MODEL          "MODELL    "
#define STR_BAD_EEPROM     "falsche EEprom Daten"
#define STR_EE_FORMAT      "formatiere EEPROM"
#define STR_GENWR_ERROR    "Schreibfehler"
#define STR_EE_OFLOW       "EEPROM oflow"

// templates.cpp
// ***********
#define STR_T_S_4CHAN      "Einfache 4-CH"
#define STR_T_TCUT         "Gas aus"
#define STR_T_STICK_TCUT   "dauer Gas aus"
#define STR_T_V_TAIL       "V-Leitw"
#define STR_T_ELEVON       "Delta\\Nurfluegler"
#define STR_T_HELI_SETUP   "Heli Einst"
#define STR_T_GYRO         "Gyro Einst"
#define STR_T_SERVO_TEST16 "Servo Test(16)"
#define STR_T_SERVO_TEST8  "Servo Test(8)"

// menus.cpp
// ***********
#define STR_TELEM_ITEMS	   "\004----A1= A2= RSSITSSITim1Tim2HoehGHoeGGesT1= T2= UPM TANKMah1Mah2CvltAkkuAmpsMah CtotFasVBesXBesYBesZVGesGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7FwatRxV Hdg A3= A4= SC1 SC2 SC3 SC4 TmOK"
#define STR_TELEM_SHORT    "\004----TIM1TIM2AKKUGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7"
#define STR_GV             "GV"
#define STR_OFF_ON         "AUSAN "
#define STR_HYPH_INV       "\003---UMK" // Umkehren
#define STR_VERSION        "Version"
#define STR_TRAINER        "Trainer"
#define STR_SLAVE          "\007Slave" 
#define STR_MENU_DONE      "[MENU] WENN FERTIG"
#define STR_CURVES         "Kurven"
#define STR_CURVE          "KURVE"
#define STR_GLOBAL_VAR     "GlobaleVar"
#define STR_VALUE          "Wert"
#define STR_PRESET         "VOREINST"
#define STR_CV             "KV"
#define STR_LIMITS         "GRENZEN"
#define STR_COPY_TRIM      "KOPIE TRIM [MENU]"
#define STR_TELEMETRY      "TELEMETRIE"
#define STR_USR_PROTO_UNITS "BenProto\037Units"
#define STR_USR_PROTO      "BenProto"
#define STR_FRHUB_WSHHI    "\005FrHubWSHhi"
#define STR_MET_IMP        "\003MetImp" // Metrisches System / Imperiales System
#define STR_A_CHANNEL      "A  Kanal"
#define STR_ALRM           "Alrm"
#define STR_TELEMETRY2     "TELEMETRIE2"
#define STR_TX_RSSIALRM    "SnRSSIAlrm" // Sender
#define STR_NUM_BLADES     "Num Blaetter"
//#if ALT_ALARM
//#define STR_ALT_ALARM      "HoeAlarm"
//#define STR_OFF122400      "\003AUS122400"
//#endif
#define STR_VOLT_THRES     "MaxSpannung"
#define STR_GPS_ALTMAIN    "GPSHoeheHalten"
#define STR_CUSTOM_DISP    "Ind. Bildschirm"
#define STR_FAS_OFFSET     "FAS Offset" // FrSky Amperage Sensor (FAS-100) Offset
//#define STR_VARIO_SRC_IDX  "Vario: Quelle\000\132\002\004----vGesA2  "
#define STR_VARIO_SRC      "Vario: Quelle" // Variometerquelle
#define STR_VSPD_A2        "\004----VGesA2  " // VGeschwindigkeit
#define STR_2SWITCH        "\001Schalter"
#define STR_2SENSITIVITY   "\001Empfindlichkt"
#define STR_GLOBAL_VARS    "GLOBALE VARS"
#define STR_GV_SOURCE      "\003---StmHtmGtmQtmRENSeiHoeGasQueP1 P2 P3 " // xtm=Trim for channel "x" REN=Rotary Encoder  ... = Variablennamen
#define STR_TEMPLATES      "Vorlagen"
#define STR_CHAN_ORDER     "Kanal Reihenfolge"
#define STR_SP_RETA        " SHGQ" // Seitenleitwerk=Rud Höhenleitwerk=Ele Gas=Thr Querruder=Ail
#define STR_CLEAR_MIXES    "LOESCHE MISCHER [MENU]"
#define STR_SAFETY_SW      "SICHERHEITS SCH"
#define STR_SAFETY_SW2     "Safety Sws"
#define STR_NUM_VOICE_SW   "Nummer Ton Sch"
#define STR_V_OPT1         "\007 8 Sek 12 Sek 16 Sek "
#define STR_VS             "VS" // ?
#define STR_VOICE_OPT      "\006AN    AUS   BEIDE 15Sek 30Sek 60Sek Eigene"
#define STR_VOICE_V2OPT    "\004  AN AUSBEID ALLONCE"
#define STR_CUST_SWITCH    "IND. SCHALTER" // Individueller Schalter
//#define STR_S              "S"
#define STR_15_ON          "\015An"
#define STR_EDIT_MIX       "Bearb MISCHER " // Bearbeite Mischer
#define STR_2SOURCE        "\001Quelle"
#define STR_2WEIGHT        "\001Dual Rate"
#define STR_OFFSET         "Offset"
#define STR_2FIX_OFFSET    "\001Fix Offset"
#define STR_FLMODETRIM     "\001FlModustrim"
#define STR_ENABLEEXPO		 "\001EnableExpoDR"
#define STR_2TRIM          "\001Trimmen"
#define STR_15DIFF         "\015Diff"
#define STR_Curve          "Kurve"
#define STR_2WARNING       "\001Warnung"
#define STR_2MULTIPLEX     "\001Multpx"
// STR_ADD_MULT_REP je genau 8 Zeichen
#define STR_ADD_MULT_REP   "\010Hinzufgn  MultipliziErsetzen  "
#define STR_2DELAY_DOWN    "\001Verz. runter"
#define STR_2DELAY_UP      "\001Verz. hoch"
#define STR_2SLOW_DOWN     "\001Langsam runtr"
#define STR_2SLOW_UP       "\001Langsam hoch"
#define STR_MAX_MIXERS_EXAB "Max mix erreicht: 32\037\037[EXIT] zum Abbrechen"
#define STR_MAX_MIXERS     "Max Mix erreicht: 32"
#define STR_PRESS_EXIT_AB  "[EXIT] zum Abbrechen"
#define STR_YES_NO_MENU_EXIT         "\003JA \013NEIN\037\003[MENU]\013[EXIT]"
#define STR_MENU_EXIT      "\003[MENU]\013[EXIT]"
#define STR_DELETE_MIX     "LOESCHE MISCHER?"
#define STR_MIX_POPUP      "BEARBEI\0EINFUEG\0KOPIER\0BEWEGE\0LOESCH\0CLEAR ALL"
#define STR_MIXER          "MISCHER"
// CHR_S S for Slow / Langsam
#define CHR_S              'L'
// CHR_D D for Delay / Verzögert
#define CHR_D              'V'
// CHR_d d for differential
#define CHR_d              'd'
#define STR_EXPO_DR        "Expo/Dr"
#define STR_4DR_HIMIDLO		 "\008\004DR Hoch\004DR Mitt\004DR Tief"
#define STR_4DR_MID        "\004DR Mittel"
#define STR_4DR_LOW        "\004DR Tief"
#define STR_4DR_HI         "\004DR Hoch"
#define STR_EXPO_TEXT			 "\002Expo\037\037\001Dual Rate\037\037DrSch1\037DrSch2"
#define STR_2EXPO          "\002Expo"
#define STR_DR_SW1         "DrSch1"
#define STR_DR_SW2         "DrSch2"
#define STR_DUP_MODEL      "KOPIERE MODELL"
#define STR_DELETE_MODEL   "LOESCHE MODELL"
#define STR_DUPLICATING    "Kopiere Modell"
#define STR_SETUP          "EINST"
#define STR_NAME           "Name"
#define STR_VOICE_INDEX    "Tonfreq \021MENU"
#define STR_TIMER_TEXT		 "Timer\037SignalA\037SignalB\037Timer\037\037\037Reset Switch"
#define STR_TIMER_TEXT_X	 "Timer\037SignalA\037SignalB\037Timer\037Reset Switch"
#define STR_TRIGGER        "SignalA"
#define STR_TRIGGERB       "SignalB"
//STR_COUNT_DOWN_UP indexed, 10 chars each
#define STR_COUNT_DOWN_UP  "\012Zaehl runtZaehl hoch"
#define STR_T_TRIM         "G-Trim"
#define STR_T_EXPO         "G-Expo-Dr"
#define STR_TRIM_INC       "Trim Ink"
// STR_TRIM_OPTIONS indexed 6 chars each
#define STR_TRIM_OPTIONS   "\006Expon ExFeinFein  MittelGrob  "
#ifdef V2
#define STR_TRIM_PAGE			 STR_TRIM_INC"\037"STR_TRIM_SWITCH"\037"STR_TRAINER"\037"STR_BEEP_CENTRE
#else
#define STR_TRIM_PAGE			 STR_TRIM_INC"\037"STR_TRIM_SWITCH"\037Hi.Res Slow/Delay\037"STR_TRAINER"\037"STR_BEEP_CENTRE
#endif
#define STR_TRIM_SWITCH    "Trim Sch"
#define STR_BEEP_CENTRE    "Piep Cen" // Zentrum
#define STR_RETA123        "SHGQ123"
#define STR_PROTO          "Proto" // Protokoll
// STR_21_USEC after \021 max 4 chars
#define STR_21_USEC        "\021uSek" 
#define STR_13_RXNUM       "\014EmNum" //Empfänger
// STR_23_US after \023 max 2 chars
#define STR_23_US          "\023uS"
// STR_PPMFRAME_MSEC before \015 max 9 chars, after max 4 chars
#define STR_PPMFRAME_MSEC  "PPM Laenge\015mSek" // Puls Pausen Modulation
#define STR_SEND_RX_NUM    "Bind  Range"
#define STR_DSM_TYPE       "DSM Typ" 
//#if defined(CPUM128) || defined(CPUM2561)
#define STR_PXX_TYPE       " Type\037 Chans\037 Country\037Bind\037Range"
//#else
//#define STR_PXX_TYPE       " Type\037 Country\037Bind\037Range"
//#endif
#ifdef MULTI_PROTOCOL
#define STR_MULTI_TYPE		"Protocol\037Type\037Power\037Bind     Autobind\037Range"
#define STR_MULTI_OPTION	"\013Option"
#define M_NONE_STR			"\004None"
#define M_NY_STR			"\001NY"
#define M_LH_STR			"\004HighLow "
#endif // MULTI_PROTOCOL
#define STR_1ST_CHAN_PROTO "1. Kanal\037Proto"
#define STR_PPM_1ST_CHAN   "1. Kanal"
#define STR_SHIFT_SEL      "Signalart" // Signalart
// STR_POS_NEG indexed 3 chars each
#define STR_VOL_PAGE				STR_E_LIMITS"\037""Thr. Default\037"STR_THR_REVERSE"\037""Throttle Open""\037"STR_T_TRIM"\037"STR_T_EXPO
#define STR_POS_NEG        "\003POSNEG"
#define STR_E_LIMITS       "E. Grenze" //Erweiterte Grenze
#define STR_Trainer        "Trainer"
#define STR_T2THTRIG       "GasStaT2" // 2. Timer startet wenn Gas um 5% bewegt
#define STR_AUTO_LIMITS    "Autogrenze"
// STR_1_RETA indexed 1 char each
#define STR_1_RETA         "\001SHGQ"
#define STR_FL_MODE        "FL MODUS"
#define STR_SWITCH_TRIMS   "Schalter\037Trimmer"
#define STR_SWITCH         "Schalter"
#define STR_TRIMS          "Trimmer"
#define STR_MODES          "MODI"
#define STR_SP_FM0         " FM0"
#define STR_SP_FM          " FM"
#define STR_HELI_SETUP     "HELI EINST"
#define STR_HELI_TEXT			 "Taumeltyp\037Kollektiv\037Anschlag\037HOE Umkehr\037QUE Umkehr\037COL Direction"
#define STR_SWASH_TYPE     "Taumeltyp" 
#define STR_COLLECTIVE     "Kollektiv"
#define STR_SWASH_RING     "Anschlag"
#define STR_ELE_DIRECTION  "HOE Umkehr"
#define STR_AIL_DIRECTION  "QUE Umkehr"
#define STR_COL_DIRECTION  "KOL Umkehr" //Kollektiv
#define STR_MODEL_POPUP    "EDIT\0BEARBEI\0SEL/EDIT\0KOPIER\0BEWEGE\0LOESCH\0BACKUP\0RESTORE"
#define STR_MODELSEL       "MODELWAHL"
// STR_11_FREE after \011 max 4 chars
#define STR_11_FREE        "\011frei"
#define STR_CALIBRATION    "Kalibrierung"
// STR_MENU_TO_START after \003 max 15 chars
#define STR_MENU_TO_START  "\003[MENU] ZUM START"
// STR_SET_MIDPOINT after \005 max 11 chars
#define STR_SET_MIDPOINT   "\005SET MITPUNKT"
// STR_MOVE_STICKS after \003 max 15 chars
#define STR_MOVE_STICKS    "\003BEWG STICKS/POTS"
#define STR_ANA            "ANA" // Analog Input und Batterie Spannung Kalibrierung
#define STR_DIAG           "DIAG" // Diagnostics
#define STR_KEYNAMES       "Links\037Recht\037 Hoch\037Runtr\037 Exit\037Menue"
#define STR_TRIM_M_P       "Trim- +"
// STR_OFF_PLUS_EQ indexed 3 chars each
#define STR_OFF_PLUS_EQ    "\003aus += :="
// STR_CH1_4 indexed 3 chars each
#define STR_CH1_4          "\003ch1ch2ch3ch4"
#define STR_MULTIPLIER     "Multiplika"
#define STR_CAL            "Kal"
#define STR_MODE_SRC_SW    "\003mode\012% que sch" // Quelle Schalter
#define STR_RADIO_SETUP    "General"
#define STR_OWNER_NAME     "Nutzername"
#define STR_BEEPER         "Pieper"
// STR_BEEP_MODES indexed 6 chars each
#define STR_BEEP_MODES     "\006Lautls""TstAus""xKurz ""Kurz  ""Normal""Lang  ""xLang " // x = seh
#define STR_SOUND_MODE     "Soundmodus"
// STR_SPEAKER_OPTS indexed 10 chars each
#define STR_SPEAKER_OPTS   "\012Pieper    ""PiLautspre""PieprTon  ""PieLautTon""MegaSound "
#define STR_VOLUME         "Lautst"
#define STR_SPEAKER_PITCH  "Tonhoehe"
#define STR_HAPTICSTRENGTH "Haptische Staerke"
#define STR_CONTRAST       "Kontrast"
#define STR_BATT_WARN      "Batterie Warnung" 
// STR_INACT_ALARM m for minutes after \023 - single char
#define STR_INACT_ALARM    "Inaktivitatalarm\023m"
#define STR_THR_REVERSE    "Gasumkehr"
#define STR_MINUTE_BEEP    "Minutenton"
#define STR_BEEP_COUNTDOWN "Piep Countdown"
#define STR_FLASH_ON_BEEP  "Blitz bei Piep"
#define STR_LIGHT_SW_TEXT  "Lichtschalter\037\037Licht aus nach\023s\037Licht an Stkbeweg\023s"
#define STR_LIGHT_SWITCH   "Lichtschalter"
#define STR_LIGHT_INVERT   "Licht umkehren"
#define STR_LIGHT_AFTER    "Licht aus nach\023s"
#define STR_LIGHT_STICK    "Licht an Stkbeweg\023s"
#define STR_SPLASH_SCREEN  "Startbildschirm"
#define STR_SPLASH_NAME    "Start Name"
#define STR_THR_WARNING    "Gas Warnung"
#define STR_DEAFULT_SW_PAGE "Stdr.Schalt\037CustomStkNames\037Autogrenze\037Throttle Default"
#define STR_DEAFULT_SW     "Stdr.Schalt"
#define STR_MEM_WARN       "Speicher Warnung"
#define STR_ALARM_WARN     "Alarm Warnung"
#define STR_POTSCROLL      "PotScroll"
#define STR_STICKSCROLL    "StickScroll"
#define STR_BANDGAP        "BandLuecke"
#define STR_ENABLE_PPMSIM  "Aktiviere PPMSIM"
#define STR_CROSSTRIM      "KreuzTrim"
#define STR_INT_FRSKY_ALRM "Int. Frsky alarm"
#define STR_MODE           "Modus"

// SWITCHES_STR 3 chars each
#ifdef XSW_MOD
#define SWITCHES_STR       "\003IDLGASSEIHOEQUEFWKPB1PB2TRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI ID0ID1ID2TH\200TH-TH\201RU\200RU-RU\201EL\200EL-EL\201AI\200AI-AI\201GE\200GE-GE\201"
#else // !XSW_MOD
#if defined(CPUM128) || defined(CPUM2561)
#define SWITCHES_STR       "\003GASSEIHOEID0ID1ID2QUEFWKTRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI EL\200EL-EL\201RU\200RU-RU\201AI\200AI-AI\201GE\200GE-GE\201PB1PB2"
#else
#define SWITCHES_STR       "\003GASSEIHOEID0ID1ID2QUEFWKTRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC "
#endif
#endif  // XSW_MOD

#define SWITCH_WARN_STR	   "Schalter Warnung"
// CURV_STR indexed 3 chars each
#define CURV_STR           "\003---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16"
// CSWITCH_STR indexed 7 chars each
#ifdef VERSION3
#if defined(CPUM128) || defined(CPUM2561)
#define CSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    v1==v2 v1!=v2 v1>v2  v1<v2  Latch  F-Flop TimeOffv\140=val 1-Shot 1-ShotR"
#else
#define CSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    v1==v2 v1!=v2 v1>v2  v1<v2  Latch  F-Flop TimeOffv\140=val "
#endif
#else
#define CSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valUND    ODER   XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""v1>=v2 ""v1<=v2 ZeitAusv1\140=val"
#endif

#define SWASH_TYPE_STR     "\004----""120 ""120X""140 ""90  "

#if defined(CPUM128) || defined(CPUM2561)
#define STR_STICK_NAMES    "Sei \0Hoe \0Gas \0Que "
#else
#define STR_STICK_NAMES    "Sei Hoe Gas Que "
#endif

#define STR_STAT           "STAT"
#define STR_STAT2          "STAT2"
// STR_TRIM_OPTS indexed 3 chars each
#define STR_TRIM_OPTS      "\003ExpxFeFeiMitStk" // ExF= extra fein; Fne = fein; Med = medium; Crs = Coarse sehr stark
#define STR_TTM            "GTm" // Gas Trim
#define STR_FUEL           "TANK"
#define STR_12_RPM         "\012UPM"
#define STR_LAT_EQ         "Bre="
#define STR_LON_EQ         "Lae="
#define STR_ALT_MAX        "Hoe=\011m   Max="
#define STR_SPD_KTS_MAX    "Ges=\011kts Max="
#define STR_11_MPH         "\011mph"

#define STR_SINK_TONES		 "Sink Tones"
//#define STR_NO_SINK_TONES  "Kein Sinkton"
#define STR_FRSKY_MOD      "Frsky Mod Fertig"
#define STR_TEZ_R90				 "TelemetrEZ>=r90"

// ersky9x strings
#define STR_ST_CARD_STAT   "SD CARD STAT"
#define STR_4_READY        "\004Bereit"
#define STR_NOT            "NICHT"
#define STR_BOOT_REASON    "BOOT GRUND"
#define STR_6_WATCHDOG     "\006WAECHTER"
#define STR_5_UNEXPECTED   "\005UNERWARTET"
#define STR_6_SHUTDOWN     "\006AUSSCHALTEN"
#define STR_6_POWER_ON     "\006EINSCHALTEN"
// STR_MONTHS indexed 3 chars each
#define STR_MONTHS         "\003XxxJanFebMrzAprMaiJunJulAugSepOktNovDez"
#define STR_MENU_REFRESH   "[MENU] NEU LADEN"
#define STR_DATE_TIME      "DATUM-ZEIT"
#define STR_SEC            "Sek."
#define STR_MIN_SET        "Min.\015Set"
#define STR_HOUR_MENU_LONG "Std.\012MENU LANG"
#define STR_DATE           "Datum"
#define STR_MONTH          "Monat"
#define STR_YEAR_TEMP      "Jahr\013Temp."
#define STR_YEAR           "Jahr"
#define STR_BATTERY        "BATTERIE"
#define STR_Battery        "Batterie"
#define STR_CURRENT_MAX    "Momentan\016Max"
#define STR_CPU_TEMP_MAX   "CPU temp.\014C Max\024C"
#define STR_MEMORY_STAT    "SPEICHER STAT"
#define STR_GENERAL        "Generell"
#define STR_Model          "Modell"
#define STR_RADIO_SETUP2   "FERNST EINST2"
#define STR_BRIGHTNESS     "Helligkeit"
#define STR_CAPACITY_ALARM "Kapazitaet Alarm"
#define STR_BT_BAUDRATE    "Bt baudrate" 
#define STR_ROTARY_DIVISOR "Rot Teiler"
#define STR_STICK_LV_GAIN  "Stick LV Anstieg"
#define STR_STICK_LH_GAIN  "Stick LH Anstieg"
#define STR_STICK_RV_GAIN  "Stick RV Anstieg"
#define STR_STICK_RH_GAIN  "Stick RH Anstieg"

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


/* Extra data for MavLink via FrSky
*/
#define STR_MAV_FM_0     "STAB"
#define STR_MAV_FM_1     "ACRO"
#define STR_MAV_FM_2     "A-Hold"
#define STR_MAV_FM_3     "AUTO"
#define STR_MAV_FM_4     "GUIDED"
#define STR_MAV_FM_5     "LOITER"
#define STR_MAV_FM_6   	 "RTL"
#define STR_MAV_FM_7     "CIRCLE"
#define STR_MAV_FM_8     "MODE8"
#define STR_MAV_FM_9 		 "LAND"
#define STR_MAV_ARMED  		 "ARMED"
#define STR_MAV_DISARMED  	 "DISARM"
#define STR_MAV_NODATA 		 "NODATA"
#define STR_MAV_CURRENT      "Cur"
#define STR_MAV_GPS_NO_GPS   "No GPS"
#define STR_MAV_GPS      	 "GPS:"
#define STR_MAV_GPS_NO_FIX	 "No Fix" // 1
#define STR_MAV_GPS_2DFIX	 "2D Fix" // 2
#define STR_MAV_GPS_3DFIX	 "3D Fix" // 3
#define STR_MAV_GPS_SAT_COUNT "sat"
#define STR_MAV_GPS_HDOP     "hdop"
#define STR_MAV_THR_OUT	     "THR%"
#define STR_MAV_WP_DIST "WP"
#define STR_MAV_ALT          "alt"
#define STR_MAV_GALT         "gAl"
#define STR_MAV_HOME         "dth"
#define STR_MAV_CPU          "cpu"
#define STR_RSSI		     "Rssi"
#define STR_RCQ		         "Rcq"
#define STR_MAV_HEALTH  "Health"
#define STR_MAV_OK  "Ok"
/* Extra data for er9x FrSky+MavLink firmware End  */
#if defined(CPUM128) || defined(CPUM2561)
// Actual for THISFIRMWARE "ArduCopter V3.3-dev"
// check actual data at https://github.com/diydrones/ardupilot/search?q=SEVERITY_HIGH
#define STR_MAV_ERR_01 "    ARMING MOTORS    "
#define STR_MAV_ERR_02 "PreArm: RC not calibr"
#define STR_MAV_ERR_03 "PreArm: BaroBadHealth"
#define STR_MAV_ERR_04 "PreArm: CompassHealth"
#define STR_MAV_ERR_05 "PreArm: Bad GPS Pos  "
#define STR_MAV_ERR_06 "   Compass disabled  "
#define STR_MAV_ERR_07 "    Check compass    "
#define STR_MAV_ERR_08 "MotorTest:RC NotCalib"
#define STR_MAV_ERR_09 "MotorTest: Not landed"
#define STR_MAV_ERR_10 "   Crash: Disarming  "
#define STR_MAV_ERR_11 "Parachute: Released! "
#define STR_MAV_ERR_12 "Error SettingRallyPnt"
#define STR_MAV_ERR_13 "  AutoTune: Started  "
#define STR_MAV_ERR_14 "  AutoTune: Stopped  "
#define STR_MAV_ERR_15 "      Trim saved     "
#define STR_MAV_ERR_16 "EKF: Compass Variance"
#define STR_MAV_ERR_17 "Verify: InvalidNavCMD"
#define STR_MAV_ERR_18 "Verify: InvlidCondCMD"
#define STR_MAV_ERR_19 "Disable fence failed "
#define STR_MAV_ERR_20 "ESC Cal:Restart Board"
#define STR_MAV_ERR_21 "ESC Cal:PassThrToESCs"
#define STR_MAV_ERR_22 "---- Low Battery ----"
#define STR_MAV_ERR_23 "----- Lost GPS ------"
#define STR_MAV_ERR_24 "  Fence autodisabled "
#endif
/* Extra data for er9x FrSky+MavLink firmware End  */


