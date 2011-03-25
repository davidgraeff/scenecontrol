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
#include <QTimer>
#include <QMap>
#include <QSet>
#undef PLUGIN_ID
#define PLUGIN_ID "serverstate"
#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include <shared/abstractserver.h>

class ServerState : public QObject, public AbstractPlugin, public AbstractPlugin_services
{
    Q_OBJECT
    PLUGIN_MACRO
public:
	ServerState() {}
	virtual ~ServerState() {}
    virtual void clear();
    virtual void initialize();
    virtual bool condition(const QVariantMap& data);
    virtual void event_changed(const QVariantMap& data);
    virtual void execute(const QVariantMap& data);
    virtual QMap< QString, QVariantMap > properties(const QString& sessionid);
private:
	QMap<int, QSet<QString> > m_state_events; //state->set of uids
};
