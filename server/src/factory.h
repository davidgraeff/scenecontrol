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

#ifndef FACTORY_H
#define FACTORY_H

#include <QtCore/QObject>
#include <QMap>
#include <QVariantMap>
#include <QTimer>
#include <QDir>

class ProfileCollection;
class AbstractServiceProvider;
class Factory : public QObject
{
    Q_OBJECT
public:
    Factory(QObject* parent = 0);
    virtual ~Factory();
    void examine(const QVariantMap& json);
    void load();
    QDir getSaveDir() const;
    void backup();
    QStringList backup_list();
    void backup_restore(const QString& id);
    void backup_remove(const QString& id);

    // all actors, conditions, events, profiles
    AbstractServiceProvider* get(const QString& id);
    QList<AbstractServiceProvider*> m_providerList;
    // Have to be public, because Playlists may be added through this function
    void addServiceProvider(AbstractServiceProvider* provider);
private:
    QMap<QString, AbstractServiceProvider*> m_provider;
    QDir m_savedir;
    QString providerFilename(AbstractServiceProvider* provider) ;
    AbstractServiceProvider* generate ( const QVariantMap& args );
    ProfileCollection* m_missingProfiles;
public Q_SLOTS:
    void objectSaveToDisk(AbstractServiceProvider*);
    void objectRemoveFromDisk(AbstractServiceProvider*);

Q_SIGNALS:
    void addedProvider(AbstractServiceProvider*);
    void removedProvider(AbstractServiceProvider*);
    void systemStarted();
    void systemGoingDown();
};

#endif // FACTORY_H
