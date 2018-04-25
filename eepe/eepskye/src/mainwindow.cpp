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
#include <QFileInfo>
#include <QSslConfiguration>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QApplication>
#include <QMdiArea>

#include "mainwindow.h"
#include "pers.h"
#include "myeeprom.h"
#include "file.h"
#include "mdichild.h"
#include "burnconfigdialog.h"
#include "avroutputdialog.h"
#include "donatorsdialog.h"
#include "preferencesdialog.h"
#include "downloaddialog.h"
#include "customizesplashdialog.h"
#include "stamp-eepskye.h"
#include "../../common/reviewOutput.h"
#include "helpers.h"
#include "../../common/telemetry.h"
#include "simulatordialog.h"

#if defined WIN32 || !defined __GNUC__
#include <windows.h>
#endif

//#include "comparedialog.h"
//#include "logsdialog.h"
//#include "flashinterface.h"
//#include "fusesdialog.h"
//#include "printdialog.h"
//#include "version.h"

//#include "burndialog.h"
//#include "hexinterface.h"
//#include "warnings.h"



#define DONATE_MB_STR "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=YHX43JR3J7XGW"
#define DNLD_VER_ERSKY9X         0
#define DNLD_VER_ERSKY9XR	       1
#define DNLD_VER_ERSKYX9D				 2
#define DNLD_VER_ERSKYX9DP			 3
#define DNLD_VER_ERSKYX9XT			 4
#define DNLD_VER_ERSKYX9E				 5
#define DNLD_VER_ERSKY9XA				 6
#define DNLD_VER_ERSKY9XQX7			 7

//#define DNLD_VER_ER9X_FRSKY      2
//#define DNLD_VER_ER9X_ARDUPILOT  3
//#define DNLD_VER_ER9X_FRSKY_NOHT 4
//#define DNLD_VER_ER9X_NOHT       5

//#define DNLD_VER_ER9X_SPKR            6
//#define DNLD_VER_ER9X_NOHT_SPKR       7
//#define DNLD_VER_ER9X_FRSKY_SPKR      8
//#define DNLD_VER_ER9X_FRSKY_NOHT_SPKR 9
//#define DNLD_VER_ER9X_NMEA            7

//#define ER9X_URL   "http://er9x.googlecode.com/svn/trunk/er9x.hex"
//#define ER9X_NOHT_URL   "http://er9x.googlecode.com/svn/trunk/er9x-noht.hex"
//#define ER9X_SPKR_URL   "http://er9x.googlecode.com/svn/trunk/er9x-spkr.hex"
//#define ER9X_NOHT_SPKR_URL   "http://er9x.googlecode.com/svn/trunk/er9x-noht-spkr.hex"
//#define ER9X_JETI_URL   "http://er9x.googlecode.com/svn/trunk/er9x-jeti.hex"
//#define ER9X_FRSKY_URL   "http://er9x.googlecode.com/svn/trunk/er9x-frsky.hex"
//#define ER9X_FRSKY_NOHT_URL   "http://er9x.googlecode.com/svn/trunk/er9x-frsky-noht.hex"
//#define ER9X_FRSKY_SPKR_URL   "http://er9x.googlecode.com/svn/trunk/er9x-frsky-spkr.hex"
//#define ER9X_FRSKY_NOHT_SPKR_URL   "http://er9x.googlecode.com/svn/trunk/er9x-frsky-noht-spkr.hex"
//#define ER9X_ARDUPILOT_URL   "http://er9x.googlecode.com/svn/trunk/er9x-ardupilot.hex"
//#define ER9X_NMEA_URL   "http://er9x.googlecode.com/svn/trunk/er9x-nmea.hex"
//#define ER9X_STAMP "http://er9x.googlecode.com/svn/trunk/src/stamp-er9x.h"
#define EEPE_URL   "http://www.er9x.com/eePeInstall.exe"
//#define EEPE_STAMP "http://eepe.googlecode.com/svn/trunk/src/stamp-eepe.h"
#define EEPSKYE_URL   "http://www.er9x.com/eePeInstall.exe"
#define EEPSKYE_STAMP "http://eepe.googlecode.com/svn/trunk/src/eepskye/src/stamp-eepskye.h"

#define ERSKY9X_STAMP "http://ersky9x.googlecode.com/svn/trunk/src/stamp-ersky9x.h"
#define ERSKY9X_URL "http://www.er9x.com/ersky9x_rom.bin"
#define ERSKY9XR_URL "http://www.er9x.com/ersky9xr_rom.bin"
#define ERSKYX9D_URL "http://www.er9x.com/x9d_rom.bin"
#define ERSKYX9DP_URL "http://www.er9x.com/x9dp_rom.bin"
#define ERSKYX9XT_URL "http://www.er9x.com/ersky9x9XT_rom.bin"
#define ERSKYX9E_URL "http://www.er9x.com/x9e_rom.bin"
#define ERSKY9XRA_URL "http://www.er9x.com/ersky9x_rom.bin"
#define ERSKY9XQX7_URL "http://www.er9x.com/x7_rom.bin"

