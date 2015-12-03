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


#define STR_ON             "ON "
#define STR_OFF            "OFF"

#define STR_ALTEQ	         "Alt=" 
#define STR_TXEQ		       "\003Tx=Swr"
#define STR_RXEQ		       "Rx="
#define STR_RX  		       "Rx"
#define STR_TRE012AG	     "TRE012AGG"

// STR_YELORGRED indexed 3 char each
#define STR_YELORGRED	     "\003---JauOrgRou"
#define STR_A_EQ		       "A ="
#define STR_SOUNDS	       "\006Warn1 Warn2 Cheep Ring  SciFi Robot Chirp Tada  CricktSiren AlmClkRatataTick  Haptc1Haptc2Haptc3"
#define STR_SWITCH_WARN	   "Alerte inters"
// STR_TIMER exactly 5 chars long
#define STR_TIMER          "Timer"

// STR_PPMCHANNELS indexed 4 char each
#define STR_PPMCHANNELS	   "\0044CH 6CH 8CH 10CH12CH14CH16CH"

#define STR_MAH_ALARM      "Limite mAh"


// er9x.cpp
// ********
#define STR_LIMITS		   "Limites"
#define STR_EE_LOW_MEM     "Espace EEPROM faible"
#define STR_ALERT	       " ALERTE"
#define STR_THR_NOT_IDLE   "Manche des gaz"
#define STR_RST_THROTTLE   "Mettre les gaz a 0"
#define STR_PRESS_KEY_SKIP "ou presser une touche"
#define STR_ALARMS_DISABLE "Alarmes desactivees"
#define STR_OLD_VER_EEPROM " Old Version EEPROM   CHECK SETTINGS/CALIB"
#define STR_RESET_SWITCHES "Re-init. inters"
#define STR_LOADING        "CHARGEMENT"
#define STR_MESSAGE        "MESSAGE"
#define STR_PRESS_ANY_KEY  "Presser une touche"
#define STR_MSTACK_UFLOW   "mStack uflow"
#define STR_MSTACK_OFLOW   "mStack oflow"

#define STR_CHANS_GV	     "\004P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16SWCHGV1 GV2 GV3 GV4 GV5 GV6 GV7 THIS"
#define STR_CHANS_RAW	     "\004P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16SWCH"
#define STR_CH	           "CH"
//#define STR_TMR_MODE	     "\003OFFON RUsRU%ELsEL%THsTH%ALsAL%P1 P1%P2 P2%P3 P3%"
#define STR_TRIGA_OPTS			"OFFON THsTH%"

// pers.cpp
// ********
#define STR_ME             "MOI       "
#define STR_MODEL          "MODELE    "
#define STR_BAD_EEPROM     "Donnees EEprom KO"
#define STR_EE_FORMAT      "Formatage EEPROM"
#define STR_GENWR_ERROR    "Erreur d'ecriture"
#define STR_EE_OFLOW       "EEPROM saturee"

// templates.cpp
// ***********
#define STR_T_S_4CHAN      "4 voies simple"
#define STR_T_TCUT         "Coupure Gaz"
#define STR_T_STICK_TCUT   "Coup. Gaz Manche"
#define STR_T_V_TAIL       "Empennage V"
#define STR_T_ELEVON       "Elevon/Delta"
#define STR_T_HELI_SETUP   "Config Heli"
#define STR_T_GYRO         "Config Gyro"
#define STR_T_SERVO_TEST16 "Test Servos(16)"
#define STR_T_SERVO_TEST8  "Test Servos(8)"

