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

protected:
	bool eventFilter(QObject *obj, QEvent *ev) ;

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
		int lcdScreen ;
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
        quint8 lcdImage[1024+(212-128)*8] ;
		quint8 lcdState ;
		quint8 lcdSize ;
		quint16 lcdCounter ;
		quint8 buttonStatus ;
		quint8 lastButtonStatus ;
		quint8 switchStatusLow ;
		quint8 switchStatusHigh ;
		quint8 displayType ;
		quint16 screenDumpIndex ;
		quint16 screenDumpSize ;
        quint32 backColour ;
		QPolygon lPolygon[10] ;

private slots:
    void on_exitButton_clicked();
    void on_startButton_clicked();
    void on_startButtonModule_clicked();
		void on_startButtonScreen_clicked() ;
		void on_saveButtonScreen_clicked() ;
		void on_backlightButtonScreen_clicked() ;
		void on_setDirButtonScreen_clicked() ;
		void on_sizeButtonScreen_clicked() ;
		void on__9X_RB_toggled( bool ) ;
		int on_startButtonUart_clicked() ;
    void on_exitButtonModule_clicked();
    void on_ReadAlarmsButton_clicked();
    void on_SetAlarmsButton_clicked();
		void makeScreenshot() ;
		void setBkColour() ;
		void setGraphics( bool ) ;

		void on_startButtonLogging_clicked() ;
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
		void leftMousePressEvent(QMouseEvent * event) ;
		void leftMouseReleaseEvent(QMouseEvent * event) ;
		void rightMousePressEvent(QMouseEvent * event) ;
		void rightMouseReleaseEvent(QMouseEvent * event) ;
		void leftXMousePressEvent(QMouseEvent * event) ;
		void leftXMouseReleaseEvent(QMouseEvent * event) ;
		void rightXMousePressEvent(QMouseEvent * event) ;
		void rightXMouseReleaseEvent(QMouseEvent * event) ;
};

#endif // TELEMETRY_H
