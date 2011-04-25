#include <QCoreApplication>
#include <QtPlugin>
#include <QDebug>

#include "xbmcclient.h"
#include "xbmcactions.h"
#include "plugin.h"
#include "configplugin.h"

Q_EXPORT_PLUGIN2(libexecute, plugin)

plugin::plugin() : m_xbmcClient(0) {
    _config(this);
}

plugin::~plugin() {
    if (m_xbmcClient)
        m_xbmcClient->SendNOTIFICATION(QCoreApplication::applicationName().toLatin1().constData(),
                                       tr("Server wird beendet").toLatin1().constData(),
                                       ICON_NONE);
    delete m_xbmcClient;
}
void plugin::clear() {}
void plugin::initialize() {
}


void plugin::execute(const QVariantMap& data, const QString& sessionid) {
	if (!m_xbmcClient) return;
	if (ServiceID::isId(data,"xbmcfocus")) {
		setSetting(QLatin1String("server"), data["server"].toString());
	} else if (ServiceID::isId(data,"xbmcvolume")) {
		//todo
	} else if (ServiceID::isId(data,"xbmcposition")) {
		//todo
	} else if (ServiceID::isId(data,"xbmcmedia")) {
		//todo
	} else if (ServiceID::isId(data,"xbmccmd")) {
		setCommand(data["state"].toInt());
	}
}

bool plugin::condition(const QVariantMap& data, const QString& sessionid)  {
	Q_UNUSED(data);
	return false;
}

void plugin::event_changed(const QVariantMap& data, const QString& sessionid) {
	Q_UNUSED(data);  
}

void plugin::setSetting(const QString& name, const QVariant& value, bool init) {
    PluginSettingsHelper::setSetting(name, value, init);
    if (name == QLatin1String("server")) {
        delete m_xbmcClient;
        const QStringList data(value.toString().split(QLatin1Char(':')));
        if (data.size()!=2) return;
        m_xbmcClient = new CXBMCClient(data[0].toAscii(), data[1].toInt());
        m_xbmcClient->SendHELO(QCoreApplication::applicationName().toLatin1().constData(), ICON_NONE);
    }
}

QList<QVariantMap> plugin::properties(const QString& sessionid) {
Q_UNUSED(sessionid);
	QList<QVariantMap> l;
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
