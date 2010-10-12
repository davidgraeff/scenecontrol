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

#include "abstractserviceprovider.h"
#include "abstractplugin.h"
#include <QUuid>

AbstractServiceProvider::AbstractServiceProvider ( QObject* parent )
        : QObject ( parent )
{
    m_id = QUuid::createUuid().toString().remove ( QLatin1Char ( '{' ) ).remove ( QLatin1Char ( '}' ) );
}

QString AbstractServiceProvider::id() const
{
    return m_id;
}

void AbstractServiceProvider::setId ( const QString& id )
{
    m_id = id;
}

QByteArray AbstractServiceProvider::type() const
{
    return metaObject()->className();
}

void AbstractServiceProvider::setDelay(int cmd) {
    m_delay = cmd;
}
int AbstractServiceProvider::delay() const {
    return m_delay;
}

void AbstractServiceProvider::setPlugin(const QString& plugin) {
    m_pluginid = plugin;
}

QString AbstractServiceProvider::plugin() const {
    return m_pluginid;
}

