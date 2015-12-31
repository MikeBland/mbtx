#ifndef GENERALEDIT_H
#define GENERALEDIT_H

#include <QtGui>
#include <QDialog>
#include "pers.h"

namespace Ui {
    class GeneralEdit;
}

class GeneralEdit : public QDialog
{
    Q_OBJECT

public:
    explicit GeneralEdit(struct t_radioData *radioData, QWidget *parent = 0);
    ~GeneralEdit();
    void updateSettings();

private:
    Ui::GeneralEdit *ui;
    struct t_radioData *rData ;
//    EEPFILE *eeFile;

    EEGeneral g_eeGeneral;
    int StudWeight1,StudWeight2, StudWeight3, StudWeight4;
		int CurrentTrainerProfile ;
		int trainerTabLock ;
		int hardwareTabLock ;

//    bool switchDefPosEditLock;
//    void getGeneralSwitchDefPos(int i, bool val);
//    void setSwitchDefPos();
		void setHwSwitchActive( void ) ;
		void setHardwareSwitchCB( QComboBox *b, int switchlist, int type ) ;
		void setHardwarePotCB( QComboBox *b, int type ) ;

signals:
    void modelValuesChanged();

private slots:
    void updateTrainerTab();
    void trainerTabValueChanged();
		void saveTrainerToProfile() ;
		void loadTrainerFromProfile() ;
    void validateWeightSB();
    void on_TrainerProfileSB_valueChanged( int ) ;

    void on_ownerNameLE_editingFinished();
		void on_BtNameText_editingFinished() ;
		void on_BtDev1Name_editingFinished() ;
		void on_BtDev2Name_editingFinished() ;
		void on_BtDev3Name_editingFinished() ;
		void on_BtDev4Name_editingFinished() ;
		void btDevEdited( int dev, QLineEdit *uiDev ) ;

		uint8_t b2hex( uint8_t c ) ;
		uint8_t *btAddrBin2Hex( uint8_t *dest, uint8_t *source ) ;
		uint32_t btAddressValid( uint8_t *address ) ;

    void on_splashScreenChkB_stateChanged(int );
    void on_beepCountDownChkB_stateChanged(int );
    void on_beepMinuteChkB_stateChanged(int );
    void on_alarmwarnChkB_stateChanged(int );
    void on_tabWidget_currentChanged(int index);

//    void on_battCalib_editingFinished();
    void on_ana1Neg_editingFinished();
    void on_ana2Neg_editingFinished();
    void on_ana3Neg_editingFinished();
    void on_ana4Neg_editingFinished();
    void on_ana5Neg_editingFinished();
    void on_ana6Neg_editingFinished();
    void on_ana7Neg_editingFinished();
    void on_ana8Neg_editingFinished();

    void on_ana1Mid_editingFinished();
    void on_ana2Mid_editingFinished();
    void on_ana3Mid_editingFinished();
    void on_ana4Mid_editingFinished();
    void on_ana5Mid_editingFinished();
    void on_ana6Mid_editingFinished();
    void on_ana7Mid_editingFinished();
    void on_ana8Mid_editingFinished();

    void on_ana1Pos_editingFinished();
    void on_ana2Pos_editingFinished();
    void on_ana3Pos_editingFinished();
    void on_ana4Pos_editingFinished();
    void on_ana5Pos_editingFinished();
    void on_ana6Pos_editingFinished();
    void on_ana7Pos_editingFinished();
    void on_ana8Pos_editingFinished();

    void on_ExtRtcCB_currentIndexChanged(int index);

