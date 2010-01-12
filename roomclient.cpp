#include "roomclient.h"
#include <QStringList>
#include <QDebug>
#include <QDateTime>

#define LISTENPORT 3101
#define API_VERSION "1.1"
#include <Transferables/transferable.h>

RoomClient::RoomClient()
{
    QObject::connect ( &socket,SIGNAL ( connected() ),SLOT ( connected() ) );
    QObject::connect ( &socket,SIGNAL ( disconnected() ),SLOT ( disconnected() ) );
    QObject::connect ( &socket,SIGNAL ( readyRead() ),SLOT ( readyRead() ) );
    QObject::connect ( &socket,SIGNAL ( error ( QAbstractSocket::SocketError ) ),SLOT ( error ( QAbstractSocket::SocketError ) ) );
    inCmd = false;
}

RoomClient::~RoomClient()
{
    socket.disconnectFromHost();
}

QString RoomClient::getServerVersion()
{
    return server_version;
}

QString RoomClient::getMinApiVersion()
{
    return server_min_api_version;
}

QString RoomClient::getMaxApiVersion()
{
    return server_max_api_version;
}

bool RoomClient::isValidApiVersion()
{
    if ( server_min_api_version.isEmpty() || server_max_api_version.isEmpty() ) return false;
    return ( ( server_min_api_version>=API_VERSION ) && ( API_VERSION>=server_max_api_version ) );
}

void RoomClient::connect ( const QString& ip, int port )
{
    server_version = QString();
    server_min_api_version = QString();
    server_max_api_version = QString();
    disconnected();
    socket.connectToHost ( ip, port );
}

void RoomClient::close()
{
    socket.disconnectFromHost();
    disconnected();
}

void RoomClient::requestVersion()
{
    send ( "version" );
}

/**
 * Use this to send profiles, alarms
 */
void RoomClient::send ( const QString& str, bool endline )
{
    //qDebug() << str;
    socket.write ( ( endline? str.toAscii() +'\n':str.toAscii() ) );
}

void RoomClient::setChannel ( uint id, uint value )
{
    send ( "setChannel\t"+QString::number ( id ) +"\t"+QString::number ( value ) );
}

void RoomClient::setPin ( uint id, uint value )
{
    send ( "setPin\t"+QString::number ( id ) +"\t"+QString::number ( value ) );
}

void RoomClient::setPlaylist ( const QString& id, const QStringList& files )
{
    send ( "music.playlist.set\t" + id + "\t" + files.join ( "\t" ) );
}

void RoomClient::removePlaylist ( const QString& id )
{
    send ( "music.playlist.remove\t"+id );
}

void RoomClient::renamePlaylist ( const QString& id, const QString& name )
{
    send ( "music.playlist.setName\t"+id +"\t"+name );
}

void RoomClient::addPlaylist ( const QString& name )
{
    send ( "music.playlist.add\t"+name );
}

void RoomClient::renameChannel ( uint channel, const QString& name )
{
    send ( "setChannelName\t"+QString::number ( channel ) +"\t"+name );
}

void RoomClient::renamePin ( uint channel, const QString& name )
{
    send ( "setPinName\t"+QString::number ( channel ) +"\t"+name );
}

void RoomClient::rename ( const QUuid& id, const QString& name )
{
    send ( "rename\t"+id.toString() +"\t"+name );
}

void RoomClient::change ( const QString& objectstring )
{
    send ( "set\t"+objectstring );
}

void RoomClient::request ( const QUuid& id, const QUuid& timestamp )
{
    QMap<QUuid, CacheObject>::iterator it = objectcache.find ( id );

    if ( it == objectcache.end() || timestamp != QUuid ( it.value().timestamp ) || ( it.value().loading && !it.value().requested ) )
    {
        it.value().requested = true;
        send ( "get\t"+id.toString() );
    }
    else if ( !it.value().loading )
    {
        emit changed ( Transferable::determineType ( it.value().object[0] ), it.value().object );
    }
}

void RoomClient::activate ( const QUuid& id )
{
    send ( "activate\t"+id.toString() );
}

void RoomClient::removeMany ( const QStringList& ids )
{
    QString str;
    foreach ( QString id, ids )
    {
        str += "remove\t"+id+"\n";
    }
    send ( str, false );
}

