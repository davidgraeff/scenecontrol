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

#include "pinsmodel.h"
#include <QStringList>
#include <QDebug>
#include <actors/actorpin.h>
#include <RoomControlClient.h>
#include <actors/actorpinname.h>
#include <stateTracker/pinvaluestatetracker.h>
#include <stateTracker/pinnamestatetracker.h>


PinsModel::PinsModel (QObject* parent) : QAbstractListModel ( parent )
{
    this->iconOn = QIcon ( ":/green" );
    this->iconOff = QIcon ( ":/red" );
    connect(RoomControlClient::getFactory(),SIGNAL(stateChanged(AbstractStateTracker*)),
            SLOT(stateChanged(AbstractStateTracker*)));
    connect(RoomControlClient::getNetworkController(),SIGNAL(disconnected()),
            SLOT(slotdisconnected()));
}

PinsModel::~PinsModel()
{
    slotdisconnected();
}

void PinsModel::slotdisconnected()
{
    qDeleteAll(m_itemvalues);
    qDeleteAll(m_itemnames);
    m_itemvalues.clear();
    m_itemnames.clear();
    m_assignment.clear();
    reset();
}

void PinsModel::stateChanged(AbstractStateTracker* tracker)
{
    if (tracker->metaObject()->className() ==
            PinValueStateTracker::staticMetaObject.className())
    {
        PinValueStateTracker* p = (PinValueStateTracker*)tracker;
        p->setProperty("managed",true);
        if (!m_assignment.contains(p->pin())) {
            beginInsertRows(QModelIndex(), p->pin(), p->pin());
            m_assignment.insert(p->pin(), m_itemvalues.size());
            m_itemvalues.append(p);
            m_itemnames.append(new PinNameStateTracker());
            endInsertRows();
        } else {
            const int listpos = m_assignment.value(p->pin());
            delete m_itemvalues[listpos];
            m_itemvalues[listpos] = p;
            QModelIndex index = createIndex(listpos,0);
            QModelIndex index2 = createIndex(listpos,1);
            emit dataChanged(index, index2);
        }
    } else if (tracker->metaObject()->className() ==
               PinNameStateTracker::staticMetaObject.className())
    {
        PinNameStateTracker* p = (PinNameStateTracker*)tracker;
        p->setProperty("managed",true);
        if (!m_assignment.contains(p->pin())) {
        } else {
            const int listpos = m_assignment.value(p->pin());
            delete m_itemnames[listpos];
            m_itemnames[listpos] = p;
            QModelIndex index = createIndex(listpos,0);
            emit dataChanged(index, index);
        }
    }
}

int PinsModel::rowCount ( const QModelIndex & ) const
{
    return m_itemvalues.size();
}

int PinsModel::columnCount ( const QModelIndex & ) const
{
    return 2;
}

QVariant PinsModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();

    if ( role==Qt::DisplayRole || role==Qt::EditRole )
    {
        if ( index.column() ==0 )
            if (m_itemnames.size() <= index.row())
                return QLatin1String("not loaded");
            else
                return m_itemnames.at(index.row())->value();
        else if ( index.column() ==1 ) {
            return (m_itemvalues.at(index.row())->value()?
                    QLatin1String("An"):QLatin1String("Aus"));
        }
    }
    else if ( role==Qt::DecorationRole  && index.column() == 0)
    {
        if (m_itemvalues.at(index.row())->value() )
            return this->iconOn;
        else
            return this->iconOff;
    }

    return QVariant();
}


QVariant PinsModel::headerData ( int section, Qt::Orientation orientation, int role ) const
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

bool PinsModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid()) return false;

    if ( index.column() ==0 && role == Qt::EditRole )
    {
        QString newname = value.toString().trimmed().replace ( '\n',"" ).replace ( '\t',"" );
        PinNameStateTracker* t= m_itemnames[index.row() ];
        if ( newname.isEmpty() || newname == t->value() ) return false;

        setName(t->pin(),newname);

        return true;
    }

    return false;
}

Qt::ItemFlags PinsModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    if (index.column()==1)
        return QAbstractListModel::flags ( index );

    return QAbstractListModel::flags ( index ) | Qt::ItemIsEditable;
}

void PinsModel::setValue ( int i, unsigned int value )
{
    ActorPin* a = new ActorPin();
    a->setPin(i);
    a->setValue(value);
    RoomControlClient::getFactory()->executeActor(a);
}

void PinsModel::setName ( int i, const QString& value )
{
    ActorPinName* a = new ActorPinName();
    a->setPin(i);
    a->setPinname(value);
    RoomControlClient::getFactory()->executeActor(a);
}

QString PinsModel::getName ( int i )
{
    if (i<0 || i>=m_itemnames.size()) return QString();
    return m_itemnames.at ( i )->value();
}

int PinsModel::getValue ( int i ) const
{
    if (i<0 || i>=m_itemvalues.size()) return 0;
    return m_itemvalues.at ( i )->value();
}

void PinsModel::toggle ( int i )
{
    if (i<0 || i>=m_itemvalues.size()) return;
    setValue(i, ( m_itemvalues.at ( i )->value() ?0:1 ));
}
