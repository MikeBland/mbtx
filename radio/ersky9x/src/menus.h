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

//extern audioQueue  audio;

/*#define IS_THROTTLE(x)  (((2-(g_eeGeneral.stickMode&1)) == x) && (x<4))
#define GET_DR_STATE(x) (!getSwitch(g_model.expoData[x].drSw1,0) ?   \
                          DR_HIGH :                                  \
                          !getSwitch(g_model.expoData[x].drSw2,0)?   \
                          DR_MID : DR_LOW);

#define DO_SQUARE(xx,yy,ww)         \
    lcd_vline(xx-ww/2,yy-ww/2,ww);  \
    lcd_hline(xx-ww/2,yy+ww/2,ww);  \
    lcd_vline(xx+ww/2,yy-ww/2,ww);  \
    lcd_hline(xx-ww/2,yy-ww/2,ww);

#define DO_CROSS(xx,yy,ww)          \
    lcd_vline(xx,yy-ww/2,ww);  \
    lcd_hline(xx-ww/2,yy,ww);  \

#define V_BAR(xx,yy,ll)       \
    lcd_vline(xx-1,yy-ll,ll); \
    lcd_vline(xx  ,yy-ll,ll); \
    lcd_vline(xx+1,yy-ll,ll); \

#define NO_HI_LEN 25

#define WCHART 32
#define X0     (128-WCHART-2)
#define Y0     32
#define WCHARTl 32l
#define X0l     (128l-WCHARTl-2)
#define Y0l     32l
#define RESX    1024
#define RESXu   1024u
#define RESXul  1024ul
#define RESXl   1024l
#define RESKul  100ul
#define RESX_PLUS_TRIM (RESX+128)

extern bool warble;*/
//extern int16_t p1valdiff;
//extern uint8_t scroll_disabled;

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
//  void check_simple(uint8_t event, uint8_t curr, MenuFuncP *menuTab, uint8_t menuTabSize, uint8_t maxrow);
//  void check_submenu_simple(uint8_t event, uint8_t maxrow);
};

uint8_t evalOffset(int8_t sub) ;
//#if defined(PCBX12D) || defined(PCBX10)
//uint8_t evalOffsetLarge(int8_t sub) ;
//#endif

//typedef PROGMEM void (*MenuFuncP_PROGMEM)(uint8_t event);
typedef const void (*MenuFunc)(uint8_t event) ;

#define TITLEP(pstr) lcd_putsAtt(0,0,pstr,INVERS)
//#define TITLE(str)   TITLEP(PSTR(str))
#define TITLE(str)   TITLEP(str)


//#define SIMPLE_MENU(title, tab, menu, lines_count)
//TITLE(title);
//static MState2 mstate2;
//mstate2.check_simple(event,menu,tab,DIM(tab),lines_count-1)

#define MENU(title, tab, menu, lines_count, ...) \
TITLE(title); \
static MState2 mstate2; \
static const uint8_t mstate_tab[] = __VA_ARGS__; \
event = mstate2.check(event,menu,tab,DIM(tab),mstate_tab,DIM(mstate_tab)-1,lines_count-1)

//#define VARMENU(title, tab, menu, lines_count, cols )
//TITLE(title);
//static MState2 mstate2;
//event = mstate2.check(event,menu,tab,DIM(tab),cols,0,lines_count-1)


//#define SUBMENU(title, lines_count, ...)
//TITLE(title);
//static MState2 mstate2;
//static const uint8_t mstate_tab[] = __VA_ARGS__;
//mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1,lines_count-1)

//#define SUBMENU_NOTITLE(lines_count, ...)
//static MState2 mstate2;
//static const uint8_t mstate_tab[] = __VA_ARGS__;
//mstate2.check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1,lines_count-1)


//#define SIMPLE_SUBMENU_NOTITLE(lines_count)
//static MState2 mstate2;
//mstate2.check_submenu_simple(event,lines_count-1)

//#define SIMPLE_SUBMENU(title, lines_count)
//TITLE(title);
//SIMPLE_SUBMENU_NOTITLE(lines_count-1)

/*
//#define SIMPLE_SUBMENU(title, lines_count) \
//TITLE(title); \
//SIMPLE_SUBMENU_NOTITLE(lines_count-1)
	*/

#define YN_NONE	0
#define YN_YES	1
#define YN_NO		2

uint8_t yesNoMenuExit( uint8_t event, const prog_char * s ) ;

struct t_popupData
{
	uint8_t PopupActive ;
	uint8_t	PopupIdx ;
	uint8_t	PopupSel ;
	uint8_t PopupTimer ;
} ;

extern struct t_popupData PopupData ;

extern int16_t calibratedStick[] ;
//extern int16_t g_chans512[NUM_SKYCHNOUT];

extern void doMainScreenGrphics( void ) ;
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

extern void parseMultiData( void ) ;

const char *get_curve_string() ;

extern uint8_t unmapPots( uint8_t value ) ;

extern int16_t calc_scaler( uint8_t index, uint16_t *unit, uint8_t *num_decimals) ;

extern uint8_t CalcScaleNest ;

#define MAXTRACE 120
extern uint8_t s_traceBuf[] ;
extern uint8_t s_traceWr;
extern uint16_t s_traceCnt;

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

#endif
