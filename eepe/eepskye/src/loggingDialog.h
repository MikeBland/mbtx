#ifndef LOGGINGDIALOG_H
#define LOGGINGDIALOG_H

#include <QDialog>
#include <QtGui>
#include <stdint.h>
#include <QCheckBox>

namespace Ui {
    class loggingDialog ;
}

struct t_loggingData
{
	uint32_t logBits[4] ;
	uint8_t rate ;
	int8_t lswitch ;
	uint8_t newFile ;
	uint8_t timeout ;
} ;


class loggingDialog : public QDialog {
    Q_OBJECT
public:
  loggingDialog(QWidget *parent, struct t_loggingData *inData, struct t_radioData *rData ) ;
  ~loggingDialog() ;

//    QString getComment();

protected:
	uint32_t *lData ;
	struct t_loggingData *pData ;
	struct t_radioData *lrData ;

private slots:
	void accept() ;
	void on_setButton_clicked() ;
	void on_clearButton_clicked() ;

private:
	Ui::loggingDialog *ui;
	QCheckBox *cb[128] ;
	int NumberOfBoxes ;

};


#endif // LOGGINGDIALOG_H

