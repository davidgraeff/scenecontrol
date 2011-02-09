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

#include <QSettings>
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>
#include <qtextcodec.h>
#include "config.h"

int main(int argc, char *argv[])
{
    QCoreApplication qapp(argc, argv);
    qapp.setApplicationName(QLatin1String("main"));
    qapp.setApplicationVersion(QLatin1String(ABOUT_VERSION));
    qapp.setOrganizationName(QLatin1String(ABOUT_ORGANIZATIONID));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

	QStringList list = qapp.arguments();

    qDebug() << QCoreApplication::applicationName().toAscii().data() << ":" << QCoreApplication::applicationVersion().toAscii().data();

	int ii = list.indexOf(QLatin1String("--install"));
	int iu = list.indexOf(QLatin1String("--uninstall"));

	if (ii>=0 && list.size()>=ii+2) {
	QSettings s;
	s.setValue(QLatin1String("path"), list.at(ii+1));
	} else if (iu >=0) {
	QSettings s;
	s.clear();
	}

    return 0;
}
