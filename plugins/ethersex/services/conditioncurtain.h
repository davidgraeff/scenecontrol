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

#ifndef ConditionCurtain_h
#define ConditionCurtain_h
#include <shared/abstractserviceprovider.h>

class ConditionCurtain : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY ( unsigned int value READ value WRITE setValue );
    Q_CLASSINFO("value_max", "10");
public:
    ConditionCurtain(QObject* parent = 0);
	virtual QString service_name(){return tr("Rollopositionsbedingung");}
	virtual QString service_desc(){return tr("Bedingung, dass das Rollo in einer gewissen Position sein muss");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Rolloposition");
        default:
            return QString();
        }
    }

    unsigned int value() const;
    void setValue ( unsigned int v );
private:
    unsigned int m_value;
};

#endif // ConditionCurtain_h
