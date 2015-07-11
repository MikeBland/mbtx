#ifndef SERIALDIALOG_H
#define SERIALDIALOG_H
#include "qextserialport.h"
#include <QDialog>

#define NO_LIST			0
#define LIST_FILES	1
#define LIST_MODELS	2

namespace Ui {
    class serialDialog;
}

class serialDialog : public QDialog
{
    Q_OBJECT

public:
    explicit serialDialog(QWidget *parent = 0);
    ~serialDialog();

private:
    Ui::serialDialog *ui;
		int fileListContents ;
		int waitForAckNak( int mS ) ;
    int waitForCan( int mS ) ;
		int sendOneFile( QString fname ) ;
        int Receive_Byte( unsigned char *c, int mS ) ;
        int Receive_Packet (unsigned char *data, int *length, int timeout) ;
		int receiveOneFile( QString fname ) ;
		int startSerialPort() ;
		QString ComPort ;
		int comPortLoading ;

//    QString fileToSend ;
    
		QextSerialPort *port ;

private slots:
    void on_cancelButton_clicked();
		void on_FileEdit_editingFinished() ;
		void on_browseButton_clicked() ;
		void on_sendButton_clicked() ;
    void on_listButton_clicked() ;
    void on_receiveButton_clicked() ;
		void on_deleteButton_clicked() ;
		void on_setDirButton_clicked() ;
		void on_portCB_currentIndexChanged( int index ) ;
};

#endif // SERIAL1DIALOG_H
