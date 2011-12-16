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
    connect(m_controller,SIGNAL(ledChanged(QString,QString,int)),SLOT(ledChanged(QString,QString,int)));
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

void plugin::execute ( const QVariantMap& data, int sessionid ) {
	Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "ledvalue_relative" ) ) {
        m_controller->setChannelRelative ( DATA("channel"),INTDATA("value"),INTDATA("fade") );
    } else if ( ServiceID::isMethod(data, "ledvalue_absolut" ) ) {
        m_controller->setChannel ( DATA("channel"),INTDATA("value"),INTDATA("fade") );
    } else if ( ServiceID::isMethod(data, "ledvalue_invers" ) ) {
        m_controller->inverseChannel ( DATA("channel"),INTDATA("fade") );
    } else if ( ServiceID::isMethod(data, "ledmoodlight" ) ) {
        m_controller->moodlight ( DATA("channel"),BOOLDATA("moodlight") );
    } else if ( ServiceID::isMethod(data, "ledname" ) ) {
        m_controller->setChannelName ( DATA("channel"), DATA("name") );
    } else if ( ServiceID::isMethod(data, "curtain" ) ) {
        m_controller->setCurtain ( INTDATA("value") );
    }
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
	Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "ledcondition" ) ) {
        const int v = m_controller->getChannel ( DATA("channel") );
        if ( v>INTDATA("upper") ) return false;
        if ( v<INTDATA("lower") ) return false;
        return true;
    } else if ( ServiceID::isMethod(data, "curtain_condition" ) ) {
        return ( INTDATA("value") == m_controller->getCurtain() );
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
}

QList<QVariantMap> plugin::properties(int sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;

	ledsCleared();
    QMap<QString, Controller::ledchannel>::iterator i = m_controller->m_leds.begin();
    for (;i!=m_controller->m_leds.end();++i) {
        {
            ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "roomcontrol.leds");
            sc.setData("channel", i.key());
            sc.setData("value", i.value().value);
            sc.setData("name", i.value().name);
            l.append(sc.getData());
        }
    }
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "curtain.state");
        sc.setData("value", m_controller->m_curtain_value);
        sc.setData("max", m_controller->m_curtain_max);
        l.append(sc.getData());
    }
    return l;
}

void plugin::curtainChanged(int current, int max) {
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "curtain.state");
    sc.setData("value", current);
    sc.setData("max", max);
    m_serverPropertyController->pluginPropertyChanged(sc.getData());
}

void plugin::ledsCleared() {
	m_serverPropertyController->pluginPropertyChanged(ServiceCreation::createModelReset(PLUGIN_ID, "roomcontrol.leds", "channel").getData());
}

void plugin::ledChanged(QString channel, QString name, int value) {
    ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "roomcontrol.leds");
    sc.setData("channel", channel);
    if (!name.isNull()) sc.setData("name", name);
    if (value != -1) sc.setData("value", value);
    m_serverPropertyController->pluginPropertyChanged(sc.getData());
}
