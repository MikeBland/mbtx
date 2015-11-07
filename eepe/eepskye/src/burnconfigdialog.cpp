#include "burnconfigdialog.h"
#include "ui_burnconfigdialog.h"
#include "avroutputdialog.h"
#include <QtGui>
#include <QFileDialog>

burnConfigDialog::burnConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::burnConfigDialog)
{
    ui->setupUi(this);
//    ui->avrdude_programmer->model()->sort(0);

    getSettings();
//    populateProgrammers();
    connect(this,SIGNAL(accepted()),this,SLOT(putSettings()));
}

burnConfigDialog::~burnConfigDialog()
{
    delete ui;
}

void burnConfigDialog::getSettings()
{
    QSettings settings("er9x-eePskye", "eePskye");

#ifdef Q_OS_WIN32
    sambaLoc = settings.value("SAM_BA_location", QFileInfo("sam-ba.exe").absoluteFilePath()).toString();
#elif __APPLE__
    sambaLoc = settings.value("SAM_BA_location", "/usr/local/bin/sam-ba").toString();
#else
    sambaLoc = settings.value("SAM_BA_location", QFileInfo("sam-ba.exe").absoluteFilePath()).toString();
#endif

		useSamba = settings.value( "Use_Sam_Ba", 0 ).toInt() ;
    QString str = settings.value("avr_arguments").toString();
    avrArgs = str.split(" ", QString::SkipEmptyParts);
    avrProgrammer =  settings.value("programmer", QString("usbasp")).toString();
    avrMCU =  settings.value("mcu", QString("at91sam3s4-9x")).toString();
    armPort =  settings.value("avr_port", "\\USBserial\\COM8").toString();

    ui->avrdude_location->setText(getAVRDUDE());
    ui->armPort->setText(getARMPort());
	  ui->UseSambaCB->setChecked(useSamba) ;

		if ( useSamba == 0 )
		{
			ui->avrdude_location->setEnabled(false) ;
	    ui->armPort->setEnabled(false) ;
		  ui->avrdude_mcu->setEnabled(false) ;
		}

//    int idx1 = ui->avrdude_programmer->findText(getProgrammer());
//    int idx2 = ui->avrdude_port->findText(getPort());
    int idx3 = ui->avrdude_mcu->findText(getMCU());
//    if(idx1>=0) ui->avrdude_programmer->setCurrentIndex(idx1);
    //if(idx2>=0) ui->avrdude_port->setCurrentIndex(idx2);
    if(idx3>=0) ui->avrdude_mcu->setCurrentIndex(idx3);
}

void burnConfigDialog::putSettings()
{
//    avrTempDir = ui->temp_location->text();
//    sambaLoc = ui->avrdude_location->text();
//    avrArgs = ui->avrArgs->text().split(" ", QString::SkipEmptyParts);
//    avrProgrammer = ui->avrdude_programmer->currentText();
//    avrPort = ui->avrdude_port->currentText();
//    avrEraseEEPROM = ui->eraseEEPROM_CB->isChecked();


		QSettings settings("er9x-eePskye", "eePskye");
    settings.setValue( "Use_Sam_Ba", useSamba ) ;
    settings.setValue("SAM_BA_location", sambaLoc ) ;
    settings.setValue("programmer", avrProgrammer ) ;
    settings.setValue("mcu", avrMCU);
    settings.setValue("avr_port", armPort);
    settings.setValue("avr_arguments", avrArgs.join(" "));
}

void burnConfigDialog::populateProgrammers()
{
    QString fileName = QDir(sambaLoc).absolutePath() + "/avrdude.conf"; //for windows
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

//    QString avrProgrammer_temp = avrProgrammer;
//    ui->avrdude_programmer->clear();
//    ui->avrdude_programmer->addItems(items);
//    int idx1 = ui->avrdude_programmer->findText(avrProgrammer_temp);
//    if(idx1>=0) ui->avrdude_programmer->setCurrentIndex(idx1);
}

//void burnConfigDialog::on_avrdude_programmer_currentIndexChanged(QString )
//{
//    avrProgrammer = ui->avrdude_programmer->currentText();
//}

void burnConfigDialog::on_UseSambaCB_stateChanged( int )
{
	bool state ;
  useSamba = ui->UseSambaCB->isChecked() ;
	if ( useSamba == 0 )
	{
		state = false ;
	}
	else
	{
		state = true ;
	}
	ui->avrdude_location->setEnabled(state) ;
  ui->armPort->setEnabled(state) ;
  ui->avrdude_mcu->setEnabled(state) ;
}

