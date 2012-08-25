#include "databaselistener.h"
#include <QDebug>
#include <time.h>
#include <QFileSystemWatcher>
#include <QSocketNotifier>
#include <qvarlengtharray.h>
#include <qfile.h>
#include <QDir>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <unistd.h>

DataStorageWatcher::DataStorageWatcher(QObject* parent) : QObject(parent), m_enabled(false)
{
	m_inotify_fd = inotify_init();
	if ( m_inotify_fd <= 0 ) {
		
	}
	fcntl(m_inotify_fd, F_SETFD, FD_CLOEXEC);
	QSocketNotifier* mSn = new QSocketNotifier( m_inotify_fd, QSocketNotifier::Read, this );
	connect(mSn, SIGNAL(activated(int)), this, SLOT(readnotify()));
	mSn->setEnabled(true);
}

DataStorageWatcher::~DataStorageWatcher()
{
	foreach (int id, pathToID) {
		inotify_rm_watch(m_inotify_fd, id);
	}
	close(m_inotify_fd);
}

void DataStorageWatcher::readnotify() {
	QSocketNotifier* mSn = (QSocketNotifier*)sender();
	mSn->setEnabled(false);
	
	    int buffSize = 0;
    ioctl(m_inotify_fd, FIONREAD, (char *) &buffSize);
    QVarLengthArray<char, 4096> buffer(buffSize);
    buffSize = read(m_inotify_fd, buffer.data(), buffSize);
    char *at = buffer.data();
    char * const end = at + buffSize;

    QHash<int, inotify_event *> eventForId;
    while (at < end) {
        inotify_event *event = reinterpret_cast<inotify_event *>(at);

        if (eventForId.contains(event->wd))
            eventForId[event->wd]->mask |= event->mask;
        else
            eventForId.insert(event->wd, event);

        at += sizeof(inotify_event) + event->len;
    }

    QHash<int, inotify_event *>::const_iterator it = eventForId.constBegin();
    while (it != eventForId.constEnd()) {
        const inotify_event &event = **it;
        ++it;

        int id = event.wd;
        QString path = idToPath.value(id);
        if (path.isEmpty()) {
			continue;
        }


        if ((event.mask & (IN_DELETE_SELF | IN_MOVE_SELF | IN_UNMOUNT)) != 0) {
            pathToID.remove(path);
            idToPath.remove(id);
            inotify_rm_watch(m_inotify_fd, event.wd);
			// dir removed; do nothing
        } else {
			// emit something
			//qDebug() << "inotify " << path << event.name;
			if (event.mask & IN_CREATE)
				emit fileChanged(QDir(path).absoluteFilePath(QString::fromUtf8(event.name)));
			else if (event.mask & IN_CLOSE_WRITE)
				emit fileChanged(QDir(path).absoluteFilePath(QString::fromUtf8(event.name)));
			else if (event.mask & IN_DELETE)
				emit fileRemoved(QDir(path).absoluteFilePath(QString::fromUtf8(event.name)));
        }
    }
    
	mSn->setEnabled(true);
}

void DataStorageWatcher::watchdir(const QString& dir) {
	if (!m_enabled || pathToID.contains(dir))
		return;
	
	int wd = inotify_add_watch(m_inotify_fd, QFile::encodeName(dir), (IN_CLOSE_WRITE | IN_MOVE | IN_CREATE | IN_DELETE | IN_DELETE_SELF ));
	if (wd <= 0) {
        qWarning("DataStorageWatcher::watchdir: inotify_add_watch failed");
		return;
	}
	pathToID.insert(dir, wd);
	idToPath.insert(wd, dir);
}

void DataStorageWatcher::setEnabled ( bool enabled ) {
	m_enabled = enabled;
}


