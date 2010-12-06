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

#include "playlistsModel.h"
#include <QSettings>
#include <qfileinfo.h>
#include <QPalette>
#include <QApplication>
#include <QUrl>
#include <QDebug>
#include <qmimedata.h>
#include <shared/abstractstatetracker.h>
#include <statetracker/playliststatetracker.h>

PlaylistsModel::PlaylistsModel (QObject* parent)
        : ClientModel(parent)
{
}

int PlaylistsModel::rowCount ( const QModelIndex & ) const
{
    return m_items.size();
}

int PlaylistsModel::columnCount ( const QModelIndex & ) const
{
    return 1;
}

QVariant PlaylistsModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( role == Qt::DisplayRole && section==0 )
        {
            return tr ( "Playlistname" );
        }
    }

    return QAbstractListModel::headerData ( section, orientation, role );
}

QVariant PlaylistsModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() && index.column()!=0) return QVariant();

    if ( role==Qt::DisplayRole || role == Qt::EditRole )
    {
        return m_items.at(index.row()).name;
    }
    else if (role ==Qt::ToolTipRole)
    {
        return m_items.at(index.row()).lastModified;
    } else if (role ==Qt::UserRole)
    {
        return m_items.at(index.row()).name;
    }

    return QVariant();
}

int PlaylistsModel::indexOf(const QVariant& data) {
    if (data.type()!=QVariant::String) return -1;
    const QString id = data.toString();

    for (int i=0;i<m_items.size();++i)
        if (m_items[i].name == id) return i;
    return -1;
}

void PlaylistsModel::serviceChanged ( AbstractServiceProvider*)
{
}

void PlaylistsModel::serviceRemoved ( AbstractServiceProvider*)
{
}

void PlaylistsModel::stateTrackerChanged(AbstractStateTracker* statetracker) {
    PlaylistStateTracker* t = qobject_cast<PlaylistStateTracker*>(statetracker);
    if (!t) return;
    // change data
    for (int i=0;i<m_items.size();++i) {
        if (m_items[i].name == t->name()) {
            m_items[i].lastModified = t->lastModified();
            emit dataChanged(createIndex(i,0,0), createIndex(i,columnCount(),0));
            return;
        }
    }
    // new data
    beginInsertRows(QModelIndex(),m_items.count(),m_items.count());
    m_items.append(PlaylistsModelItem(t->name(),t->lastModified()));
    endInsertRows();
}

void PlaylistsModel::clear() {
    m_items.clear();
    reset();
}