#define GITHUB_REVS_URL	"http://www.er9x.com/Revisions.txt"

class simulatorDialog *SimPointer = 0 ;
QString AvrdudeOutput ;

int ReleaseChecked ;

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
		title() ;
    
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

		if ( currentEEPSKYErev > currentEEPSKYErelease )
		{
			releaseNotes() ;
			if ( ReleaseChecked )
			{
				currentEEPSKYErelease = currentEEPSKYErev ;
    		QSettings settings("er9x-eePskye", "eePskye");
		    settings.setValue("currentEEPSKYErelease", currentEEPSKYErelease ) ;
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

void MainWindow::title()
{
  QSettings settings("er9x-eePskye", "eePskye");
	int dnloadVersion = settings.value("download-version", 0).toInt() ;
	QString type ;
	switch ( dnloadVersion )
	{
		default :
			type = "Sky" ;
    break ;
		case 1 :
			type = "9XR-PRO" ;
    break ;
		case 2 :
			type = "Taranis" ;
    break ;
		case 3 :
			type = "Taranis Plus" ;
    break ;
		case 4 :
			type = "9Xtreme" ;
    break ;
		case 5 :
			type = "X9E" ;
    break ;
		case 6 :
			type = "AR9X" ;
    break ;
		case 7 :
			type = "QX7" ;
    break ;
	}
  setWindowTitle(tr("eePskye - EEPROM Editor - %1").arg(type));
}


void MainWindow::releaseNotes()
{
	int *ptr ;
	
	
	QString rnotes =
	"Release files will now be found at: http://er9x.com.\n"
	"Googlecode has closed down. This project has moved to Github.\n"
	"It may be found at: https://github.com/MikeBland/mbtx \n"
	"Googlecode has blocked downloads of .exe files\n"
	"Windows users will now find eepskye updates are in a .zip file\n\n"
  "From ersky9x rev 203 changes to the Custom Switch options.\n"
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
	"models within eepe."
	 ;
	
	reviewOutput *rO = new reviewOutput(this);
	ReleaseChecked = false ;
	ptr = &ReleaseChecked ;
	if ( currentEEPSKYErev == currentEEPSKYErelease )
	{
		ptr = 0 ;
	}
  rO->showCheck( ptr, "Release Notes", rnotes ) ;
  rO->exec() ;
  delete rO ;
}


//QSslConfiguration QSSLconfig ;

void MainWindow::checkForUpdates(bool ignoreSettings)
{
    showcheckForUpdatesResult = ignoreSettings;
    check1done = true;
    check2done = true;
		checkGdone = true ;
    readSettings() ;

//    QNetworkProxyFactory::setUseSystemConfiguration(true);

    if(checkERSKY9X || ignoreSettings)
    {
        manager1 = new QNetworkAccessManager(this);
        connect(manager1, SIGNAL(finished(QNetworkReply*)),this, SLOT(reply1Finished(QNetworkReply*)));
//        manager1->get(QNetworkRequest(QUrl(ERSKY9X_STAMP)));
        QNetworkRequest request(QUrl(GITHUB_REVS_URL));
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
        manager1->get(request);
        check1done = false;
    }

    if(checkEEPSKYE || ignoreSettings)
    {
        manager2 = new QNetworkAccessManager(this);
        connect(manager2, SIGNAL(finished(QNetworkReply*)),this, SLOT(reply2Finished(QNetworkReply*)));
        //manager2->get(QNetworkRequest(QUrl(EEPSKYE_STAMP)));
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
    if(check1done && check2done && checkGdone && downloadDialog_forWait)
        downloadDialog_forWait->close();

    QByteArray qba = reply->readAll();
    int i = qba.indexOf("ersky9x");

    if(i>0)
    {
    		QSettings settings("er9x-eePskye", "eePskye");
        QByteArray qbb = qba.mid(i+6,20) ;
        int j = qbb.indexOf("-r");
        bool cres;
        int rev = QString::fromLatin1(qbb.mid(j+2,4)).replace(QChar('"'), "").toInt(&cres);

        if(!cres)
        {
            QMessageBox::warning(this, "ersky9x", tr("Unable to check for updates."));
            return;
        }
        
				int currentRev = currentERSKY9Xrev ;
        switch (settings.value("download-version", 0).toInt())
				{
					case 1 :
						currentRev = currentERSKY9XRrev ;
					break ;
					case 2 :
						currentRev = currentERSKYX9Drev ;
					break ;
					case 3 :
						currentRev = currentERSKYX9DPrev ;
					break ;
          case 4 :
						currentRev = currentERSKY9XTrev ;
					break ;
          case 5 :
						currentRev = currentERSKYX9Erev ;
					break ;
          case 6 :
						currentRev = currentERSKY9Xrev ;
					break ;
					case 7 :
						currentRev = currentERSKY9XQX7rev ;
					break ;

				}

        if(rev>currentRev)
        {

            QString dnldURL, baseFileName;
            switch (settings.value("download-version", 0).toInt())
            {
            case (DNLD_VER_ERSKY9XR):
                dnldURL = ERSKY9XR_URL;
                baseFileName = "ersky9xr_rom.bin";
                break;

            case (DNLD_VER_ERSKYX9D):
                dnldURL = ERSKYX9D_URL;
                baseFileName = "x9d_rom.bin";
                break;

            case (DNLD_VER_ERSKYX9DP):
                dnldURL = ERSKYX9DP_URL;
                baseFileName = "x9dp_rom.bin";
                break;

            case (DNLD_VER_ERSKYX9XT):
                dnldURL = ERSKYX9XT_URL;
                baseFileName = "ersky9x9XT_rom.bin";
                break;
            
            case (DNLD_VER_ERSKYX9E):
                dnldURL = ERSKYX9E_URL;
                baseFileName = "x9e_rom.bin";
                break;

						case (DNLD_VER_ERSKY9XA):
                dnldURL = ERSKY9XQX7_URL;
                baseFileName = "x7_rom.bin";
                break;

						case (DNLD_VER_ERSKY9XQX7):
                dnldURL = ERSKY9X_URL;
                baseFileName = "ersky9x_rom.bin";
                break;
								
            default:
                dnldURL = ERSKY9X_URL;
                baseFileName = "ersky9x_rom.bin";
                break;
            }

            showcheckForUpdatesResult = false; // update is available - do not show dialog
            int ret = QMessageBox::question(this, "eePskye",tr("A new version of ERSKY9x (%2) is available (r%1)<br>"
                                                                "Would you like to download it?").arg(rev).arg(baseFileName) ,
                                            QMessageBox::Yes | QMessageBox::No);

            if (ret == QMessageBox::Yes)
            {
                QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),settings.value("lastDir").toString() + "/" + baseFileName,tr(HEX_FILES_FILTER));
                if (fileName.isEmpty()) return;
                settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

                downloadDialog * dd = new downloadDialog(this,dnldURL,fileName);
                currentERSKY9Xrev_temp = rev;
                connect(dd,SIGNAL(accepted()),this,SLOT(reply1Accepted()));
                dd->show();
            }

            if(ret == QMessageBox::No)
            {
                int res = QMessageBox::question(this, "eePskye",tr("Ignore this version (r%1)?").arg(rev) ,
                                                QMessageBox::Yes | QMessageBox::No);
                if(res == QMessageBox::Yes)
                {
                    currentERSKY9Xrev = rev;
				            switch (settings.value("download-version", 0).toInt())
										{
											case 0 :
                    		settings.setValue("currentERSKY9Xrev", rev);
											break ;

											case 1 :
                    		settings.setValue("currentERSKY9XRrev", rev);
											break ;
											
											case 2 :
                    		settings.setValue("currentERSKYX9Drev", rev);
											break ;
											
											case 3 :
                    		settings.setValue("currentERSKYX9DPrev", rev);
											break ;
											
											case 4 :
                    		settings.setValue("currentERSKY9XTrev", rev);
											break ;
											
											case 5 :
                    		settings.setValue("currentERSKYX9Erev", rev);
											break ;
											
											case 6 :
                    		settings.setValue("currentERSKY9Xrev", rev);
											break ;
										
											case 7 :
                    		settings.setValue("currentERSKY9XQX7rev", rev);
											break ;
										}
                }
            }
        }
        else
        {
            if(showcheckForUpdatesResult && check1done && check2done && checkGdone)
                QMessageBox::information(this, "eePe", tr("No updates available at this time."));
        }
    }
    else
    {
        if(check1done && check2done && checkGdone)
            QMessageBox::warning(this, "eePe", tr("Unable to check for updates."));
    }
}

void MainWindow::downloadLatester9x()
{

    QSettings settings("er9x-eePskye", "eePskye");

    QString dnldURL, baseFileName;
    switch (settings.value("download-version", 0).toInt())
    {
      case (DNLD_VER_ERSKY9X) :
          dnldURL = ERSKY9X_URL;
          baseFileName = "ersky9x_rom.bin";
      break;
			
      case (DNLD_VER_ERSKY9XR) :
          dnldURL = ERSKY9XR_URL;
          baseFileName = "ersky9xr_rom.bin";
      break;

      case (DNLD_VER_ERSKYX9D):
          dnldURL = ERSKYX9D_URL;
          baseFileName = "x9d_rom.bin";
      break;

      case (DNLD_VER_ERSKYX9DP):
          dnldURL = ERSKYX9DP_URL;
          baseFileName = "x9dp_rom.bin";
      break;

      case (DNLD_VER_ERSKYX9XT):
          dnldURL = ERSKYX9XT_URL;
          baseFileName = "ersky9x9XT_rom.bin";
      break;
            
			case (DNLD_VER_ERSKYX9E):
           dnldURL = ERSKYX9E_URL;
           baseFileName = "x9e_rom.bin";
      break;

			case (DNLD_VER_ERSKY9XQX7):
           dnldURL = ERSKY9X_URL;
           baseFileName = "ersky9x_rom.bin";
      break;
								

      case (DNLD_VER_ERSKY9XA) :
          dnldURL = ERSKY9X_URL;
          baseFileName = "ersky9x_rom.bin";
      break;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),settings.value("lastDir").toString() + "/" + baseFileName,tr(HEX_FILES_FILTER));
    if (fileName.isEmpty()) return;
    settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

    downloadDialog * dd = new downloadDialog(this,dnldURL,fileName);
    switch (settings.value("download-version", 0).toInt())
		{
			case 0 :
    		currentERSKY9Xrev_temp = currentERSKY9Xrev;
			break ;
			case 1 :
    		currentERSKY9Xrev_temp = currentERSKY9XRrev ;
			break ;
      case 2 :
    		currentERSKY9Xrev_temp = currentERSKYX9Drev ;
			break ;
      case 3 :
    		currentERSKY9Xrev_temp = currentERSKYX9DPrev ;
			break ;
			case 4 :
        currentERSKY9Xrev_temp = currentERSKY9XTrev ;
			break ;
      case 5 :
    		currentERSKY9Xrev_temp = currentERSKYX9Erev ;
			break ;
      case 6 :
    		currentERSKY9Xrev_temp = currentERSKY9Xrev;
			break ;
			case 7 :
				currentERSKY9Xrev_temp = currentERSKY9XQX7rev ;
			break ;
		}
    connect(dd,SIGNAL(accepted()),this,SLOT(reply1Accepted()));
    dd->show();
}

