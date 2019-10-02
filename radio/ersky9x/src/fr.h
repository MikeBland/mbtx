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
// é  use \300
// è  use \301
// à  use \302
// î  use \303
// ç  use \304


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


#define ISTR_ON             	"ON "
#define ISTR_OFF            	"OFF"
#define ISTR_X_OFF_ON				FWx17 "\001" "\003" ISTR_OFF ISTR_ON

#define ISTR_ALTEQ	        "Alt=" 
#define ISTR_TXEQ		"\003Tx=Swr"
#define ISTR_RXEQ		"Rx="
#define ISTR_TRE012AG	     	"TRE012AG"

// ISTR_YELORGRED indexed 3 char each
#define ISTR_YELORGRED	     	"\003---JauOraRou"
#define ISTR_A_EQ		"A ="
#define ISTR_SOUNDS	       "\006Warn1 ""Warn2 ""Cheap ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Crickt""Siren ""AlmClk""Ratata""Tick  ""Haptc1""Haptc2""Haptc3" "Haptc4"
#define ISTR_SWITCH_WARN	"Alerte Inter"
//#define ISTR_SWITCH				"Inter"
#define ISTR_WARNING			"Alerte"
// ISTR_TIMER exactly 5 chars long
#define ISTR_TIMER          	"Chrono"

// ISTR_PPMCHANNELS indexed 4 char each
#define ISTR_PPMCHANNELS	"CH"

#define ISTR_MAH_ALARM      	"Alarme mAh"


// er9x.cpp
// ********
#define ISTR_LIMITS		"LIMITES"
#define ISTR_EE_LOW_MEM     	"EEPROM M\300m.Bas"
#define ISTR_ALERT		"ALERTE"
#define ISTR_THR_NOT_IDLE   	"GAZ pas \302 z\300ro"
#define ISTR_RST_THROTTLE   	"R\300init. GAZ"
#define ISTR_PRESS_KEY_SKIP 	"Touche pour Ignorer"
#define ISTR_ALARMS_DISABLE 	"Alarmes D\300sactiv\300es"
#define ISTR_OLD_VER_EEPROM 	"Mauv. Version EEPROM   VERIFIER CONFIG/CALIB"
#define ISTR_RESET_SWITCHES 	"R\300initialiser Inters"
#define ISTR_LOADING        	"CHARGEMENT"
#define ISTR_MESSAGE        	"MESSAGE"
#define ISTR_PRESS_ANY_KEY  	"Touche pour Continuer"
#define ISTR_MSTACK_UFLOW  	"mStack uflow"
#define ISTR_MSTACK_OFLOW   	"mStack oflow"

#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_CHANS_GV	   "\004P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#define ISTR_CHANS_EXTRA   "\004P4  P5  P6  P7  "
#endif
#ifdef PCBX9D
 #ifdef REV9E
#define ISTR_CHANS_GV	   	 "\004P1  P2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
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

#if defined(PCBLEM1)
#define ISTR_CHANS_GV	     "\004S1  S2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004S1  S2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#endif

#define ISTR_CH	           "CH"
#define ISTR_TMR_MODE	   "\003OFFON RUsRU%ELsEL%THsTH%ALsAL%P1 P1%P2 P2%P3 P3%"

// pers.cpp
// ********
#define ISTR_ME             "ME        "
#define ISTR_MODEL          "MODELE    "
#define ISTR_BAD_EEPROM     "EEPROM corrompues"
#define ISTR_EE_FORMAT      "Formatage EEPROM"
#define ISTR_GENWR_ERROR    "Erreur genwrite"
#define ISTR_EE_OFLOW       "D\300passement EEPROM"

// templates.cpp
// ***********
#define ISTR_T_S_4CHAN      "4 Voies simple"
#define ISTR_T_TCUT         "T-Cut"
#define ISTR_T_STICK_TCUT   "Coupure Gaz"
#define ISTR_T_V_TAIL       "Empennage V"
#define ISTR_T_ELEVON       "Elevon\\Delta"
#define ISTR_T_HELI_SETUP   "Conf. H\300lico"
#define ISTR_T_GYRO         "Conf. Gyro"
#define ISTR_T_SERVO_TEST   "Test Servo"
#define ISTR_T_RANGE_TEST   "Test Port\300e"

