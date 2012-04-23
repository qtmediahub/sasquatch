/** This file is part of Qt Media Hub**

Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation qmh-development@qt-project.org

You may use this file under the terms of the BSD license
as follows:

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Nokia Corporation and its Subsidiary(-ies)
nor the names of its contributors may be used to endorse or promote
products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. **/

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