// menus.cpp
// ***********
#define STR_TELEM_ITEMS	   "\004----A1= A2= RSSITSSITim1Tim2Alt GaltGspdT1= T2= RPM FUELMah1Mah2CvltBattAmpsMah CtotFasVAccXAccYAccZVspdGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7FwatRxV Hdg A3= A4= SC1 SC2 SC3 SC4 TmOK"
#define STR_TELEM_SHORT    "\004----TIM1TIM2BATTGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7"
#define STR_GV             "GV"
#define STR_OFF_ON         "OFFON "
#define STR_HYPH_INV       "\003---INV"
#define STR_VERSION        "Version"
#define STR_TRAINER        "Ecolage"
#define STR_SLAVE          "\007Slave" 
#define STR_MENU_DONE      "[MENU] si OK"
#define STR_CURVES         "Courbes"
#define STR_CURVE          "COURB"
#define STR_GLOBAL_VAR     "GLOBAL VAR"
#define STR_VALUE          "Valeur"
#define STR_PRESET         "PRESET"
#define STR_CV             "CV"
#define STR_XLIMITS        "LIMITES"
#define STR_COPY_TRIM      "COPY TRIM [MENU]"
#define STR_TELEMETRY      "Telemetrie"
#define STR_USR_PROTO_UNITS "UsrProto\037Unites"
#define STR_USR_PROTO		"UsrProto"
#define STR_FRHUB_WSHHI    "\005FrHubWSHhi"
#define STR_MET_IMP        "\003MetImp"
#define STR_A_CHANNEL      "A  chan."
#define STR_ALRM           "alrm"
#define STR_TELEMETRY2     "TELEMETRIE2"
#define STR_TX_RSSIALRM    "TxRSSIalrm"
#define STR_NUM_BLADES     "Nb. pales"
//#if ALT_ALARM
//#define STR_ALT_ALARM      "AltAlarm"
//#define STR_OFF122400      "\003OFF122400"
//#endif
#define STR_VOLT_THRES     "Seuil Volt="
#define STR_GPS_ALTMAIN    "GpsAltMain"
#define STR_CUSTOM_DISP    "Affich. Perso"
#define STR_FAS_OFFSET     "FAS Offset"
//#define STR_VARIO_SRC_IDX  "Vario: Source\000\132\002\004----vspdA2  "
#define STR_VARIO_SRC      "Vario: Source"
#define STR_VSPD_A2        "\004----vspdA2  "
#define STR_2SWITCH        "\001Inter "
#define STR_2SENSITIVITY   "\001Sensibilite"
#define STR_GLOBAL_VARS    "GlobalVars"
#define STR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 P3 "
#define STR_TEMPLATES      "Gabarits"
#define STR_CHAN_ORDER     "Ordre des voies"
#define STR_SP_RETA        " RETA"
#define STR_CLEAR_MIXES    "SUPPR. MIXES [MENU]"
#define STR_SAFETY_SW      "Inters Securite"
#define STR_SAFETY_SW2     "Inter/Secu"
#define STR_NUM_VOICE_SW   "Number Voice Sw"
#define STR_V_OPT1         "\007 8 Secs12 Secs16 Secs"
#define STR_VS             "VS"
#define STR_VOICE_OPT      "\006ON    OFF   BOTH  15Secs30Secs60SecsVaribl"
#define STR_VOICE_V2OPT    "\004ON  OFF BOTH  ALL ONCE"
#define STR_CUST_SWITCH    "INTERS LOGIQUES"
//#define STR_S              "S"
#define STR_15_ON          "\015On"
#define STR_EDIT_MIX       "EDIT. MIX"
#define STR_2SOURCE        "\001Source"
#define STR_2WEIGHT        "\001Ratio"
#define STR_OFFSET         "Offset"
#define STR_2FIX_OFFSET    "\001Fix Offset"
#define STR_FLMODETRIM     "\001FlModetrim"
#define STR_ENABLEEXPO	   "\001ActiveExpoDR"
#define STR_2TRIM          "\001Trim"
#define STR_15DIFF         "\010Diff"
#define STR_Curve          "Courbe"
#define STR_2WARNING       "\001Alerte"
#define STR_2MULTIPLEX     "\001Multpx"
// STR_ADD_MULT_REP indexed 8 chars each
#define STR_ADD_MULT_REP   "\010Ajoute  Multipl.Remplace"
#define STR_2DELAY_DOWN    "\001Delay Down"
#define STR_2DELAY_UP      "\001Delay Up"
#define STR_2SLOW_DOWN     "\001Slow  Down"
#define STR_2SLOW_UP       "\001Slow  Up"
#define STR_MAX_MIXERS_EXAB "Nb mixages maxi : 32\037\037[EXIT] : annuler  "
#define STR_MAX_MIXERS     "Nb mixages maxi : 32"
#define STR_PRESS_EXIT_AB  "[EXIT] : annuler  "
#define STR_YES_NO_MENU_EXIT         "\003OUI\013NON\037\003[MENU]\013[EXIT]"
#define STR_MENU_EXIT      "\003[MENU]\013[EXIT]"
#define STR_DELETE_MIX     "Suppr. mix?"
#define STR_MIX_POPUP      "EDITER\0INSERER\0COPIER\0DEPLACER\0SUPPRIMER\0SUPP.TOUT"
#define STR_MIXER          "MIXAGES"
// CHR_S S for Slow
#define CHR_S              'S'
// CHR_D D for Delay
#define CHR_D              'D'
// CHR_d d for differential
#define CHR_d              'd'
#define STR_EXPO_DR        "Expo/Dr"
#define STR_4DR_HIMIDLO		 "\003Hi Mid Low"
#define STR_4DR_MID        "\004DR Mid"
#define STR_4DR_LOW        "\004DR Low"
#define STR_4DR_HI         "\004DR Hi"
#define STR_EXPO_TEXT			 "\004DR\037\002Expo\037\037\001Ratio\037\037DrSw1\037DrSw2"
#define STR_2EXPO          "\002Expo"
#define STR_DR_SW1         "DrSw1"
#define STR_DR_SW2         "DrSw2"
#define STR_DUP_MODEL      "COPIER MODELE"
#define STR_DELETE_MODEL   "SUPPR. MODELE"
#define STR_DUPLICATING    "Copie en cours"
#define STR_SETUP          "SETUP"
#define STR_NAME           "Nom"
#define STR_VOICE_INDEX    "Fich. audio\021MENU"
#define STR_TIMER_TEXT		 "Timer\037TriggerA\037TriggerB\037Timer\037\037\037Inter/RAZ"
#define STR_TIMER_TEXT_X	 "Timer\037TriggerA\037TriggerB\037Timer\037Inter/RAZ"
#define STR_TRIGGER        "TriggerA"
#define STR_TRIGGERB       "TriggerB"
//STR_COUNT_DOWN_UP indexed, 10 chars each
#define STR_COUNT_DOWN_UP  "\012DecrementeIncremente"
#define STR_T_TRIM         "Trim gaz"
#define STR_T_EXPO         "T-Expo-Dr"
#define STR_TRIM_INC       "Trim Inc"
// STR_TRIM_OPTIONS indexed 6 chars each
#define STR_TRIM_OPTIONS   "\006Exp   ExFineFine  MediumCoarse"
#ifdef V2
#define STR_TRIM_PAGE			 STR_TRIM_INC"\037"STR_TRIM_SWITCH"\037"STR_TRAINER"\037"STR_BEEP_CENTRE
#else
#define STR_TRIM_PAGE			 STR_TRIM_INC"\037"STR_TRIM_SWITCH"\037Hi.Res Slow/Delay\037"STR_TRAINER"\037"STR_BEEP_CENTRE
#endif
#define STR_TRIM_SWITCH    "Insta-trim"
#define STR_BEEP_CENTRE    "Beep Cnt"
#define STR_RETA123        "RETA123"
#define STR_PROTO          "Codag"
// STR_21_USEC after \021 max 4 chars
#define STR_21_USEC        "\021uSec"
#define STR_13_RXNUM       "\014RxNum"
// STR_23_US after \023 max 2 chars
#define STR_23_US          "\023uS"
// STR_PPMFRAME_MSEC before \015 max 9 chars, after max 4 chars
#define STR_PPMFRAME_MSEC  "Trame PPM\015mSec"
#define STR_SEND_RX_NUM    "Bind  Portee"
#define STR_DSM_TYPE       "Type DSM"
#define STR_PXX_TYPE       " Type\037 Region \037Bind\037Portee"
#ifdef MULTI_PROTOCOL
#define STR_MULTI_TYPE		"Codage\037Type\037Power\037Bind     Autobind\037Portee"
#define STR_MULTI_OPTION	"\013Option"
#define M_NONE_STR			"\004N/A"
#define M_NY_STR			"\001NY"
#define M_LH_STR			"\004HighLow "
#endif // MULTI_PROTOCOL
#define STR_1ST_CHAN_PROTO "1ere voie\037Codag"
#define STR_PPM_1ST_CHAN   "1ere voie"
#define STR_SHIFT_SEL      "Polarite"
// STR_POS_NEG indexed 3 chars each
#define STR_POS_NEG        "\003POSNEG"
#define STR_VOL_PAGE				STR_E_LIMITS"\037""Zero gaz\037"STR_THR_REVERSE"\037""Pleins gaz""\037"STR_T_TRIM"\037"STR_T_EXPO
#define STR_E_LIMITS       "Limites etendues"
#define STR_Trainer        "Ecolage"
#define STR_T2THTRIG       "T2ThTrig"
#define STR_AUTO_LIMITS    "Limites auto"
// STR_1_RETA indexed 1 char each
#define STR_1_RETA         "\001RETA"
#define STR_FL_MODE        "FL MODE"
#define STR_SWITCH_TRIMS   "Inter\037Trims"
#define STR_SWITCH         "Inter"
#define STR_TRIMS          "Trims"
#define STR_MODES          "Modes"
#define STR_SP_FM0         " FM0"
#define STR_SP_FM          " FM"
#define STR_HELI_SETUP     "Conf. Heli"
#define STR_HELI_TEXT			 "Type Plateau\037Collectif\037Swash Ring\037Direction ELE\037Direction AIL\037Direction COL"
#define STR_SWASH_TYPE     "Type Plateau"
#define STR_COLLECTIVE     "Collectif"
#define STR_SWASH_RING     "Swash Ring"
#define STR_ELE_DIRECTION  "Direction ELE"
#define STR_AIL_DIRECTION  "Direction AIL"
#define STR_COL_DIRECTION  "Direction COL"

