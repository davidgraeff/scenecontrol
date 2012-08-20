#pragma once
#include <QObject>
#include <QVariantMap>
#include "shared/jsondocuments/scenedocument.h"

/**
 * This singleton object takes QVariantMap data and either execute the request immediatelly
 * if possible or look for a fitting plugin process to execute the request.
 */
class ExecuteRequest : public QObject
{
	Q_OBJECT
public:
	/// Singleton object
    static ExecuteRequest* instance();
    virtual ~ExecuteRequest();
public Q_SLOTS:
	/**
	 * Request execution of the document given by data. If no session id is known use -1.
	 * This slot is used to called by the network controller where a session id is always
	 * available.
	 */
    void requestExecution ( const SceneDocument& doc, int sessionid );
private:
    ExecuteRequest();
};
