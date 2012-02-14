#include <QtGui/QApplication>
#include "configurationwidget.h"
#include "networkcontroller.h"
#include "actioncontroller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QLatin1String("roomcontrol.windows.client"));
    app.setApplicationVersion(QLatin1String("1.0"));
    app.setQuitOnLastWindowClosed(false);
    ConfigurationWidget* w = new ConfigurationWidget();
    NetworkController *nc = NetworkController::intance();
    ActionController* ac = new ActionController();
    w->connect(w, SIGNAL(networkConfigChanged(QString, int)), nc, SLOT(networkConfigChanged(QString, int)));
    nc->networkConfigChanged(w->networkConfigHost(), w->networkConfigPort());
    w->connect(nc, SIGNAL(message(QString)), w, SLOT(message(QString)));
    nc->connect(nc, SIGNAL(serverJSON(QVariantMap)), ac, SLOT(serverJSON(QVariantMap)));
    int r = app.exec();
    delete nc;
    delete w;
    delete ac;
    return r;
}
