#include "generaledit.h"
#include "ui_generaledit.h"
#include "pers.h"
#include "file.h"
#include "helpers.h"
#include <QtGui>

#define BIT_WARN_THR     ( 0x01 )
#define BIT_WARN_SW      ( 0x02 )
#define BIT_WARN_MEM     ( 0x04 )
#define BIT_WARN_BEEP    ( 0x80 )
#define BIT_BEEP_VAL     ( 0x38 ) // >>3
#define BEEP_VAL_SHIFT   3

extern int GlobalModified ;
extern EEGeneral Sim_g ;
extern int GeneralDataValid ;
extern ModelData Sim_m ;
extern int ModelDataValid ;

GeneralEdit::GeneralEdit( struct t_radioData *radioData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralEdit)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/icon.png"));
//    eeFile = eFile;

    rData = radioData ;
//    switchDefPosEditLock = false;
    QSettings settings("er9x-eePskye", "eePskye");
    ui->tabWidget->setCurrentIndex(settings.value("generalEditTab", 0).toInt());

		memcpy(  &g_eeGeneral, &radioData->generalSettings, sizeof( g_eeGeneral) ) ;
		hardwareTabLock = 1 ;
//    eeFile->getGeneralSettings(&g_eeGeneral);
    createSwitchMapping( &g_eeGeneral, MAX_DRSWITCH, rData->type ) ;

    QRegExp rx(CHAR_FOR_NAMES_REGEX);
    ui->ownerNameLE->setValidator(new QRegExpValidator(rx, this));
    ui->welcomeFileNameLE->setValidator(new QRegExpValidator(rx, this));

    populateSwitchCB(ui->backlightswCB,g_eeGeneral.lightSw, rData->type );

    ui->ownerNameLE->setText(g_eeGeneral.ownerName) ;
    ui->BtNameText->setText( (char *)g_eeGeneral.btName) ;

    ui->BtDev1Name->setText( (char *)g_eeGeneral.btDevice[0].name) ;
    ui->BtDev2Name->setText( (char *)g_eeGeneral.btDevice[1].name) ;
    ui->BtDev3Name->setText( (char *)g_eeGeneral.btDevice[2].name) ;
    ui->BtDev4Name->setText( (char *)g_eeGeneral.btDevice[3].name) ;

		if ( btAddressValid( g_eeGeneral.btDevice[0].address ) )
		{
      uint8_t text[16] ;
			btAddrBin2Hex( text, g_eeGeneral.btDevice[0].address ) ;
      ui->BtDev1Address->setText((char *)text) ;
		}
		if ( btAddressValid( g_eeGeneral.btDevice[1].address ) )
		{
      uint8_t text[16] ;
			btAddrBin2Hex( text, g_eeGeneral.btDevice[1].address ) ;
      ui->BtDev2Address->setText((char *)text) ;
		}
		if ( btAddressValid( g_eeGeneral.btDevice[2].address ) )
		{
      uint8_t text[16] ;
			btAddrBin2Hex( text, g_eeGeneral.btDevice[2].address ) ;
      ui->BtDev3Address->setText((char *)text) ;
		}
		if ( btAddressValid( g_eeGeneral.btDevice[3].address ) )
		{
      uint8_t text[16] ;
			btAddrBin2Hex( text, g_eeGeneral.btDevice[3].address ) ;
      ui->BtDev4Address->setText((char *)text) ;
		}

    ui->contrastSB->setValue(g_eeGeneral.contrast);
    ui->battwarningDSB->setValue((double)g_eeGeneral.vBatWarn/10);
//    ui->battcalibDSB->setValue((double)g_eeGeneral.vBatCalib/10);
    ui->battCalib->setValue((double)g_eeGeneral.vBatCalib/10);
    ui->backlightautoSB->setValue(g_eeGeneral.lightAutoOff*5);
    ui->backlightStickMove->setValue(g_eeGeneral.lightOnStickMove*5);
    ui->inactimerSB->setValue(g_eeGeneral.inactivityTimer+10);
    ui->inactVolumeSB->setValue(g_eeGeneral.inactivityVolume+21) ;

//    ui->soundModeCB->setCurrentIndex(g_eeGeneral.speakerMode);
    ui->speakerPitchSB->setValue(g_eeGeneral.speakerPitch);
    ui->hapticStengthSB->setValue(g_eeGeneral.hapticStrength);

    ui->thrrevChkB->setChecked(g_eeGeneral.throttleReversed);
    ui->inputfilterCB->setCurrentIndex(g_eeGeneral.filterInput);
    ui->thrwarnChkB->setChecked(!g_eeGeneral.disableThrottleWarning);   //Default is zero=checked
//    ui->switchwarnChkB->setChecked(!g_eeGeneral.disableSwitchWarning); //Default is zero=checked
//    ui->memwarnChkB->setChecked(!g_eeGeneral.disableMemoryWarning);   //Default is zero=checked
    ui->alarmwarnChkB->setChecked(!g_eeGeneral.disableAlarmWarning);//Default is zero=checked
    ui->PotScrollEnableChkB->setChecked(!g_eeGeneral.disablePotScroll);//Default is zero=checked
    ui->StickScrollEnableChkB->setChecked(g_eeGeneral.stickScroll);
    ui->CrossTrimChkB->setChecked(g_eeGeneral.crosstrim);
//    ui->BandGapEnableChkB->setChecked(!g_eeGeneral.disableBG);//Default is zero=checked
    ui->beeperCB->setCurrentIndex(g_eeGeneral.beeperVal);

    ui->RotateScreenChkB->setChecked(g_eeGeneral.rotateScreen);
    ui->ReverseScreenChkB->setChecked(g_eeGeneral.reverseScreen);
    ui->OptrexDisplayChkB->setChecked(g_eeGeneral.optrexDisplay);
    if ( (rData->bitType & ( RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO )) == RADIO_BITTYPE_SKY )
		{
			ui->OptrexDisplayChkB->show() ;
		}
		else
		{
			ui->OptrexDisplayChkB->hide() ;
		}
    if ( rData->bitType & ( RADIO_BITTYPE_9XRPRO | RADIO_TYPE_9XTREME ) )
		{
			ui->ReverseScreenChkB->show() ;
		}
		else
		{
			ui->ReverseScreenChkB->hide() ;
		}
    if ( rData->bitType & ( RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO ) )
		{
			ui->RotateScreenChkB->show() ;
		}
		else
		{
			ui->RotateScreenChkB->hide() ;
		}

    ui->WelcomeCB->setCurrentIndex(g_eeGeneral.welcomeType);
    ui->welcomeFileNameLE->setText((char *)g_eeGeneral.welcomeFileName) ;
		
		ui->channelorderCB->setCurrentIndex(g_eeGeneral.templateSetup);
    ui->languageCB->setCurrentIndex(g_eeGeneral.language);
    ui->stickmodeCB->setCurrentIndex(g_eeGeneral.stickMode);
    if ( rData->bitType & ( RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_XXX ) )
		{
			g_eeGeneral.softwareVolume = 1 ;
    	ui->SoftwareVolumeChkB->hide() ;

		}
    ui->SoftwareVolumeChkB->setChecked(g_eeGeneral.softwareVolume ) ;
    ui->Ar9xChkB->setChecked(g_eeGeneral.ar9xBoard ) ;
    ui->MenuEditChkB->setChecked(g_eeGeneral.forceMenuEdit ) ;

		ui->ExtRtcCB->setCurrentIndex(g_eeGeneral.externalRtcType ) ;
    
		ui->volumeSB->setValue(g_eeGeneral.volume);
