#ifndef TEMPLATEDIALOG_H
#define TEMPLATEDIALOG_H

#include <QDialog>
#include "modeledit.h"
#include "pers.h"

namespace Ui {
    class TemplateDialog;
}

class TemplateDialog : public QDialog {
    Q_OBJECT
public:
    TemplateDialog(QWidget *parent, SKYModelData *g_model, struct t_templateValues *values, int eeType);
    ~TemplateDialog();

//    QString getComment();

//protected:

private slots:
    void valuesChanged();
//		void updateChannels() ;


private:
//    SKYMixData *md;
    Ui::TemplateDialog *ui;
//    QString * mixCommennt;
		struct t_templateValues *lvalues ;
    bool ValuesEditLock ;
};

#endif // TEMPLATEDIALOG_H
