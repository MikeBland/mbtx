#ifndef VOICEALARM_H
#define VOICEALARM_H

#include <QDialog>
#include "pers.h"
#include "myeeprom.h"

namespace Ui {
    class VoiceAlarmDialog ;
}

class VoiceAlarmDialog : public QDialog {
    Q_OBJECT
public:
	VoiceAlarmDialog(QWidget *parent, VoiceAlarmData *invad, int eeType, int stickmode, int modelVersion, SKYModelData *pModel ) ;
  ~VoiceAlarmDialog();

//    QString getComment();

//protected:
//    void changeEvent(QEvent *e);

private slots:
    void valuesChanged();


private:
	VoiceAlarmData *vad ;
  Ui::VoiceAlarmDialog *ui;
	int leeType ;
	SKYModelData *lpModel ;
	void updateDisplay( void ) ;
};

#endif // VOICEALARM_H

