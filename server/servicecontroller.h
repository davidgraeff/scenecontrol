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

class EventController;
class ExecuteService;
class AbstractPlugin;
class AbstractServiceProvider;
class ExecuteWithBase;
class ExecutePlugin;
class Category;
class NetworkController;
class ExecuteCollection;
class AbstractStateTracker;

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
    QString serviceFilename(AbstractServiceProvider* service);
    /*
     * @link: true if profiles and services should be linked together,
     * Should be false on initial loading
     */
    bool generate ( const QVariantMap& json);
    void saveToDisk(ExecuteWithBase* service);
    void removeFromDisk(ExecuteWithBase* service);
    void updateService(ExecuteWithBase* service, const QVariantMap& json, bool newid = false);

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
Q_SIGNALS:
    void systemStarted();
    void serviceSync(AbstractServiceProvider* p);
    void statetrackerSync(AbstractStateTracker* p);
};

#endif // ServiceController_H
