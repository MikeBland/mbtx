#include "printdialog.h"
#include "ui_printdialog.h"
#include "pers.h"
#include "file.h"
#include "helpers.h"
#include <QtGui>
#include <QPrinter>
#include <QPrintDialog>

printDialog::printDialog(QWidget *parent, EEGeneral *gg, SKYModelData *gm, struct t_radioData *radioData) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
    ui(new Ui::printDialog)
{
    ui->setupUi(this);
    g_model = gm;
    g_eeGeneral = gg;
    te = ui->textEdit;
    rData = radioData ;

    setWindowTitle(tr("Setup for: ") + getModelName());
    ui->textEdit->clear();

    printTitle();

    printSetup();
    printExpo();
    printMixes();
    printModes();
    printLimits();
    printCurves();
    printSwitches();
    printSafetySwitches();
		printVoice() ;

    te->scrollToAnchor("1");
}

printDialog::~printDialog()
{
    delete ui;
}

QString doTC(const QString s, const QString color="", bool bold=false)
{
    QString str = s;
    if(bold) str = "<b>" + str + "</b>";
    if(!color.isEmpty()) str = "<font color=" + color + ">" + str + "</font>";
    return "<td>" + str + "</td>";
}

QString printDialog::getModelName()
{
    char buf[sizeof(g_model->name)+1];
    memcpy(&buf,&g_model->name,sizeof(g_model->name));
    buf[sizeof(g_model->name)]=0;
    return QString(buf);
}

QString printDialog::fv(const QString name, const QString value)
{
    return "<b>" + name + ": </b><font color=green>" + value + "</font><br>";
}

QString printDialog::getTimer( uint8_t timer )
{
    QString str = ", " + g_model->timer[timer].tmrDir==0 ? ", Count Down" : " Count Up";
    return tr("%1:%2, ").arg(g_model->timer[timer].tmrVal/60, 2, 10, QChar('0')).arg(g_model->timer[0].tmrVal%60, 2, 10, QChar('0')) + getTimerMode(g_model->timer[timer].tmrModeA) + str;
}

QString printDialog::getProtocol()
{
    QString str;
    str = QString("PPM   SILV_ASILV_BSILV_CTRAC09").mid(g_model->protocol*6,6).replace(" ","");

    if(!g_model->protocol) //ppm protocol
        str.append(tr(": %1 Channels, %3msec Delay").arg(g_model->ppmNCH*2+8).arg(300+g_model->ppmDelay*50));

    return str;
}

QString printDialog::getCenterBeep()
{
    //RETA123
    QStringList strl;

    if(g_model->beepANACenter & 0x01) strl << "Rudder";
    if(g_model->beepANACenter & 0x02) strl << "Elevator";
    if(g_model->beepANACenter & 0x04) strl << "Throttle";
    if(g_model->beepANACenter & 0x08) strl << "Aileron";
    if(g_model->beepANACenter & 0x10) strl << "P1";
    if(g_model->beepANACenter & 0x20) strl << "P2";
    if(g_model->beepANACenter & 0x40) strl << "P3";

    return strl.join(", ");

}

QString printDialog::getTrimInc()
{
    switch (g_model->trimInc)
    {
    case (1): return "Extra Fine"; break;
    case (2): return "Fine"; break;
    case (3): return "Medium"; break;
    case (4): return "Coarse"; break;
    default: return "Exponential"; break;
    }

}

void printDialog::printTitle()
{
    te->append(tr("<a name=1></a><h1>ERSKY9x Model: %1</h1><br>").arg(getModelName()));
}

void printDialog::printSetup()
{
    QString str = tr("<h2>General Model Settings</h2><br>");
    str.append(fv(tr("Name"), getModelName()));
    str.append(fv(tr("Timer1"), getTimer(0)));  //value, mode, count up/down
    str.append(fv(tr("Timer2"), getTimer(1)));  //value, mode, count up/down
    str.append(fv(tr("Protocol"), getProtocol())); //proto, numch, delay,
    str.append(fv(tr("Pulse Polarity"), g_model->pulsePol ? "NEG" : "POS"));
    str.append(fv(tr("Throttle Trim"), g_model->thrTrim ? tr("Enabled") : tr("Disabled")));
    str.append(fv(tr("Throttle Expo"), g_model->thrExpo ? tr("Enabled") : tr("Disabled")));
    str.append(fv(tr("Trainer"), g_model->traineron ? tr("Enabled") : tr("Disabled")));
    str.append(fv(tr("Trim Switch"), getSWName(g_model->trimSw,0)));
    str.append(fv(tr("Trim Increment"), getTrimInc()));
    str.append(fv(tr("Extended Limits"), g_model->extendedLimits ? tr("Enabled") : tr("Disabled")));
    str.append(fv(tr("Throttle Reverse"), g_model->throttleReversed ? tr("Reversed") : tr("Normal")));
    str.append(fv(tr("Auto Limits"), tr(" %1").arg((double)g_model->sub_trim_limit/10.0 ) ));
    str.append(fv(tr("Center Beep"), getCenterBeep())); // specify which channels beep
    str.append("<br><br>");
    te->append(str);


}

