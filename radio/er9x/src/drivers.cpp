/*
 * Author - Erez Raviv <erezraviv@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include "er9x.h"
#include "pulses.h"
#include "audio.h"

#ifndef SIMU
#include "avr/interrupt.h"

//#define GREEN_CHIP	1

///opt/cross/avr/include/avr/eeprom.h
static inline void __attribute__ ((always_inline))
eeprom_write_byte_cmp (uint8_t dat, uint16_t pointer_eeprom)
{
  //see /home/thus/work/avr/avrsdk4/avr-libc-1.4.4/libc/misc/eeprom.S:98 143
#ifdef CPUM2561
  while(EECR & (1<<EEPE)) /* make sure EEPROM is ready */
#else
  while(EECR & (1<<EEWE)) /* make sure EEPROM is ready */
#endif
  {
    if (Ee_lock & EE_TRIM_LOCK)    // Only if writing trim changes
    {
      mainSequence() ;      // Keep the controls running while waiting		
    }
  } ;
  EEAR  = pointer_eeprom;

  EECR |= 1<<EERE;
  if(dat == EEDR) return;

  EEDR  = dat;
  uint8_t flags=SREG;
  cli();
#ifdef CPUM2561
  EECR |= 1<<EEMPE;
  EECR |= 1<<EEPE;
#else
  EECR |= 1<<EEMWE;
  EECR |= 1<<EEWE;
#endif
  SREG = flags;
}

void eeWriteBlockCmp(const void *i_pointer_ram, uint16_t i_pointer_eeprom, size_t size)
{
  const char* pointer_ram = (const char*)i_pointer_ram;
  uint16_t    pointer_eeprom = i_pointer_eeprom;
  while(size){
    eeprom_write_byte_cmp(*pointer_ram++,pointer_eeprom++);
    size--;
  }
}
#endif

//inline uint16_t anaIn(uint8_t chan)
//{
//  //                     ana-in:   3 1 2 0 4 5 6 7
//  static prog_char APM crossAna[]={4,2,3,1,5,6,7,0}; // wenn schon Tabelle, dann muss sich auch lohnen
//  return s_ana[pgm_read_byte(crossAna+chan)] / 4;
//}


//uint8_t ExtraInputs ;
uint8_t s_evt;

//uint8_t getEvent()
//{
//  uint8_t evt = s_evt;
//  s_evt=0;
//  return evt;
//}

class Key
{
#define FILTERBITS      4
#define FFVAL          ((1<<FILTERBITS)-1)
#define KSTATE_OFF      0
#define KSTATE_RPTDELAY 95 // gruvin: longer dely before key repeating starts
  //#define KSTATE_SHORT   96
#define KSTATE_START   97
#define KSTATE_PAUSE   98
#define KSTATE_KILLED  99
  uint8_t m_vals:FILTERBITS;   // key debounce?  4 = 40ms
  uint8_t unused_m_dblcnt:2;
  uint8_t m_cnt;
  uint8_t m_state;
public:
  void input(bool val, EnumKeys enuk);
  bool state()       { return m_vals==FFVAL;                }
#ifdef FAILSAFE
  bool isKilled()    { return m_state == KSTATE_KILLED ;    }
#endif
  void pauseEvents() { m_state = KSTATE_PAUSE;  m_cnt   = 0;}
  void killEvents()  { m_state = KSTATE_KILLED; /*m_dblcnt=0;*/ }
//  uint8_t getDbl()   { return m_dblcnt;                     }
};


Key keys[NUM_KEYS];
void Key::input(bool val, EnumKeys enuk)
{
  //  uint8_t old=m_vals;
	uint8_t t_vals ;
//  m_vals <<= 1;  if(val) m_vals |= 1; //portbit einschieben
	t_vals = m_vals ;
	t_vals <<= 1 ;
  if(val) t_vals |= 1; //portbit einschieben
	m_vals = t_vals ;
  m_cnt++;

  if(m_state && m_vals==0){  //gerade eben sprung auf 0
    if(m_state!=KSTATE_KILLED) {
      putEvent(EVT_KEY_BREAK(enuk));
//      if(!( m_state == 16 && m_cnt<16)){
//        m_dblcnt=0;
//      }
        //      }
    }
    m_cnt   = 0;
    m_state = KSTATE_OFF;
  }
  switch(m_state){
    case KSTATE_OFF:
      if(m_vals==FFVAL){ //gerade eben sprung auf ff
        m_state = KSTATE_START;
//        if(m_cnt>16) m_dblcnt=0; //pause zu lang fuer double
        m_cnt   = 0;
      }
      break;
      //fallthrough
    case KSTATE_START:
      putEvent(EVT_KEY_FIRST(enuk));
      Inactivity.inacCounter = 0;
//      m_dblcnt++;
#ifdef KSTATE_RPTDELAY
      m_state   = KSTATE_RPTDELAY;
#else
      m_state   = 16;
#endif
      m_cnt     = 0;
      break;
#ifdef KSTATE_RPTDELAY
    case KSTATE_RPTDELAY: // gruvin: longer delay before first key repeat
      if(m_cnt == 32) putEvent(EVT_KEY_LONG(enuk)); // need to catch this inside RPTDELAY time
      if (m_cnt == 40) {
        m_state = 16;
        m_cnt = 0;
      }
      break;
#endif
    case 16:
#ifndef KSTATE_RPTDELAY
      if(m_cnt == 32) putEvent(EVT_KEY_LONG(enuk));
      //fallthrough
#endif
    case 8:
    case 4:
    case 2:
      if(m_cnt >= 48)  { //3 6 12 24 48 pulses in every 480ms
        m_state /= 2 ;  //        m_state >>= 1;
        m_cnt     = 0;
      }
      //fallthrough
    case 1:
      if( (m_cnt & (m_state-1)) == 0)  putEvent(EVT_KEY_REPT(enuk));
      break;

    case KSTATE_PAUSE: //pause
      if(m_cnt >= 64)      {
        m_state = 8;
        m_cnt   = 0;
      }
      break;

    case KSTATE_KILLED: //killed
      break;
  }
}

