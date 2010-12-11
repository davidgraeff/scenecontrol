#ifndef CardReaderPCSCStateTracker_h
#define CardReaderPCSCStateTracker_h
#include <shared/abstractstatetracker.h>

class CardReaderPCSCStateTracker : public AbstractStateTracker
{
        Q_OBJECT
        Q_PROPERTY(QString atr READ atr WRITE setAtr);
        Q_PROPERTY(int state READ state WRITE setState);
public:
        CardReaderPCSCStateTracker(QObject* parent = 0) : AbstractStateTracker(parent), m_state(0){}
        QString atr() { return m_atr; }
        void setAtr(QString atr) {m_atr = atr;}
        int state() { return m_state; }
        void setState(int state) {m_state = state;}
private:
        QString m_atr;
        int m_state;
};
#endif //CardReaderPCSCStateTracker_h 
