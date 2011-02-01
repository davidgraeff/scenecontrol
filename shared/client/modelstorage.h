/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

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

#include <QObject>
#include <QMap>

class ClientModel;

class ModelStorage : public QObject
{
    Q_OBJECT
public:
    static ModelStorage* instance(bool first = false);
    static ModelStorage* instance(ModelStorage* instance);
    virtual ~ModelStorage();
    ClientModel* model(const QString& id) ;
    QMap<QString, ClientModel*> models() const;
    void registerClientModel(ClientModel* model, bool permanent = true) ;
private:
    static ModelStorage* m_instance;
    ModelStorage();
    QList< ClientModel* > m_models;
    QMap<QString, ClientModel*> m_modelsMap;
};
