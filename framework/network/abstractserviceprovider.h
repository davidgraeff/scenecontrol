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

#ifndef ABSTRACTSERVICEPROVIDER_H
#define ABSTRACTSERVICEPROVIDER_H
#include <QObject>
#include <QStringList>

class AbstractServiceProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId);
    Q_PROPERTY(QString type READ type);
    Q_PROPERTY(QString parentid READ parentid WRITE setParentid)
public:
    AbstractServiceProvider(QObject* parent = 0);
    /** Sync this service to clients */
    virtual void sync();
    /** Called after json changed properies */
    virtual void changed();
    /** Called after all objects have been loaded to link
      * childs with parents together. */
    virtual void link();
    /** Request the server to remove this provider */
    void requestRemove() ;
    /** Return the translated string associated with this provider for showing it in gui. */
    inline QString toString() { return m_string; }

    QString id() const;
    void setId(const QString& id);
    QString type() const;
    
    const QString& parentid() const {
        return m_parentid;
    }
    
    void setParentid( const QString& p ) {
        m_parentid = p;
    }
protected:
    QString m_id;
    QString m_string;
    QString m_parentid;
  Q_SIGNALS:
    void objectChanged(AbstractServiceProvider*);
    void objectSync(QObject*);
};

#endif // ABSTRACTSERVICEPROVIDER_H
