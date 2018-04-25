/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QMdiArea>
#include <QApplication>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QPen>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QTextBrowser>
#include <QSpinBox>
#include <QCheckBox>

#include "mainwindow.h"
#include "mdichild.h"
#include "burnconfigdialog.h"
#include "avroutputdialog.h"
#include "donatorsdialog.h"
#include "preferencesdialog.h"
#include "downloaddialog.h"
#include "customizesplashdialog.h"
#include "stamp-eepe.h"
#include "serialdialog.h"
#include "../../common/telemetry.h"
#include "../../common/reviewOutput.h"
#include "simulatordialog.h"

#define DONATE_ER_STR "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=B9RNATGH7DTQ6"
#define DONATE_MB_STR "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=YHX43JR3J7XGW"

//er9x.hex - with language variants (no templates)
//er9x-frsky - with language variants
//er9x-128 - with language variants and an option to choose if the THR and AIL switches have been moved

//The following in English only:
//er9x-ardupilot
//er9x-jeti
//er9x-nmea


//#define DNLD_VER_ER9X            0
//#define DNLD_VER_ER9X_JETI       1
//#define DNLD_VER_ER9X_FRSKY      2
//#define DNLD_VER_ER9X_ARDUPILOT  3
//#define DNLD_VER_ER9X_NMEA       4
//#define DNLD_VER_ER9X_128				 5

//#define DNLD_VER_ER9X_DE         6
//#define DNLD_VER_ER9X_FRSKY_DE   7
//#define DNLD_VER_ER9X_128_DE  	 8

//#define DNLD_VER_ER9X_FR         9
//#define DNLD_VER_ER9X_FRSKY_FR   10
//#define DNLD_VER_ER9X_128_FR  	 11

//#define ER9X_URL   "http://er9x.googlecode.com/svn/trunk/er9x.hex"
//#define ER9X_JETI_URL   "http://er9x.googlecode.com/svn/trunk/er9x-jeti.hex"
//#define ER9X_FRSKY_URL   "http://er9x.googlecode.com/svn/trunk/er9x-frsky.hex"
//#define ER9X_ARDUPILOT_URL   "http://er9x.googlecode.com/svn/trunk/er9x-ardupilot.hex"
//#define ER9X_NMEA_URL   "http://er9x.googlecode.com/svn/trunk/er9x-nmea.hex"
//#define ER9X_128_URL   "http://er9x.googlecode.com/svn/trunk/er9x-128.hex"

//#define ER9X_DE_URL   "http://er9x.googlecode.com/svn/trunk/er9x_de.hex"
//#define ER9X_FRSKY_DE_URL   "http://er9x.googlecode.com/svn/trunk/er9x-frsky_de.hex"
//#define ER9X_128_DE_URL   "http://er9x.googlecode.com/svn/trunk/er9x-128_de.hex"

//#define ER9X_FR_URL   "http://er9x.googlecode.com/svn/trunk/er9x_fr.hex"
//#define ER9X_FRSKY_FR_URL   "http://er9x.googlecode.com/svn/trunk/er9x-frsky_fr.hex"
//#define ER9X_128_FR_URL   "http://er9x.googlecode.com/svn/trunk/er9x-128_fr.hex"




#define DNLD_VER_ER9X            0
#define DNLD_VER_ER9X_JETI       1
#define DNLD_VER_ER9X_FRSKY      2
#define DNLD_VER_ER9X_ARDUPILOT  3
#define DNLD_VER_ER9X_FRSKY_NOHT 4
#define DNLD_VER_ER9X_NOHT       5

//#define DNLD_VER_ER9X_SPKR            6
//#define DNLD_VER_ER9X_NOHT_SPKR       7
//#define DNLD_VER_ER9X_FRSKY_SPKR      8
//#define DNLD_VER_ER9X_FRSKY_NOHT_SPKR 9
#define DNLD_VER_ER9X_NMEA       6
#define DNLD_VER_ER9X_128				 7
#define DNLD_VER_ER9X_S128			 8
#define DNLD_VER_ER9X_DE         9
#define DNLD_VER_ER9X_FRSKY_DE	 10
#define DNLD_VER_ER9X_128_DE		 11
#define DNLD_VER_ER9X_NO         12
#define DNLD_VER_ER9X_2561       13
#define DNLD_VER_ER9X_FRSKY_SV   14

#define ER9X_URL   "http://www.er9x.com/er9x.hex"
#define ER9X_NOHT_URL   "http://www.er9x.com/er9x-noht.hex"
//#define ER9X_SPKR_URL   "http://er9x.googlecode.com/svn/trunk/er9x-spkr.hex"
//#define ER9X_NOHT_SPKR_URL   "http://er9x.googlecode.com/svn/trunk/er9x-noht-spkr.hex"
#define ER9X_JETI_URL   "http://www.er9x.com/er9x-jeti.hex"
#define ER9X_FRSKY_URL   "http://www.er9x.com/er9x-frsky.hex"
#define ER9X_FRSKY_SV_URL   "http://www.er9x.com/er9x-frsky-sv.hex"
#define ER9X_FRSKY_NOHT_URL   "http://www.er9x.com/er9x-frsky.hex"
//#define ER9X_FRSKY_SPKR_URL   "http://er9x.googlecode.com/svn/trunk/er9x-frsky-spkr.hex"
//#define ER9X_FRSKY_NOHT_SPKR_URL   "http://er9x.googlecode.com/svn/trunk/er9x-frsky-noht-spkr.hex"
#define ER9X_ARDUPILOT_URL   "http://er9x.googlecode.com/svn/trunk/er9x-ardupilot.hex"
#define ER9X_NMEA_URL   "http://www.er9x.com/er9x-nmea.hex"
#define ER9X_128_URL   "http://www.er9x.com/er9x-128.hex"
#define ER9X_128_S_URL   "http://www.er9x.com/er9x-128.hex"
//#define ER9X_STAMP "http://www.er9x.com/stamp-er9x.h"
#define EEPE_URL   "http://www.er9x.com/eePeInstall.exe"
//#define EEPE_STAMP "http://www.er9x.com/stamp-eepe.h"
#define ER9X_DE_URL   "http://www.er9x.com/er9x-de.hex"
#define ER9X_FRSKY_DE_URL   "http://www.er9x.com/er9x-frsky-de.hex"
#define ER9X_128_DE_URL   "http://www.er9x.com/er9x-128-de.hex"
#define ER9X_NO_URL   "http://www.er9x.com/er9x-no.hex"
#define ER9X_2561_URL   "http://www.er9x.com/er9x-2561.hex"

#define GITHUB_REVS_URL	"http://www.er9x.com/Revisions.txt"

class simulatorDialog *SimPointer = 0 ;
QString AvrdudeOutput ;

int ReleaseChecked ;

void populateDownloads( QComboBox *b )
{
	b->clear() ;
  b->addItem( "er9x" ) ;
  b->addItem( "er9x - JETI" ) ;
  b->addItem( "er9x - FrSky" ) ;
  b->addItem( "er9x - Ardupilot" ) ;
  b->addItem( "er9x - FrSky NOHT" ) ;
  b->addItem( "er9x - NOHT" ) ;
  b->addItem( "er9x - NMEA" ) ;
  b->addItem( "er9x - 128 - FrSky" ) ;
  b->addItem( "er9x - 128 - Standard" ) ;
  b->addItem( "er9x - German" ) ;
  b->addItem( "er9x - FrSky - German" ) ;
  b->addItem( "er9x - 128 - German" ) ;
  b->addItem( "er9x - Norwegian" ) ;
  b->addItem( "er9x - 2561" ) ;
  b->addItem( "er9x - FrSky - Serial Voice" ) ;
}	

