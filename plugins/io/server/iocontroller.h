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

#ifndef IOCONTROLLER_H
#define IOCONTROLLER_H

#include <QObject>
#include <shared/server/qextserialport.h>
#include <QTimer>

class myPluginExecute;
class AbstractStateTracker;
class PinNameStateTracker;
class PinValueStateTracker;
class IOControllerThread;

class IOController : public QObject
{
    Q_OBJECT
public:
    IOController(myPluginExecute* plugin);
    ~IOController();
    QString getPinName ( uint pin );
    void setPin ( uint pin, bool value );
    void setPinName ( uint pin, const QString& name );
    void togglePin ( uint pin );
    bool getPin(unsigned int pin) const;
    int countPins();
    void setLedState(bool state);
    QList<AbstractStateTracker*> getStateTracker();
private:
	QString m_pluginname;
    QList<PinValueStateTracker*> m_values;
    QList<PinNameStateTracker*> m_names;
    QByteArray m_buffer;
	void determinePins(const QByteArray& data);
    int m_pins;
    // rs232 special
    QextSerialPort* m_serial;
    QTimer m_panicTimer;
    int m_bufferpos;
    enum readStateEnum {
        ReadOK,
        ReadEnd
    };
    readStateEnum m_readState;
private slots:
    void readyRead();
    void panicTimeout();
Q_SIGNALS:
    void stateChanged(AbstractStateTracker*);
    void dataLoadingComplete();
};

#endif // IOCONTROLLER_H
