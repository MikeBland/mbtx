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

/**
MY CHANGELOG

v.xx.01-german
- replaced special characters
- added new Strings after ISTR_BIND (line )
- added new Strings in if-case ISTR_TELEM_ITEMS and ISTR_FRHUB_WSHHI
- some minor changes in translation
**/

// Special characters:
// Ä für AE benutze \300
// ä für ae benutze \301
// Ö für OE benutze \302
// ö für oe benutze \303
// Ü für UE benutze \304
// ü für ue benutze \305
// ß für ss benutze \306

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


#define ISTR_ON             "AN "
#define ISTR_OFF            "AUS"
#define ISTR_X_OFF_ON				FWx17 "\001" "\003" ISTR_OFF ISTR_ON

#define ISTR_ALTEQ	         "H\303h=" 
#define ISTR_TXEQ			       "\003Sn=Swr" // TX Transmitter - Sender
#define ISTR_RXEQ		       "Em=" // RX Reciever - Empfänger
#define ISTR_RX  		       "Em"
#define ISTR_TRE012AG	     "TRE012AG"

// ISTR_YELORGRED je genau 3 Zeichen lang
#define ISTR_YELORGRED	     "\003---GelOrgRot"
#define ISTR_A_EQ		       "A ="
#define ISTR_SOUNDS	       "\006Warn1 ""Warn2 ""Cheap ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Crickt""Siren ""AlmClk""Ratata""Tick  ""Haptc1""Haptc2""Haptc3" "Haptc4"
#define ISTR_SWITCH_WARN	   "Schalter Warnung" 
//#define ISTR_SWITCH				   "Schalter" 
#define ISTR_WARNING			   "Warnung" 
// ISTR_TIMER genau 5 Zeichen lang 
#define ISTR_TIMER          "Timer"			

// ISTR_PPMCHANNELS je genau 4 Zeichen lang
#define ISTR_PPMCHANNELS	   "CH"

#define ISTR_MAH_ALARM      "mAh Alarm"


// er9x.cpp
// ********
#define ISTR_LIMITS		     "GRENZEN"
#define ISTR_EE_LOW_MEM     "EEPROM wenig Speicher"
#define ISTR_ALERT		       "ALARM"
#define ISTR_THR_NOT_IDLE   "Gas nicht im Ruhezstd"
#define ISTR_RST_THROTTLE   "setze auf Standgas"
#define ISTR_PRESS_KEY_SKIP "bel. Taste dr\305cken"
#define ISTR_ALARMS_DISABLE "Alarm ist deaktiviert"
#define ISTR_OLD_VER_EEPROM " EEPROM ist veraltet   TESTE EINSTELL/KALIB"
#define ISTR_RESET_SWITCHES "Schalter ausschalten"
#define ISTR_LOADING        "L\300DT"
#define ISTR_MESSAGE        "NACHRICHT"
#define ISTR_PRESS_ANY_KEY  "Taste dr\305cken"
#define ISTR_MSTACK_UFLOW   "mStack uflow"
#define ISTR_MSTACK_OFLOW   "mStack oflow"

#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_CHANS_GV	     "\004P1  P2  P3  HALBVOLLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004P1  P2  P3  HALBVOLLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#define ISTR_CHANS_EXTRA   "\004P4  P5  P6  P7  "
#endif
#ifdef PCBX9D
 #ifdef REV9E
#define ISTR_CHANS_GV	     "\004P1  P2  SL  HALBVOLLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004P1  P2  SL  HALBVOLLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#define ISTR_CHANS_EXTRA   "\004SR  S1  S2  P3  P4  "
 #else
  #ifdef PCBT12
#define ISTR_CHANS_GV	     "\004AUX4AUX5SL  HALBVOLLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
  #else
#define ISTR_CHANS_GV	     "\004S1  S2  SL  HALBVOLLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
  #endif
#define ISTR_CHANS_RAW	   "\004S1  S2  SL  HALBVOLLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
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
#define ISTR_TMR_MODE	     "\003AUSON Se0Se%Ho0Ho%Ga0Ga%Qu0Qu%P1 P1%P2 P2%P3 P3%" // OFF=AUS=Timer aus ; ABS=AN=Timer an ; RUs=Se0=Seite bei 0 ; Ru%=Se%=Seite bei x% usw.

