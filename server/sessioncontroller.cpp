#include "sessioncontroller.h"
#include "authThread.h"
#include <serializer.h>
#include <shared/abstractserver.h>

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

bool SessionController::condition(const QVariantMap& data) {
    Q_UNUSED(data);
    return false;
}

void SessionController::execute(const QVariantMap& data) {
    if (ServiceID::isId(data,"sessionlogin")) { //nonsense!
        addSession(DATA("user"),DATA("pwd"));
    } else if (ServiceID::isId(data,"sessionlogout")) {
        closeSession(DATA("sessionid"), false);
    } else if (ServiceID::isId(data,"sessionidle")) {
        Session* session = m_session.value(DATA("sessionid"));
        if (session) session->resetSessionTimer();
    }
}

void SessionController::event_changed(const QVariantMap& data) {
    Q_UNUSED(data);
}

QList<QVariantMap> SessionController::properties(const QString& sessionid) {
    Q_UNUSED(sessionid);
    QList<QVariantMap> l;
    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "server.logins.all");
        sc.setData("all", m_session.size());
        l.append(sc.getData());
    }
    return l;
}

QString SessionController::addSession(const QString& user, const QString& pwd) {
    QString sessionid = QUuid::createUuid().toString();
    m_auththread->query(sessionid, user, pwd);
    return sessionid;
}

void SessionController::closeSession(const QString& sessionid, bool timeout) {
    Session* session = m_session.take(sessionid);
    if (!session) return;
    emit sessionFinished(sessionid, timeout);
    session->deleteLater();
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "server.logins.all");
    sc.setData("all", m_session.size());
    m_server->property_changed(sc.getData());
}

void SessionController::auth_success(QString sessionid, const QString& name) {
    Session* session = new Session(this, sessionid, name);
    m_session.insert(sessionid, session);

    {
        ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "authentification.success");
		sc.setData("sessionid", sessionid);
        m_server->property_changed(sc.getData(),sessionid);
    }

    emit sessionBegin(sessionid);

	{
        ServiceCreation sc = ServiceCreation::createModelReset(PLUGIN_ID, "server.logins.own");
        m_server->property_changed(sc.getData(),sessionid);
    }

	QList<QString> sessions_with_same_user;
    foreach(Session* session, m_session) {
        if (session->user() == name) {
            sessions_with_same_user.append(session->sessionid());
            ServiceCreation sc = ServiceCreation::createModelChangeItem(PLUGIN_ID, "server.logins.own");
            sc.setData("endpoint", session->sessionid());
            m_server->property_changed(sc.getData(),sessionid);
        }
    }
}

void SessionController::auth_failed(QString sessionid, const QString& name) {
    Q_UNUSED(name);
    emit sessionAuthFailed(sessionid);
    ServiceCreation sc = ServiceCreation::createNotification(PLUGIN_ID, "authentification.failed");
    m_server->property_changed(sc.getData(),sessionid);
}
