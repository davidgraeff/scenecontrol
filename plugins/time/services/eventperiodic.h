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

#ifndef EventPeriodic_h
#define EventPeriodic_h
#include <shared/abstractserviceprovider.h>

class EventPeriodic : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString time READ time WRITE setTime);
    Q_CLASSINFO("time_props", "time")
    Q_PROPERTY(int days READ days WRITE setDays);
    Q_CLASSINFO("days_props", "daybits")
public:
    EventPeriodic(QObject* parent = 0);
	virtual QString service_name(){return tr("Periodisches Ereignis");}
	virtual QString service_desc(){return tr("Wird ausgelöst, sobald ein periodischer Zeitpunkt erreicht ist. Eine Periode ist eine Woche.");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Zeit");
        case 1:
            return tr("Tage");
        default:
            return QString();
        }
    }
    QString time() const;
    void setTime(QString value);
    int days() const;
    void setDays(int value);
    bool m_days[7];
private:
    QString m_time;
};

#endif // EventDateTime_h
