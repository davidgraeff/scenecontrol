#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include <QDebug>
#include <QUrl>

#include "xbmcclient.h"
#include "xbmcactions.h"
#include "plugin.h"
#include "configplugin.h"

Q_EXPORT_PLUGIN2(libexecute, plugin)

plugin::plugin() : m_xbmcClient(0) {
    _config(this);
}

plugin::~plugin() {
    //delete m_base;
    if (m_xbmcClient)
        m_xbmcClient->SendNOTIFICATION(QCoreApplication::applicationName().toLatin1().constData(),
                                       tr("Server wird beendet").toLatin1().constData(),
                                       ICON_NONE);
    delete m_xbmcClient;
}

/*    m_plugin->setVolume(service<ActorCinemaVolume>()->volume(),service<ActorCinemaVolume>()->relative());
    m_plugin->setCommand(service<ActorCinema>()->cmd());
    m_plugin->setVolume(service<ActorCinemaPosition>()->value(),service<ActorCinemaPosition>()->relative());*/

void plugin::setSetting(const QString& name, const QVariant& value, bool init) {
    PluginHelper::setSetting(name, value);
    if (name == QLatin1String("server")) {
        delete m_xbmcClient;
        const QStringList data(value.toString().split(QLatin1Char(':')));
        if (data.size()!=2) return;
        m_xbmcClient = new CXBMCClient(data[0].toAscii(), data[1].toInt());
        m_xbmcClient->SendHELO(QCoreApplication::applicationName().toLatin1().constData(), ICON_NONE);
    }
}

QMap<QString, QVariantMap> plugin::properties() {
	QMap<QString, QVariantMap> l;
//     l.append(m_CinemaStateTracker);
//     l.append(m_CinemaPositionStateTracker);
//     l.append(m_CinemaVolumeStateTracker);
	return l;
}

void plugin::setCommand(int cmd)
{
	if (!m_xbmcClient) return;
    switch (cmd) {
    case 0:
        m_xbmcClient->SendACTION("play",ACTION_BUTTON);
        break;
    case 1:
        m_xbmcClient->SendACTION("pause",ACTION_BUTTON);
        break;
    case 2:
        m_xbmcClient->SendACTION("stop",ACTION_BUTTON);
        break;
    case 3:
        m_xbmcClient->SendACTION("next",ACTION_BUTTON);
        break;
    case 4:
        m_xbmcClient->SendACTION("prev",ACTION_BUTTON);
        break;
    case 5:
        m_xbmcClient->SendACTION("info",ACTION_BUTTON);
        break;
    case 6:
        m_xbmcClient->SendACTION("AspectRatio",ACTION_BUTTON);
        break;
    case 7:
        m_xbmcClient->SendACTION("NextSubtitle",ACTION_BUTTON);
        break;
    case 8:
        m_xbmcClient->SendACTION("AudioNextLanguage ",ACTION_BUTTON);
        break;
    case 9:
        m_xbmcClient->SendACTION("previousmenu",ACTION_BUTTON);
        break;
    case 10:
        m_xbmcClient->SendACTION("XBMC.ActivateWindow(Home)",ACTION_BUTTON);
        break;
    case 11:
        m_xbmcClient->SendACTION("select",ACTION_BUTTON);
        break;
    case 12:
        m_xbmcClient->SendACTION("down",ACTION_BUTTON);
        break;
    case 13:
        m_xbmcClient->SendACTION("up",ACTION_BUTTON);
        break;
    case 14:
        m_xbmcClient->SendACTION("left",ACTION_BUTTON);
        break;
    case 15:
        m_xbmcClient->SendACTION("right",ACTION_BUTTON);
        break;
    case 16:
        m_xbmcClient->SendACTION("close",ACTION_BUTTON);
        break;
    case 17:
        m_xbmcClient->SendACTION("ContextMenu",ACTION_BUTTON);
        break;
    case 18:
        m_xbmcClient->SendACTION("FastForward",ACTION_BUTTON);
        break;
    case 19:
        m_xbmcClient->SendACTION("Rewind",ACTION_BUTTON);
        break;
    default:
        break;
    };
}

void plugin::setPosition ( int pos, bool relative )
{
    Q_UNUSED(pos);
    Q_UNUSED(relative);
}

void plugin::setVolume ( int vol, bool relative )
{
    Q_UNUSED(vol);
    Q_UNUSED(relative);
}
