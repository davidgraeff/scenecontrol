
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
    //connect(&m_socket, SIGNAL(connected()), SLOT(hostconnected()));
    //connect(&m_socket, SIGNAL(disconnected()), SLOT(hostdisconnected()));
    //connect(&m_socket, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(&m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)));
}

plugin::~plugin() {}

void plugin::clear() {}
void plugin::initialize() {}

void plugin::sentToYamaha(const QByteArray& content) {
	if (m_host.isEmpty())
		return;
	QByteArray t;
	t.reserve(content.size()+200);
	t += "POST /YamahaRemoteControl/ctrl HTTP/1.1\r\n";
	t += "host: "+m_host.toUtf8()+":"+QByteArray::number(m_port)+"/YamahaRemoteControl/ctrl\r\n";
	t += "Content-type: text/xml; charset=UTF-8\r\n";
	t += "Content-length: "+QByteArray::number(content.size())+"\r\n";
	t += "Connection: close\r\n\r\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	t += content;
	if (m_socket.state()!=QAbstractSocket::ConnectedState && m_socket.state()!=QAbstractSocket::ConnectingState)
		m_socket.connectToHost(m_host, m_port);
	if (m_socket.state()==QAbstractSocket::ConnectingState)
		m_socket.waitForConnected(300);
	if (m_socket.state()==QAbstractSocket::ConnectedState)
		m_socket.write(t);
	m_socket.disconnectFromHost();
	m_socket.waitForDisconnected();
}
QByteArray& yamahaPut(QByteArray& content) {
	content.insert(0,"<YAMAHA_AV cmd=\"PUT\">");
	content.append("</YAMAHA_AV>");
	return content;
}
QByteArray& yamahaMainZone(QByteArray& content) {
	content.insert(0,"<Main_Zone>");
	content.append("</Main_Zone>");
	return content;
}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
    Q_UNUSED(configid);
    if (data.contains(QLatin1String("server")) && data.contains(QLatin1String("port"))) {
        m_host = data[QLatin1String("server")].toString();
        m_port = data[QLatin1String("port")].toInt();
    }
}

void plugin::play() {
}

void plugin::stop() {

}
void plugin::next() {

}
void plugin::prev() {
}

void plugin::mute(bool mute)
{
	QByteArray d = mute?"<Volume><Mute>On</Mute></Volume>":"<Volume><Mute>Off</Mute></Volume>";
	sentToYamaha(yamahaPut(yamahaMainZone(d)));
}
void plugin::power(bool power)
{
	QByteArray d = power?"<Power_Control><Power>On</Power></Power_Control>":"<Power_Control><Power>Standby</Power></Power_Control>";
	sentToYamaha(yamahaPut(yamahaMainZone(d)));
}
void plugin::input(int inputchannel)
{
	enum inputchannelenum {
		HDMI1,
		HDMI2,
		AUDIO1,
		AUDIO2,
		NETRADIO
	};
	QByteArray channel;
	switch ((inputchannelenum)inputchannel) {
		case AUDIO1:
			channel = "AUDIO1";
			break;
		case AUDIO2:
			channel = "AUDIO2";
			break;
		case HDMI1:
			channel = "HDMI1";
			break;
		case HDMI2:
			channel = "HDMI2";
			break;
		case NETRADIO:
			channel = "NET RADIO";
			break;
	}
	QByteArray d = "<Input><Input_Sel>"+channel+"</Input_Sel></Input>";
	sentToYamaha(yamahaPut(yamahaMainZone(d)));
	
}
void plugin::output(int outputconfiguration)
{
	enum inputchannelenum {
		STRAIGHT,
		CH2,
		CH7,
		SURROUND,
		MOVIE,
		MOVIE_SCIFI,
		MUSIC_VIDEO
	};
	QByteArray channel;
	switch ((inputchannelenum)outputconfiguration) {
		case STRAIGHT:
			channel = "<Straight>On</Straight>";
			break;
		case CH2:
			channel = "<Straight>Off</Straight><Sound_Program>2ch Stereo</Sound_Program>";
			break;
		case CH7:
			channel = "<Straight>Off</Straight><Sound_Program>7ch Stereo</Sound_Program>";
			break;
		case SURROUND:
			channel = "<Straight>Off</Straight><Sound_Program>Surround Decoder</Sound_Program>";
			break;
		case MOVIE:
			channel = "<Straight>Off</Straight><Sound_Program>Standard</Sound_Program>";
			break;
		case MOVIE_SCIFI:
			channel = "<Straight>Off</Straight><Sound_Program>Sci-Fi</Sound_Program>";
			break;
		case MUSIC_VIDEO:
			channel = "<Straight>Off</Straight><Sound_Program>Music Video</Sound_Program>";
			break;
	}
	QByteArray d = "<Surround><Program_Sel><Current>"+channel+"</Current></Program_Sel></Surround>";
	sentToYamaha(yamahaPut(yamahaMainZone(d)));
}

void plugin::setVolume(int value) {
	QByteArray d = "<Volume><Lvl><Val>"+QByteArray::number(value)+"</Val><Exp>1</Exp><Unit>dB</Unit></Lvl></Volume>";
	sentToYamaha(yamahaPut(yamahaMainZone(d)));
}

void plugin::setVolumeRelative(int value)
{
	QByteArray vs;
	if (value<0) {
		vs += "Down ";
		value *= -1;
		
	} else
		vs += "Up ";
	if (value>5 || value==4)
		value= 5;
	else if (value==3)
		value = 2;
	vs += QByteArray::number(value) + " dB";
	QByteArray d = "<Volume><Lvl><Val>"+vs+"</Val><Exp></Exp><Unit></Unit></Lvl></Volume>";
	sentToYamaha(yamahaPut(yamahaMainZone(d)));
}

void plugin::hostconnected() {
    qDebug() << "Connected to yamahaRX";
}
void plugin::hostdisconnected() {
    qWarning() << "Lost connection to yamahaRX";
}
void plugin::error(QAbstractSocket::SocketError e) {
  // Ignore "host not found" error
  if (e != QAbstractSocket::ConnectionRefusedError && e != QAbstractSocket::HostNotFoundError && e != QAbstractSocket::RemoteHostClosedError)
        qWarning() << "Socket error" << e;
}

void plugin::readyRead() {
    QByteArray data = m_socket.readAll();
	qDebug()<<"Receive"<<data;
}
