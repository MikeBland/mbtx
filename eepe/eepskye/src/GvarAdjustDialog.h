#ifndef ADJUSTER_H
#define ADJUSTER_H

#include <QDialog>
#include "pers.h"
#include "myeeprom.h"

namespace Ui {
    class GvarAdjustDialog ;
}

class GvarAdjustDialog : public QDialog {
    Q_OBJECT
public:
	GvarAdjustDialog(QWidget *parent, GvarAdjust *ingad, struct t_radioData *rData ) ;
  ~GvarAdjustDialog();

//    QString getComment();

//protected:
//    void changeEvent(QEvent *e);

private slots:
    void valuesChanged();


private:
	GvarAdjust *gad ;
	struct t_radioData *rData ;
	uint32_t oldFunction ;
	uint32_t updateLock ;
  Ui::GvarAdjustDialog *ui;
	int leeType ;
//	SKYModelData *lpModel ;
	void updateDisplay( void ) ;
} ;

#endif // ADJUSTER_H

