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

#include "profilesmodel.h"
#include <Transferables/transferable.h>

ProfilesModel::ProfilesModel ( RoomClient* client, bool checkable)
        : client ( client ), checkable ( checkable )
{
    connect ( client,SIGNAL ( changed ( int,QStringList& ) ), SLOT ( changed ( int,QStringList& ) ) );
    connect ( client,SIGNAL ( removed ( int,QStringList& ) ),SLOT ( removed ( int,QStringList& ) ) );
    connect ( client,SIGNAL(stateChanged(RoomClientState)),SLOT(stateChanged(RoomClientState)));
    connect( this, SIGNAL( rename(QUuid,QString)), client, SLOT(rename(QUuid,QString)) );
}

ProfilesModel::~ProfilesModel()
{
    qDeleteAll ( list );
}

QUuid ProfilesModel::getItemID ( int index )
{
    return list.at ( index )->id;
}

QStringList ProfilesModel::getItemObjectstring ( int index )
{
    return list.at ( index )->objectstring;
}

int ProfilesModel::indexOf ( const QUuid& id ) {
  for (int i=0;i<list.size();++i)
    if (list.at(i)->id == id) return i;
  return -1;
}

void ProfilesModel::setItemLoading ( int index )
{
    list.at ( index )->setLoading(true);
    QModelIndex oben = createIndex ( index,0,0 );
    emit dataChanged ( oben, oben );
}

void ProfilesModel::changed ( int type, QStringList& objectstring )
{
    if ( type != ProfileType ) return;

    QUuid id = QUuid ( objectstring[1] );
    ModelItem* item = map.value ( id );
    if ( item )
    {
        int row = list.indexOf ( item );
        item->setData ( objectstring );
        QModelIndex oben = createIndex ( row,0,0 );
        emit dataChanged ( oben, oben );
    }
    else
    {
        beginInsertRows ( QModelIndex(), list.size(), list.size() );
        item = new ModelItem ( objectstring );
        list.append ( item );
        endInsertRows();
        map.insert ( item->id, item );
    }
    if ( item->isLoading )
    {
        client->request ( item->id, item->timestamp );
    }
}

void ProfilesModel::removed ( int type, QStringList& objectstring )
{
    if ( type != ProfileType ) return;

    QUuid id = QUuid ( objectstring[1] );
    ModelItem* item = map.value ( id );
    if ( !item ) return;
    map.remove ( id );
    int row = list.indexOf ( item );
    beginRemoveRows ( QModelIndex(), row, row );
    delete list.takeAt ( row );
    endRemoveRows();
}

QMap<QUuid,int> ProfilesModel::getChecked()
{
    QMap<QUuid,int> ids;
    foreach ( ModelItem* item, list )
    {
        if ( item->checked ) ids.insert ( item->id, item->delay );
    }
    return ids;
}

void ProfilesModel::setChecked ( const QMap<QUuid,int>& ids )
{
    QMap<QUuid,int>::const_iterator it;
    for ( int row=0;row<list.size();++row )
    {
        ModelItem* item = list.at ( row );
	it = ids.find( item->id);
	if (it != ids.constEnd()) {
	  item->checked = true;
	  item->delay = it.value();
	} else {
	  item->checked = false;
	  item->delay = 0;
	}
    }
    reset();
}

bool ProfilesModel::dropMimeData ( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
    return QAbstractListModel::dropMimeData ( data, action, row, column, parent );
}

QVariant ProfilesModel::data ( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();
    ModelItem* item = list.at ( index.row() );
    if ( (index.column()==0) && (role==Qt::DisplayRole || role==Qt::EditRole) )
        return item->name;
    else if ( (index.column()==1) && (role==Qt::DisplayRole || role==Qt::EditRole) )
        return item->delay;
    else if (index.column()==0 && checkable && role == Qt::CheckStateRole) {
      return (item->checked?Qt::Checked:Qt::Unchecked);
    }

    return QVariant();
}

int ProfilesModel::rowCount ( const QModelIndex&  ) const
{
    return list.size();
}

int ProfilesModel::columnCount(const QModelIndex& ) const {
    return 2;
}

QVariant ProfilesModel::headerData ( int section, Qt::Orientation orientation, int role ) const {
    if (orientation == Qt::Horizontal) {
      if (role == Qt::DisplayRole && section==0) {
	return tr("Profilname");
      } else
      if (role == Qt::DisplayRole && section==1) {
	return tr("Verzögerung");
      } else
      if (role == Qt::ToolTipRole && section==1) {
	return tr("Verzögerung in Sekunden, bis dieses Profil aktiviert wird");
      } else
      if (role == Qt::ToolTipRole && section==0) {
	return tr("Wenn per Checkbox eingeschaltet, dann wird dieses Profil bei Alarmauslösung aktiviert");
      }
    }

    return QVariant();
}

bool ProfilesModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() ) return false;
    ModelItem* item = list.at ( index.row() );

    if ( index.column()==0 && role == Qt::EditRole )
    {
        QString newname = value.toString().trimmed().replace ( '\n',"" ).replace ( '\t',"" );
        if ( newname.isEmpty() ) return false;

        item->setLoading(true);
	emit rename(item->id, newname);
        return true;
    }
    else if ( index.column()==1 && role == Qt::EditRole )
    {
      bool ok;
        uint newdelay = value.toUInt(&ok);
        if ( !ok ) return false;
	item->delay = newdelay;
	QModelIndex oben = createIndex ( index.row(),1,0 );
	emit dataChanged ( oben, oben );	
        return true;
    }
    else if ( index.column()==0 && checkable && role == Qt::CheckStateRole )
    {
      //qDebug() << value << item->checked;
        item->checked = (value.toInt()==Qt::Checked);
	QModelIndex oben = createIndex ( index.row(),0,0 );
	emit dataChanged ( oben, oben );
        return true;
    }

    return false;
}

bool ProfilesModel::removeRows ( int row, int count, const QModelIndex&  )
{
    QStringList ids;
    for ( int i=row+count-1;i>=row;--i )
    {
        list.at ( i )->setLoading(true);
        ids.append ( list.at ( i )->id );
    }
    QModelIndex oben = createIndex ( row,0,0 );
    QModelIndex unten = createIndex ( row+count-1,0,0 );
    emit dataChanged ( oben, unten );
    client->removeMany ( ids );
    return true;
}

Qt::ItemFlags ProfilesModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    if ( list.at ( index.row() )->isLoading )
        return Qt::ItemIsEnabled;
    else if (index.column()==0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable  | ( checkable? Qt::ItemIsUserCheckable:Qt::ItemIsEnabled );
    else if (index.column()==1) {
      if (list.at(index.row())->checked) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
      } else {
	return Qt::ItemIsEnabled;
      }
    }
    
    return QAbstractListModel::flags(index);
}

void ProfilesModel::stateChanged ( RoomClientState state)
{
  if (state == ConnectionDisconnected) {
    qDeleteAll ( list );
    map.clear();
    list.clear();
    reset();
  }
}
