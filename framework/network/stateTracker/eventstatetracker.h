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

#ifndef EventStateTracker_h
#define EventStateTracker_h
#include "abstractstatetracker.h"

class EventStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(qreal volume READ volume WRITE setVolume)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString filename READ filename WRITE setFilename)
    Q_PROPERTY(int state READ state WRITE setState)
public:
    EventStateTracker(QObject* parent = 0);
    
    qreal volume() const ;
    void setVolume( qreal v ) ;
    const QString& title() const ;
    void setTitle( const QString& t ) ;
    const QString& filename() const ;
    void setFilename( const QString& f ) ;
    int state() const ;
    void setState( int s ) ;
    
  private:
    qreal m_volume;
    QString m_title;
    QString m_filename;
    int m_state;
};

#endif // EventStateTracker_h