//    ui->enablePpmsimChkB->setChecked(g_eeGeneral.enablePpmsim);
    ui->internalFrskyAlarmChkB->setChecked(g_eeGeneral.frskyinternalalarm);
    ui->OptrexDisplayChkB->setChecked(g_eeGeneral.optrexDisplay);

    ui->beepMinuteChkB->setChecked(g_eeGeneral.minuteBeep);
    ui->beepCountDownChkB->setChecked(g_eeGeneral.preBeep);
    ui->beepFlashChkB->setChecked(g_eeGeneral.flashBeep);
    ui->splashScreenChkB->setChecked(!g_eeGeneral.disableSplashScreen);
    ui->splashScreenNameChkB->setChecked(!g_eeGeneral.hideNameOnSplash);
    ui->brightSB->setValue(100-g_eeGeneral.bright ) ;
    ui->brightGreenSB->setValue(100-g_eeGeneral.bright_white ) ;
    ui->brightBlueSB->setValue(100-g_eeGeneral.bright_blue ) ;

		if ( rData->type ==  RADIO_TYPE_SKY )
		{
			ui->Ar9xChkB->show() ;
		}

		switch ( rData->type )
		{
			case RADIO_TYPE_XXX :
				ui->BtDev1Address->hide() ;
				ui->BtDev2Address->hide() ;
				ui->BtDev3Address->hide() ;
				ui->BtDev4Address->hide() ;
				ui->BtDev1Name->hide() ;
				ui->BtDev2Name->hide() ;
				ui->BtDev3Name->hide() ;
				ui->BtDev4Name->hide() ;
				ui->label_BT_name->hide() ;
				ui->label_BTadd1->hide() ;
				ui->label_BTadd2->hide() ;
				ui->label_BTadd3->hide() ;
				ui->label_BTadd4->hide() ;
        ui->BtNameText->hide() ;
				ui->label_ExtRtc->hide() ;
				ui->label_Rotary_div->hide() ;
				ui->RotaryDivisorCB->hide() ;
				ui->stickgainLHCB->hide() ;
				ui->stickgainLVCB->hide() ;
				ui->stickgainRHCB->hide() ;
				ui->stickgainRVCB->hide() ;
				ui->label_StickGain->hide() ;
				ui->label_stickgainLH->hide() ;
				ui->label_stickgainLV->hide() ;
				ui->label_stickgainRH->hide() ;
				ui->label_stickgainRV->hide() ;
				ui->label_Encoder->hide() ;
				ui->EncoderCB->hide() ;
				ui->Pot2DetCB->hide() ;
			case RADIO_TYPE_TARANIS :
			case RADIO_TYPE_QX7 :
				ui->Ar9xChkB->hide() ;
			case RADIO_TYPE_SKY :
				ui->brightGreenSB->hide() ;
				ui->brightBlueSB->hide() ;
				ui->label_BrightGreen->hide() ;
				ui->label_BrightBlue->hide() ;
				ui->label_Bright->setText("Brightness") ;
			break ;
				
			case RADIO_TYPE_TPLUS :
			case RADIO_TYPE_X9E :
				ui->Ar9xChkB->hide() ;
				ui->label_Bright->setText("Brightness (Colour)") ;
				ui->label_BrightGreen->setText("Brightness (White)") ;
				ui->label_BrightGreen->show() ;
				ui->label_BrightBlue->hide() ;
				ui->brightGreenSB->show() ;
				ui->brightBlueSB->hide() ;
			break ;

			case RADIO_TYPE_9XTREME :
				ui->Ar9xChkB->hide() ;
				ui->brightGreenSB->show() ;
				ui->brightBlueSB->show() ;
				ui->label_BrightGreen->show() ;
				ui->label_BrightBlue->show() ;
				ui->label_Bright->setText("Brightness (Red)") ;
				ui->label_BrightGreen->setText("Brightness (Green)") ;
				ui->label_BrightBlue->setText("Brightness (Blue)") ;
			break ;

			case RADIO_TYPE_XLITE :
				ui->Ar9xChkB->hide() ;
				ui->brightGreenSB->hide() ;
				ui->brightBlueSB->hide() ;
				ui->label_BrightGreen->hide() ;
				ui->label_BrightBlue->hide() ;
				ui->label_Bright->setText("Brightness") ;
				ui->ExtRtcCB->hide() ;
				ui->label_ExtRtc->hide() ;
				ui->label_Rotary_div->hide() ;
				ui->RotaryDivisorCB->hide() ;
				ui->SixCal0_SB->hide() ;
				ui->SixCal1_SB->hide() ;
				ui->SixCal2_SB->hide() ;
				ui->SixCal3_SB->hide() ;
				ui->SixCal4_SB->hide() ;
				ui->SixCal5_SB->hide() ;
				ui->label_6Pos_2->hide() ;
				ui->label_6Pos_3->hide() ;
				ui->label_6Pos_4->hide() ;
				ui->label_6Pos_5->hide() ;
				ui->label_6Pos_6->hide() ;
				ui->label_6Pos_7->hide() ;
				ui->label_StickGain->hide() ;
				ui->stickgainLHCB->hide() ;
				ui->stickgainLVCB->hide() ;
				ui->stickgainRHCB->hide() ;
				ui->stickgainRVCB->hide() ;
				ui->label_stickgainLH->hide() ;
				ui->label_stickgainLV->hide() ;
				ui->label_stickgainRH->hide() ;
				ui->label_stickgainRV->hide() ;
			break ;

			case RADIO_TYPE_T12 :
				ui->Ar9xChkB->hide() ;
				ui->brightGreenSB->hide() ;
				ui->brightBlueSB->hide() ;
				ui->label_BrightGreen->hide() ;
				ui->label_BrightBlue->hide() ;
				ui->label_Bright->setText("Brightness") ;
				ui->ExtRtcCB->hide() ;
				ui->label_ExtRtc->hide() ;
				ui->label_Rotary_div->hide() ;
				ui->RotaryDivisorCB->hide() ;
				ui->SixCal0_SB->hide() ;
				ui->SixCal1_SB->hide() ;
				ui->SixCal2_SB->hide() ;
				ui->SixCal3_SB->hide() ;
				ui->SixCal4_SB->hide() ;
				ui->SixCal5_SB->hide() ;
				ui->label_6Pos_2->hide() ;
				ui->label_6Pos_3->hide() ;
				ui->label_6Pos_4->hide() ;
				ui->label_6Pos_5->hide() ;
				ui->label_6Pos_6->hide() ;
				ui->label_6Pos_7->hide() ;
				ui->label_StickGain->hide() ;
				ui->stickgainLHCB->hide() ;
				ui->stickgainLVCB->hide() ;
				ui->stickgainRHCB->hide() ;
				ui->stickgainRVCB->hide() ;
				ui->label_stickgainLH->hide() ;
				ui->label_stickgainLV->hide() ;
				ui->label_stickgainRH->hide() ;
				ui->label_stickgainRV->hide() ;
				ui->label_Encoder->hide() ;
				ui->label_6Pos->hide() ;
				ui->EncoderCB->hide() ;
				ui->SixPosCB->hide() ;
				ui->BtDev1Address->hide() ;
				ui->BtDev2Address->hide() ;
				ui->BtDev3Address->hide() ;
				ui->BtDev4Address->hide() ;
				ui->BtDev1Name->hide() ;
				ui->BtDev2Name->hide() ;
				ui->BtDev3Name->hide() ;
				ui->BtDev4Name->hide() ;
				ui->label_BT_name->hide() ;
				ui->label_BTadd1->hide() ;
				ui->label_BTadd2->hide() ;
				ui->label_BTadd3->hide() ;
				ui->label_BTadd4->hide() ;
        ui->BtNameText->hide() ;

			break ;



		}

		ui->BtBaudrateCB->setCurrentIndex(g_eeGeneral.bt_baudrate) ;
		ui->RotaryDivisorCB->setCurrentIndex(g_eeGeneral.rotaryDivisor) ;
    if ( rData->bitType & ( RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_XXX ) )
		{
			ui->MaHalarmSB->hide() ;
			ui->mAhLabel->hide() ;
			ui->CurrentCalibSB->hide() ;
			ui->CurrentCalibLabel->hide() ;
		}
		ui->CurrentCalibSB->setValue(g_eeGeneral.current_calib ) ;
		ui->MaHalarmSB->setValue(g_eeGeneral.mAh_alarm*50 ) ;
    ui->BluetoothTypeCB->setCurrentIndex(g_eeGeneral.BtType);
		
		ui->hapticMinRunSB->setValue( g_eeGeneral.hapticMinRun + 20 ) ;

		ui->SixCal0_SB->setValue( g_eeGeneral.SixPositionCalibration[0] ) ;
		ui->SixCal1_SB->setValue( g_eeGeneral.SixPositionCalibration[1] ) ;
		ui->SixCal2_SB->setValue( g_eeGeneral.SixPositionCalibration[2] ) ;
		ui->SixCal3_SB->setValue( g_eeGeneral.SixPositionCalibration[3] ) ;
		ui->SixCal4_SB->setValue( g_eeGeneral.SixPositionCalibration[4] ) ;
		ui->SixCal5_SB->setValue( g_eeGeneral.SixPositionCalibration[5] ) ;

    ui->ana1Neg->setValue(g_eeGeneral.calibSpanNeg[0]);
    ui->ana2Neg->setValue(g_eeGeneral.calibSpanNeg[1]);
    ui->ana3Neg->setValue(g_eeGeneral.calibSpanNeg[2]);
    ui->ana4Neg->setValue(g_eeGeneral.calibSpanNeg[3]);
    ui->ana5Neg->setValue(g_eeGeneral.calibSpanNeg[4]);
    ui->ana6Neg->setValue(g_eeGeneral.calibSpanNeg[5]);
    ui->ana7Neg->setValue(g_eeGeneral.calibSpanNeg[6]);
    ui->ana8Neg->setValue(g_eeGeneral.x9dcalibSpanNeg);

    ui->ana1Mid->setValue(g_eeGeneral.calibMid[0]);
    ui->ana2Mid->setValue(g_eeGeneral.calibMid[1]);
    ui->ana3Mid->setValue(g_eeGeneral.calibMid[2]);
    ui->ana4Mid->setValue(g_eeGeneral.calibMid[3]);
    ui->ana5Mid->setValue(g_eeGeneral.calibMid[4]);
    ui->ana6Mid->setValue(g_eeGeneral.calibMid[5]);
    ui->ana7Mid->setValue(g_eeGeneral.calibMid[6]);
    ui->ana8Mid->setValue(g_eeGeneral.x9dcalibMid);

    ui->ana1Pos->setValue(g_eeGeneral.calibSpanPos[0]);
    ui->ana2Pos->setValue(g_eeGeneral.calibSpanPos[1]);
    ui->ana3Pos->setValue(g_eeGeneral.calibSpanPos[2]);
    ui->ana4Pos->setValue(g_eeGeneral.calibSpanPos[3]);
    ui->ana5Pos->setValue(g_eeGeneral.calibSpanPos[4]);
    ui->ana6Pos->setValue(g_eeGeneral.calibSpanPos[5]);
    ui->ana7Pos->setValue(g_eeGeneral.calibSpanPos[6]);
    ui->ana8Pos->setValue(g_eeGeneral.x9dcalibSpanPos);

    if ( rData->bitType & ( RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_XXX ) )
		{
      if ( rData->bitType & RADIO_BITTYPE_XXX )
			{
				ui->ana6Neg->hide() ;
				ui->ana6Mid->hide() ;
				ui->ana6Pos->hide() ;
				ui->Ana6Label->hide() ;
			}
			ui->ana7Neg->hide() ;
			ui->ana8Neg->hide() ;
			ui->ana7Mid->hide() ;
			ui->ana8Mid->hide() ;	 
			ui->ana7Pos->hide() ;
			ui->ana8Pos->hide() ;
			ui->Ana7Label->hide() ;
			ui->Ana8Label->hide() ;
		}


