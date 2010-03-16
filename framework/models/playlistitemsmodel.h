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

#ifndef PLAYLISTITEMSMODEL_H
#define PLAYLISTITEMSMODEL_H

#include <QModelIndex>
#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QIcon>

class Playlist;
class PlaylistItemsModel : public QAbstractTableModel
{
    Q_OBJECT
    friend class Playlist;
public:
    PlaylistItemsModel (Playlist* playlist, QObject* parent=0);

    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual bool setData ( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
    virtual Qt::ItemFlags flags ( const QModelIndex& index ) const;
    bool removeRows ( QModelIndexList list );
    bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    Qt::DropActions supportedDropActions() const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QStringList mimeTypes() const;
    QMimeData *mimeData( const QModelIndexList & indexes ) const;
    bool m_waitforsync;
    void updateCurrenttrack( int old ) ;
  private:
    Playlist* m_playlist;
    
    bool addFile(QUrl url, int row);
};

#endif // PLAYLISTITEMSMODEL_H
