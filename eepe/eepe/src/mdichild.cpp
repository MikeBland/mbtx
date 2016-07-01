/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include "mdichild.h"
#include "pers.h"
#include "modeledit.h"
#include "generaledit.h"
#include "avroutputdialog.h"
#include "burnconfigdialog.h"
#include "simulatordialog.h"
#include "printdialog.h"
#include "wizarddialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>

extern class simulatorDialog *SimPointer ;

MdiChild::MdiChild()
{
    setAttribute(Qt::WA_DeleteOnClose);
    //setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint);

    this->setFont(QFont("Courier New",12));
    refreshList();
    if(!(this->isMaximized() || this->isMinimized())) this->adjustSize();
    isUntitled = true;

	radioData.valid = 0 ;
	uint32_t i ;
	for ( i = 1 ; i <= MAX_MODELS ; i += 1 )
	{
    radioData.ModelNames[i][0] = '\0' ;
	}
	radioData.type = 0 ;
//	generalDefault() ;


    connect(this, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(OpenEditWindow()));
//    connect(this, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(wizardEdit()));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(ShowContextMenu(const QPoint&)));
    connect(this,SIGNAL(currentRowChanged(int)), this,SLOT(viableModelSelected(int)));

    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

}

void MdiChild::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();

    QListWidget::mousePressEvent(event);
}

void MdiChild::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);

    QByteArray gmData;
    doCopy(&gmData);

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-eepe", gmData);

    drag->setMimeData(mimeData);

    //Qt::DropAction dropAction =
            drag->exec(Qt::CopyAction);// | Qt::MoveAction);

    //if(dropAction==Qt::MoveAction)

    QListWidget::mouseMoveEvent(event);
}

void MdiChild::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-eepe"))
    {
         event->acceptProposedAction();
         clearSelection();
         itemAt(event->pos())->setSelected(true);
    }
		else if (event->mimeData()->hasFormat("text/uri-list"))
		{
	    event->acceptProposedAction();
		}
}

void MdiChild::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-eepe"))
    {
         event->acceptProposedAction();
         clearSelection();
         itemAt(event->pos())->setSelected(true);
    }
}

void MdiChild::dropEvent(QDropEvent *event)
{
    int i = this->indexAt(event->pos()).row();
    //QMessageBox::warning(this, tr("eePe"),tr("Index :%1").arg(i));
    if(event->mimeData()->hasFormat("application/x-eepe"))
    {
        QByteArray gmData = event->mimeData()->data("application/x-eepe");
        doPaste(&gmData,i);
    }
		else
		{
		  QList<QUrl> urls = event->mimeData()->urls();
  		if (urls.isEmpty())
    		return ;
			QString fileName = urls.first().toLocalFile();
  		if (fileName.isEmpty())
  		  return ;
			setCurrentRow( i ) ;
      loadModelFromFile( fileName ) ;
		}
    event->acceptProposedAction();
}

void MdiChild::refreshList()
{
    clear();
    char buf[20] = {0};

    eeFile.eeLoadOwnerName(buf,sizeof(buf));
    QString str = QString(buf).trimmed();
    if(!str.isEmpty())
        str.prepend("-");
    
		QString efree = tr(" free %1").arg(free());

    addItem(tr("General Settings") + str + efree );

    for(uint8_t i=0; i<MAX_MODELS; i++)
    {
            eeFile.eeLoadModelName(i,buf,sizeof(buf));
            addItem(QString(buf));
    }
    if (radioData.generalSettings.currModel < 60 )
		{
			QFont f = QFont("Courier New", 12) ;
			f.setBold(true) ;
      this->item(radioData.generalSettings.currModel+1)->setFont(f) ;
    }
}

void MdiChild::cut()
{
    copy();
    deleteSelected(false);
}

void MdiChild::deleteSelected(bool ask=true)
{
    QMessageBox::StandardButton ret = QMessageBox::No;

    if(ask)
        ret = QMessageBox::warning(this, "eePe",
                 tr("Delete Selected Models?"),
                 QMessageBox::Yes | QMessageBox::No);


    if ((ret == QMessageBox::Yes) || (!ask))
    {
           foreach(QModelIndex index, this->selectionModel()->selectedIndexes())
           {
               if(index.row()>0)eeFile.DeleteModel(index.row());
           }
           setModified();
    }
}

QString MdiChild::modelName(int id)
{
    if(eeFile.eeModelExists(id))
    {
        char buf[sizeof(ModelData().name)+1];
        eeFile.getModelName(id,(char*)&buf);
        return QString(buf).trimmed();
    }
    else
        return "";
}

QString MdiChild::ownerName()
{
    EEGeneral tgen;
    eeFile.getGeneralSettings(&tgen);
    return QString::fromLatin1(tgen.ownerName,sizeof(tgen.ownerName)).trimmed();
}

int MdiChild::eepromVersion()
{
    EEGeneral tgen;
    eeFile.getGeneralSettings(&tgen);
    return tgen.myVers;
}

int MdiChild::eesize()
{
    return eeFile.eesize();
}

void MdiChild::doCopy(QByteArray *gmData)
{
    foreach(QModelIndex index, this->selectionModel()->selectedIndexes())
    {
        if(!index.row())
        {
            EEGeneral tgen;
            if(eeFile.getGeneralSettings(&tgen))
            {
                gmData->append('G');
                gmData->append((char*)&tgen,sizeof(tgen));
            }
        }
        else
        {
            ModelData tmod;
            if(eeFile.getModel(&tmod,index.row()-1))
            {
							if ( eeFile.mee_type )
							{
                gmData->append('M'+0x80);
							}
							else
							{
                gmData->append('M');
							}
                gmData->append((char*)&tmod,sizeof(tmod));
            }
        }
    }
}

