#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
    class preferencesDialog;
}

class preferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit preferencesDialog(QWidget *parent = 0);
    ~preferencesDialog();

private:
    Ui::preferencesDialog *ui;

    int currentER9Xrev;
    int currentEEPErev;

    void populateLocale();
    void initSettings();

private slots:
    void write_values();
    void on_er9x_dnld_2_clicked();
    void on_er9x_dnld_clicked();
		void on_downloadVerCB_currentIndexChanged(int index) ;
		
};

#endif // PREFERENCESDIALOG_H
