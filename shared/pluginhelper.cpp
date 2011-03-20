#include "pluginhelper.h"
#include <QSettings>
#include <QString>
#include <QDebug>
#include <cstdlib> //getenv
#include "abstractserver.h"

void PluginHelper::setSetting ( const QString& name, const QVariant& value, bool init ) {
    QSettings s;
    s.beginGroup ( pluginid() );
    QVariant oldvalue = s.value ( name );
    if ( !oldvalue.isValid() || !init )
        s.setValue ( name, value );
    m_settings.insert ( name, value );
}

void PluginHelper::registerSetting ( const char* name, const QVariant& value ) {
    QSettings s;
    s.beginGroup ( pluginid() );
    QString oldvalue = s.value ( QString::fromAscii ( name ) ).toString();

    QByteArray b;
    b.append ( pluginid().toUtf8().replace ( ' ',"" ).replace ( '-','_' ) );
    b.append ( '_' );
    b.append ( name );
    char *valueEnv = getenv ( b.data() );
    if ( valueEnv ) {
        qDebug() << "Environment variable" << b.data() << valueEnv;
        oldvalue = QString::fromUtf8 ( valueEnv );
    }


    setSetting ( QString::fromAscii ( name ), ( oldvalue.size() ?oldvalue:value ), true );
}

const QVariantMap PluginHelper::getSettings() const {
    return m_settings;
}

void PluginHelper::session_change ( const QString& id, bool running ) {
    if ( running ) m_sessions.insert ( id );
    else m_sessions.remove ( id );
}

void PluginHelper::otherPropertyChanged ( const QVariantMap& data, const QString& sessionid ) {
    Q_UNUSED ( data );
    Q_UNUSED ( sessionid );
}

void PluginHelper::initialize_plugin ( AbstractServer* server ) {m_server = server; initialize();}
