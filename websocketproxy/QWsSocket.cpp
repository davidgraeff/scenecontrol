#include "QWsSocket.h"

#include <QCryptographicHash>
#include <QtEndian>

#include "QWsServer.h"

int QWsSocket::maxBytesPerFrame = 1400;

QWsSocket::QWsSocket( QObject * parent, QTcpSocket * socket, EWebsocketVersion ws_v ) :
	QAbstractSocket( QAbstractSocket::UnknownSocketType, parent ),
	tcpSocket( socket ),
	_version( ws_v ),
	_hostPort( -1 ),
	closingHandshakeSent( false ),
	closingHandshakeReceived( false ),
	readingState( HeaderPending ),
	isFinalFragment( false ),
	hasMask( false ),
	payloadLength( 0 ),
	maskingKey( 4, 0 )
{
	tcpSocket->setParent( this );

	setSocketState( tcpSocket->state() );

	if ( _version == WS_V0 )
		connect( tcpSocket, SIGNAL(readyRead()), this, SLOT(processDataV0()) );
	else if ( _version >= WS_V4 )
		connect( tcpSocket, SIGNAL(readyRead()), this, SLOT(processDataV4()) );
	connect( tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError)) );
	connect( tcpSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)) );
	connect( tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processTcpStateChanged(QAbstractSocket::SocketState)) );
	connect( tcpSocket, SIGNAL(readChannelFinished()), this, SIGNAL(readChannelFinished()) );
}

QWsSocket::~QWsSocket()
{
	QAbstractSocket::SocketState socketState = state();
	if ( state() == QAbstractSocket::ConnectedState )
	{
		qDebug() << "CloseAway, socket destroyed in server";
		close( CloseGoingAway, QLatin1String("socket destroyed in server") );
	}
}

void QWsSocket::processDataV4()
{
	while (true) switch ( readingState ) {
	case HeaderPending: {
		if (tcpSocket->bytesAvailable() < 2)
			return;

		// FIN, RSV1-3, Opcode
		char header[2];
		tcpSocket->read(header, 2); // XXX: Handle return value
		isFinalFragment = (header[0] & 0x80) != 0;
		opcode = static_cast<EOpcode>(header[0] & 0x0F);

		// Mask, PayloadLength
		hasMask = (header[1] & 0x80) != 0;
		quint8 length = (header[1] & 0x7F);

		switch (length) {
		case 126:
			readingState = PayloadLengthPending;
			break;
		case 127:
			readingState = BigPayloadLenghPending;
			break;
		default:
			payloadLength = length;
			readingState = MaskPending;
			break;
		}
	}; break;
	case PayloadLengthPending: {
		if (tcpSocket->bytesAvailable() < 2)
			return;

		uchar length[2];
		tcpSocket->read(reinterpret_cast<char *>(length), 2); // XXX: Handle return value
		payloadLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(length));
		readingState = MaskPending;
	}; break;
	case BigPayloadLenghPending: {
		if (tcpSocket->bytesAvailable() < 8)
			return;

		uchar length[8];
		tcpSocket->read(reinterpret_cast<char *>(length), 8); // XXX: Handle return value
		// Most significant bit must be set to 0 as per http://tools.ietf.org/html/rfc6455#section-5.2
		// XXX: Check for that?
		payloadLength = qFromBigEndian<quint64>(length) & ~(1LL << 63);
		readingState = MaskPending;
	}; break;
	case MaskPending: {
		if (!hasMask) {
			readingState = PayloadBodyPending;
			break;
		}

		if (tcpSocket->bytesAvailable() < 4)
			return;

		tcpSocket->read(maskingKey.data(), 4); // XXX: Handle return value
		readingState = PayloadBodyPending;
	}; /* Intentional fall-through */
	case PayloadBodyPending: {
		// TODO: Handle large payloads
		if (tcpSocket->bytesAvailable() < static_cast<qint32>(payloadLength))
			return;

		// Extension // UNSUPPORTED FOR NOW
		QByteArray ApplicationData = tcpSocket->read( payloadLength );
		if ( hasMask )
			ApplicationData = QWsSocket::mask( ApplicationData, maskingKey );
		currentFrame.append( ApplicationData );

		readingState = HeaderPending;

		if ( !isFinalFragment )
			break;

		switch ( opcode )
		{
			case OpBinary:
				emit frameReceived( currentFrame );
				break;
			case OpText:
				emit frameReceived( QString::fromAscii(currentFrame) );
				break;
			case OpPing:
				write( QWsSocket::composeHeader( true, OpPong, 0 ) );
				break;
			case OpPong:
				emit pong( pingTimer.elapsed() );
				break;
			case OpClose:
				closingHandshakeReceived = true;
				close();
				break;
			default:
				qWarning()<< "Unhandled payload opcode";
				break;
		}

		currentFrame.clear();
	}; break;
	} /* while (true) switch */
}