//    setSwitchDefPos();
    
    ui->StickRevLH->setChecked(g_eeGeneral.stickReverse & 0x01);
    ui->StickRevLV->setChecked(g_eeGeneral.stickReverse & 0x02);
    ui->StickRevRV->setChecked(g_eeGeneral.stickReverse & 0x04);
    ui->StickRevRH->setChecked(g_eeGeneral.stickReverse & 0x08);

		ui->weightSB_1->findChild<QLineEdit*>()->setReadOnly(true);
    ui->weightSB_2->findChild<QLineEdit*>()->setReadOnly(true);
    ui->weightSB_3->findChild<QLineEdit*>()->setReadOnly(true);
    ui->weightSB_4->findChild<QLineEdit*>()->setReadOnly(true);

		ui->stickgainLVCB->setCurrentIndex(g_eeGeneral.stickGain & STICK_LV_GAIN ? 1 : 0 ) ;
		ui->stickgainLHCB->setCurrentIndex(g_eeGeneral.stickGain & STICK_LH_GAIN ? 1 : 0 ) ;
		ui->stickgainRVCB->setCurrentIndex(g_eeGeneral.stickGain & STICK_RV_GAIN ? 1 : 0 ) ;
		ui->stickgainRHCB->setCurrentIndex(g_eeGeneral.stickGain & STICK_RH_GAIN ? 1 : 0 ) ;

    ui->StickLHdeadbandSB->setValue( g_eeGeneral.stickDeadband[0] ) ;
    ui->StickLVdeadbandSB->setValue( g_eeGeneral.stickDeadband[1] ) ;
    ui->StickRVdeadbandSB->setValue( g_eeGeneral.stickDeadband[2] ) ;
    ui->StickRHdeadbandSB->setValue( g_eeGeneral.stickDeadband[3] ) ;

		CurrentTrainerProfile = g_eeGeneral.CurrentTrainerProfile ;
		loadTrainerFromProfile() ;
		updateTrainerTab();


		if ( ( rData->type == RADIO_TYPE_SKY ) || ( rData->type == RADIO_TYPE_9XTREME ) )
		{
			ui->label_AilSw->show() ;
			ui->label_EleSw->show() ;
			ui->label_ThrSw->show() ;
			ui->label_GeaSw->show() ;
			ui->label_RudSw->show() ;
			ui->label_PB1Sw->show() ;
			ui->label_PB2Sw->show() ;
			ui->label_PB3Sw->show() ;
			ui->label_PB4Sw->show() ;
			ui->label_Pot4->show() ;
			ui->Pot4CB->show() ;

      int xtype = 0 ;
			if ( rData->type == RADIO_TYPE_9XTREME )
			{
				xtype = 2 ;
				ui->label_Pot5->show() ;
				ui->Pot5CB->show() ;
			}
			else
			{
				ui->label_Pot5->hide() ;
				ui->Pot5CB->hide() ;
				if ( rData->T9xr_pro )
				{
					xtype = 1 ;
				}
			}
			setHardwareSwitchCB( ui->AilCB, 1, xtype ) ;
			setHardwareSwitchCB( ui->EleCB, 0, xtype ) ;
			setHardwareSwitchCB( ui->GeaCB, 1, xtype ) ;
			setHardwareSwitchCB( ui->RudCB, 1, xtype ) ;
			setHardwareSwitchCB( ui->ThrCB, 1, xtype ) ;
			setHardwareSwitchCB( ui->PB1CB, 1, xtype ) ;
			setHardwareSwitchCB( ui->PB2CB, 1, xtype ) ;
			setHardwareSwitchCB( ui->PB3CB, 1, xtype ) ;
			setHardwareSwitchCB( ui->PB4CB, 1, xtype ) ;
			setHardwarePotCB( ui->Pot4CB, xtype ) ;
			setHardwarePotCB( ui->Pot5CB, xtype ) ;

      ui->AilCB->show() ;
      ui->EleCB->show() ;
      ui->GeaCB->show() ;
      ui->RudCB->show() ;
			ui->ThrCB->show() ;
			ui->PB1CB->show() ;
			ui->PB2CB->show() ;
			ui->PB3CB->show() ;
			ui->PB4CB->show() ;
			ui->label_Encoder->hide() ;
//			ui->label_6Pos->hide() ;
			ui->EncoderCB->hide() ;
//			ui->SixPosCB->hide() ;
			ui->AilCB->setCurrentIndex( g_eeGeneral.ailsource ) ;
			uint8_t value = g_eeGeneral.elesource ;
			if ( rData->type != RADIO_TYPE_9XTREME )
			{
				if ( value > 5 )
				{
					value += 2 ;
				}
				if ( value >101 )
				{
					value -= 102-6 ;
				}
			}
    	ui->EleCB->setCurrentIndex( value ) ;
			ui->GeaCB->setCurrentIndex( g_eeGeneral.geasource ) ;
			ui->RudCB->setCurrentIndex( g_eeGeneral.rudsource ) ;
			ui->ThrCB->setCurrentIndex( g_eeGeneral.thrsource ) ;
			ui->PB1CB->setCurrentIndex( g_eeGeneral.pb1source ) ;
			ui->PB2CB->setCurrentIndex( g_eeGeneral.pb2source ) ;
			ui->PB3CB->setCurrentIndex( g_eeGeneral.pb3source ) ;
			ui->PB4CB->setCurrentIndex( g_eeGeneral.pb4source ) ;
			ui->Pot4CB->setCurrentIndex( g_eeGeneral.extraPotsSource[0] ) ;
			ui->Pot5CB->setCurrentIndex( g_eeGeneral.extraPotsSource[1] ) ;
		}
		else
		{
			if ( rData->type == RADIO_TYPE_QX7 )
			{
				ui->PB1CB->show() ;
				ui->PB2CB->show() ;
				ui->label_PB1Sw->show() ;
				ui->label_PB2Sw->show() ;
				setHardwareSwitchCB( ui->PB1CB, 1, 3 ) ;
				setHardwareSwitchCB( ui->PB2CB, 1, 3 ) ;
				ui->PB1CB->setCurrentIndex( g_eeGeneral.pb1source ) ;
				ui->PB2CB->setCurrentIndex( g_eeGeneral.pb2source ) ;
			}
			else
			{
				if ( rData->type == RADIO_TYPE_T12 )
				{
					ui->label_PB1Sw->hide() ;
					ui->label_PB2Sw->hide() ;
					ui->PB1CB->hide() ;
					ui->PB2CB->hide() ;
				}
				else
				{
					ui->label_PB1Sw->show() ;
					ui->label_PB2Sw->show() ;
					setHardwareSwitchCB( ui->PB1CB, 1, 3 ) ;
					setHardwareSwitchCB( ui->PB2CB, 1, 3 ) ;
					ui->PB1CB->setCurrentIndex( g_eeGeneral.pb1source ) ;
					ui->PB2CB->setCurrentIndex( g_eeGeneral.pb2source ) ;
				}
			}

			if ( rData->type == RADIO_TYPE_XLITE )
			{
      	ui->AilCB->clear() ;
				ui->AilCB->addItem("2-pos");
				ui->AilCB->addItem("3-pos");
    		ui->AilCB->setCurrentIndex(g_eeGeneral.ailsource) ;
      	ui->RudCB->clear() ;
				ui->RudCB->addItem("2-pos");
				ui->RudCB->addItem("3-pos");
    		ui->RudCB->setCurrentIndex(g_eeGeneral.rudsource) ;
				ui->label_AilSw->show() ;
				ui->label_RudSw->show() ;
      	ui->AilCB->show() ;
      	ui->RudCB->show() ;
				ui->label_Encoder->hide() ;
				ui->EncoderCB->hide() ;
				ui->label_6Pos->hide() ;
				ui->SixPosCB->hide() ;
				ui->label_AilSw->setText("Switch C") ;
				ui->label_RudSw->setText("Switch D") ;
			}
			else
			{
				ui->label_AilSw->hide() ;
				ui->label_RudSw->hide() ;
      	ui->AilCB->hide() ;
     		ui->RudCB->hide() ;
				if ( ( rData->type != RADIO_TYPE_T12 ) && ( rData->type != RADIO_TYPE_XXX ) )
				{
					ui->label_Encoder->show() ;
					ui->EncoderCB->show() ;
				}
			}
			if ( rData->type == RADIO_TYPE_XXX )
			{
				ui->SixPosCB->clear() ;
				ui->SixPosCB->addItem("--") ;
				ui->SixPosCB->addItem("P1") ;
			}

			ui->label_ThrSw->hide() ;
			ui->label_EleSw->hide() ;
			ui->label_GeaSw->hide() ;
			ui->label_PB3Sw->hide() ;
			ui->label_PB4Sw->hide() ;
			ui->label_Pot4->hide() ;
			ui->label_Pot5->hide() ;
     	ui->GeaCB->hide() ;
      ui->EleCB->hide() ;
			ui->ThrCB->hide() ;
			ui->PB3CB->hide() ;
			ui->PB4CB->hide() ;
			ui->Pot4CB->hide() ;
			ui->Pot5CB->hide() ;
	//			ui->label_6Pos->show() ;
	//			ui->SixPosCB->show() ;
			ui->EncoderCB->setCurrentIndex(g_eeGeneral.analogMapping & 3) ;
				//g_eeGeneral.analogMapping
		}
		ui->SixPosCB->setCurrentIndex((g_eeGeneral.analogMapping >> 2) & 7) ;
		
		setHwSwitchActive();

    connect(ui->modeCB_1, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));
    connect(ui->modeCB_2, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));
    connect(ui->modeCB_3, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));
    connect(ui->modeCB_4, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));

    connect(ui->weightSB_1, SIGNAL(editingFinished()), this, SLOT(trainerTabValueChanged()));
    connect(ui->weightSB_2, SIGNAL(editingFinished()), this, SLOT(trainerTabValueChanged()));
    connect(ui->weightSB_3, SIGNAL(editingFinished()), this, SLOT(trainerTabValueChanged()));
    connect(ui->weightSB_4, SIGNAL(editingFinished()), this, SLOT(trainerTabValueChanged()));

    connect(ui->sourceCB_1, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));
    connect(ui->sourceCB_2, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));
    connect(ui->sourceCB_3, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));
    connect(ui->sourceCB_4, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));

    connect(ui->swtchCB_1, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));
    connect(ui->swtchCB_2, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));
    connect(ui->swtchCB_3, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));
    connect(ui->swtchCB_4, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));

    connect(ui->trainerCalib_1, SIGNAL(editingFinished()), this, SLOT(trainerTabValueChanged()));
    connect(ui->trainerCalib_2, SIGNAL(editingFinished()), this, SLOT(trainerTabValueChanged()));
    connect(ui->trainerCalib_3, SIGNAL(editingFinished()), this, SLOT(trainerTabValueChanged()));
    connect(ui->trainerCalib_4, SIGNAL(editingFinished()), this, SLOT(trainerTabValueChanged()));

    connect(ui->PPM_MultiplierDSB, SIGNAL(editingFinished()), this, SLOT(trainerTabValueChanged()));
    connect(ui->TrainerSourceCB, SIGNAL(currentIndexChanged(int)), this, SLOT(trainerTabValueChanged()));
    connect(ui->InvertChkB, SIGNAL(stateChanged(int)), this, SLOT(trainerTabValueChanged()));

    connect(ui->weightSB_1, SIGNAL(valueChanged(int)), this, SLOT(validateWeightSB()));
    connect(ui->weightSB_2, SIGNAL(valueChanged(int)), this, SLOT(validateWeightSB()));
    connect(ui->weightSB_3, SIGNAL(valueChanged(int)), this, SLOT(validateWeightSB()));
    connect(ui->weightSB_4, SIGNAL(valueChanged(int)), this, SLOT(validateWeightSB()));

    ui->Pot1DetCB->setChecked( g_eeGeneral.potDetents & 1 ) ;
    ui->Pot2DetCB->setChecked( g_eeGeneral.potDetents & 2 ) ;
    ui->Pot3DetCB->setChecked( g_eeGeneral.potDetents & 4 ) ;
    ui->Pot4DetCB->setChecked( g_eeGeneral.potDetents & 8 ) ;
    ui->Pot5DetCB->setChecked( g_eeGeneral.potDetents & 16 ) ;

		uint32_t numberOfPots = 2 ;

		if ( rData->bitType & ( RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO | RADIO_TYPE_9XTREME | RADIO_BITTYPE_AR9X ) )
		{
			numberOfPots = 3 ;
		}
		if ( rData->bitType & ( RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E ) )
		{
			numberOfPots = 4 ;
		}
		numberOfPots += rData->extraPots ;

		ui->Pot5DetCB->setVisible( numberOfPots > 4 ) ;
		ui->Pot4DetCB->setVisible( numberOfPots > 3 ) ;
		ui->Pot3DetCB->setVisible( numberOfPots > 2 ) ;

		hardwareTabLock = 0 ;
}