// menus.cpp
// ***********
#define ISTR_TELEM_ITEMS	  "\004----A1= A2= RSSITSSITim1Tim2Alt GaltGspdT1= T2= RPM FUELMah1Mah2CvltBattAmpsMah CtotFasVAccXAccYAccZVspdGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7FwatRxV Hdg A3= A4= SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 RTC TmOKAspdCel1Cel2Cel3Cel4Cel5Cel6RBv1RBa1RBv2RBa2RBm1RBm2RBSVRBSTCel7Cel8Cel9Cl10Cl11Cl12Cus1Cus2Cus3Cus4Cus5Cus6Fmd RunTModTCls1Cls2SbcVSbcA"
#define ISTR_TELEM_SHORT    "\004----TIM1TIM2BATTGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7"
#define ISTR_GV             "GV"
#define ISTR_OFF_ON         "OFFON "
#define ISTR_HYPH_INV       FWx18 "\001" "\003---INV"
#define ISTR_VERSION        "VERSION"
#define ISTR_Music	        "Music"
#define ISTR_SLAVE          "\007Esclav." 
#define ISTR_MENU_DONE      "[MENU] QUAND PRET"
#define ISTR_CURVES         "COURBES"
#define ISTR_CURVE          "COURBE"
#define ISTR_GLOBAL_VAR     "VAR.GLOBALE"
#define ISTR_VALUE          "Valeur"
#define ISTR_PRESET         "PRESET"
#define ISTR_CV             "CV"
#define ISTR_LIMITS         "LIMITES"
#define ISTR_COPY_TRIM      "COPIER TRIM [MENU]"
#define ISTR_TELEMETRY      "TELEMETRIE"
#define ISTR_USR_PROTO      "UsrProto"
#define ISTR_FRHUB_WSHHI    "\006FrSky WSHhi DSMx  Jeti  ArduP ArduC FrHub HubRawFrMav Mavlk Hitec AFHDS2"
#define ISTR_MET_IMP        "\003MetImp"
#define ISTR_A_CHANNEL      "Canal A"
//#define ISTR_ALRM           "alrm"
//#define ISTR_TELEMETRY2     "TELEMETRIE2"
#define ISTR_TX_RSSIALRM    "alrmTxRSSI"
#define ISTR_NUM_BLADES     "Nb. Pales"
#define ISTR_ALT_ALARM      "Alarm.Alt"
#define ISTR_OFF122400      "\003OFF122400"
#define ISTR_VOLT_THRES     "Seuil Volt="
#define ISTR_GPS_ALTMAIN    "GpsAltMain"
#define ISTR_CUSTOM_DISP    "Affich.Perso"
#define ISTR_FAS_OFFSET     "D\300cal.FAS"
#define ISTR_VARIO_SRC      "Vario: Source"
#define ISTR_VSPD_A2        "\004----vspdA2  SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_2SWITCH        "\001Inter"
#define ISTR_2SENSITIVITY   "\001Sensibilit\300"
#define ISTR_GLOBAL_VARS    "Var.Globales"
#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENDirPrfGazAilP1 P2 P3 c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#ifdef PCBX9D
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENDirPrfGazAilP1 P2 SG SD c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif

#ifdef PCBLEM1
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#endif

