#ifndef REVIEWOUTPUT_H
#define REVIEWOUTPUT_H
#include <QDialog>

namespace Ui {
    class reviewOutput ;
}

class reviewOutput : public QDialog
{
    Q_OBJECT

public:
    explicit reviewOutput(QWidget *parent = 0);
    ~reviewOutput();
		void showCheck( int *checked, QString title, QString text ) ;

private:
    Ui::reviewOutput *ui;
		int *xChecked ;

//private slots:

};

#endif // REVIEWOUTPUT_H
