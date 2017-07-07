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
#else
/* lcd outdez flags */
#define LEADING0      0x10
#define PREC1         0x20
#define PREC2         0x30 /* 4 modes in 2bits! */
#define LEFT          0x40 /* align left */

uint8_t ScriptFlags ;

#endif

//#ifndef	QT
//extern uint16_t BasicDebug1 ;
//extern uint16_t BasicDebug2 ;
//extern uint16_t BasicDebug3 ;
//extern uint16_t BasicDebug4 ;
//#endif

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


#define IF   				1
#define THEN 				2
#define GOTO 				3
#define EOL  				4
#define FINISHED  	5
#define GOSUB 			6
#define RETURN  		7
#define STOP 				8
#define END					9
#define LET						10
#define REM	        	11
#define LESSEQUAL   	12
#define GREATEQUAL  	13
#define IN_FUNCTION		14
#define USER_FUNCTION	15
#define FINISH				16
#define DEFARRAY			17
#define DEFINT				18
#define DEFBYTE				19
#define NGOTO					20
#define WHILE					21

//#define FOR  					25
//#define NEXT 					22
//#define TO   					23
//#define STEP					24
//#define WEND					26
//#define REPEAT				27
//#define UNTIL					28
//#define ELSE					29

//#define CONTINUE 31
//#define BREAK	30
//#define FUNC	37
//#define WAIT	18
//#define TRACE	19

#define HASHCHAR		35
#define PERCENT			37
#define BITAND			38
#define OPENPAR			40	
#define CLOSEPAR		41
#define MULTIPLY		42	
#define PLUS				43
#define COMMA				44
#define MINUS       45
#define	DOT					46
#define DIVIDE		  47
#define COLON				58
#define SEMICOLON		59		
#define LESS				60
#define EQUALS		  61
#define GREATER			62	
#define BITOR				63
// UNARY_NOT ??

				 
// Internal Functions
#define DRAWCLEAR		1
#define DRAWNUMBER	2
#define DRAWLINE		3
#define DRAWTEXT		4
#define PLAYNUMBER	5
#define GETVALUE		6
#define DRAWPOINT		7
#define DRAWRECT		8
#define IDLETIME		9
#define GETTIME			10
#define SPORTSEND		11
#define SPORTRECEIVE	12
#define NOT					13
#define ABS					14
#define GETLASTPOS	15
#define DRAWTIMER		16
#define SYSFLAGS		17
#define SETTELITEM	18

// Assignment options:
#define SETEQUALS			0
#define PLUSEQUALS		1
#define MINUSEQUALS		2
#define TIMESEQUALS		3
#define DIVIDEEQUALS	4
#define PERCENTEQUALS	5

// Type for struct t_control
#define TYPE_IF				0
#define TYPE_WHILE		1


// & = 0x26 = 38
// | = 0x7C - problem

// More draw functions:
// refresh ?
// filled rectangle
// timer
// switch
// screen title
// getlastpos

// More general interface items
// getDateTime
// playFile
// sportTelemetryPush
// sportTelemetryPop


// Symbol types
#define SYM_LABEL 		1
#define SYM_VARIABLE	2
#define SYM_FUNCTION	3
#define SYM_ARRAY			4

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

#define MAX_VARIABLES	100

#define MAX_CALL_STACK	20
#define MAX_PARAM_STACK	40

//#define MAX_EXP_STACK	10

// Compiled coding:
// 1 - 0x3F, opcodes
// 0x80 Line number in 15 bits				1xxx
// 0x70 String                        0111
// 0x60 Variable, index in 8 bits			0110
// 0x68 Variable, index in 16 bits
// 0x64 Variable int array, index in 8 bits, dimension in 8 bits
// 0x6C Variable int array, index in 16 bits, dimension in 8 bits
// 0x66 Variable byte array, index in 8 bits, dimension in 8 bits
// 0x6E Variable byte array, index in 16 bits, dimension in 8 bits
// 0x58 Number in 8 bits ??           010x
// 0x50 Number in 32 bits
// 0x48 Number in 3 bits (0-7)
// 0x40 Number in 16 bits


union t_parameter
{
	int32_t var ;
	uint8_t *cpointer ;
} ;

union t_varAddress
{
	uint8_t *bpointer ;
	int32_t *ipointer ;
} ;


uint32_t LineNumber ;
//uint32_t ExecLinesProcessed ;
uint16_t ParseErrorLine ;
uint8_t ParseError ;
uint8_t BasicLoadedType ;


struct t_basicRunTime
{
	uint32_t ParameterIndex ;
	uint32_t RunError ;
	uint32_t RunErrorLine ;
	uint32_t RunLineNumber ;
	uint8_t *ExecProgPtr ;
	uint32_t ExecLinesProcessed ;
	uint32_t CallIndex ;
	uint8_t *CallStack[MAX_CALL_STACK] ;
	union t_parameter ParameterStack[MAX_PARAM_STACK] ;
	int32_t Variables[MAX_VARIABLES] ;
	union t_array	// Place holder, the arrays extend as declared
	{
		uint8_t byteArray[8] ;
		int32_t intArray[2] ;
	} Arrays ;
} ;

struct t_control
{
	uint16_t type ;
	uint16_t first ;
	uint16_t middle ;
} ;

#define MAX_CONTROL_INDEX		10
struct t_control CodeControl[MAX_CONTROL_INDEX] ;		// Max Nesting of control structures
uint8_t ControlIndex ;

// struct t_basicRunTime ThisRunTime ;

struct t_basicRunTime *RunTime ;

#ifdef	QT
uint32_t basicExecute( uint32_t begin, uint8_t event ) ;
int32_t expression( void ) ;
#endif
#ifndef	QT
int32_t expression( void ) ;
#endif

#ifndef	QT
int32_t basicFindValueIndexByName( const char * name ) ;
#endif

// Symbols
// Label: EntryLen, SYM_LABEL, StrLen, String/0, SYM_LAB_DEF/REF, Poslow, Poshi
// Variable: EntryLen, SYM_VARIABLE, StrLen, String/0, SYM_VAR_INT/UNS/FLOAT, Poslow, Poshi
// ArrayVariable: EntryLen, SYM_VARIABLE, StrLen, String/0, SYM_VAR_ARRAY_INT/UNS/FLOAT, Poslow, Poshi, dimension

struct commands
{ /* keyword lookup table */
  char command[22] ;
  uint16_t tok ;
} ;

