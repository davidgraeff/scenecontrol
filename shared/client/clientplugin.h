/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

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

#include <QObject>
#include <QStringList>
#include <QAbstractItemModel>
#include <QList>

class ModelStorage;
class ServiceStorage;
class AbstractServiceProvider;
class AbstractStateTracker;
class AbstractPlugin;

class ClientModel : public QAbstractListModel {
    Q_OBJECT
public:
    ClientModel(QObject* parent = 0) : QAbstractListModel(parent) {}
    virtual QString id() ;
    virtual int indexOf(const QVariant& data) = 0;
public Q_SLOTS:
    virtual void stateTrackerChanged(AbstractStateTracker*) = 0;
    virtual void serviceRemoved(AbstractServiceProvider*) = 0;
    virtual void serviceChanged(AbstractServiceProvider*) = 0;
    virtual void clear() = 0;
Q_SIGNALS:
    /*
     * The model want to hint to a special entry.
     * For example if the track in a playlist changed.
     */
    void autoFocusChanged(QModelIndex);
};

class ClientPlugin : public QObject
{
    Q_OBJECT
public:
    virtual void setStorages(ServiceStorage* servicestorage, ModelStorage* modelstorage);
    virtual void init() = 0;
    virtual AbstractPlugin* base() = 0;
};
Q_DECLARE_INTERFACE(ClientPlugin, "com.roomcontrol.ClientPlugin/1.0")