#ifdef FAILSAFE
uint8_t menuPressed()
{
	if ( keys[KEY_MENU].isKilled() )
	{
		return 0 ;
	}
	return ( read_keys() & 2 ) == 0 ;
}
#endif

static uint8_t readAilIp()
{
#if defined(CPUM128) || defined(CPUM2561)
	if ( g_eeGeneral.FrskyPins )
	{
		return PINC & (1<<INP_C_AileDR) ;
	}
	else
	{
		return PINE & (1<<INP_E_AileDR) ;
	}
#else
 #if (!(defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA)))
 	return PINE & (1<<INP_E_AileDR);
 #else
 	return PINC & (1<<INP_C_AileDR);
 #endif
#endif
}

#ifdef XSW_MOD
bool parVoiceInstalled()
{
#ifdef SERIAL_VOICE
#ifdef SERIAL_VOICE_ONLY
  return false;
#else
  return ((g_eeGeneral.speakerMode & 2) && !g_eeGeneral.MegasoundSerial);
#endif
#else // !SERIAL_VOICE
  return ((g_eeGeneral.speakerMode & 2) != 0);
#endif
}

#if defined(CPUM128) || defined(CPUM2561)
void initLVTrimPin()
{
  if (g_eeGeneral.LVTrimMod) {
    unsetSwitchSource(SSW_PC0);
    unsetSwitchSource(SSW_PC4);
    DDRC &= ~((1 << 4) | (1 << 0)); // input PC4/PC0
    PORTC |= ((1 << 4) | (1 << 0)); // pullup PC4/PC0
  }
}

void initBacklightPin()
{
  if (!parVoiceInstalled() && g_eeGeneral.pb7backlight) {
    unsetSwitchSource(SSW_PB7);
    DDRB |= (1 << 7);   // PB7->output
    PORTB &= ~(1 << 7); // PB7->LO
  }
}
#endif

void initHapticPin()
{
  if ( g_eeGeneral.hapticStrength == 0 ) {
    DDRG &= ~(1 << 2);  // make PG2 input
  } else {
    unsetSwitchSource(SSW_PG2);
    DDRG |= (1 << 2);   // make PG2 output
  }
}

// extra switch related
#if defined(CPUM128) || defined(CPUM2561)
#define SAVE_RAM  1     // saves 21B at the expense of 164B of flash
#endif
#ifndef SAVE_RAM
static int8_t switchMapTable[MAX_PSW3POS*3+3];  // +3: "---","PB1","PB2"
#endif
int8_t  MaxSwitchIndex;     // max switch selection menu list index
uint8_t MappedSwitchState;  // mapped switch state bit flag
                            // (MSB:PB2,PB1,GEA,AIL,ELE,RUD,THR,IDL)
inline void mapSwitch(uint8_t swidx0)
{
  MappedSwitchState |= (1 << swidx0);
}

inline void unmapSwitch(uint8_t swidx0)
{
  MappedSwitchState &= ~(1 << swidx0);
}

