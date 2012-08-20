
#include <QDebug>
#include "plugin.h"

#include <QCoreApplication>
#include "shared/jsondocuments/json.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc<2) {
		qWarning()<<"No instanceid provided!";
		return 1;
	}
    plugin p(QLatin1String(PLUGIN_ID), QString::fromAscii(argv[1]));
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin(const QString& pluginid, const QString& instanceid) : AbstractPlugin(pluginid, instanceid) {
    connect(&m_socket, SIGNAL(connected()), SLOT(hostconnected()));
    connect(&m_socket, SIGNAL(disconnected()), SLOT(hostdisconnected()));
    connect(&m_socket, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(&m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)));
}

plugin::~plugin() {}

void plugin::clear() {}
void plugin::initialize() {}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
    Q_UNUSED(configid);
    if (data.contains(QLatin1String("server")) && data.contains(QLatin1String("port"))) {
        m_host = data[QLatin1String("server")].toString();
        m_port = data[QLatin1String("port")].toInt();
        m_socket.connectToHost(m_host, m_port);
    }
}

void plugin::play() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Player.PlayPause";
    QVariantMap params;
    params[QLatin1String("playerid")] = 1;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "doplay";

    m_socket.write(JSON::stringify(data).toUtf8());

}
void plugin::pause() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Player.PlayPause";
    QVariantMap params;
    params[QLatin1String("playerid")] = 1;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "dopause";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::stop() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Player.Stop";
    QVariantMap params;
    params[QLatin1String("playerid")] = 1;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "dostop";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::next() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Player.GoNext";
    QVariantMap params;
    params[QLatin1String("playerid")] = 1;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "donext";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::prev() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Player.GoPrevious";
    QVariantMap params;
    params[QLatin1String("playerid")] = 1;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "doprev";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::FastForward() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Player.Seek";
    QVariantMap params;
    params[QLatin1String("playerid")] = 1;
    params[QLatin1String("value")] = "smallforward";
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "doforward";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::rewind() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Player.Seek";
    QVariantMap params;
    params[QLatin1String("playerid")] = 1;
    params[QLatin1String("value")] = "smallbackward";
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "dorewind";

    m_socket.write(JSON::stringify(data).toUtf8());
}

void plugin::select() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Input.Select";
    QVariantMap params;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "doselect";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::down() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Input.Down";
    QVariantMap params;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "dodown";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::up() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Input.Up";
    QVariantMap params;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "doup";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::left() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Input.Left";
    QVariantMap params;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "doleft";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::right() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Input.Right";
    QVariantMap params;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "doright";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::home() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Input.Home";
    QVariantMap params;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "dohome";

    m_socket.write(JSON::stringify(data).toUtf8());
}
void plugin::back() {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Input.Back";
    QVariantMap params;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "doback";

    m_socket.write(JSON::stringify(data).toUtf8());
}

void plugin::setVolume(int v) {
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Application.SetVolume";
    QVariantMap params;
    params[QLatin1String("volume")] = v;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "setVolume";

    m_socket.write(JSON::stringify(data).toUtf8());
}

void plugin::setVolumeRelative(int v)
{
    if (m_socket.state()!=QTcpSocket::ConnectedState) {
        if (m_host.isEmpty())
            return;
        m_socket.connectToHost(m_host, m_port);
    }
    QVariantMap data;
    data[QLatin1String("jsonrpc")] = "2.0";
    data[QLatin1String("method")] = "Application.SetVolume";
    QVariantMap params;
    params[QLatin1String("volume")] = v;
    data[QLatin1String("params")] = params;
    data[QLatin1String("id")] = "setVolume";

    m_socket.write(JSON::stringify(data).toUtf8());
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(data);
}

void plugin::hostconnected() {
    qDebug() << "Connected to xmbc";
}
void plugin::hostdisconnected() {
    qWarning() << "Lost connection to xbmc";
}
void plugin::error(QAbstractSocket::SocketError e) {
  // Ignore "host not found" error
    if (e != QAbstractSocket::ConnectionRefusedError && e != QAbstractSocket::HostNotFoundError)
        qWarning() << "Socket error" << e;
}

void plugin::readyRead() {
    // "Seek" to the first "{" character
    QByteArray data = m_socket.peek(m_socket.bytesAvailable());
    const int start = data.indexOf('{');
    if (start==-1) return;
    if (start) {
        // eat up chars until {-char
        m_socket.read(start);
        data = m_socket.peek(m_socket.bytesAvailable());
    }

    // Analyse stream and parse json data as soon as a matching pair of parenthesis has been found: "{..}"
    int opening = 0;
    for (int position=0; position<data.size(); ++position) {
        if (data[position] == '{')
            ++opening;
        else if (data[position] == '}') {
            if (opening==0) {
                QVariant v = JSON::parse(data.mid(0, position+1));
                if (!v.isValid()) {
                    qWarning()  << "Failed parse json" << data.mid(0, position+1);
                } else {
                    qDebug() << v;
                }
                if (data.size()) {
                    m_socket.read(position+1);
                    emit readyRead();
                }
            } else
                --opening;
        }
    }

    // To many data without {..}, drop everything
    if (m_socket.bytesAvailable()>10000)
        m_socket.readAll();
}
