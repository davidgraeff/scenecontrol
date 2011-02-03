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

#ifndef EventDateTime_h
#define EventDateTime_h
#include <QTimer>
#include <shared/abstractserviceprovider.h>

class EventDateTime : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString datetime READ datetime WRITE setDatetime);
    Q_CLASSINFO("datetime_props", "datetime")
public:
    EventDateTime(QObject* parent = 0);
	virtual QString service_name(){return tr("Zeitereignis");}
	virtual QString service_desc(){return tr("Wird ausgelöst, sobald ein bestimmter Zeitpunkt eintritt");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Datum/Zeit");
        default:
            return QString();
        }
    }
    QString datetime() const;
    void setDatetime(const QString& value);
  private:
    QString m_datetime;
};

#endif // EventDateTime_h