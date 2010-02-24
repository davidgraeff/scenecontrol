/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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
#include "abstractevent.h"
#include <QVector>
#include <QVariantMap>
#include <QTimer>

class EventPeriodic : public AbstractEvent
{
    Q_OBJECT
    Q_PROPERTY(QString time READ time WRITE setTime);
    Q_PROPERTY(int days READ days WRITE setDays);
public:
    EventPeriodic(QObject* parent = 0);
    QString time() const;
    void setTime(QString value);
    int days() const;
    void setDays(int value);
    void setDay(int day, bool value) { m_days[day] = value; }
    QVector<bool> daysvector() const { return m_days; }
    virtual void changed() ;
    virtual void link() ;
  private:
    void nextWeekday(int &day);
    QTime m_time;
    QVector<bool> m_days;
};

#endif // EventDateTime_h
