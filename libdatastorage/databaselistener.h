#pragma once
#include <QObject>
#include <QVariantMap>

class SceneDocument;
class DataStorageWatcher : public QObject
{
	Q_OBJECT
public:
    explicit DataStorageWatcher(QObject* parent = 0);
    virtual ~DataStorageWatcher();
	void watchdir(const QString& dir);
private:
	int m_inotify_fd;
	QHash<QString, int> pathToID;
	QHash<int, QString> idToPath;
private Q_SLOTS:
	void readnotify();
Q_SIGNALS:
	/// Document changed or created
    void fileChanged(const QString &filename);
	/// Document has been removed
    void fileRemoved(const QString &filename);
};

