#include "mixerdialog.h"
#include "ui_mixerdialog.h"
#include "pers.h"
#include "helpers.h"

MixerDialog::MixerDialog(QWidget *parent, SKYMixData *mixdata, EEGeneral *g_eeGeneral, QString * comment, int modelVersion, int eeType) :
    QDialog(parent),
    ui(new Ui::MixerDialog)
{
    ui->setupUi(this);
    md = mixdata;
		leeType = eeType ;

    this->setWindowTitle(tr("DEST -> CH%1%2").arg(md->destCh/10).arg(md->destCh%10));
    populateSourceCB(ui->sourceCB, g_eeGeneral->stickMode, 0, md->srcRaw, modelVersion, eeType);
    ui->sourceCB->addItem("SWCH");
    ui->sourceCB->addItem("GV1 ");
    ui->sourceCB->addItem("GV2 ");
    ui->sourceCB->addItem("GV3 ");
    ui->sourceCB->addItem("GV4 ");
    ui->sourceCB->addItem("GV5 ");
    ui->sourceCB->addItem("GV6 ");
    ui->sourceCB->addItem("GV7 ");
    ui->sourceCB->addItem("THIS");
    ui->sourceCB->addItem("SC1 ");
    ui->sourceCB->addItem("SC2 ");
    ui->sourceCB->addItem("SC3 ");
    ui->sourceCB->addItem("SC4 ");
    ui->sourceCB->addItem("SC5 ");
    ui->sourceCB->addItem("SC6 ");
    ui->sourceCB->addItem("SC7 ");
    ui->sourceCB->addItem("SC8 ");
//		uint32_t value ;
//		value = md->srcRaw ;
//		if ( eeType )
//		{
//			if ( value >= EXTRA_POTS_POSITION )
//			{
//				if ( value >= EXTRA_POTS_START )
//				{
//					value -= ( EXTRA_POTS_START - EXTRA_POTS_POSITION ) ;
//				}
//				else
//				{
//					value += eeType == 2 ? 2 : NUM_EXTRA_POTS ;
//				}
//			}
//		}
//    ui->sourceCB->setCurrentIndex(value) ;
    
		ui->sourceCB->removeItem(0);

		ValuesEditLock = true ;
	  if ( md->srcRaw >= 21 && md->srcRaw <= 44 )
		{
	  	ui->FMtrimChkB->setChecked(md->disableExpoDr) ;
		}
		else
		{
	  	ui->FMtrimChkB->setChecked(!md->disableExpoDr) ;
		}
		updateChannels() ;
		ValuesEditLock = false ;

		ui->sourceSwitchCB->clear();

		if ( eeType )
		{
    	ui->sourceSwitchCB->addItem("SA");
    	ui->sourceSwitchCB->addItem("SB");
    	ui->sourceSwitchCB->addItem("SC");
    	ui->sourceSwitchCB->addItem("SD");
    	ui->sourceSwitchCB->addItem("SE");
    	ui->sourceSwitchCB->addItem("SF");
    	ui->sourceSwitchCB->addItem("SG");
    	ui->sourceSwitchCB->addItem("SH");
      if ( g_eeGeneral->analogMapping & 0x0C /*MASK_6POS*/ )
			{
    		ui->sourceSwitchCB->addItem("6P");
			}
		}
		else
		{
    	ui->sourceSwitchCB->addItem("IDx");
    	ui->sourceSwitchCB->addItem("THR");
    	ui->sourceSwitchCB->addItem("RUD");
    	ui->sourceSwitchCB->addItem("ELE");
    	ui->sourceSwitchCB->addItem("AIL");
    	ui->sourceSwitchCB->addItem("GEA");
    	ui->sourceSwitchCB->addItem("TRN");
		}
    ui->sourceSwitchCB->setCurrentIndex(md->switchSource) ;
		
		populateSpinGVarCB( ui->weightSB, ui->weightCB, ui->weightGvChkB, md->weight, -125, 125 ) ;
    populateSpinGVarCB( ui->offsetSB, ui->offsetCB, ui->offsetGvChkB, md->sOffset, -125, 125 ) ;
    
		ui->trimChkB->setChecked(md->carryTrim==0);
//    ui->FMtrimChkB->setChecked(!md->disableExpoDr);
    ui->lateOffsetChkB->setChecked(md->lateOffset);
    populateSwitchCB(ui->switchesCB,md->swtch, eeType );
    ui->warningCB->setCurrentIndex(md->mixWarn);
    ui->mltpxCB->setCurrentIndex(md->mltpx);

		int index = md->differential ;
		if ( index == 0 )
		{
			if ( md->curve <= -28 )
			{
				index = 2 ;
			}
		}
		ui->diffcurveCB->setCurrentIndex(index) ;
		
		if (md->differential)
		{
			populateSpinGVarCB( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, md->curve, -100, 100 ) ;
			ui->curveGvChkB->setVisible( true ) ;
		}
		else
		{
			if ( md->curve <= -28 )
			{
				ui->curvesCB->setVisible( false ) ;
				ui->curvesSB->setVisible( true ) ;
				ui->curvesSB->setValue( md->curve + 128 ) ;
			}
			else
			{
				populateCurvesCB(ui->curvesCB, md->curve ) ;
				ui->curvesCB->setVisible( true ) ;
				ui->curvesSB->setVisible( false ) ;
				ui->curveGvChkB->setVisible( false ) ;
      	ui->curveGvChkB->setChecked( false ) ;
			}
		}

    ui->delayDownSB->setValue((double)md->delayDown/10);
    ui->delayUpSB->setValue((double)md->delayUp/10);
    ui->slowDownSB->setValue((double)md->speedDown/10);
    ui->slowUpSB->setValue((double)md->speedUp/10);

    mixCommennt = comment;
    ui->mixerComment->setPlainText(mixCommennt->trimmed());

		ui->Fm0CB->setChecked( !(md->modeControl & 1) ) ;
		ui->Fm1CB->setChecked( !(md->modeControl & 2) ) ;
		ui->Fm2CB->setChecked( !(md->modeControl & 4) ) ;
		ui->Fm3CB->setChecked( !(md->modeControl & 8) ) ;
		ui->Fm4CB->setChecked( !(md->modeControl & 16) ) ;
		ui->Fm5CB->setChecked( !(md->modeControl & 32) ) ;
		ui->Fm6CB->setChecked( !(md->modeControl & 64) ) ;

    valuesChanged();

    connect(ui->sourceCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->sourceSwitchCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->weightSB,SIGNAL(valueChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->weightCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->weightGvChkB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->offsetSB,SIGNAL(valueChanged(int)),this,SLOT(valuesChanged()));
		connect(ui->offsetCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->offsetGvChkB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->trimChkB,SIGNAL(toggled(bool)),this,SLOT(valuesChanged()));
		connect(ui->curvesCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->curvesSB,SIGNAL(valueChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->curveGvChkB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
		connect(ui->diffcurveCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
		connect(ui->switchesCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->warningCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->mltpxCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->delayDownSB,SIGNAL(valueChanged(double)),this,SLOT(valuesChanged()));
    connect(ui->delayUpSB,SIGNAL(valueChanged(double)),this,SLOT(valuesChanged()));
    connect(ui->slowDownSB,SIGNAL(valueChanged(double)),this,SLOT(valuesChanged()));
    connect(ui->slowUpSB,SIGNAL(valueChanged(double)),this,SLOT(valuesChanged()));
    connect(ui->FMtrimChkB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
		connect(ui->mixerComment,SIGNAL(textChanged()),this,SLOT(valuesChanged()));
    connect(ui->lateOffsetChkB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->Fm0CB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->Fm1CB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->Fm2CB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->Fm3CB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->Fm4CB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->Fm5CB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
    connect(ui->Fm6CB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));
}

MixerDialog::~MixerDialog()
{
    delete ui;
}

void MixerDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void MixerDialog::updateChannels()
{
  uint32_t lowBound = leeType ? 22 : 21 ;
	if ( leeType == 2 )
	{
		lowBound = 23 ;
	}
  if ( md->srcRaw >= lowBound && md->srcRaw <= lowBound+23 )
	{
		ui->label_expo_output->setText( "Use Output" ) ;
	  ui->FMtrimChkB->setChecked(md->disableExpoDr) ;
		if ( md->disableExpoDr )
		{
			uint32_t i ;
      for ( i = lowBound-1 ; i < lowBound+23 ; i += 1 )
			{
				ui->sourceCB->setItemText( i, QString("OP%1").arg(i-(lowBound-2)) ) ;
			}
		}
		else
		{
			uint32_t i ;
      for ( i = lowBound-1 ; i < lowBound+23 ; i += 1 )
			{
				ui->sourceCB->setItemText( i, QString("CH%1").arg(i-(lowBound-2)) ) ;
			}
		}
	}
	else
	{
		ui->label_expo_output->setText( "Enable Expo/Dr" ) ;
	  ui->FMtrimChkB->setChecked(!md->disableExpoDr) ;
	}
//  ui->FMtrimChkB->setChecked(!md->disableExpoDr) ;
}

void MixerDialog::valuesChanged()
{
	int oldcurvemode ;
	int oldSrcRaw ;
    
	if ( ValuesEditLock )
	{
		return ;
	}
	ValuesEditLock = true ;
		
		oldSrcRaw = md->srcRaw ;
    uint32_t value ;
		value = ui->sourceCB->currentIndex()+1 ;
  	value = decodePots( value, leeType ) ;
		md->srcRaw       = value ;
		
		md->switchSource = ui->sourceSwitchCB->currentIndex() ;
		ui->sourceSwitchCB->setVisible( md->srcRaw == MIX_3POS ) ;
    md->weight       = numericSpinGvarValue( ui->weightSB, ui->weightCB, ui->weightGvChkB, md->weight, 100 ) ;
    md->sOffset      = numericSpinGvarValue( ui->offsetSB, ui->offsetCB, ui->offsetGvChkB, md->sOffset, 0 ) ;
    md->carryTrim    = ui->trimChkB->checkState() ? 0 : 1;
		int limit = MAX_DRSWITCH ;
		if ( leeType )
		{
			limit = MAX_XDRSWITCH ;
		}
    md->swtch        = getSwitchCbValue( ui->switchesCB, leeType ) ;
    md->mixWarn      = ui->warningCB->currentIndex();
    md->mltpx        = ui->mltpxCB->currentIndex();
    md->delayDown    = ui->delayDownSB->value()*10+0.4;
    md->delayUp      = ui->delayUpSB->value()*10+0.4;
    md->speedDown    = ui->slowDownSB->value()*10+0.4;
    md->speedUp      = ui->slowUpSB->value()*10+0.4;
    md->lateOffset   = ui->lateOffsetChkB->checkState() ? 1 : 0;

		int lowBound = leeType ? 21 : 21 ;
    if ( md->srcRaw >= lowBound && md->srcRaw <= lowBound+23 )
		{
			if ( oldSrcRaw >= lowBound && oldSrcRaw <= lowBound+23 )
			{
	    	md->disableExpoDr = ui->FMtrimChkB->checkState() ? 1 : 0 ;
				updateChannels() ;
			}
			else
			{
				updateChannels() ;
//	    	md->disableExpoDr = ui->FMtrimChkB->checkState() ? 0 : 1 ;
			}
		}
		else
		{
			if ( oldSrcRaw >= 21 && oldSrcRaw <= 44 )
			{
				updateChannels() ;
			}
			else
			{
	    	md->disableExpoDr = ui->FMtrimChkB->checkState() ? 0 : 1 ;
				updateChannels() ;
			}
		}

		oldcurvemode = md->differential ;
		if ( oldcurvemode == 0 )
		{
			if ( md->curve <= -28 )
			{
				oldcurvemode = 2 ;
			}
		}
		
		int newcurvemode = ui->diffcurveCB->currentIndex() ;
		md->differential = ( newcurvemode == 1 ) ? 1 : 0 ;
		
		if ( newcurvemode != oldcurvemode )
		{
			if (md->differential)
			{
				populateSpinGVarCB( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, 0, -100, 100 ) ;
				ui->curveGvChkB->setVisible( true ) ;
			}
			else
			{
				if ( newcurvemode == 2 )
				{
					ui->curvesCB->setVisible( false ) ;
					ui->curvesSB->setVisible( true ) ;
					ui->curvesSB->setValue( 0 ) ;
          ui->curvesSB->setMinimum( 0 ) ;
          ui->curvesSB->setMaximum( 100 ) ;
				}
				else
				{
		      ui->curveGvChkB->setChecked( false ) ;
					populateCurvesCB(ui->curvesCB, 0 ) ;
					ui->curvesSB->setVisible( false ) ;
					ui->curvesCB->setVisible( true ) ;
				}
				ui->curveGvChkB->setVisible( false ) ;
			}
			if ( newcurvemode == 2 )
			{
	    	md->curve = -128 ;
			}
			else
			{
	    	md->curve = numericSpinGvarValue( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, 0, 0 ) ;
			}
		}
		else
		{
			if (md->differential)
			{
	   		md->curve = numericSpinGvarValue( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, md->curve, 0 ) ;
			}
			else
			{
				if ( ui->diffcurveCB->currentIndex() == 2 )
				{
          md->curve = ui->curvesSB->value() - 128 ;
				}
				else
				{
	    		md->curve = ui->curvesCB->currentIndex()-16;
				}	 
			}
		}

		int j = 127 ;
		j &= ~( ui->Fm0CB->checkState() ? 1 : 0 ) ;
		j &= ~( ui->Fm1CB->checkState() ? 2 : 0 ) ;
		j &= ~( ui->Fm2CB->checkState() ? 4 : 0 ) ;
		j &= ~( ui->Fm3CB->checkState() ? 8 : 0 ) ;
		j &= ~( ui->Fm4CB->checkState() ? 16 : 0 ) ;
		j &= ~( ui->Fm5CB->checkState() ? 32 : 0 ) ;
		j &= ~( ui->Fm6CB->checkState() ? 64 : 0 ) ;
		md->modeControl = j ;

//    if(ui->FMtrimChkB->checkState())
//        ui->offset_label->setText("FmTrimVal");
//    else
//        ui->offset_label->setText("Offset");

    mixCommennt->clear();
    mixCommennt->append(ui->mixerComment->toPlainText());
		
		ValuesEditLock = false ;
}



