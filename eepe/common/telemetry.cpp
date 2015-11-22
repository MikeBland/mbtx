
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
#include <QtCore/QList>
#include <QString>
#include <QMessageBox>

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
	Frsky_user_state = 0 ;
	ui->startButton->setText("Start") ;
  ui->WSvalue->setText(tr("%1").arg(ui->WSdial->value())) ;

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
		port->setBaudRate(BAUD115200) ;
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


void telemetryDialog::on_startButton_clicked()
{
	QString portname ;

	if ( telemetry )
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


