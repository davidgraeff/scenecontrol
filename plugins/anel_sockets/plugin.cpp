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
    connect ( m_controller,SIGNAL(dataChanged(QString,QString,int)),SLOT(dataChanged(QString,QString,int)));
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

void plugin::execute ( const QVariantMap& data, int sessionid ) {
	Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "iovalue_absolut" ) ) {
        m_controller->setPin ( DATA("channel"),BOOLDATA("value") );
    } else if ( ServiceID::isMethod(data, "iovalue_toogle" ) ) {
        m_controller->togglePin ( DATA("channel") );
    } else if ( ServiceID::isMethod(data, "ioname" ) ) {
        m_controller->setPinName ( DATA("channel"),DATA("name") );
    } else if ( ServiceID::isMethod(data, "reload" ) ) {
        m_controller->reinitialize();
    }
}

bool plugin::condition ( const QVariantMap& data, int sessionid )  {
	Q_UNUSED ( sessionid );
    if ( ServiceID::isMethod(data, "iocondition" ) ) {
        return ( m_controller->getPin ( DATA("channel") ) == BOOLDATA("value") );
    }
    return false;
}

void plugin::register_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) { 
	Q_UNUSED(sessionid);
	Q_UNUSED ( collectionuid );
    Q_UNUSED ( data );
}

void plugin::unregister_event ( const QVariantMap& data, const QString& collectionuid, int sessionid ) { 
	Q_UNUSED(sessionid);
	Q_UNUSED(data);
	Q_UNUSED(collectionuid);
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

    m_server->pluginPropertyChanged(sc.getData());
}
