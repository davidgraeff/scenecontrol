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

#pragma once
#include <QObject>
#include <QStringList>
#include "abstractplugin.h"

class MediaController;
class plugin : public AbstractPlugin
{
    Q_OBJECT
public:

    virtual ~plugin();
	virtual void initialize();
private:
	MediaController* m_mediacontroller;
    virtual void configChanged ( const QByteArray& configid, const QVariantMap& data );
    virtual void requestProperties ( int sessionid );
private Q_SLOTS:
	// from mpdController
    void playlistChanged ( QString );
    void playlistsChanged ( QString,int );
	void trackChanged(const QString& filename, const QString& trackname, int track, uint position_in_ms, uint total_in_ms, int state);
    void volumeChanged ( double );
	QVariantMap stateChanged(MediaController* client, bool propagate = true);
	// control methods
	void mpdvolume(double volume, bool relative) ;
	void mpdposition(int position_in_ms, bool relative) ;
	void play() ;
	void pause() ;
	void stop() ;
	void next() ;
	void previous() ;
	void nextPlaylist() ;
	void previousPlaylist() ;
	void dumpMediaInfo() ;
	void mpdchangeplaylist(const QString& playlistid, int track) ;
};

