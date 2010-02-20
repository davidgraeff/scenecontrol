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

#ifndef MPRISSTATETRACKER_H
#define MPRISSTATETRACKER_H
#include "abstractstatetracker.h"
#include "media/mediacmds.h"

class OrgFreedesktopMediaPlayerInterface;
class MprisStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(QString mprisid READ mprisid WRITE setMprisid)
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(int position READ position WRITE setPosition)
    Q_PROPERTY(int volume READ volume WRITE setVolume)
    Q_PROPERTY(int state READ state WRITE setState)
public:
    MprisStateTracker(QObject* parent = 0);

    const QString& mprisid() const ;
    void setMprisid( const QString& m ) ;
    const QString& url() const ;
    void setUrl( const QString& u ) ;
    int position() const ;
    void setPosition( int p ) ;
    int volume() const ;
    void setVolume( int v ) ;
    int state() const ;
    void setState( int s ) ;
    
private:
    QString m_mprisid;
    QString m_url;
    int m_position;
    int m_volume;
    int m_state;
};

#endif // MPRISSTATETRACKER_H
