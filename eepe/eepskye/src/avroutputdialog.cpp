#include "avroutputdialog.h"
#include "ui_avroutputdialog.h"
#include <QtGui>
#include <QScrollBar>
#include <QMessageBox>
#include <QScrollBar>
#include "pers.h"
#include "myeeprom.h"

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

unsigned char AModelNames[MAX_IMODELS+1][MODEL_NAME_LEN+1] ;		// Allow for general

extern void fixHeader( uint8_t *header ) ;
extern uint32_t unfixHeader( uint8_t *header ) ;

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
		quint32 mode ;
    QString fname ;
    destFile = arg.at(0) ;
    sourceFile = arg.at(1) ;
    size = arg.at(2).toInt() ;
    offset = arg.at(3).toInt() ;
    mode = arg.at(4).toInt() ;
		numBlocks = size / 4096 ;

		ui->progressBar->show() ;
    ui->progressBar->setMaximum(numBlocks) ;
    ui->progressBar->setValue(0) ;
//		thread()->wait( 400 ) ;

		show() ;
		repaint() ;
		if ( mode == 1 )
		{
		  quint8 temp[16384] ;
				
			QFile file ;
			file.setFileName(destFile) ;
			fname = sourceFile ;
			fname.append( "radio.bin" ) ;

			QFile rfile ;
			rfile.setFileName(fname) ;
    	if (!rfile.open(QIODevice::ReadOnly ))
			{  //reading file
        QMessageBox::critical( this, "eePskye",
                      tr("Error opening file %1:\n%2.")
                      .arg(fname)
                      .arg(file.errorString())) ;
 	  		reject() ;
				return ;
			}
	    ui->progressBar->setValue(1) ;
	    addText("Read Radio file\n");
			repaint() ;

		  memset(temp,0,16384) ;
			long s = rfile.size() ;
			if ( s > 16384 )
			{
				s = 16384 ;
			}
		  long result = rfile.read( (char*)&temp, s ) ;
		  rfile.close() ;
    	if (!file.open(QIODevice::WriteOnly ))
			{
        QMessageBox::critical( this, "eePskye",
                              tr("Error opening file %1:\n%2.")
                      .arg("tempfile")
                      .arg(file.errorString())) ;
 	  		reject() ;
				return ;
			}
			fixHeader( &temp[0] ) ;
			fixHeader( &temp[4096] ) ;
			fixHeader( &temp[8192] ) ;

			uint32_t seq[3] ;
			uint32_t i ;
			seq[0] = 0 ;
			seq[1] = 0 ;
			seq[2] = 0 ;
			for ( i = 0 ; i < 4 ; i += 1 )
			{
				seq[0] <<= 8 ;
				seq[1] <<= 8 ;
				seq[2] <<= 8 ;
				seq[0] |= temp[3-i] ;
				seq[1] |= temp[4099-i] ;
				seq[2] |= temp[8195-i] ;
			}
			if ( seq[2] > seq[1] )
			{
        memcpy( &temp[4096], &temp[8192], 4096 ) ;
			}

			file.write((char*)&temp, 8192 ) ;
			
	    ui->progressBar->setValue(2) ;
			repaint() ;
			
			
			fname = sourceFile ;
			fname.append( "Mnames.bin" ) ;
			rfile.setFileName(fname) ;
    	if (!rfile.open(QIODevice::ReadOnly ))
			{  //reading file
        QMessageBox::critical( this, "eePskye",
                      tr("Error opening file %1:\n%2.")
                      .arg(fname)
                      .arg(file.errorString())) ;
				file.close() ;
    	  reject() ;
				return ;
			}
      result = rfile.read( (char*)&AModelNames, sizeof(AModelNames) ) ;
		  rfile.close() ;
	    addText("Read Model Names\nReading Models\n");
			for ( i = 1 ; i <= MAX_IMODELS ; i += 1)
			{
		    memset(temp,0,8192) ;
				if ( AModelNames[i][0] && (AModelNames[i][0] != ' ') )
				{
					fname = sourceFile ;
					fname.append( "model" ) ;
					fname.append( '0'+(i)/10 ) ;
					fname.append( '0'+(i)%10 ) ;
					fname.append( "A.bin" ) ;
					rfile.setFileName(fname) ;
    			if (!rfile.open(QIODevice::ReadOnly ))
					{  //reading file
        		QMessageBox::critical( this, "eePskye",
        		              tr("Error opening file %1:\n%2.")
        		              .arg(fname)
        		              .arg(file.errorString())) ;
						file.close() ;
    	  		reject() ;
						return ;
					}
					long s = rfile.size() ;
					if ( s > 8192 )
					{
						s = 8192 ;
					}
		    	long result = rfile.read( (char*)&temp, s ) ;
		    	rfile.close() ;
					fixHeader( &temp[0] ) ;
					fixHeader( &temp[4096] ) ;
				}
				file.write((char*)&temp, 8192 ) ;
		    ui->progressBar->setValue(i+1) ;
				delayForDsiplayUpdate( 25 ) ;
				repaint() ;
			}
		  memset(temp,0,8192) ;
			file.write((char*)&temp, 8192 ) ;
			file.write((char*)&temp, 8192 ) ;
			file.write((char*)&temp, 8192 ) ;
	    ui->progressBar->setValue(numBlocks) ;
			repaint() ;

//				QMessageBox::critical(this, "eePskye", tr("X10 Not Supported" ) ) ;
			file.close() ;
   		accept() ;
//			res = 1 ;
		}
		else if ( mode == 2 )
		{
			quint8 temp[8192] ;
			QFile file ;
			QFile wfile ;
      file.setFileName(sourceFile) ;
			fname = destFile ;
			fname.append( "radio.bin" ) ;
    	if (!file.open(QIODevice::ReadOnly ))
			{  //reading file
        QMessageBox::critical( this, "eePskye",
        			        tr("Error opening file %1:\n%2.")
                      .arg(sourceFile)
        			        .arg(file.errorString())) ;
 	  		reject() ;
				return ;
			}
		  long result = file.read( (char*)&temp, 8192 ) ;
			unfixHeader( temp ) ;
			wfile.setFileName( fname ) ;
			if (!wfile.open(QIODevice::WriteOnly ))
			{
        QMessageBox::critical( this, "eePskye",
                 tr("Error opening file %1:\n%2.")
                .arg(fname)
                .arg(file.errorString())) ;
				file.close() ;
 	  		reject() ;
				return ;
			}
	    ui->progressBar->setValue(1) ;
	    addText("Written Radio file\nWriting Models\n");
			repaint() ;
			
			wfile.write((char*)&temp, 8192 ) ;
			wfile.close() ;
  		memset( AModelNames, ' ', sizeof(AModelNames) ) ;
			uint32_t i ;
			for ( i = 1 ; i <= MAX_IMODELS ; i += 1)
			{
		    result = file.read( (char*)&temp, 8192 ) ;
				uint32_t j ;
				j = unfixHeader( temp ) ;
				if ( j )
				{
          fname = destFile ;
					fname.append( "model" ) ;
					fname.append( '0'+(i)/10 ) ;
					fname.append( '0'+(i)%10 ) ;
					fname.append( "A.bin" ) ;
					wfile.setFileName(fname) ;
					if (!wfile.open(QIODevice::WriteOnly ))
					{
        		QMessageBox::critical( this, "eePskye",
                 tr("Error opening file %1:\n%2.")
                .arg(fname)
                .arg(file.errorString())) ;
						file.close() ;
		 	  		reject() ;
						return ;
					}
					wfile.write((char*)&temp, 8192 ) ;
					wfile.close() ;
					memcpy(AModelNames[i], &temp[8], 10 ) ;
					delayForDsiplayUpdate( 25 ) ;
				}
		    ui->progressBar->setValue(i+1) ;
				repaint() ;
			}
			fname = destFile ;
			fname.append( "Mnames.bin" ) ;
			wfile.setFileName(fname) ;
			if (!wfile.open(QIODevice::WriteOnly ))
			{
     		QMessageBox::critical( this, "eePskye",
               tr("Error opening file %1:\n%2.")
              .arg(fname)
              .arg(file.errorString())) ;
				file.close() ;
 	  		reject() ;
				return ;
			}
	    ui->progressBar->setValue(numBlocks) ;
	    addText("Written Model Names\n");
			repaint() ;
      wfile.write((char *)&AModelNames, sizeof(AModelNames) ) ;
			wfile.close() ;
			file.close() ;
   		accept() ;
			return ;
		}
		else
		{
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
		}
		AvrdudeOutput = ui->plainTextEdit->toPlainText() ;
	}
	else
	{
		ui->progressBar->hide() ;
		
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

void avrOutputDialog::delayForDsiplayUpdate( uint32_t milliseconds )
{
	QTime dieTime = QTime::currentTime().addMSecs( milliseconds ) ;
 	while( QTime::currentTime() < dieTime )
 	{
		QCoreApplication::processEvents( QEventLoop::AllEvents, 100 ) ;
 	}
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


// Sourcefile is Mnames.bin
//int avrOutputDialog::doSdRead( QString destFile, QString sourceFile, quint32 offset )
//{
//// create empty 512K image
//// Read model names
//// read "radio/radio.bin" to general in image
//// For each name that starts with not '\0' or ' ', read model to image
//  int hasErrors = 0 ;
//	quint32 count ;

//	unsigned char ModelNames[MAX_IMODELS+1][MODEL_NAME_LEN+1] ;		// Allow for general
//	QFile source(sourceFile);
//  if (!source.open(QIODevice::ReadOnly))
//	{
//    QMessageBox::warning(this, tr("Error"),tr("Cannot open source file"));
//    hasErrors = 1 ;
//  }
//	else
//	{
//    count = source.read( (char *)ModelNames, sizeof(ModelNames) ) ;
//		source.close() ;

//		// read modelnames, now read /RADIO/radio.bin

//	}
	
//	return hasErrors ? 0 : 1 ;
//}


int avrOutputDialog::doFileCopy( QString destFile, QString sourceFile, quint32 size, quint32 offset )
{
  char buf[4096];
  int hasErrors = 0 ;
	quint32 bytesCopied = 0 ;
	quint32 count ;
	quint32 blocks = 0 ;
	quint32 totalSize = size ;

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
					repaint() ;

					if ( totalSize > 40000 )
					{
						delayForDsiplayUpdate( 25 ) ;
					}
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

