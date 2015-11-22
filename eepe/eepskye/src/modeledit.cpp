#include "modeledit.h"
#include "ui_modeledit.h"
#include "pers.h"
#include "file.h"
#include "helpers.h"
#include "../../common/edge.h"
#include "../../common/node.h"
#include "mixerdialog.h"
#include "simulatordialog.h"
#include "VoiceAlarmDialog.h"
#include "TemplateDialog.h"

#include <QtGui>

#define BC_BIT_RUD (0x01)
#define BC_BIT_ELE (0x02)
#define BC_BIT_THR (0x04)
#define BC_BIT_AIL (0x08)
#define BC_BIT_P1  (0x10)
#define BC_BIT_P2  (0x20)
#define BC_BIT_P3  (0x40)

#define RUD  (1)
#define ELE  (2)
#define THR  (3)
#define AIL  (4)

#define GFX_MARGIN 16

#define ALARM_GREATER(channel, alarm) ((g_model.frsky.channels[channel].alarms_greater >> alarm) & 1)
#define ALARM_LEVEL(channel, alarm) ((g_model.frsky.channels[channel].alarms_level >> (2*alarm)) & 3)

extern class simulatorDialog *SimPointer ;

int GlobalModified = 0 ;
EEGeneral Sim_g ;
int GeneralDataValid = 0 ;
SKYModelData Sim_m ;
int ModelDataValid = 0 ;

ModelEdit::ModelEdit( struct t_radioData *radioData, uint8_t id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModelEdit)
{
//		int size ;
    ui->setupUi(this);

    sdptr = 0;

    switchEditLock = false;
    heliEditLock = false;
    protocolEditLock = false;
    switchDefPosEditLock = false;

//    if(!eeFile->eeLoadGeneral())  eeFile->generalDefault();
    
//		eeFile->getGeneralSettings(&g_eeGeneral);

    rData = radioData ;
		memcpy(  &g_eeGeneral, &radioData->generalSettings, sizeof( g_eeGeneral) ) ;
		
		memcpy(  &g_model, &radioData->models[id], sizeof( g_model) ) ;
//		size = eeFile->getModel(&g_model,id);
//		if ( size < sizeof(g_model) )
//		{
//			uint8_t *p ;
//			p = (uint8_t *) &g_model + size ;
//			while( size < sizeof(g_model) )
//			{
//				*p++ = 0 ;
//				size += 1 ;
//			}
//		}
    id_model = id;

		createSwitchMapping( &g_eeGeneral, ( ( rData->type == 1 ) || ( rData->type == 2 ) ) ? MAX_XDRSWITCH : MAX_DRSWITCH, rData->type ) ;
    setupMixerListWidget();

    QSettings settings("er9x-eePskye", "eePskye");
    ui->tabWidget->setCurrentIndex(settings.value("modelEditTab", 0).toInt());

    QRegExp rx(CHAR_FOR_NAMES_REGEX);
    ui->modelNameLE->setValidator(new QRegExpValidator(rx, this));
    ui->modelImageLE->setValidator(new QRegExpValidator(rx, this));

//		ModelVersion = g_model.modelVersion ;

		switchesTabDone = false ;

    tabModelEditSetup();
    tabExpo();
    tabMixes();
    tabLimits();
    tabCurves();
    tabSwitches();
    tabSafetySwitches();
    tabTrims();
    tabFrsky();
    tabTemplates();
    tabHeli();
    tabPhase();
		tabGvar();
		tabVoiceAlarms() ;

    ui->curvePreview->setMinimumWidth(260);
    ui->curvePreview->setMinimumHeight(260);

    resizeEvent();  // draws the curves and Expo

}

void ModelEdit::textUpdate( QLineEdit *source, char *dest, int length )
{
    memset( dest,' ', length ) ;
    QString str = source->text().left(10).toLatin1() ;

    for(quint8 i=0; i<(str.length()); i++)
    {
      if(i>= length) break ;
      dest[i] = (char)str.data()[i].toLatin1() ;
    }
    for(int i=0; i<length; i++) if(!dest[i]) dest[i] = ' ';
}


ModelEdit::~ModelEdit()
{
    delete ui;
}

void ModelEdit::setupMixerListWidget()
{
    MixerlistWidget = new MixersList(this);
    QPushButton * qbUp = new QPushButton(this);
    QPushButton * qbDown = new QPushButton(this);
    QPushButton * qbClear = new QPushButton(this);

    qbUp->setText("Move Up");
    qbUp->setIcon(QIcon(":/images/moveup.png"));
    qbUp->setShortcut(QKeySequence(tr("Ctrl+Up")));
    qbDown->setText("Move Down");
    qbDown->setIcon(QIcon(":/images/movedown.png"));
    qbDown->setShortcut(QKeySequence(tr("Ctrl+Down")));
    qbClear->setText("Clear Mixes");
    qbClear->setIcon(QIcon(":/images/clear.png"));

    ui->mixersLayout->addWidget(MixerlistWidget,1,1,1,3);
    ui->mixersLayout->addWidget(qbUp,2,1);
    ui->mixersLayout->addWidget(qbClear,2,2);
    ui->mixersLayout->addWidget(qbDown,2,3);

    connect(MixerlistWidget,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(mixerlistWidget_customContextMenuRequested(QPoint)));
    connect(MixerlistWidget,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(mixerlistWidget_doubleClicked(QModelIndex)));
    connect(MixerlistWidget,SIGNAL(mimeDropped(int,const QMimeData*,Qt::DropAction)),this,SLOT(mimeDropped(int,const QMimeData*,Qt::DropAction)));

    connect(qbUp,SIGNAL(pressed()),SLOT(moveMixUp()));
    connect(qbDown,SIGNAL(pressed()),SLOT(moveMixDown()));
    connect(qbClear,SIGNAL(pressed()),SLOT(clearMixes()));

    connect(MixerlistWidget,SIGNAL(keyWasPressed(QKeyEvent*)), this, SLOT(mixerlistWidget_KeyPress(QKeyEvent*)));
}

void ModelEdit::resizeEvent(QResizeEvent *event)
{

    if(ui->curvePreview->scene())
    {
        QRect qr = ui->curvePreview->contentsRect();
        ui->curvePreview->scene()->setSceneRect(GFX_MARGIN, GFX_MARGIN, qr.width()-GFX_MARGIN*2, qr.height()-GFX_MARGIN*2);
        drawCurve();
    }

    QDialog::resizeEvent(event);

}

void ModelEdit::applyBaseTemplate()
{
    clearMixes(false);
    applyTemplate(0);
    updateSettings();
    tabMixes();
}

void ModelEdit::updateSettings()
{
//    eeFile->putModel(&g_model,id_model);
    memcpy( &rData->models[id_model], &g_model, sizeof( g_model) ) ;
    rData->File_system[id_model+1].size = sizeof( g_model) ;

    emit modelValuesChanged(this);
    
		memcpy(&Sim_g, &g_eeGeneral,sizeof(EEGeneral));
    memcpy(&Sim_m,&g_model,sizeof(SKYModelData));
		GeneralDataValid = 1 ;
		ModelDataValid = 1 ;
		GlobalModified = 1 ;
}

void ModelEdit::on_tabWidget_currentChanged(int index)
{
    QSettings settings("er9x-eePskye", "eePskye");
    settings.setValue("modelEditTab",index);//ui->tabWidget->currentIndex());
}


void ModelEdit::tabModelEditSetup()
{
    //name
		QString n = g_model.name ;

		while ( n.endsWith(" ") )
		{
			n = n.left(n.size()-1) ;			
		}
    ui->modelNameLE->setText( n ) ;

		n = g_model.modelImageName ;

		while ( n.endsWith(" ") )
		{
			n = n.left(n.size()-1) ;			
		}
    ui->modelImageLE->setText( n ) ;

		if ( ( rData->type == 0 ) || ( rData->type == 3 ) )
		{
			ui->modelImageLE->hide() ;
		}
		else
		{
			ui->modelImageLE->show() ;
		}

		ui->VoiceNumberSB->setValue(g_model.modelVoice+260) ;
    //timer mode direction value
    populateTimerSwitchCB(ui->timerModeCB,g_model.timer[0].tmrModeA);
    populateTmrBSwitchCB(ui->timerModeBCB,g_model.timer[0].tmrModeB, rData->type);
    populateTmrBSwitchCB(ui->timerResetCB,g_model.timer1RstSw, rData->type);
    populateTimerSwitchCB(ui->timer2ModeCB,g_model.timer[1].tmrModeA);
    populateTmrBSwitchCB(ui->timer2ModeBCB,g_model.timer[1].tmrModeB, rData->type);
    populateTmrBSwitchCB(ui->timer2ResetCB,g_model.timer2RstSw, rData->type);
    int min = g_model.timer[0].tmrVal/60;
    int sec = g_model.timer[0].tmrVal%60;
    ui->timerValTE->setTime(QTime(0,min,sec));
    min = g_model.timer[1].tmrVal/60;
    sec = g_model.timer[1].tmrVal%60;
    ui->timer2ValTE->setTime(QTime(0,min,sec));
    ui->timer1BeepCdownCB->setChecked(g_model.timer1Cdown);
    ui->timer2BeepCdownCB->setChecked(g_model.timer2Cdown);
    ui->timer1MinuteBeepCB->setChecked(g_model.timer1Mbeep);
    ui->timer2MinuteBeepCB->setChecked(g_model.timer2Mbeep);

    ui->AutoBtConnectChkB->setChecked(g_model.autoBtConnect) ;
    ui->UseStickNamesChkB->setChecked(g_model.useCustomStickNames) ;

		ui->BtDefaultAddrSB->setValue(g_model.btDefaultAddress) ;

		customAlarmLock = true ;
		populateCustomAlarmCB( ui->CustomAlarmSourceCB, rData->type ) ;
    ui->CustomAlarmSourceCB->setCurrentIndex(g_model.customCheck.source );
		ui->CustomAlarmMinSB->setValue(g_model.customCheck.min ) ;
    ui->CustomAlarmMaxSB->setValue(g_model.customCheck.max ) ;
		customAlarmLock = false ;

    //trim inc, thro trim, thro expo, instatrim
    ui->trimIncCB->setCurrentIndex(g_model.trimInc);
    populateSwitchCB(ui->trimSWCB,g_model.trimSw, rData->type);
    ui->thrExpoChkB->setChecked(g_model.thrExpo);
    ui->thrTrimChkB->setChecked(g_model.thrTrim);
    ui->timerDirCB->setCurrentIndex(g_model.timer[0].tmrDir);
    ui->timer2DirCB->setCurrentIndex(g_model.timer[1].tmrDir);
    ui->TrainerChkB->setChecked(g_model.traineron);
//    ui->T2ThrTrgChkB->setChecked(g_model.t2throttle);

    //center beep
    ui->bcRUDChkB->setChecked(g_model.beepANACenter & BC_BIT_RUD);
    ui->bcELEChkB->setChecked(g_model.beepANACenter & BC_BIT_ELE);
    ui->bcTHRChkB->setChecked(g_model.beepANACenter & BC_BIT_THR);
    ui->bcAILChkB->setChecked(g_model.beepANACenter & BC_BIT_AIL);
    ui->bcP1ChkB->setChecked(g_model.beepANACenter & BC_BIT_P1);
    ui->bcP2ChkB->setChecked(g_model.beepANACenter & BC_BIT_P2);
    ui->bcP3ChkB->setChecked(g_model.beepANACenter & BC_BIT_P3);

    ui->extendedLimitsChkB->setChecked(g_model.extendedLimits);

    ui->autoLimitsSB->setValue( (double)g_model.sub_trim_limit/10 + 0.049 ) ;
    
		protocolEditLock = true ;

		ui->protocolCB->clear() ;
		ui->xprotocolCB->clear() ;
    ui->protocolCB->addItem("PPM");
    ui->xprotocolCB->addItem("PPM");
    ui->protocolCB->addItem("PXX");
    ui->xprotocolCB->addItem("PXX");
    ui->protocolCB->addItem("DSM");
    ui->xprotocolCB->addItem("DSM");
    ui->protocolCB->addItem("Multi");
    ui->xprotocolCB->addItem("Multi");
    ui->protocolCB->addItem("Assan");
    ui->xprotocolCB->addItem("Assan");
		if ( rData->type )
		{
      ui->protocolCB->addItem("OFF");
      ui->xprotocolCB->addItem("OFF");
		}
    //protocol channels ppm delay (disable if needed)
    protocolEditLock = false ;
    setProtocolBoxes();
		
		populateAnaVolumeCB( ui->volumeControlCB, g_model.anaVolume, rData->type ) ;
	  
    //pulse polarity
    ui->pulsePolCB->setCurrentIndex(g_model.pulsePol);
    ui->xpulsePolCB->setCurrentIndex(g_model.xpulsePol);
		ui->countryCB->setCurrentIndex(g_model.country) ;
	  ui->typeCB->setCurrentIndex(g_model.sub_protocol) ;
		
		
		ui->label_version->setText( tr("%1").arg( g_model.modelVersion ) ) ;
		ui->updateButton->setVisible( g_model.modelVersion < 2 ) ;
		ui->updateButton3->setVisible( g_model.modelVersion < 3 ) ;

    ui->switchwarnChkB->setChecked(!(g_model.modelswitchWarningStates&1)); //Default is zero=checked

    setSwitchDefPos() ;

		if ( ( rData->type ) && ( rData->type < 3 ) )
		{
			ui->switchDefPos_1->hide() ;
			ui->switchDefPos_2->hide() ;
			ui->switchDefPos_3->hide() ;
			ui->switchDefPos_4->hide() ;
			ui->switchDefPos_5->hide() ;
			ui->switchDefPos_6->hide() ;
			ui->switchDefPos_7->hide() ;
			ui->switchDefPos_8->hide() ;
			ui->widgetDefSA->show() ;
			ui->widgetDefSB->show() ;
			ui->widgetDefSC->show() ;
			ui->widgetDefSD->show() ;
			ui->widgetDefSE->show() ;
			ui->widgetDefSF->show() ;
			ui->widgetDefSG->show() ;
			ui->EnThr->hide() ;
			ui->EnEle->hide() ;
			ui->EnRud->hide() ;
			ui->EnIdx->hide() ;
			ui->EnAil->hide() ;
			ui->EnGea->hide() ;
			ui->EnA->show() ;
			ui->EnB->show() ;
			ui->EnC->show() ;
			ui->EnD->show() ;
			ui->EnE->show() ;
			ui->EnF->show() ;
			ui->EnG->show() ;
		}
		else
		{
			if ( g_eeGeneral.switchMapping & USE_THR_3POS )
			{
				ui->widgetDefSA->show() ;
				ui->switchDefPos_1->hide() ;
				ui->label_DSA->setText("THR") ;
			}
			else
			{
				ui->switchDefPos_1->show() ;
				ui->widgetDefSA->hide() ;
			}
			
			if ( g_eeGeneral.switchMapping & USE_RUD_3POS )
			{
				ui->widgetDefSB->show() ;
				ui->switchDefPos_2->hide() ;
				ui->label_DSB->setText("RUD") ;
			}
			else
			{
				ui->switchDefPos_2->show() ;
				ui->widgetDefSB->hide() ;
			}
				 
			if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
			{
				ui->widgetDefSC->show() ;
				ui->switchDefPos_3->hide() ;
				ui->label_DSC->setText("ELE") ;
			}
			else
			{
				ui->switchDefPos_3->show() ;
				ui->widgetDefSC->hide() ;
			}

			if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
			{
				ui->widgetDefSD->show() ;
				ui->switchDefPos_7->hide() ;
				ui->label_DSD->setText("AIL") ;
			}
			else
			{
				ui->switchDefPos_7->show() ;
				ui->widgetDefSD->hide() ;
			}

			if ( g_eeGeneral.switchMapping & USE_GEA_3POS )
			{
				ui->widgetDefSE->show() ;
				ui->switchDefPos_8->hide() ;
				ui->label_DSE->setText("GEA") ;
			}
			else
			{
				ui->switchDefPos_8->show() ;
				ui->widgetDefSE->hide() ;
			}
			
			ui->switchDefPos_4->show() ;
			ui->switchDefPos_5->show() ;
			ui->switchDefPos_6->show() ;
			ui->widgetDefSF->hide() ;
			ui->widgetDefSG->hide() ;
		
			ui->EnThr->show() ;
			ui->EnEle->show() ;
			ui->EnRud->show() ;
			ui->EnIdx->show() ;
			ui->EnAil->show() ;
			ui->EnGea->show() ;
			ui->EnA->hide() ;
			ui->EnB->hide() ;
			ui->EnC->hide() ;
			ui->EnD->hide() ;
			ui->EnE->hide() ;
			ui->EnF->hide() ;
			ui->EnG->hide() ;
		}


}

uint16_t ModelEdit::oneSwitchPos( uint8_t swtch, uint16_t states )
{
	uint8_t index = 0 ;
	uint8_t sm = g_eeGeneral.switchMapping ;

	switch ( swtch )
	{
		case HSW_ThrCt :
			if ( sm & USE_THR_3POS )
			{
				if ( states & 0x0001 ) index += 1 ;
				if ( states & 0x0100 ) index += 2 ;
			}
			else
			{
				if (states & 0x0101) index = 1 ;
			}
		break ;

		case HSW_RuddDR :
			if ( sm & USE_RUD_3POS )
			{
				if ( states & 0x0002 ) index += 1 ;
				if ( states & 0x0200 ) index += 2 ;
			}
			else
			{
				if (states & 0x0202) index = 1 ;
			}
		break ;
	
		case HSW_ElevDR :
			if ( sm & USE_ELE_3POS )
			{
				if ( states & 0x0004 ) index += 1 ;
				if ( states & 0x0400 ) index += 2 ;
			}
			else
			{
				if (states & 0x0C04) index = 1 ;
			}
		break ;

		case HSW_ID0 :
			if ( states & 0x10 )
			{
				index += 1 ;
			}
			if ( states & 0x20 )
			{
				index += 2 ;
			}
		break ;

		case HSW_AileDR :
			if ( sm & USE_AIL_3POS )
			{
				if ( states & 0x0040 ) index += 1 ;
				if ( states & 0x1000 ) index += 2 ;
			}
			else
			{
				if (states & 0x1040) index = 1 ;
			}
		break ;
	 
		case HSW_Gear :
			if ( sm & USE_GEA_3POS )
			{
				if ( states & 0x0080 ) index += 1 ;
				if ( states & 0x2000 ) index += 2 ;
			}
			else
			{
				if (states & 0x2080) index = 1 ;
			}
		break ;

	}
	return index ;
}


void ModelEdit::setSwitchDefPos()
{
	if ( ( rData->type == 0 ) || ( rData->type == 3 ) )
	{
		
    quint16 y = (g_model.modelswitchWarningStates >> 1 ) ;
    quint16 x = y & SWP_IL5 ;
    if(x==SWP_IL1 || x==SWP_IL2 || x==SWP_IL3 || x==SWP_IL4 || x==SWP_IL5) //illegal states for ID0/1/2
    {
      x &= ~SWP_IL5; // turn all off, make sure only one is on
      x |=  SWP_ID0B;
    }

    y &= ~SWP_IL5 ;
		x |= y ;
		 
		g_model.modelswitchWarningStates = ( x << 1 ) | (g_model.modelswitchWarningStates & 1 ) ;

    switchDefPosEditLock = true;
		if ( g_eeGeneral.switchMapping & USE_THR_3POS )
		{
    	ui->SwitchDefSA->setValue( oneSwitchPos( HSW_ThrCt, x ) ) ;
		}
		else
		{
    	ui->switchDefPos_1->setChecked(x & 0x01);
		}
		
		if ( g_eeGeneral.switchMapping & USE_RUD_3POS )
		{
    	ui->SwitchDefSB->setValue( oneSwitchPos( HSW_RuddDR, x ) ) ;
		}
		else
		{
	    ui->switchDefPos_2->setChecked(x & 0x02);
		}

		if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
		{
    	ui->SwitchDefSC->setValue( oneSwitchPos( HSW_ElevDR, x ) ) ;
		}
		else
		{
    	ui->switchDefPos_3->setChecked(x & 0x04);
		}
    ui->switchDefPos_4->setChecked(x & 0x08);
    ui->switchDefPos_5->setChecked(x & 0x10);
    ui->switchDefPos_6->setChecked(x & 0x20);
    
		if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
		{
    	ui->SwitchDefSD->setValue( oneSwitchPos( HSW_AileDR, x ) ) ;
		}
		else
		{
			ui->switchDefPos_7->setChecked(x & 0x40);
		}
		
		if ( g_eeGeneral.switchMapping & USE_GEA_3POS )
		{
      ui->SwitchDefSE->setValue( oneSwitchPos( HSW_Gear, x ) ) ;
		}
		else
		{
    	ui->switchDefPos_8->setChecked(x & 0x80);
		}

#define THR_WARN_MASK	0x0101
#define RUD_WARN_MASK	0x0202
#define ELE_WARN_MASK	0x0C04
#define IDX_WARN_MASK	0x0038
#define AIL_WARN_MASK	0x1040
#define GEA_WARN_MASK	0x2080
		uint16_t temp = g_model.modelswitchWarningDisables ;

		ui->EnThr->setChecked( !( temp & THR_WARN_MASK ) ) ;
		ui->EnRud->setChecked( !( temp & RUD_WARN_MASK ) ) ;
		ui->EnEle->setChecked( !( temp & ELE_WARN_MASK ) ) ;
		ui->EnIdx->setChecked( !( temp & IDX_WARN_MASK ) ) ;
		ui->EnAil->setChecked( !( temp & AIL_WARN_MASK ) ) ;
		ui->EnGea->setChecked( !( temp & GEA_WARN_MASK ) ) ;

		switchDefPosEditLock = false;
	}
	else
	{
    quint16 y = (g_model.modelswitchWarningStates >> 1 ) ;
    switchDefPosEditLock = true;
    ui->SwitchDefSA->setValue(y & 0x03) ;
		y >>= 2 ;
    ui->SwitchDefSB->setValue(y & 0x03) ;
		y >>= 2 ;
    ui->SwitchDefSC->setValue(y & 0x03) ;
		y >>= 2 ;
    ui->SwitchDefSD->setValue(y & 0x03) ;
		y >>= 2 ;
    ui->SwitchDefSE->setValue(y & 0x03) ;
		y >>= 2 ;
    ui->SwitchDefSF->setValue( (y & 0x03) ? 1 : 0 ) ;
		y >>= 2 ;
    ui->SwitchDefSG->setValue(y & 0x03) ;
		
		uint16_t temp = g_model.modelswitchWarningDisables ;
		ui->EnA->setChecked( !( temp & 0x0003 ) ) ;
		ui->EnB->setChecked( !( temp & 0x000C ) ) ;
		ui->EnC->setChecked( !( temp & 0x0030 ) ) ;
		ui->EnD->setChecked( !( temp & 0x00C0 ) ) ;
		ui->EnE->setChecked( !( temp & 0x0300 ) ) ;
		ui->EnF->setChecked( !( temp & 0x0C00 ) ) ;
		ui->EnG->setChecked( !( temp & 0x3000 ) ) ;

    switchDefPosEditLock = false;
	}

}


uint8_t stickScramble[]=
{
  0, 1, 2, 3,
  0, 2, 1, 3,
  3, 1, 2, 0,
  3, 2, 1, 0
} ;

void ModelEdit::updateToMV2()
{
	if ( g_model.modelVersion < 2 )
	{
		modelConvert1to2( &g_eeGeneral, &g_model ) ;
    tabSwitches();
    tabMixes();
		ui->label_version->setText( tr("%1").arg( g_model.modelVersion ) ) ;
    updateSettings();
	}
	ui->updateButton->setVisible( false ) ;
}

void ModelEdit::updateToMV3()
{
	updateToMV2() ;
	if ( g_model.modelVersion < 3 )
	{
		for (uint8_t i = 0 ; i < NUM_SKYCSW ; i += 1 )
		{
      SKYCSwData *cs = &g_model.customSw[i];
			if ( cs->func == CS_LATCH )
			{
				cs->func = CS_GREATER ;
			}
			if ( cs->func == CS_FLIP )
			{
				cs->func = CS_LESS ;
			}
		}
		g_model.modelVersion = 3 ;
	  updateSettings() ;
    tabSwitches();

//		for (uint8_t i = 0 ; i < NUM_CSW+EXTRA_CSW ; i += 1 )
//		{
//      setSwitchWidgetVisibility(i) ;
//		}
		ui->label_version->setText( tr("%1").arg( g_model.modelVersion ) ) ;
	}
	ui->updateButton3->setVisible( false ) ;
}

void ModelEdit::setProtocolBoxes()
{
    protocolEditLock = true;
		int i = g_model.protocol ;
		if ( i == PROTO_OFF )
		{
			i = 5 ;
		}
    ui->protocolCB->setCurrentIndex(i);
		i = g_model.xprotocol ;
		if ( i == PROTO_OFF )
		{
			i = 5 ;
		}
    ui->xprotocolCB->setCurrentIndex(i);

		if ( rData->type )
		{
      ui->xppmDelaySB->show() ;
      ui->xnumChannelsSB->show() ;
      ui->xppmFrameLengthDSB->show() ;
      ui->xDSM_Type->show() ;
      ui->xPxxRxNum->show() ;
      ui->xppmDelaySB->show() ;
      ui->xnumChannelsSB->show() ;
      ui->xppmFrameLengthDSB->show() ;
			ui->xtypeCB->show() ;
			ui->xstartChannelsSB->show() ;
      ui->xcountryCB->show() ;
			ui->xprotocolCB->show() ;
			ui->xpulsePolCB->show() ;
			ui->labelProtoExt->show() ;
			ui->labelxp1->show() ;
			ui->labelxp2->show() ;
			ui->labelxp3->show() ;
			ui->labelxp4->show() ;
			ui->labelxp5->show() ;
			ui->labelxp6->show() ;
			ui->labelxp7->show() ;
			ui->labelxp8->show() ;
		}
		else
		{
      ui->xppmDelaySB->hide() ;
      ui->xnumChannelsSB->hide() ;
      ui->xppmFrameLengthDSB->hide() ;
      ui->xDSM_Type->hide() ;
      ui->xPxxRxNum->hide() ;
      ui->xppmDelaySB->hide() ;
      ui->xnumChannelsSB->hide() ;
      ui->xppmFrameLengthDSB->hide() ;
			ui->xtypeCB->hide() ;
			ui->xstartChannelsSB->hide() ;
      ui->xcountryCB->hide() ;
			ui->xprotocolCB->hide() ;
			ui->xpulsePolCB->hide() ;
			ui->labelProtoExt->hide() ;
			ui->labelxp1->hide() ;
			ui->labelxp2->hide() ;
			ui->labelxp3->hide() ;
			ui->labelxp4->hide() ;
			ui->labelxp5->hide() ;
			ui->labelxp6->hide() ;
			ui->labelxp7->hide() ;
			ui->labelxp8->hide() ;
		}

    switch (g_model.protocol)
    {
    case (PROTO_PXX):
        ui->ppmDelaySB->setEnabled(false);
        ui->numChannelsSB->setEnabled(false);
        ui->ppmFrameLengthDSB->setEnabled(false);
        ui->DSM_Type->hide() ;
        ui->SubProtocolCB->hide() ;
        ui->pxxRxNum->setEnabled(true);
        ui->countryCB->setEnabled(true);
        ui->typeCB->setEnabled(true);
				ui->startChannelsSB->setEnabled(true);
				ui->pulsePolCB->setEnabled(false);

        ui->pxxRxNum->setValue(g_model.pxxRxNum);

        ui->typeCB->setCurrentIndex(g_model.sub_protocol) ;
        ui->ppmDelaySB->setValue(300);
        ui->numChannelsSB->setValue(8);
        ui->ppmFrameLengthDSB->setValue(22.5);
        break;
    case (PROTO_DSM2):
        ui->ppmDelaySB->setEnabled(false);
        ui->numChannelsSB->setEnabled(false);
        ui->ppmFrameLengthDSB->setEnabled(false);
        ui->DSM_Type->setEnabled(true);
        ui->DSM_Type->show() ;
        ui->SubProtocolCB->hide() ;
        ui->pxxRxNum->setEnabled(true);
				ui->startChannelsSB->setEnabled(true);
				ui->pulsePolCB->setEnabled(false);

        ui->DSM_Type->setCurrentIndex(g_model.sub_protocol )	;

        ui->pxxRxNum->setValue(g_model.pxxRxNum);
        ui->ppmDelaySB->setValue(300);
        ui->numChannelsSB->setValue(8);
        ui->ppmFrameLengthDSB->setValue(22.5);
        ui->countryCB->setEnabled(false);
        ui->typeCB->setEnabled(false);
        break;
	    
			case (PROTO_MULTI):
        ui->ppmDelaySB->setEnabled(false);
        ui->numChannelsSB->setEnabled(true);
        ui->ppmFrameLengthDSB->setEnabled(false);
        ui->DSM_Type->hide() ;
        ui->SubProtocolCB->show() ;
        ui->SubProtocolCB->setCurrentIndex(g_model.sub_protocol )	;
        ui->pxxRxNum->setEnabled(false);
        ui->typeCB->setEnabled(false);
        ui->countryCB->setEnabled(false);
				ui->startChannelsSB->setEnabled(true);
				ui->pulsePolCB->setEnabled(false);
      break ;

	    case (PROTO_ASSAN):
        ui->ppmDelaySB->setEnabled(false);
        ui->numChannelsSB->setEnabled(true);
        ui->ppmFrameLengthDSB->setEnabled(false);
        ui->DSM_Type->hide() ;
        ui->SubProtocolCB->hide() ;
        ui->pxxRxNum->setEnabled(false);
        ui->typeCB->setEnabled(false);
        ui->countryCB->setEnabled(true);
				ui->startChannelsSB->setEnabled(true);
				ui->pulsePolCB->setEnabled(false);
      break;
	    
	    case (PROTO_OFF):
        ui->ppmDelaySB->setEnabled(false);
        ui->numChannelsSB->setEnabled(false);
        ui->ppmFrameLengthDSB->setEnabled(false);
        ui->DSM_Type->hide() ;
        ui->SubProtocolCB->hide() ;
        ui->SubProtocolCB->setEnabled(false);
        ui->pxxRxNum->setEnabled(false);
        ui->typeCB->setEnabled(false);
        ui->countryCB->setEnabled(false);
				ui->startChannelsSB->setEnabled(false);
				ui->pulsePolCB->setEnabled(false);
      break;
    default:	// PPM
        ui->ppmDelaySB->setEnabled(true);
        ui->numChannelsSB->setEnabled(true);
        ui->ppmFrameLengthDSB->setEnabled(true);
        ui->DSM_Type->hide() ;
        ui->SubProtocolCB->hide() ;
        ui->pxxRxNum->setEnabled(false);

        ui->ppmDelaySB->setValue(300+50*g_model.ppmDelay);
        ui->numChannelsSB->setValue(8+2*g_model.ppmNCH);
        ui->ppmFrameLengthDSB->setValue(22.5+((double)g_model.ppmFrameLength)*0.5);

        ui->DSM_Type->setCurrentIndex(0);
        ui->countryCB->setEnabled(false);
        ui->typeCB->setEnabled(false);
				ui->startChannelsSB->setEnabled(true);
				ui->pulsePolCB->setEnabled(true);
        break;
    }

    switch (g_model.xprotocol)
    {
	    case (PROTO_PXX):
        ui->xppmDelaySB->setEnabled(false);
        ui->xnumChannelsSB->setEnabled(false);
        ui->xppmFrameLengthDSB->setEnabled(false);
        ui->xDSM_Type->hide() ;
        ui->xSubProtocolCB->hide() ;
        ui->xPxxRxNum->setEnabled(true);
        ui->xcountryCB->setEnabled(true);
        ui->xtypeCB->setEnabled(true);
				ui->xstartChannelsSB->setEnabled(true);
				ui->xpulsePolCB->setEnabled(false);

        ui->xPxxRxNum->setValue(g_model.xPxxRxNum);

        ui->xtypeCB->setCurrentIndex(g_model.xsub_protocol) ;
        ui->xppmDelaySB->setValue(300);
        ui->xnumChannelsSB->setValue(8);
        ui->xppmFrameLengthDSB->setValue(22.5);
        break;
	    case (PROTO_DSM2):
        ui->xppmDelaySB->setEnabled(false);
        ui->xnumChannelsSB->setEnabled(false);
        ui->xppmFrameLengthDSB->setEnabled(false);
        ui->xDSM_Type->setEnabled(true);
        ui->xDSM_Type->show() ;
        ui->xSubProtocolCB->hide() ;
        ui->xPxxRxNum->setEnabled(true);
				ui->xstartChannelsSB->setEnabled(true);
				ui->xpulsePolCB->setEnabled(false);

        ui->xDSM_Type->setCurrentIndex(g_model.xsub_protocol )	;

        ui->xPxxRxNum->setValue(g_model.xPxxRxNum);
        ui->xppmDelaySB->setValue(300);
        ui->xnumChannelsSB->setValue(8);
        ui->xppmFrameLengthDSB->setValue(22.5);
        ui->xcountryCB->setEnabled(false);
        ui->xtypeCB->setEnabled(false);
        break;
	    case (PROTO_MULTI):
        ui->xppmDelaySB->setEnabled(false);
        ui->xnumChannelsSB->setEnabled(true);
        ui->xppmFrameLengthDSB->setEnabled(false);
        ui->xDSM_Type->hide() ;
        ui->xSubProtocolCB->show() ;
        ui->xSubProtocolCB->setCurrentIndex(g_model.xsub_protocol )	;
        ui->xPxxRxNum->setEnabled(false);
        ui->xtypeCB->setEnabled(false);
        ui->xcountryCB->setEnabled(false);
				ui->xstartChannelsSB->setEnabled(true);
				ui->xpulsePolCB->setEnabled(false);
      break ;

	    case (PROTO_ASSAN):
        ui->xppmDelaySB->setEnabled(false);
        ui->xnumChannelsSB->setEnabled(true);
        ui->xppmFrameLengthDSB->setEnabled(false);
        ui->xDSM_Type->hide() ;
        ui->xSubProtocolCB->hide() ;
        ui->xPxxRxNum->setEnabled(false);
        ui->xtypeCB->setEnabled(false);
        ui->xcountryCB->setEnabled(true);
				ui->xstartChannelsSB->setEnabled(true);
				ui->xpulsePolCB->setEnabled(false);
      break;
	    
			case (PROTO_OFF):
        ui->xppmDelaySB->setEnabled(false);
        ui->xnumChannelsSB->setEnabled(false);
        ui->xppmFrameLengthDSB->setEnabled(false);
        ui->xDSM_Type->hide() ;
        ui->xSubProtocolCB->hide() ;
        ui->xPxxRxNum->setEnabled(false);
        ui->xtypeCB->setEnabled(false);
        ui->xcountryCB->setEnabled(false);
				ui->xstartChannelsSB->setEnabled(false);
				ui->xpulsePolCB->setEnabled(false);
      break;

    default:
        ui->xppmDelaySB->setEnabled(true);
        ui->xnumChannelsSB->setEnabled(true);
        ui->xppmFrameLengthDSB->setEnabled(true);
        ui->xDSM_Type->hide() ;
        ui->xSubProtocolCB->hide() ;
        ui->xPxxRxNum->setEnabled(false);

        ui->xppmDelaySB->setValue(300+50*g_model.xppmDelay);
        ui->xnumChannelsSB->setValue(8+2*g_model.xppmNCH);
        ui->xppmFrameLengthDSB->setValue(22.5+((double)g_model.xppmFrameLength)*0.5);

        ui->xDSM_Type->setCurrentIndex(0);
        ui->xcountryCB->setEnabled(false);
        ui->xtypeCB->setEnabled(false);
				ui->xstartChannelsSB->setEnabled(true);
				ui->xpulsePolCB->setEnabled(true);
        break;
    }
		

    ui->numChannels2SB->setValue(8+2*g_model.ppm2NCH) ;
    ui->startChannelsSB->setValue(g_model.startChannel+1) ;
    ui->xstartChannelsSB->setValue(g_model.xstartChannel+1) ;
    ui->startChannels2SB->setValue(g_model.startPPM2channel) ;
    ui->startChannels2SB->setSuffix( (g_model.startPPM2channel == 0) ? " =follow" : "" ) ;

		if ( ( rData->type == 2 ) || ( rData->type == 3 ) )
		{
			ui->numChannels2SB->hide() ;
			ui->startChannels2SB->hide() ;
			ui->label_PPM2Channels->hide() ;
			ui->label_PPM2Start->hide() ;
			ui->label_PPM2->hide() ;
		}
		else
		{
			ui->numChannels2SB->show() ;
			ui->startChannels2SB->show() ;
			ui->label_PPM2Channels->show() ;
			ui->label_PPM2Start->show() ;
			ui->label_PPM2->show() ;
		}

    protocolEditLock = false;
}

