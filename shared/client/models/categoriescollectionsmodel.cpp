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
#include "categoriescollectionsmodel.h"
#include <QStringList>
#include <QDebug>
#include <shared/categorize/profile.h>
#include <shared/categorize/category.h>
#include <servicestorage.h>

CategoryItem::~CategoryItem() {
    qDeleteAll(m_profiles.begin(),m_profiles.end());
}
CategoryItem::CategoryItem(Category* c, int p) {
    category = c;
    pos = p;
}
void CategoryItem::removeProfile(int pos) {
    delete m_profiles.takeAt(pos);
    for (int i=pos;i<m_profiles.size();++i)
        m_profiles[i]->pos = i;
}
ProfileItem* CategoryItem::insertProfile(Collection* profile, int row) {
    ProfileItem* p = new ProfileItem(profile,this,row);
    m_profiles.insert(row, p);
    for (int i=row+1;i<m_profiles.size();++i)
        m_profiles[i]->pos = i;
    return p;
}
ProfileItem::ProfileItem(Collection* p, CategoryItem* c, int po) {
    profile=p;
    category=c;
    pos=po;
}ProfileItem::ProfileItem() {
    profile=0;
    category=0;
    pos=-1;
}

/////////////////////////////////////////////////////////////////////////

CategoriesCollectionsModel::CategoriesCollectionsModel ( QObject* parent )
        : ClientModel ( parent )
{
    clear();
}

CategoriesCollectionsModel::~CategoriesCollectionsModel()
{
    qDeleteAll(m_catitems.begin(),m_catitems.end());
}

void CategoriesCollectionsModel::clear()
{
    qDeleteAll(m_catitems.begin(),m_catitems.end());
    m_catitems.clear();
    m_catitems.append(new CategoryItem(0,0));
    reset();
}

int CategoriesCollectionsModel::rowCount ( const QModelIndex & parent) const
{
    if (!parent.isValid()) return m_catitems.size();
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)parent.internalPointer());
    if (cat) {
        return cat->m_profiles.size();
    }
    return 0;
}

int CategoriesCollectionsModel::columnCount(const QModelIndex& ) const {
    return 1;
}

Qt::ItemFlags CategoriesCollectionsModel::flags(const QModelIndex& index) const {
    ProfileItem* p = qobject_cast<ProfileItem*>((QObject*)index.internalPointer());
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)index.internalPointer());
    if (p)
        return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
    else if (cat && cat->category!=0)
        return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable;
    else
        return QAbstractItemModel::flags ( index );
}

QVariant CategoriesCollectionsModel::headerData ( int section, Qt::Orientation orientation, int role ) const
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

QVariant CategoriesCollectionsModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) return QVariant();
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)index.internalPointer());
    if (cat) {
        if (role==Qt::UserRole) return cat->category->id();
        else if (role==Qt::DisplayRole || role==Qt::EditRole) {
            if (cat->category==0) return tr("Nicht kategorisiert");
            return cat->category->name();
        } else return QVariant();
    }
    ProfileItem* p = qobject_cast<ProfileItem*>((QObject*)index.internalPointer());
    if (!p) return QVariant();
    if (role==Qt::UserRole) return p->profile->id();
    else if (role==Qt::DisplayRole || role==Qt::EditRole) {
        return p->profile->name();
    } else if (role==Qt::CheckStateRole) {
        return (p->profile->enabled()?Qt::Checked:Qt::Unchecked);
    } else
        return QVariant();
}

