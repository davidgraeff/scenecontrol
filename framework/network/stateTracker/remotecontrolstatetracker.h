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

#ifndef REMOTECONTROLSTATETRACKER_H
#define REMOTECONTROLSTATETRACKER_H
#include "abstractstatetracker.h"

class RemoteControlStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected WRITE setConnected)
    Q_PROPERTY(int receivers READ receivers WRITE setReceivers)
    Q_PROPERTY(QString mode READ mode WRITE setMode)
public:
    RemoteControlStateTracker(QObject* parent = 0);
    bool connected() const ;
    void setConnected( bool c ) ;
    int receivers() const ;
    void setReceivers( int r ) ;
    const QString& mode() const ;
    void setMode( const QString& m ) ;
    
private:
    bool m_connected;
    int m_receivers;
    QString m_mode;
};

#endif // REMOTECONTROLSTATETRACKER_H
