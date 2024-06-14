#ifndef SIMULATORDIALOG_H
#define SIMULATORDIALOG_H

#include <QDialog>
#include "../../common/node.h"
#include <stdint.h>
#include "pers.h"
#include "file.h"
#include "qextserialport.h"

#define TMR_OFF     0
#define TMR_RUNNING 1
#define TMR_BEEPING 2
#define TMR_STOPPED 3

#define FLASH_DURATION 10

#define FADE_FIRST	0x20
#define FADE_LAST		0x40

namespace Ui {
    class simulatorDialog;
}

struct t_timer
{
	uint16_t s_sum ;
	uint8_t lastSwPos ;
	uint8_t sw_toggled ;
	uint16_t s_timeCumSw ;  //laufzeit in 1/16 sec
	uint8_t  s_timerState ;
	uint8_t lastResetSwPos;
	uint16_t s_timeCumThr ;  //gewichtete laufzeit in 1/16 sec
	uint16_t s_timeCum16ThrP ; //gewichtete laufzeit in 1/16 sec
	int16_t  s_timerVal ;
	int16_t last_tmr ;
} ;


class simulatorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit simulatorDialog(QWidget *parent = 0);
    ~simulatorDialog();

    void loadParams(const EEGeneral gg, const SKYModelData gm, struct t_radioData *radioData);

private:
    Ui::simulatorDialog *ui;
    Node *nodeLeft;
    Node *nodeRight;
    QTimer *ticktimer;
    QString modelName;


    qint8   *trimptr[4];
    quint16 g_tmr10ms;
    qint16  chanOut[NUM_SKYCHNOUT];
    qint16  calibratedStick[7+2+3+1];
    qint16  StickValues[4] ;
    qint16  g_ppmIns[8];
    qint16  ex_chans[NUM_SKYCHNOUT];
    qint8   trim[4];
    qint16  sDelay[MAX_SKYMIXERS];
    qint32  act[MAX_SKYMIXERS];
    qint16  anas [NUM_SKYXCHNRAW+1+MAX_GVARS+1];	// Extra 1 for X9D
    qint32  chans[NUM_SKYCHNOUT];
		int16_t rawSticks[4] ;
    quint8  bpanaCenter;
    quint16 parametersLoaded ;
    bool    swOn[MAX_SKYMIXERS];
    quint16 one_sec_precount;
		int16_t	CsTimer_lastVal[NUM_SKYCSW] ;
    uint8_t Last_switch[NUM_SKYCSW] ;
		uint8_t Now_switch[NUM_SKYCSW] ;
    quint8  fadePhases ;
    qint32  fade[NUM_SKYCHNOUT];
		quint16	fadeScale[MAX_PHASES+1] ;
		quint16	fadeRate ;
		quint16 fadeWeight ;
    
		qint16 qdebug ;
		qint16 serialSending ;
		qint16 serialTimer ;
    QextSerialPort *port ;

		struct t_timer s_timer[2] ;

    quint16 s_timeCumTot;
    quint16 s_timeCumAbs;
    quint16 s_timeCumSw;
    quint16 s_timeCumThr;
    quint16 s_timeCum16ThrP;
    quint8  s_timerState;
    quint8  beepAgain;
    quint16 g_LightOffCounter;
    qint16  s_timerVal[2];
    quint16 s_time;
    quint16 s_cnt;
    quint16 s_sum;
    quint8  sw_toggled;
		quint8	CurrentPhase ;
		quint8	txType ;
		quint32	txBitType ;
		quint8  CalcScaleNest ;
    
		quint8  current_limits ;
		quint8  gvar_or_scalers ;
		quint8	VoiceCheckFlag100mS ;

    SKYModelData g_model;
    EEGeneral g_eeGeneral;

		int chVal(int val) ;
    void setupSticks();
    void setupTimer();
    void resizeEvent(QResizeEvent *event  = 0);

		uint32_t adjustMode( uint32_t x ) ;
    void getValues();
    void setValues();
    void perOut(bool init, uint8_t att);
		void perOutPhase( bool init, uint8_t att ) ;
    void centerSticks();
    void timerTick();
		void processAdjusters() ;
		void processSwitches(void) ;
    void processSwitchTimer( uint32_t i ) ;
		int32_t isAgvar(uint8_t value) ;
		PhaseData *getPhaseAddress( uint32_t phase ) ;

    bool keyState(EnumKeys key);
		bool hwKeyState(int key) ;
    qint16 getValue(qint8 i);
    bool getSwitch(int swtch, bool nc, qint8 level=0);
    bool getSwitchDr(int swtch);
    void beepWarn();
    void beepWarn1();
    void beepWarn2();

    int beepVal;
    int beepShow;

    int16_t intpol(int16_t x, uint8_t idx);
		int8_t REG100_100(int8_t x) ;
		int8_t REG(int8_t x, int8_t min, int8_t max) ;
		uint8_t REGisGvar( int8_t x ) ;

		uint32_t getFlightPhase() ;
		int16_t getRawTrimValue( uint8_t phase, uint8_t idx ) ;
		uint32_t getTrimFlightPhase( uint8_t phase, uint8_t idx ) ;
		int16_t getTrimValue( uint8_t phase, uint8_t idx ) ;
		void setTrimValue(uint8_t phase, uint8_t idx, int16_t trim) ;
		int16_t calc_scaler( uint8_t index ) ;
		void configSwitches( void ) ;
		uint8_t IS_THROTTLE( uint8_t x) ;
		int8_t getGvarSourceValue( uint8_t src ) ;
		void resetTimern( uint32_t timer ) ;
		void resetTimer1() ;
		void resetTimer2() ;
		void resetTimer() ;
		uint8_t Nvs_state[NUM_SKY_VOICE_ALARMS+NUM_EXTRA_VOICE_ALARMS] ;
		int16_t Nvs_timer[NUM_SKY_VOICE_ALARMS+NUM_EXTRA_VOICE_ALARMS] ;
		int16_t getInputSourceValue( struct te_InputsData *pinput ) ;
		int16_t mixApplyCurve( SKYMixData &md, int16_t v ) ;

protected:
		void closeEvent(QCloseEvent *event) ;

private slots:
    void on_FixRightY_clicked(bool checked);
    void on_FixRightX_clicked(bool checked);
    void on_FixLeftY_clicked(bool checked);
    void on_FixLeftX_clicked(bool checked);
    void on_holdRightY_clicked(bool checked);
    void on_holdRightX_clicked(bool checked);
    void on_holdLeftY_clicked(bool checked);
    void on_holdLeftX_clicked(bool checked);
		void on_SendDataButton_clicked() ;
		void on_GvarButton_clicked() ;
    void timerEvent();
		void voiceDisplay( QString name ) ;
		void processVoiceAlarms( void ) ;


};

#endif // SIMULATORDIALOG_H
