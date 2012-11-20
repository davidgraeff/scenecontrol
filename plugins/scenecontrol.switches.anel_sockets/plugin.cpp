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
 *    Protocol:
 *    * Alle schalten: “Sw” + Steckdosen + User + Passwort ; Steckdosen: binär, MSB=Steckdose 8, LSB= Steckdose 1
 *    * Abfrage: „wer da?“ + CrLf //CrLf=(0x0D)(0x0A) als Broadcast 255.255.255.255
 *    * Antwort: NET-PwrCtrl:(Name):(I.P):(M.A.S.K):(G.a.t.e.w.a.y):(M.A.C):Name1,(1 oder 0):....:Seg_Dis:PORT(0x0D)(0x0A) ; Seg_Dis: binär dosen gesperrt
 */
#include <QDebug>
#include <QSettings>
#include <qfile.h>
#include <QStringList>

#include "plugin.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    if (argc<2) {
		qWarning()<<"No instanceid provided!";
		return 1;
	}
    plugin p(QLatin1String(PLUGIN_ID), QString::fromAscii(argv[1]));
    if(!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin(const QString& pluginid, const QString& instanceid) : AbstractPlugin(pluginid, instanceid), m_listenSocket(0)
{
	m_sendPort = 0;
    m_writesocket = new QUdpSocket(this);
    connect(&m_cacheTimer, SIGNAL(timeout()), SLOT(cacheToDevice()));
    m_cacheTimer.setInterval(50);
    m_cacheTimer.setSingleShot(true);
}

plugin::~plugin() {}

void plugin::connectToIOs(int portSend, int portListen, const QString &user, const QString &pwd)
{
    m_sendPort = portSend;
    m_user = user;
    m_pwd = pwd;
    delete m_listenSocket;
    m_listenSocket = new QUdpSocket(this);
    connect(m_listenSocket, SIGNAL(readyRead()), SLOT(readyRead()));
    if (!m_listenSocket->bind(QHostAddress::Broadcast, portListen)) {
		qWarning()<<"Bind failed" << portListen;
    } else 
		initialize();
}

void plugin::readyRead()
{
    if(!m_listenSocket->hasPendingDatagrams()) {
        return;
    }
    QByteArray bytes;
    bytes.resize(1000);
    const int read = m_listenSocket->readDatagram(bytes.data(), 1000);
    if(read == -1) return;
    bytes.resize(read - 2); // remove (0x0D)(0x0A)
    QList<QByteArray> input = bytes.split(':');
    if(input[0] != "NET-PwrCtrl") return;
    // remove identifier and Name
    input.removeFirst();
    input.removeFirst();
	// remove PORT
	input.removeLast();
	// remove Seg_Dis
	bool ok;
	unsigned char Seg_Dis = ~(input.takeLast().toInt(&ok));
	if(!ok || input.isEmpty())
        return;
    // cmd 2: ip
    const QHostAddress host = QHostAddress(QString::fromAscii(input.first()));
    // cmd 2..6: remove ip, mask, gateway, mac
    input.removeFirst();
    input.removeFirst();
    input.removeFirst();
    input.removeFirst();
    if(input.isEmpty())
        return;
    const unsigned int pins = input.size();
    // set new
    for(unsigned int pincounter = 0; pincounter < pins; ++pincounter) {
        // if socket is not active, ignore it
        if(!(Seg_Dis & (1 << pincounter))) continue;
        const QList<QByteArray> data = input.at(pincounter).split(',');
        const QString channelid = QString::fromUtf8(data[0]);
        const bool value = data[1].toInt();
        m_mapChannelToHost[channelid] = QPair<QHostAddress, uint>(host, pincounter);

        if(m_ios.contains(channelid) && m_ios[channelid].value == value)
            continue;

        m_ios[channelid].value = value;

        // send property
        SceneDocument sc = SceneDocument::createModelChangeItem("switches.anel_sockets");
        sc.setData("channel", channelid);
        sc.setData("value", value);
        changeProperty(sc.getData(), -1);

        // send to scenecontrol.switches plugin
		SceneDocument doc;
		doc.setMethod("subpluginChange");
		doc.setComponentID(m_pluginid);
		doc.setInstanceID(m_instanceid);
		doc.setData("channel",channelid);
		doc.setData("value",value);
		doc.setData("name",channelid);
		callRemoteComponentMethod("scenecontrol.switchesnull", doc.getData());
	
        // update cache
        if(value)
            m_cache[host.toString()] |= (unsigned char)(1 << pincounter);
        else
            m_cache[host.toString()] &= (unsigned char)~(1 << pincounter);

    }
}

bool plugin::getSwitch(const QString &channel) const
{
    if(!m_ios.contains(channel)) return false;
    return m_ios[channel].value;
}

void plugin::setSwitch(const QString &channel, bool value)
{
    if(!m_mapChannelToHost.contains(channel)) return;
    QPair<QHostAddress, uint> p = m_mapChannelToHost[channel];

    if(value)
        m_cache[p.first.toString()] |= (unsigned char)(1 << p.second);
    else
        m_cache[p.first.toString()] &= (unsigned char)~(1 << p.second);

    if(!m_cacheTimer.isActive()) m_cacheTimer.start();
}

void plugin::toggleSwitch(const QString &channel)
{
    if(!m_ios.contains(channel)) return;
    setSwitch(channel, !m_ios[channel].value);
}

bool plugin::isSwitchOn(const QString &channel, bool value)
{
    return (getSwitch(channel) == value);
}

int plugin::countSwitchs()
{
    return m_ios.size();
}

void plugin::cacheToDevice()
{
    QMap<QString, unsigned char>::const_iterator it = m_cache.constBegin();
    for(; it != m_cache.constEnd(); ++it) {
        //SENDEN
        QByteArray str;
        str.append("Sw");
        str.append(it.value());
        str.append(m_user.toLatin1());
        str.append(m_pwd.toLatin1());
        m_writesocket->writeDatagram(str, QHostAddress(it.key()), m_sendPort);
    }
}

void plugin::initialize()
{
	if (m_sendPort==0)
		return;
	
    clear();
    QByteArray str("wer da?");
    str.append(0x0D);
    str.append(0x0A);
    m_writesocket->writeDatagram(str, QHostAddress::Broadcast, m_sendPort);
}

void plugin::clear()
{
    m_ios.clear();
    m_ios.clear();
	
	changeProperty(SceneDocument::createModelReset("switches.anel_sockets", "channel").getData());

    SceneDocument doc;
    doc.setMethod("clear");
	doc.setComponentID(m_pluginid);
	doc.setInstanceID(m_instanceid);
    callRemoteComponentMethod("scenecontrol.switchesnull", doc.getData());
}

void plugin::configChanged(const QByteArray &configid, const QVariantMap &data)
{
    Q_UNUSED(configid);
    if(data.contains(QLatin1String("sendingport")) && data.contains(QLatin1String("listenport")) &&
            data.contains(QLatin1String("username")) && data.contains(QLatin1String("password")))
        connectToIOs(data[QLatin1String("sendingport")].toInt(), data[QLatin1String("listenport")].toInt(),
                     data[QLatin1String("username")].toString(), data[QLatin1String("password")].toString());
}

void plugin::requestProperties(int sessionid)
{
    changeProperty(SceneDocument::createModelReset("switches.anel_sockets", "channel").getData(), sessionid);
    QMap<QString, plugin::iochannel>::iterator i = m_ios.begin();
    for(; i != m_ios.end(); ++i) {
        const plugin::iochannel str = i.value();
        SceneDocument sc = SceneDocument::createModelChangeItem("switches.anel_sockets");
        sc.setData("channel", i.key());
        sc.setData("value", str.value);
        changeProperty(sc.getData(), sessionid);
    }
}