static void setMaxSwitchIndex()
{
#ifdef SAVE_RAM
  int8_t mappedSw = 0;
  for (uint8_t i = 0; i < (MAX_XSWITCH+1); i++) { // +1 for IDL
    if (qSwitchMapped(i)) { // either 3-pos or push butt
      mappedSw++;
      if (i >= MAX_PSW3POS) // mapped PB1 or PB2
        continue;
      mappedSw++;
    }
    if (i < MAX_PSW3POS)
      mappedSw++;
  }
  MaxSwitchIndex = mappedSw + MAX_CSWITCH + 2;  // +2: DSW___ and DSW_TRN
#else
  int8_t *p = switchMapTable;
  uint8_t i3 = 0;
  *p++ = 0;   // DSW____
  for (uint8_t i2 = 0; i2 < (MAX_XSWITCH+1); i2++, i3 += 3) { // +1 for IDL
    if (qSwitchMapped(i2))
		{  // either 3-pos or push butt
      if (i2 < MAX_PSW3POS)
			{ // ID0,ID1,ID2,THR^,THR-,THRv,...,GE,^GE-,GEv
        *p++ = DSW_ID0 + i3 + 0;
        *p++ = DSW_ID0 + i3 + 1;
        *p++ = DSW_ID0 + i3 + 2;
      }
			else
			{    // PB1/PB2
        *p++ = DSW_IDL + i2;
      }
    }
		else if (i2 < MAX_PSW3POS)
		{
      *p++ = DSW_IDL + i2;    // DSW_THR,DSW_RUD,DSW_ELE,DSW_AIL,DSW_GEA
    }
  }
  MaxSwitchIndex = (p - switchMapTable) + MAX_CSWITCH + 1;  // +1: DSW_TRN
#endif
}

// number of mapped switches
#define SW_MAPPED (MaxSwitchIndex-MAX_CSWITCH-2)

void initSwitchMapping()
{
  MappedSwitchState = 1;  // reserve IDL bit as it's a permanent 3-pos
  for (uint8_t i = 0; i < MAX_XSWITCH; i++) {
    uint8_t s = getSwitchSource(i);
    if (s != SSW_NONE) {
      mapSwitch(i + 1);   // from THR ... PB2
    }
  }
  setMaxSwitchIndex();
}

// composing mask bits corresponding to qualified switch sources
uint16_t getSwitchSourceMask()
{
  uint16_t m = 1;               // SSW_NONE
#ifdef SERIAL_VOICE
  if (g_eeGeneral.speakerMode & 2)
 #ifndef SERIAL_VOICE_ONLY
    if (g_eeGeneral.MegasoundSerial)
 #endif
    {
      m |= (0x7F << SSW_XBASE); // VoiceSerialValue's MSB's taken for BUSY bit
    }
#endif
  if (!g_eeGeneral.pb7backlight && !parVoiceInstalled())
    m |= (1 << SSW_PB7);
  if (!g_eeGeneral.LVTrimMod) {
    if (!g_eeGeneral.serialLCD)
        m |= (1 << SSW_PC4);    // SSW_PC4
    m |= (1 << SSW_PC0);        // SSW_PC0
    m &= ~(3 << SSW_XPD3);      // reserve SSW_XPD3/XPD4 for LV trim switches
  }
#if defined(CPUM128) || defined(CPUM2561)
  if (!g_eeGeneral.FrskyPins)
    m |= (3 << SSW_PC6);        // SSW_PC6/PC7
#elif (!(defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA)))
  m |= (3 << SSW_PC6);          // SSW_PC6/PC7
#endif
  if (g_eeGeneral.hapticStrength == 0)
    m |= (1 << SSW_PG2);
#ifdef CPUM2561
  m |= (1 << SSW_PG5);
#endif
  return m;
}

void initSwitchSrcPin( uint8_t src )
{
  switch (src) {
    case SSW_PB7:
      DDRB &= ~(1 << 7);  // input PB7
      PORTB |= (1 << 7);  // pullup PB7
      break;
    case SSW_PC0:
      DDRC &= ~(1 << 0);  // input PC0
      PORTC |= (1 << 0);  // pullup PC0
      break;
#if !SERIAL_LCD
    case SSW_PC4:
      DDRC &= ~(1 << 4);  // input PC4
      PORTC |= (1 << 4);  // pullup PC4
      break;
#endif
#if (!defined(CPUM64) || \
     !(defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA)))
    case SSW_PC6:
      DDRC &= ~(1 << 6);  // input PC6
      PORTC |= (1 << 6);  // pullup PC6
      break;
    case SSW_PC7:
      DDRC &= ~(1 << 7);  // input PC7
      PORTC |= (1 << 7);  // pullup PC7
      break;
#endif
    case SSW_PG2:
      DDRG &= ~(1 << 2);  // input PG2
      PORTG |= (1 << 2);  // pullup PG2
      break;
#ifdef CPUM2561
    case SSW_PG5:
      DDRG &= ~(1 << 5);  // input PG5
      PORTG |= (1 << 5);  // pullup PG5
      break;
#endif
    default:
      break;
  }
}

void setSwitchSource( uint8_t xsw, uint8_t src )
{
  uint8_t vo = g_eeGeneral.switchSources[xsw >> 1];
  uint8_t v1 = (xsw & 1) ? (vo >> 4) : (vo & 0x0F);
  if (v1 != src) {  // switch source changed
    if (xsw & 1) {
      vo &= 0x0F;
      vo |= (src << 4);
    } else {
      vo &= 0xF0;
      vo |= src;
    }
    g_eeGeneral.switchSources[xsw >> 1] = vo;
    xsw += DSW_IDL;       // skip IDL switch
    mapSwitch(xsw);
    if (src == SSW_NONE)
      unmapSwitch(xsw);   // uninstall this switch
    setMaxSwitchIndex();
  }
}

