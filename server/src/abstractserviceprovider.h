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

#ifndef ABSTRACTSERVICEPROVIDER_H
#define ABSTRACTSERVICEPROVIDER_H
#include <QObject>
#include <QStringList>

class AbstractServiceProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId);
    Q_PROPERTY(QString type READ type);
    Q_PROPERTY(QString parentid READ parentid WRITE setParentid)
public:
    AbstractServiceProvider(QObject* parent = 0);
    /** JSon has set new values to this object.
      * May be used by the events to do some initalisation.
      * Used to register this object by its parent. */
    virtual void newvalues() ;
    /** This provider is going to be removed. In the base
      * implementation it tells its parent about that */
    virtual void removeFromDisk() ;

    QString id() const;
    void setId(const QString& id);
    QString type() const;
    
    const QString& parentid() const {
        return m_parentid;
    }
    
    void setParentid( const QString& p ) {
        m_parentid = p;
    }
protected:
    QString m_id;
    QString m_parentid;
};

#endif // ABSTRACTSERVICEPROVIDER_H
