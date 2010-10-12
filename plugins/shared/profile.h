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

#ifndef SERVICEPROVIDERPROFILE_H
#define SERVICEPROVIDERPROFILE_H
#include "abstractserviceprovider.h"
#include <QSet>
#include <QTimer>
#include <QMap>
#include <QStringList>

class Collection : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY ( QString name READ name WRITE setName );
    Q_PROPERTY ( bool enabled READ enabled WRITE setEnabled )
public:
    Collection(QObject* parent = 0);
    QString name() const
    {
        return m_name;
    }
    void setName ( const QString& cmd )
    {
        m_name = cmd;
    }

    bool enabled() const
    {
        return m_enabled;
    }

    void setEnabled ( bool e )
    {
        m_enabled = e;
    }


    virtual ProvidedTypes providedtypes() { return NoneType; }
private:
    QString m_name;
    bool m_enabled;
};

#endif // SERVICEPROVIDERPROFILE_H