MainWindow::MainWindow()
{
    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(updateMenus()));
    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)),
            this, SLOT(setActiveSubWindow(QWidget*)));

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    updateMenus();

    readSettings();

#ifdef V2
    setWindowTitle(tr("eePeV2 - EEPROM Editor"));
#else
    setWindowTitle(tr("eePe - EEPROM Editor"));
#endif
    
		setAcceptDrops(true);
		
		setUnifiedTitleAndToolBarOnMac(true);
    this->setWindowIcon(QIcon(":/icon.png"));

    checkForUpdates(false);


    QStringList strl = QApplication::arguments();
    QString str;
    if(strl.count()>1) str = strl[1];
    if(!str.isEmpty())
    {
//        MdiChild tch;
        int fileType = MdiChild::getFileType(str);

        if(fileType==FILE_TYPE_HEX)
        {
            burnToFlash(str);
        }

        if(fileType==FILE_TYPE_EEPE || fileType==FILE_TYPE_EEPM  || fileType==FILE_TYPE_EEPG)
        {
            MdiChild *child = createMdiChild();
            if (child->loadFile(str))
            {
                statusBar()->showMessage(tr("File loaded"), 2000);
                child->show();
                if(!child->parentWidget()->isMaximized() && !child->parentWidget()->isMinimized()) child->parentWidget()->resize(400,500);
            }
        }
    }
		simulatorDialog *sd ;
 	  sd = new simulatorDialog(this) ;
		SimPointer = sd ;

		if ( currentEEPErev > currentEEPErelease )
		{
			releaseNotes() ;
			if ( ReleaseChecked )
			{
				currentEEPErelease = currentEEPErev ;
				QSettings settings("er9x-eePe", "eePe") ;
		    settings.setValue("currentEEPErelease", currentEEPErelease ) ;
			}
		}
}

void MainWindow::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();
  if (urls.isEmpty())
    return;
    
	QString fileName = urls.first().toLocalFile();
  if (fileName.isEmpty())
    return;
//  g.eepromDir(QFileInfo(fileName).dir().absolutePath());

  QMdiSubWindow *existing = findMdiChild(fileName);
  if (existing) {
    mdiArea->setActiveSubWindow(existing);
    return;
  }

  MdiChild *child = createMdiChild();
  if (child->loadFile(fileName)) {
    statusBar()->showMessage(tr("File loaded"), 2000);
    child->show();
  }
	
}

 void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list"))
	{
    event->acceptProposedAction();
	}
}

void MainWindow::releaseNotes()
{
	int *ptr ;
	
	QString rnotes =
	"Release files will now be found at: http://er9x.com.\n"
	"Googlecode is closing down. This project will move to Github.\n"
	"It may be found at: https://github.com/MikeBland/mbtx in due course.\n"
	"The voice module may now have a serial connection. This allows for model\n"
	"BACKUP to and RESTORE from the SD card on the voice module.\n"
	"The model version is now 4. The timer triggers have been changed, but Timer 2 now\n"
	"also has the same trigger options as Timer 1. Both also have a RESET switch field that may be set.\n"
	"Googlecode has blocked downloads of .exe files\n"
	"Windows users will now find eepe updates are in a .zip file\n\n"
  "er9x rev 811 changes the Custom Switch options.\n"
	"v1>=v2 and v1<=v2 are removed and replaced by Latch and F-Flop\n"
	"Loading a model into er9x from before will change:\n"
	"v1>=v2 into v1>v2 and change v1<=v2 into v1<v2\n"
	"To preserve the original functionality exactly you will need\n"
	"to change the new v1>v2 into v1<v2 and use the inverse of the switch,\n"
	"and change the new v1<v2 into v1>v2 and use the inverse of the switch.\n\n"
	"Latch takes two switches as inputs, when the first is ON, the Latch is set ON,\n"
	"when the second is on the Latch is cleared OFF. Setting takes priority if both\n"
	"switches are ON.\n"
	"F-Flop also takes two switches as inputs, the first is a 'clock' and the\n"
	"second is 'data'. When the first switch changes from OFF to ON, and stays\n"
	"ON for at least 0.1 seconds, the F-Flop is set to the same state as the 'data'.\n\n"
	"The model version is now 3, a button is available in the SETUP tab to convert\n"
	"models within eepe.\n\n"
	"The mixer function 'FlightMode Trim' has been REMOVED.\n"
	"Please now use a 'real' flight mode for this function.\n"
	"The following specific items have also been removed: 'Alt Alarm', 'mAh Alarm' and 'Volt Thres'.\n"
	"They can all be implemented using a custom switch, and a safety switch set either as an Audio\n"
	"switch or as a Voice switch."
	 ;
	
	reviewOutput *rO = new reviewOutput(this);
	ReleaseChecked = false ;
	ptr = &ReleaseChecked ;
	if ( currentEEPErev == currentEEPErelease )
	{
		ptr = 0 ;
	}
  rO->showCheck( ptr, "Release Notes", rnotes ) ;
  rO->exec() ;
  delete rO ;
}


void MainWindow::checkForUpdates(bool ignoreSettings)
{
    showcheckForUpdatesResult = ignoreSettings;
    check1done = true;
    check2done = true;

    QNetworkProxyFactory::setUseSystemConfiguration(true);

    if(checkER9X || ignoreSettings)
    {
        manager1 = new QNetworkAccessManager(this);
        connect(manager1, SIGNAL(finished(QNetworkReply*)),this, SLOT(reply1Finished(QNetworkReply*)));
        //manager1->get(QNetworkRequest(QUrl(ER9X_STAMP)));
        QNetworkRequest request(QUrl(GITHUB_REVS_URL));
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
        manager1->get(request);
        check1done = false;
    }

    if(checkEEPE || ignoreSettings)
    {
        manager2 = new QNetworkAccessManager(this);
        connect(manager2, SIGNAL(finished(QNetworkReply*)),this, SLOT(reply2Finished(QNetworkReply*)));
        //manager2->get(QNetworkRequest(QUrl(EEPE_STAMP)));
		    QNetworkRequest request(QUrl(GITHUB_REVS_URL));
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
        manager2->get(request);
        check2done = false;
    }

    if(downloadDialog_forWait!=0)
        downloadDialog_forWait = 0;

    if(ignoreSettings)
    {
        downloadDialog_forWait = new downloadDialog(this, tr("Checking for updates"));
        downloadDialog_forWait->show();
    }
}


