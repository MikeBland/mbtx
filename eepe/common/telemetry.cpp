
//#define TELEMETRY_LOGGING	1

#include <stdint.h>
#include "pers.h"
#include "telemetry.h"
#include "ui_telemetryDialog.h"
#ifdef SKY
#include "stamp-eepskye.h"
#else
#include "stamp-eepe.h"
#endif
#include "mainwindow.h"
#include <QtGui>
#include "qextserialenumerator.h"
#include "helpers.h"
#include <QtCore/QList>
#include <QString>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>

// Receive buffer state machine state defs
#define frskyDataIdle    0
#define frskyDataStart   1
#define frskyDataInFrame 2
#define frskyDataXOR     3

#define LINKPKT         0xfe
#define USRPKT          0xfd
#define A11PKT          0xfc
#define A12PKT          0xfb
#define A21PKT          0xfa
#define A22PKT          0xf9
#define ALRM_REQUEST    0xf8
#define RSSIRXPKT       0xf7
#define RSSITXPKT       0xf6
#define RSSI_REQUEST		0xf1

#define START_STOP      0x7e
#define BYTESTUFF       0x7d
#define STUFF_MASK      0x20

#define ALT_ID					0x10
#define ALT_DEC_ID			0x21
//DataID Meaning       Unit   Range   Note
//0x01   GPS altitude  m              Before”.”
//0x02   Temperature1  °C     -30-250
//0x03   RPM           BPS    0-60000
//0x04   Fuel Level    %      0, 25, 50, 75, 100
//0x05   Temperature2  °C     -30-250
//0x06   Volt          1/500v 0-4.2v
//0x07
//0x08
//0x09   GPS altitude  m              After “.”
//0x0A
//0x0B
//0x0C
//0x0D
//0x0E
//0x0F
//0x10   Altitude      m      0-9999
//0x11   GPS speed     Knots          Before “.”
//0x12   Longitude     dddmm.mmmm     Before “.”
//0x13   Latitude      ddmm.mmmm      Before “.”
//0x14   Course        degree 0-360   Before “.”
//0x15   Date/Month
//0x16   Year
//0x17   Hour /Minute
//0x18   Second
//0x19   GPS speed     Knots          After “.”
//0x1A   Longitude     dddmm.mmmm     After “.”
//0x1B   Latitude      ddmm.mmmm      After “.”
//0x1C   Course        degree 0-360   After “.”
//0x1D
//0x1E
//0x1F
//0x20
//0x21   Altitude      m              After "."
//0x22   E/W
//0x23   N/S
//0x24   Acc-x         1/256g -8g ~ +8g
//0x25   Acc-y         1/256g -8g ~ +8g
//0x26   Acc-z         1/256g -8g ~ +8g
//0x27
//0x28   Current       1A   0-100A
//// . . .
//0x3A   Voltage(amp sensor) 0.5v 0-48V Before “.”
//0x3B   Voltage(amp sensor)            After “.”

int TabHeight ;

#define X9D_EXTRA		84

telemetryDialog::telemetryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::telemetryDialog)
{
  QString temp ;
  ui->setupUi(this);
#ifndef TELEMETRY_LOGGING
  ui->startButtonLogging->hide() ;
#endif
	QList<QextPortInfo> ports = QextSerialEnumerator::getPorts() ;
	ui->TelPortCB->clear() ;
  foreach (QextPortInfo info, ports)
	{
		if ( info.portName.length() )
		{
	  	ui->TelPortCB->addItem(info.portName) ;
		}
	}
	port = NULL ;
	timer = NULL ;
	sending = 0 ;
	telemetry = 0 ;
	lcdScreen = 0 ;
	Frsky_user_state = 0 ;
	lcdSize = 0 ;
	screenDumpIndex = 1 ;
	buttonStatus = 0 ;
	lastButtonStatus = 0 ;
	switchStatusLow = 0 ;
	switchStatusHigh = 0 ;
	displayType = 0 ;
	screenDumpSize = 1024 ;
	ui->_9X_RB->setChecked(true) ;
	ui->startButton->setText("Start") ;
  ui->WSvalue->setText(tr("%1").arg(ui->WSdial->value())) ;
  QSettings settings("er9x-eePskye", "eePskye");
	temp = settings.value( "LcdDumpDir", "/home" ).toString() ;
	ui->saveDirectory->setText( temp ) ;
	ui->saveName->setText( "screenDump" ) ;
	
	ui->ButtonsLabel->setPixmap(QPixmap(":/images/9xcurs.png"));
	ui->LeftLabel->setPixmap(QPixmap(":/images/9xdl.png"));
	ui->RightLabel->setPixmap(QPixmap(":/images/9xdr.png"));
	ui->MenuExitLabel->setPixmap(QPixmap(":/images/9xmenu.png"));
	ui->TopLabel->setPixmap(QPixmap(":/images/9xdt.png"));
	ui->BottomLabel->setPixmap(QPixmap(":/images/9xdb.png"));
	ui->Lx9dButtonsLabel->setPixmap(QPixmap(":/images/x9l0.png"));
	ui->Rx9dButtonsLabel->setPixmap(QPixmap(":/images/x9r0.png"));
	ui->xTopLabel->setPixmap(QPixmap(":/images/x9t0.png"));
	ui->xBottomLabel->setPixmap(QPixmap(":/images/x9b0.png"));

	ui->SFslider->setValue(0) ;
  ui->SFwidget->hide() ;
	 
	backColour = settings.value( "LcdBacklight", 0x0000C0FF ).toInt() ;
	setBkColour() ;

  QImage image(424, 128, QImage::Format_RGB32);
  uchar b[212*8] = {0} ;
	uint32_t i ;
	for ( i = 0 ; i < 212*8 ; i += 1 )
	{
		b[i] = 4 ;
	}
  for(int y=0; y<SPLASH_HEIGHT; y++)
	{
  	for(int x=0; x<212; x++)
		{
			uint32_t pix ;
			int lx ;
			int ly ;
			lx = 2 * x ;
			ly = 2 * y ;
			pix = ((b[212*(y/8) + x]) & (1<<(y % 8))) ? 0 : backColour ;
      image.setPixel(lx,ly, pix );
      image.setPixel(lx+1,ly, pix );
      image.setPixel(lx,ly+1, pix );
      image.setPixel(lx+1,ly+1, pix );
		}
	}
  ui->ImageLabel->setPixmap(QPixmap::fromImage(image));

  lPolygon[0].setPoints(6, 68, 83, 28, 45, 51, 32, 83, 32, 105, 45, 68, 83);
  lPolygon[1].setPoints(6, 74, 90, 114, 51, 127, 80, 127, 106, 114, 130, 74, 90);
  lPolygon[2].setPoints(6, 68, 98, 28, 137, 51, 151, 83, 151, 105, 137, 68, 98);
  lPolygon[3].setPoints(6, 58, 90, 20, 51, 7, 80, 7, 106, 20, 130, 58, 90);


  lPolygon[4].setPoints(6, 20, 59, 27, 50, 45, 52, 56, 59, 50, 71, 26, 72);
  lPolygon[5].setPoints(6, 23, 107, 30, 99, 46, 100, 55, 106, 47, 117, 28, 117);
  lPolygon[6].setPoints(6, 24, 154, 32, 144, 46, 146, 57, 156, 46, 167, 29, 166);
  lPolygon[7].setPoints(6, 64, 60, 71, 50, 90, 50, 100, 60, 90, 73, 72, 73);
  lPolygon[8].setPoints(6, 63, 109, 73, 100, 88, 100, 98, 109, 88, 119, 72, 119);
  lPolygon[9].setPoints(6, 63, 155, 72, 146, 90, 146, 98, 155, 88, 166, 72, 166);


	ui->ButtonsLabel->installEventFilter(this) ;
	ui->MenuExitLabel->installEventFilter(this) ;
	ui->Lx9dButtonsLabel->installEventFilter(this) ;
	ui->Rx9dButtonsLabel->installEventFilter(this) ;

}

