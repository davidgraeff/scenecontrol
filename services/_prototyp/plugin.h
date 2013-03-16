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
#include "abstractplugin.h"

/**
 * Minimal example plugin. You always should inherit AbstractPlugin for getting
 * preimplemented funcatinaliy like plugin<-->server
 * communication as well as predefined overridable methods like initialize and clear.
 */
class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    /**
     * Do not use the constructor, instead override "initialize" for setting up your ressources.
     */
	virtual void initialize();
	virtual void clear();
    virtual void requestProperties();
	virtual void instanceConfiguration(const QVariantMap&);
    /**
    * Do not (only) clean up in the destructor. The server process may like to recycle the plugin
    * process. Use "clear" for cleaning up and "initialize" for setting up your ressources.
    */
    virtual ~plugin();
public Q_SLOTS:
    /**
     * Just implement your desired functionality here. The parameters are defined by the json schema.
	 * Example: A json schema defines a method "callthis" with one int parameter with the name "number".
	 * Then your method signature should be:
	 * void callthis(QByteArray componentid_, QByteArray instanceid_, int number);
	 * The first two parameters are necessary and give you information about the sender. The server e.g.
	 * has the componentid_==COMSERVERSTRING and en empty instanceid.
     */
private:
};