void MainWindow::reply2Finished(QNetworkReply * reply)
{
    check2done = true;
    if(check1done && check2done && checkGdone && downloadDialog_forWait)
        downloadDialog_forWait->close();

    QByteArray qba = reply->readAll();
    int i = qba.indexOf("eepskye");

    if(i>0)
    {
        QByteArray qbb = qba.mid(i+6,20) ;
        int j = qbb.indexOf("-r");
        bool cres;
        int rev = QString::fromLatin1(qbb.mid(j+2,4)).replace(QChar('"'), "").toInt(&cres);

        if(!cres)
        {
            QMessageBox::warning(this, "eePskye", tr("Unable to check for updates(1)."));
            return;
        }

        if(rev>currentEEPSKYErev)
        {
            showcheckForUpdatesResult = false; // update is available - do not show dialog

#ifdef Q_OS_WIN32
            int ret = QMessageBox::question(this, "eePskye", tr("A new version of eePskye is available (r%1)<br>"
                                                                "Would you like to download it?").arg(rev) ,
                                            QMessageBox::Yes | QMessageBox::No);

    				QSettings settings("erSKY9x-eePskye", "eePskye");

            if (ret == QMessageBox::Yes)
            {
                QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),settings.value("lastDir").toString() + "/eePeInstall.zip",tr("Zip File (*.zip)"));
                if (fileName.isEmpty()) return;

                downloadDialog * dd = new downloadDialog(this,EEPSKYE_URL,fileName);
                installer_fileName = fileName;
                connect(dd,SIGNAL(accepted()),this,SLOT(reply2Accepted()));
                dd->show();
            }           
#else
            QMessageBox::information(this, "eePe", tr("A new version of eePskye is available (r%1)\n"
                                                      "To update please visit the eepe download page\n"
                                                      "http://www.er9x.com").arg(rev) );
#endif
        }
        else
        {
            if(showcheckForUpdatesResult && check1done && check2done && checkGdone)
                QMessageBox::information(this, "eePskye", tr("No updates available at this time."));
        }
    }
    else
    {
        if(check1done && check2done && checkGdone)
            QMessageBox::warning(this, "eePskye", tr("Unable to check for updates(2)."));
    }
}

