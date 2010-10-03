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
#include "abstractserviceprovider.h"
#include "qjson/parser.h"
#include "qjson/qobjecthelper.h"
#include "qjson/serializer.h"
#include "networkcontroller.h"
#include "RoomControlServer.h"
#include <QSettings>
#include <QDir>
#include <QDebug>
#include "profile/serviceproviderprofile.h"
#include "actors/abstractactor.h"
#include "media/mediacontroller.h"
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
#include "remotecontrol/remotecontroller.h"
#include <QCoreApplication>
#include "actors/actormute.h"
#include "actors/actorwol.h"
#include "events/eventsystem.h"
#include "actors/actorprojector.h"
#include "profile/category.h"


Factory::Factory(QObject* parent) : QObject(parent)
{}

Factory::~Factory()
{
	emit systemGoingDown();
    qDeleteAll(m_providerList);
}

void Factory::load()
{
    MediaController* m = RoomControlServer::getMediaController();
    connect(this,SIGNAL(addedProvider(AbstractServiceProvider*)),
            m,SLOT(addedProvider(AbstractServiceProvider*)));
    connect(this,SIGNAL(removedProvider(AbstractServiceProvider*)),
            m,SLOT(removedProvider(AbstractServiceProvider*)));

    m_missingProfiles = new ProfileCollection(this);

    m_savedir = QFileInfo(QSettings().fileName()).absoluteDir();
    m_savedir.mkpath(m_savedir.absolutePath());
    m_savedir.mkdir(QLatin1String("json"));
    m_savedir.cd(QLatin1String("json"));

    // load
    QStringList files = m_savedir.entryList(QDir::Files);
    foreach (QString file, files)
    {
        QJson::Parser parser;
        bool ok = true;
        QFile f(m_savedir.absoluteFilePath(file));
        f.open(QIODevice::ReadOnly);
        if (!f.isOpen()) {
            qWarning() << "Couldn't open file" << file;
            continue;
        }

        QVariantMap result = parser.parse (&f, &ok).toMap();
        if (!ok) {
            qWarning() << "Not a json file" << file;
            continue;
        }

        AbstractServiceProvider* provider = generate(result);
        if (!provider) {
            qWarning() << "Json file content not recognised" << file;
            continue;
        }
        addServiceProvider(provider);
    }

    // Link providers together
    foreach (AbstractServiceProvider* p, m_providerList) {
        p->newvalues();
    }
    
    emit systemStarted();
}

AbstractServiceProvider* Factory::get(const QString& id)
{
    // Return a dummy if actual provider is not found
    // All unreferenced providers are collected this way
    return m_provider.value(id,m_missingProfiles);
}

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

void Factory::addServiceProvider(AbstractServiceProvider* provider)
{
    m_provider.insert(provider->id(), provider);
    m_providerList.append(provider);

    emit addedProvider(provider);
}

void Factory::objectRemoveFromDisk(AbstractServiceProvider* provider)
{
    m_provider.remove(provider->id());
    m_providerList.removeAll(provider);
    // set dynamic property remove to true
    provider->setProperty("remove",true);
    // propagate to all clients, so that those remove this provider, too.
    RoomControlServer::getNetworkController()->objectSync(provider);

    if ( !QFile::remove(providerFilename(provider)) )
        qWarning() << "Couldn't remove file" << providerFilename(provider);

    emit removedProvider(provider);
    provider->deleteLater();
}

void Factory::objectSaveToDisk(AbstractServiceProvider* provider)
{
    AbstractActor* asp = qobject_cast<AbstractActor*>(provider);
    if (asp && asp->iExecute()) {
        qWarning() << "Requested to save an immediately-to-execute action!" <<
        provider->type() << provider->id();
        return;
    }

    const QString path = providerFilename(provider);

    QFile file(path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        qWarning() << __FUNCTION__ << "Couldn't save" << provider->type() << "to " << path;
        return;
    }
    const QVariant variant = QJson::QObjectHelper::qobject2qvariant(provider);

    const QByteArray json = QJson::Serializer().serialize(variant);
    if (json.isNull())
    {
        qWarning() << __FUNCTION__ << "Saving failed" << provider->type() << "to " << path;
    } else {
        file.write(json);
    }
    file.close();
}

