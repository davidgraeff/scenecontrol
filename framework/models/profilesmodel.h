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

class CategoryProvider;
class ProfileCollection;
class AbstractServiceProvider;

class CategoryItem;
class ProfileItem : public QObject {
    Q_OBJECT
public:
    ProfileCollection* profile;
    CategoryItem* category;
    int pos;
    ProfileItem() {
        profile=0;
        category=0;
        pos=-1;
    }
    ProfileItem(ProfileCollection*p,CategoryItem*c,int po) {
        profile=p;
        category=c;
        pos=po;
    }
};

class CategoryItem : public QObject {
    Q_OBJECT
public:
    int pos;
    CategoryProvider* category;
    QList< ProfileItem* > m_profiles;
    ProfileItem* insertProfile(ProfileCollection* profile, int row)
    {
		ProfileItem* p = new ProfileItem(profile,this,row);
        m_profiles.insert(row, p);
        for (int i=row+1;i<m_profiles.size();++i)
            m_profiles[i]->pos = i;
		return p;
    }
    void removeProfile(int pos) {
        delete m_profiles.takeAt(pos);
        for (int i=pos;i<m_profiles.size();++i)
            m_profiles[i]->pos = i;
    }
    CategoryItem(CategoryProvider* c, int p) {
        category = c;
        pos = p;
    }
    ~CategoryItem() {
        qDeleteAll(m_profiles.begin(),m_profiles.end());
    }
};

class ProfilesModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ProfilesModel (QObject* parent = 0);
    ~ProfilesModel();

    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

//	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

    AbstractServiceProvider* get(const QModelIndex & index) ;
    QModelIndex indexOf ( const QString& id ) ;
    void addedProfile(ProfileCollection* profile);
    void addedCategory(CategoryProvider* category);
	void focusAsSoonAsAvailable(const QString& id) {m_focusid=id;}
public Q_SLOTS:
    void objectChanged(AbstractServiceProvider* provider, bool indicateMovement=true);
    void slotdisconnected();
    void childsChanged(ProfileCollection*);
    void removedProvider(AbstractServiceProvider*);
Q_SIGNALS:
    void itemMoved(const QModelIndex& newindex);
private:
    //QList< AbstractServiceProvider* > m_items;
    QList< CategoryItem* > m_catitems;
	QString m_focusid;
};

#endif // ProfilesModel_h
