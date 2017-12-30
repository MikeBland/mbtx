#ifndef CELLDIALOG_H
#define CELLDIALOG_H

#include <QDialog>
#include <QtGui>
#include <stdint.h>

namespace Ui {
    class cellDialog ;
}

struct t_cellData
{
	int8_t factors[12] ;
} ;


class cellDialog : public QDialog {
    Q_OBJECT
public:
  cellDialog(QWidget *parent, struct t_cellData *inData ) ;
  ~cellDialog() ;

//    QString getComment();

protected:
	uint32_t *lData ;
	struct t_cellData *pData ;

private slots:
	void accept() ;

private:
	Ui::cellDialog *ui;

};


#endif // CELLDIALOG_H