#define STR_MODEL_POPUP    "EDITER\0SELECT.\0SEL/EDIT\0COPIER\0DEPLACER\0SUPPRIM.\0SAUVEGA.\0RESTAUR."
#define STR_MODELSEL       "MODELSEL"
// STR_11_FREE after \011 max 4 chars
#define STR_11_FREE        "\011disp"
#define STR_CALIBRATION    "Etalonnage"
// STR_MENU_TO_START after \003 max 15 chars
#define STR_MENU_TO_START  "\003[MENU] PR DEBUT"
// STR_SET_MIDPOINT after \005 max 11 chars
#define STR_SET_MIDPOINT   "\005REG NEUTRES"
// STR_MOVE_STICKS after \003 max 15 chars
#define STR_MOVE_STICKS    "\003BUTEES MAN/POTS"
#define STR_ANA            "ANA"
#define STR_DIAG           "DIAG"
#define STR_KEYNAMES       " Left\037Right\037\003Up\037 Down\037 Exit\037 Menu"
#define STR_TRIM_M_P       "Trim- +"
// STR_OFF_PLUS_EQ indexed 3 chars each
#define STR_OFF_PLUS_EQ    "\003off += :="
// STR_CH1_4 indexed 3 chars each
#define STR_CH1_4          "\003ch1ch2ch3ch4"
#define STR_MULTIPLIER     "Multiplier"
#define STR_CAL            "Cal"
#define STR_MODE_SRC_SW    "\003mode\012% src  sw"
#define STR_RADIO_SETUP    "General"
#define STR_OWNER_NAME     "Pilote"
#define STR_BEEPER         "Beeper"
// STR_BEEP_MODES indexed 6 chars each
#define STR_BEEP_MODES     "\006Quiet NoKey xShortShort Norm  Long  xLong "
#define STR_SOUND_MODE     "Mode audio"
// STR_SPEAKER_OPTS indexed 10 chars each
#define STR_SPEAKER_OPTS   "\012Beeper    PiSpkr    BeeprVoicePiSpkVoiceMegaSound "
#define STR_VOLUME         "Volume"
#define STR_SPEAKER_PITCH  " Tonalite"
#define STR_HAPTICSTRENGTH " Vibreur"
#define STR_CONTRAST       "Contraste"
#define STR_BATT_WARN      "Alerte batterie"
// STR_INACT_ALARM m for minutes after \023 - single char
#define STR_INACT_ALARM    "Alarme inactivite\023m"
#define STR_THR_REVERSE    "Inverser gaz"
#define STR_MINUTE_BEEP    "Bip/minute"
#define STR_BEEP_COUNTDOWN "Bip/decompte"
#define STR_FLASH_ON_BEEP  "Flash av bip"
#define STR_LIGHT_SWITCH   "Inter eclairage"
#define STR_LIGHT_INVERT   "Inv. retroecl."
#define STR_LIGHT_AFTER    "Coupure apres :\023s"
#define STR_LIGHT_STICK    "Rallumer si mvmt\023s"
#define STR_SPLASH_SCREEN  "Logo Accueil"
#define STR_SPLASH_NAME    "Affiche Nom"
#define STR_THR_WARNING    "Alerte gaz"
#define STR_DEAFULT_SW_PAGE "Int/defaut\037Renom. manches\037Limites Auto\037Controle volume"
#define STR_DEAFULT_SW     "Int/defaut"
#define STR_MEM_WARN       "Alerte memoire"
#define STR_ALARM_WARN     "Alerte alarmes"
#define STR_POTSCROLL      "Defil./Pot."
#define STR_STICKSCROLL    "Defil./Manche"
#define STR_BANDGAP        "BandGap"
#define STR_ENABLE_PPMSIM  "Activer PPMSIM"
#define STR_CROSSTRIM      "Croiser Trim"
#define STR_INT_FRSKY_ALRM "Alarme Frsky Int."
#define STR_MODE           "Mode"

