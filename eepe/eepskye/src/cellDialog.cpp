#include "ui_cellDialog.h"
#include "pers.h"
#include "helpers.h"
#include "file.h"
#include "cellDialog.h"

#include <QDialog>
#include <QtGui>

cellDialog::cellDialog(QWidget *parent, struct t_cellData *inData ) :
    QDialog(parent),
    ui(new Ui::cellDialog)
{
  ui->setupUi(this) ;
	this->setWindowTitle( tr("Cell Scaling") ) ;
	pData = inData ;

  ui->Cell1DSB->setValue( float( pData->factors[0] + 1000 ) / 1000.0 ) ;
  ui->Cell2DSB->setValue( float( pData->factors[1] + 1000 ) / 1000.0 ) ;
  ui->Cell3DSB->setValue( float( pData->factors[2] + 1000 ) / 1000.0 ) ;
  ui->Cell4DSB->setValue( float( pData->factors[3] + 1000 ) / 1000.0 ) ;
  ui->Cell5DSB->setValue( float( pData->factors[4] + 1000 ) / 1000.0 ) ;
  ui->Cell6DSB->setValue( float( pData->factors[5] + 1000 ) / 1000.0 ) ;
  ui->Cell7DSB->setValue( float( pData->factors[6] + 1000 ) / 1000.0 ) ;
  ui->Cell8DSB->setValue( float( pData->factors[7] + 1000 ) / 1000.0 ) ;
  ui->Cell9DSB->setValue( float( pData->factors[8] + 1000 ) / 1000.0 ) ;
  ui->Cell10DSB->setValue( float( pData->factors[9] + 1000 ) / 1000.0 ) ;
  ui->Cell11DSB->setValue( float( pData->factors[10] + 1000 ) / 1000.0 ) ;
  ui->Cell12DSB->setValue( float( pData->factors[11] + 1000 ) / 1000.0 ) ;
	connect( ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}

cellDialog::~cellDialog()
{
    delete ui;
}

void cellDialog::accept()
{
	pData->factors[0] = (uint32_t)( ( ui->Cell1DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[1] = (uint32_t)( ( ui->Cell2DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[2] = (uint32_t)( ( ui->Cell3DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[3] = (uint32_t)( ( ui->Cell4DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[4] = (uint32_t)( ( ui->Cell5DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[5] = (uint32_t)( ( ui->Cell6DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[6] = (uint32_t)( ( ui->Cell7DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[7] = (uint32_t)( ( ui->Cell8DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[8] = (uint32_t)( ( ui->Cell9DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[9] = (uint32_t)( ( ui->Cell10DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[10] = (uint32_t)( ( ui->Cell11DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	pData->factors[11] = (uint32_t)( ( ui->Cell12DSB->value() * 1000.0 ) + 0.1 ) - 1000 ;
	done(1) ;	
}


