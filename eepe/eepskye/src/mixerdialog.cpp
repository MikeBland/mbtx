#include "mixerdialog.h"
#include "ui_mixerdialog.h"
#include "pers.h"
#include "file.h"
#include "helpers.h"

extern void populateSpinVarCB( QSpinBox *sb, QComboBox *cb, QCheckBox *ck, int value, int min, int max, int xvalue ) ;
extern int numericSpinVarValue( QSpinBox *sb, QComboBox *cb, QCheckBox *ck, int value, int defvar ) ;

void MixerDialog::addSource( uint8_t index, QString str )
{
  ui->sourceCB->addItem( str=="" ? getSourceStr( lg_eeGeneral->stickMode, index, lModelVersion, lType, lextraPots ) : str ) ;
  sourceMap[sourceMapSize++] = index ;
//	printf("\nIndex %d, %s", index, getSourceStr( lg_eeGeneral->stickMode, index, lModelVersion, lType, lextraPots ).toUtf8().data() ) ;
}

void MixerDialog::setCurveData()
{
	uint32_t i ;
	uint32_t j ;
	diffIsGvar = 0 ;
  QString str = CURV_STR ;
	curveFunction = md->differential | (md->extDiff << 1 ) ;
	curveValue = md->curve ;

//	printf("\n%d %d %d", md->differential, md->extDiff, md->curve ) ;

	if ( curveFunction == 0 )
	{
    if ( md->curve <= -28 )
		{
			curveFunction = 4 ;
// value expo is -128 to -28 as 0 - 100
			curveValue += 128 ;
		}
		else
		{
			if ( ( curveValue > 0 ) && ( curveValue <= 6 ) )
			{
				curveFunction = 1 ;
			}
			else
			{
				if ( curveValue )
				{
					curveFunction = 2 ;
				}
				else
				{
					curveFunction = 0 ;
				}
			}
		}
	}
	else
	{
		if ( curveFunction == 3 )
		{
			diffIsGvar = 1 ;
		}
		else
		{
			if ( curveFunction == 1 )
			{
				diffIsGvar = 0 ;
				curveFunction = 3 ;
			}
		}
	}
	ui->diffcurveCB->setCurrentIndex(curveFunction) ;

//	printf("\nFun = %d", curveFunction ) ;

	switch ( curveFunction )
	{
		case 0 :	// None
			ui->curvesCB->setVisible( false ) ;
			ui->curvesSB->setVisible( false ) ;
			ui->curveGvChkB->setVisible( false ) ;
      ui->curveGvChkB->setChecked( false ) ;
		break ;
		case 1 :	// Function
//	printf("\n1" ) ;
			ui->curvesCB->clear() ;
    	for( i = 1 ; i <= 6 ; i+= 1 )
			{
				ui->curvesCB->addItem(str.mid(i*3,3)) ;
			}
			ui->curvesCB->setCurrentIndex(curveValue-1) ;
			ui->curvesCB->setVisible( true ) ;
			ui->curvesSB->setVisible( false ) ;
			ui->curveGvChkB->setVisible( false ) ;
      ui->curveGvChkB->setChecked( false ) ;
		break ;
		case 2 :	// Curve
    {
	    int32_t index ;
//	printf("\n2" ) ;
			ui->curvesCB->clear() ;
			j = 6 + 19 ;
    	for( i = j ; i >= 7 ; i -= 1 )
			{
        ui->curvesCB->addItem(str.mid(i*3,3).replace("c","!Curve "));
			}
      for(int i = 7 ; i <= j ; i++)  ui->curvesCB->addItem(str.mid(i*3,3).replace("c","Curve "));
			if ( curveValue < 0 )
			{
				index = curveValue + 19 ;
			}
			else
			{
				index = curveValue + 19 - 7 ;
			}
      ui->curvesCB->setCurrentIndex(index) ;
			ui->curvesCB->setVisible( true ) ;
			ui->curvesSB->setVisible( false ) ;
			ui->curveGvChkB->setVisible( false ) ;
      ui->curveGvChkB->setChecked( false ) ;
		}
		break ;
		case 3 :	// Diff
			if (varsInUse)
			{
				populateSpinVarCB( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, md->curve, -100, 100, 0 ) ;
			}
			else
			{
				populateSpinGVarCB( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, md->curve, -100, 100 ) ;
			}
			ui->curveGvChkB->setVisible( true ) ;
		break ;
		case 4 :	// Expo
			// Fill in ui->curvesCB with vars
			if (varsInUse)
			{
				int val = curveValue ;
					
				if (md->varForExpo)
				{
					val += 950 ;
				}
				populateSpinVarCB( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, val, 0, 100, 0 ) ;
				ui->curveGvChkB->setVisible( true ) ;
			}
			else
			{
				ui->curvesCB->setVisible( false ) ;
				ui->curvesSB->setVisible( true ) ;
				ui->curvesSB->setValue( curveValue ) ;
			}
		break ;
	}
}


