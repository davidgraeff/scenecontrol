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

#pragma once
#include <QAbstractListModel>
#include <QStringList>
#include <QUuid>
#include <QTimer>

#include <QTcpSocket>
#include <qprocess.h>

class AbstractPlugin;

class MediaController : public QObject
{
    Q_OBJECT
public:
    MediaController(AbstractPlugin* plugin);
    ~MediaController();
	/**
	 * Return volumes for all known sinks
	 */
	QMap<QByteArray, QPair<double, bool> > volumes() {return m_paVolume;}

	/**
	 * Set sink Volume
	 * \param sink PA sink name
	 * \param state 0=unmute, 1=mute, 2=toggle
	 */
    void setPAMute(const QByteArray sink, int state); 
    void setPAVolume(const QByteArray sink, double volume, bool relative = false);
private:
    AbstractPlugin* m_plugin;
    QMap<QByteArray, QPair<double, bool> > m_paVolume;
    QProcess* m_playerprocess;
private Q_SLOTS:
    void slotreadyRead ();
    void slotconnected();
    void slotdisconnected(int);
    void sloterror(QProcess::ProcessError e);
    void readyReadStandardError() ;
Q_SIGNALS:
    void pulseSinkChanged(double volume, bool mute, const QString& sinkid);
};
