#include "bson.h"
#include <QDebug>

namespace BJSON {

QVariant fromBSonValue(mongo::BSONElement e) {
    switch (e.type()) {
    case mongo::NumberDouble:
        return e.numberDouble();
    case mongo::String:
        return QString::fromStdString(e.str());
    case mongo::Undefined:
        return QVariant();
    case mongo::jstOID:
        return QString::fromStdString(e.__oid().str());
    case mongo::jstNULL:
        return QVariant();
    case mongo::Bool:
        return e.boolean();
    case mongo::CodeWScope:
        return QString(QLatin1String("MongoCode[%1]")).arg(QString::fromAscii(e.codeWScopeCode()));
    case mongo::NumberInt:
        return e.numberInt();
    case mongo::Object:
        return fromBson(e.Obj());
    case mongo::Array: {
        std::vector< mongo::BSONElement > arrayElements = e.Array();
        QVariantList list;
        for (unsigned int i=0;i<arrayElements.size();++i)
            list.append(fromBSonValue(arrayElements[i]));
        return list;
        break;
    }
    case mongo::EOO:
        return QVariant();
    default:
        qCritical() << "fromBson() type" << e.type() << "unknown!";
        Q_ASSERT(false);
        return QVariant();
    }
}

QVariantMap fromBson(mongo::BSONObj bson)
{
    QVariantMap obj;

    for (mongo::BSONObjIterator i(bson); i.more();) {
        mongo::BSONElement e = i.next();
        obj[QString::fromStdString(e.fieldName())] = fromBSonValue(e);
    }
    return obj;
}

mongo::BSONObj toBson(const QVariantMap &obj)
{
    mongo::BSONObjBuilder b;

    QVariantMap::const_iterator it = obj.constBegin();
    while (it != obj.constEnd()) {
        QByteArray byteName = it.key().toStdString().c_str();
        const char *name = byteName.constData();
        QVariant v = it.value();
        int type = v.type();

        bool ok = true;
        switch (type) {
        case QVariant::Int:
            b.append(name, v.toInt(&ok));
            break;
        case QVariant::String:
            b.append(name, v.toString().toStdString());
            break;
        case QVariant::LongLong:
            b.append(name, v.toLongLong(&ok));
            break;
        case QVariant::UInt:
            b.append(name, v.toUInt(&ok));
            break;
        case QVariant::Map:
            b.append(name, toBson(v.toMap()));
            break;
        case QVariant::Double:
            b.append(name, v.toDouble(&ok));
            break;
        case QVariant::Bool:
            b.appendBool(name, v.toBool());
            break;
        case QVariant::Time:
            b.appendTimeT(name, v.toDateTime().toTime_t());
            break;
        case QVariant::Invalid:
            b.appendUndefined(name);
            break;
        case QVariant::List: {
            QVariantList list = v.toList();
            mongo::BSONArrayBuilder arr;
            for (int i = 0; i < list.size(); ++i) {
                const QVariant v = list[i];
                switch (v.type()) {
                case QVariant::Int:
                    arr.append(v.toInt(&ok));
                    break;
                case QVariant::String:
                    arr.append(v.toString().toStdString());
                    break;
                case QVariant::LongLong:
                    arr.append(v.toLongLong(&ok));
                    break;
                case QVariant::UInt:
                    arr.append(v.toUInt(&ok));
                    break;
                case QVariant::Map:
                    arr.append(toBson(v.toMap()));
                    break;
                case QVariant::Double:
                    arr.append(v.toDouble(&ok));
                    break;
                case QVariant::Bool:
                    arr.append(v.toBool());
                    break;
                default:
                    qCritical() << "toBson() failed to convert list" << obj << "list for" << name << "with type" << v.typeName();
                }
            }
            b.appendArray(name, arr.done());
        }
        break;
        default:
            qCritical() << "toBson() failed to convert" << obj << "for" << name << "with type" << v.typeName();
            ok = false;
        }
        Q_ASSERT(ok);
        ++it;
    }
    return b.obj();
}
};