void RoomClient::readyRead()
{
    while ( socket.bytesAvailable() )
    {
        buffer += socket.readLine();
        if ( buffer.rightRef ( 1 ) != "\n" )
        {
            if ( buffer.size() >1024 ) buffer.clear();
            continue;
        }
        QStringList items = buffer.replace ( '\n', "" ).replace ( '\r', "" ).split ( '\t' );
        buffer.clear();
        if ( !items.size() ) continue;

        QString cmd = items.takeFirst();

        //qDebug() << "Received" << cmd << items;

        if ( cmd == "version" )
        {
            server_version = items[0];
            server_min_api_version = items[1];
            server_max_api_version = items[2];

            emit serverinfo();
            if ( isValidApiVersion() )
                emit stateChanged ( RoomserverValid );
        }
        else
            // get all
            if ( cmd == "channels" )
            {
                emit channels ( items );
            }
            else if ( cmd == "pins" )
            {
                emit pins ( items );
            }
            else if ( cmd=="pinChanged" )
            {
                if ( items.size() <1 ) continue;
                QStringList d = items[0].split ( ':' );
                if ( d.size() !=2 ) continue;
                emit pinChanged ( d[0].toUInt(),d[1].toUInt() );
            }
            else if ( cmd == "pinNameChanged" )
            {
                if ( items.size() <2 ) continue;
                emit pinNameChanged ( items[0].toInt(), items[1] );
            }
            else if ( cmd=="channelChanged" )
            {
                if ( items.size() <1 ) continue;
                QStringList d = items[0].split ( ':' );
                if ( d.size() !=2 ) continue;
                emit channelChanged ( d[0].toUInt(),d[1].toUInt() );
            }
            else if ( cmd == "channelNameChanged" )
            {
                if ( items.size() <2 ) continue;
                emit channelNameChanged ( items[0].toInt(), items[1] );
            }
            else if ( cmd == "object" )
            {
                if ( items.size() <4 )
                {
                    qDebug() << __FUNCTION__ << __LINE__ << items.count() << items;
                    continue;
                }
                objectcache.insert ( QUuid ( items[1] ), CacheObject ( items, items[2], false ) );
                emit changed ( Transferable::determineType ( items[0] ), items );
            }
            else if ( cmd == "changed" )
            {
                if ( items.size() <3 )
                {
                    qDebug() << __FUNCTION__ << __LINE__ << items.count() << items;
                    continue;
                }
                // if the real object is already cached, ignore this event
                QMap<QUuid, CacheObject>::iterator it = objectcache.find ( QUuid ( items[1] ) );
                if ( it != objectcache.end() && it.value().timestamp == items[2] ) continue;

                // insert object type+id+timestamp
                objectcache.insert ( QUuid ( items[1] ), CacheObject ( items, items[2], true ) );
                //emit changed ( Transferable::determineType ( items[0] ), items );

                // request real object
                send ( "get\t"+items[1] );
            }
            else if ( cmd == "removed" )
            {
                emit removed ( Transferable::determineType ( items[0] ), items );
            }
            else if ( cmd == "armed" )
            {
                if ( items.size() <1 ) continue;
                emit alarmsArmed ( items.takeFirst().toInt(), items );
            }
            else if ( cmd=="music.playlist.names" )
            {
                emit playlistsChanged ( items );
            }
            else if ( cmd=="music.playlist.get" )
            {
                if ( items.size() <1 ) continue;
                emit playlistItems ( items.takeFirst(), items );
            }
            else if ( cmd=="music.playlist.add" )
            {
                if ( items.size() <2 ) continue;
                emit playlistAdded ( items[0], items[1] );
            }
            else if ( cmd=="music.playlist.remove" )
            {
                if ( items.size() <1 ) continue;
                emit playlistRemoved ( items[0] );
            }
            else if ( cmd=="music.playlist.setName" )
            {
                if ( items.size() <2 ) continue;
                emit playlistRenamed ( items[0], items[1] );
            }
            else if ( cmd=="music.volume" )
            {
                if ( items.size() <1 ) continue;
                emit musicVolume(items[0].toDouble());
            }
            else if ( cmd=="music.position" )
            {
                if ( items.size() <2 ) continue;
                emit musicPosition(items[0].toULongLong(), items[1].toULongLong());
            }
            else if ( cmd=="music.changed" )
            {
                if ( items.size() <4 ) continue;
                emit musicChanged ( items[0], items[1], items[2].toUInt(), items[3] );
            }
    }
}

void RoomClient::error ( QAbstractSocket::SocketError err )
{
    qDebug() << __FUNCTION__ << __LINE__ << err;
    objectcache.clear();
    emit stateChanged ( ConnectionFailure );
}

void RoomClient::disconnected()
{
    objectcache.clear();
    emit stateChanged ( ConnectionDisconnected );
}

void RoomClient::connected()
{
    if ( socket.isOpen() )
    {
        emit stateChanged ( ConnectionConnected );
        refresh();
        refreshArmedAlarms();
        requestChannels();
        requestPins();
        requestPlaylists();
	requestMusicCurrent();
	requestMusicPosition();
	requestMusicVolume();
    }
}

void RoomClient::requestPlaylists()
{
    send ( "music.playlist.names" );
}

void RoomClient::requestPlaylist ( const QUuid& id )
{
    send ( "music.playlist.get\t" + id );
}

void RoomClient::refreshEthersex()
{
    send ( "refresh" );
}

void RoomClient::refresh()
{
    send ( "all" );
}

void RoomClient::requestChannels()
{
    send ( "channels" );
}

void RoomClient::requestPins()
{
    send ( "pins" );
}

void RoomClient::refreshArmedAlarms()
{
    send ( "armed" );
}

void RoomClient::eventSnooze()
{
    send ( "event.snooze" );
}

void RoomClient::eventStop()
{
    send ( "event.stop" );
}

void RoomClient::eventPlay ( const QString& title, const QString& file )
{
    send ( "event.add\t" + title +"\t" + file );
}

void RoomClient::setPlaying(const QString& playlistID, int track)
{
    send ( "music.setPlaying\t" + playlistID +"\t" + QString::number(track) );
}

void RoomClient::music_play()
{
    send ( "music.play" );
}

void RoomClient::music_stop()
{
    send ( "music.stop" );
}

void RoomClient::music_pause()
{
    send ( "music.pause" );
}

void RoomClient::music_next()
{
    send ( "music.next" );
}

void RoomClient::music_previous()
{
    send ( "music.previous" );
}

void RoomClient::music_setVolume(qreal vol)
{
    send ( "music.volume\t" + QString::number(vol) );
}

void RoomClient::music_setPosition(ulong pos)
{
    send ( "music.position\t" + QString::number(pos));
}

void RoomClient::requestMusicVolume()
{
    send ( "music.volume" );
}

void RoomClient::requestMusicPosition()
{
    send ( "music.position" );
}

void RoomClient::requestMusicCurrent()
{
    send ( "music.changed" );
}
