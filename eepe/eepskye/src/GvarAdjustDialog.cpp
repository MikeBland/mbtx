#include "GvarAdjustDialog.h"
#include "ui_GvarAdjustDialog.h"
#include "pers.h"
#include "file.h"
#include "helpers.h"
#include "myeeprom.h"

GvarAdjustDialog::GvarAdjustDialog(QWidget *parent, GvarAdjust *ingad, struct t_radioData *radioData ) :
    QDialog(parent),
    ui(new Ui::GvarAdjustDialog)
{
  ui->setupUi(this);
//	leeType = eeType ;
  rData = radioData ;
//  lpModel = pModel ;
	gad = ingad ;
	updateLock = 0 ;

	oldFunction = gad->function ;
	ui->gvarCB->setCurrentIndex( gad->gvarIndex ) ;
	ui->functionCB->setCurrentIndex( gad->function ) ;
  populateSwitchCB( ui->switchCB, gad->swtch, rData->type ) ;
	ui->valueSB->setValue( gad->switch_value ) ;
  populateSwitchCB( ui->switch2CB, gad->switch_value, rData->type ) ;
	
	updateDisplay() ;

  connect(ui->functionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->gvarCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->valueSB,SIGNAL(valueChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->switchCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->switch2CB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
}

GvarAdjustDialog::~GvarAdjustDialog()
{
    delete ui;
}

void GvarAdjustDialog::updateDisplay()
{
	switch ( gad->function )
	{
		case 0 :
		case 1 :
		case 2 :
		case 7 :
		case 8 :
      ui->switch2CB->hide() ;
      ui->switch2Label->hide() ;
			ui->valueSB->show() ;
			ui->valueLabel->show() ;
		break ;
		case 3 :
			populateGvarCB( ui->switch2CB, gad->switch_value, rData->type, rData->extraPots ) ;
      ui->switch2CB->show() ;
			ui->switch2Label->setText("Variable") ;
			ui->switch2Label->show() ;
			ui->valueSB->hide() ;
			ui->valueLabel->hide() ;
		break ;
		case 4 :
		case 5 :
		case 6 :
		  populateSwitchCB( ui->switch2CB, gad->switch_value, rData->type ) ;
			ui->switch2Label->setText("Switch 2") ;
      ui->switch2CB->show() ;
			ui->switch2Label->show() ;
			ui->valueSB->hide() ;
			ui->valueLabel->hide() ;
		break ;
	}
}

void GvarAdjustDialog::valuesChanged()
{
	if ( updateLock )
	{
		return ;
	}
  gad->function = ui->functionCB->currentIndex() ;
	gad->gvarIndex = ui->gvarCB->currentIndex() ;
  gad->swtch = getSwitchCbValue( ui->switchCB, rData->type ) ;

	int func = gad->function ;
	if ( ( func < 3 ) || ( func > 6 ) )
	{
	  gad->switch_value = ui->valueSB->value() ;
	}
	else
	{
		if ( func == 3 )	// Set V
		{
      gad->switch_value = ui->switch2CB->currentIndex() ;
		}
		else
		{
	    gad->switch_value = getSwitchCbValue( ui->switch2CB, rData->type ) ;
		}
	}
	if ( oldFunction != gad->function )
	{
		gad->switch_value = 0 ;
		oldFunction = gad->function ;
	}
	updateLock = 1 ;
	updateDisplay() ;
	updateLock = 0 ;
}