void MainWindow::reply1Accepted()
{
    QSettings settings("er9x-eePskye", "eePskye");
    currentERSKY9Xrev = currentERSKY9Xrev_temp;
    switch (settings.value("download-version", 0).toInt())
		{
			case 0 :
    		settings.setValue("currentERSKY9Xrev", currentERSKY9Xrev);
			break ;

			case 1 :
    		settings.setValue("currentERSKY9XRrev", currentERSKY9Xrev);
			break ;
		
			case 2 :
    		settings.setValue("currentERSKYX9Drev", currentERSKY9Xrev);
			break ;
		
			case 3 :
    		settings.setValue("currentERSKYX9DPrev", currentERSKY9Xrev);
			break ;
											
			case 4 :
        settings.setValue("currentERSKY9XTrev", currentERSKY9XTrev);
			break ;
											
			case 5 :
        settings.setValue("currentERSKYX9Erev", currentERSKYX9Erev);
			break ;

			case 6 :
    		settings.setValue("currentERSKY9Xrev", currentERSKY9Xrev);
			break ;
		
			case 7 :
    		settings.setValue("currentERSKY9Xrev", currentERSKY9XQX7rev);
			break ;
		}	
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
    QSettings settings("er9x-eePskye", "eePskye");
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
		title() ;
}

