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

	Purpose: Authenticates users on windows (XP and later) or pam based linux
*/

#pragma once
#include <QObject>
#include <QThread>
#include <QPair>
#include <QList>
#include <QMutex>
#include <QWaitCondition>
#define MAX_SIMULTANEOUS_LOGINS 5

class AuthThread : public QThread
{
    Q_OBJECT
public:
    AuthThread() :m_keeprunning(true) {}
    void stop();
    void run();
    bool query(const QString& sessionid, const QString& name, const QString& pwd);
private:
    bool m_keeprunning;
    class AuthQueryData {
    public:
        QString sessionid;
        QByteArray name;
        QByteArray pwd;
        AuthQueryData(const QString& sessionid, const QByteArray& name, const QByteArray& pwd)
                : sessionid(sessionid), name(name), pwd(pwd) {}
    };
    QList<AuthQueryData*> m_creds;
    QMutex mutex;
    QWaitCondition bufferNotFull;
Q_SIGNALS:
    void auth_success(const QString& sessionid, const QString& name);
    void auth_failed(const QString& sessionid, const QString& name);
};
