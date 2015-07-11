#include "serialdialog.h"
#include "ui_serialdialog.h"
#include "stamp-eepe.h"
#include "mainwindow.h"
#include <QtGui>
#include "qextserialenumerator.h"
#include <QtCore/QList>
#include <QString>
#include <stdint.h>
#include <QFileDialog>
#include <QSettings>

extern int DebugMode ;

serialDialog::serialDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::serialDialog)
{
  QString temp ;
  ui->setupUi(this);

	fileListContents = NO_LIST ;
	ui->receiveButton->setDisabled( true ) ;
	ui->deleteButton->setDisabled( true ) ;
	ui->sendButton->setDisabled( true ) ;
	ui->progressBar->hide() ;
	if ( DebugMode == 0 )
	{
//		ui->monitorText->hide() ;
	}

  QSettings settings("er9x-eePe", "eePe");
  ComPort = settings.value("SerialSdPort").toString() ;

	comPortLoading = 1 ;
	QList<QextPortInfo> ports = QextSerialEnumerator::getPorts() ;
	ui->portCB->clear() ;
	int i = 0 ;
	int j = 0 ;
  foreach (QextPortInfo info, ports)
	{
		if ( info.portName != "" )
		{
  		ui->portCB->addItem(info.portName) ;
			if ( ComPort == info.portName )
			{
				j = i ;
			}
			i += 1 ;
		}
	}

  temp = settings.value("SerialSdRxDir", "").toString() ;
	ui->receiveDirectory->setText(temp) ;

	comPortLoading = 0 ;
	port = NULL ;

  ui->portCB->setCurrentIndex( j ) ;
}

void serialDialog::on_portCB_currentIndexChanged( int index )
{
	if ( comPortLoading )
	{
		return ;
	}
  ComPort = ui->portCB->currentText() ;
	
  QSettings settings("er9x-eePe", "eePe");
  settings.setValue("SerialSdPort", ComPort ) ;
}

serialDialog::~serialDialog()
{
	if ( port )
	{
		if (port->isOpen())
		{
    	port->close();
		}
    delete port ;
	}
  delete ui;
}

void serialDialog::on_cancelButton_clicked()
{
	if ( port )
	{
 		port->close();
  	delete port ;
		port = NULL ;
	}
	done(0) ;	
}


void serialDialog::on_FileEdit_editingFinished()
{
//    fileToSend = ui->FileEdit->text();
}

void serialDialog::on_browseButton_clicked()
{
//    QString fileName = QFileDialog::getOpenFileName(this, tr("File(s) To Send"),ui->FileEdit->text());
  QString dir ;
	QSettings settings("er9x-eePe", "eePe");
  dir = settings.value("SerialSdSendDir").toString() ;
		QStringList filesList = QFileDialog::getOpenFileNames(this, tr("File(s) To Send"), dir );
		ui->fileList->clear() ;
		ui->fileList->addItems( filesList ) ;
		fileListContents = LIST_FILES ;
		ui->receiveButton->setDisabled( true ) ;
		ui->deleteButton->setDisabled( true ) ;
		ui->sendButton->setDisabled( false ) ;

  if ( !filesList.isEmpty() )
	{
    dir = filesList.first() ;
    dir = QFileInfo(dir).dir().absolutePath() ;
		settings.setValue("SerialSdSendDir", dir ) ;
	}
}

void waitMs( int ms )
{
  QTime Time ;
  Time.start() ;

  while( Time.elapsed() < ms )
	{
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}
}

QString to8_3( QString name )
{
	int i ;
	int j ;
	int dotPos1 ;
	int dotPosn ;
	QString output ;

	j = name.length() ;

	dotPos1 = name.indexOf( "." ) ;
	dotPosn = name.lastIndexOf( ".", -1 ) ;

	if ( dotPos1 == -1 )
	{
		output = name.left(8) ;
	}
	else
	{
		if ( dotPos1 > 8 )
		{
			dotPos1 = 8 ;
		}
		output = name.left(dotPos1) ;
		output.append(".") ;
		i = j - dotPosn ;
		if ( i > 3 )
		{
			i = 3 ;			
		}
    output.append( name.mid( dotPosn+1, i ) ) ;
	}
	return output.toUpper() ;
}

int serialDialog::startSerialPort()
{
	QString portname ;
  portname = ui->portCB->currentText() ;
#ifdef Q_OS_UNIX
  port = new QextSerialPort(portname, QextSerialPort::Polling) ;
#else
  port = new QextSerialPort(portname, QextSerialPort::Polling) ;
#endif /*Q_OS_UNIX*/
  port->setBaudRate(BAUD38400) ;
  port->setFlowControl(FLOW_OFF) ;
  port->setParity(PAR_NONE) ;
  port->setDataBits(DATA_8) ;
  port->setStopBits(STOP_1) ;
    //set timeouts to 500 ms
//  port->setTimeout(100) ;
  if (!port->open(QIODevice::ReadWrite | QIODevice::Unbuffered) )
  {
    QMessageBox::critical(this, "eePe", tr("Com Port Unavailable"));
		ui->listButton->setDisabled( false ) ;
  	delete port ;
		port = NULL ;
		return 1 ;	// Failed
	}
	return 0 ;
	
}