bool CategoriesCollectionsModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() || index.column() !=0 ) return false;
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)index.internalPointer());
    ProfileItem* p = qobject_cast<ProfileItem*>((QObject*)index.internalPointer());
    const QString currentname = (cat && cat->category ? cat->category->name() : (p && p->profile ? p->profile->name() : QString()));

    if ( role == Qt::EditRole ) {
        QString newname = value.toString().trimmed().replace ( QLatin1Char('\n'),QString() ).replace ( QLatin1Char('\t'),QString() );
        if ( newname.isEmpty() || newname == currentname ) return false;
        if (cat && cat->category) {
            cat->category->setName(newname);
            ServiceStorage::instance()->serviceHasChanged(cat->category);
        }
        else if (p && p->profile) {
            p->profile->setName(newname);
            ServiceStorage::instance()->serviceHasChanged(p->profile);
        }
        QModelIndex index = createIndex ( index.row(),0,0 );
        emit dataChanged ( index,index );
        return true;
    } else if ( role == Qt::CheckStateRole && p && p->profile) {
        p->profile->setEnabled(value.toInt()==Qt::Checked);
        ServiceStorage::instance()->serviceHasChanged(p->profile);
        QModelIndex index = createIndex ( index.row(),0,0 );
        emit dataChanged ( index,index );
        return true;
    }
    return false;
}

QModelIndex CategoriesCollectionsModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) return QModelIndex();
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)child.internalPointer());

    if (cat) return QModelIndex();
    ProfileItem* p = qobject_cast<ProfileItem*>((QObject*)child.internalPointer());
    if (!p) {
        return QModelIndex();
    }
    return createIndex(p->category->pos,0,p->category);
}

bool CategoriesCollectionsModel::hasChildren(const QModelIndex& parent) const {
    if (!parent.isValid()) return m_catitems.size();
    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)parent.internalPointer());
    if (cat) {
        return cat->m_profiles.size();
    }
    return false;
}

QModelIndex CategoriesCollectionsModel::index(int row, int column, const QModelIndex& parent) const {
    if (row<0 || column != 0) return QModelIndex();
    if (!parent.isValid()) {
        if (row>=m_catitems.size()) return QModelIndex();
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

bool CategoriesCollectionsModel::removeRows ( int row, int count, const QModelIndex &parent )
{
    QModelIndexList list;
    for (int i=row;i<row+count;++i)
        list.append(index(row,0,parent));

    removeRows(list);
    return true;
}

bool CategoriesCollectionsModel::removeRows ( QModelIndexList list )
{
    qSort(list.begin(), list.end());

    for (int i = list.count() - 1; i > -1; --i) {
        CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)list[i].parent().internalPointer());
        if (!cat && m_catitems[list[i].row()]->category) { // remove categories // do not remove the predefined categories
            Category* c = m_catitems[list[i].row()]->category;
            ServiceStorage::instance()->deleteService(c);
        } else { // remove profiles
            Collection* c = cat->m_profiles[list[i].row()]->profile;
            ServiceStorage::instance()->deleteService(c);
        }
    }

    emit dataChanged ( list.first(),list.last() );
    return true;
}

void CategoriesCollectionsModel::addedCategory(Category* category)
{
    QModelIndex index = indexOf ( category->id());
    if ( index.isValid() ) return;

    // Find alphabetic position amoung categories (start at 1, because of the automatic first cat)
    int row=1;
    for (;row<m_catitems.size();++row)
        if (m_catitems[row]->category->name().toLower() >= category->name().toLower())
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
        if (m_catitems[0]->m_profiles[i]->profile->parentid()==category->id()) {
            ServiceStorage::instance()->serviceHasChanged(m_catitems[0]->m_profiles[i]->profile);
        }
    }

    if (m_focusid == category->name()) {
        emit itemFocus(createIndex(row,0,newitem));
        m_focusid = QString();
    }
}

void CategoriesCollectionsModel::addedProfile(Collection* profile)
{
    QModelIndex index = indexOf ( profile->id());
    if ( index.isValid() ) return;

    // Find right category
    int catpos = 0;
    CategoryItem* cat = m_catitems[0];
    for (int i=1;i<m_catitems.size();++i) {
        if (m_catitems[i]->category->id()==profile->parentid()) {
            cat = m_catitems[i];
            catpos = i;
            break;
        }
    }

    // Find alphabetic position amoung profiles in the same category
    int row=0;
    for (;row<cat->m_profiles.size();++row)
        if (cat->m_profiles[row]->profile->name().toLower() >= profile->name().toLower())
            break;

    beginInsertRows ( createIndex(catpos,0,cat),row,row );
    ProfileItem* newitem = cat->insertProfile(profile, row);
    endInsertRows();

    if (m_focusid == profile->name()) {
        emit itemFocus(createIndex(row,0,newitem));
        m_focusid = QString();
    }

}

