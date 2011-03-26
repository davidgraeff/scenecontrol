/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

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


#include "logging.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <qfile.h>
#include <QDir>

QFile logfile;
bool logToConsole = true;

void setLogOptions(bool toConsole, const char* logfilename) {
	logToConsole = toConsole;
	if (!logfilename) return;
	logfile.setFileName(QDir::home().absoluteFilePath(QLatin1String(logfilename)));
	logfile.open(QIODevice::WriteOnly|QIODevice::Append);
	logfile.write("--------------------------------------\n");
}

void logclose() {
	logfile.close();
}

void messageout(QtMsgType type, const char *msg, FILE * stream_debug, FILE * stream_warning, FILE * stream_error)
{
    time_t rawtime;
    tm * ptm;
    time ( &rawtime );
    ptm = gmtime ( &rawtime );

    switch (type) {
    case QtDebugMsg:
        fprintf(stream_debug, "[%2d:%02d] %s\n", (ptm->tm_hour+1)%24, ptm->tm_min, msg);
        break;
    case QtWarningMsg:
        fprintf(stream_warning, "[%2d:%02d] \033[33mWarning: %s\033[0m\n", (ptm->tm_hour+1)%24, ptm->tm_min, msg);
        break;
    case QtCriticalMsg:
        fprintf(stream_error, "[%2d:%02d] \033[31mCritical: %s\033[0m\n", (ptm->tm_hour+1)%24, ptm->tm_min, msg);
        break;
    case QtFatalMsg:
        fprintf(stream_error, "[%2d:%02d] \033[31mFatal: %s\033[0m\n", (ptm->tm_hour+1)%24, ptm->tm_min, msg);
        abort();
    }
}

void roomMessageOutput(QtMsgType type, const char *msg)
{
	if (logToConsole) {
		messageout(type, msg, stdout, stderr, stderr);
	}
	if (logfile.isWritable()) {
		if (logfile.size()>1024*10) logfile.resize(0);
		logfile.write(msg);
		logfile.write("\n");
	}
}
