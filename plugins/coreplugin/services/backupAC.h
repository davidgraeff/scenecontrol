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

#ifndef ACTORBACKUP_H
#define ACTORBACKUP_H
#include "shared/abstractserviceprovider.h"

class AbstractPlugin;
class ActorBackup : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(ActorBackup::ActorBackupEnum action READ action WRITE setAction);
public:
    ActorBackup(QObject* parent=0);
	virtual ProvidedTypes providedtypes(){return ActionType;}
    enum ActorBackupEnum
    {
      CreateBackup,
      RemoveBackup,
      RestoreBackup
    };
    Q_ENUMS(ActorSystemEnum);
    ActorBackup::ActorBackupEnum action() const;
    void setAction(ActorBackup::ActorBackupEnum value);
    QString backupid() const;
    void setBackupid(const QString& value);
private:
    QString m_id;
	ActorBackup::ActorBackupEnum m_action;
};

#endif // ACTORBACKUP_H