uint8_t getSwitchSource( uint8_t xsw )
{
  uint8_t src = g_eeGeneral.switchSources[xsw >> 1];
  return ((xsw & 1) ? (src >> 4) : (src & 0x0F));
}

void unsetSwitchSource( uint8_t src )
{
  for (uint8_t i = 0; i < MAX_XSWITCH; i++)
	{
    uint8_t s = getSwitchSource(i);
    if (s == src)
		{
      setSwitchSource(i, SSW_NONE);
		}
  }
}

//bool is3PosSwitch( uint8_t psw )
//{
//  return (psw >= PSW_BASE && psw <= PSW_3POS_END
//          && (psw == SW_IDL || getSwitchSource(psw - XSW_BASE) != SSW_NONE));
//}

// returns ST_UP=0/ST_MID=1/ST_DN=2 or ST_NC=3
uint8_t switchState( uint8_t psw )
{
  //assert(psw >= SW_IDL && psw <= SW_Trainer);
  uint8_t ping = PING ;
  uint8_t pine = PINE ;
  if (psw == SW_IDL) {
    //    INP_G_ID1 INP_E_ID2
    // id0    0        1
    // id1    1        1
    // id2    1        0
    if ((pine & (1<<INP_E_ID2)) == 0)
      return ST_DN;   // ID2
    if (ping & (1<<INP_G_ID1))
      return ST_MID;  // ID1
    return ST_UP;     // ID0
  }
  if (psw == SW_Trainer) {
    if (pine & (1<<INP_E_Trainer))
      return ST_DN;
    return ST_UP;
  }
  uint8_t xss = ST_NC;
  uint8_t swsrc = getSwitchSource(psw - XSW_BASE);
  if (swsrc != SSW_NONE) {
    switch (swsrc) {
      case SSW_PB7:
        xss = PINB & (1 << 7); break;
      case SSW_PC0:
        xss = PINC & (1 << 0); break;
      case SSW_PC4:
        xss = PINC & (1 << 4); break;
#if (!defined(CPUM64) || \
     !(defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA)))
      case SSW_PC6:
        xss = PINC & (1 << 6); break;
      case SSW_PC7:
        xss = PINC & (1 << 7); break;
#endif
      case SSW_PG2:
        xss = PING & (1 << 2); break;
#ifdef CPUM2561
      case SSW_PG5:
        xss = PING & (1 << 5); break;
#endif
      default:
        xss = ~Voice.VoiceSerialValue & (1 << (swsrc - SSW_XBASE));
        break;
    }
    if (xss == 0)   // extra switch is turned on (down position) - don't look further
      return ST_DN;
    xss = ST_UP;    // actually MID position (pulled-up state)
  }

  uint8_t xxx = 0;
  switch (psw) {
    case SW_ThrCt:
#if defined(CPUM128) || defined(CPUM2561)
      if ( g_eeGeneral.FrskyPins ) {
        xxx = PINC & (1<<INP_C_ThrCt) ;
      } else {
        xxx = pine & (1<<INP_E_ThrCt) ;
      }
#elif (!(defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA)))
      xxx = pine & (1<<INP_E_ThrCt);
#else
      xxx = PINC & (1<<INP_C_ThrCt); //shad974: rerouted inputs to free up UART0
#endif
      break ;
    case SW_RuddDR:
      xxx = ping & (1<<INP_G_RuddDR);
      break ;
    case SW_ElevDR:
      xxx = pine & (1<<INP_E_ElevDR);
      break;
    case SW_AileDR:
      xxx = readAilIp() ;
      break ;
    case SW_Gear:
      xxx = pine & (1<<INP_E_Gear);
      break ;
    default:
      // must be either SW_PB1 or SW_PB2
      return xss;   // returns either ST_UP or ST_NC
  }
  if (xxx == 0)
    return ST_UP;
  if (xss == ST_UP) // xxx != 0 && extra switch source turned off
    return ST_MID;
  return ST_DN;     // xxx != 0 && xss == ST_NC
}