#if defined(PCBLEM1)
#define ISTR_CHANS_GV	     "\004S1  S2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_CHANS_RAW	   "\004S1  S2  SL  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#endif

// pers.cpp
// ********
#define ISTR_ME             "ICH       "
#define ISTR_MODEL          "MODELL    "
#define ISTR_BAD_EEPROM     "falsche EEprom Daten"
#define ISTR_EE_FORMAT      "formatiere EEPROM"
#define ISTR_GENWR_ERROR    "Schreibfehler"
#define ISTR_EE_OFLOW       "EEPROM oflow"

// templates.cpp
// ***********
#define ISTR_T_S_4CHAN      "Einfache 4-CH"
#define ISTR_T_TCUT         "Gas aus"
#define ISTR_T_STICK_TCUT   "dauer Gas aus"
#define ISTR_T_V_TAIL       "V-Leitw"
#define ISTR_T_ELEVON       "Delta\\Nurfl\305gler"
#define ISTR_T_HELI_SETUP   "Heli Einst"
#define ISTR_T_GYRO         "Gyro Einst"
#define ISTR_T_SERVO_TEST   "Servo Test"
#define ISTR_T_RANGE_TEST   "Range Test"

// menus.cpp
// ***********
#define ISTR_TELEM_ITEMS	  "\004----A1= A2= RSSITSSITim1Tim2H\302heGH\302hGGesT1= T2= UPM TANKMah1Mah2CvltAkkuAmpsMah CtotFasVAccXAccYAccZVspdGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7FwatRxV Hdg A3= A4= SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 RTC TmOKAspdCel1Cel2Cel3Cel4Cel5Cel6RBv1RBa1RBv2RBa2RBm1RBm2RBSVRBSTCel7Cel8Cel9Cl10Cl11Cl12Cus1Cus2Cus3Cus4Cus5Cus6Fmd RunTModTCls1Cls2SbcVSbcA"
#define ISTR_TELEM_SHORT    "\004----TIM1TIM2AKKUGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7"
#define ISTR_GV             "GV"
#define ISTR_OFF_ON         "AUSAN "
#define ISTR_HYPH_INV       FWx18"\001""\003---UMK" // Umkehren
#define ISTR_VERSION        "VERSION"
#define ISTR_Music	        "Music"
#define ISTR_SLAVE          "\007Slave" 
#define ISTR_MENU_DONE      "[MENU] WENN FERTIG"
#define ISTR_CURVES         "KURVEN"
#define ISTR_CURVE          "KURVE"
#define ISTR_GLOBAL_VAR     "GLOBALE VAR"
#define ISTR_VALUE          "Wert"
#define ISTR_PRESET         "VOREINST"
#define ISTR_CV             "KV"
#define ISTR_LIMITS         "GRENZEN"
#define ISTR_COPY_TRIM      "KOPIE TRIM [MENU]"
#define ISTR_TELEMETRY      "TELEMETRIE"
#define ISTR_USR_PROTO      "BenProto"
#define ISTR_FRHUB_WSHHI    "\006FrSky WSHhi DSMx  Jeti  ArduP ArduC FrHub HubRawFrMav Mavlk Hitec AFHDS2"
#define ISTR_MET_IMP        "\003MetImp" // Metrisches System / Imperiales System
#define ISTR_A_CHANNEL      "A  Kanal"
//#define ISTR_ALRM           "alrm"
//#define ISTR_TELEMETRY2     "TELEMETRIE2"
#define ISTR_TX_RSSIALRM    "SnRSSIalrm" // Sender
#define ISTR_NUM_BLADES     "Num Bl\301tter"
#define ISTR_ALT_ALARM      "H\303heAlarm"
#define ISTR_OFF122400      "\003AUS122400"
#define ISTR_VOLT_THRES     "MaxSpannung"
#define ISTR_GPS_ALTMAIN    "GPS H\303heHalten"
#define ISTR_CUSTOM_DISP    "Ind. Bildschirm"
#define ISTR_FAS_OFFSET     "FAS Offset" // FrSky Amperage Sensor (FAS-100) Offset
#define ISTR_VARIO_SRC      "Vario: Quelle" // Variometerquelle
#define ISTR_VSPD_A2        "\004----VGesA2  SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 " // VGeschwindigkeit
#define ISTR_2SWITCH        "\001Schalter"
#define ISTR_2SENSITIVITY   "\001Empfindlichkt"
#define ISTR_GLOBAL_VARS    "GLOBALE VARS"
#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_GV_SOURCE      "\003---StmHtmGtmQtmRENSeiH\302hGasQueP1 P2 P3 c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24" // xtm=Trim for channel "x" REN=Rotary Encoder  ... = Variablennamen
#define ISTR_EXTRA_SOURCE   ""
#endif
#ifdef PCBX9D
#define ISTR_GV_SOURCE      "\003---StmHtmGtmQtmRENSeiH\302hGasQueP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24" // xtm=Trim for channel "x" REN=Rotary Encoder  ... = Variablennamen
#define ISTR_EXTRA_SOURCE   ""
#endif

