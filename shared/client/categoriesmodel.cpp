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

#include "categoriesmodel.h"
#include <QDebug>
#include <shared/categorize/category.h>

CategoriesModel::CategoriesModel ( QObject* parent )
        : ClientModel ( parent )
{
}

CategoriesModel::~CategoriesModel()
{
    m_items.clear();
}

void CategoriesModel::clear()
{
    m_items.clear();
    reset();
}

int CategoriesModel::rowCount ( const QModelIndex & ) const
{
    return m_items.size();
}

QVariant CategoriesModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();

	if ( role == Qt::UserRole ) return m_items[index.row()]->id();
    else if ( role==Qt::DisplayRole || role==Qt::EditRole ) {
        return m_items[index.row()]->name();
    }

    return QVariant();
}


QVariant CategoriesModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal ) {
        if ( role == Qt::DisplayRole && section==0 ) {
            return tr ( "Category" );
        }
    }

    return QAbstractListModel::headerData ( section, orientation, role );
}

bool CategoriesModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() ) return false;

    if ( index.column() ==0 && role == Qt::EditRole ) {
        QString newname = value.toString().trimmed().replace ( QLatin1Char('\n'),QString() ).replace ( QLatin1Char('\t'),QString() );
        if ( newname.isEmpty() || newname == m_items[index.row() ]->name() ) return false;
        m_items[index.row() ]->setName(newname);
        emit changeService(m_items[index.row() ]);
        QModelIndex index = createIndex ( index.row(),0,0 );
        emit dataChanged ( index,index );
        return true;
    }
    return false;
}

bool CategoriesModel::removeRows ( int row, int count, const QModelIndex & )
{
    QString str;
    for ( int i=row+count-1;i>=row;--i ) {
		m_items[i]->setProperty("remove",1);
        emit changeService(m_items[i]);
    }
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
    return true;
}

Qt::ItemFlags CategoriesModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    return QAbstractListModel::flags ( index ) | Qt::ItemIsEditable;
}

void CategoriesModel::serviceChanged ( AbstractServiceProvider* service)
{
	Category* cat = qobject_cast<Category*>(service);
	if (!cat) return;
	int index = m_items.indexOf(cat);
	int newindex = 0;
	for (;newindex<m_items.size();++newindex) {
		if (m_items[newindex]->name()<=cat->name()) break;
	}
		
	if (index == -1) {
		beginInsertRows(QModelIndex(),newindex,newindex);
		m_items.insert(newindex, qobject_cast<Category*>(service));
		endInsertRows();
	} else if (newindex != index) {
		beginMoveRows(QModelIndex(),index,index,QModelIndex(),newindex);
		m_items.move(index, newindex);
		endMoveRows();
	} else {
        QModelIndex qindex = createIndex ( index,0,0 );
        emit dataChanged ( qindex,qindex );
	}
}

void CategoriesModel::serviceRemoved ( AbstractServiceProvider* service)
{
	Category* cat = qobject_cast<Category*>(service);
	if (!cat) return;
	int index = m_items.indexOf(cat);
	if (index == -1) return;
	beginRemoveRows(QModelIndex(),index,index);
	m_items.removeAt(index);
	endRemoveRows();
}

void CategoriesModel::stateTrackerChanged(AbstractStateTracker*) {
}
Category* CategoriesModel::get(int index) {
    return m_items.value(index);
}
int CategoriesModel::indexOf(const QVariant& data) {
	if (data.type()!=QVariant::String) return -1;
    const QString id = data.toString();
	
    for (int i=0;i<m_items.size();++i)
        if (m_items[i]->id() == id) return i;
    return -1;
}
