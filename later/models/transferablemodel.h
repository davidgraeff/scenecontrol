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

#ifndef TRANSFERABLEMODEL_H
#define TRANSFERABLEMODEL_H

#include <QModelIndex>
#include <QUuid>
#include <QStringList>
#include <QApplication>
#include <QVector>
#include <roomclient.h>

class RoomClient;

class ModelItem
{
public:
    ModelItem ( const QStringList& objectstring );
    void setData ( const QStringList& objectstring );
    void setLoading ( bool b );
public:
    bool isLoading;
    QStringList objectstring;
    QUuid id;
    QUuid timestamp;
    QString name;
    int type;
    bool checked;
    int delay;
};

class TransferableModel : public QAbstractListModel
{
    Q_OBJECT
public:
    TransferableModel ( RoomClient* client, bool checkable, QVector<int> acceptedTypes );
    ~TransferableModel();
    virtual bool dropMimeData ( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent );
    virtual QVariant data ( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex& parent = QModelIndex() ) const;
    virtual bool setData ( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
    virtual bool removeRows ( int row, int count, const QModelIndex& parent = QModelIndex() );
    virtual Qt::ItemFlags flags ( const QModelIndex& index ) const;
    QList<QUuid> getChecked();
    int indexOf ( const QUuid& id );
    void setChecked ( const QList<QUuid>& ids );
    QUuid getItemID ( int index );
    QStringList getItemObjectstring ( int index );
    void setItemLoading ( int index );
private Q_SLOTS:
    void changed ( int type, QStringList& );
    void removed ( int type, QStringList& );
    void stateChanged ( RoomClientState state );
Q_SIGNALS:
    void rename ( const QUuid& id, const QString& name );
private:
    QList<ModelItem*> list;
    QMap<QUuid, ModelItem*> map;
    RoomClient* client;
    bool checkable;
    QVector< int > acceptedTypes;
};

#endif // TRANSFERABLEMODEL_H