extern uint8_t stickScramble[] ;

void printDialog::printExpo()
{
    QString str = tr("<h2>Expo/Dr Settings</h2>");

    for(int i=0; i<4; i++)
    {
        str.append("<h3>" + getSourceStr(1, i+1, g_model->modelVersion, 0, 0 ) + "</h3>");
        str.append(fv(tr("Switch 1:"), getSWName(g_model->expoData[i].drSw1,0)));
        str.append(fv(tr("Switch 2:"), getSWName(g_model->expoData[i].drSw2,0)));
        str.append("<table border=1 cellspacing=0 cellpadding=3>");

        str.append("<tr>");
        str.append(doTC("&nbsp;"));
        str.append(doTC(tr("Expo Left"), "", true));
        str.append(doTC(tr("D/R Left"), "", true));
        str.append(doTC(tr("D/R Right"), "", true));
        str.append(doTC(tr("Expo Right"), "", true));
        str.append("</tr>");


        str.append("<tr>");
        str.append(doTC(tr("High"), "", true));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_HIGH][DR_EXPO][DR_LEFT]),"green"));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_HIGH][DR_WEIGHT][DR_LEFT]+100),"green"));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_HIGH][DR_WEIGHT][DR_RIGHT]+100),"green"));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_HIGH][DR_EXPO][DR_RIGHT]),"green"));
        str.append("</tr>");

        str.append("<tr>");
        str.append(doTC(tr("Mid"), "", true));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_MID][DR_EXPO][DR_LEFT]),"green"));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_MID][DR_WEIGHT][DR_LEFT]+100),"green"));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_MID][DR_WEIGHT][DR_RIGHT]+100),"green"));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_MID][DR_EXPO][DR_RIGHT]),"green"));
        str.append("</tr>");

        str.append("<tr>");
        str.append(doTC(tr("Low"), "", true));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_LOW][DR_EXPO][DR_LEFT]),"green"));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_LOW][DR_WEIGHT][DR_LEFT]+100),"green"));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_LOW][DR_WEIGHT][DR_RIGHT]+100),"green"));
        str.append(doTC(QString::number(g_model->expoData[i].expo[DR_LOW][DR_EXPO][DR_RIGHT]),"green"));
        str.append("</tr>");

        str.append("</table>");
    }
    str.append("<br><br>");
    te->append(str);
}

