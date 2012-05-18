/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation
**
****************************************************************************/


#include "qt-bson_p.h"

static bool sMetatypeRegistered = qRegisterMetaType<BsonObject>("BsonObject");

BsonData::BsonData()
{
    memset(&mBsonBuffer, 0, sizeof(mBsonBuffer));
    mBsonBuffer.finished = true;

    bson_empty(&mBson);
    mBsonType = bson_object;
}

BsonData::~BsonData()
{
    if (!mBsonBuffer.finished)
        bson_buffer_destroy(&mBsonBuffer);
    bson_destroy(&mBson);
}

/*!
    The BsonObject class is used to transmit data via the JsonStream to the JsonServer.

    The BsonObject can be used to convert a VariantMap into a binary representation.
    Howeve the Variants used are limited to the following types:
    double, QString, arrays bool, int, long.
*/

BsonObject::BsonObject()
  : d(new BsonData())
{
}
BsonObject::BsonObject(const BsonObject &other)
  : d(other.d)
{
}

/* takes ownership of data */
BsonObject::BsonObject(char *data, int /* size */)
  : d(new BsonData())
{
    bson_init(&d->mBson, data, 1);
    d->mBsonBuffer.finished = true;
}

/*!
    Creates a BsonObject from a byte array which in principle converts it back
    into a QVariantMap (via toMap).
*/
BsonObject::BsonObject(const QByteArray &data)
  : d(new BsonData())
{
    char *mydata = (char *)malloc(data.size()+1);
    memcpy(mydata, data.data(), data.size());
    mydata[data.size()] = 0;
    bson_init(&d->mBson, mydata, 1);
    d->mBsonBuffer.finished = true;
}

BsonObject::BsonObject(bson_iterator *it, const BsonObject *parent, bson_type bt)
  : d(new BsonData())
{
    d->p = parent->d;
    d->mBsonBuffer.finished = true;
    d->mBsonType = bt;
    bson_iterator_subobject(it, &d->mBson);
    finish();
}

BsonObject::BsonObject(const QVariantMap &v)
  : d(new BsonData())
{
    for (QVariantMap::const_iterator it = v.begin(); it != v.end(); ++it) {
        insert(it.key(), it.value());
    }
    finish();
#if 0
    qDebug() << "BsonObject::BsonObject" << v;
    QJson::Serializer serializer;
    qDebug() << serializer.serialize(v);
    bson_print(&d->mBson);
    qDebug() << endl;
#endif
}

BsonObject::BsonObject(const QVariantList &v)
  : d(new BsonData())
{
    d->mBsonType = bson_array;
    for (int i = 0; i < v.size(); i++) {
        insert(QString::number(i), v[i]);
    }
    finish();
}

bool BsonObject::start()
{
    if (d->mBsonBuffer.finished) {
        //qDebug() << "BsonObject::start()" << __LINE__ << QString("%1").arg((long)d.data(), 4, 16);
        bson_buffer_init(&d->mBsonBuffer);
        d->mAddedKeys.clear();
        return true;
    } else {
        return false;
    }
}

void BsonObject::finish()
{
    if (!d->mBsonBuffer.finished) {
        d->mAddedKeys.clear();
        //qDebug() << "BsonObject::finish()" << __LINE__ << QString("%1").arg((long)d.data(), 4, 16);
        if (bson_size(&d->mBson) > 5) {
            bson bson;
            bson_from_buffer(&bson, &d->mBsonBuffer);

            bson_buffer_init(&d->mBsonBuffer);

            QSet<QString> newKeys;

            //qDebug() << "finish()" << bson_size(&bson) << QByteArray(bson.data, bson_size(&bson)).toHex();
            //qDebug() << "BsonObject::finish()" << __LINE__ << QByteArray(d->mBson.data, bson_size(&d->mBson)).toHex();

            if (bson_size(&bson) > 5) {
                bson_iterator j;
                bson_iterator_init(&j, bson.data);
                while (bson_iterator_next(&j) != bson_eoo) {
                    const char *jkey = bson_iterator_key(&j);
                    QString s = QString::fromUtf8(jkey);
                    if (newKeys.contains(s)) {
                        qCritical() << "BsonObject::finish" << "duplicate key" << s;
                    }
                    newKeys.insert(s);

                    //qDebug() << "key" << jkey << bson_iterator_type(&j) << QString(s).toLatin1().toHex();
                    bson_append_element(&d->mBsonBuffer, 0, &j);
                }
            }
            bson_destroy(&bson);

            bson_iterator i;
            bson_iterator_init(&i, d->mBson.data);
            while (bson_iterator_next(&i) != bson_eoo) {
                const char *ikey = bson_iterator_key(&i);
                QString s = QString::fromUtf8(ikey);
                if (newKeys.contains(s)) {
                    continue;
                }
                newKeys.insert(s);
                bson_append_element(&d->mBsonBuffer, 0, &i);
            }
            bson_destroy(&d->mBson);
        }
        bson_from_buffer(&d->mBson, &d->mBsonBuffer);
        //qDebug() << "BsonObject::finish()" << __LINE__ << QByteArray(d->mBson.data, bson_size(&d->mBson)).toHex();
    }
}

