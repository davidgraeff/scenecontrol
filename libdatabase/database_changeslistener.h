#ifndef DATABASE_CHANGESLISTENER_H
#define DATABASE_CHANGESLISTENER_H

#include <QObject>


class DatabaseChangesListener : public QObject
{

public:
    DatabaseChangesListener();
    virtual ~DatabaseChangesListener();
   /**
	 * Start listening to changed documents
	 * If the database does not respond 5 times in sequal the application loop will be quit.
     * doc_changed and doc_removed signals are triggered in reaction if a change occured
     */
    void startChangeListener();
};

#endif // DATABASE_CHANGESLISTENER_H