#ifdef PCBLEM1
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#endif

#if defined(PCBX12D) || defined(PCBX10)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#define ISTR_TEMPLATES      "VORLAGEN"
#define ISTR_CHAN_ORDER     "Kanal Reihenfolge"
#define ISTR_SP_RETA        " SHGQ" // Seitenleitwerk=Rud Höhenleitwerk=Ele Gas=Thr Querruder=Ail
#define ISTR_CLEAR_MIXES    "L\302SCHE MISCHER [MENU]"
#define ISTR_SAFETY_SW      "SICHERHEITS SCH"
#define ISTR_NUM_VOICE_SW   "Nummer Ton Sch"
#define ISTR_V_OPT1         "\007 8 Sek 12 Sek 16 Sek "
#define ISTR_VS             "VS" // ?
#define ISTR_VOICE_OPT      "\006AN    AUS   BEIDE 15Sek 30Sek 60Sek Eigene"
#define ISTR_CUST_SWITCH    "IND. SCHALTER" // Individueller Schalter
#define ISTR_S              "S"
#define ISTR_15_ON          "\015An"
#define ISTR_EDIT_MIX       "Bearb MISCHER" // Bearbeite Mischer
#define ISTR_2SOURCE        "\001Quelle"
#define ISTR_2WEIGHT        "\001Gewicht"
#define ISTR_FMTRIMVAL      "FmTrimVal"
#define ISTR_OFFSET         "Offset"
#define ISTR_2FIX_OFFSET    "\001Fix Offset"
#define ISTR_ENABLEEXPO     "\001EnableExpoDr"
#define ISTR_2TRIM          "\001Trimmen"
#define ISTR_15DIFF         "\010Diff"
#define ISTR_Curve          "Kurve"
#define ISTR_2WARNING       "\001Warnung"
#define ISTR_2MULTIPLEX     "\001Multpx"
// ISTR_ADD_MULT_REP je genau 8 Zeichen
#define ISTR_ADD_MULT_REP   "\010Hinzufgn  MultipliziErsetzen  "
#define ISTR_2DELAY_DOWN    "\001Verz. runter"
#define ISTR_2DELAY_UP      "\001Verz. hoch"
#define ISTR_2SLOW_DOWN     "\001Langsam runtr"
#define ISTR_2SLOW_UP       "\001Langsam hoch"
#define ISTR_MAX_MIXERS     "Max Mix erreicht: 32"
#define ISTR_PRESS_EXIT_AB  "[EXIT] zum Abbrechen"
#define ISTR_YES_NO         "\003JA \013NEIN"
#define ISTR_MENU_EXIT      "\003[MENU]\013[EXIT]"
#define ISTR_DELETE_MIX     "L\302SCHE MISCHER?"
#define ISTR_MIX_POPUP      "BEARBEI\0EINF\305GE\0KOPIER\0BEWEGE\0L\302SCHE\0CLEAR ALL\0TEMPLATES\0PASTE"
#define ISTR_MIXER          "MISCHER"
// CHR_S S for Slow / Langsam
#define ICHR_S              "L"
// CHR_D D for Delay / VerzÃ¶gert
#define ICHR_D              "V"
// CHR_d d for differential
#define ICHR_d              "d"
#define ISTR_EXPO_DR        "EXPO/DR"
#define ISTR_4DR_MID        "\004DR Mittel"
#define ISTR_4DR_LOW        "\004DR Tief"
#define ISTR_4DR_HI         "\004DR Hoch"
#define ISTR_2EXPO          "\002Expo"
#define ISTR_DR_SW1         "DrSw1"
#define ISTR_DR_SW2         "DrSw2"
#define ISTR_DUP_MODEL      "KOPIERE MODELL"
#define ISTR_DELETE_MODEL   "L\302SCHE MODELL"
#define ISTR_DUPLICATING    "Kopiere Modell"
#define ISTR_SETUP          "Modell Setup"
#define ISTR_NAME           "Name"
#define ISTR_VOICE_INDEX    "Ton Freq\021MENU"
#define ISTR_TRIGGERA       "Trigger"
#define ISTR_TRIGGERB       "TriggerB"
//ISTR_COUNT_DOWN_UP indexed, 10 chars each
#define ISTR_COUNT_DOWN_UP  "\012Z\301hl runteZ\301hl hoch"
#define ISTR_T_TRIM         "Thr-Trim"
#define ISTR_T_EXPO         "T-Expo-Dr"
// ISTR_TRIM_OPTIONS indexed 6 chars each
#define ISTR_TRIM_OPTIONS   FWx9"\004""\006Expon ExFeinFein  MittelGrob  "
#define ISTR_TRIM_SWITCH    "Insta-Trim Sch"
#define ISTR_TRIM_INC       "Trim Ink"
#define ISTR_BEEP_CENTRE    "Piep Frq" //TonhÃ¶he Frequenz
#define ISTR_RETA123        "SHGQ1234"
#define ISTR_PROTO          "Proto" // Protokoll
// ISTR_21_USEC after \021 max 4 chars
#define ISTR_21_USEC        "\021uSek" 
#define ISTR_13_RXNUM       "\014EmNum" //EmpfÃ¤nger
// ISTR_23_US after \023 max 2 chars
#define ISTR_23_US          "\023uS"
// ISTR_PPMFRAME_MSEC before \015 max 9 chars, after max 4 chars
#define ISTR_PPMFRAME_MSEC  "PPM FrLen\016mSek" // Puls Pausen Modulation
#define ISTR_SEND_RX_NUM    " Send Em Nummer [MENU]"
#define ISTR_DSM_TYPE       " DSM Typ" 
#define ISTR_PPM_1ST_CHAN   "1. Kanal"
#define ISTR_SHIFT_SEL      "Signalart" // Signalart
// ISTR_POS_NEG indexed 3 chars each
#define ISTR_POS_NEG        "\003POSNEG"
#define ISTR_E_LIMITS       "E. Grenze" //Erweiterte Grenze
#define ISTR_Trainer        "Trainer"
#define ISTR_T2THTRIG       "GasStaT2" // 2. Timer startet wenn Gas um 5% bewegt
#define ISTR_AUTO_LIMITS    "Auto Grenze"
// ISTR_1_RETA indexed 1 char each
#define ISTR_1_RETA         "\001SHGQ"
#define ISTR_FL_MODE        "FL MODUS"
#define ISTR_SWITCH         "Schalter"
#define ISTR_TRIMS          "Trimmer"
#define ISTR_MODES          "MODI"
#define ISTR_SP_FM0         " FM0"
#define ISTR_SP_FM          " FM"
#define ISTR_HELI_SETUP     "HELI EINST"
#define ISTR_SWASH_TYPE     "Taumel Typ" 
#define ISTR_COLLECTIVE     "Kollektive"
#define ISTR_SWASH_RING     "Taumel Ring"
#define ISTR_ELE_DIRECTION  "H\302H Richtung"
#define ISTR_AIL_DIRECTION  "QUE Richtung"
#define ISTR_COL_DIRECTION  "KOL Richtung" //Kollektive
#define ISTR_HELI_TEXT			ISTR_SWASH_TYPE "\037" ISTR_COLLECTIVE "\037" ISTR_SWASH_RING "\037" ISTR_ELE_DIRECTION "\037" ISTR_AIL_DIRECTION "\037" ISTR_COL_DIRECTION
#define ISTR_MODEL_POPUP    "EDIT\0BEARBEI\0SEL/EDIT\0KOPIER\0BEWEGE\0L\302SCHE\0BACKUP\0RESTORE\0REPLACE\0NOTES"
#define ISTR_MODELSEL       "MODELWAHL"
// ISTR_11_FREE after \011 max 4 chars
#define ISTR_11_FREE        "\011frei"
#define ISTR_CALIBRATION    "KALIBRIERUNG"
// ISTR_MENU_TO_START after \003 max 15 chars
#define ISTR_MENU_TO_START  "\003[MENU] ZUM START"
// ISTR_SET_MIDPOINT after \005 max 11 chars
#define ISTR_SET_MIDPOINT   "\005SET MITPUNKT"
// ISTR_MOVE_STICKS after \003 max 15 chars
#define ISTR_MOVE_STICKS    "\003BEWG STICKS/POTS"
#define ISTR_ANA            "ANA" // Analog Input und Batterie Spannung Kalibrierung
#define ISTR_DIAG           "DIAG" // Diagnostics
// ISTR_KEYNAMES indexed 5 chars each
#define ISTR_KEYNAMES       "\005 Men\305 ExitRuntr HochRechtLinks"
#define ISTR_TRIM_M_P       "Trim- +"
// ISTR_OFF_PLUS_EQ indexed 3 chars each
#define ISTR_OFF_PLUS_EQ    "\003aus += :="
// ISTR_CH1_4 indexed 3 chars each
#define ISTR_CH1_4          "\003ch1ch2ch3ch4"
#define ISTR_MULTIPLIER     "Multiplika"
#define ISTR_CAL            "Kal"
#define ISTR_MODE_SRC_SW    "\003mode\012% que sch" // Quelle Schalter
#define ISTR_RADIO_SETUP    "FUNK EINST"
#define ISTR_OWNER_NAME     "Nutzername"
#define ISTR_BEEPER         "Pieper"
// ISTR_BEEP_MODES indexed 6 chars each
#define ISTR_BEEP_MODES     "\006Lautls""TstAus""xKurz ""Kurz  ""Normal""Lang  ""xLang " // x = seh
#define ISTR_SOUND_MODE     "Sound Modus"
// ISTR_SPEAKER_OPTS indexed 10 chars each
#define ISTR_SPEAKER_OPTS   "\012Pieper    ""PiLautspre""PieprTon  ""PieLautTon""MegaSound "
#define ISTR_VOLUME         "Lautst"
#define ISTR_SPEAKER_PITCH  "Tonh\303he"
#define ISTR_HAPTICSTRENGTH "Haptische St\301rke"
#define ISTR_CONTRAST       "Kontrast"
#define ISTR_BATT_WARN      "Batterie Warnung" 
// ISTR_INACT_ALARM m for minutes after \023 - single char
#define ISTR_INACT_ALARM    "Inaktivit\301ts alarm\023m"
#define ISTR_THR_REVERSE    "Gas umkehren"
#define ISTR_MINUTE_BEEP    "Minutenton"
#define ISTR_BEEP_COUNTDOWN "Piep Countdown"
#define ISTR_FLASH_ON_BEEP  "Blitz auf Piep"
#define ISTR_LIGHT_SWITCH   "Lichtschalter"
#define ISTR_LIGHT_INVERT   "Licht umkehren"
#define ISTR_LIGHT_AFTER    "Licht an key"
#define ISTR_LIGHT_STICK    "Licht an Stk Mv"
#define ISTR_SPLASH_SCREEN  "Startbildschirm"
#define ISTR_SPLASH_NAME    "Start Name"
#define ISTR_THR_WARNING    "Gas Warnung"
#define ISTR_DEAFULT_SW     "Standard Sch"
#define ISTR_MEM_WARN       "Speicher Warnung"
#define ISTR_ALARM_WARN     "Alarm Warnung"
#define ISTR_POTSCROLL      "PotScroll"
#define ISTR_STICKSCROLL    "StickScroll"
#define ISTR_BANDGAP        "BandL\305cke"
#define ISTR_ENABLE_PPMSIM  "aktiviere PPMSIM"
#define ISTR_CROSSTRIM      "KreuzTrim"
#define ISTR_INT_FRSKY_ALRM "Int. Frsky alarm"
#define ISTR_MODE           "Modus"

