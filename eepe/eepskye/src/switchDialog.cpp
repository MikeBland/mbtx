#include "SwitchDialog.h"
#include "ui_SwitchDialog.h"
#include "pers.h"
#include "file.h"
#include "helpers.h"

SwitchDialog::SwitchDialog(QWidget *parent, int index, struct t_switchData *sdata, int modelVersion, uint8_t id, struct t_radioData *rData ) :
    QDialog(parent),
    ui(new Ui::SwitchDialog)
{
  char text[4] ;
  ui->setupUi(this) ;
	sindex = index ;
	lsdata = sdata ;
	lrData = rData ;
	
	leeType = rData->type ;
	lextraPots = rData->extraPots ;

	int type = leeType ;
	if ( type == RADIO_TYPE_TPLUS )
	{
		if ( rData->sub_type == 1 )
		{
			type = RADIO_TYPE_X9E ;
		}
	}
	ltype = type ;
	id_model = id ;
	
	lmodelVersion = modelVersion ;
	text[0] = 'L' ;
	text[1] = 'S' ;
	text[2] = ( index < 9 ) ? index + '1' : index + 'A' - 9	;
	text[3] = '\0' ;
	ui->switchNameLabel->setText( text ) ;

	ValuesEditLock = true ;
	OnEditLock = true ;
	OffEditLock = true ;
	update() ;
	ValuesEditLock = false ;
	OnEditLock = false ;
	OffEditLock = false ;
  connect(ui->functionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->v1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->v2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->sw1CB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->sw2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->andSwFuncCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->andSwCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->valSB,SIGNAL(valueChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->val2SB,SIGNAL(valueChanged(int)),this,SLOT(valuesChanged()));
	connect(ui->timeOnSB,SIGNAL(valueChanged(double)),this,SLOT(valuesChanged()));
  connect(ui->timeOffSB,SIGNAL(valueChanged(double)),this,SLOT(valuesChanged()));
  connect(ui->delaySB,SIGNAL(valueChanged(double)),this,SLOT(valuesChanged()));

	ui->v1CB->setVisible(true) ;
	ui->v1Label->setVisible(true) ;
	ui->sw1CB->setVisible(false) ;
	ui->sw1Label->setVisible(false) ;
	ui->sw2CB->setVisible(false) ;
	ui->sw2Label->setVisible(false) ;
	ui->valSB->setVisible(true) ;
	ui->valLabel->setVisible(true) ;
	ui->v2CB->setVisible(false) ;
	ui->v2Label->setVisible(false) ;
  ui->timeOnSB->setVisible(false);
  ui->timeOffSB->setVisible(false);
	ui->delaySB->setVisible(true) ;
}

SwitchDialog::~SwitchDialog()
{
    delete ui;
}

void SwitchDialog::valuesChanged()
{
	uint32_t cType ;
	if ( ValuesEditLock )
	{
		return ;
	}
	ValuesEditLock = true ;
	cType = CS_STATE( lsdata->swData.func, lmodelVersion ) ;
	lsdata->swData.func = unmapSwFunc( ui->functionCB->currentIndex() ) ;
	lsdata->swData.exfunc = ui->andSwFuncCB->currentIndex() ;
  lsdata->switchDelay = ui->delaySB->value() * 10 ;
	switch ( cType )
	{
    case CS_VOFS:
    case CS_2VAL:
    {
      int16_t value ;
      value = ui->v1CB->currentIndex() ;
			value = decodePots( value, ltype, lextraPots ) ;
		  lsdata->swData.v1 = value ;
			lsdata->swData.v2 = ui->valSB->value() ;
			if ( cType == CS_2VAL )
			{
				lsdata->swData.bitAndV3 = ui->val2SB->value() ;
			}
    }
		break ;
		case CS_U16:
		{
			int16_t val ;
      int16_t value ;
      val = ui->valSB->value() ;
			value = ui->v1CB->currentIndex() ;
			value = decodePots( value, ltype, lextraPots ) ;
		  lsdata->swData.v1 = value ;
			lsdata->swData.v2 = val ;
			lsdata->swData.bitAndV3 = val >> 8 ;
		}
		break ;
		case CS_VBOOL:
			lsdata->swData.v1 = getSwitchCbValue( ui->sw1CB, lrData->type ) ;
      lsdata->swData.v2 = getSwitchCbValue( ui->sw2CB, lrData->type ) ;
		break ;
    case CS_TMONO:
			lsdata->swData.v1 = getSwitchCbValue( ui->sw1CB, lrData->type ) ;
			int32_t x ;
      x = ui->timeOnSB->value() * 10.0 + 0.5 ;
			if ( x > 49 )
			{
				x /= 10 ;
				x -= 1 ;
			}
			else
			{
				if ( x == 10 )
				{
					x = 0 ;
				}
				else
				{
					x = -x ;
				}
			}
			lsdata->swData.v2 = x ;
		break ;
    case CS_TIMER:
		{
			int32_t x ;
      x = ui->timeOnSB->value() * 10.0 + 0.5 ;
			if ( x > 49 )
			{
				x /= 10 ;
				x -= 1 ;
			}
			else
			{
				if ( x == 10 )
				{
					x = 0 ;
				}
				else
				{
					x = -x ;
				}
			}
			lsdata->swData.v1 = x ;
      x = ui->timeOffSB->value() * 10.0 + 0.5 ;
			if ( x > 49 )
			{
				x /= 10 ;
				x -= 1 ;
			}
			else
			{
				if ( x == 10 )
				{
					x = 0 ;
				}
				else
				{
					x = -x ;
				}
			}
			lsdata->swData.v2 = x ;
		}			 
		break ;
    case CS_VCOMP:
    {
      int16_t value ;
      value = ui->v1CB->currentIndex() ;
			value = decodePots( value, ltype, lextraPots ) ;
		  lsdata->swData.v1 = value ;
      value = ui->v2CB->currentIndex() ;
			value = decodePots( value, ltype, lextraPots ) ;
		  lsdata->swData.v2 = value ;
    }
		break ;
		
	}

  if ( lrData->bitType & ( RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E  | RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_X9L | RADIO_BITTYPE_X10 | RADIO_BITTYPE_T16 | RADIO_BITTYPE_TX16S | RADIO_BITTYPE_TX18S) )
	{
    lsdata->swData.andsw = getSwitchCbValueShort( ui->andSwCB, 1 ) ;
	}
	else
	{
    lsdata->swData.andsw = getAndSwitchCbValue( ui->andSwCB ) ;
  }  
  update() ;
	ValuesEditLock = false ;
}

void SwitchDialog::update()
{
	uint32_t cType ;
	char telText[20] ;
	cType = CS_STATE( lsdata->swData.func, lmodelVersion ) ;
	int32_t value ;

	populateCSWCB( ui->functionCB, lsdata->swData.func, lmodelVersion);
	ui->delaySB->setValue( (double) lsdata->switchDelay / 10 ) ;
  if ( lrData->bitType & ( RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_X9L | RADIO_BITTYPE_X10 | RADIO_BITTYPE_T16 | RADIO_BITTYPE_TX16S | RADIO_BITTYPE_TX18S) )
	{
    x9dPopulateSwitchAndCB( ui->andSwCB, lsdata->swData.andsw ) ;
	}
	else
	{
		populateSwitchAndCB( ui->andSwCB, lsdata->swData.andsw ) ;
	}
	ui->andSwFuncCB->setCurrentIndex(lsdata->swData.exfunc) ;
	
	switch ( cType )
	{
    case CS_2VAL:
    case CS_VOFS:
		case CS_U16:
      populateSourceCB( ui->v1CB, lrData->generalSettings.stickMode, 1, lsdata->swData.v1, lmodelVersion, lrData->type, lrData->extraPots ) ;
			ui->v1CB->setVisible(true) ;
			ui->v1Label->setVisible(true) ;
			ui->sw1CB->setVisible(false) ;
			ui->sw1Label->setVisible(false) ;
			ui->sw2CB->setVisible(false) ;
			ui->sw2Label->setVisible(false) ;
			ui->valSB->setVisible(true) ;
			ui->valLabel->setVisible(true) ;
			ui->v2CB->setVisible(false) ;
			ui->v2Label->setVisible(false) ;
      ui->timeOnSB->setVisible(false);
      ui->timeOffSB->setVisible(false);
      ui->timeOnSB->setAccelerated(false);
      ui->timeOffSB->setAccelerated(false);
      ui->timeOnLabel->setVisible(false);
      ui->timeOffLabel->setVisible(false);
			ui->andSwFuncCB->setEnabled(true) ;
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
				if ( y > 32767 )
				{
					y -= 65536 ;
				}
        ui->valSB->setValue( y ) ;
       	ui->valText->setVisible(false);
       	ui->val2Text->setVisible(false);
			}
			else
			{
       	ui->val2Text->setVisible(false);
        ui->valSB->setValue(lsdata->swData.v2);
				if ( lsdata->swData.v1u > 45 )
				{
        	ui->valText->setVisible(true) ;
          value = convertTelemConstant( lsdata->swData.v1u - 45, lsdata->swData.v2, &lrData->models[id_model] ) ;
          stringTelemetryChannel( telText, lsdata->swData.v1u - 45, value, &lrData->models[id_model] ) ;
        	ui->valText->setText(telText);
				}
				else
				{
        	ui->valText->setVisible(false);
				}
			}
			if ( cType == CS_2VAL )
			{
        ui->val2SB->setValue((int8_t)lsdata->swData.bitAndV3);
				ui->val2SB->setVisible(true) ;
        ui->val2Label->setVisible(true) ;
				if ( lsdata->swData.v1u > 45 )
				{
        	ui->val2Text->setVisible(true) ;
          value = convertTelemConstant( lsdata->swData.v1u - 45, lsdata->swData.bitAndV3, &lrData->models[id_model] ) ;
          stringTelemetryChannel( telText, lsdata->swData.v1u - 45, value, &lrData->models[id_model] ) ;
        	ui->val2Text->setText(telText);
				}
				else
				{
        	ui->val2Text->setVisible(false);
				}
			}
			else
			{
				ui->val2SB->setVisible(false) ;
        ui->val2Label->setVisible(false) ;
			}
		break ;
    
		case CS_VBOOL:
      ui->sw1CB->setVisible(true) ;
      ui->sw2CB->setVisible(true) ;
			ui->sw1Label->setVisible(true) ;
			ui->sw2Label->setVisible(true) ;
			ui->v1CB->setVisible(false) ;
			ui->v1Label->setVisible(false) ;
      
			ui->valSB->setVisible(false) ;
			ui->valLabel->setVisible(false) ;
			ui->v2CB->setVisible(false) ;
			ui->v2Label->setVisible(false) ;
      ui->timeOnSB->setVisible(false);
      ui->timeOffSB->setVisible(false);
      ui->timeOnLabel->setVisible(false);
      ui->timeOffLabel->setVisible(false);
			ui->val2SB->setVisible(false) ;
      ui->val2Label->setVisible(false) ;
     	ui->valText->setVisible(false);
     	ui->val2Text->setVisible(false);
			ui->andSwFuncCB->setEnabled(true) ;
      
			populateSwitchCB( ui->sw1CB, lsdata->swData.v1, lrData->type) ;
      populateSwitchCB( ui->sw2CB, lsdata->swData.v2, lrData->type) ;
			
//			cswitchText1[i]->setVisible(false) ;
//			cswitchText2[i]->setVisible(false) ;
		break ;
    case CS_TMONO:
      ui->sw1CB->setVisible(true) ;
			ui->sw1Label->setVisible(true) ;
      ui->sw2CB->setVisible(false) ;
			ui->sw2Label->setVisible(false) ;
			ui->v1CB->setVisible(false) ;
			ui->v1Label->setVisible(false) ;
			ui->val2SB->setVisible(false) ;
      ui->val2Label->setVisible(false) ;
			ui->v2CB->setVisible(false) ;
			ui->v2Label->setVisible(false) ;

			ui->valSB->setVisible(false) ;
			ui->valLabel->setVisible(false) ;
//			ui->valSB->setAccelerated(true) ;
//      ui->valSB->setValue(lsdata->swData.v1);
      
			ui->timeOnSB->setVisible(true);
      ui->timeOffSB->setVisible(false);
      ui->timeOnSB->setAccelerated(true);
      ui->timeOnLabel->setVisible(true);
      ui->timeOffLabel->setVisible(false);
     	ui->valText->setVisible(false);
     	ui->val2Text->setVisible(false);
			ui->andSwFuncCB->setEnabled(false) ;
			ui->andSwFuncCB->setCurrentIndex(0) ;
			populateSwitchCB( ui->sw1CB, lsdata->swData.v1, lrData->type) ;
			value = lsdata->swData.v2+1 ;
	    ui->timeOnSB->setMaximum(100);
      ui->timeOnSB->setMinimum(0.1);
//      ui->spinBox->setValue( value ) ;
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
		break ;
    case CS_VCOMP:
			ui->v1CB->setVisible(true) ;
			ui->v1Label->setVisible(true) ;
			ui->v2CB->setVisible(true) ;
			ui->v2Label->setVisible(true) ;
			ui->sw1CB->setVisible(false) ;
			ui->sw1Label->setVisible(false) ;
			ui->sw2CB->setVisible(false) ;
			ui->sw2Label->setVisible(false) ;
			ui->valSB->setVisible(false) ;
			ui->valLabel->setVisible(false) ;
			ui->val2SB->setVisible(false) ;
      ui->val2Label->setVisible(false) ;
      ui->timeOnSB->setVisible(false);
      ui->timeOffSB->setVisible(false);
      ui->timeOnLabel->setVisible(false);
      ui->timeOffLabel->setVisible(false);
     	ui->valText->setVisible(false);
     	ui->val2Text->setVisible(false);
			ui->andSwFuncCB->setEnabled(true) ;
      populateSourceCB( ui->v1CB, lrData->generalSettings.stickMode, 1, lsdata->swData.v1, lmodelVersion, lrData->type, lrData->extraPots ) ;
      populateSourceCB( ui->v2CB, lrData->generalSettings.stickMode, 1, lsdata->swData.v2, lmodelVersion, lrData->type, lrData->extraPots ) ;
    break ;
    case CS_TIMER:
			ui->v1CB->setVisible(false) ;
			ui->v1Label->setVisible(false) ;
			ui->val2SB->setVisible(false) ;
      ui->val2Label->setVisible(false) ;
			ui->sw1CB->setVisible(false) ;
			ui->sw1Label->setVisible(false) ;
			ui->sw2CB->setVisible(false) ;
			ui->sw2Label->setVisible(false) ;
      ui->timeOnSB->setVisible(true);
      ui->timeOffSB->setVisible(true);
      ui->timeOnSB->setAccelerated(true);
      ui->timeOffSB->setAccelerated(true);
      ui->timeOnLabel->setVisible(true);
      ui->timeOffLabel->setVisible(true);
			ui->valSB->setVisible(false) ;
			ui->valLabel->setVisible(false) ;
			ui->v2CB->setVisible(false) ;
			ui->v2Label->setVisible(false) ;
	    ui->timeOnSB->setMaximum(100);
      ui->timeOnSB->setMinimum(0.1);
	    ui->timeOffSB->setMaximum(100);
      ui->timeOffSB->setMinimum(0.1);
     	ui->valText->setVisible(false);
     	ui->val2Text->setVisible(false);
			ui->andSwFuncCB->setEnabled(false) ;
			ui->andSwFuncCB->setCurrentIndex(0) ;
			value = lsdata->swData.v1+1 ;
//      ui->spinBox->setValue( value ) ;
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
			lastOnTime = ui->timeOnSB->value() ;
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
			lastOffTime = ui->timeOffSB->value() ;
    break ;
	}
	ui->delaySB->setValue((float)lsdata->switchDelay/10.0) ;

}

