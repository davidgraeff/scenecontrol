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

#include "serviceprovidermodel.h"
#include <QStringList>
#include <QDebug>
#include <RoomControlClient.h>
#include <profile/serviceproviderprofile.h>
#include "kcategorizedsortfilterproxymodel.h"
#include <actors/abstractactor.h>
#include <conditions/abstractcondition.h>
#include <events/abstractevent.h>

ServiceProviderModel::ServiceProviderModel ( const QString& title, QObject* parent )
        : QAbstractListModel ( parent ), m_title(title)
{
    connect ( RoomControlClient::getNetworkController(),SIGNAL ( disconnected() ),
              SLOT ( slotdisconnected() ) );
}

ServiceProviderModel::~ServiceProviderModel()
{
    slotdisconnected();
}

void ServiceProviderModel::slotdisconnected()
{
    m_items.clear();
    reset();
}

int ServiceProviderModel::rowCount ( const QModelIndex & ) const
{
    return m_items.size();
}

QVariant ServiceProviderModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( role == Qt::DisplayRole && section==0 )
        {
            return m_title;
        }
    }

    return QAbstractListModel::headerData ( section, orientation, role );
}

QVariant ServiceProviderModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();
    AbstractServiceProvider* p = m_items.at ( index.row());

    if ( role==Qt::DisplayRole )
    {
        return p->toString();
    }
    else if ( role==Qt::UserRole )
    {
        return p->metaObject()->className();
    }
    else if ( role==Qt::ToolTipRole )
    {
        Collection* pc = qobject_cast<Collection*>(RoomControlClient::getFactory()->get(p->parentid()));
        QString parentname;
        if (pc) parentname = pc->name();
        return tr("%1\nZugeteilt: %2 (%3)").arg(p->id())
               .arg(p->parentid()).arg(parentname);
    }
    else if ( role== KCategorizedSortFilterProxyModel::CategorySortRole )
    {
        if (qobject_cast<AbstractActor*>(p)) return 3;
        else if (qobject_cast<AbstractCondition*>(p)) return 2;
        else if (qobject_cast<AbstractEvent*>(p)) return 1;
        else return 0;
    }
    else if ( role== KCategorizedSortFilterProxyModel::CategoryDisplayRole )
    {
        if (qobject_cast<AbstractActor*>(p)) return tr("Aktionen");
        else if (qobject_cast<AbstractCondition*>(p)) return tr("Bedingungen");
        else if (qobject_cast<AbstractEvent*>(p)) return tr("Ereignisse");
    }

    return QVariant();
}

bool ServiceProviderModel::removeRows ( int row, int count, const QModelIndex & )
{
    QString str;
    for ( int i=row+count-1;i>=row;--i )
    {
		m_items[i]->requestRemove();
    }
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
    return true;
}

void ServiceProviderModel::addedProvider ( AbstractServiceProvider* provider )
{

    const int listpos = indexOf ( provider->id());
    if ( listpos!=-1 ) return;

    connect ( provider, SIGNAL ( objectChanged ( AbstractServiceProvider* ) ),
              SLOT ( objectChanged ( AbstractServiceProvider* ) ) );

    Collection* pc = qobject_cast<Collection*>(provider);
    if (pc)
        connect(pc,SIGNAL(childsChanged(Collection*)),SLOT(childsChanged(Collection*)));

    int row;
    for (row=0;row<m_items.size();++row)
        if (m_items[row]->toString().toLower() >= provider->toString().toLower())
            break;

    beginInsertRows ( QModelIndex(),row,row );
    m_items.insert (row, provider );
    endInsertRows();
}

void ServiceProviderModel::removedProvider ( AbstractServiceProvider* provider )
{
    const int listpos = indexOf ( provider->id() );
    if ( listpos==-1 ) return;
    beginRemoveRows ( QModelIndex(), listpos, listpos );
    m_items.removeAt ( listpos );
    endRemoveRows();
}

void ServiceProviderModel::objectChanged ( AbstractServiceProvider* provider )
{
    const int listpos = indexOf ( provider->id());
    if ( listpos==-1 ) return;
    int row;
    bool skip = false;
    for (row=0;row<m_items.size();++row)
    {
        if (listpos==row) skip =true;
        if (listpos!=row && m_items[row]->toString().toLower() >= provider->toString().toLower())
        {
            break;
        }
    }
    if (skip) row--;

    if (row!=listpos) {
        beginRemoveRows ( QModelIndex(), listpos, listpos );
        m_items.removeAt ( listpos );
        endRemoveRows();
        beginInsertRows ( QModelIndex(),row,row );
        m_items.insert (row, provider );
        endInsertRows();
    } else {
        QModelIndex index = createIndex ( listpos,0,0 );
        emit dataChanged ( index,index );
    }
}

QString ServiceProviderModel::getName ( int i ) const
{
    return m_items.at ( i )->toString();
}

int ServiceProviderModel::indexOf ( const QString& id )
{
    for (int i=0;i<m_items.size();++i)
        if (m_items[i]->id() == id) return i;
    return -1;
}
void ServiceProviderModel::childsChanged(Collection* provider) {
    const int listpos = indexOf ( provider->id());
    QModelIndex index = createIndex ( listpos,0,0 );
    //qDebug() << m_title << "changed";
    emit dataChanged ( index,index );
}
