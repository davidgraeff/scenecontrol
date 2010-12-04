#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "shared/server/executeservice.h"
#include "plugin.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

// actions that we have defined...
#define ACTION_NONE                    0
#define ACTION_MOVE_LEFT               1
#define ACTION_MOVE_RIGHT              2
#define ACTION_MOVE_UP                 3
#define ACTION_MOVE_DOWN               4
#define ACTION_PAGE_UP                 5
#define ACTION_PAGE_DOWN               6
#define ACTION_SELECT_ITEM             7
#define ACTION_HIGHLIGHT_ITEM          8
#define ACTION_PARENT_DIR              9
#define ACTION_PREVIOUS_MENU          10
#define ACTION_SHOW_INFO              11

#define ACTION_PAUSE                  12
#define ACTION_STOP                   13
#define ACTION_NEXT_ITEM              14
#define ACTION_PREV_ITEM              15
#define ACTION_FORWARD                16 // Can be used to specify specific action in a window, Playback control is handled in ACTION_PLAYER_*
#define ACTION_REWIND                 17 // Can be used to specify specific action in a window, Playback control is handled in ACTION_PLAYER_*

#define ACTION_SHOW_GUI               18 // toggle between GUI and movie or GUI and visualisation.
#define ACTION_ASPECT_RATIO           19 // toggle quick-access zoom modes. Can b used in videoFullScreen.zml window id=2005
#define ACTION_STEP_FORWARD           20 // seek +1% in the movie. Can b used in videoFullScreen.xml window id=2005
#define ACTION_STEP_BACK              21 // seek -1% in the movie. Can b used in videoFullScreen.xml window id=2005
#define ACTION_BIG_STEP_FORWARD       22 // seek +10% in the movie. Can b used in videoFullScreen.xml window id=2005
#define ACTION_BIG_STEP_BACK          23 // seek -10% in the movie. Can b used in videoFullScreen.xml window id=2005
#define ACTION_SHOW_OSD               24 // show/hide OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_SHOW_SUBTITLES         25 // turn subtitles on/off. Can b used in videoFullScreen.xml window id=2005
#define ACTION_NEXT_SUBTITLE          26 // switch to next subtitle of movie. Can b used in videoFullScreen.xml window id=2005
#define ACTION_SHOW_CODEC             27 // show information about file. Can b used in videoFullScreen.xml window id=2005 and in slideshow.xml window id=2007
#define ACTION_NEXT_PICTURE           28 // show next picture of slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_PREV_PICTURE           29 // show previous picture of slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_OUT               30 // zoom in picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_IN                31 // zoom out picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_TOGGLE_SOURCE_DEST     32 // used to toggle between source view and destination view. Can be used in myfiles.xml window id=3
#define ACTION_SHOW_PLAYLIST          33 // used to toggle between current view and playlist view. Can b used in all mymusic xml files
#define ACTION_QUEUE_ITEM             34 // used to queue a item to the playlist. Can b used in all mymusic xml files
#define ACTION_REMOVE_ITEM            35 // not used anymore
#define ACTION_SHOW_FULLSCREEN        36 // not used anymore
#define ACTION_ZOOM_LEVEL_NORMAL      37 // zoom 1x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_1           38 // zoom 2x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_2           39 // zoom 3x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_3           40 // zoom 4x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_4           41 // zoom 5x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_5           42 // zoom 6x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_6           43 // zoom 7x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_7           44 // zoom 8x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_8           45 // zoom 9x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_9           46 // zoom 10x picture during slideshow. Can b used in slideshow.xml window id=2007

