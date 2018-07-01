#include "SwitchDialog.h"
#include "ui_SwitchDialog.h"
#include "pers.h"
#include "file.h"
#include "helpers.h"

SwitchDialog::SwitchDialog(QWidget *parent, int index, struct t_switchData *sdata, int modelVersion, struct t_radioData *rData ) :
    QDialog(parent),
    ui(new Ui::SwitchDialog)
{
  char text[4] ;
  ui->setupUi(this) ;
	sindex = index ;
	lsdata = sdata ;
	lrData = rData ;
	lmodelVersion = modelVersion ;
	text[0] = 'L' ;
	text[1] = 'S' ;
	text[2] = ( index < 9 ) ? index + '1' : index + 'A' - 9	;
	text[3] = '\0' ;
	ui->switchNameLabel->setText( text ) ;

	update() ;
}

SwitchDialog::~SwitchDialog()
{
    delete ui;
}

//void MixerDialog::changeEvent(QEvent *e)
//{
//    QDialog::changeEvent(e);
//    switch (e->type()) {
//    case QEvent::LanguageChange:
//        ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
//}


void SwitchDialog::update()
{
	uint32_t cType ;
	cType = CS_STATE( lsdata->swData.func, lmodelVersion ) ;
	int32_t value ;

	populateCSWCB( ui->functionCB, lsdata->swData.func, lmodelVersion);
	ui->delaySB->setValue( (double) lsdata->switchDelay / 10 ) ;
  if ( lrData->bitType & ( RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE) )
	{
    x9dPopulateSwitchAndCB( ui->andSwCB, lsdata->swData.andsw ) ;
	}
	else
	{
		populateSwitchAndCB( ui->andSwCB, lsdata->swData.andsw ) ;
	}

	
	switch ( cType )
	{
    case CS_VOFS:
		case CS_U16:
      populateSourceCB( ui->v1CB, lrData->generalSettings.stickMode, 1, lsdata->swData.v1, lmodelVersion, lrData->type, lrData->extraPots ) ;
			ui->v1CB->setVisible(true) ;
			ui->sw1CB->setVisible(false) ;
			ui->sw1Label->setVisible(false) ;
			ui->sw2CB->setVisible(false) ;
			ui->sw2Label->setVisible(false) ;
			ui->valSB->setVisible(true) ;
			if ( cType == CS_U16 )
			{
        if ( ui->valSB->maximum() != 32767 )
				{
        	ui->valSB->setMaximum(32767);
        	ui->valSB->setMinimum(-32768);
				}
			}
			else
			{
        if ( ui->valSB->maximum() != 125 )
				{
        	ui->valSB->setMaximum(125);
        	ui->valSB->setMinimum(-125);
				}
			}
			ui->valSB->setAccelerated(true) ;
			if ( cType == CS_U16 )
			{
				int32_t y ;
				y = (uint8_t) lsdata->swData.v2 ;
        y |= lsdata->swData.bitAndV3 << 8 ;
				y -= 32768 ;
        ui->valSB->setValue( y ) ;
//       	cswitchTlabel[i]->setVisible(false);
			}
			else
			{
        ui->valSB->setValue(lsdata->swData.v2);
//				if ( ui->valSB->currentIndex() > 36 )
//				{
//        	cswitchTlabel[i]->setVisible(true);
//					value = convertTelemConstant( cswitchSource1[i]->currentIndex() - 45, lsdata->swData.v2, &g_model ) ;
//        	stringTelemetryChannel( telText, lsdata->swData].v1 - 45, value, &g_model ) ;
//					sprintf( telText, "%d", value ) ;
//        	cswitchTlabel[i]->setText(telText);
//				}
//				else
//				{
//        	cswitchTlabel[i]->setVisible(false);
//				}
			}
		break ;
    
		case CS_VBOOL:
      ui->sw1CB->setVisible(true) ;
      ui->sw2CB->setVisible(true) ;
			ui->sw1Label->setVisible(true) ;
			ui->sw1Label->setVisible(true) ;
      
			ui->valSB->setVisible(false) ;
      
			populateSwitchCB( ui->sw1CB, lsdata->swData.v1, lrData->type) ;
      populateSwitchCB( ui->sw2CB, lsdata->swData.v2, lrData->type) ;
			
//			cswitchText1[i]->setVisible(false) ;
//			cswitchText2[i]->setVisible(false) ;
		break ;
    case CS_VCOMP:
			ui->v1CB->setVisible(true) ;
			ui->v2CB->setVisible(true) ;
			ui->sw1CB->setVisible(false) ;
			ui->sw1Label->setVisible(false) ;
			ui->sw2CB->setVisible(false) ;
			ui->sw2Label->setVisible(false) ;
			ui->valSB->setVisible(true) ;
      populateSourceCB( ui->v1CB, lrData->generalSettings.stickMode, 1, lsdata->swData.v1, lmodelVersion, lrData->type, lrData->extraPots ) ;
      populateSourceCB( ui->v2CB, lrData->generalSettings.stickMode, 1, lsdata->swData.v2, lmodelVersion, lrData->type, lrData->extraPots ) ;
    break ;
    case CS_TIMER:
			ui->sw1CB->setVisible(false) ;
			ui->sw1Label->setVisible(false) ;
			ui->sw2CB->setVisible(false) ;
			ui->sw2Label->setVisible(false) ;
      ui->timeOnSB->setVisible(true);
      ui->timeOffSB->setVisible(true);
      ui->timeOnSB->setAccelerated(true);
      ui->timeOffSB->setAccelerated(true);
			value = lsdata->swData.v1+1 ;
      ui->spinBox->setValue( value ) ;
			if ( value <= 0 )
			{
				value -= 1 ;
				value = -value ;
      	ui->timeOnSB->setValue((float)value/10) ;
			}
			else
			{
      	ui->timeOnSB->setValue(value) ;
			}
			value = lsdata->swData.v2+1 ;
			if ( value <= 0 )
			{
				value -= 1 ;
				value = -value ;
      	ui->timeOffSB->setValue((float)value/10) ;
			}
			else
			{
      	ui->timeOffSB->setValue(value) ;
			}
    break ;
	}
}