const struct commands Table[] =
{ 		/* Commands must be entered lowercase */
//  { "print", PRINT},
//  { "input", INPUT},
  { "if", IF},
  { "then", THEN},
  { "goto", GOTO},
//  { "for", FOR},
//  { "next", NEXT},
//  { "to", TO},
//  { "step", STEP},
  { "gosub", GOSUB},
  { "return", RETURN},
  { "let", LET},
  { "rem", REM},
  { "stop", STOP},
  { "finish", FINISH },
//  { "wait", WAIT},
//  { "trace", TRACE},
  { "while", WHILE},
//  { "wend", WEND},
///  { "break", BREAK},
//  { "continue", CONTINUE},
//  { "repeat", REPEAT},
//  { "until", UNTIL},
//  { "else", ELSE},
//  { "function", FUNC},
	{ "int", DEFINT},
	{ "byte", DEFBYTE},
	{ "array", DEFARRAY},
	{ "end", END},
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
//setSwitch( L3, "v<val", "batt", 73, L2 )
//  { "setTelemetryValue", luaSetTelemetryValue },
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
#ifndef	QT
  { "DBLSIZE", DBLSIZE },
  { "INVERS", INVERS },
  { "BLINK", BLINK },
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
//  { "SOLID", SOLID },
//  { "DOTTED", DOTTED },
//  { "FORCE", FORCE },
//  { "ERASE", ERASE },
//  { "ROUND", ROUND },
  { "LCD_W", LCD_W },
  { "LCD_H", LCD_H },
#endif  
	{ "", 0 } /* mark end of table */
} ;



int32_t FirstLabel ;
int32_t PreviousToken ;

// In program:
// Byte 
// LineNumber

#ifdef	QT
uint8_t *RunTimeData ;
uint8_t RunTimeBuffer[100] ;
#endif

#define PROGRAM_SIZE	6000

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
#else
FIL File ;
#endif
char InputLine[200] ;

#ifdef	QT
uint8_t Data[8192] ;
#endif

#ifndef	QT
uint8_t BasicErrorText[48] ;
#endif


#ifndef	QT
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
#endif

int32_t asctoi( char *string )
{
	int32_t result = 0 ;
	uint32_t digit ;
	uint32_t base ;

    /*
     * Skip any leading blanks.
     */

//  while (isspace(*string))
//	{
//		string += 1;
//  }

    /*
     * Check for a sign.
     */

//	if (*string == '-')
//	{
//		sign = 1 ;
//		string += 1 ;
//  }
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
		if ( isdigit( digit ) )
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

  return result ;
//  return sign ? - result : result ;
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
	utoasc( p, line ) ;
	AlertType = ALERT_TYPE ;
	AlertMessage = "Script Error" ;
}

#endif