bool telemetryDialog::eventFilter(QObject *obj, QEvent *event)
{
//     if (obj == ui->tabWidget->widget(3) )
	if (obj == ui->ButtonsLabel )
	{
		if (event->type() == QEvent::MouseButtonPress)
 		{
      leftMousePressEvent((QMouseEvent *) event) ;
 		  return true;
 		}
 		else if (event->type() == QEvent::MouseButtonRelease)
 		{
      leftMouseReleaseEvent((QMouseEvent *) event) ;
 		  return true;
 		}
 		else
 		{
 		  return false;
 		}
  }
	else if (obj == ui->MenuExitLabel )
	{
		if (event->type() == QEvent::MouseButtonPress)
 		{
      rightMousePressEvent((QMouseEvent *) event) ;
 		  return true;
 		}
 		else if (event->type() == QEvent::MouseButtonRelease)
 		{
      rightMouseReleaseEvent((QMouseEvent *) event) ;
 		  return true;
 		}
 		else
 		{
 		  return false;
 		}
	}
	else if (obj == ui->Lx9dButtonsLabel )
	{
		if (event->type() == QEvent::MouseButtonPress)
 		{
      leftXMousePressEvent((QMouseEvent *) event) ;
 		  return true;
 		}
 		else if (event->type() == QEvent::MouseButtonRelease)
 		{
      leftXMouseReleaseEvent((QMouseEvent *) event) ;
 		  return true;
 		}
 		else
 		{
 		  return false;
 		}
	}
	else if (obj == ui->Rx9dButtonsLabel )
	{
		if (event->type() == QEvent::MouseButtonPress)
 		{
      rightXMousePressEvent((QMouseEvent *) event) ;
 		  return true;
 		}
 		else if (event->type() == QEvent::MouseButtonRelease)
 		{
      rightXMouseReleaseEvent((QMouseEvent *) event) ;
 		  return true;
 		}
 		else
 		{
 		  return false;
 		}
	}
	else
	{
		// pass the event on to the parent class
		return telemetryDialog::eventFilter(obj, event);
	}
}

void telemetryDialog::leftMouseReleaseEvent(QMouseEvent * event)
{
	(void) event ;
	ui->ButtonsLabel->setPixmap(QPixmap(":/images/9xcurs.png"));
	buttonStatus = 0 ;
}

void telemetryDialog::rightMouseReleaseEvent(QMouseEvent * event)
{
	(void) event ;
	ui->MenuExitLabel->setPixmap(QPixmap(":/images/9xmenu.png"));
	buttonStatus = 0 ;
}

void telemetryDialog::leftXMouseReleaseEvent(QMouseEvent * event)
{
	(void) event ;
	ui->Lx9dButtonsLabel->setPixmap(QPixmap(":/images/x9l0.png"));
	buttonStatus = 0 ;
}

void telemetryDialog::rightXMouseReleaseEvent(QMouseEvent * event)
{
	(void) event ;
	ui->Rx9dButtonsLabel->setPixmap(QPixmap(":/images/x9r0.png"));
	buttonStatus = 0 ;
}

void telemetryDialog::rightXMousePressEvent(QMouseEvent * event)
{
	if ( ui->tabWidget->currentIndex() == 3 )
	{
		int x = event->x();
		int y = event->y();
//		ui->spinBox_10->setValue(x) ;
//		ui->spinBox_11->setValue(y) ;
	
	// Bit 1 is Menu, 2 is Exit
		int i ;
		for ( i = 7 ; i < 10 ; i += 1 )
		{
      if (lPolygon[i].containsPoint(QPoint(x, y), Qt::OddEvenFill))
			{
				switch ( i )
				{
					case 7 :
						ui->Rx9dButtonsLabel->setPixmap(QPixmap(":/images/x9r1.png"));
						buttonStatus = 0x02 ;
					break ;
					case 8 :
						ui->Rx9dButtonsLabel->setPixmap(QPixmap(":/images/x9r2.png"));
						buttonStatus = 0x20 ;
					break ;
					case 9 :
						ui->Rx9dButtonsLabel->setPixmap(QPixmap(":/images/x9r3.png"));
						buttonStatus = 0x04 ;
					break ;
				}
			}
		}
  }
}

