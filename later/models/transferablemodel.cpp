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

#include "transferablemodel.h"
#include <kcategorizedsortfilterproxymodel.h>

ModelItem::ModelItem ( const QStringList& objectstring )
{
    delay = 0;
    checked = false;
    setData ( objectstring );
}

void ModelItem::setData ( const QStringList& objectstring )
{
    this->objectstring = objectstring;
    Q_ASSERT ( objectstring.size() >2 );
    type = Transferable::determineType ( objectstring[0] );
    id = objectstring[1];
    timestamp = objectstring[2];
    if ( objectstring.size() >3 )
    {
        setLoading ( false );
    }
    else
    {
        setLoading ( true );
    }
}

void ModelItem::setLoading ( bool b )
{
    isLoading = b;
    if ( b )
        name = QApplication::translate ( "ModelItem", "Lade..." );
    else
        name = objectstring[3];
}

//////////////////////////////////////////

TransferableModel::TransferableModel ( RoomClient* client, bool checkable, QVector< int > acceptedTypes )
        : client ( client ), checkable ( checkable ), acceptedTypes ( acceptedTypes )
{
    connect ( client,SIGNAL ( changed ( int,QStringList& ) ), SLOT ( changed ( int,QStringList& ) ) );
    connect ( client,SIGNAL ( removed ( int,QStringList& ) ),SLOT ( removed ( int,QStringList& ) ) );
    connect ( client,SIGNAL ( stateChanged ( RoomClientState ) ),SLOT ( stateChanged ( RoomClientState ) ) );
    connect ( this, SIGNAL ( rename ( QUuid,QString ) ), client, SLOT ( rename ( QUuid,QString ) ) );
}

TransferableModel::~TransferableModel()
{
    qDeleteAll ( list );
}

QUuid TransferableModel::getItemID ( int index )
{
    return list.at ( index )->id;
}

QStringList TransferableModel::getItemObjectstring ( int index )
{
    return list.at ( index )->objectstring;
}

int TransferableModel::indexOf ( const QUuid& id )
{
    for ( int i=0;i<list.size();++i )
        if ( list.at ( i )->id == id ) return i;
    return -1;
}

void TransferableModel::setItemLoading ( int index )
{
    list.at ( index )->setLoading ( true );
    QModelIndex oben = createIndex ( index,0,0 );
    emit dataChanged ( oben, oben );
}

void TransferableModel::changed ( int type, QStringList& objectstring )
{
    if ( !acceptedTypes.contains ( type ) ) return;

    QUuid id = QUuid ( objectstring[1] );
    ModelItem* item = map.value ( id );
    if ( item )
    {
        int row = list.indexOf ( item );
        item->setData ( objectstring );
        QModelIndex oben = createIndex ( row,0,0 );
        emit dataChanged ( oben, oben );
	//reset();
    }
    else
    {
        beginInsertRows ( QModelIndex(), list.size(), list.size() );
        item = new ModelItem ( objectstring );
        list.append ( item );
        endInsertRows();
        map.insert ( item->id, item );
    }
}

void TransferableModel::removed ( int type, QStringList& objectstring )
{
    if ( !acceptedTypes.contains ( type ) ) return;

    QUuid id = QUuid ( objectstring[1] );
    ModelItem* item = map.value ( id );
    if ( !item ) return;
    map.remove ( id );
    int row = list.indexOf ( item );
    beginRemoveRows ( QModelIndex(), row, row );
    delete list.takeAt ( row );
    endRemoveRows();
}

QList<QUuid> TransferableModel::getChecked()
{
    QList<QUuid> ids;
    foreach ( ModelItem* item, list )
    {
        if ( item->checked ) ids.append ( item->id );
    }
    return ids;
}

void TransferableModel::setChecked ( const QList<QUuid>& ids )
{
    for ( int row=0;row<list.size();++row )
    {
        ModelItem* item = list.at ( row );
        item->checked = ids.contains ( item->id );
    }
    reset();
}

bool TransferableModel::dropMimeData ( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
    return QAbstractListModel::dropMimeData ( data, action, row, column, parent );
}

QVariant TransferableModel::data ( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();
    ModelItem* item = list.at ( index.row() );
    switch ( role )
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return item->name;
    case Qt::CheckStateRole:
        if ( checkable ) return ( item->checked?Qt::Checked:Qt::Unchecked );
        else break;
    case KCategorizedSortFilterProxyModel::CategorySortRole:
        return item->type;
    case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
        return item->objectstring[0];
    default:
        break;
    }
    return QVariant();
}

int TransferableModel::rowCount ( const QModelIndex& ) const
{
    return list.size();
}

bool TransferableModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() ) return false;
    ModelItem* item = list.at ( index.row() );

    if ( role == Qt::EditRole )
    {
        QString newname = value.toString().trimmed().replace ( '\n',"" ).replace ( '\t',"" );
        if ( newname.isEmpty() || item->name == newname ) return false;

        item->setLoading ( true );
        emit rename ( item->id, newname );
        return true;
    }
    else if ( checkable && role == Qt::CheckStateRole )
    {
        //qDebug() << value << item->checked;
        item->checked = ( value.toInt() ==Qt::Checked );
        QModelIndex oben = createIndex ( index.row(),0,0 );
        emit dataChanged ( oben, oben );
        return true;
    }

    return false;
}

bool TransferableModel::removeRows ( int row, int count, const QModelIndex& )
{
    QStringList ids;
    for ( int i=row+count-1;i>=row;--i )
    {
        list.at ( i )->setLoading ( true );
        ids.append ( list.at ( i )->id );
    }
    QModelIndex oben = createIndex ( row,0,0 );
    QModelIndex unten = createIndex ( row+count-1,0,0 );
    emit dataChanged ( oben, unten );
    client->removeMany ( ids );
    return true;
}

Qt::ItemFlags TransferableModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    if ( list.at ( index.row() )->isLoading )
        return Qt::ItemIsEnabled;
    else
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable  | ( checkable? Qt::ItemIsUserCheckable:Qt::ItemIsEnabled );
}

void TransferableModel::stateChanged ( RoomClientState state )
{
    if ( state == ConnectionDisconnected )
    {
        qDeleteAll ( list );
        map.clear();
        list.clear();
        reset();
    }
}