#if defined(PCBX12D) || defined(PCBX10)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#define ISTR_TEMPLATES      "EXEMPLES"
#define ISTR_CHAN_ORDER     "Ordre voies RX"
#define ISTR_SP_RETA        " DPGA"
#define ISTR_CLEAR_MIXES    "EFFACER MIX. [MENU]"
#define ISTR_SAFETY_SW      "INT.SECURITE"
#define ISTR_NUM_VOICE_SW   "Nb. Inters Voix"
#define ISTR_V_OPT1         "\007 8 Secs12 Secs16 Secs"
#define ISTR_VS             "VS"
#define ISTR_VOICE_OPT      "\006ON    OFF   BOTH  15Secs30Secs60SecsVariab"
#define ISTR_CUST_SWITCH    "INTERS LOG."
#define ISTR_S              "S"
#define ISTR_15_ON          "\015On"
#define ISTR_EDIT_MIX       "EDITER MIXAGE"
#define ISTR_2SOURCE        "\001Source"
#define ISTR_2WEIGHT        "\001Ratio"
#define ISTR_FMTRIMVAL      "ValTrimFV"
#define ISTR_OFFSET         "D\300calage"
#define ISTR_2FIX_OFFSET    "\001D\300calage Fix"
#define ISTR_ENABLEEXPO     "\001Expo / DR"
#define ISTR_2TRIM          "\001Trim"
#define ISTR_15DIFF         "\010Diff"
#define ISTR_Curve          "Courbe"
#define ISTR_2SWITCH        "\001Inter"
#define ISTR_2WARNING       "\001Alerte"
#define ISTR_2MULTIPLEX     "\001Multpx"
#define ISTR_ADD_MULT_REP   "\010Ajoute    Multiplie Remplace  "
#define ISTR_2DELAY_DOWN    "\001Retard bas"
#define ISTR_2DELAY_UP      "\001Retard haut"
#define ISTR_2SLOW_DOWN     "\001Ralenti bas"
#define ISTR_2SLOW_UP       "\001Ralenti haut"
#define ISTR_MAX_MIXERS     "Trop de Mixages : 32"
#define ISTR_PRESS_EXIT_AB  "Touche [EXIT] pour Annuler"
#define ISTR_YES_NO         "\003OUI\013NON"
#define ISTR_MENU_EXIT      "\003[MENU]\013[EXIT]"
#define ISTR_DELETE_MIX     "EFFACER MIX?"
#define ISTR_MIX_POPUP      "EDITER\0AJOUTER\0COPIER\0DEPLACER\0EFFACER\0EFF.TOUT\0TEMPLATES\0PASTE"
#define ISTR_MIXER          "MIXAGE"
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
#define ISTR_DR_SW1         "DrInt1"
#define ISTR_DR_SW2         "DrInt2"
#define ISTR_DUP_MODEL      "RECOPIER MODELE"
#define ISTR_DELETE_MODEL   "EFFACER MODELE"
#define ISTR_DUPLICATING    "Recopie en Cours"
#define ISTR_SETUP          "CONF.MODELE"
#define ISTR_NAME           "NOM"
#define ISTR_VOICE_INDEX    "Voix\021MENU"
#define ISTR_TRIGGERA       "D\300clen."
#define ISTR_TRIGGERB       "D\300clen.B"
//ISTR_COUNT_DOWN_UP indexed, 10 chars each
#define ISTR_COUNT_DOWN_UP  "\012D\300compter Compter"
#define ISTR_T_TRIM         "Trim gaz"
#define ISTR_T_EXPO         "T-Expo-Dr"
// ISTR_TRIM_OPTIONS indexed 6 chars each
#define ISTR_TRIM_OPTIONS   FWx9 "\004" "\006Exp   ExFineFine  MediumCoarse"
#define ISTR_TRIM_SWITCH    "Inter Trim"
#define ISTR_TRIM_INC       "Pas Trim"
#define ISTR_BEEP_CENTRE    "Bip Cent."
#define ISTR_RETA123        "DPGA1234"
#define ISTR_PROTO          "Proto"
// ISTR_21_USEC after \021 max 4 chars
#define ISTR_21_USEC        "\021uSec"
#define ISTR_13_RXNUM       "\014RxNum"
// ISTR_23_US after \023 max 2 chars
#define ISTR_23_US          "\023uS"
// ISTR_PPMFRAME_MSEC before \015 max 9 chars, after max 4 chars
#define ISTR_PPMFRAME_MSEC  "Trame PPM\016mSec"
#define ISTR_SEND_RX_NUM    " Envoyer Nb Rx [MENU]"
#define ISTR_DSM_TYPE       " Type DSM"
#define ISTR_PPM_1ST_CHAN   "1er Canal"
#define ISTR_SHIFT_SEL      "Polarit\300"
// ISTR_POS_NEG indexed 3 chars each
#define ISTR_POS_NEG        "\003POSNEG"
#define ISTR_E_LIMITS       "Lim. Etendues"
#define ISTR_Trainer        "Ecolage"
#define ISTR_T2THTRIG       "T2ThTrig"
#define ISTR_AUTO_LIMITS    "Limites Auto"
// ISTR_1_RETA indexed 1 char each
#define ISTR_1_RETA         "\001RETA"
#define ISTR_FL_MODE        "PHASE"
#define ISTR_SWITCH         "Inter"
#define ISTR_TRIMS          "Trims"
#define ISTR_MODES          "PHASES DE VOL"
#define ISTR_SP_FM0         " PV0"
#define ISTR_SP_FM          " PV"
#define ISTR_HELI_SETUP     "CONF. HELI"
#define ISTR_SWASH_TYPE     "Type Plateau"
#define ISTR_COLLECTIVE     "Collectif"
#define ISTR_SWASH_RING     "Limite Cycl."
#define ISTR_ELE_DIRECTION  "Direction PRF"
#define ISTR_AIL_DIRECTION  "Direction AIL"
#define ISTR_COL_DIRECTION  "Direction COL"
#define ISTR_HELI_TEXT			ISTR_SWASH_TYPE "\037" ISTR_COLLECTIVE "\037" ISTR_SWASH_RING "\037" ISTR_ELE_DIRECTION "\037" ISTR_AIL_DIRECTION "\037" ISTR_COL_DIRECTION
#define ISTR_MODEL_POPUP    "EDITER\0SELECT.\0SEL/EDIT\0COPIER\0DEPLAC.\0SUPP.\0SAUV.\0REST.\0REPLACE\0NOTES"
#define ISTR_MODELSEL       "CHOIX MODELE"
// ISTR_11_FREE after \011 max 4 chars
#define ISTR_11_FREE        "\011disp"
#define ISTR_CALIBRATION    "CALIBRATION"
// ISTR_MENU_TO_START after \003 max 15 chars
#define ISTR_MENU_TO_START  "\003[MENU] POUR DEBUT"
// ISTR_SET_MIDPOINT after \005 max 11 chars
#define ISTR_SET_MIDPOINT   "\005REGLER NEUTRES"
// ISTR_MOVE_STICKS after \003 max 15 chars
#define ISTR_MOVE_STICKS    "\003BOUGER MANCHES/POT"
#define ISTR_ANA            "ANA"
#define ISTR_DIAG           "INTERS"
// ISTR_KEYNAMES indexed 5 chars each
#define ISTR_KEYNAMES       "\005Menu Exit Bas  Haut DroitGauch"
#define ISTR_TRIM_M_P       "Trim - +"
// ISTR_OFF_PLUS_EQ indexed 3 chars each
#define ISTR_OFF_PLUS_EQ    "\003off += :="
// ISTR_CH1_4 indexed 3 chars each
#define ISTR_CH1_4          "\003ch1ch2ch3ch4"
#define ISTR_MULTIPLIER     "Multiplieur"
#define ISTR_CAL            "Cal"
#define ISTR_MODE_SRC_SW    "\003mode\012% src  sw"
#define ISTR_RADIO_SETUP    "CONF. RADIO"
#define ISTR_OWNER_NAME     "Propri\300t."
#define ISTR_BEEPER         "Bip"
// ISTR_BEEP_MODES indexed 6 chars each
#define ISTR_BEEP_MODES     "\006Aucun ""NoKey ""xCourt""Court ""Norm  ""Long  ""xLong "
#define ISTR_SOUND_MODE     "Mode Son"
// ISTR_SPEAKER_OPTS indexed 10 chars each
#define ISTR_SPEAKER_OPTS   "\012Beeper    ""PiSpkr    ""BeeprVoice""PiSpkVoice""MegaSound "
#define ISTR_VOLUME         "Volume"
#define ISTR_SPEAKER_PITCH  "Pas H.Parleur"
#define ISTR_HAPTICSTRENGTH "Intens.Vibreur"
#define ISTR_CONTRAST       "Contraste"
#define ISTR_BATT_WARN      "Alarme Batterie" 
// ISTR_INACT_ALARM m for minutes after \023 - single char
#define ISTR_INACT_ALARM    "Alarme Inactivit\300\023m"
#define ISTR_THR_REVERSE    "Inv. gaz"
#define ISTR_MINUTE_BEEP    "Bip Minute"
#define ISTR_BEEP_COUNTDOWN "Bip Fin"
#define ISTR_FLASH_ON_BEEP  "Flash qd Bip"
#define ISTR_LIGHT_SWITCH   "Inter R\300tro."
#define ISTR_LIGHT_INVERT   "Inv. R\300tro."
#define ISTR_LIGHT_AFTER    "R\300tro./Touche"
#define ISTR_LIGHT_STICK    "R\300tro./Mvt"
#define ISTR_SPLASH_SCREEN  "Logo d'acceuil"
#define ISTR_SPLASH_NAME    "Nom Logo"
#define ISTR_THR_WARNING    "Alerte Gaz"
#define ISTR_DEAFULT_SW     "Conf.Inters"
#define ISTR_MEM_WARN       "Alerte M\300m."
#define ISTR_ALARM_WARN     "Alarm Warning"
#define ISTR_POTSCROLL      "D\300pl./Pot"
#define ISTR_STICKSCROLL    "D\300pl./Manches"
#define ISTR_BANDGAP        "Zones Neutres"
#define ISTR_ENABLE_PPMSIM  "Act. PPMSIM"
#define ISTR_CROSSTRIM      "Trim.Crois\300s"
#define ISTR_INT_FRSKY_ALRM "Alarme Int.Frsky"
#define ISTR_MODE           "Mode"