// SWITCHES_STR 3 chars each
#if defined(PCBSKY) || defined(PCB9XT)
#define ISWITCHES_STR       "\003GASSEIH\302HID\200ID-ID\201QUEFWKTRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfTH\200TH-TH\201RU\200RU-RU\201EL\200EL-EL\201AI\200AI-AI\201GE\200GE-GE\2016P06P16P26P36P46P5PB1PB2PB3PB4"
#define IHW_SWITCHES_STR     "\004sIDxsTHRsRUDsELEsAILsGEAsTRNL1  L2  L3  L4  L5  L6  L7  L8  L9  LA  LB  LC  LD  LE  LF  LG  LH  LI  LJ  LK  LL  LM  LN  LO  6POS"
//#define IHW_SWITCHES_STR     "\002SASBSCSDSESFSGSH6P"
#define IHW_SWITCHARROW_STR  "\200-\201"
#endif
#ifdef PCBX9D
#ifdef REV9E
#define ISWITCHES_STR 			 "\003SF       SC\200SC-SC\201      SH L1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI LJ LK LL LM LN LO onfSB\200SB-SB\201SE\200SE-SE\201SA\200SA-SA\201SD\200SD-SD\201SG\200SG-SG\2016P06P16P26P36P46P5"\
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
#define ISWITCH_WARN_STR	   "Schalter Warnung"
// CURV_STR indexed 3 chars each
// c17-c24 added for timer mode A display
#define ICURV_STR					 "\003---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24c25c26c27c28c29c30c31c32"
// CSWITCH_STR indexed 7 chars each
#define ICSWITCH_STR        "\007----   v>val  v<val  |v|>val|v|<valAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""Latch  F-Flop TimeOffNtmeOff1-Shot 1-ShotRv\140=val v&val  v1\140=v2 v=val  "

