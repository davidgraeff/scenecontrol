/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef myPLUGINSERVER_H
#define myPLUGINSERVER_H
#include <QObject>
#include <QStringList>
#include "shared/server/executeplugin.h"
#include <QThread>

#ifdef __APPLE__
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
#else
#include <winscard.h>
#endif

class CardThread : public QThread
{
    Q_OBJECT
public:
    explicit CardThread(QObject* parent = 0);
    virtual ~CardThread();
	virtual void run();
	void abort();
private:
    bool m_systemReady;
    void readReaders();
	int current_reader;
	LONG rv;
	SCARDCONTEXT hContext;
	SCARD_READERSTATE *rgReaderStates_t;
	SCARD_READERSTATE rgReaderStates[1];
	DWORD dwReaders;
	DWORD dwReadersOld;
	DWORD timeout;
	LPSTR mszReaders;
	char *ptr;
	char **readers;
	int nbReaders, i;
	char atr[MAX_ATR_SIZE*3+1];
	bool pnp;
Q_SIGNALS:
    void cardDetected(const QString& atr, int state);
};

class CardReaderPCSCStateTracker;
class myPluginExecute : public ExecutePlugin
{
    Q_OBJECT
    Q_INTERFACES(ExecutePlugin)
public:
    myPluginExecute();
    virtual ~myPluginExecute();
    virtual void refresh() ;
    virtual ExecuteWithBase* createExecuteService(const QString& id);
    virtual QList<AbstractStateTracker*> stateTracker();
    virtual AbstractPlugin* base() {
        return m_base;
    }
    virtual void clear() {}
private:
    AbstractPlugin* m_base;
    CardReaderPCSCStateTracker* m_cardreader;
    CardThread* m_thread;
private Q_SLOTS:
    void slotcardDetected(const QString& atr, int state);
Q_SIGNALS:
    void cardDetected(const QString& atr, int state);
};

#endif // myPLUGINSERVER_H
