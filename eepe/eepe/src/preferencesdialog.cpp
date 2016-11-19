#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "avroutputdialog.h"
#include "stamp-eepe.h"
#include "mainwindow.h"
#include <QtGui>
#include <QFileDialog>

#ifndef SKY
extern void populateDownloads( QComboBox *b ) ;
#endif

preferencesDialog::preferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::preferencesDialog)
{
    ui->setupUi(this);

    populateLocale();
    initSettings();

    connect(this,SIGNAL(accepted()),this,SLOT(write_values()));
}

preferencesDialog::~preferencesDialog()
{
    delete ui;
}

void preferencesDialog::write_values()
{
#ifdef SKY
    QSettings settings("er9x-eePskye", "eePskye");
    settings.setValue("startup_check_ersky9x", ui->startupCheck_er9x->isChecked());
    settings.setValue("startup_check_eepskye", ui->startupCheck_eepe->isChecked());
#else
    QSettings settings("er9x-eePe", "eePe");
    settings.setValue("startup_check_er9x", ui->startupCheck_er9x->isChecked());
    settings.setValue("startup_check_eepe", ui->startupCheck_eepe->isChecked());
    settings.setValue("processor", ui->ProcessorCB->currentIndex());
#endif
		settings.setValue("locale", ui->locale_QB->itemData(ui->locale_QB->currentIndex()));
    settings.setValue("default_channel_order", ui->channelorderCB->currentIndex());
    settings.setValue("default_mode", ui->stickmodeCB->currentIndex());
    settings.setValue("show_splash", ui->showSplash->isChecked());
    settings.setValue("download-version", ui->downloadVerCB->currentIndex());
    settings.setValue("default_EE_version", ui->DefaultVersionCB->currentIndex());

	settings.setValue("avrdude_location", avrLoc);
	settings.setValue("programmer", avrProgrammer);
	settings.setValue("mcu", avrMCU);
	settings.setValue("avr_port", avrPort);
	settings.setValue("avr_arguments", avrArgs.join(" "));
#ifndef SKY
  settings.setValue("disablePreread", ui->disablePreReadCB->isChecked());
#endif

}


void preferencesDialog::initSettings()
{
#ifdef SKY
    int dnloadVersion ;
		QSettings settings("er9x-eePskye", "eePskye");
#else
    QSettings settings("er9x-eePe", "eePe");
		populateDownloads( ui->downloadVerCB ) ;
#endif    

    int i=ui->locale_QB->findData(settings.value("locale",QLocale::system().name()).toString());
    if(i<0) i=0;
    ui->locale_QB->setCurrentIndex(i);

    ui->channelorderCB->setCurrentIndex(settings.value("default_channel_order", 0).toInt());
    ui->stickmodeCB->setCurrentIndex(settings.value("default_mode", 1).toInt());
    ui->downloadVerCB->setCurrentIndex(settings.value("download-version", 0).toInt());
    ui->ProcessorCB->setCurrentIndex(settings.value("processor", 0).toInt());

    ui->startupCheck_er9x->setChecked(settings.value("startup_check_er9x", true).toBool());
    ui->startupCheck_eepe->setChecked(settings.value("startup_check_eepe", true).toBool());

    ui->showSplash->setChecked(settings.value("show_splash", true).toBool());

    currentER9Xrev = settings.value("currentER9Xrev", 1).toInt();
    ui->DefaultVersionCB->setCurrentIndex(settings.value("default_EE_version", 0).toInt());

    ui->er9x_ver_label->setText(QString("r%1").arg(currentER9Xrev));
#ifdef Q_OS_WIN32
	avrLoc = settings.value("avrdude_location", QFileInfo("avrdude.exe").absoluteFilePath()).toString();
#elif __APPLE__
	avrLoc = settings.value("avrdude_location", "/usr/local/bin/avrdude").toString();
#else
	avrLoc = settings.value("avrdude_location", "avrdude").toString();
#endif
#ifndef SKY
	ui->disablePreReadCB->setChecked(settings.value("disablePreread", false).toBool());
#endif

  populateProgrammers() ;
  ui->avrdude_programmer->model()->sort(0);
	QString str = settings.value("avr_arguments").toString();
	avrArgs = str.split(" ", QString::SkipEmptyParts);
	avrProgrammer =  settings.value("programmer", QString("usbasp")).toString();
	avrMCU =  settings.value("mcu", QString("m64")).toString();
	avrPort =  settings.value("avr_port", "").toString();

	ui->avrdude_location->setText(getAVRDUDE());
	ui->avrArgs->setText(getAVRArgs().join(" "));

	int idx1 = ui->avrdude_programmer->findText(getProgrammer());
	int idx2 = ui->avrdude_port->findText(getPort());
	int idx3 = ui->avrdude_mcu->findText(getMCU());
	if(idx1>=0) ui->avrdude_programmer->setCurrentIndex(idx1);
	if(idx2>=0) ui->avrdude_port->setCurrentIndex(idx2);
	if(idx3>=0) ui->avrdude_mcu->setCurrentIndex(idx3);
}

