#include <signal.h>
#ifndef _WIN32
#include <sys/prctl.h>
#endif
#include <stdio.h>
#include "window.h"
#include <QtGui/QApplication>

static void catch_int(int)
{
    /* re-set the signal handler again to catch_int, for next time */
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);
    QApplication::exit(0);
}

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "C");
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);
#ifndef _WIN32
    signal(SIGHUP, catch_int);
    prctl(PR_SET_PDEATHSIG, SIGHUP); // quit if parent dies
#endif
    QApplication app(argc, argv);
    QApplication::setApplicationName(QLatin1String("RoomVideoHelper"));
    MediaPlayer *mw = new MediaPlayer();
    mw->hide();
    int r = app.exec();
	delete mw;
	return r;
}
