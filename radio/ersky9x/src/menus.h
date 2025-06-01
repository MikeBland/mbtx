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
  uint8_t check(uint8_t event, uint8_t curr, const MenuFuncP *menuTab, uint8_t menuTabSize, prog_uint8_t *subTab, uint8_t subTabMax, uint8_t maxrow);
	uint8_t check_columns( uint8_t event, uint8_t maxrow) ;
};

uint32_t evalOffset(int8_t sub) ;
uint8_t evalHresOffset(int8_t sub) ;
extern uint8_t s_pgOfs ;

typedef const void (*MenuFunc)(uint8_t event) ;

#ifdef COLOUR_DISPLAY
#define TITLEP(pstr) PUTS_ATT(4*FW,0,pstr,INVERS|BOLD)
#else
 #if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
#define TITLEP(pstr) PUTS_ATT(0*FW,0,pstr,INVERS)
 #else
#define TITLEP(pstr) PUTS_ATT(0*FW,0,pstr,INVERS)
 #endif
#endif
#define TITLE(str)   TITLEP(str)


#define MENU(title, tab, menu, lines_count, ...) \
TITLE(title); \
static MState2 mstate2; \
static const uint8_t mstate_tab[] = __VA_ARGS__; \
event = mstate2.check(event,menu,tab,DIM(tab),mstate_tab,DIM(mstate_tab)-1,lines_count-1)

#ifdef COLOUR_DISPLAY
uint32_t checkPageMove( uint8_t event, uint8_t *pos, uint32_t max ) ;
#endif

void touchMenuTitle( char *s ) ;

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

#define TTOP			10
#define THTOP			14
#define TFH				12
#define TRIGHT	 180
#define TMID		 110
#define TLINES		9
#define TSCALE		2
#define TVOFF			2
#define THOFF			4
#define TRMARGIN	3

#define TSCROLLLEFT		380
#define TSCROLLTOP		16
#define TSCROLLWIDTH	42
#define TSCROLLBOTTOM	(240-16)

#else

#define TTOP			10
#define THTOP			18
#define TFH				12
#define TRIGHT	 180
#define TMID		 110
#define TLINES		9
#define TSCALE		2
#define TVOFF			2
#define THOFF			4
#define TRMARGIN	3

#endif

#define NO_HI_LEN 25

#if defined(PCBX12D) || defined(PCBX10)
#define FWPx4		"\060"
#define FWPx5		"\074"
#define FWPx9		"\152"
#define FWPx10		"\170"
#define FWPx11		"\204"
#define FWPx12		"\220"
#define FWPx13		"\234"
#define FWPx14		"\250"
#define FWPx15		"\264"
#define FWPx16		"\300"
#define FWPx17		"\314"
#define FWPx18		"\330"
#else
#define FWPx4		"\030"
#define FWPx5		"\036"
#define FWPx9		"\066"
#define FWPx10		"\074"
#define FWPx11		"\102"
#define FWPx12		"\110"
#define FWPx13		"\116"
#define FWPx14		"\124"
#define FWPx15		"\132"
#define FWPx16		"\140"
#define FWPx17		"\146"
#define FWPx18		"\154"
#endif

#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
#define FWchk			"\162"
#else
#define FWchk			"\162"
#endif

#if defined(PCBX12D) || defined(PCBX10)
#define PARAM_OFSP   17*FWCOLOUR
#else
#define PARAM_OFSP   17*FW
#endif
#define PARAM_OFS   17*FW

// Access protocol
#define ACC_NORMAL		0
#define ACC_REG				1
#define ACC_BIND			2
#define ACC_RXOPTIONS		3
#define ACC_TXOPTIONS		4
#define ACC_RX_HARDWARE	5
#define ACC_SHARE				6
#define ACC_SPECTRUM		7
#define ACC_ACCST_BIND			8


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
//#if defined(PCBX12D) || defined(PCBX10)
//	uint16_t PopupHpos ;
//	uint16_t PopupVpos ;
//	uint16_t PopupWidth ;
//#endif
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

char *getIndexedText( char *p, const char * s, uint8_t idx, uint32_t maxLen ) ;
																 
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
extern void menuGlobals(uint8_t event) ;
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

extern void setStickCenter(uint32_t toSubTrims ) ; // copy state of 3 primary to subtrim

