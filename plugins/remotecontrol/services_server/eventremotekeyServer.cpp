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

#include "eventremotekeyServer.h"
#include "services/eventremotekey.h"
#include "server/plugin_server.h"

EventRemoteKeyServer::EventRemoteKeyServer(EventRemoteKey* base, myPluginExecute* plugin, QObject* parent) : ExecuteService(base, parent), m_plugin(plugin) {
    connect(&m_timer,SIGNAL(timeout()),SLOT(retrigger()));
    m_timer.setSingleShot(true);
}

void EventRemoteKeyServer::keySlot(QString,QString keyname,uint channel,int pressed)
{
    EventRemoteKey* base = service<EventRemoteKey>();
    if (base->channel()>=0 && channel != (uint)base->channel()) return;
    if (base->pressed() != pressed) {
        return;
    }
    if (base->key() != keyname) return;
    emit trigger();
    if (base->repeat()) {
        m_dorepeat=true;
        m_plugin->setRepeatingEvent(this);
        m_timer.start(base->repeatinit());
    }
}


void EventRemoteKeyServer::dataUpdate() {
    m_plugin->registerKeyEvent(this);
}

void EventRemoteKeyServer::retrigger() {
    EventRemoteKey* base = service<EventRemoteKey>();
    emit trigger();
    if (base->repeat() && m_dorepeat) m_timer.start(base->repeat());
}
void EventRemoteKeyServer::stopRepeat() {
    m_dorepeat=false;
    m_timer.stop();
}
void EventRemoteKeyServer::execute() {}

void EventRemoteKeyServer::nameUpdate() {
	EventRemoteKey* base = service<EventRemoteKey>();
	
	base->setString(tr("Fernbedienungstaste %1 %2").arg(base->key()).arg(
		(base->pressed()?tr("gedrückt"):tr("losgelassen"))));
}
