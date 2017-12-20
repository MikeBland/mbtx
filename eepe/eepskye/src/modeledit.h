#ifndef MODELEDIT_H
#define MODELEDIT_H

#include <QDialog>
#include <QtGui>
#include "pers.h"
#include "mixerslist.h"

#include <QListWidget>
class VoiceList : public QListWidget
{
    Q_OBJECT
public:
    explicit VoiceList(QWidget *parent = 0);
//    QMimeData * mimeData ( const QList<QListWidgetItem *> items );

    void keyPressEvent(QKeyEvent *event);

signals:
//    void mimeDropped(int index, const QMimeData *data, Qt::DropAction action);
    void keyWasPressed(QKeyEvent *event);

protected:

public slots:
//    bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);

private:
//    QPoint dragStartPosition;

};


class QPen;
class QSpinBox;
class QComboBox;
class QLabel;
class QTextBrowser;
class QCheckBox;

namespace Ui {
    class ModelEdit;
}

struct t_templateValues
{
	uint8_t stick ;
	uint8_t outputChannel ;
	uint8_t helperChannel ;
	uint8_t switch1 ;
	uint8_t switch2 ;
	uint8_t switch3 ;
} ;


class ModelEdit : public QDialog
{
    Q_OBJECT

public:
    explicit ModelEdit(struct t_radioData *radioData, uint8_t id, QWidget *parent = 0);
    ~ModelEdit();

    void applyBaseTemplate();
    bool redrawCurve;

    QString getNote(int i) { return mixNotes[i]; }
    void setNote(int i, QString s);
    int getModelID() { return id_model; }
    void refreshMixerList() { tabMixes(); }
		int getNodeMin( QSpinBox *sb ) ;
		int getNodeMax( QSpinBox *sb ) ;

private:
    struct t_radioData *rData ;
    Ui::ModelEdit *ui;
    EEPFILE *eeFile;
    class simulatorDialog *sdptr ;
    bool switchDefPosEditLock;

    MixersList *MixerlistWidget;
		VoiceList *VoiceListWidget ;

    QString mixNotes[MAX_SKYMIXERS + EXTRA_SKYMIXERS];

    EEGeneral g_eeGeneral;
    SKYModelData g_model;
    int       id_model;

		struct t_module oldModules[2] ;
    
		bool switchEditLock;
    bool heliEditLock;
    bool protocolEditLock;
    bool plot_curve[19];
    bool switchesTabDone ;
    bool customAlarmLock ;
    bool curveEditLock ;
    
    int oldAdjFunction[NUM_GVAR_ADJUST_SKY] ;
    QSpinBox  * cswitchOffset[NUM_SKYCSW];
    QSpinBox  * cswitchOffset0[NUM_SKYCSW];
    QComboBox * cswitchSource1[NUM_SKYCSW];
    QComboBox * cswitchSource2[NUM_SKYCSW];
    QComboBox * cswitchAndSwitch[NUM_SKYCSW];
    QLabel		* cswitchTlabel[NUM_SKYCSW];
		QTextBrowser * cswitchText1[NUM_SKYCSW];
		QTextBrowser * cswitchText2[NUM_SKYCSW];
    QDoubleSpinBox  *cswitchDelay[NUM_SKYCSW];

    QSpinBox  * safetySwitchValue[NUM_SKYCHNOUT+NUM_VOICE];
    QComboBox * safetySwitchSwtch[NUM_SKYCHNOUT+NUM_VOICE];
		QComboBox * safetySwitchType[NUM_SKYCHNOUT+NUM_VOICE];
    QComboBox * safetySwitchAlarm[NUM_SKYCHNOUT+NUM_VOICE];
		QCheckBox *safetySwitchGvar[NUM_SKYCHNOUT+NUM_VOICE] ;
		QComboBox *safetySwitchGindex[NUM_SKYCHNOUT+NUM_VOICE] ;
		QComboBox *safetySwitchSource[NUM_SKYCHNOUT+NUM_VOICE] ;