int openFile( char *filename )
{
#ifdef	QT
	File = fopen( filename,"r") ;
	return File ? 1 : 0 ;
#else
	if ( f_open( &File, filename, FA_READ) == FR_OK )
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
	f_close( &File ) ;
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
  if(strchr(" :;,+-<>/*%=#()&|[]", c) || c==9 || c=='\n' || c=='\r' || c==0) // removed '^'
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

uint32_t serror( uint32_t x )
{
	ParseError = x ;
	ParseErrorLine = LineNumber ;
	return 0 ;
}

uint32_t runError( uint32_t x )
{
	RunTime->RunError = x ;
  RunTime->RunErrorLine = RunTime->RunLineNumber ;
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

  if(strchr("+-*/%=#;(),><&|]", *ProgPtr))
	{ /* delimiter */
		char c ;
    c = *ProgPtr ;
		if ( c == '|' )
		{
			c = BITOR ;
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
			*temp++ = *ProgPtr++ ;
		}
    if(*ProgPtr == '\n')
		{
			serror(SE_SYNTAX) ;
		}
    ProgPtr += 1 ;
		*temp = 0 ;
		Token[0] = temp - Token ;
//    //printf( "QUOTE\n" );
    return( Token_type = QUOTE ) ;
  }
  
  if(isdigit(*ProgPtr))
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
	}
	r = &Program.Bytes[StartOfSymbols] ;
	uint32_t i ;
	uint32_t j ;
	j = 0 ;
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
  p = cpystr( p, (uint8_t *)"\nCode:\n" ) ;
	r = &Program.Bytes[4] ;
	j = 0 ;
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

void addSymbol( uint8_t type, uint8_t sub_type, uint16_t value, uint8_t dimension )
{
	uint32_t tindex ;
	uint32_t tstart ;
	uint32_t i ;	
	
	tstart = EndOfSymbols++ ;
	Program.Bytes[EndOfSymbols++] = type ;
	tindex = EndOfSymbols++ ;
	i = 0 ;
	while ( Token[i] )
	{
		Program.Bytes[EndOfSymbols++] = Token[i++] ;
	}
	Program.Bytes[tindex] = EndOfSymbols - tindex ;
	Program.Bytes[EndOfSymbols++] = '\0' ;
	Program.Bytes[EndOfSymbols++] = sub_type ;
	Program.Bytes[EndOfSymbols++] = value ;
	Program.Bytes[EndOfSymbols++] = value >> 8 ;
	if ( sub_type & SYM_VAR_ARRAY_TYPE )
	{
		Program.Bytes[EndOfSymbols++] = dimension ;
	}
	Program.Bytes[tstart] = EndOfSymbols - tstart ;
}

#ifdef QT	
void loadBasic( char *fileName, uint32_t type ) ;
uint32_t basicTask( uint8_t event, uint8_t flags ) ;

void parse()
{
	uint32_t i ;
  uint32_t j ;
  uint8_t *p ;
	loadBasic( (char *)"C:/Data/C/Interpreter/test.bas", 0 ) ;
	for ( i = 0 ; i < 300 ; i += 1 )
	{
		if ( basicTask( 0, 0 ) == 2 )
		{
			break ;
		}
	}
	p = RunTimeData ;
	uint8_t *r ;
	r = (uint8_t *) RunTime->Variables ;
	j = 0 ;
	for ( i = 0 ; i < 48 ; i += 1 )
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
  *p = '\0' ;
}
#endif

// States
#define BASIC_IDLE				0
#define BASIC_LOAD_START	1
#define BASIC_LOADING			2
#define BASIC_RUNNING			3

uint8_t BasicState ;
#ifdef QT	
uint8_t *QtPtr ;
#endif

char LastBasicFname[60] ;

void loadBasic( char *fileName, uint32_t type )
{
	FirstLabel = -1 ;
	PreviousToken = -1 ;
	StartOfSymbols = 100 ;
	EndOfSymbols = 100 ;
	CurrentPosition = 4 ;
	CurrentArrayIndex = 0 ;
	CurrentVariableIndex = 0 ;
	LineNumber = 0 ;

	cpystr( (uint8_t *)LastBasicFname, (uint8_t *)fileName ) ;
  if ( openFile( fileName ) )
	{
    cpystr( (uint8_t *)Token, (uint8_t *)"Event" ) ;
    addSymbol( SYM_VARIABLE, SYM_VAR_INT, CurrentVariableIndex++, 0 ) ;
		BasicState = BASIC_LOADING ;
#ifdef QT	
		Data[0] = 0 ;
		QtPtr = Data ;
#else
		BasicLoadedType = type ;
#endif

	}
	else
	{
		BasicState = BASIC_IDLE ;
		BasicLoadedType = BASIC_LOAD_NONE ;
	}
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

#ifdef QT	
	p = QtPtr ;
#endif

	q = 0 ;
	for ( i = 0 ; i < 200 ; i += 1 )
	{
		j = StartOfSymbols - CurrentPosition ;
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
    q = (uint8_t *)fgets( InputLine, 200, File ) ;
#else
    q = (uint8_t *)f_gets( InputLine, 200, &File ) ;
#endif
		LineNumber += 1 ;
		Program.Bytes[CurrentPosition++] = ( LineNumber & 0x7F ) | 0x80 ;
		Program.Bytes[CurrentPosition++] = LineNumber >> 7 ;
		processWhile = 0 ;
		if ( q )
		{
#ifdef QT
			p = cpystr( p, q ) ; 
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
							Program.Bytes[CurrentPosition++] = NGOTO ;
							NgotoLocation = CurrentPosition ;
							Program.Bytes[CurrentPosition++] = 0 ;
							Program.Bytes[CurrentPosition++] = 0 ;
						}
					}
					
					if ( Tok == REM )
					{
#ifdef QT
            p = cpystr( p, (uint8_t *)"Rem" ) ;
#endif
						CurrentPosition -= 2 ; // Remove line number
						break ;
					}
					else if ( Tok == LET )
					{
#ifdef QT
            p = cpystr( p, (uint8_t *)"Let" ) ;
#endif
						continue ;
					}
					else if ( Tok == DEFARRAY )
					{
						uint32_t type = SYM_VAR_ARRAY_INT ;
#ifdef QT
            p = cpystr( p, (uint8_t *)"Array " ) ;
#endif
						CurrentPosition -= 2 ; // Remove line number
						// What is array type
						j = scanForKeyword( Token ) ;
						if ( Tok == DEFBYTE )
						{
							type = SYM_VAR_ARRAY_BYTE ;
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
								if ( value >= 0 && value <= 100 )
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
					else if ( Tok == WHILE )
					{
						processWhile = 1 ;
						CodeControl[ControlIndex].type = TYPE_WHILE ;
						CodeControl[ControlIndex].first = CurrentPosition - 2 ;
					}
					else if ( Tok == END )
					{
						if ( ControlIndex )
						{
							ControlIndex -= 1 ;
							if ( CodeControl[ControlIndex].type == TYPE_WHILE )
							{
								Program.Bytes[CurrentPosition++] = GOTO ;
								Program.Bytes[CurrentPosition++] = CodeControl[ControlIndex].first ;
                Program.Bytes[CurrentPosition++] = CodeControl[ControlIndex].first >> 8 ;
                Program.Bytes[CodeControl[ControlIndex].middle] = CurrentPosition ;
								Program.Bytes[CodeControl[ControlIndex].middle+1] = CurrentPosition >> 8 ;
							}
						}
//						else
//						{
//							serror( SE_SYNTAX ) ;
//						}
						continue ;
					}
					if ( ( Tok != EOL ) && ( Tok != FINISHED ) )
					{
#ifdef QT
						*p++ = '[' ;
						sprintf( Tbuf, "%d,%d", j, Tok ) ;
            q = (uint8_t *)Tbuf ;
						while ( *q )
						{
							*p++ = *q++ ;
						}
						*p++ = ']' ;
#endif
						Program.Bytes[CurrentPosition++] = Tok ;
					}
					else
					{
						if ( NgotoLocation )
						{
							Program.Bytes[NgotoLocation++] = CurrentPosition ;
							Program.Bytes[NgotoLocation] = CurrentPosition >> 8 ;
							NgotoLocation = 0 ;
						}
						if ( processWhile )
						{
							Program.Bytes[CurrentPosition++] = NGOTO ;
							CodeControl[ControlIndex].middle = CurrentPosition ;
							Program.Bytes[CurrentPosition++] = 0 ;
							Program.Bytes[CurrentPosition++] = 0 ;
							ControlIndex += 1 ;
							processWhile = 0 ;
						}
					}
					PreviousToken = Tok ;
				}
				else
				{
#ifdef QT
					*p++ = '{' ;
					sprintf( Tbuf, "%d,", j ) ;
          q = (uint8_t *)Tbuf ;
					while ( *q )
					{
						*p++ = *q++ ;
					}
#endif
					if ( PreviousToken == THEN )
					{
//						if ( Tok != GOTO && Tok != GOSUB )
//					  {
							// Must be statement, put in NGOTO
							Program.Bytes[CurrentPosition++] = NGOTO ;
							NgotoLocation = CurrentPosition ;
							Program.Bytes[CurrentPosition++] = 0 ;
							Program.Bytes[CurrentPosition++] = 0 ;
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
#ifdef QT
	              p = cpystr( p, (uint8_t *)"Label " ) ;
                p = cpystr( p, (uint8_t *)Token ) ;
#endif
								loc = findSymbol( SYM_LABEL ) ;
                if ( loc == -1 )
								{
                	addSymbol( SYM_LABEL, SYM_LAB_REF, CurrentPosition, 0 ) ;
									Program.Bytes[CurrentPosition++] = 0 ;
									Program.Bytes[CurrentPosition++] = 0 ;
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

										Program.Bytes[loc++] = CurrentPosition ;
										Program.Bytes[loc] = CurrentPosition >> 8 ;
										Program.Bytes[CurrentPosition++] = index ;
										Program.Bytes[CurrentPosition++] = index >> 8 ;
									}
									else
									{
										Program.Bytes[CurrentPosition++] = Program.Bytes[loc++] ;
										Program.Bytes[CurrentPosition++] = Program.Bytes[loc] ;
									}
								}
							}
							else
							{
								int32_t index ;
								uint8_t code ;
#ifdef QT
	              p = cpystr( p, (uint8_t *)"Var " ) ;
								*p++ = Token[0] ;
#endif
								index = findSymbol( SYM_VARIABLE ) ;
								if ( index == -1 )
								{
									if ( CurrentVariableIndex >= MAX_VARIABLES )
									{
                    serror( SE_TOO_MANY_VARS ) ;
									}
									index = CurrentVariableIndex++ ;
                	addSymbol( SYM_VARIABLE, SYM_VAR_INT, index, 0 ) ;
								}
								else
								{
									index += Program.Bytes[index] - 2 ; // Index of index
									index = Program.Bytes[index] | (Program.Bytes[index+1] << 8) ;
								}
								code = 0x60 ;
								if ( index > 255 )
								{
									code = 0x68 ;
								}
								Program.Bytes[CurrentPosition++] = code ;
								Program.Bytes[CurrentPosition++] = index ;
								if ( code == 0x68 )
								{
									Program.Bytes[CurrentPosition++] = index >> 8 ;
								}
							}
						}
						break ;

						case ARRAY :
						{	
							int32_t index ;
							uint32_t type ;
							uint32_t dimension ;
							uint8_t code ;
							index = findSymbol( SYM_ARRAY ) ;
							if ( index == -1 )
							{
								serror( SE_SYNTAX ) ;
							}
							else
							{
								index += Program.Bytes[index] - 3 ; // Index of index
								type = Program.Bytes[index-1] ;
								dimension = Program.Bytes[index+2] ;
								index = Program.Bytes[index] | (Program.Bytes[index+1] << 8) ;
								code = 0x64 ;
								if ( type == SYM_VAR_ARRAY_BYTE )
								{
									code = 0x66 ;
								}
								if ( index > 255 )
								{
									code |= 0x08 ;
								}
								Program.Bytes[CurrentPosition++] = code ;
								Program.Bytes[CurrentPosition++] = index ;
								if ( code &= 0x08 )
								{
									Program.Bytes[CurrentPosition++] = index >> 8 ;
								}
								Program.Bytes[CurrentPosition++] = dimension ;
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
#ifdef QT
              p = cpystr( p, (uint8_t *)"Const " ) ;
							sprintf( Tbuf, "%d,", value ) ;
              p = cpystr( p, (uint8_t *) Tbuf ) ;
#endif
							if ( value <=7 )
							{
                code = 0x48 + value ;
							}
							else if ( value <= 127 )
							{
								code = 0x58 ;
								bytes = 1 ;
							}
							else if ( value <= 32767 )
							{
								code = 0x40 ;
								bytes = 2 ;
							}
							else
							{
								code = 0x50 ;
								bytes = 4 ;
							}
							Program.Bytes[CurrentPosition++] = code ;
							while ( bytes-- )
							{
								Program.Bytes[CurrentPosition++] = value ;
								value >>= 8 ;
							}
						}	
						break ;

						case NUMBER :
						{
							int32_t value ;	
							uint8_t code ;
							uint32_t bytes = 0 ;
							value = asctoi( Token ) ;
#ifdef QT
              p = cpystr( p, (uint8_t *)"Num " ) ;
              p = cpystr( p, (uint8_t *)Token ) ;
#endif
							code = 0x40 ;
							if ( value >=0 && value <=7 )
							{
                code = 0x48 + value ;
							}
							else if ( value >= -128 && value <= 127 )
							{
								code = 0x58 ;
								bytes = 1 ;
							}
							else if ( value >= -32768 && value <= 32767 )
							{
								code = 0x40 ;
								bytes = 2 ;
							}
							else
							{
								code = 0x50 ;
								bytes = 4 ;
							}
							Program.Bytes[CurrentPosition++] = code ;
							while ( bytes-- )
							{
								Program.Bytes[CurrentPosition++] = value ;
								value >>= 8 ;
							}
						}
						break ;
						case DELIMITER :
						{	
#ifdef QT
              p = cpystr( p, (uint8_t *)"Delim " ) ;
							if ( Token[0] == LESSEQUAL )
							{
								*p++ = '<' ;
								*p++ = '=' ;
							}
							else if ( Token[0] == GREATEQUAL )
							{
								*p++ = '>' ;
								*p++ = '=' ;
							}
							else
							{
								*p++ = Token[0] ;
							}
#endif
							Program.Bytes[CurrentPosition++] = Token[0] ;
						}
						break ;
						case LABEL :
						{	
							int32_t loc ;
							uint32_t index ;
							uint32_t type ;
#ifdef QT
              p = cpystr( p, (uint8_t *)"Label " ) ;
              p = cpystr( p, (uint8_t *)Token ) ;
							// Define a label
#endif
							loc = findSymbol( SYM_LABEL ) ;
							if ( loc == -1 )
							{
                addSymbol( SYM_LABEL, SYM_LAB_DEF, CurrentPosition, 0 ) ;
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
                  do
									{
										index = Program.Bytes[loc] ;
										index += Program.Bytes[loc+1] << 8 ;
										Program.Bytes[loc++] = CurrentPosition ;
										Program.Bytes[loc] = CurrentPosition >> 8 ;
										loc = index ;
									} while ( index ) ;
								}
							}
						}
						break ;
						case QUOTE :	// Quoted string
						{
							uint8_t *temp ;
              uint8_t c ;
              temp = (uint8_t *)Token ;
							Program.Bytes[CurrentPosition++] = 0x70 ;
							do
							{
								c = *temp++ ;
								Program.Bytes[CurrentPosition++] = c ;
							} while ( c ) ;
						}	
						break ;
						case FUNCTION :
						{
							uint32_t inFunction ;
							inFunction = look_up( Token, InternalFunctions ) ;
							if ( inFunction )
							{
								Program.Bytes[CurrentPosition++] = IN_FUNCTION ;
								Program.Bytes[CurrentPosition++] = inFunction ;
							}
							else
							{
								// No user functions (yet)
								serror( SE_NO_FUNCTION ) ;
							}
						}			 
						break ;
					}
#ifdef QT
					*p++ = '}' ;
#endif
          PreviousToken = -1 ;
        }
				if ( ParseError )
				{
#ifdef QT
					sprintf( InputLine, "Error %d at line %d", ParseError, ParseErrorLine ) ;
#else
					setErrorText( ParseError, ParseErrorLine ) ;
#endif
					break ;
				}
			} while( Tok != FINISHED ) ;
