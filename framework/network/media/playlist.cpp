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

#include "playlist.h"

Playlist::Playlist(QObject* parent) :
        AbstractServiceProvider(parent), m_currentposition(0)
{
    m_itemmodel = new PlaylistItemsModel(this, this);
}

Playlist::~Playlist()
{}

void Playlist::sync()
{
    m_itemmodel->m_waitforsync = true;
    AbstractServiceProvider::sync();
    m_itemmodel->reset();
}

void Playlist::changed()
{
    m_itemmodel->m_waitforsync = false;
    AbstractServiceProvider::changed();
    m_itemmodel->reset();
}

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
    m_currentposition = index;
    m_itemmodel->setCurrenttrack(m_currentposition);
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