void printDialog::printMixes()
{
    uint32_t j ;
    QString str = tr("<h2>Mixers</h2><br>");

    int lastCHN = 0;
    for(int i=0; i<MAX_SKYMIXERS; i++)
    {
        SKYMixData *md = &g_model->mixData[i];
        if(!md->destCh) break;

        str.append("<font size=+1 face='Courier New'>");
        if(lastCHN!=md->destCh)
        {
            lastCHN++;
            str.append(tr("<b>CH%1</b>").arg(lastCHN,2,10,QChar('0')));
        }
        else
            str.append("&nbsp;&nbsp;&nbsp;&nbsp;");

        str.append("</font>");
        if(lastCHN!=md->destCh)
        {
            str.append("<br>");
            lastCHN++;
            for (int k=lastCHN; k<md->destCh; k++) {
                str.append(tr("<font size=+1 face='Courier New'><b>CH%1</b><br></font>").arg(lastCHN,2,10,QChar('0')));
                lastCHN++;        
            }   
            str.append(tr("<font size=+1 face='Courier New'><b>CH%1</b></font>").arg(lastCHN,2,10,QChar('0')));
        } 

        str.append("<font size=+1 face='Courier New' color=green>");

        switch(md->mltpx)
        {
            case (1): str += "&nbsp;*"; break;
            case (2): str += "&nbsp;R"; break;
            default:  str += "&nbsp;&nbsp;"; break;
        };

        str += md->weight<0 ? tr(" %1\%").arg(md->weight).rightJustified(6,' ') :
                              tr(" +%1\%").arg(md->weight).rightJustified(6, ' ');


        //QString srcStr = SRC_STR;
        //str += " " + srcStr.mid(CONVERT_MODE(md->srcRaw+1)*4,4);
        str += getSourceStr(g_eeGeneral->stickMode,md->srcRaw, g_model->modelVersion, 0, 0);

        if(md->swtch) str += tr(" Switch(") + getSWName(md->swtch,0) + ")";
        if(md->carryTrim) str += tr(" noTrim");
        if(md->sOffset)  str += tr(" Offset(%1\%)").arg(md->sOffset);
        if(md->curve)
        {
            QString crvStr = CURV_STR;
            crvStr=crvStr.mid(md->curve*3,3);
            crvStr.replace(QString("<") ,QString("&lt;"));
            str += tr(" Curve(%1)").arg(crvStr.remove(' '));
        }

				str += tr(" Modes(") ;
				uint8_t b = 1 ;
				uint8_t z = md->modeControl ;
        for ( j = 0 ; j<MAX_MODES+1 ; j++ )
				{
          str += ( ( z & b ) == 0 ) ? tr("%1").arg(j) : tr(" ") ;
					b <<= 1 ;
				}
				str += tr(")") ;

        if(md->delayDown || md->delayUp) str += tr(" Delay(u%1:d%2)").arg((qreal)md->delayDown/10.0).arg((qreal)md->delayUp/10.0);
        if(md->speedDown || md->speedUp) str += tr(" Slow(u%1:d%2)").arg((qreal)md->speedUp/10.0).arg((qreal)md->speedDown/10.0);

        if(md->mixWarn)  str += tr(" Warn(%1)").arg(md->mixWarn);

        str.append("</font><br>");
    }

//    for(int j=lastCHN; j<NUM_SKYCHNOUT+EXTRA_SKYCHANNELS; j++)
//    {
//        str.append("<font size=+1 face='Courier New'>");
//        str.append(tr("<b>CH%1</b>").arg(j+1,2,10,QChar('0')));
//        str.append("</font><br>");
//    }
    str.append("<br><br>");
    te->append(str);
}

void printDialog::printLimits()
{
    QString str = tr("<h2>Limits</h2>");

    str.append("<table border=1 cellspacing=0 cellpadding=3>");
    str.append("<tr><td>&nbsp;</td><td><b>Offset</b></td><td><b>Min</b></td><td><b>Max</b></td><td><b>Invert</b></td></tr>");
    for(int i=0; i<NUM_SKYCHNOUT; i++)
    {
        str.append("<tr>");

        str.append(doTC(tr("CH%1").arg(i+1,2,10,QChar('0')),"",true));
        str.append(doTC(QString::number((qreal)g_model->limitData[i].offset/10, 'f', 1),"green"));
        str.append(doTC(QString::number(g_model->limitData[i].min-100),"green"));
        str.append(doTC(QString::number(g_model->limitData[i].max+100),"green"));
        str.append(doTC(QString(g_model->limitData[i].revert ? "INV" : "NOR"),"green"));
        str.append("</tr>");
    }
    str.append("</table>");

    str.append("<br><br>");
    te->append(str);
}

