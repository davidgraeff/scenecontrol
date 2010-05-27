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

#include "profilesmodel.h"
#include <QStringList>
#include <QDebug>
#include <RoomControlClient.h>
#include <profile/serviceproviderprofile.h>
#include "kcategorizedsortfilterproxymodel.h"
#include <actors/abstractactor.h>
#include <conditions/abstractcondition.h>
#include <events/abstractevent.h>
#include <profile/category.h>

ProfilesModel::ProfilesModel ( QObject* parent )
        : QAbstractItemModel ( parent )
{
    connect ( RoomControlClient::getNetworkController(),SIGNAL ( disconnected() ),
              SLOT ( slotdisconnected() ) );

    slotdisconnected();
}

ProfilesModel::~ProfilesModel()
{
    slotdisconnected();
}

void ProfilesModel::slotdisconnected()
{
    qDeleteAll(m_catitems.begin(),m_catitems.end());
    m_catitems.clear();
    m_catitems.append(new CategoryItem(0,0));
    reset();
}

int ProfilesModel::rowCount ( const QModelIndex & parent) const
{
    if (!parent.isValid()) return m_catitems.size();
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)parent.internalPointer());
    if (cat) {
        return cat->m_profiles.size();
    }
    return 0;
}

int ProfilesModel::columnCount(const QModelIndex& ) const {
    return 1;
}

Qt::ItemFlags ProfilesModel::flags(const QModelIndex& index) const {
    return QAbstractItemModel::flags ( index );
}

QVariant ProfilesModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( role == Qt::DisplayRole && section==0 )
        {
            return tr("Profile");
        }
    }

    return QAbstractItemModel::headerData ( section, orientation, role );
}

QVariant ProfilesModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();
    if ( role!=Qt::DisplayRole ) return QVariant();
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)index.internalPointer());
    if (cat) {
        if (cat->category==0) return tr("Nicht kategorisiert");
        return cat->category->toString();
    }
    ProfileItem* p = qobject_cast<ProfileItem*>((QObject*)index.internalPointer());
    if (!p) return QVariant();
    return p->profile->toString();
}


QModelIndex ProfilesModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) return QModelIndex();
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)child.internalPointer());
    if (cat) return QModelIndex();
    ProfileItem* p = qobject_cast<ProfileItem*>((QObject*)child.internalPointer());
    if (!p) {
        qWarning()<<__PRETTY_FUNCTION__<<"No parent found!";
        return QModelIndex();
    }
    return createIndex(p->category->pos,0,p->category);
}

QModelIndex ProfilesModel::index(int row, int column, const QModelIndex& parent) const {
    if (!parent.isValid()) {
        CategoryItem* cat = m_catitems[row];
        return createIndex(row, column, cat);
    }

    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)parent.internalPointer());
    if (!cat) {
        qWarning()<<__PRETTY_FUNCTION__<<"Category not found"<<row;
    }
    else if (cat->m_profiles.size()<=row) {
        qWarning()<<row<<"maximum categories:" << cat->m_profiles.size();
        /*			for( int i=0; i < cat->m_profiles.size();++i) {
        				qWarning()<< "position" << i << cat->m_profiles[i]->pos;
        			}*/
        return QModelIndex();
    }
    return createIndex(row, column, cat->m_profiles[row]);
}

bool ProfilesModel::removeRows ( int row, int count, const QModelIndex &parent )
{
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)parent.internalPointer());
    if (cat) {
        for ( int i=row+count-1;i>=row;--i )
        {
            if (m_catitems[row]->category)
                m_catitems[row]->category->requestRemove();
        }
        return true;
    }

    for ( int i=row+count-1;i>=row;--i )
    {
        cat->m_profiles[i]->profile->requestRemove();
    }
    QModelIndex ifrom = createIndex ( row,0 );
    QModelIndex ito = createIndex ( row+count-1,1 );
    emit dataChanged ( ifrom,ito );
    return true;
}

void ProfilesModel::addedCategory(CategoryProvider* category)
{
    QModelIndex index = indexOf ( category->id());
    if ( index.isValid() ) return;

    // Find alphabetic position amoung categories
    int row=1;
    for (row=1;row<m_catitems.size();++row)
        if (m_catitems[row]->category->toString().toLower() >= category->toString().toLower())
            break;

    beginInsertRows ( QModelIndex(),row,row );
    CategoryItem* newitem = new CategoryItem(category, row);
    m_catitems.insert(row, newitem);
    for (int i=row+1;i<m_catitems.size();++i)
        m_catitems[i]->pos = i;
    endInsertRows();

    // There may already exist profiles assigned to this category that are
    // currently visible at the unassigned-category. Reassign now
    for (int i=m_catitems[0]->m_profiles.size()-1;i>=0;--i) {
        if (m_catitems[0]->m_profiles[i]->profile->category_id()==category->id()) {
            objectChanged(m_catitems[0]->m_profiles[i]->profile, false);
        }
    }

    if (m_focusid == category->name()) {
        emit itemMoved(createIndex(row,0,newitem));
        m_focusid = QString();
    }
}

