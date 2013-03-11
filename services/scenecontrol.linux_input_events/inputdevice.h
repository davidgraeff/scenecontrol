#pragma once
#include <QObject>
#include <QSet>
#include <QTimer>
#include <QSocketNotifier>
#include <QMap>

class plugin;
class ManagedDevice;

struct EventKey {
	QMap<QString, QString> ServiceUidToCollectionUid;
	bool repeat;
};

class InputDevice : public QObject {
	Q_OBJECT
private:
	plugin* m_plugin;
	QSocketNotifier* m_socketnotifier;
	int fd;
	ManagedDevice* m_device;
	QSet<int> m_sessionids;
	QMap<QByteArray, EventKey*> m_keyToUids;
	QTimer m_stopRepeatTimer;
	QByteArray m_lastkey;
public:
	InputDevice(plugin* plugin) ;
	~InputDevice();
	bool isClosable();
	void connectSession(int sessionid);
	void disconnectSession(int sessionid);
	void setDevice(ManagedDevice* device);
	void connectDevice();
	void disconnectDevice();
	void unregisterKey(const QByteArray& uid);
	void registerKey( QString uid, QString collectionuid, const QByteArray& key, bool repeat);
	ManagedDevice* device();
private Q_SLOTS:
	void eventData();
};