void telemetryDialog::leftXMousePressEvent(QMouseEvent * event)
{
	if ( ui->tabWidget->currentIndex() == 3 )
	{
		int x = event->x();
		int y = event->y();
		ui->spinBox_10->setValue(x) ;
		ui->spinBox_11->setValue(y) ;

		if ( ( ( x >= 90 ) && ( x <= 118 ) ) && ( ( y >= 177 ) && ( y <= 197 ) ) )
		{
			// snapshot pressed
       ui->Lx9dButtonsLabel->setPixmap(QPixmap(":/images/x9l4.png"));
			makeScreenshot() ;
		}
		else
		{
			int i ;
			for ( i = 4 ; i < 7 ; i += 1 )
			{
      	if (lPolygon[i].containsPoint(QPoint(x, y), Qt::OddEvenFill))
				{
	// Bits 3-6 are down, up, right and left
					switch ( i )
					{
						case 4 :
							ui->Lx9dButtonsLabel->setPixmap(QPixmap(":/images/x9l1.png"));
							buttonStatus = 0x10 ;
						break ;
						case 5 :
							ui->Lx9dButtonsLabel->setPixmap(QPixmap(":/images/x9l2.png"));
							buttonStatus = 0x40 ;
						break ;
						case 6 :
							ui->Lx9dButtonsLabel->setPixmap(QPixmap(":/images/x9l3.png"));
							buttonStatus = 0x08 ;
						break ;
					}
				}
			}
		}
	}
}

void telemetryDialog::rightMousePressEvent(QMouseEvent * event)
{
	if ( ui->tabWidget->currentIndex() == 3 )
	{
		int x = event->x();
		int y = event->y();
//		ui->spinBox_10->setValue(x) ;
//		ui->spinBox_11->setValue(y) ;
	
	// Bit 1 is Menu, 2 is Exit
		if ( ( ( x >= 25 ) && ( x <= 71 ) ) && ( ( y >= 60 ) && ( y <= 81 ) ) )
		{
			ui->MenuExitLabel->setPixmap(QPixmap(":/images/9xmenumenu.png"));
			buttonStatus = 0x02 ;
		}
		else if ( ( ( x >= 25 ) && ( x <= 71 ) ) && ( ( y >= 117 ) && ( y <= 139 ) ) )
		{
			ui->MenuExitLabel->setPixmap(QPixmap(":/images/9xmenuexit.png"));
			buttonStatus = 0x04 ;
		}
	}
}

void telemetryDialog::leftMousePressEvent(QMouseEvent * event)
{
	if ( ui->tabWidget->currentIndex() == 3 )
	{
		int x = event->x();
		int y = event->y();
		ui->spinBox_10->setValue(x) ;
		ui->spinBox_11->setValue(y) ;

		if ( ( ( x >= 5 ) && ( x <= 39 ) ) && ( ( y >= 148 ) && ( y <= 182 ) ) )
		{
			// snapshot pressed
       ui->ButtonsLabel->setPixmap(QPixmap(":/images/9xcursphoto.png"));
			makeScreenshot() ;
		}
		else
		{
			int i ;
			for ( i = 0 ; i < 4 ; i += 1 )
			{
      	if (lPolygon[i].containsPoint(QPoint(x, y), Qt::OddEvenFill))
				{
	// Bits 3-6 are down, up, right and left
					switch ( i )
					{
						case 0 :
							ui->ButtonsLabel->setPixmap(QPixmap(":/images/9xcursup.png"));
							buttonStatus = 0x10 ;
						break ;
						case 1 :
							ui->ButtonsLabel->setPixmap(QPixmap(":/images/9xcursmin.png"));
							buttonStatus = 0x20 ;
						break ;
						case 2 :
							ui->ButtonsLabel->setPixmap(QPixmap(":/images/9xcursdown.png"));
							buttonStatus = 0x08 ;
						break ;
						case 3 :
							ui->ButtonsLabel->setPixmap(QPixmap(":/images/9xcursplus.png"));
							buttonStatus = 0x40 ;
						break ;
					}
				}
			}
		}
	}
}

void telemetryDialog::setBkColour()
{
  QImage image(2, 1, QImage::Format_RGB32) ;
  image.setPixel( 0, 0, backColour ) ;
  image.setPixel( 1, 0, backColour ) ;
  ui->ColourLabel->setPixmap(QPixmap::fromImage(image));
}

telemetryDialog::~telemetryDialog()
{
	if ( timer )
	{
    timer->stop() ;
    delete timer ;
		timer = NULL ;
	}
	if ( port )
	{
		if (port->isOpen())
		{
    	port->close();
		}
    delete port ;
		port = NULL ;
	}
  delete ui;
}

void telemetryDialog::on_ReadAlarmsButton_clicked()
{
	int size ;
	if ( port )
	{
		size = FRSKY_setTxPacket( RSSI_REQUEST, 0, 0, 0 ) ;
		port->write( QByteArray::fromRawData ( (char *)frskyTxBuffer, size ), size ) ;
		size = FRSKY_setTxPacket( ALRM_REQUEST, 0, 0, 0 ) ;
		port->write( QByteArray::fromRawData ( (char *)frskyTxBuffer, size ), size ) ;
	}
}

void telemetryDialog::on_SetAlarmsButton_clicked()
{
	if ( port )
	{
		sendState = 1 ;
	}		
}

void telemetryDialog::on_exitButton_clicked()
{
	done(0) ;	
}

void telemetryDialog::on_exitButtonModule_clicked()
{
	done(0) ;	
}
		
void telemetryDialog::clearCurrentConnection()
{
	if ( sending )
	{
		sending = 0 ;
		ui->startButton->setText("Start") ;
	}

	if ( telemetry )
	{
		telemetry = 0 ;
		ui->startButtonModule->setText("Start") ;
	}

	if ( lcdScreen )
	{
		lcdScreen = 0 ;
		ui->startButtonScreen->setText("Start") ;
	}
	 
	if ( timer )
	{
 	  timer->stop() ;
 	  delete timer ;
		timer = NULL ;
	}
	
	if ( port )
	{
		if (port->isOpen())
		{
 	  	port->close();
		}
 	  delete port ;
		port = NULL ;
	}
}

