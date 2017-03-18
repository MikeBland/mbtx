#ifndef SIMULATORDIALOG_H
#define SIMULATORDIALOG_H

#include <QDialog>
#include "../../common/node.h"
#include <stdint.h>
#include "pers.h"
#include "qextserialport.h"
#include "modeledit.h"

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

class simulatorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit simulatorDialog( QWidget *parent = 0);
    ~simulatorDialog();

#ifdef V2
    void loadParams(const V2EEGeneral gg, const V2ModelData gm);
#else
    void loadParams(const EEGeneral gg, const ModelData gm);
#endif
		void setType( uint8_t type ) ;

private:
    Ui::simulatorDialog *ui;
    Node *nodeLeft;
    Node *nodeRight;
    QTimer *timer;
    QString modelName;
    qint16	ee_type ;

    qint8   *trimptr[4];
    quint16 g_tmr10ms;
    qint16  chanOut[NUM_CHNOUT];
    qint16  calibratedStick[7+2+3];
    qint16  StickValues[4] ;
    qint16  g_ppmIns[8];
    qint16  ex_chans[NUM_CHNOUT];
    qint8   trim[4];
    qint16  sDelay[MAX_MIXERS];
    qint32  act[MAX_MIXERS];
    qint16  anas [NUM_XCHNRAW+1+MAX_GVARS-NUM_CHNOUT];
//    qint16  internalChans[NUM_CHNOUT];
//    qint32  chans[NUM_CHNOUT];
		int16_t rawSticks[4] ;
    quint8  bpanaCenter;
    quint16 parametersLoaded ;
    bool    swOn[MAX_MIXERS];
    quint16 one_sec_precount;
		qint16		CsTimer[NUM_CSW+EXTRA_CSW] ;
    quint8  fadePhases ;
    qint32  fade[NUM_CHNOUT];
		quint16	fadeScale[MAX_PHASES+1] ;
		quint16	fadeRate ;
		quint16 fadeWeight ;

		qint16 qdebug ;
		qint16 serialSending ;
		qint16 serialTimer ;
    QextSerialPort *port ;

    quint16 s_timeCumTot;
    quint16 s_timeCumAbs;
    quint16 s_timeCumSw[2];
    quint16 s_timeCumThr[2];
    quint16 s_timeCum16ThrP[2];
    quint8  s_timerState[2];
    quint8  beepAgain;
    quint16 g_LightOffCounter;
    qint16  s_timerVal[2];
    quint16 s_time[2];
    quint16 s_cnt;
    quint16 s_sum;
    quint8  sw_toggled[2];
		quint8	CurrentPhase ;
		quint8  CalcScaleNest ;
		quint8  lastResetSwPos[2] ;

		quint8  current_limits ;

#ifdef V2
    V2ModelData g_model;
    V2EEGeneral g_eeGeneral;
#else
    ModelData g_model;
    EEGeneral g_eeGeneral;
#endif

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

    bool keyState(EnumKeys key);
		bool hwKeyState(int key) ;
    qint16 getValue(qint8 i);
    bool getSwitch(int swtch, bool nc, qint8 level=0);
    void beepWarn();
    void beepWarn1();
    void beepWarn2();

    int beepVal;
    int beepShow;

    int16_t intpol(int16_t x, uint8_t idx);
		int8_t REG100_100(int8_t x) ;
		int8_t REG(int8_t x, int8_t min, int8_t max) ;
		int16_t calcExpo( uint8_t channel, int16_t value ) ;

		uint32_t getFlightPhase() ;
		int16_t getRawTrimValue( uint8_t phase, uint8_t idx ) ;
		uint32_t getTrimFlightPhase( uint8_t phase, uint8_t idx ) ;
		int16_t getTrimValue( uint8_t phase, uint8_t idx ) ;
		void setTrimValue(uint8_t phase, uint8_t idx, int16_t trim) ;
		int16_t calc_scaler( uint8_t index ) ;
		uint8_t IS_THROTTLE( uint8_t x) ;
		void configSwitches( void ) ;

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
    void timerEvent();
		void setCsVisibles( void ) ;


};

#endif // SIMULATORDIALOG_H
