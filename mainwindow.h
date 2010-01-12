#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QCheckBox>
#include <QModelIndex>
#include <QList>
#include "roomclient.h"

class SettingsDialog;
namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    SettingsDialog* settingsdialog;
    RoomClient* client;
    QString server;
    int port;

private slots:
    void on_actionConnect_triggered(bool);
    void on_actionClose_triggered(bool);
    void on_actionSync_triggered(bool);
    void on_actionRefresh_triggered(bool);
    void on_actionEventStop_triggered(bool);
    void on_actionEventSnooze_triggered(bool);
    void on_actionEventTest_triggered(bool);
    void on_actionAbout_triggered(bool);
    void on_actionAboutQt_triggered(bool);
    void on_tabWidget_currentChanged(int);

public Q_SLOTS:
    void stateChanged(RoomClientState);
    void showMessage(const QString& msg);
    void updateConnectionsSettings();
};

#endif // MAINWINDOW_H