void MainWindow::reviewOut()
{
    reviewOutput *rO = new reviewOutput(this);
    rO->exec();
}

void MainWindow::doTelemetry()
{
    telemetryDialog *td = new telemetryDialog(this);
    td->exec();
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

QStringList MainWindow::GetSambaArguments(const QString &tcl)
{
  QStringList result;
  burnConfigDialog bcd ;

	if ( bcd.getUseSamba() == 0 )
	{
    return result ;
	}

  QString tclFilename = QDir::tempPath() + "/temp.tcl";
  if (QFile::exists(tclFilename)) {
    remove(tclFilename.toLatin1());
  }
  QFile tclFile(tclFilename);
  if (!tclFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, tr("Error"),
        tr("Cannot write file %1:\n%2.")
        .arg(tclFilename)
        .arg(tclFile.errorString()));
    return result;
  }

  QTextStream outputStream(&tclFile);
  outputStream << tcl;

	result << bcd.getARMPort() << bcd.getMCU() << tclFilename ;
  return result;

}

extern QString AvrdudeOutput ;
extern QString VolNames[] ;

// Read the EEPROM from the Radio
void MainWindow::burnFrom()
{
    burnConfigDialog bcd;
    int res = 0 ;
    QString avrdudeLoc = bcd.getAVRDUDE();
//    QString avrdudeLoc = bcd.getSAMBA();
    QString tempDir    = QDir::tempPath();
//    QString programmer = bcd.getProgrammer();
//    QString mcu        = bcd.getMCU();
//    QStringList args   = bcd.getAVRArgs();
//    if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

    QString tempFile = tempDir + "/temp.bin";
//    QString str = "eeprom:r:" + tempFile + ":i"; // writing eeprom -> MEM:OPR:FILE:FTYPE"

	QString size = QString("0x%1").arg( 64*8192,5,16,QChar('0')) ;
//    QStringList arguments = GetSambaArguments(QString("SERIALFLASH::Init 0\n") + "receive_file {SerialFlash AT25} \"\" 0x0 0x1000 0\n" + "receive_file {SerialFlash AT25} \"" + tempFile + "\" 0x0 0x22000 0\n");
	
	QStringList arguments = GetSambaArguments(QString("SERIALFLASH::Init 0\n") + "receive_file {SerialFlash AT25} \"" + tempFile + "\" 0x0 " + size + " 0\n");

	if ( arguments.isEmpty() )
	{
		// Not using SAM-BA
		QString path ;
		path = FindErskyPath( 0 ) ;	// EEPROM
	  if ( path.isEmpty() )
		{
      QMessageBox::critical(this, "eePskye", tr("Tx Disk Not Mounted" ) ) ;
//			AvrdudeOutput = VolNames[0] + " , " + VolNames[1] + " , " + VolNames[2] + " , " + VolNames[3] + " , " + VolNames[4] + " , " + VolNames[5] + " , " + VolNames[6] + " , " + VolNames[7] ;
      return;
		}
		else
		{
      qint32 fsize ;
//			fsize = (MAX_IMODELS+1)*8192 ;
			fsize = 64*8192 ;
			if ( QFileInfo(path).size() == 32768 )
			{
				fsize = 32768 ;			// Taranis EEPROM
			}
			if ( QFileInfo(tempFile).size() > fsize )
			{
				QFile file ;
				file.setFileName(tempFile) ;
				file.remove() ;
			}
			avrdudeLoc = "" ;
      arguments << tempFile << path << tr("%1").arg(fsize) << "0" ;
//   	  QMessageBox::critical(this, "eePskye", tr("Read File Size = %1\n%2" ).arg(fsize).arg(tempFile) ) ;
	    avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments,tr("Read EEPROM From Tx")); //, AVR_DIALOG_KEEP_OPEN);
			res = ad->result() ;
			delete ad ;
		}
	} 
  else
	{
    avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments,tr("Read EEPROM From Tx")); //, AVR_DIALOG_KEEP_OPEN);
    ad->setWindowIcon(QIcon(":/images/read_eeprom.png"));
    res = ad->exec();
		if ( res == 0 )
		{
      QMessageBox::critical(this, "eePskye", tr("SAM-BA did not finish correctly" ) ) ;
		}
		Found9Xtreme = 0 ;
	}

  if(QFileInfo(tempFile).exists() && res)
  {
//   	  QMessageBox::critical(this, "eePskye", tr("Check tempfile Size = %1\n%2" ).arg(QFileInfo(tempFile).size()).arg(tempFile) ) ;
      MdiChild *child = createMdiChild();
      child->newFile();
      if(!child->loadFile(tempFile,true))
      {
          child->close();
          return;
      }

      child->setModified();
      child->show();
      if(!child->parentWidget()->isMaximized() && !child->parentWidget()->isMinimized()) child->parentWidget()->resize(400,500);
  }
}