// SWITCHES_STR 3 chars each
#if defined(PCBSKY) || defined(PCB9XT)
#define ISWITCHES_STR "\003THRRUDELEID\200ID-ID\201AILGEATRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfTH\200TH-TH\201RU\200RU-RU\201EL\200EL-EL\201AI\200AI-AI\201GE\200GE-GE\2016P06P16P26P36P46P5PB1PB2PB3PB4"
#define IHW_SWITCHES_STR     "\004sIDxsTHRsRUDsELEsAILsGEAsTRNL1  L2  L3  L4  L5  L6  L7  L8  L9  LA  LB  LC  LD  LE  LF  LG  LH  LI  LJ  LK  LL  LM  LN  LO  6POS"
//#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6P"
#define IHW_SWITCHARROW_STR  		"\200-\201"
#endif
#ifdef PCBX9D
#ifdef REV9E
#define ISWITCHES_STR "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5"\
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

#ifdef PCBLEM1
#define ISWITCHES_STR "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5PB1PB2PB3"
#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6PL1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"
#define IHW_SWITCHARROW_STR "\200-\201"
#define ISTR_CHANS_EXTRA   "004P4  P5  P6  P7  "
#endif

#if defined(PCBX12D) || defined(PCBX10)
#define ISWITCHES_STR				 "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5PB1PB2PB3PB4"
#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6PL1L2L3L4L5L6L7L8L9LALBLCLDLELFLGLHLILJLKLLLMLNLO"
#define IHW_SWITCHARROW_STR  "\200-\201"
#endif
#define ISWITCH_WARN_STR	   	"Alertes Inters"
// CURV_STR indexed 3 chars each
// c17-c24 added for timer mode A display
#define ICURV_STR					 "\003---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24c25c26c27c28c29c30c31c32"
// CSWITCH_STR indexed 7 chars each
#define ICSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""Latch  F-Flop TimeOffNtmeOff1-Shot 1-ShotRv\140=val v&val  v1\140=v2 v=val  "

