 

#ifndef PlaylistStateTracker_h
#define PlaylistStateTracker_h
#include <shared/abstractstatetracker.h>

class PlaylistStateTracker : public AbstractStateTracker
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name WRITE setName);
	Q_PROPERTY(QString lastModified READ lastModified WRITE setLastModified);
	Q_PROPERTY(bool removed READ removed WRITE setRemoved);
public:
	PlaylistStateTracker(QObject* parent = 0) : AbstractStateTracker(parent), m_removed(0){}
	QString name() { return m_name; }
	void setName(QString name) {m_name = name;}
	QString lastModified() { return m_lastModified; }
	void setLastModified(QString lastModified) {m_lastModified = lastModified;}
	bool removed() { return m_removed; }
	void setRemoved(bool removed) {m_removed = removed;}
private:
	QString m_name;
	QString m_lastModified;
	bool m_removed;
};
#endif //PlaylistStateTracker_h