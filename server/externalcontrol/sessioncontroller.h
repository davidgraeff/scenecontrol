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

	Purpose: Login session. Only logged in users may receive property changes and execute services.
*/

#pragma once
#include <QByteArray>
#include <QUuid>
#include <QStringList>

#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <QMap>
#include <shared/abstractplugin_services.h>

class AuthThread;

class Session : public QObject
{
    Q_OBJECT
private:
    bool m_auth;
    QTimer m_authTimer;
	QDateTime m_authSuccess;
	QDateTime m_lastAction;
    QString m_user;
	QString m_sessionid;
public:
    Session();
    ~Session();
	// authetification
    QString user() ;
    void setAuth(const QString& user) ;
    bool auth() ;
private Q_SLOTS:
    void timeout() ;
Q_SIGNALS:
    void timeoutAuth();
	void timeoutSession();
};

/**
 * Manages websessions and login data. Implements the plugin interface
 * and process actions for login, logout and provides information about sessions
 * via properties.
 */
class SessionController : AbstractPlugin_services {
    Q_OBJECT
public:
    SessionController();
    virtual ~SessionController();
	Session* getSession(const QString& sessionid);

	// plugin interface
    virtual bool condition(const QVariantMap& data);
    virtual void event_changed(const QVariantMap& data);
    virtual void execute(const QVariantMap& data);
    virtual QMap< QString, QVariantMap > properties(const QString& sessionid);
	
private:
    AuthThread* m_auththread;
	QMap<QString, Session*> m_session;

private Q_SLOTS:
    void auth_success(QString sessionid, const QString& name);
    void auth_failed(QString sessionid, const QString& name);
    void timeoutAuth(QString sessionid);
	void sessionBegin(QString sessionid);
	void sessionFinished(QString sessionid);
};