void MainWindow::reply1Finished(QNetworkReply * reply)
{
    check1done = true;
    if(check1done && check2done && downloadDialog_forWait)
        downloadDialog_forWait->close();

    QByteArray qba = reply->readAll();
    int i = qba.indexOf("er9x");

    if(i>0)
    {
        QSettings settings("er9x-eePe", "eePe");
        QByteArray qbb = qba.mid(i+4,20) ;
        int j = qbb.indexOf("-r");
        bool cres;
        int rev = QString::fromLatin1(qbb.mid(j+2,4)).replace(QChar('"'), "").toInt(&cres);

        if(!cres)
        {
            QMessageBox::warning(this, "eePe", tr("Unable to check for updates."));
            return;
        }

        if(rev>currentER9Xrev)
        {

            QString dnldURL, baseFileName;
            switch (settings.value("download-version", 0).toInt())
            {
            case (DNLD_VER_ER9X_JETI):
                dnldURL = ER9X_JETI_URL;
                baseFileName = "er9x-jeti.hex";
                break;
            case (DNLD_VER_ER9X_FRSKY):
            case (DNLD_VER_ER9X_FRSKY_NOHT):
                dnldURL = ER9X_FRSKY_URL;
                baseFileName = "er9x-frsky.hex";
                break;
            case (DNLD_VER_ER9X_FRSKY_SV):
                dnldURL = ER9X_FRSKY_SV_URL;
                baseFileName = "er9x-frsky-sv.hex";
                break;
            case (DNLD_VER_ER9X_ARDUPILOT):
                dnldURL = ER9X_ARDUPILOT_URL;
                baseFileName = "er9x-ardupilot.hex";
                break;
            case (DNLD_VER_ER9X_NMEA):
                dnldURL = ER9X_NMEA_URL;
                baseFileName = "er9x-nmea.hex";
                break;
				    case (DNLD_VER_ER9X_128):
				    case (DNLD_VER_ER9X_S128):
        				dnldURL = ER9X_128_URL;
        				baseFileName = "er9x-128.hex";
        				break;
/*
            case (DNLD_VER_ER9X_SPKR):
                dnldURL = ER9X_SPKR_URL;
                baseFileName = "er9x-spkr.hex";
                break;
            case (DNLD_VER_ER9X_NOHT_SPKR):
                dnldURL = ER9X_NOHT_SPKR_URL;
                baseFileName = "er9x-noht-spkr.hex";
                break;
            case (DNLD_VER_ER9X_FRSKY_SPKR):
                dnldURL = ER9X_FRSKY_SPKR_URL;
                baseFileName = "er9x-frsky-spkr.hex";
                break;
            case (DNLD_VER_ER9X_FRSKY_NOHT_SPKR):
                dnldURL = ER9X_FRSKY_NOHT_SPKR_URL;
                baseFileName = "er9x-frsky-noht-spkr.hex";
                break;
*/
				    case (DNLD_VER_ER9X_DE):
        				dnldURL = ER9X_DE_URL;
        				baseFileName = "er9x-de.hex";
        				break;

				    case (DNLD_VER_ER9X_NO):
        				dnldURL = ER9X_NO_URL;
        				baseFileName = "er9x-no.hex";
        				break;

				    case (DNLD_VER_ER9X_FRSKY_DE):
        				dnldURL = ER9X_FRSKY_DE_URL;
        				baseFileName = "er9x-frsky-de.hex";
        				break;

				    case (DNLD_VER_ER9X_128_DE):
        				dnldURL = ER9X_128_DE_URL;
        				baseFileName = "er9x-128-de.hex";
        				break;

				    case (DNLD_VER_ER9X_2561):
        				dnldURL = ER9X_2561_URL;
        				baseFileName = "er9x-2561.hex";
        				break;

            case (DNLD_VER_ER9X_NOHT):
            default:
                dnldURL = ER9X_URL;
                baseFileName = "er9x.hex";
                break;
            }

            showcheckForUpdatesResult = false; // update is available - do not show dialog
            int ret = QMessageBox::question(this, "eePe",tr("A new version of ER9x (%2) is available (r%1)<br>"
                                                                "Would you like to download it?").arg(rev).arg(baseFileName) ,
                                            QMessageBox::Yes | QMessageBox::No);

            if (ret == QMessageBox::Yes)
            {
                QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),settings.value("lastDir").toString() + "/" + baseFileName,tr(HEX_FILES_FILTER));
                if (fileName.isEmpty()) return;
                settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

                downloadDialog * dd = new downloadDialog(this,dnldURL,fileName);
                currentER9Xrev_temp = rev;
                connect(dd,SIGNAL(accepted()),this,SLOT(reply1Accepted()));
                dd->show();
            }

            if(ret == QMessageBox::No)
            {
                int res = QMessageBox::question(this, "eePe",tr("Ignore this version (r%1)?").arg(rev) ,
                                                QMessageBox::Yes | QMessageBox::No);
                if(res == QMessageBox::Yes)
                {
                    currentER9Xrev = rev;
                    settings.setValue("currentER9Xrev", rev);
                }
            }
        }
        else
        {
            if(showcheckForUpdatesResult && check1done && check2done)
                QMessageBox::information(this, "eePe", tr("No updates available at this time."));
        }
    }
    else
    {
        if(check1done && check2done)
            QMessageBox::warning(this, "eePe", tr("Unable to check for updates."));
    }
}

void MainWindow::downloadLatester9x()
{

    QSettings settings("er9x-eePe", "eePe");

    QString dnldURL, baseFileName;
		int index = settings.value("download-version", 0).toInt() ;
    switch ( index )
    {
    case (DNLD_VER_ER9X_JETI):
        dnldURL = ER9X_JETI_URL;
        baseFileName = "er9x-jeti.hex";
        break;
    case (DNLD_VER_ER9X_FRSKY):
        dnldURL = ER9X_FRSKY_URL;
        baseFileName = "er9x-frsky.hex";
        break;
    case (DNLD_VER_ER9X_ARDUPILOT):
        dnldURL = ER9X_ARDUPILOT_URL;
        baseFileName = "er9x-ardupilot.hex";
        break;
    case (DNLD_VER_ER9X_FRSKY_NOHT):
        dnldURL = ER9X_FRSKY_NOHT_URL;
        baseFileName = "er9x-frsky-noht.hex";
        break;
		case (DNLD_VER_ER9X_NOHT):
        dnldURL = ER9X_NOHT_URL;
        baseFileName = "er9x-noht.hex";
        break;
		case (DNLD_VER_ER9X_NMEA):
        dnldURL = ER9X_NMEA_URL;
        baseFileName = "er9x-nmea.hex";
        break;
    case (DNLD_VER_ER9X_128):
    case (DNLD_VER_ER9X_S128):
        dnldURL = ER9X_128_URL;
        baseFileName = "er9x-128.hex";
        break;
		case (DNLD_VER_ER9X_DE):
        dnldURL = ER9X_DE_URL;
        baseFileName = "er9x-de.hex";
        break;
    case (DNLD_VER_ER9X_FRSKY_DE):
        dnldURL = ER9X_FRSKY_DE_URL;
        baseFileName = "er9x-frsky-de.hex";
        break;
    case (DNLD_VER_ER9X_FRSKY_SV):
        dnldURL = ER9X_FRSKY_SV_URL;
        baseFileName = "er9x-frsky-sv.hex";
    break;

    case (DNLD_VER_ER9X_2561):
        dnldURL = ER9X_2561_URL;
        baseFileName = "er9x-2561.hex";
        break;

 /*
    case (DNLD_VER_ER9X_SPKR):
        dnldURL = ER9X_SPKR_URL;
        baseFileName = "er9x-spkr.hex";
        break;
    case (DNLD_VER_ER9X_NOHT_SPKR):
        dnldURL = ER9X_NOHT_SPKR_URL;
        baseFileName = "er9x-noht-spkr.hex";
        break;
    case (DNLD_VER_ER9X_FRSKY_SPKR):
        dnldURL = ER9X_FRSKY_SPKR_URL;
        baseFileName = "er9x-frsky-spkr.hex";
        break;
    case (DNLD_VER_ER9X_FRSKY_NOHT_SPKR):
        dnldURL = ER9X_FRSKY_NOHT_SPKR_URL;
        baseFileName = "er9x-frsky-noht-spkr.hex";
        break;
 */
    default:
        dnldURL = ER9X_URL;
        baseFileName = "er9x.hex";
        break;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),settings.value("lastDir").toString() + "/" + baseFileName,tr(HEX_FILES_FILTER));
    if (fileName.isEmpty()) return;
    settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

    downloadDialog * dd = new downloadDialog(this,dnldURL,fileName);
    currentER9Xrev_temp = currentER9Xrev;
    connect(dd,SIGNAL(accepted()),this,SLOT(reply1Accepted()));
    dd->show();
}