void QWsSocket::processDataV0()
{
	QByteArray BA, buffer;
	quint8 type, b = 0x00;

	BA = tcpSocket->read(1);
	type = BA[0];

	if ( ( type & 0x80 ) == 0x00 ) // MSB of type not set
	{
		if ( type != 0x00 )
		{
			// ABORT CONNEXION
			tcpSocket->readAll();
			return;
		}
		
		// read data
		do
		{
			BA = tcpSocket->read(1);
			b = BA[0];
			if ( b != 0xFF )
				buffer.append( b );
		} while ( b != 0xFF );

		currentFrame.append( buffer );
	}
	else // MSB of type set
	{
		if ( type != 0xFF )
		{
			// ERROR, ABORT CONNEXION
			close();
			return;
		}

		quint8 length = 0x00;
		
		bool bIsNotZero = true;
		do
		{
			BA = tcpSocket->read(1);
			b = BA[0];
			bIsNotZero = ( b != 0x00 ? true : false );
			if ( bIsNotZero ) // b must be != 0
			{
				quint8 b_v = b & 0x7F;
				length *= 128;
				length += b_v;
			}
		} while ( ( ( b & 0x80 ) == 0x80 ) && bIsNotZero );

		BA = tcpSocket->read(length); // discard this bytes
	}

	if ( currentFrame.size() > 0 )
	{
		QString byteString;
		byteString.reserve( currentFrame.size() );
		for (int i=0 ; i<currentFrame.size() ; i++)
			byteString[i] = currentFrame[i];
		emit frameReceived( byteString );
		currentFrame.clear();
	}

	if ( tcpSocket->bytesAvailable() )
		processDataV0();
}

qint64 QWsSocket::write ( const QString & string )
{
	if ( _version == WS_V0 )
	{
		return QWsSocket::write( string.toAscii() );
	}

	const QList<QByteArray> & framesList = QWsSocket::composeFrames( string.toAscii(), false, maxBytesPerFrame );
	return writeFrames( framesList );
}

qint64 QWsSocket::write ( const QByteArray & byteArray )
{
	if ( _version == WS_V0 )
	{
		QByteArray BA;
		BA.append( (char)0x00 );
		BA.append( byteArray );
		BA.append( (char)0xFF );
		return writeFrame( BA );
	}

	const QList<QByteArray> & framesList = QWsSocket::composeFrames( byteArray, true, maxBytesPerFrame );

	qint64 nbBytesWritten = writeFrames( framesList );
	emit bytesWritten( nbBytesWritten );

	return nbBytesWritten;
}

qint64 QWsSocket::writeFrame ( const QByteArray & byteArray )
{
	return tcpSocket->write( byteArray );
}

qint64 QWsSocket::writeFrames ( const QList<QByteArray> & framesList )
{
	qint64 nbBytesWritten = 0;
	for ( int i=0 ; i<framesList.size() ; i++ )
	{
		nbBytesWritten += writeFrame( framesList[i] );
	}
	return nbBytesWritten;
}

