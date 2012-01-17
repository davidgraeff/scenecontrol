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

plugin::plugin() : m_events(QLatin1String("pin")) {
    m_controller = new Controller ( this );
    connect(m_controller,SIGNAL(ledChanged(QString,QString,int)),SLOT(ledChanged(QString,QString,int)));
    connect(m_controller,SIGNAL(ledsCleared()),SLOT(ledsCleared()));
    connect(m_controller,SIGNAL(watchpinChanged(unsigned char, unsigned char)),SLOT(watchpinChanged(unsigned char, unsigned char)));
}

plugin::~plugin() {
    delete m_controller;
}

void plugin::clear() {}
void plugin::initialize() {
}

void plugin::settingsChanged(const QVariantMap& data) {
    if (data.contains(QLatin1String("server")) && data.contains(QLatin1String("port"))) {
        m_sensors.resize(8);
        m_read = false;
        m_controller->connectToLeds ( data[QLatin1String("server")].toString(), data[QLatin1String("port")].toInt() );
    }
}

void plugin::execute ( const QVariantMap& data, int sessionid ) {
    Q_UNUSED ( sessionid );
    Q_UNUSED ( data );
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
    Q_UNUSED ( sessionid );
    Controller::ledid lid =  m_controller->getPortPinFromString( DATA("channel") );
    if (lid.port == -1)
        return false;

    if ( ServiceID::isMethod(data, "udpio.condition" ) ) {
        const bool v = m_controller->getChannel ( lid );
        if ( v != BOOLDATA("value") ) return false;
        return true;
    }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) {
    Q_UNUSED(sessionid);
    if (ServiceID::isMethod(data,"udpio.watchvalue")) {
        m_events.add(data, collectionuid);
    }
}

void plugin::unregister_event ( const QString& eventid, int sessionid ) {
    Q_UNUSED(sessionid);
    m_events.remove( eventid );
}

QList<QVariantMap> plugin::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;

    l.append(ServiceCreation::createModelReset(PLUGIN_ID, "udpio.sensor", "sensorid").getData());
    if (m_read) {
        ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "udpio.sensor");
        for (int id=0;id<m_sensors.size();++id) {
            sc.setData("sensorid", id);
            sc.setData("value", m_sensors[id]);
            l.append(sc.getData());
        }
    }
    return l;
}

void plugin::ledsCleared() {
    sendCmdToPlugin("switches", "CLEAR");
}

void plugin::ledChanged(QString channel, QString name, int value) {
    QVariantMap data;
    data[QLatin1String("channel")] = channel;
    data[QLatin1String("name")] = name;
    data[QLatin1String("value")] = value;
    sendDataToPlugin("switches", data);
}

void plugin::watchpinChanged(const unsigned char port, const unsigned char pinmask) {
    qDebug() << "WATCH PIN CHANGED" << port << pinmask;
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "udpio.sensor");
    for (int i=0;i<8;++i) {
        bool newvalue = (1 << i) & pinmask;
        if (!m_read || m_sensors[i] != newvalue) {
            m_sensors[i] = newvalue;
            sc.setData("sensorid", i);
            sc.setData("value", m_sensors[i]);
            m_serverPropertyController->pluginPropertyChanged(sc.getData());
            m_events.triggerEvent(i, m_serverCollectionController);
        }
    }
    m_read = true;
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QByteArray& data) {
    if (plugin_id != "switches")
        return;

    const QList<QByteArray> t = data.split('\t');
    // t[0]: channel
    // t[1]: value
    if (t.size() < 2) {
        qWarning() << pluginid() << "DataFromPlugin expected >= 2 data blocks";
        return;
    }

    Controller::ledid lid =  m_controller->getPortPinFromString( QString::fromAscii(t[0]));
    if (lid.port == -1) {
        qWarning() << pluginid() << "DataFromPlugin channel not found" << t[0];
        return;
    }

    m_controller->setChannel(lid, t[1].toInt());
}