void validateProtocolOptions( uint32_t module ) ;
extern int16_t calcExtendedValue( int16_t value, uint8_t extValue ) ;
extern uint16_t packExtendedValue( int16_t value ) ;

extern void parseMultiData( void ) ;

#if defined(PCBX12D) || defined(PCBX10)
void checkTheme( themeData *t ) ;
uint16_t dimBackColour() ;
#endif
#ifdef COLOUR_DISPLAY
//uint16_t dimBackColour() ;
void drawItem( char *s, uint16_t y, uint16_t colour ) ;
void drawNumberedItem( char *s, uint16_t y, uint16_t selected, int32_t number ) ;
void drawNumberedItemWide( char *s, uint16_t y, uint16_t selected, int32_t number ) ;
void drawNumber( uint16_t x, uint16_t y, int32_t val, uint16_t mode) ; //, uint16_t colour ) ;
void drawText( uint16_t x, uint16_t y, char *s, uint16_t mode ) ;
void drawChar( uint16_t x, uint16_t y, uint8_t c, uint16_t mode ) ;
void drawIdxText( uint16_t y, char *s, uint32_t index, uint16_t mode ) ;
void DrawDrSwitches( coord_t x, coord_t y, int8_t idx1, LcdFlags att) ;
uint32_t touchOnOffItem( uint8_t value, coord_t y, const prog_char *s, uint8_t condition, uint16_t colour ) ;
uint32_t touchOffOnItem( uint8_t value, coord_t y, const prog_char *s, uint8_t condition, uint16_t colour ) ;
void putsChnColour( coord_t x, coord_t y, uint8_t idx1, LcdFlags att ) ;
void saveEditColours( uint32_t attr, uint16_t colour ) ;
void restoreEditColours() ;
int32_t checkTouchSelect( uint32_t rows, uint32_t pgOfs, uint32_t flag = 0 ) ;
uint16_t processSelection( uint8_t vert, int32_t newSelection ) ;
void checkTouchEnterEdit( uint16_t value ) ;
uint16_t handleTouchSelect( uint32_t rows, uint32_t pgOfs, uint8_t sub, uint32_t flag = 0 ) ;
uint32_t checkTouchposition( uint16_t x, uint16_t y, uint16_t x2, uint16_t y2 ) ;
int32_t checkTouchArea( uint32_t x, uint32_t y, uint32_t w, uint32_t h ) ;
uint32_t touchOnOffItemMultiple( uint8_t value, coord_t y, uint8_t condition, uint16_t colour ) ;
void dispGvar( coord_t x, coord_t y, uint8_t gvar, LcdFlags attr ) ;

void menuLimitsOne(uint8_t event) ;
#ifdef TOUCH
uint32_t editTimer( uint8_t sub, uint8_t event ) ;
#else
void editTimer( uint8_t sub, uint8_t event ) ;
#endif
#endif


#define ALPHA_NO_NAME		0x80
void alphaEditName( coord_t x, coord_t y, uint8_t *name, uint8_t len, uint16_t type, uint8_t *heading ) ;

extern int16_t edit_dr_switch( coord_t x, coord_t y, int16_t drswitch, LcdFlags attr, LcdFlags flags, uint8_t event ) ;

const char *get_curve_string() ;

extern uint8_t unmapPots( uint8_t value ) ;

extern int16_t calc_scaler( uint8_t index, uint16_t *unit, uint8_t *num_decimals) ;

extern uint8_t CalcScaleNest ;

#define MAXTRACE 120
extern uint8_t s_traceBuf[] ;
extern uint8_t s_traceWr;
extern uint16_t s_traceCnt;

extern uint16_t Current ;
extern uint32_t Current_sum ;
extern uint8_t Current_count ;

extern uint8_t FileSelectResult ;
extern char SelectedVoiceFileName[] ;
extern TCHAR PlaylistDirectory[] ;
extern uint16_t PlaylistIndex ;
extern uint8_t VoiceFileType ;
extern struct fileControl PlayFileControl ;
extern char CurrentPlayName[] ;
extern uint16_t PlayListCount ;
extern char PlayListNames[][MUSIC_NAME_LENGTH+2] ;
extern uint8_t TrainerMode ;
extern uint8_t TrainerPolarity ;	// Input polarity

