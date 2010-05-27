#include "RoomControlClient.h"
#include <QDebug>
#include "profile/serviceproviderprofile.h"
#include <events/eventperiodic.h>
#include <events/eventdatetime.h>
#include <profile/category.h>

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
    m_profilesModel = new ProfilesModel(this);
    m_categoriesModel = new ServiceProviderModel(tr("Kategorien"), this);
    m_profilesWithAlarmsModel = new ServiceProviderModel(tr("Alarm Profile"),this);
    connect(RoomControlClient::getFactory(),SIGNAL(removedProvider(AbstractServiceProvider*)),
            m_profilesModel, SLOT(removedProvider(AbstractServiceProvider*)));
    connect(RoomControlClient::getFactory(),SIGNAL(removedProvider(AbstractServiceProvider*)),
            m_profilesWithAlarmsModel, SLOT(removedProvider(AbstractServiceProvider*)));

    connect(RoomControlClient::getFactory(),SIGNAL(addedProvider(AbstractServiceProvider*)),
            SLOT(addFilterCategories(AbstractServiceProvider*)));
    connect(RoomControlClient::getFactory(),SIGNAL(removedProvider(AbstractServiceProvider*)),
            m_categoriesModel, SLOT(removedProvider(AbstractServiceProvider*)));

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

void RoomControlClient::addFilterCategories(AbstractServiceProvider* p) {
	CategoryProvider* cat = qobject_cast<CategoryProvider*>(p);
	if (!cat) return;
	m_categoriesModel->addedProvider(p);
}
