
#include <string.h>

#include "ersky9x.h"
#include "lcd.h"
#include "myeeprom.h"
#include "drivers.h"

#include "audio.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lrotable.h"

#include "lapi.h"
}

#include "lua_api.h"

#include "stringidx.h"
#include <string.h>

void luaLoadModelScripts()
{
	luaState |= INTERPRETER_RELOAD_PERMANENT_SCRIPTS ;
}

void luaGetValueAndPush(lua_State* L, int src)
{
//  getvalue_t value = getValue(src); // ignored for GPS, DATETIME, and CELLS
  int32_t value = getValue(src) ; // ignored for GPS, DATETIME, and CELLS

//  if (src >= MIXSRC_FIRST_TELEM && src <= MIXSRC_LAST_TELEM) {
//    div_t qr = div(src-MIXSRC_FIRST_TELEM, 3);
//    // telemetry values
//    if (TELEMETRY_STREAMING() && telemetryItems[qr.quot].isAvailable()) {
//      TelemetrySensor & telemetrySensor = g_model.telemetrySensors[qr.quot];
//      switch (telemetrySensor.unit) {
//        case UNIT_GPS:
//          luaPushLatLon(L, telemetrySensor, telemetryItems[qr.quot]);
//          break;
//        case UNIT_DATETIME:
//          luaPushTelemetryDateTime(L, telemetrySensor, telemetryItems[qr.quot]);
//          break;
//        case UNIT_CELLS:
//          if (qr.rem == 0) {
//            luaPushCells(L, telemetrySensor, telemetryItems[qr.quot]);
//            break;
//          }
//          // deliberate no break here to properly return `Cels-` and `Cels+`
//        default:
//          if (telemetrySensor.prec > 0)
//            lua_pushnumber(L, float(value)/telemetrySensor.getPrecDivisor());
//          else
//            lua_pushinteger(L, value);
//          break;
//      }
//    }
//    else {
//      // telemetry not working, return zero for telemetry sources
//      lua_pushinteger(L, (int)0);
//    }
//  }
//  else if (src == MIXSRC_TX_VOLTAGE) {
//    lua_pushnumber(L, float(value) * 0.1f);
//  }
//  else {
    lua_pushinteger(L, value);
//  }
}

//0-6 sticks/pots
//7-11 up to PPM_BASE nothing
//12-19 PPM_BASE-PPMBASE+7 trainer inputs
//20-43 channels 1-24
//44-117 telemetry
//120-124 Extra Pots
//180-187 Extra PPM
//200-207 Extra channels

uint32_t strMatch( const char *a, const char *b, uint32_t length )
{
	if ( length == 0 )
	{
		return 1 ;
	}
	while ( length )
	{
		if ( *a == 0 || *b == 0 )
		{
			return 1 ;
		}
		if ( *a++ == *b++ )
		{
			if ( --length == 0 )
			{
				return 1 ;
			}
		}
		else
		{
			break ;
		}
	}
	return 0 ;
}


