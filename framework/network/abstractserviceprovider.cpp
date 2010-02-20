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

#include "abstractserviceprovider.h"
#include <QUuid>
#include <QVariant>
#include <QDebug>

AbstractServiceProvider::AbstractServiceProvider(QObject* parent)
        : QObject(parent)
{
    m_id = QUuid::createUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

void AbstractServiceProvider::sync()
{
    // set temporary name
    if (property("remove").isValid())
        m_string = tr ( "%1 (Delete..)" ).arg ( m_string );
    else
	m_string = tr ( "%1 (Sync..)" ).arg ( m_string );
    
    emit objectChanged(this);
    emit objectSync(this);
}

void AbstractServiceProvider::changed()
{
    emit objectChanged(this);
}

QString AbstractServiceProvider::id() const {
    return m_id;
}

void AbstractServiceProvider::setId(const QString& id) {
    m_id = id;
}

QString AbstractServiceProvider::type() const {
    const QString type = QString::fromAscii(metaObject()->className());
    return type;
}
