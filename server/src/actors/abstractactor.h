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

#ifndef ACTORSERVICEPROVIDER_H
#define ACTORSERVICEPROVIDER_H
#include "abstractserviceprovider.h"

class AbstractActor : public AbstractServiceProvider
{
    Q_OBJECT
    // Delay until this actor get executed
    Q_PROPERTY(int delay READ delay WRITE setDelay);
    // Immediately execute this actor and delete it afterwards
    Q_PROPERTY(bool iExecute READ iExecute WRITE setiExecute);
public:
    AbstractActor(QObject* parent = 0);
    virtual void execute() = 0;
    int delay() const ;
    void setDelay(int cmd) ;
    bool iExecute() const ;
    void setiExecute(bool cmd) ;
private:
    bool m_iExecute;
    int m_delay;
};

#endif // ACTORSERVICEPROVIDER_H
