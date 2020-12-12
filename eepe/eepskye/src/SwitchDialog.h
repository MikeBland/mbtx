#ifndef SWITCHDIALOG_H
#define SWITCHDIALOG_H

#include <QDialog>
#include "pers.h"

struct t_switchData
{
	struct te_CSwData swData ;
	uint8_t switchDelay ;
} ;

namespace Ui {
    class SwitchDialog;
}

class SwitchDialog : public QDialog {
    Q_OBJECT
public:

	SwitchDialog(QWidget *parent, int index, struct t_switchData *sdata, int modelVersion, uint8_t id, struct t_radioData *rData ) ;
	~SwitchDialog() ;

private:
	Ui::SwitchDialog *ui;
	uint32_t sindex ;
	uint32_t leeType ;
	uint32_t lextraPots ;
	uint32_t ltype ;
	uint8_t ValuesEditLock ;
	uint8_t OnEditLock ;
	uint8_t OffEditLock ;
  uint8_t id_model ;
	double lastOnTime ;
	double lastOffTime ;
	struct t_switchData *lsdata ;
	int lmodelVersion ;
	struct t_radioData *lrData ;

	void update() ;

private slots:
	void on_timeOnSB_valueChanged( double x ) ;
	void on_timeOffSB_valueChanged( double x ) ;
  void valuesChanged() ;

} ;

#endif