void telemetryDialog::on_startButtonModule_clicked()
{
	QString portname ;

	if ( sending )
	{
		clearCurrentConnection() ;
	}
	if ( lcdScreen )
	{
		clearCurrentConnection() ;
	}
	if ( telemetry )
	{
		clearCurrentConnection() ;
	}
	else
	{
		portname = ui->TelPortCB->currentText() ;
#ifdef Q_OS_UNIX
  	port = new QextSerialPort(portname, QextSerialPort::Polling) ;
#else
		port = new QextSerialPort(portname, QextSerialPort::Polling) ;
#endif /*Q_OS_UNIX*/
		port->setBaudRate(BAUD9600) ;
  	port->setFlowControl(FLOW_OFF) ;
		port->setParity(PAR_NONE) ;
  	port->setDataBits(DATA_8) ;
		port->setStopBits(STOP_1) ;
  	//set timeouts to 500 ms
  	port->setTimeout(-1) ;
  	if (!port->open(QIODevice::ReadWrite | QIODevice::Unbuffered) )
  	{
  		QMessageBox::critical(this, "eePe", tr("Com Port Unavailable"));
			if (port->isOpen())
			{
  		  port->close();
			}
  		delete port ;
			port = NULL ;
			return ;	// Failed
		}
		telemetry = 1 ;
		dataState = frskyDataIdle ;
		numPktBytes = 0 ;

		setupTimer(50) ;
		ui->startButtonModule->setText("Stop") ;
	}
}


int telemetryDialog::on_startButtonUart_clicked()
{
	return 0 ;
	QString portname ;
	if ( telemetry )
	{
		clearCurrentConnection() ;
	}
	if ( sending )
	{
		clearCurrentConnection() ;
	}
	if ( lcdScreen )
	{
		clearCurrentConnection() ;
	}

  portname = ui->TelPortCB->currentText() ;
	port = new QextSerialPort(portname, QextSerialPort::EventDriven) ;
  port->setBaudRate(BAUD57600) ;
  port->setFlowControl(FLOW_OFF) ;
	port->setParity(PAR_NONE) ;
  port->setDataBits(DATA_8) ;
	port->setStopBits(STOP_1) ;
  port->setTimeout(50) ;  //set timeouts to 50 ms


 	if (!port->open(QIODevice::ReadWrite | QIODevice::Unbuffered) )
 	{
 		QMessageBox::critical(this, "Sport Config", tr("Com Port Unavailable"));
		if (port->isOpen())
		{
 	  	port->close();
		}
 	  delete port ;
		port = NULL ;
		return 0 ;	// Failed
	}
  connect(port,SIGNAL(readyRead()),this,SLOT(receive()));
	ui->startButtonUart->setText("Stop") ;
	return 1 ;
	
}



void telemetryDialog::on_saveButtonScreen_clicked()
{
	makeScreenshot() ;
}

void telemetryDialog::on_startButtonScreen_clicked()
{
	QString portname ;
	lcdState = 0 ;

	if ( telemetry )
	{
		clearCurrentConnection() ;
	}
	if ( sending )
	{
		clearCurrentConnection() ;
	}
	if ( lcdScreen )
	{
		clearCurrentConnection() ;
	}
	else
	{
	  portname = ui->TelPortCB->currentText() ;
#ifdef Q_OS_UNIX
  	port = new QextSerialPort(portname, QextSerialPort::Polling) ;
#else
	  port = new QextSerialPort(portname, QextSerialPort::Polling) ;
#endif /*Q_OS_UNIX*/
	  port->setBaudRate(BAUD115200) ;
  	port->setFlowControl(FLOW_OFF) ;
	  port->setParity(PAR_NONE) ;
  	port->setDataBits(DATA_8) ;
	  port->setStopBits(STOP_1) ;
    //set timeouts to 500 ms
  	port->setTimeout(50) ;
  	if (!port->open(QIODevice::ReadWrite | QIODevice::Unbuffered) )
  	{
  	  QMessageBox::critical(this, "eePe", tr("Com Port Unavailable"));
			if (port->isOpen())
			{
  	  	port->close();
			}
  	  delete port ;
			port = NULL ;
			return ;	// Failed
		}
		setupTimer(20) ;
		lcdScreen = 1 ;
		ui->startButtonScreen->setText("Stop") ;
	}
}


void telemetryDialog::on_startButton_clicked()
{
	QString portname ;

	if ( telemetry )
	{
		clearCurrentConnection() ;
	}
	if ( lcdScreen )
	{
		clearCurrentConnection() ;
	}
	if ( sending )
	{
		clearCurrentConnection() ;
	}
	else
	{
	  portname = ui->TelPortCB->currentText() ;
#ifdef Q_OS_UNIX
  	port = new QextSerialPort(portname, QextSerialPort::Polling) ;
#else
	  port = new QextSerialPort(portname, QextSerialPort::Polling) ;
#endif /*Q_OS_UNIX*/
	  port->setBaudRate(BAUD9600) ;
  	port->setFlowControl(FLOW_OFF) ;
	  port->setParity(PAR_NONE) ;
  	port->setDataBits(DATA_8) ;
	  port->setStopBits(STOP_1) ;
    //set timeouts to 500 ms
  	port->setTimeout(1000) ;
  	if (!port->open(QIODevice::ReadWrite | QIODevice::Unbuffered) )
  	{
  	  QMessageBox::critical(this, "eePe", tr("Com Port Unavailable"));
			if (port->isOpen())
			{
  	  	port->close();
			}
  	  delete port ;
			port = NULL ;
			return ;	// Failed
		}
		sending = 1 ;
		setupTimer(500) ;
		ui->startButton->setText("Stop") ;
	}
}

#ifdef TELEMETRY_LOGGING
static uint8_t Logging = 0 ;
static unsigned char LogBuffer[256] ;
static uint32_t LogCount = 0 ;
QFile LogFile("C:/data/Telemetry");

void telemetryDialog::on_startButtonLogging_clicked()
{
	if ( telemetry )
	{
		if ( Logging )
		{
			Logging = 0 ;
			ui->startButtonLogging->setText("Log") ;
      LogFile.write((char*)LogBuffer,LogCount) ;
      LogFile.close() ;
		}
		else
		{
			ui->startButtonLogging->setText("Stop Logging") ;
			Logging = 1 ;
			LogCount = 0 ;
      LogFile.open(QFile::WriteOnly) ;
		}
	}
}
#endif

void telemetryDialog::on_WSdial_valueChanged( int value )
{
	ui->WSvalue->setText(tr("%1").arg(value)) ;
}

void telemetryDialog::setupTimer(int time)
{
  timer = new QTimer(this);
  connect(timer,SIGNAL(timeout()),this,SLOT(timerEvent())) ;
  timer->start(time) ;
}

