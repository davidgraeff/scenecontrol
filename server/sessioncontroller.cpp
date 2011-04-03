#include "sessioncontroller.h"
#include "authThread.h"
#include <serializer.h>

SessionController* SessionController::m_sessioncontroller = 0;

SessionController* SessionController::instance(bool create) {
    if (!m_sessioncontroller && create) m_sessioncontroller = new SessionController();
	Q_ASSERT(m_sessioncontroller);
    return m_sessioncontroller;
}

Session::Session(SessionController* sc, const QString& sessionid, const QString& user) : m_sessioncontroller(sc), m_sessionid(sessionid), m_user(user) {
    m_sessionStarted=QDateTime::currentDateTime();
    m_lastAction=QDateTime::currentDateTime();
    connect(&m_sessionTimer, SIGNAL(timeout()),SIGNAL(timeoutSession()));
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
    if (IS_ID("sessionlogin")) { //nonsense!
        addSession(DATA("user"),DATA("pwd"));
    } else if (IS_ID("sessionlogout")) {
        closeSession(DATA("sessionid"));
    } else if (IS_ID("sessionidle")) {
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
    return l;
}

QString SessionController::addSession(const QString& user, const QString& pwd) {
    QString sessionid = QUuid::createUuid().toString();
    m_auththread->query(sessionid, user, pwd);
    return sessionid;
}

void SessionController::closeSession(const QString& sessionid) {
    Session* session = m_session.take(sessionid);
    if (!session) return;
    emit sessionFinished(sessionid, false);
    session->deleteLater();
}

void SessionController::auth_success(QString sessionid, const QString& name) {
    Session* session = new Session(this, sessionid, name);
    m_session.insert(sessionid, session);
    connect(session,SIGNAL(timeoutSession()),SLOT(timeoutSession()));
    emit sessionBegin(sessionid);
}

void SessionController::auth_failed(QString sessionid, const QString& name) {
    Q_UNUSED(name);
    emit sessionAuthFailed(sessionid);
}

void SessionController::timeoutSession() {
    Session* session = qobject_cast< Session* >(sender());
    if (!session) return;
    emit sessionFinished(session->sessionid(), true);
    m_session.remove(session->sessionid());
    session->deleteLater();
}

QByteArray SessionController::authFailed() {
    QJson::Serializer s;
    PROPERTY("authentification_failed");
    return s.serialize(data);
}
