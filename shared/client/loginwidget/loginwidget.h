#pragma once
#include <QStackedWidget>
#include <QCheckBox>
#include <QModelIndex>
#include <QList>
#include <QSystemTrayIcon>
#include <qcoreevent.h>
#include <QMouseEvent>
#include <qdesktopwidget.h>
#include <kstatusnotifieritem.h>
#include <QTimer>
#include <QSortFilterProxyModel>
#include <kwallet.h>

class AbstractServiceProvider;
class NetworkController;
class AbstractStateTracker;
namespace Ui
{
	class LoginWidget;
}

class LoginWidget : public QStackedWidget
{
    Q_OBJECT

public:
	LoginWidget ( NetworkController* network, QWidget *parent = 0 );
	~LoginWidget();

private:
    NetworkController* m_network;
	Ui::LoginWidget *ui;
	int m_reconnectTime;
    QTimer m_reconnectTimer;
    QTimer m_authTimeoutTimer;
    int m_authTimeout;
    bool m_manualDisconnect;
    void backToConnectPage(const QString& msg);

	KWallet::Wallet *m_wallet;
private slots:
	//load
	void deferredLoading();
	
    // gui
    void on_btnConnect_clicked();
	void on_btnDisconnect_clicked();
    void on_btnLogin_clicked();
	void on_btnCancelConnect_clicked();
    void on_cmbHost_currentIndexChanged ( int );

    // network controller
    void connectedToValidServer();
    void serverDisconnected();
    void timeoutData();
    void syncComplete();
    void server_versionmissmatch(const QString& serverversion);
    void auth_failed();
    void auth_notaccepted();
    void auth_success();
    void auth_required(int timeout_ms);

	void authTimeoutUpdate();
	void reconnectUpdate();

	// kwallet
	void walletOpened(bool ok);
	public Q_SLOTS:
		void requestDisconnect();
Q_SIGNALS:
    void executeService(AbstractServiceProvider*);
};