void telemetryDialog::processFrskyPacket(quint8 *packet)
{
  // What type of packet?
  switch (packet[0])
  {
    case A11PKT:
			ui->A11AlarmSB->setValue( packet[1] ) ;
			ui->A11GtLtCB->setCurrentIndex( packet[2] & 0x01 ) ;
			ui->A11LevelCB->setCurrentIndex( packet[3] & 0x03 ) ;
    break ;
    
		case A12PKT:
			ui->A12AlarmSB->setValue( packet[1] ) ;
			ui->A12GtLtCB->setCurrentIndex( packet[2] & 0x01 ) ;
			ui->A12LevelCB->setCurrentIndex( packet[3] & 0x03 ) ;
    break ;
    
		case A21PKT:
			ui->A21AlarmSB->setValue( packet[1] ) ;
			ui->A21GtLtCB->setCurrentIndex( packet[2] & 0x01 ) ;
			ui->A21LevelCB->setCurrentIndex( packet[3] & 0x03 ) ;
    break ;
    
		case A22PKT:
			ui->A22AlarmSB->setValue( packet[1] ) ;
			ui->A22GtLtCB->setCurrentIndex( packet[2] & 0x01 ) ;
			ui->A22LevelCB->setCurrentIndex( packet[3] & 0x03 ) ;
		break ;

    case LINKPKT: // A1/A2/RSSI values
			ui->A1SB->setValue( packet[1] ) ;
			ui->A2SB->setValue( packet[2] ) ;
			ui->RSSI_SB->setValue( packet[3] ) ;
			ui->TSSI_SB->setValue( packet[4] / 2 ) ;
      break;

		case RSSIRXPKT :
			ui->RssiAlarmSB->setValue( packet[1] ) ;
			ui->RssiLevelCB->setCurrentIndex( packet[3] & 0x03 ) ;
		break ;

		case RSSITXPKT :
			ui->TssiAlarmSB->setValue( packet[1] ) ;
			ui->TssiLevelCB->setCurrentIndex( packet[3] & 0x03 ) ;
		break ;

    case USRPKT: // User Data packet
    {
			quint8 i, j ;
			i = ( packet[1] & 0x07) + 3 ;  // User bytes end
			j = 3 ;              // Index to user bytes
			while ( j < i )
			{
				frsky_proc_user_byte( packet[j] ) ;
				j += 1 ;
			}
    }	
    break;
  }
}

void telemetryDialog::frsky_proc_user_byte( quint8 byte )
{
//	if (g_model.FrSkyUsrProto == 0)  // FrSky Hub
//	{
	
  	if ( Frsky_user_state == 0 )
		{ // Waiting for 0x5E
			if ( byte == 0x5E )
			{
				Frsky_user_state = 1 ;			
				if ( Frsky_user_ready )
				{
					Frsky_user_ready = 0 ;
					store_indexed_hub_data( Frsky_user_id, ( Frsky_user_hibyte << 8 ) | Frsky_user_lobyte ) ;
				}
			}		
		}
		else
		{ // In a packet
			if ( byte == 0x5E )
			{ // 
				Frsky_user_state = 1 ;			
			}
			else
			{
				if ( byte == 0x5D )
				{
					Frsky_user_stuff = 1 ;  // Byte stuffing active
				}
				else
				{
					if ( Frsky_user_stuff )
					{
						Frsky_user_stuff = 0 ;
						byte ^= 0x60 ;  // Unstuff
					}
  	      if ( Frsky_user_state == 1 )
					{
					  Frsky_user_id	= byte ;
						Frsky_user_state = 2 ;
					}
  	      else if ( Frsky_user_state == 2 )
					{
					  Frsky_user_lobyte	= byte ;
						Frsky_user_state = 3 ;
					}
					else
					{
						Frsky_user_hibyte = byte ;
						Frsky_user_ready = 1 ;
						Frsky_user_state = 0 ;
					}
				}
			}		 
		}
//	}
//	else // if (g_model.FrSkyUsrProto == 1)  // WS How High
//	{
//    if ( frskyUsrStreaming < (FRSKY_USR_TIMEOUT10ms - 10))  // At least 100mS passed since last data received
//		{
//			Frsky_user_lobyte = byte ;
//		}
//		else
//		{
//			int16 value ;
//			value = ( byte << 8 ) + Frsky_user_lobyte ;
//			store_hub_data( FR_ALT_BARO, value ) ;	 // Store altitude info
//#if defined(VARIO)
//			evalVario( value, 0 ) ;
//#endif

//		}				
//	}
}

void telemetryDialog::store_indexed_hub_data( quint8 index, quint16 value )
{
	static quint16 alt_integer ;
	static quint16 alt_decimal = 0 ;
	if ( index == ALT_ID )
	{
		alt_integer = value ;
		
		if ( alt_decimal == 0 )
		{
			ui->AltDisp->setText(tr("%1").arg( alt_integer )) ;
		}
	}
	if ( index == ALT_DEC_ID )
	{
		alt_decimal = 1 ;
		ui->AltDisp->setText(tr("%1").arg( alt_integer )) ;
		ui->AltDdisp->setText(tr("%1").arg( value )) ;
	}
}

quint32 telemetryDialog::FRSKY_setTxPacket( quint8 type, quint8 value, quint8 p1, quint8 p2 )
{
	quint8 i = 0;
  frskyTxBuffer[i++] = START_STOP;        // Start of packet
  frskyTxBuffer[i++] = type ;
  frskyPushValue(i, value) ;
  {
    quint8 *ptr ;
    ptr = &frskyTxBuffer[i] ;
    *ptr++ = p1 ;
    *ptr++ = p2 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = 0x00 ;
    *ptr++ = START_STOP ;        // End of packet
	}
	return i + 8 ;
}

void telemetryDialog::frskyPushValue(quint8 &i, quint8 value)
{
	quint8 j ;
	j = 0 ;
  // byte stuff the only byte than might need it
  if (value == START_STOP) {
    j = 1 ;
    value = 0x5e;
  }
  else if (value == BYTESTUFF) {
    j = 1 ;
    value = 0x5d;
  }
	if ( j )
	{
		frskyTxBuffer[i++] = BYTESTUFF;
	}
  frskyTxBuffer[i++] = value;
}

