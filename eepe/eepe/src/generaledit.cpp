#include "generaledit.h"
#include "ui_generaledit.h"
#include "pers.h"
#include "helpers.h"
#include <QtGui>
//#ifdef V2
#include "mdichild.h"
//#endif

#define BIT_WARN_THR     ( 0x01 )
#define BIT_WARN_SW      ( 0x02 )
#define BIT_WARN_MEM     ( 0x04 )
#define BIT_WARN_BEEP    ( 0x80 )
#define BIT_BEEP_VAL     ( 0x38 ) // >>3
#define BEEP_VAL_SHIFT   3

#ifdef V2
extern V2EEGeneral Sim_g ;
extern V2ModelData Sim_m ;
#else
extern EEGeneral Sim_g ;
extern ModelData Sim_m ;
#endif
extern int GlobalModified ;
extern int GeneralDataValid ;
extern int ModelDataValid ;

GeneralEdit::GeneralEdit(EEPFILE *eFile, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralEdit)
{
  rData = &((MdiChild *)parent)->radioData ;
#ifdef V2
	p_eeGeneral = &rData->v2generalSettings ;
#else
	p_eeGeneral = &rData->generalSettings ;
#endif
	ui->setupUi(this);
    this->setWindowIcon(QIcon(":/icon.png"));
    eeFile = eFile;

    switchDefPosEditLock = false;

    QSettings settings("er9x-eePe", "eePe");
    ui->tabWidget->setCurrentIndex(settings.value("generalEditTab", 0).toInt());


		eeFile->getGeneralSettings(p_eeGeneral);
#ifdef V2
    rData->initSwitchMapping() ;
    rData->setMaxSwitchIndex() ;
#endif

#ifndef V2
		createSwitchMapping( p_eeGeneral, eeFile->mee_type ) ;
#endif
    QRegExp rx(CHAR_FOR_NAMES_REGEX);
    ui->ownerNameLE->setValidator(new QRegExpValidator(rx, this));

#ifdef V2
		rData->populateSwitchCB(ui->backlightswCB,p_eeGeneral->lightSw,eeFile->mee_type, 0 ) ;
#else
    populateSwitchCB(ui->backlightswCB,p_eeGeneral->lightSw,eeFile->mee_type);
#endif
    ui->ownerNameLE->setText(p_eeGeneral->ownerName);


		for(quint8 i=0; i<16; i++)
		{
			if (p_eeGeneral->customStickNames[i] == 0 )
			{
				p_eeGeneral->customStickNames[i] = ' ' ;
			}
		}

    QString Str = (char *)p_eeGeneral->customStickNames ;
    ui->rudNameLE->setText( Str.mid(0,4)) ;
    ui->eleNameLE->setText( Str.mid(4,4)) ;
    ui->thrNameLE->setText( Str.mid(8,4)) ;
    ui->ailNameLE->setText( Str.mid(12,4)) ;

    ui->contrastSB->setValue(p_eeGeneral->contrast);
    ui->battwarningDSB->setValue((double)p_eeGeneral->vBatWarn/10);
    ui->battcalibDSB->setValue((double)p_eeGeneral->vBatCalib/10);
    ui->battCalib->setValue((double)p_eeGeneral->vBatCalib/10);
    ui->backlightautoSB->setValue(p_eeGeneral->lightAutoOff*5);
    ui->backlightStickMove->setValue(p_eeGeneral->lightOnStickMove*5);
    ui->inactimerSB->setValue(p_eeGeneral->inactivityTimer+10);

    ui->soundModeCB->setCurrentIndex(p_eeGeneral->speakerMode > 3 ? 4 : p_eeGeneral->speakerMode );
    ui->speakerPitchSB->setValue(p_eeGeneral->speakerPitch);
    ui->hapticStengthSB->setValue(p_eeGeneral->hapticStrength);

    ui->thrrevChkB->setChecked(p_eeGeneral->throttleReversed);
//    ui->inputfilterCB->setCurrentIndex(p_eeGeneral->filterInput);
    ui->thrwarnChkB->setChecked(!p_eeGeneral->disableThrottleWarning);   //Default is zero=checked
    ui->switchwarnChkB->setChecked(!p_eeGeneral->disableSwitchWarning); //Default is zero=checked
    ui->memwarnChkB->setChecked(!p_eeGeneral->disableMemoryWarning);   //Default is zero=checked
    ui->alarmwarnChkB->setChecked(!p_eeGeneral->disableAlarmWarning);//Default is zero=checked
    ui->PotScrollEnableChkB->setChecked(!p_eeGeneral->disablePotScroll);//Default is zero=checked
    ui->StickScrollEnableChkB->setChecked(p_eeGeneral->stickScroll);//Default is zero=not checked
    ui->CrossTrimChkB->setChecked(p_eeGeneral->crosstrim);//Default is zero=not checked
    ui->FrskyPinsChkB->setChecked(p_eeGeneral->FrskyPins);//Default is zero=not checked
    ui->TeZ_gt_90ChkB->setChecked(p_eeGeneral->TEZr90);//Default is zero=not checked
    ui->MsoundSerialChkB->setChecked(p_eeGeneral->MegasoundSerial);//Default is zero=not checked
    ui->RotateScreenChkB->setChecked(p_eeGeneral->rotateScreen);//Default is zero=not checked
    ui->SerialLCDChkB->setChecked(p_eeGeneral->serialLCD);//Default is zero=not checked
    ui->SSD1306ChkB->setChecked(p_eeGeneral->SSD1306);//Default is zero=not checked
    ui->BandGapEnableChkB->setChecked(!p_eeGeneral->disableBG);//Default is zero=checked
    ui->beeperCB->setCurrentIndex(p_eeGeneral->beeperVal);
    ui->channelorderCB->setCurrentIndex(p_eeGeneral->templateSetup);
    ui->stickmodeCB->setCurrentIndex(p_eeGeneral->stickMode);
    
		ui->volumeSB->setValue(p_eeGeneral->volume+7);
    ui->enablePpmsimChkB->setChecked(p_eeGeneral->enablePpmsim);
#ifndef V2
    ui->internalFrskyAlarmChkB->setChecked(p_eeGeneral->frskyinternalalarm);
#else
		ui->internalFrskyAlarmChkB->hide() ;
#endif
    ui->backlightinvertChkB->setChecked(p_eeGeneral->blightinv);
#ifndef V2
    ui->beepMinuteChkB->setChecked(p_eeGeneral->minuteBeep);
    ui->beepCountDownChkB->setChecked(p_eeGeneral->preBeep);
		ui->LvTrimModCB->hide() ;
		ui->PB7BacklightCB->hide() ;
#else
    ui->LvTrimModCB->setChecked(p_eeGeneral->LVTrimMod) ;
    ui->PB7BacklightCB->setChecked(p_eeGeneral->pb7backlight) ;
    ui->beepMinuteChkB->hide() ;
    ui->beepCountDownChkB->hide() ;
		ui->battcalibDSB->hide() ;
		ui->labelBatCal->hide() ;
		ui->labelDefSwitch->hide() ;
		ui->switchDefPos_1->hide() ;
		ui->switchDefPos_2->hide() ;
		ui->switchDefPos_3->hide() ;
		ui->switchDefPos_4->hide() ;
		ui->switchDefPos_5->hide() ;
		ui->switchDefPos_6->hide() ;
		ui->switchDefPos_7->hide() ;
		ui->switchDefPos_8->hide() ;
		ui->Pb7InputCB->hide() ;
		ui->Pg2InputCB->hide() ;
		ui->L_wrInputCB->hide() ;
#endif
    ui->beepFlashChkB->setChecked(p_eeGeneral->flashBeep);
    ui->splashScreenChkB->setChecked(!p_eeGeneral->disableSplashScreen);
    ui->splashScreenNameChkB->setChecked(!p_eeGeneral->hideNameOnSplash);

    uint8_t db = (p_eeGeneral->stickDeadband & 0x00F0 ) >> 4 ;
		ui->StickLVdeadbandSB->setValue(db) ;
    db = p_eeGeneral->stickDeadband & 0x000F ;
		ui->StickLHdeadbandSB->setValue(db) ;
    db = (p_eeGeneral->stickDeadband & 0x0F00 ) >> 8 ;
		ui->StickRVdeadbandSB->setValue(db) ;
    db = (p_eeGeneral->stickDeadband & 0xF000 ) >> 12 ;
		ui->StickRHdeadbandSB->setValue(db) ;

    ui->ana1Neg->setValue(p_eeGeneral->calibSpanNeg[0]);
    ui->ana2Neg->setValue(p_eeGeneral->calibSpanNeg[1]);
    ui->ana3Neg->setValue(p_eeGeneral->calibSpanNeg[2]);
    ui->ana4Neg->setValue(p_eeGeneral->calibSpanNeg[3]);
    ui->ana5Neg->setValue(p_eeGeneral->calibSpanNeg[4]);
    ui->ana6Neg->setValue(p_eeGeneral->calibSpanNeg[5]);
    ui->ana7Neg->setValue(p_eeGeneral->calibSpanNeg[6]);

    ui->ana1Mid->setValue(p_eeGeneral->calibMid[0]);
    ui->ana2Mid->setValue(p_eeGeneral->calibMid[1]);
    ui->ana3Mid->setValue(p_eeGeneral->calibMid[2]);
    ui->ana4Mid->setValue(p_eeGeneral->calibMid[3]);
    ui->ana5Mid->setValue(p_eeGeneral->calibMid[4]);
    ui->ana6Mid->setValue(p_eeGeneral->calibMid[5]);
    ui->ana7Mid->setValue(p_eeGeneral->calibMid[6]);

    ui->ana1Pos->setValue(p_eeGeneral->calibSpanPos[0]);
    ui->ana2Pos->setValue(p_eeGeneral->calibSpanPos[1]);
    ui->ana3Pos->setValue(p_eeGeneral->calibSpanPos[2]);
    ui->ana4Pos->setValue(p_eeGeneral->calibSpanPos[3]);
    ui->ana5Pos->setValue(p_eeGeneral->calibSpanPos[4]);
    ui->ana6Pos->setValue(p_eeGeneral->calibSpanPos[5]);
    ui->ana7Pos->setValue(p_eeGeneral->calibSpanPos[6]);

    setSwitchDefPos();
    
    ui->StickRevLH->setChecked(p_eeGeneral->stickReverse & 0x01);
    ui->StickRevLV->setChecked(p_eeGeneral->stickReverse & 0x02);
    ui->StickRevRV->setChecked(p_eeGeneral->stickReverse & 0x04);
    ui->StickRevRH->setChecked(p_eeGeneral->stickReverse & 0x08);

#ifndef V2
		ui->weightSB_1->findChild<QLineEdit*>()->setReadOnly(true);
    ui->weightSB_2->findChild<QLineEdit*>()->setReadOnly(true);
    ui->weightSB_3->findChild<QLineEdit*>()->setReadOnly(true);
    ui->weightSB_4->findChild<QLineEdit*>()->setReadOnly(true);
#endif

#ifdef V2
    ui->weightSB_1->setMinimum( -100 ) ;
    ui->weightSB_1->setMaximum( 100 ) ;
		ui->weightSB_1->setSingleStep( 1 ) ;
		ui->weightSB_1->setAccelerated( true ) ;
    ui->weightSB_2->setMinimum( -100 ) ;
    ui->weightSB_2->setMaximum( 100 ) ;
		ui->weightSB_2->setSingleStep( 1 ) ;
		ui->weightSB_2->setAccelerated( true ) ;
    ui->weightSB_3->setMinimum( -100 ) ;
    ui->weightSB_3->setMaximum( 100 ) ;
		ui->weightSB_3->setSingleStep( 1 ) ;
		ui->weightSB_3->setAccelerated( true ) ;
    ui->weightSB_4->setMinimum( -100 ) ;
    ui->weightSB_4->setMaximum( 100 ) ;
		ui->weightSB_4->setSingleStep( 1 ) ;
		ui->weightSB_4->setAccelerated( true ) ;
#endif

    updateTrainerTab();

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

    connect(ui->weightSB_1, SIGNAL(valueChanged(int)), this, SLOT(validateWeightSB()));
    connect(ui->weightSB_2, SIGNAL(valueChanged(int)), this, SLOT(validateWeightSB()));
    connect(ui->weightSB_3, SIGNAL(valueChanged(int)), this, SLOT(validateWeightSB()));
    connect(ui->weightSB_4, SIGNAL(valueChanged(int)), this, SLOT(validateWeightSB()));

#ifdef V2
	populateHardwareSwitch(ui->ThrSwitchSource, p_eeGeneral->switchSources[0] & 0x0F ) ;
	populateHardwareSwitch(ui->RudSwitchSource, (p_eeGeneral->switchSources[0] >> 4) & 0x0F ) ;
	populateHardwareSwitch(ui->EleSwitchSource, p_eeGeneral->switchSources[1] & 0x0F ) ;
	populateHardwareSwitch(ui->AilSwitchSource, (p_eeGeneral->switchSources[1] >> 4) & 0x0F ) ;
	populateHardwareSwitch(ui->GeaSwitchSource, p_eeGeneral->switchSources[2] & 0x0F ) ;
	populateHardwareSwitch(ui->Pb1SwitchSource, (p_eeGeneral->switchSources[2] >> 4) & 0x0F ) ;
	populateHardwareSwitch(ui->Pb2SwitchSource, p_eeGeneral->switchSources[3] & 0x0F ) ;
	ui->Pg2InputCB->hide() ;
	ui->Pb7InputCB->hide() ;
	ui->L_wrInputCB->hide() ;
#else
	populateHardwareSwitch(ui->EleSwitchSource, p_eeGeneral->ele2source ) ;
	populateHardwareSwitch(ui->AilSwitchSource, p_eeGeneral->ail2source ) ;
	ui->ThrSwitchSource->hide() ;
  ui->GeaSwitchSource->hide() ;
	ui->RudSwitchSource->hide() ;
	ui->label_ThrSw->hide() ;
  ui->label_GeaSw->hide() ;
	ui->label_RudSw->hide() ;
	populateHardwareSwitch(ui->Pb1SwitchSource, p_eeGeneral->pb1source ) ;
	populateHardwareSwitch(ui->Pb2SwitchSource, p_eeGeneral->pb2source ) ;
	ui->L_wrInputCB->setChecked( p_eeGeneral->lcd_wrInput ) ;
	ui->Pb7InputCB->setChecked( p_eeGeneral->pb7Input ) ;
  ui->Pg2InputCB->setChecked( p_eeGeneral->pg2Input ) ;
#endif
}

