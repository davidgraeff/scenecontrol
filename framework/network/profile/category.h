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

#ifndef SERVICEPROVIDERCATEGORY_H
#define SERVICEPROVIDERCATEGORY_H
#include "abstractserviceprovider.h"
#include <QSet>
#include <QTimer>
#include <QMap>

class Factory;
class AbstractActor;
class AbstractEvent;
class AbstractCondition;

class CategoryProvider : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName);
    Q_PROPERTY(bool flagAlarm READ flagAlarm WRITE setFlagAlarm)
public:
    CategoryProvider(QObject* parent = 0);
	virtual void changed() ;
	virtual void link();

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
private:
    QString m_name;
    bool m_flagAlarm;

};

#endif // SERVICEPROVIDERPROFILE_H
