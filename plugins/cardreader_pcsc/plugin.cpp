#include "plugin_server.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QtPlugin>
#include "shared/server/executeservice.h"
#include "plugin.h"
#include "services/eventcardreader.h"
#include "services_server/eventcardreaderServer.h"
#include <statetracker/cardreaderpcscstatetracker.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define TIMEOUT 1000	/* 1 second timeout */

Q_EXPORT_PLUGIN2(libexecute, myPluginExecute)

myPluginExecute::myPluginExecute() : ExecutePlugin() {
    m_base = new myPlugin();
    m_cardreader = new CardReaderPCSCStateTracker();
    m_thread = new CardThread();
    connect(m_thread,SIGNAL(cardDetected(QString,int)),SIGNAL(cardDetected(QString,int)));
    connect(m_thread,SIGNAL(cardDetected(QString,int)),SLOT(slotcardDetected(QString,int)));
    m_thread->start();
}

myPluginExecute::~myPluginExecute() {
    m_thread->abort();
    m_thread->wait();
    delete m_thread;
    delete m_cardreader;
    //delete m_base;
}

void EventCardReaderServer::cardDetected(const QString& atr, int state) {
	EventCardReader* base = service<EventCardReader>();
	if (atr == base->atr() && state == base->state()) emit trigger();
}

void myPluginExecute::refresh() {}

ExecuteWithBase* myPluginExecute::createExecuteService(const QString& id)
{
    AbstractServiceProvider* service = m_base->createServiceProvider(id);
    if (!service) return 0;
    QByteArray idb = id.toAscii();
    if (idb == EventCardReader::staticMetaObject.className()) {
        return new EventCardReaderServer((EventCardReader*)service, this);
    }
    return 0;
}

QList<AbstractStateTracker*> myPluginExecute::stateTracker() {
    QList<AbstractStateTracker*> a;
    a.append(m_cardreader);
    return a;
}

void myPluginExecute::slotcardDetected(const QString& atr, int state) {
    m_cardreader->setState(state);
    m_cardreader->setAtr(atr);
    emit stateChanged(m_cardreader);
    //qDebug()<<"CardReader:"<<atr<<state;
}

CardThread::CardThread(QObject* parent) : QThread(parent), m_systemReady(false) {
    rgReaderStates_t = 0;
    dwReaders = 0;
    mszReaders = 0;
    ptr = 0;
    readers = 0;
    pnp = true;
	atr[0] = '\0';

    rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, 0, 0, &hContext);
    if (rv != SCARD_S_SUCCESS) {
        (void)SCardReleaseContext(hContext);
        return;
    }

    rgReaderStates[0].szReader = "\\\\?PnP?\\Notification";
    rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;

    rv = SCardGetStatusChange(hContext, 0, rgReaderStates, 1);
    if (rgReaderStates[0].dwEventState && SCARD_STATE_UNKNOWN)
    {
        pnp = false;
    }

    m_systemReady = true;
}

CardThread::~CardThread() {
    /* If we get out the loop, GetStatusChange() was unsuccessful */
    if (rv != SCARD_S_SUCCESS) {
        (void)SCardReleaseContext(hContext);
        m_systemReady = false;
        return;
    }

    /* We try to leave things as clean as possible */
    rv = SCardReleaseContext(hContext);
    if (rv != SCARD_S_SUCCESS) {
        (void)SCardReleaseContext(hContext);
        m_systemReady = false;
        return;
    }

    /* free memory possibly allocated */
    if (0 != readers)
        free(readers);
    if (0 != rgReaderStates_t)
        free(rgReaderStates_t);
}

