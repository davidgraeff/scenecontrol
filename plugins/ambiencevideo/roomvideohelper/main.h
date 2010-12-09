#pragma once
#include <Phonon/MediaObject>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/VideoWidget>
#include <Phonon/Global>
#include <services/actorambiencevideo.h>
#include <QMouseEvent>
#include <qfile.h>
#include <QTimer>
#include <QSocketNotifier>

class MediaPlayer : public Phonon::VideoWidget {
    Q_OBJECT
public:
    MediaPlayer(QWidget *parent=0);
private:
    Phonon::MediaObject *m_media;
    Phonon::AudioOutput *m_aoutput;
    ActorAmbienceVideo::EnumOnClick leftclick;
    ActorAmbienceVideo::EnumOnClick rightclick;
    QFile inFile;
    QDataStream stream;
	QTimer restoretimer;
	int m_screen;
    QByteArray m_buffer;
    int m_bufferpos;
    QSocketNotifier* notifier;
    void smallWindow() ;
    void fullscreenWindow();
	void setSceenState(int state);
    virtual void mousePressEvent(QMouseEvent* );
public slots:
    void readyRead(int);
    void prefinishMarkReached(qint32);
    void restoreTimeout();
};


