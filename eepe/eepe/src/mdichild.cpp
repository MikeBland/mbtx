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
#ifdef V2
    if (radioData.v2generalSettings.currModel < 16 )
#else
    if (radioData.generalSettings.currModel < 16 )
#endif
		{
			QFont f = QFont("Courier New", 12) ;
			f.setBold(true) ;
#ifdef V2
      this->item(radioData.v2generalSettings.currModel+1)->setFont(f) ;
#else
      this->item(radioData.generalSettings.currModel+1)->setFont(f) ;
#endif
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
#ifdef V2
    V2EEGeneral tgen;
#else
    EEGeneral tgen;
#endif
    eeFile.getGeneralSettings(&tgen);
    return QString::fromLatin1(tgen.ownerName,sizeof(tgen.ownerName)).trimmed();
}

int MdiChild::eepromVersion()
{
#ifdef V2
    V2EEGeneral tgen;
#else
    EEGeneral tgen;
#endif
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
#ifdef V2
				    V2EEGeneral tgen;
#else
    				EEGeneral tgen;
#endif
            if(eeFile.getGeneralSettings(&tgen))
            {
#ifdef V2
                gmData->append('2');
#endif
                gmData->append('G');
                gmData->append((char*)&tgen,sizeof(tgen));
            }
        }
        else
        {
#ifdef V2
            V2ModelData tmod;
#else
            ModelData tmod;
#endif
            if(eeFile.getModel(&tmod,index.row()-1))
            {
#ifdef V2
              gmData->append('2');
#endif
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
        char d = '1' ;
        char c = *gData;
        i++;
        gData++;
#ifdef V2
        if ( c == '2' )
				{
					d = '2' ;
					c = *gData ;
        	gData++;
				}
#endif
        if(c=='G')  //general settings
        {
#ifdef V2
            if(!eeFile.putGeneralSettings((V2EEGeneral*)gData))
#else					
            if(!eeFile.putGeneralSettings((EEGeneral*)gData))
#endif
            {
                QMessageBox::critical(this, tr("Error"),tr("Unable set data!"));
                break;
            }
#ifdef V2
            gData += sizeof(V2EEGeneral);
            i     += sizeof(V2EEGeneral);
#else					
            gData += sizeof(EEGeneral);
            i     += sizeof(EEGeneral);
#endif
        }
#ifdef V2
        else if ( c == '2' ) //model data
				{
          if(!eeFile.putModel((ModelData*)gData,id-1))
          {
            QMessageBox::critical(this, tr("Error"),tr("Unable set model!"));
            break;
          }
          gData += sizeof(V2ModelData);
          i     += sizeof(V2ModelData);
          id++;
					
				}
#endif
        else if ( ( c & 0x7F) == 'M' ) //model data
        {
#ifdef V2
					memcpy( &radioData.v1models[id-1], (ModelData*)gData, sizeof( radioData.v1models[0]) ) ;
					v1ModelTov2( id-1 ) ;
					if(!eeFile.putModel(&radioData.models[id-1],id-1))
          {
            QMessageBox::critical(this, tr("Error"),tr("Unable set model!"));
            break;
          }
          gData += sizeof(ModelData);
          i     += sizeof(ModelData);
          id++;
#else
            if(!eeFile.putModel((ModelData*)gData,id-1))
            {
                QMessageBox::critical(this, tr("Error"),tr("Unable set model!"));
                break;
            }
            gData += sizeof(ModelData);
            i     += sizeof(ModelData);
            id++;
#endif
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
#ifdef V2
		    V2EEGeneral tgen;
#else
		    EEGeneral tgen;
#endif

        //get general data from XML file, if not, get it from iHEX

#ifdef V2
        QDomDocument doc(MBTX_EEPROM_FILE_TYPE);
#else
        QDomDocument doc(ER9X_EEPROM_FILE_TYPE);
#endif
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

#ifdef V2
        QDomDocument doc(MBTX_EEPROM_FILE_TYPE);
#else
        QDomDocument doc(ER9X_EEPROM_FILE_TYPE);
#endif
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
#ifdef V2
    V2EEGeneral tgen;
#else
    EEGeneral tgen;
#endif
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

#ifdef V2
    QDomDocument doc(MBTX_EEPROM_FILE_TYPE);
    QDomElement root = doc.createElement(MBTX_EEPROM_FILE_TYPE);
#else
    QDomDocument doc(ER9X_EEPROM_FILE_TYPE);
    QDomElement root = doc.createElement(ER9X_EEPROM_FILE_TYPE);
#endif
    doc.appendChild(root);

    if(genfile) // general data
    {
#ifdef V2
				    V2EEGeneral tgen;
#else
				    EEGeneral tgen;
#endif
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
				uint v1type = 0 ;
        
				//if file is XML read and exit saying true;
        //else process as iHex
#ifdef V2
        QDomDocument doc(ER9X_EEPROM_FILE_TYPE);
//		    QDomDocument doc(MBTX_EEPROM_FILE_TYPE);
#else
    		QDomDocument doc(ER9X_EEPROM_FILE_TYPE);
#endif
        QFile file(fileName);
        bool xmlOK = file.open(QIODevice::ReadOnly);
        if(xmlOK)
        {
          xmlOK = doc.setContent(&file);
          if(xmlOK)
          {
						QString nametype = doc.doctype().name() ;
#ifdef V2
						if ( nametype == ER9X_EEPROM_FILE_TYPE )
						{
							v1type = 1 ;
						}
						else if ( nametype != MBTX_EEPROM_FILE_TYPE )
#else
						if ( nametype != ER9X_EEPROM_FILE_TYPE )
#endif
						{
	            QMessageBox::critical(this, tr("Error"),
                                 tr("Error loading file %1:\n"
                                    "File may be corrupted, old or from a different system.\n"
                                    "You might need to update eePe to read this file.")
                                 .arg(fileName));
	            file.close();
	            return false;
						}
            QDomElement gde = doc.elementsByTagName("Type").at(0).toElement();

            if(!gde.isNull()) // couldn't find
						{
	            m128 = 1 ;
						}
							
								eeFile.setSize( m128 ) ;
							
                //format eefile
                eeFile.formatEFile();
                //read general data
#ifdef V2
                V2EEGeneral tgen;
#else
                EEGeneral tgen;
#endif
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
#ifdef V2
					  		memcpy( &radioData.v2generalSettings, &tgen, sizeof(tgen) ) ;
#else
					  		memcpy( &radioData.generalSettings, &tgen, sizeof(tgen) ) ;
#endif

                //read model data
                for(int i=0; i<MAX_MODELS; i++)
                {
#ifdef V2
									union
									{
                    V2ModelData t2mod;
                    V1ModelData tmod;
										uint8_t tbytes[sizeof(V1ModelData)] ;
									} tModel ;
                  memset(&tModel,0,sizeof(tModel));
                  if(loadModelDataXML(&doc, &tModel.t2mod, i))
									{
										if ( v1type )
										{
              				memcpy( &radioData.v1models[i], &tModel.tmod, sizeof( tModel.tmod ) ) ;
											v1ModelTov2( i ) ;
										}
										else
										{
             					memcpy( &radioData.models[i], &tModel.t2mod, sizeof( tModel.t2mod ) ) ;
										}
             					radioData.File_system[i+1].size = sizeof(tModel.t2mod) ;
            					memcpy( &radioData.ModelNames[i+1], &radioData.models[i].name, sizeof( radioData.models[0].name) ) ;
             					radioData.ModelNames[i+1][sizeof( radioData.models[0].name)+1] = '\0' ;
                   	  eeFile.putModel(&radioData.models[i],i);
                   	  getNotesFromXML(&doc, i);
                  }
#else
                    V1ModelData tmod;
                    if(loadModelDataXML(&doc, &tmod, i))
                    {
              				radioData.File_system[i+1].size = sizeof(tmod) ;
              				memcpy( &radioData.models[i], &tmod, sizeof( tmod ) ) ;
              				memcpy( &radioData.ModelNames[i+1], &radioData.models[i].name, sizeof( radioData.models[0].name) ) ;
              				radioData.ModelNames[i+1][sizeof( radioData.models[0].name)+1] = '\0' ;
                      eeFile.putModel(&tmod,i);
                      getNotesFromXML(&doc, i);
                    }
#endif
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

#ifdef V2
int MdiChild::v1SwitchTov2( int i )
{
  uint8_t sign = 0 ;
	if ( i < 0 )
	{
		i = -i ;
		sign = 1 ;
	}
	switch ( i )
	{
		case HSW_ThrCt :
			i = DSW_THR ;
	  break ;
		case HSW_RuddDR :
			i = DSW_RUD ;
	  break ;
		case HSW_ElevDR :
			i = DSW_ELE ;
	  break ;
		case HSW_AileDR :
			i = DSW_AIL ;
	  break ;
		case HSW_Gear :
			i = DSW_GEA ;
	  break ;
		case HSW_Trainer :
			i = DSW_TRN ;
	  break ;
		case HSW_ID0 :
			i = DSW_ID0 ;
	  break ;
		case HSW_ID1 :
			i = DSW_ID1 ;
	  break ;
		case HSW_ID2 :
			i = DSW_ID2 ;
	  break ;
	
		case HSW_Ele3pos0 :
			i = DSW_ID0 + 9 ;
	  break ;
		case HSW_Ele3pos1	:
			i = DSW_ID1 + 9 ;
	  break ;
		case HSW_Ele3pos2	:
			i = DSW_ID2 + 9 ;
	  break ;
		case HSW_Rud3pos0	:
			i = DSW_ID0 + 6 ;
	  break ;
		case HSW_Rud3pos1	:
			i = DSW_ID1 + 6 ;
	  break ;
		case HSW_Rud3pos2	:
			i = DSW_ID2 + 6 ;
	  break ;
		case HSW_Ail3pos0	:
			i = DSW_ID0 + 12 ;
	  break ;
		case HSW_Ail3pos1	:
			i = DSW_ID1 + 12 ;
	  break ;
		case HSW_Ail3pos2	:
			i = DSW_ID2 + 12 ;
	  break ;
		case HSW_Gear3pos0 :
			i = DSW_ID0 + 15 ;
	  break ;
		case HSW_Gear3pos1 :
			i = DSW_ID1 + 15 ;
	  break ;
		case HSW_Gear3pos2 :
			i = DSW_ID2 + 15 ;
	  break ;
		case HSW_Pb1 :
			i = DSW_PB1 ;
	  break ;
		case HSW_Pb2 :
			i = DSW_PB2 ;
	  break ;
	}
  if (sign)
  {
    i = -i ;
  }
	return i ;
}

void MdiChild::v1ModelTov2( int i )
{
  struct t_ModelData *v1m = &radioData.v1models[i] ;
  struct t_V2ModelData *v2m = &radioData.models[i] ;
	int j ;

  memset( v2m,0,sizeof(*v2m));

	for ( i = 0 ; i < MODEL_NAME_LEN ; i += 1 )
	{
		v2m->name[i] = v1m->name[i] ;
	}
	v2m->modelVoice = v1m->modelVoice ;
	v2m->timer[0].tmrModeA = v1m->tmrMode ;
	v2m->timer[0].tmrDir = v1m->tmrDir ;
	v2m->timer[0].tmrVal = v1m->tmrVal ;
	v2m->timer[0].tmrModeB = v1m->tmrModeB ;
	v2m->timer[0].tmrCdown = v1m->timer1Cdown ;
	v2m->timer[0].tmrMbeep = v1m->timer1Mbeep ;
	v2m->timer[0].tmrRstSw = v1SwitchTov2(v1m->timer1RstSw) ;

	v2m->timer[1].tmrModeA = v1m->tmr2Mode ;
	v2m->timer[1].tmrDir = v1m->tmr2Dir ;
	v2m->timer[1].tmrVal = v1m->tmr2Val ;
	v2m->timer[1].tmrModeB = v1m->tmr2ModeB ;
	v2m->timer[1].tmrCdown = v1m->timer2Cdown ;
	v2m->timer[1].tmrMbeep = v1m->timer2Mbeep ;
	v2m->timer[1].tmrRstSw = v1SwitchTov2(v1m->timer2RstSw) ;
	v2m->unused_tmrDir = 0 ;
	v2m->traineron = v1m->traineron ;
	v2m->unused_xt2throttle = 0 ;
	v2m->FrSkyUsrProto = v1m->FrSkyUsrProto ;
	v2m->FrSkyGpsAlt = v1m->FrSkyGpsAlt ;
	v2m->FrSkyImperial          = v1m->FrSkyImperial ;
	v2m->unused_FrSkyAltAlarm = 0 ;
	v2m->protocol               = v1m->protocol      ;
	v2m->country                = v1m->country       ;
	v2m->unused_xsub_protocol = 0 ;
	v2m->sub_protocol = v1m->sub_protocol ;
	v2m->option_protocol  = v1m->option_protocol        ;
	v2m->ppmNCH                 = v1m->ppmNCH        ;
	v2m->ppmFrameLength         = v1m->ppmFrameLength ;
	v2m->ppmDelay               = v1m->ppmDelay      ;
	v2m->thrTrim                = v1m->thrTrim       ;
	v2m->unused_xnumBlades = 0 ;
	v2m->unused_mixTime = 0 ;
	v2m->thrExpo                = v1m->thrExpo       ;
	v2m->ppmStart               = v1m->ppmStart      ;
	v2m->pulsePol               = v1m->pulsePol      ;
	v2m->extendedLimits         = v1m->extendedLimits ;
	v2m->swashInvertELE         = v1m->swashInvertELE ;
	v2m->swashInvertAIL         = v1m->swashInvertAIL ;
	v2m->swashInvertCOL         = v1m->swashInvertCOL ;
	v2m->swashType              = v1m->swashType     ;
	v2m->swashCollectiveSource  = v1m->swashCollectiveSource ;
	v2m->swashRingValue         = v1m->swashRingValue ;
	for ( i = 0 ; i < MAX_MIXERS ; i += 1 )
	{
		v2m->mixData[i] = v1m->mixData[i] ;
		v2m->mixData[i].swtch = v1SwitchTov2(v1m->mixData[i].swtch) ;
	}
	for ( i = 0 ; i < NUM_CHNOUT ; i += 1 )
	{
		v2m->limitData[i] = v1m->limitData[i] ;
	}
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		v2m->expoData[i] = v1m->expoData[i] ;
		v2m->expoData[i].drSw1 = v1SwitchTov2(v1m->expoData[i].drSw1) ;
		v2m->expoData[i].drSw2 = v1SwitchTov2(v1m->expoData[i].drSw2) ;
	}
	v2m->trim[0]         = v1m->trim[0] ;
	v2m->trim[1]         = v1m->trim[1] ;
	v2m->trim[2]         = v1m->trim[2] ;
	v2m->trim[3]         = v1m->trim[3] ;
	for ( i = 0 ; i < MAX_CURVE5 ; i += 1 )
	{
		for ( j = 0 ; j < 5 ; j += 1 )
		{
			v2m->curves5[i][j] = v1m->curves5[i][j] ;
		}
	}
	for ( i = 0 ; i < MAX_CURVE9 ; i += 1 )
	{
		for ( j = 0 ; j < 9 ; j += 1 )
		{
			v2m->curves9[i][j] = v1m->curves9[i][j] ;
		}
	}

// Custom Switches
//PACK(typedef struct t_CSwData { // Custom Switches data
//  int8_t  v1; //input
//  int8_t  v2; 		//offset
//  uint8_t func:4;
//  uint8_t andsw:4;
//}) CSwData;
//PACK(typedef struct t_CxSwData { // Extra Custom Switches data
//    int8_t  v1; //input
//    int8_t  v2; //offset
//    uint8_t func ;
//    int8_t andsw ;
//}) CxSwData ;
	for ( i = 0 ; i < NUM_CSW ; i += 1 )
	{
		v2m->customSw[i].func = v1m->customSw[i].func ;
    uint8_t s = CS_STATE(v1m->customSw[i].func, v1m->modelVersion ) ;
		switch ( s )
		{
			case CS_VOFS :
			case CS_VCOMP :
			case CS_TIMER :
				v2m->customSw[i].v1 = v1m->customSw[i].v1 ;
				v2m->customSw[i].v2 = v1m->customSw[i].v2 ;
			break ;
			case CS_VBOOL :
				v2m->customSw[i].v1 = v1SwitchTov2(v1m->customSw[i].v1) ;
				v2m->customSw[i].v2 = v1SwitchTov2(v1m->customSw[i].v2) ;
			break ;
		}
		s = v1m->customSw[i].andsw ;
		if ( s > 8 )
		{
			s += 1 ;
		}
		v2m->customSw[i].andsw = v1SwitchTov2( s ) ;
	}
	int k = NUM_CSW ;
	for ( i = 0 ; i < EXTRA_CSW ; i += 1 )
	{
		v2m->customSw[k].func = v1m->xcustomSw[i].func ;
    uint8_t s = CS_STATE(v1m->xcustomSw[i].func, v1m->modelVersion ) ;
		switch ( s )
		{
			case CS_VOFS :
			case CS_VCOMP :
			case CS_TIMER :
				v2m->customSw[k].v1 = v1m->xcustomSw[i].v1 ;
				v2m->customSw[k].v2 = v1m->xcustomSw[i].v2 ;
			break ;
			case CS_VBOOL :
				v2m->customSw[k].v1 = v1SwitchTov2(v1m->xcustomSw[i].v1) ;
				v2m->customSw[k].v2 = v1SwitchTov2(v1m->xcustomSw[i].v2) ;
			break ;
		}
		s = v1m->xcustomSw[i].andsw ;
		if ( ( s > 8 ) && ( s <= 9+NUM_CSW+EXTRA_CSW ) )
		{
      s += 1 ;
		}
		if ( ( s < -8 ) && ( s >= -(9+NUM_CSW+EXTRA_CSW) ) )
		{
			s -= 1 ;
		}
		if ( s == 9+NUM_CSW+EXTRA_CSW+1 )
		{
			s = 9 ;			// Tag TRN on the end, keep EEPROM values
		}
		if ( s == -(9+NUM_CSW+EXTRA_CSW+1) )
		{
			s = -9 ;			// Tag TRN on the end, keep EEPROM values
		}
		v2m->customSw[k].andsw = v1SwitchTov2( s ) ;
		k += 1 ;
	}

	v2m->varioData        = v1m->varioData ;
	v2m->trimInc                = v1m->trimInc       ;
	v2m->trimSw                 = v1SwitchTov2(v1m->trimSw)        ;
	v2m->beepANACenter          = v1m->beepANACenter ;
	v2m->numBlades        = v1m->numBlades        ;
	for ( i = 0 ; i < MAX_GVARS ; i += 1 )
	{
		v2m->gvars[i].gvar = v1m->gvars[i].gvar ;
		v2m->gvars[i].gvsource = v1m->gvars[i].gvsource ;
		v2m->gvars[i].gvswitch = v1SwitchTov2(v1m->gvswitch[i]) ;
	}

// Safety switches
	j = v1m->numVoice ; // 0=>16 safety, 16=>0 safety
	j = 16 - j ;	// J has number of safety switches
	for ( i = 0 ; i < j ; i += 1 )
	{
		int k ;
		k = v1m->safetySw[i].opt.ss.mode ;
		if ( k == 3 )
		{
			k = 1 ;
		}
		else
		{
			if ( k )
			{
				k = 2 ;
			}
		}
		if ( k < 2 )
		{
			v2m->safetySw[i].swtch = v1SwitchTov2(v1m->safetySw[i].opt.ss.swtch) ;
			v2m->safetySw[i].mode = k ;
			v2m->safetySw[i].val = v1m->safetySw[i].opt.ss.val ;
		}
	}
	// now the voice switches, where possible
	k = 0 ;
	for ( i = j ; i < j+8 ; i += 1 )
	{
		v2m->voiceSw[k].swtch = v1SwitchTov2(v1m->safetySw[i].opt.vs.vswtch) ;
		v2m->voiceSw[k].mode = v1m->safetySw[i].opt.vs.vmode ;
		v2m->voiceSw[k].val = v1m->safetySw[i].opt.vs.vval ;
		k += 1 ;
	}

	v2m->CustomDisplayIndex[0][0] = v1m->CustomDisplayIndex[0] ;
	v2m->CustomDisplayIndex[0][1] = v1m->CustomDisplayIndex[1] ;
	v2m->CustomDisplayIndex[0][2] = v1m->CustomDisplayIndex[2] ;
	v2m->CustomDisplayIndex[0][3] = v1m->CustomDisplayIndex[3] ;
	v2m->CustomDisplayIndex[0][4] = v1m->CustomDisplayIndex[4] ;
	v2m->CustomDisplayIndex[0][5] = v1m->CustomDisplayIndex[5] ;
	v2m->modelVersion     = v1m->modelVersion ;
	
	v2m->frsky.channels[0].ratio = v1m->frsky.channels[0].ratio ;
	v2m->frsky.channels[0].unit = v1m->frsky.channels[0].type ;
	v2m->frsky.channels[1].ratio = v1m->frsky.channels[1].ratio ;
	v2m->frsky.channels[1].unit = v1m->frsky.channels[1].type ;
	v2m->frsky.FASoffset      = v1m->frsky.FASoffset ;
	v2m->frsky.rxRssiLow      = 45 ;
	v2m->frsky.rxRssiCritical = 42 ;

// switchWarningStates
  // (MSB=0,Trainer:1,PB2:1,PB1:1,Gear:2,AileDR:2,ElevDR:2,RuddDR:2,ThrCt:2,IDL:2)
	// V1  Gear, Ail, ID2, ID1, ID0, Ele, Rud, Thr

	uint8_t temp = v1m->switchWarningStates ;
	
	v2m->switchWarningStates = (temp & 1 ) << 2 ;
	v2m->switchWarningStates |= (temp & 2 ) << 3 ;
	v2m->switchWarningStates |= (temp & 4 ) << 4 ;
	v2m->switchWarningStates |= (temp & 64 ) << 2 ;
	v2m->switchWarningStates |= (temp & 128 ) << 3 ;
	if ( temp & 16 )
	{
		v2m->switchWarningStates |= 1 ;
	}
	if ( temp & 32 )
	{
		v2m->switchWarningStates |= 2 ;
	}
	 
	v2m->sub_trim_limit   = v1m->sub_trim_limit ;
	
// customStickNames[16], nothing to do
	
	for ( i = 0 ; i < MAX_PHASES ; i += 1 )
	{
		for ( j = 0 ; j < 4 ; j += 1 )
		{
			v2m->phaseData[i].trim[j] = v1m->phaseData[i].trim[j] ;
		}
    v2m->phaseData[i].swtch = v1SwitchTov2(v1m->phaseData[i].swtch) ;
		v2m->phaseData[i].fadeIn = v1m->phaseData[i].fadeIn ;
		v2m->phaseData[i].fadeOut = v1m->phaseData[i].fadeOut ;
		for ( j = 0 ; j < 4 ; j += 1 )
		{
			v2m->phaseData[i].name[j] = v1m->phaseNames[i][j] ;
		}
	}

	for ( i = 0 ; i < NUM_SCALERS ; i += 1 )
	{
		v2m->Scalers[i] = v1m->Scalers[i] ;
	}
	v2m->anaVolume        = v1m->anaVolume        ;
	v2m->currentSource = v1m->currentSource        ;
	v2m->useCustomStickNames = v1m->useCustomStickNames ;
	v2m->throttleIdle = v1m->throttleIdle        ;
	v2m->throttleReversed = v1m->throttleReversed        ;

// GvarAdjusters (nothing to do)
	
	for ( i = 0 ; i < NUM_VOICE_ALARMS ; i += 1 )
	{
		v2m->vad[i] = v1m->vad[i] ;
		v2m->vad[i].swtch = v1SwitchTov2(v1m->vad[i].swtch ) ;
	}
	v2m->telemetryProtocol = v1m->telemetryProtocol        ;
	for ( i = 0 ; i < 18 ; i += 1 )
	{
		v2m->curvexy[i] = v1m->curvexy[i] ;
	}
	 
// ExtScaleData eScalers[NUM_SCALERS] nothing to do
	
	
}

void MdiChild::v2ModelTov1( int i )
{
  struct t_ModelData *v1m = &radioData.v1models[i] ;
  struct t_V2ModelData *v2m = &radioData.models[i] ;

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
//	v1m->telemetryProtocol = v2m->telemetryProtocol        ;
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
#endif


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
#ifdef V2
		    QDomDocument doc(MBTX_EEPROM_FILE_TYPE);
        QDomElement root = doc.createElement(MBTX_EEPROM_FILE_TYPE);
#else
    		QDomDocument doc(ER9X_EEPROM_FILE_TYPE);
        QDomElement root = doc.createElement(ER9X_EEPROM_FILE_TYPE);
#endif
        doc.appendChild(root);
        
				if ( (eesize()== EESIZE128) )
				{
          appendNumberElement( &doc, &root, "Type", 128, true); // have to write value here
				}

        //Save General Data
#ifdef V2
		    V2EEGeneral tgen;
#else
    		EEGeneral tgen;
#endif
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

#ifdef V2
    V2EEGeneral tgen;
#else
    EEGeneral tgen;
#endif
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
#ifdef V2
		radioData.v2generalSettings.currModel = i - 1 ;
#else
		radioData.generalSettings.currModel = i - 1 ;
#endif
		refreshList();
	}
}

void MdiChild::simulate()
{
    if(currentRow()<1) return;

#ifdef V2
    V2EEGeneral gg;
#else
    EEGeneral gg;
#endif
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

#ifdef V2
    V2EEGeneral gg;
#else
    V1EEGeneral gg;
#endif

#ifdef SKY
    SKYModelData gm;
		memcpy( &gg, &radioData.generalSettings, sizeof(EEGeneral) ) ;
    memcpy( &gm, &radioData.models[currentRow()-1], sizeof(SKYModelData) ) ;
#else
#ifdef V2
    V2ModelData gm;
#else
    V1ModelData gm;
#endif
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


#ifdef V2
void t_radioData::initSwitchMapping()
{
  MappedSwitchState = 1;  // reserve IDL bit as it's a permanent 3-pos
  for (uint8_t i = 0; i < MAX_XSWITCH; i++) {
    uint8_t s = getSwitchSource(i);
    if (s != SSW_NONE) {
      mapSwitch(i + 1);   // from THR ... PB2
    }
  }
  setMaxSwitchIndex();
}

//int8_t checkIncDecSwitch( int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags)
//{
//	i_val = switchUnMap( i_val ) ;
//  return switchMap( checkIncDec16(i_val,i_min,i_max,i_flags) ) ;
//}

#define SW_MAPPED (MaxSwitchIndex-MAX_CSWITCH-2)

int8_t t_radioData::switchMap( int8_t mIndex )
{
  int8_t drswitch = mIndex ;
  if (mIndex < 0)
	{
    drswitch = -mIndex ;
	}
  if (drswitch == MaxSwitchIndex)
	{
    drswitch = MAX_DRSWITCH ;
  }
	else
	{
    int8_t mappedSw = SW_MAPPED ;
    if (drswitch > mappedSw)
		{
      drswitch -= mappedSw ;
      drswitch += DSW_PB2 ;      // Trainer switch becomes DSW_TRN
    }
		else
		{
      drswitch = switchMapTable[drswitch] ;
	  }
	}
  if (mIndex < 0)
	{
    drswitch = -drswitch ;
	}
  return drswitch;
	
}

int8_t t_radioData::switchUnMap( int8_t drswitch )
{
  int8_t mIndex = drswitch ;
  if (drswitch < 0)
	{
    mIndex = -drswitch ;
	}
  // mIndex must be [DSW_THR..SW_3POS_END]
  if (mIndex == MAX_DRSWITCH)
	{
    mIndex = MaxSwitchIndex ;
  }
	else if (mIndex >= DSW_TRN && mIndex < DSW_ID0)
	{
    mIndex -= DSW_PB2 ;    // DSW_TRN switch becomes 1
    mIndex += SW_MAPPED ;  // skip over mapped switches
  }
	else
	{  // must be mapped between [1..SW_MAPPED]
    int8_t *pa = switchMapTable ;
    int8_t *pz = pa + SW_MAPPED + 1;
    while (pa < pz)
		{
      if (*pa == mIndex)
			{
        break;
			}
      pa++;
    }
    mIndex = (pa - switchMapTable) ;
  }
  if (drswitch < 0)
	{
    mIndex = -mIndex ;
	}
  return mIndex;
}

uint8_t t_radioData::getSwitchSource( uint8_t xsw )
{
  uint8_t temp ;
  uint8_t src = v2generalSettings.switchSources[xsw >> 1];
  temp = ((xsw & 1) ? (src >> 4) : (src & 0x0F));
  return temp ;
}

void t_radioData::setMaxSwitchIndex()
{
  int8_t *p = switchMapTable;
  int8_t i3 = 0;
  *p++ = 0;   // DSW____
  for (int8_t i2 = 0; i2 < (MAX_XSWITCH+1); i2++, i3 += 3) { // +1 for IDL
    if (qSwitchMapped(i2)) {  // either 3-pos or push butt
      if (i2 < MAX_PSW3POS) { // ID0,ID1,ID2,THR^,THR-,THRv,...,GE,^GE-,GEv
        *p++ = DSW_ID0 + i3 + 0;
        *p++ = DSW_ID0 + i3 + 1;
        *p++ = DSW_ID0 + i3 + 2;
      } else {    // PB1/PB2
        *p++ = DSW_IDL + i2;
      }
    } else if (i2 < MAX_PSW3POS) {
      *p++ = DSW_IDL + i2;    // DSW_THR,DSW_RUD,DSW_ELE,DSW_AIL,DSW_GEA
    }
  }
  MaxSwitchIndex = (p - switchMapTable) + MAX_CSWITCH + 1;  // +1: DSW_TRN
}

#define V2_SWITCHES_STR "IDLTHRRUDELEAILGEAPB1PB2TRNL1 L2 L3 L4 L5 L6 L7 L8 L9 LA LB LC LD LE LF LG LH LI ONFID0ID1ID2TH^TH-THvRU^RU-RUvEL^EL-ELvAI^AI-AIvGE^GE-GEv"

QString t_radioData::getMappedSWName(int val, int eepromType )
{
  int limit = MaxSwitchIndex ;

  if(!val) return "---";
  if(val == limit) return "ON";
	if(val == -limit) return "OFF";

	int sw = val ;
	if ( sw < 0 )
	{
		sw = -val ;
	}
	sw = switchMap( sw ) ;
	QString switches ;
	switches = V2_SWITCHES_STR ;
  QString temp ;
  temp = switches.mid((sw-1)*3,3) ;
  return QString(val<0 ? "!" : "") + temp ;
}

void t_radioData::populateSwitchCB(QComboBox *b, int value, int eepromType, int noends )
{
	int32_t limit = noends ? MaxSwitchIndex-1 : MaxSwitchIndex ;
  for(int i =- limit ; i <= limit ; i++ )
	{
    b->addItem(getMappedSWName(i,eepromType)) ;
	}
  b->setCurrentIndex(switchUnMap(value) + limit ) ;
  b->setMaxVisibleItems(10);
}

int t_radioData::getSwitchCbValue( QComboBox *b, int eepromType, int noends )
{
	int value ;
	int limit = noends ? MaxSwitchIndex-1 : MaxSwitchIndex ;
	value = b->currentIndex()-limit ;
	value = switchMap( value ) ;
  return value ;
}

extern QString TelemItems[] ;
#define NUM_TELEM_ITEMS	42

void t_radioData::populateSourceCB(QComboBox *b, int stickMode, int telem, int value, int modelVersion)
{
  b->clear();

	if ( modelVersion >= 2 )
	{
		stickMode = 0 ;
	}
  for(int i=0; i<37; i++) b->addItem(getSourceStr(stickMode,i));
	{
		if ( telem )
		{
    	for(int i=1; i<=NUM_TELEM_ITEMS; i++)
			{
    	  b->addItem(TelemItems[i]);
			}
		}
	}
  b->setCurrentIndex(value);
  b->setMaxVisibleItems(10);
}

void t_radioData::populateTmrBSwitchCB(QComboBox *b, int value, int eepromType )
{
	int i ;
	b->clear();
	
	int limit = MaxSwitchIndex-1 ;
  for( i= -limit; i<= limit; i++)
    b->addItem(getMappedSWName(i,eepromType));
	for( i=1 ; i<=limit; i++)
	{
    b->addItem('m'+getMappedSWName(i,eepromType));
	}
	int j ;
  if ( value > V2TOGGLE_INDEX )
	{
    j = switchUnMap( value - V2TOGGLE_INDEX ) + MaxSwitchIndex - 1 ;
	}
	else
	{
		j = switchUnMap( value ) ;
	}

  b->setCurrentIndex(j+limit) ;
  b->setMaxVisibleItems(10);
}

int t_radioData::getTimerSwitchCbValue( QComboBox *b, int eepromType )
{
	int value ;
//  int x = ( (eepromType == 1 ) || ( eepromType == 2 ) ) ? 1 : 0 ;
  int limit = MaxSwitchIndex ;
	value = b->currentIndex()-(limit-1) ;
	if ( value > limit-1 )
	{
    value = switchMap( value - limit + 1 ) + V2TOGGLE_INDEX ;
	}
	else
	{
    value = switchMap( value ) ;
	}
	return value ;
}


#endif


