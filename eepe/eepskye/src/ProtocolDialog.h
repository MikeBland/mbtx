#ifndef PROTOCOLDIALOG_H
#define PROTOCOLDIALOG_H

#include <QDialog>
#include "pers.h"

namespace Ui {
    class ProtocolDialog ;
}

class ProtocolDialog : public QDialog {
    Q_OBJECT
public:
		ProtocolDialog(QWidget *parent, uint32_t module, struct t_radioData *radioData, struct t_module *pmodule, uint32_t modelVersion ) ;
    ~ProtocolDialog();

//    QString getComment();

protected:
	void setBoxes() ;
	uint32_t hasFailsafe(void) ;

//    void changeEvent(QEvent *e);

private slots:
	void on_ProtocolCB_currentIndexChanged(int index) ;
	void on_xjtCountryCB_currentIndexChanged(int index) ;
	void on_xjtChannelsCB_currentIndexChanged(int index) ;
	void on_ppmFrameLengthCB_currentIndexChanged(int index) ;
	void on_ppmDelayCB_currentIndexChanged(int index) ;
	void on_startChannelSB_valueChanged(int value) ;
	void on_rxNumberSB_valueChanged(int value) ;
	void on_xjtTypeCB_currentIndexChanged(int index) ;
	void on_ppmPolarityCB_currentIndexChanged(int index) ;
	void on_channelsSB_valueChanged(int value) ;
	void on_dsmTypeCB_currentIndexChanged(int index) ;
	void on_autobindCB_currentIndexChanged(int index) ;
	void on_powerCB_currentIndexChanged(int index) ;
	void on_optionSB_valueChanged(int value) ;
	void on_multiTypeCB_currentIndexChanged(int index) ;
	void on_multiSubProtocolCB_currentIndexChanged(int value) ;
	void on_rateCB_currentIndexChanged(int index) ;
  void on_FailsafeCB_currentIndexChanged(int index) ;
//	void on_RepeatSendCB_toggled(bool checked) ;
	void on_R9MpowerCB_currentIndexChanged(int index) ;

//    void valuesChanged();
//		void updateChannels() ;


private:
//    SKYMixData *md;
	bool protocolEditLock ;

    Ui::ProtocolDialog *ui;
		struct t_module *ppd ;
    struct t_radioData *rData ;
//    QString * mixCommennt;
		uint32_t lModule ;
		uint32_t lVersion ;
//		uint32_t lextraPots ;
//    bool ValuesEditLock ;
};

#endif // PROTOCOLDIALOG_H
