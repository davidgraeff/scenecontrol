/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Purpose: Load plugins, load description xmls, route services and properties
*/

#pragma once
#include <QtCore/QObject>
#include <QMap>
#include <QVariantMap>
#include <QTimer>
#include <QDir>
#include <QtXml/QDomDocument>
#include <QFileSystemWatcher>
#include <shared/abstractplugin.h>
#include <shared/abstractplugin_services.h>
#include <shared/abstractplugin_otherproperties.h>
#include <shared/abstractplugin_settings.h>
#include <shared/abstractplugin_sessions.h>

class ServiceController;
class PluginInfo {
public:
    AbstractPlugin* plugin;
    QString version;
    void setVersion(const QString& version) {
        this->version = version;
    }
    PluginInfo(AbstractPlugin* plugin) {
        this->plugin=plugin;
    }
    ~PluginInfo() {/* do not delete plugin. will be done by QPluginLoader automaticly */ }
};

class PluginController: public QObject
{
    Q_OBJECT
public:
    PluginController (ServiceController* servicecontroller);
    virtual ~PluginController();
    void initializePlugins();
    int knownServices();
    /**
     * For objects to register their interface. E.g. the backup object to
     * execute backup actions and send changed properties to \link ServiceController.
     * The handling is the same like a plugin from file: Validation and service description
     * is made via an xml file in xml/[object-class-name].xml.
     */
    void registerPluginFromObject(AbstractPlugin* object, ServiceController* servicecontroller);
    /**
     * Deregister an object (e.g. before deleting them). Otherwise ServiceController
     * might try to acces them after deletion.
     */
    void deregisterPluginFromObject(AbstractPlugin* object, ServiceController* servicecontroller);

    AbstractPlugin* getPlugin(const QString& pluginid);
    QDomNode* getPluginDom(const QString& service_gid);

    QMap<QString,PluginInfo*>::iterator getPluginIterator();
    AbstractPlugin* nextPlugin(QMap<QString,PluginInfo*>::iterator& index);
	AbstractPlugin_settings* nextSettingsPlugin(QMap<QString,PluginInfo*>::iterator& index);
    AbstractPlugin_services* nextServicePlugin(QMap<QString,PluginInfo*>::iterator& index);
	AbstractPlugin_sessions* nextSessionPlugin(QMap<QString,PluginInfo*>::iterator& index);

private:
    QMap<QString,PluginInfo*> m_plugins;
    QMap<QString, QDomNode*> m_id_to_xml;

    /**
     * Load plugins and their corresponding xml description file and
     * registers all provided properties and services
     */
    void loadXML(const QString& filename);

    int m_index;
};