GeneralEdit::~GeneralEdit()
{
    delete ui;
}

//void GeneralEdit::setSwitchDefPos()
//{
//    quint8 x = g_eeGeneral.switchWarningStates & SWP_IL5;
//    if(x==SWP_IL1 || x==SWP_IL2 || x==SWP_IL3 || x==SWP_IL4 || x==SWP_IL5) //illegal states for ID0/1/2
//    {
//        g_eeGeneral.switchWarningStates &= ~SWP_IL5; // turn all off, make sure only one is on
//        g_eeGeneral.switchWarningStates |=  SWP_ID0B;
//    }

//    switchDefPosEditLock = true;
//    ui->switchDefPos_1->setChecked(g_eeGeneral.switchWarningStates & 0x01);
//    ui->switchDefPos_2->setChecked(g_eeGeneral.switchWarningStates & 0x02);
//    ui->switchDefPos_3->setChecked(g_eeGeneral.switchWarningStates & 0x04);
//    ui->switchDefPos_4->setChecked(g_eeGeneral.switchWarningStates & 0x08);
//    ui->switchDefPos_5->setChecked(g_eeGeneral.switchWarningStates & 0x10);
//    ui->switchDefPos_6->setChecked(g_eeGeneral.switchWarningStates & 0x20);
//    ui->switchDefPos_7->setChecked(g_eeGeneral.switchWarningStates & 0x40);
//    ui->switchDefPos_8->setChecked(g_eeGeneral.switchWarningStates & 0x80);
//    switchDefPosEditLock = false;
//}


// type = 0 for SKY, 1 for PRO, 2 for 9Xtreme
void GeneralEdit::setHardwarePotCB( QComboBox *b, int type )
{
  b->clear() ;
	switch ( type )
	{
		case 0 :
		case 1 :
			b->addItem( "NONE" ) ;
			b->addItem( "AD10" ) ;
			if ( g_eeGeneral.ar9xBoard )
			{
				b->addItem( "AD8" ) ;
			}
		break ;
		case 2 :
			b->addItem( "NONE" ) ;
			b->addItem( "EXT1" ) ;
			b->addItem( "EXT2" ) ;
			b->addItem( "EXT3" ) ;
			b->addItem( "EXT4" ) ;
		break ;
	}
}

// switchList = 0 for ELE, 1 for others
// type = 0 for SKY, 1 for PRO, 2 for 9Xtreme
void GeneralEdit::setHardwareSwitchCB( QComboBox *b, int switchList, int type )
{
  b->clear() ;
	if ( switchList )
	{
		switch ( type )
		{
			case 0 :
				b->addItem( "NONE" ) ;
				b->addItem( "LCD2" ) ;
				b->addItem( "LCD6" ) ;
				b->addItem( "LCD7" ) ;
				b->addItem( "DAC1" ) ;
				b->addItem( "LCD4" ) ;
				b->addItem( "ELE " ) ;
				b->addItem( "EXT4" ) ;
				b->addItem( "EXT5" ) ;
				b->addItem( "EXT6" ) ;
			break ;
			case 1 :
				b->addItem( "NONE" ) ;
				b->addItem( "LCD2" ) ;
				b->addItem( "LCD6" ) ;
				b->addItem( "LCD7" ) ;
				b->addItem( "DAC1" ) ;
				b->addItem( "ELE " ) ;
			break ;
			case 2 :
				b->addItem( "NONE" ) ;
				b->addItem( "EXT1" ) ;
				b->addItem( "EXT2" ) ;
				b->addItem( "EXT3" ) ;
				b->addItem( "EXT4" ) ;
				b->addItem( "EXT5" ) ;
				b->addItem( "EXT6" ) ;
				b->addItem( "EXT7" ) ;
				b->addItem( "EXT8" ) ;
			break ;
			case 3 :
				b->addItem( "NONE" ) ;
				b->addItem( "JTMS" ) ;
				b->addItem( "JTCK" ) ;
				if ( rData->type !=  RADIO_TYPE_XLITE )
				{
					b->addItem( "EXT1" ) ;
					b->addItem( "EXT2" ) ;
				}
			break ;
		}
	}
	else
	{
		switch ( type )
		{
			case 0 :
				b->addItem( "NONE" ) ;
				b->addItem( "LCD2" ) ;
				b->addItem( "LCD6" ) ;
				b->addItem( "LCD7" ) ;
				b->addItem( "DAC1" ) ;
				b->addItem( "ANA " ) ;
				b->addItem( "6PSA" ) ;
				b->addItem( "6PSB" ) ;
				b->addItem( "EXT4" ) ;
				b->addItem( "EXT5" ) ;
				b->addItem( "EXT6" ) ;
			break ;
			case 1 :
				b->addItem( "NONE" ) ;
				b->addItem( "LCD2" ) ;
				b->addItem( "LCD6" ) ;
				b->addItem( "LCD7" ) ;
				b->addItem( "DAC1" ) ;
				b->addItem( "ANA " ) ;
				b->addItem( "6PSA" ) ;
				b->addItem( "6PSB" ) ;
			break ;
			case 2 :
				b->addItem( "NONE" ) ;
				b->addItem( "EXT1" ) ;
				b->addItem( "EXT2" ) ;
				b->addItem( "EXT3" ) ;
				b->addItem( "EXT4" ) ;
				b->addItem( "EXT5" ) ;
				b->addItem( "EXT6" ) ;
				b->addItem( "EXT7" ) ;
				b->addItem( "EXT8" ) ;
			break ;
		}
	}


//	value = checkIndexed( y, XPSTR(FWx17"\005\004NONELCD2LCD6LCD7DAC1ELE "), value, condition ) ;
//#else
//#ifdef PCB9XT
//	value = checkIndexed( y, XPSTR(FWx17"\011\004NONEEXT1EXT2EXT3EXT4ELE EXT5EXT6EXT7EXT8"), value, condition ) ;
//#else
//	value = checkIndexed( y, XPSTR(FWx17"\010\004NONELCD2LCD6LCD7DAC1ELE EXT4EXT5EXT6"), value, condition ) ;

//#ifdef REVX
//				value = checkIndexed( y, XPSTR(FWx17"\007\004NONELCD2LCD6LCD7DAC1ANA 6PSA6PSB"), value, (sub==subN) ) ;
//#else
//#ifdef PCB9XT
//				value = checkIndexed( y, XPSTR(FWx17"\012\004NONEEXT1EXT2EXT3EXT4EXT56PSA6PSBEXT6EXT7EXT8"), value, (sub==subN) ) ;
//#else
//				value = checkIndexed( y, XPSTR(FWx17"\012\004NONELCD2LCD6LCD7DAC1ANA 6PSA6PSBEXT4EXT5EXT6"), value, (sub==subN) ) ;
}			
			



void GeneralEdit::updateSettings()
{
    int16_t sum=0;
    for(int i=0; i<12;i++) sum+=g_eeGeneral.calibMid[i];
    g_eeGeneral.chkSum = sum;
//    eeFile->putGeneralSettings(&g_eeGeneral);
    memcpy( &rData->generalSettings, &g_eeGeneral, sizeof( g_eeGeneral) ) ;


    emit modelValuesChanged();

		memcpy(&Sim_g, &g_eeGeneral,sizeof(EEGeneral));
		GeneralDataValid = 1 ;
		ModelDataValid = 0 ;
		GlobalModified = 1 ;
}

uint32_t GeneralEdit::btAddressValid( uint8_t *address )
{
	uint8_t x ;
	x = *address++ ;
	x |= *address++ ;
	x |= *address++ ;
	x |= *address++ ;
	x |= *address++ ;
	x |= *address ;
	return x ;
}

uint8_t GeneralEdit::b2hex( uint8_t c )
{
	c &= 0x0F ;
	if ( c > 9 )
	{
		c += 7 ;
	}
	c += '0' ;
	return c ;
}

uint8_t *GeneralEdit::btAddrBin2Hex( uint8_t *dest, uint8_t *source )
{
	uint8_t c ;
	
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	*dest++ = ',' ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	*dest++ = ',' ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	c = *source++ ;
	*dest++ = b2hex( c >> 4 ) ;
	*dest++ = b2hex( c ) ;
	*dest = '\0' ;
	return dest ;
}

void GeneralEdit::saveTrainerToProfile()
{
	uint32_t i ;
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		TrainerMix *td = &g_eeGeneral.trainer.mix[i];
		exTrainerMix *xtd = &g_eeGeneral.exTrainer[i] ;
		TrainerChannel *tc = &g_eeGeneral.trainerProfile[CurrentTrainerProfile].channel[i] ;
		tc->calib = g_eeGeneral.trainer.calib[i] ;
		tc->srcChn = td->srcChn ;
		tc->mode = td->mode ;
		tc->swtch = xtd->swtch ;
		tc->studWeight = xtd->studWeight ;
//		if ( i == 0 )
//		{
//			tc->source = g_eeGeneral.trainerSource ;
//		}
	}
}

void GeneralEdit::loadTrainerFromProfile()
{
	uint32_t i ;
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		TrainerMix *td = &g_eeGeneral.trainer.mix[i];
		exTrainerMix *xtd = &g_eeGeneral.exTrainer[i] ;
		TrainerChannel *tc = &g_eeGeneral.trainerProfile[CurrentTrainerProfile].channel[i] ;
		g_eeGeneral.trainer.calib[i] = tc->calib ;
		td->srcChn = tc->srcChn ;
		td->mode = tc->mode ;
		xtd->swtch = tc->swtch ;
		xtd->studWeight = tc->studWeight ;
//		if ( i == 0 )
//		{
//			g_eeGeneral.trainerSource = tc->source ;
//		}
	}
}

void GeneralEdit::on_TrainerProfileSB_valueChanged( int x )
{
	trainerTabLock = 1 ;
	
	saveTrainerToProfile() ;
	g_eeGeneral.CurrentTrainerProfile = ui->TrainerProfileSB->value() ;
	CurrentTrainerProfile = g_eeGeneral.CurrentTrainerProfile ;
	loadTrainerFromProfile() ;
	trainerTabLock = 0 ;
  updateTrainerTab() ;
  updateSettings() ;
}