#define ISWASH_TYPE_STR     FWx17 "\004" "\004----""120 ""120X""140 ""90  "

#define ISTR_STICK_NAMES    		"\005Dir \0Prf \0Gaz \0Ail "

#define ISTR_STAT           		"STAT"
#define ISTR_STAT2          		"STAT2"
// ISTR_TRIM_OPTS indexed 3 chars each
#define ISTR_TRIM_OPTS      		"\003ExpExFFneMedCrs"
#define ISTR_TTM            		"TTm"
#define ISTR_FUEL           		"Carb"
#define ISTR_12_RPM         		"\012RPM"
#define ISTR_LON_EQ         		"Lon="
#define ISTR_ALT_MAX        		"Alt=\011m   Max="
#define ISTR_SPD_KTS_MAX    		"Vit=\011kts Max="
#define ISTR_LAT_EQ         		"Lat=" "\037" ISTR_LON_EQ "\037" ISTR_ALT_MAX "\037" ISTR_SPD_KTS_MAX
#define ISTR_11_MPH         		"\011mph"

#define ISTR_SINK_TONES	   		"Tonalit/300s"


// ersky9x strings
#define ISTR_ST_CARD_STAT   		"STAT CARTE SD"
#define ISTR_4_READY        		"\004Pret"
#define ISTR_NOT            		"NON"
#define ISTR_BOOT_REASON    		"RAISON DEMARRAGE"
#define ISTR_6_WATCHDOG     		"\006WATCHDOG"
#define ISTR_5_UNEXPECTED   		"\005NON ATTENDU"
#define ISTR_6_SHUTDOWN     		"\006ARRET"
#define ISTR_6_POWER_ON     		"\006MISE EN ROUTE"
// ISTR_MONTHS indexed 3 chars each
#define ISTR_MONTHS         		"\003XxxJanFevMarAvrMaiJunJulAouSepOctNovDec"
#define ISTR_MENU_REFRESH   		"[MENU] ACTUALISER"
#define ISTR_DATE_TIME      		"DATE-HEURE"
#define ISTR_SEC            		"Sec."
#define ISTR_MIN_SET        		"Min.\015INIT."
#define ISTR_HOUR_MENU_LONG 		"Heure\012MENU LONG"
#define ISTR_DATE           		"Date"
#define ISTR_MONTH          		"Mois"
#define ISTR_YEAR_TEMP     		"Ann\300e\013Temp."
#define ISTR_YEAR          		"Ann\300e"
#define ISTR_BATTERY        		"BATTERIE"
#define ISTR_Battery        		"Batterie"
#define ISTR_CURRENT_MAX    		"Courant\016Max"
#define ISTR_CPU_TEMP_MAX   		"Temp.CPU\014C Max\024C"
#define ISTR_MEMORY_STAT    		"STAT MEMOIRE"
#define ISTR_GENERAL        		"G\300n\300ral"
#define ISTR_Model          		"Mod\301le"
#define ISTR_RADIO_SETUP2  		"CONF.RADIO2"
#define ISTR_BRIGHTNESS     		"Luminosit\300"
#define ISTR_CAPACITY_ALARM 		"Alerte Capacit\300"
#define ISTR_BT_BAUDRATE   		"Vitesse BT"
#define ISTR_ROTARY_DIVISOR 		"Div. Rotatif"
#define ISTR_LV             "GV"
#define ISTR_LH             "GH"
#define ISTR_RV             "DV"
#define ISTR_RH             "DH"
#define ISTR_STICK_GAIN     "Stick Gain"
#define ISTR_STICK_DEADBAND "Deadband Manche"
#define ISTR_STICK_LV_GAIN  		"Gain Manche GV"
#define ISTR_STICK_LH_GAIN  		"Gain Manche GH"
#define ISTR_STICK_RV_GAIN  		"Gain Manche DV"
#define ISTR_STICK_RH_GAIN  		"Gain Manche DH"
#define ISTR_NO_SINK_TONES  		"Silence"
#define ISTR_BIND			"Bind"
#define ISTR_RANGE			"Test Port\300e"

