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
#include "abstractevent.h"
#include <QTimer>

class EventRemoteKey : public AbstractEvent
{
    Q_OBJECT
    Q_PROPERTY(QString key READ key WRITE setKey);
    Q_PROPERTY(bool pressed READ pressed WRITE setPressed);
    Q_PROPERTY(int channel READ channel WRITE setChannel);
    Q_PROPERTY(int repeat READ repeat WRITE setRepeat)
	Q_PROPERTY(int repeatinit READ repeatinit WRITE setRepeatinit)
public:
    EventRemoteKey(QObject* parent = 0);
    virtual void newvalues();
    void keySlot(QString keycode,QString keyname,uint channel,int pressed);
	void stopRepeat() ;

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
	QTimer m_timer;
	bool m_dorepeat;
	private Q_SLOTS:
		void retrigger() ;
};

#endif // EventDateTime_h