void MainWindow::burnExtenalToEEPROM()
{
    QSettings settings("er9x-eePskye", "eePskye");
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose file to write to EEPROM memory"),settings.value("lastDir").toString(),tr(EXTERNAL_EEPROM_FILES_FILTER));
    if (!fileName.isEmpty())
    {
        settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

        int ret = QMessageBox::question(this, "eePskye", tr("Write %1 to EEPROM memory?").arg(QFileInfo(fileName).fileName()), QMessageBox::Yes | QMessageBox::No);
        if(ret!=QMessageBox::Yes) return;

        burnConfigDialog bcd;
        QString avrdudeLoc = bcd.getAVRDUDE();
//        QString programmer = bcd.getProgrammer();
//        QString mcu        = bcd.getMCU();
//        QStringList args   = bcd.getAVRArgs();
//        if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

		    QStringList arguments = GetSambaArguments(QString("SERIALFLASH::Init 0\n") + "send_file {SerialFlash AT25} \"" + fileName + "\" 0x0 0\n");
				if ( arguments.isEmpty() )
				{
					// Not using SAM-BA
					QString path ;
					path = FindErskyPath( 0 ) ;	// EEPROM
	  			if ( path.isEmpty() )
					{
    			  QMessageBox::critical(this, "eePskye", tr("Tx Disk Not Mounted" ) ) ;
    			  return;
					}
					else
					{
						avrdudeLoc = "" ;
			      qint32 fsize ;
//						fsize = (MAX_IMODELS+1)*8192 ;
						fsize = 64*8192 ;
						if ( QFileInfo(path).size() == 32768 )
						{
							fsize = 32768 ;			// Taranis EEPROM
						}
    			  arguments << path << fileName << tr("%1").arg(fsize) << "0" ;
	  			  avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments,tr("Write EEPROM to Tx")); //, AVR_DIALOG_KEEP_OPEN);
//						res = ad->result() ;
						delete ad ;
					}
				}
  			else
				{
	        avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments, "Write EEPROM To Tx", AVR_DIALOG_SHOW_DONE);
  	      ad->setWindowIcon(QIcon(":/images/write_eeprom.png"));
    	    ad->show();
				}
    }
}

void MainWindow::burnToFlash(QString fileToFlash)
{
    QSettings settings("er9x-eePskye", "eePskye");
    QString fileName;
    if(fileToFlash.isEmpty())
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to write to flash memory"),settings.value("lastDir").toString(),tr(FLASH_FILES_FILTER));
    else
        fileName = fileToFlash;

    if (!fileName.isEmpty())
    {
        settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

        int ret = QMessageBox::question(this, "eePskye", tr("Write %1 to flash memory?").arg(QFileInfo(fileName).fileName()), QMessageBox::Yes | QMessageBox::No);
        if(ret!=QMessageBox::Yes) return;

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


        burnConfigDialog bcd;
        QString avrdudeLoc = bcd.getAVRDUDE();
//        QString programmer = bcd.getProgrammer();
//        QString mcu        = bcd.getMCU();
//        QStringList args   = bcd.getAVRArgs();
//        if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

//        QString str = "flash:w:" + fileName; // writing eeprom -> MEM:OPR:FILE:FTYPE"
//        if(QFileInfo(fileName).suffix().toUpper()=="HEX") str += ":i";
//        else if(QFileInfo(fileName).suffix().toUpper()=="BIN") str += ":r";
//        else str += ":a";

        QStringList arguments = GetSambaArguments(QString("send_file {Flash} \"") + fileName + "\" 0x400000 0\n" + "FLASH::ScriptGPNMV 2\n");
//        arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;
				if ( arguments.isEmpty() )
				{
					// Not using SAM-BA
					QString path ;
					path = FindErskyPath( 1 ) ;	// FLASH
	  			if ( path.isEmpty() )
					{
    			  QMessageBox::critical(this, "eePskye", tr("Tx Disk Not Mounted" ) ) ;
    			  return;
					}
					else
					{
						avrdudeLoc = "" ;
    		    arguments << path << fileName << tr("%1").arg(QFileInfo(fileName).size()) << "0" ;
	  			  avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments,tr("Read EEPROM From Tx")); //, AVR_DIALOG_KEEP_OPEN);
		//				res = ad->result() ;
						delete ad ;
					}
				} 
  			else
				{

		      avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments, "Write Flash To Tx", AVR_DIALOG_SHOW_DONE);
    	    ad->setWindowIcon(QIcon(":/images/write_flash.png"));
      	  ad->show();
				}
    }
}


