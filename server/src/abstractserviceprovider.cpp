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
#include <QUuid>
#include "RoomControlServer.h"
#include "factory.h"
#include "networkcontroller.h"
#include "profile/serviceproviderprofile.h"

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

QString AbstractServiceProvider::type() const
{
    const QString type = QString::fromAscii ( metaObject()->className() );
    return type;
}

void AbstractServiceProvider::newvalues()
{
    // Announce oneself to the parent
    if ( m_parentid.isEmpty() ) return;
    ProfileCollection* parent = qobject_cast<ProfileCollection*> ( RoomControlServer::getFactory()->get ( m_parentid ) );
    if ( parent ) {
        parent->registerChild ( this );
	}
}

void AbstractServiceProvider::removeFromDisk()
{
    if ( !m_parentid.isEmpty() )
    {
        ProfileCollection* parent = qobject_cast<ProfileCollection*> ( RoomControlServer::getFactory()->get ( m_parentid ) );
        if ( parent )
            parent->childRemoved ( this );
    }
    RoomControlServer::getFactory()->objectRemoveFromDisk ( this );
}