uint8_t keyState(EnumKeys enuk)
{
  if (enuk < (int)DIM(keys))
    return keys[enuk].state() ? ST_DN : ST_UP;
  return switchState((uint8_t)enuk);
}
//
// conversion bewteen drswitch index and switch selection menu index
//  ---,ID0,ID1,ID2,THR|(TH^,TH-,THv),RUD|(RU^,RU-,RUv),ELE|(EL^,EL-,ELv),
//  AIL|(AI^,AI-,AIv),GEA|(GE^,GE-,GEv),[PB1,][PB2,]TRN,L1,..,LA,..,LC,
//  [LD,..,LI,]MAX_DRSWITCH,MAX_DRSWITCH+1(?)
//
// switch index to menu index
int8_t switchUnMap( int8_t drswitch )
{
#ifdef SAVE_RAM
  if (drswitch == 0)
    return 0;
#endif
  int8_t mIndex = drswitch;
  if (drswitch < 0)
    mIndex = -drswitch;
  // mIndex must be [DSW_THR..SW_3POS_END]
  if (mIndex == MAX_DRSWITCH) {
    mIndex = MaxSwitchIndex;
  } else if (mIndex >= DSW_TRN && mIndex < DSW_ID0) {
    mIndex -= DSW_PB2;    // DSW_TRN switch becomes 1
    mIndex += SW_MAPPED;  // skip over mapped switches
  } else {  // must be mapped between [1..SW_MAPPED]
#ifdef SAVE_RAM
    int8_t mi = 1;                      // menu index after "---"
    int8_t i3 = DSW_ID0;
    for (uint8_t i2 = DSW_IDL; i2 <= DSW_PB2; i2++, i3 += 3) {
      if (is3PosSwitch(i2)) {
        if (i3 == mIndex) break;        // ID0/TH^/RU^/EL^/AI^/GE^
        mi++;
        if ((i3 + 1) == mIndex) break;  // ID1/TH-/RU-/EL-/AI-/GE-
        mi++;
        if ((i3 + 2) == mIndex) break;  // ID2/THv/RUv/ELv/AIv/GEv
        mi++;
      } else {
        if (i2 == mIndex) break;        // (IDL)/THR/RUD/ELE/AIL/GEA/PB1/PB2
        mi++;
      }
    }
    mIndex = mi;
#else
    int8_t *pa = switchMapTable;
    int8_t *pz = pa + SW_MAPPED + 1;
    while (pa < pz) {
      if (*pa == mIndex)
        break;
      pa++;
    }
    mIndex = (pa - switchMapTable);
#endif  // SAVE_RAM
  }
  if (drswitch < 0)
    mIndex = -mIndex;
  return mIndex;
}

// menu index to switch index
int8_t switchMap( int8_t mIndex )
{
#ifdef SAVE_RAM
  if (mIndex == 0)
    return 0;
#endif
  int8_t drswitch = mIndex;
  if (mIndex < 0)
    drswitch = -mIndex;
  if (drswitch == MaxSwitchIndex) {
    drswitch = MAX_DRSWITCH;
  } else {
    int8_t mappedSw = SW_MAPPED;
    if (drswitch > mappedSw) {
      drswitch -= mappedSw;
      drswitch += DSW_PB2;      // Trainer switch becomes DSW_TRN
    } else {
#ifdef SAVE_RAM
      int8_t mi = drswitch;     // == |mIndex|
      int8_t i3 = DSW_ID0;
      for (int8_t i2 = DSW_IDL; i2 <= DSW_GEA; i2++, i3 += 3) {
        if (qSwitchMapped(i2 - DSW_IDL)) {   // 3 pos switch ?
          drswitch = i3;
          if (--mi == 0) break;
          drswitch++;
          if (--mi == 0) break;
          drswitch++;
          if (--mi == 0) break;
        } else {
          drswitch = i2;
          if (--mi == 0) break;
        }
      }
      if (mi > 0) {   // not a 3 pos sw, it must be either PB1 or PB2
        if (qSwitchMapped(DSW_PB1 - 1) && --mi == 0)
          drswitch = DSW_PB1;
        else /*if (mi > 0 && qSwitchMapped(DSW_PB2 - 1) && --mi == 0)*/
          drswitch = DSW_PB2;
      }
#else
      drswitch = switchMapTable[drswitch];
#endif
    }
  }
  if (mIndex < 0)
    drswitch = -drswitch;
  return drswitch;
}

#else	// !XSW_MOD

#ifdef SWITCH_MAPPING
uint8_t ExtraInputs ;

