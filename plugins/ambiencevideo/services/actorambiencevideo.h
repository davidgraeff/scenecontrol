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

class ActorAmbienceVideo : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString host READ host WRITE setHost)
    Q_PROPERTY(QString filename READ filename WRITE setFilename)
    Q_CLASSINFO("filename_props", "filename");
    Q_PROPERTY(EnumOnClick onleftclick READ onleftclick WRITE setOnleftclick)
    Q_PROPERTY(EnumOnClick onrightclick READ onrightclick WRITE setOnrightclick)
    Q_ENUMS(EnumOnClick);
    Q_PROPERTY(double volume READ volume WRITE setVolume)
    Q_CLASSINFO("volume_doublemin", "0.0");
    Q_CLASSINFO("volume_doublemax", "1.0");
    Q_PROPERTY(int display READ display WRITE setDisplay)
    Q_CLASSINFO("display_min", "-1");
    Q_CLASSINFO("display_max", "10");
    Q_PROPERTY(int restoretime READ restoretime WRITE setRestoretime)
    Q_CLASSINFO("restoretime_min", "0");
    Q_CLASSINFO("restoretime_max", "1000");
public:
    enum EnumOnClick
    {
        OnClickCloseFullscreen,
        OnClickHideVideo,
        OnClickScreenOff,
        OnClickClose
    };
    ActorAmbienceVideo(QObject* parent = 0);
    virtual QString service_name() {
        return tr("Ambience Video starten");
    }
    virtual QString service_desc() {
        return tr("Zeigt die angegebene Videodatei auf dem entsprechendem Display (-1=alle)");
    }

    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Host (leer=alle)");
        case 1:
            return tr("Dateiname");
        case 2:
            switch (enumindex) {
            case 0:
                return tr("Vollbild temporär aufheben");
            case 1:
                return tr("Video temporär verstecken");
            case 2:
                return tr("Bildschirm ausschalten");
            case 3:
                return tr("Video schließen");
            default:
                return tr("Bei Linksklick");
            }
        case 3:
            switch (enumindex) {
            case 0:
                return tr("Vollbild temporär aufheben");
            case 1:
                return tr("Video temporär verstecken");
            case 2:
                return tr("Bildschirm ein/aus umschalten");
            case 3:
                return tr("Video schließen");
            default:
                return tr("Bei Rechtsklick");
            }
        case 4:
            return tr("Lautstärke");
        case 5:
            return tr("Anzeigenummer");
        case 6:
            return tr("Wideerherstellungszeit in s");
        default:
            return QString();
        }
    }

    QString host() const {
        return m_host;
    }

    void setHost( QString m ) {
        m_host = m;
    }

    const QString& filename() const {
        return m_filename;
    }
    void setFilename( const QString& v ) {
        m_filename = v;
    }
    EnumOnClick onleftclick() const {
        return m_onleftclick;
    }

    void setOnleftclick( EnumOnClick m ) {
        m_onleftclick = m;
    }
    EnumOnClick onrightclick() const {
        return m_onrightclick;
    }

    void setOnrightclick( EnumOnClick m ) {
        m_onrightclick = m;
    }

    qreal volume() const {
        return m_volume;
    }

    void setVolume( qreal v ) {
        m_volume = v;
    }

    int display() const {
        return m_display;
    }

    void setDisplay( int v ) {
        m_display = v;
    }

    int restoretime() const {
        return m_restoretime;
    }

    void setRestoretime( int v ) {
        m_restoretime = v;
    }
private:
    QString m_filename;
    QString m_host;
    EnumOnClick m_onleftclick;
    EnumOnClick m_onrightclick;
    qreal m_volume;
    int m_display;
    int m_restoretime;
};
