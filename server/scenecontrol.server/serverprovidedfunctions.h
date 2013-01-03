#pragma once
#include <shared/jsondocuments/scenedocument.h>

class ServerProvidedFunctions : public QObject
{
	Q_OBJECT
public:
	static void execute(const SceneDocument& data, const QByteArray& responseID, QObject* responseCallbackObject, int sessionid);
	struct VariableEvent {
		QByteArray name;
		QString eventid;
		QString sceneid;
		QVariant compareValue;
	};
private:
	// For delay action
	static void msleep(unsigned long msecs);
	// For Variables (persistant for the server process lifetime)
	static QMap<QByteArray, QByteArray> textVariables;
	static QMap<QByteArray, int> numberVariables;
	static QList<VariableEvent> mVariableEvents;
	// For conditions: emit qt signal response
	void emitSlotResponse(QObject* responseCallbackObject, const QVariant& response, const QByteArray& responseid, const QString& pluginid, const QString& instanceid);
	static bool executeFailure(const QByteArray& method, const QByteArray& argument, bool condition);
Q_SIGNALS:
	void qtSlotResponse(const QVariant& response, const QByteArray& responseid, const QString& pluginid, const QString& instanceid);
};

