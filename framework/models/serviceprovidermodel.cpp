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
    // Do not delete playlist objects, they are managed through the factory
    m_items.clear();
    m_assignment.clear();
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

    if ( role==Qt::DisplayRole )
    {
        return m_items.at ( index.row() )->toString();
    }
    else if ( role==Qt::UserRole )
    {
        return m_items.at ( index.row() )->metaObject()->className();
    }

    return QVariant();
}

bool ServiceProviderModel::removeRows ( int row, int count, const QModelIndex & )
{
    QString str;
    for ( int i=row+count-1;i>=row;--i )
    {
        RoomControlClient::getFactory()->requestRemoveProvider ( m_items[i] );
    }
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
    return true;
}

void ServiceProviderModel::addedProvider ( AbstractServiceProvider* provider )
{
    const int listpos = m_assignment.value ( provider->id(),-1 );
    if ( listpos!=-1 ) return;

    connect ( provider, SIGNAL ( objectChanged ( AbstractServiceProvider* ) ),
              SLOT ( objectChanged ( AbstractServiceProvider* ) ) );
    const int row = m_items.size();
    m_assignment.insert ( provider->id(), row );
    beginInsertRows ( QModelIndex(),row,row );
    m_items.append ( provider );
    endInsertRows();
}

void ServiceProviderModel::removedProvider ( AbstractServiceProvider* provider )
{
    const int listpos = m_assignment.value ( provider->id(),-1 );
    if ( listpos==-1 ) return;
    m_assignment.remove ( provider->id() );
    beginRemoveRows ( QModelIndex(), listpos, listpos );
    m_items.removeAt ( listpos );
    endRemoveRows();
}

void ServiceProviderModel::objectChanged ( AbstractServiceProvider* provider )
{
    const int listpos = m_assignment.value ( provider->id(),-1 );
    if ( listpos==-1 ) return;
    QModelIndex index = createIndex ( listpos,0,0 );
    emit dataChanged ( index,index );
    return;
}

QString ServiceProviderModel::getName ( int i ) const
{
    return m_items.at ( i )->toString();
}

int ServiceProviderModel::indexOf ( const QString& id )
{
    return m_assignment.value ( id,-1 );
}
