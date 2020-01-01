#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "ctype.h"


//#define QT		1

#ifndef	QT
#include "ersky9x.h"
#include "myeeprom.h"
#include "stringidx.h"
#include "lcd.h"
#include "drivers.h"
#include "ff.h"
#include "basic.h"
#include "audio.h"
#include "menus.h"
#include "frsky.h"
#include "maintenance.h"

#ifdef PCBSKY
#include "AT91SAM3S4.h"                   
#endif

#if defined(PCBX9D) || defined(PCB9XT)
#include "X9D/stm32f2xx.h"
#endif

extern SKYMixData *mixAddress( uint32_t index ) ;
extern int32_t chans[NUM_SKYCHNOUT+EXTRA_SKYCHANNELS] ;
extern uint32_t isqrt32( uint32_t x ) ;

uint8_t LoadingIndex ;
uint8_t LoadingNeeded ;

uint32_t TotalExecTime ;
uint32_t TotalTimes ;

#define FILE_SUPPORT	1
#define DIR_LENGTH		40
struct fileControl BasicFileControl ;
void setupFileNames( TCHAR *dir, struct fileControl *fc, char *ext ) ;
uint8_t DirectoryName[DIR_LENGTH] ;
extern union t_sharedMemory SharedMemory ;
extern uint32_t FileSize[] ;
extern uint8_t ScriptDirNeeded ;

extern uint8_t Com2TxBuffer[] ;
extern struct t_serial_tx Com2_tx ;

extern uint8_t BtCurrentFunction ;
void scriptRequestBt() ;
void scriptReleasetBt() ;


#else
#include "basic.h"
#include "mainwindow.h"
#include "lcd.h"


uint16_t ScriptFlags ;
extern uint32_t g_tmr10ms ;


uint16_t isqrt32(uint32_t n)
{
  uint16_t c = 0x8000;
  uint16_t g = 0x8000;

  for(;;)
	{
    if((uint32_t)g*g > n)
      g ^= c;
    c >>= 1;
    if(c == 0)
      return g;
    g |= c;
  }
}

#endif


#define IsDigit(c)	(((c)>='0')&&((c)<='9'))

int32_t btSend( uint32_t length, uint8_t *data ) ;


/*
 * token types
 * as the input is parsed it is broken down into one of these token types
 * see get_token()
 */
#define DELIMITER			1
#define VARIABLE			2
#define NUMBER				3
#define COMMAND				4
#define STRING				5
#define QUOTE					6
#define ARRAY					7
#define FUNCTION			8
#define LABEL					9
#define CONSTANT			10

// Opcodes 0x01 to 0x3F (1-63)
#define IF   				1
#define THEN 				2
#define GOTO 				3
#define FINISHED  	5
#define GOSUB 			6
#define RETURN  		7
#define STOP 				8
#define END					9
#define LET						10
#define LESSEQUAL   	12
#define GREATEQUAL  	13
#define IN_FUNCTION		14
#define USER_FUNCTION	15
#define FINISH				16
#define NGOTO					20
#define WHILE					21
#define ELSE					22
#define ELSEIF				23
#define FUNC					25

// Missed out, 4, 11, 17, 18, 19, 24
// Re-allocated to >63 as not put in code

//#define FOR  					25
//#define NEXT 					22
//#define TO   					23
//#define STEP					24
//#define WEND					26
//#define REPEAT				27
//#define UNTIL					28

//#define CONTINUE 31
//#define WAIT	18
//#define TRACE	19

#define EXCLAIM			33
#define DQUOTE			34
#define HASHCHAR		35
//#define DOLLAR			36
#define BITXOR			36
#define PERCENT			37
#define BITAND			38
#define SQUOTE			39
#define OPENPAR			40	
#define CLOSEPAR		41
#define MULTIPLY		42	
#define PLUS				43
#define COMMA				44
#define MINUS       45
#define	DOT					46
#define DIVIDE		  47
// 48-57 digits, these codes might be available
#define BITOR				48

#define COLON				58
#define SEMICOLON		59		
#define LESS				60
#define EQUALS		  61
#define GREATER			62	
//#define BITOR				63
#define QMARK				63
// UNARY_NOT ??

// Special opcodes, not placed in code
#define	BREAK				64
#define REM	        65
#define DEFARRAY		66
#define DEFINT			67
#define DEFBYTE			68
#define DEFCONST		69
#define EOL  				70

				 
// Internal Functions
#define DRAWCLEAR			1
#define DRAWNUMBER		2
#define DRAWLINE			3
#define DRAWTEXT			4
#define PLAYNUMBER		5
#define GETVALUE			6
#define DRAWPOINT			7
#define DRAWRECT			8
#define IDLETIME			9
#define GETTIME				10
#define SPORTSEND			11
#define SPORTRECEIVE	12
#define NOT						13
#define ABS						14
#define GETLASTPOS		15
#define DRAWTIMER			16
#define SYSFLAGS			17
#define SETTELITEM		18
#define STRTOARRAY		19
#define GETSWITCH			20
#define SETSWITCH			21
#define PLAYFILE			22
#define GETRAWVALUE		23
#define KILLEVENTS		24
#define BITFIELD			25
#define POWER					26
#define CROSSFIRERECEIVE	27
#define CROSSFIRESEND	28
#define POPUP					29
#define SYSTOARRAY		30
#define SQRT					31
#define DRAWBITMAP		32
#define BTRECEIVE			33
#define BTSEND				34
#define DIRECTORY			35
#define FILESELECT		36
#define FOPEN					37
#define FREAD					38
#define FCLOSE				39
#define FWRITE				40
#define BYTEMOVE			41
#define ALERT					42
#define RETURNVALUE		43
#define RESETTELEMETRY 44

// Assignment options:
#define SETEQUALS			0
#define PLUSEQUALS		1
#define MINUSEQUALS		2
#define TIMESEQUALS		3
#define DIVIDEEQUALS	4
#define PERCENTEQUALS	5
#define ANDEQUALS			6
#define OREQUALS			7
#define XOREQUALS			8

// Type for struct t_control
#define TYPE_IF				0
#define TYPE_WHILE		1
#define TYPE_ELSE			2
//#define TYPE_ELSEIF		3


// & = 0x26 = 38
// | = 0x7C - problem

// More draw functions:
// refresh ?
// filled rectangle
// switch
// screen title

// More general interface items
// getDateTime


// Symbol types
#define SYM_LABEL 		1
#define SYM_VARIABLE	2
#define SYM_FUNCTION	3
#define SYM_ARRAY			4
#define SYM_CONST			5

#define SYM_LAB_REF		1
#define SYM_LAB_DEF		2

#define SYM_VAR_INT		3
#define SYM_VAR_UNS		4
#define SYM_VAR_FLOAT	5
#define SYM_VAR_ARRAY_BYTE	16
#define SYM_VAR_ARRAY_INT		17
#define SYM_VAR_ARRAY_UNS		18
#define SYM_VAR_ARRAY_FLOAT	19

#define SYM_VAR_ARRAY_TYPE	16

// Syntax errors
#define SE_DUP_LABEL			1
#define SE_EXEC_BADLINE		2
#define SE_SYNTAX					3
#define SE_TOO_MANY_VARS	4
#define SE_NO_BRACKET			5
#define SE_DIV_ZERO				6
#define SE_NO_THEN				7
#define SE_NO_GOSUB				8
#define SE_NO_FUNCTION		9
#define SE_TOO_LARGE			10
#define SE_DIMENSION			11
#define SE_TOO_MANY_CALLS	12
#define SE_NO_WHILE				13
#define SE_STRING_LONG		14
#define SE_NO_FLOAT				15
#define SE_MISSING_END		16

#define MAX_VARIABLES	2

#define MAX_CALL_STACK	20
#define MAX_PARAM_STACK	20

#define PARAM_TYPE_NUMBER		0
#define PARAM_TYPE_STRING		1
#define PARAM_TYPE_EITHER		2


#ifdef QT
const char *ErrorText[] = {
 "Duplicate Label",
 "Bad Line Number",
 "Syntax",
 "Too Many Variables",
 "Missing Bracket",
 "Divide by 0",
 "Missing 'then'",
 "Missing 'gosub'",
 "Missing Function",
 "Too Large",
 "Dimension",
 "Too Many Nested Calls",
 "Missing while",
 "String too long",
 "No Floating point",
 "Missing end"
} ;	
#endif


//#define MAX_EXP_STACK	10

// Compiled coding:
// 1 - 0x3F, opcodes
// 0x80 ( to 0xFF) Line number in 15 bits				1xxx
// 0x70 String                        0111 0000
// 0x60 Variable, index in 8 bits			0110
// 0x68 Variable, index in 16 bits
// 0x64 Variable int array, index in 8 bits, dimension in 8 bits
// 0x6C Variable int array, index in 16 bits, dimension in 8 bits
// 0x66 Variable byte array, index in 8 bits, dimension in 8 bits
// 0x6E Variable byte array, index in 16 bits, dimension in 8 bits

// 0x61 local variable/parameter offset (8 bits) for functions

// Possible Enhancement
// 0x65 Variable int array, index in 8 bits, dimension in 16 bits
// 0x6D Variable int array, index in 16 bits, dimension in 16 bits
// 0x67 Variable byte array, index in 8 bits, dimension in 16 bits
// 0x6F Variable byte array, index in 16 bits, dimension in 16 bits


// 0x58 Number in 8 bits ??           010x
// 0x50 Number in 32 bits
// 0x48 (to 0x4F) Number in 3 bits (0-7)
// 0x40 Number in 16 bits

// unused 0x41 - 0x47
// unused 0x62, 0x63, 0x69, 0x6A, 0x6B

// Possible Enhancement
// 0x71-0x7F Could be for indirection

// Consider:
// Change string to 0x7F
// 0x70 pointer to string, index in 8 bits
// 0x78 pointer to string, index in 16 bits
// 0x74 array of pointer to string, index in 8 bits, dimension in 8 bits
// 0x7C array of pointer to string, index in 16 bits, dimension in 8 bits
// 0x76 array of pointer to int array, index in 8 bits, dimension in 8 bits
// 0x7E array of pointer to int array, index in 16 bits, dimension in 8 bits
// 0x72 array of pointer to byte array, index in 8 bits, dimension in 8 bits
// 0x7A array of pointer to byte array, index in 16 bits, dimension in 8 bits

// 0x71 could be used for "Shift", next byte is extended coding byte


union t_parameter
{
	int32_t var ;
	uint8_t *cpointer ;
	uint8_t *bpointer ;
	int32_t *ipointer ;
} ;

struct byteAddress
{
	uint16_t varOffset ;
	uint16_t varIndex ;
	uint32_t dimension ;
} ;

//union t_varAddress
//{
//	uint8_t *bpointer ;
//	int32_t *ipointer ;
//} ;


uint32_t LineNumber ;
//uint32_t ExecLinesProcessed ;
uint16_t ParseErrorLine ;
uint8_t ParseError ;
uint8_t BasicLoadedType ;


struct t_basicRunTime
{
	uint32_t RunError ;
	uint32_t RunErrorLine ;
	uint32_t RunLineNumber ;
//	uint32_t RunLastLineNumber ;
	uint8_t *ExecProgPtr ;
	uint32_t ExecLinesProcessed ;
	uint32_t CallIndex ;
	uint8_t *CallStack[MAX_CALL_STACK] ;
	uint8_t *ByteArrayStart ;
	int32_t *IntArrayStart ;
	int32_t FunctionReturnValue ;
	uint16_t ParamFrameIndex ;
	uint16_t ParamStackIndex ;
	union t_parameter ParameterStack[MAX_PARAM_STACK] ;
	union t_array	// Place holder, the arrays extend as declared
	{
		int32_t Variables[MAX_VARIABLES] ;
		uint8_t byteArray[8] ;
		int32_t intArray[2] ;
	} Vars ;
} __attribute__((__may_alias__)) ;

struct t_control
{
	uint16_t type ;
	uint16_t first ;
	uint16_t middle ;
} ;

#define MAX_CONTROL_INDEX		20
struct t_control CodeControl[MAX_CONTROL_INDEX] ;		// Max Nesting of control structures
uint8_t ControlIndex ;
uint8_t ScriptPopupActive ;

struct t_basicRunTime *RunTime ;

struct t_loadedScripts LoadedScripts[3] ;

#ifdef	QT
int32_t expression( void ) ;
#endif
#ifndef	QT
int32_t expression( void ) ;
#endif

#ifndef	QT
int32_t basicFindValueIndexByName( const char * name ) ;
int32_t basicFindSwitchIndexByName( const char * name ) ;
#endif

uint32_t execOneLine(void) ;

// Symbols
// Label: EntryLen, SYM_LABEL, StrLen, String/0, SYM_LAB_DEF/REF, Poslow, Poshi
// Variable: EntryLen, SYM_VARIABLE, StrLen, String/0, SYM_VAR_INT/UNS/FLOAT, Poslow, Poshi
// ArrayVariable: EntryLen, SYM_VARIABLE, StrLen, String/0, SYM_VAR_ARRAY_INT/UNS/FLOAT, Poslow, Poshi, dimension
// Change to:
// ArrayVariable: EntryLen, SYM_VARIABLE, StrLen, String/0, SYM_VAR_ARRAY_INT/UNS/FLOAT, Poslow, Poshi, dimensionlow, dimensionhi

// Const: EntryLen, SYM_CONST, StrLen, String/0, Val0, Val1, Val2, Val3
// Function: EntryLen, SYM_FUNCTION, StrLen, String/0, SYM_LAB_DEF/REF, [ParamCount, LocalCount,] Poslow, Poshi

struct commands
{ /* keyword lookup table */
  char command[22] ;
  uint16_t tok ;
} ;

const struct commands Table[] =
{ 		/* Commands must be entered lowercase */
  { "if", IF},
  { "then", THEN},
  { "goto", GOTO},
  { "gosub", GOSUB},
  { "return", RETURN},
  { "let", LET},
  { "rem", REM},
  { "stop", STOP},
  { "finish", FINISH },
  { "while", WHILE},
  { "elseif", ELSEIF},
  { "else", ELSE},
	{ "int", DEFINT},
	{ "byte", DEFBYTE},
	{ "array", DEFARRAY},
	{ "const", DEFCONST},
	{ "end", END},
//  { "print", PRINT},
//  { "input", INPUT},
//  { "for", FOR},
//  { "next", NEXT},
//  { "to", TO},
//  { "step", STEP},
//  { "wait", WAIT},
//  { "trace", TRACE},
//  { "wend", WEND},
  { "break", BREAK},
//  { "continue", CONTINUE},
//  { "repeat", REPEAT},
//  { "until", UNTIL},
//  { "function", FUNC},
	{ "//", REM},
  { "", 0 } /* mark end of table */
} ;

const struct commands InternalFunctions[] =
{
  { "drawclear", DRAWCLEAR },
  { "drawtext", DRAWTEXT },
  { "drawnumber", DRAWNUMBER },
  { "drawline", DRAWLINE },
	{ "playnumber", PLAYNUMBER },
  { "getvalue", GETVALUE },
  { "drawpoint", DRAWPOINT },
  { "drawrectangle", DRAWRECT },
  { "idletime", IDLETIME },
  { "gettime", GETTIME },
  { "sportTelemetrySend", SPORTSEND },
  { "sportTelemetryReceive", SPORTRECEIVE },
  { "not", NOT },
  { "abs", ABS },
  { "getLastPos", GETLASTPOS },
  { "drawtimer", DRAWTIMER },
  { "sysflags", SYSFLAGS },
	{ "settelitem", SETTELITEM },
	{ "strtoarray", STRTOARRAY },
	{ "getswitch", GETSWITCH },
	{ "setswitch", SETSWITCH },
	{ "playfile", PLAYFILE },
  { "getrawvalue", GETRAWVALUE },
	{ "killevents", KILLEVENTS },
	{ "bitfield", BITFIELD },
	{ "power", POWER },
	{ "crossfirereceive", CROSSFIRERECEIVE },
	{ "crossfiresend", CROSSFIRESEND },
	{ "popup", POPUP },
	{ "sysstrtoarray", SYSTOARRAY },
  { "sqrt", SQRT },
	{ "drawbitmap", DRAWBITMAP },
	{ "serialreceive", BTRECEIVE },
	{ "serialsend", BTSEND },
#ifdef FILE_SUPPORT
	{ "directory", DIRECTORY },
	{ "fileselect", FILESELECT },
	{ "fopen", FOPEN },
	{ "fread", FREAD },
	{ "fwrite", FWRITE },
	{ "fclose", FCLOSE },
#endif
	{ "bytemove", BYTEMOVE },
	{ "alert", ALERT },
	{ "returnvalue", RETURNVALUE },
	{ "resettelemetry", RESETTELEMETRY },
//configSwitch( "L3", "v<val", "batt", 73, "L2" )
//configSwitch( "L3", "AND", "L4", "L5", "L2" )
  { "", 0 } /* mark end of table */
} ;

// Items in openTx lua
//  { "drawFilledRectangle", luaLcdDrawFilledRectangle },
//  { "drawTimer", luaLcdDrawTimer },
//  { "drawChannel", luaLcdDrawChannel },
//  { "drawSwitch", luaLcdDrawSwitch },
//  { "drawSource", luaLcdDrawSource },
//  { "drawGauge", luaLcdDrawGauge },
//  { "drawScreenTitle", luaLcdDrawScreenTitle },
//  { "drawPixmap", luaLcdDrawPixmap },
//  { "getLastPos", luaLcdGetLastPos },
//  { "getDateTime", luaGetDateTime },
//  { "getVersion", luaGetVersion },
//  { "getGeneralSettings", luaGetGeneralSettings },
//  { "getRAS", luaGetRAS },
//  { "getFieldInfo", luaGetFieldInfo },
//  { "getFlightMode", luaGetFlightMode },
//  { "playFile", luaPlayFile },
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
//  { "loadScript", luaLoadScript },
//  { "setTelemetryValue", luaSetTelemetryValue },
//  { "getInfo", luaModelGetInfo },
//  { "setInfo", luaModelSetInfo },
//  { "getModule", luaModelGetModule },
//  { "setModule", luaModelSetModule },
//  { "getTimer", luaModelGetTimer },
//  { "setTimer", luaModelSetTimer },
//  { "resetTimer", luaModelResetTimer },
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


const struct commands Constants[] =
{
  { "LEFT", LEFT },
  { "PREC1", PREC1 },
  { "PREC2", PREC2 },
  { "LEAD0", LEADING0 },
  { "DBLSIZE", DBLSIZE },
  { "INVERS", INVERS },
  { "BLINK", BLINK },
	{ "CONDENSED", CONDENSED },
//  { "FULLSCALE", RESX },
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
  { "EVT_LEFT_REPT", EVT_KEY_REPT(KEY_LEFT) },
  { "EVT_RIGHT_REPT", EVT_KEY_REPT(KEY_RIGHT) },
  { "EVT_EXIT_LONG", EVT_KEY_LONG(KEY_EXIT) },
//  { "SOLID", SOLID },
//  { "DOTTED", DOTTED },
//  { "FORCE", FORCE },
//  { "ERASE", ERASE },
//  { "ROUND", ROUND },
  { "LCD_W", LCD_W },
  { "LCD_H", LCD_H },
#ifndef	QT
#endif  
	{ "", 0 } /* mark end of table */
} ;



int32_t FirstLabel ;
int32_t PreviousToken ;
uint8_t LoadIndex ;

// In program:
// Byte 
// LineNumber

#ifdef	QT
uint8_t *RunTimeData ;
uint8_t RunTimeBuffer[100] ;
uint8_t CodeBuffer[200] ;
#endif

#ifdef	QT
#define PROGRAM_SIZE	22000
#else
 #ifdef SMALL
#define PROGRAM_SIZE	10300
 #else
  #if defined(PCBX12D) || defined(PCBX10)
#define PROGRAM_SIZE	32000
  #else
#define PROGRAM_SIZE	22000
  #endif
 #endif
#endif

union t_program
{
	uint8_t Bytes[PROGRAM_SIZE] ;
	uint32_t Words[PROGRAM_SIZE/4] ;
} Program ;

uint32_t StartOfSymbols ;
uint32_t EndOfSymbols ;
uint32_t CurrentPosition ;
uint32_t CurrentVariableIndex ;
uint32_t CurrentArrayIndex ;

char *ProgPtr ;
char *Cur_token;
char Token[40] ;
char Numeric[10] ;
uint16_t Token_type ;
uint16_t Tok ;
uint16_t NgotoLocation ;


#ifdef	QT
FILE *File ;
//#else
//FIL File ;
#endif
#define LINE_LENGTH		160
char InputLine[LINE_LENGTH] ;

#ifdef	QT
#define DATA_SIZE	60000
uint8_t Data[DATA_SIZE] ;
uint8_t ErrorReport[100] ;
#endif

#ifndef	QT
uint8_t BasicErrorText[48] ;
#endif


