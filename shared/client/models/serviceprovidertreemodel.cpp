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

#include "serviceprovidertreemodel.h"
#include <QStringList>
#include <QDebug>
#include "kcategorizedsortfilterproxymodel.h"
#include <shared/abstractserviceprovider.h>
#include <servicestorage.h>

ServiceProviderTreeModel::ServiceProviderTreeModel ( QObject* parent )
        : ClientModel ( parent )
{

}

ServiceProviderTreeModel::~ServiceProviderTreeModel()
{
    clear();
}

void ServiceProviderTreeModel::clear()
{
    // Do not delete playlist objects, they are managed through the factory
    m_events.clear();
    m_conditions.clear();
    m_actors.clear();
    reset();
}

QVariant ServiceProviderTreeModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( role == Qt::DisplayRole )
        {
            if ( section==0 )
                return tr("Services");
            else
                return tr("Delay");
        }
    }

    return QVariant();
}

QVariant ServiceProviderTreeModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();

    if ( role==Qt::DisplayRole || role==Qt::EditRole)
    {
        if (index.column()==0) {
            switch (index.internalId())
            {
            case 'e':
                return tr("Ereignisse");
            case 'c':
                return tr("Bedingungen");
            case 'a':
                return tr("Aktionen");
            default:
                AbstractServiceProvider* p = static_cast<AbstractServiceProvider*>(index.internalPointer());
                Q_ASSERT(p);
                return p->toString();
            }
        } else if (index.column()==1) {
            switch (index.internalId())
            {
            case 'e':
            case 'c':
            case 'a':
                return QVariant();
            default:
                AbstractServiceProvider* p = static_cast<AbstractServiceProvider*>(index.internalPointer());
                Q_ASSERT(p);
                return p->delay();
            }
        }
    }


    return QVariant();
}

bool ServiceProviderTreeModel::removeRows ( int row, int count, const QModelIndex &parent )
{
    if (!parent.isValid()) return false;
    for ( int i=row+count-1;i>=row;--i )
        ServiceStorage::instance()->deleteService(get(parent.child(row,0)));
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
    return true;
}

bool ServiceProviderTreeModel::removeRows ( QModelIndexList list )
{
    qSort(list.begin(), list.end());

    for (int i = list.count() - 1; i > -1; --i) {
        ServiceStorage::instance()->deleteService(get(list[i]));
    }

    emit dataChanged ( list.first(),list.last() );
    return true;
}

QModelIndex ServiceProviderTreeModel::indexOf(const QString& id) {
    for (int i=0;i<m_events.size();++i)
        if (m_events[i]->id()==id) {
            return createIndex(i,0,m_events[i]);
        }
    for (int i=0;i<m_conditions.size();++i)
        if (m_conditions[i]->id()==id) {
            return createIndex(i,0,m_conditions[i]);
        }
    for (int i=0;i<m_actors.size();++i)
        if (m_actors[i]->id()==id) {
            return createIndex(i,0,m_actors[i]);
        }
    return QModelIndex();
}

void ServiceProviderTreeModel::serviceChanged ( AbstractServiceProvider* provider )
{
    // filter out wrong parentid services
    Q_ASSERT(provider);
    if ((provider->service() ==AbstractServiceProvider::NoneService) ||
            (m_parentid.size() && provider->parentid()!=m_parentid)) return;

    // Already included?
    QModelIndex index = indexOf ( provider->id());
    if (index.isValid()) {
        emit dataChanged(index, index);
        return;
    }
    addToList(provider);

}

void ServiceProviderTreeModel::serviceRemoved ( AbstractServiceProvider* provider )
{
    // filter out wrong parentid services
    Q_ASSERT(provider);
    if ((provider->service() == AbstractServiceProvider::NoneService) ||
            (m_parentid.size() && provider->parentid()!=m_parentid)) return;

    int pos = m_events.indexOf(provider);
    if (pos!=-1) {
        beginRemoveRows ( eventsindex(0), pos, pos );
        m_events.removeAt ( pos );
        endRemoveRows();
    }
    pos = m_conditions.indexOf(provider);
    if (pos!=-1) {
        beginRemoveRows ( conditionsindex(0), pos, pos );
        m_conditions.removeAt ( pos );
        endRemoveRows();
    }
    pos = m_actors.indexOf(provider);
    if (pos!=-1) {
        beginRemoveRows ( actorsindex(0), pos, pos );
        m_actors.removeAt ( pos );
        endRemoveRows();
    }
}

Qt::ItemFlags ServiceProviderTreeModel::flags(const QModelIndex& index) const {
    if ( index.isValid() && index.column()==1)
        switch (index.internalId())
        {
        case 'e':
        case 'c':
        case 'a':
            return Qt::ItemIsEnabled;
        default:
            return Qt::ItemIsEnabled | Qt::ItemIsEditable;
        }

    return ClientModel::flags ( index );
}