int32_t luaFindValueIndexByName( const char * name )
{
	uint32_t i ;
	const char *names = PSTR(STR_TELEM_ITEMS) ;
	uint32_t nameLength = *names++ ;

	for ( i = 0 ; i < NUM_TELEM_ITEMS ;  i += 1 )
	{
    if (strMatch( names, name, nameLength ))
		{
			return i + 44 - 1 ;
		}
		names += nameLength ;
	}

	names = PSTR(STR_STICK_NAMES) ;
	nameLength = *names++ ;
	for ( i = 0 ; i < 4 ;  i += 1 )
	{
    if (strMatch( names, name, 3 ))
		{
			return i ;
		}
		names += nameLength ;
	}
  
	names = PSTR(STR_CHANS_RAW) ;
	nameLength = *names++ ;
	for ( i = 4 ; i < 7 ;  i += 1 )
	{
    if (strMatch( names, name, 2 ))
		{
			return i ;
		}
		names += nameLength ;
	}
	names += nameLength * 5 ;
	for ( i = 12 ; i < 44 ;  i += 1 )
	{
    if (strMatch( names, name, nameLength ))
		{
			return i ;
		}
		names += nameLength ;
	}
//	for ( i = 20 ; i < 29 ;  i += 1 )
//	{
//    if (strMatch( names, name, 3 ))
//		{
//			return i ;
//		}
//		names += nameLength ;
//	}
//	for ( i = 29 ; i < 44 ;  i += 1 )
//	{
//    if (strMatch( names, name, nameLength ))
//		{
//			return i ;
//		}
//		names += nameLength ;
//	}

	
	// TODO better search method (binary lookup)
//  for (unsigned int n=0; n<DIM(luaSingleFields); ++n)
//	{
//    if (!strcmp(name, luaSingleFields[n].name))
//		{
//      field.id = luaSingleFields[n].id;
//      if (flags & FIND_FIELD_DESC)
//			{
//        strncpy(field.desc, luaSingleFields[n].desc, sizeof(field.desc)-1);
//        field.desc[sizeof(field.desc)-1] = '\0';
//      }
//      else {
//        field.desc[0] = '\0';
//      }
//      return true;
//    }
//  }

//  // search in multiples
//  unsigned int len = strlen(name);
//  for (unsigned int n=0; n<DIM(luaMultipleFields); ++n) {
//    const char * fieldName = luaMultipleFields[n].name;
//    unsigned int fieldLen = strlen(fieldName);
//    if (!strncmp(name, fieldName, fieldLen)) {
//      unsigned int index;
//      if (len == fieldLen+1 && isdigit(name[fieldLen])) {
//        index = name[fieldLen] - '1';
//      }
//      else if (len == fieldLen+2 && isdigit(name[fieldLen]) && isdigit(name[fieldLen+1])) {
//        index = 10 * (name[fieldLen] - '0') + (name[fieldLen+1] - '1');
//      }
//      else {
//        continue;
//      }
//      if (index < luaMultipleFields[n].count) {
//        field.id = luaMultipleFields[n].id + index;
//        if (flags & FIND_FIELD_DESC) {
//          snprintf(field.desc, sizeof(field.desc)-1, luaMultipleFields[n].desc, index+1);
//          field.desc[sizeof(field.desc)-1] = '\0';
//        }
//        else {
//          field.desc[0] = '\0';
//        }
//        return true;
//      }
//    }
//  }

//  // search in telemetry
//  field.desc[0] = '\0';
//  for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
//    if (isTelemetryFieldAvailable(i)) {
//      char sensorName[TELEM_LABEL_LEN+1];
//      int len = zchar2str(sensorName, g_model.telemetrySensors[i].label, TELEM_LABEL_LEN);
//      if (!strncmp(sensorName, name, len)) {
//        if (name[len] == '\0') {
//          field.id = MIXSRC_FIRST_TELEM + 3*i;
//          field.desc[0] = '\0';
//          return true;
//        }
//        else if (name[len] == '-' && name[len+1] == '\0') {
//          field.id = MIXSRC_FIRST_TELEM + 3*i + 1;
//          field.desc[0] = '\0';
//          return true;
//        }
//        else if (name[len] == '+' && name[len+1] == '\0') {
//          field.id = MIXSRC_FIRST_TELEM + 3*i + 2;
//          field.desc[0] = '\0';
//          return true;
//        }
//      }
//    }
//  }

  return -1 ;  // not found
}

static int luaGetValue(lua_State * L)
{
  int src = 0;
  if (lua_isnumber(L, 1))
	{
    src = luaL_checkinteger(L, 1) ;
  }
  else
	{
    const char *name = luaL_checkstring(L, 1) ;
		src = luaFindValueIndexByName( name ) ;
		if ( src == -1 )
		{
			return 0 ;
		}
    // convert from field name to its id
//    const char *name = luaL_checkstring(L, 1);
//    LuaField field;
//    bool found = luaFindFieldByName(name, field);
//    if (found) {
//      src = field.id;
//    }
  }
  luaGetValueAndPush(L, src);
  return 1 ;
}

/*luadoc
@function playNumber(value, unit [, attributes])

Play a numerical value (text to speech)

@param value (number) number to play. Value is interpreted as integer.

(Not used) @param unit (number) unit identifier [Full list]((../appendix/units.html))

@param attributes (unsigned number) possible values:
0 * `0 or not present` plays integral part of the number (for a number 123 it plays 123)
1 * `PREC1` plays a number with one decimal place (for a number 123 it plays 12.3)
2 * `PREC2` plays a number with two decimal places (for a number 123 it plays 1.23)

@status current Introduced in 2.0.0

*/

extern uint16_t getUnitsVoice( uint16_t unit ) ;

