#include "mixerdialog.h"
#include "ui_mixerdialog.h"
#include "pers.h"
#include "file.h"
#include "helpers.h"

MixerDialog::MixerDialog(QWidget *parent, SKYMixData *mixdata, EEGeneral *g_eeGeneral, QString * comment, int modelVersion, struct t_radioData *rData ) :
    QDialog(parent),
    ui(new Ui::MixerDialog)
{
    ui->setupUi(this);
    md = mixdata;
		leeType = rData->type ;
		lextraPots = rData->extraPots ;

    this->setWindowTitle(tr("DEST -> CH%1%2").arg(md->destCh/10).arg(md->destCh%10));
		int type = leeType ;
		if ( type == RADIO_TYPE_TPLUS )
		{
			if ( rData->sub_type == 1 )
			{
				type = RADIO_TYPE_X9E ;
			}
		}
    lType = type ;

		ValuesEditLock = true ;

//		ui->spinBox->setValue(md->srcRaw);

//    ui->sourceCB->setFont(QFont("Ariel",16));

    populateSourceCB(ui->sourceCB, g_eeGeneral->stickMode, 0, md->srcRaw, modelVersion, type, lextraPots ) ;

		ui->sourceCB->addItem("SWCH");
		
//		ui->sourceCB->addItem("sIDx");
//    ui->sourceCB->addItem("sTHR");
//    ui->sourceCB->addItem("sRUD");
//    ui->sourceCB->addItem("sELE");
//    ui->sourceCB->addItem("sAIL");
//    ui->sourceCB->addItem("sGEA");
//    ui->sourceCB->addItem("sTRN");
//    ui->sourceCB->addItem("L1  ");
//    ui->sourceCB->addItem("L2  ");
//    ui->sourceCB->addItem("L3  ");
//    ui->sourceCB->addItem("L4  ");
//    ui->sourceCB->addItem("L5  ");
//    ui->sourceCB->addItem("L6  ");
//    ui->sourceCB->addItem("L7  ");
//    ui->sourceCB->addItem("L8  ");
//    ui->sourceCB->addItem("L9  ");
//		ui->sourceCB->addItem("LA  ");
//    ui->sourceCB->addItem("LB  ");
//    ui->sourceCB->addItem("LC  ");
//    ui->sourceCB->addItem("LD  ");
//    ui->sourceCB->addItem("LE  ");
//    ui->sourceCB->addItem("LF  ");
//    ui->sourceCB->addItem("LG  ");
//    ui->sourceCB->addItem("LH  ");
//    ui->sourceCB->addItem("LI  ");
//    ui->sourceCB->addItem("LJ  ");
//    ui->sourceCB->addItem("LK  ");
//    ui->sourceCB->addItem("LL  ");
//    ui->sourceCB->addItem("LM  ");
//    ui->sourceCB->addItem("LN  ");
//    ui->sourceCB->addItem("LO  ");

//		if ( g_eeGeneral->analogMapping & 0x1C /*MASK_6POS*/ )
//		{
//	    ui->sourceCB->addItem("6POS");
//    }

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
    ui->sourceCB->addItem("PPM9");
    ui->sourceCB->addItem("PPM10");
    ui->sourceCB->addItem("PPM11");
    ui->sourceCB->addItem("PPM12");
    ui->sourceCB->addItem("PPM13");
    ui->sourceCB->addItem("PPM14");
    ui->sourceCB->addItem("PPM15");
    ui->sourceCB->addItem("PPM16");
#ifdef EXTRA_SKYCHANNELS
    ui->sourceCB->addItem("CH25");
    ui->sourceCB->addItem("CH26");
    ui->sourceCB->addItem("CH27");
    ui->sourceCB->addItem("CH28");
    ui->sourceCB->addItem("CH29");
    ui->sourceCB->addItem("CH30");
    ui->sourceCB->addItem("CH31");
    ui->sourceCB->addItem("CH32");
#endif
    ui->sourceCB->addItem("Rtm");
    ui->sourceCB->addItem("Etm");
    ui->sourceCB->addItem("Ttm");
    ui->sourceCB->addItem("Atm");
//		int x = md->srcRaw ;
//		if ( x >= MIX_3POS )
//		{
//			if ( x == MIX_3POS )
//			{
//				x += md->switchSource ;
//			}
//			else
//			{
//				x += 6 + 24 ;
//		 		if ( ( (eeType == 1 ) || ( eeType == 2 ) ) )	// Taranis
//				{
//					x += 1 ;
//				}
//	      if ( g_eeGeneral->analogMapping & 0x1C /*MASK_6POS*/ )
//				{
//					x += 1 ;
//				}
//			}
//		}


//		ui->sourceCB->setCurrentIndex(x);
#ifdef SKY    
		int value ;
		value = md->srcRaw ;
		if ( ( type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_X9E ) )
		{
			if ( value >= EXTRA_POTS_POSITION )
			{
				if ( value >= EXTRA_POTS_START )
				{
					value -= ( EXTRA_POTS_START - EXTRA_POTS_POSITION ) ;
				}
				else
				{
					value += type == RADIO_TYPE_TPLUS ? 2 : type == RADIO_TYPE_X9E ? 3 : NUM_EXTRA_POTS ;
				}
			}
		}
    if ( ( type == RADIO_TYPE_SKY ) || ( type == RADIO_TYPE_9XTREME ) )
		{
			if ( value >= EXTRA_POTS_POSITION )
			{
				if ( value >= EXTRA_POTS_START )
				{
					value -= ( EXTRA_POTS_START - EXTRA_POTS_POSITION ) ;
				}
				else
				{
					value += lextraPots ;
				}
			}
		}
		if ( ( type == RADIO_TYPE_QX7 ) || ( type == RADIO_TYPE_T12 ) )
		{
			if ( value > 6 )
			{
			 value -= 1 ;	
			}
		}
#endif
    ui->sourceCB->setCurrentIndex(value) ;

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

#ifdef EXTRA_SKYCHANNELS
  if ( ( md->srcRaw >= 21 && md->srcRaw <= 21+23 ) ||
			 ( md->srcRaw >= 70 && md->srcRaw <= 70+7 ) )
#else
	  if ( md->srcRaw >= 21 && md->srcRaw <= 44 )
#endif
		{
	  	ui->FMtrimChkB->setChecked(md->disableExpoDr) ;
		}
		else
		{
	  	ui->FMtrimChkB->setChecked(!md->disableExpoDr) ;
		}
		updateChannels() ;

		ui->sourceSwitchCB->clear();

    if ( (leeType == RADIO_TYPE_TARANIS ) || ( leeType == RADIO_TYPE_TPLUS ) || ( leeType == RADIO_TYPE_QX7 ) || ( leeType == RADIO_TYPE_T12 ) )	// Taranis
		{
    	ui->sourceSwitchCB->addItem("SA");
    	ui->sourceSwitchCB->addItem("SB");
    	ui->sourceSwitchCB->addItem("SC");
    	ui->sourceSwitchCB->addItem("SD");
			if ( ( leeType != RADIO_TYPE_QX7 ) && ( leeType != RADIO_TYPE_T12 ) )
			{
    		ui->sourceSwitchCB->addItem("SE");
			}
    	ui->sourceSwitchCB->addItem("SF");
			if ( ( leeType != RADIO_TYPE_QX7 ) && ( leeType != RADIO_TYPE_T12 ) )
			{
    		ui->sourceSwitchCB->addItem("SG");
			}
    	ui->sourceSwitchCB->addItem("SH");

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
    
    ui->sourceSwitchCB->addItem("L1  ");
    ui->sourceSwitchCB->addItem("L2  ");
    ui->sourceSwitchCB->addItem("L3  ");
    ui->sourceSwitchCB->addItem("L4  ");
    ui->sourceSwitchCB->addItem("L5  ");
    ui->sourceSwitchCB->addItem("L6  ");
    ui->sourceSwitchCB->addItem("L7  ");
    ui->sourceSwitchCB->addItem("L8  ");
    ui->sourceSwitchCB->addItem("L9  ");
		ui->sourceSwitchCB->addItem("LA  ");
    ui->sourceSwitchCB->addItem("LB  ");
    ui->sourceSwitchCB->addItem("LC  ");
    ui->sourceSwitchCB->addItem("LD  ");
    ui->sourceSwitchCB->addItem("LE  ");
    ui->sourceSwitchCB->addItem("LF  ");
    ui->sourceSwitchCB->addItem("LG  ");
    ui->sourceSwitchCB->addItem("LH  ");
    ui->sourceSwitchCB->addItem("LI  ");
    ui->sourceSwitchCB->addItem("LJ  ");
    ui->sourceSwitchCB->addItem("LK  ");
    ui->sourceSwitchCB->addItem("LL  ");
    ui->sourceSwitchCB->addItem("LM  ");
    ui->sourceSwitchCB->addItem("LN  ");
    ui->sourceSwitchCB->addItem("LO  ");
		if ( g_eeGeneral->analogMapping & 0x1C /*MASK_6POS*/ )
		{
    	ui->sourceSwitchCB->addItem("6POS");
		}
    

		if ( ( leeType == RADIO_TYPE_QX7 ) || ( leeType == RADIO_TYPE_T12 ) )
		{
			uint32_t index = md->switchSource ;
			if ( md->switchSource > 4 )
			{
				index -= 1 ;
			}
			if ( md->switchSource > 6 )
			{
				index -= 1 ;
			}
			ui->sourceSwitchCB->setCurrentIndex(index) ;
		}
		else
		{
			ui->sourceSwitchCB->setCurrentIndex(md->switchSource) ;
		}

		populateSpinGVarCB( ui->weightSB, ui->weightCB, ui->weightGvChkB, md->weight, -350, 350, md->extWeight ) ;
    populateSpinGVarCB( ui->offsetSB, ui->offsetCB, ui->offsetGvChkB, md->sOffset, -350, 350, md->extOffset ) ;
    
		ui->trimChkB->setChecked(md->carryTrim==0);
//    ui->FMtrimChkB->setChecked(!md->disableExpoDr);
    ui->lateOffsetChkB->setChecked(md->lateOffset);
    populateSwitchCB(ui->switchesCB,md->swtch, leeType );
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
		ValuesEditLock = false ;

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
  uint32_t lowBound = lType ? 21 : 21 ;
  if ( lType == RADIO_TYPE_TPLUS )
	{
		lowBound = 23 ;
	}
  if ( lType == RADIO_TYPE_9XTREME )
	{
		lowBound = 21 ;
	}
  if ( lType == RADIO_TYPE_X9E )
  {
    lowBound = 24 ;
  }
	if ( ( lType == RADIO_TYPE_QX7 ) || ( lType == RADIO_TYPE_T12 ) || ( lType == RADIO_TYPE_X9L ) )
  {
    lowBound = 20 ;
  }
#ifdef EXTRA_SKYCHANNELS
  if ( ( md->srcRaw >= 21 && md->srcRaw <= 21+23 ) ||
			 ( md->srcRaw >= 70 && md->srcRaw <= 70+7 ) )
#else
  if ( md->srcRaw >= 21 && md->srcRaw <= 21+23 )
#endif
	{
		ui->label_expo_output->setText( "Use Output" ) ;
		ui->label_expo_comment->setText( "(or Expo/Dr enable)" ) ;
	  ui->FMtrimChkB->setChecked(md->disableExpoDr) ;
		if ( md->disableExpoDr )
		{
			uint32_t i ;
			uint32_t j ;

#ifdef EXTRA_SKYCHANNELS
			j = 25 ;
      for ( i = lowBound-1+70-21 ; i < lowBound+7+70-21 ; i += 1 )
			{
				ui->sourceCB->setItemText( i, QString("OP%1").arg(j) ) ;
				j += 1 ;
			}
#endif
      for ( i = lowBound-1 ; i < lowBound+23 ; i += 1 )
			{
				ui->sourceCB->setItemText( i, QString("OP%1").arg(i-(lowBound-2)) ) ;
			}
		}
		else
		{
			uint32_t i ;
			uint32_t j ;
#ifdef EXTRA_SKYCHANNELS
			j = 25 ;
      for ( i = lowBound-1+70-21 ; i < lowBound+7+70-21 ; i += 1 )
			{
				ui->sourceCB->setItemText( i, QString("CH%1").arg(j) ) ;
				j += 1 ;
			}
#endif
      for ( i = lowBound-1 ; i < lowBound+23 ; i += 1 )
			{
				ui->sourceCB->setItemText( i, QString("CH%1").arg(i-(lowBound-2)) ) ;
			}
		}
	}
	else
	{
		ui->label_expo_output->setText( "Enable Expo/Dr" ) ;
		ui->label_expo_comment->setText( "(or Select output)" ) ;
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
  	
//		int x = ui->sourceCB->currentIndex()+1 ;
//		if ( x >= MIX_3POS )
//		{
//			if ( x >= MIX_3POS+7 )
//			{
//				x -= 6 ;
//			}
//			else
//			{
//		    md->sw23pos = x - (MIX_3POS) ;
//				x = MIX_3POS ;
//			}
//		}
//    md->srcRaw       = x ;
		
		if ( ( lType == RADIO_TYPE_QX7 ) || ( lType == RADIO_TYPE_T12 ) || ( lType == RADIO_TYPE_X9L ) )
		{
      if ( value >= 7 )
			{
        value += 0 ;
			}
		}
    value = decodePots( value, lType, lextraPots ) ;
		md->srcRaw       = value ;
//		ui->spinBox->setValue(md->srcRaw);

		if ( ( leeType == RADIO_TYPE_QX7 ) || ( leeType == RADIO_TYPE_T12 ) || ( leeType == RADIO_TYPE_X9L ) )
		{
			uint32_t index = ui->sourceSwitchCB->currentIndex() ;
			if ( index > 3 )
			{
				index += 1 ;
			}
			if ( index > 5 )
			{
				index += 1 ;
			}
		 	md->switchSource = index ;
		}
		else
		{
			md->switchSource = ui->sourceSwitchCB->currentIndex() ;
		}
		ui->sourceSwitchCB->setVisible( md->srcRaw == MIX_3POS ) ;
		{
			int value ;
			int extValue = 0 ;
	    value = numericSpinGvarValue( ui->weightSB, ui->weightCB, ui->weightGvChkB, md->weight, 100, 1 ) ;
			if ( value > 500 )
			{
				value -= 501 - 126 ;
				if ( value > 128 )
				{
					value -= 256 ;
				}
			}
			else
			{
				if ( value > 125 )
				{
					extValue = 1 ;
					value -= 125 ;
					if ( value > 125 )
					{
						extValue = 2 ;
						value -= 125 ;
					}
				}
				else if ( value < -125 )
				{
					extValue = 3 ;
					value += 125 ;
					if ( value < -125 )
					{
						extValue = 2 ;
						value += 125 ;
					}
				}
			}
			md->weight = value ;
			md->extWeight = extValue ;
  	  
			value = numericSpinGvarValue( ui->offsetSB, ui->offsetCB, ui->offsetGvChkB, md->sOffset, 0, 1 ) ;
			extValue = 0 ;
			if ( value > 500 )
			{
				value -= 501 - 126 ;
				if ( value > 128 )
				{
					value -= 256 ;
				}
			}
			else
			{
				if ( value > 125 )
				{
					extValue = 1 ;
					value -= 125 ;
					if ( value > 125 )
					{
						extValue = 2 ;
						value -= 125 ;
					}
				}
				else if ( value < -125 )
				{
					extValue = 3 ;
					value += 125 ;
					if ( value < -125 )
					{
						extValue = 2 ;
						value += 125 ;
					}
				}
			}

  	  md->sOffset = value ;
			md->extOffset = extValue ;
		}
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
#ifdef EXTRA_SKYCHANNELS
	  if ( ( md->srcRaw >= lowBound && md->srcRaw <= lowBound+23 ) ||
				 ( md->srcRaw >= lowBound-21+70 && md->srcRaw <= lowBound-21+70+7 ) )
		{
		  if ( ( oldSrcRaw >= lowBound && oldSrcRaw <= lowBound+23 ) ||
					 ( oldSrcRaw >= lowBound-21+70 && oldSrcRaw <= lowBound-21+70+7 ) )
#else
    if ( md->srcRaw >= lowBound && md->srcRaw <= lowBound+23 )
		{
			if ( oldSrcRaw >= lowBound && oldSrcRaw <= lowBound+23 )
#endif
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
#ifdef EXTRA_SKYCHANNELS
			if ( ( oldSrcRaw >= 21 && oldSrcRaw <= 44 ) ||
			 ( oldSrcRaw >= 70 && oldSrcRaw <= 70+7 ) )
#else
			if ( oldSrcRaw >= 21 && oldSrcRaw <= 44 )
#endif
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
#ifdef SKY    
	    		md->curve = ui->curvesCB->currentIndex()-19;
#else
	    		md->curve = ui->curvesCB->currentIndex()-16;
#endif
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



