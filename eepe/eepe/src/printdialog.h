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
    explicit printDialog(QWidget *parent = 0, EEGeneral *gg = 0, ModelData *gm = 0);
    ~printDialog();

    ModelData *g_model;
    EEGeneral *g_eeGeneral;

private:
    Ui::printDialog *ui;

    void printTitle();
    void printSetup();
    void printExpo();
    void printMixes();
    void printLimits();
    void printCurves();
    void printSwitches();
    void printSafetySwitches();

    QString fv(const QString name, const QString value);
    QString getModelName();
    QString getTimer();
    QString getProtocol();
    QString getCenterBeep();
    QString getTrimInc();
    QTextEdit * te;

private slots:
    void on_printButton_clicked();
};

#endif // PRINTDIALOG_H