void MainWindow::reply2Finished(QNetworkReply * reply)
{
    check2done = true;
    if(check1done && check2done && downloadDialog_forWait)
        downloadDialog_forWait->close();

    QByteArray qba = reply->readAll();
    int i = qba.indexOf("eepe");

    if(i>0)
    {
        QByteArray qbb = qba.mid(i+4,20) ;
        int j = qbb.indexOf("-r");
        bool cres;
        int rev = QString::fromLatin1(qbb.mid(j+2,4)).replace(QChar('"'), "").toInt(&cres);

        if(!cres)
        {
            QMessageBox::warning(this, "eePe", tr("Unable to check for updates."));
            return;
        }

        if(rev>currentEEPErev)
        {
            showcheckForUpdatesResult = false; // update is available - do not show dialog

#ifdef Q_OS_WIN32
            int ret = QMessageBox::question(this, "eePe", tr("A new version of eePe is available (r%1)<br>"
                                                                "Would you like to download it?").arg(rev) ,
                                            QMessageBox::Yes | QMessageBox::No);

            QSettings settings("er9x-eePe", "eePe");

            if (ret == QMessageBox::Yes)
            {
                QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),settings.value("lastDir").toString() + "/eePeInstall.zip",tr("Zip File (*.zip)"));
                if (fileName.isEmpty()) return;

                downloadDialog * dd = new downloadDialog(this,EEPE_URL,fileName);
                installer_fileName = fileName;
                connect(dd,SIGNAL(accepted()),this,SLOT(reply2Accepted()));
                dd->show();
            }           
#else
            QMessageBox::information(this, "eePe", tr("A new version of eePe is available (r%1)\n"
                                                      "To update please visit the eepe code page\n"
                                                      "http://code.google.com/p/eepe/").arg(rev) );
#endif
        }
        else
        {
            if(showcheckForUpdatesResult && check1done && check2done)
                QMessageBox::information(this, "eePe", tr("No updates available at this time."));
        }
    }
    else
    {
        if(check1done && check2done)
            QMessageBox::warning(this, "eePe", tr("Unable to check for updates."));
    }
}

void MainWindow::reply1Accepted()
{
    QSettings settings("er9x-eePe", "eePe");
    currentER9Xrev = currentER9Xrev_temp;
    settings.setValue("currentER9Xrev", currentER9Xrev);
}

void MainWindow::reply2Accepted()
{
    int ret2 = QMessageBox::question(this, "eePe",tr("Would you like to launch the installer?") ,
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret2 == QMessageBox::Yes)
    {
        if(QDesktopServices::openUrl(QUrl::fromLocalFile(installer_fileName)))
            QApplication::exit();
    }

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    mdiArea->closeAllSubWindows();
    if (mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
}

void MainWindow::newFile()
{
    MdiChild *child = createMdiChild();
    child->newFile();
    child->show();

    if(!child->parentWidget()->isMaximized() && !child->parentWidget()->isMinimized()) child->parentWidget()->resize(400,500);
}

void MainWindow::open()
{
    QSettings settings("er9x-eePe", "eePe");
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open"),settings.value("lastDir").toString(),tr(EEPROM_FILES_FILTER));
    if (!fileName.isEmpty())
    {
        settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

        QMdiSubWindow *existing = findMdiChild(fileName);
        if (existing) {
            mdiArea->setActiveSubWindow(existing);
            return;
        }

        MdiChild *child = createMdiChild();
        if (child->loadFile(fileName))
        {
            statusBar()->showMessage(tr("File loaded"), 2000);
            child->show();
            if(!child->parentWidget()->isMaximized() && !child->parentWidget()->isMinimized()) child->parentWidget()->resize(400,500);
        }
    }
}

void MainWindow::save()
{
    if (activeMdiChild() && activeMdiChild()->save())
        statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::saveAs()
{
    if (activeMdiChild() && activeMdiChild()->saveAs())
        statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::preferences()
{
    preferencesDialog *pd = new preferencesDialog(this);
    pd->exec();
    readSettings();
}

void MainWindow::serial()
{
    serialDialog *sd = new serialDialog(this);
    sd->exec();
}

void MainWindow::reviewOut()
{
    reviewOutput *rO = new reviewOutput(this);
    rO->exec();
    delete rO ;
}

void MainWindow::doTelemetry()
{
    telemetryDialog *td = new telemetryDialog(this);
    td->exec();
    delete td ;
}

void MainWindow::cut()
{
    if (activeMdiChild())
    {
        activeMdiChild()->cut();
        updateMenus();
    }
}

void MainWindow::copy()
{
    if (activeMdiChild())
    {
        activeMdiChild()->copy();
        updateMenus();
    }
}

void MainWindow::paste()
{
    if (activeMdiChild())
        activeMdiChild()->paste();
}

void MainWindow::burnTo()
{
    if (activeMdiChild())
        activeMdiChild()->burnTo();
}

void MainWindow::simulate()
{
    if (activeMdiChild())
        activeMdiChild()->simulate();
}


void MainWindow::print()
{
    if (activeMdiChild())
        activeMdiChild()->print();
}

void MainWindow::burnFrom()
{
    burnConfigDialog bcd;
    QString avrdudeLoc = bcd.getAVRDUDE();
    QString tempDir    = QDir::tempPath();
    QString programmer = bcd.getProgrammer();
    QString mcu        = bcd.getMCU();
    QStringList args   = bcd.getAVRArgs();
    if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

    if(!QFileInfo(avrdudeLoc).exists())
		{
      QMessageBox::critical(this, tr("Error"),
                   tr("Avrdude not found at %1.")
                   .arg(avrdudeLoc));
			return ;
		}

    QString tempFile = tempDir + "/temp.hex";
    QString str = "eeprom:r:" + tempFile + ":i"; // writing eeprom -> MEM:OPR:FILE:FTYPE"

    QStringList arguments;
    arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;

    avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments,tr("Read EEPROM From Tx")); //, AVR_DIALOG_KEEP_OPEN);
    ad->setWindowIcon(QIcon(":/images/read_eeprom.png"));
    int res = ad->exec();

    if(QFileInfo(tempFile).exists() && res)
    {
        MdiChild *child = createMdiChild();
        child->newFile();
        if(!child->loadFile(tempFile,false))
        {
            child->close();
            return;
        }

        child->setModified();
        child->show();
        if(!child->parentWidget()->isMaximized() && !child->parentWidget()->isMinimized()) child->parentWidget()->resize(400,500);
    }
}


// Consider looking for this sort of error

//avrdude.exe: Device signature = 0x1e9702
//avrdude.exe: Expected signature for ATMEGA64 is 1E 96 02
//             Double check chip, or use -F to override this check.
//
// And prompting for correct device to be set


int MainWindow::backupEeprom()
{
    burnConfigDialog bcd;
    QString avrdudeLoc = bcd.getAVRDUDE();
    QString tempDir    = QDir::tempPath();
    QString programmer = bcd.getProgrammer();
    QString mcu        = bcd.getMCU();
    QStringList args   = bcd.getAVRArgs();
    if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

    if(!QFileInfo(avrdudeLoc).exists())
		{
      QMessageBox::critical(this, tr("Error"),
                   tr("Avrdude not found at %1.")
                   .arg(avrdudeLoc));
			return 1 ;
		}

    QString tempFile = tempDir + "/eebackup.hex";
    QString str = "eeprom:r:" + tempFile + ":i"; // writing eeprom -> MEM:OPR:FILE:FTYPE"

    QStringList arguments;
    arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;

    avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments,tr("Backing up EEPROM From Tx")); //, AVR_DIALOG_KEEP_OPEN);
    ad->setWindowIcon(QIcon(":/images/read_eeprom.png"));
    int res = ad->exec();
//    ad->show();
//		ad->waitForFinish() ;
    if(QFileInfo(tempFile).exists() && res)
		{
			return 0 ;
		}
		return 1 ;
}


void MainWindow::burnExtenalToEEPROM()
{
    QSettings settings("er9x-eePe", "eePe");
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose file to write to EEPROM memory"),settings.value("lastDir").toString(),tr(EXTERNAL_EEPROM_FILES_FILTER));
    if (!fileName.isEmpty())
    {
        settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

        int ret = QMessageBox::question(this, "eePe", tr("Write %1 to EEPROM memory?").arg(QFileInfo(fileName).fileName()), QMessageBox::Yes | QMessageBox::No);
        if(ret!=QMessageBox::Yes) return;

        burnConfigDialog bcd;
        QString avrdudeLoc = bcd.getAVRDUDE();
        QString programmer = bcd.getProgrammer();
        QString mcu        = bcd.getMCU();
        QStringList args   = bcd.getAVRArgs();
				
				if ( args.contains("-F") )
				{
    			QMessageBox::StandardButton ret = QMessageBox::question(this, tr("eePe"),
                 tr("Only use -F if ABSOLUTELY sure. Continue?"),
                 QMessageBox::Yes | QMessageBox::No);
    			if (ret != QMessageBox::Yes)
					{
						return ;
					}
				}
				
        if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

    		if(!QFileInfo(avrdudeLoc).exists())
				{
    		  QMessageBox::critical(this, tr("Error"),
    		               tr("Avrdude not found at %1.")
    		               .arg(avrdudeLoc));
					return ;
				}

        QString str = "eeprom:w:" + fileName; // writing eeprom -> MEM:OPR:FILE:FTYPE"
        if(QFileInfo(fileName).suffix().toUpper()=="HEX") str += ":i";
        else if(QFileInfo(fileName).suffix().toUpper()=="BIN") str += ":r";
        else str += ":a";

        QStringList arguments;
        arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;

        avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments, "Write EEPROM To Tx", AVR_DIALOG_SHOW_DONE);
        ad->setWindowIcon(QIcon(":/images/write_eeprom.png"));
        ad->show();
    }
}

