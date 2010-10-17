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
#include <QString>
#include <QByteArray>

class AbstractServiceProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId);
    Q_PROPERTY(QString plugin READ plugin WRITE setPlugin);
    Q_PROPERTY(QString parentid READ parentid WRITE setParentid);
    Q_PROPERTY(ProvidedTypes usetypes READ usetypes WRITE setUseTypes);
    Q_PROPERTY(QByteArray type READ type);
    Q_PROPERTY(ProvidedTypes providedtypes READ providedtypes);
    // Delay until this actor get executed
    Q_PROPERTY(int delay READ delay WRITE setDelay);
public:
    enum ProvidedType {
        NoneType = 0x0,
        EventType = 0x1,
        ConditionType = 0x2,
        ActionType = 0x4
    };
    Q_DECLARE_FLAGS(ProvidedTypes, ProvidedType)

    AbstractServiceProvider(QObject* parent = 0);
    virtual ProvidedTypes providedtypes() = 0;
    int delay() const ;
    void setDelay(int cmd) ;

    QByteArray type() const;
    QString id() const;
    void setId(const QString& id);
    QString plugin() const ;
    void setPlugin(const QString& plugin) ;
    ProvidedTypes usetypes() {
        return m_usetypes;
    }
    void setUseTypes(ProvidedTypes t) {
        m_usetypes = t;
    }
    QString parentid() const
    {
        return m_parentid;
    }

    void setParentid ( const QString& id )
    {
        m_parentid = id;
    }
protected:
    QString m_id;
    QString m_pluginid;
    QString m_parentid;
private:
    int m_delay;
    ProvidedTypes m_usetypes;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractServiceProvider::ProvidedTypes)

#endif // ABSTRACTSERVICEPROVIDER_H