void printDialog::printModes()
{
//PACK(typedef struct t_PhaseData {
//  int16_t trim[4];     // -500..500 => trim value, 501 => use trim of phase 0, 502, 503, 504 => use trim of phases 1|2|3|4 instead
//  int8_t swtch;       // swtch of phase[0] is not used
//  char name[6];
//  uint8_t fadeIn:4;
//  uint8_t fadeOut:4;
//  int8_t swtch2;       // swtch of phase[0] is not used
//	uint8_t spare ;		// Future expansion
//}) PhaseData;
	int16_t value ;
	QString str = tr("<h2>Flight Modes</h2><br>");
  str.append("Mode 0               RETA<br>") ;

	for(int i=1; i<=MAX_MODES; i++)
	{
		PhaseData *p = &g_model->phaseData[i-1] ;
  	str.append( tr("Mode %1 ").arg(i) ) ;

    char buf[10] ;
		buf[0] = '"' ;
    memcpy( &buf[1], &p->name, 6 ) ;
    buf[7] = '"' ;
    buf[8] = '\0' ;
		str.append( buf ) ;
		
    str.append("<font color=green>");
		
		str.append( tr(" Sw1(") + getSWName(p->swtch,0) + ")" ) ;
		str.append( tr(" Sw2(") + getSWName(p->swtch2,0) + ") " ) ;

		value = p->trim[0] - (TRIM_EXTENDED_MAX+1) ;
		str.append( value < 0 ? "R" : tr("%1").arg(value) ) ;
		value = p->trim[1] - (TRIM_EXTENDED_MAX+1) ;
		str.append( value < 0 ? "E" : tr("%1").arg(value) ) ;
		value = p->trim[2] - (TRIM_EXTENDED_MAX+1) ;
		str.append( value < 0 ? "T" : tr("%1").arg(value) ) ;
		value = p->trim[3] - (TRIM_EXTENDED_MAX+1) ;
		str.append( value < 0 ? "A" : tr("%1").arg(value) ) ;
		str.append( tr(" Fade In(%1) ").arg((qreal)p->fadeIn/2.0) ) ;
		str.append( tr(" Fade Out(%1)").arg((qreal)p->fadeOut/2.0) ) ;
		str.append("</font><br>");
	}
	str.append("<br><br>");
	te->append(str);
}


void printDialog::printCurves()
{
    QString str = tr("<h2>Curves</h2>");

    str.append(fv(tr("5-point Curves"), ""));
    str.append("<table border=1 cellspacing=0 cellpadding=3>");
    str.append("<tr>");
    str.append(doTC("&nbsp;"));
    for(int i=0; i<5; i++) str.append(doTC(tr("pt %1").arg(i+1), "", true));
    str.append("</tr>");
    for(int i=0; i<MAX_CURVE5; i++)
    {
        str.append("<tr>");
        str.append(doTC(tr("Curve %1").arg(i+1), "", true));
        for(int j=0; j<5; j++)
            str.append(doTC(QString::number(g_model->curves5[i][j]),"green"));
        str.append("</tr>");
    }
    str.append("</table>");
    str.append("<br><br>");


    str.append(fv(tr("9-point Curves"), ""));
    str.append("<table border=1 cellspacing=0 cellpadding=3>");
    str.append("<tr>");
    str.append(doTC("&nbsp;"));
    for(int i=0; i<9; i++) str.append(doTC(tr("pt %1").arg(i+1), "", true));
    str.append("</tr>");
    for(int i=0; i<MAX_CURVE9; i++)
    {
        str.append("<tr>");
        str.append(doTC(tr("Curve %1").arg(i+1), "", true));
        for(int j=0; j<9; j++)
            str.append(doTC(QString::number(g_model->curves9[i][j]),"green"));
        str.append("</tr>");
    }
    str.append("</table>");



    str.append("<br><br>");
    te->append(str);
}

