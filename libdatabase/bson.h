#pragma once

#include <QVariant>
#include <QDateTime>
#include <mongo/client/dbclient.h>

namespace BJSON {
	mongo::BSONObj toBson(const QVariantMap &obj);
	QVariantMap fromBson(mongo::BSONObj bson);
};