void GeneralEdit::updateTrainerTab()
{
	trainerTabLock = 1 ;
    on_tabWidget_selected(""); // updates channel name labels

		CurrentTrainerProfile = g_eeGeneral.CurrentTrainerProfile ;
		ui->TrainerProfileSB->setValue( g_eeGeneral.CurrentTrainerProfile ) ;
		ui->TrainerSourceCB->setCurrentIndex( g_eeGeneral.trainerProfile[CurrentTrainerProfile].channel[0].source ) ;
		ui->InvertChkB->setChecked( g_eeGeneral.trainerProfile[CurrentTrainerProfile].channel[1].source ) ;
    ui->modeCB_1->setCurrentIndex(g_eeGeneral.trainer.mix[0].mode);
    ui->sourceCB_1->setCurrentIndex(g_eeGeneral.trainer.mix[0].srcChn);
    if ( g_eeGeneral.trainer.mix[0].swtch == -16)
		{
			populateSwitchCB(ui->swtchCB_1,g_eeGeneral.exTrainer[0].swtch, rData->type ) ;
			ui->weightSB_1->setMaximum(100) ;
			ui->weightSB_1->setMinimum(-100) ;
			ui->weightSB_1->setSingleStep(1) ;
			ui->weightSB_1->findChild<QLineEdit*>()->setReadOnly(false);
  	  ui->weightSB_1->setValue(g_eeGeneral.exTrainer[0].studWeight);
		}
		else
		{
	    populateTrainerSwitchCB(ui->swtchCB_1,g_eeGeneral.trainer.mix[0].swtch);
  	  ui->weightSB_1->setValue(g_eeGeneral.trainer.mix[0].studWeight*13/4);
    	StudWeight1=g_eeGeneral.trainer.mix[0].studWeight*13/4;
		}

    ui->modeCB_2->setCurrentIndex(g_eeGeneral.trainer.mix[1].mode);
    ui->sourceCB_2->setCurrentIndex(g_eeGeneral.trainer.mix[1].srcChn);
    if ( g_eeGeneral.trainer.mix[1].swtch == -16)
		{
			populateSwitchCB(ui->swtchCB_2,g_eeGeneral.exTrainer[1].swtch, rData->type ) ;
			ui->weightSB_2->setMaximum(100) ;
			ui->weightSB_2->setMinimum(-100) ;
			ui->weightSB_2->setSingleStep(1) ;
			ui->weightSB_2->findChild<QLineEdit*>()->setReadOnly(false);
  	  ui->weightSB_2->setValue(g_eeGeneral.exTrainer[1].studWeight);
		}
		else
		{
	    populateTrainerSwitchCB(ui->swtchCB_2,g_eeGeneral.trainer.mix[1].swtch);
  	  ui->weightSB_2->setValue(g_eeGeneral.trainer.mix[1].studWeight*13/4);
    	StudWeight2=g_eeGeneral.trainer.mix[1].studWeight*13/4;
		}

    ui->modeCB_3->setCurrentIndex(g_eeGeneral.trainer.mix[2].mode);
    ui->sourceCB_3->setCurrentIndex(g_eeGeneral.trainer.mix[2].srcChn);
    if ( g_eeGeneral.trainer.mix[2].swtch == -16)
		{
			populateSwitchCB(ui->swtchCB_3,g_eeGeneral.exTrainer[2].swtch, rData->type ) ;
			ui->weightSB_3->setMaximum(100) ;
			ui->weightSB_3->setMinimum(-100) ;
			ui->weightSB_3->setSingleStep(1) ;
			ui->weightSB_3->findChild<QLineEdit*>()->setReadOnly(false);
  	  ui->weightSB_3->setValue(g_eeGeneral.exTrainer[2].studWeight);
		}
		else
		{
	    populateTrainerSwitchCB(ui->swtchCB_3,g_eeGeneral.trainer.mix[2].swtch);
  	  ui->weightSB_3->setValue(g_eeGeneral.trainer.mix[2].studWeight*13/4);
    	StudWeight3=g_eeGeneral.trainer.mix[2].studWeight*13/4;
		}

    ui->modeCB_4->setCurrentIndex(g_eeGeneral.trainer.mix[3].mode);
    ui->sourceCB_4->setCurrentIndex(g_eeGeneral.trainer.mix[3].srcChn);
    if ( g_eeGeneral.trainer.mix[3].swtch == -16)
		{
			populateSwitchCB(ui->swtchCB_4,g_eeGeneral.exTrainer[3].swtch, rData->type ) ;
			ui->weightSB_4->setMaximum(100) ;
			ui->weightSB_4->setMinimum(-100) ;
			ui->weightSB_4->setSingleStep(1) ;
			ui->weightSB_4->findChild<QLineEdit*>()->setReadOnly(false);
  	  ui->weightSB_4->setValue(g_eeGeneral.exTrainer[3].studWeight);
		}
		else
		{
    	populateTrainerSwitchCB(ui->swtchCB_4,g_eeGeneral.trainer.mix[3].swtch);
    	ui->weightSB_4->setValue(g_eeGeneral.trainer.mix[3].studWeight*13/4);
    	StudWeight4=g_eeGeneral.trainer.mix[3].studWeight*13/4;
		}

    ui->trainerCalib_1->setValue(g_eeGeneral.trainer.calib[0]);
    ui->trainerCalib_2->setValue(g_eeGeneral.trainer.calib[1]);
    ui->trainerCalib_3->setValue(g_eeGeneral.trainer.calib[2]);
    ui->trainerCalib_4->setValue(g_eeGeneral.trainer.calib[3]);

    ui->PPM_MultiplierDSB->setValue(double(g_eeGeneral.PPM_Multiplier+10)/10);
	trainerTabLock = 0 ;
}

void GeneralEdit::trainerTabValueChanged()
{
	if ( trainerTabLock )
	{
		return ;
	}
    g_eeGeneral.trainer.mix[0].mode       = ui->modeCB_1->currentIndex();
//    g_eeGeneral.trainer.mix[0].studWeight = ui->weightSB_1->value()*4/13;
    g_eeGeneral.trainer.mix[0].srcChn     = ui->sourceCB_1->currentIndex();
    if ( g_eeGeneral.trainer.mix[0].swtch == -16)
		{
    	g_eeGeneral.exTrainer[0].swtch      = getSwitchCbValue( ui->swtchCB_1, rData->type ) ;
		}
		else
		{
    	g_eeGeneral.trainer.mix[0].swtch      = ui->swtchCB_1->currentIndex()-15 ;
		}

    g_eeGeneral.trainer.mix[1].mode       = ui->modeCB_2->currentIndex();
//    g_eeGeneral.trainer.mix[1].studWeight = ui->weightSB_2->value()*4/13;
    g_eeGeneral.trainer.mix[1].srcChn     = ui->sourceCB_2->currentIndex();
    if ( g_eeGeneral.trainer.mix[1].swtch == -16)
		{
    	g_eeGeneral.exTrainer[1].swtch      = getSwitchCbValue( ui->swtchCB_2, rData->type ) ;
		}
		else
		{
	    g_eeGeneral.trainer.mix[1].swtch      = ui->swtchCB_2->currentIndex()-15 ;
		}

    g_eeGeneral.trainer.mix[2].mode       = ui->modeCB_3->currentIndex();
//    g_eeGeneral.trainer.mix[2].studWeight = ui->weightSB_3->value()*4/13;
    g_eeGeneral.trainer.mix[2].srcChn     = ui->sourceCB_3->currentIndex();
    if ( g_eeGeneral.trainer.mix[2].swtch == -16)
		{
    	g_eeGeneral.exTrainer[2].swtch      = getSwitchCbValue( ui->swtchCB_3, rData->type ) ;
		}
		else
		{
    	g_eeGeneral.trainer.mix[2].swtch      = ui->swtchCB_3->currentIndex()-15 ;
		}

    g_eeGeneral.trainer.mix[3].mode       = ui->modeCB_4->currentIndex();
//    g_eeGeneral.trainer.mix[3].studWeight = ui->weightSB_4->value()*4/13;
    g_eeGeneral.trainer.mix[3].srcChn     = ui->sourceCB_4->currentIndex();
    if ( g_eeGeneral.trainer.mix[3].swtch == -16)
		{
    	g_eeGeneral.exTrainer[3].swtch      = getSwitchCbValue( ui->swtchCB_4, rData->type ) ;
		}
		else
		{
    	g_eeGeneral.trainer.mix[3].swtch      = ui->swtchCB_4->currentIndex()-15 ;
		}

    g_eeGeneral.trainer.calib[0] = ui->trainerCalib_1->value();
    g_eeGeneral.trainer.calib[1] = ui->trainerCalib_2->value();
    g_eeGeneral.trainer.calib[2] = ui->trainerCalib_3->value();
    g_eeGeneral.trainer.calib[3] = ui->trainerCalib_4->value();

    g_eeGeneral.PPM_Multiplier = ((quint16)(ui->PPM_MultiplierDSB->value()*10))-10;
		
		g_eeGeneral.trainerProfile[CurrentTrainerProfile].channel[0].source = ui->TrainerSourceCB->currentIndex() ;
		g_eeGeneral.trainerProfile[CurrentTrainerProfile].channel[1].source = ui->InvertChkB->isChecked() ;

		saveTrainerToProfile() ;

    updateSettings();
}

