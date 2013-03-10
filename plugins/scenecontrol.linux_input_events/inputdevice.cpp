#include "inputdevice.h"
#include "managed_device_list.h"
#include "plugin.h"
#include "jsondocuments/scenedocument.h"
#include <linux/input.h>
#include <QFileInfo>
#include <unistd.h>
#include <fcntl.h>


#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_KEEPING_PRESSED 2


InputDevice::InputDevice ( plugin* plugin ) : m_plugin ( plugin ), m_socketnotifier ( 0 ), fd ( 0 ), m_device ( 0 ) {
	m_stopRepeatTimer.setSingleShot ( true );
	m_stopRepeatTimer.setInterval(150);
}

InputDevice::~InputDevice() {
	qDeleteAll(m_keyToUids);
	m_keyToUids.clear();
	disconnectDevice();
}

ManagedDevice* InputDevice::device() {
	return m_device;
}

bool InputDevice::isClosable() {
	return ( m_sessionids.isEmpty() && m_keyToUids.isEmpty() );
}

void InputDevice::connectSession ( int sessionid ) {
	// Cannot connect session: no device for this object known
	if ( !m_device )
		return;
	
	m_sessionids.insert ( sessionid );
	if ( !fd ) {
		connectDevice();
	}
}
void InputDevice::disconnectSession ( int sessionid ) {
	m_sessionids.remove ( sessionid );
	if ( m_sessionids.isEmpty() && m_keyToUids.isEmpty() ) {
		disconnectDevice();
	}
}

void InputDevice::disconnectDevice() {
	if ( fd ) close ( fd );
	fd = 0;
	delete m_socketnotifier;
	m_socketnotifier = 0;
}

void InputDevice::connectDevice() {
	if ( !m_device || m_socketnotifier) {
		return;
	}
	
	// only reconnect to new device if a client is actually listening or events are registered
	if ( (m_sessionids.isEmpty() && m_keyToUids.isEmpty()))
		return;
	
	SceneDocument sc = SceneDocument::createNotification ( "input.device.selected" );
	sc.setData ( "udid", m_device->udid );
	
	if ( !QFileInfo ( m_device->devPath ).isReadable() ) {
		// error
		sc.setData ( "listen", false );
		sc.setData ( "errormsg", QString ( QLatin1String ( "InputDevice " ) + m_device->devPath + QLatin1String ( " open failed. No access rights!" ) ) );
	} else {
		fd = open ( m_device->devPath, O_RDONLY|O_NDELAY );
		if ( fd!=-1 ) {
			if (!m_plugin->m_dontgrab)
				ioctl(fd, EVIOCGRAB, 1);
			m_socketnotifier = new QSocketNotifier ( fd, QSocketNotifier::Read );
			connect ( m_socketnotifier, SIGNAL ( activated ( int ) ), this, SLOT ( eventData() ) );
			// success
			sc.setData ( "listen", true );
			sc.setData ( "errormsg", QString() );
		} else {
			sc.setData ( "listen", false );
			sc.setData ( "errormsg", QString ( QLatin1String ( "InputDevice open failed: " ) + m_device->devPath ) );
		}
	}
	// Propagate to all interested clients
	foreach ( int sessionid, m_sessionids ) {
		m_plugin->changeProperty ( sc.getData(), sessionid );
	}
}

void InputDevice::setDevice ( ManagedDevice* device ) {
	m_device = device;
	disconnectDevice();
	connectDevice();
}

void InputDevice::unregisterKey ( const QByteArray& uid ) {
	QMutableMapIterator<QByteArray, EventKey* > it ( m_keyToUids );
	while ( it.hasNext() ) {
		it.next();
		it.value()->ServiceUidToCollectionUid.remove ( uid );
		if ( it.value()->ServiceUidToCollectionUid.isEmpty() )
			it.remove();
	}
	// disconnect if no one is listening anymore
	if ( m_sessionids.isEmpty() && m_keyToUids.isEmpty() )
		disconnectDevice();
}

void InputDevice::registerKey ( QString uid, QString collectionuid, const QByteArray& key, bool repeat ) {
	EventKey* eventkey = m_keyToUids[key];
	if (!eventkey)
		eventkey = new EventKey();
	eventkey->ServiceUidToCollectionUid.insert ( uid, collectionuid );
	eventkey->repeat = repeat;
	m_keyToUids[key] = eventkey;
	connectDevice();
}

void InputDevice::eventData() {
	static char readbuff[sizeof ( struct input_event ) ] = {0};
	static struct input_event* ev = ( struct input_event* ) readbuff;
	static unsigned int readbuffOffset = 0;
	__u16 lastkeyInLoop = 0;
	forever {
		int ret = read ( fd, readbuff+readbuffOffset, sizeof ( struct input_event )-readbuffOffset );
		if ( ret == -1 || ret == 0)
			break;
		
		readbuffOffset += ret;
		if ( readbuffOffset < sizeof ( struct input_event ) )
			break;
		
		readbuffOffset = 0;
		
		if ( ev->type != EV_KEY ) {
			continue;
		}
		
		// Only look at KEY_PRESS
		if (!ev->value == KEY_PRESS ) {
			continue;
		}
		
		// Multiple key events with the same key in one buffer; ignore
		if (lastkeyInLoop == ev->code)
			continue;
		lastkeyInLoop = ev->code;
		
		const QByteArray& kernelkeyname = m_plugin->m_keymapping.value ( ev->code );
		
		//qDebug() << "KEY_PRESS" << kernelkeyname;
		
		// properties
		if (m_sessionids.size()) {
			// last key property. Will be propagated to interested clients only.
			SceneDocument sc = SceneDocument::createNotification ( "input.device.key" );
			sc.setData ( "kernelkeyname", kernelkeyname );
			sc.setData ( "udid", m_device->udid );
			
			// Propagate to all interested clients
			foreach ( int sessionid, m_sessionids ) {
				m_plugin->changeProperty ( sc.getData(), sessionid );
			}
		}
		
		// Get EventKey, abort if no one is registered
		QMap<QByteArray, EventKey*>::iterator it = m_keyToUids.find ( kernelkeyname );
		if ( it == m_keyToUids.end() ) continue;
		
		const EventKey* event = *it;
		
		// If received key is the same as last key && the event is not repeatable &&
		// a sensible amount of time to the last received key is not exceeded: abort
		if (m_stopRepeatTimer.isActive() && !event->repeat && kernelkeyname == m_lastkey) {
			// Restart timer to filter out all ongoing events for this key (if repeat==false)
			m_stopRepeatTimer.stop();
			m_stopRepeatTimer.start();
			continue;
		}
		
		m_stopRepeatTimer.start();
		m_lastkey = kernelkeyname;
		
		//qDebug() << "KEY_PRESS FILTERED" << kernelkeyname;
		
		// event trigger
		{
			QMap<QString, QString>::const_iterator i = event->ServiceUidToCollectionUid.constBegin();
			for (;i!=event->ServiceUidToCollectionUid.constEnd();++i) {
				m_plugin->eventTriggered ( i.key(), i.value() );
			}
		}
	}
}
