/************************************************* ***************************
* Derechos de autor (c) 2013 por Michael Blandford. Todos los derechos reservados.
*
* La redistribución y el uso en formas fuente y binario, con o sin
* Modificación, están permitidos siempre y cuando las siguientes condiciones
* se cumplan:
*
* 1. Las redistribuciones del código fuente deben conservar el copyright anterior
* Aviso, esta lista de condiciones y el siguiente descargo de responsabilidad.
* 2. Las redistribuciones en formato binario deben reproducir el copyright anterior
* Aviso, esta lista de condiciones y el siguiente descargo de responsabilidad en el
* Documentación y / u otros materiales proporcionados con la distribución.
* 3. Ni el nombre del autor ni los nombres de sus colaboradores pueden
* Usarse para respaldar o promocionar productos derivados de este software
* Sin el permiso previo por escrito.
*
* ESTE SOFTWARE SE PROPORCIONA LOS PROPIETARIOS DEL COPYRIGHT Y SUS COLABORADORES
* "TAL COMO ESTÁ" Y CUALQUIER EXPRESA O IMPLÍCITAS, INCLUYENDO, PERO NO
* Limitarse a, las garantías implícitas de comerciabilidad Y APTITUD
* PARA UN PROPÓSITO PARTICULAR. EN NINGÚN CASO
* EL PROPIETARIO O LOS COLABORADORES DE AUTOR SERÁN RESPONSABLES POR DAÑOS DIRECTOS, INDIRECTOS,
* INCIDENTAL, ESPECIAL, O CONSECUENTE EJEMPLARES (INCLUYENDO,
* PERO NO LIMITADA A, LA OBTENCIÓN DE BIENES O SERVICIOS SUSTITUTOS; PÉRDIDA
* DE USO, DATOS O BENEFICIOS; O INTERRUPCIÓN COMERCIAL) CAUSADOS
* Y LA TEORÍA DE LA RESPONSABILIDAD, YA SEA POR CONTRATO, RESPONSABILIDAD OBJETIVA,
* O AGRAVIO (INCLUYENDO NEGLIGENCIA O CUALQUIER OTRA FORMA) DERIVADO DE CUALQUIER FORMA DE
* EL USO DE ESTE SOFTWARE, INCLUSO SI SE HA ADVERTIDO DE LA POSIBILIDAD DE
* TALES DAÑOS.
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
 * - Rafa Cuadra
*
****************************************************************************/

#define FWx4        "\030" 
#define FWx5        "\036"
#define FWx9		"\066"
#define FWx10       "\074"
#define FWx11       "\102"
#define FWx12       "\110"
#define FWx13       "\116"
#define FWx14       "\124"
#define FWx15       "\132"
#define FWx16       "\140"
#define FWx17       "\146"
#define FWx18       "\152"

#define I_REMOVED          0


#define ISTR_ON            " ON"
#define ISTR_OFF           "OFF"
#define ISTR_X_OFF_ON      FWx17 "\001" "\003" ISTR_OFF ISTR_ON

#define ISTR_ALTEQ         "Alt=" 
#define ISTR_TXEQ          "\003Tx=Swr"
#define ISTR_RXEQ		       "Rx="
#define ISTR_TRE012AG      "TRE012AG"

// ISTR_YELORGRED indexed 3 char each
#define ISTR_YELORGRED      "\003---YelOrgRed"
#define ISTR_A_EQ           "A="
#define ISTR_SOUNDS	       "\006Warn1 ""Warn2 ""Cheap ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Crickt""Siren ""AlmClk""Ratata""Tick  ""Haptc1""Haptc2""Haptc3" "Haptc4"
#define ISTR_SWITCH_WARN	   "Aviso Switch"
//#define ISTR_SWITCH				   "SWITCH" 
#define ISTR_WARNING			   "AVISO" 
// ISTR_TIMER exactly 5 chars long
#define ISTR_TIMER           "Timer"

