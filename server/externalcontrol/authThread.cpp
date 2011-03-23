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

#include "authThread.h"
#ifdef _WIN32 
#define NTDDI_VERSION NTDDI_VISTA
//#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include "Windows.h"
#else
#include <security/pam_modules.h>
#include <security/pam_appl.h>
#include <sys/param.h>
#endif
#include <stdlib.h>
#include <QDebug>

char* pwdptr;

#ifndef _WIN32 
int su_conv(int num_msg,const struct pam_message **msgm,
            struct pam_response **resp,void *appdata)
{
	Q_UNUSED(appdata);
    int count;
    struct pam_response *r;

    r = (struct pam_response*)calloc(num_msg,sizeof(struct pam_response));

    for (count=0;count < num_msg;++count)
    {

        switch (msgm[count]->msg_style)
        {
        case PAM_PROMPT_ECHO_OFF:
            r[count].resp_retcode = 0;
            r[count].resp = strdup(pwdptr);
            break;
        case PAM_PROMPT_ECHO_ON:
            printf("%s",msgm[count]->msg);
            break;
        case PAM_ERROR_MSG:
            printf(" %s\n",msgm[count]->msg);
            break;
        case PAM_TEXT_INFO:
            printf(" %s\n",msgm[count]->msg);
            break;
        default:
            printf("Erroneous Conversation (%d)\n"
                   ,msgm[count]->msg_style);

        }
    }
    *resp = r;
    return PAM_SUCCESS;
}
#endif

bool AuthThread::query(const QString& sessionid, const QString& name, const QString& pwd) {
    bool ok = false;
    mutex.lock();
    if (m_creds.size()<MAX_SIMULTANEOUS_LOGINS) {
        m_creds.append(new AuthQueryData(sessionid, name.toUtf8(), pwd.toUtf8()));
        ok = true;
    }
    mutex.unlock();
    if (ok) bufferNotFull.wakeAll();
    return ok;
}

void AuthThread::run() {
    while (1) {
        // get next request
        mutex.lock();
        while (m_creds.size()==0) bufferNotFull.wait(&mutex);
        AuthQueryData* p = m_creds.takeFirst();
        mutex.unlock();

        // check user
        char *user = (char*)p->name.constData();
        pwdptr = (char*)p->pwd.constData();

#ifdef _WIN32
		HANDLE token = 0;
		LogonUser(user, (char*)".", pwdptr, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &token);
		if (token)
			emit auth_success(p->sessionid,QString::fromUtf8(p->name));
		else
			emit auth_failed(p->sessionid,QString::fromUtf8(p->name));
		CloseHandle(token);
#else
		/* conversation-Struktur */
		static struct pam_conv conv = {
			su_conv,          /* default PAM-conversation */
			NULL                /* brauchen wir nicht */
		};

        int retval;

		pam_handle_t *pamh = NULL;

		/* PAM-Transaktion starten */
        retval = pam_start("other",      /* Dienstname */
                           user,              /* Nutzername */
                           &conv,             /* conversation-Funktion */
                           &pamh);            /* PAM-Handle das zum Fuellen */

        /* Nutzer authentifizieren */
        if (retval == PAM_SUCCESS) {
            retval = pam_authenticate(pamh, 0);

            if (retval != PAM_SUCCESS) {
                //qDebug() <<"Fehler beim authentifizieren von Nutzer" << user << ":" << pam_strerror( pamh, retval);
                emit auth_failed(p->sessionid,QString::fromUtf8(p->name));
            }
        } else {
            qWarning() <<"Fehler bei Verbindung zu pam";
            emit auth_failed(p->sessionid,QString::fromUtf8(p->name));
        }

        /* Nutzerkonto auf Guelitigkeit pruefen */
        if (retval == PAM_SUCCESS) {
            retval = pam_acct_mgmt(pamh, PAM_SILENT);

            if (retval != PAM_SUCCESS) {
                //qDebug() << "Nutzerkonto von" << user << "nicht in Ordnung:" << pam_strerror( pamh, retval);
                emit auth_failed(p->sessionid,QString::fromUtf8(p->name));
            }
        }

        if (retval == PAM_SUCCESS) {
            emit auth_success(p->sessionid,QString::fromUtf8(p->name));
        }

        delete p;

        /* PAM-Transaktion beenden */
        if (pam_end(pamh, retval) != PAM_SUCCESS) {
            pamh = NULL;
            continue;
        }
#endif
    }
}