#define	CHECK_CHECKSUM	1
#define	SET_CHECKSUM		0

uint8_t checksumBlock( unsigned char *block, uint8_t check )
{
	uint16_t csum ;
	uint8_t i ;
	uint16_t actcsum ;

	csum = 0 ;
	block += 3 ;
	for ( i = 0 ; i < 128 ; i += 1 )
	{
		csum += *block++ ;		
	}
	if ( check == 0 )
	{
		*block++ = csum ;
		*block = csum >> 8 ;	
		return 0 ;
	}
	else
	{
		actcsum = *block++ ;
		actcsum |= *block << 8 ;
		return (actcsum == csum) ;
	}
}

#define BLOCK_AREA		136
#define SOH                     (0x01)  /* start of 128-byte data packet */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* end of transmission */
#define ACK                     (0x06)  /* acknowledge */
#define	STR_ACK									"\006"
#define NAK                     (0x15)  /* negative acknowledge */
#define	STR_NAK									"\025"
#define CA                      (0x18)  /* two of these in succession aborts transfer */
#define STR_CA									"\030"
#define	STR_CA_CA								"\030\030"
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */
#define	STR_CRC16								"C"

#define ABORT1                  (0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  (0x61)  /* 'a' == 0x61, abort by user */

#define ETX                     (0x03)  /* End of Transmission */


#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)
#define PACKET_TRAILER          (2)
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)

#define NAK_TIMEOUT             (200)	// Units of 1mS 
#define PACKET_TIMEOUT          (50)	// Units of 1mS 
#define MAX_ERRORS              (5)

#define FILE_NAME_LENGTH        (116)
#define FILE_SIZE_LENGTH        (16)


uint8_t DoubleEsc[2] = {27,27} ;
uint8_t DoubleCan[2] = {24,24} ;
uint8_t Abort[1] = {'a'} ;

QString Monitor ;

int serialDialog::waitForAckNak( int mS )
{
  char buff[150] ;
  int numBytes ;
  QByteArray qba ;
	int j ;
  int i ;
	int timer ;
  char acknak ;

	timer = 0 ;

	do
	{
  	waitMs( 10 ) ;
		
  	numBytes = port->bytesAvailable() ;
		if ( numBytes)
		{
  		if(numBytes > 148)
			{
  		  numBytes = 148 ;
			}

  		i = port->read(buff, numBytes);
  		if (i != -1)
  		  buff[i] = '\0';
  		else
  		  buff[0] = '\0';

  		qba = QByteArray::fromRawData(buff, i ) ;
			acknak = 0 ;
			for ( j = 0 ; j < i ; j += 1 )
			{
  		  if ( qba[j] == (char)NAK )
				{
					acknak = NAK ;
					break ;
				}
  		  if ( qba[j] == (char)ACK )
				{
					acknak = ACK ;
					break ;
				}
			}
			qba = qba.toHex() ;
			Monitor.append(qba);
			ui->monitorText->setText(Monitor) ;
			if ( acknak )
			{
				return acknak ;
			}
		}
		timer += 10 ;
	} while ( timer < mS ) ;
  return 0 ;
}

int serialDialog::waitForCan( int mS )
{
  char buff[150] ;
  int numBytes ;
  QByteArray qba ;
	int j ;
  int i ;
	int timer ;
  char acknak ;

	timer = 0 ;

	do
	{
  	waitMs( 10 ) ;
		
  	numBytes = port->bytesAvailable() ;
		if ( numBytes)
		{
  		if(numBytes > 148)
			{
  		  numBytes = 148 ;
			}

  		i = port->read(buff, numBytes);
  		if (i != -1)
  		  buff[i] = '\0';
  		else
  		  buff[0] = '\0';

  		qba = QByteArray::fromRawData(buff, i ) ;
			acknak = 0 ;
			for ( j = 0 ; j < i ; j += 1 )
			{
  		  if ( qba[j] == (char)CA )
				{
					acknak = CA ;
					break ;
				}
			}
			qba = qba.toHex() ;
			Monitor.append(qba);
			ui->monitorText->setText(Monitor) ;
			if ( acknak )
			{
				return acknak ;
			}
		}
		timer += 10 ;
	} while ( timer < mS ) ;
  return 0 ;
}