void telemetryDialog::processTelByte( unsigned char data )
{
#ifdef TELEMETRY_LOGGING
	// Log data here
	if ( Logging )
	{
		LogBuffer[LogCount++] = data ;
		if ( LogCount >= 256 )
		{
       LogFile.write((char*)LogBuffer,256) ;
			 LogCount = 0 ;
		}
	}
#endif
	
  quint8 numbytes = numPktBytes ;
  switch (dataState) 
  {
    case frskyDataStart:
      if (data == START_STOP)
			{
				break ; // Remain in userDataStart if possible 0x7e,0x7e doublet found.
			}
      dataState = frskyDataInFrame;
      if (numbytes < 19)
	      frskyRxBuffer[numbytes++] = data ;
      break;

    case frskyDataInFrame:
      if (data == BYTESTUFF)
      { 
        dataState = frskyDataXOR; // XOR next byte
        break; 
      }
      if (data == START_STOP) // end of frame detected
      {
        processFrskyPacket(frskyRxBuffer); // FrskyRxBufferReady = 1;
 	      dataState = frskyDataIdle;
        break;
      }
      if (numbytes < 19)
	      frskyRxBuffer[numbytes++] = data;
      break;

    case frskyDataXOR:
      dataState = frskyDataInFrame;
      if (numbytes < 19)
        frskyRxBuffer[numbytes++] = data ^ STUFF_MASK;
      break;

    case frskyDataIdle:
      if (data == START_STOP)
      {
        numbytes = 0;
        dataState = frskyDataStart;
      }
      break;

  } // switch
  numPktBytes = numbytes ;
}

//uint16_t RxOK ;
uint32_t TotalBytes ;
uint16_t RxZero ;
uint16_t RxAA ;
uint16_t Rx55 ;