void CategoriesCollectionsModel::serviceRemoved ( AbstractServiceProvider* provider )
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

void CategoriesCollectionsModel::serviceChanged ( AbstractServiceProvider* provider )
{
    QModelIndex index = indexOf ( provider->id());

    if ( !index.isValid() ) {
        Category* cat = dynamic_cast<Category*>(provider);
        Collection* p = dynamic_cast<Collection*>(provider);
        if (cat) addedCategory(cat);
        else if (p) addedProfile(p);
        return;
    }

    CategoryItem* cat = qobject_cast<CategoryItem*>((QObject*)index.internalPointer());
    ProfileItem* p = qobject_cast<ProfileItem*>((QObject*)index.internalPointer());
    const QString nameOfService = (cat?cat->category->name().toLower():p->profile->name().toLower());

    bool indicateMovement = false;
    const int oldrow = index.row();
    int newCategoryRow = 0; // default is the unassigned-category

    int row=-1;
    bool skip = false;
    if (cat) {
        for (row=0;row<m_catitems.size();++row)
        {
            if (index.row() == row) skip = true;
            if (index.row() != row && m_catitems[row]->category &&
                    m_catitems[row]->category->name().toLower() >= nameOfService)
            {
                indicateMovement = true;
                break;
            }
        }
    } else if (p) {
        // determine new category
        for (int i=1;i<m_catitems.size();++i)
            if (m_catitems[i]->category->id() == ((Collection*)provider)->parentid()) {
                newCategoryRow = i;
                break;
            }

        CategoryItem* newCategory = m_catitems[newCategoryRow];
        for (row=0;row<newCategory->m_profiles.size();++row)
        {
            if (index.row() == row)
                skip = true;
            else if (newCategory->m_profiles[row]->profile->name().toLower() >= nameOfService) {
                indicateMovement = true;
                break;
            }
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
            if (indicateMovement) emit itemFocus(createIndex(row, 0, item));
        } else if (p) {
            // Remove from old category
            CategoryItem* oldCategory = p->category;
            beginRemoveRows (parent, oldrow, oldrow );
            Collection* item = oldCategory->m_profiles[oldrow]->profile;
            oldCategory->removeProfile(oldrow);
            p = 0;
            endRemoveRows();

            // add to new category
            CategoryItem* newCategory = m_catitems[newCategoryRow];
            beginInsertRows ( createIndex(newCategoryRow,0,newCategory),row,row );
            p = newCategory->insertProfile(item, row);
            endInsertRows();

            emit itemFocus(createIndex(row, 0, p));
        }
    } else {
        emit dataChanged ( index,index );
    }
}

QModelIndex CategoriesCollectionsModel::indexOf ( const QString& id )
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

int CategoriesCollectionsModel::indexOf(const QVariant& data) {
    if (data.type()!=QVariant::String) return -1;
    const QString id = data.toString();
    for (int cats=0;cats<m_catitems.size();++cats)
    {
        if (m_catitems[cats]->category && m_catitems[cats]->category->id() == id) {
            return cats; // createIndex ( cats,0,m_catitems[cats] );
        }
        for (int pros=0;pros<m_catitems[cats]->m_profiles.size();++pros) {
            if (m_catitems[cats]->m_profiles[pros]->profile->id() == id) {
                return pros; // createIndex ( pros,0,m_catitems[cats]->m_profiles[pros] );
            }
        }
    }
    return -1;
}

void CategoriesCollectionsModel::childsChanged(Collection* provider)
{
    QModelIndex index = indexOf ( provider->id());
    emit dataChanged ( index,index );
}

AbstractServiceProvider* CategoriesCollectionsModel::get(const QModelIndex& index)
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
void CategoriesCollectionsModel::stateTrackerChanged(AbstractStateTracker*) {}