#ifdef QT
			*p++ = '\n' ;
#endif
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

	Program.Bytes[CurrentPosition++] = STOP ;
	closeFile() ;
	 
	CurrentPosition += 3 ;
	CurrentPosition /= 4 ;	// Rounded word offset
	Program.Words[0] = CurrentPosition ;

#ifdef QT
	p = appendSymbols( p ) ;
	RunTimeData = p ;
#endif
	i = basicExecute( 1, 0 ) ;
#ifdef QT
	p = RunTimeData ;
	if ( i )
	{
    cpystr( p, (uint8_t *)InputLine ) ;
	}
	uint8_t *r ;
	r = (uint8_t *) RunTime->Variables ;
	j = 0 ;
	for ( i = 0 ; i < 48 ; i += 1 )
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
  r = (uint8_t *) &RunTime->Arrays ;
	for ( i = 0 ; i < 48 ; i += 1 )
	{
    sprintf( Tbuf, "%02x ", *r++ ) ;
    p = cpystr( p, (uint8_t *)Tbuf ) ;
		if ( ++j > 15 )
		{
			j = 0 ;
			*p++ = '\n' ;
		}
	}
#endif
	return 2 ;
}

void basicLoadModelScripts()
{
	uint8_t *pindex ;
	uint8_t *q ;
	uint32_t i ;
	pindex = g_model.customDisplayIndex ;
	uint32_t type ;

	// First 
	type = BASIC_LOAD_TEL0 ;
	if ( g_model.customDisplay1Extra[6] == 0 )
	{		
		if ( g_model.customDisplay2Extra[6] == 0 )
		{	
//			return false ;
			return ;
		}
		else
		{
			pindex = g_model.customDisplay2Index ;
			type = BASIC_LOAD_TEL1 ;

		}
	}
	if ( ( *pindex == ' ' ) || ( *pindex == '\0' ) )
	{
		return ;
//		return false ;
	}
	// We have a filename

//  if (luaScriptsCount < MAX_SCRIPTS)
	uint8_t scriptFilename[60] ;
	q = cpystr( scriptFilename, (uint8_t *)"/SCRIPTS/TELEMETRY/" ) ;
	for ( i = 0 ; i < 6 ; i += 1 )
	{
		if ( ( *pindex == ' ' ) || ( *pindex == '\0' ) )
		{
			break ;
		}
		*q++ = *pindex++ ;
	}
	cpystr( q, (uint8_t *)".BAS" ) ;

	loadBasic( (char *) scriptFilename, type ) ;
	return ;

//extern uint32_t mainScreenDisplaying( void ) ;
//  uint8_t view = g_model.mview & 0xf;
// 	uint8_t tview = g_model.mview & 0x70 ;
//	if ( !mainScreenDisplaying() )
//	{
//		view = 0 ; // Not 4!
//	}



//  if ( (view == 4) && (tview <= 0x10) )
	
}

