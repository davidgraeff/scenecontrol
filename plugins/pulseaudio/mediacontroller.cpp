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

#include "mediacontroller.h"
#include <QSettings>
#include <limits>
#include <stdio.h>
#include <qprocess.h>
#include "plugin_server.h"
#include <shared/abstractplugin.h>
#include <statetracker/pulsestatetracker.h>

MediaController::MediaController(myPluginExecute* plugin) : m_plugin(plugin)
{

    m_playerprocess = new QProcess();
    m_playerprocess->setReadChannel(QProcess::StandardOutput);
    connect (m_playerprocess, SIGNAL(error(QProcess::ProcessError)),SLOT(sloterror(QProcess::ProcessError)));
    connect (m_playerprocess, SIGNAL(finished(int)),SLOT(slotdisconnected(int)));
    connect (m_playerprocess, SIGNAL(started()),SLOT(slotconnected()));
    connect (m_playerprocess, SIGNAL(readyRead()),SLOT(slotreadyRead()));
    connect (m_playerprocess, SIGNAL(readyReadStandardError()),SLOT(readyReadStandardError()));

    slotdisconnected(0);
}

MediaController::~MediaController()
{
	m_playerprocess->blockSignals(true);
    m_playerprocess->terminate();
    if (!m_playerprocess->waitForFinished())
        m_playerprocess->kill();
}


void MediaController::slotconnected()
{
}

void MediaController::slotdisconnected(int)
{
    m_playerprocess->start(QLatin1String("roompulsehelper"));
}

void MediaController::sloterror(QProcess::ProcessError)
{
	qWarning()<<"PulseAudio process communication error";
}

void MediaController::readyReadStandardError()
{
    qWarning()<<"PulseAudio error:" << m_playerprocess->readAllStandardError().replace('\n',' ');
}

void MediaController::slotreadyRead()
{
    while (!m_playerprocess->atEnd())
    {
        QByteArray line = m_playerprocess->readLine();
        line.chop(1);
        QList<QByteArray> args = line.split(' ');

		if (args[0] == "pa_version" && args.size()==3) {
			qDebug() << "Pulseaudio Version:" << args[2];
		} else if (args[0] == "pa_sink" && args.size()==4) {
            PulseStateTracker* p = m_paStateTrackers[args[1]];
            if (!p) {
                p = new PulseStateTracker(this);
                p->setSinkname(QString::fromAscii(args[1]));
                m_paStateTrackers.insert(args[1], p);
            }
            p->setMute(args[2].toInt());
            p->setVolume(args[3].toUInt()/10000.0);
            emit stateChanged(p);
        }
    }
}

QList<AbstractStateTracker*> MediaController::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    QList<PulseStateTracker*> k = m_paStateTrackers.values();
    foreach(PulseStateTracker* p, k) l.append(p);
    return l;
}

void MediaController::setPAMute(const QByteArray sink, bool mute) {
    m_playerprocess->write("pa_mute " + sink + " " + QByteArray::number(mute) +"\n");
}

void MediaController::togglePAMute(const QByteArray sink) {
    m_playerprocess->write("pa_mute " + sink +" 2\n");
}

void MediaController::setPAVolume(const QByteArray sink, double volume, bool relative) {
    if (relative)
        m_playerprocess->write("pa_volume_relative " + sink + " " + QByteArray::number(volume*100) + "\n");
    else {
        m_playerprocess->write("pa_volume " + sink + " " + QByteArray::number(volume*100) + "\n");
    }
}

