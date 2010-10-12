/****************************************************************************
** This file is part of the linux remotes project
**
** Use this file under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include "remotecontroller.h"
#include "lirid_control_proxy.h"
#include "lirid_proxy.h"

#include <QSet>
#include <QStringList>
/* dbus */
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>
#include <QtXml/QtXml>
#include "stateTracker/abstractstatetracker.h"
#include <stateTracker/remotecontrolstatetracker.h>
#include <stateTracker/remotecontrolkeystatetracker.h>
#include <backtrace.h>

RemoteController::RemoteController(QObject* parent) : QObject(parent) {
    m_control = 0;
	m_repeating = 0;

    m_statetracker = new RemoteControlStateTracker();
    m_statetrackerKey = new RemoteControlKeyStateTracker();

    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(QLatin1String(LIRI_DBUS_SERVICE),
            QDBusConnection::systemBus(),
            QDBusServiceWatcher::WatchForOwnerChange,
            this);
    connect (watcher, SIGNAL(serviceUnregistered(const QString&)),
             this, SLOT(slotServiceUnregistered(const QString&)));

    connect (watcher, SIGNAL(serviceRegistered(const QString&)),
             this, SLOT(slotServiceRegistered(const QString&)));

    if (watcher->connection().interface()->isServiceRegistered(QLatin1String(LIRI_DBUS_SERVICE)))
        slotServiceRegistered(QLatin1String(LIRI_DBUS_SERVICE));
}

RemoteController::~RemoteController() {
    delete m_control;
    qDeleteAll(m_devices.values());
}

QList<AbstractStateTracker*> RemoteController::getStateTracker()
{
    QList<AbstractStateTracker*> l;
    l.append(m_statetracker);
    l.append(m_statetrackerKey);
    return l;
}

bool RemoteController::isConnected() const
{
    return (m_control != 0);
}

int RemoteController::countReceiver() const
{
    return m_devices.size();
}

QString RemoteController::mode() const
{
    return m_mode;
}

void RemoteController::setMode(const QString& mode)
{
    m_mode = mode;
    m_statetracker->sync();
}

void RemoteController::deviceAdded(const QString& rid) {
    // be on the safe side, although this slot should only be called if these
    // prerequires are fulfilled anyway
    if (!m_control) return;

    // is this a duplicate?
    if (m_devices.contains(rid)) return;

    // create device-manager device bus object. Bus objectpath: /org/liri/Devicelist/N
    QString devicename = QLatin1String(LIRI_DBUS_OBJECT_RECEIVERS"/") + rid;
    OrgLiriDeviceInterface* tmp;
    tmp = new OrgLiriDeviceInterface(
        QLatin1String(LIRI_DBUS_SERVICE),
        devicename,
        QDBusConnection::systemBus() );

    // connection established?
    if (!tmp->isValid()) {
        qWarning() << devicename << "Interface invalid" << tmp->lastError();
        delete tmp;
        tmp = 0;
        return;
    }

    // create new entry
    m_devices[rid] = tmp;

    // connect
    connect(tmp,SIGNAL(key(QString,QString,uint,int)),SLOT(keySlot(QString,QString,uint,int)));

    m_statetracker->sync();
}

void RemoteController::deviceRemoved(const QString& rid) {
    // only do something if this rid is in our map
    QMap< QString, OrgLiriDeviceInterface* >::iterator it = m_devices.find(rid);
    if (it == m_devices.end()) return;

    // remove
    delete *it;
    m_devices.erase(it);

    m_statetracker->sync();
}

void RemoteController::slotServiceUnregistered(const QString&) {
    // free m_control interface
    delete m_control;
    m_control = 0;

    // remove all entries one after the other
    QStringList rids = m_devices.keys();
    foreach(QString rid, rids) deviceRemoved(rid);
}