uint8_t NAKcount ;

void serialDialog::on_sendButton_clicked()
{
	// Send the file
  
	QString portname ;
  QString file ;
  QString temp ;
  QByteArray qba ;
	int i ;
	int numFiles ;

	ui->sendButton->setDisabled( true ) ;
	
	ui->progressBar->setValue( 0 ) ;
	ui->progressBar->show() ;
	
	if( ( numFiles = ui->fileList->count() ) == 0 )
	{
    QMessageBox::critical(this, "eePe", tr("No files specified"));
		ui->sendButton->setDisabled( false ) ;
		return ;
	}
	
	if ( startSerialPort() )
	{
		return ;
	}
	port->setTimeout(100) ;

	for ( i = 0 ; i < numFiles ; i += 1 )
	{
		ui->FileEdit->setText( ui->fileList->item( i )->text() );
		
		sendOneFile( ui->fileList->item( i )->text() ) ;
	}

	if ( port )
	{
 		port->close();
  	delete port ;
		port = NULL ;
	}

	ui->progressBar->hide() ;
	ui->sendButton->setDisabled( false ) ;
}


void serialDialog::on_setDirButton_clicked()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("Select Save Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
	ui->receiveDirectory->setText(directory) ;
  QSettings settings("er9x-eePe", "eePe");
	settings.setValue("SerialSdRxDir", directory ) ;
}

void serialDialog::on_listButton_clicked()
{
	QString portname ;
  QString file ;
  QString temp ;
  QByteArray qba ;
	int i ;
	int j ;
  int numBytes ;
  char buff[150] ;
	int timer ;

	ui->listButton->setDisabled( true ) ;
	
	if ( startSerialPort() )
	{
		return ;
	}
	
	ui->fileList->clear() ;
	fileListContents = NO_LIST ;
	ui->receiveButton->setDisabled( true ) ;
	ui->deleteButton->setDisabled( true ) ;
	ui->sendButton->setDisabled( true ) ;
  
  port->write( QByteArray::fromRawData ( ( char *)DoubleEsc, 2 ), 2 ) ;
  waitMs( 150 ) ;
  numBytes = port->bytesAvailable() ;
  if(numBytes > 148)
	{
    numBytes = 148 ;
	}
  buff[0] = '\0';
  i = port->read(buff, numBytes) ;	// Discard cr,lf,>
  if (i != -1)
	{
    buff[i] = '\0';
	}
	
	port->write( QByteArray::fromRawData ( "D", 1 ), 1 ) ;

	j = 0 ;
	timer = 0 ;
	ui->monitorText->setText("Monitor") ;
	Monitor = "Monitor" ;
	do
	{
		waitMs( 10 ) ;
  	numBytes = port->bytesAvailable() ;
		if ( numBytes)
		{
  		if(numBytes > 148)
			{
  		  numBytes = 148 ;
			}
  		i = port->read( &buff[j], numBytes);
  		j += i ;
			qba = QByteArray::fromRawData(buff, j ) ;
			
			Monitor.append("\n");
			Monitor.append(qba);
			ui->monitorText->setText(Monitor) ;

			if ( qba[0] == (char)ETX )
			{
				break ;
			}
			if ( qba[j-1] == '\r' )
			{
				qba[j-1] = '\0' ;
				ui->fileList->addItem( qba ) ;
				fileListContents = LIST_MODELS ;
				ui->receiveButton->setDisabled( false ) ;
				ui->deleteButton->setDisabled( false ) ;
				port->write( QByteArray::fromRawData ( "N", 1 ), 1 ) ;
				j = 0 ;
			}
		}
		timer += 10 ;
	} while ( timer < 500 ) ;

	if ( port )
	{
 		port->close();
  	delete port ;
		port = NULL ;
	}

	ui->listButton->setDisabled( false ) ;
}

