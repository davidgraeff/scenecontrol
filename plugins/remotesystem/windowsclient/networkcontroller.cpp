#include "networkcontroller.h"
#include "json.h"
#include <QDebug>
#include <QHostAddress>

static NetworkController* i;

NetworkController::NetworkController(QObject *parent) :
        QSslSocket(parent)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(this, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(this, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(this, SIGNAL(sslErrors (QList<QSslError>)), this, SLOT(sslErrors (QList<QSslError>)));
    this->ignoreSslErrors();
    this->setProtocol(QSsl::SslV3);
    
    m_identifier = "winclient1";
}

void NetworkController::networkConfigChanged(QString host, int port)
{
    this->connectToHostEncrypted(host, port);
}

NetworkController* NetworkController::intance() {
    if (!i)
        i= new NetworkController();
    return i;
}

void NetworkController::setId(const QByteArray& id)
{
    m_identifier = id;
}

void NetworkController::sslErrors ( const QList<QSslError> & errors ) {
    QList<QSslError> relevantErrors(errors);
    relevantErrors.removeAll(QSslError(QSslError::SelfSignedCertificate));
    relevantErrors.removeAll(QSslError(QSslError::SelfSignedCertificateInChain));
    relevantErrors.removeAll(QSslError(QSslError::HostNameMismatch));
    this->ignoreSslErrors();

    qDebug() << relevantErrors;
}

void NetworkController::readyRead() {
    QSslSocket *serverSocket = (QSslSocket *)sender();
    while (serverSocket->canReadLine()) {
        const QByteArray rawdata = serverSocket->readLine();
        if (!rawdata.length())
            continue;
        QVariant v =JSON::parse(rawdata);
        if (!v.isNull() || v.type() != QVariant::Map) {
            const QVariantMap m = v.toMap();
            if (m.value(QLatin1String("response"), 1).toInt() == 0)
                emit serverJSON(m);
            else
                qWarning() << "Server response: failure" << rawdata;
        } else {
            qWarning() << "Failed to parse json from server" << rawdata;
        }
    }
}

void NetworkController::socketDisconnected() {
    qDebug() << "socket closed" << this->errorString() << this->error();
    emit message(tr("Disconnected from %1\nError: %2").arg(this->peerAddress().toString()).arg(this->errorString()));
}

void NetworkController::socketConnected()
{
    qDebug() << "socket connected";
    emit message(tr("Connected to %1").arg(this->peerAddress().toString()));
    this->write("{\"type_\":\"execute\", \"plugin_\":\"remotesystem\", \"member_\":\"registerclient\","
                "\"host\":\""+ m_socket->localAddress().toString().toUtf8() +"\", \"identifier\":\""+m_identifier+"\"}\n");
}
