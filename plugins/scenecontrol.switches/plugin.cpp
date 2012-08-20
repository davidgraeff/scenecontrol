/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010-2012  David Gräff
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
#include <QDebug>
#include <QtPlugin>

#include "plugin.h"
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc<2) {
		qWarning()<<"No instanceid provided!";
		return 1;
	}
    plugin p(QLatin1String(PLUGIN_ID), QString::fromAscii(argv[1]));
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin(const QString& pluginid, const QString& instanceid) : AbstractPlugin(pluginid, instanceid) {
    connect(&m_cacheTimer, SIGNAL(timeout()), SLOT(cacheToDevice()));
    m_cacheTimer.setInterval(50);
    m_cacheTimer.setSingleShot(true);
}

plugin::~plugin() {
}

void plugin::clear() {
    m_ios.clear();
    m_cache.clear();
}

void plugin::clear(const QByteArray& plugin_) {
    // Remove all leds referenced by "plugin_id"
    QMutableMapIterator<QString, iochannel> i(m_ios);
    while (i.hasNext()) {
        i.next();
        if (i.value().plugin_id == plugin_) {
            SceneDocument sc = SceneDocument::createModelRemoveItem("switches");
            sc.setData("channel", i.value().channel);
            changeProperty(sc.getData());
            i.remove();
        }
    }
}

void plugin::initialize() {
    m_ios.clear();
    m_cache.clear();
}

bool plugin::isSwitchOn ( const QString& channel, bool value )  {
    return ( getSwitch ( channel ) == value );
}

void plugin::requestProperties(int sessionid) {
    changeProperty(SceneDocument::createModelReset("switches", "channel").getData(), sessionid);
    QMap<QString, plugin::iochannel>::iterator i = m_ios.begin();
    for (;i!=m_ios.end();++i) {
        const plugin::iochannel& str = i.value();
        SceneDocument sc = SceneDocument::createModelChangeItem("switches");
        sc.setData("channel", str.channel);
        sc.setData("value", str.value);
        sc.setData("name", str.name);
        changeProperty(sc.getData(), sessionid);
    }
}

bool plugin::getSwitch(const QString& channel) const
{
    if (!m_ios.contains(channel)) return false;
    return m_ios[channel].value;
}

void plugin::setSwitch ( const QString& channel, bool value )
{
    if (!m_ios.contains(channel)) return;
    iochannel& p = m_ios[channel];
    p.value = value;
    m_cache.insert(&p);

    if (!m_cacheTimer.isActive()) m_cacheTimer.start();
}

void plugin::setSwitchName ( const QString& channel, const QString& name )
{
    if ( !m_ios.contains(channel) ) return;
    if (name.isNull()) return;

    m_ios[channel].name = name;

    // change name property
    SceneDocument sc = SceneDocument::createModelChangeItem("switches");
    sc.setData("channel", channel);
    sc.setData("name", name);
    changeProperty(sc.getData());

    // save name to database
    QVariantMap settings;
    settings[QLatin1String("channel")] = channel;
    settings[QLatin1String("name")] = name;
    settings[QLatin1String("isname")] = true;
    changeConfig("channelname_" + channel.toUtf8(),settings);
}

void plugin::toggleSwitch ( const QString& channel )
{
    if (!m_ios.contains(channel)) return;
    setSwitch ( channel, !m_ios[channel].value );
}

int plugin::countSwitchs() {
    return m_ios.size();
}

QString plugin::getSwitchName(const QString& channel) {
    if (!m_ios.contains(channel)) return QString();
    return m_ios[channel].name;
}

// Get names from database
void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
    Q_UNUSED(configid);
    if (data.contains(QLatin1String("isname")) && data.contains(QLatin1String("channel")) && data.contains(QLatin1String("name"))) {
        const QByteArray channel = data.value(QLatin1String("channel")).toByteArray();
        const QString name = data.value(QLatin1String("name")).toString();
        if (m_ios.contains(channel)) {
            m_ios[channel].name = name;
        }
        m_namecache.insert(channel, name);
    }
}

// send data from set-cache (max 50ms old) to the respective plugin via interconnect communication
void plugin::cacheToDevice()
{
    QSet<iochannel*>::const_iterator it = m_cache.constBegin();
    QVariantMap datamap;
    for (;it != m_cache.constEnd(); ++it) {
		SceneDocument doc;
		doc.setMethod("setSwitch");
		doc.setData("channel",(*it)->channel);
		doc.setData("value",(*it)->value);
		sendDataToPlugin((*it)->plugin_id, doc.getData());
    }
    m_cache.clear();
}


void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(data);
}

void plugin::subpluginChange(const QByteArray& plugin_, const QString& channel, bool value, const QString& name) {
    // Assign data to structure
    bool before = m_ios.contains(channel);
    iochannel& io = m_ios[channel];
    io.plugin_id = plugin_;
    //p.moodlight = false;
    //p.fadeType = 1;
    io.channel = channel;
    io.value = value;
    if (!before)
        io.name = name.size()?name : m_namecache.value(io.channel);
    if (io.name.isEmpty()) {
        io.name = QLatin1String("Channel ") + QString::number(m_ios.size());
    }

    SceneDocument sc = SceneDocument::createModelChangeItem("switches");
    sc.setData("channel", io.channel);
    if (io.name.size()) sc.setData("name", io.name);
    if (io.value != -1) sc.setData("value", io.value);

    changeProperty(sc.getData());
}


