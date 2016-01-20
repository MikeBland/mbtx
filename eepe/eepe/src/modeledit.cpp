#include "modeledit.h"
#include "ui_modeledit.h"
#include "pers.h"
#include "helpers.h"
#include "../../common/edge.h"
#include "../../common/node.h"
#include "mixerdialog.h"
#include "simulatordialog.h"
#include "VoiceAlarmDialog.h"
#include "TemplateDialog.h"

#include <QtGui>
#include <QMessageBox>
#include <QMenu>
#include <QTextBrowser>


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
ModelData Sim_m ;
int ModelDataValid = 0 ;

ModelEdit::ModelEdit(EEPFILE *eFile, uint8_t id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModelEdit)
{
		unsigned int size ;
    ui->setupUi(this);

    eeFile = eFile;
    sdptr = 0;

    switchEditLock = false;
    heliEditLock = false;
    protocolEditLock = false;
    switchDefPosEditLock = false;

    if(!eeFile->eeLoadGeneral())  eeFile->generalDefault();
    eeFile->getGeneralSettings(&g_eeGeneral);
    size = eeFile->getModel(&g_model,id);
		if ( size < sizeof(g_model) )
		{
			uint8_t *p ;
			p = (uint8_t *) &g_model + size ;
			while( size < sizeof(g_model) )
			{
				*p++ = 0 ;
				size += 1 ;
			}
		}
    id_model = id;
#ifndef V2
    if ( g_model.numBlades == 0 )
		{
			g_model.numBlades = g_model.xnumBlades + 2 ;				
		}
#endif

		createSwitchMapping( &g_eeGeneral, eeFile->mee_type ) ;
    setupMixerListWidget();

    QSettings settings("er9x-eePe", "eePe");
    ui->tabWidget->setCurrentIndex(settings.value("modelEditTab", 0).toInt());

    QRegExp rx(CHAR_FOR_NAMES_REGEX);
    ui->modelNameLE->setValidator(new QRegExpValidator(rx, this));
		
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
    eeFile->putModel(&g_model,id_model);
    emit modelValuesChanged(this);
		
    memcpy(&Sim_g, &g_eeGeneral,sizeof(EEGeneral));
    memcpy(&Sim_m,&g_model,sizeof(ModelData));
		GeneralDataValid = 1 ;
		ModelDataValid = 1 ;
		GlobalModified = 1 ;
}

void ModelEdit::on_tabWidget_currentChanged(int index)
{
    QSettings settings("er9x-eePe", "eePe");
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

    ui->VoiceNumberSB->setValue(g_model.modelVoice+260) ;
#ifndef V2
    //timer mode direction value
    populateTimerSwitchCB(ui->timerModeCB,g_model.tmrMode,eeFile->mee_type);
    populateTmrBSwitchCB(ui->timerModeBCB,g_model.tmrModeB,eeFile->mee_type);
    int min = g_model.tmrVal/60;
    int sec = g_model.tmrVal%60;
    ui->timerValTE->setTime(QTime(0,min,sec));
    ui->timerDirCB->setCurrentIndex(g_model.tmrDir);
    populateTmrBSwitchCB(ui->timerResetCB,g_model.timer1RstSw,eeFile->mee_type);

    populateTimerSwitchCB(ui->timer2ModeCB,g_model.tmr2Mode,eeFile->mee_type);
    populateTmrBSwitchCB(ui->timer2ModeBCB,g_model.tmr2ModeB,eeFile->mee_type);
    min = g_model.tmr2Val/60;
    sec = g_model.tmr2Val%60;
    ui->timer2ValTE->setTime(QTime(0,min,sec));
    ui->timer2DirCB->setCurrentIndex(g_model.tmr2Dir);
    populateTmrBSwitchCB(ui->timer2ResetCB,g_model.timer2RstSw,eeFile->mee_type);
#else
    populateTimerSwitchCB(ui->timerModeCB,g_model.timer[0].tmrModeA,eeFile->mee_type);
    populateTmrBSwitchCB(ui->timerModeBCB,g_model.timer[0].tmrModeB,eeFile->mee_type);
    int min = g_model.timer[0].tmrVal/60;
    int sec = g_model.timer[0].tmrVal%60;
    ui->timerValTE->setTime(QTime(0,min,sec));
    ui->timerDirCB->setCurrentIndex(g_model.timer[0].tmrDir);
    populateTmrBSwitchCB(ui->timerResetCB,g_model.timer[0].tmrRstSw,eeFile->mee_type);

    populateTimerSwitchCB(ui->timer2ModeCB,g_model.timer[1].tmrModeA,eeFile->mee_type);
    populateTmrBSwitchCB(ui->timer2ModeBCB,g_model.timer[1].tmrModeB,eeFile->mee_type);
    min = g_model.timer[1].tmrVal/60;
    sec = g_model.timer[1].tmrVal%60;
    ui->timer2ValTE->setTime(QTime(0,min,sec));
    ui->timer2DirCB->setCurrentIndex(g_model.timer[1].tmrDir);
    populateTmrBSwitchCB(ui->timer2ResetCB,g_model.timer[1].tmrRstSw,eeFile->mee_type);
#endif

    //trim inc, thro trim, thro expo, instatrim
    ui->trimIncCB->setCurrentIndex(g_model.trimInc);
    populateSwitchCB(ui->trimSWCB,g_model.trimSw,eeFile->mee_type);
    ui->thrExpoChkB->setChecked(g_model.thrExpo);
    ui->thrTrimChkB->setChecked(g_model.thrTrim);
	  ui->throttleOffCB->setCurrentIndex(g_model.throttleIdle) ;

    ui->TrainerChkB->setChecked(g_model.traineron);
//    ui->T2ThrTrgChkB->setChecked(g_model.t2throttle);

    ui->throttleReversedChkB->setChecked(g_model.throttleReversed);

    //center beep
    ui->bcRUDChkB->setChecked(g_model.beepANACenter & BC_BIT_RUD);
    ui->bcELEChkB->setChecked(g_model.beepANACenter & BC_BIT_ELE);
    ui->bcTHRChkB->setChecked(g_model.beepANACenter & BC_BIT_THR);
    ui->bcAILChkB->setChecked(g_model.beepANACenter & BC_BIT_AIL);
    ui->bcP1ChkB->setChecked(g_model.beepANACenter & BC_BIT_P1);
    ui->bcP2ChkB->setChecked(g_model.beepANACenter & BC_BIT_P2);
    ui->bcP3ChkB->setChecked(g_model.beepANACenter & BC_BIT_P3);

    ui->extendedLimitsChkB->setChecked(g_model.extendedLimits);
#ifndef V2
    ui->fastMixDelayCB->setChecked(g_model.mixTime) ;
#endif

    //pulse polarity
    ui->pulsePolCB->setCurrentIndex(g_model.pulsePol);
    ui->autoLimitsSB->setValue( (double)g_model.sub_trim_limit/10 + 0.049 ) ;

    //protocol channels ppm delay (disable if needed)
    setProtocolBoxes();

		populateAnaVolumeCB( ui->volumeControlCB, g_model.anaVolume ) ;
	  ui->countryCB->setCurrentIndex(g_model.country) ;
	  ui->typeCB->setCurrentIndex(g_model.sub_protocol) ;
		ui->label_version->setText( tr("%1").arg( g_model.modelVersion ) ) ;
		ui->updateButton->setVisible( g_model.modelVersion < 2 ) ;
		ui->updateButton3->setVisible( g_model.modelVersion < 3 ) ;
    
		ui->useCustomStickNamesChkb->setChecked(g_model.useCustomStickNames) ;

//    ui->switchwarnChkB->setChecked(!(g_model.modelswitchWarningStates&1)); //Default is zero=checked

    setSwitchDefPos() ;

		ui->switchwarnChkB->hide() ;
		ui->widgetDefSA->hide() ;
		ui->widgetDefSB->hide() ;
		ui->widgetDefSC->hide() ;
		ui->widgetDefSD->hide() ;
		ui->widgetDefSE->hide() ;
		ui->widgetDefSF->hide() ;
		ui->widgetDefSG->hide() ;
		 

}

uint16_t ModelEdit::oneSwitchPos( uint8_t swtch, uint16_t states )
{
	uint8_t index = 0 ;
//	uint8_t sm = g_eeGeneral.switchMapping ;

	switch ( swtch )
	{
//		case HSW_ThrCt :
//			if ( sm & USE_THR_3POS )
//			{
//				if ( states & 0x0001 ) index += 1 ;
//				if ( states & 0x0100 ) index += 2 ;
//			}
//			else
//			{
//				if (states & 0x0101) index = 1 ;
//			}
//		break ;

//		case HSW_RuddDR :
//			if ( sm & USE_RUD_3POS )
//			{
//				if ( states & 0x0002 ) index += 1 ;
//				if ( states & 0x0200 ) index += 2 ;
//			}
//			else
//			{
//				if (states & 0x0202) index = 1 ;
//			}
//		break ;
	
//		case HSW_ElevDR :
//			if ( sm & USE_ELE_3POS )
//			{
//				if ( states & 0x0004 ) index += 1 ;
//				if ( states & 0x0400 ) index += 2 ;
//			}
//			else
//			{
//				if (states & 0x0C04) index = 1 ;
//			}
//		break ;

//		case HSW_ID0 :
//			if ( states & 0x10 )
//			{
//				index += 1 ;
//			}
//			if ( states & 0x20 )
//			{
//				index += 2 ;
//			}
//		break ;

//		case HSW_AileDR :
//			if ( sm & USE_AIL_3POS )
//			{
//				if ( states & 0x0040 ) index += 1 ;
//				if ( states & 0x1000 ) index += 2 ;
//			}
//			else
//			{
//				if (states & 0x1040) index = 1 ;
//			}
//		break ;
	 
//		case HSW_Gear :
//			if ( sm & USE_GEA_3POS )
//			{
//				if ( states & 0x0080 ) index += 1 ;
//				if ( states & 0x2000 ) index += 2 ;
//			}
//			else
//			{
//				if (states & 0x2080) index = 1 ;
//			}
//		break ;

	}
	return index ;
}