// ISTR_PPMCHANNELS indexed 4 Char each
#define ISTR_PPMCHANNELS     "CH"

#define ISTR_MAH_ALARM      "mAh Alarma"


// er9x.cpp
// ********
//#define ISTR_LIMITS		     "LIMITES"
#define ISTR_EE_LOW_MEM     "Memo Baja EEPROM"
#define ISTR_ALERT		      " ALERTA"
#define ISTR_THR_NOT_IDLE   "Throttle activo"
#define ISTR_RST_THROTTLE   "Resetea throttle"
#define ISTR_PRESS_KEY_SKIP "Pulsa EXIT salir"
#define ISTR_ALARMS_DISABLE "Alarmas desactivadas"
#define ISTR_OLD_VER_EEPROM " Vieja Version EEPROM   CHEQUE AJUST/CALIB"
#define ISTR_RESET_SWITCHES "Resetea Switch"
#define ISTR_LOADING        "CARGANDO"
#define ISTR_MESSAGE        "MENSAJE"
#define ISTR_PRESS_ANY_KEY  "pulsa una tecla"
#define ISTR_MSTACK_UFLOW   "mStack uflow"
#define ISTR_MSTACK_OFLOW   "mStack oflow"

#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_CHANS_GV         "\004P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCHGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7THISSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8"
#define ISTR_CHANS_RAW       	"\004P1  P2  P3  HALFFULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16CH17CH18CH19CH20CH21CH22CH23CH24SWCH"
#define ISTR_CHANS_EXTRA      "\004P4  P5  P6  P7"
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

#define ISTR_CH               "CH"
#define ISTR_TMR_MODE         "\003OFFON RUsRU%ELsEL%THsTH%ALsAL%P1 P1%P2 P2%%P3 P3%"

// Pers.cpp
// ********
#define ISTR_ME             "ME        "
#define ISTR_MODEL          "MODELO     "
#define ISTR_BAD_EEPROM     "Error EEprom"
#define ISTR_EE_FORMAT      "Formatea EEPROM"
#define ISTR_GENWR_ERROR    "Error escritura"
#define ISTR_EE_OFLOW       "EEPROM exedida"

// Templates.cpp
// ***********
#define ISTR_T_S_4CHAN      "Simple 4-CH"
#define ISTR_T_TCUT         "T-Cut"
#define ISTR_T_STICK_TCUT   "Sticky T-Cut"
#define ISTR_T_V_TAIL       "V-Tail"
#define ISTR_T_ELEVON       "Elevon\\Delta"
#define ISTR_T_HELI_SETUP   "Ajustes Heli"
#define ISTR_T_GYRO         "Ajustes Gyro"
#define ISTR_T_SERVO_TEST   "Test Servo"
#define ISTR_T_RANGE_TEST   "Test Rango"

