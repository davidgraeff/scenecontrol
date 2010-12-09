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

#ifndef ServiceProviderModel_h
#define ServiceProviderModel_h

#include <QModelIndex>
#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QIcon>
#include "shared/client/clientplugin.h"

class Collection;
class AbstractServiceProvider;
class ServiceProviderModel : public ClientModel
{
    Q_OBJECT
public:
    ServiceProviderModel (const QString& title, QObject* parent = 0);
    ~ServiceProviderModel();

    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    QString getName ( int i ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    inline AbstractServiceProvider* get(int index) { return m_items.at(index);}
    virtual int indexOf ( const QVariant& data ) ;
    void addedProvider(AbstractServiceProvider*);
public Q_SLOTS:
    virtual void clear();
    virtual void serviceChanged(AbstractServiceProvider* );
    virtual void serviceRemoved(AbstractServiceProvider* );
    virtual void stateTrackerChanged(AbstractStateTracker* );
private:
    QList< AbstractServiceProvider* > m_items;
    QString m_title;
};

#endif // ServiceProviderModel_h