void MdiChild::copy()
{
    QByteArray gmData;
    doCopy(&gmData);

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-eepe", gmData);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData,QClipboard::Clipboard);
}

void MdiChild::doPaste(QByteArray *gmData, int index)
{
    //QByteArray gmData = mimeD->data("application/x-eepe");
    char *gData = gmData->data();//new char[gmData.size() + 1];
    int i = 0;
    int id = index;
    if(!id) id++;

    while((i<gmData->size()) && (id<=MAX_MODELS))
    {
        char c = *gData;
        i++;
        gData++;
        if(c=='G')  //general settings
        {
            if(!eeFile.putGeneralSettings((EEGeneral*)gData))
            {
                QMessageBox::critical(this, tr("Error"),tr("Unable set data!"));
                break;
            }
            gData += sizeof(EEGeneral);
            i     += sizeof(EEGeneral);
        }
        else if ( ( c & 0x7F) == 'M' ) //model data
        {
            if(!eeFile.putModel((ModelData*)gData,id-1))
            {
                QMessageBox::critical(this, tr("Error"),tr("Unable set model!"));
                break;
            }
            gData += sizeof(ModelData);
            i     += sizeof(ModelData);
            id++;
        }
				else
				{
          QMessageBox::critical(this, tr("Error"),tr("Data not Recognised"));
					break ;
				}
    }
    setModified();
}

bool MdiChild::hasPasteData()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    return mimeData->hasFormat("application/x-eepe");
}

void MdiChild::paste()
{
    if(hasPasteData())
    {
        const QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();

        QByteArray gmData = mimeData->data("application/x-eepe");
        doPaste(&gmData,this->currentRow());
    }

}


bool MdiChild::loadModelFromFile(QString fn)
{
    int cmod = currentRow()-1;
    bool genfile = currentRow()==0;

    QString fileName;
    QSettings settings("er9x-eePe", "eePe");


    if(!fn.isEmpty())
    {
        if(!QFileInfo(fn).exists())
        {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Coulden't find %1")
                                  .arg(fn));
            return false;
        }

        fileName = fn;
    }
    else if(genfile)
    {
        char buf[sizeof(EEGeneral().ownerName)+1];

        eeFile.eeLoadOwnerName(buf,sizeof(buf));
        QString str = QString(buf).trimmed();
        if(!str.isEmpty())
        {
            int ret = QMessageBox::warning(this, "eePe",
                                           tr("Overwrite Current Settings?"),
                                           QMessageBox::Yes | QMessageBox::No);
            if(ret != QMessageBox::Yes)
                return false;
        }

        //get file to load
        fileName = QFileDialog::getOpenFileName(this,tr("Open"),settings.value("lastDir").toString(),tr(EEPG_FILES_FILTER));
    }
    else
    {
        //if slot is used - confirm overwrite
        if(eeFile.eeModelExists(cmod))
        {
            char buf[sizeof(ModelData().name)+1];
            eeFile.getModelName(cmod,(char*)&buf);
            QString cmodelName = QString(buf).trimmed();
            int ret = QMessageBox::warning(this, "eePe",
                                           tr("Overwrite %1?").arg(cmodelName),
                                           QMessageBox::Yes | QMessageBox::No);
            if(ret != QMessageBox::Yes)
                return false;
        }

        //get file to load
        fileName = QFileDialog::getOpenFileName(this,tr("Open"),settings.value("lastDir").toString(),tr(EEPM_FILES_FILTER));
    }

    if (fileName.isEmpty())
        return false;

    settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

//    quint8 temp[WRITESIZE];

    if(genfile)
    {
        EEGeneral tgen;

        //get general data from XML file, if not, get it from iHEX

        QDomDocument doc(ER9X_EEPROM_FILE_TYPE);
        QFile file(fileName);
        bool xmlOK = file.open(QIODevice::ReadOnly);
        if(xmlOK)
        {
            xmlOK = doc.setContent(&file);
            if(xmlOK)
            {
                xmlOK = loadGeneralDataXML(&doc, &tgen);
            }
            file.close();
        }

        if(!xmlOK) //if can't get XML - load iHEX
        {
            quint8 temp[sizeof(EEGeneral)];
            if(!loadiHEX(this, fileName, (quint8*)&temp, sizeof(EEGeneral), EEPE_GENERAL_FILE_HEADER))
                return false;
            memcpy(&tgen, &temp, sizeof(tgen));
        }

        if(!eeFile.putGeneralSettings(&tgen))
        {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Error writing to container"));
            return false;
        }
    }
    else
    {
        ModelData tmod;

        QDomDocument doc(ER9X_EEPROM_FILE_TYPE);
        QFile file(fileName);
        bool xmlOK = file.open(QIODevice::ReadOnly);
        if(xmlOK)
        {
            xmlOK = doc.setContent(&file);
            if(xmlOK)
            {
                xmlOK = loadModelDataXML(&doc, &tmod);
                getNotesFromXML(&doc, cmod);
            }
            file.close();
        }

        if(!xmlOK) //if can't get XML - load iHEX
        {

            //if can't load XML load from iHex
            quint8 temp[sizeof(ModelData)];
            if(!loadiHEX(this, fileName, (quint8*)&temp, sizeof(ModelData), EEPE_MODEL_FILE_HEADER))
                return false;
            memcpy(&tmod, &temp, sizeof(tmod));
        }

        if(!eeFile.putModel(&tmod,cmod))
        {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Error writing to container"));
            return false;
        }
    }

    refreshList();
    setModified();

    return true;
}

