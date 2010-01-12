#include <QtGui/QApplication>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include "mainwindow.h"
#include <qtextcodec.h>

int main(int argc, char *argv[])
{
    KCmdLineArgs::init(argc, argv, "RoomControlAlarm","RoomControlAlarm",  ki18n("RoomControlAlarm") , "1.0");
    KApplication qapp(true);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    qapp.setApplicationName("RoomControlAlarm");
    qapp.setApplicationVersion("1.0");
    qapp.setOrganizationName("davidgraeff");

    MainWindow w;
    w.show();
    return qapp.exec();
}