void ProfilesModel::addedProfile(ProfileCollection* profile)
{
    QModelIndex index = indexOf ( profile->id());
    if ( index.isValid() ) return;

    connect ( profile, SIGNAL ( objectChanged ( AbstractServiceProvider* ) ),
              SLOT ( objectChanged ( AbstractServiceProvider* ) ) );

    connect( profile, SIGNAL(childsChanged(ProfileCollection*)),
             SLOT(childsChanged(ProfileCollection*)));

    // Find right category
    int catpos = 0;
    CategoryItem* cat = m_catitems[0];
    for (int i=1;i<m_catitems.size();++i) {
        if (m_catitems[i]->category->id()==profile->category_id()) {
            cat = m_catitems[i];
            catpos = i;
            break;
        }
    }

    // Find alphabetic position amoung profiles in the same category
    int row;
    for (row=0;row<cat->m_profiles.size();++row)
        if (cat->m_profiles[row]->profile->toString().toLower() >= profile->toString().toLower())
            break;

    beginInsertRows ( createIndex(catpos,0,cat),row,row );
    ProfileItem* newitem = cat->insertProfile(profile, row);
    endInsertRows();

    if (m_focusid == profile->name()) {
        emit itemMoved(createIndex(row,0,newitem));
        m_focusid = QString();
    }

}

void ProfilesModel::removedProvider ( AbstractServiceProvider* provider )
{
    QModelIndex index = indexOf ( provider->id() );
    if ( !index.isValid() ) return;

    beginRemoveRows ( index.parent(), index.row(), index.row() );

    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)index.internalPointer());
    ProfileItem* p = qobject_cast<ProfileItem*>((QObject*)index.internalPointer());
    if (cat) {
        delete m_catitems.takeAt(index.row());
        for (int i=index.row();i<m_catitems.size();++i)
            m_catitems[i]->pos = i;
    }
    else if (p) {
        cat = p->category;
        if (p != cat->m_profiles[index.row()]) qWarning() << "EPIC FAIL" << p << index.row();
        p = 0;
        cat->removeProfile(index.row());
    }

    endRemoveRows();
}

void ProfilesModel::objectChanged ( AbstractServiceProvider* provider, bool indicateMovement )
{
    QModelIndex index = indexOf ( provider->id());
    if ( !index.isValid() ) return;

    const int oldrow = index.row();
    int newCategoryRow = 0; // default is the unassigned-category

    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)index.internalPointer());
    ProfileItem* p = qobject_cast<ProfileItem*>((QObject*)index.internalPointer());

    int row=-1;
    bool skip = false;
    if (cat) {
        for (row=0;row<m_catitems.size();++row)
        {
            if (index.row() == row) skip = true;
            if (index.row() != row && m_catitems[row]->category &&
                    m_catitems[row]->category->toString().toLower() >= provider->toString().toLower())
            {
                break;
            }
        }
    } else if (p) {
        // determine new category
        for (int i=1;i<m_catitems.size();++i)
            if (m_catitems[i]->category->id() == ((ProfileCollection*)provider)->category_id()) {
                newCategoryRow = i;
                break;
            }

        CategoryItem* newCategory = m_catitems[newCategoryRow];
        for (row=0;row<newCategory->m_profiles.size();++row)
        {
            if (index.row() == row)
                skip = true;
            else if (newCategory->m_profiles[row]->profile->toString().toLower() >= provider->toString().toLower())
                break;
        }
    }
    if (skip) row--;

    if (row != -1) {
        const QModelIndex parent = index.parent();
        if (cat) {
            beginRemoveRows ( parent, oldrow, oldrow );
            CategoryItem* item = m_catitems.takeAt(oldrow);
            endRemoveRows();
            beginInsertRows ( parent,row,row );
            m_catitems.insert(row, item);
            for (int i=0;i<m_catitems.size();++i)
                m_catitems[i]->pos = i;
            endInsertRows();
            if (indicateMovement) emit itemMoved(createIndex(row, 0, item));
        } else if (p) {
            // Remove from old category
            CategoryItem* oldCategory = p->category;
            beginRemoveRows (parent, oldrow, oldrow );
            ProfileCollection* item = oldCategory->m_profiles[oldrow]->profile;
            oldCategory->removeProfile(oldrow);
            endRemoveRows();

            // add to new category
            CategoryItem* newCategory = m_catitems[newCategoryRow];
            beginInsertRows ( createIndex(newCategoryRow,0,newCategory),row,row );
            ProfileItem* newProfileItem = newCategory->insertProfile(item, row);
            endInsertRows();

			if (indicateMovement) emit itemMoved(createIndex(row, 0, newProfileItem));
        }
    } else {
        emit dataChanged ( index,index );
    }
}

QModelIndex ProfilesModel::indexOf ( const QString& id )
{
    for (int cats=0;cats<m_catitems.size();++cats)
    {
        if (m_catitems[cats]->category && m_catitems[cats]->category->id() == id) {
            return createIndex ( cats,0,m_catitems[cats] );
        }
        for (int pros=0;pros<m_catitems[cats]->m_profiles.size();++pros) {
            if (m_catitems[cats]->m_profiles[pros]->profile->id() == id) {
                return createIndex ( pros,0,m_catitems[cats]->m_profiles[pros] );
            }
        }
    }
    return QModelIndex();
}

void ProfilesModel::childsChanged(ProfileCollection* provider)
{
    QModelIndex index = indexOf ( provider->id());
    emit dataChanged ( index,index );
}

AbstractServiceProvider* ProfilesModel::get(const QModelIndex& index)
{
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)index.internalPointer());
    if (cat) {
        return cat->category;
    }
    ProfileItem* p = qobject_cast<ProfileItem*>((QObject*)index.internalPointer());
    if (p) {
        return p->profile;
    }
    return 0;
}
