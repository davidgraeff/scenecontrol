/*
 * Network Controller
 *
 *  Created on: 10.02.2010
 *      Author: David Gräff
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
