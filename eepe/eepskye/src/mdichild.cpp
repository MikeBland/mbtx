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
#include "pers.h"
#include "myeeprom.h"
#include "mdichild.h"
#include "file.h"
#include "modeledit.h"
#include "generaledit.h"
#include "avroutputdialog.h"
#include "burnconfigdialog.h"
#include "simulatordialog.h"
#include "printdialog.h"
#include "eeprom_rlc.h"
#include "wizarddialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>

namespace er9x
{
#undef eeprom_h
#undef NUM_SCALERS
#define EXTRA_CSW	6
#define EXTRA_VOICE_SW	8
  #include "../../eepe/src/myeeprom.h"
  V1ModelData EmodelData ;
	MixData *srcMix ;
}	

extern class simulatorDialog *SimPointer ;

MdiChild::MdiChild()
{

#ifdef SKY
    QSettings settings("er9x-eePskye", "eePskye");
		defaultModelVersion = settings.value("default-model-version", 0 ).toInt() + 1 ;
#endif
	
    setAttribute(Qt::WA_DeleteOnClose);
    //setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint);

		radioData.valid = 0 ;
		changed = false ;
		defaultModelType = 1 ;

	// fill file system
	uint32_t i ;
	uint32_t max_models = MAX_IMODELS ;

	for ( i = 0 ; i < max_models + 1 ; i += 1 )
	{
    radioData.File_system[i].block_no = i * 2 ;
		radioData.File_system[i].size = 0 ;
		radioData.File_system[i].sequence_no = 1 ;
		radioData.File_system[i].flags = 0 ;
	}
	for ( i = 1 ; i <= max_models ; i += 1 )
	{
    radioData.ModelNames[i][0] = '\0' ;
	}
	radioData.type = 0 ;
	radioData.extraPots = 0 ;

	generalDefault() ;

    this->setFont(QFont("Courier New",12));
    refreshList();
    if(!(this->isMaximized() || this->isMinimized())) this->adjustSize();
    isUntitled = true;

    connect(this, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(OpenEditWindow()));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(ShowContextMenu(const QPoint&)));
    connect(this,SIGNAL(currentRowChanged(int)), this,SLOT(viableModelSelected(int)));

    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
	radioData.extraPots = countExtraPots( &radioData.generalSettings) ;

}


uint32_t MdiChild::countExtraPots(EEGeneral *g_eeGeneral)
{
	uint32_t count = 0 ;
	if ( g_eeGeneral->extraPotsSource[0] )
	{
		count = 1 ;
	}
	if ( g_eeGeneral->extraPotsSource[1] )
	{
		count += 1 ;
	}
	if ( g_eeGeneral->extraPotsSource[2] )
	{
		count += 1 ;
	}
	if ( g_eeGeneral->extraPotsSource[3] )
	{
		count += 1 ;
	}
	return count ;
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

		if ( radioData.valid )
		{
      if ( radioData.File_system[0].size )
			{
      	strncpy( buf, (char *)radioData.generalSettings.ownerName, sizeof(radioData.generalSettings.ownerName) ) ;
			}
		}
    QString str = QString(buf).trimmed();
    if(!str.isEmpty())
        str.prepend(" - ");
    addItem(tr("General Settings") + str);
		uint32_t max_models = MAX_MODELS ;
    if ( radioData.bitType & ( RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO | RADIO_BITTYPE_AR9X | RADIO_BITTYPE_9XTREME ) )
		{
			max_models = MAX_IMODELS ;
		}

    for(uint8_t i=0; i< max_models ; i++)
    {
      buf[0]='0'+(i+1)/10;
      buf[1]='0'+(i+1)%10;
    	buf[2]=':';
    	buf[3]=' ';
			if ( radioData.valid )
			{
				if ( radioData.File_system[i+1].size )
				{
		      strncpy( &buf[4], (char *)&radioData.ModelNames[i+1][0], 15 ) ;
	    		buf[14] = '\0';
				}
				else
				{
	    		buf[4] = '\0';
				}
			}
			else
			{
	    	buf[4] = '\0';
			}
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
           		if(index.row()>0)
							{
                radioData.File_system[index.row()].size = 0 ;
					 	//XXXXXXXXXXXX
//							 	eeFile.DeleteModel(index.row());
							}
           }
           setModified();
    }
}

QString MdiChild::modelName(int id)
{
    if( radioData.File_system[id+1].size)
    {
        char buf[sizeof(SKYModelData().name)+1];
        
        memcpy( buf, radioData.ModelNames[id+1], sizeof(SKYModelData().name) ) ;
        buf[sizeof(SKYModelData().name)] = '\0' ;
        
				return QString(buf).trimmed();
    }
    else
        return "";
}

QString MdiChild::ownerName()
{
//    EEGeneral tgen;
		//XXXXXXXXXXXXX
//    eeFile.getGeneralSettings(&tgen);

    if( radioData.File_system[0].size)
		{
    	char buf[sizeof(radioData.generalSettings.ownerName)+1];
      memcpy( buf, (char *)radioData.generalSettings.ownerName, sizeof(radioData.generalSettings.ownerName) ) ;
			buf[sizeof(radioData.generalSettings.ownerName)] = '\0' ;
			return QString::fromLatin1(buf).trimmed();
		}
    else
			return "";
}

int MdiChild::eepromVersion()
{
//    EEGeneral tgen;
		//XXXXXXXXXXXXX
//    eeFile.getGeneralSettings(&tgen);
    return radioData.generalSettings.myVers;
}