void serialDialog::on_deleteButton_clicked()
{
	int numFiles ;
  unsigned char c;
  int numBytes ;
  unsigned char dataBlock[BLOCK_AREA] ;
	char acknak ;
  int i ;
  char buff[150] ;
	
	ui->receiveButton->setDisabled( true ) ;
	ui->deleteButton->setDisabled( true ) ;
	
  QList<QListWidgetItem*> files = ui->fileList->selectedItems() ;

  numFiles = files.size() ;
	if( numFiles == 0 )
	{
    QMessageBox::critical(this, "eePe", tr("No files specified"));
		ui->receiveButton->setDisabled( false ) ;
		ui->deleteButton->setDisabled( false ) ;
		return ;
	}

	if ( startSerialPort() )
	{
		return ;
	}

  QString filename = (files.at(0))->text() ;
	port->write( QByteArray::fromRawData ( ( char *)DoubleEsc, 2 ), 2 ) ;
	waitMs( 150 ) ;
	numBytes = port->bytesAvailable() ;
	if(numBytes > 148)
	{
		numBytes = 148 ;
	}
	buff[0] = '\0';
	i = port->read(buff, numBytes) ;	// Discard cr,lf,>
	if (i != -1)
	{
		buff[i] = '\0';
	}

	port->write( QByteArray::fromRawData ( "U", 1 ), 1 ) ;
  
	if (Receive_Byte(&c, 200) != 0)
	{
  	QMessageBox::critical(this, "eePe", tr("Comms Failure(1)"));
	 	return ;
	}
	if ( c != 'u' )
	{
  	QMessageBox::critical(this, "eePe", tr("Comms Failure(2)"));
		return ;
	}

	memset( dataBlock, 0, BLOCK_AREA ) ;
	dataBlock[0] = SOH ;
  dataBlock[1] = 0 ;
	dataBlock[2] = 13 ;
	strcpy( (char *)&dataBlock[3], filename.toLatin1() ) ;
	dataBlock[13] = 0 ;
	dataBlock[14] = 0 ;
	dataBlock[15] = 0 ;
	i = port->write( QByteArray::fromRawData ( ( char *)dataBlock, 15 ), 15 ) ;
 	
	waitMs( 40 ) ;
	acknak = waitForAckNak( 500 ) ;

//  ui->fileList->removeItemWidget( files[0] ) ;
	
	if ( port )
	{
 		port->close();
  	delete port ;
		port = NULL ;
	}
	ui->receiveButton->setDisabled( false ) ;
	ui->deleteButton->setDisabled( false ) ;
  on_listButton_clicked() ;
}

void serialDialog::on_receiveButton_clicked()
{
	int numFiles ;
//	int i ;

	ui->receiveButton->setDisabled( true ) ;
	ui->deleteButton->setDisabled( true ) ;
	
  QList<QListWidgetItem*> files = ui->fileList->selectedItems() ;

  numFiles = files.size() ;
	if( numFiles == 0 )
	{
    QMessageBox::critical(this, "eePe", tr("No files specified"));
		ui->receiveButton->setDisabled( false ) ;
		ui->deleteButton->setDisabled( false ) ;
		return ;
	}

	if ( ui->receiveDirectory->text() == "" )
	{
    QMessageBox::critical(this, "eePe", tr("No Receive Directory Specified"));
		ui->receiveButton->setDisabled( false ) ;
		return ;
	}

	if ( startSerialPort() )
	{
		return ;
	}

//	for ( i = 0 ; i < numFiles ; i += 1 )
//	{
    QString filename = (files.at(0))->text() + ".eepm" ;
		receiveOneFile( filename ) ;
//	}
	 
	if ( port )
	{
 		port->close();
  	delete port ;
		port = NULL ;
	}
	ui->receiveButton->setDisabled( false ) ;
	ui->deleteButton->setDisabled( false ) ;
}

//			  ((DWORD)(rtc.year - 1980) << 25)
//			| ((DWORD)rtc.month << 21)
//			| ((DWORD)rtc.mday << 16)
//			| ((DWORD)rtc.hour << 11)
//			| ((DWORD)rtc.min << 5)
//			| ((DWORD)rtc.sec >> 1);

void getFileTime( uint8_t *p, QString fname )
{
	QDateTime dt ;
	uint32_t udt = 0 ;
	uint32_t temp ;

  dt = QFileInfo( fname ).lastModified() ;
	temp = dt.date().year() ;
	temp -= 1980 ;
	udt |= temp << 25 ;
	temp = dt.date().month() ;
	udt |= temp << 21 ;
	temp = dt.date().day() ;
	udt |= temp << 16 ;
	temp = dt.time().hour() ;
	udt |= temp << 11 ;
	temp = dt.time().minute() ;
	udt |= temp << 5 ;
	temp = dt.time().second() ;
	udt |= temp >> 1 ;
	*p++ = udt ;
	*p++ = udt >> 8 ;
	*p++ = udt >> 16 ;
	*p++ = udt >> 24 ;
}




