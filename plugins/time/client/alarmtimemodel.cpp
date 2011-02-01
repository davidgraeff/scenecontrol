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
#include "alarmtimemodel.h"
#include <QStringList>
#include <QDebug>
#include <shared/categorize/profile.h>
#include <services/eventperiodic.h>
#include <services/eventdatetime.h>
#include <shared/client/servicestorage.h>

AlarmTimeModel::AlarmTimeModel ( QObject* parent )
        : ClientModel ( parent )
{
    clear();
}

AlarmTimeModel::~AlarmTimeModel()
{
    m_items.clear();
}

void AlarmTimeModel::clear()
{
    m_items.clear();
    reset();
}

int AlarmTimeModel::rowCount ( const QModelIndex & ) const
{
    return m_items.size();
}

int AlarmTimeModel::columnCount(const QModelIndex& ) const {
    return 2;
}

Qt::ItemFlags AlarmTimeModel::flags(const QModelIndex& index) const {
    if (index.column()==1)
        return QAbstractItemModel::flags ( index ) | Qt::ItemIsUserCheckable;
    else
        return QAbstractItemModel::flags ( index );
}

QVariant AlarmTimeModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0:
            return tr("Profile");
        case 1:
            return tr("Time");
        default:
            break;
        }
    }

    return QAbstractItemModel::headerData ( section, orientation, role );
}

QVariant AlarmTimeModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();
    const AlarmItem item = m_items[index.row()];
    if ( role == Qt::UserRole ) return item.item->id();
    else if (role==Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return item.parent->name();
        case 1:
            return item.item->toString();
        default:
            break;
        }
    } else if (role==Qt::CheckStateRole && index.column()==0) {
        return (item.parent->enabled()?Qt::Checked:Qt::Unchecked);
    }
    return QVariant();
}

bool AlarmTimeModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() || index.column() !=0 ) return false;
    const AlarmItem item = m_items[index.row()];

    if ( role == Qt::CheckStateRole) {
        item.parent->setEnabled(value.toInt()==Qt::Checked);
        emit changeService(item.parent);
        QModelIndex index = createIndex ( index.row(),0,0 );
        emit dataChanged ( index,index );
        return true;
    }
    return false;
}

bool AlarmTimeModel::removeRows ( int row, int count, const QModelIndex & )
{
    for ( int i=row+count-1;i>=row;--i )
    {
        AbstractServiceProvider* service = m_items[i].item;
        service->setProperty("remove",true);
        emit changeService(service);
    }
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
    return true;
}

void AlarmTimeModel::serviceRemoved ( AbstractServiceProvider* provider )
{
    int index = indexOf ( provider->id() );
    if ( index == -1 ) return;

    beginRemoveRows ( QModelIndex(), index, index );
    m_items.removeAt(index);
    endRemoveRows();
}

void AlarmTimeModel::serviceChanged ( AbstractServiceProvider* service )
{
    // Filter
    Q_ASSERT(service);
    if (service->metaObject()->className() != EventPeriodic::staticMetaObject.className() &&
            service->metaObject()->className() != EventDateTime::staticMetaObject.className()) return;


    // Find out parent name
    ServiceStorage* servicestorage = ServiceStorage::instance();
    Q_ASSERT(servicestorage);
    AbstractServiceProvider* parentservice = servicestorage->services().value(service->parentid());
    Collection* collection = dynamic_cast<Collection*>(parentservice);
    if (!collection) return;

    // Search for provider
    int index = indexOf ( service->id() );

    if ( index == -1 ) {

        // Find alphabetic position amoung profiles in the same category
        int row=0;
        for (;row<m_items.size();++row)
            if (m_items[row].item->toString().toLower() >= service->toString().toLower())
                break;

        beginInsertRows (QModelIndex(),row,row );
        m_items.insert(row, AlarmItem(service, collection));
        endInsertRows();

        if (m_focusid == service->id()) {
            emit itemMoved(createIndex(row,0,service));
            m_focusid = QString();
        }

        return;
    }

    bool indicateMovement = false;
    int row=-1;
    bool skip = false;

    for (row=0;row<m_items.size();++row)
    {
        if (index == row) skip = true;
        if (index != row && m_items[row].item->toString().toLower() >= service->toString())
        {
            indicateMovement = true;
            break;
        }
    }
    if (skip) row--;

    if (row != -1) {
        beginRemoveRows ( QModelIndex(), index, index );
        m_items.removeAt(index);
        endRemoveRows();
        beginInsertRows ( QModelIndex(),row,row );
        m_items.insert(row, AlarmItem(service, collection));
        endInsertRows();
        if (indicateMovement) emit itemMoved(createIndex(row, 0, service));
    } else {
        QModelIndex mindex = createIndex(index,0);
        emit dataChanged ( mindex,mindex );
    }
}

int AlarmTimeModel::indexOf(const QVariant& data) {
    if (data.type()!=QVariant::String) return -1;
    const QString id = data.toString();
    for (int row=0;row<m_items.size();++row)
    {
        if (m_items[row].item->id() == id) {
            return row;
        }
    }
    return -1;
}

void AlarmTimeModel::stateTrackerChanged(AbstractStateTracker*) {}

