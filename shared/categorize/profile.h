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

#pragma once

#include <QSet>
#include <QTimer>
#include <QMap>
#include <QStringList>
#include "../abstractserviceprovider.h"

class Collection : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY ( QString name READ name WRITE setName );
    Q_PROPERTY ( bool enabled READ enabled WRITE setEnabled )
public:
    Collection(QObject* parent = 0);
    virtual QString service_name() {
        return tr("Profile");
    }
    virtual QString service_desc() {
        return tr("Services müssen Profilen zugeordnet sein.");
    }
    QString name() const;
    void setName ( const QString& cmd );
    bool enabled() const;
    void setEnabled ( bool e );
private:
    QString m_name;
    bool m_enabled;
};