QVariantMap BsonObject::toMap()
{
    finish();
    //qDebug() << "toMap()" << bson_size(&d->mBson) << QByteArray(d->mBson.data, bson_size(&d->mBson)).toHex();

    QVariantMap map;
    bson_iterator it;
    bson_iterator_init(&it, d->mBson.data);
    bson_type bt;
    while ((bt = bson_iterator_next(&it)) && (bt != bson_eoo)) {
        const char *ckey = bson_iterator_key(&it);
        QVariant v = elementToVariant(bt, &it);
        QString s = QString::fromUtf8(ckey);
        if (map.contains(s))
            qDebug() << "BsonObject::toMap" << "duplicate key" << ckey;
        map.insert(s, v);
    }
    return map;
}

QVariantList BsonObject::toList()
{
    finish();

    QVariantList list;
    if (d->mBsonType != bson_array)
        return list;

    bson_iterator it;
    bson_iterator_init(&it, d->mBson.data);
    bson_type bt;
    while ((bt = bson_iterator_next(&it)) && (bt != bson_eoo)) {
        const char *ckey = bson_iterator_key(&it);
        int i = strtoul(ckey, 0, 10);
        QVariant v = elementToVariant(bt, &it);
        list.insert(i, v);
    }
    return list;
}

QList<BsonObject> BsonObject::toBsonList()
{
    finish();

    QList<BsonObject> list;
    if (d->mBsonType != bson_array)
        return list;

    bson_iterator it;
    bson_iterator_init(&it, d->mBson.data);
    bson_type bt;
    while ((bt = bson_iterator_next(&it)) && (bt != bson_eoo)) {
        list.append(BsonObject(&it, this, bt));
    }
    return list;
}

QList<QString> BsonObject::keys()
{
    finish();

    QList<QString> keys;
    bson_iterator it;
    bson_iterator_init(&it, d->mBson.data);
    bson_type bt;
    while ((bt = bson_iterator_next(&it)) && (bt != bson_eoo)) {
        keys.append(QString::fromUtf8(bson_iterator_key(&it)));
    }
    return keys;
}

QVariant BsonObject::elementToVariant(bson_type bt, bson_iterator *it)
{
    QVariant v;
    switch (bt) {
    case bson_double:
        v = bson_iterator_double(it);
        break;
    case bson_string:
        v = QString::fromUtf8(bson_iterator_string(it));
        break;
    case bson_utf16:
        v = QString((QChar *)bson_iterator_utf16(it), bson_iterator_utf16_numchars(it));
        break;
    case bson_object: {
        BsonObject subBson(it, this, bt);
        v = subBson.toMap();
    } break;
    case bson_array: {
        BsonObject subBson(it, this, bt);
        v = subBson.toList();
    } break;
    case bson_bool:
        v = bson_iterator_bool(it);
        break;
    case bson_int:
        v = bson_iterator_int(it);
        break;
    case bson_long:
        v = (qlonglong)bson_iterator_long(it);
        break;
    case bson_undefined:
        v = QVariant();
        break;
    case bson_eoo:
    case bson_timestamp:
    case bson_bindata:
    case bson_oid:
    case bson_date:
    case bson_null:
    case bson_regex:
    case bson_dbref:
    case bson_code:
    case bson_symbol:
    case bson_codewscope:
        qCritical() << "BsonObject::elementToVariant: unimplemented conversion of bson_type: " << bt;
        break;
    }
    return v;
}