void MdiChild::saveModelToFile()
{
    int cmod = currentRow()-1;
    bool genfile = currentRow()==0;

    ModelData tmod;
    EEGeneral tgen;
    QString fileName;
    QSettings settings("er9x-eePe", "eePe");


    if(genfile)
    {
        if(!eeFile.getGeneralSettings(&tgen))
        {
            QMessageBox::critical(this, tr("Error"),tr("Error Getting General Settings Data"));
            return;
        }

        QString ownerName = QString::fromLatin1(tgen.ownerName,sizeof(tgen.ownerName)).trimmed() + ".eepg";

        fileName = QFileDialog::getSaveFileName(this, tr("Save Settings As"),settings.value("lastDir").toString() + "/" +ownerName,tr(EEPG_FILES_FILTER));
    }
    else
    {
        if(!eeFile.eeModelExists(cmod))
        {
            //            QMessageBox::critical(this, tr("Error"),tr("Error Getting Model #%1").arg(cmod+1));
            return;
        }

        if(!eeFile.getModel(&tmod,cmod))
        {
            QMessageBox::critical(this, tr("Error"),tr("Error Getting Model #%1").arg(cmod+1));
            return;
        }

        QString modelName = QString::fromLatin1(tmod.name,sizeof(tmod.name)).trimmed() + ".eepm";

        fileName = QFileDialog::getSaveFileName(this, tr("Save Model As"),settings.value("lastDir").toString() + "/" +modelName,tr(EEPM_FILES_FILTER));
    }

    if (fileName.isEmpty())
        return;

    settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

//    if(genfile)
//        saveiHEX(this, fileName, (quint8*)&tgen, sizeof(tgen), EEPE_GENERAL_FILE_HEADER);
//    else
//        saveiHEX(this, fileName, (quint8*)&tmod, sizeof(tmod), EEPE_MODEL_FILE_HEADER, cmod);

    QFile file(fileName);

    QDomDocument doc(ER9X_EEPROM_FILE_TYPE);
    QDomElement root = doc.createElement(ER9X_EEPROM_FILE_TYPE);
    doc.appendChild(root);

    if(genfile) // general data
    {
            EEGeneral tgen;
            if(!eeFile.getGeneralSettings(&tgen))
            {
                QMessageBox::critical(this, tr("Error"),tr("Error Getting General Settings Data"));
                return;
            }
            QDomElement genData = getGeneralDataXML(&doc, &tgen);
            root.appendChild(genData);
    }
    else  // model data - cmod
    {
        saveModelToXML(&doc, &root, cmod, MDVERS);
    }

    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream ts( &file );
    ts << doc.toString();
    file.close();
}

void MdiChild::duplicate()
{
    int i = this->currentRow();
    if(i && i<MAX_MODELS)
    {
        ModelData gmodel;
        if(eeFile.getModel(&gmodel,--i))
        {
            int j = i+1;
            while(j<MAX_MODELS && eeFile.eeModelExists(j)) j++;
            if(j<MAX_MODELS) eeFile.putModel(&gmodel,j);
        }
        setModified();
    }
}

bool MdiChild::hasSelection()
{
    return (this->selectionModel()->hasSelection());
}

void MdiChild::keyPressEvent(QKeyEvent *event)
{


    if(event->matches(QKeySequence::Delete))
    {
        deleteSelected();
        return;
    }

    if(event->matches(QKeySequence::Cut))
    {
        cut();
        return;
    }

    if(event->matches(QKeySequence::Copy))
    {
        copy();
        return;
    }

    if(event->matches(QKeySequence::Paste))
    {
        paste();
        return;
    }

    if(event->matches(QKeySequence::Underline))
    {
        duplicate();
        return;
    }



    QListWidget::keyPressEvent(event);//run the standard event in case we didn't catch an action
}


void MdiChild::OpenEditWindow()
{
    int i = this->currentRow();

    if(i)
    {
        //TODO error checking
        bool isNew = false;

        if(!eeFile.eeModelExists((uint8_t)i-1))
        {
            eeFile.modelDefault(i-1);
            isNew = true;//modeledit - clear mixes, apply first template
            setModified();
        }

        char buf[sizeof(ModelData().name)+1];
        eeFile.getModelName((i-1),(char*)&buf);
        ModelEdit *t = new ModelEdit(&eeFile,(i-1),this);
        if(isNew)
				{
    			QSettings settings("er9x-eePe", "eePe");
					t->applyBaseTemplate() ;
					int version = settings.value("default_EE_version", 0).toInt() + 1 ;
					if ( version > 1 )
					{
            t->updateToMV2() ;
					}
					if ( version > 2 )
					{
            t->updateToMV3() ;
					}
					if ( version > 3 )
					{
            t->updateToMV4() ;
					}
				}
        t->setWindowTitle(tr("Editing model %1: ").arg(i) + QString(buf));

        for(int j=0; j<MAX_MIXERS; j++)
            t->setNote(j,modelNotes[i-1][j]);
        t->refreshMixerList();

        connect(t,SIGNAL(modelValuesChanged(ModelEdit*)),this,SLOT(setModified(ModelEdit*)));
        //t->exec();
        t->show();
    }
    else
    {
        //TODO error checking
        if(eeFile.eeLoadGeneral())
        {
            //setModified();
            GeneralEdit *t = new GeneralEdit(&eeFile, this);
            connect(t,SIGNAL(modelValuesChanged()),this,SLOT(setModified()));
            t->show();
        }
        else
            QMessageBox::critical(this, tr("Error"),tr("Unable to read settings!"));
    }

}

