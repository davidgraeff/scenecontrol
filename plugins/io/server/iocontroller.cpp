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

#include "iocontroller.h"
#include <QDebug>
#include <QSettings>
#include "shared/server/qextserialport.h"
#include "statetracker/pinvaluestatetracker.h"
#include "statetracker/pinnamestatetracker.h"
#include <qfile.h>
#include "plugin_server.h"
#include <shared/abstractplugin.h>
#include <QUdpSocket>
#include <QThread>
IOController::IOController(myPluginExecute* plugin) : m_pluginname(plugin->base()->name()), m_listenSocket(0) {
    m_writesocket = new QUdpSocket(this);
}

IOController::~IOController() {
    qDeleteAll(m_values);
    qDeleteAll(m_names);
}

void IOController::connectToIOs(int portSend, int portListen, const QString& user, const QString& pwd) {
    m_sendPort = portSend;
    m_user = user;
    m_pwd = pwd;
    delete m_listenSocket;
    m_listenSocket = new QUdpSocket(this);
    connect(m_listenSocket,SIGNAL(readyRead()),SLOT(readyRead()));
    m_listenSocket->bind(QHostAddress::Broadcast,portListen);

    QByteArray str("wer da?");
    str.append(0x0D);
    str.append(0x0A);
    m_writesocket->writeDatagram(str, QHostAddress::Broadcast, portSend);
}

void IOController::readyRead() {
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
    settings.beginGroup(m_pluginname);
    settings.beginGroup ( QLatin1String("pinnames") );
    // set new
    int pins = cmd.size()-2;
    unsigned char activated = ~cmd[cmd.size()-2].toInt();
    const QHostAddress host = QHostAddress(QString::fromAscii(cmd[2]));
    for ( int i=6;i<pins;++i )
    {
        if (!(activated & (1 << (i-6)))) continue;
        const QStringList data = QString::fromLatin1(cmd[i]).split(QLatin1Char(','));
        const QString initialname = data[0];
        const int value = data[1].toInt();
        const QString name = settings.value ( initialname, initialname ).toString();
        const bool alreadyinside = m_mapPinToHost.contains(initialname);
        m_mapPinToHost[initialname] = QPair<QHostAddress,uint>(host,i-5);
        PinValueStateTracker* cv = (alreadyinside ? m_values[initialname] : new PinValueStateTracker());
        PinNameStateTracker* cn =  (alreadyinside ? m_names[initialname] : new PinNameStateTracker());
        bool changed = false;
        if (!alreadyinside || (m_values[initialname]->value() != value)) changed = true;
        cv->setPin(initialname);
        cv->setValue(value);
        m_values[initialname] = cv;
        if (changed) emit stateChanged(cv);
        changed = false;
        if (!alreadyinside || (m_names[initialname]->value() != name)) changed = true;
        cn->setPin(initialname);
        cn->setValue(name);
        m_names[initialname] = cn;
        if (changed) emit stateChanged(cn);
    }
    emit dataLoadingComplete();
}

bool IOController::getPin(const QString& pin) const
{
    if (!m_values.contains(pin)) return false;
    return m_values[pin]->value();
}

void IOController::setPin ( const QString& pin, bool value )
{
    if (!m_mapPinToHost.contains(pin)) return;
    QPair<QHostAddress,uint> p = m_mapPinToHost[pin];

    //SENDEN
    QByteArray str;
    str.append( (value?"Sw_on":"Sw_off") );
    str.append(QByteArray::number(p.second));
    str.append(m_user.toLatin1());
    str.append(m_pwd.toLatin1());
    m_writesocket->writeDatagram(str, p.first, m_sendPort);

    // kein stateChanged: wird über readyRead mitgeteilt
    //if (!m_values.contains(pin)) return;
    //m_values[pin]->setValue(value);
    //emit stateChanged(m_values[pin]);
}

void IOController::setPinName ( const QString& pin, const QString& name )
{
    if ( !m_names.contains(pin) ) return;
    m_names[pin]->setValue(name);

    QSettings settings;
    settings.beginGroup(m_pluginname);
    settings.beginGroup ( QLatin1String("pinnames") );
    settings.setValue ( pin, name );

    emit stateChanged(m_names[pin]);
}

void IOController::togglePin ( const QString& pin )
{
    if (!m_values.contains(pin)) return;
    setPin ( pin, !m_values[pin]->value() );
}

QList< AbstractStateTracker* > IOController::getStateTracker() {
    QList<AbstractStateTracker*> temp;
    foreach (PinValueStateTracker* p, m_values) temp.append(p);
    foreach (PinNameStateTracker* p, m_names) temp.append(p);
    return temp;
}

int IOController::countPins() {
    return m_values.size();
}

QString IOController::getPinName(const QString& pin) {
    if (!m_names.contains(pin)) return QString();
    return m_names[pin]->value();
}
