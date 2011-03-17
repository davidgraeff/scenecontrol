#include "pluginhelper.h"
#include <QSettings>
#include <QString>
#include <QDebug>
#include <cstdlib> //getenv

void PluginHelper::setSetting(const QString& name, const QVariant& value, bool init) {
	QSettings s;
	s.beginGroup(pluginid());
	QVariant oldvalue = s.value(name);
	if (!oldvalue.isValid() || !init)
		s.setValue(name, value);
	m_settings.insert(name, value);
}

void PluginHelper::registerSetting(const char* name, const QVariant& value) {
	QSettings s;
	s.beginGroup(pluginid());
	QString oldvalue = s.value(QString::fromAscii(name)).toString();

	QByteArray b;
	b.append(pluginid().toUtf8().replace(' ',"").replace('-','_'));
	b.append('_');
	b.append(name);
	char *valueEnv = getenv ( b.data() );
	if (valueEnv) {
		qDebug() << "Environment variable" << b.data() << valueEnv;
		oldvalue = QString::fromUtf8(valueEnv);
	}


	setSetting(QString::fromAscii(name), (oldvalue.size()?oldvalue:value), true);
}

const QVariantMap PluginHelper::getSettings() const {
	return m_settings;
}
