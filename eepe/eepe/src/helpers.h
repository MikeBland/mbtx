 #ifndef HELPERS_H
#define HELPERS_H

#include <QtGui>
#include <QtXml>
#include "pers.h"
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

//#include "../eepskye/src/myeeprom.h"

#ifdef SKY
#define TMR_NUM_OPTION  (TMR_VAROFS+24+8)
#else
#define TMR_NUM_OPTION  (TMR_VAROFS+16)
//#define TMR_NUM_OPTION  (TMR_VAROFS+2*MAX_DRSWITCH-3)
#endif
#define SPLASH_MARKER "Splash\0"
#define SPLASHS_MARKER "Spls\0"
#define SPLASH_WIDTH (128)
#define SPLASH_HEIGHT (64)
#define SPLASH_SIZE (SPLASH_WIDTH*SPLASH_HEIGHT/8)
#define SPLASHS_SIZE (SPLASH_WIDTH*SPLASH_HEIGHT/8/4)
#define SPLASH_OFFSET (6+1+3) // "Splash" + zero + 3 header bytes
#define SPLASHS_OFFSET (4+1) // "Spls" + zero
#define HEX_FILE_SIZE (1024*256)	// Allow for M2561 processor
#define BIN_FILE_SIZE (1024*1024)

//void setSubSubProtocol( QComboBox *b, int type ) ;
QString subSubProtocolText( int type, int index, QComboBox *b ) ;

#ifdef SKY
void populateAnaVolumeCB( QComboBox *b, int value, int type ) ;
void populateCustomAlarmCB( QComboBox *b, int type ) ;
#else
void populateAnaVolumeCB( QComboBox *b, int value ) ;
#endif
void populateSpinGVarCB( QSpinBox *sb, QComboBox *cb, QCheckBox *ck, int value, int min, int max, int xvalue = 0 ) ;
int numericSpinGvarValue( QSpinBox *sb, QComboBox *cb, QCheckBox *ck, int value, int defvar, int extended = 0 ) ;
int numericSpinGvarValue100( QSpinBox *sb, QComboBox *cb, QCheckBox *ck, int value, int defvar ) ;
void populateSpinGVarCB100( QSpinBox *sb, QComboBox *cb, QCheckBox *ck, int value ) ;

#ifdef SKY
void populateGvarCB(QComboBox *b, int value, int type, uint32_t extraPots) ;
#else
void populateGvarCB(QComboBox *b, int value, int type) ;
#endif

#ifdef SKY
QString gvarSourceString( int index, int type, uint32_t extraPots) ;
#endif

void populateNumericGVarCB( QComboBox *b, int value, int min, int max) ;
int numericGvarValue( QComboBox *b, int min, int max ) ;
#ifdef SKY
void populateSwitchCB(QComboBox *b, int value, int eepromType);
void populateDrSwitchCB(QComboBox *b, int value, int eepromBitType) ;
#else
void populateSwitchCB(QComboBox *b, int value, int eepromType) ;
#endif
void populateTrainerSwitchCB(QComboBox *b, int value) ;

#ifdef SKY
void populateSwitchShortCB(QComboBox *b, int value, int eepromType) ;
#else
void populateSwitchShortCB(QComboBox *b, int value, int eepromType) ;
#endif

#ifdef SKY
void populateSafetySwitchCB(QComboBox *b, int type, int value, int eepromType ) ;
#else
void populateSafetySwitchCB(QComboBox *b, int type, int value, int extra ) ;
#endif
void populateSafetyVoiceTypeCB(QComboBox *b, int type, int value);
#ifdef SKY
void populateTelItemsCB(QComboBox *b, int start, int value=0) ;
#else
void populateTelItemsCB(QComboBox *b, int start, int value) ;
#endif
void populateAlarmCB(QComboBox *b, int value);
void populateCurvesCB(QComboBox *b, int value);
#ifdef SKY
void populateTimerSwitchCB(QComboBox *b, int value ) ;
int getAndSwitchCbValue( QComboBox *b ) ;
#else
void populateTimerSwitchCB(QComboBox *b, int value, int eepromType) ;
#endif
void populateSwitchxAndCB(QComboBox *b, int value, int eepromType) ;
int getxAndSwitchCbValue( QComboBox *b, int eepromType ) ;
int32_t andSwitchMap( int32_t x ) ;
void populateSwitchAndCB(QComboBox *b, int value) ;
void x9dPopulateSwitchAndCB(QComboBox *b, int value) ;

void populateHardwareSwitch(QComboBox *b, int value ) ;

#ifdef SKY
void populateTmrBSwitchCB(QComboBox *b, int value, int eepromType) ;
#else
void populateTmrBSwitchCB(QComboBox *b, int value, int extra ) ;
#endif

