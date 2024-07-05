/*
 * Author - Mike Blandford
 *
 * Based on er9x by Erez Raviv <erezraviv@gmail.com>
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

#ifndef menus_h
#define menus_h

#define prog_uint8_t const uint8_t
#define pgm_read_adr(x) *(x)

#define min(x,y) ((x<y)?x:y)


extern uint8_t CurrentPhase ;
extern uint8_t s_noHi ;

// Menus related stuff ...
struct MState2
{
  uint8_t m_posVert;
//  uint8_t m_posHorz;
  void init(){m_posVert=0;};
  uint8_t check(uint8_t event, uint8_t curr, MenuFuncP *menuTab, uint8_t menuTabSize, prog_uint8_t *subTab, uint8_t subTabMax, uint8_t maxrow);
	uint8_t check_columns( uint8_t event, uint8_t maxrow) ;
};

uint8_t evalOffset(int8_t sub) ;

typedef const void (*MenuFunc)(uint8_t event) ;

#ifdef TOUCH
#define TITLEP(pstr) lcd_putsAtt(4*FW,0,pstr,INVERS)
#else
#define TITLEP(pstr) lcd_putsAtt(0*FW,0,pstr,INVERS)
#endif
#define TITLE(str)   TITLEP(str)


#define MENU(title, tab, menu, lines_count, ...) \
TITLE(title); \
static MState2 mstate2; \
static const uint8_t mstate_tab[] = __VA_ARGS__; \
event = mstate2.check(event,menu,tab,DIM(tab),mstate_tab,DIM(mstate_tab)-1,lines_count-1)

#define YN_NONE	0
#define YN_YES	1
#define YN_NO		2

uint8_t yesNoMenuExit( uint8_t event, const prog_char * s ) ;

#define POPUP_NONE			0
#define POPUP_SELECT		1
#define POPUP_EXIT			2

#define EDIT_DR_SWITCH_EDIT		0x01
#define EDIT_DR_SWITCH_MOMENT	0x02
#define EDIT_DR_SWITCH_FMODE	0x04

#define VOICE_FILE_TYPE_NAME	0
#define VOICE_FILE_TYPE_USER	1
#define VOICE_FILE_TYPE_MUSIC	2
#define VOICE_FILE_TYPE_SYSTEM	3

// Styles
#define TELEM_LABEL				0x01
#define TELEM_UNIT    		0x02
#define TELEM_UNIT_LEFT		0x04
#define TELEM_VALUE_RIGHT	0x08
#define TELEM_NOTIME_UNIT	0x10
#define TELEM_ARDUX_NAME	0x20
#define TELEM_CONSTANT		0x80

#if defined(PCBX12D) || defined(PCBX10)
#define TELEM_HIRES				0x40
#endif

#if defined(PCBSKY) || defined(PCB9XT)
#define NUM_ANA_ITEMS		3
#endif
#ifdef PCBX9D
#define NUM_ANA_ITEMS		2
#endif
#if defined(PCBX12D) || defined(PCBX10)
#define NUM_ANA_ITEMS		2
#endif

#ifdef PCBLEM1
#define NUM_ANA_ITEMS		2
#endif

#ifdef TOUCH

#define TTOP			8
#define THTOP			18
#define TFH				12
#define TRIGHT	 180
#define TMID		 110
#define TLINES		9
#define TSCALE		2
#define TVOFF			3
#define THOFF			4
#define TRMARGIN	3

#define TSCROLLLEFT		380
#define TSCROLLTOP		16
#define TSCROLLWIDTH	42
#define TSCROLLBOTTOM	(240-16)

#endif

struct t_popupData
{
	uint8_t PopupActive ;
	uint8_t	PopupIdx ;
	uint8_t	PopupSel ;
	uint8_t PopupTimer ;
#ifdef TOUCH
	uint16_t PopupHpos ;
	uint16_t PopupVpos ;
	uint16_t PopupWidth ;
#endif
} ;



#ifdef TOUCH

#define TEVT_DOWN 			1
#define TEVT_UP				 	2
#define TEVT_SLIDE 			4
#define TEVT_SLIDE_END	8

struct t_touchControl
{
	uint16_t event ;
	uint16_t count ;
	uint16_t x ;
	uint16_t y ;
	uint16_t startx ;
	uint16_t starty ;
	int16_t deltax ;
	int16_t deltay ;
	uint16_t repeat ;
	uint16_t xSelected ;
	uint8_t itemSelected ;
} ;
 
extern t_touchControl TouchControl ;

#define TICON_SIZE	50

uint32_t handleSelectIcon( void ) ;
uint32_t handlePlayIcon( void ) ;
uint32_t handleEditIcon() ;
uint32_t scrollBar( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t positions, uint32_t vpos ) ;

#endif

extern struct t_popupData PopupData ;

extern int16_t calibratedStick[] ;
																 
#if defined(PCBX12D) || defined(PCBX10)
extern void doMainScreenGrphics( uint16_t colour = 0 ) ;
#else
extern void doMainScreenGrphics( void ) ;
#endif
extern void menuProcStatistic2(uint8_t event) ;
extern void menuProcStatistic(uint8_t event) ;
extern void menuProcBattery(uint8_t event) ;
extern void menuProc0(uint8_t event) ;
extern void actionMainPopup( uint8_t event ) ;
extern void menuProcModelSelect(uint8_t event) ;
extern void menuProcGlobals(uint8_t event) ;
extern int16_t expo(int16_t x, int16_t k) ;
extern int16_t calcExpo( uint8_t channel, int16_t value ) ;
extern void timer(int16_t throttle_val) ;
extern void startupCalibration( void ) ;

extern void menuUp1(uint8_t event) ;
extern void menuUpdate(uint8_t event) ;
//extern void inactivityCheck( void ) ;
extern int16_t scaleAnalog( int16_t v, uint8_t channel ) ;
#if defined(PCBX12D) || defined(PCBX10)
void displayStatusLine( uint32_t scriptPercent ) ;
#endif

void validateProtocolOptions( uint32_t module ) ;

extern void parseMultiData( void ) ;

#if defined(PCBX12D) || defined(PCBX10)
void checkTheme( themeData *t ) ;
#endif
#ifdef TOUCH
uint16_t dimBackColour() ;
void drawItem( char *s, uint16_t y, uint16_t colour ) ;
void drawNumber( uint16_t x, uint16_t y, int32_t val, uint16_t mode) ; //, uint16_t colour ) ;
void drawChar( uint16_t x, uint16_t y, uint8_t c, uint16_t mode, uint16_t colour ) ;
uint32_t touchOnOffItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition, uint16_t colour ) ;
uint32_t touchOffOnItem( uint8_t value, uint8_t y, const prog_char *s, uint8_t condition, uint16_t colour ) ;
void putsChnColour( uint8_t x, uint8_t y, uint8_t idx1, uint8_t att ) ;
void saveEditColours( uint32_t attr, uint16_t colour ) ;
void restoreEditColours() ;
int32_t checkTouchSelect( uint32_t rows, uint32_t pgOfs, uint32_t flag = 0 ) ;
uint16_t handleTouchSelect( uint32_t rows, uint32_t pgOfs, uint8_t sub, uint32_t flag = 0 ) ;

#endif

const char *get_curve_string() ;

extern uint8_t unmapPots( uint8_t value ) ;

extern int16_t calc_scaler( uint8_t index, uint16_t *unit, uint8_t *num_decimals) ;

extern uint8_t CalcScaleNest ;

#define MAXTRACE 120
extern uint8_t s_traceBuf[] ;
extern uint8_t s_traceWr;
extern uint16_t s_traceCnt;

#define BaudString FWx13"\006""\006  AUTO  9600 19200 38400 57600115200100000"

#define MODELTIME	-24
#define RUNTIME		-23
#define FMODE			-22
#define TMOK			-21

#define V_RTC			-20

#define V_SC1			-19
#define V_SC2			-18
#define V_SC3			-17
#define V_SC4			-16
#define V_SC5			-15
#define V_SC6			-14
#define V_SC7			-13
#define V_SC8			-12

#define FR_WATT		-11

#define V_GVAR1		-10
#define V_GVAR2		-9
#define V_GVAR3		-8
#define V_GVAR4		-7
#define V_GVAR5		-6
#define V_GVAR6		-5
#define V_GVAR7		-4

#define BATTERY		-3
#define TIMER1		-2
#define TIMER2		-1

#define TEL_ITEM_A1			0
#define TEL_ITEM_A2			1
#define TEL_ITEM_RSSI		2
#define TEL_ITEM_TSSI		3
#define TEL_ITEM_TIM1		4
#define TEL_ITEM_TIM2		5
#define TEL_ITEM_BALT		6
#define TEL_ITEM_GALT		7
#define TEL_ITEM_GSPD		8
#define TEL_ITEM_T1			9
#define TEL_ITEM_T2			10
#define TEL_ITEM_RPM		11
#define TEL_ITEM_FUEL		12
#define TEL_ITEM_MAH1		13
#define TEL_ITEM_MAH2		14
#define TEL_ITEM_CVLT		15
#define TEL_ITEM_BATT		16
#define TEL_ITEM_AMPS		17
#define TEL_ITEM_MAHC		18
#define TEL_ITEM_CTOT		19
#define TEL_ITEM_FASV		20
#define TEL_ITEM_ACCX		21
#define TEL_ITEM_ACCY		22
#define TEL_ITEM_ACCZ		23
#define TEL_ITEM_VSPD		24
#define TEL_ITEM_GVAR1	25
#define TEL_ITEM_GVAR2	26
#define TEL_ITEM_GVAR3	27
#define TEL_ITEM_GVAR4	28
#define TEL_ITEM_GVAR5	29
#define TEL_ITEM_GVAR6	30
#define TEL_ITEM_GVAR7	31
#define TEL_ITEM_FWATT	32
#define TEL_ITEM_RXV		33
#define TEL_ITEM_GHDG		34
#define TEL_ITEM_A3			35
#define TEL_ITEM_A4			36
#define TEL_ITEM_SC1		37
#define TEL_ITEM_SC2		38
#define TEL_ITEM_SC3		39
#define TEL_ITEM_SC4		40
#define TEL_ITEM_SC5		41
#define TEL_ITEM_SC6		42
#define TEL_ITEM_SC7		43
#define TEL_ITEM_SC8		44
#define TEL_ITEM_RTC		45
#define TEL_ITEM_TMOK		46
#define TEL_ITEM_ASPD		47
#define TEL_ITEM_CELL1	48
#define TEL_ITEM_CELL2	49
#define TEL_ITEM_CELL3	50
#define TEL_ITEM_CELL4	51
#define TEL_ITEM_CELL5	52
#define TEL_ITEM_CELL6	53
#define TEL_ITEM_RBV1		54
#define TEL_ITEM_RBA1		55
#define TEL_ITEM_RBV2		56
#define TEL_ITEM_RBA2		57
#define TEL_ITEM_RBM1		58
#define TEL_ITEM_RBM2		59
#define TEL_ITEM_RBSERVO	60
#define TEL_ITEM_RBSTATE	61
#define TEL_ITEM_CELL7	  62
#define TEL_ITEM_CELL8	  63
#define TEL_ITEM_CELL9	  64
#define TEL_ITEM_CELL10	  65
#define TEL_ITEM_CELL11	  66
#define TEL_ITEM_CELL12	  67
#define TEL_ITEM_CUST1	  68
#define TEL_ITEM_CUST2	  69
#define TEL_ITEM_CUST3	  70
#define TEL_ITEM_CUST4	  71
#define TEL_ITEM_CUST5	  72
#define TEL_ITEM_CUST6	  73
#define TEL_ITEM_CUST7	  74
#define TEL_ITEM_CUST8	  75

// TextHelp types
#define TEXT_TYPE_TELE_SOURCE		0
#define TEXT_TYPE_SW_SOURCE			1
#define TEXT_TYPE_MIX_SOURCE		2
#define TEXT_TYPE_SW_FUNCTION		3
#define TEXT_TYPE_CUSTOM				4

// units
#define U_FEET			0
#define U_VOLTS			1
#define	U_DEGREES_C 2
#define	U_DEGREES_F 3
#define	U_CAPACITY	4
#define U_AMPS			5
#define	U_METRES		6
#define	U_WATTS			7
#define	U_PERCENT		8

extern uint8_t TextIndex ;
extern uint8_t TextType ;
extern uint8_t TextResult ;

void menuTextHelp(uint8_t event) ;
void sortTelemText() ;

struct t_textControl
{
	void (*TextFunction)( uint8_t x, uint8_t y, uint8_t index, uint8_t att ) ;
	uint8_t TextMax ;
	uint8_t *TextMap ;
	uint8_t TextWidth ;
	uint8_t TextOption ;
} ;

extern struct t_textControl TextControl ;

#endif