uint32_t basicTask( uint8_t event, uint8_t flags )
{
	uint32_t result ;
	ScriptFlags = flags ;
	if ( BasicState == BASIC_LOADING )
	{
		result = partLoadBasic() ;
		if ( result == 0 )
		{
			// Parse Error
			BasicState = BASIC_IDLE ;
			BasicLoadedType = BASIC_LOAD_NONE ;
			return 0 ;
		}
		else if ( result == 2 )
		{
			BasicState = BASIC_RUNNING ;
			return basicExecute( 1, event ) ;
		}
		return 1 ;
	}
	else if ( BasicState == BASIC_RUNNING )
	{
		if ( flags & SCRIPT_TELEMETRY )
		{
extern uint32_t mainScreenDisplaying( void ) ;
		  uint8_t view = g_model.mview & 0xf;
  		uint8_t tview = g_model.mview & 0x70 ;
			uint32_t nav = 0 ;
			if ( mainScreenDisplaying() )
			{
				if ( view == 4 )
				{
					if ( ( tview == 0 ) && ( BasicLoadedType == BASIC_LOAD_TEL0 ))
					{
						ScriptFlags |= SCRIPT_LCD_OK ;
						nav = 1 ;
					}
					else
					{
						if ( ( tview == 0x10 ) && ( BasicLoadedType == BASIC_LOAD_TEL1 ) )
						{
							ScriptFlags |= SCRIPT_LCD_OK ;
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
			if ( ( BasicLoadedType == BASIC_LOAD_TEL0 ) || ( BasicLoadedType == BASIC_LOAD_TEL1 ) )
			{
				uint32_t retvalue ;
				retvalue = basicExecute( 0, event ) ;
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
		if ( flags & SCRIPT_STANDALONE )
		{
			if ( BasicLoadedType == BASIC_LOAD_ALONE )
			{
				return basicExecute( 0, event ) ;
			}
		}
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
	value = opcode & 7 ;
	opcode &= 0xF8 ;
  if ( opcode == 0x58 )
	{
		value = (int8_t) *RunTime->ExecProgPtr++ ;
	}
	else if ( opcode == 0x40 )
	{
		val16 = *RunTime->ExecProgPtr++ ;
		val16 |= *RunTime->ExecProgPtr++ << 8 ;	// Index to dest variable
		value = val16 ;	// sign extend
  }
	else if ( opcode == 0x50 )
	{
		value = *RunTime->ExecProgPtr++ ;
		value |= *RunTime->ExecProgPtr++ << 8 ;	// Index to dest variable
		value |= *RunTime->ExecProgPtr++ << 16 ;	// Index to dest variable
		value |= *RunTime->ExecProgPtr++ << 24 ;	// Index to dest variable
	}
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
					destType |= 1 ;
				}
				val16 = getVarIndex( opcode ) ;
				dimension = *RunTime->ExecProgPtr++ ;
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
					value = RunTime->Arrays.byteArray[val16] ;
				}
				else
				{
					value = RunTime->Arrays.intArray[val16] ;
				}
			}
			else
			{
				val16 = getVarIndex( opcode ) ;
				value = RunTime->Variables[val16] ;
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

uint32_t getParamVarAddress( union t_varAddress *ptr )
{
	uint8_t opcode ;
	uint32_t result = 0 ;
	int32_t value ;
	uint16_t val16 ;
	
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
			value = expression() ;
			opcode = *RunTime->ExecProgPtr++ ;
			if ( opcode != ']' )
			{
				runError( SE_SYNTAX ) ;
				return 0 ;
			}
			val16 += value ;
			if ( val16 >= dimension )
			{
				runError( SE_DIMENSION ) ;
				return 0 ;
			}
			if ( destType & 1 )
			{
				ptr->bpointer = &RunTime->Arrays.byteArray[val16] ;
				result = 1 ;
			}
			else
			{
        ptr->ipointer = &RunTime->Arrays.intArray[val16] ;
				result = 2 ;
			}
		}
		else
		{
			val16 = getVarIndex( opcode ) ;
			ptr->ipointer = &RunTime->Variables[val16] ;
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

  //printf( "in L5: token is '%c'...\n", *token );
  op = 0 ;
	opcode = *RunTime->ExecProgPtr ;

  if( opcode == '+' || opcode == '-' )
	{
    op = opcode ;
    RunTime->ExecProgPtr += 1 ;
  }
  level6( result ) ; 
  //printf( "L5: checking for unary op\n" );
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

  //printf( "in L4...\n" );
  level5( result ) ;
  //printf( "L4: checking for */%\n" );
	
	opcode = *RunTime->ExecProgPtr++ ;

  while( opcode == '*' || opcode == '/' || opcode == '%')
	{
    //printf( "L4: token='%c'\n", op );
//    get_token(); 
    //printf( "L4: calling L5...\n" );
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
    level4( &hold ) ; 
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
  while( opcode == BITAND || opcode == BITOR )
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
		runError( SE_NO_THEN ) ;
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

	if ( opcode == GOTO || opcode == GOSUB ||  opcode == NGOTO )
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
	RunTime->CallStack[RunTime->CallIndex++] = RunTime->ExecProgPtr ;
	RunTime->ExecProgPtr = &Program.Bytes[destination] ;
}

void exec_return()
{
	if ( RunTime->CallIndex )
	{
		RunTime->ExecProgPtr = RunTime->CallStack[--RunTime->CallIndex] ;
	}
	else
	{
    runError( SE_NO_GOSUB ) ;
	}
}

// Return values
// 0 error
// 1 number
// 2 string
uint32_t get_parameter( union t_parameter *param, uint32_t type )
{
	uint8_t opcode ;

	if ( type )
	{
		// string
		opcode = *RunTime->ExecProgPtr++ ;
		if ( opcode == 0x70 )	// Quoted string
		{
			param->cpointer = RunTime->ExecProgPtr + 1 ;
      RunTime->ExecProgPtr += *RunTime->ExecProgPtr + 1 ;	// Skip the string
			if ( *RunTime->ExecProgPtr == ',' )
			{
				RunTime->ExecProgPtr += 1 ;
			}
			return 2 ;
		}
		else
		{
			runError( SE_SYNTAX ) ;
			return 0 ;
		}
	}
	else
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

void eatCloseBracket()
{
	if ( *RunTime->ExecProgPtr++ != ')' )
	{
		runError( SE_NO_BRACKET ) ;
	}
}
					 
void exec_drawnumber()
{
	int32_t x ;
	int32_t y ;
	int16_t value ;
	uint32_t result ;
	uint8_t attr ;
	union t_parameter param ;

	// get 3 (or 4) parameters
	attr = 0 ;
	result = get_parameter( &param, 0 ) ;
	if ( result == 1 )
	{
		x = param.var ;
		result = get_parameter( &param, 0 ) ;
		if ( result == 1 )
		{
			y = param.var ;
			result = get_parameter( &param, 0 ) ;
      if ( result == 1 )
			{
				value = param.var ;
				if ( *RunTime->ExecProgPtr != ')' )
				{
					result = get_parameter( &param, 0 ) ;
    		  if ( result == 1 )
					{
						attr = param.var ;
					}
				}
#ifdef QT
        RunTimeData = cpystr( RunTimeData, (uint8_t *)"DrawNumber()\n" ) ;
#else
				if ( ScriptFlags & SCRIPT_LCD_OK )
				{
  				lcd_outdezAtt(x, y, value, attr ) ;
				}
#endif
				eatCloseBracket() ;
			}
		}
	}
}

void exec_drawtimer()
{
	int32_t x ;
	int32_t y ;
	int16_t value ;
	uint32_t result ;
	uint8_t attr ;
	union t_parameter param ;

	// get 3 (or 4) parameters
	attr = 0 ;
	result = get_parameter( &param, 0 ) ;
	if ( result == 1 )
	{
		x = param.var ;
		result = get_parameter( &param, 0 ) ;
		if ( result == 1 )
		{
			y = param.var ;
			result = get_parameter( &param, 0 ) ;
      if ( result == 1 )
			{
				value = param.var ;
				if ( *RunTime->ExecProgPtr != ')' )
				{
					result = get_parameter( &param, 0 ) ;
    		  if ( result == 1 )
					{
						attr = param.var ;
					}
				}
#ifdef QT
        RunTimeData = cpystr( RunTimeData, (uint8_t *)"DrawTimer()\n" ) ;
#else
				if ( ScriptFlags & SCRIPT_LCD_OK )
				{
					putsTime( x, y, value, attr, attr ) ;
				}
#endif
				eatCloseBracket() ;
			}
		}
	}
}

void exec_drawpoint()
{
	int32_t x1 ;
	int32_t y1 ;
	uint32_t result ;
	union t_parameter param ;

	// get 3 (or 4) parameters
	result = get_parameter( &param, 0 ) ;
	if ( result == 1 )
	{
		x1 = param.var ;
		result = get_parameter( &param, 0 ) ;
		if ( result == 1 )
		{
			y1 = param.var ;
#ifdef QT
			RunTimeData = cpystr( RunTimeData, (uint8_t *)"DrawPoint()\n" ) ;
#else
			if ( ScriptFlags & SCRIPT_LCD_OK )
			{
			  lcd_plot(x1, y1 ) ;
			}
#endif
			eatCloseBracket() ;
		}
	}
	 
}

void exec_drawline()
{
	int32_t x1 ;
	int32_t y1 ;
	int32_t x2 ;
  int32_t y2 ;
	uint32_t result ;
	union t_parameter param ;

	// get 4 parameters
	result = get_parameter( &param, 0 ) ;
	if ( result == 1 )
	{
		x1 = param.var ;
		result = get_parameter( &param, 0 ) ;
		if ( result == 1 )
		{
			y1 = param.var ;
			result = get_parameter( &param, 0 ) ;
      if ( result == 1 )
			{
				x2 = param.var ;
				result = get_parameter( &param, 0 ) ;
	      if ( result == 1 )
				{
					y2 = param.var ;
#ifdef QT
          RunTimeData = cpystr( RunTimeData, (uint8_t *)"DrawLine()\n" ) ;
#else
					if ( ScriptFlags & SCRIPT_LCD_OK )
					{
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
					}
#endif
					eatCloseBracket() ;
				}
			}
		}
	}
}

void exec_drawrect()
{
	int32_t x ;
	int32_t y ;
	int32_t w ;
  int32_t h ;
	uint32_t result ;
	union t_parameter param ;

	// get 4 parameters
	result = get_parameter( &param, 0 ) ;
	if ( result == 1 )
	{
		x = param.var ;
		result = get_parameter( &param, 0 ) ;
		if ( result == 1 )
		{
			y = param.var ;
			result = get_parameter( &param, 0 ) ;
      if ( result == 1 )
			{
				w = param.var ;
				result = get_parameter( &param, 0 ) ;
	      if ( result == 1 )
				{
					h = param.var ;
#ifdef QT
          RunTimeData = cpystr( RunTimeData, (uint8_t *)"DrawRect()\n" ) ;
#else
					if ( ScriptFlags & SCRIPT_LCD_OK )
					{
					  lcd_rect(x, y, w, h);
					}
#endif
					eatCloseBracket() ;
				}
			}
		}
	}
}

void exec_drawtext()
{
	int32_t x ;
	int32_t y ;
	uint8_t *p ;
	uint32_t result ;
	uint8_t attr ;
	union t_parameter param ;
	uint32_t length = 0 ;

	// get 3 (or 4) parameters
	attr = 0 ;
	result = get_parameter( &param, 0 ) ;
	if ( result == 1 )
	{
		x = param.var ;
		result = get_parameter( &param, 0 ) ;
		if ( result == 1 )
		{
			y = param.var ;
			result = get_parameter( &param, 1 ) ;
      if ( result == 2 )
			{
				p = param.cpointer ;
				if ( *RunTime->ExecProgPtr != ')' )
				{
					result = get_parameter( &param, 0 ) ;
    		  if ( result == 1 )
					{
						attr = param.var ;
						if ( *RunTime->ExecProgPtr != ')' )	// length param
						{
							result = get_parameter( &param, 0 ) ;
    				  if ( result == 1 )
							{
								length = param.var ;
							}
						}
					}
				}
#ifdef QT
        RunTimeData = cpystr( RunTimeData, (uint8_t *)"DrawText()\n" ) ;
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
				eatCloseBracket() ;
			}
		}
	}
}

void exec_drawclear()
{
	// call clear lcd
#ifdef QT
  RunTimeData = cpystr( RunTimeData, (uint8_t *)"DrawClear()\n" ) ;
#else
	if ( ScriptFlags & SCRIPT_LCD_OK )
	{
		lcd_clear() ;
	}
#endif
	eatCloseBracket() ;
}


extern uint16_t getUnitsVoice( uint16_t unit ) ;

void exec_playnumber()
{
	int32_t number ;
	int32_t unit ;
	uint8_t att ;
	uint32_t result ;
	union t_parameter param ;

	// get 3 (or 4) parameters
	result = get_parameter( &param, 0 ) ;
	if ( result == 1 )
	{
		number = param.var ;
		result = get_parameter( &param, 0 ) ;
		if ( result == 1 )
		{
			unit = param.var ;
			result = get_parameter( &param, 0 ) ;
      if ( result == 1 )
			{
				att = param.var ;
#ifdef QT
        RunTimeData = cpystr( RunTimeData, (uint8_t *)"PlayNumber()\n" ) ;
#else
				if ( unit > 0 && unit < 10 )
				{
					unit = getUnitsVoice( unit ) ;
				}
				else
				{
					unit = 0 ;
				}
					voice_numeric( number, att, unit ) ;
#endif
				eatCloseBracket() ;
			}
		}
	}
}

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
		result = get_parameter( &param, 1 ) ;
		if ( result == 2 )
		{
			p = param.cpointer ;
#ifdef QT			
			number = 0 ;
#else
			number = basicFindValueIndexByName( (char *)p ) ;
#endif
			result = get_parameter( &param, 0 ) ;
			if ( result == 1 )
			{
				value = param.var ;
				number -= 44 ;
				if ( number < NUM_TELEM_ITEMS )
				{
					result = TelemValid[number] ;
					number = TelemIndex[number] ;
					retval = value ;
					if ( ( result == 1 ) || ( result == 2 ) )
					{
						storeTelemetryData( number, value ) ;
					}
				}
			}
		}
	}
	return retval ;
}


int32_t exec_getvalue()
{
	int32_t number ;
	uint32_t result ;
	union t_parameter param ;
	uint8_t *p ;

	number = 0 ;
	if ( *RunTime->ExecProgPtr == 0x70 )
	{
		result = get_parameter( &param, 1 ) ;
		if ( result == 2 )
		{
			p = param.cpointer ;
#ifdef QT			
			number = 0 ;
#else
			number = basicFindValueIndexByName( (char *)p ) ;
#endif
		}
	} 
	else
	{
		result = get_parameter( &param, 0 ) ;
		if ( result == 1 )
		{
			number = param.var ;
		}
	}
	if ( result )
	{
#ifdef QT			
		RunTimeData = cpystr( RunTimeData, (uint8_t *)"GetValue()\n" ) ;
#else
		if ( number >= 0 )
		{
		  number = getValue(number) ; // ignored for GPS, DATETIME, and CELLS
		}
#endif
	}
	eatCloseBracket() ;
	return number ;
}

static int32_t exec_idletime()
{
	eatCloseBracket() ;
#ifdef QT			
	return 50 ;
#else
extern uint32_t IdlePercent ;
	return IdlePercent ;
#endif
}

static int32_t exec_gettime()
{
	eatCloseBracket() ;
#ifdef QT			
  return 100 ;
#else
extern volatile uint32_t get_ltmr10ms() ;
	return get_ltmr10ms() ;
#endif
}

static int32_t exec_getlastpos()
{
	eatCloseBracket() ;
#ifdef QT			
  return 20 ;
#else
	return Lcd_lastPos ;
#endif
}

static int32_t exec_sysflags()
{
	eatCloseBracket() ;
#ifdef QT			
  return 128 ;
#else
	return ScriptFlags ;
#endif
}


int32_t getSingleNumericParameter()
{
	uint32_t result ;
	union t_parameter param ;

	result = get_parameter( &param, 0 ) ;
	if ( result != 1 )
	{
		runError( SE_SYNTAX ) ;
	}
	eatCloseBracket() ;
	return param.var ;
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


const uint8_t SportIds[28] = {0x00, 0xA1, 0x22, 0x83, 0xE4, 0x45, 0xC6, 0x67,
				                      0x48, 0xE9, 0x6A, 0xCB, 0xAC, 0x0D, 0x8E, 0x2F,
															0xD0, 0x71, 0xF2, 0x53, 0x34, 0x95, 0x16, 0xB7,
															0x98, 0x39, 0xBA, 0x1B/*, 0x7C, 0xDD, 0x5E, 0xFF*/ } ;

int32_t exec_sportsend()
{
	static uint8_t sportPacket[8] ;
	uint32_t result = 0 ;
	union t_parameter param ;
	int32_t x ;

	result = get_parameter( &param, 0 ) ;
	if ( result == 1 )
	{
		x = param.var ;
		if ( x == 0 )
		{
#ifdef QT
      RunTimeData = cpystr( RunTimeData, (uint8_t *)"SportSend()\n" ) ;
      result = 0 ;
#else
extern uint32_t sportPacketSend( uint8_t *pdata, uint8_t index ) ;
			result = sportPacketSend( 0, 0 ) ;
#endif
		}
		else
		{
			sportPacket[7] = SportIds[x] ;
			result = get_parameter( &param, 0 ) ;
			if ( result == 1 )
			{
		    sportPacket[0] = param.var ;
				result = get_parameter( &param, 0 ) ;
	      if ( result == 1 )
				{
					x = param.var ;
					sportPacket[1] = x ;
					sportPacket[2] = x >> 8 ;
					result = get_parameter( &param, 0 ) ;
		      if ( result == 1 )
					{
						result = param.var ;
						sportPacket[3] = result ;
						sportPacket[4] = result >> 8 ;
						sportPacket[5] = result >> 16 ;
						sportPacket[6] = result >> 24 ;
#ifdef QT
            RunTimeData = cpystr( RunTimeData, (uint8_t *)"SportSend()\n" ) ;
            result = 0 ;
#else
            result = sportPacketSend( sportPacket, sportPacket[7] ) ;
#endif	
						eatCloseBracket() ;
          }
        }
      }
    }
  }
	return result ;
}

int32_t exec_sportreceive()
{
	// This needs the address of 4 variables for the data
	union t_varAddress param[4] ;
	uint8_t type[4] ;
	uint32_t result ;
	result = getParamVarAddress( &param[0] ) ;
	if ( result )
	{
		type[0] = result ;
		result = getParamVarAddress( &param[1] ) ;
		if ( result )
		{
			type[1] = result ;
			result = getParamVarAddress( &param[2] ) ;
			if ( result )
			{
				type[2] = result ;
				result = getParamVarAddress( &param[3] ) ;
				if ( result )
				{
					type[3] = result ;
#ifdef QT
        	RunTimeData = cpystr( RunTimeData, (uint8_t *)"SportReceive()\n" ) ;
          result = 1 ;
#else
					if ( fifo128Space( &Lua_fifo ) <= ( 128 - 8 ) )
					{
						uint32_t value ;
						value = get_fifo128( &Lua_fifo ) & 0x1F ;
						if ( type[0] == 1 )
						{
							*param[0].bpointer = value ;
						}
						else
						{
							*param[0].ipointer = value ;
						}
						value = get_fifo128( &Lua_fifo ) & 0x1F ;
						if ( type[1] == 1 )
						{
							*param[1].bpointer = value ;
						}
						else
						{
							*param[1].ipointer = value ;
						}
						value = get_fifo128( &Lua_fifo ) ;
						value |= get_fifo128( &Lua_fifo ) << 8 ;
						if ( type[2] == 1 )
						{
							*param[2].bpointer = value ;
						}
						else
						{
							*param[2].ipointer = value ;
						}
						value = get_fifo128( &Lua_fifo ) ;
						value |= get_fifo128( &Lua_fifo ) << 8 ;
						value |= get_fifo128( &Lua_fifo ) << 16 ;
						value |= get_fifo128( &Lua_fifo ) << 24 ;
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
					eatCloseBracket() ;
#endif	
				}
			}
		}
	}
	return result ;
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
			result = exec_getvalue() ;
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

    case NOT :
			result = exec_not() ;
		break ;
    
		case ABS :
			result = exec_abs() ;
		break ;

		case GETLASTPOS :
			result = exec_getlastpos() ;
		break ;
		
		case SYSFLAGS :
			result = exec_sysflags() ;
		break ;

		case SETTELITEM :
			result = exec_settelitem() ;
		break ;
		
		default :
			runError( SE_NO_FUNCTION ) ;
		break ;
	}
	return result ;
}


//uint8_t BasicDump[32] ;


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
  RunTime->RunLineNumber = (opcode & 0x7F) | (*RunTime->ExecProgPtr++ << 8 ) ;
//	sprintf( InputLine, "%d\n", RunTime->RunLineNumber ) ;
//  RunTimeData = cpystr( RunTimeData, (uint8_t *) InputLine ) ;
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
						oldValue = RunTime->Arrays.byteArray[val16] ;
					}
					else
					{
						oldValue = RunTime->Arrays.intArray[val16] ;
					}
				}
				else
				{
					oldValue = RunTime->Variables[val16] ;
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
				}
				value = oldValue ;
			}
			if ( destType )
			{
				if ( destType & 1 )
				{
					RunTime->Arrays.byteArray[val16] = value ;
				}
				else
				{
					RunTime->Arrays.intArray[val16] = value ;
				}
			}
			else
			{
				RunTime->Variables[val16] = value ;
			}
		}
		else
		{
			switch ( opcode )
			{
				case IF :
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


uint32_t basicExecute( uint32_t begin, uint8_t event )
{
	uint32_t execLinesProcessed ;
	uint32_t finished ;

	if ( begin )
	{
		uint32_t i ;
		RunTime = (struct t_basicRunTime *) &Program.Words[Program.Words[0]] ;
    RunTime->RunError = 0 ;
		RunTime->ExecProgPtr = &Program.Bytes[4] ;
		RunTime->CallIndex = 0 ;
    RunTime->ParameterIndex = 0 ;
		for ( i = 0 ; i < MAX_VARIABLES ; i += 1 )
		{
			RunTime->Variables[i] = 0 ;
		}
		return 1 ;
	}
	RunTime->Variables[0] = event ;
	
	RunTime->ExecProgPtr = &Program.Bytes[4] ;
	
	execLinesProcessed = 0 ;
	while ( execLinesProcessed < 300 )
	{
		finished = execOneLine() ;
		if ( finished == 1 )	// Found Finish
		{
			finished = 3 ;
			BasicState = BASIC_IDLE ;
			BasicLoadedType = BASIC_LOAD_NONE ;
#ifndef QT
      killEvents( event ) ;
#endif
      break ;
		}
		if ( finished == 2 )
		{
			RunTime->ExecProgPtr = &Program.Bytes[4] ;
			break ;
		}
		finished = 1 ;
		if ( RunTime->RunError )
		{
			BasicState = BASIC_IDLE ;
			BasicLoadedType = BASIC_LOAD_NONE ;
			break ;
		}
		execLinesProcessed += 1 ;
	}
	if ( RunTime->RunError )
	{
#ifdef QT			
    sprintf( InputLine, "Error %d at line %d", RunTime->RunError, RunTime->RunErrorLine ) ;
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
#endif


//// Read special character (with translations)
//unsigned int read_special( char delim )
//{
//	char chr ;

//	if ( ( chr = *Input_pos++ ) == delim )
//	{
//		String_continue = 0 ;
//		return 0 ;
//	}
//	if ( chr == '\\' )
//	{
//		switch( chr = *Input_pos++ )
//		{
//			case '\0' :
//				// Backslash at end of line
//				String_continue = 1 ;
//				Input_pos -= 1 ;
//			break ;

//			case 'n':
//				/* newline */
//				chr = 0x0a ;
//			break ;

//			case 'r':
//				/* return */
//				chr = 0x0d ;
//			break ;

//			case 't':
//				/* tab */
//				chr = 0x09 ;
//			break ;

//			case 'f' :
//				/* formfeed */
//				chr = 0x0c ;
//			break ;

//			case 'b':
//				/* backspace */
//				chr = 0x08 ;
//			break ;

//			case 'x' :
//				/* hex value */
//				chr = (char) get_number( 16, 2 ) ;
//			break ;

//			default:
//				if( isnum( chr ) )
//				{	/* octal value */
//					Input_pos -= 1 ;
//					chr = (char) get_number( 8, 3 ) ;
//				}
//			break ;
//		}
//	}
//	return chr ;
//}

//// Get a number in a number base for a maximum # of digits
//// (digits = 0 means no limit)
//unsigned int get_number( int base, int digits )
//{
//	unsigned value ;
//	char c ;

//	value = 0 ;
//	do
//	{
//		if ( isnum( c = *Input_pos ) )		/* convert numeric digits */
//		{
//			c -= '0' ;
//		}
//		else if(c >= 'a')				/* convert lower case alphabetics */
//		{
//			c -= ('a' - 10) ;
//		}
//		else if(c >= 'A')				/* convert upper case alphabetics */
//		{
//			c -= ('A' - 10);
//		}
//		else							/* not a valid "digit" */
//		{
//			break ;
//		}
//		if( c >= (char) base )					/* outside of base */
//		{
//			break ;
//		}
//		value = ( value * base ) + c ;		/* include in total */
//		Input_pos += 1 ;
//	} while( --digits )	;					/* enforce maximum digits */
//	if ( ( *Input_pos == 'L' ) || ( *Input_pos == 'l' ) )
//	{
//		Input_pos += 1 ;
//	}

//	return value ;
//}


//			if ( isnum( *ptr ) )
//			{
//				// A number
//				if ( *ptr == '0' )
//				{
//					base = 8 ;
//					if ( ( *(ptr+1) == 'x' ) || ( *(ptr+1) == 'X' ) )
//					{
//						base = 16 ;
//						ptr += 1 ;
//					}
//					else if ( ( *(ptr+1) == 'b' ) || ( *(ptr+1) == 'B' ) )
//					{
//						base = 2 ;
//						ptr += 1 ;
//					}
//					ptr += 1 ;
//					Input_pos = ptr ;
//					value = get_number( base, 0 ) ;
////					printf("----gotnumber %d", value ) ;
//				}
//				else
//				{
//					Input_pos = ptr ;
//					value = get_number( 10, 0 ) ;
//				}

