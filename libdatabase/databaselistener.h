#pragma once
#include <QThread>
#include "mongo/client/dbclient.h"
#include <QVariantMap>

class DatabaseListener : public QThread
{
	Q_OBJECT
public:
    explicit DatabaseListener(const QString& serverHostname, QObject* parent = 0);
    virtual ~DatabaseListener();
	void abort();
protected:
    virtual void run();
private:
	mongo::DBClientConnection m_conn;
	bool m_abort;
Q_SIGNALS:
	/// A document changed
    void doc_changed(const QString& id, const QVariantMap& data);
	/// A document has been removed
    void doc_removed(const QString& id);
};