void QWsSocket::processTcpStateChanged( QAbstractSocket::SocketState tcpSocketState )
{
	QAbstractSocket::SocketState wsSocketState = state();
	switch ( tcpSocketState )
	{
		case QAbstractSocket::ClosingState:
		{
			if ( wsSocketState == QAbstractSocket::ConnectedState )
			{
				close( CloseGoingAway );
				setSocketState( QAbstractSocket::ClosingState );
				emit stateChanged( QAbstractSocket::ClosingState );
				emit aboutToClose();
			}
			break;
		}
		case QAbstractSocket::UnconnectedState:
		{
			if ( wsSocketState == QAbstractSocket::ConnectedState || wsSocketState == QAbstractSocket::ClosingState )
			{
				setSocketState( QAbstractSocket::UnconnectedState );
				emit stateChanged( QAbstractSocket::UnconnectedState );
				emit disconnected();
			}
			break;
		}
	}
}

void QWsSocket::close( ECloseStatusCode closeStatusCode, QString reason )
{
	if ( ! closingHandshakeSent )
	{
		switch ( _version )
		{
			case WS_V4:
			case WS_V5:
			case WS_V6:
			case WS_V7:
			case WS_V8:
			case WS_V13:
			{
				// Compose and send close frame
				QByteArray BA;

				// Close code
				BA.append( QWsSocket::composeHeader( true, OpClose, 0 ) );

				// Close status code (optional)
				BA.append( QWsServer::serializeInt( (int)closeStatusCode, 2 ).toAscii() );

				// Reason (optional)
				if ( reason.size() )
					BA.append( reason.toAscii() );
				
				// Send closing handshake
				tcpSocket->write( BA );
				tcpSocket->flush();

				break;
			}
			case WS_V0:
			{
				QByteArray closeFrame;
				closeFrame.append( (char)0xFF );
				closeFrame.append( (char)0x00 );
				tcpSocket->write( closeFrame );
				tcpSocket->flush();
				break;
			}
			default:
			{
				//DO NOTHING
				break;
			}
		}		

		closingHandshakeSent = true;

		setSocketState( QAbstractSocket::ClosingState );
		emit aboutToClose();
	}
	
	if ( closingHandshakeSent && closingHandshakeReceived )
	{
		setSocketState( QAbstractSocket::UnconnectedState );
		emit disconnected();
		tcpSocket->close();
		return;
	}
}

QByteArray QWsSocket::generateMaskingKey()
{
	QByteArray key;
	for ( int i=0 ; i<4 ; i++ )
	{
		key.append( qrand() % 0x100 );
	}
	return key;
}

QByteArray QWsSocket::generateMaskingKeyV4( QString key, QString nonce )
{
	QString concat = key + nonce + "61AC5F19-FBBA-4540-B96F-6561F1AB40A8";
	QByteArray hash = QCryptographicHash::hash ( concat.toAscii(), QCryptographicHash::Sha1 );
	return hash;
}

QByteArray QWsSocket::mask( QByteArray & data, QByteArray & maskingKey )
{
	for ( int i=0 ; i<data.size() ; i++ )
	{
		data[i] = ( data[i] ^ maskingKey[ i % 4 ] );
	}

	return data;
}

QList<QByteArray> QWsSocket::composeFrames( const QByteArray& byteArray_, bool asBinary, int maxFrameBytes )
{
	if ( maxFrameBytes == 0 )
		maxFrameBytes = maxBytesPerFrame;

	QByteArray byteArray = byteArray_;
	QList<QByteArray> framesList;

	QByteArray maskingKey;

	int nbFrames = byteArray.size() / maxFrameBytes + 1;

	for ( int i=0 ; i<nbFrames ; i++ )
	{
		QByteArray BA;

		// fin, size
		bool fin = false;
		quint64 size = maxFrameBytes;
		EOpcode opcode = OpContinue;
		if ( i == nbFrames-1 ) // for multi-frames
		{
			fin = true;
			size = byteArray.size();
		}
		if ( i == 0 )
		{
			if ( asBinary )
				opcode = OpBinary;
			else
				opcode = OpText;
		}
		
		// Header
		BA.append( QWsSocket::composeHeader( fin, opcode, size, maskingKey ) );
		
		// Application Data
		QByteArray dataForThisFrame = byteArray.left( size );
		byteArray.remove( 0, size );
		
		//dataForThisFrame = QWsSocket::mask( dataForThisFrame, maskingKey );
		BA.append( dataForThisFrame );
		
		framesList << BA;
	}

	return framesList;
}

