
#include <QDebug>
#include "plugin.h"

#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    plugin p;
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin() : AbstractPlugin(this) {}

plugin::~plugin() {}

void plugin::clear() {}
void plugin::initialize() {}

void plugin::configChanged(const QByteArray& configid, const QVariantMap& data) {
  Q_UNUSED(configid);
    if (data.contains(QLatin1String("server")) && data.contains(QLatin1String("port"))) {
//         delete m_xbmcClient;
//         m_xbmcClient = new CXBMCClient( data[QLatin1String("server")].toString().toAscii(),
//                                         data[QLatin1String("port")].toInt());
//         m_xbmcClient->SendHELO(QCoreApplication::applicationName().toLatin1().constData(), ICON_NONE);
    }
}

void plugin::play() {}
void plugin::pause() {}
void plugin::stop() {}
void plugin::next() {}
void plugin::prev() {}
void plugin::info() {}
void plugin::AspectRatio() {}
void plugin::NextSubtitle() {}
void plugin::AudioNextLanguage () {}
void plugin::previousmenu() {}
void plugin::ActivateWindow() {}
void plugin::select() {}
void plugin::down() {}
void plugin::up() {}
void plugin::left() {}
void plugin::right() {}
void plugin::close() {}
void plugin::ContextMenu() {}
void plugin::FastForward() {}
void plugin::Rewind() {}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(data);
}
