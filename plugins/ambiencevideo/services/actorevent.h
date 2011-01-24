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

#pragma once
#include <shared/abstractserviceprovider.h>

class ActorEvent : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString host READ host WRITE setHost)
    Q_PROPERTY(QString title READ title WRITE setTitle);
    Q_PROPERTY(int duration READ duration WRITE setDuration);
    Q_CLASSINFO("duration_min", "0");
    Q_CLASSINFO("duration_max", "10");
    Q_PROPERTY(QString filename READ filename WRITE setFilename);
    Q_CLASSINFO("filename_props", "filename")
public:
    ActorEvent(QObject* parent = 0);
	virtual QString service_name(){return tr("Ereignisindikator");}
	virtual QString service_desc(){return tr("Zeigt an und gibt ggfs. eine Mediendatei wieder um ein Ereignis anzuzeigen");}
    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Host (leer=alle)");
        case 1:
            return tr("Nachricht");
        case 2:
            return tr("Anzeigedauer in s");
        case 3:
            return tr("Datei abspielen");
        default:
            return QString();
        }
    }
    void setTitle(const QString& v);
    QString title();
    void setFilename(const QString& v);
    QString filename();
    QString host() const {
        return m_host;
    }

    void setHost( QString m ) {
        m_host = m;
    }
    int duration() const {
        return m_duration;
    }

    void setDuration( int m ) {
        m_duration = m;
    }
private:
    QString m_eventTitle;
    QString m_filename;
    QString m_host;
    int m_duration;
};
