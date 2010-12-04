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

#ifndef ActorMute_h
#define ActorMute_h

#include <shared/abstractserviceprovider.h>


class ActorMute : public AbstractServiceProvider
{
    Q_OBJECT
    Q_PROPERTY(QString value READ value WRITE setValue)
    Q_PROPERTY(EnumActorMute mute READ mute WRITE setMute)
    Q_PROPERTY(EnumActorAssignment assignment READ assignment WRITE setAssignment)
    Q_PROPERTY(double volume READ volume WRITE setVolume)
    Q_CLASSINFO("volume_doublemin", "-1.0");
    Q_CLASSINFO("volume_doublemax", "1.0");
    Q_ENUMS(EnumActorMute);
	Q_ENUMS(EnumActorAssignment);
public:
    enum EnumActorMute
    {
        MuteSink,
        UnmuteSink,
        ToggleSink,
    };
    enum EnumActorAssignment
    {
        NoVolumeSet,
        VolumeRelative,
        VolumeAbsolute,
    };
    ActorMute(QObject* parent = 0);
    virtual QString service_name() {
        return tr("Pulseaudiokanal steuern");
    }
    virtual QString service_desc() {
        return tr("Steuert die Laustärke und Stummschaltung eines Pulseaudiokanal");
    }

    virtual QString translate(int propindex, int enumindex = -1) {
        Q_UNUSED(enumindex);
        switch (propindex) {
        case 0:
            return tr("Kanal");
        case 1:
            switch (enumindex) {
            case 0:
                return tr("Aktiv");
            case 1:
                return tr("Inaktiv");
            case 2:
                return tr("Umschalten");
            default:
                return tr("Stumm");
            }
        case 2:
            switch (enumindex) {
            case 0:
                return tr("Lautstärke nicht ändern");
            case 1:
                return tr("Relativ setzen");
            case 2:
                return tr("Absolut setzen");
            default:
                return tr("Art");
            }
        case 3:
            return tr("Lautstärke");
        default:
            return QString();
        }
    }
    const QString& value() const {
        return m_value;
    }
    void setValue( const QString& v ) {
        m_value = v;
    }
    EnumActorMute mute() const {
        return m_mute;
    }

    void setMute( EnumActorMute m ) {
        m_mute = m;
    }

    qreal volume() const {
        return m_volume;
    }

    void setVolume( qreal v ) {
        m_volume = v;
    }

    EnumActorAssignment assignment() const {
        return m_assignment;
    }

    void setAssignment( EnumActorAssignment r ) {
        m_assignment = r;
    }
private:
    QString m_value;
    EnumActorMute m_mute;
    qreal m_volume;
    EnumActorAssignment m_assignment;
};

#endif // ActorMute_h