void MainWindow::burnToFlash(QString fileToFlash)
{
    QSettings settings("er9x-eePe", "eePe");
    QString fileName;
    if(fileToFlash.isEmpty())
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to write to flash memory"),settings.value("lastDir").toString(),tr(FLASH_FILES_FILTER));
    else
        fileName = fileToFlash;

    if (!fileName.isEmpty())
    {
        settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

        int ret = QMessageBox::question(this, "eePe", tr("Write %1 to flash memory?").arg(QFileInfo(fileName).fileName()), QMessageBox::Yes | QMessageBox::No);
        if(ret!=QMessageBox::Yes) return;
    
        burnConfigDialog bcd;
        QString avrdudeLoc = bcd.getAVRDUDE();
        QString programmer = bcd.getProgrammer();
        QString mcu        = bcd.getMCU();
        QStringList args   = bcd.getAVRArgs();
				
				if ( args.contains("-F") )
				{
    			QMessageBox::StandardButton ret = QMessageBox::question(this, tr("eePe"),
                 tr("Only use -F if ABSOLUTELY sure. Continue?"),
                 QMessageBox::Yes | QMessageBox::No);
    			if (ret != QMessageBox::Yes)
					{
						return ;
					}
				}
				
				bool disablePreRead = bcd.getPreRead() ;

    		if(!QFileInfo(avrdudeLoc).exists())
				{
    		  QMessageBox::critical(this, tr("Error"),
    		               tr("Avrdude not found at %1.")
    		               .arg(avrdudeLoc));
					return ;
				}

				if ( !disablePreRead )
				{
					if ( mcu != "m328p" )
					{
						ret = backupEeprom() ;

						if ( ret )
						{
        		  QMessageBox::warning(this, "eePe", tr("Backup failed, abandoning flash operation") ) ;
							return ;
						}

						// delay a bit to allow hardware to settle
		    		QTime dieTime= QTime::currentTime().addSecs(1);
		    		while( QTime::currentTime() < dieTime )
						{
			  		  QCoreApplication::processEvents(QEventLoop::AllEvents, 100);    
						}
					}
				}


//        ret = QMessageBox::question(this, "eePe", tr("Preserve installed spash screen"), QMessageBox::Yes | QMessageBox::No);
//        if(ret==QMessageBox::Yes)
//        {

//            QString tempDir    = QDir::tempPath();
//            QString tempFileOld = tempDir + "/temp.hex";
//            QString tempFileNew = tempDir + "/tempNew.hex";


//            if(QFile::remove(tempFileOld) && QFile::remove(tempFileNew))
//            {
//                if(QFile::copy(fileName, tempFileNew)) //copy new hex to temp
//                {
//                    //get HEX file from tx to temp folder
//                    burnConfigDialog bcd;
//                    QString avrdudeLoc = bcd.getAVRDUDE();
//                    QString programmer = bcd.getProgrammer();
//                    QString mcu        = bcd.getMCU();
//                    QStringList args   = bcd.getAVRArgs();
//                    if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

//                    QString str = "flash:r:" + tempFileOld + ":i";
//                    QStringList arguments;
//                    arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;

//                    avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments, "Read Flash From Tx", AVR_DIALOG_FORCE_CLOSE);
//                    ad->setWindowIcon(QIcon(":/images/read_flash.png"));
//                    ad->exec();


//                    uchar b[SPLASH_SIZE] = {0};
//                    if(getSplashHEX(tempFileOld, (uchar *)&b, this)) //get screen from hex
//                        if(putSplashHEX(tempFileNew, (uchar *)b, this)) //put screen to new hex
//                            fileName = tempFileNew; //make sure we burn the new version.
//                }
//            }
//        }

        if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

        QString str = "flash:w:" + fileName; // writing eeprom -> MEM:OPR:FILE:FTYPE"
        if(QFileInfo(fileName).suffix().toUpper()=="HEX") str += ":i";
        else if(QFileInfo(fileName).suffix().toUpper()=="BIN") str += ":r";
        else str += ":a";

        QStringList arguments;
        arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;

        avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments, "Write Flash To Tx", AVR_DIALOG_SHOW_DONE);
        ad->setWindowIcon(QIcon(":/images/write_flash.png"));
        ad->show();
    }
}


void MainWindow::burnExtenalFromEEPROM()
{
    QSettings settings("er9x-eePe", "eePe");
    QString fileName = QFileDialog::getSaveFileName(this,tr("Read EEPROM memory to File"),settings.value("lastDir").toString(),tr(EXTERNAL_EEPROM_FILES_FILTER));
    if (!fileName.isEmpty())
    {
        settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

        burnConfigDialog bcd;
        QString avrdudeLoc = bcd.getAVRDUDE();
        QString programmer = bcd.getProgrammer();
        QString mcu        = bcd.getMCU();
        QStringList args   = bcd.getAVRArgs();
        if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

    		if(!QFileInfo(avrdudeLoc).exists())
				{
    		  QMessageBox::critical(this, tr("Error"),
    		               tr("Avrdude not found at %1.")
    		               .arg(avrdudeLoc));
					return ;
				}

        QString str = "eeprom:r:" + fileName;
        if(QFileInfo(fileName).suffix().toUpper()=="HEX") str += ":i";
        else if(QFileInfo(fileName).suffix().toUpper()=="BIN") str += ":r";
        else str += ":a";

        QStringList arguments;
        arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;

        avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments, "Read EEPROM From Tx");
        ad->setWindowIcon(QIcon(":/images/read_eeprom.png"));
        ad->show();
    }

}


