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

#include "playlistmodel.h"
#include <QStringList>
#include <QDebug>
#include <RoomControlClient.h>

PlaylistModel::PlaylistModel ( QObject* parent )
        : QAbstractListModel ( parent )
{
    connect(RoomControlClient::getFactory(),SIGNAL(addedProvider(AbstractServiceProvider*)),
            SLOT(addedProvider(AbstractServiceProvider*)));
    connect(RoomControlClient::getFactory(),SIGNAL(removedProvider(AbstractServiceProvider*)),
            SLOT(removedProvider(AbstractServiceProvider*)));
    connect(RoomControlClient::getNetworkController(),SIGNAL(disconnected()),
            SLOT(slotdisconnected()));
}

PlaylistModel::~PlaylistModel()
{
    slotdisconnected();
}

void PlaylistModel::slotdisconnected()
{
    // Do not delete playlist objects, they are managed through the factory
    m_items.clear();
    m_assignment.clear();
    reset();
}

int PlaylistModel::rowCount ( const QModelIndex & ) const
{
    return m_items.size();
}

QVariant PlaylistModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();

    if ( role==Qt::DisplayRole || role==Qt::EditRole )
    {
        return m_items.at ( index.row() )->name();
    }

    return QVariant();
}


QVariant PlaylistModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( role == Qt::DisplayRole && section==0 )
        {
            return tr ( "Abspielliste" );
        }
    }

    return QAbstractListModel::headerData ( section, orientation, role );
}

bool PlaylistModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() ) return false;

    if ( index.column() ==0 && role == Qt::EditRole )
    {
        QString newname = value.toString().trimmed().replace ( '\n',"" ).replace ( '\t',"" );
        if ( newname.isEmpty() || newname == m_items[index.row() ]->name() ) return false;
        setName(index.row(),newname);
        QModelIndex index = createIndex ( index.row(),0,0 );
        emit dataChanged ( index,index );
        return true;
    }
    return false;
}

bool PlaylistModel::removeRows ( int row, int count, const QModelIndex & )
{
    QString str;
    for ( int i=row+count-1;i>=row;--i )
    {
        RoomControlClient::getFactory()->requestRemoveProvider(m_items[i]);
    }
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
    return true;
}

Qt::ItemFlags PlaylistModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    return QAbstractListModel::flags ( index ) | Qt::ItemIsEditable;
}

bool sortPlaylists ( const Playlist* &s1, const Playlist* &s2 )
{
    return s1->name() < s2->name();
}

void PlaylistModel::addedProvider(AbstractServiceProvider* provider)
{
    Playlist* p = qobject_cast<Playlist*>(provider);
    if (!p) return;
    const int listpos = m_assignment.value(provider->id(),-1);
    if ( listpos!=-1 ) return;

    connect(p, SIGNAL(objectChanged(AbstractServiceProvider*)),
            SLOT(objectChanged(AbstractServiceProvider*)));
    const int row = m_items.size();
    m_assignment.insert(p->id(), row);
    beginInsertRows ( QModelIndex(),row,row );
    m_items.append(p);
    endInsertRows();
}

void PlaylistModel::removedProvider(AbstractServiceProvider* provider)
{
    const int listpos = m_assignment.value(provider->id(),-1);
    if ( listpos==-1 ) return;
    m_assignment.remove(provider->id());
    beginRemoveRows ( QModelIndex(), listpos, listpos );
    m_items.removeAt(listpos);
    endRemoveRows();
}

void PlaylistModel::objectChanged(AbstractServiceProvider* provider)
{
    const int listpos = m_assignment.value(provider->id(),-1);
    if ( listpos==-1 ) return;
    QModelIndex index = createIndex ( listpos,0,0 );
    emit dataChanged ( index,index );
    return;
}

void PlaylistModel::addRequest ( const QString& name )
{
    Playlist* p = new Playlist();
    p->setName(name);
    RoomControlClient::getNetworkController()->objectSync(p);
    delete p;
}

void PlaylistModel::setName(int i, const QString& name)
{
    m_items.at(i)->setName(name);
    m_items.at(i)->sync();
}