void Factory::examine(const QVariantMap& json)
{
    const QString id = json.value(QLatin1String("id")).toString();
    if (m_provider.contains(id))
    {
        AbstractServiceProvider* service = m_provider.value(id);
        if (json.contains(QLatin1String("remove")))
        {
            // remove object if the json request contains a variable with the name "remove"
            service->removeFromDisk();
        } else {
            // update object with values from the json request
            QJson::QObjectHelper::qvariant2qobject(json, service);
            service->newvalues();
            objectSaveToDisk(service);
            RoomControlServer::getNetworkController()->objectSync(service);
        }
    } else if (!json.contains(QLatin1String("remove")))
    {
        // Object with id not known. Create new object
        AbstractServiceProvider* provider = generate(json);
        if (!provider)
        {
            qWarning() << "command not supported" << id << json.value(QLatin1String("type"));
            return;
        }

        // Generate uinque ids amoung all existing ids
        QString newid;
        do {
            newid = QUuid::createUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
        } while (m_provider.contains(newid));
            provider->setId(newid);

        // check if it is an immediately to execute actor
        AbstractActor* asp = qobject_cast<AbstractActor*>(provider);
        if (asp && asp->iExecute()) {
            asp->execute();
            delete asp;
            return;
        }

        // add new provider to the controller
        provider->newvalues();
        objectSaveToDisk(provider);
        addServiceProvider(provider);
        RoomControlServer::getNetworkController()->objectSync(provider);
    }
}

QString Factory::providerFilename(AbstractServiceProvider* provider) {
    return m_savedir.absoluteFilePath(provider->type() + QLatin1String("-") + provider->id());
}

void Factory::backup() {
	QDir destdir = m_savedir.filePath(QDateTime::currentDateTime().toString());
    if (!destdir.exists() && !destdir.mkpath(destdir.absolutePath()) ) {
		qDebug() << "Backup failed" << destdir; 
		return;
	}
    QDir sourcedir = RoomControlServer::getFactory()->getSaveDir();
    QStringList files = sourcedir.entryList(QDir::Files|QDir::NoDotAndDotDot);
    qDebug() << "Backup" << files.size() << "files to" << destdir.path();
    foreach (QString file, files) {
		QFile::remove(destdir.absoluteFilePath(file));
        if (!QFile::copy(sourcedir.absoluteFilePath(file), destdir.absoluteFilePath(file))) {
			qDebug() << "Backup of"<<file<<"failed";
		}
    }
    RoomControlServer::getNetworkController()->backup_list_changed();
}

void Factory::backup_remove(const QString& id) {
	if (id.trimmed().isEmpty()) return;
	QDir destdir = m_savedir;
    if (!destdir.cd(id)) return;
	if (destdir.exists()) {
		QStringList files = destdir.entryList(QDir::Files);
		foreach (QString file, files) destdir.remove(file);
		destdir.rmdir(destdir.absolutePath());
	}
	RoomControlServer::getNetworkController()->backup_list_changed();
}

void Factory::backup_restore(const QString& id) {
	if (id.trimmed().isEmpty()) return;
	QDir sourcedir = m_savedir;
    if (!sourcedir.cd(id)) return;	
	
	//remove current
	QStringList files = m_savedir.entryList(QDir::Files);
	foreach (QString file, files) m_savedir.remove(file);
	
	//restore backup
	files = sourcedir.entryList(QDir::Files);
	foreach (QString file, files)
		QFile::copy(sourcedir.filePath(file),m_savedir.filePath(file));
}
QDir Factory::getSaveDir() const {
    return m_savedir;
}

QStringList Factory::backup_list() {
	return m_savedir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
}

