#include "execute/executerequest.h"
#include "execute/collectioncontroller.h"
#include "plugins/plugincontroller.h"
#include "shared/jsondocuments/scenedocument.h"
#include "libdatastorage/datastorage.h"
#include "socket.h"
#include "config.h"
#include <plugins/pluginprocess.h>

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

void ExecuteRequest::requestExecution(const SceneDocument& doc, int sessionid) {
    if ( !doc.checkType(SceneDocument::TypeExecution )) return;
    // Special case: a method of the server process should be executed. handle this immediatelly
    if (doc.componentID()==QLatin1String("server") && sessionid != -1) {
        if (doc.isMethod("requestAllProperties"))
            PluginController::instance()->requestAllProperties(sessionid);
        else if (doc.isMethod("runcollection"))
            CollectionController::instance()->requestExecutionByCollectionId(doc.sceneid());
        else if (doc.isMethod("database")) {
            //Database* b = Database::instance();
            SceneDocument s = SceneDocument::createNotification("database");
            //s.setData("database", b->databaseAddress());
            //s.setData("state", b->state());
            s.setComponentID(QLatin1String("server"));
            Socket::instance()->propagateProperty(s.getData(), sessionid);
        } else if (doc.isMethod("version")) {
            SceneDocument s = SceneDocument::createNotification("version");
            s.setData("version", QLatin1String(ABOUT_VERSION));
            s.setComponentID(QLatin1String("server"));
            Socket::instance()->propagateProperty(s.getData(), sessionid);
        }
        return;
    }
    // Look for a plugin that fits "data"
    PluginProcess* plugin = PluginController::instance()->getPlugin ( doc.componentUniqueID() );
    if ( !plugin ) {
        qWarning() <<"Cannot execute service. No plugin found:"<<doc.getData() << sessionid;
        return;
    }
    plugin->callQtSlot ( doc.getData(), QByteArray(), sessionid );
}