void MdiChild::doCopy(QByteArray *gmData)
{
    foreach(QModelIndex index, this->selectionModel()->selectedIndexes())
    {
        if(!index.row())
        {
            EEGeneral tgen;
		//XXXXXXXXXXXXX
		      if ( radioData.File_system[0].size )
          {
						memcpy( &tgen, &radioData.generalSettings, sizeof( tgen ) ) ;
            gmData->append('g');
            gmData->append((char*)&tgen,sizeof(tgen));
          }
        }
        else
        {
          if ( radioData.File_system[index.row()].size )
          {
            SKYModelData tmod;
            memcpy( &tmod, &radioData.models[index.row()-1], sizeof( tmod ) ) ;
            gmData->append('m');
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
    uint32_t id = index;
    if(!id) id++;
		uint32_t max_models = MAX_MODELS ;
    if ( radioData.bitType & ( RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO | RADIO_BITTYPE_AR9X ) )
		{
			max_models = MAX_IMODELS ;
		}

    while((i<gmData->size()) && (id<=max_models))
    {
        char c = *gData;
        i++;
        gData++;
        if(c=='g')  //general settings
        {
		//XXXXXXXXXXXXX
			  	memcpy( &radioData.generalSettings, gData, sizeof(radioData.generalSettings) ) ;
          radioData.File_system[0].size = sizeof(radioData.generalSettings) ;
            
//						if(!eeFile.putGeneralSettings((EEGeneral*)gData))
//            {
//                QMessageBox::critical(this, tr("Error"),tr("Unable set data!"));
//                break;
//            }
            gData += sizeof(EEGeneral);
            i     += sizeof(EEGeneral);
        }
        else if(c=='m') //model data
        {
		//XXXXXXXXXXXXX
          memcpy( &radioData.models[id-1], gData, sizeof(radioData.models[0] ) ) ;
          setModelFile( id-1 ) ;
//            if(!eeFile.putModel((ModelData*)gData,id-1))
//            {
//                QMessageBox::critical(this, tr("Error"),tr("Unable set model!"));
//                break;
//            }
            gData += sizeof(SKYModelData);
            i     += sizeof(SKYModelData);
            id++;
        }
        else if( (c& 0x7F)=='M') // er9x model data
				{
          int size = sizeof(er9x::EmodelData ) ;
          memcpy( &er9x::EmodelData, gData, size ) ;//of(er9x::EmodelData ) ) ;
					convertFromEr9x( &radioData.models[id-1], (c & 0x80) ) ;
          setModelFile( id-1 ) ;
          gData += sizeof(er9x::EmodelData);
          i     += sizeof(er9x::EmodelData);
          id++;
          QMessageBox::critical(this, tr("Warning"),
                        tr("Only part pasted from er9x file\nPlease check model settings"));
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
    QSettings settings("er9x-eePskye", "eePskye");


    if(!fn.isEmpty())
    {
        if(!QFileInfo(fn).exists())
        {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Couldn't find %1")
                                  .arg(fn));
            return false;
        }

        fileName = fn;
    }
    else if(genfile)
    {
        char buf[sizeof(EEGeneral().ownerName)+1];

		//XXXXXXXXXXXXX
//        eeFile.eeLoadOwnerName(buf,sizeof(buf));
      	strncpy( buf, (char *)radioData.generalSettings.ownerName, sizeof(radioData.generalSettings.ownerName) ) ;
         buf[sizeof(EEGeneral().ownerName)] = '\0' ;
        QString str = QString(buf).trimmed();
        if(!str.isEmpty())
        {
            int ret = QMessageBox::warning(this, "eePskye",
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
		//XXXXXXXXXXXXX
    if(radioData.File_system[cmod+1].size > 0)		//			  if(eeFile.eeModelExists(cmod))
        {
//            char buf[sizeof(ModelData().name)+1];
		//XXXXXXXXXXXXX
//            eeFile.getModelName(cmod,(char*)&buf);
//            QString cmodelName = QString(buf).trimmed();
            QString cmodelName = modelName(cmod).trimmed() ;

            int ret = QMessageBox::warning(this, "eePskye",
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

        QDomDocument doc(ERSKY9X_EEPROM_FILE_TYPE);
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

		//XXXXXXXXXXXXX
//        if(!eeFile.putGeneralSettings(&tgen))
//        {
//            QMessageBox::critical(this, tr("Error"),
//                                  tr("Error writing to container"));
//            return false;
//        }
				radioData.File_system[0].size = sizeof(tgen) ;
			  memcpy( &radioData.generalSettings, &tgen, sizeof(tgen) ) ;
				radioData.valid = 1 ;
			  refreshList() ;
    }
    else
    {
        SKYModelData tmod;

        QDomDocument doc(ERSKY9X_EEPROM_FILE_TYPE);
        QFile file(fileName);
        bool xmlOK = file.open(QIODevice::ReadOnly);
        if(xmlOK)
        {
            xmlOK = doc.setContent(&file);
            if(xmlOK)
            {
							QDomDocumentType type = doc.doctype() ;
							if ( ( type.name() == ERSKY9X_MODEL_FILE_TYPE ) || ( type.name() == ERSKY9X_EEPROM_FILE_TYPE ) )
							{
              	xmlOK = loadModelDataXML(&doc, &tmod);
              	getNotesFromXML(&doc, -cmod ) ;
							}
							else if ( ( type.name() == ER9X_MODEL_FILE_TYPE ) || ( type.name() == ER9X_EEPROM_FILE_TYPE ) )
							{
        				SKYModelData emod;
								
              	xmlOK = loadModelDataXML( &doc, &emod ) ;
              	memcpy( &er9x::EmodelData, &emod, sizeof(er9x::EmodelData) ) ;
								convertFromEr9x( &tmod, 0 ) ;
              	getNotesFromXML(&doc, -cmod ) ;
			          QMessageBox::critical(this, tr("Warning"),
                        tr("Only part pasted from er9x file\nPlease check model settings"));
							}
            }
            file.close();
        }

        if(!xmlOK) //if can't get XML - load iHEX
        {

            //if can't load XML load from iHex
//            quint8 temp[sizeof(ModelData)];
//            if(!loadiHEX(this, fileName, (quint8*)&temp, sizeof(ModelData), EEPE_MODEL_FILE_HEADER))
                return false;
//            memcpy(&tmod, &temp, sizeof(tmod));
        }

			memcpy( &radioData.models[cmod], &tmod, sizeof( tmod ) ) ;
			
			setModelFile( cmod ) ;

		//XXXXXXXXXXXXX
//        if(!eeFile.putModel(&tmod,cmod))
//        {
//            QMessageBox::critical(this, tr("Error"),
//                                  tr("Error writing to container"));
//            return false;
//        }
    }

    refreshList();
    setModified();

    return true;
}

void MdiChild::saveModelToFile()
{
    int cmod = currentRow()-1;
    bool genfile = currentRow()==0;

//    ModelData tmod;
//    EEGeneral tgen;
    QString fileName;
    QSettings settings("er9x-eePskye", "eePskye");


    if(genfile)
    {
		//XXXXXXXXXXXXX
        if(radioData.File_system[0].size == 0)
//        if(!eeFile.getGeneralSettings(&tgen))
        {
            QMessageBox::critical(this, tr("Error"),tr("Error No General Settings Data"));
            return;
        }

        QString ownerName = QString::fromLatin1(radioData.generalSettings.ownerName,sizeof(radioData.generalSettings.ownerName)).trimmed() + ".eepg";

        fileName = QFileDialog::getSaveFileName(this, tr("Save Settings As"),settings.value("lastDir").toString() + "/" +ownerName,tr(EEPG_FILES_FILTER));
    }
    else
    {
		//XXXXXXXXXXXXX
				
        if(radioData.File_system[cmod+1].size == 0)
        {
            QMessageBox::critical(this, tr("Error"),tr("Error Getting Model #%1").arg(cmod+1));
            return;
        }

		//XXXXXXXXXXXXX
//        if(!eeFile.getModel(&tmod,cmod))
//        {
//            QMessageBox::critical(this, tr("Error"),tr("Error Getting Model #%1").arg(cmod+1));
//            return;
//        }

        QString model_Name = modelName(cmod) ;

        fileName = QFileDialog::getSaveFileName(this, tr("Save Model As"),settings.value("lastDir").toString() + "/" +model_Name,tr(EEPM_FILES_FILTER));
    }

    if (fileName.isEmpty())
        return;

    settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

//    if(genfile)
//        saveiHEX(this, fileName, (quint8*)&tgen, sizeof(tgen), EEPE_GENERAL_FILE_HEADER);
//    else
//        saveiHEX(this, fileName, (quint8*)&tmod, sizeof(tmod), EEPE_MODEL_FILE_HEADER, cmod);

    QFile file(fileName);

    QDomDocument doc(ERSKY9X_EEPROM_FILE_TYPE);
    QDomElement root = doc.createElement(ERSKY9X_EEPROM_FILE_TYPE);
    doc.appendChild(root);

    if(genfile) // general data
    {
//            EEGeneral tgen;
		//XXXXXXXXXXXXX
        		if(radioData.File_system[0].size == 0)
//            if(!eeFile.getGeneralSettings(&tgen))
            {
                QMessageBox::critical(this, tr("Error"),tr("Error Getting General Settings Data"));
                return;
            }
            QDomElement genData = getGeneralDataXML(&doc, &radioData.generalSettings);
            root.appendChild(genData);
    }
    else  // model data - cmod
    {
        saveModelToXML(&doc, &root, cmod, MDSKYVERS);
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
    uint32_t i = this->currentRow();
		uint32_t max_models = MAX_MODELS ;
    if ( radioData.bitType & ( RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO | RADIO_BITTYPE_AR9X ) )
		{
			max_models = MAX_IMODELS ;
		}
    if(i && i<max_models)
    {
			i -= 1 ;
//      ModelData gmodel;
//      SKYModelData gmodel1;
    //XXXXXXXXXXXXX
//        if(eeFile.getModel(&gmodel,--i))
//        {
            uint32_t j = i+1;
		//XXXXXXXXXXXXX
            while(j<max_models && radioData.File_system[j+1].size) j++;
		//XXXXXXXXXXXXX
            if(j<max_models)
						{
//							 eeFile.putModel(&gmodel,j);
      	      memcpy( &radioData.models[j], &radioData.models[i], sizeof( radioData.models[0] ) ) ;
			        setModelFile( j ) ;
							setModified();
						}
//        }
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

        if( !radioData.File_system[i].size )
        {
            modelDefault(i-1);
            isNew = true;//modeledit - clear mixes, apply first template
            setModified();
        }

				radioData.extraPots = countExtraPots( &radioData.generalSettings) ;
    		QString mname = modelName(i-1) ;
        ModelEdit *t = new ModelEdit( &radioData,(i-1),this);
        
				if(isNew) t->applyBaseTemplate();
				QString type = radioData.type ? " (Taranis)" : " (Sky)" ;
				if ( radioData.type == RADIO_TYPE_TPLUS )
				{
    		  type = radioData.sub_type ? " (Taranis X9E)" : " (Taranis Plus)" ;
				}
				else if ( radioData.type == RADIO_TYPE_9XTREME )
				{
    			type = " (9Xtreme)" ;
				}
				else if ( radioData.type == RADIO_TYPE_QX7 )
				{
    			type = " (Taranis QX7)" ;
				}
        t->setWindowTitle(tr("Editing model %1: ").arg(i) + mname + type ) ;

        for(int j=0; j<MAX_SKYMIXERS; j++)
            t->setNote(j,modelNotes[i-1][j]);
        t->refreshMixerList();

        connect(t,SIGNAL(modelValuesChanged(ModelEdit*)),this,SLOT(setModified(ModelEdit*)));
        //t->exec();
				t->show();
    }
    else
    {
        //TODO error checking
        if( !radioData.File_system[i].size )
				{
					generalDefault() ;
				}

        //setModified();
        GeneralEdit *t = new GeneralEdit(&radioData, this);
        connect(t,SIGNAL(modelValuesChanged()),this,SLOT(setModified()));
        t->show();
				radioData.extraPots = countExtraPots( &radioData.generalSettings) ;
    }

}

void MdiChild::getPhysicalType()
{
	uint32_t x ;
	radioData.sub_type = 0 ;
	x = radioData.generalSettings.physicalRadioType ;
	if ( x && x < 9 )
	{
		switch ( x )
		{
			case PHYSICAL_SKY :
				radioData.type = RADIO_TYPE_SKY ;
				radioData.bitType = RADIO_BITTYPE_SKY ;
			break ;
			case PHYSICAL_9XRPRO :
				radioData.type = RADIO_TYPE_SKY ;
				radioData.bitType = RADIO_BITTYPE_9XRPRO ;
			break ;
			case PHYSICAL_AR9X :
				radioData.type = RADIO_TYPE_SKY ;
				radioData.bitType = RADIO_BITTYPE_AR9X ;
			break ;
			case PHYSICAL_TARANIS :
				radioData.type = RADIO_TYPE_TARANIS ;
				radioData.bitType = RADIO_BITTYPE_TARANIS ;
			break ;
			case PHYSICAL_TARANIS_PLUS :
				radioData.type = RADIO_TYPE_TPLUS ;
				radioData.bitType = RADIO_BITTYPE_TPLUS ;
			break ;
			case PHYSICAL_TARANIS_X9E :
				radioData.type = RADIO_TYPE_TPLUS ;
				radioData.bitType = RADIO_BITTYPE_X9E ;
				radioData.sub_type = 1 ;
			break ;
			case PHYSICAL_9XTREME :
				radioData.type = RADIO_TYPE_9XTREME ;
				radioData.bitType = RADIO_BITTYPE_9XTREME ;
			break ;
			case PHYSICAL_QX7 :
				radioData.type = RADIO_TYPE_QX7 ;
				radioData.bitType = RADIO_BITTYPE_QX7 ;
			break ;

		}
	}
	else
	{
    QSettings settings("er9x-eePskye", "eePskye");
		radioData.type = 0 ;
		radioData.bitType = 0 ;
		radioData.sub_type = 0 ;
		int x ;
		x = settings.value("download-version", 0).toInt() ;
		radioData.bitType = 1 << x ;
		if ( x >= 2 )
		{
			radioData.type = x-1 ;
		}
		radioData.T9xr_pro = 0 ;
		if ( x == 1 )
		{
			radioData.T9xr_pro = 1 ;
		}
		if ( x == 5 )
		{
			radioData.type = 2 ;
			radioData.sub_type = 1 ;
		}
		if ( x == 6 )
		{
			radioData.type = 0 ;
		}
	}
}

void MdiChild::newFile()
{
    static int sequenceNumber = 1 ;
    QSettings settings("er9x-eePskye", "eePskye");
		radioData.type = 0 ;
		radioData.bitType = 0 ;
		radioData.sub_type = 0 ;
		getPhysicalType() ;
		int x ;
		x = settings.value("download-version", 0).toInt() ;
		radioData.bitType = 1 << x ;
		if ( x >= 2 )
		{
			radioData.type = x-1 ;
		}
		radioData.T9xr_pro = 0 ;
		if ( x == 1 )
		{
			radioData.T9xr_pro = 1 ;
		}
    QString type = radioData.type ? " (Taranis)" : " (Sky)" ;
		if ( x == 3 )
		{
    	type = " (Taranis Plus)" ;
		}
		else if ( x == 4 )
		{
    	type = " (9Xtreme)" ;
		}
		if ( x == 5 )
		{
			radioData.type = RADIO_TYPE_TPLUS ;
    	type = " (Taranis X9E)" ;
			radioData.sub_type = 1 ;
		}
		if ( x == 6 )
		{
			radioData.type = 0 ;
    	type = " (Sky AR9X)" ;
		}

		if ( x == 7 )
		{
			radioData.type = RADIO_TYPE_QX7 ;
    	type = " (Taranis QX7)" ;
		}

    isUntitled = true;
    curFile = tr("document%1.bin").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]" + type );
		refreshList() ;

}

uint8_t temp[EEFULLSIZE];

bool MdiChild::loadFile(const QString &fileName, bool resetCurrentFile)
{
    if(!QFileInfo(fileName).exists())
    {
        QMessageBox::critical(this, tr("Error"),tr("Unable to find file %1!").arg(fileName));
        return false;
    }


    int fileType = getFileType(fileName);

//    if(fileType==FILE_TYPE_EEPM || fileType==FILE_TYPE_EEPG)
//    {
//        //load new file and paste in
//        newFile();

//        setCurrentRow(fileType==FILE_TYPE_EEPG ? 0 : 1);
//        if(!loadModelFromFile(fileName))
//        {
//            QMessageBox::critical(this, tr("Error"),
//                                 tr("Error loading file %1:\n"
//                                    "File may be corrupted, old or from a different system.\n"
//                                    "You might need to update eePe to read this file.")
//                                 .arg(fileName));
//            return false;
//        }

//        refreshList();
//        if(resetCurrentFile) setCurrentFile(fileName);

//        return true;
//    }

//    if(fileType==FILE_TYPE_HEX || fileType==FILE_TYPE_EEPE) //read HEX file
    if(fileType==FILE_TYPE_EEPE) //read HEX file
    {
        //if file is XML read and exit saying true;
//        //else process as iHex
        QDomDocument doc(ERSKY9X_EEPROM_FILE_TYPE);
        QFile file(fileName);
        bool xmlOK = file.open(QIODevice::ReadOnly);
        if(xmlOK)
        {
          xmlOK = doc.setContent(&file);
          if(xmlOK)
          {
        		EEGeneral tgen;
            
						if(xmlOK)
            {
                xmlOK = loadGeneralDataXML(&doc, &tgen);
            }
            file.close();
                
            radioData.File_system[0].size = sizeof(tgen) ;
			  		memcpy( &radioData.generalSettings, &tgen, sizeof(tgen) ) ;
						getPhysicalType() ;
          }
					uint32_t max_models = MAX_MODELS ;
          if ( radioData.bitType & ( RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO | RADIO_BITTYPE_AR9X ) )
					{
						max_models = MAX_IMODELS ;
					}

          for(uint32_t i=0; i<max_models; i++)
          {
            SKYModelData tmod;
            memset(&tmod,0,sizeof(tmod));
            if(loadModelDataXML(&doc, &tmod, i))
            {
              radioData.File_system[i+1].size = sizeof(tmod) ;
              memcpy( &radioData.models[i], &tmod, sizeof( tmod ) ) ;
              memcpy( &radioData.ModelNames[i+1], &radioData.models[i].name, sizeof( radioData.models[0].name) ) ;
              radioData.ModelNames[i+1][sizeof( radioData.models[0].name)+1] = '\0' ;
              getNotesFromXML(&doc, i);
            }
          }
					radioData.valid = 1 ;
//          ee32_read_model_names(&radioData) ;
			  	refreshList() ;
					return true ;
        }

//        if(!xmlOK)
//        {
//            if((QFileInfo(fileName).size()>(6*1024)) || (QFileInfo(fileName).size()<(4*1024)))  //if filesize> 6k and <4kb
//            {
//                QMessageBox::critical(this, tr("Error"),tr("Error reading file:\n"
//                                                           "This might be a FW file (er9x.hex?). \n"
//                                                           "You might want to try flashing it to the TX.\n"
//                                                           "(Burn->Write Flash Memory)").arg(fileName));
//                return false;
//            }


//            QString header ="";
//            if(fileType==FILE_TYPE_EEPE)   // read EEPE file header
//                header=EEPE_EEPROM_FILE_HEADER;

//            if(!loadiHEX(this, fileName, (uint8_t*)&temp, EESIZE, header))
//                return false;


//            if(!rawloadFile( &radioData, temp ) )
//            {
//                QMessageBox::critical(this, tr("Error"),
//                                      tr("Error loading file %1:\n"
//                                         "File may be corrupted, old or from a different system."
//                                         "You might need to update eePe to read this file.")
//                                      .arg(fileName));
//                return false;
//            }
//        }

//        refreshList();
//        if(resetCurrentFile) setCurrentFile(fileName);

//        return true;
    }


    if(fileType==FILE_TYPE_BIN) //read binary
    {
        QFile file(fileName);

        if( file.size() == 32768 )
				{
					// Taranis RLC file
	        if (!file.open(QFile::ReadOnly))
					{  //reading binary file
            QMessageBox::critical(this, tr("Error"),
                                 tr("Error opening file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return false;
	        }

          /*long result = */ file.read((char*)&temp,32768);
  	      file.close();
					
//					radioData.type = RADIO_TYPE_TARANIS ;
//					radioData.bitType = RADIO_BITTYPE_TARANIS ;
//					int x ;
//			    QSettings settings("er9x-eePskye", "eePskye");
//					x = settings.value("download-version", 0).toInt() ;
//					radioData.bitType = 1 << x ;
//					if ( x >= 2 )
//					{
//						radioData.type = x-1 ;
//					}
//					radioData.T9xr_pro = 0 ;
//					if ( x == 5 )
//					{
//						radioData.type = 2 ;
//						radioData.sub_type = 1 ;
//					}
//					if ( x == 6 )
//					{
//						radioData.type = 0 ;
//					}				


//						radioData.bitType = x == 3 ? RADIO_BITTYPE_TPLUS : RADIO_BITTYPE_X9E ;
	        
					if(!rawloadFileRlc( &radioData, temp) )
  	      {
            QMessageBox::critical(this, tr("Error"),
                                 tr("Error loading file %1:\n"
                                    "File may be corrupted, old or from a different system."
                                    "You might need to update eePe to read this file.")
                                 .arg(fileName));
            return false;
    	    }
					defaultModelType = DefaultModelType ;
					getPhysicalType() ;
        	refreshList();
        	if(resetCurrentFile) setCurrentFile(fileName);
          return true ;
				
//          QMessageBox::critical(this, tr("Error"),
//                                 tr("Error opening file %1:\n%2.")
//                                 .arg(fileName)
//                                 .arg("File wrong type (Taranis)"));
//          return false;
				}

        if( (file.size()!=EESIZE) && (file.size()!=EE20MSIZE) && (file.size()!=EEFULLSIZE) )
        {
            QMessageBox::critical(this, tr("Error"),tr("Error reading file:\n"
                                                       "File wrong size - %1 %2").arg(fileName).arg(file.size()));
            return false;
        }

				memset( temp, 0xFF, EESIZE ) ;	// Default in case only 20 models loading

        if (!file.open(QFile::ReadOnly)) {  //reading binary file   - TODO HEX support
            QMessageBox::critical(this, tr("Error"),
                                 tr("Error opening file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return false;
        }

        long result = file.read((char*)&temp,EEFULLSIZE);
        file.close();

        if (result==EE20MSIZE)
				{
					result = EEFULLSIZE ;
				}
        if (result==EESIZE)
        {
          result = EEFULLSIZE ;
        }


        if (result!=EEFULLSIZE)
        {
            QMessageBox::critical(this, tr("Error"),
                                 tr("Error reading file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));

            return false;
        }

        if(!rawloadFile( &radioData, temp) )
        {
            QMessageBox::critical(this, tr("Error"),
                                 tr("Error loading file %1:\n"
                                    "File may be corrupted, old or from a different system."
                                    "You might need to update eePe to read this file.")
                                 .arg(fileName));
            return false;
        }
				defaultModelType = DefaultModelType ;
				radioData.type = 0 ;
				radioData.bitType = RADIO_BITTYPE_SKY ;

				getPhysicalType() ;
				
//				int x ;
//		    QSettings settings("er9x-eePskye", "eePskye");
//				x = settings.value("download-version", 0).toInt() ;
//				if ( ( x == 4 ) || ( radioData.generalSettings.is9Xtreme ) )
//				{
//					radioData.type = 3 ;	// 9Xtreme
//					radioData.T9xr_pro = 1 ;
//					radioData.bitType = RADIO_BITTYPE_9XTREME ;
//				}
//				if ( x == 6 )
//				{
//					radioData.bitType = RADIO_BITTYPE_AR9X ;
//				}
        refreshList();
        if(resetCurrentFile) setCurrentFile(fileName);

        return true;
    }

    return false;
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
    QSettings settings("er9x-eePskye", "eePskye");
		
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),settings.value("lastDir").toString() + "/" +strippedName(curFile),tr(EEPROM_FILES_FILTER));
    if (fileName.isEmpty())
        return false;

    settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());
    return saveFile(fileName);
}


void MdiChild::getNotesFromXML(QDomDocument * qdoc, int model_id)
{
	int whichModel = model_id ;

	if ( model_id < 0 )
	{
		whichModel = -model_id ;
		model_id = 0 ;
	}
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
            modelNotes[whichModel][mixNum] = e.firstChild().toText().data();
        }
        n = n.nextSibling();
    }
}

void MdiChild::saveModelToXML(QDomDocument * qdoc, QDomElement * pe, int model_id, int mdver)
{
		//XXXXXXXXXXXXX
    if(radioData.File_system[model_id+1].size > 0)		//			 eeFile.eeModelExists(model_id))
    {
//        SKYModelData tmod;
//        ModelData tmod1;
		//XXXXXXXXXXXXX
//        if(!eeFile.getModel(&tmod1,model_id))  // if can't get model - exit
//        {
//            return;
//        }
        QDomElement modData = getModelDataXML(qdoc, &radioData.models[model_id], model_id, mdver);
        pe->appendChild(modData);

        //add notes to model data
        QDomElement eNotes = qdoc->createElement("Notes");

        int numNodes = 0;
        for(int i=0; i<MAX_SKYMIXERS; i++)
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
        QDomDocument doc(ERSKY9X_EEPROM_FILE_TYPE);
        QDomElement root = doc.createElement(ERSKY9X_EEPROM_FILE_TYPE);
        doc.appendChild(root);

        //Save General Data
//        EEGeneral tgen;
//        memset(&tgen,0,sizeof(tgen));
//        if(!eeFile.getGeneralSettings(&tgen))
//        {
//            QMessageBox::critical(this, tr("Error"),tr("Error Getting General Settings Data"));
//            return false;
//        }
//        tgen.myVers = MDVERS; //make sure we're at the current rev
//        QDomElement genData = getGeneralDataXML(&doc, &tgen);
        QDomElement genData = getGeneralDataXML(&doc, &radioData.generalSettings ) ;
        root.appendChild(genData);

        //Save model data one by one
				uint32_t max_models = MAX_MODELS ;
        if ( radioData.bitType & ( RADIO_BITTYPE_SKY | RADIO_BITTYPE_9XRPRO | RADIO_BITTYPE_AR9X ) )
				{
					max_models = MAX_IMODELS ;
				}
        
        for(uint32_t i=0; i<max_models; i++)
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


//    if(fileType==FILE_TYPE_HEX) //write hex
//    {
//        quint8 temp[EESIZE];
//        eeFile.saveFile(&temp);
//        QString header = "";
//        saveiHEX(this, fileName, (quint8*)&temp, EESIZE, header, NOTES_ALL);


//        if(setCurrent) setCurrentFile(fileName);
//        return true;
//    }

    if(fileType==FILE_TYPE_BIN) //write binary
    {
        if (!file.open(QFile::WriteOnly)) {
            QMessageBox::warning(this, tr("Error"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return false;
        }

//        uint8_t temp[EESIZE];		// Now a global
        
        rawsaveFile( &radioData, temp);
				int fileSize = EEFULLSIZE ;
				if ( radioData.bitType & (RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_QX7) )
				{
					fileSize = 32768 ;
				}

        long result = file.write((char*)&temp,fileSize);
        if(result != fileSize)
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
		//XXXXXXXXXXXXX
//    setWindowModified(eeFile.Changed());
		changed = true ;
    setWindowModified(true);
}

bool MdiChild::maybeSave()
{
		//XXXXXXXXXXXXX
    if (changed)
		{
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("eePskye"),
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
		int x ;
		x = radioData.type ;
    QString type = x ? " (Taranis)" : " (Sky)" ;

		if ( x == RADIO_TYPE_SKY )
		{
			if ( radioData.T9xr_pro )
			{
    		type = " (Sky 9XR-PRO)" ;
			}
			if ( radioData.generalSettings.ar9xBoard )
			{
	    	type = " (Sky AR9X)" ;
			}
		}
		if ( x == RADIO_TYPE_TPLUS )
		{
			if ( radioData.sub_type )
			{
    		type = " (Taranis X9E)" ;
			}
			else
			{
    		type = " (Taranis Plus)" ;
			}
		}
		else if ( x == RADIO_TYPE_9XTREME )
		{
    	type = " (9Xtreme)" ;
		}
		else if ( x == RADIO_TYPE_QX7 )
		{
    	type = " (Taranis QX7)" ;
		}
//		if ( x == 5 )
//		{
//    	type = " (Sky AR9X)" ;
//		}
//		if ( radioData.type == 3 )
//		{
// 			type = " (9Xtreme)" ;
//		}
    
		curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
		//XXXXXXXXXXXXX
//    eeFile.setChanged(false);
		changed = false ;
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]" + type);
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

//void MdiChild::optimizeEEPROM()
//{
//    //save general settings and model data in external buffer
//    //format eeprom
//    //write settings back to eeprom

//	return ; ;
//    EEGeneral tgen;
//    ModelData mgen[MAX_MODELS];

//    memset(&tgen, 0, sizeof(tgen));
//    memset(&mgen, 0, sizeof(mgen));

//		//XXXXXXXXXXXXX
//    eeFile.getGeneralSettings(&tgen);
//    for(int i=0; i<MAX_MODELS; i++)
//		//XXXXXXXXXXXXX
//        eeFile.getModel(&mgen[i],i);

//		//XXXXXXXXXXXXX
//    eeFile.formatEFile();

//		//XXXXXXXXXXXXX
//    eeFile.putGeneralSettings(&tgen);
//    for(int i=0; i<MAX_MODELS; i++)
//		//XXXXXXXXXXXXX
//        eeFile.putModel(&mgen[i],i);
//}

QStringList MdiChild::GetSambaArguments(const QString &tcl)
{
  QStringList result;
  burnConfigDialog bcd ;

	if ( bcd.getUseSamba() == 0 )
	{
    return result ;
	}
  
	QString tclFilename = QDir::tempPath() + "/temp.tcl";
  if (QFile::exists(tclFilename)) {
    remove(tclFilename.toLatin1());
  }
  QFile tclFile(tclFilename);
  if (!tclFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, tr("Error"),
        tr("Cannot write file %1:\n%2.")
        .arg(tclFilename)
        .arg(tclFile.errorString()));
    return result;
  }

  QTextStream outputStream(&tclFile);
  outputStream << tcl;

  result << bcd.getARMPort() << bcd.getMCU() << tclFilename ;
  return result;

}

void MdiChild::burnTo()  // write to Tx
{

    QMessageBox::StandardButton ret = QMessageBox::question(this, tr("eePskye"),
                 tr("Write %1 to EEPROM memory?").arg(strippedName(curFile)),
                 QMessageBox::Yes | QMessageBox::No);

//    optimizeEEPROM();

    if (ret == QMessageBox::Yes)
    {
        burnConfigDialog bcd;
        QString avrdudeLoc = bcd.getAVRDUDE();
        QString tempDir    = QDir::tempPath();
//        QString programmer = bcd.getProgrammer();
//        QStringList args   = bcd.getAVRArgs();
//        if(!bcd.getPort().isEmpty()) args << "-P" << bcd.getPort();

        QString tempFile = tempDir + "/temp.bin";
        saveFile(tempFile, false);
        if(!QFileInfo(tempFile).exists())
        {
            QMessageBox::critical(this,tr("Error"), tr("Cannot write temporary file!"));
            return;
        }
//        QString str = "eeprom:w:" + tempFile + ":i"; // writing eeprom -> MEM:OPR:FILE:FTYPE"

        QStringList arguments = GetSambaArguments(QString("SERIALFLASH::Init 0\n") + "send_file {SerialFlash AT25} \"" + tempFile + "\" 0x0 0\n");
//        arguments << "-c" << programmer << "-p" << "m64" << args << "-U" << str;

				if ( arguments.isEmpty() )
				{
					// Not using SAM-BA
					QString path ;
					path = FindErskyPath( 0 ) ;	// EEPROM
	  			if ( path.isEmpty() )
					{
    			  QMessageBox::critical(this, "eePskye", tr("Tx Disk Not Mounted" ) ) ;
    			  return;
					}
					else
					{
      			qint32 fsize ;
//						fsize = (MAX_IMODELS+1)*8192 ;
						fsize = 64*8192 ;
						if ( radioData.bitType & (RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E) )
						{
							fsize = 32768 ;			// Taranis EEPROM
						}
						avrdudeLoc = "" ;
    			  arguments << path << tempFile << tr("%1").arg(fsize) << "0" ;
	  			  avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments,tr("Write EEPROM To Tx")); //, AVR_DIALOG_KEEP_OPEN);
//						res = ad->result() ;
						delete ad ;
					}
				}
  			else
				{
        	avrOutputDialog *ad = new avrOutputDialog(this, avrdudeLoc, arguments, "Write EEPROM To Tx", AVR_DIALOG_SHOW_DONE);
        	ad->setWindowIcon(QIcon(":/images/write_eeprom.png"));
        	ad->show();
				}
    }
}

bool MdiChild::saveToFileEnabled()
{
    int crow = currentRow();
    if(crow==0)
        return true;

		return radioData.File_system[crow].size > 0 ;
		//XXXXXXXXXXXXX
//    return eeFile.eeModelExists(crow-1);
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
//    eeFile.setChanged(true);
    documentWasModified();

    if(me)
    {
        int id = me->getModelID();
        for(int j=0; j<MAX_SKYMIXERS; j++)
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
    sd->loadParams(gg,gm, radioData.type);
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
        emit copyAvailable(radioData.File_system[currentRow()].size);

    emit saveModelToFileAvailable(saveToFileEnabled());
}


void MdiChild::generalDefault()
{
  memset(&radioData.generalSettings,0,sizeof(radioData.generalSettings));
  memset(&radioData.generalSettings.ownerName,' ',sizeof(radioData.generalSettings.ownerName));
  radioData.generalSettings.myVers   =  MDSKYVERS;
  radioData.generalSettings.currModel=  0;
  radioData.generalSettings.contrast = 30;
  radioData.generalSettings.vBatWarn = 90;
  radioData.generalSettings.stickMode=  1;
	radioData.generalSettings.disablePotScroll=  1;
	radioData.generalSettings.bright = 50 ;
	radioData.generalSettings.volume = 2 ;
  for (int i = 0; i < 7; ++i) {
    radioData.generalSettings.calibMid[i]     = 0x300;
    radioData.generalSettings.calibSpanNeg[i] = 0x400;
    radioData.generalSettings.calibSpanPos[i] = 0x300;
  }
  int16_t sum=0;
  for(int i=0; i<12;i++) sum+=radioData.generalSettings.calibMid[i];
  radioData.generalSettings.chkSum = sum;
	
	// Now update the trainer values if necessary.
  uint32_t i ;
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		if ( radioData.generalSettings.trainer.mix[i].swtch != -16 )
		{
			radioData.generalSettings.exTrainer[i].swtch = radioData.generalSettings.trainer.mix[i].swtch ;
			radioData.generalSettings.exTrainer[i].studWeight = radioData.generalSettings.trainer.mix[i].studWeight * 13 / 4 ;
			radioData.generalSettings.trainer.mix[i].swtch = -16 ;
		}
	}

  QSettings settings("er9x-eePskye", "eePskye");
  radioData.generalSettings.templateSetup = settings.value("default_channel_order", 0).toInt();
  radioData.generalSettings.stickMode = settings.value("default_mode", 1).toInt();
	radioData.File_system[0].size = sizeof(radioData.generalSettings) ;
	radioData.valid = 1 ;
//  putGeneralSettings(&g_eeGeneral);
}


void MdiChild::modelDefault(uint8_t id)
{
  if( !radioData.File_system[0].size )
	{
		generalDefault() ;
	}
	
  memset(&radioData.models[id],0,sizeof(radioData.models[0])) ;
  memset(&radioData.models[id].name, ' ', sizeof(radioData.models[0].name));
  strcpy(radioData.models[id].name,"MODEL");
  radioData.models[id].name[5]='0'+(id+1)/10;
  radioData.models[id].name[6]='0'+(id+1)%10;
//  g_model.mdVers = MDVERS;
  radioData.models[id].trimInc = 2 ;

  for(uint8_t i= 0; i<4; i++){
    radioData.models[id].mixData[i].destCh = i+1;
    radioData.models[id].mixData[i].srcRaw = i+1;
    radioData.models[id].mixData[i].weight = 100;
  }

  radioData.models[id].modelVersion = 1 ; //update mdvers
	if ( ( defaultModelType > 1 ) || (defaultModelVersion > 1 ) )
	{
    modelConvert1to2( &radioData.generalSettings, &radioData.models[id] ) ;
	}					 
	if (defaultModelVersion > 2 )
	{
		for (uint8_t i = 0 ; i < NUM_SKYCSW ; i += 1 )
		{
      SKYCSwData *cs = &radioData.models[id].customSw[i];
			if ( cs->func == CS_LATCH )
			{
				cs->func = CS_GREATER ;
			}
			if ( cs->func == CS_FLIP )
			{
				cs->func = CS_LESS ;
			}
		}
		radioData.models[id].modelVersion = 3 ;
	}
  if ( defaultModelVersion > 3 )
	{
    radioData.models[id].modelVersion = 4 ;
		radioData.models[id].Module[0].protocol = PROTO_OFF ;
		radioData.models[id].Module[1].protocol = PROTO_OFF ;
	}
	else
	{
//	if ( radioData.bitType & (RADIO_BITTYPE_TARANIS | RADIO_BITTYPE_TPLUS | RADIO_BITTYPE_X9E | RADIO_BITTYPE_9XTREME | RADIO_BITTYPE_QX7 ) )
//	{
		radioData.models[id].protocol = PROTO_OFF ;
		radioData.models[id].xprotocol = PROTO_OFF ;
	}

	setModelFile( id ) ;

//  putModel(&g_model,id);
}

void MdiChild::setModelFile(uint8_t id)
{
	radioData.File_system[id+1].size = sizeof(radioData.models[0]) ;
	memcpy( &radioData.ModelNames[id+1], &radioData.models[id].name, sizeof( radioData.models[0].name) ) ;
	radioData.ModelNames[id+1][sizeof( radioData.models[0].name)+1] = '\0' ;
	radioData.valid = 1 ;
  refreshList() ;
}

void convertEr9xSwitch( int8_t *p )
{
	int x = *p ;
	*p = x ;
}


void MdiChild::convertFromEr9x( SKYModelData *dest, uint8_t type )
{
	uint32_t i ;
	
  er9x::V1ModelData *source ;
	source = &er9x::EmodelData ;

	memset( dest, 0, sizeof(*dest) ) ;
  memcpy( dest->name, er9x::EmodelData.name, MODEL_NAME_LEN) ;
	dest->modelVoice = er9x::EmodelData.modelVoice ;
//	dest->RxNum = er9x::EmodelData.RxNum ;
	dest->traineron = er9x::EmodelData.traineron ;
	dest->FrSkyUsrProto = er9x::EmodelData.FrSkyUsrProto ;
	dest->FrSkyGpsAlt = er9x::EmodelData.FrSkyGpsAlt ;
	dest->FrSkyImperial = er9x::EmodelData.FrSkyImperial ;
	dest->FrSkyAltAlarm = er9x::EmodelData.FrSkyAltAlarm ;
  dest->modelVersion = er9x::EmodelData.modelVersion ;
	if ( dest->modelVersion > 3 )
	{
		dest->modelVersion = 3 ;
	}
	dest->protocol = er9x::EmodelData.protocol ;
	if ( dest->protocol > 2 )
	{
		dest->protocol -= 2 ;
	}
	dest->ppmNCH = er9x::EmodelData.ppmNCH ;
	dest->thrTrim = er9x::EmodelData.thrTrim ;
	dest->xnumBlades = er9x::EmodelData.numBlades ;
	dest->thrExpo = er9x::EmodelData.thrExpo ;
	dest->trimInc = er9x::EmodelData.trimInc ;
	dest->ppmDelay = er9x::EmodelData.ppmDelay ;
	dest->trimSw = er9x::EmodelData.trimSw ;
	dest->beepANACenter = er9x::EmodelData.beepANACenter ;
	dest->pulsePol = er9x::EmodelData.pulsePol ;
	dest->extendedLimits = er9x::EmodelData.extendedLimits ;
	dest->swashInvertELE = er9x::EmodelData.swashInvertELE ;
	dest->swashInvertAIL = er9x::EmodelData.swashInvertAIL ;
	dest->swashInvertCOL = er9x::EmodelData.swashInvertCOL ;
	dest->swashType = er9x::EmodelData.swashType ;
	dest->swashCollectiveSource = er9x::EmodelData.swashCollectiveSource ;
	dest->swashRingValue = er9x::EmodelData.swashRingValue ;
	dest->ppmFrameLength = er9x::EmodelData.ppmFrameLength ;
	dest->startChannel = er9x::EmodelData.ppmStart ;
  dest->sub_trim_limit = er9x::EmodelData.sub_trim_limit ;

	for ( i = 0 ; i < MAX_MIXERS ; i += 1 )
	{
		er9x::srcMix = &er9x::EmodelData.mixData[i] ;
		SKYMixData *dst = &dest->mixData[i] ;
    dst->destCh = er9x::srcMix->destCh ;
    dst->srcRaw = er9x::srcMix->srcRaw ;
		if ( dst->srcRaw == NUM_XCHNRAW+1 )		// MIX_3POS
		{
			dst->srcRaw += NUM_SKYCHNOUT - NUM_CHNOUT ;
		}

    dst->weight = er9x::srcMix->weight ;
    dst->swtch = er9x::srcMix->swtch ;
		convertEr9xSwitch( &dst->swtch ) ;
		
		dst->curve = er9x::srcMix->curve ;
		dst->delayUp = er9x::srcMix->delayUp * 10 ;
		dst->delayDown = er9x::srcMix->delayDown * 10 ;
		dst->speedUp = er9x::srcMix->speedUp * 10 ;
		dst->speedDown = er9x::srcMix->speedDown * 10 ;
		dst->carryTrim = er9x::srcMix->carryTrim ;
		dst->mltpx = er9x::srcMix->mltpx ;
		dst->mixWarn = er9x::srcMix->mixWarn ;
    dst->disableExpoDr = er9x::srcMix->disableExpoDr ;
		dst->sOffset = er9x::srcMix->sOffset ;
  	dst->lateOffset = er9x::srcMix->lateOffset ;
  	dst->modeControl = er9x::srcMix->modeControl ;
		dst->switchSource = er9x::srcMix->sw23pos ;
		dst->differential = er9x::srcMix->differential ;
	}
	for ( i = 0 ; i < NUM_CHNOUT ; i += 1 )
	{
		dest->limitData[i].min = er9x::EmodelData.limitData[i].min ;
		dest->limitData[i].max = er9x::EmodelData.limitData[i].max ;
		dest->limitData[i].revert = er9x::EmodelData.limitData[i].revert ;
		dest->limitData[i].offset = er9x::EmodelData.limitData[i].offset ;
	}
	for ( i = 0 ; i < 4 ; i += 1 )
	{
		
		dest->expoData[i] = (ExpoData&)er9x::EmodelData.expoData[i] ;
		convertEr9xSwitch( &dest->expoData[i].drSw1 ) ;
		convertEr9xSwitch( &dest->expoData[i].drSw2 ) ;
		dest->trim[i] = er9x::EmodelData.trim[i] ;
	}
  memcpy( dest->curves5, er9x::EmodelData.curves5, sizeof(er9x::EmodelData.curves5) ) ;
  memcpy( dest->curves9, er9x::EmodelData.curves9, sizeof(er9x::EmodelData.curves9) ) ;
	for ( i = 0 ; i < NUM_CSW ; i += 1 )
	{
    er9x::CSwData *src = &er9x::EmodelData.customSw[i] ;
		SKYCSwData *dst = &dest->customSw[i] ;
		dst->v1 = src->v1 ;
		dst->v2 = src->v2 ;
		dst->func = src->func ;
  	switch (dst->func)
		{
  		case (CS_AND) :
  		case (CS_OR) :
  		case (CS_XOR) :
				convertEr9xSwitch( &dst->v1 ) ;
				convertEr9xSwitch( &dst->v2 ) ;
      break;
  		case (CS_VPOS):
  		case (CS_VNEG):
  		case (CS_APOS):
  		case (CS_ANEG):
				if ( dst->v1-1 >= CHOUT_BASE+NUM_CHNOUT )
				{
					dst->v1 += NUM_SKYCHNOUT - NUM_CHNOUT ;
				}
				if ( dst->v2-1 >= CHOUT_BASE+NUM_CHNOUT )
				{
					dst->v2 += NUM_SKYCHNOUT - NUM_CHNOUT ;
				}
      break;
		}
		dst->andsw = src->andsw ;
	}
	dest->frSkyVoltThreshold = er9x::EmodelData.frSkyVoltThreshold ;
//	dest->bt_telemetry = er9x::EmodelData.bt_telemetry ;
	dest->numVoice = er9x::EmodelData.numVoice ;
	if ( dest->numVoice )
	{
		dest->numVoice += NUM_SKYCHNOUT - NUM_CHNOUT ;
	}
	for ( i = 0 ; i < NUM_CHNOUT ; i += 1 )
	{
    er9x::SafetySwData *src = &er9x::EmodelData.safetySw[i] ;
		SKYSafetySwData *dst = &dest->safetySw[i] ;
		if ( i < (uint32_t)NUM_CHNOUT - dest->numVoice )
		{
			dst->opt.ss.swtch = src->opt.ss.swtch ;
			dst->opt.ss.mode = src->opt.ss.mode ;
			dst->opt.ss.val = src->opt.ss.val ;
			if ( dst->opt.ss.mode == 3 )
			{
				dst->opt.ss.mode = 0 ;
			}
			switch ( dst->opt.ss.mode )
			{
				case 0 :
				case 1 :
				case 2 :
					convertEr9xSwitch( &dst->opt.ss.swtch ) ;
				break ;
			}
		}
		else
		{
			dst->opt.vs.vswtch = src->opt.vs.vswtch ;
			dst->opt.vs.vmode = src->opt.vs.vmode ;
			dst->opt.vs.vval = src->opt.vs.vval ;
			convertEr9xSwitch( (int8_t *)&dst->opt.vs.vswtch ) ;
		}
	}

	for ( i = 0 ; i < 2 ; i += 1 )
	{
    er9x::FrSkyChannelData *src = &er9x::EmodelData.frsky.channels[i] ;
		SKYFrSkyChannelData *dst = &dest->frsky.channels[i] ;
		dst->ratio = src->ratio ;
		dst->alarms_value[0] = src->alarms_value[0] ;
		dst->alarms_value[1] = src->alarms_value[1] ;
		dst->alarms_level = src->alarms_level ;
		dst->alarms_greater = src->alarms_greater ;
		dst->type = src->type ;
		dst->gain = 1 ;
	}

	for ( i = 0 ; i < 2 ; i += 1 )
	{
		int x ;
		if ( i == 0 )
		{
			dest->timer[i].tmrModeA = er9x::EmodelData.tmrMode ;
			dest->timer[i].tmrDir = er9x::EmodelData.tmrDir ;
			x = er9x::EmodelData.tmrModeB ;
			if ( x > 21 )
			{
				x += 13 ;
			}
			dest->timer[i].tmrModeB = x ;
			dest->timer[i].tmrVal = er9x::EmodelData.tmrVal ;
			dest->timer1RstSw  = er9x::EmodelData.timer1RstSw ;
			convertEr9xSwitch( (int8_t *)&dest->timer1RstSw ) ;
			dest->timer1Cdown  = er9x::EmodelData.timer1Cdown ;
			dest->timer1Mbeep  = er9x::EmodelData.timer1Mbeep ;
		}
		else
		{
			dest->timer[i].tmrModeA = er9x::EmodelData.tmr2Mode ;
			dest->timer[i].tmrDir = er9x::EmodelData.tmr2Dir ;
			x = er9x::EmodelData.tmr2ModeB ;
			if ( x > 21 )
			{
				x += 13 ;
			}
			dest->timer[i].tmrModeB = x ;
			dest->timer[i].tmrVal = er9x::EmodelData.tmr2Val ;
			dest->timer2RstSw  = er9x::EmodelData.timer2RstSw ;
			convertEr9xSwitch( (int8_t *)&dest->timer2RstSw ) ;
			dest->timer2Cdown  = er9x::EmodelData.timer2Cdown ;
			dest->timer2Mbeep  = er9x::EmodelData.timer2Mbeep ;
		}
		
		
  	if( dest->timer[i].tmrModeB>=(MAX_DRSWITCH))	 //momentary on-off
		{
			dest->timer[i].tmrModeB += (NUM_SKYCSW - NUM_CSW) ;
    }
	}

	for ( i = 0 ; i < MAX_GVARS ; i += 1 )
	{
		dest->gvars[i] = (GvarData&)er9x::EmodelData.gvars[i] ;
	}

	// Voice alarms
	for ( i = 0 ; i < NUM_VOICE_ALARMS ; i += 1 )
	{
		int x ;
		x = er9x::EmodelData.vad[i].source ;
		if ( x > 35 )
		{
			x += 8 ; //extra channels 
		}
		dest->vad[i].source = x ;
		dest->vad[i].func = er9x::EmodelData.vad[i].func ;
		dest->vad[i].swtch = er9x::EmodelData.vad[i].swtch ;
		convertEr9xSwitch( (int8_t *)&dest->vad[i].swtch ) ;
		dest->vad[i].rate = er9x::EmodelData.vad[i].rate ;
		dest->vad[i].fnameType = er9x::EmodelData.vad[i].fnameType ;
		dest->vad[i].haptic = er9x::EmodelData.vad[i].haptic ;
		dest->vad[i].vsource = er9x::EmodelData.vad[i].vsource ;
		dest->vad[i].mute = er9x::EmodelData.vad[i].mute ;
		dest->vad[i].offset = er9x::EmodelData.vad[i].offset ;
		dest->vad[i].file.vfile = er9x::EmodelData.vad[i].vfile ;
	}


//	dest->frskyAlarms = er9x::EmodelData.frskyAlarms ;

  memcpy( &dest->customDisplayIndex[0], &er9x::EmodelData.CustomDisplayIndex[0], 6 ) ;
}


void MdiChild::wizardEdit()
{
   EEGeneral gg;
   memcpy( &gg, &radioData.generalSettings, sizeof(EEGeneral) ) ;
	
//  int row = ui->modelsList->currentRow();
  int row = this->currentRow();
  if (row > 0)
	{
    modelDefault(row-1);
//    checkAndInitModel(row);
    WizardDialog * wizard = new WizardDialog( gg, row, this);
    wizard->exec();
    if (wizard->mix.complete /*TODO rather test the exec() result?*/)
		{
    	SKYModelData gm ;
//	    eeFile.getModel(&gm,currentRow()-1) ;
      gm = wizard->mix ;
			
    	memcpy( &radioData.models[row-1], &gm, sizeof(SKYModelData) ) ;
//      radioData.models[row - 1] = wizard->mix;
			setModelFile( row-1 ) ;
      setModified();
    }
  }
}













