#include "main.h"
#include <QtGui/QApplication>
#include <qdesktopwidget.h>
#include <QUrl>
#include <signal.h>
#include <sys/prctl.h>
#include <QSocketNotifier>
#include <stdio.h>

MediaPlayer::MediaPlayer(QWidget *parent)
        : Phonon::VideoWidget(parent), m_bufferpos(0)
{
    m_media = new Phonon::MediaObject(this);
    connect(m_media,SIGNAL(prefinishMarkReached(qint32)), SLOT(prefinishMarkReached(qint32)));
    m_media->setPrefinishMark(500);
    m_aoutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    m_screen = 0;

    Phonon::createPath(m_media, m_aoutput);
    Phonon::createPath(m_media, this);
    inFile.open(stdin,QIODevice::ReadOnly);
    //stream.setDevice(&inFile);
    notifier = new QSocketNotifier(inFile.handle(),QSocketNotifier::Read,this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(readyRead(int)));

    connect(&restoretimer,SIGNAL(timeout()),SLOT(restoreTimeout()));
    restoretimer.setInterval(30000);
}

void MediaPlayer::readyRead(int) {
    notifier->setEnabled(false);
    QByteArray line = inFile.readLine();
    line.chop(1);
    notifier->setEnabled(true);

    QList<QByteArray> args = line.split(' ');
    if (args.size()==0) return;
    if (args[0] == "hideVideo") {
        hide();
        restoretimer.start();
    } else if (args[0] == "closeFullscreen") {
        smallWindow();
    } else if (args[0] == "stop") {
        restoretimer.stop();
        hide();
    } else if (args[0] == "display" && args.size()==2) {
        m_screen = args[1].toInt();
    } else if (args[0] == "volume_relative" && args.size()==2) {
        qreal vol = m_aoutput->volume();
        vol += args[1].toDouble();
        m_aoutput->setVolume(vol);
    } else if (args[0] == "volume" && args.size()==2) {
        m_aoutput->setVolume(args[1].toDouble());
    } else if (args[0] == "clickactions" && args.size()==4) {
        leftclick = (ActorAmbienceVideo::EnumOnClick)args[1].toInt();
        rightclick = (ActorAmbienceVideo::EnumOnClick)args[2].toInt();
        restoretimer.setInterval(args[3].toInt());
    } else if (args[0] == "displaystate" && args.size()==2) {
        setSceenState(args[1].toInt());
    } else if (args[0] == "play" && args.size()==2) {
        fullscreenWindow();
        m_media->setCurrentSource(QUrl(QString::fromUtf8(args[1])));
		this->setAspectRatio(AspectRatioWidget);
        m_media->play();
    }
}

void MediaPlayer::prefinishMarkReached(qint32) {
    m_media->seek(0);
}

void MediaPlayer::mousePressEvent(QMouseEvent*event) {
    ActorAmbienceVideo::EnumOnClick i;
    if (event->button()==Qt::LeftButton) {
        i = leftclick;
    } else if (event->button()==Qt::RightButton) {
        i = rightclick;
    } else return;
    switch (i) {
    case ActorAmbienceVideo::OnClickClose:
        restoretimer.stop();
        hide();
        break;
    case ActorAmbienceVideo::OnClickCloseFullscreen:
        smallWindow();
        break;
    case ActorAmbienceVideo::OnClickHideVideo:
        hide();
        restoretimer.start();
        break;
    case ActorAmbienceVideo::OnClickScreenOff:
        setSceenState(0);
        break;
    default:
        break;
    }
}

void MediaPlayer::smallWindow() {
    QDesktopWidget w;
    showNormal();
    resize(100,100);
    move(w.width()-width()-10,w.height()-height()-10);
    restoretimer.start();
}

void MediaPlayer::fullscreenWindow() {
    showFullScreen();
	QCursor cursor;
	cursor.setShape(Qt::BlankCursor);
	setCursor(cursor);
}

void MediaPlayer::restoreTimeout() {
	fullscreenWindow();
}
void MediaPlayer::setSceenState(int state) {
    QByteArray b;
    b.append("xset -display :");
    b.append(QByteArray::number(m_screen));
    b.append(" dpms force ");
	switch (state) {
    case 0:
        b.append("off");
        if (system(b.constData()) != 0)
            qWarning() << "dmps screen off may have failed";
        break;
    case 1:
        b.append("on");
        if (system(b.constData()) != 0)
            qWarning() << "dmps screen on may have failed";
		b.clear();
		b.append("xset -display :");
		b.append(QByteArray::number(m_screen));
		b.append(" s reset");
		if (system(b.constData()) != 0)
			qWarning() << "dmps screen on may have failed (reset)";
        break;
    default:
        break;
    }
}

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
    signal(SIGHUP, catch_int);
    prctl(PR_SET_PDEATHSIG, SIGHUP); // quit if parent dies
    QApplication app(argc, argv);
    QApplication::setApplicationName(QLatin1String("RoomVideoHelper"));
    MediaPlayer mw;
    mw.hide();
    return app.exec();
}