int serialDialog::sendOneFile( QString fname )
{
  QString file ;
  QString temp ;
  QString ext ;
	int blockNumber ;
	uint8_t dataBlock[BLOCK_AREA] ;
	int j ;
  int i ;
  char buff[150] ;
  int numBytes ;
  QByteArray qba ;
	int numBlocks ;
	char acknak ;
	int retryCount ;
	
	if(QFileInfo(fname).exists())
	{
		file = QFileInfo(fname).fileName() ;
		ext = QFileInfo(fname).suffix() ;

    if ( ext == "eepm" )
		{
			file = "\\MODELS\\" + file ;
		}
		else
		{
			file = to8_3( file ) ;
		}
		ui->Name->setText( file ) ;
    temp = tr("%1").arg(QFileInfo(fname).size() ) ;
    ui->size->setText( temp ) ;
	}
	else
	{
    QMessageBox::critical(this, "eePe", tr("File not Found"));
		ui->sendButton->setDisabled( false ) ;
    return 0 ;	// Failed
	}

	ui->monitorText->setText("Monitor") ;
	Monitor = "Monitor" ;

	Monitor.append(file.toLatin1());
	ui->monitorText->setText(Monitor) ;

  numBlocks = QFileInfo(fname).size() / 128 + 1 ;
	blockNumber = 0 ;

  memset( dataBlock, 0, BLOCK_AREA ) ;
	dataBlock[0] = SOH ;
	dataBlock[1] = blockNumber ;
	dataBlock[2] = ~blockNumber ;
	strcpy( (char *)&dataBlock[3], file.toLatin1() ) ;
  j = 4 + strlen( file.toLatin1() ) ;
  strcpy( (char *)&dataBlock[j], temp.toLatin1() ) ;
  j += strlen( temp.toLatin1() ) ;
	getFileTime( &dataBlock[j+1], fname ) ;	
	
	checksumBlock( dataBlock, SET_CHECKSUM ) ;

//  if (!port->open(QIODevice::ReadWrite | QIODevice::Unbuffered) )
//  {
//    QMessageBox::critical(this, "eePe", tr("Com Port Unavailable"));
//		ui->sendButton->setDisabled( false ) ;
//		return 0 ;	// Failed
//	}
  // Put receiver into file mode
  port->write( QByteArray::fromRawData ( ( char *)DoubleEsc, 2 ), 2 ) ;
  waitMs( 150 ) ;
  numBytes = port->bytesAvailable() ;
  if(numBytes > 148)
	{
    numBytes = 148 ;
	}
  buff[0] = '\0';
  i = port->read(buff, numBytes) ;	// Discard cr,lf,>
  if (i != -1)
	{
    buff[i] = '\0';
	}
//  qba = QByteArray::fromRawData(buff, i ) ;
//  qba = qba.toHex() ;
//	ui->receiveText_2->setText(qba) ;

	retryCount = 3 ;
	do
	{
  	port->write( QByteArray::fromRawData ( ( char *)Abort, 1 ), 1 ) ;
		acknak = waitForCan( 500 ) ;
		retryCount -= 1 ;
		
	} while ( ( acknak != CA ) && retryCount ) ;
	if ( acknak != CA )
	{
    QMessageBox::critical(this, "eePe", tr("No Sync."));
		ui->sendButton->setDisabled( false ) ;
		return 0 ;	// Failed
	}

	retryCount = 10 ;
	do
	{
		i = port->write( QByteArray::fromRawData ( ( char *)dataBlock, 128 + 5 ), 128 + 5 ) ;
//  	temp = tr("%1").arg(i) ;
//  	ui->sizeSent->setText( temp ) ;

  	waitMs( 40 ) ;

		acknak = waitForAckNak( 500 ) ;
		retryCount -= 1 ;

		if ( acknak != ACK )
		{
			NAKcount += 1 ;
//	    temp = tr("%1").arg(NAKcount) ;
//  	  ui->NAKcount->setText( temp ) ;
		}

	} while ( ( acknak != ACK ) && retryCount ) ;
	if ( acknak != ACK )
	{
		// Abort
//	 	port->close();
//	  delete port ;
//		port = NULL ;
    QMessageBox::critical(this, "eePe", tr("Communication Failure(1)"));
		ui->sendButton->setDisabled( false ) ;
		return 0 ;	// Failed
	}
	ui->progressBar->setValue( 100/(numBlocks+1) ) ;
		    
//  QString msg = QLatin1String(buff);
//  temp = tr("%1").arg(i) ;
//  ui->sizeSent->setText( temp ) ;

//  ui->receiveText->setText(acknak ? (acknak == ACK ? "ACK" : "NAK" ) : "NULL") ;

  QFile thisfile(fname) ;
	thisfile.open(QIODevice::ReadOnly) ;
	while ( blockNumber < numBlocks )
	{
		blockNumber += 1 ;
		memset( dataBlock, 0, BLOCK_AREA ) ;
		dataBlock[0] = SOH ;
		dataBlock[1] = blockNumber ;
		dataBlock[2] = ~blockNumber ;
    thisfile.read( (char *)&dataBlock[3], 128 ) ;
		checksumBlock( dataBlock, SET_CHECKSUM ) ;
		retryCount = 10 ;
		do
		{
			i = port->write( QByteArray::fromRawData ( ( char *)dataBlock, 128 + 5 ), 128 + 5 ) ;
//  		temp = tr("%1").arg(i) ;
//			ui->sizeSent->setText( temp ) ;

//    	qba = QByteArray::fromRawData((char *)dataBlock, 133 ) ;
//  		qba = qba.toHex() ;
//			ui->receiveText_2->setText(tr("Block %1").arg(blockNumber)) ;

			waitMs( 30 ) ;

			acknak = waitForAckNak( 500 ) ;
//  		temp = tr("%1").arg(i) ;
//  		ui->sizeSent->setText( temp ) ;
  		
//    	temp = tr("%1,%2").arg(acknak ? ((acknak == ACK ) ? "ACK" : "NAK" ): "NULL").arg(blockNumber) ;

//    	ui->receiveText->setText(temp) ;
			retryCount -= 1 ;
			if ( acknak != ACK )
			{
				NAKcount += 1 ;
//	  	  temp = tr("%1").arg(NAKcount) ;
//  		  ui->NAKcount->setText( temp ) ;
			}
		} while ( ( acknak != ACK ) && retryCount ) ;
		
		if ( acknak != ACK )
		{
			// Abort
	  	port->write( QByteArray::fromRawData ( ( char *)DoubleCan, 2 ), 2 ) ;
//	 		port->close();
//	  	delete port ;
//			port = NULL ;
    	QMessageBox::critical(this, "eePe", tr("Communication Failure(2)"));
			ui->sendButton->setDisabled( false ) ;
			thisfile.close() ;
			return 0 ;	// Failed
		}
	ui->progressBar->setValue( ((blockNumber+1)*100)/(numBlocks+1) ) ;
	}			 
	thisfile.close() ;
	dataBlock[0] = EOT ;
	i = port->write( QByteArray::fromRawData ( ( char *)dataBlock, 1 ), 1 ) ;
//  qba = QByteArray::fromRawData((char *)dataBlock, 1 ) ;
//	qba = qba.toHex() ;
//	ui->receiveText_2->setText(qba) ;
	waitMs( 30 ) ;
	acknak = waitForAckNak( 500 ) ;
//  temp = tr("%1,%2").arg(acknak ? ((acknak == ACK ) ? "ACK" : "NAK" ): "NULL").arg("EOT") ;
//  ui->receiveText->append(temp) ;
		return 1 ;	// Success
}


