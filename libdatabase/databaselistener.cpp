#include "databaselistener.h"
#include <QDebug>
#include "bson.h"
#include "servicedata.h"

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
    mongo::BSONElement lastId;
	
	mongo::BSONObjBuilder b;
	b.appendTimestamp("_id");
    mongo::Query query = mongo::Query().sort("$natural"); // { $natural : 1 } means in forward
	query.minKey(b.done());

    // capped collection insertion order
    while ( !m_abort ) {
        std::auto_ptr<mongo::DBClientCursor> c;
        try {
            c = m_conn.query("roomcontrol.listen", query, 0, 0, 0,
                             mongo::QueryOption_CursorTailable );
        } catch (mongo::UserException& e) {
            qWarning()<<"Exception" << QString::fromStdString(e.toString());
            sleep(3);
            continue;
        }
        if (c.get()==0) {
            qWarning()<<"Pointer empty!" << QString::fromStdString(m_conn.getLastError());
            sleep(3);
            continue;
        }
        while ( !m_abort ) {
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
            if (op=="i" || op=="u") {
                const QVariantMap object = BJSON::fromBson(o.getObjectField("o"));
                const QString id = ServiceData::id(object);
                if (id.size())
                    emit doc_changed(id,object);
				else
					std::cout << "update" << o.toString() << endl;
            } else if (op=="d") {
                const QVariantMap v = BJSON::fromBson(o.getObjectField("o"));
                emit doc_removed(ServiceData::id(v));
                //qDebug() << "delete" << ServiceData::id(v);
            } else
                std::cout << o.toString() << endl;
        }

        // prepare to requery from where we left off
        if (m_abort)
            break;
		
        try {
            query = QUERY( "_id" << mongo::GT << lastId).sort("$natural");
        } catch (mongo::UserException& e) {
            qWarning()<<"Exception" << QString::fromStdString(e.toString());
            sleep(3);
            query = mongo::Query().sort("$natural");
        }
    }
    exit();
}