bool ServiceProviderTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    AbstractServiceProvider* service = static_cast<AbstractServiceProvider*>(index.internalPointer());
    int v = value.toInt();
    if (!service || role!=Qt::EditRole || v<0 || v==service->delay()) return false;
    service->setDelay(v);
    ServiceStorage::instance()->serviceHasChanged(service);
    service->setDelay(-1);
    return true;
}

int ServiceProviderTreeModel::rowCount ( const QModelIndex & parent) const
{
    if (!parent.isValid()) return 3;
    // root does know about 3 grouping items
    switch (parent.internalId())
    {
    case 'e':
        return m_events.size();
    case 'c':
        return m_conditions.size();
    case 'a':
        return m_actors.size();
    default:
        return 0;
    }
}

int ServiceProviderTreeModel::columnCount(const QModelIndex& parent) const {
    switch (parent.internalId())
    {
    case 'e':
        return 1;
    case 'c':
        return 1;
    case 'a':
        return 2;
    default:
        return 2;
    }
}


bool ServiceProviderTreeModel::hasChildren(const QModelIndex& parent) const {
    if (!parent.isValid()) return 3;
    // root does know about 3 grouping items
    switch (parent.internalId())
    {
    case 'e':
        return m_events.size();
    case 'c':
        return m_conditions.size();
    case 'a':
        return m_actors.size();
    default:
        return 0;
    }
}

QModelIndex ServiceProviderTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) return QModelIndex();
    // root does know about 3 grouping items
    switch (child.internalId())
    {
    case 'e':
    case 'c':
    case 'a':
        return QModelIndex();
    default:
    {
        AbstractServiceProvider* provider = static_cast<AbstractServiceProvider*>(child.internalPointer());
        Q_ASSERT(provider);
        if (provider->service()==AbstractServiceProvider::EventService)
            return eventsindex(0);
        else if (provider->service()==AbstractServiceProvider::ConditionService)
            return conditionsindex(0);
        else if (provider->service()==AbstractServiceProvider::ActionService)
            return actorsindex(0);
    }
    }
    qWarning()<<__PRETTY_FUNCTION__<<"No parent found!";
    return QModelIndex();
}

QModelIndex ServiceProviderTreeModel::index(int row, int column, const QModelIndex& parent) const {
    // secret root
    if (!parent.isValid()) {
        switch (row) {
        case eventsrow:
            return eventsindex(column);
        case conditionsrow:
            return conditionsindex(column);
        case actorsrow:
            return actorsindex(column);
        default:
            qWarning()<<__PRETTY_FUNCTION__<<"Root should know only 3 childs:"<<row;
            return QModelIndex();
        }
    }

    switch (parent.internalId())
    {
    case 'e':
        return createIndex(row, column, m_events[row]);
    case 'c':
        return createIndex(row, column, m_conditions[row]);
    case 'a':
        return createIndex(row, column, m_actors[row]);
    default:
        qWarning()<<__PRETTY_FUNCTION__<<"Their should be only "<<row;
        return QModelIndex();
    }
}

AbstractServiceProvider* ServiceProviderTreeModel::get(const QModelIndex& index) {
    // ignore invalid
    if (!index.isValid()) return 0;
    // grouping items are not to be considered by this method
    switch (index.internalId())
    {
    case 'e':
    case 'c':
    case 'a':
        return 0;
    default:
        return (AbstractServiceProvider*)index.internalPointer();
    }
}

void ServiceProviderTreeModel::addToList(AbstractServiceProvider* provider) {
    int row=0;
    if (provider->service()==AbstractServiceProvider::EventService) {
        for (;row<m_events.size();++row)
            if (m_events[row]->toString().toLower() >= provider->toString().toLower())
                break;
        beginInsertRows ( eventsindex(0),row,row);
        m_events.insert (row, provider );
        endInsertRows();
    }
    else if (provider->service()==AbstractServiceProvider::ConditionService)
    {
        for (;row<m_conditions.size();++row)
            if (m_conditions[row]->toString().toLower() >= provider->toString().toLower())
                break;
        beginInsertRows ( conditionsindex(0),row,row);
        m_conditions.insert (row, provider );
        endInsertRows();
    }
    else if (provider->service()==AbstractServiceProvider::ActionService)
    {
        for (;row<m_actors.size();++row)
            if (m_actors[row]->toString().toLower() >= provider->toString().toLower())
                break;
        beginInsertRows ( actorsindex(0),row,row);
        m_actors.insert (row, provider );
        endInsertRows();
    } else
        qWarning()<<__PRETTY_FUNCTION__<<"Failed to get provider servicetype"<<provider->service();

    QModelIndex index = indexOf(provider->id());
    if (!index.isValid()) {
        qWarning()<<__PRETTY_FUNCTION__<<"Failed to add at"<<row;
        /*    } else {
        		qDebug() << __FUNCTION__<<"Added at"<<index.row()<<((AbstractServiceProvider*)index.internalPointer())->type()<<this;*/
    }
}
void ServiceProviderTreeModel::stateTrackerChanged(AbstractStateTracker*) {}
