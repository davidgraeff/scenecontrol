#include "executerequest.h"
#include <shared/pluginservicehelper.h>
#include "plugincontroller.h"
#include "socket.h"
#include "config.h"
#include "collectioncontroller.h"
#include "pluginprocess.h"
#include <shared/database.h>

static ExecuteRequest* ExecuteRequest_instance = 0;

ExecuteRequest* ExecuteRequest::instance() {
    if (!ExecuteRequest_instance) {
        ExecuteRequest_instance = new ExecuteRequest();
    }
    return ExecuteRequest_instance;
}

ExecuteRequest::ExecuteRequest()
{

}

ExecuteRequest::~ExecuteRequest()
{

}

void ExecuteRequest::requestExecution(const QVariantMap& data, int sessionid) {
    if ( !ServiceData::checkType ( data, ServiceData::TypeExecution )) return;
    // Special case: all properties are requested, handle this immediatelly.
    if (ServiceData::pluginid(data)==QLatin1String("server") && sessionid != -1) {
        if (ServiceData::isMethod(data, "requestAllProperties"))
            PluginController::instance()->requestAllProperties(sessionid);
        else if (ServiceData::isMethod(data, "runcollection"))
            CollectionController::instance()->requestExecutionByCollectionId(ServiceData::collectionid(data));
        else if (ServiceData::isMethod(data, "database")) {
            ServiceData s = ServiceData::createNotification("database");
            s.setData("database", Database::instance()->databaseAddress());
            s.setPluginid("server");
            Socket::instance()->propagateProperty(s.getData(), sessionid);
		} else if (ServiceData::isMethod(data, "version")) {
            ServiceData s = ServiceData::createNotification("version");
            s.setData("version", QLatin1String(ABOUT_VERSION));
            s.setPluginid("server");
            Socket::instance()->propagateProperty(s.getData(), sessionid);
        }
        return;
    }
    // Look for a plugin that fits "data"
    PluginCommunication* plugin = PluginController::instance()->getPlugin ( ServiceData::pluginid ( data ) );
    if ( !plugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<data << sessionid;
        return;
    }

    plugin->callQtSlot ( data, QByteArray(), sessionid );
}