void telemetryDialog::timerEvent()
{
	if ( sending )
	{
  	unsigned char data[2] ;
		data[0] = ui->WSdial->value() ;
		data[1] = ui->WSdial->value() >> 8 ;
		if ( port )
		{
  		port->write( QByteArray::fromRawData ( (char *)data, 2 ), 2 ) ;
		}
	}
//	if ( lcdScreen )
//	{
//		QByteArray data ;
//		int count ;
//		int index ;
//		uint32_t i ;
//		index = 0 ;
//		if ( port )
//		{
//      data = port->read( 400 ) ;
//      count = data.size() ;
//			TotalBytes += count ;
//			ui->spinBox_4->setValue( TotalBytes ) ;

//			if ( count == 0 )
//			{
//				RxZero += 1 ;
//				ui->spinBox_5->setValue( RxZero ) ;
//				lcdState = 1 ;
//				lcdCounter = 0 ;
//			}
//			if ( lcdState == 0 )
//			{
//				if ( count == 0 )
//				{
//					lcdState = 1 ;
//					lcdCounter = 0 ;
//				}
//			}
//			else if ( lcdState == 1 )
//			{
//				if ( count )
//				{
//					uint8_t byte ;
//					byte = data.data()[0] ;
//					if ( byte == 0xAA )
//					{
////						RxAA += 1 ;
////						ui->spinBox_6->setValue( RxAA ) ;
//						lcdState = 2 ;
//						count -= 1 ;
//						index = 1 ;
//						while ( count )
//						{
//							lcdImage[lcdCounter] = data.data()[index++] ;
//							count -= 1 ;
//							lcdCounter += 1 ;
//						}
//					}
//				}
//			}
//			else if ( lcdState == 2 )
//			{
//				if ( count )
//				{
//					index = 0 ;
//					while ( count )
//					{
//						lcdImage[lcdCounter] = data.data()[index++] ;
//						count -= 1 ;
//						if ( ++lcdCounter >= 1024 )
//						{
//							lcdState = 3 ;
//							break ;
//						}
//					}
//				}
//			}
//			if ( lcdState == 3 )
//			{
////				RxOK += 1 ;
////				ui->spinBox_3->setValue( RxOK ) ;
//				if ( count )
//				{
//					uint8_t byte ;
//					byte = data.data()[index] ;
////					ui->spinBox_6->setValue( byte ) ;
//					if ( byte == 0x55 )
//					{
//						// got a complete screen image
//					  QImage image(256, 128, QImage::Format_RGB32);
//						lcdState = 1 ;
//						lcdCounter = 0 ;

//  					for(int y=0; y<SPLASH_HEIGHT; y++)
//						{
//  					  for(int x=0; x<SPLASH_WIDTH; x++)
//							{
//								uint32_t pix ;
//								int lx ;
//								int ly ;
//								lx = 2 * x ;
//								ly = 2 * y ;
//								pix = ((lcdImage[SPLASH_WIDTH*(y/8) + x]) & (1<<(y % 8))) ? 0 : backColour ;
//      					image.setPixel(2*x,2*y, pix );
//      					image.setPixel(2*x+1,2*y, pix );
//      					image.setPixel(2*x,2*y+1, pix );
//      					image.setPixel(2*x+1,2*y+1, pix );
//							}
//						}
//  					ui->ImageLabel->setPixmap(QPixmap::fromImage(image));
//					}
//				}
//			}
//		}
//	}
	
	
	if ( lcdScreen )
	{
		QByteArray data ;
		int count ;
		int index ;
		uint32_t i ;
		index = 0 ;
		if ( port )
		{
	  	
//			unsigned char sendData[8] ;
//			switchStatusLow = 0 ;
//			if ( ui->SFslider->value() )
//			{
//				switchStatusLow |= 0x20 ;
//			}
//			if ( buttonStatus )
//			{
//				sendData[0] = buttonStatus | 0x80 ;
//			}
//			else
//			{
//				sendData[0] = lastButtonStatus | 0x80  ;
//			}
//			sendData[1] = switchStatusLow  ;
//			sendData[2] = switchStatusHigh  ;
//			lastButtonStatus = buttonStatus ;
//  		port->write( QByteArray::fromRawData ( (char *)sendData, 3 ), 3 ) ;
  		
			unsigned char sendData[1] ;
			if ( buttonStatus )
			{
				sendData[0] = buttonStatus ;
			}
			else
			{
				sendData[0] = lastButtonStatus  ;
			}
			lastButtonStatus = buttonStatus ;
			port->write( QByteArray::fromRawData ( (char *)sendData, 1 ), 1 ) ;
			ui->spinBox_10->setValue(buttonStatus) ;
      data = port->read( 400 ) ;
      count = data.size() ;
			TotalBytes += count ;
			ui->spinBox_4->setValue( TotalBytes ) ;
			ui->spinBox_3->setValue(screenDumpSize) ;

			if ( count == 0 )
			{
				RxZero += 1 ;
				ui->spinBox_5->setValue( RxZero ) ;
				if ( lcdCounter == 0 )
				{
					lcdState = 1 ;
				}
//				lcdCounter = 0 ;
			}
			if ( lcdState == 0 )
			{
				if ( count == 0 )
				{
					lcdState = 1 ;
					lcdCounter = 0 ;
				}
			}

			for ( i = 0 ; i < 400 && count ; i += 1 )
//			while ( count )
			{
				if ( lcdState == 1 )
				{
					uint8_t byte ;
					byte = data.data()[index++] ;
					if ( byte == 0xAA )
					{
						RxAA += 1 ;
						ui->spinBox_6->setValue( RxAA ) ;
						lcdState = 2 ;
						count -= 1 ;
						while ( count )
						{
							lcdImage[lcdCounter] = data.data()[index++] ;
							count -= 1 ;
							lcdCounter += 1 ;
						}
					}
					else
					{
						count = 0 ;
						lcdState = 0 ;
					}
				}
				else if ( lcdState == 2 )
				{
					if ( count )
					{
						index = 0 ;
						while ( count )
						{
							lcdImage[lcdCounter] = data.data()[index++] ;
							count -= 1 ;
							if ( ++lcdCounter >= screenDumpSize )
							{
								lcdState = 3 ;
								break ;
							}
						}
					}
				}
				if ( lcdState == 3 )
				{
	//				RxOK += 1 ;
	//				ui->spinBox_3->setValue( RxOK ) ;
					if ( count )
					{
						uint8_t byte ;
						byte = data.data()[index++] ;
						count -= 1 ;

						Rx55 += 1 ;
						ui->spinBox_7->setValue( byte ) ;
						if ( byte == 0x55 )
						{
							// got a complete screen image
							if ( displayType )
							{
						  	QImage image(256+X9D_EXTRA+X9D_EXTRA, 128, QImage::Format_RGB32);
								lcdState = 1 ;

  							for(int y=0; y<64 ; y++)
								{
  							  for(int x=0; x<128+X9D_EXTRA; x++)
									{
										uint32_t pix ;
										int lx ;
										int ly ;
										lx = 2 * x ;
										ly = 2 * y ;
										pix = ((lcdImage[(128+X9D_EXTRA)*(y/8) + x]) & (1<<(y % 8))) ? 0 : backColour ;
      							image.setPixel(lx,ly, pix );
      							image.setPixel(lx+1,ly, pix );
      							image.setPixel(lx,ly+1, pix );
      							image.setPixel(lx+1,ly+1, pix );
									}
								}
                ui->ImageLabel->setPixmap(QPixmap::fromImage(image));
              }
							else
							{
						  	QImage image(256, 128, QImage::Format_RGB32);
								lcdState = 1 ;

  							for(int y=0; y<SPLASH_HEIGHT; y++)
								{
  							  for(int x=0; x<SPLASH_WIDTH; x++)
									{
										uint32_t pix ;
										int lx ;
										int ly ;
										lx = 2 * x ;
										ly = 2 * y ;
										pix = ((lcdImage[SPLASH_WIDTH*(y/8) + x]) & (1<<(y % 8))) ? 0 : backColour ;
      							image.setPixel(lx,ly, pix );
      							image.setPixel(lx+1,ly, pix );
      							image.setPixel(lx,ly+1, pix );
      							image.setPixel(lx+1,ly+1, pix );
									}
								}
                ui->ImageLabel->setPixmap(QPixmap::fromImage(image));
              }
						}
						else
						{
							lcdState = 0 ;
							count = 0 ;
						}
						lcdCounter = 0 ;
					}
				}
				if ( count == 0 )
				{
					break ;
				}
			}
		}
	}
	

	if ( telemetry )
	{
		QByteArray data ;
		int count ;
		int i ;
		int size ;
		if ( port )
		{
      data = port->read( 60 ) ;
      count = data.size() ;
			for ( i = 0 ; i < count ; i += 1 )
			{
				processTelByte( data.data()[i] ) ;
			}
			switch ( sendState )
			{
				case 1 :
					size = FRSKY_setTxPacket( RSSIRXPKT, ui->RssiAlarmSB->value(), 0, ui->RssiLevelCB->currentIndex() ) ;
					port->write( QByteArray::fromRawData ( (char *)frskyTxBuffer, size ), size ) ;
					sendState = 2 ;
				break ;
				case 2 :
					size = FRSKY_setTxPacket( RSSITXPKT, ui->TssiAlarmSB->value(), 0, ui->TssiLevelCB->currentIndex() ) ;
					port->write( QByteArray::fromRawData ( (char *)frskyTxBuffer, size ), size ) ;
					sendState = 3 ;
				break ;
				case 3 :
					size = FRSKY_setTxPacket( A11PKT, ui->A11AlarmSB->value(), ui->A11GtLtCB->currentIndex(), ui->A11LevelCB->currentIndex() ) ;
					port->write( QByteArray::fromRawData ( (char *)frskyTxBuffer, size ), size ) ;
					sendState = 4 ;
				break ;
				case 4 :
					size = FRSKY_setTxPacket( A12PKT, ui->A12AlarmSB->value(), ui->A12GtLtCB->currentIndex(), ui->A12LevelCB->currentIndex() ) ;
					port->write( QByteArray::fromRawData ( (char *)frskyTxBuffer, size ), size ) ;
					sendState = 5 ;
				break ;
				case 5 :
					size = FRSKY_setTxPacket( A21PKT, ui->A21AlarmSB->value(), ui->A21GtLtCB->currentIndex(), ui->A21LevelCB->currentIndex() ) ;
					port->write( QByteArray::fromRawData ( (char *)frskyTxBuffer, size ), size ) ;
					sendState = 6 ;
				break ;
				case 6 :
					size = FRSKY_setTxPacket( A22PKT, ui->A22AlarmSB->value(), ui->A22GtLtCB->currentIndex(), ui->A22LevelCB->currentIndex() ) ;
					port->write( QByteArray::fromRawData ( (char *)frskyTxBuffer, size ), size ) ;
					sendState = 0 ;
				break ;
			}
		}
		else
		{
			sendState = 0 ;
		}
	}
}


