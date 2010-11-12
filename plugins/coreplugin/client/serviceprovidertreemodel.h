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

#ifndef ServiceProviderTreeModel_h
#define ServiceProviderTreeModel_h

#include <QModelIndex>
#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QIcon>

class Collection;
class AbstractServiceProvider;

class ServiceProviderTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ServiceProviderTreeModel (const QString& title, QObject* parent = 0);
    ~ServiceProviderTreeModel();

    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );


    //QString getName ( int i ) const;
    AbstractServiceProvider* get(const QModelIndex & index) ;
    //int indexOf ( const QString& id ) ;
    void addedProvider(AbstractServiceProvider*);
    void removedProvider(AbstractServiceProvider*);
	
	QModelIndex indexOf(const QString& id) ;
	#define eventsrow 0
	#define conditionsrow 1
	#define actorsrow 2
	inline QModelIndex eventsindex(int c) const { return createIndex(eventsrow,c,'e'); }
	inline QModelIndex conditionsindex(int c) const { return createIndex(conditionsrow,c,'c'); }
	inline QModelIndex actorsindex(int c) const { return createIndex(actorsrow,c,'a'); }
public Q_SLOTS:
    void objectChanged(AbstractServiceProvider*);
    void slotdisconnected();
private:
    QList< AbstractServiceProvider* > m_events;
    QList< AbstractServiceProvider* > m_conditions;
    QList< AbstractServiceProvider* > m_actors;
    QString m_title;
	void addToList(QList< AbstractServiceProvider* >& list, AbstractServiceProvider* provider);
};

#endif // ServiceProviderTreeModel_h