void MdiChild::newFile()
{
    static int sequenceNumber = 1;
    QSettings settings("er9x-eePe", "eePe");
		int ptype = settings.value("processor", 1).toInt() ;
    QString type ;
		switch (ptype)
		{
			case 1 :
				type = " (M128)" ;
			break ;
			case 2 :
				type = " (M1281)" ;
			break ;
			case 3 :
				type = " (M2561)" ;
			break ;
			default :
				type = " (M64)" ;
			break ;
		}
    isUntitled = true;
    curFile = tr("document%1.eepe").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]" + type );

}

bool MdiChild::loadFile(const QString &fileName, bool resetCurrentFile)
{
    if(!QFileInfo(fileName).exists())
    {
        QMessageBox::critical(this, tr("Error"),tr("Unable to find file %1!").arg(fileName));
        return false;
    }


    int fileType = getFileType(fileName);

    if(fileType==FILE_TYPE_EEPM || fileType==FILE_TYPE_EEPG)
    {
        //load new file and paste in
        newFile();

        setCurrentRow(fileType==FILE_TYPE_EEPG ? 0 : 1);
        if(!loadModelFromFile(fileName))
        {
            QMessageBox::critical(this, tr("Error"),
                                 tr("Error loading file %1:\n"
                                    "File may be corrupted, old or from a different system.\n"
                                    "You might need to update eePe to read this file.")
                                 .arg(fileName));
            return false;
        }

        refreshList();
        if(resetCurrentFile) setCurrentFile(fileName);

        return true;
    }

    if(fileType==FILE_TYPE_HEX || fileType==FILE_TYPE_EEPE) //read HEX file
    {
				uint m128 = 0 ;
        
				//if file is XML read and exit saying true;
        //else process as iHex
        QDomDocument doc(ER9X_EEPROM_FILE_TYPE);
        QFile file(fileName);
        bool xmlOK = file.open(QIODevice::ReadOnly);
        if(xmlOK)
        {
					
            xmlOK = doc.setContent(&file);
            if(xmlOK)
            {
              QDomElement gde = doc.elementsByTagName("Type").at(0).toElement();

              if(!gde.isNull()) // couldn't find
							{
		            m128 = 1 ;
							}
							
								eeFile.setSize( m128 ) ;
							
                //format eefile
                eeFile.formatEFile();
                //read general data
                EEGeneral tgen;
                memset(&tgen,0,sizeof(tgen));
                if(!loadGeneralDataXML(&doc, &tgen))
                {
                    QMessageBox::critical(this, tr("Error"),tr("Error reading file:\n"
                                                               "Cannot read General Settings from file %1").arg(fileName));
                    return false;
                }
                if(!eeFile.putGeneralSettings(&tgen))
                {
                    QMessageBox::critical(this, tr("Error"),tr("Error reading file:\n"
                                                               "Cannot set General Settings"));
                    return false;
                }
		            radioData.File_system[0].size = sizeof(tgen) ;
					  		memcpy( &radioData.generalSettings, &tgen, sizeof(tgen) ) ;

                //read model data
                for(int i=0; i<MAX_MODELS; i++)
                {
                    ModelData tmod;
                    memset(&tmod,0,sizeof(tmod));
                    if(loadModelDataXML(&doc, &tmod, i))
                    {
              				radioData.File_system[i+1].size = sizeof(tmod) ;
              				memcpy( &radioData.models[i], &tmod, sizeof( tmod ) ) ;
											v1ModelTov2( i ) ;
              				memcpy( &radioData.ModelNames[i+1], &radioData.models[i].name, sizeof( radioData.models[0].name) ) ;
              				radioData.ModelNames[i+1][sizeof( radioData.models[0].name)+1] = '\0' ;
                        eeFile.putModel(&tmod,i);
                        getNotesFromXML(&doc, i);
                    }
                }
            }
            file.close();
        }

        if(!xmlOK)
        {
					// 9K to 11K for '128
            if((QFileInfo(fileName).size()>(6*1024)) || (QFileInfo(fileName).size()<(4*1024)))  //if filesize> 6k or <4kb
            {
							
              if((QFileInfo(fileName).size()>(12*1024)) || (QFileInfo(fileName).size()<(9*1024)))  //if filesize> 11k or <9kb
							{
							
                QMessageBox::critical(this, tr("Error"),tr("Error reading file:\n"
                                                           "This might be a FW file (er9x.hex?). \n"
                                                           "You might want to try flashing it to the TX.\n"
                                                           "(Burn->Write Flash Memory)").arg(fileName));
                return false;
							}
            }


            quint8 temp[EESIZE128];

            QString header ="";
            if(fileType==FILE_TYPE_EEPE)   // read EEPE file header
                header=EEPE_EEPROM_FILE_HEADER;

            if(!loadiHEX(this, fileName, (quint8*)&temp, EESIZE128, header))
                return false;

						eeFile.setSize( (QFileInfo(fileName).size()>(9*1024)) ) ;

            if(!eeFile.loadFile(&temp))
            {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Error loading file %1:\n"
                                         "File may be corrupted, old or from a different system."
                                         "You might need to update eePe to read this file.")
                                      .arg(fileName));
                return false;
            }
        }

        refreshList();
        if(resetCurrentFile) setCurrentFile(fileName);

        return true;
    }


    if(fileType==FILE_TYPE_BIN) //read binary
    {
        QFile file(fileName);

        if ( (file.size()!=EESIZE64) && (file.size()!=EESIZE128) )
        {
            QMessageBox::critical(this, tr("Error"),tr("Error reading file:\n"
                                                       "File wrong size - %1").arg(fileName));
            return false;
        }

        if (!file.open(QFile::ReadOnly)) {  //reading binary file   - TODO HEX support
            QMessageBox::critical(this, tr("Error"),
                                 tr("Error opening file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return false;
        }

        uint8_t temp[EESIZE128];
        long result = file.read((char*)&temp,file.size());
        file.close();

        if (result!=file.size())
        {
            QMessageBox::critical(this, tr("Error"),
                                 tr("Error reading file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));

            return false;
        }

        if(!eeFile.loadFile(&temp))
        {
            QMessageBox::critical(this, tr("Error"),
                                 tr("Error loading file %1:\n"
                                    "File may be corrupted, old or from a different system."
                                    "You might need to update eePe to read this file.")
                                 .arg(fileName));
            return false;
        }
        refreshList();
        if(resetCurrentFile) setCurrentFile(fileName);

        return true;
    }

    return false;
}

void MdiChild::v1ModelTov2( int i )
{
  struct t_ModelData *v1m = &radioData.models[i] ;
  struct t_V2ModelData *v2m = &radioData.v2models[i] ;
	uint32_t i ;

	v2m->traineron = v1m->traineron ;
	v2m->FrSkyUsrProto = v1m->FrSkyUsrProto ;
	v2m->FrSkyGpsAlt = v1m->FrSkyGpsAlt ;
	v2m->FrSkyImperial          = v1m->FrSkyImperial ;
	v2m->protocol               = v1m->protocol      ;
	v2m->country                = v1m->country       ;
	v2m->ppmNCH                 = v1m->ppmNCH        ;
	v2m->thrTrim                = v1m->thrTrim       ;
	v2m->thrExpo                = v1m->thrExpo       ;
	v2m->ppmStart               = v1m->ppmStart      ;
	v2m->trimInc                = v1m->trimInc       ;
	v2m->ppmDelay               = v1m->ppmDelay      ;
	v2m->trimSw                 = v1m->trimSw        ;
	v2m->beepANACenter          = v1m->beepANACenter ;
	v2m->pulsePol               = v1m->pulsePol      ;
	v2m->extendedLimits         = v1m->extendedLimits ;
	v2m->swashInvertELE         = v1m->swashInvertELE ;
	v2m->swashInvertAIL         = v1m->swashInvertAIL ;
	v2m->swashInvertCOL         = v1m->swashInvertCOL ;
	v2m->swashType              = v1m->swashType     ;
	v2m->swashCollectiveSource  = v1m->swashCollectiveSource ;
	v2m->swashRingValue         = v1m->swashRingValue ;
	v2m->ppmFrameLength         = v1m->ppmFrameLength ;
	v2m->trim[0]         = v1m->trim[0] ;
	v2m->trim[1]         = v1m->trim[1] ;
	v2m->trim[2]         = v1m->trim[2] ;
	v2m->trim[3]         = v1m->trim[3] ;
	v2m->anaVolume        = v1m->anaVolume        ;
	v2m->numBlades        = v1m->numBlades        ;
	v2m->sub_trim_limit   = v1m->sub_trim_limit        ;
	v2m->CustomDisplayIndex[0][0] = v1m->CustomDisplayIndex[0] ;
	v2m->CustomDisplayIndex[0][1] = v1m->CustomDisplayIndex[1] ;
	v2m->CustomDisplayIndex[0][2] = v1m->CustomDisplayIndex[2] ;
	v2m->CustomDisplayIndex[0][3] = v1m->CustomDisplayIndex[3] ;
	v2m->CustomDisplayIndex[0][4] = v1m->CustomDisplayIndex[4] ;
	v2m->CustomDisplayIndex[0][5] = v1m->CustomDisplayIndex[5] ;
	v2m->varioData        = v1m->varioData        ;
	v2m->modelVersion     = v1m->modelVersion        ;
	v2m->sub_protocol     = v1m->sub_protocol        ;
	v2m->option_protocol  = v1m->option_protocol        ;
	v2m->telemetryProtocol = v1m->telemetryProtocol        ;
	v2m->currentSource = v1m->currentSource        ;
	v2m->throttleIdle = v1m->throttleIdle        ;
	v2m->throttleReversed = v1m->throttleReversed        ;
	for ( i = 0 ; i < MAX_MIXERS ; i += 1 )
	{
		v2m->mixData[i] = v1m->mixData[i] ;
	}
	for ( i = 0 ; i < NUM_VOICE_ALARMS ; i += 1 )
	{
		v2m->vad[i] = v1m->vad[i] ;
	}


}

void MdiChild::v2ModelTov1( int i )
{
  struct t_ModelData *v1m = &radioData.models[i] ;
  struct t_V2ModelData *v2m = &radioData.v2models[i] ;

	v1m->traineron = v2m->traineron ;
	v1m->FrSkyUsrProto = v2m->FrSkyUsrProto ;
	v1m->FrSkyGpsAlt = v2m->FrSkyGpsAlt ;
	v1m->FrSkyImperial         = v2m->FrSkyImperial         ;
	v1m->protocol              = v2m->protocol              ;
	v1m->country               = v2m->country               ;
	v1m->ppmNCH                = v2m->ppmNCH                ;
	v1m->thrTrim               = v2m->thrTrim               ;
	v1m->thrExpo               = v2m->thrExpo               ;
	v1m->ppmStart              = v2m->ppmStart              ;
	v1m->trimInc               = v2m->trimInc               ;
	v1m->ppmDelay              = v2m->ppmDelay              ;
	v1m->trimSw                = v2m->trimSw                ;
	v1m->beepANACenter         = v2m->beepANACenter         ;
	v1m->pulsePol              = v2m->pulsePol              ;
	v1m->extendedLimits        = v2m->extendedLimits        ;
	v1m->swashInvertELE        = v2m->swashInvertELE        ;
	v1m->swashInvertAIL        = v2m->swashInvertAIL        ;
	v1m->swashInvertCOL        = v2m->swashInvertCOL        ;
	v1m->swashType             = v2m->swashType             ;
	v1m->swashCollectiveSource = v2m->swashCollectiveSource ;
	v1m->swashRingValue        = v2m->swashRingValue        ;
	v1m->ppmFrameLength        = v2m->ppmFrameLength        ;
	v1m->trim[0]         = v2m->trim[0] ;
	v1m->trim[1]         = v2m->trim[1] ;
	v1m->trim[2]         = v2m->trim[2] ;
	v1m->trim[3]         = v2m->trim[3] ;
	v1m->anaVolume        = v2m->anaVolume ; // May truncate
	v1m->numBlades        = v2m->numBlades        ;
	v1m->sub_trim_limit   = v2m->sub_trim_limit        ;
	v1m->CustomDisplayIndex[0] = v2m->CustomDisplayIndex[0][0] ;
	v1m->CustomDisplayIndex[1] = v2m->CustomDisplayIndex[0][1] ;
	v1m->CustomDisplayIndex[2] = v2m->CustomDisplayIndex[0][2] ;
	v1m->CustomDisplayIndex[3] = v2m->CustomDisplayIndex[0][3] ;
	v1m->CustomDisplayIndex[4] = v2m->CustomDisplayIndex[0][4] ;
	v1m->CustomDisplayIndex[5] = v2m->CustomDisplayIndex[0][5] ;
	v1m->varioData        = v2m->varioData        ;
	v1m->modelVersion     = v2m->modelVersion        ;
	v1m->sub_protocol     = v2m->sub_protocol        ;
	v1m->option_protocol  = v2m->option_protocol        ;
	v1m->telemetryProtocol = v2m->telemetryProtocol        ;
	v1m->currentSource = v2m->currentSource        ;
	v1m->throttleIdle = v2m->throttleIdle        ;
	v1m->throttleReversed = v2m->throttleReversed        ;
	for ( i = 0 ; i < MAX_MIXERS ; i += 1 )
	{
		v1m->mixData[i] = v2m->mixData[i] ;
	}
	for ( i = 0 ; i < NUM_VOICE_ALARMS ; i += 1 )
	{
		v1m->vad[i] = v2m->vad[i] ;
	}


}

bool MdiChild::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MdiChild::saveAs()
{
    QSettings settings("er9x-eePe", "eePe");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),settings.value("lastDir").toString() + "/" +strippedName(curFile),tr(EEPROM_FILES_FILTER));
    if (fileName.isEmpty())
        return false;

    settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());
    return saveFile(fileName);
}


