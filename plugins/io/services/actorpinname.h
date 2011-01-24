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

#ifndef ACTORPINNAMESERVICEPROVIDER_H
#define ACTORPINNAMESERVICEPROVIDER_H

#include "shared/abstractserviceprovider.h"


class ActorPinName : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString pin READ pin WRITE setPin);
    Q_CLASSINFO("pin_model", "PinsModel")
    Q_CLASSINFO("pin_model_displaytype", "0");
    Q_CLASSINFO("pin_model_savetype", "32");
    Q_PROPERTY(QString pinname READ pinname WRITE setPinname);
public:
    ActorPinName(QObject* parent = 0);
	virtual QString service_name(){return tr("PinName");}
	virtual QString service_desc(){return tr("Setzt den Namen eines Pins");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Pin");
        case 1:
            return tr("Name");
        default:
            return QString();
        }
    }
    QString pinname() const;
    void setPinname(const QString& value);
    QString pin() const;
    void setPin(QString value);
private:
    QString m_pinname;
    QString m_pin;
};

#endif // ACTORPINNAMESERVICEPROVIDER_H
