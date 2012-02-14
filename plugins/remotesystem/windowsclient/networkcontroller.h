#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <QObject>
#include <QSslSocket>

class NetworkController : public QSslSocket
{
    Q_OBJECT
public:
    static NetworkController* intance();
    void setId(const QByteArray& id);
private:
    QSslSocket* m_socket;
    QByteArray m_identifier;
    explicit NetworkController(QObject *parent = 0);
public slots:
    void networkConfigChanged(QString host, int port);
private Q_SLOTS:
    void readyRead();
    void socketDisconnected();
    void socketConnected();
    void sslErrors ( const QList<QSslError> & errors );
Q_SIGNALS:
    void message(const QString& msg);
    void serverJSON(const QVariantMap& data);
};

#endif // NETWORKCONTROLLER_H
