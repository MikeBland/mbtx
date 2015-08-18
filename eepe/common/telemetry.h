#ifndef TELEMETRY_H
#define TELEMETRY_H
#include "qextserialport.h"
#include <QDialog>

namespace Ui {
    class telemetryDialog ;
}

class telemetryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit telemetryDialog(QWidget *parent = 0);
    ~telemetryDialog();

private:
    Ui::telemetryDialog *ui;
    QTimer *timer ;
//		int waitForAckNak( int mS ) ;
//    int waitForCan( int mS ) ;
//		int sendOneFile( QString fname ) ;

//    QString fileToSend ;
    
		QextSerialPort *port ;
		int sending ;
		int telemetry ;
		quint8 frskyRxBuffer[19];   // Receive buffer. 9 bytes (full packet), worst case 18 bytes with byte-stuffing (+1)
		quint8 frskyTxBuffer[19];   // Ditto for transmit buffer
		quint8 numPktBytes ;
		quint8 dataState ;
		quint8 sendState ;
		quint8 Frsky_user_state ;
		quint8 Frsky_user_stuff ;
		quint8 Frsky_user_id ;
		quint8 Frsky_user_lobyte ;
		quint8 Frsky_user_hibyte ;
		quint8 Frsky_user_ready ;

private slots:
    void on_exitButton_clicked();
    void on_startButton_clicked();
    void on_startButtonModule_clicked();
		int on_startButtonUart_clicked() ;
    void on_exitButtonModule_clicked();
    void on_ReadAlarmsButton_clicked();
    void on_SetAlarmsButton_clicked();
#ifdef TELEMETRY_LOGGING
		void on_startButtonLogging_clicked() ;
#endif		
		void on_WSdial_valueChanged( int value ) ;
    void timerEvent() ;
		void setupTimer(int time) ;
		void clearCurrentConnection() ;
		void processTelByte( unsigned char c ) ;				
		void processFrskyPacket(quint8 *packet) ;
		quint32 FRSKY_setTxPacket( quint8 type, quint8 value, quint8 p1, quint8 p2 ) ;
		void frskyPushValue(quint8 &i, quint8 value) ;
		void frsky_proc_user_byte( quint8 byte ) ;
		void store_indexed_hub_data( quint8 index, quint16 value ) ;
};

#endif // TELEMETRY_H