uint8_t hwKeyState( uint8_t key )
{
  CPU_UINT xxx = 0 ;
  if( key > HSW_MAX )  return 0 ;

	if ( ( key >= HSW_ThrCt ) && ( key <= HSW_Trainer ) )
	{
		return keyState( (EnumKeys)(key + ( SW_ThrCt - HSW_ThrCt ) ) ) ;
	}

  CPU_UINT yyy = key - HSW_Ele3pos0 ;
  CPU_UINT zzz ;
	
	if ( yyy <= 2 )	// 31-33
	{
		xxx = ~PINE & (1<<INP_E_ElevDR) ;
		if ( yyy )
		{
			zzz = (g_eeGeneral.ele2source-1) ;
			zzz = ExtraInputs & (1<<zzz) ; 
			if ( yyy == 2 )
			{
				xxx = zzz ;
			}
			else
			{
				xxx |= zzz ;
				xxx = !xxx ;
			}
		}
	}
	
	yyy -= 3 ;
	if ( yyy <= 2 )	// 34-36
	{
    xxx = ~PING & (1<<INP_G_RuddDR);
		if ( yyy )
		{
			zzz = (g_eeGeneral.rud2source-1) ;
			zzz = ExtraInputs & (1<<zzz) ; 
			if ( yyy == 2 )
			{
				xxx = zzz ;
			}
			else
			{
				xxx |= zzz ;
				xxx = !xxx ;
			}
		}
	}
	
	yyy -= 3 ;
	if ( yyy <= 2 )	// 37-39
	{
		xxx = !readAilIp() ;
		if ( yyy )
		{
			zzz = (g_eeGeneral.ail2source-1) ;
			zzz = ExtraInputs & (1<<zzz) ; 
			if ( yyy == 2 )
			{
				xxx = zzz ;
			}
			else
			{
				xxx |= zzz ;
				xxx = !xxx ;
			}
		}
	}
	yyy -= 3 ;
	if ( yyy <= 2 )	// 40-42
	{
		xxx = ~PINE & (1<<INP_E_Gear) ;
		if ( yyy )
		{
			zzz = (g_eeGeneral.gea2source-1) ;
			zzz = ExtraInputs & (1<<zzz) ; 
			if ( yyy == 2 )
			{
				xxx = zzz ;
			}
			else
			{
				xxx |= zzz ;
				xxx = !xxx ;
			}
		}
	}
	if ( yyy == 3 )	// 43
	{
		xxx = ExtraInputs & (1<<(g_eeGeneral.pb1source-1)) ;
	}
	if ( yyy == 4 )	// 44
	{
		xxx = ExtraInputs & (1<<(g_eeGeneral.pb2source-1)) ;
	}
	
  if ( xxx )
  {
    return 1 ;
  }
  return 0;
	
}

// Returns 0, 1 or 2 (or 3,4,5) for ^ - or v (or 6pos)
uint8_t switchPosition( uint8_t swtch )
{
	if ( hwKeyState( swtch ) )
	{
		return 0 ;
	}
	swtch += 1 ;
	if ( hwKeyState( swtch ) )
	{
		return 1 ;			
	}
//	if ( swtch == HSW_Ele6pos1 )
//	{
//		swtch += 2 ;
//		if ( hwKeyState( swtch ) )
//		{
//			return 3 ;
//		}
//		swtch += 1 ;
//		if ( hwKeyState( swtch ) )
//		{
//			return 4 ;
//		}
//		swtch += 1 ;
//		if ( hwKeyState( swtch ) )
//		{
//			return 5 ;
//		}
//	}
	return 2 ;
}

#else

// Returns 0, 1
uint8_t switchPosition( uint8_t swtch )
{
	return keyState( (EnumKeys)swtch ) ? 0 : 1;
}
#endif

bool keyState(EnumKeys enuk)
{
  uint8_t xxx = 0 ;
	uint8_t ping = PING ;
	uint8_t pine = PINE ;
  if(enuk < (int)DIM(keys))  return keys[enuk].state() ? 1 : 0;

  switch((uint8_t)enuk){
    case SW_ElevDR : xxx = pine & (1<<INP_E_ElevDR);
    break ;

    case SW_AileDR :
			xxx = readAilIp() ;
    break ;

    case SW_RuddDR : xxx = ping & (1<<INP_G_RuddDR);
    break ;
      //     INP_G_ID1 INP_E_ID2
      // id0    0        1
      // id1    1        1
      // id2    1        0
    case SW_ID0    : xxx = ~ping & (1<<INP_G_ID1);
    break ;
    case SW_ID1    : xxx = (ping & (1<<INP_G_ID1)) ; if ( xxx ) xxx = (PINE & (1<<INP_E_ID2));
    break ;
    case SW_ID2    : xxx = ~pine & (1<<INP_E_ID2);
    break ;
    case SW_Gear   : xxx = pine & (1<<INP_E_Gear);
    break ;
    //case SW_ThrCt  : return PINE & (1<<INP_E_ThrCt);

#if defined(CPUM128) || defined(CPUM2561)
    case SW_ThrCt :
			if ( g_eeGeneral.FrskyPins )
			{
				xxx = PINC & (1<<INP_C_ThrCt) ;
			}
			else
			{
				xxx = pine & (1<<INP_E_ThrCt) ;
			}
#else
 #if (!(defined(JETI) || defined(FRSKY) || defined(ARDUPILOT) || defined(NMEA)))
     case SW_ThrCt  : xxx = pine & (1<<INP_E_ThrCt);
 #else
    case SW_ThrCt  : xxx = PINC & (1<<INP_C_ThrCt); //shad974: rerouted inputs to free up UART0
 #endif
#endif
		break ;

    case SW_Trainer: xxx = pine & (1<<INP_E_Trainer);
    break ;
    default:;
  }
  if ( xxx )
  {
    return 1 ;
  }
  return 0;
}
#endif  // XSW_MOD

