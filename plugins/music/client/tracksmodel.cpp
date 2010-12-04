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

#include "tracksmodel.h"
#include <QSettings>
#include <qfileinfo.h>
#include <QPalette>
#include <QApplication>
#include <QUrl>
#include <QDebug>
#include <services/playlist.h>
#include <qmimedata.h>

PlaylistTracksModel::PlaylistTracksModel (ActorPlaylist* playlist, QObject* parent)
        : ClientModel(), m_waitforsync(false), m_playlist(playlist)
{
}

Qt::DropActions PlaylistTracksModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

int PlaylistTracksModel::rowCount ( const QModelIndex & ) const
{
    return m_playlist->m_files.size();
}

QVariant PlaylistTracksModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( role == Qt::DisplayRole && section==0 )
        {
            return tr ( "Name" );
        }
        else  if ( role == Qt::DisplayRole && section==1 )
        {
            return tr ( "Datei" );
        }
    }

    return QAbstractListModel::headerData ( section, orientation, role );
}


bool PlaylistTracksModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() || role != Qt::EditRole) return false;

    QString newname = value.toString().trimmed().replace ( QLatin1Char('\n'),QString() ).replace ( QLatin1Char('\t'),QString() );

    if ( index.column() ==0)
    {
        if ( newname.isEmpty() || newname == m_playlist->m_titles[index.row()] ) return false;

        m_playlist->m_titles[index.row()] = newname;
	emit serviceChanged(m_playlist);

        return true;
    } else if (index.column()==1)
    {
        if ( newname.isEmpty() || newname == m_playlist->m_files[index.row()] ) return false;
        m_playlist->m_files[index.row()] = newname;
        	emit serviceChanged(m_playlist);

        return true;
    }

    return false;
}

Qt::ItemFlags PlaylistTracksModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return Qt::ItemIsDropEnabled;

    if (m_waitforsync)
        return QAbstractListModel::flags ( index );
    else
        return QAbstractListModel::flags ( index ) | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

QMimeData *PlaylistTracksModel::mimeData( const QModelIndexList & indexes ) const
{
    if (m_playlist->m_files.isEmpty())
        return 0;

    QMimeData *mimeData = QAbstractListModel::mimeData(indexes);
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes) {
        if (index.isValid() && index.column()==1) {
            const QString text = data(index, Qt::DisplayRole).toString();
            stream << text;
        }
    }

    mimeData->setData(QLatin1String("text/uri-list"), encodedData);
    return mimeData;

}

QStringList PlaylistTracksModel::mimeTypes() const
{
    QStringList types = QAbstractListModel::mimeTypes();
    types << QLatin1String("text/uri-list");
    return types;
}

bool PlaylistTracksModel::addFile(QUrl url, int row)
{
    const QSettings settings;
    const QString localbase = settings.value ( QLatin1String("baselocal") ).toString();
    const QString remotebase = settings.value ( QLatin1String("baseremote") ).toString();

    if (row==-1)
        row = m_playlist->m_files.size();
    const QFileInfo info(url.path());
    const QString suffix = info.suffix().toLower();
    QString path = url.path();
    if (suffix == QLatin1String("pls"))
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
            return false;
        QList<QByteArray> filecontent = f.readAll().split('\n');
        f.close();
        foreach (QByteArray line, filecontent) {
            QList<QByteArray> l = line.split('=');
            if (l.size()==2 && l[0].toLower()=="file1") {
                QByteArray suburl = l[1];
                suburl = suburl.replace('\r',"").replace('\n',"");
                return addFile(QUrl(QString::fromUtf8(suburl)), row);
            }
        }
    } if (suffix == QLatin1String("m3u"))
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
            return false;
        QList<QByteArray> filecontent = f.readAll().split('\n');
        f.close();
        foreach (QByteArray line, filecontent) {
	  line = line.trimmed().replace('\r',"").replace('\n',"");
	  if (line.size() && line[0] == '#') continue;
          return addFile(QUrl(QString::fromUtf8(line)), row);
        }
    } else if (url.scheme().startsWith(QLatin1String("http")))
    {
        m_playlist->m_files.insert(row, url.toString());
        m_playlist->m_titles.insert(row, url.authority());
        return true;
    } else if (path.startsWith ( localbase ) && (suffix == QLatin1String("mp3") || suffix == QLatin1String("ogg")))
    {
        path = path.replace ( 0, localbase.length(), remotebase );
        m_playlist->m_files.insert(row, path);
        m_playlist->m_titles.insert(row, info.completeBaseName());
        return true;
    }
    return false;
}


