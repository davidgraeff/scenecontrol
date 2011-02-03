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
#include "collectionsmodel.h"
#include <QStringList>
#include <QDebug>
#include <shared/categorize/profile.h>
#include <servicestorage.h>

CollectionsModel::CollectionsModel ( QObject* parent )
        : ClientModel ( parent )
{
    clear();
}

CollectionsModel::~CollectionsModel()
{
    m_collections.clear();
}

void CollectionsModel::clear()
{
    m_collections.clear();
    reset();
}

int CollectionsModel::rowCount ( const QModelIndex & ) const
{
    return m_collections.size();
}

int CollectionsModel::columnCount(const QModelIndex& ) const {
    return 1;
}

Qt::ItemFlags CollectionsModel::flags(const QModelIndex& index) const {
    return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

QVariant CollectionsModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( role == Qt::DisplayRole && section==0 )
        {
            return tr("Profiles");
        }
    }

    return QAbstractItemModel::headerData ( section, orientation, role );
}

QVariant CollectionsModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();
    Collection* collection = m_collections[index.row()];
	if ( role == Qt::UserRole ) return collection->id();
    else if (role==Qt::DisplayRole || role==Qt::EditRole) {
        return collection->name();
    } else if (role==Qt::CheckStateRole) {
        return (collection->enabled()?Qt::Checked:Qt::Unchecked);
    } else
        return QVariant();
}

bool CollectionsModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() || index.column() !=0 ) return false;
    Collection* collection = m_collections[index.row()];

    if ( role == Qt::EditRole ) {
        QString newname = value.toString().trimmed().replace ( QLatin1Char('\n'),QString() ).replace ( QLatin1Char('\t'),QString() );
        if ( newname.isEmpty() || newname == collection->name() ) return false;
        collection->setName(newname);
	ServiceStorage::instance()->serviceHasChanged(collection);
        QModelIndex index = createIndex ( index.row(),0,0 );
        emit dataChanged ( index,index );
        return true;
    } else if ( role == Qt::CheckStateRole) {
        collection->setEnabled(value.toInt()==Qt::Checked);
	ServiceStorage::instance()->serviceHasChanged(collection);
        QModelIndex index = createIndex ( index.row(),0,0 );
        emit dataChanged ( index,index );
        return true;
    }
    return false;
}

bool CollectionsModel::removeRows ( int row, int count, const QModelIndex & )
{
    for ( int i=row+count-1;i>=row;--i )
	ServiceStorage::instance()->deleteService(m_collections[i]);
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
    return true;
}

void CollectionsModel::serviceRemoved ( AbstractServiceProvider* provider )
{
    int index = indexOf ( provider->id() );
    if ( index == -1 ) return;

    beginRemoveRows ( QModelIndex(), index, index );
    m_collections.removeAt(index);
    endRemoveRows();
}

void CollectionsModel::serviceChanged ( AbstractServiceProvider* provider )
{
    Collection* p = dynamic_cast<Collection*>(provider);
    if (!p) return;
    int index = indexOf ( provider->id() );

    if ( index == -1 ) {

        // Find alphabetic position amoung profiles in the same category
        int row=0;
        for (;row<m_collections.size();++row)
            if (m_collections[row]->name().toLower() >= p->name().toLower())
                break;

        beginInsertRows (QModelIndex(),row,row );
        m_collections.insert(row, p);
        endInsertRows();

        if (m_focusid == p->id()) {
            emit itemMoved(createIndex(row,0,p));
            m_focusid = QString();
        }

        return;
    }

    bool indicateMovement = false;
    int row=-1;
    bool skip = false;
	
    for (row=0;row<m_collections.size();++row)
    {
        if (index == row) skip = true;
        if (index != row && m_collections[row]->name().toLower() >= p->name())
        {
            indicateMovement = true;
            break;
        }
    }
    if (skip) row--;

    if (row != -1) {
        beginRemoveRows ( QModelIndex(), index, index );
        m_collections.removeAt(index);
        endRemoveRows();
        beginInsertRows ( QModelIndex(),row,row );
        m_collections.insert(row, p);
        endInsertRows();
        if (indicateMovement) emit itemMoved(createIndex(row, 0, p));
    } else {
		QModelIndex mindex = createIndex(index,0);
        emit dataChanged ( mindex,mindex );
    }
}

int CollectionsModel::indexOf(const QVariant& data) {
    if (data.type()!=QVariant::String) return -1;
    const QString id = data.toString();
    for (int cats=0;cats<m_collections.size();++cats)
    {
        if (m_collections[cats]->id() == id) {
            return cats;
        }
    }
    return -1;
}

void CollectionsModel::stateTrackerChanged(AbstractStateTracker*) {}

