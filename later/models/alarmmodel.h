#ifndef ALARMMODEL_H
#define ALARMMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QIcon>
#include "transferablemodel.h"

class Timeable;
class RoomClient;

class AlarmModel : public QAbstractListModel
{
    Q_OBJECT
public:
    AlarmModel(RoomClient* client);
    ~AlarmModel();

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    QStringList getItemObjectstring(int index);
    void setItemLoading ( int index );
    
private Q_SLOTS:
    void changed ( int type, QStringList& );
    void removed ( int type, QStringList& );
    void stateChanged ( RoomClientState state );
Q_SIGNALS:
    void rename ( const QUuid& id, const QString& name );

private:
    QList<ModelItem*> list;
    QMap<QUuid, ModelItem*> map;
    RoomClient* client;
    QIcon iconAbsolute;
    QIcon iconDays;
    QIcon iconTemplate;
    QVector< int > acceptedTypes;
};

#endif // ALARMMODEL_H
