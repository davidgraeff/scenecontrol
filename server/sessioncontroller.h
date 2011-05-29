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
#undef PLUGIN_ID
#define PLUGIN_ID "sessioncontroller"
#include <shared/abstractplugin_services.h>
#include "shared/pluginservicehelper.h"
#include <shared/abstractplugin.h>

class AuthThread;
class SessionController;

/**
 * A session object will only be established after a successful authentification of the user
 */
class Session : public QObject
{
    Q_OBJECT
private:
    SessionController* m_sessioncontroller;
    QTimer m_sessionTimer;
    QDateTime m_sessionStarted;
    QDateTime m_lastAction;
    QString m_sessionid;
    QString m_user;
public:
    Session(SessionController* sc, const QString& sessionid, const QString& user);
    ~Session();
    void resetSessionTimer();
    QString user();
    QDateTime sessionStarted();
    QDateTime lastAction();
    QString sessionid();
private Q_SLOTS:
    void timeoutSession();
};

/**
 * Manages websessions and login data. Implements the plugin interface
 * and process actions for login, logout and provides information about sessions
 * via properties.
 */
class SessionController : public QObject, public AbstractPlugin, public AbstractPlugin_services {
    Q_OBJECT
    PLUGIN_MACRO
public:
    static SessionController* instance(bool create = false);
    virtual ~SessionController();
    virtual void clear() {}
    virtual void initialize() {}

	int tryLogin(const QVariantMap& data, QString& sessionid);
    Session* getSession(const QString& sessionid);
    /**
     * \return Return temporary session id
     */
    QString addSession(const QString& user, const QString& pwd);
    void closeSession(const QString& sessionid, bool timeout);
	
    // plugin interface
    virtual bool condition(const QVariantMap& data, const QString& sessionid);
    virtual void register_event ( const QVariantMap& data, const QString& collectionuid );
	virtual void unregister_event ( const QVariantMap& data, const QString& collectionuid );
    virtual void execute(const QVariantMap& data, const QString& sessionid);
    QList< QVariantMap > properties(const QString& sessionid);

private:
    static SessionController* m_sessioncontroller;
    SessionController();
    AuthThread* m_auththread;
    QMap<QString, Session*> m_session;

private Q_SLOTS:
    void auth_success(QString sessionid, const QString& name);
    void auth_failed(QString sessionid, const QString& name);
Q_SIGNALS:
    void sessionAuthFailed(QString sessionid);
    void sessionBegin(QString sessionid);
    void sessionFinished(QString sessionid, bool timeout);
};
