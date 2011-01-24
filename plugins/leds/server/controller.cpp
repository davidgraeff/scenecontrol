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
#include <QSettings>
#include "statetracker/curtainstatetracker.h"
#include "statetracker/ledvaluestatetracker.h"
#include "statetracker/lednamestatetracker.h"
#include "plugin_server.h"
#include <qfile.h>
#include <QDebug>
#include <shared/server/qextserialport.h>
#include <shared/abstractplugin.h>

Controller::Controller(myPluginExecute* plugin) : m_pluginname(plugin->base()->name()), m_channels(0), m_bufferpos(0), m_readState(ReadOK), m_serial(0)
{
    m_curtainStateTracker = new CurtainStateTracker();
}

Controller::~Controller()
{
    qDeleteAll(m_values);
    qDeleteAll(m_names);
    delete m_serial;
    delete m_curtainStateTracker;
}


void Controller::readyRead()
{
    QByteArray bytes;
    bytes.resize(m_serial->bytesAvailable());
    m_serial->read(bytes.data(), bytes.size());
    m_buffer.append(bytes);
    if (m_readState==ReadOK) {
        for (int i=m_bufferpos;i<m_buffer.size();++i) {
            if (m_buffer.size()<=i) break;
            if (m_buffer[i] == 'O' && m_buffer[i+1] == 'K') {
                m_readState = ReadEnd;
                m_buffer.remove(0,i+2);
                m_bufferpos = 0;
                break;
            }
        }
    }
    if (m_readState == ReadEnd) {
        for (int i=m_bufferpos;i<m_buffer.size();++i) {
            if (m_buffer.size()<=i+1) break;
            if (m_buffer[i] == 'E' && m_buffer[i+1] == 'N' && m_buffer[i+2] == 'D') {
                m_readState = ReadOK;
                determineChannels(m_buffer.mid(0,i));
                m_buffer.remove(0,i+3);
                m_bufferpos = 0;
                break;
            }
        }
    }
}

void Controller::determineChannels(const QByteArray& data)
{
    if (data.isEmpty() || data.size() != (int)data[2]+3) {
        qWarning()<<m_pluginname<<__FUNCTION__<<"size missmatch:"<<(data.size()?((int)data[2]+3):0)<<data.size();
        return;
    }
    QSettings settings;
    settings.beginGroup(m_pluginname);
    settings.beginGroup ( QLatin1String("channelnames") );
    // clear old
    qDeleteAll(m_values);
    qDeleteAll(m_names);
    m_values.clear();
    m_names.clear();
    // set new
    m_curtainStateTracker->setCurtain((uint)data[0]);
    m_curtainStateTracker->setCurtainMax((uint)data[1]);
    emit stateChanged(m_curtainStateTracker);
    m_channels = (int)data[2];
    for ( int i=0;i<m_channels;++i )
    {
        const QString name = settings.value ( QLatin1String("channel")+QString::number( i ),
                                              tr("Channel %1").arg( i ) ).toString();
        ChannelValueStateTracker* cv = new ChannelValueStateTracker();
        m_values.append(cv);
        cv->setChannel(i);
        const int value = (uint8_t)data[i+3];
        cv->setValue(value);
        emit stateChanged(cv);
        ChannelNameStateTracker* cn = new ChannelNameStateTracker();
        m_names.append(cn);
        cn->setChannel(i);
        cn->setValue(name);
        emit stateChanged(cn);
    }
    emit dataLoadingComplete();
}

void Controller::refresh() {}

QList<AbstractStateTracker*> Controller::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    foreach (ChannelValueStateTracker* p, m_values) l.append(p);
    foreach (ChannelNameStateTracker* p, m_names) l.append(p);
    l.append(m_curtainStateTracker);
    return l;
}

void Controller::setCurtain(unsigned int position)
{
    if (!m_serial) return;
    m_curtainStateTracker->setCurtain(position);
    emit stateChanged(m_curtainStateTracker);
    const char t1[] = {0xdf, position};
    m_serial->write(t1, sizeof(t1));
}

unsigned int Controller::getCurtain()
{
    return m_curtainStateTracker->curtain();
}