void GeneralEdit::validateWeightSB()
{
	if ( trainerTabLock )
	{
		return ;
	}
    ui->weightSB_1->blockSignals(true);
    ui->weightSB_2->blockSignals(true);
    ui->weightSB_3->blockSignals(true);
    ui->weightSB_4->blockSignals(true);

    if ( g_eeGeneral.trainer.mix[0].swtch != -16)
		{
    	if ((ui->weightSB_1->value()>StudWeight1) && (g_eeGeneral.trainer.mix[0].studWeight<31))
    	{
    	  g_eeGeneral.trainer.mix[0].studWeight++;
    	}
    	else if ((ui->weightSB_1->value()<StudWeight1) && (g_eeGeneral.trainer.mix[0].studWeight>-31))
    	{
    	  g_eeGeneral.trainer.mix[0].studWeight--;
    	}
    	ui->weightSB_1->setValue(g_eeGeneral.trainer.mix[0].studWeight*13/4);
    	StudWeight1=ui->weightSB_1->value();
    }
		else
		{
   	  g_eeGeneral.exTrainer[0].studWeight = ui->weightSB_1->value() ;
		}

    if ( g_eeGeneral.trainer.mix[1].swtch != -16)
		{
    	if ((ui->weightSB_2->value()>StudWeight2) && (g_eeGeneral.trainer.mix[1].studWeight<31))
			{
    	  g_eeGeneral.trainer.mix[1].studWeight++;
    	}
			else if ((ui->weightSB_2->value()<StudWeight2) && (g_eeGeneral.trainer.mix[1].studWeight>-31))
			{
    	  g_eeGeneral.trainer.mix[1].studWeight--;
    	}
    	ui->weightSB_2->setValue(g_eeGeneral.trainer.mix[1].studWeight*13/4);
    	StudWeight2=ui->weightSB_2->value();
    }
		else
		{
   	  g_eeGeneral.exTrainer[1].studWeight = ui->weightSB_2->value() ;
		}
 
    if ( g_eeGeneral.trainer.mix[2].swtch != -16)
		{
    	if ((ui->weightSB_3->value()>StudWeight3) && (g_eeGeneral.trainer.mix[2].studWeight<31))
			{
    	  g_eeGeneral.trainer.mix[2].studWeight++;
    	}
			else if ((ui->weightSB_3->value()<StudWeight3) && (g_eeGeneral.trainer.mix[2].studWeight>-31))
			{
    	  g_eeGeneral.trainer.mix[2].studWeight--;
    	}
    	ui->weightSB_3->setValue(g_eeGeneral.trainer.mix[2].studWeight*13/4);
    	StudWeight3=ui->weightSB_3->value();
    }
		else
		{
   	  g_eeGeneral.exTrainer[2].studWeight = ui->weightSB_3->value() ;
		}

    if ( g_eeGeneral.trainer.mix[3].swtch != -16)
		{
    	if ((ui->weightSB_4->value()>StudWeight4) && (g_eeGeneral.trainer.mix[3].studWeight<31))
			{
    	  g_eeGeneral.trainer.mix[3].studWeight++;
    	}
			else if ((ui->weightSB_4->value()<StudWeight4)  && (g_eeGeneral.trainer.mix[3].studWeight>-31))
			{
    	  g_eeGeneral.trainer.mix[3].studWeight--;
    	}
    	ui->weightSB_4->setValue(g_eeGeneral.trainer.mix[3].studWeight*13/4);
    	StudWeight4=ui->weightSB_4->value();    
    }
		else
		{
   	  g_eeGeneral.exTrainer[3].studWeight = ui->weightSB_4->value() ;
		}
    
		ui->weightSB_1->blockSignals(false);
    ui->weightSB_2->blockSignals(false);
    ui->weightSB_3->blockSignals(false);
    ui->weightSB_4->blockSignals(false);
}

void GeneralEdit::on_contrastSB_editingFinished()
{
    g_eeGeneral.contrast = ui->contrastSB->value();
    updateSettings();
}

void GeneralEdit::on_volumeSB_editingFinished()
{
    g_eeGeneral.volume = ui->volumeSB->value() ;
    updateSettings();
}

void GeneralEdit::on_CurrentCalibSB_editingFinished()
{
    g_eeGeneral.current_calib = ui->CurrentCalibSB->value() ;
    updateSettings();
}

void GeneralEdit::on_MaHalarmSB_editingFinished()
{
    g_eeGeneral.mAh_alarm = ui->MaHalarmSB->value()/50 ;
    updateSettings();
}

void GeneralEdit::on_brightSB_editingFinished()
{
    g_eeGeneral.bright = 100-ui->brightSB->value() ;
    updateSettings();
}

void GeneralEdit::on_brightGreenSB_editingFinished()
{
    g_eeGeneral.bright_white = 100-ui->brightGreenSB->value() ;
    updateSettings();
}

void GeneralEdit::on_brightBlueSB_editingFinished()
{
    g_eeGeneral.bright_blue = 100-ui->brightBlueSB->value() ;
    updateSettings();
}

void GeneralEdit::do_stick_gain()
{
	uint8_t value = 0 ;	
	
	if ( ui->stickgainLVCB->currentIndex() )
	{
		value |= STICK_LV_GAIN ;
	}
	if ( ui->stickgainLHCB->currentIndex() )
	{
		value |= STICK_LH_GAIN ;
	}
	if ( ui->stickgainRVCB->currentIndex() )
	{
		value |= STICK_RV_GAIN ;
	}
	if ( ui->stickgainRHCB->currentIndex() )
	{
		value |= STICK_RH_GAIN ;
	}
	
	g_eeGeneral.stickGain = value ;
}


void GeneralEdit::on_stickgainLVCB_currentIndexChanged(int index)
{
	do_stick_gain() ;
}

void GeneralEdit::on_stickgainLHCB_currentIndexChanged(int index)
{
	do_stick_gain() ;
}

void GeneralEdit::on_stickgainRVCB_currentIndexChanged(int index)
{
	do_stick_gain() ;
}

void GeneralEdit::on_stickgainRHCB_currentIndexChanged(int index)
{
	do_stick_gain() ;
}


void GeneralEdit::on_battwarningDSB_editingFinished()
{
    g_eeGeneral.vBatWarn = (int)(ui->battwarningDSB->value()*10);
    updateSettings();
}

//void GeneralEdit::on_battcalibDSB_editingFinished()
//{
//    g_eeGeneral.vBatCalib = ui->battcalibDSB->value()*10;
//    ui->battCalib->setValue(ui->battcalibDSB->value());
//    updateSettings();
//}

void GeneralEdit::on_backlightswCB_currentIndexChanged(int index)
{
    g_eeGeneral.lightSw =  getSwitchCbValue( ui->backlightswCB, rData->type ) ;
    updateSettings();
}

void GeneralEdit::on_BluetoothTypeCB_currentIndexChanged(int index)
{
    g_eeGeneral.BtType = index ;
    updateSettings();
}

void GeneralEdit::on_backlightautoSB_editingFinished()
{
    int i = ui->backlightautoSB->value()/5;
    if((i*5)!=ui->backlightautoSB->value())
        ui->backlightautoSB->setValue(i*5);

    g_eeGeneral.lightAutoOff = i;
    updateSettings();
}

void GeneralEdit::on_backlightStickMove_editingFinished()
{
    int i = ui->backlightStickMove->value()/5;
    if((i*5)!=ui->backlightStickMove->value())
        ui->backlightStickMove->setValue(i*5);

    g_eeGeneral.lightOnStickMove = i;
    updateSettings();
}


void GeneralEdit::on_inactVolumeSB_editingFinished()
{
    g_eeGeneral.inactivityVolume = ui->inactVolumeSB->value() - 21 ;
    updateSettings();
}

void GeneralEdit::on_inactimerSB_editingFinished()
{
    g_eeGeneral.inactivityTimer = ui->inactimerSB->value() - 10;
    updateSettings();
}

void GeneralEdit::on_thrrevChkB_stateChanged(int )
{
    g_eeGeneral.throttleReversed = ui->thrrevChkB->isChecked() ? 1 : 0;
    updateSettings();
}

//void GeneralEdit::on_enablePpmsimChkB_stateChanged(int )
//{
//    g_eeGeneral.enablePpmsim = ui->enablePpmsimChkB->isChecked() ? 1 : 0;
//    updateSettings();
//}

void GeneralEdit::on_internalFrskyAlarmChkB_stateChanged(int )
{
    g_eeGeneral.frskyinternalalarm = ui->internalFrskyAlarmChkB->isChecked() ? 1 : 0;
    updateSettings();
}
		
//void GeneralEdit::on_backlightinvertChkB_stateChanged(int )
//{
//    g_eeGeneral.blightinv = ui->backlightinvertChkB->isChecked() ? 1 : 0;
//    updateSettings();
//}

void GeneralEdit::on_inputfilterCB_currentIndexChanged(int index)
{
    g_eeGeneral.filterInput = index;
    updateSettings();
}

void GeneralEdit::on_thrwarnChkB_stateChanged(int )
{
    g_eeGeneral.disableThrottleWarning = ui->thrwarnChkB->isChecked() ? 0 : 1;
    updateSettings();
}

//void GeneralEdit::on_switchwarnChkB_stateChanged(int )
//{
//    g_eeGeneral.disableSwitchWarning = ui->switchwarnChkB->isChecked() ? 0 : 1;
//    updateSettings();
//}

void GeneralEdit::on_OptrexDisplayChkB_stateChanged(int )
{
	g_eeGeneral.optrexDisplay = ui->OptrexDisplayChkB->isChecked() ;
  updateSettings();
}

//void GeneralEdit::on_memwarnChkB_stateChanged(int )
//{
//    g_eeGeneral.disableMemoryWarning = ui->memwarnChkB->isChecked() ? 0 : 1;
//    updateSettings();
//}

void GeneralEdit::on_alarmwarnChkB_stateChanged(int )
{
    g_eeGeneral.disableAlarmWarning = ui->alarmwarnChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_PotScrollEnableChkB_stateChanged(int )
{
    g_eeGeneral.disablePotScroll = ui->PotScrollEnableChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_StickScrollEnableChkB_stateChanged(int )
{
    g_eeGeneral.stickScroll = ui->StickScrollEnableChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_SoftwareVolumeChkB_stateChanged(int )
{
		if ( rData->bitType & (RADIO_BITTYPE_QX7 | RADIO_BITTYPE_T12 | RADIO_BITTYPE_XXX) )
		{
			g_eeGeneral.softwareVolume = 1 ;
		}
		else
		{
		  g_eeGeneral.softwareVolume = ui->SoftwareVolumeChkB->isChecked() ? 1 : 0 ;
		}
    updateSettings();
}

void GeneralEdit::on_Ar9xChkB_stateChanged(int )
{
    g_eeGeneral.ar9xBoard = ui->Ar9xChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_Pot1DetCB_stateChanged(int )
{
    g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~1) | ( ui->Pot1DetCB->isChecked() ? 1 : 0 ) ;
    updateSettings();
}

void GeneralEdit::on_Pot2DetCB_stateChanged(int )
{
    g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~2) | ( ui->Pot2DetCB->isChecked() ? 2 : 0 ) ;
    updateSettings();
}

void GeneralEdit::on_Pot3DetCB_stateChanged(int )
{
    g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~4) | ( ui->Pot3DetCB->isChecked() ? 4 : 0 ) ;
    updateSettings();
}

void GeneralEdit::on_Pot4DetCB_stateChanged(int )
{
    g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~8) | ( ui->Pot4DetCB->isChecked() ? 8 : 0 ) ;
    updateSettings();
}

void GeneralEdit::on_Pot5DetCB_stateChanged(int )
{
    g_eeGeneral.potDetents = (g_eeGeneral.potDetents & ~16) | ( ui->Pot5DetCB->isChecked() ? 16 : 0 ) ;
    updateSettings();
}

