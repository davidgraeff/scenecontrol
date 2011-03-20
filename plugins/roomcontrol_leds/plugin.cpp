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
    _config ( this );
	connect(m_controller,SIGNAL(curtainChanged(int,int)),SLOT(curtainChanged(int,int)));
	connect(m_controller,SIGNAL(lednameChanged(int,QString)),SLOT(lednameChanged(int,QString)));
	connect(m_controller,SIGNAL(ledvalueChanged(int,int)),SLOT(ledvalueChanged(int,int)));
}

plugin::~plugin() {
    delete m_controller;
}

void plugin::initialize(){
}

void plugin::setSetting ( const QString& name, const QVariant& value, bool init ) {
    PluginHelper::setSetting ( name, value, init );
    if ( name == QLatin1String ( "serialport" ) ) {
        const QString device = value.toString();
        m_controller->connectToLeds ( device );
    }
}

void plugin::execute ( const QVariantMap& data ) {
    if ( IS_ID ( "ledvalue_relative" ) ) {
		m_controller->setChannelRelative ( INTDATA("channel"),INTDATA("value"),INTDATA("fade") );
    } else if ( IS_ID ( "ledvalue_absolut" ) ) {
		 m_controller->setChannel ( INTDATA("channel"),INTDATA("value"),INTDATA("fade") );
    } else if ( IS_ID ( "ledvalue_invers" ) ) {
		m_controller->inverseChannel ( INTDATA("channel"),INTDATA("fade") );
    } else if ( IS_ID ( "ledname" ) ) {
		m_controller->setChannelName ( INTDATA("channel"), DATA("name") );
    } else if ( IS_ID ( "curtain" ) ) {
		m_controller->setCurtain ( INTDATA("value") );
    }
}

bool plugin::condition ( const QVariantMap& data )  {
    if ( IS_ID ( "ledcondition" ) ) {
        const int v = m_controller->getChannel ( INTDATA("channel") );
        if ( v>INTDATA("upper") ) return false;
        if ( v<INTDATA("lower") ) return false;
        return true;
    } else if ( IS_ID ( "curtaincondition" ) ) {
        return ( INTDATA("value") == m_controller->getCurtain() );
    }
    return false;
}

void plugin::event_changed ( const QVariantMap& data ) {
    Q_UNUSED ( data );
}

QMap<QString, QVariantMap> plugin::properties() {
    QMap<QString, QVariantMap> l;
    return l;
}

void plugin::curtainChanged(int current, int max) {
	PROPERTY("curtainstate");
	data[QLatin1String("value")] = current;
	data[QLatin1String("max")] = max;
	m_server->property_changed(data);
}

void plugin::ledvalueChanged(int channel, int value) {
	PROPERTY("ledvalue");
	data[QLatin1String("channel")] = channel;
	data[QLatin1String("value")] = value;
	m_server->property_changed(data);
}

void plugin::lednameChanged(int channel, const QString& name) {
	PROPERTY("ledname");
	data[QLatin1String("channel")] = channel;
	data[QLatin1String("name")] = name;
	m_server->property_changed(data);
}