// Menus.cpp
// ***********
#define ISTR_TELEM_ITEMS	  "\004----A1= A2= RSSITSSITim1Tim2Alt GaltGspdT1= T2= RPM FUELMah1Mah2CvltBattAmpsMah CtotFasVAccXAccYAccZVspdGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7FwatRxV Hdg A3= A4= SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 RTC TmOKAspdCel1Cel2Cel3Cel4Cel5Cel6RBv1RBa1RBv2RBa2RBm1RBm2RBSVRBSTCel7Cel8Cel9Cl10Cl11Cl12Cus1Cus2Cus3Cus4Cus5Cus6Fmd RunTModTCls1Cls2SbcVSbcA"
#define ISTR_TELEM_SHORT    "\004----TIM1TIM2BATTGvr1Gvr2Gvr3Gvr4Gvr5Gvr6Gvr7"
#define ISTR_GV             "GV"
#define ISTR_OFF_ON         "OFF ON"
#define ISTR_HYPH_INV       FWx18 "\001" "\003---INV"
#define ISTR_VERSION        "VERSION"
#define ISTR_Music	        "Music"
#define ISTR_SLAVE          "\007Esclavo" 
#define ISTR_MENU_DONE      "[MENU] para salir"
#define ISTR_CURVES         "CURVAS"
#define ISTR_CURVE          "CURVA"
#define ISTR_GLOBAL_VAR     "GLOBAL VAR"
#define ISTR_VALUE          "Valor"
#define ISTR_PRESET         "PROGRAMAR"
#define ISTR_CV             "CV"
#define ISTR_LIMITS         "LIMITES"
#define ISTR_COPY_TRIM      "COPIA TRIM [MENU]"
#define ISTR_TELEMETRY      "TELEMETRIA"
#define ISTR_USR_PROTO      "UsrProto"
#define ISTR_FRHUB_WSHHI    "\006FrSky WSHhi DSMx  Jeti  ArduP ArduC FrHub HubRawFrMav Mavlk Hitec AFHDS2"
#define ISTR_MET_IMP        "\003MetImp"
#define ISTR_A_CHANNEL      "Un canal"
//#define ISTR_ALRM           "alarma"
//#define ISTR_TELEMETRY2     "TELEMETRIA2"
#define ISTR_TX_RSSIALRM    "TxRSSIalrm"
#define ISTR_NUM_BLADES     "Num Palas"
#define ISTR_ALT_ALARM      "AltAlarm"
#define ISTR_OFF122400      "\003OFF122400"
#define ISTR_VOLT_THRES     "Volt Thres="
#define ISTR_GPS_ALTMAIN    "GpsAltMain"
#define ISTR_CUSTOM_DISP    "Editar Pantalla"
#define ISTR_FAS_OFFSET     "FAS Offset"
#define ISTR_VARIO_SRC      "Vario: Fuente"
#define ISTR_VSPD_A2        "\004----vspdA2  SC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
#define ISTR_2SWITCH        "\001Switch"
#define ISTR_2SENSITIVITY   "\001Sensible"
#define ISTR_GLOBAL_VARS    "GLOBAL VARS"
#if defined(PCBSKY) || defined(PCB9XT)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 P3 c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#ifdef PCBX9D
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#if defined(PCBX12D) || defined(PCBX10)
#define ISTR_GV_SOURCE      "\003---RtmEtmTtmAtmRENRudEleThrAilP1 P2 SL SR c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16c17c18c19c20c21c22c23c24SC1SC2SC3SC4SC5SC6SC7SC8O1 O2 O3 O4 O5 O6 O7 O8 O9 O10O11O12O13O14O15O16O17O18O19O20O21O22O23O24"
#define ISTR_EXTRA_SOURCE   ""
#endif
#define ISTR_TEMPLATES      "PLANTILLAS"
#define ISTR_CHAN_ORDER     "Ordena Canales"
#define ISTR_SP_RETA        " RETA"
#define ISTR_CLEAR_MIXES    "BORRAR MEZCLAS [MENU]"
#define ISTR_SAFETY_SW      "SW SEGURIDAD"
#define ISTR_NUM_VOICE_SW   "Num Voces Sw"
#define ISTR_V_OPT1         "\007 8 Secs12 Secs16 Secs"
#define ISTR_VS             "VS"
#define ISTR_VOICE_OPT      "\006ON    OFF   AMBOS 15Secs30Secs60SecsVaribl"
#define ISTR_CUST_SWITCH    "SWITCH LOGICOS"
#define ISTR_S              "S"
#define ISTR_15_ON          "\015On"
#define ISTR_EDIT_MIX       "EDITAR MEZCLA"
#define ISTR_2SOURCE        "\001Fuente"
#define ISTR_2WEIGHT        "\001Weight"
#define ISTR_FMTRIMVAL      "FmTrimVal"
#define ISTR_OFFSET         "Offset"
#define ISTR_2FIX_OFFSET    "\001Fix Offset"
#define ISTR_ENABLEEXPO     "\001ActivoExpoDr"
#define ISTR_2TRIM          "\001Trim"
#define ISTR_15DIFF         "\010Diff"
#define ISTR_Curve          "Curva"
#define ISTR_2WARNING       "\001Aviso"
#define ISTR_2MULTIPLEX     "\001Multpx"
// ISTR_ADD_MULT_REP indexed 8 chars each
#define ISTR_ADD_MULT_REP   "\010Add     MultiplyReplace "
#define ISTR_2DELAY_DOWN    "\001Retardo Abajo"
#define ISTR_2DELAY_UP      "\001Retardo Arriba"
#define ISTR_2SLOW_DOWN     "\001Lento Abajo"
#define ISTR_2SLOW_UP       "\001Lento Arriba"
#define ISTR_MAX_MIXERS     "alcance max mix: 32"
#define ISTR_PRESS_EXIT_AB  "Pulsa [EXIT] salir"
#define ISTR_YES_NO         "\003SI\013NO"
#define ISTR_MENU_EXIT      "\003[MENU]\013[EXIT]"
#define ISTR_DELETE_MIX     "BORRAR MEXCLAS?"
#define ISTR_MIX_POPUP      "EDITAR\0INSERT\0COPIAR\0MOVER\0BORRAR\0B.TODO\0TEMPLATES\0PASTE"
#define ISTR_MIXER          "MEXCLAS"
// CHR_S S for Slow
#define ICHR_S              "S"
// CHR_D D for Delay
#define ICHR_D              "D"
// CHR_d d for differential
#define ICHR_d              "d"
#define ISTR_EXPO_DR        "EXPO/DR"
#define ISTR_4DR_MID        "\004DR Med"
#define ISTR_4DR_LOW        "\004DR Baj"
#define ISTR_4DR_HI         "\004DR Alt"
#define ISTR_2EXPO          "\002Expo"
#define ISTR_DR_SW1         "DrSw1"
#define ISTR_DR_SW2         "DrSw2"
#define ISTR_DUP_MODEL      "COPIAR MODELO"
#define ISTR_DELETE_MODEL   "BORRAR MODELO"
#define ISTR_DUPLICATING    "Modelo copiado"
#define ISTR_SETUP          "Ajuste Modelo"
#define ISTR_NAME           "Nombre"
#define ISTR_VOICE_INDEX    "Voces\021MENU"
#define ISTR_TRIGGERA       "Trigger"
#define ISTR_TRIGGERB       "TriggerB"
//ISTR_COUNT_DOWN_UP indexed, 10 chars each
#define ISTR_COUNT_DOWN_UP  "\012CuentaDownCuenta Up "
//#define ISTR_COUNT_DOWN_UP  "\012Cuenta AtrasCuenta Adelante"
#define ISTR_T_TRIM         "Thr-Trim"
#define ISTR_T_EXPO         "T-Expo-Dr"
// ISTR_TRIM_OPTIONS indexed 6 chars each
#define ISTR_TRIM_OPTIONS   FWx9 "\004" "\006Exp   ExFinoFino  Medio Grueso"
#define ISTR_TRIM_SWITCH    "Insta-Trim Sw"
#define ISTR_TRIM_INC       "Trim Inc"
#define ISTR_BEEP_CENTRE    "Beep Cnt"
#define ISTR_RETA123        "RETA1234"
#define ISTR_PROTO          "Proto"
// ISTR_21_USEC after \ 021 max 4 chars
#define ISTR_21_USEC        "\021uSec"
#define ISTR_13_RXNUM       "\014RxNum"
// ISTR_23_US after \ 023 max 2 chars
#define ISTR_23_US          "\023uS"
// ISTR_PPMFRAME_MSEC before \015 max 9 chars, after max 4 chars
#define ISTR_PPMFRAME_MSEC  "PPM FrLen\016mSec"
#define ISTR_SEND_RX_NUM    " Busca num RX [MENU]"
#define ISTR_DSM_TYPE       " DSM Tipo"
#define ISTR_PPM_1ST_CHAN   "1er Canal"
#define ISTR_SHIFT_SEL      "Polaridad"
// ISTR_POS_NEG indexed 3 chars each
#define ISTR_POS_NEG        "\003POSNEG"
#define ISTR_E_LIMITS       "E. Limites"
#define ISTR_Trainer        "Entrenador"
#define ISTR_T2THTRIG       "T2ThTrig"
#define ISTR_AUTO_LIMITS    "Auto Limites"
// ISTR_1_RETA indexed 1 char each
#define ISTR_1_RETA         "\001RETA"
#define ISTR_FL_MODE        "FL MODO"
#define ISTR_SWITCH         "Switch"
#define ISTR_TRIMS          "Trims"
#define ISTR_MODES          "MODOS"
#define ISTR_SP_FM0         " FM0"
#define ISTR_SP_FM          " FM"
#define ISTR_HELI_SETUP     "AJUSTES HELI"
#define ISTR_SWASH_TYPE     "Tipo Plato"
#define ISTR_COLLECTIVE     "Collectivo"
#define ISTR_SWASH_RING     "Anillo Plato"
#define ISTR_ELE_DIRECTION  "ELE Direccion"
#define ISTR_AIL_DIRECTION  "AIL Direccion"
#define ISTR_COL_DIRECTION  "COL Direccion"
#define ISTR_HELI_TEXT      ISTR_SWASH_TYPE "\037" ISTR_COLLECTIVE "\037" ISTR_SWASH_RING "\037" ISTR_ELE_DIRECTION "\037" ISTR_AIL_DIRECTION "\037" ISTR_COL_DIRECTION
//#define ISTR_MODEL_POPUP    "SELECT\0COPIAR\0MOVER\0BORRAR"
#define ISTR_MODEL_POPUP    "EDITAR\0SELECT\0SEL/EDIT\0COPIAR\0MOVER\0BORRAR\0BACKUP\0RESTAURAR\0REPLACE\0NOTES"
#define ISTR_MODELSEL       "SELEC MODELO"
// ISTR_11_FREE after \011 max 4 chars
#define ISTR_11_FREE        "\011libre"
#define ISTR_CALIBRATION    "CALIBRAR"
// ISTR_MENU_TO_START after \003 max 15 chars
#define ISTR_MENU_TO_START  "\003[MENU] INICIAR"
// ISTR_SET_MIDPOINT after \005 max 11 chars
#define ISTR_SET_MIDPOINT   "\005CENTRAR STICK"
// ISTR_MOVE_STICKS after \003 max 15 chars
#define ISTR_MOVE_STICKS    "\003MOVER STICKS/POTS"
#define ISTR_ANA            "ANA"
#define ISTR_DIAG           "DIAG"
// ISTR_KEYNAMES indexed 5 chars each
#define ISTR_KEYNAMES       "\005 MenuSalirAbajoArribDerecIquie"
#define ISTR_TRIM_M_P       "Trim- +"
// ISTR_OFF_PLUS_EQ indexed 3 chars each
#define ISTR_OFF_PLUS_EQ    "\003off += :="
// ISTR_CH1_4 indexed 3 chars each
#define ISTR_CH1_4          "\003ch1ch2ch3ch4"
#define ISTR_MULTIPLIER     "Multiplo"
#define ISTR_CAL            "Cal"
#define ISTR_MODE_SRC_SW    "\003modo\012% src  sw"
#define ISTR_RADIO_SETUP    "AJUSTE RADIO"
#define ISTR_OWNER_NAME     "Nombre Usuario"
#define ISTR_BEEPER         "Beeper"
// ISTR_BEEP_MODES indexed 6 chars each
#define ISTR_BEEP_MODES     "\006Quiet ""NOKEY ""xShort ""Corto ""Norma ""Long  ""xLong "
#define ISTR_SOUND_MODE     "Modo Sonido"
// ISTR_SPEAKER_OPTS indexed 10 chars each
#define ISTR_SPEAKER_OPTS   "\012Beeper    ""PiSpkr    ""BeeprVoice""PiSpkVoice""MegaSound "
#define ISTR_VOLUME         "Volumen"
#define ISTR_SPEAKER_PITCH  "Tono Altavoz"
#define ISTR_HAPTICSTRENGTH "Fuerza hactic"
#define ISTR_CONTRAST       "Contraste"
#define ISTR_BATT_WARN      "Aviso bateria" 
// ISTR_INACT_ALARM m for minutes after \023 - single char
#define ISTR_INACT_ALARM    "Alarma inactiva\023m"
#define ISTR_THR_REVERSE    "Throttle reverso"
#define ISTR_MINUTE_BEEP    "Beep minuto"
#define ISTR_BEEP_COUNTDOWN "Beep cuentaatras"
#define ISTR_FLASH_ON_BEEP  "Flash on beep"
#define ISTR_LIGHT_SWITCH   "Luz switch"
#define ISTR_LIGHT_INVERT   "Invert luz"
#define ISTR_LIGHT_AFTER    "Luz con Tecla"
#define ISTR_LIGHT_STICK    "Luz con Stk Mv"
#define ISTR_SPLASH_SCREEN  "Pantalla Inicio"
#define ISTR_SPLASH_NAME    "Nombre Inicio"
#define ISTR_THR_WARNING    "Aviso Throttle"
#define ISTR_DEAFULT_SW     "Defecto Sw"
#define ISTR_MEM_WARN       "Aviso Memoria"
#define ISTR_ALARM_WARN     "Aviso Alarma"
#define ISTR_POTSCROLL      "PotScroll"
#define ISTR_STICKSCROLL    "StickScroll"
#define ISTR_BANDGAP        "BandGap"
#define ISTR_ENABLE_PPMSIM  "Activo PPMSIM"
#define ISTR_CROSSTRIM      "CrossTrim"
#define ISTR_INT_FRSKY_ALRM "Int. Frsky alarm"
#define ISTR_MODE           "Modo"

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
#define ISWITCH_WARN_STR	   "Aviso Switch"
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

