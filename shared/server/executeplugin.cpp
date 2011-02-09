#include "executeplugin.h"
#include "shared/abstractplugin.h"
#include <QSettings>
#include <QString>
#include <QDebug>

void ExecutePlugin::serverserviceChanged(AbstractServiceProvider* service) {
	emit _serviceChanged(service);
}

void ExecutePlugin::setSetting(const QString& name, const QVariant& value, bool init) {
	QSettings s;
	s.beginGroup(base()->name());
	QVariant oldvalue = s.value(name);
	if (!oldvalue.isValid() || !init)
		s.setValue(name, value);
	m_settings.insert(name, value);
}

void ExecutePlugin::registerSetting(const char* name, const QVariant& value) {
	QSettings s;
	s.beginGroup(base()->name());
	QString oldvalue = s.value(QString::fromAscii(name)).toString();

	QByteArray b;
	if (base())
		b.append(base()->name().toUtf8().replace(' ',"").replace('-','_'));
	b.append('_');
	b.append(name);
	char *valueEnv = getenv ( b.data() );
	if (valueEnv) {
		qDebug() << "Environment variable" << b.data() << valueEnv;
		oldvalue = QString::fromUtf8(valueEnv);
	}


	setSetting(QString::fromAscii(name), (oldvalue.size()?oldvalue:value), true);
}

const QVariantMap ExecutePlugin::getSettings() const {
	return m_settings;
}
