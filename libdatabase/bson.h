#ifndef BSON_H
#define BSON_H

#include <QVariant>
#include <QDateTime>
#include <mongo/client/dbclient.h>

namespace BJSON {
	mongo::BSONObj toBson(const QVariantMap &obj);
	QVariantMap fromBson(mongo::BSONObj bson);
};

#endif // BSON_H