GeneralEdit::~GeneralEdit()
{
    delete ui;
}

void GeneralEdit::setSwitchDefPos()
{
#ifndef V2
  quint8 x = p_eeGeneral->switchWarningStates & SWP_IL5;
    if(x==SWP_IL1 || x==SWP_IL2 || x==SWP_IL3 || x==SWP_IL4 || x==SWP_IL5) //illegal states for ID0/1/2
    {
        p_eeGeneral->switchWarningStates &= ~SWP_IL5; // turn all off, make sure only one is on
        p_eeGeneral->switchWarningStates |=  SWP_ID0B;
    }

    switchDefPosEditLock = true;
    ui->switchDefPos_1->setChecked(p_eeGeneral->switchWarningStates & 0x01);
    ui->switchDefPos_2->setChecked(p_eeGeneral->switchWarningStates & 0x02);
    ui->switchDefPos_3->setChecked(p_eeGeneral->switchWarningStates & 0x04);
    ui->switchDefPos_4->setChecked(p_eeGeneral->switchWarningStates & 0x08);
    ui->switchDefPos_5->setChecked(p_eeGeneral->switchWarningStates & 0x10);
    ui->switchDefPos_6->setChecked(p_eeGeneral->switchWarningStates & 0x20);
    ui->switchDefPos_7->setChecked(p_eeGeneral->switchWarningStates & 0x40);
    ui->switchDefPos_8->setChecked(p_eeGeneral->switchWarningStates & 0x80);
    switchDefPosEditLock = false;
#endif
}

