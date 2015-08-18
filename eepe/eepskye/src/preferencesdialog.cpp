#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "stamp-eepskye.h"
#include "mainwindow.h"
#include <QtGui>

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
#ifdef SKY
    settings.setValue("default-model-version", ui->defaultVersionCB->currentIndex());
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

		dnloadVersion = settings.value("download-version", 0).toInt() ;
    ui->downloadVerCB->setCurrentIndex( dnloadVersion ) ;

    ui->startupCheck_er9x->setChecked(settings.value("startup_check_ersky9x", true).toBool());
    ui->startupCheck_eepe->setChecked(settings.value("startup_check_eepskye", true).toBool());

    ui->showSplash->setChecked(settings.value("show_splash", true).toBool());

		// download version 0 for ersky9x, 1 for ersky9xR
		if ( dnloadVersion == 0 )
		{
    	currentER9Xrev = settings.value("currentERSKY9Xrev", 1).toInt();
			ui->label_CurrentVersion->setText( "Current Version - ersky9x" ) ;
		}
		else if ( dnloadVersion == 1 )
		{
    	currentER9Xrev = settings.value("currentERSKY9XRrev", 1).toInt();
			ui->label_CurrentVersion->setText( "Current Version - ersky9xr" ) ;
		}
		else if ( dnloadVersion == 2 )
		{
    	currentER9Xrev = settings.value("currentERSKYX9Drev", 1).toInt();
			ui->label_CurrentVersion->setText( "Current Version - erskyX9D" ) ;
		}
		else if ( dnloadVersion == 3 )
		{
    	currentER9Xrev = settings.value("currentERSKYX9DPrev", 1).toInt();
			ui->label_CurrentVersion->setText( "Current Version - erskyX9DP" ) ;
		}
		else
		{
    	currentER9Xrev = settings.value("currentERSKY9XTrev", 1).toInt();
			ui->label_CurrentVersion->setText( "Current Version - ersky9XT" ) ;
		}
    ui->er9x_ver_label->setText(QString("r%1").arg(currentER9Xrev));
#ifdef SKY
		ui->defaultVersionCB->setCurrentIndex( settings.value("default-model-version", 0 ).toInt() ) ;
#endif
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
  else if ( index == 1 )
	{
    currentER9Xrev = settings.value("currentERSKY9XRrev", 1).toInt();
		ui->label_CurrentVersion->setText( "Current Version - ersky9xr" ) ;
	}
  else if ( index == 2 )
	{
    currentER9Xrev = settings.value("currentERSKYX9Drev", 1).toInt();
		ui->label_CurrentVersion->setText( "Current Version - erskyX9D" ) ;
	}
  else if ( index == 3 )
	{
    currentER9Xrev = settings.value("currentERSKYX9DPrev", 1).toInt();
		ui->label_CurrentVersion->setText( "Current Version - erskyX9DP" ) ;
	}
	else
	{
    currentER9Xrev = settings.value("currentERSKY9XTrev", 1).toInt();
		ui->label_CurrentVersion->setText( "Current Version - ersky9XT" ) ;
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
		
		write_values() ;			// In case changed
    mw->downloadLatester9x();
}