QByteArray QWsSocket::composeHeader( bool fin, QWsSocket::EOpcode opcode, quint64 payloadLength, const QByteArray& maskingKey )
{
	QByteArray BA;
	quint8 byte;

	// FIN, RSV1-3, Opcode
	byte = 0x00;
	// FIN
	if ( fin )
		byte = (byte | 0x80);
	// Opcode
	byte = (byte | opcode);
	BA.append( byte );

	// Mask, PayloadLength
	byte = 0x00;
	QByteArray BAsize;
	// Mask
	if ( maskingKey.size() == 4 )
		byte = (byte | 0x80);
	// PayloadLength
	if ( payloadLength <= 125 )
	{
		byte = (byte | payloadLength);
	}
	// Extended payloadLength
	else
	{
		// 2 bytes
		if ( payloadLength <= 0xFFFF )
		{
			byte = ( byte | 126 );
			BAsize.append( ( payloadLength >> 1*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 0*8 ) & 0xFF );
		}
		// 8 bytes
		else if ( payloadLength <= 0x7FFFFFFF )
		{
			byte = ( byte | 127 );
			BAsize.append( ( payloadLength >> 7*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 6*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 5*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 4*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 3*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 2*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 1*8 ) & 0xFF );
			BAsize.append( ( payloadLength >> 0*8 ) & 0xFF );
		}
	}
	BA.append( byte );
	BA.append( BAsize );

	// Masking
	if ( maskingKey.size() == 4 )
		BA.append( maskingKey );

	return BA;
}

void QWsSocket::ping()
{
	pingTimer.restart();
	QByteArray pingFrame = QWsSocket::composeHeader( true, OpPing, 0 );
	writeFrame( pingFrame );
}

void QWsSocket::setResourceName( QString rn )
{
	_resourceName = rn;
}

void QWsSocket::setHost( QString h )
{
	_host = h;
}

void QWsSocket::setHostAddress( QString ha )
{
	_hostAddress = ha;
}

void QWsSocket::setHostPort( int hp )
{
	_hostPort = hp;
}

void QWsSocket::setOrigin( QString o )
{
	_origin = o;
}

void QWsSocket::setProtocol( QString p )
{
	_protocol = p;
}

void QWsSocket::setExtensions( QString e )
{
	_extensions = e;
}

EWebsocketVersion QWsSocket::version()
{
	return _version;
}

QString QWsSocket::resourceName()
{
	return _resourceName;
}

QString QWsSocket::host()
{
	return _host;
}

QString QWsSocket::hostAddress()
{
	return _hostAddress;
}

int QWsSocket::hostPort()
{
	return _hostPort;
}

QString QWsSocket::origin()
{
	return _origin;
}

QString QWsSocket::protocol()
{
	return _protocol;
}

QString QWsSocket::extensions()
{
	return _extensions;
}

QString QWsSocket::composeOpeningHandShake( QString resourceName, QString host, QString origin, QString extensions, QString key )
{
	QString hs;
	hs.append("GET " + resourceName + " HTTP/1.1\r\n");
	hs.append("Host: " + host + "\r\n");
	hs.append("Upgrade: websocket\r\n");
	hs.append("Connection: Upgrade\r\n");
	hs.append("Sec-WebSocket-Key: " + key + "\r\n");
	hs.append("Origin: " + origin + "\r\n");
	hs.append("Sec-WebSocket-Extensions: " + extensions + "\r\n");
	hs.append("Sec-WebSocket-Version: 13\r\n");
	hs.append("\r\n");
	return hs;
}
