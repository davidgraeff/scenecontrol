/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once
#include <QObject>
#include <QStringList>
#include "shared/abstractplugin.h"
#include "shared/abstractserver_collectioncontroller.h"

#include "shared/abstractserver_propertycontroller.h"
#include "shared/pluginservicehelper.h"
#include "shared/abstractplugin_services.h"

class QextSerialPort;
class plugin : public QObject, public AbstractPlugin,  public AbstractPlugin_services
{
    Q_OBJECT


public:
    plugin();
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
    virtual void execute(const QVariantMap& data, );
    virtual bool condition(const QVariantMap& data, ) ;
    virtual void register_event ( const QVariantMap& data, const QString& collectionuid);
    virtual void unregister_event ( const QString& eventid);
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data);
private:
    QextSerialPort* m_serial;
    char m_buffer[4];
    void writeToDevice();
public slots:
    void readyRead();
};