void MdiChild::getNotesFromXML(QDomDocument * qdoc, int model_id)
{
    //look for MODEL_DATA with modelNum attribute.
    //if modelNum = -1 then just pick the first one
    QDomNodeList ndl = qdoc->elementsByTagName("MODEL_DATA");

    //cycle through nodes to find correct model number
    QDomNode k = ndl.at(0);
    if(model_id>=0) //if we should look for SPECIFIC model cycle through models
    {
        while(!k.isNull())
        {
            int a = k.toElement().attribute("number").toInt();
            if(a==model_id)
                break;
            k = k.nextSibling();
        }
    }

    if(k.isNull()) // couldn't find
        return;

    QDomNode n = k.toElement().elementsByTagName("Notes").at(0).firstChild();// get all children under "Notes"
    while (!n.isNull())
    {
        if(n.nodeName()=="note")
        {
            QDomElement e = n.toElement();
            int mixNum = QString(e.attribute("mix")).toInt();
            modelNotes[model_id][mixNum] = e.firstChild().toText().data();
        }
        n = n.nextSibling();
    }
}

void MdiChild::saveModelToXML(QDomDocument * qdoc, QDomElement * pe, int model_id, int mdver)
{
    if(eeFile.eeModelExists(model_id))
    {
        ModelData tmod;
        if(!eeFile.getModel(&tmod,model_id))  // if can't get model - exit
        {
            return;
        }
        QDomElement modData = getModelDataXML(qdoc, &tmod, model_id, mdver);
        pe->appendChild(modData);

        //add notes to model data
        QDomElement eNotes = qdoc->createElement("Notes");

        int numNodes = 0;
        for(int i=0; i<MAX_MIXERS; i++)
            if(!modelNotes[model_id][i].isEmpty())
            {
                numNodes++;
                QDomElement e = qdoc->createElement("note");
                QDomText t = qdoc->createTextNode("note");
                t.setNodeValue(modelNotes[model_id][i]);
                e.appendChild(t);
                e.setAttribute("model", model_id);
                e.setAttribute("mix", i);
                eNotes.appendChild(e);
            }

        if(numNodes)  // add only if non-empty
            modData.appendChild(eNotes);
    }
}

