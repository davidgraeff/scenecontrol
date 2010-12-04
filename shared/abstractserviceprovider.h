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
#include <QObject>
#include <QString>
#include <QByteArray>

class AbstractServiceProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId);
    Q_PROPERTY(QString parentid READ parentid WRITE setParentid);
    Q_PROPERTY(QByteArray type READ type);
    Q_PROPERTY(int delay READ delay WRITE setDelay);
	Q_ENUMS(ProvidedService);
public:
	// Types that this class objects can be
    enum ProvidedService {
        NoneService = 0x0,
        EventService = 0x1,
        ConditionService = 0x2,
        ActionService = 0x4
    };

    AbstractServiceProvider(QObject* parent = 0);
	virtual QString service_name() = 0;
	virtual QString service_desc() = 0;
	virtual QString translate(int propindex, int enumindex = -1);
    virtual QByteArray type() const;
    virtual AbstractServiceProvider::ProvidedService service();

    QString id() const;
    void setId(const QString& id);

    int delay() const ;
    void setDelay(int cmd) ;

	QString toString() ;
    void setString(const QString& name);
    
    QString parentid() const;
    void setParentid ( const QString& id );
protected:
    QString m_id;
    QString m_name;
    QString m_parentid;
private:
    int m_delay;
};
