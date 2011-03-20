#pragma once
#include <Phonon/MediaObject>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/VideoWidget>
#include <Phonon/Global>
#include <QMouseEvent>
#include <qfile.h>
#include <QTimer>
#include <QSocketNotifier>
#include <QTcpSocket>
#include <QTcpServer>

class MediaPlayer : public Phonon::VideoWidget {
    Q_OBJECT
public:
    MediaPlayer(QWidget *parent=0);
	virtual ~MediaPlayer() {}
private:
    Phonon::MediaObject *m_videomedia;
    Phonon::MediaObject *m_eventmedia;
    Phonon::AudioOutput *m_outputvideo;
    Phonon::AudioOutput *m_outputevents;
    QTimer restoretimer;
    int m_screen;
    QByteArray m_buffer;
    int m_bufferpos;
    QTcpSocket* m_socket;
    QTcpServer m_tcpserver;
    void smallWindow() ;
    void fullscreenWindow();
    void setSceenState(int state);
    virtual void mousePressEvent(QMouseEvent* );
public slots:
    void readyRead();
    void prefinishMarkReached(qint32);
    void restoreTimeout();
    void newConnection();
};