int serialDialog::Receive_Byte( uint8_t *c, int mS )
{
  int numBytes ;
  int i ;
	char buff[2] ;
  QByteArray qba ;
  QTime Time ;
  
	Time.start() ;
  while( Time.elapsed() < mS )
  {
  	numBytes = port->bytesAvailable() ;
		if ( numBytes)
		{
			i = port->read(buff, 1 ) ;
  		qba = QByteArray::fromRawData(buff, i ) ;
 		  *c = qba[0] ;
			return 0 ;
		
		}
	}
	return 0xFFFF ;
}	
	


/*******************************************************************************
* Function Name  : Receive_Packet
* Description    : Receive a packet from sender
* Input 1        : - data
* Input 2        : - length
* Input 3        : - timeout
* Output         : *length:
*                  0: end of transmission
*                  -1: abort by sender
*                  >0: packet length
* Return         : 0: normally return
*                  -1: timeout or packet error
*                  1: abort by user
*******************************************************************************/
int serialDialog::Receive_Packet (uint8_t *data, int *length, int timeout)
{
  int i, packet_size;
  uint8_t c;
  *length = 0;

  if (Receive_Byte(&c, timeout) != 0)
  {
    return -1;
  }
	char text[2] ;
	text[0] = c ;
	text[1] = 0 ;
			 
  QByteArray qba ;
	qba = QByteArray::fromRawData( text, 1 ) ;
	qba = qba.toHex() ;
	Monitor.append(qba);
	ui->monitorText->setText(Monitor) ;
	
  switch (c)
  {
    case SOH:
      packet_size = PACKET_SIZE;
    break;
    case STX:
      packet_size = PACKET_1K_SIZE;
      break;
    case EOT:
  		*length = 0 ;
    return 0;
    case CA:
      if ((Receive_Byte(&c, timeout) == 0) && (c == CA))
      {
        *length = -1;
        return 0;
      }
      else
      {
        return -1;
      }

    case ABORT1:
    case ABORT2:
      port->write( QByteArray::fromRawData ( (char *)&c, 1 ), 1 ) ;
    return 1;

    default:
      return -1;
  }
  *data = c;
  for (i = 1; i < (packet_size + PACKET_OVERHEAD); i ++)
  {
    if (Receive_Byte(data + i, PACKET_TIMEOUT) != 0)
    {
      return -1;
    }
  }
  if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff))
  {
    return -1;
  }
  *length = packet_size;
  return 0;
}


