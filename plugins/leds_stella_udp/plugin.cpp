/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <QDebug>
#include <QtPlugin>

#include "plugin.h"
#include "controller.h"

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    m_controller = new Controller ( this );
    connect(m_controller,SIGNAL(ledChanged(QString,QString,int)),SLOT(ledChanged(QString,QString,int)));
    connect(m_controller,SIGNAL(ledsCleared()),SLOT(ledsCleared()));
}

plugin::~plugin() {
    delete m_controller;
}

void plugin::clear() {}
void plugin::initialize() {
}

void plugin::settingsChanged(const QVariantMap& data) {
    if (data.contains(QLatin1String("server")) && data.contains(QLatin1String("port")))
       m_controller->connectToLeds ( data[QLatin1String("server")].toString(), data[QLatin1String("port")].toInt() );
}

void plugin::execute ( const QVariantMap& data, int sessionid ) {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "udpled.value_relative" ) ) {
        m_controller->setChannelRelative ( DATA("channel"),INTDATA("value"),INTDATA("fade") );
    } else if ( ServiceID::isMethod(data, "udpled.value_absolut" ) ) {
        m_controller->setChannel ( DATA("channel"),INTDATA("value"),INTDATA("fade") );
    } else if ( ServiceID::isMethod(data, "udpled.value_invers" ) ) {
        m_controller->inverseChannel ( DATA("channel"),INTDATA("fade") );
    } else if ( ServiceID::isMethod(data, "udpled.value_exp" ) ) {
        m_controller->setChannelExponential ( DATA("channel"),INTDATA("multiplicator") ,INTDATA("fade") );
    } else if ( ServiceID::isMethod(data, "udpled.moodlight" ) ) {
        m_controller->moodlight ( DATA("channel"),BOOLDATA("moodlight") );
    } else if ( ServiceID::isMethod(data, "udpled.name" ) ) {
        m_controller->setChannelName ( DATA("channel"), DATA("name") );
    }
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "udpled.condition" ) ) {
        const int v = m_controller->getChannel ( DATA("channel") );
        if ( v>INTDATA("upper") ) return false;
        if ( v<INTDATA("lower") ) return false;
        return true;
    }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) {
    Q_UNUSED(sessionid);
    Q_UNUSED ( data );
    Q_UNUSED ( collectionuid );
}

void plugin::unregister_event ( const QString& eventid, int sessionid ) {
    Q_UNUSED(sessionid);
    Q_UNUSED(eventid);
}

QList<QVariantMap> plugin::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;

    l.append(ServiceCreation::createModelReset(PLUGIN_ID, "udpled.names", "channel").getData());

    QMap<QString, Controller::ledchannel>::iterator i = m_controller->m_leds.begin();
    for (;i!=m_controller->m_leds.end();++i) {
        {
            ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "udpled.names");
            sc.setData("channel", i.key());
            sc.setData("value", i.value().value);
            sc.setData("name", i.value().name);
            l.append(sc.getData());
        }
    }
    return l;
}

void plugin::ledsCleared() {
  sendCmdToPlugin("leds", "CLEAR");
}

void plugin::ledChanged(QString channel, QString name, int value) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "udpled.names");
    sc.setData("channel", channel);
    if (!name.isNull()) sc.setData("name", name);
    if (value != -1) sc.setData("value", value);
    m_serverPropertyController->pluginPropertyChanged(sc.getData());
}
