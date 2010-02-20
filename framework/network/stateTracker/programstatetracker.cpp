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

#include "programstatetracker.h"

ProgramStateTracker::ProgramStateTracker(QObject* parent)
        : AbstractStateTracker(parent)
{}

void ProgramStateTracker::setStatetracker(const QStringList& s) {
    m_statetracker = s;
}
const QStringList& ProgramStateTracker::statetracker() const {
    return m_statetracker;
}
void ProgramStateTracker::setServiceprovider(const QStringList& s) {
    m_serviceprovider = s;
}
const QStringList& ProgramStateTracker::serviceprovider() const {
    return m_serviceprovider;
}
void ProgramStateTracker::setMaxversion(const QString& m) {
    m_maxversion = m;
}
const QString& ProgramStateTracker::maxversion() const {
    return m_maxversion;
}
void ProgramStateTracker::setMinversion(const QString& m) {
    m_minversion = m;
}
const QString& ProgramStateTracker::minversion() const {
    return m_minversion;
}
void ProgramStateTracker::setAppversion(const QString& a) {
    m_appversion = a;
}
const QString& ProgramStateTracker::appversion() const {
    return m_appversion;
}
