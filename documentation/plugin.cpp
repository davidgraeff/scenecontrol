#include <QDebug>
#include <QtPlugin>

#include "plugin.h"
#include "configplugin.h"

Q_EXPORT_PLUGIN2(libexecute, plugin)

plugin::plugin() {
    _config(this);
}

plugin::~plugin() {

}

void plugin::init(AbstractServer* server) {

}

void plugin::clear() {

}

void plugin::otherPropertyChanged(const QString& unqiue_property_id, const QVariantMap& value) {

}

void plugin::setSetting(const QString& name, const QVariant& value, bool init = false) {
	PluginHelper::setSetting(name, value);
}

void plugin::execute(const QVariantMap& data) {

}

bool plugin::condition(const QVariantMap& data)  {

}

bool plugin::event_changed(const QVariantMap& data) {
  
}

QMap<QString, QVariantMap> plugin::properties() {
	QMap<QString, QVariantMap> l;
	return l;
}