    QSpinBox *expoDrSpin[4][3][2][2] ;
    QComboBox *expoDrVal[4][3][2][2] ;
		QCheckBox *expoDrGvar[4][3][2][2] ;

		QSpinBox *posb[NUM_SCALERS] ;
		QSpinBox *pmsb[NUM_SCALERS] ;
		QSpinBox *pdivsb[NUM_SCALERS] ;
		QSpinBox *pdpsb[NUM_SCALERS] ;
		QComboBox *pucb[NUM_SCALERS] ;
		QComboBox *psgncb[NUM_SCALERS] ;
		QComboBox *poffcb[NUM_SCALERS] ;
		QComboBox *psrccb[NUM_SCALERS] ;
		QLineEdit *psname[NUM_SCALERS] ;
		QSpinBox *pmodsb[NUM_SCALERS] ;
		QComboBox *pdestcb[NUM_SCALERS] ;


    void setupMixerListWidget();
    void updateSettings();
    void tabModelEditSetup();
    void tabExpo();
    void tabMixes();
    void tabHeli();
    void tabLimits();
    void tabCurves();
    void tabSwitches();
    void tabSafetySwitches();
    void tabTrims();
    void tabFrsky();
    void tabTemplates();
		void tabPhase();
		void tabGvar();
		void tabVoiceAlarms() ;
		void voiceAlarmsList() ;
    void updateCurvesTab();
    void setSwitchWidgetVisibility(int i);
		void setSafetyWidgetVisibility(int i);
//		void oneGvarVisibility(int index, QComboBox *b, QSpinBox *sb ) ;
//		void gvarVisibility() ;
//		void oneGvarGetValue(int index, QComboBox *b, QSpinBox *sb ) ;
    void setLimitMinMax();
    void updateSwitchesList( int lOrR ) ;
    void updateSwitchesTab();
    void updateHeliTab();
		void setSafetyLabels() ;
		void updatePhaseTab() ;
		void textUpdate( QLineEdit *source, char *dest, int length ) ;
		int16_t getRawTrimValue( uint8_t phase, uint8_t idx ) ;
		uint32_t getTrimFlightPhase( uint8_t phase, uint8_t idx ) ;
		int16_t getTrimValue( uint8_t phase, uint8_t idx ) ;
		void phaseSet(int phase, int trim, QComboBox *cb, QSpinBox *sb );
    uint32_t countExtraPots() ;
		int curveXcheck( QSpinBox *sb ) ;

    void launchSimulation();
    void resizeEvent(QResizeEvent *event  = 0);

    void setProtocolBoxes();
//		void setSubSubProtocol( QComboBox *b, int type ) ;

    void drawCurve();
    int currentCurve;
    void setCurrentCurve(int curveId);

    QSpinBox *getNodeSB(int i);

    SKYMixData *mixAddress( uint32_t index ) ;
    bool gm_insertMix(int idx);
    int getMixerIndex(int dch);
    void gm_deleteMix(int index);
    void gm_openMix(int index);
    int gm_moveMix(int idx, bool dir);
    void mixersDeleteList(QList<int> list);
    QList<int> createListFromSelected();
    void setSelectedByList(QList<int> list);

    void applyTemplate(uint8_t idx);
    SKYMixData* setDest(uint8_t dch);
    void setCurve(uint8_t c, int8_t ar[]);
    void setSwitch(uint8_t idx, uint8_t func, int8_t v1, int8_t v2);

		struct t_templateValues templateValues ;
	void voiceAlarmsBlank( int i ) ;



signals:
    void modelValuesChanged(ModelEdit * = 0);

private slots:
    void clearMixes(bool ask=true);
    void clearCurves(bool ask=true);