#define ISWASH_TYPE_STR     FWx17"\004""\004----""120 ""120X""140 ""90  "

#define ISTR_STICK_NAMES    "\005Sei \0H\302h \0Gas \0Que "

#define ISTR_STAT           "STAT"
#define ISTR_STAT2          "STAT2"
// ISTR_TRIM_OPTS indexed 3 chars each
#define ISTR_TRIM_OPTS      "\003ExFxFeFeiMitStk" // ExF= extra fein; Fei = fein; Mit = medium - mittel; Crs = Coarse sehr stark
#define ISTR_TTM            "GTm" // Gas Trim
#define ISTR_FUEL           "TANK"
#define ISTR_12_RPM         "\012UPM"
#define ISTR_LON_EQ         "L\301n="
#define ISTR_ALT_MAX        "H\302h=\011m   Max="
#define ISTR_SPD_KTS_MAX    "Ges=\011kts Max="
#define ISTR_LAT_EQ         "Bre=" "\037" ISTR_LON_EQ "\037" ISTR_ALT_MAX "\037" ISTR_SPD_KTS_MAX
#define ISTR_11_MPH         "\011mph"

#define ISTR_SINK_TONES	   "Sink Tones"
#define ISTR_FRSKY_MOD      "Frksy Mod Done"

// ersky9x strings
#define ISTR_ST_CARD_STAT   "SD CARD STAT"
#define ISTR_4_READY        "\004Bereit"
#define ISTR_NOT            "NICHT"
#define ISTR_BOOT_REASON    "BOOT GRUND"
#define ISTR_6_WATCHDOG     "\006W\300CHTER"
#define ISTR_5_UNEXPECTED   "\005UNERWARTET"
#define ISTR_6_SHUTDOWN     "\006AUSSCHALTEN"
#define ISTR_6_POWER_ON     "\006EINSCHALTEN"
// ISTR_MONTHS indexed 3 chars each
#define ISTR_MONTHS         "\003XxxJanFebMrzAprMaiJunJulAugSepOktNovDez"
#define ISTR_MENU_REFRESH   "[MENU] NEU LADEN"
#define ISTR_DATE_TIME      "DATUM-ZEIT"
#define ISTR_SEC            "Sek."
#define ISTR_MIN_SET        "Min.\015Set"
#define ISTR_HOUR_MENU_LONG "Std.\012MENU LANG"
#define ISTR_DATE           "Datum"
#define ISTR_MONTH          "Monat"
#define ISTR_YEAR_TEMP      "Jahr\013Temp."
#define ISTR_YEAR           "Jahr"
#define ISTR_BATTERY        "BATTERIE"
#define ISTR_Battery        "Batterie"
#define ISTR_CURRENT_MAX    "Momentan\016Max"
#define ISTR_CPU_TEMP_MAX   "CPU temp.\014C Max\024C"
#define ISTR_MEMORY_STAT    "SPEICHER STAT"
#define ISTR_GENERAL        "Generell"
#define ISTR_Model          "Modell"
#define ISTR_RADIO_SETUP2   "FERNST EINST2"
#define ISTR_BRIGHTNESS     "Helligkeit"
#define ISTR_CAPACITY_ALARM "Kapazit\301ts Alarm"
#define ISTR_BT_BAUDRATE    "Bt baudrate" 
#define ISTR_ROTARY_DIVISOR "Rot Teiler"
#define ISTR_LV             "LV"
#define ISTR_LH             "LH"
#define ISTR_RV             "RV"
#define ISTR_RH             "RH"
#define ISTR_STICK_GAIN     "Stick Gain"
#define ISTR_STICK_DEADBAND "Stick Deadband"
#define ISTR_STICK_LV_GAIN  "Stick LV Anstieg"
#define ISTR_STICK_LH_GAIN  "Stick LH Anstieg"
#define ISTR_STICK_RV_GAIN  "Stick RV Anstieg"
#define ISTR_STICK_RH_GAIN  "Stick RH Anstieg"
#define ISTR_BIND					  "Binden"
#define ISTR_RANGE					"RWeite Test"