bool PlaylistTracksModel::dropMimeData(const QMimeData *data,
                                      Qt::DropAction action, int row, int column, const QModelIndex &)
{
    Q_UNUSED(column);

    if (action == Qt::IgnoreAction)
        return true;

    QByteArray itemData = data->data(QLatin1String("application/x-qabstractitemmodeldatalist"));
    if (itemData.size() && row!=-1) {
      QDataStream stream(&itemData, QIODevice::ReadOnly);

      while (!stream.atEnd()) {
	int r, c;
	QMap<int, QVariant> v;
	stream >> r >> c >> v;
	qDebug()<< r << row << m_playlist->m_files.size();
	
	QString title = m_playlist->m_titles[r];
	QString filename = m_playlist->m_files[r];
	if (action == Qt::MoveAction) {
	  m_playlist->m_files.removeAt ( r );
	  m_playlist->m_titles.removeAt( r );
	  if (r<row) --row;
	}
	m_playlist->m_files.insert(row,filename);
        m_playlist->m_titles.insert(row,title);
/*	const QModelIndex index = createIndex(qMin(r,row), 0);
	const QModelIndex index2 = createIndex(qMax(r,row), 1);
	emit dataChanged(index, index2);*/
      }
	emit serviceChanged(m_playlist);
      return false;
    }
    
    if (!data->hasUrls())
        return false;

    QList<QUrl> urls = data->urls();
    int count = -1;
    foreach (QUrl url, urls)
    {
        if (addFile(url, row))
            ++count;
    }

    if (count<0) return false;
	emit serviceChanged(m_playlist);

    return true;
}

QVariant PlaylistTracksModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();

    if ( role==Qt::DisplayRole || role == Qt::EditRole )
    {
        if ( index.column() ==0 )
            return m_playlist->m_titles.at(index.row());
        else if ( index.column() ==1 )
            return m_playlist->m_files.at(index.row());
    }
    else if (role ==Qt::ToolTipRole)
    {
      return m_playlist->m_files.at(index.row());
    }
    else if ( role==Qt::BackgroundColorRole )
    {
        if (m_waitforsync)
            return Qt::lightGray;
        return (m_playlist->currentTrack()==index.row()?QColor(Qt::darkGreen):QApplication::palette().color(QPalette::Base));
    }

    return QVariant();
}

bool PlaylistTracksModel::removeRows ( int row, int count, const QModelIndex & )
{
    QString str;
    for ( int i=row+count-1;i>=row;--i )
    {
        m_playlist->m_files.removeAt ( i );
        m_playlist->m_titles.removeAt( i );
    }

    if (count) emit serviceChanged(m_playlist);
    return true;
}

bool PlaylistTracksModel::removeRows ( QModelIndexList list )
{
    for (int i = list.size()-1; i >= 0; i--)
    {
        m_playlist->m_files.removeAt ( list[i].row() );
        m_playlist->m_titles.removeAt( list[i].row() );
    }
    if (list.size()) emit serviceChanged(m_playlist);
    return true;
}

bool PlaylistTracksModel::insertRows ( int row, int count, const QModelIndex &)
{
    beginInsertRows(QModelIndex(), row, row+count-1);
    for ( int i=row+count-1;i>=row;--i )
    {
        m_playlist->m_files.insert(row,QString());
        m_playlist->m_titles.insert(row,QString());
    }
    endInsertRows();
    //if (count) m_playlist->sync();
    return true;
}

int PlaylistTracksModel::columnCount ( const QModelIndex & ) const
{
    return 1;//2;
}
void PlaylistTracksModel::updateCurrenttrack(int old) {
    const QModelIndex index = createIndex(m_playlist->currentTrack(), 0);
    emit dataChanged(index, index);
    const QModelIndex index2 = createIndex(old, 0);
    emit dataChanged(index2, index2);
}