void GeneralEdit::updateSettings()
{
    int16_t sum=0;
    for(int i=0; i<12;i++) sum+=p_eeGeneral->calibMid[i];
    p_eeGeneral->chkSum = sum;
    eeFile->putGeneralSettings(p_eeGeneral);

    emit modelValuesChanged();
    
#ifdef V2
		memcpy(&Sim_g, p_eeGeneral,sizeof(V2EEGeneral));
#else
    memcpy(&Sim_g, p_eeGeneral,sizeof(EEGeneral));
#endif
		GeneralDataValid = 1 ;
		ModelDataValid = 0 ;
		GlobalModified = 1 ;
}


void GeneralEdit::updateTrainerTab()
{
    on_tabWidget_selected(""); // updates channel name labels

    ui->modeCB_1->setCurrentIndex(p_eeGeneral->trainer.mix[0].mode);
    ui->sourceCB_1->setCurrentIndex(p_eeGeneral->trainer.mix[0].srcChn);
#ifndef V2
    populateTrainerSwitchCB(ui->swtchCB_1,p_eeGeneral->trainer.mix[0].swtch);
    ui->weightSB_1->setValue(p_eeGeneral->trainer.mix[0].studWeight*13/4);
    StudWeight1=p_eeGeneral->trainer.mix[0].studWeight*13/4;
#else
		rData->populateSwitchCB(ui->swtchCB_1,p_eeGeneral->trainer.mix[0].swtch, eeFile->mee_type, 0 );
    ui->weightSB_1->setValue(p_eeGeneral->trainer.mix[0].studWeight );
    StudWeight1=p_eeGeneral->trainer.mix[0].studWeight ;
#endif
    ui->modeCB_2->setCurrentIndex(p_eeGeneral->trainer.mix[1].mode);
    ui->sourceCB_2->setCurrentIndex(p_eeGeneral->trainer.mix[1].srcChn);
#ifndef V2
    populateTrainerSwitchCB(ui->swtchCB_2,p_eeGeneral->trainer.mix[1].swtch);
    ui->weightSB_2->setValue(p_eeGeneral->trainer.mix[1].studWeight*13/4);
    StudWeight2=p_eeGeneral->trainer.mix[1].studWeight*13/4;
#else
    rData->populateSwitchCB(ui->swtchCB_2,p_eeGeneral->trainer.mix[1].swtch, eeFile->mee_type, 0 );
    ui->weightSB_2->setValue(p_eeGeneral->trainer.mix[1].studWeight );
    StudWeight2=p_eeGeneral->trainer.mix[1].studWeight ;
#endif

    ui->modeCB_3->setCurrentIndex(p_eeGeneral->trainer.mix[2].mode);
    ui->sourceCB_3->setCurrentIndex(p_eeGeneral->trainer.mix[2].srcChn);
#ifndef V2
    populateTrainerSwitchCB(ui->swtchCB_3,p_eeGeneral->trainer.mix[2].swtch);
    ui->weightSB_3->setValue(p_eeGeneral->trainer.mix[2].studWeight*13/4);
    StudWeight3=p_eeGeneral->trainer.mix[2].studWeight*13/4;
#else
    rData->populateSwitchCB(ui->swtchCB_3,p_eeGeneral->trainer.mix[2].swtch, eeFile->mee_type, 0 );
    ui->weightSB_3->setValue(p_eeGeneral->trainer.mix[2].studWeight );
    StudWeight3=p_eeGeneral->trainer.mix[3].studWeight ;
#endif

    ui->modeCB_4->setCurrentIndex(p_eeGeneral->trainer.mix[0].mode);
    ui->sourceCB_4->setCurrentIndex(p_eeGeneral->trainer.mix[3].srcChn);
#ifndef V2
    populateTrainerSwitchCB(ui->swtchCB_4,p_eeGeneral->trainer.mix[3].swtch);
    ui->weightSB_4->setValue(p_eeGeneral->trainer.mix[3].studWeight*13/4);
    StudWeight4=p_eeGeneral->trainer.mix[3].studWeight*13/4;
#else
    rData->populateSwitchCB(ui->swtchCB_4,p_eeGeneral->trainer.mix[3].swtch, eeFile->mee_type, 0 );
    ui->weightSB_4->setValue(p_eeGeneral->trainer.mix[3].studWeight );
    StudWeight4=p_eeGeneral->trainer.mix[3].studWeight ;
#endif

    ui->trainerCalib_1->setValue(p_eeGeneral->trainer.calib[0]);
    ui->trainerCalib_2->setValue(p_eeGeneral->trainer.calib[1]);
    ui->trainerCalib_3->setValue(p_eeGeneral->trainer.calib[2]);
    ui->trainerCalib_4->setValue(p_eeGeneral->trainer.calib[3]);

    ui->PPM_MultiplierDSB->setValue(double(p_eeGeneral->PPM_Multiplier+10)/10);
}

