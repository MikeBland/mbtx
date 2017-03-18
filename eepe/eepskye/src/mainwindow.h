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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "downloaddialog.h"
#include <QMainWindow>
#include <QDropEvent>

class QNetworkAccessManager;
class QDateTime;
class MdiChild;
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    void downloadLatester9x();

public slots:
    void checkForUpdates(bool ignoreSettings=true);

protected:
    void closeEvent(QCloseEvent *event);
		void dropEvent(QDropEvent *event) ;
		void dragEnterEvent(QDragEnterEvent *ev) ;

private slots:

    void reply1Finished(QNetworkReply * reply);
    void reply2Finished(QNetworkReply * reply);

    void reply1Accepted();
    void reply2Accepted();

    void newFile();
    void open();
    void save();
    void saveAs();
    void cut();
    void copy();
    void paste();
    void burnTo();
    void burnFrom();
    void burnToFlash(QString fileToFlash="");
    void burnFromFlash();
    void burnExtenalToEEPROM();
    void burnExtenalFromEEPROM();
    void burnConfig();
    void burnList();
    void simulate();
    void about();
    void print();
    void preferences();
		void reviewOut() ;
    void donators();
    void showEr9xManual();
    void updateMenus();
    void updateWindowMenu();
    MdiChild *createMdiChild();
    void switchLayoutDirection();
    void setActiveSubWindow(QWidget *window);
    void loadModelFromFile();
    void saveModelToFile();
    void customizeSplash();
//    void setFuses();
//    void resetFuses();
    void showEEPROMInfo();
    void doTelemetry();
		void releaseNotes() ;
    void title();


private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    MdiChild *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);
//		QString FindErskyPath( int type ) ;

    QStringList GetSambaArguments(const QString &tcl) ;

    QMdiArea *mdiArea;
    QSignalMapper *windowMapper;

    QString installer_fileName;
    downloadDialog * downloadDialog_forWait;

    int currentERSKY9Xrev;
    int currentERSKY9XRrev;
    int currentERSKYX9Drev;
    int currentERSKYX9DPrev;
		int currentERSKY9XTrev ;
    int currentERSKYX9Erev;
		int currentERSKY9XQX7rev ;
    int currentERSKY9Xrev_temp;
    int currentEEPSKYErev;
    int currentEEPSKYErelease ;
    bool checkERSKY9X;
    bool checkEEPSKYE;
    bool showcheckForUpdatesResult;
    bool checkGdone;
    bool check1done;
    bool check2done;

    QNetworkAccessManager *managerG ;
    QNetworkAccessManager *manager1;
    QNetworkAccessManager *manager2;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *burnMenu;
    QMenu *telemetryMenu;
    QMenu *windowMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *burnToolBar;
    QToolBar *helpToolBar;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *preferencesAct;
    QAction *reviewBurnOutput ;
    QAction *checkForUpdatesAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *burnToAct;
    QAction *burnFromAct;
    QAction *burnConfigAct;
    QAction *burnListAct;
    QAction *burnToFlashAct;
    QAction *burnFromFlashAct;
    QAction *burnExtenalToEEPROMAct;
    QAction *burnExtenalFromEEPROMAct;
    QAction *simulateAct;
    QAction *closeAct;
    QAction *closeAllAct;
    QAction *tileAct;
    QAction *cascadeAct;
    QAction *nextAct;
    QAction *previousAct;
    QAction *separatorAct;
    QAction *aboutAct;
    QAction *releaseAct;
    QAction *donatorsAct;
    QAction *printAct;
    QAction *switchLayoutDirectionAct;
    QAction *showEr9xManualAct;
    QAction *loadModelFromFileAct;
    QAction *saveModelToFileAct;
    QAction *customizeSplashAct;
//    QAction *setFusesAct;
//    QAction *resetFusesAct;
    QAction *eepromInfoAct;
    QAction *telemetryAct;

    //QAction *aboutQtAct;
};

#endif
