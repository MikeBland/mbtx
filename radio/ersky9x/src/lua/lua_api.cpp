
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

#include "frsky.h"

extern struct t_telemetryTx TelemetryTx ;

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

uint32_t strMatch( const char *a, const char *b, uint32_t length ) ;
//{
//	if ( length == 0 )
//	{
//		return 1 ;
//	}
//	while ( length )
//	{
//		if ( *a == 0 || *b == 0 )
//		{
//			return 1 ;
//		}
//		if ( *a++ == *b++ )
//		{
//			if ( --length == 0 )
//			{
//				return 1 ;
//			}
//		}
//		else
//		{
//			break ;
//		}
//	}
//	return 0 ;
//}


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

static int luaKillEvents(lua_State * L)
{
  int number = luaL_checkinteger(L, 1);
extern void killEvents(uint8_t event) ;
	killEvents( number) ;
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
    lua_pushboolean(L, sportPacketSend( (uint8_t *)0, (uint16_t)0 ) ) ;
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
		value = sportPacketSend( sportPacket, (uint16_t)sportPacket[7] ) ;
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

	if ( fifo128Space( &Script_fifo ) <= ( 128 - 8 ) )
	{
		uint32_t value ;
		value = get_fifo128( &Script_fifo ) & 0x1F ;
    lua_pushnumber(L, value ) ;	// physical ID
		value = get_fifo128( &Script_fifo ) ;
    lua_pushnumber(L, value ) ;	// prim
		value = get_fifo128( &Script_fifo ) ;
		value |= get_fifo128( &Script_fifo ) << 8 ;
    lua_pushnumber(L, value ) ;	// App ID
		value = get_fifo128( &Script_fifo ) ;
		value |= get_fifo128( &Script_fifo ) << 8 ;
		value |= get_fifo128( &Script_fifo ) << 16 ;
		value |= get_fifo128( &Script_fifo ) << 24 ;
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

//#if defined(XFIRE)
///*luadoc
//@function crossfireTelemetryPop()

//Pops a received Crossfire Telemetry packet from the queue.

//@retval nil queue does not contain any (or enough) bytes to form a whole packet

//@retval multiple returns 2 values:
// * command (number)
// * packet (table) data bytes

//@status current Introduced in 2.2.0
//*/
static int luaCrossfireTelemetryPop(lua_State * L)
{
//  if (!luaInputTelemetryFifo) {
//    luaInputTelemetryFifo = new Fifo<uint8_t, LUA_TELEMETRY_INPUT_FIFO_SIZE>();
//    if (!luaInputTelemetryFifo) {
//      return 0;
//    }
//  }

  int32_t length = 0 ;
	uint8_t data = 0 ;
	length = peek_fifo128( &Script_fifo ) ;
	if ( length != -1)
	{
		if ( fifo128Space( &Script_fifo ) <= (uint32_t)(127 - length ) )
		{
	    // length value includes the length field
			length = get_fifo128( &Script_fifo ) ;
			data = get_fifo128( &Script_fifo ) ;
	    lua_pushnumber(L, data) ;
	    lua_newtable(L) ;
	    for ( uint32_t i = 1 ; i<(uint32_t)length-1 ; i += 1)
			{
				data = get_fifo128( &Script_fifo ) ;
	      lua_pushinteger(L, i) ;
	      lua_pushinteger(L, data) ;
	      lua_settable(L, -3) ;
			}	
    	return 2 ;
		}	

//  if (luaInputTelemetryFifo->probe(length) && luaInputTelemetryFifo->size() >= uint32_t(length)) {
//    luaInputTelemetryFifo->pop(length);
//    luaInputTelemetryFifo->pop(data); // command
//    lua_pushnumber(L, data);
//    lua_newtable(L);
//    for (uint8_t i=1; i<length-1; i++) {
//      luaInputTelemetryFifo->pop(data);
//      lua_pushinteger(L, i);
//      lua_pushinteger(L, data);
//      lua_settable(L, -3);
//    }
//    return 2;
  }

  return 0 ;
}

///*luadoc
//@function crossfireTelemetryPush()

//This functions allows for sending telemetry data toward the TBS Crossfire link.

//When called without parameters, it will only return the status of the output buffer without sending anything.

//@param command command

//@param data table of data bytes

//@retval boolean  data queued in output buffer or not.

//@retval nil      incorrect telemetry protocol.

//@status current Introduced in 2.2.0, retval nil added in 2.3.4
//*/
static int luaCrossfireTelemetryPush(lua_State * L)
{
//  if (telemetryProtocol != PROTOCOL_TELEMETRY_CROSSFIRE) {
//    lua_pushnil(L);
//    return 1;
//  }

	if (lua_gettop(L) == 0)
	{
    lua_pushboolean(L, xfirePacketSend( 0, 0, 0 ) ) ;
  }
  else if (lua_gettop(L) > 64 )
	{
    lua_pushboolean(L, false);
    return 1;
  }
  else if ( xfirePacketSend( 0, 0, 0 ) )
	{
    uint8_t command = luaL_checkunsigned(L, 1) ;
    luaL_checktype(L, 2, LUA_TTABLE) ;
    uint8_t length = luaL_len(L, 2);

		TelemetryTx.XfireTx.command = command ;
		for ( uint32_t i = 0 ; i < length ; i += 1 )
		{
      lua_rawgeti(L, 2, i+1);

			TelemetryTx.XfireTx.data[i] = luaL_checkunsigned(L, -1) ;
		}
		TelemetryTx.XfireTx.count = length ;

//    outputTelemetryBuffer.pushByte(MODULE_ADDRESS);
//    outputTelemetryBuffer.pushByte(2 + length); // 1(COMMAND) + data length + 1(CRC)
//    outputTelemetryBuffer.pushByte(command); // COMMAND
//    for (int i=0; i<length; i++) {
//      lua_rawgeti(L, 2, i+1);
//      outputTelemetryBuffer.pushByte(luaL_checkunsigned(L, -1));
//    }
//    outputTelemetryBuffer.pushByte(crc8(outputTelemetryBuffer.data+2, 1 + length));
//    outputTelemetryBuffer.setDestination(TELEMETRY_ENDPOINT_SPORT);
    
		lua_pushboolean(L, true);
  }
  else
	{
    lua_pushboolean(L, false) ;
  }
  return 1;
}
//#endif





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

static int luaGetVersion(lua_State * L)
{
  lua_pushstring(L, "12345");
  lua_pushstring(L, "X10");
  lua_pushnumber(L, 2);
  lua_pushnumber(L, 3);
  lua_pushnumber(L, 4);
  return 5;
}

/*luadoc
@function getRSSI()
Get RSSI value as well as low and critical RSSI alarm levels (in dB)
@retval rssi RSSI value (0 if no link)
@retval alarm_low Configured low RSSI alarm level
@retval alarm_crit Configured critical RSSI alarm level
@status current Introduced in 2.2.0
*/
static int luaGetRSSI(lua_State * L)
{
	int8_t offset ;
  lua_pushunsigned(L, TelemetryData[FR_RXRSI_COPY]);
	offset = rssiOffsetValue( 0 ) ;
  lua_pushunsigned(L, g_model.rssiOrange + offset);
	offset = rssiOffsetValue( 1 ) ;
  lua_pushunsigned(L, g_model.rssiRed + offset);
  return 3;
}

#define MESSAGEBOX_W	15

void drawMessageBoxBackground(coord_t top, coord_t height)
{
  // white background
	uint32_t i ;
	for ( i = 0 ; i < 5 ; i += 1)
	{
		PUTS_ATT_N( 3*FW, top+i*FH, "\223\223\223\223\223\223\223\223\223\223\223\223\223\223\223\223", MESSAGEBOX_W, 0 ) ;
	}
  // border
  lcd_rect( 3*FW-1, top-1, MESSAGEBOX_W*FW+1, height+1 ) ;
}

void drawMessageBox(const char * title)
{
//  char title_buf[WARNING_LINE_LEN + 1];
//  uint8_t title_len = 0;
//  uint8_t title_index = 0;
//  uint8_t line_index = 0;
//  uint8_t space_cnt = 0;

  // background + border
  drawMessageBoxBackground( FH+3, 5*FH ) ;

	lcdDrawSizedText( 4*FW, 2*FH+3, title, 14 ) ;

}

uint8_t WarningResult = 0 ;
const char *WarningText = 0 ;
const char *WarningInfoText ;

void runPopupWarning( uint8_t event)
{
  WarningResult = false ;

  drawMessageBox( WarningText ) ;

//  if (warningInfoText) {
//    lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y+FH, warningInfoText, warningInfoLength, warningInfoFlags);
//  }

//  switch (warningType) {
//    case WARNING_TYPE_WAIT:
//      return;

//    case WARNING_TYPE_INFO:
//      lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+2*FH+2, STR_OK);
//      break;

//    case WARNING_TYPE_ASTERISK:
//      lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+2*FH+2, STR_EXIT);
//      break;

//    default:
//      lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+2*FH+2, STR_POPUPS_ENTER_EXIT);
	lcdDrawSizedText( 4*FW, 4*FH+3, "[EXIT]  [ENTER]", 15 ) ;
//      break;
//  }


	switch (event)
	{
    case EVT_KEY_BREAK(KEY_MENU):
    case EVT_KEY_BREAK(BTN_RE):
//      if (warningType == WARNING_TYPE_ASTERISK)
//        // key ignored, the user has to press [EXIT]
//        break;

//      if (warningType == WARNING_TYPE_CONFIRM) {
//        warningType = WARNING_TYPE_ASTERISK;
		      WarningText = 0 ;
//        warningText = nullptr;
//        if (popupMenuHandler)
//          popupMenuHandler(STR_OK);
//        else
          WarningResult = true;
//        break;
//      }
//      // no break

    case EVT_KEY_BREAK(KEY_EXIT):
//      if (warningType == WARNING_TYPE_CONFIRM) {
//        if (popupMenuHandler)
//          popupMenuHandler(STR_EXIT);
//      }
      WarningText = 0 ;
//      warningType = WARNING_TYPE_ASTERISK;
    break ;
  }
}


int luaPopupConfirmation(lua_State * L)
{
////   warningType = WARNING_TYPE_CONFIRM ;
  uint8_t event ;

  if (lua_isnone(L, 3))
	{
    // only two args: deprecated mode
    WarningText = luaL_checkstring(L, 1) ;
    event = luaL_checkinteger(L, 2) ;
  }
  else
	{
    WarningText = luaL_checkstring(L, 1) ;
    WarningInfoText = luaL_checkstring(L, 2) ;
    event = luaL_optinteger(L, 3, 0) ;
  }

  runPopupWarning(event) ;
  
	if (!WarningText)
	{
    lua_pushstring(L, WarningResult ? "OK" : "CANCEL" ) ;
  }
  else
	{
    WarningText = NULL ;
    lua_pushnil(L) ;
  }
  return 1 ;
}



const luaL_Reg ersky9xLib[] = {
  { "getTime", luaGetTime },
  { "getDateTime", luaGetDateTime },
  { "getVersion", luaGetVersion },
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
  { "popupConfirmation", luaPopupConfirmation },
//  { "defaultStick", luaDefaultStick },
//  { "defaultChannel", luaDefaultChannel },
//  { "getRSSI", luaGetRSSI },
  { "killEvents", luaKillEvents },
  { "loadScript", luaLoadScript },
//#if !defined(COLORLCD)
//  { "GREY", luaGrey },
//#endif
  { "sportTelemetryPop", luaSportTelemetryPop },
  { "sportTelemetryPush", luaSportTelemetryPush },
//  { "setTelemetryValue", luaSetTelemetryValue },
//#if defined(XFIRE)
  { "crossfireTelemetryPop", luaCrossfireTelemetryPop },
  { "crossfireTelemetryPush", luaCrossfireTelemetryPush },
//#endif
  { "idleTime", luaIdleTime },
  { "getRSSI", luaGetRSSI },
  { NULL, NULL }  /* sentinel */
};


const luaR_value_entry ersky9xConstants[] = {
  { "FULLSCALE", RESX },
  { "XXLSIZE", XXLSIZE },
#if defined(PCBX12D) || defined(PCBX10)
  { "DBLSIZE", DBLSIZE },
  { "MIDSIZE", MIDSIZE },
  { "SMLSIZE", LUA_SMLSIZE },
#else
  { "XXLSIZE", DBLSIZE },
  { "DBLSIZE", DBLSIZE },
  { "MIDSIZE", MIDSIZE },
  { "SMLSIZE", LUA_SMLSIZE },
#endif
  { "INVERS", INVERS },
  { "RIGHT", LUA_RIGHT },
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
  { "BOLD", BOLD },
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

#if defined(PCBX10)
  { "EVT_MENU_BREAK", EVT_KEY_BREAK(KEY_UP) },
  { "EVT_MENU_LONG", EVT_KEY_LONG(KEY_UP) },
  { "EVT_EXIT_BREAK", EVT_KEY_BREAK(KEY_RTN) },
  { "EVT_PLUS_BREAK", EVT_KEY_BREAK(KEY_MENU) },
  { "EVT_MINUS_BREAK", EVT_KEY_BREAK(KEY_RIGHT) },
  { "EVT_PLUS_FIRST", EVT_KEY_FIRST(KEY_DOWN) },
  { "EVT_MINUS_FIRST", EVT_KEY_FIRST(KEY_UP) },
  { "EVT_PLUS_REPT", EVT_KEY_REPT(KEY_MENU) },
  { "EVT_MINUS_REPT", EVT_KEY_REPT(KEY_RIGHT) },
  { "EVT_PAGE_FIRST", EVT_KEY_FIRST(KEY_LEFT) },
  { "EVT_ENTER_FIRST", EVT_KEY_FIRST(BTN_RE) },
  { "EVT_ENTER_BREAK", EVT_KEY_BREAK(BTN_RE) },
  { "EVT_ENTER_LONG", EVT_KEY_LONG(BTN_RE) },
  { "TEXT_COLOR", LUA_TEXT_COLOUR },
  { "TEXT_INVERTED_COLOR", LUA_TEXT_INVERTED_COLOR },
  { "TEXT_INVERTED_BGCOLOR", LUA_TEXT_INVERTED_BGCOLOR },
  { "CUSTOM_COLOR", LUA_CUSTOM_COLOUR },
  { "RED", (double)LCD_RED },
  { "WHITE", (double)LCD_WHITE },
  { "BLACK", (double)LCD_BLACK },
//  { "TEXT_COLOR", TEXT_COLOR },
//  { "TEXT_BGCOLOR", TEXT_BGCOLOR },
//  { "TEXT_INVERTED_COLOR", TEXT_INVERTED_COLOR },
//  { "TEXT_INVERTED_BGCOLOR", TEXT_INVERTED_BGCOLOR },
#else
	// Map to X9D normal buttons
 #if defined (PCBX9LITE)
	{ "EVT_MENU_BREAK", EVT_KEY_BREAK(KEY_MENU) },
  { "EVT_MENU_LONG", EVT_KEY_LONG(KEY_MENU) },
  { "EVT_EXIT_BREAK", EVT_KEY_BREAK(KEY_EXIT) },
//  { "EVT_PLUS_BREAK", EVT_KEY_BREAK(KEY_MENU) },
//  { "EVT_MINUS_BREAK", EVT_KEY_BREAK(KEY_RIGHT) },
  { "EVT_PLUS_FIRST", EVT_ROTARY_RIGHT },
  { "EVT_MINUS_FIRST", EVT_ROTARY_LEFT },
//  { "EVT_PLUS_REPT", EVT_KEY_REPT(KEY_MENU) },
//  { "EVT_MINUS_REPT", EVT_KEY_REPT(KEY_RIGHT) },
  { "EVT_PAGE_FIRST", EVT_KEY_FIRST(KEY_PAGE) },
  { "EVT_PAGE_LONG", EVT_KEY_LONG(KEY_PAGE) },
	{ "EVT_PAGE_BREAK", EVT_KEY_BREAK(KEY_PAGE) },
  { "EVT_ENTER_FIRST", EVT_KEY_FIRST(BTN_RE) },
  { "EVT_ENTER_BREAK", EVT_KEY_BREAK(BTN_RE) },
	{ "EVT_ENTER_LONG", EVT_KEY_LONG(BTN_RE) },
 #else
	#if defined (PCBX7)
	{ "EVT_MENU_BREAK", EVT_KEY_BREAK(KEY_MENU) },
  { "EVT_MENU_LONG", EVT_KEY_LONG(KEY_MENU) },
  { "EVT_EXIT_BREAK", EVT_KEY_BREAK(KEY_EXIT) },
  { "EVT_PLUS_FIRST", EVT_ROTARY_RIGHT },
  { "EVT_MINUS_FIRST", EVT_ROTARY_LEFT },
  { "EVT_PAGE_FIRST", EVT_KEY_FIRST(KEY_PAGE) },
  { "EVT_PAGE_LONG", EVT_KEY_LONG(KEY_PAGE) },
	{ "EVT_PAGE_BREAK", EVT_KEY_BREAK(KEY_PAGE) },
  { "EVT_ENTER_FIRST", EVT_KEY_FIRST(BTN_RE) },
  { "EVT_ENTER_BREAK", EVT_KEY_BREAK(BTN_RE) },
	{ "EVT_ENTER_LONG", EVT_KEY_LONG(BTN_RE) },
  #else
	{ "EVT_MENU_BREAK", EVT_KEY_BREAK(KEY_UP) },
  { "EVT_MENU_LONG", EVT_KEY_LONG(KEY_UP) },
  { "EVT_EXIT_BREAK", EVT_KEY_BREAK(KEY_DOWN) },
  { "EVT_PLUS_BREAK", EVT_KEY_BREAK(KEY_MENU) },
  { "EVT_MINUS_BREAK", EVT_KEY_BREAK(KEY_RIGHT) },
  { "EVT_PLUS_FIRST", EVT_KEY_FIRST(KEY_MENU) },
  { "EVT_MINUS_FIRST", EVT_KEY_FIRST(KEY_RIGHT) },
  { "EVT_PLUS_REPT", EVT_KEY_REPT(KEY_MENU) },
  { "EVT_MINUS_REPT", EVT_KEY_REPT(KEY_RIGHT) },
  { "EVT_PAGE_FIRST", EVT_KEY_FIRST(KEY_LEFT) },
  { "EVT_PAGE_LONG", EVT_KEY_LONG(KEY_LEFT) },
	{ "EVT_PAGE_BREAK", EVT_KEY_BREAK(KEY_LEFT) },
  { "EVT_ENTER_FIRST", EVT_KEY_FIRST(KEY_EXIT) },
  { "EVT_ENTER_BREAK", EVT_KEY_BREAK(KEY_EXIT) },
	{ "EVT_ENTER_LONG", EVT_KEY_LONG(KEY_EXIT) },
  #endif
 #endif
#endif

#if defined(PCBX12D) || defined(PCBX10) || defined (PCBX9LITE) || defined (PCBX7)
	{ "EVT_ROT_LEFT", EVT_ROTARY_LEFT },
  { "EVT_ROT_RIGHT", EVT_ROTARY_RIGHT },
#endif

#if defined(PCBX9D) && ( defined(REVPLUS) || defined(REVNORM) )
  { "EVT_VIRTUAL_ENTER", EVT_KEY_BREAK(KEY_MENU) },
  { "EVT_VIRTUAL_ENTER_LONG", EVT_KEY_LONG(KEY_MENU) },
#else
 #if defined (PCB9XT)
  { "EVT_VIRTUAL_ENTER", EVT_KEY_BREAK(KEY_MENU) },
  { "EVT_VIRTUAL_ENTER_LONG", EVT_KEY_LONG(KEY_MENU) },
 #else
  { "EVT_VIRTUAL_ENTER", EVT_KEY_BREAK(BTN_RE) },
  { "EVT_VIRTUAL_ENTER_LONG", EVT_KEY_LONG(BTN_RE) },
 #endif
#endif
#if defined(PCBX12D) || defined(PCBX10) || defined (PCBX9LITE)
//  { "EVT_VIRTUAL_ENTER", EVT_KEY_BREAK(BTN_RE) },
//  { "EVT_VIRTUAL_ENTER_LONG", EVT_KEY_LONG(BTN_RE) },
 #if !defined(PCBX12D)	
	{ "EVT_VIRTUAL_PREV_PAGE", EVT_KEY_LONG(KEY_PAGE) },
	{ "EVT_VIRTUAL_NEXT_PAGE", EVT_KEY_BREAK(KEY_PAGE) },
 #endif
	{ "EVT_VIRTUAL_PREV", EVT_ROTARY_LEFT },
	{ "EVT_VIRTUAL_NEXT", EVT_ROTARY_RIGHT },
  { "EVT_VIRTUAL_DEC", EVT_ROTARY_LEFT },
  { "EVT_VIRTUAL_INC", EVT_ROTARY_RIGHT },
 #if defined (PCBX9LITE)
	{ "EVT_VIRTUAL_MENU", EVT_KEY_BREAK(KEY_MENU) },
  { "EVT_VIRTUAL_MENU_LONG", EVT_KEY_LONG(KEY_MENU) },
  { "EVT_VIRTUAL_EXIT", EVT_KEY_BREAK(KEY_EXIT) },
 #else
  #if defined(PCBX12D)	
	{ "EVT_VIRTUAL_MENU", EVT_KEY_BREAK(KEY_MENU) },
  { "EVT_VIRTUAL_MENU_LONG", EVT_KEY_LONG(KEY_MENU) },
  { "EVT_VIRTUAL_EXIT", EVT_KEY_BREAK(KEY_EXIT) },
  #else
	{ "EVT_VIRTUAL_MENU", EVT_KEY_BREAK(KEY_MDL) },
  { "EVT_VIRTUAL_MENU_LONG", EVT_KEY_LONG(KEY_MDL) },
  { "EVT_VIRTUAL_EXIT", EVT_KEY_BREAK(KEY_RTN) },
  #endif
 #endif
#else
 #if defined (PCBX7)
	{ "EVT_VIRTUAL_PREV_PAGE", EVT_KEY_LONG(KEY_PAGE) },
	{ "EVT_VIRTUAL_NEXT_PAGE", EVT_KEY_BREAK(KEY_PAGE) },
	{ "EVT_VIRTUAL_PREV", EVT_ROTARY_LEFT },
	{ "EVT_VIRTUAL_NEXT", EVT_ROTARY_RIGHT },
  { "EVT_VIRTUAL_DEC", EVT_ROTARY_LEFT },
  { "EVT_VIRTUAL_INC", EVT_ROTARY_RIGHT },
 #else
	{ "EVT_VIRTUAL_PREV_PAGE", EVT_KEY_BREAK(KEY_LEFT) },
	{ "EVT_VIRTUAL_NEXT_PAGE", EVT_KEY_BREAK(KEY_RIGHT) },
	{ "EVT_VIRTUAL_PREV", EVT_KEY_FIRST(KEY_UP) },
	{ "EVT_VIRTUAL_NEXT", EVT_KEY_FIRST(KEY_DOWN) },
 #endif
#endif


//EVT_VIRTUAL_NEXT_PAGE 	for PAGE navigation
//EVT_VIRTUAL_PREVIOUS_PAGE 	for PAGE navigation
//EVT_VIRTUAL_ENTER 	
//EVT_VIRTUAL_ENTER_LONG 	
//EVT_VIRTUAL_MENU 	
//EVT_VIRTUAL_MENU_LONG 	
//EVT_VIRTUAL_NEXT 	for FIELDS navigation
//EVT_VIRTUAL_NEXT_REPT 	for FIELDS navigation
//EVT_VIRTUAL_PREVIOUS 	for FIELDS navigation
//EVT_VIRTUAL_PREV_REPT 	for FIELDS navigation
//EVT_VIRTUAL_INC 	for VALUES navigation
//EVT_VIRTUAL_INC_REPT 	for VALUES navigation
//EVT_VIRTUAL_DEC 	for VALUES navigation
//EVT_VIRTUAL_DEC_REPT 	for VALUES navigation


//  { "EVT_MENU_BREAK", EVT_KEY_BREAK(KEY_MENU) },
//  { "EVT_MENU_LONG", EVT_KEY_LONG(KEY_MENU) },
//  { "EVT_EXIT_BREAK", EVT_KEY_BREAK(KEY_EXIT) },
//  { "EVT_UP_BREAK", EVT_KEY_BREAK(KEY_UP) },
//  { "EVT_DOWN_BREAK", EVT_KEY_BREAK(KEY_DOWN) },
//  { "EVT_UP_FIRST", EVT_KEY_FIRST(KEY_UP) },
//  { "EVT_DOWN_FIRST", EVT_KEY_FIRST(KEY_DOWN) },
//  { "EVT_UP_REPT", EVT_KEY_REPT(KEY_UP) },
//  { "EVT_DOWN_REPT", EVT_KEY_REPT(KEY_DOWN) },
//  { "EVT_LEFT_FIRST", EVT_KEY_FIRST(KEY_LEFT) },
//  { "EVT_RIGHT_FIRST", EVT_KEY_FIRST(KEY_RIGHT) },
  
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
#if (LCD_W == 212)
  { "LCD_W", (LCD_W-22) },
#else
  { "LCD_W", LCD_W },
#endif
#if LCD_H == 272
	{ "LCD_H", 240 },
#else
	{ "LCD_H", LCD_H },
#endif
//  { "PLAY_NOW", PLAY_NOW },
//  { "PLAY_BACKGROUND", PLAY_BACKGROUND },
//  { "TIMEHOUR", TIMEHOUR },
  { NULL, 0 }  /* sentinel */
};


/*luadoc
@function model.getModule(index)

Get RF module parameters

`Type` values:
  * 0 NONE
  * 1 PPM
  * 2 XJT_PXX1
  * 3 ISRM_PXX2
  * 4 DSM2
  * 5 CROSSFIRE
  * 6 MULTIMODULE
  * 7 R9M_PXX1
  * 8 R9M_PXX2
  * 9 R9M_LITE_PXX1
  * 10 R9M_LITE_PXX2
  * 11 R9M_LITE_PRO_PXX1
  * 12 R9M_LITE_PRO_PXX2
  * 13 SBUS
  * 14 XJT_LITE_PXX2

`subType` values for XJT_PXX1:
 * -1 OFF
 * 0 D16
 * 1 D8
 * 2 LR12

@param index (number) module index (0 for internal, 1 for external)

@retval nil requested module does not exist

@retval table module parameters:
 * `subType` (number) protocol index
 * `modelId` (number) receiver number
 * `firstChannel` (number) start channel (0 is CH1)
 * `channelsCount` (number) number of channels sent to module
 * `Type` (number) module type
 * if the module type is Multi additional information are available
 * `protocol` (number) protocol number (Multi only)
 * `subProtocol` (number) sub-protocol number (Multi only)
 * `channelsOrder` (number) first 4 channels expected order (Multi only)

@status current Introduced in 2.2.0
*/

const uint8_t ProtTable[] = { 1,2,4,6,5,3,13,0,0,0,0,0,0,0,0,0 } ;

static int luaModelGetModule(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < 2)
	{
    struct t_module & module = g_model.Module[idx];
    lua_newtable(L);
    lua_pushtableinteger(L, "subType", module.sub_protocol);
    lua_pushtableinteger(L, "modelId", module.pxxRxNum);
    lua_pushtableinteger(L, "firstChannel", module.startChannel);
    lua_pushtableinteger(L, "channelsCount", module.channels);
    lua_pushtableinteger(L, "Type", ProtTable[module.protocol]);
//#if defined(MULTIMODULE)
    if (module.protocol == PROTO_MULTI)
		{
//      int protocol = g_model.moduleData[idx].getMultiProtocol() + 1 ;
//      int subprotocol = g_model.moduleData[idx].subType;
//      convertOtxProtocolToMulti(&protocol, &subprotocol); // Special treatment for the FrSky entry...
//      lua_pushtableinteger(L, "protocol", protocol);
//      lua_pushtableinteger(L, "subProtocol", subprotocol);
      lua_pushtableinteger(L, "protocol", module.protocol);
      lua_pushtableinteger(L, "subProtocol", module.sub_protocol);
//      if (getMultiModuleStatus(idx).isValid())
//			{
//        if (getMultiModuleStatus(idx).ch_order == 0xFF)
//				{
//          lua_pushtableinteger(L, "channelsOrder", -1);
//				}
//        else
//				{
//          lua_pushtableinteger(L, "channelsOrder", getMultiModuleStatus(idx).ch_order);
//				}
//      }
//      else
//			{
        lua_pushtableinteger(L, "channelsOrder", -1);
//      }
    }
//#endif
  }
  else
	{
    lua_pushnil(L);
  }
  return 1;
}




const luaL_Reg modelLib[] = {
//  { "getInfo", luaModelGetInfo },
//  { "setInfo", luaModelSetInfo },
  { "getModule", luaModelGetModule },
//  { "setModule", luaModelSetModule },
//  { "getTimer", luaModelGetTimer },
//  { "setTimer", luaModelSetTimer },
//  { "resetTimer", luaModelResetTimer },
//  { "deleteFlightModes", luaModelDeleteFlightModes },
//  { "getFlightMode", luaModelGetFlightMode },
//  { "setFlightMode", luaModelSetFlightMode },
//  { "getInputsCount", luaModelGetInputsCount },
//  { "getInput", luaModelGetInput },
//  { "insertInput", luaModelInsertInput },
//  { "deleteInput", luaModelDeleteInput },
//  { "deleteInputs", luaModelDeleteInputs },
//  { "defaultInputs", luaModelDefaultInputs },
//  { "getMixesCount", luaModelGetMixesCount },
//  { "getMix", luaModelGetMix },
//  { "insertMix", luaModelInsertMix },
//  { "deleteMix", luaModelDeleteMix },
//  { "deleteMixes", luaModelDeleteMixes },
//  { "getLogicalSwitch", luaModelGetLogicalSwitch },
//  { "setLogicalSwitch", luaModelSetLogicalSwitch },
//  { "getCustomFunction", luaModelGetCustomFunction },
//  { "setCustomFunction", luaModelSetCustomFunction },
//  { "getCurve", luaModelGetCurve },
//  { "setCurve", luaModelSetCurve },
//  { "getOutput", luaModelGetOutput },
//  { "setOutput", luaModelSetOutput },
//  { "getGlobalVariable", luaModelGetGlobalVariable },
//  { "setGlobalVariable", luaModelSetGlobalVariable },
//  { "getSensor", luaModelGetSensor },
//  { "resetSensor", luaModelResetSensor },
//  { "getSwashRing", luaModelGetSwashRing },
//  { "setSwashRing", luaModelSetSwashRing },
  { NULL, 0 }  /* sentinel */
};