#ifndef	QT
uint32_t strMatch( const char *a, const char *b, uint32_t length )
{
	uint8_t c ;
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
		c = *a++ ;
		if ( c == '\200' )
		{
			c = '^' ;
		}
		else if ( c == '\201' )
		{
			c = 'v' ;
		}
		if ( (char)c == *b++ )
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
#endif

uint32_t serror( uint32_t x )
{
	ParseError = x ;
	ParseErrorLine = LineNumber ;
	return 0 ;
}


// Get a number in a number base for a maximum # of digits
// (digits = 0 means no limit)
unsigned int get_number( int base, int digits )
{
	unsigned value ;
	char c ;

	value = 0 ;
	do
	{
		c = *ProgPtr ;
		if ( IsDigit( c ) )		/* convert numeric digits */
		{
			c -= '0' ;
		}
		else if(c >= 'a')				/* convert lower case alphabetics */
		{
			c -= ('a' - 10) ;
		}
		else if(c >= 'A')				/* convert upper case alphabetics */
		{
			c -= ('A' - 10);
		}
		else							/* not a valid "digit" */
		{
			if ( c == '.' )
			{
				serror(SE_NO_FLOAT) ;
			}
			break ;
		}
		if( c >= (char) base )					/* outside of base */
		{
			break ;
		}
		value = ( value * base ) + c ;		/* include in total */
		ProgPtr += 1 ;
	} while( --digits )	;					/* enforce maximum digits */
//	if ( ( *ProgPtr == 'L' ) || ( *ProgPtr == 'l' ) )
//	{
//		ProgPtr += 1 ;
//	}

	return value ;
}


int32_t asctoi( char *string )
{
	int32_t result = 0 ;
	uint32_t digit ;
	uint32_t base ;
	uint32_t sign = 0 ;

    /*
     * Skip any leading blanks.
     */

  while (isspace(*string))
	{
		string += 1;
  }

    /*
     * Check for a sign.
     */

	if (*string == '-')
	{
		sign = 1 ;
		string += 1 ;
  }
//	else
//	{
//		sign = 0 ;
//		if (*string == '+')
//		{
//	    string += 1;
//		}
//  }

	base = 10 ;
	if ( *string == '0' )
	{
		base = 8 ;
		string += 1 ;
		if ( *string == 'x' )
		{
			base = 16 ;
			string += 1 ;
		}
		else if ( *string == 'b' )
		{
			base = 2 ;
			string += 1 ;
		}
	}

  for ( ; ; string += 1)
	{
		digit = *string ;
		if ( IsDigit( digit ) )
		{
			digit -= '0' ;
		}
		else if ( digit >= 'a' )
		{
			digit -= ('a' - 10) ;
		}
		else if ( digit >= 'A' )
		{
			digit -= ('A' - 10) ;
		}
		else
		{
			break ;
		}

		if (digit > base)
		{
	    break ;
		}
		result = (base*result) + digit ;
  }

//  return result ;
  return sign ? - result : result ;
}

#ifndef	QT
uint8_t *utoasc( uint8_t *p, uint32_t value )
{
	uint8_t *q ;
  if (value >= 10000)
    p += 1 ;
  if (value >= 1000)
    p += 1 ;
  if (value >= 100)
    p += 1 ;
  if (value >= 10)
    p += 1 ;

	q = p + 1 ;
	do
	{
		*p-- = value % 10 + '0' ;
		value /= 10 ;
	} while ( value ) ;
	return q ;
}

void setErrorText( uint32_t error, uint32_t line )
{
	uint8_t *p ;
	p = BasicErrorText ;
	p = cpystr( p, (uint8_t *)"Error " ) ;
	p = utoasc( p, error ) ;
	p = cpystr( p, (uint8_t *)" at line " ) ;
	p = utoasc( p, line ) ;
//	p = cpystr( p, (uint8_t *)"\037" ) ;
//	utoasc( p, RunTime->RunLastLineNumber ) ;
	AlertType = ALERT_TYPE ;
	AlertMessage = "Script Error" ;
}

#endif

// Read special character (with translations)
//unsigned int read_special( char delim )
unsigned int read_special()
{
	char chr ;

	chr = *ProgPtr++ ;

//	if ( ( chr = *ProgPtr++ ) == delim )
//	{
////		String_continue = 0 ;
//		return 0 ;
//	}
//	if ( chr == '\\' )
//	{
		switch( chr )
		{
//			case '\0' :
				// Backslash at end of line
//				String_continue = 1 ;
//				ProgPtr -= 1 ;
//			break ;

			case 'n':
				/* newline */
				chr = 0x0a ;
			break ;

			case 'r':
				/* return */
				chr = 0x0d ;
			break ;

			case 't':
				/* tab */
				chr = 0x09 ;
			break ;

			case 'f' :
				/* formfeed */
				chr = 0x0c ;
			break ;

			case 'b':
				/* backspace */
				chr = 0x08 ;
			break ;

			case 'x' :
				/* hex value */
				chr = (char) get_number( 16, 2 ) ;
			break ;

			case '"' :
				chr = '"' ;
			break ;

			default:
				if( IsDigit( chr ) )
				{	/* octal value */
					ProgPtr -= 1 ;
					chr = (char) get_number( 8, 3 ) ;
				}
			break ;
		}
//	}
	return chr ;
}


int openFile( char *filename )
{
#ifdef	QT
	File = fopen( filename,"r") ;
	return File ? 1 : 0 ;
#else
	if ( f_open( &MultiBasicFile, filename, FA_READ) == FR_OK )
	{
		return 1 ;
	}
	return 0 ;
//	File = f_open( filename,"r") ;
#endif
}
 
void closeFile()
{
#ifdef	QT
	fclose( File ) ;
#else
	f_close( &MultiBasicFile ) ;
#endif
}

#ifdef	QT
uint8_t *cpystr( uint8_t *dest, uint8_t *source )
{
  while ( (*dest++ = *source++) )
    ;
  return dest - 1 ;
}
#endif

/* 
 * LOOKUP
 * Look up a token's internal representation in the command token table.
 */
uint32_t look_up( char *s, const struct commands *table )
{
  uint32_t i ;
//  char *p ;

//  /* convert to lowercase */
//  p = s;
//  while(*p)
//	{
//		*p = tolower(*p) ;
//		p++ ;
//	}

  /* see if token is in table */
  for( i=0 ; table->command[0] ; i += 1, table += 1 )
  {
    if(!strcmp(table->command, s) )
    {
      return table->tok + 0x80000 ;
    }
  }
  return 0 ; /* unknown command */
}


/* Return true if c is a delimiter. */
uint32_t isdelim(char c)
{
  if(strchr(" :;,+-<>/*%=#()&|[]^", c) || c==9 || c=='\n' || c=='\r' || c==0) // removed '^'
	{
    return 1 ;
	}
  return 0 ;
}

/* Return 1 if c is space or tab. */
uint32_t iswhite(char c)
{
  if(c==' ' || c=='\t') return 1;
  else return 0;
}

uint32_t runError( uint32_t x )
{
	if ( RunTime->RunError == 0 )	// Keep first error
	{
		RunTime->RunError = x ;
  	RunTime->RunErrorLine = RunTime->RunLineNumber ;
	}
	return 0 ;
}


uint32_t scanForKeyword( char *dest )
{
  char *temp ;
//  char bitbuff[20];	/* an extract from the main buffer */
//  char **funcp;
//  char match_quote ;

  Token_type = 0 ;
	Tok = 0 ;
  temp = dest ;

  if(*ProgPtr=='\0')
	{ // end of line
    *Token = 0 ;
    Tok = FINISHED ;
//    //printf( "FINISHED found!\n" );
    return( Token_type = DELIMITER ) ;
  }

//  /*
//   * show a snippet of the buffer ahead
//  strncpy( bitbuff, prog, 10 );
//  bitbuff[10] = '\0';
//  printf( "get_token(): examining \"%s...\": ", bitbuff );
//   */

  while(iswhite(*ProgPtr)) ProgPtr += 1 ;  /* skip over white space */

  if( *ProgPtr == '\r')
	{
		ProgPtr += 1 ;
	}

  if( *ProgPtr == '\n')
	{ /* newline */
    ProgPtr += 1 ;
    Tok = EOL;
//		*Token = '\n' ;
    Token[0]='\n';
		Token[1] = 0 ;
//    //printf( "NEWLINE\n" );
    return ( Token_type = DELIMITER ) ;
  }

  if(strchr("+-*/%=#;(),><&|]^", *ProgPtr))
	{ /* delimiter */
		char c ;
    c = *ProgPtr ;
		if ( c == '|' )
		{
			c = BITOR ;
		}
		if ( c == '^' )
		{
			c = BITXOR ;
		}
		if ( c == '/' )
		{
			if ( *(ProgPtr+1) == '/' )
			{
				// Rem
				ProgPtr += 2 ;
    		*temp++ = 'r' ;
    		*temp++ = 'e' ;
    		*temp++ = 'm' ;
    		*temp = '\0' ;
				Tok = look_up(Token, Table) ;
				if ( Tok )
				{
					Token_type = COMMAND ; /* is a command */
				  return Token_type ;
				}
			}
		}
    *temp = c ;
    ProgPtr += 1 ; /* advance to next position */
		if ( ( c == '<' ) || ( c == '>') )
		{
			if ( *ProgPtr == '=' )
			{
    		ProgPtr += 1 ; /* advance to next position */
				*temp = ( c == '<' ) ? LESSEQUAL : GREATEQUAL ;				
			}
		}


    temp += 1 ;
    *temp = 0 ; 
//    //printf( "DELIM\n" );
    return ( Token_type = DELIMITER ) ;
  }
	if ( ( *ProgPtr == '=' ) && ( *(ProgPtr+1) == '=' ) )
	{
		*temp++ = '=' ;
    *temp = 0 ; 
		ProgPtr += 2 ;
    return ( Token_type = DELIMITER ) ;
	}
	if ( ( *ProgPtr == '!' ) && ( *(ProgPtr+1) == '=' ) )
	{
		*temp++ = HASHCHAR ;
    *temp = 0 ; 
		ProgPtr += 2 ;
    return ( Token_type = DELIMITER ) ;
	}
    
//  /*
//   * now that we've skipped spaces and delimiters, take a copy of *prog
//   * for improved error reporting
//   */
  Cur_token = ProgPtr ;

  if(*ProgPtr == '"' ) // || *ProgPtr == '\'')
	{ /* quoted string */
//    match_quote = *ProgPtr ;
    ProgPtr += 1 ;
		temp += 1 ;
    while( *ProgPtr != '"' && *ProgPtr !='\n')
		{
			if ( *ProgPtr == '\\' )
			{
				ProgPtr += 1 ;
//				*temp++ = read_special( '"' ) ;
				*temp++ = read_special() ;
			}
			else
			{
				*temp++ = *ProgPtr++ ;
			}
		}
    if(*ProgPtr == '\n')
		{
			serror(SE_SYNTAX) ;
		}
    ProgPtr += 1 ;
		*temp = 0 ;
		if ( temp-Token > 254 )
		{
    	serror(SE_STRING_LONG) ;
		}
		Token[0] = temp - Token ;
//    //printf( "QUOTE\n" );
    return( Token_type = QUOTE ) ;
  }

	if(*ProgPtr == '\'' )
	{
		// char constant
		uint8_t chr ;
		uint8_t ch1 ;
    ProgPtr += 1 ;
		chr = *ProgPtr++ ;
		if ( chr == '\\' )
		{
			chr = read_special() ;
		}
		if ( *ProgPtr++ != '\'' )
		{
			serror(SE_SYNTAX) ;
		}
		ch1 = chr ;
		if ( chr > 99 )
		{
			*temp++ = chr / 100 + '0' ;
			chr %= 100 ;
		}
		if ( ( chr > 9 ) || (ch1 > 99) )
		{
			*temp++ = chr / 10 + '0' ;
			chr %= 10 ;
		}
		*temp++ = chr + '0' ;
    *temp = '\0';
    return(Token_type = NUMBER);
	}
  
  if(IsDigit(*ProgPtr))
	{ /* number */
    while(!isdelim(*ProgPtr)) *temp++ = *ProgPtr++ ;
    *temp = '\0';
//    //printf( "NUMBER\n" );
    return(Token_type = NUMBER);
  }

  if(isalpha(*ProgPtr))// || *ProgPtr == '@') 
  { 
    /* var or command */
    while(!isdelim(*ProgPtr)) 
       *temp++ = *ProgPtr++ ;
    Token_type = STRING;
  }

  *temp = '\0' ;

  /* see if a string is a command or a variable */
  if(Token_type==STRING)
  {
//    /* can we find this string in the command table? */
//    //printf( "checking the command table for \"%s\"\n", token );

		uint32_t constant ;	 
		if ( Token[0] >= 'A' && Token[0] <= 'Z' )
		{
			constant = look_up(Token, Constants) ;
			Tok = 0 ;
			if ( constant & 0x80000 )
			{
				Token[0] = constant ;
				Token[1] = constant >> 8 ;
		    return( Token_type = CONSTANT ) ;
			}
		}
		else
		{
			Tok = look_up(Token, Table) ;
		}
    if(!Tok) 
    {
			/* no.  Is it an array? */
			if ( *ProgPtr == '[' )
			{
      	Token_type = ARRAY ;
				ProgPtr += 1 ;
			}
      else
      {
        /* no.  Is it a function? */
//		  for( funcp=functions; *funcp && strcasecmp( token, *funcp ); funcp++ )
//	     ;
//		  if( *funcp == (char *)0 )
//	    {
//	     /* no. must be a variable */
				Token_type = VARIABLE ;
				if ( *ProgPtr == ':' )
				{
         	Token_type = LABEL ;
					ProgPtr += 1 ;
				}
				if ( *ProgPtr == '(' )
				{
         	Token_type = FUNCTION ;
					ProgPtr += 1 ;
				}
				if ( ( *ProgPtr == ' ' ) || (*ProgPtr == 9 ) )
				{
					if ( *(ProgPtr+1) == '(' )
					{
        	 	Token_type = FUNCTION ;
						ProgPtr += 2 ;
					}
				}
				if ( Cur_token == InputLine )
				{
				  if ( ( *ProgPtr == '\r') || ( *ProgPtr == '\n') || ( *ProgPtr == '\0') )
					{
	         	Token_type = LABEL ;
					}
				}
		  }
		}
    else 
		{
			Token_type = COMMAND ; /* is a command */
		}
  }
	else
	{
    serror(SE_SYNTAX) ;
	}
//  //printf( "%d\n", token_type );
  return Token_type ;
}


#ifdef QT
char Tbuf[20] ;

uint8_t *appendSymbols( uint8_t *p )
{
	uint8_t *r ;
	uint32_t value ;
	r = &Program.Bytes[StartOfSymbols] ;
  p = cpystr( p, (uint8_t *)"\nSymbols\n" ) ;
	while ( r < &Program.Bytes[EndOfSymbols] )
	{
    uint8_t *q = r ;
    q += *r++ ;
		switch ( *r++ )
		{
			case SYM_LABEL :
// Label: EntryLen, SYM_LABEL, StrLen, String/0, SYM_LAB_DEF/REF, Poslow, Poshi
        p = cpystr( p, (uint8_t *)"Sym:" ) ;
        p = cpystr( p, (uint8_t *)r+1 ) ;
				r += *r + 1 ;
				if ( *r++ == SYM_LAB_DEF )
				{
          p = cpystr( p, (uint8_t *)",DEF," ) ;
				}
				else
				{
          p = cpystr( p, (uint8_t *)",REF," ) ;
				}
				value = *r++ ;
				value += *r << 8 ;
        sprintf( Tbuf, "%d", value ) ;
        p = cpystr( p, (uint8_t *)Tbuf ) ;
				*p++ = '\n' ;
			break ;
			case SYM_VARIABLE :
			break ;
			case SYM_FUNCTION :
			break ;
		}
		r = q ;
		if ( p > &Data[DATA_SIZE-200] )
		{
			break ;
		}
	}
	r = &Program.Bytes[StartOfSymbols] ;
	uint32_t i ;
	uint32_t j ;
	j = 0 ;
	if ( p < &Data[DATA_SIZE-1000] )
	{
		for ( i = 0 ; i < 192 ; i += 1 )
		{
  	  sprintf( Tbuf, "%02x ", *r++ ) ;
  	  p = cpystr( p, (uint8_t *)Tbuf ) ;
			if ( ++j > 15 )
			{
				j = 0 ;
				*p++ = '\n' ;
			}
		}
	}
  p = cpystr( p, (uint8_t *)"\nCode:\n" ) ;
	r = &Program.Bytes[0] ;
	j = 0 ;
	if ( p < &Data[DATA_SIZE-9000] )
  for ( i = 0 ; i < 1280+512 ; i += 1 )
	{
		if ( j == 0 )
		{
    	sprintf( Tbuf, "%04x: ", i ) ;
    	p = cpystr( p, (uint8_t *)Tbuf ) ;
		}
    sprintf( Tbuf, "%02x ", *r++ ) ;
    p = cpystr( p, (uint8_t *)Tbuf ) ;
		if ( ++j > 15 )
		{
			j = 0 ;
			*p++ = '\n' ;
		}
	}
	return p ;
}
#endif

int32_t findSymbol( uint8_t type )
{
	uint8_t *tstart ;
//	uint32_t tindex ;
//	uint32_t i ;

	tstart = &Program.Bytes[StartOfSymbols] ;
	while ( tstart < &Program.Bytes[EndOfSymbols] )
	{
    if ( !strcmp( Token, (char *)(tstart+3) ) )
		{
			if ( *(tstart+1) == type )
			{
      	return tstart - Program.Bytes ; // [StartOfSymbols] ;
			}
		}
		tstart += *tstart ;
	}
	return -1 ;
}

void addSymbol( uint8_t type, uint8_t sub_type, uint16_t value, uint16_t dimension )
{
	uint32_t tindex ;
	uint32_t tstart ;
	uint32_t i ;
	uint32_t end ;
	
	end = EndOfSymbols ;
	tstart = end++ ;
	Program.Bytes[end++] = type ;
	tindex = end++ ;
	i = 0 ;
	while ( Token[i] )
	{
		Program.Bytes[end++] = Token[i++] ;
	}
	Program.Bytes[tindex] = end - tindex ;
	Program.Bytes[end++] = '\0' ;
	if ( type != SYM_CONST )
	{
		Program.Bytes[end++] = sub_type ;
		Program.Bytes[end++] = value ;
		Program.Bytes[end++] = value >> 8 ;
		if ( sub_type & SYM_VAR_ARRAY_TYPE )
		{
			Program.Bytes[end++] = dimension ;
//			Program.Bytes[end++] = dimension >> 8 ;
		}
	}
	else
	{
		Program.Bytes[end++] = value ;
		Program.Bytes[end++] = value >> 8 ;
		Program.Bytes[end++] = dimension  ;
		Program.Bytes[end++] = dimension  >> 8 ;
	}
	Program.Bytes[tstart] = end - tstart ;
	EndOfSymbols = end ;
}

#ifdef QT	
uint32_t loadBasic( char *fileName, uint32_t type ) ;
uint32_t basicTask( uint8_t event, uint8_t flags ) ;

// Bitfields in ScriptFlags
#define	SCRIPT_LCD_OK					1
#define	SCRIPT_STANDALONE			2
#define	SCRIPT_TELEMETRY			4
#define	SCRIPT_RESUME					8
#define	SCRIPT_BACKGROUND			16
#define	SCRIPT_FRSKY					32
#define	SCRIPT_ROTARY					64

void scriptReleasetBt(void)
{

}


uint32_t parse( char *filename )
{
	uint32_t i ;
	uint32_t retValue ;
	ErrorReport[0] = '\0' ;
	loadBasic( filename, BASIC_LOAD_ALONE ) ;
	for ( i = 0 ; i < 300 ; i += 1 )
	{
		retValue = basicTask( 0, SCRIPT_STANDALONE | SCRIPT_LCD_OK ) ;
		if ( ( retValue == 3 ) || ( retValue == 4 ) )
		{
			scriptReleasetBt() ;			
			break ;
		}
	}
	return retValue ;
}	
	
uint32_t run( uint32_t event )
{	
	uint32_t i ;
	uint32_t retValue ;
	for ( i = 0 ; i < 300 ; i += 1 )
	{
		retValue = basicTask( event,  SCRIPT_STANDALONE | SCRIPT_LCD_OK ) ;
		if ( ( retValue == 2 ) || ( retValue == 3 ) )
		{
			break ;
		}
		if ( retValue == 1 )
		{
			event = 0 ;
		}

	}
	return retValue ;
}

void report()
{
	uint32_t i ;
  uint32_t j ;
  uint8_t *p ;
  p = RunTimeData ;
	if ( p )
	{
		uint8_t *r ;
		if (RunTime)
		{
			p = cpystr( p, (uint8_t *)"Debug Data\n" ) ;

	  	r = (uint8_t *) &(RunTime->Vars.Variables) ;
			j = 0 ;
			for ( i = 0 ; i < 96 ; i += 1 )
			{
  		  sprintf( Tbuf, "%02x ", *r++ ) ;
  		  p = cpystr( p, (uint8_t *)Tbuf ) ;
				if ( ++j > 15 )
				{
					j = 0 ;
					*p++ = '\n' ;
				}
				if ( p > &Data[DATA_SIZE-200] )
				{
					break ;
				}
			}
			*p++ = '\n' ;
  		*p = '\0' ;
		}
	}
//  r = (uint8_t *) &RunTime->Vars ;
//	for ( i = 0 ; i < 48 ; i += 1 )
//	{
//    sprintf( Tbuf, "%02x ", *r++ ) ;
//    p = cpystr( p, (uint8_t *)Tbuf ) ;
//		if ( ++j > 15 )
//		{
//			j = 0 ;
//			*p++ = '\n' ;
//		}
//	}
}
#endif

uint8_t BasicState ;
#ifdef QT	
uint8_t *QtPtr ;
#endif

char LastBasicFname[60] ;


// locate where to load
uint32_t findStartOffset()
{
	uint32_t position = 0 ;
	uint32_t i ;
	uint32_t x ;
	
	for ( i = 0 ; i <= 2 ; i += 1 )
	{
		if ( LoadedScripts[i].loaded )
		{
			x = LoadedScripts[i].offsetOfStart ;
			x += LoadedScripts[i].size ;
			if ( x > position )
			{
				position = x ;
			}
		}
	}
	return position ;
}


uint32_t loadBasic( char *fileName, uint32_t type )
{
	if ( type == BASIC_LOAD_ALONE )
	{
		LoadedScripts[0].loaded = 0 ;
		LoadedScripts[1].loaded = 0 ;
		LoadedScripts[2].loaded = 0 ;
//		LoadedScripts[0].offsetOfStart = 0 ;
		LoadedScripts[0].type = type ;
		LoadIndex = 0 ;
	}
	else if ( type == BASIC_LOAD_BG )
	{
		LoadedScripts[0].loaded = 0 ;
		LoadedScripts[0].type = type ;
		LoadIndex = 0 ;
	}
	else if ( type == BASIC_LOAD_TEL0 )
	{
		LoadedScripts[1].loaded = 0 ;
		LoadedScripts[1].type = type ;
		LoadIndex = 1 ;
	}
	else if ( type == BASIC_LOAD_TEL1 )
	{
		LoadedScripts[2].loaded = 0 ;
		LoadedScripts[2].type = type ;
		LoadIndex = 2 ;
	}
	
	CurrentPosition = findStartOffset() ;
	
	// temp
//	CurrentPosition = 0 ;
	
	LoadedScripts[LoadIndex].offsetOfStart = CurrentPosition ;
	CurrentPosition += 8 ;
	
	StartOfSymbols = CurrentPosition + 92 ;
	EndOfSymbols = StartOfSymbols ;
	FirstLabel = -1 ;
	PreviousToken = -1 ;
	CurrentArrayIndex = 0 ;
	CurrentVariableIndex = 0 ;
	LineNumber = 0 ;

	cpystr( (uint8_t *)LastBasicFname, (uint8_t *)fileName ) ;
  if ( openFile( fileName ) )
	{
    cpystr( (uint8_t *)Token, (uint8_t *)"Event" ) ;
    addSymbol( SYM_VARIABLE, SYM_VAR_INT, CurrentVariableIndex++, 0 ) ;
		
    cpystr( (uint8_t *)Token, (uint8_t *)"Size" ) ;
    addSymbol( SYM_VARIABLE, SYM_VAR_INT, CurrentVariableIndex++, 0 ) ;
		
#ifndef	QT
		BasicErrorText[0] = 0 ;
#endif
		BasicState = BASIC_LOADING ;
#ifdef QT	
		Data[0] = 0 ;
		QtPtr = Data ;
#else
		BasicLoadedType = type ;
#endif
		return 1 ;		// Loading started

	}
	else
	{
#ifndef QT	
		BasicState = BASIC_IDLE ;
		BasicLoadedType = BASIC_LOAD_NONE ;
#endif
		return 0 ;		// Didn't load
	}
}

void setJumpAddress( uint32_t offset, uint32_t value )
{
	Program.Bytes[offset] = value ;
	Program.Bytes[offset+1] = value >> 8 ;
}


void setLinkedJumpAddress( uint32_t offset, uint32_t value )
{
	uint32_t index ;
  do
	{
		index = Program.Bytes[offset] ;
		index += Program.Bytes[offset+1] << 8 ;
		Program.Bytes[offset++] = value ;
		Program.Bytes[offset] = value >> 8 ;
		offset = index ;
	} while ( index ) ;
}


uint16_t codeNumeric( int32_t value )
{
	if ( value >=0 && value <=7 )
	{
		return 0x48 + value ;
	}
	else if ( value >= -128 && value <= 127 )
	{
		return 0x0158 ;
	}
	else if ( value >= -32768 && value <= 32767 )
	{
		return 0x0240 ;
	}
	return 0x0450 ;
}

uint32_t setArray( uint32_t index, uint32_t cPosition )
{
	uint32_t dimension ;
	uint32_t type ;
	uint8_t code ;
	// Found
	index += Program.Bytes[index] - 3 ; // Index of index
	type = Program.Bytes[index-1] ;
	dimension = Program.Bytes[index+2] ;
	index = Program.Bytes[index] | (Program.Bytes[index+1] << 8) ;
	
//	index += Program.Bytes[index] - 4 ; // Index of index
//	type = Program.Bytes[index-1] ;
//	dimension = Program.Bytes[index+2] ;
//	dimension |= Program.Bytes[index+3] << 8 ;
//	index = Program.Bytes[index] | (Program.Bytes[index+1] << 8) ;
	
	code = 0x64 ;
	if ( type == SYM_VAR_ARRAY_BYTE )
	{
		code = 0x66 ;
	}
	if ( index > 255 )
	{
		code |= 0x08 ;
	}

//	if ( dimension > 255 )
//	{
//		code |= 0x01 ;
//	}
	
	Program.Bytes[cPosition++] = code ;
	Program.Bytes[cPosition++] = index ;
	if ( code &= 0x08 )
	{
		Program.Bytes[cPosition++] = index >> 8 ;
	}
	Program.Bytes[cPosition++] = dimension ;
//	if ( code & 1 )
//	{
//		Program.Bytes[cPosition++] = dimension >> 8 ;
//	}
	return cPosition ;
}

uint32_t setNgotoLocation(uint32_t cPosition )
{
//	Program.Bytes[cPosition++] = NGOTO ;
//	NgotoLocation = cPosition ;
//	Program.Bytes[cPosition++] = 0 ;
//	Program.Bytes[cPosition++] = 0 ;
	
	uint32_t index ;
	Program.Bytes[cPosition++] = NGOTO ;
  index = NgotoLocation ;
	NgotoLocation = cPosition ;
	Program.Bytes[cPosition++] = index ;
	Program.Bytes[cPosition++] = index >> 8 ;
	return cPosition ;
}


// return  values:
// 0 error
// 1 loading
// 2 loaded

uint32_t partLoadBasic()
{
#ifdef QT	
	uint8_t *p ;
#endif
	uint8_t *q ;
	uint32_t i ;
	uint32_t j ;
	uint32_t processWhile ;
	uint32_t processIf ;
	uint32_t cPosition ;
	uint32_t lineNumberPosition ;

	cPosition = CurrentPosition ;
	lineNumberPosition = cPosition ;
#ifdef QT	
	p = QtPtr ;
#endif

	q = 0 ;
	for ( i = 0 ; i < 200 ; i += 1 )
	{
		j = StartOfSymbols - cPosition ;
		if ( j < 200 )
		{
			j = 300 - j ;
      uint8_t *ps = &Program.Bytes[EndOfSymbols] ;
			uint8_t *pd = ps + j ;
			if ( pd >= &Program.Bytes[PROGRAM_SIZE] )
			{
				serror( SE_TOO_LARGE ) ;
				break ;
			}
			while ( ps >= &Program.Bytes[StartOfSymbols] )
			{
				*pd-- = *ps-- ;
			}
			StartOfSymbols += j ;
			EndOfSymbols += j ;
		}
#ifdef QT
    q = (uint8_t *)fgets( InputLine, LINE_LENGTH, File ) ;
#else
    q = (uint8_t *)f_gets( InputLine, LINE_LENGTH, &MultiBasicFile ) ;
#endif
		LineNumber += 1 ;
		if ( lineNumberPosition == cPosition - 2 )
		{
			cPosition -= 2 ; // Remove line number if whole line is a REM
		}
		lineNumberPosition = cPosition ;
		Program.Bytes[cPosition++] = ( LineNumber & 0x7F ) | 0x80 ;
		Program.Bytes[cPosition++] = LineNumber >> 7 ;
		processWhile = 0 ;
		processIf = 0 ;
		if ( q )
		{
#ifdef QT
			if ( p < &Data[DATA_SIZE-500] )
			{
    		sprintf( Tbuf, "%04d: ", LineNumber ) ;
        p = cpystr( p, (uint8_t *)Tbuf ) ;
				uint8_t *r ;
				uint8_t c ;
				r = q ;
				do
				{
					c = *r++ ;
					if ( c == 9 )
					{
						c = ' ' ;
					}
					*p++ = c ;
				} while (c) ;
				p -= 1 ;
			}
#endif
      ProgPtr = (char *) q ;
			do
			{
				j = scanForKeyword( Token ) ;
				if ( Tok )
				{
					if ( PreviousToken == THEN )
					{
						if ( Tok != GOTO && Tok != GOSUB )
					  {
							// Must be statement, put in NGOTO
							cPosition = setNgotoLocation( cPosition ) ;
						}
					}
					
					if ( Tok == REM )
					{
//#ifdef QT
//            p = cpystr( p, (uint8_t *)"Rem" ) ;
//#endif
						if ( lineNumberPosition == cPosition - 2 )
						{
							cPosition -= 2 ; // Remove line number if whole line is a REM
						}
						break ;
					}
					else if ( Tok == LET )
					{
//#ifdef QT
//            p = cpystr( p, (uint8_t *)"Let" ) ;
//#endif
						continue ;
					}
					else if ( Tok == DEFARRAY )
					{
						uint32_t type = SYM_VAR_ARRAY_INT ;
//#ifdef QT
//            p = cpystr( p, (uint8_t *)"Array " ) ;
//#endif
						cPosition -= 2 ; // Remove line number
						// What is array type
						j = scanForKeyword( Token ) ;
						if ( Tok == DEFBYTE )
						{
							type = SYM_VAR_ARRAY_BYTE ;
							j = scanForKeyword( Token ) ;
						}
						if ( Tok == DEFINT )
						{
							j = scanForKeyword( Token ) ;
						}
						if ( j != ARRAY )
						{
							serror( SE_SYNTAX ) ;
						}
           	else
						{
							j = scanForKeyword( Numeric ) ;
							if ( j != NUMBER )
							{
								serror( SE_SYNTAX ) ;
							}
							else
							{
								int32_t value ;	
								if ( *ProgPtr != ']' )
								{
									serror( SE_SYNTAX ) ;
								}
								ProgPtr += 1 ;
								value = asctoi( Numeric ) ;
                int32_t limit ;
								limit = 100 ;
                if ( type == SYM_VAR_ARRAY_BYTE )
								{
									limit = 252 ;
								}
								if ( value >= 0 && value <= limit )
								{
									uint32_t index ;
									uint32_t allocWords ;
									index = CurrentArrayIndex ;	// Offset in words
									allocWords = value ;
                  if ( type == SYM_VAR_ARRAY_BYTE )
									{
										allocWords = ( value + 3 ) / 4 ;
										index *= 4 ;
									}
									CurrentArrayIndex += allocWords ;
									// Allocate array space
									addSymbol( SYM_ARRAY, type, index, value ) ;
								}
								else
								{
									serror( SE_TOO_LARGE ) ;
								}
							}
						}
						continue ;
					}
					else if ( Tok == DEFCONST )
					{
						uint32_t sign = 0 ;
						j = scanForKeyword( Token ) ;
						j = scanForKeyword( Numeric ) ;
						if ( j == DELIMITER )
						{
							if ( Numeric[0] == '-' )
							{
								sign = 1 ;
								j = scanForKeyword( Numeric ) ;
							}
						}
						if ( j != NUMBER )
						{
							serror( SE_SYNTAX ) ;
						}
						else
						{
							int32_t value ;	
							ProgPtr += 1 ;
							value = asctoi( Numeric ) ;
							if ( sign )
							{
								value = -value ;
							}
							addSymbol( SYM_CONST, 0, value, value >> 16 ) ;
							cPosition -= 2 ;		// Remove line number
						}
					}
					else if ( Tok == WHILE )
					{
						processWhile = 1 ;
						CodeControl[ControlIndex].type = TYPE_WHILE ;
						CodeControl[ControlIndex].first = cPosition - 2 ;
					}
					else if ( Tok == IF )
					{
						CodeControl[ControlIndex].type = TYPE_IF ;
						CodeControl[ControlIndex].middle = 0 ;
						processIf = 1 ;
					}
					else if ( Tok == ELSEIF )
					{
						if ( ControlIndex )
						{
							ControlIndex -= 1 ;
							if ( CodeControl[ControlIndex].type == TYPE_IF )
							{
								uint32_t index ;
								processIf = 2 ;
								Program.Bytes[cPosition++] = GOTO ;
	//              CodeControl[ControlIndex].type = TYPE_ELSE ;
            	  index = CodeControl[ControlIndex].middle ;
								CodeControl[ControlIndex].middle = cPosition ;
								Program.Bytes[cPosition++] = index ;
            	  Program.Bytes[cPosition++] = index >> 8 ;
	//							Program.Bytes[cPosition++] = 0 ;
	//              Program.Bytes[cPosition++] = 0 ;
								setJumpAddress( CodeControl[ControlIndex].first, cPosition ) ;
            	  ControlIndex += 1 ;
								Program.Bytes[cPosition++] = ( LineNumber & 0x7F ) | 0x80 ;
								Program.Bytes[cPosition++] = LineNumber >> 7 ;
							}
							else
							{
								serror( SE_SYNTAX ) ;
							}
						}
						else
						{
							serror( SE_SYNTAX ) ;
						}
					}
					else if ( Tok == THEN )
					{
						processIf = 0 ;
					}
					else if ( Tok == ELSE )
					{
						if ( ControlIndex )
						{
							ControlIndex -= 1 ;
							if ( CodeControl[ControlIndex].type == TYPE_IF )
							{
								uint32_t index ;
								Program.Bytes[cPosition++] = GOTO ;
            	  CodeControl[ControlIndex].type = TYPE_ELSE ;
            	  index = CodeControl[ControlIndex].middle ;
								CodeControl[ControlIndex].middle = cPosition ;
								Program.Bytes[cPosition++] = index ;
            	  Program.Bytes[cPosition++] = index >> 8 ;
	//							Program.Bytes[cPosition++] = 0 ;
	//              Program.Bytes[cPosition++] = 0 ;
								setJumpAddress( CodeControl[ControlIndex].first, cPosition ) ;
								ControlIndex += 1 ;
								Tok = 0 ;
							}
							else
							{
								serror( SE_SYNTAX ) ;
							}
						}
						else
						{
							serror( SE_SYNTAX ) ;
						}
					}
					else if ( Tok == FUNC )
					{
						// defining a user function
						j = scanForKeyword( Token ) ;
						if ( j != FUNCTION )
						{
							serror( SE_SYNTAX ) ;
						}
           	addSymbol( SYM_FUNCTION, SYM_LAB_DEF, cPosition, 0 ) ;
						j = scanForKeyword( Token ) ;
						if ( ( j != DELIMITER ) && (Token[0] != ')' ) )
						{
							serror( SE_SYNTAX ) ;
						}
						// found a function
					}
					else if ( Tok == END )
					{
						if ( ControlIndex )
						{
							ControlIndex -= 1 ;
							if ( CodeControl[ControlIndex].type == TYPE_WHILE )
							{
								Program.Bytes[cPosition++] = GOTO ;
								Program.Bytes[cPosition++] = CodeControl[ControlIndex].first ;
                Program.Bytes[cPosition++] = CodeControl[ControlIndex].first >> 8 ;
								setLinkedJumpAddress( CodeControl[ControlIndex].middle, cPosition ) ;
							}
							else if ( CodeControl[ControlIndex].type == TYPE_IF )
							{
								cPosition -= 2 ;		// Remove line number
								setJumpAddress( CodeControl[ControlIndex].first, cPosition ) ;
								if ( CodeControl[ControlIndex].middle )
								{
									setLinkedJumpAddress( CodeControl[ControlIndex].middle, cPosition ) ;
								}
							}
							else if ( CodeControl[ControlIndex].type == TYPE_ELSE )
							{
								cPosition -= 2 ;		// Remove line number
								setLinkedJumpAddress( CodeControl[ControlIndex].middle, cPosition ) ;
							}
							Tok = 0 ;
						}
						continue ;
					}
					else if ( Tok == BREAK )
					{
						uint8_t copyIndex ;
						copyIndex = ControlIndex ;
						while ( ControlIndex )
						{
							ControlIndex -= 1 ;
							if ( CodeControl[ControlIndex].type == TYPE_WHILE )
							{
								uint32_t index ;
								Program.Bytes[cPosition++] = GOTO ;
            	  index = CodeControl[ControlIndex].middle ;
								CodeControl[ControlIndex].middle = cPosition ;
								Program.Bytes[cPosition++] = index ;
            	  Program.Bytes[cPosition++] = index >> 8 ;
                break ;
							}
							else
							{
								if( ControlIndex == 0 )
								{
									serror(SE_NO_WHILE) ;
								}
							}
						}
						ControlIndex = copyIndex ;
						Tok = 0 ;
					}

					if ( ( Tok != EOL ) && ( Tok != FINISHED ) )
					{
//#ifdef QT
//						*p++ = '[' ;
//						sprintf( Tbuf, "%d,%d", j, Tok ) ;
//            q = (uint8_t *)Tbuf ;
//						while ( *q )
//						{
//							*p++ = *q++ ;
//						}
//						*p++ = ']' ;
//#endif
						if ( Tok )
						{
							Program.Bytes[cPosition++] = Tok ;
						}
					}
					else
					{
						if ( NgotoLocation )
						{
							setLinkedJumpAddress( NgotoLocation, cPosition ) ;
//							setJumpAddress( NgotoLocation, cPosition ) ;
							NgotoLocation = 0 ;
						}
						if ( processWhile )
						{
							Program.Bytes[cPosition++] = NGOTO ;
							CodeControl[ControlIndex].middle = cPosition ;
							Program.Bytes[cPosition++] = 0 ;
							Program.Bytes[cPosition++] = 0 ;
							ControlIndex += 1 ;
							processWhile = 0 ;
						}
						if ( processIf )
						{
							// Didn't see a "then"
              if (processIf == 2 )	// Doing elseif
							{
              	ControlIndex -= 1 ;
							}
							Program.Bytes[cPosition++] = THEN ;
							Program.Bytes[cPosition++] = NGOTO ;
							CodeControl[ControlIndex].first = cPosition ;
							Program.Bytes[cPosition++] = 0 ;
							Program.Bytes[cPosition++] = 0 ;
              ControlIndex += 1 ;
							processIf = 0 ;
						}
#ifdef QT
						uint32_t i ;
						for ( i = 0 ; i < 200 ; i += 1 )
						{
              CodeBuffer[i] = Program.Bytes[i] ;
						}
#endif
					}
					PreviousToken = Tok ;
				}
				else
				{
//#ifdef QT
//					*p++ = '{' ;
//					sprintf( Tbuf, "%d,", j ) ;
//          q = (uint8_t *)Tbuf ;
//					while ( *q )
//					{
//						*p++ = *q++ ;
//					}
//#endif
					if ( PreviousToken == THEN )
					{
//						if ( Tok != GOTO && Tok != GOSUB )
//					  {
							// Must be statement, put in NGOTO
							cPosition = setNgotoLocation( cPosition ) ;
//						}
					}
          switch ( j )
					{
						case VARIABLE :
						{	
							int32_t loc ;
							uint32_t type ;
							uint32_t index ;
							if ( ( PreviousToken == GOTO ) || ( PreviousToken == GOSUB ) )
							{
//#ifdef QT
//	              p = cpystr( p, (uint8_t *)"Label " ) ;
//                p = cpystr( p, (uint8_t *)Token ) ;
//#endif
								loc = findSymbol( SYM_LABEL ) ;
                if ( loc == -1 )
								{
                	addSymbol( SYM_LABEL, SYM_LAB_REF, cPosition, 0 ) ;
									Program.Bytes[cPosition++] = 0 ;
									Program.Bytes[cPosition++] = 0 ;
								}
								else
								{
									loc += Program.Bytes[loc] - 3 ; // Index of type
									type = Program.Bytes[loc] ;
									loc += 1 ;
									if ( type == SYM_LAB_REF )
									{
										index = Program.Bytes[loc] ;
										index += Program.Bytes[loc+1] << 8 ;

										Program.Bytes[loc++] = cPosition ;
										Program.Bytes[loc] = cPosition >> 8 ;
										Program.Bytes[cPosition++] = index ;
										Program.Bytes[cPosition++] = index >> 8 ;
									}
									else
									{
										Program.Bytes[cPosition++] = Program.Bytes[loc++] ;
										Program.Bytes[cPosition++] = Program.Bytes[loc] ;
									}
								}
							}
							else
							{
								int32_t index ;
								uint8_t code ;
								uint8_t constant = 0 ;
//#ifdef QT
//	              p = cpystr( p, (uint8_t *)"Var " ) ;
//								*p++ = Token[0] ;
//#endif
								index = findSymbol( SYM_ARRAY ) ;	// Is it an array with no subscript?
								if ( index != -1 )
								{
									// Found
									cPosition = setArray( index, cPosition ) ;
									Program.Bytes[cPosition++] = 0x48 ;	// number 0
									Program.Bytes[cPosition++] = ']' ;
									break ;
								}
								index = findSymbol( SYM_VARIABLE ) ;
								if ( index == -1 )
								{
									index = findSymbol( SYM_CONST ) ;
									if ( index == -1 )
									{
//										if ( CurrentVariableIndex >= MAX_VARIABLES )
//										{
//    	                serror( SE_TOO_MANY_VARS ) ;
//										}
										index = CurrentVariableIndex++ ;
                		addSymbol( SYM_VARIABLE, SYM_VAR_INT, index, 0 ) ;
									}
									else
									{
										// Found a constant
										int32_t value ;
                    uint32_t bytes ;
                    index += Program.Bytes[index] - 4 ; // Index of value
										value = Program.Bytes[index++] ;
										value |= Program.Bytes[index++] << 8 ;
										value |= Program.Bytes[index++] << 16 ;
										value |= Program.Bytes[index++] << 24 ;
							
										bytes = codeNumeric( value ) ;
										code = bytes ;
										bytes >>= 8 ;
										
//										if ( value < 0 )
//										{
//											code = 0x50 ;
//											bytes = 4 ;
//										}
//										else
//										{
//											if ( value <=7 )
//											{
//              				  code = 0x48 + value ;
//											}
//											else if ( value <= 127 )
//											{
//												code = 0x58 ;
//												bytes = 1 ;
//											}
//											else if ( value <= 32767 )
//											{
//												code = 0x40 ;
//												bytes = 2 ;
//											}
//											else
//											{
//												code = 0x50 ;
//												bytes = 4 ;
//											}
//										}
										Program.Bytes[cPosition++] = code ;
										while ( bytes-- )
										{
											Program.Bytes[cPosition++] = value ;
											value >>= 8 ;
										}
										constant = 1 ;
									}
								}
								else
								{
									index += Program.Bytes[index] - 2 ; // Index of index
									index = Program.Bytes[index] | (Program.Bytes[index+1] << 8) ;
								}
								if ( constant == 0 )
								{
									code = 0x60 ;
									if ( index > 255 )
									{
										code = 0x68 ;
									}
									Program.Bytes[cPosition++] = code ;
									Program.Bytes[cPosition++] = index ;
									if ( code == 0x68 )
									{
										Program.Bytes[cPosition++] = index >> 8 ;
									}
								}	
							}
						}
						break ;

						case ARRAY :
						{	
							int32_t index ;
							index = findSymbol( SYM_ARRAY ) ;
							if ( index == -1 )
							{
								serror( SE_SYNTAX ) ;
							}
							else
							{
								cPosition = setArray( index, cPosition ) ;
							}
						}	
						break ;
							 
						case CONSTANT :
						{	
							uint8_t code ;
							uint16_t value ;	
							uint32_t bytes = 0 ;
							value = Token[0] ;
							value |= Token[1] << 8 ;
//#ifdef QT
//              p = cpystr( p, (uint8_t *)"Const " ) ;
//							sprintf( Tbuf, "%d,", value ) ;
//              p = cpystr( p, (uint8_t *) Tbuf ) ;
//#endif
							bytes = codeNumeric( value ) ;
							code = bytes ;
							bytes >>= 8 ;
//							if ( value <=7 )
//							{
//                code = 0x48 + value ;
//							}
//							else if ( value <= 127 )
//							{
//								code = 0x58 ;
//								bytes = 1 ;
//							}
//							else if ( value <= 32767 )
//							{
//								code = 0x40 ;
//								bytes = 2 ;
//							}
//							else
//							{
//								code = 0x50 ;
//								bytes = 4 ;
//							}
							Program.Bytes[cPosition++] = code ;
							while ( bytes-- )
							{
								Program.Bytes[cPosition++] = value ;
								value >>= 8 ;
							}
						}	
						break ;

						case NUMBER :
						{
							int32_t value ;	
							uint8_t code ;
							uint32_t bytes ;
							value = asctoi( Token ) ;
//#ifdef QT
//              p = cpystr( p, (uint8_t *)"Num " ) ;
//              p = cpystr( p, (uint8_t *)Token ) ;
//#endif
							bytes = codeNumeric( value ) ;
							code = bytes ;
							bytes >>= 8 ;
							Program.Bytes[cPosition++] = code ;
							while ( bytes-- )
							{
								Program.Bytes[cPosition++] = value ;
								value >>= 8 ;
							}
						}
						break ;
						case DELIMITER :
						{	
//#ifdef QT
//              p = cpystr( p, (uint8_t *)"Delim " ) ;
//							if ( Token[0] == LESSEQUAL )
//							{
//								*p++ = '<' ;
//								*p++ = '=' ;
//							}
//							else if ( Token[0] == GREATEQUAL )
//							{
//								*p++ = '>' ;
//								*p++ = '=' ;
//							}
//							else
//							{
//								*p++ = Token[0] ;
//							}
//#endif
							Program.Bytes[cPosition++] = Token[0] ;
						}
						break ;
						case LABEL :
						{	
							int32_t loc ;
//							uint32_t index ;
							uint32_t type ;
//#ifdef QT
//              p = cpystr( p, (uint8_t *)"Label " ) ;
//              p = cpystr( p, (uint8_t *)Token ) ;
//							// Define a label
//#endif
							loc = findSymbol( SYM_LABEL ) ;
							if ( loc == -1 )
							{
                addSymbol( SYM_LABEL, SYM_LAB_DEF, cPosition, 0 ) ;
							}
							else
							{
								loc += Program.Bytes[loc] - 3 ; // Index of type
								type = Program.Bytes[loc] ;
								if ( type == SYM_LAB_DEF )
								{
									serror( SE_DUP_LABEL ) ; // Duplicate label
								}
								else
								{
									// A ref, these need updating
									Program.Bytes[loc] = SYM_LAB_DEF ;
                  loc += 1 ;
									setLinkedJumpAddress( loc, cPosition ) ;
								}
							}
						}
						break ;
						case QUOTE :	// Quoted string
						{
							uint8_t *temp ;
              uint8_t c ;
							uint32_t count ;
              temp = (uint8_t *)Token ;
							Program.Bytes[cPosition++] = 0x70 ;
							count = *temp + 1 ;
							do
							{
								c = *temp++ ;
								Program.Bytes[cPosition++] = c ;
							} while ( --count ) ;
						}	
						if ( *ProgPtr == '[' )
						{
							Program.Bytes[cPosition++] = '[' ;
							ProgPtr += 1 ;
						}
						break ;
						case FUNCTION :
						{
							uint32_t inFunction ;
							inFunction = look_up( Token, InternalFunctions ) ;
							if ( inFunction )
							{
								Program.Bytes[cPosition++] = IN_FUNCTION ;
								Program.Bytes[cPosition++] = inFunction ;
							}
							else
							{
								// No user functions (yet)
								int32_t loc ;
								uint32_t type ;
								loc = findSymbol( SYM_FUNCTION ) ;
                if ( loc != -1 )
								{
									loc += Program.Bytes[loc] - 3 ; // Index of type
									type = Program.Bytes[loc] ;
									loc += 1 ;
									if ( type == SYM_LAB_DEF )
									{
										Program.Bytes[cPosition++] = USER_FUNCTION ;
										Program.Bytes[cPosition++] = Program.Bytes[loc++] ;
										Program.Bytes[cPosition++] = Program.Bytes[loc] ;
									}
									else
									{								
										serror( SE_NO_FUNCTION ) ;
									}
								}
								else
								{								
									serror( SE_NO_FUNCTION ) ;
								}
							}
						}			 
						break ;
					}
//#ifdef QT
//					*p++ = '}' ;
//#endif
          PreviousToken = -1 ;
        }
				if ( ParseError )
				{
#ifdef QT
					sprintf( InputLine, "Error %d at line %d %s", ParseError, ParseErrorLine, ErrorText[ParseError-1] ) ;
					*p++ = '\n' ;
					p = cpystr( p, (uint8_t *)InputLine ) ;
					cpystr( ErrorReport, (uint8_t *)InputLine ) ;
#else
					setErrorText( ParseError, ParseErrorLine ) ;
#endif
					break ;
				}
			} while( Tok != FINISHED ) ;
//#ifdef QT
//			*p++ = '\n' ;
//#endif
		}
		else
		{
			break ;
		}
		if ( ParseError )
		{
			break ;
		}
	}
	CurrentPosition = cPosition ;
	if ( ParseError )
	{
		closeFile() ;
		return 0 ;
	}
	if ( q )
	{
#ifdef QT	
		QtPtr = p ;
#endif
		return 1 ;
	}

	if ( ControlIndex )
	{
		serror( SE_MISSING_END ) ;
#ifdef QT
		sprintf( InputLine, "Error %d at line %d %s", ParseError, ParseErrorLine, ErrorText[ParseError-1] ) ;
		*p++ = '\n' ;
		p = cpystr( p, (uint8_t *)InputLine ) ;
		cpystr( ErrorReport, (uint8_t *)InputLine ) ;
#else
		setErrorText( ParseError, ParseErrorLine ) ;
#endif
		closeFile() ;
		return 0 ;
	}
#define SE_MISSING_END		16
	Program.Bytes[CurrentPosition++] = STOP ;
	closeFile() ;
	 
	CurrentPosition += 3 ;
	CurrentPosition /= 4 ;	// Rounded word offset
	i = LoadedScripts[LoadIndex].offsetOfStart / 4 ;
	Program.Words[i] = CurrentPosition ;
	j = CurrentPosition*4 + sizeof(struct t_basicRunTime) - (MAX_VARIABLES*4) + (CurrentVariableIndex*4) + CurrentArrayIndex * 4 ;
	Program.Words[i+1] = j ;
	LoadedScripts[LoadIndex].size = j ;

	RunTime = (struct t_basicRunTime *) &Program.Words[CurrentPosition] ;
	RunTime->IntArrayStart = &RunTime->Vars.Variables[CurrentVariableIndex] ;
	RunTime->ByteArrayStart = &RunTime->Vars.byteArray[CurrentVariableIndex*4] ;

#ifdef QT
	p = appendSymbols( p ) ;
	RunTimeData = p ;
#endif

	i = basicExecute( 1, 0, LoadIndex ) ;

#ifdef QT
	p = RunTimeData ;
	if ( i )
	{
    cpystr( p, (uint8_t *)InputLine ) ;
	}
	uint8_t *r ;
  r = (uint8_t *) RunTime->Vars.Variables ;
	j = 0 ;
	for ( i = 0 ; i < 96 ; i += 1 )
	{
    sprintf( Tbuf, "%02x ", *r++ ) ;
    p = cpystr( p, (uint8_t *)Tbuf ) ;
		if ( ++j > 15 )
		{
			j = 0 ;
			*p++ = '\n' ;
		}
	}
	*p++ = '\n' ;
//  r = (uint8_t *) &RunTime->Arrays ;
//	for ( i = 0 ; i < 48 ; i += 1 )
//	{
//    sprintf( Tbuf, "%02x ", *r++ ) ;
//    p = cpystr( p, (uint8_t *)Tbuf ) ;
//		if ( ++j > 15 )
//		{
//			j = 0 ;
//			*p++ = '\n' ;
//		}
//	}
#endif
	return 2 ;
}



#ifndef QT

void setScriptFilename( uint8_t *filepath, uint8_t *name )
{
	uint32_t i ;
	
	filepath = cpystr( filepath, (uint8_t *)"/SCRIPTS/" ) ;
	filepath = cpystr( filepath, ( LoadingIndex == 0 ) ? (uint8_t *)"MODEL/" : (uint8_t *)"TELEMETRY/" ) ;
	
	for ( i = 0 ; i < 6 ; i += 1 )
	{
		if ( ( *name == ' ' ) || ( *name == '\0' ) )
		{
			break ;
		}
		*filepath++ = *name++ ;
	}
	cpystr( filepath, (uint8_t *)".BAS" ) ;
}

void loadNextScript()
{
	uint8_t *pindex ;
	uint32_t type ;
	uint8_t scriptFilename[60] ;
	
	type = BASIC_LOAD_BG ;
	while ( LoadingIndex < 3 )
	{
		LoadedScripts[LoadingIndex].loaded = 0 ;
		pindex = (uint8_t*)"" ;
		
		switch ( LoadingIndex )
		{
			case 0 :
				pindex = g_model.backgroundScript ;
			break ;
			
			case 1 :
				type = BASIC_LOAD_TEL0 ;
				if ( g_model.customDisplay1Extra[6] )
				{
					pindex = g_model.customDisplayIndex ;
				}
			break ;
			
			case 2 :
				type = BASIC_LOAD_TEL1 ;
				if ( g_model.customDisplay2Extra[6] )
				{
					pindex = g_model.customDisplay2Index ;
				}
			break ;
		}
		
		if ( !( ( *pindex == ' ' ) || ( *pindex == '\0' ) ) )
		{
		// We have a filename
			setScriptFilename( scriptFilename, pindex ) ;
			if ( loadBasic( (char *) scriptFilename, type ) )
			{
				return ;
			}
		}
		LoadingIndex += 1 ;
	} 
}



void basicLoadModelScripts()
{
	LoadingIndex = 0 ;
	BasicState = BASIC_IDLE ;	// Terminate existing script(s)
#ifndef QT
//	if ( !SdMounted )
	if ( !sd_card_ready() )
	{
		LoadingNeeded = 1 ;
		return ;
	}
#endif
	loadNextScript() ;
}
#endif



//#ifndef QT
//void xbasicLoadModelScripts()
//{
//	uint8_t *pindex ;
////	uint8_t *q ;
////	uint32_t i ;
//	uint32_t type ;
//	uint8_t scriptFilename[60] ;

//	BasicState = BASIC_IDLE ;	// Terminate existing script(s)
//	BasicLoadedType = BASIC_LOAD_NONE ;

//	// First 
//	type = BASIC_LOAD_BG ;
//	pindex = g_model.backgroundScript ;
//	if ( ( *pindex == ' ' ) || ( *pindex == '\0' ) )
//	{
//		pindex = g_model.customDisplayIndex ;
//		type = BASIC_LOAD_TEL0 ;
//		if ( g_model.customDisplay1Extra[6] == 0 )
//		{		
//			if ( g_model.customDisplay2Extra[6] == 0 )
//			{	
//	//			return false ;
//				return ;
//			}
//			else
//			{
//				pindex = g_model.customDisplay2Index ;
//				type = BASIC_LOAD_TEL1 ;
//			}
//		}
//	}
//	if ( ( *pindex == ' ' ) || ( *pindex == '\0' ) )
//	{
//		return ;
////		return false ;
//	}
//	// We have a filename
//	setScriptFilename( scriptFilename, pindex ) ;
//	loadBasic( (char *) scriptFilename, type ) ;
//	return ;

////extern uint32_t mainScreenDisplaying( void ) ;
////  uint8_t view = g_model.mview & 0xf;
//// 	uint8_t tview = g_model.mview & 0x70 ;
////	if ( !mainScreenDisplaying() )
////	{
////		view = 0 ; // Not 4!
////	}



////  if ( (view == 4) && (tview <= 0x10) )
	
//}
//#endif

// Return values:
// 0 No script running
// 1 After begin or Still running or RunError
// 2 Found Stop
// 3 Script finished (unloaded) or Loading needed
// 4 (For QT) Loading complete

uint32_t basicTask( uint8_t event, uint8_t flags )
{
	uint32_t result ;
	ScriptFlags = flags ;

	if ( flags & SCRIPT_BACKGROUND )
	{
		if ( BasicState == BASIC_RUNNING )
		{
			if ( BasicLoadedType != BASIC_LOAD_ALONE )
			{
				if ( LoadedScripts[0].loaded )
				{
					return basicExecute( 0, 0, 0 ) ;
				}
			}
		}
	}

	if ( BasicState == BASIC_LOADING )
	{
		result = partLoadBasic() ;

		if ( result == 0 )
		{
			// Parse Error
#ifndef QT
			if ( BasicLoadedType == BASIC_LOAD_ALONE )
			{
				BasicState = BASIC_IDLE ;
				BasicLoadedType = BASIC_LOAD_NONE ;
				LoadedScripts[0].loaded = 0 ;
				basicLoadModelScripts() ;
			}
			else
			{
				LoadingIndex += 1 ;
				loadNextScript() ;
			}
#endif
			return 0 ;
		}
		else if ( result == 2 )
		{
			BasicState = BASIC_RUNNING ;
#ifndef QT
			LoadedScripts[LoadingIndex].loaded = 1 ;
			if ( BasicLoadedType != BASIC_LOAD_ALONE )
			{
				if ( LoadingIndex < 3 )
				{
					LoadingIndex += 1 ;
					loadNextScript() ;
				}
			}
#else
			return 4 ;
#endif

//#ifndef QT
//			return basicExecute( 1, ( BasicLoadedType == BASIC_LOAD_BG ) ? 0 : event ) ;
//#else
//			return basicExecute( 1, 0 ) ;
//#endif
		}
//		return 1 ;
		return 0 ;
	}
	else if ( BasicState == BASIC_RUNNING )
	{
#ifndef QT
		if ( flags & SCRIPT_TELEMETRY )
		{
//			return 0 ;
extern uint32_t mainScreenDisplaying( void ) ;
		  uint8_t view = g_model.mview & 0xf;
  		uint8_t tview = g_model.mview & 0x70 ;
			uint32_t nav = 0 ;
			uint32_t lcd = 0 ;
			if ( mainScreenDisplaying() )
			{
				if ( view == 4 )
				{
					if ( ( tview == 0 ) && ( LoadedScripts[1].loaded ))
					{
						lcd = 1 ;
						nav = 1 ;
					}
					else
					{
						if ( ( tview == 0x10 ) && ( LoadedScripts[2].loaded ) )
						{
							lcd |= 2 ;
							nav = 1 ;
						}
					}
					if ( nav )
					{
extern void navigateCustomTelemetry(uint8_t event, uint32_t mode ) ;
						navigateCustomTelemetry( event, 1 ) ;
//						return basicExecute( 0, event ) ;		
					}
				}
			}

			uint32_t running = 0 ;
			uint32_t retvalue ;
			retvalue = 0 ;
//			if ( LoadedScripts[0].loaded )
//			{
//				retvalue = basicExecute( 0, 0, 0 ) ;
//			}
			if ( LoadedScripts[1].loaded )
			{
				if ( lcd & 1 )
				{
					ScriptFlags |= SCRIPT_LCD_OK ;
				}			 
				retvalue = basicExecute( 0, event, 1 ) ;
				if ( retvalue != 3 )
				{
					running = 1 ;
				}
				ScriptFlags &= ~SCRIPT_LCD_OK ;
			}
			if ( LoadedScripts[2].loaded )
			{
				if ( lcd & 2 )
				{
					ScriptFlags |= SCRIPT_LCD_OK ;
				}			 
				retvalue = basicExecute( 0, event, 2 ) ;
				if ( retvalue != 3 )
				{
					running |= 2 ;
				}
			}
			if ( running )
			{
				if ( nav )
				{
					return retvalue ;
				}
				else
				{
					return 0 ;
				}
			}	
		}
#endif
#ifndef QT
		if ( flags & SCRIPT_STANDALONE )
		{
			if ( BasicLoadedType == BASIC_LOAD_ALONE )
			{
//				if ( LoadedScripts[0].loaded )
//				{
				return basicExecute( 0, event, 0 ) ;
//				}
			}
		}
#else
		return basicExecute( 0, event, 0 ) ;
#endif
	}
	else if ( BasicState == BASIC_IDLE )
	{
#ifndef QT
		if ( LoadingNeeded )
		{
			LoadingNeeded = 0 ;
			return 3 ;
		}
#endif
	}
  return 0 ;
}


uint32_t getVarIndex( uint8_t opcode )
{
	// ToDo Handle arrays
	uint32_t index ;
	index = *RunTime->ExecProgPtr++ ;	// Index to dest variable
//	if ( opcode & 0x04 )
//	{ // An array
//	}	
	if ( opcode & 0x08 )
	{
		index |= *RunTime->ExecProgPtr++ << 8 ;	// Index to dest variable
	}
	return index ;
}

int32_t getInteger( uint8_t opcode )
{
	int32_t value ;		
	int16_t val16 ;
	uint8_t *execPtr ;

	execPtr = RunTime->ExecProgPtr ;
	value = opcode & 7 ;
	opcode &= 0xF8 ;
  if ( opcode == 0x58 )
	{
		value = (int8_t) *execPtr++ ;
	}
	else if ( opcode == 0x40 )
	{
		val16 = *execPtr++ ;
		val16 |= *execPtr++ << 8 ;	// Index to dest variable
		value = val16 ;	// sign extend
  }
	else if ( opcode == 0x50 )
	{
		value = *execPtr++ ;
		value |= *execPtr++ << 8 ;	// Index to dest variable
		value |= *execPtr++ << 16 ;	// Index to dest variable
		value |= *execPtr++ << 24 ;	// Index to dest variable
	}


	RunTime->ExecProgPtr = execPtr ;
	return value ;
}

// Add- level1:  &, |
// level2: (comparison operators) #, =, <, >, <=, >=
// level3: +, -
// level4: *, /, %
// level5: unary +, -
// level6: functions
// level7: parenthesis

int32_t getPrimitive( uint8_t opcode )	// From variable or number
{
	int32_t value ;
	uint16_t val16 ;

	if ( ( opcode & 0xE0 ) == 0x40 )	// A number
	{
		value = getInteger( opcode ) ;
	}
	else
	{
		if ( ( opcode & 0xF0 ) == 0x60 )	// A variable
		{
			if ( opcode & 0x04 )
			{ // An array
				uint32_t destType = 0 ;
				uint32_t dimension ;
				if ( opcode & 0x02 )
				{
					destType |= 1 ;		// Byte array
				}
				val16 = getVarIndex( opcode ) ;
				dimension = *RunTime->ExecProgPtr++ ;
				
//				if ( opcode & 0x01 )	// 16-bit dimension
//				{
//					dimension |= *RunTime->ExecProgPtr++ << 8 ;
//				}

				value = expression() ;
				opcode = *RunTime->ExecProgPtr++ ;
				if ( opcode != ']' )
				{
					return runError( SE_SYNTAX ) ;
				}
				if ( (uint32_t) value >= dimension )
				{
					return runError( SE_DIMENSION ) ;
				}
				val16 += value ;
				if ( destType & 1 )
				{
					value = RunTime->ByteArrayStart[val16] ;
				}
				else
				{
					value = RunTime->IntArrayStart[val16] ;
				}
			}
			else
			{
				if ( opcode == 0x61 )
				{
					uint32_t index ;
					index = *RunTime->ExecProgPtr++ ;	// Index to dest variable
					value = RunTime->ParameterStack[RunTime->ParamFrameIndex + index].var ;
				}
				else
				{
					val16 = getVarIndex( opcode ) ;
					value = RunTime->Vars.Variables[val16] ;
				}
			}
		}
		else
		{
			return runError( SE_SYNTAX ) ;
		}
	}
	return value ;
}


// return values:
// 0 error
// 1 byte
// 2 int

// size is for byte array with that much space
uint32_t getParamVarAddress( union t_parameter *ptr, uint32_t size )
{
	uint8_t opcode ;
	uint32_t result = 0 ;
  uint32_t value ;
	uint16_t val16 ;
	
	// Note that ipointer and bpointer occupy the same space so
	// this clears bpointer as well
	ptr->ipointer = 0 ;

	opcode = *RunTime->ExecProgPtr++ ;
	if ( ( opcode & 0xF0 ) == 0x60 )	// A variable
	{
		if ( opcode & 0x04 )
		{ // An array
			uint32_t destType = 0 ;
			uint32_t dimension ;
			if ( opcode & 0x02 )
			{
				destType |= 1 ;
			}
			val16 = getVarIndex( opcode ) ;
			dimension = *RunTime->ExecProgPtr++ ;

//				if ( opcode & 0x01 )	// 16-bit dimension
//				{
//					dimension |= *RunTime->ExecProgPtr++ << 8 ;
//				}

			value = expression() ;
			opcode = *RunTime->ExecProgPtr++ ;
			if ( opcode != ']' )
			{
				runError( SE_SYNTAX ) ;
				return 0 ;
			}
			val16 += value ;
			if ( value + size > dimension )
			{
				runError( SE_DIMENSION ) ;
				return 0 ;
			}
			if ( destType & 1 )
			{
				ptr->bpointer = &RunTime->ByteArrayStart[val16] ;
				result = 1 ;
			}
			else
			{
        ptr->ipointer = &RunTime->IntArrayStart[val16] ;
				result = 2 ;
			}
		}
		else
		{
			val16 = getVarIndex( opcode ) ;
			ptr->ipointer = &RunTime->Vars.Variables[val16] ;
			result = 2 ;
		}
	}
	else
	{
		runError( SE_SYNTAX ) ;
	}
	if ( *RunTime->ExecProgPtr == ',' )
	{
		RunTime->ExecProgPtr += 1 ;
	}
	return result ;
}

//struct byteAddress
//{
//	uint16_t varOffset ;
//	uint16_t varIndex ;
//	uint32_t dimension ;
//} ;

// Assumes opcode is 0x66, a byte array
// Needs opcode to handle 16-bit dimensions
uint32_t getParamByteArrayAddress( struct byteAddress *ptr, uint32_t size )
{
	uint8_t opcode ;
		
	ptr->varOffset = getVarIndex( 0x66 ) ;
	ptr->dimension = *RunTime->ExecProgPtr++ ;
	ptr->varIndex = expression() ;
	opcode = *RunTime->ExecProgPtr++ ;
	if ( opcode != ']' )
	{
		runError( SE_SYNTAX ) ;
		return 0 ;
	}
	if ( ( ptr->varIndex >= ptr->dimension ) || 
			( ptr->varIndex + size >= ptr->dimension ) )
	{
		runError( SE_DIMENSION ) ;
		return 0 ; ;
	}
	if ( *RunTime->ExecProgPtr == ',' )
	{
		RunTime->ExecProgPtr += 1 ;
	}
	return 1 ;
}

/* Perform the specified arithmetic. */
void arith( uint8_t op, int32_t *r, int32_t *h )
{
  switch(op)
	{
    case '-':
      *r = *r-*h ;
    break ;
    case '+':
      *r = *r+*h; 
    break; 
    case '*':  
      *r = *r * *h; 
    break; 
    case '/':
			if ( *h == 0 )
			{
				runError( SE_DIV_ZERO ) ;
				*h = 1 ;
			}
      *r = (*r)/(*h);
    break; 
    case '%':
			if ( *h == 0 )
			{
				runError( SE_DIV_ZERO ) ;
				*h = 1 ;
			}
      *r = (*r)%(*h); 
    break; 
//    case '^':
//      ex = *r; 
//      if(*h==0) {
//        *r = 1; 
//        break; 
//      }
//      for(t=*h-1; t>0; --t) *r = (*r) * ex;
//    break;       
  }
}

/* Perform the specified logic. */
void logic( uint8_t op, int32_t *r, int32_t *h )
{
  switch(op)
	{
    case BITAND :
      *r = *r & *h ;
    break ;
    case BITOR :
      *r = *r | *h; 
    break ; 
    case BITXOR :
      *r = *r ^ *h; 
    break ; 
  }
}


void compare( uint8_t op, int32_t *left, int32_t *right )
{
	switch( op )
	{
   	case '=':
      *left = (*left == *right) ;
    break ;
   	case '#':
      *left = (*left != *right) ;
    break ;
   	case '<':
      *left = (*left < *right) ;
    break ;
   	case '>':
      *left = (*left > *right) ;
    break ;
   case LESSEQUAL :
      *left = (*left <= *right);
      break;
   case GREATEQUAL :
      *left = (*left >= *right);
      break;
	}
	return ;
}


void level1( int32_t *result ) ;
void level2( int32_t *result ) ;

/* Process parenthesized expression. */
void level7( int32_t *result )
{
	uint8_t opcode ;
	opcode = *RunTime->ExecProgPtr++ ;
	
	if ( opcode == '(' )
	{
    level1( result ) ;
		opcode = *RunTime->ExecProgPtr++ ;
		{
			if ( opcode != ')' )
			{
				runError( SE_NO_BRACKET ) ;
			}
		}
	}
	else
	{
		*result = getPrimitive( opcode ) ;
	}
}

int32_t execInFunction( void ) ;
//int32_t execUserFunction( void ) ;

/* Is it a function or array? */
void level6( int32_t *result )
{
	uint8_t opcode ;
	// Currently not implemented
	opcode = *RunTime->ExecProgPtr ;
	if ( opcode == IN_FUNCTION )
	{
		RunTime->ExecProgPtr += 1 ;
    *result = execInFunction() ;
		return ;
	}
//	else if ( opcode == USER_FUNCTION )
//	{
//		uint32_t destination ;
//		RunTime->ExecProgPtr += 1 ;
//    *result = execUserFunction() ;
//		return ;
//	}

	level7( result) ;
//   char f[16];

//   //printf( "L6: token=\"%s\"\n", token );
//   if( strcasecmp( token, "rnd" ) == 0 ||
//       strcasecmp( token, "abs" ) == 0 ||
//       *token == '@' )
//   {
//      strncpy( f, token, sizeof( f )-1 );
//      get_token();
//   }
//   else
//      strcpy( f, "" );

//   level7( result, isassign );
//   if( strlen( f ) )
//   {
//      //printf( "L6: envaluating function/array %s(%d)\n", f, *result );
//      if( strcasecmp( f, "rnd" ) == 0 )
//         *result = (rand() % *result)+1;
//      else if( strcasecmp( f, "abs" ) == 0 )
//         *result = abs( *result );
//      else if( *f == '@' )
//         arrayRetrieve( *result, result );

//      //printf( "L6 returning.  *token->'%c'\n", *token );
//      return;
//   }
}

/* Is a unary + or -. */
void level5( int32_t *result )
{
  uint8_t op ;
	uint8_t opcode ;

  op = 0 ;
	opcode = *RunTime->ExecProgPtr ;

  if( opcode == '+' || opcode == '-' )
	{
    op = opcode ;
    RunTime->ExecProgPtr += 1 ;
  }
  level6( result ) ; 
  if(op == '-' )
	{
		*result = - (*result) ;
	}
}

/* Multiply or divide two factors. */
void level4( int32_t *result )
{
  uint8_t opcode ;
  int32_t hold ;

  level5( result ) ;
	
	opcode = *RunTime->ExecProgPtr++ ;

  while( opcode == '*' || opcode == '/' || opcode == '%')
	{
    level5( &hold ); 
    arith( opcode, result, &hold ) ;
		opcode = *RunTime->ExecProgPtr++ ;
  }
	RunTime->ExecProgPtr -= 1 ;
}

/*  Add or subtract two terms. */
void level3( int32_t *result )
{
  uint8_t opcode ;
  int32_t hold ;
	
	level4( result) ;
	opcode = *RunTime->ExecProgPtr++ ;
  while( opcode == '+' || opcode == '-' )
	{
    level4( &hold ) ; 
    arith( opcode, result, &hold ) ;
		opcode = *RunTime->ExecProgPtr++ ;
	}
	RunTime->ExecProgPtr -= 1 ;
}

/* L2: comparison operators */
void level2( int32_t *result )
{
  uint8_t opcode ;
  int32_t hold ;
	
	level3( result) ;

	opcode = *RunTime->ExecProgPtr++ ;
  while( opcode == '#' || opcode == '=' || opcode == '<' || opcode == '>' || opcode == LESSEQUAL || opcode == GREATEQUAL )
	{
    level3( &hold ) ; 
    compare( opcode, result, &hold ) ;
		opcode = *RunTime->ExecProgPtr++ ;
	}
	RunTime->ExecProgPtr -= 1 ;
}

void level1( int32_t *result )
{
  uint8_t opcode ;
  int32_t hold ;
	
	level2( result) ;
	opcode = *RunTime->ExecProgPtr++ ;
  while( opcode == BITAND || opcode == BITOR || opcode == BITXOR )
	{
    level2( &hold ) ; 
    logic( opcode, result, &hold ) ;
		opcode = *RunTime->ExecProgPtr++ ;
	}
	RunTime->ExecProgPtr -= 1 ;
}


int32_t expression()
{
	uint8_t opcode ;
	int32_t value ;

	opcode = *RunTime->ExecProgPtr ;
	if ( opcode & 0x80 )	// LineNumber
	{
    return runError( SE_SYNTAX ) ;
	}
	
	level1( &value ) ;

	return value ;
}

void exec_while()
{
	int32_t test ;
	uint8_t opcode ;
	uint32_t destination ;

	test = expression() ;
	
	opcode = *RunTime->ExecProgPtr++ ;
	if ( opcode != NGOTO )
	{
		runError( SE_SYNTAX ) ;
	}
	destination = *RunTime-> ExecProgPtr++ ;
  destination |= *RunTime->ExecProgPtr++ << 8 ;
	if ( test == 0 )
	{
		RunTime->ExecProgPtr = &Program.Bytes[destination] ;
	}
}

void exec_if()
{
	int32_t test ;
	uint8_t opcode ;
	uint32_t destination ;
	
	test = expression() ;

	opcode = *RunTime->ExecProgPtr++ ;
	if ( opcode != THEN )
	{
		runError( SE_NO_THEN ) ;
	}
	opcode = *RunTime->ExecProgPtr++ ;

	if ( opcode == GOTO || opcode == GOSUB || opcode == NGOTO )
	{
		destination = *RunTime-> ExecProgPtr++ ;
    destination |= *RunTime->ExecProgPtr++ << 8 ;
		if ( opcode == NGOTO )
		{
			if ( !test )
			{
				RunTime->ExecProgPtr = &Program.Bytes[destination] ;
			}
		}
		else
		{
			if ( ( *RunTime->ExecProgPtr & 0x80 ) == 0 )
			{
				runError( SE_SYNTAX ) ;
			}
		 
			if ( test )
			{
				if ( opcode == GOSUB )
				{
					if ( RunTime->CallIndex >= MAX_CALL_STACK )
					{
						runError( SE_TOO_MANY_CALLS ) ;
					}
					RunTime->CallStack[RunTime->CallIndex++] = RunTime->ExecProgPtr ;
				}
				RunTime->ExecProgPtr = &Program.Bytes[destination] ;
			}
		}
	}
	else
	{
		runError( SE_SYNTAX ) ;
	}
}

void exec_goto()
{
	uint32_t destination ;
	destination = *RunTime->ExecProgPtr++ ;
	destination |= *RunTime->ExecProgPtr++ << 8 ;
	
	if ( ( *RunTime->ExecProgPtr & 0x80 ) == 0 )
	{
		runError( SE_SYNTAX ) ;
	}
	RunTime->ExecProgPtr = &Program.Bytes[destination] ;
}

void exec_gosub()
{
	uint32_t destination ;
	destination = *RunTime->ExecProgPtr++ ;
	destination |= *RunTime->ExecProgPtr++ << 8 ;
	
	if ( ( *RunTime->ExecProgPtr & 0x80 ) == 0 )
	{
    runError( SE_SYNTAX ) ;
	}
	if ( RunTime->CallIndex >= MAX_CALL_STACK )
	{
		runError( SE_TOO_MANY_CALLS ) ;
	}
	RunTime->CallStack[RunTime->CallIndex++] = RunTime->ExecProgPtr ;
	RunTime->ExecProgPtr = &Program.Bytes[destination] ;
}

void exec_return()
{
	if ( RunTime->CallIndex )
	{
		if ( ( *RunTime->ExecProgPtr & 0x80 ) == 0 )
		{
			// must be return <value>
			RunTime->FunctionReturnValue = expression() ;
		}
		RunTime->ExecProgPtr = RunTime->CallStack[--RunTime->CallIndex] ;
	}
	else
	{
    runError( SE_NO_GOSUB ) ;
	}
}

uint32_t stringSubscript( uint32_t len )
{
	uint32_t t = 0 ;
	if ( *RunTime->ExecProgPtr == '[' )
	{
    RunTime->ExecProgPtr += 1 ;
		t = expression() ;
		if ( t >= len )
		{
			runError( SE_DIMENSION ) ;
			return 0 ;
		}
		if ( *RunTime->ExecProgPtr != ']' )
		{
			runError( SE_SYNTAX ) ;
			return 0 ;
		}
    RunTime->ExecProgPtr += 1 ;
	}
	return t ;
}


// Input type:
// 0 - number
// 1 - string/byte array address
// 2 - string/bytearray/number

// Return values
// 0 error
// 1 number
// 2 string
uint32_t get_parameter( union t_parameter *param, uint32_t type )
{
	uint8_t opcode ;

	if ( type != PARAM_TYPE_NUMBER )
	{
		// string
		opcode = *RunTime->ExecProgPtr ;
		if ( opcode == 0x70 )	// Quoted string
		{
			uint32_t len ;
			RunTime->ExecProgPtr += 1 ;
			len = *RunTime->ExecProgPtr++ ;
			param->cpointer = RunTime->ExecProgPtr ;
      RunTime->ExecProgPtr += len ;	// Skip the string
			uint32_t t = stringSubscript( len ) ;
			param->cpointer += t ;
			if ( *RunTime->ExecProgPtr == ',' )
			{
				RunTime->ExecProgPtr += 1 ;
			}
			return 2 ;
		}
		else if( ( opcode & 0xF6 ) == 0x66 )	// A byte array
		{
			struct byteAddress baddr ;
			RunTime->ExecProgPtr += 1 ;
			if ( getParamByteArrayAddress( &baddr, 0 ) == 0 )
			{
				return 0 ;
			}
			
//			uint32_t dimension ;
//			uint16_t val16 ;
//			uint32_t value ;
//			val16 = getVarIndex( opcode ) ;
//			dimension = *RunTime->ExecProgPtr++ ;
//			value = expression() ;
//			opcode = *RunTime->ExecProgPtr++ ;
//			if ( opcode != ']' )
//			{
//				runError( SE_SYNTAX ) ;
//				return 0 ;
//			}
//			if ( value >= dimension )
//			{
//				runError( SE_DIMENSION ) ;
//				return 0 ; ;
//			}
			param->cpointer = &RunTime->ByteArrayStart[baddr.varOffset + baddr.varIndex] ;
			if ( *RunTime->ExecProgPtr == ',' )
			{
				RunTime->ExecProgPtr += 1 ;
			}
			return 2 ;
		}
		else
		{
			if ( type == PARAM_TYPE_EITHER )
			{
				type = PARAM_TYPE_NUMBER ;
			}
			else
			{
				runError( SE_SYNTAX ) ;
				return 0 ;
			}
		}
	}
	if ( type == PARAM_TYPE_NUMBER )
	{
		param->var = expression() ;
		if ( RunTime->RunError )
		{
			return 0 ;
		}
		if ( *RunTime->ExecProgPtr == ',' )
		{
			RunTime->ExecProgPtr += 1 ;
		}
		return 1 ;
	}
  return 0 ;
}

uint32_t get_numeric_parameters( union t_parameter *param, uint32_t count )
{
	uint32_t result ;
	while ( count )
	{
		result = get_parameter( param, PARAM_TYPE_NUMBER ) ;
		if ( result != 1 )
		{
			return 0 ;
		}
		param += 1 ;
		count -= 1 ;		
	}
	return 1 ;
}

uint32_t get_optional_numeric_parameter( union t_parameter *param )
{
	uint32_t result ;
	result = 0 ;
	if ( *RunTime->ExecProgPtr != ')' )
	{
		result = get_parameter( param, PARAM_TYPE_NUMBER ) ;
	}
	return result ;
}

//void eatCloseBracket()
//{
//	if ( *RunTime->ExecProgPtr++ != ')' )
//	{
//		runError( SE_NO_BRACKET ) ;
//	}
//}
					 
void exec_drawnumber()
{
	int32_t x ;
	int32_t y ;
	int32_t value ;
	uint32_t result ;
	uint16_t attr ;
	uint8_t width ;
	union t_parameter params[3] ;

	// get 3 (or 4) parameters
	attr = 0 ;
	width = 10 ;
	
	result = get_numeric_parameters( params, 3 ) ;
	if ( result == 1 )
	{
		x = params[0].var ;
		y = params[1].var ;
		value = params[2].var ;
		result = get_optional_numeric_parameter( params ) ;
		if ( result == 1 )
		{
			attr = params[0].var ;
			result = get_optional_numeric_parameter( params ) ;
			if ( result == 1 )
			{
				width = params[0].var ;
			}
		}
#ifdef QT
		lcd_outdezNAtt( x, y, value, attr, width ) ;
#else
		if ( ScriptFlags & SCRIPT_LCD_OK )
		{
			lcd_outdezNAtt( x, y, value, attr, width ) ;
//				lcd_outdezAtt(x, y, value, attr ) ;
		}
#endif
//		eatCloseBracket() ;
	}
}


int32_t exec_power()
{
	int32_t value ;
	int32_t answer = 1 ;
	uint32_t power ;
	uint32_t result ;
	union t_parameter params[2] ;
	
	result = get_numeric_parameters( params, 2 ) ;
	if ( result == 1 )
	{
		value = params[0].var ;
	 	power = params[1].var ;
		if ( power > 20 )
		{
			power = 20 ;
		}
		while ( power )
		{
			answer *= value ;
			power -= 1 ;
		}
//		eatCloseBracket() ;
	}
	return answer ;
}

int32_t exec_bitField()
{
	uint32_t value = 0 ;
	uint32_t offset ;
	uint16_t size ;
	uint32_t result ;
	union t_parameter params[3] ;

	// get 3 parameters
	result = get_numeric_parameters( params, 3 ) ;
	if ( result == 1 )
	{
		value = params[0].var ;
		offset = params[1].var ;
		size = params[2].var ;
		value >>= offset ;
		// now pick out size 
		uint32_t mask = ( 1 << (size) ) - 1 ;
		value &= mask ;
//		eatCloseBracket() ;
	}
	return value ;
}

void exec_drawtimer()
{
	int32_t x ;
	int32_t y ;
	int16_t value ;
	uint32_t result ;
	uint8_t attr ;
	union t_parameter params[3] ;

	// get 3 (or 4) parameters
	attr = 0 ;
	result = get_numeric_parameters( params, 3 ) ;
	if ( result == 1 )
	{
		x = params[0].var ;
		y = params[1].var ;
		value = params[2].var ;
		result = get_optional_numeric_parameter( params ) ;
		if ( result == 1 )
		{
			attr = params[0].var ;
		}
#ifdef QT
			putsTime( x, y, value, attr, attr ) ;
#else
		if ( ScriptFlags & SCRIPT_LCD_OK )
		{
			putsTime( x, y, value, attr, attr ) ;
		}
#endif
//		eatCloseBracket() ;
	}
}

uint32_t getTypeColour()
{
	uint32_t result ;
	uint32_t paintType ;
	union t_parameter param ;
	paintType = 0 ;
	result = get_optional_numeric_parameter( &param ) ;
	if ( result == 1 )
	{
		if ( param.var < 5 )
		{
			paintType = param.var ;
		}
	}
	result = get_optional_numeric_parameter( &param ) ;
	if ( result == 1 )
	{
#if defined(PCBX12D) || defined(PCBX10)
		LcdCustomColour = param.var ;
#else
		if ( paintType == PLOT_CUSTOM )
		{
			paintType = PLOT_BLACK ;
		}
#endif
	}
	else
	{
		if ( paintType == PLOT_CUSTOM )
		{
			paintType = PLOT_BLACK ;
		}
	}
	return paintType ;
}


void exec_drawpoint()
{
	int32_t x1 ;
	int32_t y1 ;
	uint32_t result ;
	uint32_t paintType ;
	union t_parameter params[2] ;

	// get 2 parameters
	result = get_numeric_parameters( params, 2 ) ;
	if ( result == 1 )
	{
		x1 = params[0].var ;
		y1 = params[1].var ;
		paintType = getTypeColour() ;
#ifdef QT
	  lcd_plot(x1, y1 ) ;
#else
		if ( ScriptFlags & SCRIPT_LCD_OK )
		{
			pushPlotType( paintType ) ;
		  lcd_plot(x1, y1 ) ;
			popPlotType() ;
		}
#endif
//		eatCloseBracket() ;
	}
}

void exec_drawline()
{
	int32_t x1 ;
	int32_t y1 ;
	int32_t x2 ;
  int32_t y2 ;
	uint32_t result ;
	uint32_t paintType ;
	union t_parameter params[4] ;

	// get 4 parameters
	result = get_numeric_parameters( params, 4 ) ;
	if ( result == 1 )
	{
		x1 = params[0].var ;
		y1 = params[1].var ;
		x2 = params[2].var ;
		y2 = params[3].var ;
		paintType = getTypeColour() ;
#ifndef QT
		if ( ScriptFlags & SCRIPT_LCD_OK )
#endif
		{
			pushPlotType( paintType ) ;
			if (x1 == x2)
			{
    		lcd_vline(x1, y2 >= y1 ? y1 : y1+1, y2 >= y1 ? y2-y1+1 : y2-y1-1 ) ;
    	}
    	else if (y1 == y2)
			{
    		lcd_hline(x2 >= x1 ? x1 : x1+1, y1, x2 >= x1 ? x2-x1+1 : x2-x1-1 ) ;
    	}
			else
			{
				lcd_line(x1, y1, x2, y2, 0xFF, 0) ;
			}
			popPlotType() ;
		}
//		eatCloseBracket() ;
	}
}

void exec_drawrect()
{
	int32_t x ;
	int32_t y ;
	int32_t w ;
  int32_t h ;
  int32_t percent ;
	uint32_t result ;
	union t_parameter params[4] ;

	// get 4 parameters
	result = get_numeric_parameters( params, 4 ) ;
	if ( result == 1 )
	{
		x = params[0].var ;
		y = params[1].var ;
		w = params[2].var ;
		h = params[3].var ;
		percent = 0 ;
		result = get_optional_numeric_parameter( params ) ;
		if ( result == 1 )
		{
			percent = params[0].var ;
			if ( percent > 100 )
			{
				percent = 100 ;
			}
			if ( percent < 0 )
			{
				percent = 0 ;
			}
		}
#ifndef QT
		if ( ScriptFlags & SCRIPT_LCD_OK )
#endif
		{
		  lcd_hbar( x, y, w, h, percent ) ;
		}
//		eatCloseBracket() ;
	}
}

void exec_drawtext()
{
	int32_t x ;
	int32_t y ;
	uint8_t *p ;
	uint32_t result ;
	uint8_t attr ;
	union t_parameter params[2] ;
	uint32_t length = 0 ;

	// get 3 (or 4 or 5) parameters
	attr = 0 ;
//	result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
	result = get_numeric_parameters( params, 2 ) ;
	if ( result == 1 )
	{
		x = params[0].var ;
		y = params[1].var ;

//	if ( result == 1 )
//	{
//		x = param.var ;
//		result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
//		if ( result == 1 )
//		{
//			y = param.var ;
			result = get_parameter( &params[0], PARAM_TYPE_STRING ) ;
      if ( result == 2 )
			{
				p = params[0].cpointer ;
				result = get_optional_numeric_parameter( &params[0] ) ;
				if ( result == 1 )
				{
					attr = params[0].var ;
					result = get_optional_numeric_parameter( &params[0] ) ;
					if ( result == 1 )
					{
						length = params[0].var ;
					}
				}
#ifdef QT
				if ( length )
				{
					lcd_putsnAtt( x, y, (char *)p, length, attr ) ;
				}
				else
				{
			  	lcd_putsAtt( x, y, (char *)p, attr ) ;
				}
#else
				if ( ScriptFlags & SCRIPT_LCD_OK )
				{
					if ( length )
					{
						lcd_putsnAtt( x, y, (char *)p, length, attr ) ;
					}
					else
					{
				  	lcd_putsAtt( x, y, (char *)p, attr ) ;
					}
				}
#endif
//				eatCloseBracket() ;
			}
//		}
	}
}


// drawbitmap( x, y, bitmap )
void exec_drawbitmap()
{
	int32_t x ;
	int32_t y ;
	uint8_t *p ;
	uint32_t result ;
	union t_parameter params[2] ;
	uint32_t w ;

	// get 3 (or 4) parameters
	result = get_numeric_parameters( params, 2 ) ;
//	result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
	if ( result == 1 )
	{
		x = params[0].var ;
		y = params[1].var ;
			result = get_parameter( &params[0], PARAM_TYPE_STRING ) ;
      if ( result == 2 )
			{
				p = params[0].cpointer ;
				w = *p++ ;
#ifndef QT
				if ( ScriptFlags & SCRIPT_LCD_OK )
#endif
				{
					lcd_bitmap( x, y, p, w, 1, 0 ) ;
				}
//				eatCloseBracket() ;
			}
//		}
	}
}


void exec_drawclear()
{
	// call clear lcd
#ifdef QT
//  RunTimeData = cpystr( RunTimeData, (uint8_t *)"DrawClear()\n" ) ;
	lcd_clear() ;
#else
	if ( ScriptFlags & SCRIPT_LCD_OK )
	{
		lcd_clear() ;
	}
#endif
//	eatCloseBracket() ;
}


extern uint16_t getUnitsVoice( uint16_t unit ) ;

void exec_playnumber()
{
	int32_t number ;
	int32_t unit ;
	uint8_t decimals ;
	uint32_t result ;
	union t_parameter params[3] ;

	// get 3 (or 4) parameters
	result = get_numeric_parameters( params, 3 ) ;
	if ( result == 1 )
	{
		number = params[0].var ;
		unit = params[1].var ;
		decimals = params[2].var ;
#ifdef QT
//    RunTimeData = cpystr( RunTimeData, (uint8_t *)"PlayNumber()\n" ) ;
        (void) number ;
        (void) unit ;
        (void) decimals ;
#else
		if ( unit > 0 && unit < 10 )
		{
			unit = getUnitsVoice( unit ) ;
		}
		else
		{
			unit = 0 ;
		}
		voice_numeric( number, decimals, unit ) ;
#endif
//		eatCloseBracket() ;
	}
}

//uint32_t channelUsed( uint32_t channel )
//{
//#if EXTRA_SKYMIXERS
//	for(uint8_t i=0;i<MAX_SKYMIXERS+EXTRA_SKYMIXERS;i++)
//#else
//	for(uint8_t i=0;i<MAX_SKYMIXERS;i++)
//#endif
//	{
//		SKYMixData *md = mixAddress( i ) ;
//    if ((md->destCh==0) || (md->destCh>NUM_SKYCHNOUT+EXTRA_SKYCHANNELS))
//		{
//			break ;
//		}
//    if (md->destCh == channel)
//		{
//			return 1 ;
//		}
//	}
//	return 0 ;
//}


//extern const int8_t TelemIndex[] ;
//extern const int8_t TelemValid[] ;
void storeTelemetryData( uint8_t index, uint16_t value ) ;

int32_t exec_settelitem()
{
	int32_t number ;
	int32_t value ;
	uint32_t result ;
	union t_parameter param ;
	uint8_t *p ;
	int32_t retval ;

	retval = -1 ;
	number = 0 ;
	if ( *RunTime->ExecProgPtr == 0x70 )
	{
		result = get_parameter( &param, PARAM_TYPE_STRING ) ;
		if ( result == 2 )
		{
			p = param.cpointer ;
#ifdef QT			
			number = 0 ;
#else
			number = basicFindValueIndexByName( (char *)p ) ;
#endif
			result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
			if ( result == 1 )
			{
				value = param.var ;
#ifdef QT			
//        RunTimeData = cpystr( RunTimeData, (uint8_t *)"setTelItem()\n" ) ;
                (void) number ;
                (void) value ;
                (void) p ;
#else
				number -= 44 ;
				if ( ( number >= 0 ) && ( number < NUM_TELEM_ITEMS ) )
				{
					result = TelemValid[number] ;
					number = TelemIndex[number] ;
					retval = value ;
					if ( ( result == 1 ) || ( result == 2 ) )
					{
						storeTelemetryData( number, value ) ;
					}
					else if ( result == 0 )
					{
						if ( ( number >= V_GVAR1 ) && ( number <= V_GVAR7 ) )
						{
							number -= V_GVAR1 ;
				    }
						if ( value < -125 )
						{
							value = -125 ;
						}
						if ( value > 125 )
						{
							value = 125 ;
						}
						g_model.gvars[number].gvar = value ;
					}
				}
//				else
//				{
//					number += 44 - 12 - 8 ;
//					if ( (number >= 0) && (number < 24) )
//					{
//						if ( channelUsed( number + 1 ) == 0 )
//						{
//							if ( value < -125 )
//							{
//								value = -125 ;
//							}
//							if ( value > 125 )
//							{
//								value = 125 ;
//							}
//							chans[number] = value * 1024 ;
//							retval = number + value * 100 ;
//						}
//					}
//				}
#endif
			}
		}
	}
	return retval ;
}

int32_t convertGpsValue( uint16_t whole, uint16_t frac, uint16_t sign )
{
	int32_t result ;
	int32_t value ;
	div_t qr ;
	qr = div( whole, 100 ) ;
	result = qr.quot * 1000000 ;
	value = qr.rem ;
	value *= 10000 ;
	value += frac ;
	value *= 10 ;
	value /= 6 ;
	result += value ;
	return sign ? -result : result ;
}

int32_t exec_getvalue(uint32_t type)
{
	int32_t number ;
	uint32_t result ;
	union t_parameter param ;
	uint8_t *p ;

	number = 0 ;
	if ( *RunTime->ExecProgPtr == 0x70 )
	{
		result = get_parameter( &param, PARAM_TYPE_STRING ) ;
		if ( result == 2 )
		{
			p = param.cpointer ;
#ifdef QT			
extern int GetValue ;
			number = GetValue ;
#else
			number = basicFindValueIndexByName( (char *)p ) ;
#endif
		}
	} 
	else
	{
		result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
		if ( result == 1 )
		{
			number = param.var ;
		}
	}
	if ( result )
	{
#ifdef QT			
//		RunTimeData = cpystr( RunTimeData, (uint8_t *)"GetValue()\n" ) ;
        (void) p ;
        (void) type ;
#else
		if ( number >= 0 )
		{
#ifndef SMALL
			if ( number >= 1000 )
			{
				number -= 1000 ;
				result = 0 ;
#if defined(at91sam3s4) || defined(at91sam3s8)
extern uint32_t IdlePercent ;
extern uint32_t MixerRate ;
				switch ( number )
				{
					case 0 :
						result = PIOA->PIO_PDSR ;
					break ;
					case 1 :
						result = PIOB->PIO_PDSR ;
					break ;
					case 2 :
						result = PIOC->PIO_PDSR ;
					break ;
					case 20 :
						result = MixerRate ;
					break ;
					case 21 :
						result = IdlePercent ;
					break ;
					case 22 :
						result = g_eeGeneral.physicalRadioType ;
					break ;
				}
#endif
#if defined(stm32f205) || defined(STM32F429_439xx)
extern uint32_t IdlePercent ;
extern uint32_t MixerRate ;
				switch ( number )
				{
					case 0 :
						result = GPIOA->IDR ;
					break ;
					case 1 :
						result = GPIOB->IDR ;
					break ;
					case 2 :
						result = GPIOC->IDR ;
					break ;
					case 3 :
						result = GPIOD->IDR ;
					break ;
					case 4 :
						result = GPIOE->IDR ;
					break ;
					case 20 :
						result = MixerRate ;
					break ;
					case 21 :
						result = IdlePercent ;
					break ;
					case 22 :
						result = g_eeGeneral.physicalRadioType ;
					break ;
				}
#endif
			}
			else
#endif //SMALL
			{
				result = number ;
		  	number = getValue(result) ; // ignored for GPS, DATETIME, and CELLS
  			if ( (result == CHOUT_BASE+NUM_SKYCHNOUT) || (result == CHOUT_BASE+NUM_SKYCHNOUT+1) ) // A1 or A2
				{
					number = scale_telem_value( number, result - (CHOUT_BASE+NUM_SKYCHNOUT), 0 ) ;
				}
				if ( type == 0 )
				{
  			  if ( (result < CHOUT_BASE+NUM_SKYCHNOUT) || ( ( result >= EXTRA_POTS_START ) && (result < EXTRA_POTS_START + 8) ) )
					{
						number *= 100 ;
						number /= RESX ;
					}
				}
			}
		}
		else if ( number == -2 )	// Latitude
		{
			return convertGpsValue( FrskyHubData[FR_GPS_LAT], FrskyHubData[FR_GPS_LATd], FrskyHubData[FR_LAT_N_S] == 'S' ) ;
		}
		else if ( number == -3 )	// Longitude
		{
			return convertGpsValue( FrskyHubData[FR_GPS_LONG], FrskyHubData[FR_GPS_LONGd], FrskyHubData[FR_LONG_E_W] == 'W' ) ;
		}
#endif
	}
//	eatCloseBracket() ;
	return number ;
}

static int32_t exec_idletime()
{
//	eatCloseBracket() ;
#ifdef QT			
	return 50 ;
#else
extern uint32_t IdlePercent ;
	return IdlePercent ;
#endif
}

int32_t getSingleNumericParameter()
{
	uint32_t result ;
	union t_parameter param ;

	result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
	if ( result != 1 )
	{
		runError( SE_SYNTAX ) ;
	}
//	eatCloseBracket() ;
	return param.var ;
}

static int32_t exec_gettime()
{
	if ( *RunTime->ExecProgPtr != ')' )
	{
		int32_t index ;
		index = getSingleNumericParameter() ;
		switch ( index )
		{
			case 0 :
				return Time.year ;
			break ;
			case 1 :
				return Time.month ;
			break ;
			case 2 :
				return Time.date ;
			break ;
			case 3 :
				return Time.hour ;
			break ;
			case 4 :
				return Time.minute ;
			break ;
			case 5 :
				return Time.second ;
			break ;
		}
		return 0 ;
	}
//	eatCloseBracket() ;
#ifdef QT
	return g_tmr10ms ;
#else
extern volatile uint32_t get_ltmr10ms() ;
	return get_ltmr10ms() ;
#endif
}

static int32_t exec_getlastpos()
{
//	eatCloseBracket() ;
#ifdef QT			
  return 20 ;
#else
	return Lcd_lastPos ;
#endif
}

static int32_t exec_sysflags()
{
//	eatCloseBracket() ;
	
#if defined(PCBX9D) || defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE) || defined(PCBX12D) || defined(PCBX10) || defined(REV19)
	return ScriptFlags | SCRIPT_FRSKY ;
#else
	return ScriptFlags ;
#endif
}

static int32_t exec_not()
{
	return ~getSingleNumericParameter() ;
}

static int32_t exec_abs()
{
	int32_t value ;
	value = getSingleNumericParameter() ;
  return value >= 0 ? value : -value ;
}

static int32_t exec_sqrt()
{
	int32_t value ;
	value = getSingleNumericParameter() ;
	if ( value < 0 )
	{
		return 0 ;
	}
  return isqrt32(value) ;
}

static void exec_killEvents()
{
	int32_t value ;
	value = getSingleNumericParameter() ;
#ifndef QT
  killEvents(value) ;
#else
  (void) value ;
#endif
}



const uint8_t SportIds[28] = {0x00, 0xA1, 0x22, 0x83, 0xE4, 0x45, 0xC6, 0x67,
				                      0x48, 0xE9, 0x6A, 0xCB, 0xAC, 0x0D, 0x8E, 0x2F,
															0xD0, 0x71, 0xF2, 0x53, 0x34, 0x95, 0x16, 0xB7,
															0x98, 0x39, 0xBA, 0x1B/*, 0x7C, 0xDD, 0x5E, 0xFF*/ } ;

int32_t exec_sportsend()
{
	static uint8_t sportPacket[8] ;
	uint32_t result = 0 ;
	union t_parameter params[3] ;
//	union t_parameter param ;
	int32_t x ;
	int32_t access = 0 ;

	result = get_parameter( &params[0], PARAM_TYPE_NUMBER ) ;
	if ( result == 1 )
	{
		x = params[0].var ;
		if ( x == 0xFF )
		{
#ifdef QT
//      RunTimeData = cpystr( RunTimeData, (uint8_t *)"SportSend()\n" ) ;
      result = 0 ;
      (void) sportPacket ;
      (void) access ;
#else
//extern uint32_t sportPacketSend( uint8_t *pdata, uint16_t index ) ;
			result = sportPacketSend( 0, 0 ) ;
#endif
		}
		else
		{
//			sportPacket[7] = SportIds[x & 0xFF] ;
			access = (x & 0xFF00) | SportIds[x & 0xFF] ;
			result = get_numeric_parameters( params, 2 ) ;
	    sportPacket[0] = params[0].var ;
			x = params[1].var ;
			sportPacket[1] = x ;
			sportPacket[2] = x >> 8 ;
			result = get_parameter( &params[0], PARAM_TYPE_EITHER ) ;
			if ( result == 1 )
			{
				result = params[0].var ;
				sportPacket[3] = result ;
				sportPacket[4] = result >> 8 ;
				sportPacket[5] = result >> 16 ;
				sportPacket[6] = result >> 24 ;
			}
			else if ( result == 2 )
			{
				sportPacket[3] = params[0].cpointer[0] ;
				sportPacket[4] = params[0].cpointer[1] ;
				sportPacket[5] = params[0].cpointer[2] ;
				sportPacket[6] = params[0].cpointer[3] ;
			}

//			result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
//			if ( result == 1 )
//			{
//		    sportPacket[0] = param.var ;
//				result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
//	      if ( result == 1 )
//				{
//					x = param.var ;
//					sportPacket[1] = x ;
//					sportPacket[2] = x >> 8 ;
//					result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
//		      if ( result == 1 )
//					{
//						result = param.var ;
//						sportPacket[3] = result ;
//						sportPacket[4] = result >> 8 ;
//						sportPacket[5] = result >> 16 ;
//						sportPacket[6] = result >> 24 ;
#ifdef QT
//            RunTimeData = cpystr( RunTimeData, (uint8_t *)"SportSend()\n" ) ;
            result = 0 ;
#else
//#ifdef ACCESS
//						if ( ( access & 0x4000 ) || ( ( access & 0x0300 ) == 0 ) )
////						if ( ( ( access & 0x0300 ) == 0 ) || (access & 0x4000 ) )	// Module 0
//						{
//            	result = accessSportPacketSend( sportPacket, access ) ;
//						}
//						else
//						{
            	result = sportPacketSend( sportPacket, access ) ;
//						}
//#else
//            result = sportPacketSend( sportPacket, sportPacket[7] ) ;
//#endif	
#endif	
//						eatCloseBracket() ;
//          }
//        }
//      }
    }
  }
	return result ;
}

int32_t exec_sportreceive()
{
	// This needs the address of 4 variables for the data
	union t_parameter param[4] ;
	uint8_t type[4] ;
	uint32_t result ;
	result = getParamVarAddress( &param[0], 0 ) ;
	if ( result )
	{
		type[0] = result ;
		result = getParamVarAddress( &param[1], 0 ) ;
		if ( result )
		{
			type[1] = result ;
			result = getParamVarAddress( &param[2], 0 ) ;
			if ( result )
			{
				type[2] = result ;
				result = getParamVarAddress( &param[3], 0 ) ;
				if ( result )
				{
					type[3] = result ;
#ifdef QT
//        	RunTimeData = cpystr( RunTimeData, (uint8_t *)"SportReceive()\n" ) ;
          result = 1 ;
          (void) type ;
#else
					if ( fifo128Space( &Script_fifo ) <= ( 127 - 8 ) )
					{
						uint32_t value ;
						value = get_fifo128( &Script_fifo ) & 0x1F ;
						if ( type[0] == 1 )
						{
							*param[0].bpointer = value ;
						}
						else
						{
							*param[0].ipointer = value ;
						}
						value = get_fifo128( &Script_fifo ) ;
						if ( type[1] == 1 )
						{
							*param[1].bpointer = value ;
						}
						else
						{
							*param[1].ipointer = value ;
						}
						value = get_fifo128( &Script_fifo ) ;
						value |= get_fifo128( &Script_fifo ) << 8 ;
						if ( type[2] == 1 )
						{
							*param[2].bpointer = value ;
						}
						else
						{
							*param[2].ipointer = value ;
						}
						value = get_fifo128( &Script_fifo ) ;
						value |= get_fifo128( &Script_fifo ) << 8 ;
						value |= get_fifo128( &Script_fifo ) << 16 ;
						value |= get_fifo128( &Script_fifo ) << 24 ;
						if ( type[3] == 1 )
						{
							*param[3].bpointer = value ;
						}
						else
						{
							*param[3].ipointer = value ;
						}
          	result = 1 ;
					}
					else
					{
          	result = 0 ;
					}
//					eatCloseBracket() ;
#endif	
				}
			}
		}
	}
	return result ;
}

int32_t exec_crossfirereceive()
{
	union t_parameter param[3] ;
	uint8_t type[4] ;
  uint8_t length ;
	int32_t value ;
	uint32_t result ;
	uint32_t tidy = 1 ;
	
	result = getParamVarAddress( &param[0], 0 ) ;
	if ( result )
	{
		type[0] = result ;
		result = getParamVarAddress( &param[1], 0 ) ;
		if ( result )
		{
			type[1] = result ;
			if ( result )
			{
#ifdef QT
        value = 0 ;
        (void) type ;
        (void) length ;
#else
				value = peek_fifo128( &Script_fifo ) ;
#endif
				if ( value != -1 )
				{
					length = value ;	// Includes length byte
#ifndef QT
					if ( fifo128Space( &Script_fifo ) <= (uint32_t)(127 - length ) )
					{
						// OK to read packet
						// values to return : length, command = byte after length, data into array if possible
						length -= 2 ;
						result = getParamVarAddress( &param[2], length ) ;
						tidy = 0 ;
						if ( result )
						{
							value = get_fifo128( &Script_fifo ) ;
							if ( type[0] == 1 )
							{
								*param[0].bpointer = length ;
							}
							else
							{
								*param[0].ipointer = length ;
							}
							value = get_fifo128( &Script_fifo ) ;
							if ( type[1] == 1 )
							{
								*param[1].bpointer = value ;
							}
							else
							{
								*param[1].ipointer = value ;
							}
							for ( ; length ; length -= 1 )
							{
								*param[2].bpointer++ = get_fifo128( &Script_fifo ) ;
							}
							result = 1 ;
						}
					}
#endif
				}
				if ( tidy )
				{
					result = 0 ;
					getParamVarAddress( &param[2], 0 ) ;
				}
//				eatCloseBracket() ;
			}
		}
	}
  return result ;
}

// command, count(# of bytes), byte_array
int32_t exec_crossfiresend()
{
//	send MODULE_ADDRESS, count+2, command, byte_array, crc
	uint32_t result = 0 ;
	union t_parameter param ;
//	union t_parameter address ;
	int32_t command ;
	uint32_t length ;

	result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
	if ( result == 1 )
	{
		command = param.var ;
		if ( command == 0xFF )
		{
#ifdef QT
//      RunTimeData = cpystr( RunTimeData, (uint8_t *)"XfireSend()\n" ) ;
      result = 0 ;
#else
			result = xfirePacketSend( 0, 0, 0 ) ;
#endif
		}
		else
		{
			result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
			if ( result == 1 )
			{
				length = param.var ;
				result = getParamVarAddress( &param, length ) ;
				if ( result == 1 )	// Byte array
				{
#ifdef QT
//      RunTimeData = cpystr( RunTimeData, (uint8_t *)"XfireSend()\n" ) ;
      result = 0 ;
#else
					result = xfirePacketSend( length, command, param.bpointer ) ;
#endif
				}
				else
				{
					runError( SE_SYNTAX ) ;
				}
			}	
			else
			{
				runError( SE_SYNTAX ) ;
			}
//			eatCloseBracket() ;
		}
	}
	else
	{
		runError( SE_SYNTAX ) ;
	}
	return result ;
}

// length, buffer = byte array
// BT or COM2
int32_t  exec_btreceive()
{
	uint32_t result = 0 ;
	union t_parameter param ;
  uint32_t length ;
  uint32_t i ;
	int32_t x ;

	result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
	if ( result )
	{
		length = param.var ;
		result = getParamVarAddress( &param, length ) ;
		if ( result )
		{
#ifdef QT
			result = 0 ;
            (void) i ;
            (void) x ;
#else
#ifdef BLUETOOTH

			if ( ScriptFlags & SCRIPT_STANDALONE )
			{
				if ( g_model.com2Function != COM2_FUNC_SCRIPT )
				{
					if ( BtCurrentFunction != BT_SCRIPT )
					{
						scriptRequestBt() ;
					}
				}
			}

			if ( ( BtCurrentFunction == BT_SCRIPT ) || ( g_model.com2Function == COM2_FUNC_SCRIPT ) )
			{
				for ( i = 0 ; i < length ; i += 1 )
				{
					if ( BtCurrentFunction == BT_SCRIPT )
					{
						x = rxBtuart() ;
					}
					else
					{
						x = get_fifo128( &Com2_fifo ) ;
					}
					if ( x != -1 )
					{
						*param.bpointer++ = x ;
					}
					else
					{
						break ;
					}
				}
				result = i ;
			}
#else
			if ( g_model.com2Function == COM2_FUNC_SCRIPT )
			{
				for ( i = 0 ; i < length ; i += 1 )
				{
					x = get_fifo128( &Com2_fifo ) ;
					if ( x != -1 )
					{
						*param.bpointer++ = x ;
					}
					else
					{
						break ;
					}
				}
				result = i ;
			}
#endif
			else
			{
				result = 0 ;
			}
#endif
		}
	}
	return result ;
}

// length, buffer = byte array
int32_t exec_btsend()
{
	uint32_t result = 0 ;
#ifdef BLUETOOTH
	union t_parameter param ;
	uint32_t length ;

	result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
	if ( result == 1 )
	{
		length = param.var ;
		if ( length == 0 )
		{
#ifdef QT
      result = 0 ;
#else
			result = btSend( 0, 0 ) ;
#endif
		}
		else
		{
			result = getParamVarAddress( &param, length ) ;
			if ( result == 1 )	// Byte array
			{
				if ( length <= 64 )
				{
		      result = 0 ;
#ifdef QT
#else
					if ( BtCurrentFunction == BT_SCRIPT )
					{
						result = btSend( length, param.bpointer ) ;
					}
	#ifndef PCB9XT 
	#ifndef PCBX7
					else if ( g_model.com2Function == COM2_FUNC_SCRIPT )
					{
						if ( Com2_tx.ready == 0 )	// Buffer available
						{
							uint32_t x ;
							if ( length )
							{
								if ( length > 64 )
								{
									result = 0 ;
								}
								else
								{
									x = 0 ;
									while ( x < length )
									{
										Com2TxBuffer[x++] = *param.bpointer++ ;
									}
									Com2_tx.size = length ;
									Com2_tx.buffer = Com2TxBuffer ;
									txPdcCom2( &Com2_tx ) ;
								}
								result = 1 ;
							}
						}
					}
	#endif
	#endif
#endif
				}
				else
				{
					return 0 ;
				}
			}
		}
	}
#endif
	return result ;
}

#ifdef FILE_SUPPORT
void exec_directory()
{
	uint32_t result ;
	union t_parameter param ;
	struct fileControl *fc ;
	uint8_t *dir ;
	uint8_t *ext ;

	result = get_parameter( &param, PARAM_TYPE_STRING ) ;
	if ( result == 2 )
	{
		dir = param.cpointer ;
		uint32_t i ;
		for ( i = 0 ; i < DIR_LENGTH-1 ; i += 1 )
		{
			if ( (DirectoryName[i] = *dir++) == '\0' )
			{
				break ;
			}
		}
		DirectoryName[i] = '\0' ;
		dir = param.cpointer ;
		result = get_parameter( &param, PARAM_TYPE_STRING ) ;
		if ( result == 2 )
	  {
			ext = param.cpointer ;
			if ( ScriptFlags & SCRIPT_STANDALONE )
			{
				fc = &BasicFileControl ;
				setupFileNames( (TCHAR *)dir, fc, (char *) ext ) ;
			}
//			eatCloseBracket() ;
		}
	}
	killEvents(RunTime->Vars.Variables[0]) ;
	RunTime->Vars.Variables[0] = 0 ;
}

// Returns
// -1 error
// 0 still runing
// 1 Select (Long Menu)
// 2 Exit
// 3 Tagged (Short Menu)

int32_t exec_filelist()
{
	struct fileControl *fc = &BasicFileControl ;
	uint32_t i ;
	uint16_t val16 ;
	uint16_t value ;
	uint32_t dimension ;
	uint8_t *p ;
	uint8_t *q ;
	uint8_t opcode ;
	union t_parameter param ;

	opcode = *RunTime->ExecProgPtr++ ;
	if ( ( opcode & 0xF6 ) == 0x66 )	// A byte array
	{
		val16 = getVarIndex( opcode ) ;
		dimension = *RunTime->ExecProgPtr++ ;

//				if ( opcode & 0x01 )	// 16-bit dimension
//				{
//					dimension |= *RunTime->ExecProgPtr++ << 8 ;
//				}

		value = expression() ;
		opcode = *RunTime->ExecProgPtr++ ;
		if ( opcode != ']' )
		{
			runError( SE_SYNTAX ) ;
			return -1 ; ;
		}
		if ( *RunTime->ExecProgPtr != ',' )
		{
			runError( SE_SYNTAX ) ;
			return -1 ;
		}
		RunTime->ExecProgPtr += 1 ;
		i = getParamVarAddress( &param, 0 ) ;
		if ( i != 2 )		// An int pointer
		{
			runError( SE_SYNTAX ) ;
		}
//		eatCloseBracket() ;
	}
	else
	{
		runError( SE_SYNTAX ) ;
		return 1 ;
	}
	if ( ScriptFlags & SCRIPT_STANDALONE )
	{
		lcd_clear() ;
		lcd_putsAtt( 0, 0, "Select File", 0 ) ;
		
		ScriptDirNeeded = 1 ;
		i = fileList( RunTime->Vars.Variables[0], fc ) ;
		if ( i )
		{
			if ( i == 1 )	// Select
			{
				uint32_t j ;
				q = (uint8_t *)SharedMemory.FileList.Filenames[fc->vpos] ;
				j = strlen( (char *)q ) ;
				j += strlen( (char *)DirectoryName ) ;
				// val16 = start of byte array
				// value index into byte array
				// dimension = size of byte array
				p = &RunTime->ByteArrayStart[val16 + value] ;
				if ( ( value + j < dimension - 2 ) && *q)	// allow for '\'
				{
					p = cpystr( p, DirectoryName ) ;
					*p++ = '\\' ;
					p = cpystr( p, q ) ;
					*param.ipointer = FileSize[fc->vpos] ;
				}
				else
				{
					*p = '\0' ;
				}
			}
			killEvents(RunTime->Vars.Variables[0]) ;
			RunTime->Vars.Variables[0] = 0 ;
		}
		return i ;
	}
	return 0 ;
}

// fopen( name, r/w )
int32_t exec_fopen()
{
	uint32_t result ;
	FRESULT fresult ;
	union t_parameter params[2] ;
	int32_t retValue = 0 ;

	result = get_parameter( &params[0], PARAM_TYPE_STRING ) ;
	if ( result == 2 )
	{
		result = get_parameter( &params[1], PARAM_TYPE_NUMBER ) ;
		if ( result == 1 )
	  {
			if ( ScriptFlags & SCRIPT_STANDALONE )
			{
				fresult = f_open( &MultiBasicFile, (TCHAR *)params[0].cpointer,
											(params[1].var == 0) ? FA_READ : FA_WRITE | FA_CREATE_ALWAYS ) ;
  			if (fresult == FR_OK)
				{
					retValue = 1 ;
				}
			}
		}
	}
//	eatCloseBracket() ;
	return retValue ;
}

// fread( size, dest, numread )
int32_t exec_fread()
{
	uint32_t result ;
	FRESULT fresult ;
	union t_parameter param[2] ;
	uint32_t length ;
	UINT nread ;
	int32_t retValue = 0 ;

	result = get_parameter( &param[0], PARAM_TYPE_NUMBER ) ;
	if ( result == 1 )
	{
		length = param[0].var ;
		result = getParamVarAddress( &param[0], length ) ;
		if ( result == 1 )	// Byte array
	  {
			result = getParamVarAddress( &param[1], 0 ) ;
			if ( result == 2 )	// int address
			{
				if ( ScriptFlags & SCRIPT_STANDALONE )
				{
					fresult = f_read( &MultiBasicFile, param[0].bpointer, length, &nread ) ;
					*param[1].ipointer = 0 ;
  				if (fresult == FR_OK)
					{
						*param[1].ipointer = nread ;
						retValue = 1 ;
					}
				}
			}
		}
	}
//	eatCloseBracket() ;
	return retValue ;
}

// fwrite( size, source, numwritten )
int32_t exec_fwrite()
{
	uint32_t result ;
	FRESULT fresult ;
	union t_parameter param[2] ;
	uint32_t length ;
	UINT nwritten ;
	int32_t retValue = 0 ;

	result = get_parameter( &param[0], PARAM_TYPE_NUMBER ) ;
	if ( result == 1 )
	{
		length = param[0].var ;
		result = getParamVarAddress( &param[0], 0 ) ;
		if ( result == 1 )	// Byte array
	  {
			result = getParamVarAddress( &param[1], 0 ) ;
			if ( result == 2 )	// int address
			{
				if ( ScriptFlags & SCRIPT_STANDALONE )
				{
					fresult = f_write( &MultiBasicFile, param[0].bpointer, length, &nwritten ) ;
					*param[1].ipointer = 0 ;
  				if (fresult == FR_OK)
					{
						*param[1].ipointer = nwritten ;
						retValue = 1 ;
					}
				}
			}
		}
	}
//	eatCloseBracket() ;
	return retValue ;
}


// fclose()
static void exec_fclose()
{
	if ( ScriptFlags & SCRIPT_STANDALONE )
	{
		f_close( &MultiBasicFile ) ;
	}
//	eatCloseBracket() ;
	return ;
}

#endif	// FILE_SUPPORT

int32_t exec_returnvalue()
{
	return RunTime->FunctionReturnValue ; ;
}

void exec_resettelemetry()
{
	int32_t value ;
	value = getSingleNumericParameter() ;
#ifndef QT
  resetTelemetry( value ) ;
#else
    (void) value ;
#endif
}

extern uint32_t doPopup( const char *list, uint16_t mask, uint8_t width, uint8_t event ) ;
extern struct t_popupData PopupData ;

#define POPUP_NONE			0
#define POPUP_SELECT		1
#define POPUP_EXIT			2

// int32_t popup( string, mask, width )
// returns 0 - nothing
//				 1 - 16 item eelected
//				99 - EXIT
int32_t exec_popup()
{
	uint32_t result ;
	char *list ;
	uint16_t mask ;
	uint8_t width ;
	union t_parameter param ;
			
	result = get_parameter( &param, PARAM_TYPE_STRING ) ;
  if ( result == 2 )
	{
    list = (char *)param.cpointer ;
		result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
		if ( result == 1 )
		{
			mask = param.var ;
			result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
			if ( result == 1 )
			{
        width = param.var ;
//#ifdef QT
//        RunTimeData = cpystr( RunTimeData, (uint8_t *)"popup()" ) ;
//				result = 0 ;
//#else
				if ( ScriptPopupActive == 0 )
				{
					ScriptPopupActive = 1 ;
					PopupData.PopupIdx = 0 ;
				}
				result = doPopup( list, mask, width, RunTime->Vars.Variables[0] ) ;
//				eatCloseBracket() ;
				if ( result )
				{
					if ( result == POPUP_SELECT )
					{
						result = PopupData.PopupSel + 1 ;
					}
					else if ( result == POPUP_EXIT )
					{
						result = 99 ;
					}
					RunTime->Vars.Variables[0] = 0 ;
					ScriptPopupActive = 0 ;
				}
//#endif
				return result ;
			}
		}
	}
	return 0 ;
}

int32_t exec_strToArray()
{
	uint8_t opcode ;
	uint16_t val16 ;
	opcode = *RunTime->ExecProgPtr++ ;
	uint32_t value ;
	uint32_t sent = 0 ;
	uint32_t dimension ;

	if ( ( opcode & 0xF6 ) == 0x66 )	// A byte array
	{
		struct byteAddress baddr ;
		if ( getParamByteArrayAddress( &baddr, 0 ) == 0 )
		{
			return 0 ;
		}
		val16 = baddr.varOffset ;
		value = baddr.varIndex ;
		dimension = baddr.dimension ;

//		uint32_t dimension ;
//		val16 = getVarIndex( opcode ) ;
//		dimension = *RunTime->ExecProgPtr++ ;
//		value = expression() ;
//		opcode = *RunTime->ExecProgPtr++ ;
//		if ( opcode != ']' )
//		{
//			runError( SE_SYNTAX ) ;
//			return 0 ;
//		}
//		if ( value >= dimension )
//		{
//			runError( SE_DIMENSION ) ;
//			return 0 ;
//		}
		// Now we need the string
//		if ( *RunTime->ExecProgPtr != ',' )
//		{
//			runError( SE_SYNTAX ) ;
//			return 0 ;
//		}
//		RunTime->ExecProgPtr += 1 ;
		opcode = *RunTime->ExecProgPtr++ ;
		if ( opcode == 0x70 )	// Quoted string
		{
			// Now copy the string
			uint8_t *ptr ;
			uint32_t length = *RunTime->ExecProgPtr++ ;
			ptr = RunTime->ExecProgPtr ;			// Point to string
      RunTime->ExecProgPtr += length ;	// Skip the string
			uint32_t t = stringSubscript( length ) ;
			ptr += t ;
			length -= t ;
			while ( length-- )
			{
				uint8_t c ;
				c = *ptr++ ;
				if ( value < baddr.dimension )
				{
					RunTime->ByteArrayStart[val16 + value] = c ;
					value += 1 ;
					sent += 1 ;
				}
				else
				{
					RunTime->ByteArrayStart[val16 + baddr.dimension - 1] = '\0' ;
					break ;
				}
			}
//			eatCloseBracket() ;
		}
		else
		{
			if ( ( opcode & 0xF6 ) == 0x66 )	// A byte array
			{
				uint8_t c ;
				struct byteAddress baddr ;
				uint32_t dim1 ;
				uint16_t val16a ;
				uint32_t valuea ;
				if ( getParamByteArrayAddress( &baddr, 0 ) == 0 )
				{
					return 0 ;
				}
				val16a = baddr.varOffset ;
				valuea = baddr.varIndex ;
				dim1 = baddr.dimension ;
				
//				uint32_t dim1 ;
//				uint16_t val16a = getVarIndex( opcode ) ;
//				dim1 = *RunTime->ExecProgPtr++ ;
//				uint32_t valuea = expression() ;
//				opcode = *RunTime->ExecProgPtr++ ;
//				if ( opcode != ']' )
//				{
//					runError( SE_SYNTAX ) ;
//					return 0 ;
//				}
//				if ( value >= dim1 )
//				{
//					runError( SE_DIMENSION ) ;
//					return 0 ;
//				}
				do
				{
					if ( valuea < dim1 )
					{
						c = RunTime->ByteArrayStart[val16a + valuea] ;
					}
					else
					{
						c = 0 ;
					}
					if ( value < dimension )
					{
						RunTime->ByteArrayStart[val16 + value] = c ;
						value += 1 ;
						valuea += 1 ;
						sent += 1 ;
					}
					else
					{
						RunTime->ByteArrayStart[val16 + dimension - 1] = '\0' ;
						break ;
					}
				} while (c) ;
			}
			else
			{			
				runError( SE_SYNTAX ) ;
				return 0 ;
			}
		}
	}
	else
	{
		runError( SE_SYNTAX ) ;
		return 0 ;
	}
	return sent ;
}

// bytemove( destByteArray, srcByteArray, length )
int32_t exec_bytemove()
{
	uint8_t opcode ;
	uint16_t val16 ;
	opcode = *RunTime->ExecProgPtr++ ;
	uint32_t value ;
	uint32_t dimension ;
	union t_parameter param ;
	uint32_t length = 0 ;
	uint32_t sent = 0 ;

	if ( ( opcode & 0xF6 ) == 0x66 )	// A byte array
	{
		struct byteAddress baddr ;
		if ( getParamByteArrayAddress( &baddr, 0 ) == 0 )
		{
			return 0 ;
		}
		val16 = baddr.varOffset ;
		value = baddr.varIndex ;
		dimension = baddr.dimension ;

//		uint32_t dimension ;
//		val16 = getVarIndex( opcode ) ;
//		dimension = *RunTime->ExecProgPtr++ ;
//		value = expression() ;
//		opcode = *RunTime->ExecProgPtr++ ;
//		if ( opcode != ']' )
//		{
//			runError( SE_SYNTAX ) ;
//			return 0 ;
//		}
//		if ( value >= dimension )
//		{
//			runError( SE_DIMENSION ) ;
//			return 0 ;
//		}
		// Now we need the string
//		if ( *RunTime->ExecProgPtr != ',' )
//		{
//			runError( SE_SYNTAX ) ;
//			return 0 ;
//		}
//		RunTime->ExecProgPtr += 1 ;
		opcode = *RunTime->ExecProgPtr++ ;
		if ( ( opcode & 0xF6 ) == 0x66 )	// A byte array
		{
			uint8_t c ;
			struct byteAddress baddr ;
			uint32_t dim1 ;
			uint16_t val16a ;
			uint32_t valuea ;
			uint32_t result ;
			if ( getParamByteArrayAddress( &baddr, 0 ) == 0 )
			{
				return 0 ;
			}
			val16a = baddr.varOffset ;
			valuea = baddr.varIndex ;
			dim1 = baddr.dimension ;
				
//				uint32_t dim1 ;
//				uint16_t val16a = getVarIndex( opcode ) ;
//				dim1 = *RunTime->ExecProgPtr++ ;
//				uint32_t valuea = expression() ;
//				opcode = *RunTime->ExecProgPtr++ ;
//				if ( opcode != ']' )
//				{
//					runError( SE_SYNTAX ) ;
//					return 0 ;
//				}
//				if ( value >= dim1 )
//				{
//					runError( SE_DIMENSION ) ;
//					return 0 ;
//				}

			result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
			if ( result == 1 )
			{
				length = param.var ;

				while ( length-- )
				{
					if ( valuea < dim1 )
					{
						c = RunTime->ByteArrayStart[val16a + valuea] ;
						if ( value < dimension )
						{
							RunTime->ByteArrayStart[val16 + value] = c ;
							value += 1 ;
							valuea += 1 ;
							sent += 1 ;
						}
						else
						{
							break ;
						}
					}
					else
					{
						break ;
					}
				}
//				eatCloseBracket() ;
			}
		}
		else
		{			
			runError( SE_SYNTAX ) ;
			return 0 ;
		}
	}
	else
	{
		runError( SE_SYNTAX ) ;
		return 0 ;
	}
	return sent ;
}

void exec_alert()
{
	uint32_t result ;
	union t_parameter param[2] ;
#ifndef QT
  uint32_t type = ALERT_TYPE ;
#endif
  result = get_parameter( &param[0], PARAM_TYPE_STRING ) ;
	if ( result == 2 )
	{
		result = get_optional_numeric_parameter( &param[1] ) ;
		if ( result == 1 )
		{
#ifndef QT
      if ( param[1].var )
			{
				type = MESS_TYPE ;
			}
#endif
    }
#ifndef QT
    AlertType = type ;
		AlertMessage = (char *)param[0].cpointer ;
#endif
	}
}


void exec_systemStrToArray()
{
	uint8_t opcode ;
	uint16_t val16 ;
	opcode = *RunTime->ExecProgPtr++ ;
	uint32_t value ;
	union t_parameter param ;
	uint32_t result ;

	if ( ( opcode & 0xF6 ) == 0x66 )	// A byte array
	{
		uint32_t dimension ;
		struct byteAddress baddr ;
		if ( getParamByteArrayAddress( &baddr, 0 ) == 0 )
		{
			return ;
		}
		val16 = baddr.varOffset ;
		value = baddr.varIndex ;
		dimension = baddr.dimension ;
		
//		val16 = getVarIndex( opcode ) ;
//		dimension = *RunTime->ExecProgPtr++ ;
//		value = expression() ;
//		opcode = *RunTime->ExecProgPtr++ ;
//		if ( opcode != ']' )
//		{
//			runError( SE_SYNTAX ) ;
//			return ;
//		}
//		if ( value >= dimension )
//		{
//			runError( SE_DIMENSION ) ;
//			return ;
//		}
		// Now we need the index
		
//		if ( *RunTime->ExecProgPtr != ',' )
//		{
//			runError( SE_SYNTAX ) ;
//			return ;
//		}
//		RunTime->ExecProgPtr += 1 ;

		result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
		if ( result == 1 )
		{
			uint8_t *p = 0 ;
			uint32_t length = 0 ;

//			eatCloseBracket() ;
			// Find the string
			switch ( param.var )
			{
				case 0 :
#ifndef QT
					p = (uint8_t *)g_model.name ;
					length = MODEL_NAME_LEN ;
#else
					p = (uint8_t *)"MODEL01   " ;
					length = 10 ;
#endif 
				break ;
				case 1 :
#ifndef QT
					p = (uint8_t *)g_eeGeneral.ownerName ;
					length = GENERAL_OWNER_NAME_LEN ;
#else
					p = (uint8_t *)"OWNER     " ;
					length = 10 ;
#endif 
				break ;
			}

			// Now copy the string
			if ( p )
			{
				while ( length-- )
				{
					uint8_t c ;
					c = *p++ ;
					if ( value < dimension )
					{
						RunTime->ByteArrayStart[val16 + value] = c ;
						value += 1 ;
					}
					else
					{
						RunTime->ByteArrayStart[val16 + dimension - 1] = '\0' ;
					}
				}
				RunTime->ByteArrayStart[val16+value] = '\0' ;
			}
			else
			{
				RunTime->ByteArrayStart[val16+value] = '\0' ;
			}
		}
		else
		{
			runError( SE_SYNTAX ) ;
			return ;
		}
	}
}



#ifndef QT
#if defined(PCBSKY) || defined(PCB9XT)
const uint8_t SwTranslate[] = {
HSW_ThrCt,HSW_RuddDR,HSW_ElevDR,HSW_ID0,HSW_ID1,HSW_ID2,HSW_AileDR,HSW_Gear,HSW_Trainer,
10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,
HSW_Thr3pos0,HSW_Thr3pos1,HSW_Thr3pos2,HSW_Rud3pos0,HSW_Rud3pos1,HSW_Rud3pos2,HSW_Ele3pos0,
HSW_Ele3pos1,HSW_Ele3pos2,HSW_Ail3pos0,HSW_Ail3pos1,HSW_Ail3pos2,HSW_Gear3pos0,HSW_Gear3pos1,
HSW_Gear3pos2,HSW_Ele6pos0,HSW_Ele6pos1,HSW_Ele6pos2,HSW_Ele6pos3,HSW_Ele6pos4,HSW_Ele6pos5,
HSW_Pb1,HSW_Pb2,HSW_Pb3,HSW_Pb4,HSW_Etrmdn,HSW_Etrmup,HSW_Atrmdn,HSW_Atrmup,HSW_Rtrmdn,HSW_Rtrmup,HSW_Ttrmdn,HSW_Ttrmup
} ;
#else
const uint8_t SwTranslate[] = {
HSW_SF2,0,0,HSW_SC0,HSW_SC1,HSW_SC2,0,0,HSW_SH2,
10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,
HSW_SB0,HSW_SB1,HSW_SB2,HSW_SE0,HSW_SE1,HSW_SE2,HSW_SA0,
HSW_SA1,HSW_SA2,HSW_SD0,HSW_SD1,HSW_SD2,HSW_SG0,HSW_SG1,
HSW_SG2,HSW_Ele6pos0,HSW_Ele6pos1,HSW_Ele6pos2,HSW_Ele6pos3,HSW_Ele6pos4,HSW_Ele6pos5,
HSW_Pb1,HSW_Pb2,HSW_Etrmdn,HSW_Etrmup,HSW_Atrmdn,HSW_Atrmup,HSW_Rtrmdn,HSW_Rtrmup,HSW_Ttrmdn,HSW_Ttrmup
} ;
#endif
#endif

int32_t exec_getSwitch()
{
	int32_t number ;
	uint32_t result ;
	union t_parameter param ;
	uint8_t *p ;

	number = -1 ;
	result = get_parameter( &param, PARAM_TYPE_STRING ) ;
	if ( result == 2 )
	{
		p = param.cpointer ;
#ifdef QT
		number = 0 ;
        (void) p ;
#else
		number = basicFindSwitchIndexByName( (char *)p ) ;
		if ( ( number >= 0 ) && (number < (int32_t)sizeof(SwTranslate)) )
		{
			number = SwTranslate[number] ;
			number = getSwitch00( number ) ;
		}
		else
		{
			number = -1 ;
		}
#endif
//		eatCloseBracket() ;
	}
	return number ;
}

extern uint8_t Now_switch[] ;

void exec_setSwitch()
{
	int32_t number ;
	uint32_t result ;
	union t_parameter param ;
	uint8_t *p ;

	result = get_parameter( &param, PARAM_TYPE_STRING ) ;
	if ( result == 2 )
	{
		p = param.cpointer ;
		result = get_parameter( &param, PARAM_TYPE_NUMBER ) ;
		if ( result == 1 )
		{
#ifdef QT			
			number = 0 ;
            (void) number ;
            (void) p ;
#else
			number = basicFindSwitchIndexByName( (char *)p ) ;
			if (number < HSW_MAX)
			{
				number = SwTranslate[number] ;
				number -= 10 ;
				if ( ( number >= 0 ) && ( number < NUM_SKYCSW ) )
				{
  				SKYCSwData &cs = g_model.customSw[number] ;
  				if( cs.func == 0 )
					{
						Now_switch[number] = param.var ? 1 : 0 ;
					}
				}
			}
#endif
    }
	}
}


void exec_playFile()
{
	uint32_t result ;
#ifndef QT
	uint32_t i ;
#endif
	union t_parameter param ;
	uint8_t *p ;
#ifndef QT
  char name[10] ;
#endif

	result = get_parameter( &param, PARAM_TYPE_STRING ) ;
	p = param.cpointer ;
	if ( result == 2 )
	{
#ifndef QT
  	ncpystr( (uint8_t *)name, p, 8 ) ;
		if ( name[0] && ( name[0] != ' ' ) )
		{
			const char *names = PSTR(STR_SOUNDS) ;
			uint32_t nameLength = *names++ ;
			for ( i = 0 ; i < 16 ; i += 1 )
			{
  		  if (strMatch( names, name, nameLength ))
				{
					audio.event( i, 0, 1 ) ;
					break ;
				}
				names += nameLength ;
			}
			if ( i >= 16 )
			{
				putUserVoice( name, 0 ) ;
			}
		}
#else
        (void) p ;
#endif
  }
//	eatCloseBracket() ;
}


//void exec_print()
//{
//	uint8_t opcode ;
//	int32_t value ;

//	opcode = * RunTime->ExecProgPtr++ ;
//	while ( ( opcode & 0x80 ) == 0 )
//	{
//		if ( opcode == 0x70 )	// Quoted string
//		{
//      RunTime->ExecProgPtr += * RunTime->ExecProgPtr + 1 ;	// Skip the string
//		}
//		else
//		{
//			value = expression() ;
//		}
//		opcode = *RunTime->ExecProgPtr++ ;
//	}
//	RunTime->ExecProgPtr -= 1 ;
//}


//int32_t execUserFunction()
//{
//	uint32_t destination ;
//	uint32_t execLinesProcessed ;
//	uint32_t finished ;
//	uint32_t callLevel ;

// 	destination = *RunTime->ExecProgPtr++ ;
// 	destination |= *RunTime->ExecProgPtr++ << 8 ;
// 	if ( RunTime->CallIndex >= MAX_CALL_STACK )
// 	{
// 		runError( SE_TOO_MANY_CALLS ) ;
// 	}
//	callLevel = RunTime->CallIndex ;
// 	RunTime->CallStack[RunTime->CallIndex++] = RunTime->ExecProgPtr ;
// 	RunTime->ExecProgPtr = &Program.Bytes[destination] ;

//	execLinesProcessed = 0 ;
//	while ( execLinesProcessed < 150 )
//	{
//		finished = execOneLine() ;
//		if ( callLevel == RunTime->CallIndex )
//		{
//			break ;
//		}
//	}
//	return RunTime->FunctionReturnValue ;
//}

int32_t execInFunction()
{
	int32_t result = 0 ;
	switch ( *RunTime->ExecProgPtr++ )
	{
		case DRAWCLEAR :
			exec_drawclear() ;
		break ;
		
		case DRAWNUMBER :
			exec_drawnumber() ;
		break ;

		case DRAWTEXT :
			exec_drawtext() ;
		break ;

		case DRAWLINE :
			exec_drawline() ;
		break ;

		case DRAWPOINT :
			exec_drawpoint() ;
		break ;

		case DRAWRECT :
			exec_drawrect() ;
		break ;

		case PLAYNUMBER :
			exec_playnumber() ;
		break ;

		case GETVALUE :
			result = exec_getvalue(0) ;
		break ;

		case GETRAWVALUE :
			result = exec_getvalue(1) ;
		break ;

    case IDLETIME :
			result = exec_idletime() ;
		break ;

    case GETTIME :
			result = exec_gettime() ;
		break ;

    case SPORTSEND :
			result = exec_sportsend() ;
		break ;

    case SPORTRECEIVE :
			result = exec_sportreceive() ;
		break ;

    case NOT :
			result = exec_not() ;
		break ;
    
		case ABS :
			result = exec_abs() ;
		break ;

		case SQRT :
			result = exec_sqrt() ;
		break ;

		case GETLASTPOS :
			result = exec_getlastpos() ;
		break ;
		
		case DRAWTIMER :
			exec_drawtimer() ;
		break ;
		
		case SYSFLAGS :
			result = exec_sysflags() ;
		break ;

		case SETTELITEM :
			result = exec_settelitem() ;
		break ;

		case STRTOARRAY :
			result = exec_strToArray() ;
		break ;

		case BYTEMOVE :
			result = exec_bytemove() ;
		break ;
		
		case ALERT :
 			exec_alert() ;
		break ;

		case GETSWITCH :
			result = exec_getSwitch() ;
		break ;
		 
		case SETSWITCH :
			exec_setSwitch() ;
		break ;

		case PLAYFILE :
			exec_playFile() ;
		break ;

		case KILLEVENTS :
			exec_killEvents() ;
		break ;
		 
		case BITFIELD :
			result = exec_bitField() ;
		break ;

		case POWER :
			result = exec_power() ;
		break ;

		case CROSSFIRERECEIVE :
			result = exec_crossfirereceive() ;
		break ;
		 
		case CROSSFIRESEND :
			result = exec_crossfiresend() ;
		break ;

		case BTRECEIVE :
			result = exec_btreceive() ;
		break ;

		case BTSEND :
			result = exec_btsend() ;
		break ;

#ifdef FILE_SUPPORT
		case DIRECTORY :
			exec_directory() ;
		break ;
			
		case FILESELECT :
			result = exec_filelist() ;
		break ;
		
		case FOPEN :
			result = exec_fopen() ;
		break ;

		case FREAD :
			result = exec_fread() ;
		break ;

		case FWRITE :
			result = exec_fwrite() ;
		break ;

		case FCLOSE :
			exec_fclose() ;
		break ;
#endif

		case RETURNVALUE :
			result = exec_returnvalue() ;
		break ;

		case RESETTELEMETRY :
			exec_resettelemetry() ;
		break ;
		
		case POPUP :
			result = exec_popup() ;
		break ;
		 
		case SYSTOARRAY :
      exec_systemStrToArray() ;
		break ;

		case DRAWBITMAP  :
      exec_drawbitmap() ;
		break ;
			
		default :
			runError( SE_NO_FUNCTION ) ;
		break ;
	}
	if ( *RunTime->ExecProgPtr++ != ')' )
	{
		runError( SE_NO_BRACKET ) ;
	}
	return result ;
}


//uint8_t BasicDump[32] ;

// Return values:
// 0 Still running or RunError
// 1 Found Finish
// 2 Found Stop
uint32_t execOneLine()
{
	uint8_t opcode ;
	uint16_t val16 ;
//	uint32_t val32 ;
	int32_t value ;

//	ExpressionIndex = 0 ;
	opcode = *RunTime->ExecProgPtr++ ;
	if ( ( opcode & 0x80 ) == 0 )
	{
		return runError( SE_EXEC_BADLINE ) ;
	}
  RunTime->RunLineNumber = (opcode & 0x7F) | (*RunTime->ExecProgPtr++ << 7 ) ;
	do
	{
		opcode = *RunTime->ExecProgPtr++ ;
		if ( opcode & 0x80 )	// LineNumber
		{
			break ;
		}

		if ( opcode == STOP )
		{
			return 2 ;
		}
		if ( opcode == FINISH )
		{
			return 1 ;
		}
		if ( ( opcode & 0xF0 ) == 0x60 )
		{
			uint32_t destType = 0 ;
			uint32_t dimension ;
			uint32_t assignType = SETEQUALS ;
			// assign
			// Test for array
			if ( opcode & 0x04 )
			{ // An array
				destType = 2 ;
				if ( opcode & 0x02 )
				{
					destType |= 1 ;
				}
				val16 = getVarIndex( opcode ) ;
				dimension = *RunTime->ExecProgPtr++ ;

//				if ( opcode & 0x01 )	// 16-bit dimension
//				{
//					dimension |= *RunTime->ExecProgPtr++ << 8 ;
//				}

				value = expression() ;
				opcode = *RunTime->ExecProgPtr++ ;
				if ( opcode != ']' )
				{
					return runError( SE_SYNTAX ) ;
				}
				if ( (uint32_t) value >= dimension )
				{
					return runError( SE_DIMENSION ) ;
				}
				val16 += value ;
			}
			else
			{
				val16 = getVarIndex( opcode ) ;
			}
			opcode = *RunTime->ExecProgPtr++ ;
			if ( opcode != '=' )
			{
				switch ( opcode )
				{
					case '+' :
						assignType = PLUSEQUALS ;
					break ;
					case '-' :
						assignType = MINUSEQUALS ;
					break ;
					case '*' :
						assignType = TIMESEQUALS ;
					break ;
					case '/' :
						assignType = DIVIDEEQUALS ;
					break ;
					case '%' :
						assignType = PERCENTEQUALS ;
					break ;
					case '&' :
						assignType = ANDEQUALS ;
					break ;
					case BITOR :
						assignType = OREQUALS ;
					break ;
					case BITXOR :
						assignType = XOREQUALS ;
					break ;
					default :
//						{
////							uint8_t *p = RunTime->ExecProgPtr - 8 ;
//							uint8_t *p = &Program.Bytes[4+16] ;
//							uint32_t i ;
//							for ( i = 0 ; i < 32 ; i += 1 )
//							{
//								BasicDump[i] = *p++ ;
//							}
//						}	
						return runError( SE_SYNTAX ) ;
					break ;
				}
				opcode = *RunTime->ExecProgPtr++ ;
				if ( opcode != '=' )
				{
					return runError( SE_SYNTAX ) ;
				}
			}
			value = expression() ;
			if ( assignType != SETEQUALS )
			{
				int32_t oldValue ;
				if ( destType )
				{
					if ( destType & 1 )
					{
						oldValue = RunTime->ByteArrayStart[val16] ;
					}
					else
					{
						oldValue = RunTime->IntArrayStart[val16] ;
					}
				}
				else
				{
					oldValue = RunTime->Vars.Variables[val16] ;
				}
				switch ( assignType )
				{
					case PLUSEQUALS :
						oldValue += value ;
					break ;
					case MINUSEQUALS :
						oldValue -= value ;
					break ;
					case TIMESEQUALS :
						oldValue *= value ;
					break ;
					case DIVIDEEQUALS :
						oldValue /= value ;
					break ;
					case PERCENTEQUALS :
						oldValue %= value ;
					break ;
					case ANDEQUALS :
						oldValue &= value ;
					break ;
					case OREQUALS :
						oldValue |= value ;
					break ;
					case XOREQUALS :
						oldValue ^= value ;
					break ;
				}
				value = oldValue ;
			}
			if ( destType )
			{
				if ( destType & 1 )
				{
					RunTime->ByteArrayStart[val16] = value ;
				}
				else
				{
					RunTime->IntArrayStart[val16] = value ;
				}
			}
			else
			{
				RunTime->Vars.Variables[val16] = value ;
			}
		}
		else
		{
			switch ( opcode )
			{
				case IF :
				case ELSEIF :
					exec_if() ;
				break ;

				case GOTO :
					exec_goto() ;
				break ;

				case GOSUB :
					exec_gosub() ;
				break ;
			
				case RETURN :
					exec_return() ;
				break ;

				case IN_FUNCTION :
					execInFunction() ;
				break ;

				case WHILE :
					exec_while() ;
				break ;

//				case PRINT :
//					exec_print() ;
//				break ;

				case END :
					return 1 ;
				break ;
			}
		}
	} while ( ( opcode & 0x80 ) == 0 ) ;
	RunTime->ExecProgPtr -= 1 ;
	return 0 ;
}

// Return values:
// 0 
// 1 After begin or Still running or RunError
// 2 Found Stop
// 3 Script finished (unloaded)

// events need queueing
uint16_t BasicEventQueue0[3] ;
uint16_t BasicEventQueue1[3] ;

uint32_t basicExecute( uint32_t begin, uint16_t event, uint32_t index )
{
	uint32_t execLinesProcessed ;
	uint32_t finished ;
	uint32_t i ;
	uint32_t j ;
	j = LoadedScripts[index].offsetOfStart ;
	i = j / 4 ;
	RunTime = (struct t_basicRunTime *) &Program.Words[Program.Words[i]] ;

	if ( begin )
	{
		RunTime->ExecProgPtr = &Program.Bytes[j+8] ;
    RunTime->RunError = 0 ;
		RunTime->CallIndex = 0 ;
    RunTime->ParamFrameIndex = 0 ;
    RunTime->ParamStackIndex = 0 ;

    j = (uint32_t *)&RunTime->Vars.Variables[0] - &Program.Words[i] ;
		j = Program.Words[i+1] / 4 - j ;

		for ( i = 0 ; i < j ; i += 1 )
		{
			RunTime->Vars.Variables[i] = 0 ;
		}
		RunTime->Vars.Variables[1] = LoadedScripts[index].size ;
		ScriptPopupActive = 0 ;
		return 1 ;
	}

#ifndef QT
  if ( index == 0 )
	{
    if ( BasicLoadedType == BASIC_LOAD_ALONE )
		{
			if ( event == EVT_KEY_LONG(KEY_EXIT) )
			{
      	killEvents( event ) ;
				LoadedScripts[0].loaded = 0 ;
				BasicLoadedType = BASIC_LOAD_NONE ;
				basicLoadModelScripts() ;
				return 3 ;
			}
		}
	}
#endif
	if ( RunTime->ExecProgPtr != &Program.Bytes[j+8] )
	{
		// Resuming, not at start
#ifndef QT
    ScriptFlags |= SCRIPT_RESUME ;
#endif
		if ( event )
		{
			if ( BasicEventQueue0[index] )
			{ 
				BasicEventQueue1[index] = event ;
			}
			else
			{
				BasicEventQueue0[index] = event ;
				BasicEventQueue1[index] = 0 ;
			}
		}	
	}
	else
	{
		// Starting at top
		uint16_t tempEvent ;
		if ( BasicEventQueue0[index] )
		{
			tempEvent = BasicEventQueue0[index] ;
			if ( BasicEventQueue1[index] )
			{
				BasicEventQueue0[index] = BasicEventQueue1[index] ;
				BasicEventQueue1[index] = event ;
			}
			else
			{
				BasicEventQueue0[index] = event ;
			}
			event = tempEvent ;
		}
		RunTime->Vars.Variables[0] = event & 0x00FF ;	// Mask off rotary flag
		// Clear call stack?
		RunTime->CallIndex = 0 ;
	}
	if ( event & 0x0100 )		// A rotary movement event
	{
    ScriptFlags |= SCRIPT_ROTARY ;
		event &= 0x00FF ;
	}

	execLinesProcessed = 0 ;
#ifndef QT
	uint16_t t1 = getTmr2MHz() ;
#endif
	while ( execLinesProcessed < 250 )
	{
		finished = execOneLine() ;

#ifdef QT
		uint32_t i ;
		for ( i = 0 ; i < 100 ; i += 1 )
		{
			RunTimeBuffer[i] = Program.Bytes[i] ;			
		}
#endif
		if ( finished == 1 )	// Found Finish
		{
			finished = 3 ;
#ifndef QT
      killEvents( event & 0x00FF ) ;
			LoadedScripts[index].loaded = 0 ;
#endif
      break ;
		}
		if ( finished == 2 )
		{
			RunTime->ExecProgPtr = &Program.Bytes[j+8] ;
			break ;
		}
		finished = 1 ;
		if ( RunTime->RunError )
		{
#ifndef QT
			LoadedScripts[index].loaded = 0 ;
			if ( BasicLoadedType == BASIC_LOAD_ALONE )
			{
				BasicLoadedType = BASIC_LOAD_NONE ;
			}
#endif
			break ;
		}
		execLinesProcessed += 1 ;
#ifndef QT
		if ( (uint16_t)( getTmr2MHz() - t1 ) > 2000 )
		{
			break ;
		}
#endif

	}
	if ( RunTime->RunError )
	{
#ifdef QT			
    sprintf( InputLine, "Error %d at line %d", RunTime->RunError+100, RunTime->RunErrorLine ) ;
		cpystr( ErrorReport, (uint8_t *)InputLine ) ;
#else
		setErrorText( RunTime->RunError+100, RunTime->RunErrorLine ) ;
#endif
		return RunTime->RunError ;
	}
	return finished ;
}

#ifndef QT			
int32_t basicFindValueIndexByName( const char * name )
{
	uint32_t i ;
	const char *names = PSTR(STR_TELEM_ITEMS) ;
	uint32_t nameLength = *names++ ;

	for ( i = 0 ; i < NUM_TELEM_ITEMS ; i += 1 )
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

	if (strMatch( "LAT", name, 3 ))
	{
		return -2 ;
	}
	if (strMatch( "LONG", name, 4 ))
	{
		return -3 ;
	}
  return -1 ;  // not found
}

int32_t basicFindSwitchIndexByName( const char * name )
{
	const char *names = PSTR(SWITCHES_STR) ;
	uint32_t i ;
	uint32_t nameLength = *names++ ;

	for ( i = 0 ; i < sizeof(SwTranslate) ; i += 1 )
	{
		if ( i == sizeof(SwTranslate) - 8 )
		{
			names = "\003EtdEtuAtdAtuRtdRtuTtuTtd" ;
			nameLength = *names++ ;
		}
    if (strMatch( names, name, nameLength ))
		{
			return i ;
		}
		names += nameLength ;
	}
	return -1 ;  // not found
}


#endif

