#include "VoiceAlarmDialog.h"
#include "ui_VoiceAlarmDialog.h"
#include "pers.h"
#include "helpers.h"
#include "myeeprom.h"

VoiceAlarmDialog::VoiceAlarmDialog(QWidget *parent, VoiceAlarmData *invad, int eeType, int stickmode, int modelVersion, ModelData *pModel ) :
    QDialog(parent),
    ui(new Ui::VoiceAlarmDialog)
{
  ui->setupUi(this);
	leeType = eeType ;
  lpModel = pModel ;
	vad = invad ;
  populateSourceCB( ui->SourceCB, stickmode, 1, vad->source, modelVersion ) ; // , eeType ) ;
	populateSwitchCB( ui->SwitchCB, vad->swtch, eeType ) ;
	ui->FunctionCB->setCurrentIndex( vad->func ) ;
	ui->RateCB->setCurrentIndex( vad->rate ) ;
	ui->HapticCB->setCurrentIndex( vad->haptic ) ;
	ui->ValueSB->setValue( vad->offset ) ;
	ui->MuteCB->setCurrentIndex( vad->mute ) ;
	ui->FileTypeCB->setCurrentIndex( vad->fnameType ) ;
	ui->PlaySourceCB->setCurrentIndex( vad->vsource ) ;
	populateAlarmCB( ui->AudioCB, vad->vfile ) ;

	switch ( vad->fnameType )
	{
		case 1 :
//      ui->FileName->setText( (char *)vad->file.name ) ;
			ui->FileNumberSB->setValue( vad->vfile ) ;
		break ;
		case 2 :
//			ui->FileNumberSB->setValue( vad->file.vfile ) ;
			ui->AudioCB->setCurrentIndex( vad->vfile ) ;
		break ;
//		case 3 :
//			ui->AudioCB->setCurrentIndex( vad->file.vfile ) ;
//		break ;
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
//		case 1 :
//			ui->FileName->show() ;
//			ui->FileNumberSB->hide() ;
//			ui->FileNumberSB->setValue( 0 ) ;
//			ui->AudioCB->hide() ;
//			ui->AudioCB->setCurrentIndex( 0 ) ;
//		break ;
		case 1 :
			ui->FileName->hide() ;
  	  ui->FileName->setText( "" ) ;
			ui->FileNumberSB->show() ;
			ui->AudioCB->hide() ;
			ui->AudioCB->setCurrentIndex( 0 ) ;
		break ;
		case 2 :
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
	vad->swtch = getSwitchCbValue( ui->SwitchCB, leeType ) ;
//	int limit = MAX_DRSWITCH ;
//  if ( leeType )
//	{
//   	limit += EXTRA_CSW ;
//	}
//  vad->swtch = ui->SwitchCB->currentIndex() - limit ;
	vad->source = ui->SourceCB->currentIndex() ;
	vad->func = ui->FunctionCB->currentIndex() ;
	vad->rate = ui->RateCB->currentIndex() ;
	vad->haptic = ui->HapticCB->currentIndex() ;
	vad->mute = ui->MuteCB->currentIndex() ;
	vad->offset = ui->ValueSB->value() ;
	vad->fnameType = ui->FileTypeCB->currentIndex() ;
	vad->vsource = ui->PlaySourceCB->currentIndex() ;
	switch ( vad->fnameType )
	{
		case 1 :
      vad->vfile = ui->FileNumberSB->value() ;
		break ;
		case 2 :
      vad->vfile = ui->AudioCB->currentIndex() ;
		break ;
//    	memset(&vad->file.name,'\0',sizeof(vad->file.name));
//    	for(quint8 i=0; i<(ui->FileName->text().length()); i++)
//	    {
//        if(i>=sizeof(vad->file.name)) break;
//        vad->file.name[i] = ui->FileName->text().toStdString()[i] ;
//  	  }
//		break ;
//		case 2 :
//      vad->file.vfile = ui->FileNumberSB->value() ;
//		break ;
//		case 3 :
//      vad->file.vfile = ui->AudioCB->currentIndex() ;
//		break ;
	}

	updateDisplay() ;
}

