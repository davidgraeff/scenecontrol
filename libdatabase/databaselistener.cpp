#include "databaselistener.h"
#include <QDebug>
#include "bson.h"
#include "mongo/bson/bson.h"
#include "servicedata.h"
#include <time.h>

DatabaseListener::DatabaseListener(const QString& serverHostname, QObject* parent):
        QThread(parent), m_abort(false)
{
    m_conn.setSoTimeout(5);
    m_conn.connect(serverHostname.toStdString());
}

DatabaseListener::~DatabaseListener()
{
    m_abort = true;
}

void DatabaseListener::abort()
{
    m_abort = true;
    while (!isFinished());
}

void DatabaseListener::run()
{
    if (m_conn.isFailed())
        return;
    // minKey is smaller than any other possible value
	
	mongo::BSONObjBuilder b;
	b.appendTimestamp("$gt", time (NULL)*1000, 0);
    mongo::BSONElement lastId = b.done().firstElement();
	
    mongo::Query query;
	std::unique_ptr<mongo::DBClientCursor> c;
	
    // capped collection insertion order
    while ( !m_abort ) {
		query = QUERY( "_id" << mongo::GT << lastId).sort("$natural");
		std::cout << query.toString() << std::endl;
		c = m_conn.query("roomcontrol.listen", query, 0, 0, 0, mongo::QueryOption_CursorTailable );
		
        if (c.get()==0) {
            qWarning()<<"Pointer empty!" << QString::fromStdString(m_conn.getLastError());
            sleep(3);
            continue;
        }
        
        while ( 1 ) {
            if ( !c->more() ) {
                if ( c->isDead() ) {
                    // we need to requery
                    break;
                }
                continue; // we will try more() again
            }
            if (m_abort)
                break;
            mongo::BSONObj o = c->next();
            lastId = o["_id"];
            std::string op = o.getStringField("op");
            if (op=="u") {
                const QVariantMap object = BJSON::fromBson(o.getObjectField("o"));
                const QString id = ServiceData::id(object);
                if (id.size())
                    emit doc_changed(id,object);
				else
					std::cout << "update" << o.toString() << std::endl;
            } else if (op=="d") {
                const QVariantMap v = BJSON::fromBson(o.getObjectField("o"));
                emit doc_removed(ServiceData::id(v));
                //qDebug() << "delete" << ServiceData::id(v);
            }
			std::cout << o.toString() << std::endl;
        }

        // prepare to requery from where we left off
        if (m_abort)
            break;
		
		sleep(3);
    }
    exit();
}