bool MdiChild::saveFile(const QString &fileName, bool setCurrent)
{
    QFile file(fileName);

    int fileType = getFileType(fileName);


    if(fileType==FILE_TYPE_EEPE) //write hex
    {
        QDomDocument doc( ER9X_EEPROM_FILE_TYPE );
        QDomElement root = doc.createElement(ER9X_EEPROM_FILE_TYPE);
        doc.appendChild(root);
        
				if ( (eesize()== EESIZE128) )
				{
          appendNumberElement( &doc, &root, "Type", 128, true); // have to write value here
				}

        //Save General Data
        EEGeneral tgen;
        memset(&tgen,0,sizeof(tgen));
        if(!eeFile.getGeneralSettings(&tgen))
        {
            QMessageBox::critical(this, tr("Error"),tr("Error Getting General Settings Data"));
            return false;
        }
        tgen.myVers = MDVERS; //make sure we're at the current rev
        QDomElement genData = getGeneralDataXML(&doc, &tgen);
        root.appendChild(genData);

        //Save model data one by one
        for(int i=0; i<MAX_MODELS; i++)
        {
            saveModelToXML(&doc, &root, i, MDVERS);
        }

        if (!file.open(QFile::WriteOnly)) {
            QMessageBox::warning(this, tr("Error"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return false;
        }

        QTextStream ts( &file );
        ts << doc.toString();
        file.close();

        if(setCurrent) setCurrentFile(fileName);
        return true;
    }


    if(fileType==FILE_TYPE_HEX) //write hex
    {
        quint8 temp[EESIZE128];
        eeFile.saveFile(&temp);
        QString header = "";
        saveiHEX(this, fileName, (quint8*)&temp, eeFile.mee_type ? EESIZE128 : EESIZE64, header, NOTES_ALL);


        if(setCurrent) setCurrentFile(fileName);
        return true;
    }

    if(fileType==FILE_TYPE_BIN) //write binary
    {
        if (!file.open(QFile::WriteOnly)) {
            QMessageBox::warning(this, tr("Error"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return false;
        }

        uint8_t temp[EESIZE128];
        eeFile.saveFile(&temp);

        long result = file.write((char*)&temp,eeFile.mee_type ? EESIZE128 : EESIZE64);
				{
					char value[20] ;
          sprintf( value, "%ld,%d", result, eeFile.mee_type ) ;
        	QMessageBox::warning(this, tr("Info"),
                                 tr("Writing file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(value));

				}	
        if(result!= (eeFile.mee_type ? EESIZE128 : EESIZE64) )
        {
            QMessageBox::warning(this, tr("Error"),
                                 tr("Error writing file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return false;
        }

        if(setCurrent) setCurrentFile(fileName);
        return true;
    }

    return false;
}

QString MdiChild::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

void MdiChild::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MdiChild::documentWasModified()
{
    setWindowModified(eeFile.Changed());
}

bool MdiChild::maybeSave()
{
    if (eeFile.Changed()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("eePe"),
                     tr("'%1' has been modified.\n"
                        "Do you want to save your changes?")
                     .arg(userFriendlyCurrentFile()),
                     QMessageBox::Save | QMessageBox::Discard
                     | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;

}

void MdiChild::setCurrentFile(const QString &fileName)
{
    
    QString type = (eeFile.eesize() == EESIZE128) ? " (M128)" : " (M64)";
	
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    eeFile.setChanged(false);
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]" + type ) ;
}

QString MdiChild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

int MdiChild::getFileType(const QString &fullFileName)
{
    if(QFileInfo(fullFileName).suffix().toUpper()=="HEX")  return FILE_TYPE_HEX;
    if(QFileInfo(fullFileName).suffix().toUpper()=="BIN")  return FILE_TYPE_BIN;
    if(QFileInfo(fullFileName).suffix().toUpper()=="EEPM") return FILE_TYPE_EEPM;
    if(QFileInfo(fullFileName).suffix().toUpper()=="EEPG") return FILE_TYPE_EEPG;
    if(QFileInfo(fullFileName).suffix().toUpper()=="EEPE") return FILE_TYPE_EEPE;
    return 0;
}

void MdiChild::optimizeEEPROM()
{
    //save general settings and model data in external buffer
    //format eeprom
    //write settings back to eeprom

    EEGeneral tgen;
    ModelData mgen[MAX_MODELS];

    memset(&tgen, 0, sizeof(tgen));
    memset(&mgen, 0, sizeof(mgen));

    eeFile.getGeneralSettings(&tgen);
    for(int i=0; i<MAX_MODELS; i++)
        eeFile.getModel(&mgen[i],i);

    eeFile.formatEFile();

    eeFile.putGeneralSettings(&tgen);
    for(int i=0; i<MAX_MODELS; i++)
        eeFile.putModel(&mgen[i],i);
}

void MdiChild::burnTo()  // write to Tx
{
// Add towards eeprom checking, don't write a '128 to a '64 etc.
//    int size = eesize() ;

    QMessageBox::StandardButton ret = QMessageBox::question(this, tr("eePe"),
                 tr("Write %1 to EEPROM memory?").arg(strippedName(curFile)),
                 QMessageBox::Yes | QMessageBox::No);

//    optimizeEEPROM();

    if (ret == QMessageBox::Yes)
    {
        burnConfigDialog bcd;
        QString avrdudeLoc = bcd.getAVRDUDE();
        QString tempDir    = QDir::tempPath();
        QString programmer = bcd.getProgrammer();
    		QString mcu        = bcd.getMCU();
        QStringList args   = bcd.getAVRArgs();
        
				if ( args.contains("-F") )
				{
    			QMessageBox::StandardButton ret = QMessageBox::question(this, tr("eePe"),
                 tr("Only use -F if ABSOLUTELY sure. Continue?"),
                 QMessageBox::Yes | QMessageBox::No);
    			if (ret != QMessageBox::Yes)
					{
						return ;
					}
				}
				
				if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

        QString tempFile = tempDir + "/temp.hex";
        saveFile(tempFile, false);
        if(!QFileInfo(tempFile).exists())
        {
            QMessageBox::critical(this,tr("Error"), tr("Cannot write temporary file!"));
            return;
        }
        QString str = "eeprom:w:" + tempFile + ":i"; // writing eeprom -> MEM:OPR:FILE:FTYPE"

        QStringList arguments;
        arguments << "-c" << programmer << "-p" << mcu << args << "-U" << str;

        avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments, "Write EEPROM To Tx", AVR_DIALOG_SHOW_DONE);
        ad->setWindowIcon(QIcon(":/images/write_eeprom.png"));
        ad->show();
    }
}

bool MdiChild::saveToFileEnabled()
{
    int crow = currentRow();
    if(crow==0)
        return true;

    return eeFile.eeModelExists(crow-1);
}

void MdiChild::ShowContextMenu(const QPoint& pos)
{
    QPoint globalPos = this->mapToGlobal(pos);

    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    bool hasData = mimeData->hasFormat("application/x-eepe");

    QMenu contextMenu;
    contextMenu.addAction(QIcon(":/images/edit.png"), tr("&Edit"),this,SLOT(OpenEditWindow()));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/edit.png"), tr("&Wizard"),this,SLOT(wizardEdit()));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/clear.png"), tr("&Delete"),this,SLOT(deleteSelected(bool)),tr("Delete"));
    contextMenu.addAction(QIcon(":/images/copy.png"), tr("&Copy"),this,SLOT(copy()),tr("Ctrl+C"));
    contextMenu.addAction(QIcon(":/images/cut.png"), tr("&Cut"),this,SLOT(cut()),tr("Ctrl+X"));
    contextMenu.addAction(QIcon(":/images/paste.png"), tr("&Paste"),this,SLOT(paste()),tr("Ctrl+V"))->setEnabled(hasData);
    contextMenu.addAction(QIcon(":/images/duplicate.png"), tr("D&uplicate"),this,SLOT(duplicate()),tr("Ctrl+U"));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/load_model.png"), tr("&Load Model/Settings"),this,SLOT(loadModelFromFile()),tr("Ctrl+L"));
    contextMenu.addAction(QIcon(":/images/save_model.png"), tr("&Save Model/Settings"),this,SLOT(saveModelToFile()),tr("Ctrl+S"))->setEnabled(saveToFileEnabled());
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/simulate.png"), tr("Simulate"),this,SLOT(simulate()),tr("Alt+S"));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/write_eeprom.png"), tr("&Write To Tx"),this,SLOT(burnTo()),tr("Ctrl+Alt+W"));
    contextMenu.addSeparator();
    contextMenu.addAction(QIcon(":/images/edit.png"), tr("Set &Active Model"),this,SLOT(setActive()));

    contextMenu.exec(globalPos);
}

void MdiChild::setModified(ModelEdit * me)
{
    refreshList();
    eeFile.setChanged(true);
    documentWasModified();

    if(me)
    {
        int id = me->getModelID();
        for(int j=0; j<MAX_MIXERS; j++)
            modelNotes[id][j] = me->getNote(j);
    }
}

void MdiChild::setActive()
{
  int i = this->currentRow() ;
  
	if ( i )
	{
		radioData.generalSettings.currModel = i - 1 ;
		refreshList();
	}
}

void MdiChild::simulate()
{
    if(currentRow()<1) return;

    EEGeneral gg;
#ifdef SKY
    SKYModelData gm;
		if( !radioData.File_system[0].size ) return ;
    if( !radioData.File_system[currentRow()].size ) return ;
    memcpy( &gg, &radioData.generalSettings, sizeof(EEGeneral) ) ;
    memcpy( &gm, &radioData.models[currentRow()-1], sizeof(SKYModelData) ) ;
#else
    if(!eeFile.getGeneralSettings(&gg)) return;

    ModelData gm;
    if(!eeFile.getModel(&gm,currentRow()-1)) return;
#endif
 		simulatorDialog *sd ;

		sd = SimPointer ;
    sd->loadParams(gg,gm);
    sd->show();
}

void MdiChild::print()
{
    if(currentRow()<1) return;

    EEGeneral gg;

#ifdef SKY
    SKYModelData gm;
		memcpy( &gg, &radioData.generalSettings, sizeof(EEGeneral) ) ;
    memcpy( &gm, &radioData.models[currentRow()-1], sizeof(SKYModelData) ) ;
#else
    ModelData gm;
    if(!eeFile.getGeneralSettings(&gg)) return;
    if(!eeFile.getModel(&gm,currentRow()-1)) return;
#endif
    printDialog *pd = new printDialog(this, &gg, &gm);
    pd->show();
}

void MdiChild::viableModelSelected(int idx)
{
    if(!isVisible())
        emit copyAvailable(false);
    else if(idx<1)
        emit copyAvailable(false);
    else
        emit copyAvailable(eeFile.eeModelExists(currentRow()-1));

    emit saveModelToFileAvailable(saveToFileEnabled());
}

void MdiChild::wizardEdit()
{
   EEGeneral gg;
   if(!eeFile.getGeneralSettings(&gg)) return;
	
//  int row = ui->modelsList->currentRow();
  int row = this->currentRow();
  if (row > 0)
	{
    eeFile.modelDefault(row-1) ;
//    checkAndInitModel(row);
    WizardDialog * wizard = new WizardDialog( gg, row, this);
    wizard->exec();
    if (wizard->mix.complete /*TODO rather test the exec() result?*/)
		{
    	ModelData gm ;
//	    eeFile.getModel(&gm,currentRow()-1) ;
      gm = wizard->mix ;
			
      eeFile.putModel( &gm,row-1) ;
//      radioData.models[row - 1] = wizard->mix;
      setModified();
    }
  }
}




