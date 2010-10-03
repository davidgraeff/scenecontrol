/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SERVICEPROVIDERPROFILE_H
#define SERVICEPROVIDERPROFILE_H
#include "abstractserviceprovider.h"
#include <QSet>
#include <QTimer>
#include <QMap>

class Factory;
class AbstractActor;
class AbstractEvent;
class AbstractCondition;

class ProfileCollection : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName);
    Q_PROPERTY(QString category_id READ category_id WRITE setCategory_id);
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
public:
    ProfileCollection(QObject* parent = 0);
    virtual void removeFromDisk();
    /** Called if a child of this profile is going to be removed from disk */
    void childRemoved(AbstractServiceProvider*) ;
    /** Called by a child, that register itself as a child for this profile */
    void registerChild(AbstractServiceProvider*);
    bool isRunning();
    void cancel();
    void run();

    QString name() const {
        return m_name;
    }
    void setName(const QString& cmd) {
        m_name = cmd;
    }

    bool enabled() const {
        return m_enabled;
    }

    void setEnabled( bool e ) {
        m_enabled = e;
    }

    QString category_id() const {
        return m_category_id;
    }

    void setCategory_id( const QString& id ) {
        m_category_id = id;
    }

private:
    QString m_name;
    QString m_category_id;
    bool m_enabled;

    // runtime; execution
    QTimer m_executionTimer;
    int m_currentExecution;
    QMultiMap<int, AbstractActor*> m_actors_linked_map;

    QList<int> m_actors_delays;
    QSet<AbstractActor*> m_actors_linked;
    QSet<AbstractCondition*> m_conditions_linked;
    QSet<AbstractEvent*> m_events_linked;

private Q_SLOTS:
    void timeout();
    void eventTriggered();
};

#endif // SERVICEPROVIDERPROFILE_H
