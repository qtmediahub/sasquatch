/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Nokia Corporation donald.carr@nokia.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "httpserver.h"

#include "httpclient.h"
#include "globalsettings.h"
#include "skinmanager.h"

HttpServer::HttpServer(GlobalSettings *settings, quint16 port, QObject *parent) :
    QTcpServer(parent),
    m_settings(settings)
{    
    // FIXME whole address thing should be improved
    m_address = m_settings->value(GlobalSettings::StreamingAddress).toString();
    if (m_address == "")
        m_address = getAddress();

    if (listen(
            #ifdef QT5
                QHostAddress::AnyIPv4,
            #else
                QHostAddress::Any,
            #endif
                port))
        qDebug() << "Streaming server listening" << m_address << "on" << serverPort();
    else
        qDebug() << "Streaming server failed to listen on"
                 << m_address
                 << "on"
                 << serverPort()
                 << "with the following error"
                 << errorString();

    m_skinManager = new SkinManager(m_settings, this);
}

#ifdef QT5
void HttpServer::incomingConnection(qintptr socket)
#else
void HttpServer::incomingConnection(int socket)
#endif
{
    QThread *thread = new QThread(this);
    HttpClient *client = new HttpClient(socket, this, m_skinManager);
    client->moveToThread(thread);
    connect(client, SIGNAL(disconnected()), client, SLOT(deleteLater()));
    connect(client, SIGNAL(disconnected()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

QString HttpServer::getAddress()
{
    foreach (QNetworkInterface i, QNetworkInterface::allInterfaces()) {
        if (!(i.flags() & QNetworkInterface::IsLoopBack) && (i.flags() & QNetworkInterface::IsUp)) {
            foreach (QNetworkAddressEntry a,  i.addressEntries()) {
                if (a.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    return a.ip().toString();
                }
            }
        }
    }

    return QString::fromLatin1("127.0.0.1");
}
