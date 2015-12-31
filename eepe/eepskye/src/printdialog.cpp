#include "printdialog.h"
#include "ui_printdialog.h"
#include "pers.h"
#include "helpers.h"
#include <QtGui>

printDialog::printDialog(QWidget *parent, EEGeneral *gg, SKYModelData *gm) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
    ui(new Ui::printDialog)
{
    ui->setupUi(this);
    g_model = gm;
    g_eeGeneral = gg;
    te = ui->textEdit;

    setWindowTitle(tr("Setup for: ") + getModelName());
    ui->textEdit->clear();

    printTitle();

    printSetup();
    printExpo();
    printMixes();
    printLimits();
    printCurves();
    printSwitches();
    printSafetySwitches();

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
        str.append("<h3>" + getSourceStr(1, i+1, g_model->modelVersion, 0 ) + "</h3>");
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
        str += getSourceStr(g_eeGeneral->stickMode,md->srcRaw, g_model->modelVersion, 0);

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

        if(md->delayDown || md->delayUp) str += tr(" Delay(u%1:d%2)").arg(md->delayUp).arg(md->delayDown);
        if(md->speedDown || md->speedUp) str += tr(" Slow(u%1:d%2)").arg(md->speedUp).arg(md->speedDown);

        if(md->mixWarn)  str += tr(" Warn(%1)").arg(md->mixWarn);

        str.append("</font><br>");
    }

    for(int j=lastCHN; j<NUM_SKYCHNOUT; j++)
    {
        str.append("<font size=+1 face='Courier New'>");
        str.append(tr("<b>CH%1</b>").arg(j+1,2,10,QChar('0')));
        str.append("</font><br>");
    }
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
    QString str = tr("<h2>Custom Switches</h2>");


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
                       getSourceStr(g_eeGeneral->stickMode,g_model->customSw[i].v1, 0, 0) :
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
                       getSourceStr(g_eeGeneral->stickMode,g_model->customSw[i].v1, 0, 0) :
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
                        getSourceStr(g_eeGeneral->stickMode,g_model->customSw[i].v2, 0, 0) :
                        "0";
                break;
            default:
                break;
            }
        }

        str.append(doTC(tstr,"green"));
        str.append("</tr>");
    }
    str.append("</table>");


    str.append("<br><br>");
    te->append(str);
}

void printDialog::printSafetySwitches()
{
    QString str = tr("<h2>Safety Switches</h2>");


    str.append("<table border=1 cellspacing=0 cellpadding=3>");
    str.append("<tr>");
    str.append(doTC("&nbsp;"));
    str.append(doTC(tr("Switch"), "", true));
    str.append(doTC(tr("Value"), "", true));
    str.append("</tr>");
    for(int i=0; i<NUM_SKYCHNOUT; i++)
    {
        str.append("<tr>");
        str.append(doTC(tr("CH%1").arg(i+1),"",true));
        str.append(doTC(getSWName(g_model->safetySw[i].opt.ss.swtch,0),"green"));
        str.append(doTC(QString::number(g_model->safetySw[i].opt.ss.val,0),"green"));
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