extern void copyFileName( char *dest, char *source, uint32_t size ) ;
extern void menuSelectVoiceFile(uint8_t event) ;
extern uint32_t fillPlaylist( TCHAR *dir, struct fileControl *fc, char *ext ) ;

#if defined(PCBX12D) || defined(PCBX10) || defined(PROP_TEXT)
#define BaudString FWx19"\006""\006  AUTO  9600 19200 38400 57600115200100000"
#else
#define BaudString FWx19"\006""\006  AUTO  9600 19200 38400 57600115200100000"
#endif

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

#define ALPHA_NO_NAME		0x80
#define ALPHA_FILENAME	0x40
#define ALPHA_HEX				0x100

#ifdef COLOUR_DISPLAY
#define RIGHT_POSITION	239
#else
#define RIGHT_POSITION	127
#endif


extern uint8_t TextIndex ;
extern uint8_t TextType ;
extern uint8_t TextResult ;

extern uint8_t InverseBlink ;
extern uint8_t EditColumns ;

void menuTextHelp(uint8_t event) ;
void sortTelemText() ;

struct t_textControl
{
	void (*TextFunction)( coord_t x, coord_t y, uint8_t index, LcdFlags att ) ;
	uint8_t TextMax ;
	uint8_t *TextMap ;
	uint8_t TextWidth ;
	uint8_t TextOption ;
} ;

extern struct t_textControl TextControl ;
extern uint8_t MaskRotaryLong ;

extern uint8_t EditType ;
extern uint8_t checkIndexed( coord_t y, const char *s, uint8_t value, uint8_t edit ) ;
extern void setCaptureMode(uint32_t mode) ;

extern void displayInputName( uint16_t x, uint16_t y, uint8_t inputIndex, LcdFlags attr ) ;

// Items now in menuscommon:
uint8_t checkOutOfOrder( uint8_t value, uint8_t *options, uint32_t count ) ;
const char *get_curve_string() ;
uint8_t locateMappedItem( uint8_t value, uint8_t *options, uint32_t count ) ;
#if defined(PCBX12D) || defined(PCBX10)
void menu_lcd_onoff( coord_t x, coord_t y, uint8_t value, uint16_t mode ) ;
#else
void menu_lcd_onoff( coord_t x,coord_t y, uint8_t value, uint8_t mode ) ;
#endif
uint8_t offonMenuItem( uint8_t value, coord_t y, const char *s, uint8_t condition ) ;
uint8_t onoffMenuItem( uint8_t value, coord_t y, const prog_char *s, uint8_t condition ) ;
uint8_t offonItem( uint8_t value, coord_t y, uint8_t condition ) ;
uint8_t onoffItem( uint8_t value, coord_t y, uint8_t condition ) ;
uint32_t checkForMenuEncoderLong( uint8_t event ) ;
uint32_t checkForMenuEncoderBreak( uint8_t event ) ;
uint32_t checkForExitEncoderLong( uint8_t event ) ;
int16_t m_to_ft( int16_t metres ) ;
int16_t c_to_f( int16_t degrees ) ;
int16_t knots_to_other( int16_t knots, uint32_t type ) ;
char *arduFlightMode( uint16_t mode ) ;
#if defined(PCBX12D) || defined(PCBX10)
uint8_t hyphinvMenuItem( uint8_t value, coord_t y, uint8_t condition, coord_t newX = 0 ) ;
#else
uint8_t hyphinvMenuItem( uint8_t value, coord_t y, uint8_t condition ) ;
#endif
uint16_t xnormalize( int16_t alpha ) ;
int8_t rxsin100( int16_t a ) ;
int8_t rxcos100( int16_t a ) ;
char *getTelText( uint32_t index ) ;
void displayIndex( const uint16_t *strings, uint8_t extra, uint8_t lines, uint8_t highlight ) ;
void lcd_xlabel_decimal( coord_t x, coord_t y, uint16_t value, LcdFlags attr, const char *s ) ;
void DisplayScreenIndex(uint32_t index, uint32_t count, LcdFlags attr) ;
int16_t calcExtendedValue( int16_t value, uint8_t extValue ) ;
uint16_t packExtendedValue( int16_t value ) ;

