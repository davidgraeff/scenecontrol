#ifndef ACTIONCONTROLLER_H
#define ACTIONCONTROLLER_H

#include <QObject>
#include <QVariantMap>

class ActionController : public QObject
{
    Q_OBJECT
public:
    explicit ActionController(QObject *parent = 0);
    
signals:
    
public slots:
    void serverJSON(const QVariantMap& data);
};

#endif // ACTIONCONTROLLER_H
