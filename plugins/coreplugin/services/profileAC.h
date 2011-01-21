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
#include <Qt>

class AbstractPlugin;
class ActorCollection : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString profileid READ profileid WRITE setProfileid);
    Q_CLASSINFO("profileid_model", "CollectionsModel")
    Q_CLASSINFO("profileid_model_displaytype", "0");
    Q_CLASSINFO("profileid_model_savetype", "32");
    Q_PROPERTY(actionEnum action READ action WRITE setAction);
    Q_ENUMS(actionEnum);
public:
    enum actionEnum
    {
        StartProfile,
        CancelProfile
    };

    ActorCollection(QObject* parent=0);
	virtual QString service_name(){return tr("Profilsteuerung");}
	virtual QString service_desc(){return tr("Startet oder stopped ein Profil");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
			case 0:
                return tr("Profil");
        case 1:
            switch (enumindex) {
            case 0:
                return tr("Starten");
            case 1:
                return tr("Stoppen");
            default:
                return tr("Aktion");
            }
        default:
            return QString();
        }
    }
    QString profileid() const;
    void setProfileid(const QString& value);
    ActorCollection::actionEnum action() const;
    void setAction(ActorCollection::actionEnum value);
private:
    QString m_id;
    ActorCollection::actionEnum m_action;
};
