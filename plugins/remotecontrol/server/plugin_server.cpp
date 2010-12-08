#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "shared/server/executeservice.h"
#include "plugin.h"
#include <qdbusservicewatcher.h>
#include <QDBusConnection>
#include <qdbusinterface.h>
#include <qdbusreply.h>
#include <qdom.h>
#include "services_server/eventremotekeyServer.h"
#include "services/eventremotekey.h"
#include <qdbusconnectioninterface.h>
#include "lirid_proxy.h"
#include "lirid_control_proxy.h"

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)
#define LIRI_DBUS_SERVICE "org.liri.Devices"
#define LIRI_DBUS_OBJECT_RECEIVERS "/org/liri/Devicelist"

myPluginExecute::myPluginExecute() : ExecutePlugin() {
    m_base = new myPlugin();
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

myPluginExecute::~myPluginExecute() {
    //delete m_base;
    delete m_control;
    qDeleteAll(m_devices.values());
}

void myPluginExecute::refresh() {}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == EventRemoteKey::staticMetaObject.className()) {
        return new EventRemoteKeyServer((EventRemoteKey*)service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    QList<AbstractStateTracker*> a;
    m_statetracker->setConnected(m_control!=0);
    m_statetracker->setReceivers(m_devices.size());
    a.append(m_statetracker);
    return a;
}


void myPluginExecute::deviceAdded(const QString& rid) {
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

    m_statetracker->setConnected(m_control!=0);
    m_statetracker->setReceivers(m_devices.size());
    emit stateChanged(m_statetracker);
}

void myPluginExecute::deviceRemoved(const QString& rid) {
    // only do something if this rid is in our map
    QMap< QString, OrgLiriDeviceInterface* >::iterator it = m_devices.find(rid);
    if (it == m_devices.end()) return;

    // remove
    delete *it;
    m_devices.erase(it);
    
    m_statetracker->setConnected(m_control!=0);
    m_statetracker->setReceivers(m_devices.size());
    emit stateChanged(m_statetracker);
}

void myPluginExecute::slotServiceUnregistered(const QString&) {
    // free m_control interface
    delete m_control;
    m_control = 0;

    // remove all entries one after the other
    QStringList rids = m_devices.keys();
    foreach(QString rid, rids) deviceRemoved(rid);
}

void myPluginExecute::slotServiceRegistered(const QString&) {
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

QStringList myPluginExecute::parseIntrospect() {
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

void myPluginExecute::keySlot(const QString &t, const QString &keyname, uint channel, int pressed)
{
    if (m_repeating) m_repeating->stopRepeat();
    QList<EventRemoteKeyServer*> providers = m_keyevents.value(keyname);

    foreach (EventRemoteKeyServer* e, providers) {
        e->keySlot(t,keyname,channel,pressed);
    }

    if (keyname.isEmpty()) return;

    m_statetrackerKey->setKey(keyname);
    m_statetrackerKey->setChannel(channel);
    m_statetrackerKey->setPressed(pressed);
    emit stateChanged(m_statetrackerKey);
}

void myPluginExecute::keyEventDestroyed(QObject* obj) {
	if (m_keyevents.isEmpty()) return;
	EventRemoteKeyServer* event = qobject_cast<EventRemoteKeyServer*>(obj);
	if (!event) return;
    EventRemoteKey* base = (EventRemoteKey*)event->base();
    Q_ASSERT(base);
	qDebug()<<__FUNCTION__<<base;
	QMap<QString, QList<EventRemoteKeyServer*> >::iterator it = m_keyevents.find(base->key());
    it.value().removeAll(event);
    if (it.value().isEmpty())
      m_keyevents.remove(base->key());
    //qDebug() << "remove" << k << r;
}

void myPluginExecute::registerKeyEvent(EventRemoteKeyServer* event) {
    EventRemoteKey* base = (EventRemoteKey*)event->base();
    // first remove if already registered
    QMap<QString, QList<EventRemoteKeyServer*> >::iterator it = m_keyevents.begin();
    for (;it != m_keyevents.end();++it) {
        QList<EventRemoteKeyServer*> list = it.value();
        for (int i=list.size()-1;i>=0;--i) {
            if (list[i] == event) {
                // remove old key
                m_keyevents.erase(it);
                goto endloop;
            }
        }
    }
    endloop:
    // register
    m_keyevents[base->key()].append(event);
    connect(event,SIGNAL(destroyed(QObject*)),SLOT(keyEventDestroyed(QObject*)));
}

bool myPluginExecute::isRegistered(EventRemoteKeyServer* event) {
    EventRemoteKey* base = (EventRemoteKey*)event->base();
    QList<EventRemoteKeyServer*> list = m_keyevents.value(base->key());
    return list.contains(event);
}
void myPluginExecute::clear() {
	m_keyevents.clear();
}
