#include "mainwindow.h"
#include <QtGui>

#include "remotecontrol.h"

#ifndef QMH_NO_AVAHI
#include "avahiservicebrowserview.h"
#else
#include "staticservicebrowserview.h"
#include "ui_addservicedialog.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{    
#if !defined(Q_WS_MAEMO_5) && !defined(Q_OS_SYMBIAN)
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
#endif
    setWindowTitle(tr("QtMediaHub Remote Control"));

    m_stackedWidget = new QStackedWidget;
    setCentralWidget(m_stackedWidget);

    m_remoteControl = new RemoteControl;
    m_stackedWidget->addWidget(m_remoteControl);

#ifndef QMH_NO_AVAHI
    m_serviceBrowserView = new AvahiServiceBrowserView;
    m_optionsAction = new QAction(this); // dummy
#else
    m_serviceBrowserView = new StaticServiceBrowserView;

    QMenu *menu = new QMenu;
    menu->addAction(tr("Add Service"), this, SLOT(addService()));
    menu->addAction(tr("Remove Service"), m_serviceBrowserView, SLOT(removeService()));

    m_optionsAction = new QAction(this);
    m_optionsAction->setText(tr("Options"));
    m_optionsAction->setMenu(menu);
    m_optionsAction->setSoftKeyRole(QAction::PositiveSoftKey);
    addAction(m_optionsAction);
#endif
    m_stackedWidget->addWidget(m_serviceBrowserView);
    connect(m_serviceBrowserView, SIGNAL(serviceSelected(QHostAddress, int)),
            this, SLOT(handleServiceSelected(QHostAddress, int)));

    m_backAction = new QAction(this);
    m_backAction->setText(tr("Back"));
    m_backAction->setSoftKeyRole(QAction::PositiveSoftKey);
    connect(m_backAction, SIGNAL(triggered()), this, SLOT(showServiceBrowser()));
    addAction(m_backAction);

    QAction *exitAction = new QAction(this);
    exitAction->setText(tr("Exit"));
    exitAction->setSoftKeyRole(QAction::NegativeSoftKey);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    addAction(exitAction);

    showServiceBrowser();

    // create network connection
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN)
    QNetworkConfigurationManager manager;

    const bool selectIap = (manager.capabilities()& QNetworkConfigurationManager::CanStartAndStopInterfaces);
    QNetworkConfiguration defaultIap = manager.defaultConfiguration();

    if(!defaultIap.isValid() && (!selectIap && defaultIap.state() != QNetworkConfiguration::Active)) {
        return;
    }

    m_session = new QNetworkSession(defaultIap,this);
    m_session->open();
#endif
}

MainWindow::~MainWindow()
{
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN)
    m_session->close();
#endif
}

void MainWindow::showServiceBrowser()
{
    m_remoteControl->disconnectFromService();
    m_stackedWidget->setCurrentWidget(m_serviceBrowserView);
    m_backAction->setVisible(false);
    m_optionsAction->setVisible(true);
}

void MainWindow::handleServiceSelected(const QHostAddress &address, int port)
{
    m_remoteControl->connectToService(address, port);
    m_stackedWidget->setCurrentWidget(m_remoteControl);
    m_optionsAction->setVisible(false);
    m_backAction->setVisible(true);
}

#ifdef QMH_NO_AVAHI
void MainWindow::addService()
{
    QDialog dialog(this);
    Ui::AddServiceDialog ui;
    ui.setupUi(&dialog);
    connect(ui.buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(ui.buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    dialog.resize(width(), dialog.height()); // ##

    if (dialog.exec() == QDialog::Accepted) {
        QString hostname = ui.hostnameEdit->text();
        QString ip = ui.ipEdit->text();
        QString port = ui.portEdit->text();

        if (!hostname.isEmpty() && !ip.isEmpty() && !port.isEmpty())
            (static_cast<StaticServiceBrowserView *>(m_serviceBrowserView))->addService(hostname, ip, port);
    }
}
#endif