void SwitchDialog::on_timeOnSB_valueChanged( double x )
{
	if ( OnEditLock )
	{
		return ;
	}
	OnEditLock = true ;
	
	if ( x > 4.9 )
	{
		ui->timeOnSB->setSingleStep( 1.0 ) ;
	}
	else if ( x < 5.1 )
	{
		ui->timeOnSB->setSingleStep( 0.1 ) ;
	}
	if ( ( x == 4.0 ) && ( lastOnTime == 5.0 ) )
	{
		x = 4.9 ;
    ui->timeOnSB->setValue(x) ;
	}
	lastOnTime = x ;
	OnEditLock = false ;
}

void SwitchDialog::on_timeOffSB_valueChanged( double x )
{
	if ( OffEditLock )
	{
		return ;
	}
	OffEditLock = true ;
	if ( x > 4.9 )
	{
		ui->timeOffSB->setSingleStep( 1.0 ) ;
	}
	else if ( x < 5.1 )
	{
		ui->timeOffSB->setSingleStep( 0.1 ) ;
	}
	if ( ( x == 4.0 ) && ( lastOffTime == 5.0 ) )
	{
		x = 4.9 ;
    ui->timeOffSB->setValue(x) ;
	}
	lastOffTime = x ;
	OffEditLock = false ;
}

