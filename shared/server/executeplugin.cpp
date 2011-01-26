#include "executeplugin.h"
#include "shared/abstractplugin.h"
#include <QDebug>

void ExecutePlugin::serverserviceChanged(AbstractServiceProvider* service) {
	emit _serviceChanged(service);
}

void ExecutePlugin::setSetting(const QString& name, const QVariant& value) {
	m_settings.insert(name, value);
}

void ExecutePlugin::registerSetting(const char* name, const QVariant& value) {
	QByteArray b;
	if (base())
		b.append(base()->name().toUtf8().replace(' ',"").replace('-','_'));
	b.append('_');
	b.append(name);
	char *valueEnv = getenv ( b.data() );
	if (valueEnv)
		qDebug() << "Environment variable" << b.data() << valueEnv;
	setSetting(QString::fromAscii(name), (valueEnv?QString::fromUtf8(valueEnv):value));
}

const QVariantMap ExecutePlugin::getSettings() const {
	return m_settings;
}
