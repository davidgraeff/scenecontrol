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

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    plugin();
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties(int sessionid);
    virtual void configChanged(const QByteArray& configid, const QVariantMap& data);
public Q_SLOTS:
    void play();
    void pause();
    void stop();
    void next();
    void prev();
    void info();
    void AspectRatio();
    void NextSubtitle();
    void AudioNextLanguage ();
    void previousmenu();
    void ActivateWindow();
    void select();
    void down();
    void up();
    void left();
    void right();
    void close();
    void ContextMenu();
    void FastForward();
    void Rewind();
private:
    virtual void dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data);
};
