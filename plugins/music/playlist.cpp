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

#include "playlist.h"
#include <QSettings>
#include <qfileinfo.h>
#include <QUuid>

Playlist::Playlist(QObject* parent) :
        AbstractServiceProvider(parent), m_currentposition(0), m_changed(false)
{}

Playlist::~Playlist()
{}

void Playlist::setName ( QString name )
{
    m_name = name;
}

QString Playlist::name() const
{
    return m_name;
}

QStringList Playlist::files() const
{
    return m_files;
}

void Playlist::setFiles ( const QStringList& files )
{
    m_files = files;
}

QStringList Playlist::titles() const
{
    return m_titles;
}

void Playlist::setTitles ( const QStringList& titles )
{
    m_titles = titles;
}

void Playlist::setCurrentTrack ( int index )
{
    if (index<0) index = m_files.size()-1;
    else if (index>=m_files.size()) index=0;
    m_currentposition = index;
}

int Playlist::currentTrack() const
{
    return m_currentposition;
}

QString Playlist::currentFilename() const
{
    return getFilename(m_currentposition);
}

QString Playlist::getFilename(int index) const
{
    if (index<0 || index >= m_files.size()) return QString();
    return m_files.at(index);
}

int Playlist::size() const
{
    return m_files.size();
}
