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
#include <shared/abstractstatetracker.h>
#include <statetracker/pinvaluestatetracker.h>
#include <statetracker/pinnamestatetracker.h>
#include <services/actorpin.h>
#include <services/actorpinname.h>
#include "kicon.h"
#include <shared/client/servicestorage.h>

PinsModel::PinsModel (QObject* parent) : ClientModel ( parent )
{
    this->iconOn = KIcon ( QLatin1String("flag-green") );
    this->iconOff = KIcon ( QLatin1String("flag-red") );
}

PinsModel::~PinsModel()
{
    clear();
}

void PinsModel::clear()
{
    qDeleteAll(m_itemvalues);
    qDeleteAll(m_itemnames);
    m_itemvalues.clear();
    m_itemnames.clear();
    m_assignment.clear();
    reset();
}

void PinsModel::stateTrackerChanged(AbstractStateTracker* tracker)
{
    if (tracker->metaObject()->className() ==
            PinValueStateTracker::staticMetaObject.className())
    {
        PinValueStateTracker* original = (PinValueStateTracker*)tracker;
        PinValueStateTracker* p = new PinValueStateTracker();
        p->setPin(original->pin());
        p->setValue(original->value());
        if (!m_assignment.contains(p->pin())) {
            beginInsertRows(QModelIndex(),m_itemvalues.size() ,m_itemvalues.size());
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
        PinNameStateTracker* original = (PinNameStateTracker*)tracker;
        PinNameStateTracker* p = new PinNameStateTracker();
        p->setPin(original->pin());
        p->setValue(original->value());
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
    return 1;
}

QVariant PinsModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();

    if ( role==Qt::UserRole) return m_itemnames.at(index.row())->pin();
    if ( role==Qt::DisplayRole || role==Qt::EditRole )
    {
        if (m_itemnames.size() <= index.row())
            return QLatin1String("not loaded");
        else
            return m_itemnames.at(index.row())->value();
    }
    else if ( role==Qt::DecorationRole )
    {
        if (m_itemvalues.at(index.row())->value() )
            return this->iconOn;
        else
            return this->iconOff;
    }
    else if ( role==Qt::CheckStateRole )
    {
        return (m_itemvalues.at(index.row())->value()?Qt::Checked:Qt::Unchecked);
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
    }

    return QAbstractListModel::headerData ( section, orientation, role );
}

bool PinsModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid()) return false;

    if ( role == Qt::EditRole )
    {
        QString newname = value.toString().trimmed().replace ( QLatin1Char('\n'),QString() ).replace ( QLatin1Char('\t'),QString() );
        PinNameStateTracker* t= m_itemnames[index.row() ];
        if ( newname.isEmpty() || newname == t->value() ) return false;

        setName(t->pin(),newname);

        return true;
    } else if ( role == Qt::CheckStateRole )
    {
        PinNameStateTracker* t= m_itemnames[index.row() ];
        setValue(t->pin(), (value.toInt()==Qt::Checked));

        return true;
    }

    return false;
}

Qt::ItemFlags PinsModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    return QAbstractListModel::flags ( index ) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

void PinsModel::setValue ( const QString& pin, unsigned int value )
{
    ActorPin* a = new ActorPin();
    a->setPin(pin);
    a->setValue((ActorPin::ActorPinEnum)value);
    ServiceStorage::instance()->executeService(a);
}

void PinsModel::setName ( const QString& pin, const QString& value )
{
    ActorPinName* a = new ActorPinName();
    a->setPin(pin);
    a->setPinname(value);
    ServiceStorage::instance()->executeService(a);
}

/*
QString PinsModel::getName ( const QString& pin )
{
    if (i<0 || i>=m_itemnames.size()) return QString();
    return m_itemnames.at ( i )->value();
}

int PinsModel::getValue ( const QString& pin ) const
{
    if (i<0 || i>=m_itemvalues.size()) return 0;
    return m_itemvalues.at ( i )->value();
}

void PinsModel::toggle ( const QString& pin )
{
    if (i<0 || i>=m_itemvalues.size()) return;
    setValue(i, ( m_itemvalues.at ( i )->value() ?0:1 ));
}
*/

void PinsModel::serviceRemoved(AbstractServiceProvider*) {}
void PinsModel::serviceChanged(AbstractServiceProvider*) {}

int PinsModel::indexOf(const QVariant& data) {
    if (data.type()!=QVariant::String) return -1;
    return m_assignment.value(data.toString(), -1);
}
