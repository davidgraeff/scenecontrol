


#include "qextserialenumerator.h"
#include <QDebug>
#include <QMetaType>
#include <QStringList>
#include <QDir>

QextSerialEnumerator::QextSerialEnumerator( )
{
    if( !QMetaType::isRegistered( QMetaType::type("QextPortInfo") ) )
        qRegisterMetaType<QextPortInfo>("QextPortInfo");
}

QextSerialEnumerator::~QextSerialEnumerator( )
{
}

QList<QextPortInfo> QextSerialEnumerator::getPorts()
{
    QList<QextPortInfo> infoList;
#ifdef Q_OS_LINUX
    QStringList portNamePrefixes, portNameList;
    portNamePrefixes << QLatin1String("ttyS*"); // list normal serial ports first

    QDir dir(QLatin1String("/dev"));
    portNameList = dir.entryList(portNamePrefixes, (QDir::System | QDir::Files), QDir::Name);

    // remove the values which are not serial ports for e.g.  /dev/ttysa
    for (int i = 0; i < portNameList.size(); i++) {
        bool ok;
        QString current = portNameList.at(i);
        // remove the ttyS part, and check, if the other part is a number
        current.remove(0,4).toInt(&ok, 10);
        if (!ok) {
            portNameList.removeAt(i);
            i--;
        }
    }

    // get the non standard serial ports names
    // (USB-serial, bluetooth-serial, 18F PICs, and so on)
    // if you know an other name prefix for serial ports please let us know
    portNamePrefixes.clear();
    portNamePrefixes << QLatin1String("ttyACM*") << QLatin1String("ttyUSB*") << QLatin1String("rfcomm*");
    portNameList.append(dir.entryList(portNamePrefixes, (QDir::System | QDir::Files), QDir::Name));

    foreach (QString str , portNameList) {
        QextPortInfo inf;
        inf.physName = QLatin1String("/dev/")+str;
        inf.portName = str;

        if (str.contains(QLatin1String("ttyS"))) {
            inf.friendName = QLatin1String("Serial port ")+str.remove(0, 4);
        }
        else if (str.contains(QLatin1String("ttyUSB"))) {
            inf.friendName = QLatin1String("USB-serial adapter ")+str.remove(0, 6);
        }
        else if (str.contains(QLatin1String("rfcomm"))) {
            inf.friendName = QLatin1String("Bluetooth-serial adapter ")+str.remove(0, 6);
        }
        inf.enumName = QLatin1String("/dev"); // is there a more helpful name for this?
        infoList.append(inf);
    }
#else
    qCritical("Enumeration for POSIX systems (except Linux) is not implemented yet.");
#endif
    return infoList;
}

void QextSerialEnumerator::setUpNotifications( )
{
    qCritical("Notifications for *Nix/FreeBSD are not implemented yet");
}
