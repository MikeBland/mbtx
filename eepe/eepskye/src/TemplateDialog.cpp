#include "modeledit.h"
#include "TemplateDialog.h"
#include "ui_TemplateDialog.h"
#include "pers.h"
#include "helpers.h"


TemplateDialog::TemplateDialog(QWidget *parent, SKYModelData *g_model, struct t_templateValues *values, int eeType) :
    QDialog(parent),
    ui(new Ui::TemplateDialog)
{
  lvalues = values ;
    ui->setupUi(this);
		ui->SourceCB->setCurrentIndex(values->stick-1) ;
		ui->OutputSB->setValue(values->outputChannel) ;
		ui->HelperSB->setValue(values->helperChannel) ;
		ui->Switch1CB->setCurrentIndex(values->switch1-1 ) ;
		ui->Switch2CB->setCurrentIndex(values->switch2-1 ) ;
		ui->Switch3CB->setCurrentIndex(values->switch3-1 ) ;
		
		connect( ui->SourceCB, SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
		connect( ui->OutputSB, SIGNAL(editingFinished()),this,SLOT(valuesChanged()));
		connect( ui->HelperSB, SIGNAL(editingFinished()),this,SLOT(valuesChanged()));
    connect( ui->Switch1CB, SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
    connect( ui->Switch2CB, SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
    connect( ui->Switch3CB, SIGNAL(currentIndexChanged(int)),this,SLOT(valuesChanged()));
}

TemplateDialog::~TemplateDialog()
{
    delete ui;
}


void TemplateDialog::valuesChanged()
{
	lvalues->stick = ui->SourceCB->currentIndex() + 1 ;
	lvalues->outputChannel = ui->OutputSB->value() ;
	lvalues->helperChannel = ui->HelperSB->value() ;
	lvalues->switch1 = ui->Switch1CB->currentIndex() + 1 ;
	lvalues->switch2 = ui->Switch2CB->currentIndex() + 1 ;
	lvalues->switch3 = ui->Switch3CB->currentIndex() + 1 ;
}





