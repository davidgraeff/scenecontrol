#include "RoomControlClient.h"
#include <QDebug>
#include "profile/serviceproviderprofile.h"
#include <events/eventperiodic.h>
#include <events/eventdatetime.h>

RoomControlClient* RoomControlClient::instance = 0;

RoomControlClient::RoomControlClient () {}

RoomControlClient::~RoomControlClient()
{
    qDebug() << "Shutdown: Network server";
    delete networkController;
    qDebug() << "Shutdown: Factory";
    delete factory;
}

void RoomControlClient::init()
{
    // Network Controller must be initialized first
    qDebug() << "Start: Network controller";
    networkController = new NetworkController();
    // Factory must be initialized last
    qDebug() << "Start: Factory";
    factory = new Factory();
    // others
    m_ChannelsModel = new ChannelsModel(this);
    m_PinsModel = new PinsModel(this);
    m_PlaylistModel = new PlaylistModel(this);
    m_profilesModel = new ServiceProviderModel(tr("Profile"),this);
    m_profilesWithAlarmsModel = new ServiceProviderModel(tr("Alarm Profile"),this);
    connect(RoomControlClient::getFactory(),SIGNAL(addedProvider(AbstractServiceProvider*)),
            SLOT(addedProvider(AbstractServiceProvider*)));
    connect(RoomControlClient::getFactory(),SIGNAL(removedProvider(AbstractServiceProvider*)),
            SLOT(removedProvider(AbstractServiceProvider*)));

}

RoomControlClient* RoomControlClient::createInstance ()
{
    if ( instance == 0 )
    {
        instance = new RoomControlClient ();
        instance->init();
    }
    return instance;
}

void RoomControlClient::addedProvider(AbstractServiceProvider* p) {
    ProfileCollection* c = qobject_cast<ProfileCollection*>(p);
    if (!c) return;
    m_profilesModel->addedProvider(p);
}

void RoomControlClient::removedProvider(AbstractServiceProvider* p) {
    ProfileCollection* c = qobject_cast<ProfileCollection*>(p);
    if (!c) return;
    m_profilesModel->removedProvider(p);
    m_profilesWithAlarmsModel->removedProvider(p);
}