void pauseEvents(uint8_t event)
{
  event=event & EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].pauseEvents();
}
void killEvents(uint8_t event)
{
  event=event & EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].killEvents();
}

//uint8_t getEventDbl(uint8_t event)
//{
//  event=event & EVT_KEY_MASK;
//  if(event < (int)DIM(keys))  return keys[event].getDbl();
//  return 0;
//}

//uint16_t g_anaIns[8];
volatile uint16_t g_tmr10ms;
//volatile uint8_t g8_tmr10ms ;
volatile uint8_t  g_blinkTmr10ms;
extern uint8_t StickScrollTimer ;


void per10ms()
{
	uint16_t tmr ;
//  g_tmr10ms++;				// 16 bit sized
//	g8_tmr10ms += 1 ;		// byte sized
//  g_blinkTmr10ms++;
  tmr = g_tmr10ms + 1 ;
	g_tmr10ms = tmr ;
	g_blinkTmr10ms = tmr ;
  uint8_t enuk = KEY_MENU;
  uint8_t    in = ~PINB;
	
	static uint8_t current ;
	uint8_t dir_keys ;
	uint8_t lcurrent ;

#ifdef GREEN_CHIP
extern uint8_t WdogTimer ;
	if ( WdogTimer )
	{
		if ( --WdogTimer )
		{
     	wdt_reset();
		}
	}
#endif

	dir_keys = in & 0x78 ;		// Mask to direction keys
	if ( ( lcurrent = current ) )
	{ // Something already pressed
		if ( ( lcurrent & dir_keys ) == 0 )
		{
			lcurrent = 0 ;	// No longer pressed
		}
		else
		{
			in &= lcurrent | 0x06 ;	// current or MENU or EXIT allowed
		}
	}
	if ( lcurrent == 0 )
	{ // look for a key
		if ( dir_keys & 0x20 )	// right
		{
			lcurrent = 0x60 ;		// Allow L and R for 9X
		}
		else if ( dir_keys & 0x40 )	// left
		{
			lcurrent = 0x60 ;		// Allow L and R for 9X
		}
		else if ( dir_keys & 0x08 )	// down
		{
			lcurrent = 0x08 ;
		}
		else if ( dir_keys & 0x10 )	// up
		{
			lcurrent = 0x10 ;
		}
		in &= lcurrent | 0x06 ;	// current or MENU or EXIT allowed
	}
	current = lcurrent ;

  for(uint8_t i=1; i<7; i++)
  {
    //INP_B_KEY_MEN 1  .. INP_B_KEY_LFT 6
    keys[enuk].input(in & 2,(EnumKeys)enuk);
    ++enuk;
		in >>= 1 ;
  }

  const static  prog_uchar  APM crossTrim[]={
    1<<INP_D_TRM_LH_DWN,
    1<<INP_D_TRM_LH_UP,
    1<<INP_D_TRM_LV_DWN,
    1<<INP_D_TRM_LV_UP,
    1<<INP_D_TRM_RV_DWN,
    1<<INP_D_TRM_RV_UP,
    1<<INP_D_TRM_RH_DWN,
    1<<INP_D_TRM_RH_UP
  };
  
	if ( Backup_RestoreRunning )
	{
		in = 0 ;
	}
	else
	{
		in = ~PIND ;
#ifdef XSW_MOD
 #if defined(CPUM128) || defined(CPUM2561)
    if ( g_eeGeneral.LVTrimMod ) {
      in &= 0xF3;
      if ((PINC & (1 << 4)) == 0) // PC4
        in |= (1<<INP_D_TRM_LV_DWN);
      if ((PINC & (1 << 0)) == 0) // PC0
        in |= (1<<INP_D_TRM_LV_UP);
    }
  #ifdef SERIAL_VOICE
    else
  #endif
 #endif
 #ifdef SERIAL_VOICE
	#ifndef SERIAL_VOICE_ONLY
		if ( g_eeGeneral.MegasoundSerial )
  #endif
		{
			in &= 0xF3 ;
			in |= (Voice.VoiceSerialValue) & 0x0C ;  // get XPD4/XPD3
//			in |= (Voice.VoiceSerialValue >> 2) & 0x0C ;  // get XPD4/XPD3
		} // NOTE: bit order of VoiceSerialValue changed
 #endif
 
#else // !XSW_MOD
#ifdef SERIAL_VOICE
#ifndef SERIAL_VOICE_ONLY
		if ( g_eeGeneral.MegasoundSerial )
#endif
		{
			in &= 0xF3 ;
			in |= Voice.VoiceSerialValue & 0x0C ;
		}
#endif
#endif  // XSW_MOD
	}

	for(int i=0; i<8; i++)
  {
    // INP_D_TRM_RH_UP   0 .. INP_D_TRM_LH_UP   7
    keys[enuk].input(in & pgm_read_byte(crossTrim+i),(EnumKeys)enuk);
    ++enuk;
  }
	
	uint8_t value = Rotary.RotEncoder & 0x20 ;
	keys[enuk].input( value,(EnumKeys)enuk); // Rotary Enc. Switch
	
	in = ~PINB & 0x7E ;
	value |= in ;
	if ( value )
	{
		StickScrollTimer = STICK_SCROLL_TIMEOUT ;
	}

#ifdef SWITCH_MAPPING
// Read all "extra" inputs to ExtraInputs
// Bit0											 BIT6
// EXT1 EXT2 PC0 PG2 PB7 PG5 L-WR
	value = Voice.VoiceSerialValue & 0x41 ;
	if ( value & 0x40 )
	{
		value |= 2 ;
		value &= 3 ;
	}
	if ( ( PING & 4 ) == 0 )	// PG2
	{
		value |= 8 ;
	}
	if ( ( PING & 0x20 ) == 0 )	// PG5
	{
		value |= 0x20 ;
	}
	if ( ( PINC & 1 ) == 0 )	// PC0
	{
		value |= 4 ;
	}
	if ( ( PINC & 0x10 ) == 0 )	// PC4 (LCD_WR)
	{
		value |= 0x40 ;
	}
	if ( ( PINB & 0x80 ) == 0 )	// PB7
	{
		value |= 0x10 ;
	}
	ExtraInputs = value ;
#endif  // SWITCH_MAPPING
}


