/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "channelsmodel.h"
#include <QStringList>
#include <QDebug>
#include <actors/actorled.h>
#include <RoomControlClient.h>
#include <actors/actorledname.h>
#include <stateTracker/channelvaluestatetracker.h>
#include <stateTracker/channelnamestatetracker.h>

ChannelsModel::ChannelsModel (QObject* parent) : QAbstractListModel ( parent )
{
    connect(RoomControlClient::getFactory(),SIGNAL(stateChanged(AbstractStateTracker*)),
            SLOT(stateChanged(AbstractStateTracker*)));
    connect(RoomControlClient::getNetworkController(),SIGNAL(disconnected()),
            SLOT(slotdisconnected()));
}

ChannelsModel::~ChannelsModel()
{
    slotdisconnected();
}

void ChannelsModel::slotdisconnected()
{
    qDeleteAll(m_itemvalues);
    qDeleteAll(m_itemnames);
    m_itemvalues.clear();
    m_itemnames.clear();
    m_assignment.clear();
    reset();
}

void ChannelsModel::stateChanged(AbstractStateTracker* tracker)
{
    if (tracker->metaObject()->className() ==
            ChannelValueStateTracker::staticMetaObject.className())
    {
        ChannelValueStateTracker* p = (ChannelValueStateTracker*)tracker;
        p->setManaged();
        if (!m_assignment.contains(p->channel())) {
            beginInsertRows(QModelIndex(), p->channel(), p->channel());
            m_assignment.insert(p->channel(), m_itemvalues.size());
            m_itemvalues.append(p);
            m_itemnames.append(new ChannelNameStateTracker());
            endInsertRows();
        } else {
            const int listpos = m_assignment.value(p->channel());
            delete m_itemvalues[listpos];
            m_itemvalues[listpos] = p;
            QModelIndex index = createIndex(listpos,0);
            QModelIndex index2 = createIndex(listpos,1);
            emit dataChanged(index, index2);
        }
    } else if (tracker->metaObject()->className() ==
               ChannelNameStateTracker::staticMetaObject.className())
    {
        ChannelNameStateTracker* p = (ChannelNameStateTracker*)tracker;
        p->setManaged();
        if (!m_assignment.contains(p->channel())) {
        } else {
            const int listpos = m_assignment.value(p->channel());
            delete m_itemnames[listpos];
            m_itemnames[listpos] = p;
            QModelIndex index = createIndex(listpos,0);
            emit dataChanged(index, index);
        }
    }
}

int ChannelsModel::rowCount ( const QModelIndex & ) const
{
    return m_itemvalues.size();
}

int ChannelsModel::columnCount ( const QModelIndex & ) const
{
    return 2;
}

QVariant ChannelsModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();


    if ( role==Qt::DisplayRole || role==Qt::EditRole )
    {
        if ( index.column() ==0 )
            if (m_itemnames.size() < index.row())
                return QLatin1String("not loaded");
            else
                return m_itemnames.at(index.row())->value();
        else if ( index.column() ==1 ) {
            return m_itemvalues.at(index.row())->value();
        }
    }

    return QVariant();
}


QVariant ChannelsModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( role == Qt::DisplayRole && section==0 )
        {
            return tr ( "Name" );
        }
        else  if ( role == Qt::DisplayRole && section==1 )
        {
            return tr ( "Zustand" );
        }
    }

    return QAbstractListModel::headerData ( section, orientation, role );
}

bool ChannelsModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid()) return false;

    if ( index.column() ==0 && role == Qt::EditRole )
    {
        QString newname = value.toString().trimmed().replace ( '\n',"" ).replace ( '\t',"" );
        ChannelNameStateTracker* t= m_itemnames[index.row() ];
        if ( newname.isEmpty() || newname == t->value() ) return false;

        setName(t->channel(),newname);

        return true;
    }
    else if (index.column() ==1 && role == Qt::EditRole )
    {
        uint v = value.toUInt();
        ChannelValueStateTracker* t= m_itemvalues[index.row() ];
        if ( v == t->value()) return false;

        setValue(t->channel(),v);
        return true;
    }

    return false;
}

Qt::ItemFlags ChannelsModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    return QAbstractListModel::flags ( index ) | Qt::ItemIsEditable;
}

void ChannelsModel::setValue ( int i, unsigned int value )
{
    ActorLed* a = new ActorLed();
    a->setChannel(i);
    a->setValue(value);
    RoomControlClient::getFactory()->executeActor(a);
}

void ChannelsModel::setName ( int i, const QString& value )
{
    ActorLedName* a = new ActorLedName();
    a->setChannel(i);
    a->setLedname(value);
    RoomControlClient::getFactory()->executeActor(a);
}

QString ChannelsModel::getName ( int i )
{
    if (i<0 || i>=m_itemnames.size()) return QString();
    return m_itemnames.at ( i )->value();
}

int ChannelsModel::getValue ( int i ) const
{
    if (i<0 || i>=m_itemvalues.size()) return 0;
    return m_itemvalues.at ( i )->value();
}
