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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>
#include <QVariantMap>
#include "shared/abstractserviceprovider.h"
#include <QStringList>

class ActorPlaylist : public AbstractServiceProvider
{
  Q_OBJECT
  Q_PROPERTY(QString name READ name WRITE setName);
  Q_PROPERTY(QStringList files READ files WRITE setFiles);
  Q_PROPERTY(QStringList titles READ titles WRITE setTitles);
  Q_PROPERTY(int currentTrack READ currentTrack WRITE setCurrentTrack);
  public:
    ActorPlaylist(QObject* parent = 0);
    virtual ProvidedTypes providedtypes(){return NoneType;}
    QString name() const;
    QStringList files() const;
    QStringList titles() const;
    int currentTrack() const;
    void setName ( QString name );
    void setFiles(const QStringList& files);
    void setTitles(const QStringList& titles);
    void setCurrentTrack(int index);

    QString currentFilename() const;
    QString getFilename(int index) const;
    int size() const;
  protected:
    QString m_name;
    QStringList m_files;
    QStringList m_titles;
    int m_currentposition;
};

#endif // PLAYLIST_H