QByteArray BsonObject::data()
{
    finish();

    return QByteArray(d->mBson.data, bson_size(&d->mBson));
}
int BsonObject::dataSize()
{
    finish();

    return bson_size(&d->mBson);
}
bool BsonObject::isEmpty()
{
    finish();

    return (bson_size(&d->mBson) <= 5);
}
int BsonObject::size()
{
    return count();
}
int BsonObject::count()
{
    finish();

    int count = 0;
    bson_iterator it;
    bson_iterator_init(&it, d->mBson.data);
    bson_type bt;
    while ((bt = bson_iterator_next(&it)) && (bt != bson_eoo)) {
        count++;
    }
    return count;
}

BsonObject &BsonObject::insert(const QString &key, int v)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    start();
    d->mAddedKeys.insert(key);
    bson_append_int(&d->mBsonBuffer, key.toUtf8().data(), v);
    return *this;
}

BsonObject &BsonObject::insert(const QString &key, quint32 v)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    start();
    d->mAddedKeys.insert(key);
    bson_append_int(&d->mBsonBuffer, key.toUtf8().data(), v);
    return *this;
}

BsonObject &BsonObject::insert(const QString &key, double v)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    start();
    d->mAddedKeys.insert(key);
    bson_append_double(&d->mBsonBuffer, key.toUtf8().data(), v);
    return *this;
}

BsonObject &BsonObject::insert(const QString &key, bool v)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    start();
    d->mAddedKeys.insert(key);
    bson_append_bool(&d->mBsonBuffer, key.toUtf8().data(), v);
    return *this;
}

BsonObject &BsonObject::insert(const QString &key, const char *s)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    d->mAddedKeys.insert(key);
    return insert(key, QString::fromUtf8(s));
}

BsonObject &BsonObject::insert(const QString &key, const QString &v)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    start();
    d->mAddedKeys.insert(key);
    bson_append_utf16(&d->mBsonBuffer, key.toUtf8().data(), v.constData(), v.size());
    return *this;
}

BsonObject &BsonObject::insert(const QString &key, const QVariantMap &v)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    start();
    d->mAddedKeys.insert(key);
    BsonObject b(v);
    b.finish();
    bson_append_bson(&d->mBsonBuffer, key.toUtf8().data(), b.d->mBsonType, &b.d->mBson);
    return *this;
}

BsonObject &BsonObject::insert(const QString &key, const QVariant &v)
{
    if (!v.isValid()) {
        if (false && d->mAddedKeys.contains(key)) {
            finish();
        }
        start();
        d->mAddedKeys.insert(key);
        bson_append_undefined(&d->mBsonBuffer, key.toUtf8().data());
        return *this;
    }
    switch (v.type()) {
    case QVariant::Bool:
        insert(key, v.toBool());
        break;
    case QVariant::Int:
        insert(key, v.toInt());
        break;
    case QVariant::Double:
        insert(key, v.toDouble());
        break;
    case QVariant::String:
        insert(key, v.toString());
        break;
    case QVariant::List:
        insert(key, v.toList());
        break;
    case QVariant::Map:
        insert(key, v.toMap());
        break;
    default:
        insert(key, v.toString());
        break;
    }
    return *this;
}

BsonObject &BsonObject::insert(const QString &key, const QVariantList &list)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    start();
    d->mAddedKeys.insert(key);
    bson_append_start_array(&d->mBsonBuffer, key.toUtf8().data());
    for (int i = 0; i < list.size(); i++) {
        insert(QString::number(i), list[i]);
    }
    bson_append_finish_object(&d->mBsonBuffer);
    return *this;
}

BsonObject &BsonObject::insert(const QString &key, const BsonList &list)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    start();
    d->mAddedKeys.insert(key);
    bson_append_start_array(&d->mBsonBuffer, key.toUtf8().data());
    for (int i = 0; i < list.size(); i++) {
        insert(QString::number(i), list[i]);
    }
    bson_append_finish_object(&d->mBsonBuffer);
    return *this;
}


BsonObject &BsonObject::insert(const QString &key, const QStringList &list)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    start();
    d->mAddedKeys.insert(key);
    bson_append_start_array(&d->mBsonBuffer, key.toUtf8().data());
    for (int i = 0; i < list.size(); i++) {
        insert(QString::number(i), list[i]);
    }
    bson_append_finish_object(&d->mBsonBuffer);
    return *this;
}