void GeneralEdit::trainerTabValueChanged()
{
    p_eeGeneral->trainer.mix[0].mode       = ui->modeCB_1->currentIndex();
//    p_eeGeneral->trainer.mix[0].studWeight = ui->weightSB_1->value()*4/13;
    p_eeGeneral->trainer.mix[0].srcChn     = ui->sourceCB_1->currentIndex();
    p_eeGeneral->trainer.mix[0].swtch      = ui->swtchCB_1->currentIndex()-15 ;

    p_eeGeneral->trainer.mix[1].mode       = ui->modeCB_2->currentIndex();
//    p_eeGeneral->trainer.mix[1].studWeight = ui->weightSB_2->value()*4/13;
    p_eeGeneral->trainer.mix[1].srcChn     = ui->sourceCB_2->currentIndex();
    p_eeGeneral->trainer.mix[1].swtch      = ui->swtchCB_2->currentIndex()-15 ;

    p_eeGeneral->trainer.mix[2].mode       = ui->modeCB_3->currentIndex();
//    p_eeGeneral->trainer.mix[2].studWeight = ui->weightSB_3->value()*4/13;
    p_eeGeneral->trainer.mix[2].srcChn     = ui->sourceCB_3->currentIndex();
    p_eeGeneral->trainer.mix[2].swtch      = ui->swtchCB_3->currentIndex()-15 ;

    p_eeGeneral->trainer.mix[3].mode       = ui->modeCB_4->currentIndex();
//    p_eeGeneral->trainer.mix[3].studWeight = ui->weightSB_4->value()*4/13;
    p_eeGeneral->trainer.mix[3].srcChn     = ui->sourceCB_4->currentIndex();
    p_eeGeneral->trainer.mix[3].swtch      = ui->swtchCB_4->currentIndex()-15 ;

    p_eeGeneral->trainer.calib[0] = ui->trainerCalib_1->value();
    p_eeGeneral->trainer.calib[1] = ui->trainerCalib_2->value();
    p_eeGeneral->trainer.calib[2] = ui->trainerCalib_3->value();
    p_eeGeneral->trainer.calib[3] = ui->trainerCalib_4->value();

    p_eeGeneral->PPM_Multiplier = ((quint16)(ui->PPM_MultiplierDSB->value()*10))-10;

    updateSettings();
}

void GeneralEdit::validateWeightSB()
{
    ui->weightSB_1->blockSignals(true);
    ui->weightSB_2->blockSignals(true);
    ui->weightSB_3->blockSignals(true);
    ui->weightSB_4->blockSignals(true);

#ifndef V2
    if ((ui->weightSB_1->value()>StudWeight1) && (p_eeGeneral->trainer.mix[0].studWeight<31))
    {
      p_eeGeneral->trainer.mix[0].studWeight++;
    }
    else if ((ui->weightSB_1->value()<StudWeight1) && (p_eeGeneral->trainer.mix[0].studWeight>-31))
    {
      p_eeGeneral->trainer.mix[0].studWeight--;
    }
    ui->weightSB_1->setValue(p_eeGeneral->trainer.mix[0].studWeight*13/4);
    StudWeight1=ui->weightSB_1->value();
    
    if ((ui->weightSB_2->value()>StudWeight2) && (p_eeGeneral->trainer.mix[1].studWeight<31))
		{
      p_eeGeneral->trainer.mix[1].studWeight++;
    }
		else if ((ui->weightSB_2->value()<StudWeight2) && (p_eeGeneral->trainer.mix[1].studWeight>-31))
		{
      p_eeGeneral->trainer.mix[1].studWeight--;
    }
    ui->weightSB_2->setValue(p_eeGeneral->trainer.mix[1].studWeight*13/4);
    StudWeight2=ui->weightSB_2->value();
 
    if ((ui->weightSB_3->value()>StudWeight3) && (p_eeGeneral->trainer.mix[2].studWeight<31))
		{
      p_eeGeneral->trainer.mix[2].studWeight++;
    }
		else if ((ui->weightSB_3->value()<StudWeight3) && (p_eeGeneral->trainer.mix[2].studWeight>-31))
		{
      p_eeGeneral->trainer.mix[2].studWeight--;
    }
    ui->weightSB_3->setValue(p_eeGeneral->trainer.mix[2].studWeight*13/4);
    StudWeight3=ui->weightSB_3->value();
    
    if ((ui->weightSB_4->value()>StudWeight4) && (p_eeGeneral->trainer.mix[3].studWeight<31))
		{
      p_eeGeneral->trainer.mix[3].studWeight++;
    }
		else if ((ui->weightSB_4->value()<StudWeight4)  && (p_eeGeneral->trainer.mix[3].studWeight>-31))
		{
      p_eeGeneral->trainer.mix[3].studWeight--;
    }
    ui->weightSB_4->setValue(p_eeGeneral->trainer.mix[3].studWeight*13/4);
    StudWeight4=ui->weightSB_4->value();    
#else
    StudWeight1=ui->weightSB_1->value();
    StudWeight2=ui->weightSB_2->value();
    StudWeight3=ui->weightSB_3->value();
    StudWeight4=ui->weightSB_4->value();    
#endif    
		ui->weightSB_1->blockSignals(false);
    ui->weightSB_2->blockSignals(false);
    ui->weightSB_3->blockSignals(false);
    ui->weightSB_4->blockSignals(false);
}