void ModelEdit::tabExpo()
{
	int x ;
	int y ;
    populateSwitchCB(ui->RUD_edrSw1,g_model.expoData[CONVERT_MODE(RUD,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1, rData->type);
    populateSwitchCB(ui->RUD_edrSw2,g_model.expoData[CONVERT_MODE(RUD,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2, rData->type);
    populateSwitchCB(ui->ELE_edrSw1,g_model.expoData[CONVERT_MODE(ELE,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1, rData->type);
    populateSwitchCB(ui->ELE_edrSw2,g_model.expoData[CONVERT_MODE(ELE,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2, rData->type);
    populateSwitchCB(ui->THR_edrSw1,g_model.expoData[CONVERT_MODE(THR,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1, rData->type);
    populateSwitchCB(ui->THR_edrSw2,g_model.expoData[CONVERT_MODE(THR,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2, rData->type);
    populateSwitchCB(ui->AIL_edrSw1,g_model.expoData[CONVERT_MODE(AIL,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1, rData->type);
    populateSwitchCB(ui->AIL_edrSw2,g_model.expoData[CONVERT_MODE(AIL,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2, rData->type);


//#define DR_HIGH   0
//#define DR_MID    1
//#define DR_LOW    2
//#define DR_EXPO   0
//#define DR_WEIGHT 1
//#define DR_RIGHT  0
//#define DR_LEFT   1
//expo[3][2][2] //[HI/MID/LOW][expo/weight][R/L]
    
		int i, j, k ;
		QSpinBox *sb ;
		QComboBox *cb ;
		QCheckBox *chkb ;
		
		for ( i = 0 ; i < 3 ; i += 1 )
		{ // 0=High, 1=Mid, 2=Low
			for ( j = 0 ; j < 2 ; j += 1 )
			{ // 0=Weight, 1=Expo - WRONG - 0=expo, 1=weight
				for ( k = 0 ; k < 2 ; k += 1 )
				{ // 0=Right, 1=Left
					int xpos ;
					xpos = k*2+j ;
					switch ( xpos )
					{
						case 3 :
							xpos = 2 ;
						break ;
						case 2 :
							xpos = 1 ;
						break ;
						case 1 :
							xpos = 3 ;
						break ;
						case 0 :
							xpos = 4 ;
						break ;
					}
					sb = expoDrSpin[1][i][j][k] = new QSpinBox(this) ;
					sb->setFixedSize( 64, 20 ) ;
					cb = expoDrVal[1][i][j][k] = new QComboBox(this) ;
					cb->setFixedSize( 64, 20 ) ;
    			ui->gridLayout_Ail->addWidget( sb,i*2+1,xpos);
    			ui->gridLayout_Ail->addWidget( cb,i*2+1,xpos);
					chkb = expoDrGvar[1][i][j][k] = new QCheckBox(this) ;
    			ui->gridLayout_Ail->addWidget( chkb,i*2+2,xpos);
					chkb->setText( "Gvar" ) ;

          x = g_model.expoData[CONVERT_MODE(AIL,g_model.modelVersion,g_eeGeneral.stickMode)-1].expo[i][j][k] ;
					y = -100 ;
					if ( j == 1 )
					{
    				/*if ( ( x >= -100 && x <= 100 ) )*/ x += 100 ;
						y = 0 ;
					}
					populateSpinGVarCB( sb, cb, chkb, x, y, 100 ) ;
			    
					connect( sb, SIGNAL(editingFinished()),this,SLOT(expoEdited()));
			    connect( cb, SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
			    connect( chkb,SIGNAL(stateChanged(int)),this,SLOT(expoEdited()));

					sb = expoDrSpin[0][i][j][k] = new QSpinBox(this) ;
					sb->setFixedSize( 64, 20 ) ;
					cb = expoDrVal[0][i][j][k] = new QComboBox(this) ;
					cb->setFixedSize( 64, 20 ) ;
    			ui->gridLayout_Rud->addWidget( sb,i*2+1,xpos);
    			ui->gridLayout_Rud->addWidget( cb,i*2+1,xpos);
					chkb = expoDrGvar[0][i][j][k] = new QCheckBox(this) ;
    			ui->gridLayout_Rud->addWidget( chkb,i*2+2,xpos);
					chkb->setText( "Gvar" ) ;

          x = g_model.expoData[CONVERT_MODE(RUD,g_model.modelVersion,g_eeGeneral.stickMode)-1].expo[i][j][k] ;
					y = -100 ;
					if ( j == 1 )
					{
    				/*if ( ( x >= -100 && x <= 100 ) )*/ x += 100 ;
						y = 0 ;
					}
					populateSpinGVarCB( sb, cb, chkb, x, y, 100 ) ;
			    
					connect( sb, SIGNAL(editingFinished()),this,SLOT(expoEdited()));
			    connect( cb, SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
			    connect( chkb,SIGNAL(stateChanged(int)),this,SLOT(expoEdited()));

					sb = expoDrSpin[2][i][j][k] = new QSpinBox(this) ;
					sb->setFixedSize( 64, 20 ) ;
					cb = expoDrVal[2][i][j][k] = new QComboBox(this) ;
					cb->setFixedSize( 64, 20 ) ;
    			ui->gridLayout_Thr->addWidget( sb,i*2+1,xpos);
    			ui->gridLayout_Thr->addWidget( cb,i*2+1,xpos);
					chkb = expoDrGvar[2][i][j][k] = new QCheckBox(this) ;
    			ui->gridLayout_Thr->addWidget( chkb,i*2+2,xpos);
					chkb->setText( "Gvar" ) ;

          x = g_model.expoData[CONVERT_MODE(THR,g_model.modelVersion,g_eeGeneral.stickMode)-1].expo[i][j][k] ;
					y = -100 ;
					if ( j == 1 )
					{
    				/*if ( ( x >= -100 && x <= 100 ) )*/ x += 100 ;
						y = 0 ;
					}
					populateSpinGVarCB( sb, cb, chkb, x, y, 100 ) ;
			    
					connect( sb, SIGNAL(editingFinished()),this,SLOT(expoEdited()));
			    connect( cb, SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
			    connect( chkb,SIGNAL(stateChanged(int)),this,SLOT(expoEdited()));
			    if(g_model.thrExpo)
					{
						if ( k == 1 )
						{
			        sb->setEnabled(false);
      			  cb->setEnabled(false);
			        chkb->setEnabled(false);
						}
					}

					sb = expoDrSpin[3][i][j][k] = new QSpinBox(this) ;
					sb->setFixedSize( 64, 20 ) ;
					cb = expoDrVal[3][i][j][k] = new QComboBox(this) ;
					cb->setFixedSize( 64, 20 ) ;
    			ui->gridLayout_Ele->addWidget( sb,i*2+1,xpos);
    			ui->gridLayout_Ele->addWidget( cb,i*2+1,xpos);
					chkb = expoDrGvar[3][i][j][k] = new QCheckBox(this) ;
    			ui->gridLayout_Ele->addWidget( chkb,i*2+2,xpos);
					chkb->setText( "Gvar" ) ;

          x = g_model.expoData[CONVERT_MODE(ELE,g_model.modelVersion,g_eeGeneral.stickMode)-1].expo[i][j][k] ;
					y = -100 ;
					if ( j == 1 )
					{
    				/*if ( ( x >= -100 && x <= 100 ) )*/ x += 100 ;
						y = 0 ;
					}
					populateSpinGVarCB( sb, cb, chkb, x, y, 100 ) ;
			    
					connect( sb, SIGNAL(editingFinished()),this,SLOT(expoEdited()));
			    connect( cb, SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
			    connect( chkb,SIGNAL(stateChanged(int)),this,SLOT(expoEdited()));

				}
			}
		} 
		

    connect(ui->RUD_edrSw1,SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
    connect(ui->RUD_edrSw2,SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
    connect(ui->ELE_edrSw1,SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
    connect(ui->ELE_edrSw2,SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
    connect(ui->THR_edrSw1,SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
    connect(ui->THR_edrSw2,SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
    connect(ui->AIL_edrSw1,SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));
    connect(ui->AIL_edrSw2,SIGNAL(currentIndexChanged(int)),this,SLOT(expoEdited()));

}

void expoDrSet( int8_t *pval, int x )
{
//  if ( ( x >= -100 && x <= 100 ) ) x -= 100 ;
	x -= 100 ;
  *pval = x ;
}


void ModelEdit::expoEdited()
{
		int i, j, k ;
		QSpinBox *sb ;
		QComboBox *cb ;
		QCheckBox *chkb ;
//	int limit = MAX_DRSWITCH ;
//#ifdef SKY
//	if ( rData->type )
//	{
//		limit = MAX_XDRSWITCH ;
//	}
//#endif

  int8_t *pval ;
    g_model.expoData[CONVERT_MODE(RUD,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1 = getSwitchCbValue( ui->RUD_edrSw1, rData->type ) ;
    g_model.expoData[CONVERT_MODE(RUD,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2 = getSwitchCbValue( ui->RUD_edrSw2, rData->type ) ;
    g_model.expoData[CONVERT_MODE(ELE,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1 = getSwitchCbValue( ui->ELE_edrSw1, rData->type ) ;
    g_model.expoData[CONVERT_MODE(ELE,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2 = getSwitchCbValue( ui->ELE_edrSw2, rData->type ) ;
    g_model.expoData[CONVERT_MODE(THR,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1 = getSwitchCbValue( ui->THR_edrSw1, rData->type ) ;
    g_model.expoData[CONVERT_MODE(THR,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2 = getSwitchCbValue( ui->THR_edrSw2, rData->type ) ;
    g_model.expoData[CONVERT_MODE(AIL,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1 = getSwitchCbValue( ui->AIL_edrSw1, rData->type ) ;
    g_model.expoData[CONVERT_MODE(AIL,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2 = getSwitchCbValue( ui->AIL_edrSw2, rData->type ) ;

		for ( i = 0 ; i < 3 ; i += 1 )
		{ // 0=High, 1=Mid, 2=Low
			for ( j = 0 ; j < 2 ; j += 1 )
			{ // 0=Weight, 1=Expo
				for ( k = 0 ; k < 2 ; k += 1 )
				{ // 0=Right, 1=Left
					sb = expoDrSpin[1][i][j][k] ;
					cb = expoDrVal[1][i][j][k] ;
					chkb = expoDrGvar[1][i][j][k] ;
          pval = &g_model.expoData[CONVERT_MODE(AIL,g_model.modelVersion,g_eeGeneral.stickMode)-1].expo[i][j][k] ;
					if ( j==0 )
					{
    				*pval = numericSpinGvarValue( sb, cb, chkb, *pval, 0 ) ;
					}
					else
					{
            int temp = *pval + 100 ;
            if ( temp > 127) temp -= 256 ;
			    	expoDrSet( pval, numericSpinGvarValue( sb, cb, chkb, temp, 100 ) ) ;
					}

					sb = expoDrSpin[0][i][j][k] ;
					cb = expoDrVal[0][i][j][k] ;
          chkb = expoDrGvar[0][i][j][k] ;
          pval = &g_model.expoData[CONVERT_MODE(RUD,g_model.modelVersion,g_eeGeneral.stickMode)-1].expo[i][j][k] ;
					if ( j==0 )
					{
    				*pval = numericSpinGvarValue( sb, cb, chkb, *pval, 0 ) ;
					}
					else
					{
            int temp = *pval + 100 ;
            if ( temp > 127) temp -= 256 ;
			    	expoDrSet( pval, numericSpinGvarValue( sb, cb, chkb, temp, 100 ) ) ;
					}

					sb = expoDrSpin[2][i][j][k] ;
					cb = expoDrVal[2][i][j][k] ;
          chkb = expoDrGvar[2][i][j][k] ;
          pval = &g_model.expoData[CONVERT_MODE(THR,g_model.modelVersion,g_eeGeneral.stickMode)-1].expo[i][j][k] ;
					if ( j==0 )
					{
    				*pval = numericSpinGvarValue( sb, cb, chkb, *pval, 0 ) ;
					}
					else
					{
            int temp = *pval + 100 ;
            if ( temp > 127) temp -= 256 ;
			    	expoDrSet( pval, numericSpinGvarValue( sb, cb, chkb, temp, 100 ) ) ;
					}

					sb = expoDrSpin[3][i][j][k] ;
					cb = expoDrVal[3][i][j][k] ;
          chkb = expoDrGvar[3][i][j][k] ;
          pval = &g_model.expoData[CONVERT_MODE(ELE,g_model.modelVersion,g_eeGeneral.stickMode)-1].expo[i][j][k] ;
					if ( j==0 )
					{
    				*pval = numericSpinGvarValue( sb, cb, chkb, *pval, 0 ) ;
					}
					else
					{
            int temp = *pval + 100 ;
            if ( temp > 127) temp -= 256 ;
			    	expoDrSet( pval, numericSpinGvarValue( sb, cb, chkb, temp, 100 ) ) ;
					}

				}
			}
		} 
    updateSettings();
}

void ModelEdit::tabVoiceAlarms()
{
	QByteArray qba ;
  uint32_t i ;

	ui->VoiceAlarmList->setFont(QFont("Courier New",12)) ;
	ui->VoiceAlarmList->clear() ;
	for(i=0 ; i<NUM_SKY_VOICE_ALARMS+NUM_EXTRA_VOICE_ALARMS ; i += 1)
	{
		VoiceAlarmData *vad = ( i >= NUM_SKY_VOICE_ALARMS) ? &g_model.vadx[i-NUM_SKY_VOICE_ALARMS] : &g_model.vad[i] ;
		QString str = "";
		str = tr("VA%1%2  ").arg((i+1)/10).arg((i+1)%10) ;
		QString srcstr ;
    uint32_t limit = 45 ;
		uint32_t value = vad->source ;
		if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
		{
			limit = 46 ;
			if ( rData->type == 2 )
			{
				limit = 47 ;
			}
			if ( value == EXTRA_POTS_START )
			{
        value = 8 ;
			}
			else
			{
				if ( rData->type == 2 )
				{
					if ( value == EXTRA_POTS_START + 1 )
					{
        		value = 9 ;
					}
					else if ( value >= EXTRA_POTS_POSITION )
					{
						value += 2 ;
					}
				}
				else if ( value >= EXTRA_POTS_POSITION )
				{
					value += 1 ;
				}
			}
		}
		if ( value < limit )
		{
			str += tr("(%1) ").arg(getSourceStr(g_eeGeneral.stickMode,value,g_model.modelVersion, rData->type )) ;
		}
		else
		{
      str += tr("(%1) ").arg(getTelemString(value-limit+1 )) ;
		}
    srcstr = "-------v>val  v<val  |v|>val|v|<valv~=val " ;
		str += tr("%1 ").arg(srcstr.mid( vad->func * 7, 7 )) ;
  	if ( vad->source > 44 )
		{
			char telText[20] ;
  	  stringTelemetryChannel( telText, vad->source - 45, vad->offset, &g_model ) ;
			str += tr("(%1) ").arg(telText) ;
		}
		else
		{
			str += tr("(%1) ").arg(vad->offset) ;
		}
		str += tr("Switch(%1) ").arg(getSWName(vad->swtch, rData->type)) ;
		if ( vad->rate < 4 )
		{
			srcstr = (vad->rate > 1 ) ? (vad->rate == 2 ? "BOTH " : "ALL ") : (vad->rate == 0 ? "ON " : "OFF" ) ;
			str += srcstr ;
		}
		else if ( vad->rate > 32 )
		{
			str += "ONCE" ;
		}
		else
		{
			str += tr("Rate(%1) ").arg(vad->rate-2) ;
		}
		if ( vad->mute )
		{
			str += tr("Mute ") ;
		}
		if ( vad->haptic )
		{
			str += tr("Haptic(%1) ").arg(vad->haptic) ;
		}
		if ( vad->vsource )
		{
			srcstr = ( vad->vsource == 1 ) ? "Before" : "After " ;
			str += tr("PlaySrc(%1) ").arg(srcstr) ;
		}
		switch ( vad->fnameType )
		{
			case 1 :
      {
        QString xstr = (char *)vad->file.name ;
				xstr = xstr.left(8) ;
        str += tr("File(%1)").arg(xstr ) ;
      }
			break ;
			case 2 :
				str += tr("File(%1)").arg(vad->file.vfile ) ;
			break ;
			case 3 :
				str += tr("Alarm(%1)").arg(getAudioAlarmName(vad->file.vfile) ) ;
			break ;
		}
    ui->VoiceAlarmList->addItem(str) ;
	}
}

void ModelEdit::tabMixes()
{
    // curDest -> destination channel
    // i -> mixer number
    QByteArray qba;
    MixerlistWidget->clear();
    int curDest = 0;
    int i;
    for(i=0; i<MAX_SKYMIXERS; i++)
    {
        SKYMixData *md = &g_model.mixData[i];
        if((md->destCh==0) || (md->destCh>NUM_SKYCHNOUT)) break;
        QString str = "";
        while(curDest<(md->destCh-1))
        {
            curDest++;
            str = tr("CH%1%2").arg(curDest/10).arg(curDest%10);
            qba.clear();
            qba.append((quint8)-curDest);
            QListWidgetItem *itm = new QListWidgetItem(str);
            itm->setData(Qt::UserRole,qba);
            MixerlistWidget->addItem(itm);
        }

        if(curDest!=md->destCh)
        {
            str = tr("CH%1%2").arg(md->destCh/10).arg(md->destCh%10);
            curDest=md->destCh;
        }
        else
            str = "    ";

        switch(md->mltpx)
        {
        case (1): str += " *"; break;
        case (2): str += " R"; break;
        default:  str += "  "; break;
        };

				int j ;
				j = md->weight ;
				if ( j < -125 )
				{
					j += 256 ;					
				}
				if ( j > 125 )
				{
        	str += QString(" GV%1\%").arg(j-125).rightJustified(6,' ') ;
				}
				else
				{
        	str += j<0 ? QString(" %1\%").arg(j).rightJustified(6,' ') :
                              QString(" +%1\%").arg(j).rightJustified(6, ' ');
				}


        //QString srcStr = SRC_STR;
        //str += " " + srcStr.mid(CONVERT_MODE(md->srcRaw+1)*4,4);
        QString srcstr ;
			  uint32_t lowBound = rData->type ? 21 : 21 ;
//				if ( rData->type == 2 )
//				{
//					lowBound = 23 ;
//				}
			  if ( ( md->srcRaw >= lowBound && md->srcRaw <= lowBound+23 ) && ( md->disableExpoDr ) )
				{
          srcstr = QString("OP%1").arg(md->srcRaw-(lowBound-1)) ;
				}
				else
				{
					uint32_t value ;
					value = md->srcRaw ;
					if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
					{
						if ( value == EXTRA_POTS_START )
						{
              value = 8 ;
						}
						else
						{
							if ( rData->type == 2 )
							{
								if ( value == EXTRA_POTS_START + 1 )
								{
        		      value = 9 ;
								}
								else if ( value >= EXTRA_POTS_POSITION )
								{
									value += 2 ;
								}
							}
							else if ( value >= EXTRA_POTS_POSITION )
							{
								value += 1 ;
							}
						}
					}
          srcstr = getSourceStr(g_eeGeneral.stickMode, value,g_model.modelVersion, rData->type );
				}

        str += srcstr ;
				if ( srcstr == "s" )
				{
					if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
					{
						srcstr = "_SA_SB_SC_SD_SE_SF_SG_SH_6P" ;
					}
					else
					{
						srcstr = "IdxThrRudEleAilGeaTrn" ;
					}
					str += srcstr.mid( md->switchSource * 3, 3 ) ;
				}

        if(md->swtch) str += tr(" Switch(") + getSWName(md->swtch, rData->type) + ")";
        if(md->carryTrim) str += tr(" noTrim");
				j = md->sOffset ;
        if(j)
				{
					if ( j < -125 )
					{
						j += 256 ;					
					}
					if ( j > 125 )
					{
            str += tr(" Offset(GV%1)").arg(j-125) ;
					}
					else
					{
        		str += tr(" Offset(%1\%)").arg(j);
					}
				}
        if(md->curve)
        {
					if ( md->differential )
					{
						if ( ( md->curve >= -100 ) && ( md->curve <= 100) )
						{
            	str += tr(" Diff(%1)").arg(md->curve);
						}
						else
						{ // GVAR
							int x = md->curve ;
							if ( x < 0 )
							{
								x += 256 ;								
							}
							x -= 125 ;
            	str += tr(" Diff(GV%1)").arg(x) ;
						}
					}
					else
					{
            QString crvStr = CURV_STR;
						int x ;
						x = md->curve ;
						if ( x < 0 )
						{
							if ( x <= -28 )
							{
            		str += tr("Expo(%1)").arg( x+128 ) ;
							}
							else
							{
								x = -x + 6 ;
            		str += tr("!Curve(%1)").arg(crvStr.mid(x*3,3).remove(' '));
							}
						}
						else
						{
            	str += tr(" Curve(%1)").arg(crvStr.mid(md->curve*3,3).remove(' '));
						}
					}
        }

        if(md->delayDown || md->delayUp) str += tr(" Delay(u%1:d%2)").arg((double)md->delayUp/10).arg((double)md->delayDown/10) ;
        if(md->speedDown || md->speedUp) str += tr(" Slow(u%1:d%2)").arg((double)md->speedUp/10).arg((double)md->speedDown/10) ;

        if(md->mixWarn)  str += tr(" Warn(%1)").arg(md->mixWarn);

        if(!mixNotes[i].isEmpty())
            str += " (Note)";

        qba.clear();
        qba.append((quint8)i);
        qba.append((const char*)md, sizeof(SKYMixData));
        QListWidgetItem *itm = new QListWidgetItem(str);
        itm->setData(Qt::UserRole,qba);  // mix number
        MixerlistWidget->addItem(itm);//(str);
        MixerlistWidget->item(MixerlistWidget->count()-1)->setToolTip(mixNotes[i]);
    }

    while(curDest<NUM_SKYCHNOUT)
    {
        curDest++;
        QString str = tr("CH%1%2").arg(curDest/10).arg(curDest%10);

        qba.clear();
        qba.append((quint8)-curDest);
        QListWidgetItem *itm = new QListWidgetItem(str);
        itm->setData(Qt::UserRole,qba); // add new mixer
        MixerlistWidget->addItem(itm);
    }

    if(MixerlistWidget->selectedItems().isEmpty())
    {
        MixerlistWidget->setCurrentRow(0);
        MixerlistWidget->item(0)->setSelected(true);
    }

}

void ModelEdit::mixesEdited()
{
    updateSettings();
}

int16_t ModelEdit::getRawTrimValue( uint8_t phase, uint8_t idx )
{
	if ( phase )
	{
		return g_model.phaseData[phase-1].trim[idx] ;
	}	
	else
	{
		return g_model.trim[idx] ;
	}
}

uint32_t ModelEdit::getTrimFlightPhase( uint8_t phase, uint8_t idx )
{
  for ( uint32_t i=0 ; i<MAX_PHASES ; i += 1 )
	{
    if (phase == 0) return 0;
    int16_t trim = getRawTrimValue( phase, idx ) ;
    if ( trim <= TRIM_EXTENDED_MAX )
		{
			return phase ;
		}
    uint32_t result = trim-TRIM_EXTENDED_MAX-1 ;
    if (result >= phase)
		{
			result += 1 ;
		}
    phase = result;
  }
  return 0;
}

int16_t ModelEdit::getTrimValue( uint8_t phase, uint8_t idx )
{
  return getRawTrimValue( getTrimFlightPhase( phase, idx ), idx ) ;
}


void ModelEdit::tabPhase()
{
	updatePhaseTab() ;
	
	connect(ui->FP1_sw,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP2_sw,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP3_sw,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP4_sw,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP5_sw,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP6_sw,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 

	connect(ui->FP1_RudCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP1_EleCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP1_ThrCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP1_AilCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP2_RudCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP2_EleCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP2_ThrCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP2_AilCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP3_RudCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP3_EleCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP3_ThrCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP3_AilCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP4_RudCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP4_EleCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP4_ThrCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP4_AilCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP5_RudCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP5_EleCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP5_ThrCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP5_AilCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP6_RudCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP6_EleCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP6_ThrCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP6_AilCB,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	
  connect(ui->FP1rudTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP1eleTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP1thrTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP1ailTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP2rudTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP2eleTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP2thrTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP2ailTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP3rudTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP3eleTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP3thrTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP3ailTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP4rudTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP4eleTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP4thrTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP4ailTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP5rudTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP5eleTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP5thrTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP5ailTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP6rudTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP6eleTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP6thrTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
  connect(ui->FP6ailTrimSB,SIGNAL(valueChanged(int)),this,SLOT(phaseEdited()));
	
	
	connect(ui->FM1FadeIn,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM1FadeOut,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM2FadeIn,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM2FadeOut,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM3FadeIn,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM3FadeOut,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM4FadeIn,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM4FadeOut,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM5FadeIn,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM5FadeOut,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM6FadeIn,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM6FadeOut,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 

  connect( ui->FM1Name, SIGNAL(editingFinished()),this,SLOT(phaseEdited()));
  connect( ui->FM2Name, SIGNAL(editingFinished()),this,SLOT(phaseEdited()));
  connect( ui->FM3Name, SIGNAL(editingFinished()),this,SLOT(phaseEdited()));
  connect( ui->FM4Name, SIGNAL(editingFinished()),this,SLOT(phaseEdited()));
  connect( ui->FM5Name, SIGNAL(editingFinished()),this,SLOT(phaseEdited()));
  connect( ui->FM6Name, SIGNAL(editingFinished()),this,SLOT(phaseEdited()));

}

void ModelEdit::updatePhaseTab()
{
	uint8_t smode = g_eeGeneral.stickMode ;
	if ( g_model.modelVersion >= 2 )
 	{
		smode = 0 ;
	}
	
  if ( smode & 1 )
	{
		ui->label_M2->setText("Throttle") ;
		ui->label_M3->setText("Elevator") ;
	}
	else
	{
		ui->label_M2->setText("Elevator") ;
		ui->label_M3->setText("Throttle") ;
	}
	
  if ( smode & 2 )
	{
		ui->label_M1->setText("Aileron") ;
		ui->label_M4->setText("Rudder") ;
	}
	else
	{
		ui->label_M1->setText("Rudder") ;
		ui->label_M4->setText("Aileron") ;
	}
	
	populateSwitchShortCB( ui->FP1_sw, g_model.phaseData[0].swtch, rData->type ) ;
	populateSwitchShortCB( ui->FP2_sw, g_model.phaseData[1].swtch, rData->type ) ;
	populateSwitchShortCB( ui->FP3_sw, g_model.phaseData[2].swtch, rData->type ) ;
	populateSwitchShortCB( ui->FP4_sw, g_model.phaseData[3].swtch, rData->type ) ;
	populateSwitchShortCB( ui->FP5_sw, g_model.phaseData[4].swtch, rData->type ) ;
	populateSwitchShortCB( ui->FP6_sw, g_model.phaseData[5].swtch, rData->type ) ;

	
	ui->FP1rudTrimSB->setDisabled( populatePhasetrim( ui->FP1_RudCB, 1, g_model.phaseData[0].trim[0] ) ) ;
	ui->FP1eleTrimSB->setDisabled( populatePhasetrim( ui->FP1_EleCB, 1, g_model.phaseData[0].trim[1] ) ) ;
	ui->FP1thrTrimSB->setDisabled( populatePhasetrim( ui->FP1_ThrCB, 1,  g_model.phaseData[0].trim[2] ) ) ;
	ui->FP1ailTrimSB->setDisabled( populatePhasetrim( ui->FP1_AilCB, 1,  g_model.phaseData[0].trim[3] ) ) ;
	ui->FP2rudTrimSB->setDisabled( populatePhasetrim( ui->FP2_RudCB, 2,  g_model.phaseData[1].trim[0] ) ) ;
	ui->FP2eleTrimSB->setDisabled( populatePhasetrim( ui->FP2_EleCB, 2,  g_model.phaseData[1].trim[1] ) ) ;
	ui->FP2thrTrimSB->setDisabled( populatePhasetrim( ui->FP2_ThrCB, 2,  g_model.phaseData[1].trim[2] ) ) ;
	ui->FP2ailTrimSB->setDisabled( populatePhasetrim( ui->FP2_AilCB, 2,  g_model.phaseData[1].trim[3] ) ) ;
	ui->FP3rudTrimSB->setDisabled( populatePhasetrim( ui->FP3_RudCB, 3,  g_model.phaseData[2].trim[0] ) ) ;
	ui->FP3eleTrimSB->setDisabled( populatePhasetrim( ui->FP3_EleCB, 3,  g_model.phaseData[2].trim[1] ) ) ;
	ui->FP3thrTrimSB->setDisabled( populatePhasetrim( ui->FP3_ThrCB, 3,  g_model.phaseData[2].trim[2] ) ) ;
	ui->FP3ailTrimSB->setDisabled( populatePhasetrim( ui->FP3_AilCB, 3,  g_model.phaseData[2].trim[3] ) ) ;
	ui->FP4rudTrimSB->setDisabled( populatePhasetrim( ui->FP4_RudCB, 4,  g_model.phaseData[3].trim[0] ) ) ;
	ui->FP4eleTrimSB->setDisabled( populatePhasetrim( ui->FP4_EleCB, 4,  g_model.phaseData[3].trim[1] ) ) ;
	ui->FP4thrTrimSB->setDisabled( populatePhasetrim( ui->FP4_ThrCB, 4,  g_model.phaseData[3].trim[2] ) ) ;
	ui->FP4ailTrimSB->setDisabled( populatePhasetrim( ui->FP4_AilCB, 4,  g_model.phaseData[3].trim[3] ) ) ;
	ui->FP5rudTrimSB->setDisabled( populatePhasetrim( ui->FP5_RudCB, 5,  g_model.phaseData[4].trim[0] ) ) ;
	ui->FP5eleTrimSB->setDisabled( populatePhasetrim( ui->FP5_EleCB, 5,  g_model.phaseData[4].trim[1] ) ) ;
	ui->FP5thrTrimSB->setDisabled( populatePhasetrim( ui->FP5_ThrCB, 5,  g_model.phaseData[4].trim[2] ) ) ;
	ui->FP5ailTrimSB->setDisabled( populatePhasetrim( ui->FP5_AilCB, 5,  g_model.phaseData[4].trim[3] ) ) ;
	ui->FP6rudTrimSB->setDisabled( populatePhasetrim( ui->FP6_RudCB, 6,  g_model.phaseData[5].trim[0] ) ) ;
	ui->FP6eleTrimSB->setDisabled( populatePhasetrim( ui->FP6_EleCB, 6,  g_model.phaseData[5].trim[1] ) ) ;
	ui->FP6thrTrimSB->setDisabled( populatePhasetrim( ui->FP6_ThrCB, 6,  g_model.phaseData[5].trim[2] ) ) ;
	ui->FP6ailTrimSB->setDisabled( populatePhasetrim( ui->FP6_AilCB, 6,  g_model.phaseData[5].trim[3] ) ) ;

//	ui->FP1rudTrimSB->setValue( getTrimValue( 1, 0 ) ) ;
//	ui->FP1eleTrimSB->setValue( getTrimValue( 1, 1 ) ) ;

	ui->FP1rudTrimSB->setValue(g_model.phaseData[0].trim[0]) ;
	ui->FP1eleTrimSB->setValue(g_model.phaseData[0].trim[1]) ;
	ui->FP1thrTrimSB->setValue(g_model.phaseData[0].trim[2]) ;
	ui->FP1ailTrimSB->setValue(g_model.phaseData[0].trim[3]) ;
	ui->FP2rudTrimSB->setValue(g_model.phaseData[1].trim[0]) ;
	ui->FP2eleTrimSB->setValue(g_model.phaseData[1].trim[1]) ;
	ui->FP2thrTrimSB->setValue(g_model.phaseData[1].trim[2]) ;
	ui->FP2ailTrimSB->setValue(g_model.phaseData[1].trim[3]) ;
	ui->FP3rudTrimSB->setValue(g_model.phaseData[2].trim[0]) ;
	ui->FP3eleTrimSB->setValue(g_model.phaseData[2].trim[1]) ;
	ui->FP3thrTrimSB->setValue(g_model.phaseData[2].trim[2]) ;
	ui->FP3ailTrimSB->setValue(g_model.phaseData[2].trim[3]) ;
	ui->FP4rudTrimSB->setValue(g_model.phaseData[3].trim[0]) ;
	ui->FP4eleTrimSB->setValue(g_model.phaseData[3].trim[1]) ;
	ui->FP4thrTrimSB->setValue(g_model.phaseData[3].trim[2]) ;
	ui->FP4ailTrimSB->setValue(g_model.phaseData[3].trim[3]) ;
	ui->FP5rudTrimSB->setValue(g_model.phaseData[4].trim[0]) ;
	ui->FP5eleTrimSB->setValue(g_model.phaseData[4].trim[1]) ;
	ui->FP5thrTrimSB->setValue(g_model.phaseData[4].trim[2]) ;
	ui->FP5ailTrimSB->setValue(g_model.phaseData[4].trim[3]) ;
	ui->FP6rudTrimSB->setValue(g_model.phaseData[5].trim[0]) ;
	ui->FP6eleTrimSB->setValue(g_model.phaseData[5].trim[1]) ;
	ui->FP6thrTrimSB->setValue(g_model.phaseData[5].trim[2]) ;
	ui->FP6ailTrimSB->setValue(g_model.phaseData[5].trim[3]) ;

	ui->FM1FadeIn->setValue(g_model.phaseData[0].fadeIn/2.0) ;
	ui->FM1FadeOut->setValue(g_model.phaseData[0].fadeOut/2.0) ;
	ui->FM2FadeIn->setValue(g_model.phaseData[1].fadeIn/2.0) ;
	ui->FM2FadeOut->setValue(g_model.phaseData[1].fadeOut/2.0) ;
	ui->FM3FadeIn->setValue(g_model.phaseData[2].fadeIn/2.0) ;
	ui->FM3FadeOut->setValue(g_model.phaseData[2].fadeOut/2.0) ;
	ui->FM4FadeIn->setValue(g_model.phaseData[3].fadeIn/2.0) ;
	ui->FM4FadeOut->setValue(g_model.phaseData[3].fadeOut/2.0) ;
	ui->FM5FadeIn->setValue(g_model.phaseData[4].fadeIn/2.0) ;
	ui->FM5FadeOut->setValue(g_model.phaseData[4].fadeOut/2.0) ;
	ui->FM6FadeIn->setValue(g_model.phaseData[5].fadeIn/2.0) ;
	ui->FM6FadeOut->setValue(g_model.phaseData[5].fadeOut/2.0) ;

	QString n = g_model.phaseData[0].name ;
	while ( n.endsWith(" ") )
	{
		n = n.left(n.size()-1) ;			
	}
  ui->FM1Name->setText( n ) ;
  n = g_model.phaseData[1].name ;
	while ( n.endsWith(" ") )
	{
		n = n.left(n.size()-1) ;			
	}
  ui->FM2Name->setText( n ) ;
  n = g_model.phaseData[2].name ;
	while ( n.endsWith(" ") )
	{
		n = n.left(n.size()-1) ;			
	}
  ui->FM3Name->setText( n ) ;
  n = g_model.phaseData[3].name ;
	while ( n.endsWith(" ") )
	{
		n = n.left(n.size()-1) ;			
	}
  ui->FM4Name->setText( n ) ;
  n = g_model.phaseData[4].name ;
	while ( n.endsWith(" ") )
	{
		n = n.left(n.size()-1) ;			
	}
  ui->FM5Name->setText( n ) ;
  n = g_model.phaseData[5].name ;
	while ( n.endsWith(" ") )
	{
		n = n.left(n.size()-1) ;			
	}
  ui->FM6Name->setText( n ) ;

	phaseEdited() ;

}


void ModelEdit::phaseSet(int phase, int trim, QComboBox *cb, QSpinBox *sb )
{
	int idx ;
  if ( (idx = decodePhaseTrim( &g_model.phaseData[phase].trim[trim], cb->currentIndex() ) < 0 ) )
	{
		g_model.phaseData[phase].trim[trim] = sb->value() ;
		sb->setEnabled( true ) ;
	}
	else
	{
    sb->setValue( idx ? g_model.phaseData[idx].trim[trim] : g_model.trim[trim] ) ;		// Needs recursion added
		sb->setDisabled( true ) ;
	}
}

void ModelEdit::phaseEdited()
{
//  int idx ;
//	int limit = MAX_DRSWITCH ;
//#ifdef SKY
//	if ( rData->type )
//	{
//		limit = MAX_XDRSWITCH ;
//	}
//#endif
  g_model.phaseData[0].swtch = getSwitchCbValueShort( ui->FP1_sw, rData->type ) ;
  g_model.phaseData[1].swtch = getSwitchCbValueShort( ui->FP2_sw, rData->type ) ;
  g_model.phaseData[2].swtch = getSwitchCbValueShort( ui->FP3_sw, rData->type ) ;
  g_model.phaseData[3].swtch = getSwitchCbValueShort( ui->FP4_sw, rData->type ) ;
  g_model.phaseData[4].swtch = getSwitchCbValueShort( ui->FP5_sw, rData->type ) ;
  g_model.phaseData[5].swtch = getSwitchCbValueShort( ui->FP6_sw, rData->type ) ;

  textUpdate( ui->FM1Name, g_model.phaseData[0].name, 6 ) ;
  textUpdate( ui->FM2Name, g_model.phaseData[1].name, 6 ) ;
  textUpdate( ui->FM3Name, g_model.phaseData[2].name, 6 ) ;
  textUpdate( ui->FM4Name, g_model.phaseData[3].name, 6 ) ;
  textUpdate( ui->FM5Name, g_model.phaseData[4].name, 6 ) ;
  textUpdate( ui->FM6Name, g_model.phaseData[5].name, 6 ) ;

	phaseSet(0, 0, ui->FP1_RudCB, ui->FP1rudTrimSB ) ;
	phaseSet(0, 1, ui->FP1_EleCB, ui->FP1eleTrimSB ) ;
	phaseSet(0, 2, ui->FP1_ThrCB, ui->FP1thrTrimSB ) ;
	phaseSet(0, 3, ui->FP1_AilCB, ui->FP1ailTrimSB ) ;
	phaseSet(1, 0, ui->FP2_RudCB, ui->FP2rudTrimSB ) ;
	phaseSet(1, 1, ui->FP2_EleCB, ui->FP2eleTrimSB ) ;
	phaseSet(1, 2, ui->FP2_ThrCB, ui->FP2thrTrimSB ) ;
	phaseSet(1, 3, ui->FP2_AilCB, ui->FP2ailTrimSB ) ;
	phaseSet(2, 0, ui->FP3_RudCB, ui->FP3rudTrimSB ) ;
	phaseSet(2, 1, ui->FP3_EleCB, ui->FP3eleTrimSB ) ;
	phaseSet(2, 2, ui->FP3_ThrCB, ui->FP3thrTrimSB ) ;
	phaseSet(2, 3, ui->FP3_AilCB, ui->FP3ailTrimSB ) ;
	phaseSet(3, 0, ui->FP4_RudCB, ui->FP4rudTrimSB ) ;
	phaseSet(3, 1, ui->FP4_EleCB, ui->FP4eleTrimSB ) ;
	phaseSet(3, 2, ui->FP4_ThrCB, ui->FP4thrTrimSB ) ;
	phaseSet(3, 3, ui->FP4_AilCB, ui->FP4ailTrimSB ) ;
	phaseSet(4, 0, ui->FP5_RudCB, ui->FP5rudTrimSB ) ;
	phaseSet(4, 1, ui->FP5_EleCB, ui->FP5eleTrimSB ) ;
	phaseSet(4, 2, ui->FP5_ThrCB, ui->FP5thrTrimSB ) ;
	phaseSet(4, 3, ui->FP5_AilCB, ui->FP5ailTrimSB ) ;
	phaseSet(5, 0, ui->FP6_RudCB, ui->FP6rudTrimSB ) ;
	phaseSet(5, 1, ui->FP6_EleCB, ui->FP6eleTrimSB ) ;
	phaseSet(5, 2, ui->FP6_ThrCB, ui->FP6thrTrimSB ) ;
	phaseSet(5, 3, ui->FP6_AilCB, ui->FP6ailTrimSB ) ;

	g_model.phaseData[0].fadeIn = ( ui->FM1FadeIn->value() + 0.01 ) * 2 ;
	g_model.phaseData[0].fadeOut = ( ui->FM1FadeOut->value() + 0.01 ) * 2 ;
	g_model.phaseData[1].fadeIn = ( ui->FM2FadeIn->value() + 0.01 ) * 2 ;
	g_model.phaseData[1].fadeOut = ( ui->FM2FadeOut->value() + 0.01 ) * 2 ;
	g_model.phaseData[2].fadeIn = ( ui->FM3FadeIn->value() + 0.01 ) * 2 ;
	g_model.phaseData[2].fadeOut = ( ui->FM3FadeOut->value() + 0.01 ) * 2 ;
	g_model.phaseData[3].fadeIn = ( ui->FM4FadeIn->value() + 0.01 ) * 2 ;
	g_model.phaseData[3].fadeOut = ( ui->FM4FadeOut->value() + 0.01 ) * 2 ;
	g_model.phaseData[4].fadeIn = ( ui->FM5FadeIn->value() + 0.01 ) * 2 ;
	g_model.phaseData[4].fadeOut = ( ui->FM5FadeOut->value() + 0.01 ) * 2 ;
	g_model.phaseData[5].fadeIn = ( ui->FM6FadeIn->value() + 0.01 ) * 2 ;
	g_model.phaseData[5].fadeOut = ( ui->FM6FadeOut->value() + 0.01 ) * 2 ;

  updateSettings();
}

void ModelEdit::tabHeli()
{
    updateHeliTab();

    connect(ui->swashTypeCB,SIGNAL(currentIndexChanged(int)),this,SLOT(heliEdited()));
    connect(ui->swashCollectiveCB,SIGNAL(currentIndexChanged(int)),this,SLOT(heliEdited()));
    connect(ui->swashRingValSB,SIGNAL(editingFinished()),this,SLOT(heliEdited()));
    connect(ui->swashInvertELE,SIGNAL(stateChanged(int)),this,SLOT(heliEdited()));
    connect(ui->swashInvertAIL,SIGNAL(stateChanged(int)),this,SLOT(heliEdited()));
    connect(ui->swashInvertCOL,SIGNAL(stateChanged(int)),this,SLOT(heliEdited()));
}

void ModelEdit::updateHeliTab()
{
    heliEditLock = true;

    ui->swashTypeCB->setCurrentIndex(g_model.swashType);
    populateSourceCB(ui->swashCollectiveCB,g_eeGeneral.stickMode,0,g_model.swashCollectiveSource,g_model.modelVersion, rData->type);
    ui->swashRingValSB->setValue(g_model.swashRingValue);
    ui->swashInvertELE->setChecked(g_model.swashInvertELE);
    ui->swashInvertAIL->setChecked(g_model.swashInvertAIL);
    ui->swashInvertCOL->setChecked(g_model.swashInvertCOL);

    heliEditLock = false;
}

void ModelEdit::heliEdited()
{
    if(heliEditLock) return;
    g_model.swashType  = ui->swashTypeCB->currentIndex();
    uint32_t value ;
  	value = decodePots( ui->swashCollectiveCB->currentIndex(), rData->type ) ;
		g_model.swashCollectiveSource = value ;
		g_model.swashRingValue = ui->swashRingValSB->value();
    g_model.swashInvertELE = ui->swashInvertELE->isChecked();
    g_model.swashInvertAIL = ui->swashInvertAIL->isChecked();
    g_model.swashInvertCOL = ui->swashInvertCOL->isChecked();
    updateSettings();
}

void ModelEdit::tabLimits()
{
    ui->offsetDSB_1->setValue((double)g_model.limitData[0].offset/10 + 0.049);   connect(ui->offsetDSB_1,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_2->setValue((double)g_model.limitData[1].offset/10 + 0.049);   connect(ui->offsetDSB_2,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_3->setValue((double)g_model.limitData[2].offset/10 + 0.049);   connect(ui->offsetDSB_3,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_4->setValue((double)g_model.limitData[3].offset/10 + 0.049);   connect(ui->offsetDSB_4,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_5->setValue((double)g_model.limitData[4].offset/10 + 0.049);   connect(ui->offsetDSB_5,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_6->setValue((double)g_model.limitData[5].offset/10 + 0.049);   connect(ui->offsetDSB_6,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_7->setValue((double)g_model.limitData[6].offset/10 + 0.049);   connect(ui->offsetDSB_7,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_8->setValue((double)g_model.limitData[7].offset/10 + 0.049);   connect(ui->offsetDSB_8,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_9->setValue((double)g_model.limitData[8].offset/10 + 0.049);   connect(ui->offsetDSB_9,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_10->setValue((double)g_model.limitData[9].offset/10 + 0.049);  connect(ui->offsetDSB_10,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_11->setValue((double)g_model.limitData[10].offset/10 + 0.049); connect(ui->offsetDSB_11,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_12->setValue((double)g_model.limitData[11].offset/10 + 0.049); connect(ui->offsetDSB_12,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_13->setValue((double)g_model.limitData[12].offset/10 + 0.049); connect(ui->offsetDSB_13,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_14->setValue((double)g_model.limitData[13].offset/10 + 0.049); connect(ui->offsetDSB_14,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_15->setValue((double)g_model.limitData[14].offset/10 + 0.049); connect(ui->offsetDSB_15,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_16->setValue((double)g_model.limitData[15].offset/10 + 0.049); connect(ui->offsetDSB_16,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_17->setValue((double)g_model.limitData[16].offset/10 + 0.049); connect(ui->offsetDSB_17,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_18->setValue((double)g_model.limitData[17].offset/10 + 0.049); connect(ui->offsetDSB_18,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_19->setValue((double)g_model.limitData[18].offset/10 + 0.049); connect(ui->offsetDSB_19,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_20->setValue((double)g_model.limitData[19].offset/10 + 0.049); connect(ui->offsetDSB_20,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_21->setValue((double)g_model.limitData[20].offset/10 + 0.049); connect(ui->offsetDSB_21,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_22->setValue((double)g_model.limitData[21].offset/10 + 0.049); connect(ui->offsetDSB_22,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_23->setValue((double)g_model.limitData[22].offset/10 + 0.049); connect(ui->offsetDSB_23,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));
    ui->offsetDSB_24->setValue((double)g_model.limitData[23].offset/10 + 0.049); connect(ui->offsetDSB_24,SIGNAL(valueChanged(double)),this,SLOT(limitEdited()));

    ui->minSB_1->setValue(g_model.limitData[0].min-100);   connect(ui->minSB_1,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_2->setValue(g_model.limitData[1].min-100);   connect(ui->minSB_2,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_3->setValue(g_model.limitData[2].min-100);   connect(ui->minSB_3,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_4->setValue(g_model.limitData[3].min-100);   connect(ui->minSB_4,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_5->setValue(g_model.limitData[4].min-100);   connect(ui->minSB_5,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_6->setValue(g_model.limitData[5].min-100);   connect(ui->minSB_6,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_7->setValue(g_model.limitData[6].min-100);   connect(ui->minSB_7,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_8->setValue(g_model.limitData[7].min-100);   connect(ui->minSB_8,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_9->setValue(g_model.limitData[8].min-100);   connect(ui->minSB_9,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_10->setValue(g_model.limitData[9].min-100);  connect(ui->minSB_10,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_11->setValue(g_model.limitData[10].min-100); connect(ui->minSB_11,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_12->setValue(g_model.limitData[11].min-100); connect(ui->minSB_12,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_13->setValue(g_model.limitData[12].min-100); connect(ui->minSB_13,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_14->setValue(g_model.limitData[13].min-100); connect(ui->minSB_14,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_15->setValue(g_model.limitData[14].min-100); connect(ui->minSB_15,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_16->setValue(g_model.limitData[15].min-100); connect(ui->minSB_16,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_17->setValue(g_model.limitData[16].min-100); connect(ui->minSB_17,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_18->setValue(g_model.limitData[17].min-100); connect(ui->minSB_18,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_19->setValue(g_model.limitData[18].min-100); connect(ui->minSB_19,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_20->setValue(g_model.limitData[19].min-100); connect(ui->minSB_20,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_21->setValue(g_model.limitData[20].min-100); connect(ui->minSB_21,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_22->setValue(g_model.limitData[21].min-100); connect(ui->minSB_22,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_23->setValue(g_model.limitData[22].min-100); connect(ui->minSB_23,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->minSB_24->setValue(g_model.limitData[23].min-100); connect(ui->minSB_24,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));

    ui->maxSB_1->setValue(g_model.limitData[0].max+100);   connect(ui->maxSB_1,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_2->setValue(g_model.limitData[1].max+100);   connect(ui->maxSB_2,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_3->setValue(g_model.limitData[2].max+100);   connect(ui->maxSB_3,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_4->setValue(g_model.limitData[3].max+100);   connect(ui->maxSB_4,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_5->setValue(g_model.limitData[4].max+100);   connect(ui->maxSB_5,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_6->setValue(g_model.limitData[5].max+100);   connect(ui->maxSB_6,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_7->setValue(g_model.limitData[6].max+100);   connect(ui->maxSB_7,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_8->setValue(g_model.limitData[7].max+100);   connect(ui->maxSB_8,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_9->setValue(g_model.limitData[8].max+100);   connect(ui->maxSB_9,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_10->setValue(g_model.limitData[9].max+100);  connect(ui->maxSB_10,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_11->setValue(g_model.limitData[10].max+100); connect(ui->maxSB_11,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_12->setValue(g_model.limitData[11].max+100); connect(ui->maxSB_12,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_13->setValue(g_model.limitData[12].max+100); connect(ui->maxSB_13,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_14->setValue(g_model.limitData[13].max+100); connect(ui->maxSB_14,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_15->setValue(g_model.limitData[14].max+100); connect(ui->maxSB_15,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_16->setValue(g_model.limitData[15].max+100); connect(ui->maxSB_16,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_17->setValue(g_model.limitData[16].max+100); connect(ui->maxSB_17,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_18->setValue(g_model.limitData[17].max+100); connect(ui->maxSB_18,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_19->setValue(g_model.limitData[18].max+100); connect(ui->maxSB_19,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_20->setValue(g_model.limitData[19].max+100); connect(ui->maxSB_20,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_21->setValue(g_model.limitData[20].max+100); connect(ui->maxSB_21,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_22->setValue(g_model.limitData[21].max+100); connect(ui->maxSB_22,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_23->setValue(g_model.limitData[22].max+100); connect(ui->maxSB_23,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));
    ui->maxSB_24->setValue(g_model.limitData[23].max+100); connect(ui->maxSB_24,SIGNAL(valueChanged(int)),this,SLOT(limitEdited()));

    ui->chInvCB_1->setCurrentIndex((g_model.limitData[0].revert) ? 1 : 0);   connect(ui->chInvCB_1,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_2->setCurrentIndex((g_model.limitData[1].revert) ? 1 : 0);   connect(ui->chInvCB_2,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_3->setCurrentIndex((g_model.limitData[2].revert) ? 1 : 0);   connect(ui->chInvCB_3,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_4->setCurrentIndex((g_model.limitData[3].revert) ? 1 : 0);   connect(ui->chInvCB_4,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_5->setCurrentIndex((g_model.limitData[4].revert) ? 1 : 0);   connect(ui->chInvCB_5,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_6->setCurrentIndex((g_model.limitData[5].revert) ? 1 : 0);   connect(ui->chInvCB_6,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_7->setCurrentIndex((g_model.limitData[6].revert) ? 1 : 0);   connect(ui->chInvCB_7,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_8->setCurrentIndex((g_model.limitData[7].revert) ? 1 : 0);   connect(ui->chInvCB_8,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_9->setCurrentIndex((g_model.limitData[8].revert) ? 1 : 0);   connect(ui->chInvCB_9,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_10->setCurrentIndex((g_model.limitData[9].revert) ? 1 : 0);  connect(ui->chInvCB_10,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_11->setCurrentIndex((g_model.limitData[10].revert) ? 1 : 0); connect(ui->chInvCB_11,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_12->setCurrentIndex((g_model.limitData[11].revert) ? 1 : 0); connect(ui->chInvCB_12,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_13->setCurrentIndex((g_model.limitData[12].revert) ? 1 : 0); connect(ui->chInvCB_13,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_14->setCurrentIndex((g_model.limitData[13].revert) ? 1 : 0); connect(ui->chInvCB_14,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_15->setCurrentIndex((g_model.limitData[14].revert) ? 1 : 0); connect(ui->chInvCB_15,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_16->setCurrentIndex((g_model.limitData[15].revert) ? 1 : 0); connect(ui->chInvCB_16,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_17->setCurrentIndex((g_model.limitData[16].revert) ? 1 : 0); connect(ui->chInvCB_17,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_18->setCurrentIndex((g_model.limitData[17].revert) ? 1 : 0); connect(ui->chInvCB_18,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_19->setCurrentIndex((g_model.limitData[18].revert) ? 1 : 0); connect(ui->chInvCB_19,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_20->setCurrentIndex((g_model.limitData[19].revert) ? 1 : 0); connect(ui->chInvCB_20,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_21->setCurrentIndex((g_model.limitData[20].revert) ? 1 : 0); connect(ui->chInvCB_21,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_22->setCurrentIndex((g_model.limitData[21].revert) ? 1 : 0); connect(ui->chInvCB_22,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_23->setCurrentIndex((g_model.limitData[22].revert) ? 1 : 0); connect(ui->chInvCB_23,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));
    ui->chInvCB_24->setCurrentIndex((g_model.limitData[23].revert) ? 1 : 0); connect(ui->chInvCB_24,SIGNAL(currentIndexChanged(int)),this,SLOT(limitEdited()));

		limitAuto() ;

    setLimitMinMax();
}

void ModelEdit::updateCurvesTab()
{
   ControlCurveSignal(true);
   ui->plotCB_1->setChecked(plot_curve[0]);
   ui->plotCB_2->setChecked(plot_curve[1]);
   ui->plotCB_3->setChecked(plot_curve[2]);
   ui->plotCB_4->setChecked(plot_curve[3]);
   ui->plotCB_5->setChecked(plot_curve[4]);
   ui->plotCB_6->setChecked(plot_curve[5]);
   ui->plotCB_7->setChecked(plot_curve[6]);
   ui->plotCB_8->setChecked(plot_curve[7]);
   ui->plotCB_9->setChecked(plot_curve[8]);
   ui->plotCB_10->setChecked(plot_curve[9]);
   ui->plotCB_11->setChecked(plot_curve[10]);
   ui->plotCB_12->setChecked(plot_curve[11]);
   ui->plotCB_13->setChecked(plot_curve[12]);
   ui->plotCB_14->setChecked(plot_curve[13]);
   ui->plotCB_15->setChecked(plot_curve[14]);
   ui->plotCB_16->setChecked(plot_curve[15]);
   
	 ui->curvePt1_1->setValue(g_model.curves5[0][0]);
   ui->curvePt2_1->setValue(g_model.curves5[0][1]);
   ui->curvePt3_1->setValue(g_model.curves5[0][2]);
   ui->curvePt4_1->setValue(g_model.curves5[0][3]);
   ui->curvePt5_1->setValue(g_model.curves5[0][4]);

   ui->curvePt1_2->setValue(g_model.curves5[1][0]);
   ui->curvePt2_2->setValue(g_model.curves5[1][1]);
   ui->curvePt3_2->setValue(g_model.curves5[1][2]);
   ui->curvePt4_2->setValue(g_model.curves5[1][3]);
   ui->curvePt5_2->setValue(g_model.curves5[1][4]);

   ui->curvePt1_3->setValue(g_model.curves5[2][0]);
   ui->curvePt2_3->setValue(g_model.curves5[2][1]);
   ui->curvePt3_3->setValue(g_model.curves5[2][2]);
   ui->curvePt4_3->setValue(g_model.curves5[2][3]);
   ui->curvePt5_3->setValue(g_model.curves5[2][4]);

   ui->curvePt1_4->setValue(g_model.curves5[3][0]);
   ui->curvePt2_4->setValue(g_model.curves5[3][1]);
   ui->curvePt3_4->setValue(g_model.curves5[3][2]);
   ui->curvePt4_4->setValue(g_model.curves5[3][3]);
   ui->curvePt5_4->setValue(g_model.curves5[3][4]);

   ui->curvePt1_5->setValue(g_model.curves5[4][0]);
   ui->curvePt2_5->setValue(g_model.curves5[4][1]);
   ui->curvePt3_5->setValue(g_model.curves5[4][2]);
   ui->curvePt4_5->setValue(g_model.curves5[4][3]);
   ui->curvePt5_5->setValue(g_model.curves5[4][4]);

   ui->curvePt1_6->setValue(g_model.curves5[5][0]);
   ui->curvePt2_6->setValue(g_model.curves5[5][1]);
   ui->curvePt3_6->setValue(g_model.curves5[5][2]);
   ui->curvePt4_6->setValue(g_model.curves5[5][3]);
   ui->curvePt5_6->setValue(g_model.curves5[5][4]);

   ui->curvePt1_7->setValue(g_model.curves5[6][0]);
   ui->curvePt2_7->setValue(g_model.curves5[6][1]);
   ui->curvePt3_7->setValue(g_model.curves5[6][2]);
   ui->curvePt4_7->setValue(g_model.curves5[6][3]);
   ui->curvePt5_7->setValue(g_model.curves5[6][4]);

   ui->curvePt1_8->setValue(g_model.curves5[7][0]);
   ui->curvePt2_8->setValue(g_model.curves5[7][1]);
   ui->curvePt3_8->setValue(g_model.curves5[7][2]);
   ui->curvePt4_8->setValue(g_model.curves5[7][3]);
   ui->curvePt5_8->setValue(g_model.curves5[7][4]);

   ui->curvePt1_9->setValue(g_model.curves9[0][0]);
   ui->curvePt2_9->setValue(g_model.curves9[0][1]);
   ui->curvePt3_9->setValue(g_model.curves9[0][2]);
   ui->curvePt4_9->setValue(g_model.curves9[0][3]);
   ui->curvePt5_9->setValue(g_model.curves9[0][4]);
   ui->curvePt6_9->setValue(g_model.curves9[0][5]);
   ui->curvePt7_9->setValue(g_model.curves9[0][6]);
   ui->curvePt8_9->setValue(g_model.curves9[0][7]);
   ui->curvePt9_9->setValue(g_model.curves9[0][8]);

   ui->curvePt1_10->setValue(g_model.curves9[1][0]);
   ui->curvePt2_10->setValue(g_model.curves9[1][1]);
   ui->curvePt3_10->setValue(g_model.curves9[1][2]);
   ui->curvePt4_10->setValue(g_model.curves9[1][3]);
   ui->curvePt5_10->setValue(g_model.curves9[1][4]);
   ui->curvePt6_10->setValue(g_model.curves9[1][5]);
   ui->curvePt7_10->setValue(g_model.curves9[1][6]);
   ui->curvePt8_10->setValue(g_model.curves9[1][7]);
   ui->curvePt9_10->setValue(g_model.curves9[1][8]);

   ui->curvePt1_11->setValue(g_model.curves9[2][0]);
   ui->curvePt2_11->setValue(g_model.curves9[2][1]);
   ui->curvePt3_11->setValue(g_model.curves9[2][2]);
   ui->curvePt4_11->setValue(g_model.curves9[2][3]);
   ui->curvePt5_11->setValue(g_model.curves9[2][4]);
   ui->curvePt6_11->setValue(g_model.curves9[2][5]);
   ui->curvePt7_11->setValue(g_model.curves9[2][6]);
   ui->curvePt8_11->setValue(g_model.curves9[2][7]);
   ui->curvePt9_11->setValue(g_model.curves9[2][8]);

   ui->curvePt1_12->setValue(g_model.curves9[3][0]);
   ui->curvePt2_12->setValue(g_model.curves9[3][1]);
   ui->curvePt3_12->setValue(g_model.curves9[3][2]);
   ui->curvePt4_12->setValue(g_model.curves9[3][3]);
   ui->curvePt5_12->setValue(g_model.curves9[3][4]);
   ui->curvePt6_12->setValue(g_model.curves9[3][5]);
   ui->curvePt7_12->setValue(g_model.curves9[3][6]);
   ui->curvePt8_12->setValue(g_model.curves9[3][7]);
   ui->curvePt9_12->setValue(g_model.curves9[3][8]);

   ui->curvePt1_13->setValue(g_model.curves9[4][0]);
   ui->curvePt2_13->setValue(g_model.curves9[4][1]);
   ui->curvePt3_13->setValue(g_model.curves9[4][2]);
   ui->curvePt4_13->setValue(g_model.curves9[4][3]);
   ui->curvePt5_13->setValue(g_model.curves9[4][4]);
   ui->curvePt6_13->setValue(g_model.curves9[4][5]);
   ui->curvePt7_13->setValue(g_model.curves9[4][6]);
   ui->curvePt8_13->setValue(g_model.curves9[4][7]);
   ui->curvePt9_13->setValue(g_model.curves9[4][8]);

   ui->curvePt1_14->setValue(g_model.curves9[5][0]);
   ui->curvePt2_14->setValue(g_model.curves9[5][1]);
   ui->curvePt3_14->setValue(g_model.curves9[5][2]);
   ui->curvePt4_14->setValue(g_model.curves9[5][3]);
   ui->curvePt5_14->setValue(g_model.curves9[5][4]);
   ui->curvePt6_14->setValue(g_model.curves9[5][5]);
   ui->curvePt7_14->setValue(g_model.curves9[5][6]);
   ui->curvePt8_14->setValue(g_model.curves9[5][7]);
   ui->curvePt9_14->setValue(g_model.curves9[5][8]);

   ui->curvePt1_15->setValue(g_model.curves9[6][0]);
   ui->curvePt2_15->setValue(g_model.curves9[6][1]);
   ui->curvePt3_15->setValue(g_model.curves9[6][2]);
   ui->curvePt4_15->setValue(g_model.curves9[6][3]);
   ui->curvePt5_15->setValue(g_model.curves9[6][4]);
   ui->curvePt6_15->setValue(g_model.curves9[6][5]);
   ui->curvePt7_15->setValue(g_model.curves9[6][6]);
   ui->curvePt8_15->setValue(g_model.curves9[6][7]);
   ui->curvePt9_15->setValue(g_model.curves9[6][8]);

   ui->curvePt1_16->setValue(g_model.curves9[7][0]);
   ui->curvePt2_16->setValue(g_model.curves9[7][1]);
   ui->curvePt3_16->setValue(g_model.curves9[7][2]);
   ui->curvePt4_16->setValue(g_model.curves9[7][3]);
   ui->curvePt5_16->setValue(g_model.curves9[7][4]);
   ui->curvePt6_16->setValue(g_model.curves9[7][5]);
   ui->curvePt7_16->setValue(g_model.curves9[7][6]);
   ui->curvePt8_16->setValue(g_model.curves9[7][7]);
   ui->curvePt9_16->setValue(g_model.curves9[7][8]);
   ControlCurveSignal(false);
}


void ModelEdit::tabCurves()
{
   for (int i=0; i<16;i++)
	 {
     plot_curve[i]=false;
   }
   redrawCurve=true;
   updateCurvesTab();

   QGraphicsScene *scene = new QGraphicsScene(ui->curvePreview);
   scene->setItemIndexMethod(QGraphicsScene::NoIndex);
   ui->curvePreview->setScene(scene);
   currentCurve = 0;

   connect(ui->clearMixesPB,SIGNAL(pressed()),this,SLOT(clearCurves()));

   connect(ui->curvePt1_1,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_1,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_1,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_1,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_1,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_2,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_2,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_2,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_2,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_2,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_3,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_3,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_3,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_3,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_3,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_4,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_4,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_4,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_4,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_4,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_5,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_5,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_5,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_5,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_5,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_6,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_6,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_6,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_6,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_6,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_7,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_7,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_7,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_7,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_7,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_8,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_8,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_8,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_8,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_8,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_9,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_9,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_9,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_9,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_9,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt6_9,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt7_9,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt8_9,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt9_9,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_10,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_10,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_10,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_10,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_10,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt6_10,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt7_10,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt8_10,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt9_10,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_11,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_11,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_11,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_11,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_11,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt6_11,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt7_11,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt8_11,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt9_11,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_12,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_12,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_12,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_12,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_12,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt6_12,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt7_12,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt8_12,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt9_12,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_13,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_13,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_13,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_13,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_13,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt6_13,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt7_13,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt8_13,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt9_13,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_14,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_14,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_14,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_14,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_14,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt6_14,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt7_14,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt8_14,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt9_14,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_15,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_15,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_15,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_15,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_15,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt6_15,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt7_15,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt8_15,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt9_15,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));

   connect(ui->curvePt1_16,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt2_16,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt3_16,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt4_16,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt5_16,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt6_16,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt7_16,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt8_16,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
   connect(ui->curvePt9_16,SIGNAL(valueChanged(int)),this,SLOT(curvePointEdited()));
}

void ModelEdit::limitAuto()
{ // Set the values on the amin and Amax labels
	int value ;
	int i ;
	QLabel *Amin[24] ;
	QLabel *Amax[24] ;
	Amin[0] = ui->CH1Amin ;
	Amin[1] = ui->CH2Amin ;
	Amin[2] = ui->CH3Amin ;
	Amin[3] = ui->CH4Amin ;
	Amin[4] = ui->CH5Amin ;
	Amin[5] = ui->CH6Amin ;
	Amin[6] = ui->CH7Amin ;
	Amin[7] = ui->CH8Amin ;
	Amin[8] = ui->CH9Amin ;
	Amin[9] = ui->CH10Amin ;
	Amin[10] = ui->CH11Amin ;
	Amin[11] = ui->CH12Amin ;
	Amin[12] = ui->CH13Amin ;
	Amin[13] = ui->CH14Amin ;
	Amin[14] = ui->CH15Amin ;
	Amin[15] = ui->CH16Amin ;
	Amin[16] = ui->CH17Amin ;
	Amin[17] = ui->CH18Amin ;
	Amin[18] = ui->CH19Amin ;
	Amin[19] = ui->CH20Amin ;
	Amin[20] = ui->CH21Amin ;
	Amin[21] = ui->CH22Amin ;
	Amin[22] = ui->CH23Amin ;
	Amin[23] = ui->CH24Amin ;
	Amax[0] = ui->CH1Amax ;
	Amax[1] = ui->CH2Amax ;
	Amax[2] = ui->CH3Amax ;
	Amax[3] = ui->CH4Amax ;
	Amax[4] = ui->CH5Amax ;
	Amax[5] = ui->CH6Amax ;
	Amax[6] = ui->CH7Amax ;
	Amax[7] = ui->CH8Amax ;
	Amax[8] = ui->CH9Amax ;
	Amax[9] = ui->CH10Amax ;
	Amax[10] = ui->CH11Amax ;
	Amax[11] = ui->CH12Amax ;
	Amax[12] = ui->CH13Amax ;
	Amax[13] = ui->CH14Amax ;
	Amax[14] = ui->CH15Amax ;
	Amax[15] = ui->CH16Amax ;
	Amax[16] = ui->CH17Amax ;
	Amax[17] = ui->CH18Amax ;
	Amax[18] = ui->CH19Amax ;
	Amax[19] = ui->CH20Amax ;
	Amax[20] = ui->CH21Amax ;
	Amax[21] = ui->CH22Amax ;
	Amax[22] = ui->CH23Amax ;
	Amax[23] = ui->CH24Amax ;

	for ( i = 0 ; i < 24 ; i += 1 )
	{
		if (g_model.sub_trim_limit)
		{
			if ( ( value = g_model.limitData[i].offset ) )
			{
				if ( value > g_model.sub_trim_limit )
				{
					value = g_model.sub_trim_limit ;				
				}
				if ( value < -g_model.sub_trim_limit )
				{
					value = -g_model.sub_trim_limit ;				
				}
				Amin[i]->setText(tr("%1").arg( g_model.limitData[i].min-100+value/10 )) ;
				Amax[i]->setText(tr("%1").arg( g_model.limitData[i].max+100+value/10 )) ;
  	    Amin[i]->setVisible(true) ;
  	    Amax[i]->setVisible(true) ;
			}
			else
			{
  	  	Amin[i]->setVisible(false) ;
		    Amax[i]->setVisible(false) ;
			}
		}
		else
		{
  	  Amin[i]->setVisible(false) ;
  	  Amax[i]->setVisible(false) ;
		}
	}
}



void ModelEdit::limitEdited()
{
    g_model.limitData[0].offset = ui->offsetDSB_1->value()*10 + 0.49;
    g_model.limitData[1].offset = ui->offsetDSB_2->value()*10 + 0.49;
    g_model.limitData[2].offset = ui->offsetDSB_3->value()*10 + 0.49;
    g_model.limitData[3].offset = ui->offsetDSB_4->value()*10 + 0.49;
    g_model.limitData[4].offset = ui->offsetDSB_5->value()*10 + 0.49;
    g_model.limitData[5].offset = ui->offsetDSB_6->value()*10 + 0.49;
    g_model.limitData[6].offset = ui->offsetDSB_7->value()*10 + 0.49;
    g_model.limitData[7].offset = ui->offsetDSB_8->value()*10 + 0.49;
    g_model.limitData[8].offset = ui->offsetDSB_9->value()*10 + 0.49;
    g_model.limitData[9].offset = ui->offsetDSB_10->value()*10 + 0.49;
    g_model.limitData[10].offset = ui->offsetDSB_11->value()*10 + 0.49;
    g_model.limitData[11].offset = ui->offsetDSB_12->value()*10 + 0.49;
    g_model.limitData[12].offset = ui->offsetDSB_13->value()*10 + 0.49;
    g_model.limitData[13].offset = ui->offsetDSB_14->value()*10 + 0.49;
    g_model.limitData[14].offset = ui->offsetDSB_15->value()*10 + 0.49;
    g_model.limitData[15].offset = ui->offsetDSB_16->value()*10 + 0.49;
    g_model.limitData[16].offset = ui->offsetDSB_17->value()*10 + 0.49;
    g_model.limitData[17].offset = ui->offsetDSB_18->value()*10 + 0.49;
    g_model.limitData[18].offset = ui->offsetDSB_19->value()*10 + 0.49;
    g_model.limitData[19].offset = ui->offsetDSB_20->value()*10 + 0.49;
    g_model.limitData[20].offset = ui->offsetDSB_21->value()*10 + 0.49;
    g_model.limitData[21].offset = ui->offsetDSB_22->value()*10 + 0.49;
    g_model.limitData[22].offset = ui->offsetDSB_23->value()*10 + 0.49;
    g_model.limitData[23].offset = ui->offsetDSB_24->value()*10 + 0.49;

    g_model.limitData[0].min = ui->minSB_1->value()+100;
    g_model.limitData[1].min = ui->minSB_2->value()+100;
    g_model.limitData[2].min = ui->minSB_3->value()+100;
    g_model.limitData[3].min = ui->minSB_4->value()+100;
    g_model.limitData[4].min = ui->minSB_5->value()+100;
    g_model.limitData[5].min = ui->minSB_6->value()+100;
    g_model.limitData[6].min = ui->minSB_7->value()+100;
    g_model.limitData[7].min = ui->minSB_8->value()+100;
    g_model.limitData[8].min = ui->minSB_9->value()+100;
    g_model.limitData[9].min = ui->minSB_10->value()+100;
    g_model.limitData[10].min = ui->minSB_11->value()+100;
    g_model.limitData[11].min = ui->minSB_12->value()+100;
    g_model.limitData[12].min = ui->minSB_13->value()+100;
    g_model.limitData[13].min = ui->minSB_14->value()+100;
    g_model.limitData[14].min = ui->minSB_15->value()+100;
    g_model.limitData[15].min = ui->minSB_16->value()+100;
    g_model.limitData[16].min = ui->minSB_17->value()+100;
    g_model.limitData[17].min = ui->minSB_18->value()+100;
    g_model.limitData[18].min = ui->minSB_19->value()+100;
    g_model.limitData[19].min = ui->minSB_20->value()+100;
    g_model.limitData[20].min = ui->minSB_21->value()+100;
    g_model.limitData[21].min = ui->minSB_22->value()+100;
    g_model.limitData[22].min = ui->minSB_23->value()+100;
    g_model.limitData[23].min = ui->minSB_24->value()+100;

    g_model.limitData[0].max = ui->maxSB_1->value()-100;
    g_model.limitData[1].max = ui->maxSB_2->value()-100;
    g_model.limitData[2].max = ui->maxSB_3->value()-100;
    g_model.limitData[3].max = ui->maxSB_4->value()-100;
    g_model.limitData[4].max = ui->maxSB_5->value()-100;
    g_model.limitData[5].max = ui->maxSB_6->value()-100;
    g_model.limitData[6].max = ui->maxSB_7->value()-100;
    g_model.limitData[7].max = ui->maxSB_8->value()-100;
    g_model.limitData[8].max = ui->maxSB_9->value()-100;
    g_model.limitData[9].max = ui->maxSB_10->value()-100;
    g_model.limitData[10].max = ui->maxSB_11->value()-100;
    g_model.limitData[11].max = ui->maxSB_12->value()-100;
    g_model.limitData[12].max = ui->maxSB_13->value()-100;
    g_model.limitData[13].max = ui->maxSB_14->value()-100;
    g_model.limitData[14].max = ui->maxSB_15->value()-100;
    g_model.limitData[15].max = ui->maxSB_16->value()-100;
    g_model.limitData[16].max = ui->maxSB_17->value()-100;
    g_model.limitData[17].max = ui->maxSB_18->value()-100;
    g_model.limitData[18].max = ui->maxSB_19->value()-100;
    g_model.limitData[19].max = ui->maxSB_20->value()-100;
    g_model.limitData[20].max = ui->maxSB_21->value()-100;
    g_model.limitData[21].max = ui->maxSB_22->value()-100;
    g_model.limitData[22].max = ui->maxSB_23->value()-100;
    g_model.limitData[23].max = ui->maxSB_24->value()-100;

    g_model.limitData[0].revert = ui->chInvCB_1->currentIndex();
    g_model.limitData[1].revert = ui->chInvCB_2->currentIndex();
    g_model.limitData[2].revert = ui->chInvCB_3->currentIndex();
    g_model.limitData[3].revert = ui->chInvCB_4->currentIndex();
    g_model.limitData[4].revert = ui->chInvCB_5->currentIndex();
    g_model.limitData[5].revert = ui->chInvCB_6->currentIndex();
    g_model.limitData[6].revert = ui->chInvCB_7->currentIndex();
    g_model.limitData[7].revert = ui->chInvCB_8->currentIndex();
    g_model.limitData[8].revert = ui->chInvCB_9->currentIndex();
    g_model.limitData[9].revert = ui->chInvCB_10->currentIndex();
    g_model.limitData[10].revert = ui->chInvCB_11->currentIndex();
    g_model.limitData[11].revert = ui->chInvCB_12->currentIndex();
    g_model.limitData[12].revert = ui->chInvCB_13->currentIndex();
    g_model.limitData[13].revert = ui->chInvCB_14->currentIndex();
    g_model.limitData[14].revert = ui->chInvCB_15->currentIndex();
    g_model.limitData[15].revert = ui->chInvCB_16->currentIndex();
    g_model.limitData[16].revert = ui->chInvCB_17->currentIndex();
    g_model.limitData[17].revert = ui->chInvCB_18->currentIndex();
    g_model.limitData[18].revert = ui->chInvCB_19->currentIndex();
    g_model.limitData[19].revert = ui->chInvCB_20->currentIndex();
    g_model.limitData[20].revert = ui->chInvCB_21->currentIndex();
    g_model.limitData[21].revert = ui->chInvCB_22->currentIndex();
    g_model.limitData[22].revert = ui->chInvCB_23->currentIndex();
    g_model.limitData[23].revert = ui->chInvCB_24->currentIndex();

		limitAuto() ;

    updateSettings();
}

void ModelEdit::setCurrentCurve(int curveId)
{
    currentCurve = curveId;
    QString ss = "QSpinBox { background-color:rgb(255, 255, 127);}";

    QSpinBox* spn[][16] = {
          { ui->curvePt1_1, ui->curvePt2_1, ui->curvePt3_1, ui->curvePt4_1, ui->curvePt5_1 }
        , { ui->curvePt1_2, ui->curvePt2_2, ui->curvePt3_2, ui->curvePt4_2, ui->curvePt5_2 }
        , { ui->curvePt1_3, ui->curvePt2_3, ui->curvePt3_3, ui->curvePt4_3, ui->curvePt5_3 }
        , { ui->curvePt1_4, ui->curvePt2_4, ui->curvePt3_4, ui->curvePt4_4, ui->curvePt5_4 }
        , { ui->curvePt1_5, ui->curvePt2_5, ui->curvePt3_5, ui->curvePt4_5, ui->curvePt5_5 }
        , { ui->curvePt1_6, ui->curvePt2_6, ui->curvePt3_6, ui->curvePt4_6, ui->curvePt5_6 }
        , { ui->curvePt1_7, ui->curvePt2_7, ui->curvePt3_7, ui->curvePt4_7, ui->curvePt5_7 }
        , { ui->curvePt1_8, ui->curvePt2_8, ui->curvePt3_8, ui->curvePt4_8, ui->curvePt5_8 }
        , { ui->curvePt1_9, ui->curvePt2_9, ui->curvePt3_9, ui->curvePt4_9, ui->curvePt5_9, ui->curvePt6_9, ui->curvePt7_9, ui->curvePt8_9, ui->curvePt9_9 }
        , { ui->curvePt1_10, ui->curvePt2_10, ui->curvePt3_10, ui->curvePt4_10, ui->curvePt5_10, ui->curvePt6_10, ui->curvePt7_10, ui->curvePt8_10, ui->curvePt9_10 }
        , { ui->curvePt1_11, ui->curvePt2_11, ui->curvePt3_11, ui->curvePt4_11, ui->curvePt5_11, ui->curvePt6_11, ui->curvePt7_11, ui->curvePt8_11, ui->curvePt9_11 }
        , { ui->curvePt1_12, ui->curvePt2_12, ui->curvePt3_12, ui->curvePt4_12, ui->curvePt5_12, ui->curvePt6_12, ui->curvePt7_12, ui->curvePt8_12, ui->curvePt9_12 }
        , { ui->curvePt1_13, ui->curvePt2_13, ui->curvePt3_13, ui->curvePt4_13, ui->curvePt5_13, ui->curvePt6_13, ui->curvePt7_13, ui->curvePt8_13, ui->curvePt9_13 }
        , { ui->curvePt1_14, ui->curvePt2_14, ui->curvePt3_14, ui->curvePt4_14, ui->curvePt5_14, ui->curvePt6_14, ui->curvePt7_14, ui->curvePt8_14, ui->curvePt9_14 }
        , { ui->curvePt1_15, ui->curvePt2_15, ui->curvePt3_15, ui->curvePt4_15, ui->curvePt5_15, ui->curvePt6_15, ui->curvePt7_15, ui->curvePt8_15, ui->curvePt9_15 }
        , { ui->curvePt1_16, ui->curvePt2_16, ui->curvePt3_16, ui->curvePt4_16, ui->curvePt5_16, ui->curvePt6_16, ui->curvePt7_16, ui->curvePt8_16, ui->curvePt9_16 }
    };
    for (int i = 0; i < 16; i++)
    {
        int jMax = 5;
        if (i > 7) { jMax = 9; }
        for (int j = 0; j < jMax; j++)
        {
            if (curveId == i)
            {
                spn[i][j]->setStyleSheet(ss);
            }
            else
            {
                spn[i][j]->setStyleSheet("");
            }
        }
   }
}

void ModelEdit::curvePointEdited()
{
    
    QSpinBox *spinBox = qobject_cast<QSpinBox*>(sender());

    int curveId = spinBox->objectName().right(1).toInt() - 1;
    if (spinBox->objectName().right(2).left(1).toInt() == 1)
    {
        curveId += 10;
    }

    setCurrentCurve(curveId);

    g_model.curves5[0][0] = ui->curvePt1_1->value();
    g_model.curves5[0][1] = ui->curvePt2_1->value();
    g_model.curves5[0][2] = ui->curvePt3_1->value();
    g_model.curves5[0][3] = ui->curvePt4_1->value();
    g_model.curves5[0][4] = ui->curvePt5_1->value();

    g_model.curves5[1][0] = ui->curvePt1_2->value();
    g_model.curves5[1][1] = ui->curvePt2_2->value();
    g_model.curves5[1][2] = ui->curvePt3_2->value();
    g_model.curves5[1][3] = ui->curvePt4_2->value();
    g_model.curves5[1][4] = ui->curvePt5_2->value();

    g_model.curves5[2][0] = ui->curvePt1_3->value();
    g_model.curves5[2][1] = ui->curvePt2_3->value();
    g_model.curves5[2][2] = ui->curvePt3_3->value();
    g_model.curves5[2][3] = ui->curvePt4_3->value();
    g_model.curves5[2][4] = ui->curvePt5_3->value();

    g_model.curves5[3][0] = ui->curvePt1_4->value();
    g_model.curves5[3][1] = ui->curvePt2_4->value();
    g_model.curves5[3][2] = ui->curvePt3_4->value();
    g_model.curves5[3][3] = ui->curvePt4_4->value();
    g_model.curves5[3][4] = ui->curvePt5_4->value();

    g_model.curves5[4][0] = ui->curvePt1_5->value();
    g_model.curves5[4][1] = ui->curvePt2_5->value();
    g_model.curves5[4][2] = ui->curvePt3_5->value();
    g_model.curves5[4][3] = ui->curvePt4_5->value();
    g_model.curves5[4][4] = ui->curvePt5_5->value();

    g_model.curves5[5][0] = ui->curvePt1_6->value();
    g_model.curves5[5][1] = ui->curvePt2_6->value();
    g_model.curves5[5][2] = ui->curvePt3_6->value();
    g_model.curves5[5][3] = ui->curvePt4_6->value();
    g_model.curves5[5][4] = ui->curvePt5_6->value();

    g_model.curves5[6][0] = ui->curvePt1_7->value();
    g_model.curves5[6][1] = ui->curvePt2_7->value();
    g_model.curves5[6][2] = ui->curvePt3_7->value();
    g_model.curves5[6][3] = ui->curvePt4_7->value();
    g_model.curves5[6][4] = ui->curvePt5_7->value();

    g_model.curves5[7][0] = ui->curvePt1_8->value();
    g_model.curves5[7][1] = ui->curvePt2_8->value();
    g_model.curves5[7][2] = ui->curvePt3_8->value();
    g_model.curves5[7][3] = ui->curvePt4_8->value();
    g_model.curves5[7][4] = ui->curvePt5_8->value();


    g_model.curves9[0][0] = ui->curvePt1_9->value();
    g_model.curves9[0][1] = ui->curvePt2_9->value();
    g_model.curves9[0][2] = ui->curvePt3_9->value();
    g_model.curves9[0][3] = ui->curvePt4_9->value();
    g_model.curves9[0][4] = ui->curvePt5_9->value();
    g_model.curves9[0][5] = ui->curvePt6_9->value();
    g_model.curves9[0][6] = ui->curvePt7_9->value();
    g_model.curves9[0][7] = ui->curvePt8_9->value();
    g_model.curves9[0][8] = ui->curvePt9_9->value();

    g_model.curves9[1][0] = ui->curvePt1_10->value();
    g_model.curves9[1][1] = ui->curvePt2_10->value();
    g_model.curves9[1][2] = ui->curvePt3_10->value();
    g_model.curves9[1][3] = ui->curvePt4_10->value();
    g_model.curves9[1][4] = ui->curvePt5_10->value();
    g_model.curves9[1][5] = ui->curvePt6_10->value();
    g_model.curves9[1][6] = ui->curvePt7_10->value();
    g_model.curves9[1][7] = ui->curvePt8_10->value();
    g_model.curves9[1][8] = ui->curvePt9_10->value();

    g_model.curves9[2][0] = ui->curvePt1_11->value();
    g_model.curves9[2][1] = ui->curvePt2_11->value();
    g_model.curves9[2][2] = ui->curvePt3_11->value();
    g_model.curves9[2][3] = ui->curvePt4_11->value();
    g_model.curves9[2][4] = ui->curvePt5_11->value();
    g_model.curves9[2][5] = ui->curvePt6_11->value();
    g_model.curves9[2][6] = ui->curvePt7_11->value();
    g_model.curves9[2][7] = ui->curvePt8_11->value();
    g_model.curves9[2][8] = ui->curvePt9_11->value();

    g_model.curves9[3][0] = ui->curvePt1_12->value();
    g_model.curves9[3][1] = ui->curvePt2_12->value();
    g_model.curves9[3][2] = ui->curvePt3_12->value();
    g_model.curves9[3][3] = ui->curvePt4_12->value();
    g_model.curves9[3][4] = ui->curvePt5_12->value();
    g_model.curves9[3][5] = ui->curvePt6_12->value();
    g_model.curves9[3][6] = ui->curvePt7_12->value();
    g_model.curves9[3][7] = ui->curvePt8_12->value();
    g_model.curves9[3][8] = ui->curvePt9_12->value();

    g_model.curves9[4][0] = ui->curvePt1_13->value();
    g_model.curves9[4][1] = ui->curvePt2_13->value();
    g_model.curves9[4][2] = ui->curvePt3_13->value();
    g_model.curves9[4][3] = ui->curvePt4_13->value();
    g_model.curves9[4][4] = ui->curvePt5_13->value();
    g_model.curves9[4][5] = ui->curvePt6_13->value();
    g_model.curves9[4][6] = ui->curvePt7_13->value();
    g_model.curves9[4][7] = ui->curvePt8_13->value();
    g_model.curves9[4][8] = ui->curvePt9_13->value();

    g_model.curves9[5][0] = ui->curvePt1_14->value();
    g_model.curves9[5][1] = ui->curvePt2_14->value();
    g_model.curves9[5][2] = ui->curvePt3_14->value();
    g_model.curves9[5][3] = ui->curvePt4_14->value();
    g_model.curves9[5][4] = ui->curvePt5_14->value();
    g_model.curves9[5][5] = ui->curvePt6_14->value();
    g_model.curves9[5][6] = ui->curvePt7_14->value();
    g_model.curves9[5][7] = ui->curvePt8_14->value();
    g_model.curves9[5][8] = ui->curvePt9_14->value();

    g_model.curves9[6][0] = ui->curvePt1_15->value();
    g_model.curves9[6][1] = ui->curvePt2_15->value();
    g_model.curves9[6][2] = ui->curvePt3_15->value();
    g_model.curves9[6][3] = ui->curvePt4_15->value();
    g_model.curves9[6][4] = ui->curvePt5_15->value();
    g_model.curves9[6][5] = ui->curvePt6_15->value();
    g_model.curves9[6][6] = ui->curvePt7_15->value();
    g_model.curves9[6][7] = ui->curvePt8_15->value();
    g_model.curves9[6][8] = ui->curvePt9_15->value();

    g_model.curves9[7][0] = ui->curvePt1_16->value();
    g_model.curves9[7][1] = ui->curvePt2_16->value();
    g_model.curves9[7][2] = ui->curvePt3_16->value();
    g_model.curves9[7][3] = ui->curvePt4_16->value();
    g_model.curves9[7][4] = ui->curvePt5_16->value();
    g_model.curves9[7][5] = ui->curvePt6_16->value();
    g_model.curves9[7][6] = ui->curvePt7_16->value();
    g_model.curves9[7][7] = ui->curvePt8_16->value();
    g_model.curves9[7][8] = ui->curvePt9_16->value();

    if (redrawCurve)
    {
        drawCurve();
    }
    updateSettings();
}


void ModelEdit::setSwitchWidgetVisibility(int i)
{
	char telText[20] ;
	int16_t value ;
    
    switch ( CS_STATE(g_model.customSw[i].func, g_model.modelVersion) )
    {
    case CS_VOFS:
        cswitchSource1[i]->setVisible(true);
        cswitchSource2[i]->setVisible(false);
        cswitchOffset[i]->setVisible(true);
        if ( cswitchOffset[i]->maximum() != 125 )
				{
        	cswitchOffset[i]->setMaximum(125);
        	cswitchOffset[i]->setMinimum(-125);
				}
        cswitchOffset[i]->setAccelerated(true);
        cswitchOffset0[i]->setVisible(false);
        populateSourceCB(cswitchSource1[i],g_eeGeneral.stickMode,1,g_model.customSw[i].v1,g_model.modelVersion, rData->type);
        cswitchOffset[i]->setValue(g_model.customSw[i].v2);
				if ( cswitchSource1[i]->currentIndex() > 36 )
				{
        	cswitchTlabel[i]->setVisible(true);
					value = convertTelemConstant( cswitchSource1[i]->currentIndex() - 45, g_model.customSw[i].v2, &g_model ) ;
          stringTelemetryChannel( telText, g_model.customSw[i].v1 - 45, value, &g_model ) ;
//					sprintf( telText, "%d", value ) ;
        	cswitchTlabel[i]->setText(telText);
				}
				else
				{
        	cswitchTlabel[i]->setVisible(false);
				}
				cswitchText1[i]->setVisible(false) ;
				cswitchText2[i]->setVisible(false) ;
        break;
    case CS_VBOOL:
        cswitchSource1[i]->setVisible(true);
        cswitchSource2[i]->setVisible(true);
        cswitchOffset[i]->setVisible(false);
        cswitchOffset0[i]->setVisible(false);
        populateSwitchCB(cswitchSource1[i],g_model.customSw[i].v1, rData->type);
        populateSwitchCB(cswitchSource2[i],g_model.customSw[i].v2, rData->type);
				cswitchText1[i]->setVisible(false) ;
				cswitchText2[i]->setVisible(false) ;
        break;
    case CS_VCOMP:
        cswitchSource1[i]->setVisible(true);
        cswitchSource2[i]->setVisible(true);
        cswitchOffset[i]->setVisible(false);
        cswitchOffset0[i]->setVisible(false);
        populateSourceCB(cswitchSource1[i],g_eeGeneral.stickMode,1,g_model.customSw[i].v1,g_model.modelVersion, rData->type);
        populateSourceCB(cswitchSource2[i],g_eeGeneral.stickMode,1,g_model.customSw[i].v2,g_model.modelVersion, rData->type);
				cswitchText1[i]->setVisible(false) ;
				cswitchText2[i]->setVisible(false) ;
        break;
    case CS_TIMER:
        if ( cswitchOffset[i]->maximum() != 100 )
				{
	        cswitchOffset[i]->setMaximum(100);
  	      cswitchOffset[i]->setMinimum(-49);
				}
        cswitchSource1[i]->setVisible(false);
        cswitchSource2[i]->setVisible(false);
        cswitchOffset[i]->setVisible(true);
        cswitchOffset[i]->setAccelerated(true);
        cswitchOffset0[i]->setVisible(true);
				value = g_model.customSw[i].v2+1 ;
        cswitchOffset[i]->setValue(value);
				if ( value <= 0 )
				{
					cswitchText2[i]->setVisible(true) ;
					value = -value + 1 ;
          cswitchText2[i]->setText( tr("%1.%2").arg( value/10 ).arg( value%10 ) ) ;
        	cswitchText2[i]->resize( ui->cswCol2->minimumWidth(), 20 );
					cswitchText2[i]->raise() ;
				}
				else
				{
					cswitchText2[i]->setVisible(false) ;
				}
				value = g_model.customSw[i].v1+1 ;
        cswitchOffset0[i]->setValue(value);
				if ( value <= 0 )
				{
					cswitchText1[i]->setVisible(true) ;
					value = -value + 1 ;
          cswitchText1[i]->setText( tr("%1.%2").arg( value/10 ).arg( value%10 ) ) ;
        	cswitchText1[i]->resize( ui->cswCol1->minimumWidth(), 20 );
					cswitchText1[i]->raise() ;
				}
				else
				{
					cswitchText1[i]->setVisible(false) ;
				}
        break;
    case CS_TMONO :
        cswitchSource1[i]->setVisible(true);
        populateSwitchCB(cswitchSource1[i],g_model.customSw[i].v1, rData->type);
        cswitchSource2[i]->setVisible(false);
        cswitchOffset[i]->setVisible(true);
        cswitchOffset0[i]->setVisible(false);
        cswitchOffset[i]->setAccelerated(true);
				value = g_model.customSw[i].v2+1 ;
        cswitchOffset[i]->setValue(value);
        if ( cswitchOffset[i]->minimum() != 0 )
				{
        	cswitchOffset[i]->setMaximum(125);
        	cswitchOffset[i]->setMinimum(-125);
				}
				if ( value <= 0 )
				{
					cswitchText2[i]->setVisible(true) ;
					value = -value + 1 ;
          cswitchText2[i]->setText( tr("%1.%2").arg( value/10 ).arg( value%10 ) ) ;
        	cswitchText2[i]->resize( ui->cswCol2->minimumWidth(), 20 );
					cswitchText2[i]->raise() ;
				}
				else
				{
					cswitchText2[i]->setVisible(false) ;
				}
    break ;
    default:
    break;
  }
}

void ModelEdit::updateSwitchesTab()
{
    switchEditLock = true;

    populateCSWCB(ui->cswitchFunc_1, g_model.customSw[0].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_2, g_model.customSw[1].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_3, g_model.customSw[2].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_4, g_model.customSw[3].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_5, g_model.customSw[4].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_6, g_model.customSw[5].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_7, g_model.customSw[6].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_8, g_model.customSw[7].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_9, g_model.customSw[8].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_10,g_model.customSw[9].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_11,g_model.customSw[10].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_12,g_model.customSw[11].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_13,g_model.customSw[12].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_14,g_model.customSw[13].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_15,g_model.customSw[14].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_16,g_model.customSw[15].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_17,g_model.customSw[16].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_18,g_model.customSw[17].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_19,g_model.customSw[18].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_20,g_model.customSw[19].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_21,g_model.customSw[20].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_22,g_model.customSw[21].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_23,g_model.customSw[22].func, g_model.modelVersion);
    populateCSWCB(ui->cswitchFunc_24,g_model.customSw[23].func, g_model.modelVersion);

    for(int i=0; i<NUM_SKYCSW; i++)
        setSwitchWidgetVisibility(i);

    switchEditLock = false;
}

void ModelEdit::tabSwitches()
{
	int width ;

    switchEditLock = true;

    for(int i=0; i<NUM_SKYCSW; i++)
    {
			int j = i ;
			int k = 2 ;
			if ( i > 11 )
			{
				j = i - 12 ;
				k = 8 ;
			}
      if ( !switchesTabDone )
			{
				
        cswitchSource1[i] = new QComboBox(this);
        connect(cswitchSource1[i],SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
        ui->gridLayout_8->addWidget(cswitchSource1[i],j+1,k);
        cswitchSource1[i]->setVisible(false);

        cswitchSource2[i] = new QComboBox(this);
        connect(cswitchSource2[i],SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
        ui->gridLayout_8->addWidget(cswitchSource2[i],j+1,k+1);
        cswitchSource2[i]->setVisible(false);

        cswitchAndSwitch[i] = new QComboBox(this);
        connect(cswitchAndSwitch[i],SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
        ui->gridLayout_8->addWidget(cswitchAndSwitch[i],j+1,k+3);
        cswitchAndSwitch[i]->setVisible(true);
			}
      if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
			{
        x9dPopulateSwitchAndCB(cswitchAndSwitch[i], g_model.customSw[i].andsw) ;//+(MAX_XDRSWITCH-1)) ;
			}
			else
			{
				populateSwitchAndCB(cswitchAndSwitch[i], g_model.customSw[i].andsw) ;
			}
      if ( !switchesTabDone )
			{
				cswitchTlabel[i] = new QLabel(this) ;
        ui->gridLayout_8->addWidget(cswitchTlabel[i],j+1,k+2);
        cswitchTlabel[i]->setVisible(false);
        cswitchTlabel[i]->setText("");

        cswitchOffset[i] = new QSpinBox(this);
        cswitchOffset[i]->setMaximum(125);
        cswitchOffset[i]->setMinimum(-125);
        cswitchOffset[i]->setAccelerated(true);
        connect(cswitchOffset[i],SIGNAL(valueChanged(int)),this,SLOT(switchesEdited()));
        ui->gridLayout_8->addWidget(cswitchOffset[i],j+1,k+1);
        cswitchOffset[i]->setVisible(false);
				width = ui->cswCol2->width() ;
				cswitchOffset[i]->resize( width, 20 );
        
				width -= 14 ;
        cswitchText2[i] = new QTextBrowser(this);
        ui->gridLayout_8->addWidget(cswitchText2[i],j+1,k+1);
        cswitchText2[i]->setVisible(false);
				cswitchText2[i]->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff ) ;
        cswitchText2[i]->setMinimumSize( width, 20 );
        cswitchText2[i]->setMaximumSize( width, 20 );
        cswitchText2[i]->resize( width, 20 );

				cswitchOffset0[i] = new QSpinBox(this);
        cswitchOffset0[i]->setMaximum(100);
        cswitchOffset0[i]->setMinimum(-49);
        cswitchOffset0[i]->setAccelerated(true);
        connect(cswitchOffset0[i],SIGNAL(valueChanged(int)),this,SLOT(switchesEdited()));
        ui->gridLayout_8->addWidget(cswitchOffset0[i],j+1,k);
        cswitchOffset0[i]->setVisible(false);
				width = ui->cswCol2->width() ;
        cswitchOffset0[i]->resize( width, 20 );

				width -= 14 ;
        cswitchText1[i] = new QTextBrowser(this);
        ui->gridLayout_8->addWidget(cswitchText1[i],j+1,k);
        cswitchText1[i]->setVisible(false);
        cswitchText1[i]->setMinimumSize( width, 20 );
        cswitchText1[i]->setMaximumSize( width, 20 );
        cswitchText1[i]->resize( width, 20 );
				cswitchText1[i]->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff ) ;
			}
    }

    updateSwitchesTab();

    //connects
  if ( !switchesTabDone )
	{
    connect(ui->cswitchFunc_1,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_2,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_3,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_4,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_5,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_6,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_7,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_8,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_9,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_10,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_11,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_12,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_13,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_14,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_15,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_16,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_17,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_18,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_19,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_20,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_21,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_22,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_23,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
    connect(ui->cswitchFunc_24,SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
	}

		switchesTabDone = true ;
    switchEditLock = false;
}


void ModelEdit::setSafetyLabels()
{
	ui->SS1->setText(g_model.numVoice < 24 ? "CH1" : "VS1");
	ui->SS2->setText(g_model.numVoice < 23 ? "CH2" : "VS2");
	ui->SS3->setText(g_model.numVoice < 22 ? "CH3" : "VS3");
	ui->SS4->setText(g_model.numVoice < 21 ? "CH4" : "VS4");
	ui->SS5->setText(g_model.numVoice < 20 ? "CH5" : "VS5");
	ui->SS6->setText(g_model.numVoice < 19 ? "CH6" : "VS6");
	ui->SS7->setText(g_model.numVoice < 18 ? "CH7" : "VS7");
	ui->SS8->setText(g_model.numVoice  < 17 ? "CH8" : "VS8");
	ui->SS9->setText(g_model.numVoice  < 16 ? "CH9" : "VS9");
	ui->SS10->setText(g_model.numVoice < 15 ? "CH10" : "VS10");
	ui->SS11->setText(g_model.numVoice < 14 ? "CH11" : "VS11");
	ui->SS12->setText(g_model.numVoice < 13 ? "CH12" : "VS12");
	ui->SS13->setText(g_model.numVoice < 12 ? "CH13" : "VS13");
	ui->SS14->setText(g_model.numVoice < 11 ? "CH14" : "VS14");
	ui->SS15->setText(g_model.numVoice < 10 ? "CH15" : "VS15");
	ui->SS16->setText(g_model.numVoice < 9 ? "CH16" : "VS16");
	ui->SS17->setText(g_model.numVoice < 8 ? "CH17" : "VS17");
	ui->SS18->setText(g_model.numVoice < 7 ? "CH18" : "VS18");
	ui->SS19->setText(g_model.numVoice < 6 ? "CH19" : "VS19");
	ui->SS20->setText(g_model.numVoice < 5 ? "CH20" : "VS20");
	ui->SS21->setText(g_model.numVoice < 4 ? "CH21" : "VS21");
	ui->SS22->setText(g_model.numVoice < 3 ? "CH22" : "VS22");
	ui->SS23->setText(g_model.numVoice < 2 ? "CH23" : "VS23");
	ui->SS24->setText(g_model.numVoice < 1 ? "CH24" : "VS24");
}

void ModelEdit::setSafetyWidgetVisibility(int i)
{
	int limit = MAX_DRSWITCH ;
	if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
	{
		limit = MAX_XDRSWITCH ;
	}
	if ( g_model.numVoice < NUM_SKYCHNOUT-i )
	{
		safetySwitchGvar[i]->setVisible(false) ;
		safetySwitchGindex[i]->setVisible(false) ;
  	switch (g_model.safetySw[i].opt.ss.mode)
		{
			case 0 :		// 'S'
			case 3 :		// 'S'
  	    safetySwitchValue[i]->setVisible(true);
  	    safetySwitchAlarm[i]->setVisible(false);
				safetySwitchValue[i]->setMaximum(125);
  	    safetySwitchValue[i]->setMinimum(-125);
			break ;
			case 2 :		// 'V'
				if ( g_model.safetySw[i].opt.ss.swtch > limit )
				{
		      safetySwitchValue[i]->setVisible(false);
  		    safetySwitchAlarm[i]->setVisible(true);
				}
				else
				{
  	    	safetySwitchValue[i]->setVisible(true);
  	    	safetySwitchAlarm[i]->setVisible(false);
				}	 
			break ;
		
			case 1 :		// 'A'
  	    safetySwitchValue[i]->setVisible(false);
  	    safetySwitchAlarm[i]->setVisible(true);
			break ;
		}
	}
	else
	{
		safetySwitchValue[i]->setMaximum(250);
    safetySwitchValue[i]->setMinimum(0);
		SKYSafetySwData *sd = &g_model.safetySw[i];
		if ( i >= NUM_SKYCHNOUT )
		{
			sd = (SKYSafetySwData*)&g_model.voiceSwitches[i-NUM_SKYCHNOUT];
		}

		if ( sd->opt.vs.vmode > 5 )
		{
			safetySwitchValue[i]->setVisible(false);
  		safetySwitchAlarm[i]->setVisible(true);
			safetySwitchGvar[i]->setVisible(false) ;
			safetySwitchGindex[i]->setVisible(false) ;
		} 
		else
		{
  	  safetySwitchAlarm[i]->setVisible(false);
			safetySwitchGvar[i]->setVisible(true) ;
			if ( safetySwitchGvar[i]->isChecked() )
			{
				safetySwitchValue[i]->setVisible(false) ;
				safetySwitchGindex[i]->setVisible(true) ;
			}
			else
			{
				safetySwitchGindex[i]->setVisible(false) ;
				safetySwitchValue[i]->setVisible(true) ;
			}
		}
	}
}



void ModelEdit::tabSafetySwitches()
{
//		g_model.numVoice
		ui->NumVoiceSwSB->setValue(g_model.numVoice+8);
		setSafetyLabels() ;

    for(int i=0; i<NUM_SKYCHNOUT+NUM_VOICE; i++)
    {
			int j = i ;
			int k = 1 ;
			if ( i > 15 )
			{
				j = i - 16 ;
				k = 7 ;
			}
        safetySwitchType[i] = new QComboBox(this);
        safetySwitchSwtch[i] = new QComboBox(this);
				safetySwitchAlarm[i] = new QComboBox(this);
        safetySwitchValue[i] = new QSpinBox(this);
				safetySwitchGvar[i] = new QCheckBox(this) ;
				safetySwitchGindex[i] = new QComboBox(this) ;
				
  			safetySwitchGindex[i]->clear() ;
  			for (int j=3; j<=7; j++)
				{
  			  safetySwitchGindex[i]->addItem(QObject::tr("GV%1").arg(j));
  			}

        safetySwitchValue[i]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        safetySwitchValue[i]->setAccelerated(true);
				
				safetySwitchGvar[i]->setText( "Gvar" ) ;
				 
				if ( g_model.numVoice < NUM_SKYCHNOUT-i )	// Normal switch
				{
    			SKYSafetySwData *sd = &g_model.safetySw[i];
        	populateSafetyVoiceTypeCB(safetySwitchType[i], 0, sd->opt.ss.mode);
        	populateSafetySwitchCB(safetySwitchSwtch[i],sd->opt.ss.mode,sd->opt.ss.swtch, rData->type);
					populateAlarmCB(safetySwitchAlarm[i],sd->opt.ss.val);
					if ( sd->opt.ss.mode == 2 )		// 'V'
					{
						int limit = MAX_DRSWITCH ;
						if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
						{
							limit = MAX_XDRSWITCH ;
						}
						if ( sd->opt.ss.swtch > limit )
						{
							populateTelItemsCB( safetySwitchAlarm[i], 1,sd->opt.ss.val ) ;
						}
						safetySwitchValue[i]->setMaximum(239);
       			safetySwitchValue[i]->setMinimum(0);
       			safetySwitchValue[i]->setValue(sd->opt.ss.val+128);
					}
					else
					{
						safetySwitchValue[i]->setMaximum(125);
        		safetySwitchValue[i]->setMinimum(-125);
        		safetySwitchValue[i]->setValue(sd->opt.ss.val);
					}
				}	 
				else // voice switch
				{
    			SKYSafetySwData *sd = &g_model.safetySw[i];
					if ( i >= NUM_SKYCHNOUT )
					{
						sd = (SKYSafetySwData*)&g_model.voiceSwitches[i-NUM_SKYCHNOUT];
					}
        	populateSafetyVoiceTypeCB(safetySwitchType[i], 1, sd->opt.vs.vmode);
          populateSafetySwitchCB(safetySwitchSwtch[i],VOICE_SWITCH,sd->opt.vs.vswtch, rData->type);
					safetySwitchValue[i]->setMaximum(250);
     			safetySwitchValue[i]->setMinimum(0);
       		safetySwitchValue[i]->setValue(sd->opt.vs.vval);
					populateTelItemsCB( safetySwitchAlarm[i], 1,sd->opt.vs.vval ) ;
					if ( g_model.safetySw[i].opt.vs.vval > 250 )
					{
            safetySwitchGindex[i]->setCurrentIndex( sd->opt.vs.vval - 251 ) ;
						safetySwitchGvar[i]->setChecked(true) ;
					}
					else
					{
						safetySwitchGvar[i]->setChecked(false) ;
					}
				}
        ui->grid_tabSafetySwitches->addWidget(safetySwitchType[i],j+2,k);
        ui->grid_tabSafetySwitches->addWidget(safetySwitchSwtch[i],j+2,k+1);

        ui->grid_tabSafetySwitches->addWidget(safetySwitchAlarm[i],j+2,k+2);
        ui->grid_tabSafetySwitches->addWidget(safetySwitchValue[i],j+2,k+2);
        ui->grid_tabSafetySwitches->addWidget(safetySwitchGindex[i],j+2,k+2);
        ui->grid_tabSafetySwitches->addWidget(safetySwitchGvar[i],j+2,k+3);
        
				setSafetyWidgetVisibility(i);
        connect(safetySwitchType[i],SIGNAL(currentIndexChanged(int)),this,SLOT(safetySwitchesEdited()));
        connect(safetySwitchSwtch[i],SIGNAL(currentIndexChanged(int)),this,SLOT(safetySwitchesEdited()));
        connect(safetySwitchAlarm[i],SIGNAL(currentIndexChanged(int)),this,SLOT(safetySwitchesEdited()));
        connect(safetySwitchValue[i],SIGNAL(editingFinished()),this,SLOT(safetySwitchesEdited()));
        connect(safetySwitchGindex[i],SIGNAL(currentIndexChanged(int)),this,SLOT(safetySwitchesEdited()));
			  connect( safetySwitchGvar[i],SIGNAL(stateChanged(int)),this,SLOT(safetySwitchesEdited()));
    }
    connect(ui->NumVoiceSwSB,SIGNAL(valueChanged(int)),this,SLOT(safetySwitchesEdited()));
}

static int EditedNesting = 0 ;

void ModelEdit::safetySwitchesEdited()
{
		int val ;
		int modechange[NUM_SKYCHNOUT] ;
		int numVoice ;
//		int voiceindexchange[NUM_CHNOUT] ;

		if ( EditedNesting )
		{
			return ;
		}
		EditedNesting = 1 ;
		
		numVoice = g_model.numVoice ;
    g_model.numVoice = ui->NumVoiceSwSB->value()-8 ;

		int limit = MAX_DRSWITCH ;
		if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
		{
			limit = MAX_XDRSWITCH ;
		}
    for(int i=0; i<NUM_SKYCHNOUT+NUM_VOICE; i++)
    {
    	SKYSafetySwData *sd = &g_model.safetySw[i];
			if ( i >= NUM_SKYCHNOUT )
			{
				sd = (SKYSafetySwData*)&g_model.voiceSwitches[i-NUM_SKYCHNOUT];
			}
      val = safetySwitchValue[i]->value();
			if ( numVoice < NUM_SKYCHNOUT-i )	// Normal switch
			{
				modechange[i] = sd->opt.ss.mode ;	// Previous value
				if ( sd->opt.ss.mode == 2)	// Voice
				{
					val -= 128 ;
					if ( sd->opt.ss.swtch > limit )
					{
						val = safetySwitchAlarm[i]->currentIndex() ;
					}
				}
				if ( sd->opt.ss.mode == 1)	// Alarm
				{
					val = safetySwitchAlarm[i]->currentIndex() ;
				}
        sd->opt.ss.val = val ;

        sd->opt.ss.mode  = safetySwitchType[i]->currentIndex() ;
        sd->opt.ss.swtch = getSwitchCbValue( safetySwitchSwtch[i], rData->type ) ;
			}
			else // voice switch
			{
				sd->opt.vs.vmode = safetySwitchType[i]->currentIndex() ;
				if ( sd->opt.vs.vmode > 5 )
				{
					val = safetySwitchAlarm[i]->currentIndex() ;
        }	
				else
				{
					if ( safetySwitchGvar[i]->isChecked() )
					{
						val = 251 + safetySwitchGindex[i]->currentIndex() ;						
					}
				}
        sd->opt.vs.vval = val ;
        sd->opt.vs.vswtch = getSwitchCbValueShort( safetySwitchSwtch[i], rData->type ) ;
			}	
		}
    updateSettings();
		
		if ( g_model.numVoice != numVoice )
		{
			setSafetyLabels() ;
		}
    
		for(int i=0; i<NUM_SKYCHNOUT+NUM_VOICE; i++)
		{
			if ( g_model.numVoice < NUM_SKYCHNOUT-i )		// Normal switch
			{
		    if ( i > NUM_SKYCHNOUT-numVoice-1 && i < NUM_SKYCHNOUT-g_model.numVoice )
				{
					g_model.safetySw[i].opt.ss.swtch = 0 ;
          populateSafetySwitchCB(safetySwitchSwtch[i],g_model.safetySw[i].opt.ss.mode,g_model.safetySw[i].opt.ss.swtch, rData->type);
				}
        populateSafetyVoiceTypeCB(safetySwitchType[i], 0, g_model.safetySw[i].opt.ss.mode);
				if ( modechange[i] != g_model.safetySw[i].opt.ss.mode )
				{
          populateSafetySwitchCB(safetySwitchSwtch[i],g_model.safetySw[i].opt.ss.mode,g_model.safetySw[i].opt.ss.swtch, rData->type);
					if ( g_model.safetySw[i].opt.ss.mode != 2 )
					{
						if ( g_model.safetySw[i].opt.ss.swtch > limit )
						{
							g_model.safetySw[i].opt.ss.swtch = limit ;
    					safetySwitchSwtch[i]->setCurrentIndex(limit+limit) ;
						}					
					}
					populateAlarmCB(safetySwitchAlarm[i],g_model.safetySw[i].opt.ss.val);
					if ( g_model.safetySw[i].opt.ss.mode == 2 )
					{
						safetySwitchValue[i]->setMaximum(239);
  	  		  safetySwitchValue[i]->setMinimum(0);
      		 	safetySwitchValue[i]->setValue(g_model.safetySw[i].opt.ss.val+128);
					}
					else
					{
      		  safetySwitchValue[i]->setValue(g_model.safetySw[i].opt.ss.val);
					}
				}
				if ( g_model.safetySw[i].opt.ss.swtch > limit )
				{
					populateTelItemsCB( safetySwitchAlarm[i], 1,g_model.safetySw[i].opt.ss.val ) ;
				}
			}
			else
			{
	    	SKYSafetySwData *sd = &g_model.safetySw[i];
				if ( i >= NUM_SKYCHNOUT )
				{
					sd = (SKYSafetySwData*)&g_model.voiceSwitches[i-NUM_SKYCHNOUT];
				}
		    if ( i >= NUM_SKYCHNOUT-g_model.numVoice-1 && i < NUM_SKYCHNOUT-numVoice )
				{
					sd->opt.vs.vswtch = 0 ;
          sd->opt.vs.vval = 0 ;
					sd->opt.vs.vmode = 0 ;
				}
        populateSafetySwitchCB(safetySwitchSwtch[i],VOICE_SWITCH,sd->opt.vs.vswtch, rData->type);
        populateSafetyVoiceTypeCB(safetySwitchType[i], 1, sd->opt.vs.vmode);
				safetySwitchValue[i]->setMaximum(250);
     		safetySwitchValue[i]->setMinimum(0);
       	safetySwitchValue[i]->setValue(sd->opt.vs.vval);
				if ( sd->opt.vs.vmode > 5 )
				{
					populateTelItemsCB( safetySwitchAlarm[i], 1,sd->opt.ss.val ) ;
				}
			}
      setSafetyWidgetVisibility(i);
		}
		EditedNesting = 0 ;

}


void ModelEdit::switchesEdited()
{
	char telText[20] ;
	int16_t value ;
    if(switchEditLock) return;
    switchEditLock = true;

    bool chAr[NUM_SKYCSW];
//		int limit = MAX_DRSWITCH ;
//		if ( rData->type )
//		{
//			limit = MAX_XDRSWITCH ;
//		}

    chAr[0]  = (CS_STATE(g_model.customSw[0].func, g_model.modelVersion)) !=(CS_STATE(ui->cswitchFunc_1->currentIndex(), g_model.modelVersion));
    chAr[1]  = (CS_STATE(g_model.customSw[1].func, g_model.modelVersion)) !=(CS_STATE(ui->cswitchFunc_2->currentIndex(), g_model.modelVersion));
    chAr[2]  = (CS_STATE(g_model.customSw[2].func, g_model.modelVersion)) !=(CS_STATE(ui->cswitchFunc_3->currentIndex(), g_model.modelVersion));
    chAr[3]  = (CS_STATE(g_model.customSw[3].func, g_model.modelVersion)) !=(CS_STATE(ui->cswitchFunc_4->currentIndex(), g_model.modelVersion));
    chAr[4]  = (CS_STATE(g_model.customSw[4].func, g_model.modelVersion)) !=(CS_STATE(ui->cswitchFunc_5->currentIndex(), g_model.modelVersion));
    chAr[5]  = (CS_STATE(g_model.customSw[5].func, g_model.modelVersion)) !=(CS_STATE(ui->cswitchFunc_6->currentIndex(), g_model.modelVersion));
    chAr[6]  = (CS_STATE(g_model.customSw[6].func, g_model.modelVersion)) !=(CS_STATE(ui->cswitchFunc_7->currentIndex(), g_model.modelVersion));
    chAr[7]  = (CS_STATE(g_model.customSw[7].func, g_model.modelVersion)) !=(CS_STATE(ui->cswitchFunc_8->currentIndex(), g_model.modelVersion));
    chAr[8]  = (CS_STATE(g_model.customSw[8].func, g_model.modelVersion)) !=(CS_STATE(ui->cswitchFunc_9->currentIndex(), g_model.modelVersion));
    chAr[9]  = (CS_STATE(g_model.customSw[9].func, g_model.modelVersion)) !=(CS_STATE(ui->cswitchFunc_10->currentIndex(), g_model.modelVersion));
    chAr[10] = (CS_STATE(g_model.customSw[10].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_11->currentIndex(), g_model.modelVersion));
    chAr[11] = (CS_STATE(g_model.customSw[11].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_12->currentIndex(), g_model.modelVersion));
    chAr[12] = (CS_STATE(g_model.customSw[12].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_13->currentIndex(), g_model.modelVersion));
    chAr[13] = (CS_STATE(g_model.customSw[13].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_14->currentIndex(), g_model.modelVersion));
    chAr[14] = (CS_STATE(g_model.customSw[14].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_15->currentIndex(), g_model.modelVersion));
    chAr[15] = (CS_STATE(g_model.customSw[15].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_16->currentIndex(), g_model.modelVersion));
    chAr[16] = (CS_STATE(g_model.customSw[16].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_17->currentIndex(), g_model.modelVersion));
    chAr[17] = (CS_STATE(g_model.customSw[17].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_18->currentIndex(), g_model.modelVersion));
    chAr[18] = (CS_STATE(g_model.customSw[18].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_19->currentIndex(), g_model.modelVersion));
    chAr[19] = (CS_STATE(g_model.customSw[19].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_20->currentIndex(), g_model.modelVersion));
    chAr[20] = (CS_STATE(g_model.customSw[20].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_21->currentIndex(), g_model.modelVersion));
    chAr[21] = (CS_STATE(g_model.customSw[21].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_22->currentIndex(), g_model.modelVersion));
    chAr[22] = (CS_STATE(g_model.customSw[22].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_23->currentIndex(), g_model.modelVersion));
    chAr[23] = (CS_STATE(g_model.customSw[23].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_24->currentIndex(), g_model.modelVersion));

    g_model.customSw[0].func  = ui->cswitchFunc_1->currentIndex();
    g_model.customSw[1].func  = ui->cswitchFunc_2->currentIndex();
    g_model.customSw[2].func  = ui->cswitchFunc_3->currentIndex();
    g_model.customSw[3].func  = ui->cswitchFunc_4->currentIndex();
    g_model.customSw[4].func  = ui->cswitchFunc_5->currentIndex();
    g_model.customSw[5].func  = ui->cswitchFunc_6->currentIndex();
    g_model.customSw[6].func  = ui->cswitchFunc_7->currentIndex();
    g_model.customSw[7].func  = ui->cswitchFunc_8->currentIndex();
    g_model.customSw[8].func  = ui->cswitchFunc_9->currentIndex();
    g_model.customSw[9].func  = ui->cswitchFunc_10->currentIndex();
    g_model.customSw[10].func = ui->cswitchFunc_11->currentIndex();
    g_model.customSw[11].func = ui->cswitchFunc_12->currentIndex();
    g_model.customSw[12].func = ui->cswitchFunc_13->currentIndex();
    g_model.customSw[13].func = ui->cswitchFunc_14->currentIndex();
    g_model.customSw[14].func = ui->cswitchFunc_15->currentIndex();
    g_model.customSw[15].func = ui->cswitchFunc_16->currentIndex();
    g_model.customSw[16].func = ui->cswitchFunc_17->currentIndex();
    g_model.customSw[17].func = ui->cswitchFunc_18->currentIndex();
    g_model.customSw[18].func = ui->cswitchFunc_19->currentIndex();
    g_model.customSw[19].func = ui->cswitchFunc_20->currentIndex();
    g_model.customSw[20].func = ui->cswitchFunc_21->currentIndex();
    g_model.customSw[21].func = ui->cswitchFunc_22->currentIndex();
    g_model.customSw[22].func = ui->cswitchFunc_23->currentIndex();
    g_model.customSw[23].func = ui->cswitchFunc_24->currentIndex();

    
		for(int i=0; i<NUM_SKYCSW; i++)
    {
      if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
			{
//        g_model.customSw[i].andsw = cswitchAndSwitch[i]->currentIndex()-(MAX_XDRSWITCH-1);
        g_model.customSw[i].andsw = getSwitchCbValueShort( cswitchAndSwitch[i], 1 ) ;
			}
			else
			{
        g_model.customSw[i].andsw = getAndSwitchCbValue( cswitchAndSwitch[i] ) ;
										//				getSwitchCbValueShort( cswitchAndSwitch[i], 0 ) ;
      }  
				if(chAr[i])
        {
            g_model.customSw[i].v1 = 0;
            g_model.customSw[i].v2 = 0;
            setSwitchWidgetVisibility(i);
        }

        switch(CS_STATE(g_model.customSw[i].func, g_model.modelVersion))
        {
        case (CS_VOFS):
            g_model.customSw[i].v1 = decodePots( cswitchSource1[i]->currentIndex(), rData->type ) ;
            g_model.customSw[i].v2 = cswitchOffset[i]->value();
						if ( g_model.customSw[i].v1 > 36 )
						{
							value = convertTelemConstant( g_model.customSw[i].v1 - 37, g_model.customSw[i].v2, &g_model ) ;
              stringTelemetryChannel( telText, g_model.customSw[i].v1 - 37, value, &g_model ) ;
							//sprintf( telText, "%d", value ) ;
        			cswitchTlabel[i]->setText(telText);
						}
            break;
        case (CS_VBOOL):
            g_model.customSw[i].v1 =  getSwitchCbValue( cswitchSource1[i] , rData->type ) ;
            g_model.customSw[i].v2 =  getSwitchCbValue( cswitchSource2[i] , rData->type ) ;
            break;
        case (CS_VCOMP):
            g_model.customSw[i].v1 = decodePots( cswitchSource1[i]->currentIndex(), rData->type ) ;
            g_model.customSw[i].v2 = decodePots( cswitchSource2[i]->currentIndex(), rData->type ) ;
            break;
        case (CS_TIMER):
            g_model.customSw[i].v2 = cswitchOffset[i]->value()-1;
            g_model.customSw[i].v1 = cswitchOffset0[i]->value()-1;
            break;
        case (CS_TMONO):
            g_model.customSw[i].v2 = cswitchOffset[i]->value()-1;
            g_model.customSw[i].v1 =  getSwitchCbValue( cswitchSource1[i] , rData->type ) ;
        break ;
        default:
            break;
        }
    }

    for(int i=0; i<NUM_SKYCSW; i++)
        setSwitchWidgetVisibility(i);

    updateSettings();

    switchEditLock = false;
}

void ModelEdit::tabTrims()
{
		ui->spinBox_S1->setValue(g_model.trim[(g_eeGeneral.stickMode>1)   ? 3 : 0]);//CONVERT_MODE(RUD)-1]);
    ui->spinBox_S2->setValue(g_model.trim[(g_eeGeneral.stickMode & 1) ? 2 : 1]);//CONVERT_MODE(ELE)-1]);
    ui->spinBox_S3->setValue(g_model.trim[(g_eeGeneral.stickMode & 1) ? 1 : 2]);//CONVERT_MODE(THR)-1]);
    ui->spinBox_S4->setValue(g_model.trim[(g_eeGeneral.stickMode>1)   ? 0 : 3]);//CONVERT_MODE(AIL)-1]);

		int i = g_eeGeneral.stickMode ;
		if ( g_eeGeneral.crosstrim )
		{
			i = 3 - i ;
		}

    switch (i)
    {
        case (0):
            ui->Label_S1->setText("RUD");
            ui->Label_S2->setText("ELE");
            ui->Label_S3->setText("THR");
            ui->Label_S4->setText("AIL");
            if(throttleReversed( &g_eeGeneral, &g_model ) )
                ui->slider_S3->setInvertedAppearance(true);
            break;
        case (1):
            ui->Label_S1->setText("RUD");
            ui->Label_S2->setText("THR");
            ui->Label_S3->setText("ELE");
            ui->Label_S4->setText("AIL");
            if(throttleReversed( &g_eeGeneral, &g_model ) )
                ui->slider_S2->setInvertedAppearance(true);
            break;
        case (2):
            ui->Label_S1->setText("AIL");
            ui->Label_S2->setText("ELE");
            ui->Label_S3->setText("THR");
            ui->Label_S4->setText("RUD");
            if(throttleReversed( &g_eeGeneral, &g_model ) )
                ui->slider_S3->setInvertedAppearance(true);
            break;
        case (3):
            ui->Label_S1->setText("AIL");
            ui->Label_S2->setText("THR");
            ui->Label_S3->setText("ELE");
            ui->Label_S4->setText("RUD");
            if(throttleReversed( &g_eeGeneral, &g_model ) )
                ui->slider_S2->setInvertedAppearance(true);
            break;
    }

}


void ModelEdit::oneGvarVisibility(int index, QComboBox *b, QSpinBox *sb )
{
	int function = g_model.gvarAdjuster[index].function ;
	int value = g_model.gvarAdjuster[index].switch_value ;
	int oldFunction = oldAdjFunction[index] ;

	if ( function == 3 )
	{
		if ( oldFunction != 3 )
		{
			value = 0 ;
		}
	}
	if ( function > 3 )
	{
		if ( oldFunction <= 3 )
		{
			value = 0 ;
		}
	}
	g_model.gvarAdjuster[index].switch_value = value ;

	if ( ( function < 3 ) || ( function > 6 ) )
	{
		sb->show() ;
		b->hide() ;
	}
	else
	{
		if ( function == 3 )	// Set V
		{
			populateGvarCB( b, value, rData->type  ) ;
		}
		else
		{
			populateSwitchCB( b, value, rData->type);
		}
		sb->hide() ;
		b->show() ;
	}
	
}


void ModelEdit::gvarVisibility()
{
	oneGvarVisibility( 0, ui->Adj1Sw2CB, ui->Adj1ValueSB ) ;
	oneGvarVisibility( 1, ui->Adj2Sw2CB, ui->Adj2ValueSB ) ;
	oneGvarVisibility( 2, ui->Adj3Sw2CB, ui->Adj3ValueSB ) ;
	oneGvarVisibility( 3, ui->Adj4Sw2CB, ui->Adj4ValueSB ) ;
	oneGvarVisibility( 4, ui->Adj5Sw2CB, ui->Adj5ValueSB ) ;
	oneGvarVisibility( 5, ui->Adj6Sw2CB, ui->Adj6ValueSB ) ;
	oneGvarVisibility( 6, ui->Adj7Sw2CB, ui->Adj7ValueSB ) ;
	oneGvarVisibility( 7, ui->Adj8Sw2CB, ui->Adj8ValueSB ) ;
}


void ModelEdit::tabGvar()
{
		posb[0] = ui->Sc1OffsetSB ;
		posb[1] = ui->Sc2OffsetSB ;
		posb[2] = ui->Sc3OffsetSB ;
		posb[3] = ui->Sc4OffsetSB ;
		posb[4] = ui->Sc5OffsetSB ;
		posb[5] = ui->Sc6OffsetSB ;
		posb[6] = ui->Sc7OffsetSB ;
		posb[7] = ui->Sc8OffsetSB ;

		pmsb[0] = ui->Sc1MultSB ;
		pmsb[1] = ui->Sc2MultSB ;
		pmsb[2] = ui->Sc3MultSB ;
		pmsb[3] = ui->Sc4MultSB ;
		pmsb[4] = ui->Sc5MultSB ;
		pmsb[5] = ui->Sc6MultSB ;
		pmsb[6] = ui->Sc7MultSB ;
		pmsb[7] = ui->Sc8MultSB ;

		pdivsb[0] = ui->Sc1DivSB ;
		pdivsb[1] = ui->Sc2DivSB ;
		pdivsb[2] = ui->Sc3DivSB ;
		pdivsb[3] = ui->Sc4DivSB ;
		pdivsb[4] = ui->Sc5DivSB ;
		pdivsb[5] = ui->Sc6DivSB ;
		pdivsb[6] = ui->Sc7DivSB ;
		pdivsb[7] = ui->Sc8DivSB ;
		
		pdpsb[0] = ui->Sc1DecimalsSB ;
		pdpsb[1] = ui->Sc2DecimalsSB ;
		pdpsb[2] = ui->Sc3DecimalsSB ;
		pdpsb[3] = ui->Sc4DecimalsSB ;
		pdpsb[4] = ui->Sc5DecimalsSB ;
		pdpsb[5] = ui->Sc6DecimalsSB ;
		pdpsb[6] = ui->Sc7DecimalsSB ;
		pdpsb[7] = ui->Sc8DecimalsSB ;

		pucb[0] = ui->Sc1UnitsCB ;
		pucb[1] = ui->Sc2UnitsCB ;
		pucb[2] = ui->Sc3UnitsCB ;
		pucb[3] = ui->Sc4UnitsCB ;
		pucb[4] = ui->Sc5UnitsCB ;
		pucb[5] = ui->Sc6UnitsCB ;
		pucb[6] = ui->Sc7UnitsCB ;
		pucb[7] = ui->Sc8UnitsCB ;

		psgncb[0] = ui->Sc1SignCB ;
		psgncb[1] = ui->Sc2SignCB ;
		psgncb[2] = ui->Sc3SignCB ;
		psgncb[3] = ui->Sc4SignCB ;
		psgncb[4] = ui->Sc5SignCB ;
		psgncb[5] = ui->Sc6SignCB ;
		psgncb[6] = ui->Sc7SignCB ;
		psgncb[7] = ui->Sc8SignCB ;

		poffcb[0] = ui->Sc1OffAtCB ;
		poffcb[1] = ui->Sc2OffAtCB ;
		poffcb[2] = ui->Sc3OffAtCB ;
		poffcb[3] = ui->Sc4OffAtCB ;
		poffcb[4] = ui->Sc5OffAtCB ;
		poffcb[5] = ui->Sc6OffAtCB ;
		poffcb[6] = ui->Sc7OffAtCB ;
		poffcb[7] = ui->Sc8OffAtCB ;
		
		psrccb[0] = ui->Sc1SrcCB ;
		psrccb[1] = ui->Sc2SrcCB ;
		psrccb[2] = ui->Sc3SrcCB ;
		psrccb[3] = ui->Sc4SrcCB ;
		psrccb[4] = ui->Sc5SrcCB ;
		psrccb[5] = ui->Sc6SrcCB ;
		psrccb[6] = ui->Sc7SrcCB ;
		psrccb[7] = ui->Sc8SrcCB ;

		psname[0] = ui->SC1Name ;
		psname[1] = ui->SC2Name ;
		psname[2] = ui->SC3Name ;
		psname[3] = ui->SC4Name ;
		psname[4] = ui->SC5Name ;
		psname[5] = ui->SC6Name ;
		psname[6] = ui->SC7Name ;
		psname[7] = ui->SC8Name ;
		 
		int i ;
		for ( i = 0 ; i < NUM_SCALERS ; i += 1 )
		{
			posb[i]->setValue(g_model.Scalers[i].offset ) ;
			pmsb[i]->setValue(g_model.Scalers[i].mult+1 ) ;
			pdivsb[i]->setValue(g_model.Scalers[i].div+1 ) ;
			pdpsb[i]->setValue(g_model.Scalers[i].precision ) ;
      pucb[i]->setCurrentIndex(g_model.Scalers[i].unit ) ;
      psgncb[i]->setCurrentIndex(g_model.Scalers[i].neg ) ;
			poffcb[i]->setCurrentIndex(g_model.Scalers[i].offsetLast ) ;
      populateSourceCB(psrccb[i],g_eeGeneral.stickMode,1,g_model.Scalers[i].source,g_model.modelVersion, rData->type ) ;
      QString n = (char *)g_model.Scalers[i].name ;
			while ( n.endsWith(" ") )
			{
				n = n.left(n.size()-1) ;			
			}
  		psname[i]->setText( n ) ;

    	connect(posb[i],SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    	connect(pmsb[i],SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    	connect(pdivsb[i],SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    	connect(pdpsb[i],SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    	connect(pucb[i],SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    	connect(psgncb[i],SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    	connect(poffcb[i],SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    	connect(psrccb[i],SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
			connect(psname[i], SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
		}
		 
    populateGvarCB( ui->Gvar1CB, g_model.gvars[0].gvsource, rData->type  ) ;
    populateGvarCB( ui->Gvar2CB, g_model.gvars[1].gvsource, rData->type  ) ;
    populateGvarCB( ui->Gvar3CB, g_model.gvars[2].gvsource, rData->type  ) ;
    populateGvarCB( ui->Gvar4CB, g_model.gvars[3].gvsource, rData->type  ) ;
    populateGvarCB( ui->Gvar5CB, g_model.gvars[4].gvsource, rData->type  ) ;
    populateGvarCB( ui->Gvar6CB, g_model.gvars[5].gvsource, rData->type  ) ;
    populateGvarCB( ui->Gvar7CB, g_model.gvars[6].gvsource, rData->type  ) ;
    ui->Gv1SB->setValue(g_model.gvars[0].gvar);
    ui->Gv2SB->setValue(g_model.gvars[1].gvar);
    ui->Gv3SB->setValue(g_model.gvars[2].gvar);
    ui->Gv4SB->setValue(g_model.gvars[3].gvar);
    ui->Gv5SB->setValue(g_model.gvars[4].gvar);
    ui->Gv6SB->setValue(g_model.gvars[5].gvar);
    ui->Gv7SB->setValue(g_model.gvars[6].gvar);

    populateSwitchCB(ui->GvSw1CB,g_model.gvswitch[0], rData->type);
    populateSwitchCB(ui->GvSw2CB,g_model.gvswitch[1], rData->type);
    populateSwitchCB(ui->GvSw3CB,g_model.gvswitch[2], rData->type);
    populateSwitchCB(ui->GvSw4CB,g_model.gvswitch[3], rData->type);
    populateSwitchCB(ui->GvSw5CB,g_model.gvswitch[4], rData->type);
    populateSwitchCB(ui->GvSw6CB,g_model.gvswitch[5], rData->type);
    populateSwitchCB(ui->GvSw7CB,g_model.gvswitch[6], rData->type);

    connect(ui->Gvar1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Gvar2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Gvar3CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Gvar4CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Gvar5CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Gvar6CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Gvar7CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));

    connect(ui->Gv1SB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Gv2SB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Gv3SB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Gv4SB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Gv5SB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Gv6SB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Gv7SB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));

    connect(ui->GvSw1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->GvSw2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->GvSw3CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->GvSw4CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->GvSw5CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->GvSw6CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->GvSw7CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));

		// Adjusters
    populateSwitchCB( ui->Adj1Sw1CB, g_model.gvarAdjuster[0].swtch, rData->type);
    populateSwitchCB( ui->Adj2Sw1CB, g_model.gvarAdjuster[1].swtch, rData->type);
    populateSwitchCB( ui->Adj3Sw1CB, g_model.gvarAdjuster[2].swtch, rData->type);
    populateSwitchCB( ui->Adj4Sw1CB, g_model.gvarAdjuster[3].swtch, rData->type);
    populateSwitchCB( ui->Adj5Sw1CB, g_model.gvarAdjuster[4].swtch, rData->type);
    populateSwitchCB( ui->Adj6Sw1CB, g_model.gvarAdjuster[5].swtch, rData->type);
    populateSwitchCB( ui->Adj7Sw1CB, g_model.gvarAdjuster[6].swtch, rData->type);
    populateSwitchCB( ui->Adj8Sw1CB, g_model.gvarAdjuster[7].swtch, rData->type);

    connect(ui->Adj1Sw1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj2Sw1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj3Sw1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj4Sw1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj5Sw1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj6Sw1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj7Sw1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj8Sw1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));

		populateSwitchCB( ui->Adj1Sw2CB, g_model.gvarAdjuster[0].switch_value, rData->type);
    populateSwitchCB( ui->Adj2Sw2CB, g_model.gvarAdjuster[1].switch_value, rData->type);
    populateSwitchCB( ui->Adj3Sw2CB, g_model.gvarAdjuster[2].switch_value, rData->type);
    populateSwitchCB( ui->Adj4Sw2CB, g_model.gvarAdjuster[3].switch_value, rData->type);
    populateSwitchCB( ui->Adj5Sw2CB, g_model.gvarAdjuster[4].switch_value, rData->type);
    populateSwitchCB( ui->Adj6Sw2CB, g_model.gvarAdjuster[5].switch_value, rData->type);
    populateSwitchCB( ui->Adj7Sw2CB, g_model.gvarAdjuster[6].switch_value, rData->type);
    populateSwitchCB( ui->Adj8Sw2CB, g_model.gvarAdjuster[7].switch_value, rData->type);

    connect(ui->Adj1Sw2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj2Sw2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj3Sw2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj4Sw2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj5Sw2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj6Sw2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj7Sw2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj8Sw2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));

    ui->Adj1ValueSB->setValue(g_model.gvarAdjuster[0].switch_value);
    ui->Adj2ValueSB->setValue(g_model.gvarAdjuster[1].switch_value);
    ui->Adj3ValueSB->setValue(g_model.gvarAdjuster[2].switch_value);
    ui->Adj4ValueSB->setValue(g_model.gvarAdjuster[3].switch_value);
    ui->Adj5ValueSB->setValue(g_model.gvarAdjuster[4].switch_value);
    ui->Adj6ValueSB->setValue(g_model.gvarAdjuster[5].switch_value);
    ui->Adj7ValueSB->setValue(g_model.gvarAdjuster[6].switch_value);
    ui->Adj8ValueSB->setValue(g_model.gvarAdjuster[7].switch_value);

    connect(ui->Adj1ValueSB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Adj2ValueSB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Adj3ValueSB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Adj4ValueSB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Adj5ValueSB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Adj6ValueSB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Adj7ValueSB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));
    connect(ui->Adj8ValueSB,SIGNAL(editingFinished()),this,SLOT(GvarEdited()));

    ui->Adj1FunctionCB->setCurrentIndex(g_model.gvarAdjuster[0].function ) ;
    ui->Adj2FunctionCB->setCurrentIndex(g_model.gvarAdjuster[1].function ) ;
    ui->Adj3FunctionCB->setCurrentIndex(g_model.gvarAdjuster[2].function ) ;
    ui->Adj4FunctionCB->setCurrentIndex(g_model.gvarAdjuster[3].function ) ;
    ui->Adj5FunctionCB->setCurrentIndex(g_model.gvarAdjuster[4].function ) ;
    ui->Adj6FunctionCB->setCurrentIndex(g_model.gvarAdjuster[5].function ) ;
    ui->Adj7FunctionCB->setCurrentIndex(g_model.gvarAdjuster[6].function ) ;
    ui->Adj8FunctionCB->setCurrentIndex(g_model.gvarAdjuster[7].function ) ;
		
    connect(ui->Adj1FunctionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj2FunctionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj3FunctionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj4FunctionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj5FunctionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj6FunctionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj7FunctionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj8FunctionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));

    ui->Adj1GvarCB->setCurrentIndex(g_model.gvarAdjuster[0].gvarIndex ) ;
    ui->Adj2GvarCB->setCurrentIndex(g_model.gvarAdjuster[1].gvarIndex ) ;
    ui->Adj3GvarCB->setCurrentIndex(g_model.gvarAdjuster[2].gvarIndex ) ;
    ui->Adj4GvarCB->setCurrentIndex(g_model.gvarAdjuster[3].gvarIndex ) ;
    ui->Adj5GvarCB->setCurrentIndex(g_model.gvarAdjuster[4].gvarIndex ) ;
    ui->Adj6GvarCB->setCurrentIndex(g_model.gvarAdjuster[5].gvarIndex ) ;
    ui->Adj7GvarCB->setCurrentIndex(g_model.gvarAdjuster[6].gvarIndex ) ;
    ui->Adj8GvarCB->setCurrentIndex(g_model.gvarAdjuster[7].gvarIndex ) ;

    connect(ui->Adj1GvarCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj2GvarCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj3GvarCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj4GvarCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj5GvarCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj6GvarCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj7GvarCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
    connect(ui->Adj8GvarCB,SIGNAL(currentIndexChanged(int)),this,SLOT(GvarEdited()));
		
		for ( i = 0 ; i < NUM_GVAR_ADJUST ; i += 1 )
		{
			oldAdjFunction[i] = g_model.gvarAdjuster[0].function ;
		}
    switchEditLock = true;
		gvarVisibility() ;
    switchEditLock = false;

}

void ModelEdit::oneGvarGetValue(int index, QComboBox *b, QSpinBox *sb )
{
	int func = g_model.gvarAdjuster[index].function ;
	if ( ( func < 3 ) || ( func > 6 ) )
	{
	  g_model.gvarAdjuster[index].switch_value = sb->value() ;
	}
	else
	{
		if ( func == 3 )	// Set V
		{
      g_model.gvarAdjuster[index].switch_value = b->currentIndex() ;
		}
		else
		{
	    g_model.gvarAdjuster[index].switch_value = getSwitchCbValue( b, rData->type ) ;
		}
	}
}

void ModelEdit::GvarEdited()
{
    if(switchEditLock) return ;
    switchEditLock = true;
	
	  g_model.gvars[0].gvsource = ui->Gvar1CB->currentIndex() ;
	  g_model.gvars[1].gvsource = ui->Gvar2CB->currentIndex() ;
	  g_model.gvars[2].gvsource = ui->Gvar3CB->currentIndex() ;
  	g_model.gvars[3].gvsource = ui->Gvar4CB->currentIndex() ;
 	  g_model.gvars[4].gvsource = ui->Gvar5CB->currentIndex() ;
 	  g_model.gvars[5].gvsource = ui->Gvar6CB->currentIndex() ;
 	  g_model.gvars[6].gvsource = ui->Gvar7CB->currentIndex() ;

    g_model.gvars[0].gvar = ui->Gv1SB->value();
    g_model.gvars[1].gvar = ui->Gv2SB->value();
    g_model.gvars[2].gvar = ui->Gv3SB->value();
    g_model.gvars[3].gvar = ui->Gv4SB->value();
    g_model.gvars[4].gvar = ui->Gv5SB->value();
    g_model.gvars[5].gvar = ui->Gv6SB->value();
    g_model.gvars[6].gvar = ui->Gv7SB->value();
	
		g_model.gvswitch[0] = getSwitchCbValue( ui->GvSw1CB, rData->type ) ;
		g_model.gvswitch[1] = getSwitchCbValue( ui->GvSw2CB, rData->type ) ;
		g_model.gvswitch[2] = getSwitchCbValue( ui->GvSw3CB, rData->type ) ;
		g_model.gvswitch[3] = getSwitchCbValue( ui->GvSw4CB, rData->type ) ;
		g_model.gvswitch[4] = getSwitchCbValue( ui->GvSw5CB, rData->type ) ;
		g_model.gvswitch[5] = getSwitchCbValue( ui->GvSw6CB, rData->type ) ;
		g_model.gvswitch[6] = getSwitchCbValue( ui->GvSw7CB, rData->type ) ;

		int i ;
		for ( i = 0 ; i < NUM_SCALERS ; i += 1 )
		{
			g_model.Scalers[i].offset = posb[i]->value() ;
			g_model.Scalers[i].mult = pmsb[i]->value()-1 ;
			g_model.Scalers[i].div = pdivsb[i]->value()-1 ;
			g_model.Scalers[i].precision = pdpsb[i]->value() ;
      g_model.Scalers[i].unit = pucb[i]->currentIndex() ;
			g_model.Scalers[i].neg = psgncb[i]->currentIndex() ;
			g_model.Scalers[i].offsetLast = poffcb[i]->currentIndex() ;
			g_model.Scalers[i].source = decodePots( psrccb[i]->currentIndex(), rData->type ) ;
      textUpdate( psname[i], (char *)g_model.Scalers[i].name, 4 ) ;
		}

		for ( i = 0 ; i < NUM_GVAR_ADJUST ; i += 1 )
		{
			oldAdjFunction[i] = g_model.gvarAdjuster[0].function ;
		}
		g_model.gvarAdjuster[0].function = ui->Adj1FunctionCB->currentIndex() ;
		g_model.gvarAdjuster[1].function = ui->Adj2FunctionCB->currentIndex() ;
		g_model.gvarAdjuster[2].function = ui->Adj3FunctionCB->currentIndex() ;
		g_model.gvarAdjuster[3].function = ui->Adj4FunctionCB->currentIndex() ;
		g_model.gvarAdjuster[4].function = ui->Adj5FunctionCB->currentIndex() ;
		g_model.gvarAdjuster[5].function = ui->Adj6FunctionCB->currentIndex() ;
		g_model.gvarAdjuster[6].function = ui->Adj7FunctionCB->currentIndex() ;
		g_model.gvarAdjuster[7].function = ui->Adj8FunctionCB->currentIndex() ;
    
		g_model.gvarAdjuster[0].gvarIndex = ui->Adj1GvarCB->currentIndex() ;
    g_model.gvarAdjuster[1].gvarIndex = ui->Adj2GvarCB->currentIndex() ;
    g_model.gvarAdjuster[2].gvarIndex = ui->Adj3GvarCB->currentIndex() ;
    g_model.gvarAdjuster[3].gvarIndex = ui->Adj4GvarCB->currentIndex() ;
    g_model.gvarAdjuster[4].gvarIndex = ui->Adj5GvarCB->currentIndex() ;
    g_model.gvarAdjuster[5].gvarIndex = ui->Adj6GvarCB->currentIndex() ;
    g_model.gvarAdjuster[6].gvarIndex = ui->Adj7GvarCB->currentIndex() ;
    g_model.gvarAdjuster[7].gvarIndex = ui->Adj8GvarCB->currentIndex() ;

		g_model.gvarAdjuster[0].swtch = getSwitchCbValue( ui->Adj1Sw1CB, rData->type ) ;
		g_model.gvarAdjuster[1].swtch = getSwitchCbValue( ui->Adj2Sw1CB, rData->type ) ;
		g_model.gvarAdjuster[2].swtch = getSwitchCbValue( ui->Adj3Sw1CB, rData->type ) ;
		g_model.gvarAdjuster[3].swtch = getSwitchCbValue( ui->Adj4Sw1CB, rData->type ) ;
		g_model.gvarAdjuster[4].swtch = getSwitchCbValue( ui->Adj5Sw1CB, rData->type ) ;
		g_model.gvarAdjuster[5].swtch = getSwitchCbValue( ui->Adj6Sw1CB, rData->type ) ;
		g_model.gvarAdjuster[6].swtch = getSwitchCbValue( ui->Adj7Sw1CB, rData->type ) ;
		g_model.gvarAdjuster[7].swtch = getSwitchCbValue( ui->Adj8Sw1CB, rData->type ) ;

		oneGvarGetValue( 0, ui->Adj1Sw2CB, ui->Adj1ValueSB ) ;
		oneGvarGetValue( 1, ui->Adj2Sw2CB, ui->Adj2ValueSB ) ;
		oneGvarGetValue( 2, ui->Adj3Sw2CB, ui->Adj3ValueSB ) ;
		oneGvarGetValue( 3, ui->Adj4Sw2CB, ui->Adj4ValueSB ) ;
		oneGvarGetValue( 4, ui->Adj5Sw2CB, ui->Adj5ValueSB ) ;
		oneGvarGetValue( 5, ui->Adj6Sw2CB, ui->Adj6ValueSB ) ;
		oneGvarGetValue( 6, ui->Adj7Sw2CB, ui->Adj7ValueSB ) ;
		oneGvarGetValue( 7, ui->Adj8Sw2CB, ui->Adj8ValueSB ) ;

		gvarVisibility() ;

		updateSettings();
    
		switchEditLock = false;
}

void ModelEdit::tabFrsky()
{
    populateTelItemsCB( ui->Ct1, 0, g_model.customDisplayIndex[0] ) ;
    populateTelItemsCB( ui->Ct2, 0, g_model.customDisplayIndex[1] ) ;
    populateTelItemsCB( ui->Ct3, 0, g_model.customDisplayIndex[2] ) ;
    populateTelItemsCB( ui->Ct4, 0, g_model.customDisplayIndex[3] ) ;
    populateTelItemsCB( ui->Ct5, 0, g_model.customDisplayIndex[4] ) ;
    populateTelItemsCB( ui->Ct6, 0, g_model.customDisplayIndex[5] ) ;
		
    populateTelItemsCB( ui->Ct1_2, 0, g_model.customDisplay2Index[0] ) ;
    populateTelItemsCB( ui->Ct2_2, 0, g_model.customDisplay2Index[1] ) ;
    populateTelItemsCB( ui->Ct3_2, 0, g_model.customDisplay2Index[2] ) ;
    populateTelItemsCB( ui->Ct4_2, 0, g_model.customDisplay2Index[3] ) ;
    populateTelItemsCB( ui->Ct5_2, 0, g_model.customDisplay2Index[4] ) ;
    populateTelItemsCB( ui->Ct6_2, 0, g_model.customDisplay2Index[5] ) ;

    ui->frsky_ratio_0->setValue(g_model.frsky.channels[0].ratio);
    ui->frsky_type_0->setCurrentIndex(g_model.frsky.channels[0].type);
    ui->frsky_ratio_1->setValue(g_model.frsky.channels[1].ratio);
    ui->frsky_type_1->setCurrentIndex(g_model.frsky.channels[1].type);
		FrSkyA1changed(g_model.frsky.channels[0].ratio) ;
		FrSkyA2changed(g_model.frsky.channels[1].ratio) ;

    ui->frsky_val_0_0->setValue(g_model.frsky.channels[0].alarms_value[0]);
    ui->frsky_val_0_1->setValue(g_model.frsky.channels[0].alarms_value[1]);
    ui->frsky_val_1_0->setValue(g_model.frsky.channels[1].alarms_value[0]);
    ui->frsky_val_1_1->setValue(g_model.frsky.channels[1].alarms_value[1]);

    ui->frsky_level_0_0->setCurrentIndex(ALARM_LEVEL(0,0));
    ui->frsky_level_0_1->setCurrentIndex(ALARM_LEVEL(0,1));
    ui->frsky_level_1_0->setCurrentIndex(ALARM_LEVEL(1,0));
    ui->frsky_level_1_1->setCurrentIndex(ALARM_LEVEL(1,1));

    ui->frsky_gr_0_0->setCurrentIndex(ALARM_GREATER(0,0));
    ui->frsky_gr_0_1->setCurrentIndex(ALARM_GREATER(0,1));
    ui->frsky_gr_1_0->setCurrentIndex(ALARM_GREATER(1,0));
    ui->frsky_gr_1_1->setCurrentIndex(ALARM_GREATER(1,1));

    ui->GpsAltMain->setChecked(g_model.FrSkyGpsAlt);
    ui->InvertCom1CB->setChecked(g_model.telemetryRxInvert);
    ui->HubComboBox->setCurrentIndex(g_model.FrSkyUsrProto);
    ui->UnitsComboBox->setCurrentIndex(g_model.FrSkyImperial);
    ui->BladesSpinBox->setValue(g_model.numBlades ) ;
    ui->COMportCB->setCurrentIndex(g_model.frskyComPort ) ;
    ui->BT_telemetry->setChecked(g_model.bt_telemetry) ;
    ui->FASoffsetSB->setValue( (double)g_model.FASoffset/10 + 0.049) ;
		ui->currentSource->setCurrentIndex(g_model.currentSource) ;

    ui->Com2BaudrateCB->setCurrentIndex(g_model.com2Baudrate) ;

    populateSwitchCB(ui->VarioSwitchCB, g_model.varioData.swtch, rData->type ) ;
    ui->VarioSourceCB->setCurrentIndex( g_model.varioData.varioSource ) ;
    ui->VarioSensitivitySB->setValue( g_model.varioData.param ) ;
    ui->SinkTonesOff->setChecked(g_model.varioData.sinkTonesOff);

    populateSwitchCB(ui->LogSwitchCB, g_model.logSwitch, rData->type ) ;
    ui->LogRateCB->setCurrentIndex( g_model.logRate ) ;
		
		ui->frsky_RSSI_Warn->setValue( g_model.rssiOrange + 45 ) ;
		ui->frsky_RSSI_Critical->setValue( g_model.rssiRed + 42 ) ;
		ui->RssiWarnEnabled->setChecked( !g_model.enRssiOrange ) ;
		ui->RssiCriticalEnabled->setChecked( !g_model.enRssiRed ) ;
		ui->frsky_RxV_ratio->setValue( g_model.rxVratio ) ;

    connect(ui->frsky_ratio_0,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_ratio_1,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_type_0,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_type_1,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));

    connect(ui->frsky_val_0_0,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_val_0_1,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_val_1_0,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_val_1_1,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));

    connect(ui->frsky_level_0_0,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_level_0_1,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_level_1_0,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_level_1_1,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));

    connect(ui->frsky_gr_0_0,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_gr_0_1,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_gr_1_0,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_gr_1_1,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
    
		connect(ui->GpsAltMain,SIGNAL(stateChanged(int)),this,SLOT(FrSkyEdited()));
    connect(ui->InvertCom1CB,SIGNAL(stateChanged(int)),this,SLOT(FrSkyEdited()));
		connect(ui->HubComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect(ui->UnitsComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect(ui->BladesSpinBox,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
		connect(ui->COMportCB,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect(ui->BT_telemetry,SIGNAL(stateChanged(int)),this,SLOT(FrSkyEdited()));
    
		connect( ui->Ct1,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct2,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct3,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct4,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct5,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct6,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct1_2,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct2_2,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct3_2,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct4_2,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct5_2,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct6_2,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));

		connect( ui->FASoffsetSB,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
		connect( ui->currentSource,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		
		connect( ui->VarioSensitivitySB,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
		connect( ui->VarioSourceCB,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->VarioSwitchCB,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->SinkTonesOff,SIGNAL(stateChanged(int)),this,SLOT(FrSkyEdited()));

		connect( ui->LogSwitchCB,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->LogRateCB,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));

    connect(ui->frsky_RSSI_Warn,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_RSSI_Critical,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
    connect(ui->frsky_RxV_ratio,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
		connect(ui->RssiWarnEnabled,SIGNAL(stateChanged(int)),this,SLOT(FrSkyEdited()));
		connect(ui->RssiCriticalEnabled,SIGNAL(stateChanged(int)),this,SLOT(FrSkyEdited()));

    connect(ui->frsky_ratio_0,SIGNAL(valueChanged(int)),this,SLOT(FrSkyA1changed(int)));
    connect(ui->frsky_ratio_1,SIGNAL(valueChanged(int)),this,SLOT(FrSkyA2changed(int)));

}

void ModelEdit::FrSkyA1changed(int value)
{
	char telText[20] ;
  stringTelemetryChannel( telText, 0, value, &g_model ) ;
  ui->CH1Value->setText(telText) ;
}

void ModelEdit::FrSkyA2changed(int value)
{
	char telText[20] ;
  stringTelemetryChannel( telText, 1, value, &g_model ) ;
  ui->CH2Value->setText(telText) ;
}


void ModelEdit::FrSkyEdited()
{
//	int limit = MAX_DRSWITCH ;
//	if ( rData->type )
//	{
//		limit = MAX_XDRSWITCH ;
//	}
    g_model.frsky.channels[0].ratio = ui->frsky_ratio_0->value();
    g_model.frsky.channels[1].ratio = ui->frsky_ratio_1->value();
		
		g_model.frsky.channels[0].type  = ui->frsky_type_0->currentIndex();
    g_model.frsky.channels[1].type  = ui->frsky_type_1->currentIndex();

		FrSkyA1changed(g_model.frsky.channels[0].ratio) ;
		FrSkyA2changed(g_model.frsky.channels[1].ratio) ;

    g_model.frsky.channels[0].alarms_value[0] = ui->frsky_val_0_0->value();
    g_model.frsky.channels[0].alarms_value[1] = ui->frsky_val_0_1->value();
    g_model.frsky.channels[1].alarms_value[0] = ui->frsky_val_1_0->value();
    g_model.frsky.channels[1].alarms_value[1] = ui->frsky_val_1_1->value();

    g_model.frsky.channels[0].alarms_level = (ui->frsky_level_0_0->currentIndex() & 3) + ((ui->frsky_level_0_1->currentIndex() & 3) << 2);
    g_model.frsky.channels[1].alarms_level = (ui->frsky_level_1_0->currentIndex() & 3) + ((ui->frsky_level_1_1->currentIndex() & 3) << 2);

    g_model.frsky.channels[0].alarms_greater = (ui->frsky_gr_0_0->currentIndex() & 1) + ((ui->frsky_gr_0_1->currentIndex() & 1) << 1);
    g_model.frsky.channels[1].alarms_greater = (ui->frsky_gr_1_1->currentIndex() & 1) + ((ui->frsky_gr_1_1->currentIndex() & 1) << 1);

    g_model.FrSkyGpsAlt = ui->GpsAltMain->isChecked();
    g_model.telemetryRxInvert = ui->InvertCom1CB->isChecked() ;
		g_model.FrSkyUsrProto = ui->HubComboBox->currentIndex();
    g_model.FrSkyImperial = ui->UnitsComboBox->currentIndex();
    g_model.numBlades = ui->BladesSpinBox->value() ;

		g_model.frskyComPort = ui->COMportCB->currentIndex() ;
    g_model.bt_telemetry = ui->BT_telemetry->isChecked() ;

    g_model.customDisplayIndex[0] = ui->Ct1->currentIndex() ;
    g_model.customDisplayIndex[1] = ui->Ct2->currentIndex() ;
    g_model.customDisplayIndex[2] = ui->Ct3->currentIndex() ;
    g_model.customDisplayIndex[3] = ui->Ct4->currentIndex() ;
    g_model.customDisplayIndex[4] = ui->Ct5->currentIndex() ;
    g_model.customDisplayIndex[5] = ui->Ct6->currentIndex() ;

    g_model.customDisplay2Index[0] = ui->Ct1_2->currentIndex() ;
    g_model.customDisplay2Index[1] = ui->Ct2_2->currentIndex() ;
    g_model.customDisplay2Index[2] = ui->Ct3_2->currentIndex() ;
    g_model.customDisplay2Index[3] = ui->Ct4_2->currentIndex() ;
    g_model.customDisplay2Index[4] = ui->Ct5_2->currentIndex() ;
    g_model.customDisplay2Index[5] = ui->Ct6_2->currentIndex() ;

		g_model.FASoffset = ui->FASoffsetSB->value() * 10 + 0.49 ;
		g_model.currentSource = ui->currentSource->currentIndex() ;
    
		g_model.varioData.swtch = getSwitchCbValue( ui->VarioSwitchCB, rData->type ) ;
		g_model.varioData.varioSource = ui->VarioSourceCB->currentIndex() ;
		g_model.varioData.param = ui->VarioSensitivitySB->value() ;
    g_model.varioData.sinkTonesOff = ui->SinkTonesOff->isChecked();
		
		g_model.logSwitch = getSwitchCbValue( ui->LogSwitchCB, rData->type ) ;
    g_model.logRate = ui->LogRateCB->currentIndex() ;

		g_model.rssiOrange = ui->frsky_RSSI_Warn->value() - 45 ;
		g_model.rssiRed = ui->frsky_RSSI_Critical->value() - 42 ;
		g_model.enRssiOrange = !ui->RssiWarnEnabled->isChecked() ;
		g_model.enRssiRed = !ui->RssiCriticalEnabled->isChecked() ;
		g_model.rxVratio = ui->frsky_RxV_ratio->value() ;
		 
		updateSettings();
}

void ModelEdit::tabTemplates()
{
    ui->templateList->clear();
    ui->templateList->addItem("Simple 4-CH");
    ui->templateList->addItem("T-Cut");
    ui->templateList->addItem("Sticky T-Cut");
    ui->templateList->addItem("V-Tail");
    ui->templateList->addItem("Elevon\\Delta");
    ui->templateList->addItem("Heli Setup");
    ui->templateList->addItem("Heli Gyro Setup");
    ui->templateList->addItem("Servo Test");
    ui->templateList->addItem("Range Test");
    ui->templateList->addItem("Progressive");


}

void ModelEdit::on_modelNameLE_editingFinished()
{
//    uint8_t temp = g_model.mdVers;
    memset(&g_model.name,' ',sizeof(g_model.name));
    QString str = ui->modelNameLE->text().left(10).toLatin1() ;

    for(quint8 i=0; i<(str.length()); i++)
    {
      if(i>=sizeof(g_model.name)) break ;
      g_model.name[i] = (char)str.data()[i].toLatin1() ;
    }
//    g_model.mdVers = temp;  //in case strcpy overruns
	  memcpy( &rData->ModelNames[id_model+1], &g_model.name, sizeof(g_model.name) ) ;
    rData->ModelNames[id_model+1][sizeof( rData->models[0].name)+1] = '\0' ;
    for(int i=0; i<10; i++) if(!g_model.name[i]) g_model.name[i] = ' ';
    updateSettings();

}

void ModelEdit::on_modelImageLE_editingFinished()
{
//    uint8_t temp = g_model.mdVers;
    memset(&g_model.modelImageName,' ',sizeof(g_model.modelImageName));
    QString str = ui->modelImageLE->text().left(10).toLatin1() ;

    for(quint8 i=0; i<(str.length()); i++)
    {
      if(i>=sizeof(g_model.modelImageName)) break ;
      g_model.modelImageName[i] = (char)str.data()[i].toLatin1() ;
    }
//    g_model.mdVers = temp;  //in case strcpy overruns
//    for(int i=0; i<10; i++) if(!g_model.modelImageName[i]) g_model.modelImageName[i] = ' ';
    updateSettings();

}

void ModelEdit::on_timerModeCB_currentIndexChanged(int index)
{
    g_model.timer[0].tmrModeA = index ;
    updateSettings();
}

void ModelEdit::on_timerModeBCB_currentIndexChanged(int index)
{
	(void) index ;
    g_model.timer[0].tmrModeB = getTimerSwitchCbValue( ui->timerModeBCB, rData->type ) ;
    updateSettings();
}

void ModelEdit::on_timerResetCB_currentIndexChanged(int index)
{
	(void) index ;
    g_model.timer1RstSw = getTimerSwitchCbValue( ui->timerResetCB, rData->type ) ;
    updateSettings();
}

void ModelEdit::on_timerDirCB_currentIndexChanged(int index)
{
    g_model.timer[0].tmrDir = index;
    updateSettings();
}

void ModelEdit::on_timer2ModeCB_currentIndexChanged(int index)
{
    g_model.timer[1].tmrModeA = index ;
    updateSettings();
}

void ModelEdit::on_timer2ModeBCB_currentIndexChanged(int index)
{
	(void) index ;
    g_model.timer[1].tmrModeB = getTimerSwitchCbValue( ui->timer2ModeBCB, rData->type ) ;
    updateSettings();
}

void ModelEdit::on_timer2ResetCB_currentIndexChanged(int index)
{
	(void) index ;
    g_model.timer2RstSw = getTimerSwitchCbValue( ui->timer2ResetCB, rData->type ) ;
    updateSettings();
}

void ModelEdit::on_timer2DirCB_currentIndexChanged(int index)
{
    g_model.timer[1].tmrDir = index;
    updateSettings();
}

void ModelEdit::on_trimIncCB_currentIndexChanged(int index)
{
    g_model.trimInc = index;
    updateSettings();
}

void ModelEdit::on_volumeControlCB_currentIndexChanged(int index)
{
    g_model.anaVolume = index ;
    updateSettings();
}

void ModelEdit::on_trimSWCB_currentIndexChanged(int index)
{
	(void) index ;
    g_model.trimSw =  getSwitchCbValue( ui->trimSWCB, rData->type ) ;
    updateSettings();
}

void ModelEdit::on_countryCB_currentIndexChanged(int index)
{
  g_model.country = index ;
  updateSettings();
}
	
void ModelEdit::on_typeCB_currentIndexChanged(int index)
{
  g_model.sub_protocol = index ;
  updateSettings();
}

void ModelEdit::on_pulsePolCB_currentIndexChanged(int index)
{
    g_model.pulsePol = index;
    updateSettings();
}

void ModelEdit::on_xcountryCB_currentIndexChanged(int index)
{
  g_model.xcountry = index ;
  updateSettings();
}
	
void ModelEdit::on_xtypeCB_currentIndexChanged(int index)
{
  g_model.xsub_protocol = index ;
  updateSettings();
}

void ModelEdit::on_xpulsePolCB_currentIndexChanged(int index)
{
    g_model.xpulsePol = index;
    updateSettings();
}


void ModelEdit::on_protocolCB_currentIndexChanged(int index)
{
    if(protocolEditLock) return;
		if ( index == 5 )
		{
			index = PROTO_OFF ;
		}
    g_model.protocol = index;
    g_model.ppmNCH = 0;

    setProtocolBoxes();

    updateSettings();
}

void ModelEdit::on_xprotocolCB_currentIndexChanged(int index)
{
    if(protocolEditLock) return;
		if ( index == 5 )
		{
			index = PROTO_OFF ;
		}
    g_model.xprotocol = index;
    g_model.xppmNCH = 0;

    setProtocolBoxes();

    updateSettings();
}

void ModelEdit::on_timerValTE_editingFinished()
{
    g_model.timer[0].tmrVal = ui->timerValTE->time().minute()*60 + ui->timerValTE->time().second();
    updateSettings();
}

void ModelEdit::on_timer2ValTE_editingFinished()
{
    g_model.timer[1].tmrVal = ui->timer2ValTE->time().minute()*60 + ui->timer2ValTE->time().second();
    updateSettings();
}

void ModelEdit::on_numChannelsSB_editingFinished()
{
    if(protocolEditLock) return;
    int i = (ui->numChannelsSB->value()-8)/2;
    if((i*2+8)!=ui->numChannelsSB->value()) ui->numChannelsSB->setValue(i*2+8);
    g_model.ppmNCH = i;
    updateSettings();
}

void ModelEdit::on_xnumChannelsSB_editingFinished()
{
    if(protocolEditLock) return;
    int i = (ui->numChannels2SB->value()-8)/2;
    if((i*2+8)!=ui->numChannels2SB->value()) ui->numChannels2SB->setValue(i*2+8);
    g_model.ppm2NCH = i ;
    updateSettings();
}

void ModelEdit::on_numChannels2SB_editingFinished()
{
    if(protocolEditLock) return;
    int i = (ui->xnumChannelsSB->value()-8)/2;
    if((i*2+8)!=ui->xnumChannelsSB->value()) ui->xnumChannelsSB->setValue(i*2+8);
    g_model.xppmNCH = i;
    updateSettings();
}

void ModelEdit::on_startChannelsSB_editingFinished()
{
    if(protocolEditLock) return;

    g_model.startChannel = ui->startChannelsSB->value()-1 ;
    updateSettings();
}

void ModelEdit::on_xstartChannelsSB_editingFinished()
{
    if(protocolEditLock) return;

    g_model.xstartChannel = ui->xstartChannelsSB->value()-1 ;
    updateSettings();
}

void ModelEdit::on_startChannels2SB_editingFinished()
{
    if(protocolEditLock) return;

    g_model.startPPM2channel = ui->startChannels2SB->value() ;
    ui->startChannels2SB->setSuffix( (g_model.startPPM2channel == 0) ? " =follow" : "" ) ;
    updateSettings();
}


void ModelEdit::on_DSM_Type_currentIndexChanged(int index)
{
    if(protocolEditLock) return;

    g_model.sub_protocol = index;
    updateSettings();
}

void ModelEdit::on_xDSM_Type_currentIndexChanged(int index)
{
    if(protocolEditLock) return;

    g_model.xsub_protocol = index;
    updateSettings();
}

void ModelEdit::on_SubProtocolCB_currentIndexChanged(int index)
{
    if(protocolEditLock) return;

    g_model.sub_protocol = index;
    updateSettings();
}

void ModelEdit::on_xSubProtocolCB_currentIndexChanged(int index)
{
    if(protocolEditLock) return;

    g_model.xsub_protocol = index;
    updateSettings();
}

void ModelEdit::on_VoiceNumberSB_editingFinished()
{
    g_model.modelVoice = ui->VoiceNumberSB->value()-260;
    updateSettings();
}

void ModelEdit::on_pxxRxNum_editingFinished()
{
    if(protocolEditLock) return;

    g_model.pxxRxNum = ui->pxxRxNum->value() ;
    updateSettings();
}


void ModelEdit::on_ppmDelaySB_editingFinished()
{
    if(protocolEditLock) return;
    int i = (ui->ppmDelaySB->value()-300)/50;
    if((i*50+300)!=ui->ppmDelaySB->value()) ui->ppmDelaySB->setValue(i*50+300);
    g_model.ppmDelay = i;
    updateSettings();
}


void ModelEdit::on_autoLimitsSB_editingFinished()
{
   g_model.sub_trim_limit = ui->autoLimitsSB->value()*10 + 0.49 ;
	 limitAuto() ;
   updateSettings();
}

void ModelEdit::on_thrTrimChkB_toggled(bool checked)
{
    g_model.thrTrim = checked;
    updateSettings();
}

void ModelEdit::on_TrainerChkB_toggled(bool checked)
{
    g_model.traineron = checked;
    updateSettings();
}

void ModelEdit::on_thrExpoChkB_toggled(bool checked)
{
// 	int i, j ;
//	bool x = true ;
//	if ( checked )
//	{
//		x = false ;		
//	}
  g_model.thrExpo = checked ;
//	for ( i = 0 ; i < 3 ; i += 1 )
//	{ // 0=High, 1=Mid, 2=Low
//		for ( j = 0 ; j < 2 ; j += 1 )
//		{ // 0=Weight, 1=Expo
//			expoDrSpin[2][i][j][1]->setEnabled(x);
//			expoDrVal[2][i][j][1]->setEnabled(x);
//			expoDrGvar[2][i][j][1]->setEnabled(x);
//		}
//	}		
  updateSettings();
}

void ModelEdit::on_bcRUDChkB_toggled(bool checked)
{
    if(checked)
        g_model.beepANACenter |= BC_BIT_RUD;
    else
        g_model.beepANACenter &= ~BC_BIT_RUD;
    updateSettings();
}

void ModelEdit::on_bcELEChkB_toggled(bool checked)
{
    if(checked)
        g_model.beepANACenter |= BC_BIT_ELE;
    else
        g_model.beepANACenter &= ~BC_BIT_ELE;
    updateSettings();
}

void ModelEdit::on_bcTHRChkB_toggled(bool checked)
{
    if(checked)
        g_model.beepANACenter |= BC_BIT_THR;
    else
        g_model.beepANACenter &= ~BC_BIT_THR;
    updateSettings();
}

void ModelEdit::on_bcAILChkB_toggled(bool checked)
{
    if(checked)
        g_model.beepANACenter |= BC_BIT_AIL;
    else
        g_model.beepANACenter &= ~BC_BIT_AIL;
    updateSettings();
}

void ModelEdit::on_bcP1ChkB_toggled(bool checked)
{
    if(checked)
        g_model.beepANACenter |= BC_BIT_P1;
    else
        g_model.beepANACenter &= ~BC_BIT_P1;
    updateSettings();
}

void ModelEdit::on_bcP2ChkB_toggled(bool checked)
{
    if(checked)
        g_model.beepANACenter |= BC_BIT_P2;
    else
        g_model.beepANACenter &= ~BC_BIT_P2;
    updateSettings();
}

void ModelEdit::on_bcP3ChkB_toggled(bool checked)
{
    if(checked)
        g_model.beepANACenter |= BC_BIT_P3;
    else
        g_model.beepANACenter &= ~BC_BIT_P3;
    updateSettings();
}

void ModelEdit::on_timer1BeepCdownCB_toggled(bool checked)
{
	g_model.timer1Cdown = checked ;
  updateSettings();
}

void ModelEdit::on_timer2BeepCdownCB_toggled(bool checked)
{
	g_model.timer2Cdown = checked ;
  updateSettings();
}

void ModelEdit::on_timer1MinuteBeepCB_toggled(bool checked)
{
	g_model.timer1Mbeep = checked ;
  updateSettings();
}

void ModelEdit::on_timer2MinuteBeepCB_toggled(bool checked)
{
	g_model.timer2Mbeep = checked ;
  updateSettings();
}

void ModelEdit::on_switchwarnChkB_stateChanged(int )
{
    g_model.modelswitchWarningStates = (g_model.modelswitchWarningStates & ~1) | (ui->switchwarnChkB->isChecked() ? 0 : 1);
    updateSettings();
}

void ModelEdit::getModelSwitchDefPos(int i, bool val)
{
    if(val)
        g_model.modelswitchWarningStates |= (1<<(i));
    else
        g_model.modelswitchWarningStates &= ~(1<<(i));
}

void ModelEdit::on_EnA_stateChanged(int )
{
	if ( ui->EnA->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~0x0003 ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= 0x0003 ;
	}
	updateSettings();
}

void ModelEdit::on_EnB_stateChanged(int )
{
	if ( ui->EnB->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~0x000C ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= 0x000C ;
	}
	updateSettings();
}

void ModelEdit::on_EnC_stateChanged(int )
{
	if ( ui->EnC->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~0x0030 ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= 0x0030 ;
	}
	updateSettings();
}

void ModelEdit::on_EnD_stateChanged(int )
{
	if ( ui->EnD->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~0x00C0 ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= 0x00C0 ;
	}
	updateSettings();
}

void ModelEdit::on_EnE_stateChanged(int )
{
	if ( ui->EnE->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~0x0300 ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= 0x0300 ;
	}
	updateSettings();
}

void ModelEdit::on_EnF_stateChanged(int )
{
	if ( ui->EnF->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~0x0C00 ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= 0x0C00 ;
	}
	updateSettings();
}

void ModelEdit::on_EnG_stateChanged(int )
{
	if ( ui->EnG->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~0x3000 ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= 0x3000 ;
	}
	updateSettings();
}

void ModelEdit::on_EnThr_stateChanged(int )
{
	if ( ui->EnThr->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~THR_WARN_MASK ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= THR_WARN_MASK ;
	}
	updateSettings();
}

void ModelEdit::on_EnRud_stateChanged(int )
{
	if ( ui->EnRud->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~RUD_WARN_MASK ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= RUD_WARN_MASK ;
	}
	updateSettings();
}

void ModelEdit::on_EnEle_stateChanged(int )
{
	if ( ui->EnEle->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~ELE_WARN_MASK ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= ELE_WARN_MASK ;
	}
	updateSettings();
}

void ModelEdit::on_EnIdx_stateChanged(int )
{
	if ( ui->EnIdx->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~IDX_WARN_MASK ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= IDX_WARN_MASK ;
	}
	updateSettings();
}

void ModelEdit::on_EnAil_stateChanged(int )
{
	if ( ui->EnAil->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~AIL_WARN_MASK ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= AIL_WARN_MASK ;
	}
	updateSettings();
}

void ModelEdit::on_EnGea_stateChanged(int )
{
	if ( ui->EnGea->isChecked() )
	{
		g_model.modelswitchWarningDisables &= ~GEA_WARN_MASK ;
	}
	else
	{
		g_model.modelswitchWarningDisables |= GEA_WARN_MASK ;
	}
	updateSettings();
}


void ModelEdit::on_switchDefPos_1_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getModelSwitchDefPos(1,ui->switchDefPos_1->isChecked());
    updateSettings();
}
void ModelEdit::on_switchDefPos_2_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getModelSwitchDefPos(2,ui->switchDefPos_2->isChecked());
    updateSettings();
}
void ModelEdit::on_switchDefPos_3_stateChanged(int )
{
    getModelSwitchDefPos(3,ui->switchDefPos_3->isChecked());
    updateSettings();
}
void ModelEdit::on_switchDefPos_4_stateChanged(int )
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

    g_model.modelswitchWarningStates &= ~(0x30<<1); //turn off ID1/2
    getModelSwitchDefPos(4,ui->switchDefPos_4->isChecked());
    updateSettings();
}
void ModelEdit::on_switchDefPos_5_stateChanged(int )
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

    g_model.modelswitchWarningStates &= ~(0x28<<1); //turn off ID0/2
    getModelSwitchDefPos(5,ui->switchDefPos_5->isChecked());
    updateSettings();
}
void ModelEdit::on_switchDefPos_6_stateChanged(int )
{
    if(switchDefPosEditLock) return;

    if(ui->switchDefPos_6->isChecked())
    {
        switchDefPosEditLock = true;
        ui->switchDefPos_4->setChecked(false);
        ui->switchDefPos_5->setChecked(false);
        switchDefPosEditLock = false;
    }
    else
        return;

    g_model.modelswitchWarningStates &= ~(0x18<<1); //turn off ID1/2
    getModelSwitchDefPos(6,ui->switchDefPos_6->isChecked());
    updateSettings();
}
void ModelEdit::on_switchDefPos_7_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getModelSwitchDefPos(7,ui->switchDefPos_7->isChecked());
    updateSettings();
}
void ModelEdit::on_switchDefPos_8_stateChanged(int )
{
    if(switchDefPosEditLock) return;
    getModelSwitchDefPos(8,ui->switchDefPos_8->isChecked());
    updateSettings();
}

void ModelEdit::on_SwitchDefSA_valueChanged( int x )
{
    if(switchDefPosEditLock) return;
		if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
		{
	    x <<= 1 ;
	    g_model.modelswitchWarningStates = ( g_model.modelswitchWarningStates & ~0x0006 ) | x ;
		}
		else
		{ // must be THR 3POS
			uint16_t y = g_model.modelswitchWarningStates ;
			y &= ~(0x0101 << 1) ;
			if ( x )
			{
				if ( x == 1 )
				{
					y |= 0x0001 << 1 ;
				}
				else
				{
					y |= 0x0100 << 1 ;
				}
			}
			g_model.modelswitchWarningStates = y ;
		}
		updateSettings();
}

void ModelEdit::on_SwitchDefSB_valueChanged( int x )
{
    if(switchDefPosEditLock) return;
		if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
		{
	    x <<= 3 ;
  	  g_model.modelswitchWarningStates = ( g_model.modelswitchWarningStates & ~0x0018 ) | x ;
		}
		else
		{ // must be RUD 3POS
			uint16_t y = g_model.modelswitchWarningStates ;
			y &= ~(0x0202 << 1) ;
			if ( x )
			{
				if ( x == 1 )
				{
					y |= 0x0002 << 1 ;
				}
				else
				{
					y |= 0x0200 << 1 ;
				}
			}
			g_model.modelswitchWarningStates = y ;
		}
		updateSettings();
}

void ModelEdit::on_SwitchDefSC_valueChanged( int x )
{
    if(switchDefPosEditLock) return;
		if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
		{
	    x <<= 5 ;
  	  g_model.modelswitchWarningStates = ( g_model.modelswitchWarningStates & ~0x0060 ) | x ;
		}
		else
		{ // must be ELE 3POS
			uint16_t y = g_model.modelswitchWarningStates ;
			y &= ~(0x0404 << 1) ;
			if ( x )
			{
				if ( x == 1 )
				{
					y |= 0x0004 << 1 ;
				}
				else
				{
					y |= 0x0400 << 1 ;
				}
			}
			g_model.modelswitchWarningStates = y ;
		}
		updateSettings();
}
void ModelEdit::on_SwitchDefSD_valueChanged( int x )
{
    if(switchDefPosEditLock) return;
		if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
		{
	    x <<= 7 ;
  	  g_model.modelswitchWarningStates = ( g_model.modelswitchWarningStates & ~0x0180 ) | x ;
		}
		else
		{ // must be AIL 3POS
			uint16_t y = g_model.modelswitchWarningStates ;
			y &= ~(0x1040 << 1) ;
			if ( x )
			{
				if ( x == 1 )
				{
					y |= 0x0040 << 1 ;
				}
				else
				{
					y |= 0x1000 << 1 ;
				}
			}
			g_model.modelswitchWarningStates = y ;
		}
		updateSettings();
}
void ModelEdit::on_SwitchDefSE_valueChanged( int x )
{
    if(switchDefPosEditLock) return;
		if ( ( rData->type == 1 ) || ( rData->type == 2 ) )
		{
	    x <<= 9 ;
  	  g_model.modelswitchWarningStates = ( g_model.modelswitchWarningStates & ~0x0600 ) | x ;
		}
		else
		{ // must be GEA 3POS
			uint16_t y = g_model.modelswitchWarningStates ;
			y &= ~(0x2080 << 1) ;
			if ( x )
			{
				if ( x == 1 )
				{
					y |= 0x0080 << 1 ;
				}
				else
				{
					y |= 0x2000 << 1 ;
				}
			}
			g_model.modelswitchWarningStates = y ;
		}
		updateSettings();
}
void ModelEdit::on_SwitchDefSF_valueChanged( int x )
{
		if ( x )
		{
			x = 2 ;
		}
    if(switchDefPosEditLock) return;
    x <<= 11 ;
    g_model.modelswitchWarningStates = ( g_model.modelswitchWarningStates & ~0x1800 ) | x ;
		updateSettings();
}
void ModelEdit::on_SwitchDefSG_valueChanged( int x )
{
    if(switchDefPosEditLock) return;
    x <<= 13 ;
    g_model.modelswitchWarningStates = ( g_model.modelswitchWarningStates & ~0x6000 ) | x ;
		updateSettings();
}

void ModelEdit::on_spinBox_S1_valueChanged(int value)
{
        g_model.trim[(g_eeGeneral.stickMode>1) ? 3 : 0] = value;
        updateSettings();
}

void ModelEdit::on_spinBox_S2_valueChanged(int value)
{
        g_model.trim[(g_eeGeneral.stickMode & 1) ? 2 : 1] = value;
        updateSettings();
}

void ModelEdit::on_spinBox_S3_valueChanged(int value)
{
        g_model.trim[(g_eeGeneral.stickMode & 1) ? 1 : 2] = value;
        updateSettings();
}

void ModelEdit::on_spinBox_S4_valueChanged(int value)
{
        g_model.trim[(g_eeGeneral.stickMode>1)   ? 0 : 3] = value;
        updateSettings();
}

QSpinBox *ModelEdit::getNodeSB(int i)   // get the SpinBox that corresponds to the selected node
{
    if(currentCurve==0 && i==0) return ui->curvePt1_1;
    if(currentCurve==0 && i==1) return ui->curvePt2_1;
    if(currentCurve==0 && i==2) return ui->curvePt3_1;
    if(currentCurve==0 && i==3) return ui->curvePt4_1;
    if(currentCurve==0 && i==4) return ui->curvePt5_1;

    if(currentCurve==1 && i==0) return ui->curvePt1_2;
    if(currentCurve==1 && i==1) return ui->curvePt2_2;
    if(currentCurve==1 && i==2) return ui->curvePt3_2;
    if(currentCurve==1 && i==3) return ui->curvePt4_2;
    if(currentCurve==1 && i==4) return ui->curvePt5_2;

    if(currentCurve==2 && i==0) return ui->curvePt1_3;
    if(currentCurve==2 && i==1) return ui->curvePt2_3;
    if(currentCurve==2 && i==2) return ui->curvePt3_3;
    if(currentCurve==2 && i==3) return ui->curvePt4_3;
    if(currentCurve==2 && i==4) return ui->curvePt5_3;

    if(currentCurve==3 && i==0) return ui->curvePt1_4;
    if(currentCurve==3 && i==1) return ui->curvePt2_4;
    if(currentCurve==3 && i==2) return ui->curvePt3_4;
    if(currentCurve==3 && i==3) return ui->curvePt4_4;
    if(currentCurve==3 && i==4) return ui->curvePt5_4;

    if(currentCurve==4 && i==0) return ui->curvePt1_5;
    if(currentCurve==4 && i==1) return ui->curvePt2_5;
    if(currentCurve==4 && i==2) return ui->curvePt3_5;
    if(currentCurve==4 && i==3) return ui->curvePt4_5;
    if(currentCurve==4 && i==4) return ui->curvePt5_5;

    if(currentCurve==5 && i==0) return ui->curvePt1_6;
    if(currentCurve==5 && i==1) return ui->curvePt2_6;
    if(currentCurve==5 && i==2) return ui->curvePt3_6;
    if(currentCurve==5 && i==3) return ui->curvePt4_6;
    if(currentCurve==5 && i==4) return ui->curvePt5_6;

    if(currentCurve==6 && i==0) return ui->curvePt1_7;
    if(currentCurve==6 && i==1) return ui->curvePt2_7;
    if(currentCurve==6 && i==2) return ui->curvePt3_7;
    if(currentCurve==6 && i==3) return ui->curvePt4_7;
    if(currentCurve==6 && i==4) return ui->curvePt5_7;

    if(currentCurve==7 && i==0) return ui->curvePt1_8;
    if(currentCurve==7 && i==1) return ui->curvePt2_8;
    if(currentCurve==7 && i==2) return ui->curvePt3_8;
    if(currentCurve==7 && i==3) return ui->curvePt4_8;
    if(currentCurve==7 && i==4) return ui->curvePt5_8;


    if(currentCurve==8 && i==0) return ui->curvePt1_9;
    if(currentCurve==8 && i==1) return ui->curvePt2_9;
    if(currentCurve==8 && i==2) return ui->curvePt3_9;
    if(currentCurve==8 && i==3) return ui->curvePt4_9;
    if(currentCurve==8 && i==4) return ui->curvePt5_9;
    if(currentCurve==8 && i==5) return ui->curvePt6_9;
    if(currentCurve==8 && i==6) return ui->curvePt7_9;
    if(currentCurve==8 && i==7) return ui->curvePt8_9;
    if(currentCurve==8 && i==8) return ui->curvePt9_9;

    if(currentCurve==9 && i==0) return ui->curvePt1_10;
    if(currentCurve==9 && i==1) return ui->curvePt2_10;
    if(currentCurve==9 && i==2) return ui->curvePt3_10;
    if(currentCurve==9 && i==3) return ui->curvePt4_10;
    if(currentCurve==9 && i==4) return ui->curvePt5_10;
    if(currentCurve==9 && i==5) return ui->curvePt6_10;
    if(currentCurve==9 && i==6) return ui->curvePt7_10;
    if(currentCurve==9 && i==7) return ui->curvePt8_10;
    if(currentCurve==9 && i==8) return ui->curvePt9_10;

    if(currentCurve==10 && i==0) return ui->curvePt1_11;
    if(currentCurve==10 && i==1) return ui->curvePt2_11;
    if(currentCurve==10 && i==2) return ui->curvePt3_11;
    if(currentCurve==10 && i==3) return ui->curvePt4_11;
    if(currentCurve==10 && i==4) return ui->curvePt5_11;
    if(currentCurve==10 && i==5) return ui->curvePt6_11;
    if(currentCurve==10 && i==6) return ui->curvePt7_11;
    if(currentCurve==10 && i==7) return ui->curvePt8_11;
    if(currentCurve==10 && i==8) return ui->curvePt9_11;

    if(currentCurve==11 && i==0) return ui->curvePt1_12;
    if(currentCurve==11 && i==1) return ui->curvePt2_12;
    if(currentCurve==11 && i==2) return ui->curvePt3_12;
    if(currentCurve==11 && i==3) return ui->curvePt4_12;
    if(currentCurve==11 && i==4) return ui->curvePt5_12;
    if(currentCurve==11 && i==5) return ui->curvePt6_12;
    if(currentCurve==11 && i==6) return ui->curvePt7_12;
    if(currentCurve==11 && i==7) return ui->curvePt8_12;
    if(currentCurve==11 && i==8) return ui->curvePt9_12;

    if(currentCurve==12 && i==0) return ui->curvePt1_13;
    if(currentCurve==12 && i==1) return ui->curvePt2_13;
    if(currentCurve==12 && i==2) return ui->curvePt3_13;
    if(currentCurve==12 && i==3) return ui->curvePt4_13;
    if(currentCurve==12 && i==4) return ui->curvePt5_13;
    if(currentCurve==12 && i==5) return ui->curvePt6_13;
    if(currentCurve==12 && i==6) return ui->curvePt7_13;
    if(currentCurve==12 && i==7) return ui->curvePt8_13;
    if(currentCurve==12 && i==8) return ui->curvePt9_13;

    if(currentCurve==13 && i==0) return ui->curvePt1_14;
    if(currentCurve==13 && i==1) return ui->curvePt2_14;
    if(currentCurve==13 && i==2) return ui->curvePt3_14;
    if(currentCurve==13 && i==3) return ui->curvePt4_14;
    if(currentCurve==13 && i==4) return ui->curvePt5_14;
    if(currentCurve==13 && i==5) return ui->curvePt6_14;
    if(currentCurve==13 && i==6) return ui->curvePt7_14;
    if(currentCurve==13 && i==7) return ui->curvePt8_14;
    if(currentCurve==13 && i==8) return ui->curvePt9_14;

    if(currentCurve==14 && i==0) return ui->curvePt1_15;
    if(currentCurve==14 && i==1) return ui->curvePt2_15;
    if(currentCurve==14 && i==2) return ui->curvePt3_15;
    if(currentCurve==14 && i==3) return ui->curvePt4_15;
    if(currentCurve==14 && i==4) return ui->curvePt5_15;
    if(currentCurve==14 && i==5) return ui->curvePt6_15;
    if(currentCurve==14 && i==6) return ui->curvePt7_15;
    if(currentCurve==14 && i==7) return ui->curvePt8_15;
    if(currentCurve==14 && i==8) return ui->curvePt9_15;

    if(currentCurve==15 && i==0) return ui->curvePt1_16;
    if(currentCurve==15 && i==1) return ui->curvePt2_16;
    if(currentCurve==15 && i==2) return ui->curvePt3_16;
    if(currentCurve==15 && i==3) return ui->curvePt4_16;
    if(currentCurve==15 && i==4) return ui->curvePt5_16;
    if(currentCurve==15 && i==5) return ui->curvePt6_16;
    if(currentCurve==15 && i==6) return ui->curvePt7_16;
    if(currentCurve==15 && i==7) return ui->curvePt8_16;
    if(currentCurve==15 && i==8) return ui->curvePt9_16;

    return 0;
}

void ModelEdit::drawCurve()
{
    int k,i;
    QColor * plot_color[16];
    plot_color[0]=new QColor(0,0,127);
    plot_color[1]=new QColor(0,127,0);
    plot_color[2]=new QColor(127,0,0);
    plot_color[3]=new QColor(0,127,127);
    plot_color[4]=new QColor(127,0,127);
    plot_color[5]=new QColor(127,127,0);
    plot_color[6]=new QColor(127,127,127);
    plot_color[7]=new QColor(0,0,255);
    plot_color[8]=new QColor(0,127,255);
    plot_color[9]=new QColor(127,0,255);
    plot_color[10]=new QColor(0,255,0);
    plot_color[11]=new QColor(0,255,127);
    plot_color[12]=new QColor(127,255,0);
    plot_color[13]=new QColor(255,0,0);
    plot_color[14]=new QColor(255,0,127);
    plot_color[15]=new QColor(255,127,0);
    
		if(currentCurve<0 || currentCurve>15) return;

    Node *nodel = 0;
    Node *nodex = 0;

    QGraphicsScene *scene = ui->curvePreview->scene();
    QPen pen;
    QColor color;
    scene->clear();

    qreal width  = scene->sceneRect().width();
    qreal height = scene->sceneRect().height();

    qreal centerX = scene->sceneRect().left() + width/2; //center X
    qreal centerY = scene->sceneRect().top() + height/2; //center Y

    QGraphicsSimpleTextItem *ti;
    ti = scene->addSimpleText(tr("Editing curve %1").arg(currentCurve+1));
    ti->setPos(3,3);

    scene->addLine(centerX,GFX_MARGIN,centerX,height+GFX_MARGIN);
    scene->addLine(GFX_MARGIN,centerY,width+GFX_MARGIN,centerY);

    pen.setWidth(2);
    pen.setStyle(Qt::SolidLine);
    for(k=0; k<8; k++) {
        pen.setColor(*plot_color[k]);
        if ((currentCurve!=k) && (plot_curve[k])) {
           for(i=0; i<4; i++) {
                scene->addLine(GFX_MARGIN + i*width/(5-1),centerY - (qreal)g_model.curves5[k][i]*height/200,GFX_MARGIN + (i+1)*width/(5-1),centerY - (qreal)g_model.curves5[k][i+1]*height/200,pen);    
           }
        }
    }
    for(k=0; k<8; k++) {
        pen.setColor(*plot_color[k+8]);
        if ((currentCurve!=(k+8)) && (plot_curve[k+8])) {
           for(i=0; i<8; i++) {
                scene->addLine(GFX_MARGIN + i*width/(9-1),centerY - (qreal)g_model.curves9[k][i]*height/200,GFX_MARGIN + (i+1)*width/(9-1),centerY - (qreal)g_model.curves9[k][i+1]*height/200,pen);    
           }
        }
    }

    if(currentCurve<8)
        for(i=0; i<5; i++)
        {
            nodel = nodex;
            nodex = new Node(getNodeSB(i));
            nodex->setFixedX(true);

            nodex->setPos(GFX_MARGIN + i*width/(5-1),centerY - (qreal)g_model.curves5[currentCurve][i]*height/200);
            scene->addItem(nodex);
            if(i>0) scene->addItem(new Edge(nodel, nodex));
        }
    else
        for(i=0; i<9; i++)
        {
            nodel = nodex;
            nodex = new Node(getNodeSB(i));
            nodex->setFixedX(true);

            nodex->setPos(GFX_MARGIN + i*width/(9-1),centerY - (qreal)g_model.curves9[currentCurve-8][i]*height/200);
            scene->addItem(nodex);
            if(i>0) scene->addItem(new Edge(nodel, nodex));
        }
}



void ModelEdit::on_curveEdit_1_clicked()
{
    setCurrentCurve(0);
    drawCurve();
}

void ModelEdit::on_curveEdit_2_clicked()
{
    setCurrentCurve(1);
    drawCurve();
}

void ModelEdit::on_curveEdit_3_clicked()
{
    setCurrentCurve(2);
    drawCurve();
}

void ModelEdit::on_curveEdit_4_clicked()
{
    setCurrentCurve(3);
    drawCurve();
}

void ModelEdit::on_curveEdit_5_clicked()
{
    setCurrentCurve(4);
    drawCurve();
}

void ModelEdit::on_curveEdit_6_clicked()
{
    setCurrentCurve(5);
    drawCurve();
}

void ModelEdit::on_curveEdit_7_clicked()
{
    setCurrentCurve(6);
    drawCurve();
}

void ModelEdit::on_curveEdit_8_clicked()
{
    setCurrentCurve(7);
    drawCurve();
}

void ModelEdit::on_curveEdit_9_clicked()
{
    setCurrentCurve(8);
    drawCurve();
}

void ModelEdit::on_curveEdit_10_clicked()
{
    setCurrentCurve(9);
    drawCurve();
}

void ModelEdit::on_curveEdit_11_clicked()
{
    setCurrentCurve(10);
    drawCurve();
}

void ModelEdit::on_curveEdit_12_clicked()
{
    setCurrentCurve(11);
    drawCurve();
}

void ModelEdit::on_curveEdit_13_clicked()
{
    setCurrentCurve(12);
    drawCurve();
}

void ModelEdit::on_curveEdit_14_clicked()
{
    setCurrentCurve(13);
    drawCurve();
}

void ModelEdit::on_curveEdit_15_clicked()
{
    setCurrentCurve(14);
    drawCurve();
}

void ModelEdit::on_curveEdit_16_clicked()
{
    setCurrentCurve(15);
    drawCurve();
}


bool ModelEdit::gm_insertMix(int idx)
{
    if(idx<0 || idx>MAX_SKYMIXERS) return false;
    if(g_model.mixData[MAX_SKYMIXERS-1].destCh) return false; //if last mixer isn't empty - can't add more

    int i = g_model.mixData[idx].destCh;
    memmove(&g_model.mixData[idx+1],&g_model.mixData[idx],
            (MAX_SKYMIXERS-(idx+1))*sizeof(SKYMixData) );
    memset(&g_model.mixData[idx],0,sizeof(SKYMixData));
    g_model.mixData[idx].destCh = i;
    g_model.mixData[idx].weight = 100;
		g_model.mixData[idx].lateOffset = 1 ;

    for(int j=(MAX_SKYMIXERS-1); j>idx; j--)
    {
        mixNotes[j].clear();
        mixNotes[j].append(mixNotes[j-1]);
    }
    mixNotes[idx].clear();

    return true;
}

void ModelEdit::gm_deleteMix(int index)
{
  memmove(&g_model.mixData[index],&g_model.mixData[index+1],
            (MAX_SKYMIXERS-(index+1))*sizeof(SKYMixData));
  memset(&g_model.mixData[MAX_SKYMIXERS-1],0,sizeof(SKYMixData));

  for(int j=index; j<(MAX_SKYMIXERS-1); j++)
  {
      mixNotes[j].clear();
      mixNotes[j].append(mixNotes[j+1]);
  }
  mixNotes[MAX_SKYMIXERS-1].clear();
}

void ModelEdit::gm_openMix(int index)
{
    if(index<0 || index>MAX_SKYMIXERS) return;

    SKYMixData mixd;
    memcpy(&mixd,&g_model.mixData[index],sizeof(SKYMixData));

    updateSettings();
    tabMixes();

    QString comment = mixNotes[index];

    MixerDialog *g = new MixerDialog(this,&mixd, &g_eeGeneral, &comment, g_model.modelVersion, rData->type );
    if(g->exec())
    {
        memcpy(&g_model.mixData[index],&mixd,sizeof(SKYMixData));

        mixNotes[index] = comment;

        updateSettings();
        tabMixes();
    }
}

int ModelEdit::getMixerIndex(int dch)
{
    int i = 0;
    while ((g_model.mixData[i].destCh<=dch) && (g_model.mixData[i].destCh) && (i<MAX_SKYMIXERS)) i++;
    if(i==MAX_SKYMIXERS) return -1;
    return i;
}

void ModelEdit::on_VoiceAlarmList_doubleClicked( QModelIndex index )
{
	int i = index.row() ;
	VoiceAlarmData *vad = ( i >= NUM_SKY_VOICE_ALARMS) ? &g_model.vadx[i-NUM_SKY_VOICE_ALARMS] : &g_model.vad[i] ;
  VoiceAlarmDialog *dlg = new VoiceAlarmDialog( this, vad, rData->type, g_eeGeneral.stickMode, g_model.modelVersion, &g_model ) ;
  dlg->setWindowTitle(tr("Voice Alarm %1").arg(index.row()+1)) ;
  if(dlg->exec())
  {
    updateSettings() ;
		tabVoiceAlarms() ;
  }
}

void ModelEdit::mixerlistWidget_doubleClicked(QModelIndex index)
{
    int idx= MixerlistWidget->item(index.row())->data(Qt::UserRole).toByteArray().at(0);
    if(idx<0)
    {
        int i = -idx;
        idx = getMixerIndex(i); //get mixer index to insert
        if(!gm_insertMix(idx))
            return; //if full - don't add any more mixes
        g_model.mixData[idx].destCh = i;
    }
    gm_openMix(idx);
}

void ModelEdit::mixersDeleteList(QList<int> list)
{
    qSort(list.begin(), list.end());

    int iDec = 0;
    foreach(int idx, list)
    {
        gm_deleteMix(idx-iDec);
        iDec++;
    }
}

QList<int> ModelEdit::createListFromSelected()
{
    QList<int> list;
    foreach(QListWidgetItem *item, MixerlistWidget->selectedItems())
    {
        int idx= item->data(Qt::UserRole).toByteArray().at(0);
        if(idx>=0 && idx<MAX_SKYMIXERS) list << idx;
    }
    return list;
}


void ModelEdit::setSelectedByList(QList<int> list)
{
    for(int i=0; i<MixerlistWidget->count(); i++)
    {
        int t = MixerlistWidget->item(i)->data(Qt::UserRole).toByteArray().at(0);
        if(list.contains(t))
            MixerlistWidget->item(i)->setSelected(true);
    }
}

void ModelEdit::mixersDelete(bool ask)
{
    int curpos = MixerlistWidget->currentRow();

    QMessageBox::StandardButton ret = QMessageBox::No;

    if(ask)
        ret = QMessageBox::warning(this, "eePe",
                 tr("Delete Selected Mixes?"),
                 QMessageBox::Yes | QMessageBox::No);


    if ((ret == QMessageBox::Yes) || (!ask))
    {
        mixersDeleteList(createListFromSelected());
        updateSettings();
        tabMixes();

        MixerlistWidget->setCurrentRow(curpos);
    }
}

void ModelEdit::mixersCut()
{
    mixersCopy();
    mixersDelete(false);
}

void ModelEdit::mixersCopy()
{
    QList<int> list = createListFromSelected();

    QByteArray mxData;
    foreach(int idx, list)
        mxData.append((char*)&g_model.mixData[idx],sizeof(SKYMixData));

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-eepe-mix", mxData);

    QApplication::clipboard()->setMimeData(mimeData,QClipboard::Clipboard);
}

void ModelEdit::mimeDropped(int index, const QMimeData *data, Qt::DropAction action)
{
    int idx= MixerlistWidget->item(index)->data(Qt::UserRole).toByteArray().at(0);
    pasteMIMEData(data,idx);
    if(action) {}
}

void ModelEdit::pasteMIMEData(const QMimeData * mimeData, int destIdx)
{
    int curpos = MixerlistWidget->currentRow();

    if(mimeData->hasFormat("application/x-eepe-mix"))
    {
        int idx = MixerlistWidget->currentItem()->data(Qt::UserRole).toByteArray().at(0);
        if(destIdx!=1000)
            idx = destIdx>=0 ? destIdx-1 : destIdx;


        int dch = -idx;
        if(idx<0)
            idx = getMixerIndex(-idx) - 1; //get mixer index to insert
        else
            dch = g_model.mixData[idx].destCh;

        QByteArray mxData = mimeData->data("application/x-eepe-mix");

        int i = 0;
        while(i<mxData.size())
        {
            idx++;
            if(idx==MAX_SKYMIXERS) break;

            if(!gm_insertMix(idx))
                break; //memory full - can't add any more
            SKYMixData *md = &g_model.mixData[idx];
            memcpy(md,mxData.mid(i,sizeof(SKYMixData)).constData(),sizeof(SKYMixData));
            md->destCh = dch;

            i     += sizeof(SKYMixData);
        }

        updateSettings();
        tabMixes();

        MixerlistWidget->setCurrentRow(curpos);
    }
}

void ModelEdit::mixersPaste()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    pasteMIMEData(mimeData);
}

void ModelEdit::mixersDuplicate()
{
    mixersCopy();
    mixersPaste();
}

void ModelEdit::mixerOpen()
{
    int idx = MixerlistWidget->currentItem()->data(Qt::UserRole).toByteArray().at(0);
    if(idx<0)
    {
        int i = -idx;
        idx = getMixerIndex(i); //get mixer index to insert
        if(!gm_insertMix(idx))
            return;
        g_model.mixData[idx].destCh = i;
    }
    gm_openMix(idx);
}

void ModelEdit::setNote(int i, QString s)
{
    if(!s.isEmpty())
    {
        mixNotes[i].clear();
        mixNotes[i].append(s);
    }
}

void ModelEdit::mixerAdd()
{
    int index = MixerlistWidget->currentItem()->data(Qt::UserRole).toByteArray().at(0);

    if(index<0)  // if empty then return relavent index
    {
        int i = -index;
        index = getMixerIndex(i); //get mixer index to insert
        if(!gm_insertMix(index))
            return;
        g_model.mixData[index].destCh = i;
    }
    else
    {
        index++;
        if(!gm_insertMix(index))
            return;
        g_model.mixData[index].destCh = g_model.mixData[index-1].destCh;
    }

    gm_openMix(index);

}

void ModelEdit::mixerlistWidget_customContextMenuRequested(QPoint pos)
{
    QPoint globalPos = MixerlistWidget->mapToGlobal(pos);

    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    bool hasData = mimeData->hasFormat("application/x-eepe-mix");

    QMenu contextMenu;
    contextMenu.addAction(QIcon(":/images/add.png"), tr("&Add"),this,SLOT(mixerAdd()),tr("Ctrl+A"));
    contextMenu.addAction(QIcon(":/images/edit.png"), tr("&Edit"),this,SLOT(mixerOpen()),tr("Enter"));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/clear.png"), tr("&Delete"),this,SLOT(mixersDelete()),tr("Delete"));
    contextMenu.addAction(QIcon(":/images/copy.png"), tr("&Copy"),this,SLOT(mixersCopy()),tr("Ctrl+C"));
    contextMenu.addAction(QIcon(":/images/cut.png"), tr("&Cut"),this,SLOT(mixersCut()),tr("Ctrl+X"));
    contextMenu.addAction(QIcon(":/images/paste.png"), tr("&Paste"),this,SLOT(mixersPaste()),tr("Ctrl+V"))->setEnabled(hasData);
    contextMenu.addAction(QIcon(":/images/duplicate.png"), tr("Du&plicate"),this,SLOT(mixersDuplicate()),tr("Ctrl+U"));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/moveup.png"), tr("Move Up"),this,SLOT(moveMixUp()),tr("Ctrl+Up"));
    contextMenu.addAction(QIcon(":/images/movedown.png"), tr("Move Down"),this,SLOT(moveMixDown()),tr("Ctrl+Down"));

    contextMenu.exec(globalPos);
}

void ModelEdit::mixerlistWidget_KeyPress(QKeyEvent *event)
{
    if(event->matches(QKeySequence::SelectAll)) mixerAdd();  //Ctrl A
    if(event->matches(QKeySequence::Delete))    mixersDelete();
    if(event->matches(QKeySequence::Copy))      mixersCopy();
    if(event->matches(QKeySequence::Cut))       mixersCut();
    if(event->matches(QKeySequence::Paste))     mixersPaste();
    if(event->matches(QKeySequence::Underline)) mixersDuplicate();

    if(event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter) mixerOpen();
    if(event->matches(QKeySequence::MoveToNextLine))
        MixerlistWidget->setCurrentRow(MixerlistWidget->currentRow()+1);
    if(event->matches(QKeySequence::MoveToPreviousLine))
        MixerlistWidget->setCurrentRow(MixerlistWidget->currentRow()-1);
}

int ModelEdit::gm_moveMix(int idx, bool dir) //true=inc=down false=dec=up
{
    SKYMixData &src=g_model.mixData[idx];
    if(idx==0 && !dir)
		{
      if (src.destCh>1)
			{
        src.destCh--;
			}
			return idx ;
		}
	
    if(idx>MAX_SKYMIXERS || (idx==MAX_SKYMIXERS && dir)) return idx;

    int tdx = dir ? idx+1 : idx-1;
    SKYMixData &tgt=g_model.mixData[tdx];

    if((src.destCh==0) || (src.destCh>NUM_SKYCHNOUT) || (tgt.destCh>NUM_SKYCHNOUT)) return idx;

    if(tgt.destCh!=src.destCh) {
        if ((dir)  && (src.destCh<NUM_SKYCHNOUT)) src.destCh++;
        if ((!dir) && (src.destCh>0))          src.destCh--;
        return idx;
    }

    //flip between idx and tgt
    SKYMixData temp;
    memcpy(&temp,&src,sizeof(SKYMixData));
    memcpy(&src,&tgt,sizeof(SKYMixData));
    memcpy(&tgt,&temp,sizeof(SKYMixData));

    //do the same for the notes
    QString ix = mixNotes[idx];
    mixNotes[idx].clear();
    mixNotes[idx].append(mixNotes[tdx]);
    mixNotes[tdx].clear();
    mixNotes[tdx].append(ix);


    return tdx;
}

void ModelEdit::moveMixUp()
{
    QList<int> list = createListFromSelected();
    QList<int> highlightList;
    foreach(int idx, list)
        highlightList << gm_moveMix(idx, false);

    updateSettings();
    tabMixes();

    setSelectedByList(highlightList);
}

void ModelEdit::moveMixDown()
{
    QList<int> list = createListFromSelected();
    QList<int> highlightList;
    foreach(int idx, list)
        highlightList << gm_moveMix(idx, true);

    updateSettings();
    tabMixes();

    setSelectedByList(highlightList);
}

void ModelEdit::launchSimulation()
{
    EEGeneral gg;
    memcpy(&gg, &g_eeGeneral,sizeof(gg));

    SKYModelData gm;
    memcpy(&gm, &g_model,sizeof(gm));

    if ( sdptr == 0 )
    {
			if ( SimPointer == 0 )
			{
        sdptr = new simulatorDialog(this) ;
				SimPointer = sdptr ;
			}
			else
			{
				sdptr = SimPointer ;
			}
    }
    sdptr->loadParams(gg,gm, rData->type);
    sdptr->show();
}

void ModelEdit::on_updateButton_clicked()
{
	updateToMV2() ;	
}

void ModelEdit::on_updateButton3_clicked()
{
	updateToMV3() ;	
}

void ModelEdit::on_pushButton_clicked()
{
    launchSimulation();
}

void ModelEdit::on_resetCurve_1_clicked()
{
    memset(&g_model.curves5[0],0,sizeof(g_model.curves5[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_2_clicked()
{
    memset(&g_model.curves5[1],0,sizeof(g_model.curves5[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_3_clicked()
{
    memset(&g_model.curves5[2],0,sizeof(g_model.curves5[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_4_clicked()
{
    memset(&g_model.curves5[3],0,sizeof(g_model.curves5[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_5_clicked()
{
    memset(&g_model.curves5[4],0,sizeof(g_model.curves5[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_6_clicked()
{
    memset(&g_model.curves5[5],0,sizeof(g_model.curves5[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_7_clicked()
{
    memset(&g_model.curves5[6],0,sizeof(g_model.curves5[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_8_clicked()
{
    memset(&g_model.curves5[7],0,sizeof(g_model.curves5[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}




void ModelEdit::on_resetCurve_9_clicked()
{
    memset(&g_model.curves9[0],0,sizeof(g_model.curves9[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_10_clicked()
{
    memset(&g_model.curves9[1],0,sizeof(g_model.curves9[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_11_clicked()
{
    memset(&g_model.curves9[2],0,sizeof(g_model.curves9[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_12_clicked()
{
    memset(&g_model.curves9[3],0,sizeof(g_model.curves9[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_13_clicked()
{
    memset(&g_model.curves9[4],0,sizeof(g_model.curves9[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_14_clicked()
{
    memset(&g_model.curves9[5],0,sizeof(g_model.curves9[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_15_clicked()
{
    memset(&g_model.curves9[6],0,sizeof(g_model.curves9[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_resetCurve_16_clicked()
{
    memset(&g_model.curves9[7],0,sizeof(g_model.curves9[0]));
    updateCurvesTab();
    updateSettings();
    drawCurve();
}

void ModelEdit::on_extendedLimitsChkB_toggled(bool checked)
{
    g_model.extendedLimits = checked;
    setLimitMinMax();
		if ( !checked )
		{
      for( LimitData *ld = &g_model.limitData[0] ; ld < &g_model.limitData[NUM_SKYCHNOUT] ; ld += 1 )
      {
        if (ld->min < 0) ld->min = 0;
        if (ld->max > 0) ld->max = 0;
      }
		}
    updateSettings();
}

void ModelEdit::setLimitMinMax()
{
    int v = g_model.extendedLimits ? 125 : 100;
    ui->minSB_1->setMaximum(v);
    ui->minSB_2->setMaximum(v);
    ui->minSB_3->setMaximum(v);
    ui->minSB_4->setMaximum(v);
    ui->minSB_5->setMaximum(v);
    ui->minSB_6->setMaximum(v);
    ui->minSB_7->setMaximum(v);
    ui->minSB_8->setMaximum(v);
    ui->minSB_9->setMaximum(v);
    ui->minSB_10->setMaximum(v);
    ui->minSB_11->setMaximum(v);
    ui->minSB_12->setMaximum(v);
    ui->minSB_13->setMaximum(v);
    ui->minSB_14->setMaximum(v);
    ui->minSB_15->setMaximum(v);
    ui->minSB_16->setMaximum(v);

    ui->minSB_1->setMinimum(-v);
    ui->minSB_2->setMinimum(-v);
    ui->minSB_3->setMinimum(-v);
    ui->minSB_4->setMinimum(-v);
    ui->minSB_5->setMinimum(-v);
    ui->minSB_6->setMinimum(-v);
    ui->minSB_7->setMinimum(-v);
    ui->minSB_8->setMinimum(-v);
    ui->minSB_9->setMinimum(-v);
    ui->minSB_10->setMinimum(-v);
    ui->minSB_11->setMinimum(-v);
    ui->minSB_12->setMinimum(-v);
    ui->minSB_13->setMinimum(-v);
    ui->minSB_14->setMinimum(-v);
    ui->minSB_15->setMinimum(-v);
    ui->minSB_16->setMinimum(-v);

    ui->maxSB_1->setMaximum(v);
    ui->maxSB_2->setMaximum(v);
    ui->maxSB_3->setMaximum(v);
    ui->maxSB_4->setMaximum(v);
    ui->maxSB_5->setMaximum(v);
    ui->maxSB_6->setMaximum(v);
    ui->maxSB_7->setMaximum(v);
    ui->maxSB_8->setMaximum(v);
    ui->maxSB_9->setMaximum(v);
    ui->maxSB_10->setMaximum(v);
    ui->maxSB_11->setMaximum(v);
    ui->maxSB_12->setMaximum(v);
    ui->maxSB_13->setMaximum(v);
    ui->maxSB_14->setMaximum(v);
    ui->maxSB_15->setMaximum(v);
    ui->maxSB_16->setMaximum(v);

    ui->maxSB_1->setMinimum(-v);
    ui->maxSB_2->setMinimum(-v);
    ui->maxSB_3->setMinimum(-v);
    ui->maxSB_4->setMinimum(-v);
    ui->maxSB_5->setMinimum(-v);
    ui->maxSB_6->setMinimum(-v);
    ui->maxSB_7->setMinimum(-v);
    ui->maxSB_8->setMinimum(-v);
    ui->maxSB_9->setMinimum(-v);
    ui->maxSB_10->setMinimum(-v);
    ui->maxSB_11->setMinimum(-v);
    ui->maxSB_12->setMinimum(-v);
    ui->maxSB_13->setMinimum(-v);
    ui->maxSB_14->setMinimum(-v);
    ui->maxSB_15->setMinimum(-v);
    ui->maxSB_16->setMinimum(-v);
}



void ModelEdit::on_templateList_doubleClicked(QModelIndex index)
{
    QString text = ui->templateList->item(index.row())->text();

		if ( index.row() == 9 )
		{
			templateValues.stick = STK_RUD ;
			templateValues.outputChannel = 8 ;
			templateValues.helperChannel = 16 ;
			templateValues.switch1 = 1 ;
			templateValues.switch2 = 2 ;
			templateValues.switch3 = 3 ;
      TemplateDialog *tem = new TemplateDialog(this, &g_model, &templateValues, rData->type );
			if(tem->exec())
    	{
    		applyTemplate(index.row());
    		updateSettings();
		    tabMixes();
    	}
			return ;
		}
		else
		{
	    int res = QMessageBox::question(this,tr("Apply Template?"),tr("Apply template \"%1\"?").arg(text),QMessageBox::Yes | QMessageBox::No);
  	  if(res!=QMessageBox::Yes) return;
		}

    applyTemplate(index.row());
    updateSettings();
    tabMixes();

}


SKYMixData* ModelEdit::setDest(uint8_t dch)
{
    uint8_t i = 0;
    while ((g_model.mixData[i].destCh<=dch) && (g_model.mixData[i].destCh) && (i<MAX_SKYMIXERS)) i++;
    if(i==MAX_SKYMIXERS) return &g_model.mixData[0];

    memmove(&g_model.mixData[i+1],&g_model.mixData[i],
            (MAX_SKYMIXERS-(i+1))*sizeof(SKYMixData) );
    memset(&g_model.mixData[i],0,sizeof(SKYMixData));
    g_model.mixData[i].destCh = dch;
		g_model.mixData[i].weight = 100 ;
		g_model.mixData[i].lateOffset = 1 ;
    return &g_model.mixData[i];
}

void ModelEdit::clearMixes(bool ask)
{
    if(ask)
    {
        int res = QMessageBox::question(this,tr("Clear Mixes?"),tr("Really clear all the mixes?"),QMessageBox::Yes | QMessageBox::No);
        if(res!=QMessageBox::Yes) return;
    }
    memset(g_model.mixData,0,sizeof(g_model.mixData)); //clear all mixes
    updateSettings();
    tabMixes();
}

void ModelEdit::clearCurves(bool ask)
{
    if(ask)
    {
        int res = QMessageBox::question(this,tr("Clear Curves?"),tr("Really clear all the curves?"),QMessageBox::Yes | QMessageBox::No);
        if(res!=QMessageBox::Yes) return;
    }
    memset(g_model.curves5,0,sizeof(g_model.curves5)); //clear all curves
    memset(g_model.curves9,0,sizeof(g_model.curves9)); //clear all curves
    updateSettings();
    updateCurvesTab();
    resizeEvent();
}

void ModelEdit::setCurve(uint8_t c, int8_t ar[])
{
    if(c<MAX_CURVE5) //5 pt curve
        for(uint8_t i=0; i<5; i++) g_model.curves5[c][i] = ar[i];
    else  //9 pt curve
        for(uint8_t i=0; i<9; i++) g_model.curves9[c-MAX_CURVE5][i] = ar[i];
}

void ModelEdit::setSwitch(uint8_t idx, uint8_t func, int8_t v1, int8_t v2)
{
    g_model.customSw[idx-1].func = func;
  	g_model.customSw[idx-1].andsw = 0 ;
    g_model.customSw[idx-1].v1   = v1;
    g_model.customSw[idx-1].v2   = v2;
}

void ModelEdit::applyTemplate(uint8_t idx)
{
    int8_t heli_ar1[] = {-100, -20, 30, 70, 90};
    int8_t heli_ar2[] = {80, 70, 60, 70, 100};
    int8_t heli_ar3[] = {100, 90, 80, 90, 100};
    int8_t heli_ar4[] = {-30,  -15, 0, 50, 100};
    int8_t heli_ar5[] = {-100, -50, 0, 50, 100};


    SKYMixData *md = &g_model.mixData[0];

    //CC(STK)   -> vSTK
    //ICC(vSTK) -> STK
#define ICC(x) icc[(x)-1]
    uint8_t icc[4] = {0};
    for(uint8_t i=1; i<=4; i++) //generate inverse array
        for(uint8_t j=1; j<=4; j++) if(CC(i)==j) icc[j-1]=i;


    uint8_t j = 0;


    //Simple 4-Ch
    if(idx==j++)
    {
        if (md->destCh)
        {
          clearMixes();
        }
        md=setDest(ICC(STK_RUD));  md->srcRaw=CM(STK_RUD,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=100;
        md=setDest(ICC(STK_ELE));  md->srcRaw=CM(STK_ELE,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=100;
        md=setDest(ICC(STK_THR));  md->srcRaw=CM(STK_THR,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=100;
        md=setDest(ICC(STK_AIL));  md->srcRaw=CM(STK_AIL,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=100;
    }

    //T-Cut
    if(idx==j++)
    {
      SKYSafetySwData *sd = &g_model.safetySw[ICC(STK_THR)-1] ;
			sd->opt.ss.mode = 0 ;
			sd->opt.ss.swtch = DSW_THR ;
			sd->opt.ss.val = g_model.throttleIdle ? 0 : -100 ;
    }

    //sticky t-cut
    if(idx==j++)
    {
//        md=setDest(ICC(STK_THR));  md->srcRaw=MIX_MAX;  md->weight=-100;  md->swtch=DSW_SWC;  md->mltpx=MLTPX_REP;
//        md=setDest(14);            md->srcRaw=CH(14);   md->weight= 100;
//        md=setDest(14);            md->srcRaw=MIX_MAX;  md->weight=-100;  md->swtch=DSW_SWB;  md->mltpx=MLTPX_REP;
//        md=setDest(14);            md->srcRaw=MIX_MAX;  md->weight= 100;  md->swtch=DSW_THR;  md->mltpx=MLTPX_REP;

//        setSwitch(0xB,CS_VNEG, CM(STK_THR,g_model.modelVersion,g_eeGeneral.stickMode), -99);
//        setSwitch(0xC,CS_VPOS, CH(14), 0);

      SKYSafetySwData *sd = &g_model.safetySw[ICC(STK_THR)-1] ;
			sd->opt.ss.mode = 3 ;
			sd->opt.ss.swtch = DSW_THR ;
			sd->opt.ss.val = g_model.throttleIdle ? 0 : -100 ;
			
			EditedNesting = 1  ;
      populateSafetySwitchCB(safetySwitchSwtch[ICC(STK_THR)-1],sd->opt.ss.mode,sd->opt.ss.swtch, rData->type);
			safetySwitchType[ICC(STK_THR)-1]->setCurrentIndex( sd->opt.ss.mode ) ;
			safetySwitchValue[ICC(STK_THR)-1]->setValue( sd->opt.ss.val ) ;
			setSafetyWidgetVisibility(ICC(STK_THR)-1) ;
			EditedNesting = 0  ;
    }

    //V-Tail
    if(idx==j++)
    {
        clearMixes();
        md=setDest(ICC(STK_THR));  md->srcRaw=CM(STK_THR,g_model.modelVersion,g_eeGeneral.stickMode);
        md=setDest(ICC(STK_AIL));  md->srcRaw=CM(STK_AIL,g_model.modelVersion,g_eeGeneral.stickMode);
        md=setDest(ICC(STK_RUD));  md->srcRaw=CM(STK_RUD,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 50;
        md=setDest(ICC(STK_RUD));  md->srcRaw=CM(STK_ELE,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=-50;
        md=setDest(ICC(STK_ELE));  md->srcRaw=CM(STK_RUD,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 50;
        md=setDest(ICC(STK_ELE));  md->srcRaw=CM(STK_ELE,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 50;
    }

    //Elevon\\Delta
    if(idx==j++)
    {
        clearMixes();
        md=setDest(ICC(STK_RUD));  md->srcRaw=CM(STK_RUD,g_model.modelVersion,g_eeGeneral.stickMode);
        md=setDest(ICC(STK_THR));  md->srcRaw=CM(STK_THR,g_model.modelVersion,g_eeGeneral.stickMode);
        md=setDest(ICC(STK_ELE));  md->srcRaw=CM(STK_ELE,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 50;
        md=setDest(ICC(STK_ELE));  md->srcRaw=CM(STK_AIL,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 50;
        md=setDest(ICC(STK_AIL));  md->srcRaw=CM(STK_ELE,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 50;
        md=setDest(ICC(STK_AIL));  md->srcRaw=CM(STK_AIL,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=-50;
    }


    //Heli Setup
    if(idx==j++)
    {
        clearMixes();  //This time we want a clean slate
        clearCurves();

        //Set up Mixes
        //3 cyclic channels
        md=setDest(1);  md->srcRaw=MIX_CYC1;  md->weight= 100;
        md=setDest(2);  md->srcRaw=MIX_CYC2;  md->weight= 100;
        md=setDest(3);  md->srcRaw=MIX_CYC3;  md->weight= 100;

        //rudder
        md=setDest(4);  md->srcRaw=CM(STK_RUD,g_model.modelVersion,g_eeGeneral.stickMode); md->weight=100;

        //Throttle
        md=setDest(5);  md->srcRaw=CM(STK_THR,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 100; md->swtch= DSW_ID0; md->curve=CV(1); md->carryTrim=TRIM_OFF;
        md=setDest(5);  md->srcRaw=CM(STK_THR,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 100; md->swtch= DSW_ID1; md->curve=CV(2); md->carryTrim=TRIM_OFF;
        md=setDest(5);  md->srcRaw=CM(STK_THR,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 100; md->swtch= DSW_ID2; md->curve=CV(3); md->carryTrim=TRIM_OFF;
        md=setDest(5);  md->srcRaw=MIX_MAX;      md->weight=-100; md->swtch= DSW_THR; md->mltpx=MLTPX_REP;

        //gyro gain
        md=setDest(6);  md->srcRaw=MIX_FULL; md->weight=30; md->swtch=-DSW_GEA;

        //collective
        md=setDest(11); md->srcRaw=CM(STK_THR,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=100; md->swtch= DSW_ID0; md->curve=CV(4); md->carryTrim=TRIM_OFF;
        md=setDest(11); md->srcRaw=CM(STK_THR,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=100; md->swtch= DSW_ID1; md->curve=CV(5); md->carryTrim=TRIM_OFF;
        md=setDest(11); md->srcRaw=CM(STK_THR,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=100; md->swtch= DSW_ID2; md->curve=CV(6); md->carryTrim=TRIM_OFF;

        g_model.swashType = SWASH_TYPE_120;
        g_model.swashCollectiveSource = CH(11);

        //Set up Curves
        setCurve(CURVE5(1),heli_ar1);
        setCurve(CURVE5(2),heli_ar2);
        setCurve(CURVE5(3),heli_ar3);
        setCurve(CURVE5(4),heli_ar4);
        setCurve(CURVE5(5),heli_ar5);
        setCurve(CURVE5(6),heli_ar5);

        // make sure curves are redrawn
        updateHeliTab();
        updateCurvesTab();
        resizeEvent();
    }

    //Gyro Gain
    if(idx==j++)
    {
        md=setDest(6);  md->srcRaw=STK_P2; md->weight= 50; md->swtch=-DSW_GEA; md->sOffset=100;
        md=setDest(6);  md->srcRaw=STK_P2; md->weight=-50; md->swtch= DSW_GEA; md->sOffset=100;
    }

    //Servo Test
    if(idx==j++)
    {
        md=setDest(15); md->srcRaw=CH(16);   md->weight= 100; md->speedUp = 8; md->speedDown = 8;
        md=setDest(16); md->srcRaw=MIX_FULL; md->weight= 110; md->swtch=DSW_SW1;
        md=setDest(16); md->srcRaw=MIX_MAX;  md->weight=-110; md->swtch=DSW_SW2; md->mltpx=MLTPX_REP;
        md=setDest(16); md->srcRaw=MIX_MAX;  md->weight= 110; md->swtch=DSW_SW3; md->mltpx=MLTPX_REP;

        setSwitch(1,CS_LESS,CH(15), CH(16));
        setSwitch(2,CS_VPOS,CH(15), 105);
        setSwitch(3,CS_VNEG,CH(15),-105);

        // redraw switches tab
        updateSwitchesTab();
    }

    // Range Test
    if(idx==j++)
    {
        md=setDest(16); md->srcRaw=MIX_FULL; md->weight= 100; md->swtch=DSW_SW1; md->speedUp = 4; md->speedDown = 4;

        setSwitch(1,CS_TIME, 4, 4 ) ;

        // redraw switches tab
        updateSwitchesTab();
    }
    
		// Progressive
    if(idx==j++)
    {
			md=setDest(templateValues.helperChannel); md->srcRaw=CM(templateValues.stick,g_model.modelVersion,g_eeGeneral.stickMode); md->weight= 20;
			md=setDest(templateValues.outputChannel); md->srcRaw=CH(templateValues.outputChannel); md->weight= 100;
			md=setDest(templateValues.outputChannel); md->srcRaw=CH(templateValues.helperChannel); md->weight= 2;
			md=setDest(templateValues.outputChannel); md->srcRaw=MIX_FULL; md->weight= 100; md->swtch=DSW_SW1-1+templateValues.switch2; md->mltpx=MLTPX_REP;
			md=setDest(templateValues.outputChannel); md->srcRaw=MIX_FULL; md->weight= -100; md->swtch=DSW_SW1-1+templateValues.switch3; md->mltpx=MLTPX_REP;

      setSwitch(templateValues.switch1,CS_APOS, CM(templateValues.stick,g_model.modelVersion,g_eeGeneral.stickMode), 1);
      setSwitch(templateValues.switch2,CS_VPOS, CH(templateValues.outputChannel), 100);
      setSwitch(templateValues.switch3,CS_VNEG, CH(templateValues.outputChannel), -100);
      updateSwitchesTab();
    }

}




void ModelEdit::on_ppmFrameLengthDSB_editingFinished()
{
    if(protocolEditLock) return;
    g_model.ppmFrameLength = (ui->ppmFrameLengthDSB->value()-22.5)/0.5;
    updateSettings();
}

void ModelEdit::on_xppmFrameLengthDSB_editingFinished()
{
    if(protocolEditLock) return;
    g_model.xppmFrameLength = (ui->xppmFrameLengthDSB->value()-22.5)/0.5;
    updateSettings();
}

void ModelEdit::on_plotCB_1_toggled(bool checked)
{
    plot_curve[0] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_2_toggled(bool checked)
{
    plot_curve[1] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_3_toggled(bool checked)
{
    plot_curve[2] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_4_toggled(bool checked)
{
    plot_curve[3] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_5_toggled(bool checked)
{
    plot_curve[4] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_6_toggled(bool checked)
{
    plot_curve[5] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_7_toggled(bool checked)
{
    plot_curve[6] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_8_toggled(bool checked)
{
    plot_curve[7] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_9_toggled(bool checked)
{
    plot_curve[8] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_10_toggled(bool checked)
{
    plot_curve[9] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_11_toggled(bool checked)
{
    plot_curve[10] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_12_toggled(bool checked)
{
    plot_curve[11] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_13_toggled(bool checked)
{
    plot_curve[12] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_14_toggled(bool checked)
{
    plot_curve[13] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_15_toggled(bool checked)
{
    plot_curve[14] = checked;
    drawCurve();
}

void ModelEdit::on_plotCB_16_toggled(bool checked)
{
    plot_curve[15] = checked;
    drawCurve();
}

void ModelEdit::ControlCurveSignal(bool flag)
{
  ui->curvePt1_1->blockSignals(flag);
  ui->curvePt2_1->blockSignals(flag);
  ui->curvePt3_1->blockSignals(flag);
  ui->curvePt4_1->blockSignals(flag);
  ui->curvePt5_1->blockSignals(flag);
  ui->curvePt1_2->blockSignals(flag);
  ui->curvePt2_2->blockSignals(flag);
  ui->curvePt3_2->blockSignals(flag);
  ui->curvePt4_2->blockSignals(flag);
  ui->curvePt5_2->blockSignals(flag);
  ui->curvePt1_3->blockSignals(flag);
  ui->curvePt2_3->blockSignals(flag);
  ui->curvePt3_3->blockSignals(flag);
  ui->curvePt4_3->blockSignals(flag);
  ui->curvePt5_3->blockSignals(flag);
  ui->curvePt1_4->blockSignals(flag);
  ui->curvePt2_4->blockSignals(flag);
  ui->curvePt3_4->blockSignals(flag);
  ui->curvePt4_4->blockSignals(flag);
  ui->curvePt5_4->blockSignals(flag);
  ui->curvePt1_5->blockSignals(flag);
  ui->curvePt2_5->blockSignals(flag);
  ui->curvePt3_5->blockSignals(flag);
  ui->curvePt4_5->blockSignals(flag);
  ui->curvePt5_5->blockSignals(flag);
  ui->curvePt1_6->blockSignals(flag);
  ui->curvePt2_6->blockSignals(flag);
  ui->curvePt3_6->blockSignals(flag);
  ui->curvePt4_6->blockSignals(flag);
  ui->curvePt5_6->blockSignals(flag);
  ui->curvePt1_7->blockSignals(flag);
  ui->curvePt2_7->blockSignals(flag);
  ui->curvePt3_7->blockSignals(flag);
  ui->curvePt4_7->blockSignals(flag);
  ui->curvePt5_7->blockSignals(flag);
  ui->curvePt1_8->blockSignals(flag);
  ui->curvePt2_8->blockSignals(flag);
  ui->curvePt3_8->blockSignals(flag);
  ui->curvePt4_8->blockSignals(flag);
  ui->curvePt5_8->blockSignals(flag);
  ui->curvePt1_9->blockSignals(flag);
  ui->curvePt2_9->blockSignals(flag);
  ui->curvePt3_9->blockSignals(flag);
  ui->curvePt4_9->blockSignals(flag);
  ui->curvePt5_9->blockSignals(flag);
  ui->curvePt6_9->blockSignals(flag);
  ui->curvePt7_9->blockSignals(flag);
  ui->curvePt8_9->blockSignals(flag);
  ui->curvePt9_9->blockSignals(flag);
  ui->curvePt1_10->blockSignals(flag);
  ui->curvePt2_10->blockSignals(flag);
  ui->curvePt3_10->blockSignals(flag);
  ui->curvePt4_10->blockSignals(flag);
  ui->curvePt5_10->blockSignals(flag);
  ui->curvePt6_10->blockSignals(flag);
  ui->curvePt7_10->blockSignals(flag);
  ui->curvePt8_10->blockSignals(flag);
  ui->curvePt9_10->blockSignals(flag);
  ui->curvePt1_11->blockSignals(flag);
  ui->curvePt2_11->blockSignals(flag);
  ui->curvePt3_11->blockSignals(flag);
  ui->curvePt4_11->blockSignals(flag);
  ui->curvePt5_11->blockSignals(flag);
  ui->curvePt6_11->blockSignals(flag);
  ui->curvePt7_11->blockSignals(flag);
  ui->curvePt8_11->blockSignals(flag);
  ui->curvePt9_11->blockSignals(flag);
  ui->curvePt1_12->blockSignals(flag);
  ui->curvePt2_12->blockSignals(flag);
  ui->curvePt3_12->blockSignals(flag);
  ui->curvePt4_12->blockSignals(flag);
  ui->curvePt5_12->blockSignals(flag);
  ui->curvePt6_12->blockSignals(flag);
  ui->curvePt7_12->blockSignals(flag);
  ui->curvePt8_12->blockSignals(flag);
  ui->curvePt9_12->blockSignals(flag);
  ui->curvePt1_13->blockSignals(flag);
  ui->curvePt2_13->blockSignals(flag);
  ui->curvePt3_13->blockSignals(flag);
  ui->curvePt4_13->blockSignals(flag);
  ui->curvePt5_13->blockSignals(flag);
  ui->curvePt6_13->blockSignals(flag);
  ui->curvePt7_13->blockSignals(flag);
  ui->curvePt8_13->blockSignals(flag);
  ui->curvePt9_13->blockSignals(flag);
  ui->curvePt1_14->blockSignals(flag);
  ui->curvePt2_14->blockSignals(flag);
  ui->curvePt3_14->blockSignals(flag);
  ui->curvePt4_14->blockSignals(flag);
  ui->curvePt5_14->blockSignals(flag);
  ui->curvePt6_14->blockSignals(flag);
  ui->curvePt7_14->blockSignals(flag);
  ui->curvePt8_14->blockSignals(flag);
  ui->curvePt9_14->blockSignals(flag);
  ui->curvePt1_15->blockSignals(flag);
  ui->curvePt2_15->blockSignals(flag);
  ui->curvePt3_15->blockSignals(flag);
  ui->curvePt4_15->blockSignals(flag);
  ui->curvePt5_15->blockSignals(flag);
  ui->curvePt6_15->blockSignals(flag);
  ui->curvePt7_15->blockSignals(flag);
  ui->curvePt8_15->blockSignals(flag);
  ui->curvePt9_15->blockSignals(flag);
  ui->curvePt1_16->blockSignals(flag);
  ui->curvePt2_16->blockSignals(flag);
  ui->curvePt3_16->blockSignals(flag);
  ui->curvePt4_16->blockSignals(flag);
  ui->curvePt5_16->blockSignals(flag);
  ui->curvePt6_16->blockSignals(flag);
  ui->curvePt7_16->blockSignals(flag);
  ui->curvePt8_16->blockSignals(flag);
  ui->curvePt9_16->blockSignals(flag);
}

void ModelEdit::on_CustomAlarmSourceCB_currentIndexChanged(int index)
{
	if ( customAlarmLock )
	{
		return ;
	}
	g_model.customCheck.source = index ;
  updateSettings();
}

void ModelEdit::on_CustomAlarmMinSB_editingFinished()
{
	if ( customAlarmLock )
	{
		return ;
	}
	g_model.customCheck.min	= ui->CustomAlarmMinSB->value() ;
  updateSettings();
}
	
void ModelEdit::on_CustomAlarmMaxSB_editingFinished()
{
	if ( customAlarmLock )
	{
		return ;
	}
	g_model.customCheck.max = ui->CustomAlarmMaxSB->value() ;
  updateSettings();
}

void ModelEdit::on_Com2BaudrateCB_currentIndexChanged(int index)
{
	g_model.com2Baudrate = index ;
  updateSettings() ;
}

void ModelEdit::on_AutoBtConnectChkB_stateChanged(int )
{
	g_model.autoBtConnect = ui->AutoBtConnectChkB->isChecked() ;
  updateSettings() ;
}

void ModelEdit::on_UseStickNamesChkB_stateChanged(int )
{
	g_model.useCustomStickNames = ui->UseStickNamesChkB->isChecked() ;
  updateSettings() ;
}

void ModelEdit::on_BtDefaultAddrSB_editingFinished()
{
	g_model.btDefaultAddress = ui->BtDefaultAddrSB->value() ;
  updateSettings();
}


