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

#ifndef BackupsModel_h
#define BackupsModel_h

#include <QModelIndex>
#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include "shared/client/clientplugin.h"
struct BackupItem {
	QString id;
	QString name;
};

class BackupsModel : public ClientModel
{
    Q_OBJECT
public:
    BackupsModel (QObject* parent = 0);
    ~BackupsModel();

    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
	
    virtual int indexOf(const QVariant& data);
public Q_SLOTS:
    virtual void clear();
    virtual void serviceChanged(AbstractServiceProvider* );
    virtual void serviceRemoved(AbstractServiceProvider* );
    virtual void stateTrackerChanged(AbstractStateTracker* );
private:
    QList<BackupItem> m_items;
};

#endif // BackupsModel_h