#ifndef SIMU

void serialVoiceInit()
{
#undef BAUD
#define BAUD 38400
#include <util/setbaud.h>

//	DDRD |= 0x08 ;
	PORTD |= 0x04 ;		// Pullup on RXD1

  UBRR1H = UBRRH_VALUE;
  UBRR1L = UBRRL_VALUE;
  UCSR1A &= ~(1 << U2X1); // disable double speed operation.

  // set 8 N1
  UCSR1C = 0 | (1 << UCSZ11) | (1 << UCSZ10);
  
  while (UCSR1A & (1 << RXC1)) UDR1; // flush receive buffer
  
  UCSR1B = (1 << RXCIE1) | (0 << TXCIE1) | (0 << UDRIE1) | (1 << RXEN1) | (1 << TXEN1) | (0 << UCSZ12) ;
//  UCSR1B |= (1 << TXEN1) | (1 << RXEN1) ; // enable TX & Rx
//	UCSR1B |= (1 << RXCIE1); // enable Interrupt
}

void startSerialVoice()
{
	PausePulses = 1 ;
	Backup_RestoreRunning = 1 ;

#ifndef SERIAL_VOICE_ONLY
	if ( g_eeGeneral.MegasoundSerial == 0 )
#endif
	{
		serialVoiceInit() ;
	}

}

void stopSerialVoice()
{
//	DDRD &= ~0x08 ;
#ifdef SERIAL_VOICE
#ifndef SERIAL_VOICE_ONLY
	if ( g_eeGeneral.MegasoundSerial == 0 )
#endif
	{
		UCSR1B = 0 ; // disable Interrupt, TX, RX
	}
#else	
#ifndef SERIAL_VOICE
	UCSR1B = 0 ; // disable Interrupt, TX, RX
#endif
#endif
	
	Backup_RestoreRunning = 0 ;
	startPulses() ;
}

void serialVoiceTx( uint8_t byte )
{
	while ( ( UCSR1A & ( 1 << UDRE1 ) ) == 0 )
		/*wait*/ ;
	UDR1 = byte ;
}

struct t_fifo16
{
	uint8_t fifo[16] ;
	uint8_t in ;
	uint8_t out ;
} SvFifo ;

int16_t getSvFifo()
{
	struct t_fifo16 *pfifo = &SvFifo ;
	FORCE_INDIRECT( pfifo ) ;

	int16_t rxbyte ;
	if ( pfifo->in != pfifo->out )				// Look for char available
	{
		rxbyte = pfifo->fifo[pfifo->out] ;
		pfifo->out = ( pfifo->out + 1 ) & 0x0F ;
		return rxbyte ;
	}
	return -1 ;
}

//uint16_t SerialVoiceDebug ;

ISR(USART1_RX_vect)
{
	UCSR1B &= ~(1 << RXCIE1); // disable Interrupt
	sei() ;
//SerialVoiceDebug += 1 ;
	struct t_fifo16 *pfifo = &SvFifo ;
  uint8_t next = (pfifo->in + 1) & 0x0f ;
	uint8_t data = UDR1 ;
	if ( next != pfifo->out )
	{
		pfifo->fifo[pfifo->in] = data ;
		pfifo->in = next ;
	}
	cli() ;
  UCSR1B |= (1 << RXCIE1); // enable Interrupt
}

#endif


