#include "avroutputdialog.h"
#include "ui_avroutputdialog.h"
#include <QtGui>

//#if !__GNUC__
#if defined WIN32 || !defined __GNUC__
#include <Windows.h>
#include <WinBase.h>
#include <tlhelp32.h>
#else
#include <unistd.h>
//#include "mountlist.h"
#endif
//#endif

extern QString AvrdudeOutput ;

avrOutputDialog::avrOutputDialog(QWidget *parent, QString prog, QStringList arg, QString wTitle, int closeBehaviour) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
    ui(new Ui::avrOutputDialog),
		kill_timer(NULL)
{
  ui->setupUi(this);

  cmdLine = prog;

  if (cmdLine.isEmpty())
	{
    setWindowTitle( tr( "Data Transfer" ) ) ;
		
		QString sourceFile ;
		QString destFile ;
		quint32 size ;
		quint32 offset ;
		quint32 numBlocks ;
    destFile = arg.at(0) ;
    sourceFile = arg.at(1) ;
    size = arg.at(2).toInt() ;
    offset = arg.at(3).toInt() ;
		numBlocks = size / 4096 ;

    ui->progressBar->setMaximum(numBlocks) ;
    ui->progressBar->setValue(0) ;
//		thread()->wait( 400 ) ;

		show() ;
		repaint() ;
		if ( doFileCopy( destFile, sourceFile, size, offset ) == 0 )
		{
      QMessageBox::critical(this, "eePskye", tr("Operation Failed"));
      reject();
		}
		else
		{
      QMessageBox::information(this, "eePskye", tr("Operation Successful"));
    	accept();
		}
		AvrdudeOutput = ui->plainTextEdit->toPlainText() ;
	}
	else
	{
		if(wTitle.isEmpty())
        setWindowTitle(tr("SAM-BA result"));
    else
        setWindowTitle(tr("SAM_BA - ") + wTitle);

    foreach(QString str, arg) cmdLine.append(" " + str);
    closeOpt = closeBehaviour;

    lfuse = 0;
    hfuse = 0;
    efuse = 0;
		has_errors = 0 ;

    process = new QProcess(this);

    connect(process,SIGNAL(readyReadStandardError()), this, SLOT(doAddTextStdErr()));
    connect(process,SIGNAL(started()),this,SLOT(doProcessStarted()));
    connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(doAddTextStdOut()));
    connect(process,SIGNAL(finished(int)),this,SLOT(doFinished(int)));

//#if !__GNUC__
      kill_timer = new QTimer(this);
      connect(kill_timer, SIGNAL(timeout()), this, SLOT(killTimerElapsed()));
      kill_timer->start(2000);
//#endif
    
		process->start(prog,arg);
	}
}

#if defined WIN32 || !defined __GNUC__
BOOL KillProcessByName(char *szProcessToKill){
        HANDLE hProcessSnap;
        HANDLE hProcess;
        PROCESSENTRY32 pe32;
//        DWORD dwPriorityClass;
				char xname[20] ;


        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // Takes a snapshot of all the processes

        if(hProcessSnap == INVALID_HANDLE_VALUE){
                return( FALSE );
        }

        pe32.dwSize = sizeof(PROCESSENTRY32);

        if(!Process32First(hProcessSnap, &pe32)){
                CloseHandle(hProcessSnap);
                return( FALSE );
        }

        do{
					
					 char *p ;
					 int i ;
					 p = (char *)pe32.szExeFile ;
					 for ( i = 0 ; i < 20 ; i += 1 )
					 {
						xname[i] = *p ;
						p += 2 ;					 	
					 }
					
                if(!strcmp( xname, szProcessToKill)){    //  checks if process at current position has the name of to be killed app
                        hProcess = OpenProcess(PROCESS_TERMINATE,0, pe32.th32ProcessID);  // gets handle to process
                        TerminateProcess(hProcess,0);   // Terminate process by handle
                        CloseHandle(hProcess);  // close the handle
                }
        }while(Process32Next(hProcessSnap,&pe32));  // gets next member of snapshot

        CloseHandle(hProcessSnap);  // closes the snapshot handle
        return( TRUE );
}
#endif

void avrOutputDialog::killTimerElapsed()
{
  delete kill_timer;
  kill_timer = NULL;
#if defined WIN32 || !defined __GNUC__
  KillProcessByName( (char *)"tasklist.exe");
#endif
}


avrOutputDialog::~avrOutputDialog()
{
    delete ui;
    delete kill_timer;
}

void avrOutputDialog::runAgain(QString prog, QStringList arg, int closeBehaviour)
{
    cmdLine = prog;
    foreach(QString str, arg) cmdLine.append(" " + str);
    closeOpt = closeBehaviour;
    process->start(prog,arg);
}

void avrOutputDialog::waitForFinish()
{
    process->waitForFinished();
}

void avrOutputDialog::addText(const QString &text)
{
    int val = ui->plainTextEdit->verticalScrollBar()->maximum();
    ui->plainTextEdit->insertPlainText(text);
    if(val!=ui->plainTextEdit->verticalScrollBar()->maximum())
        ui->plainTextEdit->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->maximum());
}