    void on_extendedLimitsChkB_toggled(bool checked);
    void on_resetCurve_1_clicked();
    void on_resetCurve_2_clicked();
    void on_resetCurve_3_clicked();
    void on_resetCurve_4_clicked();
    void on_resetCurve_5_clicked();
    void on_resetCurve_6_clicked();
    void on_resetCurve_7_clicked();
    void on_resetCurve_8_clicked();
    void on_resetCurve_9_clicked();
    void on_resetCurve_10_clicked();
    void on_resetCurve_11_clicked();
    void on_resetCurve_12_clicked();
    void on_resetCurve_13_clicked();
    void on_resetCurve_14_clicked();
    void on_resetCurve_15_clicked();
    void on_resetCurve_16_clicked();
    void on_resetCurve_17_clicked();
    void on_resetCurve_18_clicked();
    void on_resetCurve_19_clicked();

    void mimeDropped(int index, const QMimeData *data, Qt::DropAction action);
    void pasteMIMEData(const QMimeData * mimeData, int destIdx=1000);
    void on_pushButton_clicked();
		void on_updateButton_clicked() ;
		void on_updateButton3_clicked() ;
		void on_updateButton4_clicked() ;
		void on_loggingButton_clicked() ;
    void mixersDelete(bool ask=true);
    void mixersCut();
    void mixersCopy();
    void mixersPaste();
    void mixersDuplicate();
    void mixerOpen();
    void mixerAdd();
    void moveMixUp();
    void moveMixDown();

    void mixerlistWidget_customContextMenuRequested(QPoint pos);
    void mixerlistWidget_doubleClicked(QModelIndex index);
    void mixerlistWidget_KeyPress(QKeyEvent *event);

    void voiceAlarmList_doubleClicked(QModelIndex index) ;
		void on_AdjusterList_doubleClicked( QModelIndex index ) ;

    void on_curveEdit_1_clicked();
    void on_curveEdit_2_clicked();
    void on_curveEdit_3_clicked();
    void on_curveEdit_4_clicked();
    void on_curveEdit_5_clicked();
    void on_curveEdit_6_clicked();
    void on_curveEdit_7_clicked();
    void on_curveEdit_8_clicked();
    void on_curveEdit_9_clicked();
    void on_curveEdit_10_clicked();
    void on_curveEdit_11_clicked();
    void on_curveEdit_12_clicked();
    void on_curveEdit_13_clicked();
    void on_curveEdit_14_clicked();
    void on_curveEdit_15_clicked();
    void on_curveEdit_16_clicked();
    void on_curveEdit_17_clicked();
    void on_curveEdit_18_clicked();
		void on_curveEdit_19_clicked();

    void on_plotCB_1_toggled(bool checked);
    void on_plotCB_2_toggled(bool checked);
    void on_plotCB_3_toggled(bool checked);
    void on_plotCB_4_toggled(bool checked);
    void on_plotCB_5_toggled(bool checked);
    void on_plotCB_6_toggled(bool checked);
    void on_plotCB_7_toggled(bool checked);
    void on_plotCB_8_toggled(bool checked);
    void on_plotCB_9_toggled(bool checked);
    void on_plotCB_10_toggled(bool checked);
    void on_plotCB_11_toggled(bool checked);
    void on_plotCB_12_toggled(bool checked);
    void on_plotCB_13_toggled(bool checked);
    void on_plotCB_14_toggled(bool checked);
    void on_plotCB_15_toggled(bool checked);
    void on_plotCB_16_toggled(bool checked);
    void on_plotCB_17_toggled(bool checked);
    void on_plotCB_18_toggled(bool checked);
		void on_plotCB_19_toggled(bool checked);

    void curvePointEdited();
    void curveXPointEdited();
    void limitEdited();
    void limitAuto();
    void switchesEdited();
    void safetySwitchesEdited();
    void expoEdited();
    void mixesEdited();
    void heliEdited();
    void FrSkyEdited();
		void FrSkyA1changed(int value) ;
		void FrSkyA2changed(int value) ;
		void GvarEdited() ;
		void phaseEdited() ;
    void setSwitchDefPos();
		uint16_t oneSwitchPos( uint8_t swtch, uint16_t states ) ;

    void on_spinBox_S1_valueChanged(int value);
    void on_spinBox_S2_valueChanged(int value);
    void on_spinBox_S3_valueChanged(int value);
    void on_spinBox_S4_valueChanged(int value);