#define ISTR_SINK_TONES	   "Caida Tonos"


// ersky9x strings
#define ISTR_ST_CARD_STAT   "SD CARD STAT"
#define ISTR_4_READY        "\004Inicio"
#define ISTR_NOT            "NO"
#define ISTR_BOOT_REASON    "ENCENDIDO"
#define ISTR_6_WATCHDOG     "\006WATCHDOG"
#define ISTR_5_UNEXPECTED   "\005INESPERADO"
#define ISTR_6_SHUTDOWN     "\006APAGADO"
#define ISTR_6_POWER_ON     "\006ENCENDIDO"
// ISTR_MONTHS indexed 3 chars each
#define ISTR_MONTHS         "\003XxxJanFebMarAprMayJunJulAugSepOctNovDec"
#define ISTR_MENU_REFRESH   "[MENU] REFRESCAR"
#define ISTR_DATE_TIME      "FECHA-HORA"
#define ISTR_SEC            "Sec."
#define ISTR_MIN_SET        "Min.\015Set"
#define ISTR_HOUR_MENU_LONG "Hora\012MENU LARGO"
#define ISTR_DATE           "Dia"
#define ISTR_MONTH          "Mes"
#define ISTR_YEAR_TEMP      "Ano\013Temp."
#define ISTR_YEAR           "Ano"
#define ISTR_BATTERY        "BATERIA"
#define ISTR_Battery        "Bateria"
#define ISTR_CURRENT_MAX    "Corriente\016Max"
#define ISTR_CPU_TEMP_MAX   "CPU temp.\014C Max\024C"
#define ISTR_MEMORY_STAT    "MEMORIA STAT"
#define ISTR_GENERAL        "General"
#define ISTR_Model          "Modelo"
#define ISTR_RADIO_SETUP2   "AJUSTES RADIO2"
#define ISTR_BRIGHTNESS     "Brillo"
#define ISTR_CAPACITY_ALARM "Alarma Capacidad"
#define ISTR_BT_BAUDRATE    "Bt baudios"
#define ISTR_ROTARY_DIVISOR "Rotar Pantalla"
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
#define ISTR_BIND					  "Bind"
#define ISTR_RANGE					"Cheque Rango"