MixerDialog::MixerDialog(QWidget *parent, SKYMixData *mixdata, EEGeneral *g_eeGeneral, QString * comment, int modelVersion, struct t_radioData *rData ) :
    QDialog(parent),
    ui(new Ui::MixerDialog)
{
    ui->setupUi(this);
    md = mixdata;
		leeType = rData->type ;
		lextraPots = rData->extraPots ;
		lBitType = rData->bitType ;
  lModelVersion = modelVersion ;
	lg_eeGeneral = g_eeGeneral ;
	varsInUse = rData->models[g_eeGeneral->currModel].vars ;
	uint32_t i ;

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

	initRadioHw( lType, rData ) ;
	struct t_radioHardware *prh = &rData->radioHardware ;

//		ui->spinBox->setValue(md->srcRaw);

//    ui->sourceCB->setFont(QFont("Ariel",16));

	ui->sourceCB->clear() ;
	sourceMapSize = 0 ;

//	printf("\nType = %d", leeType ) ;

	for ( i = 1 ; i < 5 ; i += 1 )
	{
		addSource( i ) ;
	}
	for ( i = 0 ; i < prh->numberPots ; i += 1 )
	{
		addSource( prh->potIndices[i], prh->potNames[i] ) ;
	}
	for ( i = 8 ; i < 21 ; i += 1 ) // HALF,FULL..PPM8
	{
		addSource( i ) ;
	}
	for ( i = 62 ; i < 70 ; i += 1 ) // PPM9-16
	{
		addSource( i ) ;
	}
	for ( i = 21 ; i < 45 ; i += 1 )	// CH1-24
	{
		addSource( i ) ;
	}
	for ( i = 70 ; i < 78 ; i += 1 )	// CH25-32
	{
		addSource( i ) ;
	}
	for ( i = 46 ; i < 62 ; i += 1 )	// GV1-SC8
	{
		addSource( i ) ;
	}
	addSource( 45 ) ;
	addSource( 78 ) ;
	addSource( 79 ) ;
	addSource( 80 ) ;
	addSource( 81 ) ;
	for ( i = 224 ; i < 256 ; i += 1 )	// Inputs
	{
		addSource( i ) ;
	}


//    populateSourceCB(ui->sourceCB, g_eeGeneral->stickMode, 0, md->srcRaw, modelVersion, type, lextraPots ) ;

//		ui->sourceCB->addItem("SWCH");
		
////		ui->sourceCB->addItem("sIDx");
////    ui->sourceCB->addItem("sTHR");
////    ui->sourceCB->addItem("sRUD");
////    ui->sourceCB->addItem("sELE");
////    ui->sourceCB->addItem("sAIL");
////    ui->sourceCB->addItem("sGEA");
////    ui->sourceCB->addItem("sTRN");
////    ui->sourceCB->addItem("L1  ");
////    ui->sourceCB->addItem("L2  ");
////    ui->sourceCB->addItem("L3  ");
////    ui->sourceCB->addItem("L4  ");
////    ui->sourceCB->addItem("L5  ");
////    ui->sourceCB->addItem("L6  ");
////    ui->sourceCB->addItem("L7  ");
////    ui->sourceCB->addItem("L8  ");
////    ui->sourceCB->addItem("L9  ");
////		ui->sourceCB->addItem("LA  ");
////    ui->sourceCB->addItem("LB  ");
////    ui->sourceCB->addItem("LC  ");
////    ui->sourceCB->addItem("LD  ");
////    ui->sourceCB->addItem("LE  ");
////    ui->sourceCB->addItem("LF  ");
////    ui->sourceCB->addItem("LG  ");
////    ui->sourceCB->addItem("LH  ");
////    ui->sourceCB->addItem("LI  ");
////    ui->sourceCB->addItem("LJ  ");
////    ui->sourceCB->addItem("LK  ");
////    ui->sourceCB->addItem("LL  ");
////    ui->sourceCB->addItem("LM  ");
////    ui->sourceCB->addItem("LN  ");
////    ui->sourceCB->addItem("LO  ");

////		if ( g_eeGeneral->analogMapping & 0x1C /*MASK_6POS*/ )
////		{
////	    ui->sourceCB->addItem("6POS");
////    }

//		ui->sourceCB->addItem("GV1 ");
//    ui->sourceCB->addItem("GV2 ");
//    ui->sourceCB->addItem("GV3 ");
//    ui->sourceCB->addItem("GV4 ");
//    ui->sourceCB->addItem("GV5 ");
//    ui->sourceCB->addItem("GV6 ");
//    ui->sourceCB->addItem("GV7 ");
//    ui->sourceCB->addItem("THIS");
//    ui->sourceCB->addItem("SC1 ");
//    ui->sourceCB->addItem("SC2 ");
//    ui->sourceCB->addItem("SC3 ");
//    ui->sourceCB->addItem("SC4 ");
//    ui->sourceCB->addItem("SC5 ");
//    ui->sourceCB->addItem("SC6 ");
//    ui->sourceCB->addItem("SC7 ");
//    ui->sourceCB->addItem("SC8 ");
//    ui->sourceCB->addItem("PPM9");
//    ui->sourceCB->addItem("PPM10");
//    ui->sourceCB->addItem("PPM11");
//    ui->sourceCB->addItem("PPM12");
//    ui->sourceCB->addItem("PPM13");
//    ui->sourceCB->addItem("PPM14");
//    ui->sourceCB->addItem("PPM15");
//    ui->sourceCB->addItem("PPM16");
//#ifdef EXTRA_SKYCHANNELS
//    ui->sourceCB->addItem("CH25");
//    ui->sourceCB->addItem("CH26");
//    ui->sourceCB->addItem("CH27");
//    ui->sourceCB->addItem("CH28");
//    ui->sourceCB->addItem("CH29");
//    ui->sourceCB->addItem("CH30");
//    ui->sourceCB->addItem("CH31");
//    ui->sourceCB->addItem("CH32");
//#endif
//    ui->sourceCB->addItem("Rtm");
//    ui->sourceCB->addItem("Etm");
//    ui->sourceCB->addItem("Ttm");
//    ui->sourceCB->addItem("Atm");




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
//#ifdef SKY    
//		int value ;
//		value = md->srcRaw ;
//		if ( ( type == RADIO_TYPE_TARANIS ) || ( type == RADIO_TYPE_TPLUS ) || ( type == RADIO_TYPE_X9E ) || ( type == RADIO_TYPE_X10 ) )
//		{
//			if ( value >= EXTRA_POTS_POSITION )
//			{
//				if ( value >= EXTRA_POTS_START )
//				{
//					value -= ( EXTRA_POTS_START - EXTRA_POTS_POSITION ) ;
//				}
//				else
//				{
//					value += type == RADIO_TYPE_TPLUS ? 2 : type == RADIO_TYPE_X9E ? 3 : NUM_EXTRA_POTS ;
//				}
//			}
//		}
//    if ( ( type == RADIO_TYPE_SKY ) || ( type == RADIO_TYPE_9XTREME ) )
//		{
//			if ( value >= EXTRA_POTS_POSITION )
//			{
//				if ( value >= EXTRA_POTS_START )
//				{
//					value -= ( EXTRA_POTS_START - EXTRA_POTS_POSITION ) ;
//				}
//				else
//				{
//					value += lextraPots ;
//				}
//			}
//		}
//		if ( ( type == RADIO_TYPE_QX7 ) || ( type == RADIO_TYPE_T12 ) )
//		{
//			if ( value > 6 )
//			{
//			 value -= 1 ;	
//			}
//		}
//#endif

	for ( i = 0 ; i < sourceMapSize ; i += 1 )
	{
		if ( md->srcRaw == sourceMap[i] )
		{
			ui->sourceCB->setCurrentIndex(i) ;
			break ;
		}
	}
	if ( i >= sourceMapSize )
	{
//		if ( id->chn == 0 )
//		{
//			id->chn = 1 ;
//		}
		ui->sourceCB->setCurrentIndex(0) ;
	}

  ui->sourceCB->setMaxVisibleItems(10) ;
    
//		ui->sourceCB->setCurrentIndex(value) ;

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
    
//		ui->sourceCB->removeItem(0);

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

    if ( (leeType == RADIO_TYPE_TARANIS ) || ( leeType == RADIO_TYPE_TPLUS ) || ( leeType == RADIO_TYPE_QX7 ) || ( leeType == RADIO_TYPE_T12 ) || ( leeType == RADIO_TYPE_X10 ) )	// Taranis
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
			if ( rData->bitType & ( RADIO_BITTYPE_X12 | RADIO_BITTYPE_X10 | RADIO_BITTYPE_T16 | RADIO_BITTYPE_TX16S | RADIO_BITTYPE_X10E | RADIO_BITTYPE_TX18S )  )
			{
    		ui->sourceSwitchCB->addItem("6POS");
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
		
		if (varsInUse)
		{
			int val = md->weight ;
			ui->weightGvChkB->setText( "Var" ) ;
			ui->offsetGvChkB->setText( "Var" ) ;
			ui->curveGvChkB->setText( "Var" ) ;
			if (md->varForWeight)
			{
				val += 1000 ;
			}
			populateSpinVarCB( ui->weightSB, ui->weightCB, ui->weightGvChkB, val, -350, 350, md->extWeight ) ;
			val = md->sOffset ;
			if (md->varForOffset)
			{
				val += 1000 ;
			}
			populateSpinVarCB( ui->offsetSB, ui->offsetCB, ui->offsetGvChkB, val, -350, 350, md->extOffset ) ;
		}
		else
		{
			ui->weightGvChkB->setText( "Gvar" ) ;
			ui->offsetGvChkB->setText( "Gvar" ) ;
			ui->curveGvChkB->setText( "Gvar" ) ;
			populateSpinGVarCB( ui->weightSB, ui->weightCB, ui->weightGvChkB, md->weight, -350, 350, md->extWeight ) ;
  	  populateSpinGVarCB( ui->offsetSB, ui->offsetCB, ui->offsetGvChkB, md->sOffset, -350, 350, md->extOffset ) ;
		}

		ui->trimChkB->setChecked(md->carryTrim==0);
//    ui->FMtrimChkB->setChecked(!md->disableExpoDr);
    ui->lateOffsetChkB->setChecked(md->lateOffset);
    populateSwitchCB(ui->switchesCB,md->swtch, leeType );
    ui->warningCB->setCurrentIndex(md->mixWarn);
    ui->mltpxCB->setCurrentIndex(md->mltpx);

// Now the Curve/Diff/Expo
		setCurveData() ;


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
		ui->Fm7CB->setChecked( !(md->modeControl & 128) ) ;
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
    connect(ui->Fm7CB,SIGNAL(stateChanged(int)),this,SLOT(valuesChanged()));

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
//  uint32_t lowBound = lType ? 21 : 21 ;
//  if ( lType == RADIO_TYPE_TPLUS )
//	{
//		lowBound = 23 ;
//	}
//  if ( lType == RADIO_TYPE_9XTREME )
//	{
//		lowBound = 21 ;
//	}
//  if ( lType == RADIO_TYPE_X9E )
//  {
//    lowBound = 24 ;
//  }
//	if ( ( lType == RADIO_TYPE_QX7 ) || ( lType == RADIO_TYPE_T12 ) || ( lType == RADIO_TYPE_X9L ) )
//  {
//    lowBound = 20 ;
//  }
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

			// Search for map value of 21

      for ( i = 0 ; i < sourceMapSize ; i += 1 )
			{
        if ( sourceMap[i] == 21 )
				{
					break ;
				}
			}
      if ( i < (uint32_t)sourceMapSize - 32 )
			{
				j = i ;
      	for ( i = 0 ; i < 32 ; i += 1 )
				{
					ui->sourceCB->setItemText( j+i, QString("OP%1").arg(i+1) ) ;
				}
			}

		}
		else
		{
			uint32_t i ;
			uint32_t j ;
      for ( i = 0 ; i < sourceMapSize ; i += 1 )
			{
        if ( sourceMap[i] == 21 )
				{
					break ;
				}
			}
      if ( i < (uint32_t)sourceMapSize - 32 )
			{
				j = i ;
      	for ( i = 0 ; i < 32 ; i += 1 )
				{
					ui->sourceCB->setItemText( j+i, QString("CH%1").arg(i+1) ) ;
				}
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


int extendedSpinGvarValue( QSpinBox *sb, QComboBox *cb, QCheckBox *ck, int value, int defvar, int extValue, uint8_t varsInUse )
{
	if (varsInUse)
	{
		if ( value > 900 )
		{
			// was a VAR
			if ( ck->checkState() )
			{ // still is
				value = cb->currentIndex() + 1000 - NUM_VARS ;
			}
			else
			{
				value = defvar ;
				sb->setValue( value ) ;
				sb->setVisible( true ) ;
				cb->setVisible( false ) ;
			}
		}
		else
		{
			// was not a VAR
			if ( ck->checkState() )
			{ // Now is a VAR
			  value = 1000 ;
				cb->setCurrentIndex( NUM_VARS ) ;
				cb->setVisible( true ) ;
				sb->setVisible( false ) ;
			}
			else
			{
				value = sb->value() ;
			}
		}
		return value ;
	}

	if ( ( value < -125 ) || ( value > 125) )
	{
		if ( value < 0 )
		{
			value += 256 ;
		}
		value += 510 - 126 ;
	}
	else
	{
		if ( extValue == 1 )
		{
      value += 125 ;
		}
		else if ( extValue == 3 )
		{
			value -= 125 ; 
		}
		else if ( extValue == 2 )
		{
			if ( value < 0 )
			{
				value -= 250 ;
			}
			else
			{
				value += 250 ;
			}
		}
		if ( value > 350 )
		{
			value += 510 - 360 ;
		}
	}
	if ( value > 500 )
	{
		// Was a GVAR
		if ( ck->checkState() )
		{ // still is
			value = cb->currentIndex() + 503 ;
		}
		else
		{
			value = defvar ;
			sb->setValue( value ) ;
			sb->setVisible( true ) ;
			cb->setVisible( false ) ;
		}
	}
	else
	{ // Not a GVAR
		if ( ck->checkState() )
		{ // Now is a GVAR
		  value = 510 ;
			cb->setCurrentIndex( value-503 ) ;
			cb->setVisible( true ) ;
			sb->setVisible( false ) ;
		}
		else
		{
			value = sb->value() ;
		}
	}
	return value ;
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
		value = ui->sourceCB->currentIndex() ;

  md->srcRaw = (value < sourceMapSize) ? sourceMap[value] : 1 ;
  	
		
		
		 
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
		
//		if ( ( lType == RADIO_TYPE_QX7 ) || ( lType == RADIO_TYPE_T12 ) || ( lType == RADIO_TYPE_X9L ) )
//		{
//      if ( value >= 7 )
//			{
//        value += 0 ;
//			}
//		}
//    value = decodePots( value, lType, lextraPots ) ;
//		md->srcRaw       = value ;
//		ui->spinBox->setValue(md->srcRaw);

		uint32_t index = ui->sourceSwitchCB->currentIndex() ;
		if ( ( lBitType & ( RADIO_BITTYPE_X12 | RADIO_BITTYPE_X10 | RADIO_BITTYPE_T16 | RADIO_BITTYPE_TX16S | RADIO_BITTYPE_X10E | RADIO_BITTYPE_TX18S )  ) == 0 )
		{
			if ( index > 7 )
			{
				index += 1 ;
			}
		}

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
		}
	 	md->switchSource = index ;
    ui->sourceSwitchCB->setVisible( md->srcRaw == MIX_3POS ) ;

//		printf("\nSrc = %d, SWCH = %d", md->srcRaw, md->switchSource ) ;

		{
			int value ;
			int extValue = 0 ;

			value = md->weight ;
			if ( md->varForWeight )
			{
				value += 1000 ;
			}
	    value = extendedSpinGvarValue( ui->weightSB, ui->weightCB, ui->weightGvChkB, value, 100, md->extWeight, varsInUse ) ;
			if (varsInUse)
			{
				if ( value > 900 )
				{
					value -= 1000 ;
					extValue = 0 ;
					md->varForWeight = 1 ;
				}
				else
				{
					md->varForWeight = 0 ;
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
			}
			else
			{
				if ( value > 500 )
				{
					value -= 400 ;
					extValue = 2 ;
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
			}
			md->weight = value ;
			md->extWeight = extValue ;
  	  
			value = md->sOffset ;
			if ( md->varForOffset )
			{
				value += 1000 ;
			}
			value = extendedSpinGvarValue( ui->offsetSB, ui->offsetCB, ui->offsetGvChkB, value, 0, md->extOffset, varsInUse ) ;
			extValue = 0 ;
			if (varsInUse)
			{
				if ( value > 900 )
				{
					value -= 1000 ;
					extValue = 0 ;
					md->varForOffset = 1 ;
				}
				else
				{
					md->varForOffset = 0 ;
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
			}
			else
			{
				if ( value > 500 )
				{
					value -= 400 ;
					extValue = 2 ;
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
			}
  	  md->sOffset = value ;
			md->extOffset = extValue ;
		}
    md->carryTrim    = ui->trimChkB->checkState() ? 0 : 1;
//		int limit = MAX_DRSWITCH ;
//		if ( leeType )
//		{
//			limit = MAX_XDRSWITCH ;
//		}
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

//		oldcurvemode = md->differential ;
//		if ( oldcurvemode == 0 )
//		{
//			if ( md->curve <= -28 )
//			{
//				oldcurvemode = 2 ;
//			}
//		}
		
		uint32_t newcurvemode = ui->diffcurveCB->currentIndex() ;
		if ( curveFunction != newcurvemode )
		{
			switch ( newcurvemode )
			{
				case 0 :
					md->curve = 0 ;
					md->differential = 0 ;
					md->extDiff = 0 ;
				break ;
				case 1 :
					md->curve = 1 ;
					md->differential = 0 ;
					md->extDiff = 0 ;
				break ;
				case 2 :
					md->curve = 7 ;
					md->differential = 0 ;
					md->extDiff = 0 ;
				break ;
				case 3 :
					md->curve = 0 ;
					md->differential = 1 ;
					md->extDiff = 0 ;
				break ;
				case 4 :
					md->curve = -128 ;
					md->differential = 0 ;
					md->extDiff = 0 ;
				break ;
			}
			setCurveData() ;
		}
		else
		{
			switch ( curveFunction )
			{
				case 0 :
					md->curve = 0 ;
				break ;
				case 1 :
					md->curve = ui->curvesCB->currentIndex() + 1 ;
				break ;
				case 2 :
				{
					int32_t index = ui->curvesCB->currentIndex() - 19 ;
					if ( index >= 0 )
					{
						index += 7 ;
					}
					md->curve = index ;
				}
				break ;
				case 3 :
				if (varsInUse)
				{
					md->curve = numericSpinVarValue( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, md->curve, 0 ) ;
				}
				else
				{
		   		md->curve = numericSpinGvarValue( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, md->curve, 0 ) ;
				}
				break ;
				case 4 :
				if (varsInUse)
				{
					int32_t value ;
          value = md->curve + 128 ;
					if ( md->varForExpo )
					{
						value -= 50 ;
						if ( value >= 0 )
						{
							value += 101 ;
						}
						else
						{
							value -= 100 ;
						}
					}
					value = numericSpinVarValue( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, value, 0 ) ;
					if ( ( value > 100 ) || ( value < -100 ) )
					{ // a VAR
						if ( value > 100 )
						{
							value -= 101 ;
						}
						else
						{
							value += 100 ;
						}
						value -= (128-50) ;
          	md->curve = value ;
						md->varForExpo = 1 ;
					}
					else
					{
          	md->curve = value - 128 ;
						md->varForExpo = 0 ;
					}
				}
				else
				{
          md->curve = ui->curvesSB->value() - 128 ;
				}
				break ;
			}
		}
		
//		md->differential = ( newcurvemode == 1 ) ? 1 : 0 ;
		
//		if ( newcurvemode != oldcurvemode )
//		{
//			if (md->differential)
//			{
//				populateSpinGVarCB( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, 0, -100, 100 ) ;
//				ui->curveGvChkB->setVisible( true ) ;
//			}
//			else
//			{
//				if ( newcurvemode == 2 )
//				{
//					ui->curvesCB->setVisible( false ) ;
//					ui->curvesSB->setVisible( true ) ;
//					ui->curvesSB->setValue( 0 ) ;
//          ui->curvesSB->setMinimum( 0 ) ;
//          ui->curvesSB->setMaximum( 100 ) ;
//				}
//				else
//				{
//		      ui->curveGvChkB->setChecked( false ) ;
//					populateCurvesCB(ui->curvesCB, 0 ) ;
//					ui->curvesSB->setVisible( false ) ;
//					ui->curvesCB->setVisible( true ) ;
//				}
//				ui->curveGvChkB->setVisible( false ) ;
//			}
//			if ( newcurvemode == 2 )
//			{
//	    	md->curve = -128 ;
//			}
//			else
//			{
//	    	md->curve = numericSpinGvarValue( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, 0, 0 ) ;
//			}
//		}
//		else
//		{
//			if (md->differential)
//			{
//	   		md->curve = numericSpinGvarValue( ui->curvesSB, ui->curvesCB, ui->curveGvChkB, md->curve, 0 ) ;
//			}
//			else
//			{
//				if ( ui->diffcurveCB->currentIndex() == 2 )
//				{
//          md->curve = ui->curvesSB->value() - 128 ;
//				}
//				else
//				{
//#ifdef SKY    
//	    		md->curve = ui->curvesCB->currentIndex()-19;
//#else
//	    		md->curve = ui->curvesCB->currentIndex()-16;
//#endif
//				}	 
//			}
//		}

		int j = 127 ;
		j &= ~( ui->Fm0CB->checkState() ? 1 : 0 ) ;
		j &= ~( ui->Fm1CB->checkState() ? 2 : 0 ) ;
		j &= ~( ui->Fm2CB->checkState() ? 4 : 0 ) ;
		j &= ~( ui->Fm3CB->checkState() ? 8 : 0 ) ;
		j &= ~( ui->Fm4CB->checkState() ? 16 : 0 ) ;
		j &= ~( ui->Fm5CB->checkState() ? 32 : 0 ) ;
		j &= ~( ui->Fm6CB->checkState() ? 64 : 0 ) ;
		j &= ~( ui->Fm7CB->checkState() ? 128 : 0 ) ;
		md->modeControl = j ;

//    if(ui->FMtrimChkB->checkState())
//        ui->offset_label->setText("FmTrimVal");
//    else
//        ui->offset_label->setText("Offset");

    mixCommennt->clear();
    mixCommennt->append(ui->mixerComment->toPlainText());
		
		ValuesEditLock = false ;
}



