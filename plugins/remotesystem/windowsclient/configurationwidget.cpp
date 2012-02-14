#include "configurationwidget.h"
#include "ui_configurationwidget.h"
#include <QtGui/QApplication>
#include <QSettings>
#include <QStyle>
#include <QDesktopWidget>
#include <QMenu>
#include <windows.h>
#include <winuser.h>


ConfigurationWidget::ConfigurationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigurationWidget)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowSystemMenuHint|Qt::Window);
    setWindowTitle(qApp->applicationName());
    m_tray.setIcon(this->windowIcon());
    connect(&m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(activated(QSystemTrayIcon::ActivationReason)));
    m_tray.show();
    QMenu* menu = new QMenu(this);
    menu->addAction(QIcon(QLatin1String(":/icons/close")), tr("Beenden"), qApp, SLOT(quit()));
    m_tray.setContextMenu(menu);
    ui->host->setText(networkConfigHost());
    ui->port->setValue(networkConfigPort());
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));
}

ConfigurationWidget::~ConfigurationWidget()
{
    delete ui;
}

QString ConfigurationWidget::networkConfigHost()
{
    QSettings s;
    s.beginGroup(QLatin1String("network"));
    return s.value(QLatin1String("host"),QLatin1String("192.168.1.2")).toString();
}

int ConfigurationWidget::networkConfigPort()
{
    QSettings s;
    s.beginGroup(QLatin1String("network"));
    return s.value(QLatin1String("port"),3101).toInt();
}

void ConfigurationWidget::activated(QSystemTrayIcon::ActivationReason r) {
    if (r == QSystemTrayIcon::Trigger) {
        this->show();
    }
}

void ConfigurationWidget::on_btnExit_clicked(bool) {
    QApplication::exit(0);
}

void ConfigurationWidget::on_btnMonitorOff_clicked(bool) {
    const int MONITOR_ON = -1;
    const int MONITOR_OFF = 2;
//    const int MONITOR_STANBY = 1;

    bool turnOff = true;
    PostMessage(HWND_TOPMOST, WM_SYSCOMMAND, SC_MONITORPOWER, turnOff ? MONITOR_OFF : MONITOR_ON);
}

void ConfigurationWidget::on_btnConnect_clicked(bool)
{
    QSettings s;
    s.beginGroup(QLatin1String("network"));
    s.setValue(QLatin1String("host"), ui->host->text());
    s.setValue(QLatin1String("port"), ui->port->value());
    emit networkConfigChanged(ui->host->text(), ui->port->value());
}

void ConfigurationWidget::message(const QString &msg)
{
    m_tray.showMessage(qApp->applicationName(), msg);
}
