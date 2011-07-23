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
#include "configplugin.h"
#include "controller.h"

Q_EXPORT_PLUGIN2 ( libexecute, plugin )

plugin::plugin() {
    m_controller = new Controller ( this );
    connect(m_controller,SIGNAL(ledChanged(QString,QString,int)),SLOT(ledChanged(QString,QString,int)));
    connect(m_controller,SIGNAL(ledsCleared()),SLOT(ledsCleared()));

    _config ( this );
}

plugin::~plugin() {
    delete m_controller;
}

void plugin::clear() {}
void plugin::initialize() {
}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginSettingsHelper::setSetting ( name, value, init );
    if ( name == QLatin1String ( "server" ) ) {
        const QString server = value.toString();
        m_controller->connectToLeds ( server );
    }
}

void plugin::execute ( const QVariantMap& data, const QString& sessionid ) {
    Q_UNUSED ( sessionid );
	Controller::ledid lid =  m_controller->getPortPinFromString( DATA("channel") );
	if (lid.port == -1) return;
	 
    if ( ServiceID::isId(data, "udpio.value_absolut" ) ) {
        m_controller->setChannel ( lid,BOOLDATA("value") );
    } else if ( ServiceID::isId(data, "udpio.value_toggle" ) ) {
        m_controller->toogleChannel ( lid );
    } else if ( ServiceID::isId(data, "udpio.name" ) ) {
        m_controller->setChannelName ( lid, DATA("name") );
    }
}

bool plugin::condition ( const QVariantMap& data, const QString& sessionid )  {
    Q_UNUSED ( sessionid );
	Controller::ledid lid =  m_controller->getPortPinFromString( DATA("channel") );
	if (lid.port == -1)
		return false;
	
    if ( ServiceID::isId(data, "udpio.condition" ) ) {
        const bool v = m_controller->getChannel ( lid );
        if ( v != BOOLDATA("value") ) return false;
        return true;
    }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED ( data );
    Q_UNUSED ( collectionuid );
}

void plugin::unregister_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED(data);
    Q_UNUSED(collectionuid);
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;

    l.append(ServiceCreation::createModelReset(PLUGIN_ID, "udpio.names", "channel").getData());
	
    QMap<Controller::ledid, Controller::ledchannel>::iterator i = m_controller->m_leds.begin();
    for (;i!=m_controller->m_leds.end();++i) {
        {
            ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "udpio.names");
            sc.setData("channel", m_controller->getStringFromPortPin(i.key()));
            sc.setData("value", i.value().value);
            sc.setData("name", i.value().name);
            l.append(sc.getData());
        }
    }
    return l;
}

void plugin::ledsCleared() {
    m_server->property_changed(ServiceCreation::createModelReset(PLUGIN_ID, "udpio.names", "channel").getData());
}

void plugin::ledChanged(QString channel, QString name, int value) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "udpio.names");
    sc.setData("channel", channel);
    if (!name.isNull()) sc.setData("name", name);
    if (value != -1) sc.setData("value", value?true:false);
    m_server->property_changed(sc.getData());
}