void SwitchDialog::on_timeOnSB_valueChanged( double x )
{
	if ( x > 4.9 )
	{
		ui->timeOnSB->setSingleStep( 1.0 ) ;
	}
	else if ( x < 5.1 )
	{
		ui->timeOnSB->setSingleStep( 0.1 ) ;
	}
	ui->spinBox->setValue( x * 10 ) ;
}


//void MixerDialog::updateChannels()
//{
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
//  if ( lType == RADIO_TYPE_QX7 )
//  {
//    lowBound = 20 ;
//  }
//#ifdef EXTRA_SKYCHANNELS
//  if ( ( md->srcRaw >= 21 && md->srcRaw <= 21+23 ) ||
//			 ( md->srcRaw >= 70 && md->srcRaw <= 70+7 ) )
//#else
//  if ( md->srcRaw >= 21 && md->srcRaw <= 21+23 )
//#endif
//	{
//		ui->label_expo_output->setText( "Use Output" ) ;
//		ui->label_expo_comment->setText( "(or Expo/Dr enable)" ) ;
//	  ui->FMtrimChkB->setChecked(md->disableExpoDr) ;
//		if ( md->disableExpoDr )
//		{
//			uint32_t i ;
//			uint32_t j ;

//#ifdef EXTRA_SKYCHANNELS
//			j = 25 ;
//      for ( i = lowBound-1+70-21 ; i < lowBound+7+70-21 ; i += 1 )
//			{
//				ui->sourceCB->setItemText( i, QString("OP%1").arg(j) ) ;
//				j += 1 ;
//			}
//#endif
//      for ( i = lowBound-1 ; i < lowBound+23 ; i += 1 )
//			{
//				ui->sourceCB->setItemText( i, QString("OP%1").arg(i-(lowBound-2)) ) ;
//			}
//		}
//		else
//		{
//			uint32_t i ;
//			uint32_t j ;
//#ifdef EXTRA_SKYCHANNELS
//			j = 25 ;
//      for ( i = lowBound-1+70-21 ; i < lowBound+7+70-21 ; i += 1 )
//			{
//				ui->sourceCB->setItemText( i, QString("CH%1").arg(j) ) ;
//				j += 1 ;
//			}
//#endif
//      for ( i = lowBound-1 ; i < lowBound+23 ; i += 1 )
//			{
//				ui->sourceCB->setItemText( i, QString("CH%1").arg(i-(lowBound-2)) ) ;
//			}
//		}
//	}
//	else
//	{
//		ui->label_expo_output->setText( "Enable Expo/Dr" ) ;
//		ui->label_expo_comment->setText( "(or Select output)" ) ;
//	  ui->FMtrimChkB->setChecked(!md->disableExpoDr) ;
//	}
////  ui->FMtrimChkB->setChecked(!md->disableExpoDr) ;
//}

//void MixerDialog::valuesChanged()
//{
//	int oldcurvemode ;
//	int oldSrcRaw ;
    
//	if ( ValuesEditLock )
//	{
//		return ;
//	}
//	ValuesEditLock = true ;
		
//		oldSrcRaw = md->srcRaw ;
//    uint32_t value ;
//		value = ui->sourceCB->currentIndex()+1 ;
  	
////		int x = ui->sourceCB->currentIndex()+1 ;
////		if ( x >= MIX_3POS )
////		{
////			if ( x >= MIX_3POS+7 )
////			{
////				x -= 6 ;
////			}
////			else
////			{
////		    md->sw23pos = x - (MIX_3POS) ;
////				x = MIX_3POS ;
////			}
////		}
////    md->srcRaw       = x ;
		
