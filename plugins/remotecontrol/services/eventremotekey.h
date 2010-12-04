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

#ifndef EventRemoteKey_h
#define EventRemoteKey_h
#include <QTimer>
#include <shared/abstractserviceprovider.h>

class EventRemoteKey : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString key READ key WRITE setKey);
    Q_CLASSINFO("key_statetracker_id", "RemoteControlKeyStateTracker")
    Q_CLASSINFO("key_statetracker_property", "key")
    Q_PROPERTY(bool pressed READ pressed WRITE setPressed);
    Q_CLASSINFO("pressed_statetracker_id", "RemoteControlKeyStateTracker")
    Q_CLASSINFO("pressed_statetracker_property", "pressed")
    Q_PROPERTY(int channel READ channel WRITE setChannel);
    Q_CLASSINFO("channel_statetracker_id", "RemoteControlKeyStateTracker")
    Q_CLASSINFO("channel_statetracker_property", "channel")
    Q_PROPERTY(int repeat READ repeat WRITE setRepeat)
    Q_CLASSINFO("repeat_min", "-1");
    Q_CLASSINFO("repeat_max", "1000");
    Q_PROPERTY(int repeatinit READ repeatinit WRITE setRepeatinit)
    Q_CLASSINFO("repeatinit_min", "-1");
    Q_CLASSINFO("repeatinit_max", "1000");
public:
    EventRemoteKey(QObject* parent = 0);
	virtual QString service_name(){return tr("Fernbedienungsereignis");}
	virtual QString service_desc(){return tr("Wird ausgelöst, sobald ein Fernbedienungsknopf gedrückt wurde. Taste kann wiederholt werden, sobald die Wiederholungsstartzeit überschritten ist.");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Taste");
        case 1:
            return tr("Gedrückt");
        case 2:
            return tr("Kanal");
        case 3:
            return tr("Wiederholen (ms)");
        case 4:
            return tr("Wiederholen/Startzeit (ms)");
        default:
            return QString();
        }
    }
    QString key();
    void setKey(QString value);
    bool pressed();
    void setPressed(bool value);
    int channel();
    void setChannel(int value);
    int repeat() const ;
    void setRepeat( int r ) ;

    int repeatinit() const {
        return m_repeatinit;
    }

    void setRepeatinit( int r ) {
        m_repeatinit = r;
    }
private:
    QString m_key;
    bool m_pressed;
    int m_repeat;
    int m_repeatinit;
    int m_channel;
};

#endif // EventDateTime_h
