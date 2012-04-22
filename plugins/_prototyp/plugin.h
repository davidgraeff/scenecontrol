/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010-2012  David Gräff
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
#include "_sharedsrc/abstractplugin.h"

/**
 * Minimal example plugin. You always should inherit AbstractPlugin for getting
 * preimplemented funcatinaliy like plugin<-->plugin and plugin<-->server
 * communication as well as predefined overridable methods like initialize and clear.
 */
class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    /**
     * Do not use the constructor, instead use override "initialize" for setting up your ressources.
     */
    plugin(const QString& instanceid);
    /**
    * Do not (only) clean up in the destructor. The server process may like to recycle the plugin
    * process. Use "clear" for cleaning up and "initialize" for setting up your ressources.
    */
    virtual ~plugin();
    /**
     * Implement dataFromPlugin for receiving data from other plugins identified by plugin_id or 
     * from the server (plugin_id==COMSERVERSTRING)
     */
    void dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data);
private:
};