static int luaPlayNumber(lua_State * L)
{
  int number = luaL_checkinteger(L, 1);
  int unit = luaL_checkinteger(L, 2);
  unsigned int att = luaL_optunsigned(L, 3, 0);
	if ( unit > 0 && unit < 10 )
	{
		unit = getUnitsVoice( unit ) ;
	}
	else
	{
		unit = 0 ;
	}
	voice_numeric( number, att, unit ) ;
//  playNumber(number, unit, att, 0);
  return 0;
}

/*luadoc
@function sportTelemetryPush()

This functions allows for sending SPORT telemetry data toward the receiver,
and more generally, to anything connected SPORT bus on the receiver or transmitter.

When called without parameters, it will only return the status of the ouput buffer without sending anything.

@param sensorId  physical sensor ID

@param frameId   frame ID

@param dataId    data ID

@param value     value

@retval boolean  data queued in output buffer or not.

@status current Introduced in 2.2.0
*/

extern uint32_t sportPacketSend( uint8_t *pdata, uint8_t index ) ;
//extern const uint8_t SportIds[28] ;
const uint8_t SportIds[28] = {0x00, 0xA1, 0x22, 0x83, 0xE4, 0x45, 0xC6, 0x67,
				                      0x48, 0xE9, 0x6A, 0xCB, 0xAC, 0x0D, 0x8E, 0x2F,
															0xD0, 0x71, 0xF2, 0x53, 0x34, 0x95, 0x16, 0xB7,
															0x98, 0x39, 0xBA, 0x1B/*, 0x7C, 0xDD, 0x5E, 0xFF*/ } ;

static int luaSportTelemetryPush(lua_State * L)
{
	static uint8_t sportPacket[8] ;
	
  if (lua_gettop(L) == 0)
	{
    lua_pushboolean(L, sportPacketSend( 0, 0 ) ) ;
  }
  else
	{
		uint32_t value ;
//    sportPacket[7] = getDataId(luaL_checkunsigned(L, 1)) ;
		sportPacket[7] = SportIds[luaL_checkunsigned(L, 1)] ;
    sportPacket[0] = luaL_checkunsigned(L, 2) ;
    value = luaL_checkunsigned(L, 3) ;
		sportPacket[1] = value ;
		sportPacket[2] = value >> 8 ;
    value = luaL_checkunsigned(L, 4) ;
		sportPacket[3] = value ;
		sportPacket[4] = value >> 8 ;
		sportPacket[5] = value >> 16 ;
		sportPacket[6] = value >> 24 ;
		value = sportPacketSend( sportPacket, sportPacket[7] ) ;
		lua_pushboolean(L, value ? true : false ) ;
  }
  return 1 ;
}

/*luadoc
@function sportTelemetryPop()

Pops a received SPORT packet from the queue. Please note that only packets using a data ID within 0x5000 to 0x52FF
(frame ID == 0x10), as well as packets with a frame ID equal 0x32 (regardless of the data ID) will be passed to
the LUA telemetry receive queue.

@retval SPORT paket as a quadruple:
 * sensor ID (number)
 * frame ID (number)
 * data ID (number)
 * value (number)

@status current Introduced in 2.2.0
*/

uint32_t PushedValue ;

static int luaSportTelemetryPop(lua_State * L)
{
//  if (!luaInputTelemetryFifo)
//	{
//    luaInputTelemetryFifo = new Fifo<uint8_t, LUA_TELEMETRY_INPUT_FIFO_SIZE>();
//    if (!luaInputTelemetryFifo)
//		{
//      return 0;
//    }
//  }

	if ( fifo128Space( &Lua_fifo ) <= ( 128 - 8 ) )
	{
		uint32_t value ;
		value = get_fifo128( &Lua_fifo ) & 0x1F ;
    lua_pushnumber(L, value ) ;	// physical ID
		value = get_fifo128( &Lua_fifo ) ;
    lua_pushnumber(L, value ) ;	// prim
		value = get_fifo128( &Lua_fifo ) ;
		value |= get_fifo128( &Lua_fifo ) << 8 ;
    lua_pushnumber(L, value ) ;	// App ID
		value = get_fifo128( &Lua_fifo ) ;
		value |= get_fifo128( &Lua_fifo ) << 8 ;
		value |= get_fifo128( &Lua_fifo ) << 16 ;
		value |= get_fifo128( &Lua_fifo ) << 24 ;
    lua_pushunsigned(L, value ) ;	// Data
    return 4 ;
	}
//  if (luaInputTelemetryFifo->size() >= sizeof(SportTelemetryPacket))
//	{
//    SportTelemetryPacket packet;
//    for (uint8_t i=0; i<sizeof(packet); i++)
//		{
//      luaInputTelemetryFifo->pop(packet.raw[i]);
//    }
//    lua_pushnumber(L, packet.physicalId);
//    lua_pushnumber(L, packet.primId);
//    lua_pushnumber(L, packet.dataId);
//    lua_pushunsigned(L, packet.value);
//    return 4;
//  }

  return 0;
}

