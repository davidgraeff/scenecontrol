#ifndef BSON_H
#define BSON_H

#include <QVariant>
#include <QDateTime>
#include <mongo/client/dbclient.h>

class BJSON {
public:
	static mongo::BSONObj toBson(const QVariantMap &obj);
	static QVariantMap fromBson(mongo::BSONObj bson);
};

#endif // BSON_H