#define ISTR_ALRMS_OFF			"Alarmas Inactiva"
#define ISTR_OLD_EEPROM			" Vieja Version EEPROM   CHEQUEO AJUSTES/CALIB"
#define ISTR_TRIGA_OPTS			"OFFON THsTH%"
#define ISTR_CHK_MIX_SRC		"CHEQUEO FUENTE MEXCLAS"

#define ISTR_BT_TELEMETRY		"BT Telemetria"
#define ISTR_FRSKY_COM_PORT "Telem. Com Port"
#define ISTR_INVERT_COM1		"Invert COM 1"
#define ISTR_LOG_SWITCH			"Switch Registro"
#define ISTR_LOG_RATE				"Tasa de Registro"
#define ISTR_6_BINDING			"\006BINDING"
#define ISTR_RANGE_RSSI			"CHEQUEO RANGO RSSI:"
#define ISTR_FAILSAFE				"FAILSAFE"
#define ISTR_VOLUME_CTRL		"Volumen Control"
//#ifdef ASSAN
//#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiAssan"
//#else
#define ISTR_PROT_OPT				"\005PPM  XJT  DSM2 MultiXfireAcces"
//#endif
#define ISTR_TYPE						" Tipo"
#define ISTR_COUNTRY				" Region"
#define ISTR_SP_FAILSAFE		" Failsafe"
#define ISTR_PPM2_START			"PPM2 StartChan"
#define ISTR_FOLLOW					"Segir"
#define ISTR_PPM2_CHANNELS	"PPM2 Canales"
#define ISTR_FILTER_ADC			"Filtro ADC"
#define ISTR_SCROLLING			"Scrolling"
#define ISTR_ALERT_YEL			"Alerta [Ama]"
#define ISTR_ALERT_ORG			"Alerta [Nar]"
#define ISTR_ALERT_RED			"Alerta [Roj]"
#define ISTR_LANGUAGE				"Idioma"