#define ISTR_ALRMS_OFF			"Alarmes D\300sactiv\300es"
#define ISTR_OLD_EEPROM			"MAUVAISE EEPROM    VERIFIER CONFIG/CALIB"
#define ISTR_TRIGA_OPTS			"OFFON THsTH%"
#define ISTR_CHK_MIX_SRC		"VERIFIER SOURCES MIX."

#define ISTR_BT_TELEMETRY		"T\300l\300m\300trie BT"
#define ISTR_FRSKY_COM_PORT 		"Port Com.Telem"
#define ISTR_INVERT_COM1		"Invers\300r COM 1"
#define ISTR_LOG_SWITCH			"Inter Enreg."
#define ISTR_LOG_RATE			"Fr\300q.Enreg."
#define ISTR_6_BINDING			"\006BINDING"
#define ISTR_RANGE_RSSI			"TEST PORTEE RSSI:"
#define ISTR_FAILSAFE			"FAILSAFE"
#define ISTR_VOLUME_CTRL		"Ctrl Volume"
//#ifdef ASSAN
//#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiAssan"
//#else
#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiXfireAcces"
//#endif
#define ISTR_TYPE			"Type"
#define ISTR_COUNTRY			"Pays"
#define ISTR_SP_FAILSAFE		"Failsafe"
#define ISTR_PPM2_START			"Canal D\300b.PPM2"
#define ISTR_FOLLOW			"Suivant"
#define ISTR_PPM2_CHANNELS		"Canaux PPM2"
#define ISTR_FILTER_ADC			"Filtre DAC"
#define ISTR_SCROLLING			"D\300filement"
#define ISTR_ALERT_YEL			"Alerte [Jau]"
#define ISTR_ALERT_ORG			"Alerte [Org]"
#define ISTR_ALERT_RED			"Alerte [Rou]"
#define ISTR_LANGUAGE			"Langue"

