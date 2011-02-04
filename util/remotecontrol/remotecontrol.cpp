/****************************************************************************

This file is part of the QtMediaHub project on http://www.gitorious.org.

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation (qt-info@nokia.com)**

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

****************************************************************************/

#include "rpc/rpcconnection.h"
#include "remotecontrol.h"
#include "actionmapper.h"

#include <QtGui>

RemoteControl::RemoteControl(QWidget *parent)
    : QWidget(parent), m_connection(0)
{
    ui.setupUi(this);

    QSignalMapper *mapper = new QSignalMapper(this);
    #define MAPBUTTON(button, action) \
        mapper->setMapping(button, action); \
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()))

    MAPBUTTON(ui.upButton, ActionMapper::Up);
    MAPBUTTON(ui.downButton, ActionMapper::Down);
    MAPBUTTON(ui.leftButton, ActionMapper::Left);
    MAPBUTTON(ui.rightButton, ActionMapper::Right);
    MAPBUTTON(ui.okButton, ActionMapper::Enter);
    MAPBUTTON(ui.escButton, ActionMapper::Menu);
    MAPBUTTON(ui.infoButton, ActionMapper::Context);

    connect(mapper, SIGNAL(mapped(int)), this, SLOT(sendButtonPress(int)));

    m_connection = new RpcConnection(RpcConnection::Client, this);
    connect(m_connection, SIGNAL(clientConnected()), this, SLOT(enableButtons()));
    connect(m_connection, SIGNAL(clientDisconnected()), this, SLOT(disableButtons()));

    ui.buttonsFrame->setEnabled(false);
}

RemoteControl::~RemoteControl()
{
}

void RemoteControl::sendButtonPress(int action)
{
    m_connection->call("qmhrpc.takeAction", action);
}

void RemoteControl::connectToService(const QHostAddress &address, int port)
{
    qDebug() << "Connecting to " << address.toString() << ":" << port;
    m_connection->connectToHost(address, port);
}

void RemoteControl::disconnectFromService()
{
    m_connection->disconnectFromHost();
}

void RemoteControl::closeEvent(QCloseEvent *event)
{
    m_connection->disconnectFromHost();
}

void RemoteControl::enableButtons()
{
    ui.buttonsFrame->setEnabled(true);
}

void RemoteControl::disableButtons()
{
    ui.buttonsFrame->setEnabled(false);
}
