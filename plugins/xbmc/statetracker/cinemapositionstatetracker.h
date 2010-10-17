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


#ifndef CinemaPositionStateTracker_h
#define CinemaPositionStateTracker_h
#include <shared/abstractstatetracker.h>

class CinemaPositionStateTracker : public AbstractStateTracker
{
	Q_OBJECT
	Q_PROPERTY(int position READ position WRITE setPosition);
	Q_PROPERTY(int total READ total WRITE setTotal);
public:
	CinemaPositionStateTracker(QObject* parent = 0) : AbstractStateTracker(parent), m_position(0), m_total(0) {}
	int position() { return m_position; }
	void setPosition(int position) {m_position = position;}
	int total() { return m_total; }
	void setTotal(int total) {m_total = total;}
private:
	int m_position;
	int m_total;
};
#endif //CinemaPositionStateTracker_h