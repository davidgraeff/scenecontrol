#include "alarmmodel.h"
#include "roomclient.h"
#include <QStringList>
#include <QDebug>
#include <Transferables/transferable.h>
#include <Transferables/timeableabsolute.h>
#include <Transferables/timeablerepeating.h>
#include <Transferables/transferablefactory.h>

/*
        Timeable* timeable = TransferableFactory::generateTimeable(objectstring);
        if (!timeable) return;
        // Provide a better display name if timeable available
        display = timeable->getName() + "\n";
        TimeableAbsolute* aa = dynamic_cast<TimeableAbsolute*>(alarm);
        if (aa!=0) {
            this->type = 0;
            this->display += aa->getDateTime().toString("hh:mm, dd:MM:yyyy");
        }
        TimeableRepeating* ad = dynamic_cast<TimeableRepeating*>(alarm);
        if (ad!=0) {
            this->type = 1;
            this->display += ad->getTime().toString("hh:mm")+", ";
            for (int i=0;i<ad->getDays().size();++i)
                if (ad->getDays()[i])
                    this->display += QDate::shortDayName(i+1) + " ";
        }
*/

AlarmModel::AlarmModel(RoomClient* client) : QAbstractListModel(0), client(client) {
    acceptedTypes.append ( AlarmAbsoluteType );
    acceptedTypes.append ( AlarmPeriodicType );

    this->iconAbsolute = QIcon(":/absolute");
    this->iconDays = QIcon(":/days");
    this->iconTemplate = QIcon(":/template");
    connect ( client,SIGNAL ( changed ( int,QStringList& ) ), SLOT ( changed ( int,QStringList& ) ) );
    connect ( client,SIGNAL ( removed ( int,QStringList& ) ),SLOT ( removed ( int,QStringList& ) ) );
    connect ( client,SIGNAL ( stateChanged ( RoomClientState ) ),SLOT ( stateChanged ( RoomClientState ) ) );
    connect ( this, SIGNAL ( rename ( QUuid,QString ) ), client, SLOT ( rename ( QUuid,QString ) ) );
}

AlarmModel::~AlarmModel() {
    qDeleteAll(list);
}

QVariant AlarmModel::data ( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();
    
    ModelItem* item = list.at ( index.row() );

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        return item->name;

    } else if (role == Qt::ToolTipRole) {
        if (item->type == AlarmAbsoluteType) {
	  if (item->checked)
            return tr("Alarm");
	  else
	    return tr("Alarm Template");
	} else if (item->type == AlarmPeriodicType)
            return tr("Periodischer Alarm");

    } else if (role == Qt::CheckStateRole) {
        return (item->checked?Qt::Checked:Qt::Unchecked);
	
    } else if (role==Qt::DecorationRole) {
        if (item->type == AlarmAbsoluteType) {
	  if (item->checked)
            return this->iconAbsolute;
	  else
	    return this->iconTemplate;
	} else if (item->type == AlarmPeriodicType)
            return this->iconDays;
    }
    
    return QVariant();
}

int AlarmModel::rowCount ( const QModelIndex&  ) const
{
    return list.size();
}

QVariant AlarmModel::headerData ( int section, Qt::Orientation orientation, int role ) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole && section==0) {
            return tr("Alarmname");
        } else
            if (role == Qt::ToolTipRole && section==0) {
                return tr("Wenn per Checkbox eingeschaltet, dann wird dieser Alarm bei Alarmauslösung aktiviert");
            }
    }

    return QVariant();
}

bool AlarmModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() ) return false;
    ModelItem* item = list.at ( index.row() );

    if ( index.column()==0 && role == Qt::EditRole )
    {
        QString newname = value.toString().trimmed().replace ( '\n',"" ).replace ( '\t',"" );
        if ( newname.isEmpty() || newname == item->name) return false;

        item->setLoading(true);
        emit rename(item->id, newname);
        return true;
    }
    else if (role == Qt::CheckStateRole )
    {
        //qDebug() << value << item->checked;
        Timeable* obj = TransferableFactory::generateTimeable(item->objectstring);
        obj->setEnabled((value.toInt()==Qt::Checked));

	client->change(obj->stringRepresentation());
        delete obj;
        return true;
    }

    return false;
}

bool AlarmModel::removeRows ( int row, int count, const QModelIndex&  )
{
    QStringList ids;
    for ( int i=row+count-1;i>=row;--i )
    {
        list.at ( i )->setLoading(true);
        ids.append ( list.at ( i )->id );
    }
    QModelIndex oben = createIndex ( row,0,0 );
    QModelIndex unten = createIndex ( row+count-1,0,0 );
    emit dataChanged ( oben, unten );
    client->removeMany ( ids );
    return true;
}

Qt::ItemFlags AlarmModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return 0;

    if ( list.at ( index.row() )->isLoading )
        return Qt::ItemIsEnabled;
    else if (index.column()==0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable  | Qt::ItemIsUserCheckable;
    return QAbstractListModel::flags(index);
}

void AlarmModel::stateChanged ( RoomClientState state)
{
    if (state == ConnectionDisconnected) {
        qDeleteAll ( list );
        list.clear();
	map.clear();
        reset(); 
    }
}

QStringList AlarmModel::getItemObjectstring ( int index )
{
    return list.at ( index )->objectstring;
}

void AlarmModel::setItemLoading ( int index )
{
    list.at ( index )->setLoading(true);
    QModelIndex oben = createIndex ( index,0,0 );
    emit dataChanged ( oben, oben );
}


void AlarmModel::changed ( int type, QStringList& objectstring )
{
    if ( !acceptedTypes.contains ( type ) ) return;
    Timeable* obj = TransferableFactory::generateTimeable(objectstring);
    Q_ASSERT(obj);

    ModelItem* item = map.value ( obj->getID() );
    if ( item )
    {
        int row = list.indexOf ( item );
        item->setData ( objectstring );
	item->checked = obj->isEnabled();
        QModelIndex oben = createIndex ( row,0,0 );
        emit dataChanged ( oben, oben );
    }
    else
    {
        beginInsertRows ( QModelIndex(), list.size(), list.size() );
        item = new ModelItem ( objectstring );
	item->checked = obj->isEnabled();
        list.append ( item );
        endInsertRows();
        map.insert ( item->id, item );
    }
    delete obj;      
    if ( item->isLoading )
    {
        client->request ( item->id, item->timestamp );
    }
}

void AlarmModel::removed ( int type, QStringList& objectstring )
{
    if ( !acceptedTypes.contains ( type ) ) return;

    QUuid id = QUuid ( objectstring[1] );
    ModelItem* item = map.value ( id );
    if ( !item ) return;
    map.remove ( id );
    int row = list.indexOf ( item );
    beginRemoveRows ( QModelIndex(), row, row );
    delete list.takeAt ( row );
    endRemoveRows();
}
