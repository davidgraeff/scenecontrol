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

#ifndef SERVICEPROVIDERCATEGORY_H
#define SERVICEPROVIDERCATEGORY_H
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSet>
#include "abstractserviceprovider.h"

class Category : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName);
    /**
     * Is this the categorie with all the alarms?
     */
    Q_PROPERTY(bool flagAlarm READ flagAlarm WRITE setFlagAlarm)
public:
    Category(QObject* parent=0);
    QString name() const {
        return m_name;
    }
    void setName(const QString& cmd) {
        m_name = cmd;
    }

    bool flagAlarm() const {
        return m_flagAlarm;
    }

    void setFlagAlarm( bool e ) {
        m_flagAlarm = e;
    }
    virtual ProvidedTypes providedtypes() { return NoneType; }

private:
    QString m_name;
    bool m_flagAlarm;

};

#endif // SERVICEPROVIDERPROFILE_H