#define ISTR_ALRMS_OFF			"Alarme inaktiv"
#define ISTR_OLD_EEPROM			" EEPROM ist veraltet   TESTE EINSTELL/KALIB"
#define ISTR_TRIGA_OPTS			"OFFON THsTH%"
#define ISTR_CHK_MIX_SRC		"PR\304FE MIX QUELLEN"

#define ISTR_BT_TELEMETRY		"BT Telemetrie"
#define ISTR_FRSKY_COM_PORT "Telem. Com Port"
#define ISTR_INVERT_COM1		"Invert COM 1"
#define ISTR_LOG_SWITCH			"Log Schalt"
#define ISTR_LOG_RATE				"Log Rate"
#define ISTR_6_BINDING			"\006BINDE"
#define ISTR_RANGE_RSSI			"RWeite Test RSSI:"
#define ISTR_FAILSAFE				"FAILSAFE"
#define ISTR_VOLUME_CTRL		"Volume Control"
//#ifdef ASSAN
//#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiAssan"
//#else
#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiXfireAcces"
//#endif
#define ISTR_TYPE						"  Typ"
#define ISTR_COUNTRY				"    Land"
#define ISTR_SP_FAILSAFE		" Failsafe"
#define ISTR_PPM2_START			"PPM2 StartChan"
#define ISTR_FOLLOW					"Folgen"
#define ISTR_PPM2_CHANNELS	"PPM2 Kan\301le"
#define ISTR_FILTER_ADC			"Filter ADC"
#define ISTR_SCROLLING			"Scrolling"
#define ISTR_ALERT_YEL			"Alarm [Gel]"
#define ISTR_ALERT_ORG			"Alarm [Org]"
#define ISTR_ALERT_RED			"Alarm [Rot]"
#define ISTR_LANGUAGE				"Sprache"

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

#define ISTR_Mixer		      "Mischer" 
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
#define ISTR_POPUP_GLOBALS			"GVARS\0GVadjusters\0Scalers\0Telemetry\0Custom\0Mischer\0Templates\0Logging\0Blocking\0Vario\0Sensors"

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