void GeneralEdit::on_contrastSB_editingFinished()
{
    p_eeGeneral->contrast = ui->contrastSB->value();
    updateSettings();
}

void GeneralEdit::on_volumeSB_editingFinished()
{
    p_eeGeneral->volume = ui->volumeSB->value()-7;
    updateSettings();
}

void GeneralEdit::on_battwarningDSB_editingFinished()
{
    p_eeGeneral->vBatWarn = (int)(ui->battwarningDSB->value()*10);
    updateSettings();
}

void GeneralEdit::on_battcalibDSB_editingFinished()
{
    p_eeGeneral->vBatCalib = ui->battcalibDSB->value()*10;
    ui->battCalib->setValue(ui->battcalibDSB->value());
    updateSettings();
}

void GeneralEdit::on_backlightswCB_currentIndexChanged(int index)
{
//	int limit = MAX_DRSWITCH ;
//#ifndef SKY
//  if ( eeFile->mee_type )
//	{
//   	limit += EXTRA_CSW ;
//	}
//#endif
#ifdef V2
		p_eeGeneral->lightSw = rData->getSwitchCbValue( ui->backlightswCB, eeFile->mee_type, 0 ) ;
#else    
		p_eeGeneral->lightSw = getSwitchCbValue( ui->backlightswCB, eeFile->mee_type ) ;
#endif
    updateSettings();
}


void GeneralEdit::on_backlightautoSB_editingFinished()
{
    int i = ui->backlightautoSB->value()/5;
    if((i*5)!=ui->backlightautoSB->value())
        ui->backlightautoSB->setValue(i*5);

    p_eeGeneral->lightAutoOff = i;
    updateSettings();
}

void GeneralEdit::on_backlightStickMove_editingFinished()
{
    int i = ui->backlightStickMove->value()/5;
    if((i*5)!=ui->backlightStickMove->value())
        ui->backlightStickMove->setValue(i*5);

    p_eeGeneral->lightOnStickMove = i;
    updateSettings();
}

void GeneralEdit::on_inactimerSB_editingFinished()
{
    p_eeGeneral->inactivityTimer = ui->inactimerSB->value() - 10;
    updateSettings();
}

void GeneralEdit::on_thrrevChkB_stateChanged(int )
{
    p_eeGeneral->throttleReversed = ui->thrrevChkB->isChecked() ? 1 : 0;
    updateSettings();
}

void GeneralEdit::on_enablePpmsimChkB_stateChanged(int )
{
    p_eeGeneral->enablePpmsim = ui->enablePpmsimChkB->isChecked() ? 1 : 0;
    updateSettings();
}

void GeneralEdit::on_internalFrskyAlarmChkB_stateChanged(int )
{
#ifndef V2
  p_eeGeneral->frskyinternalalarm = ui->internalFrskyAlarmChkB->isChecked() ? 1 : 0;
    updateSettings();
#endif
}
		
void GeneralEdit::on_backlightinvertChkB_stateChanged(int )
{
    p_eeGeneral->blightinv = ui->backlightinvertChkB->isChecked() ? 1 : 0;
    updateSettings();
}

//void GeneralEdit::on_inputfilterCB_currentIndexChanged(int index)
//{
//    p_eeGeneral->filterInput = index;
//    updateSettings();
//}