void burnConfigDialog::on_avrdude_mcu_currentIndexChanged(QString )
{
    avrMCU = ui->avrdude_mcu->currentText();
}

void burnConfigDialog::on_avrdude_location_editingFinished()
{
    sambaLoc = ui->avrdude_location->text();
}

void burnConfigDialog::on_armPort_editingFinished()
{
    armPort = ui->armPort->text() ; //.split(" ", QString::SkipEmptyParts);
}

//void burnConfigDialog::on_avrdude_port_currentIndexChanged(QString )
//{
//    avrPort = ui->avrdude_port->currentText();
//}

void burnConfigDialog::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Location"),ui->avrdude_location->text());

    if(!fileName.isEmpty())
    {
        ui->avrdude_location->setText(fileName);
        sambaLoc = fileName;
    }
}


void burnConfigDialog::listProgrammers()
{
    QStringList arguments;
    arguments << "-c?";

    avrOutputDialog *ad = new avrOutputDialog(this, ui->avrdude_location->text(), arguments, "List available programmers", AVR_DIALOG_KEEP_OPEN);
    ad->setWindowIcon(QIcon(":/images/list.png"));
    ad->show();
}

//void burnConfigDialog::on_pushButton_3_clicked()
//{
//    listProgrammers();
//}



void burnConfigDialog::on_pushButton_4_clicked()
{
    QStringList arguments;
    arguments << "-?";

    avrOutputDialog *ad = new avrOutputDialog(this, ui->avrdude_location->text(), arguments, "Show help", AVR_DIALOG_KEEP_OPEN);
    ad->setWindowIcon(QIcon(":/images/configure.png"));
    ad->show();
}


//void burnConfigDialog::readFuses()
//{
//    QStringList args   = avrArgs;
//    if(!armPort.isEmpty()) args << "-P" << armPort;

//    QStringList str;
//    str << "-U" << "lfuse:r:-:i" << "-U" << "hfuse:r:-:i" << "-U" << "efuse:r:-:i";

//    QStringList arguments;
//    arguments << "-c" << avrProgrammer << "-p" << avrMCU << args << str;

//    avrOutputDialog *ad = new avrOutputDialog(this, sambaLoc, arguments, "Read Fuses",AVR_DIALOG_KEEP_OPEN);
//    ad->setWindowIcon(QIcon(":/images/fuses.png"));
//    ad->show();
//}

//void burnConfigDialog::restFuses(bool eeProtect)
//{
//    //fuses
//    //avrdude -c usbasp -p m64 -U lfuse:w:<0x0E>:m
//    //avrdude -c usbasp -p m64 -U hfuse:w:<0x89>:m  0x81 for eeprom protection
//    //avrdude -c usbasp -p m64 -U efuse:w:<0xFF>:m

//    QMessageBox::StandardButton ret = QMessageBox::Cancel;

//    QString msg = "<b><u>SET FUSES</u></b><br>";

//    if(eeProtect)
//        msg.append(tr("The following action will protect the EEPROM from being deleted when flashing new Firmware. "));
//    else
//        msg.append(tr("This will reset the fuses to the factory settings. "));

//    msg.append(tr("Before continuing make sure that your radio is connected and the programmer works reliably.<p>"));
//    msg.append(tr("<font color=red>DO NOT DISCONNECT OR POWER DOWN UNTIL THE PROGRAM COMPLETES!</font><p>"));
//    msg.append(tr("Click 'Ok' to continue or 'Cancel' to quit."));

//    ret = QMessageBox::information(this, tr("eePe"), msg, QMessageBox::Cancel | QMessageBox::Ok);
//    if (ret == QMessageBox::Ok)
//    {
//        QStringList args   = avrArgs;
//        if(!armPort.isEmpty()) args << "-P" << armPort;

//        QString erStr = eeProtect ? "hfuse:w:0x81:m" : "hfuse:w:0x89:m";
//        QStringList str;
//        str << "-U" << "lfuse:w:0x0E:m" << "-U" << erStr << "-U" << "efuse:w:0xFF:m";
//        //use hfuse = 0x81 to prevent eeprom being erased with every flashing

//        QStringList arguments;
//        arguments << "-c" << avrProgrammer << "-p" << avrMCU << args << "-u" << str;

//        avrOutputDialog *ad = new avrOutputDialog(this, sambaLoc, arguments, "Reset Fuses",AVR_DIALOG_KEEP_OPEN);
//        ad->setWindowIcon(QIcon(":/images/fuses.png"));
//        ad->show();
//    }

//}