void CardThread::readReaders() {    /* free memory possibly allocated in a previous loop */
    if (0 != readers)
    {
        free(readers);
        readers = 0;
    }

    if (0 != rgReaderStates_t)
    {
        free(rgReaderStates_t);
        rgReaderStates_t = 0;
    }

    /* Retrieve the available readers list.
     *
     * 1. Call with a null buffer to get the number of bytes to allocate
     * 2. malloc the necessary storage
     * 3. call with the real allocated buffer
     */
    rv = SCardListReaders(hContext, 0, 0, &dwReaders);
    if (rv != SCARD_E_NO_READERS_AVAILABLE)
        if (rv != SCARD_S_SUCCESS) {
            (void)SCardReleaseContext(hContext);
            m_systemReady = false;
            return;
        }
    dwReadersOld = dwReaders;

    /* if non 0 we came back so free first */
    if (mszReaders)
    {
        free(mszReaders);
        mszReaders = 0;
    }

    mszReaders = (char*)malloc(sizeof(char)*dwReaders);
    if (mszReaders == 0)
    {
        m_systemReady = false;
        return;
    }

    *mszReaders = '\0';
    rv = SCardListReaders(hContext, 0, mszReaders, &dwReaders);

    /* Extract readers from the null separated string and get the total
     * number of readers */
    nbReaders = 0;
    ptr = mszReaders;
    while (*ptr != '\0')
    {
        ptr += strlen(ptr)+1;
        nbReaders++;
    }
}
void CardThread::run() {
    if (!m_systemReady) return;

    readReaders();
    if (!m_systemReady) return;

    if (SCARD_E_NO_READERS_AVAILABLE == rv || 0 == nbReaders)
    {
        if (pnp)
        {
            rgReaderStates[0].szReader = "\\\\?PnP?\\Notification";
            rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;

            rv = SCardGetStatusChange(hContext, INFINITE, rgReaderStates, 1);
            if (rv != SCARD_S_SUCCESS) {
                (void)SCardReleaseContext(hContext);
                return;
            }
        }
        else
        {
            rv = SCARD_S_SUCCESS;
            while ((SCARD_S_SUCCESS == rv) && (dwReaders == dwReadersOld))
            {
                rv = SCardListReaders(hContext, 0, 0, &dwReaders);
                if (SCARD_E_NO_READERS_AVAILABLE == rv)
                    rv = SCARD_S_SUCCESS;
                sleep(1);
            }
        }
        readReaders();
    }
    else if (rv != SCARD_S_SUCCESS) {
        (void)SCardReleaseContext(hContext);
        return;
    }

    /* allocate the readers table */
    readers = (char**)calloc(nbReaders+1, sizeof(char *));
    if (0 == readers)
    {
        return;
    }

    /* fill the readers table */
    nbReaders = 0;
    ptr = mszReaders;
    while (*ptr != '\0')
    {
        readers[nbReaders] = ptr;
        ptr += strlen(ptr)+1;
        nbReaders++;
    }

    /* allocate the ReaderStates table */
    rgReaderStates_t = (SCARD_READERSTATE*)calloc(nbReaders+1, sizeof(* rgReaderStates_t));
    if (0 == rgReaderStates_t)
    {
        return;
    }

    /* Set the initial states to something we do not know
     * The loop below will include this state to the dwCurrentState
     */
    for (i=0; i<nbReaders; i++)
    {
        rgReaderStates_t[i].szReader = readers[i];
        rgReaderStates_t[i].dwCurrentState = SCARD_STATE_UNAWARE;
    }
    rgReaderStates_t[nbReaders].szReader = "\\\\?PnP?\\Notification";
    rgReaderStates_t[nbReaders].dwCurrentState = SCARD_STATE_UNAWARE;

    /* Wait endlessly for all events in the list of readers
     * We only stop in case of an error
     */
    if (pnp)
    {
        timeout = INFINITE;
        nbReaders++;
    }
    else
        timeout = TIMEOUT;
    rv = SCardGetStatusChange(hContext, timeout, rgReaderStates_t, nbReaders);
    while (m_systemReady && ((rv == SCARD_S_SUCCESS) || (rv == SCARD_E_TIMEOUT)))
    {
        if (pnp)
        {
            if (rgReaderStates_t[nbReaders-1].dwEventState &
                    SCARD_STATE_CHANGED)
            {
                readReaders();
                if (!m_systemReady) return;
            }
        }
        else
        {
            /* A new reader appeared? */
            if ((SCardListReaders(hContext, 0, 0, &dwReaders)
                    == SCARD_S_SUCCESS) && (dwReaders != dwReadersOld))
            {
                readReaders();
                if (!m_systemReady) return;
            }
        }

        /* Now we have an event, check all the readers in the list to see what
         * happened */
        for (current_reader=0; current_reader < nbReaders; current_reader++)
        {
            if (rgReaderStates_t[current_reader].dwEventState &
                    SCARD_STATE_CHANGED)
            {
                /* If something has changed the new state is now the current
                 * state */
                rgReaderStates_t[current_reader].dwCurrentState =
                    rgReaderStates_t[current_reader].dwEventState;
            }
            else
                /* If nothing changed then skip to the next reader */
                continue;

            if (rgReaderStates_t[current_reader].dwEventState &
                    SCARD_STATE_IGNORE)
                continue;

            if (rgReaderStates_t[current_reader].dwEventState &
                    SCARD_STATE_UNKNOWN)
            {
                readReaders();
                if (!m_systemReady) return;
            }

            int cardstate = 2;

            if (rgReaderStates_t[current_reader].dwEventState &
                    SCARD_STATE_EMPTY)
                cardstate = 0;

            else if (rgReaderStates_t[current_reader].dwEventState &
                     SCARD_STATE_PRESENT)
                cardstate = 1;

            else if (rgReaderStates_t[current_reader].dwEventState &
                     SCARD_STATE_MUTE)
                cardstate = 2;

            else if (rgReaderStates_t[current_reader].dwEventState &
                     SCARD_STATE_UNAVAILABLE)
                cardstate = 2;

//             if (rgReaderStates_t[current_reader].dwEventState &
//                     SCARD_STATE_ATRMATCH)
//                 printf("ATR matches card, ");

//             if (rgReaderStates_t[current_reader].dwEventState &
//                     SCARD_STATE_EXCLUSIVE)
//                 printf("Exclusive Mode, ");

//             if (rgReaderStates_t[current_reader].dwEventState &
//                     SCARD_STATE_INUSE)
//                 printf("Shared Mode, ");

            /* Also dump the ATR if available */
            if (cardstate==1) atr[0] = '\0';
            if (rgReaderStates_t[current_reader].cbAtr  > 0)
            {
                for (i=0; i<(int)rgReaderStates_t[current_reader].cbAtr; i++)
                    sprintf(&atr[i*3], "%02X ",
                            rgReaderStates_t[current_reader].rgbAtr[i]);

                atr[i*3-1] = '\0';
            }
            const QString atrstr = QString::fromAscii(atr);
			if (atrstr.size())
				emit cardDetected(atrstr, cardstate);
        } /* for */

        rv = SCardGetStatusChange(hContext, timeout, rgReaderStates_t,
                                  nbReaders);
    } /* while */
}

void CardThread::abort() {
    m_systemReady = false;
    SCardCancel(hContext);
}