#define ISTR_RSSI_WARN		 	"Alert.RSSI"
#define ISTR_RSSI_CRITICAL  		"RSSI Critique"
#define ISTR_RX_VOLTAGE		 	"Tension Rx"
#define ISTR_DSM_WARNING	  	"Alerte DSM"
#define ISTR_FADESLOSSHOLDS 		"\006fades lossesholds "
#define ISTR_DSM_CRITICAL	  	"DSM Critique"
#define ISTR_BT_TRAINER		  	"Ecolage BT"
#define ISTR_MULTI_TEXT     		"Protocole\037Type\037Autobind\037Power"
#define ISTR_MULTI_PROTO    		"Protocole"
#define ISTR_MULTI_TYPE	    		"Type"
#define ISTR_MULTI_AUTO	    		"Autobind"
#define ISTR_MULTI_POWER    "Power\013Rate\023mS"
#define ISTR_MULTI_OPTION   		"\013Option"

#define ISTR_Display		     	"Affichage" 
#define ISTR_AudioHaptic		"Audio/Vib." 
#define ISTR_Alarms		     	"Alarmes" 
#define ISTR_General		     	"G\300n\300ral" 
#define ISTR_Controls			"Controles"
#define ISTR_Hardware			"Mat\300riel"
#define ISTR_Calibration		"Calibration" 
//#define ISTR_Trainer		     	"Ecolage" 
#define ISTR_Version		     	"Version" 
#define ISTR_ModuleRssi			"FrSky xSSI"
#define ISTR_DateTime			"Date-Heure" 
#define ISTR_DiagSwtch		   	"DiagInter"  
#define ISTR_DiagAna		     	"DiagAna" 

#define ISTR_Mixer		      	"Mixage" 
#define ISTR_Cswitches		  	"Int.Logiq." 
#define ISTR_Telemetry		 	"T\300l\300m\300trie" 
#define ISTR_limits			"Limites" 
#define ISTR_Bluetooth		  	"BlueTooth" 
#define ISTR_heli_setup			"H\300li" 
#define ISTR_Curves			"Courbes" 
#define ISTR_Expo			"Expo/D.Rate" 
#define ISTR_Globals		    	"Globales" 
#define ISTR_Timer		      	"Chronos" 
#define ISTR_Modes			"Phases" 
#define ISTR_Voice		      	"Voix-Audio" 
#define ISTR_Protocol			"Protocole" 
#define ISTR_Safety			"Int.S\300cur."
#define ISTR_Eeprom			"EEPROM" 

