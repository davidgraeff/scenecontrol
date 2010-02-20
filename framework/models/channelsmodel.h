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

#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H

#include <QModelIndex>
#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QIcon>

class AbstractStateTracker;
class ChannelNameStateTracker;
class ChannelValueStateTracker;
class ChannelsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ChannelsModel (QObject* parent = 0);
    ~ChannelsModel();

    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual bool setData ( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
    virtual Qt::ItemFlags flags ( const QModelIndex& index ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    void setValue ( int i, unsigned int value );
    void setName ( int i, const QString& value );
    QString getName ( int i );
    int getValue ( int i ) const;
private Q_SLOTS:
    void stateChanged(AbstractStateTracker*);
    void slotdisconnected();
private:
    QList<ChannelValueStateTracker*> m_itemvalues;
    QList<ChannelNameStateTracker*> m_itemnames;
    QMap<uint,int> m_assignment; // pin id -> list id
};

#endif // CHANNELSMODEL_H
