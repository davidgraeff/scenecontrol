/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef FACTORY_H
#define FACTORY_H

#include <QtCore/QObject>
#include <QMap>
#include <QVariantMap>
#include <QTimer>
#include <QDir>

class AbstractActor;
class AbstractStateTracker;
class AbstractServiceProvider;
class Factory : public QObject
{
    Q_OBJECT
public:
    Factory(QObject* parent = 0);
    virtual ~Factory();
    void examine(const QVariantMap& json);
    /**
      * Add service provider to the managed list
      */
    void addServiceProvider(AbstractServiceProvider* provider);

    /**
      * Execute provider on the server and free the object immediately
      */
    void executeActor(AbstractActor* actor);

    /**
      * Ask the server to remove this provider. The answer is propagated with
      * the @removedProvider Signal.
      */
    void requestRemoveProvider(AbstractServiceProvider* provider);

    // all actors, conditions, events, profiles
    AbstractServiceProvider* get(const QString& id);
    void syncComplete();
    void syncStarted();
private:
    QMap<QString, AbstractServiceProvider*> m_provider;
    QList<AbstractServiceProvider*> m_providerList;
    AbstractServiceProvider* generate ( const QVariantMap& args );
    bool m_sync;
private Q_SLOTS:
    void slotdisconnected();
Q_SIGNALS:
    void addedProvider(AbstractServiceProvider*);
    void removedProvider(AbstractServiceProvider*);
    void stateChanged(AbstractStateTracker*);
	void syncCompleted();
};

#endif // FACTORY_H
