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

#ifndef ACTORMPRISSERVICEPROVIDER_H
#define ACTORMPRISSERVICEPROVIDER_H

#include "abstractactor.h"

class ActorMpris : public AbstractActor
{
    Q_OBJECT
    Q_PROPERTY(int cmd READ cmd WRITE setCmd);
    Q_PROPERTY(QString mprisid READ mprisid WRITE setMprisid);
public:
    ActorMpris(QObject* parent = 0);
    int cmd() const ;
    void setCmd(int value) ;
    QString mprisid() const ;
    void setMprisid(const QString& value) ;
    virtual void changed() ;
private:
    QString m_mprisid;
    int m_cmd;
};
#endif // ACTORMPRISSERVICEPROVIDER_H