uint32_t FileTime ;

int serialDialog::receiveOneFile( QString fname )
{
  uint8_t *file_ptr; //, *buf_ptr;
  uint8_t c;
  int i, session_done, file_done, packets_received, errors, session_begin ;
  int packet_length ;
//	, session_done, file_done, packets_received, errors, session_begin ;
  int numBytes ;
  uint8_t dataBlock[BLOCK_AREA] ;
//	FRESULT fr ;
	uint32_t written ;
	int size ;
	int blockNumber ;
	char acknak ;
	int retryCount ;
  char buff[150] ;
	char file_name[50] ;
	QString simpleName = fname ;

	size = 0 ;
//  file_name[0] = 0 ;
//	rx_fifo_running = 1 ;

	fname = "\\MODELS\\" + fname ;
//  QMessageBox::critical(this, "eePe", tr("Receive File %1").arg(fname));
//	return 0 ;

  port->write( QByteArray::fromRawData ( ( char *)DoubleEsc, 2 ), 2 ) ;
  waitMs( 150 ) ;
  numBytes = port->bytesAvailable() ;
  if(numBytes > 148)
	{
    numBytes = 148 ;
	}
  buff[0] = '\0';
  i = port->read(buff, numBytes) ;	// Discard cr,lf,>
  if (i != -1)
	{
    buff[i] = '\0';
	}
  
//	QMessageBox::critical(this, "eePe", tr("Esc Esc sent"));

	port->write( QByteArray::fromRawData ( "S", 1 ), 1 ) ;
  
	if (Receive_Byte(&c, 200) != 0)
	{
  	QMessageBox::critical(this, "eePe", tr("Comms Failure(1)"));
	 	return 0 ;	
	}
	if ( c != 's' )
	{
  	QMessageBox::critical(this, "eePe", tr("Comms Failure(2)"));
		return 0 ;
	}

	// Next send a packet with the filename in it
  
	blockNumber = 0 ;
	memset( dataBlock, 0, BLOCK_AREA ) ;
	dataBlock[0] = SOH ;
	dataBlock[1] = blockNumber ;
	dataBlock[2] = ~blockNumber ;
	strcpy( (char *)&dataBlock[3], fname.toLatin1() ) ;
	checksumBlock( dataBlock, SET_CHECKSUM ) ;
	
	retryCount = 3 ;
	do
	{
		i = port->write( QByteArray::fromRawData ( ( char *)dataBlock, 128 + 5 ), 128 + 5 ) ;
//  	temp = tr("%1").arg(i) ;
//  	ui->sizeSent->setText( temp ) ;

  	waitMs( 40 ) ;

		acknak = waitForAckNak( 500 ) ;
		retryCount -= 1 ;

		if ( acknak != ACK )
		{
			NAKcount += 1 ;
//	    temp = tr("%1").arg(NAKcount) ;
//  	  ui->NAKcount->setText( temp ) ;
		}

	} while ( ( acknak != ACK ) && retryCount ) ;
	if ( acknak != ACK )
	{
    QMessageBox::critical(this, "eePe", tr("Comms Failure(3)"));
		ui->receiveButton->setDisabled( false ) ;
		return 0 ;	// Failed
	}

	Monitor.append("\nName Sent-");
	ui->monitorText->setText(Monitor) ;

	QString fn = ui->receiveDirectory->text() + "/Tempfile" ;
	
	QFile thisfile( fn ) ;

  for (session_done = 0, errors = 0, session_begin = 0; ;)
  {
    for (packets_received = 0, file_done = 0 ; ;)
    {
      switch (Receive_Packet( dataBlock, &packet_length, NAK_TIMEOUT))
      {
        case 0:
	Monitor.append(" R0");
	ui->monitorText->setText(Monitor) ;
 	waitMs( 3 ) ;
          errors = 0;
          switch (packet_length)
          {
              /* Abort by sender */
            case -1:
	Monitor.append(" -1");
	ui->monitorText->setText(Monitor) ;
							port->write( QByteArray::fromRawData ( STR_ACK, 1 ), 1 ) ;
							thisfile.close() ;
            return 1 ;
	Monitor.append(" +1");
	ui->monitorText->setText(Monitor) ;
              /* End of transmission */
            case 0:
	Monitor.append(" 0");
	ui->monitorText->setText(Monitor) ;
							thisfile.close() ;
							port->write( QByteArray::fromRawData ( STR_ACK, 1 ), 1 ) ;
              file_done = 1;
            break;
              /* Normal packet */
            default:
//	Monitor.append(" def");
//	ui->monitorText->setText(Monitor) ;
              if ((dataBlock[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff))//
              {
								port->write( QByteArray::fromRawData ( STR_NAK, 1 ), 1 ) ;
              }
              else
              {
                if (packets_received == 0)
                {/* Filename packet */
                  if (dataBlock[PACKET_HEADER] != 0)
                  {/* Filename packet has valid data */
										if ( checksumBlock( dataBlock, CHECK_CHECKSUM ) )
										{
											size = 0 ;
										  file_name[0] = 0 ;
											FileTime = 0 ;
											
                    	for (i = 0, file_ptr = dataBlock + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH);)
                    	{
                    	  file_name[i++] = *file_ptr++;
                    	}
                    	file_name[i++] = '\0';
//  QByteArray qba ;
//  qba = QByteArray::fromRawData( (char *)dataBlock, 128+5 ) ;
//	qba = qba.toHex() ;
//	Monitor.append(qba);
//	ui->monitorText->setText(Monitor) ;
//	qba = QByteArray::fromRawData( file_name, i ) ;
//	Monitor.append(qba);
//	ui->monitorText->setText(Monitor) ;

                    	for (i = 0, file_ptr += 1; (*file_ptr) && (i < FILE_SIZE_LENGTH);)
                    	{
												size *= 10 ;
												size += *file_ptr++ - '0' ;
                    	}
											uint32_t ftime = 0 ;
                    	for (i = 0, file_ptr += 4; i < 4 ; i += 1, file_ptr -= 1 )
											{
												ftime <<= 8 ;
												ftime |= *file_ptr ;
											}
											FileTime = ftime ;
											thisfile.remove() ;
											thisfile.open( QIODevice::WriteOnly ) ;
							//		  	// Check fr value here

											port->write( QByteArray::fromRawData ( STR_ACK, 1 ), 1 ) ;
//                    	Send_Byte(CRC16);
  			              packets_received ++;
										}
										else
										{
											port->write( QByteArray::fromRawData ( STR_NAK, 1 ), 1 ) ;
//                    	Send_Byte(CRC16);
										}
                  }
                  /* Filename packet is empty, end session */
                  else
                  {
											port->write( QByteArray::fromRawData ( STR_ACK, 1 ), 1 ) ;
                    file_done = 1;
                    session_done = 1;
                    break;
                  }
                }
                else
                { /* Data packet */
									if ( checksumBlock( dataBlock, CHECK_CHECKSUM ) )
									{
										if ( size > 128 )
										{
											i = 128 ;
											size -= 128 ;
										}
										else
										{
											i = size ;
											size = 0 ;
										}
										if ( i )
										{
//  QByteArray qba ;
//  qba = QByteArray::fromRawData( (char*)dataBlock, 128+5 ) ;
//	qba = qba.toHex() ;
//	Monitor.append(qba);
//	ui->monitorText->setText(Monitor) ;
// 	waitMs( 3 ) ;
                      written = thisfile.write( (char *)&dataBlock[3], i ) ;
//											fr = f_write( &Tfile, &packet_data[3], i, (UINT *)&written ) ;
											// should check fr here
										}
										port->write( QByteArray::fromRawData ( STR_ACK, 1 ), 1 ) ;
	                	packets_received ++;
									}
									else
									{
										port->write( QByteArray::fromRawData ( STR_NAK, 1 ), 1 ) ;
									}
                }
                session_begin = 1;
              }
//							(void)fr ;
          }
        break ;

        case 1:
//	Monitor.append(" R1");
//	ui->monitorText->setText(Monitor) ;
					port->write( QByteArray::fromRawData ( STR_CA_CA, 2 ), 2 ) ;
        return 0 ;
        
				default:
//	Monitor.append(" Rdef");
//	ui->monitorText->setText(Monitor) ;
          if (session_begin > 0)
          {
            errors ++;
          }
          if (errors > MAX_ERRORS)
          {
  Monitor.append(" CACA");
	ui->monitorText->setText(Monitor) ;
						port->write( QByteArray::fromRawData ( STR_CA_CA, 2 ), 2 ) ;
						thisfile.close() ;
            return 0 ;
          }
	Monitor.append(" X");
	ui->monitorText->setText(Monitor) ;
					port->write( QByteArray::fromRawData ( STR_CA, 1 ), 1 ) ;
//					port->write( QByteArray::fromRawData ( STR_CRC16, 1 ), 1 ) ;
        break ;
      }
      if (file_done != 0)
      {
        break ;
      }
    }
	fn = ui->receiveDirectory->text() + '/' + simpleName ;
		QFile oldfile( fn ) ;
		oldfile.remove() ;				/* Delete existing file */
  
	  Monitor.append(fn) ;
		ui->monitorText->setText(Monitor) ;

		thisfile.rename( fn ) ;
    
		if (session_done != 0)
    {
      break;
    }
  }

//	rx_fifo_running = 0 ;
  return 0 ;
}