void MainWindow::burnFromFlash()
{

    QSettings settings("er9x-eePe", "eePe");
    QString fileName = QFileDialog::getSaveFileName(this,tr("Read Flash to File"),settings.value("lastDir").toString(),tr(FLASH_FILES_FILTER));

    if (!fileName.isEmpty())
    {
        settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

        burnConfigDialog bcd;
        QString avrdudeLoc = bcd.getAVRDUDE();
        QString programmer = bcd.getProgrammer();
        QString mcu        = bcd.getMCU();
        QStringList args   = bcd.getAVRArgs();
        if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

    		if(!QFileInfo(avrdudeLoc).exists())
				{
    		  QMessageBox::critical(this, tr("Error"),
    		               tr("Avrdude not found at %1.")
    		               .arg(avrdudeLoc));
					return ;
				}

        QString str = "flash:r:" + fileName; // writing eeprom -> MEM:OPR:FILE:FTYPE"
        if(QFileInfo(fileName).suffix().toUpper()=="HEX") str += ":i";
        else if(QFileInfo(fileName).suffix().toUpper()=="BIN") str += ":r";
        else str += ":a";

        QStringList arguments;
        arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;

        avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments, "Read Flash From Tx");
        ad->setWindowIcon(QIcon(":/images/read_flash.png"));
        ad->show();
    }

}

void MainWindow::burnConfig()
{
    burnConfigDialog *bcd = new burnConfigDialog(this);
    bcd->exec();
}

void MainWindow::burnList()
{
    burnConfigDialog *bcd = new burnConfigDialog(this);
    bcd->listProgrammers();
}

void MainWindow::setFuses()
{
    burnConfigDialog *bcd = new burnConfigDialog(this);
    bcd->restFuses(true);
}

void MainWindow::resetFuses()
{
    burnConfigDialog *bcd = new burnConfigDialog(this);
    bcd->restFuses(false);
}

void MainWindow::readFuses()
{
    burnConfigDialog *bcd = new burnConfigDialog(this);
    bcd->readFuses();
}

void MainWindow::showEEPROMInfo()
{
    //show info about:
    // eeprom size
    // available models
    // model sizes
    // EEPROM version
    // Free space

	int eesize ;
	int free ;

	eesize = activeMdiChild()->eesize() ;
  free = activeMdiChild()->free() ;

    if(activeMdiChild() == 0)
        return;

    QString msg = "<table cellspacing=0 cellpadding=0 border=0>";
    msg.append("<tr><td colspan=2><u><b>");
    msg.append(QString("%1").arg(QFileInfo(activeMdiChild()->currentFile()).fileName()));
    msg.append("</b></u></td></tr>");

    int modelSizes[MAX_MODELS+1] = {0};
    int totalSize = 0;

    for(int i=0; i<=MAX_MODELS; i++)
    {
        modelSizes[i] = activeMdiChild()->modelSize(i);
        totalSize += modelSizes[i];
    }

    msg.append(tr("<tr><td>Owner: </td><td align=right>%1</td></tr>").arg(activeMdiChild()->ownerName()));
    msg.append(tr("<tr><td>Version: </td><td align=right>%1</td></tr>").arg(activeMdiChild()->eepromVersion()));

    msg.append(tr("<tr><td>Bytes Used: </td><td align=right>%1</td></tr>").arg(totalSize));
    msg.append(tr("<tr><td>Bytes Free: </td><td align=right>%1</td></tr>").arg(free));
    msg.append(tr("<tr><td>Bytes Total:</td><td align=right> %1</td></tr>").arg(eesize));

    msg.append("<tr><td colspan=2><br><u><b>");
    msg.append(tr("Details:"));
    msg.append("</b></u></td></tr>");

    for(int i=0; i<=MAX_MODELS; i++)
    {
        if(modelSizes[i]>0)
        {
            if(i==0)
                msg.append(tr("<tr><td>Settings: </td><td align=right>%1 Bytes</td></tr>").arg(modelSizes[i]));
            else
                msg.append(tr("<tr><td>%2: </td><td align=right>%1 Bytes</td></tr>").arg(modelSizes[i]).arg(activeMdiChild()->modelName(i-1)));
        }
    }

    msg.append("</table>");


    QMessageBox::information(this, "eePe", msg);

}

void MainWindow::donators()
{
    donatorsDialog *dd = new donatorsDialog(this);
    dd->exec();
}

void MainWindow::showEr9xManual()
{
//    ER9x Users Guide.pdf
    QString cdir = QApplication::applicationDirPath();
#ifdef Q_WS_WIN
    QDesktopServices::openUrl(QUrl::fromLocalFile(cdir + "/ER9x_Manual_2015-v01.pdf")); // WIN
#else
    QDesktopServices::openUrl(QUrl("file:///" + cdir + "/ER9x_Manual_2015-v01.pdf"));   // MAC & Linux (X11)
#endif
}

void MainWindow::loadModelFromFile()
{
    if(activeMdiChild())
        activeMdiChild()->loadModelFromFile();
}

void MainWindow::saveModelToFile()
{
    if(activeMdiChild())
        activeMdiChild()->saveModelToFile();
}

void MainWindow::customizeSplash()
{
    customizeSplashDialog *csd = new customizeSplashDialog(this);
    csd->show();
}

void MainWindow::about()
{
    QString aboutStr = "<center><img src=\":/images/eepe-title.png\"><br>";
    aboutStr.append(tr("Copyright") +" Mike Blandford &copy;2012<br>");
    aboutStr.append(QString("<a href='https://github.com/MikeBland/mbtx'>https://github.com/MikeBland/mbtx/</a><br>Revision: p%1, %2<br><br>").arg(currentEEPErev).arg(__DATE__));
    aboutStr.append(tr("If you've found this program and/or the er9x firmware useful please support by donating<br>"));
		aboutStr.append(" <a href='" DONATE_MB_STR "'>");
    aboutStr.append(tr("to Mike Blandford (current maintainer)") + "</a><br>");
		aboutStr.append(" <a href='" DONATE_ER_STR "'>");
    aboutStr.append(tr("to Erez Raviv (original author)") + "</a></center><br>");


//    aboutStr.append(tr("geegeneral size = %1").arg(sizeof(EEGeneral)));

    QMessageBox::about(this, tr("About eePe"),aboutStr);
}

void MainWindow::updateMenus()
{
    bool hasMdiChild = (activeMdiChild() != 0);
    saveAct->setEnabled(hasMdiChild);
    saveAsAct->setEnabled(hasMdiChild);
    eepromInfoAct->setEnabled(hasMdiChild);
    pasteAct->setEnabled(hasMdiChild ? activeMdiChild()->hasPasteData() : false);
    closeAct->setEnabled(hasMdiChild);
    closeAllAct->setEnabled(hasMdiChild);
    tileAct->setEnabled(hasMdiChild);
    cascadeAct->setEnabled(hasMdiChild);
    nextAct->setEnabled(hasMdiChild);
    previousAct->setEnabled(hasMdiChild);
    burnToAct->setEnabled(hasMdiChild);
    separatorAct->setVisible(hasMdiChild);
    saveModelToFileAct->setEnabled(hasMdiChild ? activeMdiChild()->saveToFileEnabled() : false);
    loadModelFromFileAct->setEnabled(hasMdiChild);

    bool hasSelection = (activeMdiChild() &&
                         activeMdiChild()->hasSelection());
    cutAct->setEnabled(hasSelection);
    copyAct->setEnabled(hasSelection);
}

