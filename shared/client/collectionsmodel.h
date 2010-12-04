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

#pragma once
#include <QModelIndex>
#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QIcon>
#include "shared/client/clientplugin.h"

class Collection;
class AbstractServiceProvider;

class CollectionsModel : public ClientModel
{
    Q_OBJECT
public:
    CollectionsModel (QObject* parent = 0);
    ~CollectionsModel();

    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

//	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

	virtual int indexOf(const QVariant& data);
	void focusAsSoonAsAvailable(const QString& id) {m_focusid=id;}
public Q_SLOTS:
    virtual void clear();
    virtual void serviceChanged(AbstractServiceProvider*);
    virtual void serviceRemoved(AbstractServiceProvider* );
    virtual void stateTrackerChanged(AbstractStateTracker* );
Q_SIGNALS:
    void itemMoved(const QModelIndex& newindex);
private:
    QList< Collection* > m_collections;
    QString m_focusid;
};
