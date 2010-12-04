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

#ifndef ProfilesModel_h
#define ProfilesModel_h

#include <QModelIndex>
#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QIcon>
#include "shared/client/clientplugin.h"

class Category;
class Collection;
class AbstractServiceProvider;

class CategoryItem;
class ProfileItem : public QObject {
    Q_OBJECT
public:
    ProfileItem() ;
    ProfileItem(Collection*p,CategoryItem*c,int po) ;
public:
    Collection* profile;
    CategoryItem* category;
    int pos;
};

class CategoryItem : public QObject {
    Q_OBJECT
public:
    CategoryItem(Category* c, int p);
    ~CategoryItem();
    ProfileItem* insertProfile(Collection* profile, int row);
    void removeProfile(int pos);
public:
    int pos;
    Category* category;
    QList< ProfileItem* > m_profiles;
};

class CategoriesCollectionsModel : public ClientModel
{
    Q_OBJECT
public:
    CategoriesCollectionsModel (QObject* parent = 0);
    ~CategoriesCollectionsModel();

    virtual bool hasChildren(const QModelIndex& parent) const;
    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

//	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

    AbstractServiceProvider* get(const QModelIndex & index) ;
    QModelIndex indexOf ( const QString& id ) ;
	virtual int indexOf(const QVariant& data);
    void addedProfile(Collection* profile);
    void addedCategory(Category* category);
	void focusAsSoonAsAvailable(const QString& id) {m_focusid=id;}
public Q_SLOTS:
    virtual void clear();
    virtual void serviceChanged(AbstractServiceProvider*);
    virtual void serviceRemoved(AbstractServiceProvider* );
    virtual void stateTrackerChanged(AbstractStateTracker* );
    void childsChanged(Collection*);
Q_SIGNALS:
    void itemMoved(const QModelIndex& newindex);
private:
    //QList< AbstractServiceProvider* > m_items;
    QList< CategoryItem* > m_catitems;
	QString m_focusid;
};

#endif // ProfilesModel_h