void MainWindow::updateWindowMenu()
{
    windowMenu->clear();
    windowMenu->addAction(closeAct);
    windowMenu->addAction(closeAllAct);
    windowMenu->addSeparator();
    windowMenu->addAction(tileAct);
    windowMenu->addAction(cascadeAct);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAct);
    windowMenu->addAction(previousAct);
    windowMenu->addAction(separatorAct);

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    separatorAct->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i) {
        MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());

        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                               .arg(child->userFriendlyCurrentFile());
        } else {
            text = tr("%1 %2").arg(i + 1)
                              .arg(child->userFriendlyCurrentFile());
        }
        QAction *action  = windowMenu->addAction(text);
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action, windows.at(i));
    }
}

MdiChild *MainWindow::createMdiChild()
{
    MdiChild *child = new MdiChild;
    mdiArea->addSubWindow(child);

    connect(child, SIGNAL(copyAvailable(bool)),cutAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(copyAvailable(bool)),copyAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(copyAvailable(bool)),simulateAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(copyAvailable(bool)),printAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(saveModelToFileAvailable(bool)),saveModelToFileAct, SLOT(setEnabled(bool)));

    return child;
}

void MainWindow::createActions()
{
    newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    preferencesAct = new QAction(QIcon(":/images/preferences.png"), tr("&Preferences..."), this);
    preferencesAct->setStatusTip(tr("Edit general preferences"));
    connect(preferencesAct, SIGNAL(triggered()), this, SLOT(preferences()));

    checkForUpdatesAct = new QAction(QIcon(":/images/update.png"), tr("&Check for updates..."), this);
    checkForUpdatesAct->setStatusTip(tr("Check for new version of eePe/er9x"));
    connect(checkForUpdatesAct, SIGNAL(triggered()), this, SLOT(checkForUpdates()));


		serialAct = new QAction(tr("Serial SD"), this) ;
    serialAct->setStatusTip(tr("Update Megasound SD card"));
    connect(serialAct, SIGNAL(triggered()), this, SLOT(serial()));

    reviewBurnOutput = new QAction(tr("review Output"), this) ;
    reviewBurnOutput->setStatusTip(tr("display last AvrDude output"));
    connect(reviewBurnOutput, SIGNAL(triggered()), this, SLOT(reviewOut()));


//! [0]
    exitAct = new QAction(QIcon(":/images/exit.png"), tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
//! [0]

    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));


    burnToAct = new QAction(QIcon(":/images/write_eeprom.png"), tr("&Write Memory To Tx"), this);
    burnToAct->setShortcut(tr("Ctrl+Alt+W"));
    burnToAct->setStatusTip(tr("Write EEPROM memory to transmitter"));
    connect(burnToAct,SIGNAL(triggered()),this,SLOT(burnTo()));

    burnFromAct = new QAction(QIcon(":/images/read_eeprom.png"), tr("&Read Memory From Tx"), this);
    burnFromAct->setShortcut(tr("Ctrl+Alt+R"));
    burnFromAct->setStatusTip(tr("Read EEPROM memory from transmitter"));
    connect(burnFromAct,SIGNAL(triggered()),this,SLOT(burnFrom()));

    burnToFlashAct = new QAction(QIcon(":/images/write_flash.png"), tr("Flash Firmware to Tx"), this);
    burnToFlashAct->setStatusTip(tr("Write flash firmware to transmitter"));
    connect(burnToFlashAct,SIGNAL(triggered()),this,SLOT(burnToFlash()));

    burnFromFlashAct = new QAction(QIcon(":/images/read_flash.png"), tr("Read Firmware from Tx"), this);
    burnFromFlashAct->setStatusTip(tr("Read flash memory from transmitter"));
    connect(burnFromFlashAct,SIGNAL(triggered()),this,SLOT(burnFromFlash()));

    burnExtenalToEEPROMAct = new QAction(QIcon(":/images/write_eeprom_file.png"), tr("Write EEPROM memory from file"), this);
    burnExtenalToEEPROMAct->setStatusTip(tr("Write EEPROM memory from file to transmitter"));
    connect(burnExtenalToEEPROMAct,SIGNAL(triggered()),this,SLOT(burnExtenalToEEPROM()));

    burnExtenalFromEEPROMAct = new QAction(QIcon(":/images/read_eeprom_file.png"), tr("Read EEPROM memory to file"), this);
    burnExtenalFromEEPROMAct->setStatusTip(tr("Read EEPROM memory from transmitter to file"));
    connect(burnExtenalFromEEPROMAct,SIGNAL(triggered()),this,SLOT(burnExtenalFromEEPROM()));

    burnConfigAct = new QAction(QIcon(":/images/configure.png"), tr("&Configure..."), this);
    burnConfigAct->setStatusTip(tr("Configure burning software"));
    connect(burnConfigAct,SIGNAL(triggered()),this,SLOT(burnConfig()));

    burnListAct = new QAction(QIcon(":/images/list.png"), tr("&List programmers"), this);
    burnListAct->setStatusTip(tr("List available programmers"));
    connect(burnListAct,SIGNAL(triggered()),this,SLOT(burnList()));

    simulateAct = new QAction(QIcon(":/images/simulate.png"), tr("&Simulate"), this);
    simulateAct->setShortcut(tr("Alt+S"));
    simulateAct->setStatusTip(tr("Simulate selected model."));
    simulateAct->setEnabled(false);
    connect(simulateAct,SIGNAL(triggered()),this,SLOT(simulate()));

    printAct = new QAction(QIcon(":/images/print.png"), tr("&Print"), this);
    printAct->setShortcut(tr("Ctrl+P"));
    printAct->setStatusTip(tr("Print current model."));
    printAct->setEnabled(false);
    connect(printAct,SIGNAL(triggered()),this,SLOT(print()));

    closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, SIGNAL(triggered()),
            mdiArea, SLOT(closeActiveSubWindow()));

    closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, SIGNAL(triggered()),
            mdiArea, SLOT(closeAllSubWindows()));

    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()));

    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, SIGNAL(triggered()), mdiArea, SLOT(cascadeSubWindows()));

    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcuts(QKeySequence::NextChild);
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, SIGNAL(triggered()),
            mdiArea, SLOT(activateNextSubWindow()));

    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcuts(QKeySequence::PreviousChild);
    previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));
    connect(previousAct, SIGNAL(triggered()),
            mdiArea, SLOT(activatePreviousSubWindow()));

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);

    aboutAct = new QAction(QIcon(":/eepe.png"), tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    releaseAct = new QAction(QIcon(":/eepe.png"), tr("&Release Notes"), this);
    releaseAct->setStatusTip(tr("Show release notes"));
    connect( releaseAct, SIGNAL(triggered()), this, SLOT(releaseNotes()));

    donatorsAct = new QAction(QIcon(":/images/contributors.png"), tr("&Contributors"), this);
    donatorsAct->setStatusTip(tr("List er9x/eePe Contributors"));
    connect(donatorsAct, SIGNAL(triggered()), this, SLOT(donators()));

    switchLayoutDirectionAct = new QAction(QIcon(":/images/switch_dir.png"),  tr("Switch layout direction"), this);
    switchLayoutDirectionAct->setStatusTip(tr("Switch layout Left/Right"));
    connect(switchLayoutDirectionAct, SIGNAL(triggered()), this, SLOT(switchLayoutDirection()));

    showEr9xManualAct = new QAction(QIcon(":/images/er9x_manual.png"), tr("&ER9x Users Guide"), this);
    showEr9xManualAct->setStatusTip(tr("Show ER9x Users Guide"));
    connect(showEr9xManualAct, SIGNAL(triggered()), this, SLOT(showEr9xManual()));


    loadModelFromFileAct = new QAction(QIcon(":/images/load_model.png"), tr("&Load Model/Settings"), this);
    loadModelFromFileAct->setStatusTip(tr("Load Model/Settings From File"));
    connect(loadModelFromFileAct, SIGNAL(triggered()), this, SLOT(loadModelFromFile()));

    saveModelToFileAct = new QAction(QIcon(":/images/save_model.png"), tr("S&ave Model/Settings"), this);
    saveModelToFileAct->setStatusTip(tr("Save Model/Settings To File"));
    connect(saveModelToFileAct, SIGNAL(triggered()), this, SLOT(saveModelToFile()));

    customizeSplashAct = new QAction(QIcon(":/images/c_home.png"), tr("Cu&stomize Splash Screen"), this);
    customizeSplashAct->setStatusTip(tr("Customize Splash Screen"));
    connect(customizeSplashAct, SIGNAL(triggered()), this, SLOT(customizeSplash()));

    setFusesAct = new QAction(QIcon(":/images/fuses_set.png"), tr("Set fuses to protect EEPROM"), this);
    setFusesAct->setStatusTip(tr("Sets the fuses to protect EEPROM from being erased."));
    connect(setFusesAct, SIGNAL(triggered()), this, SLOT(setFuses()));

    resetFusesAct = new QAction(QIcon(":/images/fuses_set.png"), tr("Reset fuses to factory default"), this);
    resetFusesAct->setStatusTip(tr("Resets the fuses to factory default - EEPROM erase."));
    connect(resetFusesAct, SIGNAL(triggered()), this, SLOT(resetFuses()));

    readFusesAct = new QAction( tr("Read Fuses"), this);
    connect(readFusesAct, SIGNAL(triggered()), this, SLOT(readFuses()));

    eepromInfoAct = new QAction(QIcon(":/images/info.png"), tr("EEPROM Info"), this);
    eepromInfoAct->setStatusTip(tr("Show information about current EEPROM."));
    connect(eepromInfoAct, SIGNAL(triggered()), this, SLOT(showEEPROMInfo()));

    telemetryAct = new QAction( tr("&Telemetry"), this);
//    telemetryAct->setShortcut(tr("Ctrl+Alt+T"));
//    telemetryAct->setStatusTip(tr("Write EEPROM memory to transmitter"));
    connect( telemetryAct,SIGNAL(triggered()),this,SLOT(doTelemetry()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(loadModelFromFileAct);
    fileMenu->addAction(saveModelToFileAct);
    fileMenu->addSeparator();
    fileMenu->addAction(simulateAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(preferencesAct);
    fileMenu->addAction(switchLayoutDirectionAct);
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addAction(eepromInfoAct);

    burnMenu = menuBar()->addMenu(tr("&Burn"));
    burnMenu->addAction(burnToAct);
    burnMenu->addAction(burnFromAct);
    burnMenu->addSeparator();
    burnMenu->addAction(burnToFlashAct);
    burnMenu->addAction(burnFromFlashAct);
    burnMenu->addSeparator();
    burnMenu->addAction(burnExtenalToEEPROMAct);
    burnMenu->addAction(burnExtenalFromEEPROMAct);
    burnMenu->addSeparator();
    burnMenu->addAction(burnConfigAct);
    burnMenu->addAction(burnListAct);
    burnMenu->addSeparator();
    burnMenu->addAction(setFusesAct);
    burnMenu->addAction(resetFusesAct);
    burnMenu->addAction(readFusesAct);
    burnMenu->addSeparator();
    burnMenu->addAction(reviewBurnOutput);
    burnMenu->addSeparator();
    burnMenu->addAction(serialAct);

    telemetryMenu = menuBar()->addMenu(tr("&Telemetry"));
    telemetryMenu->addAction(telemetryAct) ;

    windowMenu = menuBar()->addMenu(tr("&Window"));
    updateWindowMenu();
    connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(showEr9xManualAct);
    helpMenu->addSeparator();
    helpMenu->addAction(customizeSplashAct);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(releaseAct);
    helpMenu->addAction(donatorsAct);
    helpMenu->addSeparator();
    helpMenu->addAction(checkForUpdatesAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addSeparator();
    fileToolBar->addAction(simulateAct);
    fileToolBar->addAction(printAct);
    fileToolBar->addSeparator();
    fileToolBar->addAction(preferencesAct);
    fileToolBar->addSeparator();
    fileToolBar->addAction(loadModelFromFileAct);
    fileToolBar->addAction(saveModelToFileAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(eepromInfoAct);

    burnToolBar = addToolBar(tr("Burn"));
    burnToolBar->addAction(burnToAct);
    burnToolBar->addAction(burnFromAct);
//    burnToolBar->addSeparator();
//    burnToolBar->addAction(burnExtenalToEEPROMAct);
//    burnToolBar->addAction(burnExtenalFromEEPROMAct);
    burnToolBar->addSeparator();
    burnToolBar->addAction(burnToFlashAct);
    burnToolBar->addAction(burnFromFlashAct);
    burnToolBar->addSeparator();
    burnToolBar->addAction(burnConfigAct);
    burnToolBar->addAction(setFusesAct);

    helpToolBar = addToolBar(tr("Help"));
//    helpToolBar->addAction(showEr9xManualAct);
    helpToolBar->addAction(customizeSplashAct);
    helpToolBar->addAction(aboutAct);
//    helpToolBar->addAction(donatorsAct);
//    helpToolBar->addAction(checkForUpdatesAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings("er9x-eePe", "eePe");
    bool maximized = settings.value("maximized", false).toBool();
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();

    currentER9Xrev = settings.value("currentER9Xrev", 1).toInt();
    currentEEPErelease = settings.value("currentEEPErelease", 1).toInt();
    currentEEPErev = SVN_VER_NUM;
    processor = settings.value("processor", 1).toInt();

    checkER9X  = settings.value("startup_check_er9x", true).toBool();
    checkEEPE  = settings.value("startup_check_eepe", true).toBool();

    if(maximized)
    {
         setWindowState(Qt::WindowMaximized);
    }
    else
    {
        move(pos);
        resize(size);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings("er9x-eePe", "eePe");

    settings.setValue("maximized", isMaximized());
    if(!isMaximized())
    {
        settings.setValue("pos", pos());
        settings.setValue("size", size());
    }
}

MdiChild *MainWindow::activeMdiChild()
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
        return qobject_cast<MdiChild *>(activeSubWindow->widget());
    return 0;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        if (mdiChild->currentFile() == canonicalFilePath)
            return window;
    }
    return 0;
}

void MainWindow::switchLayoutDirection()
{
    if (layoutDirection() == Qt::LeftToRight)
        qApp->setLayoutDirection(Qt::RightToLeft);
    else
        qApp->setLayoutDirection(Qt::LeftToRight);
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if (!window)
        return;
    mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}
