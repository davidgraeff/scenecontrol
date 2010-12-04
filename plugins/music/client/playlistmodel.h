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

#ifndef PLAYLISTSMODEL_H
#define PLAYLISTSMODEL_H

#include <QModelIndex>
#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QIcon>
#include "services/playlist.h"
#include "shared/client/clientplugin.h"

class AbstractServiceProvider;
class ActorPlaylist;
class PlaylistModel : public ClientModel
{
    Q_OBJECT
public:
    PlaylistModel (QObject* parent=0);
    ~PlaylistModel();
    
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual bool setData ( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
    virtual Qt::ItemFlags flags ( const QModelIndex& index ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

    /**
      * Set playlist, that is currently beeing played
      */
    void setCurrentActorPlaylist ( int pos ) ;

    /**
      * Get the name of the playlist with the index i.
      * Used by the rename dialog
      */
    inline QString getName ( int i ) const
    {
        return m_items.at ( i )->name();
    }
    void setName ( int i, const QString& name);

    /**
      * Get the playlist with the index i.
      * Used to get the file models.
      */
    inline ActorPlaylist* getActorPlaylist ( int i ) const
    {
      if (i==-1) return 0;
        return m_items.at ( i );
    }

    /**
      * Get the playlist with the id id
      * Used to get the file models.
      */
    inline ActorPlaylist* getActorPlaylistByID ( const QString& id, int* listpos = 0 ) const
    {
        for (int i=0;i<m_items.size();++i)
            if (m_items[i]->id()==id) {
                if (listpos)
                    *listpos = i;
                return m_items[i];
            }
        return 0;
    }
    /**
      * Get the playlist position with the id id
      * Used to get the file models.
      */
    inline int getPositionByID ( const QString& id) const
    {
        for (int i=0;i<m_items.size();++i)
            if (m_items[i]->id()==id) {
                return i;
            }
        return -1;
    }

    /**
      * Request the server to add a playlist.
      * The server does not have to follow that request.
      * If it does it will propagated via the stateChanged
      * slot.
      */
    void addRequest ( const QString& name );
	virtual int indexOf(const QVariant&) {return -1;}
private Q_SLOTS:
    void stateTrackerChanged(AbstractStateTracker*);
    void serviceRemoved(AbstractServiceProvider*);
    void serviceChanged(AbstractServiceProvider*);
	void clear();
private:
    QList< ActorPlaylist* > m_items;
    int m_current;
};

#endif // PLAYLISTMODEL_H
