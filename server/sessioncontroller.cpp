#include "sessioncontroller.h"
#include "authThread.h"
#include <serializer.h>
#include <shared/abstractserver.h>
#include <QDebug>

SessionController* SessionController::m_sessioncontroller = 0;

SessionController* SessionController::instance(bool create) {
    if (!m_sessioncontroller && create) m_sessioncontroller = new SessionController();
    Q_ASSERT(m_sessioncontroller);
    return m_sessioncontroller;
}

Session::Session(SessionController* sc, const QString& sessionid, const QString& user) : m_sessioncontroller(sc), m_sessionid(sessionid), m_user(user) {
    m_sessionStarted=QDateTime::currentDateTime();
    m_lastAction=QDateTime::currentDateTime();
    connect(&m_sessionTimer, SIGNAL(timeout()),SLOT(timeoutSession()));
    m_sessionTimer.setSingleShot(true);
    m_sessionTimer.setInterval(1000*60*5);
    m_sessionTimer.start();
}

Session::~Session() {}

QString Session::user() {
    return m_user;
}

QDateTime Session::sessionStarted() {
    return m_sessionStarted;
}

QDateTime Session::lastAction() {
    return m_lastAction;
}

void Session::resetSessionTimer() {
    m_sessionTimer.stop();
    m_sessionTimer.start();
    m_lastAction=QDateTime::currentDateTime();
}

QString Session::sessionid() {
    return m_sessionid;
}

void Session::timeoutSession() {
    m_sessioncontroller->closeSession(m_sessionid, true);
}

SessionController::SessionController() : m_auththread(new AuthThread()) {
    connect(m_auththread,SIGNAL(auth_failed(QString,QString)),SLOT(auth_failed(QString,QString)));
    connect(m_auththread,SIGNAL(auth_success(QString,QString)),SLOT(auth_success(QString,QString)));
    m_auththread->start();
}

SessionController::~SessionController() {
    m_auththread->blockSignals(true);
    m_auththread->stop();
    if (!m_auththread->wait(1000*5))
        m_auththread->terminate();
    delete m_auththread;
}

Session* SessionController::getSession(const QString& sessionid) {
    return m_session.value(sessionid);
}

bool SessionController::condition(const QVariantMap& data, const QString& sessionid) {
    Q_UNUSED(data);
    Q_UNUSED(sessionid);
    return false;
}

void SessionController::execute(const QVariantMap& data, const QString& sessionid) {
    Q_UNUSED(sessionid);
    if (ServiceID::isId(data,"sessionlogout")) {
        closeSession(sessionid, false);
    }
}

void SessionController::register_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED(data);
    Q_UNUSED(collectionuid);
}

void SessionController::unregister_event ( const QVariantMap& data, const QString& collectionuid ) {
    Q_UNUSED(data);
    Q_UNUSED(collectionuid);
}

QList<QVariantMap> SessionController::properties(const QString& sessionid) {
    QList<QVariantMap> l;
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "server.logins.all");
        sc.setData("all", m_session.size());
        l.append(sc.getData());
    }

    {
        ServiceCreation sc = ServiceCreation::createModelReset(PLUGIN_ID, "server.logins.own", "endpoint");
        m_server->property_changed(sc.getData(),sessionid);
    }

    Session* ownsession = m_session.value(sessionid);
    if (ownsession) {
        QList<QString> sessions_with_same_user;
        foreach(Session* session, m_session) {
            if (session->user() == ownsession->user()) {
                sessions_with_same_user.append(session->sessionid());
                ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "server.logins.own");
                sc.setData("endpoint", session->sessionid());
                m_server->property_changed(sc.getData(),sessionid);
            }
        }
    }
    return l;
}

bool SessionController::addSession(const QString& user, const QString& pwd, QString& sessionid) {
    sessionid = QUuid::createUuid().toString();
    return m_auththread->query(sessionid, user, pwd);
}

void SessionController::closeSession(const QString& sessionid, bool timeout) {
    Session* session = m_session.take(sessionid);
    if (!session) return;
    emit sessionFinished(sessionid, timeout);
    session->deleteLater();
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "server.logins.all");
        sc.setData("all", m_session.size());
        m_server->property_changed(sc.getData());
    }
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "authentification.revoked");
		sc.setData("timeout", timeout);
        m_server->property_changed(sc.getData(), sessionid);
    }
}

void SessionController::auth_success(QString sessionid, const QString& name) {
    Session* session = new Session(this, sessionid, name);
    m_session.insert(sessionid, session);

    emit sessionBegin(sessionid);

    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "authentification.success");
    sc.setData("sessionid", sessionid);
    m_server->property_changed(sc.getData(),sessionid);
}

void SessionController::auth_failed(QString sessionid, const QString& name) {
    Q_UNUSED(name);

    emit sessionAuthFailed(sessionid);

    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "authentification.failed");
    m_server->property_changed(sc.getData(),sessionid);
}

SessionController::SessionState SessionController::tryLoginAndResetSessionTimer(const QVariantMap& data, QString& sessionid) {
    if (ServiceID::pluginid(data)==QLatin1String(PLUGIN_ID) && ServiceID::isId(data,"sessionlogin")) { // login have to happen here
        if (addSession(DATA("user"),DATA("pwd"), sessionid))
			return SessionRequestValidation;
		else
			return SessionNewSessionDenied;;
    }

    Session* session = getSession(sessionid);
    if (!session)
        return SessionInValid;

    session->resetSessionTimer();

    return SessionValid;
}