#define ACTION_CALIBRATE_SWAP_ARROWS  47 // select next arrow. Can b used in: settingsScreenCalibration.xml windowid=11
#define ACTION_CALIBRATE_RESET        48 // reset calibration to defaults. Can b used in: settingsScreenCalibration.xml windowid=11/settingsUICalibration.xml windowid=10
#define ACTION_ANALOG_MOVE            49 // analog thumbstick move. Can b used in: slideshow.xml window id=2007/settingsScreenCalibration.xml windowid=11/settingsUICalibration.xml windowid=10
#define ACTION_ROTATE_PICTURE         50 // rotate current picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_CLOSE_DIALOG           51 // action for closing the dialog. Can b used in any dialog
#define ACTION_SUBTITLE_DELAY_MIN     52 // Decrease subtitle/movie Delay.  Can b used in videoFullScreen.xml window id=2005
#define ACTION_SUBTITLE_DELAY_PLUS    53 // Increase subtitle/movie Delay.  Can b used in videoFullScreen.xml window id=2005
#define ACTION_AUDIO_DELAY_MIN        54 // Increase avsync delay.  Can b used in videoFullScreen.xml window id=2005
#define ACTION_AUDIO_DELAY_PLUS       55 // Decrease avsync delay.  Can b used in videoFullScreen.xml window id=2005
#define ACTION_AUDIO_NEXT_LANGUAGE    56 // Select next language in movie.  Can b used in videoFullScreen.xml window id=2005
#define ACTION_CHANGE_RESOLUTION      57 // switch 2 next resolution. Can b used during screen calibration settingsScreenCalibration.xml windowid=11
#define ACTION_TOGGLE_WATCHED         58 // Toggle watched status (videos)

#define REMOTE_0                    58  // remote keys 0-9. are used by multiple windows
#define REMOTE_1                    59  // for example in videoFullScreen.xml window id=2005 you can
#define REMOTE_2                    60  // enter time (mmss) to jump to particular point in the movie
#define REMOTE_3                    61
#define REMOTE_4                    62  // with spincontrols you can enter 3digit number to quickly set
#define REMOTE_5                    63  // spincontrol to desired value
#define REMOTE_6                    64
#define REMOTE_7                    65
#define REMOTE_8                    66
#define REMOTE_9                    67

#define ACTION_PLAY                 68  // Unused at the moment
#define ACTION_OSD_SHOW_LEFT        69  // Move left in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_RIGHT       70  // Move right in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_UP          71  // Move up in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_DOWN        72  // Move down in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_SELECT      73  // toggle/select option in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_VALUE_PLUS  74  // increase value of current option in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_VALUE_MIN   75  // decrease value of current option in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_SMALL_STEP_BACK      76  // jumps a few seconds back during playback of movie. Can b used in videoFullScreen.xml window id=2005

#define ACTION_PLAYER_FORWARD        77  // FF in current file played. global action, can be used anywhere
#define ACTION_PLAYER_REWIND         78  // RW in current file played. global action, can be used anywhere
#define ACTION_PLAYER_PLAY           79  // Play current song. Unpauses song and sets playspeed to 1x. global action, can be used anywhere

#define ACTION_DELETE_ITEM          80  // delete current selected item. Can be used in myfiles.xml window id=3 and in myvideoTitle.xml window id=25
#define ACTION_COPY_ITEM            81  // copy current selected item. Can be used in myfiles.xml window id=3
#define ACTION_MOVE_ITEM            82  // move current selected item. Can be used in myfiles.xml window id=3
#define ACTION_SHOW_MPLAYER_OSD     83  // toggles mplayers OSD. Can be used in videofullscreen.xml window id=2005
#define ACTION_OSD_HIDESUBMENU      84  // removes an OSD sub menu. Can be used in videoOSD.xml window id=2901
#define ACTION_TAKE_SCREENSHOT      85  // take a screenshot
#define ACTION_POWERDOWN            86  // restart
#define ACTION_RENAME_ITEM          87  // rename item

#define ACTION_VOLUME_UP            88
#define ACTION_VOLUME_DOWN          89
#define ACTION_MUTE                 91

#include <QDebug>
#include <sys/socket.h>
#include "xbmcclient.h"
#include <QCoreApplication>
#include "statetracker/cinamestatetracker.h"
#include "services/actorcinema.h"
#include "services/actorcinemaposition.h"
#include "services/actorcinemavolume.h"
#include "statetracker/cinemapositionstatetracker.h"
#include "statetracker/volumestatetracker.h"
#include "services_server/actorcinemaServer.h"
#include "services_server/actorcinemavolumeServer.h"
#include "services_server/actorcinemapositionServer.h"

myPluginExecute::myPluginExecute() : ExecutePlugin() {
  m_base = new myPlugin();
    m_CinemaStateTracker = new CinemaStateTracker();
    m_CinemaPositionStateTracker = new CinemaPositionStateTracker();
    m_CinemaVolumeStateTracker = new CinemaVolumeStateTracker();
    m_xbmcClient = new CXBMCClient();
    m_xbmcClient->SendHELO(QCoreApplication::applicationName().toLatin1().constData(), ICON_NONE);
}

