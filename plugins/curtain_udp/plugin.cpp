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

#include "plugin.h"
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    plugin p;
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin() : m_socket(0) {
    connect(&m_connectTimer, SIGNAL(timeout()), SLOT(resendConnectSequence()));
    m_connectTimer.setSingleShot(true);
}
plugin::~plugin() {
    delete m_socket;
}

void plugin::clear() {
    m_connectTimer.stop();
    m_curtainvalue = -1;
    m_curtainmax = -1;
    m_curtainButtons = -1;
    curtainChanged();
}

void plugin::initialize() {}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
    Q_UNUSED(configid);
    if (data.contains(QLatin1String("server")) && data.contains(QLatin1String("port")))
        connectToServer ( data[QLatin1String("server")].toString(), data[QLatin1String("port")].toInt() );
}

bool plugin::isCurtainValue(  int lower, int upper ) {
    const int v = getCurtain ( );
    if ( v>upper ) return false;
    if ( v<lower ) return false;
    return true;
}

void plugin::requestProperties(int sessionid) {
    requestProperties(sessionid);
}

void plugin::curtainChanged(int sessionid) {
    ServiceData sc = ServiceData::createModelChangeItem("curtain.value");
    if (m_curtainvalue != -1) sc.setData("value", m_curtainvalue);
    if (m_curtainmax != -1) sc.setData("max", m_curtainmax);
    if (m_curtainButtons != -1) sc.setData("buttons", m_curtainButtons);
    changeProperty(sc.getData(), sessionid);
}

int plugin::getCurtain (  ) const {
    return m_curtainvalue;
}

int plugin::getCurtainMax() const
{
    return m_curtainmax;
}


void plugin::syncCurtain () {
    uint8_t data = 254;
    m_socket->write ( (char*)&data, sizeof ( data ) );
}

void plugin::setCurtainRelative (  int value ) {
    setCurtain ( value + m_curtainvalue );
}

void plugin::setCurtain (  int value ) {
    if ( !m_socket ) return;

    value = qBound ( 0, value, m_curtainmax );
    m_curtainvalue = value;
    curtainChanged ( );

    uint8_t data = m_curtainvalue;
    m_socket->write ( (char*)&data, sizeof ( data ) );
}

void plugin::readyRead() {
    m_connectTimer.stop();
    while (m_socket->hasPendingDatagrams()) {
        QByteArray bytes;
        bytes.resize ( m_socket->pendingDatagramSize() );
        m_socket->readDatagram ( bytes.data(), bytes.size() );

        while ( bytes.size() > 9 ) {
            if (bytes.startsWith("curtain"))  {
                m_curtainvalue = bytes[7];
                m_curtainmax = bytes[8];
		m_curtainButtons = bytes[9];
                curtainChanged();
                bytes = bytes.mid(10);
            } else {
                qWarning() << m_plugin->pluginid() << "Failed to parse" << bytes << bytes.size() << 8+bytes[7];
                break;
            }
        } //while
    }
}

void plugin::connectToServer ( const QString& host, int port ) {
    clear();
    m_sendPort = port;
    delete m_socket;
    m_socket = new QUdpSocket(this);
    connect(m_socket,SIGNAL(readyRead()),SLOT(readyRead()));
    m_socket->connectToHost(QHostAddress(host),m_sendPort);
    m_connectTime=1000;
    m_connectTimer.start(m_connectTime);
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(data);
}

void plugin::resendConnectSequence() {
    // request all channel values
    uint8_t data = 255;
    m_socket->write ( (char*)&data, sizeof ( data ) );
    m_socket->flush();
    m_connectTime *= 2;
    if (m_connectTime>60000)
        m_connectTime=60000;
    m_connectTimer.start(m_connectTime);
}

