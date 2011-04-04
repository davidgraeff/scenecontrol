/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once
#include <QObject>
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
	QString getAtr() { return QString::fromAscii(atr); }
	int getState() {return cardstate; }
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
	int cardstate;
	bool pnp;
Q_SIGNALS:
    void cardDetected(const QString& atr, int state);
};
