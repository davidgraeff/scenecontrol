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
    connect ( m_controller,SIGNAL(nameChanged(QString,QString)),SLOT(nameChanged(QString,QString)));
    connect ( m_controller,SIGNAL(valueChanged(QString,int)),SLOT(valueChanged(QString,int)));
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
    if ( name == QLatin1String ( "autoconfig" ) ) {
        QStringList data = value.toString().split ( QLatin1Char ( ':' ) );
        if ( data.size() >=4 )
            m_controller->connectToIOs ( data[0].toInt(), data[1].toInt(), data[2], data[3] );
    }
}

void plugin::execute ( const QVariantMap& data ) {
    if ( IS_ID ( "iovalue_absolut" ) ) {
        m_controller->setPin ( DATA("channel"),BOOLDATA("value") );
    } else if ( IS_ID ( "iovalue_toogle" ) ) {
        m_controller->togglePin ( DATA("channel") );
    } else if ( IS_ID ( "ioname" ) ) {
        m_controller->setPinName ( DATA("channel"),DATA("name") );
    }
}

bool plugin::condition ( const QVariantMap& data )  {
    if ( IS_ID ( "iocondition" ) ) {
        return ( m_controller->getPin ( DATA("channel") ) == BOOLDATA("value") );
    }
    return false;
}

void plugin::event_changed ( const QVariantMap& data ) {
    Q_UNUSED ( data );
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    return l;
}

void plugin::nameChanged(QString channel, QString name) {
    PROPERTY("ionamechange");
    data[QLatin1String("channel")] = channel;
	data[QLatin1String("name")] = name;
    m_server->property_changed(data);
}

void plugin::valueChanged(QString channel, int value) {
    PROPERTY("iovaluechange");
    data[QLatin1String("channel")] = channel;
    data[QLatin1String("value")] = value;
    m_server->property_changed(data);
}
