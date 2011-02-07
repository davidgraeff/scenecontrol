#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QStringList>
#include <QSettings>
#include <QUuid>
#include <QSize>
#include <qcompleter.h>
#include <qmessagebox.h>
#include <QHostAddress>
#include <kicon.h>
#include <shared/client/networkcontroller.h>
#include <QApplication>
#include <QMenu>
#include <QInputDialog>
#include <shared/abstractserviceprovider.h>

#define RECONNECTTIME_INIT 5000

LoginWidget::LoginWidget ( NetworkController* network, QWidget* parent )
        : QStackedWidget ( parent ), m_network(network), ui ( new Ui::LoginWidget )
{
    ui->setupUi ( this );
    ui->lineUser->setPlaceholderText(tr("Benutzername"));
    ui->linePwd->setPlaceholderText(tr("Passwort"));
    ui->linePwd->setEchoMode(QLineEdit::Password);
    ui->btnCancelConnect->setIcon(KIcon(QLatin1String("dialog-cancel")));
    ui->btnConnect->setIcon(KIcon(QLatin1String("network-connect")));
    setCurrentWidget(ui->pageSettings);

    QTimer::singleShot(0,this,SLOT(deferredLoading()));
}

LoginWidget::~LoginWidget()
{
#ifdef WITHKDE
    delete m_wallet;
#endif
    delete ui;
}

#ifdef WITHKDE
void LoginWidget::walletOpened(bool ok)
{

    if (ok &&
            (m_wallet->hasFolder(KWallet::Wallet::FormDataFolder()) ||
             m_wallet->createFolder(KWallet::Wallet::FormDataFolder())) &&
            m_wallet->setFolder(KWallet::Wallet::FormDataFolder())) {
        ui->chkPwd->setEnabled(true);
    } else
        ui->chkPwd->setEnabled(false);

}
#endif

void LoginWidget::reconnectUpdate()
{
    m_reconnectTime -= m_reconnectTimer.interval();
    if (m_reconnectTime<0) {
        m_reconnectTime = RECONNECTTIME_INIT;
        on_btnConnect_clicked();
    }
    ui->connect_status_label->setText(tr("Wiederverbindung in %1s").arg(m_reconnectTime/1000.0,3,'g',1));
}

void LoginWidget::on_btnConnect_clicked()
{
    QSettings settings;
    const QString host = ui->cmbHost->currentText();
    const int port = ui->spinPort->value();
    settings.beginGroup(QLatin1String("hosts"));
    settings.setValue(QLatin1String("host"), host);
    settings.setValue(QLatin1String("port"), port);
    settings.beginGroup(host);
    settings.setValue(QLatin1String("host"), host);
    settings.setValue(QLatin1String("port"), port);
    settings.endGroup();
    settings.endGroup();
    m_network->start(host, port);
}

void LoginWidget::on_cmbHost_currentIndexChanged(int)
{
    QSettings settings;
    settings.beginGroup(QLatin1String("hosts"));
    settings.beginGroup(ui->cmbHost->currentText());
    //ui->cmbHost->setEditText(settings.value("host","127.0.0.1").toString());
    ui->spinPort->setValue(settings.value(QLatin1String("port"),3101).toInt());
}

void LoginWidget::connectedToValidServer()
{
    m_manualDisconnect = false;
    ui->groupLogin->setEnabled(false);
    ui->login_status_label->setText(tr("Serverdaten abfragen..."));
    setCurrentWidget(ui->pageLoading);
    m_reconnectTimer.stop();
}

void LoginWidget::serverDisconnected()
{
    if (m_manualDisconnect) return;
    m_reconnectTimer.start();
    backToConnectPage(tr("Not connected"));
}

void LoginWidget::requestDisconnect()
{
    backToConnectPage(tr("Getrennt"));
}

void LoginWidget::on_btnLogin_clicked() {
    ui->groupLogin->setEnabled(false);
    m_network->authenticate(ui->lineUser->text(), ui->linePwd->text());
    ui->login_status_label->setText(tr("Authentifizierungsdaten validieren"));
}

void LoginWidget::timeoutData() {
    serverDisconnected();
}

void LoginWidget::syncComplete() {
}

void LoginWidget::server_versionmissmatch(const QString& serverversion) {
    backToConnectPage(tr("Version missmatch: %1").arg(serverversion));
}

void LoginWidget::auth_failed() {
    ui->groupLogin->setEnabled(true);
    ui->lineUser->setFocus();
    ui->login_status_label->setText(tr("Falsche Logindaten!"));
}

