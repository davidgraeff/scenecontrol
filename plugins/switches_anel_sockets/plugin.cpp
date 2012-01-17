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
    connect ( m_controller,SIGNAL(dataChanged(QString,QString,int)),SLOT(dataChanged(QString,QString,int)));
}

plugin::~plugin() {
    delete m_controller;
}

void plugin::clear() {}
void plugin::initialize() {

}

void plugin::settingsChanged(const QVariantMap& data) {
    if (data.contains(QLatin1String("sendingport")) && data.contains(QLatin1String("listenport")) &&
            data.contains(QLatin1String("username")) && data.contains(QLatin1String("password")))
        m_controller->connectToIOs ( data[QLatin1String("sendingport")].toInt(), data[QLatin1String("listenport")].toInt(),
				     data[QLatin1String("username")].toString(), data[QLatin1String("password")].toString() );
}

void plugin::execute ( const QVariantMap& data, int sessionid ) {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "iovalue_absolut" ) ) {
        m_controller->setChannel ( DATA("channel"),BOOLDATA("value") );
    } else if ( ServiceID::isMethod(data, "iovalue_toogle" ) ) {
        m_controller->toggleChannel ( DATA("channel") );
    } else if ( ServiceID::isMethod(data, "ioname" ) ) {
        m_controller->setChannelName ( DATA("channel"),DATA("name") );
    } else if ( ServiceID::isMethod(data, "reload" ) ) {
        m_controller->reinitialize();
    }
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
    Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "iocondition" ) ) {
        return ( m_controller->getChannel ( DATA("channel") ) == BOOLDATA("value") );
    }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) {
    Q_UNUSED(sessionid);
    Q_UNUSED ( collectionuid );
    Q_UNUSED ( data );
}

void plugin::unregister_event ( const QString& eventid, int sessionid ) {
    Q_UNUSED(sessionid);
    Q_UNUSED(eventid);
}

QList<QVariantMap> plugin::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    {
        l.append(ServiceCreation::createModelReset(PLUGIN_ID, "anel.io", "channel").getData());
        QMap<QString, Controller::iochannel>::iterator i = m_controller->m_ios.begin();
        for (;i!=m_controller->m_ios.end();++i) {
            const Controller::iochannel str = i.value();
            ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "anel.io");
            sc.setData("channel", i.key());
            sc.setData("value", str.value);
            sc.setData("name", str.name);
            l.append(sc.getData());
        }
    }
    return l;
}

void plugin::dataChanged(QString channel, QString name, int value) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "anel.io");
    sc.setData("channel", channel);
    if (!name.isNull()) sc.setData("name", name);
    if (value != -1) sc.setData("value", value);

    m_serverPropertyController->pluginPropertyChanged(sc.getData());
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

    Controller::ledid lid =  m_controller->getPortChannelFromString( QString::fromAscii(t[0]));
    if (lid.port == -1) {
        qWarning() << pluginid() << "DataFromPlugin channel not found" << t[0];
        return;
    }

    m_controller->setChannel(lid, t[1].toInt());
}

