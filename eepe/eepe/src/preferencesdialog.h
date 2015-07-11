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

	QString getAVRDUDE() {return avrLoc;}
	QStringList getAVRArgs() {return avrArgs;}
	QString getProgrammer() {return avrProgrammer;}
	QString getMCU() {return avrMCU;}
	QString getPort() {return avrPort;}

private:
    Ui::preferencesDialog *ui;

  void populateProgrammers();
    int currentER9Xrev;
    int currentEEPErev;

	void listProgrammers() ;
	QString avrLoc;
	QStringList avrArgs;
	QString avrProgrammer;
	QString avrMCU;
	QString avrPort;

    void populateLocale();
    void initSettings();

private slots:
    void write_values();
    void on_er9x_dnld_2_clicked();
    void on_er9x_dnld_clicked();
  void on_pushButton_4_clicked();
  void on_pushButton_3_clicked();
  void on_pushButton_clicked();
  void on_avrArgs_editingFinished();
  void on_avrdude_location_editingFinished();
  void on_avrdude_programmer_currentIndexChanged(QString );
  void on_avrdude_mcu_currentIndexChanged(QString );
  void on_avrdude_port_currentIndexChanged(QString );
};

#endif // PREFERENCESDIALOG_H
