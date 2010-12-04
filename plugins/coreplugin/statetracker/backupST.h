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

#ifndef BACKUPSTATETRACKER_H
#define BACKUPSTATETRACKER_H
#include "shared/abstractstatetracker.h"

class BackupStateTracker : public AbstractStateTracker
{ 
  Q_OBJECT
  Q_PROPERTY(QStringList backupids READ backupids WRITE setBackupids);
  Q_PROPERTY(QStringList backupnames READ backupnames WRITE setBackupnames);
  public:
    BackupStateTracker(QObject* parent = 0);
    QStringList backupids() ;
    void setBackupids(const QStringList& b) ;
    QStringList backupnames() ;
    void setBackupnames(const QStringList& b) ;
private:
	QStringList m_backupids;
	QStringList m_backupnames;
};

#endif // BACKUPSTATETRACKER_H
