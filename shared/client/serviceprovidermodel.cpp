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
#include <shared/abstractserviceprovider.h>
#include <shared/categorize/profile.h>

ServiceProviderModel::ServiceProviderModel ( const QString& title, QObject* parent )
        : ClientModel ( parent ), m_title(title)
{

}

ServiceProviderModel::~ServiceProviderModel()
{
    clear();
}

void ServiceProviderModel::clear()
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
        return p->id();
    }
    else if ( role==Qt::UserRole+1 )
    {
        return p->type();
    }
    return QVariant();
}

bool ServiceProviderModel::removeRows ( int row, int count, const QModelIndex & )
{
    QString str;
    for ( int i=row+count-1;i>=row;--i )
    {
        m_items[i]->setProperty("remove",1);
        emit changeService(m_items.at ( i ));
    }
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
    return true;
}

void ServiceProviderModel::serviceChanged ( AbstractServiceProvider* provider )
{
//FIXME filter profiles and categories
    const int listpos = indexOf ( provider->id());
    if ( listpos!=-1 ) {
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
    } else {
        int row;
        for (row=0;row<m_items.size();++row)
            if (m_items[row]->toString().toLower() >= provider->toString().toLower())
                break;

        beginInsertRows ( QModelIndex(),row,row );
        m_items.insert (row, provider );
        endInsertRows();
    }
}

void ServiceProviderModel::serviceRemoved ( AbstractServiceProvider* provider )
{
    const int listpos = indexOf ( provider->id() );
    if ( listpos==-1 ) return;
    beginRemoveRows ( QModelIndex(), listpos, listpos );
    m_items.removeAt ( listpos );
    endRemoveRows();
}

QString ServiceProviderModel::getName ( int i ) const
{
    return m_items.at ( i )->toString();
}

int ServiceProviderModel::indexOf ( const QVariant& data )
{    
	if (data.type()!=QVariant::String) return -1;
    const QString id = data.toString();
	
    for (int i=0;i<m_items.size();++i)
        if (m_items[i]->id() == id) return i;
    return -1;
}

void ServiceProviderModel::stateTrackerChanged(AbstractStateTracker*) {}