uint8_t indexProcess( uint8_t event, MState2 *pmstate, uint8_t extra ) ;
void menuProcDiagAna(uint8_t event) ;
void menuProcDiagKeys(uint8_t event) ;
void menuProcDiagCalib(uint8_t event) ;
void menuProcTrainer(uint8_t event) ;
void menuProcDiagVers(uint8_t event) ;
void menuBackupEeprom( uint8_t event ) ;
void menuRestoreEeprom( uint8_t event ) ;
void menuProcDate(uint8_t event) ;
void menuProcRSSI(uint8_t event) ;
void displayNext() ;
void displayIndex( const uint16_t *strings, uint8_t extra, uint8_t lines, uint8_t highlight ) ;
uint32_t checkForMenuEncoderBreak( uint8_t event ) ;
uint32_t checkForMenuEncoderLong( uint8_t event ) ;
void menuGlobalVoiceAlarm(uint8_t event) ;
void displayGPSformat( uint16_t x, uint16_t y, uint8_t attr ) ;
uint8_t edit3posSwitchSource( coord_t y, uint8_t value, uint16_t mask, uint8_t condition, uint8_t allowAna ) ;

void voiceMinutes( int16_t value ) ;

#ifdef PCB9XT
void editExtraPot( coord_t y, uint32_t index, uint32_t active ) ;
int16_t gvarMenuItem(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, uint8_t event ) ;
#endif


#ifdef COLOUR_DISPLAY
void menuBackground( uint8_t event ) ;
void menuForeground( uint8_t event ) ;
void displayIndexIcons( const uint8_t *icons[], uint8_t extra, uint8_t lines ) ;
#endif


#ifdef ARUNI
void putSwitchName( coord_t x, coord_t y, uint8_t z, LcdFlags att); // @menus.cpp
#else
void putSwitchName( coord_t x, coord_t y, uint8_t z, LcdFlags att) ;
// #if defined(PCBX12D) || defined(PCBX10)
//void putSwitchName( uint16_t x, uint16_t y, uint8_t z, uint16_t att) ;
// #else
//void putSwitchName(uint8_t x, uint8_t y, int8_t z, uint8_t att) ;
// #endif
#endif

#if defined(PCBX12D) || defined(PCBX10)
//void putsChnRaw(uint16_t x,uint16_t y,uint8_t idx,uint16_t att, uint16_t colour , uint16_t bgColour ) ;
uint32_t putTxSwr( coord_t x, coord_t y, LcdFlags attr, uint16_t colour = LcdForeground, uint16_t bgColour = LcdBackground ) ;
void putsAttIdxTelemItems( coord_t x, coord_t y, uint8_t index, LcdFlags attr, uint16_t colour = LcdForeground, uint16_t bgColour = LcdBackground ) ;
#else
//void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att) ;
uint32_t putTxSwr( coord_t x, coord_t y, LcdFlags attr ) ;
void putsAttIdxTelemItems( coord_t x, coord_t y, uint8_t index, LcdFlags attr ) ;
#endif



#if defined(PCBX12D) || defined(PCBX10)
void displayIndexIcons( const uint8_t *icons[], uint8_t extra, uint8_t lines ) ;
extern void lcdDrawIcon( uint16_t x, uint16_t y, const uint8_t * bitmap, uint8_t type ) ;
#endif


void put_curve( coord_t x, coord_t y, int8_t idx, LcdFlags attr ) ;
void displayVoiceRate( coord_t x, coord_t y, uint8_t rate, LcdFlags attr ) ;


#endif
void menuBindOptions(uint8_t event) ;

//void displayInputName( uint16_t x, uint16_t y, uint8_t inputIndex, uint16_t attr ) ;
int16_t processOneInput( struct te_InputsData *pinput, int16_t value ) ;
int16_t evalInput( uint8_t channel, int16_t value ) ;
void evalAllInputs() ;
int16_t getInputSourceValue( struct te_InputsData *pinput ) ;

#ifdef COLOUR_DISPLAY
void putTick( coord_t x, coord_t y, uint16_t colour = LcdForeground ) ;
#else
void putTick( coord_t x, coord_t y ) ;
#endif

struct t_clipboard
{
	uint32_t content ;
	union
	{
		VoiceAlarmData clipvoice ;
		SKYCSwData clipswitch ;
	} ;
} ;