    void on_bcRUDChkB_toggled(bool checked);
    void on_bcELEChkB_toggled(bool checked);
    void on_bcTHRChkB_toggled(bool checked);
    void on_bcAILChkB_toggled(bool checked);
    void on_bcP1ChkB_toggled(bool checked);
    void on_bcP2ChkB_toggled(bool checked);
    void on_bcP3ChkB_toggled(bool checked);
		void on_bcP4ChkB_toggled(bool checked) ;
    void on_timer1BeepCdownCB_toggled(bool checked);
    void on_timer2BeepCdownCB_toggled(bool checked);
    void on_timer1MinuteBeepCB_toggled(bool checked);
    void on_timer2MinuteBeepCB_toggled(bool checked);

    void on_thrExpoChkB_toggled(bool checked);
    void on_thrTrimChkB_toggled(bool checked);
    void on_thrIdleChkB_toggled(bool checked) ;
		void on_thrRevChkB_toggled(bool checked) ;
		void on_trainerCB_currentIndexChanged(int index) ;
//    void on_trimScaledChkB_toggled(bool checked);
//    void on_T2ThrTrgChkB_toggled(bool checked);
		
    void on_ppmDelaySB_editingFinished();
    void on_numChannelsSB_editingFinished();
    void on_xnumChannelsSB_editingFinished();
		void on_numChannels2SB_editingFinished() ;
		void on_TrainerChannelsSB_editingFinished() ;
		void on_TrainerDelaySB_editingFinished() ;
		void on_TrainerFrameLengthDSB_editingFinished() ;
		void on_TrainerStartChannelSB_editingFinished() ;
		void on_TrainerPolarityCB_currentIndexChanged(int index) ;
		void on_startChannelsSB_editingFinished() ;
		void on_xstartChannelsSB_editingFinished() ;
		void on_startChannels2SB_valueChanged( int x ) ;
    void on_timerValTE_editingFinished();
    void on_timer2ValTE_editingFinished();
    void on_protocolCB_currentIndexChanged(int index);
    void on_xprotocolCB_currentIndexChanged(int index);
    void on_pulsePolCB_currentIndexChanged(int index);
    void on_xpulsePolCB_currentIndexChanged(int index);
    void on_trimSWCB_currentIndexChanged(int index);
    void on_trimIncCB_currentIndexChanged(int index);
    void on_volumeControlCB_currentIndexChanged(int index) ;
    void on_timerDirCB_currentIndexChanged(int index);
    void on_timerModeCB_currentIndexChanged(int index);
    void on_timerModeBCB_currentIndexChanged(int index);
		void on_timerResetCB_currentIndexChanged(int index);
    void on_timer2DirCB_currentIndexChanged(int index);
    void on_timer2ModeCB_currentIndexChanged(int index);
    void on_timer2ModeBCB_currentIndexChanged(int index);
		void on_timer2ResetCB_currentIndexChanged(int index);
    void on_modelNameLE_editingFinished();
		void on_modelImageLE_editingFinished() ;
    void on_voiceNameLE_editingFinished() ;
    void on_tabWidget_currentChanged(int index);
    void on_templateList_doubleClicked(QModelIndex index);
    void on_ppmFrameLengthDSB_editingFinished();
    void on_xppmFrameLengthDSB_editingFinished();
    void ControlCurveSignal(bool flag);
    void on_DSM_Type_currentIndexChanged(int index);
    void on_xDSM_Type_currentIndexChanged(int index);
    void on_SubProtocolCB_currentIndexChanged(int index);
		void on_SubSubProtocolCB_currentIndexChanged(int index) ;
    void on_xSubProtocolCB_currentIndexChanged(int index);
		void on_xsubSubProtocolCB_currentIndexChanged(int index) ;
    void on_pxxRxNum_editingFinished();
		void on_VoiceNumberSB_editingFinished() ;
		void on_VoiceNumberSB_valueChanged( int x ) ;
		void on_autoLimitsSB_editingFinished() ;
		void on_countryCB_currentIndexChanged(int index) ;
		void on_xcountryCB_currentIndexChanged(int index) ;
		void on_typeCB_currentIndexChanged(int index) ;
		void on_xtypeCB_currentIndexChanged(int index) ;
		void on_multiOption_editingFinished() ;
		void on_autobindCB_currentIndexChanged(int index) ;
		void on_powerCB_currentIndexChanged(int index) ;
		void on_xmultiOption_editingFinished() ;
		void on_xautobindCB_currentIndexChanged(int index) ;
		void on_xpowerCB_currentIndexChanged(int index) ;
		void updateToMV2( void ) ;
		void updateToMV3( void ) ;
		void updateToMV4( void ) ;
		void convertToModules( struct t_module *modules ) ;
		void convertFromModules( struct t_module *modules ) ;
		void buildProtocoText( struct t_module *pmodule, QListWidget *pdisplayList, int module ) ;
		void on_Com2BaudrateCB_currentIndexChanged(int index) ;
		void on_internalModuleDisplayList_doubleClicked() ;
		void on_externalModuleDisplayList_doubleClicked() ;
		void on_SwListL_doubleClicked( QModelIndex index ) ;
		void on_SwListR_doubleClicked( QModelIndex index ) ;