#define ISTR_RSSI_WARN		  "Aviso RSSI"
#define ISTR_RSSI_CRITICAL  "RSSI Critica"
#define ISTR_RX_VOLTAGE		  "Rx Voltage"
#define ISTR_DSM_WARNING	  "Aviso DSM"
#define ISTR_FADESLOSSHOLDS "\006Perdida de senal "
#define ISTR_DSM_CRITICAL	  "DSM Critica"
#define ISTR_BT_TRAINER		  "BT Entrenador"
#define ISTR_MULTI_TEXT     "Protocolo\037Tipo\037Autobind\037Power"
#define ISTR_MULTI_PROTO    "Protocolo"
#define ISTR_MULTI_TYPE	    "Tipo"
#define ISTR_MULTI_AUTO	    "Autobind"
#define ISTR_MULTI_POWER    "Power"
#define ISTR_MULTI_OPTION   "\013Opcion"

#define ISTR_Display		     "Pantallas" 
#define ISTR_AudioHaptic		 "AudioHaptic" 
#define ISTR_Alarms			     "Alarmas" 
#define ISTR_General		     "General" 
#define ISTR_Controls			   "Controles"
#define ISTR_Hardware			   "Hardware"
#define ISTR_Calibration		 "Calibrar" 
//#define ISTR_Trainer		     "Entrenador" 
#define ISTR_Version		     "Version" 
#define ISTR_ModuleRssi			 "FrSky xSSI"
#define ISTR_DateTime			   "Fecha-Hora" 
#define ISTR_DiagSwtch		   "DiagSwtch"  
#define ISTR_DiagAna		     "DiagAna" 