void preferencesDialog::populateLocale()
{
    ui->locale_QB->clear();
    ui->locale_QB->addItem("English (default)", "");


    QStringList strl = QApplication::arguments();
    if(!strl.count()) return;

    QString path = ".";
#ifdef Q_OS_WIN32
    if(strl.count()) path = QFileInfo(strl[0]).canonicalPath() + "/lang";
#else
    if(strl.count()) path = QFileInfo(strl[0]).canonicalPath() + "/eepefiles";
#endif


    QDir directory = QDir(path);
    QStringList files = directory.entryList(QStringList("eepe_*.qm"), QDir::Files | QDir::NoSymLinks);

    foreach(QString file, files)
    {
        QLocale loc(file.mid(5,2));
        ui->locale_QB->addItem(QLocale::languageToString(loc.language()), loc.name());
    }


    //ui->locale_QB->addItems(files);


}


#ifdef SKY
void preferencesDialog::on_downloadVerCB_currentIndexChanged(int index)
{
	QSettings settings("er9x-eePskye", "eePskye");
	if ( index == 0 )
	{
    currentER9Xrev = settings.value("currentERSKY9Xrev", 1).toInt();
		ui->label_CurrentVersion->setText( "Current Version - ersky9x" ) ;
	}
	else
	{
    currentER9Xrev = settings.value("currentERSKY9XRrev", 1).toInt();
		ui->label_CurrentVersion->setText( "Current Version - ersky9xr" ) ;
	}
  ui->er9x_ver_label->setText(QString("r%1").arg(currentER9Xrev));
  settings.setValue("download-version", ui->downloadVerCB->currentIndex());
}
#endif

void preferencesDialog::on_er9x_dnld_2_clicked()
{
    MainWindow * mw = (MainWindow *)this->parent();

		write_values() ;		// In case changed
    mw->checkForUpdates(true);
}

void preferencesDialog::on_er9x_dnld_clicked()
{
    MainWindow * mw = (MainWindow *)this->parent();

		write_values() ;		// In case changed
    mw->downloadLatester9x();
}



void preferencesDialog::populateProgrammers()
{
  QString fileName = QFileInfo(avrLoc).dir().absolutePath() + "/avrdude.conf"; //for windows
//  QString fileName = QDir(avrLoc).absolutePath() + "/avrdude.conf"; //for windows
  if(!QFileInfo(fileName).exists()) fileName = "/etc/avrdude.conf"; //for linux
  if(!QFileInfo(fileName).exists()) return; // not found avrdude.conf - returning

  QFile file(fileName);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

  QStringList items;
  QString line = "";
  QString prevline = "";
  QTextStream in(&file);
  while (!in.atEnd())
  {
      prevline = line;
      line = in.readLine();

      if(prevline.left(10).toLower().replace(" ","")=="programmer")
          items << line.section('"',1,1);
  }
  file.close();

  items.sort();

  QString avrProgrammer_temp = avrProgrammer;
  ui->avrdude_programmer->clear();
  ui->avrdude_programmer->addItems(items);
  int idx1 = ui->avrdude_programmer->findText(avrProgrammer_temp);
  if(idx1>=0) ui->avrdude_programmer->setCurrentIndex(idx1);
}

void preferencesDialog::on_avrdude_programmer_currentIndexChanged(QString )
{
    avrProgrammer = ui->avrdude_programmer->currentText();
}

void preferencesDialog::on_avrdude_mcu_currentIndexChanged(QString )
{
    avrMCU = ui->avrdude_mcu->currentText();
}

void preferencesDialog::on_avrdude_location_editingFinished()
{
    avrLoc = ui->avrdude_location->text();
}

void preferencesDialog::on_avrArgs_editingFinished()
{
    avrArgs = ui->avrArgs->text().split(" ", QString::SkipEmptyParts);
}

void preferencesDialog::on_avrdude_port_currentIndexChanged(QString )
{
    avrPort = ui->avrdude_port->currentText();
}

void preferencesDialog::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Location"),ui->avrdude_location->text());

    if(!fileName.isEmpty())
    {
        ui->avrdude_location->setText(fileName);
        avrLoc = fileName;
    }
}


void preferencesDialog::listProgrammers()
{
    QStringList args   = avrArgs;
    QStringList arguments;
    arguments << "-c?" << args ;

    avrOutputDialog *ad = new avrOutputDialog(this, ui->avrdude_location->text(), arguments, "List available programmers", AVR_DIALOG_KEEP_OPEN);
    ad->setWindowIcon(QIcon(":/images/list.png"));
    ad->show();
}

void preferencesDialog::on_pushButton_3_clicked()
{
    listProgrammers();
}



void preferencesDialog::on_pushButton_4_clicked()
{
    QStringList arguments;
    arguments << "-?";

    avrOutputDialog *ad = new avrOutputDialog(this, ui->avrdude_location->text(), arguments, "Show help", AVR_DIALOG_KEEP_OPEN);
    ad->setWindowIcon(QIcon(":/images/configure.png"));
    ad->show();
}

