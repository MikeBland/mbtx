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
		MixerDialog(QWidget *parent, MixData *mixdata, int stickMode, QString * comment, int modelVersion, int eepromType, int delaySpeed) ;
    ~MixerDialog();

//    QString getComment();

protected:
    void changeEvent(QEvent *e);
		void updateChannels() ;

private slots:
    void valuesChanged();
		void setSpeeds() ;


private:
    struct t_radioData *rData ;
    MixData *md;
    Ui::MixerDialog *ui;
    QString * mixCommennt;
		int mType ;
		int delaySlowSpeed ;
    bool ValuesEditLock ;
};

#endif // MIXERDIALOG_H
