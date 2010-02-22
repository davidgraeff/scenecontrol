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

#ifndef SERVICEPROVIDERPROFILE_H
#define SERVICEPROVIDERPROFILE_H
#include "abstractserviceprovider.h"
#include <QSet>
#include <QTimer>
#include <QMap>

class ServiceProviderModel;
class AbstractActor;
class AbstractEvent;
class AbstractCondition;

class ProfileCollection : public AbstractServiceProvider
{
  Q_OBJECT
  Q_PROPERTY(QString name READ name WRITE setName);
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
  public:
    ProfileCollection(QObject* parent = 0);
    void removedChild(AbstractServiceProvider*);
    void addChild(AbstractServiceProvider* provider);
    
    QString name() const ;
    void setName(const QString& cmd) ;


    virtual void changed() ;
    ServiceProviderModel* events_model() const ;
    ServiceProviderModel* events_and_conditions_model() const ;
    ServiceProviderModel* conditions_model() const ;
    ServiceProviderModel* actors_model() const ;
    
    
    bool enabled() const {
        return m_enabled;
    }
    
    void setEnabled( bool e ) {
        m_enabled = e;
    }
  private:
    QString m_name;
    bool m_enabled;
    
    QMultiMap<int, AbstractActor*> m_actors_linked;
    QSet<AbstractCondition*> m_conditions_linked;
    QSet<AbstractEvent*> m_events_linked;
    ServiceProviderModel* m_events_model;
    ServiceProviderModel* m_events_and_conditions_model;
    ServiceProviderModel* m_conditions_model;
    ServiceProviderModel* m_actors_model;
};

#endif // SERVICEPROVIDERPROFILE_H