// SWITCHES_STR 3 chars each
#ifdef XSW_MOD
#define SWITCHES_STR       "\003IDLTHRRUDELEAILGEAPB1PB2TRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI ID0ID1ID2TH\200TH-TH\201RU\200RU-RU\201EL\200EL-EL\201AI\200AI-AI\201GE\200GE-GE\201"
#else	// !XSW_MOD
#if defined(CPUM128) || defined(CPUM2561)
#define SWITCHES_STR       "\003THRRUDELEID0ID1ID2AILGEATRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI EL\200EL-EL\201RU\200RU-RU\201AI\200AI-AI\201GE\200GE-GE\201PB1PB2"
#else
#define SWITCHES_STR       "\003THRRUDELEID0ID1ID2AILGEATRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC "
#endif
#endif  // XSW_MOD

#define SWITCH_WARN_STR	   "Alerte Inters"
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
#define CSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    v1==v2 v1!=v2 v1>v2  v1<v2  v1>=v2 v1<=v2 TimeOffv\140=val "
#endif

#define SWASH_TYPE_STR     "\004----""120 ""120X""140 ""90  "

#if defined(CPUM128) || defined(CPUM2561)
#define STR_STICK_NAMES    "Rud \0Ele \0Thr \0Ail "
#else
#define STR_STICK_NAMES    "Rud Ele Thr Ail "
#endif

