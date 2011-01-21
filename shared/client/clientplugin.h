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

#ifndef CLIENTPLUGIN_H
#define CLIENTPLUGIN_H
#include <QObject>
#include <QStringList>
#include <QAbstractItemModel>
#include <QList>

class AbstractServiceProvider;
class AbstractStateTracker;
class AbstractPlugin;

class ClientModel : public QAbstractListModel {
    Q_OBJECT
public:
	ClientModel(QObject* parent = 0) : QAbstractListModel(parent) {}
    /*
    Get QML for embedding this model
    */
    virtual QString getQML() { return QString(); }
    virtual QString id() {
		return QString::fromAscii(metaObject()->className());
	}
	virtual int indexOf(const QVariant& data) = 0;
public Q_SLOTS:
    virtual void stateTrackerChanged(AbstractStateTracker*) = 0;
	virtual void serviceRemoved(AbstractServiceProvider*) = 0;
	virtual void serviceChanged(AbstractServiceProvider*) = 0;
	virtual void clear() = 0;
Q_SIGNALS:
    void changeService(AbstractServiceProvider*);
	void executeService(AbstractServiceProvider*);
    /*
     * The model want to hint to a special entry.
     * For example if the track in a playlist changed.
     */
    void autoFocusChanged(QModelIndex);
};

class ClientPlugin : public QObject
{
    Q_OBJECT
public:
    QList<ClientModel*> models() { return m_models; }
    virtual AbstractPlugin* base() = 0;
	void addModel(ClientModel* model) {
		connect(model, SIGNAL(changeService(AbstractServiceProvider*)), SIGNAL(changeService(AbstractServiceProvider*)));
		connect(model, SIGNAL(executeService(AbstractServiceProvider*)), SIGNAL(executeService(AbstractServiceProvider*)));
		connect(this, SIGNAL(serviceChanged(AbstractServiceProvider*)), model, SLOT(serviceChanged(AbstractServiceProvider*)));
		connect(this, SIGNAL(serviceRemoved(AbstractServiceProvider*)), model, SLOT(serviceRemoved(AbstractServiceProvider*)));
		connect(this, SIGNAL(stateTrackerChanged(AbstractStateTracker*)), model, SLOT(stateTrackerChanged(AbstractStateTracker*)));
		connect(this, SIGNAL(clear()), model, SLOT(clear()));
		m_models.append(model);
	}
protected:
   QList< ClientModel* > m_models;
Q_SIGNALS:
    void stateTrackerChanged(AbstractStateTracker*);
    void serviceRemoved(AbstractServiceProvider*);
    void serviceChanged(AbstractServiceProvider*);
	void clear();
	/*
	 * An object of this plugin changed and have to be saved and propagated to the
	 * connected clients.
	 */
    void changeService(AbstractServiceProvider*);
	/*
	 * Service will be propagated to the server and be executed as an immediately service.
	 * The handed over service object will get an iExecute flag and be freed by the networkController after sending.
	 */
	void executeService(AbstractServiceProvider*);
};
Q_DECLARE_INTERFACE(ClientPlugin, "com.roomcontrol.ClientPlugin/1.0")
#endif // ClientPlugin_H
