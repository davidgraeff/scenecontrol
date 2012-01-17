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
#include <QObject>
#include <QMap>
#include <QStringList>
#include <QUdpSocket>
#include <QTimer>
#include <QVariantMap>
#include <stdint.h>

class AbstractPlugin;

class Controller : public QObject
{
    Q_OBJECT
public:
    struct ledchannel {
        bool value;
        QString name;
        int port;
        int pin;
        ledchannel(int port, int pin, bool value, const QString& name) {
            this->port = port;
            this->pin = pin;
            this->value = value;
            this->name = name;
        }
        ledchannel() {
            value = false;
        }
    };
    struct ledid {
        int port;
        uint8_t pin;
        ledid(int port, uint8_t pin) {
            this->port = port;
            this->pin = pin;
        }
        bool operator<(ledid& vgl) {
            return (port<vgl.port || (port==vgl.port && pin < vgl.pin));
        }
    };

    Controller(AbstractPlugin* plugin);
    ~Controller();
    void connectToLeds(const QString& host, int port);

    int countChannels();
    QString getChannelName ( const Controller::ledid& channel );
    void setChannel ( const Controller::ledid& channel, bool value);
    void setChannelName ( const Controller::ledid& channel, const QString& name );
    void toogleChannel(const Controller::ledid& channel);
    bool getChannel(const Controller::ledid& channel) const;
    void registerPortObserver(unsigned char ioport, unsigned char pinmask) const;
    Controller::ledid getPortPinFromString(const QString& channel) const;
    QString getStringFromPortPin(const Controller::ledid& channel) const;

    QMap<ledid,ledchannel> m_leds;
private:
    AbstractPlugin* m_plugin;

    // udp
    int m_sendPort;
    QUdpSocket *m_socket;
private Q_SLOTS:
    // LIGHTS //
    void readyRead();
Q_SIGNALS:
    void ledChanged(const QString& id, const QString& name, int value);
    void watchpinChanged(const unsigned char port, const unsigned char pinmask);
    void dataLoadingComplete();
    void ledsCleared();
};
