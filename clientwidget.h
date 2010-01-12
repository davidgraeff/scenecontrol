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

#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include "roomclient.h"

class ClientWidget : public QWidget
{
    Q_OBJECT
public:
    ClientWidget ( RoomClient* client, const QString& text, QWidget* parent = 0 );
    QString getText();
protected:
    RoomClient* client;
    QString text;

Q_SIGNALS:
    void showMessage ( const QString& msg );
};

#endif // CLIENTWIDGET_H