static int luaGetTime(lua_State *L)
{
  lua_pushunsigned(L, get_ltmr10ms()) ;
  return 1 ;
}

static void luaPushDateTime(lua_State *L )	//, uint32_t year, uint32_t mon, uint32_t day,
//                            uint32_t hour, uint32_t min, uint32_t sec)
{
  lua_createtable( L, 0, 6 ) ;
  lua_pushtableinteger( L, "year", Time.year ) ;
  lua_pushtableinteger( L, "mon", Time.month ) ;
  lua_pushtableinteger( L, "day", Time.date ) ;
  lua_pushtableinteger( L, "hour", Time.hour ) ;
  lua_pushtableinteger( L, "min", Time.minute ) ;
  lua_pushtableinteger( L, "sec", Time.second ) ;
}

static int luaGetDateTime(lua_State *L)
{
//  struct gtm utm;
//  gettime(&utm);
//  luaPushDateTime(L, utm.tm_year + 1900, utm.tm_mon + 1, utm.tm_mday, utm.tm_hour, utm.tm_min, utm.tm_sec);
  luaPushDateTime( L ) ;	//, utm.tm_year + 1900, utm.tm_mon + 1, utm.tm_mday, utm.tm_hour, utm.tm_min, utm.tm_sec);
  return 1 ;
}

extern uint32_t IdlePercent ;

static int luaIdleTime(lua_State *L)
{
  lua_pushnumber(L, IdlePercent ) ;	// prim
	return 1 ;
}


/*luadoc
@function loadScript(file [, mode], [,env])

Load a Lua script file. This is similar to Lua's own [loadfile()](https://www.lua.org/manual/5.2/manual.html#pdf-loadfile)
API method,  but it uses OpenTx's optional pre-compilation feature to save memory and time during load.

Return values are same as from Lua API loadfile() method: If the script was loaded w/out errors
then the loaded script (or "chunk") is returned as a function. Otherwise, returns nil plus the error message.

@param file (string) Full path and file name of script. The file extension is optional and ignored (see `mode` param to control
  which extension will be used). However, if an extension is specified, it should be ".lua" (or ".luac"), otherwise it is treated
  as part of the file name and the .lua/.luac will be appended to that.

@param mode (string) (optional) Controls whether to force loading the text (.lua) or pre-compiled binary (.luac)
  version of the script. By default OTx will load the newest version and compile a new binary if necessary (overwriting any
  existing .luac version of the same script, and stripping some debug info like line numbers).
  You can use `mode` to control the loading behavior more specifically. Possible values are:
   * `b` only binary.
   * `t` only text.
   * `T` (default on simulator) prefer text but load binary if that is the only version available.
   * `bt` (default on radio) either binary or text, whichever is newer (binary preferred when timestamps are equal).
   * Add `x` to avoid automatic compilation of source file to .luac version.
       Eg: "tx", "bx", or "btx".
   * Add `c` to force compilation of source file to .luac version (even if existing version is newer than source file).
       Eg: "tc" or "btc" (forces "t", overrides "x").
   * Add `d` to keep extra debug info in the compiled binary.
       Eg: "td", "btd", or "tcd" (no effect with just "b" or with "x").

@notice
  Note that you will get an error if you specify `mode` as "b" or "t" and that specific version of the file does not exist (eg. no .luac file when "b" is used).
  Also note that `mode` is NOT passed on to Lua's loader function, so unlike with loadfile() the actual file content is not checked (as if no mode or "bt" were passed to loadfile()).

@param env (integer) See documentation for Lua function loadfile().

@retval function The loaded script, or `nil` if there was an error (e.g. file not found or syntax error).

@retval string Error message(s), if any. Blank if no error occurred.

@status current Introduced in 2.2.0

### Example

```lua
  fun, err = loadScript("/SCRIPTS/FUNCTIONS/print.lua")
  if (fun ~= nil) then
     fun("Hello from loadScript()")
  else
     print(err)
  end
```

*/
//static int luaLoadScript(lua_State * L)
//{
//  // this function is replicated pretty much verbatim from luaB_loadfile() and load_aux() in lbaselib.c
//  const char *fname = luaL_optstring(L, 1, NULL);
//  const char *mode = luaL_optstring(L, 2, NULL);
//  int env = (!lua_isnone(L, 3) ? 3 : 0);  // 'env' index or 0 if no 'env'
//  lua_settop(L, 0);
//  if (fname != NULL && luaLoadScriptFileToState(L, fname , mode) == SCRIPT_OK) {
//    if (env != 0) {  // 'env' parameter?
//      lua_pushvalue(L, env);  // environment for loaded function
//      if (!lua_setupvalue(L, -2, 1))  // set it as 1st upvalue
//        lua_pop(L, 1);  // remove 'env' if not used by previous call
//    }
//    return 1;
//  }
//  else {
//    // error (message should be on top of the stack)
//    if (!lua_isstring(L, -1)) {
//      // probably didn't find a file or had some other error before luaL_loadfile() was run
//      lua_pushfstring(L, "loadScript(\"%s\", \"%s\") error: File not found", (fname != NULL ? fname : "nul"), (mode != NULL ? mode : "bt"));
//    }
//    lua_pushnil(L);
//    lua_insert(L, -2);  // move nil before error message
//    return 2;  // return nil plus error message
//  }
//}


