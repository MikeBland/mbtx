#ifndef MIXERDIALOG_H
#define MIXERDIALOG_H

#include <QDialog>
#include "pers.h"

namespace Ui {
    class MixerDialog;
}

class MixerDialog : public QDialog {
    Q_OBJECT
public:
		MixerDialog(QWidget *parent, SKYMixData *mixdata, EEGeneral *g_eeGeneral, QString * comment, int modelVersion, struct t_radioData *rData, int flightModeGvars = 0 ) ;
    ~MixerDialog();

//    QString getComment();

protected:
    void changeEvent(QEvent *e);

private slots:
    void valuesChanged();
		void updateChannels() ;


private:
		void addSource( uint8_t index, QString str = "" ) ;
    SKYMixData *md;
    Ui::MixerDialog *ui;
    QString * mixCommennt;
		int leeType ;
		int lType ;
		int lBitType ;
		int lModelVersion ;
		uint32_t lextraPots ;
		EEGeneral *lg_eeGeneral ;
    bool ValuesEditLock ;
		uint8_t sourceMap[120] ;
		uint8_t sourceMapSize ;
		int fmGvars ;
};

#endif // MIXERDIALOG_H
