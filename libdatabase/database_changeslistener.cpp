#include "database_changeslistener.h"

DatabaseChangesListener::DatabaseChangesListener()
{

}

DatabaseChangesListener::~DatabaseChangesListener()
{

}

void DatabaseChangesListener::startChangeListener()
{
//     QNetworkRequest request(couchdbAbsoluteUrl(QLatin1String("_changes?feed=continuous&since=%1&heartbeat=5000")).arg(m_last_changes_seq_nr));
//     request.setRawHeader("Connection", "keep-alive");
//     m_listenerReply = get(request);
//     connect(m_listenerReply, SIGNAL(readyRead()), SLOT(replyChange()));
}

// void DatabaseChangesListener::replyChange()
// {
//     if (checkFailure(m_listenerReply, "replyChange failed")) {
//         m_listenerReply = 0;
//         return;
//     }
// 
//     while (m_listenerReply->canReadLine()) {
//         const QByteArray line = m_listenerReply->readLine();
//         if (line.size() <= 1) continue;
//         QVariantMap data = JSON::parse(line).toMap();
//         if (data.isEmpty() || !data.contains(QLatin1String("seq")))
//             continue;
// 
//         const int seq = data.value(QLatin1String("seq")).toInt();
// 
//         if (seq > m_last_changes_seq_nr)
//             m_last_changes_seq_nr = seq;
// 
//         const QString docid = ServiceData::idChangeSeq(data);
// 
//         if (data.contains(QLatin1String("deleted"))) {
//             emit doc_removed(docid);
//             continue;
//         }
// 
//         // request document that is mentioned in the changes feed
//         QNetworkRequest request(couchdbAbsoluteUrl(docid));
// 
//         QNetworkReply *r = get(request);
//         QEventLoop eventLoop;
//         connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
//         eventLoop.exec();
// 
//         QVariantMap document = JSON::parse(r->readAll()).toMap();
// 
//         emit doc_changed(ServiceData::id(document), document);
//     }
// }
