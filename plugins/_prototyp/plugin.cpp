/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <QDebug>

#include "plugin.h"
#include <QCoreApplication>

/**
 * A plugin is a separate process and for that reason a main function have to be implemented
 * which instantiate the plugin object.
 */
int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    if (argc<2) {
		qWarning()<<"No instanceid provided!";
		return 1;
	}
    plugin p(QString::fromAscii(argv[1]));
    if (!p.createCommunicationSockets())
        return -1;
    return app.exec();
}

plugin::plugin(const QString& instanceid) : AbstractPlugin(instanceid) {
}

plugin::~plugin() {
}

void plugin::dataFromPlugin(const QByteArray& plugin_id, const QVariantMap& data) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(data);
}