#define ISTR_Mixer		      "Mexclas" 
#define ISTR_Cswitches		  "L.Switch" 
#define ISTR_Telemetry		  "Telemetria" 
#define ISTR_limits			    "Limites" 
#define ISTR_Bluetooth		  "BlueTooth" 
#define ISTR_heli_setup			"Heli" 
#define ISTR_Curves			    "Curvas" 
#define ISTR_Expo				    "Expo/D.Rate" 
#define ISTR_Globals		    "Globales" 
#define ISTR_Timer		      "Tiempos" 
#define ISTR_Modes			 		"Modos" 
#define ISTR_Voice		      "VocesAudio" 
#define ISTR_Protocol			  "Protocolo" 
#define ISTR_Safety					"Seguridad"
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
#define ISTR_POPUP_GLOBALS			"GVARS\0GVadjusters\0Scalers\0Telemetry\0Custom\0Mexclas\0Templates\0Logging\0Blocking\0Vario\0Sensors"

#define ISTR_SHUT_DOWN					"Shutting Down"

//"Fuente Actual"
//"\004----A1  A2  FASVSC1 SC2 SC3 SC4 SC5 SC6 SC7 SC8 "
//"SC  ="
//"Fuente"
//"Multiplo"
//"Divisor"
//"Unit"
//"Sign"
//"Decimales"
//"Offset At"
//"\005FirstLast "      //  5 chars each only //"\005PrimeroUltimo "
//"Voces Switch"
//"Function"
//"\007----   v>val  v<val  |v|>val|v|<valON     OFF    AMBOS  "
//"Switch"
//"Tasa"
//"\017Once"
//"Offset"
//"Tipo Archivo"
//"\006  NombreNumeroHaptic"
//"Archivo Voces"
//"\006Haptc1Haptc2Haptc3"
// SKY "\003IDxTHRRUDELEAILGEATRN"
// X9D "\002SASBSCSDSESFSGSH"
//"\002MODOS"
// SKY "\004sIDxsTHRsRUDsELEsAILsGEAsTRN"
// X9D "\002SASBSCSDSESFSGSH"
//"Resetea Switch"
// SKY "\003---P1 P2 P3 GV4GV5GV6GV7"
// X9D "\003---P1 P2 SL SR GV5GV6GV7"
//"Interno"
//"\003AmeJapEur"
//"Externo"
//"Fade In"
//"Fade Out"
//"Nombre"
//"Co Proc"
//"On Time"
//"ttimer1        us"
//"\013rssi"
//"Vbat"
//"\013RxV"
//"AMP\013Temp"
//"RPM\021DSM2"
//"AJUSTES"
//"Pantallas"
//"AudioHaptic"
//"Alarmas"
//"General"
//"Controles"
//"Hardware"
//"Calibrar"
//"Entrenador"
//"Version"
//"Fecha-Hora"
//"DiagSwtch"
//"DiagAna"
//"PANTALLA"
//"AZUL"
//"BLANCO"
//"Optrex Pantalla"
//"AUDIO/HAPTIC"
//"ALARMAS"
//"[Next]"
//"GENERAL"
//"\012   ENGLISH  FRANCAIS   DEUTSCH NORWEGIAN   SWEDISH SPANISH" 
//"\005NONE POT  STICKBOTH "  // 5 chars each  //"\005NO   TOQUEAMBOS STICK "
//"CONTROLES"
//"HARDWARE"
//"ELE  switch"
//"\0042POS3POS6POS"
//"THR  switch"
//"\0042POS3POS"
//"RUD  switch"
//"GEAR switch"
//"AJUSTES MODELOS"
//"Mexclas"
//"C.Switches"
//"Telemetria"
//"Limites"
//"pantalla"
//"MPANTALLA"
//"\001Haptic Min Inicio"
