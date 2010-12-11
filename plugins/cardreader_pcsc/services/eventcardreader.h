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
#include <shared/abstractserviceprovider.h>

class EventCardReader : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString atr READ atr WRITE setAtr);
	Q_CLASSINFO("atr_statetracker_id", "CardReaderPCSCStateTracker")
	Q_CLASSINFO("atr_statetracker_property", "atr")
	Q_PROPERTY(int state READ state WRITE setState);
	Q_CLASSINFO("state_min", "0");
	Q_CLASSINFO("state_max", "1");
public:
	EventCardReader(QObject* parent = 0);
	virtual QString service_name(){return tr("Kartenleserereignis");}
	virtual QString service_desc(){return tr("Wird ausgelöst, sobald eine Karte in den Kartenleser eingesteckt oder aus dem Kartenleser entfernt wird");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("ATR");
        case 1:
            return tr("Status");
        default:
            return QString();
        }
    }
    QString atr();
	void setAtr(QString value);
	int state();
	void setState(int value);
private:
    QString m_atr;
	int m_state;
};
