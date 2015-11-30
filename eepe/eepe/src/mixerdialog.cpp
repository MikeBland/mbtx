#include "mixerdialog.h"
#include "ui_mixerdialog.h"
#include "pers.h"
#include "helpers.h"

MixerDialog::MixerDialog(QWidget *parent, MixData *mixdata, int stickMode, QString * comment, int modelVersion, int eepromType, int delaySpeed) :
    QDialog(parent),
    ui(new Ui::MixerDialog)
{
    ui->setupUi(this);
    md = mixdata;
		mType = eepromType ;
#ifndef V2
		delaySlowSpeed = delaySpeed ;
#else
		delaySpeed = delaySlowSpeed = md->hiResSlow ;
		ui->FixHiResLabel->setText("HiResDelay") ;
#endif
		ValuesEditLock = false ;

    this->setWindowTitle(tr("DEST -> CH%1%2").arg(md->destCh/10).arg(md->destCh%10));
    populateSourceCB(ui->sourceCB, stickMode, 0, md->srcRaw, modelVersion);
    
//		ui->sourceCB->addItem("Switch");
    
    ui->sourceCB->addItem("sIDx");
    ui->sourceCB->addItem("sTHR");
    ui->sourceCB->addItem("sRUD");
    ui->sourceCB->addItem("sELE");
    ui->sourceCB->addItem("sAIL");
    ui->sourceCB->addItem("sGEA");
    ui->sourceCB->addItem("sTRN");
		
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
    
		int x = md->srcRaw ;
		if ( x >= MIX_3POS )
		{
			if ( x == MIX_3POS )
			{
				x += md->sw23pos ;
			}
			else
			{
				x += 6 ;
			}
		}
		ui->sourceCB->setCurrentIndex(x);
		
		ui->sourceCB->removeItem(0);

		ValuesEditLock = true ;
	  if ( md->srcRaw >= 21 && md->srcRaw <= 36 )
		{
	  	ui->FMtrimChkB->setChecked(md->disableExpoDr) ;
		}
		else
		{
	  	ui->FMtrimChkB->setChecked(!md->disableExpoDr) ;
		}
		updateChannels() ;
		ValuesEditLock = false ;

//    ui->sw23posCB->addItem("sIDx");
//    ui->sw23posCB->addItem("sTHR");
//    ui->sw23posCB->addItem("sRUD");
//    ui->sw23posCB->addItem("sELE");
//    ui->sw23posCB->addItem("sAIL");
//    ui->sw23posCB->addItem("sGEA");
//    ui->sw23posCB->addItem("sTRN");
//    ui->sw23posCB->setCurrentIndex(md->sw23pos);
    

    populateSpinGVarCB( ui->weightSB, ui->weightCB, ui->weightGvChkB, md->weight, -125, 125 ) ;
    populateSpinGVarCB( ui->offsetSB, ui->offsetCB, ui->offsetGvChkB, md->sOffset, -125, 125 ) ;

    ui->trimChkB->setChecked(md->carryTrim==0);
#ifndef V2
    ui->lateOffsetChkB->setChecked(md->lateOffset);
#else
    ui->lateOffsetChkB->setChecked(md->hiResSlow);
#endif
    populateSwitchCB(ui->switchesCB,md->swtch, eepromType ) ;
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

		setSpeeds() ;

    mixCommennt = comment;
    ui->mixerComment->setPlainText(mixCommennt->trimmed());

		ui->Fm0CB->setChecked( !(md->modeControl & 1) ) ;
		ui->Fm1CB->setChecked( !(md->modeControl & 2) ) ;
		ui->Fm2CB->setChecked( !(md->modeControl & 4) ) ;
		ui->Fm3CB->setChecked( !(md->modeControl & 8) ) ;
		ui->Fm4CB->setChecked( !(md->modeControl & 16) ) ;

    valuesChanged();

    connect(ui->sourceCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
//    connect(ui->sw23posCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
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
    connect(ui->delayDownSB,SIGNAL(valueChanged( double )),this,SLOT(valuesChanged()));
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
}

MixerDialog::~MixerDialog()
{
    delete ui;
}

void MixerDialog::setSpeeds()
{
  if ( delaySlowSpeed )
	{
		ui->delayDownSB->setDecimals( 1 ) ;
		ui->delayDownSB->setSingleStep( 0.2 ) ;
		ui->delayDownSB->setMaximum ( 3.0 ) ;
		ui->delayDownSB->setValue(md->delayDown/5.0) ;
			
		ui->delayUpSB->setDecimals( 1 ) ;
		ui->delayUpSB->setSingleStep( 0.2 ) ;
		ui->delayUpSB->setMaximum ( 3.0 ) ;
    ui->delayUpSB->setValue(md->delayUp/5.0) ;
			
		ui->slowDownSB->setDecimals( 1 ) ;
		ui->slowDownSB->setSingleStep( 0.2 ) ;
		ui->slowDownSB->setMaximum ( 3.0 ) ;
    ui->slowDownSB->setValue(md->speedDown/5.0) ;
			
		ui->slowUpSB->setDecimals( 1 ) ;
		ui->slowUpSB->setSingleStep( 0.2 ) ;
		ui->slowUpSB->setMaximum ( 3.0 ) ;
    ui->slowUpSB->setValue(md->speedUp/5.0) ;
	}
	else
	{
		ui->delayDownSB->setDecimals( 0 ) ;
		ui->delayDownSB->setSingleStep( 1 ) ;
		ui->delayDownSB->setMaximum ( 15 ) ;
		ui->delayUpSB->setDecimals( 0 ) ;
		ui->slowDownSB->setDecimals( 0 ) ;
		ui->slowUpSB->setDecimals( 0 ) ;
	  ui->delayDownSB->setValue(md->delayDown) ;
    ui->delayUpSB->setValue(md->delayUp) ;
    ui->slowDownSB->setValue(md->speedDown) ;
    ui->slowUpSB->setValue(md->speedUp) ;
	}
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
  if ( md->srcRaw >= 21 && md->srcRaw <= 36 )
	{
		ui->label_expo_output->setText( "Use Output" ) ;
	  ui->FMtrimChkB->setChecked(md->disableExpoDr) ;
		if ( md->disableExpoDr )
		{
			uint32_t i ;
			for ( i = 20 ; i < 36 ; i += 1 )
			{
				ui->sourceCB->setItemText( i, QString("OP%1").arg(i-19) ) ;
			}
		}
		else
		{
			uint32_t i ;
			for ( i = 20 ; i < 36 ; i += 1 )
			{
				ui->sourceCB->setItemText( i, QString("CH%1").arg(i-19) ) ;
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
	int limit = MAX_DRSWITCH ;
	int oldSrcRaw ;

	if ( ValuesEditLock )
	{
		return ;
	}
	ValuesEditLock = true ;


#ifndef SKY
  if ( mType )
	{
    limit += EXTRA_CSW ;
	}
#endif

		int x = ui->sourceCB->currentIndex()+1 ;
		if ( x >= MIX_3POS )
		{
			if ( x >= MIX_3POS+7 )
			{
				x -= 6 ;
			}
			else
			{
		    md->sw23pos = x - (MIX_3POS) ;
				x = MIX_3POS ;
			}
		}
		 
		oldSrcRaw = md->srcRaw ;

    md->srcRaw       = x ;
//    md->sw23pos      = ui->sw23posCB->currentIndex() ;
    md->weight       = numericSpinGvarValue( ui->weightSB, ui->weightCB, ui->weightGvChkB, md->weight, 100 ) ;
    md->sOffset      = numericSpinGvarValue( ui->offsetSB, ui->offsetCB, ui->offsetGvChkB, md->sOffset, 0 ) ;
    md->carryTrim    = ui->trimChkB->checkState() ? 0 : 1;
    md->swtch        = getSwitchCbValue( ui->switchesCB, mType ) ;
    md->mixWarn      = ui->warningCB->currentIndex();
    md->mltpx        = ui->mltpxCB->currentIndex();

#ifndef V2
		md->lateOffset   = ui->lateOffsetChkB->checkState() ? 1 : 0 ;
#else
		md->hiResSlow   = ui->lateOffsetChkB->checkState() ? 1 : 0 ;
    delaySlowSpeed = md->hiResSlow ;
		setSpeeds() ;
#endif
		 
		if ( delaySlowSpeed )
		{
    	md->delayDown    = (ui->delayDownSB->value()+0.1 ) * 5 ;
    	md->delayUp      = (ui->delayUpSB->value()+0.1 ) * 5 ;
    	md->speedDown    = (ui->slowDownSB->value()+0.1 ) * 5 ;
    	md->speedUp      = (ui->slowUpSB->value()+0.1 ) * 5 ;
		}
		else
		{
    	md->delayDown    = ui->delayDownSB->value()+0.1 ;
    	md->delayUp      = ui->delayUpSB->value()+0.1 ;
    	md->speedDown    = ui->slowDownSB->value()+0.1 ;
    	md->speedUp      = ui->slowUpSB->value()+0.1 ;
		}

    if ( md->srcRaw >= 21 && md->srcRaw <= 36 )
		{
			if ( oldSrcRaw >= 21 && oldSrcRaw <= 36 )
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
			if ( oldSrcRaw >= 21 && oldSrcRaw <= 36 )
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

		int j = 31 ;
		j &= ~( ui->Fm0CB->checkState() ? 1 : 0 ) ;
		j &= ~( ui->Fm1CB->checkState() ? 2 : 0 ) ;
		j &= ~( ui->Fm2CB->checkState() ? 4 : 0 ) ;
		j &= ~( ui->Fm3CB->checkState() ? 8 : 0 ) ;
		j &= ~( ui->Fm4CB->checkState() ? 16 : 0 ) ;
		md->modeControl = j ;

    ui->offset_label->setText("Offset");

//		ui->sw23posCB->setVisible( md->srcRaw == MIX_3POS ) ;
//		ui->label_sw23pos->setVisible( md->srcRaw == MIX_3POS ) ;

    mixCommennt->clear();
    mixCommennt->append(ui->mixerComment->toPlainText());
	
		 	 
		ValuesEditLock = false ;
}
