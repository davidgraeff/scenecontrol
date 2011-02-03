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

#include "pulseaudiosinkmodel.h"
#include <QSettings>
#include <qfileinfo.h>
#include <QPalette>
#include <QApplication>
#include <QUrl>
#include <QDebug>
#include <qmimedata.h>
#include <services/actorpulsesink.h>
#include <shared/abstractstatetracker.h>
#include <statetracker/pulsestatetracker.h>
#include <shared/client/servicestorage.h>

PulseAudioSinkModel::PulseAudioSinkModel (QObject* parent)
        : ClientModel(parent)
{
}

int PulseAudioSinkModel::rowCount ( const QModelIndex & ) const
{
    return m_items.size();
}

int PulseAudioSinkModel::columnCount ( const QModelIndex & ) const
{
    return 2;
}

QVariant PulseAudioSinkModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( role == Qt::DisplayRole && section==0 )
        {
            return tr ( "Sinkname" );
        }
        else  if ( role == Qt::DisplayRole && section==1 )
        {
            return tr ( "Volume" );
        }
    }

    return QAbstractListModel::headerData ( section, orientation, role );
}


bool PulseAudioSinkModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid()) return false;


    if ( index.column() == 1 && role == Qt::EditRole)
    {
        double v = value.toDouble();
        m_items[index.row()].volume = v;
        ActorPulseSink* service = new ActorPulseSink();
        service->setAssignment(ActorPulseSink::VolumeAbsolute);
        service->setSinkid(m_items.at(index.row()).name);
        service->setMute(ActorPulseSink::MuteNoChangeSink);
        service->setVolume(v);
        ServiceStorage::instance()->executeService(service);
        return true;
    } else if (index.column()==0 && role == Qt::CheckStateRole)
    {
        m_items[index.row()].mute = (value.toInt()==Qt::Unchecked);
        ActorPulseSink* service = new ActorPulseSink();
        service->setSinkid(m_items.at(index.row()).name);
        service->setAssignment(ActorPulseSink::NoVolumeSet);
        if (m_items[index.row()].mute)
            service->setMute(ActorPulseSink::MuteSink);
        else
            service->setMute(ActorPulseSink::UnmuteSink);
        ServiceStorage::instance()->executeService(service);

        return true;
    }

    return false;
}

Qt::ItemFlags PulseAudioSinkModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return QAbstractListModel::flags ( index );

    switch (index.column()) {
    case 0:
        return QAbstractListModel::flags ( index ) | Qt::ItemIsUserCheckable;
    case 1:
        return QAbstractListModel::flags ( index ) | Qt::ItemIsEditable;
    default:
        return QAbstractListModel::flags ( index );
    }
}

QVariant PulseAudioSinkModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();

    if ( role==Qt::DisplayRole || role == Qt::EditRole )
    {
        if ( index.column() == 0 )
            return m_items.at(index.row()).name;
        else if ( index.column() == 1 )
            return m_items.at(index.row()).volume;
    }
    else if (role ==Qt::ToolTipRole)
    {
        return m_items.at(index.row()).name;
    }
    else if ( index.column() == 0 && role==Qt::CheckStateRole )
    {
        return (m_items.at(index.row()).mute?Qt::Unchecked:Qt::Checked);
    }
    else if ( role==Qt::UserRole )
    {
        return m_items.at(index.row()).name;
    }
    return QVariant();
}

int PulseAudioSinkModel::indexOf(const QVariant& data) {
    if (data.type()!=QVariant::String) return -1;
    const QString id = data.toString();

    for (int i=0;i<m_items.size();++i)
        if (m_items[i].name == id) return i;
    return -1;
}

void PulseAudioSinkModel::serviceChanged ( AbstractServiceProvider*)
{
}

void PulseAudioSinkModel::serviceRemoved ( AbstractServiceProvider*)
{
}

void PulseAudioSinkModel::stateTrackerChanged(AbstractStateTracker* statetracker) {
    PulseStateTracker* t = qobject_cast<PulseStateTracker*>(statetracker);
    if (!t) return;
    // change data
    for (int i=0;i<m_items.size();++i) {
        if (m_items[i].name == t->sinkname()) {
            m_items[i].volume = t->volume();
            m_items[i].mute = t->mute();
            emit dataChanged(createIndex(i,0,0), createIndex(i,columnCount(),0));
            return;
        }
    }
    // new data
    beginInsertRows(QModelIndex(),m_items.count(),m_items.count());
    m_items.append(PulseAudioSinkItem(t->sinkname(),t->volume(),t->mute()));
    endInsertRows();
}

void PulseAudioSinkModel::clear() {
    m_items.clear();
    reset();
}