void MainWindow::burnExtenalFromEEPROM()
{
    QSettings settings("er9x-eePskye", "eePskye");
    QString fileName = QFileDialog::getSaveFileName(this,tr("Read EEPROM memory to File"),settings.value("lastDir").toString(),tr(EXTERNAL_EEPROM_FILES_FILTER));
    if (!fileName.isEmpty())
    {
        settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

        burnConfigDialog bcd;
        QString avrdudeLoc = bcd.getAVRDUDE();
//        QString programmer = bcd.getProgrammer();
//        QString mcu        = bcd.getMCU();
//        QStringList args   = bcd.getAVRArgs();
//        if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();


//        QString str = "eeprom:r:" + fileName;
//        if(QFileInfo(fileName).suffix().toUpper()=="HEX") str += ":i";
//        else if(QFileInfo(fileName).suffix().toUpper()=="BIN") str += ":r";
//        else str += ":a";

		    QStringList arguments = GetSambaArguments(QString("SERIALFLASH::Init 0\n") + "receive_file {SerialFlash AT25} \"" + fileName + "\" 0x0 0x80000 0\n");
//        arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;
				int res ;
				if ( arguments.isEmpty() )
				{
					// Not using SAM-BA
					QString path ;
					path = FindErskyPath( 0 ) ;	// EEPROM
				  if ( path.isEmpty() )
					{
  			    QMessageBox::critical(this, "eePskye", tr("Tx Disk Not Mounted" ) ) ;
  			    return;
					}
					else
					{
      			qint32 fsize ;
//						fsize = (MAX_IMODELS+1)*8192 ;
						fsize = 64*8192 ;
						if ( QFileInfo(path).size() == 32768 )
						{
							fsize = 32768 ;			// Taranis EEPROM
						}
						if ( QFileInfo(fileName).size() > fsize )
						{
							QFile file ;
							file.setFileName(fileName) ;
							file.remove() ;
						}
						avrdudeLoc = "" ;
  			    arguments << fileName << path << tr("%1").arg(fsize) << "0" ;
				    avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments,tr("Read EEPROM From Tx")); //, AVR_DIALOG_KEEP_OPEN);
						res = ad->result() ;
						delete ad ;
					}
				} 
  			else
				{

		    	avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments,tr("Read EEPROM From Tx"), AVR_DIALOG_SHOW_DONE);
		    	ad->setWindowIcon(QIcon(":/images/read_eeprom.png"));
    			res = ad->exec();
				}	
					
				if ( res )
				{
					if( QFileInfo(fileName).exists() )
					{
        		QFile file(fileName);
	      	  if ( (file.size()!=EEFULLSIZE) && (file.size()!=32768) )
						{
    	  	  	QMessageBox::critical(this, tr("Error"),tr("Error reading file:\n"
        		                                             "File wrong size(2) - %1").arg(fileName));
      		  	return ;
						}
					}
					else
					{
	      	  QMessageBox::critical(this, tr("Error"),tr("Error reading file:\n").arg(fileName));
	      	  return ;
					}
				}
    }

}


void MainWindow::burnFromFlash()
{

  QSettings settings("er9x-eePskye", "eePskye");
  QString fileName = QFileDialog::getSaveFileName(this,tr("Read Flash to File"),settings.value("lastDir").toString(),tr(FLASH_FILES_FILTER));

  if (!fileName.isEmpty())
  {
    settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

    burnConfigDialog bcd;
    QString avrdudeLoc = bcd.getAVRDUDE();
//        QString programmer = bcd.getProgrammer();
//        QString mcu        = bcd.getMCU();
//        QStringList args   = bcd.getAVRArgs();
//        if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();


//        QString str = "flash:r:" + fileName; // writing eeprom -> MEM:OPR:FILE:FTYPE"
//        if(QFileInfo(fileName).suffix().toUpper()=="HEX") str += ":i";
//        else if(QFileInfo(fileName).suffix().toUpper()=="BIN") str += ":r";
//        else str += ":a";

    QStringList arguments = GetSambaArguments(QString("receive_file {Flash} \"") + fileName + "\" 0x400000 0x40000 0\n") ;
//        arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;

		if ( arguments.isEmpty() )
		{
			// Not using SAM-BA
			QString path ;
			path = FindErskyPath( 1 ) ;	// FLASH
	  	if ( path.isEmpty() )
			{
    	  QMessageBox::critical(this, "eePskye", tr("Tx Disk Not Mounted" ) ) ;
    	  return;
			}
			else
			{
				avrdudeLoc = "" ;
        arguments << fileName << path << tr("%1").arg(QFileInfo(path).size()) << "0" ;
	  	  avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments,tr("Read EEPROM From Tx")); //, AVR_DIALOG_KEEP_OPEN);
//				res = ad->result() ;
				delete ad ;
			}
		} 
  	else
		{
      avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments, "Read Flash From Tx", AVR_DIALOG_SHOW_DONE);
      ad->setWindowIcon(QIcon(":/images/read_flash.png"));
      ad->show();
		}
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

//void MainWindow::setFuses()
//{
//    burnConfigDialog *bcd = new burnConfigDialog(this);
//    bcd->restFuses(true);
//}

//void MainWindow::resetFuses()
//{
//    burnConfigDialog *bcd = new burnConfigDialog(this);
//    bcd->restFuses(false);
//}

