#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientwidget.h"
#include "tabs/alarmswidget.h"
#include "tabs/channelwidget.h"
#include "tabs/pinwidget.h"
#include "tabs/profileswidget.h"
#include <QStringList>
#include <QSettings>
#include <QUuid>
#include <QSize>
#include "tabs/actionswidget.h"
#include <qcompleter.h>
#include "settingsdialog.h"
#include <qmessagebox.h>
#include "tabs/playlistwidget.h"
#include "tabs/musicwidget.h"

MainWindow::MainWindow ( QWidget *parent )
        : QMainWindow ( parent ), ui ( new Ui::MainWindow )
{
    ui->setupUi ( this );
    ui->tabWidget->clear();



    settingsdialog = new SettingsDialog ( this );
    settingsdialog->hide();

    client = new RoomClient();
    connect ( client,SIGNAL ( stateChanged ( RoomClientState ) ),SLOT ( stateChanged ( RoomClientState ) ) );

    stateChanged ( ConnectionDisconnected );

    ClientWidget* clientwidget;

    clientwidget = new ActionsWidget ( client );
    connect ( clientwidget, SIGNAL ( showMessage ( QString ) ), SLOT ( showMessage ( QString ) ) );
    ui->tabWidget->addTab ( clientwidget, clientwidget->getText() );

    clientwidget = new ProfilesWidget ( client, this );
    connect ( clientwidget, SIGNAL ( showMessage ( QString ) ), SLOT ( showMessage ( QString ) ) );
    ui->tabWidget->addTab ( clientwidget, clientwidget->getText() );

    clientwidget = new AlarmWidget ( client, this );
    connect ( clientwidget, SIGNAL ( showMessage ( QString ) ), SLOT ( showMessage ( QString ) ) );
    ui->tabWidget->addTab ( clientwidget, clientwidget->getText() );

    clientwidget = new ChannelWidget ( client, this );
    connect ( clientwidget, SIGNAL ( showMessage ( QString ) ), SLOT ( showMessage ( QString ) ) );
    ui->tabWidget->addTab ( clientwidget, clientwidget->getText() );

    clientwidget = new PinWidget ( client );
    connect ( clientwidget, SIGNAL ( showMessage ( QString ) ), SLOT ( showMessage ( QString ) ) );
    ui->tabWidget->addTab ( clientwidget, clientwidget->getText() );

    clientwidget = new PlaylistWidget ( client );
    connect ( clientwidget, SIGNAL ( showMessage ( QString ) ), SLOT ( showMessage ( QString ) ) );
    ui->tabWidget->addTab ( clientwidget, clientwidget->getText() );

    clientwidget = new MusicWidget ( client );
    connect ( clientwidget, SIGNAL ( showMessage ( QString ) ), SLOT ( showMessage ( QString ) ) );
    ui->tabWidget->addTab ( clientwidget, clientwidget->getText() );

    updateConnectionsSettings();
}

MainWindow::~MainWindow()
{
    delete settingsdialog;
    delete client;
    delete ui;
}

void MainWindow::updateConnectionsSettings()
{
    QSettings settings;
    settings.beginGroup ( "hosts" );
    server = settings.value ( "host","127.0.0.1" ).toString();
    port = settings.value ( "port",3101 ).toInt();
    //ui->actionConnect->setText(tr("Verbinden mit %1").arg(server));
    client->close();
    client->connect ( server, port );
    
}

void MainWindow::stateChanged ( RoomClientState state )
{
    if ( state==RoomserverValid )
    {
        ui->statusBar->showMessage ( tr ( "Verbunden mit %1 auf Port %2. Server Version %3" ).arg ( server ).arg ( port ).arg ( client->getServerVersion() ) );
        for ( int i=0; i<ui->tabWidget->count();++i )
        {
            ui->tabWidget->widget ( i )->setEnabled ( true );
        }
        ui->menuAktionen->setEnabled(true);
    }
    else if ( state==ConnectionConnected )
    {
        ui->statusBar->showMessage ( tr ( "Verbunden mit %1 auf Port %2. Warte auf Server API Version..." ).arg ( server ).arg ( port ) );
        client->requestVersion();
    }
    else if ( state==ConnectionDisconnected )
    {
        if ( client->getServerVersion().isEmpty() )
            ui->statusBar->showMessage ( tr ( "Verbindung getrennt" ) );
        else
            ui->statusBar->showMessage ( tr ( "Verbindung getrennt. Server Version: %1" ).arg ( client->getServerVersion() ) );
        for ( int i=0; i<ui->tabWidget->count();++i )
        {
            ui->tabWidget->widget ( i )->setEnabled ( false );
        }
        ui->menuAktionen->setEnabled(false);
    }
    else if ( state==ConnectionFailure )
    {
        ui->statusBar->showMessage ( tr ( "Verbindungsaufbau gescheitert" ) );
	ui->menuAktionen->setEnabled(false);
    }
}

void MainWindow::on_tabWidget_currentChanged ( int )
{
    ClientWidget* w = dynamic_cast<ClientWidget*> ( ui->tabWidget->currentWidget() );
    if ( !w ) return;
    ui->statusBar->showMessage ( tr ( "%1" ).arg ( w->getText() ) );
}

void MainWindow::on_actionClose_triggered ( bool )
{
    close();
}

void MainWindow::on_actionSync_triggered ( bool )
{
    client->refresh();
}

void MainWindow::on_actionRefresh_triggered ( bool )
{
    client->refreshEthersex();
}

void MainWindow::on_actionEventStop_triggered ( bool )
{
    client->eventStop();
}

void MainWindow::on_actionEventSnooze_triggered ( bool )
{
    client->eventSnooze();
}

void MainWindow::on_actionEventTest_triggered ( bool )
{
    client->eventPlay ( "Testsound","/media/roomserver/daten/Audio/Samples/BLACKH.mp3" );
}

void MainWindow::on_actionConnect_triggered ( bool )
{
    settingsdialog->show();
}

void MainWindow::on_actionAbout_triggered ( bool )
{
    QMessageBox::about ( this, tr ( "Über RoomClient" ),
                         tr ( "RoomClient" ) );

}

void MainWindow::on_actionAboutQt_triggered ( bool )
{
    QApplication::aboutQt();
}

void MainWindow::showMessage ( const QString& msg )
{
    ClientWidget* w = dynamic_cast<ClientWidget*> ( this->sender() );
    if ( !w || ui->tabWidget->currentWidget() !=w ) return;
    ui->statusBar->showMessage ( tr ( "%1: %2" ).arg ( w->getText() ).arg ( msg ) );
}
