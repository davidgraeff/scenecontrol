/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
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
#include <QSettings>
#include <qfile.h>
#include <QStringList>

#include "plugin.h"
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    plugin p;
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin() : m_listenSocket(0) {
    m_writesocket = new QUdpSocket(this);
    connect(&m_cacheTimer, SIGNAL(timeout()), SLOT(cacheToDevice()));
    m_cacheTimer.setInterval(50);
    m_cacheTimer.setSingleShot(true);
}

plugin::~plugin() {}

void plugin::connectToIOs(int portSend, int portListen, const QString& user, const QString& pwd) {
    m_sendPort = portSend;
    m_user = user;
    m_pwd = pwd;
    delete m_listenSocket;
    m_listenSocket = new QUdpSocket(this);
    connect(m_listenSocket,SIGNAL(readyRead()),SLOT(readyRead()));
    m_listenSocket->bind(QHostAddress::Broadcast,portListen);

    initialize();
}

void plugin::readyRead() {
    if (!m_listenSocket->hasPendingDatagrams()) {
        return;
    }
    QByteArray bytes;
    bytes.resize(1000);
    const int read = m_listenSocket->readDatagram(bytes.data(), 1000);
    if (read == -1) return;
    bytes.resize(read-2);
    const QList<QByteArray> cmd = bytes.split(':');
    if (cmd[0] != "NET-PwrCtrl") return;
    //cmd[2] = ip
    //cmd[6] .. ende -2
    // set new
    int pins = cmd.size()-2;
    unsigned char activated = ~cmd[cmd.size()-2].toInt();
    const QHostAddress host = QHostAddress(QString::fromAscii(cmd[2]));
    for ( int i=6;i<pins;++i )
    {
        const int pin = i-6;
        if (!(activated & (1 << pin))) continue;
        const QList<QByteArray> data = cmd[i].split(',');
        const QByteArray channelid = data[0];
        const int value = data[1].toInt();

//        const bool alreadyinside = m_mapChannelToHost.contains(channelid);
        m_mapChannelToHost[channelid] = QPair<QHostAddress,uint>(host,pin);

        if (m_ios[channelid].value == value)
            continue;

        m_ios[channelid].value = value;

	// send property
        ServiceData sc = ServiceData::createModelChangeItem("anel.io");
        sc.setData("channel", channelid);
        sc.setData("value", value);
        changeProperty(sc.getData(), -1);

	// send to switches plugin
        QVariantMap datamap;
        ServiceData::setMethod(datamap,"subpluginChange");
        ServiceData::setPluginid(datamap, PLUGIN_ID);
        datamap[QLatin1String("channel")] = channelid;
        datamap[QLatin1String("value")] = value;
        datamap[QLatin1String("name")] = channelid;
        sendDataToPlugin("switches", datamap);

        // update cache
        if (value)
            m_cache[host.toString()] |= (unsigned char)(1 << pin);
        else
            m_cache[host.toString()] &= (unsigned char)~(1 << pin);

    }
}

bool plugin::getSwitch( const QString& channel ) const
{
    if (!m_ios.contains(channel)) return false;
    return m_ios[channel].value;
}

void plugin::setSwitch ( const QString& channel, bool value )
{
    if (!m_mapChannelToHost.contains(channel)) return;
    QPair<QHostAddress,uint> p = m_mapChannelToHost[channel];

    if (value)
        m_cache[p.first.toString()] |= (unsigned char)(1 << p.second);
    else
        m_cache[p.first.toString()] &= (unsigned char)~(1 << p.second);

    if (!m_cacheTimer.isActive()) m_cacheTimer.start();

    ServiceData sc = ServiceData::createModelChangeItem("anel.io");
    sc.setData("channel", channel);
    sc.setData("value", value);
    changeProperty(sc.getData(), -1);
}

void plugin::toggleSwitch ( const QString& channel )
{
    if (!m_ios.contains(channel)) return;
    setSwitch ( channel, !m_ios[channel].value );
}

bool plugin::isSwitchOn( const QString& channel, bool value ) {
    return ( getSwitch ( channel ) == value );
}

int plugin::countSwitchs() {
    return m_ios.size();
}

void plugin::cacheToDevice()
{
    QMap<QString, unsigned char>::const_iterator it = m_cache.constBegin();
    for (;it != m_cache.constEnd(); ++it) {
        //SENDEN
        QByteArray str;
        str.append( "Sw" );
        str.append(it.value());
        str.append(m_user.toLatin1());
        str.append(m_pwd.toLatin1());
        m_writesocket->writeDatagram(str, QHostAddress( it.key()), m_sendPort);
    }
}

void plugin::initialize() {
    clear();
    QByteArray str("wer da?");
    str.append(0x0D);
    str.append(0x0A);
    m_writesocket->writeDatagram(str, QHostAddress::Broadcast, m_sendPort);
}

void plugin::clear() {
    m_ios.clear();
    m_ios.clear();

    QVariantMap datamap;
    ServiceData::setMethod(datamap,"clear");
    ServiceData::setPluginid(datamap, PLUGIN_ID);
    sendDataToPlugin("switches", datamap);
}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
    Q_UNUSED(configid);
    if (data.contains(QLatin1String("sendingport")) && data.contains(QLatin1String("listenport")) &&
            data.contains(QLatin1String("username")) && data.contains(QLatin1String("password")))
        connectToIOs ( data[QLatin1String("sendingport")].toInt(), data[QLatin1String("listenport")].toInt(),
                       data[QLatin1String("username")].toString(), data[QLatin1String("password")].toString() );
}

void plugin::requestProperties(int sessionid) {
    changeProperty(ServiceData::createModelReset("anel.io", "channel").getData(), sessionid);
    QMap<QString, plugin::iochannel>::iterator i = m_ios.begin();
    for (;i!=m_ios.end();++i) {
        const plugin::iochannel str = i.value();
        ServiceData sc = ServiceData::createModelChangeItem("anel.io");
        sc.setData("channel", i.key());
        sc.setData("value", str.value);
        changeProperty(sc.getData(), sessionid);
    }
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(data);
}

