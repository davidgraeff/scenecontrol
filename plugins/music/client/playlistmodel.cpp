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
#include "services/playlist.h"

ActorPlaylistModel::ActorPlaylistModel ( QObject* parent )
                : QAbstractListModel ( parent ), m_current ( 0 )
{
}

ActorPlaylistModel::~ActorPlaylistModel()
{
        slotdisconnected();
}

void ActorPlaylistModel::slotdisconnected()
{
        // Do not delete playlist objects, they are managed through the factory
        m_items.clear();
        reset();
}

int ActorPlaylistModel::rowCount ( const QModelIndex & ) const
{
        return m_items.size();
}

QVariant ActorPlaylistModel::data ( const QModelIndex & index, int role ) const
{
        if ( !index.isValid() ) return QVariant();

        if ( role==Qt::DisplayRole || role==Qt::EditRole ) {
                return m_items.at ( index.row() )->toString();
        } else if ( role==Qt::BackgroundColorRole && m_current==index.row() ) {
                return QColor ( Qt::darkGreen );
        }

        return QVariant();
}


QVariant ActorPlaylistModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
        if ( orientation == Qt::Horizontal ) {
                if ( role == Qt::DisplayRole && section==0 ) {
                        return tr ( "Abspielliste" );
                }
        }

        return QAbstractListModel::headerData ( section, orientation, role );
}

bool ActorPlaylistModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
        if ( !index.isValid() ) return false;

        if ( index.column() ==0 && role == Qt::EditRole ) {
                QString newname = value.toString().trimmed().replace ( '\n',"" ).replace ( '\t',"" );
                if ( newname.isEmpty() || newname == m_items[index.row() ]->name() ) return false;
                setName ( index.row(),newname );
                QModelIndex index = createIndex ( index.row(),0,0 );
                emit dataChanged ( index,index );
                return true;
        }
        return false;
}

bool ActorPlaylistModel::removeRows ( int row, int count, const QModelIndex & )
{
        QString str;
        for ( int i=row+count-1;i>=row;--i ) {
			m_items[i]->requestRemove();
        }
        QModelIndex ifrom = createIndex ( row,0 );
        QModelIndex ito = createIndex ( row+count-1,1 );
        emit dataChanged ( ifrom,ito );
        return true;
}

Qt::ItemFlags ActorPlaylistModel::flags ( const QModelIndex& index ) const
{
        if ( !index.isValid() )
                return 0;

        return QAbstractListModel::flags ( index ) | Qt::ItemIsEditable;
}

bool sortActorPlaylists ( const ActorPlaylist* &s1, const ActorPlaylist* &s2 )
{
        return s1->name() < s2->name();
}

void ActorPlaylistModel::addedProvider ( AbstractServiceProvider* provider )
{
        ActorPlaylist* p = qobject_cast<ActorPlaylist*> ( provider );
        if ( !p ) return;
        if ( getPositionByID ( provider->id() ) !=-1 ) return;

        connect ( p, SIGNAL ( objectChanged ( AbstractServiceProvider* ) ),
                  SLOT ( objectChanged ( AbstractServiceProvider* ) ) );
        const int row = m_items.size();
        beginInsertRows ( QModelIndex(),row,row );
        m_items.append ( p );
        endInsertRows();
}

void ActorPlaylistModel::removedProvider ( AbstractServiceProvider* provider )
{
        const int listpos = getPositionByID ( provider->id() );

        if ( listpos==-1 ) return;
        beginRemoveRows ( QModelIndex(), listpos, listpos );
        m_items.removeAt ( listpos );
        endRemoveRows();
}

void ActorPlaylistModel::objectChanged ( AbstractServiceProvider* provider )
{
        const int listpos = getPositionByID ( provider->id() );
        if ( listpos==-1 ) return;
        QModelIndex index = createIndex ( listpos,0,0 );
        emit dataChanged ( index,index );
        return;
}

void ActorPlaylistModel::addRequest ( const QString& name )
{
        ActorPlaylist* p = new ActorPlaylist();
        p->setName ( name );
        RoomControlClient::getNetworkController()->objectSync ( p );
        delete p;
}

void ActorPlaylistModel::setName ( int i, const QString& name )
{
        m_items.at ( i )->setName ( name );
        emit changeObject(m_items.at ( i ));
}

void ActorPlaylistModel::setCurrentActorPlaylist ( int pos )
{
        const int old = m_current;
        m_current = pos;
        const QModelIndex index = createIndex ( pos, 0 );
        emit dataChanged ( index, index );
        const QModelIndex index2 = createIndex ( old, 0 );
        emit dataChanged ( index2, index2 );
}

