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
#include <Qt>

class AbstractPlugin;
class ActorCollection : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString profileid READ profileid WRITE setProfileid);
    Q_CLASSINFO("profileid_model", "CollectionModel")
    Q_CLASSINFO("profileid_model_displaytype", "0");
    Q_CLASSINFO("profileid_model_savetype", "32");
    Q_PROPERTY(ActorCollection::actionEnum action READ action WRITE setAction);
public:
    enum actionEnum
    {
        StartProfile,
        CancelProfile
    };
    Q_ENUMS(actionEnum);

    ActorCollection(QObject* parent=0);
    virtual ProvidedTypes providedtypes() {
        return ActionType;
    }
    QString profileid() const;
    void setProfileid(const QString& value);
    ActorCollection::actionEnum action() const;
    void setAction(ActorCollection::actionEnum value);
private:
    QString m_id;
    ActorCollection::actionEnum m_action;
};

#endif // ACTORPROFILESERVICEPROVIDER_H
