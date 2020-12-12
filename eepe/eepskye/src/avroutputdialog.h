#ifndef AVROUTPUTDIALOG_H
#define AVROUTPUTDIALOG_H

#include <QDialog>
#include <QtGui>

#define AVR_DIALOG_CLOSE_IF_SUCCESSFUL 0x00
#define AVR_DIALOG_KEEP_OPEN           0x01
#define AVR_DIALOG_FORCE_CLOSE         0x02
#define AVR_DIALOG_SHOW_DONE           0x04


namespace Ui {
    class avrOutputDialog;
}

class avrOutputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit avrOutputDialog(QWidget *parent, QString prog, QStringList arg, QString wTitle, int closeBehaviour=AVR_DIALOG_CLOSE_IF_SUCCESSFUL);
    ~avrOutputDialog();

    void addText(const QString &text);
    void runAgain(QString prog, QStringList arg, int closeBehaviour=AVR_DIALOG_CLOSE_IF_SUCCESSFUL);
    void waitForFinish();
//    void addReadFuses();
    int doFileCopy( QString destFile, QString sourceFile, quint32 size, quint32 offset ) ;
//    int doSdRead( QString destFile, QString sourceFile, quint32 offset ) ;

protected slots:
    void doAddTextStdOut();
    void doAddTextStdErr();
    void doProcessStarted();
    void doFinished(int code);

    void killTimerElapsed();

private:
    Ui::avrOutputDialog *ui;

		void delayForDsiplayUpdate( uint32_t milliseconds ) ;
    QProcess *process;
    QTimer *kill_timer;
    QString cmdLine;
    int closeOpt;
    quint8 lfuse;
    quint8 hfuse;
    quint8 efuse;
		int has_errors ;
};

#endif // AVROUTPUTDIALOG_H
