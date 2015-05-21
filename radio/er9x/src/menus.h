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

#ifndef menus_h
#define menus_h

extern audioQueue  audio;

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

extern uint8_t g_posHorz ;

// Menus related stuff ...
struct MState2
{
  uint8_t m_posVert;
//  uint8_t m_posHorz;
  void init(){m_posVert=0;};
//  void check(uint8_t event, const prog_uint8_t *subTab, uint8_t subTabMax, uint8_t maxrow);
	void check_columns( uint8_t event, uint8_t maxrow) ;

//  void check_simple(uint8_t event, uint8_t curr, const MenuFuncP *menuTab, uint8_t menuTabSize, uint8_t maxrow);
//  void check_submenu_simple( uint8_t event, const prog_uint8_t *subTab, uint8_t subTabMax, uint8_t maxrow);
};

uint8_t evalOffset(int8_t sub) ;

typedef PROGMEM void (*MenuFuncP_PROGMEM)(uint8_t event);

//#define TITLEP(pstr) lcd_putsAtt(0,0,pstr,INVERS)
#define TITLE(str)   TITLEP(PSTR(str))

#define MENU(title, lines_count, ...) \
TITLE(title); \
static MState2 mstate2; \
const static prog_uint8_t APM mstate_tab[] = __VA_ARGS__; \
mstate2.check(event,mstate_tab,DIM(mstate_tab)-1,lines_count-1)

#define VARMENU(title, lines_count, cols ) \
TITLE(title); \
static MState2 mstate2; \
mstate2.check(event,cols,0,lines_count-1)

#define SIMPLE_MENU(title, lines_count) \
TITLE(title); \
static MState2 mstate2; \
mstate2.check_simple(event,lines_count-1)

#define SUBMENU(title, lines_count, ...) \
TITLE(title); \
static MState2 mstate2; \
const static prog_uint8_t APM mstate_tab[] = __VA_ARGS__; \
mstate2.check(event,mstate_tab,DIM(mstate_tab)-1,lines_count-1)

#define SUBMENU_NOTITLE(lines_count, ...) \
static MState2 mstate2; \
static const prog_uint8_t APM mstate_tab[] = __VA_ARGS__; \
mstate2.check(event,mstate_tab,DIM(mstate_tab)-1,lines_count-1)

#define SIMPLE_SUBMENU(title, lines_count) \
TITLE(title); \
SIMPLE_SUBMENU_NOTITLE(lines_count-1, {0})

//#if defined(GVARS)
//void displayGVar(uint8_t x, uint8_t y, int8_t value);
//#else
//#define displayGVar(x, y, v) lcd_outdez8(x, y, v)
//#endif

struct t_popupData
{
	uint8_t PopupActive ;
	uint8_t	PopupIdx ;
	uint8_t	PopupSel ;
	uint8_t PopupTimer ;
} ;

extern struct t_popupData PopupData ;

extern uint8_t CalcScaleNest ;
extern int16_t calc_scaler( uint8_t index, uint8_t *unit, uint8_t *num_decimals) ;
extern int16_t scaleAnalog( int16_t v, uint8_t channel ) ;

//const prog_char *get_curve_string() ;
extern const prog_char APM Curve_Str[] ;

#define TMOK			-16

#define V_SC1			-15
#define V_SC2			-14
#define V_SC3			-13
#define V_SC4			-12

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


// units
#define U_FEET			0
#define U_VOLTS			1
#define	U_DEGREES_C 2
#define	U_DEGREES_F 3
#define	U_CAPACITY	4
#define U_AMPS			5
#define	U_METRES		6
#define	U_WATTS			7


#endif
