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

#ifndef ACTORPROFILESERVICEPROVIDER_H
#define ACTORPROFILESERVICEPROVIDER_H
#include "shared/abstractserviceprovider.h"

class AbstractPlugin;
class ActorCollection : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString profileid READ profileid WRITE setProfileid);
	Q_PROPERTY(ActorCollection::ActorCollectionEnum action READ action WRITE setAction);
public:
	enum ActorCollectionEnum
	{
		StartProfile,
		CancelProfile
	};
	Q_ENUMS(ActorProfileEnum);
	
	ActorCollection(QObject* parent=0);
	virtual ProvidedTypes providedtypes(){return ActionType;}
    QString profileid() const;
    void setProfileid(const QString& value);
	ActorCollection::ActorCollectionEnum action() const;
	void setAction(ActorCollection::ActorCollectionEnum value);
private:
    QString m_id;
	ActorCollection::ActorCollectionEnum m_action;
};

#endif // ACTORPROFILESERVICEPROVIDER_H
