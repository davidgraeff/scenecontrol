/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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
#include "abstractserviceprovider.h"
#include "qjson/parser.h"
#include "qjson/qobjecthelper.h"
#include "qjson/serializer.h"
#include "networkcontroller.h"
#include "RoomControlClient.h"
#include <QSettings>
#include <QDir>
#include <QDebug>
#include "profile/serviceproviderprofile.h"
#include "actors/abstractactor.h"
#include "media/playlist.h"
#include "actors/actorcurtain.h"
#include "actors/actorled.h"
#include "actors/actorledname.h"
#include "actors/actorcinema.h"
#include "actors/actorcinemaposition.h"
#include "actors/actorcinemavolume.h"
#include "actors/actorpin.h"
#include "actors/actorpinname.h"
#include "actors/actorplaylistcmd.h"
#include "actors/actorplaylistposition.h"
#include "actors/actorplaylistvolume.h"
#include "actors/actorplaylisttrack.h"
#include "actors/actorprofile.h"
#include "actors/actoreventcmd.h"
#include "actors/actorevent.h"
#include "actors/actoreventvolume.h"
#include "events/eventdatetime.h"
#include "events/eventperiodic.h"
#include "events/eventremotekey.h"
#include "conditions/conditionpin.h"
#include "conditions/conditionled.h"
#include "conditions/conditioncurtain.h"
#include "conditions/conditionmode.h"
#include "conditions/conditioneventstate.h"
#include "conditions/conditionmusicstate.h"
#include "conditions/conditiontimespan.h"
#include "stateTracker/abstractstatetracker.h"
#include "stateTracker/channelnamestatetracker.h"
#include "stateTracker/channelvaluestatetracker.h"
#include "stateTracker/pinnamestatetracker.h"
#include "stateTracker/pinvaluestatetracker.h"
#include "stateTracker/curtainstatetracker.h"
#include "stateTracker/eventstatetracker.h"
#include "stateTracker/mediastatetracker.h"
#include "stateTracker/cinemastatetracker.h"
#include "stateTracker/programstatetracker.h"
#include "stateTracker/remotecontrolstatetracker.h"
#include "stateTracker/remotecontrolkeystatetracker.h"
#include "stateTracker/volumestatetracker.h"
#include "actors/actormute.h"
#include "events/eventsystem.h"
#include "actors/actorwol.h"
#include "stateTracker/pastatetracker.h"
#include "actors/actorprojector.h"


Factory::Factory(QObject* parent) : QObject(parent)
{
    connect(RoomControlClient::getNetworkController(),SIGNAL(disconnected()),
            SLOT(slotdisconnected()));
}

Factory::~Factory()
{
    qDeleteAll(m_providerList);
}

AbstractServiceProvider* Factory::get(const QString& id)
{
    return m_provider.value(id, 0);
}

