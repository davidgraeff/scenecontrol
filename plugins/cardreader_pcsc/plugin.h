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
#include "shared/abstractserver.h"
#include "shared/pluginsettingshelper.h" 
#include "shared/abstractplugin_services.h"
#include <QSet>

class CardThread;
class plugin : public QObject, public PluginSettingsHelper, public AbstractPlugin_services
{
    Q_OBJECT
    PLUGIN_MACRO
    Q_INTERFACES(AbstractPlugin AbstractPlugin_settings AbstractPlugin_services)
public:
    plugin();
    virtual ~plugin();

    virtual void session_change(const QString& id, bool running) {Q_UNUSED(id);Q_UNUSED(running);}
    virtual void initialize();
    virtual void clear();
    virtual QMap<QString, QVariantMap> properties(const QString& sessionid);
    virtual void setSetting ( const QString& name, const QVariant& value, bool init = false );
    virtual void execute ( const QVariantMap& data );
    virtual bool condition ( const QVariantMap& data ) ;
    virtual void event_changed ( const QVariantMap& data );
private:
    CardThread* m_thread;
	QMap<QString, QSet<QString> > m_card_events; //atr->set of uids

private Q_SLOTS:
    void slotcardDetected ( const QString& atr, int state );
};

