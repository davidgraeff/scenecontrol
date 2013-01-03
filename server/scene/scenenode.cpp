#include "scenenode.h"
#include <libdatastorage/datastorage.h>
#include <plugins/plugincontroller.h>
#include <qeventloop.h>
#include <QElapsedTimer>

SceneNode* SceneNode::createEmptyNode() {return new SceneNode(SceneDocument::TypeUnknown,QString(),QList<QString>(),QList<QString>());}
SceneNode* SceneNode::createNode(SceneDocument::TypeEnum type, const QString& id, const QList<QString>& nextNodes, const QList<QString>& alternativeNextNodes) {
	return new SceneNode(type, id, nextNodes, alternativeNextNodes);
}

SceneDocument::TypeEnum SceneNode::getType() const { return mType; }
QString SceneNode::getID() const { return mID; }

void SceneNode::setNextNodes(const QList<QString>& nextNodes)
{
	mNextNodes = nextNodes;
}

QList<QString> SceneNode::run() {
	SceneDocument doc = DataStorage::instance()->getDocumentCopy(SceneDocument::uid(mType,mID));
	if (!doc.isValid()) {
		qWarning() << "SceneNode::Run(): Document not valid!";
		return QList<QString>();
	}
	switch (doc.type()) {
		case SceneDocument::TypeAction:
			PluginController::instance()->execute(doc);
			return mNextNodes;
		case SceneDocument::TypeCondition:
		{
			// Only execute next nodes if no plugin for this condition can be found: Otherwise wait for the pluginResponse callback
			if (!PluginController::instance()->execute(doc, mID.toAscii(), this)) {
				qWarning() << "SceneNode::Run(): Condition failed. Plugin does not exist" << doc.getjson();
				return mNextNodes;
			}
			QEventLoop eventloop;
			QElapsedTimer timer;
			timer.start();
			QTimer t;
			connect(&t, SIGNAL(timeout()), SLOT(timeout()));
			t.setSingleShot(true);
			t.setInterval(600);
			t.start();
			qDebug() << "Wait for condition response";
			while (!timer.hasExpired(600)) { // wait at most 600ms for a response
				eventloop.processEvents(QEventLoop::AllEvents|QEventLoop::WaitForMoreEvents);
				qDebug() << "Wait for condition response 2";
				if (mPluginResponseAvailable) {
					if (mResponse)
						return mNextNodes;
					else
						return mNextAlternativeNodes;
				}
			}
			
			qWarning() << "SceneNode::Run(): Condition failed. Plugin does not respond" << doc.getjson();
			return mNextNodes;
			break;
		}
		case SceneDocument::TypeEvent:
			return mNextNodes;
		default:
			return QList<QString>();
	}
}

void SceneNode::pluginResponse(const QVariant& response, const QByteArray& responseid, const QString& pluginid, const QString& instanceid) {
	// We only want a reponse once
	sender()->disconnect(this);
	
	qDebug() << "Condition response 1";
	
	// This response is not addressed to us
	if (responseid != mID.toAscii())
		return;
	
	// Only use boolean responses
	if (!response.canConvert(QVariant::Bool)) {
		qWarning() << "Condition failed." << responseid << pluginid << instanceid << "Not a boolean response" << response;
		return;
	}
	
	qDebug() << "Condition response 2";
	// Either execute next nodes or alternative next nodes
	mResponse = response.toBool();
	mPluginResponseAvailable = 1;
}

SceneNode::SceneNode(SceneDocument::TypeEnum type, const QString& id, const QList<QString>& nextNodes, const QList<QString>& alternativeNextNodes) : mType(type), mID(id), mNextNodes(nextNodes), mNextAlternativeNodes(alternativeNextNodes) {}