void telemetryDialog::setGraphics( bool checked )
{
	if ( ( lcdSize ) || ( checked ) )
	{
		lcdSize = 0 ;
		ui->ImageLabel->setGeometry( 170, 98, 258, 130) ;
		ui->sizeButtonScreen->setText("Larger") ;
	}
	if ( lcdSize )
	{
		ui->ImageLabel->setScaledContents( true ) ;
		ui->ButtonsLabel->hide() ;
		ui->LeftLabel->hide() ;
		ui->RightLabel->hide() ;
		ui->MenuExitLabel->hide() ;
		ui->TopLabel->hide() ;
		ui->BottomLabel->hide() ;
	}
	else if ( checked )
	{
		screenDumpSize = 1024 ;
		ui->ImageLabel->setScaledContents( false ) ;
		displayType = 0 ;
		ui->ButtonsLabel->show() ;
		ui->LeftLabel->show() ;
		ui->RightLabel->show() ;
		ui->MenuExitLabel->show() ;
		ui->TopLabel->show() ;
		ui->BottomLabel->show() ;
		ui->Lx9dButtonsLabel->hide() ;
		ui->Rx9dButtonsLabel->hide() ;
		ui->xTopLabel->hide() ;
		ui->xBottomLabel->hide() ;
	}
	else
	{
		displayType = 1 ;
		ui->ImageLabel->setScaledContents( false ) ;
		screenDumpSize = 1024 + X9D_EXTRA*8 ;
    ui->ImageLabel->setGeometry( 120, 98, 426, 130) ;
		ui->ButtonsLabel->hide() ;
		ui->LeftLabel->hide() ;
		ui->RightLabel->hide() ;
		ui->MenuExitLabel->hide() ;
		ui->TopLabel->hide() ;
		ui->BottomLabel->hide() ;
		ui->Lx9dButtonsLabel->show() ;
		ui->Rx9dButtonsLabel->show() ;
		ui->xTopLabel->show() ;
		ui->xBottomLabel->show() ;
	}
}

void telemetryDialog::on__9X_RB_toggled( bool checked )
{
	setGraphics( checked ) ;
}

void telemetryDialog::on_backlightButtonScreen_clicked()
{
  QColor c ;
	c = QColorDialog::getColor ( backColour, this ) ;	// , const QString & title, ColorDialogOptions options = 0 ) [static]
	backColour = c.rgb() ;
  QSettings settings("er9x-eePskye", "eePskye");
	settings.setValue("LcdBacklight", backColour ) ;
	setBkColour() ;
}

void telemetryDialog::on_setDirButtonScreen_clicked()
{
	QString directory = QFileDialog::getExistingDirectory(this, tr("Select Save Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
	ui->saveDirectory->setText(directory) ;
  QSettings settings("er9x-eePskye", "eePskye");
	settings.setValue("LcdDumpDir", directory ) ;
}

void telemetryDialog::on_sizeButtonScreen_clicked()
{
	if ( displayType )
	{
		ui->sizeButtonScreen->setText("Larger") ;
		return ;
	}
	if ( lcdSize )
	{
		lcdSize = 0 ;
		ui->sizeButtonScreen->setText("Larger") ;
		ui->ImageLabel->setScaledContents( false ) ;
		ui->ImageLabel->setGeometry( 170, 100, 258, 130 ) ;
		ui->ButtonsLabel->show() ;
		ui->LeftLabel->show() ;
		ui->RightLabel->show() ;
		ui->MenuExitLabel->show() ;
		ui->TopLabel->show() ;
		ui->BottomLabel->show() ;
	}
	else
	{
		lcdSize = 1 ;
		ui->ImageLabel->setScaledContents( true ) ;
		ui->sizeButtonScreen->setText("Smaller") ;
		ui->ImageLabel->setGeometry( 170-64, 60, 384+2, 192+2 ) ;
		ui->ButtonsLabel->hide() ;
		ui->LeftLabel->hide() ;
		ui->RightLabel->hide() ;
		ui->MenuExitLabel->hide() ;
		ui->TopLabel->hide() ;
		ui->BottomLabel->hide() ;
	}
}


void telemetryDialog::makeScreenshot()
{
//	int t ;
	QString path = ui->saveDirectory->text() ;
	QString name = ui->saveName->text() ;
	if ( name.length() == 0 )
	{
		name = "screenDump" ;
	}
  path.append(QDir::separator() + name + tr("_%1.png").arg(screenDumpIndex ) ) ;
	if ( displayType )
	{
		QImage image(256+X9D_EXTRA+X9D_EXTRA, 128, QImage::Format_RGB32);

  	for(int y=0; y<64 ; y++)
		{
  		for(int x=0; x<128+X9D_EXTRA; x++)
			{
				uint32_t pix ;
				int lx ;
				int ly ;
				lx = 2 * x ;
				ly = 2 * y ;
				pix = ((lcdImage[(128+X9D_EXTRA)*(y/8) + x]) & (1<<(y % 8))) ? 0 : backColour ;
      	image.setPixel(lx,ly, pix );
      	image.setPixel(lx+1,ly, pix );
      	image.setPixel(lx,ly+1, pix );
      	image.setPixel(lx+1,ly+1, pix );
			}
		}
		image.save( path ) ;
	}
	else
	{
		QImage image(256, 128, QImage::Format_RGB32);
  	for(int y=0; y<SPLASH_HEIGHT; y++)
		{
  		for(int x=0; x<SPLASH_WIDTH; x++)
			{
				uint32_t pix ;
				int lx ;
				int ly ;
				lx = 2 * x ;
				ly = 2 * y ;
				pix = ((lcdImage[SPLASH_WIDTH*(y/8) + x]) & (1<<(y % 8))) ? 0 : backColour ;
  	    image.setPixel(lx,ly, pix );
  	    image.setPixel(lx+1,ly, pix );
  	    image.setPixel(lx,ly+1, pix );
  	    image.setPixel(lx+1,ly+1, pix );
			}
		}
		image.save( path ) ;
	}
	screenDumpIndex += 1 ;
//	ui->spinBox_3->setValue(screenDumpIndex) ;
}