void printDialog::printSwitches()
{
    QString str = tr("<h2>Logical Switches</h2>");


    str.append("<table border=1 cellspacing=0 cellpadding=3>");
//    str.append("<tr>");
//    str.append(doTC("&nbsp;"));
//    str.append(doTC(tr("Source"), "", true));
//    str.append(doTC(tr("Offset"), "", true));
//    str.append(doTC(tr("Function"), "", true));
//    str.append("</tr>");
    for(int i=0; i<NUM_SKYCSW; i++)
    {
        str.append("<tr>");
        str.append(doTC(tr("SW%1").arg(i+1),"",true));

        QString tstr;

        if(g_model->customSw[i].func)
        {
            switch (CS_STATE(g_model->customSw[i].func, g_model->modelVersion))
            {
            case CS_VOFS:
                tstr = g_model->customSw[i].v1 ?
                       getSourceStr(g_eeGeneral->stickMode,g_model->customSw[i].v1, 0, 0, 0) :
                       "0";
                tstr.remove(" ");
                if(g_model->customSw[i].func==CS_APOS || g_model->customSw[i].func==CS_ANEG)
                    tstr = "|" + tstr + "|";

                if(g_model->customSw[i].func==CS_APOS || g_model->customSw[i].func==CS_VPOS)
                    tstr += " &gt; ";
                if(g_model->customSw[i].func==CS_ANEG || g_model->customSw[i].func==CS_VNEG)
                    tstr += " &lt; ";

                tstr += QString::number(g_model->customSw[i].v2);
                break;


            case CS_VBOOL:
                tstr = getSWName(g_model->customSw[i].v1,0);

                switch (g_model->customSw[i].func)
                {
                case CS_AND:
                    tstr += " AND ";
                    break;
                case CS_OR:
                    tstr += " OR ";
                    break;
                case CS_XOR:
                    tstr += " XOR ";
                    break;
                default:
                    break;
                }

                tstr += getSWName(g_model->customSw[i].v2,0);
                break;


            case CS_VCOMP:
                tstr = g_model->customSw[i].v1 ?
                       getSourceStr(g_eeGeneral->stickMode,g_model->customSw[i].v1, 0, 0, 0) :
                       "0";

                switch (g_model->customSw[i].func)
                {
                case CS_EQUAL:
                    tstr += " = ";
                    break;
                case CS_NEQUAL:
                    tstr += " != ";
                    break;
                case CS_GREATER:
                    tstr += " &gt; ";
                    break;
                case CS_LESS:
                    tstr += " &lt; ";
                    break;
                case CS_EGREATER:
                    tstr += " &gt;= ";
                    break;
                case CS_ELESS:
                    tstr += " &lt;= ";
                    break;
                default:
                    break;
                }

                tstr += g_model->customSw[i].v2 ?
                        getSourceStr(g_eeGeneral->stickMode,g_model->customSw[i].v2, 0, 0, 0) :
                        "0";
                break;
            default:
                break;
            }
        }
				if(g_model->customSw[i].andsw)
				{
					tstr += " AND(";
					tstr += getSWName(g_model->customSw[i].andsw,0);
					tstr += ")";
				}
				if(g_model->switchDelay[i])
				{
					tstr += " Delay(";
					tstr += QString::number((double)g_model->switchDelay[i]/10.0);
					tstr += ")";
				}
        str.append(doTC(tstr,"green"));
        str.append("</tr>");
    }
    str.append("</table>");


    str.append("<br><br>");
    te->append(str);
}


void printDialog::printVoice()
{
//	uint32_t j ;
	QString str = tr("<h2>Voice Alerts</h2><br>");
	
	for(int i = 0 ; i < NUM_SKY_VOICE_ALARMS+NUM_EXTRA_VOICE_ALARMS + NUM_GLOBAL_VOICE_ALARMS ; i++)
	{
    VoiceAlarmData *vad = ( i >= NUM_SKY_VOICE_ALARMS) ? &g_model->vadx[i-NUM_SKY_VOICE_ALARMS] : &g_model->vad[i] ;
    if ( i >= NUM_SKY_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS )
		{
      uint8_t z = i - ( NUM_SKY_VOICE_ALARMS + NUM_EXTRA_VOICE_ALARMS ) ;
			
      vad = &g_eeGeneral->gvad[z] ;
			str += tr("GVA%1  ").arg(z+1) ;
		}
		else
		{
			str += tr("VA%1%2  ").arg((i+1)/10).arg((i+1)%10) ;
		}
    str.append("<font color=green>");
		QString srcstr ;
		uint32_t value = vad->source ;
    uint32_t limit = 45 ;
    if ( rData->bitType & (RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7 | RADIO_BITTYPE_XLITE | RADIO_BITTYPE_T12 | RADIO_BITTYPE_XXX) )
		{
			limit = 46 ;
    	if ( rData->bitType & ( RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E ) )
			{
				limit = 47 ;
			}
			if ( value == EXTRA_POTS_START )
			{
        value = 8 ;
			}
			else
			{
    		if ( rData->bitType & ( RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E ) )
				{
					if ( value == EXTRA_POTS_START + 1 )
					{
        		value = 9 ;
					}
					else if ( value >= EXTRA_POTS_POSITION )
					{
						value += 2 ;
					}
				}
				else if ( value >= EXTRA_POTS_POSITION )
				{
					value += 1 ;
				}
			}
		}
		if ( value < limit )
		{
			int type = rData->type ;
			if ( type == RADIO_TYPE_TPLUS )
			{
				if ( rData->sub_type == 1 )
				{
					type = RADIO_TYPE_X9E ;
				}
			}
			str += tr("(%1) ").arg(getSourceStr(g_eeGeneral->stickMode,value,g_model->modelVersion, type, rData->extraPots )) ;
		}
		else
		{
      str += tr("(%1) ").arg(getTelemString(value-limit+1 )) ;
		}
    srcstr = "-------   v&gt;val  v&lt;val  |v|&gt;val|v|&lt;valv~=val    " ;
		str += tr("%1 ").arg(srcstr.mid( vad->func * 10, 10 )) ;
  	if ( vad->source > 44 )
		{
			char telText[20] ;
      stringTelemetryChannel( telText, vad->source - 45, vad->offset, g_model ) ;
			str += tr("(%1) ").arg(telText) ;
		}
		else
		{
			str += tr("(%1) ").arg(vad->offset) ;
		}
		str += tr("Switch(%1) ").arg(getSWName(vad->swtch, rData->type)) ;
		if ( vad->rate < 4 )
		{
			srcstr = (vad->rate > 1 ) ? (vad->rate == 2 ? "BOTH " : "ALL ") : (vad->rate == 0 ? "ON " : "OFF " ) ;
			str += srcstr ;
		}
		else if ( vad->rate > 32 )
		{
			str += "ONCE " ;
		}
		else
		{
			str += tr("Rate(%1) ").arg(vad->rate-2) ;
		}
		if ( vad->mute )
		{
			str += tr("Mute ") ;
		}
		if ( vad->haptic )
		{
			str += tr("Haptic(%1) ").arg(vad->haptic) ;
		}
		if ( vad->vsource )
		{
			srcstr = ( vad->vsource == 1 ) ? "Before" : "After " ;
			str += tr("PlaySrc(%1) ").arg(srcstr) ;
		}
		switch ( vad->fnameType )
		{
			case 1 :
      {
        QString xstr = (char *)vad->file.name ;
				xstr = xstr.left(8) ;
        str += tr("File(%1)").arg(xstr ) ;
      }
			break ;
			case 2 :
				str += tr("File(%1)").arg(vad->file.vfile ) ;
			break ;
			case 3 :
				str += tr("Alarm(%1)").arg(getAudioAlarmName(vad->file.vfile) ) ;
			break ;
		}
		if ( vad->delay )
		{
      str += tr(" delay(%1)").arg((float)vad->delay / 10.0) ;
		}

		 
		str.append("</font><br>");
	}
	
	str.append("<br><br>");
  te->append(str);
}


