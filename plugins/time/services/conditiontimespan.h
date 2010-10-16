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

#ifndef ConditionTimespan_h
#define ConditionTimespan_h
#include <QDateTime>
#include <shared/abstractserviceprovider.h>

class ConditionTimespan : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString lower READ lower WRITE setLower);
    Q_PROPERTY(QString upper READ upper WRITE setUpper);
public:
    ConditionTimespan(QObject* parent = 0);
    virtual ProvidedTypes providedtypes() { return ConditionType; }
    QString lower() const;
    void setLower(QString value);
    QString upper() const;
    void setUpper(QString value);
    QTime lowerTime() { return m_lower; }
    QTime upperTimer() { return m_upper; }
private:
    QTime m_lower;
    QTime m_upper;
};

#endif // ConditionTimespan_h
