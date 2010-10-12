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

#include "factory.h"
#include "qjson/parser.h"
#include "qjson/qobjecthelper.h"
#include "qjson/serializer.h"
#include "networkcontroller.h"
#include "RoomControlServer.h"
#include <QSettings>
#include <QDir>
#include <QDebug>

AbstractServiceProvider* Factory::generate ( const QVariantMap& args )
{
    AbstractServiceProvider* provider = 0;
    const QByteArray type = args.value(QLatin1String("type"), QString()).toString().toAscii();
    if (type.isEmpty()) {
        qWarning() << __FUNCTION__ << "detected json object without type" << args;
        return false;
    }

    if (type == Playlist::staticMetaObject.className()) {
        provider = new Playlist();
    } else if (type == ProfileCollection::staticMetaObject.className()) {
        ProfileCollection* t = new ProfileCollection();
        provider = t;
    } else if (type == CategoryProvider::staticMetaObject.className()) {
        provider = new CategoryProvider();
    } else if (type == ActorCurtain::staticMetaObject.className()) {
        provider = new ActorCurtain();
    } else if (type == ActorLed::staticMetaObject.className()) {
        provider = new ActorLed();
    } else if (type == ActorLedName::staticMetaObject.className()) {
        provider = new ActorLedName();
    } else if (type == ActorCinema::staticMetaObject.className()) {
        provider = new ActorCinema();
    } else if (type == ActorCinemaPosition::staticMetaObject.className()) {
        provider = new ActorCinemaPosition();
    } else if (type == ActorCinemaVolume::staticMetaObject.className()) {
        provider = new ActorCinemaVolume();
    } else if (type == ActorPin::staticMetaObject.className()) {
        provider = new ActorPin();
    } else if (type == ActorPinName::staticMetaObject.className()) {
        provider = new ActorPinName();
    } else if (type == ActorPlaylistCmd::staticMetaObject.className()) {
        provider = new ActorPlaylistCmd();
    } else if (type == ActorPlaylistPosition::staticMetaObject.className()) {
        provider = new ActorPlaylistPosition();
    } else if (type == ActorPlaylistVolume::staticMetaObject.className()) {
        provider = new ActorPlaylistVolume();
    } else if (type == ActorPlaylistTrack::staticMetaObject.className()) {
        provider = new ActorPlaylistTrack();
    } else if (type == ActorProfile::staticMetaObject.className()) {
        provider = new ActorProfile();
    } else if (type == ActorEvent::staticMetaObject.className()) {
        provider = new ActorEvent();
    } else if (type == ActorEventCmd::staticMetaObject.className()) {
        provider = new ActorEventCmd();
    } else if (type == ActorEventVolume::staticMetaObject.className()) {
        provider = new ActorEventVolume();
    } else if (type == ActorMute::staticMetaObject.className()) {
        provider = new ActorMute();
    } else if (type == ActorWOL::staticMetaObject.className()) {
        provider = new ActorWOL();
    } else if (type == ActorProjector::staticMetaObject.className()) {
        provider = new ActorProjector();
    } else if (type == EventDateTime::staticMetaObject.className()) {
        provider = new EventDateTime();
    } else if (type == EventPeriodic::staticMetaObject.className()) {
        provider = new EventPeriodic();
    } else if (type == EventRemoteKey::staticMetaObject.className()) {
        provider = new EventRemoteKey();
    } else if (type == EventSystem::staticMetaObject.className()) {
        provider = new EventSystem();
    } else if (type == ConditionPin::staticMetaObject.className()) {
        provider = new ConditionPin();
    } else if (type == ConditionLed::staticMetaObject.className()) {
        provider = new ConditionLed();
    } else if (type == ConditionCurtain::staticMetaObject.className()) {
        provider = new ConditionCurtain();
    } else if (type == ConditionMode::staticMetaObject.className()) {
        provider = new ConditionMode();
    } else if (type == ConditionEventState::staticMetaObject.className()) {
        provider = new ConditionEventState();
    } else if (type == ConditionMusicState::staticMetaObject.className()) {
        provider = new ConditionMusicState();
    } else if (type == ConditionTimespan::staticMetaObject.className()) {
        provider = new ConditionTimespan();
    } else
        return 0;

    // load json data into object
    QJson::QObjectHelper::qvariant2qobject(args, provider);
    return provider;
}

