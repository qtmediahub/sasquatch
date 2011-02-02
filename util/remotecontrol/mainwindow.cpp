#include "mainwindow.h"
#include <QtGui>

#include "remotecontrol.h"

#ifndef QMH_NO_AVAHI
#include "avahiservicebrowserview.h"
#else
#include "staticservicebrowserview.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{    
#if !defined(Q_WS_MAEMO_5) && !defined(Q_OS_SYMBIAN)
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
#endif

    m_stackedWidget = new QStackedWidget;
    setCentralWidget(m_stackedWidget);

    m_remoteControl = new RemoteControl;
    m_stackedWidget->addWidget(m_remoteControl);
    connect(m_remoteControl, SIGNAL(disconnected()), this, SLOT(showServiceBrowser()));

#ifndef QMH_NO_AVAHI
    m_serviceBrowserView = new AvahiServiceBrowserView;
#else
    m_serviceBrowserView = new StaticServiceBrowserView;
#endif
    m_stackedWidget->addWidget(m_serviceBrowserView);
    connect(m_serviceBrowserView, SIGNAL(serviceSelected(QHostAddress, int)),
            this, SLOT(handleServiceSelected(QHostAddress, int)));

    m_stackedWidget->setCurrentWidget(m_serviceBrowserView);

    QAction *exitAction = new QAction(this);
    exitAction->setText(tr("Exit"));
    exitAction->setSoftKeyRole(QAction::NegativeSoftKey);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    addAction(exitAction);
}

void MainWindow::showServiceBrowser()
{
    m_stackedWidget->setCurrentWidget(m_serviceBrowserView);
}

void MainWindow::handleServiceSelected(const QHostAddress &address, int port)
{
    m_remoteControl->connectToService(address, port);
    m_stackedWidget->setCurrentWidget(m_remoteControl);
}