void GeneralEdit::on_MenuEditChkB_stateChanged(int )
{
    g_eeGeneral.forceMenuEdit = ui->MenuEditChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_CrossTrimChkB_stateChanged(int )
{
    g_eeGeneral.crosstrim = ui->CrossTrimChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_RotateScreenChkB_stateChanged(int )
{
    g_eeGeneral.rotateScreen = ui->RotateScreenChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_ReverseScreenChkB_stateChanged(int )
{
    g_eeGeneral.reverseScreen = ui->ReverseScreenChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

//void GeneralEdit::on_BandGapEnableChkB_stateChanged(int )
//{
//    g_eeGeneral.disableBG = ui->BandGapEnableChkB->isChecked() ? 0 : 1;
//    updateSettings();
//}

void GeneralEdit::on_beeperCB_currentIndexChanged(int index)
{
    g_eeGeneral.beeperVal = index;
    updateSettings();
}

void GeneralEdit::on_WelcomeCB_currentIndexChanged(int index)
{
    g_eeGeneral.welcomeType = index ;
    updateSettings();
}

void GeneralEdit::on_hapticMinRunSB_editingFinished()
{
	g_eeGeneral.hapticMinRun = ui->hapticMinRunSB->value() - 20 ;
  updateSettings() ;
}


void GeneralEdit::on_channelorderCB_currentIndexChanged(int index)
{
    g_eeGeneral.templateSetup = index;
    updateSettings();
}

void GeneralEdit::on_languageCB_currentIndexChanged(int index)
{
    g_eeGeneral.language = index;
    updateSettings();
}

void GeneralEdit::on_stickmodeCB_currentIndexChanged(int index)
{
    g_eeGeneral.stickMode = index;
//		StickMode = index ;
    updateSettings();
}

void GeneralEdit::on_RotaryDivisorCB_currentIndexChanged(int index)
{
    g_eeGeneral.rotaryDivisor = index;
    updateSettings();
}

void GeneralEdit::on_BtBaudrateCB_currentIndexChanged(int index)
{
    g_eeGeneral.bt_baudrate = index;
    updateSettings();
}

void GeneralEdit::on_ana1Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[0] = ui->ana1Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana2Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[1] = ui->ana2Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana3Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[2] = ui->ana3Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana4Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[3] = ui->ana4Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana5Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[4] = ui->ana5Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana6Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[5] = ui->ana6Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana7Neg_editingFinished()
{
    g_eeGeneral.calibSpanNeg[6] = ui->ana7Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana8Neg_editingFinished()
{
    g_eeGeneral.x9dcalibSpanNeg = ui->ana8Neg->value();
    updateSettings();
}


void GeneralEdit::on_ana1Mid_editingFinished()
{
    g_eeGeneral.calibMid[0] = ui->ana1Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana2Mid_editingFinished()
{
    g_eeGeneral.calibMid[1] = ui->ana2Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana3Mid_editingFinished()
{
    g_eeGeneral.calibMid[2] = ui->ana3Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana4Mid_editingFinished()
{
    g_eeGeneral.calibMid[3] = ui->ana4Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana5Mid_editingFinished()
{
    g_eeGeneral.calibMid[4] = ui->ana5Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana6Mid_editingFinished()
{
    g_eeGeneral.calibMid[5] = ui->ana6Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana7Mid_editingFinished()
{
    g_eeGeneral.calibMid[6] = ui->ana7Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana8Mid_editingFinished()
{
    g_eeGeneral.x9dcalibMid = ui->ana8Mid->value();
    updateSettings();
}



void GeneralEdit::on_ana1Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[0] = ui->ana1Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana2Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[1] = ui->ana2Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana3Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[2] = ui->ana3Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana4Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[3] = ui->ana4Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana5Pos_editingFinished()
{
    g_eeGeneral.calibSpanNeg[4] = ui->ana5Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana6Pos_editingFinished()
{
    g_eeGeneral.calibSpanNeg[5] = ui->ana6Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana7Pos_editingFinished()
{
    g_eeGeneral.calibSpanPos[6] = ui->ana7Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana8Pos_editingFinished()
{
    g_eeGeneral.x9dcalibSpanPos = ui->ana8Pos->value();
    updateSettings();
}

//void GeneralEdit::on_battCalib_editingFinished()
//{
//    g_eeGeneral.vBatCalib = ui->battCalib->value()*10;
//    ui->battcalibDSB->setValue(ui->battCalib->value());
//    updateSettings();
//}


void GeneralEdit::on_tabWidget_currentChanged(int index)
{
    QSettings settings("er9x-eePe", "eePe");
    settings.setValue("generalEditTab",index);//ui->tabWidget->currentIndex());
}


void GeneralEdit::on_beepMinuteChkB_stateChanged(int )
{
    g_eeGeneral.minuteBeep = ui->beepMinuteChkB->isChecked() ? 1 : 0;
    updateSettings();
}

void GeneralEdit::on_beepCountDownChkB_stateChanged(int )
{
    g_eeGeneral.preBeep = ui->beepCountDownChkB->isChecked() ? 1 : 0;
    updateSettings();
}

void GeneralEdit::on_beepFlashChkB_stateChanged(int )
{
    g_eeGeneral.flashBeep = ui->beepFlashChkB->isChecked() ? 1 : 0;
    updateSettings();
}

void GeneralEdit::on_splashScreenChkB_stateChanged(int )
{
    g_eeGeneral.disableSplashScreen = ui->splashScreenChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_ownerNameLE_editingFinished()
{
    memset(&g_eeGeneral.ownerName,' ',sizeof(g_eeGeneral.ownerName));
    for(quint8 i=0; i<(ui->ownerNameLE->text().length()); i++)
    {
        if(i>=sizeof(g_eeGeneral.ownerName)) break;
        g_eeGeneral.ownerName[i] = ui->ownerNameLE->text().toStdString()[i];
    }
    updateSettings();
}

void GeneralEdit::on_welcomeFileNameLE_editingFinished()
{
    memset(&g_eeGeneral.welcomeFileName,' ',sizeof(g_eeGeneral.welcomeFileName));
    for(quint8 i=0; i<(ui->welcomeFileNameLE->text().length()); i++)
    {
        if(i>=sizeof(g_eeGeneral.welcomeFileName)) break ;
        g_eeGeneral.welcomeFileName[i] = ui->welcomeFileNameLE->text().toStdString()[i];
    }
    updateSettings();
}

void GeneralEdit::on_BtNameText_editingFinished()
{
	quint8 i ;
//    memset(&g_eeGeneral.btName,' ',sizeof(g_eeGeneral.ownerName));
    for( i=0; i<(sizeof(g_eeGeneral.btName)); i++)
    {
			quint8 c ;
      if(i>=sizeof(g_eeGeneral.btName)-1) break;
      c = ui->BtNameText->text().toStdString()[i];
			g_eeGeneral.btName[i] = c ;
			if ( c == 0 )
			{
				break ;
			}
    }
    while ( i<(sizeof(g_eeGeneral.btName)) )
		{
			g_eeGeneral.btName[i++] = '\0' ;
		}
    updateSettings();
}

void GeneralEdit::btDevEdited( int dev, QLineEdit *uiDev )
{
	quint8 i ;
//    memset(&g_eeGeneral.btName,' ',sizeof(g_eeGeneral.ownerName));
    for( i=0; i<7 ; i++ )
    {
			quint8 c ;
      if(i>=6) break;
      c = uiDev->text().toStdString()[i];
			g_eeGeneral.btDevice[dev].name[i] = c ;
			if ( c == 0 )
			{
				break ;
			}
    }
    while ( i<7 )
		{
			g_eeGeneral.btDevice[dev].name[i++] = '\0' ;
		}
    updateSettings();
}

void GeneralEdit::on_BtDev1Name_editingFinished()
{
	btDevEdited( 0, ui->BtDev1Name ) ;
}

void GeneralEdit::on_BtDev2Name_editingFinished()
{
	btDevEdited( 1, ui->BtDev2Name ) ;
}

void GeneralEdit::on_BtDev3Name_editingFinished()
{
	btDevEdited( 2, ui->BtDev3Name ) ;
}

void GeneralEdit::on_BtDev4Name_editingFinished()
{
	btDevEdited( 3, ui->BtDev4Name ) ;
}

void GeneralEdit::on_speakerPitchSB_editingFinished()
{
    g_eeGeneral.speakerPitch = ui->speakerPitchSB->value();
    updateSettings();
}

void GeneralEdit::on_hapticStengthSB_editingFinished()
{
    g_eeGeneral.hapticStrength = ui->hapticStengthSB->value();
    updateSettings();
}

//void GeneralEdit::on_soundModeCB_currentIndexChanged(int index)
//{
//    g_eeGeneral.speakerMode = index;
//    updateSettings();
//}

void GeneralEdit::on_tabWidget_selected(QString )
{
    ui->chnLabel_1->setText(getSourceStr(g_eeGeneral.stickMode,1,2,0,0));
    ui->chnLabel_2->setText(getSourceStr(g_eeGeneral.stickMode,2,2,0,0));
    ui->chnLabel_3->setText(getSourceStr(g_eeGeneral.stickMode,3,2,0,0));
    ui->chnLabel_4->setText(getSourceStr(g_eeGeneral.stickMode,4,2,0,0));
}



void GeneralEdit::on_splashScreenNameChkB_stateChanged(int )
{
    g_eeGeneral.hideNameOnSplash = ui->splashScreenNameChkB->isChecked() ? 0 : 1;
    updateSettings();
}

//void GeneralEdit::getGeneralSwitchDefPos(int i, bool val)
//{
//    if(val)
//        g_eeGeneral.switchWarningStates |= (1<<(i-1));
//    else
//        g_eeGeneral.switchWarningStates &= ~(1<<(i-1));
//}

//void GeneralEdit::on_switchDefPos_1_stateChanged(int )
//{
//    if(switchDefPosEditLock) return;
//    getGeneralSwitchDefPos(1,ui->switchDefPos_1->isChecked());
//    updateSettings();
//}
//void GeneralEdit::on_switchDefPos_2_stateChanged(int )
//{
//    if(switchDefPosEditLock) return;
//    getGeneralSwitchDefPos(2,ui->switchDefPos_2->isChecked());
//    updateSettings();
//}
//void GeneralEdit::on_switchDefPos_3_stateChanged(int )
//{
//    getGeneralSwitchDefPos(3,ui->switchDefPos_3->isChecked());
//    updateSettings();
//}
//void GeneralEdit::on_switchDefPos_4_stateChanged(int )
//{
//    if(switchDefPosEditLock) return;

//    if(ui->switchDefPos_4->isChecked())
//    {
//        switchDefPosEditLock = true;
//        ui->switchDefPos_5->setChecked(false);
//        ui->switchDefPos_6->setChecked(false);
//        switchDefPosEditLock = false;
//    }
//    else
//        return;

//    g_eeGeneral.switchWarningStates &= ~0x30; //turn off ID1/2
//    getGeneralSwitchDefPos(4,ui->switchDefPos_4->isChecked());
//    updateSettings();
//}
//void GeneralEdit::on_switchDefPos_5_stateChanged(int )
//{
//    if(switchDefPosEditLock) return;

//    if(ui->switchDefPos_5->isChecked())
//    {
//        switchDefPosEditLock = true;
//        ui->switchDefPos_4->setChecked(false);
//        ui->switchDefPos_6->setChecked(false);
//        switchDefPosEditLock = false;
//    }
//    else
//        return;

//    g_eeGeneral.switchWarningStates &= ~0x28; //turn off ID0/2
//    getGeneralSwitchDefPos(5,ui->switchDefPos_5->isChecked());
//    updateSettings();
//}
//void GeneralEdit::on_switchDefPos_6_stateChanged(int )
//{
//    if(switchDefPosEditLock) return;

//    if(ui->switchDefPos_6->isChecked())
//    {
//        switchDefPosEditLock = true;
//        ui->switchDefPos_4->setChecked(false);
//        ui->switchDefPos_5->setChecked(false);
//        switchDefPosEditLock = false;
//    }
//    else
//        return;

//    g_eeGeneral.switchWarningStates &= ~0x18; //turn off ID1/2
//    getGeneralSwitchDefPos(6,ui->switchDefPos_6->isChecked());
//    updateSettings();
//}
//void GeneralEdit::on_switchDefPos_7_stateChanged(int )
//{
//    if(switchDefPosEditLock) return;
//    getGeneralSwitchDefPos(7,ui->switchDefPos_7->isChecked());
//    updateSettings();
//}
//void GeneralEdit::on_switchDefPos_8_stateChanged(int )
//{
//    if(switchDefPosEditLock) return;
//    getGeneralSwitchDefPos(8,ui->switchDefPos_8->isChecked());
//    updateSettings();
//}

void GeneralEdit::on_StickRevLH_stateChanged(int )
{
	g_eeGeneral.stickReverse &= ~0xF1 ;
	if (ui->StickRevLH->isChecked() )
	{
		g_eeGeneral.stickReverse |= 0x01 ;
	}
  updateSettings();
}

void GeneralEdit::on_StickRevLV_stateChanged(int )
{
	g_eeGeneral.stickReverse &= ~0xF2 ;
	if (ui->StickRevLV->isChecked() )
	{
		g_eeGeneral.stickReverse |= 0x02 ;
	}
  updateSettings();
}

void GeneralEdit::on_StickRevRV_stateChanged(int )
{
	g_eeGeneral.stickReverse &= ~0xF4 ;
	if (ui->StickRevRV->isChecked() )
	{
		g_eeGeneral.stickReverse |= 0x04 ;
	}
  updateSettings();
}

void GeneralEdit::on_StickRevRH_stateChanged(int )
{
	g_eeGeneral.stickReverse &= ~0xF8 ;
	if (ui->StickRevRH->isChecked() )
	{
		g_eeGeneral.stickReverse |= 0x08 ;
	}
  updateSettings();
}

void GeneralEdit::on_StickLHdeadbandSB_editingFinished()
{
  g_eeGeneral.stickDeadband[0] = ui->StickLHdeadbandSB->value() ;
  updateSettings() ;
}

void GeneralEdit::on_StickLVdeadbandSB_editingFinished()
{
  g_eeGeneral.stickDeadband[1] = ui->StickLVdeadbandSB->value() ;
  updateSettings() ;
}

void GeneralEdit::on_StickRVdeadbandSB_editingFinished()
{
  g_eeGeneral.stickDeadband[2] = ui->StickRVdeadbandSB->value() ;
  updateSettings() ;
}

void GeneralEdit::on_StickRHdeadbandSB_editingFinished()
{
  g_eeGeneral.stickDeadband[3] = ui->StickRHdeadbandSB->value() ;
  updateSettings() ;
}



void GeneralEdit::on_EncoderCB_currentIndexChanged(int x )
{
	g_eeGeneral.analogMapping &= ~3 ;
	g_eeGeneral.analogMapping |= x & 3 ;
  updateSettings();
}

void GeneralEdit::on_SixPosCB_currentIndexChanged(int x )
{
	g_eeGeneral.analogMapping &= ~0x1C ;
	g_eeGeneral.analogMapping |= (x & 7 ) << 2 ;
  updateSettings();
}

void GeneralEdit::on_AilCB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
	uint16_t value = x ? USE_AIL_3POS : 0 ;
	g_eeGeneral.switchMapping &= ~USE_AIL_3POS ;
	g_eeGeneral.switchMapping |= value ;
	g_eeGeneral.ailsource = x ;
	if ( x )
	{
//		g_eeGeneral.switchMapping &= ~USE_GEA_3POS  ;
//		ui->GeaNumWaysCB->setCurrentIndex( 0 ) ;
	}
	setHwSwitchActive() ;
  updateSettings();
}

void GeneralEdit::on_EleCB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
  g_eeGeneral.switchMapping &= ~(USE_ELE_3POS | USE_ELE_6POS | USE_ELE_6PSB ) ;
	uint16_t value = x ;
	if ( rData->type != RADIO_TYPE_9XTREME )
	{
		if ( value > 5 )
		{
			value -= 2 ;
			if ( value < 6 )
			{
				value += 102-6 ;
			}
		}
	}
	if ( value )
	{
		uint16_t mask = USE_ELE_3POS ;
		if ( value == 100 )
		{
			mask = USE_ELE_6POS ;
		}
		else if ( value == 101 )
		{
			mask = USE_ELE_6PSB ;
		}
		g_eeGeneral.switchMapping |= mask ;
	} 
	g_eeGeneral.elesource = value ;
//	if ( x == 0 )
//	{
//		g_eeGeneral.switchMapping &= ~(USE_RUD_3POS | USE_THR_3POS ) ;
//		ui->ThrNumWaysCB->setCurrentIndex( 0 ) ;
//		ui->RudNumWaysCB->setCurrentIndex( 0 ) ;
//	}
	setHwSwitchActive() ;
  updateSettings();
}

void GeneralEdit::on_GeaCB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
	uint16_t value = x ? USE_GEA_3POS : 0 ;
	g_eeGeneral.switchMapping &= ~USE_GEA_3POS ;
	g_eeGeneral.switchMapping |= value ;
	g_eeGeneral.geasource = x ;
//	if ( x )
//	{
//		g_eeGeneral.switchMapping &= ~USE_AIL_3POS  ;
//		ui->AilNumWaysCB->setCurrentIndex( 0 ) ;
//	}
	setHwSwitchActive() ;
  updateSettings();
}

void GeneralEdit::on_RudCB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
	uint16_t value = x ? USE_RUD_3POS : 0 ;
	g_eeGeneral.switchMapping &= ~USE_RUD_3POS ;
	g_eeGeneral.switchMapping |= value ;
	g_eeGeneral.rudsource = x ;
//	if ( x )
//	{
//		g_eeGeneral.switchMapping &= ~USE_THR_3POS  ;
//		ui->ThrNumWaysCB->setCurrentIndex( 0 ) ;
//	}
	setHwSwitchActive() ;
  updateSettings();
}

void GeneralEdit::on_ThrCB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
	uint16_t value = x ? USE_THR_3POS : 0 ;
	g_eeGeneral.switchMapping &= ~USE_THR_3POS ;
	g_eeGeneral.switchMapping |= value ;
	g_eeGeneral.thrsource = x ;
//	if ( x )
//	{
//		g_eeGeneral.switchMapping &= ~USE_RUD_3POS  ;
//		ui->RudNumWaysCB->setCurrentIndex( 0 ) ;
//	}
	setHwSwitchActive() ;
  updateSettings();
}

void GeneralEdit::on_PB1CB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
	uint16_t value = x ? USE_PB1 : 0 ;
	g_eeGeneral.switchMapping &= ~USE_PB1 ;
	g_eeGeneral.switchMapping |= value ;
	g_eeGeneral.pb1source = x ;
	setHwSwitchActive() ;
  updateSettings();
}

void GeneralEdit::on_PB2CB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
	uint16_t value = x ? USE_PB2 : 0 ;
	g_eeGeneral.switchMapping &= ~USE_PB2 ;
	g_eeGeneral.switchMapping |= value ;
	g_eeGeneral.pb2source = x ;
	setHwSwitchActive() ;
  updateSettings();
}

void GeneralEdit::on_PB3CB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
	uint16_t value = x ? USE_PB3 : 0 ;
	g_eeGeneral.switchMapping &= ~USE_PB3 ;
	g_eeGeneral.switchMapping |= value ;
	g_eeGeneral.pb3source = x ;
	setHwSwitchActive() ;
  updateSettings();
}

void GeneralEdit::on_PB4CB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
	uint16_t value = x ? USE_PB4 : 0 ;
	g_eeGeneral.switchMapping &= ~USE_PB4 ;
	g_eeGeneral.switchMapping |= value ;
	g_eeGeneral.pb4source = x ;
	setHwSwitchActive() ;
  updateSettings();
}

void GeneralEdit::on_Pot4CB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
	g_eeGeneral.extraPotsSource[0] = x ;
  updateSettings();
}
void GeneralEdit::on_Pot5CB_currentIndexChanged(int x )
{
	if ( hardwareTabLock )
	{
		return ;
	}
	g_eeGeneral.extraPotsSource[1] = x ;
  updateSettings();
}
void GeneralEdit::on_ExtRtcCB_currentIndexChanged(int index)
{
	g_eeGeneral.externalRtcType = index ;
  updateSettings() ;
}

void GeneralEdit::setHwSwitchActive()
{
//	if ( rData->type )
//	{
//		ui->EleNumWaysCB->setEnabled( false ) ;
//		ui->RudNumWaysCB->setEnabled( false ) ;
////		ui->ThrNumWaysCB->setEnabled( false ) ;
//		ui->GeaNumWaysCB->setEnabled( false ) ;
//		ui->AilNumWaysCB->setEnabled( false ) ;
//	}
//	else
//	{
//  	if ( (g_eeGeneral.switchMapping & (USE_ELE_3POS | USE_ELE_6POS)) == 0 )
//		{
//			ui->RudNumWaysCB->setEnabled( false ) ;
////			ui->ThrNumWaysCB->setEnabled( false ) ;
//		}
//		else
//		{
//			ui->RudNumWaysCB->setEnabled( (g_eeGeneral.switchMapping & USE_THR_3POS) ? false : true ) ;
////			ui->ThrNumWaysCB->setEnabled( (g_eeGeneral.switchMapping & USE_RUD_3POS) ? false : true ) ;
//		}

//		ui->GeaNumWaysCB->setEnabled( (g_eeGeneral.switchMapping & USE_AIL_3POS) ? false : true ) ;
//		ui->AilNumWaysCB->setEnabled( (g_eeGeneral.switchMapping & USE_GEA_3POS) ? false : true ) ;
    createSwitchMapping( &g_eeGeneral, MAX_DRSWITCH, rData->type ) ;
//	}
}