AbstractServiceProvider* Factory::generate ( const QVariantMap& args )
{
    AbstractServiceProvider* provider = 0;
    AbstractStateTracker* tracker = 0;
    const QByteArray type = args.value(QLatin1String("type"), QString()).toString().toAscii();
    if (type.isEmpty()) {
        qWarning() << __FUNCTION__ << "detected json object without type" << args;
        return false;
    }

    if (type == Playlist::staticMetaObject.className()) {
        provider = new Playlist();
    } else if (type == ProfileCollection::staticMetaObject.className()) {
        provider = new ProfileCollection();
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
    } else if (type == ChannelNameStateTracker::staticMetaObject.className()) {
        tracker = new ChannelNameStateTracker();
    } else if (type == ChannelValueStateTracker::staticMetaObject.className()) {
        tracker = new ChannelValueStateTracker();
    } else if (type == PinNameStateTracker::staticMetaObject.className()) {
        tracker = new PinNameStateTracker();
    } else if (type == PinValueStateTracker::staticMetaObject.className()) {
        tracker = new PinValueStateTracker();
    } else if (type == CurtainStateTracker::staticMetaObject.className()) {
        tracker = new CurtainStateTracker();
    } else if (type == EventStateTracker::staticMetaObject.className()) {
        tracker = new EventStateTracker();
    } else if (type == MediaStateTracker::staticMetaObject.className()) {
        tracker = new MediaStateTracker();
    } else if (type == CinemaStateTracker::staticMetaObject.className()) {
        tracker = new CinemaStateTracker();
    } else if (type == ProgramStateTracker::staticMetaObject.className()) {
        tracker = new ProgramStateTracker();
    } else if (type == RemoteControlStateTracker::staticMetaObject.className()) {
        tracker = new RemoteControlStateTracker();
    } else if (type == RemoteControlKeyStateTracker::staticMetaObject.className()) {
        tracker = new RemoteControlKeyStateTracker();
    } else if (type == VolumeStateTracker::staticMetaObject.className()) {
        tracker = new VolumeStateTracker();
    } else if (type == PAStateTracker::staticMetaObject.className()) {
        tracker = new PAStateTracker();
    } else {
        qWarning() << "command not supported" << type;
    }
    if (tracker)
    {
        QJson::QObjectHelper::qvariant2qobject(args, tracker);
        emit stateChanged(tracker);
        if (tracker->property("managed").isNull())
            delete tracker;
    }
    return provider;
}

void Factory::addServiceProvider(AbstractServiceProvider* provider)
{
    NetworkController* nc = RoomControlClient::getNetworkController();
    connect(provider,SIGNAL(objectSync(QObject*)), nc,
            SLOT(objectSync(QObject*)));
    m_provider.insert(provider->id(), provider);
    m_providerList.append(provider);
    emit addedProvider(provider);
}

void Factory::requestRemoveProvider(AbstractServiceProvider* provider)
{
    provider->setProperty("remove",true);
    provider->sync();
}

void Factory::executeActor(AbstractActor* actor)
{
    actor->setiExecute(true);
    NetworkController* nc = RoomControlClient::getNetworkController();
    nc->objectSync(actor);
    delete actor;
}

void Factory::examine(const QVariantMap& json)
{
    const QString id = json.value(QLatin1String("id")).toString();
    if (id.size() && m_provider.contains(id))
    {
        AbstractServiceProvider* service = m_provider.value(id);
        if (json.contains(QLatin1String("remove"))) {
            // remove object if the json request contains a variable with the name "remove"
            // first remove object from the parent profile (if !profile)
            const QString parentid = service->parentid();
            ProfileCollection* profile = qobject_cast<ProfileCollection*>(get(parentid));
            if (profile)
                profile->removedChild(service);
            // remove object
            emit removedProvider(service);
            m_provider.remove(id);
            m_providerList.removeAll(service);
            delete service;
        } else {
            // update object with values from the json request
            QJson::QObjectHelper::qvariant2qobject(json, service);
            service->changed();
            service->link();
        }
    } else if (!json.contains(QLatin1String("remove")))
    {
        // Object with id not known. Create new object
        AbstractServiceProvider* service = generate(json);
        if (!service)
            return;
        QJson::QObjectHelper::qvariant2qobject(json, service);
        // init (e.g. set human readable toString)
        if (!m_sync) {
            service->changed();
            service->link();
        }
        addServiceProvider(service);
    }
}

void Factory::slotdisconnected()
{
    foreach (AbstractServiceProvider* p, m_providerList)
    {
        //emit removedProvider(p);
        delete p;
    }
    m_providerList.clear();
    m_provider.clear();
}

void Factory::syncComplete()
{
    m_sync = false;
    // Sync is completed, Link provider together and init them
    foreach (AbstractServiceProvider* p, m_providerList)
    {
        // Playlists will reset the model, set by the state tracker already
        // => don't emit their change signal
        if (p->metaObject()->className()!= Playlist::staticMetaObject.className())
            p->changed();
        p->link();
    }
    emit syncCompleted();
}

void Factory::syncStarted() {
    m_sync=true;
}
