#include "main.h"
#include <QtGui/QApplication>
#include <qdesktopwidget.h>
#include <QUrl>
#include <QSocketNotifier>
#include <signal.h>
#ifndef _WIN32
#include <sys/prctl.h>
#endif
#include <stdio.h>
#include "configexternal.h"

MediaPlayer::MediaPlayer(QWidget *parent)
        : Phonon::VideoWidget(parent), m_bufferpos(0), m_socket(0)
{
    m_videomedia = new Phonon::MediaObject(this);
    m_eventmedia = new Phonon::MediaObject(this);
    connect(m_videomedia,SIGNAL(prefinishMarkReached(qint32)), SLOT(prefinishMarkReached(qint32)));
    m_videomedia->setPrefinishMark(500);
    m_outputvideo = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    m_outputevents = new Phonon::AudioOutput(Phonon::NotificationCategory, this);
    m_screen = 0;
    setAspectRatio(AspectRatioWidget);

    Phonon::createPath(m_videomedia, m_outputvideo);
    Phonon::createPath(m_videomedia, this);

    Phonon::createPath(m_eventmedia, m_outputevents);

    connect(&restoretimer,SIGNAL(timeout()),SLOT(restoreTimeout()));
    restoretimer.setInterval(30000);

    connect(&m_tcpserver,SIGNAL(newConnection()),SLOT(newConnection()));
    m_tcpserver.listen(QHostAddress::Any,CONFIG_LISTENPORT);
}

void MediaPlayer::readyRead() {
    if (!m_socket->canReadLine()) {
        if (m_socket->bytesAvailable()>1000) m_socket->readAll();
        return;
    }
    QByteArray line = m_socket->readLine();
    line.chop(1);

    QList<QByteArray> args = line.split(' ');
    if (args.size()==0) return;
    if (args[0] == "hideVideo") {
        hide();
        restoretimer.start();
    } else if (args[0] == "closeFullscreen") {
        smallWindow();
    } else if (args[0] == "stopvideo") {
        restoretimer.stop();
        m_videomedia->stop();
        hide();
    } else if (args[0] == "stopevent") {
        m_eventmedia->stop();
    } else if (args[0] == "display" && args.size()==2) {
        m_screen = args[1].toInt();
    } else if (args[0] == "videovolume_relative" && args.size()==2) {
        qreal vol = m_outputvideo->volume();
        vol += args[1].toDouble();
        m_outputvideo->setVolume(vol);
    } else if (args[0] == "eventvolume_relative" && args.size()==2) {
        qreal vol = m_outputevents->volume();
        vol += args[1].toDouble();
        m_outputevents->setVolume(vol);
    } else if (args[0] == "videovolume" && args.size()==2) {
        m_outputvideo->setVolume(args[1].toDouble());
    } else if (args[0] == "eventvolume" && args.size()==2) {
        m_outputevents->setVolume(args[1].toDouble());
    } else if (args[0] == "clickactions" && args.size()==4) {
        leftclick = (ActorAmbienceVideo::EnumOnClick)args[1].toInt();
        rightclick = (ActorAmbienceVideo::EnumOnClick)args[2].toInt();
        restoretimer.setInterval(args[3].toInt());
    } else if (args[0] == "displaystate" && args.size()==2) {
        setSceenState(args[1].toInt());
    } else if (args[0] == "showmessage" && args.size()==3) {
        const int duration = args[1].toInt();
        const QString title = QString::fromUtf8(line.mid(2+args[0].count()+args[1].count()));
    } else if (args[0] == "playvideo" && args.size()>1) {
        const QUrl filename = QUrl(QString::fromUtf8(line.mid(6)));
        fullscreenWindow();
        if (m_videomedia->currentSource().url()!=filename) {
            m_videomedia->setCurrentSource(filename);
            m_videomedia->play();
        } else if (m_videomedia->state()!=Phonon::PlayingState)
            m_videomedia->play();
    } else if (args[0] == "playevent" && args.size()>1) {
        const QUrl filename = QUrl(QString::fromUtf8(line.mid(1+args[0].count())));
        if (m_eventmedia->currentSource().url()!=filename) {
            m_eventmedia->setCurrentSource(filename);
            m_eventmedia->play();
        } else if (m_eventmedia->state()!=Phonon::PlayingState)
            m_eventmedia->play();
    }
}

void MediaPlayer::prefinishMarkReached(qint32) {
    m_videomedia->seek(0);
}

void MediaPlayer::mousePressEvent(QMouseEvent*event) {
    ActorAmbienceVideo::EnumOnClick i=ActorAmbienceVideo::OnClickClose;
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
    setVisible(true);
}

void MediaPlayer::restoreTimeout() {
    fullscreenWindow();
}
void MediaPlayer::setSceenState(int state) {
#ifdef _WIN32
    qWarning() << "screen states not supported within windows";
#else
    QByteArray xsetdpms;
    xsetdpms.append("xset -display :");
    xsetdpms.append(QByteArray::number(m_screen));
    xsetdpms.append(" dpms force ");
    QByteArray xsetreset;
    xsetreset.append("xset -display :");
    xsetreset.append(QByteArray::number(m_screen));
    xsetreset.append(" s reset");
    QByteArray xsetnoblank;
    xsetnoblank.append("xset -display :");
    xsetnoblank.append(QByteArray::number(m_screen));
    xsetnoblank.append(" -dpms");
    switch (state) {
    case 0:
        xsetdpms.append("off");
        if (system(xsetdpms.constData()) != 0)
            qWarning() << "dmps screen off may have failed";
        break;
    case 1:
        xsetdpms.append("on");
        if (system(xsetdpms.constData()) != 0)
            qWarning() << "dmps screen on may have failed";
        if (system(xsetreset.constData()) != 0)
            qWarning() << "dmps screen on may have failed (reset)";
        if (system(xsetnoblank.constData()) != 0)
            qWarning() << "deactivation of dmps may have failed";
        break;
    default:
        break;
    }
#endif
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
#ifndef _WIN32
    signal(SIGHUP, catch_int)
    prctl(PR_SET_PDEATHSIG, SIGHUP); // quit if parent dies
#endif
    QApplication app(argc, argv);
    QApplication::setApplicationName(QLatin1String("RoomVideoHelper"));
    MediaPlayer mw;
    mw.hide();
    return app.exec();
}

void MediaPlayer::newConnection() {
    if (m_socket) delete m_socket;
    m_socket = m_tcpserver.nextPendingConnection();
    connect(m_socket,SIGNAL(readyRead()),SLOT(readyRead()));
}

