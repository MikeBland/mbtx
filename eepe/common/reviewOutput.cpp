#include "reviewOutput.h"
#include "ui_reviewOutput.h"

//#include "mainwindow.h"
#include <QtGui>
#include <QString>
#include <stdint.h>

extern QString AvrdudeOutput ;

reviewOutput::reviewOutput(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::reviewOutput)
{
  ui->setupUi(this) ;
	ui->outputText->setText( AvrdudeOutput ) ;
	xChecked = 0 ;
	ui->checkBox->setVisible( false ) ;
}

reviewOutput::~reviewOutput()
{
	if ( ui->checkBox->isChecked() )
	{
		if ( xChecked )
		{
			*xChecked = true ;		
		}
	}
  delete ui ;
}

void reviewOutput::showCheck( int *checked, QString title,  QString text )
{
	int lchecked = false ;
	if ( checked )
	{
		lchecked = *checked ;
		xChecked = checked ;
		ui->checkBox->setVisible( true ) ;
	}
	ui->checkBox->setChecked( lchecked ) ;
  this->setWindowTitle( title ) ;
	ui->outputText->setText( text ) ;

}