#define STR_STAT           "STAT"
#define STR_STAT2          "STAT2"
// STR_TRIM_OPTS indexed 3 chars each
#define STR_TRIM_OPTS      "\003ExpExFFneMedCrs"
#define STR_TTM            "TTm"
#define STR_FUEL           "Fuel"
#define STR_12_RPM         "\012RPM"
#define STR_LAT_EQ         "Lat=\022Hdg"
#define STR_LON_EQ         "Lon="
#define STR_ALT_MAX        "Alt=\011m   Max="
#define STR_SPD_KTS_MAX    "Vit=\011kts Max="
#define STR_11_MPH         "\011mph"

#define STR_SINK_TONES	   "Bip/descente"
#define STR_FRSKY_MOD      "Mod Frsky faite"
#define STR_TEZ_R90				 "TelemetrEZ>=r90"

// ersky9x strings
#define STR_ST_CARD_STAT   "SD CARD STAT"
#define STR_4_READY        "\004Pret"
#define STR_NOT            "NOT"
#define STR_BOOT_REASON    "BOOT REASON"
#define STR_6_WATCHDOG     "\006WATCHDOG"
#define STR_5_UNEXPECTED   "\005UNEXPECTED"
#define STR_6_SHUTDOWN     "\006SHUTDOWN"
#define STR_6_POWER_ON     "\006POWER ON"
// STR_MONTHS indexed 3 chars each
#define STR_MONTHS         "\003XxxJanFebMarAprMayJunJulAugSepOctNovDec"
#define STR_MENU_REFRESH   "[MENU] pour rafraichir"
#define STR_DATE_TIME      "DATE-HEURE"
#define STR_SEC            "Sec."
#define STR_MIN_SET        "Min.\015Set"
#define STR_HOUR_MENU_LONG "Heure\012MENU LONG"
#define STR_DATE           "Date"
#define STR_MONTH          "Mois"
#define STR_YEAR_TEMP      "Annee\013Temp."
#define STR_YEAR           "Annee"
#define STR_BATTERY        "BATTERIE"
#define STR_Battery        "Batterie"
#define STR_CURRENT_MAX    "Courant\016Max"
#define STR_CPU_TEMP_MAX   "temp. CPU\014C Max\024C"
#define STR_MEMORY_STAT    "MEMORY STAT"
#define STR_GENERAL        "General"
#define STR_Model          "Modele"
#define STR_RADIO_SETUP2   "CONF. RADIO2"
#define STR_BRIGHTNESS     "Luminosite"
#define STR_CAPACITY_ALARM "Capacity Alarm"
#define STR_BT_BAUDRATE    "Bt baudrate"
#define STR_ROTARY_DIVISOR "Rotary Divisor"
#define STR_STICK_LV_GAIN  "Stick LV Gain"
#define STR_STICK_LH_GAIN  "Stick LH Gain"
#define STR_STICK_RV_GAIN  "Stick RV Gain"
#define STR_STICK_RH_GAIN  "Stick RH Gain"

#define STR_DISPLAY	   "Affichage"
#define STR_PROTOCOL	   "Codage"
#define STR_HARDWARE	   "Hardware"
#define STR_ALARMS  	   "Alarmes"
#define STR_CONTROLS  	   "Controles"
#define STR_AUDIOHAPTIC	   "Audio/Vibr."
#define STR_DIAGSWTCH	   "DiagInters"
#define STR_DIAGANA 	   "DiagAnalog"
#define STR_MIXER2  	   "Mixages"
#define STR_CSWITCHES 	   "Inters Log."
#define STR_VOICE   	   "Voix/Audio"
#define STR_VOICEALA   	   "Alarmes Vocales"
#define STR_CLEAR_ALL_MIXES "Suppr. TOUS les mix?"

#define STR_MAIN_POPUP		"Sel. Modele \0Conf.Modele\0Menu prec\0Conf. Radio\0Stats"
#define MODEL_SETUP_OFFSET	13
#define RADIO_SETUP_OFFSET	35


