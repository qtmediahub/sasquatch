#include "mainwindow.h"

#include "rpc/rpcconnection.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    m_connected(false)
{
    m_connection = new RpcConnection(RpcConnection::Client, this);
    connect(m_connection, SIGNAL(clientConnected()), this, SLOT(handleConnected()));
    connect(m_connection, SIGNAL(clientDisconnected()), this, SLOT(handleDisconnected()));

    m_connectButton = new QPushButton(tr("Connect"), this);
    m_connectAddress = new QLineEdit("127.0.0.1", this);
    m_connectPort = new QSpinBox(this);
    m_connectPort->setRange(49152,65535);
    m_connectPort->setValue(51337);

    m_refreshButton = new QPushButton(tr("Push"), this);
    m_refreshButton->setEnabled(false);
    connect(m_refreshButton, SIGNAL(clicked()), SLOT(handleRefreshButton()));

    QVBoxLayout *vbox = new QVBoxLayout(this);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->addWidget(m_connectAddress);
    hbox->addWidget(m_connectPort);
    hbox->addWidget(m_connectButton);

    vbox->addLayout(hbox);
    vbox->addWidget(m_refreshButton);

    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(handleConnectButton()));
}

void MainWindow::handleConnected()
{
    m_connected = true;
    m_connectAddress->setEnabled(false);
    m_connectPort->setEnabled(false);
    m_connectButton->setText(tr("Disconnect"));
    m_refreshButton->setEnabled(true);
}

void MainWindow::handleDisconnected()
{
    m_connected = false;
    m_connectAddress->setEnabled(true);
    m_connectPort->setEnabled(true);
    m_connectButton->setText(tr("Connect"));
    m_refreshButton->setEnabled(false);
}

void MainWindow::handleConnectButton()
{
    if (m_connected) {
        m_connection->disconnectFromHost();
    } else {
        m_connection->connectToHost(m_connectAddress->text(), m_connectPort->value());
    }
}

void MainWindow::handleRefreshButton()
{
    m_connection->call("pushqml.refresh");
}
