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
#include <RoomControlClient.h>
#include <profile/serviceproviderprofile.h>
#include "kcategorizedsortfilterproxymodel.h"
#include <actors/abstractactor.h>
#include <conditions/abstractcondition.h>
#include <events/abstractevent.h>

ServiceProviderTreeModel::ServiceProviderTreeModel ( const QString& title, QObject* parent )
        : QAbstractItemModel ( parent ), m_title(title)
{
    connect ( RoomControlClient::getNetworkController(),SIGNAL ( disconnected() ),
              SLOT ( slotdisconnected() ) );
}

ServiceProviderTreeModel::~ServiceProviderTreeModel()
{
    slotdisconnected();
}

void ServiceProviderTreeModel::slotdisconnected()
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
                return m_title;
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
                AbstractServiceProvider* p = (AbstractServiceProvider*)index.internalPointer();
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
                AbstractActor* p = qobject_cast<AbstractActor*>((AbstractServiceProvider*)index.internalPointer());
                if (p) return p->delay();
            }
        }
    }


    return QVariant();
}

bool ServiceProviderTreeModel::removeRows ( int row, int count, const QModelIndex &parent )
{
    QString str;
    for ( int i=row+count-1;i>=row;--i )
    {
        AbstractServiceProvider* p = get(parent.child(row,0));
        if (!p) {
            qWarning()<<__PRETTY_FUNCTION__<<"Failed to remove"<<row<<parent.internalId();
        } else
            p->requestRemove();
    }
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
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

void ServiceProviderTreeModel::addedProvider ( AbstractServiceProvider* provider )
{
    // Already included?
    if (indexOf ( provider->id()).isValid()) return;

    connect ( provider, SIGNAL ( objectChanged ( AbstractServiceProvider* ) ),
              SLOT ( objectChanged ( AbstractServiceProvider* ) ) );

    if (qobject_cast<AbstractEvent*>(provider))
        addToList(m_events, provider);
    else if (qobject_cast<AbstractCondition*>(provider))
        addToList(m_conditions, provider);
    else if (qobject_cast<AbstractActor*>(provider))
        addToList(m_actors, provider);
}

void ServiceProviderTreeModel::removedProvider ( AbstractServiceProvider* provider )
{
    int pos = m_events.indexOf(provider);
    if (pos!=-1) {
        beginRemoveRows ( eventsindex(0), pos, pos );
        m_events.removeAt ( pos );
        endRemoveRows();
        return;
    }
    pos = m_conditions.indexOf(provider);
    if (pos!=-1) {
        beginRemoveRows ( conditionsindex(0), pos, pos );
        m_conditions.removeAt ( pos );
        endRemoveRows();
        return;
    }
    pos = m_actors.indexOf(provider);
    if (pos!=-1) {
        beginRemoveRows ( actorsindex(0), pos, pos );
        m_actors.removeAt ( pos );
        endRemoveRows();
        return;
    }
}

void ServiceProviderTreeModel::objectChanged ( AbstractServiceProvider* provider )
{
    removedProvider(provider);
    addedProvider(provider);
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
            if (qobject_cast<AbstractActor*>((AbstractServiceProvider*)index.internalPointer()))
                return Qt::ItemIsEnabled | Qt::ItemIsEditable;
        }

    return QAbstractItemModel::flags ( index );
}

bool ServiceProviderTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    AbstractActor* actor = qobject_cast<AbstractActor*>((AbstractServiceProvider*)index.internalPointer());
    int v = value.toInt();
    if (!actor || role!=Qt::EditRole || v<0 || v==actor->delay()) return false;
    actor->setDelay(v);
    actor->sync();
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
        if (qobject_cast<AbstractEvent*>((AbstractServiceProvider*)child.internalPointer()))
            return eventsindex(0);
        else if (qobject_cast<AbstractCondition*>((AbstractServiceProvider*)child.internalPointer()))
            return conditionsindex(0);
        else if (qobject_cast<AbstractActor*>((AbstractServiceProvider*)child.internalPointer()))
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

void ServiceProviderTreeModel::addToList(QList< AbstractServiceProvider* >& list, AbstractServiceProvider* provider) {
    int row;
    for (row=0;row<list.size();++row)
        if (list[row]->toString().toLower() >= provider->toString().toLower())
            break;

    if (qobject_cast<AbstractEvent*>(provider))
        beginInsertRows ( eventsindex(0),row,row);
    else if (qobject_cast<AbstractCondition*>(provider))
        beginInsertRows ( conditionsindex(0),row,row);
    else if (qobject_cast<AbstractActor*>(provider))
        beginInsertRows ( actorsindex(0),row,row);
    list.insert (row, provider );
    endInsertRows();

    QModelIndex index = indexOf(provider->id());
    if (!index.isValid()) {
        qWarning()<<__PRETTY_FUNCTION__<<"Failed to add at"<<row;
        /*    } else {
        		qDebug() << __FUNCTION__<<"Added at"<<index.row()<<((AbstractServiceProvider*)index.internalPointer())->type()<<this;*/
    }
}
