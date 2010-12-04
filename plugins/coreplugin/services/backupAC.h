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
#include "shared/abstractserviceprovider.h"

class AbstractPlugin;
class ActorBackup : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(actionEnum action READ action WRITE setAction);
    Q_ENUMS(actionEnum);
    Q_PROPERTY(QString backupid READ backupid WRITE setBackupid);
    Q_CLASSINFO("backupid_model", "BackupsModel")
    Q_CLASSINFO("backupid_model_displaytype", "0");
    Q_CLASSINFO("backupid_model_savetype", "32");
	Q_CLASSINFO("backupid_props", "modeleditable|optional")
	Q_PROPERTY(QString backupname READ backupname WRITE setBackupname);
public:
    enum actionEnum
    {
      CreateBackup, // no id, name optional
      RemoveBackup, // id only
      RestoreBackup, // id only
	  RenameBackup // id, name optional
    };
    ActorBackup(QObject* parent=0);
	virtual QString service_name(){return tr("Backupkommando");}
	virtual QString service_desc(){return tr("Erstellt, entfernt, stellt wieder her und kann Backups umbennenen");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            switch (enumindex) {
            case 0:
                return tr("Erstellen (ID nicht benötigt)");
            case 1:
                return tr("Entfernen");
            case 2:
                return tr("Wiederherstellen");
            case 3:
                return tr("Umbennenen");
            default:
                return tr("Aktion");
            }
			case 1:
                return tr("ID");
			case 2:
                return tr("Name");
        default:
            return QString();
        }
    }
    ActorBackup::actionEnum action() const;
    void setAction(ActorBackup::actionEnum value);
    QString backupid() const;
    void setBackupid(const QString& value);
    QString backupname() const;
    void setBackupname(const QString& value);
private:
    QString m_backupid;
	QString m_backupname;
	ActorBackup::actionEnum m_action;
};
