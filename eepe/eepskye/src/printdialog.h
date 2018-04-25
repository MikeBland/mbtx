#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QDialog>
#include <QtGui>
#include "pers.h"

class QTextEdit;

namespace Ui {
    class printDialog;
}

class printDialog : public QDialog
{
    Q_OBJECT

public:
    explicit printDialog(QWidget *parent = 0, EEGeneral *gg = 0, SKYModelData *gm = 0, struct t_radioData *radioData = 0 );
    ~printDialog();

    SKYModelData *g_model;
    EEGeneral *g_eeGeneral;

private:
    Ui::printDialog *ui;
    struct t_radioData *rData ;

    void printTitle();
    void printSetup();
    void printExpo();
    void printMixes();
    void printLimits();
    void printCurves();
    void printSwitches();
    void printSafetySwitches();
		void printModes() ;
		void printVoice() ;

    QString fv(const QString name, const QString value);
    QString getModelName();
    QString getTimer( uint8_t timer );
    QString getProtocol();
    QString getCenterBeep();
    QString getTrimInc();
    QTextEdit * te;

private slots:
    void on_printButton_clicked();
};

#endif // PRINTDIALOG_H