#ifdef SKY
void populateSourceCB(QComboBox *b, int stickMode, int telem, int value, int modelVersion, int type, uint32_t extraPots ) ;
#else
void populateSourceCB(QComboBox *b, int stickMode, int telem, int value, int modelVersion) ;
#endif
#ifdef SKY    
uint32_t decodePots( uint32_t value, int type, uint32_t extraPots ) ;
#endif
void populateCSWCB(QComboBox *b, int value, uint8_t modelVersion );
#ifdef SKY
uint8_t locateSwFunc( int value ) ;
uint8_t unmapSwFunc( int value ) ;
int16_t convertTelemConstant( int8_t index, int8_t value, SKYModelData *model ) ;
#else
int16_t convertTelemConstant( int8_t index, int8_t value, ModelData *model ) ;
#endif
QString getTelemString( int index ) ;
#ifdef SKY    
QString getInputSourceStr(int idx ) ;
QString getSourceStr(int stickMode, int idx, int modelVersion, int type, uint32_t extraPots ) ;
#else
QString getSourceStr(int stickMode=1, int idx=0, int modelVersion=0 ) ;
#endif
#ifdef SKY
QString getTimerMode(int tm) ;
#else
QString getTimerMode(int tm, int modelVersion ) ;
#endif
#ifdef SKY
QString getSWName(int val, int eepromType);
#else
#ifndef V2
QString getSWName(int val, int extra ) ;
#endif
#endif
QString getMappedSWName(int val, int eepromType) ;
QString getCSWFunc(int val, uint8_t modelVersion ) ;
QString getAudioAlarmName(int val) ;

// Safety switch types
#define VOICE_SWITCH		6
int populatePhasetrim(QComboBox *b, int which, int value=0) ;
int decodePhaseTrim( int16_t *existing, int index ) ;

#ifdef SKY
void stringTelemetryChannel( char *string, int8_t index, int16_t val, SKYModelData *model ) ;
#else

void stringTelemetryChannel( char *string, int8_t index, int16_t val, ModelData *model ) ;
#endif

int  loadiHEX(QWidget *parent, QString fileName, quint8 * data, int datalen, QString header);
bool saveiHEX(QWidget *parent, QString fileName, quint8 * data, int datalen, QString header, int notesIndex=0, int useBlocks = 0);

void appendTextElement(QDomDocument * qdoc, QDomElement * pe, QString name, QString value);
void appendNumberElement(QDomDocument * qdoc, QDomElement * pe,QString name, int value, bool forceZeroWrite = false);
void appendCDATAElement(QDomDocument * qdoc, QDomElement * pe,QString name, const char * data, int size);

QDomElement getGeneralDataXML(QDomDocument * qdoc, EEGeneral * tgen);   //parse out data to XML format
bool loadGeneralDataXML(QDomDocument * qdoc, EEGeneral * tgen); // get data from XML

#ifdef SKY
QDomElement getModelDataXML(QDomDocument * qdoc, SKYModelData * tmod, int modelNum, int mdver); //parse out data to XML format
bool loadModelDataXML(QDomDocument * qdoc, SKYModelData * tmod, int modelNum = -1); // get data from XML
#else
bool loadModelDataXML(QDomDocument * qdoc, ModelData * tmod, int modelNum = -1); // get data from XML
QDomElement getModelDataXML(QDomDocument * qdoc, ModelData * tmod, int modelNum, int mdver); //parse out data to XML format
#endif

uint32_t getSplashHEX(QString fileName, uchar * b, QWidget *parent = 0);
bool putSplashHEX(QString fileName, uchar * b, QWidget *parent = 0);

bool getSplashBIN(QString fileName, uchar * b, QWidget *parent = 0);

//#ifdef SKY
//uint8_t CONVERT_MODE( uint8_t x ) ;
//#else
uint8_t CONVERT_MODE( uint8_t x, int modelVersion, int stickMode ) ;
//#endif

#ifdef SKY
extern int Found9Xtreme ;
QString FindErskyPath( int type ) ;
void modelConvert1to2( EEGeneral *g_eeGeneral, SKYModelData *g_model ) ;
#endif

#ifdef SKY
void createSwitchMapping( EEGeneral *pgeneral, uint8_t max_switch, int type ) ;
int8_t switchUnMap( int8_t x, int type ) ;
int8_t switchMap( int8_t x, int type ) ;
int getSwitchCbValue( QComboBox *b, int eepromType ) ;
int getDrSwitchCbValue( QComboBox *b, int eepromType ) ;
int getSwitchCbValueShort( QComboBox *b, int eepromType ) ;
int getTimerSwitchCbValue( QComboBox *b, int eepromType ) ;
uint8_t getSw3PosList( int index ) ;
uint8_t getSw3PosCount( int index ) ;
#else
void createSwitchMapping( EEGeneral *pgeneral, int type ) ;
int8_t switchUnMap( int8_t x ) ;
int8_t switchMap( int8_t x ) ;
int getSwitchCbValue( QComboBox *b, int type ) ;
int getSwitchCbValueShort( QComboBox *b, int type ) ;
int getTimerSwitchCbValue( QComboBox *b, int type ) ;
#endif

#ifdef SKY
uint8_t throttleReversed( EEGeneral *g_eeGeneral, SKYModelData *g_model ) ;
#else
uint8_t throttleReversed( EEGeneral *g_eeGeneral, ModelData *g_model ) ;
#endif

extern uint8_t CS_STATE( uint8_t x, uint8_t modelVersion ) ;

#ifdef SKY
extern QString RadioNames[] ;
#endif

#endif // HELPERS_H