#define ISTR_MAIN_POPUP			"Choix Mod\301le\0Config.Mod\301le\0Dernier Menu\0Conf.Radio\0Statistics\0Notes\0Zero Alt.\0Zero A1 Offs\0Zero A2 Offs\0RAZ GPS\0Aide\0Main Display\0Run Script\0Reset Telemetry"
#define ISTR_ROTATE_SCREEN		"Rotation Ecran"
#define ISTR_REVERSE_SCREEN		"Inverser Ecran"
#define ISTR_MENU_ONLY_EDIT		"MENU Modifie"
#define ISTR_Voice_Alarm		"Alarme Voix"
#define ISTR_Voice_Alarms		"Alarmes Voix"
#define ISTR_CUSTOM_CHECK		"V/300rif.Perso."
#define ISTR_CUSTOM_STK_NAMES		"Manches Perso."
#define ISTR_HAPTIC			"Vibreur"
#define ISTR_RESET_SWITCH		"Inter RAZ"
#define ISTR_THROTTLE_OPEN		"Gaz Ouvert"
#define ISTR_THR_DEFAULT		"D/300faut Gaz"
#define ISTR_TOTAL_TIME			"Temps Total"
#define ISTR_POPUP_GLOBALS		"GVARS\0GVadjusters\0Calibreurs\0T/300l/300m/300trie\0Perso.\0Mixage\0Templates\0Logging\0Blocking\0Vario\0Sensors"

#define ISTR_SHUT_DOWN					"Shutting Down"

//"Alerte RSSI"
//"RSSI Critique"
//"Tension Rx"
//"Alerte DSM"
//"\006fondus lossesholds "
//"DSM Critique"
//"Ecolage BT"
//"Source Actuelle"
//"\004----A1  A2  FASVSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
//"SC  ="
//"Source"
//"Multiplieur"
//"Diviseur"
//"Unit\300"
//"Signe"
//"D\300cimales"
//"D\300calage \302"
//"\005FirstLast "
//"Inter Voix"
//"Fonction"
//"\007----   v>val  v<val  |v|>val|v|<valON     OFF    BOTH   "
//"Inter"
//"Fr\300q"
//"\017Once"
//"D\300calage"
//"Type Fichier"
//"\006   NomNum\300ro Vibr."
//"Fich. Voix"
//"\006Haptc1Haptc2Haptc3"
// SKY "\003IDxTHRRUDELEAILGEATRN"
// X9D "\002SASBSCSDSESFSGSH"
//"\002Phases"
// SKY "\004sIDxsTHRsRUDsELEsAILsGEAsTRN"
// X9D "\002SASBSCSDSESFSGSH"
//"Inter RAZ"
// SKY "\003---P1 P2 P3 GV4GV5GV6GV7"
// X9D "\003---P1 P2 SL SR GV5GV6GV7"
//"Interne"
//"\003AmeJapEur"
//"Externe"
//"Fondu ON"
//"Fondu OFF"
//"Nom"
//"Co Proc"
//"On Time"
//"ttimer1        us"
//"\013rssi"
//"Vbat"
//"\013RxV"
//"AMP\013Temp"
//"RPM\021DSM2"
//"CONFIG."
//"Affichage"
//"Audio/Vib."
//"Alarmes"
//"G\300n\300rale"
//"Controles"
//"Mat\300riel"
//"Calibration"
//"Ecolage"
//"Version"
//"Date-Heure"
//"DiagInter"
//"DiagAna"
//"AFFICHAGE"
//"BLEU"
//"BLANC"
//"Aff. Optrex"
//"AUDIO/VIBREUR"
//"ALARMES"
//"[Suiv]"
//"GENERALE"
//"\012   ENGLISH  FRANCAIS   DEUTSCH NORWEGIAN   SWEDISH"
//"\005NONE POT  STICKBOTH "
//"CONTROLES"
//"MATERIEL"
//"Inter ELE"
//"\0042POS3POS6POS"
//"Inter THR"
//"\0042POS3POS"
//"Inter RUD"
//"Inter GEAR"
//"PARAM.MODELE"
//"Mixeur"
//"C.Switches"
//"T\300l\300m\300trie"
//"Limites"
//"Affichage"
//"MAFFICHAGE"
//"\001Fonct.Min.Vibr