extern "C" int luaB_loadfile(lua_State *L) ;

static int luaLoadScript(lua_State *L)
{
	return luaB_loadfile(L) ;
}


const luaL_Reg ersky9xLib[] = {
  { "getTime", luaGetTime },
  { "getDateTime", luaGetDateTime },
//  { "getVersion", luaGetVersion },
//  { "getGeneralSettings", luaGetGeneralSettings },
  { "getValue", luaGetValue },
//  { "getFieldInfo", luaGetFieldInfo },
//  { "getFlightMode", luaGetFlightMode },
//  { "playFile", luaPlayFile },
  { "playNumber", luaPlayNumber },
//  { "playDuration", luaPlayDuration },
//  { "playTone", luaPlayTone },
//  { "playHaptic", luaPlayHaptic },
//  { "popupInput", luaPopupInput },
//  { "popupWarning", luaPopupWarning },
//  { "popupConfirmation", luaPopupConfirmation },
//  { "defaultStick", luaDefaultStick },
//  { "defaultChannel", luaDefaultChannel },
//  { "getRSSI", luaGetRSSI },
//  { "killEvents", luaKillEvents },
  { "loadScript", luaLoadScript },
//#if !defined(COLORLCD)
//  { "GREY", luaGrey },
//#endif
  { "sportTelemetryPop", luaSportTelemetryPop },
  { "sportTelemetryPush", luaSportTelemetryPush },
//  { "setTelemetryValue", luaSetTelemetryValue },
  { "idleTime", luaIdleTime },
  { NULL, NULL }  /* sentinel */
};


