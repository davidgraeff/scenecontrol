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
#include <QSet>
#include "abstractplugin_settings.h"
#include "abstractplugin.h"

class AbstractServer;
/**
 * Implements all settings related methods of the plugin interface.
 */
class PluginSettingsHelper : public AbstractPlugin_settings, public AbstractPlugin
{
public:
    virtual void setSetting(const QString& name, const QVariant& value, bool init = false);
    virtual void registerSetting(const char* name, const QVariant& value);
    virtual const QVariantMap getSettings() const;
protected:
    QVariantMap m_settings;
};