void avrOutputDialog::doAddTextStdOut()
{
    QByteArray data = process->readAllStandardOutput();
    QString text = QString(data);

    addText(text);

    if (text.contains("Complete ")) {
//#if !__GNUC__
      if (kill_timer) {
        delete kill_timer;
        kill_timer = NULL;
      }
//#endif
//      int start = text.indexOf("Complete ");
//      int end = text.indexOf("%");
//      if (start > 0) {
//        start += 9;
//        int value = text.mid(start, end-start).toInt();
//        ui->progressBar->setValue(value);
//      }
    }

    //addText("\n=====\n" + text + "\n=====\n");
    
		if(text.contains(":010000")) //contains fuse info
    {
        QStringList stl = text.split(":01000000");

        foreach (QString t, stl)
        {
            bool ok = false;
            if(!lfuse)        lfuse = t.left(2).toInt(&ok,16);
            if(!hfuse && !ok) hfuse = t.left(2).toInt(&ok,16);
            if(!efuse && !ok) efuse = t.left(2).toInt(&ok,16);
        }
    }
    
		if (text.contains("-E-"))
		{
			has_errors = 1 ;
		}	
}


void avrOutputDialog::doAddTextStdErr()
{
    QByteArray data = process->readAllStandardError();
    QString text = QString(data);
    addText(text);
}

#define HLINE_SEPARATOR "================================================================================="
void avrOutputDialog::doFinished(int code=0)
{
    addText("\n" HLINE_SEPARATOR);
    if (code==1) code=0 ;
    if(code)
        addText("\n" + tr("SAM-BA done - exit code %1").arg(code));
    else if (has_errors)
		{
      addText("\nSAM-BA" + tr(" done with errors"));
		}
    else
        addText("\n" + tr("SAM-BA done - SUCCESSFUL"));
    addText("\n" HLINE_SEPARATOR "\n");

//    if(lfuse || hfuse || efuse) addReadFuses();
		AvrdudeOutput = ui->plainTextEdit->toPlainText() ;

    switch(closeOpt)
    {
    case (AVR_DIALOG_CLOSE_IF_SUCCESSFUL): if(code || has_errors) reject(); else accept(); break;
    case (AVR_DIALOG_FORCE_CLOSE): if(code || has_errors) reject(); else accept(); break;
    case (AVR_DIALOG_SHOW_DONE):
        if(code || has_errors)
        {
            QMessageBox::critical(this, "eePskye", tr("SAM-BA did not finish correctly"));
            reject();
        }
        else
        {
            QMessageBox::information(this, "eePskye", tr("SAM-BA finished correctly"));
            accept();
        }
        break;
    default: //AVR_DIALOG_KEEP_OPEN
        break;
    }


}

void avrOutputDialog::doProcessStarted()
{
    addText(HLINE_SEPARATOR "\n");
    addText(tr("Started SAM-BA") + "\n");
    addText(cmdLine);
    addText("\n" HLINE_SEPARATOR "\n");
}


int avrOutputDialog::doFileCopy( QString destFile, QString sourceFile, quint32 size, quint32 offset )
{
  char buf[4096];
  int hasErrors = 0 ;
	quint32 bytesCopied = 0 ;
	quint32 count ;
	quint32 blocks = 0 ;

	QFile source(sourceFile);
  QFile dest(destFile);
  addText("Starting\n");
	repaint() ;
  if (!source.open(QIODevice::ReadOnly))
	{
    QMessageBox::warning(this, tr("Error"),tr("Cannot open source file"));
    hasErrors = 1 ;
  }
	else
	{
    addText("Opened source file\n");
		repaint() ;
    if (!dest.open(QIODevice::ReadWrite))
		{
      QMessageBox::warning(this, tr("Error"),tr("Cannot write destination"));
      hasErrors=true;
	  }
		else
		{
    	addText("Opened destination file\n");
			repaint() ;
			if ( offset )
			{
				size -= offset ;
				blocks += offset / 4096 ;
				source.seek(offset) ;
				dest.seek(offset) ;
			}
			do
			{
				count = size - bytesCopied ;
				if ( count > 4096 )
				{
					count = 4096 ;
				}
				count = source.read( buf, count ) ;
				if ( count )
				{
        	if (dest.write( buf, count ) != count )
					{
        	  hasErrors = 1 ;
						break ;
					}
					bytesCopied += count ;
					blocks += 1 ;
    			ui->progressBar->setValue(blocks) ;
				}
			} while ( count && ( bytesCopied < size ) ) ;
		}
		source.close() ;
    dest.flush() ;
    dest.close() ;
  }
	if ( bytesCopied < size )
	{
    QMessageBox::warning(this, tr("Error"),tr("Operation Failed"));
    hasErrors=true;
	}
	return hasErrors ? 0 : 1 ;
}


//void avrOutputDialog::addReadFuses()
//{
//    addText(HLINE_SEPARATOR "\n");
//    addText(tr("FUSES: Low=%1 High=%2 Ext=%3").arg(lfuse,2,16,QChar('0')).arg(hfuse,2,16,QChar('0')).arg(efuse,2,16,QChar('0')));
//    addText("\n" HLINE_SEPARATOR "\n");
//}

