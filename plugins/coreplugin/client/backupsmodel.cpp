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

#include "backupsmodel.h"
#include <QStringList>
#include <QDebug>
#include <services/backupAC.h>
#include <statetracker/backupST.h>

BackupsModel::BackupsModel ( QObject* parent )
        : ClientModel ( parent )
{
}

BackupsModel::~BackupsModel()
{
    clear();
}

void BackupsModel::clear()
{
    // Do not delete playlist objects, they are managed through the factory
    m_items.clear();
    reset();
}

int BackupsModel::rowCount ( const QModelIndex & ) const
{
    return m_items.size();
}

QVariant BackupsModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();

	if ( role==Qt::UserRole) return m_items.at(index.row()).id;
    if ( role==Qt::DisplayRole || role==Qt::EditRole ) {
        return m_items.at ( index.row() ).name;
    }

    return QVariant();
}


QVariant BackupsModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal ) {
        if ( role == Qt::DisplayRole && section==0 ) {
            return tr ( "Backups" );
        }
    }

    return QAbstractListModel::headerData ( section, orientation, role );
}

bool BackupsModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() ) return false;

    if ( index.column() ==0 && role == Qt::EditRole ) {
        QString newname = value.toString().trimmed().replace ( QLatin1Char('\n'),QString() ).replace ( QLatin1Char('\t'),QString() );
        if ( newname.isEmpty() || newname == m_items[index.row() ].name ) return false;
        m_items[index.row() ].name = newname;
        ActorBackup* p = new ActorBackup();
        p->setBackupid(m_items[index.row() ].id);
        p->setBackupname(m_items[index.row() ].name);
		p->setAction(ActorBackup::RenameBackup);
        emit executeService(p);
        QModelIndex index = createIndex ( index.row(),0,0 );
        emit dataChanged ( index,index );
        return true;
    }
    return false;
}

bool BackupsModel::removeRows ( int row, int count, const QModelIndex & )
{
    QString str;
    for ( int i=row+count-1;i>=row;--i ) {
        
        ActorBackup* p = new ActorBackup();
        p->setBackupid(m_items.at ( i ).id);
        p->setBackupname(m_items.at ( i ).name);
		p->setAction(ActorBackup::RemoveBackup);
        emit executeService(p);
    }
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
    return true;
}

Qt::ItemFlags BackupsModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    return QAbstractListModel::flags ( index ) | Qt::ItemIsEditable;
}

void BackupsModel::serviceChanged ( AbstractServiceProvider* )
{
}

void BackupsModel::serviceRemoved ( AbstractServiceProvider* )
{
}
void BackupsModel::stateTrackerChanged(AbstractStateTracker* tracker) {
	BackupStateTracker* t = qobject_cast<BackupStateTracker*>(tracker);
	if (!t) return;
	if (t->backupids().size() != t->backupnames().size()) {
		qWarning() << "Backups: id size != name size";
		return;
	}
	clear();
	for(int i=0; i<t->backupids().size(); ++i) {
		BackupItem b;
		b.id = t->backupids().at(i);
		b.name = t->backupnames().at(i);
		m_items.append(b);
	}
}

int BackupsModel::indexOf(const QVariant& data) {
    if (data.type()!=QVariant::String) return -1;
	const QString id = data.toString();
	for (int i=0;i<m_items.size();++i)
		if (m_items[i].id == id) return i;
    return -1;
}