myPluginExecute::~myPluginExecute() {
  //delete m_base;
    m_xbmcClient->SendNOTIFICATION(QCoreApplication::applicationName().toLatin1().constData(),
                                   tr("Server wird beendet").toLatin1().constData(),
                                   ICON_NONE);
    delete m_xbmcClient;
    delete m_CinemaStateTracker;
    delete m_CinemaPositionStateTracker;
    delete m_CinemaVolumeStateTracker;
}

void myPluginExecute::refresh() {}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == ActorCinema::staticMetaObject.className()) {
        return new ActorCinemaServer((ActorCinema*)service, this);
    } else if (idb == ActorCinemaPosition::staticMetaObject.className()) {
        return new ActorCinemaPositionServer((ActorCinemaPosition*)service, this);
    } else if (idb == ActorCinemaVolume::staticMetaObject.className()) {
        return new ActorCinemaVolumeServer((ActorCinemaVolume*)service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    QList<AbstractStateTracker*> l;
    l.append(m_CinemaStateTracker);
    l.append(m_CinemaPositionStateTracker);
    l.append(m_CinemaVolumeStateTracker);
    return l;
}

void myPluginExecute::setCommand(int cmd)
{
    switch (cmd) {
      case ActorCinema::PlayCmd:
        m_xbmcClient->SendACTION("play",ACTION_BUTTON);
        break;
    case ActorCinema::PauseCmd:
        m_xbmcClient->SendACTION("pause",ACTION_BUTTON);
        break;
    case ActorCinema::StopCmd:
        m_xbmcClient->SendACTION("stop",ACTION_BUTTON);
        break;
    case ActorCinema::NextCmd:
        m_xbmcClient->SendACTION("next",ACTION_BUTTON);
        break;
    case ActorCinema::PrevCmd:
        m_xbmcClient->SendACTION("prev",ACTION_BUTTON);
        break;
    case ActorCinema::InfoCmd:
        m_xbmcClient->SendACTION("info",ACTION_BUTTON);
        break;
    case ActorCinema::AspectRatioCmd:
        m_xbmcClient->SendACTION("AspectRatio",ACTION_BUTTON);
        break;
    case ActorCinema::NextSubtitleCmd:
        m_xbmcClient->SendACTION("NextSubtitle",ACTION_BUTTON);
        break;
    case ActorCinema::NextLanguageCmd:
        m_xbmcClient->SendACTION("AudioNextLanguage ",ACTION_BUTTON);
        break;
    case ActorCinema::NavigationBackCmd:
        m_xbmcClient->SendACTION("previousmenu",ACTION_BUTTON);
        break;
    case ActorCinema::NavigationHomeCmd:
        m_xbmcClient->SendACTION("XBMC.ActivateWindow(Home)",ACTION_BUTTON);
        break;
    case ActorCinema::NavigationOKCmd:
        m_xbmcClient->SendACTION("select",ACTION_BUTTON);
        break;
    case ActorCinema::NavigationDownCmd:
        m_xbmcClient->SendACTION("down",ACTION_BUTTON);
        break;
    case ActorCinema::NavigationUpCmd:
        m_xbmcClient->SendACTION("up",ACTION_BUTTON);
        break;
    case ActorCinema::NavigationLeftCmd:
        m_xbmcClient->SendACTION("left",ACTION_BUTTON);
        break;
    case ActorCinema::NavigationRightCmd:
        m_xbmcClient->SendACTION("right",ACTION_BUTTON);
        break;
    case ActorCinema::NavigationCloseCmd:
        m_xbmcClient->SendACTION("close",ACTION_BUTTON);
        break;
    case ActorCinema::NavigationContextMenuCmd:
        m_xbmcClient->SendACTION("ContextMenu",ACTION_BUTTON);
        break;
    case ActorCinema::FastForwardCmd:
        m_xbmcClient->SendACTION("FastForward",ACTION_BUTTON);
        break;
    case ActorCinema::FastRewindCmd:
        m_xbmcClient->SendACTION("Rewind",ACTION_BUTTON);
        break;
    default:
        break;
    };
}

void myPluginExecute::setPosition ( int pos, bool relative )
{
  Q_UNUSED(pos);
  Q_UNUSED(relative);
}

void myPluginExecute::setVolume ( int vol, bool relative )
{
  Q_UNUSED(vol);
  Q_UNUSED(relative);
}