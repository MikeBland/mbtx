#include "ui_loggingDialog.h"
#include "pers.h"
#include "helpers.h"
#include "file.h"
#include "loggingDialog.h"

#include <QDialog>
#include <QtGui>

#define LOG_STK_THR 100
#define LOG_STK_AIL 101
#define LOG_STK_ELE 102
#define LOG_STK_RUD 103

#define LOG_BTRX  125
#define LOG_LAT	  126
#define LOG_LONG  127

QString ExTelemItems[] = {
	"Lat ",
	"Long",
	"BtRx",
	"Stk-THR",
	"Stk-AIL",
	"Stk-ELE",
	"Stk-RUD"
} ;

#define NUM_TEL_ITEMS	79

loggingDialog::loggingDialog(QWidget *parent, struct t_loggingData *inData, struct t_radioData *rData ) :
    QDialog(parent),
    ui(new Ui::loggingDialog)
{
	uint32_t i ;
	uint32_t mask ;
	uint32_t offset ;
  ui->setupUi(this) ;
	this->setWindowTitle( tr("Logging") ) ;
	pData = inData ;
	lData = pData->logBits ;
	lrData = rData ;

  populateSwitchCB(ui->LogSwitchCB, pData->lswitch, rData->type ) ;
	i = pData->rate ;
	if ( i == 2 )
	{
    i = 0 ;
	}
	else
	{
		i += 1 ;
	}
  ui->LogRateCB->setCurrentIndex( i ) ;
	ui->newFileCB->setChecked( pData->newFile ) ;
	ui->timeoutDSB->setValue( (float)pData->timeout / 10.0 + 2.5 ) ;

  for ( i = 0 ; i < NUM_TEL_ITEMS + 7 ; i += 1 )
	{
		uint32_t index = i ;
    QListWidgetItem* item ;
		QString str ;

		if ( i >= NUM_TEL_ITEMS )
		{
			switch ( i )
			{
				case NUM_TEL_ITEMS :
					index = LOG_LAT ;
				break ;
				case NUM_TEL_ITEMS + 1 :
					index = LOG_LONG ;
				break ;
				case NUM_TEL_ITEMS + 2 :
					index = LOG_BTRX ;
				break ;
				case NUM_TEL_ITEMS + 3 :
				case NUM_TEL_ITEMS + 4 :
				case NUM_TEL_ITEMS + 5 :
				case NUM_TEL_ITEMS + 6 :
					index = LOG_STK_THR + i - (NUM_TEL_ITEMS+3) ;
				break ;
			}

//			index = LOG_STK_THR + k - (7+sizeof(LogLookup)) ;
			str = ExTelemItems[i-NUM_TEL_ITEMS] ;
			
		}
		else
		{
			str = getTelemString( i + 1 ) ;
		}

		offset = index >> 5 ;
		mask = 1 << (index & 0x1F) ;
		
		item = new QListWidgetItem( str, ui->listWidget ) ;
		if ( ( i != 45 ) && ( i != 46 ) )
		{
	    item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
  	  item->setCheckState(lData[offset] & mask ? Qt::Unchecked : Qt::Checked ) ; // AND initialize check state
		}
		ui->listWidget->addItem( item) ;
		
//		cb[i] = new QCheckBox(this) ;
		
//		cb[i]->setLayoutDirection(Qt::RightToLeft) ;
//	  cb[i]->setText( getTelemString( i + 1) ) ;
//    ui->listWidget->addItem( cb[i] ) ;
//		ui->tableWidget->setCellWidget( i, 0, cb[i] ) ;
	}
	connect( ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}

loggingDialog::~loggingDialog()
{
    delete ui;
}

void loggingDialog::accept()
{
	uint32_t i ;
	uint32_t mask ;
	uint32_t offset ;

  for ( i = 0 ; i < NUM_TEL_ITEMS + 7 ; i += 1 )
	{
		uint32_t index = i ;
    QListWidgetItem* item = ui->listWidget->item( i ) ;
		if ( i >= NUM_TEL_ITEMS )
		{
			switch ( i )
			{
				case NUM_TEL_ITEMS :
					index = LOG_LAT ;
				break ;
				case NUM_TEL_ITEMS + 1 :
					index = LOG_LONG ;
				break ;
				case NUM_TEL_ITEMS + 2 :
					index = LOG_BTRX ;
				break ;
				case NUM_TEL_ITEMS + 3 :
				case NUM_TEL_ITEMS + 4 :
				case NUM_TEL_ITEMS + 5 :
				case NUM_TEL_ITEMS + 6 :
					index = LOG_STK_THR + i - (NUM_TEL_ITEMS+3) ;
				break ;
			}
		}
		offset = index >> 5 ;
		mask = 1 << (index & 0x1F) ;
		if ( item->checkState() == Qt::Checked )
		{
			lData[offset] &= ~mask ;
		}
		else
		{
			lData[offset] |= mask ;
		}
	}

	pData->lswitch = getSwitchCbValue( ui->LogSwitchCB, lrData->type ) ;
	i = ui->LogRateCB->currentIndex() ;
	if ( i == 0 )
	{
    i = 2 ;
	}
	else
	{
		i -= 1 ;
	}
	pData->rate = i ;
	pData->newFile = ui->newFileCB->isChecked() ? 1 : 0 ;
  pData->timeout = ( ui->timeoutDSB->value() - 2.45 ) * 10 ;
	done(1) ;	
}

void loggingDialog::on_setButton_clicked()
{
	uint32_t i ;
  for ( i = 0 ; i < NUM_TEL_ITEMS + 7 ; i += 1 )
	{
    QListWidgetItem* item = ui->listWidget->item( i ) ;
    item->setCheckState( Qt::Checked ) ;
	}
}

void loggingDialog::on_clearButton_clicked()
{
	uint32_t i ;
  for ( i = 0 ; i < NUM_TEL_ITEMS + 7 ; i += 1 )
	{
    QListWidgetItem* item = ui->listWidget->item( i ) ;
    item->setCheckState( Qt::Unchecked ) ;
	}
}


