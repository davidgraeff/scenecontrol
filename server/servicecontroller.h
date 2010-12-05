#ifndef ServiceController_H
#define ServiceController_H

#include <QtCore/QObject>
#include <QDBusConnection>
#include <QtCore/QObject>
#include <QMap>
#include <QVariantMap>
#include <QTimer>
#include <QDir>
#define EXIT_WITH_RESTART 1

class AbstractPlugin;
class AbstractServiceProvider;
class AbstractStateTracker;
class ExecuteWithBase;
class ExecutePlugin;
class ExecuteService;
class NetworkController;
class ExecuteCollection;

/**
 * Used as singleton to access all other controllers.
 * Initiate controllers and frees them again.
 */
class ServiceController: public QObject
{
    friend class NetworkController;
    Q_OBJECT
public:
    ServiceController ();
    virtual ~ServiceController();
    QList<AbstractStateTracker*> stateTracker();
    /**
     * Refresh data of all plugins.
     */
    void refresh();
private:
    QString serviceFilename(const QByteArray& type, const QString& id);
    /*
     * @link: true if profiles and services should be linked together,
     * Should be false on initial loading
     */
    bool generate ( const QVariantMap& json, bool loading = false);
    void saveToDisk(ExecuteWithBase* eservice);
    void removeFromDisk(ExecuteWithBase* eservice);
    void updateService(ExecuteWithBase* service, bool newid = false, bool loading = false);

    // service providers
    QList<ExecuteWithBase*> m_servicesList;
    QMap<QString, ExecuteWithBase*> m_services;

    // plugins
    QList<ExecutePlugin*> m_plugins;
    QMap<QString, ExecutePlugin*> m_plugin_provider;

    /**
     * Link childs with parents (actions/events/conditions with profiles)
     */
    void addToExecuteProfiles(ExecuteService* service);
    void removeFromExecuteProfiles(ExecuteService* service);

    QDir m_savedir;
public Q_SLOTS:
    void runProfile(const QString& id);
    void stopProfile(const QString& id) ;
    void pluginobjectChanged(ExecuteWithBase* service);
	void pluginexecuteService(AbstractServiceProvider*);
    void executeservice(ExecuteService* service);
Q_SIGNALS:
    void systemStarted();
    void serviceSync(AbstractServiceProvider* p);
    void statetrackerSync(AbstractStateTracker* p);
};

#endif // ServiceController_H
