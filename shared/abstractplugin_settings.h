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
#include <QVariantMap>
#include <QString>

class AbstractPlugin_settings
{
public:
    /**
     * Called by server process if a setting is changed. Use the plugin helper file
     * and call setSettings to save settings permanantly.
     * \param name Name of the setting
     * \param value Value of the setting
     */
    virtual void setSetting(const QString& name, const QVariant& value, bool init = false) = 0;
    /**
     * Called by an autogenerated file from cmake build process and registers/sets initial values
     * for settings. Use the plugin helper file and its implementation of registerSettings
     * to (1) use environment variables, (2) load saved settings or use the inital value.
     * \param name Name of the setting
     * \param value Value of the setting
     */
    virtual void registerSetting(const char* name, const QVariant& value) = 0;
    /**
     * Called by the server if a client requested settings of this plugin.
     */
    virtual const QVariantMap getSettings() const = 0;
};
Q_DECLARE_INTERFACE(AbstractPlugin_settings, "com.roomcontrol.PluginSettings/2.0")