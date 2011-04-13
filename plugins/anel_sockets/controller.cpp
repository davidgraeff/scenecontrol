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

#include "controller.h"
#include <QDebug>
#include <QSettings>
#include "shared/qextserialport/qextserialport.h"
#include <qfile.h>
#include <shared/abstractplugin.h>
#include <QUdpSocket>
#include <QThread>
#include <QStringList>

Controller::Controller(AbstractPlugin* plugin) : m_plugin(plugin), m_listenSocket(0) {
    m_writesocket = new QUdpSocket(this);
    connect(&m_cacheTimer, SIGNAL(timeout()), SLOT(cacheToDevice()));
    m_cacheTimer.setInterval(50);
    m_cacheTimer.setSingleShot(true);
}

Controller::~Controller() {}

void Controller::connectToIOs(int portSend, int portListen, const QString& user, const QString& pwd) {
    m_sendPort = portSend;
    m_user = user;
    m_pwd = pwd;
    delete m_listenSocket;
    m_listenSocket = new QUdpSocket(this);
    connect(m_listenSocket,SIGNAL(readyRead()),SLOT(readyRead()));
    m_listenSocket->bind(QHostAddress::Broadcast,portListen);

    reinitialize();
}

void Controller::readyRead() {
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
    QSettings settings;
    settings.beginGroup(m_plugin->pluginid());
    settings.beginGroup ( QLatin1String("pinnames") );
    // set new
    int pins = cmd.size()-2;
    unsigned char activated = ~cmd[cmd.size()-2].toInt();
    const QHostAddress host = QHostAddress(QString::fromAscii(cmd[2]));
    for ( int i=6;i<pins;++i )
    {
        const int pin = i-6;
        if (!(activated & (1 << pin))) continue;
        const QStringList data = QString::fromLatin1(cmd[i]).split(QLatin1Char(','));
        const QString initialname = data[0];
        const int value = data[1].toInt();
        const QString name = settings.value ( initialname, initialname ).toString();
        const bool alreadyinside = m_mapPinToHost.contains(initialname);
        m_mapPinToHost[initialname] = QPair<QHostAddress,uint>(host,pin);
        bool changed = false;
        if (!alreadyinside || (m_values[initialname] != value)) changed = true;
        m_values[initialname] = value;
        if (changed) emit valueChanged(initialname, value);
        changed = false;
        if (!alreadyinside || (m_names[initialname] != name)) changed = true;
        m_names[initialname] = name;
        if (changed) emit nameChanged(initialname, name);
        // update cache
        if (value)
            m_cache[host.toString()] |= (unsigned char)(1 << pin);
        else
            m_cache[host.toString()] &= (unsigned char)~(1 << pin);

    }
    emit dataLoadingComplete();
}

bool Controller::getPin(const QString& pin) const
{
    if (!m_values.contains(pin)) return false;
    return m_values[pin];
}

void Controller::setPin ( const QString& pin, bool value )
{
    if (!m_mapPinToHost.contains(pin)) return;
    QPair<QHostAddress,uint> p = m_mapPinToHost[pin];

    if (value)
        m_cache[p.first.toString()] |= (unsigned char)(1 << p.second);
    else
        m_cache[p.first.toString()] &= (unsigned char)~(1 << p.second);

    if (!m_cacheTimer.isActive()) m_cacheTimer.start();
}

void Controller::setPinName ( const QString& pin, const QString& name )
{
    if ( !m_names.contains(pin) ) return;
    m_names[pin] = name;

    QSettings settings;
    settings.beginGroup(m_plugin->pluginid());
    settings.beginGroup ( QLatin1String("pinnames") );
    settings.setValue ( pin, name );

    emit nameChanged(pin, name);
}

void Controller::togglePin ( const QString& pin )
{
    if (!m_values.contains(pin)) return;
    setPin ( pin, !m_values[pin] );
}

void Controller::cacheToDevice()
{
    QMap<QString, unsigned char>::const_iterator it = m_cache.constBegin();
    for (;it != m_cache.constEnd(); ++it) {
        //SENDEN
        QByteArray str;
        str.append( "Sw" );
        str.append(it.value());
        str.append(m_user.toLatin1());
        str.append(m_pwd.toLatin1());
        m_writesocket->writeDatagram(str, QHostAddress(it.key()), m_sendPort);
    }
}

int Controller::countPins() {
    return m_values.size();
}

QString Controller::getPinName(const QString& pin) {
    if (!m_names.contains(pin)) return QString();
    return m_names[pin];
}

void Controller::reinitialize() {
	m_values.clear();
	m_names.clear();
	
    QByteArray str("wer da?");
    str.append(0x0D);
    str.append(0x0A);
    m_writesocket->writeDatagram(str, QHostAddress::Broadcast, m_sendPort);
}