		void on_switchwarnChkB_stateChanged(int ) ;
		void getModelSwitchDefPos(int i, bool val) ;
    void on_switchDefPos_1_stateChanged(int );
    void on_switchDefPos_2_stateChanged(int );
    void on_switchDefPos_3_stateChanged(int );
    void on_switchDefPos_4_stateChanged(int );
    void on_switchDefPos_5_stateChanged(int );
    void on_switchDefPos_6_stateChanged(int );
    void on_switchDefPos_7_stateChanged(int );
    void on_switchDefPos_8_stateChanged(int );
		void on_SwitchDefSA_valueChanged( int x ) ;
		void on_SwitchDefSB_valueChanged( int x ) ;
		void on_SwitchDefSC_valueChanged( int x ) ;
		void on_SwitchDefSD_valueChanged( int x ) ;
		void on_SwitchDefSE_valueChanged( int x ) ;
		void on_SwitchDefSF_valueChanged( int x ) ;
		void on_SwitchDefSG_valueChanged( int x ) ;
		void on_EnA_stateChanged(int ) ;
		void on_EnB_stateChanged(int ) ;
		void on_EnC_stateChanged(int ) ;
    void on_EnD_stateChanged(int ) ;
		void on_EnE_stateChanged(int ) ;
		void on_EnF_stateChanged(int ) ;
		void on_EnG_stateChanged(int ) ;
		void on_EnThr_stateChanged(int ) ;
		void on_EnRud_stateChanged(int ) ;
		void on_EnEle_stateChanged(int ) ;
		void on_EnIdx_stateChanged(int ) ;
		void on_EnAil_stateChanged(int ) ;
		void on_EnGea_stateChanged(int ) ;

		void on_AutoBtConnectChkB_stateChanged(int ) ;
		void on_UseStickNamesChkB_stateChanged(int ) ;

    void on_CustomAlarmSourceCB_currentIndexChanged(int index) ;
		void on_CustomAlarmMinSB_editingFinished() ;
		void on_CustomAlarmMaxSB_editingFinished() ;

		void on_BtDefaultAddrSB_editingFinished() ;

		void on_MusicStartCB_currentIndexChanged(int) ;
		void on_MusicPauseCB_currentIndexChanged(int) ;
		void on_MusicPrevCB_currentIndexChanged(int) ;
		void on_MusicNextCB_currentIndexChanged(int) ;

		void voiceAdd() ;
		void voiceRemove() ;
	void voiceBlank() ;
	void voiceMoveUp() ;
	void voiceMoveDown() ;
    void showVoiceContextMenu(QPoint pos);
	void voiceCopy() ;
	void voicePaste() ;
	void voice_KeyPress(QKeyEvent *event) ;


};



#endif // MODELEDIT_H