QString ssTypes[4] = {
	"S", "A", "V", "X"
} ;

QString ssAnames[3] = {
" 8 secs",
"12 secs",
"16 secs"
} ;

void printDialog::printSafetySwitches()
{
    QString str = tr("<h2>Safety Switches</h2>");


    str.append("<table border=1 cellspacing=0 cellpadding=3>");
    str.append("<tr>");
    str.append(doTC("&nbsp;"));
    str.append(doTC(tr("Type"), "", true));
    str.append(doTC(tr("Switch"), "", true));
    str.append(doTC(tr("Value"), "", true));
    str.append("</tr>");
    for(int i=0; i<NUM_SKYCHNOUT; i++)
    {
        str.append("<tr>");
        str.append(doTC(tr("CH%1").arg(i+1),"",true));
				int x = g_model->safetySw[i].opt.ss.mode ;
        str.append(doTC(ssTypes[x],"green"));
        if ( ( x ==0 ) || ( x == 3 ) ) // 'S' or 'X'
				{
					str.append(doTC(getSWName(g_model->safetySw[i].opt.ss.swtch,0),"green"));
        	str.append(doTC(QString::number(g_model->safetySw[i].opt.ss.val),"green"));
				}
				else if ( x == 1 )	// 'A'
				{
					str.append(doTC(getSWName(g_model->safetySw[i].opt.ss.swtch,0),"green"));
          str.append(doTC(getAudioAlarmName(g_model->safetySw[i].opt.ss.val),"green"));
				}
				else // 'V'
				{
					x = g_model->safetySw[i].opt.ss.swtch ;
					if ( ( x >= 35 ) && ( x <= 37 ) )
					{
						// 35-37 8, 12, 16 secs
						str.append(doTC(ssAnames[x-35],"green"));
	          str.append(doTC(getTelemString( g_model->safetySw[i].opt.ss.val+1),"green"));
					}
					else
					{
						str.append(doTC(getSWName(x,0),"green"));
	        	str.append(doTC(QString::number(g_model->safetySw[i].opt.ss.val+128),"green"));
					}
				}
        str.append("</tr>");
    }
    str.append("</table>");


    str.append("<br><br>");
    te->append(str);
}


void printDialog::on_printButton_clicked()
{
    QPrinter printer;

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return;

    te->print(&printer);
}
