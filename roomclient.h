#ifndef ROOMCLIENT_H
#define ROOMCLIENT_H

#include <QTcpSocket>
#include <QObject>
#include <QMap>
#include <QList>
#include <QUuid>
#include <QStringList>

enum RoomClientState
{
    RoomserverValid,
    ConnectionConnected,
    ConnectionDisconnected,
    ConnectionFailure
};

class CacheObject
{
public:
    CacheObject ( QStringList object, QUuid timestamp, bool loading ) : object ( object ), timestamp ( timestamp ), loading ( loading ), requested ( false ) {}
    QStringList object;
    QUuid timestamp;
    bool loading;
    bool requested;
};

class RoomClient : public QObject
{
    Q_OBJECT
public:
    RoomClient();
    ~RoomClient();
    void connect ( const QString& ip, int port );
    void refresh();
    void refreshArmedAlarms();
    void refreshEthersex();
    void requestChannels();
    void requestPins();
    void requestPlaylists();
    void requestPlaylist ( const QUuid& id );
    void requestMusicVolume();
    void requestMusicPosition();
    void requestMusicCurrent();    
    void send ( const QString& str, bool endline=true );
    void close();
    void request ( const QUuid& id, const QUuid& timestamp );
    QString getServerVersion();
    QString getMinApiVersion();
    QString getMaxApiVersion();
    bool isValidApiVersion();

private:
    QString server_version;
    QString server_min_api_version;
    QString server_max_api_version;
    QTcpSocket socket;
    QString buffer;
    bool inCmd;
    QString cmd;
    QString type;
    QMap<QUuid, CacheObject> objectcache;

public Q_SLOTS:
    void rename ( const QUuid& id, const QString& name );
    void renameChannel ( uint id, const QString& name );
    void renamePin ( uint id, const QString& name );
    void setChannel ( uint id, uint value );
    void setPin ( uint id, uint value );
    void setPlaylist ( const QString& id, const QStringList& files );
    void removePlaylist( const QString& id );
    void renamePlaylist( const QString& id, const QString& name );
    void addPlaylist ( const QString& name );
    void change ( const QString& objectstring );
    void removeMany ( const QStringList& ids );
    void activate ( const QUuid& id );
    void requestVersion();
    void eventSnooze();
    void eventStop();
    void eventPlay ( const QString& title, const QString& file );
    void setPlaying(const QString& playlistID, int track);
    void music_play();
    void music_stop();
    void music_pause();
    void music_next();
    void music_previous();
    void music_setVolume(qreal vol);
    void music_setPosition(ulong pos);

private Q_SLOTS:
    void readyRead();
    void disconnected();
    void connected();
    void error ( QAbstractSocket::SocketError );

Q_SIGNALS:
    void stateChanged ( RoomClientState );
    void serverinfo();

    void channels ( QStringList& );
    void pins ( QStringList& );
    void channelChanged ( uint id, uint value );
    void channelNameChanged ( uint id, const QString& name );
    void pinChanged ( uint id, uint value );
    void pinNameChanged ( uint id, const QString& name );

    void playlistItems ( const QString& id, const QStringList& files );
    void playlistsChanged ( const QStringList& playlists );
    void playlistRemoved ( const QString& id );
    void playlistRenamed ( const QString& id, const QString& name );
    void playlistAdded ( const QString& id, const QString& name );

    void changed ( int type, QStringList& );
    void removed ( int type, QStringList& );
    void alarmsArmed ( int distance, QStringList ids );
    
    void musicChanged(const QUuid& playlist_id, const QString& playlist_name,
		      uint track_no, const QString& track_name);
    void musicVolume(qreal volume);
    void musicPosition(quint64 current, quint64 total);
};

#endif // ROOMCLIENT_H
