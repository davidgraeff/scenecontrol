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
#include "shared/pluginservicehelper.h"
#include <QSet>
#include <shared/plugineventmap.h>

class plugin : public AbstractPlugin
{
    Q_OBJECT


public:
    plugin();
    virtual ~plugin();

    virtual void requestProperties(int sessionid);
    virtual void unregister_event ( const QString& eventid);
    virtual void dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data);
public Q_SLOTS:
    void eventmode ( const QString& eventid, const QString& mode, const QString& collectionuid);
    void modeChange(const QString& mode);
    bool isMode(const QString& mode);
private:
    QString m_mode;
    // mode -> (eventid, [collectionids])
    QMultiMap<QString, QPair<QString, QString> > m_collectionsOnMode;
};