void RemoteController::slotServiceRegistered(const QString&) {
    // create m_control object
    m_control = new OrgLiriControlInterface(
        QLatin1String(LIRI_DBUS_SERVICE),
        QLatin1String(LIRI_DBUS_OBJECT_RECEIVERS),
        QDBusConnection::systemBus());

    // connection established?
    if (!m_control->isValid()) {
        qWarning() << "m_control Interface invalid" <<
        m_control->lastError();
        delete m_control;
        m_control = 0;
        return;
    }

    // connect signals
    connect(m_control, SIGNAL ( deviceAdded(const QString &) ),
            SLOT( deviceAdded(const QString &) ));

    connect(m_control, SIGNAL ( deviceRemoved(const QString &) ),
            SLOT( deviceRemoved(const QString &) ));

    // add m_devices through parsing dbus introsspect xml
    QStringList list = parseIntrospect();
    foreach(QString rid, list) deviceAdded(rid);
}

QStringList RemoteController::parseIntrospect() {
    QStringList tmp;
    QDBusInterface iface(QLatin1String(LIRI_DBUS_SERVICE), QLatin1String(LIRI_DBUS_OBJECT_RECEIVERS),
                         QLatin1String("org.freedesktop.DBus.Introspectable"), QDBusConnection::systemBus());
    if (!iface.isValid()) {
        QDBusError err(iface.lastError());
        qDebug() << "Cannot introspect child nodes (m_devices): interface invalid";
        return tmp;
    }

    QDBusReply<QString> xml = iface.call(QLatin1String("Introspect"));
    if (!xml.isValid()) {
        QDBusError err(xml.error());
        if (err.isValid()) {
            qDebug() << "Cannot introspect child nodes (m_devices): Call to object failed";
        } else {
            qDebug() << "Cannot introspect child nodes (m_devices): Invalid XML";
        }
        return tmp;
    }

    QDomDocument doc;
    doc.setContent(xml);

    QDomElement node = doc.documentElement();
    QDomElement child = node.firstChildElement();
    while (!child.isNull()) {
        QString name = child.attribute(QLatin1String("name"));
        if (child.tagName() == QLatin1String("node") && name[0].isNumber()) {
            tmp.append(name);
        }
        child = child.nextSiblingElement();
    }
    return tmp;
}

void RemoteController::keySlot(const QString &t, const QString &keyname, uint channel, int pressed)
{
	if (m_repeating) m_repeating->stopRepeat();
    QList<EventRemoteKey*> providers = m_keyevents.values(keyname);

    foreach (EventRemoteKey* e, providers) {
        e->keySlot(t,keyname,channel,pressed);
    }

    if (!pressed || keyname.isEmpty()) return;
    if (keyname == QLatin1String("*MUSIC") ||
            keyname == QLatin1String("*VIDEO") ||
            keyname == QLatin1String("*PHOTO") ||
            keyname == QLatin1String("*TV") ||
            keyname == QLatin1String("*RECTV"))
    {
        setMode(keyname);
    }
    m_statetrackerKey->setKey(keyname);
    m_statetrackerKey->sync();
}

void RemoteController::keyEventDestroyed(QObject* obj) {
    EventRemoteKey* k = qobject_cast<EventRemoteKey*>(obj);
    Q_ASSERT(k);
    m_keyevents.remove(k->key(),k);
    //qDebug() << "remove" << k << r;
}

void RemoteController::registerKeyEvent(EventRemoteKey* event) {
  qDebug() << __FUNCTION__ << event->key();// << getBackTrace().join(QLatin1String("\n"));
    // first remove if already registered
    QMultiMap<QString, EventRemoteKey*>::iterator it = m_keyevents.begin();
    for (;it != m_keyevents.end();++it)
        if (it.value() == event) {
            // Don't do anything if key stayed the same
            if (it.key() == event->key()) return;
            // remove old key
  qDebug() << __FUNCTION__ << "remove1" << m_keyevents.size();
            m_keyevents.remove(it.key(),it.value());
  qDebug() << __FUNCTION__ << "remove2" << m_keyevents.size();
            break;
        }
    // register
  qDebug() << __FUNCTION__ << "insert1" << m_keyevents.size();
    m_keyevents.insert(event->key(),event);
  qDebug() << __FUNCTION__ << "insert2" << m_keyevents.size();
    connect(event,SIGNAL(destroyed(QObject*)),SLOT(keyEventDestroyed(QObject*)));
}

bool RemoteController::isRegistered(EventRemoteKey* event) {
    return m_keyevents.contains(event->key(), event);
}