BsonObject &BsonObject::insert(const QString &key, BsonObject b)
{
    if (false && d->mAddedKeys.contains(key)) {
      finish();
    }
    start();
    d->mAddedKeys.insert(key);

    Q_ASSERT(!d->mBsonBuffer.finished);
    b.finish();
    bson_append_bson(&d->mBsonBuffer, key.toUtf8().data(), b.d->mBsonType, &b.d->mBson);

    return *this;
}
bool BsonObject::contains(const QString &key)
{
    finish();
    bson_iterator it;
    bson_type t = bson_find(&it, &d->mBson, key.toUtf8().data());
    return (t != bson_eoo);
}

BsonObject BsonObject::subObject(const QString &key)
{
    finish();
    bson_iterator it;
    bson_type bt = bson_find(&it, &d->mBson, key.toUtf8().data());
    if ((bt == bson_object) || (bt == bson_array))
        return BsonObject(&it, this, bt);
    else
        return BsonObject();
}
BsonList BsonObject::subList(const QString &key)
{
    finish();
    bson_iterator it;
    bson_type t = bson_find(&it, &d->mBson, key.toUtf8().data());
    BsonList list;
    if (t != bson_array)
        return list;
    BsonObject o(&it, this, t);

    bson_iterator_init(&it, o.d->mBson.data);
    bson_type bt;
    while ((bt = bson_iterator_next(&it)) && (bt != bson_eoo)) {
        list.append(BsonObject(&it, this, bt));
    }
    return list;
}

QVariant BsonObject::value(const QString &key)
{
    finish();
    bson_iterator it;

    bson_type t = bson_find(&it, &d->mBson, key.toUtf8().data());
    switch (t) {
    case bson_double:
        return bson_iterator_double(&it);
    case bson_string:
        return QString::fromUtf8(bson_iterator_string(&it));
    case bson_utf16:
        return QString((QChar *)bson_iterator_utf16(&it), bson_iterator_utf16_numchars(&it));
    case bson_array:
    case bson_object: {
        BsonObject bson(&it, this, t);
        if (t == bson_object)
            return bson.toMap();
        else
            return bson.toList();
    }
    case bson_bool:
        return bson_iterator_bool(&it);
    case bson_null:
        return QVariant();
    case bson_int:
        return bson_iterator_int(&it);
    case bson_long:
        return (qlonglong)bson_iterator_long(&it);
    case bson_undefined:
    case bson_eoo: // when attempting to fetch a value from an empty object
        return QVariant();
    default:
        qCritical() << QString::fromLatin1("Converting BsonObject element of type %1 to QVariant()").arg(t);
        return QVariant();
    }
}

int BsonObject::valueInt(const QString &key, int default_value)
{
    finish();
    bson_iterator it;

    bson_type t = bson_find(&it, &d->mBson, key.toUtf8().data());
    if (t == bson_eoo)
        return default_value;

    switch (t) {
    case bson_double:
        return (int)bson_iterator_double(&it);
    case bson_string:
        return QString::fromLatin1(bson_iterator_string(&it)).toInt();
    case bson_null:
        return 0;
    case bson_int:
        return bson_iterator_int(&it);
    case bson_long:
        return (int)bson_iterator_long(&it);
    case bson_array:
    case bson_object:
    case bson_bool:
    default:
        return default_value;
    }
}

QString BsonObject::valueString(const QString &key)
{
    finish();
    bson_iterator it;

    bson_type t = bson_find(&it, &d->mBson, key.toUtf8().data());
    switch (t) {
    case bson_string:
        return QString::fromUtf8(bson_iterator_string(&it));
    case bson_utf16:
        return QString((QChar *)bson_iterator_utf16(&it), bson_iterator_utf16_numchars(&it));
    default:
        return value(key).toString();
    }
}

void BsonObject::dump()
{
    finish();
    bson_print(&d->mBson);
}

#if 0
BsonList::BsonList()
{
    d->mBsonType = bson_array;
}
BsonList::BsonList(const QVariantList &list)
{
    d->mBsonType = bson_array;
    for (int i = 0; i < list.size(); i++) {
        insert(QByteArray::number(i), list[i]);
    }
    finish();
}
#endif
