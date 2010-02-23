/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef MediaStateTracker_h
#define MediaStateTracker_h
#include "abstractstatetracker.h"

class MediaStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(QString playlistid READ playlistid WRITE setPlaylistid)
    Q_PROPERTY(qint64 position READ position WRITE setPosition)
    Q_PROPERTY(qint64 total READ total WRITE setTotal)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume)
    Q_PROPERTY(int state READ state WRITE setState)
    Q_PROPERTY(int track READ track WRITE setTrack)
public:
    MediaStateTracker(QObject* parent = 0);
    
    const QString& playlistid() const ;
    void setPlaylistid( const QString& p ) ;
    qint64 position() const ;
    void setPosition( qint64 p ) ;
    qint64 total() const ;
    void setTotal( qint64 t ) ;
    qreal volume() const ;
    void setVolume( qreal v ) ;
    int state() const ;
    void setState( int s ) ;
    int track() const ;
    void setTrack( int t ) ;
    
  private:
    QString m_playlistid;
    qint64 m_position;
    qint64 m_total;
    qreal m_volume;
    int m_state;
    int m_track;

};

#endif // MediaStateTracker_h