void MainWindow::showEEPROMInfo()
{
    //show info about:
    // eeprom size
    // available models
    // model sizes
    // EEPROM version
    // Free space

    if(activeMdiChild() == 0)
        return;

    QString msg = "<table cellspacing=0 cellpadding=0 border=0>";
    msg.append("<tr><td colspan=2><u><b>");
    msg.append(QString("%1").arg(QFileInfo(activeMdiChild()->currentFile()).fileName()));
    msg.append("</b></u></td></tr>");

    int modelSizes[MAX_IMODELS+1] = {0};
    int totalSize = 0;


    for(int i=0; i<=MAX_MODELS; i++)
    {
//        modelSizes[i] = activeMdiChild()->modelSize(i);
//        totalSize += modelSizes[i];
    }

    msg.append(tr("<tr><td>Owner: </td><td align=right>%1</td></tr>").arg(activeMdiChild()->ownerName()));
    msg.append(tr("<tr><td>Version: </td><td align=right>%1</td></tr>").arg(activeMdiChild()->eepromVersion()));

    msg.append(tr("<tr><td>Bytes Used: </td><td align=right>%1</td></tr>").arg(totalSize));
    msg.append(tr("<tr><td>Bytes Free: </td><td align=right>%1</td></tr>").arg(EESIZE - totalSize));
    msg.append(tr("<tr><td>Bytes Total:</td><td align=right> %1</td></tr>").arg(EESIZE));

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
    QString aboutStr = "<center><img src=\":/images/eepskye-title.png\"><br>";
    aboutStr.append(tr("Copyright") +" Michael Blandford &copy;2013<br>");
    aboutStr.append(QString("<a href='https://github.com/MikeBland/mbtx'>https://github.com/MikeBland/mbtx/</a><br>Revision: p%1, %2<br><br>").arg(currentEEPSKYErev).arg(__DATE__));
    aboutStr.append(tr("If you've found this program and/or the ersky9x firmware useful please support by"));
    aboutStr.append(" <a href='" DONATE_MB_STR "'>");
//    aboutStr.append(tr("donating") + "</a></center><br>");
    aboutStr.append(tr("donating") + "</a><br>");
		aboutStr.append(QString("Click for latest <a href='https://raw.githubusercontent.com/MikeBland/mbtx/master/README.md'>Revisions</a></center><br><br>"));

//    aboutStr.append(tr("geegeneral size = %1").arg(sizeof(EEGeneral)));

    QMessageBox::about(this, tr("About eePskye"),aboutStr);
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

//    burnListAct = new QAction(QIcon(":/images/list.png"), tr("&List programmers"), this);
//    burnListAct->setStatusTip(tr("List available programmers"));
//    connect(burnListAct,SIGNAL(triggered()),this,SLOT(burnList()));

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

    aboutAct = new QAction(QIcon(":/eepskye.png"), tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    releaseAct = new QAction(QIcon(":/eepskye.png"), tr("&Release Notes"), this);
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

//    setFusesAct = new QAction(QIcon(":/images/fuses_set.png"), tr("Set fuses to protect EEPROM"), this);
//    setFusesAct->setStatusTip(tr("Sets the fuses to protect EEPROM from being erased."));
//    connect(setFusesAct, SIGNAL(triggered()), this, SLOT(setFuses()));

//    resetFusesAct = new QAction(QIcon(":/images/fuses_set.png"), tr("Reset fuses to factory default"), this);
//    resetFusesAct->setStatusTip(tr("Resets the fuses to factory default - EEPROM erase."));
//    connect(resetFusesAct, SIGNAL(triggered()), this, SLOT(resetFuses()));

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
//    burnMenu->addAction(burnListAct);
    burnMenu->addSeparator();
//    burnMenu->addAction(setFusesAct);
//    burnMenu->addAction(resetFusesAct);
//    burnMenu->addSeparator();
    burnMenu->addAction(reviewBurnOutput);

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
//    burnToolBar->addAction(setFusesAct);

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
    QSettings settings("er9x-eePskye", "eePskye");
    bool maximized = settings.value("maximized", false).toBool();
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();

    currentERSKY9Xrev = settings.value("currentERSKY9Xrev", 1).toInt();
    currentERSKY9XRrev = settings.value("currentERSKY9XRrev", 1).toInt();
    currentERSKYX9Drev = settings.value("currentERSKYX9Drev", 1).toInt();
    currentERSKYX9DPrev = settings.value("currentERSKYX9DPrev", 1).toInt();
    currentERSKY9XTrev = settings.value("currentERSKY9XTrev", 1).toInt();
		currentEEPSKYErelease = settings.value("currentEEPSKYErelease", 1).toInt();
    currentEEPSKYErev = SVN_VER_NUM ;

    checkERSKY9X  = settings.value("startup_check_ersky9x", true).toBool();
    checkEEPSKYE  = settings.value("startup_check_eepskye", true).toBool();
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
    QSettings settings("er9x-eePskye", "eePskye");

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

//#define DWORD uint32_t
//#define WCHAR wchar_t