void LoginWidget::auth_notaccepted() {
    ui->groupLogin->setEnabled(true);
    ui->lineUser->setFocus();
    ui->login_status_label->setText(tr("Warten und erneut versuchen!"));
}
void LoginWidget::auth_success() {
    ui->groupLogin->setEnabled(false);
    QSettings settings;
    settings.beginGroup(QLatin1String("login_form"));
    settings.setValue(QLatin1String("save_data"), ui->chkPwd->isChecked());
#ifdef WITHKDE
    if (ui->chkPwd->isChecked()) {
        m_wallet->writeEntry(QLatin1String("login_user"), ui->lineUser->text().toUtf8());
        m_wallet->writePassword(QLatin1String("login_pwd"), ui->linePwd->text());
    }
#endif
    m_authTimeoutTimer.stop();
    ui->login_status_label->setText(tr("Synchronisiere Daten..."));
}

void LoginWidget::auth_required(int timeout_ms) {
    ui->login_status_label->setText(tr("Authentifizierung benötigt!"));
    ui->groupLogin->setEnabled(true);
    QSettings settings;
    ui->chkPwd->blockSignals(true);
    settings.beginGroup(QLatin1String("login_form"));
    if (settings.value(QLatin1String("save_data"), true).toBool()) {
        ui->chkPwd->setChecked(true);
        QString pwd;
        QByteArray user;
#ifdef WITHKDE
        m_wallet->readEntry(QLatin1String("login_user"), user);
        m_wallet->readPassword(QLatin1String("login_pwd"), pwd);
#endif
        ui->lineUser->setText(QString::fromUtf8(user));
        ui->linePwd->setText(pwd);
    } else {
        ui->chkPwd->setChecked(false);
        ui->lineUser->setText(QString());
        ui->linePwd->setText(QString());
    }
    ui->chkPwd->blockSignals(false);
    ui->lineUser->setSelection(0,ui->lineUser->text().size());
    ui->lineUser->setFocus();
    m_authTimeout=timeout_ms-2000;
    m_authTimeoutTimer.start();
}


void LoginWidget::authTimeoutUpdate() {
    m_authTimeout -= 1000;
    ui->login_status_label->setText(tr("Authentifizierungstimeout in %1s").arg(m_authTimeout/1000));
    if (m_authTimeout<0) {
        m_authTimeoutTimer.stop();
        backToConnectPage(tr("Auth Timeout"));
    }
}

void LoginWidget::backToConnectPage(const QString& msg) {
    ui->connect_status_label->setText(msg);
    m_manualDisconnect = true;
    m_network->disconnectFromHost();
    m_authTimeoutTimer.stop();
    m_reconnectTimer.stop();
    setCurrentWidget(ui->pageSettings);
}

void LoginWidget::on_btnCancelConnect_clicked() {
    backToConnectPage(tr("Abgebrochen"));
}

void LoginWidget::deferredLoading() {
    QSettings settings;
    settings.beginGroup(QLatin1String("hosts"));
    QStringList hosts = settings.childGroups();

    m_authTimeoutTimer.setInterval(1000);
    connect(&m_authTimeoutTimer,SIGNAL(timeout()),SLOT(authTimeoutUpdate()));

    for (int i=0;i<hosts.size();++i) {
        settings.beginGroup(hosts[i]);
        ui->cmbHost->addItem(settings.value(QLatin1String("host")).toString(), settings.value(QLatin1String("port")).toInt());
        settings.endGroup();
    }

    //default settings
    ui->cmbHost->setEditText(settings.value ( QLatin1String("host"),QLatin1String("127.0.0.1") ).toString());
    ui->spinPort->setValue(settings.value ( QLatin1String("port"), 3101 ).toInt());

    QCompleter *completer = new QCompleter(hosts, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->cmbHost->setCompleter(completer);

    m_reconnectTime = RECONNECTTIME_INIT;
    m_reconnectTimer.setInterval(100);
    connect(&m_reconnectTimer,SIGNAL(timeout()),SLOT(reconnectUpdate()));

    connect(m_network,SIGNAL(auth_failed()),SLOT(auth_failed()));
    connect(m_network,SIGNAL(auth_notaccepted()),SLOT(auth_notaccepted()));
    connect(m_network,SIGNAL(auth_required(int)),SLOT(auth_required(int)));
    connect(m_network,SIGNAL(auth_success()),SLOT(auth_success()));
    connect(m_network,SIGNAL(connectedToValidServer()),SLOT(connectedToValidServer()));
    connect(m_network,SIGNAL(disconnected()),SLOT(serverDisconnected()));
    connect(m_network,SIGNAL(server_versionmissmatch(QString)),SLOT(server_versionmissmatch(QString)));
    connect(m_network,SIGNAL(syncComplete()),SLOT(syncComplete()));
    connect(m_network,SIGNAL(timeoutData()),SLOT(timeoutData()));
    connect(this, SIGNAL(executeService(AbstractServiceProvider*)), m_network, SLOT(executeService(AbstractServiceProvider*)));

#ifdef WITHKDE
    m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), winId(), KWallet::Wallet::Asynchronous);
    connect(m_wallet, SIGNAL(walletOpened(bool)), SLOT(walletOpened(bool)));
#endif

    ui->connect_status_label->setText(QString());
    on_btnConnect_clicked();
    m_network->loadPlugins();
}
