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
    connect(m_controller,SIGNAL(curtainChanged(int,int)),SLOT(curtainChanged(int,int)));
    connect(m_controller,SIGNAL(lednameChanged(int,QString)),SLOT(lednameChanged(int,QString)));
    connect(m_controller,SIGNAL(ledvalueChanged(int,int)),SLOT(ledvalueChanged(int,int)));
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
    if ( name == QLatin1String ( "serialport" ) ) {
        const QString device = value.toString();
        m_controller->connectToLeds ( device );
    }
}

void plugin::execute ( const QVariantMap& data ) {
    if ( ServiceID::isId(data, "ledvalue_relative" ) ) {
        m_controller->setChannelRelative ( INTDATA("channel"),INTDATA("value"),INTDATA("fade") );
    } else if ( ServiceID::isId(data, "ledvalue_absolut" ) ) {
        m_controller->setChannel ( INTDATA("channel"),INTDATA("value"),INTDATA("fade") );
    } else if ( ServiceID::isId(data, "ledvalue_invers" ) ) {
        m_controller->inverseChannel ( INTDATA("channel"),INTDATA("fade") );
    } else if ( ServiceID::isId(data, "ledname" ) ) {
        m_controller->setChannelName ( INTDATA("channel"), DATA("name") );
    } else if ( ServiceID::isId(data, "curtain" ) ) {
        m_controller->setCurtain ( INTDATA("value") );
    }
}

bool plugin::condition ( const QVariantMap& data )  {
    if ( ServiceID::isId(data, "ledcondition" ) ) {
        const int v = m_controller->getChannel ( INTDATA("channel") );
        if ( v>INTDATA("upper") ) return false;
        if ( v<INTDATA("lower") ) return false;
        return true;
    } else if ( ServiceID::isId(data, "curtaincondition" ) ) {
        return ( INTDATA("value") == m_controller->getCurtain() );
    }
    return false;
}

void plugin::event_changed ( const QVariantMap& data ) {
    Q_UNUSED ( data );
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;

	l.append(ServiceCreation::createModelReset(PLUGIN_ID, "roomcontrol.led.value").getData());
	l.append(ServiceCreation::createModelReset(PLUGIN_ID, "roomcontrol.led.name").getData());
    QMap<int, Controller::ledchannel>::iterator i = m_controller->m_leds.begin();
    for (;i!=m_controller->m_leds.end();++i) {
        {
            ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "roomcontrol.led.value");
            sc.setData("channel", i.key());
            sc.setData("value", i.value().value);
            l.append(sc.getData());
        }
        {
            ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "roomcontrol.led.name");
            sc.setData("channel", i.key());
            sc.setData("name", i.value().name);
            l.append(sc.getData());
        }
    }
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "roomcontrol.curtain.state");
        sc.setData("value", m_controller->m_curtain_value);
        sc.setData("max", m_controller->m_curtain_max);
        l.append(sc.getData());
    }
    return l;
}

void plugin::curtainChanged(int current, int max) {
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "roomcontrol.curtain.state");
    sc.setData("value", current);
    sc.setData("max", max);
    m_server->property_changed(sc.getData());
}

void plugin::ledvalueChanged(int channel, int value) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "roomcontrol.led.value");
    sc.setData("channel", channel);
    sc.setData("value", value);
    m_server->property_changed(sc.getData());
}

void plugin::lednameChanged(int channel, const QString& name) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "roomcontrol.led.name");
    sc.setData("channel", channel);
    sc.setData("name", name);
    m_server->property_changed(sc.getData());
}

void plugin::ledsCleared() {
	m_server->property_changed(ServiceCreation::createModelReset(PLUGIN_ID, "roomcontrol.led.value").getData());
	m_server->property_changed(ServiceCreation::createModelReset(PLUGIN_ID, "roomcontrol.led.name").getData());
}