void Controller::setChannelName ( uint channel, const QString& name )
{
    if ( channel>= ( unsigned int ) m_names.size() ) return;
    m_names[channel]->setValue(name);
    emit stateChanged(m_names[channel]);

    QSettings settings;
    settings.beginGroup(m_pluginname);
    settings.beginGroup ( QLatin1String("channelnames") );
    settings.setValue ( QLatin1String("channel")+QString::number ( channel ), name );
}

unsigned int Controller::getChannel(unsigned int channel) const
{
    if ( channel>= ( unsigned int ) m_values.size() ) return 300;
    return m_values.at(channel)->value();
}

void Controller::setChannel ( uint channel, uint value, uint fade )
{
    if (!m_serial) return;
    if ( channel>= ( unsigned int ) m_values.size() ) return;
    value = qBound ( ( unsigned int ) 0, value, ( unsigned int ) 255 );
    m_values[channel]->setValue(value);
    emit stateChanged(m_values[channel]);

    unsigned char cfade=0;
    switch (fade) {
    case STELLA_SET_IMMEDIATELY:
        cfade = 0xcf;
        break;
    case STELLA_SET_FADE:
        cfade = 0xbf;
        break;
    case STELLA_SET_FLASHY:
        cfade = 0xaf;
        break;
    case STELLA_SET_IMMEDIATELY_RELATIVE:
        cfade = 0x9f;
        break;
    default:
        break;
    };
    const char t1[] = {cfade, channel, value};
    m_serial->write(t1, sizeof(t1));
}

void Controller::inverseChannel(uint channel, uint fade)
{
    if ( channel>= ( unsigned int ) m_values.size() ) return;
    const unsigned int newvalue = 255 - m_values[channel]->value();
    setChannel(channel, newvalue, fade);
}

void Controller::setChannelExponential ( uint channel, int multiplikator, uint fade )
{
    if ( channel>= ( unsigned int ) m_values.size() ) return;
    unsigned int v = m_values[channel]->value();
    if (multiplikator>100) {
        if (v==0)
            v=1;
        else if (v==1)
            v=2;
        else
            v = (v * multiplikator)/100;
    } else {
        if (v==0 || v==1)
            v = 0;
        else
            v = (v * multiplikator)/100;
    }

    setChannel(channel, v, fade);
}

void Controller::setChannelRelative ( uint channel, int value, uint fade )
{
    if ( channel>= ( unsigned int ) m_values.size() ) return;
    value += m_values[channel]->value();
    const unsigned int v = ( unsigned int ) qMin ( 0, value);
    setChannel ( channel, v, fade );
}


int Controller::countChannels()
{
    return m_channels;
}

QString Controller::getChannelName(uint channel) {
    if (channel>=(uint)m_names.size()) return QString();
    return m_names[channel]->value();
}

void Controller::panicTimeout() {
    if (!m_serial) return;
    const char t1[] = {0x00};
    if (!m_serial->isOpen() || m_serial->write(t1, sizeof(t1)) == -1) {
        qWarning()<< "IO: Failed to reset panic counter. Try reconnection";
        m_serial->close();
        const char t1[] = {0xef};
        if (!m_serial->open(QIODevice::ReadWrite) || !m_serial->write(t1,  sizeof(t1))) {
            qWarning() << "IO: rs232 init fehler";
        }
    }
}
void Controller::connectToLeds(const QString& device) {
    // disable old
    m_panicTimer.stop();
    delete m_serial;
    m_serial = 0;
    // create new
    QSettings settings;
    settings.beginGroup(m_pluginname);
    m_serial = new QextSerialPort(device,QextSerialPort::EventDriven);
    m_serial->setBaudRate(BAUD115200);
    m_serial->setFlowControl(FLOW_OFF);
    m_serial->setParity(PAR_NONE);
    m_serial->setDataBits(DATA_8);
    m_serial->setStopBits(STOP_1);
    connect(m_serial, SIGNAL(readyRead()), SLOT(readyRead()));
    // Open device and ask for pins
    if (!QFile::exists(device)) {
        qWarning() << m_pluginname << "device not found"<<device;
        return;
    }
    const char t1[] = {0xef};
    if (!m_serial->open(QIODevice::ReadWrite) || !m_serial->write(t1, sizeof(t1))) {
        qWarning() << m_pluginname << "rs232 init fehler";
    }
    // panic counter
    m_panicTimer.setInterval(60000);
    connect(&m_panicTimer,SIGNAL(timeout()),SLOT(panicTimeout()));
    m_panicTimer.start();
}