void ModelEdit::setSwitchDefPos()
{
//	if ( rData->type == 0 )
  {
		
    quint16 y = (g_model.switchWarningStates >> 1 ) ;
    quint16 x = y & SWP_IL5 ;
    if(x==SWP_IL1 || x==SWP_IL2 || x==SWP_IL3 || x==SWP_IL4 || x==SWP_IL5) //illegal states for ID0/1/2
    {
      x &= ~SWP_IL5; // turn all off, make sure only one is on
      x |=  SWP_ID0B;
    }

    y &= ~SWP_IL5 ;
		x |= y ;
		 
//		g_model.modelswitchWarningStates = ( x << 1 ) | (g_model.switchWarningStates & 1 ) ;

    switchDefPosEditLock = true;
//		if ( g_eeGeneral.switchMapping & USE_THR_3POS )
//		{
//    	ui->SwitchDefSA->setValue( oneSwitchPos( HSW_ThrCt, x ) ) ;
//		}
//		else
//		{
    	ui->switchDefPos_1->setChecked(x & 0x01);
//		}
		
//		if ( g_eeGeneral.switchMapping & USE_RUD_3POS )
//		{
//    	ui->SwitchDefSB->setValue( oneSwitchPos( HSW_RuddDR, x ) ) ;
//		}
//		else
//		{
	    ui->switchDefPos_2->setChecked(x & 0x02);
//		}

//		if ( g_eeGeneral.switchMapping & USE_ELE_3POS )
//		{
//    	ui->SwitchDefSC->setValue( oneSwitchPos( HSW_ElevDR, x ) ) ;
//		}
//		else
//		{
    	ui->switchDefPos_3->setChecked(x & 0x04);
//		}
    ui->switchDefPos_4->setChecked(x & 0x08);
    ui->switchDefPos_5->setChecked(x & 0x10);
    ui->switchDefPos_6->setChecked(x & 0x20);
    
//		if ( g_eeGeneral.switchMapping & USE_AIL_3POS )
//		{
//    	ui->SwitchDefSD->setValue( oneSwitchPos( HSW_AileDR, x ) ) ;
//		}
//		else
//		{
			ui->switchDefPos_7->setChecked(x & 0x40);
//		}
		
//		if ( g_eeGeneral.switchMapping & USE_GEA_3POS )
//		{
//      ui->SwitchDefSE->setValue( oneSwitchPos( HSW_Gear, x ) ) ;
//		}
//		else
//		{
    	ui->switchDefPos_8->setChecked(x & 0x80);
//		}

		switchDefPosEditLock = false;
	}
//	else
//	{
//    quint16 y = (g_model.modelswitchWarningStates >> 1 ) ;
//    switchDefPosEditLock = true;
//    ui->SwitchDefSA->setValue(y & 0x03) ;
//		y >>= 2 ;
//    ui->SwitchDefSB->setValue(y & 0x03) ;
//		y >>= 2 ;
//    ui->SwitchDefSC->setValue(y & 0x03) ;
//		y >>= 2 ;
//    ui->SwitchDefSD->setValue(y & 0x03) ;
//		y >>= 2 ;
//    ui->SwitchDefSE->setValue(y & 0x03) ;
//		y >>= 2 ;
//    ui->SwitchDefSF->setValue( (y & 0x03) ? 1 : 0 ) ;
//		y >>= 2 ;
//    ui->SwitchDefSG->setValue(y & 0x03) ;
//    switchDefPosEditLock = false;
//	}

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
    for(uint8_t i=0;i<MAX_MIXERS;i++)
		{
      MixData *md = &g_model.mixData[i] ;
      if (md->srcRaw)
			{
        if (md->srcRaw <= 4)		// Stick
				{
					md->srcRaw = stickScramble[g_eeGeneral.stickMode*4+md->srcRaw-1] + 1 ;
				}
			}
		}
#ifndef V2
    for (uint8_t i = 0 ; i < NUM_CSW ; i += 1 )
		{
    	CSwData *cs = &g_model.customSw[i];
      uint8_t cstate = CS_STATE(cs->func, g_model.modelVersion);
    	if(cstate == CS_VOFS)
			{
      	if (cs->v1)
				{
    		  if (cs->v1 <= 4)		// Stick
					{
    	    	cs->v1 = stickScramble[g_eeGeneral.stickMode*4+cs->v1-1] + 1 ;
					}
				}
			}
			else if(cstate == CS_VCOMP)
			{
      	if (cs->v1)
				{
    		  if (cs->v1 <= 4)		// Stick
					{
		    	  cs->v1 = stickScramble[g_eeGeneral.stickMode*4+cs->v1-1] + 1 ;
    	    }
				}
      	if (cs->v2)
				{
    		  if (cs->v2 <= 4)		// Stick
					{
						cs->v2 = stickScramble[g_eeGeneral.stickMode*4+cs->v2-1] + 1 ;
				  }
				}
			}
		}
#endif

#ifndef V2
    if ( eeFile->mee_type )
		{
			for (uint8_t i = NUM_CSW ; i < NUM_CSW+EXTRA_CSW ; i += 1 )
			{
	  	  CxSwData *cs = &g_model.xcustomSw[i-NUM_CSW];
        uint8_t cstate = CS_STATE(cs->func, g_model.modelVersion);
    		if(cstate == CS_VOFS)
				{
      		if (cs->v1)
					{
    		  	if (cs->v1 <= 4)		// Stick
						{
    	    		cs->v1 = stickScramble[g_eeGeneral.stickMode*4+cs->v1-1] + 1 ;
						}
					}
				}
				else if(cstate == CS_VCOMP)
				{
      		if (cs->v1)
					{
    		  	if (cs->v1 <= 4)		// Stick
						{
		    		  cs->v1 = stickScramble[g_eeGeneral.stickMode*4+cs->v1-1] + 1 ;
    	    	}
					}
      		if (cs->v2)
					{
    		  	if (cs->v2 <= 4)		// Stick
						{
							cs->v2 = stickScramble[g_eeGeneral.stickMode*4+cs->v2-1] + 1 ;
				  	}
					}
				}
			}
		}
#endif

		// EXPO/DR corrections
		
		int i, j, k ;
		int dest, src ;
	  ExpoData  lexpoData[4];
		
		for ( i = 0 ; i < 3 ; i += 1 )
		{ // 0=High, 1=Mid, 2=Low
			for ( j = 0 ; j < 2 ; j += 1 )
			{ // 0=Weight, 1=Expo - WRONG - 0=expo, 1=weight
				for ( k = 0 ; k < 2 ; k += 1 )
				{ // 0=Right, 1=Left
          dest = CONVERT_MODE(1, 2, g_eeGeneral.stickMode)-1 ;
          src = CONVERT_MODE(1, 1, g_eeGeneral.stickMode)-1 ;
          lexpoData[dest].expo[i][j][k] = g_model.expoData[src].expo[i][j][k] ;
          dest = CONVERT_MODE(2, 2, g_eeGeneral.stickMode)-1 ;
          src = CONVERT_MODE(2, 1, g_eeGeneral.stickMode)-1 ;
          lexpoData[dest].expo[i][j][k] = g_model.expoData[src].expo[i][j][k] ;
          dest = CONVERT_MODE(3, 2, g_eeGeneral.stickMode)-1 ;
          src = CONVERT_MODE(3, 1, g_eeGeneral.stickMode)-1 ;
          lexpoData[dest].expo[i][j][k] = g_model.expoData[src].expo[i][j][k] ;
          dest = CONVERT_MODE(4, 2, g_eeGeneral.stickMode)-1 ;
          src = CONVERT_MODE(4, 1, g_eeGeneral.stickMode)-1 ;
          lexpoData[dest].expo[i][j][k] = g_model.expoData[src].expo[i][j][k] ;
		    }
			}
		}
		for ( i = 1 ; i < 4 ; i += 1 )
		{
      dest = CONVERT_MODE(i, 2, g_eeGeneral.stickMode)-1 ;
      src = CONVERT_MODE(i, 1, g_eeGeneral.stickMode)-1 ;
      lexpoData[dest].drSw1 = g_model.expoData[src].drSw1 ;
      lexpoData[dest].drSw2 = g_model.expoData[src].drSw2 ;
		}
		memmove( &g_model.expoData, &lexpoData, sizeof(lexpoData));
		 
		g_model.modelVersion = 2 ;
    tabSwitches();
    tabMixes();
  	updateSettings();
		ui->label_version->setText( tr("%1").arg( g_model.modelVersion ) ) ;
	}
	ui->updateButton->setVisible( false ) ;
}

