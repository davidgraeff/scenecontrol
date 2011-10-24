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
#include <glib/gtypes.h>
#include <QString>
#include <QList>

struct PulseChannel {
	double volume;
	bool mute;
	QString sinkid;
	PulseChannel(double volume, bool mute, const QString& sinkid) : volume(volume), mute(mute), sinkid(sinkid) {}
};

class plugin;
void reconnect_to_pulse(plugin* p);
void close_pulseaudio();
void set_sink_muted(const char* sinkname, int muted);
void set_sink_volume_relative(const char* sinkname, gdouble percent);
void set_sink_volume(const char* sinkname, gdouble percent);
QList<PulseChannel> getAllChannels();
int getServerVersion();
int getProtocolVersion();