//		if ( lType == RADIO_TYPE_QX7 )
//		{
//      if ( value >= 7 )
//			{
//				value += 1 ;
//			}
//		}
//    value = decodePots( value, lType, lextraPots ) ;
//		md->srcRaw       = value ;
////		ui->spinBox->setValue(md->srcRaw);

//		if ( leeType == RADIO_TYPE_QX7 ) 
//		{
//			uint32_t index = ui->sourceSwitchCB->currentIndex() ;
//			if ( index > 3 )
//			{
//				index += 1 ;
//			}
//			if ( index > 5 )
//			{
//				index += 1 ;
//			}
//		 	md->switchSource = index ;
//		}
//		else
//		{
//			md->switchSource = ui->sourceSwitchCB->currentIndex() ;
//		}
//		ui->sourceSwitchCB->setVisible( md->srcRaw == MIX_3POS ) ;
//    md->weight       = numericSpinGvarValue( ui->weightSB, ui->weightCB, ui->weightGvChkB, md->weight, 100 ) ;
//    md->sOffset      = numericSpinGvarValue( ui->offsetSB, ui->offsetCB, ui->offsetGvChkB, md->sOffset, 0 ) ;
//    md->carryTrim    = ui->trimChkB->checkState() ? 0 : 1;
//		int limit = MAX_DRSWITCH ;
//		if ( leeType )
//		{
//			limit = MAX_XDRSWITCH ;
//		}
//    md->swtch        = getSwitchCbValue( ui->switchesCB, leeType ) ;
//    md->mixWarn      = ui->warningCB->currentIndex();
//    md->mltpx        = ui->mltpxCB->currentIndex();
//    md->delayDown    = ui->delayDownSB->value()*10+0.4;
//    md->delayUp      = ui->delayUpSB->value()*10+0.4;
//    md->speedDown    = ui->slowDownSB->value()*10+0.4;
//    md->speedUp      = ui->slowUpSB->value()*10+0.4;
//    md->lateOffset   = ui->lateOffsetChkB->checkState() ? 1 : 0;

//		int lowBound = leeType ? 21 : 21 ;
//#ifdef EXTRA_SKYCHANNELS
//	  if ( ( md->srcRaw >= lowBound && md->srcRaw <= lowBound+23 ) ||
//				 ( md->srcRaw >= lowBound-21+70 && md->srcRaw <= lowBound-21+70+7 ) )
//		{
//		  if ( ( oldSrcRaw >= lowBound && oldSrcRaw <= lowBound+23 ) ||
//					 ( oldSrcRaw >= lowBound-21+70 && oldSrcRaw <= lowBound-21+70+7 ) )
//#else
//    if ( md->srcRaw >= lowBound && md->srcRaw <= lowBound+23 )
//		{
//			if ( oldSrcRaw >= lowBound && oldSrcRaw <= lowBound+23 )
//#endif
//			{
//	    	md->disableExpoDr = ui->FMtrimChkB->checkState() ? 1 : 0 ;
//				updateChannels() ;
//			}
//			else
//			{
//				updateChannels() ;
////	    	md->disableExpoDr = ui->FMtrimChkB->checkState() ? 0 : 1 ;
//			}
//		}
//		else
//		{
//#ifdef EXTRA_SKYCHANNELS
//			if ( ( oldSrcRaw >= 21 && oldSrcRaw <= 44 ) ||
//			 ( oldSrcRaw >= 70 && oldSrcRaw <= 70+7 ) )
//#else
//			if ( oldSrcRaw >= 21 && oldSrcRaw <= 44 )
//#endif
//			{
//				updateChannels() ;
//			}
//			else
//			{
//	    	md->disableExpoDr = ui->FMtrimChkB->checkState() ? 0 : 1 ;
//				updateChannels() ;
//			}
//		}

//		oldcurvemode = md->differential ;
//		if ( oldcurvemode == 0 )
//		{
//			if ( md->curve <= -28 )
//			{
//				oldcurvemode = 2 ;
//			}
//		}
		
//		int newcurvemode = ui->diffcurveCB->currentIndex() ;
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

//		int j = 127 ;
//		j &= ~( ui->Fm0CB->checkState() ? 1 : 0 ) ;
//		j &= ~( ui->Fm1CB->checkState() ? 2 : 0 ) ;
//		j &= ~( ui->Fm2CB->checkState() ? 4 : 0 ) ;
//		j &= ~( ui->Fm3CB->checkState() ? 8 : 0 ) ;
//		j &= ~( ui->Fm4CB->checkState() ? 16 : 0 ) ;
//		j &= ~( ui->Fm5CB->checkState() ? 32 : 0 ) ;
//		j &= ~( ui->Fm6CB->checkState() ? 64 : 0 ) ;
//		md->modeControl = j ;

////    if(ui->FMtrimChkB->checkState())
////        ui->offset_label->setText("FmTrimVal");
////    else
////        ui->offset_label->setText("Offset");

//    mixCommennt->clear();
//    mixCommennt->append(ui->mixerComment->toPlainText());
		
//		ValuesEditLock = false ;
//}



