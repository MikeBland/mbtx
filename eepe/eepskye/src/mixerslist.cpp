#include "mixerslist.h"
//Q_DECLARE_METATYPE(QModelIndex); //at start of BookMarkList.cpp file (after #includes)

MixersList::MixersList(QWidget *parent) :
    QListWidget(parent)
{
    setFont(QFont("Courier New",12));
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
}

void MixersList::keyPressEvent(QKeyEvent *event)
{
    emit keyWasPressed(event);
}

/**
    @brief Override to give us different mime type for mixers and inputs
*/
QStringList MixersList::mimeTypes () const
{
    QStringList types;
    types << "application/x-eepskye-mix"; 
    return types;
}

bool MixersList::dropMimeData( int index, const QMimeData * data, Qt::DropAction action )
{    
    QByteArray dropData = data->data("application/x-eepskye-mix");
    QDataStream stream(&dropData, QIODevice::ReadOnly);
    QByteArray qba;

    while (!stream.atEnd())
    {
        int r,c;
        QMap<int, QVariant> v;
        stream >> r >> c >> v;
        QList<QVariant> lsVars;
        lsVars = v.values();
        QString itemString = lsVars.at(0).toString();
        qba.append(lsVars.at(2).toByteArray().mid(1));

        if(itemString.isEmpty()) {};
    }

    if(qba.length()>0)
    {
        QMimeData *mimeData = new QMimeData;
        mimeData->setData("application/x-eepskye-mix", qba);

        emit mimeDropped(index, mimeData, action);
    }


    return true;
}


