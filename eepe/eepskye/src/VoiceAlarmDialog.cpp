#include "VoiceAlarmDialog.h"
#include "ui_VoiceAlarmDialog.h"
#include "pers.h"
#include "helpers.h"
#include "myeeprom.h"

VoiceAlarmDialog::VoiceAlarmDialog(QWidget *parent, VoiceAlarmData *invad, int eeType, int stickmode, int modelVersion, SKYModelData *pModel ) :
    QDialog(parent),
    ui(new Ui::VoiceAlarmDialog)
{
  ui->setupUi(this);
	leeType = eeType ;
  lpModel = pModel ;
	vad = invad ;
  populateSourceCB( ui->SourceCB, stickmode, 1, vad->source, modelVersion, eeType, 0 ) ;
	
//	uint32_t value ;
//	value = vad->source ;
//	if ( eeType )
//	{
//		if ( value >= EXTRA_POTS_POSITION )
//		{
//			if ( value >= EXTRA_POTS_START )
//			{
//				value -= ( EXTRA_POTS_START - EXTRA_POTS_POSITION ) ;
//			}
//			else
//			{
//				value += eeType == 2 ? 2 : NUM_EXTRA_POTS ;
//			}
//		}
//	}
//  ui->SourceCB->setCurrentIndex(value) ;

	populateSwitchCB( ui->SwitchCB, vad->swtch, eeType ) ;
  int x = ui->SwitchCB->count() ;
	ui->SwitchCB->addItem("Fmd") ;
	if ( vad->swtch == x / 2 + 1 )
	{
    ui->SwitchCB->setCurrentIndex(x) ;
	}
	ui->FunctionCB->setCurrentIndex( vad->func ) ;
	ui->RateCB->setCurrentIndex( vad->rate ) ;
	ui->HapticCB->setCurrentIndex( vad->haptic ) ;
	ui->ValueSB->setValue( vad->offset ) ;
	ui->MuteCB->setCurrentIndex( vad->mute ) ;
	ui->FileTypeCB->setCurrentIndex( vad->fnameType ) ;
	ui->PlaySourceCB->setCurrentIndex( vad->vsource ) ;
	ui->DelaySB->setValue( (float)vad->delay / 10.0 ) ;
	switch ( vad->fnameType )
	{
		case 1 :
      ui->FileName->setText( (char *)vad->file.name ) ;
		break ;
		case 2 :
			ui->FileNumberSB->setValue( vad->file.vfile ) ;
		break ;
		case 3 :
			ui->AudioCB->setCurrentIndex( vad->file.vfile ) ;
		break ;
	}
	updateDisplay() ;

  connect(ui->SourceCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->FunctionCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->SwitchCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->RateCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->HapticCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->MuteCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->ValueSB,SIGNAL(valueChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->FileTypeCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->DelaySB,SIGNAL(valueChanged(double)),this,SLOT(valuesChanged()));

  connect(ui->PlaySourceCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->FileName, SIGNAL(editingFinished()),this,SLOT(valuesChanged()));
  connect(ui->FileNumberSB,SIGNAL(valueChanged(int)),this,SLOT(valuesChanged()));
  connect(ui->AudioCB,SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));

}

VoiceAlarmDialog::~VoiceAlarmDialog()
{
    delete ui;
}

void VoiceAlarmDialog::updateDisplay()
{
	switch ( vad->fnameType )
	{
		case 0 :
			ui->FileName->hide() ;
			ui->FileNumberSB->hide() ;
			ui->AudioCB->hide() ;
  	  ui->FileName->setText( "" ) ;
			ui->FileNumberSB->setValue( 0 ) ;
			ui->AudioCB->setCurrentIndex( 0 ) ;
		break ;
		case 1 :
			ui->FileName->show() ;
			ui->FileNumberSB->hide() ;
			ui->FileNumberSB->setValue( 0 ) ;
			ui->AudioCB->hide() ;
			ui->AudioCB->setCurrentIndex( 0 ) ;
		break ;
		case 2 :
			ui->FileName->hide() ;
  	  ui->FileName->setText( "" ) ;
			ui->FileNumberSB->show() ;
			ui->AudioCB->hide() ;
			ui->AudioCB->setCurrentIndex( 0 ) ;
		break ;
		case 3 :
			ui->FileName->hide() ;
  	  ui->FileName->setText( "" ) ;
			ui->FileNumberSB->hide() ;
			ui->FileNumberSB->setValue( 0 ) ;
			ui->AudioCB->show() ;
		break ;
	}
  if ( vad->source > 44 )
	{
		char telText[20] ;
    stringTelemetryChannel( telText, vad->source - 45, vad->offset, lpModel ) ;
    ui->TelemValue->setText(telText) ;
		ui->TelemValue->show() ;
	}
	else
	{
		ui->TelemValue->hide() ;
	}
}

void VoiceAlarmDialog::valuesChanged()
{
//	if ( ui->SwitchCB->currentIndex() == ui->SwitchCB->count() )
//	{
//		vad->swtch = limit + 1 ;
//	}
//	else
//	{
		vad->swtch = getSwitchCbValue( ui->SwitchCB, leeType ) ;
//	}
  uint32_t value ;
	value = ui->SourceCB->currentIndex() ;
  value = decodePots( value, leeType, 0 ) ; // Needs to be extraPots
	vad->source = value ;
	vad->func = ui->FunctionCB->currentIndex() ;
	vad->rate = ui->RateCB->currentIndex() ;
	vad->haptic = ui->HapticCB->currentIndex() ;
	vad->mute = ui->MuteCB->currentIndex() ;
	vad->offset = ui->ValueSB->value() ;
	vad->fnameType = ui->FileTypeCB->currentIndex() ;
	vad->vsource = ui->PlaySourceCB->currentIndex() ;
	vad->delay = (ui->DelaySB->value() + 0.05 ) * 10 ;
	switch ( vad->fnameType )
	{
		case 1 :
    	memset(&vad->file.name,'\0',sizeof(vad->file.name));
    	for(quint8 i=0; i<(ui->FileName->text().length()); i++)
	    {
        if(i>=sizeof(vad->file.name)) break;
        vad->file.name[i] = ui->FileName->text().toStdString()[i] ;
  	  }
		break ;
		case 2 :
      vad->file.vfile = ui->FileNumberSB->value() ;
		break ;
		case 3 :
      vad->file.vfile = ui->AudioCB->currentIndex() ;
		break ;
	}

	updateDisplay() ;
}