void ModelEdit::updateToMV3()
{
	updateToMV2() ;
  if ( g_model.modelVersion < 3 )
	{
#ifndef V2
    for (uint8_t i = 0 ; i < NUM_CSW ; i += 1 )
		{
    	CSwData *cs = &g_model.customSw[i];
			if ( cs->func == CS_LATCH )
			{
				cs->func = CS_GREATER ;
			}
			if ( cs->func == CS_FLIP )
			{
				cs->func = CS_LESS ;
			}
		}
#endif
#ifndef V2
    if ( eeFile->mee_type )
		{
			for (uint8_t i = NUM_CSW ; i < NUM_CSW+EXTRA_CSW ; i += 1 )
			{
	    	CxSwData *cs = &g_model.xcustomSw[i-NUM_CSW];
				if ( cs->func == CS_LATCH )
				{
					cs->func = CS_GREATER ;
				}
				if ( cs->func == CS_FLIP )
				{
					cs->func = CS_LESS ;
				}
			}
		}
#endif

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

void ModelEdit::updateToMV4()
{
	g_model.modelVersion = 4 ;
	ui->label_version->setText( tr("%1").arg( g_model.modelVersion ) ) ;
  updateSettings() ;
}
	 
void ModelEdit::setSubSubProtocol( QComboBox *b, int type )
{
	b->clear() ;
	switch ( type )
	{
		case M_Flysky :
			b->addItem("Flysky");
			b->addItem("V9x9");
			b->addItem("V6x6");
			b->addItem("V912");
		break ;
		case M_Hisky :
			b->addItem("Hisky");
			b->addItem("HK310");
		break ;
		case M_DSM2 :
			b->addItem("DSM2");
			b->addItem("DSMX");
		break ;
		case M_YD717 :
			b->addItem("YD717");
			b->addItem("SKYWLKR");
			b->addItem("SYMAX2");
      b->addItem("NIHUI");
			b->addItem("NIHUI");
		break ;
		case M_SymaX :
			b->addItem("SYMAX");
			b->addItem("SYMAX5C");
		break ;
		case M_CX10 :
			b->addItem("GREEN");
			b->addItem("BLUE");
			b->addItem("DM007");
		break ;
		case M_CG023 :
			b->addItem("CG023");
			b->addItem("YD829");
		break ;
		default :
			b->addItem("NONE");
		break ;
	}
}

void ModelEdit::setProtocolBoxes()
{
    protocolEditLock = true;
    ui->protocolCB->setCurrentIndex(g_model.protocol);
		ui->PPM1stChan->setValue( g_model.ppmStart + 1 ) ;

    switch (g_model.protocol)
    {
    case (PROTO_PXX):
        ui->ppmDelaySB->setEnabled(false);
        ui->numChannelsSB->setEnabled(false);
        ui->ppmFrameLengthDSB->setEnabled(false);
        ui->DSM_Type->hide() ;
        ui->SubProtocolCB->hide() ;
        ui->SubSubProtocolCB->hide() ;
        ui->pxxRxNum->setEnabled(true);
        ui->countryCB->setEnabled(true);
        ui->typeCB->setEnabled(true);

        ui->pxxRxNum->setValue(g_model.ppmNCH);

        ui->DSM_Type->setCurrentIndex(0);
        ui->ppmDelaySB->setValue(300);
        ui->numChannelsSB->setValue(8);
        ui->ppmFrameLengthDSB->setValue(22.5);
        break;
    case (PROTO_DSM2):
        ui->ppmDelaySB->setEnabled(false);
        ui->numChannelsSB->setEnabled(false);
        ui->ppmFrameLengthDSB->setEnabled(false);
        ui->DSM_Type->show() ;
        ui->SubProtocolCB->hide() ;
        ui->SubSubProtocolCB->hide() ;
        ui->pxxRxNum->setEnabled(false);

        ui->DSM_Type->setCurrentIndex(g_model.sub_protocol);

        ui->pxxRxNum->setValue(1);
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
        ui->SubSubProtocolCB->show() ;
        {
          int x = g_model.sub_protocol&0x1F ;
          setSubSubProtocol( ui->SubSubProtocolCB, x ) ;
          ui->SubSubProtocolCB->setCurrentIndex((g_model.ppmNCH & 0x70)>>4)	;
        }
        ui->pxxRxNum->setEnabled(false);
        ui->typeCB->setEnabled(false);
        ui->countryCB->setEnabled(false);
//				ui->startChannelsSB->setEnabled(true);
				ui->pulsePolCB->setEnabled(false);
      break ;

    default:
        ui->ppmDelaySB->setEnabled(true);
        ui->numChannelsSB->setEnabled(true);
        ui->ppmFrameLengthDSB->setEnabled(true);
        ui->DSM_Type->hide() ;
        ui->SubProtocolCB->hide() ;
        ui->SubSubProtocolCB->hide() ;
        ui->pxxRxNum->setEnabled(false);

        ui->ppmDelaySB->setValue(300+50*g_model.ppmDelay);
        ui->numChannelsSB->setValue(8+2*g_model.ppmNCH);
        ui->ppmFrameLengthDSB->setValue(22.5+((double)g_model.ppmFrameLength)*0.5);

        ui->pxxRxNum->setValue(1);
        ui->DSM_Type->setCurrentIndex(0);
        ui->countryCB->setEnabled(false);
        ui->typeCB->setEnabled(false);
        break;
    }
    protocolEditLock = false;
}

void ModelEdit::tabExpo()
{
	int x ;
	int y ;
    populateSwitchCB(ui->RUD_edrSw1,g_model.expoData[CONVERT_MODE(RUD,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1,eeFile->mee_type);
    populateSwitchCB(ui->RUD_edrSw2,g_model.expoData[CONVERT_MODE(RUD,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2,eeFile->mee_type);
    populateSwitchCB(ui->ELE_edrSw1,g_model.expoData[CONVERT_MODE(ELE,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1,eeFile->mee_type);
    populateSwitchCB(ui->ELE_edrSw2,g_model.expoData[CONVERT_MODE(ELE,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2,eeFile->mee_type);
    populateSwitchCB(ui->THR_edrSw1,g_model.expoData[CONVERT_MODE(THR,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1,eeFile->mee_type);
    populateSwitchCB(ui->THR_edrSw2,g_model.expoData[CONVERT_MODE(THR,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2,eeFile->mee_type);
    populateSwitchCB(ui->AIL_edrSw1,g_model.expoData[CONVERT_MODE(AIL,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1,eeFile->mee_type);
    populateSwitchCB(ui->AIL_edrSw2,g_model.expoData[CONVERT_MODE(AIL,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2,eeFile->mee_type);



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
    				/*if ( ( x >= -100 && x <= 0 ) )*/ x += 100 ;
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
//  if ( ( x >= 0 && x <= 100 ) ) 
	x -= 100 ;
  *pval = x ;
}


void ModelEdit::expoEdited()
{
		int i, j, k ;
		QSpinBox *sb ;
		QComboBox *cb ;
		QCheckBox *chkb ;
  int8_t *pval ;
    g_model.expoData[CONVERT_MODE(RUD,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1 = getSwitchCbValue( ui->RUD_edrSw1, eeFile->mee_type ) ;
    g_model.expoData[CONVERT_MODE(RUD,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2 = getSwitchCbValue( ui->RUD_edrSw2, eeFile->mee_type ) ;
    g_model.expoData[CONVERT_MODE(ELE,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1 = getSwitchCbValue( ui->ELE_edrSw1, eeFile->mee_type ) ;
    g_model.expoData[CONVERT_MODE(ELE,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2 = getSwitchCbValue( ui->ELE_edrSw2, eeFile->mee_type ) ;
    g_model.expoData[CONVERT_MODE(THR,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1 = getSwitchCbValue( ui->THR_edrSw1, eeFile->mee_type ) ;
    g_model.expoData[CONVERT_MODE(THR,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2 = getSwitchCbValue( ui->THR_edrSw2, eeFile->mee_type ) ;
    g_model.expoData[CONVERT_MODE(AIL,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw1 = getSwitchCbValue( ui->AIL_edrSw1, eeFile->mee_type ) ;
    g_model.expoData[CONVERT_MODE(AIL,g_model.modelVersion,g_eeGeneral.stickMode)-1].drSw2 = getSwitchCbValue( ui->AIL_edrSw2, eeFile->mee_type ) ;
		
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
	for(i=0 ; i<NUM_VOICE_ALARMS ; i += 1)
	{
		VoiceAlarmData *vad = &g_model.vad[i] ;
		QString str = "";
		str = tr("VA%1%2  ").arg((i+1)/10).arg((i+1)%10) ;
		QString srcstr ;
		int limit = 45-8 ;
//		if ( rData->type )
//		{
//			limit = 46 ;
//		}
		if ( vad->source < limit )
		{
      str += tr("(%1) ").arg(getSourceStr(g_eeGeneral.stickMode,vad->source,g_model.modelVersion )) ;//, rData->type )) ;
		}
		else
		{
			str += tr("(%1) ").arg(getTelemString(vad->source-limit+1 )) ;
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
    str += tr("Switch(%1) ").arg(getSWName(vad->swtch, eeFile->mee_type)) ;// rData->type)) ;
		if ( vad->rate < 3 )
		{
			srcstr = vad->rate ? (vad->rate == 1 ? "OFF " : "BOTH ") : "ON " ;
			str += srcstr ;
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
//        QString xstr = (char *)vad->file.name ;
//				xstr = xstr.left(8) ;
//        str += tr("File(%1)").arg(xstr ) ;
				str += tr("File(%1)").arg(vad->vfile ) ;
      }
			break ;
			case 2 :
//				str += tr("File(%1)").arg(vad->file.vfile ) ;
				str += tr("Alarm(%1)").arg(getAudioAlarmName(vad->vfile) ) ;
			break ;
//			case 3 :
//				str += tr("Alarm(%1)").arg(getAudioAlarmName(vad->file.vfile) ) ;
//			break ;
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
    for(i=0; i<MAX_MIXERS; i++)
    {
        MixData *md = &g_model.mixData[i];
        if((md->destCh==0) || (md->destCh>NUM_CHNOUT)) break;
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
        	str += QString(" GV%1").arg(j-125).rightJustified(6,' ') ;
				}
				else
				{
        	str += j<0 ? QString(" %1\%").arg(j).rightJustified(6,' ') :
                              QString(" +%1\%").arg(j).rightJustified(6, ' ');
				}


        //QString srcStr = SRC_STR;
        //str += " " + srcStr.mid(CONVERT_MODE(md->srcRaw+1)*4,4);
//				if ( md->srcRaw == MIX_3POS )
//				{
//					str+= "3POS" ;					
//				}
//				else
//				{
					if ( md->srcRaw == MIX_3POS )
					{
						QString swstr = "sIDxsTHRsRUDsELEsAILsGEAsTRN" ;
            str += swstr.mid(md->sw23pos*4,4) ;
					}
				  else if ( ( md->srcRaw >= 21 && md->srcRaw <= 36 ) && ( md->disableExpoDr ) )
					{
        		str += QString("OP%1").arg(md->srcRaw-20) ;
					}
					else
					{
        		str += getSourceStr(g_eeGeneral.stickMode,md->srcRaw, g_model.modelVersion ) ;
					}
//				}

        if(md->swtch) str += tr(" Switch(") + getSWName(md->swtch,eeFile->mee_type) + ")";
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

#ifndef V2
        if ( g_model.mixTime )
#else
				if ( md->hiResSlow )
#endif
				
				{
        	if(md->delayDown || md->delayUp) str += tr(" Delay(u%1:d%2)").arg((double)md->delayDown/5).arg((double)md->delayUp/5) ;
	        if(md->speedDown || md->speedUp) str += tr(" Slow(u%1:d%2)").arg((double)md->speedUp/5).arg((double)md->speedDown/5) ;
				}
				else
        {
        	if(md->delayDown || md->delayUp) str += tr(" Delay(u%1:d%2)").arg(md->delayDown).arg(md->delayUp) ;
	        if(md->speedDown || md->speedUp) str += tr(" Slow(u%1:d%2)").arg(md->speedUp).arg(md->speedDown) ;
				}

        if(md->mixWarn)  str += tr(" Warn(%1)").arg(md->mixWarn);

        if(!mixNotes[i].isEmpty())
            str += " (Note)";

        qba.clear();
        qba.append((quint8)i);
        qba.append((const char*)md, sizeof(MixData));
        QListWidgetItem *itm = new QListWidgetItem(str);
        itm->setData(Qt::UserRole,qba);  // mix number
        MixerlistWidget->addItem(itm);//(str);
        MixerlistWidget->item(MixerlistWidget->count()-1)->setToolTip(mixNotes[i]);
    }

    while(curDest<NUM_XCHNOUT)
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

void ModelEdit::tabPhase()
{
	updatePhaseTab() ;
	
	connect(ui->FP1_sw,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP2_sw,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP3_sw,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 
	connect(ui->FP4_sw,SIGNAL(currentIndexChanged(int)),this,SLOT(phaseEdited())); 

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
	connect(ui->FM1FadeIn,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM1FadeOut,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM2FadeIn,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM2FadeOut,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM3FadeIn,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM3FadeOut,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM4FadeIn,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
	connect(ui->FM4FadeOut,SIGNAL(valueChanged(double)),this,SLOT(phaseEdited())); 
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

	populateSwitchShortCB( ui->FP1_sw, g_model.phaseData[0].swtch, eeFile->mee_type ) ;
	populateSwitchShortCB( ui->FP2_sw, g_model.phaseData[1].swtch, eeFile->mee_type ) ;
	populateSwitchShortCB( ui->FP3_sw, g_model.phaseData[2].swtch, eeFile->mee_type ) ;
	populateSwitchShortCB( ui->FP4_sw, g_model.phaseData[3].swtch, eeFile->mee_type ) ;

	populatePhasetrim( ui->FP1_RudCB, 1,  g_model.phaseData[0].trim[0] ) ;
	populatePhasetrim( ui->FP1_EleCB, 1,  g_model.phaseData[0].trim[1] ) ;
	populatePhasetrim( ui->FP1_ThrCB, 1,  g_model.phaseData[0].trim[2] ) ;
	populatePhasetrim( ui->FP1_AilCB, 1,  g_model.phaseData[0].trim[3] ) ;
	populatePhasetrim( ui->FP2_RudCB, 2,  g_model.phaseData[1].trim[0] ) ;
	populatePhasetrim( ui->FP2_EleCB, 2,  g_model.phaseData[1].trim[1] ) ;
	populatePhasetrim( ui->FP2_ThrCB, 2,  g_model.phaseData[1].trim[2] ) ;
	populatePhasetrim( ui->FP2_AilCB, 2,  g_model.phaseData[1].trim[3] ) ;
	populatePhasetrim( ui->FP3_RudCB, 3,  g_model.phaseData[2].trim[0] ) ;
	populatePhasetrim( ui->FP3_EleCB, 3,  g_model.phaseData[2].trim[1] ) ;
	populatePhasetrim( ui->FP3_ThrCB, 3,  g_model.phaseData[2].trim[2] ) ;
	populatePhasetrim( ui->FP3_AilCB, 3,  g_model.phaseData[2].trim[3] ) ;
	populatePhasetrim( ui->FP4_RudCB, 4,  g_model.phaseData[3].trim[0] ) ;
	populatePhasetrim( ui->FP4_EleCB, 4,  g_model.phaseData[3].trim[1] ) ;
	populatePhasetrim( ui->FP4_ThrCB, 4,  g_model.phaseData[3].trim[2] ) ;
	populatePhasetrim( ui->FP4_AilCB, 4,  g_model.phaseData[3].trim[3] ) ;

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

	ui->FP1rudTrimSB->setDisabled( true ) ;
	ui->FP1eleTrimSB->setDisabled( true ) ;
	ui->FP1thrTrimSB->setDisabled( true ) ;
	ui->FP1ailTrimSB->setDisabled( true ) ;
	ui->FP2rudTrimSB->setDisabled( true ) ;
	ui->FP2eleTrimSB->setDisabled( true ) ;
	ui->FP2thrTrimSB->setDisabled( true ) ;
	ui->FP2ailTrimSB->setDisabled( true ) ;
	ui->FP3rudTrimSB->setDisabled( true ) ;
	ui->FP3eleTrimSB->setDisabled( true ) ;
	ui->FP3thrTrimSB->setDisabled( true ) ;
	ui->FP3ailTrimSB->setDisabled( true ) ;
	ui->FP4rudTrimSB->setDisabled( true ) ;
	ui->FP4eleTrimSB->setDisabled( true ) ;
	ui->FP4thrTrimSB->setDisabled( true ) ;
	ui->FP4ailTrimSB->setDisabled( true ) ;

	ui->FM1FadeIn->setValue(g_model.phaseData[0].fadeIn/2.0) ;
	ui->FM1FadeOut->setValue(g_model.phaseData[0].fadeOut/2.0) ;
	ui->FM2FadeIn->setValue(g_model.phaseData[1].fadeIn/2.0) ;
	ui->FM2FadeOut->setValue(g_model.phaseData[1].fadeOut/2.0) ;
	ui->FM3FadeIn->setValue(g_model.phaseData[2].fadeIn/2.0) ;
	ui->FM3FadeOut->setValue(g_model.phaseData[2].fadeOut/2.0) ;
	ui->FM4FadeIn->setValue(g_model.phaseData[3].fadeIn/2.0) ;
	ui->FM4FadeOut->setValue(g_model.phaseData[3].fadeOut/2.0) ;
}

void ModelEdit::phaseEdited()
{
	
	int limit = MAX_DRSWITCH-1 ;
#ifndef SKY
	if ( eeFile->mee_type )
	{
  	limit += EXTRA_CSW ;
	}
#endif
	
  g_model.phaseData[0].swtch = ui->FP1_sw->currentIndex() - limit ;
  g_model.phaseData[1].swtch = ui->FP2_sw->currentIndex() - limit ;
  g_model.phaseData[2].swtch = ui->FP3_sw->currentIndex() - limit ;
  g_model.phaseData[3].swtch = ui->FP4_sw->currentIndex() - limit ;

//  if ( (idx = decodePhaseTrim( &g_model.phaseData[0].trim[0], 1, ui->FP1_RudCB->currentIndex() ) < 0 ) )
//	{
//		ui->FP1rudTrimSB->setValue(g_model.phaseData[0].trim[0]) ;
//		ui->FP1rudTrimSB->setEnabled( true ) ;
//	}
//	else
//	{
//    ui->FP1rudTrimSB->setValue( idx ? g_model.phaseData[idx-1].trim[0] : g_model.trim[0] ) ;		// Needs recursion added
//		ui->FP1rudTrimSB->setDisabled( true ) ;
//	}
//	decodePhaseTrim( &g_model.phaseData[0].trim[1], 1, ui->FP1_EleCB->currentIndex() ) ;
	
//	ui->FP1eleTrimSB->setValue(g_model.phaseData[0].trim[1]) ;
//	ui->FP1eleTrimSB->setDisabled( true ) ;

	decodePhaseTrim( &g_model.phaseData[0].trim[0], ui->FP1_RudCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[0].trim[1], ui->FP1_EleCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[0].trim[2], ui->FP1_ThrCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[0].trim[3], ui->FP1_AilCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[1].trim[0], ui->FP2_RudCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[1].trim[1], ui->FP2_EleCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[1].trim[2], ui->FP2_ThrCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[1].trim[3], ui->FP2_AilCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[2].trim[0], ui->FP3_RudCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[2].trim[1], ui->FP3_EleCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[2].trim[2], ui->FP3_ThrCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[2].trim[3], ui->FP3_AilCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[3].trim[0], ui->FP4_RudCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[3].trim[1], ui->FP4_EleCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[3].trim[2], ui->FP4_ThrCB->currentIndex() ) ;
	decodePhaseTrim( &g_model.phaseData[3].trim[3], ui->FP4_AilCB->currentIndex() ) ;

	g_model.phaseData[0].fadeIn = ( ui->FM1FadeIn->value() + 0.01 ) * 2 ;
	g_model.phaseData[0].fadeOut = ( ui->FM1FadeOut->value() + 0.01 ) * 2 ;
	g_model.phaseData[1].fadeIn = ( ui->FM2FadeIn->value() + 0.01 ) * 2 ;
	g_model.phaseData[1].fadeOut = ( ui->FM2FadeOut->value() + 0.01 ) * 2 ;
	g_model.phaseData[2].fadeIn = ( ui->FM3FadeIn->value() + 0.01 ) * 2 ;
	g_model.phaseData[2].fadeOut = ( ui->FM3FadeOut->value() + 0.01 ) * 2 ;
	g_model.phaseData[3].fadeIn = ( ui->FM4FadeIn->value() + 0.01 ) * 2 ;
	g_model.phaseData[3].fadeOut = ( ui->FM4FadeOut->value() + 0.01 ) * 2 ;



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
    populateSourceCB(ui->swashCollectiveCB,g_eeGeneral.stickMode,0,g_model.swashCollectiveSource,g_model.modelVersion);
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
    g_model.swashCollectiveSource = ui->swashCollectiveCB->currentIndex();
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

		limitAuto() ;

    setLimitMinMax();

//    limitOffset[0] = new QSpinBox(this);
//		limitOffset[0]->setMaximum(100);
//		limitOffset[0]->setMinimum(-100);
//    limitOffset[1] = new QSpinBox(this);
//		limitOffset[1]->setMaximum(100);
//		limitOffset[1]->setMinimum(-100);
//    ui->grid_tabLimits->addWidget(limitOffset[0],17,1) ;
//    ui->grid_tabLimits->addWidget(limitOffset[1],18,1) ;

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
     plot_curve[i]=0;
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
	QLabel *Amin[16] ;
	QLabel *Amax[16] ;
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

	for ( i = 0 ; i < 16 ; i += 1 )
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
	int v1 = g_model.customSw[i].v1 ;
	int v2 = g_model.customSw[i].v2 ;
	int function = g_model.customSw[i].func ;


#ifndef V2
  if ( i >= NUM_CSW )
	{
		if ( eeFile->mee_type == 0 )
		{
      cswitchSource1[i]->setVisible(false);
      cswitchSource2[i]->setVisible(false);
      cswitchOffset0[i]->setVisible(false);
     	cswitchTlabel[i]->setVisible(false);
			cswitchText1[i]->setVisible(false) ;
			cswitchText2[i]->setVisible(false) ;
			return ;
		}

		v1 = g_model.xcustomSw[i-NUM_CSW].v1 ;
		v2 = g_model.xcustomSw[i-NUM_CSW].v2 ;
		function = g_model.xcustomSw[i-NUM_CSW].func ;
	}
#endif

  switch (CS_STATE(function, g_model.modelVersion))
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
        cswitchOffset0[i]->setVisible(false);
        populateSourceCB(cswitchSource1[i],g_eeGeneral.stickMode,1,v1,g_model.modelVersion);
        cswitchOffset[i]->setValue(v2);
				if ( cswitchSource1[i]->currentIndex() > 36 )
				{
        	cswitchTlabel[i]->setVisible(true);
					value = convertTelemConstant( cswitchSource1[i]->currentIndex() - 37, v2, &g_model ) ;
          stringTelemetryChannel( telText, v1 - 37, value, &g_model ) ;
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
        populateSwitchCB(cswitchSource1[i],v1, eeFile->mee_type ) ;
        populateSwitchCB(cswitchSource2[i],v2, eeFile->mee_type ) ;
				cswitchText1[i]->setVisible(false) ;
				cswitchText2[i]->setVisible(false) ;
        break;
    case CS_VCOMP:
        cswitchSource1[i]->setVisible(true);
        cswitchSource2[i]->setVisible(true);
        cswitchOffset[i]->setVisible(false);
        cswitchOffset0[i]->setVisible(false);
        populateSourceCB(cswitchSource1[i],g_eeGeneral.stickMode,1,v1,g_model.modelVersion);
        populateSourceCB(cswitchSource2[i],g_eeGeneral.stickMode,1,v2,g_model.modelVersion);
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
        cswitchOffset0[i]->setVisible(true);

				value = v2+1 ;
        cswitchOffset[i]->setValue(value);
				if ( value <= 0 )
				{
					cswitchText2[i]->setVisible(true) ;
					value = -value + 1 ;
          cswitchText2[i]->setText( tr("%1.%2").arg( value/10 ).arg( value%10 ) ) ;
        	cswitchText2[i]->resize( ui->cswCol2->width(), 20 );
					cswitchText2[i]->raise() ;
				}
				else
				{
					cswitchText2[i]->setVisible(false) ;
				}

				value = v1+1 ;
        cswitchOffset0[i]->setValue(value);
				if ( value <= 0 )
				{
					cswitchText1[i]->setVisible(true) ;
					value = -value + 1 ;
          cswitchText1[i]->setText( tr("%1.%2").arg( value/10 ).arg( value%10 ) ) ;
        	cswitchText1[i]->resize( ui->cswCol1->width(), 20 );
					cswitchText1[i]->raise() ;
				}
				else
				{
					cswitchText1[i]->setVisible(false) ;
				}
        break;
    default:
        break;
  }
}

void ModelEdit::updateSwitchesTab()
{
    switchEditLock = true;


#ifndef V2
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
    populateCSWCB(ui->cswitchFunc_13,g_model.xcustomSw[0].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_14,g_model.xcustomSw[1].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_15,g_model.xcustomSw[2].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_16,g_model.xcustomSw[3].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_17,g_model.xcustomSw[4].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_18,g_model.xcustomSw[5].func, g_model.modelVersion|0x80);
#else
    populateCSWCB(ui->cswitchFunc_1, g_model.customSw[0].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_2, g_model.customSw[1].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_3, g_model.customSw[2].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_4, g_model.customSw[3].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_5, g_model.customSw[4].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_6, g_model.customSw[5].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_7, g_model.customSw[6].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_8, g_model.customSw[7].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_9, g_model.customSw[8].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_10,g_model.customSw[9].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_11,g_model.customSw[10].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_12,g_model.customSw[11].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_13,g_model.customSw[12].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_14,g_model.customSw[13].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_15,g_model.customSw[14].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_16,g_model.customSw[15].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_17,g_model.customSw[16].func, g_model.modelVersion|0x80);
    populateCSWCB(ui->cswitchFunc_18,g_model.customSw[17].func, g_model.modelVersion|0x80);
#endif

#ifndef V2
	if ( eeFile->mee_type )
	{
		ui->cswitchFunc_13->setVisible(true) ;
		ui->cswitchFunc_14->setVisible(true) ;
		ui->cswitchFunc_15->setVisible(true) ;
		ui->cswitchFunc_16->setVisible(true) ;
		ui->cswitchFunc_17->setVisible(true) ;
		ui->cswitchFunc_18->setVisible(true) ;
		cswitchAndSwitch[12]->setVisible(true) ;
		cswitchAndSwitch[13]->setVisible(true) ;
		cswitchAndSwitch[14]->setVisible(true) ;
		cswitchAndSwitch[15]->setVisible(true) ;
		cswitchAndSwitch[16]->setVisible(true) ;
		cswitchAndSwitch[17]->setVisible(true) ;
	}
	else
	{
		ui->cswitchFunc_13->setVisible(false) ;
		ui->cswitchFunc_14->setVisible(false) ;
		ui->cswitchFunc_15->setVisible(false) ;
		ui->cswitchFunc_16->setVisible(false) ;
		ui->cswitchFunc_17->setVisible(false) ;
		ui->cswitchFunc_18->setVisible(false) ;
		cswitchAndSwitch[12]->setVisible(false) ;
		cswitchAndSwitch[13]->setVisible(false) ;
		cswitchAndSwitch[14]->setVisible(false) ;
		cswitchAndSwitch[15]->setVisible(false) ;
		cswitchAndSwitch[16]->setVisible(false) ;
		cswitchAndSwitch[17]->setVisible(false) ;
	}
#else
		ui->cswitchFunc_13->setVisible(true) ;
		ui->cswitchFunc_14->setVisible(true) ;
		ui->cswitchFunc_15->setVisible(true) ;
		ui->cswitchFunc_16->setVisible(true) ;
		ui->cswitchFunc_17->setVisible(true) ;
		ui->cswitchFunc_18->setVisible(true) ;
		cswitchAndSwitch[12]->setVisible(true) ;
		cswitchAndSwitch[13]->setVisible(true) ;
		cswitchAndSwitch[14]->setVisible(true) ;
		cswitchAndSwitch[15]->setVisible(true) ;
		cswitchAndSwitch[16]->setVisible(true) ;
		cswitchAndSwitch[17]->setVisible(true) ;
#endif
    for(int i=0; i<NUM_CSW+EXTRA_CSW; i++)
        setSwitchWidgetVisibility(i);

    switchEditLock = false;
}

// Fault report:
// Also when adjusting safety switches to voice function, in eePe if you specify
// only 1 safety switch (as I need a lot of Voice switches) it will not take the
// channel just replaces what I enter "THR" with "----". You can trick it to stay
// of you expand the safety switches to 2, enter your parameters then switch back
// to one. But its not a good work around. 

void ModelEdit::tabSwitches()
{
	int width ;
	int last ;

	last = NUM_CSW ;
	if ( eeFile->mee_type )
	{
		last += EXTRA_CSW ;
	}

    switchEditLock = true;

    for(int i=0; i<NUM_CSW+EXTRA_CSW; i++)
    {
      if ( !switchesTabDone )
			{
				
        cswitchSource1[i] = new QComboBox(this);
        connect(cswitchSource1[i],SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
        ui->gridLayout_8->addWidget(cswitchSource1[i],i+1,2);
        cswitchSource1[i]->setVisible(false);

        cswitchSource2[i] = new QComboBox(this);
        connect(cswitchSource2[i],SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
        ui->gridLayout_8->addWidget(cswitchSource2[i],i+1,3);
        cswitchSource2[i]->setVisible(false);

        cswitchAndSwitch[i] = new QComboBox(this);
        connect(cswitchAndSwitch[i],SIGNAL(currentIndexChanged(int)),this,SLOT(switchesEdited()));
        ui->gridLayout_8->addWidget(cswitchAndSwitch[i],i+1,5);
        cswitchAndSwitch[i]->setVisible(true);
			}
#ifndef V2
				if (i<NUM_CSW)
				{
					populateSwitchAndCB(cswitchAndSwitch[i], g_model.customSw[i].andsw) ;
				}
        else
				{
          populateSwitchxAndCB(cswitchAndSwitch[i], g_model.xcustomSw[i-NUM_CSW].andsw, eeFile->mee_type ) ;
				}
#else
        populateSwitchxAndCB(cswitchAndSwitch[i], g_model.customSw[i].andsw, 1 ) ;
#endif
        if ( !switchesTabDone )
			{
				cswitchTlabel[i] = new QLabel(this) ;
        ui->gridLayout_8->addWidget(cswitchTlabel[i],i+1,4);
        cswitchTlabel[i]->setVisible(false);
        cswitchTlabel[i]->setText("");

        cswitchOffset[i] = new QSpinBox(this);
        cswitchOffset[i]->setMaximum(125);
        cswitchOffset[i]->setMinimum(-125);
        cswitchOffset[i]->setAccelerated(true);
        connect(cswitchOffset[i],SIGNAL(valueChanged(int)),this,SLOT(switchesEdited()));
        ui->gridLayout_8->addWidget(cswitchOffset[i],i+1,3);
        cswitchOffset[i]->setVisible(false);
				width = ui->cswCol1->width() ;
				cswitchOffset[i]->resize( width, 20 );

				width -= 18 ;
        cswitchText2[i] = new QTextBrowser(this);
        ui->gridLayout_8->addWidget(cswitchText2[i],i+1,3);
        cswitchText2[i]->setVisible(false);
        cswitchText2[i]->setMinimumSize( width, 20 );
        cswitchText2[i]->setMaximumSize( width, 20 );
        cswitchText2[i]->resize( width, 20 );
				cswitchText2[i]->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff ) ;

				cswitchOffset0[i] = new QSpinBox(this);
        cswitchOffset0[i]->setMaximum(100);
        cswitchOffset0[i]->setMinimum(-49);
        cswitchOffset0[i]->setAccelerated(true);
        connect(cswitchOffset0[i],SIGNAL(valueChanged(int)),this,SLOT(switchesEdited()));
        ui->gridLayout_8->addWidget(cswitchOffset0[i],i+1,2);
        cswitchOffset0[i]->setVisible(false);
				width = ui->cswCol2->width() ;
        cswitchOffset0[i]->resize( width, 20 );

				width -= 18 ;
        cswitchText1[i] = new QTextBrowser(this);
        ui->gridLayout_8->addWidget(cswitchText1[i],i+1,2);
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
	}
    switchEditLock = false;
		switchesTabDone = true ;
}


void ModelEdit::setSafetyLabels()
{
#ifndef V2
  ui->SS1->setText(g_model.numVoice < 16 ? "CH1" : "VS1");
	ui->SS2->setText(g_model.numVoice < 15 ? "CH2" : "VS2");
	ui->SS3->setText(g_model.numVoice < 14 ? "CH3" : "VS3");
	ui->SS4->setText(g_model.numVoice < 13 ? "CH4" : "VS4");
	ui->SS5->setText(g_model.numVoice < 12 ? "CH5" : "VS5");
	ui->SS6->setText(g_model.numVoice < 11 ? "CH6" : "VS6");
	ui->SS7->setText(g_model.numVoice < 10 ? "CH7" : "VS7");
	ui->SS8->setText(g_model.numVoice < 9 ? "CH8" : "VS8");
	ui->SS9->setText(g_model.numVoice < 8 ? "CH9" : "VS9");
	ui->SS10->setText(g_model.numVoice < 7 ? "CH10" : "VS10");
	ui->SS11->setText(g_model.numVoice < 6 ? "CH11" : "VS11");
	ui->SS12->setText(g_model.numVoice < 5 ? "CH12" : "VS12");
	ui->SS13->setText(g_model.numVoice < 4 ? "CH13" : "VS13");
	ui->SS14->setText(g_model.numVoice < 3 ? "CH14" : "VS14");
	ui->SS15->setText(g_model.numVoice < 2 ? "CH15" : "VS15");
	ui->SS16->setText(g_model.numVoice < 1 ? "CH16" : "VS16");
#endif
  ui->SS17->setText("VS17") ;
	ui->SS18->setText("VS18") ;
	ui->SS19->setText("VS19") ;
	ui->SS20->setText("VS20") ;
	ui->SS21->setText("VS21") ;
	ui->SS22->setText("VS22") ;
	ui->SS23->setText("VS23") ;
	ui->SS24->setText("VS24") ;
}

void ModelEdit::setSafetyWidgetVisibility(int i)
{
//	if ( i >= NUM_CHNOUT )
//	{
//		if ( eeFile->mee_type == 0 )
//		{
//			safetySwitchGvar[i]->setVisible(false) ;
//			safetySwitchGindex[i]->setVisible(false) ;
// 	    safetySwitchValue[i]->setVisible(false);
// 	    safetySwitchAlarm[i]->setVisible(false);
//			return ;
//		}
//	}		
	
	
#ifndef V2
  if ( g_model.numVoice < 16-i )
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
				if ( g_model.safetySw[i].opt.ss.swtch > MAX_DRSWITCH )
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
	else // voice switch
#endif
  {
		safetySwitchValue[i]->setMaximum(250);
    safetySwitchValue[i]->setMinimum(0);

		int test ;
		if ( i >= NUM_CHNOUT )
		{
#ifndef V2
      test = g_model.xvoiceSw[i-NUM_CHNOUT].opt.vs.vmode > 5 ;
#endif
    }
		else
		{
#ifndef V2
      test = g_model.safetySw[i].opt.vs.vmode > 5 ;
#endif
    }

		if ( test )
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
#ifndef V2
    ui->NumVoiceSwSB->setValue(g_model.numVoice);
#endif
    setSafetyLabels() ;

    for(int i=0; i<NUM_CHNOUT+EXTRA_VOICE_SW; i++)
    {
			int j = i ;
			int k = 1 ;
			if ( i > 15 )
			{
				j = i - 16 ;
				k = 6 ;
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
				 
#ifndef V2
        if ( g_model.numVoice < NUM_CHNOUT-i )	// Normal switch
#endif
        {
#ifndef V2
          populateSafetyVoiceTypeCB(safetySwitchType[i], 0, g_model.safetySw[i].opt.ss.mode);
        	populateSafetySwitchCB(safetySwitchSwtch[i],g_model.safetySw[i].opt.ss.mode,g_model.safetySw[i].opt.ss.swtch, eeFile->mee_type);
					populateAlarmCB(safetySwitchAlarm[i],g_model.safetySw[i].opt.ss.val);
					if ( g_model.safetySw[i].opt.ss.mode == 2 )		// 'V'
					{
						if ( g_model.safetySw[i].opt.ss.swtch > MAX_DRSWITCH )
						{
							populateTelItemsCB( safetySwitchAlarm[i], 1,g_model.safetySw[i].opt.ss.val ) ;
						}
						safetySwitchValue[i]->setMaximum(239);
       			safetySwitchValue[i]->setMinimum(0);
       			safetySwitchValue[i]->setValue(g_model.safetySw[i].opt.ss.val+128);
					}
					else
					{
						safetySwitchValue[i]->setMaximum(125);
        		safetySwitchValue[i]->setMinimum(-125);
        		safetySwitchValue[i]->setValue(g_model.safetySw[i].opt.ss.val);
					}
#endif
        }
#ifndef V2
        else // voice switch
				{
					int mode ;
					int vswitch ;
					int val ;
					
					if ( i >= NUM_CHNOUT )
					{
						int j = i - NUM_CHNOUT ;
						mode = g_model.xvoiceSw[j].opt.vs.vmode ;
						vswitch = g_model.xvoiceSw[j].opt.vs.vswtch ;
						val = g_model.xvoiceSw[j].opt.vs.vval ;
					}
					else
					{
						mode = g_model.safetySw[i].opt.vs.vmode ;
						vswitch = g_model.safetySw[i].opt.vs.vswtch ;
						val = g_model.safetySw[i].opt.vs.vval ;
					}

					populateSafetyVoiceTypeCB(safetySwitchType[i], 1, mode);
        	populateSafetySwitchCB(safetySwitchSwtch[i],VOICE_SWITCH,vswitch, eeFile->mee_type ) ;
					safetySwitchValue[i]->setMaximum(250);
     			safetySwitchValue[i]->setMinimum(0);
       		safetySwitchValue[i]->setValue(val);
					populateTelItemsCB( safetySwitchAlarm[i], 1,val ) ;
					if ( val > 250 )
					{
            safetySwitchGindex[i]->setCurrentIndex( val - 251 ) ;
						safetySwitchGvar[i]->setChecked(true) ;
					}
					else
					{
						safetySwitchGvar[i]->setChecked(false) ;
					}
				}
#endif
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
	if ( 1 )
	{
		ui->SS17->setVisible(true) ;
		ui->SS18->setVisible(true) ;
		ui->SS19->setVisible(true) ;
		ui->SS20->setVisible(true) ;
		ui->SS21->setVisible(true) ;
		ui->SS22->setVisible(true) ;
		ui->SS23->setVisible(true) ;
		ui->SS24->setVisible(true) ;
		int i ;
		for ( i = 16 ; i < 24 ; i += 1 )
		{
			safetySwitchType[i]->setVisible(true) ;
			safetySwitchSwtch[i]->setVisible(true) ;
		}
	}
	else
	{
		ui->SS17->setVisible(false) ;
		ui->SS18->setVisible(false) ;
		ui->SS19->setVisible(false) ;
		ui->SS20->setVisible(false) ;
		ui->SS21->setVisible(false) ;
		ui->SS22->setVisible(false) ;
		ui->SS23->setVisible(false) ;
		ui->SS24->setVisible(false) ;
		int i ;
		for ( i = 16 ; i < 24 ; i += 1 )
		{
			safetySwitchType[i]->setVisible(false) ;
			safetySwitchSwtch[i]->setVisible(false) ;
		}
	}
}

static int EditedNesting = 0 ;

void ModelEdit::safetySwitchesEdited()
{
		int val ;
		int modechange[NUM_CHNOUT+EXTRA_VOICE_SW] ;
		int numVoice ;
//		int voiceindexchange[NUM_CHNOUT] ;

		if ( EditedNesting )
		{
			return ;
		}
		EditedNesting = 1 ;
		
#ifndef V2
    numVoice = g_model.numVoice ;
    g_model.numVoice = ui->NumVoiceSwSB->value() ;
#endif

#ifndef V2
    for(int i=0; i<NUM_CHNOUT+EXTRA_VOICE_SW; i++)
    {
			val = safetySwitchValue[i]->value();
			
			if ( numVoice < NUM_CHNOUT-i )	// Normal switch
			{
				modechange[i] = g_model.safetySw[i].opt.ss.mode ;	// Previous value
				if ( g_model.safetySw[i].opt.ss.mode == 2)	// Voice
				{
					val -= 128 ;
					if ( g_model.safetySw[i].opt.ss.swtch > MAX_DRSWITCH )
					{
						val = safetySwitchAlarm[i]->currentIndex() ;
					}
				}
				if ( g_model.safetySw[i].opt.ss.mode == 1)	// Alarm
				{
					val = safetySwitchAlarm[i]->currentIndex() ;
				}
        g_model.safetySw[i].opt.ss.val = val ;

        g_model.safetySw[i].opt.ss.mode  = safetySwitchType[i]->currentIndex() ;
        g_model.safetySw[i].opt.ss.swtch = safetySwitchSwtch[i]->currentIndex()-MAX_DRSWITCH;
			}
			else // voice switch
			{
				if ( i >= NUM_CHNOUT )
				{
					int j = i - NUM_CHNOUT ;
					g_model.xvoiceSw[j].opt.vs.vmode = safetySwitchType[i]->currentIndex() ;
					if ( g_model.xvoiceSw[j].opt.vs.vmode > 5 )
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
        	g_model.xvoiceSw[j].opt.vs.vval = val ;
        	g_model.xvoiceSw[j].opt.vs.vswtch = safetySwitchSwtch[i]->currentIndex() ;
				}
				else
				{
					g_model.safetySw[i].opt.vs.vmode = safetySwitchType[i]->currentIndex() ;
					if ( g_model.safetySw[i].opt.vs.vmode > 5 )
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
        	g_model.safetySw[i].opt.vs.vval = val ;
        	g_model.safetySw[i].opt.vs.vswtch = safetySwitchSwtch[i]->currentIndex() ;
				}
			}	
		}
#endif
    updateSettings();
		
#ifndef V2
    if ( g_model.numVoice != numVoice )
		{
			setSafetyLabels() ;
		}
#endif

#ifndef V2
    for(int i=0; i<NUM_CHNOUT+EXTRA_VOICE_SW; i++)
		{
			if ( g_model.numVoice < NUM_CHNOUT-i )		// Normal switch
			{
		    if ( i > NUM_CHNOUT-numVoice-1 && i < NUM_CHNOUT-g_model.numVoice )
				{
					g_model.safetySw[i].opt.ss.swtch = 0 ;
	    		populateSafetySwitchCB(safetySwitchSwtch[i],g_model.safetySw[i].opt.ss.mode,g_model.safetySw[i].opt.ss.swtch, eeFile->mee_type);
				}
        populateSafetyVoiceTypeCB(safetySwitchType[i], 0, g_model.safetySw[i].opt.ss.mode);
				if ( modechange[i] != g_model.safetySw[i].opt.ss.mode )
				{
	    		populateSafetySwitchCB(safetySwitchSwtch[i],g_model.safetySw[i].opt.ss.mode,g_model.safetySw[i].opt.ss.swtch, eeFile->mee_type);
					if ( g_model.safetySw[i].opt.ss.mode != 2 )
					{
						if ( g_model.safetySw[i].opt.ss.swtch > MAX_DRSWITCH )
						{
							g_model.safetySw[i].opt.ss.swtch = MAX_DRSWITCH ;
    					safetySwitchSwtch[i]->setCurrentIndex(MAX_DRSWITCH+MAX_DRSWITCH) ;
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
				if ( g_model.safetySw[i].opt.ss.swtch > MAX_DRSWITCH )
				{
					populateTelItemsCB( safetySwitchAlarm[i], 1,g_model.safetySw[i].opt.ss.val ) ;
				}
			}
			else
			{
		    if ( i >= NUM_CHNOUT-g_model.numVoice-1 && i < NUM_CHNOUT-numVoice )
				{
					g_model.safetySw[i].opt.vs.vswtch = 0 ;
          g_model.safetySw[i].opt.vs.vval = 0 ;
					g_model.safetySw[i].opt.vs.vmode = 0 ;
				}
				safetySwitchValue[i]->setMaximum(250);
     		safetySwitchValue[i]->setMinimum(0);
				if ( i >= NUM_CHNOUT )
				{
					int j = i - NUM_CHNOUT ;
					populateSafetySwitchCB(safetySwitchSwtch[i],VOICE_SWITCH,g_model.xvoiceSw[j].opt.vs.vswtch, eeFile->mee_type);
        	populateSafetyVoiceTypeCB(safetySwitchType[i], 1, g_model.xvoiceSw[j].opt.vs.vmode);
       		safetySwitchValue[i]->setValue(g_model.xvoiceSw[j].opt.vs.vval);
					if ( g_model.xvoiceSw[j].opt.vs.vmode > 5 )
					{
						populateTelItemsCB( safetySwitchAlarm[i], 1,g_model.xvoiceSw[j].opt.ss.val ) ;
					}
				}
				else
				{
					populateSafetySwitchCB(safetySwitchSwtch[i],VOICE_SWITCH,g_model.safetySw[i].opt.vs.vswtch, eeFile->mee_type);
        	populateSafetyVoiceTypeCB(safetySwitchType[i], 1, g_model.safetySw[i].opt.vs.vmode);
       		safetySwitchValue[i]->setValue(g_model.safetySw[i].opt.vs.vval);
					if ( g_model.safetySw[i].opt.vs.vmode > 5 )
					{
						populateTelItemsCB( safetySwitchAlarm[i], 1,g_model.safetySw[i].opt.ss.val ) ;
					}
				}
			}
      setSafetyWidgetVisibility(i);
		}
#endif
    EditedNesting = 0 ;

}


void ModelEdit::switchesEdited()
{
	char telText[20] ;
	int16_t value ;
	int16_t limit ;
		if(switchEditLock) return;
    switchEditLock = true;
		limit = MAX_DRSWITCH ;
#ifndef SKY
	if ( eeFile->mee_type )
	{
		limit += EXTRA_CSW ;
	}
#endif

    bool chAr[NUM_CSW+EXTRA_CSW];

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
#ifndef V2
    chAr[12] = (CS_STATE(g_model.xcustomSw[0].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_13->currentIndex(), g_model.modelVersion));
    chAr[13] = (CS_STATE(g_model.xcustomSw[1].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_14->currentIndex(), g_model.modelVersion));
    chAr[14] = (CS_STATE(g_model.xcustomSw[2].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_15->currentIndex(), g_model.modelVersion));
    chAr[15] = (CS_STATE(g_model.xcustomSw[3].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_16->currentIndex(), g_model.modelVersion));
    chAr[16] = (CS_STATE(g_model.xcustomSw[4].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_17->currentIndex(), g_model.modelVersion));
    chAr[17] = (CS_STATE(g_model.xcustomSw[5].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_18->currentIndex(), g_model.modelVersion));
#else
    chAr[12] = (CS_STATE(g_model.customSw[12].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_13->currentIndex(), g_model.modelVersion));
    chAr[13] = (CS_STATE(g_model.customSw[13].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_14->currentIndex(), g_model.modelVersion));
    chAr[14] = (CS_STATE(g_model.customSw[14].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_15->currentIndex(), g_model.modelVersion));
    chAr[15] = (CS_STATE(g_model.customSw[15].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_16->currentIndex(), g_model.modelVersion));
    chAr[16] = (CS_STATE(g_model.customSw[16].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_17->currentIndex(), g_model.modelVersion));
    chAr[17] = (CS_STATE(g_model.customSw[17].func, g_model.modelVersion))!=(CS_STATE(ui->cswitchFunc_18->currentIndex(), g_model.modelVersion));
#endif

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
#ifndef V2
    g_model.xcustomSw[0].func = ui->cswitchFunc_13->currentIndex();
    g_model.xcustomSw[1].func = ui->cswitchFunc_14->currentIndex();
    g_model.xcustomSw[2].func = ui->cswitchFunc_15->currentIndex();
    g_model.xcustomSw[3].func = ui->cswitchFunc_16->currentIndex();
    g_model.xcustomSw[4].func = ui->cswitchFunc_17->currentIndex();
    g_model.xcustomSw[5].func = ui->cswitchFunc_18->currentIndex();
#else
    g_model.customSw[12].func = ui->cswitchFunc_13->currentIndex();
    g_model.customSw[13].func = ui->cswitchFunc_14->currentIndex();
    g_model.customSw[14].func = ui->cswitchFunc_15->currentIndex();
    g_model.customSw[15].func = ui->cswitchFunc_16->currentIndex();
    g_model.customSw[16].func = ui->cswitchFunc_17->currentIndex();
    g_model.customSw[17].func = ui->cswitchFunc_18->currentIndex();
#endif


#ifndef V2
    for(int i=0; i<NUM_CSW+EXTRA_CSW; i++)
    {
			if ( ( eeFile->mee_type ) && ( i >= NUM_CSW ) )
			{
				g_model.xcustomSw[i-NUM_CSW].andsw = getxAndSwitchCbValue( cswitchAndSwitch[i], eeFile->mee_type ) ;
        if(chAr[i])
        {
            g_model.xcustomSw[i-NUM_CSW].v1 = 0;
            g_model.xcustomSw[i-NUM_CSW].v2 = 0;
            setSwitchWidgetVisibility(i);
        }
				int j = i - NUM_CSW ;
        switch(CS_STATE( g_model.xcustomSw[j].func, g_model.modelVersion))
        {
	        case (CS_VOFS):
            g_model.xcustomSw[j].v1 = cswitchSource1[i]->currentIndex();
            g_model.xcustomSw[j].v2 = cswitchOffset[i]->value();
						if ( g_model.xcustomSw[j].v1 > 36 )
						{
              value = convertTelemConstant( g_model.xcustomSw[j].v1 - 37, g_model.xcustomSw[j].v2, &g_model ) ;
              stringTelemetryChannel( telText, g_model.xcustomSw[j].v1 - 37, value, &g_model ) ;
							//sprintf( telText, "%d", value ) ;
        			cswitchTlabel[i]->setText(telText);
						}
            break;
	        case (CS_VBOOL):
            g_model.xcustomSw[j].v1 = getSwitchCbValue( cswitchSource1[i], eeFile->mee_type ) ;
            g_model.xcustomSw[j].v2 = getSwitchCbValue( cswitchSource2[i], eeFile->mee_type ) ;
            break;
  	      case (CS_VCOMP):
            g_model.xcustomSw[j].v1 = cswitchSource1[i]->currentIndex();
            g_model.xcustomSw[j].v2 = cswitchSource2[i]->currentIndex();
            break;
    	    case (CS_TIMER):
            g_model.xcustomSw[j].v2 = cswitchOffset[i]->value()-1;
            g_model.xcustomSw[j].v1 = cswitchOffset0[i]->value()-1;
            break;
      	  default:
            break;
        }
			}
			else if ( i < NUM_CSW )
			{
				g_model.customSw[i].andsw = cswitchAndSwitch[i]->currentIndex();
        if(chAr[i])
        {
            g_model.customSw[i].v1 = 0;
            g_model.customSw[i].v2 = 0;
            setSwitchWidgetVisibility(i);
        }
        switch(CS_STATE( g_model.customSw[i].func, g_model.modelVersion))
        {
	        case (CS_VOFS):
            g_model.customSw[i].v1 = cswitchSource1[i]->currentIndex();
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
            g_model.customSw[i].v1 = getSwitchCbValue( cswitchSource1[i], eeFile->mee_type ) ;
            g_model.customSw[i].v2 = getSwitchCbValue( cswitchSource2[i], eeFile->mee_type ) ;
            break;
  	      case (CS_VCOMP):
            g_model.customSw[i].v1 = cswitchSource1[i]->currentIndex();
            g_model.customSw[i].v2 = cswitchSource2[i]->currentIndex();
            break;
    	    case (CS_TIMER):
            g_model.customSw[i].v2 = cswitchOffset[i]->value()-1;
            g_model.customSw[i].v1 = cswitchOffset0[i]->value()-1;
            break;
      	  default:
            break;
        }
			}
        
    }
#else
    for(int i=0; i<NUM_CSW+EXTRA_CSW; i++)
    {
			g_model.customSw[i].andsw = getxAndSwitchCbValue( cswitchAndSwitch[i], 1 ) ;
      if(chAr[i])
      {
          g_model.customSw[i].v1 = 0;
          g_model.customSw[i].v2 = 0;
          setSwitchWidgetVisibility(i);
      }
      switch(CS_STATE( g_model.customSw[i].func, g_model.modelVersion))
      {
	      case (CS_VOFS):
          g_model.customSw[i].v1 = cswitchSource1[i]->currentIndex();
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
          g_model.customSw[i].v1 = getSwitchCbValue( cswitchSource1[i], 1 ) ;
          g_model.customSw[i].v2 = getSwitchCbValue( cswitchSource2[i], 1 ) ;
          break;
  	    case (CS_VCOMP):
          g_model.customSw[i].v1 = cswitchSource1[i]->currentIndex();
          g_model.customSw[i].v2 = cswitchSource2[i]->currentIndex();
          break;
    	  case (CS_TIMER):
          g_model.customSw[i].v2 = cswitchOffset[i]->value()-1;
          g_model.customSw[i].v1 = cswitchOffset0[i]->value()-1;
          break;
      	default:
          break;
      }
		}

#endif

    for(int i=0; i<NUM_CSW+EXTRA_CSW; i++)
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
						if ( throttleReversed( &g_eeGeneral, &g_model ) )
                ui->slider_S3->setInvertedAppearance(true);
            break;
        case (1):
            ui->Label_S1->setText("RUD");
            ui->Label_S2->setText("THR");
            ui->Label_S3->setText("ELE");
            ui->Label_S4->setText("AIL");
						if ( throttleReversed( &g_eeGeneral, &g_model ) )
                ui->slider_S2->setInvertedAppearance(true);
            break;
        case (2):
            ui->Label_S1->setText("AIL");
            ui->Label_S2->setText("ELE");
            ui->Label_S3->setText("THR");
            ui->Label_S4->setText("RUD");
						if ( throttleReversed( &g_eeGeneral, &g_model ) )
                ui->slider_S3->setInvertedAppearance(true);
            break;
        case (3):
            ui->Label_S1->setText("AIL");
            ui->Label_S2->setText("THR");
            ui->Label_S3->setText("ELE");
            ui->Label_S4->setText("RUD");
						if ( throttleReversed( &g_eeGeneral, &g_model ) )
                ui->slider_S2->setInvertedAppearance(true);
            break;
    }

}

void ModelEdit::tabGvar()
{
		posb[0] = ui->Sc1OffsetSB ;
		posb[1] = ui->Sc2OffsetSB ;
		posb[2] = ui->Sc3OffsetSB ;
		posb[3] = ui->Sc4OffsetSB ;

		pmsb[0] = ui->Sc1MultSB ;
		pmsb[1] = ui->Sc2MultSB ;
		pmsb[2] = ui->Sc3MultSB ;
		pmsb[3] = ui->Sc4MultSB ;

		pdivsb[0] = ui->Sc1DivSB ;
		pdivsb[1] = ui->Sc2DivSB ;
		pdivsb[2] = ui->Sc3DivSB ;
		pdivsb[3] = ui->Sc4DivSB ;
		
		pdpsb[0] = ui->Sc1DecimalsSB ;
		pdpsb[1] = ui->Sc2DecimalsSB ;
		pdpsb[2] = ui->Sc3DecimalsSB ;
		pdpsb[3] = ui->Sc4DecimalsSB ;

		pucb[0] = ui->Sc1UnitsCB ;
		pucb[1] = ui->Sc2UnitsCB ;
		pucb[2] = ui->Sc3UnitsCB ;
		pucb[3] = ui->Sc4UnitsCB ;

		psgncb[0] = ui->Sc1SignCB ;
		psgncb[1] = ui->Sc2SignCB ;
		psgncb[2] = ui->Sc3SignCB ;
		psgncb[3] = ui->Sc4SignCB ;

		poffcb[0] = ui->Sc1OffAtCB ;
		poffcb[1] = ui->Sc2OffAtCB ;
		poffcb[2] = ui->Sc3OffAtCB ;
		poffcb[3] = ui->Sc4OffAtCB ;
		
		psrccb[0] = ui->Sc1SrcCB ;
		psrccb[1] = ui->Sc2SrcCB ;
		psrccb[2] = ui->Sc3SrcCB ;
		psrccb[3] = ui->Sc4SrcCB ;

		psname[0] = ui->SC1Name ;
		psname[1] = ui->SC2Name ;
		psname[2] = ui->SC3Name ;
		psname[3] = ui->SC4Name ;
		 
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
      populateSourceCB(psrccb[i],g_eeGeneral.stickMode,1,g_model.Scalers[i].source,g_model.modelVersion ) ;
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
	
		populateGvarCB( ui->Gvar1CB, g_model.gvars[0].gvsource, eeFile->mee_type ) ;
    populateGvarCB( ui->Gvar2CB, g_model.gvars[1].gvsource, eeFile->mee_type ) ;
    populateGvarCB( ui->Gvar3CB, g_model.gvars[2].gvsource, eeFile->mee_type ) ;
    populateGvarCB( ui->Gvar4CB, g_model.gvars[3].gvsource, eeFile->mee_type ) ;
    populateGvarCB( ui->Gvar5CB, g_model.gvars[4].gvsource, eeFile->mee_type ) ;
    populateGvarCB( ui->Gvar6CB, g_model.gvars[5].gvsource, eeFile->mee_type ) ;
    populateGvarCB( ui->Gvar7CB, g_model.gvars[6].gvsource, eeFile->mee_type ) ;
    
		ui->Gv1SB->setValue(g_model.gvars[0].gvar);
    ui->Gv2SB->setValue(g_model.gvars[1].gvar);
    ui->Gv3SB->setValue(g_model.gvars[2].gvar);
    ui->Gv4SB->setValue(g_model.gvars[3].gvar);
    ui->Gv5SB->setValue(g_model.gvars[4].gvar);
    ui->Gv6SB->setValue(g_model.gvars[5].gvar);
    ui->Gv7SB->setValue(g_model.gvars[6].gvar);

#ifndef V2
    populateSwitchCB(ui->GvSw1CB,g_model.gvswitch[0], eeFile->mee_type);
    populateSwitchCB(ui->GvSw2CB,g_model.gvswitch[1], eeFile->mee_type);
    populateSwitchCB(ui->GvSw3CB,g_model.gvswitch[2], eeFile->mee_type);
    populateSwitchCB(ui->GvSw4CB,g_model.gvswitch[3], eeFile->mee_type);
    populateSwitchCB(ui->GvSw5CB,g_model.gvswitch[4], eeFile->mee_type);
    populateSwitchCB(ui->GvSw6CB,g_model.gvswitch[5], eeFile->mee_type);
    populateSwitchCB(ui->GvSw7CB,g_model.gvswitch[6], eeFile->mee_type);
#else
    populateSwitchCB(ui->GvSw1CB,g_model.gvars[0].gvswitch, eeFile->mee_type);
    populateSwitchCB(ui->GvSw2CB,g_model.gvars[1].gvswitch, eeFile->mee_type);
    populateSwitchCB(ui->GvSw3CB,g_model.gvars[2].gvswitch, eeFile->mee_type);
    populateSwitchCB(ui->GvSw4CB,g_model.gvars[3].gvswitch, eeFile->mee_type);
    populateSwitchCB(ui->GvSw5CB,g_model.gvars[4].gvswitch, eeFile->mee_type);
    populateSwitchCB(ui->GvSw6CB,g_model.gvars[5].gvswitch, eeFile->mee_type);
    populateSwitchCB(ui->GvSw7CB,g_model.gvars[6].gvswitch, eeFile->mee_type);
#endif

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

}

void ModelEdit::GvarEdited()
{
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
	
//		int limit = MAX_DRSWITCH ;
//#ifndef SKY
//	  if ( eeFile->mee_type )
//		{
//    	limit += EXTRA_CSW ;
//		}
//#endif
#ifndef V2
    g_model.gvswitch[0] = getSwitchCbValue( ui->GvSw1CB, eeFile->mee_type ) ;
		g_model.gvswitch[1] = getSwitchCbValue( ui->GvSw2CB, eeFile->mee_type ) ;
		g_model.gvswitch[2] = getSwitchCbValue( ui->GvSw3CB, eeFile->mee_type ) ;
		g_model.gvswitch[3] = getSwitchCbValue( ui->GvSw4CB, eeFile->mee_type ) ;
		g_model.gvswitch[4] = getSwitchCbValue( ui->GvSw5CB, eeFile->mee_type ) ;
		g_model.gvswitch[5] = getSwitchCbValue( ui->GvSw6CB, eeFile->mee_type ) ;
		g_model.gvswitch[6] = getSwitchCbValue( ui->GvSw7CB, eeFile->mee_type ) ;
#else
    g_model.gvars[0].gvswitch = getSwitchCbValue( ui->GvSw1CB, eeFile->mee_type ) ;
		g_model.gvars[1].gvswitch = getSwitchCbValue( ui->GvSw2CB, eeFile->mee_type ) ;
		g_model.gvars[2].gvswitch = getSwitchCbValue( ui->GvSw3CB, eeFile->mee_type ) ;
		g_model.gvars[3].gvswitch = getSwitchCbValue( ui->GvSw4CB, eeFile->mee_type ) ;
		g_model.gvars[4].gvswitch = getSwitchCbValue( ui->GvSw5CB, eeFile->mee_type ) ;
		g_model.gvars[5].gvswitch = getSwitchCbValue( ui->GvSw6CB, eeFile->mee_type ) ;
		g_model.gvars[6].gvswitch = getSwitchCbValue( ui->GvSw7CB, eeFile->mee_type ) ;
#endif

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
			g_model.Scalers[i].source = psrccb[i]->currentIndex() ;
      textUpdate( psname[i], (char *)g_model.Scalers[i].name, 4 ) ;
		}

		updateSettings();
}


void ModelEdit::tabFrsky()
{
#ifndef V2
    populateTelItemsCB( ui->Ct1, 0, g_model.CustomDisplayIndex[0] ) ;
    populateTelItemsCB( ui->Ct2, 0, g_model.CustomDisplayIndex[1] ) ;
    populateTelItemsCB( ui->Ct3, 0, g_model.CustomDisplayIndex[2] ) ;
    populateTelItemsCB( ui->Ct4, 0, g_model.CustomDisplayIndex[3] ) ;
    populateTelItemsCB( ui->Ct5, 0, g_model.CustomDisplayIndex[4] ) ;
    populateTelItemsCB( ui->Ct6, 0, g_model.CustomDisplayIndex[5] ) ;
#else
    populateTelItemsCB( ui->Ct1, 0, g_model.CustomDisplayIndex[0][0] ) ;
    populateTelItemsCB( ui->Ct2, 0, g_model.CustomDisplayIndex[0][1] ) ;
    populateTelItemsCB( ui->Ct3, 0, g_model.CustomDisplayIndex[0][2] ) ;
    populateTelItemsCB( ui->Ct4, 0, g_model.CustomDisplayIndex[0][3] ) ;
    populateTelItemsCB( ui->Ct5, 0, g_model.CustomDisplayIndex[0][4] ) ;
    populateTelItemsCB( ui->Ct6, 0, g_model.CustomDisplayIndex[0][5] ) ;
#endif

    ui->frsky_ratio_0->setValue(g_model.frsky.channels[0].ratio);
#ifndef V2
    ui->frsky_type_0->setCurrentIndex(g_model.frsky.channels[0].type);
#endif
    ui->frsky_ratio_1->setValue(g_model.frsky.channels[1].ratio);
#ifndef V2
    ui->frsky_type_1->setCurrentIndex(g_model.frsky.channels[1].type);
#endif

#ifndef V2
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
#endif

    ui->GpsAltMain->setChecked(g_model.FrSkyGpsAlt);
    ui->HubComboBox->setCurrentIndex(g_model.FrSkyUsrProto);
    ui->UnitsComboBox->setCurrentIndex(g_model.FrSkyImperial);
    ui->BladesSpinBox->setValue(g_model.numBlades);
    ui->FASoffsetSB->setValue( (double)g_model.frsky.FASoffset/10 + 0.049) ;

    populateSwitchCB(ui->VarioSwitchCB, g_model.varioData.swtch,eeFile->mee_type ) ;
    ui->VarioSourceCB->setCurrentIndex( g_model.varioData.varioSource ) ;
    ui->VarioSensitivitySB->setValue( g_model.varioData.param ) ;
    ui->SinkTonesOff->setChecked(g_model.varioData.sinkTonesOff);

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
		connect(ui->HubComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect(ui->UnitsComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect(ui->BladesSpinBox,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
    
		connect( ui->Ct1,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct2,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct3,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct4,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct5,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->Ct6,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));

		connect( ui->FASoffsetSB,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
		
		connect( ui->VarioSensitivitySB,SIGNAL(editingFinished()),this,SLOT(FrSkyEdited()));
		connect( ui->VarioSourceCB,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->VarioSwitchCB,SIGNAL(currentIndexChanged(int)),this,SLOT(FrSkyEdited()));
		connect( ui->SinkTonesOff,SIGNAL(stateChanged(int)),this,SLOT(FrSkyEdited()));
}

void ModelEdit::FrSkyEdited()
{
    g_model.frsky.channels[0].ratio = ui->frsky_ratio_0->value();
    g_model.frsky.channels[1].ratio = ui->frsky_ratio_1->value();
#ifndef V2
    g_model.frsky.channels[0].type  = ui->frsky_type_0->currentIndex();
    g_model.frsky.channels[1].type  = ui->frsky_type_1->currentIndex();
#endif

#ifndef V2
    g_model.frsky.channels[0].alarms_value[0] = ui->frsky_val_0_0->value();
    g_model.frsky.channels[0].alarms_value[1] = ui->frsky_val_0_1->value();
    g_model.frsky.channels[1].alarms_value[0] = ui->frsky_val_1_0->value();
    g_model.frsky.channels[1].alarms_value[1] = ui->frsky_val_1_1->value();

    g_model.frsky.channels[0].alarms_level = (ui->frsky_level_0_0->currentIndex() & 3) + ((ui->frsky_level_0_1->currentIndex() & 3) << 2);
    g_model.frsky.channels[1].alarms_level = (ui->frsky_level_1_0->currentIndex() & 3) + ((ui->frsky_level_1_1->currentIndex() & 3) << 2);

    g_model.frsky.channels[0].alarms_greater = (ui->frsky_gr_0_0->currentIndex() & 1) + ((ui->frsky_gr_0_1->currentIndex() & 1) << 1);
    g_model.frsky.channels[1].alarms_greater = (ui->frsky_gr_1_1->currentIndex() & 1) + ((ui->frsky_gr_1_1->currentIndex() & 1) << 1);
#endif

    g_model.FrSkyGpsAlt = ui->GpsAltMain->isChecked();
    g_model.FrSkyUsrProto = ui->HubComboBox->currentIndex();
    g_model.FrSkyImperial = ui->UnitsComboBox->currentIndex();
    g_model.numBlades = ui->BladesSpinBox->value() ;

#ifndef V2
    g_model.CustomDisplayIndex[0] = ui->Ct1->currentIndex() ;
		g_model.CustomDisplayIndex[1] = ui->Ct2->currentIndex() ;
		g_model.CustomDisplayIndex[2] = ui->Ct3->currentIndex() ;
		g_model.CustomDisplayIndex[3] = ui->Ct4->currentIndex() ;
		g_model.CustomDisplayIndex[4] = ui->Ct5->currentIndex() ;
		g_model.CustomDisplayIndex[5] = ui->Ct6->currentIndex() ;
#else
    g_model.CustomDisplayIndex[0][0] = ui->Ct1->currentIndex() ;
		g_model.CustomDisplayIndex[0][1] = ui->Ct2->currentIndex() ;
		g_model.CustomDisplayIndex[0][2] = ui->Ct3->currentIndex() ;
		g_model.CustomDisplayIndex[0][3] = ui->Ct4->currentIndex() ;
		g_model.CustomDisplayIndex[0][4] = ui->Ct5->currentIndex() ;
		g_model.CustomDisplayIndex[0][5] = ui->Ct6->currentIndex() ;
#endif

		g_model.frsky.FASoffset = ui->FASoffsetSB->value() * 10 + 0.49 ;
    
	int limit = MAX_DRSWITCH ;
#ifndef SKY
  if ( eeFile->mee_type )
	{
   	limit += EXTRA_CSW ;
	}
#endif
		g_model.varioData.swtch = getSwitchCbValue( ui->VarioSwitchCB, eeFile->mee_type ) ;
		g_model.varioData.varioSource = ui->VarioSourceCB->currentIndex() ;
		g_model.varioData.param = ui->VarioSensitivitySB->value() ;
    g_model.varioData.sinkTonesOff = ui->SinkTonesOff->isChecked();
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
    const char *c = ui->modelNameLE->text().left(10).toLatin1();
    strcpy((char*)&g_model.name,c);
//    memcpy((char*)&g_model.name,c,sizeof(g_model.name));
//		strcpy((char*)&g_model.name,c);
//    g_model.mdVers = temp;  //in case strcpy overruns
    for(int i=0; i<10; i++) if(!g_model.name[i]) g_model.name[i] = ' ';
    updateSettings();

}

void ModelEdit::on_throttleReversedChkB_stateChanged( int )
{
  g_model.throttleReversed = ui->throttleReversedChkB->isChecked() ? 1 : 0 ;
  updateSettings() ;
}

void ModelEdit::on_timerModeCB_currentIndexChanged(int index)
{
//		int num_options = TMR_NUM_OPTION ;
//    if ( eeFile->mee_type )
//		{
//			num_options += EXTRA_CSW * 2 ;
//		}
	
//    g_model.tmrMode = index-num_options;
#ifndef V2
    g_model.tmrMode = index ;
#else
    g_model.timer[0].tmrModeA = index ;
#endif
    updateSettings();
}

void ModelEdit::on_throttleOffCB_currentIndexChanged(int index)
{
	g_model.throttleIdle = index ;
	updateSettings() ;
}

void ModelEdit::on_timerModeBCB_currentIndexChanged(int index)
{
	(void) index ;
#ifndef V2
  g_model.tmrModeB = getTimerSwitchCbValue( ui->timerModeBCB, eeFile->mee_type ) ;
#else
  g_model.timer[0].tmrModeB = getTimerSwitchCbValue( ui->timerModeBCB, eeFile->mee_type ) ;
#endif
  updateSettings();
}

void ModelEdit::on_timerDirCB_currentIndexChanged(int index)
{
#ifndef V2
    g_model.tmrDir = index;
#else
    g_model.timer[0].tmrDir = index;
#endif
    updateSettings();
}

void ModelEdit::on_timerResetCB_currentIndexChanged(int index)
{
//	int limit = MAX_DRSWITCH-1 ;
//#ifndef SKY
//  if ( eeFile->mee_type )
//	{
//   	limit += EXTRA_CSW ;
//	}
//#endif
	
#ifndef V2
  g_model.timer1RstSw = getTimerSwitchCbValue( ui->timerResetCB, eeFile->mee_type ) ;
#else
  g_model.timer[0].tmrRstSw = getTimerSwitchCbValue( ui->timerResetCB, eeFile->mee_type ) ;
#endif
  updateSettings() ;
}

void ModelEdit::on_timer2ModeCB_currentIndexChanged(int index)
{
//		int num_options = TMR_NUM_OPTION ;
//    if ( eeFile->mee_type )
//		{
//			num_options += EXTRA_CSW * 2 ;
//		}
	
//    g_model.tmr2Mode = index-num_options;
#ifndef V2
    g_model.tmr2Mode = index ;
#else
    g_model.timer[1].tmrModeA = index ;
#endif
    updateSettings();
}

void ModelEdit::on_timer2ModeBCB_currentIndexChanged(int index)
{
	(void) index ;
#ifndef V2
  g_model.tmr2ModeB = getTimerSwitchCbValue( ui->timer2ModeBCB, eeFile->mee_type ) ;
#else
  g_model.timer[1].tmrModeB = getTimerSwitchCbValue( ui->timer2ModeBCB, eeFile->mee_type ) ;
#endif
  updateSettings();
}

void ModelEdit::on_timer2DirCB_currentIndexChanged(int index)
{
#ifndef V2
    g_model.tmr2Dir = index;
#else
    g_model.timer[1].tmrDir = index;
#endif
    updateSettings();
}

void ModelEdit::on_timer2ResetCB_currentIndexChanged(int index)
{
//	int limit = MAX_DRSWITCH-1 ;
//#ifndef SKY
//  if ( eeFile->mee_type )
//	{
//   	limit += EXTRA_CSW ;
//	}
//#endif
	
#ifndef V2
  g_model.timer2RstSw = getTimerSwitchCbValue( ui->timer2ResetCB, eeFile->mee_type ) ;
#else
  g_model.timer[1].tmrRstSw = getTimerSwitchCbValue( ui->timer2ResetCB, eeFile->mee_type ) ;
#endif
  updateSettings() ;
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
    g_model.trimSw = getSwitchCbValue( ui->trimSWCB, eeFile->mee_type ) ;
    updateSettings();
}

void ModelEdit::on_pulsePolCB_currentIndexChanged(int index)
{
    g_model.pulsePol = index;
    updateSettings();
}

void ModelEdit::on_protocolCB_currentIndexChanged(int index)
{
    if(protocolEditLock) return;
    g_model.protocol = index;
    g_model.ppmNCH = 0;

    setProtocolBoxes();

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

void ModelEdit::on_timerValTE_editingFinished()
{
#ifndef V2
    g_model.tmrVal = ui->timerValTE->time().minute()*60 + ui->timerValTE->time().second();
#else
    g_model.timer[0].tmrVal = ui->timerValTE->time().minute()*60 + ui->timerValTE->time().second();
#endif
    updateSettings();
}

void ModelEdit::on_timer2ValTE_editingFinished()
{
#ifndef V2
    g_model.tmr2Val = ui->timer2ValTE->time().minute()*60 + ui->timer2ValTE->time().second();
#else
    g_model.timer[1].tmrVal = ui->timer2ValTE->time().minute()*60 + ui->timer2ValTE->time().second();
#endif
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

void ModelEdit::on_DSM_Type_currentIndexChanged(int index)
{
    if(protocolEditLock) return;

    g_model.sub_protocol = index;
    updateSettings();
}

void ModelEdit::on_SubProtocolCB_currentIndexChanged(int index)
{
    if(protocolEditLock) return;

    g_model.sub_protocol = index;
    setProtocolBoxes();
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

    g_model.ppmNCH = ui->pxxRxNum->value();
    updateSettings();
}

void ModelEdit::on_PPM1stChan_editingFinished()
{
    if(protocolEditLock) return;
    g_model.ppmStart = ui->PPM1stChan->value()-1 ;
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

//void ModelEdit::on_T2ThrTrgChkB_toggled(bool checked)
//{
//    g_model.t2throttle = checked;
//    updateSettings();
//}

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

void ModelEdit::on_useCustomStickNamesChkb_toggled(bool checked)
{
	g_model.useCustomStickNames = checked ;
  updateSettings() ;
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


void ModelEdit::getModelSwitchDefPos(int i, bool val)
{
    if(val)
        g_model.switchWarningStates |= (1<<(i));
    else
        g_model.switchWarningStates &= ~(1<<(i));
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

    g_model.switchWarningStates &= ~(0x30<<1); //turn off ID1/2
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

    g_model.switchWarningStates &= ~(0x28<<1); //turn off ID0/2
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

    g_model.switchWarningStates &= ~(0x18<<1); //turn off ID1/2
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
    if(idx<0 || idx>=MAX_MIXERS) return false;
    if(g_model.mixData[MAX_MIXERS-1].destCh) return false; //if last mixer isn't empty - can't add more

    int i = g_model.mixData[idx].destCh;
    memmove(&g_model.mixData[idx+1],&g_model.mixData[idx],
            (MAX_MIXERS-(idx+1))*sizeof(MixData) );
    memset(&g_model.mixData[idx],0,sizeof(MixData));
    g_model.mixData[idx].destCh = i;
    g_model.mixData[idx].weight = 100;
#ifndef V2
		g_model.mixData[idx].lateOffset = 1 ;
#endif

    for(int j=(MAX_MIXERS-1); j>idx; j--)
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
            (MAX_MIXERS-(index+1))*sizeof(MixData));
  memset(&g_model.mixData[MAX_MIXERS-1],0,sizeof(MixData));

  for(int j=index; j<(MAX_MIXERS-1); j++)
  {
      mixNotes[j].clear();
      mixNotes[j].append(mixNotes[j+1]);
  }
  mixNotes[MAX_MIXERS-1].clear();
}

void ModelEdit::gm_openMix(int index)
{
    if(index<0 || index>=MAX_MIXERS) return;

    MixData mixd;
    memcpy(&mixd,&g_model.mixData[index],sizeof(MixData));

    updateSettings();
    tabMixes();

    QString comment = mixNotes[index];

#ifndef V2
    MixerDialog *g = new MixerDialog(this,&mixd,g_eeGeneral.stickMode, &comment, g_model.modelVersion, eeFile->mee_type, g_model.mixTime ) ;
#else
    MixerDialog *g = new MixerDialog(this,&mixd,g_eeGeneral.stickMode, &comment, g_model.modelVersion, eeFile->mee_type, 0 ) ;
#endif
    if(g->exec())
    {
        memcpy(&g_model.mixData[index],&mixd,sizeof(MixData));

        mixNotes[index] = comment;

        updateSettings();
        tabMixes();
    }
}

int ModelEdit::getMixerIndex(int dch)
{
    int i = 0;
    while ((g_model.mixData[i].destCh<=dch) && (g_model.mixData[i].destCh) && (i<MAX_MIXERS)) i++;
    if(i==MAX_MIXERS) return -1;
    return i;
}

void ModelEdit::on_VoiceAlarmList_doubleClicked( QModelIndex index )
{
  VoiceAlarmDialog *dlg = new VoiceAlarmDialog( this, &g_model.vad[index.row()], eeFile->mee_type, g_eeGeneral.stickMode, g_model.modelVersion, &g_model ) ;
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
        if(idx>=0 && idx<MAX_MIXERS) list << idx;
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
        mxData.append((char*)&g_model.mixData[idx],sizeof(MixData));

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
            if(idx==MAX_MIXERS) break;

            if(!gm_insertMix(idx))
                break; //memory full - can't add any more
            MixData *md = &g_model.mixData[idx];
            memcpy(md,mxData.mid(i,sizeof(MixData)).constData(),sizeof(MixData));
            md->destCh = dch;

            i     += sizeof(MixData);
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
    MixData &src=g_model.mixData[idx];
    if(idx==0 && !dir)
		{
      if (src.destCh>1)
			{
        src.destCh--;
			}
			return idx ;
		}
		
    if(idx>MAX_MIXERS || (idx==MAX_MIXERS && dir)) return idx;

    int tdx = dir ? idx+1 : idx-1;
    MixData &tgt=g_model.mixData[tdx];

    if((src.destCh==0) || (src.destCh>NUM_CHNOUT) || (tgt.destCh>NUM_CHNOUT)) return idx;

    if(tgt.destCh!=src.destCh) {
        if ((dir)  && (src.destCh<NUM_CHNOUT)) src.destCh++;
        if ((!dir) && (src.destCh>0))          src.destCh--;
        return idx;
    }

    //flip between idx and tgt
    MixData temp;
    memcpy(&temp,&src,sizeof(MixData));
    memcpy(&src,&tgt,sizeof(MixData));
    memcpy(&tgt,&temp,sizeof(MixData));

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

    ModelData gm;
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
    sdptr->setType( eeFile->mee_type ) ;
    sdptr->loadParams(gg,gm);
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
      for( LimitData *ld = &g_model.limitData[0] ; ld < &g_model.limitData[NUM_CHNOUT] ; ld += 1 )
      {
        if (ld->min < 0) ld->min = 0;
        if (ld->max > 0) ld->max = 0;
      }
		}
    updateSettings();
}

void ModelEdit::on_fastMixDelayCB_toggled(bool checked)
{
#ifndef V2
    g_model.mixTime = checked ;
#endif
    tabMixes() ;
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
      TemplateDialog *tem = new TemplateDialog(this, &g_model, &templateValues, eeFile->mee_type );
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


MixData* ModelEdit::setDest(uint8_t dch)
{
    uint8_t i = 0;
    while ((g_model.mixData[i].destCh<=dch) && (g_model.mixData[i].destCh) && (i<MAX_MIXERS)) i++;
    if(i==MAX_MIXERS) return &g_model.mixData[0];

    memmove(&g_model.mixData[i+1],&g_model.mixData[i],
            (MAX_MIXERS-(i+1))*sizeof(MixData) );
    memset(&g_model.mixData[i],0,sizeof(MixData));
    g_model.mixData[i].destCh = dch;
#ifndef V2
		g_model.mixData[i].lateOffset = 1 ;
#endif
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
    g_model.customSw[idx-1].v1   = v1;
    g_model.customSw[idx-1].v2   = v2;
    g_model.customSw[idx-1].andsw   = 0;
}

void ModelEdit::applyTemplate(uint8_t idx)
{
    int8_t heli_ar1[] = {-100, -20, 30, 70, 90};
    int8_t heli_ar2[] = {80, 70, 60, 70, 100};
    int8_t heli_ar3[] = {100, 90, 80, 90, 100};
    int8_t heli_ar4[] = {-30,  -15, 0, 50, 100};
    int8_t heli_ar5[] = {-100, -50, 0, 50, 100};


    MixData *md = &g_model.mixData[0];

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
        md=setDest(ICC(STK_THR));  md->srcRaw=MIX_MAX;  md->weight=-100;  md->swtch=DSW_THR;  md->mltpx=MLTPX_REP;
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

#ifndef V2
      SafetySwData *sd = &g_model.safetySw[ICC(STK_THR)-1] ;
			sd->opt.ss.mode = 3 ;
			sd->opt.ss.swtch = DSW_THR ;
			sd->opt.ss.val = g_model.throttleIdle ? 0 : -100 ;
			
			EditedNesting = 1  ;
      populateSafetySwitchCB(safetySwitchSwtch[ICC(STK_THR)-1],sd->opt.ss.mode,sd->opt.ss.swtch, eeFile->mee_type);
			safetySwitchType[ICC(STK_THR)-1]->setCurrentIndex( sd->opt.ss.mode ) ;
			safetySwitchValue[ICC(STK_THR)-1]->setValue( sd->opt.ss.val ) ;
#endif
      setSafetyWidgetVisibility(ICC(STK_THR)-1) ;
			EditedNesting = 0  ;
//      updateSwitchesTab();
    }

    //V-Tail
    if(idx==j++)
    {
        clearMixes();
        md=setDest(ICC(RUD_STICK+1));  md->srcRaw=CM(STK_RUD,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 100;
        md=setDest(ICC(RUD_STICK+1));  md->srcRaw=CM(STK_ELE,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=-100;
        md=setDest(ICC(ELE_STICK+1));  md->srcRaw=CM(STK_RUD,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 100;
        md=setDest(ICC(ELE_STICK+1));  md->srcRaw=CM(STK_ELE,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 100;
    }

    //Elevon\\Delta
    if(idx==j++)
    {
        clearMixes();
        md=setDest(ICC(ELE_STICK+1));  md->srcRaw=CM(STK_ELE,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 100;
        md=setDest(ICC(ELE_STICK+1));  md->srcRaw=CM(STK_AIL,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 100;
        md=setDest(ICC(AIL_STICK+1));  md->srcRaw=CM(STK_ELE,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight= 100;
        md=setDest(ICC(AIL_STICK+1));  md->srcRaw=CM(STK_AIL,g_model.modelVersion,g_eeGeneral.stickMode);  md->weight=-100;
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




