#ifndef DONATORSDIALOG_H
#define DONATORSDIALOG_H

#include <QDialog>

namespace Ui {
    class donatorsDialog;
}

class donatorsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit donatorsDialog(QWidget *parent = 0);
    ~donatorsDialog();

private:
    Ui::donatorsDialog *ui;

    void showEvent ( QShowEvent * );
};

#endif // DONATORSDIALOG_H