const luaR_value_entry ersky9xConstants[] = {
  { "FULLSCALE", RESX },
//  { "XXLSIZE", XXLSIZE },
  { "DBLSIZE", DBLSIZE },
//  { "MIDSIZE", MIDSIZE },
//  { "SMLSIZE", SMLSIZE },
  { "INVERS", INVERS },
//#if defined(PCBFLAMENCO)
//  { "WHITE",        WHITE },
//  { "BLACK",        BLACK },
//  { "YELLOW",       YELLOW },
//  { "BLUE",         BLUE },
//  { "GREY_DEFAULT", GREY_DEFAULT },
//  { "DARKGREY",     DARKGREY },
//  { "RED",          RED },
//  { "LIGHTGREY",    LIGHTGREY },
//  { "WHITE_ON_BLACK",       WHITE_ON_BLACK },
//  { "RED_ON_BLACK",         RED_ON_BLACK },
//  { "BLUE_ON_BLACK",        BLUE_ON_BLACK },
//  { "GREY_ON_BLACK",        GREY_ON_BLACK },
//  { "LIGHTGREY_ON_BLACK",   LIGHTGREY_ON_BLACK },
//  { "YELLOW_ON_BLACK",      YELLOW_ON_BLACK },
//  { "WHITE_ON_DARKGREY",    WHITE_ON_DARKGREY },
//  { "WHITE_ON_BLUE",        WHITE_ON_BLUE },
//  { "BLACK_ON_YELLOW",      BLACK_ON_YELLOW },
//  { "LIGHTGREY_ON_YELLOW",  LIGHTGREY_ON_YELLOW },
//#endif
//  { "BOLD", BOLD },
  { "BLINK", BLINK },
//  { "FIXEDWIDTH", FIXEDWIDTH },
  { "LEFT", LEFT },
  { "PREC1", PREC1 },
  { "PREC2", PREC2 },
//  { "VALUE", 0 },
//  { "SOURCE", 1 },
//  { "REPLACE", MLTPX_REP },
//  { "MIXSRC_FIRST_INPUT", MIXSRC_FIRST_INPUT },
//  { "MIXSRC_Rud", MIX_RUD },
//  { "MIXSRC_Ele", MIX_ELE },
//  { "MIXSRC_Thr", MIX_THR },
//  { "MIXSRC_Ail", MIX_AIL },
//  { "MIXSRC_SA", MIXSRC_SA },
//  { "MIXSRC_SB", MIXSRC_SB },
//  { "MIXSRC_SC", MIXSRC_SC },
//#if defined(PCBTARANIS)
//  { "MIXSRC_SD", MIXSRC_SD },
//#endif
//  { "MIXSRC_SE", MIXSRC_SE },
//  { "MIXSRC_SF", MIXSRC_SF },
//#if defined(PCBTARANIS)
//  { "MIXSRC_SG", MIXSRC_SG },
//  { "MIXSRC_SH", MIXSRC_SH },
//#endif
//  { "MIXSRC_CH1", MIXSRC_CH1 },
//  { "SWSRC_LAST", SWSRC_LAST_LOGICAL_SWITCH },
//  { "EVT_MENU_BREAK", EVT_KEY_BREAK(KEY_MENU) },
//#if !defined(PCBHORUS)
//  { "EVT_PAGE_BREAK", EVT_KEY_BREAK(KEY_PAGE) },
//  { "EVT_PAGE_LONG", EVT_KEY_LONG(KEY_PAGE) },
//#endif
  { "EVT_MENU_BREAK", EVT_KEY_BREAK(KEY_MENU) },
  { "EVT_MENU_LONG", EVT_KEY_LONG(KEY_MENU) },
  { "EVT_EXIT_BREAK", EVT_KEY_BREAK(KEY_EXIT) },
  { "EVT_UP_BREAK", EVT_KEY_BREAK(KEY_UP) },
  { "EVT_DOWN_BREAK", EVT_KEY_BREAK(KEY_DOWN) },
  { "EVT_UP_FIRST", EVT_KEY_FIRST(KEY_UP) },
  { "EVT_DOWN_FIRST", EVT_KEY_FIRST(KEY_DOWN) },
  { "EVT_UP_REPT", EVT_KEY_REPT(KEY_UP) },
  { "EVT_DOWN_REPT", EVT_KEY_REPT(KEY_DOWN) },
  { "EVT_LEFT_FIRST", EVT_KEY_FIRST(KEY_LEFT) },
  { "EVT_RIGHT_FIRST", EVT_KEY_FIRST(KEY_RIGHT) },
  { "EVT_BTN_BREAK", EVT_KEY_BREAK(BTN_RE) },
  { "EVT_BTN_LONG", EVT_KEY_LONG(BTN_RE) },
//#if !defined(COLORLCD)
//  { "FILL_WHITE", FILL_WHITE },
//  { "GREY_DEFAULT", GREY_DEFAULT },
//#endif
  { "SOLID", SOLID },
  { "DOTTED", DOTTED },
  { "FORCE", FORCE },
  { "ERASE", ERASE },
  { "ROUND", ROUND },
  { "LCD_W", LCD_W },
  { "LCD_H", LCD_H },
//  { "PLAY_NOW", PLAY_NOW },
//  { "PLAY_BACKGROUND", PLAY_BACKGROUND },
//  { "TIMEHOUR", TIMEHOUR },
  { NULL, 0 }  /* sentinel */
};




