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

#ifndef ACTORPROFILESERVICEPROVIDER_H
#define ACTORPROFILESERVICEPROVIDER_H

#include "abstractactor.h"

enum ActorProfileEnum
{
  StartProfile,
  CancelProfile
};
Q_ENUMS(ActorProfileEnum);

class ActorProfile : public AbstractActor
{
    Q_OBJECT
    Q_PROPERTY(QString profileid READ profileid WRITE setProfileid);
    Q_PROPERTY(int action READ action WRITE setAction);
public:
    ActorProfile(QObject* parent = 0);
    QString profileid() const;
    void setProfileid(const QString& value);
    int action() const;
    void setAction(int value);
    virtual void changed() ;
private:
    QString m_id;
    int m_action;
};

#endif // ACTORPROFILESERVICEPROVIDER_H