void GeneralEdit::on_thrwarnChkB_stateChanged(int )
{
    p_eeGeneral->disableThrottleWarning = ui->thrwarnChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_switchwarnChkB_stateChanged(int )
{
    p_eeGeneral->disableSwitchWarning = ui->switchwarnChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_memwarnChkB_stateChanged(int )
{
    p_eeGeneral->disableMemoryWarning = ui->memwarnChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_alarmwarnChkB_stateChanged(int )
{
    p_eeGeneral->disableAlarmWarning = ui->alarmwarnChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_PotScrollEnableChkB_stateChanged(int )
{
    p_eeGeneral->disablePotScroll = ui->PotScrollEnableChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_StickScrollEnableChkB_stateChanged(int )
{
    p_eeGeneral->stickScroll = ui->StickScrollEnableChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_CrossTrimChkB_stateChanged(int )
{
    p_eeGeneral->crosstrim = ui->CrossTrimChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_FrskyPinsChkB_stateChanged(int )
{
    p_eeGeneral->FrskyPins = ui->FrskyPinsChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_TeZ_gt_90ChkB_stateChanged(int )
{
    p_eeGeneral->TEZr90 = ui->TeZ_gt_90ChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_MsoundSerialChkB_stateChanged(int )
{
    p_eeGeneral->MegasoundSerial = ui->MsoundSerialChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}


void GeneralEdit::on_RotateScreenChkB_stateChanged(int )
{
    p_eeGeneral->rotateScreen = ui->RotateScreenChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_SerialLCDChkB_stateChanged(int )
{
    p_eeGeneral->serialLCD = ui->SerialLCDChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_SSD1306ChkB_stateChanged(int )
{
    p_eeGeneral->SSD1306 = ui->SSD1306ChkB->isChecked() ? 1 : 0 ;
    updateSettings();
}

void GeneralEdit::on_BandGapEnableChkB_stateChanged(int )
{
    p_eeGeneral->disableBG = ui->BandGapEnableChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_beeperCB_currentIndexChanged(int index)
{
    p_eeGeneral->beeperVal = index;
    updateSettings();
}

void GeneralEdit::on_channelorderCB_currentIndexChanged(int index)
{
    p_eeGeneral->templateSetup = index;
    updateSettings();
}

void GeneralEdit::on_stickmodeCB_currentIndexChanged(int index)
{
    p_eeGeneral->stickMode = index;
    updateSettings();
}



void GeneralEdit::on_ana1Neg_editingFinished()
{
    p_eeGeneral->calibSpanNeg[0] = ui->ana1Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana2Neg_editingFinished()
{
    p_eeGeneral->calibSpanNeg[1] = ui->ana2Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana3Neg_editingFinished()
{
    p_eeGeneral->calibSpanNeg[2] = ui->ana3Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana4Neg_editingFinished()
{
    p_eeGeneral->calibSpanNeg[3] = ui->ana4Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana5Neg_editingFinished()
{
    p_eeGeneral->calibSpanNeg[4] = ui->ana5Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana6Neg_editingFinished()
{
    p_eeGeneral->calibSpanNeg[5] = ui->ana6Neg->value();
    updateSettings();
}

void GeneralEdit::on_ana7Neg_editingFinished()
{
    p_eeGeneral->calibSpanNeg[6] = ui->ana7Neg->value();
    updateSettings();
}


void GeneralEdit::on_ana1Mid_editingFinished()
{
    p_eeGeneral->calibMid[0] = ui->ana1Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana2Mid_editingFinished()
{
    p_eeGeneral->calibMid[1] = ui->ana2Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana3Mid_editingFinished()
{
    p_eeGeneral->calibMid[2] = ui->ana3Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana4Mid_editingFinished()
{
    p_eeGeneral->calibMid[3] = ui->ana4Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana5Mid_editingFinished()
{
    p_eeGeneral->calibMid[4] = ui->ana5Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana6Mid_editingFinished()
{
    p_eeGeneral->calibMid[5] = ui->ana6Mid->value();
    updateSettings();
}

void GeneralEdit::on_ana7Mid_editingFinished()
{
    p_eeGeneral->calibMid[6] = ui->ana7Mid->value();
    updateSettings();
}


void GeneralEdit::on_ana1Pos_editingFinished()
{
    p_eeGeneral->calibSpanPos[0] = ui->ana1Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana2Pos_editingFinished()
{
    p_eeGeneral->calibSpanPos[1] = ui->ana2Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana3Pos_editingFinished()
{
    p_eeGeneral->calibSpanPos[2] = ui->ana3Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana4Pos_editingFinished()
{
    p_eeGeneral->calibSpanPos[3] = ui->ana4Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana5Pos_editingFinished()
{
    p_eeGeneral->calibSpanNeg[4] = ui->ana5Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana6Pos_editingFinished()
{
    p_eeGeneral->calibSpanNeg[5] = ui->ana6Pos->value();
    updateSettings();
}

void GeneralEdit::on_ana7Pos_editingFinished()
{
    p_eeGeneral->calibSpanPos[6] = ui->ana7Pos->value();
    updateSettings();
}

void GeneralEdit::on_battCalib_editingFinished()
{
    p_eeGeneral->vBatCalib = ui->battCalib->value()*10;
    ui->battcalibDSB->setValue(ui->battCalib->value());
    updateSettings();
}


void GeneralEdit::on_tabWidget_currentChanged(int index)
{
    QSettings settings("er9x-eePe", "eePe");
    settings.setValue("generalEditTab",index);//ui->tabWidget->currentIndex());
}


void GeneralEdit::on_beepMinuteChkB_stateChanged(int )
{
#ifndef V2
    p_eeGeneral->minuteBeep = ui->beepMinuteChkB->isChecked() ? 1 : 0;
    updateSettings();
#endif
}

void GeneralEdit::on_beepCountDownChkB_stateChanged(int )
{
#ifndef V2
    p_eeGeneral->preBeep = ui->beepCountDownChkB->isChecked() ? 1 : 0;
    updateSettings();
#endif
}

void GeneralEdit::on_beepFlashChkB_stateChanged(int )
{
    p_eeGeneral->flashBeep = ui->beepFlashChkB->isChecked() ? 1 : 0;
    updateSettings();
}

void GeneralEdit::on_splashScreenChkB_stateChanged(int )
{
    p_eeGeneral->disableSplashScreen = ui->splashScreenChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::on_ownerNameLE_editingFinished()
{
    memset(&p_eeGeneral->ownerName,' ',sizeof(p_eeGeneral->ownerName));
    for(quint8 i=0; i<(ui->ownerNameLE->text().length()); i++)
    {
        if(i>=sizeof(p_eeGeneral->ownerName)) break;
        p_eeGeneral->ownerName[i] = ui->ownerNameLE->text().toStdString()[i];
    }
    updateSettings();
}

void GeneralEdit::on_rudNameLE_editingFinished()
{
  memset(&p_eeGeneral->customStickNames[0],' ', 4);
	
	for(quint8 i=0; i<4; i++)
  {
		uint8_t x = ui->rudNameLE->text().toStdString()[i] ;
		if ( x == 0 )
		{
			break ;
		}
		p_eeGeneral->customStickNames[i] = x ;
  }
  updateSettings();
}

void GeneralEdit::on_eleNameLE_editingFinished()
{
  memset(&p_eeGeneral->customStickNames[4],' ', 4);
	for(quint8 i=0; i<4; i++)
  {
		uint8_t x = ui->eleNameLE->text().toStdString()[i] ;
		if ( x == 0 )
		{
			break ;
		}
		p_eeGeneral->customStickNames[i+4] = x ;
  }
  updateSettings();
}

void GeneralEdit::on_thrNameLE_editingFinished()
{
  memset(&p_eeGeneral->customStickNames[8],' ', 4);
	for(quint8 i=0; i<4; i++)
  {
		uint8_t x = ui->thrNameLE->text().toStdString()[i] ;
		if ( x == 0 )
		{
			break ;
		}
		p_eeGeneral->customStickNames[i+8] = x ;
  }
  updateSettings();
}

void GeneralEdit::on_ailNameLE_editingFinished()
{
  memset(&p_eeGeneral->customStickNames[12],' ', 4);
	for(quint8 i=0; i<4; i++)
  {
		uint8_t x = ui->ailNameLE->text().toStdString()[i] ;
		if ( x == 0 )
		{
			break ;
		}
		p_eeGeneral->customStickNames[i+12] = x ;
  }
  updateSettings();
}


void GeneralEdit::on_speakerPitchSB_editingFinished()
{
    p_eeGeneral->speakerPitch = ui->speakerPitchSB->value();
    updateSettings();
}

void GeneralEdit::on_hapticStengthSB_editingFinished()
{
    p_eeGeneral->hapticStrength = ui->hapticStengthSB->value();
    updateSettings();
}

void GeneralEdit::on_soundModeCB_currentIndexChanged(int index)
{
	if ( index > 3 )
	{
		index = 7 ;
	}
  p_eeGeneral->speakerMode = index ;
  updateSettings() ;
}

void GeneralEdit::on_tabWidget_selected(QString )
{
    ui->chnLabel_1->setText(getSourceStr(p_eeGeneral->stickMode,1,2));
    ui->chnLabel_2->setText(getSourceStr(p_eeGeneral->stickMode,2,2));
    ui->chnLabel_3->setText(getSourceStr(p_eeGeneral->stickMode,3,2));
    ui->chnLabel_4->setText(getSourceStr(p_eeGeneral->stickMode,4,2));
}



void GeneralEdit::on_splashScreenNameChkB_stateChanged(int )
{
    p_eeGeneral->hideNameOnSplash = ui->splashScreenNameChkB->isChecked() ? 0 : 1;
    updateSettings();
}

void GeneralEdit::getGeneralSwitchDefPos(int i, bool val)
{
#ifndef V2
    if(val)
        p_eeGeneral->switchWarningStates |= (1<<(i-1));
    else
        p_eeGeneral->switchWarningStates &= ~(1<<(i-1));
#endif
}

void GeneralEdit::on_switchDefPos_1_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getGeneralSwitchDefPos(1,ui->switchDefPos_1->isChecked());
    updateSettings();
}
void GeneralEdit::on_switchDefPos_2_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getGeneralSwitchDefPos(2,ui->switchDefPos_2->isChecked());
    updateSettings();
}
void GeneralEdit::on_switchDefPos_3_stateChanged(int )
{
    getGeneralSwitchDefPos(3,ui->switchDefPos_3->isChecked());
    updateSettings();
}
void GeneralEdit::on_switchDefPos_4_stateChanged(int )
{
    if(switchDefPosEditLock) return;

    if(ui->switchDefPos_4->isChecked())
    {
        switchDefPosEditLock = true;
        ui->switchDefPos_5->setChecked(false);
        ui->switchDefPos_6->setChecked(false);
        switchDefPosEditLock = false;
    }
    else
        return;

#ifndef V2
    p_eeGeneral->switchWarningStates &= ~0x30; //turn off ID1/2
    getGeneralSwitchDefPos(4,ui->switchDefPos_4->isChecked());
#endif
    updateSettings();
}
void GeneralEdit::on_switchDefPos_5_stateChanged(int )
{
    if(switchDefPosEditLock) return;

    if(ui->switchDefPos_5->isChecked())
    {
        switchDefPosEditLock = true;
        ui->switchDefPos_4->setChecked(false);
        ui->switchDefPos_6->setChecked(false);
        switchDefPosEditLock = false;
    }
    else
        return;

#ifndef V2
    p_eeGeneral->switchWarningStates &= ~0x28; //turn off ID0/2
    getGeneralSwitchDefPos(5,ui->switchDefPos_5->isChecked());
#endif
    updateSettings();
}
void GeneralEdit::on_switchDefPos_6_stateChanged(int )
{
    if(switchDefPosEditLock) return;

#ifndef V2
    if(ui->switchDefPos_6->isChecked())
    {
        switchDefPosEditLock = true;
        ui->switchDefPos_4->setChecked(false);
        ui->switchDefPos_5->setChecked(false);
        switchDefPosEditLock = false;
    }
    else
        return;

    p_eeGeneral->switchWarningStates &= ~0x18; //turn off ID1/2
    getGeneralSwitchDefPos(6,ui->switchDefPos_6->isChecked());
    updateSettings();
#endif
}
void GeneralEdit::on_switchDefPos_7_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getGeneralSwitchDefPos(7,ui->switchDefPos_7->isChecked());
    updateSettings();
}
void GeneralEdit::on_switchDefPos_8_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getGeneralSwitchDefPos(8,ui->switchDefPos_8->isChecked());
    updateSettings();
}

void GeneralEdit::on_StickRevLH_stateChanged(int )
{
	p_eeGeneral->stickReverse &= ~0xF1 ;
	if (ui->StickRevLH->isChecked() )
	{
		p_eeGeneral->stickReverse |= 0x01 ;
	}
  updateSettings();
}

void GeneralEdit::on_StickRevLV_stateChanged(int )
{
	p_eeGeneral->stickReverse &= ~0xF2 ;
	if (ui->StickRevLV->isChecked() )
	{
		p_eeGeneral->stickReverse |= 0x02 ;
	}
  updateSettings();
}

void GeneralEdit::on_StickRevRV_stateChanged(int )
{
	p_eeGeneral->stickReverse &= ~0xF4 ;
	if (ui->StickRevRV->isChecked() )
	{
		p_eeGeneral->stickReverse |= 0x04 ;
	}
  updateSettings();
}

void GeneralEdit::on_StickRevRH_stateChanged(int )
{
	p_eeGeneral->stickReverse &= ~0xF8 ;
	if (ui->StickRevRH->isChecked() )
	{
		p_eeGeneral->stickReverse |= 0x08 ;
	}
  updateSettings();
}

void GeneralEdit::on_EleSwitchSource_currentIndexChanged(int index)
{
#ifdef V2
	uint8_t x = p_eeGeneral->switchSources[1] & 0xF0 ;
	x |= index & 0x0F ;
	p_eeGeneral->switchSources[1] = x ;
#else
	p_eeGeneral->ele2source = index ;
	p_eeGeneral->switchMapping &= ~USE_ELE_3POS ;
	if ( index )
	{
		p_eeGeneral->switchMapping |= USE_ELE_3POS ;
	}
	createSwitchMapping( p_eeGeneral, eeFile->mee_type ) ;
#endif
  updateSettings();
}

void GeneralEdit::on_AilSwitchSource_currentIndexChanged(int index)
{
#ifdef V2
	uint8_t x = p_eeGeneral->switchSources[1] & 0x0F ;
	x |= ( index << 4 ) & 0xF0 ;
	p_eeGeneral->switchSources[1] = x ;
#else
	p_eeGeneral->ail2source = index ;
	p_eeGeneral->switchMapping &= ~USE_AIL_3POS ;
	if ( index )
	{
		p_eeGeneral->switchMapping |= USE_AIL_3POS ;
	}
	createSwitchMapping( p_eeGeneral, eeFile->mee_type ) ;
#endif
  updateSettings();
}

void GeneralEdit::on_ThrSwitchSource_currentIndexChanged(int index)
{
#ifdef V2
	uint8_t x = p_eeGeneral->switchSources[0] & 0xF0 ;
	x |= index & 0x0F ;
	p_eeGeneral->switchSources[0] = x ;
#else
	p_eeGeneral->thr2source = index ;
	p_eeGeneral->switchMapping &= ~USE_THR_3POS ;
	if ( index )
	{
		p_eeGeneral->switchMapping |= USE_THR_3POS ;
	}
	createSwitchMapping( p_eeGeneral, eeFile->mee_type ) ;
#endif
  updateSettings();
}

void GeneralEdit::on_RudSwitchSource_currentIndexChanged(int index)
{
#ifdef V2
	uint8_t x = p_eeGeneral->switchSources[0] & 0x0F ;
	x |= ( index << 4 ) & 0xF0 ;
	p_eeGeneral->switchSources[0] = x ;
#else
	p_eeGeneral->rud2source = index ;
	p_eeGeneral->switchMapping &= ~USE_RUD_3POS ;
	if ( index )
	{
		p_eeGeneral->switchMapping |= USE_RUD_3POS ;
	}
	createSwitchMapping( p_eeGeneral, eeFile->mee_type ) ;
#endif
  updateSettings();
}

void GeneralEdit::on_GeaSwitchSource_currentIndexChanged(int index)
{
#ifdef V2
	uint8_t x = p_eeGeneral->switchSources[2] & 0xF0 ;
	x |= index & 0x0F ;
	p_eeGeneral->switchSources[2] = x ;
#else
	p_eeGeneral->gea2source = index ;
	p_eeGeneral->switchMapping &= ~USE_GEA_3POS ;
	if ( index )
	{
		p_eeGeneral->switchMapping |= USE_GEA_3POS ;
	}
	createSwitchMapping( p_eeGeneral, eeFile->mee_type ) ;
#endif
  updateSettings();
}


void GeneralEdit::on_Pb1SwitchSource_currentIndexChanged(int index)
{
#ifdef V2
	uint8_t x = p_eeGeneral->switchSources[2] & 0x0F ;
	x |= ( index << 4 ) & 0xF0 ;
	p_eeGeneral->switchSources[2] = x ;
#else
	p_eeGeneral->pb1source = index ;
	p_eeGeneral->switchMapping &= ~USE_PB1 ;
	if ( index )
	{
		p_eeGeneral->switchMapping |= USE_PB1 ;
	}
	createSwitchMapping( p_eeGeneral, eeFile->mee_type ) ;
#endif
  updateSettings();
}

void GeneralEdit::on_Pb2SwitchSource_currentIndexChanged(int index)
{
#ifdef V2
	uint8_t x = p_eeGeneral->switchSources[3] & 0xF0 ;
	x |= index & 0x0F ;
	p_eeGeneral->switchSources[3] = x ;
#else
	p_eeGeneral->pb2source = index ;
	p_eeGeneral->switchMapping &= ~USE_PB2 ;
	if ( index )
	{
		p_eeGeneral->switchMapping |= USE_PB2 ;
	}
	createSwitchMapping( p_eeGeneral, eeFile->mee_type ) ;
#endif
  updateSettings();
}

void GeneralEdit::on_Pb7InputCB_stateChanged(int x )
{
#ifndef V2
	p_eeGeneral->pb7Input = x ;
#endif
  updateSettings();
}

void GeneralEdit::on_Pg2InputCB_stateChanged(int x )
{
#ifndef V2
	p_eeGeneral->pg2Input = x ;
#endif
  updateSettings();
}

void GeneralEdit::on_L_wrInputCB_stateChanged(int x )
{
#ifndef V2
	p_eeGeneral->lcd_wrInput = x ;
#endif
  updateSettings();
}

void GeneralEdit::on_LvTrimModCB_stateChanged(int x )
{
#ifdef V2
  p_eeGeneral->LVTrimMod = x ;
  updateSettings();
#endif
}

void GeneralEdit::on_PB7BacklightCB_stateChanged(int x )
{
#ifdef V2
  p_eeGeneral->pb7backlight = x ;
  updateSettings();
#endif
}

void GeneralEdit::on_StickLVdeadbandSB_editingFinished()
{
  uint16_t db = ui->StickLVdeadbandSB->value() ;
  p_eeGeneral->stickDeadband = ( p_eeGeneral->stickDeadband & 0xFF0F ) | ( db << 4 ) ;
  updateSettings();
}

void GeneralEdit::on_StickLHdeadbandSB_editingFinished()
{
  uint16_t db = ui->StickLVdeadbandSB->value() ;
  p_eeGeneral->stickDeadband = ( p_eeGeneral->stickDeadband & 0xFFF0 ) | db ;
  updateSettings();
}

void GeneralEdit::on_StickRVdeadbandSB_editingFinished()
{
  uint16_t db = ui->StickLVdeadbandSB->value() ;
  p_eeGeneral->stickDeadband = ( p_eeGeneral->stickDeadband & 0xF0FF ) | ( db << 8 ) ;
  updateSettings();
}

void GeneralEdit::on_StickRHdeadbandSB_editingFinished()
{
  uint16_t db = ui->StickLVdeadbandSB->value() ;
  p_eeGeneral->stickDeadband = ( p_eeGeneral->stickDeadband & 0x0FFF ) | ( db << 12 ) ;
  updateSettings();
}






