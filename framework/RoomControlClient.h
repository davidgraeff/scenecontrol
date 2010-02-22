#ifndef RoomControlClient_H
#define RoomControlClient_H

#include <QtCore/QObject>
#include <QDBusConnection>
#include "network/networkcontroller.h"
#include "network/factory.h"
#include "models/channelsmodel.h"
#include "models/pinsmodel.h"
#include "models/playlistmodel.h"
#include "models/serviceprovidermodel.h"

/**
 * Used as singleton to access all other controllers.
 * Initiate controllers and frees them again.
 */
class RoomControlClient: public QObject
{
    Q_OBJECT
public:
    enum EnumMediaState
    {
        PlayState,
        PauseState,
        StopState
    };

    enum EnumMediaCmd
    {
        PlayCmd,
        StopCmd,
        PauseCmd,
        NextCmd,
        PrevCmd,
        NextPlaylistCmd,
        PrevPlaylistCmd,
        InfoCmd,
        AspectRatioCmd,
        NextSubtitleCmd,
        NextLanguageCmd,
        NavigationUpCmd,
        NavigationDownCmd,
        NavigationLeftCmd,
        NavigationRightCmd,
        NavigationBackCmd,
        NavigationOKCmd,
        NavigationHomeCmd,
        NavigationCloseCmd,
        NavigationContextMenuCmd,
        FastForwardCmd,
        FastRewindCmd
    };
    Q_ENUMS(EnumMediaCmd EnumMediaState);

    virtual ~RoomControlClient();
    static RoomControlClient* createInstance ();
    static inline RoomControlClient* getRoomControlClient ()
    {
        return instance;
    }

    static inline NetworkController* getNetworkController()
    {
        return instance->networkController;
    }

    static inline Factory* getFactory()
    {
        return instance->factory;
    }
    static inline ChannelsModel* getChannelsModel()
    {
        return instance->m_ChannelsModel;
    }
    static inline PinsModel* getPinsModel()
    {
        return instance->m_PinsModel;
    }
    static inline PlaylistModel* getPlaylistModel()
    {
        return instance->m_PlaylistModel;
    }
    static inline ServiceProviderModel* getProfilesModel()
    {
        return instance->m_profilesModel;
    }
    static inline ServiceProviderModel* getProfilesWithAlarmsModel()
    {
        return instance->m_profilesWithAlarmsModel;
    }
private:
    static RoomControlClient* instance;
    RoomControlClient ( const RoomControlClient& );
    void init();
    NetworkController* networkController;
    Factory* factory;
    ChannelsModel* m_ChannelsModel;
    PinsModel* m_PinsModel;
    PlaylistModel* m_PlaylistModel;
    ServiceProviderModel* m_profilesModel;
    ServiceProviderModel* m_profilesWithAlarmsModel;
private Q_SLOTS:
    void addedProvider(AbstractServiceProvider* p) ;
    void removedProvider(AbstractServiceProvider* p) ;
protected:
    RoomControlClient ();
};

#endif // RoomControlClient_H