    void on_stickmodeCB_currentIndexChanged(int index);
    void on_channelorderCB_currentIndexChanged(int index);
    void on_languageCB_currentIndexChanged(int index);
    void on_beeperCB_currentIndexChanged(int index);
//    void on_memwarnChkB_stateChanged(int );
//    void on_switchwarnChkB_stateChanged(int );
    void on_thrwarnChkB_stateChanged(int );
    void on_inputfilterCB_currentIndexChanged(int index);
    void on_thrrevChkB_stateChanged(int );
    void on_inactimerSB_editingFinished();
		void on_inactVolumeSB_editingFinished() ;
    void on_backlightautoSB_editingFinished();
    void on_backlightswCB_currentIndexChanged(int index);
//    void on_battcalibDSB_editingFinished();
    void on_battwarningDSB_editingFinished();
    void on_contrastSB_editingFinished();
		void on_volumeSB_editingFinished();
    void on_beepFlashChkB_stateChanged(int );
    void on_speakerPitchSB_editingFinished();
    void on_hapticStengthSB_editingFinished();
//    void on_soundModeCB_currentIndexChanged(int index);
    void on_tabWidget_selected(QString );
    void on_PotScrollEnableChkB_stateChanged(int );
		void on_StickScrollEnableChkB_stateChanged(int ) ;
		void on_SoftwareVolumeChkB_stateChanged(int ) ;
		void on_Ar9xChkB_stateChanged(int ) ;
		void on_MenuEditChkB_stateChanged(int ) ;
		void on_CrossTrimChkB_stateChanged(int ) ;
		void on_hapticMinRunSB_editingFinished() ;
//    void on_BandGapEnableChkB_stateChanged(int );
    void on_splashScreenNameChkB_stateChanged(int );
    void on_backlightStickMove_editingFinished();
//		void on_enablePpmsimChkB_stateChanged(int );
		void on_internalFrskyAlarmChkB_stateChanged(int );
		void on_OptrexDisplayChkB_stateChanged(int ) ;
//		void on_backlightinvertChkB_stateChanged(int );
		void on_RotaryDivisorCB_currentIndexChanged(int ) ;
		void on_BtBaudrateCB_currentIndexChanged(int ) ;
		void on_CurrentCalibSB_editingFinished() ;
		void on_MaHalarmSB_editingFinished() ;
		void on_brightSB_editingFinished() ;
		void on_brightGreenSB_editingFinished() ;
		void on_brightBlueSB_editingFinished() ;
		void do_stick_gain() ;
		void on_stickgainLVCB_currentIndexChanged(int ) ;
		void on_stickgainLHCB_currentIndexChanged(int ) ;
		void on_stickgainRVCB_currentIndexChanged(int ) ;
		void on_stickgainRHCB_currentIndexChanged(int ) ;
		void on_BluetoothTypeCB_currentIndexChanged(int) ;

//    void on_switchDefPos_1_stateChanged(int );
//    void on_switchDefPos_2_stateChanged(int );
//    void on_switchDefPos_3_stateChanged(int );
//    void on_switchDefPos_4_stateChanged(int );
//    void on_switchDefPos_5_stateChanged(int );
//    void on_switchDefPos_6_stateChanged(int );
//    void on_switchDefPos_7_stateChanged(int );
//    void on_switchDefPos_8_stateChanged(int );

		void on_StickRevLH_stateChanged(int ) ;
		void on_StickRevLV_stateChanged(int ) ;
		void on_StickRevRV_stateChanged(int ) ;
		void on_StickRevRH_stateChanged(int ) ;

    void on_StickLHdeadbandSB_editingFinished() ;
    void on_StickLVdeadbandSB_editingFinished() ;
    void on_StickRVdeadbandSB_editingFinished() ;
    void on_StickRHdeadbandSB_editingFinished() ;

		void on_AilCB_currentIndexChanged(int ) ;
		void on_EleCB_currentIndexChanged(int ) ;
		void on_GeaCB_currentIndexChanged(int ) ;
		void on_RudCB_currentIndexChanged(int ) ;
		void on_ThrCB_currentIndexChanged(int ) ;
		void on_PB1CB_currentIndexChanged(int ) ;
		void on_PB2CB_currentIndexChanged(int ) ;
		void on_PB3CB_currentIndexChanged(int ) ;
		void on_PB4CB_currentIndexChanged(int ) ;
		void on_Pot4CB_currentIndexChanged(int ) ;
		void on_Pot5CB_currentIndexChanged(int ) ;
		void on_EncoderCB_currentIndexChanged(int ) ;
		void on_SixPosCB_currentIndexChanged(int ) ;

};

#endif // GENERALEDIT_H
