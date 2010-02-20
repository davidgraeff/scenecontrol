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

#ifndef PROGRAMSTATETRACKER_H
#define PROGRAMSTATETRACKER_H
#include "abstractstatetracker.h"

class ProgramStateTracker : public AbstractStateTracker
{
    Q_OBJECT
    Q_PROPERTY(QString appversion READ appversion WRITE setAppversion)
    Q_PROPERTY(QString minversion READ minversion WRITE setMinversion)
    Q_PROPERTY(QString maxversion READ maxversion WRITE setMaxversion)
    Q_PROPERTY(QStringList serviceprovider READ serviceprovider WRITE setServiceprovider)
    Q_PROPERTY(QStringList statetracker READ statetracker WRITE setStatetracker)
public:
    ProgramStateTracker(QObject* parent = 0);
    
    const QString& appversion() const ;
    void setAppversion( const QString& a ) ;
    const QString& minversion() const ;
    void setMinversion( const QString& m ) ;
    const QString& maxversion() const ;
    void setMaxversion( const QString& m ) ;
    const QStringList& serviceprovider() const ;
    void setServiceprovider( const QStringList& s ) ;
    const QStringList& statetracker() const ;
    void setStatetracker( const QStringList& s ) ;
    
private:
    QString m_appversion;
    QString m_minversion;
    QString m_maxversion;
    QStringList m_serviceprovider;
    QStringList m_statetracker;

};

#endif // PROGRAMSTATETRACKER_H
